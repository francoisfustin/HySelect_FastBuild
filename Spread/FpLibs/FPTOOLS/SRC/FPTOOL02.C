/*    PortTool v2.2     FPTOOL02.C          */

#include "fptools.h"

BOOL FPLIB fpDrawText3D (HDC hDC, LPTSTR lpszText, short dLen, LPRECT lpRect,
   WORD wFormat, LPTEXT3DINFO lp3DText)
{
COLORREF cr3DColorHighlight = (lp3DText->idClrHiLite ? FPCOLOR(fpCM_GetItem(lp3DText->idClrHiLite, NULL, NULL)) : FPCOLOR(lp3DText->colorHilite));
COLORREF cr3DColorShadow    = (lp3DText->idClrShadow ? FPCOLOR(fpCM_GetItem(lp3DText->idClrShadow, NULL, NULL)) : FPCOLOR(lp3DText->colorShadow));
short    d3DOffset          = lp3DText->bVOffset;
LONG     l3DStyle           = lp3DText->bStyle; 

TEXTMETRIC tm;
COLORREF   clrSave = GetTextColor (hDC);
LONG       cShadow = -1;
LONG       cHiLight = -1;
SIZE       dwSize = {0};
#if defined(_WIN64) || defined(_IA64)
LONG       x;
LONG       y;
#else
short      x;
short      y;
#endif

if (dLen == -1)
   dLen = lstrlen(lpszText);

if (wFormat & (DT_CALCRECT | DT_EXPANDTABS | DT_TABSTOP | DT_WORDBREAK))
   DrawText(hDC, lpszText, dLen, lpRect, wFormat);

else
   {
   wFormat = LOBYTE(wFormat);

   /*******************
   * Cal X coordinate
   *******************/

   if (wFormat & (DT_RIGHT | DT_CENTER))
      {
      GetTextExtentPoint(hDC, lpszText, lstrlen(lpszText), &dwSize);

      if (wFormat & DT_RIGHT)
         x = lpRect->right - (short)dwSize.cx;
      else
         x = lpRect->left - (((short)dwSize.cx - (lpRect->right -
             lpRect->left)) / 2);
      }
   else
      x = (short)lpRect->left;

   if (wFormat & (DT_BOTTOM | DT_VCENTER))
      {
      if ((dwSize.cx == 0) && (dwSize.cy == 0))
         {
         GetTextMetrics(hDC, &tm);
         dwSize.cy = tm.tmHeight;
         }

      if (wFormat & DT_BOTTOM)
         y = lpRect->bottom - (short)dwSize.cy;
      else
         y = lpRect->top - (((short)dwSize.cy - (lpRect->bottom -
             lpRect->top)) / 2);
      }
   else
      y = (short)lpRect->top;

   /***************
   * Draw 3D Text
   ***************/

   if (l3DStyle)
      {
      switch (l3DStyle)
         {
         case TEXT3D_MOREUP:
            cHiLight = cr3DColorShadow;
         case TEXT3D_UP:
            cShadow = cr3DColorHighlight;
            break;

         case TEXT3D_MOREDOWN:
            cShadow = cr3DColorShadow;
         case TEXT3D_DOWN:
            cHiLight = cr3DColorHighlight;
            break;
         }

      /******************
      * Draw Upper Text
      ******************/

      if (cHiLight > 0)
         {
         if (!(wFormat & (DT_RIGHT | DT_CENTER)))
            x += d3DOffset;

         if (!(wFormat & (DT_BOTTOM | DT_VCENTER)))
            y += d3DOffset;
         }

      if (cShadow > 0)
         {
         if (wFormat & DT_RIGHT)
            x -= d3DOffset;

         if (wFormat & DT_BOTTOM)
            y -= d3DOffset;
         }

      clrSave = GetTextColor(hDC);

      if (cHiLight > 0)
         {
         SetTextColor (hDC, (COLORREF)cHiLight);

         ExtTextOut(hDC, x - d3DOffset, y - d3DOffset,
                    (wFormat & DT_NOCLIP) ? 0 : ETO_CLIPPED, lpRect,
                    lpszText, dLen, NULL);
         }

      /******************
      * Draw Lower Text
      ******************/

      if (cShadow > 0)
         {
         SetTextColor (hDC, (COLORREF)cShadow);

         ExtTextOut(hDC, x + d3DOffset, y + d3DOffset,
                    (wFormat & DT_NOCLIP) ? 0 : ETO_CLIPPED, lpRect,
                    lpszText, dLen, NULL);
         }

      SetTextColor (hDC, clrSave);
      }

   ExtTextOut(hDC, x, y, wFormat & DT_NOCLIP ? 0 : ETO_CLIPPED, lpRect,
              lpszText, dLen, NULL);
   }

return (TRUE);
}
