// KohonenSOMDoc.cpp : implementation of the CKohonenSOMDoc class
//

#include "stdafx.h"
#include "KohonenSOM.h"

#include "KohonenSOMDoc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#undef max

// CKohonenSOMDoc

IMPLEMENT_DYNCREATE(CKohonenSOMDoc, CDocument)

BEGIN_MESSAGE_MAP(CKohonenSOMDoc, CDocument)
END_MESSAGE_MAP()

const int nWidth = 6;
const int nHeight = 6;

// CKohonenSOMDoc construction/destruction

CKohonenSOMDoc::CKohonenSOMDoc()
: m_som(nWidth, nHeight)
{
}

CKohonenSOMDoc::~CKohonenSOMDoc()
{
}

BOOL CKohonenSOMDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// initialize data
	AddData(RGB(255,   0,   0), _T("red"));
	AddData(RGB(255, 170, 170), _T("pink"));
	AddData(RGB(170,   0, 255), _T("purple"));
	AddData(RGB(114, 205, 180), _T("turquoise"));
	AddData(RGB(  0,   0, 255), _T("blue"));
	AddData(RGB(235, 153,   1), _T("orange"));
	AddData(RGB(128,  64,   0), _T("brown"));
	AddData(RGB(  0,   0,  64), _T("navy"));
	AddData(RGB(0x22,0x8B,0x22), _T("forest grn"));
	AddData(RGB(0xCD,0x5C,0x5C), _T("indian red"));
	AddData(RGB(0x99,0x32,0xCC), _T("dark orchid")); 
	AddData(RGB(0xFF,0xA0,0x7A), _T("lt salmon ")); 
	AddData(RGB(0x66,0xCD,0xAA), _T("aquamarine")); 
	AddData(RGB(0x00,0x80,0x80), _T("teal")); 
	AddData(RGB(0xFF,0xFA,0xCD), _T("lemon chiffon")); 
	AddData(RGB(0x64,0x95,0xED), _T("cornflower blue")); 
	AddData(RGB(0x2E,0x8B,0x57), _T("sea green")); 
	AddData(RGB(0xDA,0xA5,0x20), _T("goldenrod")); 
	AddData(RGB(0xB0,0x30,0x60), _T("maroon")); 

	AddData(RGB(0xFF,0x00,0xFF), _T("Fuchsia"));
	AddData(RGB(0xB2,0x22,0x22), _T("Firebrick"));
	AddData(RGB(0x19,0x19,0x70), _T("Midnight blue"));

	AddData(RGB(0xFF,0xEF,0xD5), _T("Papaya whip"));
	AddData(RGB(0xFF,0xDA,0xB9), _T("Peach puff"));
	AddData(RGB(0xCD,0x85,0x3F), _T("Peru"));

  AddData(RGB(237, 145, 33), _T("carrot"));

  AddData(RGB(0, 255, 127), _T("springgreen"));

  AddData(RGB(0, 201, 87), _T("emeraldgreen"));

  for (int n = 0; n < 100; n++)
    AddData(RGB(rand() * 255 / RAND_MAX, rand() * 255 / RAND_MAX, rand() * 255 / RAND_MAX), _T(""));

	// Seed the random-number generator with current time so that
	// the numbers will be different every time we run.
	//
	srand( (unsigned)time( NULL ) );

	// init grid offsets
	CUnitNode::m_arrGridOffset.clear();
	CUnitNode::m_arrGridOffset.push_back(CVectorD<2>(-nWidth, 0));
	CUnitNode::m_arrGridOffset.push_back(CVectorD<2>(0, 0));
	CUnitNode::m_arrGridOffset.push_back(CVectorD<2>(nWidth, 0));
	CUnitNode::m_arrGridOffset.push_back(CVectorD<2>(0, -nHeight));
	CUnitNode::m_arrGridOffset.push_back(CVectorD<2>(0, nHeight));

	InitTrain(0.45, 2.0, 0.0005, true);

	return TRUE;
}


void CKohonenSOMDoc::AddData(COLORREF color, CString strName)
{
	CVectorN<> vPos(3);
	vPos[0] = (double) GetRValue(color) / 255.0;
	vPos[1] = (double) GetGValue(color) / 255.0;
	vPos[2] = (double) GetBValue(color) / 255.0;

	m_arrData.push_back(vPos);
	m_arrLabels.push_back(strName);
}

// trains using the dataset

void CKohonenSOMDoc::InitTrain(double alpha, double neighborhood, double K, bool bRandRef)
{
    if (bRandRef)
    {
        // initialize reference vectors to random values
        for (int nAt = 0; nAt < (int) m_som.m_arrReferenceNodes.size(); nAt++) 
        {
	        CVectorN<> vInit(m_arrData[0].GetDim());
	        for (int nV = 0; nV < vInit.GetDim(); nV++)
	        {
		        vInit[nV] = (double) rand() / (double) RAND_MAX;
	        }

	        m_som.m_arrReferenceNodes[nAt]->SetReference(vInit);
        }
    }

    m_lambda = exp(-K);
	m_alpha = alpha;
	m_neighborhood = neighborhood;
	m_nCountSinceInit = 0;
}


void CKohonenSOMDoc::Train(int nTrials)
{
	bool bNN = true; // m_neighborhood > 0.5 || m_alpha > 0.10; 

    for (; nTrials > 0; nTrials--, m_nCountSinceInit++ ) 
	{
        m_alpha *= m_lambda;
        m_neighborhood *= m_lambda;

		int nAtActual = rand() % (int) m_arrData.size(); // ) / RAND_MAX;
		ASSERT(nAtActual < (int) m_arrData.size());
		const CVectorN<>& vInput = m_arrData[nAtActual];

		CVectorD<2> vMapPos;
		if (bNN)
		{
			int nClosest = FindClosestUnit(vInput);
			vMapPos = m_som.m_arrReferenceNodes[nClosest]->GetPosition();
		}
		else
		{
			vMapPos = InterpMapPos(vInput);
		}

		for (int nAt = 0; nAt < (int) m_som.m_arrReferenceNodes.size(); nAt++)
		{
			m_som.m_arrReferenceNodes[nAt]->Update(vInput, vMapPos, m_alpha, m_neighborhood);
		}
    }
}

int CKohonenSOMDoc::FindClosestUnit(const CVectorN<>& vInput)
{
	double minDistSq = numeric_limits<double>::max();
	int nClosest = -1;
	for (int nAt = 0; nAt < (int) m_som.m_arrReferenceNodes.size(); nAt++)
	{
		CVectorN<> vOfs = m_som.m_arrReferenceNodes[nAt]->GetReference();
		vOfs -= vInput;
		double distSq = vOfs * vOfs;
		if (distSq < minDistSq)
		{
			nClosest = nAt;
			minDistSq = distSq;
		}
	}
	ASSERT(nClosest != -1);

    return nClosest;
}


CVectorD<2> 
	CKohonenSOMDoc::InterpMapPos(const CVectorN<>& vInput)
{
	int nClosest = FindClosestUnit(vInput);
	return m_som.m_arrReferenceNodes[nClosest]->InterpMapPos(vInput);
}



// CKohonenSOMDoc serialization

void CKohonenSOMDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CKohonenSOMDoc diagnostics

#ifdef _DEBUG
void CKohonenSOMDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CKohonenSOMDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

