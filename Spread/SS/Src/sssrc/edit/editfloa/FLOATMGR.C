/*
$Revision:   1.0  $
*/

/*
$Log:   C:/PVCS/SOURCE/FLOATMGR.C_V  $
 *
 *    Rev 1.0   23 Jan 1991 17:39:28   DC
 * Initial revision.
 *
 *    Rev 1.20   04 Dec 1990 13:40:24   Dirk
 *
 *    Rev 1.19   28 Sep 1990 13:40:06   Sonny
 *
 *    Rev 1.18   18 Jul 1990 15:59:56   Sonny
 * Fixed the problem with ProcessFloatWMKeyDown
 * 1) When all the items are hilited I forget to delete everything
 *
 *    Rev 1.17   17 Jul 1990 16:04:12   Sonny
 * Fixed the problem in ProcessFloatEMValidate.
 * If only FS_MONEY is used then it was not painting the screen properly
 * with the right format.
 *
 *    Rev 1.16   12 Jun 1990 14:10:18   Dirk
 * No change.
 *
 *    Rev 1.15   08 Jun 1990 16:12:20   Sonny
 * No change.
 *
 *    Rev 1.14   04 Jun 1990 15:28:18   Randall
 * Allow negative numbers in SetText.
 *
 *    Rev 1.13   30 May 1990 16:00:04   Sonny
 * Fixed the problem in EMValidate. It remembers the decimal pos incorrectly
 *
 *    Rev 1.12   28 May 1990 15:58:42   Sonny
 *
 *    Rev 1.11   25 May 1990 17:56:18   Sonny
 *
 *    Rev 1.10   24 May 1990 16:21:52   Sonny
 *
 *    Rev 1.9   23 May 1990 18:15:54   Sonny
 *
 *    Rev 1.8   23 May 1990 16:50:56   Sonny
 *
 *    Rev 1.7   22 May 1990 17:02:34   Sonny
 *
 *    Rev 1.6   21 May 1990 13:28:06   Randall
 *
 *    Rev 1.5   16 May 1990 17:35:58   Randall
 * No change.
 *
 *    Rev 1.4   15 May 1990 17:05:36   Sonny
 *
 *    Rev 1.3   14 May 1990 17:41:32   Randall
 * cd ..\editgen
 *
 *    Rev 1.2   14 May 1990 15:03:26   Randall
 * Notify parent if Alloc fails
 *
 *    Rev 1.1   14 May 1990 14:22:44   Bruce
 * No change.
 *
 *    Rev 1.0   11 May 1990 16:17:36   Bruce
 * Initial revision.
 *
 *    Rev 1.2   09 May 1990 15:27:18   Randall
 * Correctly position caret on WM_SETFOCUS and VK_DELETE (when highlighted)
 *
 *    Rev 1.1   01 May 1990 10:38:52   Sonny
 *
 *    Rev 1.0   23 Apr 1990 18:41:22   Sonny
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <float.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editfloa.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "editfloa\editfloa.h"
<-BORLAND*/
#endif

//#ifdef  BUGS
// Bug-001
extern void      SS_BeepLowlevel(HWND hwnd);
//#endif

void FloatSetText(HWND hWnd, LPEDITFLOATINFO lpInfo, double dfVal,
                  BOOL fPassOnOnly);
void FloatCopyDecDigits(LPTSTR lpszDest, LPTSTR lpszSrc, int iLen);
BOOL FloatIsValInRange(LPEDITFLOATINFO lpInfo, double dfVal);

#define ULONG unsigned long

static   double   dfZero = 0.0;

static   BOOL  GetFloatRange(LPEDITFLOATINFO lpInfo, LPTSTR szRange)
{
   LPTSTR   szTmp;
   double   dfMinVal;
   double   dfMaxVal;
   int      ch;
   int      iLenMin;
   int      iLenMax;

   // first for valid characters
   szTmp = szRange;
   while (ch = *szTmp)
   {
      if (ch != '-' && !isdigit(ch) && ch != ' ' && ch != '.')
         return(FALSE);
      ++szTmp;
   }
   szTmp = szRange;
   while (ch = *szTmp)       // look for the ' '
   {
      if (ch == ' ')
         break;
      ++szTmp;
   }
   if (!*szTmp)         // we have found none
      return(FALSE);
   ++szTmp;              // point to the next char
   // now szTmp points to the max range
   StringToFloat(szTmp, &dfMaxVal);
   *(szTmp - 1) = '\0';    // zero it out
   iLenMax = StrLen(szTmp);
   iLenMin = StrLen(szRange);
   StringToFloat(szRange, &dfMinVal);
   if (dfMinVal <= dfMaxVal)
   {
      lpInfo->bMinWidth = (BYTE)iLenMin;
      lpInfo->bMaxWidth = (BYTE)iLenMax;
      lpInfo->dfMaxVal = dfMaxVal;
      lpInfo->dfMinVal = dfMinVal;
      return(TRUE);
   }
   return(FALSE);
}


BOOL FloatFormat (LPEDITFLOATINFO lpInfo, LPDOUBLE lpVal, LPTSTR szString)

// This function returns the formatted float string to the caller via
// szString.  It used to be called FloatSetValue, and used to send a
// WM_SETTEXT straight to the float window rather than returning the
// string to the caller.  Returning the string allows the caller to
// change the field contents in site, if he has a pointer to it, which
// is preferable to sending WM_SETTEXT.
//
// Randall.  4 Sep 91.

{
#ifdef  BUGS
// Bug-002
   TCHAR szText[330];
#else
   TCHAR szText[60];
#endif
   BYTE bFractionalWidth;

   lpInfo->dfVal =  *lpVal;

   if (lpInfo->bFractionalWidth == 0)
      bFractionalWidth = 1;
   else
      bFractionalWidth = lpInfo->bFractionalWidth;

   if (InSet(lpInfo->style, FS_SEPARATOR))
      lpInfo->iPrevStrLen = StrPrintf(szText, _T("%.*Sm"), bFractionalWidth,
                                      lpInfo->cSeparator, lpInfo->cDecimalSign,
                                      *lpVal);

   else
      {
      lpInfo->iPrevStrLen = StrPrintf(szText, _T("%.*f"), bFractionalWidth,
                                      *lpVal);
      StrReplaceCh(szText, MAKEWORD(lpInfo->cDecimalSign, '.'));
      }

   if (lpInfo->bFractionalWidth == 0)
      {
      szText[lstrlen(szText) - 2] = '\0';
      lpInfo->iPrevStrLen -= 2;
      }

   if (InSet(lpInfo->style, FS_MONEY))
      {
      int iCurrencyPos;

      iCurrencyPos = *lpVal < dfZero ? 1 : 0;
      StrInsertChar(lpInfo->cCurrencySign, szText, iCurrencyPos);
      }

   StrCpy (szString, szText);

   return TRUE;
}

