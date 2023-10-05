/***************************************************************************/
/* FPWAPI5.CPP   FarPoint Support for Windows API in non-hWnd OCX's        */
/***************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
//#include "ctlcore.h"
#include "stdafx.h"
#include "fpatl.h"
#endif
#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif


HWND FPLIB fpGetParent(LPFPCONTROL lpObject)
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;
  CWnd *pPrevWnd;
  HWND hWndParent;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
     if (hWndParent = GetParent((HWND)lpObject->hWnd))
        return (hWndParent);
  
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

#if 0
  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))) &&
      (((CWnd *)pObject)->GetSafeHwnd()))
  {
    pPrevWnd = ((COleControl)pObject)->m_pReflect != NULL ? ((CWnd*)(((COleControl)pObject)->m_pReflect))->m_hWnd : ((CWnd)pObject)->hWnd);
    pPrevWnd = ::GetParent(pPrevWnd);

    if (pPrevWnd)
      return pPrevWnd->GetSafeHwnd();
    else
      return NULL;
  }
    
  else

#endif
   if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))) &&
           (((CWnd *)pObject)->GetSafeHwnd()))
  {
    pPrevWnd = ((CWnd *)pObject)->GetParent();
	    
    if (pPrevWnd)
      return pPrevWnd->GetSafeHwnd();
    else
      return NULL;
  }

  else
    return NULL;  
#else	   // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndParent = NULL;

  if (lpObject->hWnd && IsWindow(lpObject->hWnd))
    hWndParent = GetParent(lpObject->hWnd);
  else if (pObject && pObject->fpAtlGetHWnd())
    hWndParent = GetParent(pObject->fpAtlGetHWnd());

  return hWndParent;  
#endif
#else
  return GetParent((HWND)lpObject->hWnd);                           
#endif
}

BOOL FPLIB fpRenderWindow(HWND hWnd, HDC hDC, LPRECT lpRect, BOOL fClientOnly)
{
   BOOL ret = FALSE;
   int iSaveDC;
   RECT rc;

   if( lpRect )
   {
      POINT vptOrg;

      if( fClientOnly )
         GetClientRect(hWnd, &rc);
      else
         GetWindowRect(hWnd, &rc);
      iSaveDC = SaveDC(hDC);
      if( RC_WIDTH(&rc) != RC_WIDTH(lpRect) || RC_HEIGHT(&rc) != RC_HEIGHT(lpRect) )
      {  // scale to cell rect
		   SetMapMode(hDC, MM_ANISOTROPIC);
		   SetWindowExtEx(hDC, RC_WIDTH(&rc), RC_HEIGHT(&rc), NULL);
		   SetViewportExtEx(hDC, RC_WIDTH(lpRect), RC_HEIGHT(lpRect), NULL);
      }
      SetWindowOrgEx(hDC, 0, 0, NULL);
      GetViewportOrgEx(hDC, &vptOrg);
      SetViewportOrgEx(hDC, vptOrg.x + lpRect->left, vptOrg.y + lpRect->top, NULL);
      if( fClientOnly )
         SendMessage(hWnd, WM_PRINT, (WPARAM)hDC, PRF_CHILDREN|PRF_CLIENT);
      else
         SendMessage(hWnd, WM_PRINT, (WPARAM)hDC, PRF_CHILDREN|PRF_CLIENT|PRF_NONCLIENT);
      RestoreDC(hDC, iSaveDC);
      ret = TRUE;
   }
   return ret;
}


#ifdef __cplusplus
}
#endif
