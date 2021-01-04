/// <remarks>
/// Elliptangle.h
/// Interface of the Elliptangle RadialShape class.
///
/// Copyright (C) 2006-2007 Derek G Lane
/// </remarks>

#pragma once

// base class
#include "RadialShape.h"

namespace theWheel2007 {

/// <summary>
/// represents an "elliptangle" radial shape
/// </summary>
class Elliptangle : public RadialShape
{
public:
	/// <summary>
	/// construct a new elliptangle with given extent and elliptangle-ness
	/// </summary>
	/// <param name="extOuter">outer rectangle for the elliptangle</param>
	/// <param name="ellipt">elliptangle-ness</param>
	Elliptangle(CExtent<2, REAL> rectInner, REAL ellipt)
	{
		SetExtentEllipt(rectInner, ellipt);
	}

	// destructor
	virtual ~Elliptangle(void)
	{
	}

	/// <summary>
	// sets the extents (both inner and outer)
	/// </summary>
	/// <param name="extOuter">outer rectangle for the elliptangle</param>
	/// <param name="ellipt">elliptangle-ness</param>
	void SetExtentEllipt(CExtent<2, REAL> rectInner, REAL ellipt);

	/// <summary cref="RadialShape::Eval">
	// evaluates the radial shape at the given point
	/// </summary>
	/// <param name="angle">angle at which to evaluate the radial shape</param>
	/// <returns>point on the elliptangle at the angle</returns>
	virtual CVectorD<2, REAL> Eval(REAL angle) override;

	/// <summary cref="RadialShape::InvEvalY">
	// evaluates the radial shape at the given point
	/// </summary>
	/// <param name="angle">angle at which to evaluate the radial shape</param>
	/// <returns>point on the elliptangle at the angle</returns>
	virtual void InvEvalY(REAL Y, REAL& angle1, REAL& angle2);

protected:

	// the "fat" ellipse extent
	CExtent<2, REAL> m_extFatEllipse;

	// the "tall" ellipse extent
	CExtent<2, REAL> m_extTallEllipse;

};	// class Elliptangle

}	// namespace theWheel2007