///////////////////////////////////////////////////////////////////////////////
// Popup Calendar Class                      Steve Malikoff   May/June 1990  //
///////////////////////////////////////////////////////////////////////////////
// Description
// ===========
// This file contains all code for the Drover ToolBox PopupCalendar() function.
// This function does the following:
//    1) Gets the text from the TBdate window given as the argument to PopupCalendar
//    2) The format is retrieved with DateGetFormat
//    3) The date contents are checked against the required format and validated
//    4) The date string is converted to integers and stored in a global (to this
//       file anyway) TB_DATE structure.
//    5) The Dialog with scrollbar, buttons and title for the calendar is invoked.
//    6) The Calendar class is invoked from the Calendar Dialog.
//    7) The Focus and Month is set if the TBdate was valid, else set to the current date.
//    8) The selected date (Day and Month changeable, not the Year) is converted
//       to text.
//    9) The popup calendar disappears and the text is set back into the TBdate field.
///////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------//
// 
//    Modifications made 28 Aug 91.
//       
//    Randall Smallwood.
// 
//    1. Return the focus to the body of the calendar after any scroll-
//       bar actions.
// 
//    2. Check for blank date field on initialization and if blank use
//       today's date instead.
// 
//    3. Fix bug whereby today's date was being overwritten when the
//       underlying date field contained a blank or invalid date.
// 
//    4. Check the return from DialogBox in order to avoid setting the
//       contents of the underlying field when the user hit ESCAPE or
//       the CANCEL button.
// 
//    5. Change EndDialog return to FALSE on IDCANCEL.
// 
//    6. Avoid redrawing the whole calendar when the user clicks on
//       a day number; change the highlight only.
// 
//    7. Accept the currently highlighted date and exit on double-click.
//       (Added CS_DBLCLKS in LibMain.c)
// 
//    8. Process SB_PAGEUP and SB_PAGEDOWN to provide the same action as
//       keyboard PageUp and PageDown: Next, previous year.
// 
//    9. Improve the appearance of the calendar:
// 
//          -  Reduce the overall size
//          -  Move the month and year into the window caption
//          -  Center day names above their columns
//          -  Center day numbers vertically in their boxes
//          -  Allow a border around the calendar portion of the dialog
//          -  Replace the so-called focus rectangle with an inverted
//             rectangle for better highlighting
// 
//       (Changed sizes and positions of elements in ToolBox.dlg)
// 
//--------------------------------------------------------------------------//


#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <tchar.h>
#include <string.h>
#include <toolbox.h>                         
#include <ctype.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "..\..\dialog.h"
#include "calendar.h"
#include "calmonth.h"      // Contains definition of CalMonth[] array
#include "editdate.h"
#include "..\editfld.h"
#include "..\..\classes\wintools.h"
#include "..\..\..\vbx\stringrc.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "..\dialog.h"
//borland #include "editdate\calendar.h"
//borland #include "editdate\calmonth.h"      // Contains definition of CalMonth[] array
//borland #include "editdate\editdate.h"
//borland #include "editfld.h"
//borland #include "..\classes\wintools.h"
//borland #include "..\..\stringrc.h"
<-BORLAND*/
#endif
#include "fptools.h"

#define ISLEAP(nYear)        (nYear%4 == 0 && nYear%100 != 0 || nYear%400 == 0)
#define SPINWIDTH(lpCalInfo) (lpCalInfo->iBoxW / 2)

#if defined(_WIN64) || defined(_IA64)
  #define GetWindowCalInfo(hWnd) (HANDLE)GetWindowLongPtr(hWnd,0)
  #define SetWindowCalInfo(hWnd,value) SetWindowLongPtr(hWnd,0,(LONG_PTR)(value))
#elif WIN32
  #define GetWindowCalInfo(hWnd) (HANDLE)GetWindowLong(hWnd,0)
  #define SetWindowCalInfo(hWnd,value) SetWindowLong(hWnd,0,(LONG)(value))
#else
  #define GetWindowCalInfo(hWnd) (HANDLE)GetWindowWord(hWnd,0)
  #define SetWindowCalInfo(hWnd,value) SetWindowWord(hWnd,0,(WORD)(value))
#endif

/////////////////////////////////////////////////////////////////////////////////
//////////////////////// GLOBAL Variables (Yuk!) ////////////////////////////////
// The following TB_DATE structure is used to pass the date TO and FROM the Calendar
//   Class to the PopupCalendar() function:
static TB_DATE   CurrentDate;
static LPTB_DATE lpDateMinGlobal;
static LPTB_DATE lpDateMaxGlobal;
static TCHAR     szOkText[15] = _T("");
static TCHAR     szCancelText[15] = _T("");
static BOOL      fUseEnhanced;
static COLORREF  CalBackGround;
static COLORREF  CalUpperHeader;
static COLORREF  CalLowerHeader;
static COLORREF CalHeaders;
static COLORREF CalButtonBorder;
static COLORREF CalUpperNormalStart; 
static COLORREF CalUpperNormalEnd;
static COLORREF CalLowerNormalStart;
static COLORREF CalLowerNormalEnd;
static COLORREF CalUpperPushedStart;
static COLORREF CalUpperPushedEnd;
static COLORREF CalLowerPushedStart;
static COLORREF CalLowerPushedEnd;
//- This parameter is used to specify which display 
//  format to use in the calendar (Masanori Iwasa)
static int  nDateFormatGlobal;

#define PT_TO_PIXELS_HDC(hDC, pt)   (short)(((double)(pt) * (double)GetDeviceCaps(hDC, LOGPIXELSY) / 72.0) + 0.5);

//- All string values are loaded from the resource file (Masanori Iwasa)
static TCHAR MonthLongNames[12][DATENAMESZ] = {_T("")};
static TCHAR MonthShortNames[12][DATENAMESZ] = {_T("")};
static TCHAR DayLongNames[7][DATENAMESZ] = {_T("")};
static TCHAR DayShortNames[7][DATENAMESZ] = {_T("")};

extern TCHAR szYearGlobal[8];            //- Global Year variable (Masanori Iwasa)
extern TCHAR szMonthGlobal[8];           //- Global Month variable
extern TCHAR szDayGlobal[8];             //- Global Day variable

extern short DateJulianStart;
#define CAL_LINESIZE 2

#if 0
#include "dlgs.c"    // CONTAINS DATA FOR THE DLGTEMPLATE AND DLGITEMTEMPLATE STRUCTURES
#endif
#ifdef SS_V80
extern void SSx_GetCalColors(HWND hWnd, LPCOLORREF lpBackGround, LPCOLORREF lpUpperHeader, LPCOLORREF lpLowerHeader,
					  LPCOLORREF lpHeaders, LPCOLORREF lpButtonBorder, LPCOLORREF lpUpperNormalStart, LPCOLORREF lpUpperNormalEnd,
					  LPCOLORREF lpLowerNormalStart, LPCOLORREF lpLowerNormalEnd, LPCOLORREF lpUpperPushedStart, LPCOLORREF lpUpperPushedEnd,
					  LPCOLORREF lpLowerPushedStart, LPCOLORREF lpLowerPushedEnd);

#endif
#ifdef SS_V35
extern void SSx_GetCalText(HWND hWnd, LPCALTEXT lpCalText);

#if defined(_WIN64) || defined(_IA64)
INT_PTR WINAPI DialogBoxParamProc(HINSTANCE hInst, LPCTSTR lpszDlg, HWND hWnd,
                              DLGPROC DlgProc, LPARAM lParam)
#else
int WINAPI DialogBoxParamProc(HINSTANCE hInst, LPCTSTR lpszDlg, HWND hWnd,
                              DLGPROC DlgProc, LPARAM lParam)
#endif
{
return (DialogBoxParam(hInst, lpszDlg, hWnd, DlgProc, lParam));
}
#endif

extern int WINAPI DialogBoxProc(HINSTANCE, LPCTSTR, HWND, DLGPROC);

BOOL FAR PASCAL CalAdjustSize(HWND hWnd, LPCALINFO lpCalInfo);
LPRECT          CalGetYearRect(LPCALINFO lpCalInfo, LPRECT lpRect);
LPRECT          CalGetMonthRect(LPCALINFO lpCalInfo, LPRECT lpRect);
void            CalSetMonth(LPCALINFO lpCalInfo);
void            CalSetMonth2(LPCALINFO lpCalInfo, int iMonth, int iDay,
                             int iYear);
void            CalDrawButton(HDC hDC, LPRECT lpRect, HBRUSH hBrush,
                              short dShadowSize, BOOL fButtonDown);
void            CalDraw(HWND hWnd, LPCALINFO lpCalInfo, int dAction);
void            CalDateChanged(HWND hWnd, LPCALINFO lpCalInfo);
BOOL            CalIsValidDate(LPCALINFO lpCalInfo, int nMonth, int nDay,
                               int nYear);
void            _DrawDay(HDC hDC, LPCALINFO lpCalInfo, int iDayNumber,
                         int iBoxNumber, HBRUSH hBrushFace, COLORREF ColorText,
                         BOOL fButtonDown, BOOL fDrawButton);

/////////////////////////////////////////////////////////////////////////////////
// PopupCalendar() : THIS FUNCTION IS THE ENTRY POINT INTO THE CALENDAR CLASS //
/////////////////////////////////////////////////////////////////////////////////
/////// The Window Handle for PopupCalendar() is that of the TBdate class: /////
BOOL DLLENTRY PopupCalendar(HWND hDateWnd, BOOL fEnhanced)
{
LPDATEFIELD lpField;
DATEFORMAT  df;
TB_DATE     DateMin;
TB_DATE     DateMax;
LPTB_DATE   lpDateMin;
LPTB_DATE   lpDateMax;
TCHAR       szDateText[41];        // Max Date string length: Maybe there is a define somewhere for this?
BOOL        fTBdateStatus = TRUE;   // Whether TBdate was valid or not

fUseEnhanced = fEnhanced;

//- Loads the Month & DayofWeek names from the resource file. (Masanori Iwasa)
LoadCalendarResource();

// Get the Date text string from the TBdate edit field:
GetWindowText(hDateWnd, szDateText, 40);
// Since it is a TBdate class, find out what format it is meant to be:
DateGetFormat(hDateWnd, &df);

lpField = LockDateField(hDateWnd);

if (lpField->lRangeMin != UNDEFINED && lpField->lRangeMax != UNDEFINED)
   {
   lpDateMin = DateJulianToDMY(lpField->lRangeMin, &DateMin);
   lpDateMax = DateJulianToDMY(lpField->lRangeMax, &DateMax);
   }
else
   {
   DateMin.nMonth = 1;
   DateMin.nDay = 1;
   DateMin.nYear = LOYEAR;
   lpDateMin = &DateMin;
   DateMax.nMonth = 12;
   DateMax.nDay = 31;
   DateMax.nYear = HIYEAR;
   lpDateMax = &DateMax;
   }

if (CalendarEx(GetParent(hDateWnd), szDateText, &df, lpDateMin, lpDateMax,
               lpField->nTwoDigitYearMax))
   {
   SetWindowText(hDateWnd, szDateText);
   fTBdateStatus = TRUE;
   }
else
   fTBdateStatus = FALSE;

UnlockDateField(hDateWnd);

// Set the focus back to the TBdate edit field:
SetFocus(hDateWnd);

return (fTBdateStatus);
}


