// AxWheel.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "AxWheel.h"
#include "dlldatax.h"


class CAxWheelModule : public CAtlDllModuleT< CAxWheelModule >
{
public :
	DECLARE_LIBID(LIBID_AxWheelLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_AXWHEEL, "{A8EE08F3-083B-4639-AF2C-4D0EDE69AB6A}")
};

CAxWheelModule _AtlModule;

class CAxWheelApp : public CWinApp
{
public:

// Overrides
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CAxWheelApp, CWinApp)
END_MESSAGE_MAP()

CAxWheelApp theApp;

BOOL CAxWheelApp::InitInstance()
{
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(m_hInstance, DLL_PROCESS_ATTACH, NULL))
		return FALSE;
#endif

	  // call static init of IPP library
	  IppStatus ippInitStat = ippStaticInit();
	  if (ippStsNoErr != ippInitStat
		  && ippStsDllNotFoundBestUsed != ippInitStat)
	  {
		  ::AfxMessageBox(_T("Problem initializing IPP"));
	  }
	  
    return CWinApp::InitInstance();
}

int CAxWheelApp::ExitInstance()
{
    return CWinApp::ExitInstance();
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hr = PrxDllCanUnloadNow();
    if (hr != S_OK)
        return hr;
#endif
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _AtlModule.GetLockCount()==0) ? S_OK : S_FALSE;
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
#ifdef _MERGE_PROXYSTUB
    if (FAILED(hr))
        return hr;
    hr = PrxDllRegisterServer();
#endif
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
#ifdef _MERGE_PROXYSTUB
    if (FAILED(hr))
        return hr;
    hr = PrxDllRegisterServer();
    if (FAILED(hr))
        return hr;
    hr = PrxDllUnregisterServer();
#endif
	return hr;
}

[
  module(unspecified, uuid = "{EEB23191-CFE6-4CBE-A421-5128E99A1DCE}", helpstring = "AxWheel 1.0 Type Library", name = "AxWheelLib")
];
