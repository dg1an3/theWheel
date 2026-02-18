// test_node.cpp - Unit tests for CNode class

#include "stdafx.h"
#include <gtest/gtest.h>
#include "Node.h"
#include "Space.h"
#include <cmath>

// ===== Construction =====

TEST(Node, DefaultConstruction)
{
    CNode node;
    EXPECT_EQ(node.GetSpace(), nullptr);
    EXPECT_EQ(node.GetParent(), nullptr);
}

TEST(Node, ConstructionWithName)
{
    CNode node(nullptr, _T("TestNode"));
    CString name = node.GetName();
    EXPECT_STREQ((LPCTSTR)name, _T("TestNode"));
}

TEST(Node, ConstructionWithNameAndDescription)
{
    CNode node(nullptr, _T("TestNode"), _T("A description"));
    CString desc = node.GetDescription();
    EXPECT_STREQ((LPCTSTR)desc, _T("A description"));
}

TEST(Node, InitialActivation)
{
    // Node.cpp:57-58: primary=0.005, secondary=0.005
    CNode node;
    EXPECT_NEAR(node.GetActivation(), 0.01, 1e-6);
    EXPECT_NEAR(node.GetPrimaryActivation(), 0.005, 1e-6);
    EXPECT_NEAR(node.GetSecondaryActivation(), 0.005, 1e-6);
}

// ===== Attributes =====

TEST(Node, SetGetName)
{
    CNode node;
    node.SetName(_T("NewName"));
    EXPECT_STREQ((LPCTSTR)node.GetName(), _T("NewName"));
}

TEST(Node, SetGetDescription)
{
    CNode node;
    node.SetDescription(_T("NewDesc"));
    EXPECT_STREQ((LPCTSTR)node.GetDescription(), _T("NewDesc"));
}

TEST(Node, SetGetClass)
{
    CNode node;
    node.SetClass(_T("MyClass"));
    EXPECT_STREQ((LPCTSTR)node.GetClass(), _T("MyClass"));
}

// ===== Activation =====

TEST(Node, SetActivationPrimarySecondarySplit)
{
    // With pActivator=NULL, PRIM_FRAC=0.5 applies
    CNode node;
    node.SetActivation(1.0f);
    // delta = 1.0 - 0.01 = 0.99
    // primary += 0.5 * 0.99 = 0.005 + 0.495 = 0.5
    // secondary += 0.5 * 0.99 = 0.005 + 0.495 = 0.5
    EXPECT_NEAR(node.GetPrimaryActivation(), 0.5, 1e-4);
    EXPECT_NEAR(node.GetSecondaryActivation(), 0.5, 1e-4);
    EXPECT_NEAR(node.GetActivation(), 1.0, 1e-4);
}

TEST(Node, SetActivationMultipleTimes)
{
    CNode node;
    node.SetActivation(0.5f);
    EXPECT_NEAR(node.GetActivation(), 0.5, 1e-4);
    node.SetActivation(0.8f);
    EXPECT_NEAR(node.GetActivation(), 0.8, 1e-4);
}

// ===== Linking =====

TEST(Node, LinkToCreatesLink)
{
    CNode a, b;
    a.LinkTo(&b, 0.5f, FALSE);
    EXPECT_EQ(a.GetLinkCount(), 1);
    EXPECT_EQ(a.GetLinkAt(0)->GetTarget(), &b);
}

TEST(Node, LinkToReciprocal)
{
    CNode a, b;
    a.LinkTo(&b, 0.5f, TRUE);
    EXPECT_EQ(a.GetLinkCount(), 1);
    EXPECT_EQ(b.GetLinkCount(), 1);
    EXPECT_EQ(a.GetLinkTo(&b)->GetTarget(), &b);
    EXPECT_EQ(b.GetLinkTo(&a)->GetTarget(), &a);
}

TEST(Node, SelfLinkIsIgnored)
{
    // Node.cpp:415-418: self-link returns early
    CNode a;
    a.LinkTo(&a, 0.5f);
    EXPECT_EQ(a.GetLinkCount(), 0);
}

TEST(Node, GetLinkWeight)
{
    CNode a, b;
    a.LinkTo(&b, 0.7f, FALSE);
    EXPECT_FLOAT_EQ(a.GetLinkWeight(&b), 0.7f);
}

TEST(Node, GetLinkToNonExistent)
{
    CNode a, b;
    EXPECT_EQ(a.GetLinkTo(&b), nullptr);
}

