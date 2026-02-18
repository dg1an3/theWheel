// test_space_layout_manager.cpp - Unit tests for CSpaceLayoutManager class

#include "stdafx.h"
#include <gtest/gtest.h>
#include "Space.h"
#include "SpaceLayoutManager.h"
#include <cmath>

// Helper: create a CSpace with a hidden root node
static CSpace* CreateSpaceWithRoot()
{
    CSpace* space = new CSpace();
    CNode* root = new CNode();
    root->SetName(_T("%%hiddenroot%%"));
    space->SetRootNode(root);
    return space;
}

// ===== Construction =====

TEST(SpaceLayoutManager, CreatedBySpace)
{
    CSpace* space = CreateSpaceWithRoot();
    EXPECT_NE(space->GetLayoutManager(), nullptr);
    delete space;
}

// ===== KPos default and set/get =====

TEST(SpaceLayoutManager, KPosDefault)
{
    CSpace* space = CreateSpaceWithRoot();
    // Default K_POS = 600.0f (from SpaceLayoutManager.cpp)
    EXPECT_FLOAT_EQ(space->GetLayoutManager()->GetKPos(), 600.0f);
    delete space;
}

TEST(SpaceLayoutManager, SetGetKPos)
{
    CSpace* space = CreateSpaceWithRoot();
    space->GetLayoutManager()->SetKPos(1000.0f);
    EXPECT_FLOAT_EQ(space->GetLayoutManager()->GetKPos(), 1000.0f);
    delete space;
}

// ===== KRep default and set/get =====

TEST(SpaceLayoutManager, KRepDefault)
{
    CSpace* space = CreateSpaceWithRoot();
    // Default K_REP = 3200.0f (from SpaceLayoutManager.cpp)
    EXPECT_FLOAT_EQ(space->GetLayoutManager()->GetKRep(), 3200.0f);
    delete space;
}

TEST(SpaceLayoutManager, SetGetKRep)
{
    CSpace* space = CreateSpaceWithRoot();
    space->GetLayoutManager()->SetKRep(5000.0f);
    EXPECT_FLOAT_EQ(space->GetLayoutManager()->GetKRep(), 5000.0f);
    delete space;
}

// ===== Tolerance default and set/get =====

TEST(SpaceLayoutManager, ToleranceDefault)
{
    CSpace* space = CreateSpaceWithRoot();
    // Default TOLERANCE = 1e+3 (from SpaceLayoutManager.cpp)
    EXPECT_FLOAT_EQ(space->GetLayoutManager()->GetTolerance(), 1e+3f);
    delete space;
}

TEST(SpaceLayoutManager, SetGetTolerance)
{
    CSpace* space = CreateSpaceWithRoot();
    space->GetLayoutManager()->SetTolerance(500.0f);
    EXPECT_FLOAT_EQ(space->GetLayoutManager()->GetTolerance(), 500.0f);
    delete space;
}

// ===== StateDim =====

TEST(SpaceLayoutManager, StateDimDefault)
{
    CSpace* space = CreateSpaceWithRoot();
    // Default state dimension = 80 (from SpaceLayoutManager.cpp:92)
    EXPECT_EQ(space->GetLayoutManager()->GetStateDim(), 80);
    delete space;
}

// ===== Energy =====

TEST(SpaceLayoutManager, EnergyFiniteAfterCreateSimpleSpace)
{
    CSpace* space = CreateSpaceWithRoot();
    space->CreateSimpleSpace();

    REAL energy = space->GetLayoutManager()->GetEnergy();
    EXPECT_TRUE(std::isfinite(energy));
    EXPECT_GE(energy, 0.0f);

    delete space;
}
