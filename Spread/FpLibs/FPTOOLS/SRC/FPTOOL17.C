/*    PortTool v2.2     FPTOOL17.C          */

// 
// FarPoint Button Drawing Utilities
//
#define OEMRESOURCE 1
#include "fptools.h"

#define BUTTON_OFFSET     3
#define BUTTON_LINESIZE   1
           
#define SQRT_3             1.732
           
static void ButtonAdjustRect(LPRECT dstRect, LPRECT srcRect, short style)
{
#if defined(_WIN64) || defined(_IA64)
   LONG dShortSide;
#else
   short dShortSide;
#endif
   short dWidth = (short)(srcRect->right - srcRect->left);   //KM jis3462
   short dHeight = (short)(srcRect->bottom - srcRect->top);  //KM jis3462
   
   CopyRect(dstRect, srcRect); 
   if (dWidth < (BUTTON_OFFSET*2) || dHeight < (BUTTON_OFFSET*2)) //KM jis3462
      return;                                                     //KM jis3462
   fpShrinkRect(dstRect, BUTTON_OFFSET, BUTTON_OFFSET);

   if (style & FPB_SPIN) // up/down arrows
      {
      dShortSide = ((dstRect->right - dstRect->left) / 2) + 1;

      if (dShortSide < dstRect->bottom - dstRect->top)
         {
         fpShrinkRect(dstRect, 0, (dstRect->bottom - dstRect->top -
            dShortSide) / 2);
         dstRect->bottom = dstRect->top + dShortSide;

         if (((dstRect->right - dstRect->left) % 2) == 0)
            dstRect->right++;
         }
      }
   else if (style & FPB_SPEED) // left/right arrows
      {
      dShortSide = ((dstRect->bottom - dstRect->top) / 2) + 1;

      if (dShortSide < dstRect->right - dstRect->left)
         {
         fpShrinkRect(dstRect, ((dstRect->right - dstRect->left -
            dShortSide) / 2) / 2, 0);
         if (((dstRect->bottom - dstRect->top) % 2) == 0)
           dstRect->bottom++;
         }
      }
   else if ((style & FPB_LEFT) ||
            (style & FPB_RIGHT))              // left/right arrows
      {
      dShortSide = ((dstRect->bottom - dstRect->top) / 2) + 1;

      if (dShortSide < dstRect->right - dstRect->left)
         {
         fpShrinkRect(dstRect, (dstRect->right - dstRect->left -
            dShortSide) / 2, 0);
         dstRect->right = dstRect->left + dShortSide;

         if (((dstRect->bottom - dstRect->top) % 2) == 0)
            dstRect->bottom++;
         }
      }
   else if ((style & FPB_UP) ||
            (style & FPB_DOWN))              // up/down arrows
      {
      dShortSide = ((dstRect->bottom - dstRect->top) / 2) + 1;

      if (dShortSide < dstRect->right - dstRect->left)
         {
         fpShrinkRect(dstRect, (dstRect->right - dstRect->left -
            dShortSide) / 2, 0);
         dstRect->right = dstRect->left + dShortSide;

         if (((dstRect->bottom - dstRect->top) % 2) == 0)
            dstRect->bottom++;
         }
      }
   else
      {
      dShortSide = ((dstRect->bottom - dstRect->top) / 2) + 1;

      if (dShortSide < dstRect->right - dstRect->left)
         {
         fpShrinkRect(dstRect, (dstRect->right - dstRect->left -
            dShortSide) / 2, 0);
         dstRect->right = dstRect->left + dShortSide;

         if (((dstRect->bottom - dstRect->top) % 2) == 0)
            dstRect->bottom++;
         }
      }

   return;
}

