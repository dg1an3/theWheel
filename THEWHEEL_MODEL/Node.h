// Node.h: interface for the CNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODE_H__0C8AA66B_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_NODE_H__0C8AA66B_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Value.h>
#include <Collection.h>
#include <ModelObject.h>

#include "NodeLink.h"

class CNode : public CModelObject  
{
public:
	// constructors/destructors
	CNode(const CString& strName = "", const CString& strDesc = "");
	virtual ~CNode();

	// serialization support
	DECLARE_SERIAL(CNode)

	// the node name
//	CValue< CString > name;

	// the node description
	CValue< CString > description;

	// the child nodes
	CCollection< CNode > children;

	// the node links
	CCollection< CNodeLink > links;

	// accessor helpers for the links
	void LinkTo(CNode *toNode, float weight);
	CNodeLink *GetLink(CNode * toNode);
	float GetLinkWeight(CNode * toNode);
	
	// serialization of this node
	virtual void Serialize(CArchive &ar);
};

#endif // !defined(AFX_NODE_H__0C8AA66B_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
