/*    PortTool v2.2     FPTOOL40.C          */

//
// FarPoint DrawText
//

#include "fptools.h"
#include "string.h"
#include <tchar.h>

#define DIVUP(Var1, Var2)     (((Var1) + (Var2) - 1) / (Var2))

// internal macros to prevent mistakes - note that local variables are used
//
#define dtVTOP       (!(fuFormat & (DT_BOTTOM | DT_VCENTER | DT_VJUSTIFY)))
#define dtVBOTTOM    (fuFormat & DT_BOTTOM)
#define dtVCENTER    (fuFormat & DT_VCENTER)
#define dtVJUSTIFY   (fuFormat & DT_VJUSTIFY)

#define dtHLEFT      (!(fuFormat & (DT_RIGHT | DT_CENTER | DT_JUSTIFY)))
#define dtHCENTER    (fuFormat & DT_CENTER)
#define dtHRIGHT     (fuFormat & DT_RIGHT)
#define dtHJUSTIFY   (fuFormat & DT_JUSTIFY)

#define dtEXTERNALLEADING (fuFormat & DT_EXTERNALLEADING)
#define dtEXPANDTABS      (fuFormat & DT_EXPANDTABS)
#define dtMNEMONIC        (!(fuFormat & DT_NOPREFIX))
#define dtWORDWRAP        (fuFormat & DT_WORDBREAK)
#define dtSINGLELINE      (fuFormat & DT_SINGLELINE)
#define dtCLIP            (!(fuFormat & DT_NOCLIP))
#define dt3D              (fuExtFormat & DTX_3D)
#define dtSAVEDATA        (fuExtFormat & DTX_SAVEDATA)
#define dtFOCUSRECT       (fuExtFormat & DTX_FOCUSRECT)
#define dtGRAYTEXT        (fuExtFormat & DTX_GRAYTEXT)
#define dtCALCRECT        (fuFormat & DT_CALCRECT)
#define dtESCAPEMENT      (fuExtFormat & (DTX_ROTATE | DTX_FLIP))
#define dtROTATE          (fuExtFormat & DTX_ROTATE)
#define dtFLIP            (fuExtFormat & DTX_FLIP)
#define dtTRANSPARENT     (fuExtFormat & DTX_TRANSPARENT)
#define dtVERTICAL_RTL    (fuExtFormat & DTX_VERTICAL_RTL)
#define dtVERTICAL_LTR    (fuExtFormat & DTX_VERTICAL_LTR)
#define dtVERTICAL        (dtVERTICAL_RTL || dtVERTICAL_LTR)

