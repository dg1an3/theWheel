/// GeometryWrappers.cpp
/// Compiles RadialShape and Elliptangle source files within theWheelGL context.
/// These source files include "StdAfx.h" — we block theWheelView's MFC stdafx.h
/// by pre-defining its include guard. Our StdAfx.h shim is force-included via CMake.

// Block theWheelView's MFC-based stdafx.h
#define AFX_STDAFX_H__0C8AA656_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

// The actual implementations
#include "../theWheelView/RadialShape.cpp"
#include "../theWheelView/Elliptangle.cpp"
