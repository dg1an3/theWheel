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
// CSpace construction/destruction
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpace::CSpace
// 
// constructs an empty CSpace object.  use OnNewDocument to initialize
//////////////////////////////////////////////////////////////////////
CSpace::CSpace()
	: m_pRootNode(NULL),
		m_nSuperNodeCount(20),
		m_pCluster(NULL)
{
	SetClusterCount(1);
}

//////////////////////////////////////////////////////////////////////
// CSpace::~CSpace
// 
// deletes the CSpace object
//////////////////////////////////////////////////////////////////////
CSpace::~CSpace()
{
	delete m_pRootNode;

	delete m_pCluster;
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

int CSpace::GetNodeCount()
{
	return m_arrNodes.GetSize();
}

CNode *CSpace::GetNodeAt(int nAt)
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
	float actWeight = 
		0.5f * (1.4f - 0.8f * (float) rand() / (float) RAND_MAX);

	// set up the link
	pNewNode->LinkTo(pParentNode, (float) actWeight);

	// add the new node to the parent
	pNewNode->SetParent(pParentNode);

	// set the activation for the new node
	pNewNode->SetActivation(actWeight * 0.1f);

	// add the node to the array
	AddNodeToArray(pNewNode);

	// update all the views, passing the new node as a hint
	UpdateAllViews(NULL, NULL, pNewNode);	
}


//////////////////////////////////////////////////////////////////////
// CSpace::ActivateNode
// 
// activates a particular node
//////////////////////////////////////////////////////////////////////
void CSpace::ActivateNode(CNode *pNode, float scale)
{
	// first, compute the new activation of the node up to the max
	double oldActivation = pNode->GetActivation();
	double newActivation = 
		oldActivation + (TOTAL_ACTIVATION * 0.60 - oldActivation) * scale;

	// set the node's new activation
	pNode->SetActivation(newActivation);

	// now, propagate the activation
	m_pRootNode->ResetForPropagation();
	pNode->PropagateActivation(0.4);

	// normalize the nodes
	NormalizeNodes();

	// sort the nodes
	SortNodes();

	// and finally regenerate the clusters (if enabled)
	ComputeClusters();
}


//////////////////////////////////////////////////////////////////////
// CSpace::NormalizeNodes
// 
// normalizes the nodes to a known sum
//////////////////////////////////////////////////////////////////////
void CSpace::NormalizeNodes(double sum)
{
	// compute the scale factor for normalization
	const double ratio = 1.5;		// primary scale / secondary scale

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
}


// accessors for the clusters
int CSpace::GetClusterCount()
{
	if (m_pCluster)
		return m_pCluster->GetSiblingCount();
	else
		return 0;
}

void CSpace::SetClusterCount(int nCount)
{
	delete m_pCluster;
	m_pCluster = new CNodeCluster(this, nCount);
}

CNodeCluster *CSpace::GetClusterAt(int nAt)
{
	if (m_pCluster)
		return m_pCluster->GetSibling(nAt);
	else
		return NULL;
}

// accessors for the super node count
int CSpace::GetSuperNodeCount()
{
	return m_nSuperNodeCount;
}