BOOL GetFloatPicture(LPEDITFLOATINFO lpInfo, LPCTSTR szRange)
{
   LPTSTR lpDecimal;
   int   iLen;
   BOOL  fRet = FALSE;

   if (szRange)
   {
      lpDecimal = StrChr(szRange, lpInfo->cDecimalSign);

      iLen = StrLen(szRange);

      if (!lpDecimal)
         lpDecimal = StrChr(szRange, '.');      // look for the default

      if ((iLen > 0) && (iLen < 70)) // The total width must be greater than 0
      {
         if (lpDecimal)
         {
            lpInfo->bIntegerWidth = (BYTE)(lpDecimal ?
                                    (lpDecimal - szRange) :
                                    iLen + 1);
            lpInfo->bTotalWidth = (BYTE)iLen;
            lpInfo->bFractionalWidth = (BYTE)(lpInfo->bTotalWidth - lpInfo->bIntegerWidth - 1);

            fRet = TRUE;
         }
         else
         {
            lpInfo->bFractionalWidth = 0;
            lpInfo->bTotalWidth = (BYTE)iLen;
            lpInfo->bMinWidth = (BYTE)iLen;
            lpInfo->bMaxWidth = (BYTE)iLen;
            lpInfo->bIntegerWidth = (BYTE)iLen;
         }
      }
   }
   return(fRet);
}

BOOL  ProcessFloatWMCreate(HWND hWnd, LPARAM lParam)
{
   HANDLE         hMem;
   LPEDITFLOATINFO  lpTmp;
   LONG           style;
   BOOL           fRet = FALSE;

   if (!(hMem = GlobalAlloc(GHND, (DWORD)sizeof(EDITFLOATINFO))))
      NotifyParent(hWnd, EN_ERRSPACE);
   else
   {
      TCHAR szText[5];

      // Set the parent handler at offset 4 of the window structure
      SetDefWndProc(hWnd, GetTBEditClassProc());
      SetWindowFloatField(hWnd, hMem);
      lpTmp = tbMemLock(hMem);
      if (lpTmp)
      {
         lpTmp->fEmpty = TRUE;
         lpTmp->dfMinVal = -DBL_MAX;
         lpTmp->dfMaxVal = DBL_MAX;
         lpTmp->hWnd = hWnd;
         lpTmp->bPostCount = 0;
         lpTmp->iWindowID = (LPARAM)((LPCREATESTRUCT)lParam)->hMenu;
         lpTmp->hWndParent = ((LPCREATESTRUCT)lParam)->hwndParent;
         // make the style to be right justified
         style = ((LPCREATESTRUCT)lParam)->style;
         style |= ES_RIGHT;
         lpTmp->bIntegerWidth = 10;
         lpTmp->bFractionalWidth = 2;
         lpTmp->bTotalWidth = 13;
         lpTmp->bMinWidth = 13;
         lpTmp->bMaxWidth = 13;
         ((LPCREATESTRUCT)lParam)->style = lpTmp->style = style;
         GetProfileString(WININTL, WIN_THOUSAND, WIN_DEFTHOUSAND, szText, 2);
         lpTmp->cSeparator = szText[0];
         GetProfileString(WININTL, WIN_DECIMAL, WIN_DEFDECIMAL, szText, 2);
         lpTmp->cDecimalSign = szText[0];
         GetProfileString(WININTL, WIN_CURRENCY, WIN_DEFCURRENCY, szText, 2);
         lpTmp->cCurrencySign = szText[0];
         lpTmp->iPrevStrLen = 0;
         SetWindowLong(hWnd, GWL_STYLE, style);
         GetFloatPicture(lpTmp, (LPTSTR)((LPCREATESTRUCT)lParam)->lpszName);
         fRet = TRUE;
      }
      else
         NotifyParent(hWnd, EN_ERRSPACE);
      tbMemUnlock(hMem);
   }
   return(fRet);
}

static   BOOL  CheckFloatRange(HWND hWnd, double dfVal)
{
   BOOL     fValid = FALSE;

   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      if (lpInfo->dfMinVal <= dfVal && dfVal <= lpInfo->dfMaxVal)
         fValid = TRUE;
      UnlockFloatHeader(hInfo);
   }
   return(fValid);
}


void  ProcessFloatWMDestroy(HWND hWnd)
{
   HANDLE   hMem;

   if (IsWindow(hWnd) && (hMem = GetWindowFloatField(hWnd)))
      {
      GlobalFree(hMem);
      SetWindowFloatField(hWnd, 0);
      }
}

BOOL  ProcessFloatEMRange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   BOOL  fRetCode = FALSE;

   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      fRetCode = GetFloatRange(lpInfo, (LPTSTR)lParam);
      UnlockFloatHeader(hInfo);
   }
   return(fRetCode);
}


BOOL  ProcessFloatWMChar(HWND hWnd, WPARAM wParam, LPARAM lParam)

// This function has been modified and, on the whole, simplified.  Its
// purpose now is to intercept and reject invalid characters and to
// process the minus and plus signs and the decimal point.  Special
// processing that was executed when the underlying field contained
// zero has been entirely removed.  Zero is no longer a special case.
//
// As far as the remaining functionallity is concerned, it is mostly
// unchanged.  If a plus, minus, or decimal point are detected, they
// are processed here and the function returns FALSE, preventing the
// character from being passed on to the superclass.
//
// Randall.  4 Sep 91.

