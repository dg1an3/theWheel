//////////////////////////////////////////////////////////////////////
// Space.h: interface for the CSpace class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id: Space.h,v 1.13 2007/05/28 18:46:09 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#pragma once

// associated classes
#include "Node.h"
#include "SpaceLayoutManager.h"
#include "SpaceStateVector.h"


class CSpaceView;

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
	DECLARE_ATTRIBUTE_PTR(RootNode, CNode);

	// accessors for nodes (sorted by activation)
	int GetNodeCount() const;
	CNode *GetNodeAt(int nAt);

	// adds a new node to the space as a child of the parent
	void AddNode(CNode *pNewNode, CNode *pParentNode);

	// removes a node from the space
	void RemoveNode(CNode *pMarkedNode);

	// accessors for current node
	DECLARE_ATTRIBUTE_PTR_GI(CurrentNode, CNode);

	// base path for space
	DECLARE_ATTRIBUTE(PathName, CString)
	
	// class description accessors
	CMap<CString, LPCTSTR, COLORREF, COLORREF>& GetClassColorMap();

	///////////////////////////////////////////////////////////////////
	// activation

	// activates a particular node
	void ActivateNode(CNode *pNode, REAL scale);

	// primary/secondary ratio: used for decay
	DECLARE_ATTRIBUTE(PrimSecRatio, REAL);

	// adjusts nodes so that sum of all activations = sum
	void NormalizeNodes(REAL sum = 1.0);

	// helper function to sort the nodes by activation
	void SortNodes();

	// returns the total activation of the space
	REAL GetTotalActivation(BOOL bCompute = FALSE);
	REAL GetTotalPrimaryActivation(BOOL bCompute = FALSE);
	REAL GetTotalSecondaryActivation(BOOL bCompute = FALSE);

	// accessors for the super node count
	// int GetSuperNodeCount();
	// void SetMaxSuperNodeCount(int nSuperNodeCount);

	/////////////////////////////////////////////////////////////////////////////
	// layout functions

	// sets the center of the nodes
	DECLARE_ATTRIBUTE(Center, CVectorD<3>);

	// spring constant: used for adjusting layout
	DECLARE_ATTRIBUTE(SpringConst, REAL);

	// returns a pointer to the layout manager
	DECLARE_ATTRIBUTE_PTR(LayoutManager, CSpaceLayoutManager);

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

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// recursively adds nodes to the array
	void AddNodeToArray(CNode *pNode);

	// allow CNode access to the total activation
	friend CNode;
	friend CSpaceLayoutManager;
	friend CSpaceStateVector;
	friend CSpaceView;

	// holds the computed total activation
	mutable REAL m_totalPrimaryActivation;
	mutable REAL m_totalSecondaryActivation;

	// the array of nodes
/// #define STL_COLL_SPACE_NODES
#ifdef STL_COLL_SPACE_NODES
	vector<CNode*> m_arrNodes;
#else
	CAtlArray<CNode*> m_arrNodes;
#endif

private:

	// stores the last post-super node index
	int m_nLastPostSuper;

	// flag to indicate sorting
	BOOL m_bNodesSorted;

	// the mapping from classes to colors
	CMap<CString, LPCTSTR, COLORREF, COLORREF> m_mapClassColors;

};	// class CSpace
