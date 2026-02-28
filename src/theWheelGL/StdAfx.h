/// StdAfx.h — minimal shim for compiling theWheelGL and geometry files
/// outside theWheelView. Provides the same types that theWheelView's stdafx.h
/// brings in via MFC headers, but without any MFC or D3D dependencies.

#pragma once

// Standard library
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <cassert>

// On MSVC, Extent.h uses RECT from windows.h.
// On other platforms, wxcompat.h/mfc_compat.h provides stubs.
#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include "mfc_compat.h"
#endif

// VectorD.h uses ASSERT (MFC macro) — provide a stub
#ifndef ASSERT
#define ASSERT(expr) assert(expr)
#endif

// OptimizeN math types (includes REAL, PIf, CVectorD, CExtent)
#include <MathUtil.h>
#include <VectorD.h>
#include <Extent.h>
#include <UtilMacros.h>
