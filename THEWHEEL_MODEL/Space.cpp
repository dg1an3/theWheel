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
const REAL PRIM_SEC_RATIO = 1.4;		// primary scale / secondary scale


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
		// m_nSuperNodeCount(20),
		m_pCluster(NULL),
		m_pLayoutManager(NULL)
{
	// set up the clusters
	SetClusterCount(0);

	// create the layout manager
	m_pLayoutManager = new CSpaceLayoutManager(this);
}

//////////////////////////////////////////////////////////////////////
// CSpace::~CSpace
// 
// deletes the CSpace object
//////////////////////////////////////////////////////////////////////
CSpace::~CSpace()
{
	// delete the layout manager
	delete m_pLayoutManager;

	// delete the clusters
	delete m_pCluster;

	// delete the root node (deletes other nodes)
	delete m_pRootNode;
}

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
}


//////////////////////////////////////////////////////////////////////
// CSpace::GetNodeCount
// 
// returns the number of nodes in the space
//////////////////////////////////////////////////////////////////////
int CSpace::GetNodeCount() const
{
	return m_arrNodes.GetSize();
}


//////////////////////////////////////////////////////////////////////
// CSpace::GetNodeAt
// 
// returns a node by index
//////////////////////////////////////////////////////////////////////
CNode *CSpace::GetNodeAt(int nAt) const
{
	return (CNode *) m_arrNodes.GetAt(nAt);
}


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

	// update the clusters
	SetClusterCount(GetClusterCount());

	// update all the views, passing the new node as a hint
	UpdateAllViews(NULL, NULL, pNewNode);	
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

	// update total activation
	m_totalActivation = GetRootNode()->GetDescendantActivation();

	// sort the nodes
	SortNodes();

	// and finally regenerate the clusters (if enabled)
	// ComputeClusters();
	// no clusters, so don't compute
	if (NULL != m_pCluster && GetClusterCount() > 0)
	{
		m_pCluster->UpdateClusters();
	}
}


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
}


