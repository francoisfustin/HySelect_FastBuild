/****************************************************************************
* FPATL.CPP  -  Implementation of FarPoint's ATL classes & templates
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
#include "stdafx.h"
#include "fpatl.h"

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   ( (HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli) )
#define MAP_LOGHIM_TO_PIX(x,ppli)   ( ((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH )

/***************************************************************************
 *
 * FUNCTION:  fpRectHiMetricToPixel()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  If successful, a newly created CPBValCtl object is returned.
 *
 **************************************************************************/
void fpRectHiMetricToPixel(LPRECT lprc)
{
  int nPixelsPerInchX;    // Pixels per logical inch along width
  int nPixelsPerInchY;    // Pixels per logical inch along height

  HDC hDCScreen = ::GetDC(NULL);
  _ASSERTE(hDCScreen != NULL);
  nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
  nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
  ::ReleaseDC(NULL, hDCScreen);

  lprc->left = MAP_LOGHIM_TO_PIX(lprc->left, nPixelsPerInchX);
  lprc->top  = MAP_LOGHIM_TO_PIX(lprc->top,  nPixelsPerInchY);
  lprc->right  = MAP_LOGHIM_TO_PIX(lprc->right,  nPixelsPerInchX);
  lprc->bottom = MAP_LOGHIM_TO_PIX(lprc->bottom, nPixelsPerInchY);
}

/***************************************************************************
 *
 * FUNCTION:  fpGetOleShiftState()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  If successful, a newly created CPBValCtl object is returned.
 *
 **************************************************************************/