{
   LPEDITFIELD lpField;
   LPTSTR   lpszDecimal;
   BOOL     fDigit = isdigit((int)wParam);
   BOOL     fRetCode = TRUE;
   int      iCaretPos;
   int      iDecimalPos;
   WORD     wLen;
#ifdef  BUGS
// Bug-002
   TCHAR     szText[330];
#else
   TCHAR     szText[64];       
#endif
   double   dfVal;
   LONG     lSel;

   LockFloatHeader(lpInfo, hInfo, hWnd);

   lpField = LockField (hWnd);

   if (lpInfo)
   {
		/* RFW - 11/3/99 - GIC10710
		if (wParam == '.' && (GetKeyState(VK_DECIMAL) & 0x8000))
		*/
		// RFW - 3/10/03 - 11402
		// if (GetKeyState(VK_DECIMAL) & 0x8000)
		if (VK_DECIMAL == lpInfo->wKeyDownVirtualKeyCode || VK_DECIMAL == wParam)
         wParam = (WPARAM)lpInfo->cDecimalSign;

      if (wParam != VK_BACK && !fDigit && wParam != '-'  && wParam != '+' && (TUCHAR)wParam != lpInfo->cDecimalSign)
      {
//#ifdef  BUGS
// Bug-001
         SS_BeepLowlevel(hWnd);
//#else
//         MessageBeep(1);
//#endif
         fRetCode = FALSE;
      }
      if (!fRetCode && lpInfo->hWndParent)
      {
         NotifyParent(hWnd, EN_INVALIDCHAR);
         lpInfo->bPostCount = 2;
      }

      if (fRetCode)
      {

         // Highlighted Text
         if (lpField->bHighlighted)
         {
            SendMessage(hWnd, WM_GETTEXT, sizeof(szText),
                        (LPARAM)((LPTSTR)szText));
            wLen = StrLen (szText);
            lSel = (LONG) SendMessage(hWnd, EM_GETSEL, 0, 0l);
            if (LOWORD(lSel) == 0 && (HIWORD(lSel)) == wLen)
            {
               lpInfo->fNegative = FALSE;
               EditFieldModifed(hWnd);
               ProcessFloatWMSetText(hWnd, 0, _T("0.0"), FALSE, TRUE);
               SendMessage(hWnd, WM_GETTEXT, sizeof(szText),
                           (LPARAM)(LPTSTR)szText);
               // 9/13/99 - RFW - TIB5870
               if (lpField->bHighlighted) DeselectText (hWnd, lpField);

               if (lpszDecimal = StrChr(szText, lpInfo->cDecimalSign))
                  iDecimalPos = (int)(lpszDecimal - (LPTSTR)szText);
               else
                  iDecimalPos = lstrlen(szText);

               if (GetOverstrikeMode(hWnd, FALSE))
                  SetInsertMode(hWnd, FALSE);

               lpInfo->dfVal = dfZero;
               SendMessage(hWnd, EM_SETCARETPOS, iDecimalPos, 0L);
            }
         }

         // Minus Sign
         if (wParam == '-')         // the user wants to negate the value
         {
            if (lpInfo->dfMinVal >= dfZero)        // min val is greater than zero
//#ifdef  BUGS
// Bug-001
               SS_BeepLowlevel(hWnd);
//#else
//               MessageBeep(1);
//#endif

            else if (lpInfo->dfVal == dfZero)      // Field contains zero
               {
               iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);   // Pos before dec point

               if (lpInfo->fNegative)
               {
                  if (iCaretPos > 0)
                     --iCaretPos;      // going + so decrease the count
               }
               else
                  ++iCaretPos;         // going - so increase the count

               lpInfo->fNegative = !lpInfo->fNegative;

               if (FloatIsValInRange(lpInfo, lpInfo->dfVal)) // RFW - 6/27/93
               {
                  EditFieldModifed(hWnd);
                  FloatSetText(hWnd, lpInfo, lpInfo->dfVal, 1);
                  SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);   // Pos before dec point
               }

               // else out of range
               else
//#ifdef  BUGS
// Bug-001
                  SS_BeepLowlevel(hWnd);
//#else
//                  MessageBeep(1);
//#endif
               }
               /*
               SendMessage (hWnd, EM_SETSEL, 0, 0L);
               */
            else
            {
               iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);   // Pos before dec point
               if (lpInfo->dfVal < dfZero)
               {
                  if (iCaretPos > 0)
                     --iCaretPos;      // going + so decrease the count
               }
               else
                  ++iCaretPos;         // going - so increase the count
               dfVal = -lpInfo->dfVal;
               if (FloatIsValInRange(lpInfo, dfVal)) // RFW - 6/27/93
               {
                  EditFieldModifed(hWnd);
                  FloatFormat(lpInfo, &dfVal, szText);
                  ProcessFloatWMSetText(hWnd, 0, szText, TRUE, TRUE);
                  SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);   // Pos before dec point
               }

               // else out of range
               else
//#ifdef  BUGS
// Bug-001
                  SS_BeepLowlevel(hWnd);
//#else
//                  MessageBeep(1);
//#endif
            }
            fRetCode = FALSE;
         }

         // Plus Sign
         else if (wParam == '+')
         {
            lpInfo->fNegative = FALSE;

            if (lpInfo->dfMaxVal < dfZero)        // max val is less than zero
//#ifdef  BUGS
// Bug-001
               SS_BeepLowlevel(hWnd);
//#else
//               MessageBeep(1);
//#endif

            else if (lpInfo->dfVal == dfZero)      // Field contains zero
               {
               EditFieldModifed(hWnd);
               ProcessFloatWMSetText(hWnd, 0, _T("0.0"), FALSE, TRUE);
               Edit_SetSel(hWnd, 0, 0);
               }

            else
            {
               if (lpInfo->dfVal < dfZero)
               {
                  iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);   // Pos before dec point
                  if (iCaretPos > 0)
                     --iCaretPos;
                  dfVal = -lpInfo->dfVal;
                  if (FloatIsValInRange(lpInfo, dfVal))// RFW - 6/27/93
                  {
                     EditFieldModifed(hWnd);
                     FloatFormat(lpInfo, &dfVal, szText);
                     ProcessFloatWMSetText(hWnd, 0, szText, TRUE, TRUE);
                     SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);   // Pos before dec point
                  }

                  // else out of range
                  else
//#ifdef  BUGS
// Bug-001
                     SS_BeepLowlevel(hWnd); 
//#else
//                     MessageBeep(1);
//#endif
               }
            }
            fRetCode = FALSE;
         }
#ifdef  BUGS
// Bug-003
         else if((TUCHAR)wParam == lpInfo->cDecimalSign && !lpInfo->bFractionalWidth)
            fRetCode = FALSE;
#endif

         // Decimal Point
         else if ((TUCHAR)wParam == lpInfo->cDecimalSign &&
                  lpInfo->bFractionalWidth)
         {

            // Is the field highlighted ?
            SendMessage(hWnd, WM_GETTEXT, sizeof(szText),
                        (LPARAM)(LPTSTR)szText);
            wLen = StrLen(szText);
            lSel = (LONG) SendMessage(hWnd, EM_GETSEL, 0, 0l);
            if (LOWORD(lSel) == 0 && (HIWORD(lSel)) == wLen)
            {
               if (InSet(lpInfo->style, FS_MONEY) &&
                   ((TUCHAR)szText[0] == lpInfo->cCurrencySign ||
                   (szText[0] == '-' && (TUCHAR)szText[1] == lpInfo->cCurrencySign)))
                  iDecimalPos = 3;
               else
                  iDecimalPos = 2;

               if (lpInfo->dfVal == dfZero)     // Just deselect it.
               {
                  Edit_SetSel(hWnd, iDecimalPos, iDecimalPos);
               }
               else
               {
                  EditFieldModifed(hWnd);
                  lpInfo->fNegative = FALSE;
                  ProcessFloatWMSetText(hWnd, 0, _T("0.0"), FALSE, TRUE);
               }

               PostMessage(hWnd, EM_SETCARETPOS, iDecimalPos, 0L);
            }

            else
            {
               iDecimalPos = (int)(StrChr(szText, lpInfo->cDecimalSign) - (LPTSTR)szText);
               iCaretPos = (int) SendMessage (hWnd, EM_GETCARETPOS, 0, 0L);
               if (iCaretPos != iDecimalPos)
//#ifdef  BUGS
// Bug-001
                  SS_BeepLowlevel(hWnd);  
//#else
//                  MessageBeep(1);
//#endif
               else
// 97' 2/6 Modified by BOC Gao. for BUG 00763 at WIN32
#ifdef SPREAD_JPN
#ifdef WIN32
                  SendMessage(hWnd, EM_SETCARETPOS, iDecimalPos+1, 0l);
#else
                  PostMessage(hWnd, EM_SETCARETPOS, iDecimalPos+1, 0l);
#endif
#else
                  PostMessage(hWnd, EM_SETCARETPOS, iDecimalPos+1, 0l);
#endif
// ----------------------------------<<
            }
            fRetCode = FALSE;
         }
      }

      UnlockFloatHeader(hInfo);
   }

   UnlockField (hWnd);
   return(fRetCode);
}


