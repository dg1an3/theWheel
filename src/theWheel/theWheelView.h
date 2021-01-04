// theWheelView.h : interface of the CtheWheelView class
//


#pragma once
#include <SpaceView.h>

class CtheWheelView : public CView
{
protected: // create from serialization only
	CtheWheelView();
	DECLARE_DYNCREATE(CtheWheelView)

// Attributes
public:
	CtheWheelDoc* GetDocument() const;

	// the child space view
	CSpaceView m_spaceView;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CtheWheelView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnViewDesigntime();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

};

#ifndef _DEBUG  // debug version in theWheelView.cpp
inline CtheWheelDoc* CtheWheelView::GetDocument() const
   { return reinterpret_cast<CtheWheelDoc*>(m_pDocument); }
#endif

