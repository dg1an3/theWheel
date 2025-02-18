//////////////////////////////////////////////////////////////////////
// Space.cpp: implementation of the CSpace class.
//
// Copyright (C) 1999-2002 Derek G Lane	
// $Id: Space.cpp,v 1.44 2007/07/05 00:12:56 Derek Lane Exp $
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
const REAL PRIM_NORM_SCALE = 0.020 * 3.5; // 3.5; // 0.050 * 3.5;		
const REAL SEC_NORM_SCALE = 0.060 * 3.5; // 3.5;

// constants for runaway adjustment
const REAL MAX_RUNAWAY_RADIUS = 500.0;
const REAL RUNAWAY_DIVISOR = 3.0;

// constant for sub-threshold decay toward center
const REAL SUBNODE_DECAY_RATE = 0.9;

// min and max for randomly assigning initial weights
const REAL MIN_INITIAL_WGT = 0.4;
const REAL MAX_INITIAL_WGT = 0.7;

const REAL TOTAL_ACTIVATION_FRACTION = // 2.0; // 
	0.40;
	// 0.20;

const REAL DEFAULT_SPRING_CONST = 0.95;

const REAL PROPAGATE_SCALE = 0.40; // 0.25; // 0.40; // 0.80; // 1.0;
const REAL PROPAGATE_ALPHA = 0.99; // 1.00; // 0.90; // 0.98;

//////////////////////////////////////////////////////////////////////
REAL 
	GenerateRandom(REAL min, REAL max)
	// generates a random number in the specified range
{
	REAL value = min;
	value += (max - min) * (REAL) rand() / (REAL) RAND_MAX;

	return value;

}	// GenerateRandom


//////////////////////////////////////////////////////////////////////
// Event Firing
//////////////////////////////////////////////////////////////////////
#define SPACE_FIRE_CHANGE(TAG, NODE) UpdateAllViews(NULL, TAG, NODE); 

/////////////////////////////////////////////////////////////////////////////
// CSpace construction/destruction
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CSpace::CSpace()
	// constructs an empty CSpace object.  use OnNewDocument to initialize
	: m_pRootNode(NULL)
	, m_bNodesSorted(FALSE)

	, m_pLayoutManager(NULL)
	, m_nLastPostSuper(0)

	, m_pCurrentNode(NULL)

	, m_totalPrimaryActivation(0.0)
	, m_totalSecondaryActivation(0.0)

	, m_SpringConst(DEFAULT_SPRING_CONST)
{
	// construct layout manager
	m_pLayoutManager = new CSpaceLayoutManager(this);

	// set the spring constant (from the profile)
	SetSpringConst( 1.0 / 100.0 * 
		(REAL) ::AfxGetApp()->GetProfileInt(_T("LAYOUT"), _T("SPRING_CONST"), 
			Round<int>(100.0 * GetSpringConst())));

}	// CSpace::CSpace


//////////////////////////////////////////////////////////////////////
CSpace::~CSpace()
	// deletes the CSpace object
{
	// delete the layout manager
	delete m_pLayoutManager;

	// delete the root node (deletes other nodes)
	if (m_pRootNode)
	{
		delete m_pRootNode;
	}

}	// CSpace::~CSpace


/////////////////////////////////////////////////////////////////////////////
// implements CSpace's dynamic creation
/////////////////////////////////////////////////////////////////////////////
#define SPACE_SCHEMA 3
IMPLEMENT_SERIAL(CSpace, CDocument, VERSIONABLE_SCHEMA | SPACE_SCHEMA)
//	3 - optimization parameters
//	2 - serialize class color map + root node
//	1 - serialize root node


//////////////////////////////////////////////////////////////////////
int 
	CSpace::GetNodeCount() const
	// returns the number of nodes in the space
{
#ifdef STL_COLL_SPACE_NODES
	return (int) m_arrNodes.size(); 
#else
	return (int) m_arrNodes.GetCount(); 
#endif


}	// CSpace::GetNodeCount


//////////////////////////////////////////////////////////////////////
CNode *
	CSpace::GetNodeAt(int nAt)
	// returns a node by index
{
#ifdef STL_COLL_SPACE_NODES
  return m_arrNodes.at(nAt);
#else
  return m_arrNodes[nAt];
#endif


}	// CSpace::GetNodeAt


