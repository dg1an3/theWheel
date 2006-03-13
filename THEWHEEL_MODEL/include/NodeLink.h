//////////////////////////////////////////////////////////////////////
// NodeLink.h: interface for the CNodeLink class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(NODELINK_H)
#define NODELINK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MathUtil.h>

#include "Attributes.h"

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

	//////////////////////////////////////////////////////////////////
	// serialization

	// serialization of this node link
	virtual void Serialize(CArchive &ar);

protected:

	// declares CSpace as a friend class, to access the helper functions
	friend CNode;

	// protected, because the CNode needs to update the link map if changed
	void SetWeight(REAL weight);

	//////////////////////////////////////////////////////////////////
	// propagation helpers

	// flag to indicate that propagation has occurred through the link
	DECLARE_ATTRIBUTE(HasPropagated, BOOL);

private:
	// weight of this link
	REAL m_weight;

	// returns target activation w.r.t. source node
	REAL GetTargetActivation(void);
	REAL m_targetActivation;
	CNode *m_pFrom;

};	// class CNodeLink


#endif // !defined(NODELINK_H)
