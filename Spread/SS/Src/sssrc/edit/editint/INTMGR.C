/*
$Revision:   1.13  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/EDITINT/INTMGR.C_V  $
 *
 *    Rev 1.13   04 Dec 1990 13:44:14   Dirk
 * Sonny's changes
 *
 *    Rev 1.12   12 Jun 1990 14:10:44   Dirk
 * No change.
 *
 *    Rev 1.11   08 Jun 1990 12:13:38   Sonny
 * 1) Fixed the problem with toggling minus sign
 * 2) When deleting a negative value it now clears the field (i.e. zero)
 *    on the last character. before hand it leaves the minus sign floating
 *    around
 *
 *    Rev 1.10   07 Jun 1990 17:21:20   Sonny
 *
 *    Rev 1.9   06 Jun 1990 17:51:40   Randall
 * Set field to "0" when blank SETTEXT is received
 *
 *    Rev 1.8   04 Jun 1990 15:25:32   Randall
 * Removed MinWidth and MaxWidth.  Do all checking in terms of MinVal, MaxVal.
 * Allow negative numbers in SetText.
 *
 *    Rev 1.7   25 May 1990 17:58:00   Sonny
 *
 *    Rev 1.6   24 May 1990 17:24:02   Sonny
 *
 *    Rev 1.5   23 May 1990 16:50:02   Sonny
 *
 *    Rev 1.4   21 May 1990 13:28:46   Randall
 * No change.
 *
 *    Rev 1.3   15 May 1990 17:04:42   Sonny
 *
 *    Rev 1.2   14 May 1990 17:41:14   Randall
 *
 *    Rev 1.1   14 May 1990 15:02:58   Randall
 * Notify parent if Alloc fails
 *
 *    Rev 1.0   11 May 1990 16:14:18   Bruce
 * Initial revision.
 *
 *    Rev 1.2   07 May 1990 13:47:52   Randall
 * Fixed bug in SetText whereby null string was being sent to CheckRange
 *
 *    Rev 1.1   30 Apr 1990 13:21:16   Bruce
 * No change.
 *
 *    Rev 1.0   23 Apr 1990 18:41:56   Sonny
 * Initial revision.
*/


//---------*---------*---------*---------*---------*---------*---------*-----
//
// UPDATE LOG:
//
//		RWP01	10.14.96	<GRB2234> The spreadsheet is not validating
//							integers correctly
//
//---------*---------*---------*---------*---------*---------*---------*-----


#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <ctype.h>
#include <limits.h>
#include <windows.h>
#include <windowsx.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\classes\wintools.h"
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editint.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\classes\wintools.h"
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "editint\editint.h"
<-BORLAND*/
#endif

//#ifdef  BUGS
// Bug-001
extern void SS_BeepLowlevel(HWND hWnd);
//#endif

void IntSetText(HWND hWnd, LPEDITINTINFO lpInfo, long lVal);
BOOL IntIsValInRange(LPEDITINTINFO lpInfo, long lVal);

static   BOOL  GetNumberRange(LPEDITINTINFO lpInfo, LPTSTR szRange)
{
   LPTSTR szTmp;
   LONG  lMinVal;
   LONG  lMaxVal;
   int   ch;

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
      if (ch == '.')      // we have found one!
      {
         if (*++szTmp != '.')    // make sure the next one is also a '.'
            return(FALSE);
         break;
      }
      if (ch == ' ')
         break;
      ++szTmp;
   }
   if (!*szTmp)         // we have found none
      return(FALSE);
   ++szTmp;              // point to the next char
   // now szTmp points to the max range
   lMaxVal = StringToLong(szTmp);
   lpInfo->bMaxWidth = (BYTE) StrLen(szTmp);
   if (ch == '.')
      *(szTmp - 2) = '\0';    // zero it out
   else
      *(szTmp - 1) = '\0';    // zero it out
   lMinVal = StringToLong(szRange);
   lpInfo->bMinWidth = (BYTE) StrLen(szRange);
   if (lMinVal <= lMaxVal)
   {
      lpInfo->lMaxVal = lMaxVal;
      lpInfo->lMinVal = lMinVal;
#ifdef SPREAD_JPN
// 96' 12/10 Modified by BOC Gao. BUG0684
// When set Max = 0 and Min = -11, can not input 2 char's.
      if (lMaxVal <= 0l)
#else
      if (lMaxVal < 0l)
#endif
      {
         BYTE  bWidth;

         bWidth = lpInfo->bMinWidth;
         lpInfo->bMinWidth = lpInfo->bMaxWidth;
         lpInfo->bMaxWidth = bWidth;
      }
      return(TRUE);
   }
   return(FALSE);
}