int FPLIB fpDrawText (HWND hWnd, HDC hDC, LPTSTR lpsz, int cbString, LPRECT lprc, UINT fuFormat, LPFPDRAWTEXT lpdt)
{
   UINT  fuSpace, cb, bc, fuHeight, fuLines;
   long  fuWidth;
   long  w;
   HFONT hFontOld = NULL, hFont = NULL;
   UINT  nMnemonic;
   int   fuLineHeight;
   HGLOBAL hLineTable = 0;
   LPFPLINEDATA lpLineTable = NULL;
   int   fuExtFormat = 0;
   UINT  fuOldBkMode = 0;
   UINT  fuOldFormat = fuFormat;
   short nColInc;
   TEXTMETRIC tm;
   FPFONT fpFont = NOFONT;
   RECT rc;
   short     nVertRight;
   short     nVertLeft;
// enhancement SEL8276
   BOOL	fDrawEllipses = FALSE;
// enhancement SEL8276
      
   if (!lprc)
      return DTERR_NORECT;

   if (dtCALCRECT)
      {
      if (lprc->right - lprc->left <= 0)
         lprc->right = 32000;

      if (lprc->bottom - lprc->top <= 0)
         lprc->bottom = 32000;
      }

   else if (lprc->left >= lprc->right || lprc->top >= lprc->bottom)
      return (0);

   CopyRect (&rc, lprc);

   if (cbString == -1)
      cbString = lstrlen (lpsz);
      
   if (lpdt)
      {
      fuSpace = lpdt->fuLineSpacing;
      fuExtFormat = (lpdt->fuStyle);
      lpdt->bRetIsTextTooLong = FALSE;
      }
   else
      fuSpace = 0;

   if (dtVERTICAL)
      fuFormat &= ~(DT_WORDBREAK | DT_SINGLELINE);

   SaveDC (hDC);

   if (dtTRANSPARENT)
      fuOldBkMode = SetBkMode(hDC, TRANSPARENT);

   /*************************
   * Create clipping region
   *************************/

   if (dtESCAPEMENT)
      {
      if (GetDeviceCaps(hDC, CLIPCAPS) == CP_RECTANGLE &&
          GetDeviceCaps(hDC, TECHNOLOGY) == DT_RASPRINTER)
         {
         HRGN  hRgn;
         POINT pt;

         GetViewportOrgEx(hDC, &pt);
         hRgn = CreateRectRgn(lprc->left + pt.x, lprc->top + pt.y,
                              lprc->right + pt.x, lprc->bottom + pt.y);
         SelectClipRgn(hDC, hRgn);
         DeleteObject(hRgn);
         }
      else
         IntersectClipRect(hDC, lprc->left, lprc->top, lprc->right, lprc->bottom);
      }

   if (dtESCAPEMENT)
      {
      int lfEscapement = 0;

      hFontOld = SelectObject(hDC, GetStockObject(SYSTEM_FONT));
      hFont = hFontOld;
      
      if (!dtTRANSPARENT)
         fuOldBkMode = SetBkMode(hDC, TRANSPARENT);

      if (dtFLIP)
         lfEscapement += 1800;
      if (dtROTATE)
         lfEscapement += 900;

      fpFont = fpRotateFont(hDC, hFont, lfEscapement);
      hFont = fpLockFont(fpFont, NULL);
      SelectObject(hDC, hFont);
		lpdt->lpCharWidths = 0;
      }
/*
   else
   {
      hFontOld = SelectObject(hDC, GetStockObject(SYSTEM_FONT));
      hFont = hFontOld;
      if (!dtTRANSPARENT)
         fuOldBkMode = SetBkMode(hDC, TRANSPARENT);
      fpFont = fpRotateFont(hDC, hFont, 0);
      hFont = fpLockFont(fpFont, NULL);
      SelectObject(hDC, hFont);
//		lpdt->lpCharWidths = 0;
   }
*/
/*
   else
      {
      LOGFONT LogFont;

      GetObject(hFont, sizeof(LOGFONT), &LogFont);
      fpFont = fp_GetLogFontPrim(&LogFont, hDC, &hFont);
      }
*/

   if (dtROTATE)
      {
      rc.left   = lprc->top;
      rc.top    = lprc->left;
      rc.right  = lprc->bottom;
      rc.bottom = lprc->right;
      }

   fuHeight = rc.bottom - rc.top;
   fuWidth = rc.right - rc.left;

   // if GetTextMetrics failed, abort function
   if (!GetTextMetrics (hDC, &tm))
   {
     if (fuOldBkMode)
       SetBkMode (hDC, fuOldBkMode);
     if (fpFont != NOFONT)
        {
        SelectObject (hDC, hFontOld);
        fpReleaseFont(fpFont);
        }

     RestoreDC(hDC, -1);
     return -1;
   }

   if( tm.tmBreakChar == 13 ) // 25935 -scl (if tmBreakChar is CR, replace with space)
      tm.tmBreakChar = 32;

   fuLineHeight = (tm.tmHeight + fuSpace);
   if (dtEXTERNALLEADING)
      {
      fuLineHeight += tm.tmExternalLeading;
      lprc->top += tm.tmExternalLeading;
      }

   if (dtSINGLELINE)
      {
      FPWORDWRAP ww;

      fuLines = 1;

      ww.fuMnemonic  = (dtMNEMONIC ? (UINT)-1 : 0);
      ww.tmBreakChar = tm.tmBreakChar;                              
      ww.cb          = cbString;
      ww.pageWidth   = fuWidth;
      ww.lpszFirst   = lpsz;
      ww.fpFont      = fpFont;
      ww.lpCharWidths = (lpdt ? lpdt->lpCharWidths : NULL); // SCP 12/31/98
		ww.dwFontLanguageInfo = (lpdt ? lpdt->dwFontLanguageInfo : 0);

// enhancement SEL8276
	  ww.fDrawEllipses = (lpdt ? lpdt->fuStyle & DTX_ELLIPSES : FALSE);
// enhancement SEL8276
      
      fpProcessLine (hDC, &ww, FALSE, fuFormat, &tm);

// enhancement SEL8276
	  fDrawEllipses = ww.fDrawEllipses;
// enhancement SEL8276

      lpsz = ww.lpszFirst;
      cb   = ww.dLength;
      bc   = ww.dBreakCount;
      w    = ww.dWidth;

      if (lpdt)
         {
         if (ww.fuMnemonic != (UINT)-1)
            lpdt->cMnemonic = ww.fuMnemonic;

         lpdt->bRetIsTextTooLong = (ww.dWidth > fuWidth);
         }
      }
   else
      {
      if (dtVERTICAL)
         {
         LPTSTR p = lpsz;
         fuLines = 0;

         for (fuLines = 0; *p; p = CharNext(p))
            fuLines++;
         }
      else
         {
         if (dtCALCRECT)
            {
            LPTSTR p = lpsz;
            int nLen;

            fuLines = 0;

            for (nLen = 0; *p; p = CharNext(p))
               {
               if (*p == (TCHAR)'\r' || *p == (TCHAR)'\n')
                  {
                  fuLines += (int)(((long)nLen * (long)tm.tmMaxCharWidth) /
                                    (long)fuWidth) + 1;
                  nLen = 0;

                  if (*(p + 1) == (TCHAR)'\r' || *(p + 1) == (TCHAR)'\n')
                     p++;
                  }
               else
                  nLen++;
               }

            if (nLen)
					/* RFW - 8/27/03
               fuLines += (int)(((long)nLen * (long)tm.tmMaxCharWidth) /
                                 (long)fuWidth) + 1;
					*/
					/* RFW - 5/23/07
					   This is a definite mistake and was calculating fuLines to be huge
               fuLines += (int)((long)nLen * max(1, ((long)fuWidth /
                                (long)tm.tmMaxCharWidth))) + 1;
					*/
               fuLines += (int)((double)nLen * ((double)(tm.tmMaxCharWidth + 1.0) / (double)fuWidth)) + 1;

            fuLines++;
            }
         else
            fuLines = fuHeight / (tm.tmHeight+fuSpace) + 1;
         }

      hLineTable = GlobalAlloc(GHND, sizeof(FPLINEDATA) * fuLines);
      if (!hLineTable)
         return DTERR_MEMOUT;
      else
         {
         LPTSTR     p = lpsz;
         FPWORDWRAP ww;
         UINT       i = 0;

         lpLineTable = GlobalLock(hLineTable);
      
         ww.fuMnemonic  = (dtMNEMONIC ? (UINT)-1 : 0);
         ww.tmBreakChar = tm.tmBreakChar;                              
         ww.pageWidth   = fuWidth;
         ww.cb          = cbString;
         ww.fpFont      = fpFont;
         ww.lpCharWidths = (lpdt ? lpdt->lpCharWidths : NULL); // SCP 12/31/98
			ww.dwFontLanguageInfo = (lpdt ? lpdt->dwFontLanguageInfo : 0);
// enhancement SEL8276
	     ww.fDrawEllipses = FALSE;	// no ellipses if text is wrapped
// enhancement SEL8276

         nMnemonic = (UINT)-1;
         while (p && i < fuLines)
            {
            ww.lpszFirst = p;
            
            if (dtWORDWRAP)
               p = fpWordWrap (hDC, &ww, &tm);
            else
               p = fpProcessLine (hDC, &ww, dtVERTICAL, fuFormat, &tm);
               
            lpLineTable[i].lpsz = ww.lpszFirst;
            lpLineTable[i].len  = ww.dLength;
            lpLineTable[i].bc   = ww.dBreakCount;
            lpLineTable[i].w    = ww.dWidth;
            
            if (ww.fuMnemonic != (UINT)-1)
               {
               nMnemonic = i;
               lpdt->cMnemonic = ww.fuMnemonic;
               }
            i++;
            }

         fuLines = i;

         if ((p && *p) || ((fuLines * (tm.tmHeight + fuSpace)) > fuHeight))
            lpdt->bRetIsTextTooLong = TRUE;
         }
      }

   // Paint Text
   //
   {
   UINT hMax = fuLines*fuLineHeight;
   int  xLeft = 0, xxLeft = 0, yyTop = 0;
   UINT cTabs = 0, nTabOrigin = 0;
   long wMax = 0;
   int  FAR *lpnTabs = NULL;
   COLORREF colorText;
   int  x, y, xx, yy, yTop;
   short nCols = 0;
   UINT i;

   if (dtGRAYTEXT)
      {
      COLORREF bkColor = GetBkColor(hDC);
      COLORREF clrGray = GetSysColor(COLOR_GRAYTEXT);

      if (bkColor != clrGray)
         colorText = SetTextColor(hDC, clrGray);
      else if (bkColor == RGBCOLOR_PALEGRAY)
         colorText = SetTextColor(hDC, RGBCOLOR_DARKGRAY);
      else
         colorText = SetTextColor(hDC, RGBCOLOR_PALEGRAY);
      }

   if (dtVERTICAL)
      {
	   if (dtSINGLELINE)
			{
         fuLines = min(fuLines, fuHeight / fuLineHeight);
	      hMax = min(fuLines, fuHeight / fuLineHeight) * fuLineHeight;
			}
		else
			{
			UINT uiHeight = 0;

			hMax = 0;
			nCols = 1;
	      for (i = 0; i < fuLines; )
				{
				if (*(lpLineTable[i].lpsz) == '\r' || *(lpLineTable[i].lpsz) == '\n')
					{
					if (*(lpLineTable[i].lpsz) == '\r' && *(lpLineTable[i + 1].lpsz) == '\n')
						i++;
					i++;
					nCols++;
					uiHeight = 0;
					}
				else if (uiHeight > 0 && uiHeight + fuLineHeight > fuHeight)
					{
					nCols++;
					uiHeight = 0;
					}
				else
					{
					i++;
					uiHeight += fuLineHeight;
					hMax = max(hMax, uiHeight);
					}
				}
			}
      }

   // compute vertical alignment
   //                   
   y = lprc->top;            
   if (fuHeight < hMax)
      ;
   else if (dtVCENTER)
      y += (((short)(fuHeight - hMax)) / 2);
   else if (dtVBOTTOM)
      y += fuHeight - hMax;
   else if (dtVJUSTIFY)
      {
      fuLineHeight = fuHeight / fuLines; 
      y += ((short)(fuLineHeight - tm.tmHeight)) / 2;
      }

   if (dtSINGLELINE)
      {
      wMax = w;
      
      // compute horizontal alignment
      //
      if (dtHCENTER)
         x = (lprc->left + ((short)(fuWidth - w)) / 2);
      else if (dtHRIGHT)
         x = (lprc->left + (int)(fuWidth - w));
      else if (dtHJUSTIFY)
         {
         x = lprc->left;
         if (bc) // break count
            SetTextJustification(hDC, (int)(fuWidth - w), bc);
         }
      else
         x = lprc->left;
      
      if (dtROTATE)
         {
         if (dtFLIP)
            {
            xx = lprc->right  - (y - lprc->top);
            yy = lprc->top    + (x - lprc->left);
            }   
         else
            {
            xx = lprc->left   + (y - lprc->top);
            yy = lprc->bottom - (x - lprc->left);
            }
         }
      else if (dtFLIP)
         {
         xx = lprc->right  - (x - lprc->left);
         yy = lprc->bottom - (y - lprc->top);
         }
      else
         {
         xx = x;
         yy = y;
         }

      xxLeft = xx;
      yyTop  = yy;
      
      if (dtCALCRECT)
        ; // do not paint
      else if (dtEXPANDTABS)
	  {
         TabbedTextOut(hDC, xx, yy, lpsz, cb, cTabs, lpnTabs, nTabOrigin);
// enhancement SEL8276
		 if (fDrawEllipses)
			 TabbedTextOut(hDC, xx + w, yy, _T("..."), 3, cTabs, lpnTabs, nTabOrigin);
// enhancement SEL8276
	  }
      else
         {
         UINT fuOpt = (dtCLIP ? ETO_CLIPPED : 0) | (dtMNEMONIC ? ETO_MNEMONIC : 0) | (!dtTRANSPARENT ? ETO_OPAQUE : 0);
         fuOpt |= ETO_MNSHOW;
         fpExtTextOut (hDC, xx, yy, fuOpt, lprc, lpsz, cb,
            NULL, (dt3D ? &lpdt->z3DInfo : NULL), lpdt);
// enhancement SEL8276
		 if (fDrawEllipses)
			 fpExtTextOut(hDC, xx + w, yy, fuOpt, lprc, _T("..."), 3,
			 NULL, (dt3D ? &lpdt->z3DInfo : NULL), lpdt);
// enhancement SEL8276
         }
      if (dtHJUSTIFY)
         SetTextJustification(hDC, 0, 0);
      }
   else
      {
      if (dtROTATE)
         xLeft = lprc->left + fuHeight;
      else
         xLeft = lprc->left + (int)fuWidth;

      if (dtVERTICAL)
         {
         nColInc = (short)tm.tmMaxCharWidth;

         if (dtHRIGHT)
            x = lprc->right - 1 - nColInc;
         else if (nCols * nColInc >= fuWidth)
            {
            short nColsVis = max(1, (short)(fuWidth / nColInc));

            x = lprc->left + ((nColsVis - 1) * nColInc);
            }
         else if (dtHCENTER)
            x = lprc->right - ((short)(fuWidth -
                (nCols * nColInc)) / 2) - nColInc;
         else if (dtHJUSTIFY)          // Not Done
            x = lprc->left + ((nCols - 1) * nColInc);
         else
            x = lprc->left + ((nCols - 1) * nColInc);

         if (dtVERTICAL_LTR)
            x = max(lprc->left, x - ((nCols - 1) * nColInc));

         nVertLeft = x;
         nVertRight = x + nColInc;
         yTop = y;
         }

      for (i = 0; i < fuLines; i++)
         {
         wMax = (wMax > lpLineTable[i].w ? wMax : lpLineTable[i].w);

         // compute horizontal alignment
         //
         if (dtVERTICAL)
            ;
         else if (dtHCENTER)
            x = (lprc->left + ((short)(fuWidth - lpLineTable[i].w)) / 2);
         else if (dtHRIGHT)
            x = (lprc->left + (int)(fuWidth - lpLineTable[i].w));
         else if (dtHJUSTIFY)
            {
            x = lprc->left;
            if (lpLineTable[i].bc)
               SetTextJustification(hDC, (int)(fuWidth - lpLineTable[i].w),
                                    lpLineTable[i].bc);
            }
         else
            x = lprc->left;
  
         if (dtROTATE)
            {
            if (dtFLIP)
               {
               xx = lprc->right  - (y - lprc->top);
               yy = lprc->top    + (x - lprc->left);
               }   
            else
               {
               xx = lprc->left   + (y - lprc->top);
               yy = lprc->bottom - (x - lprc->left);
               }
            if (x < xLeft)
               xLeft = x, yyTop = yy;
            if (i == 0)
               {
               xxLeft = xx;
               yyTop = yy;
               }
            }
         else 
            {
            if (dtFLIP)
               {
               xx = lprc->right  - (x - lprc->left);
               yy = lprc->bottom - (y - lprc->top);
               }
            else
               {
               xx = x;
               yy = y;
               }
           if (x < xLeft)
              xLeft = x, xxLeft = xx;
           if (i == 0)
              {
              xxLeft = xx;
              yyTop = yy;
              }
            }

         if (dtVERTICAL && (*(lpLineTable[i].lpsz) == '\r' || *(lpLineTable[i].lpsz) == '\n'))
            ; // do not paint
         else if (dtCALCRECT)
            ; // do not paint
         else if (dtEXPANDTABS)
            TabbedTextOut(hDC, xx, yy, lpLineTable[i].lpsz, lpLineTable[i].len, cTabs, lpnTabs, nTabOrigin);
         else
            {
            UINT fuOpt = (dtCLIP ? ETO_CLIPPED : 0) | (dtMNEMONIC ? ETO_MNEMONIC : 0) | (!dtTRANSPARENT ? ETO_OPAQUE : 0);
// SCS7680            if (nMnemonic == i) 
               fuOpt |= ETO_MNSHOW;
            fpExtTextOut (hDC, xx, yy, fuOpt, lprc, lpLineTable[i].lpsz, lpLineTable[i].len,
               NULL, (dt3D ? &lpdt->z3DInfo : NULL), lpdt);
            }
         if (dtHJUSTIFY && lpLineTable[i].bc)
            SetTextJustification(hDC, 0, 0);

         y += fuLineHeight;

         if (dtVERTICAL && ((y + fuLineHeight > lprc->bottom && i < fuLines - 1) ||
             *(lpLineTable[i].lpsz) == '\r' || *(lpLineTable[i].lpsz) == '\n'))
            {
            y = yTop;

            if (dtVERTICAL_LTR)
               {
               x += nColInc;
               nVertRight += nColInc;
               }
            else
               {
               x -= nColInc;
               nVertLeft -= nColInc;
               }

            if (*(lpLineTable[i].lpsz) == '\r')
					{
					if (*(lpLineTable[i + 1].lpsz) == '\n')
						i++;
					}
            else if (*(lpLineTable[i].lpsz) == '\n')
					;
	         else if (*(lpLineTable[i + 1].lpsz) == '\r' || *(lpLineTable[i + 1].lpsz) == '\n')
					{
	            if (*(lpLineTable[i + 1].lpsz) == '\r' && i + 2 < fuLines && *(lpLineTable[i + 2].lpsz) == '\n')
						i++;
					i++;
					}
            }
         }         
      }
      
   if (dtGRAYTEXT)
      SetTextColor(hDC, colorText);

   if (fuOldBkMode)
      SetBkMode (hDC, fuOldBkMode);
   
   // cleanup font used for rotation
   //
   
   if (fpFont != NOFONT)
      {
      SelectObject (hDC, hFontOld);
      fpReleaseFont(fpFont);
      }

   RestoreDC(hDC, -1);
   
   if (dtFOCUSRECT || dtCALCRECT)
      {
      RECT rcTmp, rcBound;

      if (dtVERTICAL)
         {
         xxLeft = nVertLeft;
         wMax = nVertRight - nVertLeft;
         }

      if (dtROTATE)
         {
         if (dtFLIP)
            {
            rcTmp.left   = xxLeft - hMax;
            rcTmp.top    = yyTop;
            rcTmp.right  = xxLeft;
            rcTmp.bottom = yyTop + (int)wMax;
            }
         else
            {
            rcTmp.left   = xxLeft;
            rcTmp.top    = yyTop - (int)wMax;
            rcTmp.right  = xxLeft + hMax;
            rcTmp.bottom = yyTop;
            }
         }

      else if (dtFLIP)
         {
         rcTmp.left   = xxLeft - (int)wMax;
         rcTmp.top    = yyTop - hMax;
         rcTmp.right  = xxLeft;
         rcTmp.bottom = yyTop;
         }
      else
         {
         rcTmp.left   = xxLeft;
         rcTmp.top    = yyTop;
         rcTmp.right  = xxLeft + (int)wMax;
         rcTmp.bottom = yyTop + hMax;
         }

      if (dtVJUSTIFY)
         {
         rcTmp.top    = lprc->top;
         rcTmp.bottom = lprc->bottom;
         }

      if (dtHJUSTIFY)
         {
         rcTmp.left   = lprc->left;
         rcTmp.right  = lprc->right;
         }

      if (dtCALCRECT)
         {
//         IntersectRect (&rcBound, &rcTmp, lprc);
//         CopyRect (lprc, &rcBound);
         CopyRect (lprc, &rcTmp);
         }

      else if (dtFOCUSRECT)
         {
         IntersectRect(&rcBound, &rcTmp, lprc);
         InflateRect(&rcBound, 1, 1);
         fpDrawFocusRect (hDC, &rcBound);
         }
      }
   }

   if (hLineTable)
      {
      GlobalUnlock(hLineTable);
      GlobalFree(hLineTable);
      }

   return 0;
}

