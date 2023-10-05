/****************************************************************************
* FPATLCTL.H  -  Declaration of FarPoint's CfpAtlCtl class
*
* Copyright (C) 1991-1997 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
****************************************************************************/
#ifndef FPATLCTL_H
#define FPATLCTL_H

#include "fpatl.h"        // Include definition of subclass (CfpAtlBase)
#include "fpatlvbx.h"
#include "mshtmdid.h"

#define CTLHWND   m_hWnd

#ifdef __cplusplus
extern "C" {
#endif

typedef HRESULT (__cdecl *FPATL_LOADPROPSET_FUNCTYPE)(LPSTREAM *pRetStmDest, 
  LPSTREAM pStmSrc, ATL_PROPMAP_ENTRY* pMap, CfpAtlBase *pfpAtlCtl);
#define FPATL_LOADPROPSET_FUNC "fpLoadPropsetTofpStream"

typedef HRESULT (__cdecl *FPATL_VBXSAVESTREAM_FUNCTYPE)(LPSTREAM *ppstmSrc, 
  LPSTREAM pStmDest, BOOL fClearDirty, DWORD dwCtlVersion, 
  SIZE *psizeExtent, CfpAtlBase *pfpAtlCtl);
#define FPATL_VBXSAVESTREAM_FUNC "fpAtlVbxSaveFromfpStream"

#ifdef __cplusplus
}
#endif    

//---------------------------------------------------------------------------
// Macro for declaring Control class
//---------------------------------------------------------------------------
#define FP_DECLARE_SUBCTL_WNDCLASSNAME(WndClassName) \
static LPTSTR fpGetSubCtlClassName() \
{ \
  static LPTSTR lpszSubCtl = _T(WndClassName); \
  return lpszSubCtl; \
} \
static LPTSTR fpGetHookedSubCtlClassName() \
{ \
  static LPTSTR lpszHookedSubCtl = _T("ATL") _T(WndClassName); \
  return lpszHookedSubCtl; \
}

#define FP_DECLARE_SUBCTL_MSGMAPID(dwMsgMapID) \
static DWORD fpGetSubCtlMsgMapID() \
{ \
  static DWORD dwSubCtlMsgMapID = dwMsgMapID; \
  return dwSubCtlMsgMapID; \
}

#define FP_DECLARE_PROPLIST(PropList, PropCt) \
static void fpGetPropList(LPFP_PROPLIST *lpRetPropList, int *lpRetPropCt) \
{ \
  *lpRetPropList = PropList; \
  *lpRetPropCt = PropCt; \
}

#define FP_DECLARE_SIMPLEFRAMECTL(fIsSimpleFrame) \
static BOOL fpIsSimpleFrameCtl() \
{ \
  static BOOL fSimpleFrame = fIsSimpleFrame; \
  return fSimpleFrame; \
}

#define FP_DECLARE_VBXCONVERT_DLLNAME(DllName) \
static LPCTSTR fpGetVbxCnvtDllName() \
{ \
  static LPCTSTR lpszDllName = _T(DllName); \
  return lpszDllName; \
}

//---------------------------------------------------------------------------
// Macro for declaring MESSAGE MAP
//---------------------------------------------------------------------------
// NOTE: This declares 2 functions
#define FPATL_BEGIN_MSG_MAP(theClass) \
public: \
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0) \
	{ \
      BOOL fRet;\
      LPUNKNOWN pUnk = m_pT->GetUnknown();\
      pUnk->AddRef();\
      if (m_fSimpleFrame) \
        fRet = fpAtlProcessSimpleFrameMsg(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);\
      else \
        fRet = fpAtlProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);\
      pUnk->Release();\
      return fRet;\
    } \
	BOOL fpAtlProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0) \
	{ \
		BOOL bHandled = TRUE; \
		hWnd; \
		switch(dwMsgMapID) \
		{ \
		case 0:

#define FPATL_END_MSG_MAP() \
			break; \
		} \
		return fpAtlDefaultProcessWndMsg(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);\
	}

//---------------------------------------------------------------------------
// Macro for declaring VERB MAP
//---------------------------------------------------------------------------
#define FPATL_BEGIN_VERB_MAP(theClass) \
public: \
	static const FPATL_VERB_ENTRY* fpAtlGetVerbMap() \
	{ \
      static FPATL_VERB_ENTRY _VerbEntries[] = { \

#define FPATL_END_VERB_MAP() \
		{0, 0, FPATL_VERB_RESID_LAST, (FPATL_PVERBFN)0 } \
	  }; \
	  return &_VerbEntries[0]; \
    }

#define FPATL_STDOLEVERB(iVerb, memberFxn) \
	{ iVerb, NULL, 0, (FPATL_PVERBFN)&memberFxn },

#define FPATL_OLEVERB(iVerb, strVerbName, iResIdVerbName, memberFxn) \
	{ iVerb, OLESTR(strVerbName), iResIdVerbName, (FPATL_PVERBFN)&memberFxn },


