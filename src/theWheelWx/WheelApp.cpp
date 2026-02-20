//////////////////////////////////////////////////////////////////////
// WheelApp.cpp: wxWidgets application and main frame
//
// Port of theWheel MFC application to wxWidgets.
// Replaces CWinApp/CMainFrame/CDocument/CView with wxApp/wxFrame.
//////////////////////////////////////////////////////////////////////

#include "WheelApp.h"
#include "SpaceTreeView.h"
#include "SpacePanel.h"
#include "SpaceSerializer.h"
#include "PropertyDialogs.h"
#include <SpaceLayoutManager.h>

wxIMPLEMENT_APP(WheelApp);

bool WheelApp::OnInit()
{
    WheelFrame* frame = new WheelFrame();
    frame->Show(true);
    return true;
}

// Main Frame

wxBEGIN_EVENT_TABLE(WheelFrame, wxFrame)
    EVT_MENU(ID_FILE_NEW, WheelFrame::OnFileNew)
    EVT_MENU(ID_FILE_OPEN, WheelFrame::OnFileOpen)
    EVT_MENU(ID_FILE_SAVE, WheelFrame::OnFileSave)
    EVT_MENU(ID_FILE_EXIT, WheelFrame::OnFileExit)
    EVT_MENU(ID_EDIT_NODE_PROPS, WheelFrame::OnEditNodeProperties)
    EVT_MENU(ID_EDIT_SPACE_PROPS, WheelFrame::OnEditSpaceProperties)
    EVT_MENU(ID_VIEW_RESET, WheelFrame::OnViewResetView)
    EVT_MENU(ID_VIEW_RUN_LAYOUT, WheelFrame::OnViewRunLayout)
    EVT_MENU(ID_HELP_ABOUT, WheelFrame::OnHelpAbout)
wxEND_EVENT_TABLE()

WheelFrame::WheelFrame()
    : wxFrame(nullptr, wxID_ANY, "theWheel", wxDefaultPosition, wxSize(1024, 700))
{
    CreateMenuBar();
    SetupStatusBar();

    // Create splitter with tree view (left) and rendering panel (right)
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition,
                                       wxDefaultSize, wxSP_LIVE_UPDATE);

    m_treeView = new SpaceTreeView(m_splitter);
    m_spacePanel = new SpacePanel(m_splitter);

    // Wire them together
    m_treeView->SetSpacePanel(m_spacePanel);
    m_spacePanel->SetTreeView(m_treeView);

    m_splitter->SplitVertically(m_treeView, m_spacePanel, 200);
    m_splitter->SetMinimumPaneSize(100);

    // Start with a sample space
    SpaceSerializer::CreateSampleSpace(&m_space);
    m_treeView->SetSpace(&m_space);
    m_spacePanel->SetSpace(&m_space);

    // Run initial layout
    m_space.GetLayoutManager()->LayoutNodes();

    UpdateStatusBar();

    Centre();
}

WheelFrame::~WheelFrame()
{
}

void WheelFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    // File menu
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_FILE_NEW, "&New Sample Space\tCtrl+N",
                     "Create a new sample space");
    fileMenu->Append(ID_FILE_OPEN, "&Open...\tCtrl+O", "Open a space file");
    fileMenu->Append(ID_FILE_SAVE, "&Save...\tCtrl+S", "Save the current space");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_FILE_EXIT, "E&xit\tCtrl+Q", "Exit the application");
    menuBar->Append(fileMenu, "&File");

    // Edit menu
    wxMenu* editMenu = new wxMenu();
    editMenu->Append(ID_EDIT_NODE_PROPS, "Node &Properties...",
                     "Edit properties of the selected node");
    editMenu->Append(ID_EDIT_SPACE_PROPS, "&Space Properties...",
                     "View space properties");
    menuBar->Append(editMenu, "&Edit");

    // View menu
    wxMenu* viewMenu = new wxMenu();
    viewMenu->Append(ID_VIEW_RESET, "&Reset View\tCtrl+R",
                     "Reset pan and zoom");
    viewMenu->Append(ID_VIEW_RUN_LAYOUT, "Run &Layout\tCtrl+L",
                     "Run force-directed layout optimization");
    menuBar->Append(viewMenu, "&View");

    // Help menu
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(ID_HELP_ABOUT, "&About...", "About theWheel");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);
}