void CSpace::SetSuperNodeCount(int nSuperNodeCount)
{
	m_nSuperNodeCount = nSuperNodeCount;

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

// helper function to sort the nodes by activation
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
			if (pThisNode->GetActivation() < pNextNode->GetActivation())
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


void CSpace::ComputeClusters()
{
	// no clusters, so don't compute
	if (!m_pCluster || GetClusterCount() == 0)
		return;

	m_pCluster->UpdateClusters();

/*
	// store total activation
	double total_activation = m_pRootNode->GetDescendantActivation();
	// 
	// initialize node clusters with first N subnodes
	// iterations for the k-mean computation
	for (int nIter = 0; nIter < 1; nIter++)
	{
		// finalize node clusters
		for (int nAtCluster = 0; nAtCluster < GetClusterCount(); nAtCluster++)
		{
			CNodeCluster *pCluster = GetClusterAt(nAtCluster);
			pCluster->ResetTotalActivation();

			// updates the link vector from the current weights
			pCluster->UpdateLinkVector();
		}

		// assign nodes to the clusters based on distance
		for (int nAtNode = GetSuperNodeCount(); 
			nAtNode < min(GetSuperNodeCount() * GetClusterCount(), GetNodeCount()); 
			nAtNode++)
		{
			CNode *pNode = GetNodeAt(nAtNode);

			float p_update = 0.5;

			if ((float) rand() < p_update * (float) RAND_MAX)
			{
				// TODO: make a member function on CNodeCluster to do this
				float minDist = FLT_MAX;
				CNodeCluster *pNearestCluster;
				for (int nAtCluster = 0; nAtCluster < GetClusterCount(); nAtCluster++)
				{
					CNodeCluster *pCluster = GetClusterAt(nAtCluster);
					float dist = pCluster->GetNodeDistanceSq(pNode);
					if (dist < minDist)
					{
						minDist = dist;
						pNearestCluster = pCluster;
					}
				}

				// now add to the appropriate cluster
				pNearestCluster->AddNodeToCluster(pNode);
			}
		}

		// finalize node clusters
		for (nAtCluster = 0; nAtCluster < GetClusterCount(); nAtCluster++)
		{
			CNodeCluster *pCluster = GetClusterAt(nAtCluster);
			pCluster->LoadLinkWeights();
			// pCluster->ResetTotalActivation();

//			TRACE("Cluster %i at iteration %i\n", nAtCluster, nIter);
//			pCluster->DumpLinkVector();

			// load the links to other clusters
			for (int nAtOtherCluster = 0; nAtOtherCluster < GetClusterCount(); nAtOtherCluster++)
			{
				CNodeCluster *pOtherCluster = GetClusterAt(nAtOtherCluster);
				if (pCluster != pOtherCluster)
				{
					float weight = pCluster->GetLinkWeightToCluster(pOtherCluster);
					pCluster->LinkTo(pOtherCluster, weight, FALSE);

					// output the weights to other cluster
//					TRACE("\tLink to cluster %i: %f\n", nAtOtherCluster, weight);
				}
			}
		}
	} */
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

		pChild->SetDescription("Ud dfjaskf rtertj 23 adsjf.  "
			"Lkdjfsdfj sdaf ajskjgew ajg ajsdklgj; slrj jagifj ajdfgjkld.  "
			"I d fdmgj sdl jdsgiow mrektmrejgj migmoergmmsos m ksdfogkf.");
		pParent->children.Add(pChild);
		pChild->SetParent(pParent);

		// set the image filename
		char pszImageFilename[_MAX_FNAME];
		sprintf(pszImageFilename, "image%i.bmp", rand() % 8);
		pChild->SetImageFilename(pszImageFilename);

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

void CSpace::AddNodeToArray(CNode *pNode)
{
	m_arrNodes.Add(pNode);
	for (int nAt = 0; nAt < pNode->children.GetSize(); nAt++)
		AddNodeToArray((CNode *)pNode->children.Get(nAt));
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

		// and do some activation
		ActivateNode(m_pRootNode, 0.5);
		ActivateNode(m_pRootNode, 0.5);

		// initialize clusters
		for (int nAtCluster = 0; nAtCluster < GetClusterCount(); nAtCluster++)
		{
			CNodeCluster *pCluster = GetClusterAt(nAtCluster);
			CNode *pNode = GetNodeAt(GetSuperNodeCount() + nAtCluster);
			pCluster->AddNodeToCluster(pNode, 0.0f);
			pCluster->LoadLinkWeights();

	//		TRACE("Cluster %i initial state\n", nAtCluster);
	//		pCluster->DumpLinkVector();
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

	for (int nAt = 0; nAt < m_pRootNode->children.GetSize(); nAt++)
	{
		const CNode *pNode = (const CNode *) m_pRootNode->children.Get(nAt);
		for (int nAtLink = 0; nAtLink < pNode->GetLinkCount(); nAtLink++)
		{
			const CNodeLink *pLink = pNode->GetLinkAt(nAtLink);
			const CNode *pLinkedNode = pLink->GetTarget();
			dc  << "Link " << pNode->name.Get() 
				<< " to " << pLinkedNode->name.Get() 
				<< " : " << pLink->GetWeight()
				<< "\n";
		}
	}
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpace commands
