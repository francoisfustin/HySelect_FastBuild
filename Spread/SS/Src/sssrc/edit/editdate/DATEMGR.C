/*
$Revision:   1.17  $
*/

/*
$Log:   G:/DRLIBS/TOOLBOX/EDIT/EDITDATE/DATEMGR.C_V  $
 * 
 *    Rev 1.17   10 Jul 1990 10:04:42   Sonny
 * Just did nothing
 * 
 *    Rev 1.16   12 Jun 1990 14:09:10   Dirk
 * No change.
 * 
 *    Rev 1.15   29 May 1990  9:17:24   Randall
 * Made external objects "static".
 * 
 *    Rev 1.14   28 May 1990 15:27:14   Randall
 * Split validation stuff off into Static.c and Dynamic1.c and Dynamic2.c
 * 
 *    Rev 1.13   24 May 1990 17:32:56   Randall
 * Added LoadCurrentDate
 * 
 *    Rev 1.12   24 May 1990 11:02:02   Randall
 * Added DateSet(Get)Format
 * 
 *    Rev 1.8   21 May 1990 16:23:16   Randall
 * Added Date Range setting and checking
 * 
 *    Rev 1.6   17 May 1990 14:19:08   Randall
 * Finalized year range in Dynamic Validate: Will accept 0001 to 2099.
 * 
 *    Rev 1.5   16 May 1990 16:17:50   Randall
 * Fixed bug in DateInitializeField: Unlocked too soon.
 * 
 *    Rev 1.2   14 May 1990 15:02:14   Randall
 * Notify parent if Alloc fails
 * 
 *    Rev 1.1   14 May 1990 11:16:36   Randall
 * Fixed bug in the Month of DD-MON-YY fields when the field is initially empty.
 * 
 *    Rev 1.0   11 May 1990 16:13:10   Bruce
 * Deleted logfile from the server.
 * 
 *    Rev 1.2   01 May 1990 13:10:58   Randall
 * Changed the way I get the date format from the edit field style.
 * 
 *    Rev 1.0   23 Apr 1990 18:40:42   Sonny
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <toolbox.h>
#include <ctype.h>
#include <stdlib.h>

#include "..\..\classes\wintools.h"
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editdate.h"
#include "..\..\spread\spread.h"        //- Added by Masanori Iwasa at FarPoint (SPREAD_JPN)

//----------------------------Internal Functions----------------------------//
static void FreeDateField (HWND);
//--------------------------------------------------------------------------//

BOOL  DateCreateField (HWND hWnd, LPARAM lParam)
{
   LPDATEFIELD lpField;
   HANDLE      hDate;
   LPCTSTR     Ptr;
   LONG        lStyle;
   LONG        lFormat;
   TCHAR       szTemp[20];
   int         nWinIniStyle;
   BOOL        fRetCode = FALSE;

   if (!(hDate = GlobalAlloc (GHND, (DWORD) sizeof(DATEFIELD))))
   {
      NotifyParent (hWnd, EN_ERRSPACE);
   }

   else
   {
      lpField = (LPDATEFIELD) GlobalLock (hDate);
      if (lpField)
      {
         SetDefWndProc(hWnd, GetTBEditClassProc());
         SetWindowDateField (hWnd, hDate);

         if (((LPCREATESTRUCT)lParam)->style & WS_DISABLED)
            PostMessage(hWnd, WM_ENABLE, 0, 0L);

         lStyle = ((LPCREATESTRUCT) lParam)->style;
         //----------------------------------------------------------------
         // In the Japanese version the 'ES_UPPERCASE' flag cannot be
         // used, because the 2nd byte of DBCS separators uses characters
         // that are in the lower case range. I fixed this by responding
         // to the WM_CHAR message. (Masanori Iwasa)
         //----------------------------------------------------------------
         lStyle |= ES_OVERSTRIKE;
         ((LPCREATESTRUCT) lParam)->style = lStyle;
         lpField->hParent = ((LPCREATESTRUCT) lParam)->hwndParent;

         lpField->df.nFormat = IDF_DDMONYY;       // Default date format;
         lFormat = lStyle & 0xE000L;      // Isolate the 3 format bits;

         if (lFormat == DS_DDMONYY)
            lpField->df.nFormat = IDF_DDMONYY;
         else if (lFormat == DS_DDMMYY)
            lpField->df.nFormat = IDF_DDMMYY;
         else if (lFormat == DS_MMDDYY)
            lpField->df.nFormat = IDF_MMDDYY;
         else if (lFormat == DS_YYMMDD)
            lpField->df.nFormat = IDF_YYMMDD;
         else                 // Default: Consult win.ini
         {
            nWinIniStyle = (BOOL) GetProfileInt (_T("intl"), _T("iDate"), -1);
            switch (nWinIniStyle)
            {
               case -1:    // Our own default.  Not supported by Windows.
                  lpField->df.nFormat = IDF_DDMONYY;
                  break;

               case 0:
                  lpField->df.nFormat = IDF_MMDDYY;
                  break;

               case 1:
                  lpField->df.nFormat = IDF_DDMMYY;
                  break;

               case 2:
                  lpField->df.nFormat = IDF_YYMMDD;
                  break;
            }
         }

         GetProfileString (_T("intl"), _T("sDate"), DEFAULT_SEPARATOR, szTemp, 2);
         if (szTemp[0])
            lpField->df.cSeparator = szTemp[0];
         else                                   // Windows seems to return NULL 
            lpField->df.cSeparator = ' ';           //    if you specified a space.

         lpField->df.bCentury = (lStyle & DS_CENTURY) != 0;

         if (lStyle & DS_DEFCENTURY)
            {
            lpField->df.bCentury = 0;

            GetProfileString(_T("intl"), _T("sShortDate"), _T("M/d/yy"), szTemp, 11);
            if (Ptr = StrChr(szTemp, 'y'))
               if (StriStr(Ptr, _T("yyyy")))
                  lpField->df.bCentury = 1;
            }

         lpField->lRangeMin = UNDEFINED;
         lpField->lRangeMax = UNDEFINED;
         lpField->nTwoDigitYearMax = TWODIGITYEARMAX_DEF;

         GlobalUnlock (hDate);
         fRetCode = TRUE;
      }
      else
         NotifyParent (hWnd, EN_ERRSPACE);
   }

   return(fRetCode);
}

//--------------------------------------------------------------------------//

void DateInitializeField (HWND hWnd, LPARAM lParam)

{
   LPDATEFIELD lpField;
   TB_DATE     Date;
   TCHAR       szDate[DATELENGTH];

   lpField = LockDateField (hWnd);

   if (((LPCREATESTRUCT)lParam)->lpszName)
      StrnnCpy(szDate, (LPTSTR)((LPCREATESTRUCT)lParam)->lpszName,
               sizeof (szDate));
   else
      szDate[0] = '\0';

   if (StrCmp (StrTrim (szDate), _T("")) == 0)
   {
      SysGetDate (&Date.nDay, &Date.nMonth, &Date.nYear);
      DateDMYToString (&Date, szDate, &lpField->df);
   }

   if (GetWindowLong(hWnd, GWL_STYLE) & DS_SPIN)
      {
      lpField->df.bSpin = TRUE;
      DateResetSpinBtn(hWnd, TRUE);
      }

   // Send the string to the Edit Field processor
   if (DateStringIsValidEx(szDate, &lpField->df, lpField->nTwoDigitYearMax))
      SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szDate);

   SetOverstrikeMode(hWnd, FALSE);
   UnlockDateField(hWnd);
}

//--------------------------------------------------------------------------//
#ifdef SS_V80
void DateSetSpinStyle (HWND hWnd, BOOL fEnhanced)

{
	LPEDITFIELD lpField = LockField(hWnd);
	lpField->fEnhanced = fEnhanced;
	UnlockField(hWnd);
}
#endif
BOOL DateSetValidRange (HWND hWnd, LPARAM lParam)

{
   LPCTSTR     lpszString = (LPCTSTR)lParam;
   LPCTSTR     Ptr;
   LPDATEFIELD lpField;
   LONG        lMin = 0L;
   LONG        lMax = 0L;
   BOOL        Ok; 

   lpField = LockDateField (hWnd);

   /*
   StrScanf (lpszString, "%ld %ld", (LPLONG) &lMin, (LPLONG) &lMax);
   */
   lMin = StringToLong(lpszString);

   if (Ptr = StrChr(lpszString, ' '))
      lMax = StringToLong(Ptr + 1);

   if (lMin <= lMax)
   {
      lpField->lRangeMin = lMin;
      lpField->lRangeMax = lMax;
      Ok = TRUE;
   }

   else
      Ok = FALSE;

   UnlockDateField (hWnd);

   return Ok;
}

