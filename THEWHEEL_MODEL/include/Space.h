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

#include "Node.h"
#include "SpaceLayoutManager.h"

//////////////////////////////////////////////////////////////////////
// constant for total allowed activation of nodes
//////////////////////////////////////////////////////////////////////
const REAL TOTAL_ACTIVATION = (REAL) 0.50;


//////////////////////////////////////////////////////////////////////
// class CSpace
// 
// a space object has a collection of CNodes arranged as a containment
// hierarchy.  the CNodes are linked together.
//////////////////////////////////////////////////////////////////////
class CSpace : public CDocument
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

	// removes a node from the space
	void RemoveNode(CNode *pMarkedNode);

	// accessors for current node
	CNode *GetCurrentNode();
	void SetCurrentNode(CNode *pNode);

	///////////////////////////////////////////////////////////////////
	// operations

	// activates a particular node
	void ActivateNode(CNode *pNode, REAL scale);

	// adjusts nodes so that sum of all activations = sum
	void NormalizeNodes(REAL sum = 1.0);

	// returns the total activation of the space
	REAL GetTotalActivation(BOOL bCompute = FALSE) const;
	REAL GetTotalPrimaryActivation(BOOL bCompute = FALSE) const;
	REAL GetTotalSecondaryActivation(BOOL bCompute = FALSE) const;

	// accessors for the super node count
	int GetSuperNodeCount();
	void SetMaxSuperNodeCount(int nSuperNodeCount);

	// accessors for primary/secondary ratio
	double GetPrimSecRatio() const;
	void SetPrimSecRatio(double primSecRatio);

	// accessors for spring constant
	double GetSpringConst();
	void SetSpringConst(double springConst);

	// class description accessors
	CMap<CString, LPCSTR, COLORREF, COLORREF>& GetClassColorMap();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpace)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	// returns a pointer to the layout manager
	CSpaceLayoutManager *GetLayoutManager();

	// layout the nodes
	void LayoutNodes();

	// sets the center of the node views
	void SetCenter(double x, double y);

	// get the master DirectSound object
	LPDIRECTSOUND GetDirectSound();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// recursively adds nodes to the array
	void AddNodeToArray(CNode *pNode);

	// helper function to sort the nodes by activation
	void SortNodes();

// Generated message map functions
protected:
	//{{AFX_MSG(CSpace)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// allow CNode access to the total activation
	friend CNode;

	// holds the computed total activation
	mutable REAL m_totalPrimaryActivation;
	mutable REAL m_totalSecondaryActivation;

private:
	// the parent node contains all of this space's nodes as children
	CNode *m_pRootNode;

	// the array of nodes
	CObArray m_arrNodes;

	// the currently selected node
	CNode *m_pCurrentNode;

	// the direct sound interface
	LPDIRECTSOUND m_pDS;

	// the manager for laying out the nodes
	CSpaceLayoutManager *m_pLayoutManager;

	// the center of the node views
	CVectorD<3> m_vCenter;

	// primary/secondary ratio for the space
	double m_primSecRatio;

	// spring constant for node views
	double m_springConst;

	// the mapping from classes to colors
	CMap<CString, LPCSTR, COLORREF, COLORREF> m_mapClassColors;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(SPACE_H)
