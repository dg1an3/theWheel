// SpaceLayoutPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "theWheel.h"
#include "SpaceLayoutPropPage.h"

#include <Space.h>
#include <SpaceLayoutManager.h>


//////////////////////////////////////////////////////////////////////
// slider ranges for the layout parameters
//
// the sliders are integral; every parameter they drive is comfortably
//		integral over its range except the spring constant, which runs
//		0..2 and is scaled by SPRING_SCALE
//////////////////////////////////////////////////////////////////////

const int KPOS_MIN = 0;
const int KPOS_MAX = 2000;

const int KREP_MIN = 0;
const int KREP_MAX = 12000;

// CSpaceLayoutManager indexes MAX_STATE_DIM-square matrices by node
//		(see LoadSizesLinks), so the visible node count cannot exceed it
const int NODES_MIN = 5;
const int NODES_MAX = MAX_STATE_DIM;

const int SPRING_SCALE = 100;
const int SPRING_MIN = 1;
const int SPRING_MAX = 200;

// timer that refreshes the energy readout
const UINT ENERGY_TIMER_ID = 1;
const UINT ENERGY_TIMER_ELAPSED = 250;


// CSpaceLayoutPropPage dialog

IMPLEMENT_DYNAMIC(CSpaceLayoutPropPage, CPropertyPage)

CSpaceLayoutPropPage::CSpaceLayoutPropPage()
	: CPropertyPage(CSpaceLayoutPropPage::IDD)
	, m_pSpace(NULL)
	, m_kPosOrig(0.0)
	, m_kRepOrig(0.0)
	, m_springOrig(0.0)
	, m_toleranceOrig(0.0)
	, m_superNodesOrig(0)
	, m_bInitialized(FALSE)
	, m_bUpdating(FALSE)
{

}

CSpaceLayoutPropPage::~CSpaceLayoutPropPage()
{
}

void CSpaceLayoutPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SLIDER_KPOS, m_sliderKPos);
	DDX_Control(pDX, IDC_SLIDER_KREP, m_sliderKRep);
	DDX_Control(pDX, IDC_SLIDER_SUPERNODES, m_sliderSuperNodes);
	DDX_Control(pDX, IDC_SLIDER_SPRING, m_sliderSpring);
}


BEGIN_MESSAGE_MAP(CSpaceLayoutPropPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT_KPOS, &CSpaceLayoutPropPage::OnChangeKPos)
	ON_EN_CHANGE(IDC_EDIT_KREP, &CSpaceLayoutPropPage::OnChangeKRep)
	ON_EN_CHANGE(IDC_EDIT_SUPERNODES, &CSpaceLayoutPropPage::OnChangeSuperNodes)
	ON_EN_CHANGE(IDC_EDIT_SPRING, &CSpaceLayoutPropPage::OnChangeSpring)
	ON_EN_CHANGE(IDC_EDIT_TOLERANCE, &CSpaceLayoutPropPage::OnChangeTolerance)
	ON_BN_CLICKED(IDC_BUTTON_REVERT, &CSpaceLayoutPropPage::OnBnClickedRevert)
END_MESSAGE_MAP()


// CSpaceLayoutPropPage message handlers