BOOL  GetIntPicture(LPEDITINTINFO lpInfo, LPCTSTR szRange)
{
   int i;

   if (szRange)
   {
      lpInfo->bMaxWidth = (BYTE)StrLen(szRange);
      i = (int)lpInfo->bMaxWidth;
      lpInfo->lMaxVal = 1l;                   // lMaxVal =
      while (i--)
         lpInfo->lMaxVal *= 10l;     //    (10 raised to i)-1
      --lpInfo->lMaxVal;                  //
      lpInfo->lMinVal = -lpInfo->lMaxVal;
      lpInfo->bMinWidth = 1;
      return(TRUE);
   }
   else
      return(FALSE);
}


BOOL  ProcessIntWMNCCreate(HWND hWnd, LPARAM lParam)
{
   HANDLE         hMem;
   LPEDITINTINFO  lpTmp;
   BOOL           fRet = FALSE;

   if (!(hMem = GlobalAlloc(GHND, (DWORD)sizeof(EDITINTINFO))))
   {
      NotifyParent (hWnd, EN_ERRSPACE);
   }

   else
   {
      // Set the parent handler at offset 4 of the window structure
      SetDefWndProc(hWnd, GetTBEditClassProc());
      SetWindowIntField(hWnd, hMem);
      lpTmp = tbMemLock(hMem);
      if (lpTmp)
      {
         lpTmp->lInc = 1;

         if (((LPCREATESTRUCT)lParam)->style & WS_DISABLED)
            PostMessage(hWnd, WM_ENABLE, 0, 0L);

         if (((LPCREATESTRUCT)lParam)->style & IS_SPINWRAP)
            lpTmp->fSpinWrap = TRUE;
         else
            lpTmp->fSpinWrap = FALSE;

         lpTmp->fEmpty = TRUE;
         lpTmp->bPostCount = 0;
         lpTmp->lMinVal = LONG_MIN;
         lpTmp->lMaxVal = LONG_MAX;
         lpTmp->iWindowID = (LPARAM)((LPCREATESTRUCT)lParam)->hMenu;
         lpTmp->hWndParent = ((LPCREATESTRUCT)lParam)->hwndParent;
         // make the style to be right justified
         lpTmp->lVal = 0l;
         ((LPCREATESTRUCT)lParam)->style |= ES_RIGHT;
         GetIntPicture(lpTmp, (LPTSTR)((LPCREATESTRUCT)lParam)->lpszName);
         tbMemUnlock(hMem);
         fRet = TRUE;
      }
      else
         NotifyParent (hWnd, EN_ERRSPACE);
   }
   return(fRet);
}


void ProcessIntWMCreate(HWND hWnd, LPARAM lParam)
{
   LockIntHeader(lpInfo, hInfo, hWnd);

   if (GetWindowLong(hWnd, GWL_STYLE) & IS_SPIN)
      IntResetSpinBtn(hWnd, TRUE);

   UnlockIntHeader(hInfo);
}

static   BOOL  CheckIntRange(HWND hWnd, LPCTSTR szVal)
{
   int   iLen = StrLen(szVal);
   BOOL  fValid = FALSE;
   LONG  lVal;

   LockIntHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      lVal = StringToLong(szVal);
      if (!*szVal || (lpInfo->lMinVal <= lVal && lVal <= lpInfo->lMaxVal))
         fValid = TRUE;
   }
   UnlockIntHeader(hInfo);

   return(fValid);
}