int FPLIB fpDrawTextEx (HINSTANCE hInstance, HWND hWnd, HDC hDC, LPTSTR lpsz, int cbString, LPRECT lprc, UINT fuFormat, LPFPDRAWTEXT lpdt)
{
   UINT  fuSpace, cb, bc, fuHeight, fuLines;
   long  fuWidth;
   long  w;
   HFONT hFontOld = NULL, hFont = NULL;
   UINT  nMnemonic;
   int   fuLineHeight;
   HGLOBAL hLineTable = 0;
   LPFPLINEDATA lpLineTable = NULL;
   int   fuExtFormat = 0;
   UINT  fuOldBkMode = 0;
   UINT  fuOldFormat = fuFormat;
   short nColInc;
   TEXTMETRIC tm;
   FPFONT fpFont = NOFONT;
   RECT rc;
//   HINSTANCE hInstance;
   short     nVertRight;
   short     nVertLeft;
// enhancement SEL8276
   BOOL	fDrawEllipses = FALSE;
// enhancement SEL8276
      
   if (!lprc)
      return DTERR_NORECT;

   if (dtCALCRECT)
      {
      if (lprc->right - lprc->left <= 0)
         lprc->right = 32000;

      if (lprc->bottom - lprc->top <= 0)
         lprc->bottom = 32000;
      }

   else if (lprc->left >= lprc->right || lprc->top >= lprc->bottom)
      return (0);

   CopyRect (&rc, lprc);

   if (cbString == -1)
      cbString = lstrlen (lpsz);
      
   if (lpdt)
      {
      fuSpace = lpdt->fuLineSpacing;
      fuExtFormat = (lpdt->fuStyle);
      lpdt->bRetIsTextTooLong = FALSE;
      }
   else
      fuSpace = 0;

   if (dtVERTICAL)
      fuFormat &= ~(DT_WORDBREAK | DT_SINGLELINE);

   SaveDC (hDC);

   if (dtTRANSPARENT)
      fuOldBkMode = SetBkMode(hDC, TRANSPARENT);

   /*************************
   * Create clipping region
   *************************/

   if (dtESCAPEMENT)
      {
      if (GetDeviceCaps(hDC, CLIPCAPS) == CP_RECTANGLE &&
          GetDeviceCaps(hDC, TECHNOLOGY) == DT_RASPRINTER)
         {
         HRGN  hRgn;
         POINT pt;

         GetViewportOrgEx(hDC, &pt);
         hRgn = CreateRectRgn(lprc->left + pt.x, lprc->top + pt.y,
                              lprc->right + pt.x, lprc->bottom + pt.y);
         SelectClipRgn(hDC, hRgn);
         DeleteObject(hRgn);
         }
      else
         IntersectClipRect(hDC, lprc->left, lprc->top, lprc->right, lprc->bottom);
      }

   if (dtESCAPEMENT)
      {
      int lfEscapement = 0;

      hFontOld = SelectObject(hDC, GetStockObject(SYSTEM_FONT));
      hFont = hFontOld;
      
      if (!dtTRANSPARENT)
         fuOldBkMode = SetBkMode(hDC, TRANSPARENT);

      if (dtFLIP)
         lfEscapement += 1800;
      if (dtROTATE)
         lfEscapement += 900;

// fix for JUS47 -scl
      fpFont = fpRotateFontEx(hInstance, hDC, hFont, lfEscapement);
      hFont = fpLockFontEx(hInstance, fpFont, NULL);
// fix for JUS47 -scl
      SelectObject(hDC, hFont);
      }
/*
   else
      {
      LOGFONT LogFont;

      GetObject(hFont, sizeof(LOGFONT), &LogFont);
      fpFont = fp_GetLogFontPrim(&LogFont, hDC, &hFont);
      }
*/

   if (dtROTATE)
      {
      rc.left   = lprc->top;
      rc.top    = lprc->left;
      rc.right  = lprc->bottom;
      rc.bottom = lprc->right;
      }

   fuHeight = rc.bottom - rc.top;
   fuWidth = rc.right - rc.left;

   // if GetTextMetrics failed, abort function
   if (!GetTextMetrics (hDC, &tm))
   {
     if (fuOldBkMode)
       SetBkMode (hDC, fuOldBkMode);
     if (fpFont != NOFONT)
        {
        SelectObject (hDC, hFontOld);
// fix for JUS47 -scl
        fpReleaseFontEx(hInstance, fpFont);
// fix for JUS47 -scl
        }

     RestoreDC(hDC, -1);
     return -1;
   }

   fuLineHeight = (tm.tmHeight + fuSpace);
   if (dtEXTERNALLEADING)
      {
      fuLineHeight += tm.tmExternalLeading;
      lprc->top += tm.tmExternalLeading;
      }

   if (dtSINGLELINE)
      {
      FPWORDWRAP ww;

      fuLines = 1;

      ww.fuMnemonic  = (dtMNEMONIC ? (UINT)-1 : 0);
      ww.tmBreakChar = tm.tmBreakChar;                              
      ww.cb          = cbString;
      ww.pageWidth   = fuWidth;
      ww.lpszFirst   = lpsz;
      ww.fpFont      = fpFont;
      ww.lpCharWidths = (lpdt ? lpdt->lpCharWidths : NULL); // SCP 12/31/98
		ww.dwFontLanguageInfo = (lpdt ? lpdt->dwFontLanguageInfo : 0);
// enhancement SEL8276
	  ww.fDrawEllipses = (lpdt ? lpdt->fuStyle & DTX_ELLIPSES : FALSE);
// enhancement SEL8276
      
      fpProcessLine (hDC, &ww, FALSE, fuFormat, &tm);

// enhancement SEL8276
      fDrawEllipses = ww.fDrawEllipses;
// enhancement SEL8276

      lpsz = ww.lpszFirst;
      cb   = ww.dLength;
      bc   = ww.dBreakCount;
      w    = ww.dWidth;

      if (lpdt)
         {
         if (ww.fuMnemonic != (UINT)-1)
            lpdt->cMnemonic = ww.fuMnemonic;

         lpdt->bRetIsTextTooLong = (ww.dWidth > fuWidth);
         }
      }
   else
      {
// removed 7/22/99 -scl
// there is already a hLineTable declared locally in this function.
// this one leaks memory.
//      HGLOBAL hLineTable;

      if (dtVERTICAL)
         {
         LPTSTR p = lpsz;
         fuLines = 0;

         for (fuLines = 0; *p; p = CharNext(p))
            fuLines++;
         }
      else
         {
         if (dtCALCRECT)
            {
            LPTSTR p = lpsz;
            short nLen;

            fuLines = 0;

            for (nLen = 0; *p; p = CharNext(p))
               {
               if (*p == (TCHAR)'\r' || *p == (TCHAR)'\n')
                  {
                  fuLines += (int)(((long)nLen * (long)tm.tmMaxCharWidth) /
                                    (long)fuWidth) + 1;
                  nLen = 0;

                  if (*(p + 1) == (TCHAR)'\r' || *(p + 1) == (TCHAR)'\n')
                     p++;
                  }
               else
                  nLen++;
               }

            if (nLen)
               fuLines += (int)(((long)nLen * (long)tm.tmMaxCharWidth) /
                                 (long)fuWidth) + 1;

            fuLines++;
            }
         else
            fuLines = fuHeight / (tm.tmHeight+fuSpace) + 1;
         }

      hLineTable = GlobalAlloc(GHND, sizeof(FPLINEDATA) * fuLines);
      if (!hLineTable)
         return DTERR_MEMOUT;
      else
         {
         LPTSTR      p = lpsz;
         FPWORDWRAP ww;
         UINT       i = 0;

         lpLineTable = GlobalLock(hLineTable);
      
         ww.fuMnemonic  = (dtMNEMONIC ? (UINT)-1 : 0);
         ww.tmBreakChar = tm.tmBreakChar;                              
         ww.pageWidth   = fuWidth;
         ww.cb          = cbString;
         ww.fpFont      = fpFont;
         ww.lpCharWidths = (lpdt ? lpdt->lpCharWidths : NULL); // SCP 12/31/98
			ww.dwFontLanguageInfo = (lpdt ? lpdt->dwFontLanguageInfo : 0);
// enhancement SEL8276
	     ww.fDrawEllipses = FALSE;	// no ellipses if text is wrapped
// enhancement SEL8276

         nMnemonic = (UINT)-1;
         while (p && i < fuLines)
            {
            ww.lpszFirst = p;
            
            if (dtWORDWRAP)
               p = fpWordWrap (hDC, &ww, &tm);
            else
               p = fpProcessLine (hDC, &ww, dtVERTICAL, fuFormat, &tm);
               
            lpLineTable[i].lpsz = ww.lpszFirst;
            lpLineTable[i].len  = ww.dLength;
            lpLineTable[i].bc   = ww.dBreakCount;
            lpLineTable[i].w    = ww.dWidth;
            
            if (ww.fuMnemonic != (UINT)-1)
               {
               nMnemonic = i;
               lpdt->cMnemonic = ww.fuMnemonic;
               }
            i++;
            }

         fuLines = i;

         if ((p && *p) || ((fuLines * (tm.tmHeight + fuSpace)) > fuHeight))
            lpdt->bRetIsTextTooLong = TRUE;
         }
      }
   
   // Paint Text
   //
   {
   UINT hMax = fuLines*fuLineHeight;
   int  xLeft = 0, xxLeft = 0, yyTop = 0;
   UINT cTabs = 0, nTabOrigin = 0;
   long wMax = 0;
   int  FAR *lpnTabs = NULL;
   COLORREF colorText;
   int  x, y, xx, yy, yTop;
   UINT i;
       
   if (dtGRAYTEXT)
      {
      COLORREF bkColor = GetBkColor(hDC);
      COLORREF clrGray = GetSysColor(COLOR_GRAYTEXT);

      if (bkColor != clrGray)
         colorText = SetTextColor(hDC, clrGray);
      else if (bkColor == RGBCOLOR_PALEGRAY)
         colorText = SetTextColor(hDC, RGBCOLOR_DARKGRAY);
      else
         colorText = SetTextColor(hDC, RGBCOLOR_PALEGRAY);
      }

   if (dtVERTICAL)
      {
      if (!(fuOldFormat & DT_WORDBREAK))
         fuLines = min(fuLines, fuHeight / fuLineHeight);

      hMax = min(fuLines, fuHeight / fuLineHeight) * fuLineHeight;
      }

   // compute vertical alignment
   //                   
   y = lprc->top;            
   if (fuHeight < hMax)
      ;
   else if (dtVCENTER)
      y += (((short)(fuHeight - hMax)) / 2);
   else if (dtVBOTTOM)
      y += fuHeight - hMax;
   else if (dtVJUSTIFY)
      {
      fuLineHeight = fuHeight / fuLines; 
      y += ((short)(fuLineHeight - tm.tmHeight)) / 2;
      }

   if (dtSINGLELINE)
      {
      wMax = w;
      
      // compute horizontal alignment
      //
      if (dtHCENTER)
         x = (lprc->left + ((short)(fuWidth - w)) / 2);
      else if (dtHRIGHT)
         x = (lprc->left + (int)(fuWidth - w));
      else if (dtHJUSTIFY)
         {
         x = lprc->left;
         if (bc) // break count
            SetTextJustification(hDC, (int)(fuWidth - w), bc);
         }
      else
         x = lprc->left;
      
      if (dtROTATE)
         {
         if (dtFLIP)
            {
            xx = lprc->right  - (y - lprc->top);
            yy = lprc->top    + (x - lprc->left);
            }   
         else
            {
            xx = lprc->left   + (y - lprc->top);
            yy = lprc->bottom - (x - lprc->left);
            }
         }
      else if (dtFLIP)
         {
         xx = lprc->right  - (x - lprc->left);
         yy = lprc->bottom - (y - lprc->top);
         }
      else
         {
         xx = x;
         yy = y;
         }

      xxLeft = xx;
      yyTop  = yy;
      
      if (dtCALCRECT)
        ; // do not paint
      else if (dtEXPANDTABS)
	  {
         TabbedTextOut(hDC, xx, yy, lpsz, cb, cTabs, lpnTabs, nTabOrigin);
// enhancement SEL8276
		 if (fDrawEllipses)
			 TabbedTextOut(hDC, xx + w, yy, _T("..."), 3, cTabs, lpnTabs, nTabOrigin);
// enhancement SEL8276
	  }
      else
         {
         UINT fuOpt = (dtCLIP ? ETO_CLIPPED : 0) | (dtMNEMONIC ? ETO_MNEMONIC : 0) | (!dtTRANSPARENT ? ETO_OPAQUE : 0);
         fuOpt |= ETO_MNSHOW;
// fix for JUS49 -scl
         fpExtTextOutEx (hInstance, hDC, xx, yy, fuOpt, lprc, lpsz, cb,
            NULL, (dt3D ? &lpdt->z3DInfo : NULL), lpdt);
// fix for JUS49 -scl
// enhancement SEL8276
		 if (fDrawEllipses)
			 fpExtTextOutEx(hInstance, hDC, xx + w, yy, fuOpt, lprc, _T("..."), 3,
			 NULL, (dt3D ? &lpdt->z3DInfo : NULL), lpdt);
// enhancement SEL8276
        }
      if (dtHJUSTIFY)
         SetTextJustification(hDC, 0, 0);
      }
   else
      {
      if (dtROTATE)
         xLeft = lprc->left + fuHeight;
      else
         xLeft = lprc->left + (int)fuWidth;

      if (dtVERTICAL)
         {
         short nCols = DIVUP(fuLines, max(1, fuHeight / fuLineHeight));

         nColInc = (short)tm.tmMaxCharWidth;

         if (dtHRIGHT)
            x = lprc->right - 1 - nColInc;
         else if (nCols * nColInc >= fuWidth)
            {
            short nColsVis = max(1, (short)(fuWidth / nColInc));

            x = lprc->left + ((nColsVis - 1) * nColInc);
            }
         else if (dtHCENTER)
            x = lprc->right - ((short)(fuWidth -
                (nCols * nColInc)) / 2) - nColInc;
         else if (dtHJUSTIFY)          // Not Done
            x = lprc->left + ((nCols - 1) * nColInc);
         else
            x = lprc->left + ((nCols - 1) * nColInc);

         if (dtVERTICAL_LTR)
            x = max(lprc->left, x - ((nCols - 1) * nColInc));

         nVertLeft = x;
         nVertRight = x + nColInc;
         yTop = y;
         }

      for (i = 0; i < fuLines; i++)
         {
         wMax = (wMax > lpLineTable[i].w ? wMax : lpLineTable[i].w);

         // compute horizontal alignment
         //
         if (dtVERTICAL)
            ;
         else if (dtHCENTER)
            x = (lprc->left + ((short)(fuWidth - lpLineTable[i].w)) / 2);
         else if (dtHRIGHT)
            x = (lprc->left + (int)(fuWidth - lpLineTable[i].w));
         else if (dtHJUSTIFY)
            {
            x = lprc->left;
            if (lpLineTable[i].bc)
               SetTextJustification(hDC, (int)(fuWidth - lpLineTable[i].w),
                                    lpLineTable[i].bc);
            }
         else
            x = lprc->left;
  
         if (dtROTATE)
            {
            if (dtFLIP)
               {
               xx = lprc->right  - (y - lprc->top);
               yy = lprc->top    + (x - lprc->left);
               }   
            else
               {
               xx = lprc->left   + (y - lprc->top);
               yy = lprc->bottom - (x - lprc->left);
               }
            if (x < xLeft)
               xLeft = x, yyTop = yy;
            if (i == 0)
               {
               xxLeft = xx;
               yyTop = yy;
               }
            }
         else 
            {
            if (dtFLIP)
               {
               xx = lprc->right  - (x - lprc->left);
               yy = lprc->bottom - (y - lprc->top);
               }
            else
               {
               xx = x;
               yy = y;
               }
           if (x < xLeft)
              xLeft = x, xxLeft = xx;
           if (i == 0)
              {
              xxLeft = xx;
              yyTop = yy;
              }
            }

         if (dtCALCRECT)
           ; // do not paint
         else if (dtEXPANDTABS)
            TabbedTextOut(hDC, xx, yy, lpLineTable[i].lpsz, lpLineTable[i].len, cTabs, lpnTabs, nTabOrigin);
         else
            {
            UINT fuOpt = (dtCLIP ? ETO_CLIPPED : 0) | (dtMNEMONIC ? ETO_MNEMONIC : 0) | (!dtTRANSPARENT ? ETO_OPAQUE : 0);
            if (nMnemonic == i) 
               fuOpt |= ETO_MNSHOW;
// fix for JUS49 -scl
            fpExtTextOutEx (hInstance, hDC, xx, yy, fuOpt, lprc, lpLineTable[i].lpsz, lpLineTable[i].len,
               NULL, (dt3D ? &lpdt->z3DInfo : NULL), lpdt);
 // fix for JUS49 -scl
           }
         if (dtHJUSTIFY && lpLineTable[i].bc)
            SetTextJustification(hDC, 0, 0);

         y += fuLineHeight;

         if (dtVERTICAL && y + fuLineHeight > lprc->bottom &&
             i < fuLines - 1)
            {
            y = yTop;

            if (dtVERTICAL_LTR)
               {
               x += nColInc;
               nVertRight += nColInc;
               }
            else
               {
               x -= nColInc;
               nVertLeft -= nColInc;
               }
            }
         }         
      }
      
   if (dtGRAYTEXT)
      SetTextColor(hDC, colorText);

   if (fuOldBkMode)
      SetBkMode (hDC, fuOldBkMode);
   
   // cleanup font used for rotation
   //
   
   if (fpFont != NOFONT)
      {
      SelectObject (hDC, hFontOld);
// fix for JUS47 -scl
      fpReleaseFontEx(hInstance, fpFont);
// fix for JUS47 -scl
      }

   RestoreDC(hDC, -1);
   
   if (dtFOCUSRECT || dtCALCRECT)
      {
      RECT rcTmp, rcBound;

      if (dtVERTICAL)
         {
         xxLeft = nVertLeft;
         wMax = nVertRight - nVertLeft;
         }

      if (dtROTATE)
         {
         if (dtFLIP)
            {
            rcTmp.left   = xxLeft - hMax;
            rcTmp.top    = yyTop;
            rcTmp.right  = xxLeft;
            rcTmp.bottom = yyTop + (int)wMax;
            }
         else
            {
            rcTmp.left   = xxLeft;
            rcTmp.top    = yyTop - (int)wMax;
            rcTmp.right  = xxLeft + hMax;
            rcTmp.bottom = yyTop;
            }
         }

      else if (dtFLIP)
         {
         rcTmp.left   = xxLeft - (int)wMax;
         rcTmp.top    = yyTop - hMax;
         rcTmp.right  = xxLeft;
         rcTmp.bottom = yyTop;
         }
      else
         {
         rcTmp.left   = xxLeft;
         rcTmp.top    = yyTop;
         rcTmp.right  = xxLeft + (int)wMax;
         rcTmp.bottom = yyTop + hMax;
         }

      if (dtVJUSTIFY)
         {
         rcTmp.top    = lprc->top;
         rcTmp.bottom = lprc->bottom;
         }

      if (dtHJUSTIFY)
         {
         rcTmp.left   = lprc->left;
         rcTmp.right  = lprc->right;
         }

      if (dtCALCRECT)
         {
//         IntersectRect (&rcBound, &rcTmp, lprc);
//         CopyRect (lprc, &rcBound);
         CopyRect (lprc, &rcTmp);
         }

      else if (dtFOCUSRECT)
         {
         IntersectRect(&rcBound, &rcTmp, lprc);
         InflateRect(&rcBound, 1, 1);
         fpDrawFocusRect (hDC, &rcBound);
         }
      }
   }
      
   if (hLineTable)
      {
      GlobalUnlock(hLineTable);
      GlobalFree(hLineTable);
      }

   return 0;
}

