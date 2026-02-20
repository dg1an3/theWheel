// test_vectord.cpp - Unit tests for CVectorD template class

#include "stdafx.h"
#include <gtest/gtest.h>
#include "VectorD.h"
#include <cmath>

// ===== Construction =====

TEST(VectorD, DefaultConstructionIsZero)
{
    CVectorD<3> v;
    EXPECT_DOUBLE_EQ(v[0], 0.0);
    EXPECT_DOUBLE_EQ(v[1], 0.0);
    EXPECT_DOUBLE_EQ(v[2], 0.0);
}

TEST(VectorD, ConstructFrom2Args)
{
    CVectorD<3> v(1.0f, 2.0f);
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);
    EXPECT_FLOAT_EQ(v[2], 0.0f);
}

TEST(VectorD, ConstructFrom3Args)
{
    CVectorD<3> v(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);
    EXPECT_FLOAT_EQ(v[2], 3.0f);
}

TEST(VectorD, CopyConstruction)
{
    CVectorD<3> v1(1.0f, 2.0f, 3.0f);
    CVectorD<3> v2(v1);
    EXPECT_FLOAT_EQ(v2[0], 1.0f);
    EXPECT_FLOAT_EQ(v2[1], 2.0f);
    EXPECT_FLOAT_EQ(v2[2], 3.0f);
}

TEST(VectorD, GetDim)
{
    CVectorD<3> v3;
    EXPECT_EQ(v3.GetDim(), 3);

    CVectorD<2> v2;
    EXPECT_EQ(v2.GetDim(), 2);

    CVectorD<4> v4;
    EXPECT_EQ(v4.GetDim(), 4);
}

// ===== Arithmetic =====

TEST(VectorD, Addition)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(4.0f, 5.0f, 6.0f);
    CVectorD<3> c = a + b;
    EXPECT_FLOAT_EQ(c[0], 5.0f);
    EXPECT_FLOAT_EQ(c[1], 7.0f);
    EXPECT_FLOAT_EQ(c[2], 9.0f);
}

TEST(VectorD, Subtraction)
{
    CVectorD<3> a(4.0f, 5.0f, 6.0f);
    CVectorD<3> b(1.0f, 2.0f, 3.0f);
    CVectorD<3> c = a - b;
    EXPECT_FLOAT_EQ(c[0], 3.0f);
    EXPECT_FLOAT_EQ(c[1], 3.0f);
    EXPECT_FLOAT_EQ(c[2], 3.0f);
}

TEST(VectorD, ScalarMultiplyRight)
{
    CVectorD<3> v(1.0f, 2.0f, 3.0f);
    CVectorD<3> r = v * 2.0f;
    EXPECT_FLOAT_EQ(r[0], 2.0f);
    EXPECT_FLOAT_EQ(r[1], 4.0f);
    EXPECT_FLOAT_EQ(r[2], 6.0f);
}

TEST(VectorD, ScalarMultiplyLeft)
{
    CVectorD<3> v(1.0f, 2.0f, 3.0f);
    CVectorD<3> r = 2.0f * v;
    EXPECT_FLOAT_EQ(r[0], 2.0f);
    EXPECT_FLOAT_EQ(r[1], 4.0f);
    EXPECT_FLOAT_EQ(r[2], 6.0f);
}

TEST(VectorD, PlusEquals)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(4.0f, 5.0f, 6.0f);
    a += b;
    EXPECT_FLOAT_EQ(a[0], 5.0f);
    EXPECT_FLOAT_EQ(a[1], 7.0f);
    EXPECT_FLOAT_EQ(a[2], 9.0f);
}

TEST(VectorD, MinusEquals)
{
    CVectorD<3> a(4.0f, 5.0f, 6.0f);
    CVectorD<3> b(1.0f, 2.0f, 3.0f);
    a -= b;
    EXPECT_FLOAT_EQ(a[0], 3.0f);
    EXPECT_FLOAT_EQ(a[1], 3.0f);
    EXPECT_FLOAT_EQ(a[2], 3.0f);
}

