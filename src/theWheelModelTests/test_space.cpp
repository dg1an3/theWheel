// test_space.cpp - Unit tests for CSpace class

#include "stdafx.h"
#include <gtest/gtest.h>
#include "Space.h"
#include "Node.h"

// Helper: create a CSpace with a hidden root node (required for most operations)
static CSpace* CreateSpaceWithRoot()
{
    CSpace* space = new CSpace();
    CNode* root = new CNode();
    root->SetName(_T("%%hiddenroot%%"));
    space->SetRootNode(root);
    return space;
}

// ===== Construction =====

TEST(Space, Construction)
{
    CSpace space;
    EXPECT_NE(space.GetLayoutManager(), nullptr);
    EXPECT_EQ(space.GetNodeCount(), 0);
}

TEST(Space, ConstructionWithRoot)
{
    CSpace* space = CreateSpaceWithRoot();
    EXPECT_NE(space->GetRootNode(), nullptr);
    EXPECT_EQ(space->GetNodeCount(), 0);
    delete space;
}

// ===== Node Management =====

TEST(Space, AddNodeIncreasesCount)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* node = new CNode();
    node->SetName(_T("TestNode"));
    node->SetParent(space->GetRootNode());
    space->AddNode(node, nullptr);

    EXPECT_EQ(space->GetNodeCount(), 1);
    EXPECT_EQ(space->GetNodeAt(0), node);

    delete space;
}

TEST(Space, AddMultipleNodes)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* n1 = new CNode();
    n1->SetName(_T("Node1"));
    n1->SetParent(space->GetRootNode());
    space->AddNode(n1, nullptr);

    CNode* n2 = new CNode();
    n2->SetName(_T("Node2"));
    n2->SetParent(space->GetRootNode());
    space->AddNode(n2, nullptr);

    EXPECT_EQ(space->GetNodeCount(), 2);
    delete space;
}

TEST(Space, RemoveNode)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* node = new CNode();
    node->SetName(_T("Node"));
    node->SetParent(space->GetRootNode());
    space->AddNode(node, nullptr);
    EXPECT_EQ(space->GetNodeCount(), 1);

    space->RemoveNode(node);
    EXPECT_EQ(space->GetNodeCount(), 0);

    delete node;
    delete space;
}

// ===== Activation =====

TEST(Space, ActivateNodeIncreasesActivation)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* node = new CNode();
    node->SetName(_T("Node"));
    node->SetParent(space->GetRootNode());
    space->AddNode(node, nullptr);

    REAL before = node->GetActivation();
    space->ActivateNode(node, 0.5f);
    REAL after = node->GetActivation();

    EXPECT_GT(after, before);
    delete space;
}

TEST(Space, TotalActivation)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* n1 = new CNode();
    n1->SetName(_T("N1"));
    n1->SetParent(space->GetRootNode());
    space->AddNode(n1, nullptr);

    CNode* n2 = new CNode();
    n2->SetName(_T("N2"));
    n2->SetParent(space->GetRootNode());
    space->AddNode(n2, nullptr);

    // Compute total activation from scratch
    REAL total = space->GetTotalActivation(TRUE);
    REAL sum = n1->GetActivation() + n2->GetActivation();
    EXPECT_NEAR(total, sum, 1e-4);

    delete space;
}

// ===== Normalization =====

TEST(Space, NormalizeKeepsBounded)
{
    CSpace* space = CreateSpaceWithRoot();

    // Add several nodes with various activations
    for (int i = 0; i < 5; i++)
    {
        CNode* node = new CNode();
        node->SetName(_T("N"));
        node->SetParent(space->GetRootNode());
        space->AddNode(node, nullptr);
    }

    // Boost one node
    space->ActivateNode(space->GetNodeAt(0), 1.0f);

    // Normalize
    space->NormalizeNodes(1.0);

    REAL total = space->GetTotalActivation(TRUE);
    // After normalization, total should be <= 1.0 (it only scales down, not up)
    EXPECT_LE(total, 1.01f); // small tolerance
    delete space;
}

