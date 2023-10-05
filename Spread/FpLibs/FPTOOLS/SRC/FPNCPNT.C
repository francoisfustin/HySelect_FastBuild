/*    PortTool v2.2     FPNCPNT.C          */

#include "fptools.h"
#include "fpncpnt.h" 
#include "string.h"

#define FPCOLOR_SMART(idClr, clr) (FPCOLOR((idClr ? fpCM_GetItem(idClr, NULL, NULL) : clr)))

_inline short CalcBorderSize (LPFPNCINFO lpInfo, short margin)
{
   if (lpInfo->lStyle & FP_NCBORDER)
      margin += lpInfo->bBorderSize;
   return margin;
}

_inline short Calc3DSize (LPFPNCINFO lpInfo, short margin)
{
   if (lpInfo->lStyle & FP_NC3DOUTMASK)
      {
      margin += lpInfo->bOutShadowWidth + lpInfo->bFrameWidth;
      if (lpInfo->lStyle & FP_NC3DINMASK)
         margin += lpInfo->bInShadowWidth;
      }        
   return margin;
}

_inline short CalcDropShadowWidth (LPFPNCINFO lpInfo, short margin)
{
   if (lpInfo->lStyle & (FP_NCDROPSHADOW | FP_NCDROPSHADOWFOCUS))
      margin += lpInfo->bDropShadowWidth;
   return margin;
}

_inline short fpCalcLeftMargin (LPFPNCINFO lpInfo, short dButtonWidth)
{
   short dLM = 0;
   
   dLM = CalcBorderSize (lpInfo, dLM);
   dLM = Calc3DSize     (lpInfo, dLM);

   if (FP_LBUTTON(lpInfo))
      dLM += dButtonWidth;

//#ifdef BTNALIGN
   if (lpInfo->fButtonLeft && (lpInfo->lStyle & FP_NCSLIDE) != FP_NCSLIDE)
      dLM += dButtonWidth;
//#endif
   return dLM;
}

_inline short fpCalcTopMargin (LPFPNCINFO lpInfo)
{
   short dTM = 0;
   
   dTM = CalcBorderSize (lpInfo, dTM);
   dTM = Calc3DSize     (lpInfo, dTM);

   return dTM;
}

_inline short fpCalcRightMargin (LPFPNCINFO lpInfo, short dButtonWidth)
{
   short dRM = 0;

   dRM = CalcBorderSize     (lpInfo, dRM);
   dRM = Calc3DSize         (lpInfo, dRM);
   dRM = CalcDropShadowWidth (lpInfo, dRM);
   if (FP_RBUTTON(lpInfo))
      dRM += dButtonWidth;

//#ifdef BTNALIGN
   if (lpInfo->fButtonLeft && (lpInfo->lStyle & FP_NCSLIDE) != FP_NCSLIDE)
      dRM -= dButtonWidth;
//#endif      
   return dRM;
}

_inline short fpCalcBottomMargin (LPFPNCINFO lpInfo)
{
   short dBM = 0;

   dBM = CalcBorderSize     (lpInfo, dBM);
   dBM = Calc3DSize         (lpInfo, dBM);
   dBM = CalcDropShadowWidth (lpInfo, dBM);
   //if (lpInfo->dHSize)
   //   dBM += lpInfo->dHSize-(lpInfo->lStyle & FP_NCSCROLLIN ? 0 : 1);
   return dBM;
}

LONG FPLIB fpWMNCCalcSize (HWND hWnd, LPFPNCINFO lpInfo, BOOL fCalcValidRects, LPNCCALCSIZE_PARAMS lpCalc)
{                                             
   short dButtonWidth = 0;
   LONG  rCode = 0L;
   
   lpCalc->rgrc[0].top    += fpCalcTopMargin    (lpInfo);
   lpCalc->rgrc[0].bottom -= fpCalcBottomMargin (lpInfo);

   {
   BOOL f1 = !(lpInfo->lStyle & FP_NCBUTTONHIDE);
   BOOL f2 = (GetFocus() == hWnd);
   if ((lpInfo->lStyle & FP_NCBUTTON) &&
       (!(lpInfo->lStyle & FP_NCBUTTONHIDE) || (GetFocus() == hWnd)))
      {
      BOOL  fSpin = ((lpInfo->lStyle & FP_NCSPIN) == FP_NCSPIN);
      short dHeight = fpWIN_HEIGHT(*lpCalc->rgrc);
      dButtonWidth = fpButtonWidth (hWnd, lpInfo->dButtonWidth, fSpin, dHeight);
      }
   }
   lpCalc->rgrc[0].left   += fpCalcLeftMargin   (lpInfo, dButtonWidth);
   lpCalc->rgrc[0].right  -= fpCalcRightMargin  (lpInfo, dButtonWidth);   
   
   return rCode;
}

