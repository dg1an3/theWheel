// NodeRenderer.cpp: implementation of the CNodeRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NodeRenderer.h"

#include <gl/gl.h>
#include <gl/glu.h>

#include <glMatrixVector.h>

#include <OpenGLView.h>

const int NUM_STEPS = 4;

class CMolding 
{
public:
	CMolding();
	CMolding(double theta, CVector<3> vOffset, double radius);

	CVector<3> m_arrVertices[NUM_STEPS + 1];
	CVector<3> m_arrNormals[NUM_STEPS + 1];

	static CMolding g_masterMolding;
};

CMolding::CMolding()
{
	int nAt = 0;
	const double step = (PI / 2) / (NUM_STEPS - 1);
	for (double angle = 0; angle <= PI / 2; angle += step)
	{
		m_arrVertices[nAt][0] = cos(angle);
		m_arrVertices[nAt][2] = sin(angle);

		m_arrNormals[nAt][0] = cos(angle);
		m_arrNormals[nAt][2] = sin(angle);

		nAt++;
	}

	m_arrVertices[nAt][0] = -2.5;
	m_arrVertices[nAt][2] = 1.0;

	m_arrNormals[nAt][2] = 1.0;
}

CMolding::CMolding(double theta, CVector<3> vOffset, double radius)
{
	CMatrix<4> mRotate = 
		CMatrix<4>(CreateRotate(theta, CVector<3>(0.0, 0.0, 1.0)));
	for (int nAt = 0; nAt < NUM_STEPS+1; nAt++)
	{
		m_arrNormals[nAt] = 
			FromHomogeneous<3, double>(mRotate * ToHomogeneous(
				g_masterMolding.m_arrNormals[nAt]));

		m_arrVertices[nAt] = 
			radius * FromHomogeneous<3, double>(mRotate * ToHomogeneous(
				g_masterMolding.m_arrVertices[nAt]));

		m_arrVertices[nAt] += vOffset;
	}
}

CMolding CMolding::g_masterMolding;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeRenderer::CNodeRenderer(COpenGLView *pView)
	: COpenGLRenderer(pView),
		m_borderRadius(2.0),
		m_rectangularity(1.0 / sqrt(2.0)),
		m_width(25.0),
		m_aspectRatio(0.625),
		m_activation(0.1)
		// m_color(RGB(192, 192, 192))
{
	// color.Set(RGB(255, 255, 255));

	// activation.AddObserver(this, (ChangeFunction) OnActivationChanged);
	// rectangularity.AddObserver(this, (ChangeFunction) OnChange);
	// width.AddObserver(this, (ChangeFunction) OnChange);

	// load an image
	BOOL bResult = m_Image.Load("C:\\dragon_glow.bmp");
}

CNodeRenderer::~CNodeRenderer()
{

}

CVector<3> CNodeRenderer::EvalEllipseMolding(double theta, double a, double b)
{
	// compute theta_prime for the ellipse
	double sign = cos(theta) > 0 ? 1.0 : -1.0;
	double theta_prime = atan2(sign * a * tan(theta), sign * b);

	return CVector<3>(a * cos(theta_prime), b * sin(theta_prime), 0.0);

}

CMolding *CNodeRenderer::RenderMoldingSection(double angle_start, double angle_stop,
										 double a, double b, CMolding *pOldMolding)
{
	if (pOldMolding == NULL)
		pOldMolding = 
			new CMolding(angle_start, EvalEllipseMolding(angle_start, a, b), m_borderRadius);

	double step = (angle_stop - angle_start) / 8.0;
	for (double theta = angle_start; theta <= angle_stop; theta += step)
	{
		// create the molding from the template
		CVector<3> vShapePt = EvalEllipseMolding(theta, a, b);

		// create a new molding object
		CMolding *pNewMolding = new CMolding(theta, vShapePt, m_borderRadius);

		for (int nAt = 0; nAt < NUM_STEPS; nAt++)
		{
			glBegin(GL_QUADS);

				glNormal(pOldMolding->m_arrNormals[nAt]);
				glVertex(pOldMolding->m_arrVertices[nAt]);

				glNormal(pOldMolding->m_arrNormals[nAt+1]);
				glVertex(pOldMolding->m_arrVertices[nAt+1]);

				glNormal(pNewMolding->m_arrNormals[nAt+1]);
				glVertex(pNewMolding->m_arrVertices[nAt+1]);

				glNormal(pNewMolding->m_arrNormals[nAt]);
				glVertex(pNewMolding->m_arrVertices[nAt]);

			glEnd();
		}

		delete pOldMolding;
		pOldMolding = pNewMolding;
	}

	return pOldMolding;
}