//////////////////////////////////////////////////////////////////////
void 
	CSpace::AddNode(CNode *pNewNode, CNode *pParentNode)
	// adds a new node as the child of the specified node, with the 
	//		indicated parent
{
	// determine the initial activation
	REAL actWeight = GenerateRandom(MIN_INITIAL_WGT, MAX_INITIAL_WGT);

	if (pParentNode)
	{
		// set up the link
		pNewNode->LinkTo(pParentNode, (REAL) actWeight);

		// add the new node to the parent
		pNewNode->SetParent(pParentNode);
	}

	// add the node to the array
	AddNodeToArray(pNewNode);

	// set the activation for the new node, only after adding to
	//		the array (because it will update the total activation)
	pNewNode->SetActivation(actWeight * 0.1);

}	// CSpace::AddNode


//////////////////////////////////////////////////////////////////////
void 
	CSpace::RemoveNode(CNode *pMarkedNode)
	// removes a particular node from the space
{
	// unlink from current nodes
#ifdef STL_COLL_SPACE_NODES
	vector<CNode*>::iterator iter;
	for (iter = m_arrNodes.begin(); iter != m_arrNodes.end(); iter++)
		(*iter)->Unlink(pMarkedNode, FALSE);
#else
  int nMarkedAt = -1;
  for (int nAt = 0; nAt < m_arrNodes.GetCount(); nAt++)
  {
    m_arrNodes[nAt]->Unlink(pMarkedNode, FALSE);
    if (m_arrNodes[nAt] == pMarkedNode)
    {
      nMarkedAt = nAt;
    }
  }
#endif

	// remove the node from the parent
	pMarkedNode->SetParent(NULL);

	// de-activate
	pMarkedNode->SetActivation(0.00001);

	// find and remove the node from the array
#ifdef STL_COLL_SPACE_NODES
	iter = remove(m_arrNodes.begin(), m_arrNodes.end(), pMarkedNode);
	m_arrNodes.erase(iter, m_arrNodes.end());
#else
  if (nMarkedAt > -1)
  {
    m_arrNodes.RemoveAt(nMarkedAt);
  }
#endif

}	// CSpace::RemoveNode


//////////////////////////////////////////////////////////////////////
void 
	CSpace::SetCurrentNode(CNode *pNode)
	// sets the space's current node
{
	m_pCurrentNode = pNode;

}	// CSpace::SetCurrentNode


//////////////////////////////////////////////////////////////////////
void 
	CSpace::ActivateNode(CNode *pNode, REAL scale)
	// activates a particular node
{
	// first, compute the new activation of the node up to the max
	REAL oldActivation = pNode->GetActivation();
	REAL newActivation = oldActivation // + scale;
		+ scale * (TOTAL_ACTIVATION * TOTAL_ACTIVATION_FRACTION - oldActivation);

	// set the node's new activation
	pNode->SetActivation(newActivation);

	// now, propagate the activation
	m_pRootNode->ResetForPropagation();

	// now propagate
	/// TODO: use this instead of SetActivation
	pNode->PropagateActivation(NULL, 0.0, PROPAGATE_SCALE, PROPAGATE_ALPHA);

	// now update the activation for all nodes
	m_pRootNode->UpdateFromNewActivation();

	// normalize the nodes
	NormalizeNodes();

	// flag the nodes as unsorted
	m_bNodesSorted = FALSE;

}	// CSpace::ActivateNode


//////////////////////////////////////////////////////////////////////
void 
	CSpace::NormalizeNodes(REAL sum)
	// normalizes the nodes to a known sum
{
	REAL totalActivation = GetTotalActivation(true);
	REAL diffFrac = (sum / totalActivation - 1.0);
	if (diffFrac > 0.0) diffFrac = 0.0;

#ifdef STL_COLL_SPACE_NODES
	// scale the nodes
	vector<CNode*>::iterator iter;
	for (iter = m_arrNodes.begin(); iter != m_arrNodes.end(); iter++)
	{
		// scale this node's activation
		m_totalPrimaryActivation -= (*iter)->m_primaryActivation;
		(*iter)->m_primaryActivation += (*iter)->m_primaryActivation * diffFrac * PRIM_NORM_SCALE;
		m_totalPrimaryActivation += (*iter)->m_primaryActivation;

		m_totalSecondaryActivation -= (*iter)->m_secondaryActivation;
		(*iter)->m_secondaryActivation += (*iter)->m_secondaryActivation * diffFrac * SEC_NORM_SCALE;
		m_totalSecondaryActivation += (*iter)->m_secondaryActivation;
	}
#else
	// scale the nodes
	for (int nAt = 0; nAt < m_arrNodes.GetCount(); nAt++)
	{
    CNode *pNode = m_arrNodes[nAt];

		// scale this node's activation
		m_totalPrimaryActivation -= pNode->m_primaryActivation;
		pNode->m_primaryActivation += pNode->m_primaryActivation * diffFrac * PRIM_NORM_SCALE;
		m_totalPrimaryActivation += pNode->m_primaryActivation;

		m_totalSecondaryActivation -= pNode->m_secondaryActivation;
		pNode->m_secondaryActivation += pNode->m_secondaryActivation * diffFrac * SEC_NORM_SCALE;
		m_totalSecondaryActivation += pNode->m_secondaryActivation;
	}
#endif

}	// CSpace::NormalizeNodes


