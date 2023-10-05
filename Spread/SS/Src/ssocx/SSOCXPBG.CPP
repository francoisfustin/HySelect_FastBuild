//--------------------------------------------------------------------
//
// File: ssocxpbg.cpp
//
// This file was originally copied from \vc5\mfc\src\ctlpbag.cpp
//
// The following modifications were made to this file...
// 1) Added afxconv.h include file
// 2) Added ssocx.h and ssocxctl.h include files.
// 3) Changed COleControl to CSsocxCtrl.
// 4) Removed calls to DELETE_EXCEPTION().
// 5) Removed calls to _afxAmbientCache->Cache().
// 6) Modified CBlobProperty::Load().
// 7) Copied needed functions from OLEIMPL2.H, CTLFONT.CPP,
//    CTLPROPX.CPP and CTLPSET.CPP files.
// 8) Copied IQuickActivate interface from \v5\mfc\src\ctlquick.cpp
//
//--------------------------------------------------------------------

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include <afxconv.h>
#include "ssocx.h"
#include "ssocxctl.h"
#include "..\sssrc\spread\ss_save4.h"

#ifdef AFXCTL_PROP_SEG
#pragma code_seg(AFXCTL_PROP_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\OLEIMPL2.H file.

#define RELEASE(lpUnk) do \
  { if ((lpUnk) != NULL) { (lpUnk)->Release(); (lpUnk) = NULL; } } while (0)

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLFONT.CPP file.

static const FONTDESC _fdDefault =
  { sizeof(FONTDESC), OLESTR("MS Sans Serif"), FONTSIZE(12), FW_NORMAL,
    DEFAULT_CHARSET, FALSE, FALSE, FALSE };

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLFONT.CPP file.

BOOL AFXAPI _AfxIsSameFont(CFontHolder& font, const FONTDESC* pFontDesc,
  LPFONTDISP pFontDispAmbient)
{
  if (font.m_pFont == NULL)
    return FALSE;

  BOOL bSame = FALSE;

  if (pFontDispAmbient != NULL)
  {
    LPFONT pFontAmbient;
    if (SUCCEEDED(pFontDispAmbient->QueryInterface(IID_IFont,
      (LPVOID*)&pFontAmbient)))
    {
      ASSERT_POINTER(pFontAmbient, IFont);
      bSame = pFontAmbient->IsEqual(font.m_pFont) == S_OK;
      pFontAmbient->Release();
    }
  }
  else
  {
    if (pFontDesc == NULL)
      pFontDesc = &_fdDefault;

    bSame = TRUE;
    BOOL bFlag;

    font.m_pFont->get_Italic(&bFlag);
    bSame = (bFlag == pFontDesc->fItalic);

    if (bSame)
    {
      font.m_pFont->get_Underline(&bFlag);
      bSame = (bFlag == pFontDesc->fUnderline);
    }

    if (bSame)
    {
      font.m_pFont->get_Strikethrough(&bFlag);
      bSame = (bFlag == pFontDesc->fStrikethrough);
    }

    if (bSame)
    {
      short sCharset;
      font.m_pFont->get_Charset(&sCharset);
      bSame = (sCharset == pFontDesc->sCharset);
    }

    if (bSame)
    {
      short sWeight;
      font.m_pFont->get_Weight(&sWeight);
      bSame = (sWeight == pFontDesc->sWeight);
    }

    if (bSame)
    {
      CURRENCY cy;
      font.m_pFont->get_Size(&cy);
      bSame = (memcmp(&cy, &pFontDesc->cySize, sizeof(CURRENCY)) == 0);
    }

    if (bSame)
    {
      BSTR bstrName;
      font.m_pFont->get_Name(&bstrName);
      CString strName1(bstrName);
      CString strName2(pFontDesc->lpstrName);
      bSame = (strName1 == strName2);
      SysFreeString(bstrName);
    }
  }

  return bSame;
}

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLPROPX.CPP file.