void  ProcessIntWMDestroy(HWND hWnd)
{
   HANDLE        hMem;
   LPEDITINTINFO lpIntField;

   if (hMem = GetWindowIntField(hWnd))
      {
      lpIntField = (LPEDITINTINFO)GlobalLock(hMem);

      if (lpIntField->hWndSpinBtn)
         DestroyWindow(lpIntField->hWndSpinBtn);

      GlobalUnlock(hMem);
      GlobalFree(hMem);
      SetWindowIntField(hWnd, 0);
      }
}

BOOL  ProcessIntEMRange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   BOOL  fRetCode = FALSE;

   LockIntHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      fRetCode = GetNumberRange(lpInfo, (LPTSTR)lParam);
      UnlockIntHeader(hInfo);
   }
   return(fRetCode);
}


BOOL  ProcessIntWMChar(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   LPEDITFIELD lpField;
   BOOL  fDigit = (BOOL)isdigit((int)wParam);
   int   iCaretPos;
   LONG  lVal;
   BOOL  fRetCode = TRUE;
   /*
   TCHAR szText[30];
   */

   LockIntHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      if (wParam != VK_BACK && !fDigit && wParam != '-' && wParam != '+')
      {

//#ifdef  BUGS
// Bug-001
         SS_BeepLowlevel(hWnd);
//#else
//         MessageBeep(1);
//#endif

         return(FALSE);
      }

/* RFW 1/25/92 to allow a minus sign to be entered when zero
      if (lpInfo->lVal != 0L)
*/
      {

			lpField = LockField (hWnd);

         if (lpField->bHighlighted && (wParam == '-' || wParam == '+'))
         {
				TCHAR szText[64];
				WORD  wLen;
				long  lSel;

            SendMessage(hWnd, WM_GETTEXT, sizeof(szText),
                        (LPARAM)((LPTSTR)szText));
            wLen = StrLen (szText);
            lSel = (LONG) SendMessage(hWnd, EM_GETSEL, 0, 0l);
            if (LOWORD(lSel) == 0 && (HIWORD(lSel)) == wLen)
            {
               lpInfo->fNegative = FALSE;
               EditFieldModifed(hWnd);
               lpInfo->lVal = 0;
               IntSetText(hWnd, lpInfo, lpInfo->lVal);
               if (lpField->bHighlighted) DeselectText (hWnd, lpField);
               SendMessage(hWnd, EM_SETCARETPOS, 1, 0L);
            }
         }

			UnlockField (hWnd);

         if (wParam == '-')
         {
            if (lpInfo->lMinVal >= 0l)        // min val is greater than zero
//#ifdef  BUGS
// Bug-001
               SS_BeepLowlevel(hWnd); 
//#else
//               MessageBeep(1);
//#endif
            else
            {
               iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);   // Pos before dec point
               if (lpInfo->lVal == 0l)
                  {
                  if (lpInfo->fNegative)
                     {
                     if (iCaretPos > 0)
                        --iCaretPos;   // Going + so decrease the count;
                     }
                  else
                     ++iCaretPos;      // Going + so decrease the count;

                  lpInfo->fNegative = !lpInfo->fNegative;
                  }

               else if (lpInfo->lVal < 0l)
               {
                  if (iCaretPos > 0)
                     --iCaretPos;            // Going + so decrease the count;
               }
               else
                  ++iCaretPos;            // Going + so decrease the count;

               lVal = -lpInfo->lVal;

               if (IntIsValInRange(lpInfo, lVal)) // RFW - 6/27/93
               {
                  lpInfo->lVal = lVal;
                  EditFieldModifed(hWnd);
                  IntSetText(hWnd, lpInfo, lpInfo->lVal);
                  SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);   // Pos before dec point
               }
               // else out of range
            }
            fRetCode = FALSE;
         }

         else if (wParam == '+')
         {
            lpInfo->fNegative = FALSE;

            if (lpInfo->lMaxVal < 0L)        // max val is less than zero
            {
//#ifdef  BUGS
// Bug-001
               SS_BeepLowlevel(hWnd);
//#else
//               MessageBeep(1);
//#endif
               fRetCode = FALSE;
            }
            else
            if (lpInfo->lVal < 0L)
            {
               iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);   // Pos before dec point
               lVal = -lpInfo->lVal;
               if (iCaretPos > 0)
                  --iCaretPos;
               if (IntIsValInRange(lpInfo, lVal)) // RFW - 6/27/93
               {
                  EditFieldModifed(hWnd);
                  lpInfo->lVal = lVal;
                  IntSetText(hWnd, lpInfo, lpInfo->lVal);
                  SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);   // Pos before dec point
               }
               // else out of range
            }
            fRetCode = FALSE;
         }

      else if (lpInfo->lVal == 0L && fDigit && !GetOverstrikeMode(hWnd, FALSE))
         {
            lVal = (LONG)(wParam - '0');
            iCaretPos = 1;

            if (lpInfo->fNegative)
               {
               iCaretPos++;
               lVal *= -1;
               }

            fRetCode = FALSE;

            if (IntIsValInRange(lpInfo, lVal)) // RFW - 6/27/93
               {
               lpInfo->lVal = lVal;

               EditFieldModifed(hWnd);
               IntSetText(hWnd, lpInfo, lpInfo->lVal);
               SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);
               fRetCode = FALSE;
               }

            else if (lpInfo->hWndParent)
               NotifyParent(hWnd, EN_INVALIDCHAR);
         }
      }
      UnlockIntHeader(hInfo);
   }
   return(fRetCode);
}

