//////////////////////////////////////////////////////////////////////
// Space.cpp: implementation of the CSpace class.
//
// Copyright (C) 1999-2001	
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

#include <math.h>
#include <float.h>

// the class definition
#include "Space.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// the scale factor for normalization
const REAL PRIM_SEC_RATIO = 2.0;		// primary scale / secondary scale


//////////////////////////////////////////////////////////////////////
// CompareNodeActivations
// 
// comparison function for sorting nodes by activation, descending
//////////////////////////////////////////////////////////////////////
int __cdecl CompareNodeActivations(const void *elem1, const void *elem2)
{
	CNode *pNode1 = *(CNode**) elem1;
	CNode *pNode2 = *(CNode**) elem2;

	return 1000.0 * (pNode2->GetActivation() - pNode1->GetActivation());

}	// CompareNodeActivations


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
		m_pDS(NULL),
		m_pLayoutManager(NULL)
{
	// create the layout manager
	m_pLayoutManager = new CSpaceLayoutManager(this);

}	// CSpace::CSpace


//////////////////////////////////////////////////////////////////////
// CSpace::~CSpace
// 
// deletes the CSpace object
//////////////////////////////////////////////////////////////////////
CSpace::~CSpace()
{
	// delete the layout manager
	delete m_pLayoutManager;

	// delete the root node (deletes other nodes)
	delete m_pRootNode;

}	// CSpace::~CSpace


/////////////////////////////////////////////////////////////////////////////
// implements CSpace's dynamic creation
/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSpace, CDocument)


//////////////////////////////////////////////////////////////////////
// CSpace::GetRootNode
// 
// returns the root node for the space
//////////////////////////////////////////////////////////////////////
CNode *CSpace::GetRootNode()
{
	return m_pRootNode;

}	// CSpace::GetRootNode


//////////////////////////////////////////////////////////////////////
// CSpace::GetNodeCount
// 
// returns the number of nodes in the space
//////////////////////////////////////////////////////////////////////
int CSpace::GetNodeCount() const
{
	return m_arrNodes.GetSize();

}	// CSpace::GetNodeCount


//////////////////////////////////////////////////////////////////////
// CSpace::GetNodeAt
// 
// returns a node by index
//////////////////////////////////////////////////////////////////////
CNode *CSpace::GetNodeAt(int nAt) const
{
	return (CNode *) m_arrNodes.GetAt(nAt);

}	// CSpace::GetNodeAt


//////////////////////////////////////////////////////////////////////
// CSpace::AddNode
// 
// adds a new node as the child of the specified node, with the 
//		indicated parent
//////////////////////////////////////////////////////////////////////
void CSpace::AddNode(CNode *pNewNode, CNode *pParentNode)
{
	// determine the initial activation
	REAL actWeight = 
		0.5f * (1.4f - 0.8f * (REAL) rand() / (REAL) RAND_MAX);

	// set up the link
	pNewNode->LinkTo(pParentNode, (REAL) actWeight);

	// add the new node to the parent
	pNewNode->SetParent(pParentNode);

	// set the activation for the new node
	pNewNode->SetActivation(actWeight * 0.1f);

	// add the node to the array
	AddNodeToArray(pNewNode);

	// update all the views, passing the new node as a hint
	UpdateAllViews(NULL, NULL, pNewNode);	

}	// CSpace::AddNode


//////////////////////////////////////////////////////////////////////
// CSpace::ActivateNode
// 
// activates a particular node
//////////////////////////////////////////////////////////////////////
void CSpace::ActivateNode(CNode *pNode, REAL scale)
{
	// first, compute the new activation of the node up to the max
	REAL oldActivation = pNode->GetActivation();
	REAL newActivation = 
		oldActivation + (TOTAL_ACTIVATION * (REAL) 0.60 - oldActivation) 
			* scale;

	// set the node's new activation
	pNode->SetActivation(newActivation);

	// now, propagate the activation
	m_pRootNode->ResetForPropagation();
	pNode->PropagateActivation((REAL) 0.4);

	// normalize the nodes
	NormalizeNodes();

	// update total activation
	m_totalActivation = GetRootNode()->GetDescendantActivation();

	// sort the nodes
	SortNodes();

}	// CSpace::ActivateNode


