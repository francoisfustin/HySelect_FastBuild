/*    PortTool v2.2     FPTOOL45.C          */

//
// FarPoint DrawText
//

#include "fptools.h"
#include "string.h"

#include "fptstr.h"

#define CHARS_MOVED(lpNext, lpPrev)  (int)(lpNext - lpPrev)

LPTSTR FPLIB fpWordWrap (HDC hdc, LPFPWORDWRAP lpww, TEXTMETRIC FAR *lptm)
{                                     
   int          tmBreakChar = lpww->tmBreakChar;
   int          pageWidth   = (int)lpww->pageWidth;
   FPTSTR       lpszLine    = (FPTSTR)lpww->lpszFirst;
   FPFONT       fpFont      = lpww->fpFont;
   int          cb          = lpww->cb;
   LPCHARWIDTHS lpCharWidths = lpww->lpCharWidths;  // SCP 12/31/98
   int          dWidth = 0;
   BYTE         fFirstTime = (BYTE)TRUE;
   FPTSTR       lpszPrev, lpszNext, lpszTmp, lpszOrig;
   short        nWord, nWhite = 0, nCh = 0;
   BOOL         fNewLine = FALSE;
   UINT         nLastChar = (UINT)max(lptm->tmLastChar, 255);
   GLOBALHANDLE hCharWidths = 0;
   int          iCharWidths[255 + 1];
   BOOL         fCharWidthsLocked = FALSE;
   int          iCharOverhang = 0;

   // Lock the char width array (from fptools) if none were specified.
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
/* GAB 04/22/02 */    GetCharWidth(hdc, 0, nLastChar, lpCharWidths);
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

   if (lpww->fuMnemonic) lpww->fuMnemonic = (UINT)(-1);
   
   lpww->dBreakCount = 0;
   lpww->dWidth = 0;
   lpww->dLength = 0;
   lpszPrev = lpszLine;
   lpszNext = lpszLine;  
   lpszOrig = lpszLine;  
   while (cb > 0 && dWidth <= pageWidth && !fNewLine)
   {
      // scan white space
      nWhite = 0, nWord = 0;
      while (cb > 0 && *lpszLine == tmBreakChar && dWidth <= pageWidth)
      {
         lpszPrev = lpszLine;
         // RFW - 8/2/04 - 14984
         // dWidth += lpCharWidths[(unsigned)tmBreakChar] - iCharOverhang;
         dWidth += (int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang;
         lpszLine = CharNext(lpszLine);
         nWhite++;
         cb -= CHARS_MOVED(lpszLine, lpszPrev);
      }
      
      // scan word
      lpszNext = lpszLine;
      if (lpww->fuMnemonic)               
      {
         BOOL fMnemonic = FALSE;
         while (cb > 0 && *lpszLine != tmBreakChar && dWidth <= pageWidth)
         {
            lpszTmp = CharNext(lpszLine);
            lpszPrev = lpszLine;
            cb -= CHARS_MOVED(lpszTmp, lpszLine);
            if (cb > 0 && (*lpszLine == (TCHAR)'\n' || *lpszLine == (TCHAR)'\r'))
            {
               if (*lpszLine == (TCHAR)'\r' && *lpszTmp == (TCHAR)'\n')
               {
                  lpszLine = CharNext(lpszTmp);
                  cb -= CHARS_MOVED(lpszLine, lpszTmp);
               }
               else
                  lpszLine = lpszTmp;

               fNewLine = TRUE;
               break;
            }
            if (fMnemonic)
            {
               fMnemonic = FALSE;
               dWidth += (int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang;
            }
            else if (*lpszLine == (TCHAR)'&')
            {
               fMnemonic = TRUE;
               lpww->fuMnemonic = fpDBCSGetChar(lpszTmp);
            }
            else 
               dWidth += (int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang;

            nWord += (short)CHARS_MOVED(lpszTmp, lpszLine);
            lpszLine = lpszTmp;
         }  //end of while
      } // end if
      else
      {
         int iCharSize;
#ifdef WIN32 // RFW - 9/21/01 - 8600
			DWORD dwFontLanguageInfo = lpww->dwFontLanguageInfo;
#endif

         while (cb > 0 && *lpszLine != tmBreakChar && dWidth <= pageWidth)
         {
            lpszTmp = CharNext(lpszLine);
            lpszPrev = lpszLine;
            iCharSize = (int)CHARS_MOVED(lpszTmp, lpszLine);
            if (cb > 0 && (*lpszLine == (TCHAR)'\n' || *lpszLine == (TCHAR)'\r'))
            {
               if (*lpszLine == (TCHAR)'\r' && *lpszTmp == (TCHAR)'\n')
               {
                  lpszLine = CharNext(lpszTmp);
                  cb -= 2;
               }
               else
               {
                  lpszLine = lpszTmp;
                  cb -= 1;
               }

               fNewLine = TRUE;
               break;
            }

#ifdef WIN32 // RFW - 9/21/01 - 8600
				    if (dwFontLanguageInfo && (dwFontLanguageInfo & GCP_REORDER))
					  {
					    long lWidthOld = dWidth;
					    SIZE tmpSize;

					    GetTextExtentPoint(hdc, (LPTSTR)lpszOrig, nCh + nWord + nWhite + iCharSize, &tmpSize);
					    dWidth = tmpSize.cx - iCharOverhang;
					  }
				    else
#endif
// GAB 04/20/02
#ifdef _UNICODE
              /* RFW - 7/29/08 - 22700
              if ((*lpszLine >= 0xD800 && *lpszLine <= 0xDBFF) &&
                  (*CharNext(lpszLine) >= 0xDC00 && *CharNext(lpszLine) <= 0xDFFF))
              */
              if (((*lpszLine >= 0xD800 && *lpszLine <= 0xDBFF) &&
                   (lpszLine[1] >= 0xDC00 && lpszLine[1] <= 0xDFFF)) ||
                  (iCharSize == 2))
              {
                  TCHAR acSurrogate[2] = {0,0};
                  SIZE tmpSize;

                  acSurrogate[0] = lpszLine[0];
                  acSurrogate[1] = lpszLine[1];
			            GetTextExtentPoint32(hdc, acSurrogate, 2, &tmpSize);
                  dWidth += (int)tmpSize.cx - iCharOverhang;
               }
               else
#endif //_UNICODE
				        dWidth += (int)fpDBCSGetCharWidth(hdc, (LPTSTR)lpszLine, lpCharWidths, nLastChar) - iCharOverhang;

/* GAB 04/21/02 original code
        cb -= (int)CHARS_MOVED(lpszTmp, lpszLine);
				nWord += (short)CHARS_MOVED(lpszTmp, lpszLine);
				lpszLine = lpszTmp;
*/
#ifdef _UNICODE
              /* RFW - 7/29/08 - 22700
              if ((*lpszLine >= 0xD800 && *lpszLine <= 0xDBFF) &&
                  (*CharNext(lpszLine) >= 0xDC00 && *CharNext(lpszLine) <= 0xDFFF))
              */
              if ((*lpszLine >= 0xD800 && *lpszLine <= 0xDBFF) &&
                  (lpszLine[1] >= 0xDC00 && lpszLine[1] <= 0xDFFF))
              {
						cb -= 2*iCharSize;
						nWord += 2*iCharSize;
						/* RFW - 6/23/08 - Bug in Vista
						lpszLine = CharNext(lpszTmp);
						*/
						lpszLine = &lpszTmp[1];
              }
              else
#endif
              {
						cb -= iCharSize;
						nWord += (short)iCharSize;
						lpszLine = lpszTmp;
              }
// GAB 04/21/02
          } // end while
      } //end else

      if (fFirstTime)
      {
         fFirstTime = (BYTE)FALSE;

         /* RFW - 5/23/07 - 20480 - I moved it outside of the if statement.
         dWidth += (int)lptm->tmOverhang;
			*/
         dWidth += iCharOverhang;
      }

      if (nWord && dWidth <= pageWidth) 
      {
         nCh += nWord + nWhite;
         lpww->dBreakCount += nWhite;
         lpww->dWidth = dWidth;
      }

   } // end while
   if (dWidth > pageWidth)
   {
     if (lpww->dBreakCount == 0)
     {
       if (nWhite == 0 && nWord == 1)
         nCh = 1;

         else if (nWhite == 0)
         {
// GAB 04/20/02 lpww->dWidth = dWidth - (fpDBCSGetCharWidth(hdc, (LPTSTR)lpszPrev,
//                                     lpCharWidths, nLastChar) - iCharOverhang);
//              nCh = nWord - CHARS_MOVED(lpszLine, lpszPrev);
//              cb += CHARS_MOVED(lpszLine, lpszPrev);
//              lpszLine = lpszPrev; // RFW - 9/25/97 - JIS3634
#ifdef _UNICODE
           if (*lpszPrev >= 0xD800 && *lpszPrev <= 0xDBFF)
           {
              if (*CharNext(lpszPrev) >= 0xDC00 && *CharNext(lpszPrev) <= 0xDFFF)
              {
                TCHAR acSurrogate[2] = {0,0};
                SIZE tmpSize;
                acSurrogate[0] = lpszPrev[0];
                acSurrogate[1] = lpszPrev[1];
			          GetTextExtentPoint32(hdc, acSurrogate, 2, &tmpSize);
                lpww->dWidth = dWidth - ((int)tmpSize.cx - iCharOverhang);
                nCh = nWord - ((CHARS_MOVED(lpszLine, lpszPrev)));
                cb += ((int)CHARS_MOVED(lpszLine, lpszPrev));
                lpszLine = lpszPrev;
               }
           }
           else
#endif
           {
              lpww->dWidth = dWidth - (fpDBCSGetCharWidth(hdc, (LPTSTR)lpszPrev,
                                       lpCharWidths, nLastChar) - iCharOverhang);
              nCh = nWord - CHARS_MOVED(lpszLine, lpszPrev);
              cb += CHARS_MOVED(lpszLine, lpszPrev);
              lpszLine = lpszPrev; // RFW - 9/25/97 - JIS3634
           }
// GAB 04/20/02
         }
         else
         {
            cb += nWord;
            lpszLine = lpszNext;
         }
      }
      else
         cb += nWord;         
      if (*lpszPrev != tmBreakChar && nCh > 1)
         lpszLine = lpszPrev;
      }

   lpww->dLength = nCh;

   if (!fNewLine && *lpszLine == tmBreakChar && cb > 0)
   {
      lpszNext = CharNext(lpszLine);
      cb -= CHARS_MOVED(lpszNext, lpszLine);
   }
   else if (fNewLine || nWhite == 0)
      lpszNext = lpszLine;   // rdw
      
   lpww->lpszNext = (LPTSTR)lpszNext;
   
   if (!fNewLine && (*lpww->lpszNext == 0 || cb == 0))
      lpww->lpszNext = NULL;
   
   lpww->cb = cb;

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
