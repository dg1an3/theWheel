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

#include "Node.h"
#include "NodeCluster.h"

//////////////////////////////////////////////////////////////////////
// constant for total allowed activation of nodes
//////////////////////////////////////////////////////////////////////
const float TOTAL_ACTIVATION = 0.50f;


//////////////////////////////////////////////////////////////////////
// class CSpace
// 
// a space object has a collection of CNodes arranged as a containment
// hierarchy.  the CNodes are linked together.
//////////////////////////////////////////////////////////////////////
class CSpace : public CDocument
{
protected: // create from serialization only
	CSpace();
	DECLARE_DYNCREATE(CSpace)

public:

	///////////////////////////////////////////////////////////////////
	// hierarchy

	// the root node contains all of this space's nodes as children
	CNode *GetRootNode();

	// accessors for nodes (sorted by activation)
	int GetNodeCount();
	CNode *GetNodeAt(int nAt);

	// adds a new node to the space as a child of the parent
	void AddNode(CNode *pNewNode, CNode *pParentNode);

	///////////////////////////////////////////////////////////////////
	// operations

	// activates a particular node
	void ActivateNode(CNode *pNode, float scale);

	// adjusts nodes so that sum of all activations = sum
	void NormalizeNodes(double sum = 1.0);

	///////////////////////////////////////////////////////////////////
	// clusters

	// accessors for the clusters
	int GetClusterCount();
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
public:
	void ComputeClusters();

	// helper function to add random children to a node
	void AddChildren(CNode *pParent, int nLevels, 
				 int nCount = 3, float weight = 0.50f);

	// helper function to randomly cross-link nodes
	void CrossLinkNodes(int nCount, float weight = 0.50f);

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

	// the clusters for this space
	CNodeCluster *m_pCluster;

	// stores the number of super nodes
	int m_nSuperNodeCount;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACE_H__0C8AA65A_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