#ifdef _DEBUG
#define ASSERT_BUFFER_VALID(p, cb, bWrite) \
  ASSERT(AfxIsValidAddress(p, cb, bWrite))
#else
#define ASSERT_BUFFER_VALID(p, cb, bWrite)
#endif

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLPROPX.CPP file.

BOOL AFXAPI _AfxInitBlob(HGLOBAL* phDst, void* pvSrc)
{
  BOOL bResult = FALSE;
  ULONG cb;
  if ((cb = *(long*)pvSrc) > 0)
  {
    ASSERT_BUFFER_VALID(pvSrc, sizeof(cb) + cb, TRUE);
    *phDst = GlobalAlloc(GMEM_MOVEABLE, sizeof(cb) + cb);
    if (*phDst != NULL)
    {
      void* pvDst = GlobalLock(*phDst);
      ASSERT(pvDst != NULL);
      memcpy(pvDst, pvSrc, sizeof(cb) + cb);
      bResult = TRUE;
      GlobalUnlock(*phDst);
    }
  }
  return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLPROPX.CPP file.

BOOL AFXAPI _AfxCopyBlob(HGLOBAL* phDst, HGLOBAL hSrc)
{
  BOOL bResult = FALSE;
  void* pvSrc = GlobalLock(hSrc);
  if (pvSrc != NULL)
  {
    bResult = _AfxInitBlob(phDst, pvSrc);
    GlobalUnlock(hSrc);
  }
  return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLPROPX.CPP file.

BOOL AFXAPI _AfxCopyPropValue(VARTYPE vtProp, void* pvDest, const void * pvSrc)
{
  ASSERT(AfxIsValidAddress(pvDest, 1));

  if (pvSrc != NULL)
  {
    ASSERT(AfxIsValidAddress(pvSrc, 1, FALSE));

    switch (vtProp)
    {
    case VT_UI1:
      *(BYTE*)pvDest = *(BYTE*)pvSrc;
      break;
    case VT_I2:
      *(short*)pvDest = *(short*)pvSrc;
      break;
    case VT_I4:
      *(long*)pvDest = *(long*)pvSrc;
      break;
    case VT_BOOL:
      *(BOOL*)pvDest = *(BOOL*)pvSrc;
      break;
    case VT_BSTR:
      *(CString*)pvDest = *(CString*)pvSrc;
      break;
    case VT_LPSTR:
      *(CString*)pvDest = (LPCTSTR)pvSrc;
      break;
    case VT_CY:
      *(CY*)pvDest = *(CY*)pvSrc;
      break;
    case VT_R4:
      //*(_AFX_FLOAT*)pvDest = *(_AFX_FLOAT*)pvSrc;
      *(float*)pvDest = *(float*)pvSrc;
      break;
    case VT_R8:
      //*(_AFX_DOUBLE*)pvDest = *(_AFX_DOUBLE*)pvSrc;
      *(double*)pvDest = *(double*)pvSrc;
      break;
    default:
      return FALSE;
    }
  }
  return pvSrc != NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLPROPX.CPP file.

BOOL AFXAPI _AfxIsSameUnknownObject(REFIID iid, LPUNKNOWN pUnk1, LPUNKNOWN pUnk2)
{
  if (pUnk1 == pUnk2)
    return TRUE;

  if (pUnk1 == NULL || pUnk2 == NULL)
    return FALSE;

  LPUNKNOWN pI1 = NULL;
  LPUNKNOWN pI2 = NULL;
  BOOL bResult = FALSE;
  if (SUCCEEDED(pUnk1->QueryInterface(iid, (void**)&pI1)))
  {
    ASSERT_POINTER(pI1, IUnknown);
    if (SUCCEEDED(pUnk2->QueryInterface(iid, (void**)&pI2)))
    {
      ASSERT_POINTER(pI2, IUnknown);
      bResult = (pI1 == pI2);
      pI2->Release();
    }
    pI1->Release();
  }
  return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLPSET.CPP file.

static size_t PASCAL _AfxGetSizeOfVarType(VARTYPE vt)
{
  switch (vt)
  {
  case VT_I2:
  case VT_BOOL:
    return 2;

  case VT_I4:
  case VT_R4:
    return 4;

  case VT_R8:
    return 8;

  case VT_CY:
    return sizeof(CURRENCY);

  case VT_BSTR:
    return sizeof(BSTR);
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Copied from \VC5\MFC\SRC\CTLPSET.CPP file.

BOOL AFXAPI _AfxIsSamePropValue(VARTYPE vtProp, const void* pv1, const void* pv2)
{
  if (pv1 == pv2)
    return TRUE;

  if ((pv1 == NULL) || (pv2 == NULL))
    return FALSE;

  BOOL bSame = FALSE;

  switch (vtProp)
  {
  case VT_BSTR:
    bSame = ((CString*)pv1)->Compare(*(CString*)pv2) == 0;
    break;
  case VT_LPSTR:
    bSame = ((CString*)pv1)->Compare((LPCTSTR)pv2) == 0;
    break;

  case VT_BOOL:
  case VT_I2:
  case VT_I4:
  case VT_CY:
  case VT_R4:
  case VT_R8:
    bSame = memcmp(pv1, pv2, _AfxGetSizeOfVarType(vtProp)) == 0;
    break;
  }

  return bSame;
}

/////////////////////////////////////////////////////////////////////////////
// CBlobProperty

class CBlobProperty : public IPersistStream
{
public:
  CBlobProperty(HGLOBAL pBlob = NULL);
  HGLOBAL GetBlob();

  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();
  STDMETHOD(QueryInterface)(REFIID, LPVOID*);

  STDMETHOD(GetClassID)(LPCLSID);
  STDMETHOD(IsDirty)();
  STDMETHOD(Load)(LPSTREAM);
  STDMETHOD(Save)(LPSTREAM, BOOL);
  STDMETHOD(GetSizeMax)(ULARGE_INTEGER*);

protected:
  long m_dwRef;
  HGLOBAL m_hBlob;
};

CBlobProperty::CBlobProperty(HGLOBAL hBlob) :
  m_hBlob(hBlob),
  m_dwRef(1)
{
}

HGLOBAL CBlobProperty::GetBlob()
{
  return m_hBlob;
}

STDMETHODIMP_(ULONG) CBlobProperty::AddRef()
{
  return InterlockedIncrement(&m_dwRef);
}

STDMETHODIMP_(ULONG) CBlobProperty::Release()
{
  if (InterlockedDecrement(&m_dwRef) > 0)
    return m_dwRef;

  delete this;
  return 0;
}

STDMETHODIMP CBlobProperty::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
  if (IsEqualIID(riid, IID_IUnknown) ||
    IsEqualIID(riid, IID_IPersist) ||
    IsEqualIID(riid, IID_IPersistStream))
  {
    AddRef();
    *ppvObj = this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

static const CLSID _clsidBlobProperty =
{ 0xf6f07540, 0x42ec, 0x11ce, { 0x81, 0x35, 0x0, 0xaa, 0x0, 0x4b, 0xb8, 0x51 } };

STDMETHODIMP CBlobProperty::GetClassID(LPCLSID pClsid)
{
  *pClsid = _clsidBlobProperty;
  return S_OK;
}

STDMETHODIMP CBlobProperty::IsDirty()
{
  return S_OK;
}

STDMETHODIMP CBlobProperty::Load(LPSTREAM pStream)
{
  ULONG cb;
  ULONG cbRead;
  HRESULT hr = pStream->Read(&cb, sizeof(ULONG), &cbRead);

  if (FAILED(hr))
    return hr;
  if (sizeof(ULONG) != cbRead)
    return E_FAIL;

  // check blob length with buffer length
  SS_REC_FILE recFile;
  hr = pStream->Read(&recFile, sizeof(SS_REC_FILE), &cbRead);
  if (FAILED(hr))
    return hr;
  if (sizeof(SS_REC_FILE) != cbRead)
    return E_FAIL;
  if( cb != (ULONG)recFile.lFileLen )
  {
    LARGE_INTEGER li;
    ULARGE_INTEGER uli;
    LISet32(li, -(long)(sizeof(ULONG)+sizeof(CLSID)+sizeof(ULONG)+sizeof(SS_REC_FILE)));
    hr = pStream->Seek(li, STREAM_SEEK_CUR, &uli);
    if (FAILED(hr))
      return hr;
    hr = pStream->Read(&cb, sizeof(ULONG), &cbRead);
    if (FAILED(hr))
      return hr;
    if (sizeof(ULONG) != cbRead)
      return E_FAIL;
    hr = pStream->Read(&recFile, sizeof(SS_REC_FILE), &cbRead);
    if (FAILED(hr))
      return hr;
    if (sizeof(SS_REC_FILE) != cbRead)
      return E_FAIL;
    if( cb != (ULONG)recFile.lFileLen )
      return E_FAIL;
  }

  HGLOBAL hBlobNew = GlobalAlloc(GMEM_MOVEABLE, sizeof(ULONG)+cb);
  if (hBlobNew == NULL)
    return E_OUTOFMEMORY;

  void* pBlobNew = GlobalLock(hBlobNew);
  *(ULONG*)pBlobNew = cb;
  *(SS_REC_FILE*)((LPBYTE)pBlobNew+sizeof(ULONG)) = recFile;
  hr = pStream->Read(((LPBYTE)pBlobNew+sizeof(ULONG)+sizeof(SS_REC_FILE)), cb-sizeof(SS_REC_FILE), &cbRead);
  GlobalUnlock(hBlobNew);

  if (FAILED(hr))
  {
    GlobalFree(hBlobNew);
    return hr;
  }

  if (cb - sizeof(SS_REC_FILE) != cbRead)
  {
    GlobalFree(hBlobNew);
    return E_FAIL;
  }

  GlobalFree(m_hBlob);
  m_hBlob = hBlobNew;
  return S_OK;
}

STDMETHODIMP CBlobProperty::Save(LPSTREAM pStream, BOOL)
{
  void* pBlob = GlobalLock(m_hBlob);
  if (pBlob == NULL)
    return E_OUTOFMEMORY;

  ULONG cb = sizeof(ULONG) + *(ULONG*)pBlob;
  ULONG cbWritten;
  HRESULT hr = pStream->Write(pBlob, cb, &cbWritten);

  GlobalUnlock(m_hBlob);

  if (FAILED(hr))
    return hr;

  if (cb != cbWritten)
    return E_FAIL;

  return S_OK;
}

STDMETHODIMP CBlobProperty::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
  void* pBlob = GlobalLock(m_hBlob);
  if (pBlob == NULL)
    return E_OUTOFMEMORY;

  pcbSize->HighPart = 0;
  pcbSize->LowPart = sizeof(ULONG) + *(ULONG*)pBlob;

  GlobalUnlock(m_hBlob);
  return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CPropbagPropExchange

class CPropbagPropExchange : public CPropExchange
{
public:
  CPropbagPropExchange(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog,
    BOOL bLoading, BOOL bSaveAllProperties=FALSE);
  ~CPropbagPropExchange();

// Operations
  virtual BOOL ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
        void* pvProp, const void* pvDefault = NULL);
  virtual BOOL ExchangeBlobProp(LPCTSTR pszPropName, HGLOBAL* phBlob,
        HGLOBAL hBlobDefault = NULL);
  virtual BOOL ExchangeFontProp(LPCTSTR pszPropName, CFontHolder& font,
        const FONTDESC* pFontDesc, LPFONTDISP pFontDispAmbient);
  virtual BOOL ExchangePersistentProp(LPCTSTR pszPropName,
        LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault);

// Implementation
  LPPROPERTYBAG m_pPropBag;
  LPERRORLOG m_pErrorLog;
  BOOL m_bSaveAllProperties;
};

CPropbagPropExchange::CPropbagPropExchange(LPPROPERTYBAG pPropBag,
  LPERRORLOG pErrorLog, BOOL bLoading, BOOL bSaveAllProperties) :
  m_pPropBag(pPropBag),
  m_pErrorLog(pErrorLog),
  m_bSaveAllProperties(bSaveAllProperties)
{
  m_bLoading = bLoading;
  if (pPropBag != NULL)
    pPropBag->AddRef();
  if (pErrorLog != NULL)
    pErrorLog->AddRef();
}

CPropbagPropExchange::~CPropbagPropExchange()
{
  RELEASE(m_pPropBag);
  RELEASE(m_pErrorLog);
}

BOOL CPropbagPropExchange::ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
  void* pvProp, const void* pvDefault)
{
  USES_CONVERSION;

  ASSERT_POINTER(m_pPropBag, IPropertyBag);
  ASSERT(AfxIsValidString(pszPropName));
  ASSERT(AfxIsValidAddress(pvProp, 1, FALSE));
  ASSERT((pvDefault == NULL) || AfxIsValidAddress(pvDefault, 1, FALSE));

  if (m_pPropBag == NULL)
    return FALSE;

  BOOL bSuccess = TRUE;
  VARIANT var;
  AfxVariantInit(&var);
  V_VT(&var) = vtProp;

  if (vtProp == VT_LPSTR)
    V_VT(&var) = VT_BSTR;

  if (m_bLoading)
  {
    if (FAILED(m_pPropBag->Read(T2COLE(pszPropName), &var, m_pErrorLog)))
      return _AfxCopyPropValue(vtProp, pvProp, pvDefault);

    switch (vtProp)
    {
    case VT_UI1:
      *(BYTE*)pvProp = V_UI1(&var);
      break;

    case VT_I2:
      *(short*)pvProp = V_I2(&var);
      break;

    case VT_I4:
      *(long*)pvProp = V_I4(&var);
      break;

    case VT_BOOL:
      *(BOOL*)pvProp = (BOOL)V_BOOL(&var);
      break;

    case VT_LPSTR:
    case VT_BSTR:
      *(CString*)pvProp = OLE2CT(V_BSTR(&var));
      break;

    case VT_CY:
      *(CY*)pvProp = V_CY(&var);
      break;

    case VT_R4:
      memcpy(pvProp, &V_R4(&var), sizeof(float));
      break;

    case VT_R8:
      memcpy(pvProp, &V_R8(&var), sizeof(double));
      break;

    default:
      bSuccess = FALSE;
    }
  }
  else
  {
    if (m_bSaveAllProperties ||
      !_AfxIsSamePropValue(vtProp, pvProp, pvDefault))
    {
      switch (vtProp)
      {
      case VT_UI1:
        V_UI1(&var) = *(BYTE*)pvProp;
        break;

      case VT_I2:
        V_I2(&var) = *(short*)pvProp;
        break;

      case VT_I4:
        V_I4(&var) = *(long*)pvProp;
        break;

      case VT_BOOL:
        V_BOOL(&var) = (VARIANT_BOOL)*(BOOL*)pvProp;
        break;

      case VT_LPSTR:
      case VT_BSTR:
        V_BSTR(&var) = SysAllocString(T2COLE(*(CString*)pvProp));
        break;

      case VT_CY:
        V_CY(&var) = *(CY*)pvProp;
        break;

      case VT_R4:
        memcpy(&V_R4(&var), pvProp, sizeof(float));
        break;

      case VT_R8:
        memcpy(&V_R8(&var), pvProp, sizeof(double));
        break;

      default:
        return FALSE;
      }
      bSuccess = SUCCEEDED(m_pPropBag->Write(T2COLE(pszPropName), &var));
    }
  }

  VariantClear(&var);
  return bSuccess;
}

BOOL CPropbagPropExchange::ExchangeBlobProp(LPCTSTR pszPropName,
  HGLOBAL* phBlob, HGLOBAL hBlobDefault)
{
  USES_CONVERSION;

  ASSERT_POINTER(m_pPropBag, IPropertyBag);
  ASSERT(AfxIsValidString(pszPropName));
  ASSERT_POINTER(phBlob, HGLOBAL);

  BOOL bSuccess = FALSE;
  VARIANT var;
  AfxVariantInit(&var);
  V_VT(&var) = VT_UNKNOWN;

  if (m_bLoading)
  {
    if (*phBlob != NULL)
    {
      GlobalFree(*phBlob);
      *phBlob = NULL;
    }

    CBlobProperty* pBlobProp = new CBlobProperty;
    V_UNKNOWN(&var) = pBlobProp;

    if (SUCCEEDED(m_pPropBag->Read(T2COLE(pszPropName), &var, m_pErrorLog)))
    {
      *phBlob = pBlobProp->GetBlob();
      bSuccess = TRUE;
    }
    else
    {
      if (hBlobDefault != NULL)
        bSuccess = _AfxCopyBlob(phBlob, hBlobDefault);
    }

    pBlobProp->Release();
  }
  else
  {
    CBlobProperty* pBlobProp = new CBlobProperty(*phBlob);
    V_UNKNOWN(&var) = pBlobProp;
    bSuccess = SUCCEEDED(m_pPropBag->Write(T2COLE(pszPropName), &var));
    pBlobProp->Release();
  }
  return bSuccess;
}

BOOL CPropbagPropExchange::ExchangeFontProp(LPCTSTR pszPropName,
  CFontHolder& font, const FONTDESC* pFontDesc,
  LPFONTDISP pFontDispAmbient)
{
  USES_CONVERSION;

  ASSERT_POINTER(m_pPropBag, IPropertyBag);
  ASSERT(AfxIsValidString(pszPropName));
  ASSERT_POINTER(&font, CFontHolder);
  ASSERT_NULL_OR_POINTER(pFontDesc, FONTDESC);
  ASSERT_NULL_OR_POINTER(pFontDispAmbient, IFontDisp);

  BOOL bSuccess = FALSE;
  VARIANT var;
  AfxVariantInit(&var);
  V_VT(&var) = VT_UNKNOWN;

  if (m_bLoading)
  {
    LPFONT pFont = NULL;

    bSuccess =
      SUCCEEDED(m_pPropBag->Read(T2COLE(pszPropName), &var,
        m_pErrorLog)) &&
      SUCCEEDED(V_UNKNOWN(&var)->QueryInterface(IID_IFont,
        (LPVOID*)&pFont));

    if (bSuccess)
    {
      ASSERT_POINTER(pFont, IFont);
      font.SetFont(pFont);
    }
    else
    {
      // Initialize font to its default state
      font.InitializeFont(pFontDesc, pFontDispAmbient);
    }
    VariantClear(&var);
  }
  else
  {
    if ((font.m_pFont == NULL) ||
      (_AfxIsSameFont(font, pFontDesc, pFontDispAmbient) &&
        !m_bSaveAllProperties))
    {
      bSuccess = TRUE;
    }
    else
    {
      V_UNKNOWN(&var) = font.m_pFont;
      bSuccess = SUCCEEDED(m_pPropBag->Write(T2COLE(pszPropName), &var));
    }
  }

  return bSuccess;
}

BOOL CPropbagPropExchange::ExchangePersistentProp(LPCTSTR pszPropName,
  LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault)
{
  USES_CONVERSION;

  ASSERT_POINTER(m_pPropBag, IPropertyBag);
  ASSERT(AfxIsValidString(pszPropName));
  ASSERT_POINTER(ppUnk, LPUNKNOWN);
  ASSERT_NULL_OR_POINTER(pUnkDefault, IUnknown);

  BOOL bSuccess = FALSE;
  VARIANT var;
  AfxVariantInit(&var);
  V_VT(&var) = VT_UNKNOWN;

  if (m_bLoading)
  {
    RELEASE(*ppUnk);
    *ppUnk = NULL;

    bSuccess =
      SUCCEEDED(m_pPropBag->Read(T2COLE(pszPropName), &var,
        m_pErrorLog)) &&
      SUCCEEDED(V_UNKNOWN(&var)->QueryInterface(iid, (LPVOID*)ppUnk));

    if (!bSuccess)
    {
      // Use default value.
      if (pUnkDefault != NULL)
      {
        bSuccess = SUCCEEDED(pUnkDefault->QueryInterface(iid,
          (LPVOID*)ppUnk));
      }
      else
      {
        bSuccess = TRUE;
      }
    }
    VariantClear(&var);
  }
  else
  {
    if ((*ppUnk == NULL) ||
      (_AfxIsSameUnknownObject(iid, *ppUnk, pUnkDefault) &&
        !m_bSaveAllProperties))
    {
      bSuccess = TRUE;
    }
    else
    {
      V_UNKNOWN(&var) = *ppUnk;
      bSuccess = SUCCEEDED(m_pPropBag->Write(T2COLE(pszPropName), &var));
    }
  }

  return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::XPersistPropertyBag

STDMETHODIMP_(ULONG) CSsocxCtrl::XPersistPropertyBag::AddRef()
{
  METHOD_PROLOGUE_EX_(CSsocxCtrl, PersistPropertyBag)
  return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CSsocxCtrl::XPersistPropertyBag::Release()
{
  METHOD_PROLOGUE_EX_(CSsocxCtrl, PersistPropertyBag)
  return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP CSsocxCtrl::XPersistPropertyBag::QueryInterface(
  REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE_EX_(CSsocxCtrl, PersistPropertyBag)
  return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CSsocxCtrl::XPersistPropertyBag::GetClassID(LPCLSID lpClassID)
{
  METHOD_PROLOGUE_EX_(CSsocxCtrl, PersistPropertyBag)
  return pThis->GetClassID(lpClassID);
}

STDMETHODIMP CSsocxCtrl::XPersistPropertyBag::InitNew()
{
  METHOD_PROLOGUE_EX(CSsocxCtrl, PersistPropertyBag)

  // Delegate to OnResetState.
  pThis->OnResetState();

  // Unless IOleObject::SetClientSite is called after this, we can
  // count on ambient properties being available while loading.
  pThis->m_bCountOnAmbients = TRUE;

  // Properties have been initialized
  pThis->m_bInitialized = TRUE;

  // Uncache cached ambient properties
  //_afxAmbientCache->Cache(NULL);

  return S_OK;
}

STDMETHODIMP CSsocxCtrl::XPersistPropertyBag::Load(LPPROPERTYBAG pPropBag,
  LPERRORLOG pErrorLog)
{
  METHOD_PROLOGUE_EX(CSsocxCtrl, PersistPropertyBag)

  HRESULT hr;

  TRY
  {
    CPropbagPropExchange px(pPropBag, pErrorLog, TRUE);
    pThis->DoPropExchange(&px);
    hr = S_OK;
  }
  CATCH_ALL(e)
  {
    hr = E_FAIL;
    //DELETE_EXCEPTION(e);  // DELETE_EXCEPTION is only used in MFC library 
  }
  END_CATCH_ALL

  // Properties have probably changed
  pThis->BoundPropertyChanged(DISPID_UNKNOWN);
  pThis->InvalidateControl();

  // Clear the modified flag.
  pThis->m_bModified = FALSE;

  // Properties have been initialized
  pThis->m_bInitialized = TRUE;

  // Uncache cached ambient properties
  //_afxAmbientCache->Cache(NULL);

  return hr;
}

STDMETHODIMP CSsocxCtrl::XPersistPropertyBag::Save(LPPROPERTYBAG pPropBag,
  BOOL fClearDirty, BOOL fSaveAllProperties)
{
  METHOD_PROLOGUE_EX(CSsocxCtrl, PersistPropertyBag)

  HRESULT hr;

  TRY
  {
    CPropbagPropExchange px(pPropBag, NULL, FALSE, fSaveAllProperties);
    pThis->DoPropExchange(&px);
    hr = S_OK;
  }
  CATCH_ALL(e)
  {
    hr = E_FAIL;
    //DELETE_EXCEPTION(e);  // DELETE_EXCEPTION is only used in MFC library 
  }
  END_CATCH_ALL

  // Bookkeeping:  Clear the dirty flag, if requested.
  if (fClearDirty)
    pThis->m_bModified = FALSE;

  return hr;
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::XQuickActivate

STDMETHODIMP_(ULONG) CSsocxCtrl::XQuickActivate::AddRef()
{
	// Delegate to our exported AddRef.
	METHOD_PROLOGUE_EX_(CSsocxCtrl, QuickActivate)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CSsocxCtrl::XQuickActivate::Release()
{
	// Delegate to our exported Release.
	METHOD_PROLOGUE_EX_(CSsocxCtrl, QuickActivate)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP CSsocxCtrl::XQuickActivate::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	// Delegate to our exported QueryInterface.
	METHOD_PROLOGUE_EX_(CSsocxCtrl, QuickActivate)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CSsocxCtrl::XQuickActivate::QuickActivate(
	QACONTAINER *pQAContainer, QACONTROL *pQAControl)
{
	METHOD_PROLOGUE_EX_(CSsocxCtrl, QuickActivate)

	// Get the IOleObject interface
	HRESULT hr = S_OK;
	IOleObject* pOleObject = NULL;
	if (FAILED(hr = pThis->ExternalQueryInterface(&IID_IOleObject,
		reinterpret_cast<void**>(&pOleObject))))
	{
		return hr;
	}

	// Keep copy of ambient properties
	//_afxAmbientCache->Cache(pQAContainer);

	// Set client site
	ASSERT(pOleObject != NULL);
	pOleObject->SetClientSite(pQAContainer->pClientSite);

	// Establish connections
	DWORD dwDummy;
	if (pQAContainer->pAdviseSink != NULL)
		pOleObject->Advise(pQAContainer->pAdviseSink, &dwDummy);

	if (pQAContainer->pPropertyNotifySink != NULL)
		pThis->m_xPropConnPt.m_xConnPt.Advise(pQAContainer->pPropertyNotifySink,
			&pQAControl->dwPropNotifyCookie);
	if (pQAContainer->pUnkEventSink != NULL)
		pThis->m_xEventConnPt.m_xConnPt.Advise(pQAContainer->pUnkEventSink,
			&pQAControl->dwEventCookie);

	// Fill in return values
	IViewObjectEx* pViewObject;
	if (SUCCEEDED(pThis->ExternalQueryInterface(&IID_IViewObjectEx,
		reinterpret_cast<void**>(&pViewObject))))
	{
		pViewObject->GetViewStatus(&pQAControl->dwViewStatus);

		// Set advise sink on IViewObject, while we're here.
		if (pQAContainer->pAdviseSink != NULL)
			pViewObject->SetAdvise(DVASPECT_CONTENT, 0, pQAContainer->pAdviseSink);

		pViewObject->Release();
	}
	else
	{
		pQAControl->dwViewStatus = 0;
	}

	pOleObject->GetMiscStatus(DVASPECT_CONTENT, &pQAControl->dwMiscStatus);

	pOleObject->Release();

	return S_OK;
}

STDMETHODIMP CSsocxCtrl::XQuickActivate::SetContentExtent(LPSIZEL lpsizel)
{
	METHOD_PROLOGUE_EX_(CSsocxCtrl, QuickActivate)
	return pThis->m_xOleObject.SetExtent(DVASPECT_CONTENT, lpsizel);
}

STDMETHODIMP CSsocxCtrl::XQuickActivate::GetContentExtent(LPSIZEL lpsizel)
{
	METHOD_PROLOGUE_EX_(CSsocxCtrl, QuickActivate)
	return pThis->m_xOleObject.GetExtent(DVASPECT_CONTENT, lpsizel);
}
