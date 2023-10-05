/***************************************************************************/
/* FPCONV8.CPP   FarPoint Support for VBX functions in non-VB environments */
/***************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#include <string.h>

#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
//#include "ctlcore.h"
#include "stdafx.h"
#include "fpatl.h"
// this table is used for copying data around, and persisting properties.
// basically, it contains the size of a given data type
//
const BYTE g_rgcbDataTypeSize[] = {
    0,                      // VT_EMPTY= 0,
    0,                      // VT_NULL= 1,
    sizeof(short),          // VT_I2= 2,
    sizeof(long),           // VT_I4 = 3,
    sizeof(float),          // VT_R4  = 4,
    sizeof(double),         // VT_R8= 5,
    sizeof(CURRENCY),       // VT_CY= 6,
    sizeof(DATE),           // VT_DATE = 7,
    sizeof(BSTR),           // VT_BSTR = 8,
    sizeof(IDispatch *),    // VT_DISPATCH    = 9,
    sizeof(SCODE),          // VT_ERROR    = 10,
    sizeof(VARIANT_BOOL),   // VT_BOOL    = 11,
    sizeof(VARIANT),        // VT_VARIANT= 12,
    sizeof(IUnknown *),     // VT_UNKNOWN= 13,
};
#endif

#ifdef NO_MFC
#if (defined( __T) && !defined(_INC_TCHAR))
#undef __T
#endif
#endif

#include "tchar.h"

#include "vbinterf.h"
// following lines taken from VBINTERF.H
#include <initguid.h>

#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif

// the last two parameters are used for OCX only
ERR FPLIB fpVBSetControlProperty(LPFPCONTROL lpObject, USHORT usPropID, LONG lData, LPTSTR lpszPropName, USHORT usDataType)
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
  LPDISPATCH lpDispInternal = 0;
  LPDISPATCH lpDispTemp;
  DISPID dispID;
  LCID lcID = 0; // LOCALE_SYSTEM_DEFAULT;                        
  COleDispatchDriver dispDrv;
  BSTR lpOleStr = (BSTR)fpStr2OLE(lpszPropName);
  IOleClientSite *pOleClientSite;
  IOleControlSite *pOleControlSite;
  IOleObject *pOleObject;
  HRESULT hrResult;

  if (!lpDisp)
  {
     if (!pObject && (lpObject->hWnd) && IsWindow((HWND)lpObject->hWnd))
        pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

     if (pObject)
     {
       if (pObject->IsKindOf(RUNTIME_CLASS(COleControl)))
	   {
         lpDisp = ((COleControl *)pObject)->GetExtendedControl();                
         if (lpDisp)
            lpDispInternal = lpDisp;
       }           
       else if (pObject->IsKindOf(RUNTIME_CLASS(CCmdTarget)))
       {
	     ((CCmdTarget *)pObject)->EnableAutomation(); 
	     lpDisp = ((CCmdTarget *)pObject)->GetIDispatch(TRUE); 
	     lpDispInternal = lpDisp;
      
         if (NOERROR == lpDisp->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject))
	     {
	       pOleObject->GetClientSite(&pOleClientSite);
           if (NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
	       {
	         pOleControlSite->GetExtendedControl(&lpDispTemp);
		     if (lpDispInternal)
		       fpDispatchRelease((LONG)lpDispInternal);
	         lpDisp = lpDispTemp;
		     lpDispInternal = lpDisp;
    	     pOleControlSite->Release();
	       }
		   pOleClientSite->Release();
           pOleObject->Release();
	     }
	   }
     }
  }

  if (!lpDisp)
    {
    SysFreeString(lpOleStr);
    return (ERR)-1; 
    }

  if (NOERROR == lpDisp->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject))
  {
	if( NOERROR == pOleObject->GetClientSite(&pOleClientSite) && pOleClientSite )
	{
      if( NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite) )
      {
	     pOleControlSite->GetExtendedControl(&lpDispTemp);
	     if (lpDispInternal)
           fpDispatchRelease((LONG)lpDispInternal);

	     lpDisp = lpDispTemp;
	     lpDispInternal = lpDisp;
	     pOleControlSite->Release();
      }
	   pOleClientSite->Release();
	}
	pOleObject->Release();
  }

  hrResult = lpDisp->GetIDsOfNames(IID_NULL, (OLECHAR FAR * FAR *)&lpOleStr, 1, 
                                   lcID, &dispID);
 
  SysFreeString(lpOleStr);

  if (hrResult != S_OK || dispID == DISPID_UNKNOWN) 
     {
     if (lpDispInternal)
       fpDispatchRelease((LONG)lpDispInternal);
 
     return (ERR)-1;
     }

  dispDrv.AttachDispatch(lpDisp);
  // for strings, lData must be a BSTR or LPSTR with
  //    memory previously allocated
  if (usDataType == VT_STRING)
  {
    BSTR lpDataStr = (BSTR)fpStr2OLE((LPTSTR)lData);
    usDataType = VT_BSTR;
    TRY
       {
       dispDrv.SetProperty((DISPID)dispID, 
                           usDataType,
                           lpDataStr);  

	   }    
    CATCH_ALL(e)
	   {
       SysFreeString(lpDataStr);
       
	   dispDrv.DetachDispatch();

       if (lpDispInternal)
          fpDispatchRelease((LONG)lpDispInternal);
        
       return ((ERR)-1);         
	   }
	END_CATCH_ALL

    SysFreeString(lpDataStr);
  }
  else 
  {
    if (usDataType == VT_CURRENCY)
      usDataType = VT_CY;        
    TRY
       {
       dispDrv.SetProperty((DISPID)dispID, 
                        usDataType,
                        lData);  
	   }   
    CATCH_ALL(e)
	   {
       dispDrv.DetachDispatch();

       if (lpDispInternal)
          fpDispatchRelease((LONG)lpDispInternal);
        
       return ((ERR)-1);         
	   }
	END_CATCH_ALL
  }

  dispDrv.DetachDispatch();

  if (lpDispInternal)
    fpDispatchRelease((LONG)lpDispInternal);
        
  return 0;         
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
  LPDISPATCH lpDispInternal = 0;
  LPDISPATCH lpDispTemp = NULL;
  DISPID dispID;
  LCID lcID = 0; // LOCALE_SYSTEM_DEFAULT;                        
  BSTR lpOleStr = (BSTR)fpStr2OLE(lpszPropName);
  IOleClientSite *pOleClientSite;
  IOleControlSite *pOleControlSite;
  IOleObject *pOleObject;
  HRESULT hrResult;

  DISPPARAMS dispparams;
  VARIANT v;

  if (!lpDisp)
  {
     if (!pObject && (lpObject->hWnd) && IsWindow((HWND)lpObject->hWnd))
     {
//		pObject = COleControl::ControlFromHwnd((HWND)lpObject->hWnd);
        _ASSERT(FALSE); // not supported for ATL
        return 0;
     }

     if (pObject)
	 {
	    pOleClientSite = pObject->fpAtlGetClientSite();
	    if (pOleClientSite &&
            NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
	    { 
	       pOleControlSite->GetExtendedControl(&lpDisp);
           lpDispInternal = lpDisp;
	       pOleControlSite->Release();
	    }
	    pOleClientSite->Release();
	 }
  }
  
  if (!lpDisp)
  {
    SysFreeString(lpOleStr);
    return (ERR)-1; 
  }


  if (NOERROR == lpDisp->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject))
  {
	pOleObject->GetClientSite(&pOleClientSite);
	if (NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
	{
	  pOleControlSite->GetExtendedControl(&lpDispTemp);
	  if (lpDispInternal)
        fpDispatchRelease((LONG)lpDispInternal);

	  lpDisp = lpDispTemp;
	  lpDispInternal = lpDisp;
	  pOleControlSite->Release();
	}
	pOleClientSite->Release();
	pOleObject->Release();
  }

  hrResult = lpDisp->GetIDsOfNames(IID_NULL, (OLECHAR FAR * FAR *)&lpOleStr, 1, 
                                   lcID, &dispID);
 
  SysFreeString(lpOleStr);

  if (hrResult != S_OK || dispID == DISPID_UNKNOWN) 
     {
     if (lpDispInternal)
       fpDispatchRelease((LONG)lpDispInternal);
 
     return (ERR)-1;
     }

  // for strings, lData must be a BSTR or LPSTR with
  //    memory previously allocated
  if (usDataType == VT_STRING)
  {
    BSTR lpDataStr = (BSTR)fpStr2OLE((LPSTR)lData);
    usDataType = VT_BSTR;
    try
       {
		DISPID dpp = DISPID_PROPERTYPUT;
        v.vt = usDataType;
        v.bstrVal = lpDataStr;
        // now go and get the property into a variant.
        //
        memset(&dispparams, 0, sizeof(DISPPARAMS));
//        hrResult = lpDisp->Invoke(dispID, IID_NULL, 0, DISPATCH_PROPERTYPUT, &dispparams,
//                                  &v, NULL, NULL);
		dispparams.rgvarg = &v;
		dispparams.rgdispidNamedArgs = &dpp;
		dispparams.cArgs = 1;
		dispparams.cNamedArgs = 1;
        hrResult = lpDisp->Invoke(dispID, IID_NULL, 0, DISPATCH_PROPERTYPUT, &dispparams,
                                  NULL, NULL, NULL);
	   }    
    catch(...)
	   {
       SysFreeString(lpDataStr);
       
       if (lpDispInternal)
          fpDispatchRelease((LONG)lpDispInternal);
        
       return ((ERR)-1);         
	   }

    SysFreeString(lpDataStr);
  }
  else 
  {
    if (usDataType == VT_CURRENCY)
      usDataType = VT_CY;        
    try
       {
		DISPID dpp = DISPID_PROPERTYPUT;
        v.vt = usDataType;
        v.lVal = lData;

        memset(&dispparams, 0, sizeof(DISPPARAMS));
//        hrResult = lpDisp->Invoke(dispID, IID_NULL, 0, DISPATCH_PROPERTYPUT, &dispparams,
//                                  &v, NULL, NULL);
		dispparams.rgvarg = &v;
		dispparams.rgdispidNamedArgs = &dpp;
		dispparams.cArgs = 1;
		dispparams.cNamedArgs = 1;
        hrResult = lpDisp->Invoke(dispID, IID_NULL, 0, DISPATCH_PROPERTYPUT, &dispparams,
                                  NULL, NULL, NULL);
	   }
    
    catch(...)
	   {
       if (lpDispInternal)
          fpDispatchRelease((LONG)lpDispInternal);
        
       return ((ERR)-1);         
	   }
  }

  if (lpDispInternal)
    fpDispatchRelease((LONG)lpDispInternal);

  return 0;         
#endif
#else
  return VBSetControlProperty((HCTL)lpObject->hCtl, usPropID, lData);
#endif
}

// the last two parameters are used for OCX only
ERR FPLIB fpVBGetControlProperty(LPFPCONTROL lpObject, USHORT usPropID, LPVOID lData, LPTSTR lpszPropName, USHORT usDataType)
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
  LPDISPATCH lpDispTemp = NULL;
  LPDISPATCH lpDispInternal = 0;
  DISPID dispID;
  LCID lcID = 0; // LOCALE_SYSTEM_DEFAULT;                        
  COleDispatchDriver dispDrv;
  BSTR lpOleStr = (BSTR)fpStr2OLE(lpszPropName);
  IOleClientSite *pOleClientSite;
  IOleControlSite *pOleControlSite;
  IOleObject *pOleObject;
  HRESULT hrResult;

  if (!lpDisp)
     {
     if (!pObject && (lpObject->hWnd) && IsWindow((HWND)lpObject->hWnd))
       pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

     if (pObject)
     {
       if (pObject->IsKindOf(RUNTIME_CLASS(COleControl)))
       {
         lpDisp = ((COleControl *)pObject)->GetExtendedControl();
         if (lpDisp)
            lpDispInternal = lpDisp;
       }
       else if (pObject->IsKindOf(RUNTIME_CLASS(CCmdTarget)))
       {
         ((CCmdTarget *)pObject)->EnableAutomation();
         lpDisp = ((CCmdTarget *)pObject)->GetIDispatch(TRUE);
         lpDispInternal = lpDisp;

         if (NOERROR == lpDisp->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject))
         {
           pOleObject->GetClientSite(&pOleClientSite);
           if (pOleClientSite && NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
           {
             pOleControlSite->GetExtendedControl(&lpDispTemp);
             if (lpDispInternal)
               fpDispatchRelease((LONG)lpDispInternal);
             lpDisp = lpDispTemp;
             lpDispInternal = lpDisp;
             pOleControlSite->Release();
           }
           if (pOleClientSite)
			   pOleClientSite->Release();
           pOleObject->Release();
         }
       }
     }
  }

  if (!lpDisp)
    {
    SysFreeString(lpOleStr);
    return (ERR)-1; 
    }

  if (NOERROR == lpDisp->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject))
  {
	pOleObject->GetClientSite(&pOleClientSite);
    if ((!_ftcsncmp(lpszPropName, _T("Ambient"), 7)) && pOleClientSite &&
        (NOERROR == pOleClientSite->QueryInterface(IID_IDispatch, (LPVOID FAR *)&lpDispTemp)))
    {
	  if (lpDispInternal)
        fpDispatchRelease((LONG)lpDispInternal);
	  
	  lpDisp = lpDispTemp;
	  lpDispInternal = lpDisp;

    }
    else if (pOleClientSite && NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
	{
	  if (NOERROR == pOleControlSite->GetExtendedControl(&lpDispTemp))
      {
	    if (lpDispInternal)
          fpDispatchRelease((LONG)lpDispInternal);

	    lpDisp = lpDispTemp;
	    lpDispInternal = lpDisp;
      }
  	  pOleControlSite->Release();
	}
	if (pOleClientSite)
		pOleClientSite->Release();
    pOleObject->Release();
  }
  

  hrResult = lpDisp->GetIDsOfNames(IID_NULL, (OLECHAR FAR * FAR *)&lpOleStr, 1,
                                   lcID, &dispID);
  
  SysFreeString(lpOleStr);

  if (hrResult != S_OK || dispID == DISPID_UNKNOWN)
     {
     if (lpDispInternal)
       fpDispatchRelease((LONG)lpDispInternal);

     return (ERR)-1;
     }

  dispDrv.AttachDispatch(lpDisp);
  // for strings, lData must be the address of BSTR or LPSTR with
  //    memory previously allocated
  if (usDataType == VT_STRING)
  {
    CString lpOutput((TCHAR)'\0', 514);
    TRY
      {
         dispDrv.GetProperty((DISPID)dispID, 
                             VT_BSTR,
                             (LPVOID)&lpOutput);
      }
    CATCH_ALL(e)
      {
         dispDrv.DetachDispatch();

         if (lpDispInternal)
           fpDispatchRelease((LONG)lpDispInternal);

         return ((ERR)-1);

      }
    END_CATCH_ALL
    
    *(HSZ FAR *)lData = (HSZ)SysAllocString((OLECHAR FAR *)lpOutput.GetBuffer(lpOutput.GetLength() + 1));
//    lstrcpy((LPTSTR)lData, (LPTSTR)lpOutput.GetBuffer(lpOutput.GetLength() + 1));
  }
  else 
  {
    if (usDataType == VT_CURRENCY)
      usDataType = VT_CY;

    TRY
       {
       dispDrv.GetProperty((DISPID)dispID,
                           usDataType,
                           lData);
       }
    CATCH_ALL(e)
       {
         dispDrv.DetachDispatch();

         if (lpDispInternal)
           fpDispatchRelease((LONG)lpDispInternal);

         return ((ERR)-1);

       }
    END_CATCH_ALL
  }

  dispDrv.DetachDispatch();

  if (lpDispInternal)
    fpDispatchRelease((LONG)lpDispInternal);
          
  return 0;         

#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
  LPDISPATCH lpDispInternal = 0;
  LPDISPATCH lpDispTemp;
  DISPID dispID;
  LCID lcID = 0; // LOCALE_SYSTEM_DEFAULT;                        
  BSTR lpOleStr = (BSTR)fpStr2OLE(lpszPropName);
  IOleClientSite *pOleClientSite;
  IOleControlSite *pOleControlSite;
  IOleObject *pOleObject;
  HRESULT hrResult;

  DISPPARAMS dispparams;
  VARIANT v, v2;

  if (!lpDisp)
  {
     if (!pObject && (lpObject->hWnd) && IsWindow((HWND)lpObject->hWnd))
     {
//		pObject = COleControl::ControlFromHwnd((HWND)lpObject->hWnd);
       // _ASSERT(FALSE); // not supported for ATL
        //return 0;
		return (ERR)-1; //scl
     }

     if (pObject)
	 {
	    pOleClientSite = pObject->fpAtlGetClientSite();
	    if (pOleClientSite && 
            NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
	    { 
	       pOleControlSite->GetExtendedControl(&lpDisp);
           lpDispInternal = lpDisp;
	       pOleControlSite->Release();
	    }
	    if (pOleClientSite)
			pOleClientSite->Release();
	 }
  }
  
  if (!lpDisp)
  {
    SysFreeString(lpOleStr);
    return (ERR)-1; 
  }


  if (NOERROR == lpDisp->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject))
  {
	pOleObject->GetClientSite(&pOleClientSite);
	if (pOleClientSite && NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
	{
	  pOleControlSite->GetExtendedControl(&lpDispTemp);
	  if (lpDispInternal)
        fpDispatchRelease((LONG)lpDispInternal);

	  lpDisp = lpDispTemp;
	  lpDispInternal = lpDisp;
	  pOleControlSite->Release();
	}
	if (pOleClientSite)
		pOleClientSite->Release();
	pOleObject->Release();
  }

  hrResult = lpDisp->GetIDsOfNames(IID_NULL, (OLECHAR FAR * FAR *)&lpOleStr, 1, 
                                   lcID, &dispID);
 
  SysFreeString(lpOleStr);

  if (hrResult != S_OK || dispID == DISPID_UNKNOWN) 
     {
     if (lpDispInternal)
       fpDispatchRelease((LONG)lpDispInternal);
 
     return (ERR)-1;
     }

  v.vt = VT_EMPTY;
  v.lVal = 0;
  v2.vt = VT_EMPTY;
  v2.lVal = 0;
  
  memset(&dispparams, 0, sizeof(DISPPARAMS));
  try
    {
    hrResult = lpDisp->Invoke(dispID, IID_NULL, 0, DISPATCH_PROPERTYGET, &dispparams,
                              &v, NULL, NULL);
    }    
  catch(...)
    {
    if (lpDispInternal)
       fpDispatchRelease((LONG)lpDispInternal);
        
    return ((ERR)-1);         
    }

  if (hrResult != 0 || v.vt == VT_EMPTY)
    return (ERR)-1;

  if (usDataType == VT_STRING)
     v2.vt = usDataType = VT_BSTR;

  if (usDataType == VT_CURRENCY)
     v2.vt = usDataType = VT_CY;
 
  if (lpDispInternal)
    fpDispatchRelease((LONG)lpDispInternal);

  hrResult = VariantChangeType(&v2, &v, 0, usDataType);
  
  if (FAILED(hrResult)) 
  {
     VariantClear(&v);
     return ((ERR)-1);         
  }

  // copy the data to where the user wants it
  //
  CopyMemory(lData, &(v2.lVal), g_rgcbDataTypeSize[usDataType]);
  VariantClear(&v);

  return 0;         
#endif
#else
  return VBGetControlProperty((HCTL)lpObject->hCtl, usPropID, lData);
#endif
}

LPSTR FPLIB fpVBGetControlName(LPFPCONTROL lpObject, LPSTR lpszName)                
{
#ifdef FP_OCX  
#ifndef NO_MFC	
  LPTSTR pParend;
  
  *(LPTSTR)lpszName = (TCHAR)'\0';
  
  CObject *pObject = (CObject *)lpObject->lObject;
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);
  
  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
  {
    CString cTempStr;
    cTempStr = ((COleControl *)pObject)->AmbientDisplayName();
    _ftcscpy((LPTSTR)lpszName, (LPTSTR)cTempStr.GetBuffer(cTempStr.GetLength() + 1));
   // if string contains '(' it has index appended to it, null it out
   if (pParend = _ftcschr((LPTSTR)lpszName, (TCHAR)'('))
     *pParend = (TCHAR)0;
   cTempStr.ReleaseBuffer(); 
  }
  else 
  {
    ERR     nErr;
    HSZ     bstr = 0;

    nErr = fpVBGetControlProperty(lpObject, 0, &bstr, (LPTSTR)_T("Name"), VT_STRING);

    if (nErr != -1)
      lstrcpy((LPTSTR)lpszName, (LPCTSTR)fpVBDerefHsz(bstr));
    else
      lpszName = NULL;
      
    if (bstr)
      fpVBDestroyHsz(bstr);
  }
  
  return lpszName; 

#else			 // NO_MFC
  ERR     nErr = 0;
  //BSTR     bstr;
  BSTR     bstr = 0; //scl
 
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  

  if (!lpszName)
	  return 0;

  *(LPTSTR)lpszName = (TCHAR)'\0';
  
//  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
//  {
//	  pObject = COleControl::ControlFromHwnd((HWND)lpObject->hWnd);
//      _ASSERT(FALSE); // not supported for ATL
//      return NULL;
//  }

  if (pObject)
  {
	 pObject->fpAtlGetAmbientDisplayName((BSTR&)bstr);
  }
  else
  {
	  nErr = fpVBGetControlProperty(lpObject, 0, (LPVOID)&bstr, (LPSTR)_T("Name"), VT_STRING);
  }
  
  if (nErr != -1)
  {				
	  FPTSTR fptstr = fpSysAllocTStringFromBSTR(bstr);
	  lstrcpy((LPTSTR)lpszName, (LPCTSTR)fptstr);
	  fpSysFreeTString(fptstr);
  }
  else
     lpszName = NULL;

  if (bstr)
     SysFreeString(bstr);
  
  return lpszName; 
#endif
#else
  return VBGetControlName((HCTL)lpObject->hCtl, lpszName);
#endif
}       

HWND FPLIB fpVBGetControlHwnd(LPFPCONTROL lpObject)                          
{
#ifdef FP_OCX
#ifndef NO_MFC
  HWND hWndTemp = NULL;
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
  IOleWindow *pOleWindow;

  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    return (HWND)lpObject->hWnd;

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
  {
       hWndTemp = ((CWnd *)pObject)->GetSafeHwnd();
  }
  else
  {
    LONG lWindow = 0L;
    
//    if (fpVBGetMode(lpObject) != MODE_DESIGN)
//	{
      if( fpVBGetControlProperty(lpObject, 0, &lWindow, (LPTSTR)_T("hWnd"), VT_I4) != 0 )
//	}
//	else
	{ // try Handle property (.Net equivalent to hWnd)
     if( fpVBGetControlProperty(lpObject, 0, &lWindow, (LPTSTR)_T("Handle"), VT_I4) != 0 )
     {
	     if (lpDisp)
           if (NOERROR == lpDisp->QueryInterface(IID_IOleWindow, (LPVOID FAR *)&pOleWindow))
		   {
             pOleWindow->GetWindow((HWND *)&lWindow);
		     pOleWindow->Release();
		   }
     }
	}
	hWndTemp = (HWND)lWindow;
  }
  return hWndTemp;
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (pObject)
	 hWndTemp = pObject->fpAtlGetHWnd();
  else
  {
//    if (fpVBGetMode(lpObject) != MODE_DESIGN)
//	{
      long lWindow;
      if (fpVBGetControlProperty(lpObject, 0, &lWindow, (LPSTR)_T("hWnd"), VT_I4) == 0)
        hWndTemp = (HWND)lWindow;
//      else if( fpVBGetControlProperty(lpObject, 0, &lWindow, (LPSTR)_T("Handle"), VT_I4) == 0 )
//        hWndTemp = (HWND)lWindow;
//	}
	else // try Handle property (.Net equivalent to hWnd)
     if( fpVBGetControlProperty(lpObject, 0, &lWindow, (LPTSTR)_T("Handle"), VT_I4) == 0 )
        hWndTemp = (HWND)lWindow;
   else
	{
	  // use lpObject->lpDispatch
      LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
	  IOleWindow* pOleWindow = NULL;
	  if (lpDisp && NOERROR == lpDisp->QueryInterface(IID_IOleWindow, (LPVOID FAR *)&pOleWindow) && pOleWindow)
	  {
		pOleWindow->GetWindow(&hWndTemp);
		pOleWindow->Release();
	  }
    }
  }
  return hWndTemp;

#endif
#else
  return VBGetControlHwnd((HCTL)lpObject->hCtl);
#endif
}            

int FPLIB fpVBDialogBoxParam(LPFPCONTROL lpObject, HANDLE hinst, LPSTR lpszTemplateName, 
                                FARPROC lpDialogFunc, LONG lp)   
{
  int nReturn = 0;
#ifdef FP_OCX
#ifndef NO_MFC

  if (lpObject)
  {
    CObject *pObject = (CObject *)lpObject->lObject;

    if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
      pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

    if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
    {
      ((COleControl *)pObject)->PreModalDialog();
      nReturn = ::DialogBoxParam((HINSTANCE)hinst, (LPTSTR)lpszTemplateName,
                       (HWND)((COleControl *)pObject)->GetHwnd(),
                       (DLGPROC)lpDialogFunc, lp);
      ((COleControl *)pObject)->PostModalDialog();
    }
  }
  else
    nReturn = ::DialogBoxParam((HINSTANCE)hinst, (LPTSTR)lpszTemplateName, (HWND)NULL,   /* GetDesktopWindow(), */
                     (DLGPROC)lpDialogFunc, lp);
