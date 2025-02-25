// theWheelDoc.cpp : implementation of the CtheWheelDoc class
//

#include "stdafx.h"
#include "theWheel.h"

#include "theWheelDoc.h"

// #include "SpacePropagationPropPage.h"
// #include "SpaceLayoutPropPage.h"

// #include "SpaceScaleLinkWeightsDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CtheWheelDoc

IMPLEMENT_DYNCREATE(CtheWheelDoc, CDocument)

BEGIN_MESSAGE_MAP(CtheWheelDoc, CDocument)
	ON_COMMAND(32774, &CtheWheelDoc::OnScaleLinkWeights)
END_MESSAGE_MAP()


// CtheWheelDoc construction/destruction

CtheWheelDoc::CtheWheelDoc()
: m_pSpacePropSheet(NULL)
{
	//m_space.NodeAddedEvent.AddObserver(this,
	//	(ListenerFunction) &CtheWheelDoc::OnSpaceChanged);
	//m_space.NodeRemovedEvent.AddObserver(this,
	//	(ListenerFunction) &CtheWheelDoc::OnSpaceChanged);
	//m_space.NodeAttributeChangedEvent.AddObserver(this,
	//	(ListenerFunction) &CtheWheelDoc::OnSpaceChanged);
}

CtheWheelDoc::~CtheWheelDoc()
{
	delete m_pSpacePropSheet;
}


void CtheWheelDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	CDocument::SetPathName(lpszPathName, bAddToMRU);
	m_space.SetPathName(lpszPathName);
}

// called when a node is removed from the space
void CtheWheelDoc::OnSpaceChanged(/*CObservableEvent * pEvent, void * pParam*/)
{
	SetModifiedFlag(TRUE);
}

BOOL CtheWheelDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if (!m_space.CreateSimpleSpace())
		return FALSE;

	return TRUE;
}




// CtheWheelDoc serialization

void CtheWheelDoc::Serialize(CArchive& ar)
{
	CDocument::Serialize(ar);

	m_space.Serialize(ar);
}


// CtheWheelDoc diagnostics

#ifdef _DEBUG
void CtheWheelDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CtheWheelDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CtheWheelDoc commands



void CtheWheelDoc::OnScaleLinkWeights()
{
	m_space.GetRootNode()->ScaleLinkWeights(sqrtf(2.0f));
}