LONG DLLENTRY Calendar(HWND hWndParent, LPTSTR lpszDateText,
                       LPDATEFORMAT lpDateFormat, LPTB_DATE lpDateMin,
                       LPTB_DATE lpDateMax)
{
return (CalendarEx(hWndParent, lpszDateText, lpDateFormat, lpDateMin,
                   lpDateMax, TWODIGITYEARMAX_DEF));
}

LONG DLLENTRY CalendarEx(HWND hWndParent, LPTSTR lpszDateText,
                         LPDATEFORMAT lpDateFormat, LPTB_DATE lpDateMin,
                         LPTB_DATE lpDateMax, short nTwoDigitYearMax)
{
TCHAR   szDateText[41];
BOOL    fTBdateStatus = TRUE;       // Whether TBdate was valid or not
int     iStatus = 0;
FARPROC lpfnProc;
LONG    lDate = 0L;
LONG    lDateMin;
LONG    lDateMax;
#if defined(_WIN64) || defined(_IA64)
INT_PTR iBox;
#else
int     iBox;
#endif

#ifdef SS_V35
CALTEXT CalText;

memset(&CalText, '\0', sizeof(CALTEXT));

SSx_GetCalText(hWndParent, &CalText);
#endif // SS_V35
#ifdef SS_V80
SSx_GetCalColors(hWndParent, &CalBackGround, &CalUpperHeader, &CalLowerHeader, &CalHeaders,
	&CalButtonBorder, &CalUpperNormalStart, &CalUpperNormalEnd, &CalLowerNormalStart, &CalLowerNormalEnd,
	&CalUpperPushedStart, &CalUpperPushedEnd, &CalLowerPushedStart, &CalLowerPushedEnd);

#endif
if (lpszDateText)
   {
   StrnCpy(szDateText, lpszDateText, 40);
   szDateText[40] = '\0';
   }
else
   szDateText[0] = '\0';

// Validate it. If junk, use Today's Date, but return false when done:
// Assume TRUE to start with:
fTBdateStatus = TRUE;
if (StrEqu (StrTrim (szDateText), _T("")) ||     // Blank
    !lpDateFormat ||
    !DateStringIsValidEx(szDateText, lpDateFormat, nTwoDigitYearMax)) // Invalid
   {
	long lJulianMin,lJulianMax,lJulian;
   // The contents of the TBdate field is invalid, so bring up the Calendar
   // with Today's Date:
   SysGetDate(&CurrentDate.nDay, &CurrentDate.nMonth, &CurrentDate.nYear);
//Add by BOC 99.7.22 (hyt)-----------------------------
//for current date not in range should rejust
   lJulianMin = DateDMYToJulian(lpDateMin);
   lJulianMax = DateDMYToJulian(lpDateMax);
   lJulian  = DateDMYToJulian(&CurrentDate);
   if(lJulian < lJulianMin)
	   _fmemcpy(&CurrentDate,lpDateMin,sizeof(TB_DATE));
   else if(lJulian > lJulianMax)
	   _fmemcpy(&CurrentDate,lpDateMax,sizeof(TB_DATE));
//-----------------------------------------------------
   fTBdateStatus = FALSE;
   }
else
   // Convert the Date string to integers in the global DATE structure CurrentDate:
   DateStringToDMYEx(szDateText, &CurrentDate, lpDateFormat, nTwoDigitYearMax);

//- Cell format is retrieved and used to display the date (used later). (Masanori Iwasa)
nDateFormatGlobal = lpDateFormat->nFormat;

if(hDynamicInst == NULL)
   return FALSE;

lpDateMinGlobal = lpDateMin;
lpDateMaxGlobal = lpDateMax;
lpfnProc = MakeProcInstance ((FARPROC)CalendarProc, hDynamicInst);
if (!lpfnProc)
   return FALSE;

#ifdef SS_V35

#ifndef SPREAD_JPN
if (iBox = DialogBoxParamProc(hDynamicInst, _T("Calendar"), hWndParent, (DLGPROC)lpfnProc, (LPARAM)(LPVOID)&CalText))
#else
if (iBox = DialogBoxParamProc(hDynamicInst, _T("Calendar_jpn"), hWndParent, (DLGPROC)lpfnProc, (LPARAM)(LPVOID)&CalText))
#endif
#else
#ifndef SPREAD_JPN
if (iBox = DialogBoxProc(hDynamicInst, _T("Calendar"), hWndParent, (DLGPROC)lpfnProc))
#else
if (iBox = DialogBoxProc(hDynamicInst, _T("Calendar_jpn"), hWndParent, (DLGPROC)lpfnProc))
#endif
#endif   // SS_V35
   {
   // Convert the global DATE CurrentDate contents back to a string and put
   // back into the TBdate edit field:

   if (lpszDateText && lpDateFormat)
      DateDMYToString(&CurrentDate, lpszDateText, lpDateFormat);

   lDate = DateDMYToJulian(&CurrentDate);

   if (lpDateMin && lpDateMax)
      {
      lDateMin = DateDMYToJulian(lpDateMin);
      lDateMax = DateDMYToJulian(lpDateMax);

      if (!(lDateMin <= lDate && lDate <= lDateMax))
         lDate = 0L;
      }
   }

FreeProcInstance (lpfnProc);

return (lDate);
}

///////////////////////////////////////////////////////////////////////////////
// Window Function for the Calendar Dialog                                   //
///////////////////////////////////////////////////////////////////////////////
DLGENTRY CalendarProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
HANDLE    hCalInfo;
LPCALINFO lpCalInfo;    // defined in "calendar.h". Contains all details.
HBRUSH    hBrush;
RECT      Rect;

switch (message)
   {
	case WM_DRAWITEM:
	{
		TCHAR       szDateText[255];
		LPDRAWITEMSTRUCT lpDI = (LPDRAWITEMSTRUCT)lParam;
		COLORREF ColorOld = SetTextColor(lpDI->hDC, RGB(106,90,205));
		CalDrawButton(lpDI->hDC, &(lpDI->rcItem), NULL, 0, lpDI->itemState & ODS_SELECTED );
		GetWindowText(lpDI->hwndItem, szDateText, 255);
		SetBkMode(lpDI->hDC, TRANSPARENT);
		DrawText(lpDI->hDC, szDateText, lstrlen(szDateText), &(lpDI->rcItem), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SetTextColor(lpDI->hDC, ColorOld);
		SetBkMode(lpDI->hDC, OPAQUE);
		return TRUE;

	}
   case WM_INITDIALOG:
      {
	  
#ifdef SS_V35
      LPCALTEXT lpCalText = (LPCALTEXT)lParam;

      hCalInfo = GetWindowCalInfo(GetDlgItem(hDlg, IDD_DAYLIST));
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);
      memcpy(&lpCalInfo->calText, lpCalText, sizeof(CALTEXT));
      tbMemUnlock(hCalInfo);
       
#endif
	  if (fUseEnhanced)
	  {
		  long style = GetWindowLong(GetDlgItem(hDlg, IDOK), GWL_STYLE);
		  style |= BS_OWNERDRAW;
		  SetWindowLong(GetDlgItem(hDlg, IDOK), GWL_STYLE, style);
		  style = GetWindowLong(GetDlgItem(hDlg, IDCANCEL), GWL_STYLE);
		  style |= BS_OWNERDRAW;
		  SetWindowLong(GetDlgItem(hDlg, IDCANCEL), GWL_STYLE, style);
		  ShowWindow(GetDlgItem(hDlg, IDD_STATIC), SW_HIDE);
	  }

      DlgBoxCenter(hDlg);  // BJO 19Aug96 JAP4285
      {
      HFONT hFont = (HFONT)SendMessage(GetDlgItem(hDlg, IDD_DAYLIST),
                                       WM_GETFONT, 0, 0);
      SendMessage(GetDlgItem(hDlg, IDOK), WM_SETFONT, (WPARAM)hFont, 1);
      SendMessage(GetDlgItem(hDlg, IDCANCEL), WM_SETFONT, (WPARAM)hFont, 1);
      }
#ifdef SS_V35
      if (lpCalText->OkText[0])
         SendMessage(GetDlgItem(hDlg, IDOK), WM_SETTEXT, 0,
                     (LPARAM)(LPSTR)lpCalText->OkText);
      else if (szOkText[0])
         SendMessage(GetDlgItem(hDlg, IDOK), WM_SETTEXT, 0,
                     (LPARAM)(LPSTR)szOkText);

      if (lpCalText->CancelText[0])
         SendMessage(GetDlgItem(hDlg, IDCANCEL), WM_SETTEXT, 0,
                     (LPARAM)(LPSTR)lpCalText->CancelText);
      else if (szCancelText[0])
         SendMessage(GetDlgItem(hDlg, IDCANCEL), WM_SETTEXT, 0,
                     (LPARAM)(LPSTR)szCancelText);

#else
      if (szOkText[0])
         SendMessage(GetDlgItem(hDlg, IDOK), WM_SETTEXT, 0,
                     (LPARAM)(LPSTR)szOkText);

      if (szCancelText[0])
         SendMessage(GetDlgItem(hDlg, IDCANCEL), WM_SETTEXT, 0,
                     (LPARAM)(LPSTR)szCancelText);

#endif

      if ((HWND)wParam != GetDlgItem(hDlg, IDOK))
         {
         SetFocus(GetDlgItem(hDlg, IDOK));
         return (0);
         }
      }
      break;

   // Keep the title bar up-to-date - rdw
   case WM_WININICHANGE:
      {
      TCHAR szText[64];
      TB_DATE date;

      // Get the handle for the struture for this window's details:
      hCalInfo = GetWindowCalInfo(GetDlgItem(hDlg, IDD_DAYLIST));
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

      // Update Title Bar in case the format changed
      date.nDay   = lpCalInfo->iDayFocus;
      date.nMonth = lpCalInfo->iMonth;
      date.nYear  = lpCalInfo->iYear;
      // 9113...
#ifdef SS_V35
      DateWinFormat(szText, (LPTB_DATE)&date, NULL, (LPVOID)lpCalInfo);
#else
      DateWinFormat(szText, (LPTB_DATE)&date, NULL, NULL);
#endif
      // ... 9113
      SetWindowText(hDlg, szText);
      tbMemUnlock(hCalInfo);
      }
      break;

   case WM_COMMAND:
 #ifdef WIN32
      switch (LOWORD(wParam))
 #else
      switch (wParam)
 #endif
         {
         case IDOK:
            // OK, put these values back into the global structure and
            // finish up.  Remember, don't need the Year!:

            // The comment above came from Steve Malikoff, I think.
            // I do not understand why he thought the year wasn't needed.
            // Without it a date selected from the calendar retains the
            // current year.  Anyway, I changed it to store the year as
            // well as the day and month.  --  Randall.

            hCalInfo = GetWindowCalInfo(GetDlgItem(hDlg, IDD_DAYLIST));
            lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);
            CurrentDate.nDay =  lpCalInfo->iDayFocus;
            CurrentDate.nMonth = lpCalInfo->iMonth;
            CurrentDate.nYear =  lpCalInfo->iYear;  // I think we DO need the year.
            tbMemUnlock(hCalInfo);
            EndDialog(hDlg, TRUE);
            return TRUE;
            break;

         case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return TRUE;
            break;

         default:
            return FALSE;
         }

      break;

   case WM_ERASEBKGND:
	   {
#if SS_V80
		if (fUseEnhanced)
//			hBrush = CreateSolidBrush(RGB(161, 186, 221));
			hBrush = CreateSolidBrush(CalBackGround);
		else
#endif
			hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	    GetClientRect(hDlg, &Rect);
		FillRect((HDC)wParam, &Rect, hBrush);
		DeleteObject(hBrush);

      return (TRUE);
	   }

   default:
      return FALSE;
   }

