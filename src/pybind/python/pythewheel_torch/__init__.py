"""
pythewheel_torch: PyTorch port of theWheel's activation and layout.

The activation port is a literal transcription of CSpace::ActivateNode
(see activation.py). The layout port is vectorized but produces the
same scalar energy as CSpaceLayoutManager::operator() (see layout.py).

Both are intended to be validated against the existing C++ pythewheel
bindings — see ../tests/test_vs_cpp.py.
"""

from . import _constants as constants
from .activation import ActivationModule
from .layout import LayoutModule, pairwise_link_weights
from .space import SpaceTensors

__all__ = [
    "ActivationModule",
    "LayoutModule",
    "SpaceTensors",
    "constants",
    "pairwise_link_weights",
]
