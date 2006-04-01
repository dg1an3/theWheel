#include "include\Space.h"
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

// #include "SSELayoutManager.h"

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

// constants for runaway adjustment
const REAL MAX_RUNAWAY_RADIUS = 500.0;
const REAL RUNAWAY_DIVISOR = 3.0;

// constant for sub-threshold decay toward center
const REAL SUBNODE_DECAY_RATE = 0.9;

// displacement factor for max activator displacement
const REAL MAX_ACT_DISPLACEMENT = 100.0;

// fraction of old SSV to keep during update
const REAL UPDATE_SSV_FRAC = 0.75;

// min and max for randomly assigning initial weights
const REAL MIN_INITIAL_WGT = 0.4;
const REAL MAX_INITIAL_WGT = 0.7;


//////////////////////////////////////////////////////////////////////
// CompareNodeActivations
// 
// comparison function for sorting nodes by activation, descending
//////////////////////////////////////////////////////////////////////
int __cdecl CompareNodeActivations(const void *elem1, const void *elem2)
{
	CNode *pNode1 = *(CNode**) elem1;
	CNode *pNode2 = *(CNode**) elem2;

	return Round<int>(1000.0 * (pNode2->GetActivation() - pNode1->GetActivation()));

}	// CompareNodeActivations


//////////////////////////////////////////////////////////////////////
// GenerateRandom
// 
// generates a random number in the specified range
//////////////////////////////////////////////////////////////////////
REAL GenerateRandom(REAL min, REAL max)
{
	REAL value = min;
	value += (max - min) * (REAL) rand() / (REAL) RAND_MAX;

	return value;

}	// GenerateRandom

// __declspec(cpu_specific(pentium))
BOOL HasMMX()
{
	return FALSE;
}

/*
// __declspec(cpu_specific(pentium_MMX)) 
BOOL HasMMX()
{
	return TRUE;
}

// __declspec(cpu_dispatch(pentium, pentium_MMX))
BOOL HasMMX()
{
   /* Empty function body informs the compiler to generate the
   CPU-dispatch function listed in the cpu_dispatch clause. * /
}  
*/

//////////////////////////////////////////////////////////////////////
// Event Firing
//////////////////////////////////////////////////////////////////////
#define SPACE_FIRE_CHANGE(TAG, NODE) UpdateAllViews(NULL, TAG, NODE); 

/////////////////////////////////////////////////////////////////////////////
// CSpace construction/destruction
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpace::CSpace
// 
// constructs an empty CSpace object.  use OnNewDocument to initialize
//////////////////////////////////////////////////////////////////////
CSpace::CSpace()
	: m_pRootNode(NULL)
	, m_pDS(NULL)
	, m_bNodesSorted(FALSE)

	, m_pStateVector(NULL)
	, m_pLayoutManager(NULL)
	, m_nLastPostSuper(0)

	, m_pCurrentNode(NULL)

	, m_totalPrimaryActivation(0.0)
	, m_totalSecondaryActivation(0.0)
	, m_PrimSecRatio(PRIM_SEC_RATIO)
	, m_SpringConst(0.95)
{
	m_pStateVector = new CSpaceStateVector(this);

	m_pLayoutManager = new CSpaceLayoutManager(this);

	// set the spring constant (from the profile)
	SetSpringConst( 1.0 / 100.0 * 
		(REAL) ::AfxGetApp()->GetProfileInt("LAYOUT", "SPRING_CONST", 
			Round<int>(100.0 * GetSpringConst())));

}	// CSpace::CSpace


//////////////////////////////////////////////////////////////////////
// CSpace::~CSpace
// 
// deletes the CSpace object
//////////////////////////////////////////////////////////////////////
CSpace::~CSpace()
{
	// delete the state vector
	delete m_pStateVector;

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

	// update all the views, passing the new node as a hint
	NodeAddedEvent.Fire(pNewNode);

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
	for (int nAt = 0; nAt < GetNodeCount(); nAt++)
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

	// update all the views, passing the removed node as a hint
	NodeRemovedEvent.Fire(pMarkedNode);

}	// CSpace::RemoveNode

/*
//////////////////////////////////////////////////////////////////////
// CSpace::GetCurrentNode
// 
// accessors for current node
//////////////////////////////////////////////////////////////////////
CNode *CSpace::GetCurrentNode()
{
	return m_pCurrentNode;

}	// CSpace::GetCurrentNode
*/

//////////////////////////////////////////////////////////////////////
// CSpace::SetCurrentNode
// 
// sets the space's current node
//////////////////////////////////////////////////////////////////////
void CSpace::SetCurrentNode(CNode *pNode)
{
	m_pCurrentNode = pNode;

	// update all views for the new current node
	CurrentNodeChangedEvent.Fire(m_pCurrentNode);

}	// CSpace::SetCurrentNode