//////////////////////////////////////////////////////////////////////
CMap<CString, LPCTSTR, COLORREF, COLORREF>& 
	CSpace::GetClassColorMap()
	// class description accessors
{
	return m_mapClassColors;

}	// CSpace::GetClassColorMap


//////////////////////////////////////////////////////////////////////
BOOL 
	CSpace::CreateSimpleSpace()
	// initializes a new document
{
	DeleteContents();

	// clear the array
	m_totalPrimaryActivation = 0.0;
	m_totalSecondaryActivation = 0.0;

	// create a new root node
	m_pRootNode = new CNode();
	m_pRootNode->SetName(_T("%%hiddenroot%%"));

	// create the hidden root
	CNode *pMainNode = new CNode();
	pMainNode->SetName(_T("root"));
	pMainNode->SetParent(m_pRootNode);
	AddNode(pMainNode, NULL);

	auto child1 = new CNode(this, _T("Child1"));
	AddNode(child1, pMainNode);
	AddNode(new CNode(this, _T("Child2")), pMainNode);
	AddNode(new CNode(this, _T("Child3")), pMainNode);

	AddNode(new CNode(this, _T("Child4")), child1);
	auto child5 = new CNode(this, _T("Child5"));
	AddNode(child5, child1);

	auto child6 = new CNode(this, _T("Child6"));
	AddNode(child6, child1);

	AddNode(new CNode(this, _T("Child7")), child6);
	AddNode(new CNode(this, _T("Child8")), child6);
	AddNode(new CNode(this, _T("Child81")), child6);
	AddNode(new CNode(this, _T("Child82")), child6);

	AddNode(new CNode(this, _T("Child9")), child5);
	AddNode(new CNode(this, _T("Child10")), child5);
	AddNode(new CNode(this, _T("Child9")), child5);
	AddNode(new CNode(this, _T("Child10")), child5);
	AddNode(new CNode(this, _T("Child9")), child5);
	AddNode(new CNode(this, _T("Child10")), child5);

	// initialize the node activations from the root node
	pMainNode->SetActivation((REAL) 0.5);
	pMainNode->ResetForPropagation();
	pMainNode->PropagateActivation(NULL, 0.0, (REAL) 0.2, // (REAL) 0.5, 
		(REAL) 0.9);
	// TODO: params

	SortNodes();
	NormalizeNodes();

	// everything OK, return TRUE
	return TRUE;

}	// CSpace::CreateSimpleSpace

//////////////////////////////////////////////////////////////////////
int NodeSortByActivation(const void *pLeft, const void *pRight)
{
  CNode *pNodeL = *(CNode**) pLeft;
	REAL act1 = pNodeL->GetActivation();
	// now add additional factor for post-super count: max 40 adds 0.1 to activation
	act1 += 0.1 * (REAL) pNodeL->GetPostSuperCount() / 40.0;

  CNode *pNodeR = *(CNode**) pRight;
	REAL act2 = pNodeR->GetActivation();
	act2 += 0.1 * (REAL) pNodeR->GetPostSuperCount() / 40.0;

  return (act1 > act2) ? -1 : 1;
}


