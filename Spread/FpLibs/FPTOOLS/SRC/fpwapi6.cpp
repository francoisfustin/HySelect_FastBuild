/***************************************************************************/
/* FPWAPI.CPP   FarPoint Support for Windows API in non-hWnd OCX's        */
/***************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
#include "stdafx.h"
#include "fpatl.h"
#endif
#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif


HINSTANCE FPLIB fpGetInstanceHandle(LPFPCONTROL lpObject)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  HINSTANCE hInst = 0;

  if (lpObject->hWnd)
     hInst = GETGW_HINSTANCE(lpObject->hWnd);  
  else if (lpObject->lObject)
  {
    CObject *pObject = (CObject *)lpObject->lObject;

    if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
      hInst = GETGW_HINSTANCE(((CWnd *)pObject)->GetSafeHwnd());  
  }
  else
  {
    hInst = AfxGetInstanceHandle();
	// AfxGetInstanceHandle returned null, attempt AfxGetResourceHandle
//	if (!hInst)
//       hInst = AfxGetResourceHandle();
  }

  return hInst;
#else  // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;
  HINSTANCE hInst = 0;

  if (lpObject->hWnd)
    hWndTemp = lpObject->hWnd;
  else if (pObject && pObject->fpAtlGetHWnd())
    hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
    hInst = GETGW_HINSTANCE(hWndTemp);  

  return hInst;
#endif // NO_MFC
#else     
  HINSTANCE hInst = 0;
  if (lpObject->hWnd)
     hInst = GETGW_HINSTANCE(lpObject->hWnd);  
  return hInst;
#endif
}
         
void FPLIB fpGetClientRect(LPFPCONTROL lpObject, LPRECT lprect)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
  {
    ((CWnd *)pObject)->GetClientRect(lprect);
  }
  else
  {
    fpGetRectFromDispatch(lpObject, lprect);
    OffsetRect(lprect, -lprect->left, -lprect->top);
  }
#else  // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
    hWndTemp = lpObject->hWnd;
  else if (pObject && pObject->fpAtlGetHWnd())
    hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
  {
    GetClientRect(hWndTemp, lprect);
  }
  else
  {
    fpGetRectFromDispatch(lpObject, lprect);
    OffsetRect(lprect, -lprect->left, -lprect->top);
  }
#endif // NO_MFC
#else
  GetClientRect((HWND)lpObject->hWnd, lprect);
#endif
}   

HWND FPLIB fpSetFocus(LPFPCONTROL lpObject)
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;
  CWnd *pPrevWnd;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);
  
  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
  {
    pPrevWnd = ((CWnd *)pObject)->SetFocus();
    if (pPrevWnd)
      return pPrevWnd->GetSafeHwnd();
    else
      return NULL;
  }
  else
    return NULL;  
#else  // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
    hWndTemp = lpObject->hWnd;
  else if (pObject && pObject->fpAtlGetHWnd())
    hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
    return SetFocus(hWndTemp);

  return NULL;

#endif // NO_MFC
#else
  return SetFocus((HWND)lpObject->hWnd);                           
#endif
}

HWND FPLIB fpGetFocus(LPFPCONTROL lpObject)
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;
  CWnd *pPrevWnd;
  
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
  {
    pPrevWnd = ((CWnd *)pObject)->GetFocus();
    if (pPrevWnd)
      return pPrevWnd->GetSafeHwnd();
    else
      return NULL;
  }
  else
    return NULL;  
#else  // NO_MFC
  return GetFocus();                           
#endif // NO_MFC
#else
  return GetFocus();                           
#endif
}

BOOL FPLIB fpIsEnabled(LPFPCONTROL lpObject)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
    return ((COleControl *)pObject)->GetEnabled();
  else if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    return ((CWnd *)pObject)->IsWindowEnabled();
  else
  {
    LONG lEnabled = FALSE;
    fpVBGetControlProperty(lpObject, 0, &lEnabled, _T("Enabled"), VT_I4);
    return (BOOL)lEnabled;
  }
#else  // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  

  if (pObject)
  {
    VARIANT var;
	HRESULT h = pObject->fpAtlGetAmbientProperty(DISPID_ENABLED, var);

    if (h == S_OK && var.vt == VT_BOOL)
      return var.bVal;
    else
      return FALSE;
  }

  if (lpObject->hWnd && IsWindow(lpObject->hWnd))
    return IsWindowEnabled(lpObject->hWnd);

  {
    LONG lEnabled = FALSE;
    fpVBGetControlProperty(lpObject, 0, &lEnabled, _T("Enabled"), VT_I4);
    return (BOOL)lEnabled;
  }
#endif // NO_MFC
#else  // FP_OCX
  if (lpObject->hWnd  && IsWindow((HWND)lpObject->hWnd))
  {
    return IsWindowEnabled(lpObject->hWnd);
  }
#endif

return (0);
}

BOOL FPLIB fpIsWindow(LPFPCONTROL lpObject)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  BOOL bReturn = FALSE;
  CObject *pObject = (CObject *)lpObject->lObject;
  
  if (lpObject->hWnd)
  {
    if (IsWindow((HWND)lpObject->hWnd))
	  bReturn = TRUE;
	else
	  bReturn = FALSE;
  }
  else if ((pObject) && (AfxIsValidAddress((LPVOID)pObject, sizeof(CWnd *), TRUE)) 
      && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
	bReturn = TRUE;

  return bReturn;
#else   // NO_MFC
  BOOL bReturn = FALSE;
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  
  if (lpObject->hWnd)
  {
    if (IsWindow((HWND)lpObject->hWnd))
	  bReturn = TRUE;
  }
  else if (pObject && IsWindow(pObject->fpAtlGetHWnd()))
	bReturn = TRUE;

  return bReturn;
#endif  // NO_MFC
#else
  return IsWindow((HWND)lpObject->hWnd);
#endif
}   

#ifdef __cplusplus
}
#endif
