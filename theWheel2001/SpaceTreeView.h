// SpaceTreeView.h : interface of the CSpaceTreeView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPACETREEVIEW_H__0C8AA65E_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_SPACETREEVIEW_H__0C8AA65E_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Collection.h>

#include <ObjectExplorer.h>

#include "Node.h"

class CSpace;

class CSpaceTreeView : public CView
{
protected: // create from serialization only
	CSpaceTreeView();
	DECLARE_DYNCREATE(CSpaceTreeView)

// Attributes
public:
	CSpace* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceTreeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
//	void AddNodeItems(CNode *pNode, CObjectTreeItem *pParent);
	virtual ~CSpaceTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSpaceTreeView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CObjectExplorer m_ObjectExplorer;
};

#ifndef _DEBUG  // debug version in SpaceTreeView.cpp
inline CSpace* CSpaceTreeView::GetDocument()
   { return (CSpace*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACETREEVIEW_H__0C8AA65E_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