LONG  ProcessIntEMValidate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   LPCTSTR     szVal = (LPTSTR)lParam;
   LPCTSTR     szMinus;
   LPEDITFIELD lpField;
   LONG        lVal;
   BOOL        fRetCode = FALSE;

//RWP01d
// int         iLenTemp;
//RWP01d

   int         iLen;
   int         iCaretPos;

   LockIntHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      lpField = LockField(hWnd);

      if (!szVal)
         szVal = lpField->lpszString;

      for (szMinus = szVal; szMinus = StrChr(szMinus, '-'); szMinus++)
			if (szMinus && szMinus != szVal)
			{
				UnlockIntHeader(hInfo);
				return(fRetCode);
			}

      iLen = StrLen(szVal);
      lVal = StringToLong(szVal);

      if (IntIsValInRange(lpInfo, lVal)) // RFW - 6/27/93
         fRetCode = TRUE;

//RWP01c
//---------*---------
//    iLenTemp = iLen;
//    if (szMinus)
//       iLenTemp--;
//
//    if (iLenTemp > (short)lpInfo->bMaxWidth)
//       fRetCode = FALSE;
//---------*---------
	  //Modify by BOC 99.6.22 (hyt)---------------------------
	  //for min&max value all is negative such as -50&-999
	  //can't input -100
      //if ((szMinus && (iLen > (short)lpInfo->bMinWidth)) || (!szMinus && (iLen > (short)lpInfo->bMaxWidth)))
	  if ((szMinus && (iLen > (short)(lpInfo->lMaxVal<0?lpInfo->bMaxWidth:lpInfo->bMinWidth))) || (!szMinus && (iLen > (short)lpInfo->bMaxWidth)))
         fRetCode = FALSE;
	  //--------------------------------------------------------