//////////////////////////////////////////////////////////////////////
// CSpaceLayoutPropPage::OnInitDialog
//
// sets up the slider ranges and loads the space's current parameters
//////////////////////////////////////////////////////////////////////
BOOL CSpaceLayoutPropPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_sliderKPos.SetRange(KPOS_MIN, KPOS_MAX);
	m_sliderKRep.SetRange(KREP_MIN, KREP_MAX);
	m_sliderSuperNodes.SetRange(NODES_MIN, NODES_MAX);
	m_sliderSpring.SetRange(SPRING_MIN, SPRING_MAX);

	// without a space there is nothing to edit
	if (NULL == m_pSpace)
	{
		GetDlgItem(IDC_EDIT_KPOS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_KREP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SUPERNODES)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SPRING)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_TOLERANCE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_REVERT)->EnableWindow(FALSE);
		m_sliderKPos.EnableWindow(FALSE);
		m_sliderKRep.EnableWindow(FALSE);
		m_sliderSuperNodes.EnableWindow(FALSE);
		m_sliderSpring.EnableWindow(FALSE);
		return TRUE;
	}

	// remember where we started, so Revert has something to go back to
	CSpaceLayoutManager *pLayout = m_pSpace->GetLayoutManager();
	m_kPosOrig = pLayout->GetKPos();
	m_kRepOrig = pLayout->GetKRep();
	m_toleranceOrig = pLayout->GetTolerance();
	m_superNodesOrig = pLayout->GetStateDim() / 2;
	m_springOrig = m_pSpace->GetSpringConst();

	m_bInitialized = TRUE;
	LoadFromSpace();

	// the readout tracks the layout as it settles
	SetTimer(ENERGY_TIMER_ID, ENERGY_TIMER_ELAPSED, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control

}	// CSpaceLayoutPropPage::OnInitDialog


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutPropPage::OnDestroy
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutPropPage::OnDestroy()
{
	KillTimer(ENERGY_TIMER_ID);

	CPropertyPage::OnDestroy();

}	// CSpaceLayoutPropPage::OnDestroy


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutPropPage::LoadFromSpace
//
// pushes the space's current parameters out to every control
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutPropPage::LoadFromSpace()
{
	if (!m_bInitialized || NULL == m_pSpace)
	{
		return;
	}

	CSpaceLayoutManager *pLayout = m_pSpace->GetLayoutManager();

	const REAL kPos = pLayout->GetKPos();
	const REAL kRep = pLayout->GetKRep();
	const REAL spring = m_pSpace->GetSpringConst();
	const int nNodes = pLayout->GetStateDim() / 2;

	// writing the edit boxes fires EN_CHANGE, which would otherwise come
	//		straight back in here and reformat what the user is typing
	m_bUpdating = TRUE;

	CString str;
	str.Format(_T("%g"), (double) kPos);
	SetDlgItemText(IDC_EDIT_KPOS, str);
	str.Format(_T("%g"), (double) kRep);
	SetDlgItemText(IDC_EDIT_KREP, str);
	str.Format(_T("%d"), nNodes);
	SetDlgItemText(IDC_EDIT_SUPERNODES, str);
	str.Format(_T("%g"), (double) spring);
	SetDlgItemText(IDC_EDIT_SPRING, str);
	str.Format(_T("%g"), (double) pLayout->GetTolerance());
	SetDlgItemText(IDC_EDIT_TOLERANCE, str);

	m_bUpdating = FALSE;

	m_sliderKPos.SetPos(Round<int>(kPos));
	m_sliderKRep.SetPos(Round<int>(kRep));
	m_sliderSuperNodes.SetPos(nNodes);
	m_sliderSpring.SetPos(Round<int>(spring * SPRING_SCALE));

	UpdateEnergy();

}	// CSpaceLayoutPropPage::LoadFromSpace


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutPropPage::OnHScroll
//
// a slider moved: apply it to the space and echo it to its edit box
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutPropPage::OnHScroll(UINT nSBCode, UINT nPos,
									 CScrollBar *pScrollBar)
{
	if (!m_bInitialized || NULL == m_pSpace)
	{
		CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	CSpaceLayoutManager *pLayout = m_pSpace->GetLayoutManager();

	// EN_CHANGE would fire as we echo the new value into the edit box
	m_bUpdating = TRUE;

	CString str;

	if (pScrollBar == (CScrollBar *) &m_sliderKPos)
	{
		const int nValue = m_sliderKPos.GetPos();
		pLayout->SetKPos((REAL) nValue);
		str.Format(_T("%d"), nValue);
		SetDlgItemText(IDC_EDIT_KPOS, str);
	}
	else if (pScrollBar == (CScrollBar *) &m_sliderKRep)
	{
		const int nValue = m_sliderKRep.GetPos();
		pLayout->SetKRep((REAL) nValue);
		str.Format(_T("%d"), nValue);
		SetDlgItemText(IDC_EDIT_KREP, str);
	}
	else if (pScrollBar == (CScrollBar *) &m_sliderSuperNodes)
	{
		const int nValue = m_sliderSuperNodes.GetPos();
		m_pSpace->SetMaxSuperNodeCount(nValue);
		str.Format(_T("%d"), nValue);
		SetDlgItemText(IDC_EDIT_SUPERNODES, str);
	}
	else if (pScrollBar == (CScrollBar *) &m_sliderSpring)
	{
		const REAL spring = (REAL) m_sliderSpring.GetPos() / (REAL) SPRING_SCALE;
		m_pSpace->SetSpringConst(spring);
		str.Format(_T("%g"), (double) spring);
		SetDlgItemText(IDC_EDIT_SPRING, str);
	}

	m_bUpdating = FALSE;

	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);

}	// CSpaceLayoutPropPage::OnHScroll


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutPropPage::ApplyEdit
//
// applies one edit box to the space and moves its slider to match
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutPropPage::ApplyEdit(UINT nEditID)
{
	// ignore the EN_CHANGE we caused ourselves
	if (!m_bInitialized || m_bUpdating || NULL == m_pSpace)
	{
		return;
	}

	CString strValue;
	GetDlgItemText(nEditID, strValue);
	strValue.Trim();

	// an empty box is mid-edit, not a value
	if (strValue.IsEmpty())
	{
		return;
	}

	const double value = _tstof(strValue);

	CSpaceLayoutManager *pLayout = m_pSpace->GetLayoutManager();

	switch (nEditID)
	{
	case IDC_EDIT_KPOS:
		pLayout->SetKPos((REAL) value);
		m_sliderKPos.SetPos(Round<int>(value));
		break;

	case IDC_EDIT_KREP:
		pLayout->SetKRep((REAL) value);
		m_sliderKRep.SetPos(Round<int>(value));
		break;

	case IDC_EDIT_SUPERNODES:
		{
			// out-of-range counts would overrun the layout manager's
			//		MAX_STATE_DIM-square matrices
			int nNodes = (int) value;
			nNodes = __max(nNodes, NODES_MIN);
			nNodes = __min(nNodes, NODES_MAX);
			m_pSpace->SetMaxSuperNodeCount(nNodes);
			m_sliderSuperNodes.SetPos(nNodes);
		}
		break;

	case IDC_EDIT_SPRING:
		m_pSpace->SetSpringConst((REAL) value);
		m_sliderSpring.SetPos(Round<int>(value * SPRING_SCALE));
		break;

	case IDC_EDIT_TOLERANCE:
		pLayout->SetTolerance((REAL) value);
		break;
	}

}	// CSpaceLayoutPropPage::ApplyEdit