static short SetUpArrow(LPPOINT lpPoints, LPRECT lpRect)
{
   RECT r;

   ButtonAdjustRect (&r, lpRect, FPB_SPIN);

   lpPoints[0].x = r.left;
   lpPoints[0].y = r.bottom - 1;

   lpPoints[1].x = r.left + ((r.right - r.left) >> 1);
   lpPoints[1].y = r.top;

   lpPoints[2].x = r.right - 1;
   lpPoints[2].y = r.bottom - 1;

   lpPoints[3].x = r.left;
   lpPoints[3].y = r.bottom - 1;

   lpPoints[4].x = lpPoints[1].x;
   lpPoints[4].y = lpPoints[1].y + ((lpPoints[0].y - lpPoints[1].y) >> 1);

   return (2);
}

static short SetDownArrow(LPPOINT lpPoints, LPRECT lpRect)
{
   RECT r;

   ButtonAdjustRect (&r, lpRect, FPB_SPIN);

   lpPoints[0].x = r.left;
   lpPoints[0].y = r.top;

   lpPoints[1].x = r.right - 1;
   lpPoints[1].y = r.top;

   lpPoints[2].x = r.left + ((r.right - r.left) >> 1);
   lpPoints[2].y = r.bottom - 1;

   lpPoints[3].x = r.left;
   lpPoints[3].y = r.top;

   lpPoints[4].x = lpPoints[2].x;
   lpPoints[4].y = lpPoints[1].y + ((lpPoints[2].y - lpPoints[1].y) >> 1);

   return (1);
}

static short SetRightArrow(LPPOINT lpPoints, LPRECT lpRect)
{
   RECT r;

   ButtonAdjustRect (&r, lpRect, FPB_SLIDE);

   lpPoints[0].x = r.left;
   lpPoints[0].y = r.top;

   lpPoints[1].x = r.right - 1;
   lpPoints[1].y = r.top + ((r.bottom - r.top) >> 1);

   lpPoints[2].x = r.left;
   lpPoints[2].y = r.bottom - 1;

   lpPoints[3].x = r.left;
   lpPoints[3].y = r.top;

   lpPoints[4].x = lpPoints[0].x + ((lpPoints[1].x - lpPoints[0].x) >> 1);
   lpPoints[4].y = lpPoints[1].y;

   return (1);
}

static short SetLeftArrow(LPPOINT lpPoints, LPRECT lpRect)
{
   RECT r;

   ButtonAdjustRect (&r, lpRect, FPB_SLIDE);

   lpPoints[0].x = r.right - 1;
   lpPoints[0].y = r.top;

   lpPoints[1].x = r.right - 1;
   lpPoints[1].y = r.bottom - 1;

   lpPoints[2].x = r.left;
   lpPoints[2].y = r.top + ((r.bottom - r.top) >> 1);

   lpPoints[3].x = r.right - 1;
   lpPoints[3].y = r.top;

   lpPoints[4].x = lpPoints[2].x + ((lpPoints[1].x - lpPoints[2].x) >> 1);
   lpPoints[4].y = lpPoints[2].y;

   return (0);
}

static short SetDoubleRightArrow(LPPOINT lpPoints, LPRECT lpRect)
{
   RECT r;
   POINT ptCenter;

   ButtonAdjustRect (&r, lpRect, FPB_SPEED);

   ptCenter.x = r.left + ((r.right - r.left) >> 1);
   ptCenter.y = r.top + ((r.bottom - r.top) >> 1);

   lpPoints[0].x = r.left;
   lpPoints[0].y = r.top;

   lpPoints[1].x = ptCenter.x;
   lpPoints[1].y = ptCenter.y;

   lpPoints[2].x = ptCenter.x;
   lpPoints[2].y = r.top;

   lpPoints[3].x = r.right - 1;
   lpPoints[3].y = ptCenter.y;

   lpPoints[4].x = ptCenter.x;
   lpPoints[4].y = r.bottom - 1;

   lpPoints[5].x = ptCenter.x;
   lpPoints[5].y = ptCenter.y;

   lpPoints[6].x = r.left;
   lpPoints[6].y = r.bottom - 1;

   lpPoints[7].x = r.left;
   lpPoints[7].y = r.top;

   return (1);
}

