/*==========================================================================
 *
 *  Copyright (C) 1995 Microsoft Corporation. All Rights Reserved.
 *
 *  File:       ddutil.cpp
 *  Content:    Routines for loading bitmap and palettes from resources
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */

//////////////////////////////////////////////////////////////////////
// CHECK_HRESULT
// 
// macro to check the result of a DirectDraw call
//////////////////////////////////////////////////////////////////////
#define CHECK_HRESULT(x) \
	if (!SUCCEEDED(x))			\
    {							\
		return FALSE;			\
    }	


//////////////////////////////////////////////////////////////////////
// ASSERT_HRESULT
// 
// macro to assert the result of a DirectDraw call
//////////////////////////////////////////////////////////////////////
#define ASSERT_HRESULT(x) \
	{							\
		HRESULT hr = (x);		\
		ASSERT(SUCCEEDED(hr));	\
    }	


//////////////////////////////////////////////////////////////////////
// ASSERT_BOOL
// 
// macro to assert the result of a DirectDraw call
//////////////////////////////////////////////////////////////////////
#define ASSERT_BOOL(x) \
	{							\
		BOOL br = (x);		\
		ASSERT(br);	\
    }	


//////////////////////////////////////////////////////////////////////
// GET_ATTACH_DC
// 
// macro to obtain a DC and attach to a CDC
//////////////////////////////////////////////////////////////////////
#define GET_ATTACH_DC(lpDDS, dc) \
	{								\
		HDC hdc;					\
		ASSERT_HRESULT(lpDDS->GetDC(&hdc));	\
		dc.Attach(hdc);				\
    }	


//////////////////////////////////////////////////////////////////////
// RELEASE_DETACH_DC
// 
// macro to release an attached DC
//////////////////////////////////////////////////////////////////////
#define RELEASE_DETACH_DC(lpDDS, dc) \
	{								\
		HDC hdc = dc.m_hDC;			\
		dc.Detach();				\
		ASSERT_HRESULT(lpDDS->ReleaseDC(hdc));	\
    }	


extern IDirectDrawPalette * DDLoadPalette(IDirectDraw *pdd, LPCSTR szBitmap);
extern IDirectDrawSurface * DDLoadBitmap(IDirectDraw *pdd, LPCSTR szBitmap, int dx, int dy);
extern HRESULT              DDReLoadBitmap(IDirectDrawSurface *pdds, LPCSTR szBitmap);
extern HRESULT              DDCopyBitmap(IDirectDrawSurface *pdds, HBITMAP hbm, int x, int y, int dx, int dy);
extern DWORD                DDColorMatch(IDirectDrawSurface *pdds, COLORREF rgb);
extern HRESULT              DDSetColorKey(IDirectDrawSurface *pdds, COLORREF rgb);

#ifdef __cplusplus
}
#endif	/* __cplusplus */