TEST(Node, GetLinkWeightNonExistent)
{
    CNode a, b;
    EXPECT_FLOAT_EQ(a.GetLinkWeight(&b), 0.0f);
}

TEST(Node, UnlinkRemovesLink)
{
    CNode a, b;
    a.LinkTo(&b, 0.5f, FALSE);
    EXPECT_EQ(a.GetLinkCount(), 1);
    a.Unlink(&b, FALSE);
    EXPECT_EQ(a.GetLinkCount(), 0);
    EXPECT_EQ(a.GetLinkTo(&b), nullptr);
}

TEST(Node, UnlinkReciprocal)
{
    CNode a, b;
    a.LinkTo(&b, 0.5f, TRUE);
    a.Unlink(&b, TRUE);
    EXPECT_EQ(a.GetLinkCount(), 0);
    EXPECT_EQ(b.GetLinkCount(), 0);
}

TEST(Node, RemoveAllLinks)
{
    CNode a, b, c;
    a.LinkTo(&b, 0.5f, FALSE);
    a.LinkTo(&c, 0.3f, FALSE);
    EXPECT_EQ(a.GetLinkCount(), 2);
    a.RemoveAllLinks();
    EXPECT_EQ(a.GetLinkCount(), 0);
}

// ===== Hierarchy =====

TEST(Node, ParentChild)
{
    // Parent owns children; parent destructor deletes children
    CNode* parent = new CNode(nullptr, _T("Parent"));
    CNode* child = new CNode(nullptr, _T("Child"));
    child->SetParent(parent);

    EXPECT_EQ(parent->GetChildCount(), 1);
    EXPECT_EQ(parent->GetChildAt(0), child);
    EXPECT_EQ(child->GetParent(), parent);

    // parent destructor will delete child
    delete parent;
}

TEST(Node, GetDescendantCount)
{
    CNode* root = new CNode(nullptr, _T("Root"));
    CNode* c1 = new CNode(nullptr, _T("C1"));
    CNode* c2 = new CNode(nullptr, _T("C2"));
    CNode* c1a = new CNode(nullptr, _T("C1a"));

    c1->SetParent(root);
    c2->SetParent(root);
    c1a->SetParent(c1);

    EXPECT_EQ(root->GetDescendantCount(), 3); // c1, c2, c1a
    EXPECT_EQ(c1->GetDescendantCount(), 1);   // c1a
    EXPECT_EQ(c2->GetDescendantCount(), 0);

    delete root; // cascading delete of c1, c2, c1a
}

TEST(Node, ReparentChild)
{
    CNode* p1 = new CNode(nullptr, _T("P1"));
    CNode* p2 = new CNode(nullptr, _T("P2"));
    CNode* child = new CNode(nullptr, _T("Child"));

    child->SetParent(p1);
    EXPECT_EQ(p1->GetChildCount(), 1);

    child->SetParent(p2);
    EXPECT_EQ(p1->GetChildCount(), 0);
    EXPECT_EQ(p2->GetChildCount(), 1);

    delete p1;
    delete p2; // deletes child
}

// ===== Position =====

TEST(Node, DefaultPositionIsZero)
{
    CNode node;
    const CVectorD<3>& pos = node.GetPosition();
    EXPECT_FLOAT_EQ(pos[0], 0.0f);
    EXPECT_FLOAT_EQ(pos[1], 0.0f);
    EXPECT_FLOAT_EQ(pos[2], 0.0f);
}

TEST(Node, SetGetPosition)
{
    CNode node;
    CVectorD<3> newPos(1.0f, 2.0f, 3.0f);
    node.SetPosition(newPos);
    const CVectorD<3>& pos = node.GetPosition();
    EXPECT_FLOAT_EQ(pos[0], 1.0f);
    EXPECT_FLOAT_EQ(pos[1], 2.0f);
    EXPECT_FLOAT_EQ(pos[2], 3.0f);
}

// ===== Radius =====

TEST(Node, RadiusIsSqrtActivation)
{
    CNode node;
    node.SetActivation(0.25f);
    EXPECT_NEAR(node.GetRadius(), sqrt(0.25), 1e-4);
}

TEST(Node, RadiusForActivation)
{
    CNode node;
    EXPECT_NEAR(node.GetRadiusForActivation(0.16f), sqrt(0.16), 1e-4);
}
