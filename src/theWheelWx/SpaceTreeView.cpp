//////////////////////////////////////////////////////////////////////
// SpaceTreeView.cpp: Node hierarchy tree view
//
// Port of theWheel/SpaceTreeView.cpp from MFC CTreeCtrl to wxTreeCtrl.
//////////////////////////////////////////////////////////////////////

#include "SpaceTreeView.h"
#include "SpacePanel.h"

wxBEGIN_EVENT_TABLE(SpaceTreeView, wxPanel)
    EVT_TREE_SEL_CHANGED(SpaceTreeView::ID_TREE, SpaceTreeView::OnSelChanged)
    EVT_TREE_ITEM_RIGHT_CLICK(SpaceTreeView::ID_TREE, SpaceTreeView::OnItemRightClick)
    EVT_TREE_END_LABEL_EDIT(SpaceTreeView::ID_TREE, SpaceTreeView::OnEndLabelEdit)
    EVT_MENU(SpaceTreeView::ID_NEW_NODE, SpaceTreeView::OnNewNode)
    EVT_MENU(SpaceTreeView::ID_DELETE_NODE, SpaceTreeView::OnDeleteNode)
    EVT_MENU(SpaceTreeView::ID_RENAME_NODE, SpaceTreeView::OnRenameNode)
wxEND_EVENT_TABLE()

SpaceTreeView::SpaceTreeView(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
    , m_pSpace(nullptr)
    , m_pPanel(nullptr)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    m_pTree = new wxTreeCtrl(this, ID_TREE, wxDefaultPosition, wxDefaultSize,
                             wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT |
                             wxTR_EDIT_LABELS | wxTR_SINGLE);
    sizer->Add(m_pTree, 1, wxEXPAND);
    SetSizer(sizer);
}

void SpaceTreeView::SetSpace(CSpace* pSpace)
{
    m_pSpace = pSpace;
    PopulateTree();
}

void SpaceTreeView::PopulateTree()
{
    m_pTree->DeleteAllItems();
    m_nodeItemMap.clear();
    m_itemNodeMap.clear();

    if (!m_pSpace || !m_pSpace->GetRootNode()) return;

    wxTreeItemId rootId = m_pTree->AddRoot("Space");
    m_nodeItemMap[m_pSpace->GetRootNode()] = rootId;
    m_itemNodeMap[rootId] = m_pSpace->GetRootNode();

    // Add children of root
    CNode* pRoot = m_pSpace->GetRootNode();
    for (int i = 0; i < pRoot->GetChildCount(); i++) {
        AddNodeToTree(pRoot->GetChildAt(i), rootId);
    }

    m_pTree->ExpandAll();
}

void SpaceTreeView::AddNodeToTree(CNode* pNode, wxTreeItemId parentId)
{
    if (pNode->GetName().GetLength() == 0) return;

    wxString name((const char*)pNode->GetName());
    wxTreeItemId itemId = m_pTree->AppendItem(parentId, name);
    m_nodeItemMap[pNode] = itemId;
    m_itemNodeMap[itemId] = pNode;

    for (int i = 0; i < pNode->GetChildCount(); i++) {
        AddNodeToTree(pNode->GetChildAt(i), itemId);
    }
}

void SpaceTreeView::SelectNode(CNode* pNode)
{
    auto it = m_nodeItemMap.find(pNode);
    if (it != m_nodeItemMap.end()) {
        m_pTree->SelectItem(it->second);
    }
}

void SpaceTreeView::OnSelChanged(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    auto it = m_itemNodeMap.find(itemId);
    if (it != m_itemNodeMap.end()) {
        CNode* pNode = it->second;
        if (m_pSpace) {
            m_pSpace->SetCurrentNode(pNode);
        }
    }
}

void SpaceTreeView::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    m_pTree->SelectItem(itemId);

    wxMenu menu;
    menu.Append(ID_NEW_NODE, "New Child Node");
    menu.Append(ID_RENAME_NODE, "Rename");
    menu.AppendSeparator();
    menu.Append(ID_DELETE_NODE, "Delete");

    // Disable delete if node has children
    auto it = m_itemNodeMap.find(itemId);
    if (it != m_itemNodeMap.end()) {
        CNode* pNode = it->second;
        if (pNode->GetChildCount() > 0 || pNode == m_pSpace->GetRootNode()) {
            menu.Enable(ID_DELETE_NODE, false);
        }
    }

    PopupMenu(&menu);
}

void SpaceTreeView::OnNewNode(wxCommandEvent& event)
{
    wxTreeItemId selItem = m_pTree->GetSelection();
    if (!selItem.IsOk()) return;

    auto it = m_itemNodeMap.find(selItem);
    if (it == m_itemNodeMap.end()) return;

    CNode* pParent = it->second;
    CNode* pNewNode = new CNode();
    pNewNode->SetName(_T("NewNode"));

    m_pSpace->AddNode(pNewNode, pParent);

    // Add to tree
    wxTreeItemId newItem = m_pTree->AppendItem(selItem, "NewNode");
    m_nodeItemMap[pNewNode] = newItem;
    m_itemNodeMap[newItem] = pNewNode;

    m_pTree->Expand(selItem);
    m_pTree->SelectItem(newItem);
    m_pTree->EditLabel(newItem);

    if (m_pPanel) m_pPanel->Refresh();
}

void SpaceTreeView::OnDeleteNode(wxCommandEvent& event)
{
    wxTreeItemId selItem = m_pTree->GetSelection();
    if (!selItem.IsOk()) return;

    auto it = m_itemNodeMap.find(selItem);
    if (it == m_itemNodeMap.end()) return;

    CNode* pNode = it->second;
    if (pNode == m_pSpace->GetRootNode()) return;
    if (pNode->GetChildCount() > 0) {
        wxMessageBox("Cannot delete a node with children.", "Error",
                     wxOK | wxICON_ERROR);
        return;
    }

    m_pSpace->RemoveNode(pNode);
    m_nodeItemMap.erase(pNode);
    m_itemNodeMap.erase(selItem);
    m_pTree->Delete(selItem);

    if (m_pPanel) m_pPanel->Refresh();
}

void SpaceTreeView::OnRenameNode(wxCommandEvent& event)
{
    wxTreeItemId selItem = m_pTree->GetSelection();
    if (selItem.IsOk()) {
        m_pTree->EditLabel(selItem);
    }
}

void SpaceTreeView::OnEndLabelEdit(wxTreeEvent& event)
{
    if (event.IsEditCancelled()) return;

    wxTreeItemId itemId = event.GetItem();
    auto it = m_itemNodeMap.find(itemId);
    if (it != m_itemNodeMap.end()) {
        wxString newLabel = event.GetLabel();
        it->second->SetName((const char*)newLabel.c_str());
    }
}
