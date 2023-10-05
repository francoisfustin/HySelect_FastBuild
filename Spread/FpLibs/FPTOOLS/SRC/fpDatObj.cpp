/***********************************************************************
* fpDatObj.cpp -- Implementation of CfpDataObject, CfpDataObjectFiles,
*				  and CfpDataObjectFilesEnum
*
* Copyright (C) 1991-1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/
#include "fpdatobj.h"
#include <string.h>
#include <shlobj.h>
#include <fpconvrt.h>
#include <fptstr.h>
#include "gdicopy.h"

CfpDataObject::CfpDataObject()
{
	m_lRefCount = 1;
	m_pData = NULL;
	m_pFiles = NULL;
	m_pTypeInfo = NULL;
	for (int i=0; i<8; i++)
		m_fFireOLESetData[i] = FALSE;
	m_fFormatSet = FALSE;
}

CfpDataObject::CfpDataObject(LPDATAOBJECT pData)
	: m_pData(pData) 
{
	m_lRefCount = 1;
	m_pFiles = NULL;
	m_pTypeInfo = NULL;
	for (int i=0; i<8; i++)
		m_fFireOLESetData[i] = FALSE;
	if (m_pData)
		m_pData->AddRef();
}

CfpDataObject::~CfpDataObject()
{
	if (m_pFiles)
		m_pFiles->Release();
	if (m_pData)
		m_pData->Release();
	if (m_pTypeInfo)
		m_pTypeInfo->Release();
}

HRESULT CfpDataObject::QueryInterface(REFIID iid, void** ppvObj)
{
	if (!ppvObj)
		return E_POINTER;
	*ppvObj = NULL;
	if (IID_IUnknown == iid || IID_IDispatch == iid || IID_IfpDataObject == iid)
		*ppvObj = this;

	if (NULL != *ppvObj)
    {
        ((LPUNKNOWN)*ppvObj)->AddRef();
        return S_OK;
	}

    return E_NOINTERFACE;
}

ULONG CfpDataObject::AddRef()
{
	return ++m_lRefCount;
}

ULONG CfpDataObject::Release()
{
   long tempCount;

   tempCount = --m_lRefCount;
   if(tempCount==0)
      delete this;
   return tempCount; 
}

HRESULT CfpDataObject::LoadTypeInfo()
{
	HRESULT hr;
	ITypeLib* pTypeLib;
	TCHAR szPath[_MAX_PATH];
	BSTR bstrPath;

	GetModuleFileName((HINSTANCE)fpInstance, szPath, _MAX_PATH);
	bstrPath = (BSTR)fpSysAllocBSTRFromTString(szPath);
	if (FAILED(hr = LoadTypeLib(bstrPath, &pTypeLib)))
	{
		pTypeLib = NULL;
		SysFreeString(bstrPath);
		return hr;
	}
	if (FAILED(hr = pTypeLib->GetTypeInfoOfGuid(IID_IfpDataObject, &m_pTypeInfo)))
		m_pTypeInfo = NULL;
	pTypeLib->Release();
	SysFreeString(bstrPath);
	return hr;
}

HRESULT CfpDataObject::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();

	return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgDispId);
}

HRESULT CfpDataObject::GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();
	if (!m_pTypeInfo)
		return E_UNEXPECTED;
	m_pTypeInfo->AddRef();
	*ppTInfo = m_pTypeInfo;
	return S_OK;	
}

HRESULT	CfpDataObject::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
	if (!pctinfo)
		return E_POINTER;
	*pctinfo = 1;
	return S_OK;
}

HRESULT CfpDataObject::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();
	return DispInvoke(this, m_pTypeInfo, dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}

HRESULT CfpDataObject::Clear()
{
	FORMATETC formatetc = {0L, NULL, DVASPECT_CONTENT, -1L, TYMED_NULL};
	STGMEDIUM stgmedium = {TYMED_NULL, 0L, NULL};

	if (m_pData)
		m_pData->SetData(&formatetc, &stgmedium, FALSE);
	else
		return E_FAIL;
	m_fFormatSet = FALSE;
	return S_OK;
}

HRESULT CfpDataObject::GetData(short nFormat, VARIANT* pvData)
{
	FORMATETC formatetc = {(CLIPFORMAT)nFormat, NULL, DVASPECT_CONTENT, -1L, TYMED_NULL};
	STGMEDIUM stgmedium;
	HRESULT hr;
	LPVOID  pData;
//GAB 8/3/04 Added for Unicode support. Bugs 14816 and 14996
    DWORD dwIsUnicode = 0;

	if (!pvData)
		return E_POINTER;

	if (CF_HDROP == nFormat)
		return STG_E_INVALIDFUNCTION;

	if (!m_pData)
		return E_FAIL;

	switch ((WORD)nFormat)
	{
	case CF_TEXT:
	case 0xBF01: // Rich Text Format
	case CF_HDROP:
	case CF_DIB:
		stgmedium.tymed = formatetc.tymed = TYMED_HGLOBAL;
		break;
	case CF_BITMAP:
	case CF_PALETTE:
		stgmedium.tymed = formatetc.tymed = TYMED_GDI;
		break;
	case CF_METAFILEPICT:
		stgmedium.tymed = formatetc.tymed = TYMED_MFPICT;
		break;
	case CF_ENHMETAFILE:
		stgmedium.tymed = formatetc.tymed = TYMED_ENHMF;
		break;
	}

// for some reason, Microsoft appears to be using the contant 
// 0xBF01 in their typelibs for vbCFRTF -- this seems to be consistent
// across versions of VB and their OCX controls.  But, this value
// does not work as the cfFormat member of the FORMATETC struct passed
// to their implementation of IDataObject::GetData when 0xBF01 is used
// as the clipboard format from the source VB control.
// I have found through trial and error that 0xC075 works, so in order
// to mimick the way VB controls treat the rich text clipboard format
// (and permit our controls to act as targets and sources for rich text
// in OLE drag and drop operations involving VB controls) we must translate
// 0xBF01 as 0xC075.
	if (formatetc.cfFormat == 0xBF01)
		formatetc.cfFormat = 0xC075;

	if (!(S_OK == (hr = m_pData->GetData(&formatetc, &stgmedium))))
		return hr;

	switch ((WORD)nFormat)
	{
	case CF_TEXT:
	case 0xBF01: // Rich Text Format
		if (TYMED_HGLOBAL == stgmedium.tymed)
		{
			pData = GlobalLock(stgmedium.hGlobal);
			if (pData)
      {

// fix for 14354 -scl
#if _UNICODE
//GAB 8/3/04 Added for Unicode support. Bugs 14816 and 14996
	  int len = GlobalSize(stgmedium.hGlobal);
	  int i = IS_TEXT_UNICODE_UNICODE_MASK;
	  dwIsUnicode = IsTextUnicode(pData, len, &i);

      if ( !dwIsUnicode )  //Convert file from ANSI to Unicode
      {
		  len = MultiByteToWideChar(CP_ACP, 0, (LPSTR)pData, -1, NULL, 0);
		  pvData->bstrVal = SysAllocStringByteLen(NULL, len * sizeof(TCHAR));
		  MultiByteToWideChar(CP_ACP, 0, (LPSTR)pData, -1, pvData->bstrVal, len);

		//GAB 8/11/04 Check the new string to see if it's UNICODE, if it's not UNICODE
		//return the original string.
/*		len = lstrlen((LPCTSTR)pvData->bstrVal) * sizeof(TCHAR);
		i = IS_TEXT_UNICODE_UNICODE_MASK;
		dwIsUnicode = IsTextUnicode(pvData->bstrVal, len, &i);
		if( !dwIsUnicode )
		{
			SysFreeString(pvData->bstrVal);
			len = GlobalSize(stgmedium.hGlobal);
			pvData->bstrVal = (BSTR)SysAllocStringByteLen((LPCSTR)pData, len);
		}
*/
	  }
	  else
		  pvData->bstrVal = (BSTR)fpSysAllocBSTRFromTString((FPTSTR)pData);