//////////////////////////////////////////////////////////////////////
// CSpace::NormalizeNodes
// 
// normalizes the nodes to a known sum
//////////////////////////////////////////////////////////////////////
void CSpace::NormalizeNodes(REAL sum)
{
	// scale for secondary
	REAL scale_2 = sum 
		/ (PRIM_SEC_RATIO * m_pRootNode->GetDescendantPrimaryActivation()
			+ m_pRootNode->GetDescendantSecondaryActivation());

	// scale for primary
	REAL scale_1 = scale_2 * PRIM_SEC_RATIO;

	// scale for equal primary/secondary weighting
	REAL scale = sum / m_pRootNode->GetDescendantActivation();

	// normalize to equal sum
	m_pRootNode->ScaleDescendantActivation(scale_1, scale_2);

}	// CSpace::NormalizeNodes


//////////////////////////////////////////////////////////////////////
// CSpace::GetSuperNodeCount
// 
// returns the number of super nodes
//////////////////////////////////////////////////////////////////////
int CSpace::GetSuperNodeCount()
{
	return __min(GetLayoutManager()->GetStateDim() / 2, // m_nSuperNodeCount, 
		GetNodeCount());

}	// CSpace::GetSuperNodeCount


//////////////////////////////////////////////////////////////////////
// CSpace::SetMaxSuperNodeCount
// 
// sets the maximum number of super nodes
//////////////////////////////////////////////////////////////////////
void CSpace::SetMaxSuperNodeCount(int nSuperNodeCount)
{
	// m_nSuperNodeCount = nSuperNodeCount;
	GetLayoutManager()->SetStateDim(nSuperNodeCount * 2);

}	// CSpace::SetMaxSuperNodeCount


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
	m_pRootNode->SetName("root");
	AddNode(new CNode(this, "Child1"), m_pRootNode);
	AddNode(new CNode(this, "Child2"), m_pRootNode);
	AddNode(new CNode(this, "Child3"), m_pRootNode);

#ifdef USE_FAKE_NODES
	m_pRootNode->SetDescription("Eadf eeqrjij afga gdfijagg ahvuert8qu4 vadfgahg."
		"Jkdjfwheu sdfg hahrewgakdjf asg hag7un34gafasdgha vhg haeirnga."
		"Sdff jdf jdskljfa; lkdjfsjd fkjweu iagh eurafgnls uashfre.");

	for (int nAt = 0; nAt < 2173; nAt++)
		rand();

	// add random children to the root node
	AddChildren(&rootNode, 4, 3);
	CrossLinkNodes(m_pRootNode->GetDescendantCount() / 50);
#endif

	AddNodeToArray(m_pRootNode);

	// initialize the node activations from the root node
	m_pRootNode->SetActivation((REAL) 0.5);
	m_pRootNode->ResetForPropagation();
	m_pRootNode->PropagateActivation((REAL) 0.8);
	NormalizeNodes();

	// everything OK, return TRUE
	return TRUE;

}	// CSpace::OnNewDocument


//////////////////////////////////////////////////////////////////////
// CSpace::SortNodes
// 
// sorts the nodes by activation value
//////////////////////////////////////////////////////////////////////
void CSpace::SortNodes()
{
	qsort(m_arrNodes.GetData(), m_arrNodes.GetSize(), sizeof(CNode*),
		CompareNodeActivations);

}	// CSpace::SortNodes


//////////////////////////////////////////////////////////////////////
// CSpace::AddChildren
// 
// helper function to add random children to the CSpace object
//////////////////////////////////////////////////////////////////////
void CSpace::AddChildren(CNode *pParent, int nLevels, 
				 int nCount, REAL weight)
{
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		CString strChildName;
		strChildName.Format("%s%s%d", 
			pParent->GetName(), "->", nAt+1);
		CNode *pChild = new CNode(this, strChildName);

		pChild->SetDescription("Ud dfjaskf rtertj 23 adsjf.  "
			"Lkdjfsdfj sdaf ajskjgew ajg ajsdklgj; slrj jagifj ajdfgjkld.  "
			"I d fdmgj sdl jdsgiow mrektmrejgj migmoergmmsos m ksdfogkf.");
		// pParent->m_arrChildren.Add(pChild);
		pChild->SetParent(pParent);

		// set the image filename
		char pszImageFilename[_MAX_FNAME];
		sprintf(pszImageFilename, "image%i.bmp", rand() % 8);
		pChild->SetImageFilename(pszImageFilename);

		// generate a randomly varying weight
		REAL actWeight = weight * (1.4f - 0.8f * (REAL) rand() / (REAL) RAND_MAX);
		pChild->LinkTo(pParent, actWeight);

		if (nLevels > 0)
			AddChildren(pChild, nLevels-1, nCount, weight);
	}

	// now sort the links
	pParent->SortLinks();	

}	// CSpace::AddChildren


