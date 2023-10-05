/***********************************************************************
* PvOleDrg.cpp -- OLE Drag and drop support for Preview OCX controls
*
* Copyright (C) 1991-1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/
#include "gdicopy.h"

#ifndef FPMYDISP_CTRL_NAME
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" before including SSOleDrg.cpp ***" ) 
#endif

#ifdef NO_MFC	// ATL implementation:

#ifndef FPBASECLS
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" before including SSOleDrg.cpp ***" ) 
#endif

STDMETHODIMP FPMYDISP_CTRL_NAME::QueryContinueDrag(int fEscapePressed, ULONG grfKeyState)
{
	// need to check if mouse button was released or escape pressed
	if (fEscapePressed)
		return DRAGDROP_S_CANCEL;
	else if (!(grfKeyState & m_nDragButton))
		return DRAGDROP_S_DROP;
	else
		return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::GiveFeedback(ULONG dwEffect)
{
	BOOL defCursors = -1;

/*
#ifdef DRAGAUTO
	if (dwEffect & DROPEFFECT_COPY && ((short)GetAsyncKeyState(VK_CONTROL) & 0x8000))
		dwEffect = DROPEFFECT_COPY;
	else if (dwEffect & DROPEFFECT_MOVE)
		dwEffect = DROPEFFECT_MOVE;
#endif
*/
	Fire_OLEGiveFeedback((long*)&dwEffect, &defCursors);
	if (dwEffect && defCursors)
		return DRAGDROP_S_USEDEFAULTCURSORS;
	else
		return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::OnDragOver(LPDATAOBJECT pDataObject, ULONG* pdwEffect, ULONG grfKeyState, POINTL pt, short State)
{   // need to store previous data object since this is not provided
	// in IDropTarget::OnDragOver() or IDropTarget::OnDragLeave(). -scl

	short			Button = (grfKeyState & MK_LBUTTON ? 1 : 0) +
							 (grfKeyState & MK_RBUTTON ? 2 : 0) +
							 (grfKeyState & MK_MBUTTON ? 4 : 0);
	short			Shift  = (grfKeyState & MK_SHIFT   ? 1 : 0) +
							 (grfKeyState & MK_CONTROL ? 2 : 0) +
							 (grfKeyState & MK_ALT     ? 4 : 0);
	FPCONTROL		Ctl = {m_hWnd, 0, (long)this, 0L};
	SCALEMODE		sm;
	float			x, y;
	BOOL			fDelete = FALSE;
	LPDATAOBJECT	pMyDataObject = NULL;

	ControlQueryInterface(IID_IDataObject, (void**)&pMyDataObject);
	ControlQueryInterface(IID_IDispatch, (void**)&Ctl.lpDispatch);
	fpVBGetScaleMode(&Ctl, &sm, TRUE);
	fpPixelsToScaleMode(&sm, pt.x, pt.y, &x, &y, FALSE);
	((LPDISPATCH)Ctl.lpDispatch)->Release();

	if (pDataObject && !m_pfpDataObject)
	{	// DragEnter
		// should be NULL, unless this object is 
		// also the drop source
		// in which case pDataObject == &m_xfpDataObject
		m_pfpDataObject = new CfpDataObject(pDataObject);
	}
	else if (State == 1)	// DragLeave
		fDelete = TRUE;		// Release fpDataObject after firing event
	if (pdwEffect)
		m_lEffect = (long)*pdwEffect;
	Fire_OLEDragOver(&m_pfpDataObject, &m_lEffect, &Button, &Shift, &x, &y, &State);
	if (pdwEffect && *pdwEffect != (ULONG)m_lEffect)
		*pdwEffect = m_lEffect;
	if (fDelete && m_pfpDataObject->GetIDataObject(FALSE) != pMyDataObject)
	{	// don't delete if this object is the drop source
		m_pfpDataObject->Release();
		m_pfpDataObject = NULL;
	}
	pMyDataObject->Release();
	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::DragEnter(LPDATAOBJECT pDataObject, ULONG grfKeyState, POINTL pt, ULONG * pdwEffect)
{
	ScreenToClient((LPPOINT)&pt);
	return OnDragOver(pDataObject, pdwEffect, grfKeyState, pt, 0);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::DragLeave()
{
	ULONG Effect = 0L;
	POINTL pt = {0L,0L};
	ULONG grfKeyState =	(GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? MK_LBUTTON : 0) +
								(GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? MK_RBUTTON : 0) +
								(GetAsyncKeyState(VK_MBUTTON) & 0x8000 ? MK_MBUTTON : 0) +
								(GetAsyncKeyState(VK_SHIFT  ) & 0x8000 ? MK_SHIFT   : 0) +
								(GetAsyncKeyState(VK_MENU   ) & 0x8000 ? MK_ALT     : 0) +
								(GetAsyncKeyState(VK_CONTROL) & 0x8000 ? MK_CONTROL : 0);

	return OnDragOver(NULL, &Effect, grfKeyState, pt, 1);
}

HRESULT FPMYDISP_CTRL_NAME::DragOver(ULONG grfKeyState, POINTL pt, ULONG * pdwEffect)
{
	ScreenToClient((LPPOINT)&pt);
	return OnDragOver(NULL, pdwEffect, grfKeyState, pt, 2);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::Drop(LPDATAOBJECT pDataObject, ULONG grfKeyState, POINTL pt, ULONG * pdwEffect)
{
	DWORD dwKeyState =	(GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? MK_LBUTTON : 0) +
						(GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? MK_RBUTTON : 0) +
						(GetAsyncKeyState(VK_MBUTTON) & 0x8000 ? MK_MBUTTON : 0) +
						(GetAsyncKeyState(VK_SHIFT  ) & 0x8000 ? MK_SHIFT   : 0) +
						(GetAsyncKeyState(VK_MENU   ) & 0x8000 ? MK_ALT     : 0) +
						(GetAsyncKeyState(VK_CONTROL) & 0x8000 ? MK_CONTROL : 0);
	short Button =	(dwKeyState & MK_LBUTTON ? 1 : 0) +
					(dwKeyState & MK_RBUTTON ? 2 : 0) +
					(dwKeyState & MK_MBUTTON ? 4 : 0);
	short Shift  =	(dwKeyState & MK_SHIFT   ? 1 : 0) +
					(dwKeyState & MK_CONTROL ? 2 : 0) +
					(dwKeyState & MK_ALT     ? 4 : 0);
	FPCONTROL Ctl = {fpAtlGetHWnd(), 0, (long)this, 0L};
	LPDATAOBJECT pMyDataObject = NULL;
	SCALEMODE sm;
	float x, y;

	ControlQueryInterface(IID_IDataObject, (void**)&pMyDataObject);
	ControlQueryInterface(IID_IDispatch, (void**)&Ctl.lpDispatch);
	ScreenToClient((LPPOINT)&pt);
	fpVBGetScaleMode(&Ctl, &sm, TRUE);
	fpPixelsToScaleMode(&sm, pt.x, pt.y, &x, &y, FALSE);
	((LPDISPATCH)Ctl.lpDispatch)->Release();

	Fire_OLEDragDrop(&m_pfpDataObject, (long*)pdwEffect, &Button, &Shift, &x, &y);
	// check to make sure this control is not the drop source,
	// in which case OLEDrag will release the fpDataObject
	if (m_pfpDataObject->GetIDataObject(FALSE) != pMyDataObject)
	{	// then release fpDataObject
		m_pfpDataObject->Release();
		m_pfpDataObject = NULL;
	}
	pMyDataObject->Release();
	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::GetData(LPFORMATETC pFormat, LPSTGMEDIUM pMedium)
{
    HRESULT hr;
	short   i;

	if (!pFormat || !pMedium)
		return E_POINTER;
	if (pFormat->lindex != -1)
		return DV_E_LINDEX;
	if (pFormat->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	// check to see if OLESetData event should fire
	// determine which array element is being requested
	i = ClipFormatToArrayIndex(pFormat->cfFormat);
	if (-1 != i && m_pfpDataObject && m_pfpDataObject->m_fFireOLESetData[i])
	{
		// check to see if pFormat->cfFormat needs to be changed to the correct value
		// for vbCFRTF 
		if (pFormat->cfFormat == 0xC075)
			pFormat->cfFormat = 0xBF01;
		// prevent recursion
		m_pfpDataObject->m_fFireOLESetData[i] = FALSE;
		Fire_OLESetData(&m_pfpDataObject, (short*)&pFormat->cfFormat);
		// then change it back again
		if (pFormat->cfFormat == 0xBF01)
			pFormat->cfFormat = 0xC075;
	}
	// and validate the format
	if (-1 == i)
		return DV_E_FORMATETC;

	switch (pFormat->cfFormat)
	{	// copy the data from the array to the medium passed in
	case CF_TEXT:
	case 0xC075: // Rich Text
	case CF_HDROP:
	case CF_DIB:
		if (m_Mediums[i].tymed != TYMED_HGLOBAL || !(pFormat->tymed & TYMED_HGLOBAL))
			return DV_E_TYMED;
		else
		{	// copy the HGLOBAL
			if (S_OK != (hr = CopyHGlobal(m_Mediums[i].hGlobal, &pMedium->hGlobal)))
				return hr;
			pMedium->tymed = TYMED_HGLOBAL;
		}			
		break;
	case CF_BITMAP:
		if (m_Mediums[i].tymed != TYMED_GDI || !(pFormat->tymed & TYMED_GDI))
			return DV_E_TYMED;
		else
		{	// copy the HBITMAP
			if (S_OK != (hr = CopyHBitmap(m_Mediums[i].hBitmap, &pMedium->hBitmap)))
				return hr;
			pMedium->tymed = TYMED_GDI;
		}
		break;
	case CF_PALETTE:
		if (m_Mediums[i].tymed != TYMED_GDI || !(pFormat->tymed & TYMED_GDI))
			return DV_E_TYMED;
		else
		{	// copy the HPALETTE
			if (S_OK != (hr = CopyHPalette((HPALETTE)m_Mediums[i].hBitmap, (HPALETTE*)&pMedium->hBitmap)))
				return hr;
			pMedium->tymed = TYMED_GDI;
		}
		break;
	case CF_METAFILEPICT:
		if (m_Mediums[i].tymed != TYMED_MFPICT || !(pFormat->tymed & TYMED_MFPICT))
			return DV_E_TYMED;
		else
		{	// copy the HMETAFILEPICT
			if (S_OK != (hr = CopyHMetaFilePict(m_Mediums[i].hMetaFilePict, &pMedium->hMetaFilePict)))
				return hr;
			pMedium->tymed = TYMED_MFPICT;
		}
		break;
	case CF_ENHMETAFILE:
		if (m_Mediums[i].tymed != TYMED_ENHMF || !(pFormat->tymed & TYMED_ENHMF))
			return DV_E_TYMED;
		else
		{	// copy the HENHMETAFILE
			if (S_OK != (hr = CopyHEnhMetaFile(m_Mediums[i].hEnhMetaFile, &pMedium->hEnhMetaFile)))
				return hr;
			pMedium->tymed = TYMED_ENHMF;
		}
		break;
	}
	pMedium->pUnkForRelease = NULL;
	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::GetDataHere(LPFORMATETC pFormat, LPSTGMEDIUM pMedium)
{	// this method cannot be called for GDI objects and is optional for HGLOBALs
	return E_NOTIMPL;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::QueryGetData(LPFORMATETC pFormat)
{
	short i;

	// determine which array element is being requested
	// and validate the format
	if (!pFormat)
		return E_POINTER;
	if (pFormat->lindex != -1)
		return DV_E_LINDEX;
	if (pFormat->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	if (pFormat->tymed == TYMED_NULL)
		return DV_E_TYMED;

	// check to see if pFormat->cfFormat needs to be changed to the correct value
	// for vbCFRTF 
	if (pFormat->cfFormat == 0xBF01)
		pFormat->cfFormat = 0xC075;

	i = ClipFormatToArrayIndex(pFormat->cfFormat);
	if (-1 == i)
		return DV_E_FORMATETC;
	if (m_Mediums[i].tymed == TYMED_NULL)
		return S_FALSE;
	else
		return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::GetCanonicalFormatEtc(LPFORMATETC pFormatetc, LPFORMATETC pFormatetcOut)
{
	return E_NOTIMPL;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::SetData(LPFORMATETC pFormat, LPSTGMEDIUM pMedium, BOOL fRelease)
{
	short i;
	HRESULT hr;
	STGMEDIUM medium = {TYMED_NULL, 0L, NULL};

	if (!pFormat || !pMedium)
		return E_POINTER;
	if (pFormat->lindex != -1)
		return DV_E_LINDEX;
	if (pFormat->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;

    if (pFormat->tymed != pMedium->tymed)
		return DV_E_TYMED;

	// default handles metafile, IStream, and IStorage
	// don't let it handle the metafile format
	if (TYMED_MFPICT != pFormat->tymed && S_OK == (hr = FPBASECLS::SetData(pFormat, pMedium, fRelease)))
		return hr;

	// special case: when TYMED_NULL is passed for both the format and medium,
	// then clear out all data in the object (for the Clear method of the fpDataObject)
	if (TYMED_NULL == pMedium->tymed)
	{
		for (i=0; i<8; i++)
		{
			if (m_Mediums[i].pUnkForRelease)
				m_Mediums[i].pUnkForRelease->Release();
			else
				ReleaseStgMedium(&m_Mediums[i]);
			m_Mediums[i].tymed = TYMED_NULL;
		}
		return S_OK;
	}

	switch (pFormat->cfFormat)
	{	// copy the data to local medium
	case CF_TEXT:
	case 0xC075: // Rich Text
	case CF_HDROP:
	case CF_DIB:
		if (pMedium->tymed != TYMED_HGLOBAL)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hGlobal)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HGLOBAL
			if (S_OK != (hr = CopyHGlobal(pMedium->hGlobal, &medium.hGlobal)))
				return hr;
			medium.tymed = TYMED_HGLOBAL;
		}
		break;
	case CF_BITMAP:
		if (pMedium->tymed != TYMED_GDI)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hBitmap)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HBITMAP
			if (S_OK != (hr = CopyHBitmap(pMedium->hBitmap, &medium.hBitmap)))
				return hr;
			medium.tymed = TYMED_GDI;
		}
		break;
	case CF_PALETTE:
		if (pMedium->tymed != TYMED_GDI)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hBitmap)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HPALETTE
			if (S_OK != (hr = CopyHPalette((HPALETTE)pMedium->hBitmap, (HPALETTE*)&medium.hBitmap)))
				return hr;
			medium.tymed = TYMED_GDI;
		}
		break;
	case CF_METAFILEPICT:
		if (pMedium->tymed != TYMED_MFPICT)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hMetaFilePict)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HMETAFILEPICT
			if (S_OK != (hr = CopyHMetaFilePict(pMedium->hMetaFilePict, &medium.hMetaFilePict)))
				return hr;
			medium.tymed = TYMED_MFPICT;
		}
		break;
	case CF_ENHMETAFILE:
		if (pMedium->tymed != TYMED_ENHMF)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hEnhMetaFile)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HENHMETAFILE
			if (S_OK != (hr = CopyHEnhMetaFile(pMedium->hEnhMetaFile, &medium.hEnhMetaFile)))
				return hr;
			medium.tymed = TYMED_ENHMF;
		}
		break;
	default:
		return DV_E_FORMATETC;
	}
	// copy the format and medium to array
	i = ClipFormatToArrayIndex(pFormat->cfFormat);
	m_Formats[i] = *pFormat;
	if (TYMED_NULL != m_Mediums[i].tymed)
	{	// data was already set for this entry in the array,
		// must release it
		if (m_Mediums[i].pUnkForRelease)
			m_Mediums[i].pUnkForRelease->Release();
		else
			ReleaseStgMedium(&m_Mediums[i]);
	}
	m_Mediums[i] = medium;

	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::EnumFormatEtc(ULONG dwDirection, LPENUMFORMATETC FAR* ppenumFormatetc)
{
	if (!ppenumFormatetc)
		return E_POINTER;
	
	CfpEnumFormatEtc* pEnum = new CfpEnumFormatEtc(this, dwDirection);
	*ppenumFormatetc = (IEnumFORMATETC*)pEnum;
	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::DAdvise(FORMATETC* pFormatetc, ULONG advf, LPADVISESINK pAdvise, ULONG* pdwConnection)
{
	return FPBASECLS::DAdvise(pFormatetc, advf, pAdvise, pdwConnection);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::DUnadvise(ULONG dwConnection)
{
	return FPBASECLS::DUnadvise(dwConnection);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::EnumDAdvise(LPENUMSTATDATA* ppEnumAdvise)
{
	return FPBASECLS::EnumDAdvise(ppEnumAdvise);
}

short FPMYDISP_CTRL_NAME::ClipFormatToArrayIndex(CLIPFORMAT cf)
{
	switch((WORD)cf)
	{
	case CF_TEXT:			return 0;
	case CF_BITMAP:			return 1;
	case CF_METAFILEPICT:	return 2;
	case CF_DIB:			return 3;
	case CF_PALETTE:		return 4;
	case CF_ENHMETAFILE:	return 5;
	case CF_HDROP:			return 6;
	case 0xC075:
	case 0xBF01:			return 7;	//	Rich Text
	default:				return -1;
	}
}

STDMETHODIMP FPMYDISP_CTRL_NAME::OLEDrag(void)
{
	long AllowedEffects = 0L;
	LPDATAOBJECT pDataObject = NULL;
	IDropSource* pDropSource = NULL;

	// create fpDataObject
	ControlQueryInterface(IID_IDataObject, (void**)&pDataObject);
	ASSERT(!m_pfpDataObject);	// should be NULL
	m_pfpDataObject = new CfpDataObject(pDataObject);
	if (!m_pfpDataObject)
		return E_OUTOFMEMORY;

	// clear the data object
	m_pfpDataObject->Clear();

	Fire_OLEStartDrag(&m_pfpDataObject, &AllowedEffects);
	// user must (1) change AllowedEffects to non-zero value, and
	// (2) specify a data format for the data object, or
	// the drag does not happen
	if (AllowedEffects && m_pfpDataObject->m_fFormatSet)
	{
		ULONG ResultEffect = 0L;
		m_nDragButton = 0;
		// check to see which button is down for the drag
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			m_nDragButton |= VK_LBUTTON;
		if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
			m_nDragButton |= VK_RBUTTON;
		if (GetAsyncKeyState(VK_MBUTTON) & 0x8000)
			m_nDragButton |= VK_MBUTTON;
		ControlQueryInterface(IID_IDropSource, (void**)&pDropSource);
//		m_bDragging = TRUE;
		::DoDragDrop(pDataObject, pDropSource, AllowedEffects, &ResultEffect);
//		m_bDragging = FALSE;
		pDropSource->Release();
		Fire_OLECompleteDrag((long*)&ResultEffect);
	}
	m_pfpDataObject->Clear();
	m_pfpDataObject->Release();
	m_pfpDataObject = NULL;
	pDataObject->Release();
	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::get_OLEDropMode(OLEDropModeConstants* pVal)
{
	if (!pVal)
		return E_POINTER;
	*pVal = m_OLEDropMode;
	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::put_OLEDropMode(OLEDropModeConstants newValue)
{
	if (newValue >=0 && newValue <= 1)
	{
		m_OLEDropMode = newValue;
		if (fpAtlGetUserMode())
		{
			IDropTarget* pDropTarget = NULL;
			ControlQueryInterface(IID_IDropTarget, (void**)&pDropTarget);
			if (m_OLEDropMode)
				::RegisterDragDrop(fpAtlGetHWnd(), pDropTarget);
			else
				::RevokeDragDrop(fpAtlGetHWnd());
			pDropTarget->Release();
		}
	}
	else
		return E_INVALIDARG;
	return S_OK;
}

#ifndef EXCLUDE_OLEDRAGMODE
STDMETHODIMP FPMYDISP_CTRL_NAME::get_OLEDragMode(OLEDragModeConstants* pVal)
{
	if (!pVal)
		return E_POINTER;
	*pVal = m_OLEDragMode;
	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::put_OLEDragMode(OLEDragModeConstants newValue)
{
	if (newValue >=0 && newValue <= 1)
		m_OLEDragMode = newValue;
	else
		return E_INVALIDARG;
	return S_OK;
}
#endif

#else	// MFC implementation:

#define DEFAULT_XDATAOBJECT_PTR(pCtl)   \
  ((IDataObject FAR *)&(pCtl->m_xDataObject))

#define DEFAULT_XFPDATAOBJECT_PTR(pCtl) \
  ((IDataObject FAR *)&(pCtl->m_xfpDataObject))

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XfpDropSource::AddRef()
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropSource)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XfpDropSource::Release()
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropSource)
	return pThis->ExternalRelease();
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDropSource::QueryInterface(REFIID iid, void FAR* FAR* ppvObj)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropSource)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDropSource::QueryContinueDrag(int fEscapePressed, ULONG grfKeyState)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropSource)
	// need to check if mouse button was released or escape pressed
	if (fEscapePressed)
		return DRAGDROP_S_CANCEL;
	else if (!(grfKeyState & m_nDragButton))
		return DRAGDROP_S_DROP;
	else
		return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDropSource::GiveFeedback(ULONG dwEffect)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropSource)
	BOOL defCursors = -1;

#ifdef SS_V35
    {
    LPSSPREVIEW lpPV = PV_Lock(pThis->m_hWnd);

    if (lpPV->bScriptEnhanced)
       {
       VARIANT arg[2];
       VARIANT varParam[2];

       memset(&arg, 0, sizeof(VARIANT) * 2);
       memset(&varParam, 0, sizeof(VARIANT) * 2);

       varParam[0].vt = VT_UI4; 
       varParam[0].ulVal  = dwEffect; 

       varParam[1].vt = VT_BOOL; 
       varParam[1].boolVal  = (defCursors ? VARIANT_TRUE : VARIANT_FALSE); 

       pThis->FireScriptOLEGiveFeedback(&varParam[0], &varParam[1]);

       VariantChangeType(&arg[0], &varParam[0], 0, VT_UI4); 
       VariantChangeType(&arg[1], &varParam[1], 0, VT_BOOL); 
       dwEffect = arg[0].ulVal;
       defCursors = (arg[1].boolVal == VARIANT_TRUE ? TRUE : FALSE);
       }
    else
       pThis->FireOLEGiveFeedback((long*)&dwEffect, &defCursors);
    PV_Unlock(pThis->m_hWnd);
    }
#else
	pThis->FireOLEGiveFeedback((long*)&dwEffect, &defCursors);
#endif  // SS_V35

	if (dwEffect && defCursors)
		return DRAGDROP_S_USEDEFAULTCURSORS;
	else
		return S_OK;
}

STDMETHODIMP_(ULONG) FAR EXPORT FPMYDISP_CTRL_NAME::XfpDropTarget::AddRef()
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropTarget)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XfpDropTarget::Release()
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropTarget)
	return pThis->ExternalRelease();
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDropTarget::QueryInterface(REFIID iid, void FAR* FAR* ppvObj)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropTarget)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::OnDragOver(LPDATAOBJECT pDataObject, ULONG* pdwEffect, ULONG grfKeyState, POINTL pt, short State)
{   // need to store previous data object since this is not provided
	// in IDropTarget::OnDragOver() or IDropTarget::OnDragLeave(). -scl

	//if (DEFAULT_XDATAOBJECT_PTR(this))
	//{
		short Button = (grfKeyState & MK_LBUTTON ? 1 : 0) +
					   (grfKeyState & MK_RBUTTON ? 2 : 0) +
					   (grfKeyState & MK_MBUTTON ? 4 : 0);
		short Shift  = (grfKeyState & MK_SHIFT   ? 1 : 0) +
					   (grfKeyState & MK_CONTROL ? 2 : 0) +
					   (grfKeyState & MK_ALT     ? 4 : 0);
		FPCONTROL Ctl = {m_hWnd, 0, (long)this, (long)GetIDispatch(FALSE)};
		SCALEMODE sm;
		float x, y;
		BOOL fDelete = FALSE;

		fpVBGetScaleMode(&Ctl, &sm, TRUE);
		fpPixelsToScaleMode(&sm, pt.x, pt.y, &x, &y, FALSE);

		if (pDataObject && !m_pfpDataObject)
		{	// DragEnter
			// should be NULL, unless this object is 
			// also the drop source
			// in which case pDataObject == &m_xfpDataObject
			m_pfpDataObject = new CfpDataObject(pDataObject);
		}
		else if (State == 1)	// DragLeave
			fDelete = TRUE;		// Release fpDataObject after firing event
		if (pdwEffect)
			m_lEffect = (long)*pdwEffect;
#ifdef SS_V35
	    {
        LPSSPREVIEW lpPV = PV_Lock(m_hWnd);

        if (lpPV->bScriptEnhanced)
           {
           VARIANT arg;
           VARIANT varParam;
           VARIANT data;
           VARIANT varButton;
           VARIANT varShift;
           VARIANT varx;
           VARIANT vary;
           VARIANT varState;

           memset(&arg, 0, sizeof(VARIANT));
           memset(&varParam, 0, sizeof(VARIANT));
           memset(&data, 0, sizeof(VARIANT));
           memset(&varButton, 0, sizeof(VARIANT));
           memset(&varShift, 0, sizeof(VARIANT));
           memset(&varx, 0, sizeof(VARIANT));
           memset(&vary, 0, sizeof(VARIANT));
           memset(&varState, 0, sizeof(VARIANT));

           varParam.vt = VT_I4; 
           varParam.lVal  = m_lEffect; 

           data.vt = VT_DISPATCH;
           data.pdispVal = (IDispatch*)m_pfpDataObject;

           varButton.vt = VT_I2;
           varButton.iVal = Button;

           varShift.vt = VT_I2;
           varShift.iVal = Shift;

           varx.vt = VT_R4;
           varx.fltVal = x;

           vary.vt = VT_R4;
           vary.fltVal = y;

           varState.vt = VT_I2;
           varState.iVal = State;

//		   FireScriptOLEDragOver(&m_pfpDataObject, &varParam, &Button, &Shift, &x, &y, &State);
//		   FireScriptOLEDragOver(&data, &varParam, &Button, &Shift, &x, &y, &State);
		   FireScriptOLEDragOver(&data, &varParam, &varButton, &varShift, &varx, &vary, &varState);

           VariantChangeType(&arg, &varParam, 0, VT_UI4); 
           m_lEffect = arg.ulVal;
           }
        else
	       FireOLEDragOver(&m_pfpDataObject, &m_lEffect, &Button, &Shift, &x, &y, &State);
        PV_Unlock(m_hWnd);
        }
#else
		FireOLEDragOver(&m_pfpDataObject, &m_lEffect, &Button, &Shift, &x, &y, &State);
#endif

		if (pdwEffect && *pdwEffect != (ULONG)m_lEffect)
			*pdwEffect = m_lEffect;
		if (fDelete && m_pfpDataObject->GetIDataObject(FALSE) != &m_xfpDataObject)
		{	// don't delete if this object is the drop source
			m_pfpDataObject->Release();
			m_pfpDataObject = NULL;
		}
		return S_OK;
	//}
	//else
	//	return (HRESULT)E_UNEXPECTED;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDropTarget::DragEnter(LPDATAOBJECT pDataObject, ULONG grfKeyState, POINTL pt, ULONG * pdwEffect)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropTarget)
	//if (DEFAULT_XDROPTARGET_PTR(pThis))
	//{
		//DEFAULT_XDROPTARGET_PTR(pThis)->DragEnter(pDataObject, grfKeyState, pt, pdwEffect);
		pThis->ScreenToClient((LPPOINT)&pt);
		return pThis->OnDragOver(pDataObject, pdwEffect, grfKeyState, pt, 0);
	//}
	//else
	//	return (HRESULT)E_UNEXPECTED;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDropTarget::DragLeave()
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropTarget)
	//if (DEFAULT_XDROPTARGET_PTR(pThis))
	//{
		ULONG Effect = 0L;// = (long)DEFAULT_XDROPTARGET_PTR(pThis)->DragLeave();
		POINTL pt = {0L,0L};
		ULONG grfKeyState =	(GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? MK_LBUTTON : 0) +
									(GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? MK_RBUTTON : 0) +
									(GetAsyncKeyState(VK_MBUTTON) & 0x8000 ? MK_MBUTTON : 0) +
									(GetAsyncKeyState(VK_SHIFT  ) & 0x8000 ? MK_SHIFT   : 0) +
									(GetAsyncKeyState(VK_MENU   ) & 0x8000 ? MK_ALT     : 0) +
									(GetAsyncKeyState(VK_CONTROL) & 0x8000 ? MK_CONTROL : 0);

		return pThis->OnDragOver(NULL, &Effect, grfKeyState, pt, 1);
	//}
	//else
	//	return (HRESULT)E_UNEXPECTED;
}