void WheelFrame::SetupStatusBar()
{
    wxFrame::CreateStatusBar(2);
    SetStatusText("Ready");
}

void WheelFrame::UpdateStatusBar()
{
    if (m_space.GetRootNode()) {
        wxString status = wxString::Format("Nodes: %d  |  Total Activation: %.3f",
                                            m_space.GetNodeCount(),
                                            m_space.GetTotalActivation(TRUE));
        SetStatusText(status, 1);
    }
}

void WheelFrame::OnFileNew(wxCommandEvent& event)
{
    m_space.DeleteContents();
    SpaceSerializer::CreateSampleSpace(&m_space);
    m_treeView->SetSpace(&m_space);
    m_spacePanel->SetSpace(&m_space);
    m_space.GetLayoutManager()->LayoutNodes();
    UpdateStatusBar();
    SetStatusText("Created new sample space");
}

void WheelFrame::OnFileOpen(wxCommandEvent& event)
{
    wxFileDialog openDialog(this, "Open Space File", "", "",
                            "Space JSON files (*.json)|*.json|All files (*.*)|*.*",
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openDialog.ShowModal() == wxID_CANCEL) return;

    std::string path = (const char*)openDialog.GetPath().c_str();
    CSpace newSpace;
    if (SpaceSerializer::Load(&newSpace, path)) {
        m_space.DeleteContents();
        // TODO: copy loaded space data
        m_treeView->SetSpace(&m_space);
        m_spacePanel->SetSpace(&m_space);
        UpdateStatusBar();
        SetStatusText(wxString::Format("Loaded: %s", openDialog.GetFilename()));
    } else {
        wxMessageBox("Failed to load space file.\n\nJSON loading is not yet implemented.",
                     "Open Failed", wxOK | wxICON_INFORMATION);
    }
}

void WheelFrame::OnFileSave(wxCommandEvent& event)
{
    wxFileDialog saveDialog(this, "Save Space File", "", "space.json",
                            "Space JSON files (*.json)|*.json|All files (*.*)|*.*",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL) return;

    std::string path = (const char*)saveDialog.GetPath().c_str();
    if (SpaceSerializer::Save(&m_space, path)) {
        SetStatusText(wxString::Format("Saved: %s", saveDialog.GetFilename()));
    } else {
        wxMessageBox("Failed to save space file.", "Save Failed",
                     wxOK | wxICON_ERROR);
    }
}

void WheelFrame::OnFileExit(wxCommandEvent& event)
{
    Close(true);
}

void WheelFrame::OnEditNodeProperties(wxCommandEvent& event)
{
    CNode* pNode = m_space.GetCurrentNode();
    if (!pNode) {
        wxMessageBox("No node selected.", "Properties",
                     wxOK | wxICON_INFORMATION);
        return;
    }

    NodePropertiesDialog dlg(this, pNode);
    if (dlg.ShowModal() == wxID_OK) {
        m_treeView->SetSpace(&m_space);
        m_spacePanel->Refresh();
    }
}

void WheelFrame::OnEditSpaceProperties(wxCommandEvent& event)
{
    SpacePropertiesDialog dlg(this, &m_space);
    dlg.ShowModal();
}

void WheelFrame::OnViewResetView(wxCommandEvent& event)
{
    m_spacePanel->ResetView();
    m_spacePanel->Refresh();
}

void WheelFrame::OnViewRunLayout(wxCommandEvent& event)
{
    SetStatusText("Running layout optimization...");
    m_space.GetLayoutManager()->LayoutNodes();
    m_spacePanel->Refresh();
    UpdateStatusBar();
    SetStatusText("Layout complete");
}

void WheelFrame::OnHelpAbout(wxCommandEvent& event)
{
    wxMessageBox(
        "theWheel - Semantic Visualization System\n\n"
        "A spreading activation network for semantic browsing\n"
        "and knowledge exploration.\n\n"
        "Copyright (c) 1996-2025 Derek Graham Lane\n\n"
        "wxWidgets port for macOS",
        "About theWheel",
        wxOK | wxICON_INFORMATION);
}