return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void Calendar_OnHScroll(HWND hWnd, HWND hWndCtl, UINT nCode, int nPos)
{
HANDLE      hCalInfo;
LPCALINFO   lpCalInfo;

hCalInfo = GetWindowCalInfo(hWnd);
lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

if (hWndCtl == lpCalInfo->hWndSpinYearLeft)
   SendMessage(hWnd, WM_KEYDOWN, VK_PRIOR, 0L);

else if (hWndCtl == lpCalInfo->hWndSpinYearRight)
   SendMessage(hWnd, WM_KEYDOWN, VK_NEXT, 0L);

else if (hWndCtl == lpCalInfo->hWndSpinMonthLeft)
   SendMessage(hWnd, CM_CHANGEMONTH, 0, lpCalInfo->iMonth - 1);

else if (hWndCtl == lpCalInfo->hWndSpinMonthRight)
   SendMessage(hWnd, CM_CHANGEMONTH, 0, lpCalInfo->iMonth + 1);

UpdateWindow(hWnd);

tbMemUnlock(hCalInfo);
}

////////////////////////////////////////////////////////////////////////////////
// Window Function for the Calendar Class  (registered in libmain.c)          //
////////////////////////////////////////////////////////////////////////////////
WINENTRY CalendarWndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam)
{
HANDLE      hCalInfo;
LPCALINFO   lpCalInfo;
PAINTSTRUCT Paint;
HBITMAP     hBitmapOld;
HBITMAP     hBitmapDC;
RECT        Rect;
HDC         hDCMemory;
HDC         hDC;
int         iMonth;
int         iDay;
int         iYear;
int         iBoxNumber;
int         iStartDofW;
int         iDaysInThisMonth;
int         iPreviousDayFocus;
int         iWhereLast;
BOOL        fSetMonth;

switch (message)
   {
   case WM_CREATE:
      FillCalInfoStruct(hWnd);
      break;

   case WM_SIZE:
      hCalInfo = GetWindowCalInfo(hWnd);
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

      CalAdjustSize(hWnd, lpCalInfo);

      CalGetYearRect(lpCalInfo, &Rect);

      MoveWindow(lpCalInfo->hWndSpinYearLeft, Rect.left, Rect.top,
             SPINWIDTH(lpCalInfo), Rect.bottom - Rect.top, TRUE);
      MoveWindow(lpCalInfo->hWndSpinYearRight, Rect.right -
             SPINWIDTH(lpCalInfo), Rect.top,
             SPINWIDTH(lpCalInfo), Rect.bottom - Rect.top, TRUE);

      CalGetMonthRect(lpCalInfo, &Rect);

      MoveWindow(lpCalInfo->hWndSpinMonthLeft, Rect.left, Rect.top,
             SPINWIDTH(lpCalInfo), Rect.bottom - Rect.top, TRUE);
      MoveWindow(lpCalInfo->hWndSpinMonthRight, Rect.right -
             SPINWIDTH(lpCalInfo), Rect.top,
             SPINWIDTH(lpCalInfo), Rect.bottom - Rect.top, TRUE);

      tbMemUnlock(hCalInfo);
      break;

   case WM_GETFONT:
      hCalInfo = GetWindowCalInfo(hWnd);
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

      if (lpCalInfo->hFont)
         {
         HFONT hFont = lpCalInfo->hFont;
         tbMemUnlock(hCalInfo);
         return ((LRESULT)hFont);
         }

      tbMemUnlock(hCalInfo);
      return (DefWindowProc(hWnd, message, wParam, lParam));

   case WM_ERASEBKGND:
      return (TRUE);

   case WM_PAINT:
      hDC = BeginPaint(hWnd, &Paint);

      hCalInfo = GetWindowCalInfo(hWnd);
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

      if (lpCalInfo->iAction != DRAW_FOCUS)
         {
         GetClientRect(hWnd, &Rect);

         /*********************************
         * Create a memory device context
         *********************************/

         hBitmapDC = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,
                            Rect.bottom - Rect.top);

         hDCMemory = CreateCompatibleDC(hDC);
         hBitmapOld = SelectObject(hDCMemory, hBitmapDC);

         hCalInfo = GetWindowCalInfo(hWnd);
         lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

         CalGetYearRect(lpCalInfo, &Rect);
         Rect.right = Rect.left + SPINWIDTH(lpCalInfo);
         ExcludeClipRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

         CalGetYearRect(lpCalInfo, &Rect);
         Rect.left = Rect.right - SPINWIDTH(lpCalInfo);
         ExcludeClipRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

         CalGetMonthRect(lpCalInfo, &Rect);
         Rect.right = Rect.left + SPINWIDTH(lpCalInfo);
         ExcludeClipRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

         CalGetMonthRect(lpCalInfo, &Rect);
         Rect.left = Rect.right - SPINWIDTH(lpCalInfo);
         ExcludeClipRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

         DrawCalendar(hWnd, hDC);
#if 0
         DrawCalendar(hWnd, hDCMemory);

         /*******************************************************
         * Copy the memory device context bitmap to the display
         *******************************************************/

         BitBlt(hDC, Paint.rcPaint.left, Paint.rcPaint.top,
            Paint.rcPaint.right - Paint.rcPaint.left,
            Paint.rcPaint.bottom - Paint.rcPaint.top, hDCMemory,
            Paint.rcPaint.left, Paint.rcPaint.top, SRCCOPY);
#endif

         /***********
         * Clean up
         ***********/

         SelectObject(hDCMemory, hBitmapOld);
         DeleteDC(hDCMemory);
         DeleteObject(hBitmapDC);
         }

      else
         DrawCalendar(hWnd, hDC);

      lpCalInfo->iAction = DRAW_EVERYTHING;
      tbMemUnlock(hCalInfo);

      EndPaint(hWnd, &Paint);
      return (0);

   case CM_CHANGEMONTH:
		// The following code was changed to fix 9503.

      hCalInfo = GetWindowCalInfo(hWnd);
		if (hCalInfo)
			{
			TB_DATE Date;
			BOOL fValid;
			BOOL fForward;

			lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

			Date.nMonth = (int)lParam;
			Date.nYear = lpCalInfo->iYear;
			Date.nDay = lpCalInfo->iDayFocus;

			if (Date.nMonth == 0)
				{
				Date.nYear--;
				Date.nMonth = 12;
				fForward = FALSE;
//				Date.nDay = DateGetDaysInMonth(&Date);
				}

			else if (Date.nMonth > 12)
				{
				Date.nYear++;
				Date.nMonth = 1;
				fForward = TRUE;
//				Date.nDay = 1;
				}

         else if (Date.nMonth > lpCalInfo->iMonth)
				fForward = TRUE;
//            Date.nDay = 1;

         else
				fForward = FALSE;
//				Date.nDay = DateGetDaysInMonth(&Date);

			fValid = CalIsValidDate(lpCalInfo, Date.nMonth, Date.nDay, Date.nYear);

			if (!fValid)
				{
				if (fForward)
					Date = lpCalInfo->DateMax;
				else
					Date = lpCalInfo->DateMin;

				fValid = TRUE;
				}

			if (fValid)
				{
				lpCalInfo->iMonth = Date.nMonth;
				lpCalInfo->iYear = Date.nYear;
				lpCalInfo->iDayFocus = Date.nDay;

				CalSetMonth(lpCalInfo);

				lpCalInfo->iPreviousDayFocus = lpCalInfo->iDayFocus;
				CalDraw(hWnd, lpCalInfo, DRAW_EVERYTHING);
				}

			tbMemUnlock(hCalInfo);
			}
      break;

   case CM_GETMONTH:
      hCalInfo = GetWindowCalInfo(hWnd);
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);
      iMonth = lpCalInfo->iMonth;
      tbMemUnlock(hCalInfo);
      return iMonth;
      break;

   case WM_GETDLGCODE:
      return DLGC_WANTARROWS;

   case WM_KEYDOWN:
      hCalInfo = GetWindowCalInfo(hWnd);
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

      switch (wParam)
         {
         case VK_HOME:
            // Set the Focus to the first Day of this Month.
            // Don't redraw Focus if already there:

            if (lpCalInfo->iWhereLast != FOCUS_HOME)
               {
               if (CalIsValidDate(lpCalInfo, lpCalInfo->iMonth, 1,
                   lpCalInfo->iYear))
                  {
                  lpCalInfo->iDayFocus = 1;
                  CalDraw(hWnd, lpCalInfo, DRAW_FOCUS);
                  lpCalInfo->iWhereLast = FOCUS_HOME;
                  }
               }

            break;

         case VK_END:
            // Set the Focus to the last Day of this Month:
            // Don't redraw Focus if already there:
            if (lpCalInfo->iWhereLast != FOCUS_END)
               {
               if (CalIsValidDate(lpCalInfo, lpCalInfo->iMonth,
                   lpCalInfo->iDaysInThisMonth, lpCalInfo->iYear))
                  {
                  lpCalInfo->iDayFocus = lpCalInfo->iDaysInThisMonth;
                  CalDraw(hWnd, lpCalInfo, DRAW_FOCUS);
                  lpCalInfo->iWhereLast = FOCUS_END;
                  }
               }

            break;

         case VK_LEFT:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               SendMessage(hWnd, CM_CHANGEMONTH, 0, lpCalInfo->iMonth - 1);

            else
               {
               iStartDofW = lpCalInfo->iStartDofW;
               iDaysInThisMonth = lpCalInfo->iDaysInThisMonth;
               iPreviousDayFocus = lpCalInfo->iPreviousDayFocus;
               iWhereLast = lpCalInfo->iWhereLast;
               fSetMonth = FALSE;

               lpCalInfo->iAction = DRAW_FOCUS;
               lpCalInfo->iWhereLast = lpCalInfo->iDayFocus;

               iMonth = lpCalInfo->iMonth;
               iDay = lpCalInfo->iDayFocus;
               iYear = lpCalInfo->iYear;

               iDay--;

               if (iDay < 1)
                  {
                  lpCalInfo->iAction = DRAW_EVERYTHING;
                  lpCalInfo->iWhereLast = FOCUS_END;

                  if (--iMonth < 1)
                     {
                     iMonth = 12;
                     iYear--;
                     }

                  CalSetMonth2(lpCalInfo, iMonth, iDay, iYear);
                  iDay = lpCalInfo->iDaysInThisMonth;
                  fSetMonth = TRUE;
                  }

               if (CalIsValidDate(lpCalInfo, iMonth, iDay, iYear))
                  {
                  lpCalInfo->iMonth = iMonth;
                  lpCalInfo->iDayFocus = iDay;
                  lpCalInfo->iYear = iYear;

                  if (fSetMonth)
                     lpCalInfo->iPreviousDayFocus  = lpCalInfo->iDayFocus;

                  CalDraw(hWnd, lpCalInfo, lpCalInfo->iAction);
                  UpdateWindow(hWnd);

                  lpCalInfo->iWhereLast = lpCalInfo->iDayFocus;
                  if (lpCalInfo->iDayFocus == 1)
                     lpCalInfo->iWhereLast = FOCUS_HOME;
                  if (lpCalInfo->iWhereLast == lpCalInfo->iDaysInThisMonth)
                     lpCalInfo->iWhereLast = FOCUS_END;
                  }
               else
                  {
                  lpCalInfo->iStartDofW = iStartDofW;
                  lpCalInfo->iDaysInThisMonth = iDaysInThisMonth;
                  lpCalInfo->iPreviousDayFocus = iPreviousDayFocus;
                  lpCalInfo->iWhereLast = iWhereLast;
                  }
               }

            break;

         case VK_RIGHT:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               SendMessage(hWnd, CM_CHANGEMONTH, 0, lpCalInfo->iMonth + 1);

            else
               {
               iStartDofW = lpCalInfo->iStartDofW;
               iDaysInThisMonth = lpCalInfo->iDaysInThisMonth;
               iPreviousDayFocus = lpCalInfo->iPreviousDayFocus;
               iWhereLast = lpCalInfo->iWhereLast;
               fSetMonth = FALSE;

               lpCalInfo->iAction = DRAW_FOCUS;
               lpCalInfo->iWhereLast = lpCalInfo->iDayFocus;

               iMonth = lpCalInfo->iMonth;
               iDay = lpCalInfo->iDayFocus;
               iYear = lpCalInfo->iYear;

               iDay++;

               if (iDay > lpCalInfo->iDaysInThisMonth)
                  {
                  lpCalInfo->iAction = DRAW_EVERYTHING;
                  iDay = 1;
                  lpCalInfo->iWhereLast = FOCUS_HOME;

                  if (++iMonth > 12)
                     {
                     iMonth = 1;
                     iYear++;
                     }

                  CalSetMonth2(lpCalInfo, iMonth, iDay, iYear);
                  }

               if (CalIsValidDate(lpCalInfo, iMonth, iDay, iYear))
                  {
                  lpCalInfo->iMonth = iMonth;
                  lpCalInfo->iDayFocus = iDay;
                  lpCalInfo->iYear = iYear;

                  CalDraw(hWnd, lpCalInfo, lpCalInfo->iAction);
                  UpdateWindow(hWnd);

                  lpCalInfo->iWhereLast = lpCalInfo->iDayFocus;
                  if (lpCalInfo->iDayFocus == 1)
                     lpCalInfo->iWhereLast = FOCUS_HOME;
                  if (lpCalInfo->iWhereLast == lpCalInfo->iDaysInThisMonth)
                     lpCalInfo->iWhereLast = FOCUS_END;
                  }
               else
                  {
                  lpCalInfo->iStartDofW = iStartDofW;
                  lpCalInfo->iDaysInThisMonth = iDaysInThisMonth;
                  lpCalInfo->iPreviousDayFocus = iPreviousDayFocus;
                  lpCalInfo->iWhereLast = iWhereLast;
                  }
               }

            break;

         case VK_UP:
            iDay = lpCalInfo->iDayFocus;
            iDay -= 7;
            if (iDay < 1)
               {
               iDay += 42;
               while (iDay > lpCalInfo->iDaysInThisMonth)
                  iDay -= 7;
               }

            if (CalIsValidDate(lpCalInfo, lpCalInfo->iMonth, iDay,
                   lpCalInfo->iYear))
               {
               lpCalInfo->iDayFocus = iDay;

               if (lpCalInfo->iDayFocus == 1)
                  lpCalInfo->iWhereLast = FOCUS_HOME;
               else
                  lpCalInfo->iWhereLast = lpCalInfo->iDayFocus;

               CalDraw(hWnd, lpCalInfo, DRAW_FOCUS);
               }

            break;

         case VK_DOWN:
            iDay = lpCalInfo->iDayFocus;
            iDay += 7;
            if (iDay > lpCalInfo->iDaysInThisMonth)
               {
               iDay += lpCalInfo->iStartDofW;
               iDay %= 7;
               if (iDay <= lpCalInfo->iStartDofW)
                  iDay += 7;
               iDay -= lpCalInfo->iStartDofW;
               }

            if (CalIsValidDate(lpCalInfo, lpCalInfo->iMonth, iDay,
                   lpCalInfo->iYear))
               {
               lpCalInfo->iDayFocus = iDay;

               if (lpCalInfo->iDayFocus == lpCalInfo->iDaysInThisMonth)
                  lpCalInfo->iWhereLast = FOCUS_END;
               else
                  lpCalInfo->iWhereLast = lpCalInfo->iDayFocus;

               CalDraw(hWnd, lpCalInfo, DRAW_FOCUS);
               }

            break;

         case VK_PRIOR:
            if (CalIsValidDate(lpCalInfo, lpCalInfo->iMonth,
                   lpCalInfo->iDayFocus, lpCalInfo->iYear - 1))
               --lpCalInfo->iYear;
				else
					{
					lpCalInfo->iMonth = lpCalInfo->DateMin.nMonth;
					lpCalInfo->iYear = lpCalInfo->DateMin.nYear;
					lpCalInfo->iDayFocus = lpCalInfo->DateMin.nDay;
					}

            CalSetMonth(lpCalInfo);
            lpCalInfo->iPreviousDayFocus = lpCalInfo->iDayFocus;
            CalDraw(hWnd, lpCalInfo, DRAW_EVERYTHING);

            break;

         case VK_NEXT:
            if (CalIsValidDate(lpCalInfo, lpCalInfo->iMonth,
                   lpCalInfo->iDayFocus, lpCalInfo->iYear + 1))
               ++lpCalInfo->iYear;
				else
					{
					lpCalInfo->iMonth = lpCalInfo->DateMax.nMonth;
					lpCalInfo->iYear = lpCalInfo->DateMax.nYear;
					lpCalInfo->iDayFocus = lpCalInfo->DateMax.nDay;
					}

            CalSetMonth(lpCalInfo);
            lpCalInfo->iPreviousDayFocus = lpCalInfo->iDayFocus;
            CalDraw(hWnd, lpCalInfo, DRAW_EVERYTHING);

            break;

         default:
            break;
         }

      tbMemUnlock(hCalInfo);
      break;

   case WM_HSCROLL:
      HANDLE_WM_HSCROLL(hWnd, wParam, lParam, Calendar_OnHScroll);
      break;

   case WM_LBUTTONDOWN:
      hCalInfo = GetWindowCalInfo(hWnd);
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);
      iBoxNumber = DayPointXYToBoxNumber(lpCalInfo, (short)LOWORD(lParam),
                         (short)HIWORD(lParam));

      if (iBoxNumber != -1)
         {
         if (CalIsValidDate(lpCalInfo, lpCalInfo->iMonth,
                            iBoxNumber - lpCalInfo->iStartDofW,
                            lpCalInfo->iYear))
            {
            lpCalInfo->iDayFocus = iBoxNumber - lpCalInfo->iStartDofW;

            CalDraw(hWnd, lpCalInfo, DRAW_FOCUS);
            lpCalInfo->iWhereLast = lpCalInfo->iDayFocus;
            }
         }

      tbMemUnlock(hCalInfo);
      break;

   case WM_LBUTTONDBLCLK:
      hCalInfo = GetWindowCalInfo(hWnd);
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

      iBoxNumber = DayPointXYToBoxNumber(lpCalInfo, (short)LOWORD(lParam),
                         (short)HIWORD(lParam));

      if (iBoxNumber != -1)
         FORWARD_WM_COMMAND(GetParent(hWnd), IDOK, 0, 0, SendMessage);

      tbMemUnlock(hCalInfo);
      break;

   case WM_DESTROY:
      hCalInfo = GetWindowCalInfo(hWnd);
      lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);
      if (lpCalInfo->hFont)
         DeleteObject(lpCalInfo->hFont);
      tbMemUnlock(hCalInfo);
      tbMemFree(hCalInfo);
      break;

   case WM_MOUSEACTIVATE:
      SetFocus(hWnd);      // fall through

   default:
      return(DefWindowProc(hWnd, message, wParam, lParam));
   }