//RWP01c

      if (fRetCode)
         lpInfo->lVal = lVal;

      if (!fRetCode)
      {
         if (lpInfo->hWndParent)
            NotifyParent(hWnd, EN_INVALIDCHAR);
#ifdef TBPRO
         lpInfo->bPostCount = 2;
#endif

//#ifdef  BUGS
// Bug-001
         SS_BeepLowlevel(hWnd);
//#else
//         MessageBeep(1);
//#endif
      }
      if ((iLen == 0) || (lVal == 0l))    // if empty then we must post a message to the parent
      {
         if (!lpInfo->fEmpty)
         {
            if (lpInfo->hWndParent)
               NotifyParent(hWnd, EN_FIELDEMPTY);
            lpInfo->fEmpty = TRUE;
         }
         iCaretPos = (short)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);

         // Force the field to display '0' instead of blank.
         // Must Post, not Send, because the parent class still has a
         // blank string for this field, which it will paint with when
         // we return from here.  We want that PAINT to happen before
         // the SETTEXT, not the other way around.

         EditFieldModifed(hWnd);
         lpInfo->fNegative = FALSE;
         IntWMSetText(hWnd, (LPARAM)(LPCTSTR)"0");

         /*
         if (*szVal == '-')
            --iCaretPos;
         SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);
         */

         lpField->bHighlighted = FALSE;
         lpInfo->lVal = 0l;
         fRetCode = FALSE;  // I don't think this is right.  RJS  11Apr91
      }
      else
      if (lpInfo->fEmpty)
      {
         NotifyParent(hWnd, EN_FIELDNONEMPTY);
         lpInfo->fEmpty = FALSE;
      }
      UnlockField(hWnd);
      UnlockIntHeader(hInfo);
   }
   return(fRetCode);
}


BOOL ProcessIntWMSetText(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   LPTSTR lpszStr = (LPTSTR)lParam;
   int    ch;
   BOOL   fRetCode = TRUE;

   if (wParam == 0)
   {
      LockIntHeader(lpInfo, hInfo, hWnd);
      if (lpszStr)
      {
         while (*lpszStr == ' ')
            lpszStr++;

         if (*lpszStr == '-')
            ++lpszStr;     // Leading minus sign. Ok.

         while (ch = *lpszStr)
         {
            if (!isdigit(ch))
            {
               fRetCode = FALSE;
               break;
            }
            ++lpszStr;
         }

         lpszStr = (LPTSTR)lParam;
      }

      if (fRetCode)
      {
         fRetCode = CheckIntRange(hWnd, lpszStr);
         if (fRetCode)
            lpInfo->lVal = StringToLong(lpszStr);
      }

      // Blank String: Set to "0"
      if (fRetCode)
      {
         if (*lpszStr == '\0')
         {
            lpInfo->fNegative = FALSE;
            EditFieldModifed(hWnd);
            IntWMSetText(hWnd, (LPARAM)(LPCTSTR)"0");
            fRetCode = FALSE;
         }
      }

      UnlockIntHeader(hInfo);
   }
   return(fRetCode);
}

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³   Func: void  ProcessIntWMKillFocus(HWND hWnd)                          ³
³                                                                         ³
³   Desc: In kill focus we check whether the val in the window is still   ³
³         valid. We notify the the parent (if any) and check their respond³
³         ; if non-zero then we do not lose the focus otherwise we ignore ³
³         continue.                                                       ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/

void  ProcessIntWMKillFocus(HWND hWnd, WPARAM wParam)
{
   TCHAR szVal[30];
   LONG  lVal;
   int   iNotifyCode;

   LockIntHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      // check if this kill focus is caused by the programmer displaying
      // a message to the user that this field is invalid. If post count is no
      // zero then it is so don't send another one!
      if (lpInfo->bPostCount == 0)
      {
         SendMessage(hWnd, WM_GETTEXT, sizeof(szVal), (LPARAM)(LPTSTR)szVal);
         lVal = StringToLong(szVal);
         if ((lpInfo->lMinVal > lVal) || (lVal > lpInfo->lMaxVal))
         {
            if (lpInfo->hWndParent)
            {
               NotifyParent(hWnd, EN_INVALIDDATA);
            }
         }
         if (lpInfo->hWndParent)
         {
           iNotifyCode = StrLen(szVal) == 0 ? EN_FIELDEMPTY : EN_FIELDNONEMPTY;
           NotifyParent(hWnd, iNotifyCode);
         }
      }
      UnlockIntHeader(hInfo);
   }
}

void ProcessIntWMSetFocus(HWND hWnd)
{
   LockIntHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      if (lpInfo->bPostCount > 0)
         --lpInfo->bPostCount;
      UnlockIntHeader(hInfo);
   }
}

