/*
$Revision:   1.12  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITTIME/EDITTIME.C_V  $
 *
 *    Rev 1.12   12 Jun 1990 14:08:44   Dirk
 * No change.
 *
 *    Rev 1.11   04 Jun 1990 12:44:24   Randall
 * Moved WM_NCCREATE to top of switch
 *
 *    Rev 1.10   24 May 1990 17:34:44   Randall
 * Load current time on F3
 *
 *    Rev 1.9   24 May 1990 12:01:04   Randall
 * Added TimeGetFormat
 *
 *    Rev 1.8   23 May 1990 16:49:10   Sonny
 *
 *    Rev 1.7   23 May 1990 12:13:08   Randall
 *
 *    Rev 1.6   23 May 1990  9:17:52   Randall
 * No change.
 *
 *    Rev 1.5   22 May 1990 15:46:16   Randall
 * Added support for TimeSetFormat
 *
 *    Rev 1.4   22 May 1990 12:45:22   Randall
 * Added SETRANGE handling
 *
 *    Rev 1.3   21 May 1990 13:29:54   Randall
 *
 *    Rev 1.2   17 May 1990 11:47:02   Randall
 * Notify EN_INVALIDDATE on SetText changed to EN_INVALIDSETTEXT
 *
 *    Rev 1.1   16 May 1990 16:15:32   Randall
 *
 *    Rev 1.0   11 May 1990 16:12:24   Bruce
 * Initial revision.
 *
 *    Rev 1.2   10 May 1990 10:38:38   Randall
 * No change.
 *
 *    Rev 1.1   30 Apr 1990 13:20:28   Bruce
 * No change.
 *
 *    Rev 1.0   23 Apr 1990 18:42:30   Sonny
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "edittime.h"
#include "..\..\..\vbx\stringrc.h"          //- Added by Charles Feng at FarPoint (SPREAD_JPN)
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "edittime\edittime.h"
//borland #include "..\..\..\stringrc.h"          //- Added by Charles Feng at FarPoint (SPREAD_JPN)
<-BORLAND*/
#endif

//- These variables are implemented here so that they will be intialized
//  only once when creating the date field, and prevents reading from
//  the resource file each time. Also, it is used to intergrate Japanese
//  separators. Charles Feng BOC
TCHAR   szHourGlobal[3];
TCHAR   szMinuteGlobal[3];
TCHAR   szSecondGlobal[3];
TCHAR   szAmGlobal[5];
TCHAR   szPmGlobal[5];

//#ifdef  BUGS
// Bug-001
extern  void    SS_BeepLowlevel(HWND hwnd);
//#endif

WINENTRY  EditTimeWndProc
(
   HWND   hWnd,
   UINT   uMessage,
   WPARAM wParam,
   LPARAM lParam
)

{
#if defined(_WIN64) || defined(_IA64)
   LRESULT   lReturnCode;
#else
   LONG   lReturnCode;
#endif

    switch (uMessage)
   {
      case WM_NCCREATE:
         if (!TimeCreateField (hWnd, lParam))
         {
            lReturnCode = 0l;
            break;                     
         }
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;                 

      case WM_CREATE:   
         EditSuspendNotify(hWnd, TRUE);
         TimeInitializeField (hWnd, lParam);
         EditSuspendNotify(hWnd, FALSE);

         //- Load the separator string from the resources file. Charles Feng BOC
         LoadString(hDynamicInst, IDS_SEPARATOR_HOUR + LANGUAGE_BASE, szHourGlobal,  STRING_SIZE(szHourGlobal));
         LoadString(hDynamicInst, IDS_SEPARATOR_MINUTE + LANGUAGE_BASE, szMinuteGlobal, STRING_SIZE(szMinuteGlobal));
         LoadString(hDynamicInst, IDS_SEPARATOR_SECOND + LANGUAGE_BASE, szSecondGlobal,   STRING_SIZE(szSecondGlobal));

         LoadString(hDynamicInst, IDS_TIME_AM + LANGUAGE_BASE, szAmGlobal,   STRING_SIZE(szAmGlobal));
         LoadString(hDynamicInst, IDS_TIME_PM + LANGUAGE_BASE, szPmGlobal,   STRING_SIZE(szPmGlobal));

         lReturnCode = 0l;
         break;

      case WM_SIZE:
         TimeSize(hWnd, lParam);
         return (0);

      case WM_ENABLE:
         TimeEnable(hWnd, wParam);
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_KEYDOWN:
         if (Edit_IsActionKey(SS_KBA_CURRENT, (WORD)wParam))
         {
            LoadCurrentTime (hWnd);
            EditFieldModifed(hWnd);
            lReturnCode = 1;
         }
         else if (wParam == VK_INSERT && !HIBYTE(GetKeyState(VK_CONTROL)) &&
                  !IsShiftDown(hWnd))
         {
//#ifdef  BUGS
// Bug-001
            SS_BeepLowlevel(hWnd);
//#else
//            MessageBeep (MB_OK);
//#endif
            lReturnCode = 0;
         }
         else if (!TimeProcessWMKeyDown(hWnd, wParam))
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage,
                                         wParam, lParam);
         break;

#ifndef _UNICODE
#ifdef  SPREAD_JPN
//- The time edit style in US version is always low case, so we added  the following 
//  code to handle the character which is inputed into the time edit and to see if 
//  it is US version(from resource file we know), we modify the character to low case.
//  Because there is a problem in KANJI process if the time edit style is always low case.
//  Charles.
      case WM_CHAR:
         // if this is USA version.
         if ( !_TIsDBCSLeadByte((BYTE)szHourGlobal[0]) ) {
             if( (TCHAR)wParam >= 'A' && (TCHAR)wParam <= 'Z' )
                #if defined(WIN32)
                wParam = (WPARAM)tolower((int)wParam);
                #else
                wParam = (WPARAM)(TCHAR)(long)AnsiLower(wParam);
                #endif
         }
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;
//-
#endif
#endif

      case EM_VALIDATE:
         if (lReturnCode = TimeDynamicValidate (hWnd, wParam, lParam))
            ;
            /*
            if (!(lReturnCode = TimeFinalValidate (hWnd)))
               NotifyParent (hWnd, EN_INVALIDDATA);
            */

         break;

      case WM_SETTEXT:
         if (lReturnCode = TimeStaticValidate (hWnd, &lParam))
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         else
            NotifyParent (hWnd, EN_INVALIDSETTEXT);
         break;

      case EM_SETRANGE:
         return TimeSetValidRange (hWnd, lParam);

      case EM_SETFORMAT:
         TimeSetFieldFormat (hWnd, lParam);
         break;

      case EM_GETFORMAT:
         lReturnCode = TimeGetFieldFormat(hWnd, lParam);
         break;

      case WM_SETFOCUS:
         if (GetWindowTimeField(hWnd))
            {
            if (!(GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC))
               SendMessage(hWnd, EM_SETCARETPOS, 0, 0L);

            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
            }

         break;

      case WM_KILLFOCUS:
         if (!TimeFinalValidate (hWnd))
            NotifyParent (hWnd, EN_INVALIDDATA);

         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_VSCROLL:
#ifdef WIN32
         TimeVScroll(hWnd, LOWORD(wParam));
#else
         TimeVScroll(hWnd, wParam);
#endif
         break;

		/* RFW - 6/3/09 - 19184 - Changed it to WM_NCDESTROY
      case WM_DESTROY:
		*/
      case WM_NCDESTROY:
         TimeDestroyField (hWnd);
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      default:
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;
   }

   return lReturnCode;
}