#else			 // NO_MFC
  if (lpObject)
  {
    CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  

    if (pObject)
    {
	  CComPtr<IOleInPlaceFrame> spInPlaceFrame;
      HWND hWndTemp = NULL;

      // if we have a frame ptr, use ctl hWnd, else use NULL.
	  if (pObject->fpAtlGetInPlaceFramePtr(&spInPlaceFrame))
        hWndTemp = pObject->fpAtlGetHWnd();

      pObject->fpAtlPreModalDialog();
      nReturn = ::DialogBoxParam((HINSTANCE)hinst, (LPTSTR)lpszTemplateName,
                       hWndTemp,
                       (DLGPROC)lpDialogFunc, lp);
      pObject->fpAtlPostModalDialog(FALSE);
    }
    else if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    {
//    pObject = COleControl::ControlFromHwnd((HWND)lpObject->hWnd);
      _ASSERT(0);  // No support for this case (ATL).  SCP 7/30/97
      return -1;
    }
  }
  else
    nReturn = ::DialogBoxParam((HINSTANCE)hinst, (LPTSTR)lpszTemplateName, (HWND)NULL,   /* GetDesktopWindow(), */
                     (DLGPROC)lpDialogFunc, lp);
#endif
#else
  nReturn = VBDialogBoxParam(hinst, lpszTemplateName, lpDialogFunc, lp);   
