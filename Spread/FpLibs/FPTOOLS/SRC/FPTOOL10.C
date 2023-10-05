/*    PortTool v2.2     FPTOOL10.C          */

#include "fptools.h"

HANDLE FPLIB fpGetTextFromClipboard(HWND hWnd)
{
   LPTSTR  lpszMem, lpszText;
   HANDLE hText = 0;
   HANDLE hMem;
   DWORD  dwLen;

   OpenClipboard(hWnd);

   if (hMem = GetClipboardData(CF_TEXT))
      {
      lpszMem = (LPTSTR)GlobalLock(hMem);

      dwLen = (DWORD)(UINT)lstrlen(lpszMem);
      if (dwLen)
//         if (hText = GlobalAlloc(GHND, ((DWORD)lstrlen(lpszMem) + 1L)))
         if (hText = GlobalAlloc(GHND, (dwLen + 1L) * sizeof(TCHAR)))
            {
            lpszText = (LPTSTR)GlobalLock(hText);
            lstrcpy(lpszText, lpszMem);
            GlobalUnlock(hText);
            }

      GlobalUnlock(hMem);
      }

   CloseClipboard();

   return (hText);
}

