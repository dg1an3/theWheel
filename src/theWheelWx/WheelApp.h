//////////////////////////////////////////////////////////////////////
// WheelApp.h: wxWidgets application and main frame
//
// Replaces MFC CWinApp/CMainFrame with wxApp/wxFrame.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/wx.h>
#include <wx/splitter.h>
#include <Space.h>

class SpaceTreeView;
class SpacePanel;

// Application class
class WheelApp : public wxApp
{
public:
    virtual bool OnInit() override;
};

// Main frame
class WheelFrame : public wxFrame
{
public:
    WheelFrame();
    ~WheelFrame();

private:
    // Menu handlers
    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileSave(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);
    void OnEditNodeProperties(wxCommandEvent& event);
    void OnEditSpaceProperties(wxCommandEvent& event);
    void OnViewResetView(wxCommandEvent& event);
    void OnViewRunLayout(wxCommandEvent& event);
    void OnHelpAbout(wxCommandEvent& event);

    void CreateMenuBar();
    void SetupStatusBar();
    void UpdateStatusBar();

    // The data model
    CSpace m_space;

    // UI components
    wxSplitterWindow* m_splitter;
    SpaceTreeView* m_treeView;
    SpacePanel* m_spacePanel;

    enum {
        ID_FILE_NEW = wxID_NEW,
        ID_FILE_OPEN = wxID_OPEN,
        ID_FILE_SAVE = wxID_SAVE,
        ID_FILE_EXIT = wxID_EXIT,
        ID_EDIT_NODE_PROPS = 2000,
        ID_EDIT_SPACE_PROPS,
        ID_VIEW_RESET,
        ID_VIEW_RUN_LAYOUT,
        ID_HELP_ABOUT = wxID_ABOUT,
    };

    wxDECLARE_EVENT_TABLE();
};
