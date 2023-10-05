/*    PortTool v2.2     FPTOOL07.C          */

#include "fptools.h"

// NOTE:  lpbIsHwndValid is a <return only> variable


#if defined(_WIN64) || defined(_IA64)
LONG_PTR FPLIB fpNotifyParent (HWND hWnd, WORD wNotifyMsg, BOOL fSuspend, BOOL FAR *lpbIsHwndValid)  //A849
#else
long FPLIB fpNotifyParent (HWND hWnd, WORD wNotifyMsg, BOOL fSuspend, BOOL FAR *lpbIsHwndValid)  //A849
#endif
{
#if defined(_WIN64) || defined(_IA64)
  LONG_PTR lRet = 0L;
#else
  long lRet = 0L;
#endif
  
  if (!fSuspend)
	 {
#ifdef WIN32
     lRet = fpSendMessageToParent (hWnd, WM_COMMAND, 
        MAKELONG((WORD)GetWindowLong(hWnd, GWL_ID), wNotifyMsg), (LPARAM)hWnd);
#else
     lRet = fpSendMessageToParent (hWnd, WM_COMMAND, 
        GetWindowWord(hWnd, GWW_ID), MAKELONG(hWnd, wNotifyMsg));
#endif
	 }
  *lpbIsHwndValid = IsWindow(hWnd);											//A849

  return lRet;
}