#else
				pvData->bstrVal = (BSTR)fpSysAllocBSTRFromTString((FPTSTR)pData);
#endif
        GlobalUnlock(stgmedium.hGlobal); // need to unlock it or it won't get freed! -scl
		pvData->vt = VT_BSTR;
      }
			else  // need to handle NULL hGlobal in stgmedium
				pvData->bstrVal = NULL; // GIC12124 -scl
		}	
		else if (TYMED_FILE == stgmedium.tymed)
			pvData->bstrVal = SysAllocString(stgmedium.lpszFileName);
/*// fix for 14354 -scl
#if _UNICODE
		{
//GAB 8/3/04 Added for Unicode support.
			int len = wcslen((LPCTSTR)stgmedium.lpszFileName) * sizeof(TCHAR);
			dwIsUnicode = IsTextUnicode(stgmedium.lpszFileName, len, NULL);
			
			if ( !dwIsUnicode )  //Convert file from ANSI to Unicode
			{
				len = MultiByteToWideChar(CP_ACP, 0, (LPSTR)stgmedium.lpszFileName, -1, NULL, 0);
				pvData->bstrVal = SysAllocStringByteLen(NULL, len * sizeof(TCHAR));
				MultiByteToWideChar(CP_ACP, 0, (LPSTR)stgmedium.lpszFileName, len, pvData->bstrVal, len);
			}
			else
				pvData->bstrVal = (BSTR)fpSysAllocBSTRFromTString((FPTSTR)stgmedium.lpszFileName);
		}
#else
			pvData->bstrVal = (BSTR)fpSysAllocBSTRFromTString((FPTSTR)stgmedium.lpszFileName);
#endif
*/
		else
			return DV_E_TYMED;
		pvData->vt = VT_BSTR;
		break;
	case CF_BITMAP:
	case CF_METAFILEPICT:
	case CF_DIB:
	case CF_PALETTE:
	case CF_ENHMETAFILE:
		//pvData->vt = VT_I4;
		//pvData->lVal = (long)stgmedium.hBitmap;
		{	// create an OLE Picture object
			// since the GDI object passed in will be freed by ReleaseStgMedium,
			// it must be copied before bing used to create the picture object
			PICTDESC pd;
			pd.cbSizeofstruct = sizeof(PICTDESC);
//			if (!stgmedium.hBitmap)
//			{	// empty picture
//				pd.picType = PICTYPE_NONE;
//				pd.bmp.hbitmap = 0;
//			}
			if (CF_METAFILEPICT == nFormat)
			{	// copy the metafile
				LPMETAFILEPICT pMetaSrc;	// source METAFILEPICT struct
				LPBYTE		   pMetaData;	// metafile data
				UINT		   nSize;		// size of metafile data
				
				pMetaSrc = (LPMETAFILEPICT)GlobalLock(stgmedium.hMetaFilePict);
				if (!pMetaSrc)
					return DV_E_TYMED;
				// get the size of the metafile data
				nSize = GetMetaFileBitsEx(pMetaSrc->hMF, 0, NULL);
				if (!nSize)
					return E_UNEXPECTED;
				// allocate memory for this data
				pMetaData = new BYTE[nSize];
				if (!pMetaData)
				{
					GlobalUnlock(stgmedium.hMetaFilePict);
					return E_OUTOFMEMORY;
				}
				// get the metafile data
				GetMetaFileBitsEx(pMetaSrc->hMF, nSize, (LPVOID)pMetaData);
				// create a new metafile using this data
				pd.wmf.hmeta = SetMetaFileBitsEx(nSize, pMetaData);
				// copy the other characteristics of the metafile to the new hMetaFilePict
				pd.wmf.xExt = pMetaSrc->xExt;
				pd.wmf.yExt = pMetaSrc->yExt;
				pd.picType = PICTYPE_METAFILE;
				// cleanup
				delete pMetaData;
				GlobalUnlock(stgmedium.hMetaFilePict);
			}
			else if (CF_ENHMETAFILE == nFormat)
			{	// copy the enhanced metafile
				if (S_OK != (hr = CopyHEnhMetaFile(stgmedium.hEnhMetaFile, &pd.emf.hemf)))
					return hr;
				pd.picType = PICTYPE_ENHMETAFILE;
			}
			else if (CF_PALETTE == nFormat)
			{	// stgmedium.hBitmap contains an HPALETTE?
				if (S_OK != (hr = CopyHPalette((HPALETTE)stgmedium.hBitmap, &pd.bmp.hpal)))
					return hr;
				pd.picType = PICTYPE_BITMAP;
				pd.bmp.hbitmap = CreateBitmap(0, 0, 1, 1, NULL);
			}
			else
			{	// copy the bitmap
				pd.picType = PICTYPE_BITMAP;
				pd.bmp.hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
				if (CF_BITMAP == nFormat)
				{
					if (S_OK != (hr = CopyHBitmap(stgmedium.hBitmap, &pd.bmp.hbitmap)))
						return hr;
				}
				else	// CF_DIB or CF_PALETTE
				{	// create a bitmap from the DIB
					pd.bmp.hpal = CreateDibPalette(stgmedium.hGlobal);
					pd.bmp.hbitmap = BitmapFromDib(stgmedium.hGlobal, pd.bmp.hpal, TRUE);
				}
			}
			// transfer ownership of this GDI object to the picture object
			OleCreatePictureIndirect(&pd, IID_IPictureDisp, TRUE, (void**)&pvData->pdispVal); 
  		pvData->vt = VT_PICTURE;
		}
		break;
	}
	// release the stgmedium
	if (stgmedium.pUnkForRelease)
		stgmedium.pUnkForRelease->Release();
	else
		ReleaseStgMedium(&stgmedium);
	return S_OK;
}