void FPLIB fpWMNCPaint (HWND hWnd, LPFPNCINFO info, short redraw, HDC hDCInput)
{
   HDC      hDC;   
   COLORREF pColor = 0L;
   short    winLeft, winTop;
   int      oldMode;
   RECT     r;
   int rright,rbottom;
   BOOL     fFocus = (GetFocus() == hWnd);
   BOOL     fTest;
   HWND     phWnd = GetParent (hWnd);
   BOOL		fMouseDown = FALSE;	
   int iSwap;
  
   //if (info->fSuspend)
	   //return;

   // CTF to get NC Paint to go to printer  
   if (!hDCInput)
     hDC = GetWindowDC (hWnd);
   else
     hDC = hDCInput;
     
   // Get parent background color
   //fTest = (fpCM_GetItem(info->idColorBk, NULL, NULL) == RGBCOLOR_DEFAULT);
   fTest = (FPCOLOR_SMART(info->idColorBk, info->colorBk) == RGBCOLOR_DEFAULT);
   if (fTest && phWnd)
      {
// JIS3669... 
      HBRUSH hbrBack;
      HDC phDC = GetDC(phWnd);  
         
#ifndef WIN32
      hbrBack = (HBRUSH)SendMessage(phWnd, 
                                    WM_CTLCOLOR, 
                                    (WPARAM)phDC, 
                                    (LPARAM)MAKELONG(hWnd, CTLCOLOR_DLG));

#else
      hbrBack = (HBRUSH)SendMessage(phWnd, 
                                    WM_CTLCOLORDLG, 
                                    (WPARAM)phDC, 
                                    (LPARAM)phWnd);
#endif
      if (hbrBack) 
         {
         LOGBRUSH logBrush;
         GetObject(hbrBack, sizeof(LOGBRUSH), &logBrush);
         pColor = logBrush.lbColor;
         }
      else
// JIS3669 
         {
         pColor = GetBkColor(phDC);
         }
      ReleaseDC(phWnd, phDC);
      }
   else
      pColor = FPCOLOR_SMART(info->idColorBk, info->colorBk);

   // Get window size
   GetWindowRect (hWnd, &r);

   SetMapMode (hDC, MM_TEXT);
   oldMode = SetBkMode  (hDC, OPAQUE);   
   
   winLeft = (short)r.left;
   winTop  = (short)r.top;
   r.right -= r.left;
   r.bottom -= r.top;
   r.left = r.top = 0;
   rright = r.right;
   rbottom = r.bottom;
   // Paint drop shadow
   if (info->lStyle & (FP_NCDROPSHADOW | FP_NCDROPSHADOWFOCUS))
      {
      if (redraw & FPNC_REDRAW_DROPSHADOW)
         {
#if defined(_WIN64) || defined(_IA64)
         LONG  x1 = (short)r.left, x2 = (short)r.right-1;
         LONG  y1 = (short)r.top,  y2 = (short)r.bottom-1;
#else
         short x1 = (short)r.left, x2 = (short)r.right-1;
         short y1 = (short)r.top,  y2 = (short)r.bottom-1;
#endif
         short i = info->bDropShadowWidth;
         HPEN  hPen, savePen;
         
         // Use parent bkcolor to fill in drop region 
         {
         hPen = CreatePen (PS_SOLID, 1, pColor);
         savePen = SelectObject (hDC, hPen);
         while (i)
            {
            i--;
            MoveToEx (hDC, x1, y2-i, NULL); LineTo (hDC, x2+1, y2-i);
            MoveToEx (hDC, x2-i, y1, NULL); LineTo (hDC, x2-i, y2+1);
            }
         SelectObject (hDC, savePen);
         DeleteObject (hPen);
         }
         
         // paint drop shadow when in focus
         if ((info->lStyle & FP_NCDROPSHADOW) || fFocus)
            {
            HPEN hPen = CreatePen (PS_SOLID, 1, FPCOLOR_SMART(info->idColorDropShadow, info->colorDropShadow));

            HPEN savePen = SelectObject (hDC, hPen);
            x1 += max(info->bDropShadowWidth, info->bBorderSize);
            y1 += max(info->bDropShadowWidth, info->bBorderSize);
            i = info->bDropShadowWidth;

            while (i)
               {
               i--;
               MoveToEx (hDC, x1, y2-i, NULL); LineTo (hDC, x2+1, y2-i);
               MoveToEx (hDC, x2-i, y1, NULL); LineTo (hDC, x2-i, y2);
               }
            SelectObject (hDC, savePen);
            DeleteObject (hPen);

            // Paint rounded corners          

            if (info->lStyle & FP_NCROUND)
               {
               if (!(redraw & FPNC_REDRAW_BORDER))
                  {
                  x2 = r.right - 1 - info->bDropShadowWidth;
                  y2 = r.bottom - 1 - info->bDropShadowWidth;
                  i = info->bBorderSize;

                  hPen = CreatePen (PS_SOLID, 1, FPCOLOR_SMART(info->idColorDropShadow, info->colorDropShadow));
                  savePen = SelectObject (hDC, hPen);

                  while (i)
                     {
                     MoveToEx (hDC, x2, y2, NULL); LineTo (hDC, x2-i, y2);
                     i--, y2--;
                     }
                  SelectObject(hDC, savePen);
                  DeleteObject(hPen);
                  }
               }                  
             /*if (info->lStyle & FP_NCROUND)
               {
               x2 = r.right-1;
               y2 = r.bottom-1;
               i = info->bBorderSize;
         
               hPen = CreatePen (PS_SOLID, 1, FPCOLOR(info->colorDropShadow));
               savePen = SelectObject (hDC, hPen);

               while (i)
                  {
                  MoveToEx (hDC, x2, y2, NULL); LineTo (hDC, x2-i, y2);
                  i--, y2--;   
                  }
               SelectObject(hDC, savePen);
               DeleteObject(hPen);
               }                  */
            }
         }
      r.right  -= info->bDropShadowWidth;
      r.bottom -= info->bDropShadowWidth;
      }

   if (info->lStyle & FP_NCBORDER)
      {
      if (redraw & FPNC_REDRAW_BORDER)
         {                
         COLORREF saveBkColor = SetBkColor (hDC, pColor);
#if defined(_WIN64) || defined(_IA64)
         LONG  x1 = (short)r.left, x2 = (short)r.right-1;
         LONG  y1 = (short)r.top,  y2 = (short)r.bottom-1;
#else
         short x1 = (short)r.left, x2 = (short)r.right-1;
         short y1 = (short)r.top,  y2 = (short)r.bottom-1;
#endif
         short i = info->bBorderSize;
         HPEN  hPen, savePen;

         if (fFocus)
            hPen = CreatePen (info->bPenStyleOnFocus, 1, FPCOLOR_SMART(info->idColorBorder, info->colorBorder));
         else
            hPen = CreatePen (info->bPenStyleNoFocus, 1, FPCOLOR_SMART(info->idColorBorder, info->colorBorder));
         
         savePen = SelectObject (hDC, hPen);
     
         while (i)
            {
            i--;
            MoveToEx (hDC, x1, y1+i, NULL); LineTo (hDC, x2+1, y1+i);  // top
            MoveToEx (hDC, x1+i, y1, NULL); LineTo (hDC, x1+i, y2+1);  // left
            MoveToEx (hDC, x2-i, y2, NULL); LineTo (hDC, x2-i, y1);    // right
            MoveToEx (hDC, x2, y2-i, NULL); LineTo (hDC, x1, y2-i);    // bottom
            }                

         SelectObject (hDC, savePen);
         DeleteObject (hPen);

         // Paint rounded corners          

         if (info->lStyle & FP_NCROUND)
            {      
            //HPEN hPenShadow;
            x1 = (short)r.left, x2 = (short)r.right-1;
            y1 = (short)r.top,  y2 = (short)r.bottom-1;
            i = info->bBorderSize;
         
            hPen = CreatePen (PS_SOLID, 1, pColor); //KM jis3466 
            savePen = SelectObject (hDC, hPen);        // 

            while (i)          
               {
               MoveToEx (hDC, x1, y1, NULL); LineTo (hDC, x1+i, y1);
               MoveToEx (hDC, x1, y2, NULL); LineTo (hDC, x1+i, y2);
               MoveToEx (hDC, x2, y1, NULL); LineTo (hDC, x2-i, y1);
               /*if ((info->lStyle & FP_NCDROPSHADOWFOCUS && fFocus) ||
                      info->lStyle & FP_NCDROPSHADOW)  
                  {
                  hPenShadow = CreatePen (PS_SOLID, 1, FPCOLOR(info->colorDropShadow));
                  SelectObject (hDC, hPenShadow);
                  MoveToEx (hDC, x2, y2, NULL), LineTo (hDC, x2-i, y2);
                  SelectObject(hDC, hPen);
                  DeleteObject(hPenShadow);

                  }
 			   else if ((info->lStyle & FP_NCDROPSHADOWFOCUS) && !fFocus)
                  {
                  hPenShadow = CreatePen (PS_SOLID, 1, pColor);
                  SelectObject (hDC, hPenShadow);
                  MoveToEx (hDC, x2, y2, NULL), LineTo (hDC, x2-i, y2);
                  SelectObject(hDC, hPen);
                  DeleteObject(hPenShadow);
                  }
               else   // no shadows at all
                  {
                  MoveToEx (hDC, x2, y2, NULL), LineTo (hDC, x2-i, y2);
                  }   
 			     */                
               MoveToEx (hDC, x2, y2, NULL), LineTo (hDC, x2-i, y2);
               i--, y1++, y2--;
               }
               SelectObject(hDC,savePen);   
               DeleteObject(hPen);
            
            // Erase the corner of the border with the shadow

            if ((((info->lStyle & FP_NCDROPSHADOWFOCUS) && fFocus) ||
                  (info->lStyle & FP_NCDROPSHADOW)))
               {
               x2 = r.right - 1;
               y2 = r.bottom - 1;
               i = info->bBorderSize;

               hPen = CreatePen (PS_SOLID, 1, FPCOLOR_SMART(info->idColorDropShadow, info->colorDropShadow));
               savePen = SelectObject (hDC, hPen);               

               while (i)
                  {
                  MoveToEx (hDC, x2, y2, NULL); LineTo (hDC, x2-i, y2);
                  i--, y2--;
                  }

               SelectObject(hDC, savePen);
               DeleteObject(hPen);
               }
           }

         SetBkColor   (hDC, saveBkColor);
         }
      fpShrinkRect (&r, info->bBorderSize, info->bBorderSize);
      }
   // end draw border
   
   // If there is a round border and dropshadow, erase the bottom right corner  
   if ((info->lStyle & (FP_NCDROPSHADOW | FP_NCDROPSHADOWFOCUS)) &&
       (redraw & FPNC_REDRAW_DROPSHADOW) &&
       (fFocus || (info->lStyle & FP_NCDROPSHADOW)) &&
       (info->lStyle & FP_NCROUND))
      {
      short x2 = rright-1;
      short y2 = rbottom-1;
      short i = info->bBorderSize;
      HPEN  hPen = CreatePen (PS_SOLID, 1, pColor);
      HPEN  savePen = SelectObject (hDC, hPen);

      while (i)
         {
         MoveToEx (hDC, x2, y2, NULL); LineTo (hDC, x2-i, y2);
         i--, y2--;
         }
      SelectObject(hDC, savePen);
      DeleteObject(hPen);
      }
        // draw 3d effect
   if (info->lStyle & FP_NC3DOUTMASK)
      {
      LONG  cShadow, cHiLight, cInShadow, cInHiLight;
      BOOL  fInvert = ((info->lStyle & FP_NC3DINVERT) && fFocus);
      COLORREF saveBkColor = GetBkColor (hDC);
      short dSize;
         
      cHiLight   = FPCOLOR_SMART(info->idClr3DOutHiLite, info->color3DOutHiLite);
      cShadow    = FPCOLOR_SMART(info->idClr3DOutShadow, info->color3DOutShadow);
      cInHiLight = FPCOLOR_SMART(info->idClr3DInHiLite, info->color3DInHiLite);
      cInShadow  = FPCOLOR_SMART(info->idClr3DInShadow, info->color3DInShadow);
      // paint outer bevel
      dSize = info->bOutShadowWidth;
      if (dSize && NC_GET3DOUTSIDESTYLE((*info)))
         {
         if (redraw & FPNC_REDRAW_3D)
            {
            switch (NC_GET3DOUTSIDESTYLE((*info)))
               {
               case FP_NC3DOUTRAISE:
                  fpDrawShadows (hDC, &r, dSize, cShadow, cHiLight, fInvert, FALSE);
                  break;
               case FP_NC3DOUTLOWER:
                  fpDrawShadows (hDC, &r, dSize, cShadow, cHiLight, !fInvert, FALSE);
                  break;
               case FP_NC3DOUTCHISEL:
                  fpDrawShadows (hDC, &r, dSize, cShadow, cHiLight, TRUE, FALSE);
                  break;
               }
            }
         fpShrinkRect (&r, dSize, dSize);
         }
         
      // paint 3D border
      if (info->bFrameWidth)
         {
         COLORREF clrFrame = FPCOLOR_SMART(info->idClr3DFrame, info->clr3DFrame);
         dSize = info->bFrameWidth;
         if (redraw & FPNC_REDRAW_3D)
            fpDrawShadows (hDC, &r, dSize, clrFrame, clrFrame, FALSE, FALSE);
         fpShrinkRect (&r, dSize, dSize);
         }

      // paint inner bevel
      dSize = info->bInShadowWidth;
      if (dSize && NC_GET3DINSIDESTYLE((*info)))
         {
         if (redraw & FPNC_REDRAW_3D)
            {
            switch (NC_GET3DINSIDESTYLE((*info)))
               {
               case FP_NC3DINRAISE:
                  fpDrawShadows (hDC, &r, dSize, cInShadow, cInHiLight, fInvert, FALSE);
                  break;
               case FP_NC3DINLOWER:
                  fpDrawShadows (hDC, &r, dSize, cInShadow, cInHiLight, !fInvert, FALSE);
                  break;
               case FP_NC3DINCHISEL:
                  fpDrawShadows (hDC, &r, dSize, cInShadow, cInHiLight, TRUE, FALSE);
                  break;
               }
            fpShrinkRect (&r, dSize, dSize);
            }
         }
      SetBkColor   (hDC, saveBkColor);
      }                       
   // end draw 3d effect                 
   
   // draw buttons
   {
   BOOL  fSpin = ((info->lStyle & FP_NCSPIN) == FP_NCSPIN);
   short dButtonWidth = info->dButtonWidth;    
   BOOL f1 = !(info->lStyle & FP_NCBUTTONHIDE);
   BOOL f2 = fFocus;
   if (f1 || f2)
      {
      short btnLeft  = FP_LBUTTON(info);
      short btnRight = FP_RBUTTON(info); 
      RECT  buttonRect;
//#ifdef BTNALIGN
      if (info->fButtonLeft && (info->lStyle & FP_NCSLIDE) != FP_NCSLIDE)
      {
      btnRight = FP_LBUTTON(info); 
      btnLeft  = FP_RBUTTON(info) & ~FPB_BORDERLEFT;
      }
//#endif      
      if (btnLeft && (redraw & FPNC_REDRAW_BTNLEFT))
         {
		 COLORREF clrButton = FPCOLOR_SMART(info->idClrButton, info->clrButton);
   	     fpButtonRect  (&buttonRect, hWnd, TRUE, fSpin, dButtonWidth);
         if (info->fButtonLeft && (info->lStyle & FP_NCSLIDE) != FP_NCSLIDE)
         {
            if (info->dButtonState == 1 && IsWindowEnabled(hWnd))
			   fpButtonPaint (hDC, buttonRect, 18 | FPB_BORDERRIGHT, 2, clrButton);               
            else
			   fpButtonPaint (hDC, buttonRect, (short)(btnLeft | FPB_BORDERRIGHT), 
                 (IsWindowEnabled(hWnd) && !(info->lStyle & FP_NCBUTTONDISABLED)), clrButton);
         }
         else
         {
            if (info->dButtonState == 1 && IsWindowEnabled(hWnd))
			   fpButtonPaint (hDC, buttonRect, 18, 2, clrButton);               
            else
			   fpButtonPaint (hDC, buttonRect, btnLeft, 
                 (IsWindowEnabled(hWnd) && !(info->lStyle & FP_NCBUTTONDISABLED)), clrButton);
         }
         }

      if (btnRight && (redraw & FPNC_REDRAW_BTNRIGHT))
         {
		 COLORREF clrButton = FPCOLOR_SMART(info->idClrButton, info->clrButton);
         fpButtonRect  (&buttonRect, hWnd, FALSE, fSpin, dButtonWidth);
         if (info->dButtonState == 1 && IsWindowEnabled(hWnd))
			 fpButtonPaint (hDC, buttonRect, 18, 2, clrButton);               
         else
			 fpButtonPaint (hDC, buttonRect, btnRight, 
               (IsWindowEnabled(hWnd) && !(info->lStyle & FP_NCBUTTONDISABLED)), clrButton);
         }
         
      }
   

   if (fSpin)
   {
	  RECT spinRect;
	  RECT r;
	  POINT tmpPt,pt,testPt;
	  short bWidth,xOffset,yOffset,winHeight;
      LONG fHitTest;

      iSwap = GetSystemMetrics(SM_SWAPBUTTON);
      if (iSwap)
	  {
	   if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
          fMouseDown = FALSE;  
	   else
          fMouseDown = TRUE; 
	  }
	  else
      {
	   if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
          fMouseDown = FALSE;  
	   else
          fMouseDown = TRUE;  
	  }

	  GetClientRect (hWnd, &spinRect);
      GetCursorPos  (&tmpPt);
	  testPt = tmpPt;
      bWidth = fpButtonWidth (hWnd, dButtonWidth, fSpin, (short)fpWIN_HEIGHT(spinRect));

      if (info->fButtonLeft)
          pt.x = spinRect.left, pt.y = spinRect.top; 
      else
          pt.x = spinRect.right, pt.y = spinRect.top;
      
      ClientToScreen (hWnd, &pt);
      winHeight = fpWIN_HEIGHT(spinRect);
   
      pt.x += (info->fButtonLeft ? -bWidth : 0);

      r.left   = 0; 
      r.top    = 0;
      r.right  = bWidth;      
      r.bottom = winHeight-1;
   
      tmpPt.x -= pt.x;
      tmpPt.y -= pt.y;
   
      
      GetWindowRect (hWnd, &spinRect);
      xOffset = (short)(pt.x - spinRect.left);
      yOffset = (short)(pt.y - spinRect.top);
      
      if (tmpPt.y <= r.bottom >> 1)
         r.bottom >>= 1;
      else
         r.top = (r.bottom >> 1) + 1;
      info->fButtonLeft ? r.right-- : r.left++ ;
      
      r.left   += xOffset;
      r.top    += yOffset;
      r.right  += xOffset;
      r.bottom += yOffset;

      fHitTest = fpNCHitTest (hWnd, 1, info, &testPt, FALSE);
	  
	  if (fMouseDown && GetFocus() == hWnd && fHitTest == HTBORDER && !(info->lStyle & FP_NCBUTTONDISABLED) )
	     InvertRect (hDC, &r);
   }

   }

   SetBkMode  (hDC, oldMode);   

   
   
   // CTF to get NC Paint to go to printer  
   if (!hDCInput)
      ReleaseDC (hWnd, hDC);
}
                                                             
