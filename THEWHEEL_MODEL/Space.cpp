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
	: m_pRootNode(NULL),
		m_totalActivation(0.0)
{
}

//////////////////////////////////////////////////////////////////////
// CSpace::~CSpace
// 
// deletes the CSpace object
//////////////////////////////////////////////////////////////////////
CSpace::~CSpace()
{
	delete m_pRootNode;
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
	const double ratio = 1.2;		// primary scale / secondary scale

	// scale for secondary
	double scale_2 = sum 
		/ (ratio * m_pRootNode->GetDescendantPrimaryActivation()
			+ m_pRootNode->GetDescendantSecondaryActivation());

	// scale for primary
	double scale_1 = scale_2 * ratio;

	// scale for equal primary/secondary weighting
	double scale = sum / m_pRootNode->GetDescendantActivation();

	// normalize to equal sum
	m_pRootNode->ScaleDescendantActivation(scale_1, scale_2);

	// set the total activation for the space
	m_totalActivation = m_pRootNode->GetDescendantActivation();
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
	delete m_pRootNode;

	// create a new root node
	m_pRootNode = new CNode();
	m_pRootNode->name.Set("root");

#ifdef USE_FAKE_NODES
	m_pRootNode->description.Set("Eadf eeqrjij afga gdfijagg ahvuert8qu4 vadfgahg."
		"Jkdjfwheu sdfg hahrewgakdjf asg hag7un34gafasdgha vhg haeirnga."
		"Sdff jdf jdskljfa; lkdjfsjd fkjweu iagh eurafgnls uashfre.");

	for (int nAt = 0; nAt < 2173; nAt++)
		rand();

	// add random children to the root node
	AddChildren(&rootNode, 4, 3);
	CrossLinkNodes(m_pRootNode->GetDescendantCount() / 50);
#endif

	// initialize the node activations from the root node
	m_pRootNode->SetActivation(0.5);
	m_pRootNode->ResetForPropagation();
	m_pRootNode->PropagateActivation(0.8);
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
		CNode *pChild = new CNode(this, strChildName);

		pChild->description.Set("Ud dfjaskf rtertj 23 adsjf.  "
			"Lkdjfsdfj sdaf ajskjgew ajg ajsdklgj; slrj jagifj ajdfgjkld.  "
			"I d fdmgj sdl jdsgiow mrektmrejgj migmoergmmsos m ksdfogkf.");
		pParent->children.Add(pChild);
		pChild->parent.Set(pParent);

		// set the image filename
		char pszImageFilename[_MAX_FNAME];
		sprintf(pszImageFilename, "image%i.bmp", rand() % 8);
		pChild->imageFilename.Set(pszImageFilename);

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
// CSpace::CrossLinkNodes
// 
// randomly cross-links some nodes
//////////////////////////////////////////////////////////////////////
void CSpace::CrossLinkNodes(int nCount, float weight)
{
	// cross-link
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		// select the first random child
		CNode *pChild1 = m_pRootNode->GetRandomDescendant();

		// select the second random child
		CNode *pChild2 = m_pRootNode->GetRandomDescendant();

		if (pChild1 != pChild2)
		{
			float actWeight = weight * (1.4f - 0.8f * (float) rand() / (float) RAND_MAX);

			TRACE("Linking child %s to child %s with weight %lf\n",
				pChild1->name.Get(),
				pChild2->name.Get(), 
				actWeight);

			pChild1->LinkTo(pChild2, weight);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSpace::Serialize
// 
// reads/writes the CSpace to an archive
//////////////////////////////////////////////////////////////////////
void CSpace::Serialize(CArchive& ar)
{
	// if we are loading the space...
	if (ar.IsLoading())
	{
		// delete the current root node
		delete m_pRootNode;

		// read in the root node pointer
		ar >> m_pRootNode;
	}
	else
	{
		// just serialize the root node pointer
		ar << m_pRootNode;
	}
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

	for (int nAt = 0; nAt < m_pRootNode->children.GetSize(); nAt++)
	{
		const CNode *pNode = (const CNode *) m_pRootNode->children.Get(nAt);
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
