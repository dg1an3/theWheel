//////////////////////////////////////////////////////////////////////
// Space.cpp: implementation of the CSpace class.
//
// Copyright (C) 1999-2002 Derek G Lane	
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
		m_pLayoutManager(NULL),
		m_totalPrimaryActivation(0.0),
		m_totalSecondaryActivation(0.0),
		m_primSecRatio(PRIM_SEC_RATIO),
		m_springConst(0.95)
{
	// create the layout manager
	m_pLayoutManager = new CSpaceLayoutManager(this);

	// set the spring constant (from the profile)
	SetSpringConst( 1.0 / 100.0 * 
		(double) ::AfxGetApp()->GetProfileInt("LAYOUT", "SPRING_CONST", 
			100.0 * GetSpringConst()));

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
#define SPACE_SCHEMA 3
IMPLEMENT_SERIAL(CSpace, CDocument, VERSIONABLE_SCHEMA | SPACE_SCHEMA)
//	3 - optimization parameters
//	2 - serialize class color map + root node
//	1 - serialize root node

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
	pNewNode->SetActivation(actWeight * 0.1f);

	// update all the views, passing the new node as a hint
	UpdateAllViews(NULL, NULL, pNewNode);	

}	// CSpace::AddNode


//////////////////////////////////////////////////////////////////////
// CSpace::RemoveNode
// 
// removes a particular node from the space
//////////////////////////////////////////////////////////////////////
void CSpace::RemoveNode(CNode *pMarkedNode)
{
	// delete any links to the node
	for (int nAt = 0; nAt < GetNodeCount(); nAt++)
	{
		// get the other node
		CNode *pOtherNode = GetNodeAt(nAt);

		// if its not the node to be removed,
		if (pOtherNode != pMarkedNode)
		{
			// then unlink the marked node
			//		don't unlink reciprocal, as we would like this
			//		node to preserve its weights for graceful exit
			pOtherNode->Unlink(pMarkedNode, FALSE);
		}
	}

	// remove the node from the parent
	pMarkedNode->SetParent(NULL);

	// de-activate
	pMarkedNode->SetActivation(0.00001);

	// find and remove the node from the array
	for (nAt = 0; nAt < GetNodeCount(); nAt++)
	{
		// get the other node
		CNode *pNode = (CNode *) m_arrNodes.GetAt(nAt);

		// if its not the node to be removed,
		if (pNode == pMarkedNode)
		{
			// remove the marked node
			m_arrNodes.RemoveAt(nAt);

			// and break from the for loop
			break;
		}
	}

}	// CSpace::RemoveNode


//////////////////////////////////////////////////////////////////////
// CSpace::GetCurrentNode
// 
// accessors for current node
//////////////////////////////////////////////////////////////////////
CNode *CSpace::GetCurrentNode()
{
	return m_pCurrentNode;
}

//////////////////////////////////////////////////////////////////////
// CSpace::SetCurrentNode
// 
// sets the space's current node
//////////////////////////////////////////////////////////////////////
void CSpace::SetCurrentNode(CNode *pNode)
{
	m_pCurrentNode = pNode;

	// update all views for the new current node
	UpdateAllViews(NULL, 0L, m_pCurrentNode);
}


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
		/ (m_primSecRatio * GetTotalPrimaryActivation() 
			+ GetTotalSecondaryActivation()); 

	// scale for primary
	REAL scale_1 = scale_2 * m_primSecRatio;

	// scale the nodes
	for (int nAt = 0; nAt < GetNodeCount(); nAt++)
	{
		CNode *pNode = GetNodeAt(nAt);

		// scale this node's activation
		m_totalPrimaryActivation -= pNode->m_primaryActivation;
		pNode->m_primaryActivation *= scale_1;
		m_totalPrimaryActivation += pNode->m_primaryActivation;

		m_totalSecondaryActivation -= pNode->m_secondaryActivation;
		pNode->m_secondaryActivation *= scale_2;
		m_totalSecondaryActivation += pNode->m_secondaryActivation;
	}

}	// CSpace::NormalizeNodes


