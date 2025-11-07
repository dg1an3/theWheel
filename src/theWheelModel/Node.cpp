//////////////////////////////////////////////////////////////////////
// Node.cpp: implementation of the CNode class.
//
// Copyright (C) 1999-2003 Derek Graham Lane
// $Id: Node.cpp,v 1.39 2007/06/04 02:43:47 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "Node.h"

// include the document class
#include "Space.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// PROPAGATE_THRESHOLD_WEIGHT
// 
// constant defining threshold for propagation
//////////////////////////////////////////////////////////////////////
const REAL PROPAGATE_THRESHOLD_WEIGHT = 0.01f;		// TODO: move to CNodeLink

//////////////////////////////////////////////////////////////////////
// Event Firing
//////////////////////////////////////////////////////////////////////
#define NODE_FIRE_CHANGE() /*\
	if (m_pSpace) m_pSpace->NodeAttributeChangedEvent.Fire(this);*/

//////////////////////////////////////////////////////////////////////
const REAL PRIM_FRAC = 0.5f;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////	
CNode::CNode(CSpace *pSpace,
			 const CString& strName, 
			 const CString& strDesc)
	// constructs a CNode object with the given name and description
	: m_pSpace(pSpace)
		, m_pParent(NULL)

		, m_pDib(NULL)

		, m_primaryActivation((REAL) 0.005)		// initialize with a very 
		, m_secondaryActivation((REAL) 0.005)		// small activation

		, m_pMaxActivator(NULL)
		, m_maxDeltaActivation((REAL) 0.0)

		, m_IsSubThreshold(TRUE)
		, m_PostSuperCount(0)
		, m_bPositionReset(false)
		, m_IsResetPositionValid(false)

		, m_pView(NULL)
{
	SetName(strName);
	SetDescription(strDesc);

	// TODO: fix this (why is this being initialized?)
	m_vPosition[0] = 0.0;
	m_vPosition[1] = 0.0;
	m_vPosition[2] = 0.0;

}	// CNode::CNode


//////////////////////////////////////////////////////////////////////
CNode::~CNode()		
	// constructs a CNode object with the given name and description
{
	// delete the children
	for (auto iterNode = m_arrChildren.begin(); iterNode != m_arrChildren.end(); iterNode++)
		delete (*iterNode);

	// delete the links	
	for (auto iterLink = m_arrLinks.begin(); iterLink != m_arrLinks.end(); iterLink++)
		delete (*iterLink);

	// delete the DIB, if present
	if (m_pDib)
	{
		delete m_pDib;
	}

}	// CNode::~CNode


//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CNode, CObject, VERSIONABLE_SCHEMA|8);
// implements CNode's dynamic serialization
//		8 -- added optimal SSV
//		7 -- added activation and position
//		6 -- added class description
//		5 -- added URL


//////////////////////////////////////////////////////////////////////
CSpace *CNode::GetSpace()
	// returns a pointer to the space containing the node
{
	return m_pSpace;

}	// CNode::GetSpace


//////////////////////////////////////////////////////////////////////
void CNode::SetParent(CNode *pParent)
	// sets a pointer to the node's parent
{
	// if this currently has a parent
	if (m_pParent)
	{
		// find this in the children array
		auto iter = std::remove(m_pParent->m_arrChildren.begin(), m_pParent->m_arrChildren.end(), this);
		m_pParent->m_arrChildren.erase(iter, m_pParent->m_arrChildren.end());
	}

	// set the parent pointer
	m_pParent = pParent;

	// if a parent has been set,
	if (m_pParent)
	{
		// set the space, if it hasn't been set yet
		if (NULL == m_pSpace)
		{
			m_pSpace = pParent->m_pSpace;
		}

		// add to the children array
	    m_pParent->m_arrChildren.push_back(this); 
	}

}	// CNode::SetParent


//////////////////////////////////////////////////////////////////////
int CNode::GetChildCount() const
	// returns the number of child nodes for this node
{
	return (int) m_arrChildren.size();

}	// CNode::GetChildCount


