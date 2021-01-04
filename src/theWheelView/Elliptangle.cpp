/// <remarks>
/// Elliptangle.cpp
/// Implementation of the Elliptangle RadialShape class.
///
/// Copyright (C) 2006-2007 Derek G Lane
/// </remarks>

#include "StdAfx.h"

// class declaration
#include "Elliptangle.h"

namespace theWheel2007 {

/////////////////////////////////////////////////////////////////////////////
void 
	Elliptangle::SetExtentEllipt(CExtent<2, REAL> rectInner, REAL ellipt)
		// sets the extents (both inner and outer)
{
	// set inner extent
	InnerRect = rectInner;

	// calculate outer extent
	rectInner.Deflate(
		-(1.0f - ellipt) * rectInner.GetSize(0) / 2.0f, 
		-(1.0f - ellipt) * rectInner.GetSize(1) / 2.0f,
		-(1.0f - ellipt) * rectInner.GetSize(0) / 2.0f, 
		-(1.0f - ellipt) * rectInner.GetSize(1) / 2.0f);
	OuterRect = rectInner;

	// now calculate the supporting extents

	// calculate the fat ellipse extent

	// compute the fat ellipse's b
	REAL ry = InnerRect.GetSize(1) / OuterRect.GetSize(1); 
	REAL by = sqrt(InnerRect.GetSize(0) * InnerRect.GetSize(0) * 0.25f
						/ (1.0f - ry * ry));

	// compute the fat ellipse's rectangle
	m_extFatEllipse = OuterRect;
	m_extFatEllipse.Deflate(m_extFatEllipse.GetSize(0) / 2.0f, 0, m_extFatEllipse.GetSize(0) / 2.0f, 0);
	m_extFatEllipse.Deflate(-by, 0, -by, 0);


	// calculate the tall ellipse extent

	// compute the tall ellipse's b
	REAL rx = InnerRect.GetSize(0) / OuterRect.GetSize(0); 
	REAL bx = sqrt(InnerRect.GetSize(1) * InnerRect.GetSize(1) * 0.25f
						/ (1.0f - rx * rx));

	// compute the tall ellipse's rectangle
	m_extTallEllipse = OuterRect;
	m_extTallEllipse.Deflate(0, m_extTallEllipse.GetSize(1) / 2.0f, 0, m_extTallEllipse.GetSize(1) / 2.0f);
	m_extTallEllipse.Deflate(0, -bx, 0, -bx);


	// now set up discont list

	// compute the angle dividing the sections of the elliptangle
	REAL sectionAngle = atan2(OuterRect.GetSize(1), OuterRect.GetSize(0));

	// determine whether angle is in fat or tall ellipse
	m_arrDiscont.resize(4); // = gcnew array<REAL>(4);
	m_arrDiscont[0] = sectionAngle;
	m_arrDiscont[1] = PIf - sectionAngle;
	m_arrDiscont[2] = PIf + sectionAngle;
	m_arrDiscont[3] = 2.0f * PIf - sectionAngle;

}	// Elliptangle::SetExtentEllipt


//////////////////////////////////////////////////////////////////////////
CVectorD<2, REAL>
	Elliptangle::Eval(REAL theta)
		// helper function to evaluate an elliptangle
{
	CVectorD<2, REAL> vPoint;

	// check theta range
	if (theta < 0.0f)
	{
		vPoint = Eval(theta + 2.0f * PIf);
	}
	else if (2.0f * PIf < theta)
	{
		vPoint = Eval(theta - 2.0f * PIf);
	}
	else
	{
		// compare tan(theta) to slope, to see which section we are in 
		REAL slope = OuterRect.GetSize(1) / OuterRect.GetSize(0);
		if (fabs(tan(theta)) >= fabs(slope))
		{
			// tangent greater than slope, so we are in fat ellipse domain
			vPoint[0] = m_extFatEllipse.GetSize(0) / 2.0f;
			vPoint[1] = m_extFatEllipse.GetSize(1) / 2.0f;
		}
		else
		{
			// tangent less than slope, so we are in tall ellipse domain
			vPoint[0] = m_extTallEllipse.GetSize(0) / 2.0f;
			vPoint[1] = m_extTallEllipse.GetSize(1) / 2.0f;
		}

		// compute theta_prime for the ellipse
		REAL sign = cos(theta) > 0 ? 1.0f : -1.0f;
		REAL theta_prime = atan2(sign * vPoint[0] * tan(theta), sign * vPoint[1]);

		vPoint[0] *= cos(theta_prime);
		vPoint[1] *= sin(theta_prime);
	}

	return vPoint;

}	// Elliptangle::Eval

//////////////////////////////////////////////////////////////////////////
void 
	Elliptangle::InvEvalY(REAL Y, REAL& angle1, REAL& angle2)
{
	// compute point on tall ellipse
	REAL a = m_extTallEllipse.GetSize(0) / 2.0f;
	REAL b = m_extTallEllipse.GetSize(1) / 2.0f;
	REAL X = a * sqrt(1.0f - Y * Y / (b * b));

	// see if the point is greater than tall slope
	REAL slope = OuterRect.GetSize(1) / OuterRect.GetSize(0);
	if (fabs(Y / X) >= fabs(slope))
	{
		// yes, so compute for fat ellipse
		a = m_extFatEllipse.GetSize(0)  / 2.0f;
		b = m_extFatEllipse.GetSize(1) / 2.0f;
		X = a * sqrt(1.0f - Y * Y / (b * b));
	}

	// assign the values
	angle1 = atan2(Y, X);
	angle2 = atan2(Y, -X);
}

}	// namespace theWheel2007