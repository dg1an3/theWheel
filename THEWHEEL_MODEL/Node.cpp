//////////////////////////////////////////////////////////////////////
// Node.cpp: implementation of the CNode class.
//
// Copyright (C) 1999-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "Node.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CNode::CNode
// 
// constructs a CNode object with the given name and description
//////////////////////////////////////////////////////////////////////
CNode::CNode(const CString& strName, const CString& strDesc)
	: parent(NULL),
		description(strDesc),
		m_pDib(NULL),
		// initialize with a very small activation
		m_activation(0.01),
		m_maxDeltaActivation(0.0),
		m_pMaxActivator(NULL),
		m_pView(NULL)
{
	// set the node's name
	name.Set(strName);
}

//////////////////////////////////////////////////////////////////////
// CNode::~CNode
// 
// constructs a CNode object with the given name and description
//////////////////////////////////////////////////////////////////////
CNode::~CNode()
{
	// delete the DIB, if present
	delete m_pDib;
}

//////////////////////////////////////////////////////////////////////
// implements CNode's dynamic serialization
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CNode, CModelObject, 4);

//////////////////////////////////////////////////////////////////////
// CNode::GetDib
// 
// returns a pointer to the node's image as a CDib.  loads the image
// if it is not already present
//////////////////////////////////////////////////////////////////////
CDib *CNode::GetDib()
{
	// if we have not loaded the image,
	if (m_pDib == NULL && imageFilename.Get() != "")
	{
		// create a new DIB
		m_pDib = new CDib();

		// and try to load the image
		if (!m_pDib->Load("./images/" + imageFilename.Get()))
		{
			// no luck -- delete the DIB and return NULL
			delete m_pDib;
			m_pDib = NULL;
		}
	}

	return m_pDib;
}

//////////////////////////////////////////////////////////////////////
// CNode::LinkTo
// 
// links the node to the target node (creating a CNodeLink in the
// proces, if necessary).
//////////////////////////////////////////////////////////////////////
void CNode::LinkTo(CNode *pToNode, float weight)
{
	// find any existing links
	CNodeLink *pLink = GetLink(pToNode);

	// was a link found
	if (pLink == NULL)
	{
		// create a new node link with the target and weight
		links.Add(new CNodeLink(pToNode, weight));

		// cross-link at the same weight
		pToNode->links.Add(new CNodeLink(this, weight));
	}
	else
	{
		// otherwise, just set the weight in one direction
		pLink->weight.Set(weight);
	}
}

//////////////////////////////////////////////////////////////////////
// CNode::GetLink
// 
// finds a link to the given target, if it exists.
// otherwise returns NULL.
//////////////////////////////////////////////////////////////////////
CNodeLink * CNode::GetLink(CNode * pToNode)
{
	// search through the links,
	for (int nAt = 0; nAt < links.GetSize(); nAt++)

		// looking for the one with the desired target
		if (links.Get(nAt)->forTarget.Get() == pToNode)
		{
			// and return it
			return links.Get(nAt); 
		}

	// not found?  return NULL
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CNode::GetLinkWeight
// 
// finds a link to the given target, if it exists.
// otherwise returns NULL.
//////////////////////////////////////////////////////////////////////
float CNode::GetLinkWeight(CNode * pToNode)
{
	// try to find the link
	CNodeLink *pLink = GetLink(pToNode);

	// found it?
	if (pLink)
	
		// return the weight
		return pLink->weight.Get();

	// not found? return 0.0
	return 0.0f;
}

//////////////////////////////////////////////////////////////////////
// CNode::SortLinks
// 
// sorts the links from greatest to least weight.
//////////////////////////////////////////////////////////////////////
void CNode::SortLinks()
{
	// flag to indicate a rearrangement has occurred
	BOOL bRearrange;
	do 
	{
		// initially, no rearrangement has occurred
		bRearrange = FALSE;

		// for each link,
		for (int nAt = 0; nAt < links.GetSize()-1; nAt++)
		{
			// get the current and next links
			CNodeLink *pThisLink = links.Get(nAt);
			CNodeLink *pNextLink = links.Get(nAt+1);

			// compare their weights
			if (pThisLink->weight.Get() < pNextLink->weight.Get())
			{
				// if first is less than second, swap
				links.Set(nAt, pNextLink);
				links.Set(nAt+1, pThisLink);

				// a rearrangement has occurred
				bRearrange = TRUE;
			}
		}

	// continue as long as rearrangements occur
	} while (bRearrange);

}

//////////////////////////////////////////////////////////////////////
// CNode::GetActivation
// 
// returns the node's activation
//////////////////////////////////////////////////////////////////////
double CNode::GetActivation()
{
	return m_activation;
}

//////////////////////////////////////////////////////////////////////
// CNode::SetActivation
// 
// sets the node's activation.  if an activator is passed, it is
//		compared to the current Max Activator
//////////////////////////////////////////////////////////////////////
void CNode::SetActivation(double newActivation, CNode *pActivator)
{
	// compute the delta
	double deltaActivation = newActivation - GetActivation();

	// set the activation
	m_activation = newActivation;

	// compare the delta for a new max activator
	if (deltaActivation > m_maxDeltaActivation 
		&& pActivator != NULL)
	{
		m_pMaxActivator = pActivator;
	}
}

//////////////////////////////////////////////////////////////////////
// CNode::GetMaxActivator
// 
// returns the current maximum activator
//////////////////////////////////////////////////////////////////////
CNode *CNode::GetMaxActivator()
{
	return m_pMaxActivator;
}

//////////////////////////////////////////////////////////////////////
// CNode::GetDescendantActivation
// 
// sums the activation value of this node with all children nodes
//////////////////////////////////////////////////////////////////////
double CNode::GetDescendantActivation()
{
	// initialize with the activation of this node
	double sum = GetActivation();

	// iterate over child nodes
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		// for each child node
		CNode *pNode = (CNode *)children.Get(nAt);

		// sum its descendent activation
		sum += pNode->GetDescendantActivation();
	}

	// return the sum
	return sum;
}

