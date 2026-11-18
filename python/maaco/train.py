"""Episodic MAACO training loop.

One episode = one OMNeT++ run: train.py picks a seed, exports MAACO_PORT /
MAACO_SHM, spawns the simulation (simulations/run wrapper), serves the ZMQ
bridge until EPISODE_END, learns online throughout, checkpoints, and logs to
TensorBoard. Crash-isolated: a failed episode is logged and skipped.

Usage:
  python -m maaco.train --algo dqn  --config SmartFarm-DQNonly --episodes 100
  python -m maaco.train --algo a2c  --config SmartFarm-A2Conly --episodes 100
  python -m maaco.train --algo both --config SmartFarm-MAACO   --episodes 200 \
      [--reward-mode shared|independent] [--sim-time 600s] [--port 5561]

Every argparse default below is the value the final (v3) thesis campaigns
actually ran with, so the artifact is self-documenting: the only flags those
campaigns passed beyond --algo/--config/--episodes/--sim-time/--port/--tag were
ones that are now defaults. Provenance and the bit-exact replay that
established them: docs/thesis/hyperparameter_provenance.md. Each run prints a
full hyperparameter header and drops a run.json sidecar in its checkpoint
directory; do not silently change a default without updating both documents.
"""

import argparse
import json
import os
import pathlib
import platform
import random
import subprocess
import sys
import time

import numpy as np
import torch

# One BLAS thread per process: these trainers are run many-at-once, and
# torch's default intra-op pool oversubscribes the machine catastrophically
# (measured 25x slowdown with 7 concurrent trainers).
torch.set_num_threads(1)

from .agents import MaacoAgents, HeuristicA2C
from .bridge import BridgeServer
from .coordinator import RewardCoordinator
from .dqn_placement import DQNPlacementAgent

PROJ = pathlib.Path(__file__).resolve().parents[2]
RUN = PROJ / "simulations" / "run"


def make_placement(args):
    """Placement learner: MAACO's DQN by default, or one of the literature
    baselines (`--placement mab|tabq|recency`). All of them expose the same
    interface and run on the identical substrate, reward channel and credit
    horizon."""
    if args.placement == "dqn":
        return DQNPlacementAgent(seed=args.seed, lr=args.lr, gamma=args.gamma,
                                 prior_kappa=args.placement_prior_kappa,
                                 buffer_size=args.buffer, batch_size=args.batch,
                                 eps_decay_steps=args.eps_decay,
                                 eps_end=args.eps_end, tau=args.tau)
    from .baselines import make_placement_agent
    kw = json.loads(args.placement_args) if args.placement_args else {}
    if args.placement in ("mab", "tabq"):
        kw.setdefault("eps_decay_steps", args.eps_decay)
        kw.setdefault("eps_end", args.eps_end)
    if args.placement == "tabq":
        kw.setdefault("gamma", args.gamma)
    return make_placement_agent(args.placement, seed=args.seed, **kw)


def make_agents(args, log_cb):
    dqn = a2c = None
    if args.algo in ("dqn", "both"):
        dqn = make_placement(args)
    if args.algo in ("a2c", "both"):
        try:
            from .a2c_replacement import A2CReplacementAgent
            a2c = A2CReplacementAgent(seed=args.seed, prior_kappa=args.prior_kappa,
                                       prior_mode=args.prior_mode)
        except ImportError:
            print("[train] a2c_replacement not available yet; using LFF heuristic")
            a2c = HeuristicA2C()
    coord = RewardCoordinator(mode=args.reward_mode, w_hit=args.w_hit,
                              w_stale=args.w_stale, w_energy=args.w_energy, w_miss=args.w_miss,
                              w_insert=args.w_insert, w_local=args.w_local,
                              w_shared=args.w_shared)
    agents = MaacoAgents(dqn=dqn, a2c=a2c, coordinator=coord, log_cb=log_cb,
                         credit_horizon=args.credit_horizon,
                         horizon_mode=args.horizon_mode,
                         bootstrap=args.bootstrap)
    return agents, dqn, a2c, coord


