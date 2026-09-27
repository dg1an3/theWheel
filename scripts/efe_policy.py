"""
Expected-free-energy display policies for theWheel (prototype).

Casts browsing as active inference:

- hidden state s: the node the user is looking for
- belief q(s): categorical distribution over nodes
- policy pi: the set of K nodes to display
- outcome o: which displayed node the user clicks, or no click

The user is modeled as clicking displayed node i with probability
softmax(beta * relatedness(i, s)), competing against a "no click" option
with score beta * tau.  Relatedness is the strongest path between two
nodes (maximum product of link weights), so clicks lead toward the
target through the link graph.

The expected free energy of a policy is

    G(pi) = -I(s; o | pi) - gamma * P(hit | pi)

where I is the mutual information between target and click (epistemic
value) and P(hit) is the probability that the user clicks their target
(pragmatic value; gamma is the preference for finding it).  Because I is
not additive over the displayed nodes, policies are built greedily one
node at a time, scoring each candidate set as a whole.

Usage:
    python scripts/efe_policy.py antiques/theWheel/data/SonicSpace.spx
    python scripts/efe_policy.py <file.spx> --k 8 --trials 100
"""

import argparse
import sys
from dataclasses import dataclass
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
from spx_parser import parse_spx  # noqa: E402


# default model parameters
BETA = 6.0          # how sharply the user clicks related nodes
TAU = 0.3           # relatedness a displayed node needs to beat "no click"
GAMMA = 1.0         # preference for the user finding their target (nats)


def load_space(spx_path):
    """Returns (names, weights) for an .spx file; weights is a symmetric
    matrix of the strongest link between each pair of nodes."""
    nodes = []

    def walk(node):
        nodes.append(node)
        for child in node.children:
            walk(child)

    walk(parse_spx(spx_path))
    nodes = [n for n in nodes if n.name != "%%hiddenroot%%"]
    index = {id(n): i for i, n in enumerate(nodes)}

    weights = np.zeros((len(nodes), len(nodes)))
    for i, node in enumerate(nodes):
        for link in node.links:
            j = index.get(id(link.target))
            if j is not None and j != i:
                weights[i, j] = max(weights[i, j], link.weight)
    weights = np.maximum(weights, weights.T)
    return [n.name for n in nodes], weights


def relatedness(weights):
    """Strongest-path relatedness: the maximum over paths of the product
    of link weights (1 on the diagonal, 0 if disconnected)."""
    related = np.clip(weights, 0.0, 1.0).copy()
    np.fill_diagonal(related, 1.0)
    for k in range(len(related)):
        related = np.maximum(related, np.outer(related[:, k], related[k, :]))
    return related


@dataclass
class PolicyScores:
    """Scores for each candidate node added to the current display set."""
    candidates: np.ndarray
    info_gain: np.ndarray      # I(s; o | set + candidate), nats
    p_hit: np.ndarray          # P(user clicks target | set + candidate)
    efe: np.ndarray            # G = -info_gain - gamma * p_hit


class ClickModel:
    """Likelihood of clicks given the target and the displayed set."""

    def __init__(self, related, beta=BETA, tau=TAU):
        self.related = related
        self.beta = beta
        self.tau = tau
        self.logits = beta * related              # [target, node]
        self.exp = np.exp(self.logits)
        self.none_logit = beta * tau
        self.none_exp = np.exp(self.none_logit)

    @property
    def size(self):
        return len(self.related)

    def likelihood(self, shown):
        """A[s, o]: probability of outcome o (each shown node, then no
        click) for each target s."""
        shown = list(shown)
        e = np.concatenate([self.exp[:, shown],
                            np.full((self.size, 1), self.none_exp)], axis=1)
        return e / e.sum(axis=1, keepdims=True)

    def score_additions(self, q, shown, candidates, gamma=GAMMA):
        """Scores adding each candidate to the displayed set, vectorized
        over candidates.  Returns PolicyScores."""
        shown = list(shown)
        candidates = np.asarray(candidates)

        # partition function per (target, candidate)
        e_base = self.exp[:, shown]                          # N x k
        e_cand = self.exp[:, candidates]                     # N x C
        z = e_base.sum(axis=1, keepdims=True) + e_cand + self.none_exp

        # sum over outcomes of e * logit, for the conditional entropy
        el_base = (e_base * self.logits[:, shown]).sum(axis=1, keepdims=True)
        el = el_base + e_cand * self.logits[:, candidates] \
            + self.none_exp * self.none_logit

        # E_q H[P(o | s)], with H = ln Z - sum(e * logit) / Z
        cond_entropy = q @ (np.log(z) - el / z)

        # predicted outcome distribution Q(o) for each candidate set
        inv_z = 1.0 / z
        q_base = (q[:, None] * e_base).T @ inv_z             # k x C
        q_cand = (q[:, None] * e_cand * inv_z).sum(axis=0)   # C
        q_none = (q[:, None] * self.none_exp * inv_z).sum(axis=0)
        outcomes = np.vstack([q_base, q_cand, q_none])       # (k+2) x C
        outcome_entropy = -(outcomes * np.log(np.maximum(outcomes, 1e-300))).sum(axis=0)

        info_gain = outcome_entropy - cond_entropy

        # probability the user clicks their target
        e_self = np.exp(self.beta)
        p_hit_base = (q[shown, None] * e_self * inv_z[shown, :]).sum(axis=0) \
            if shown else np.zeros(len(candidates))
        p_hit_cand = q[candidates] * e_self * inv_z[candidates, np.arange(len(candidates))]
        p_hit = p_hit_base + p_hit_cand

        return PolicyScores(candidates, info_gain, p_hit,
                            -info_gain - gamma * p_hit)


