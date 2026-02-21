// AxWheelCtrl.h : Declaration of the CAxWheelCtrl
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>

#include <SpaceView.h>
#include "NodeProxy.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


// IAxWheelCtrl
[
	object,
	uuid(D68D28B5-8292-4399-B9ED-01D545664046),
	dual,
	helpstring("IAxWheelCtrl Interface"),
	pointer_default(unique)
]
__interface IAxWheelCtrl : public IDispatch
{
	[propget, id(1), helpstring("property BaseURL")] HRESULT BaseURL([out, retval] BSTR* pVal);
	[propput, id(1), helpstring("property BaseURL")] HRESULT BaseURL([in] BSTR newVal);
	[id(2), helpstring("method WriteXML")] HRESULT WriteXML([in] BSTR bstrXmlFilename);
};


// CAxWheelCtrl
[
	coclass,
	control,
	default(IAxWheelCtrl),
	threading(apartment),
	vi_progid("AxWheel.AxWheelCtrl"),
	progid("AxWheel.AxWheelCtrl.1"),
	version(1.0),
	uuid("ABD048FC-0ED9-4B8C-BEF7-D609B44E7072"),
	helpstring("AxWheelCtrl Class"),
	support_error_info(IAxWheelCtrl),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CAxWheelCtrl :
	public IAxWheelCtrl,
	public IPersistStreamInitImpl<CAxWheelCtrl>,
	public IOleControlImpl<CAxWheelCtrl>,
	public IOleObjectImpl<CAxWheelCtrl>,
	public IOleInPlaceActiveObjectImpl<CAxWheelCtrl>,
	public IViewObjectExImpl<CAxWheelCtrl>,
	public IOleInPlaceObjectWindowlessImpl<CAxWheelCtrl>,
	public IPersistStorageImpl<CAxWheelCtrl>,
	public IPersistPropertyBagImpl<CAxWheelCtrl>,
	public ISpecifyPropertyPagesImpl<CAxWheelCtrl>,
	public IQuickActivateImpl<CAxWheelCtrl>,
//#ifndef _WIN32_WCE
	public IDataObjectImpl<CAxWheelCtrl>,
//#endif
	public IProvideClassInfo2Impl<&__uuidof(CAxWheelCtrl), NULL>,
//#ifdef _WIN32_WCE // IObjectSafety is required on Windows CE for the control to be loaded correctly
//	public IObjectSafetyImpl<CAxWheelCtrl, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
//#endif
	public CComControl<CAxWheelCtrl>
{
public:

	CAxWheelCtrl()
	{
		m_bWindowOnly = true;
	}

	//////////////////////////////////////////////////////
	DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
		OLEMISC_INSIDEOUT |
		OLEMISC_ACTIVATEWHENVISIBLE |
		OLEMISC_SETCLIENTSITEFIRST
		)

		//////////////////////////////////////////////////////
		BEGIN_PROP_MAP(CAxWheelCtrl)
			PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
			PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
			PROP_ENTRY("BaseURL", 1, CLSID_NULL)
			// Example entries
			// PROP_ENTRY("Property Description", dispid, clsid)
			// PROP_PAGE(CLSID_StockColorPage)
		END_PROP_MAP()

		//////////////////////////////////////////////////////
		BEGIN_MSG_MAP(CAxWheelCtrl)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			CHAIN_MSG_MAP(CComControl<CAxWheelCtrl>)
			DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

		// IViewObjectEx
		DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

	// IAxWheelCtrl
public:
	// message handlers
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// drawing function
	HRESULT OnDraw(ATL_DRAWINFO& di);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	// constructor
	HRESULT FinalConstruct()
	{
		// construct the root node / proxy
		m_mapNPs.SetAt(0, new NodeService::NodeProxy(0));

		return S_OK;
	}

	// destructor
	void FinalRelease()
	{
    delete m_pSV;
    delete m_pSpace;
//		POSITION pos = m_mapNPs.GetStartPosition();
//		while (pos != NULL)
//		{
//			NodeService::NodeProxy *pNP = m_mapNPs.GetNextValue(pos);
//			/// TODO: shouldn't need to delete this
//			delete pNP->GetNode();
//			delete pNP;
//		}
	}

	//////////////////////////////////////
	// Properties

	STDMETHOD(get_BaseURL)(BSTR* pVal);
	STDMETHOD(put_BaseURL)(BSTR newVal); 

	CComBSTR m_strBaseURL;
	CComBSTR m_strTitle;

	//////////////////////////////////////
	// Methods

	STDMETHOD(WriteXML)(BSTR bstrXmlFilename);

	//////////////////////////////////////
	// Node Service Helpers

	BSTR GetRootName();

private:
	// the SpaceView
	CSpaceView *m_pSV;
	CSpace *m_pSpace;

	// thread pool for the node proxy retrieval
	CAsyncSoapThreadPool m_threadPool;

	// the node proxies
	NodeService::NodeProxy::Map m_mapNPs;
};

