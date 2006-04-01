//////////////////////////////////////////////////////////////////////
// Node.h: interface for the CNode class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(NODE_H)
#define NODE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>

// the wave file utilities
#include "WAVE.h"

// the image utilities
#include "Dib.h"

// attribute helpers
#include "Attributes.h"

// the links
#include "NodeLink.h"

// forward declaration of the space class
class CSpace;

// forward declaration of state vector
class CSpaceStateVector;

const int UNKNOWN_DEGSEP = -1;

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
		const CString& strName = "",
		const CString& strDesc = "");
	virtual ~CNode();

	// serialization support
	DECLARE_SERIAL(CNode)

	//////////////////////////////////////////////////////////////////
	// hierarchy

	// the space that contains the node
	CSpace *GetSpace();

	// the node's parent
	DECLARE_ATTRIBUTE_PTR_GI(Parent, CNode);

	// the node's children
	int GetChildCount() const;
	CNode *GetChildAt(int nAt);
	const CNode *GetChildAt(int nAt) const;

	// returns the number of descendants of this node
	int GetDescendantCount() const;

	//////////////////////////////////////////////////////////////////
	// attribute accessors

	// the node's primary attributes
	DECLARE_ATTRIBUTE_GI(Name, CString);
	DECLARE_ATTRIBUTE_GI(Description, CString);
	DECLARE_ATTRIBUTE_GI(Class, CString);
	DECLARE_ATTRIBUTE_GI(ImageFilename, CString);

	// loads the image file, if necessary
	CDib *GetDib();

	// gets a Windows icon for the node (instead of DIB, if applicable)
	HICON GetIcon();

	// the node description
	DECLARE_ATTRIBUTE(Url, CString);

	// sound accessors, if present
	DECLARE_ATTRIBUTE(SoundFilename, CString);
	LPDIRECTSOUNDBUFFER GetSoundBuffer();

	// the node's position
	const CVectorD<3>& GetPosition() const;
	void SetPosition(const CVectorD<3>& vPos, bool bResetFlag = false);

	// read reset flag
	bool IsPositionReset(bool bClearFlag = true);

	// returns the size for a particular activation
	CVectorD<3> GetSize(REAL activation) const;

	// returns the RMSE for node position
	REAL GetRMSE();

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
	REAL GetLinkWeight(CNode * toNode) const;
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

	// returns the current maximum activator
	CNode *GetMaxActivator();

	// flag for sub-threshold nodes
	DECLARE_ATTRIBUTE(IsSubThreshold, BOOL);

	// flag for post-super threshold
	DECLARE_ATTRIBUTE(IsPostSuper, BOOL);

	// count for how long i am post super
	DECLARE_ATTRIBUTE(PostSuperCount, int);

	// convenience pointer to a view object
	DECLARE_ATTRIBUTE_PTR(View, CObject);

	//////////////////////////////////////////////////////////////////
	// serialization

	// serialization of this node
	virtual void Serialize(CArchive &ar);

protected:

	// declares CSpace as a friend class, to access the helper functions
	friend CSpace;
	friend CSpaceStateVector;

	//////////////////////////////////////////////////////////////////
	// activation helper functions

	// propagation management
	void PropagateActivation(REAL initScale, REAL alpha); // REAL scale);
	void ResetForPropagation();

	// transfers new_activation (from Propagate) to current activation for all child nodes
	void UpdateFromNewActivation(void);

#ifdef PROP_PULL_MODEL
	// updates the current activation
	void UpdateActivation(REAL scale, int nDegSep, 
							 CArray<CNode *, CNode *>& arrNextDegSep);
#endif

private:
	// pointer to the space that contains this node
	CSpace *m_pSpace;

	// the collection of children
	CObArray m_arrChildren;

	// pointer to the DIB, if it is loaded
	CDib *m_pDib;

	// alternatively, an icon
	HICON m_hIcon;

	// pointer to the DIB, if it is loaded
	LPDIRECTSOUNDBUFFER m_pSoundBuffer;

	// position and size
	CVectorD<3> m_vPosition;

	// flag to indicate that the nodes position has been reset
	bool m_bPositionReset;

	// stores rmse for positions
	REAL m_rmse;

	// the collection of links
	CObArray m_arrLinks;
	CMap<CNode *, CNode *, REAL, REAL> m_mapLinks;

	// the current activation value of the node
	REAL m_primaryActivation;
	REAL m_secondaryActivation;

	// stores the new activation value for the node during propagation
	REAL m_newActivation;	// TODO: this should be m_newSecondaryActivation

	// maximum activation delta
	REAL m_maxDeltaActivation;

	// stores a pointer to the maximum activator for this
	//		propagation cycle
	CNode *m_pMaxActivator;

	// flag to indicate that a max activator was found during
	//		the previous propagation
	BOOL m_bFoundMaxActivator;

	// stores degree of separation from initiator of propagation
	int m_nDegSep;

};	// class CNode

#endif // !defined(NODE_H)