//////////////////////////////////////////////////////////////////////
void 
	CSpace::SortNodes()
	// sorts the nodes by activation value
{
	// if nodes are sorted, leave alone
	if (!m_bNodesSorted)
	{	
#ifdef STL_COLL_SPACE_NODES
		// sort(m_arrNodes.begin(), m_arrNodes.end(), mem_fun1(&CNode::IsActivationGreater));
    sort(&m_arrNodes[0], &m_arrNodes[0]+m_arrNodes.size(), 
      &CNode::IsActivationGreaterStatic);
#else
    //qsort(&m_arrNodes[0], m_arrNodes.GetCount(), sizeof(CNode*), 
    //  &NodeSortByActivation);
    sort(&m_arrNodes[0], &m_arrNodes[0]+m_arrNodes.GetCount(), 
      &CNode::IsActivationGreaterStatic);
#endif

		// flag as sorted
		m_bNodesSorted = TRUE;
	}

}	// CSpace::SortNodes


//////////////////////////////////////////////////////////////////////
void 
	CSpace::AddNodeToArray(CNode *pNode)
	// adds a node (and its children) to the node array
{
	// set the pointer to the space
	pNode->m_pSpace = this;
	pNode->m_pSpace->m_totalPrimaryActivation += pNode->GetPrimaryActivation();
	pNode->m_pSpace->m_totalSecondaryActivation += pNode->GetSecondaryActivation();

	// add to the array
#ifdef STL_COLL_SPACE_NODES
  m_arrNodes.push_back(pNode);
#else
  m_arrNodes.Add(pNode);
#endif

	// add the children
	for (int nAt = 0; nAt < pNode->GetChildCount(); nAt++)
	{
		AddNodeToArray(pNode->GetChildAt(nAt));
	}

	// flag that the nodes need to be sorted
	m_bNodesSorted = FALSE;

}	// CSpace::AddNodeToArray


//////////////////////////////////////////////////////////////////////
void 
	CSpace::DeleteContents()
	// removes all nodes
{
	// remove existing nodes from the array
#ifdef STL_COLL_SPACE_NODES
	m_arrNodes.clear();
#else
	m_arrNodes.RemoveAll();
#endif


	// delete the current root node
	delete m_pRootNode;
	m_pRootNode = NULL;

	// clear the class color map
	m_mapClassColors.RemoveAll();

}	// CSpace::DeleteContents


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
#ifdef CHECK_SORTING
	for (int nAt = 0; nAt < GetNodeCount()-1; nAt++)
	{
		ASSERT(GetNodeAt(nAt)->GetActivation() >= 
			GetNodeAt(nAt+1)->GetActivation());
	}
#endif

	CObject::AssertValid();

}	// CSpace::AssertValid