//--------------------------------------------------------------------------//

void DateSetFieldFormat (HWND hWnd, LPARAM lParam)

{
   LPDATEFIELD  lpField;
   LPDATEFORMAT df;

   lpField = LockDateField (hWnd);

   df = (LPDATEFORMAT)lParam;

   if (IsCharAlphaNumeric(df->cSeparator) || df->cSeparator == '\0')
      {
      UnlockDateField (hWnd);
      return;
      }

   // Store the new format
   MemCpy(&lpField->df, (LPDATEFORMAT)lParam, sizeof(DATEFORMAT));

   DateResetSpinBtn(hWnd, lpField->df.bSpin);

   SetWindowText(hWnd,_T(""));
   InvalidateRect(hWnd, NULL, TRUE);
   UpdateWindow(hWnd);

   UnlockDateField (hWnd);
}

//--------------------------------------------------------------------------//

BOOL DateGetFieldFormat (HWND hWnd, LPARAM lParam)

{
   LPDATEFIELD lpField;

   lpField = LockDateField (hWnd);

   MemCpy((LPVOID)lParam, (LPVOID)&lpField->df, sizeof(DATEFORMAT));

   UnlockDateField (hWnd);

   return (TRUE);
}

//--------------------------------------------------------------------------//

void  DateDestroyField (HWND hWnd)

