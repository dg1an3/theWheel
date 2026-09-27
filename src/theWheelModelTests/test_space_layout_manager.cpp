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

// ===== GetDistError =====

// optimal normalized distance (OPT_DIST in SpaceLayoutManager.cpp)
static const REAL EXPECTED_OPT_DIST = 1.0f + 0.35f * (1.0f - 0.5f / 0.75f);

// expected normalized distance error for an offset between two nodes
static REAL ExpectedDistError(CNode* from, CNode* to, REAL dx, REAL dy)
{
    const REAL sizeAvg = 0.5f * ((150.0f * from->GetRadius() + 10.0f)
        + (150.0f * to->GetRadius() + 10.0f));
    return std::sqrt((dx * dx + dy * dy) / (sizeAvg * sizeAvg)) + 0.001f
        - EXPECTED_OPT_DIST;
}

TEST(SpaceLayoutManager, DistErrorUsesActualOffset)
{
    CSpace* space = CreateSpaceWithRoot();
    CNode a, b;
    a.SetActivation(0.1f);
    b.SetActivation(0.1f);
    a.SetPosition(CVectorD<3>(0.0, 0.0, 0.0));
    b.SetPosition(CVectorD<3>(30.0, 40.0, 0.0));

    REAL err = space->GetLayoutManager()->GetDistError(&a, &b);
    EXPECT_NEAR(err, ExpectedDistError(&a, &b, 30.0f, 40.0f), 1e-4f);

    // moving the nodes apart must increase the error
    b.SetPosition(CVectorD<3>(90.0, 120.0, 0.0));
    REAL errFar = space->GetLayoutManager()->GetDistError(&a, &b);
    EXPECT_GT(errFar, err);

    delete space;
}

TEST(SpaceLayoutManager, DistErrorUsesNearestWrappedImage)
{
    CSpace* space = CreateSpaceWithRoot();
    CNode a, b;
    a.SetActivation(0.1f);
    b.SetActivation(0.1f);
    a.SetPosition(CVectorD<3>(0.0, 0.0, 0.0));

    // 790 apart in x wraps (width 800) to 10 apart
    b.SetPosition(CVectorD<3>(790.0, 0.0, 0.0));
    EXPECT_NEAR(space->GetLayoutManager()->GetDistError(&a, &b),
        ExpectedDistError(&a, &b, 10.0f, 0.0f), 1e-4f);

    // diagonal wrap: (790, 395) wraps to (10, 5)
    b.SetPosition(CVectorD<3>(790.0, 395.0, 0.0));
    EXPECT_NEAR(space->GetLayoutManager()->GetDistError(&a, &b),
        ExpectedDistError(&a, &b, 10.0f, 5.0f), 1e-4f);

    delete space;
}

// ===== Free energy =====

TEST(SpaceLayoutManager, FreeEnergyIsEnergyPlusHalfLogDet)
{
    CSpace* space = CreateSpaceWithRoot();
    space->CreateSimpleSpace();
    CSpaceLayoutManager* layout = space->GetLayoutManager();
    layout->LayoutNodes();

    EXPECT_TRUE(std::isfinite(layout->GetFreeEnergy()));
    EXPECT_TRUE(std::isfinite(layout->GetLogDetHessian()));

    // free energy is computed before Relax() changes the gains, so
    //		recompute at the laid-out state for the comparison
    layout->LoadSizesLinks(0, layout->GetStateDim() / 2);
    layout->UpdateFreeEnergy();
    EXPECT_NEAR(layout->GetFreeEnergy(),
        layout->GetEnergy() + 0.5f * layout->GetLogDetHessian(),
        1e-3f * std::fabs(layout->GetFreeEnergy()) + 1e-3f);

    delete space;
}

TEST(SpaceLayoutManager, UpdateFreeEnergyPreservesEnergy)
{
    CSpace* space = CreateSpaceWithRoot();
    space->CreateSimpleSpace();
    CSpaceLayoutManager* layout = space->GetLayoutManager();
    layout->LayoutNodes();
    layout->LoadSizesLinks(0, layout->GetStateDim() / 2);
    layout->UpdateFreeEnergy();
    REAL energy = layout->GetEnergy();

    // a second evaluation at the same state gives the same results
    REAL freeEnergy = layout->GetFreeEnergy();
    layout->UpdateFreeEnergy();
    EXPECT_FLOAT_EQ(layout->GetEnergy(), energy);
    EXPECT_FLOAT_EQ(layout->GetFreeEnergy(), freeEnergy);

    delete space;
}