LONG  ProcessFloatEMValidate(HWND hWnd, WPARAM wParam, LPARAM lParam)

// The last part of this function, which concerns updating the field
// contents and repositioning the caret, has been simplified.  The major
// significant change is that the field contents are no longer updated
// using a WM_SETTEXT message.  Rather the field contents are changed
// in situ, via the far pointer to the field string, passed in lParam.
//
// The function used to send a WM_SETTEXT in some cases, and in other
// cases called FloatSetValue which would format the string and then
// send WM_SETTEXT.  FloatSetValue has been renamed FloatFormat and now
// returns the formatted string to the caller rather than sending it to
// the field via WM_SETTEXT.
//
// Randall.  4 Sep 91.

{
   LPEDITFIELD lpField;
   LPTSTR      lpszString = (LPTSTR) lParam;
   LPTSTR      lpszDecimalPos;
   LPTSTR      lpCurrency;
   LPTSTR      szVal;
   LPTSTR      szValTemp;
   double      dfVal;
   BOOL        fRetCode = FALSE;
   BOOL        fCurrencyNotBeg = FALSE;
   BOOL        fMoveCaretFarRight = FALSE;
   BOOL        fAddOneToCurrentPos = FALSE;
   short       Pos;
   int         iLen;
   int         iStrLen;
   LPTSTR      szDecimalPos;
   LPTSTR      szMinus;
   int         iCaretPos;
   int         iDecimalPos;
   int         iDistFromRHS;

   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      lpField = LockField (hWnd);

      /*
      if (lpField->iCurrentPos <= (int)wParam)
         fAddOneToCurrentPos = TRUE;
      */

      if (lParam)
			{
			LPTSTR lpszSrcPtr = lpszString;
			LPTSTR lpszDestPtr = lpszString;

			do
				{
				if (*lpszSrcPtr != ' ')
					*(lpszDestPtr++) = *lpszSrcPtr;

				} while (*(lpszSrcPtr++));

			szVal = lpszString;
			}
      else
         szVal = lpField->lpszString;

      Pos = (short)wParam;

      if (InSet(lpInfo->style, FS_MONEY) &&
          (lpCurrency = _ftcschr(szVal, lpInfo->cCurrencySign)))
         {
         StrDeleteCh(szVal, lpInfo->cCurrencySign);

         if (lpCurrency - szVal < Pos)
           Pos--;

         if (lpCurrency > szVal)
           fCurrencyNotBeg = TRUE;
         }

      szMinus = StrChr(szVal, '-');
      if (szMinus && (szMinus != szVal || StrChr(&szVal[1], '-')))
         {
         UnlockFloatHeader(hInfo);
         return(fRetCode);
         }

      if (lParam)
         {
         /********************************************************
         * If the value of the field is 0 before a character is
         * inserted and the cursor is to the left of the 0, then
         * delete the 0.
         ********************************************************/

         if (lpField->bCharBeingInserted)
            {
            szValTemp = szVal;

            if (((!szMinus && Pos == 0) || (szMinus && Pos <= 1)) &&
                lstrlen(szValTemp) - Pos > 1 &&
                szValTemp[Pos + 1] == '0' &&
                ((lpInfo->bFractionalWidth > 0 &&
                (TUCHAR)szValTemp[Pos + 2] == lpInfo->cDecimalSign) ||  // BJO 12Mar96 SEL3376
                (lpInfo->bFractionalWidth == 0 &&
                szValTemp[Pos + 2] == '\0')))
               {
               StrDeleteChar(szValTemp, Pos + 1);
               if (lpInfo->iPrevStrLen != 0)
                  lpInfo->iPrevStrLen--;

               if (fCurrencyNotBeg)
                  if (lpInfo->iPrevStrLen != 0)
                     lpInfo->iPrevStrLen--;

               if (lpInfo->bFractionalWidth == 0)
                  fMoveCaretFarRight = TRUE;
               }
            }
         }

      iLen = StrLen(szVal);

      // retrieve the previous string length
      iStrLen = (lpInfo->iPrevStrLen != 0) ? lpInfo->iPrevStrLen : iLen;

      // calculate the caret from the hi end
      // in this way we can position the caret again correctly
      if (fMoveCaretFarRight)
         iDistFromRHS = 0;
      else
         {
         // BJO 27Jun97 GIL708 - Before fix
         //iDistFromRHS = iStrLen - wParam;
         // BJO 27Jun97 GIL708 - Begin fix
         if (lpField->bProcessingBackspace && lpField->bHighlighted)
            iDistFromRHS = iStrLen - lpField->iHiEnd;
         else if (lpField->bProcessingDelete && lpField->bHighlighted)
            iDistFromRHS = iStrLen - lpField->iHiEnd;
         else if (lpField->bProcessingDelete)
            iDistFromRHS = (int)(iStrLen - wParam - 1);
         else
            iDistFromRHS = iStrLen - (int)wParam;
         // BJO 27Jun97 GIL708 - End fix
         if (InSet(lpInfo->style, FS_MONEY))
            ++iDistFromRHS;
         }

      // replace the decimal character with a valid one '.'
      // because the normal floating point conversion routine works
      // with english system

      if (InSet(lpInfo->style, FS_SEPARATOR))
      {
         StrDeleteCh((LPTSTR)lParam, lpInfo->cSeparator);
         iLen = StrLen(szVal);
      }

      if (lpInfo->cDecimalSign != '.')
         StrReplaceCh((LPTSTR)lParam, MAKEWORD('.', lpInfo->cDecimalSign));

      // look for the decimal position
      szDecimalPos = StrrChr(szVal, '.');

      // if the character before the decimal is non-digit then
      // we must insert one
      if (szDecimalPos && !isdigit(*(szDecimalPos - 1)) &&
          lstrlen(szVal) < MAXFIELD)
         StrInsertChar('0', szVal, (int)(szDecimalPos - szVal));

      // szVal should now be pointing to a valid floating point string
      // now check for ranges

      StringToFloat(szVal, &dfVal);
      if (FloatIsValInRange(lpInfo, dfVal))  // RFW - 6/27/93
         fRetCode = TRUE;

      // the length is greater than the total width so don't allow it

      if (iLen > 0 && szVal[0] == '0')
// 97' 2/4 Modified by BOC Gao. for BUG00763
// when IME mode is on, and at this ->  0|.00  situation
// input jp char and f8 to change to asc code
// can not input correctly.
#ifdef SPREAD_JPN
	  {
//Modify by BOC 99.7.27 (hyt)--------------------
//when IME mode is on, and at this ->  0|.00  situation
// input jp char "0.23" and f8 to change to asc code
// can not input correctly.
		 if(iLen>1 && szVal[1] != '.')
			lpField->iCurrentPos--;
		 iLen--;
//------------------------------------------------
	  }
#else
         iLen--;
#endif
      else if (iLen > 1 && szVal[0] == '-' && szVal[1] == '0')
// 97' 2/4 Modified by BOC Gao. for BUG00763
// when IME mode is on, and at this ->  0|.00  situation
// input jp char and f8 to change to asc code
// can not input correctly.
#ifdef SPREAD_JPN
	  {
//Modify by BOC 99.7.27 (hyt)--------------------
//when IME mode is on, and at this ->  0|.00  situation
// input jp char "0.23" and f8 to change to asc code
// can not input correctly.
		 if(iLen>2 && szVal[2] != '.')
			lpField->iCurrentPos--;
		 iLen--;
//------------------------------------------------
	  }
#else
         iLen--;
#endif

/* RFW - 8/8/03
      if (dfVal > dfZero)
      {
         if (iLen > (int)lpInfo->bTotalWidth)
            fRetCode = FALSE;
      }
      else
         if (iLen > (int)lpInfo->bTotalWidth + 1)  // Allow for minus sign
            fRetCode = FALSE;
*/
      if (iLen > 60)
         fRetCode = FALSE;

      // If the entry is OK store the result
      if (fRetCode)
         lpInfo->dfVal = dfVal;
      else
//#ifdef  BUGS
// Bug-001
         SS_BeepLowlevel(hWnd);
//#else
//         MessageBeep(MB_OK);
//#endif

      if (iLen == 0)    // if empty then we must post a message to the parent
      {
         if (!lpInfo->fEmpty)
         {
            if (lpInfo->hWndParent)
               NotifyParent(hWnd, EN_FIELDEMPTY);
            lpInfo->fEmpty = TRUE;
         }
      }

      else if (lpInfo->fEmpty)
      {
         NotifyParent(hWnd, EN_FIELDNONEMPTY);
         lpInfo->fEmpty = FALSE;
      }

      if (fRetCode)
      {
         // Format the new string

         /*
         if (dfVal == dfZero)
            lpInfo->fNegative = FALSE;
         */

         lpInfo->dfVal = dfVal;

         if (dfVal == dfZero && lpInfo->fNegative)
            {
            lpszString[0] = '-';
            FloatFormat(lpInfo, &dfVal, &lpszString[1]);
            // GIC8519 - RFW - 2/17/99
            // lpInfo->iPrevStrLen = lstrlen(lpszString);
            lpInfo->iPrevStrLen++;
            }

         else
            FloatFormat(lpInfo, &dfVal, lpszString);

         PostMessage (hWnd, EM_EDIT_ADJUSTLENGTH, 0, 0l);

         // Reposition the caret
         iCaretPos = StrLen (lpszString) - iDistFromRHS;
         if (lpszDecimalPos = StrChr(lpszString, lpInfo->cDecimalSign))
            iDecimalPos = (int)(lpszDecimalPos - lpszString);
         else
            iDecimalPos = lstrlen(lpszString);

         if (iCaretPos > iDecimalPos && iDistFromRHS > 1)
            ++iCaretPos;
         if (iCaretPos < 0)   // May happen after deleting a substring.
            iCaretPos = 0;

         if (lpszString[iCaretPos] == '-')
            iCaretPos++;

         if (InSet(lpInfo->style, FS_MONEY) &&
             (TUCHAR)lpszString[iCaretPos] == lpInfo->cCurrencySign)
            iCaretPos++;

         if (fAddOneToCurrentPos)
            iCaretPos++;

         PostMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0l);

         lpField->nChars = lstrlen(lpszString);
      }

      UnlockField(hWnd);
      UnlockFloatHeader(hInfo);
   }
   return(fRetCode);
}