{
   FreeDateField (hWnd);
}

//--------------------------------------------------------------------------//

LPDATEFIELD LockDateField (HWND hWnd)

{
   HANDLE hField;

   if (hField = GetWindowDateField (hWnd))
      return (LPDATEFIELD) GlobalLock (hField);
   else
      return NULL;
}

//--------------------------------------------------------------------------//

void UnlockDateField (HWND hWnd)

{
   HANDLE hField;

   if (hField = GetWindowDateField (hWnd))
      GlobalUnlock (hField);
}

//--------------------------------------------------------------------------//

void FreeDateField (HWND hWnd)

{
   LPDATEFIELD lpDateField;
   HANDLE      hField;

   if (IsWindow(hWnd) && (hField = GetWindowDateField (hWnd)))
   {
      lpDateField = (LPDATEFIELD)GlobalLock(hField);

      if (lpDateField->hWndSpinBtn)
         DestroyWindow(lpDateField->hWndSpinBtn);

      GlobalUnlock(hField);
      tbMemFree(hField);
      SetWindowDateField(hWnd, 0);
   }
}

//--------------------------------------------------------------------------//

void LoadCurrentDate(HWND hWnd)

{
   LPDATEFIELD lpField;
   TB_DATE     Date;
   TCHAR       szDate[DATELENGTH];
   long        lDate;

   lpField = LockDateField (hWnd);

   SysGetDate (&Date.nDay, &Date.nMonth, &Date.nYear);

   // RFW - 4/29/99 - SPR_CEL_004_010

   // Decode the date to be tested
   lDate = DateDMYToJulian (&Date);

   if (lpField->lRangeMin != UNDEFINED && lpField->lRangeMax != UNDEFINED &&
       (lDate < lpField->lRangeMin || lDate > lpField->lRangeMax))
      ; // Date is out of Range
   else
      {
      DateDMYToString (&Date, szDate, &lpField->df);
      SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szDate);
      }

   UnlockDateField (hWnd);
}


void DateSize(HWND hWnd, LPARAM lParam)
{
LPDATEFIELD lpDateField;
LPEDITFIELD lpField;

lpDateField = LockDateField(hWnd);
lpField = LockField(hWnd);

if (lpDateField->hWndSpinBtn)
   DateResetSpinBtn(hWnd, TRUE);
else
   lpField->xRightNonClientXtra = 0;

UnlockField(hWnd);
UnlockDateField(hWnd);
}