void FPLIB fpForceNCUpdate (HWND hWnd, LPFPNCINFO lpNCInfo, short redraw)
{
   lpNCInfo->lStyle |= FP_NCUPDATE;
   if (redraw == FPNC_REDRAW_REFRESH)
      SetWindowPos (hWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | 
         SWP_NOMOVE | SWP_DRAWFRAME | SWP_NOZORDER);
   else
      fpWMNCPaint (hWnd, lpNCInfo, redraw, 0);
   lpNCInfo->lStyle &= ~FP_NCUPDATE;

   return;
}

void FPLIB fpSetButtonStyle (HWND hWnd, HANDLE hInst, LONG lStyle, 
   LPFPNCINFO info)
{
   info->lStyle = (info->lStyle & ~FP_NCBUTTON) | lStyle;
   return;
};

void FPLIB fpNCInit(LPFPNCINFO lpzNCInfo, BOOL fSet)
{
   _fmemset (lpzNCInfo, 0, sizeof (FPNCINFO));
   
   // Init NonClient Info
   //
   if (fSet)
      {
      lpzNCInfo->lStyle           = FP_NCBORDER | FP_NCBUTTONWRAP
                                  | FP_NCBUTTONDEFAULTACTION;
      lpzNCInfo->dButtonMax       = 100;
      lpzNCInfo->dButtonIncrement = 1;
      lpzNCInfo->dButtonWidth     = 0;
	  lpzNCInfo->dButtonState     = 0;
      }
   // Outer 3D shadow
   lpzNCInfo->bOutShadowWidth   = 1;
   lpzNCInfo->color3DOutShadow = 0x80000000 | COLOR_BTNSHADOW; 
   lpzNCInfo->idClr3DOutShadow	= fpCM_AddItem(COLOR_INIT(COLOR_BTNSHADOW));
   lpzNCInfo->color3DOutHiLite = RGBCOLOR_WHITE;
#ifdef WIN32   //KAM
   lpzNCInfo->idClr3DOutHiLite	= fpCM_AddItem(COLOR_INIT(COLOR_BTNHIGHLIGHT));
#else
   lpzNCInfo->idClr3DOutHiLite	= fpCM_AddItem(RGBCOLOR_WHITE);
#endif
   // Inner 3D shadow
   lpzNCInfo->bInShadowWidth    = 1;
   lpzNCInfo->color3DInShadow  = 0x80000000 | COLOR_WINDOWFRAME;
   lpzNCInfo->idClr3DInShadow	= fpCM_AddItem(COLOR_INIT(COLOR_WINDOWFRAME));

   lpzNCInfo->idClr3DInHiLite	= fpCM_AddItem(COLOR_INIT(COLOR_BTNFACE));
   lpzNCInfo->color3DInHiLite  = 0x80000000 | COLOR_BTNFACE;
   // Border
   lpzNCInfo->colorBorder      = 0x80000000 | COLOR_WINDOWFRAME;          
   lpzNCInfo->idColorBorder		= fpCM_AddItem(COLOR_INIT(COLOR_WINDOWFRAME));
   lpzNCInfo->bBorderSize      = 1;
//#ifdef FP_VB
   lpzNCInfo->colorBk          = RGBCOLOR_DEFAULT;
   lpzNCInfo->idColorBk			= fpCM_AddItem(RGBCOLOR_DEFAULT);
//#else
//   lpzNCInfo->colorBk          = 0x80000000 | COLOR_WINDOW;
   //lpzNCInfo->idColorBk        = fpCM_AddItem(COLOR_INIT(COLOR_WINDOW));
 //#endif

   // Drop Shadow
   lpzNCInfo->colorDropShadow  = 0x80000000 | COLOR_BTNSHADOW;
   lpzNCInfo->idColorDropShadow	= fpCM_AddItem(COLOR_INIT(COLOR_BTNSHADOW));

   lpzNCInfo->bDropShadowWidth  = 3;

   lpzNCInfo->clr3DFrame       = 0x80000000 | COLOR_BTNFACE;                
   lpzNCInfo->idClr3DFrame		= fpCM_AddItem(COLOR_INIT(COLOR_BTNFACE));

   lpzNCInfo->clrButton       = 0x80000000 | COLOR_BTNFACE;                
   lpzNCInfo->idClrButton		= fpCM_AddItem(COLOR_INIT(COLOR_BTNFACE));  
//#ifdef BTNALIGN
   lpzNCInfo->fButtonLeft = FALSE;
//#endif
//   lpzNCInfo->fSuspend = FALSE;
   return;
}      