BOOL ProcessFloatWMSetText(HWND hWnd, WPARAM wParam, LPCTSTR lParam,
                           BOOL fPassOnOnly, BOOL fInternal)
{
   LPCTSTR  lpszStr = lParam;
   LPCTSTR  lpszSave = lParam;
   LPCTSTR  lpszTemp;
   int      ch;
   BOOL     fRetCode = TRUE;
   double   dfVal;

#ifdef  BUGS
// Bug-002
   TCHAR     szText[330];
#else
   TCHAR     szText[60];
#endif

   short    iDecimalPos;
   short    nSign;

   if (!fPassOnOnly)
   {
      LockFloatHeader(lpInfo, hInfo, hWnd);
      if (lpszStr)
      {
         while (*lpszStr == ' ')
            lpszStr++;

         lpszSave = lpszStr;

         if (*lpszStr == '-') ++lpszStr;     // Leading minus sign. Ok.
         if (InSet(lpInfo->style, FS_MONEY) &&
             (TUCHAR)*lpszStr == lpInfo->cCurrencySign)
            ++lpszStr;  // Leading '$'

         /*
         if (StrLen(lpszStr) > (int)lpInfo->bTotalWidth)
            fRetCode = FALSE;
         */
         if (StrLen(lpszStr) >= 60)
            fRetCode = FALSE;

         if (fRetCode)
         {
            while (ch = (int)(TUCHAR)*lpszStr)
            {
               if (!isdigit(ch) && ((ch != lpInfo->cDecimalSign) &&
                   (ch != '.')) && (ch != lpInfo->cSeparator))
               {
                  fRetCode = FALSE;
                  break;
               }
               ++lpszStr;
            }
         }

         if (fRetCode)
         {
            lpszStr = lpszSave;

            if (*lpszStr == '-')
               {
               nSign = -1;
               lpszStr++;
               }
            else
               nSign = 1;

            if (InSet(lpInfo->style, FS_MONEY) &&
                (TUCHAR)*lpszStr == lpInfo->cCurrencySign)
               ++lpszStr;  // Leading '$'

            StrCpy(szText, lpszStr);
            if (InSet(lpInfo->style, FS_SEPARATOR))
               StrDeleteCh(szText, lpInfo->cSeparator);

            if (lpInfo->cDecimalSign != '.')
            {
               // replace the decimal character with a valid one '.'
               // because the normal floating point conversion routine works
               // with english system
               StrReplaceCh(szText, MAKEWORD('.', lpInfo->cDecimalSign));
            }

            StringToFloat(szText, &dfVal);
            dfVal *= nSign;

            if (dfVal != dfZero)
               fRetCode = CheckFloatRange(hWnd, dfVal);
         }
      }
      if (fRetCode)
      {
         if (!fInternal && dfVal == dfZero)
            lpInfo->fNegative = FALSE;

         FloatSetText(lpInfo->hWnd, lpInfo, dfVal, 1);

         if (dfVal == dfZero)
            {
            SendMessage(hWnd, WM_GETTEXT, sizeof(szText), (LPARAM)(LPTSTR)szText);
            if (lpszTemp = StrChr(szText, lpInfo->cDecimalSign))
               iDecimalPos = (int)(lpszTemp - (LPTSTR)szText);
            else
               iDecimalPos = lstrlen(szText);

            SendMessage(hWnd, EM_SETCARETPOS, iDecimalPos, 0l);
            }

         if (lpInfo->fEmpty)
         {
            NotifyParent(hWnd, EN_FIELDNONEMPTY);
            lpInfo->fEmpty = FALSE;
         }
         fRetCode = FALSE;
      }
      UnlockFloatHeader(hInfo);
   }

   if (fRetCode)
      fRetCode = (BOOL)CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, WM_SETTEXT,
                                      wParam, (LPARAM)lpszSave);
   else
      fRetCode = TRUE;

   return (fRetCode);
}

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³   Func: void  ProcessFloatWMKillFocus(HWND hWnd)                        ³
³                                                                         ³
³   Desc: In kill focus we check whether the val in the window is still   ³
³         valid. We notify the the parent (if any) and check their respond³
³         ; if non-zero then we do not lose the focus otherwise we ignore ³
³         continue.                                                       ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/