return 0;
}


/////////////////////////////////////////////////////////////////////////////
// This function initialises the structure associated with each window, and
// takes the validated contents of the global TB_DATE structure CurrentDate
// as the date to display:

BOOL FAR PASCAL FillCalInfoStruct(HWND hWnd)
{
HANDLE      hCalInfo;
LPCALINFO   lpCalInfo;
TB_DATE     da;
TB_DATE     DayStart;

// Get the TBdate's converted contents from the global TB_DATE structure:
da.nDay   = CurrentDate.nDay;
da.nMonth = CurrentDate.nMonth;
da.nYear  = CurrentDate.nYear;

DayStart.nDay   = 1;
DayStart.nMonth = CurrentDate.nMonth;
DayStart.nYear  = CurrentDate.nYear;

hCalInfo = tbMemAlloc(NULL, (DWORD) sizeof(CALINFO), GHND);
SetWindowCalInfo(hWnd, hCalInfo);
lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);
if (!lpCalInfo)
   {
   TCHAR  szMsg[64];
   TCHAR  szMsgCaption[24];

   //- Load the message from the resource file. (Masanori Iwasa)
   LoadString( hDynamicInst, IDS_ERR_CALENDARLOCK + LANGUAGE_BASE, szMsg, STRING_SIZE(szMsg)-1 );
   LoadString( hDynamicInst, IDS_CALENDAR_CAPTION + LANGUAGE_BASE, szMsgCaption, STRING_SIZE(szMsgCaption)-1 );
   MessageBox( GetFocus(), szMsg, szMsgCaption, MB_OK );
   tbMemUnlock(hCalInfo);
   return FALSE;
   }