def update_belief(q, model, shown, outcome):
    """Bayesian update of the target belief after observing outcome
    (index into shown, or len(shown) for no click)."""
    posterior = q * model.likelihood(shown)[:, outcome]
    return posterior / posterior.sum()


#####################################################################
# policies: each returns the list of K node indices to display

def select_pragmatic(q, model, k, rng):
    """Top K by belief (activation alone); ties broken at random."""
    order = np.lexsort((rng.random(len(q)), -q))
    return list(order[:k])


def select_additive(q, model, k, rng, weight=0.05):
    """Top K by belief + weight * single-node information gain: the
    per-node sort used in the C++ model (CNode::GetSortValue)."""
    single = model.score_additions(q, [], np.arange(len(q)))
    order = np.lexsort((rng.random(len(q)), -(q + weight * single.info_gain)))
    return list(order[:k])


def select_efe(q, model, k, rng, gamma=GAMMA):
    """Greedy minimization of the expected free energy of the whole
    displayed set."""
    shown = []
    remaining = np.arange(len(q))
    for _ in range(k):
        scores = model.score_additions(q, shown, remaining, gamma)
        # random tie-break among equal scores
        best = np.lexsort((rng.random(len(remaining)), scores.efe))[0]
        shown.append(int(remaining[best]))
        remaining = np.delete(remaining, best)
    return shown


POLICIES = {
    "pragmatic": select_pragmatic,
    "additive": select_additive,
    "efe": select_efe,
}


#####################################################################
# simulated browsing

def simulate(model, policy, target, k, max_steps, rng, user=None):
    """Simulates a user looking for target; returns the number of steps
    until they click it, or None if not found within max_steps.  The
    user clicks according to the user ClickModel (default: the same
    model the policy uses)."""
    user = user or model
    q = np.full(model.size, 1.0 / model.size)
    for step in range(1, max_steps + 1):
        shown = policy(q, model, k, rng)
        outcome = rng.choice(len(shown) + 1, p=user.likelihood(shown)[target])
        if outcome < len(shown) and shown[outcome] == target:
            return step
        q = update_belief(q, model, shown, outcome)
    return None


def compare(model, k, trials, max_steps, seed, user=None):
    """Runs every policy on the same random targets; returns
    {policy: (success rate, mean steps with misses counted as
    max_steps)}."""
    rng = np.random.default_rng(seed)
    targets = rng.integers(model.size, size=trials)
    results = {}
    for name, policy in POLICIES.items():
        policy_rng = np.random.default_rng(seed + 1)
        steps = [simulate(model, policy, t, k, max_steps, policy_rng, user)
                 for t in targets]
        found = [s for s in steps if s is not None]
        results[name] = (len(found) / trials,
                         float(np.mean([s or max_steps for s in steps])))
    return results


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("spx", help=".spx file to simulate browsing on")
    parser.add_argument("--k", type=int, default=8, help="nodes displayed")
    parser.add_argument("--trials", type=int, default=100)
    parser.add_argument("--max-steps", type=int, default=20)
    parser.add_argument("--seed", type=int, default=0)
    parser.add_argument("--user-beta", type=float, default=BETA,
                        help="click sharpness of the simulated user")
    args = parser.parse_args(argv)

    names, weights = load_space(args.spx)
    related = relatedness(weights)
    model = ClickModel(related)
    user = ClickModel(related, beta=args.user_beta)
    print(f"{args.spx}: {len(names)} nodes, K={args.k}, "
          f"{args.trials} targets, up to {args.max_steps} steps, "
          f"user beta {args.user_beta:g}")
    print(f"{'policy':<10} {'found':>6} {'mean steps':>11}")
    for name, (rate, steps) in compare(model, args.k, args.trials,
                                       args.max_steps, args.seed,
                                       user).items():
        print(f"{name:<10} {rate:>6.0%} {steps:>11.2f}")


if __name__ == "__main__":
    main()
