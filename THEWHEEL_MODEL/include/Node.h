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

#include <Vector.h>

// the wave file utilities
#include "WAVE.h"

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

	// the node's children
	int GetChildCount() const;
	CNode *GetChildAt(int nAt);
	const CNode *GetChildAt(int nAt) const;

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

	// a sound filename, if present
	const CString& GetSoundFilename() const;
	void SetSoundFilename(const CString& strSoundFilename);

	// loads the sound file, if necessary
	LPDIRECTSOUNDBUFFER GetSoundBuffer();

	// the node description
	const CString& GetUrl() const;
	void SetUrl(const CString& strUrl);

	// the node's position
	const CVector<3>& GetPosition() const;
	void SetPosition(const CVector<3>& vPos);
	CVector<3> GetSize(REAL activation) const;

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

	// creates or modifies an existing link
	void LinkTo(CNode *toNode, REAL weight, BOOL bReciprocalLink = TRUE);

	// clears all links
	void RemoveAllLinks();

	// sorts the links descending by weight, should be called after
	//		changing link weights
	void SortLinks();

	// hebbian learning
	void LearnFromNode(CNode *pOtherNode, REAL k = 0.000001f);

	//////////////////////////////////////////////////////////////////
	// activation accessors

	// accessors for the node's activation
	REAL GetActivation() const;
	REAL GetPrimaryActivation() const;
	REAL GetSecondaryActivation() const;

	// spring activation member functions
	REAL GetSpringActivation() const;
	void UpdateSpring(REAL springConst = 0.0);

	// returns the number of descendants of this node
	int GetDescendantCount() const;

	// returns the sum of the activation of all descendants, 
	//		plus this node's activation
	REAL GetDescendantActivation() const;
	REAL GetDescendantPrimaryActivation() const;
	REAL GetDescendantSecondaryActivation() const;

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
	void SetActivation(REAL newActivation, CNode *pActivator = NULL);

	// propagation management
	void ResetForPropagation();
	void PropagateActivation(REAL scale);

	//////////////////////////////////////////////////////////////////
	// descendant helper functions

	// scales the activation of this node and all descendants by
	//		the scale amount
	void ScaleDescendantActivation(REAL primScale, REAL secScale);

	// returns a random descendant
	CNode * GetRandomDescendant();

private:
	// pointer to the space that contains this node
	CSpace *m_pSpace;

	// pointer to the node's parent
	CNode *m_pParent;

	// the collection of children
	CObArray m_arrChildren;

	// the node's name
	CString m_strName;

	// the node's description
	CString m_strDescription;

	// the node's image filename
	CString m_strImageFilename;

	// pointer to the DIB, if it is loaded
	CDib *m_pDib;

	// the node's sound filename
	CString m_strSoundFilename;

	// pointer to the DIB, if it is loaded
	LPDIRECTSOUNDBUFFER m_pSoundBuffer;

	// url
	CString m_strUrl;

	// position and size
	CVector<3> m_vPosition;

	// the collection of links
	CObArray m_arrLinks;
	CMap<CNode *, CNode *, REAL, REAL> m_mapLinks;

	// the current activation value of the node
	REAL m_primaryActivation;
	REAL m_secondaryActivation;

	// the spring activation
	REAL m_springActivation;

	// maximum activation delta
	REAL m_maxDeltaActivation;

	// stores a pointer to the maximum activator for this
	//		propagation cycle
	CNode *m_pMaxActivator;

	// convenience pointer to a view object
	CObject *m_pView;
};

#endif // !defined(AFX_NODE_H__0C8AA66B_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
