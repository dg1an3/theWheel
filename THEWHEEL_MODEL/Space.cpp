//////////////////////////////////////////////////////////////////////
// Space.cpp: implementation of the CSpace class.
//
// Copyright (C) 1999-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "Space.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpace construction/destruction
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpace::CSpace
// 
// constructs an empty CSpace object.  use OnNewDocument to initialize
//////////////////////////////////////////////////////////////////////
CSpace::CSpace()
{
}

//////////////////////////////////////////////////////////////////////
// CSpace::~CSpace
// 
// deletes the CSpace object
//////////////////////////////////////////////////////////////////////
CSpace::~CSpace()
{
}

/////////////////////////////////////////////////////////////////////////////
// implements CSpace's dynamic creation
/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSpace, CDocument)

//////////////////////////////////////////////////////////////////////
// CSpace::NormalizeNodes
// 
// normalizes the nodes to a known sum
//////////////////////////////////////////////////////////////////////
void CSpace::NormalizeNodes(double sum)
{
	// compute the scale factor for normalization
	double scale = sum 
		/ rootNode.GetDescendantActivation();

	// normalize to equal sum
	rootNode.ScaleDescendantActivation(scale);
}

/////////////////////////////////////////////////////////////////////////////
// CSpace message map
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSpace, CDocument)
	//{{AFX_MSG_MAP(CSpace)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// CSpace::OnNewDocument
// 
// initializes a new document
//////////////////////////////////////////////////////////////////////
BOOL CSpace::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// remove all nodes
	// TODO: ensure that this really deletes all children
	rootNode.children.RemoveAll();

	// set the name of the root node
	rootNode.name.Set("root");

	// add random children to the root node
	AddChildren(&rootNode, 3, 3);

	// initialize the node activations from the root node
	rootNode.SetActivation(0.5);
	rootNode.ResetForPropagation();
	rootNode.PropagateActivation(0.8);
	NormalizeNodes();

	// everything OK, return TRUE
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSpace::AddChildren
// 
// helper function to add random children to the CSpace object
//////////////////////////////////////////////////////////////////////
void CSpace::AddChildren(CNode *pParent, int nLevels, 
				 int nCount, float weight)
{
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		CString strChildName;
		strChildName.Format("%s%s%d", 
			pParent->name.Get(), "->", nAt+1);
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

//////////////////////////////////////////////////////////////////////
// CSpace::Serialize
// 
// reads/writes the CSpace to an archive
//////////////////////////////////////////////////////////////////////
void CSpace::Serialize(CArchive& ar)
{
	// just serialize the root node
	rootNode.Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CSpace diagnostics

#ifdef _DEBUG
//////////////////////////////////////////////////////////////////////
// CSpace::AssertValid
// 
// ensures the validity of the CSpace object
//////////////////////////////////////////////////////////////////////
void CSpace::AssertValid() const
{
	CDocument::AssertValid();
}

//////////////////////////////////////////////////////////////////////
// CSpace::Dump
// 
// outputs the CSpace to the dump context
//////////////////////////////////////////////////////////////////////
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
