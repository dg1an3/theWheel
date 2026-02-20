//////////////////////////////////////////////////////////////////////
// SpaceTreeView.h: Node hierarchy tree view
//
// Replaces the MFC CSpaceTreeView with wxTreeCtrl.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <Space.h>
#include <Node.h>
#include <map>

// Forward declaration
class SpacePanel;

class SpaceTreeView : public wxPanel
{
public:
    SpaceTreeView(wxWindow* parent);

    void SetSpace(CSpace* pSpace);
    CSpace* GetSpace() const { return m_pSpace; }

    void SetSpacePanel(SpacePanel* pPanel) { m_pPanel = pPanel; }

    // Select a node in the tree (called from panel on click)
    void SelectNode(CNode* pNode);

private:
    // Populate the tree from the space
    void PopulateTree();
    void AddNodeToTree(CNode* pNode, wxTreeItemId parentId);

    // Event handlers
    void OnSelChanged(wxTreeEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    void OnNewNode(wxCommandEvent& event);
    void OnDeleteNode(wxCommandEvent& event);
    void OnRenameNode(wxCommandEvent& event);
    void OnEndLabelEdit(wxTreeEvent& event);

    wxTreeCtrl* m_pTree;
    CSpace* m_pSpace;
    SpacePanel* m_pPanel;

    // Map from CNode* to tree item
    std::map<CNode*, wxTreeItemId> m_nodeItemMap;
    // Map from tree item data to CNode*
    std::map<wxTreeItemId, CNode*> m_itemNodeMap;

    enum {
        ID_TREE = 1000,
        ID_NEW_NODE,
        ID_DELETE_NODE,
        ID_RENAME_NODE,
    };

    wxDECLARE_EVENT_TABLE();
};
