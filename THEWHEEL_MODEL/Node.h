//////////////////////////////////////////////////////////////////////
// Node.h: interface for the CNode class.
//
// Copyright (C) 1999-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODE_H__0C8AA66B_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_NODE_H__0C8AA66B_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Value.h>
#include <Collection.h>
#include <ModelObject.h>

#include <Dib.h>

#include "NodeLink.h"

//////////////////////////////////////////////////////////////////////
// class CNode
// 
// a node object has a collection of CNodeLinks to other node objects
// the nodes are collected into a CSpace object.
//////////////////////////////////////////////////////////////////////
class CNode : public CModelObject  
{
public:
	// constructors/destructors
	CNode(const CString& strName = "", const CString& strDesc = "");
	virtual ~CNode();

	// serialization support
	DECLARE_SERIAL(CNode)

	// the node's parent
	CAssociation< CNode > parent;

	// the node description
	CValue< CString > description;

	// an image filename, if present
	CValue< CString > imageFilename;

	// loads the image file, if necessary
	CDib *GetDib();

	// the node links
	CCollection< CNodeLink > links;

	// accessor helpers for the links
	void LinkTo(CNode *toNode, float weight);
	CNodeLink *GetLink(CNode * toNode);
	float GetLinkWeight(CNode * toNode);

	// sorts the links descending by weight
	void SortLinks();

	// accessors for the node's activation
	double GetActivation();
	void SetActivation(double newActivation, CNode *pActivator = NULL);

	// returns the current maximum activator
	CNode *GetMaxActivator();

	// returns the sum of the activation of all descendants, 
	//		plus this node's activation
	double GetDescendantActivation();

	// scales the activation of this node and all descendants by
	//		the scale amount
	void ScaleDescendantActivation(double scale);

	// propagation management
	void ResetForPropagation();
	void PropagateActivation(double scale);

	// convenience pointer to a view object
	CObject *GetView();
	void SetView(CObject *pView);

	// serialization of this node
	virtual void Serialize(CArchive &ar);

protected:
	// pointer to the DIB, if it is loaded
	CDib *m_pDib;

	// the current activation value of the node
	double m_activation;

	// maximum activation delta
	double m_maxDeltaActivation;

	// stores a pointer to the maximum activator for this
	//		propagation cycle
	CNode *m_pMaxActivator;

	// convenience pointer to a view object
	CObject *m_pView;
};

#endif // !defined(AFX_NODE_H__0C8AA66B_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