#ifdef NO_MFC
HRESULT CfpDataObject::GetFormat(short nFormat, VARIANT_BOOL* pfFormatSupported)
#else
HRESULT CfpDataObject::GetFormat(short nFormat, BOOL* pfFormatSupported)
#endif
{
//	LPENUMFORMATETC penumFormats;
	FORMATETC format = {(CLIPFORMAT)nFormat, NULL, DVASPECT_CONTENT, -1L, TYMED_HGLOBAL|TYMED_FILE|TYMED_ISTREAM|TYMED_ISTORAGE|TYMED_GDI|TYMED_MFPICT|TYMED_ENHMF};
	BOOL	  fDone = FALSE;

	if (!pfFormatSupported)
		return E_POINTER;
	else
		*pfFormatSupported = FALSE;

	if (!m_pData)
		return E_FAIL;
/*
	if (S_OK == m_pData->EnumFormatEtc(1, &penumFormats))
		while (!fDone && S_OK == penumFormats->Next(1, &format, NULL))
			if (format.cfFormat == nFormat)
			{
				*pfFormatSupported = TRUE;
				fDone = TRUE;
				penumFormats->Release();
			}
*/
	if (S_OK == m_pData->QueryGetData(&format))
		*pfFormatSupported = TRUE;
	return S_OK;
}

