"""Frozen-policy evaluation runs (thesis RO4).

Loads trained checkpoints, serves the bridge with greedy policies
(epsilon=0 / argmax), and sweeps configs x seeds. Baseline (non-RL) configs
are launched without the bridge (mode is baked into the config).

Usage:
  python -m maaco.eval --config SmartFarm-MAACO --seeds 10 \
      --dqn-ckpt ../results/checkpoints/SmartFarm-DQNonly/dqn_final.pt \
      --a2c-ckpt ../results/checkpoints/SmartFarm-MAACO/a2c_final.pt
  python -m maaco.eval --baseline SmartFarm-LCE-LRU --seeds 10
"""

import argparse
import json
import os
import pathlib
import subprocess
import numpy as np

import torch
torch.set_num_threads(1)

from .agents import MaacoAgents, HeuristicA2C
from .bridge import BridgeServer
from .coordinator import RewardCoordinator
from .dqn_placement import DQNPlacementAgent

PROJ = pathlib.Path(__file__).resolve().parents[2]
RUN = PROJ / "simulations" / "run"


def sim_cmd(config, seed, sim_time, result_dir, sim_opts=(), stem=None):
    stem = stem or config
    return [str(RUN), "-c", config, "-f", "omnetpp.ini", "-r", "0",
            f"--seed-set={seed}", f"--sim-time-limit={sim_time}",
            f"--result-dir={result_dir}",
            f"--output-scalar-file={result_dir}/{stem}-s{seed}.sca",
            f"--output-vector-file={result_dir}/{stem}-s{seed}.vec",
            "--**.vector-recording=false"] + list(sim_opts)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", help="RL config to evaluate (needs bridge)")
    ap.add_argument("--baseline", help="non-RL config (no bridge needed)")
    ap.add_argument("--seeds", type=int, default=10)
    ap.add_argument("--seed-offset", type=int, default=0,
                    help="first seed-set (validation runs use an offset "
                         "disjoint from the 0..seeds-1 test range)")
    ap.add_argument("--sim-time", default="600s")
    ap.add_argument("--port", type=int, default=5571)
    ap.add_argument("--dqn-ckpt")
    ap.add_argument("--a2c-ckpt")
    ap.add_argument("--pin-coord", default=None, metavar="IDX:VALUE",
                    help="overwrite one observation coordinate at inference, e.g. "
                         "9:1.0 pins residual energy at full. Used to ask whether a "
                         "trained policy's decisions actually depend on a coordinate, "
                         "holding the network and the policy fixed.")
    ap.add_argument("--placement", default="dqn",
                    choices=["dqn", "mab", "tabq", "rlcache", "recency", "popularity"],
                    help="which placement learner the --dqn-ckpt belongs to")
    ap.add_argument("--placement-args", default="",
                    help="JSON kwargs for the placement baseline (recency "
                         'needs no checkpoint: {"threshold": 30})')
    ap.add_argument("--result-dir", default="../../results/eval")
    ap.add_argument("--sim-opt", action="append", default=[],
                    help="extra opp option(s) passed to the simulation, e.g. "
                         "--sim-opt='--*.sensor[*].app[0].csMaxEntries=5'")
    ap.add_argument("--stem", default=None,
                    help="result-file stem override (default: config name)")
    args = ap.parse_args()

    (PROJ / "results" / "eval").mkdir(parents=True, exist_ok=True)

    if args.baseline:
        for seed in range(args.seed_offset, args.seed_offset + args.seeds):
            rc = subprocess.call(sim_cmd(args.baseline, seed, args.sim_time,
                                         args.result_dir, args.sim_opt,
                                         args.stem),
                                 stdout=subprocess.DEVNULL)
            print(f"[eval] {args.baseline} seed={seed} rc={rc}")
        return

    assert args.config, "--config or --baseline required"

    dqn = a2c = None
    if args.placement in ("recency", "popularity"):
        from .baselines import make_placement_agent
        kw = json.loads(args.placement_args) if args.placement_args else {}
        dqn = make_placement_agent(args.placement, **kw)
    elif args.dqn_ckpt:
        if args.placement == "dqn":
            dqn = DQNPlacementAgent()
        else:
            from .baselines import make_placement_agent
            kw = json.loads(args.placement_args) if args.placement_args else {}
            dqn = make_placement_agent(args.placement, **kw)
        dqn.load(args.dqn_ckpt)
        dqn.greedy = True
    if args.a2c_ckpt:
        from .a2c_replacement import A2CReplacementAgent
        a2c = A2CReplacementAgent()
        a2c.load(args.a2c_ckpt)
        a2c.greedy = True
    else:
        a2c = HeuristicA2C()

    if args.pin_coord:
        idx, val = args.pin_coord.split(":")
        idx, val = int(idx), float(val)
        inner = dqn

        class _PinnedCoord:
            """Wraps a frozen placement policy and overwrites one coordinate of
            every observation before the policy sees it.  Nothing about the
            simulator or the weights changes, so any difference in the result is
            attributable to that coordinate alone."""

            def __init__(self, a): self._a = a
            def __getattr__(self, n): return getattr(self._a, n)
            def act(self, state, mask=3):
                s = np.array(state, dtype=np.float32, copy=True)
                s[idx] = val
                return self._a.act(s, mask)

        dqn = _PinnedCoord(inner)
        print(f"[eval] pinning observation coordinate {idx} at {val}")

    coord = RewardCoordinator(mode="shared")
    agents = MaacoAgents(dqn=dqn, a2c=a2c, coordinator=coord)
    server = BridgeServer(port=args.port, shm_name=f"maaco_{args.port}",
                          agents=agents, verbose=False)

    env = os.environ.copy()
    env["MAACO_PORT"] = str(args.port)
    env["MAACO_SHM"] = f"maaco_{args.port}"

    for seed in range(args.seed_offset, args.seed_offset + args.seeds):
        for attempt in range(3):   # transient inter-episode shm/port races
            proc = subprocess.Popen(sim_cmd(args.config, seed, args.sim_time,
                                            args.result_dir, args.sim_opt,
                                            args.stem),
                                    env=env, stdout=subprocess.DEVNULL)
            try:
                server.serve_episode(proc=proc)
                rc = proc.wait()
                break
            except RuntimeError as e:
                proc.wait()
                print(f"[eval] seed={seed} attempt {attempt + 1} failed: {e}")
                import time
                time.sleep(1.0)
        else:
            print(f"[eval] seed={seed} FAILED after retries")
            coord.reset()
            continue
        s = coord.episode_summary()
        print(f"[eval] {args.config} seed={seed} rc={rc} "
              f"hit={s['hitRatio']:.3f} fresh={s['freshnessSat']:.3f}")
        coord.reset()

    server.close()


if __name__ == "__main__":
    main()
