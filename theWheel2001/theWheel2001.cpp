//////////////////////////////////////////////////////////////////////
// theWheel2001.cpp: implementation of the CtheWheelApp class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// main include
#include "theWheel2001.h"

#include "MainFrm.h"
#include "Space.h"
#include "SpaceTreeView.h"
#include "SpaceView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CtheWheelApp construction

//////////////////////////////////////////////////////////////////////
// CtheWheelApp::CtheWheelApp
// 
// constructs a new instance of the application object 
//////////////////////////////////////////////////////////////////////
CtheWheelApp::CtheWheelApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CtheWheelApp object

CtheWheelApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CtheWheelApp initialization

//////////////////////////////////////////////////////////////////////
// CtheWheelApp::InitInstance
// 
// constructs a new instance of the application object 
//////////////////////////////////////////////////////////////////////
//   GdiplusStartupInput gdiplusStartupInput;
//   ULONG_PTR           gdiplusToken;

BOOL CtheWheelApp::InitInstance()
{
	CString strLogFilename;
	::GetTempPath(_MAX_PATH, strLogFilename.GetBuffer(_MAX_PATH));

	AfxEnableControlContainer();

	// initialize the COM support application
	HRESULT hr;
	if (FAILED(hr = CoInitialize(NULL))) 
	{ 
		return -1; 
	}  

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	   // Initialize GDI+.
	// GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSpace),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CSpaceTreeView));
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	// WriteProfileInt("Settings", "InitWindowState", SW_MINIMIZE);
	int nShowState = GetProfileInt("Settings", "InitWindowState", SW_SHOW);
	m_pMainWnd->ShowWindow(nShowState);
	m_pMainWnd->UpdateWindow();

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CtheWheelApp Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CtheWheelApp, CWinApp)
	//{{AFX_MSG_MAP(CtheWheelApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// class CAboutDlg
//
// dialog used for App About
//////////////////////////////////////////////////////////////////////
class CAboutDlg : public CDialog
{
public:
	// Construction/Destruction
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////
// CAboutDlg::CAboutDlg
// 
// constructs a new CAboutDlg
//////////////////////////////////////////////////////////////////////
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

//////////////////////////////////////////////////////////////////////
// CAboutDlg::DoDataExchange
// 
// performs data exchange for the about dialog
//////////////////////////////////////////////////////////////////////
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CtheWheelApp message handlers

//////////////////////////////////////////////////////////////////////
// CtheWheelApp::OnAppAbout
// 
// Menu command to display the about dialog
//////////////////////////////////////////////////////////////////////
void CtheWheelApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


int CtheWheelApp::ExitInstance() 
{
	// shut down COM support
	CoUninitialize(); 
	
	return CWinApp::ExitInstance();
}