void DateResetSpinBtn(HWND hWnd, BOOL fShowSpin)
{
LPDATEFIELD lpDateField;
LPEDITFIELD lpField;
RECT        Rect;
short       SBWidth = 0;
short       SBHeight = 0;

lpDateField = LockDateField(hWnd);
lpField = LockField(hWnd);

GetClientRect(hWnd, &Rect);

if (fShowSpin)
   {
//   SBHeight = (short)(Rect.bottom - Rect.top);
   SBHeight = (short)min(Rect.bottom - Rect.top, (lpField->yHeight * 3) / 2);
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

if (fShowSpin && !lpDateField->hWndSpinBtn)
   {
   lpDateField->hWndSpinBtn = CreateWindow(lpszSpinClassName, NULL, WS_CHILD |
                                           WS_VISIBLE | SBNS_BORDERLEFT,
                                           Rect.right - SBWidth, Rect.top,
                                           SBWidth, SBHeight, hWnd, 0,
                                           GetWindowInstance(hWnd), NULL);

  if (GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC)
      EnableWindow(lpDateField->hWndSpinBtn, FALSE);
   }

else if (!fShowSpin && lpDateField->hWndSpinBtn)
   {
   DestroyWindow(lpDateField->hWndSpinBtn);
   lpDateField->hWndSpinBtn = 0;
   }

else if (fShowSpin && lpDateField->hWndSpinBtn)
   MoveWindow(lpDateField->hWndSpinBtn, Rect.right - SBWidth, Rect.top,
              SBWidth, SBHeight, TRUE);

if (lpDateField->hWndSpinBtn)
	SendMessage(lpDateField->hWndSpinBtn , EM_SETSPINSTYLE, 0, (LPARAM)lpField->fEnhanced);
UnlockField(hWnd);
UnlockDateField(hWnd);
}


void DateVScroll(HWND hWnd, int nScrollCode)
{
LPEDITFIELD lpField;
LPDATEFIELD lpDateField;
TB_DATE     Date;
TCHAR       szDate[MAXFIELD + 1];
short       dMax;
short       iCursorPos;

lpDateField = LockDateField(hWnd);

if (!lpDateField->hWndSpinBtn || (nScrollCode != SB_LINEDOWN &&
    nScrollCode != SB_LINEUP))
   {
   UnlockDateField(hWnd);
   return;
   }

lpField = LockField(hWnd);

if ((iCursorPos = DateGetCursorPos(hWnd, (short)lpField->iCurrentPos)) == -1)
   {
   UnlockField(hWnd);
   UnlockDateField(hWnd);
   return;
   }

if (*lpField->lpszString == '\0')
   LoadCurrentDate(hWnd);

Date.nMonth = DateGetMonth(lpDateField->df.cSeparator, lpField->lpszString,
                         (short)lpDateField->df.nFormat, lpDateField->df.bCentury);
Date.nDay = DateGetDay(lpDateField->df.cSeparator, lpField->lpszString, 
                         (short)lpDateField->df.nFormat, lpDateField->df.bCentury);
if ((Date.nYear = DateGetYear(lpDateField->df.cSeparator, lpField->lpszString, 
                         (short)lpDateField->df.nFormat,lpDateField->df.bCentury,
                         lpDateField->nTwoDigitYearMax)) == -1)
   Date.nYear = 0;

switch (iCursorPos)
   {
   case DATE_MM:
   case DATE_MON:
      if (nScrollCode == SB_LINEDOWN)
         DatePrevious(&Date, 0, 1, 0);
      else
         DateNext(&Date, 0, 1, 0);

      dMax = DaysPerMonth((short)Date.nMonth, (short)Date.nYear);
      if (Date.nDay > dMax)
         Date.nDay = dMax;

      break;

   case DATE_DD:
      if (nScrollCode == SB_LINEDOWN)
         DatePrevious(&Date, 0, 0, 1);
      else
         DateNext(&Date, 0, 0, 1);

      break;

   case DATE_YY:
      if (nScrollCode == SB_LINEDOWN)
         DatePrevious(&Date, 1, 0, 0);
      else
         DateNext(&Date, 1, 0, 0);

      dMax = DaysPerMonth((short)Date.nMonth, (short)Date.nYear);
      if (Date.nDay > dMax)
         Date.nDay = dMax;

      break;

   case DATE_NYY:
      NengoScroll(&Date, (nScrollCode == SB_LINEDOWN) ? -1 : 1);
      break;
   }

DateDMYToString(&Date, szDate, &lpDateField->df);

// RFW - 4/29/99 - SPR_CEL_004_004
lpDateField->fStrictValidate = TRUE;
SetWindowText(hWnd, szDate);
lpDateField->fStrictValidate = FALSE;
UpdateWindow(hWnd);

UnlockField(hWnd);
UnlockDateField(hWnd);
}


short DateGetCursorPos(HWND hWnd, short iPos)
{
LPDATEFIELD lpField;
short       iCursorPos = -1;

lpField = LockDateField (hWnd);

switch (lpField->df.nFormat)
   {
   case IDF_DDMONYY:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_DD, DATE_MON, DATE_YY);
      break;

   case IDF_DDMMYY:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_DD, DATE_MM, DATE_YY);
      break;

   case IDF_MMDDYY:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_MM, DATE_DD, DATE_YY);
      break;

   case IDF_YYMMDD:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_YY, DATE_MM, DATE_DD);
      break;

   //------------------------------------------------------------------------
   // Date format types from here handle Era etc.. (Masanori Iwasa)
   //------------------------------------------------------------------------
   case IDF_YYMM:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_YY, DATE_MM, DATE_NULL);
      break;

   case IDF_MMDD:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_MM, DATE_DD, DATE_NULL);
      break;

   case IDF_NYYMMDD:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_NYY, DATE_MM, DATE_DD);
      break;

   case IDF_NNYYMMDD:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_NNYY, DATE_MM, DATE_DD);
      break;

   case IDF_NNNNYYMMDD:
      iCursorPos = DateGetCursorPos2(hWnd, iPos, lpField->df,
                                     DATE_NNNNYY, DATE_MM, DATE_DD);
   }

