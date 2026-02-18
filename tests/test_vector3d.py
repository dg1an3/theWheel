"""
Unit tests for Vector3D class
Tests the CVectorD<3> Python bindings
"""
import pytest
import sys
import os

# Add the build directory to Python path to import pythewheel
# You may need to adjust this path based on your build configuration
build_dir = os.path.join(os.path.dirname(__file__), '..', 'src', 'out', 'build', 'x64-debug', 'python')
if os.path.exists(build_dir):
    sys.path.insert(0, build_dir)

import pythewheel


@pytest.mark.unit
class TestVector3D:
    """Test cases for Vector3D class"""

    def test_default_construction(self):
        """Test default constructor creates zero vector"""
        v = pythewheel.Vector3D()
        assert v[0] == 0.0
        assert v[1] == 0.0
        assert v[2] == 0.0

    def test_parameterized_construction(self):
        """Test constructor with x, y, z parameters"""
        v = pythewheel.Vector3D(1.0, 2.0, 3.0)
        assert v[0] == 1.0
        assert v[1] == 2.0
        assert v[2] == 3.0

    def test_indexing(self):
        """Test element access via indexing"""
        v = pythewheel.Vector3D(1.0, 2.0, 3.0)
        assert v[0] == 1.0
        assert v[1] == 2.0
        assert v[2] == 3.0

    def test_set_elements(self):
        """Test setting individual elements"""
        v = pythewheel.Vector3D()
        v[0] = 4.0
        v[1] = 5.0
        v[2] = 6.0
        assert v[0] == 4.0
        assert v[1] == 5.0
        assert v[2] == 6.0

    def test_index_out_of_bounds(self):
        """Test that out-of-bounds indexing raises error"""
        v = pythewheel.Vector3D(1.0, 2.0, 3.0)
        with pytest.raises(IndexError):
            _ = v[3]
        with pytest.raises(IndexError):
            _ = v[-1]

    def test_length(self):
        """Test vector length calculation"""
        v = pythewheel.Vector3D(3.0, 4.0, 0.0)
        assert abs(v.get_length() - 5.0) < 1e-6

    def test_normalize(self):
        """Test vector normalization"""
        v = pythewheel.Vector3D(3.0, 4.0, 0.0)
        v.normalize()
        assert abs(v.get_length() - 1.0) < 1e-6
        assert abs(v[0] - 0.6) < 1e-6
        assert abs(v[1] - 0.8) < 1e-6

    def test_addition(self):
        """Test vector addition"""
        v1 = pythewheel.Vector3D(1.0, 2.0, 3.0)
        v2 = pythewheel.Vector3D(4.0, 5.0, 6.0)
        v3 = v1 + v2
        assert v3[0] == 5.0
        assert v3[1] == 7.0
        assert v3[2] == 9.0

    def test_subtraction(self):
        """Test vector subtraction"""
        v1 = pythewheel.Vector3D(4.0, 5.0, 6.0)
        v2 = pythewheel.Vector3D(1.0, 2.0, 3.0)
        v3 = v1 - v2
        assert v3[0] == 3.0
        assert v3[1] == 3.0
        assert v3[2] == 3.0

    def test_scalar_multiplication(self):
        """Test vector scalar multiplication"""
        v1 = pythewheel.Vector3D(1.0, 2.0, 3.0)
        v2 = v1 * 2.0
        assert v2[0] == 2.0
        assert v2[1] == 4.0
        assert v2[2] == 6.0

    def test_repr(self):
        """Test string representation"""
        v = pythewheel.Vector3D(1.0, 2.0, 3.0)
        s = repr(v)
        assert "Vector3D" in s
        assert "1.0" in s or "1.00" in s