static short SetDoubleLeftArrow(LPPOINT lpPoints, LPRECT lpRect)
{
   RECT r;
   POINT ptCenter;

   ButtonAdjustRect (&r, lpRect, FPB_SPEED);

   ptCenter.x = r.left + ((r.right - r.left) >> 1);
   ptCenter.y = r.top + ((r.bottom - r.top) >> 1);

   lpPoints[0].x = r.right;
   lpPoints[0].y = r.top;

   lpPoints[1].x = ptCenter.x;
   lpPoints[1].y = ptCenter.y;

   lpPoints[2].x = ptCenter.x;
   lpPoints[2].y = r.top;

   lpPoints[3].x = r.left + 1;
   lpPoints[3].y = ptCenter.y;

   lpPoints[4].x = ptCenter.x;
   lpPoints[4].y = r.bottom - 1;

   lpPoints[5].x = ptCenter.x;
   lpPoints[5].y = ptCenter.y;

   lpPoints[6].x = r.right;
   lpPoints[6].y = r.bottom - 1;

   lpPoints[7].x = r.right;
   lpPoints[7].y = r.top;

   return (0);
}

static short SetDropDownArrow(LPPOINT lpPoints, LPRECT lpRect)
{
   RECT r;
   short nHeight, nWidth, nSize;
   
   ButtonAdjustRect (&r, lpRect, FPB_DOWN);

   // adjust button rectangle

   nWidth = (short)(r.right - r.left);
   nHeight = (short)(r.bottom - r.top);

   if (nHeight <= nWidth)
   {
     nSize = (short) SQRT_3 * nHeight; 
     r.left += (nWidth - nSize) / 2;
     r.right = r.left + nSize;
  
   }
   else
   {
     nSize = (short) SQRT_3 * nWidth; 
     r.top += (nHeight - nSize) / 2;
     r.bottom = r.top + nSize;   
     
   }
   
   lpPoints[0].x = r.left;
   lpPoints[0].y = r.top;

   lpPoints[1].x = r.right - 1;
   lpPoints[1].y = r.top;

   lpPoints[2].x = r.left + ((r.right - r.left) >> 1);
   lpPoints[2].y = r.bottom - 1;

   lpPoints[3].x = r.left;
   lpPoints[3].y = r.top;

   lpPoints[4].x = lpPoints[2].x;
   lpPoints[4].y = lpPoints[1].y + ((lpPoints[2].y - lpPoints[1].y) >> 1);

   return (1);
}

short FPLIB fpSetAnyArrow(LPPOINT lpPoints, LPRECT lpRect, short styleArrow)
{
   short nReturn = 0;
   
   switch (styleArrow)
     {
     case FPB_LEFT:
        nReturn = SetLeftArrow(lpPoints, lpRect);
        break;
     case FPB_RIGHT:
        nReturn = SetRightArrow(lpPoints, lpRect);
        break;
     case FPB_UP:
        nReturn = SetUpArrow(lpPoints, lpRect);
        break;
     case FPB_DOWN:
        nReturn = SetDownArrow(lpPoints, lpRect);
        break;
     case FPB_SPEEDLEFT:
        nReturn = SetDoubleLeftArrow(lpPoints, lpRect);
        break;
     case FPB_SPEEDRIGHT:
        nReturn = SetDoubleRightArrow(lpPoints, lpRect);
        break;
     case FPB_DROPDOWN:
        nReturn = SetDropDownArrow(lpPoints, lpRect);
        break;
     }

   return nReturn;
}