//---------------------------------------------------------------------------
// Macro for declaring an alternate property map corresponding to the
// properties that would be found in a property bag saved by an old
// version of the control. Use the other standard property map macros with
// this to define the function GetPropertyMapAlt().  -scl
//---------------------------------------------------------------------------
#define BEGIN_PROPERTY_MAP_ALT(theClass) \
	typedef _ATL_PROP_NOTIFY_EVENT_CLASS __ATL_PROP_NOTIFY_EVENT_CLASS; \
	static ATL_PROPMAP_ENTRY* GetPropertyMapAlt()\
	{\
		static ATL_PROPMAP_ENTRY pPropMap[] = \
		{


//---------------------------------------------------------------------------
// VC60's "BEGIN_PROPERTY_MAP" macro is different than 
// VC50's "BEGIN_PROPERTY_MAP" macro.  However, VC60's BEGIN_PROP_MAP macro
// is the same as VC50's "BEGIN_PROPERTY_MAP".
// Normalize on "BEGIN_PROP_MAP".
//---------------------------------------------------------------------------
#ifndef BEGIN_PROP_MAP
#define BEGIN_PROP_MAP  BEGIN_PROPERTY_MAP
#endif

//---------------------------------------------------------------------------
// Macro for declaring property functions
//---------------------------------------------------------------------------
#define FP_ATLPROPFUNC_W_INDEX(Cls, PropName, PropType, PropId, DispId, sIndex) \
STDMETHODIMP Cls::get_##PropName(PropType *pVal) \
{ long lValue; \
  long lRet = fpGetProp(PropId, (long)sIndex, &lValue); \
  if (lRet == S_OK) \
    *pVal = *(PropType*)&lValue; \
  return (HRESULT)lRet; \
}  \
STDMETHODIMP Cls::put_##PropName(PropType newVal) \
{ \
	if (FireOnRequestEdit(DispId) == S_FALSE) \
		return S_FALSE; \
  HRESULT hr; \
hr = (HRESULT)fpSetProp(PropId, *(long*)&newVal, (long)sIndex, DispId); \
fpAtlSetModifiedFlag(); \
FireOnChanged(DispId); \
return hr; \
} 

#define FP_ATLPROPFUNC_W_INDEX_NO_GET(Cls, PropName, PropType, PropId, DispId, sIndex) \
STDMETHODIMP Cls::put_##PropName(PropType newVal) \
{ HRESULT hr; \
if (FireOnRequestEdit(DispId) == S_FALSE) \
  return S_FALSE; \
hr = (HRESULT)fpSetProp(PropId, *(long*)&newVal, (long)sIndex, DispId); \
fpAtlSetModifiedFlag(); \
FireOnChanged(DispId); \
return hr; \
} 

#define FP_ATLPROPFUNC_W_INDEX_NO_PUT(Cls, PropName, PropType, PropId, DispId, sIndex) \
STDMETHODIMP Cls::get_##PropName(PropType *pVal) \
{ long lValue; \
  long lRet = fpGetProp(PropId, (long)sIndex, &lValue); \
  if (lRet == S_OK) \
    *pVal = *(PropType*)&lValue; \
  return (HRESULT)lRet; \
}

#define FP_ATLPROPFUNC(Cls, PropName, PropType, PropId, DispId) \
FP_ATLPROPFUNC_W_INDEX(Cls, PropName, PropType, PropId, DispId, 0)

#define FP_ATLPROPFUNC_NO_GET(Cls, PropName, PropType, PropId, DispId) \
FP_ATLPROPFUNC_W_INDEX_NO_GET(Cls, PropName, PropType, PropId, DispId, 0)

#define FP_ATLPROPFUNC_NO_PUT(Cls, PropName, PropType, PropId, DispId) \
FP_ATLPROPFUNC_W_INDEX_NO_PUT(Cls, PropName, PropType, PropId, DispId, 0)

#define FP_ATLFONTPROPFUNC(Cls, PropName, FontMemberVar, DispId) \
STDMETHODIMP Cls::get_##PropName(IFontDisp** ppFont) \
  { return fpAtl_get_Font(ppFont, FontMemberVar); } \
STDMETHODIMP Cls::putref_##PropName(IFontDisp* pFont) \
  { return fpAtl_putref_Font(pFont, DispId, FontMemberVar); } \
STDMETHODIMP Cls::put_##PropName(IFontDisp* pFont) \
  { return fpAtl_put_Font(pFont, DispId, FontMemberVar); }

#define FP_ATLPICTPROPFUNC_W_INDEX(Cls, PropName, PropId, DispId, sIndex) \
STDMETHODIMP Cls::get_##PropName(LPPICTUREDISP *ppPict) \
{ long lValue; \
  long lRet = fpGetProp(PropId, (long)sIndex, &lValue); \
  if (lRet == S_OK) \
    *ppPict = (LPPICTUREDISP)lValue; \
  return (HRESULT)lRet; \
}  \
STDMETHODIMP Cls::putref_##PropName(LPPICTUREDISP pPict) \
  { \
  HRESULT hr; \
	if (FireOnRequestEdit(DispId) == S_FALSE) \
		return S_FALSE; \
  hr = (HRESULT)fpSetProp(PropId, (long)pPict, (long)sIndex, DispId); \
  fpAtlSetModifiedFlag(); \
	FireOnChanged(DispId); \
  return hr; \
  } \
STDMETHODIMP Cls::put_##PropName(LPPICTUREDISP pPict) \
  { \
	if (FireOnRequestEdit(DispId) == S_FALSE) \
		return S_FALSE; \
  LPPICTUREDISP pPictClone = NULL; \
  HRESULT hr = fpAtl_ClonePicture(pPict, DispId, &pPictClone); \
  if (hr == S_OK) \
    { hr = (HRESULT)fpSetProp(PropId, (long)pPictClone, (long)sIndex, DispId); \
/* fix for SCS9117 -scl */ \
      if (pPictClone) pPictClone->Release(); \
    } \
  fpAtlSetModifiedFlag(); \
	FireOnChanged(DispId); \
  return hr; \
  }

#define FP_ATLPICTPROPFUNC(Cls, PropName, PropId, DispId) \
FP_ATLPICTPROPFUNC_W_INDEX(Cls, PropName, PropId, DispId, 0)

#define FP_ATL_IMPL_BSTR_PROP(Cls, fname, pname, dispid) \
HRESULT STDMETHODCALLTYPE Cls::put_##fname(BSTR pname) \
{ \
	if (FireOnRequestEdit(dispid) == S_FALSE) \
		return S_FALSE; \
	m_##pname = pname; \
	fpAtlSetDirty(TRUE); \
	FireOnChanged(dispid); \
	FireViewChange(); \
	return S_OK; \
} \
HRESULT STDMETHODCALLTYPE Cls::get_##fname(BSTR* p##pname) \
{ \
	*p##pname = m_##pname.Copy(); \
	return S_OK; \
}

#define FP_ATL_INITSTATICVARS(Cls) \
int Cls::m_iBlobProp = FPATL_BLOBPROP_UNINIT; \
LPCTSTR Cls::m_lptszBlobPropName = NULL;

#define FPATL_DEFAULT_HANDLER(msg, func) \
case msg: \
{ \
  bHandled = TRUE; \
  lResult = m_pT->func(uMsg, wParam, lParam, bHandled); \
  if(bHandled) \
    return TRUE; \
  break; \
}


//---------------------------------------------------------------------------
// FarPoint's Base class (template) for controls using ATL
//---------------------------------------------------------------------------
// Example (From ButtonMaker):
//
// #define BTN_FPBASECLS CfpAtlCtl<CBtnCtl, &CLSID_BtnCtl, IBtnCtl, &IID_IBtnCtl, &LIBID_BTNLib>
// class ATL_NO_VTABLE CBtnCtl : public BTN_FPBASECLS ...
//
//---------------------------------------------------------------------------
template <class T, const CLSID* pclsid,  // as on CComCoClass
  class InterfaceName, const IID* piid, const GUID* plibid, // as on IDispatchImpl
  const IID* psrcid = NULL, // defaults as on 2nd parameter on IProvideClassInfo2Impl
  WORD wMajor = 1, WORD wMinor = 0,    // defaults as on IDispatchImpl
  class tihclass = CComTypeInfoHolder> // defaults as on IDispatchImpl
class ATL_NO_VTABLE CfpAtlCtl : 
    public CfpAtlBase,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<T, pclsid>,
	public CComControl<T>,
//	public IDispatchImpl<InterfaceName, piid, plibid, wMajor, wMinor, tihclass>,
	public CStockPropImpl<T, InterfaceName, piid, plibid, wMajor, wMinor>,
	public IProvideClassInfo2Impl<pclsid, psrcid, plibid, wMajor, wMinor, tihclass>,
	public IPersistStreamInitImpl<T>,
	public IPersistStorageImpl<T>,
	public IQuickActivateImpl<T>,
	public IOleControlImpl<T>,
	public IOleObjectImpl<T>,
	public IOleInPlaceActiveObjectImpl<T>,
	public IViewObjectExImpl<T>,
	public IOleInPlaceObjectWindowlessImpl<T>, // Needed by our controls? -SCP
	public IDataObjectImpl<T>,
	public ISpecifyPropertyPagesImpl<T>,       // Needed by our controls? -SCP
    public IPerPropertyBrowsingImpl<T>,
  	public IConnectionPointContainerImpl<T>,
    public IPersistPropertyBagImpl<T>,
    public IPropertyNotifySinkCP<T>
{
public:
    T* m_pT;                   // ptr to parent class

#define FPATL_BLOBPROP_UNINIT   -2
#define FPATL_BLOBPROP_NONE     -1
    static int m_iBlobProp;  // FPATL_BLOBPROP_UNINIT
    static LPCTSTR m_lptszBlobPropName;    // storage of blob prop name

	LPSIMPLEFRAMESITE m_pSimpleFrameSite;
    BOOL m_fSimpleFrame;                   
    BOOL m_fCtlOnDesktop;  
    BOOL m_bPendingUIActivation;
	int  m_iDblClkState;
	int  m_iButtonState;
    BOOL m_fClientSiteSet;
    LPSTREAM m_pstmVbxLoad;      // Tmp IStream of prop values
    HINSTANCE m_hVbxConvertDll;  // Instance handle for loaded dll.
	CComPtr<IOleControlSite> m_spControlSite;

    //----------------------------------------------------------------
    // Constructor for class.
    //----------------------------------------------------------------
    CfpAtlCtl() :
      m_pT(static_cast<T*>(this)),
      CfpAtlBase(m_pT->fpGetSubCtlClassName(), this, m_pT->fpGetSubCtlMsgMapID())
    {
      m_pT->fpGetPropList(&m_PropList, &m_PropCt);
      _ASSERT(m_PropList);  // Subclass should override with "FP_DECLARE_PROPLIST()"
      m_fSimpleFrame = m_pT->fpIsSimpleFrameCtl();
      m_pSimpleFrameSite = NULL;
      m_fCtlOnDesktop = FALSE;
      m_bPendingUIActivation = FALSE;
	  m_iDblClkState = 0;
	  m_iButtonState = 0;
      m_fClientSiteSet = FALSE;
      m_pstmVbxLoad = NULL;
      m_hVbxConvertDll = NULL;
    }

    //----------------------------------------------------------------
    // Destructor for class.  Frees interface ptrs and other required
    // clean-up.
    //----------------------------------------------------------------
    virtual ~CfpAtlCtl()
    {
      if (m_pSimpleFrameSite) // release ptr
        m_pSimpleFrameSite->Release();
      if (m_pstmVbxLoad)      // release stream
        m_pstmVbxLoad->Release();
	  if (m_pStmLoad)
	  {
		m_pStmLoad->Release();
		m_pStmLoad = NULL;
	  }
      if (m_hVbxConvertDll)   // free unload dll
        FreeLibrary(m_hVbxConvertDll);
	  }

    //----------------------------------------------------------------
    // Defaults--usually overridden by subclass
    //----------------------------------------------------------------
    FP_DECLARE_SUBCTL_WNDCLASSNAME("Static")
    FP_DECLARE_SUBCTL_MSGMAPID(0)
    FP_DECLARE_PROPLIST(NULL, 0)
    FP_DECLARE_SIMPLEFRAMECTL(FALSE)
    static LPTSTR fpGetVbxCnvtDllName() { return NULL; }  // default

    //----------------------------------------------------------------
    // Default (empty) verb list.
    //----------------------------------------------------------------
    FPATL_BEGIN_VERB_MAP(CfpAtlCtl)
    FPATL_END_VERB_MAP()

    static DWORD fpGetCtlVersion() { return MAKELONG(wMinor, wMajor); }


    //----------------------------------------------------------------
    // This function implements the class info used to create the
    // control's window.  CfpAtlWndClassInfo is a subclass of
    // ATL's CWndClassInfo class used to create the control's window.
    //----------------------------------------------------------------
//  static CWndClassInfo& GetWndClassInfo()
    static CfpAtlWndClassInfo& GetWndClassInfo()
    {
      static WNDCLASSEX wc_ex = 
		{ sizeof(WNDCLASSEX), NULL, StartWindowProc, 
		  0, 0, 0, 0, 0, NULL, 0, 
          T::fpGetHookedSubCtlClassName(), // prefix class name with "ATL"
          0 };
	  static CfpAtlWndClassInfo wc( wc_ex,
		T::fpGetSubCtlClassName(), NULL, NULL, TRUE, 0, _T("")); 
	  return wc;
    }

    //----------------------------------------------------------------
    // Creates the control's window. After the window has been
    // created, the property values are loaded into the control.
    //----------------------------------------------------------------
// fix for bug 9635 -scl
//	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
	virtual HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE, DWORD dwExStyle = 0,
			UINT nID = 0)
	{
      HWND hWnd;
      dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	  hWnd = CWindowImpl<T>::Create(hWndParent, rcPos, szWindowName, 
        dwStyle, dwExStyle,	nID);

      m_pT->fpOnCreatePreLoad();
      m_pT->m_fIsLoadingWithHWnd = TRUE;  // indicate that props are being loaded
      m_pT->fpAtlLoad();
      m_pT->m_fIsLoadingWithHWnd = FALSE;
      m_pT->fpOnCreatePostLoad();

      return hWnd;
	}

    //----------------------------------------------------------------
    // Interface function.
    //----------------------------------------------------------------
	STDMETHOD(SetObjectRects)(LPCRECT prcPos,LPCRECT prcClip)
	{
        HRESULT hr;
		hr = IOleInPlaceObjectWindowlessImpl<T>::SetObjectRects(prcPos, prcClip);
		return hr;
	}


    //----------------------------------------------------------------
    // Interface function, called to set the control size.
    //----------------------------------------------------------------
	STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL *psizel)
	{
        HRESULT h;
        SIZE size;

        // Call default function.
		h = m_pT->IOleObject_SetExtent(dwDrawAspect, psizel);

        // if func did not return S_OK, or we don't have a hWnd yet, return.
        if (!m_fCtlOnDesktop || h != S_OK || !m_hWnd)
          return h;

        // Convert to pixels, if needed (result size is in "size").
    	if (!m_bDrawGetDataInHimetric)
    	  AtlHiMetricToPixel(&m_sizeExtent, &size);
        else
          size = m_sizeExtent;

        // Resize subctl window. // -- and don't move it -scl
		::SetWindowPos(CTLHWND, NULL, 0,
			0, size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

        // return results
        return h;
	}

    STDMETHOD(OnChanged)(DISPID dispID)
    {
      return S_OK;
    }

    STDMETHOD(OnRequestEdit)(DISPID dispID)
    {
      return S_OK;
    }


    //----------------------------------------------------------------
    // This function creates the control's window.  If this function
    // is being called, it's because the window was not created 
    // upon activation.  The window is created with the Desktop as
    // its parent (and invisible do to that fact).
    //----------------------------------------------------------------
    virtual HWND fpAtlCreateControlWindow(void)
	{
      if (m_hWnd == NULL)
      {
        SIZE size;
        RECT rcPos = {0, 0, m_sizeExtent.cx, m_sizeExtent.cy};
        HWND hWndParent;

    	if (!m_bDrawGetDataInHimetric)
        {
    	  AtlHiMetricToPixel(&m_sizeExtent, &size);
          rcPos.right = size.cx; rcPos.bottom = size.cy;
        }

        // if run-time, try using Active window, else use desktop
        m_fCtlOnDesktop = TRUE;
        hWndParent = ::GetDesktopWindow();

        Create(hWndParent, rcPos, NULL, WS_CHILD);
	  }
	
      return m_hWnd;
    }

#define fpATL_RECTWIDTH(rect)  (int)((rect).right-(rect).left)
#define fpATL_RECTHEIGHT(rect) (int)((rect).bottom-(rect).top)
    //----------------------------------------------------------------
    // This function is called to draw the control (but not as
    // metafile and without a non-client area).  If the control has
    // not been created, this function will created it.
    //----------------------------------------------------------------
    virtual void fpAtlOnDrawSubCtl(ATL_DRAWINFO& di)
    {
      // If no hWnd, create control
      if (m_hWnd == NULL)
        m_pT->fpAtlCreateControlWindow();

      RECT rcBounds;
      CopyRect(&rcBounds, (CONST RECT *)di.prcBounds);

	  if (CTLHWND != NULL)
	  { 
		RECT rcClient;
		::GetClientRect(CTLHWND, &rcClient);

//		if (rcClient.Size() != rcBounds.Size())
        if (fpATL_RECTWIDTH(rcClient) != fpATL_RECTWIDTH(rcBounds) ||
            fpATL_RECTHEIGHT(rcClient) != fpATL_RECTHEIGHT(rcBounds))
		{
			SetMapMode(di.hdcDraw, MM_ANISOTROPIC);
			SetWindowExtEx(di.hdcDraw, rcClient.right, rcClient.bottom, NULL);
			SetViewportExtEx(di.hdcDraw,
              fpATL_RECTWIDTH(rcBounds), fpATL_RECTHEIGHT(rcBounds), NULL);
		}
		SetWindowOrgEx(di.hdcDraw, 0, 0, NULL);
		SetViewportOrgEx(di.hdcDraw, rcBounds.left, rcBounds.top, NULL);

        BOOL bWin4 = FALSE;
        m_pT->DefWindowProc(
	 	          (bWin4 ? WM_PRINT : WM_PAINT),
				  (WPARAM)(di.hdcDraw),
				  (LPARAM)(bWin4 ? PRF_CHILDREN | PRF_CLIENT : 0));
	  }
    }


    //----------------------------------------------------------------
    // This virtual function is called to set the size of the control.
    // It may be called on a CfpCtlBase object ptr.
    //----------------------------------------------------------------
    virtual BOOL fpAtlSetControlSize(int cx, int cy)
    {
      SIZEL szlPixels;
      SIZEL szlHimetric;
      szlPixels.cx = cx;
      szlPixels.cy = cy;
      AtlPixelToHiMetric(&szlPixels, &szlHimetric);
      return SUCCEEDED(m_pT->SetExtent(DVASPECT_CONTENT, &szlHimetric));
    }

    //----------------------------------------------------------------
    // This virtual function is called to indicate that a Bound
    // property has changed.
    //----------------------------------------------------------------
    virtual void fpAtlBoundPropertyChanged(DISPID dispID)
    {
      m_pT->FireOnChanged(dispID);
    }

    //----------------------------------------------------------------
    // This virtual functionis called to determine if a Bound property
    // is allowed to change.
    //----------------------------------------------------------------
    virtual BOOL fpAtlBoundPropertyRequestEdit(DISPID dispID)
    {
      return (m_pT->FireOnRequestEdit(dispID) == S_OK);
    }

    //----------------------------------------------------------------
    // This virtual function indicates whether the control is in User
    // mode or Design mode. 
    //----------------------------------------------------------------
    virtual BOOL fpAtlGetUserMode()
    {
      BOOL fRet;
	  HRESULT h = m_pT->GetAmbientUserMode(fRet);

// fix for bug ????? -scl
//      return (h == S_OK ? fRet : FALSE);
      return (h == S_OK ? fRet : TRUE);
    }

    //----------------------------------------------------------------
    // This virtual function sets or resets the "IsDirty" flag for
    // the control.  It really just sets a member variable which is
    // used by an ATL function which is called when the container wants
    // to query the status.
    //----------------------------------------------------------------
    virtual void fpAtlSetDirty(BOOL bDirty)
    {
      m_pT->m_bRequiresSave = bDirty;
    }

    //----------------------------------------------------------------
    // This virtual function is called to fire one of the control's
    // events.  This MUST BE OVERRIDDEN by your control if you are
    // going to call this function.  To override it, you typically
    // would switch on the EventId and call the corresponding 
    // "Fire" function.
    //----------------------------------------------------------------
    virtual long fpAtlFireEvent(USHORT usidEvent, LPVOID lpparams)
    {
      _ASSERT(FALSE);  // Subclass must override this function!
      return 0;
    }

    //----------------------------------------------------------------
    // This virtual function is called to get the Client site interface
    // ptr.  This ptr must be released by the caller.
    //----------------------------------------------------------------
    virtual LPOLECLIENTSITE fpAtlGetClientSite()
    {
      LPOLECLIENTSITE lpClientSite;
      HRESULT h = m_pT->GetClientSite(&lpClientSite);
      return (h == S_OK ? lpClientSite : NULL);
    }

    //----------------------------------------------------------------
    // This virtual function is called to get an ambient property's
    // value.
    //----------------------------------------------------------------
	virtual HRESULT fpAtlGetAmbientProperty(DISPID dispid, VARIANT& var)
    {
      return m_pT->GetAmbientProperty(dispid, var);
    }

    //----------------------------------------------------------------
    // This virtual function is called to get an ambient property's
    // value.
    //----------------------------------------------------------------
	virtual HRESULT fpAtlGetAmbientDisplayName(BSTR& bstrDisplayName)
    {
      return m_pT->GetAmbientDisplayName(bstrDisplayName);
    }

    //----------------------------------------------------------------
    // This virtual function is called to get the InPlaceFrame
    // interface ptr.  This must be released by the caller.
    //----------------------------------------------------------------
    virtual BOOL fpAtlGetInPlaceFramePtr(LPOLEINPLACEFRAME *ppInPlaceFrame)
    {
	  OLEINPLACEFRAMEINFO frameInfo;
	  RECT rcPos, rcClip;
	  CComPtr<IOleInPlaceUIWindow> spInPlaceUIWindow;
	  frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
	  HWND hwndParent;
      BOOL fRet = FALSE;
	  if (m_spInPlaceSite && 
          m_spInPlaceSite->GetWindow(&hwndParent) == S_OK)
	  {
		if (m_spInPlaceSite->GetWindowContext(ppInPlaceFrame,
			&spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo) == S_OK &&
	        *ppInPlaceFrame != NULL)
          fRet = TRUE;
      }
      return fRet;
    }

    //----------------------------------------------------------------
    // This virtual function is called before putting-up a dialog
    // box (like a property designer).
    //----------------------------------------------------------------
    virtual void fpAtlPreModalDialog(HWND hWndParent = NULL)
    {
	  CComPtr<IOleInPlaceFrame> spInPlaceFrame;

	  if (fpAtlGetInPlaceFramePtr(&spInPlaceFrame))
	    spInPlaceFrame->EnableModeless(FALSE);
    }

    //----------------------------------------------------------------
    // This virtual function is called after putting-up a dialog
    // box (like a property designer).
    //----------------------------------------------------------------
    virtual void fpAtlPostModalDialog(HWND hWndParent = NULL)
    {
	  CComPtr<IOleInPlaceFrame> spInPlaceFrame;

	  if (fpAtlGetInPlaceFramePtr(&spInPlaceFrame))
	    spInPlaceFrame->EnableModeless(TRUE);
    }

    //----------------------------------------------------------------
    // This virtual function is called to invalidate the control's
    // painting rect.
    //----------------------------------------------------------------
    virtual HRESULT fpAtlInvalidateControl(LPCRECT lpRect = NULL, BOOL bErase = TRUE)
    {
	  if (m_bInPlaceActive)
	  {
		// Active
		if (m_hWndCD != NULL)
			return ::InvalidateRect(m_hWndCD, lpRect, bErase); // Window based
		if (m_spInPlaceSite != NULL)
			return m_spInPlaceSite->InvalidateRect(lpRect, bErase); // Windowless
	  }
	  // Inactive
	  m_pT->SendOnViewChange(DVASPECT_CONTENT);
	  return S_OK;
    }

    //----------------------------------------------------------------
    // This virtual function is called to force the control to 
    // update its painting rect.
    //----------------------------------------------------------------
  	virtual BOOL fpAtlUpdateWindow()
    {
	  if (m_bInPlaceActive)
	  {
		// Active
		if (m_hWndCD != NULL)
			return ::UpdateWindow(m_hWndCD); // Window based
		if (m_spInPlaceSite != NULL)
			return (m_spInPlaceSite->InvalidateRect(NULL, TRUE) == S_OK); // Windowless
	  }
	  // Inactive
	  m_pT->SendOnViewChange(DVASPECT_CONTENT);
	  return TRUE;
    }

    //----------------------------------------------------------------
    // This virtual function is called to set the rect of the control
    // within the container.
    //----------------------------------------------------------------
    virtual BOOL fpAtlSetRectInContainer(LPRECT lprect)
    {
		HWND hWnd = fpAtlGetHWnd();

// fix for 9356 -scl
//		if (m_spInPlaceSite != NULL && m_bInPlaceActive)
		if (!fpAtlGetUserMode() && m_spInPlaceSite != NULL && m_bInPlaceActive)
		{	// try to move grab handles
			if (S_OK != m_spInPlaceSite->OnPosRectChange(lprect))
				hWnd = 0;	// cancel moving window
		}
		if (hWnd)
		{	// try to move window
			::SetWindowPos(hWnd, NULL, 
				lprect->left, lprect->top, 
				lprect->right - lprect->left,
				lprect->bottom - lprect->top,
				SWP_NOACTIVATE | SWP_NOZORDER);
		}
		return FALSE;
    }

    //----------------------------------------------------------------
    // This virtual function returns the DISPATCH interface ptr for
    // the control. 
    //----------------------------------------------------------------
    virtual LPDISPATCH fpAtlGetIDispatch(BOOL bAddRef)
    {
      LPDISPATCH lpDispatch = NULL;
      IUnknown* lpUnk = m_pT->GetUnknown();
      if (lpUnk)
      {
        HRESULT h = lpUnk->QueryInterface(IID_IDispatch, (LPVOID *)&lpDispatch);
        // if caller does not want to add a reference, release the ptr.
        if (h == S_OK && lpDispatch && !bAddRef)
          lpDispatch->Release();

        lpUnk->Release(); // release Unknown interface.
      }

      return lpDispatch;
    }

    //----------------------------------------------------------------
    // This virtual function is called to indicate that the view has
    // changed.
    //----------------------------------------------------------------
    virtual HRESULT fpAtlFireViewChange()
    {
      return m_pT->FireViewChange();
    }

    //----------------------------------------------------------------
    // This virtual function is called to indicate that the property
    // value has changed.
    //----------------------------------------------------------------
    virtual HRESULT fpAtlFireOnChanged(DISPID dispID)
    {
      return m_pT->FireOnChanged(dispID);
    }

    //----------------------------------------------------------------
    // This virtual function is called to determine if a property
    // is allowed to change.
    //----------------------------------------------------------------
    virtual HRESULT fpAtlFireOnRequestEdit(DISPID dispID)
    {
      return m_pT->FireOnRequestEdit(dispID);
    }

    //----------------------------------------------------------------
    // This virtual function is called to indicate that the Mnemonics
    // have changed.
    //----------------------------------------------------------------
    virtual HRESULT fpAtlControlInfoChanged()
    {
	  CComQIPtr <IOleControlSite, &IID_IOleControlSite> spSite(m_spClientSite);
      _ASSERT(spSite);  // if we don't have a site, we can't call OnControlInfoChanged().
	  if (!spSite)
        return S_OK;
	  return spSite->OnControlInfoChanged();
    }

    //----------------------------------------------------------------
    // This virtual function is called to inform the container that
    // the control has the focus.
    //----------------------------------------------------------------
    virtual HRESULT fpAtlOnSetFocus()
    {
      BOOL fDummy;
	  return m_pT->OnSetFocus(0, 0, 0, fDummy);
    }

    //----------------------------------------------------------------
    // This virtual function is called to inform the container that
    // the control has lost the focus.
    //----------------------------------------------------------------
    virtual HRESULT fpAtlOnKillFocus()
    {
      BOOL fDummy;
	  return m_pT->OnKillFocus(0, 0, 0, fDummy);
    }


    //----------------------------------------------------------------
    // This gets the Mnemonic info for the control. To specify the 
    // mnemonics for your control, override fpOnGetMnemonicsInfo(), 
    // see CfpAtlBase class.
    //----------------------------------------------------------------
	STDMETHOD(GetControlInfo)(LPCONTROLINFO lpi)
    {
      return m_pT->fpAtlGetControlInfo(lpi);
    }

#define FPATL_LOADINGDLL_ERROR_MSG  \
_T("Error loading Dll.  Please install this Dll on the system.")
    //----------------------------------------------------------------
    // Load DLL which contains functions to convert the VBX control to 
    // the OCX control.
    //----------------------------------------------------------------
    BOOL fpAtlLoadVbxCnvtDll()
    {
      // if dll is not loaded, try to load it
      if (!m_hVbxConvertDll)
      {
        LPCTSTR lptstrDllName = m_pT->fpGetVbxCnvtDllName();
        if (lptstrDllName == NULL)
        {
          _ASSERT(0);  // use FP_DECLARE_VBXCONVERT_DLLNAME() macro
          return FALSE;
        }
        m_hVbxConvertDll = LoadLibrary(lptstrDllName);
        if ((int)m_hVbxConvertDll <= HINSTANCE_ERROR)
        {
          // Pop-up dialog: "Error loading DLL."
          ::MessageBox(NULL, FPATL_LOADINGDLL_ERROR_MSG, lptstrDllName, MB_OK);
          return FALSE;
        }
      }
      return TRUE;  // already loaded, or loaded successfully
    }


    //----------------------------------------------------------------
    // This interface function is called to load the control with
    // the property values stored in the passed IStream.
    //----------------------------------------------------------------
	STDMETHOD(Load)(LPSTREAM pStm)
    {
      DWORD dwVersion = m_pT->fpGetCtlVersion();
      SIZE  sizeExtent = m_sizeExtent;

	  ATLTRACE(_T("fpAtlCtl (PersistStreamInit)::Load\n"));

      // Load (read) control version and x-y extents.
      m_pT->fpAtlStream_LoadVerAndExtent(pStm, &dwVersion, &sizeExtent); // ignore result;

      // If extent is different, change control.
      if (memcmp(&sizeExtent, &m_sizeExtent, sizeof(sizeExtent)) != 0)
        m_pT->SetExtent(DVASPECT_CONTENT, &sizeExtent);

      // The idea is to copy the prop values from the Stream passed
      // to the stream created (which we will use when we have a HWND).
      // NOTE: The new stream's current position is at the first
      // property (skipped past Version & Extent).
      return m_pT->fpAtlLoadTofpStream((CComControlBase *)m_pT,
        (CComDispatchDriver *)m_pT, pStm, NULL, NULL,
        T::GetPropertyMap());

      return S_OK;
    }

    //----------------------------------------------------------------
    // This interface function saves the property values to the 
    // passed IStream.  These values are save as Dispid/value pairs, 
    // which is a format invented by FarPoint.
    //----------------------------------------------------------------
	STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty)
	{
      HRESULT hr;
	  ATL_PROPMAP_ENTRY* pMap = T::GetPropertyMap();
        
	  if (m_pStmLoad != NULL)
	  {
		m_pStmLoad->Release();
		m_pStmLoad = NULL;
	  }

	  ATLTRACE(_T("fpAtlCtl (PersistStreamInit)::Save\n"));
      _ASSERTE(pMap != NULL);

      // If we are converting from a VBX project.  Copy properties
      // from "m_pstmVbxLoad" to Save stream.  This is because we are
      // not able to set the property values in the control (for some
      // reason, the control in this state will GPF if we set a prop).
      if (m_pstmVbxLoad && m_pT->fpAtlLoadVbxCnvtDll())
      {
        FPATL_VBXSAVESTREAM_FUNCTYPE savefunc =
          (FPATL_VBXSAVESTREAM_FUNCTYPE)GetProcAddress(
            m_hVbxConvertDll, FPATL_VBXSAVESTREAM_FUNC);
        _ASSERT(savefunc);
        if (!savefunc)
          return S_FALSE;
        hr = (*savefunc)(&m_pstmVbxLoad, pStm,
           fClearDirty, m_pT->fpGetCtlVersion(), &m_sizeExtent,
           m_pT);
	    if (hr == S_OK && fClearDirty)
		  m_bRequiresSave = FALSE;
        return hr;  // return if saving from VBX
      }
      // Save prop values from control to IStream.
      hr = m_pT->fpAtlStream_Save((CComControlBase *)this,
        (CComDispatchDriver *)this, pStm, fClearDirty, pMap,
        m_pT->fpGetCtlVersion(), &m_sizeExtent);

      return hr;
	}

    //----------------------------------------------------------------
    // This function is called right after the control's window has
    // been created. The property values stored in "m_pStmLoad" are
    // loaded into the control (now that we have a window handle).
    //----------------------------------------------------------------
	virtual HRESULT fpAtlLoad(void)
    {
      HRESULT h = S_OK;
      HGLOBAL hBlob = NULL;
      int iPropId = -1;

      // if we have a Stream as a result of a PropertyBag_Load,
      // fetch prop values from that stream.
      if (m_pStmLoad)
      {
		ATL_PROPMAP_ENTRY* pMap = T::GetPropertyMap();
		_ASSERTE(pMap != NULL);
        h = m_pT->fpAtlLoadFromfpStream((CComControlBase *)m_pT,
          (CComDispatchDriver *)m_pT, pMap);
		m_pStmLoad->Release();
		m_pStmLoad = NULL;
      }
 	  return S_OK;
    }

    //----------------------------------------------------------------
    // This function is called to determine if the control has 
    // blob data.  This will look thru the properties in the 
    // "pinfo" list. The BETTER method (faster) is for the control
    // to override this function and return the required information.
    //
    // NOTE: If YOUR implementation is returning TRUE, beware that
    //       "lptszRetPropName" is optional and may be NULL.
    //----------------------------------------------------------------
    virtual BOOL fpAtlHasBlobData(int *lpiRetPropId, 
      LPCTSTR *lptszRetPropName)
    {
      // if static variable is uninitialized, scan PropList looking for
      // a property with BLOB data.
      if (m_iBlobProp == FPATL_BLOBPROP_UNINIT)
      {
	    int i = 0;
        m_iBlobProp = FPATL_BLOBPROP_NONE;
	    if (!m_PropList || !m_PropCt)
          return FALSE;
    
        while (i < m_PropCt && m_iBlobProp == FPATL_BLOBPROP_NONE)
        {
          if (m_PropList[i].wEnvironment & FP_BLOB)
          {
            m_iBlobProp = i;  // blob prop found
            m_lptszBlobPropName = 
              (LPCTSTR)m_PropList[i].pPropInfo->npszName;
          }
          else
            ++i;              // blob prop not found, try next prop.
        }
      }
    
      // If found, return information.
      if (m_iBlobProp != FPATL_BLOBPROP_NONE)
      {
        *lpiRetPropId = m_iBlobProp;
        if (lptszRetPropName)
          *lptszRetPropName = m_lptszBlobPropName;
        return TRUE;
      }
    
      return FALSE;
    }

    //----------------------------------------------------------------
    // This function queries the ambient property to see if 
    // message reflection is supported.
    //----------------------------------------------------------------
    virtual BOOL fpAtlGetMessageReflect()
    {
	  VARIANT var;

      if (m_pT->GetAmbientProperty( DISPID_AMBIENT_MESSAGEREFLECT, var)
           == S_OK && var.vt == VT_BOOL && var.boolVal)
        return TRUE;
      else
        return FALSE;
    }

    //----------------------------------------------------------------
    // This function is supplied as backward support of MFC function.
    // You may override this function.
    //----------------------------------------------------------------
    virtual BOOL fpAtlOnProperties(LPMSG lpMsg, HWND hWndParent, LPCRECT rc)
    { 
      _ASSERT(0);
      return TRUE;
    }

    //----------------------------------------------------------------
    // This returns NULL as parent since we don't usually know the
    // true parent of the control in various environments.
    //----------------------------------------------------------------
    virtual HWND fpAtlGetParent()
    {
      return NULL;
    }

    //----------------------------------------------------------------
    // This function can be used to get the value of a property.
    //----------------------------------------------------------------
    virtual HRESULT fpAtlGetProperty(DISPID dwDispID, VARIANT* pVar)
    {
      return CComDispatchDriver::GetProperty(m_pT, dwDispID, pVar);
    }


    //----------------------------------------------------------------
    // This virtual function is used to get the control's window handle.
    //----------------------------------------------------------------
    virtual HWND fpAtlGetHWnd()
      { return CTLHWND; }

    //----------------------------------------------------------------
    // This virtual function allows the control the first chance
    // to process the Accelerator key.  To get control, YOU MUST
    // OVERRIDE this function.
    //----------------------------------------------------------------
    virtual BOOL fpAtlPreTranslateMessage(LPMSG lpMsg)
      { return FALSE; }

    //----------------------------------------------------------------
    // Interface function. This give the control the chance to process
    // the keystroke first.
    //----------------------------------------------------------------
	STDMETHOD(TranslateAccelerator)(LPMSG lpmsg)
    {
	  ATLTRACE(_T("CfpAtlCtl::TranslateAccelerator\n"));

      // Give the control the first chance.
      if (m_pT->fpAtlPreTranslateMessage(lpmsg))
	    return S_OK;
      else
      {
        HRESULT hr = S_FALSE;
// fix for SCS9110 -scl
// use m_spControlSite instead of local CComQIPtr
//	    CComQIPtr <IOleControlSite, &IID_IOleControlSite> spSite(m_spClientSite);

	    // Give the site a chance.
//	    if (spSite != NULL)
		if (m_spControlSite)
//          hr = spSite->TranslateAccelerator(lpmsg,
//			  (DWORD)fpGetOleShiftState());
          hr = m_spControlSite->TranslateAccelerator(lpmsg,
			  (DWORD)fpGetOleShiftState());
	    return hr;
      }
	}

    //----------------------------------------------------------------
    // Note: We are overriding the default function because the default
    //       is calling UIDeactivate() when fActivate is FALSE.  This
    //       screws-up VB when MDI forms are involved.  Simply return S_OK.
    // SCP 3/1/00.
    //----------------------------------------------------------------
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate)
    {
      return S_OK;
    }

    //----------------------------------------------------------------
    // Call this function when the control get the focus.
    //----------------------------------------------------------------
	LRESULT OnSetFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComControlBase::OnSetFocus(nMsg, wParam, lParam, bHandled);
        bHandled = FALSE;
		return 0;
	}

    //----------------------------------------------------------------
    // Call this function when the control loses the focus.
    //----------------------------------------------------------------
	LRESULT OnKillFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComControlBase::OnKillFocus(nMsg, wParam, lParam, bHandled);
        bHandled = FALSE;
		return 0;
	}