HRESULT FPMYDISP_CTRL_NAME::XfpDropTarget::DragOver(ULONG grfKeyState, POINTL pt, ULONG * pdwEffect)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropTarget)
	//if (DEFAULT_XDROPTARGET_PTR(pThis))
	//{
		//long Effect = (long)DEFAULT_XDROPTARGET_PTR(pThis)->DragOver(grfKeyState, pt, pdwEffect);
		pThis->ScreenToClient((LPPOINT)&pt);
		return pThis->OnDragOver(NULL, pdwEffect, grfKeyState, pt, 2);
	//}
	//else
	//	return (HRESULT)E_UNEXPECTED;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDropTarget::Drop(LPDATAOBJECT pDataObject, ULONG grfKeyState, POINTL pt, ULONG * pdwEffect)
{
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDropTarget)
	//if (DEFAULT_XDATAOBJECT_PTR(pThis))
	//{
		DWORD dwKeyState =	(GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? MK_LBUTTON : 0) +
							(GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? MK_RBUTTON : 0) +
							(GetAsyncKeyState(VK_MBUTTON) & 0x8000 ? MK_MBUTTON : 0) +
							(GetAsyncKeyState(VK_SHIFT  ) & 0x8000 ? MK_SHIFT   : 0) +
							(GetAsyncKeyState(VK_MENU   ) & 0x8000 ? MK_ALT     : 0) +
							(GetAsyncKeyState(VK_CONTROL) & 0x8000 ? MK_CONTROL : 0);
		short Button =	(dwKeyState & MK_LBUTTON ? 1 : 0) +
						(dwKeyState & MK_RBUTTON ? 2 : 0) +
						(dwKeyState & MK_MBUTTON ? 4 : 0);
		short Shift  =	(dwKeyState & MK_SHIFT   ? 1 : 0) +
						(dwKeyState & MK_CONTROL ? 2 : 0) +
						(dwKeyState & MK_ALT     ? 4 : 0);
		FPCONTROL Ctl = {pThis->m_hWnd, 0, (long)pThis, (long)pThis->GetIDispatch(FALSE)};
		SCALEMODE sm;
		float x, y;

		pThis->ScreenToClient((LPPOINT)&pt);
		fpVBGetScaleMode(&Ctl, &sm, TRUE);
		fpPixelsToScaleMode(&sm, pt.x, pt.y, &x, &y, FALSE);

