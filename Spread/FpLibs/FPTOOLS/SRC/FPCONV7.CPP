/***************************************************************************/
/* FPCONV7.CPP   FarPoint Support for VBX functions in non-VB environments */
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
#endif

#ifdef WIN32
#ifndef NO_MFC
#include <afxpriv.h>
#endif

#ifdef __T
#undef __T
#endif
#include "tchar.h"
#else

#define WM_NOTIFY                       0x004E

#define USES_CONVERSION int _convert; _convert

#define T2OLE  
#define OLE2T

#define NON_STANDARD_INDEX  12

#endif

#include "vbinterf.h"
// following lines taken from VBINTERF.H
#include <initguid.h>

DEFINE_GUID(IID_IGetOleObject, 0x8A701DA0L, 0x4FEB, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);
DEFINE_GUID(IID_IGetVBAObject, 0x91733A60L, 0x3F4C, 0x101B, 0xA3, 0xF6, 0x00, 0xAA, 0x00, 0x34, 0xE4, 0xE9);
DEFINE_GUID(IID_IVBFormat, 0x9849FD60L, 0x3768, 0x101B, 0x8D, 0x72, 0xAE, 0x61, 0x64, 0xFF, 0xE3, 0xCF);

#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif
 
SHORT FPLIB fpVBFormat(LPFPCONTROL lpObject, SHORT vtype, LPVOID lpData, LPTSTR lpszFmt, LPVOID pb, USHORT cb)
{
#ifdef FP_OCX
  LPOLECLIENTSITE pClientSite = NULL;
  IGetVBAObject  *lpVBAObject;
  IVBFormat      *lpFormat;
  VARIANTARG      varData;
  USHORT          retcb = (USHORT)0;
  BOOL            bStr32 = FALSE;
  BOOL            fReleaseClientSite = TRUE;

#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (!pObject && lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);
  
  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
  {
    // Note:  COleControl::GetClientSite() does not increment "ref count"
    pClientSite = ((COleControl *)pObject)->GetClientSite();
    fReleaseClientSite = FALSE;
  }
  else if (lpObject->lpDispatch)
  {
    IOleObject *pOleObject;
    IGetOleObject *pGetOleObject;

    LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
    
    if (NOERROR == lpDisp->QueryInterface(IID_IGetOleObject, (LPVOID FAR *)&pGetOleObject))
    {
      // If IGetOleObject is supported, get IOleObject from IGetOleObject.
      pGetOleObject->GetOleObject(IID_IOleObject, (LPVOID FAR *)&pOleObject);
      // now release IGetOleObject
      if (pOleObject)
      {
         pOleObject->GetClientSite(&pClientSite);
      }
      pGetOleObject->Release();
    }
    else  // IGetOleObject is NOT supported by given Dispatch interface...
    {
      // Try getting the IOleObject directly from lpDispatch
      lpDisp->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject);
      if (pOleObject)
      {
         pOleObject->GetClientSite(&pClientSite);
         pOleObject->Release();                            
      }
    }
  
  }
#else // ATL
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  

//  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
//  {
//  pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);
//    _ASSERT(FALSE); // not supported for ATL
//    return retcb;
//  }
  if (pObject)
  {
    pClientSite = pObject->fpAtlGetClientSite();
  }
  else if (lpObject->lpDispatch)
  {
    IOleObject *pOleObject;
    IGetOleObject *pGetOleObject;

    LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
    
    if (NOERROR == lpDisp->QueryInterface(IID_IGetOleObject, (LPVOID FAR *)&pGetOleObject))
    {
      // If IGetOleObject is supported, get IOleObject from IGetOleObject.
      pGetOleObject->GetOleObject(IID_IOleObject, (LPVOID FAR *)&pOleObject);
      // now release IGetOleObject
      if (pOleObject)
      {
         pOleObject->GetClientSite(&pClientSite);
      }
      pGetOleObject->Release();
    }
    else  // IGetOleObject is NOT supported by given Dispatch interface...
    {
      // Try getting the IOleObject directly from lpDispatch
      lpDisp->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject);
      if (pOleObject)
      {
         pOleObject->GetClientSite(&pClientSite);
         pOleObject->Release();                            
      }
    }
  
  }

#endif // NO_MFC

  if (pClientSite)
  {
     VariantInit(&varData);
     varData.vt = vtype;

     switch (vtype)
     {
       case VT_I2:
         varData.iVal = *(short *)lpData;
         break;
       case VT_I4:
         varData.lVal = *(long *)lpData;
         break;
       case VT_R4:
         varData.fltVal = *(float *)lpData;
         break;
       case VT_R8:
         varData.dblVal = *(double *)lpData;
         break;
       case VT_STRING:
         // for strings, lData must be the address of BSTR or LPSTR with
         //    memory previously allocated
         varData.vt = VT_BSTR;
#ifdef WIN32
         if (varData.bstrVal = (BSTR)fpStr2OLE(*(LPTSTR *)lpData))
           bStr32 = TRUE;
#else
         varData.bstrVal = *(BSTR *)lpData;
#endif
         break;
       case VT_CURRENCY:
         varData.vt = VT_CY;
         varData.cyVal = *(CY *)lpData;
         break;
       case VT_DATE:
         varData.date = *(DATE *)lpData;
         break;
     }            

     if (NOERROR == pClientSite->QueryInterface(IID_IGetVBAObject,
                                                (LPVOID FAR *)&lpVBAObject))
     {
       if (NOERROR == lpVBAObject->GetObject(IID_IVBFormat,
                                             (LPVOID FAR *)&lpFormat, 0L))
       {        
#ifdef WIN32
#define VBCHAR  WCHAR
#define LPVBSTR LPWSTR
        LPOLESTR bstrFmt = (LPOLESTR)fpStr2OLE(lpszFmt);
        BSTR bstrBuff = SysAllocStringByteLen(NULL, (cb + 1) * sizeof(VBCHAR));
        if (NOERROR == lpFormat->Format((VARIANTARG FAR *)&varData,
                                         bstrFmt, (LPVOID)bstrBuff, cb + 1, 0,
                                         1, 1, &retcb))
        {
          // copy buff to pb
          LPTSTR tstr;
          // null-terminate source string.
          ((LPVBSTR)bstrBuff)[retcb] = (VBCHAR)0;
          tstr = (LPTSTR)fpOLE2Str((LPTSTR)bstrBuff);
          if (tstr && (lstrlen(tstr) < cb))
            lstrcpy((LPTSTR)pb, (LPCTSTR)tstr);

          if (tstr)
            SysFreeString((BSTR)tstr);
        }
        else
          retcb = 0;
        SysFreeString(bstrBuff);
        SysFreeString((BSTR)bstrFmt);
#else //not WIN32
        if (NOERROR != lpFormat->Format((VARIANTARG FAR *)&varData,
                                         (BSTR)lpszFmt, pb, cb, 0, 1, 1, &retcb))
           retcb = 0;
#endif

        lpFormat->Release();        
        lpVBAObject->Release();                            
       }                                       
     } 
   
     if (bStr32)
        SysFreeString((BSTR)varData.bstrVal);
  
     if (fReleaseClientSite && pClientSite)
       pClientSite->Release();
  }

  return retcb;
#else
  return VBFormat(vtype, lpData, lpszFmt, pb, cb);
#endif
} 

#ifdef __cplusplus
}
#endif
