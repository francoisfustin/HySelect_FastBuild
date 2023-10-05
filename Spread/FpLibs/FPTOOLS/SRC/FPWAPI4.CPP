/***************************************************************************/
/* FPWAPI4.CPP   FarPoint Support for Windows API in non-hWnd OCX's        */
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


BOOL FPLIB fpPostMessage(LPFPCONTROL lpObject, UINT Msg, WPARAM wParam, LPARAM lParam)
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    return PostMessage((HWND)lpObject->hWnd, Msg, wParam, lParam);  
  else if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))) &&
      (((CWnd *)pObject)->GetSafeHwnd()))
    return ((CWnd *)pObject)->PostMessage(Msg, wParam, lParam);
  else
    return 0L;
#else  // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
    hWndTemp = lpObject->hWnd;
  else if (pObject && pObject->fpAtlGetHWnd())
    hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
    return PostMessage(hWndTemp, Msg, wParam, lParam);
  else
    return 0L;
#endif // NO_MFC
#else            
  return PostMessage((HWND)lpObject->hWnd, Msg, wParam, lParam);
#endif
}

LRESULT FPLIB fpSendMessage(LPFPCONTROL lpObject, UINT Msg, WPARAM wParam, LPARAM lParam)
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    return SendMessage((HWND)lpObject->hWnd, Msg, wParam, lParam);
  else if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))) &&
      (((CWnd *)pObject)->GetSafeHwnd()))
    return ((CWnd *)pObject)->SendMessage(Msg, wParam, lParam);
  else
    return 0L;
#else  // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
    hWndTemp = lpObject->hWnd;
  else if (pObject && pObject->fpAtlGetHWnd())
    hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
    return SendMessage(hWndTemp, Msg, wParam, lParam);
  else
    return 0L;
#endif // NO_MFC
#else            
  return SendMessage((HWND)lpObject->hWnd, Msg, wParam, lParam);
#endif
}    

#ifdef __cplusplus
}
#endif
