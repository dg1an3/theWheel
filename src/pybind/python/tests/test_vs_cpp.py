"""
Cross-language validation of pythewheel_torch against the C++ pythewheel
bindings (the oracle).

The C++ module is built into <build>/python/pythewheel.cpython-*.so. The
test harness prepends that directory to sys.path so the tests run from a
plain pytest invocation.
"""

from __future__ import annotations

import os
import sys
from pathlib import Path

import pytest
import torch

# --- locate the C++ pythewheel module ---------------------------------------

_REPO_ROOT = Path(__file__).resolve().parents[4]
_PYTHON_DIRS = [
    _REPO_ROOT / "build" / "linux-debug" / "python",
    _REPO_ROOT / "build" / "macos-debug" / "python",
    _REPO_ROOT / "build" / "x64-debug" / "python",
]
for _d in _PYTHON_DIRS:
    if _d.is_dir():
        sys.path.insert(0, str(_d))

# Also let tests import the sibling pythewheel_torch package without an
# install step.
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

try:
    import pythewheel as pw  # noqa: E402  (the C++ oracle)
except ImportError:
    pw = None

import pythewheel_torch as pwt  # noqa: E402


pytestmark = pytest.mark.skipif(
    pw is None,
    reason="C++ pythewheel module not built; run `cmake --build ... --target pythewheel` first",
)


# ----------------------------------------------------------------- fixtures


def build_simple_cpp_space():
    """A small directed-link space with a known shape.

    Hierarchy: hidden_root -> {a, b, c, d}
    Links: a<->b (w=0.5), a<->c (w=0.3), b<->d (w=0.4), c<->d (w=0.2)
    All bidirectional; a starts at 0.4 activation, others at 0.1.
    """
    s = pw.Space()
    nodes = []
    for name in ("a", "b", "c", "d"):
        n = pw.Node(s, name, "")
        s.add_node(n)
        nodes.append(n)
    a, b, c, d = nodes

    # Reset activations to known starting values. SetActivation goes through
    # the primary/secondary split, so use it directly.
    for n in nodes:
        n.set_activation(0.1)
    a.set_activation(0.4)

    a.link_to(b, 0.5, True)
    a.link_to(c, 0.3, True)
    b.link_to(d, 0.4, True)
    c.link_to(d, 0.2, True)

    return s, nodes


def snapshot_cpp(space):
    """Return (primary, secondary, total) tensors over space's nodes."""
    n = space.get_node_count()
    prim = torch.zeros(n, dtype=torch.float32)
    sec = torch.zeros(n, dtype=torch.float32)
    for i in range(n):
        nd = space.get_node_at(i)
        prim[i] = nd.get_primary_activation()
        sec[i] = nd.get_secondary_activation()
    return prim, sec, prim + sec


# ------------------------------------------------------------- activation ---


def test_activation_matches_cpp():
    """One ActivateNode cycle in PyTorch matches the C++ oracle."""
    cpp_space, cpp_nodes = build_simple_cpp_space()
    torch_space = pwt.SpaceTensors.from_pythewheel(cpp_space)

    # Sanity: snapshots agree before the activation step.
    prim0, sec0, _ = snapshot_cpp(cpp_space)
    assert torch.allclose(torch_space.primary_activation, prim0, atol=1e-6)
    assert torch.allclose(torch_space.secondary_activation, sec0, atol=1e-6)

    # Run ActivateNode on index 0 (= 'a') with scale=0.5 in both worlds.
    cpp_space.activate_node(cpp_nodes[0], 0.5)
    module = pwt.ActivationModule()
    module(torch_space, node_index=0, scale=0.5)

    cpp_prim, cpp_sec, cpp_total = snapshot_cpp(cpp_space)

    # Bit-for-bit match isn't realistic (float ordering differs slightly
    # between the C++ inline math and the per-step Python tensor ops), but
    # the values should agree to ~1e-5 absolute.
    assert torch.allclose(
        torch_space.primary_activation, cpp_prim, atol=1e-5
    ), (torch_space.primary_activation, cpp_prim)
    assert torch.allclose(
        torch_space.secondary_activation, cpp_sec, atol=1e-5
    ), (torch_space.secondary_activation, cpp_sec)
    assert torch.allclose(
        torch_space.activation(), cpp_total, atol=1e-5
    )