//////////////////////////////////////////////////////////////////////
// CSpace::ActivateNode
// 
// activates a particular node
//////////////////////////////////////////////////////////////////////
void CSpace::ActivateNode(CNode *pNode, REAL scale)
{
	// scale = _cpp_min<REAL>(scale, 0.5);

	// first, compute the new activation of the node up to the max
	REAL oldActivation = pNode->GetActivation();
	REAL newActivation = 
		oldActivation + (TOTAL_ACTIVATION * (REAL) 0.40 - oldActivation) 
			* scale;

	// set the node's new activation
	pNode->SetActivation(newActivation);

	// now, propagate the activation
	m_pRootNode->ResetForPropagation();

	// now propagate
	pNode->PropagateActivation(
		(REAL) 1.0,		// 0.4,
		(REAL) 0.98); // 0.7); // 0.999);

	// now update the activation for all nodes
	m_pRootNode->UpdateFromNewActivation();

	// normalize the nodes
	NormalizeNodes();

	// flag the nodes as unsorted
	m_bNodesSorted = FALSE;

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
		/ (GetTotalPrimaryActivation(true) 
			+ GetPrimSecRatio() * GetTotalSecondaryActivation(true) + 1e-6); 

	// scale for primary
	REAL scale_1 = sum 
		 / (GetTotalPrimaryActivation(false) / GetPrimSecRatio()
			+ GetTotalSecondaryActivation(false) + 1e-6); 

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
// CSpace::Relax
// 
// relaxes the link weights (adjusts gain based on dist error)
//////////////////////////////////////////////////////////////////////
void CSpace::Relax(bool bSubThreshold)
{
	// iterate over all super nodes
	for (int nAt = 0; nAt < GetSuperNodeCount(); nAt++)
	{
		// get current node
		CNode *pNode = GetNodeAt(nAt);

		// iterate over linked nodes
		for (int nAtLinked = 0; nAtLinked < GetSuperNodeCount(); nAtLinked++)
		{
			// skip if we are at the same node
			if (nAt == nAtLinked)
			{
				continue;
			}

			// get the target and link
			CNode *pTarget = GetNodeAt(nAtLinked);

			if (bSubThreshold 
				&& !pTarget->GetIsSubThreshold()
				&& !pNode->GetIsSubThreshold()
				&& !(pTarget->GetPostSuperCount() > 0)
				&& !(pNode->GetPostSuperCount() > 0))
			{
				continue;
			}

			// get the link, if there is one
			CNodeLink *pLink = pNode->GetLinkTo(pTarget);
			if (pLink != NULL)
			{
				// compute the distance error
				const REAL distErr = m_pLayoutManager->GetDistError(pNode, pTarget);
				const REAL distErrScaled = distErr / pNode->GetActivation();

				// compute the exponential of the distance
				REAL exp_dist = 1.0 * exp( 0.02 // 0.004 
					* distErrScaled); //  + 2.0);

				if (pLink->GetIsStabilizer())
				{
					exp_dist = 0.1 * exp(0.2 * distErrScaled);
				}

				// check that the exponential is finite
				if (_finite(exp_dist))
				{
					// compute the gain and set it
					REAL new_gain = 1.0 - Sigmoid(distErr - 6.0, 8.0);
					// exp_dist / (exp_dist + 
					//	20.0); // 12.0); 
					pLink->SetGain(new_gain);
				}
				else
				{
					// getting too far away, need more power
					pLink->SetGain(0.01);
				}
			}
		}
	}

}	// CSpace::Relax


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

	// flag to indicate dirty doc
	// SetModifiedFlag(TRUE);
	// TODO: need to fire a change so that modified flag

}	// CSpace::SetMaxSuperNodeCount


/*
//////////////////////////////////////////////////////////////////////
// CSpace::GetPrimSecRatio
// 
// returns the primary/secondary ratio
//////////////////////////////////////////////////////////////////////
REAL CSpace::GetPrimSecRatio() const
{
	return m_primSecRatio;

}	// CSpace::GetPrimSecRatio


//////////////////////////////////////////////////////////////////////
// CSpace::SetPrimSecRatio
// 
// sets the primary/secondary ratio
//////////////////////////////////////////////////////////////////////
void CSpace::SetPrimSecRatio(REAL primSecRatio)
{
	m_primSecRatio = primSecRatio;

	// flag to indicate dirty doc
	// SetModifiedFlag(TRUE);
	// TODO: need to fire a change so that modified flag

}	// CSpace::SetPrimSecRatio
*/

