// theWheelDoc.h : interface of the CtheWheelDoc class
//


#pragma once
#include <Space.h>


class CtheWheelDoc : public CDocument
{
protected: // create from serialization only
	CtheWheelDoc();
	DECLARE_DYNCREATE(CtheWheelDoc)

// Attributes
public:
	// the space for the document
	CSpace m_space;

	// the space property sheet
	CPropertySheet *m_pSpacePropSheet;

	// called to set the path name
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);

// Operations
public:
	// called when the space is changed
	void OnSpaceChanged(/*CObservableEvent * pEvent, void * pParam*/);

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
	afx_msg void OnSpaceScalelinkweights();
	afx_msg void OnSpacePerturblinkweights();
	afx_msg void OnSpaceProperties();
public:
	afx_msg void OnScaleLinkWeights();
};


