/*    PortTool v2.2     FPTOOL42.C          */

//
// FarPoint DrawText
//

#include "fptools.h"
#include "string.h"

void FPLIB fp3dTextOut (HDC hdc, int x, int y, UINT fuOpt, 
   LPRECT lprc, LPCTSTR lpsz, UINT cbString, LPINT lpDx,
   LPTEXT3DINFO lp3d);

                  
void  FPLIB fpExtTextOut (HDC hdc, int x, int y, UINT fuOptions, 
         LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx,
         LPTEXT3DINFO lp3d, LPFPDRAWTEXT lpdt)
{
   UINT  fuOpt = (fuOptions & ETO_CLIPPED);
   UINT  fuOldTA;
#if defined(_WIN64) || defined(_IA64)
   UINT_PTR cb = cbString;
#endif

   if (!lpsz)
      return;
      
   if (fuOptions & ETO_MNEMONIC)
      {
      POINT ptMnemonic, ptTrailing;
      LPTSTR lpszTemp = lpsz;
      LPTSTR lpszTempOrig;
      LPTSTR lpszCh = NULL;
      TEXTMETRIC tm;
#if defined(_WIN64) || defined(_IA64)
	  UINT_PTR n;
#else
      short n;
#endif

      GetTextMetrics (hdc, &tm);
      ptTrailing.x = x;
      ptTrailing.y = y;
//      MoveTo(hdc, ptTrailing.x, ptTrailing.y);
      fuOldTA = SetTextAlign(hdc, TA_UPDATECP);

#if defined(_WIN64) || defined(_IA64)
      while (*lpszTemp && cb > 0)
#else
      while (*lpszTemp && cbString > 0)
#endif
         {
         MoveToEx(hdc, ptTrailing.x, ptTrailing.y, NULL);
         n = 0;
#if defined(_WIN64) || defined(_IA64)
         while (*lpszTemp && cb > 0 && *lpszTemp != (TCHAR)'&')
#else
         while (*lpszTemp && cbString > 0 && *lpszTemp != (TCHAR)'&')
#endif
            {
            lpszTempOrig = lpszTemp;
            lpszTemp = CharNext(lpszTemp);
#if defined(_WIN64) || defined(_IA64)
            cb -= lpszTemp - lpszTempOrig, n += lpszTemp - lpszTempOrig;
#else
            cbString -= lpszTemp - lpszTempOrig, n += lpszTemp - lpszTempOrig;
#endif
            }
         if (n)
            {
#if defined(_WIN64) || defined(_IA64)
            fp3dTextOut (hdc, 0, 0, fuOpt, lprc, lpsz, (UINT)n, lpDx, lp3d);
#else
            fp3dTextOut (hdc, 0, 0, fuOpt, lprc, lpsz, n, lpDx, lp3d);
#endif
            GetCurrentPositionEx (hdc, &ptTrailing);
            //if (!lpdt || !(lpdt->fuStyle & (DTX_ROTATE | DTX_FLIP)))
               //ptTrailing.x -= tm.tmOverhang; // This needs fixed for rotation - RFW
            }
         else
            GetCurrentPositionEx (hdc, &ptTrailing);

#if defined(_WIN64) || defined(_IA64)
         if (cb > 0 && *lpszTemp == (TCHAR)'&')
#else
         if (cbString > 0 && *lpszTemp == (TCHAR)'&')
#endif
            {
            lpszTempOrig = lpszTemp;
            lpszTemp = CharNext(lpszTemp);
#if defined(_WIN64) || defined(_IA64)
            cb -= lpszTemp - lpszTempOrig;
#else
            cbString -= lpszTemp - lpszTempOrig;
#endif
            lpsz = lpszTemp;
            if (*lpszTemp && *lpszTemp != (TCHAR)'&' && fuOptions & ETO_MNSHOW)
               {
               _fmemcpy(&ptMnemonic, &ptTrailing, sizeof(POINT));
               lpszCh = lpszTemp;
               }
            
            if (*lpszTemp == (TCHAR)'&')
               {
               MoveToEx(hdc, ptTrailing.x, ptTrailing.y, NULL);
               fp3dTextOut (hdc, 0, 0, fuOpt, lprc, lpsz, 1, lpDx, lp3d);
               GetCurrentPositionEx (hdc, &ptTrailing);
               //if (!lpdt || !(lpdt->fuStyle & (DTX_ROTATE | DTX_FLIP)))
                  //ptTrailing.x -= tm.tmOverhang;
               lpszTempOrig = lpszTemp;
               lpszTemp = CharNext(lpszTemp);
#if defined(_WIN64) || defined(_IA64)
               cb -= lpszTemp - lpszTempOrig;
#else
               cbString -= lpszTemp - lpszTempOrig;
#endif
               lpsz = lpszTemp;
               }
            }
         }
      // go back and print the mnemonic character with the underline
      // after we know we have found the last one.
      if (lpszCh)
         {
         HFONT   hFontOld, hFont;
         LOGFONT LogFont;
         FPFONT  fpFont;
         
         hFontOld = SelectObject(hdc, GetStockObject(SYSTEM_FONT));
         _fmemset(&LogFont, 0, sizeof(LOGFONT));
         GetObject(hFontOld, sizeof(LOGFONT), &LogFont);
         LogFont.lfUnderline = TRUE;
         fpFont = fpGetLogFont(&LogFont);
         hFont = fpGetFontHandle(fpFont);
         
         SelectObject(hdc, hFont);
         MoveToEx(hdc, ptMnemonic.x, ptMnemonic.y, NULL);
         fp3dTextOut (hdc, 0, 0, fuOpt, lprc, lpszCh, 1, lpDx, lp3d);
         MoveToEx(hdc, ptTrailing.x, ptTrailing.y, NULL);
         SelectObject (hdc, hFontOld);
         fpReleaseFont(fpFont);
         }
      SetTextAlign(hdc, fuOldTA);
      }
   else
#if defined(_WIN64) || defined(_IA64)
      fp3dTextOut (hdc, x, y, fuOpt, lprc, lpsz, (UINT)cb, lpDx, lp3d);
#else
      fp3dTextOut (hdc, x, y, fuOpt, lprc, lpsz, cbString, lpDx, lp3d);
#endif

   return;
}

