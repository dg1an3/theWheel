// Space.cpp : implementation of the CSpace class
//

#include "stdafx.h"
// #include "theWheel2001.h"

#include "Space.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpace

IMPLEMENT_DYNCREATE(CSpace, CDocument)

BEGIN_MESSAGE_MAP(CSpace, CDocument)
	//{{AFX_MSG_MAP(CSpace)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpace construction/destruction

CSpace::CSpace()
{
	// TODO: add one-time construction code here

}

CSpace::~CSpace()
{
}

BOOL CSpace::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// remove all nodes
	rootNode.children.RemoveAll();

	// now populate with some dummy data
	CNode *pSongsNode = new CNode("Songs");
	rootNode.children.Add(pSongsNode);

	CNode *pBandsNode = new CNode("Bands/Singers");
	rootNode.children.Add(pBandsNode); 

	pSongsNode->LinkTo(pBandsNode, 0.3f);

	CNode *pFoodNode = new CNode("Food");
	rootNode.children.Add(pFoodNode);

	pFoodNode->LinkTo(pSongsNode, 0.2f);
	pFoodNode->LinkTo(pBandsNode, 0.2f);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSpace serialization

void CSpace::Serialize(CArchive& ar)
{
	// myNodes.Serialize(ar);

	// CObArray arrNodes;
	if (ar.IsStoring())
	{
		rootNode. // children.
			Serialize(ar);
//
//		for (int nAt = 0; nAt < myNodes.GetSize(); nAt++)
//			arrNodes.Add(myNodes.Get(nAt));
//		arrNodes.Serialize(ar);
	}
	else
	{
		rootNode.children.Serialize(ar);
//		arrNodes.Serialize(ar);
//		for (int nAt = 0; nAt < arrNodes.GetSize(); nAt++)
//			myNodes.Add((CNode *)arrNodes.GetAt(nAt));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpace diagnostics

#ifdef _DEBUG
void CSpace::AssertValid() const
{
	CDocument::AssertValid();
}

void CSpace::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);

	for (int nAt = 0; nAt < rootNode.children.GetSize(); nAt++)
	{
		const CNode *pNode = (const CNode *) rootNode.children.Get(nAt);
		for (int nAtLink = 0; nAtLink < pNode->links.GetSize(); nAtLink++)
		{
			const CNodeLink *pLink = pNode->links.Get(nAtLink);
			const CNode *pLinkedNode = pLink->forTarget.Get();
			dc  << "Link " << pNode->name.Get() 
				<< " to " << pLinkedNode->name.Get() 
				<< " : " << pLink->weight.Get()
				<< "\n";
		}
	}
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpace commands