//////////////////////////////////////////////////////////////////////
// CNode::ScaleDescendantActivation
// 
// sums the activation value of this node with all children nodes
//////////////////////////////////////////////////////////////////////
void CNode::ScaleDescendantActivation(double scale)
{
	// scale this node's activation
	SetActivation(GetActivation() * scale);

	// iterate over child nodes
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		// for each child node
		CNode *pNode = (CNode *)children.Get(nAt);

		// scale the child's activation
		pNode->ScaleDescendantActivation(scale);
	}
}

//////////////////////////////////////////////////////////////////////
// CNode::PropagateActivation
// 
// propagates the activation of this node through the other nodes
//		in the space
//////////////////////////////////////////////////////////////////////
void CNode::PropagateActivation(double scale)
{
	// sort the links
	SortLinks();

	// iterate through the links from highest to lowest activation
	for (int nAt = 0; nAt < links.GetSize(); nAt++)
	{
		// get the link
		CNodeLink *pLink = links.Get(nAt);

		// only propagate through the link if we have not already done so
		if (!pLink->hasPropagated.Get())
		{
			// mark this link as having propagated
			pLink->hasPropagated.Set(TRUE);

			// get the link target
			CNode *pTarget = pLink->forTarget.Get();

			// compute the desired new activation = this activation * weight
			double targetActivation = GetActivation() 
				* pLink->weight.Get();

			// perturb the new activation
			targetActivation *= 
				(1.0005f - 0.001f * (float) rand() / (float) RAND_MAX);

			// the new activation defaults to the original activation
			double newActivation = pTarget->GetActivation();

			// now change it if the current target activation is less than the target
			if (pTarget->GetActivation() < targetActivation)
			{
				// compute the new actual activation
				newActivation = pTarget->GetActivation() + 
					(targetActivation - pTarget->GetActivation()) * scale;
			}

			// set the new actual activation
			pTarget->SetActivation(newActivation, this);

			// and propagate to linked nodes
			pTarget->PropagateActivation(scale);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CNode::ResetForPropagation
// 
// resets the "hasPropagated" flags on the link weights
//////////////////////////////////////////////////////////////////////
void CNode::ResetForPropagation()
{
	// reset the max delta activation
	m_maxDeltaActivation = 0.0;
	m_pMaxActivator = NULL;

	// reset each of the node's links
	for (int nAt = 0; nAt < links.GetSize(); nAt++)
	{
		CNodeLink *pLink = links.Get(nAt);
		pLink->hasPropagated.Set(FALSE);
	}

	// now do the same for all children
	for (nAt = 0; nAt < children.GetSize(); nAt++)
	{
		// for each child,
		CNode *pChildNode = (CNode *) children.Get(nAt);
		// now recursively reset the children
		pChildNode->ResetForPropagation();
	}
}

//////////////////////////////////////////////////////////////////////
// CNode::GetView
// 
// returns the view object for this node
//////////////////////////////////////////////////////////////////////
CObject *CNode::GetView()
{
	return m_pView;
}

//////////////////////////////////////////////////////////////////////
// CNode::SetView
// 
// sets the view object for this node
//////////////////////////////////////////////////////////////////////
void CNode::SetView(CObject *pView)
{
	m_pView = pView;
}

//////////////////////////////////////////////////////////////////////
// CNode::Serialize
// 
// serialize the node
//////////////////////////////////////////////////////////////////////
void CNode::Serialize(CArchive &ar)
{
	// serialize the node name
	name.Serialize(ar);

	// serialize the node body
	description.Serialize(ar);

	// serialize the image filename
	imageFilename.Serialize(ar);

	// serialize children
	children.Serialize(ar);

	// serialize links
	links.Serialize(ar);

	// if we are loading,
	if (ar.IsLoading())
	{
		// set the children's parent
		for (int nAt = 0; nAt < children.GetSize(); nAt++)
			((CNode *)children.Get(nAt))->parent.Set(this);

		// and sort the links
		SortLinks();
	}
}



CNode * CNode::GetRandomDescendant()
{
	int nDescendant = rand() * GetDescendantCount() / RAND_MAX;

	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		CNode *pChild = ((CNode *)children.Get(nAt));
		if (pChild->GetDescendantCount() > nDescendant)
			return pChild->GetRandomDescendant();
		nDescendant -= pChild->GetDescendantCount();
	}

	return (CNode *) children.Get(nDescendant);
}

int CNode::GetDescendantCount()
{
	int nCount = 0;
	for (int nAt = 0; nAt < children.GetSize(); nAt++)
	{
		CNode *pChild = ((CNode *)children.Get(nAt));
		nCount += pChild->GetDescendantCount() + 1;		// 1 for the child itself
	}

	return nCount;
}