else
   {
   LOGFONT LogFont;
   HDC     hDC;

   if (lpDateMinGlobal)
      {
      lpCalInfo->fCheckMin = TRUE;
      _fmemcpy(&lpCalInfo->DateMin, lpDateMinGlobal, sizeof(TB_DATE));
      }

   if (lpDateMaxGlobal)
      {
      lpCalInfo->fCheckMax = TRUE;
      _fmemcpy(&lpCalInfo->DateMax, lpDateMaxGlobal, sizeof(TB_DATE));
      }

   _fmemset(&LogFont, '\0', sizeof(LOGFONT));
   hDC = GetDC(hWnd);
   LogFont.lfWeight = FW_NORMAL;
   LogFont.lfCharSet = DEFAULT_CHARSET;
   
//Modify by Boc 1999.4.12(hyt)------------------------
#ifndef SPREAD_JPN
   LogFont.lfHeight = PT_TO_PIXELS_HDC(hDC, 8);
   lstrcpy(LogFont.lfFaceName, _T("MS Sans Serif"));
//   LogFont.lfCharSet = ANSI_CHARSET;
#else
   LogFont.lfHeight = PT_TO_PIXELS_HDC(hDC, 9);
   lstrcpy(LogFont.lfFaceName, _T("‚l‚r ‚oƒSƒVƒbƒN"));
   //LogFont.lfCharSet = SHIFTJIS_CHARSET;
#endif // SPREAD_JPN
//--------------------------------------------------------
	ReleaseDC(hWnd, hDC);

   lpCalInfo->hFont = CreateFontIndirect(&LogFont);

   lpCalInfo->iMonth             = da.nMonth;
   lpCalInfo->iYear              = da.nYear;
   lpCalInfo->iDayFocus          = da.nDay;
   lpCalInfo->iPreviousDayFocus  = da.nDay;
   lpCalInfo->iAction            = DRAW_EVERYTHING;

   lpCalInfo->iStartDofW         = DateGetWeekdayInt(&DayStart);
   lpCalInfo->iDaysInThisMonth   = DateGetDaysInMonth(&da);

   //- Set the date format so it's the same as the cell. (Masanori Iwasa)
   lpCalInfo->nFormat = nDateFormatGlobal;

   CalAdjustSize(hWnd, lpCalInfo);
   lpCalInfo->iWhereLast         = -1; // Neither HOME nor END to start with

   lpCalInfo->hWndSpinYearLeft = CreateWindow(lpszSpinClassName, NULL,
                      WS_CHILD | WS_VISIBLE |
                      SBNS_BORDERRIGHT |
                      SBNS_UPARROW |
                      SBNS_HORIZONTAL, 0, 0, 0, 0,
                      hWnd, 0, GetWindowInstance(hWnd),
                      NULL);

   SendMessage(lpCalInfo->hWndSpinYearLeft , EM_SETSPINSTYLE, 0, (LPARAM)fUseEnhanced);
   lpCalInfo->hWndSpinYearRight = CreateWindow(lpszSpinClassName, NULL,
                       WS_CHILD | WS_VISIBLE |
                       SBNS_BORDERLEFT |
                       SBNS_DOWNARROW |
                       SBNS_HORIZONTAL, 0, 0, 0, 0,
                       hWnd, 0, GetWindowInstance(hWnd),
                       NULL);
   SendMessage(lpCalInfo->hWndSpinYearRight , EM_SETSPINSTYLE, 0, (LPARAM)fUseEnhanced);
   lpCalInfo->hWndSpinMonthLeft = CreateWindow(lpszSpinClassName, NULL,
                       WS_CHILD | WS_VISIBLE |
                       SBNS_BORDERRIGHT |
                       SBNS_UPARROW |
                       SBNS_HORIZONTAL, 0, 0, 0, 0,
                       hWnd, 0, GetWindowInstance(hWnd),
                       NULL);
   SendMessage(lpCalInfo->hWndSpinMonthLeft , EM_SETSPINSTYLE, 0, (LPARAM)fUseEnhanced);
   lpCalInfo->hWndSpinMonthRight = CreateWindow(lpszSpinClassName, NULL,
                        WS_CHILD | WS_VISIBLE |
                        SBNS_BORDERLEFT |
                        SBNS_DOWNARROW |
                        SBNS_HORIZONTAL, 0, 0, 0, 0,
                        hWnd, 0, GetWindowInstance(hWnd),
                        NULL);
SendMessage(lpCalInfo->hWndSpinMonthRight , EM_SETSPINSTYLE, 0, (LPARAM)fUseEnhanced);
   }

tbMemUnlock(hCalInfo);
return TRUE;
}


BOOL FAR PASCAL CalAdjustSize(HWND hWnd, LPCALINFO lpCalInfo)
{
HDC         hDC;
TEXTMETRIC  tm;
RECT        DaysRect;
HFONT       hFontOld;

// Only need the text height, throw away the rest of the structure:
hDC = GetDC(hWnd);

if (lpCalInfo->hFont);
   hFontOld = SelectObject(hDC, lpCalInfo->hFont);
GetTextMetrics(hDC, &tm);
if (lpCalInfo->hFont);
   SelectObject(hDC, hFontOld);
ReleaseDC(hWnd, hDC);
GetClientRect(hWnd, &DaysRect);

lpCalInfo->iAction            = DRAW_EVERYTHING;

lpCalInfo->iCalWidth          = DaysRect.right;
lpCalInfo->iCalHeight         = DaysRect.bottom;
lpCalInfo->iHeaderItemHeight  = (tm.tmHeight + 2);
SetRect(&lpCalInfo->RectMonthHeader, 0, 0, DaysRect.right * 60 / 100,
        lpCalInfo->iHeaderItemHeight);
SetRect(&lpCalInfo->RectYearHeader, lpCalInfo->RectMonthHeader.right, 0,
         DaysRect.right, lpCalInfo->iHeaderItemHeight);
lpCalInfo->iWeekRectTop       = lpCalInfo->RectMonthHeader.bottom +
                                CAL_LINESIZE;
lpCalInfo->iDaysRectTop       = lpCalInfo->iWeekRectTop +
                                lpCalInfo->iHeaderItemHeight + CAL_LINESIZE;
lpCalInfo->iDaysRectBottom    = DaysRect.bottom;
lpCalInfo->iBoxW              = (lpCalInfo->iCalWidth - 2) / 7 + 1;

return (TRUE);
}



////////////////////////////////////////////////////////////////////////////////
void DrawCalendar(HWND hWnd, HDC hDC)
{
LPCALINFO   lpCalInfo;
HANDLE      hCalInfo;
HBRUSH      hBrushFace;
HBRUSH      hBrushDark;
HBRUSH      hBrushLight;
HBRUSH      hBrushOld;
RECT        RectClient;
RECT        RectTemp;
TB_DATE     da;
HFONT       hFontOld;
short       y;
short       i;
static WORD Bits[] = {0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00};

// Get the handle for the struture for this window's details:
hCalInfo = GetWindowCalInfo(hWnd);
lpCalInfo = (LPCALINFO)tbMemLock(hCalInfo);

if (lpCalInfo->hFont);
   hFontOld = SelectObject(hDC, lpCalInfo->hFont);

// Get the position across (1..7) for the First of the Month:
da.nDay = 1;         // Set to first day of the Month
da.nMonth = lpCalInfo->iMonth;
da.nYear = lpCalInfo->iYear;

CalDateChanged(hWnd, lpCalInfo);

GetClientRect(hWnd, &RectClient);
hBrushFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
hBrushDark = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
hBrushLight = CreateSolidBrush(RGBCOLOR_WHITE);

switch (lpCalInfo->iAction)
   {
   case DRAW_EVERYTHING:
   case DRAW_MONTH:
   case DRAW_DAYS:

      /***************
      * Draw Headers
      ***************/
       SetRect(&RectTemp, 0, 0, RectClient.right, lpCalInfo->iDaysRectTop);
       FillRect(hDC, &RectClient, hBrushFace);
#if SS_V80
	   if (fUseEnhanced)
	   {
		HBRUSH upperBrush, lowerBrush;
		upperBrush = CreateSolidBrush(CalUpperHeader);       
		lowerBrush = CreateSolidBrush(CalLowerHeader);
		fpGradientFill(hDC, RectTemp, upperBrush, lowerBrush, GRADIENT_FILL_RECT_V);
		DeleteObject(upperBrush);
		DeleteObject(lowerBrush);
	   }
       else
#endif
	   {
		for (i = 0, y = 0; i < 2; i++)
         {
         y += lpCalInfo->iHeaderItemHeight;
         hBrushOld = SelectObject(hDC, hBrushDark);
         PatBlt(hDC, 0, y, RectClient.right, 1, PATCOPY);
         if (1 == i)
            SelectObject(hDC, GetStockObject(BLACK_BRUSH));
         else
            SelectObject(hDC, hBrushLight);
         PatBlt(hDC, 0, y + 1, RectClient.right, 1, PATCOPY);
         SelectObject(hDC, hBrushOld);
         y += 2;
         }
	   }
      lpCalInfo->iStartDofW = DateGetWeekdayInt(&da);
      lpCalInfo->iDaysInThisMonth = DateGetDaysInMonth(&da);
      DrawHeader(hWnd, hDC, lpCalInfo);
      /*****************************
      * Draw Previous Month's Days
      *****************************/
      {
      TB_DATE Date;
      int     iDaysInThisMonth;
      int     i;

      Date.nDay   = 1;                // Set to first day of the Month
      Date.nMonth = lpCalInfo->iMonth - 1;
      Date.nYear  = lpCalInfo->iYear;

      if (Date.nMonth == 0)
         {
         Date.nYear--;
         Date.nMonth = 12;
         }

      iDaysInThisMonth = DateGetDaysInMonth(&Date);

      for (i = 0; i < lpCalInfo->iStartDofW; i++)
         _DrawDay(hDC, lpCalInfo,
                  iDaysInThisMonth - (lpCalInfo->iStartDofW - i - 1),
                  i + 1, hBrushFace, RGBCOLOR_DARKGRAY, FALSE, fUseEnhanced);

      /*************************
      * Draw Next Month's Days
      *************************/

      Date.nMonth = lpCalInfo->iMonth + 1;
      Date.nYear  = lpCalInfo->iYear;

      if (Date.nMonth > 12)
         {
         Date.nYear++;
         Date.nMonth = 1;
         }

      for (i = lpCalInfo->iStartDofW + lpCalInfo->iDaysInThisMonth + 1; i <= 42;
           i++)
         _DrawDay(hDC, lpCalInfo,
                  i - (lpCalInfo->iStartDofW + lpCalInfo->iDaysInThisMonth),
                  i, hBrushFace, RGBCOLOR_DARKGRAY, FALSE, fUseEnhanced);
      }

      /****************************
      * Draw Current Month's Days
      ****************************/

      for (i = 1; i <= lpCalInfo->iDaysInThisMonth; i++)
         DrawDayAt(hDC, lpCalInfo, i, hBrushFace, RGBCOLOR_BLACK, FALSE);

      // Fall through to Next Action

   case DRAW_FOCUS:
      // Set the Focus to the current Day, or last if > days in this Month:
      DayFocusAt(hDC, lpCalInfo);
}

if (lpCalInfo->hFont);
   SelectObject(hDC, hFontOld);

DeleteObject(hBrushFace);
DeleteObject(hBrushDark);
DeleteObject(hBrushLight);
tbMemUnlock(hCalInfo);
}