BOOL  ProcessIntWMKeyDown(HWND hWnd, WPARAM wParam)
{
   BOOL  fRetCode = TRUE;
   TCHAR szText[40];
   int   iCaretPos;

   // LockIntHeader is a macro defined in editint.h
   LockIntHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      if ((wParam == VK_BACK) || (wParam == VK_DELETE && !IsShiftDown(hWnd)))
      {
         iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0);
         if (iCaretPos > 0 && wParam == VK_BACK)
            --iCaretPos;
         SendMessage(hWnd, WM_GETTEXT, 40, (LPARAM)(LPTSTR)szText);
         if (iCaretPos < 39)
         {
            if (szText[iCaretPos] == '-')       // Can't delete minus sign
            {
               fRetCode = FALSE;
//#ifdef  BUGS
// Bug-001
               SS_BeepLowlevel(hWnd); 
//#else
//               //MessageBeep(1);
//#endif
            }
         }
      }
      else if (wParam == VK_DOWN)
      {
         IntVScroll(hWnd, SB_LINEDOWN);
         fRetCode = FALSE;
      }
      else if (wParam == VK_UP)
      {
         IntVScroll(hWnd, SB_LINEUP);
         fRetCode = FALSE;
      }

      UnlockIntHeader(hInfo);
   }
   return(fRetCode);
}


void IntSize(HWND hWnd, LPARAM lParam)
{
LPEDITFIELD lpField;

LockIntHeader(lpInfo, hInfo, hWnd);
lpField = LockField(hWnd);

if (lpInfo->hWndSpinBtn)
   IntResetSpinBtn(hWnd, TRUE);

else
   lpField->xRightNonClientXtra = 0;

UnlockField(hWnd);
UnlockIntHeader(hInfo);
}

#ifdef SS_V80
void IntSetSpinStyle (HWND hWnd, BOOL fEnhanced)

{
	LPEDITFIELD lpField = LockField(hWnd);
	lpField->fEnhanced = fEnhanced;
	UnlockField(hWnd);
}
#endif
void IntResetSpinBtn(HWND hWnd, BOOL fShowSpin)
{
LPEDITFIELD lpField;
RECT        Rect;
short       SBWidth = 0;
short       SBHeight = 0;

LockIntHeader(lpInfo, hInfo, hWnd);
lpField = LockField(hWnd);

GetClientRect(hWnd, &Rect);

if (fShowSpin)
   {
//   SBHeight = (short)(Rect.bottom - Rect.top);
#ifdef SD40
	SBHeight = (short)min(Rect.bottom - Rect.top, (lpField->yHeight * 3) / 2)-2;
#else
	SBHeight = (short)min(Rect.bottom - Rect.top, (lpField->yHeight * 3) / 2);
#endif
   SBWidth = (SBHeight * 2) / 3;
   }

lpField->xRightNonClientXtra = SBWidth;

if (lpField->f3D)
   {
   tbShrinkRect(&Rect, lpField->Edit3DFormat.dShadowSize,
                lpField->Edit3DFormat.dShadowSize);

   if (lpField->Edit3DFormat.wStyle & EDIT3D_INSIDEBORDER)
      tbShrinkRect(&Rect, 1, 1);
   }

if (fShowSpin && !lpInfo->hWndSpinBtn)
   {
   lpInfo->hWndSpinBtn = CreateWindow(lpszSpinClassName, NULL, WS_CHILD |
                                      WS_VISIBLE | SBNS_BORDERLEFT,
                                      Rect.right - SBWidth, Rect.top,
                                      SBWidth, SBHeight, hWnd, 0,
                                      GetWindowInstance(hWnd), NULL);

   if (GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC)
      EnableWindow(lpInfo->hWndSpinBtn, FALSE);
   }

else if (!fShowSpin && lpInfo->hWndSpinBtn)
   {
   DestroyWindow(lpInfo->hWndSpinBtn);
   lpInfo->hWndSpinBtn = 0;
   }

else if (fShowSpin && lpInfo->hWndSpinBtn)
   MoveWindow(lpInfo->hWndSpinBtn, Rect.right - SBWidth, Rect.top, SBWidth,
              SBHeight, TRUE);

if (lpInfo->hWndSpinBtn)
  SendMessage(lpInfo->hWndSpinBtn , EM_SETSPINSTYLE, 0, (LPARAM)lpField->fEnhanced);
InvalidateRect(hWnd, NULL, TRUE);
UpdateWindow(hWnd);

UnlockField(hWnd);
UnlockIntHeader(hInfo);
}