// ===== Sorting =====

TEST(Space, SortNodesDescendingActivation)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* low = new CNode();
    low->SetName(_T("Low"));
    low->SetParent(space->GetRootNode());
    space->AddNode(low, nullptr);

    CNode* high = new CNode();
    high->SetName(_T("High"));
    high->SetParent(space->GetRootNode());
    space->AddNode(high, nullptr);

    // Give 'high' more activation
    space->ActivateNode(high, 1.0f);

    space->SortNodes();

    // First node should have higher activation
    EXPECT_GE(space->GetNodeAt(0)->GetActivation(),
              space->GetNodeAt(1)->GetActivation());

    delete space;
}

// ===== CreateSimpleSpace =====

TEST(Space, CreateSimpleSpacePopulatesNodes)
{
    CSpace* space = CreateSpaceWithRoot();
    BOOL result = space->CreateSimpleSpace();
    EXPECT_TRUE(result);
    EXPECT_GT(space->GetNodeCount(), 0);
    delete space;
}

// ===== CurrentNode =====

TEST(Space, SetGetCurrentNode)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* node = new CNode();
    node->SetName(_T("Current"));
    node->SetParent(space->GetRootNode());
    space->AddNode(node, nullptr);

    space->SetCurrentNode(node);
    EXPECT_EQ(space->GetCurrentNode(), node);

    delete space;
}

TEST(Space, CurrentNodeDefaultNull)
{
    CSpace space;
    EXPECT_EQ(space.GetCurrentNode(), nullptr);
}

// ===== DeleteContents =====

TEST(Space, DeleteContentsClearsEverything)
{
    CSpace* space = CreateSpaceWithRoot();
    space->CreateSimpleSpace();
    EXPECT_GT(space->GetNodeCount(), 0);

    space->DeleteContents();
    EXPECT_EQ(space->GetNodeCount(), 0);
    EXPECT_EQ(space->GetRootNode(), nullptr);

    delete space;
}

// ===== Activation variance =====

TEST(Space, ActivateNodeReducesVarianceOfActivatedNode)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* a = new CNode();
    a->SetParent(space->GetRootNode());
    space->AddNode(a, nullptr);
    CNode* b = new CNode();
    b->SetParent(space->GetRootNode());
    space->AddNode(b, nullptr);

    space->ActivateNode(a, 0.5f);

    EXPECT_LT(a->GetActivationVariance(), CNode::GetActivationVariancePrior());
    EXPECT_FLOAT_EQ(b->GetActivationVariance(), CNode::GetActivationVariancePrior());

    delete space;
}

TEST(Space, VarianceGrowsBackWhenAttentionMovesElsewhere)
{
    CSpace* space = CreateSpaceWithRoot();

    CNode* a = new CNode();
    a->SetParent(space->GetRootNode());
    space->AddNode(a, nullptr);
    CNode* b = new CNode();
    b->SetParent(space->GetRootNode());
    space->AddNode(b, nullptr);

    space->ActivateNode(a, 0.5f);
    const REAL afterClick = a->GetActivationVariance();

    // clicking elsewhere lets the belief about a relax
    space->ActivateNode(b, 0.5f);
    EXPECT_GT(a->GetActivationVariance(), afterClick);

    // and it returns to the prior after enough clicks elsewhere
    for (int i = 0; i < 50; i++)
    {
        space->ActivateNode(b, 0.5f);
    }
    EXPECT_FLOAT_EQ(a->GetActivationVariance(), CNode::GetActivationVariancePrior());
    EXPECT_LT(b->GetActivationVariance(), a->GetActivationVariance());

    delete space;
}

// ===== Epistemic value =====

// adds a node directly under the hidden root
static CNode* AddTestNode(CSpace* space, REAL activation, REAL variance)
{
    CNode* node = new CNode();
    node->SetParent(space->GetRootNode());
    space->AddNode(node, nullptr);
    node->SetActivation(activation);
    node->SetActivationVariance(variance);
    return node;
}

