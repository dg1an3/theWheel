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
#include <Vector.h>

#include <Dib.h>

#include "NodeLink.h"

// forward declaration of the space class
class CSpace;

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
	CNode(CSpace *pSpace = NULL,
		const CString& strName = "",
		const CString& strDesc = "");
	virtual ~CNode();

	// serialization support
	DECLARE_SERIAL(CNode)

	//////////////////////////////////////////////////////////////////
	// hierarchy

	// the node's parent
	CNode *GetParent();
	const CNode *GetParent() const;
	void SetParent(CNode *pParent);

	//////////////////////////////////////////////////////////////////
	// attribute accessors

	// the node's name
	const CString& GetName() const;
	void SetName(const CString& strName);

	// the node description
	const CString& GetDescription() const;
	void SetDescription(const CString& strDesc);

	// an image filename, if present
	const CString& GetImageFilename() const;
	void SetImageFilename(const CString& strImageFilename);

	// loads the image file, if necessary
	CDib *GetDib();

	// the node description
	const CString& GetUrl() const;
	void SetUrl(const CString& strUrl);

	// the node's position
	const CVector<3>& GetPosition() const;
	void SetPosition(const CVector<3>& vPos);
	CVector<3> GetSize(float activation) const;

	//////////////////////////////////////////////////////////////////
	// link accessors

	// accessors for the node links
	int GetLinkCount() const;
	CNodeLink *GetLinkAt(int nAt);
	const CNodeLink *GetLinkAt(int nAt) const;

	// accessors for links by the target
	CNodeLink *GetLinkTo(CNode * toNode);
	const CNodeLink *GetLinkTo(CNode * toNode) const;

	// accessor to just get the link weight
	float GetLinkWeight(CNode * toNode) const;

	// creates or modifies an existing link
	void LinkTo(CNode *toNode, float weight, BOOL bReciprocalLink = TRUE);

	// clears all links
	void RemoveAllLinks();

	// sorts the links descending by weight, should be called after
	//		changing link weights
	void SortLinks();

	// hebbian learning
	void LearnFromNode(CNode *pOtherNode, float k = 0.0001f);

	//////////////////////////////////////////////////////////////////
	// activation accessors

	// accessors for the node's activation
	double GetActivation() const;
	double GetPrimaryActivation() const;
	double GetSecondaryActivation() const;

	// returns the number of descendants of this node
	int GetDescendantCount() const;

	// returns the sum of the activation of all descendants, 
	//		plus this node's activation
	double GetDescendantActivation() const;
	double GetDescendantPrimaryActivation() const;
	double GetDescendantSecondaryActivation() const;

	// returns the current maximum activator
	CNode *GetMaxActivator();

	//////////////////////////////////////////////////////////////////
	// view object

	// convenience pointer to a view object
	CObject *GetView();
	void SetView(CObject *pView);

	//////////////////////////////////////////////////////////////////
	// serialization

	// serialization of this node
	virtual void Serialize(CArchive &ar);

protected:

	// declares CSpace as a friend class, to access the helper functions
	friend CSpace;

	//////////////////////////////////////////////////////////////////
	// activation helper functions

	// set accessor sets either the primary or the secondary activation, based
	//		on whether the activator is NULL
	void SetActivation(double newActivation, CNode *pActivator = NULL);

	// propagation management
	void ResetForPropagation();
	void PropagateActivation(double scale);

	//////////////////////////////////////////////////////////////////
	// descendant helper functions

	// scales the activation of this node and all descendants by
	//		the scale amount
	void ScaleDescendantActivation(double primScale, double secScale);

	// returns a random descendant
	CNode * GetRandomDescendant();

private:
	// pointer to the space that contains this node
	CSpace *m_pSpace;

	// pointer to the node's parent
	CNode *m_pParent;

	// the node's description
	CString m_strDescription;

	// the node's image filename
	CString m_strImageFilename;

	// pointer to the DIB, if it is loaded
	CDib *m_pDib;

	// url
	CString m_strUrl;

	// position and size
	CVector<3> m_vPosition;

	// the collection of links
	CObArray m_arrLinks;
	CMap<CNode *, CNode *, float, float> m_mapLinks;

	// the current activation value of the node
	double m_primaryActivation;
	double m_secondaryActivation;

	// maximum activation delta
	double m_maxDeltaActivation;

	// stores a pointer to the maximum activator for this
	//		propagation cycle
	CNode *m_pMaxActivator;

	// convenience pointer to a view object
	CObject *m_pView;
};

#endif // !defined(AFX_NODE_H__0C8AA66B_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