LONG FPLIB fpNCLeftButtonDown (HWND hWnd, LPFPNCINFO lpNCInfo, POINT point)
{
   short direction = 0;
   LONG  dwTime = 0L;
   short redraw = 0;
   short style;
   HDC   hDC;
   RECT  r;                       
   BOOL  bPopupDelay = FALSE;                                       //A885
	BOOL  bHwndValid;																  //A849

   short dIndex = lpNCInfo->dButtonIndex;
   LONG  lStyle = lpNCInfo->lStyle;
   BOOL  fWrap  = (lStyle & FP_NCBUTTONWRAP ? TRUE : FALSE);
   short dMin   = lpNCInfo->dButtonMin;
   short dMax   = lpNCInfo->dButtonMax;
   BOOL  fSpin  = ((lpNCInfo->lStyle & FP_NCSPIN) == FP_NCSPIN);
   short dButtonWidth = lpNCInfo->dButtonWidth;
#if defined(_WIN64) || defined(_IA64)
   LONG_PTR  rCode;
#else
   LONG  rCode;
#endif
               
   BOOL f1 = !IsWindowEnabled(hWnd);
   BOOL f2 = (lpNCInfo->lStyle & FP_NCBUTTONDISABLED ? TRUE : FALSE);

   if (f1 || f2)
      return 0;
      
   style = FP_RBUTTON (lpNCInfo);
   
   if (lpNCInfo->fButtonLeft && (lpNCInfo->lStyle & FP_NCSLIDE) != FP_NCSLIDE)
      {
      style  = style | FPB_BORDERRIGHT;
      }
//#ifdef BTNALIGN
   if (style && lpNCInfo->fButtonLeft && (lpNCInfo->lStyle & FP_NCSLIDE) != FP_NCSLIDE)
      direction = fpHitTest (hWnd, &point, style, TRUE, fSpin, dButtonWidth, &r);
   else 
//#else
//   if (style)
//#endif     
      direction = fpHitTest (hWnd, &point, style, FALSE, fSpin, dButtonWidth, &r);
   
   if (!direction)
      {
      style = FP_LBUTTON (lpNCInfo);
      if (style)
         direction = fpHitTest (hWnd, &point, style, TRUE, fSpin, dButtonWidth, &r);
      }

      
   // Invert Rectangle
   if (point.x < 0 && point.y < 0)
      ;
   else
      {
	  COLORREF clrButton = FPCOLOR_SMART(lpNCInfo->idClrButton, lpNCInfo->clrButton);
      hDC = GetWindowDC (hWnd);
      fpButtonDown (hDC, &r, style, clrButton);
      ReleaseDC (hWnd, hDC);
      }
   
	  SetCapture(hWnd);

   dwTime = GetTickCount();


   while (dwTime + (DWORD)FPB_INITIALDELAY > GetTickCount())
      ;

 //  if (fSpin)
//	   lpNCInfo->fSuspend = TRUE;
   for (;;)
      {
      if (dwTime + (DWORD)FPB_TIMEDELAY <= GetTickCount())
         {
            switch (direction)
               { // Loyal Changes Begin
               case FPB_POPUP:
                 bPopupDelay = TRUE;
                 // intentionally drop into following case
               case FPB_RIGHT:               
               case FPB_UP:  
                  dIndex += lpNCInfo->dButtonIncrement;
                  if (dIndex > dMax)
                     {
                     if (fWrap)
                        dIndex = dMin;
                     else
                        {
                        if (lpNCInfo->dButtonIndex == dMax)
                            if (!(lpNCInfo->lStyle & FP_NCBUTTONDEFAULTACTION))
                               fpNotifyParent (hWnd, FPN_BUTTONMAX, FALSE, &bHwndValid);  //A849
                        dIndex = dMax;
                        }
                     }
//#ifdef BTNALIGN
				  if (lpNCInfo->fButtonLeft && (lpNCInfo->lStyle & FP_NCSLIDE) != FP_NCSLIDE)
				     redraw = FPNC_REDRAW_BTNLEFT;
				  else                             
//#endif				  
                     redraw = FPNC_REDRAW_BTNRIGHT;                     
                  break;
               case FPB_DROPDOWN:    
                 bPopupDelay = TRUE;
                 // intentionally drop into following case
               case FPB_LEFT:
               case FPB_DOWN:
                  dIndex -= lpNCInfo->dButtonIncrement;
                  if (dIndex < dMin)
                     {
                     if (fWrap)
                        dIndex = dMax;
                     else
                        {
                        if (lpNCInfo->dButtonIndex == dMin)
                           if (!(lpNCInfo->lStyle & FP_NCBUTTONDEFAULTACTION))
                              fpNotifyParent (hWnd, FPN_BUTTONMIN, FALSE, &bHwndValid);		//A849
                        dIndex = dMin;
                        }
                     }
//#ifdef BTNALIGN
                  if (direction == FPB_LEFT || lpNCInfo->fButtonLeft)
//#else
//                  if (direction == FPB_LEFT)
//#endif                  
                     redraw = FPNC_REDRAW_BTNLEFT;
                   else
                     redraw = FPNC_REDRAW_BTNRIGHT;
                  break;
               }  // Loyal Changes End
            if ((direction != FPB_POPUP) && (direction != FPB_DROPDOWN))
               {
               rCode = fpSendMessageToParent(hWnd, FPM_BUTTONHIT, 
                  direction, (LPARAM)(short FAR *)&dIndex);
               if (!IsWindow(hWnd))
                  return 0;
               if (rCode)
                  lpNCInfo->dButtonIndex = dIndex;
               }
            dwTime = GetTickCount();
         }
    
         if (GetSystemMetrics(SM_SWAPBUTTON))
            {
            if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
               break;
            }
         else
            if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
               break;
      
   }

//   if (fSpin)
//	   lpNCInfo->fSuspend = FALSE;

   ReleaseCapture();
   
   if (bPopupDelay)
      {
      rCode = fpSendMessageToParent(hWnd, FPM_BUTTONHIT, 
                 direction, (LPARAM)(short FAR *)&dIndex);
      if (!IsWindow(hWnd))
         return 0;
      if (rCode)
         lpNCInfo->dButtonIndex = dIndex;
      }
    fpForceNCUpdate (hWnd, lpNCInfo, redraw);

   return 0;
}

