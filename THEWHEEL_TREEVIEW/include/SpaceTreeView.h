//////////////////////////////////////////////////////////////////////
// SpaceTreeView.h: interface for the CSpaceTreeView class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(SPACETREEVIEW_H)
#define SPACETREEVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// the displayed model CSpace object
#include <Space.h>

// special tree control that handles menus
#include "TreeMenuCtrl.h"

// property dialogs for the space
#include "SpacePropDlg.h"
#include "SpaceOptDlg.h"
#include "SpaceClassDlg.h"

// property dialogs for the nodes
#include "NodePropDlg.h"
#include "ImagePropDlg.h"
#include "LinkPropDlg.h"

//////////////////////////////////////////////////////////////////////
// class CSpaceTreeView
//
// manages hierarchical navigation and editing of the space
//////////////////////////////////////////////////////////////////////
class CSpaceTreeView : public CView
{
protected: // create from serialization only
	CSpaceTreeView();
	virtual ~CSpaceTreeView();

	// CSpaceTreeView must be dynamically created
	DECLARE_DYNCREATE(CSpaceTreeView)

// Attributes
public:
	// returns the CSpace with is being displayed
	CSpace* GetDocument();

// Operations
public:

	// selects a node in the tree view
	void SelectNode(CNode *pNode);

	// enables property pages
	void EnablePropertyPage(BOOL bEnable = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceTreeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:

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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSpaceTabChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNodeTabChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewNode();
	afx_msg void OnDeletenode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// the contained tree control
	CTreeMenuCtrl m_wndTree;

	// flag to indicate that property pages are enabled
	BOOL m_bPropertyPagesEnabled;

	// the space properties dialogs
	CTabCtrl m_wndSpaceProperties;

	CSpacePropDlg m_dlgSpaceProp;
	CSpaceOptDlg m_dlgSpaceOpt;
	CSpaceClassDlg m_dlgSpaceClass;

	// the node properties dialogs
	CTabCtrl m_wndNodeProperties;

	CNodePropDlg m_dlgNodeProp;
	CImagePropDlg m_dlgImageProp;
	CLinkPropDlg m_dlgLinkProp;

    // the map of handles to CNode objects
    CMap<CNode *, CNode *, HTREEITEM, HTREEITEM> m_mapItemHandles;

	// the image list for tree icons
	CImageList m_imageList;

	// holds the index of the icons
	int m_nNodeImageIndex;
	int m_nSpaceImageIndex;

	// handle to the space item
	HTREEITEM m_hSpaceItem;

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

#endif // !defined(SPACETREEVIEW_H)
