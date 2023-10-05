/*    PortTool v2.2     FPTOOL21.C          */

#include "fptools.h"

short FPLIB fpComputeHAlign 
   (UINT uMargin, HWND hWnd, HDC hDC, LPRECT r, LPTSTR lpszData, 
   DWORD dwFirst, DWORD dwLast, DWORD dwLength)
{
   short xLine = (short)r->left+fpLM(uMargin);
   
   // If string fits, compute horizontal alignment
   short dVisibleLength 
      = (short)fpVISIBLE_LENGTH(dwLength, dwFirst, dwLast);
   if (dVisibleLength == (short)dwLength)
      {
      SIZE dwExt;

      GetTextExtentPoint(hDC, lpszData, dVisibleLength, &dwExt);

      if (fpIS_STYLE (hWnd, ES_CENTER))
         xLine += (((short)fpWIN_WIDTH(*r)-(short)dwExt.cx - 1) / 2);
      else if (fpIS_STYLE (hWnd, ES_RIGHT))
         xLine += ((short)fpWIN_WIDTH(*r)-(short)dwExt.cx - 1);
      }
               
   return xLine;
}
