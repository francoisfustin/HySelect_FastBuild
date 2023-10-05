/*    PortTool v2.2     fptool04.c          */

#include "fptools.h"

void FPLIB fpCenterWindow (HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
   LONG  x, y, width, height;
#else
   short x, y, width, height;
#endif
   RECT  rect;

   // screen size
   width  = GetSystemMetrics(SM_CXSCREEN);
   height = GetSystemMetrics(SM_CYSCREEN);

   // window size
   GetWindowRect(hWnd, &rect);

   // compute center position
   x = (width  - (rect.right - rect.left)) / 2;
   y = (height - (rect.bottom - rect.top)) / 2;

   // center window
   SetWindowPos (hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

   return;
}  // fpWindowCenter
