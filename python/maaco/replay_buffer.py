"""Uniform experience replay for the DQN placement agent."""

import numpy as np


class ReplayBuffer:
    def __init__(self, capacity: int, state_dim: int, seed: int = 0):
        self.capacity = capacity
        self.state = np.zeros((capacity, state_dim), dtype=np.float32)
        self.action = np.zeros(capacity, dtype=np.int64)
        self.reward = np.zeros(capacity, dtype=np.float32)
        self.next_state = np.zeros((capacity, state_dim), dtype=np.float32)
        self.done = np.zeros(capacity, dtype=np.float32)
        self.idx = 0
        self.full = False
        self.rng = np.random.default_rng(seed)

    def __len__(self):
        return self.capacity if self.full else self.idx

    def push(self, s, a, r, s2, done):
        i = self.idx
        self.state[i] = s
        self.action[i] = a
        self.reward[i] = r
        self.next_state[i] = s2
        self.done[i] = float(done)
        self.idx = (self.idx + 1) % self.capacity
        if self.idx == 0:
            self.full = True

    def sample(self, batch_size: int):
        n = len(self)
        idx = self.rng.integers(0, n, size=batch_size)
        return (self.state[idx], self.action[idx], self.reward[idx],
                self.next_state[idx], self.done[idx])