TEST(VectorD, TimesEquals)
{
    CVectorD<3> v(1.0f, 2.0f, 3.0f);
    v *= 3.0f;
    EXPECT_FLOAT_EQ(v[0], 3.0f);
    EXPECT_FLOAT_EQ(v[1], 6.0f);
    EXPECT_FLOAT_EQ(v[2], 9.0f);
}

// ===== Dot Product =====

TEST(VectorD, DotProductOrthogonal)
{
    CVectorD<3> a(1.0f, 0.0f, 0.0f);
    CVectorD<3> b(0.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(a * b, 0.0f);
}

TEST(VectorD, DotProductParallel)
{
    CVectorD<3> a(1.0f, 0.0f, 0.0f);
    CVectorD<3> b(3.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(a * b, 3.0f);
}

TEST(VectorD, DotProductGeneral)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(4.0f, 5.0f, 6.0f);
    // 1*4 + 2*5 + 3*6 = 32
    EXPECT_FLOAT_EQ(a * b, 32.0f);
}

// ===== Length =====

TEST(VectorD, LengthKnown345)
{
    CVectorD<3> v(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.GetLength(), 5.0f);
}

TEST(VectorD, LengthUnit)
{
    CVectorD<3> v(1.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.GetLength(), 1.0f);
}

TEST(VectorD, LengthZero)
{
    CVectorD<3> v;
    EXPECT_FLOAT_EQ(v.GetLength(), 0.0f);
}

// ===== Normalize =====

TEST(VectorD, NormalizeNonZero)
{
    CVectorD<3> v(3.0f, 4.0f, 0.0f);
    v.Normalize();
    EXPECT_NEAR(v.GetLength(), 1.0f, 1e-5f);
    EXPECT_NEAR(v[0], 0.6f, 1e-5f);
    EXPECT_NEAR(v[1], 0.8f, 1e-5f);
}

TEST(VectorD, NormalizeZeroVectorSafe)
{
    CVectorD<3> v;
    v.Normalize(); // should not crash or produce NaN
    EXPECT_FLOAT_EQ(v[0], 0.0f);
    EXPECT_FLOAT_EQ(v[1], 0.0f);
    EXPECT_FLOAT_EQ(v[2], 0.0f);
}

// ===== Cross Product =====

TEST(VectorD, CrossProductBasisVectors)
{
    CVectorD<3> x(1.0f, 0.0f, 0.0f);
    CVectorD<3> y(0.0f, 1.0f, 0.0f);
    CVectorD<3> z = Cross(x, y);
    EXPECT_FLOAT_EQ(z[0], 0.0f);
    EXPECT_FLOAT_EQ(z[1], 0.0f);
    EXPECT_FLOAT_EQ(z[2], 1.0f);
}

TEST(VectorD, CrossProductAntiCommutativity)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(4.0f, 5.0f, 6.0f);
    CVectorD<3> ab = Cross(a, b);
    CVectorD<3> ba = Cross(b, a);
    EXPECT_FLOAT_EQ(ab[0], -ba[0]);
    EXPECT_FLOAT_EQ(ab[1], -ba[1]);
    EXPECT_FLOAT_EQ(ab[2], -ba[2]);
}

TEST(VectorD, CrossProductParallelIsZero)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(2.0f, 4.0f, 6.0f);
    CVectorD<3> c = Cross(a, b);
    EXPECT_FLOAT_EQ(c[0], 0.0f);
    EXPECT_FLOAT_EQ(c[1], 0.0f);
    EXPECT_FLOAT_EQ(c[2], 0.0f);
}

// ===== Equality =====

TEST(VectorD, EqualityOperator)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(a == b);
}

TEST(VectorD, InequalityOperator)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(1.0f, 2.0f, 4.0f);
    EXPECT_TRUE(a != b);
}

TEST(VectorD, IsApproxEqualClose)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(1.0f + 1e-7f, 2.0f, 3.0f);
    EXPECT_TRUE(a.IsApproxEqual(b));
}

TEST(VectorD, IsApproxEqualFar)
{
    CVectorD<3> a(1.0f, 2.0f, 3.0f);
    CVectorD<3> b(2.0f, 2.0f, 3.0f);
    EXPECT_FALSE(a.IsApproxEqual(b));
}