//////////////////////////////////////////////////////////////////////
// CSpace::GetClassColorMap
// 
// class description accessors
//////////////////////////////////////////////////////////////////////
CMap<CString, LPCSTR, COLORREF, COLORREF>& CSpace::GetClassColorMap()
{
	return m_mapClassColors;

}	// CSpace::GetClassColorMap


//////////////////////////////////////////////////////////////////////
// CSpace::OnNewDocument
// 
// initializes a new document
//////////////////////////////////////////////////////////////////////
BOOL CSpace::CreateSimpleSpace()
{
	DeleteContents();

	// clear the array
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
	pMainNode->PropagateActivation((REAL) 0.2, // (REAL) 0.5, 
		(REAL) 0.9);

	SortNodes();
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
	// if nodes are sorted, leave alone
	if (!m_bNodesSorted)
	{	
		// sort them
		qsort(m_arrNodes.GetData(), m_arrNodes.GetSize(), sizeof(CNode*),
			CompareNodeActivations);

		// arrange post-super nodes
		m_nLastPostSuper = GetSuperNodeCount();
		for (int nAt = GetSuperNodeCount(); nAt < GetNodeCount(); nAt++)
		{
			CNode *pNode = GetNodeAt(nAt);
			if (pNode->GetIsPostSuper())
			{
				m_arrNodes[nAt] = m_arrNodes[m_nLastPostSuper];
				m_arrNodes[m_nLastPostSuper] = pNode;
				m_nLastPostSuper++;
			}
		}

		// flag as sorted
		m_bNodesSorted = TRUE;
	}

}	// CSpace::SortNodes


