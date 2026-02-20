// theWheelDoc.h : interface of the CtheWheelDoc class
//


#pragma once
#include "c:\projects\thewheel\src\thewheel_model\include\space.h"


class CtheWheelDoc : public CDocument
{
protected: // create from serialization only
	CtheWheelDoc();
	DECLARE_DYNCREATE(CtheWheelDoc)

// Attributes
public:

// Operations
public:
	// called when the space is changed
	void OnSpaceChanged(CObservableEvent * pEvent, void * pParam);

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CtheWheelDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSpaceScalelinkweights();
	afx_msg void OnSpacePerturblinkweights();
public:
	afx_msg void OnSpaceProperties();
public:
	// the space property sheet
	CPropertySheet *m_pSpacePropSheet;
public:
	// the space for the document
	CSpace m_space;
public:
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
};


