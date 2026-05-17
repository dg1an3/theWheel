"""
Constants ported from the C++ implementation.

Names and values mirror the originals so cross-references stay obvious:
- Node.cpp:          PRIM_FRAC, PROPAGATE_THRESHOLD_WEIGHT
- NodeLink.cpp:      PROPAGATE_THRESHOLD_WEIGHT (same value)
- Space.cpp:         PRIM_NORM_SCALE, SEC_NORM_SCALE, PROPAGATE_SCALE,
                     PROPAGATE_ALPHA, TOTAL_ACTIVATION_FRACTION,
                     DEFAULT_SPRING_CONST
- Space.h:           TOTAL_ACTIVATION
- SpaceLayoutManager.cpp:
                     K_POS, K_REP, SIZE_SCALE, DIST_SCALE_MIN/MAX,
                     ACTIVATION_MIDPOINT, MIDWEIGHT, OPT_DIST
"""

# --- Activation -------------------------------------------------------------

# Fraction of an explicit SetActivation delta that goes to primary activation;
# remainder goes to secondary. Both Node.cpp and Space.cpp use 0.5.
PRIM_FRAC = 0.5

# Links with weight at or below this threshold do not propagate.
PROPAGATE_THRESHOLD_WEIGHT = 0.01

# Initial scale and per-recursion attenuation applied during spreading
# activation. The C++ uses these in CSpace::ActivateNode.
PROPAGATE_SCALE = 0.40
PROPAGATE_ALPHA = 0.99

# Target sum for NormalizeNodes — equals TOTAL_ACTIVATION * TOTAL_ACTIVATION_FRACTION
# multiplied through the activation curve. The activation cap in ActivateNode
# is also TOTAL_ACTIVATION * TOTAL_ACTIVATION_FRACTION.
TOTAL_ACTIVATION = 0.75
TOTAL_ACTIVATION_FRACTION = 0.40

# NormalizeNodes scales each node's primary/secondary by these multipliers
# times the normalization delta. Values match Space.cpp (0.020*3.5, 0.060*3.5).
PRIM_NORM_SCALE = 0.020 * 3.5
SEC_NORM_SCALE = 0.060 * 3.5

# Stabilizer links attenuate target activation by this factor and drop the
# source node identity (so PropagateActivation does not chain from the source).
STABILIZER_ATTENUATION = 0.25

# Default sum NormalizeNodes targets when no explicit value is supplied.
DEFAULT_NORMALIZE_SUM = 1.0


# --- Layout -----------------------------------------------------------------

# Multiplier on each node's radius before adding the floor of 10.0.
SIZE_SCALE = 150.0

# Optimal normalized distance between linked nodes (the dist_error target).
# Derivation lives in SpaceLayoutManager.cpp; reproduced here so the
# constants stay in one place.
DIST_SCALE_MIN = 1.0
DIST_SCALE_MAX = 1.35
ACTIVATION_MIDPOINT = 0.25
MIDWEIGHT = 0.5
OPT_DIST = DIST_SCALE_MIN + (DIST_SCALE_MAX - DIST_SCALE_MIN) * (
    1.0 - MIDWEIGHT / (MIDWEIGHT + ACTIVATION_MIDPOINT)
)

# Attraction and repulsion weights in the layout energy.
K_POS = 600.0
K_REP = 3200.0

# Epsilons embedded in the energy expression (kept here so tests can poke them).
ENERGY_DIST_EPS = 0.001   # added inside sqrt for act_dist
ENERGY_REP_EPS = 3.0      # added to denominator of repulsion term
LINK_WEIGHT_EPS = 1.0e-6  # added to the per-pair link weight
SIZE_FLOOR = 10.0         # added to size after SIZE_SCALE * radius
