//////////////////////////////////////////////////////////////////////
// Node.h: interface for the CNode class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id: Node.h,v 1.17 2007/06/04 02:43:25 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#pragma once

// math vector
#include <VectorD.h>

// the image utilities
#include "Dib.h"

// the links
#include "NodeLink.h"

// forward declaration of the space class
class CSpace;


//////////////////////////////////////////////////////////////////////
// class CNode
// 
// a node object has a collection of CNodeLinks to other node objects
// the nodes are collected into a CSpace object.
//////////////////////////////////////////////////////////////////////
class CNode : public CObject  
{
public:
	// constructors/destructors
	CNode(CSpace *pSpace = NULL,
		const CString& strName = _T(""),
		const CString& strDesc = _T(""));
	virtual ~CNode();

	//////////////////////////////////////////////////////////////////
	// hierarchy

	// the space that contains the node
	CSpace *GetSpace();

	// the node's parent
	DECLARE_ATTRIBUTE_PTR_GI(Parent, CNode);

	// the node's children
	int GetChildCount() const;
	CNode *GetChildAt(int nAt);

	// returns the number of descendants of this node
	int GetDescendantCount();


	//////////////////////////////////////////////////////////////////
	// attribute accessors

	// the node's primary attributes
	DECLARE_ATTRIBUTE_GI(Name, CString);
	DECLARE_ATTRIBUTE_GI(Description, CString);
	DECLARE_ATTRIBUTE_GI(Class, CString);
	DECLARE_ATTRIBUTE_GI(ImageFilename, CString);

	// loads the image file, if necessary
	CDib *GetDib();

private:
	// pointer to the space that contains this node
	CSpace *m_pSpace;

	// the collection of children
	vector<CNode*> m_arrChildren;

	// pointer to the DIB, if it is loaded
	CDib *m_pDib;

public:

	//////////////////////////////////////////////////////////////////
	// link accessors

	// accessors for the node links
	int GetLinkCount() const;
	CNodeLink *GetLinkAt(int nAt);

	// accessors for links by the target
	CNodeLink *GetLinkTo(CNode * toNode);

	// accessor to just get the link weight
	REAL GetLinkWeight(CNode * toNode);
	REAL GetLinkGainWeight(CNode *pToNode);

	// creates or modifies an existing link
	void LinkTo(CNode *toNode, REAL weight, BOOL bReciprocalLink = TRUE);

	// unlinks a particular node (in both directions)
	void Unlink(CNode *pNode, BOOL bReciprocalLink = TRUE);

	// clears all links
	void RemoveAllLinks();

	// sorts the links descending by weight, should be called after
	//		changing link weights
	void SortLinks();

	// returns max link weight for this and all child nodes
	REAL GetMaxLinkWeight(void);

	// scales all link weights by scale factor
	void ScaleLinkWeights(REAL scale);

private:

	// the collection of links
	vector< CNodeLink* >  m_arrLinks;
	
	map< CNode*, CNodeLink* > m_mapLinks;

public:

	//////////////////////////////////////////////////////////////////
	// activation accessors

	// accessors for the node's activation
	REAL GetActivation() const;

	// set accessor sets either the primary or the secondary activation, based
	//		on whether the activator is NULL
	// over-rides can perform special functions, for instance when the
	//		activation reaches a threshold
	void SetActivation(REAL newActivation, 
		CNode *pActivator = NULL, REAL weight = 0.0);

	// primary vs. secondary activation
	REAL GetPrimaryActivation() const;
	REAL GetSecondaryActivation() const;

	// helper to sort nodes
	bool IsActivationGreater(CNode *pThanNode);
  static bool IsActivationGreaterStatic(CNode *, CNode*);

	// stores a pointer to the maximum activator for this
	//		propagation cycle
	DECLARE_ATTRIBUTE_PTR(MaxActivator, CNode);

protected:

	// declares CSpace and CNodeLink as friend classes, to access the helper functions
	friend CSpace;
	friend CNodeLink;

	//////////////////////////////////////////////////////////////////
	// activation helper functions

	// propagation management
	void PropagateActivation(CNode *pFrom, REAL deltaActivation, 
			REAL initScale, REAL alph);
	void ResetForPropagation();

	// transfers new_activation (from Propagate) to current activation for all child nodes
	void UpdateFromNewActivation(void);

private:

	//////////////////////////////////////////////////////////////////
	// activation variables 

	// the current activation value of the node
	REAL m_primaryActivation;
	REAL m_secondaryActivation;

	// stores the new activation value for the node during propagation
	REAL m_newSecondaryActivation;

	// maximum activation delta
	REAL m_maxDeltaActivation;

public:

	//////////////////////////////////////////////////////////////////
	// positioning functions

	// the node's position
	const CVectorD<3>& GetPosition() const;
	void SetPosition(const CVectorD<3>& vPos, bool bResetFlag = false);

	// read reset flag
	bool IsPositionReset(bool bClearFlag = true);
	DECLARE_ATTRIBUTE(IsResetPositionValid, bool);
	DECLARE_ATTRIBUTE(ResetPosition, CVectorD<3>);

	// returns the size for a particular activation
	REAL GetRadius() const;
	REAL GetRadiusForActivation(REAL activation) const;

	// helper to position new super-threshold nodes
	void PositionNewSuper();

	// helper to update node post-super
	void UpdatePostSuper();

	// flag for sub-threshold nodes
	DECLARE_ATTRIBUTE_VAL(IsSubThreshold, BOOL);

	// count for how long i am post super
	DECLARE_ATTRIBUTE(PostSuperCount, int);

	// convenience pointer to a view object
	DECLARE_ATTRIBUTE_PTR(View, CObject);

private:

	//////////////////////////////////////////////////////////////////
	// position variables 

	// position and size
	CVectorD<3> m_vPosition;

	// flag to indicate that the nodes position has been reset
	bool m_bPositionReset;

public:

	//////////////////////////////////////////////////////////////////
	// serialization

	// serialization support
	DECLARE_SERIAL(CNode)

	// serialization of this node
	virtual void Serialize(CArchive &ar);

};	// class CNode