//////////////////////////////////////////////////////////////////////
// CSpace::GetClusterCount
// 
// returns the number of clusters
//////////////////////////////////////////////////////////////////////
int CSpace::GetClusterCount() const
{
	if (m_pCluster)
	{
		return m_pCluster->GetSiblingCount();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////
// CSpace::GetClusterCount
// 
// sets the number of clusters
//////////////////////////////////////////////////////////////////////
void CSpace::SetClusterCount(int nCount)
{
	// delete the existing clusters
	delete m_pCluster;
	m_pCluster = NULL;

	if (nCount > 0)
	{
		// allocate new clusters
		m_pCluster = new CNodeCluster(this, nCount);
	}
}


//////////////////////////////////////////////////////////////////////
// CSpace::GetClusterAt
// 
// returns a cluster by index
//////////////////////////////////////////////////////////////////////
CNodeCluster *CSpace::GetClusterAt(int nAt)
{
	if (m_pCluster)
	{
		return m_pCluster->GetSibling(nAt);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////
// CSpace::GetSuperNodeCount
// 
// returns the number of super nodes
//////////////////////////////////////////////////////////////////////
int CSpace::GetSuperNodeCount()
{
	return __min(GetLayoutManager()->GetStateDim() / 2, // m_nSuperNodeCount, 
		GetNodeCount());
}


//////////////////////////////////////////////////////////////////////
// CSpace::SetMaxSuperNodeCount
// 
// sets the maximum number of super nodes
//////////////////////////////////////////////////////////////////////
void CSpace::SetMaxSuperNodeCount(int nSuperNodeCount)
{
	// m_nSuperNodeCount = nSuperNodeCount;
	GetLayoutManager()->SetStateDim(nSuperNodeCount * 2);

	// reconstruct the clusters
	SetClusterCount(GetClusterCount());
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
	m_pRootNode->SetName("root");

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
}


//////////////////////////////////////////////////////////////////////
// CSpace::SortNodes
// 
// sorts the nodes by activation value
//////////////////////////////////////////////////////////////////////
void CSpace::SortNodes()
{
	// flag to indicate a rearrangement has occurred
	BOOL bRearrange;
	do 
	{
		// initially, no rearrangement has occurred
		bRearrange = FALSE;

		// for each link,
		for (int nAt = 0; nAt < GetNodeCount()-1; nAt++)
		{
			// get the current and next links
			CNode *pThisNode = GetNodeAt(nAt);
			CNode *pNextNode = GetNodeAt(nAt+1);

			// compare their weights
			double thisAct = pThisNode->GetActivation();
			double nextAct = pNextNode->GetActivation();
			if (thisAct < nextAct)
			{
				// if first is less than second, swap
				m_arrNodes.SetAt(nAt, pNextNode);
				m_arrNodes.SetAt(nAt+1, pThisNode);

				// a rearrangement has occurred
				bRearrange = TRUE;
			}
		}

	// continue as long as rearrangements occur
	} while (bRearrange);
}


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
}


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
}


//////////////////////////////////////////////////////////////////////
// CSpace::AddNodeToArray
// 
// adds a node (and its children) to the node array
//////////////////////////////////////////////////////////////////////
void CSpace::AddNodeToArray(CNode *pNode)
{
	m_arrNodes.Add(pNode);
	for (int nAt = 0; nAt < pNode->CNode::GetChildCount(); nAt++)
	{
		AddNodeToArray(pNode->CNode::GetChildAt(nAt));
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

		// add the nodes to the array
		m_arrNodes.RemoveAll();
		AddNodeToArray(m_pRootNode);

		// set the cluster count (re-initializes clusters)
		SetClusterCount(GetClusterCount());

		// and do some activation
		ActivateNode(m_pRootNode, 0.5);
		// ActivateNode(m_pRootNode, 0.5);

		// initialize clusters
		if (GetSuperNodeCount() < GetNodeCount())
		{
			for (int nAtCluster = 0; nAtCluster < GetClusterCount(); nAtCluster++)
			{
				CNodeCluster *pCluster = GetClusterAt(nAtCluster);
				CNode *pNode = GetNodeAt(GetSuperNodeCount() + nAtCluster);
				pCluster->AddNodeToCluster(pNode, 0.0f);
				pCluster->LoadLinkWeights();
			}
		}
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
}
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
}

// returns a pointer to the layout manager
CSpaceLayoutManager *CSpace::GetLayoutManager()
{
	return m_pLayoutManager;
}

//////////////////////////////////////////////////////////////////////
// CSpace::LayoutNodes
// 
// lays out the nodes in the array
//////////////////////////////////////////////////////////////////////
void CSpace::LayoutNodes()
{
	// layout the nodes
	m_pLayoutManager->LayoutNodes();

	BOOL m_bAdjustPrinEigenvector = FALSE;
	if (m_bAdjustPrinEigenvector)
	{
		// now adjust for center and rotation
		CVector<3> vCenter = GetCentralMoment();
		CVector<2> vPrinEigenvector;
		REAL prinEigenvalue = Eigen(GetInertiaTensor(), 1, &vPrinEigenvector);
		
		if (fabs(prinEigenvalue) < EPS)
		{
			// compute rotation angle
			REAL angle;
			if (fabs(vPrinEigenvector[0]) > EPS)
			{
				angle = atan(vPrinEigenvector[1] / vPrinEigenvector[0]);
			}
			else if (vPrinEigenvector[1] > 0.0)
			{
				angle = PI / 2.0;
			}
			else
			{
				angle = -PI / 2.0;
			}
			
			REAL scale = 1.0; // fabs(prinEigenvalue) / (fabs(prinEigenvalue) + 400.0);
			CMatrix<2> mRotate = CreateRotate(-scale * angle);

			// rotate each node about the central moment, by the angle
			for (int nAt = 0; nAt < GetNodeCount(); nAt++)
			{
				CVector<3> vPosition = GetNodeAt(nAt)->GetPosition();
				vPosition -= vCenter;
				vPosition = CVector<3>(mRotate * CVector<2>(vPosition));
				vPosition += vCenter;
				GetNodeAt(nAt)->SetPosition(vPosition);
			} 
		}
	}
}

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
}

//////////////////////////////////////////////////////////////////////
// CSpace::GetCentralMoment
// 
// returns the central moment of the space
//////////////////////////////////////////////////////////////////////
CVector<3> CSpace::GetCentralMoment()
{
	CVector<3> vCenter;
	REAL total_activation = (REAL) 0.0;

	// for each node
	for (int nAt = 0; nAt < GetSuperNodeCount(); nAt++)
	{
		total_activation += GetNodeAt(nAt)->GetActivation();
		vCenter += GetNodeAt(nAt)->GetActivation() * GetNodeAt(nAt)->GetPosition();
	}

	// scale by the total activation
	vCenter *= (REAL) 1.0 / total_activation;

	return vCenter;
}

//////////////////////////////////////////////////////////////////////
// CSpace::GetInertiaTensor
// 
// returns the inertia tensor for the space
//////////////////////////////////////////////////////////////////////
CMatrix<2> CSpace::GetInertiaTensor()
{
	// get the center
	CVector<3> vCenter = GetCentralMoment();

	// initialize total activation
	REAL total_activation = (REAL) 0.0;

	// set up the inertia tensor matrix
	CMatrix<2> mIT;

	// set to all zeros
	mIT[0][0] = 0.0;
	mIT[0][1] = 0.0;
	mIT[1][0] = 0.0;
	mIT[1][1] = 0.0;
	
	// for each node
	for (int nAt = 0; nAt < GetSuperNodeCount(); nAt++)
	{
		total_activation += GetNodeAt(nAt)->GetActivation();
		CVector<3> vVar = GetNodeAt(nAt)->GetActivation()
			* (GetNodeAt(nAt)->GetPosition() - vCenter);

		mIT[0][0] += vVar[0] * vVar[0];
		mIT[0][1] += vVar[0] * vVar[1];
		mIT[1][0] += vVar[1] * vVar[0];
		mIT[1][1] += vVar[1] * vVar[1];
	}

	// scale by total activation
	mIT *= (REAL) 1.0 / total_activation;

	// return inertia tensor
	return mIT;
}