FPFONT FPLIB fpRotateFont(HDC hDC, HFONT hFont, int lfEscapement)
{
TEXTMETRIC tm;
LOGFONT    LogFont;
FPFONT     fpFont;

fpFont = fpGetFontFromHDC(hFont, hDC);
hFont = fpGetFontHandle(fpFont);
fpGetFontLogFont(fpFont, &LogFont);
fpGetFontTextMetric(fpFont, &tm);
LogFont.lfWidth = 0;
LogFont.lfOrientation = lfEscapement;
LogFont.lfEscapement = lfEscapement;
LogFont.lfQuality = DEFAULT_QUALITY;
LogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;

if (lfEscapement == 2700 || lfEscapement == 900)
   LogFont.lfHeight = LogFont.lfHeight * GetDeviceCaps(hDC, LOGPIXELSX) /
      GetDeviceCaps(hDC, LOGPIXELSY);

if (!(tm.tmPitchAndFamily & TMPF_TRUETYPE) && tm.tmCharSet == SHIFTJIS_CHARSET)
   {
   _fmemset(LogFont.lfFaceName, '\0', LF_FACESIZE);
   lstrcpy(LogFont.lfFaceName, _T("‚l‚r –¾’©"));
   lstrcpy(LogFont.lfFaceName, _T("lr >)"));
   LogFont.lfCharSet = SHIFTJIS_CHARSET;
   LogFont.lfOrientation = 0;
   }
else
   {
	// RFW - 5/13/02 - RUN_CEL_004_001
//   LogFont.lfPitchAndFamily = tm.tmPitchAndFamily;
//   LogFont.lfCharSet = tm.tmCharSet;
//   LogFont.lfPitchAndFamily = 0;
//   LogFont.lfCharSet = 0;
   }

fpReleaseFont(fpFont);
fpFont = fpGetLogFont(&LogFont); // (WORD)CreateFontIndirect(&LogFont);

#if 0
// If the current font could not be rotated,
// then choose the closest TrueType font.
SelectObject (hDC, hFont);

if (!(GetDeviceCaps(hDC, TEXTCAPS) & (TC_CR_90 | TC_CR_ANY)))
   {
   SelectObject (hDC, hFontOld);
   LogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;

   fpFont = fpGetLogFont (&LogFont); // (WORD)CreateFontIndirect(&LogFont);
   hFont  = fpGetFontHandle(fpFont);
   }
#endif

return (fpFont);
}