void CNodeRenderer::OnRenderScene()
{
	// compute the base ellipse a and b
	double a = __max(m_width, 10.0);
	double b = a * m_aspectRatio;
	
	glTranslate(m_position);

	// draw an image
/*	void *pBits = m_Image.GetDIBits();
	CSize bitmapSize = m_Image.GetBitmapSize();
	float zoomFactor = (float) (2.0f * width.Get() * aspectRatio.Get() * rectangularity.Get()) 
		/ (float) bitmapSize.cy;
	glPixelZoom(zoomFactor, zoomFactor);

	glRasterPos3d(0.0 - zoomFactor * (float) bitmapSize.cx / 2.0f, 
		0.0 - zoomFactor * (float) bitmapSize.cy / 2.0f, 
		borderRadius.Get());

	glDrawPixels(bitmapSize.cx, bitmapSize.cy, GL_RGB, GL_UNSIGNED_BYTE, pBits);
*/
	// set the color for the node
	glColor(GetColor());

	// draw an elliptangle
	glBegin(GL_QUADS);
		glNormal3d(  0.0, 0.0,   1.0);
		glVertex3d(-a * m_rectangularity, -b * m_rectangularity, m_borderRadius);
		glVertex3d( a * m_rectangularity, -b * m_rectangularity, m_borderRadius);
		glVertex3d( a * m_rectangularity,  b * m_rectangularity, m_borderRadius);
		glVertex3d(-a * m_rectangularity,  b * m_rectangularity, m_borderRadius);
	glEnd();

	// compute the angle dividing the sections of the elliptangle
	double sectionAngle = atan2(b, a);

	// computing the a and b adjustments for the sections of the elliptangle
	double r_factor = m_rectangularity
		/ sqrt(1.0 - m_rectangularity * m_rectangularity);

	// now render the four sections of the molding
	CMolding* pOld;
	pOld = RenderMoldingSection(2*PI,                2*PI + sectionAngle, 
		a, b * r_factor);
	pOld = RenderMoldingSection(       sectionAngle,   PI - sectionAngle, 
		a * r_factor, b, pOld);
	pOld = RenderMoldingSection(  PI - sectionAngle,   PI + sectionAngle, 
		a, b * r_factor, pOld);
	pOld = RenderMoldingSection(  PI + sectionAngle, 2*PI - sectionAngle, 
		a * r_factor, b, pOld);
	pOld = RenderMoldingSection(2*PI - sectionAngle, 2*PI + 0.1, 
		a, b * r_factor, pOld);

	// delete the connecting molding
	delete pOld;

	// draw the text for this scene
//	DrawText("Testing Ellipses", CRect(0, 0, (int)(rectangularity.Get() * 2.0 * a), 
//		(int)(rectangularity.Get() * 2.0 * b)));
}

void CNodeRenderer::DrawText(const CString& strText, CRect rect)
{
	// create a memory device context
	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);

	// create the drawing buffer
	CBitmap buffer;
	buffer.CreateBitmap(rect.Width(), rect.Height(), 1, 32, NULL); 
	CBitmap *pOldBitmap = dcMem.SelectObject(&buffer);

	// now clear the drawing buffer
	CBrush brush;
	brush.CreateSolidBrush(RGB(216, 216, 216));
	CBrush *pOldBrush = dcMem.SelectObject(&brush);

	CPen *pOldPen = (CPen *)dcMem.SelectStockObject(NULL_PEN);

	dcMem.Rectangle(0, 0, rect.Width()+1, rect.Height()+1);

	dcMem.SelectObject(pOldPen);
	dcMem.SelectObject(pOldBrush);

	// draw some text

	// create a font
	int nFontHeight = __min(rect.Height(), 36);
	CFont font;
	font.CreateFont(nFontHeight, // height
		0,						// width
		0, 0,					// escapement, orientation
		FW_DONTCARE,			// weight
		FALSE,					// italic
		FALSE,					// underline
		FALSE,					// strikeout
		ANSI_CHARSET,			// character set
		OUT_TT_PRECIS,			// select TrueType
		CLIP_CHARACTER_PRECIS,	// clipping
		PROOF_QUALITY,			// quality
		TMPF_TRUETYPE,			// pitch and family
		"Arial");				// facename
	CFont *pOldFont = dcMem.SelectObject(&font);

	// draw the text
	dcMem.SetBkMode(TRANSPARENT);
	dcMem.DrawText(strText, rect, 
		DT_CENTER | DT_END_ELLIPSIS | DT_VCENTER); 

	dcMem.SelectObject(pOldFont);
	font.DeleteObject();

	dcMem.SelectObject(pOldBitmap);

	// retrieve the pixels from the buffer
	static unsigned int pixels[1024 * 1024]; 
	// CSize bitmapSize(rect.Width(), rect.Height());

	DWORD dwCount = 
		buffer.GetBitmapBits(rect.Width() * rect.Height() * 4, pixels);
	ASSERT(dwCount == (DWORD)(rect.Width() * rect.Height() * 4));
	buffer.DeleteObject();

	// draw the bitmap
	float zoomFactor = 1.0;
	glPixelZoom(zoomFactor, -zoomFactor);
	glRasterPos3d(0.0 - zoomFactor * (float) rect.Width() / 2.0f, 
		0.0 + zoomFactor * (float) rect.Height() / 2.0f, 
		m_borderRadius + 0.1);
	glDrawPixels(rect.Width(), rect.Height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void CNodeRenderer::SetActivation(double activation)
{
	m_activation = activation;

	// set the new width for the node renderer
	m_width = 30.0 + 100.0 * sqrt(m_activation / m_aspectRatio);

	// set the new rectangularity for the shape
	const double c = 0.99999 - 1.0 / sqrt(2.0);
	double sigmoid = (exp(2.0 * m_activation) - exp(-2.0 * m_activation))
		/ (exp(2.0 * m_activation) + exp(-2.0 * m_activation));
	m_rectangularity = sigmoid * c + 1.0 / sqrt(2.0);

	// set the aspect ratio for the renderer
	m_aspectRatio = 0.75 - 0.375 * exp(-8.0 * m_activation);

	// set the new border radius
	m_borderRadius = 6.0 + sqrt(m_activation) * 5.0;

	Invalidate();
}