// fix for JUS49 -scl
void  FPLIB fpExtTextOutEx (HINSTANCE hInst, HDC hdc, int x, int y, UINT fuOptions, 
         LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx,
         LPTEXT3DINFO lp3d, LPFPDRAWTEXT lpdt)
{
   UINT  fuOpt = (fuOptions & ETO_CLIPPED);
   UINT  fuOldTA;
#if defined(_WIN64) || defined(_IA64)
   UINT_PTR cb = cbString;
#endif

   if (!lpsz)
      return;
      
   if (fuOptions & ETO_MNEMONIC)
      {
      POINT ptMnemonic, ptTrailing;
      LPTSTR lpszTemp = lpsz;
      LPTSTR lpszTempOrig;
      LPTSTR lpszCh = NULL;
      TEXTMETRIC tm;
#if defined(_WIN64) || defined(_IA64)
	  UINT_PTR n;
#else
      short n;
#endif

      GetTextMetrics (hdc, &tm);
      ptTrailing.x = x;
      ptTrailing.y = y;
//      MoveTo(hdc, ptTrailing.x, ptTrailing.y);
      fuOldTA = SetTextAlign(hdc, TA_UPDATECP);

#if defined(_WIN64) || defined(_IA64)
      while (*lpszTemp && cb > 0)
#else
      while (*lpszTemp && cbString > 0)
#endif
         {
         MoveToEx(hdc, ptTrailing.x, ptTrailing.y, NULL);
         n = 0;
#if defined(_WIN64) || defined(_IA64)
         while (*lpszTemp && cb > 0 && *lpszTemp != (TCHAR)'&')
#else
         while (*lpszTemp && cbString > 0 && *lpszTemp != (TCHAR)'&')
#endif
            {
            lpszTempOrig = lpszTemp;
            lpszTemp = CharNext(lpszTemp);
#if defined(_WIN64) || defined(_IA64)
            cb -= lpszTemp - lpszTempOrig, n += lpszTemp - lpszTempOrig;
#else
            cbString -= lpszTemp - lpszTempOrig, n += lpszTemp - lpszTempOrig;
#endif
            }
         if (n)
            {
#if defined(_WIN64) || defined(_IA64)
            fp3dTextOut (hdc, 0, 0, fuOpt, lprc, lpsz, (UINT)n, lpDx, lp3d);
#else
            fp3dTextOut (hdc, 0, 0, fuOpt, lprc, lpsz, n, lpDx, lp3d);
#endif
            GetCurrentPositionEx (hdc, &ptTrailing);
            // messes up the positioning of the mnemonic character -scl
            //if (!lpdt || !(lpdt->fuStyle & (DTX_ROTATE | DTX_FLIP)))
            //   ptTrailing.x -= tm.tmOverhang; // This needs fixed for rotation - RFW
            }
         else
            GetCurrentPositionEx (hdc, &ptTrailing);
#if defined(_WIN64) || defined(_IA64)
         if (cb > 0 && *lpszTemp == (TCHAR)'&')
#else
         if (cbString > 0 && *lpszTemp == (TCHAR)'&')
#endif
            {
            lpszTempOrig = lpszTemp;
            lpszTemp = CharNext(lpszTemp);
#if defined(_WIN64) || defined(_IA64)
            cb -= lpszTemp - lpszTempOrig;
#else
            cbString -= lpszTemp - lpszTempOrig;
#endif
            lpsz = lpszTemp;
            if (*lpszTemp && *lpszTemp != (TCHAR)'&' && fuOptions & ETO_MNSHOW)
               {
               _fmemcpy(&ptMnemonic, &ptTrailing, sizeof(POINT));
               lpszCh = lpszTemp;
               }
            
            if (*lpszTemp == (TCHAR)'&')
               {
               MoveToEx(hdc, ptTrailing.x, ptTrailing.y, NULL);
               fp3dTextOut (hdc, 0, 0, fuOpt, lprc, lpsz, 1, lpDx, lp3d);
               GetCurrentPositionEx (hdc, &ptTrailing);
               // messes up the positioning of the mnemonic character -scl
               //if (!lpdt || !(lpdt->fuStyle & (DTX_ROTATE | DTX_FLIP)))
               //   ptTrailing.x -= tm.tmOverhang;
               lpszTempOrig = lpszTemp;
               lpszTemp = CharNext(lpszTemp);
#if defined(_WIN64) || defined(_IA64)
               cb -= lpszTemp - lpszTempOrig;
#else
               cbString -= lpszTemp - lpszTempOrig;
#endif
               lpsz = lpszTemp;
               }
            }
         }
      // go back and print the mnemonic character with the underline
      // after we know we have found the last one.
      if (lpszCh)
         {
         HFONT   hFontOld, hFont;
         LOGFONT LogFont;
         FPFONT  fpFont;
         
         hFontOld = SelectObject(hdc, GetStockObject(SYSTEM_FONT));
         _fmemset(&LogFont, 0, sizeof(LOGFONT));
         GetObject(hFontOld, sizeof(LOGFONT), &LogFont);
         LogFont.lfUnderline = TRUE;
         fpFont = fpGetLogFontEx(hInst, &LogFont);
         hFont = fpGetFontHandleEx(hInst, fpFont);
         
         SelectObject(hdc, hFont);
         MoveToEx(hdc, ptMnemonic.x, ptMnemonic.y, NULL);
         fp3dTextOut (hdc, 0, 0, fuOpt, lprc, lpszCh, 1, lpDx, lp3d);
         MoveToEx(hdc, ptTrailing.x, ptTrailing.y, NULL);
         SelectObject (hdc, hFontOld);
         fpReleaseFontEx(hInst, fpFont);
         }
      SetTextAlign(hdc, fuOldTA);
      }
   else
#if defined(_WIN64) || defined(_IA64)
      fp3dTextOut (hdc, x, y, fuOpt, lprc, lpsz, (UINT)cb, lpDx, lp3d);
#else
      fp3dTextOut (hdc, x, y, fuOpt, lprc, lpsz, cbString, lpDx, lp3d);
#endif
             
   return;
}
// fix for JUS49 -scl