void IntVScroll(HWND hWnd, int nScrollCode)
{
long lNumOld;
long lNum;
long lStyle;

LockIntHeader(lpInfo, hInfo, hWnd);

if (!lpInfo->hWndSpinBtn || (nScrollCode != SB_LINEDOWN &&
    nScrollCode != SB_LINEUP))
   {
   UnlockIntHeader(hInfo);
   return;
   }

lStyle = GetWindowLong(hWnd, GWL_STYLE);

IntGetValue(hWnd, &lNum);
lNumOld = lNum;

switch (nScrollCode)
   {
   // RDW - Rewrite to prevent overflow
   case SB_LINEUP:
      if (lNum < lpInfo->lMaxVal - lpInfo->lInc)
         lNum += lpInfo->lInc;
      else if (lNum < lpInfo->lMaxVal)
         lNum = lpInfo->lMaxVal;
      else if (lpInfo->fSpinWrap) // && lNum = lpInfo->lMaxVal (implied)
         lNum = lpInfo->lMinVal;
      /*
      else
         MessageBeep(1);
      */
      break;

   // RDW - Rewrite to prevent overflow
   case SB_LINEDOWN:
      if (lNum > lpInfo->lMinVal + lpInfo->lInc)
         lNum -= lpInfo->lInc;
      else if (lNum > lpInfo->lMinVal)
         lNum = lpInfo->lMinVal;
      else if (lpInfo->fSpinWrap) // && lNum = lpInfo->lMinVal (implied)
         lNum = lpInfo->lMaxVal;
      /*
      else
         MessageBeep(1);
      */
      break;

   }

if (lNumOld != lNum)
   {
   EditFieldModifed(hWnd);
   IntSetValue(hWnd, lNum);
   UpdateWindow(hWnd);
   }

UnlockIntHeader(hInfo);
}


void IntSetText(hWnd, lpInfo, lVal)

HWND          hWnd;
LPEDITINTINFO lpInfo;
long          lVal;
{
TCHAR         szText[64];

if (lVal == 0L)
   {
   if (lpInfo->fNegative)
      {
      szText[0] = '-';
      LongToString(lVal, &szText[1]);
      }

   else
      LongToString(lVal, szText);
   }

else
   LongToString(lVal, szText);

IntWMSetText(hWnd, (LPARAM)(LPTSTR)szText);
}

#if defined(_WIN64) || defined(_IA64)
LRESULT IntWMSetText(hWnd, lParam)
#else
long IntWMSetText(hWnd, lParam)
#endif

HWND hWnd;
LPARAM lParam;
{
if (!ProcessIntWMSetText(hWnd, 1, lParam))
   return(1l);

return (CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, WM_SETTEXT, 0, lParam));
}


void IntEnable(HWND hWnd, WPARAM wParam)
{
LockIntHeader(lpInfo, hInfo, hWnd);

if (lpInfo->hWndSpinBtn)
   EnableWindow(lpInfo->hWndSpinBtn, (BOOL)wParam);

UnlockIntHeader(hInfo);
}


/******************
* IntIsValInRange
* RFW - 6/27/93
******************/

BOOL IntIsValInRange(LPEDITINTINFO lpInfo, long lVal)
{
BOOL fRet = FALSE;

if (lpInfo->lMinVal <= lVal && lVal <= lpInfo->lMaxVal)
   fRet = TRUE;

//  for case the range is 500 to 5000
//  we must return TRUE for 0 to 500
if (!fRet && lpInfo->lMinVal > 0L && lVal >= 0L && lVal < lpInfo->lMinVal)
   fRet = TRUE;

 // converse
if (!fRet && lpInfo->lMaxVal < 0L && lVal <= 0L && lVal > lpInfo->lMaxVal)
   fRet = TRUE;

return (fRet);
}
