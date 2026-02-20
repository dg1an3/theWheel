//////////////////////////////////////////////////////////////////////
// PropertyDialogs.h: Property dialogs for nodes, links, and spaces
//
// Replaces MFC CDialog + DoDataExchange with wxDialog + sizers.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/wx.h>
#include <Node.h>
#include <Space.h>

// Node Properties Dialog
class NodePropertiesDialog : public wxDialog
{
public:
    NodePropertiesDialog(wxWindow* parent, CNode* pNode);

private:
    void OnOK(wxCommandEvent& event);

    CNode* m_pNode;
    wxTextCtrl* m_nameCtrl;
    wxTextCtrl* m_descCtrl;
    wxTextCtrl* m_classCtrl;
    wxStaticText* m_activationLabel;
    wxStaticText* m_positionLabel;
    wxStaticText* m_linkCountLabel;
};

// Space Properties Dialog
class SpacePropertiesDialog : public wxDialog
{
public:
    SpacePropertiesDialog(wxWindow* parent, CSpace* pSpace);

private:
    void OnOK(wxCommandEvent& event);

    CSpace* m_pSpace;
    wxStaticText* m_nodeCountLabel;
    wxStaticText* m_totalActivationLabel;
};
