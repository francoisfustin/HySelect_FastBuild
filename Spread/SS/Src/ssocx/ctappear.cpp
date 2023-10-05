/***********************************************************************
* ctappear.cpp -- Implementation of CTAppearance class
*
* Copyright (C) 2008-  FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#if SS_V80

#pragma optimize("", off)

#include "ctappear.h"
#include <fptstr.h>
#include "ssdll.h"

CCTAppearance::CCTAppearance(SS_CT_APPEARANCE *app)
{
	m_lRefCount = 1;
	m_pTypeInfo = NULL;
   if( app )
   {
      m_clrBackColor = app->clrBackColor;
      m_clrForeColor = app->clrForeColor;
      m_pFont = NULL;
      if( app->hFont )
      {
         LOGFONT  lf;
         FONTDESC fd = {0};

         if( GetObject(app->hFont, sizeof(LOGFONT), &lf) )
         {
            HDC   hdc = GetDC(0);
 	         long  lHeight;

            lHeight = lf.lfHeight;
            lHeight *= lHeight > 0 ? 72 : -72;
            lHeight *= 10000;
            lHeight /= ::GetDeviceCaps(hdc, LOGPIXELSY);
            fd.cbSizeofstruct = sizeof(FONTDESC);
            fd.cySize.Lo = lHeight;
            fd.cySize.Hi = 0;
            fd.sWeight = (short)lf.lfWeight;
            fd.sCharset = lf.lfCharSet;
            fd.fItalic = lf.lfItalic;
            fd.fUnderline = lf.lfUnderline;
            fd.fStrikethrough = lf.lfStrikeOut;
            fd.lpstrName = (LPOLESTR)fpSysAllocBSTRFromTString(lf.lfFaceName);
            OleCreateFontIndirect(&fd, IID_IFontDisp, (LPVOID*)&m_pFont);
            ReleaseDC(0, hdc);
         }
      }
      m_clrSelBackColor = app->clrSelBackColor;
      m_clrSelForeColor = app->clrSelForeColor;
      m_clrLockBackColor = app->clrLockBackColor;
      m_clrLockForeColor = app->clrLockForeColor;
      m_bDrawPrimaryButton = app->fDrawPrimaryButton ? VARIANT_TRUE : VARIANT_FALSE;
      m_bDrawSecondaryButton = app->fDrawSecondaryButton ? VARIANT_TRUE : VARIANT_FALSE;
      m_bDrawColors = app->fDrawColors ? VARIANT_TRUE : VARIANT_FALSE;
      m_lStyle = app->lStyle;
   }
}

CCTAppearance::~CCTAppearance()
{
	if (m_pTypeInfo)
   {
		m_pTypeInfo->Release();
      m_pTypeInfo = NULL;
   }
   if( m_pFont )
   {
      m_pFont->Release();
      m_pFont = NULL;
   }
}

HRESULT CCTAppearance::QueryInterface(REFIID iid, void** ppvObj)
{
	if (!ppvObj)
		return E_POINTER;
	*ppvObj = NULL;
	if (IID_IUnknown == iid || IID_IDispatch == iid || IID_ICTAppearance == iid)
		*ppvObj = this;

	if (NULL != *ppvObj)
    {
        ((LPUNKNOWN)*ppvObj)->AddRef();
        return S_OK;
	}

    return E_NOINTERFACE;
}

ULONG CCTAppearance::AddRef()
{
	return ++m_lRefCount;
}

ULONG CCTAppearance::Release()
{
   long tempCount;

   tempCount = --m_lRefCount;
   if(tempCount==0)
      delete this;
   return tempCount; 
}

HRESULT CCTAppearance::LoadTypeInfo()
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
	if (FAILED(hr = pTypeLib->GetTypeInfoOfGuid(IID_ICTAppearance, &m_pTypeInfo)))
		m_pTypeInfo = NULL;
	pTypeLib->Release();
	SysFreeString(bstrPath);
	return hr;
}

HRESULT CCTAppearance::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();

	return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgDispId);
}

HRESULT CCTAppearance::GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();
	if (!m_pTypeInfo)
		return E_UNEXPECTED;
	m_pTypeInfo->AddRef();
	*ppTInfo = m_pTypeInfo;
	return S_OK;	
}

HRESULT	CCTAppearance::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
	if (!pctinfo)
		return E_POINTER;
	*pctinfo = 1;
	return S_OK;
}

HRESULT CCTAppearance::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr)
{
	if (!m_pTypeInfo)
		LoadTypeInfo();
	return DispInvoke(this, m_pTypeInfo, dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}

HRESULT CCTAppearance::get_BackColor(OLE_COLOR *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_clrBackColor;
   return S_OK;
}

HRESULT CCTAppearance::put_BackColor(OLE_COLOR Val)
{
   m_clrBackColor = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_ForeColor(OLE_COLOR *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_clrForeColor;
   return S_OK;
}

HRESULT CCTAppearance::put_ForeColor(OLE_COLOR Val)
{
   m_clrForeColor = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_Font(LPFONTDISP *pVal)
{
   if( !pVal )
      return E_POINTER;
   if( m_pFont )
      m_pFont->AddRef();
   *pVal = m_pFont;
   return S_OK;
}

HRESULT CCTAppearance::put_Font(LPFONTDISP Val)
{
   if( m_pFont )
      m_pFont->Release();
   m_pFont = Val;
   if( m_pFont )
      m_pFont->AddRef();
   return S_OK;
}

HRESULT CCTAppearance::get_SelBackColor(OLE_COLOR *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_clrSelBackColor;
   return S_OK;
}

HRESULT CCTAppearance::put_SelBackColor(OLE_COLOR Val)
{
   m_clrSelBackColor = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_SelForeColor(OLE_COLOR *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_clrSelForeColor;
   return S_OK;
}

HRESULT CCTAppearance::put_SelForeColor(OLE_COLOR Val)
{
   m_clrSelForeColor = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_LockBackColor(OLE_COLOR *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_clrLockBackColor;
   return S_OK;
}

HRESULT CCTAppearance::put_LockBackColor(OLE_COLOR Val)
{
   m_clrLockBackColor = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_LockForeColor(OLE_COLOR *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_clrLockForeColor;
   return S_OK;
}

HRESULT CCTAppearance::put_LockForeColor(OLE_COLOR Val)
{
   m_clrLockForeColor = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_DrawPrimaryButton(VARIANT_BOOL *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_bDrawPrimaryButton;
   return S_OK;
}

HRESULT CCTAppearance::put_DrawPrimaryButton(VARIANT_BOOL Val)
{
   m_bDrawPrimaryButton = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_DrawSecondaryButton(VARIANT_BOOL *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_bDrawSecondaryButton;
   return S_OK;
}

HRESULT CCTAppearance::put_DrawSecondaryButton(VARIANT_BOOL Val)
{
   m_bDrawSecondaryButton = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_DrawColors(VARIANT_BOOL *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_bDrawColors;
   return S_OK;
}

HRESULT CCTAppearance::put_DrawColors(VARIANT_BOOL Val)
{
   m_bDrawColors = Val;
   return S_OK;
}

HRESULT CCTAppearance::get_Style(long *pVal)
{
   if( !pVal )
      return E_POINTER;
   *pVal = m_lStyle;
   return S_OK;
}

HRESULT CCTAppearance::put_Style(long Val)
{
   m_lStyle = Val;
   return S_OK;
}

#endif // SS_V80