HRESULT CfpDataObject::SetData(VARIANT vData, VARIANT vFormat)
{
	FORMATETC formatetc = {0L, NULL, 1L, -1L, TYMED_NULL};
	STGMEDIUM stgmedium = {TYMED_NULL, 0L, NULL};
	BOOL	  fNoData = (VT_EMPTY == vData.vt || VT_ERROR == vData.vt);
	BOOL	  fNoFormat = (VT_EMPTY == vFormat.vt || VT_ERROR == vFormat.vt);
	IPicture* pPict;	// to get the handle for GDI objects
	BOOL	  fRelease = TRUE;	// flag for releasing the stgmedium
	short	  i;	// array index into m_fFireOLESetData

	// must pass one or the other, or both
	if (fNoData && fNoFormat)
		return DISP_E_BADVARTYPE;

	// must have a data object
	if (!m_pData)
		return E_FAIL;

	if (fNoFormat)
	{	// assume CF_TEXT if format is omitted
		// that appears to be the way VB controls behave -scl
		vFormat.vt = VT_I4;
		vFormat.lVal = (long)CF_TEXT;
	}
	m_fFormatSet = TRUE;

	if (VT_I2 == vFormat.vt || S_OK == VariantChangeType((VARIANTARG FAR*)&vFormat, (VARIANTARG FAR*)&vFormat, 0, VT_I2))
		formatetc.cfFormat = (CLIPFORMAT)vFormat.iVal;
	else
		return DISP_E_BADVARTYPE;
	
// change the clipformat to 0xC075 if is 0xBF01
// (see above) -scl
	if (formatetc.cfFormat == 0xBF01)
		formatetc.cfFormat = 0xC075;

	// determine array index into m_fFireOLESetData for this format 
	switch((WORD)formatetc.cfFormat)
	{
	case CF_TEXT:			i = 0;	break;
	case CF_BITMAP:			i = 1;	break;
	case CF_METAFILEPICT:	i = 2;	break;
	case CF_DIB:			i = 3;	break;
	case CF_PALETTE:		i = 4;	break;
	case CF_ENHMETAFILE:	i = 5;	break;
	case CF_HDROP:			i = 6;	break;
	case 0xC075:			i = 7;	break;	//	Rich Text
	default:				return DV_E_FORMATETC;
	}
	if (fNoData)
		m_fFireOLESetData[i] = TRUE;	// adding a format only
	else
		m_fFireOLESetData[i] = FALSE;	// adding format and data

	// initialize the tymed for this clipboard format
	switch (formatetc.cfFormat)
	{
	case CF_TEXT:
	case 0xC075: // Rich Text Format
	case CF_HDROP:
	case CF_DIB:
		stgmedium.tymed = formatetc.tymed = TYMED_HGLOBAL;
		break;
	case CF_BITMAP:
	case CF_PALETTE:
		stgmedium.tymed = formatetc.tymed = TYMED_GDI;
		break;
	case CF_METAFILEPICT:
		stgmedium.tymed = formatetc.tymed = TYMED_MFPICT;
		break;
	case CF_ENHMETAFILE:
		stgmedium.tymed = formatetc.tymed = TYMED_ENHMF;
		break;
	}
		
	if (!m_fFireOLESetData[i] || CF_HDROP == formatetc.cfFormat)
	{	// put the data into the stgmedium
		switch ((WORD)formatetc.cfFormat)
		{
		case CF_TEXT:
		case 0xC075: // Rich Text Format
			// data is a TSTR inside a HGLOBAL
			if (VT_BSTR == vData.vt || S_OK == VariantChangeType((VARIANTARG FAR*)&vData, (VARIANTARG FAR*)&vData, 0, VT_BSTR))
			{
				//FPTSTR	   fptstr = fpSysAllocTStringFromBSTR(vData.bstrVal);
				long lByteLen = WideCharToMultiByte(CP_ACP, 0, vData.bstrVal, -1, NULL, 0, NULL, NULL);
				FPTSTR fptstr = (FPTSTR)SysAllocStringByteLen(NULL, lByteLen);
				WideCharToMultiByte(CP_ACP, 0, vData.bstrVal, -1, (LPSTR)fptstr, lByteLen, NULL, NULL);
// -> GIC12124 -scl        
//        long	   lByteLen = fptstr ? lstrlen(fptstr) + 1 : 0;
				TCHAR FAR* pChars;

        stgmedium.hGlobal = NULL;
		//GAB 8/3/04 Updated to account for Unicode strings Bug 14816
				if (fptstr && (stgmedium.hGlobal = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, lByteLen * sizeof(TCHAR))))
				{
					pChars = (TCHAR FAR*)GlobalLock(stgmedium.hGlobal);
					memcpy(pChars, fptstr, lByteLen); 	//GAB 8/3/04 Bug 14816
					GlobalUnlock(stgmedium.hGlobal);
					stgmedium.tymed = formatetc.tymed = TYMED_HGLOBAL;
					fpSysFreeTString(fptstr);
				}
				else if (fptstr)
				{
					fpSysFreeTString(fptstr);
					return E_OUTOFMEMORY;
				}
// <- GIC12124 -scl        
			}
			else
				return DISP_E_BADVARTYPE;
			break;
/*
		case CF_HDROP:
			if (stgmedium.hGlobal = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DROPFILES) + 4))
			{
				LPDROPFILES lpDropFiles = (LPDROPFILES)GlobalLock(stgmedium.hGlobal);
				char*       lpNullTerm;
				lpDropFiles->pFiles = sizeof(DROPFILES);
				lpDropFiles->fWide = TRUE;
				lpNullTerm = (char*)lpDropFiles;
				lpNullTerm += sizeof(DROPFILES);
				*lpNullTerm = *++lpNullTerm = *++lpNullTerm = *++lpNullTerm = 0;
				if (!m_pFiles)
					m_pFiles = new CfpDataObjectFiles(this, (HDROP)stgmedium.hGlobal);
				else
					m_pFiles->SetHDrop((HDROP)stgmedium.hGlobal);
				stgmedium.tymed = formatetc.tymed = TYMED_HGLOBAL;
				GlobalUnlock(stgmedium.hGlobal);
			}
			else
				return E_OUTOFMEMORY;
			break;
*/
		case CF_BITMAP:
		case CF_DIB:
		case CF_PALETTE:
			// these types expect a dispatch pointer to an IPictureDispatch
			if (VT_PICTURE == vData.vt && vData.pdispVal && S_OK == vData.pdispVal->QueryInterface(IID_IPicture, (void**)&pPict))
			{
				if (CF_DIB == formatetc.cfFormat)
				{
					HBITMAP hBm;
					HPALETTE hPal;
					pPict->get_Handle((OLE_HANDLE*)&hBm);
					pPict->get_hPal((OLE_HANDLE*)&hPal);
					stgmedium.hGlobal = BitmapToDIB(hBm, hPal);
					stgmedium.tymed = formatetc.tymed = TYMED_HGLOBAL;
				}
				else if (CF_PALETTE == formatetc.cfFormat)
				{
					pPict->get_hPal((OLE_HANDLE*)&stgmedium.hBitmap);
					stgmedium.tymed = formatetc.tymed = TYMED_GDI;
					fRelease = FALSE;	// don't free this handle
				}
				else
				{
					pPict->get_Handle((OLE_HANDLE*)&stgmedium.hBitmap);
					stgmedium.tymed = formatetc.tymed = TYMED_GDI;
					fRelease = FALSE;	// don't free this handle
				}
				// let the IDataObject implementataion free the reference to pPict
				//stgmedium.pUnkForRelease = (IUnknown*)pPict;
				pPict->Release();
			}
			else
				return DISP_E_BADVARTYPE;
			break;
		case CF_METAFILEPICT:
			if (VT_PICTURE == vData.vt && vData.pdispVal && S_OK == vData.pdispVal->QueryInterface(IID_IPicture, (void**)&pPict))
			{
				HGLOBAL hMetaFilePict = GlobalAlloc(GMEM_DDESHARE, sizeof(METAFILEPICT));
				if (!hMetaFilePict)
					return E_OUTOFMEMORY;
				
				LPMETAFILEPICT pMetaFilePict = (LPMETAFILEPICT)GlobalLock(hMetaFilePict);
				pMetaFilePict->mm = MM_ANISOTROPIC;
				pPict->get_Handle((OLE_HANDLE*)&pMetaFilePict->hMF);
				pPict->get_Width((OLE_XSIZE_HIMETRIC*)&pMetaFilePict->xExt);
				pPict->get_Height((OLE_YSIZE_HIMETRIC*)&pMetaFilePict->yExt);
				GlobalUnlock(hMetaFilePict);
				stgmedium.hMetaFilePict = hMetaFilePict;
				stgmedium.tymed = formatetc.tymed = TYMED_MFPICT;
				// the IDataObject implementataion can't free this handle because
				// ReleaseStgMedium will free the hMetafile in the hMetaFilePict
				// so we must not pass ownership of this handle to the data object
				//fRelease = FALSE;
				pPict->Release();
			}
			else
				return DISP_E_BADVARTYPE;
			break;
		case CF_ENHMETAFILE:
			if (VT_PICTURE == vData.vt && vData.pdispVal && S_OK == vData.pdispVal->QueryInterface(IID_IPicture, (void**)&pPict))
			{
				pPict->get_Handle((OLE_HANDLE*)&stgmedium.hEnhMetaFile);
				stgmedium.tymed = formatetc.tymed = TYMED_ENHMF;
				// let the IDataObject implementataion free the reference to pPict
				//stgmedium.pUnkForRelease = (IUnknown*)pPict;
				pPict->Release();
				fRelease = FALSE;
			}
			else
				return DISP_E_BADVARTYPE;
			break;
		}
	}
	// set the format (and possibly the data)
	m_pData->SetData(&formatetc, &stgmedium, FALSE);
	
	if (formatetc.cfFormat == CF_METAFILEPICT)
	{	// free metafilepict by hand (to prevent ReleaseStgMedium 
		// from freeing the HMETAFILE it contains) -scl
		GlobalFree(stgmedium.hMetaFilePict);
		fRelease = FALSE;
	}
	
	// release stgmedium (unless flag was set to FALSE)
	if (fRelease)
	{
		if (stgmedium.pUnkForRelease)
			stgmedium.pUnkForRelease->Release();
		else
			ReleaseStgMedium(&stgmedium);
	}
	return S_OK;
}