UnlockDateField(hWnd);
return (iCursorPos);
}


//- CAUTION ----------------------------------------------------------------------+
// The function prototype changed. See EditDate.h for details. (Masanori Iwasa)   |
//--------------------------------------------------------------------------------+
BOOL DateGetCursorPos2(HWND hWnd, short iPos, DATEFORMAT df,
                       short First, short Second, short Third)
{
short          Formats[3];
short          iCursorPos = -1;
short          iCurrentPos = 0;
short          i;
short          NengoLen;

Formats[0] = First;
Formats[1] = Second;
Formats[2] = Third;

for (i = 0; iCursorPos == -1 && i < 3; i++)
   {
   switch (Formats[i])
      {
      case DATE_DD:
         if (iPos >= iCurrentPos && iPos < iCurrentPos + 2)
            iCursorPos = DATE_DD;

         iCurrentPos += 2;
         break;

      case DATE_YY:
         if ((df.bCentury && iPos >= iCurrentPos && iPos < iCurrentPos + 4) ||
             (!df.bCentury && iPos >= iCurrentPos && iPos < iCurrentPos + 2))
            iCursorPos = DATE_YY;

         if (df.bCentury)
            iCurrentPos += 4;
         else
            iCurrentPos += 2;

         break;

      case DATE_MM:
         if (iPos >= iCurrentPos && iPos < iCurrentPos + 2)
            iCursorPos = DATE_MM;

         iCurrentPos += 2;
         break;

      case DATE_MON:
         if (iPos >= iCurrentPos && iPos < iCurrentPos + 3)
            iCursorPos = DATE_MON;

         iCurrentPos += 3;
         break;

      //------------------------------------------------------------------------
      // Date format types from here handle Era etc.. (Masanori Iwasa)
      //------------------------------------------------------------------------
      case DATE_NYY:
      case DATE_NNYY:
      case DATE_NNNNYY:
//GAB 3/2/05 - Bug 15794
 //        NengoLen = (Formats[i] / DATE_NNNNYY) + Formats[i] - 2;
#ifdef _UNICODE
        if (Formats[i] == DATE_NNNNYY) 
          NengoLen = 4;
        else // DATE_NYY || DATE_NNYY
          NengoLen = 3;
#else
        if (Formats[i] == DATE_NYY) NengoLen = 3;
        else if (Formats[i] == DATE_NNYY) NengoLen = 4;
        else if (Formats[i] == DATE_NNNNYY) NengoLen = 6;
#endif

         if (iPos >= iCurrentPos && iPos < iCurrentPos + NengoLen)
             if (iPos >= iCurrentPos && iPos < iCurrentPos + NengoLen - 2)
                 iCursorPos = DATE_NYY;
             else
                 iCursorPos = DATE_YY;

         iCurrentPos += NengoLen;
         break;  
      }

   if (Formats[i] != DATE_NULL)
       {
       iCurrentPos++;
//GAB 3/2/05 - Bug 15794
#ifndef UNICODE
      if (IsDbcsSeparator(df.cSeparator)) iCurrentPos++;
#endif 
   }
   }

return (iCursorPos);
}


