// AxWheelCtrl.cpp : Implementation of CAxWheelCtrl
#include "stdafx.h"
#include "AxWheelCtrl.h"

#include <RuntimeTracker.h>

// CAxWheelCtrl

//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAxWheelCtrl::get_BaseURL(BSTR* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	(*pVal) = m_strBaseURL.Copy();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAxWheelCtrl::put_BaseURL(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_strBaseURL = newVal;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAxWheelCtrl::WriteXML(BSTR bstrXmlFilename)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	for (int nAt = 0; nAt < m_pSpace->GetNodeCount(); nAt++)
	{
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
BSTR CAxWheelCtrl::GetRootName()
{
	m_strTitle = m_strBaseURL;

	POSITION pos = m_mapNPs.GetStartPosition();
	while (pos != NULL)
	{
		NodeService::NodeProxy *pNP = m_mapNPs.GetNextValue(pos);
		if (pNP->NodesAreCreated())
		{
			m_strTitle.Append(pNP->Name);
			m_strTitle.Append(L"\n");
		}
	}

	return m_strTitle;
}

class CTempDoc : public CDocument
{
public:
	CTempDoc() { }
};

//////////////////////////////////////////////////////////////////////////////
LRESULT CAxWheelCtrl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, 
							   BOOL& bHandled)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	{
		CFile file;
		BOOL bRet = file.Open(_T("C:\\Projects\\theWheel\\data\\SonicSpace_destab_scaled3.spx"), // "C:\\test.bin"), 
			CFile::modeRead);

		CArchive ar(&file, CArchive::load);

		CTempDoc tempDoc;
		tempDoc.Serialize(ar);

		m_pSpace = new CSpace();
		m_pSpace->Serialize(ar);

		ar.Close();
	}

	m_pSV = new CSpaceView();
	BOOL bRet = m_pSV->Create(NULL, NULL, 
		WS_CHILD | WS_VISIBLE, 
		CRect(0, 0, 300, 100), 
		CWnd::FromHandle(m_hWnd), 0);

	m_pSV->SetSpace(m_pSpace);

	// create a run time tracker
	CRuntimeTracker *pTracker = new CRuntimeTracker(m_pSV);

	// install the tracker on the space view
	m_pSV->SetTracker(pTracker);

	//// Initialize the thread pool to 4 threads per CPU
	//m_threadPool.Initialize(0, -4);

	//// set up timer
	//UINT_PTR hTimer = SetTimer(7, 500);
	//ATLASSERT(hTimer == 7);

	bHandled = TRUE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
LRESULT CAxWheelCtrl::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, 
							   BOOL& bHandled)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	CRect rectClient;
	GetClientRect(&rectClient);

	m_pSV->MoveWindow(rectClient);

	bHandled = TRUE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
LRESULT CAxWheelCtrl::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, 
							  BOOL& bHandled)
{
	POSITION pos = m_mapNPs.GetStartPosition();
	while (pos != NULL)
	{
		NodeService::NodeProxy *pNP = m_mapNPs.GetNextValue(pos);
		if (!pNP->IsRequestQueued())
		{
			// set up fetch for root node
			HRESULT hRet = pNP->QueueRequest(&m_threadPool);
			ATLASSERT( hRet == E_PENDING );
			return 0;
		}
		else if (pNP->IsRequestCompleted()
			&& !pNP->NodesAreCreated())
		{
			pNP->CreateNodesFromNS(m_mapNPs);
			Invalidate(FALSE);
			return 0;
		}
	}

	bHandled = TRUE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
HRESULT CAxWheelCtrl::OnDraw(ATL_DRAWINFO& di)
{
	return S_OK;
  
  RECT& rc = *(RECT*)di.prcBounds;
	// Set Clip region to the rectangle specified by di.prcBounds
	HRGN hRgnOld = NULL;
	if (GetClipRgn(di.hdcDraw, hRgnOld) != 1)
		hRgnOld = NULL;
	bool bSelectOldRgn = false;

	HRGN hRgnNew = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

	if (hRgnNew != NULL)
	{
		bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);
	}

	Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
	SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
	LPCTSTR pszText = GetRootName();
	TextOut(di.hdcDraw,
		(rc.left + rc.right) / 2,
		(rc.top + rc.bottom) / 2,
		pszText,
		lstrlen(pszText));

	if (bSelectOldRgn)
		SelectClipRgn(di.hdcDraw, hRgnOld);

	return S_OK;
}


