//////////////////////////////////////////////////////////////////////
// Space.h: interface for the CSpace class.
//
// Copyright (C) 1999-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPACE_H__0C8AA65A_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_SPACE_H__0C8AA65A_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Matrix.h>

#include "Node.h"
#include "NodeCluster.h"
#include "SpaceLayoutManager.h"	// Added by ClassView

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

	DECLARE_DYNCREATE(CSpace)

	///////////////////////////////////////////////////////////////////
	// hierarchy

	// the root node contains all of this space's nodes as children
	CNode *GetRootNode();

	// accessors for nodes (sorted by activation)
	int GetNodeCount() const;
	CNode *GetNodeAt(int nAt) const;

	// adds a new node to the space as a child of the parent
	void AddNode(CNode *pNewNode, CNode *pParentNode);

	///////////////////////////////////////////////////////////////////
	// operations

	// activates a particular node
	void ActivateNode(CNode *pNode, REAL scale);

	// adjusts nodes so that sum of all activations = sum
	void NormalizeNodes(REAL sum = 1.0);

	// returns the total activation of the space
	REAL GetTotalActivation() const;

	///////////////////////////////////////////////////////////////////
	// clusters

	// accessors for the clusters
	int GetClusterCount() const;
	void SetClusterCount(int nCount);
	CNodeCluster *GetClusterAt(int nAt);

	// accessors for the super node count
	int GetSuperNodeCount();
	void SetMaxSuperNodeCount(int nSuperNodeCount);

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

	// get mass statistics for the nodes
	CVector<3> GetCentralMoment();
	CMatrix<2> GetInertiaTensor();

	// get the master DirectSound object
	LPDIRECTSOUND GetDirectSound();

	// destroy the space
	virtual ~CSpace();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// recursively adds nodes to the array
	void AddNodeToArray(CNode *pNode);

	// helper function to sort the nodes by activation
	void SortNodes();

	// cluster analysis
	// void ComputeClusters();

	// helper function to add random children to a node
	void AddChildren(CNode *pParent, int nLevels, 
				 int nCount = 3, REAL weight = 0.50f);

	// helper function to randomly cross-link nodes
	void CrossLinkNodes(int nCount, REAL weight = 0.50f);

// Generated message map functions
protected:
	//{{AFX_MSG(CSpace)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// the parent node contains all of this space's nodes as children
	CNode *m_pRootNode;

	// the array of nodes
	CObArray m_arrNodes;

	// the direct sound interface
	LPDIRECTSOUND m_pDS;

	// the clusters for this space
	CNodeCluster *m_pCluster;

	// the manager for laying out the nodes
	CSpaceLayoutManager *m_pLayoutManager;

	// returns the computed total activation
	REAL m_totalActivation;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACE_H__0C8AA65A_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