void FPLIB fpDrawArrowEx(HDC hDC, LPPOINT lpPoints, short dHighlight, BOOL fEnabled, COLORREF foreColor)
{
   HPEN    hPenFore = NULL, hPen = NULL, hPenOld = NULL;
   HBRUSH  hBrushFore = NULL, hBrushOld = NULL;

   if (fEnabled)
      {
      if (foreColor == RGBCOLOR_DEFAULT)
         {
         hBrushOld = SelectObject(hDC, GetStockObject(BLACK_BRUSH));
         hPenOld   = SelectObject(hDC, GetStockObject(BLACK_PEN));
         }
      else
         {
         LOGBRUSH lbrush = {BS_SOLID, FPCOLOR(foreColor), 0L}; 

         hPenFore = CreatePen(PS_SOLID, 1, FPCOLOR(foreColor));
         hPenOld = SelectObject(hDC, hPenFore);
         hBrushFore = CreateBrushIndirect(&lbrush);
         hBrushOld = SelectObject(hDC, hBrushFore);
         }   

      Polygon (hDC, lpPoints, 4);

      SelectObject(hDC, hBrushOld);
      SelectObject(hDC, hPenOld);
      if (hBrushFore)
         DeleteObject(hBrushFore);
      if (hPenFore)
         DeleteObject(hPenFore);
  
      }
   else
      {
      hPen    = CreatePen(PS_SOLID, 1, RGBCOLOR_DARKGRAY);
      hPenOld = SelectObject(hDC, hPen);

      Polygon (hDC, lpPoints, 4);

      SelectObject(hDC, GetStockObject(WHITE_PEN));

      MoveToEx(hDC, lpPoints[dHighlight].x, lpPoints[dHighlight].y, NULL);
      LineTo  (hDC, lpPoints[dHighlight+1].x, lpPoints[dHighlight+1].y);
   
      SelectObject(hDC, hPenOld);
      DeleteObject(hPen);
      }
   return;
}

static void fpDrawArrow(HDC hDC, LPPOINT lpPoints, short dHighlight, BOOL fEnabled)
{
   fpDrawArrowEx(hDC, lpPoints, dHighlight, fEnabled, RGBCOLOR_DEFAULT);
}                                                                         

void FPLIB fpDrawDoubleArrowEx(HDC hDC, LPPOINT lpPoints, short dHighlight, BOOL fEnabled, COLORREF foreColor)
{
   HPEN    hPenFore = NULL, hPen = NULL, hPenOld = NULL;
   HBRUSH  hBrushFore = NULL, hBrushOld = NULL;

   if (fEnabled)
      {
      if (foreColor == RGBCOLOR_DEFAULT)
         {
         hBrushOld = SelectObject(hDC, GetStockObject(BLACK_BRUSH));
         hPenOld   = SelectObject(hDC, GetStockObject(BLACK_PEN));
         }
      else
         {
         LOGBRUSH lbrush = {BS_SOLID, FPCOLOR(foreColor), 0L}; 

         hPenFore = CreatePen(PS_SOLID, 1, FPCOLOR(foreColor));
         hPenOld = SelectObject(hDC, hPenFore);
         hBrushFore = CreateBrushIndirect(&lbrush);
         hBrushOld = SelectObject(hDC, hBrushFore);
         }   

      Polygon (hDC, lpPoints, 8);

      SelectObject(hDC, hBrushOld);
      SelectObject(hDC, hPenOld);

      if (hBrushFore)
         DeleteObject(hBrushFore);
      if (hPenFore)
         DeleteObject(hPenFore);
      }
   else
      {
      hPen    = CreatePen(PS_SOLID, 1, RGBCOLOR_DARKGRAY);
      hPenOld = SelectObject(hDC, hPen);

      Polygon (hDC, lpPoints, 8);

      SelectObject(hDC, GetStockObject(WHITE_PEN));

      MoveToEx(hDC, lpPoints[dHighlight].x, lpPoints[dHighlight].y, NULL);
      LineTo  (hDC, lpPoints[dHighlight+1].x, lpPoints[dHighlight+1].y);
   
      SelectObject(hDC, hPenOld);
      DeleteObject(hPen);
      }
   return;
}