void  ProcessFloatWMKillFocus(HWND hWnd)
{
#ifdef  BUGS
// Bug-002
   TCHAR     szVal[330];
#else
   TCHAR     szVal[30];
#endif

   double   dfVal;

   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      if (lpInfo->bPostCount == 0)
      {
         // check if we currently in this call and prevent a ripple of
         // WM_KILLFOCUS
         SendMessage(hWnd, WM_GETTEXT, sizeof(szVal), (LPARAM)(LPTSTR)szVal);
         // BJO 23Jun98 LYW38 - Begin fix
         if (InSet(lpInfo->style, FS_MONEY))
            StrDeleteCh(szVal, lpInfo->cCurrencySign);
         if (InSet(lpInfo->style, FS_SEPARATOR))
            StrDeleteCh(szVal, lpInfo->cSeparator);
         StrReplaceCh(szVal, MAKEWORD('.', lpInfo->cDecimalSign));
         // BJO 23Jun98 LYW38 - End fix
         StringToFloat(szVal, &dfVal);
         if ((lpInfo->dfMinVal > dfVal) || (dfVal > lpInfo->dfMaxVal))
         {
            if (lpInfo->hWndParent)
            {
               NotifyParent(hWnd, EN_INVALIDDATA);
            }
         }
      }
      UnlockFloatHeader(hInfo);
   }
}

void  SendFloatPictureText(HWND hWnd)
{
   double dfVal;

   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      dfVal = dfZero;
      EditFieldModifed(hWnd);
      FloatSetText(hWnd, lpInfo, dfVal, 1);
      UnlockFloatHeader(hInfo);
   }
}

BOOL ProcessFloatEMCaretPos(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   LPEDITFIELD lpField;
   LPTSTR      lpszDecimalPos;
   LPTSTR      szText;
   int         iDecimalPos = -1;
   BOOL        fRet = 0;

   LockFloatHeader(lpInfo, hInfo, hWnd);

   lpField = LockField (hWnd);
   szText = lpField->lpszString;

   if (lpszDecimalPos = StrChr(szText, lpInfo->cDecimalSign))
      iDecimalPos = (int)(lpszDecimalPos - (LPTSTR)szText);

   UnlockField (hWnd);

   if (iDecimalPos != -1 && wParam > (WORD)iDecimalPos)
   {
      if (lParam)
         fRet = 0;

      else if (!GetOverstrikeMode(hWnd, FALSE))
         SetOverstrikeMode(hWnd, FALSE);
   }
   else
   {
      if (lParam)
         fRet = 1;

      else if (GetOverstrikeMode(hWnd, FALSE))
         SetInsertMode(hWnd, FALSE);
   }

   UnlockFloatHeader(hInfo);
   return (fRet);
}


BOOL  ProcessFloatWMKeyDown(HWND hWnd, WPARAM wParam)
{
   LPEDITFIELD lpField;
   LPTSTR      lpszDecimalPos;
#if defined(_WIN64) || defined(_IA64)
   LRESULT     lRet;
#else
   LONG        lRet;
#endif
   BOOL        fRetCode = FALSE;

#ifdef  BUGS
// Bug-002
   TCHAR        szText[330];
#else
   TCHAR        szText[40];
#endif

   short       iDecimalPos;
   int         iCaretPos;
   int         iLen;
   int         i;

   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
		lpInfo->wKeyDownVirtualKeyCode = wParam;

      if ((wParam == VK_DELETE && !IsShiftDown(hWnd)) || wParam == VK_BACK)
		{
			lRet = SendMessage(hWnd, EM_GETSEL, 0, 0l);
			// check if all of the text is hilited. if it is then we delete the
			// whole thing
			iCaretPos = 0;
			iLen = (int)SendMessage(hWnd, WM_GETTEXT, 40, (LPARAM)(LPTSTR)szText);
			// This is temporary!
			iLen = StrLen(szText);
			// the above line is temporary
			if (LOWORD(lRet) == (WORD)iCaretPos && (HIWORD(lRet)) == (WORD)iLen)
			{
				SendFloatPictureText(hWnd);
				Edit_SetSel(hWnd, 0, 0);         // Turn off highlight

				SendMessage (hWnd, WM_GETTEXT, sizeof(szText), (LPARAM)(LPTSTR)szText);

				if (lpszDecimalPos = StrChr(szText, '.'))
					iDecimalPos = (short)(lpszDecimalPos - (LPTSTR)szText);
				else
					iDecimalPos = lstrlen(szText);

				SendMessage(hWnd, EM_SETCARETPOS, iDecimalPos, 0L); // Pos before dec point
				fRetCode = TRUE;
			}
			else
			{
				iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0);

				if (GetOverstrikeMode(hWnd, LOCAL))     // Overstrike mode
					{
					szText[iCaretPos] = '0';
					SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szText);
					lpField = LockField(hWnd);
					PositionCaret(hWnd, lpField, iCaretPos);
					UnlockField(hWnd);
					}

				if (wParam == VK_BACK)
					--iCaretPos;
				if (iCaretPos < 39)
				{
				if (LOWORD(lRet) < HIWORD(lRet))
					{
					fRetCode = TRUE;

					for (i = (short)LOWORD(lRet); i < (short)HIWORD(lRet); i++)
						{
						if (isdigit(szText[i]))
							fRetCode = FALSE;

						else if ((TUCHAR)szText[i] == lpInfo->cDecimalSign)
							{
							fRetCode = TRUE;
							break;
							}
						}
					}

				else if (wParam == VK_BACK && GetOverstrikeMode(hWnd, FALSE))
					{
					lpField = LockField(hWnd);
					PositionCaret(hWnd, lpField, iCaretPos);
					UnlockField(hWnd);
					fRetCode = TRUE;
					}

				else
					fRetCode = !isdigit(szText[iCaretPos]);
				}

				// check if we are deleting a zero before a '.'
				if (!fRetCode && iCaretPos < iLen)
				{
	// RFW - 5/31/99 - SPR_CEL_006_002
					if (szText[iCaretPos] == '0' && (iCaretPos == 0 ||
						 (!isdigit(szText[iCaretPos-1]) &&
						  szText[iCaretPos-1] != lpInfo->cSeparator)))
						fRetCode = TRUE;
					/*
					else
						{
						CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, WM_KEYDOWN, wParam,
											0);

						iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0);
						SendMessage(hWnd, WM_GETTEXT, sizeof(szText),
										(LONG)(LPTSTR)szText);

						iCaretPosTemp = iCaretPos;
						if (szText[iCaretPosTemp] == '-')
							iCaretPosTemp++;

						if (InSet(lpInfo->style, FS_MONEY) &&
							 (TUCHAR)szText[iCaretPosTemp] == lpInfo->cCurrencySign)
							iCaretPosTemp++;

						if (iCaretPosTemp != iCaretPos)
							SendMessage(hWnd, EM_SETCARETPOS, iCaretPosTemp, 0L);

						fRetCode = TRUE;
						}
					*/
				}
			}
		}
      UnlockFloatHeader(hInfo);
   }
   return(fRetCode);
}


