//////////////////////////////////////////////////////////////////////
// NodeLink.h: interface for the CNodeLink class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id: NodeLink.h,v 1.9 2007/05/28 18:46:09 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#pragma once

#include <MathUtil.h>

// forward declaration of CNode
class CNode;

//////////////////////////////////////////////////////////////////////
// class CNodeLink
// 
// represents a directed link from a source node (the parent of this
//		object) to the target node
//////////////////////////////////////////////////////////////////////
class CNodeLink : public CObject  
{
public:
	// constructors/destructors
	CNodeLink(CNode *pToNode = NULL, REAL weight = 0.0);
	virtual ~CNodeLink();

	// serialization support
	DECLARE_SERIAL(CNodeLink)

	//////////////////////////////////////////////////////////////////
	// attributes

	// node weight
	REAL GetWeight() const;

	// accessors for gain setting
	DECLARE_ATTRIBUTE(Gain, REAL);

	// gets the weight adjusted for gain
	REAL GetGainWeight() const;

	// accessors for stabilizer flag
	DECLARE_ATTRIBUTE(IsStabilizer, BOOL);

	// node target
	DECLARE_ATTRIBUTE_PTR(Target, CNode);

	// helper function for finding node links
	bool IsLinkTo(CNode *pToNode);

	// helper function for sorting
	static bool IsWeightGreater(CNodeLink *pLink1, CNodeLink *pLink2);

	//////////////////////////////////////////////////////////////////
	// serialization

#ifdef _MSC_VER
	// serialization of this node link
	virtual void Serialize(CArchive &ar);
#endif

protected:

	// declares CNode as a friend class, to access the helper functions
	friend CNode;

	// protected, because the CNode needs to update the link map if changed
	void SetWeight(REAL weight);

	//////////////////////////////////////////////////////////////////
	// propagation helpers

	void PropagateActivation(CNode *pFromNode, REAL initScale, REAL alpha);

	// flag to indicate that propagation has occurred through the link
	DECLARE_ATTRIBUTE_VAL(HasPropagated, BOOL);

private:
	// weight of this link
	REAL m_weight;

};	// class CNodeLink