short fpGetOleShiftState()
{
  short sRet  = (GetKeyState(VK_SHIFT) < 0)   ? 1 : 0;
	    sRet |= (GetKeyState(VK_CONTROL) < 0) ? 2 : 0;
	    sRet |= (GetKeyState(VK_MENU) < 0)    ? 4 : 0;

  return sRet;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlCoTaskCopyString()
 *
 * DESCRIPTION:
 *
 *   NOTE:  This code was copied from MS's _AfxCopyString().
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
LPOLESTR fpAtlCoTaskCopyString(LPCTSTR psz)
{
	if (psz == NULL)
		return NULL;

	int cch = lstrlen(psz) + 1;
	LPOLESTR pszCopy = NULL;

	if ((pszCopy = (LPOLESTR)CoTaskMemAlloc(cch * sizeof(OLECHAR))) != NULL)
	{
#ifdef _UNICODE
		wcscpy(pszCopy, psz);
#elif !defined(OLE2ANSI)
		MultiByteToWideChar(CP_ACP, 0, psz, -1, pszCopy, cch);
#else
		lstrcpy(pszCopy, psz);
#endif
	}

	return pszCopy;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlCoTaskCopyOleStr()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
LPOLESTR fpAtlCoTaskCopyOleStr(LPOLESTR psz)
{
	if (psz == NULL)
		return NULL;

	LPOLESTR pszCopy = NULL;
    int len = (wcslen(psz) + 1) * sizeof(OLECHAR);

	if ((pszCopy = (LPOLESTR)CoTaskMemAlloc(len)) != NULL)
	  memcpy(pszCopy, psz, len);

	return pszCopy;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlTranslateVTtype()
 *
 * DESCRIPTION:
 *
 *   VB 4.0 does not recognize certain VT types, so we must
 *   translates those VT types to ones it knows.
 *
 * PARAMETERS:
 *
 * RETURNS:  vt (different vt or the same vt)
 *
 **************************************************************************/
VARTYPE fpAtlTranslateVTtype(VARTYPE vt)
{
  switch (vt)
  {
    case VT_UI1:
      return VT_I1;

    case VT_UI2:
      return VT_I2;

    case VT_UI4:
      return VT_I4;

    case VT_I8:
    case VT_UI8:
      return VT_R8;

    case VT_DISPATCH:
      return VT_UNKNOWN;
  }

  // default: return original type
  return vt;
}

//---------------------------------------------------------------------------
//
// class CfpAtlWndClassInfo 
//
// This class will automatically Unregister the "Window class" which 
// is created (registered) by the control in order to subclass it.
// The "new" Window class name is "ATL" + original window class name.
//
//---------------------------------------------------------------------------

/***************************************************************************
 *
 * FUNCTION:  CfpAtlWndClassInfo() constructor
 *
 * CLASS:     CfpAtlWndClassInfo
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  void
 *
 **************************************************************************/
CfpAtlWndClassInfo::CfpAtlWndClassInfo(
    WNDCLASSEX wc,
	LPCTSTR lpszOrigName,
	WNDPROC _pWndProc,
	LPCTSTR lpszCursorID,
	BOOL bSystemCursor,
	ATOM atom,
	LPCTSTR szAutoName) : m_hInst(NULL), m_atomReg(0)
{
  m_wc = wc; 
  m_lpszOrigName = lpszOrigName;
  pWndProc = _pWndProc;
  m_lpszCursorID = lpszCursorID;
  m_bSystemCursor = bSystemCursor;
  m_atom = atom;
  _tcscpy(m_szAutoName, szAutoName);
}

/***************************************************************************
 *
 * FUNCTION:  ~CfpAtlWndClassInfo() destructor
 *
 * CLASS:     CfpAtlWndClassInfo
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  void
 *
 **************************************************************************/
CfpAtlWndClassInfo::~CfpAtlWndClassInfo()
{
  if (m_atomReg && m_hInst)
    ::UnregisterClass((LPCTSTR)m_atomReg, m_hInst);
}

/***************************************************************************
 *
 * FUNCTION:  CfpAtlWndClassInfo::Register() 
 *
 * CLASS:     CfpAtlWndClassInfo
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  void
 *
 **************************************************************************/
ATOM CfpAtlWndClassInfo::Register(WNDPROC* wndProc)
{
  ATOM atom = CWndClassInfo::Register(wndProc);
  if (!m_atomReg)
  {
    m_atomReg = atom;
    m_hInst = _Module.GetModuleInstance();
  }
  return atom;
}


//---------------------------------------------------------------------------
//
// class CfpBlobObject definition
//
//---------------------------------------------------------------------------
// {9FA4D291-50AE-11d1-8DA1-0000C004958C}
static const CLSID _clsidfpBlobObject =
{ 0x9fa4d291, 0x50ae, 0x11d1, { 0x8d, 0xa1, 0x0, 0x0, 0xc0, 0x4, 0x95, 0x8c } };


class CfpBlobObject : 
  public CComObjectRoot, public IPersistStream
{
public:
	CfpBlobObject(HGLOBAL hBlob = NULL) :
	  m_hBlob(hBlob)//,
	  //m_dwRef(1)
      { InternalAddRef(); }

	HGLOBAL GetBlob()  {return m_hBlob;}

	STDMETHOD_(ULONG, AddRef)()  {return InternalAddRef();}

	STDMETHOD_(ULONG, Release)() 
	{
		ULONG ret = InternalRelease();
		if( ret == 0 )
			delete this;
		return ret;
	}

	STDMETHOD(QueryInterface)(REFIID iid, LPVOID*ppvObj)
      { return _InternalQueryInterface(iid, ppvObj); }

	STDMETHOD(GetClassID)(LPCLSID pClsid)
    {
	  *pClsid = _clsidfpBlobObject;
	  return S_OK;
    }

	STDMETHOD(IsDirty)()  {return S_OK;}

	STDMETHOD(Load)(LPSTREAM);
	STDMETHOD(Save)(LPSTREAM, BOOL);
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER*);

BEGIN_COM_MAP(CfpBlobObject)
	COM_INTERFACE_ENTRY(IPersistStream)
FP_END_COM_MAP()

protected:
	//long m_dwRef;
	HGLOBAL m_hBlob;
};

//---------------------------------------------------------------------------
//
// class CfpBlobObjectOld definition
//
//---------------------------------------------------------------------------
// {???}

static const CLSID _clsidfpBlobObjectOld =
{ 0xf6f07540, 0x42ec, 0x11ce, { 0x81, 0x35, 0x0, 0xaa, 0x0, 0x4b, 0xb8, 0x51 } };

class CfpBlobObjectOld : public CfpBlobObject
{
public:
	STDMETHOD(GetClassID)(LPCLSID pClsid)
    {
	  *pClsid = _clsidfpBlobObjectOld;
	  return S_OK;
    }

BEGIN_COM_MAP(CfpBlobObjectOld)
	COM_INTERFACE_ENTRY(IPersistStream)
FP_END_COM_MAP()

};

/***************************************************************************
 *
 * FUNCTION:  Load() 
 *
 * CLASS:     CfpBlobObject
 *
 * DESCRIPTION:
 *
 *   Loads (reads) a blob from the stream.  The blob is stored in
 *   a member variable.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
STDMETHODIMP CfpBlobObject::Load(LPSTREAM pStream)
{
	ULONG cb;
	ULONG cbRead;
	HRESULT hr = pStream->Read(&cb, sizeof(ULONG), &cbRead);

	if (FAILED(hr))
		return hr;

	if (sizeof(ULONG) != cbRead)
		return E_FAIL;

	HGLOBAL hBlobNew = GlobalAlloc(GMEM_MOVEABLE, sizeof(ULONG)+cb);
	if (hBlobNew == NULL)
		return E_OUTOFMEMORY;

	void* pBlobNew = GlobalLock(hBlobNew);
	*(ULONG*)pBlobNew = cb;
	hr = pStream->Read(((ULONG*)pBlobNew)+1, cb, &cbRead);
	GlobalUnlock(hBlobNew);

	if (FAILED(hr))
	{
		GlobalFree(hBlobNew);
		return hr;
	}
	if (cb != cbRead)
	{
		GlobalFree(hBlobNew);
		return E_FAIL;
	}

	if (m_hBlob) GlobalFree(m_hBlob);
	m_hBlob = hBlobNew;
	return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  Save() 
 *
 * CLASS:     CfpBlobObject
 *
 * DESCRIPTION:
 *
 *   Saves (writes) a blob stored in a member variable to the stream.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
STDMETHODIMP CfpBlobObject::Save(LPSTREAM pStream, BOOL)
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

/***************************************************************************
 *
 * FUNCTION:  GetSizeMax() 
 *
 * CLASS:     CfpBlobObject
 *
 * DESCRIPTION:
 *
 *   Returns the size of the blob.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
STDMETHODIMP CfpBlobObject::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
	void* pBlob = GlobalLock(m_hBlob);
	if (pBlob == NULL)
		return E_OUTOFMEMORY;

	pcbSize->HighPart = 0;
	pcbSize->LowPart = sizeof(ULONG) + *(ULONG*)pBlob;

	GlobalUnlock(m_hBlob);
	return S_OK;
}



/***************************************************************************
 *
 * FUNCTION:  fpAtl_putref_Font() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This sets the font in the control to the one passed-in.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
STDMETHODIMP CfpAtlBase::fpAtl_putref_Font(IFontDisp* pFont,
  DISPID DispId, CfpAtlFont& pComFont)
{
	if (fpAtlFireOnRequestEdit(DispId) == S_FALSE)
		return S_FALSE;

	pComFont = pFont;  // set member variable

/* SCP 12/11/97: This is commented-out because the WM_SETFONT is now
                 handled (sent) in fpAtlOnFontChanged().
                 
    if (fpAtlGetHWnd())
    {
	  CComQIPtr<IFont, &IID_IFont> pFontTmp(pFont);
      if (pFontTmp)
      {
        HFONT hFont = NULL;
        pFontTmp->get_hFont(&hFont);
        pFontTmp->AddRefHfont(hFont);
        ::SendMessage(fpAtlGetHWnd(), WM_SETFONT, (WPARAM)hFont, 0);
        pFontTmp->ReleaseHfont(hFont);
      }
      else
        ::SendMessage(fpAtlGetHWnd(), WM_SETFONT, NULL, 0);
    }
*/
	fpAtlSetModifiedFlag();
	fpAtlOnFontChanged(DispId, pComFont);
	fpAtlFireOnChanged(DispId);
	fpAtlFireViewChange();
	return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtl_put_Font() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This makes a copy of the passed-in font and then sets the font
 *   in the control.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
STDMETHODIMP CfpAtlBase::fpAtl_put_Font(IFontDisp* pFont, DISPID DispId,
  CfpAtlFont& pComFont)
{
  CComPtr<IFontDisp> pFontClone;
  if (fpAtlFireOnRequestEdit(DispId) == S_FALSE)
  	return S_FALSE;
  if (pFont)
  {
	CComQIPtr<IFont, &IID_IFont> p(pFont);
	if (p)
	{
	  CComPtr<IFont> pFont;
	  p->Clone(&pFont);
	  if (pFont)
		pFont->QueryInterface(IID_IFontDisp, (void**) &pFontClone);
	}
  }

  return fpAtl_putref_Font(pFontClone, DispId, pComFont);
}



/***************************************************************************
 *
 * FUNCTION:  fpAtl_get_Font() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This gets the font from the control.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
STDMETHODIMP CfpAtlBase::fpAtl_get_Font(IFontDisp** ppFont,
  CfpAtlFont& pComFont)
{
  LOGFONT  LogFont;
  HFONT    hFont;
  FONTDESC fontDesc;
  long     lHeight;
  HDC      hdc;
  CComPtr<IFont>   pFontNew;

  // Get font from control, create a IFont with same attributes and
  // put it in m_pFont.
  if (fpAtlGetHWnd() &&
      (hFont = (HFONT)::SendMessage(fpAtlGetHWnd(), WM_GETFONT, 0, 0)))
  {
    GetObject(hFont, sizeof(LOGFONT), &LogFont);
    hdc = ::GetDC(GetDesktopWindow());
    lHeight = LogFont.lfHeight;
    lHeight *= lHeight > 0 ? 72 : -72;
    lHeight *= 10000;
    lHeight /= ::GetDeviceCaps(hdc, LOGPIXELSY);
    fontDesc.cbSizeofstruct = sizeof(fontDesc);
    // if necessary, convert from CHAR to WCHAR
    fontDesc.lpstrName = (BSTR)fpSysAllocBSTRFromTString(LogFont.lfFaceName);
    fontDesc.cySize.Lo = lHeight;
    fontDesc.cySize.Hi = 0;
    fontDesc.sWeight = (short)LogFont.lfWeight;
    fontDesc.sCharset = LogFont.lfCharSet;
    fontDesc.fItalic = LogFont.lfItalic;
    fontDesc.fUnderline = LogFont.lfUnderline;
    fontDesc.fStrikethrough = LogFont.lfStrikeOut;
	if (OleCreateFontIndirect(&fontDesc, IID_IFont,
			(LPVOID *)&pFontNew) == S_OK && pFontNew)            
    {
      CComPtr<IFontDisp> pFontTmp;

//      pComFont = (IFontDisp*)NULL; // Clear current font
      // if we get a new IFontDisp ptr, set member variable
	  if (pFontNew->QueryInterface(IID_IFontDisp, (void**)&pFontTmp) == S_OK)
	    pComFont = pFontTmp;
    }
    if (fontDesc.lpstrName)
      SysFreeString(fontDesc.lpstrName);
    if (hdc)
       ::ReleaseDC(GetDesktopWindow(), hdc);
  }

  // Return ptr to font
  *ppFont = pComFont;
  if (*ppFont != NULL)   //"pComFont = pFontTmp" calls AddRef()  --RAP 23Apr98 -- but not for this reference! -scl
	(*ppFont)->AddRef();
//  if (pFontNew)            //Release the font created above --RAP 23Apr98
//    pFontNew->Release();
  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtl_ClonePicture() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This clones the passed-in picture object and returns it.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
STDMETHODIMP CfpAtlBase::fpAtl_ClonePicture(LPPICTUREDISP pPicture,
  DISPID dispId, LPPICTUREDISP *ppPictClone)
{
  if (fpAtlFireOnRequestEdit(dispId) == S_FALSE)
	return S_FALSE;

  if (pPicture)
  {
	CComQIPtr<IPersistStream, &IID_IPersistStream> p(pPicture);
	if (p)
	{
	  ULARGE_INTEGER l;
	  p->GetSizeMax(&l);
	  HGLOBAL hGlob = GlobalAlloc(GHND, l.LowPart);
	  if (hGlob)
	  {
		CComPtr<IStream> spStream;
		CreateStreamOnHGlobal(hGlob, TRUE, &spStream);
		if (spStream)
		{
		  if (SUCCEEDED(p->Save(spStream, FALSE)))
		  {
			LARGE_INTEGER l;
			l.QuadPart = 0;
			spStream->Seek(l, STREAM_SEEK_SET, NULL);
			OleLoadPicture(spStream, l.LowPart, FALSE, IID_IPictureDisp, (void**)ppPictClone);
		  }
		  spStream.Release();
		}
		GlobalFree(hGlob);
	  }
	}
  }
  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlGetControlInfo() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function is used to override the implementation of the 
 *   interface function: IOleControl::GetControlInfo().  This gets the
 *   mnemonics info from the control and puts it in the expected
 *   format.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlGetControlInfo(LPCONTROLINFO lpi)
{
#define FPATL_MAX_MNEMONIC_CT  255
  WORD     wzKeys[FPATL_MAX_MNEMONIC_CT];
  HACCEL   hAccel = NULL;
  int      nCnt = 0;
  int      i;
  LPACCEL  lpAccel;
  DWORD    dwFlags = 0;

  if (!lpi)
    return E_POINTER;

  lpi->cb = sizeof(CONTROLINFO);

  // Get the list of mnemonics from the control. If returns false then exit.
  if (!fpOnGetMnemonicsInfo(FPATL_MAX_MNEMONIC_CT, &nCnt, wzKeys, &dwFlags)
      || (nCnt == 0))
  {
    lpi->hAccel = NULL;
    lpi->cAccel = 0;
    lpi->dwFlags = 0;
    return S_OK;
  }

  hAccel = (HACCEL)GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, 
                               sizeof(ACCEL) * nCnt);
  if (!hAccel)
    return E_OUTOFMEMORY;

  lpAccel = (LPACCEL)GlobalLock(hAccel);
  
  for (i = 0; i < nCnt; i++)
  {
    lpAccel[i].fVirt = FALT | FVIRTKEY;
    lpAccel[i].key = toupper((int)wzKeys[i]);
    lpAccel[i].cmd = TRUE;
  }
  
  
  lpAccel[nCnt - 1].fVirt |= 0x80;  // Mark last record for WIN16
  
// fix for JAP7586 -scl
//  {
    if (m_hAccel)
	   DestroyAcceleratorTable(m_hAccel);
//    HACCEL hAccelNew = CreateAcceleratorTable((LPACCEL)lpAccel, nCnt);
	m_hAccel = CreateAcceleratorTable((LPACCEL)lpAccel, nCnt);
    GlobalUnlock(hAccel);
    GlobalFree(hAccel);
//    hAccel = hAccelNew;
    hAccel = m_hAccel;
//  }
// fix for JAP7586 -scl
  
  lpi->hAccel = hAccel;
  lpi->cAccel = nCnt;
  lpi->dwFlags = dwFlags;

  return S_OK;  
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlGetPredefinedStrings() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function is used to override the implementation of the 
 *   interface function: IPerPropertyBrowsing::GetPredefinedStrings().
 *   This gets the list of values (in string format) for a property.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlGetPredefinedStrings(DISPID dispid,
  CALPOLESTR *pCaStringsOut, CADWORD *pCaCookiesOut)
{
  HRESULT hResult = S_OK;
  int     i = 0;
  int     index;

  if (pCaStringsOut == NULL || pCaCookiesOut == NULL)
    return E_POINTER;

  index = fpAtlGetPinfoIndex(dispid);
  if (index <= -1 || !m_PropList)
    return S_FALSE;

  pCaStringsOut->cElems = 0;
  pCaStringsOut->pElems = NULL;
  pCaCookiesOut->cElems = 0;
  pCaCookiesOut->pElems = NULL;

  LPFP_PROPLIST pPropEntry = &m_PropList[index];
  PFP_PROPINFO pPropInfo = pPropEntry->pPropInfo;

  // if data type is ENUM and there is a list, copy strings from list.
  if ((ULONG)pPropInfo < (ULONG)PPROPINFO_STD_LAST &&
      !(pPropEntry->wEnvironment & FP_NO_OCX) &&
      (pPropInfo->fl & PF_datatype) == DT_ENUM &&
      pPropInfo->npszEnumList)
  {
    LPTSTR tstrEnumString = pPropInfo->npszEnumList;
    int    enumMax = pPropInfo->enumMax;
    LPOLESTR lpOleStr;

    // alloc memory for lists
    pCaStringsOut->pElems = (LPOLESTR*)CoTaskMemAlloc(
		sizeof(LPOLESTR) * (enumMax+1));
    if (pCaStringsOut->pElems == NULL)
      hResult = E_OUTOFMEMORY;
    else
    {
	  pCaCookiesOut->pElems = (DWORD*)CoTaskMemAlloc(
		sizeof(DWORD*) * (enumMax+1));
      if (pCaCookiesOut->pElems == NULL)
        hResult = E_OUTOFMEMORY;
      else
      {
        pCaStringsOut->cElems = enumMax+1;
        pCaCookiesOut->cElems = enumMax+1;
      }
    }

    // loop thru strings creating copies and putting in list
    while (i <= enumMax && hResult == S_OK && *tstrEnumString)
    {
      lpOleStr = fpAtlCoTaskCopyString(tstrEnumString);
      if (lpOleStr == NULL)
        hResult = E_OUTOFMEMORY;
      else
      {
        pCaStringsOut->pElems[i] = lpOleStr;
        pCaCookiesOut->pElems[i] = (DWORD)i;
        tstrEnumString += lstrlen(tstrEnumString) + 1; // move lpsz to next string
        ++i;
      }
    }
  }

  // If any errors, free any strings which were allocated.
  if (hResult == E_OUTOFMEMORY)
  {
    if (pCaStringsOut->pElems)
    {
      while (--i >= 0)
        CoTaskMemFree(pCaStringsOut->pElems[i]);
      CoTaskMemFree(pCaStringsOut->pElems);
    }
    if (pCaCookiesOut->pElems)
      CoTaskMemFree(pCaCookiesOut->pElems);
  }
  

  return hResult;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlGetDisplayString() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function is used to override the implementation of the 
 *   interface function: IPerPropertyBrowsing::GetDisplayString().
 *   This gets the current value (in string format) of a property.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlGetDisplayString(DISPID dispID, BSTR *pBstr)
{
  HRESULT hResult = S_FALSE;
  int     index;
  CComVariant var;
  LPFP_PROPLIST pPropEntry;
  PFP_PROPINFO pPropInfo;

  *pBstr = NULL;
  index = fpAtlGetPinfoIndex(dispID);
  if (index == -1 || !m_PropList)
    return hResult; // this will cause the default function to be called.

  pPropEntry = &m_PropList[index];
  pPropInfo = pPropEntry->pPropInfo;

  // if data type is ENUM and there is a list...
  if ((ULONG)pPropInfo < (ULONG)PPROPINFO_STD_LAST &&
      !(pPropEntry->wEnvironment & FP_NO_OCX) &&
	  !(pPropInfo->fl & PF_fNoShow) &&
      (pPropInfo->fl & PF_datatype) == DT_ENUM &&
      pPropInfo->npszEnumList)
  {
    int i;
    LPTSTR tstrEnumString = pPropInfo->npszEnumList;
    HRESULT h = fpAtlGetProperty(dispID, &var);
    if (h != S_OK || (var.vt != VT_I2 && var.vt != VT_I4) || var.iVal < 0 ||
        var.iVal > (short)pPropInfo->enumMax)
    {
      _ASSERT(0);  // one of the above conditions should not be!
	  *pBstr = NULL;
	  return S_FALSE;
    }

    // get string for value
    for (i = 0; i < var.iVal; ++i)
      tstrEnumString += lstrlen(tstrEnumString) + 1; // move lpsz to next string

    *pBstr = (BSTR)fpSysAllocBSTRFromTString(tstrEnumString);
    hResult = S_OK;
  }

  return hResult;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlGetPredefinedValue() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function is used to override the implementation of the 
 *   interface function: IPerPropertyBrowsing::GetPredefinedValue().
 *   This gets the corresponding property value for a cookie.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlGetPredefinedValue(DISPID dispID, DWORD dwCookie,
  VARIANT* pVarOut)
{
  HRESULT hResult = E_FAIL;
  int     index;
  LPFP_PROPLIST pPropEntry;
  PFP_PROPINFO pPropInfo;

  index = fpAtlGetPinfoIndex(dispID);
  if (index == -1 || !m_PropList)
    return hResult; // this will cause the default function to be called.

  pPropEntry = &m_PropList[index];
  pPropInfo = pPropEntry->pPropInfo;

  // if data type is ENUM and there is a list...
  if ((ULONG)pPropInfo < (ULONG)PPROPINFO_STD_LAST &&
      !(pPropEntry->wEnvironment & FP_NO_OCX) &&
      (pPropInfo->fl & PF_datatype) == DT_ENUM &&
      pPropInfo->npszEnumList)
  {
    if ((int)dwCookie < 0 || (int)dwCookie > (int)pPropInfo->enumMax)
    {
      _ASSERT(0);  // one of the above conditions should not be!
	  return E_INVALIDARG;
    }

    // else, value in cookie is the actual value, put in VARIANT
    pVarOut->iVal = (short)dwCookie;
    pVarOut->vt = VT_I2;
    hResult = S_OK;
  }

  return hResult;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlMapPropertyToPage() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function is used to override the implementation of the 
 *   interface function: IPerPropertyBrowsing::MapPropertyToPage().
 *   This gets the corresponding property page for a property.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlMapPropertyToPage(DISPID dispID, CLSID *pClsid,
  ATL_PROPMAP_ENTRY* pMap)
{
    if (!pMap)
      return E_INVALIDARG;

	for(int i = 0; pMap[i].pclsidPropPage != NULL; i++)
	{
		if (pMap[i].szDesc == NULL)
			continue;
		if (pMap[i].dispid == dispID)
		{
			_ASSERTE(pMap[i].pclsidPropPage != NULL);
			*pClsid = *(pMap[i].pclsidPropPage);
            if (pMap[i].pclsidPropPage == &CLSID_NULL)
              return PERPROP_E_NOPAGEAVAILABLE;
            else
			  return S_OK;
		}
	}
	*pClsid = CLSID_NULL;
	return E_INVALIDARG;
}



/***************************************************************************
 *
 * FUNCTION:  fpAtlPropBag_LoadVerAndExtent() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function loads the version and extents (width & height) of
 *   the control from the PropertyBag.  These elements are read
 *   first because the version can decide which properties to load.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlPropBag_LoadVerAndExtent(
  LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, 
  DWORD *pdwRetCtlVersion, SIZE *psizeRetExtent)
{
	USES_CONVERSION;
    HRESULT hr;
	CComVariant var;
    LPCOLESTR lpOleStr;

    // read Version from PropertyBag
    for (int i = 0; i < 3; i++)  // loop for 3 values
    {
      switch (i)  // specify "property" name
      {
        case 0: lpOleStr = OLESTR("_Version"); break;
        case 1: lpOleStr = OLESTR("_ExtentX"); break;
        case 2: lpOleStr = OLESTR("_ExtentY"); break;
      }
      var.vt = VT_I4;
	  hr = pPropBag->Read(lpOleStr, &var, pErrorLog);
	  if (FAILED(hr))
	  {
		if (hr == E_INVALIDARG)
		{
		  ATLTRACE(_T("Property %s not in Bag\n"), OLE2CT(lpOleStr));
		}
		else
		{
		  // Many containers return different ERROR values for Member not found
		  ATLTRACE(_T("Error attempting to read Property %s from PropertyBag \n"), OLE2CT(lpOleStr));
		}
		continue;
	  }

      switch (i)  // copy value from "var" to "return parameter".
      {
        case 0: *pdwRetCtlVersion = (DWORD)var.lVal;  break;
        case 1: psizeRetExtent->cx = var.lVal;  break;
        case 2: psizeRetExtent->cy = var.lVal;  break;
      }
	}

	return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlPropBag_SaveVerAndExtent() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function saves the version and extents (width & height) of
 *   the control to the PropertyBag.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlPropBag_SaveVerAndExtent(
  LPPROPERTYBAG pPropBag, DWORD dwCtlVersion, SIZE *psizeExtent)
{
    HRESULT hr;
	CComVariant var;
    LPCOLESTR lpOleStr;

    var.vt = VT_I4;
    // read Version from PropertyBag
    for (int i = 0; i < 3; i++)  // loop for 3 values
    {
      switch (i)  // specify "property" name
      {
        case 0: 
          lpOleStr = OLESTR("_Version");
          var.lVal = (long)dwCtlVersion;
          break;
        case 1:
          lpOleStr = OLESTR("_ExtentX");
          var.lVal = psizeExtent->cx;
          break;
        case 2:
          lpOleStr = OLESTR("_ExtentY");
          var.lVal = psizeExtent->cy;
          break;
      }
	  hr = pPropBag->Write(lpOleStr, &var);
	  if (FAILED(hr))
	    return hr;
	}

	return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlLoadTofpStream() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This functions read the information from either a PropertyBag or
 *   IStream and copies it to a newly created IStream (in dispid/value
 *   pairs).  This new IStream is kept around until the control has
 *   a HWND, then the properties are read from the IStream and set 
 *   in the control.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlLoadTofpStream(CComControlBase *pComCtlBase,
  CComDispatchDriver *pComDispDriver, LPSTREAM pStm,
  LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, ATL_PROPMAP_ENTRY* pMap)
{
    HRESULT hr;

    // If we have a stream, reset it.
    if (m_pStmLoad)
    {
      LARGE_INTEGER li;

      LISet32(li, 0);
      hr = m_pStmLoad->Seek(li, STREAM_SEEK_SET, NULL);
      _ASSERT(hr == S_OK);
      if (hr != S_OK)
      {
        m_pStmLoad->Release();
        m_pStmLoad = NULL;
      }
    }
    // if no stream, create one.
    if (!m_pStmLoad)
    {
      hr = CreateStreamOnHGlobal(NULL, TRUE, &m_pStmLoad);
      _ASSERT(hr == S_OK);
      if (hr != S_OK)
        return E_OUTOFMEMORY;
    }


    // if loading from property bag, call that function, else
    // call function for loading from a stream.
    if (pPropBag)
      hr = fpAtlLoadPropBagTofpStream(pComCtlBase, pComDispDriver,
        pPropBag, pErrorLog, pMap, m_pStmLoad);
    else
      hr = fpAtlCopyPropStream(m_pStmLoad, pStm);

    // if successful, write 0xFFFF as index for "terminating" map index.
    if (hr == S_OK)
    {
      SHORT dispid = FPATL_STM_NULLPROP;
      hr = m_pStmLoad->Write(&dispid, sizeof(SHORT), NULL); // write index
    }

	return hr;
}



/***************************************************************************
 *
 * FUNCTION:  fpAtlLoadPropBagTofpStream() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This functions reads the information from a PropertyBag and 
 *   copies it to an IStream (in dispid/value pairs).
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlLoadPropBagTofpStream(
  CComControlBase *pComCtlBase, CComDispatchDriver *pComDispDriver, 
  LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, ATL_PROPMAP_ENTRY* pMap,
  LPSTREAM pStmLoad)
{
	USES_CONVERSION;
    HRESULT hr;
	CComPtr<IDispatch> pDispatch;
	const IID* piidOld = NULL;
    SHORT dispid;
	CComVariant var;
    HGLOBAL hBlob = NULL;
    int iPropId = -1;
    LPCTSTR tstrBlobProp = NULL;

    // For each property in PropMap, read value from bag and
    // write to Stream.
	for(int i = 0; pMap[i].pclsidPropPage != NULL; i++)
	{
		if (pMap[i].szDesc == NULL)
		  continue;

		if(pMap[i].piidDispatch != piidOld)
		{
		  pDispatch.Release();
		  if(FAILED(pComCtlBase->ControlQueryInterface(*pMap[i].piidDispatch, (void**)&pDispatch)))
		  {
		    ATLTRACE(_T("Failed to get a dispatch pointer for property #%i\n"), i);
			return E_FAIL;
		  }
		  piidOld = pMap[i].piidDispatch;
		}

        // Insure that this control supports this property.
		// (unless this is a blob property) -scl
		if (FPATL_STM_BLOBPROP != pMap[i].dispid)
		{
		   var.Clear(); // RFW - 4/30/04 - 9079
			if (FAILED(pComDispDriver->GetProperty(pDispatch, pMap[i].dispid, &var)))
			{
			  ATLTRACE(_T("Invoked failed on DISPID %x\n"), pMap[i].dispid);
			  return E_FAIL;
			}
			// Empty anything that was put in "var" by GetProperty(),
			// but keep the type (vt) which is used below by Read().
			if (var.lVal)
			{
			  VARTYPE vt = var.vt;  // save vt
			  var.Clear();          // clear value
			  var.lVal = 0;
			  var.vt = vt;          // restore vt.
			}

			// VB 4.0 does not recognize certain VT types, so we must
			// translates those VT types to ones it knows.
			var.vt = fpAtlTranslateVTtype(var.vt); // restore translated type.

			// read value from PropertyBag
			hr = pPropBag->Read(pMap[i].szDesc, &var, pErrorLog);
			if (FAILED(hr))
			{
			  if (hr == E_INVALIDARG)
			  {
				ATLTRACE(_T("Property %s not in Bag\n"), OLE2CT(pMap[i].szDesc));
			  }
			  else
			  {
				// Many containers return different ERROR values for Member not found
				ATLTRACE(_T("Error attempting to read Property %s from PropertyBag \n"), OLE2CT(pMap[i].szDesc));
			  }
			  hr = S_OK;
			  continue;
			}

			// Since we don't have a hWnd, write property value to stream.
			dispid = (SHORT)pMap[i].dispid;
			hr = pStmLoad->Write(&dispid, sizeof(SHORT), NULL); // write index
			if (FAILED(hr))
			  break;
			hr = var.WriteToStream(pStmLoad);
			if (FAILED(hr))
			  break;
		}
	}

    // if successful and control uses blob data, get data and write to
    // stream.
    if (hr == S_OK && fpAtlHasBlobData(&iPropId, &tstrBlobProp))
    {
      hr = fpAtlLoadBlob(pPropBag, pErrorLog, &hBlob, tstrBlobProp);

      if (hr == S_OK && hBlob)
      {
        dispid = FPATL_STM_BLOBPROP;
        hr = pStmLoad->Write(&dispid, sizeof(SHORT), NULL); // write index
        if (hr == S_OK)
          fpAtlSaveBlob(pStmLoad, hBlob);
      }
      if (hBlob)
        GlobalFree(hBlob);

      hr = S_OK;
    }
    
	return hr;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlCopyPropStream() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This functions reads the information from an IStream and 
 *   copies it to a different IStream (in dispid/value pairs).
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlCopyPropStream( LPSTREAM pStmDest, LPSTREAM pStmSrc,
  BOOL fCopyNullProp)
{
    HRESULT hr;
    SHORT dispid;
	CComVariant var;
    HGLOBAL hBlob = NULL;
    int iPropId = -1;
    int iBlobPropId;

    // For each property in PropMap, read value from bag/stream and
    // write to Stream.
    // For each property in the stream, read value from Stream and set in control.
    do 
    {
      hr = pStmSrc->Read(&dispid, sizeof(SHORT), NULL); // read "map" index
	  if (FAILED(hr))
		break;

      // if this is a blob prop...
      if (dispid == FPATL_STM_BLOBPROP)  
      {
        HGLOBAL hBlob = NULL;

        // read blob from stream
        hr = fpAtlLoadBlob(pStmSrc, &hBlob);
        // if control actually uses blob data then set those properties.
        if (hr == S_OK && fpAtlHasBlobData(&iBlobPropId, NULL))
        {
          hr = pStmDest->Write(&dispid, sizeof(SHORT), NULL); // write index
          if (hr == S_OK)
            fpAtlSaveBlob(pStmDest, hBlob);

          hr = S_OK;
        }
        if (hBlob)
          GlobalFree(hBlob);

        continue;  // loop to next property
      }
	  else if (dispid == FPATL_STM_NULLPROP)
      {
        // if flag is set, copy null prop to pStmDest stream.
        if (fCopyNullProp)
          hr = pStmDest->Write(&dispid, sizeof(SHORT), NULL); // write index
		break;  // end loop
      }

      // read value from stream.
	  hr = var.ReadFromStream(pStmSrc);
      if (FAILED(hr))
		break;
      // write Map index to stream and then property value.
      hr = pStmDest->Write(&dispid, sizeof(SHORT), NULL); // write index
	  if (FAILED(hr))
		break;
      hr = var.WriteToStream(pStmDest); // write prop value
      var.Clear(); // RFW - 3/24/05 - 15949
	  if (FAILED(hr))
		break;

	} while (dispid != FPATL_STM_NULLPROP && hr == S_OK);

	return hr;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlLoadFromfpStream() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function loads the property values saved in a IStream (which
 *   was created by the control during the Load).  This function is
 *   called by the process which creates the HWND for the control, 
 *   AFTER the creation of the HWND.  It "loads" the prop values 
 *   by reading the Dispid/Value pairs from the IStream
 *   and setting the values in the control.  
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlLoadFromfpStream(CComControlBase *pComCtlBase,
  CComDispatchDriver *pComDispDriver, ATL_PROPMAP_ENTRY* pMap)
{
	USES_CONVERSION;
    HRESULT hr;
	CComPtr<IDispatch> pDispatch;
	const IID* piidOld = NULL;
    SHORT dispid;
    CComVariant var;

    // If we don't have a stream, or if we cannot reset it, return.
    if (!m_pStmLoad)
    {
      _ASSERT(0);
      return S_OK;
    }
    else
    {
      LARGE_INTEGER li;
      LISet32(li, 0);
      hr = m_pStmLoad->Seek(li, STREAM_SEEK_SET, NULL);
      _ASSERT(hr == S_OK);
      if (hr != S_OK)
        return S_OK;
    }

    // For each property in the stream, read value from Stream and set in control.
    do 
    {
      hr = m_pStmLoad->Read(&dispid, sizeof(SHORT), NULL); // read "map" index
	  if (FAILED(hr))
		break;

      // if this is a blob prop...
      if (dispid == FPATL_STM_BLOBPROP)  
      {
        HGLOBAL hBlob = NULL;
        int iPropId = -1;
        // read blob from stream
        hr = fpAtlLoadBlob(m_pStmLoad, &hBlob);
        // if control actually uses blob data then set those properties.
        if (hr == S_OK && fpAtlHasBlobData(&iPropId, NULL))
        {
          fpAtlSetBlobData(hBlob, iPropId);
        }
        if (hBlob)
          GlobalFree(hBlob);

        continue;  // loop to next property
      }
	  else if (dispid == FPATL_STM_NULLPROP)
		break; // end loop

	  // read value from our stream.
	  hr = var.ReadFromStream(m_pStmLoad);
	  if (FAILED(hr))
	  {
		if (hr == E_INVALIDARG)
		{
		  ATLTRACE(_T("Property (DISPID %d) not in Bag\n"), (int)dispid);
		}
		else
		{
		  // Many containers return different ERROR values for Member not found
		  ATLTRACE(_T("Error attempting to read Property (DISPID %d) from PropertyBag \n"), (int)dispid);
		}
		continue;
	  }

	  if(&IID_IDispatch != piidOld)
	  {
		if(FAILED(pComCtlBase->ControlQueryInterface(IID_IDispatch, (void**)&pDispatch)))
		{
		  ATLTRACE(_T("Failed to get a dispatch pointer for property (DISPID %d)\n"), (int)dispid);
		  hr = E_FAIL;
		  break;
		}
		piidOld = &IID_IDispatch;
	  }
      // Set property value in control
   	  if (FAILED(pComDispDriver->PutProperty(pDispatch, (DISPID)dispid, &var)))
	  {
		ATLTRACE(_T("Invoked failed on DISPID %d\n"), (int)dispid);
		hr = E_FAIL;
		break;
	  }
     var.Clear(); // RFW - 3/24/05 - 15949

	} while (dispid != FPATL_STM_NULLPROP && hr == S_OK);

//    fpAtlSetModifiedFlag(FALSE);
	return hr;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlPropBag_Save() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function writes the property values to the PropertyBag.
 *   It writes-out the version number and extents (width & height),
 *   loops thru the list of persist props and saves each one to
 *   the PropertyBag.  It also saves the BLOB property as the last
 *   property.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlPropBag_Save(CComControlBase *pComCtlBase,
  CComDispatchDriver *pComDispDriver, LPPROPERTYBAG pPropBag,
  BOOL fClearDirty, BOOL fSaveAllProperties, ATL_PROPMAP_ENTRY* pMap,
  DWORD dwCtlVersion, SIZE *psizeExtent)
{
    VARTYPE vtSave;
    HRESULT hr;
    HGLOBAL hBlob = NULL;
    int iPropId = -1;
    LPCTSTR tstrPropName = NULL;

	if (pPropBag == NULL)
	{
		ATLTRACE(_T("PropBag pointer passed in was invalid\n"));
		return E_POINTER;
	}

    hr = fpAtlPropBag_SaveVerAndExtent(pPropBag, dwCtlVersion, 
                                       psizeExtent);
    if (hr != S_OK)
      return hr;

    // If no HWND, then get property values from "m_pStmLoad".
    if (fpAtlGetHWnd() == NULL)
    {
      if (m_pStmLoad)  // if stream, call func to save props to PropertyBag.
        hr = fpAtlPropBagSaveFromfpStream(pPropBag, pMap);
      if (fClearDirty)
	    pComCtlBase->m_bRequiresSave = FALSE;
      return hr;       // return results.
    }

	CComPtr<IDispatch> pDispatch;
	const IID* piidOld = NULL;
	for(int i = 0; pMap[i].pclsidPropPage != NULL; i++)
	{
		if (pMap[i].szDesc == NULL)
			continue;
		CComVariant var;

		if(pMap[i].piidDispatch != piidOld)
		{
			pDispatch.Release();
			if(FAILED(pComCtlBase->ControlQueryInterface(*pMap[i].piidDispatch, (void**)&pDispatch)))
			{
				ATLTRACE(_T("Failed to get a dispatch pointer for property #%i\n"), i);
				return E_FAIL;
			}
			piidOld = pMap[i].piidDispatch;
		}

		if (FAILED(pComDispDriver->GetProperty(pDispatch, pMap[i].dispid, &var)))
		{
			ATLTRACE(_T("Invoked failed on DISPID %x\n"), pMap[i].dispid);
			return E_FAIL;
		}

		if (var.vt == VT_UNKNOWN || var.vt == VT_DISPATCH)
		{
			if (var.punkVal == NULL)
			{
				ATLTRACE(_T("Warning skipping empty IUnknown in Save\n"));
				continue;
			}
		}

        // VB 4.0 does not recognize certain VT types, so we must
        // translates those VT types to ones it knows.
        vtSave = var.vt;  // save vt type
        var.vt = fpAtlTranslateVTtype(var.vt);

		HRESULT hr = pPropBag->Write(pMap[i].szDesc, &var);
        var.vt = vtSave;  // restore vt type
		if (FAILED(hr))
			return hr;
	}

    // if control uses blob data, save it to property bag.
    if (hr == S_OK && fpAtlHasBlobData(&iPropId, &tstrPropName) && 
        fpAtlGetBlobData(&hBlob, iPropId))
    {
// fix for container that don't support blob props in a property bag -scl
//      fpAtlSaveBlob(pPropBag, hBlob, tstrPropName);
      HRESULT hr = fpAtlSaveBlob(pPropBag, hBlob, tstrPropName);
	  // TESTING VINTERDEV -SCL
	  // hr = E_FAIL;
	  // TESTING VINTERDEV -SCL
      if (hBlob)
        GlobalFree(hBlob);
	  if (hr)
		return E_FAIL;
// fix for container that don't support blob props in a property bag -scl
    }

    if (fClearDirty)
	  pComCtlBase->m_bRequiresSave = FALSE;
	return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlPropBagSaveFromfpStream() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This functions saves the properties stored in a Stream (FP's format)
 *   to the given PropertyBag.  This is used when the control is told
 *   to save its properties to a PropertyBag but no HWND has been created
 *   for the control (so we get the prop values from the temp stream).
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlPropBagSaveFromfpStream(LPPROPERTYBAG pPropBag,
  ATL_PROPMAP_ENTRY* pMap)
{
    HRESULT hr;
    SHORT dispid;
    CComVariant var;
    HGLOBAL hBlob = NULL;
    int iPropId = -1;
    int i;
    BOOL fFound;
    LPCTSTR tstrPropName = NULL;

    // If we don't have a stream, or if we cannot reset it, return.
    if (!m_pStmLoad)
    {
      _ASSERT(0);
      return S_OK;
    }
    else
    {
      LARGE_INTEGER li;
      LISet32(li, 0);
      hr = m_pStmLoad->Seek(li, STREAM_SEEK_SET, NULL);
      _ASSERT(hr == S_OK);
      if (hr != S_OK)
        return S_OK;
    }

    // For each property in the stream, read value from Stream and 
    // set in PropertyBag.
    do 
    {
      hr = m_pStmLoad->Read(&dispid, sizeof(SHORT), NULL); // read "map" index
	  if (FAILED(hr))
		break;

      // if this is a blob prop...
      if (dispid == FPATL_STM_BLOBPROP)  
      {
        // read blob from stream
        hr = fpAtlLoadBlob(m_pStmLoad, &hBlob);
        continue;  // loop to next property
      }
	  else if (dispid == FPATL_STM_NULLPROP)
		break; // end loop

	  // read value from our stream.
	  hr = var.ReadFromStream(m_pStmLoad);
	  if (FAILED(hr))
	  {
		if (hr == E_INVALIDARG)
		{
		  ATLTRACE(_T("Property (DISPID %d) not in Bag\n"), (int)dispid);
		}
		else
		{
		  // Many containers return different ERROR values for Member not found
		  ATLTRACE(_T("Error attempting to read Property (DISPID %d) from PropertyBag \n"), (int)dispid);
		}
		continue;
	  }

      // Find Map entry for this dispid
      fFound = FALSE;
      i = 0;
      while (!fFound && pMap[i].pclsidPropPage != NULL)
      {
        if (pMap[i].dispid == dispid)
          fFound = TRUE;
        else
          ++i;
      }

      // If map entry was found, write prop value to Bag.
      _ASSERT(fFound && pMap[i].szDesc);
      if (fFound && pMap[i].szDesc)
      {
        // VB 4.0 does not recognize certain VT types, so we must
        // translates those VT types to ones it knows.
        VARTYPE vtSave = var.vt;  // save vt type
        var.vt = fpAtlTranslateVTtype(var.vt);
	    hr = pPropBag->Write(pMap[i].szDesc, &var);
        var.vt = vtSave;  // restore vt type
      }

	} while (dispid != FPATL_STM_NULLPROP && hr == S_OK);

    // if control uses blob data, save it to property bag.
    if (hr == S_OK && fpAtlHasBlobData(&iPropId, &tstrPropName))
      fpAtlSaveBlob(pPropBag, hBlob, tstrPropName);

    // Free blob if allocated
    if (hBlob)
      GlobalFree(hBlob);

	return hr;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlStream_LoadVerAndExtent() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This functions saves the version number and extents (width & height)
 *   to the IStream.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlStream_LoadVerAndExtent(LPSTREAM pStm, 
  DWORD *pdwRetCtlVersion, SIZE *psizeRetExtent)
{
	HRESULT hr = pStm->Read(pdwRetCtlVersion, sizeof(DWORD), NULL);
	if (SUCCEEDED(hr))
	  hr = pStm->Read(psizeRetExtent, sizeof(SIZE), NULL);

	return hr;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlStream_Save() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function writes the property values to the IStream.
 *   It writes-out the version number and extents (width & height),
 *   loops thru the list of persist props and saves each one to
 *   the IStream (as Dispid/Value pairs).  It also saves the 
 *   BLOB property as the last property and writes-out a "NULL" dispid
 *   to indicate the end of the property values.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlStream_Save(CComControlBase *pComCtlBase,
  CComDispatchDriver *pComDispDriver, LPSTREAM pStm, 
  BOOL fClearDirty, ATL_PROPMAP_ENTRY* pMap,
  DWORD dwCtlVersion, SIZE *psizeExtent)
{
    SHORT dispid;
    HGLOBAL hBlob = NULL;
    int iPropId = -1;
    LPCTSTR tstrPropName = NULL;

	_ASSERTE(pMap != NULL);
	HRESULT hr = 
      pStm->Write(&dwCtlVersion, sizeof(DWORD), NULL); // write version
	if (FAILED(hr))
	  return hr;
	hr = pStm->Write(psizeExtent, sizeof(SIZE), NULL); // write extent
	if (FAILED(hr))
	  return hr;

    // If the control has data in the "Load stream", then get the 
    // properties values from that stream instead of from the control.
    //
    // NOTE: If "m_pStmLoad" has data, it's because the HWND for the
    // control has not been created yet; therefore, we cannot get the
    // properties values via "GetProperty()".
    if (m_pStmLoad)
    {
      LARGE_INTEGER li;
      LISet32(li, 0);
      hr = m_pStmLoad->Seek(li, STREAM_SEEK_SET, NULL);
      _ASSERT(hr == S_OK);
      if (SUCCEEDED(hr))
        hr = fpAtlCopyPropStream(pStm, m_pStmLoad, TRUE); // TRUE = include "NULL terminator"
	  if (SUCCEEDED(hr) && fClearDirty)
	    pComCtlBase->m_bRequiresSave = FALSE;
      return hr;  // RETURN here.
    }

    // "Else", get properties from the control and put them in the stream.    
	CComPtr<IDispatch> pDispatch;
	const IID* piidOld = NULL;
	for(int i = 0; pMap[i].pclsidPropPage != NULL; i++)
	{
	  if (pMap[i].szDesc == NULL)
		continue;
	  CComVariant var;

	  if(pMap[i].piidDispatch != piidOld)
	  {
		if(FAILED(pComCtlBase->ControlQueryInterface(*pMap[i].piidDispatch, (void**)&pDispatch)))
		{
		  ATLTRACE(_T("Failed to get a dispatch pointer for property #%i\n"), i);
		  hr = E_FAIL;
		  break;
		}
		piidOld = pMap[i].piidDispatch;
	  }

	  if (FAILED(pComDispDriver->GetProperty(pDispatch, pMap[i].dispid, &var)))
	  {
		ATLTRACE(_T("Invoked failed on DISPID %x\n"), pMap[i].dispid);
		hr = E_FAIL;
		break;
	  }

      dispid = (SHORT)pMap[i].dispid;
      hr = pStm->Write(&dispid, sizeof(SHORT), NULL); // write index
	  if (FAILED(hr))
		break;

	  hr = var.WriteToStream(pStm);  // write prop value
     var.Clear(); // RFW - 3/24/05 - 15949
	  if (FAILED(hr))
		break;
	}

    // Write-out the blob data if there is any.
    if (hr == S_OK && fpAtlHasBlobData(&iPropId, &tstrPropName) && 
        fpAtlGetBlobData(&hBlob, iPropId))
    {
      SHORT sdispid = FPATL_STM_BLOBPROP;
      hr = pStm->Write(&sdispid, sizeof(SHORT), NULL); // write index

      fpAtlSaveBlob(pStm, hBlob);
      if (hBlob)
        GlobalFree(hBlob);
    }

    // if successful, write 0xFFFF as index for "terminating" map index.
    if (hr == S_OK)
    {
      SHORT sdispid = FPATL_STM_NULLPROP;
      hr = pStm->Write(&sdispid, sizeof(SHORT), NULL); // write index
    }

	if (SUCCEEDED(hr) && fClearDirty)
	  pComCtlBase->m_bRequiresSave = FALSE;

	return hr;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlGetBlobData() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This is called when the control is being saved.  This may be
 *   overridden by the subclass to put data in the blob and return TRUE.
 *
 * PARAMETERS:
 *
 * RETURNS:  TRUE.
 *
 **************************************************************************/
BOOL CfpAtlBase::fpAtlGetBlobData(HGLOBAL *lphBlob, int iPropId)
{
  if (fpAtlGetHWnd())
  {  
    *lphBlob = 
     (HGLOBAL)SendMessage(fpAtlGetHWnd(), FPM_SAVEPROPERTY, 
       (WPARAM)iPropId, 0);
  }

  return TRUE;
}
 
/***************************************************************************
 *
 * FUNCTION:  fpAtlSetBlobData() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This is called when the control is being load.  This may be
 *   overridden by the subclass to handle the blob data during a load.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
void CfpAtlBase::fpAtlSetBlobData(HGLOBAL hBlob, int iPropId)
{
  SendMessage(fpAtlGetHWnd(), FPM_LOADPROPERTY, (WPARAM)iPropId,
    (LPARAM)hBlob);

  return;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlSaveBlob() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function saves the passed BLOB to the passed IStream.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlSaveBlob(LPSTREAM pStm, HGLOBAL hBlob)
{
  HRESULT h;
  LPBLOB lpBlob = (LPBLOB)(hBlob ? GlobalLock(hBlob) : NULL);
  ULONG  len = (lpBlob ? lpBlob->cbSize : 0);  // use size or 0

  _ASSERT(!(hBlob != 0 && lpBlob == NULL)); // failed to lock.

  // Write LEN and, if LEN > 0, the data to the stream.
  h = pStm->Write(&len, sizeof(len), NULL);  // write len to stream.
  if (h == S_OK && len > 0)
    h = pStm->Write(&lpBlob->pBlobData, len, NULL);

  if (lpBlob)
    GlobalUnlock(hBlob);

  return h;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlSaveBlob() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function saves the passed BLOB to the passed PropertyBag.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlSaveBlob(LPPROPERTYBAG pPropBag, 
  HGLOBAL hBlob, LPCTSTR tstrPropName)
{
  HRESULT hr = S_OK;
  LPBLOB lpBlob = (LPBLOB)(hBlob ? GlobalLock(hBlob) : NULL);

  _ASSERT(!(hBlob != 0 && lpBlob == NULL)); // failed to lock.

  // the data to the prop bag.
  if (lpBlob && tstrPropName)
  {
    CComVariant var;
    CfpBlobObject *blobProp = new CfpBlobObject(hBlob);

    hr = blobProp->QueryInterface(IID_IUnknown, (void **)&var.punkVal);
    if (hr == S_OK)
    {
      BSTR bstr = (BSTR)fpSysAllocBSTRFromTString((LPTSTR)tstrPropName);

      var.vt = VT_UNKNOWN;
      if (!bstr)
        hr = E_OUTOFMEMORY;
      else  
      {
        hr = pPropBag->Write(bstr, &var);
        fpSysFreeTString((LPTSTR)bstr);
      }
    }
	blobProp->Release();
    GlobalUnlock(hBlob);
  }

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlLoadBlob() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function load a BLOB from the passed IStream.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlLoadBlob(LPSTREAM pStm, HGLOBAL *lphBlob)
{
  HRESULT h;
  LPBLOB lpBlob;
  ULONG  len = 0;

  *lphBlob = NULL;  // zero-out the handle.

  // Read len. If error or len is zero, return "h".
  h = pStm->Read(&len, sizeof(len), NULL);  // read len from stream.
  if (h != S_OK || len == 0)
    return h;

  // Read the data into an allocated blob.
  *lphBlob = GlobalAlloc(GMEM_MOVEABLE, len + sizeof(ULONG));
  if (*lphBlob == NULL)
    return CTL_E_OUTOFMEMORY;

  lpBlob = (LPBLOB)GlobalLock(*lphBlob);
  _ASSERT(lpBlob);
  if (!lpBlob)                    // if Lock failed, return error
    h = CTL_E_OUTOFMEMORY;
  else                            // else proceed to retrieve the data.
  {
    lpBlob->cbSize = len;
    // Read data into blob
    h = pStm->Read(&lpBlob->pBlobData, len, NULL);
    GlobalUnlock(*lphBlob);
  }

  // if something failed and we have a Global handle, free the handle.
  if (h != S_OK && *lphBlob)
  {
    GlobalFree(*lphBlob);
    *lphBlob = NULL;
  }

  return h;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlLoadBlob() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function load a BLOB from the passed PropertyBag.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code
 *
 **************************************************************************/
HRESULT CfpAtlBase::fpAtlLoadBlob(LPPROPERTYBAG pPropBag,
  LPERRORLOG pErrorLog, HGLOBAL *lphBlob, LPCTSTR tstrPropName)
{
  HRESULT hr = S_OK;

  *lphBlob = NULL;  // zero-out the handle.
  _ASSERT(tstrPropName);

  if (tstrPropName)
  {
    CComVariant var;
    CfpBlobObject blobProp;

    hr = blobProp.QueryInterface(IID_IUnknown, (void **)&var.punkVal);
    if (hr == S_OK)
    {
      BSTR bstr = (BSTR)fpSysAllocBSTRFromTString((LPTSTR)tstrPropName);
      var.vt = VT_UNKNOWN;
      if (!bstr)
        hr = E_OUTOFMEMORY;
      else  
      {
        hr = pPropBag->Read(bstr, &var, pErrorLog);
        if (hr == S_OK)
          *lphBlob = blobProp.GetBlob();
		else	// if CfpBlobObject fails, try CfpBlobObjectOld -scl
		{
			CfpBlobObjectOld blobPropOld;

			hr = blobPropOld.QueryInterface(IID_IUnknown, (void **)&var.punkVal);
			if (S_OK == hr)
				hr = pPropBag->Read(bstr, &var, pErrorLog);
			if (S_OK == hr)
				*lphBlob = blobPropOld.GetBlob();
		}
        fpSysFreeTString((LPTSTR)bstr);
      }
    }
  }

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlGetExtendedControl() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function gets the "ExtendedControl" dispatch interface from
 *   the ClientSite.  
 * 
 *   NOTE:  If a ptr is returned, it MUST BE RELEASED by the caller 
 *          using "ptr->Release()".
 *
 * PARAMETERS:
 *
 * RETURNS:  NULL if not extended control, otherwise a LPDISPATCH ptr.
 *
 **************************************************************************/
LPDISPATCH CfpAtlBase::fpAtlGetExtendedControl()
{
  IOleClientSite *pOleClientSite = fpAtlGetClientSite();
  IOleControlSite *pOleControlSite;
  LPDISPATCH lpDisp = NULL;
  if (pOleClientSite &&
        NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
  { 
     pOleControlSite->GetExtendedControl(&lpDisp);
     pOleControlSite->Release();
  }

  if (pOleClientSite)
    pOleClientSite->Release();

  return lpDisp;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlEnumOleVerbs()
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function builds a list of verbs in the required format from
 *   a passed list of custom verbs.
 *
 *   NOTE:  The ability to specify a ResId instead of a Verb Name string
 *          is not currently supported.  If this is needed, YOU may ADD
 *          support for it in this function!
 *
 * PARAMETERS:
 *
 * RETURNS:  TRUE if successful, else FALSE.
 *
 **************************************************************************/
BOOL CfpAtlBase::fpAtlEnumOleVerbs(const FPATL_VERB_ENTRY *lpVerbEntries,
  LPENUMOLEVERB* ppenumOleVerb)
{
	LPOLEVERB lpVerbList = NULL;
	LPOLEVERB lpVerbListNew;
	LPOLEVERB lpVerb;
    const FPATL_VERB_ENTRY *lpEntry;
    
	long nVerbs = 0;
	long nAlloc = 0;
	LPOLESTR strVerbName;
    HRESULT hr = S_OK;

    if (!lpVerbEntries || !ppenumOleVerb)
      return FALSE;

	// walk the chain of message maps
	// find all verb entries in the map that have non-negative IDs
	lpEntry = lpVerbEntries;
	while (lpEntry != NULL && lpEntry->iResId != FPATL_VERB_RESID_LAST)
	{
      // Only place positive verbs in list
      if (lpEntry->iVerbId >= FPATL_FIRST_CUSTOM_VERB)
      {
		if (nVerbs == nAlloc)
		{
		  // not enough space for new item -- allocate more
		  lpVerbListNew = new OLEVERB[nVerbs + 10];
		  nAlloc += 10;
		  if (lpVerbList) // BoundsChecker reported exception -- make sure there is something to copy and delete! -scl
		  {
			memcpy(lpVerbListNew, lpVerbList, (size_t)(nVerbs *
				sizeof(OLEVERB)));
			delete [] lpVerbList;
		  }
		  lpVerbList = lpVerbListNew;
		}

	    strVerbName = lpEntry->lpolestrVerbName;
        // if name is null, get name by loading from resource file
        if (!strVerbName)
        {
          _ASSERT(lpEntry->iResId);  // MUST have either name or iResId!
//		if (!strVerbName.LoadString(lpEntry->nSig))
//			strVerbName = _T("<unknown verb>");     // LoadString failed
//		ASSERT(strVerbName.GetLength() > 0);
        }

		// add this item to the list
		_ASSERT(nVerbs < nAlloc);
		lpVerb = &lpVerbList[nVerbs];
		lpVerb->lVerb = lpEntry->iVerbId;

		lpVerb->lpszVerbName = fpAtlCoTaskCopyOleStr(strVerbName);
        if (!lpVerb->lpszVerbName)
        {
          hr = E_OUTOFMEMORY;
          break;
        }

		lpVerb->fuFlags = 0;
		lpVerb->grfAttribs = OLEVERBATTRIB_ONCONTAINERMENU;
		++nVerbs;
      } // if non-negative verb
	  ++lpEntry;
	}
	
	if (nVerbs > 0)
	{
	  // create and return the IEnumOLEVERB object
	  *ppenumOleVerb = 
        (LPENUMOLEVERB) new CfpEnumOleVerb(lpVerbList, (UINT)nVerbs);
      _ASSERT(*ppenumOleVerb);  // Allocation failed!!!
      if (*ppenumOleVerb)
        (*ppenumOleVerb)->AddRef();
	}
	else
	{
	  // no verbs: return NULL
	  *ppenumOleVerb = NULL;
	}

	return (*ppenumOleVerb != NULL);
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlDoVerb()
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   Given a VerbID, a list of verbs and some other parameters, this 
 *   function will call the function that corresponds to this verb.
 *
 * PARAMETERS:
 *
 * RETURNS:  FALSE if verb was not found, otherwise it returns the 
 *           results of the function (verb) call.
 *
 **************************************************************************/
BOOL CfpAtlBase::fpAtlDoVerb(LONG iVerb, LPMSG lpMsg, HWND hWndParent,
  LPCRECT lprcPosRect, const FPATL_VERB_ENTRY *lpVerbEntries)
{
    const FPATL_VERB_ENTRY *lpEntry;
    BOOL fRet = FALSE;

    // if verb is not custom (less than 1), return False
    if (iVerb < FPATL_FIRST_CUSTOM_VERB)
      return FALSE;

	lpEntry = lpVerbEntries;
	while (lpEntry != NULL && lpEntry->iVerbId != iVerb &&
             lpEntry->iResId != FPATL_VERB_RESID_LAST)
      ++lpEntry;

    // Assert if custom verb (negative "iVerb") is not found!
    _ASSERT(lpEntry != NULL && lpEntry->iResId != FPATL_VERB_RESID_LAST);

    // if found...
    if (lpEntry != NULL && lpEntry->iResId != FPATL_VERB_RESID_LAST)
    {
      FPATL_PVERBFN pfn = lpEntry->pfn;
      _ASSERT(pfn != NULL); // This function in the struct should not be null!
      if (pfn)       // if not null, invoke function
	    fRet = (this->*pfn)(lpMsg, hWndParent, lprcPosRect);
    }

    return fRet;
}

/***************************************************************************
 *
 * FUNCTION:  fpAtlOnFontChanged() 
 *
 * CLASS:     CfpAtlBase
 *
 * DESCRIPTION:
 *
 *   This function updates the FONT in the control by sending a
 *   WM_SETFONT to the control's HWND.
 *
 * PARAMETERS:
 *
 * RETURNS:  void
 *
 **************************************************************************/
void CfpAtlBase::fpAtlOnFontChanged(DISPID dispID, CfpAtlFont& fpAtlFont)
{
  if (fpAtlGetHWnd())
  {
    CComQIPtr<IFont, &IID_IFont> pFontTmp(fpAtlFont.p);
    if (pFontTmp)
    {
      HFONT hFont = NULL;
      pFontTmp->get_hFont(&hFont);
      pFontTmp->AddRefHfont(hFont);
      ::SendMessage(fpAtlGetHWnd(), WM_SETFONT, (WPARAM)hFont, 0);
      pFontTmp->ReleaseHfont(hFont);
    }
    else
      ::SendMessage(fpAtlGetHWnd(), WM_SETFONT, NULL, 0);

    fpAtlFireViewChange();
  }
}