#ifdef NO_MFC
HRESULT CfpDataObject::get_Files(IfpDataObjectFiles** ppFiles)
#else
HRESULT CfpDataObject::Files(IfpDataObjectFiles** ppFiles)
#endif
{
#ifdef NO_MFC
	VARIANT_BOOL bValid = FALSE;
#else
	BOOL bValid = FALSE;
#endif
	if (!ppFiles)
		return E_POINTER;
	GetFormat(CF_HDROP, &bValid);
	if (!bValid)
		return DV_E_CLIPFORMAT;
	if (!m_pData)
		return E_UNEXPECTED;
	if (!m_pFiles)
		m_pFiles = new CfpDataObjectFiles(this, m_pData);
	ASSERT(m_pFiles);
	m_pFiles->AddRef();
	*ppFiles = m_pFiles;
	return S_OK;
}

// initialize format for calls to IDataObject::GetData
FORMATETC CfpDataObjectFiles::format = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

HRESULT CfpDataObjectFiles::GetHDrop(HDROP* phDrop)
{
	if (!m_pData)
		return E_UNEXPECTED;

	HRESULT hr;

	if (S_OK != (hr = m_pData->GetData(&format, &medium)))
		return hr;

	if (!medium.hGlobal)
	{	// initialize a new HDROP
		LPDROPFILES pDropFiles;
		char*		pNullTerm;

// fix for CHB1575
// Windows 95 and 98 do not appear to expect wide-character filenames
// in the DROPFILES struct.  NT seems to work well with either, so 
// to make it work for both, we'll use regular chars
//		medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DROPFILES) + 2 * sizeof(WCHAR));
		medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DROPFILES) + 2 * sizeof(char));
		if (!medium.hGlobal)
			return E_OUTOFMEMORY;
		pDropFiles = (LPDROPFILES)GlobalLock(medium.hGlobal);
		pDropFiles->pFiles = sizeof(DROPFILES);
//		pDropFiles->fWide = TRUE;
		pDropFiles->fWide = FALSE;
		pNullTerm = (char*)pDropFiles;
		pNullTerm += sizeof(DROPFILES);
//		for (UINT i=0; i<2*sizeof(WCHAR); i++)
		for (UINT i=0; i<2*sizeof(char); i++)
			*pNullTerm++ = 0;
// fix for CHB1575
		GlobalUnlock(medium.hGlobal);
	}
	// handle is now owned by this object
	// must be freed with GlobalFree() 
	// or transferred with SetHDrop().
	*phDrop = (HDROP)medium.hGlobal;
	return S_OK;
}

HRESULT CfpDataObjectFiles::SetHDrop(HDROP hDrop)
{
	HRESULT hr;

	if (!m_pData)
		return E_UNEXPECTED;

	medium.hGlobal = (HGLOBAL)hDrop;
	hr = m_pData->SetData(&format, &medium, FALSE);
	GlobalFree((HGLOBAL)hDrop);
	return hr;
}

CfpDataObjectFiles::CfpDataObjectFiles(CfpDataObject* pfpDataObject, LPDATAOBJECT pDataObject)
	: m_pfpDataObject(pfpDataObject), m_pData(pDataObject)
{
	m_lRefCount = 1;
	m_pTypeInfo = NULL;
// can't call AddRef() because then this object and the fpDataObject would
// each hold a reference on the other, preventing either from getting released -scl
//	if (m_pfpDataObject)
//		m_pfpDataObject->AddRef();
	if (m_pData)
		m_pData->AddRef();
}

CfpDataObjectFiles::~CfpDataObjectFiles()
{
//   if (m_pfpDataObject)
//	   m_pfpDataObject->Release();
   if (m_pData)
	   m_pData->Release();
   if (m_pTypeInfo)
	   m_pTypeInfo->Release();
}

HRESULT CfpDataObjectFiles::QueryInterface(REFIID iid, void** ppvObj)
{
	if (!ppvObj)
		return E_POINTER;
	*ppvObj = NULL;
	if (IID_IUnknown == iid || IID_IDispatch == iid || IID_IfpDataObjectFiles == iid)
		*ppvObj = this;

	if (NULL != *ppvObj)
	{
		((LPUNKNOWN)*ppvObj)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG CfpDataObjectFiles::AddRef()
{
	return ++m_lRefCount;
}

ULONG CfpDataObjectFiles::Release()
{
   long tempCount;

   tempCount = --m_lRefCount;
   if(tempCount==0)
      delete this;
   return tempCount; 
}

HRESULT CfpDataObjectFiles::LoadTypeInfo()
{
	HRESULT hr;
	ITypeLib* pTypeLib;
	TCHAR szPath[_MAX_PATH];
	BSTR bstrPath;

	GetModuleFileName((HINSTANCE)fpInstance, szPath, _MAX_PATH);
	bstrPath = (BSTR)fpSysAllocBSTRFromTString(szPath);
	if (FAILED(hr = LoadTypeLib(bstrPath, &pTypeLib)))
	{
		pTypeLib = NULL;
		SysFreeString(bstrPath);
		return hr;
	}
	if (FAILED(hr = pTypeLib->GetTypeInfoOfGuid(IID_IfpDataObjectFiles, &m_pTypeInfo)))
		m_pTypeInfo = NULL;
	pTypeLib->Release();
	SysFreeString(bstrPath);
	return hr;
}

HRESULT CfpDataObjectFiles::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();

	return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgDispId);
}

