// SpaceTreeView.h : interface of the CSpaceTreeView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPACETREEVIEW_H__0C8AA65E_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_SPACETREEVIEW_H__0C8AA65E_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Node.h>

class CSpace;

class CSpaceTreeView : public CTreeView
{
protected: // create from serialization only
	CSpaceTreeView();
	DECLARE_DYNCREATE(CSpaceTreeView)

// Attributes
public:
	// returns the CSpace with is being displayed
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

	virtual ~CSpaceTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// adds a node item to the tree control
	void AddNodeItem(CNode *pNode, HTREEITEM hParentItem = NULL);

	// removes the node's item from the tree control, 
	//		including all children
	void RemoveNodeItem(CNode *pNode);

// Generated message map functions
protected:
	//{{AFX_MSG(CSpaceTreeView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    // the map of handles to CNode objects
    CMap<CNode *, CNode *, HTREEITEM, HTREEITEM> m_mapItemHandles;

	// the image list for tree icons
	CImageList m_imageList;

	// holds the index of the node image
	int m_nNodeImageIndex;

	// dragging state flag
	BOOL m_bDragging;

	// drag image list
    CImageList * m_pDragImageList;

	// pointers for the dragging node and the dropped node
	CNode *m_pNodeDrag;
	CNode *m_pNodeDrop;

    // handles for the drop cursor and the no-drop cursor
    HCURSOR m_hDropCursor;
    HCURSOR m_hNoDropCursor;

    // handles auto-scrolling when the user drags off the end of the control
    UINT m_nScrollTimerID;
};

#ifndef _DEBUG  // debug version in SpaceTreeView.cpp
inline CSpace* CSpaceTreeView::GetDocument()
   { return (CSpace*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACETREEVIEW_H__0C8AA65E_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