#ifndef VC9
// fix for bugs 8380, 9095 -scl
// ->
	virtual WNDPROC GetWindowProc()
	{
		return WindowProc;
	}

  static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
// code copied from ATLWIN.H and modified 
//	  CWindowImplBaseT< TBase, TWinTraits >* pThis = (CWindowImplBaseT< TBase, TWinTraits >*)hWnd;
	  CWindowImpl< T >* pThis = (CWindowImpl< T >*)hWnd;
    HWND hWndSave = pThis->m_hWnd;

    // set a ptr to this message and save the old value
	  MSG msg = { pThis->m_hWnd, uMsg, wParam, lParam, 0, { 0, 0 } };
	  const MSG* pOldMsg = pThis->m_pCurrentMsg;
	  pThis->m_pCurrentMsg = &msg;
	  // pass to the message map to process
	  LRESULT lRes;

	  BOOL bRet = pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
	  // restore saved value for the current message
// *** fix inserted here ***
    if( !::IsWindow(hWndSave) )
      return lRes;
// *** fix inserted here ***
    ATLASSERT(pThis->m_pCurrentMsg == &msg);
	  pThis->m_pCurrentMsg = pOldMsg;
	  // do the default processing if message was not handled
	  if(!bRet)
	  {
		  if(uMsg != WM_NCDESTROY)
			  lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
		  else
		  {
			  // unsubclass, if needed
			  LONG pfnWndProc = ::GetWindowLong(pThis->m_hWnd, GWL_WNDPROC);
			  lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
			  if(pThis->m_pfnSuperWindowProc != ::DefWindowProc && ::GetWindowLong(pThis->m_hWnd, GWL_WNDPROC) == pfnWndProc)
				  ::SetWindowLong(pThis->m_hWnd, GWL_WNDPROC, (LONG)pThis->m_pfnSuperWindowProc);
			  // clear out window handle
			  HWND hWnd = pThis->m_hWnd;
			  pThis->m_hWnd = NULL;
			  // clean up after window is destroyed
			  pThis->OnFinalMessage(hWnd);
		  }
	  }
	  return lRes;
  }
