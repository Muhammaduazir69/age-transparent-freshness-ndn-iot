"""DQN placement agent (thesis RO1): binary cache-admission decisions.

Standard DQN (Mnih et al. 2015), MLP 256x256 -> 2 Q-values, epsilon-greedy,
Polyak target updates, parameters SHARED across all routers (per-router
transitions tagged by agentId; independent-nets ablation via share_params).
Hyperparameters follow thesis 3.8.5 ranges.
"""

import numpy as np
import torch
import torch.nn as nn

from .replay_buffer import ReplayBuffer
from . import shm_layout as L

# State features the admit-value prior reads: short-window request frequency
# of the candidate name (popularity) and its remaining freshness lifetime.
FREQ_SHORT, CAND_REMAIN = 0, 12


class QNet(nn.Module):
    """Q-network with an optional value prior on the admit action.

    With prior_kappa > 0 the admit value is the learned output plus
    kappa * freqShort * remainingLife, so an untrained network already admits
    popular content that still has life in it and training only has to learn
    where that rule is wrong. This is the placement-side counterpart of the
    keep-value prior in the replacement actor, and the same residual policy
    learning idea (Silver et al. 2018; Johannink et al. 2019)."""

    def __init__(self, state_dim=L.STATE_DIM, hidden=256, n_actions=2,
                 prior_kappa=0.0):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(state_dim, hidden), nn.ReLU(),
            nn.Linear(hidden, hidden), nn.ReLU(),
            nn.Linear(hidden, n_actions),
        )
        # buffers travel inside the checkpoint, so an evaluation run rebuilds
        # exactly the policy that was trained
        self.register_buffer("prior_kappa", torch.tensor(float(prior_kappa)))

    def forward(self, x):
        q = self.net(x)
        if float(self.prior_kappa) == 0.0:
            return q
        prior = x[:, FREQ_SHORT] * x[:, CAND_REMAIN]
        bonus = torch.stack([torch.zeros_like(prior), self.prior_kappa * prior], 1)
        return q + bonus


