/***************************************************************************/
/* FPWAPI1.CPP   FarPoint Support for Windows API in non-hWnd OCX's        */
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
         
void FPLIB fpUpdateWindow(LPFPCONTROL lpObject)                           
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))) &&
      (((CWnd *)pObject)->GetSafeHwnd()))
    ((CWnd *)pObject)->UpdateWindow();
#else // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  
  if (pObject)
  	 pObject->fpAtlUpdateWindow();
  else if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
     UpdateWindow(lpObject->hWnd);

#endif // NO_MFC
#else

  if (lpObject->hWnd && IsWindowVisible((HWND)lpObject->hWnd))
     UpdateWindow((HWND)lpObject->hWnd);
#endif
}    

void FPLIB  fpInvalidateRect(LPFPCONTROL lpObject, LPRECT lprect, BOOL bErase) 
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if (pObject)
  {
    if (pObject->IsKindOf(RUNTIME_CLASS(COleControl)))
    {
      ((COleControl *)pObject)->InvalidateControl(lprect);
    }    
    else if (pObject->IsKindOf(RUNTIME_CLASS(CWnd)))
    {       
      if (((CWnd *)pObject)->GetSafeHwnd())      
        ((CWnd *)pObject)->InvalidateRect(lprect);    
    } 
  }  
#else // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  
  if (pObject)
  	 pObject->fpAtlInvalidateControl(lprect, bErase);
  else if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
  	 InvalidateRect(lpObject->hWnd, lprect, bErase);

#endif // NO_MFC  
#else  // if FP_OCX...
  InvalidateRect((HWND)lpObject->hWnd, lprect, bErase);
#endif
}

BOOL FPLIB fpMoveWindow(LPFPCONTROL lpObject, int x, int y, int cx, int cy,
                         BOOL fRepaint, BYTE bLineOrient)
{
#ifdef FP_OCX
#ifndef NO_MFC
  BOOL bReturn = FALSE;
  RECT rect;
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
  {
     if (((CWnd *)pObject)->GetSafeHwnd())
	 {
        if (pObject->IsKindOf(RUNTIME_CLASS(COleControl)))
        {
           RECT Rect = {x, y, x + cx, y + cy};

           ((COleControl *)pObject)->SetRectInContainer(&Rect);
//           ((COleControl *)pObject)->SetControlSize(cx, cy);
        }
        else
           ((CWnd *)pObject)->MoveWindow(x, y, cx, cy, fRepaint);

		bReturn = TRUE;
	 }
  }
  else
  {
    rect.left = x;
    rect.top = y;
	rect.bottom = rect.top + cy;
	rect.right = rect.left + cx;
    fpSetRectFromDispatch(lpObject, (LPRECT)&rect, bLineOrient);
	bReturn = TRUE;
  }
  return bReturn;
#else // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  RECT rect = {x, y, x + cx, y + cy};

  if (pObject)
  {
    pObject->fpAtlSetRectInContainer(&rect);
  }
  else
  {
    fpSetRectFromDispatch(lpObject, (LPRECT)&rect, bLineOrient);
  }
  return TRUE;
#endif // NO_MFC
#else  // FP_OCX
  return MoveWindow((HWND)lpObject->hWnd, x, y, cx, cy, fRepaint);
#endif
}       

#ifdef __cplusplus
}
#endif
