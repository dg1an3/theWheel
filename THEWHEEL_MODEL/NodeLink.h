//////////////////////////////////////////////////////////////////////
// Node.h: interface for the CNode class.
//
// Copyright (C) 1999-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODELINK_H__0C8AA66C_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_NODELINK_H__0C8AA66C_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

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

	// target of this link
	CNode *m_pTarget;

	// flag to indicate that propagation has already occurred
	BOOL m_bHasPropagated;
};

#endif // !defined(AFX_NODELINK_H__0C8AA66C_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