#ifdef SS_V35
		{
        LPSSPREVIEW lpPV = PV_Lock(pThis->m_hWnd);

        if (lpPV->bScriptEnhanced)
           {
           VARIANT arg;
           VARIANT varParam;
           VARIANT data;
           VARIANT varButton;
           VARIANT varShift;
           VARIANT varx;
           VARIANT vary;

           memset(&arg, 0, sizeof(VARIANT));
           memset(&varParam, 0, sizeof(VARIANT));
           memset(&data, 0, sizeof(VARIANT));
           memset(&varButton, 0, sizeof(VARIANT));
           memset(&varShift, 0, sizeof(VARIANT));
           memset(&varx, 0, sizeof(VARIANT));
           memset(&vary, 0, sizeof(VARIANT));

           varParam.vt = VT_I4; 
           varParam.lVal  = *pdwEffect; 

           data.vt = VT_DISPATCH;
           data.pdispVal = (IDispatch*)pThis->m_pfpDataObject;

           varButton.vt = VT_I2;
           varButton.iVal = Button;

           varShift.vt = VT_I2;
           varShift.iVal = Shift;

           varx.vt = VT_R4;
           varx.fltVal = x;

           vary.vt = VT_R4;
           vary.fltVal = y;

//           pThis->FireScriptOLEDragDrop(&pThis->m_pfpDataObject, &varParam, &Button, &Shift, &x, &y);
//           pThis->FireScriptOLEDragDrop(&data, &varParam, &Button, &Shift, &x, &y);
           pThis->FireScriptOLEDragDrop(&data, &varParam, &varButton, &varShift, &varx, &vary);

           VariantChangeType(&arg, &varParam, 0, VT_UI4); 
           *pdwEffect = arg.ulVal;
           }
        else
           pThis->FireOLEDragDrop(&pThis->m_pfpDataObject, (long*)pdwEffect, &Button, &Shift, &x, &y);
        PV_Unlock(pThis->m_hWnd);
        }