static void fpDrawDoubleArrow(HDC hDC, LPPOINT lpPoints, short dHighlight, BOOL fEnabled)
{
   fpDrawDoubleArrowEx(hDC, lpPoints, dHighlight, fEnabled, RGBCOLOR_DEFAULT);  
}
                                                                         
BOOL FPLIB fpDot (HDC hdc, int l, int t, int r, int b)
{
   MoveToEx(hdc,l,t, NULL);
   LineTo(hdc,r,b);
   return TRUE;
}

void FPLIB fpDrawEllipseEx(HDC hDC, LPRECT lprc, BOOL fEnabled, COLORREF foreColor)
{
#if defined(_WIN64) || defined(_IA64)
   LONG    bottom, width, w2, top, left;
#else
   short   bottom, width, w2, top, left;
#endif
   HPEN    hPenFore = NULL, hPen = NULL, hPenOld = NULL;
   HBRUSH  hBrushFore = NULL, hBrushOld = NULL;
   FARPROC draw;
   
   width  = fpWIN_WIDTH(*lprc) / 7;
   w2     = width<<1;
   bottom = lprc->bottom - (width+2);
   top    = bottom - width;
   left   = lprc->right - ((fpWIN_WIDTH(*lprc) - (6*width) + 1) >> 1);
   
   draw = (width < 2 ? (FARPROC)fpDot : (FARPROC)Ellipse);
   
   if (fEnabled)
      {
      if (foreColor == RGBCOLOR_DEFAULT)
         {
         hBrushOld = SelectObject(hDC, GetStockObject(BLACK_BRUSH));
         hPenOld   = SelectObject(hDC, GetStockObject(BLACK_PEN));
         }
      else
         {
         LOGBRUSH lbrush = {BS_SOLID, FPCOLOR(foreColor), 0L}; 

         hPenFore = CreatePen(PS_SOLID, 1, FPCOLOR(foreColor));
         hPenOld = SelectObject(hDC, hPenFore);   
         hBrushFore = CreateBrushIndirect(&lbrush);
         hBrushOld = SelectObject(hDC, hBrushFore);
         }   
      }
   else
      { 
#if defined(_WIN64) || defined(_IA64)
      ULONG lf = left+1, tp = top+1;
#else
      short lf = left+1, tp = top+1;
#endif
      hPenOld = SelectObject(hDC, GetStockObject(WHITE_PEN));

      draw (hDC, lf-width, tp, lf, bottom);
      lf -= w2;
      draw (hDC, lf-width, tp, lf, bottom);
      lf -= w2;
      draw (hDC, lf-width, tp, lf, bottom);

      hPen    = CreatePen(PS_SOLID, 1, RGBCOLOR_DARKGRAY);
      SelectObject(hDC, hPen);
      }

   draw (hDC, left-width, top, left, bottom);
   left -= w2;                          
   draw (hDC, left-width, top, left, bottom);
   left -= w2;
   draw (hDC, left-width, top, left, bottom);

   if (hBrushOld)
      SelectObject(hDC, hBrushOld);
   if (hPenOld)
      SelectObject(hDC, hPenOld);   
   if (hPen)
      DeleteObject(hPen);
   if (hPenFore)
      DeleteObject(hPenFore);
   if (hBrushFore)
      DeleteObject(hBrushFore);

   return;
}

static void fpDrawEllipse(HDC hDC, LPRECT lprc, BOOL fEnabled)
{
   fpDrawEllipseEx(hDC, lprc, fEnabled, RGBCOLOR_DEFAULT);
}

