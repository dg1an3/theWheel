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

void AddChildren(CNode *pParent, int nLevels, 
				 int nCount = 3, float weight = 0.50f)
{
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		CString strChildName;
		strChildName.Format("%s%s%d", 
			pParent->name.Get(), "son", nAt+1);
		CNode *pChild = new CNode(strChildName);

		pParent->children.Add(pChild);
		pChild->parent.Set(pParent);

		// generate a randomly varying weight
		float actWeight = weight * (1.4f - 0.8f * (float) rand() / (float) RAND_MAX);
		pChild->LinkTo(pParent, actWeight);

		if (nLevels > 0)
			AddChildren(pChild, nLevels-1, nCount, weight);
	}

	// now sort the links
	pParent->SortLinks();	
}

BOOL CSpace::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// remove all nodes
	rootNode.children.RemoveAll();

	CNode *pNewRoot = new CNode();
	rootNode.children.Add(pNewRoot);
	pNewRoot->name.Set("root");

	// add random children to the root node
	AddChildren(pNewRoot, 3, 3);

// #define VSIM
#ifdef VSIM
	// now populate with some dummy data
	CNode *pVSIMNode = new CNode("VSIM");
	rootNode.children.Add(pVSIMNode);
	pVSIMNode->parent.Set(NULL);

	CNode *pContouringNode = new CNode("Contouring");
	pVSIMNode->children.Add(pContouringNode); 
	pContouringNode->parent.Set(pVSIMNode);
	pContouringNode->LinkTo(pVSIMNode, 0.3f);

	CNode *pIsocenterNode = new CNode("Isocenter Mgmt");
	pVSIMNode->children.Add(pIsocenterNode); 
	pIsocenterNode->parent.Set(pVSIMNode);
	pIsocenterNode->LinkTo(pVSIMNode, 0.3f);

	CNode *pBeamNode = new CNode("Beam Design");
	pVSIMNode->children.Add(pBeamNode); 
	pBeamNode->parent.Set(pVSIMNode);
	pBeamNode->LinkTo(pVSIMNode, 0.3f);
#endif

#ifdef RECREATION
	// now populate with some dummy data

	CNode *pR_B_Soul = new CNode("R&B/Soul");
	rootNode.children.Add(pR_B_Soul); 

//	CNode *pJazz = new CNode("Jazz");
//	rootNode.children.Add(pJazz);

	CNode *pGroove = new CNode("Groove");
	rootNode.children.Add(pGroove); 

	CNode *pAlternative = new CNode("Alternative");
	rootNode.children.Add(pAlternative); 

	CNode *pTechnoDance = new CNode("Techno/Dance");
	rootNode.children.Add(pTechnoDance);

	// pR_B_Soul->LinkTo(pJazz, 0.3f);
	pR_B_Soul->LinkTo(pGroove, 0.3f);
	pR_B_Soul->LinkTo(pAlternative, 0.1f);
	pR_B_Soul->LinkTo(pTechnoDance, 0.3f);

	pGroove->LinkTo(pAlternative, 0.2f);
	pGroove->LinkTo(pTechnoDance, 0.1f);

	pAlternative->LinkTo(pTechnoDance, 0.3f);

#endif

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSpace serialization

void CSpace::Serialize(CArchive& ar)
{
	// just serialize the root node
	rootNode.Serialize(ar);
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
