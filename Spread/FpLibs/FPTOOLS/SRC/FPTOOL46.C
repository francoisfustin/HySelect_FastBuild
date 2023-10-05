/*    PortTool v2.2     FPTOOL46.C          */

//
// FarPoint DrawText
//

#include "fptools.h"
#include "string.h"
#include "fptstr.h"

#define CHARS_MOVED(lpNext, lpPrev)  (int)(lpNext - lpPrev)

#define dtHLEFT      (!(fuFormat & (DT_RIGHT | DT_CENTER | DT_JUSTIFY)))
#define dtCALCRECT   (fuFormat & DT_CALCRECT)


LPTSTR FPLIB fpProcessLine(HDC hdc, LPFPWORDWRAP lpww, BOOL fVertical,
                             UINT fuFormat, TEXTMETRIC FAR *lptm)
{
   int          tmBreakChar = (int)lpww->tmBreakChar;
   int          pageWidth   = (int)lpww->pageWidth;
   FPTSTR       lpszLineOrg = (FPTSTR)lpww->lpszFirst;
   FPTSTR       lpszLine    = (FPTSTR)lpww->lpszFirst;
   FPTSTR       lpszLineSave;
   FPFONT       fpFont      = lpww->fpFont;
   int          cb          = lpww->cb;
   LPCHARWIDTHS lpCharWidths = lpww->lpCharWidths;  // SCP 12/31/98
   FPTSTR       lpszNext;
   short        nCh = 0;
   UINT         nLastChar = (UINT)max(lptm->tmLastChar, 255);
   GLOBALHANDLE hCharWidths = 0;
   int          iCharWidths[255 + 1];
   BOOL         fCharWidthsLocked = FALSE;
   int          iCharOverhang = 0;
   int			cWidth;

   // Lock the char width array (from fptools) if none was specified.
   if (!lpCharWidths && fpFont != NOFONT)
   {
     lpCharWidths = fpLockCharWidths(fpFont);
     if (lpCharWidths)
       fCharWidthsLocked = TRUE;
   }

   // If still no char widths, then get them from Windows.
   if (!lpCharWidths)
     {
     // RFW - 7/30/04 - 14874
     if (nLastChar < 1000)
       {
       if (nLastChar > 255)
         {
         hCharWidths = GlobalAlloc(GSHARE, sizeof(int) * (nLastChar + 1));

         if (!hCharWidths)
           return (NULL);

         lpCharWidths = (int FAR *)GlobalLock(hCharWidths);
         }
       else
         lpCharWidths = (int FAR *)iCharWidths;

#ifdef WIN32
//RAP 2/24/98   if (!GetCharWidth32(hdc, 0, nLastChar, lpCharWidths))
/* GAB 04/25/02 */    GetCharWidth(hdc, 0, nLastChar, lpCharWidths);
//     GetCharWidth32(hdc, 0, nLastChar, lpCharWidths);
#else
       GetCharWidth(hdc, 0, nLastChar, lpCharWidths);
#endif
       }

//   for (j = 0; j <= nLastChar; j++)	
//     lpCharWidths[j] -= lptm->tmOverhang;
     // *** Instead of changing each entry in the array, set this variable
     // *** and it will be used at each calculation in this function.
     iCharOverhang = (int)lptm->tmOverhang;

     } // if not locked

// enhancement SEL8276
   if (lpww->fDrawEllipses)
     lpww->dEllipsesWidth = 3 * ((int)fpDBCSGetCharWidth(hdc, _T("."), lpCharWidths, nLastChar) - iCharOverhang);
// enhancement SEL8276

   if (lpww->fuMnemonic) lpww->fuMnemonic = (UINT)(-1);

   lpww->dBreakCount = 0;
   lpww->dWidth      = lptm->tmOverhang;

   if (lpww->fuMnemonic)
      {
	   short nChChange;
      BOOL fMnemonic = FALSE;
      while (cb > 0 && (dtCALCRECT || !dtHLEFT || lpww->dWidth < pageWidth))
// RFW-7/18/96      while (*lpszLine != (TCHAR)'\n' && *lpszLine != (TCHAR)'\r' && cb > 0)
         {
         if (lpszLine && *lpszLine == (TCHAR)tmBreakChar)
            lpww->dBreakCount++;

         lpszNext = CharNext(lpszLine);
         cb -= CHARS_MOVED(lpszNext, lpszLine);

         if (fMnemonic)
            {
            fMnemonic = FALSE;
            lpww->dWidth += cWidth = ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang);
            }
         else if (lpszLine && *lpszLine == '&')
            {
            fMnemonic = TRUE;
            if (*lpszNext != '&')
               lpww->fuMnemonic = fpDBCSGetChar(lpszNext);
            }
         else 
    			lpww->dWidth += cWidth = ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang);
         nCh += nChChange = (short)(lpszNext - lpszLine);
         lpszLine = lpszNext;

		 if (lpww->fDrawEllipses && *lpszLine && (*lpszLine != tmBreakChar) && lpww->dWidth + ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang) > pageWidth)
		 {
			lpszLine = CharPrev(lpszLineOrg, lpszLine);
			while (lpww->dWidth - cWidth + lpww->dEllipsesWidth > pageWidth)
			{
				nChChange++;
				lpszLine = CharPrev(lpszLineOrg, lpszLine);
				cWidth += ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang);
			}
			nCh -= nChChange;
			lpww->dWidth -= cWidth;
			pageWidth = (int)lpww->dWidth;
			lpww->dWidth++;
			cb = 0;
		 }

         if (!fMnemonic && fVertical)
            break;
         }
      }
   else
      {
#ifdef WIN32 // RFW - 9/21/01 - 8600
		DWORD dwFontLanguageInfo = lpww->dwFontLanguageInfo;
#endif
	   int cWidth;

      while (cb > 0 && (dtCALCRECT || !dtHLEFT || lpww->dWidth < pageWidth))
// RFW-7/18/96      while (cb > 0 && *lpszLine != (TCHAR)'\n' && *lpszLine != (TCHAR)'\r')
         {

#ifdef WIN32 // RFW - 9/21/01 - 8600
			if (dwFontLanguageInfo && (dwFontLanguageInfo & GCP_REORDER))
				{
				long lWidthOld = lpww->dWidth;
				SIZE tmpSize;
            LPTSTR lpszTemp = CharNext(lpszLine);
            int iCharSize = CHARS_MOVED(lpszTemp, lpszLine);

	         GetTextExtentPoint(hdc, (LPTSTR)lpszLineOrg, nCh + iCharSize, &tmpSize);
				   lpww->dWidth = tmpSize.cx - iCharOverhang;
				   cWidth = lpww->dWidth - lWidthOld;
				}
			else
#endif
// GAB 04/25/02	lpww->dWidth += cWidth = ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang);

#ifdef _UNICODE
              if ((*lpszLine >= 0xD800 && *lpszLine <= 0xDBFF) &&
                  (*CharNext(lpszLine) >= 0xDC00 && *CharNext(lpszLine) <= 0xDFFF))
              {
                  TCHAR acSurrogate[2] = {0,0};
                  SIZE tmpSize;

                  acSurrogate[0] = lpszLine[0];
                  acSurrogate[1] = lpszLine[1];
			            GetTextExtentPoint32(hdc, acSurrogate, 2, &tmpSize);
                  lpww->dWidth += cWidth = (int)tmpSize.cx - iCharOverhang;
               }
               else
#endif //_UNICODE
				        	lpww->dWidth += cWidth = ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang);


         if (*lpszLine == tmBreakChar)
            lpww->dBreakCount++;
         lpszLineSave = lpszLine;

// GAB 04/25/02 Original code
//     lpszLine = CharNext(lpszLine);
//     cb -= CHARS_MOVED(lpszLine, lpszLineSave);
//     nCh += (short)CHARS_MOVED(lpszLine, lpszLineSave);

#ifdef _UNICODE
              if ((*lpszLine >= 0xD800 && *lpszLine <= 0xDBFF) &&
                  (*CharNext(lpszLine) >= 0xDC00 && *CharNext(lpszLine) <= 0xDFFF))
              {
						/* RFW - 6/23/08 - Bug in Vista
						lpszLine = CharNext(lpszLine);
						lpszLine = CharNext(lpszLine);
						*/
						lpszLine = &lpszLine[2];

						cb -= ((int)CHARS_MOVED(lpszLine, lpszLineSave));
						nCh += ((short)CHARS_MOVED(lpszLine, lpszLineSave));
              }
              else
#endif
              {
                 lpszLine = CharNext(lpszLine);
                 cb -= CHARS_MOVED(lpszLine, lpszLineSave);
                 nCh += (short)CHARS_MOVED(lpszLine, lpszLineSave);
              }


//     if (current width + width of next char > pagewidth)
// fix for bug 9145 -scl
//		 if (lpww->fDrawEllipses && *lpszLine && (*lpszLine != tmBreakChar) && lpww->dWidth + ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths) - iCharOverhang) > pageWidth)
		 if (lpww->fDrawEllipses && *lpszLine && lpww->dWidth + ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang) > pageWidth)
		 {	// while (current width - width of current char + width of ellipses > pagewidth)
			while (lpww->dWidth - cWidth + lpww->dEllipsesWidth > pageWidth)
			{	// go back one char
				lpszLineSave = CharPrev(lpszLineOrg, lpszLineSave);
				cWidth += ((int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLineSave, lpCharWidths, nLastChar) - iCharOverhang);
			}
			// force code to break out of outer while loop
			lpww->dWidth -= cWidth;
			nCh -= (short)CHARS_MOVED(lpszLine, lpszLineSave);
			pageWidth = (int)lpww->dWidth;
			lpww->dWidth++;
			cb = 0;
		 }

         if (fVertical)
            break;
         }
      }

   lpww->dLength = nCh;
   
