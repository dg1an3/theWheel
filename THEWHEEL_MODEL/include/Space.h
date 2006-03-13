//////////////////////////////////////////////////////////////////////
// Space.h: interface for the CSpace class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(SPACE_H)
#define SPACE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MatrixD.h>

#include <Observer.h>

#include "Attributes.h"
#include "Node.h"
#include "SpaceLayoutManager.h"
#include "SpaceStateVector.h"


//////////////////////////////////////////////////////////////////////
// constant for total allowed activation of nodes
//////////////////////////////////////////////////////////////////////
const REAL TOTAL_ACTIVATION = (REAL) 0.55;


//////////////////////////////////////////////////////////////////////
// class CSpace
// 
// a space object has a collection of CNodes arranged as a containment
// hierarchy.  the CNodes are linked together.
//////////////////////////////////////////////////////////////////////
class CSpace : public CObject
{
public:
	CSpace();

	// destroy the space
	virtual ~CSpace();

	// declare dynamic creation
	DECLARE_SERIAL(CSpace)

	///////////////////////////////////////////////////////////////////
	// hierarchy

	// the root node contains all of this space's nodes as children
	CNode *GetRootNode();

	// accessors for nodes (sorted by activation)
	int GetNodeCount() const;
	CNode *GetNodeAt(int nAt) const;

	// adds a new node to the space as a child of the parent
	void AddNode(CNode *pNewNode, CNode *pParentNode);
	CObservableEvent NodeAddedEvent;

	// removes a node from the space
	void RemoveNode(CNode *pMarkedNode);
	CObservableEvent NodeRemovedEvent;

	// accessors for current node
	DECLARE_ATTRIBUTE_PTR_GI(CurrentNode, CNode);
	CObservableEvent CurrentNodeChangedEvent;

	// node-level change events
	CObservableEvent NodeAttributeChangedEvent;

	// base path for space
	DECLARE_ATTRIBUTE(PathName, CString)
	
	///////////////////////////////////////////////////////////////////
	// operations

	// activates a particular node
	void ActivateNode(CNode *pNode, REAL scale);

	// adjusts nodes so that sum of all activations = sum
	void NormalizeNodes(REAL sum = 1.0);

	// relaxes the node link weights
	void Relax();

	// returns the total activation of the space
	REAL GetTotalActivation(BOOL bCompute = FALSE) const;
	REAL GetTotalPrimaryActivation(BOOL bCompute = FALSE) const;
	REAL GetTotalSecondaryActivation(BOOL bCompute = FALSE) const;

	// accessors for the super node count
	int GetSuperNodeCount();
	void SetMaxSuperNodeCount(int nSuperNodeCount);

	// layout parameters

	// sets the center of the nodes
	DECLARE_ATTRIBUTE(Center, CVectorD<3>);

	// primary/secondary ratio: used for decay
	DECLARE_ATTRIBUTE(PrimSecRatio, REAL);

	// spring constant: used for adjusting layout
	DECLARE_ATTRIBUTE(SpringConst, REAL);

	// event for layout parameter change
	CObservableEvent LayoutParamsChangedEvent;

	// class description accessors
	CMap<CString, LPCSTR, COLORREF, COLORREF>& GetClassColorMap();

	// get the master DirectSound object
	LPDIRECTSOUND GetDirectSound();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpace)
	public:
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:

	// sets up a simple space
	virtual BOOL CreateSimpleSpace();

	// deletes all content
	virtual void DeleteContents();

	// returns a pointer to the state vector
	CSpaceStateVector *GetStateVector();

	// returns a pointer to the layout manager
	CSpaceLayoutManager *GetLayoutManager();

	// layout the nodes
	void LayoutNodes();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// recursively adds nodes to the array
	void AddNodeToArray(CNode *pNode);

	// helper function to sort the nodes by activation
	void SortNodes();

	// helper functions for positioning nodes
	void PositionSubNodes();
	void AdjustRunawayNodes();
	void PositionNewSuperNodes();

	// allow CNode access to the total activation
	friend CNode;
	friend CSpaceStateVector;

	// holds the computed total activation
	mutable REAL m_totalPrimaryActivation;
	mutable REAL m_totalSecondaryActivation;

private:
	// the parent node contains all of this space's nodes as children
	CNode *m_pRootNode;

	// the array of nodes
	CObArray m_arrNodes;

	// the state vector for the space 
	CSpaceStateVector *m_pStateVector;

	// the manager for laying out the nodes
	CSpaceLayoutManager *m_pLayoutManager;

	// stores the last post-super node index
	int m_nLastPostSuper;

	// flag to indicate sorting
	BOOL m_bNodesSorted;

	// the mapping from classes to colors
	CMap<CString, LPCSTR, COLORREF, COLORREF> m_mapClassColors;

	// the direct sound interface
	LPDIRECTSOUND m_pDS;

};	// class CSpace


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(SPACE_H)