//////////////////////////////////////////////////////////////////////
// CSpace::PositionNewSuperNodes
// 
// positions the new super-nodes
//////////////////////////////////////////////////////////////////////
void CSpace::PositionNewSuperNodes()
{
	// stores the highest new supernode
	int nHighestNewSuper = GetSuperNodeCount();

	// TODO: static this
	// CArray arrNewSuper;

	// check super-nodes for distance from center
	for (int nAt = 0; nAt < nHighestNewSuper /* GetSuperNodeCount() */; nAt++)  
		// GetSuperNodeCount()-1; nAt >= 0; nAt--)
	{
		// get the node
		CNode *pNode = GetNodeAt(nAt);

		if (pNode->GetIsSubThreshold())
		{
			// add to the list
			// arrNewSuper.Add(pNode);
			m_arrNodes.RemoveAt(nAt);
			nAt--;		// repeat, since we moved the current

			m_arrNodes.InsertAt(GetSuperNodeCount()-1, pNode);

			// we have a new highest
			//nHighestNewSuper = nAt;
			nHighestNewSuper--;

			// set its position based on max activator, if it has one
			CNode *pMaxAct = pNode->GetMaxActivator();
			if (pMaxAct != NULL)
			{
				// the new position -- initially at the max act position
				CVectorD<3> vNewPosition = pMaxAct->GetPosition();

// #define SHOOT
#ifdef SHOOT
				// find direction by default from center
				CVectorD<3> vCenter = GetCenter(); // m_vCenter;

				// see if IT has a max activator
				if (pMaxAct->GetMaxActivator() != NULL)
				{
					// find direction based on both max activators
			 		vCenter = pMaxAct->GetMaxActivator()->GetPosition();
				}

				// form the direction
				CVectorD<3> vDirection = vNewPosition - vCenter;
				if (vDirection.GetLength() < 1e-6)
				{
					// add some random length
					::RandomVector<REAL>(1.0, &vDirection[0], 3);
				}
				vDirection.Normalize();

				// get the size of the max activator
				CVectorD<3> vSize = pMaxAct->GetSize(
					pMaxAct->GetActivation());

				// adjust the direction
				vDirection *= vSize.GetLength() * MAX_ACT_DISPLACEMENT;

				// set position to it's position
				vNewPosition += vDirection;
#endif

				// set the new position
				pNode->SetPosition(vNewPosition, true);		// BEFORE was FALSE
				// set flag to indicate position has been reset
				// pNode->m_bPositionReset = true;

				// NEW: set gain for max activator link
				for (int nAtLink = 0; nAtLink < pNode->GetLinkCount(); nAtLink++)
				{
					pNode->GetLinkAt(nAtLink)->SetGain(1.0);
					CNode *pLinkTo = pNode->GetLinkAt(nAtLink)->GetTarget();
					CNodeLink *pLinkToLink = pLinkTo->GetLinkTo(pNode);
					if (pLinkToLink != NULL)
					{
						pLinkToLink->SetGain(1.0);
					}

/*					if (pLinkTo->GetIsSubThreshold())
					{
						pNode->GetLinkAt(nAtLink)->SetGain(1.0);
						pLinkTo->GetLinkTo(pNode)->SetGain(1.0);
					}
					else
					{
						pNode->GetLinkAt(nAtLink)->SetGain(1.0);
						pLinkTo->GetLinkTo(pNode)->SetGain(1.0);
					} */
				}
/*				if (pNode->GetLinkTo(pMaxAct) != NULL)
				{
					pNode->GetLinkTo(pMaxAct)->SetGain(1.0);
					pMaxAct->GetLinkTo(pNode)->SetGain(1.0);
				}  */
			}
		}
	}

	// now find any sub-threshold new post-super nodes
	for (int nAt = GetSuperNodeCount(); nAt < GetNodeCount(); nAt++)
	{
		// get the node
		CNode *pNode = GetNodeAt(nAt);
		if (pNode->GetPostSuperCount() > 0)
		{
			// delete from array
			m_arrNodes.RemoveAt(nAt);

			// move to last position
			m_arrNodes.InsertAt(nHighestNewSuper, pNode);
		}
	}

	// now move remaining new post-super nodes to bottom
	for (int nAt = 0; nAt < nHighestNewSuper; nAt++)
	{
		// get the node
		CNode *pNode = GetNodeAt(nAt);
		if (pNode->GetPostSuperCount() > 0)
		{
			// delete from array
			m_arrNodes.RemoveAt(nAt);
			nAt--;

			// move to last position
			nHighestNewSuper--;
			m_arrNodes.InsertAt(nHighestNewSuper, pNode);
		}
	}

#define LAYOUT_AFTER
#ifdef LAYOUT_AFTER
	// now layout the new supernodes
	// REAL oldTolerance = m_pLayoutManager->GetTolerance();
	// m_pLayoutManager->SetTolerance(oldTolerance / (REAL) 100.0);

	for (int nRep = 0; nRep < 3; nRep++)
	{
		// TODO: load state vector
		m_pLayoutManager->LayoutNodes(m_pStateVector, nHighestNewSuper);

		// now relax the new super nodes
		Relax(true);
	}

	// m_pLayoutManager->SetTolerance(oldTolerance);
#endif

	// finish processing by moving the new super-nodes to
	//		their updated position
	for (int nAt = 0; nAt < GetSuperNodeCount(); nAt++)
	{
		// get the node
		CNode *pNode = GetNodeAt(nAt);

		// reset max activator shit
		pNode->m_maxDeltaActivation = 0.0;
		pNode->m_pMaxActivator = NULL;
		pNode->m_bFoundMaxActivator = FALSE;

		// adjust the sub-threshold nodes
		if (pNode->GetIsSubThreshold())
		{
			// and flag as no longer sub threshold
			pNode->SetIsSubThreshold(FALSE);
			pNode->SetIsPostSuper(FALSE);

			// set the post super count
			pNode->SetPostSuperCount(40);

			// set the new position, triggering a reset
			pNode->SetPosition(pNode->GetPosition(), true);

			// set flag to indicate position has been reset
			// pNode->m_bPositionReset = true;
		}
		else if (pNode->GetPostSuperCount() > 0)
		{
			// decrement post super count
			pNode->SetPostSuperCount(pNode->GetPostSuperCount()-1);
		}

	}

}	// CSpace::PositionNewSuperNodes


//////////////////////////////////////////////////////////////////////
// CSpace::AdjustRunawayNodes
// 
// adjusts for run-away nodes
//////////////////////////////////////////////////////////////////////
void CSpace::AdjustRunawayNodes()
{
	// check super-nodes for distance from center
	for (int nAt = 0; nAt < GetSuperNodeCount(); nAt++)
	{
		// get the node
		CNode *pNode = GetNodeAt(nAt);

		// compute the position, offset from the current center point
		CVectorD<3> vOffset = pNode->GetPosition() - GetCenter();

		// are we too far away?
		if (vOffset.GetLength() > MAX_RUNAWAY_RADIUS)
		{
			REAL overflow = vOffset.GetLength() - MAX_RUNAWAY_RADIUS;
			vOffset.Normalize();
			vOffset *= MAX_RUNAWAY_RADIUS + overflow / RUNAWAY_DIVISOR;
		}

		// set newly calculated position
		pNode->SetPosition(GetCenter() + vOffset);

		// ensure we are not subthreshold, for good measure
		pNode->SetIsSubThreshold(FALSE);
	}

}	// CSpace::AdjustRunawayNodes