HRESULT CfpDataObjectFiles::GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();
	if (!m_pTypeInfo)
		return E_UNEXPECTED;
	m_pTypeInfo->AddRef();
	*ppTInfo = m_pTypeInfo;
	return S_OK;	
}

HRESULT	CfpDataObjectFiles::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
	if (!pctinfo)
		return E_POINTER;
	*pctinfo = 1;
	return S_OK;
}

HRESULT CfpDataObjectFiles::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();
	return DispInvoke(this, m_pTypeInfo, dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}

/*
HRESULT CfpDataObjectFiles::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr)
{
  HRESULT hr;
  IUnknown **ppUnk = NULL;

  // The only property that we are expecting is _NewEnum (-4), for
  // all others we will return error.
  if (dispIdMember != DISPID_NEWENUM)
    return DISP_E_MEMBERNOTFOUND;

  // If no return value and no parameter, then return an error.
  if (!pVarResult && (!pDispParams || pDispParams->cArgs < 1))
    return DISP_E_PARAMNOTFOUND;


  // If this is a put, then return an error, only "get".
  if (wFlags & (DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYPUT))
    return E_UNEXPECTED;

  // Determine which parameter to use...
  if (pVarResult)
  {
    ppUnk = &pVarResult->punkVal;
  }
  else
  {
    // Verify that parameter is a ptr
    if (pDispParams->rgvarg->vt != (VT_BYREF|VT_UNKNOWN) || 
      !pDispParams->rgvarg->ppunkVal)
      return STG_E_INVALIDPARAMETER;
    ppUnk = pDispParams->rgvarg->ppunkVal;
  }

  // Init result variant (if specified)
  if (pVarResult)
    VariantInit(pVarResult);

  // Call member function to get interface ptr.
  hr = _NewEnum(ppUnk);

  // If successful and we have a ptr, set the "vt" to VT_DISPATCH in 
  // the result variant (the value is already set via "ppUnk").
  if (hr == S_OK && pVarResult && *ppUnk)
    pVarResult->vt = VT_UNKNOWN;  

  return hr;
}
*/

#ifdef NO_MFC
HRESULT CfpDataObjectFiles::get_Item(long lIndex, BSTR FAR* pbstrItem)
#else
HRESULT CfpDataObjectFiles::Item(long lIndex, BSTR FAR* pbstrItem)
#endif
{
	HRESULT hr;
	HDROP   hDrop;		// DROPFILES handle
	long    lCount;		// count of filenames in list
	FPTSTR  fptstrTemp;	// temp LPTSTR to store filename
	
	if (!pbstrItem)
		return E_POINTER;
	// get the HDROP
	if (S_OK != (hr = GetHDrop(&hDrop)))
		return hr;
	else
	{	// make sure this index is valid
		lCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		if (lIndex < 0 || lIndex >= lCount)
			return DISP_E_BADINDEX;
		// allocate a temp string for the filename
		fptstrTemp = fpSysAllocTStringLen(_T(""), _MAX_PATH);
		// get the string itself (using the nice WINAPI function)
		DragQueryFile(hDrop, lIndex, fptstrTemp, _MAX_PATH);
		// allocate a BSTR for the filename
		*pbstrItem = (BSTR)fpSysAllocBSTRFromTString(fptstrTemp);
		fpSysFreeTString(fptstrTemp);
		GlobalFree(hDrop);
	}
	return S_OK;
}

#ifdef NO_MFC
HRESULT CfpDataObjectFiles::get_Count(long* plCount)
#else
HRESULT CfpDataObjectFiles::Count(long* plCount)
#endif
{
	HRESULT hr;
	HDROP	hDrop;

	if (!plCount)
		return E_POINTER;
	// get the HDROP
	if (S_OK != (hr = GetHDrop(&hDrop)))
		return hr;
	else
	{
		*plCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		GlobalFree(hDrop);
	}
	return S_OK;
}