#endif
  return nReturn;
}                    

#ifdef __cplusplus
}
#endif

#ifdef FP_OCX
#ifndef NO_MFC
class CfpAboutDlg : public CDialog
{
public:
	CfpAboutDlg();
	CfpAboutDlg(UINT nIDTemplate, BOOL fTrialVersion);

	void SetObject(COleControl FAR *pObj) { m_pObject = pObj; }

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnProps();
	DECLARE_MESSAGE_MAP()

	COleControl FAR *m_pObject;
    BOOL m_fTrialVersion;
};

CfpAboutDlg::CfpAboutDlg() : CDialog()
{
}

CfpAboutDlg::CfpAboutDlg(UINT nIDTemplate, BOOL fTrialVersion)
 : CDialog(nIDTemplate)
{
  m_fTrialVersion = fTrialVersion;
}

void CfpAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CfpAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CfpAboutDlg)
	ON_BN_CLICKED(FP_IDC_ABOUTBOX_PROPS_BTN, OnProps)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CfpAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    if (m_fTrialVersion == FALSE)  // release build
    {
      CWnd FAR *lpCWnd = GetDlgItem(FP_IDC_ABOUTBOX_PROPS_BTN);

      if (lpCWnd)
        lpCWnd->ShowWindow(SW_HIDE);
    }
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CfpAboutDlg::OnProps()
{   
   	EndDialog(0);
	m_pObject->OnProperties(NULL, 
					     (m_pObject->GetParent())->GetSafeHwnd(), 
					     NULL);
}

