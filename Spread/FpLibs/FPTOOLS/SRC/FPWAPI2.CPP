/***************************************************************************/
/* FPWAPI2.CPP   FarPoint Support for Windows API in non-hWnd OCX's        */
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

LONG FPLIB fpGetIDispatchFromObject(LPFPCONTROL lpObject)
{
  LONG lReturn = 0L;
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CCmdTarget))))
  {  
   	((CCmdTarget *)pObject)->EnableAutomation();
    lReturn  = (LONG)((CCmdTarget *)pObject)->GetIDispatch(TRUE);
  }
#else  // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  

  if (pObject)
  {  
    lReturn  = (LONG)pObject->fpAtlGetIDispatch(TRUE);
  }
#endif // NO_MFC
#endif // FP_OCX
  return lReturn;
}

LONG FPLIB fpGetObjectFromIDispatch(LPFPCONTROL lpObject)
{
  LONG lReturn = 0L;
#ifdef FP_OCX
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;

//  if (lpDisp)
//  {  
//    lReturn  = (LONG)CCmdTarget::FromIDispatch(lpDisp);
//  }
#endif
  return lReturn;
}

#ifdef __cplusplus
}
#endif