//////////////////////////////////////////////////////////////////////
// CSpace::GetSuperNodeCount
// 
// returns the number of super nodes
//////////////////////////////////////////////////////////////////////
int CSpace::GetSuperNodeCount()
{
	return __min(GetLayoutManager()->GetStateDim() / 2,
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



//////////////////////////////////////////////////////////////////////
// CSpace::GetPrimSecRatio
// 
// returns the primary/secondary ratio
//////////////////////////////////////////////////////////////////////
double CSpace::GetPrimSecRatio() const
{
	return m_primSecRatio;

}	// CSpace::GetPrimSecRatio


//////////////////////////////////////////////////////////////////////
// CSpace::SetPrimSecRatio
// 
// sets the primary/secondary ratio
//////////////////////////////////////////////////////////////////////
void CSpace::SetPrimSecRatio(double primSecRatio)
{
	m_primSecRatio = primSecRatio;

}	// CSpace::SetPrimSecRatio


//////////////////////////////////////////////////////////////////////
// CSpace::GetSpringConst
// 
// gets the spring constant for node position updates
//////////////////////////////////////////////////////////////////////
double CSpace::GetSpringConst()
{
	return m_springConst;

}	// CSpace::GetSpringConst


//////////////////////////////////////////////////////////////////////
// CSpace::SetSpringConst
// 
// sets the primary/secondary ratio
//////////////////////////////////////////////////////////////////////
void CSpace::SetSpringConst(double springConst)
{
	m_springConst = springConst;

}	// CSpace::SetSpringConst


//////////////////////////////////////////////////////////////////////
// CSpace::GetClassColorMap
// 
// class description accessors
//////////////////////////////////////////////////////////////////////
CMap<CString, LPCSTR, COLORREF, COLORREF>& CSpace::GetClassColorMap()
{
	return m_mapClassColors;

}	// CSpace::GetClassColorMap


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

	// clear the array
	m_arrNodes.RemoveAll();
	m_totalPrimaryActivation = 0.0;
	m_totalSecondaryActivation = 0.0;

	// create a new root node
	m_pRootNode = new CNode();
	m_pRootNode->SetName("%%hiddenroot%%");

	// create the hidden root
	CNode *pMainNode = new CNode();
	pMainNode->SetName("root");
	pMainNode->SetParent(m_pRootNode);
	AddNode(pMainNode, NULL);

	AddNode(new CNode(this, "Child1"), pMainNode);
	AddNode(new CNode(this, "Child2"), pMainNode);
	AddNode(new CNode(this, "Child3"), pMainNode);

	// initialize the node activations from the root node
	pMainNode->SetActivation((REAL) 0.5);
	pMainNode->ResetForPropagation();
	pMainNode->PropagateActivation((REAL) 0.8);

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
// CSpace::AddNodeToArray
// 
// adds a node (and its children) to the node array
//////////////////////////////////////////////////////////////////////
void CSpace::AddNodeToArray(CNode *pNode)
{
	// set the pointer to the space
	pNode->m_pSpace = this;
	pNode->m_pSpace->m_totalPrimaryActivation += 
		pNode->GetPrimaryActivation();
	pNode->m_pSpace->m_totalSecondaryActivation += 
		pNode->GetSecondaryActivation();

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
	// serialize base class
	CDocument::Serialize(ar);

	// set up the schema
	DWORD dwSchema = SPACE_SCHEMA;

	// if we are loading the space...
	if (ar.IsLoading())
	{
		// serialize schema
		ar >> dwSchema;

		// delete the current root node
		delete m_pRootNode;

		// read in the root node pointer
		ar >> m_pRootNode;

		// see if the name is the default (hidden) root name
		if (m_pRootNode->GetName() != "%%hiddenroot%%")
		{
			// create the hidden root
			CNode *pHiddenRoot = new CNode();
			pHiddenRoot->SetName("%%hiddenroot%%");

			// set the parent for the default node
			m_pRootNode->SetParent(pHiddenRoot);

			// now set the root to the hidden root
			m_pRootNode = pHiddenRoot;
		}

		// remove existing nodes from the array
		m_arrNodes.RemoveAll();
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

		// set the total activation
		GetTotalPrimaryActivation(TRUE);
		GetTotalSecondaryActivation(TRUE);
	
		// clear the class color map
		m_mapClassColors.RemoveAll();
	}
	else
	{
		// serialize schema
		ar << (DWORD) dwSchema;

		// just serialize the root node pointer
		ar << m_pRootNode;
	}

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
			ar >> m_primSecRatio;

			// super node count
			int nSuperNodeCount;
			ar >> nSuperNodeCount;
			GetLayoutManager()->SetStateDim(nSuperNodeCount * 2);

			// layout manager parameters
			double tolerance;
			ar >> tolerance;
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
			ar << GetSpringConst();

			// primary/secondary ratio
			ar << GetPrimSecRatio();

			// super node count
			ar << GetLayoutManager()->GetStateDim() / 2;

			// layout manager parameters
			ar << GetLayoutManager()->GetTolerance();
			ar << GetLayoutManager()->GetKPos();
			ar << GetLayoutManager()->GetKRep();
		}
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
#ifdef CHECK_SORTING
	for (int nAt = 0; nAt < GetNodeCount()-1; nAt++)
	{
		ASSERT(GetNodeAt(nAt)->GetActivation() >= 
			GetNodeAt(nAt+1)->GetActivation());
	}
#endif

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
REAL CSpace::GetTotalActivation(BOOL bCompute) const
{
	return GetTotalPrimaryActivation(bCompute)
		+ GetTotalSecondaryActivation(bCompute);

}	// CSpace::GetTotalActivation


//////////////////////////////////////////////////////////////////////
// CSpace::GetTotalPrimaryActivation
// 
// returns the total primary activation of all nodes
//////////////////////////////////////////////////////////////////////
REAL CSpace::GetTotalPrimaryActivation(BOOL bCompute) const
{
	// are we recomputing?
	if (bCompute)
	{
		// reset total
		m_totalPrimaryActivation = 0.0;

		// sum all primary activations
		for (int nAt = 0; nAt < GetNodeCount(); nAt++)
		{
			m_totalPrimaryActivation += 
				GetNodeAt(nAt)->GetPrimaryActivation();
		}
	}

	return m_totalPrimaryActivation;

}	// CSpace::GetTotalPrimaryActivation


//////////////////////////////////////////////////////////////////////
// CSpace::GetTotalSecondaryActivation
// 
// returns the total secondary activation of all nodes
//////////////////////////////////////////////////////////////////////
REAL CSpace::GetTotalSecondaryActivation(BOOL bCompute) const
{
	// are we recomputing?
	if (bCompute)
	{
		// reset total
		m_totalSecondaryActivation = 0.0;

		// sum all secondary activations
		for (int nAt = 0; nAt < GetNodeCount(); nAt++)
		{
			m_totalSecondaryActivation += 
				GetNodeAt(nAt)->GetSecondaryActivation();
		}
	}

	return m_totalSecondaryActivation;

}	// CSpace::GetTotalSecondaryActivation


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

	// check super-nodes for distance from center
	for (int nAt = 0; nAt < GetSuperNodeCount(); nAt++)
	{
		// get the node
		CNode *pNode = GetNodeAt(nAt);
		CVectorD<3> vOffset = pNode->GetPosition() - m_vCenter;
		
		if (vOffset.GetLength() > 500.0)
		{
			REAL overflow = vOffset.GetLength() - 500.0;
			vOffset.Normalize();
			vOffset *= (500.0 + overflow / 2.0);
		}

		pNode->SetPosition(m_vCenter + vOffset);
	}

	// position sub-threshold nodes
	for (nAt = GetSuperNodeCount(); nAt < GetNodeCount(); nAt++)
	{
		// get the node
		CNode *pNode = GetNodeAt(nAt);
		CVectorD<3> vNewPosition = m_vCenter;

		// see if there is a max activator
		if (pNode->GetMaxActivator() != NULL)
		{
			// the the new position
			vNewPosition = pNode->GetMaxActivator()->GetPosition();
			CVectorD<3> vCenter = m_vCenter;

			if (pNode->GetMaxActivator()->GetMaxActivator() != NULL)
			{
				vCenter = pNode->GetMaxActivator()->GetMaxActivator()->GetPosition();
			}
			CVectorD<3> vDirection = vNewPosition - vCenter;
			vDirection.Normalize();

			CVectorD<3> vSize = pNode->GetMaxActivator()->GetSize(
				pNode->GetMaxActivator()->GetActivation());

			vDirection *= vSize.GetLength() * 30.0;

			// set position to it's position
			vNewPosition += vDirection;
		}

		// set the new position
		pNode->SetPosition(vNewPosition);
	}

}	// CSpace::LayoutNodes


//////////////////////////////////////////////////////////////////////
// CSpace::SetCenter
// 
// lays out the nodes in the array
//////////////////////////////////////////////////////////////////////
void CSpace::SetCenter(double x, double y)
{
	m_vCenter[0] = x;
	m_vCenter[1] = y;

}	// CSpace::SetCenter


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
