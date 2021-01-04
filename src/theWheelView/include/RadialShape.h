/// <remarks>
/// RadialShape.h
/// Interface of the RadialShape class.
///
/// Copyright (C) 2006-2007 Derek G Lane
/// </remarks>

#pragma once

namespace theWheel2007 {

/// <summary>
/// base class for radial shapes (shapes which are functions of angle)
/// </summary>
class RadialShape
{
public:
	/// <summary>
	/// constructs a new RadialShape
	/// </summer>
	RadialShape(void)
	{
	}

	// default destructor
	virtual ~RadialShape(void)
	{
	}

	// evaluates the radial shape at the given point
	virtual CVectorD<2, REAL> Eval(REAL angle) = 0;

	// inverse evaluation, given Y
	virtual void InvEvalY(REAL Y, REAL& angle1, REAL& angle2) = 0;

	/// <summary>
	/// collection of discontinuities for the shape
	/// </summer>
	std::vector<REAL>& Discontinuities()
	{
		return m_arrDiscont;
	}

	/// <summary>
	/// the outer rectangle for the shape
	/// </summary>
	CExtent<2, REAL> OuterRect;

	/// <summary>
	/// the inner rectangle for the shape
	/// </summer>
	CExtent<2, REAL> InnerRect;

protected:
	// stores the array of discontinuities
	std::vector<REAL> m_arrDiscont;
};	// class RadialShape

}	// namespace theWheel2007