void CSpaceLayoutPropPage::OnChangeKPos()
{
	ApplyEdit(IDC_EDIT_KPOS);
}

void CSpaceLayoutPropPage::OnChangeKRep()
{
	ApplyEdit(IDC_EDIT_KREP);
}

void CSpaceLayoutPropPage::OnChangeSuperNodes()
{
	ApplyEdit(IDC_EDIT_SUPERNODES);
}

void CSpaceLayoutPropPage::OnChangeSpring()
{
	ApplyEdit(IDC_EDIT_SPRING);
}

void CSpaceLayoutPropPage::OnChangeTolerance()
{
	ApplyEdit(IDC_EDIT_TOLERANCE);
}


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutPropPage::OnBnClickedRevert
//
// restores the parameters as they stood when the page was opened
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutPropPage::OnBnClickedRevert()
{
	if (!m_bInitialized || NULL == m_pSpace)
	{
		return;
	}

	CSpaceLayoutManager *pLayout = m_pSpace->GetLayoutManager();
	pLayout->SetKPos((REAL) m_kPosOrig);
	pLayout->SetKRep((REAL) m_kRepOrig);
	pLayout->SetTolerance((REAL) m_toleranceOrig);
	m_pSpace->SetMaxSuperNodeCount(m_superNodesOrig);
	m_pSpace->SetSpringConst((REAL) m_springOrig);

	LoadFromSpace();

}	// CSpaceLayoutPropPage::OnBnClickedRevert


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutPropPage::UpdateEnergy
//
// refreshes the energy readout
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutPropPage::UpdateEnergy()
{
	if (NULL == m_pSpace)
	{
		return;
	}

	CString str;
	str.Format(_T("%.1f    (%d of %d nodes)"),
		(double) m_pSpace->GetLayoutManager()->GetEnergy(),
		m_pSpace->GetSuperNodeCount(),
		m_pSpace->GetNodeCount());
	SetDlgItemText(IDC_STATIC_ENERGY, str);

}	// CSpaceLayoutPropPage::UpdateEnergy


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutPropPage::OnTimer
//
// the view lays out on its own timer, so poll the resulting energy
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutPropPage::OnTimer(UINT_PTR nIDEvent)
{
	if (ENERGY_TIMER_ID == nIDEvent)
	{
		UpdateEnergy();
	}

	CPropertyPage::OnTimer(nIDEvent);

}	// CSpaceLayoutPropPage::OnTimer