#else
		pThis->FireOLEDragDrop(&pThis->m_pfpDataObject, (long*)pdwEffect, &Button, &Shift, &x, &y);
#endif

		// check to make sure this control is not the drop source,
		// in which case OLEDrag will release the fpDataObject
		if (pThis->m_pfpDataObject->GetIDataObject(FALSE) != &pThis->m_xfpDataObject)
		{	// then release fpDataObject
			pThis->m_pfpDataObject->Release();
			pThis->m_pfpDataObject = NULL;
		}
		return S_OK;
	//}
	//else
	//	return (HRESULT)E_UNEXPECTED;
}

FPMYDISP_CTRL_NAME::XfpDataObject::XfpDataObject()
{
	FORMATETC formatetc = {0, NULL, DVASPECT_CONTENT, -1, TYMED_NULL};
	STGMEDIUM stgmedium = {TYMED_NULL, 0L, NULL};

	for (int i=0; i<8; i++)
	{
		m_Formats[i] = formatetc;
		m_Mediums[i] = stgmedium;
	}
	// each element in the array is associated with a different clipboard format
	m_Formats[0].cfFormat = CF_TEXT;
	m_Formats[1].cfFormat = CF_BITMAP;
	m_Formats[2].cfFormat = CF_METAFILEPICT;
	m_Formats[3].cfFormat = CF_DIB;
	m_Formats[4].cfFormat = CF_PALETTE;
	m_Formats[5].cfFormat = CF_ENHMETAFILE;
	m_Formats[6].cfFormat = CF_HDROP;
	m_Formats[7].cfFormat = 0xC075;	//	Rich Text
}