/* RFW-7/18/96
   while (*lpszLine == (TCHAR)'\r' && cb > 0)
      {
      lpszLineSave = lpszLine;
      lpszLine = CharNext(lpszLine);
      cb -= CHARS_MOVED(lpszLine, lpszLineSave);
      }

   if (*lpszLine == (TCHAR)'\n')
      {
      lpszLineSave = lpszLine;
      lpszLine = CharNext(lpszLine);
      cb -= CHARS_MOVED(lpszLine, lpszLineSave);
      }
*/
      
   if (lpszLine && *lpszLine && cb > 0)
      lpww->lpszNext = (LPTSTR)lpszLine;
   else
      lpww->lpszNext = NULL;
   
   //lpww->dWidth++;
   lpww->cb = cb;

// enhancement SEL8276
   if (lpww->fDrawEllipses)
      {
      if (lpww->dWidth <= pageWidth)
         lpww->fDrawEllipses = FALSE;
      else
         lpww->dWidth--;
      }
// enhancement SEL8276

   // If the char widths were locked, then unlock them.
   if (fCharWidthsLocked)
      fpUnlockCharWidths(fpFont);
   else if (hCharWidths)  // else if width array was allocated, then free it.
      {
      GlobalUnlock(hCharWidths);
      GlobalFree(hCharWidths);
      }

   return lpww->lpszNext; 
}
