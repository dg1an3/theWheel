// KohonenSOMView.h : interface of the CKohonenSOMView class
//


#pragma once


class CKohonenSOMView : public CView
{
protected: // create from serialization only
	CKohonenSOMView();
	DECLARE_DYNCREATE(CKohonenSOMView)

// Attributes
public:
	CKohonenSOMDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CKohonenSOMView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	afx_msg void OnReset();
};

#ifndef _DEBUG  // debug version in KohonenSOMView.cpp
inline CKohonenSOMDoc* CKohonenSOMView::GetDocument() const
   { return reinterpret_cast<CKohonenSOMDoc*>(m_pDocument); }
#endif