FPMYDISP_CTRL_NAME::XfpDataObject::~XfpDataObject()
{	// Release the stgmediums
	for (int i=0; i<8; i++)
		if (m_Mediums[i].tymed != TYMED_NULL)
			if (m_Mediums[i].pUnkForRelease)
				m_Mediums[i].pUnkForRelease->Release();
			else
				ReleaseStgMedium(&m_Mediums[i]);
}

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

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::GetData(LPFORMATETC pFormat, LPSTGMEDIUM pMedium)
{
    HRESULT hr;
	short   i;
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)

	if (!pFormat || !pMedium)
		return E_POINTER;
	if (pFormat->lindex != -1)
		return DV_E_LINDEX;
	if (pFormat->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	// check to see if OLESetData event should fire
	// determine which array element is being requested
	i = ClipFormatToArrayIndex(pFormat->cfFormat);
	if (-1 != i && pThis->m_pfpDataObject && pThis->m_pfpDataObject->m_fFireOLESetData[i])
	{
		// check to see if pFormat->cfFormat needs to be changed to the correct value
		// for vbCFRTF 
		if (pFormat->cfFormat == 0xC075)
			pFormat->cfFormat = 0xBF01;
		// prevent recursion
		pThis->m_pfpDataObject->m_fFireOLESetData[i] = FALSE;
		pThis->FireOLESetData(&pThis->m_pfpDataObject, (short*)&pFormat->cfFormat);
		// then change it back again
		if (pFormat->cfFormat == 0xBF01)
			pFormat->cfFormat = 0xC075;
	}
	// default handles metafile, IStream, and IStorage
	if (!(TYMED_MFPICT & pFormat->tymed) && S_OK == (hr = DEFAULT_XDATAOBJECT_PTR(pThis)->GetData(pFormat, pMedium)))
		return hr;
	// default failed, handle here
	// and validate the format
	if (-1 == i)
		return DV_E_FORMATETC;

	switch (pFormat->cfFormat)
	{	// copy the data from the array to the medium passed in
	case CF_TEXT:
	case 0xC075: // Rich Text
	case CF_HDROP:
	case CF_DIB:
		if (m_Mediums[i].tymed != TYMED_HGLOBAL || !(pFormat->tymed & TYMED_HGLOBAL))
			return DV_E_TYMED;
		else
		{	// copy the HGLOBAL
			if (S_OK != (hr = CopyHGlobal(m_Mediums[i].hGlobal, &pMedium->hGlobal)))
				return hr;
			pMedium->tymed = TYMED_HGLOBAL;
		}			
		break;
	case CF_BITMAP:
		if (m_Mediums[i].tymed != TYMED_GDI || !(pFormat->tymed & TYMED_GDI))
			return DV_E_TYMED;
		else
		{	// copy the HBITMAP
			if (S_OK != (hr = CopyHBitmap(m_Mediums[i].hBitmap, &pMedium->hBitmap)))
				return hr;
			pMedium->tymed = TYMED_GDI;
		}
		break;
	case CF_PALETTE:
		if (m_Mediums[i].tymed != TYMED_GDI || !(pFormat->tymed & TYMED_GDI))
			return DV_E_TYMED;
		else
		{	// copy the HPALETTE
			if (S_OK != (hr = CopyHPalette((HPALETTE)m_Mediums[i].hBitmap, (HPALETTE*)&pMedium->hBitmap)))
				return hr;
			pMedium->tymed = TYMED_GDI;
		}
		break;
	case CF_METAFILEPICT:
		if (m_Mediums[i].tymed != TYMED_MFPICT || !(pFormat->tymed & TYMED_MFPICT))
			return DV_E_TYMED;
		else
		{	// copy the HMETAFILEPICT
			if (S_OK != (hr = CopyHMetaFilePict(m_Mediums[i].hMetaFilePict, &pMedium->hMetaFilePict)))
				return hr;
			pMedium->tymed = TYMED_MFPICT;
		}
		break;
	case CF_ENHMETAFILE:
		if (m_Mediums[i].tymed != TYMED_ENHMF || !(pFormat->tymed & TYMED_ENHMF))
			return DV_E_TYMED;
		else
		{	// copy the HENHMETAFILE
			if (S_OK != (hr = CopyHEnhMetaFile(m_Mediums[i].hEnhMetaFile, &pMedium->hEnhMetaFile)))
				return hr;
			pMedium->tymed = TYMED_ENHMF;
		}
		break;
	}
	pMedium->pUnkForRelease = NULL;
	return S_OK;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::GetDataHere(LPFORMATETC pFormat, LPSTGMEDIUM pMedium)
{	// this method cannot be called for GDI objects and is optional for HGLOBALs
//	short i;
//	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	// check to see if OLESetData event should fire
//	i = ClipFormatToArrayIndex(pFormat->cfFormat);
//	if (-1 != i && pThis->m_pfpDataObject && pThis->m_pfpDataObject->m_fFireOLESetData[i])
//		pThis->FireOLESetData(&pThis->m_pfpDataObject, (short*)&pFormat->cfFormat);
//	return DEFAULT_XDATAOBJECT_PTR(pThis)->GetDataHere(pFormat, pMedium);
	return E_NOTIMPL;
} // GetDataHere

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::QueryGetData(LPFORMATETC pFormat)
{
	short i;

	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
	// don't let default handle the metafile format
	if (!(TYMED_MFPICT & pFormat->tymed) && S_OK == DEFAULT_XDATAOBJECT_PTR(pThis)->QueryGetData(pFormat))
		return S_OK;
	// default failed, handle here
	// determine which array element is being requested
	// and validate the format
	if (!pFormat)
		return E_POINTER;
	if (pFormat->lindex != -1)
		return DV_E_LINDEX;
	if (pFormat->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	if (pFormat->tymed == TYMED_NULL)
		return DV_E_TYMED;

	// check to see if pFormat->cfFormat needs to be changed to the correct value
	// for vbCFRTF 
	if (pFormat->cfFormat == 0xBF01)
		pFormat->cfFormat = 0xC075;

	i = ClipFormatToArrayIndex(pFormat->cfFormat);
	if (-1 == i)
		return DV_E_FORMATETC;
	if (m_Mediums[i].tymed == TYMED_NULL)
		return S_FALSE;
	else
		return S_OK;
} // QueryGetData

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::GetCanonicalFormatEtc(LPFORMATETC pFormatetc, LPFORMATETC pFormatetcOut)
{
//	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
//	return DEFAULT_XDATAOBJECT_PTR(pThis)->GetCanonicalFormatEtc(pFormatetc, pFormatetcOut);
	return E_NOTIMPL;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::SetData(LPFORMATETC pFormat, STGMEDIUM* pMedium, int fRelease)
{
	short i;
	HRESULT hr;
	STGMEDIUM medium = {TYMED_NULL, 0L, NULL};
	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)

	// default handles metafile, IStream, and IStorage
	// don't let it handle the metafile format
	if (TYMED_MFPICT != pFormat->tymed && S_OK == (hr = DEFAULT_XDATAOBJECT_PTR(pThis)->SetData(pFormat, pMedium, fRelease)))
		return hr;
	// default failed, handle here
	if (!pFormat || !pMedium)
		return E_POINTER;
	if (pFormat->lindex != -1)
		return DV_E_LINDEX;
	if (pFormat->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;

    if (pFormat->tymed != pMedium->tymed)
		return DV_E_TYMED;

	// special case: when TYMED_NULL is passed for both the format and medium,
	// then clear out all data in the object (for the Clear method of the fpDataObject)
	if (TYMED_NULL == pMedium->tymed)
	{
		for (i=0; i<8; i++)
		{
			if (m_Mediums[i].pUnkForRelease)
				m_Mediums[i].pUnkForRelease->Release();
			else
				ReleaseStgMedium(&m_Mediums[i]);
			m_Mediums[i].tymed = TYMED_NULL;
		}
		return S_OK;
	}

	switch (pFormat->cfFormat)
	{	// copy the data to local medium
	case CF_TEXT:
	case 0xC075: // Rich Text
	case CF_HDROP:
	case CF_DIB:
		if (pMedium->tymed != TYMED_HGLOBAL)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hGlobal)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HGLOBAL
			if (S_OK != (hr = CopyHGlobal(pMedium->hGlobal, &medium.hGlobal)))
				return hr;
			medium.tymed = TYMED_HGLOBAL;
		}
		break;
	case CF_BITMAP:
		if (pMedium->tymed != TYMED_GDI)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hBitmap)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HBITMAP
			if (S_OK != (hr = CopyHBitmap(pMedium->hBitmap, &medium.hBitmap)))
				return hr;
			medium.tymed = TYMED_GDI;
		}
		break;
	case CF_PALETTE:
		if (pMedium->tymed != TYMED_GDI)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hBitmap)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HPALETTE
			if (S_OK != (hr = CopyHPalette((HPALETTE)pMedium->hBitmap, (HPALETTE*)&medium.hBitmap)))
				return hr;
			medium.tymed = TYMED_GDI;
		}
		break;
	case CF_METAFILEPICT:
		if (pMedium->tymed != TYMED_MFPICT)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hMetaFilePict)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HMETAFILEPICT
			if (S_OK != (hr = CopyHMetaFilePict(pMedium->hMetaFilePict, &medium.hMetaFilePict)))
				return hr;
			medium.tymed = TYMED_MFPICT;
		}
		break;
	case CF_ENHMETAFILE:
		if (pMedium->tymed != TYMED_ENHMF)
			return DV_E_TYMED;
		else if (fRelease || !pMedium->hEnhMetaFile)	// data object owns the stgmedium after this call
			medium = *pMedium;
		else	// stgmedium valid for this call only
		{	// copy the HENHMETAFILE
			if (S_OK != (hr = CopyHEnhMetaFile(pMedium->hEnhMetaFile, &medium.hEnhMetaFile)))
				return hr;
			medium.tymed = TYMED_ENHMF;
		}
		break;
	default:
		return DV_E_FORMATETC;
	}
	// copy the format and medium to array
	i = ClipFormatToArrayIndex(pFormat->cfFormat);
	m_Formats[i] = *pFormat;
	if (TYMED_NULL != m_Mediums[i].tymed)
	{	// data was already set for this entry in the array,
		// must release it
		if (m_Mediums[i].pUnkForRelease)
			m_Mediums[i].pUnkForRelease->Release();
		else
			ReleaseStgMedium(&m_Mediums[i]);
	}
	m_Mediums[i] = medium;

	return S_OK;
} // SetData

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpDataObject::EnumFormatEtc(ULONG dwDirection, LPENUMFORMATETC FAR* ppenumFormatetc)
{
// fix for SEL5002 -scl
//	METHOD_PROLOGUE(FPMYDISP_CTRL_NAME, fpDataObject)
//	return DEFAULT_XDATAOBJECT_PTR(pThis)->EnumFormatEtc(dwDirection, ppenumFormatetc);
	if (!ppenumFormatetc)
		return E_POINTER;
	
	CfpEnumFormatEtc* pEnum = new CfpEnumFormatEtc(this, dwDirection);
	*ppenumFormatetc = (IEnumFORMATETC*)pEnum;
	return S_OK;
// fix for SEL5002 -scl
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

short FPMYDISP_CTRL_NAME::XfpDataObject::ClipFormatToArrayIndex(CLIPFORMAT cf)
{
	switch((WORD)cf)
	{
	case CF_TEXT:			return 0;
	case CF_BITMAP:			return 1;
	case CF_METAFILEPICT:	return 2;
	case CF_DIB:			return 3;
	case CF_PALETTE:		return 4;
	case CF_ENHMETAFILE:	return 5;
	case CF_HDROP:			return 6;
	case 0xC075:			
	case 0xBF01:			return 7;	//	Rich Text
	default:				return -1;
	}
}

void FPMYDISP_CTRL_NAME::OLEDrag(void)
{
	long AllowedEffects = 0L;
	//LPDATAOBJECT pDataObject = DEFAULT_XDATAOBJECT_PTR(this);
	LPDATAOBJECT pDataObject = DEFAULT_XFPDATAOBJECT_PTR(this);

	// create fpDataObject
	ASSERT(!m_pfpDataObject);	// should be NULL
	m_pfpDataObject = new CfpDataObject(pDataObject);
	if (!m_pfpDataObject)
		return;

	// clear the data object
	m_pfpDataObject->Clear();

#ifdef SS_V35
	    {
        LPSSPREVIEW lpPV = PV_Lock(m_hWnd);

        if (lpPV->bScriptEnhanced)
           {
           VARIANT arg;
           VARIANT varParam;
           VARIANT data;

           memset(&arg, 0, sizeof(VARIANT));
           memset(&varParam, 0, sizeof(VARIANT));
           memset(&data, 0, sizeof(VARIANT));

           varParam.vt = VT_I4; 
           varParam.lVal  = AllowedEffects; 

           data.vt = VT_DISPATCH; 
           data.pdispVal  = (IDispatch*)m_pfpDataObject; 

//	       FireScriptOLEStartDrag(&m_pfpDataObject, &varParam);
	       FireScriptOLEStartDrag(&data, &varParam);

           VariantChangeType(&arg, &varParam, 0, VT_I4); 
           AllowedEffects = arg.lVal;
           }
        else
	       FireOLEStartDrag(&m_pfpDataObject, &AllowedEffects);
        PV_Unlock(m_hWnd);
        }
#else
	FireOLEStartDrag(&m_pfpDataObject, &AllowedEffects);
#endif

	// user must (1) change AllowedEffects to non-zero value, and
	// (2) specify a data format for the data object, or
	// the drag does not happen
	if (AllowedEffects && m_pfpDataObject->m_fFormatSet)
	{
		ULONG ResultEffect = 0L;
		m_xfpDropSource.m_nDragButton = 0;
		// check to see which button is down for the drag
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			m_xfpDropSource.m_nDragButton |= VK_LBUTTON;
		if(GetAsyncKeyState(VK_RBUTTON) & 0x8000)
			m_xfpDropSource.m_nDragButton |= VK_RBUTTON;
		if(GetAsyncKeyState(VK_MBUTTON) & 0x8000)
			m_xfpDropSource.m_nDragButton |= VK_MBUTTON;
		// Constructor of CfpDataObject has already called AddRef on pDataObject
		::DoDragDrop(pDataObject, &m_xfpDropSource, AllowedEffects, &ResultEffect);

#ifdef SS_V35
	    {
        LPSSPREVIEW lpPV = PV_Lock(m_hWnd);

        if (lpPV->bScriptEnhanced)
           {
           VARIANT arg;
           VARIANT varParam;

           memset(&arg, 0, sizeof(VARIANT));
           memset(&varParam, 0, sizeof(VARIANT));

           varParam.vt = VT_UI4; 
           varParam.ulVal  = ResultEffect; 

		   FireScriptOLECompleteDrag(&varParam);

           VariantChangeType(&arg, &varParam, 0, VT_UI4); 
           ResultEffect = arg.ulVal;
           }
        else
		   FireOLECompleteDrag((long*)&ResultEffect);
        PV_Unlock(m_hWnd);
        }
#else
		FireOLECompleteDrag((long*)&ResultEffect);
#endif
	}
	m_pfpDataObject->Clear();
	m_pfpDataObject->Release();
	m_pfpDataObject = NULL;
	ReleaseCapture();
}

short FPMYDISP_CTRL_NAME::GetOLEDropMode()
{
	return m_OLEDropMode;
}

void FPMYDISP_CTRL_NAME::SetOLEDropMode(short newValue)
{
	if (newValue >=0 && newValue <=1)
	{
		m_OLEDropMode = newValue;
		if (AmbientUserMode())
		{
			if (m_OLEDropMode)
				::RegisterDragDrop(m_hWnd, &m_xfpDropTarget);
			else
				::RevokeDragDrop(m_hWnd);
		}
	}
}

#endif // MFC implementation