// test_display_policy.cpp - Unit tests for CDisplayPolicy and its use in CSpace

#include "stdafx.h"
#include <gtest/gtest.h>
#include "Space.h"
#include "DisplayPolicy.h"
#include <algorithm>
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

// Helper: adds n unlinked nodes under the root
static std::vector<CNode*> AddNodes(CSpace* space, int n)
{
    std::vector<CNode*> nodes;
    for (int i = 0; i < n; i++)
    {
        CNode* node = new CNode();
        node->SetParent(space->GetRootNode());
        space->AddNode(node, nullptr);
        nodes.push_back(node);
    }
    return nodes;
}

// ===== Relatedness =====

TEST(DisplayPolicy, RelatednessIsStrongestPathProduct)
{
    CNode a, b, c, d;
    a.LinkTo(&b, 0.5f);
    b.LinkTo(&c, 0.4f);
    std::vector<CNode*> nodes = { &a, &b, &c, &d };

    CDisplayPolicy policy;
    policy.Update(nodes);
    EXPECT_DOUBLE_EQ(policy.GetRelatedness(&a, &a), 1.0);
    EXPECT_NEAR(policy.GetRelatedness(&a, &b), 0.5, 1e-6);
    EXPECT_NEAR(policy.GetRelatedness(&c, &a), 0.2, 1e-6);
    EXPECT_DOUBLE_EQ(policy.GetRelatedness(&a, &d), 0.0);
}

// ===== Belief =====

TEST(DisplayPolicy, BeliefIsActivationTimesEvidence)
{
    CNode a, b;
    a.SetActivation(0.2f);
    b.SetActivation(0.2f);
    b.SetClickEvidence((REAL) log(3.0));
    std::vector<CNode*> nodes = { &a, &b };

    CDisplayPolicy policy;
    std::vector<double> q;
    policy.GetBelief(nodes, q);
    EXPECT_NEAR(q[0], 0.25, 1e-6);
    EXPECT_NEAR(q[1], 0.75, 1e-6);
}

// ===== Click evidence =====

TEST(DisplayPolicy, ClickFavorsTargetsRelatedToClickedNode)
{
    // chain a - b - c, and an unrelated d
    CNode a, b, c, d;
    a.LinkTo(&b, 0.8f);
    b.LinkTo(&c, 0.8f);
    std::vector<CNode*> nodes = { &a, &b, &c, &d };
    std::vector<CNode*> displayed = { &a, &d };

    CDisplayPolicy policy;
    policy.ObserveClick(nodes, displayed, &a, 1.0);

    EXPECT_FLOAT_EQ(a.GetClickEvidence(), 0.0f);    // the largest is kept at 0
    EXPECT_GT(b.GetClickEvidence(), c.GetClickEvidence());
    EXPECT_GT(c.GetClickEvidence(), d.GetClickEvidence());
}

TEST(DisplayPolicy, ClickOnUndisplayedNodeOnlyDecaysEvidence)
{
    CNode a, b;
    a.SetClickEvidence(-2.0f);
    std::vector<CNode*> nodes = { &a, &b };
    std::vector<CNode*> displayed = { &a };

    CDisplayPolicy policy;
    policy.ObserveClick(nodes, displayed, &b, 1.0);
    EXPECT_GT(a.GetClickEvidence(), -2.0f);
    EXPECT_LT(a.GetClickEvidence(), 0.0f);
    EXPECT_FLOAT_EQ(b.GetClickEvidence(), 0.0f);
}

// ===== Selection =====

TEST(DisplayPolicy, SelectReturnsDistinctNodes)
{
    CSpace* space = CreateSpaceWithRoot();
    std::vector<CNode*> nodes = AddNodes(space, 10);
    for (int i = 0; i + 1 < 10; i++)
    {
        nodes[i]->LinkTo(nodes[i + 1], 0.6f);
    }

    std::vector<int> selected;
    space->GetDisplayPolicy().Select(nodes, 4, selected);
    ASSERT_EQ(selected.size(), 4u);
    std::sort(selected.begin(), selected.end());
    EXPECT_EQ(std::unique(selected.begin(), selected.end()), selected.end());

    // asking for more than there are returns them all
    space->GetDisplayPolicy().Select(nodes, 20, selected);
    EXPECT_EQ(selected.size(), 10u);

    delete space;
}

TEST(DisplayPolicy, SelectAvoidsRedundantNodes)
{
    // two identical twins and a distinct node: with two slots, a twin
    //		plus the distinct node tells more than both twins
    CNode twin1, twin2, other;
    twin1.LinkTo(&twin2, 1.0f);
    twin1.SetActivation(0.35f);
    twin2.SetActivation(0.35f);
    other.SetActivation(0.30f);
    std::vector<CNode*> nodes = { &twin1, &twin2, &other };

    CDisplayPolicy policy;
    policy.SetGamma(0.0);
    std::vector<int> selected;
    policy.Select(nodes, 2, selected);
    EXPECT_NE(std::find(selected.begin(), selected.end(), 2), selected.end());
}

// ===== Use in CSpace =====

TEST(DisplayPolicy, SpaceUsesEfeDisplayByDefault)
{
    CSpace* space = CreateSpaceWithRoot();
    EXPECT_TRUE(space->GetEfeDisplay());
    delete space;
}

TEST(DisplayPolicy, SortPutsSelectedNodesFirst)
{
    CSpace* space = CreateSpaceWithRoot();
    std::vector<CNode*> nodes = AddNodes(space, 12);
    for (int i = 0; i + 1 < 12; i++)
    {
        nodes[i]->LinkTo(nodes[i + 1], 0.6f);
    }
    space->GetLayoutManager()->SetMaxSuperNodeCount(4);

    // the policy's choice on the activation-sorted nodes
    space->SetEfeDisplay(false);
    space->SortNodes();
    std::vector<CNode*> sorted;
    for (int i = 0; i < space->GetNodeCount(); i++)
    {
        sorted.push_back(space->GetNodeAt(i));
    }
    std::vector<int> selected;
    space->GetDisplayPolicy().Select(sorted, 4, selected);

    space->SetEfeDisplay(true);
    space->SortNodes();
    for (int nAt : selected)
    {
        CNode* node = sorted[nAt];
        int pos = 0;
        while (space->GetNodeAt(pos) != node) pos++;
        EXPECT_LT(pos, 4);
    }

    delete space;
}

TEST(DisplayPolicy, ActivateNodeRecordsClickEvidence)
{
    CSpace* space = CreateSpaceWithRoot();
    std::vector<CNode*> nodes = AddNodes(space, 4);
    nodes[0]->LinkTo(nodes[1], 0.8f);
    nodes[0]->SetIsSubThreshold(FALSE);
    nodes[2]->SetIsSubThreshold(FALSE);

    space->ActivateNode(nodes[0], 0.5f);

    // clicking a displayed node favors it and its neighbor over the
    //		node that was displayed but passed over
    EXPECT_GT(nodes[0]->GetClickEvidence(), nodes[2]->GetClickEvidence());
    EXPECT_GT(nodes[1]->GetClickEvidence(), nodes[2]->GetClickEvidence());

    delete space;
}
