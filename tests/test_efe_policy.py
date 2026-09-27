"""
Tests for the expected-free-energy display policies (scripts/efe_policy.py).
Pure Python + numpy; does not need the pythewheel module.
"""
import os
import sys

import numpy as np
import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'scripts'))

from efe_policy import (  # noqa: E402
    ClickModel, compare, load_space, relatedness, select_efe,
    select_pragmatic, update_belief,
)

THEWELL = os.path.join(os.path.dirname(__file__), '..',
                       'antiques', 'theWheel', 'data', 'TheWell3.spx')


def entropy(p):
    return -(p * np.log(np.maximum(p, 1e-300))).sum(axis=-1)


@pytest.fixture
def chain_model():
    """Five nodes linked in a chain 0-1-2-3-4, plus one isolated node."""
    weights = np.zeros((6, 6))
    for i in range(4):
        weights[i, i + 1] = weights[i + 1, i] = 0.6
    return ClickModel(relatedness(weights))


@pytest.mark.unit
class TestRelatedness:
    def test_strongest_path_product(self):
        weights = np.zeros((3, 3))
        weights[0, 1] = weights[1, 0] = 0.5
        weights[1, 2] = weights[2, 1] = 0.4
        related = relatedness(weights)
        assert related[0, 2] == pytest.approx(0.2)
        assert np.allclose(np.diag(related), 1.0)

    def test_disconnected_is_zero(self, chain_model):
        assert chain_model.related[0, 5] == 0.0


@pytest.mark.unit
class TestClickModel:
    def test_likelihood_rows_sum_to_one(self, chain_model):
        likelihood = chain_model.likelihood([0, 2, 4])
        assert likelihood.shape == (6, 4)
        assert np.allclose(likelihood.sum(axis=1), 1.0)

    def test_user_most_likely_clicks_shown_target(self, chain_model):
        likelihood = chain_model.likelihood([0, 2, 4])
        assert np.argmax(likelihood[2]) == 1

    def test_scores_match_brute_force(self, chain_model):
        rng = np.random.default_rng(0)
        q = rng.dirichlet(np.ones(6))
        shown = [1, 3]
        candidates = np.array([0, 2, 5])
        scores = chain_model.score_additions(q, shown, candidates, gamma=1.5)
        for j, c in enumerate(candidates):
            display = shown + [int(c)]
            likelihood = chain_model.likelihood(display)
            info_gain = entropy(q @ likelihood) - q @ entropy(likelihood)
            p_hit = sum(q[t] * likelihood[t, display.index(t)] for t in display)
            assert scores.info_gain[j] == pytest.approx(info_gain)
            assert scores.p_hit[j] == pytest.approx(p_hit)
            assert scores.efe[j] == pytest.approx(-info_gain - 1.5 * p_hit)

    def test_scores_for_empty_display(self, chain_model):
        q = np.full(6, 1 / 6)
        scores = chain_model.score_additions(q, [], np.array([2]))
        likelihood = chain_model.likelihood([2])
        info_gain = entropy(q @ likelihood) - q @ entropy(likelihood)
        assert scores.info_gain[0] == pytest.approx(info_gain)


@pytest.mark.unit
class TestBeliefAndPolicies:
    def test_click_concentrates_belief_near_clicked_node(self, chain_model):
        q = np.full(6, 1 / 6)
        posterior = update_belief(q, chain_model, [0, 3], outcome=1)
        assert posterior.sum() == pytest.approx(1.0)
        assert np.argmax(posterior) == 3
        assert posterior[5] < q[5]

    def test_no_click_lowers_belief_in_shown_nodes(self, chain_model):
        q = np.full(6, 1 / 6)
        posterior = update_belief(q, chain_model, [0, 3], outcome=2)
        assert posterior[0] < q[0]
        assert posterior[3] < q[3]

    def test_policies_return_k_distinct_nodes(self, chain_model):
        rng = np.random.default_rng(0)
        q = rng.dirichlet(np.ones(6))
        for policy in (select_pragmatic, select_efe):
            shown = policy(q, chain_model, 3, rng)
            assert len(shown) == 3
            assert len(set(shown)) == 3

    def test_efe_avoids_redundant_nodes(self):
        # two identical twins and a distinct node; with two slots, showing
        # both twins tells us less than a twin plus the distinct node
        weights = np.zeros((3, 3))
        weights[0, 1] = weights[1, 0] = 1.0
        model = ClickModel(relatedness(weights))
        q = np.array([0.35, 0.35, 0.3])
        shown = select_efe(q, model, 2, np.random.default_rng(0), gamma=0.0)
        assert 2 in shown


@pytest.mark.slow
class TestSimulation:
    def test_load_thewell(self):
        names, weights = load_space(THEWELL)
        assert len(names) == 90
        assert np.allclose(weights, weights.T)

    def test_efe_finds_targets_faster_than_pragmatic(self):
        names, weights = load_space(THEWELL)
        model = ClickModel(relatedness(weights))
        results = compare(model, k=4, trials=100, max_steps=20, seed=0)
        assert results['efe'][1] < results['pragmatic'][1]
