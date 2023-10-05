/****************************************************************************
* FPATL.H  -  Declaration of FarPoint's ATL classes & templates
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
#ifndef FPATL_H
#define FPATL_H

//---------------------------------------------------------------------------
// Macro for declaring COM interface map
//---------------------------------------------------------------------------
#ifdef _ATL_DEBUG_QI
#define FP_END_COM_MAP()   {NULL, 0, 0}};\
	return &_entries[1];}
#else
#define FP_END_COM_MAP()   {NULL, 0, 0}};\
	return _entries;}
#endif // _ATL_DEBUG_QI

#include "fptools.h"
#include "fptstr.h"
#include "fpatlcor.h"   // core classes (Font & Picture)
#include "fppinfo.h"

#ifdef SubclassWindow     // being defined in windowsx.h
#undef SubclassWindow
#endif

// forward declaration
class CfpAtlBase;


//---------------------------------------------------------------------------
//
// Macro definitions
//
//---------------------------------------------------------------------------
#define FPATL_STM_BLOBPROP    (SHORT)-2
#define FPATL_STM_NULLPROP    (SHORT)-1

//---------------------------------------------------------------------------
//
// Function prototypes
//
//---------------------------------------------------------------------------
void fpRectHiMetricToPixel(LPRECT lprc);
short fpGetOleShiftState();
LPOLESTR fpAtlCoTaskCopyString(LPCTSTR psz);
LPOLESTR fpAtlCoTaskCopyOleStr(LPOLESTR psz);
VARTYPE fpAtlTranslateVTtype(VARTYPE vt);


//---------------------------------------------------------------------------
// Typedef's
//---------------------------------------------------------------------------
typedef BOOL (CfpAtlBase::*FPATL_PVERBFN)(LPMSG, HWND, LPCRECT);

struct FPATL_VERB_ENTRY
{
	INT      iVerbId;
    LPOLESTR lpolestrVerbName;
    INT      iResId;
	FPATL_PVERBFN pfn;    // routine to call (or special value)
};

#define FPATL_VERB_RESID_LAST   -1  // marks last element (iResId == -1) 

#define FPATL_FIRST_CUSTOM_VERB  0  // id of first custom verb (iVerbId == 0)

//---------------------------------------------------------------------------
//
// Class declarations
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// class CfpAtlWndClassInfo - implemented in FPATL.CPP
//
// NOTE:  Create only "static" instances of this class. -SCP
//---------------------------------------------------------------------------
class CfpAtlWndClassInfo : public CWndClassInfo
{
public:
  HINSTANCE m_hInst;
  ATOM      m_atomReg;

public:
  CfpAtlWndClassInfo::CfpAtlWndClassInfo(
    WNDCLASSEX wc,
	LPCTSTR lpszOrigName,
	WNDPROC _pWndProc,
	LPCTSTR lpszCursorID,
	BOOL bSystemCursor,
	ATOM atom,
	LPCTSTR szAutoName);
  virtual ~CfpAtlWndClassInfo();
  ATOM Register(WNDPROC* wndProc);
};

//---------------------------------------------------------------------------
// class CfpAtlBase
//---------------------------------------------------------------------------
class ATL_NO_VTABLE CfpAtlBase
{ 
public:

//	CContainedWindow m_subctl;
    LPSTREAM         m_pStmLoad;
    LPSTREAM         m_pStmReLoad;
    LPFP_PROPLIST    m_PropList;
    int              m_PropCt;
// fix for leaking accelerator table in Win95 -scl
	HACCEL			 m_hAccel;
// fix for leaking accelerator table in Win95 -scl
    BOOL  m_fIsLoadingWithHWnd; // set when loading props upon hWnd creation

public:
    // Initialization parameters are for initializing "m_subctl".
    CfpAtlBase::CfpAtlBase(LPTSTR lpszClassName, CMessageMap* pObject,
                           DWORD dwMsgMapID = 0) /*:
        m_subctl(lpszClassName, pObject, dwMsgMapID) */
    {
      m_pStmLoad = NULL;
      m_pStmReLoad = NULL;
	  m_PropList = NULL;
      m_PropCt = 0;
// fix for JAP7586 -scl
	  m_hAccel = NULL;
// fix for JAP7586 -scl
      m_fIsLoadingWithHWnd = FALSE;
    }

    virtual ~CfpAtlBase()
    {
// fix for JAP7586 -scl
	  if (m_hAccel)
		  DestroyAcceleratorTable(m_hAccel);
// fix for JAP7586 -scl
      if (m_pStmLoad)
        m_pStmLoad->Release();
      if (m_pStmReLoad)
        m_pStmReLoad->Release();
    }

    STDMETHOD(fpAtl_putref_Font)(IFontDisp* pFont, DISPID DispId, CfpAtlFont& fpAtlFont);
    STDMETHOD(fpAtl_put_Font)(IFontDisp* pFont, DISPID DispId, CfpAtlFont& pComFont);
    STDMETHOD(fpAtl_get_Font)(IFontDisp** ppFont, CfpAtlFont& pComFont);
    STDMETHOD(fpAtl_ClonePicture)(LPPICTUREDISP pPicture,
      DISPID dispId, LPPICTUREDISP *ppPictClone);

    virtual HRESULT fpAtlGetPredefinedStrings(DISPID dispid, CALPOLESTR *pCaStringsOut, 
      CADWORD *pCaCookiesOut);
    virtual HRESULT fpAtlGetDisplayString(DISPID dispID, BSTR *pBstr);
    virtual HRESULT fpAtlGetPredefinedValue(DISPID dispID, DWORD dwCookie,
      VARIANT* pVarOut);
    virtual HRESULT fpAtlMapPropertyToPage(DISPID dispID, CLSID *pClsid,
      ATL_PROPMAP_ENTRY* pMap);

    virtual BOOL fpAtlSetControlSize(int cx, int cy) = 0;
    virtual void fpAtlBoundPropertyChanged(DISPID dispID) = 0;
    virtual BOOL fpAtlBoundPropertyRequestEdit(DISPID dispID) = 0;
    virtual BOOL fpAtlGetUserMode() = 0;
    virtual void fpAtlSetDirty(BOOL bDirty) = 0;
    virtual void fpAtlSetModifiedFlag(BOOL bDirty = TRUE)  { fpAtlSetDirty(bDirty); }

    virtual long fpAtlFireEvent(USHORT usidEvent, LPVOID lpparams) = 0;
    virtual LPOLECLIENTSITE fpAtlGetClientSite() = 0;
	virtual HRESULT fpAtlGetAmbientProperty(DISPID dispid, VARIANT& var) = 0;
	virtual HRESULT fpAtlGetAmbientDisplayName(BSTR& bstrDisplayName) = 0;
    virtual BOOL fpAtlGetInPlaceFramePtr(LPOLEINPLACEFRAME *ppInPlaceFrame) = 0;
	virtual void fpAtlPreModalDialog(HWND hWndParent = NULL) = 0;
	virtual void fpAtlPostModalDialog(HWND hWndParent = NULL) = 0;
    virtual HRESULT fpAtlInvalidateControl(LPCRECT lpRect = NULL, BOOL bErase = TRUE) = 0;
  	virtual BOOL fpAtlUpdateWindow() = 0;
    virtual BOOL fpAtlSetRectInContainer(LPRECT lprect) = 0;
    virtual LPDISPATCH fpAtlGetIDispatch(BOOL bAddRef) = 0;
    virtual BOOL fpAtlGetMessageReflect() = 0;
    virtual BOOL fpAtlOnProperties(LPMSG lpMsg, HWND hWndParent, LPCRECT rc) = 0;
    virtual HWND fpAtlGetHWnd() = 0;
    virtual HWND fpAtlGetParent() = 0;
    virtual HRESULT fpAtlGetProperty(DISPID dwDispID, VARIANT* pVar) = 0;

    virtual HRESULT fpAtlFireViewChange() = 0;
    virtual HRESULT fpAtlFireOnChanged(DISPID dispID) = 0;
    virtual HRESULT fpAtlFireOnRequestEdit(DISPID dispID) = 0;
    virtual HRESULT fpAtlControlInfoChanged() = 0;
    virtual HRESULT fpAtlOnSetFocus() = 0;
    virtual HRESULT fpAtlOnKillFocus() = 0;

    virtual LPDISPATCH fpAtlGetExtendedControl();

    // This may be overridden by the subclassed control.
    virtual BOOL fpOnGetMnemonicsInfo(int iMaxCt, int *piRetCt,
      WORD *wzKeyList, DWORD *pdwFlags)
     { return FALSE; }

    // This function, by default, calls fpOnGetMnemonicsInfo().
    virtual HRESULT fpAtlGetControlInfo(LPCONTROLINFO lpi);

    virtual BOOL fpAtlEnumOleVerbs(const FPATL_VERB_ENTRY *lpVerbEntries,
      LPENUMOLEVERB* ppenumOleVerb);

    virtual BOOL fpAtlDoVerb(LONG iVerb, LPMSG lpMsg, HWND hWndParent,
      LPCRECT lprcPosRect, const FPATL_VERB_ENTRY *lpVerbEntries);