HRESULT CfpDataObjectFiles::Add(BSTR bstrFileName, VARIANT vIndex)
{
	HRESULT		hr;
	HDROP		hDrop;		// handle to DROPFILES struct
	LPDROPFILES	pDropFiles;// pointer to DROPFILES struct
	FPTSTR		pFiles;    // pointer to file list
	FPTSTR		fptstrTemp;	// temp FPTSTR to store filename
	long		lCount;		// count of files
	long		lLen;		// length of bstrFileName passed in
	long		lByteLen;	// lendth of bstrFileName in bytes
	DWORD		lMemSize;	// size of global memory block (in bytes)
	long		lIndex;		// index to insert file at
	long		lOffset;	// offset of new filename relative to lpFiles (in bytes)
	long		lTemp;		// loop variable

	// get the HDROP
	if (S_OK != (hr = GetHDrop(&hDrop)))
		return hr;
	// lock the handle
	pDropFiles = (LPDROPFILES)GlobalLock(hDrop);
	// get the count of files
	lCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);	// if no index was passed, add to the end
	// if no index was passed, append the new filename to the end
	if (VT_EMPTY == vIndex.vt || VT_ERROR == vIndex.vt)	
		lIndex = lCount;
	else 
	{	// change the vartype to VT_I4 if necessary
		if (VT_I4 != vIndex.vt) 
			if (S_OK != VariantChangeType((VARIANTARG FAR*)&vIndex, (VARIANTARG FAR*)&vIndex, 0, VT_I4))
			{
				GlobalFree(hDrop);
				// return bad vartype if conversion fails
				return DISP_E_BADVARTYPE; 
			}
		if (vIndex.lVal < 0 || vIndex.lVal > lCount)
		{
			// return bad index if the one passed is out of range
			GlobalFree(hDrop);
			return DISP_E_BADINDEX;		
		}
		lIndex = vIndex.lVal;
	}
	// copy filename passed in to fptstrTemp
	// and get the length of the filename passed in (in characters)
	if (pDropFiles->fWide) // if wide characters, just copy bstrFileName
	{
		fptstrTemp = (LPTSTR)SysAllocString(bstrFileName);
		lLen = SysStringLen((BSTR)fptstrTemp);
	}
	else	// convert from wide characters to ansi (must be ANSI build)
	{
		fptstrTemp = (LPTSTR)fpSysAllocTStringFromBSTR(bstrFileName);
		lLen = strlen((char*)fptstrTemp);
	}
	// check to make sure the string was copied
	if (!lLen)
		return E_OUTOFMEMORY;
	// calculate the memory required for this string (including NULL terminator)
	lByteLen = (pDropFiles->fWide ? sizeof(WCHAR) : sizeof(char)) * (lLen + 1);
	// get the size of the global memory block (in bytes)
	lMemSize = GlobalSize(hDrop);
	// unlock the handle
	GlobalUnlock(hDrop);
	// reallocate memory to add enough for this new filename. including the terminating NULL
	hDrop = (HDROP)GlobalReAlloc((HGLOBAL)hDrop, lMemSize + lByteLen, GMEM_MOVEABLE|GMEM_DDESHARE);
	// lock the returned handle
	pDropFiles = (LPDROPFILES)GlobalLock(hDrop);
	// compute pointer to the list of filenames
	pFiles = (FPTSTR)(pDropFiles + 1);
	// compute the offset from pFiles to the filename in the list at lIndex
	if (pDropFiles->fWide)
		for (lOffset = 0, lTemp = 0; lTemp < lIndex; lTemp++)
			lOffset += sizeof(WCHAR) * wcslen((WCHAR*)((char*)pFiles + lOffset)) + sizeof(WCHAR);
													//  ^- cast necessary to compute lOffset in bytes
	else // !fWide
		for (lOffset = 0, lTemp = 0; lTemp < lIndex; lTemp++)
			lOffset += sizeof(char) * strlen((char*)((char*)pFiles + lOffset)) + sizeof(char);
													//  ^- cast necessary to compute lOffset in bytes
	// move the filenames starting at lIndex forward by lByteLen bytes
	memcpy((void*)((char*)pFiles + lOffset + lByteLen), 
		   (void*)((char*)pFiles + lOffset),
		   (size_t)(lMemSize - sizeof(DROPFILES) - lOffset));
	// copy the new filename into the list
	if (pDropFiles->fWide)
		wcscpy((WCHAR*)((char*)pFiles + lOffset), (WCHAR*)fptstrTemp);
	else
		strcpy((char*)((char*)pFiles + lOffset), (char*)fptstrTemp);
	// if this is the last filename in the list, terminate the list with a NULL
//	if (lIndex == lCount) // lOffset must be 0 in this case
//		for (UINT i=1; i<=(pDropFiles->fWide ? sizeof(WCHAR) : sizeof(char)); i++)
//			*((char*)pFiles + lByteLen + i) = 0;
	// cleanup
	fpSysFreeTString(fptstrTemp);
	GlobalUnlock(hDrop);
	// put new hDrop into IDataObject
	return SetHDrop(hDrop);
}

HRESULT CfpDataObjectFiles::Clear()
{
	HDROP		hDrop;
	LPDROPFILES pDropFiles;
	char*		pNullTerm;
	// allocate memory for no filenames, just the two terminating NULLs
	// (one to terminate the string and one to terminate the list)
	hDrop = (HDROP)GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, (DWORD)(sizeof(DROPFILES) + 2 * sizeof(WCHAR)));
	if (!hDrop)
		return E_OUTOFMEMORY;
	// lock the returned handle
	pDropFiles = (LPDROPFILES)GlobalLock(hDrop);
	pDropFiles->pFiles = sizeof(DROPFILES);
	pDropFiles->fWide = TRUE;
	// compute the address of the terminating NULLs
	pNullTerm = (char*)pDropFiles;
	pNullTerm += sizeof(DROPFILES);
	// make these two characters into NULLs
	for (int i=0; i<2*sizeof(WCHAR); i++)
		*pNullTerm++ = 0;
	// cleanup
	GlobalUnlock(hDrop);
	// put new hDrop into IDataObject
	return SetHDrop(hDrop);
}