BOOL DatePrevious(lpDate, Years, Months, Days)

LPTB_DATE lpDate;
short     Years;
short     Months;
short     Days;
{
/********************
* Calculate new day
********************/

while (Days)
   {
   if (Days < lpDate->nDay)
      {
      lpDate->nDay -= Days;
      Days = 0;
      }

   else
      {
      lpDate->nYear -= (1 - lpDate->nMonth + 12) / 12;

      if (1 < lpDate->nMonth)
         lpDate->nMonth -= 1;
      else
         lpDate->nMonth = 12 - ((1 - lpDate->nMonth) % 12);

      Days -= lpDate->nDay;
      lpDate->nDay = DaysPerMonth((short)lpDate->nMonth, (short)lpDate->nYear);
      }
   }

/*********************
* Calculate new year
*********************/

lpDate->nYear -= Years;

/**********************
* Calculate new month
**********************/

lpDate->nYear -= (Months - lpDate->nMonth + 12) / 12;

if (Months < lpDate->nMonth)
   lpDate->nMonth -= Months;
else
   lpDate->nMonth = 12 - ((Months - lpDate->nMonth) % 12);

return (TRUE);
}


BOOL DateNext(lpDate, Years, Months, Days)

LPTB_DATE lpDate;
short     Years;
short     Months;
short     Days;
{
/********************
* Calculate new day
********************/

while (Days > 0)
   {                                                           
   if (Days <= DaysPerMonth((short)lpDate->nMonth, (short)lpDate->nYear) - lpDate->nDay)
      {
      lpDate->nDay += Days;
      Days = 0;
      }

   else
      {
      lpDate->nYear += lpDate->nMonth / 12;

      if (lpDate->nMonth == 12)
         lpDate->nMonth = 1;
      else
         lpDate->nMonth++;

      Days -= DaysPerMonth((short)lpDate->nMonth, (short)lpDate->nYear);
      lpDate->nDay = 1;
      }
   }

/*********************
* Calculate new year
*********************/

lpDate->nYear += Years;

/**********************
* Calculate new month
**********************/

lpDate->nYear += (Months + lpDate->nMonth - 1) / 12;

if (Months <= 12 - lpDate->nMonth)
   lpDate->nMonth += Months;
else
   {
   lpDate->nMonth = (Months + lpDate->nMonth) % 12;
   if (lpDate->nMonth == 0)
      lpDate->nMonth = 12;
   }

return (TRUE);
}


BOOL DateProcessWMKeyDown(HWND hWnd, WPARAM wParam)
{
BOOL fRet = FALSE;

switch (wParam)
   {
   case VK_DOWN:
      DateVScroll(hWnd, SB_LINEDOWN);
      fRet = TRUE;
      break;

   case VK_UP:
      DateVScroll(hWnd, SB_LINEUP);
      fRet = TRUE;
      break;
   }

return (fRet);
}


void DateEnable(HWND hWnd, WPARAM wParam)
{
LPDATEFIELD lpInfo;

lpInfo = LockDateField(hWnd);

if (lpInfo->hWndSpinBtn)
   EnableWindow(lpInfo->hWndSpinBtn, (BOOL)wParam);

UnlockDateField(hWnd);
}
