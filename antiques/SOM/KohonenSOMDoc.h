// KohonenSOMDoc.h : interface of the CKohonenSOMDoc class
//


#pragma once
#include "afxtempl.h"

#include "UnitNode.h"
#include "SelfOrgMap.h"

class CKohonenSOMDoc : public CDocument
{
protected: // create from serialization only
	CKohonenSOMDoc();
	DECLARE_DYNCREATE(CKohonenSOMDoc)

// Attributes
public:

	void AddData(COLORREF color, CString strName);

// Operations
public:
	// trains using the dataset
	void InitTrain(double alpha, double neighborhood, double K, bool bRandRef);

	// trains using the dataset
	void Train(int nTrials);

    int FindClosestUnit(const CVectorN<>& vInput);
	CVectorD<2> InterpMapPos(const CVectorN<>& vInput);

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CKohonenSOMDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	// holds the data vectors
    // vector< CUnitNode* > m_arrData;
	vector< CVectorN<> > m_arrData;
	vector< CString > m_arrLabels;

	// the map
    SelfOrgMap m_som;

	double m_lambda;
	double m_alpha;
	double m_neighborhood;
	int m_nCountSinceInit;
};
