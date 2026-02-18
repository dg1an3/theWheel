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