extern "C" {

void FPLIB fpOcxAboutBox(LPVOID lpOleControl, UINT nIDTemplate, 
  BOOL fTrialVersion)
{
   COleControl FAR *lpOle = (COleControl FAR *)lpOleControl;

   CfpAboutDlg dlgAbout(nIDTemplate, fTrialVersion);
   
   dlgAbout.SetObject(lpOle);
   
   lpOle->PreModalDialog();

   dlgAbout.DoModal();

   lpOle->PostModalDialog();
}
} // extern C

#else  // NO_MFC

//class ATL_NO_VTABLE CfpAboutDlg : public CDialogImplBase
class CfpAboutDlg : public CDialogImpl<CfpAboutDlg>
{
public:
	CfpAboutDlg();
	CfpAboutDlg(UINT nIDTemplate, BOOL fTrialVersion);


    BEGIN_MSG_MAP(CfpAboutDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(FP_IDC_ABOUTBOX_PROPS_BTN, BN_CLICKED, OnProps)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnClose)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClose)
    END_MSG_MAP()

    virtual LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, 
      LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnProps(WORD wNotifyCode, WORD wID, 
      HWND hWndCtl, BOOL& bHandled);
    virtual LRESULT OnClose(WORD wNotifyCode, WORD wID, 
      HWND hWndCtl, BOOL& bHandled);

