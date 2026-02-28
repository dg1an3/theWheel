//////////////////////////////////////////////////////////////////////
// test_space_tree_view.cpp: Tests for SpaceTreeView
//////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>
#include <wx/wx.h>
#include "SpaceTreeView.h"
#include "SpaceSerializer.h"
#include <Space.h>
#include <Node.h>

// Test accessor for SpaceTreeView private members
class SpaceTreeViewTestAccessor
{
public:
    static wxTreeCtrl* GetTree(SpaceTreeView* p) { return p->m_pTree; }
    static const std::map<CNode*, wxTreeItemId>& GetNodeItemMap(const SpaceTreeView* p) { return p->m_nodeItemMap; }
    static const std::map<wxTreeItemId, CNode*>& GetItemNodeMap(const SpaceTreeView* p) { return p->m_itemNodeMap; }
};

class SpaceTreeViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_frame = new wxFrame(nullptr, wxID_ANY, "Test", wxDefaultPosition, wxSize(400, 600));
        m_treeView = new SpaceTreeView(m_frame);
        m_space = new CSpace();
        SpaceSerializer::CreateSampleSpace(m_space);
        m_treeView->SetSpace(m_space);
        m_frame->Show(false);
    }

    void TearDown() override
    {
        m_frame->Destroy();
        wxTheApp->ProcessPendingEvents();
        delete m_space;
    }

    wxFrame* m_frame;
    SpaceTreeView* m_treeView;
    CSpace* m_space;
};

TEST_F(SpaceTreeViewTest, TestPopulateTree)
{
    wxTreeCtrl* tree = SpaceTreeViewTestAccessor::GetTree(m_treeView);
    ASSERT_TRUE(tree != nullptr);

    // Tree should have a root item
    wxTreeItemId rootId = tree->GetRootItem();
    EXPECT_TRUE(rootId.IsOk());

    // The node-to-item map should have entries
    auto& nodeMap = SpaceTreeViewTestAccessor::GetNodeItemMap(m_treeView);
    EXPECT_GT(nodeMap.size(), 0u);

    // The nodeItemMap includes the root node (mapped to "Space" tree item)
    // plus all descendants with non-empty names. Verify it's reasonable.
    EXPECT_GE(nodeMap.size(), 2u); // at least root + 1 child
}

TEST_F(SpaceTreeViewTest, TestSelectNode)
{
    CNode* pNode = m_space->GetNodeAt(1);
    m_treeView->SelectNode(pNode);

    wxTreeCtrl* tree = SpaceTreeViewTestAccessor::GetTree(m_treeView);
    wxTreeItemId selItem = tree->GetSelection();
    EXPECT_TRUE(selItem.IsOk());

    // Verify the selected item maps to our node
    auto& itemMap = SpaceTreeViewTestAccessor::GetItemNodeMap(m_treeView);
    auto it = itemMap.find(selItem);
    ASSERT_NE(it, itemMap.end());
    EXPECT_EQ(it->second, pNode);
}

TEST_F(SpaceTreeViewTest, TestNewChildNode)
{
    int countBefore = m_space->GetNodeCount();
    auto& nodeMap = SpaceTreeViewTestAccessor::GetNodeItemMap(m_treeView);
    size_t mapSizeBefore = nodeMap.size();

    // Select a node to be the parent
    CNode* pParent = m_space->GetNodeAt(1);
    m_treeView->SelectNode(pParent);

    // Simulate the "New Child Node" menu command
    wxCommandEvent newNodeEvent(wxEVT_MENU, SpaceTreeView::ID_NEW_NODE);
    newNodeEvent.SetEventObject(m_treeView);
    m_treeView->ProcessWindowEvent(newNodeEvent);

    // A new node should have been added to the model
    EXPECT_EQ(m_space->GetNodeCount(), countBefore + 1);

    // The tree map should have one more entry
    EXPECT_EQ(nodeMap.size(), mapSizeBefore + 1);
}

TEST_F(SpaceTreeViewTest, TestSetSpaceRepopulates)
{
    // Create a different space using CreateSimpleSpace (which sets root properly)
    CSpace space2;
    space2.CreateSimpleSpace();

    m_treeView->SetSpace(&space2);

    auto& nodeMap = SpaceTreeViewTestAccessor::GetNodeItemMap(m_treeView);
    EXPECT_GT(nodeMap.size(), 0u);

    wxTreeCtrl* tree = SpaceTreeViewTestAccessor::GetTree(m_treeView);
    wxTreeItemId rootId = tree->GetRootItem();
    EXPECT_TRUE(rootId.IsOk());

    // Restore original space for TearDown
    m_treeView->SetSpace(m_space);
}
