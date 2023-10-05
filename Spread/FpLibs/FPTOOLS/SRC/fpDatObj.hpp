/***********************************************************************
* FPDATOBJ.Hpp - implementation for XfpDataObject class override for
*				 COleControl::XDataObject 
*
* Copyright (C) 1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/
// this file should be included in the file containing 
// the implementation for your control.
// NOTE: FPMYDISP_CTRL_NAME must be declared BEFORE including this file!!!
#ifndef FPMYDISP_CTRL_NAME
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" before including fpprpbag.h ***" ) 
#endif

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XfpDataObject::AddRef()
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XfpDataObject::Release()
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return pThis->ExternalRelease();
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::QueryInterface(REFIID iid, void FAR* FAR* ppvObj)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::GetData(LPFORMATETC pFormatetc, LPSTGMEDIUM pMedium)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->GetData(pFormatetc, pMedium);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::GetDataHere(LPFORMATETC pFormat, LPSTGMEDIUM pMedium)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->GetDataHere(pFormat, pMedium);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::QueryGetData(LPFORMATETC pFormatetc)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->QueryGetData(pFormatetc);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::GetCanonicalFormatEtc(LPFORMATETC pFormatetc, LPFORMATETC pFormatetcOut)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->GetCanonicalFormatEtc(pFormatetc, pFormatetcOut);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::SetData(LPFORMATETC pFormatetc, STGMEDIUM* pMedium, int fRelease)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->SetData(pFormatetc, pMedium, fRelease);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::EnumFormatEtc(ULONG dwDirection, LPENUMFORMATETC FAR* ppenumFormatetc)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->EnumFormatEtc(dwDirection, ppenumFormatetc);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::DAdvise(FORMATETC* pFormatetc, ULONG advf, LPADVISESINK pAdvise, ULONG* pdwConnection)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->DAdvise(pFormatetc, advf, pAdvise, pdwConnection);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::DUnadvise(ULONG dwConnection)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->DUnadvise(dwConnection);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::EnumDAdvise(LPENUMSTATDATA* ppEnumAdvise)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	return DEFAULT_XDATAOBJECT_PTR(pThis)->EnumDAdvise(ppEnumAdvise);
}