//---------------------------------------------------------------------
// The following were taken from MS's CDialogImpl implementation and
// modified slightly.
	int DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		_ASSERTE(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBase*)this);
		int nRet = ::DialogBoxParam(_Module.GetResourceInstance(),
				MAKEINTRESOURCE(m_IDD),
				hWndParent,
				(DLGPROC)StartDialogProc,
				NULL);
		m_hWnd = NULL;
		return nRet;
	}

	HWND Create(HWND hWndParent)
	{
		_ASSERTE(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBase*)this);
		HWND hWnd = ::CreateDialogParam(_Module.GetResourceInstance(),
				MAKEINTRESOURCE(m_IDD),
				hWndParent,
				(DLGPROC)StartDialogProc,
				NULL);
		_ASSERTE(m_hWnd == hWnd);
		return hWnd;
	}
//---------------------------------------------------------------------

	void SetObject(CfpAtlBase *pObj) { m_pObject = pObj; }

    WORD m_IDD;
	static WORD IDD;
	
// Implementation
protected:
	CfpAtlBase *m_pObject;
    BOOL m_fTrialVersion;
};

CfpAboutDlg::CfpAboutDlg()
{
  m_IDD = 0;  // invalid ID
}

CfpAboutDlg::CfpAboutDlg(UINT nIDTemplate, BOOL fTrialVersion)
{
  m_IDD = (WORD)nIDTemplate;
  m_fTrialVersion = fTrialVersion;
}