static void fpDrawDropDown (HDC hDC, LPRECT lprc, BOOL fEnabled)
{
   HDC hDCMem = CreateCompatibleDC(hDC);
   POINT ptSize, ptOrg;
   HBITMAP hBitmap;
   BITMAP bm;
   int x, y;
   
   hBitmap = LoadBitmap(NULL, (LPTSTR)OBM_COMBO);
   SelectObject(hDCMem, hBitmap);
   SetMapMode(hDCMem, GetMapMode(hDC));

   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

   x = lprc->left + (((lprc->right - lprc->left) - bm.bmWidth) / 2);
   y = lprc->top + (((lprc->bottom - lprc->top) - bm.bmHeight) / 2);
   
   ptSize.x = min(bm.bmWidth, lprc->right - lprc->left);
   ptSize.y = min(bm.bmHeight, lprc->bottom - lprc->top);

   if (x < 0)
      ptOrg.x = -x;
   else
      ptOrg.x = 0;

   if (y < 0)
      ptOrg.y = -y;
   else
      ptOrg.y = 0;

   BitBlt(hDC, max(x, lprc->left), max(y, lprc->top), ptSize.x, ptSize.y,
      hDCMem, ptOrg.x, ptOrg.y, (fEnabled ? SRCCOPY : SRCINVERT));

   DeleteDC(hDCMem);
   DeleteObject(hBitmap);

   return;
}

short FPLIB fpButtonWidth (HWND hWnd, short dButtonWidth, BOOL fSpin, short dHeight)
{
   short maxWidth;
   RECT r;
   
   GetWindowRect (hWnd, &r);
   maxWidth = (fpWIN_WIDTH(r) >> 1) - 1;
   
   if (dButtonWidth)
      ;
   else if (fSpin)
      dButtonWidth = (dHeight << 1) / 3;
   else
      dButtonWidth = (dHeight);
   
   return (maxWidth < dButtonWidth ? maxWidth : dButtonWidth);
}

void FPLIB fpButtonRect (LPRECT r, HWND hWnd, BOOL fLeft, BOOL fSpin, short dButtonWidth)
{
   short bWidth, winHeight, xOffset, yOffset;
   RECT  rect;
   POINT pt;
   
   GetClientRect (hWnd, &rect);
   bWidth = fpButtonWidth (hWnd, dButtonWidth, fSpin, (short)fpWIN_HEIGHT(rect));

   if (fLeft)
      pt.x = rect.left, pt.y = rect.top; 
   else
      pt.x = rect.right, pt.y = rect.top;
      
   ClientToScreen (hWnd, &pt);
   winHeight = fpWIN_HEIGHT(rect);
   
   pt.x += (fLeft ? -bWidth : 0);  

   r->left   = 0; 
   r->top    = 0;
   r->right  = bWidth;    
   r->bottom = winHeight-1; 
   
   GetWindowRect (hWnd, &rect);
   xOffset = (short)(pt.x - rect.left);
   yOffset = (short)(pt.y - rect.top);
      
   r->left   += xOffset;
   r->top    += yOffset;
   r->right  += xOffset;
   r->bottom += yOffset;
}

void FPLIB fpButtonDown  (HDC hDC, LPRECT lprc, short style, COLORREF clrButton)
{
   if ((style & FPB_SPIN) == FPB_SPIN)
      InvertRect (hDC, lprc);
   else
	  fpButtonPaint (hDC, *lprc, style, 2, clrButton);
}

