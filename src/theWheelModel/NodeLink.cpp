//////////////////////////////////////////////////////////////////////
// NodeLink.cpp: implementation of the CNodeLink class.
//
// Copyright (C) 1999-2003 Derek Graham Lane
// $Id: NodeLink.cpp,v 1.15 2007/05/28 18:45:47 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// class declaration
#include "NodeLink.h"

// the CNode
#include "Node.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const REAL PROPAGATE_THRESHOLD_WEIGHT = 0.01;	

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CNodeLink::CNodeLink
// 
// constructs a CNodeLink to the given target, with the given weight
//////////////////////////////////////////////////////////////////////
CNodeLink::CNodeLink(CNode *pToNode, REAL weight)
	: m_pTarget(pToNode),
		m_weight(weight),
		m_Gain((REAL) 1.0),
		m_IsStabilizer(FALSE),
		m_HasPropagated(TRUE)
		// initialize to HasPropagated to prevent immediate 
		//		propagation
{
}	// CNodeLink::CNodeLink


//////////////////////////////////////////////////////////////////////
// CNodeLink::~CNodeLink
// 
// destroys this node link
//////////////////////////////////////////////////////////////////////
CNodeLink::~CNodeLink()
{
}	// CNodeLink::~CNodeLink


//////////////////////////////////////////////////////////////////////
// implements CNodeLink's dynamic serialization
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CNodeLink, CObject, VERSIONABLE_SCHEMA|2);
//		2 -- added stabilizer flag
//		1 -- initial version

//////////////////////////////////////////////////////////////////////
// CNodeLink::GetWeight
// 
// returns the weight of this link
//////////////////////////////////////////////////////////////////////
REAL CNodeLink::GetWeight() const
{
	return m_weight;

}	// CNodeLink::GetWeight


//////////////////////////////////////////////////////////////////////
// CNodeLink::SetWeight
// 
// sets the weight of this link
//////////////////////////////////////////////////////////////////////
void CNodeLink::SetWeight(REAL newWeight)
{
	m_weight = newWeight;

}	// CNodeLink::SetWeight


//////////////////////////////////////////////////////////////////////
REAL 
	CNodeLink::GetGainWeight() const
	// gets the gain weight = weight * gain
{
	return 0.99 * GetGain() * GetWeight() + 0.01 * GetWeight();

}	// CNodeLink::GetGainWeight


//////////////////////////////////////////////////////////////////////
bool 
	CNodeLink::IsLinkTo(CNode *pToNode)
	// helper function for finding node links
{
	return pToNode == GetTarget();
}

//////////////////////////////////////////////////////////////////////
bool 
	CNodeLink::IsWeightGreater(CNodeLink *pLink1, CNodeLink *pLink2)
{
   return pLink1->GetWeight() > pLink2->GetWeight();
}

//////////////////////////////////////////////////////////////////////
void 
CNodeLink::PropagateActivation(CNode *pFromNode, REAL initScale, REAL alpha)
{
	// only propagate through the link if we have not already done so
	if (!GetHasPropagated()
		&& GetWeight() > PROPAGATE_THRESHOLD_WEIGHT)
	{
		// set flag to prevent re-propagation
		SetHasPropagated(TRUE);

		// compute the desired new activation = this activation * weight
		REAL targetActivation = pFromNode->GetActivation() * GetWeight();

		// attenuate if the link is a stabilizer
		if (GetIsStabilizer())
		{
			targetActivation *= (REAL) 0.25;
			pFromNode = NULL;	// prevent propagating node identity
		}

		// compute the new actual activation
		REAL deltaActivation = 
			(targetActivation - GetTarget()->GetActivation()) * initScale * GetWeight();

		// and propagate to the target
		GetTarget()->PropagateActivation(pFromNode,
			deltaActivation, initScale * alpha, alpha);
	}

}	// CNodeLink::PropagateActivation


//////////////////////////////////////////////////////////////////////
// CNodeLink::Serialize
// 
// serializes the node link
//////////////////////////////////////////////////////////////////////
void CNodeLink::Serialize(CArchive &ar)
{
	UINT nSchema = ar.GetObjectSchema();

	if (ar.IsLoading())
	{
		// load the values
		float weight;
		ar >> weight;
		m_weight = (REAL) weight;
		ar >> m_pTarget;

		if (nSchema >= 2)
		{
			ar >> m_IsStabilizer;
		}

		if (m_weight < 0.01)
			m_weight = 0.0;
	}
	else
	{
		// store the values
		ar << (float) m_weight;
		ar << m_pTarget;

		if (nSchema >= 2)
		{
			ar << m_IsStabilizer;
		}
	}

}	// CNodeLink::Serialize
