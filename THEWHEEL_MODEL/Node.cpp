// Node.cpp: implementation of the CNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "theWheel2001.h"
#include "Node.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNode::CNode(const CString& strName, const CString& strDesc)
	: parent(NULL),
		description(strDesc),
		m_pDib(NULL),
		activation(0.01f)
{
	// set the node's name
	name.Set(strName);
}

CNode::~CNode()
{
	// delete the DIB, if present
	delete m_pDib;
}

IMPLEMENT_SERIAL(CNode, CModelObject, 4);

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

void CNode::LinkTo(CNode *pToNode, float weight)
{
	// create a new node link with the target and weight
	links.Add(new CNodeLink(pToNode, weight));

	// cross-link at the same weight
	pToNode->links.Add(new CNodeLink(this, weight));
}

CNodeLink * CNode::GetLink(CNode * pToNode)
{
	// search through the links,
	for (int nAt = 0; nAt < links.GetSize(); nAt++)

		// looking for the one with the desired target
		if (links.Get(nAt)->forTarget.Get() == pToNode)

			// and return it
			return links.Get(nAt); 

	// not found?  return NULL
	return NULL;
}

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

void CNode::SortLinks()
{
	BOOL bRearrange;
	do 
	{
		bRearrange = FALSE;
		for (int nAt = 0; nAt < links.GetSize()-1; nAt++)
		{
			CNodeLink *pThisLink = links.Get(nAt);
			CNodeLink *pNextLink = links.Get(nAt+1);

			if (pThisLink->weight.Get() < pNextLink->weight.Get())
			{
				links.Set(nAt, pNextLink);
				links.Set(nAt+1, pThisLink);
				bRearrange = TRUE;
			}
		}
	} while (bRearrange);

}

void CNode::PropagateActivation(float percent, float factor)
{
	// sort the links
	SortLinks();

	// iterate through the links from highest to lowest activation
	for (int nAt = 0; nAt < links.GetSize(); nAt++)
	{
		CNodeLink *pLink = links.Get(nAt);
		CNode *pTarget = pLink->forTarget.Get();

		// compute the new activation
		double oldActivation = pTarget->activation.Get();
		if (oldActivation < percent * pLink->weight.Get())
		{
			float newActivation = 
				CNode::ActivationCurve(oldActivation * factor * 1.3f,
					percent * pLink->weight.Get() * 1.5f);
			newActivation *= (1.0005f - 0.001f * (float) rand() / (float) RAND_MAX);
			pTarget->activation.Set(newActivation);
			pTarget->PropagateActivation(newActivation);
		}
	}
}

void CNode::ResetForPropagation()
{
	// reset each of the passed node's links
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

float CNode::ActivationCurve(float a, float a_max)
{
	float frac = a / (a + a_max);

	return (1 - frac) * a + (frac * a_max);
}


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

