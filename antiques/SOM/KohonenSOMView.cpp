// KohonenSOMView.cpp : implementation of the CKohonenSOMView class
//

#include "stdafx.h"
#include "KohonenSOM.h"

#include "KohonenSOMDoc.h"
#include "KohonenSOMView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

 
// CKohonenSOMView

IMPLEMENT_DYNCREATE(CKohonenSOMView, CView)

BEGIN_MESSAGE_MAP(CKohonenSOMView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_COMMAND(ID_RESET, &CKohonenSOMView::OnReset)
END_MESSAGE_MAP()

// CKohonenSOMView construction/destruction

CKohonenSOMView::CKohonenSOMView()
{
	// TODO: add construction code here

}

CKohonenSOMView::~CKohonenSOMView()
{
}

BOOL CKohonenSOMView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CKohonenSOMView drawing

void CKohonenSOMView::OnDraw(CDC* pDC)
{
	CKohonenSOMDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	pDC->SetBkMode(TRANSPARENT);

	for (int nAt = 0; nAt < pDoc->m_som.m_arrReferenceNodes.size(); nAt++)
	{
    
			COLORREF color = RGB(pDoc->m_som.m_arrReferenceNodes[nAt]->GetReference()[0] * 255.0, 
				pDoc->m_som.m_arrReferenceNodes[nAt]->GetReference()[1] * 255.0, 
				pDoc->m_som.m_arrReferenceNodes[nAt]->GetReference()[2] * 255.0);
			CBrush brush(color);
			pDC->SelectObject(&brush);

		CVectorD<2> vPos = pDoc->m_som.m_arrReferenceNodes[nAt]->GetPosition();
		pDC->Rectangle(
			90.0 + vPos[0] * 150.0 - 30.0, 
			150.0 + vPos[1] * 100.0 - 30.0, 
			90.0 + vPos[0] * 150.0 + 30.0, 
			150.0 + vPos[1] * 100.0 + 30.0);
	}

	//for (int nAtOfs = 0; nAtOfs < CUnitNode::m_arrGridOffset.size(); nAtOfs++)
	//{
		for (int nAt = 0; nAt < pDoc->m_arrData.size(); nAt++)
		{
			// int nClosest = pDoc->FindClosestUnit(pDoc->m_arrData[nAt]);
			CVectorD<2> vPos = // m_arrUnitPos[nClosest];
				pDoc->InterpMapPos(pDoc->m_arrData[nAt]);

			CVectorD<2> vOfs;// = CUnitNode::m_arrGridOffset[nAtOfs];
			// vOfs[0] = vOfs[0] * 150.0;
			// vOfs[1] = vOfs[1] * 100.0;
			vPos += vOfs;

			COLORREF color = RGB(pDoc->m_arrData[nAt][0] * 255.0, 
				pDoc->m_arrData[nAt][1] * 255.0, 
				pDoc->m_arrData[nAt][2] * 255.0);
			CBrush brush(color);
			pDC->SelectObject(&brush);
			pDC->Ellipse(
				90.0 + vPos[0] * 150.0 - 10.0, 
				150.0 + vPos[1] * 100.0 - 10.0, 
				90.0 + vPos[0] * 150.0 + 10.0, 
				150.0 + vPos[1] * 100.0 + 10.0);

			pDC->TextOutW(
				90.0 + vPos[0] * 150.0 /* + nAt * 4 */, 
				150.0 + vPos[1] * 100.0 /* + nAt * 4 */, pDoc->m_arrLabels[nAt]);
		}
	//}

	pDC->SelectStockObject(HOLLOW_BRUSH);

	CString strNeigh;
	strNeigh.Format(_T("neigh = %lf"), pDoc->m_neighborhood);
	pDC->TextOutW(10.0, 10.0, strNeigh);

	CString strAlpha;
	strAlpha.Format(_T("alpha = %lf"), pDoc->m_alpha);
	pDC->TextOutW(10.0, 50.0, strAlpha);
}


// CKohonenSOMView printing

BOOL CKohonenSOMView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CKohonenSOMView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CKohonenSOMView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CKohonenSOMView diagnostics

#ifdef _DEBUG
void CKohonenSOMView::AssertValid() const
{
	CView::AssertValid();
}

void CKohonenSOMView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CKohonenSOMDoc* CKohonenSOMView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CKohonenSOMDoc)));
	return (CKohonenSOMDoc*)m_pDocument;
}
#endif //_DEBUG


// CKohonenSOMView message handlers

void CKohonenSOMView::OnTimer(UINT_PTR nIDEvent)
{
	CKohonenSOMDoc* pDoc = GetDocument();
	pDoc->Train(10);

	Invalidate(TRUE);

	CView::OnTimer(nIDEvent);
}

int CKohonenSOMView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTimer(7, 500, NULL);

	return 0;
}

void CKohonenSOMView::OnReset()
{
	CKohonenSOMDoc* pDoc = GetDocument();
	pDoc->InitTrain(0.45, 2.0, 0.0005, false);
}