def test_activation_repeated_cycles():
    """Multiple sequential ActivateNode cycles still match."""
    cpp_space, cpp_nodes = build_simple_cpp_space()
    torch_space = pwt.SpaceTensors.from_pythewheel(cpp_space)
    module = pwt.ActivationModule()

    for target_idx, scale in [(0, 0.5), (3, 0.3), (1, 0.2), (2, 0.4)]:
        cpp_space.activate_node(cpp_nodes[target_idx], scale)
        module(torch_space, node_index=target_idx, scale=scale)

    cpp_prim, cpp_sec, cpp_total = snapshot_cpp(cpp_space)
    assert torch.allclose(torch_space.primary_activation, cpp_prim, atol=1e-4)
    assert torch.allclose(torch_space.secondary_activation, cpp_sec, atol=1e-4)
    assert torch.allclose(torch_space.activation(), cpp_total, atol=1e-4)


# ----------------------------------------------------------------- layout ---


def test_layout_energy_finite_and_decreases():
    """LBFGS reduces the layout energy from a perturbed initial state."""
    cpp_space, _ = build_simple_cpp_space()
    # Give the nodes nontrivial positions so the energy isn't trivially zero.
    # NOTE: pythewheel.Node.get_position returns a *copy*, so we must build
    # a fresh Vector3D and pass it to set_position.
    for i in range(cpp_space.get_node_count()):
        nd = cpp_space.get_node_at(i)
        nd.set_position(pw.Vector3D(50.0 * (i - 1.5), 30.0 * ((i % 2) * 2 - 1), 0.0))

    torch_space = pwt.SpaceTensors.from_pythewheel(cpp_space)
    layout = pwt.LayoutModule(torch_space)

    e0 = layout.forward().item()
    assert torch.isfinite(torch.tensor(e0)), f"initial energy not finite: {e0}"

    e_final = layout.optimize(n_steps=50)
    assert e_final < e0, f"layout failed to reduce energy: {e0} -> {e_final}"


def test_layout_gradient_flows_to_positions():
    """positions should be a leaf with a gradient after a backward pass."""
    cpp_space, _ = build_simple_cpp_space()
    # Give the nodes nontrivial positions; (0,0)-everywhere collapses all
    # pairwise displacements to zero and yields a zero gradient.
    for i in range(cpp_space.get_node_count()):
        nd = cpp_space.get_node_at(i)
        nd.set_position(pw.Vector3D(50.0 * (i - 1.5), 30.0 * ((i % 2) * 2 - 1), 0.0))
    torch_space = pwt.SpaceTensors.from_pythewheel(cpp_space)
    layout = pwt.LayoutModule(torch_space)

    energy = layout()
    energy.backward()

    assert layout.positions.grad is not None
    assert layout.positions.grad.shape == layout.positions.shape
    # Some nodes carry nonzero gradient — otherwise the optimizer can't move.
    assert layout.positions.grad.abs().sum().item() > 0


def test_pairwise_link_weight_matches_loadsizeslinks():
    """The pairwise_link_weights helper reproduces the C++ LoadSizesLinks formula."""
    cpp_space, _ = build_simple_cpp_space()
    torch_space = pwt.SpaceTensors.from_pythewheel(cpp_space)

    from pythewheel_torch.layout import _pairwise_gain_matrix

    gain = _pairwise_gain_matrix(torch_space)
    avg, weight = pwt.pairwise_link_weights(torch_space.activation(), gain)

    # Spot-check (a, b): a_i, a_j are a's and b's totals; gain_ij and gain_ji
    # come from the link's GetGain. The formula is symmetric in (i,j).
    a_idx, b_idx = 0, 1
    a_i = float(torch_space.activation()[a_idx].item())
    a_j = float(torch_space.activation()[b_idx].item())
    g_ij = float(gain[a_idx, b_idx].item())
    g_ji = float(gain[b_idx, a_idx].item())
    expected_avg = a_i + a_j
    expected_w = (
        a_i / expected_avg * g_ij + a_j / expected_avg * g_ji + pwt.constants.LINK_WEIGHT_EPS
    ) * expected_avg

    assert avg[a_idx, b_idx].item() == pytest.approx(expected_avg, abs=1e-6)
    assert weight[a_idx, b_idx].item() == pytest.approx(expected_w, abs=1e-6)