BOOL FPLIB fpButtonPaint (HDC hDC, RECT rect, short style, BOOL fEnabled, COLORREF clrButton)
{
   short        x, y, dHighlight;
   POINT        lpPoints[9];
   HBRUSH       oldBrush = NULL;
   HPEN         oldPen = NULL;
   HBRUSH       hBrush = NULL;
   RECT         r;
   RECT			original = rect;

   COLORREF		bkColor = clrButton;
   COLORREF     hiColor = GetSysColor(COLOR_BTNHIGHLIGHT);
   COLORREF     shColor = GetSysColor(COLOR_BTNSHADOW);
   COLORREF     oldColor;
   
   // Erase background
   oldColor = SetBkColor (hDC, bkColor);
   hBrush = CreateSolidBrush(bkColor);
   oldBrush = SelectObject (hDC, hBrush);
   
   if ((style & FPB_BORDER) == FPB_BORDER)
      {
      oldPen = SelectObject (hDC, GetStockObject(BLACK_PEN));
      Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
      fpShrinkRect(&rect, 1, 1);
      }
   else
      {
      oldPen = SelectObject (hDC, GetStockObject(NULL_PEN));
      Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
      SelectObject (hDC, GetStockObject(BLACK_PEN));
      if (style & FPB_BORDERLEFT)
         {
         MoveToEx(hDC, rect.left, rect.top, NULL);
         LineTo(hDC, rect.left, rect.bottom);
         rect.left++;
         }

      if (style & FPB_BORDERRIGHT)
         {
         MoveToEx(hDC, rect.right - 1, rect.top, NULL);
         LineTo(hDC, rect.right - 1, rect.bottom);
         rect.right--;
         }

      if (style & FPB_BORDERTOP)
         {
         MoveToEx(hDC, rect.left, rect.top, NULL);
         LineTo(hDC, rect.right, rect.top);
         rect.top++;
         }

      if (style & FPB_BORDERBOTTOM)
         {
         MoveToEx(hDC, rect.left, rect.bottom - 1, NULL);
         LineTo(hDC, rect.right, rect.bottom - 1);
         rect.bottom--;
         }
      }

   if (fEnabled > 1)
      fpDrawShadows(hDC, &rect, 1, RGBCOLOR_PALEGRAY, RGBCOLOR_DARKGRAY, FALSE, FALSE);
   else
      fpDrawShadows(hDC, &rect, 1, RGBCOLOR_DARKGRAY, RGBCOLOR_WHITE, FALSE, FALSE);
   
   // Draw Horizontal Shadow Lines
   {
   BOOL fLine = FALSE;
   if ((style & FPB_SPIN) == FPB_SPIN)
      {
      y = (short)((rect.bottom - rect.top) >> 1);
      r.left   = rect.left;
      r.right  = rect.right;
      r.top    = rect.top + y;
      r.bottom = rect.top + y + BUTTON_LINESIZE;
      fLine = TRUE;
      }
   else
      {
      short dWidth = (short)(rect.right - rect.left);
      short dHeight = (short)(rect.bottom - rect.top);
      if ( dWidth >= 2 && dHeight >= 2)   //KM jis3462
         fpShrinkRect (&rect, 1, 1);
      if (fEnabled > 1)
         {
         fpDrawShadows(hDC, &rect, 1, bkColor, shColor, FALSE, FALSE);
         rect.left++; rect.right++; rect.top++; rect.bottom++;
         }
      else
         fpDrawShadows(hDC, &rect, 1, shColor, bkColor, FALSE, FALSE);
      }
      
   // Draw Vertical Shadow Lines
   if ((style & FPB_SLIDE) == FPB_SLIDE)
      {
      x = (short)((rect.right - rect.left) >> 1);
      r.left   = rect.left + x;
      r.right  = rect.left + x + BUTTON_LINESIZE;
      r.top    = rect.top;
      r.bottom = rect.bottom;
      fLine = TRUE;
      }

   if (fLine)
      FillRect(hDC, &r, GetStockObject(DKGRAY_BRUSH));
   }
   // Draw Arrows
   if (style & FPB_SPIN)
      {
      if ((style & FPB_SPIN) == FPB_SPIN)
         {
         // Draw Down Arrow
         rect.top += (rect.bottom - rect.top) >> 1;
         dHighlight = SetDownArrow (lpPoints, &rect);
         fpDrawArrow (hDC, lpPoints, dHighlight, fEnabled);
         
		 // Draw Up Arrow
         rect.bottom = rect.top + 1;
         rect.top = original.top;
         dHighlight = SetUpArrow (lpPoints, &rect);
         fpDrawArrow (hDC, lpPoints, dHighlight, fEnabled);

         }
      else if (style & FPB_POPUP)                 // ellipses
         fpDrawEllipse(hDC, &rect, fEnabled);
      else                                        // combobox drop
         {
         dHighlight = SetDropDownArrow (lpPoints, &rect);
         fpDrawArrow (hDC, lpPoints, dHighlight, fEnabled);
//         fpDrawDropDown(hDC, &rect, fEnabled);
         }
      }
   else // Horizontal
      {
      if ((style & FPB_SLIDE) == FPB_SLIDE)
         {
         // Draw Right Arrow
         rect.left += (rect.right - rect.left) >> 1;
         dHighlight = SetRightArrow (lpPoints, &rect);
         fpDrawArrow (hDC, lpPoints, dHighlight, fEnabled);

         // Draw Left Arrow
         rect.right = rect.left + 1;
         rect.left = 0;
         dHighlight = SetLeftArrow (lpPoints, &rect);
         fpDrawArrow (hDC, lpPoints, dHighlight, fEnabled);
         }
      else if (style & FPB_SPEEDLEFT)
         {
         dHighlight = SetDoubleLeftArrow(lpPoints, &rect);
         fpDrawDoubleArrow (hDC, lpPoints, dHighlight, fEnabled);
         }
      else if (style & FPB_SPEEDRIGHT)
         {
         dHighlight = SetDoubleRightArrow(lpPoints, &rect);
         fpDrawDoubleArrow (hDC, lpPoints, dHighlight, fEnabled);
         }
      else if (style & FPB_LEFT)
         {
         dHighlight = SetLeftArrow(lpPoints, &rect);
         fpDrawArrow (hDC, lpPoints, dHighlight, fEnabled);
         }
      else
         {
         dHighlight = SetRightArrow(lpPoints, &rect);
         fpDrawArrow (hDC, lpPoints, dHighlight, fEnabled);
         }
      }

   SelectObject(hDC, oldBrush);
   DeleteObject(hBrush);
   SetBkColor (hDC, oldColor);
   SelectObject (hDC, oldPen);

   return (TRUE);
}