// <-
// fix for bugs 8380, 9095 -scl
#endif

    //----------------------------------------------------------------
    // This function is called when the control receives a window
    // message (normally, it's fpAtlProcessSimpleFrameMsg() or
    // ProcessWindowMessage() who calls this function).  This function 
    // is usually overridden by your control via the 
    // "FPATL_BEGIN_MSG_MAP()" macro.
    //----------------------------------------------------------------
    virtual BOOL fpAtlProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0)
    {
      _ASSERT(FALSE);
      return FALSE;
    }

//	WEB921350539984 -scl
//	The simple frame site implementation needs to ignore certain messages.
//	This code is copied and adapted from COleControl::IgnoreWindowMessage().
	BOOL IgnoreWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam,
		LRESULT* plResult)
	{
		BOOL bUIDead;

		GetAmbientUIDead(bUIDead);

		if (!bUIDead)
			return FALSE;

		switch (msg)
		{
		case WM_NCHITTEST:
			*plResult = HTNOWHERE;
			return TRUE;

		case WM_SETCURSOR:
			*plResult = ::SendMessage(::GetParent(m_hWnd), msg, wParam, lParam);
			return TRUE;
		}

		if ((msg >= WM_KEYFIRST) && (msg <= WM_KEYLAST))
		{
			*plResult = 0;
			return TRUE;
		}

		return FALSE;
	}


    //----------------------------------------------------------------
    // This function is called when the control receives a window
    // message (normally, it's ProcessWindowMessage() who calls this 
    // function).  This function will process the message dependent
    // on the surrounding situation (simple frame/not simple frame).
    //----------------------------------------------------------------
	virtual BOOL fpAtlProcessSimpleFrameMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0) \
	{
      BOOL fRet;

      DWORD dwCookie;

	  if (!m_pSimpleFrameSite)
        fRet = m_pT->fpAtlProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);
      else
      {
// we need to check the return val from PreMessageFilter()
// and call the defwndproc if it returns nether S_OK nor S_FALSE -scl
		HRESULT hr;
		hr = m_pSimpleFrameSite->PreMessageFilter(hWnd, uMsg, wParam, lParam,
             &lResult, &dwCookie);
		if (hr == S_OK || hr == E_NOTIMPL || hr == 0x80000001)
        {
		  //if (uMsg == WM_NCCREATE)
		  //	m_pT->DefWindowProc(uMsg, wParam, lParam);
		  if (!IgnoreWindowMessage(uMsg, wParam, lParam, &lResult))
		  {
			  fRet = m_pT->fpAtlProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);
			  if (!fRet)      // If not processed, call default ourself and 
			  {               // return that msg was processed.
				fRet = TRUE;
          		lResult = m_pT->DefWindowProc(uMsg, wParam, lParam);
			  }
		  }
		  if (m_pSimpleFrameSite)
			m_pSimpleFrameSite->PostMessageFilter(m_hWnd, uMsg, wParam, lParam,
            &lResult, dwCookie);
        }
		//else if (hr != S_FALSE)
		//	lResult = m_pT->DefWindowProc(uMsg, wParam, lParam);
      }
      return fRet;
    }

    //----------------------------------------------------------------
    // Interface function.  Called when the client site has been 
    // set.  We override it so we can initialize our member variable,
    // m_pSimpleFrameSite.
    //----------------------------------------------------------------
	STDMETHOD(SetClientSite)(IOleClientSite *pClientSite)
	{
        HRESULT hr;
      	LPVOID pInterface;

		ATLTRACE(_T("CfpAtlCtl::SetClientSite\n"));
		hr = m_pT->IOleObject_SetClientSite(pClientSite);

		// initialize pointer to control site
		if (pClientSite)
			pClientSite->QueryInterface(IID_IOleControlSite, (void**)&m_spControlSite);
		else
			m_spControlSite = NULL;

    	// Initialize pointer to simple frame site
	    if (pClientSite == NULL || !m_fSimpleFrame ||
		     pClientSite->QueryInterface(IID_ISimpleFrameSite, &pInterface) != S_OK)
		  pInterface = NULL;

        if (m_pSimpleFrameSite)
	      m_pSimpleFrameSite->Release();
	    m_pSimpleFrameSite = (LPSIMPLEFRAMESITE)pInterface;

        // Indicate whether client site has been set.
        m_fClientSiteSet = (pClientSite != NULL);

        return hr;
	}


    //----------------------------------------------------------------
    // Interface function (IPerPropertyBrowsing)
    //----------------------------------------------------------------
	STDMETHOD(GetDisplayString)(DISPID dispID, BSTR *pBstr)
	{
      HRESULT hResult;
	  ATLTRACE(_T("CfpAtlCtl::GetDisplayString\n"));
      // Call "helper" function in CfpCtlBase
      hResult = m_pT->fpAtlGetDisplayString(dispID, pBstr);
      // if helper func returned E_NOTIMPL, call default
      if (hResult == E_NOTIMPL)
        hResult = IPerPropertyBrowsingImpl<T>::GetDisplayString(dispID, pBstr);
      return hResult;
    }

    //----------------------------------------------------------------
    // Interface function (IPerPropertyBrowsing)
    //----------------------------------------------------------------
    STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pCaStringsOut,
      CADWORD *pCaCookiesOut)
    {
	  ATLTRACE(_T("CfpAtlCtl::GetPredefinedStrings\n"));
      // Call "helper" function in CfpCtlBase
      return m_pT->fpAtlGetPredefinedStrings(dispID, pCaStringsOut, pCaCookiesOut);
    }

    //----------------------------------------------------------------
    // Interface function (IPerPropertyBrowsing)
    //----------------------------------------------------------------
	STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT* pVarOut)
	{
	  ATLTRACE(_T("CfpAtlCtl::GetPredefinedValue\n"));
      // Call "helper" function in CfpCtlBase
      return m_pT->fpAtlGetPredefinedValue(dispID, dwCookie, pVarOut);
	}

    //----------------------------------------------------------------
    // Interface function (IPerPropertyBrowsing)
    //----------------------------------------------------------------
	STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pClsid)
    {
		ATLTRACE(_T("CfpAtlCtl::MapPropertyToPage\n"));
		ATL_PROPMAP_ENTRY* pMap = T::GetPropertyMap();
		_ASSERTE(pMap != NULL);
        return m_pT->fpAtlMapPropertyToPage(dispID, pClsid, pMap);

    }

    //----------------------------------------------------------------
    // This function is called to do the default processing of
    // a window's message.
    //----------------------------------------------------------------
	virtual BOOL fpAtlDefaultProcessWndMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0)
	{
	  BOOL bHandled = TRUE;
	  switch(dwMsgMapID)
	  { 
		case 0:
          switch (uMsg)
          {
            // Required for OCX painting
	        FPATL_DEFAULT_HANDLER(WM_PAINT, OnPaint)

            // Required for OCX set/kill focus
	        FPATL_DEFAULT_HANDLER(WM_SETFOCUS, OnSetFocus)
	        FPATL_DEFAULT_HANDLER(WM_KILLFOCUS, OnKillFocus)

            // Used for default processing of "stock" events.
            FPATL_DEFAULT_HANDLER(WM_MOUSEMOVE    , OnMouseMove)    
            FPATL_DEFAULT_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
            FPATL_DEFAULT_HANDLER(WM_LBUTTONDOWN  , OnLButtonDown)  
            FPATL_DEFAULT_HANDLER(WM_LBUTTONUP    , OnLButtonUp)    
            FPATL_DEFAULT_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
            FPATL_DEFAULT_HANDLER(WM_MBUTTONDOWN  , OnMButtonDown)  
            FPATL_DEFAULT_HANDLER(WM_MBUTTONUP    , OnMButtonUp)    
            FPATL_DEFAULT_HANDLER(WM_MBUTTONDBLCLK, OnMButtonDblClk)
            FPATL_DEFAULT_HANDLER(WM_RBUTTONDOWN  , OnRButtonDown)  
            FPATL_DEFAULT_HANDLER(WM_RBUTTONUP    , OnRButtonUp)    
            FPATL_DEFAULT_HANDLER(WM_RBUTTONDBLCLK, OnRButtonDblClk)
            FPATL_DEFAULT_HANDLER(WM_KEYDOWN      , m_pT->OnKeyDown)      
            FPATL_DEFAULT_HANDLER(WM_KEYUP        , OnKeyUp)        
            FPATL_DEFAULT_HANDLER(WM_CHAR         , OnChar)         
            FPATL_DEFAULT_HANDLER(WM_SYSKEYDOWN   , OnSysKeyDown)   
            FPATL_DEFAULT_HANDLER(WM_SYSKEYUP     , OnSysKeyUp)     
          }
          break;
		default:
		  ATLTRACE(_T("Invalid message map ID (%i)\n"), dwMsgMapID);
		  _ASSERTE(FALSE);
		  break;
	  }
	  return FALSE;
    }

    //----------------------------------------------------------------
    // Interface function. This function loads the property values
    // from the PropertyBag to the control.  The property values
    // are actually copied to a temporary IStream which is loaded
    // after the control gets a window handle.
    //----------------------------------------------------------------
	STDMETHOD(Load)(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
	{
      DWORD dwVersion = m_pT->fpGetCtlVersion();
      SIZE  sizeExtent = m_sizeExtent;
	  ATL_PROPMAP_ENTRY* pMap = T::GetPropertyMap();
	  ATLTRACE(_T("fpAtlCtl (PropertyBag)::Load\n"));
	  _ASSERTE(pMap != NULL);
      if (!pMap)
        return S_OK;

      // Load (read) control version and x-y extents.
      m_pT->fpAtlPropBag_LoadVerAndExtent(pPropBag, pErrorLog, 
          &dwVersion, &sizeExtent);  // ignore result;

      // If extent is different, change control.
      if (memcmp(&sizeExtent, &m_sizeExtent, sizeof(sizeExtent)) != 0)
        m_pT->SetExtent(DVASPECT_CONTENT, &sizeExtent);

      // The idea is to copy the prop values from the property bag
      // to the stream created (which we will use when we have a HWND).
      // NOTE: The new stream's current position is at the first
      // property (skipped past Version & Extent).
      return m_pT->fpAtlLoadTofpStream((CComControlBase *)m_pT,
        (CComDispatchDriver *)m_pT, NULL, pPropBag, pErrorLog,
        pMap);
	}

    //----------------------------------------------------------------
    // Interface function. This function saves the property values
    // from the control to the PropertyBag. First it saves the
    // version and extents (width & height), then the properties and
    // finally, the blob data (if any).
    //----------------------------------------------------------------
	STDMETHOD(Save)(LPPROPERTYBAG pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
	{
      HRESULT hr;

	  ATLTRACE(_T("fpAtlCtl (PropertyBag)::Save\n"));
	  ATL_PROPMAP_ENTRY* pMap = T::GetPropertyMap();
      _ASSERTE(pMap != NULL);
//	  return IPersistPropertyBag_Save(pPropBag, fClearDirty, fSaveAllProperties, pMap);
      hr = m_pT->fpAtlPropBag_Save((CComControlBase *)this,
        (CComDispatchDriver *)this, pPropBag, fClearDirty, 
        fSaveAllProperties, pMap,
        m_pT->fpGetCtlVersion(), &m_sizeExtent);

      return hr;
	}

    //----------------------------------------------------------------
    // Interface function. This function returns the list of custom
    // verbs specified by the control (see FPATL_BEGIN_VERB_MAP() 
    // macro).  The list is build by fpAtlEnumOleVerbs().
    //----------------------------------------------------------------
	STDMETHOD(EnumVerbs)(IEnumOLEVERB **ppEnumOleVerb)
	{
      if (m_pT->fpAtlEnumOleVerbs(m_pT->fpAtlGetVerbMap(), ppEnumOleVerb))
        return S_OK;
      return IOleObjectImpl<T>::EnumVerbs(ppEnumOleVerb);
	}

    //----------------------------------------------------------------
    // Interface function. This function is called to execute a 
    // "verb" action.  The function first checks to see if the "verb id"
    // indicates a custom verb, if so, the function for that verb is
    // called.  If not a custom verb, the default ATL function is called.
    //----------------------------------------------------------------
	STDMETHOD(DoVerb)(LONG iVerb, LPMSG lpMsg, IOleClientSite* pActiveSite,
      LONG lindex, HWND hWndParent, LPCRECT lprcPosRect)
    {
      // if custom verb, call fpAtl function to invoke, if FALSE is
      // returned, call default function.
      if (iVerb >= FPATL_FIRST_CUSTOM_VERB)
        if (m_pT->fpAtlDoVerb(iVerb, lpMsg, hWndParent, lprcPosRect,
             m_pT->fpAtlGetVerbMap()))
          return S_OK;
      return IOleObjectImpl<T>::DoVerb(iVerb, lpMsg, pActiveSite,
        lindex, hWndParent, lprcPosRect);
    }


    //----------------------------------------------------------------
    // This function is called when a WM_PAINT message is received.
    // If a HDC is passed in wParam, it's copied to the "di" structure,
    // otherwise the HDC is from BeginPaint(). After filling-in the
    // "di" structure, OnDrawAdvanced() is called to paint the
    // control.  NOTE: OnDrawAdvanced() will call OnDraw() to complete
    // the painting.
    //----------------------------------------------------------------
	LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& lResult)
    {
      RECT rc;
      PAINTSTRUCT ps;
      HDC hdc = (wParam ? (HDC)wParam : ::BeginPaint(m_hWndCD, &ps));

      if (hdc == NULL)
    	return 0;
      ::GetClientRect(m_hWndCD, &rc);
    
      ATL_DRAWINFO di;
	  memset(&di, 0, sizeof(di));
      di.cbSize = sizeof(di);
      di.dwDrawAspect = DVASPECT_CONTENT;
      di.lindex = -1;
      di.hdcDraw = hdc;

		::GetClientRect(m_hWndCD, &rc);
		di.prcBounds = (LPCRECTL)&rc;

		/* RFW - 5/24/05 - 16240
		if (wParam)
		{
			::GetClientRect(m_hWndCD, &rc);
			di.prcBounds = (LPCRECTL)&rc;
		}
		else
			di.prcBounds = (LPCRECTL)&ps.rcPaint;
		*/

      OnDrawAdvanced(di);

      // if we did not have a HDC in wParam, then we created one by
      // calling BeginPaint(), now call EndPaint().
      if (!wParam)
        ::EndPaint(m_hWndCD, &ps);

      return 0;
    }

    //----------------------------------------------------------------
    // Interface function.  This function is called to exchange data.
    // The only case we "care" about is when we are converting from
    // a VBX project.  In that case, we want to convert the
    // properties from PropSet to IStream.  
    //
    // NOTE: The function to do the conversion is in a seperate DLL 
    // (usually the property designer) because it is only converted 
    // at design time.  The property values are "converted" and
    // stored in the IStream, m_pstmVbxLoad, because the control (in 
    // this state) is not able to set it's own properties (I don't
    // know why -SP).
    //----------------------------------------------------------------
	STDMETHOD(SetData)(FORMATETC* pformatetc, STGMEDIUM* lpStgMedium, BOOL fRelease)
	{
	  static CLIPFORMAT _cfConvertVBX = 0;
      HRESULT hr = E_FAIL;

	  ATLTRACE(_T("CfpAtlCtl::SetData\n"));

      // Clipboard format is not registered, register it.
	  if (_cfConvertVBX == 0)
      {
	    TCHAR pszClsid[40];
        REFCLSID clsid = CLSID_ConvertVBX;
	    wsprintf(pszClsid,
		  _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
		  clsid.Data1, clsid.Data2, clsid.Data3,
		  clsid.Data4[0], clsid.Data4[1], clsid.Data4[2], clsid.Data4[3],
		  clsid.Data4[4], clsid.Data4[5], clsid.Data4[6], clsid.Data4[7]);
	    _cfConvertVBX = (CLIPFORMAT)RegisterClipboardFormat(pszClsid);
      }

      // If the passed format is "ConvertVBX", then do the conversion
      // of the property values.
      if (pformatetc->cfFormat == _cfConvertVBX)
	  {
	    IStorage *lpStorage = NULL;
	    IStream  *lpStream = NULL;

	    switch (lpStgMedium->tymed)
	    {
	      case TYMED_ISTORAGE:
			lpStorage = lpStgMedium->pstg;
			_ASSERT(lpStorage);
            _ASSERT(0);  // Add support (do something)!!!
		    break;

	      case TYMED_ISTREAM:  // It's assumed that this is the only case.
		    lpStream = lpStgMedium->pstm;
			_ASSERT(lpStream);
            if (m_pT->fpAtlLoadVbxCnvtDll())  // load DLL
            {                                 // get address of function
              FPATL_LOADPROPSET_FUNCTYPE loadfunc = 
                (FPATL_LOADPROPSET_FUNCTYPE)GetProcAddress(
                  m_hVbxConvertDll, FPATL_LOADPROPSET_FUNC);
              _ASSERT(loadfunc);
              if (loadfunc)                   // if valid address, call func.
                hr = (*loadfunc)(&m_pstmVbxLoad, lpStream,
                  T::GetPropertyMap(), m_pT);
            }
		    break;

	      default:
		    ATLTRACE("Propset only supported for stream or storage.\n");
	    }
	  }

	  if (hr == S_OK && fRelease)
	    ReleaseStgMedium(lpStgMedium);

      return hr;
	}

  HRESULT InPlaceActivate(LONG iVerb, const RECT* prcPosRect)
  {
   	HRESULT hr;

  	if (m_spClientSite == NULL)
	  	return S_OK;

  	CComPtr<IOleInPlaceObject> pIPO;
	  ControlQueryInterface(IID_IOleInPlaceObject, (void**)&pIPO);
	  _ASSERTE(pIPO != NULL);
	  if (prcPosRect != NULL)
  		pIPO->SetObjectRects(prcPosRect, prcPosRect);

	  if (!m_bNegotiatedWnd)
	  {
  		if (!m_bWindowOnly)
	  		// Try for windowless site
		  	hr = m_spClientSite->QueryInterface(IID_IOleInPlaceSiteWindowless, (void **)&m_spInPlaceSite);

  		if (m_spInPlaceSite)
	  	{
		  	m_bInPlaceSiteEx = TRUE;
			  m_bWndLess = SUCCEEDED(m_spInPlaceSite->CanWindowlessActivate());
  			m_bWasOnceWindowless = TRUE;
	  	}
		  else
		  {
  			m_spClientSite->QueryInterface(IID_IOleInPlaceSiteEx, (void **)&m_spInPlaceSite);
	  		if (m_spInPlaceSite)
		  		m_bInPlaceSiteEx = TRUE;
			  else
  				hr = m_spClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_spInPlaceSite);
	  	}
	  }

	  _ASSERTE(m_spInPlaceSite);
  	if (!m_spInPlaceSite)
	  	return E_FAIL;
  
	  m_bNegotiatedWnd = TRUE;

  	if (!m_bInPlaceActive)
	  {

		  BOOL bNoRedraw = FALSE;
  		if (m_bWndLess)
	  		m_spInPlaceSite->OnInPlaceActivateEx(&bNoRedraw, ACTIVATE_WINDOWLESS);
		  else
  		{
	  		if (m_bInPlaceSiteEx)
		  		m_spInPlaceSite->OnInPlaceActivateEx(&bNoRedraw, 0);
			  else
  			{
	  			HRESULT hr = m_spInPlaceSite->CanInPlaceActivate();
		  		if (FAILED(hr))
			  		return hr;
				  m_spInPlaceSite->OnInPlaceActivate();
  			}
	  	}
  	}

	  m_bInPlaceActive = TRUE;

  	// get location in the parent window,
	  // as well as some information about the parent
	  //
	  OLEINPLACEFRAMEINFO frameInfo;
	  RECT rcPos, rcClip;
  	CComPtr<IOleInPlaceFrame> spInPlaceFrame;
	  CComPtr<IOleInPlaceUIWindow> spInPlaceUIWindow;
	  frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
	  HWND hwndParent;
	  if (m_spInPlaceSite->GetWindow(&hwndParent) == S_OK)
	  {
		  m_spInPlaceSite->GetWindowContext(&spInPlaceFrame,
  			&spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);

	  	if (!m_bWndLess)
		  {
  			if (m_hWndCD)
	  		{
          if (::GetParent(m_hWndCD) != hwndParent)
            ::SetParent(m_hWndCD, hwndParent);        

		  		::ShowWindow(m_hWndCD, SW_SHOW);
				//::SetFocus(m_hWndCD);  //km132
  			}
	  		else
		  	{
			  	HWND h = CreateControlWindow(hwndParent, rcPos);
				  _ASSERTE(h == m_hWndCD);
			  }
  		}
  
	  	pIPO->SetObjectRects(&rcPos, &rcClip);
  	}

	  CComPtr<IOleInPlaceActiveObject> spActiveObject;
  	ControlQueryInterface(IID_IOleInPlaceActiveObject, (void**)&spActiveObject);

	  // Gone active by now, take care of UIACTIVATE
  	if (DoesVerbUIActivate(iVerb))
	  {
		  if (!m_bUIActive)
  		{
	  		m_bUIActive = TRUE;
		  	hr = m_spInPlaceSite->OnUIActivate();
			  if (FAILED(hr))
				  return hr;

			SetControlFocus(TRUE);

	  		// set ourselves up in the host.
		  	//
			  if (spActiveObject)
  			{
	  			if (spInPlaceFrame)
		  			spInPlaceFrame->SetActiveObject(spActiveObject, NULL);
			  	if (spInPlaceUIWindow)
				  	spInPlaceUIWindow->SetActiveObject(spActiveObject, NULL);
  			}
  
	  		if (spInPlaceFrame)
		  		spInPlaceFrame->SetBorderSpace(NULL);
			  if (spInPlaceUIWindow)
				  spInPlaceUIWindow->SetBorderSpace(NULL);
  		}
	  }

  	m_spClientSite->ShowObject();

	  return S_OK;
  }

  // SCP 3/8/00
  // We are overriding the Invoke() function so that we can trap and
  // change the vartype returned in the variant during GETs.
  // The problem has to do with VB not handling certain vartypes when
  // getting properties on a Object, here an example:
  // ------------------------
  //   Dim obj as Object
  //   set obj = fpText1
  //   if obj.BackColor = 0 then ...
  // ------------------------
  // VB chokes on that last line when the vartype returned is VT_UI4, VB
  // works fine when VT_I4 is returned.  We circumvent the problem with
  // the following function override.
  //
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
    HRESULT hr =
      IDispatchImpl<InterfaceName, piid, plibid, wMajor, wMinor, tihclass>::Invoke(
        dispidMember, riid, lcid, wFlags, pdispparams, pvarResult,
		    pexcepinfo, puArgErr);

    // If getting property value, translate certain variant types.
    if (hr == S_OK && pvarResult && (wFlags & DISPATCH_PROPERTYGET))
    {
      switch (pvarResult->vt)
      {
        case VT_UI1:  pvarResult->vt = VT_I1;  break;
        case VT_UI2:  pvarResult->vt = VT_I2;  break;
        case VT_UI4:  pvarResult->vt = VT_I4;  break;
        case VT_UI8:  pvarResult->vt = VT_I8;  break;
      }
    }
    return hr;
	}


#include "fpatlevt.h"
};


#endif // FPATLCTL_H