//----------------------------------------------------------------------------
//
//  THESE FUNCTIONS CAN BE OVERRIDDEN BY THE SUBCLASS
//
//----------------------------------------------------------------------------
    // This is called when a hWnd has been created, but before the
    // properties for the control are loaded.
    virtual void fpOnCreatePreLoad()   // This may be overridden by subclass
      { return; }

    // This is called when a hWnd has been created and after the
    // properties for the control have been loaded.
    virtual void fpOnCreatePostLoad()  // This may be overridden by subclass
      { return; }

    // This is called when the control by the framework to determine
    // if there is blob data to be saved/loaded. This may be
    // overridden by the subclass to return TRUE.
    virtual BOOL fpAtlHasBlobData(int *lpiRetPropId, 
      LPCTSTR *lptszRetPropName) = 0;

    // This is called when the control is being saved.  This may be
    // overridden by the subclass to put data in the blob and return TRUE.
    virtual BOOL fpAtlGetBlobData(HGLOBAL *lphBlob, int iPropId);

    // This is called when the control is being load.  This may be
    // overridden by the subclass to handle the blob data during a load.
    virtual void fpAtlSetBlobData(HGLOBAL hBlob, int iPropId);

    // This is called to get the PInfo index associated with the
    // passed "dispid".  This may be overridden by the subclass
    // to return a value different than dispid-1.
    // NOTE: Returns -1 if no index for dispid.
    virtual int fpAtlGetPinfoIndex(DISPID dispid)
    {
      int iRet = (dispid > 0 ? (int)dispid - 1 : -1);
      if (m_PropCt && iRet >= m_PropCt)
      {
        _ASSERT(0);  // error, should not be greater than m_PropCt
        iRet = -1;
      }  
      return iRet;
    }

   virtual void fpAtlOnFontChanged(DISPID dispID, CfpAtlFont& fpAtlFont);

   //----------------------------------------------------------------
   // IPropertyBag functions...
   //----------------------------------------------------------------
   virtual HRESULT fpAtlPropBag_Save(CComControlBase *pComCtlBase,
     CComDispatchDriver *pComDispDriver, LPPROPERTYBAG pPropBag,
	 BOOL fClearDirty, BOOL fSaveAllProperties, ATL_PROPMAP_ENTRY* pMap,
     DWORD dwCtlVersion, SIZE *psizeExtent);

   virtual HRESULT fpAtlPropBag_LoadVerAndExtent(
     LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, 
     DWORD *pdwRetCtlVersion, SIZE *psizeRetExtent);

   virtual HRESULT fpAtlPropBag_SaveVerAndExtent(
     LPPROPERTYBAG pPropBag, DWORD dwCtlVersion, SIZE *psizeExtent);

   virtual HRESULT fpAtlPropBagSaveFromfpStream(
     LPPROPERTYBAG pPropBag, ATL_PROPMAP_ENTRY* pMap);

   //----------------------------------------------------------------
   // IPersistStreamInit Functions...
   //----------------------------------------------------------------
   virtual HRESULT fpAtlStream_LoadVerAndExtent(LPSTREAM pStm, 
     DWORD *pdwRetCtlVersion, SIZE *psizeRetExtent);

   virtual HRESULT fpAtlStream_Save(CComControlBase *pComCtlBase,
     CComDispatchDriver *pComDispDriver, LPSTREAM pStm, BOOL fClearDirty,
     ATL_PROPMAP_ENTRY* pMap, DWORD dwCtlVersion, SIZE *psizeExtent);

   //----------------------------------------------------------------
   // Function to read from IPropertyBag or IStream to our Stream
   //----------------------------------------------------------------
   virtual HRESULT fpAtlLoadTofpStream(CComControlBase *pComCtlBase,
     CComDispatchDriver *pComDispDriver, LPSTREAM pStm,
     LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, 
     ATL_PROPMAP_ENTRY* pMap);

   //----------------------------------------------------------------
   // Function to read from our Stream and set prop values in control.
   //----------------------------------------------------------------
   virtual HRESULT fpAtlLoadFromfpStream(CComControlBase *pComCtlBase,
     CComDispatchDriver *pComDispDriver, ATL_PROPMAP_ENTRY* pMap);


   virtual HRESULT fpAtlSaveBlob(LPSTREAM pStm, HGLOBAL hBlob);
   virtual HRESULT fpAtlSaveBlob(LPPROPERTYBAG pPropBag, 
     HGLOBAL hBlob, LPCTSTR tstrPropName);
   virtual HRESULT fpAtlLoadBlob(LPSTREAM pStm, HGLOBAL *lphBlob);
   virtual HRESULT fpAtlLoadBlob(LPPROPERTYBAG pPropBag,
     LPERRORLOG pErrorLog, HGLOBAL *lphBlob, LPCTSTR tstrPropName);

   virtual HRESULT fpAtlLoadPropBagTofpStream(
     CComControlBase *pComCtlBase, CComDispatchDriver *pComDispDriver, 
     LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, ATL_PROPMAP_ENTRY* pMap,
     LPSTREAM pStmLoad);

   virtual HRESULT fpAtlCopyPropStream( LPSTREAM pStmDest, 
     LPSTREAM pStmSrc, BOOL fCopyNullProp = FALSE);


//----------------------------------------------------------------------------

}; // CfpAtlBase


#endif // FPATL_H
