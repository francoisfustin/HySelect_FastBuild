/***********************************************************************
* FPOCX.CPP
*
* Copyright (C) 1991-1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/
#include "fptstr.h"

#ifndef FPMYDISP_CTRL_NAME
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" ***" ) 
#endif

// DECLARE STATIC CLASS MEMBER
CfpIDispatchTbl FPMYDISP_CTRL_NAME::m_mydpIDispTbl;

// MACROS:
#ifndef METHOD_PROLOGUE_EX_
#define METHOD_PROLOGUE_EX_(theClass, localClass)  \
METHOD_PROLOGUE(theClass, localClass)
#endif

#ifdef WIN32
// Don't try understanding this macro, just accept it! - SCP
#ifndef NO_MFC
#define DEFAULT_XDISPATCH_PTR(pCtl)   \
  ((IDispatch FAR *)&(pCtl->m_xDispatch.m_vtbl))
#else
#define DEFAULT_XDISPATCH_PTR(pCtl)   (pCtl)
#endif

// DELETE_EXCEPTION() copied from \msvc40\mfc\src\stdafx.h
#ifdef _DEBUG
#define DELETE_EXCEPTION(e)   do { e->Delete(); } while (0)
#else
#define DELETE_EXCEPTION(e)
#endif

#else //WIN16
#define DEFAULT_XDISPATCH_PTR(pCtl)   \
  ((IDispatch FAR *)&(pCtl->m_xDispatch))

#define DELETE_EXCEPTION(e)
#endif

// IMPLEMENTATION:

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XMyDispatch::AddRef()
{
  METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, MyDispatch)
  return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XMyDispatch::Release()
{
  METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, MyDispatch)
  return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XMyDispatch::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, MyDispatch)
#ifndef NO_MFC
  return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
#else
  return (HRESULT)pThis->ExternalQueryInterface(iid, ppvObj);
#endif
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XMyDispatch::GetTypeInfoCount(
	unsigned int* p1)
{
  METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, MyDispatch)

  if (DEFAULT_XDISPATCH_PTR(pThis))
    return DEFAULT_XDISPATCH_PTR(pThis)->GetTypeInfoCount(p1);
  else
    return (HRESULT)E_NOTIMPL;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XMyDispatch::GetTypeInfo(
	unsigned int p1, LCID p2, ITypeInfo** p3)
{
  METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, MyDispatch)

  if (DEFAULT_XDISPATCH_PTR(pThis))
    return DEFAULT_XDISPATCH_PTR(pThis)->GetTypeInfo(p1, p2, p3);
  else
    return (HRESULT)E_NOTIMPL;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XMyDispatch::GetIDsOfNames(
	REFIID p1, LPOLESTR* p2, unsigned int p3, LCID p4, DISPID* p5)
{
  METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, MyDispatch)

  if (DEFAULT_XDISPATCH_PTR(pThis))
    return DEFAULT_XDISPATCH_PTR(pThis)->GetIDsOfNames(p1, p2, p3, p4, p5);
  else
    return (HRESULT)E_NOTIMPL;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XMyDispatch::Invoke(
	DISPID dispid, REFIID riid, LCID lcid,
	WORD wFlags, DISPPARAMS* pDispParams, LPVARIANT pvarResult,
	LPEXCEPINFO pexcepinfo, UINT* puArgErr)
{
    METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, MyDispatch)

	// check arguments
	if (riid != IID_NULL)
		return (HRESULT)DISP_E_UNKNOWNINTERFACE;
#ifdef WIN32    
	ASSERT_NULL_OR_POINTER(pvarResult, VARIANT);
	ASSERT_NULL_OR_POINTER(pexcepinfo, EXCEPINFO);
	ASSERT_NULL_OR_POINTER(puArgErr, UINT);

	// make sure pvarResult is initialized
	if (pvarResult != NULL)
#ifndef NO_MFC
		AfxVariantInit(pvarResult);
#else
		VariantInit(pvarResult);
#endif

#ifndef NO_MFC
	// allow subclass to disable Invoke
	if (!pThis->IsInvokeAllowed(dispid))
		return (HRESULT)E_UNEXPECTED;
#endif  // NO_MFC

#else //WIN16
	ASSERT_VALID(pThis);
#endif

	// copy param block for safety
	DISPPARAMS params = *pDispParams;
	pDispParams = &params;

	// most of the time, named arguments are not supported
	if (pDispParams->cNamedArgs != 0)
	{
		// only special PROPERTYPUT named argument is allowed
		if (pDispParams->cNamedArgs != 1 ||
			pDispParams->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT)
		{
			return (HRESULT)DISP_E_NONAMEDARGS;
		}
	}

	
	// get entry for the member ID
	const AFX_DISPMAP_ENTRY* pEntry = pThis->GetDispEntry(dispid);
	if (pEntry == NULL)
		return (HRESULT)DISP_E_MEMBERNOTFOUND;

    // treat member calls on properties just like property get/set
	if ((wFlags == DISPATCH_METHOD) &&
		((pEntry->pfn == NULL && pEntry->pfnSet == NULL) ||
		 (pEntry->pfn == NULL && pEntry->pfnSet != NULL) ||
		 (pEntry->pfn != NULL && pEntry->pfnSet != NULL)))
	{
		// the entry describes a property but a method call is being
		//  attempted -- change it to a property get/set based on the
		//  number of parameters being passed.
		wFlags &= ~DISPATCH_METHOD;
		UINT nExpectedArgs = 0;

#ifdef WIN32        
        if (pEntry->lpszParams != NULL)
            nExpectedArgs = (UINT)lstrlenA(pEntry->lpszParams);
#else
        if (pEntry->pbParams != NULL)
            nExpectedArgs = (UINT)lstrlen((LPSTR)pEntry->pbParams);
#endif
		if (pDispParams->cArgs <= nExpectedArgs)
		{
			// no extra param -- so treat as property get
			wFlags |= DISPATCH_PROPERTYGET;
		}
		else
		{
			// extra params -- treat as property set
			wFlags |= DISPATCH_PROPERTYPUTREF;
			pDispParams->cNamedArgs = 1;
		}
	}

	// property puts should not require a return value
	if (wFlags & (DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYPUT))
	{
		pvarResult = NULL;
		// catch attempt to do property set on method
		if (pEntry->pfn != NULL && pEntry->pfnSet == NULL)
			return (HRESULT)DISP_E_TYPEMISMATCH;
	}

    ///////////////////////////////////////////////////////////////////
    //
    // if this is NOT one of our Get/Set, call default Invoke()
    //
    ///////////////////////////////////////////////////////////////////
    //if ((pEntry->pfn != ((AFX_PMSG)(void (FPMYDISP_CTRL_NAME::*)(void))pThis->mydpGetProp)) ||
    //    (pEntry->pfnSet != ((AFX_PMSG)(void (FPMYDISP_CTRL_NAME::*)(void))pThis->mydpSetProp)))
    if ((pEntry->pfn != ((AFX_PMSG)(void (FPMYDISP_CTRL_NAME::*)(void))&FPMYDISP_CTRL_NAME::mydpGetProp)) ||
        (pEntry->pfnSet != ((AFX_PMSG)(void (FPMYDISP_CTRL_NAME::*)(void))&FPMYDISP_CTRL_NAME::mydpSetProp)))
    {
      if (DEFAULT_XDISPATCH_PTR(pThis))
        return DEFAULT_XDISPATCH_PTR(pThis)->Invoke(
                   dispid, riid, lcid, wFlags, 
                   pDispParams, pvarResult, pexcepinfo, puArgErr);
      else
        return (HRESULT)E_NOTIMPL;
    }

	LONG lSetIndex = (LONG)-1;
    VARIANT FAR *pVar = NULL;
    VARIANT tmpVar;
    DWORD dwResult;

#ifndef NO_MFC
  #ifdef WIN32
    AfxVariantInit(&tmpVar);
  #else
    VariantInit(&tmpVar);
  #endif
#else
	VariantInit(&tmpVar);
#endif

    // Retrieve the parameters to use in Get/Set function
    if (wFlags & (DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYPUT))
    {
      if ((pDispParams->cArgs > 2) || (pDispParams->cArgs < 1))
        return (HRESULT)DISP_E_BADPARAMCOUNT;
      // if 2 parameters for SET, one must be an index
      if (pDispParams->cArgs == 2)
      {
        if (pDispParams->rgvarg[1].vt == VT_I2)
          lSetIndex = MAKELONG(pDispParams->rgvarg[1].iVal, 0);
      }
      pVar = pDispParams->rgvarg;
      // If "pVar->vt" is valid and does not match "pEntry->vt",
      // coerce the data.  NOTE: pVar->vt is not always initialized.
      if ((pVar->vt != VT_EMPTY) && (pVar->vt != pEntry->vt))
      {
//		HRESULT h = (HRESULT)VariantChangeType(pVar, pVar, 0, pEntry->vt);
        HRESULT h = (HRESULT)VariantChangeType(&tmpVar, pVar, 0, pEntry->vt);
		if (h != (HRESULT)S_OK)
		   return (HRESULT)DISP_E_TYPEMISMATCH;	  //KM - GRB5010
        // Point to tmpVar which contains the conversion
        pVar = &tmpVar;
      }
      pVar->vt = pEntry->vt;

    }
    else
    {
      if (pDispParams->cArgs > 1)
        return (HRESULT)DISP_E_BADPARAMCOUNT;
      // if 1 parameter for GET, it must be an index
      if (pDispParams->cArgs == 1)
      {
        if (pDispParams->rgvarg[0].vt == VT_I2)
          lSetIndex = MAKELONG(pDispParams->rgvarg[0].iVal, 0);
      }
    }

	UINT uArgErr = (UINT)-1;    // no error yet
	SCODE sc = S_OK;                                    
#ifdef WIN32	
#ifndef NO_MFC
	BOOL bResultExpected = pThis->m_bResultExpected;
    pThis->m_bResultExpected = (pvarResult != NULL);
#endif	// NO_MFC
#endif
#ifndef NO_MFC
	AFX_MANAGE_STATE(pThis->m_pModuleState);
#else
	FP_MANAGE_STATE(pThis->m_pModuleState);
#endif

#ifndef NO_MFC
	TRY
	{
	    if (wFlags & (DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYPUT))
        {
          pThis->mydpSetProp(pEntry->lDispID, lSetIndex, pVar);          
        }
        else
        {
          dwResult = (DWORD)pThis->mydpGetProp(pEntry->lDispID, lSetIndex);
        }
	}
	CATCH(COleException, e)
	{
		sc = e->m_sc;
		DELETE_EXCEPTION(e);
	}
	AND_CATCH_ALL(e)
	{
		AFX_MANAGE_STATE(pThis->m_pModuleState);
		if (pexcepinfo != NULL)
		{
			// fill exception with translation of MFC exception
			COleDispatchException::Process(pexcepinfo, e);
		}
		DELETE_EXCEPTION(e);
		sc = DISP_E_EXCEPTION;
	}
	END_CATCH_ALL
#else
	try
	{
	    if (wFlags & (DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYPUT))
        {
          pThis->mydpSetProp(dispid, lSetIndex, pVar);          
        }
        else
        {
          dwResult = (DWORD)pThis->mydpGetProp(dispid, lSetIndex);
        }
	}
	catch(...)
	{
		sc = DISP_E_EXCEPTION;
	}
#endif	   // NO_MFC

    // if SET, retrieve the parameter.
	if ((wFlags & DISPATCH_PROPERTYGET) && (pvarResult != NULL) &&
        (sc == S_OK))
    {
        WORD vt = pEntry->vt;

		// build return value VARIANT from result union
		switch (pEntry->vt)
		{
		case VT_I2:
			pvarResult->iVal = (short)dwResult;
			break;
		case VT_I4:
			pvarResult->lVal = (long)dwResult;
			break;
		case VT_R4:
			pvarResult->fltVal = *(float FAR *)&dwResult;
			break;
		case VT_R8:
			//pvarResult->dblVal
		case VT_CY:
			//pvarResult->cyVal
		case VT_DATE:
			//pvarResult->date
#ifndef NO_MFC
            ASSERT(0);  // NOT SUPPORTED!!!
#else
            ASSERT(0, 0);  // NOT SUPPORTED!!!
#endif
            vt = VT_EMPTY;
			break;
		case VT_BSTR:
			pvarResult->bstrVal = (BSTR)dwResult;
			break;
		case VT_ERROR:
			pvarResult->scode = (SCODE)dwResult;
			break;
		case VT_BOOL:
			V_BOOL(pvarResult) = (VARIANT_BOOL)((BOOL)dwResult != 0 ? -1 : 0);
			break;
		case VT_VARIANT:
			//*pvarResult = result.vaVal;
#ifndef NO_MFC
            ASSERT(0);  // NOT SUPPORTED!!!
#else
            ASSERT(0, 0);  // NOT SUPPORTED!!!
#endif
            vt = VT_EMPTY;
			break;
		case VT_DISPATCH:
		case VT_UNKNOWN:
			pvarResult->punkVal = (LPUNKNOWN)dwResult; // already AddRef
			break;
		}
        pvarResult->vt = vt;
	}

	// fill error argument if one is available
	if (sc != S_OK && puArgErr != NULL && uArgErr != -1)
		*puArgErr = uArgErr;

	// restore original m_bResultExpected flag
#ifdef WIN32	
#ifndef NO_MFC
	pThis->m_bResultExpected = bResultExpected;
#endif  // NO_MFC
#endif

	// Free any BSTR in the tmp variant structure.
	if ((tmpVar.vt == VT_BSTR) && (tmpVar.bstrVal))
	   SysFreeString(tmpVar.bstrVal);

	return (HRESULT)sc;
}


void FPMYDISP_CTRL_NAME::mydpSetProp(DISPID dispID, long lSetIndex, VARIANT FAR *lpVar)
{
  LPTSTR tstr = NULL;
  WORD wPropID = mydpGetPropIDFromDispID(dispID);
  long lValue = (long)0;

  // if wPropID is invalid, just return.
  if (wPropID == (WORD)0xffff)
    return;

  switch (lpVar->vt)
  {
	case VT_I2:
		lValue = (long)lpVar->iVal;
		break;
	case VT_I4:
		lValue = (long)lpVar->lVal;
		break;
	case VT_R4:
		*(float *)&lValue = lpVar->fltVal;
		break;
	case VT_BSTR:
		lValue = (long)lpVar->bstrVal;
        if (lpVar->bstrVal && (sizeof(TCHAR) != sizeof(OLECHAR)))
        {
          tstr = fpSysAllocTStringFromBSTR(lpVar->bstrVal);
          lValue = (long)tstr;
        }
		else if (!lpVar->bstrVal) // only if there is no string! -scl
		{
		  tstr = fpSysAllocTString((LPCTSTR)"");
		  lValue = (long)tstr;
		}
		break;
	case VT_ERROR:
		lValue = (long)lpVar->scode;
		break;
	case VT_BOOL:
		lValue = (long)V_BOOL(lpVar);
		break;
	case VT_DISPATCH:
	case VT_UNKNOWN:
		lValue = (long)lpVar->punkVal; // already AddRef
		break;
	case VT_R8:
	case VT_CY:
	case VT_DATE:
	case VT_VARIANT:
    default:
#ifndef NO_MFC
            ASSERT(0);  // NOT SUPPORTED!!!
#else
            ASSERT(0, 0);  // NOT SUPPORTED!!!
#endif
		break;
  }
  fpSetProp(wPropID, lValue, lSetIndex);

  // if tstr was allocated, free it.
  if (tstr)
    fpSysFreeTString(tstr);
}

DWORD FPMYDISP_CTRL_NAME::mydpGetProp(DISPID dispID, long lGetIndex)
{
  WORD wPropID = mydpGetPropIDFromDispID(dispID);

  // if wPropID is invalid, just return.
  if (wPropID == (WORD)0xffff)
    return (DWORD)0;

  return (DWORD)(fpGetProp(wPropID, lGetIndex));
}

WORD FPMYDISP_CTRL_NAME::mydpGetPropIDFromDispID(DISPID dispID)
{
  WORD wRet = (WORD)-1;
  int index = (int)dispID - 1;

  // if index (dispid) is valid (in the table)...
  if ((index >= 0) && (index < m_mydpIDispTbl.m_PropIdTblCt))
  {
    wRet = m_mydpIDispTbl.m_PropIdTbl[index];
#ifndef NO_MFC
    ASSERT(wRet != (WORD)0xffff);    // invalid PropID in table
#else
    ASSERT(wRet != (WORD)0xffff, 0);    // invalid PropID in table
#endif
  }
  else // invalid index
  {
#ifndef NO_MFC
    ASSERT(0);       // Error, invalid index (dispid)
#else
    ASSERT(0, 0);       // Error, invalid index (dispid)
#endif
  }

  return wRet;
}


long FPMYDISP_CTRL_NAME::mydpInitTable(FP_PROPLIST FAR *propList, 
  int iListCt)
{
  long lresult = S_OK;

  if (!m_mydpIDispTbl.m_dispatchTbl)  
  {
	short sizeETable = sizeof(_dispatchEntries);

    // Create Dispatch table
    lresult = m_mydpIDispTbl.BuildDispatchMap(
                propList, 
                iListCt, 
#if VC8
                (AFX_PMSG)(void (FPMYDISP_CTRL_NAME::*)(void))&FPMYDISP_CTRL_NAME::mydpGetProp, 
                (AFX_PMSG)(void (FPMYDISP_CTRL_NAME::*)(void))&FPMYDISP_CTRL_NAME::mydpSetProp, 
#else
                (AFX_PMSG)(void (FPMYDISP_CTRL_NAME::*)(void))mydpGetProp, 
                (AFX_PMSG)(void (FPMYDISP_CTRL_NAME::*)(void))mydpSetProp, 
#endif
                &_dispatchEntries[0], 
                sizeETable / sizeof(AFX_DISPMAP_ENTRY));
    
    // Was table created properly?
#ifndef NO_MFC
    ASSERT(lresult == S_OK);
#else
    ASSERT(lresult == S_OK, 0);
#endif
    
    // modify FPMYDISP_CTRL_NAME::GetDispatchMap() to use m_fpIDispTbl.m_dispatchTbl
    if ((lresult == S_OK) && m_mydpIDispTbl.m_dispatchTbl)
    {
#if WIN32
      MyDispatchMap.lpEntries = 
        (const AFX_DISPMAP_ENTRY*)&m_mydpIDispTbl.m_dispatchTbl[0];
#else  //WIN16 - following not declared with "const"    
      MyDispatchMap.lpEntries = 
        (AFX_DISPMAP_ENTRY FAR*)&m_mydpIDispTbl.m_dispatchTbl[0];
#endif      
    }
  }
  
  return lresult;
}         

#ifdef NO_MFC
UINT FPMYDISP_CTRL_NAME::GetEntryCount(const AFX_DISPMAP* pDispMap)
{
	ASSERT(pDispMap->lpEntryCount != NULL, 0);

	// compute entry count cache if not available
	if (*pDispMap->lpEntryCount == -1)
	{
		// count them
		const AFX_DISPMAP_ENTRY* pEntry = pDispMap->lpEntries;
		while (pEntry->nPropOffset != -1)
			++pEntry;

		// store it
		*pDispMap->lpEntryCount = pEntry - pDispMap->lpEntries;
	}

	ASSERT(*pDispMap->lpEntryCount != -1, 0);
	return *pDispMap->lpEntryCount;
}

MEMBERID FPMYDISP_CTRL_NAME::MemberIDFromName(
	const AFX_DISPMAP* pDispMap, LPCTSTR lpszName)
{
	// search all maps and their base maps
	UINT nInherit = 0;
	while (pDispMap != NULL)
	{
		// search all entries in this map
		const AFX_DISPMAP_ENTRY* pEntry = pDispMap->lpEntries;
		UINT nEntryCount = GetEntryCount(pDispMap);
		for (UINT nIndex = 0; nIndex < nEntryCount; nIndex++)
		{
			if (pEntry->vt != VT_MFCVALUE &&
				lstrcmpi(pEntry->lpszName, lpszName) == 0)
			{
				if (pEntry->lDispID == DISPID_UNKNOWN)
				{
					// the MEMBERID is combination of nIndex & nInherit
					ASSERT(MAKELONG(nIndex+1, nInherit) != DISPID_UNKNOWN, 0);
					return MAKELONG(nIndex+1, nInherit);
				}
				// the MEMBERID is specified as the lDispID
				return pEntry->lDispID;
			}
			++pEntry;
		}
		pDispMap = (*pDispMap->pfnGetBaseMap)();
		++nInherit;
	}
	return DISPID_UNKNOWN;  // name not found
}

const AFX_DISPMAP_ENTRY* FPMYDISP_CTRL_NAME::GetDispEntry(MEMBERID memid)
{
	const AFX_DISPMAP* pDerivMap = GetDispatchMap();
	const AFX_DISPMAP* pDispMap;
	const AFX_DISPMAP_ENTRY* pEntry;

	if (memid == DISPID_VALUE)
	{
		// DISPID_VALUE is a special alias (look for special alias entry)
		pDispMap = pDerivMap;
		while (pDispMap != NULL)
		{
			// search for special entry with vt == VT_MFCVALUE
			pEntry = pDispMap->lpEntries;
			while (pEntry->nPropOffset != -1)
			{
				if (pEntry->vt == VT_MFCVALUE)
				{
					memid = pEntry->lDispID;
					if (memid == DISPID_UNKNOWN)
					{
						// attempt to map alias name to member ID
						memid = MemberIDFromName(pDerivMap, pEntry->lpszName);
						if (memid == DISPID_UNKNOWN)
							return NULL;
					}
					// break out and map the member ID to an entry
					goto LookupDispID;
				}
				++pEntry;
			}
			pDispMap = (*pDispMap->pfnGetBaseMap)();
		}
	}

LookupDispID:
	if ((long)memid > 0)
	{
		// find AFX_DISPMAP corresponding to HIWORD(memid)
		UINT nTest = 0;
		pDispMap = pDerivMap;
		while (pDispMap != NULL && nTest < (UINT)HIWORD(memid))
		{
			pDispMap = (*pDispMap->pfnGetBaseMap)();
			++nTest;
		}
		if (pDispMap != NULL)
		{
			UINT nEntryCount = GetEntryCount(pDispMap);
			if ((UINT)LOWORD(memid) <= nEntryCount)
			{
				pEntry = pDispMap->lpEntries + LOWORD(memid)-1;

				// must have automatic DISPID or same ID
				ASSERT(pEntry->lDispID == DISPID_UNKNOWN ||
					pEntry->lDispID == memid, 0);

				return pEntry;
			}
		}
	}

	// second pass, look for DISP_XXX_ID entries
	pDispMap = pDerivMap;
	while (pDispMap != NULL)
	{
		// find AFX_DISPMAP_ENTRY where (pEntry->lDispID == memid)
		pEntry = pDispMap->lpEntries;
		while (pEntry->nPropOffset != -1)
		{
			if (pEntry->lDispID == memid)
				return pEntry;

			++pEntry;
		}
		// check base class
		pDispMap = (*pDispMap->pfnGetBaseMap)();
	}

	return NULL;    // no matching entry
}
#endif   // NO_MFC