//////////////////////////////////////////////////////////////////////
// CSpace::Dump
// 
// outputs the CSpace to the dump context
//////////////////////////////////////////////////////////////////////
void CSpace::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	for (int nAt = 0; nAt < m_pRootNode->GetChildCount(); nAt++)
	{
		CNode *pNode = m_pRootNode->GetChildAt(nAt);
		for (int nAtLink = 0; nAtLink < pNode->GetLinkCount(); nAtLink++)
		{
			CNodeLink *pLink = pNode->GetLinkAt(nAtLink);
			CNode *pLinkedNode = pLink->GetTarget();
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
REAL
	CSpace::GetTotalActivation(BOOL bCompute)
	// returns the total activation of all nodes

{
	return GetTotalPrimaryActivation(bCompute)
		+ GetTotalSecondaryActivation(bCompute);

}	// CSpace::GetTotalActivation


//////////////////////////////////////////////////////////////////////
REAL 
	CSpace::GetTotalPrimaryActivation(BOOL bCompute)
	// returns the total primary activation of all nodes
{
	// are we recomputing?
	if (bCompute)
	{
		// reset total
		REAL m_totalPrimaryActivation = 0.0;

		// sum all secondary activations
#ifdef STL_COLL_SPACE_NODES
		vector<CNode*>::iterator iterNode;
		for (iterNode = m_arrNodes.begin(); iterNode != m_arrNodes.end(); iterNode++)
			m_totalPrimaryActivation += (*iterNode)->GetPrimaryActivation();
#else
		for (int nAt = 0; nAt < m_arrNodes.GetCount(); nAt++)
    {
		  CNode *pNode = m_arrNodes[nAt];
      m_totalPrimaryActivation += pNode->GetPrimaryActivation();
    }
#endif

  }

	return m_totalPrimaryActivation;

}	// CSpace::GetTotalPrimaryActivation


//////////////////////////////////////////////////////////////////////
REAL 
	CSpace::GetTotalSecondaryActivation(BOOL bCompute)
	// returns the total secondary activation of all nodes
{
	// are we recomputing?
	if (bCompute)
	{
		// reset total
		m_totalSecondaryActivation = 0.0;

		// sum all secondary activations
#ifdef STL_COLL_SPACE_NODES
		vector<CNode*>::iterator iterNode;
		for (iterNode = m_arrNodes.begin(); iterNode != m_arrNodes.end(); iterNode++)
      m_totalSecondaryActivation += (*iterNode)->GetSecondaryActivation();
#else
		for (int nAt = 0; nAt < m_arrNodes.GetCount(); nAt++)
    {
		  CNode *pNode = m_arrNodes[nAt];
      m_totalSecondaryActivation += pNode->GetSecondaryActivation();
    }
#endif
	}

	return m_totalSecondaryActivation;

}	// CSpace::GetTotalSecondaryActivation


//////////////////////////////////////////////////////////////////////
// CSpace::Serialize
// 
// reads/writes the CSpace to an archive
//////////////////////////////////////////////////////////////////////
void CSpace::Serialize(CArchive& ar)
{
	// set up the schema
	DWORD dwSchema = SPACE_SCHEMA;

	// if we are loading the space...
	if (ar.IsLoading())
	{
		// clear all content
		DeleteContents();

		// serialize schema
		// ar >> dwSchema;

		// read in the root node pointer
		ar >> m_pRootNode;

		// see if the name is the default (hidden) root name
		if (m_pRootNode->GetName() != _T("%%hiddenroot%%"))
		{
			// create the hidden root
			CNode *pHiddenRoot = new CNode();
			pHiddenRoot->SetName(_T("%%hiddenroot%%"));

			// set the parent for the default node
			m_pRootNode->SetParent(pHiddenRoot);

			// now set the root to the hidden root
			m_pRootNode = pHiddenRoot;
		}

		// remove existing nodes from the array
		m_totalPrimaryActivation = 0.0;
		m_totalSecondaryActivation = 0.0;

		// add all of the nodes (except the hidden root)
		//		to the array of nodes
		for (int nAt = 0; nAt < m_pRootNode->GetChildCount(); nAt++)
		{
			AddNodeToArray(m_pRootNode->GetChildAt(nAt));
		}

		// sort the nodes
		SortNodes();

		// normalize the nodes
		NormalizeNodes();

		// mark super-threshold nodes
		for (int nAt = 0; nAt < GetLayoutManager()->GetSuperNodeCount(); nAt++)
		{
			GetNodeAt(nAt)->SetIsSubThreshold(FALSE);
		}

		// set the total activation
		GetTotalPrimaryActivation(TRUE);
		GetTotalSecondaryActivation(TRUE);
	}
	else
	{
		// serialize schema
		// ar << (DWORD) dwSchema;

		// just serialize the root node pointer
		ar << m_pRootNode;
	}

	// super node count
	int nSuperNodeCount = 60;
	GetLayoutManager()->SetMaxSuperNodeCount(nSuperNodeCount);
	return;

	if (dwSchema >= 2)
	{
		// serialize color map
		m_mapClassColors.Serialize(ar);
	}

	if (dwSchema >= 3)
	{
		if (ar.IsLoading())
		{
			// spring constant
			double springConst;
			ar >> springConst;
			SetSpringConst(springConst);

			// primary/secondary ratio
			double primSecRatio;
			ar >> primSecRatio;
			SetPrimSecRatio(primSecRatio);

			// super node count
			int nSuperNodeCount;
			ar >> nSuperNodeCount;
			nSuperNodeCount = 100;
			GetLayoutManager()->SetMaxSuperNodeCount(nSuperNodeCount);

			// layout manager parameters
			double tolerance;
			ar >> tolerance;
			tolerance = 0.01;
			GetLayoutManager()->SetTolerance(tolerance);

			double k_pos;
			ar >> k_pos;
			GetLayoutManager()->SetKPos(k_pos);

			double k_rep;
			ar >> k_rep;
			GetLayoutManager()->SetKRep(k_rep);
		}
		else
		{
			// spring constant
			ar << (double) GetSpringConst();

			// primary/secondary ratio
			ar << (double) GetPrimSecRatio();

			// super node count
			ar << GetLayoutManager()->GetStateDim() / 2;

			// layout manager parameters
			ar << (double) GetLayoutManager()->GetTolerance();
			ar << (double) GetLayoutManager()->GetKPos();
			ar << (double) GetLayoutManager()->GetKRep();
		}
	}

}	// CSpace::Serialize


