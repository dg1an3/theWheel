// test_nodelink.cpp - Unit tests for CNodeLink class

#include "stdafx.h"
#include <gtest/gtest.h>
#include "Node.h"
#include "NodeLink.h"

TEST(NodeLink, ConstructionWithTargetAndWeight)
{
    CNode target;
    CNodeLink link(&target, 0.5f);
    EXPECT_EQ(link.GetTarget(), &target);
    EXPECT_FLOAT_EQ(link.GetWeight(), 0.5f);
}

TEST(NodeLink, DefaultGainIsOne)
{
    CNode target;
    CNodeLink link(&target, 0.5f);
    EXPECT_FLOAT_EQ(link.GetGain(), 1.0f);
}

TEST(NodeLink, SetGetGainRoundTrip)
{
    CNode target;
    CNodeLink link(&target, 0.5f);
    link.SetGain(0.75f);
    EXPECT_FLOAT_EQ(link.GetGain(), 0.75f);
}

TEST(NodeLink, GainWeightFormula)
{
    // GainWeight = 0.99 * gain * weight + 0.01 * weight
    CNode target;
    CNodeLink link(&target, 0.5f);
    link.SetGain(0.8f);

    REAL expected = 0.99f * 0.8f * 0.5f + 0.01f * 0.5f;
    EXPECT_FLOAT_EQ(link.GetGainWeight(), expected);
}

TEST(NodeLink, GainWeightWithDefaultGain)
{
    // With gain=1.0: GainWeight = 0.99 * 1.0 * weight + 0.01 * weight = weight
    CNode target;
    CNodeLink link(&target, 0.6f);
    EXPECT_FLOAT_EQ(link.GetGainWeight(), 0.6f);
}

TEST(NodeLink, IsStabilizerDefaultFalse)
{
    CNode target;
    CNodeLink link(&target, 0.5f);
    EXPECT_EQ(link.GetIsStabilizer(), FALSE);
}

TEST(NodeLink, SetGetIsStabilizer)
{
    CNode target;
    CNodeLink link(&target, 0.5f);
    link.SetIsStabilizer(TRUE);
    EXPECT_EQ(link.GetIsStabilizer(), TRUE);
}

TEST(NodeLink, IsLinkToCorrectTarget)
{
    CNode target;
    CNodeLink link(&target, 0.5f);
    EXPECT_TRUE(link.IsLinkTo(&target));
}

TEST(NodeLink, IsLinkToIncorrectTarget)
{
    CNode target;
    CNode other;
    CNodeLink link(&target, 0.5f);
    EXPECT_FALSE(link.IsLinkTo(&other));
}

TEST(NodeLink, HasPropagatedDefaultTrue)
{
    CNode target;
    CNodeLink link(&target, 0.5f);
    EXPECT_EQ(link.GetHasPropagated(), TRUE);
}

TEST(NodeLink, IsWeightGreaterStatic)
{
    CNode t1, t2;
    CNodeLink link1(&t1, 0.8f);
    CNodeLink link2(&t2, 0.3f);
    EXPECT_TRUE(CNodeLink::IsWeightGreater(&link1, &link2));
    EXPECT_FALSE(CNodeLink::IsWeightGreater(&link2, &link1));
}
