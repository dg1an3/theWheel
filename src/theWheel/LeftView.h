// LeftView.h : interface of the CLeftView class
//


#pragma once
#include "SpaceTreeView.h"

class CtheWheelDoc;

class CLeftView : public CView
{
protected: // create from serialization only
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

// Attributes
public:
	CtheWheelDoc* GetDocument();

	// the child space tree view
	CSpaceTreeView m_spaceTreeView;

// Operations
public:

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual void OnDraw(CDC* /*pDC*/);
};

#ifndef _DEBUG  // debug version in LeftView.cpp
inline CtheWheelDoc* CLeftView::GetDocument()
   { return reinterpret_cast<CtheWheelDoc*>(m_pDocument); }
#endif

