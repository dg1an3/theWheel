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

// the links
#include "NodeLink.h"

// forward declaration of the space class
class CSpace;

// forward declaration of state vector
class CSpaceStateVector;

//////////////////////////////////////////////////////////////////////
// Event Tags
//////////////////////////////////////////////////////////////////////
const LPARAM EVT_NODE_REPARENTED =		2001;
const LPARAM EVT_NODE_NAMED_CHANGED =	2002;
const LPARAM EVT_NODE_DESC_CHANGED =	2003;
const LPARAM EVT_NODE_CLASS_CHANGED =	2004;
const LPARAM EVT_NODE_IMAGE_CHANGED =	2005;
const LPARAM EVT_NODE_LINKWGT_CHANGED =	2006;
const LPARAM EVT_NODE_POSITION_CHANGED =2007;


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

	// the node class
	const CString& GetClass() const;
	void SetClass(const CString& strClass);

	// an image filename, if present
	const CString& GetImageFilename() const;
	void SetImageFilename(const CString& strImageFilename);

	// loads the image file, if necessary
	CDib *GetDib();

	// gets a Windows icon for the node (instead of DIB, if applicable)
	HICON GetIcon();

	// a sound filename, if present
	const CString& GetSoundFilename() const;
	void SetSoundFilename(const CString& strSoundFilename);

	// loads the sound file, if necessary
	LPDIRECTSOUNDBUFFER GetSoundBuffer();

	// the node description
	const CString& GetUrl() const;
	void SetUrl(const CString& strUrl);

	// the node's position
	const CVectorD<3>& GetPosition() const;
	void SetPosition(const CVectorD<3>& vPos, BOOL bFireChange = FALSE);
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

	//////////////////////////////////////////////////////////////////
	// activation accessors

	// accessors for the node's activation
	REAL GetActivation() const;
	REAL GetPrimaryActivation() const;
	REAL GetSecondaryActivation() const;

	// returns the number of descendants of this node
	int GetDescendantCount() const;

	// returns the current maximum activator
	CNode *GetMaxActivator();

	// flag for sub-threshold nodes
	BOOL IsSubThreshold();
	void SetSubThreshold(BOOL bIs = TRUE);

	// flag for post-super threshold
	BOOL IsPostSuper();
	void SetPostSuper(BOOL bIs = TRUE);

	// optimal state vector (for interpolated layout)
	CSpaceStateVector *GetOptimalStateVector();

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
	friend CSpaceStateVector;

	//////////////////////////////////////////////////////////////////
	// activation helper functions

	// set accessor sets either the primary or the secondary activation, based
	//		on whether the activator is NULL
	// over-rides can perform special functions, for instance when the
	//		activation reaches a threshold
	virtual void SetActivation(REAL newActivation, 
		CNode *pActivator = NULL, REAL weight = 0.0);

	// propagation management
	void ResetForPropagation();
	void PropagateActivation(REAL scale);

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

	// the node's class
	CString m_strClass;

	// the node's image filename
	CString m_strImageFilename;

	// pointer to the DIB, if it is loaded
	CDib *m_pDib;

	// alternatively, an icon
	HICON m_hIcon;

	// the node's sound filename
	CString m_strSoundFilename;

	// pointer to the DIB, if it is loaded
	LPDIRECTSOUNDBUFFER m_pSoundBuffer;

	// url
	CString m_strUrl;

	// position and size
	CVectorD<3> m_vPosition;

	// the collection of links
	CObArray m_arrLinks;
	CMap<CNode *, CNode *, REAL, REAL> m_mapLinks;

	// the current activation value of the node
	REAL m_primaryActivation;
	REAL m_secondaryActivation;

	// maximum activation delta
	REAL m_maxDeltaActivation;

	// stores a pointer to the maximum activator for this
	//		propagation cycle
	CNode *m_pMaxActivator;

	// flag to indicate that a max activator was found during
	//		the previous propagation
	BOOL m_bFoundMaxActivator;

	// flag to indicate that the node is sub-threshold
	BOOL m_bSubThreshold;

	// flag to indicate that the node is post-super
	BOOL m_bPostSuper;

	// optimal SSV
	CSpaceStateVector *m_pOptSSV;

	// stores rmse for positions
	REAL m_rmse;

	// convenience pointer to a view object
	CObject *m_pView;

};	// class CNode

#endif // !defined(NODE_H)