def run_provenance(args, run_name, dqn, a2c):
    """Everything needed to re-run this campaign, as a plain dict.

    Written both to the log header and to a run.json sidecar in the checkpoint
    directory. The v3 campaigns predate this and recorded only the reward
    weights, which is why gamma had to be recovered by replay afterwards
    (docs/thesis/hyperparameter_provenance.md). Nothing that affects a
    trajectory should be missing from here.
    """
    prov = {
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "run_name": run_name,
        "config": args.config,
        "algo": args.algo,
        "episodes": args.episodes,
        "sim_time": args.sim_time,
        "seed": args.seed,
        "port": args.port,
        "tag": args.tag,
        "reward_mode": args.reward_mode,
        "checkpoint_every": args.checkpoint_every,
        "sim_opt": list(args.sim_opt),
        "cs_sizes": args.cs_sizes,
        "battery_sizes": args.battery_sizes,
        "init_dqn": args.init_dqn,
        "init_a2c": args.init_a2c,
        "reward_weights": {
            "w_hit": args.w_hit, "w_miss": args.w_miss, "w_stale": args.w_stale,
            "w_energy": args.w_energy, "w_insert": args.w_insert,
            "w_local": args.w_local, "w_shared": args.w_shared,
            "credit_horizon_s": args.credit_horizon,
            "horizon_mode": args.horizon_mode,
        },
        "placement_bootstrap": args.bootstrap,
        "placement_family": args.placement,
        "placement_args": args.placement_args,
        "placement_dqn": dqn.hparams() if dqn is not None else None,
        "replacement_a2c": ({
            "class": type(a2c).__name__,
            "lr": float(a2c.opt.param_groups[0]["lr"]),
            "gamma": float(a2c.gamma),
            "entropy_start": float(a2c.entropy_start),
            "entropy_end": float(a2c.entropy_end),
            "entropy_decay_steps": int(a2c.entropy_decay_steps),
            "update_every": int(a2c.update_every),
            "prior_kappa": float(getattr(a2c, "prior_kappa", 0.0)),
            "prior_mode": getattr(a2c, "prior_mode", None),
        } if a2c is not None and hasattr(a2c, "opt") else
            (type(a2c).__name__ if a2c is not None else None)),
        "code": {
            # No VCS here, so identify the code by content: mtime + size of
            # every module that can change a trajectory.
            "python": sys.version.split()[0],
            "torch": torch.__version__,
            "host": platform.node(),
            "sim_binary": os.environ.get("MAACO_BIN", str(RUN)),
            "modules": {
                p.name: {"mtime": time.strftime(
                             "%Y-%m-%dT%H:%M:%S",
                             time.localtime(p.stat().st_mtime)),
                         "bytes": p.stat().st_size}
                for p in sorted(pathlib.Path(__file__).resolve().parent
                                .glob("*.py"))
            },
        },
    }
    return prov


def provenance_header(prov):
    """The original two header lines (kept verbatim so old logs stay
    diffable against new ones) plus the full hyperparameter record."""
    w = prov["reward_weights"]
    d = prov["placement_dqn"]
    a = prov["replacement_a2c"]
    lines = [
        f"[train] {prov['algo']} on {prov['config']}: {prov['episodes']} episodes, "
        f"sim-time {prov['sim_time']}, reward={prov['reward_mode']}, port {prov['port']}",
        f"[train] weights: hit={w['w_hit']} stale={w['w_stale']} "
        f"insert={w['w_insert']} local={w['w_local']} "
        f"horizon={w['credit_horizon_s']}s",
        f"[train] weights(full): hit={w['w_hit']} miss={w['w_miss']} "
        f"stale={w['w_stale']} insert={w['w_insert']} energy={w['w_energy']} "
        f"local={w['w_local']} shared={w['w_shared']} "
        f"horizon={w['credit_horizon_s']}s",
    ]
    if d and prov.get("placement_family", "dqn") != "dqn":
        lines.append(f"[train] placement({prov['placement_family']}): "
                     + " ".join(f"{k}={v}" for k, v in sorted(d.items())))
    elif d:
        lines.append(
            f"[train] placement(dqn): gamma={d['gamma']} lr={d['lr']} "
            f"batch={d['batch_size']} buffer={d['buffer_size']} "
            f"tau={d['tau']} train_every={d['train_every']} "
            f"warmup={d['warmup']} eps={d['eps_start']}->{d['eps_end']}"
            f"/{d['eps_decay_steps']}dec")
    if isinstance(a, dict):
        lines.append(
            f"[train] replacement(a2c): gamma={a['gamma']} lr={a['lr']} "
            f"entropy={a['entropy_start']}->{a['entropy_end']}"
            f"/{a['entropy_decay_steps']} update_every={a['update_every']}")
    elif a is not None:
        lines.append(f"[train] replacement: {a} (no learned parameters)")
    lines.append(
        f"[train] run={prov['run_name']} seed={prov['seed']} "
        f"tag={prov['tag'] or '-'} started={prov['timestamp']} "
        f"torch={prov['code']['torch']} py={prov['code']['python']}")
    return lines


