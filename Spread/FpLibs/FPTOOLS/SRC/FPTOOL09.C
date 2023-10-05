/*    PortTool v2.2     FPTOOL09.C          */

#include "fptools.h"
#include "string.h"

BOOL FPLIB fpSendTextToClipboard  (HWND hWnd, LPTSTR lpszText, UINT fuLen)
{
   BOOL   fRet = FALSE;
   LPTSTR  lpszMem;
   HANDLE hMem;

   if (hMem = GlobalAlloc(GHND, (DWORD)(fuLen + 1)*sizeof(TCHAR)))
      {
      lpszMem = (LPTSTR)GlobalLock(hMem);
      lstrcpyn(lpszMem, lpszText, fuLen+1);
      GlobalUnlock(hMem);

      OpenClipboard(hWnd);
      EmptyClipboard();

      SetClipboardData(CF_TEXT, hMem);
      CloseClipboard();

      fRet = TRUE;
      }

   return fRet;
}


