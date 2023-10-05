/***********************************************************************
* ctappear.h - header for CTAppearance
*
* Copyright (C) 2008 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#ifndef __CT_APPEARANCE_H
#define __CT_APPEARANCE_H

#ifdef SS_V80

#include "stdafx.h"

// {716346AD-37FC-45b4-B3E8-60E49F102BBB}
const GUID CDECL BASED_CODE IID_ICTAppearance = 
{ 0x716346ad, 0x37fc, 0x45b4, { 0xb3, 0xe8, 0x60, 0xe4, 0x9f, 0x10, 0x2b, 0xbb } };

class ICTAppearance : public IDispatch
{
public:
   STDMETHOD(get_BackColor)(OLE_COLOR *pVal) = 0;
   STDMETHOD(put_BackColor)(OLE_COLOR Val) = 0;
   STDMETHOD(get_ForeColor)(OLE_COLOR *pVal) = 0;
   STDMETHOD(put_ForeColor)(OLE_COLOR Val) = 0;
   STDMETHOD(get_Font)(LPFONTDISP *pVal) = 0;
   STDMETHOD(put_Font)(LPFONTDISP Val) = 0;
   STDMETHOD(get_SelBackColor)(OLE_COLOR *pVal) = 0;
   STDMETHOD(put_SelBackColor)(OLE_COLOR Val) = 0;
   STDMETHOD(get_SelForeColor)(OLE_COLOR *pVal) = 0;
   STDMETHOD(put_SelForeColor)(OLE_COLOR Val) = 0;
   STDMETHOD(get_LockBackColor)(OLE_COLOR *pVal) = 0;
   STDMETHOD(put_LockBackColor)(OLE_COLOR Val) = 0;
   STDMETHOD(get_LockForeColor)(OLE_COLOR *pVal) = 0;
   STDMETHOD(put_LockForeColor)(OLE_COLOR Val) = 0;
   STDMETHOD(get_DrawPrimaryButton)(VARIANT_BOOL *pVal) = 0;
   STDMETHOD(put_DrawPrimaryButton)(VARIANT_BOOL Val) = 0;
   STDMETHOD(get_DrawSecondaryButton)(VARIANT_BOOL *pVal) = 0;
   STDMETHOD(put_DrawSecondaryButton)(VARIANT_BOOL Val) = 0;
   STDMETHOD(get_DrawColors)(VARIANT_BOOL *pVal) = 0;
   STDMETHOD(put_DrawColors)(VARIANT_BOOL Val) = 0;
   STDMETHOD(get_Style)(long *plVal) = 0;
   STDMETHOD(put_Style)(long Val) = 0;
};

class CCTAppearance : public ICTAppearance
{

private:
   OLE_COLOR   m_clrBackColor;
   OLE_COLOR   m_clrForeColor;
   OLE_COLOR   m_clrSelBackColor;
   OLE_COLOR   m_clrSelForeColor;
   OLE_COLOR   m_clrLockBackColor;
   OLE_COLOR   m_clrLockForeColor;
   LPFONTDISP  m_pFont;
   VARIANT_BOOL        m_bDrawPrimaryButton;
   VARIANT_BOOL        m_bDrawSecondaryButton;
   VARIANT_BOOL        m_bDrawColors;
	ITypeInfo*	m_pTypeInfo;	// ptr to typeinfo for this object
	long			m_lRefCount;
   long        m_lStyle;

   HRESULT     LoadTypeInfo();		// loads the typeinfo for this object

public:
//   CCTAppearance();
   CCTAppearance(SS_CT_APPEARANCE *app);
   ~CCTAppearance();
   // IDispatch:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();
	STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId); 
	STDMETHOD(GetTypeInfo)(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo); 
	STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo); 
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr); 
   // ICTAppearance:
   STDMETHOD(get_BackColor)(OLE_COLOR *pVal);
   STDMETHOD(put_BackColor)(OLE_COLOR Val);
   STDMETHOD(get_ForeColor)(OLE_COLOR *pVal);
   STDMETHOD(put_ForeColor)(OLE_COLOR Val);
   STDMETHOD(get_Font)(LPFONTDISP *pVal);
   STDMETHOD(put_Font)(LPFONTDISP Val);
   STDMETHOD(get_SelBackColor)(OLE_COLOR *pVal);
   STDMETHOD(put_SelBackColor)(OLE_COLOR Val);
   STDMETHOD(get_SelForeColor)(OLE_COLOR *pVal);
   STDMETHOD(put_SelForeColor)(OLE_COLOR Val);
   STDMETHOD(get_LockBackColor)(OLE_COLOR *pVal);
   STDMETHOD(put_LockBackColor)(OLE_COLOR Val);
   STDMETHOD(get_LockForeColor)(OLE_COLOR *pVal);
   STDMETHOD(put_LockForeColor)(OLE_COLOR Val);
   STDMETHOD(get_DrawPrimaryButton)(VARIANT_BOOL *pVal);
   STDMETHOD(put_DrawPrimaryButton)(VARIANT_BOOL Val);
   STDMETHOD(get_DrawSecondaryButton)(VARIANT_BOOL *pVal);
   STDMETHOD(put_DrawSecondaryButton)(VARIANT_BOOL Val);
   STDMETHOD(get_DrawColors)(VARIANT_BOOL *pVal);
   STDMETHOD(put_DrawColors)(VARIANT_BOOL Val);
   STDMETHOD(get_Style)(long *pVal);
   STDMETHOD(put_Style)(long Val);
};

#endif // SS_V80

#endif // !__CT_APPEARANCE_H