TEST(Space, ExpectedInformationGainFallsWithVariance)
{
    CSpace* space = CreateSpaceWithRoot();
    CNode* unexplored = AddTestNode(space, 0.1f, CNode::GetActivationVariancePrior());
    CNode* explored = AddTestNode(space, 0.1f, 0.01f);
    CNode* certain = AddTestNode(space, 0.1f, 0.0f);

    EXPECT_GT(space->GetExpectedInformationGain(unexplored),
        space->GetExpectedInformationGain(explored));
    EXPECT_GT(space->GetExpectedInformationGain(explored), 0.0f);
    EXPECT_FLOAT_EQ(space->GetExpectedInformationGain(certain), 0.0f);

    delete space;
}

TEST(Space, EpistemicWeightDefaultAndSet)
{
    CSpace* space = CreateSpaceWithRoot();
    EXPECT_FLOAT_EQ(space->GetEpistemicWeight(), 0.0f);
    space->SetEpistemicWeight(0.0f);
    EXPECT_FLOAT_EQ(space->GetEpistemicWeight(), 0.0f);
    delete space;
}

TEST(Space, SortPrefersUncertainNodeWhenActivationsTie)
{
    CSpace* space = CreateSpaceWithRoot();
    CNode* explored = AddTestNode(space, 0.1f, 0.01f);
    CNode* unexplored = AddTestNode(space, 0.1f, CNode::GetActivationVariancePrior());

    space->SetEpistemicWeight(0.02f);
    space->SortNodes();
    EXPECT_EQ(space->GetNodeAt(0), unexplored);
    EXPECT_EQ(space->GetNodeAt(1), explored);

    delete space;
}

TEST(Space, SortByActivationAloneWhenEpistemicWeightZero)
{
    CSpace* space = CreateSpaceWithRoot();
    CNode* unexplored = AddTestNode(space, 0.10f, CNode::GetActivationVariancePrior());
    CNode* explored = AddTestNode(space, 0.11f, 0.01f);

    // a small activation lead loses to the epistemic bonus...
    space->SetEpistemicWeight(0.02f);
    space->SortNodes();
    EXPECT_EQ(space->GetNodeAt(0), unexplored);

    // ...but wins with the epistemic value turned off
    space->SetEpistemicWeight(0.0f);
    space->SortNodes();
    EXPECT_EQ(space->GetNodeAt(0), explored);

    delete space;
}

TEST(Space, SortStillFollowsLargeActivationDifferences)
{
    CSpace* space = CreateSpaceWithRoot();
    CNode* unexplored = AddTestNode(space, 0.05f, CNode::GetActivationVariancePrior());
    CNode* relevant = AddTestNode(space, 0.30f, 0.01f);

    space->SetEpistemicWeight(0.02f);
    space->SortNodes();
    EXPECT_EQ(space->GetNodeAt(0), relevant);
    EXPECT_EQ(space->GetNodeAt(1), unexplored);

    delete space;
}

TEST(Space, DisplayedNodesPassedOverLoseSomeUncertainty)
{
    CSpace* space = CreateSpaceWithRoot();
    const REAL prior = CNode::GetActivationVariancePrior();
    CNode* clicked = AddTestNode(space, 0.1f, prior);
    CNode* displayed = AddTestNode(space, 0.1f, prior);
    CNode* hidden = AddTestNode(space, 0.1f, prior);
    clicked->SetIsSubThreshold(FALSE);
    displayed->SetIsSubThreshold(FALSE);
    hidden->SetIsSubThreshold(TRUE);

    space->ActivateNode(clicked, 0.5f);

    // seeing a node and not choosing it is weaker evidence than a click
    EXPECT_LT(displayed->GetActivationVariance(), prior);
    EXPECT_LT(clicked->GetActivationVariance(), displayed->GetActivationVariance());

    // a node that was not displayed gives no evidence
    EXPECT_FLOAT_EQ(hidden->GetActivationVariance(), prior);

    delete space;
}