// RFW - 2/2/05 - 15673
void ProcessFloatWMKeyUp(HWND hWnd, WPARAM wParam)
{
LockFloatHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
	{
	lpInfo->wKeyDownVirtualKeyCode = 0;
   UnlockFloatHeader(hInfo);
	}
}


void  SetInitialCaretPosition (HWND hWnd)

// This function used to be called ProcessFloatWMSetFocus, but its name
// was changed because it needs to be called in response to WM_PAINT as
// well as WM_SETFOCUS.
//
// Randall.  4 Sep 91.

{
   LPEDITFIELD    lpField;
   LPTSTR          szText;
//   TCHAR          szText[40];
   register int   iIndex;
   register int   ch;
//   LONG           lSelection;
//   int            iStart, iEnd;


   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      lpField = LockField (hWnd);
      szText = lpField->lpszString;

      /* RFW 11/19/91 Replaced by lpField->lpszString for speed.
      szText[0] = '\0';
      SendMessage(hWnd, WM_GETTEXT, 0, (LONG)(LPTSTR)szText);
      */

      // now look for the the dot pos
      iIndex = 0;
      while ((ch = szText[iIndex]) && (TUCHAR)ch != lpInfo->cDecimalSign)
         ++iIndex;

      UnlockField (hWnd);


// The following block has been disabled so that the caret will be
// positioned to the left of the decimal point REGARDLESS OF WHETHER
// THE FIELD IS CURRENTLY SELECTED.  If this turns out to be inappropriate
// please uncomment this block of code and the commented declarations above.
//
// Randall.  4 Sep 91.
//
//      // If the field's selected, position the caret just before the dot.
//      lSelection = SendMessage (hWnd, EM_GETSEL, NULL, 0L);
//      iStart = LOWORD (lSelection);
//      iEnd = HIWORD (lSelection);
//      if (iStart <= iIndex && iIndex <= iEnd)

      // Position the caret just before the dot.
      SendMessage (hWnd, EM_SETCARETPOS, iIndex, 0L);

      if (lpInfo->bPostCount > 0)
         --lpInfo->bPostCount;

      UnlockFloatHeader(hInfo);
   }
}

void  ProcessWMIniChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
#ifdef  BUGS
// Bug-002
   TCHAR  szText[330];
#else
   TCHAR  szText[40];
#endif

   double   dbVal;

   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      FloatGetValue(hWnd, &dbVal);
      GetProfileString(WININTL, WIN_THOUSAND, WIN_DEFTHOUSAND, szText, 2);
      lpInfo->cSeparator = szText[0];
      GetProfileString(WININTL, WIN_DECIMAL, WIN_DEFDECIMAL, szText, 2);
      lpInfo->cDecimalSign = szText[0];
      GetProfileString(WININTL, WIN_CURRENCY, WIN_DEFCURRENCY, szText, 2);
      lpInfo->cCurrencySign = szText[0];
      UnlockFloatHeader(hInfo);
      StrPrintf(szText, _T("%f"), dbVal);
      ProcessFloatWMSetText(hWnd, 0, szText, FALSE, TRUE);
   }
}


BOOL ProcessEMSetFormat(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
LPFLOATFORMAT ff;
double        dfVal;

LockFloatHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
   {
   ff = (LPFLOATFORMAT)lParam;

#ifdef  BUGS
// Bug-004
   if (ff->cCurrencySign != 0)
       lpInfo->cCurrencySign = ff->cCurrencySign;
   else
       lpInfo->cCurrencySign = 92;

   if (ff->cDecimalSign != 0)
       lpInfo->cDecimalSign = ff->cDecimalSign;
   else
       lpInfo->cDecimalSign = 46;

   if (ff->cSeparator != 0)
       lpInfo->cSeparator = ff->cSeparator;
   else
       lpInfo->cSeparator = 44;
#else
   lpInfo->cCurrencySign = ff->cCurrencySign;
   lpInfo->cDecimalSign = ff->cDecimalSign;
   lpInfo->cSeparator = ff->cSeparator;
#endif

   FloatGetValue(hWnd, &dfVal);
   FloatSetValue(hWnd, dfVal);
   UnlockFloatHeader(hInfo);
   return (TRUE);
   }

return (FALSE);
}


BOOL  ProcessEMGetFormat(HWND hWnd, LPARAM lParam)
{
LPFLOATFORMAT ff;

LockFloatHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
   {
   ff = (LPFLOATFORMAT)lParam;
   ff->cCurrencySign = lpInfo->cCurrencySign;
   ff->cDecimalSign = lpInfo->cDecimalSign;
   ff->cSeparator = lpInfo->cSeparator;
   UnlockFloatHeader(hInfo);
   return (TRUE);
   }

return (FALSE);
}


#ifndef  IN_LINE

int   DLLENTRY StrDeleteCh(LPTSTR szStr, register int ch)
{
   register int   iNext = 0;
   LPTSTR    szPtr = szStr;

   while (*szStr)
   {
      if (*szStr == ch)
      {
         ++iNext;
         *szStr = *(szStr + iNext);       // go to the next char
      }
      else        // move any characters upwards
      {
         ++szStr;
         *szStr = *(szStr + iNext);
      }
   }
   return((int)(szStr - szPtr));
}

int   DLLENTRY StrReplaceCh(LPTSTR szStr, register WORD ch)
{
   LPTSTR    szPtr = szStr;

   while (*szStr)
   {
      if (*szStr == HIBYTE(ch))
         *szStr = LOBYTE(ch);
      ++szStr;
   }
   return((int)(szStr - szPtr));
}

#endif


void FloatSetText(hWnd, lpInfo, dfVal, fPassOnOnly)