LRESULT CfpAboutDlg::OnInitDialog(UINT uMsg, WPARAM wParam, 
      LPARAM lParam, BOOL& bHandled) 
{
//	CDialog::OnInitDialog();
    if (m_fTrialVersion == FALSE)  // release build
    {
      HWND hWnd = GetDlgItem(FP_IDC_ABOUTBOX_PROPS_BTN);

      if (hWnd)
        ::ShowWindow(hWnd, SW_HIDE);
    }

    bHandled = TRUE;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT CfpAboutDlg::OnProps(WORD wNotifyCode, WORD wID, HWND hWndCtl,
  BOOL& bHandled)
{   
	EndDialog(0);
	m_pObject->fpAtlOnProperties(NULL, 
					     m_pObject->fpAtlGetParent(), 
					     NULL);
    bHandled = TRUE;
    return 0;
}

LRESULT CfpAboutDlg::OnClose(WORD wNotifyCode, WORD wID, HWND hWndCtl,
  BOOL& bHandled)
{   
	EndDialog(0);
    bHandled = TRUE;
    return 0;
}

extern "C" {

void FPLIB fpOcxAboutBox(LPVOID lpOleControl, UINT nIDTemplate, 
  BOOL fTrialVersion)
{
   CfpAtlBase *pfpAtl = (CfpAtlBase *)lpOleControl;

   CfpAboutDlg dlgAbout(nIDTemplate, fTrialVersion);
   
   dlgAbout.SetObject(pfpAtl);
   
//   pfpAtl->fpAtlPreModalDialog();

   dlgAbout.DoModal();

//   pfpAtl->fpAtlPostModalDialog();
}
} // extern C

#endif // ifndef NO_MFC

#endif

