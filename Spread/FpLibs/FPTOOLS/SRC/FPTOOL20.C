/*    PortTool v2.2     FPTOOL20.C          */

#include "fptools.h"

LONG FPLIB fpIsStyle (HWND hWnd, DWORD dwStyle)
{                                  
   LONG lLong = GetWindowLong(hWnd, GWL_STYLE);
   return (lLong & (LONG)dwStyle);
};

