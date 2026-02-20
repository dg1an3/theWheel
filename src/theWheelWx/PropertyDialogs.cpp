//////////////////////////////////////////////////////////////////////
// PropertyDialogs.cpp: Property dialogs for nodes, links, and spaces
//////////////////////////////////////////////////////////////////////

#include "PropertyDialogs.h"

// Node Properties Dialog

NodePropertiesDialog::NodePropertiesDialog(wxWindow* parent, CNode* pNode)
    : wxDialog(parent, wxID_ANY, "Node Properties", wxDefaultPosition,
               wxSize(350, 300))
    , m_pNode(pNode)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Name
    wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
    nameSizer->Add(new wxStaticText(this, wxID_ANY, "Name:"), 0,
                   wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_nameCtrl = new wxTextCtrl(this, wxID_ANY,
                                wxString((const char*)pNode->GetName()));
    nameSizer->Add(m_nameCtrl, 1, wxEXPAND);
    mainSizer->Add(nameSizer, 0, wxEXPAND | wxALL, 10);

    // Description
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Description:"), 0,
                   wxLEFT | wxRIGHT, 10);
    m_descCtrl = new wxTextCtrl(this, wxID_ANY,
                                wxString((const char*)pNode->GetDescription()),
                                wxDefaultPosition, wxSize(-1, 60),
                                wxTE_MULTILINE);
    mainSizer->Add(m_descCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Class
    wxBoxSizer* classSizer = new wxBoxSizer(wxHORIZONTAL);
    classSizer->Add(new wxStaticText(this, wxID_ANY, "Class:"), 0,
                    wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_classCtrl = new wxTextCtrl(this, wxID_ANY,
                                 wxString((const char*)pNode->GetClass()));
    classSizer->Add(m_classCtrl, 1, wxEXPAND);
    mainSizer->Add(classSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Read-only info
    wxString actStr = wxString::Format("Activation: %.4f", pNode->GetActivation());
    m_activationLabel = new wxStaticText(this, wxID_ANY, actStr);
    mainSizer->Add(m_activationLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    const CVectorD<3>& pos = pNode->GetPosition();
    wxString posStr = wxString::Format("Position: (%.1f, %.1f, %.1f)",
                                       pos[0], pos[1], pos[2]);
    m_positionLabel = new wxStaticText(this, wxID_ANY, posStr);
    mainSizer->Add(m_positionLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxString linkStr = wxString::Format("Links: %d", pNode->GetLinkCount());
    m_linkCountLabel = new wxStaticText(this, wxID_ANY, linkStr);
    mainSizer->Add(m_linkCountLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // OK / Cancel buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okBtn = new wxButton(this, wxID_OK, "OK");
    wxButton* cancelBtn = new wxButton(this, wxID_CANCEL, "Cancel");
    btnSizer->Add(okBtn, 0, wxRIGHT, 5);
    btnSizer->Add(cancelBtn, 0);
    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizer(mainSizer);

    okBtn->Bind(wxEVT_BUTTON, &NodePropertiesDialog::OnOK, this);
}

void NodePropertiesDialog::OnOK(wxCommandEvent& event)
{
    m_pNode->SetName((const char*)m_nameCtrl->GetValue().c_str());
    m_pNode->SetDescription((const char*)m_descCtrl->GetValue().c_str());
    m_pNode->SetClass((const char*)m_classCtrl->GetValue().c_str());
    EndModal(wxID_OK);
}

// Space Properties Dialog

SpacePropertiesDialog::SpacePropertiesDialog(wxWindow* parent, CSpace* pSpace)
    : wxDialog(parent, wxID_ANY, "Space Properties", wxDefaultPosition,
               wxSize(300, 200))
    , m_pSpace(pSpace)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxString nodeStr = wxString::Format("Node Count: %d", pSpace->GetNodeCount());
    m_nodeCountLabel = new wxStaticText(this, wxID_ANY, nodeStr);
    mainSizer->Add(m_nodeCountLabel, 0, wxALL, 10);

    wxString actStr = wxString::Format("Total Activation: %.4f",
                                        pSpace->GetTotalActivation(TRUE));
    m_totalActivationLabel = new wxStaticText(this, wxID_ANY, actStr);
    mainSizer->Add(m_totalActivationLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxButton* okBtn = new wxButton(this, wxID_OK, "OK");
    mainSizer->Add(okBtn, 0, wxALIGN_CENTER | wxALL, 10);

    SetSizer(mainSizer);
}

void SpacePropertiesDialog::OnOK(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}
