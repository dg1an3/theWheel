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
	void SetGain(REAL gain);
	REAL GetGainWeight() const;

	// accessors for stabilizer flag
	BOOL IsStabilizer() const;
	void SetStabilizer(BOOL bIsStabilizer = TRUE);

	// node target
	CNode *GetTarget();
	const CNode *GetTarget() const;
	void SetTarget(CNode *pTarget);

	//////////////////////////////////////////////////////////////////
	// serialization

	// serialization of this node link
	virtual void Serialize(CArchive &ar);

protected:

	// declares CSpace as a friend class, to access the helper functions
	friend CNode;

	void SetWeight(REAL weight);

	//////////////////////////////////////////////////////////////////
	// propagation helpers

	// flag to indicate that propagation has occurred through the link
	BOOL HasPropagated() const;
	void SetHasPropagated(BOOL bPropagated = TRUE);

private:
	// weight of this link
	REAL m_weight;

	// link's gain
	REAL m_gain;

	// target of this link
	CNode *m_pTarget;

	// flag to indicate this is a stabilizer link
	BOOL m_bIsStabilizer;

	// flag to indicate that propagation has already occurred
	BOOL m_bHasPropagated;
};

#endif // !defined(NODELINK_H)