//////////////////////////////////////////////////////////////////////
// CSpace::PositionSubNodes
// 
// adjusts the positions of sub-threshold nodes
//////////////////////////////////////////////////////////////////////
void CSpace::PositionSubNodes()
{
	// store the old state dimension
	int nSuperNodes = GetSuperNodeCount();
	int nOldStateDim = m_pLayoutManager->GetStateDim();

	// store super node count, because SetStateDim will change
	m_pLayoutManager->SetStateDim(m_nLastPostSuper * 2);

	// now layout the nodes
	m_pLayoutManager->LayoutNodes(m_pStateVector, nSuperNodes);

	// restore layout manager values
	m_pLayoutManager->SetStateDim(nOldStateDim);

	// position sub-threshold nodes
	for (int nAt = m_nLastPostSuper; nAt < GetNodeCount(); nAt++)
	{
		// get the node
		CNode *pNode = GetNodeAt(nAt);

		// mark node as sub-threshold
		pNode->SetIsSubThreshold(TRUE);

		if (pNode->GetMaxActivator() != NULL)
		{
			// compute the new position
			CVectorD<3> vNewPosition = 
				pNode->GetMaxActivator()->GetPosition() - GetCenter();

			// decay towards center
			vNewPosition *= SUBNODE_DECAY_RATE;
			vNewPosition += GetCenter();

			// and set the new position
			pNode->SetPosition(vNewPosition); 
		}
	}

}	// CSpace::PositionSubNodes


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

	// flag that the nodes need to be sorted
	m_bNodesSorted = FALSE;

}	// CSpace::AddNodeToArray


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
		ar >> dwSchema;

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
		for (int nAt = 0; nAt < GetSuperNodeCount(); nAt++)
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
			double primSecRatio;
			ar >> primSecRatio;
			SetPrimSecRatio(primSecRatio);

			// super node count
			int nSuperNodeCount;
			ar >> nSuperNodeCount;
			SetMaxSuperNodeCount(nSuperNodeCount);

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

// #define NORMALIZE_LINK_WEIGHTS
#ifdef NORMALIZE_LINK_WEIGHTS
	// get max link weight for all nodes
	REAL maxWeight = m_pRootNode->GetMaxLinkWeight();

	// adjust max to 0.80
	m_pRootNode->ScaleLinkWeights(0.40 / maxWeight);
#endif

}	// CSpace::Serialize


//////////////////////////////////////////////////////////////////////
// CSpace::DeleteContents
// 
// removes all nodes
//////////////////////////////////////////////////////////////////////
void CSpace::DeleteContents()
{
	// delete the current root node
	delete m_pRootNode;
	m_pRootNode = NULL;

	// remove existing nodes from the array
	m_arrNodes.RemoveAll();

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
		REAL totalPrimaryActivation = 0.0;

		// sum all primary activations
		for (int nAt = 0; nAt < GetNodeCount(); nAt++)
		{
			totalPrimaryActivation += 
				GetNodeAt(nAt)->GetPrimaryActivation();
		}

		m_totalPrimaryActivation = m_totalPrimaryActivation * 0.01 
			+ totalPrimaryActivation * 0.99;
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
		REAL totalSecondaryActivation = 0.0;

		// sum all secondary activations
		for (int nAt = 0; nAt < GetNodeCount(); nAt++)
		{
			totalSecondaryActivation += 
				GetNodeAt(nAt)->GetSecondaryActivation();
		}

		m_totalSecondaryActivation = m_totalSecondaryActivation * 0.01
			+ totalSecondaryActivation * 0.99;
	}

	return m_totalSecondaryActivation;

}	// CSpace::GetTotalSecondaryActivation


// returns a pointer to the state vector
CSpaceStateVector *CSpace::GetStateVector()
{
	return m_pStateVector;
}

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
	// ensure nodes are sorted
	SortNodes();

	// position any new super-threshold nodes
	PositionNewSuperNodes();

	// layout the nodes
	m_pLayoutManager->LayoutNodes(m_pStateVector, 0);

	// adjust any runaway nodes
	AdjustRunawayNodes();

	// relax
	Relax();

	// finally, position the sub-threshold nodes
	PositionSubNodes();

}	// CSpace::LayoutNodes

/*
//////////////////////////////////////////////////////////////////////
// CSpace::SetCenter
// 
// lays out the nodes in the array
//////////////////////////////////////////////////////////////////////
void CSpace::SetCenter(REAL x, REAL y)
{
	m_vCenter[0] = x;
	m_vCenter[1] = y;

}	// CSpace::SetCenter
*/

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