short FPLIB fpHitTest (HWND hWnd, LPPOINT lpPoint, short style, BOOL fLeft, 
   BOOL fSpin, short dButtonWidth, LPRECT r)
{
   short direction, bWidth, winHeight, xOffset, yOffset;
   RECT  rect;
   POINT pt;
   POINT tmpPt = *lpPoint;   // use tmp point since we change the value

   
   if (lpPoint->x == -1)
      return FPB_UP;
   else if (lpPoint->x == -2)
      return FPB_DOWN;
      
   GetClientRect (hWnd, &rect);
   bWidth = fpButtonWidth (hWnd, dButtonWidth, fSpin, (short)fpWIN_HEIGHT(rect));

   if (fLeft)
      pt.x = rect.left, pt.y = rect.top; 
   else
      pt.x = rect.right, pt.y = rect.top;
      
   ClientToScreen (hWnd, &pt);
   winHeight = fpWIN_HEIGHT(rect);
   
   pt.x += (fLeft ? -bWidth : 0);

   r->left   = 0; 
   r->top    = 0;
   r->right  = bWidth;      
   r->bottom = winHeight-1;
   
   tmpPt.x -= pt.x;
   tmpPt.y -= pt.y;
   
   if (!PtInRect (r, tmpPt))
      return 0;
      
   GetWindowRect (hWnd, &rect);
   xOffset = (short)(pt.x - rect.left);
   yOffset = (short)(pt.y - rect.top);
      
   // Loyal Begin Changes
   if ((style & FPB_SPIN) == FPB_SPIN)
      {
      if (tmpPt.y <= r->bottom >> 1)
         r->bottom >>= 1, direction = FPB_UP;
      else
         r->top = (r->bottom >> 1) + 1, direction = FPB_DOWN;
      fLeft ? r->right-- : r->left++ ;
      
      }
   else 
      direction = (style & 0x000F);
  // Loyal End Changes
   r->left   += xOffset;
   r->top    += yOffset;
   r->right  += xOffset;
   r->bottom += yOffset;
   
   return direction;
}


                                 