class DQNPlacementAgent:
    # Defaults below are the values the thesis campaigns actually ran with,
    # not generic textbook values. See
    # docs/thesis/hyperparameter_provenance.md for the bit-exact replay that
    # established them.
    #   gamma = 0.9            -> placement is a discounted decision process
    #                             whose step is one credit horizon: the reward
    #                             is what this decision's own copy earned
    #                             inside its window, and the bootstrap is the
    #                             value of the first decision the same agent
    #                             takes AFTER that window closes
    #                             (MaacoAgents(bootstrap="post-horizon")), so
    #                             reward and bootstrap cover disjoint time.
    #                             The pre-2026 code bootstrapped on the
    #                             immediately following decision, about 0.75 s
    #                             later, whose value still contains the hits
    #                             the reward had already paid for; that
    #                             duplicate term is action-independent and
    #                             40-50x the admit/skip gap, which is why
    #                             gamma had to be 0 then (validation hit
    #                             0.0442 at gamma 0.98 against 0.2021 at
    #                             gamma 0 under that formulation).
    #   eps_decay_steps = 250k -> ~34 episodes to the 0.05 floor at ~7.4k
    #                             placement decisions/episode. The old 50 000
    #                             hit the floor by episode 6.
    # Replacement (A2CReplacementAgent) discounts its own step at 0.98.
    def __init__(self, state_dim=L.STATE_DIM, lr=3e-4, gamma=0.9,
                 buffer_size=50_000, batch_size=64, tau=0.001,
                 eps_start=1.0, eps_end=0.05, eps_decay_steps=250_000,
                 train_every=8, warmup=1_000, seed=0, device="cpu",
                 prior_kappa=0.0):
        torch.manual_seed(seed)
        self.device = torch.device(device)
        self.prior_kappa = float(prior_kappa)
        self.q = QNet(state_dim, prior_kappa=prior_kappa).to(self.device)
        self.q_target = QNet(state_dim, prior_kappa=prior_kappa).to(self.device)
        self.q_target.load_state_dict(self.q.state_dict())
        self.opt = torch.optim.Adam(self.q.parameters(), lr=lr)
        self.replay = ReplayBuffer(buffer_size, state_dim, seed=seed)

        self.gamma = gamma
        self.batch_size = batch_size
        self.tau = tau
        self.eps_start, self.eps_end = eps_start, eps_end
        self.eps_decay_steps = eps_decay_steps
        self.train_every = train_every
        self.warmup = warmup

        self.steps = 0          # decisions taken (drives epsilon)
        self.train_steps = 0
        self.last_loss = float("nan")
        self.rng = np.random.default_rng(seed)
        self.greedy = False     # eval mode

    @property
    def epsilon(self):
        if self.greedy:
            return 0.0
        frac = min(1.0, self.steps / self.eps_decay_steps)
        return self.eps_start + (self.eps_end - self.eps_start) * frac

    wants_mask = True

    def act(self, state: np.ndarray, mask: int = 3) -> int:
        """`mask` is the coordination/energy action mask: bit 0 = "skip" is
        legal, bit 1 = "admit" is legal. The redundancy rule and the energy
        guard used to veto BEFORE this call, so the agent never saw a
        constrained state and never learned from one; they are now expressed
        here, over an action set the policy still selects within."""
        self.steps += 1
        legal = [a for a in (0, 1) if (mask >> a) & 1] or [0, 1]
        if len(legal) == 1:
            return legal[0]
        if not self.greedy and self.rng.random() < self.epsilon:
            return int(legal[self.rng.integers(0, len(legal))])
        with torch.no_grad():
            s = torch.as_tensor(state, dtype=torch.float32,
                                device=self.device).unsqueeze(0)
            q = self.q(s)[0].cpu().numpy()
            return int(max(legal, key=lambda a: q[a]))

    def push_transition(self, s, a, r, s2, done):
        # Frozen-policy evaluation must be FROZEN. Without this guard the
        # eval runs kept filling the replay buffer and calling train_step(),
        # so a "greedy checkpoint evaluation" silently fine-tuned the network
        # on eval-time rewards for thousands of gradient steps and drifted far
        # away from the checkpoint being reported (measured: greedy admission
        # rate collapsing from ~0.5 to ~0.08 within a single 3-seed sweep).
        if self.greedy:
            return
        self.replay.push(s, a, r, s2, done)
        if (len(self.replay) >= self.warmup
                and self.replay.idx % self.train_every == 0):
            self.train_step()

    def train_step(self):
        s, a, r, s2, done = self.replay.sample(self.batch_size)
        s = torch.as_tensor(s, device=self.device)
        a = torch.as_tensor(a, device=self.device)
        r = torch.as_tensor(r, device=self.device)
        s2 = torch.as_tensor(s2, device=self.device)
        done = torch.as_tensor(done, device=self.device)

        q_sa = self.q(s).gather(1, a.unsqueeze(1)).squeeze(1)
        with torch.no_grad():
            # Standard DQN (Mnih et al. 2015): target net both selects and
            # evaluates the next action: y = r + gamma * max_a Q_target(s',a)
            q_next = self.q_target(s2).max(1).values
            target = r + self.gamma * (1.0 - done) * q_next

        loss = nn.functional.smooth_l1_loss(q_sa, target)
        self.opt.zero_grad()
        loss.backward()
        nn.utils.clip_grad_norm_(self.q.parameters(), 10.0)
        self.opt.step()

        # Polyak target update
        with torch.no_grad():
            for p, pt in zip(self.q.parameters(), self.q_target.parameters()):
                pt.mul_(1 - self.tau).add_(self.tau * p)

        self.train_steps += 1
        self.last_loss = float(loss.item())

    def hparams(self):
        """Everything needed to rebuild this agent, for checkpoint provenance."""
        return {"lr": float(self.opt.param_groups[0]["lr"]),
                "gamma": float(self.gamma),
                "buffer_size": int(self.replay.capacity),
                "batch_size": int(self.batch_size),
                "tau": float(self.tau),
                "eps_start": float(self.eps_start),
                "eps_end": float(self.eps_end),
                "eps_decay_steps": int(self.eps_decay_steps),
                "train_every": int(self.train_every),
                "warmup": int(self.warmup),
                "prior_kappa": float(self.prior_kappa)}

    def save(self, path):
        # hparams ride along inside the checkpoint so a .pt found on its own,
        # without the JSON sidecar next to it, still says how it was trained.
        torch.save({"q": self.q.state_dict(), "steps": self.steps,
                    "train_steps": self.train_steps,
                    "hparams": self.hparams()}, path)

    def load(self, path):
        ckpt = torch.load(path, map_location=self.device, weights_only=True)
        sd = ckpt["q"]
        # the checkpoint carries its own prior weight, so rebuild the nets to
        # match it rather than whatever this process was constructed with
        kappa = float(sd["prior_kappa"]) if "prior_kappa" in sd else 0.0
        self.prior_kappa = kappa
        self.q = QNet(prior_kappa=kappa).to(self.device)
        self.q_target = QNet(prior_kappa=kappa).to(self.device)
        self.opt = torch.optim.Adam(self.q.parameters(),
                                    lr=self.opt.param_groups[0]["lr"])
        self.q.load_state_dict(sd, strict=False)
        self.q_target.load_state_dict(sd, strict=False)
        self.steps = ckpt.get("steps", 0)
