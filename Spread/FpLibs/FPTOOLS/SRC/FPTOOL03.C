/*    PortTool v2.2     fptool03.c          */

#include "fptools.h"

void FPLIB fpDrawShadows(HDC hDC, LPRECT lpRect, short iShadowSize, COLORREF ColorShadow, 
   COLORREF ColorHighlight, BOOL fShadowIn, BOOL fShadowButton)
{
   HBRUSH   hBrushOld;
   HBRUSH   hBrush;
   short    i;

   hBrush    = CreateSolidBrush(fShadowIn ? ColorShadow : ColorHighlight);
   hBrushOld = SelectObject(hDC, hBrush);

   // left
   for (i = 0; i < iShadowSize; i++)
      PatBlt(hDC, lpRect->left + i, lpRect->top, 1, lpRect->bottom -
         lpRect->top - i, PATCOPY);

   // top
   for (i = 0; i < iShadowSize; i++)
      PatBlt(hDC, lpRect->left, lpRect->top + i, lpRect->right -
         lpRect->left - i, 1, PATCOPY);

   SelectObject(hDC, hBrushOld);
   DeleteObject(hBrush);

   if (!fShadowButton || !fShadowIn)
      {
      hBrush    = CreateSolidBrush(fShadowIn ? ColorHighlight : ColorShadow);
      hBrushOld = SelectObject(hDC, hBrush);

      // right
      for (i = 0; i < iShadowSize; i++)
         PatBlt(hDC, lpRect->right - i - 1, lpRect->top + i, 1, lpRect->bottom -
            lpRect->top - i, PATCOPY);

      // bottom
      for (i = 0; i < iShadowSize; i++)
         PatBlt(hDC, lpRect->left + i, lpRect->bottom - i - 1, lpRect->right -
            lpRect->left - i, 1, PATCOPY);

      SelectObject(hDC, hBrushOld);
      DeleteObject(hBrush);
      } 

   return;
} // fpDrawShadows