// fix for JUS47 -scl
FPFONT FPLIB fpRotateFontEx(HINSTANCE hInst, HDC hDC, HFONT hFont, int lfEscapement)
{
TEXTMETRIC tm;
LOGFONT    LogFont;
FPFONT     fpFont;

fpFont = fpGetFontFromHDCEx(hInst, hFont, hDC);
hFont = fpGetFontHandleEx(hInst, fpFont);
fpGetFontLogFontEx(hInst, fpFont, &LogFont);
fpGetFontTextMetricEx(hInst, fpFont, &tm);

LogFont.lfWidth = 0;
LogFont.lfOrientation = lfEscapement;
LogFont.lfEscapement = lfEscapement;
LogFont.lfQuality = DEFAULT_QUALITY;
LogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;

if (lfEscapement == 2700 || lfEscapement == 900)
   LogFont.lfHeight = LogFont.lfHeight * GetDeviceCaps(hDC, LOGPIXELSX) /
      GetDeviceCaps(hDC, LOGPIXELSY);

if (!(tm.tmPitchAndFamily & TMPF_TRUETYPE) && tm.tmCharSet == SHIFTJIS_CHARSET)
   {
   _fmemset(LogFont.lfFaceName, '\0', LF_FACESIZE);
   lstrcpy(LogFont.lfFaceName, _T("‚l‚r –¾’©"));
//   lstrcpy(LogFont.lfFaceName, "lr >)");
   LogFont.lfCharSet = SHIFTJIS_CHARSET;
   LogFont.lfOrientation = 0;
   }
else
   {
	// RFW - 5/13/02 - RUN_CEL_004_001
   LogFont.lfPitchAndFamily = tm.tmPitchAndFamily;
   LogFont.lfCharSet = tm.tmCharSet;
//   LogFont.lfPitchAndFamily = 0;
//   LogFont.lfCharSet = 0;
   }

fpReleaseFontEx(hInst, fpFont);
fpFont = fpGetLogFontEx(hInst, &LogFont); // (WORD)CreateFontIndirect(&LogFont);

#if 0
// If the current font could not be rotated,
// then choose the closest TrueType font.
SelectObject (hDC, hFont);

if (!(GetDeviceCaps(hDC, TEXTCAPS) & (TC_CR_90 | TC_CR_ANY)))
   {
   SelectObject (hDC, hFontOld);
   LogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;

   fpFont = fpGetLogFontEx (hInst, &LogFont); // (WORD)CreateFontIndirect(&LogFont);
   hFont  = fpGetFontHandleEx(hInst, fpFont);
   }
#endif

return (fpFont);
}
// fix for JUS47 -scl