//////////////////////////////////////////////////////////////////////////////////
void DrawHeader(HWND hWnd, HDC hDC, LPCALINFO lpCalInfo)
{
RECT  Rect;
RECT  WeekDayRect;
TCHAR szText[100];
int   iDay;
int bkMode;
/************
* Draw Year
************/

StrPrintf(szText, _T("%d"), lpCalInfo->iYear);
if (IsDbcsSeparator('?'))
     lstrcat(szText, szYearGlobal);

CalGetYearRect(lpCalInfo, &Rect);
if (fUseEnhanced)
{
	HBRUSH hBrush = CreateSolidBrush(CalHeaders);
//	HBRUSH hBrush = CreateSolidBrush(RGB(216, 231, 250));
	FillRect(hDC, &Rect, hBrush);
	DeleteObject(hBrush);
}
bkMode = GetBkMode(hDC);
SetBkMode(hDC, TRANSPARENT);
DrawText(hDC, szText, lstrlen(szText), &Rect, DT_CENTER | DT_VCENTER |
     DT_SINGLELINE);
SetBkMode(hDC, bkMode);
/*************
* Draw Month
*************/

CalGetMonthRect(lpCalInfo, &Rect);
if (fUseEnhanced)
{
	HBRUSH hBrush = CreateSolidBrush(CalHeaders);
	//HBRUSH hBrush = CreateSolidBrush(RGB(216, 231, 250));
	FillRect(hDC, &Rect, hBrush);
	DeleteObject(hBrush);
}

#ifdef SS_V35
if (lpCalInfo->calText.MonthLongNames[lpCalInfo->iMonth - 1][0])
   lstrcpy(szText, lpCalInfo->calText.MonthLongNames[lpCalInfo->iMonth - 1]);
else
   lstrcpy(szText, MonthLongNames[lpCalInfo->iMonth - 1]);
#else
lstrcpy(szText, MonthLongNames[lpCalInfo->iMonth - 1]);
#endif
SetBkMode(hDC, TRANSPARENT);
DrawText(hDC, szText, lstrlen(szText), &Rect,
         DT_CENTER | DT_VCENTER | DT_SINGLELINE);
SetBkMode(hDC, bkMode);
/*********************
* Draw Weekday Names
*********************/

WeekDayRect.top = lpCalInfo->iWeekRectTop;
WeekDayRect.bottom = lpCalInfo->iDaysRectTop - CAL_LINESIZE;
for (iDay = 0; iDay < 7; iDay++)
   {
   DayBoxToRECT(lpCalInfo, &Rect, iDay + 1);
   WeekDayRect.left = Rect.left;
   WeekDayRect.right = Rect.right;

#ifdef SS_V35
   if (lpCalInfo->calText.DayShortNames[iDay][0])
      lstrcpy(szText, lpCalInfo->calText.DayShortNames[iDay]);
else
      lstrcpy(szText, DayShortNames[iDay]);
#else
   lstrcpy(szText, DayShortNames[iDay]);
#endif
   SetBkMode(hDC, TRANSPARENT);
   DrawText(hDC, szText, lstrlen(szText),
            &WeekDayRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
   SetBkMode(hDC, bkMode);
/*
   DrawText(hDC, DayShortNames[iDay], lstrlen(DayShortNames[iDay]),
            &WeekDayRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
*/
   }
}

              
////////////////////////////////////////////////////////////////////////////////

void DrawDayAt(HDC hDC, LPCALINFO lpCalInfo, int iDayNumber, HBRUSH hBrushFace,
               COLORREF ColorText, BOOL fButtonDown)
{
_DrawDay(hDC, lpCalInfo, iDayNumber, lpCalInfo->iStartDofW + iDayNumber,
         hBrushFace, ColorText, fButtonDown, TRUE);
}


void _DrawDay(HDC hDC, LPCALINFO lpCalInfo, int iDayNumber, int iBoxNumber,
              HBRUSH hBrushFace, COLORREF ColorText, BOOL fButtonDown,
              BOOL fDrawButton)
{
COLORREF ColorOld;
RECT     Rect;
TCHAR    szDayStr[3];

DayBoxToRECT(lpCalInfo, &Rect, iBoxNumber);

if (fDrawButton)
   CalDrawButton(hDC, &Rect, hBrushFace, 1, fButtonDown);

// Draw the iDayNumber string at this box, with offsets:
SetBkMode(hDC, TRANSPARENT);

// Turn iDayNumber into a string:
IntToString(iDayNumber, szDayStr);

ColorOld = SetTextColor(hDC, ColorText);
DrawText(hDC, szDayStr, lstrlen(szDayStr), &Rect, DT_CENTER | DT_VCENTER |
     DT_SINGLELINE);
SetTextColor(hDC, ColorOld);
}


void DayFocusAt(HDC hDC, LPCALINFO lpCalInfo)
{
HBRUSH      hBrushFace;
HBITMAP     hBitmap;
COLORREF    Color;
COLORREF    ColorBk;
short       iDayLocalFocus;
RECT        aBox;
static WORD Bits[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

/**************************************************************
* If the current focus is the same as where it was last time,
* don't do anything: Clear the Old Focus:
**************************************************************/

if (lpCalInfo->iDayFocus != lpCalInfo->iPreviousDayFocus &&
    lpCalInfo->iPreviousDayFocus <= lpCalInfo->iDaysInThisMonth)
   {
   iDayLocalFocus = lpCalInfo->iPreviousDayFocus;

   hBrushFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   DrawDayAt(hDC, lpCalInfo, iDayLocalFocus, hBrushFace, RGBCOLOR_BLACK,
         FALSE);
   DeleteObject(hBrushFace);
   }

/***********************************************************************
* Draw the New Focus. Check that it is valid for this particular Month
***********************************************************************/

iDayLocalFocus = lpCalInfo->iDayFocus;

/**************************************************************************
* If the Day Focus has been set, check that it hasn't exceeded the number
* of days in this current Month. Set to the last Day if it has:
**************************************************************************/

iDayLocalFocus = min(lpCalInfo->iDayFocus, lpCalInfo->iDaysInThisMonth);
lpCalInfo->iDayFocus = iDayLocalFocus;

DayBoxToRECT(lpCalInfo, &aBox, lpCalInfo->iStartDofW + iDayLocalFocus);

hBitmap = CreateBitmap(8, 8, 1, 1, (LPVOID)Bits);
hBrushFace = CreatePatternBrush(hBitmap);
Color = SetTextColor(hDC, RGBCOLOR_PALEGRAY);
ColorBk = SetBkColor(hDC, RGBCOLOR_WHITE);
DrawDayAt(hDC, lpCalInfo, iDayLocalFocus, hBrushFace, RGBCOLOR_DARKRED, TRUE);
DeleteObject(hBrushFace);
DeleteObject(hBitmap);
SetTextColor(hDC, Color);
SetBkColor(hDC, ColorBk);

/****************************************************************
* Make this Focus the Old Focus, so it can be removed next time
****************************************************************/

lpCalInfo->iPreviousDayFocus = lpCalInfo->iDayFocus;
}


////////////////////////////////////////////////////////////////////////////////
int FAR PASCAL DayPointXYToBoxNumber(LPCALINFO lpCalInfo, int iX, int iY)
{
short dDay;

if (iY < lpCalInfo->iDaysRectTop)
   return (-1);

dDay = (((iX * 7) / lpCalInfo->iCalWidth) + 1) +
       ((((iY - lpCalInfo->iDaysRectTop) * 6) /
         (lpCalInfo->iDaysRectBottom - lpCalInfo->iDaysRectTop)) * 7);

if (dDay < lpCalInfo->iStartDofW + 1 ||
    dDay > lpCalInfo->iStartDofW + lpCalInfo->iDaysInThisMonth)
   return (-1);

return (dDay);
}


LPRECT CalGetYearRect(LPCALINFO lpCalInfo, LPRECT lpRect)
{
CopyRect(lpRect, &lpCalInfo->RectYearHeader);

return (lpRect);
}


LPRECT CalGetMonthRect(LPCALINFO lpCalInfo, LPRECT lpRect)
{
CopyRect(lpRect, &lpCalInfo->RectMonthHeader);

return (lpRect);
}


LPRECT DayBoxToRECT(LPCALINFO lpCalInfo, LPRECT aRect, int iBoxNumber)
{
iBoxNumber--;

aRect->left   = ((lpCalInfo->iCalWidth * (iBoxNumber % 7)) / 7);
aRect->right   = ((lpCalInfo->iCalWidth * ((iBoxNumber % 7) + 1)) / 7);

aRect->top    = lpCalInfo->iDaysRectTop +
                (((lpCalInfo->iDaysRectBottom - lpCalInfo->iDaysRectTop) *
                (iBoxNumber / 7)) / 6);
aRect->bottom = lpCalInfo->iDaysRectTop +
                (((lpCalInfo->iDaysRectBottom - lpCalInfo->iDaysRectTop) *
                ((iBoxNumber / 7) + 1)) / 6);

return (aRect);
}


////////////////////////////////////////////////////////////////////////////////
// Get the length of a string in pixels using the widths for each character
// from the global array CharWidthTable (or CharWidths as I've used):
int FAR PASCAL LineLength(LPCTSTR String, LPINT CharWidthTable)
{
int Char, Length = 0;

while (Char = *String++)
   if (Char >= FIRSTCHAR && Char <= LASTCHAR)
      Length += CharWidthTable[Char - FIRSTCHAR];

return Length;
}


// Get the Day of Week (0=Sunday, 6=Saturday a la UNIX) for a Date:
int FAR PASCAL DateGetWeekdayInt(LPTB_DATE da)
{
long  lJulian;
short DateJulianStartSave;

extern short DateJulianStart;

DateJulianStartSave = DateJulianStart;

DateJulianStart = LOYEAR;

lJulian = DateDMYToJulian(da);
DateJulianStart = DateJulianStartSave;
// Julian date uses 0 for Monday, but I'm using UNIX's 0 = Sunday, so add 1:
return (int)((lJulian + 1L) % 7L);
}

// Get the maximum days for a Month and year (don't need the day):
int FAR PASCAL DateGetDaysInMonth(LPTB_DATE da)
{
return CalMonth[da->nMonth - 1].nDays[ISLEAP(da->nYear)];
}


void CalSetMonth(LPCALINFO lpCalInfo)
{
CalSetMonth2(lpCalInfo, lpCalInfo->iMonth, lpCalInfo->iDayFocus,
             lpCalInfo->iYear);

lpCalInfo->iDayFocus = min(lpCalInfo->iDayFocus, lpCalInfo->iDaysInThisMonth);
}


void CalSetMonth2(LPCALINFO lpCalInfo, int iMonth, int iDay, int iYear)
{
TB_DATE Date;

Date.nDay   = 1;                // Set to first day of the Month
Date.nMonth = iMonth;
Date.nYear  = iYear;

lpCalInfo->iStartDofW = DateGetWeekdayInt(&Date);
lpCalInfo->iDaysInThisMonth = DateGetDaysInMonth(&Date);
lpCalInfo->iPreviousDayFocus  = iDay;
}

void CalDrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
  POINT pt[2] = {{x1, y1}, {x2, y2}};

  Polyline(hdc, pt, 2);
}
void CalDrawButton(HDC hDC, LPRECT lpRect, HBRUSH hBrush, short dShadowSize,
           BOOL fButtonDown)
{
#if SS_V80
	if (fUseEnhanced)
	{
		HBRUSH upperBrush1;
		HBRUSH upperBrush2;
		HBRUSH bottomBrush1;
		HBRUSH bottomBrush2;
//		HBRUSH borderBrush = CreateSolidBrush(RGB(255,0,0));
		HBRUSH borderBrush = CreateSolidBrush(CalButtonBorder);
        RECT topRC, bottomRC;
		int x = lpRect->left;
		int y = lpRect->top;
		int width = lpRect->right-lpRect->left;
		int height = lpRect->bottom-lpRect->top;
		if (fButtonDown)
		{
//			upperBrush1 = CreateSolidBrush(RGB(199, 217, 243));
//			upperBrush2 = CreateSolidBrush(RGB(195, 214, 242));
//			bottomBrush1 = CreateSolidBrush(RGB(146, 182, 232));
//			bottomBrush2 = CreateSolidBrush(RGB(190, 212, 241));
			upperBrush1 = CreateSolidBrush(CalUpperPushedStart);
			upperBrush2 = CreateSolidBrush(CalUpperPushedEnd);
			bottomBrush1 = CreateSolidBrush(CalLowerPushedStart);
			bottomBrush2 = CreateSolidBrush(CalLowerPushedEnd);
		}
		else
		{
			upperBrush1	= CreateSolidBrush(CalUpperNormalStart);
			upperBrush2 = CreateSolidBrush(CalUpperNormalEnd);
			bottomBrush1 = CreateSolidBrush(CalLowerNormalStart);
			bottomBrush2 = CreateSolidBrush(CalLowerNormalEnd);
//			upperBrush1	= CreateSolidBrush(RGB(216, 231, 250));
//			upperBrush2 = CreateSolidBrush(RGB(213, 229, 249));
//			bottomBrush1 = CreateSolidBrush(RGB(182, 210, 245));
//			bottomBrush2 = CreateSolidBrush(RGB(217, 231, 249));
		}


		topRC.left = lpRect->left;
		topRC.right = lpRect->right;
		topRC.top = lpRect->top;
		topRC.bottom = lpRect->top + ((lpRect->bottom-lpRect->top)/2);
		bottomRC.left = topRC.left;
		bottomRC.right = topRC.right;
		bottomRC.bottom = lpRect->bottom;
		bottomRC.top = topRC.bottom+1;

        fpGradientFill(hDC, topRC, upperBrush1, upperBrush2, GRADIENT_FILL_RECT_V);
        fpGradientFill(hDC, bottomRC, bottomBrush1, bottomBrush2, GRADIENT_FILL_RECT_V);
		//InflateRect(lpRect, 1, 1);
		FrameRect(hDC, lpRect, borderBrush);
		//InflateRect(lpRect, -1, -1);

		DeleteObject(borderBrush);
		DeleteObject(upperBrush1);
		DeleteObject(upperBrush2);
		DeleteObject(bottomBrush1);
		DeleteObject(bottomBrush2);

	}
	else
#endif
	{
		FillRect(hDC, lpRect, hBrush);

		if (dShadowSize)
			tbDrawShadows(hDC, lpRect, dShadowSize, GetSysColor(COLOR_BTNSHADOW),
                 fButtonDown ? RGBCOLOR_PALEGRAY : RGBCOLOR_WHITE, fButtonDown,
                 FALSE);
	}
}


void CalDraw(HWND hWnd, LPCALINFO lpCalInfo, int dAction)
{
RECT Rect;

switch (dAction)
   {
   case DRAW_EVERYTHING:
   case DRAW_MONTH:
   case DRAW_DAYS:
      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case DRAW_FOCUS:
      if (lpCalInfo->iPreviousDayFocus <= lpCalInfo->iDaysInThisMonth)
         {
         DayBoxToRECT(lpCalInfo, &Rect, lpCalInfo->iStartDofW +
                      lpCalInfo->iPreviousDayFocus);
         InvalidateRect(hWnd, &Rect, TRUE);
         }

      DayBoxToRECT(lpCalInfo, &Rect, lpCalInfo->iStartDofW +
                   lpCalInfo->iDayFocus);
      InvalidateRect(hWnd, &Rect, TRUE);

      break;
   }

lpCalInfo->iAction = dAction;
}


// Macros used to implement date formatting for international settings
//
#define DATEWINLEX_INVALID 0
#define DATEWINLEX_VALUE   100
#define DATEWINLEX_D       DATEWINLEX_VALUE+1
#define DATEWINLEX_DD      DATEWINLEX_VALUE+2
#define DATEWINLEX_DDD     DATEWINLEX_VALUE+3
#define DATEWINLEX_DDDD    DATEWINLEX_VALUE+4
#define DATEWINLEX_M       DATEWINLEX_VALUE+5
#define DATEWINLEX_MM      DATEWINLEX_VALUE+6
#define DATEWINLEX_MMM     DATEWINLEX_VALUE+7
#define DATEWINLEX_MMMM    DATEWINLEX_VALUE+8
#define DATEWINLEX_Y       DATEWINLEX_VALUE+9
#define DATEWINLEX_YY      DATEWINLEX_VALUE+10
#define DATEWINLEX_YYY     DATEWINLEX_VALUE+11
#define DATEWINLEX_YYYY    DATEWINLEX_VALUE+12
#define DATEWINLEX_WW      DATEWINLEX_VALUE+13 // (Added by Masanori Iwasa)
#define DATEWIN_DEFAULT    _T("mmmm d, yyyy")

LPCTSTR DateDayName(LPTB_DATE lpDate, BOOL bLong)
{
return (bLong ? DayLongNames[DateGetWeekdayInt(lpDate)] :
                DayShortNames[DateGetWeekdayInt(lpDate)]);
}

LPCTSTR DateMonthName(LPTB_DATE lpDate, BOOL bLong)
{
return (bLong ? MonthLongNames[lpDate->nMonth-1] :
                MonthShortNames[lpDate->nMonth-1]);
}

// not used by Spread, will not retrieve CalTextOverride
LPTSTR DLLENTRY DateGetWeekday(LPTB_DATE da, LPTSTR str)
{
StrCpy(str, DateDayName(da, TRUE));
return str;
}

// Quick itoa conversion for date functions
LPCTSTR DateQitoa(UINT i, UINT n, BOOL bPad)
{
static TCHAR szBuffer[5];
TCHAR szFormat[5];

// Be sure not too exceed sz limits
n = (n < 5 ? n : 5);
i = (i < 9999 ? i : 9999);

// Build format string
if (bPad)
    wsprintf(szFormat,_T("%%0%dd"), n);
else
    wsprintf(szFormat,_T("%%%dd"), n);

// Format integer
wsprintf(szBuffer,szFormat,i);
return szBuffer;
}

// DateWinLex does lexical analysis of date pictures from the [Intl] 
// section of WIN.INI
LPCTSTR DateWinLex (LPCTSTR lpszText, LPWORD lpwFormat, LPTSTR lpszValue)
{
//Modify by BOC 99.7.22 (hyt)------------------------
//for sometimes not display correct
#ifdef SPREAD_JPN
#define ISDATECHAR(ch) (StrChr(_T("YyMmDdWwGg"),ch))
#else
#define ISDATECHAR(ch) (StrChr(_T("YyMmDdWw"),ch))
#endif
char chDelimit, chType;
int  i;

chDelimit = 0;
i  = 0;

// If the next token is a value, process it
while (*lpszText && !ISDATECHAR(*lpszText))
   if ((!chDelimit && (*lpszText == '"' || *lpszText == '\''))
      || (chDelimit == *lpszText))
      {
      chDelimit = (chDelimit ? 0 : *lpszText);
      *lpszText++;
      }
   else
      *(lpszValue+(i++)) = *lpszText++;

// Did we find a value token?
if (i)
   {
   // Yes
   *lpwFormat = DATEWINLEX_VALUE;
   *(lpszValue+i) = 0;
   }
else
   {
   // otherwise compute the token
   // (d, dd, ddd, dddd, m, mm, mmm, mmmm, yy, yyyy)
   chType = toupper(*lpszText);
   while (*lpszText && toupper(*lpszText) == chType)
      { i++; lpszText++; }

   switch (chType)
      {
      case 'D' : *lpwFormat = DATEWINLEX_VALUE+i; break;
      case 'M' : *lpwFormat = DATEWINLEX_VALUE+i+4; break;
      case 'Y' : *lpwFormat = DATEWINLEX_VALUE+i+8; break;
      case 'W' : *lpwFormat = DATEWINLEX_VALUE+i+11; break;
#ifdef SPREAD_JPN
	  case 'G' : *lpwFormat = DATEWINLEX_VALUE+i+13; break;
#endif
      }

   *lpszValue = 0;
   }

#ifdef SPREAD_JPN
	if(*lpwFormat == DATEWINLEX_Y)
		*lpwFormat = DATEWINLEX_YY;
#endif
//End BOC---------------------------------------------------------------------

// Make sure we are not in the middle of a delimited string
// or that an invalid format was found.
if (chDelimit || *lpwFormat == DATEWINLEX_Y || *lpwFormat == DATEWINLEX_YYY)
   *lpwFormat = DATEWINLEX_INVALID;

// Return the rest of the format text string or NULL if end of string
return (*lpszText ? lpszText : NULL);
}

// DateWinFormat constructs a formatted date string given a picture.
// Refer to the chapter on International Applications in the Windows
// Programmer's Reference for details.
//
//    lpszText        buffer to receive the formatted date string
//    lpDate          TB_DATE structure with the date to format
//    lpszDateFormat  picture
//    lpCalInfo       calendar override info, long days, short days, etc.
//
void DLLENTRY DateWinFormat(LPTSTR lpszText, LPTB_DATE lpDate, LPCTSTR lpszDateFormat, LPVOID lpCalPtr)
{
TCHAR   szFormat[64], szValue[64];
LPCTSTR lpszFormat;
WORD    wLex;
TCHAR   szNengo[16];
LPCALINFO lpCalInfo = (LPCALINFO)lpCalPtr;

// If no format is specified, use the format in the [Intl] section
// of WIN.INI
if (lpszDateFormat == NULL)
   {
   GetProfileString(_T("Intl"), _T("sLongDate"), DATEWIN_DEFAULT, szFormat,
                    STRING_SIZE(szFormat));
   lpszFormat = szFormat;
   }
else
   lpszFormat = lpszDateFormat;

// Parse format string and build date text
*lpszText = 0;

while (lpszFormat)
   {
   lpszFormat = DateWinLex(lpszFormat, &wLex, szValue);
   switch (wLex)
      {
      case DATEWINLEX_VALUE   :
         lstrcat(lpszText,szValue);
         break;
      case DATEWINLEX_D       :
         lstrcat(lpszText,DateQitoa(lpDate->nDay,2,FALSE));
         break;
      case DATEWINLEX_DD      :
         lstrcat(lpszText,DateQitoa(lpDate->nDay,2,TRUE));
         break;
      case DATEWINLEX_DDD     :
#ifdef SS_V35
         if (lpCalInfo && lpCalInfo->calText.DayShortNames[DateGetWeekdayInt(lpDate)][0])
            lstrcat(lpszText,lpCalInfo->calText.DayShortNames[DateGetWeekdayInt(lpDate)]);
         else
#endif
            lstrcat(lpszText,DateDayName(lpDate,FALSE));
         break;
      case DATEWINLEX_DDDD    :
#ifdef SS_V35
         if (lpCalInfo && lpCalInfo->calText.DayLongNames[DateGetWeekdayInt(lpDate)][0])
            lstrcat(lpszText,lpCalInfo->calText.DayLongNames[DateGetWeekdayInt(lpDate)]);
         else
#endif
            lstrcat(lpszText,DateDayName(lpDate,TRUE));
         break;
      case DATEWINLEX_M       :
         lstrcat(lpszText,DateQitoa(lpDate->nMonth,2,FALSE));
         break;
      case DATEWINLEX_MM      :
         lstrcat(lpszText,DateQitoa(lpDate->nMonth,2,TRUE));
         break;
      case DATEWINLEX_MMM     :
#ifdef SS_V35
         if (lpCalInfo && lpCalInfo->calText.MonthShortNames[lpDate->nMonth-1][0])
            lstrcat(lpszText,lpCalInfo->calText.MonthShortNames[lpDate->nMonth-1]);
         else
#endif
            lstrcat(lpszText,DateMonthName(lpDate,FALSE));
         break;
      case DATEWINLEX_MMMM    :
#ifdef SS_V35
         if (lpCalInfo && lpCalInfo->calText.MonthLongNames[lpDate->nMonth-1][0])
            lstrcat(lpszText,lpCalInfo->calText.MonthLongNames[lpDate->nMonth-1]);
         else
#endif
            lstrcat(lpszText,DateMonthName(lpDate,TRUE));
         break;
      case DATEWINLEX_YY      :
         //- This placed was modified a little so that when Japanese
         //  Era is currently being used, the display for the calendar
         //  will also be accordingly. (Masanori Iwasa)
         if (nDateFormatGlobal < IDF_NYYMMDD)
            lstrcat(lpszText,&DateQitoa(lpDate->nYear,4,TRUE)[2]);
         else
            {
            GetNengoFromDate(lpDate, szNengo, 2);
            lstrcat(lpszText, szNengo);
            }

         break;

      case DATEWINLEX_YYYY    :
         //- This placed was modified a little so that when Japanese
         //  Era is currently being used, the display for the calendar
         //  will also be accordingly. (Masanori Iwasa)
         if (nDateFormatGlobal < IDF_NYYMMDD)
            lstrcat(lpszText,DateQitoa(lpDate->nYear,4,TRUE));
         else
            {
            GetNengoFromDate(lpDate, szNengo, 2);
            lstrcat(lpszText, szNengo);
            }

         break;

      //- Japanese .INI file uses 'WW to specify the token for
      //  day of week. (Masanori Iwasa)
      case DATEWINLEX_WW      :
         lstrcat(lpszText,_T("("));
#ifdef SS_V35
         if (lpCalInfo && lpCalInfo->calText.DayShortNames[DateGetWeekdayInt(lpDate)][0])
            lstrcat(lpszText,lpCalInfo->calText.DayShortNames[DateGetWeekdayInt(lpDate)]);
         else
#endif
            lstrcat(lpszText,DateDayName(lpDate,FALSE));
         lstrcat(lpszText,_T(")"));
         break;

      default:
         *lpszText = 0;
      }
   }
}

void CalDateChanged(HWND hWnd, LPCALINFO lpCalInfo)
{
TCHAR szText[64];
TB_DATE date;

date.nDay   = lpCalInfo->iDayFocus;
date.nMonth = lpCalInfo->iMonth;
date.nYear  = lpCalInfo->iYear;
#ifdef SS_V35
DateWinFormat(szText, (LPTB_DATE)&date, NULL, (LPVOID)lpCalInfo);
#else
DateWinFormat(szText, (LPTB_DATE)&date, NULL, NULL);
#endif

SetWindowText(GetParent(hWnd), szText);
}


BOOL CalIsValidDate(LPCALINFO lpCalInfo, int nMonth, int nDay, int nYear)
{
TB_DATE Date;
long    lDate;
long    lDateMin;
long    lDateMax;

Date.nMonth = nMonth;
Date.nDay = nDay;
Date.nYear = nYear;

lDate = DateDMYToJulian(&Date);

if (lpCalInfo->fCheckMin)
   {
   lDateMin = DateDMYToJulian(&lpCalInfo->DateMin);
   if (lDate < lDateMin)
      return (FALSE);
   }

if (lpCalInfo->fCheckMax)
   {
   lDateMax = DateDMYToJulian(&lpCalInfo->DateMax);
   if (lDate > lDateMax)
      return (FALSE);
   }

return (TRUE);
}


void DLLENTRY CalSetNames(LPCTSTR lpszDays, LPCTSTR lpszMonths)
{
DateSetNames (lpszDays, NULL, NULL, lpszMonths);
}


void DLLENTRY DateSetNames(LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                           LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths)
{
SSSetCalText(lpszShortDays, lpszLongDays, lpszShortMonths, lpszLongMonths,
             NULL, NULL);
}


void DLLENTRY SSSetCalText(LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                           LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths,
                           LPCTSTR lpszOkText, LPCTSTR lpszCancelText)
{
LPCTSTR lpPtr;
LPCTSTR lpPtrOld;
short   dCnt;

// Set short day names
if (lpszShortDays)
   {
   _fmemset(DayShortNames, '\0', sizeof(DayShortNames));

   for (dCnt = 0, lpPtr = lpszShortDays, lpPtrOld = lpPtr; dCnt < 7 &&
        (lpPtr = _ftcschr(lpPtr, '\t')); dCnt++, lpPtr++, lpPtrOld = lpPtr)
      {
      _fmemcpy(DayShortNames[dCnt], lpPtrOld,
               min((short)(lpPtr - lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      DayShortNames[dCnt][min((short)(lpPtr - lpPtrOld), DATENAMESZ)] = '\0';
      }

   if (lpPtrOld && dCnt < 7)
      {
      _fmemcpy(DayShortNames[dCnt], lpPtrOld,
               min(lstrlen(lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      DayShortNames[dCnt][min(lstrlen(lpPtrOld), DATENAMESZ)] = '\0';
      }
   }

// Set long day names
if (lpszLongDays)
   {
   _fmemset(DayLongNames, '\0', sizeof(DayLongNames));

   for (dCnt = 0, lpPtr = lpszLongDays, lpPtrOld = lpPtr; dCnt < 7 &&
        (lpPtr = _ftcschr(lpPtr, '\t')); dCnt++, lpPtr++, lpPtrOld = lpPtr)
      {
      _fmemcpy(DayLongNames[dCnt], lpPtrOld,
               min((short)(lpPtr - lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      DayLongNames[dCnt][min((short)(lpPtr - lpPtrOld), DATENAMESZ)] = '\0';
      }

   if (lpPtrOld && dCnt < 7)
      {
      _fmemcpy(DayLongNames[dCnt], lpPtrOld,
               min(lstrlen(lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      DayLongNames[dCnt][min(lstrlen(lpPtrOld), DATENAMESZ)] = '\0';
      }
   }

if (lpszShortMonths)
   {
   _fmemset(MonthShortNames, '\0', sizeof(MonthShortNames));

   for (dCnt = 0, lpPtr = lpszShortMonths, lpPtrOld = lpPtr; dCnt < 12 &&
        (lpPtr = _ftcschr(lpPtr, '\t')); dCnt++, lpPtr++, lpPtrOld = lpPtr)
      {                                        
      _fmemcpy(MonthShortNames[dCnt], lpPtrOld,
               min((short)(lpPtr - lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      MonthShortNames[dCnt][min((short)(lpPtr - lpPtrOld), DATENAMESZ)] = '\0';
      }

   if (lpPtrOld && dCnt < 12)
      {
      _fmemcpy(MonthShortNames[dCnt], lpPtrOld,
               min(lstrlen(lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      MonthShortNames[dCnt][min(lstrlen(lpPtrOld), DATENAMESZ)] = '\0';
      }
   }

if (lpszLongMonths)
   {
   _fmemset(MonthLongNames, '\0', sizeof(MonthLongNames));

   for (dCnt = 0, lpPtr = lpszLongMonths, lpPtrOld = lpPtr; dCnt < 12 &&
        (lpPtr = _ftcschr(lpPtr, '\t')); dCnt++, lpPtr++, lpPtrOld = lpPtr)
      {
      _fmemcpy(MonthLongNames[dCnt], lpPtrOld,
               min((short)(lpPtr - lpPtrOld), 14) * sizeof(TCHAR));
      MonthLongNames[dCnt][min((short)(lpPtr - lpPtrOld), 14)] = '\0';
      }

   if (lpPtrOld && dCnt < 12)
      {
      _fmemcpy(MonthLongNames[dCnt], lpPtrOld,
               min(lstrlen(lpPtrOld), 14) * sizeof(TCHAR));
      MonthLongNames[dCnt][min(lstrlen(lpPtrOld), 14)] = '\0';
      }
   }

if (lpszOkText)
   lstrcpyn(szOkText, lpszOkText, STRING_SIZE(szOkText));

if (lpszCancelText)
   lstrcpyn(szCancelText, lpszCancelText, STRING_SIZE(szCancelText));
}

//--------------------------------------------------------------------------------
// Reads in the resource strings that are used in the calendar. (Masanori Iwasa)
//--------------------------------------------------------------------------------
void LoadCalendarResource()
{
int         i;
TCHAR       strTemp[DATENAMESZ];
BOOL        fReadMonthLong = !MonthLongNames[0][0];
BOOL        fReadMonthShort = !MonthShortNames[0][0];
BOOL        fReadDayLong = !DayLongNames[0][0];
BOOL        fReadDayShort = !DayShortNames[0][0];

//- Reads in the Month names from the resource file.
if (fReadMonthLong || fReadMonthShort)
   for ( i = 0; i < 12; i++ )
      {
      LoadString( hDynamicInst, IDS_JANUARY + i + LANGUAGE_BASE,
                      strTemp, STRING_SIZE(strTemp)-1 );

      if (fReadMonthLong)
         lstrcpy((LPTSTR)MonthLongNames[i], &strTemp[4] );   //- Long Name
      if (fReadMonthShort)
         {
         strTemp[3] = 0;
         lstrcpy((LPTSTR)MonthShortNames[i], strTemp );      //- Short Name
         }
      }

//- Reads in the Day of week names from the resource file.
if (fReadDayLong || fReadDayShort)
   for ( i = 0; i < 7; i++ )
      {
      LoadString( hDynamicInst, IDS_SUNDAY + i + LANGUAGE_BASE,
                      strTemp, STRING_SIZE(strTemp)-1 );
      if (fReadDayLong)
         lstrcpy(DayLongNames[i], &strTemp[IsDbcsSeparator('?') ? 3 : 4] );  //- Long Name
      if (fReadDayShort)
         {
         strTemp[IsDbcsSeparator('?') ? 2 : 3] = '\0';
         lstrcpy(DayShortNames[i], strTemp ); //- Short Name
         }
      }
}
