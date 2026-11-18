"""ZeroMQ REP server side of the OMNeT++ <-> Python MAACO coupling.

Serves one simulation episode: HELLO (layout handshake, shm attach),
DECISION_READY (delegate to agents, write ActionSlot), EPISODE_END.

Run standalone as a random-action stub (M4 verification):
    python -m maaco.bridge --stub [--port 5555] [--shm /maaco]
"""

import argparse
import struct
import numpy as np
import zmq

from . import shm_layout as L

MSG_HELLO = ord("H")
MSG_DECISION = ord("D")
MSG_EPISODE_END = ord("E")

HELLO_FMT = "<BIIIIIII"       # type, magic, version, stateDim, itemFeat, maxItems, rewardCap, runNumber
DECISION_FMT = "<BQ"          # type, seq
EPISODE_END_FMT = "<BQ"       # type, decisions
REPLY_FMT = "<BQ"             # ok, seq


class RandomAgents:
    """M4 stub: uniform random actions; counts decisions per agent type."""

    def __init__(self, seed=0):
        self.rng = np.random.default_rng(seed)
        self.dqn_decisions = 0
        self.a2c_decisions = 0

    def act(self, decision):
        if decision.agentType == L.AGENT_DQN:
            self.dqn_decisions += 1
            return int(self.rng.integers(0, 2)), 0.0
        self.a2c_decisions += 1
        n = max(1, decision.numItems)
        return int(self.rng.integers(0, n)), 0.0

    def observe_rewards(self, events):
        pass

    def on_episode_end(self, total_decisions):
        print(f"[stub] episode end: sim decisions={total_decisions} "
              f"dqn={self.dqn_decisions} a2c={self.a2c_decisions}")


class BridgeServer:
    def __init__(self, port=5555, shm_name="/maaco", agents=None, verbose=True):
        self.port = port
        self.shm_name = shm_name
        self.agents = agents or RandomAgents()
        self.verbose = verbose
        self.ctx = zmq.Context()
        self.sock = self.ctx.socket(zmq.REP)
        self.sock.bind(f"tcp://127.0.0.1:{port}")
        self.shm = None
        self.run_number = None

    def serve_episode(self, proc=None):
        """Block until one full episode (HELLO .. EPISODE_END) is served.

        If `proc` (the simulation subprocess) is given, its liveness is
        checked while waiting so a crashed simulation raises instead of
        deadlocking the trainer.

        Returns the total number of decisions reported by the simulation.
        """
        total = None
        while total is None:
            # wait for a request, checking sim liveness every 2 s
            while not self.sock.poll(2000):
                if proc is not None and proc.poll() is not None:
                    raise RuntimeError(
                        f"simulation exited (rc={proc.returncode}) without "
                        f"completing the episode handshake")
            msg = self.sock.recv()
            mtype = msg[0]

            if mtype == MSG_HELLO:
                (_, magic, version, state_dim, item_feat, max_items,
                 reward_cap, run_number) = struct.unpack(HELLO_FMT, msg)
                L.verify_hello(magic, version, state_dim, item_feat,
                               max_items, reward_cap)
                self.shm = L.ShmView(self.shm_name)
                self.run_number = run_number
                if self.verbose:
                    print(f"[bridge] HELLO ok (run #{run_number}), shm attached: {self.shm_name}")
                self.sock.send(struct.pack(REPLY_FMT, 1, 0))

            elif mtype == MSG_DECISION:
                _, seq = struct.unpack(DECISION_FMT, msg)
                dec = self.shm.seg.decision
                assert dec.seq == seq, f"seq mismatch: shm {dec.seq} != msg {seq}"
                # feed pending reward events to the learner before acting
                events = self.shm.drain_rewards()
                if events:
                    self.agents.observe_rewards(events)
                action, aux = self.agents.act(dec)
                act = self.shm.seg.action
                act.action = int(action)
                act.aux = float(aux)
                act.seq = seq
                self.sock.send(struct.pack(REPLY_FMT, 1, seq))

            elif mtype == MSG_EPISODE_END:
                _, total = struct.unpack(EPISODE_END_FMT, msg)
                if self.shm is not None:
                    events = self.shm.drain_rewards()
                    if events:
                        self.agents.observe_rewards(events)
                self.agents.on_episode_end(total)
                self.sock.send(struct.pack(REPLY_FMT, 1, 0))

            else:
                print(f"[bridge] unknown message type {mtype}, len {len(msg)}")
                self.sock.send(struct.pack(REPLY_FMT, 0, 0))

        if self.shm is not None:
            self.shm.close()
            self.shm = None
        return total

    def close(self):
        self.sock.close(0)
        self.ctx.term()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--stub", action="store_true", help="random-action stub agents")
    ap.add_argument("--port", type=int, default=5555)
    ap.add_argument("--shm", default="/maaco")
    ap.add_argument("--episodes", type=int, default=1)
    ap.add_argument("--seed", type=int, default=0)
    args = ap.parse_args()

    server = BridgeServer(port=args.port, shm_name=args.shm,
                          agents=RandomAgents(seed=args.seed))
    print(f"[bridge] listening on tcp://127.0.0.1:{args.port}, serving "
          f"{args.episodes} episode(s)")
    try:
        for ep in range(args.episodes):
            n = server.serve_episode()
            print(f"[bridge] episode {ep + 1}/{args.episodes} done ({n} decisions)")
    finally:
        server.close()


if __name__ == "__main__":
    main()