HWND            hWnd;
LPEDITFLOATINFO lpInfo;
double          dfVal;
BOOL            fPassOnOnly;
{
#ifdef  BUGS
// Bug-002
TCHAR            szText[330];
#else
TCHAR            szText[64];
#endif

if (dfVal == dfZero)
   {
   if (lpInfo->fNegative)
      {
      szText[0] = '-';
      FloatFormat(lpInfo, &dfVal, &szText[1]);
      lpInfo->iPrevStrLen++;
      }

   else
      FloatFormat(lpInfo, &dfVal, szText);
   }

else
   FloatFormat(lpInfo, &dfVal, szText);

CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szText);
}


void FloatResetPrevLen(HWND hWnd)
{
LockFloatHeader(lpInfo, hInfo, hWnd);
lpInfo->iPrevStrLen = 0;
UnlockFloatHeader(hInfo);
}


#if 0
void FloatReplaceText(HWND hWnd, long lParam)
{
TCHAR       lpszLocal[MAXFIELD + 1];
LPEDITFIELD lpField;
LPTSTR      lpNewText = (LPTSTR)lParam;
LPTSTR      Ptr;
short       Len;

LockFloatHeader(lpInfo, hInfo, hWnd);
lpField = LockField(hWnd);

StrnnCpy(lpszLocal, lpField->lpszString, lpField->iHiStart + 1);
StrCat(lpszLocal, lpNewText);

if ((Ptr = StrChr(&lpField->lpszString[lpField->iHiStart],
                  lpInfo->cDecimalSign)) && (lpField->iHiEnd >
                  Ptr - (LPTSTR)lpField->lpszString))
   {
   StrCat(&lpszLocal[lstrlen(lpszLocal)],
          &lpField->lpszString[Ptr - (LPTSTR)lpField->lpszString]);

   Len = lpField->iHiEnd - (Ptr - (LPTSTR)lpField->lpszString + 1);

   if (Len > 0)
      {
      Ptr = StrChr(lpszLocal, lpInfo->cDecimalSign);
      _fmemset(Ptr + 1, '0', Len * sizeof(TCHAR));
      }
   }
else
   StrCat(lpszLocal, &lpField->lpszString[lpField->iHiEnd]);

StrCpy(lpNewText, lpszLocal);

UnlockField(hWnd);
UnlockFloatHeader(hInfo);
}
#endif


void FloatReplaceText(HWND hWnd, LPARAM lParam)
{
TCHAR       lpszLocal[MAXFIELD + 1];
LPEDITFIELD lpField;
LPTSTR      lpNewText = (LPTSTR)lParam;
LPTSTR      Ptr;
short       dDecPosNew = -1;
short       dDecPosString = -1;
short       iHiEnd;
BOOL        fHighlightLeftOfDec = FALSE;

LockFloatHeader(lpInfo, hInfo, hWnd);
lpField = LockField(hWnd);

if (InSet(lpInfo->style, FS_MONEY) && (TUCHAR)*lpNewText == lpInfo->cCurrencySign)
   lpNewText++;

_fmemset(lpszLocal, '\0', sizeof(lpszLocal));

/**********************************************
* Determine if new string has a decimal place
**********************************************/

if (Ptr = StrChr(lpNewText, lpInfo->cDecimalSign))
   dDecPosNew = (short)(Ptr - lpNewText);

/**********************************************
* Determine if old string has a decimal place
**********************************************/

if (Ptr = StrChr(&lpField->lpszString[lpField->iHiStart],
                 lpInfo->cDecimalSign))
   {
   fHighlightLeftOfDec = TRUE;

   if (lpField->iHiEnd > Ptr - (LPTSTR)lpField->lpszString)
      dDecPosString = (int)(Ptr - lpField->lpszString);
   }

StrnnCpy(lpszLocal, lpField->lpszString, lpField->iHiStart + 1);

iHiEnd = min(lpField->iHiEnd, (int)lstrlen(lpField->lpszString) - 1);

if (lpInfo->bFractionalWidth == 0 || (fHighlightLeftOfDec &&
    dDecPosString == -1))
   {
   StrnCat(lpszLocal, lpNewText, dDecPosNew >= 0 ? dDecPosNew :
           lstrlen(lpNewText));
   }

else if (fHighlightLeftOfDec && dDecPosString >= 0)
   {
   StrnCat(lpszLocal, lpNewText, dDecPosNew >= 0 ? dDecPosNew :
           lstrlen(lpNewText));

   lpszLocal[lstrlen(lpszLocal)] = lpInfo->cDecimalSign;

   if (dDecPosNew >= 0)
     FloatCopyDecDigits(&lpszLocal[lstrlen(lpszLocal)],
                        &lpNewText[dDecPosNew + 1], iHiEnd - dDecPosString);
//      StrPrintf(&lpszLocal[lstrlen(lpszLocal)], _T("%-0*d"), iHiEnd -
//                dDecPosString, StringToInt(&lpNewText[dDecPosNew + 1]));
   }

else if (!fHighlightLeftOfDec)
   FloatCopyDecDigits(&lpszLocal[lstrlen(lpszLocal)],
                      &lpNewText[dDecPosNew + 1], iHiEnd - lpField->iHiStart);
//   StrPrintf(&lpszLocal[lstrlen(lpszLocal)], _T("%-0*d"), iHiEnd -
//             lpField->iHiStart, StringToInt(&lpNewText[dDecPosNew + 1]));

lpField->iCurrentPos = lstrlen(lpszLocal);

if (lpField->iHiEnd < (int)lstrlen(lpField->lpszString))
  StrCat(lpszLocal, &lpField->lpszString[lpField->iHiEnd]);

if (lpInfo->bFractionalWidth && lpField->iCurrentPos == lstrlen(lpszLocal))
   lpField->iCurrentPos--;

StrCpy((LPTSTR)lParam, lpszLocal);

UnlockField(hWnd);
UnlockFloatHeader(hInfo);
}


// This function was added to handle copying text that have an
// inconsistent number of decimal places

void FloatCopyDecDigits(LPTSTR lpszDest, LPTSTR lpszSrc, int iLen)
{
// Old method - TIB35
//StrPrintf(lpszDest, _T("%-0*d"), iLen, StringToInt(lpszSrc));

_fmemset(lpszDest, '0', iLen);
lpszDest[iLen] = '\0';

while (iLen-- > 0 && *lpszSrc && isdigit(*lpszSrc))
   *lpszDest++ = *lpszSrc++;
}


/********************
* FloatIsValInRange
* RFW - 6/27/93
********************/

BOOL FloatIsValInRange(LPEDITFLOATINFO lpInfo, double dfVal)
{
BOOL fRet = FALSE;

if (lpInfo->dfMinVal <= dfVal && dfVal <= lpInfo->dfMaxVal)
   fRet = TRUE;

//  for case the range is 500 to 5000
//  we must return TRUE for 0 to 500
if (!fRet && lpInfo->dfMinVal > dfZero &&
    dfVal >= dfZero && dfVal < lpInfo->dfMinVal)
   fRet = TRUE;

 // converse
if (!fRet && lpInfo->dfMaxVal < dfZero &&
    dfVal <= dfZero && dfVal > lpInfo->dfMaxVal)
   fRet = TRUE;

return (fRet);
}