//////////////////////////////////////////////////////////////////////
// CSpace::CrossLinkNodes
// 
// randomly cross-links some nodes
//////////////////////////////////////////////////////////////////////
void CSpace::CrossLinkNodes(int nCount, REAL weight)
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
			REAL actWeight = weight * (1.4f - 0.8f * (REAL) rand() / (REAL) RAND_MAX);

			TRACE("Linking child %s to child %s with weight %lf\n",
				pChild1->GetName(),
				pChild2->GetName(), 
				actWeight);

			pChild1->LinkTo(pChild2, weight);
		}
	}

}	// CSpace::CrossLinkNodes


//////////////////////////////////////////////////////////////////////
// CSpace::AddNodeToArray
// 
// adds a node (and its children) to the node array
//////////////////////////////////////////////////////////////////////
void CSpace::AddNodeToArray(CNode *pNode)
{
	// set the pointer to the space
	pNode->m_pSpace = this;

	// add to the array
	m_arrNodes.Add(pNode);

	// add the children
	for (int nAt = 0; nAt < pNode->CNode::GetChildCount(); nAt++)
	{
		AddNodeToArray(pNode->CNode::GetChildAt(nAt));
	}

}	// CSpace::AddNodeToArray


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

		// remove existing nodes from the array
		m_arrNodes.RemoveAll();

		// add the root node to the array of nodes
		AddNodeToArray(m_pRootNode);

		// and do some activation
		ActivateNode(m_pRootNode, 0.5);
	}
	else
	{
		// just serialize the root node pointer
		ar << m_pRootNode;
	}

}	// CSpace::Serialize


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
/*	for (int nAt = 0; nAt < GetNodeCount()-1; nAt++)
	{
		ASSERT(GetNodeAt(nAt)->GetActivation() >= 
			GetNodeAt(nAt+1)->GetActivation());
	}
*/
	CDocument::AssertValid();

}	// CSpace::AssertValid


//////////////////////////////////////////////////////////////////////
// CSpace::Dump
// 
// outputs the CSpace to the dump context
//////////////////////////////////////////////////////////////////////
void CSpace::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);

	for (int nAt = 0; nAt < m_pRootNode->GetChildCount(); nAt++)
	{
		const CNode *pNode = m_pRootNode->GetChildAt(nAt);
		for (int nAtLink = 0; nAtLink < pNode->GetLinkCount(); nAtLink++)
		{
			const CNodeLink *pLink = pNode->GetLinkAt(nAtLink);
			const CNode *pLinkedNode = pLink->GetTarget();
			dc  << "Link " << pNode->GetName() 
				<< " to " << pLinkedNode->GetName() 
				<< " : " << pLink->GetWeight()
				<< "\n";
		}
	}

}	// CSpace::Dump

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpace commands


//////////////////////////////////////////////////////////////////////
// CSpace::GetTotalActivation
// 
// returns the total activation of all nodes
//////////////////////////////////////////////////////////////////////
REAL CSpace::GetTotalActivation() const
{
	return m_totalActivation;

}	// CSpace::GetTotalActivation


//////////////////////////////////////////////////////////////////////
// CSpace::GetLayoutManager
// 
// returns a pointer to the layout manager
//////////////////////////////////////////////////////////////////////
CSpaceLayoutManager *CSpace::GetLayoutManager()
{
	return m_pLayoutManager;

}	// CSpace::GetLayoutManager


//////////////////////////////////////////////////////////////////////
// CSpace::LayoutNodes
// 
// lays out the nodes in the array
//////////////////////////////////////////////////////////////////////
void CSpace::LayoutNodes()
{
	// layout the nodes
	m_pLayoutManager->LayoutNodes();

}	// CSpace::LayoutNodes


//////////////////////////////////////////////////////////////////////
// CSpace::GetDirectSound
// 
// returns the DirectSound object for the space
//////////////////////////////////////////////////////////////////////
LPDIRECTSOUND CSpace::GetDirectSound()
{
	if (NULL == m_pDS)
	{
		if (DS_OK != DirectSoundCreate(NULL, &m_pDS, NULL))
		{
			m_pDS = NULL;
		}
	}

	return m_pDS;

}	// CSpace::GetDirectSound