HRESULT CfpDataObjectFiles::Remove(VARIANT vIndex)
{
	HRESULT		hr;
	HDROP		hDrop;		// handle to DROPFILES struct
	LPDROPFILES	pDropFiles; // pointer to DROPFILES struct
	LPTSTR		pFiles;     // pointer to file list
	long		lCount;		// count of files
	long		lLen;		// length of filename to remove
	long		lByteLen;	// byte length of filename to remove
	DWORD		lMemSize;	// size of global memory block
	long		lIndex;		// index to insert file at
	long		lOffset;	// offset of new filename relative to lpFiles
	long		lTemp;		// loop variable
	
	// get the HDROP
	if (S_OK != (hr = GetHDrop(&hDrop)))
		return hr;
	// get the count of files
	lCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);	// if no index was passed, add to the end
	// if no items are in list, fail
	if (!lCount)
		return E_FAIL;
	// if no index was passed, return E_INVALIDARG
	if (VT_EMPTY == vIndex.vt || VT_ERROR == vIndex.vt)	
		return E_INVALIDARG;
	else 
	{	// change the vartype to VT_I4 if necessary
		if (VT_I4 != vIndex.vt) 
			if (S_OK != VariantChangeType((VARIANTARG FAR*)&vIndex, (VARIANTARG FAR*)&vIndex, 0, VT_I4))
				// return bad vartype if conversion fails
				return DISP_E_BADVARTYPE; 
		if (vIndex.lVal < 0 || vIndex.lVal > lCount - 1)
				// return bad index if ithe one passed is out of range
				return DISP_E_BADINDEX;		
		lIndex = vIndex.lVal;
	}
	// get the size of the global memory block
	lMemSize = GlobalSize(hDrop);
	// lock the handle to the files
	pDropFiles = (LPDROPFILES)GlobalLock(hDrop);
	// compute pointer to the list of filenames
	pFiles = (LPTSTR)(pDropFiles + 1);
	// compute the offset from lpFiles to the filename in the list at lIndex
	if (pDropFiles->fWide)
		for (lOffset = 0, lTemp = 0; lTemp < lIndex; lTemp++)
			lOffset += 2 * wcslen((WCHAR*)((char*)pFiles + lOffset)) + 2;
									//  ^- cast necessary to compute lOffset in bytes
	else // !fWide
		for (lOffset = 0, lTemp = 0; lTemp < lIndex; lTemp++)
			lOffset += strlen((char*)pFiles + lOffset) + 1;
									//  ^- cast necessary to compute lOffset in bytes
	// get the length of the filename at lIndex
	if (pDropFiles->fWide)
		lLen = wcslen((wchar_t*)((char*)pFiles + lOffset));
	else
		lLen = strlen(((char*)pFiles + lOffset));
	// calculate the memory required for this string (including NULL terminator)
	lByteLen = (pDropFiles->fWide ? sizeof(WCHAR) : sizeof(char)) * (lLen + 1);
	// move the filenames starting at (lIndex + 1) backward by lByteLen bytes
	memcpy((void*)((char*)pFiles + lOffset), 
		   (void*)((char*)pFiles + lOffset + lByteLen), 
		   (size_t)(lMemSize - sizeof(DROPFILES) - lOffset));
	// adjust lMemSize by the number of bytes in the filename just removed
	lMemSize -= lByteLen;
	// unlock handle
	GlobalUnlock(hDrop);
	// reallocate global memory for the new size of the block
	hDrop = (HDROP)GlobalReAlloc(hDrop, lMemSize, GHND|GMEM_SHARE);
	// put new hDrop into IDataObject
	return SetHDrop(hDrop);
}

#ifdef NO_MFC
HRESULT CfpDataObjectFiles::get__NewEnum(IUnknown** ppUnk)
#else
HRESULT CfpDataObjectFiles::_NewEnum(IUnknown** ppUnk)
#endif
{
	if (!ppUnk)
		return E_POINTER;

	HRESULT hr;
	HDROP	hDrop;

	// get the HDROP
	if (S_OK != (hr = GetHDrop(&hDrop)))
		return hr;

	CfpDataObjectFilesEnum* penum = new CfpDataObjectFilesEnum(hDrop);
	// hDrop is now owned by the CfpDataObjectFilesEnum object
	*ppUnk = (IUnknown*)penum;
	return S_OK;
}

CfpDataObjectFilesEnum::CfpDataObjectFilesEnum(HDROP hDrop)
	: m_hDrop(hDrop)
{
    m_lRefCount = 1;

	if (m_hDrop)
		m_lCount = DragQueryFile(m_hDrop, 0xFFFFFFFF, NULL, 0);
	else
		m_lCount = 0;
	m_lCurrent = 0;
}

CfpDataObjectFilesEnum::~CfpDataObjectFilesEnum()
{
   GlobalFree(m_hDrop);
}

HRESULT CfpDataObjectFilesEnum::QueryInterface(REFIID iid, void** ppvObj)
{
	if (!ppvObj)
		return E_POINTER;
	*ppvObj = NULL;
	if (IID_IUnknown == iid || IID_IEnumVARIANT == iid)
		*ppvObj = this;

	if (NULL != *ppvObj)
	{
		((LPUNKNOWN)*ppvObj)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG CfpDataObjectFilesEnum::AddRef()
{
	return ++m_lRefCount;
}

ULONG CfpDataObjectFilesEnum::Release()
{
   long tempCount;

   tempCount = --m_lRefCount;
   if(tempCount==0)
      delete this;
   return tempCount; 
}

HRESULT CfpDataObjectFilesEnum::Next(ULONG celt, VARIANT *rgVar, 
  ULONG *pCeltFetched)
{
	FPTSTR	fptstrTemp;
	long	lIndex1;
	ULONG	lIndex2;

	if (pCeltFetched != NULL)
		*pCeltFetched = 0;

    // If num of elements is 0, then just return
    if (!celt)
      return S_OK;

    // If invalid ptr, then return
    if (!rgVar)
      return E_POINTER;

	fptstrTemp = fpSysAllocTStringLen(_T(""), _MAX_PATH);
	// Retrieve the next celt elements.
	for (lIndex1 = m_lCurrent, lIndex2 = 0;
         lIndex1 < m_lCount && lIndex2 < celt;
         lIndex1++, lIndex2++)
	{
	    DragQueryFile(m_hDrop, lIndex1, fptstrTemp, _MAX_PATH);
	    rgVar[lIndex2].bstrVal = (BSTR)fpSysAllocBSTRFromTString(fptstrTemp);
        rgVar[lIndex2].vt = VT_BSTR;
	}
	fpSysFreeTString(fptstrTemp);

	// Set count of elements retrieved
	if (pCeltFetched != NULL)
		*pCeltFetched = lIndex2;
	m_lCurrent = lIndex1;

	return  (lIndex2 < celt) ? ResultFromScode(S_FALSE) : NOERROR;
}

HRESULT CfpDataObjectFilesEnum::Skip(ULONG celt)
{   
	m_lCurrent += celt; 
	if (m_lCurrent >= (long)(m_lCount))
	{
		m_lCurrent =  m_lCount - 1;
		return ResultFromScode(S_FALSE);
	} 
	else 
      return NOERROR;
}

HRESULT CfpDataObjectFilesEnum::Reset(void)
{ 
	m_lCurrent = 0;
	return NOERROR;
}

HRESULT CfpDataObjectFilesEnum::Clone(IEnumVARIANT **ppEnum)
{
	if (!ppEnum)
		return E_POINTER;

	CfpDataObjectFilesEnum FAR* pEnum = NULL;

	pEnum = new CfpDataObjectFilesEnum(m_hDrop);
	pEnum->m_lCurrent = m_lCurrent; 

	*ppEnum = (IEnumVARIANT*)pEnum;
	return NOERROR;
}
