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

// Attributes
public:
	// the parent node contains all of this space's nodes as children
	CNode rootNode;

// Operations
public:

	// adjusts nodes so that sum of all activations = sum
	void NormalizeNodes(double sum = 1.0);

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
	// helper function to add random children to a node
	void AddChildren(CNode *pParent, int nLevels, 
				 int nCount = 3, float weight = 0.50f);

// Generated message map functions
protected:
	//{{AFX_MSG(CSpace)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACE_H__0C8AA65A_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
