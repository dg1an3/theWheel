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

#include <Value.h>
#include <Association.h>

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
	CNodeLink(CNode *pToNode = NULL, float weight = 0.0);
	virtual ~CNodeLink();

	// serialization support
	DECLARE_SERIAL(CNodeLink)

	// node weight
	CValue< float > weight;

	// node target
	CAssociation< CNode > forTarget;

	// flag to indicate that propagation has occurred through the link
	CValue<BOOL> hasPropagated;

	// serialization of this node link
	virtual void Serialize(CArchive &ar);
};

#endif // !defined(AFX_NODELINK_H__0C8AA66C_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