LONG FPLIB fpNCHitTest (HWND hWnd, LONG code, LPFPNCINFO lpNCInfo, LPPOINT lpPoint, BOOL fStatic)
{
//   if (fStatic)                                                                                             //A259       removed to generate
//      code = HTMENU;                                                                               //A259      mousemove events
//   else                                                                                                     //A259       in VB static controls
      {
      BOOL  fSpin = ((lpNCInfo->lStyle & FP_NCSPIN) == FP_NCSPIN);
      short direction = 0;
      RECT  r;
      short style = FP_RBUTTON (lpNCInfo);
      
      if (style)
         direction = fpHitTest (hWnd, lpPoint, style, FALSE, fSpin, lpNCInfo->dButtonWidth, &r);
      
      if (!direction)
         {
//#ifdef BTNALIGN
        if (lpNCInfo->fButtonLeft && (lpNCInfo->lStyle & FP_NCSLIDE) != FP_NCSLIDE)
             style = FP_RBUTTON(lpNCInfo);
		else
//#endif		 
             style = FP_LBUTTON (lpNCInfo);
         if (style)
            direction = fpHitTest (hWnd, lpPoint, style, TRUE, fSpin, lpNCInfo->dButtonWidth, &r);
         }
      
      if (direction)
        code = HTBORDER;
      }
      
   return code;
}
