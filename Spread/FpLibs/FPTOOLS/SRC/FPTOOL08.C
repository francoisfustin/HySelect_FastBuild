/*    PortTool v2.2     FPTOOL08.C          */


#include "fptools.h"
#if defined(_WIN64) || defined(_IA64)
LONG_PTR
#else
long 
#endif
         FPLIB fpSendMessageToParent (HWND hWnd, WORD wMsg, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN64) || defined(_IA64)
	LONG_PTR lRet;
#else
   long lRet;
#endif

   if (lRet = SendMessage(hWnd, wMsg, wParam, lParam))
      return (lRet);

   return (SendMessage(GetParent(hWnd), wMsg, wParam, lParam));
}


long FPLIB fpPostMessageToParent (HWND hWnd, WORD wMsg, WPARAM wParam, LPARAM lParam)
{
   long lRet;

   if (lRet = PostMessage(hWnd, wMsg, wParam, lParam))
      return (lRet);

   return (PostMessage(GetParent(hWnd), wMsg, wParam, lParam));
}