//////////////////////////////////////////////////////////////////////
CNode *CNode::GetChildAt(int nAt)
	// returns the child node at the given index
{
	return m_arrChildren.at(nAt);

}	// CNode::GetChildAt


//////////////////////////////////////////////////////////////////////
int CNode::GetDescendantCount()
	// returns the number of descendants
{
	int nCount = 0;
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		CNode *pChild = GetChildAt(nAt);
		nCount += pChild->GetDescendantCount() + 1;		// 1 for the child itself
	}

	return nCount;

}	// CNode::GetDescendantCount


//////////////////////////////////////////////////////////////////////
void CNode::SetName(const CString& strName)
	// sets the name of the node
{
	m_Name = strName;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetName


//////////////////////////////////////////////////////////////////////
void CNode::SetDescription(const CString& strDesc)
	// sets the node's description text
{
	m_Description = strDesc;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetDescription


//////////////////////////////////////////////////////////////////////
void 
	CNode::SetClass(const CString& strClass)
	// sets the node class	
{
	m_Class = strClass;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetClass



//////////////////////////////////////////////////////////////////////
void 
	CNode::SetImageFilename(const CString& strImageFilename)
	// sets the name of the node's image file
{
	m_ImageFilename = strImageFilename;

	// trigger re-loading of image filename
	if (m_pDib != NULL)
	{
		delete m_pDib;
		m_pDib = NULL;
	}

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetImageFilename


//////////////////////////////////////////////////////////////////////
CDib *
	CNode::GetDib()
	// returns a pointer to the node's image as a CDib.  loads the image
	// if it is not already present
{
	// if we have not loaded the image,
	if (m_pDib == NULL && GetImageFilename() != "")
	{
		// create a new DIB
		m_pDib = new CDib();

		// and try to load the image
		const CString& strPath = GetSpace()->GetPathName();
		int nLastSlash = strPath.ReverseFind('\\');
		CString strImagePath = strPath.Left(nLastSlash);
		if (!m_pDib->Load(strImagePath + "/images/" + GetImageFilename()))
		{
			// no luck -- delete the DIB and return NULL
			delete m_pDib;
			m_pDib = NULL;
		}
	}

	return m_pDib;

}	// CNode::GetDib

/// TODO: should position + size stuff be in an adjacent class?
//////////////////////////////////////////////////////////////////////
const 
	CVectorD<3>& CNode::GetPosition() const
	// returns the position of the node
{
	return m_vPosition;

}	// CNode::GetPosition


//////////////////////////////////////////////////////////////////////
void 
	CNode::SetPosition(const CVectorD<3>& vPos, bool bResetFlag)
	// sets the position of the node
{
	m_vPosition = vPos;

	if (bResetFlag)
	{
		m_bPositionReset = true;
	}

}	// CNode::SetPosition


//////////////////////////////////////////////////////////////////////
bool CNode::IsPositionReset(bool bClearFlag)
	// returns flag to indicate if position has been reset; optionally
	//		clears flag
{
	bool bFlag = m_bPositionReset;
	if (bClearFlag)
	{
		m_bPositionReset = false;
	}

	return bFlag;

}	// CNode::IsPositionReset


//////////////////////////////////////////////////////////////////////
REAL 
	CNode::GetRadius() const
{
	return GetRadiusForActivation(GetActivation());
}

//////////////////////////////////////////////////////////////////////
REAL 
	CNode::GetRadiusForActivation(REAL activation) const
{
	return (REAL) sqrt(activation);
}


//////////////////////////////////////////////////////////////////////
int CNode::GetLinkCount() const
	// returns the number of links for this node
{
	return (int) m_arrLinks.size(); 

}	// CNode::GetLinkCount


// finds a link at the given index position.
//////////////////////////////////////////////////////////////////////
CNodeLink *CNode::GetLinkAt(int nAt)
{
	return m_arrLinks.at(nAt); 

}	// CNode::GetLinkAt


// finds a link to the given target, if it exists.
// otherwise returns NULL.
//////////////////////////////////////////////////////////////////////
CNodeLink * CNode::GetLinkTo(CNode * pToNode)
{
	// find the weight in the map
	auto iter = m_mapLinks.find(pToNode);
	if (iter != m_mapLinks.end())
	{	
		// return the weight
		return iter->second;
	}

	// not found?  return NULL
	return NULL;

}	// CNode::GetLinkTo


// finds a link to the given target, if it exists.
// otherwise returns NULL.
//////////////////////////////////////////////////////////////////////
REAL CNode::GetLinkWeight(CNode * pToNode)
{
	// find the weight in the map
	auto iter = m_mapLinks.find(pToNode);
	if (iter != m_mapLinks.end())
	{	
		// return the weight
		return iter->second->GetWeight();
	}

	// not found? return 0.0
	return (REAL) 0.0;

}	// CNode::GetLinkWeight


// returns the gained weight for the given link
//////////////////////////////////////////////////////////////////////
REAL CNode::GetLinkGainWeight(CNode *pToNode)
{
	CNodeLink *pLink = GetLinkTo(pToNode);
	if (pLink && !pLink->GetIsStabilizer())
	{
		return pLink->GetGainWeight();
	}

	// not found? return 0.0
	return (REAL) 0.0;

}	// CNode::GetLinkGainWeight


// links the node to the target node (creating a CNodeLink in the
// proces, if necessary).
//////////////////////////////////////////////////////////////////////
void CNode::LinkTo(CNode *pToNode, REAL weight, BOOL bReciprocalLink)
{
	// don't link to NULL
	ASSERT(pToNode != NULL);

	// don't self-link
	if (pToNode == this)
	{
		return;
	}

	// find any existing links
	CNodeLink *pLink = GetLinkTo(pToNode);

	// was a link found
	if (pLink == NULL)
	{
		// create a new node link with the target and weight
		CNodeLink *pLink = new CNodeLink(pToNode, weight);
		m_arrLinks.push_back(pLink);

		// add to the map
		m_mapLinks.insert(make_pair(pToNode, pLink));

		// cross-link at the same weight
		if (bReciprocalLink)
		{
			pToNode->LinkTo(this, weight, FALSE);
		}
	}
	else
	{
		// otherwise, just set the weight in one direction
		pLink->SetWeight(weight);

		// cross-link at the same weight
		if (bReciprocalLink)
		{
			REAL otherWeight = pToNode->GetLinkWeight(this);
			pToNode->LinkTo(this, (weight + otherWeight) * (REAL) 0.5, FALSE);
		}

		// and update the map
		/// m_mapLinks.insert(make_pair(pToNode, pLink));
	}

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::LinkTo


// removes a link
//////////////////////////////////////////////////////////////////////
void CNode::Unlink(CNode *pNode, BOOL bReciprocalLink)
{
	// search through the links,
	for (auto iter = m_arrLinks.begin(); iter != m_arrLinks.end(); iter++)
	{
		// looking for the one with the desired target
		if ((*iter)->GetTarget() == pNode)
		{
			delete (*iter);
			m_arrLinks.erase(iter);

			// break from the loop
			break;
		}
	}

	/// TODO: does this delete the link object???

	// remove from the weight map
	auto iter_map = m_mapLinks.find(pNode);
	if (iter_map != m_mapLinks.end())
		m_mapLinks.erase(iter_map);

	// if the reciprocal link should be removed,
	if (bReciprocalLink)
	{
		// then unlink in the other direction
		pNode->Unlink(this, FALSE);
	}

	// fire change
	NODE_FIRE_CHANGE();
}

// removes all the links for the node
//////////////////////////////////////////////////////////////////////
void CNode::RemoveAllLinks()
{
	for (auto iter = m_arrLinks.begin(); iter != m_arrLinks.end(); iter++)
	{
		delete (*iter);
	}
	m_arrLinks.clear();
	m_mapLinks.clear();

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::RemoveAllLinks


//////////////////////////////////////////////////////////////////////
REAL 
	CNode::GetMaxLinkWeight(void)
	// returns max link weight for this and all child nodes
{	
	// find the max of my link weights
	auto iterMax = max_element(m_arrLinks.begin(), m_arrLinks.end(), 
		&CNodeLink::IsWeightGreater);
	REAL maxWeight = (*iterMax)->GetWeight();

	// now do the same for all children
	for (auto iterNode = m_arrChildren.begin(); iterNode != m_arrChildren.end(); iterNode++)
		maxWeight = __max(maxWeight, (*iterNode)->GetMaxLinkWeight());

	return maxWeight;
}

//////////////////////////////////////////////////////////////////////
REAL ScaleWeight(REAL scale, REAL weight)
{
	ASSERT(weight >= 0.0);
	ASSERT(weight < 1.0);
	REAL scaledXformWeight = (-logf(1.0f - weight) * scale);
	return 0.99f * (1.0f - expf(-scaledXformWeight));
}

//////////////////////////////////////////////////////////////////////
void 
	CNode::ScaleLinkWeights(REAL scale)
	// scales all link weights by scale factor
{
	// scale each of the node's links
	for (auto iterLink = m_arrLinks.begin(); iterLink != m_arrLinks.end(); iterLink++)
		(*iterLink)->SetWeight(ScaleWeight(scale, (*iterLink)->GetWeight()));

	// now do the same for all children
	for (auto iterNode = m_arrChildren.begin(); iterNode != m_arrChildren.end(); iterNode++)
		(*iterNode)->ScaleLinkWeights(scale);
}


//////////////////////////////////////////////////////////////////////
REAL 
	CNode::GetActivation() const
	// returns the node's activation
{
	return m_primaryActivation + m_secondaryActivation;

}	// CNode::GetActivation


//////////////////////////////////////////////////////////////////////
void 
	CNode::SetActivation(REAL newActivation, CNode *pActivator, REAL weight)
	// sets the node's activation.  if an activator is passed, it is
	//		compared to the current Max Activator
{
	// compute the delta
	REAL deltaActivation = newActivation - GetActivation();

	// set the activation, based on whether primary or secondary
	if (pActivator == NULL)
	{
		m_primaryActivation += PRIM_FRAC * deltaActivation;
		m_secondaryActivation += (1.0f - PRIM_FRAC) * deltaActivation;

		// update the total activation value
		if (m_pSpace)
		{
			m_pSpace->m_totalPrimaryActivation += PRIM_FRAC * deltaActivation;
			m_pSpace->m_totalSecondaryActivation += (1.0f - PRIM_FRAC) * deltaActivation;
		}
	}
	else
	{
		m_secondaryActivation += deltaActivation;

		// update the total activation value
		if (m_pSpace)
		{
			m_pSpace->m_totalSecondaryActivation += deltaActivation;
		}
	}

}	// CNode::SetActivation


//////////////////////////////////////////////////////////////////////
REAL 
	CNode::GetPrimaryActivation() const
	// returns the node's primary activation
{
	return m_primaryActivation;

}	// CNode::GetPrimaryActivation


//////////////////////////////////////////////////////////////////////
REAL 
	CNode::GetSecondaryActivation() const
	// returns the node's secondary activation
{
	return m_secondaryActivation;

}	// CNode::GetSecondaryActivation


//////////////////////////////////////////////////////////////////////
bool 
	CNode::IsActivationGreater(CNode *pThanNode)
	// helper to sort nodes
{
	REAL act1 = GetActivation();
	// now add additional factor for post-super count: max 40 adds 0.1 to activation
	act1 += 0.1f * GetPostSuperCount() / 40.0f;

	REAL act2 = pThanNode->GetActivation();
	act2 += 0.1f * pThanNode->GetPostSuperCount() / 40.0f;

	return act1 > act2;
}

//////////////////////////////////////////////////////////////////////
bool 
	CNode::IsActivationGreaterStatic(CNode *pLeft, CNode *pRight)
	// helper to sort nodes
{
	REAL act1 = pLeft->GetActivation();
	// now add additional factor for post-super count: max 40 adds 0.1 to activation
	act1 += 0.1f * pLeft->GetPostSuperCount() / 40.0f;

	REAL act2 = pRight->GetActivation();
	act2 += 0.1f * pRight->GetPostSuperCount() / 40.0f;

	return act1 > act2;
}


//////////////////////////////////////////////////////////////////////
void 
	CNode::PositionNewSuper()
	// helper to position new super-threshold nodes
{
	/// ADDED to set for new super nodes
	if (GetIsSubThreshold())
	{
		// set default position
		SetPosition(CVectorD<3>(0.0, 0.0, 0.0), true);	// to flag reset

		// set its position based on max activator, if it has one
		CNode *pMaxAct = GetMaxActivator();
		if (pMaxAct != NULL)
		{
			// set the new position
			SetPosition(pMaxAct->GetPosition(), true);	// to flag reset
			SetIsResetPositionValid(true);
			SetResetPosition(pMaxAct->GetPosition());
		}

		// set gain to full for all links
		for (int nAtLink = 0; nAtLink < GetLinkCount(); nAtLink++)
		{
			// turn on full gain
			GetLinkAt(nAtLink)->SetGain(1.0);

			// and reciprocal link's gain as well
			CNode *pLinkTo = GetLinkAt(nAtLink)->GetTarget();
			CNodeLink *pLinkToLink = pLinkTo->GetLinkTo(this);
			if (pLinkToLink != NULL)
			{
				pLinkToLink->SetGain(1.0);
			}
		}
	}

}	// CNode::PositionNewSuper


//////////////////////////////////////////////////////////////////////
void 
	CNode::UpdatePostSuper()
	// helper to update node post-super
{
	// adjust the sub-threshold nodes
	if (GetIsSubThreshold())
	{
		// and flag as no longer sub threshold
		SetIsSubThreshold(FALSE);

		// set the post super count
		SetPostSuperCount(5);
	}
	else if (GetPostSuperCount() > 0)
	{
		// decrement post super count
		m_PostSuperCount--;
	}


	if (GetIsResetPositionValid())
	{
		SetResetPosition(GetPosition());
	}

}	// CNode::UpdatePostSuper

//////////////////////////////////////////////////////////////////////
void 
	CNode::PropagateActivation(CNode *pFrom, REAL deltaActivation, 
			REAL initScale, REAL alpha)
	// updates node's activation and propagates through the other nodes
	//		in the space
{
	if (deltaActivation > 0.0)
	{
		// add to the new activation
		m_newSecondaryActivation += deltaActivation;

		// are we the max activator???
		if (pFrom != NULL
			&& deltaActivation > m_maxDeltaActivation)
		{
			m_pMaxActivator = pFrom;
			m_maxDeltaActivation = deltaActivation;
		}
	}

	// propagate through all links
	for (auto iter = m_arrLinks.begin(); iter != m_arrLinks.end(); iter++)
		(*iter)->PropagateActivation(this, initScale, alpha);

}	// CNode::PropagateActivation


//////////////////////////////////////////////////////////////////////
void 
	CNode::ResetForPropagation()
	// resets the "hasPropagated" flags on the link weights
{
	// reset all node link propagation flags
	for_each(m_arrLinks.begin(), m_arrLinks.end(),
		[](auto pLink) { pLink->SetHasPropagated(FALSE); });

	// reset new activation
	m_newSecondaryActivation = m_secondaryActivation;

	// reset max activator stuff
	m_maxDeltaActivation = 0.0;
	m_pMaxActivator = NULL;

	// recursively call for children
	for_each(m_arrChildren.begin(), m_arrChildren.end(),
		[](auto pLink) { pLink->ResetForPropagation(); });


}	// CNode::ResetForPropagation


//////////////////////////////////////////////////////////////////////
void 
	CNode::UpdateFromNewActivation(void)
	// transfers new_activation (from Propagate) to current activation for all child nodes
{
	// update the total activation value
	if (m_pSpace)
	{
		m_pSpace->m_totalSecondaryActivation += 
			(m_newSecondaryActivation - m_secondaryActivation);
	}

	// check this
	m_secondaryActivation = m_newSecondaryActivation;

	// recursively call for children
	for_each(m_arrChildren.begin(), m_arrChildren.end(),
		[](auto pChild) { pChild->UpdateFromNewActivation();  });
}


//////////////////////////////////////////////////////////////////////
// CNode::Serialize
// 
// serialize the node
//////////////////////////////////////////////////////////////////////
void CNode::Serialize(CArchive &ar)
{
	UINT nSchema = ar.GetObjectSchema();

	// serialize the node body
	if (ar.IsLoading())
	{
		ar >> m_Name;
		ar >> m_Description;
		ar >> m_ImageFilename;

		if (nSchema >= 5)
		{
			// placeholder for SetUrl(strUrl);
			CString strUrl;
			ar >> strUrl;
		}

		if (nSchema >= 6)
		{
			ar >> m_Class;
		}

		if (nSchema >= 7)
		{
			double primaryActivation;
			ar >> primaryActivation;
			m_primaryActivation = (REAL) primaryActivation;

			double secondaryActivation;
			ar >> secondaryActivation;
			m_secondaryActivation = (REAL)secondaryActivation;

			CVectorD<3, double> vPosition;
			ar >> vPosition;
			m_vPosition[0] = (REAL) vPosition[0];
			m_vPosition[1] = (REAL) vPosition[1];
			m_vPosition[2] = (REAL) vPosition[2];

			ar >> m_pMaxActivator;
			double maxDeltaActivation;
			ar >> maxDeltaActivation;
			m_maxDeltaActivation = (REAL) maxDeltaActivation;
		}

		if (nSchema >= 8)
		{
			// placeholder for m_pOptSSV
			CObject *pOptSSV;
			ar >> pOptSSV;
			delete pOptSSV;
		}
	}
	else
	{
		ar << GetName();
		ar << m_Description;
		ar << m_ImageFilename;

		// schema 5 (URL)
		// placeholder for GetUrl();
		CString strUrl;
		ar << strUrl;

		// schema 6
		ar << m_Class;

		// schema 7
		ar << (double) m_primaryActivation;
		ar << (double) m_secondaryActivation;

		CVectorD<3, double> vPosition;
		vPosition[0] = m_vPosition[0];
		vPosition[1] = m_vPosition[1];
		vPosition[2] = m_vPosition[2];
		ar << vPosition;

		ar << m_pMaxActivator;
		ar << (double) m_maxDeltaActivation;

		if (nSchema >= 8)
		{
			CObject *pOptSSV = NULL;
			ar << pOptSSV;
		}
	}

	// serialize children
	CObArray arrChildren;

	if (ar.IsStoring())
	{
		for (auto iter = m_arrChildren.begin(); iter != m_arrChildren.end(); iter++)
			arrChildren.Add(*iter);
	}
	arrChildren.Serialize(ar);
	if (ar.IsLoading())
	{
		// populate link array
		m_arrChildren.clear();
		for (int nAt = 0; nAt < arrChildren.GetSize(); nAt++)
			m_arrChildren.push_back((CNode*) arrChildren[nAt]);
	}

	// serialize links
	CObArray arrLinks;

	if (ar.IsStoring())
	{
		for (auto iter = m_arrLinks.begin(); iter != m_arrLinks.end(); iter++)
		{
			if ((*iter)->GetWeight() > 1e-6)
				arrLinks.Add(*iter);
		}
	}

	arrLinks.Serialize(ar);

	if (ar.IsLoading())
	{
		// populate link array
		m_arrLinks.clear();
		for (int nAt = 0; nAt < arrLinks.GetSize(); nAt++)
			m_arrLinks.push_back((CNodeLink*) arrLinks[nAt]);
	}

	// if we are loading,
	if (ar.IsLoading())
	{
		// set the children's parent
		for (auto nAt = 0; nAt < GetChildCount(); nAt++)
		{
			GetChildAt(nAt)->m_pParent = this;
		}

		// sort the links
		// SortLinks();

		// and set up the map
		m_mapLinks.clear();
		for (int nAt = 0; nAt < GetLinkCount(); nAt++)
		{
			m_mapLinks.insert(make_pair(GetLinkAt(nAt)->GetTarget(), GetLinkAt(nAt)));
		}
	}

}	// CNode::Serialize