def run_episode(args, episode, seed):
    env = os.environ.copy()
    env["MAACO_PORT"] = str(args.port)
    env["MAACO_SHM"] = f"maaco_{args.port}"
    # include the tag in result filenames so parallel sweeps never clobber
    # each other's episode scalars
    stem = args.config + (f"-{args.tag}" if args.tag else "")
    cmd = [str(RUN), "-c", args.config, "-f", "omnetpp.ini",
           "-r", "0", f"--seed-set={seed}",
           f"--sim-time-limit={args.sim_time}",
           "--result-dir=../../results/train",
           f"--output-scalar-file=../../results/train/{stem}-ep{episode}.sca",
           f"--output-vector-file=../../results/train/{stem}-ep{episode}.vec",
           "--**.vector-recording=false"] + list(args.sim_opt)
    if args.cs_sizes:
        # capacity randomization: one store size per episode, drawn from the
        # episode seed so a campaign is reproducible
        sizes = [int(x) for x in args.cs_sizes.split(",")]
        cap = sizes[random.Random(seed).randrange(len(sizes))]
        cmd.append(f"--*.sensor[*].app[0].csMaxEntries={cap}")
    if args.battery_sizes:
        # battery randomization: the residual-energy feature only leaves 1.0 on
        # a small battery, so a policy trained at 50 J alone never sees it move.
        # Drawn from its own stream so the capacity draw above is unchanged.
        batts = [float(x) for x in args.battery_sizes.split(",")]
        b = batts[random.Random(seed * 7919 + 1).randrange(len(batts))]
        cmd += [f"--*.sensor[*].energyStorage.nominalCapacity={b}J",
                f"--*.sensor[*].energyStorage.initialCapacity={b}J"]
        if b < 50:
            # small-battery episodes mirror the energy-binding evaluation twin
            cmd.append("--*.sensor[*].app[0].cacheEnergyAccounting=true")
    return subprocess.Popen(cmd, env=env, stdout=subprocess.DEVNULL,
                            stderr=subprocess.STDOUT)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--algo", choices=["dqn", "a2c", "both"], default="dqn")
    ap.add_argument("--config", default="SmartFarm-DQNonly")
    ap.add_argument("--episodes", type=int, default=100)
    ap.add_argument("--sim-time", default="600s")
    ap.add_argument("--port", type=int, default=5561)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--reward-mode", choices=["shared", "independent"], default="shared")
    ap.add_argument("--w-hit", type=float, default=1.0, help="reward weight: cache hit")
    ap.add_argument("--w-stale", type=float, default=1.0, help="reward weight: stale delivery penalty")
    ap.add_argument("--w-energy", type=float, default=0.0, help="reward weight: Joules drained per energy report")
    ap.add_argument("--w-miss", type=float, default=0.2,
                    help="reward weight: cache miss penalty")
    ap.add_argument("--w-insert", type=float, default=0.2,
                    help="reward weight: per-admission cache-write cost (EV_CS_INSERT)")
    ap.add_argument("--w-local", type=float, default=1.0,
                    help="weight of the name-attributed per-decision credit channel "
                         "(0 = original pure shared-window reward)")
    ap.add_argument("--w-shared", type=float, default=0.1,
                    help="scale of the network-wide shared-utility reward channel")
    ap.add_argument("--horizon-mode", choices=["fixed", "lifetime"], default="fixed",
                    help="placement credit window: the fixed --credit-horizon, or each "
                         "packet's own freshness period")
    ap.add_argument("--credit-horizon", type=float, default=60.0,
                    help="sim-seconds over which an admission's own copy is credited "
                         "(0 = close transitions at the next decision, as before)")
    ap.add_argument("--placement", default="dqn",
                    choices=["dqn", "mab", "tabq", "rlcache", "recency", "popularity"],
                    help="placement (cache-admission) learner: MAACO's DQN, "
                         "the Cache-MAB bandit baseline, tabular Q-learning, "
                         "or the recency heuristic control")
    ap.add_argument("--placement-args", default="",
                    help="JSON dict of extra kwargs for the placement baseline, "
                         'e.g. \'{"arms":"content","explore":"ucb1","c":0.5}\'')
    ap.add_argument("--tag", default="", help="suffix for checkpoint/tensorboard naming (weight sweeps)")
    ap.add_argument("--prior-kappa", type=float, default=0.0,
                    help="replacement actor: weight of the aged-frequency prior (0 = none)")
    ap.add_argument("--prior-mode", choices=["freq", "value"], default="value",
                    help="replacement prior: aged frequency, or frequency x remaining origin life")
    ap.add_argument("--cs-sizes", default="",
                    help="comma list of Content Store capacities drawn per episode (empty = config value)")
    ap.add_argument("--placement-prior-kappa", type=float, default=0.0,
                    help="placement Q-network: weight of the admit-value prior "
                         "(freqShort x remaining lifetime); 0 = none")
    ap.add_argument("--init-dqn", default="", help="placement checkpoint to fine-tune from")
    ap.add_argument("--init-a2c", default="", help="replacement checkpoint to fine-tune from")
    ap.add_argument("--battery-sizes", default="",
                    help="comma list of battery capacities in J drawn per episode (empty = config value)")
    ap.add_argument("--lr", type=float, default=3e-4)
    ap.add_argument("--gamma", type=float, default=0.9,
                    help="PLACEMENT discount, applied to the value of the first "
                         "decision after this one's credit horizon closes "
                         "(--bootstrap post-horizon). 0 makes placement a "
                         "contextual bandit, which is what the pre-2026 "
                         "campaigns ran. Replacement (A2C) is hard-wired to "
                         "0.98 and is not affected by this flag.")
    ap.add_argument("--bootstrap", choices=["post-horizon", "next"],
                    default="post-horizon",
                    help="which state the placement TD target bootstraps on: "
                         "the first decision after the credit window closes "
                         "(default, non-overlapping, so gamma > 0 is well "
                         "posed), or the immediately next decision (pre-2026 "
                         "behaviour, only meaningful at gamma = 0)")
    ap.add_argument("--buffer", type=int, default=50_000)
    ap.add_argument("--batch", type=int, default=64)
    ap.add_argument("--eps-decay", type=int, default=250_000,
                    help="placement decisions over which epsilon anneals "
                         "1.0 -> --eps-end (~34 episodes at 7.4k decisions/ep)")
    ap.add_argument("--eps-end", type=float, default=0.05)
    ap.add_argument("--tau", type=float, default=0.001)
    ap.add_argument("--checkpoint-every", type=int, default=10)
    ap.add_argument("--logdir", default=str(PROJ / "results" / "tb"))
    ap.add_argument("--sim-opt", action="append", default=[],
                    help="extra opp option(s) forwarded to every training episode")
    args = ap.parse_args()

    random.seed(args.seed)
    np.random.seed(args.seed)
    torch.manual_seed(args.seed)

    (PROJ / "results" / "train").mkdir(parents=True, exist_ok=True)
    run_name = args.config + (f"-{args.tag}" if args.tag else "")
    ckpt_dir = PROJ / "results" / "checkpoints" / run_name
    ckpt_dir.mkdir(parents=True, exist_ok=True)

    try:
        from torch.utils.tensorboard import SummaryWriter
        writer = SummaryWriter(log_dir=f"{args.logdir}/{run_name}")
    except ImportError:
        writer = None
        print("[train] tensorboard unavailable; console logging only")

    episode_stats = {}

    def log_cb(stats):
        episode_stats.update(stats)

    agents, dqn, a2c, coord = make_agents(args, log_cb)
    # Fine-tuning: continue from a trained model instead of from scratch. The
    # step counters come with the weights, so exploration and the entropy
    # bonus resume at their annealed values.
    if args.init_dqn and dqn is not None:
        dqn.load(args.init_dqn)
    if args.init_a2c and a2c is not None and hasattr(a2c, "load"):
        a2c.load(args.init_a2c)
    server = BridgeServer(port=args.port, shm_name=f"maaco_{args.port}",
                          agents=agents, verbose=False)

    prov = run_provenance(args, run_name, dqn, a2c)
    for line in provenance_header(prov):
        print(line)
    sidecar = ckpt_dir / "run.json"
    sidecar.write_text(json.dumps(prov, indent=2, sort_keys=True) + "\n")
    print(f"[train] provenance sidecar: {sidecar}", flush=True)

    for ep in range(args.episodes):
        seed = args.seed * 100_000 + ep
        t0 = time.time()
        proc = run_episode(args, ep, seed)
        try:
            sim_decisions = server.serve_episode(proc=proc)
        except Exception as e:
            print(f"[train] episode {ep} bridge error: {e}; killing sim")
            proc.kill()
            proc.wait()
            # The episode died mid-run.  Drop its open transitions instead of
            # carrying them into the next episode, and clear the stats dict so
            # the next episode does not inherit this one's counters.
            if hasattr(agents, "abort_episode"):
                agents.abort_episode()
            coord.reset()
            episode_stats.clear()
            continue
        rc = proc.wait()
        wall = time.time() - t0

        s = dict(episode_stats)
        py_decisions = s.get("dqnDecisions", 0) + s.get("a2cDecisions", 0)
        line = (f"[ep {ep:4d}] seed={seed} rc={rc} wall={wall:5.1f}s "
                f"hit={s.get('hitRatio', 0):.3f} fresh={s.get('freshnessSat', 0):.3f} "
                f"R={s.get('meanTransitionReward', 0):+.3f} "
                f"dec={s.get('dqnDecisions', 0)}+{s.get('a2cDecisions', 0)}"
                f"{'' if py_decisions == sim_decisions else f' PARITY-MISMATCH(sim={sim_decisions})'}")
        if dqn is not None:
            line += f" eps={s.get('epsilon', 0):.3f} loss={s.get('dqnLoss', float('nan')):.4f}"
        line += (f" R0={s.get('meanRSkip', float('nan')):+.3f}/{s.get('nSkip',0)}"
                 f" R1={s.get('meanRAdmit', float('nan')):+.3f}/{s.get('nAdmit',0)}")
        if "entropy" in s:
            line += f" ent={s['entropy']:.3f}"
        print(line, flush=True)

        if writer:
            for k in ("hitRatio", "freshnessSat", "totalReward",
                      "meanTransitionReward", "epsilon", "dqnLoss",
                      "dqnDecisions", "a2cDecisions", "entropy", "a2cLoss"):
                if k in s and s[k] == s[k]:  # skip NaN
                    writer.add_scalar(k, s[k], ep)
            writer.flush()

        coord.reset()
        episode_stats.clear()

        if (ep + 1) % args.checkpoint_every == 0:
            if dqn is not None:
                dqn.save(ckpt_dir / f"dqn_ep{ep + 1}.pt")
            if a2c is not None and hasattr(a2c, "save"):
                a2c.save(ckpt_dir / f"a2c_ep{ep + 1}.pt")
            # keep the sidecar honest about how far the run actually got: a
            # campaign stopped early (the A2C-only v3 run was) should not be
            # described by its intended episode count alone
            prov["episodes_completed"] = ep + 1
            sidecar.write_text(json.dumps(prov, indent=2, sort_keys=True) + "\n")

    prov["episodes_completed"] = args.episodes
    prov["finished"] = time.strftime("%Y-%m-%dT%H:%M:%S%z")
    sidecar.write_text(json.dumps(prov, indent=2, sort_keys=True) + "\n")

    if dqn is not None:
        dqn.save(ckpt_dir / "dqn_final.pt")
    if a2c is not None and hasattr(a2c, "save"):
        a2c.save(ckpt_dir / "a2c_final.pt")
    server.close()
    if writer:
        writer.close()
    print(f"[train] done; checkpoints in {ckpt_dir}")


if __name__ == "__main__":
    main()
