/*
$Revision:   1.14  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITDATE/EDITDATE.C_V  $
 * 
 *    Rev 1.14   12 Jun 1990 14:09:16   Dirk
 * No change.
 * 
 *    Rev 1.13   07 Jun 1990 10:08:52   Steve
 * 
 *    Rev 1.12   06 Jun 1990 10:31:54   Steve
 * No change.
 * 
 *    Rev 1.11   05 Jun 1990 15:11:06   Randall
 * Added invocation of Popup Calendar on VK_F4 (or whatever).
 * 
 *    Rev 1.10   04 Jun 1990 12:43:02   Randall
 * Moved WM_NCCREATE to top of switch 
 * 
 *    Rev 1.9   28 May 1990 15:25:40   Randall
 * No change.
 * 
 *    Rev 1.8   24 May 1990 17:33:58   Randall
 * Load current date on F3
 * 
 *    Rev 1.7   24 May 1990 11:02:58   Randall
 * Added DateSet(Get)Format
 * 
 *    Rev 1.6   23 May 1990 16:48:16   Sonny
 * 
 *    Rev 1.5   23 May 1990  9:19:34   Randall
 * No change.
 * 
 *    Rev 1.4   21 May 1990 16:22:50   Randall
 * Added Date Range Checking
 * 
 *    Rev 1.3   21 May 1990 13:26:56   Randall
 * No change.
 * 
 *    Rev 1.2   17 May 1990 11:45:56   Randall
 * Notify EN_INVALIDDATA on SetText changed to EN_INVALIDSETTEXT
 * 
 *    Rev 1.1   14 May 1990 17:44:34   Randall
 * 
 *    Rev 1.0   11 May 1990 16:13:20   Bruce
 * Initial revision.
 * 
 *    Rev 1.2   07 May 1990 15:02:46   Steve
 * No change.
 * 
 *    Rev 1.1   30 Apr 1990 13:20:40   Bruce
 * No change.
 * 
 *    Rev 1.0   23 Apr 1990 18:41:02   Sonny
 * Initial revision.
*/


//---------*---------*---------*---------*---------*---------*---------*-----
// 
// UPDATE LOG:
//
//		RWP01	10.18.96	<JIS3305> Can't turn off the beep sound
//	
//---------*---------*---------*---------*---------*---------*---------*-----


#define  NOCOMM
#define  NOKANJI

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editdate.h"
#include "..\..\..\vbx\stringrc.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "editdate\editdate.h"
//borland #include "..\..\stringrc.h"
<-BORLAND*/
#endif

//- These variables are implemented here so that they will be intialized
//  only once when creating the date field, and prevents reading from
//  the resource file each time. Also, it is used to intergrate Japanese
//  separators. (Masanori Iwasa)
TCHAR   szYearGlobal[4];
TCHAR   szMonthGlobal[4];
TCHAR   szDayGlobal[4];

//- Global variable to hold era names (Masanori Iwasa)
extern  TCHAR NengoName[12][8];

//RWP01a
extern void SS_Beep2(HWND hWnd);
//RWP01a

//Add by BOC 1999.4.15(hyt) for waring
#ifdef SPREAD_JPN
 extern void SS_BeepLowlevel(HWND hWnd);
#endif

void EditDataCallCalendar(HWND hWnd);

WINENTRY  EditDateWndProc
(
   HWND   hWnd,
   UINT   uMessage,
   WPARAM wParam,
   LPARAM lParam
)

{
#if defined(_WIN64) || defined(_IA64)
	LRESULT lReturnCode;
#else
   LONG  lReturnCode;
#endif
   int   i;

   switch (uMessage)
   {
      case WM_NCCREATE:
         if (!DateCreateField (hWnd, lParam))
         {
            lReturnCode = 0l;
            break;
         }
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_CREATE:
         EditSuspendNotify(hWnd, TRUE);
         DateInitializeField (hWnd, lParam);
         EditSuspendNotify(hWnd, FALSE);

         //- Load the separator string from the resources file. (Masanori Iwasa)
         LoadString(hDynamicInst, IDS_SEPARATOR_YEAR +LANGUAGE_BASE, szYearGlobal,  STRING_SIZE(szYearGlobal)-1);
         LoadString(hDynamicInst, IDS_SEPARATOR_MONTH+LANGUAGE_BASE, szMonthGlobal, STRING_SIZE(szMonthGlobal)-1);
         LoadString(hDynamicInst, IDS_SEPARATOR_DAY  +LANGUAGE_BASE, szDayGlobal,   STRING_SIZE(szDayGlobal)-1);

         //- Load the Era names from the resource file. (Masanori Iwasa)       
         for ( i = 0; i < 4; i++ )
            {
            TCHAR    szTemp[36];
            
            LoadString(hDynamicInst, IDS_ERA_MEIJI + i + LANGUAGE_BASE, szTemp,  STRING_SIZE(szTemp)-1);
            //- Copy the full Era name and it starts from the 2 byte in the string
            lstrcpy((LPTSTR)NengoName[i + 8], &szTemp[1]);
            //- Copy the first part of the Era name and it's the 2 and 3 byte.
#ifdef _UNICODE
            szTemp[2] = '\0';
#else
            szTemp[3] = '\0';
#endif
            lstrcpy((LPTSTR)NengoName[i + 4], &szTemp[1]);
            //- Copy the Era name prefix in the string
            szTemp[1] = '\0';
            lstrcpy((LPTSTR)NengoName[i], (LPTSTR)szTemp);
            }

         lReturnCode = 0l;
         break;

      case WM_SIZE:
         DateSize(hWnd, lParam);
         return (0);

      case WM_ENABLE:
         DateEnable(hWnd, wParam);
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      //-------------------------------------------------------------------
      // This message had to be handled because there was a problem
      // setting the 'ES_UPPERCASE' style. (Masanori Iwasa)
      //-------------------------------------------------------------------
      case WM_CHAR:
         if( (BYTE)wParam >= 'a' && (BYTE)wParam <= 'z' )
            wParam &= 0xffdf;
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;


      case WM_KEYDOWN:
         if (Edit_IsActionKey(SS_KBA_CURRENT, (WORD)wParam))
         {
            LoadCurrentDate (hWnd);
            EditFieldModifed(hWnd);
			//Modify by BOC 99.7.13 (hyt)----------------------
			//Already notify by LoadCurrentDate should not call agagin
            //NotifyParent(hWnd, EN_CHANGE);
			//--------------------------------------------------
            lReturnCode = 1;
         }
         else if (Edit_IsActionKey(SS_KBA_POPUP, (WORD)wParam))
         {
            EditDataCallCalendar(hWnd);
            lReturnCode = 1;
         }
         else if (wParam == VK_INSERT && !HIBYTE(GetKeyState(VK_CONTROL)) &&
                  !IsShiftDown(hWnd))
         {
#ifdef SPREAD_JPN //MASA
// 96' 6/26 Modified by BOC Gao. changed MessageBeep to SS_BeepLowlevel
            SS_BeepLowlevel(hWnd);
// ------------------------<<
#else

//RWP01c
//-------*--------
//          MessageBeep (MB_OK);
//-------*--------
			SS_Beep2(hWnd);
//RWP01c

#endif
            lReturnCode = 0;
         }
         else if (!DateProcessWMKeyDown(hWnd, wParam))
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage,
                                         wParam, lParam);

         break;

      //- This custom message was added so that I can check the range
      //  according to era easily. (Masanori Iwasa)
      case UM_CHECKNENGO:
          //- Check if the era year is in the right range
          CheckNengoRange(hWnd);
          break;

      case EM_VALIDATE:
         if (lReturnCode = DateDynamicValidate (hWnd, wParam, lParam))
            ;
            /*
            if (!(lReturnCode = DateStringIsValidate(hWnd)))
               NotifyParent (hWnd, EN_INVALIDDATA);
            */
         break;

      case WM_SETTEXT:
         if (lReturnCode = DateStaticValidate (hWnd, &lParam))
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         else
            {
            NotifyParent (hWnd, EN_INVALIDSETTEXT);
            /*
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage,
                                         wParam, (long)(LPSTR)"");
            */
            }

         break;

      case EM_SETRANGE:
         return DateSetValidRange (hWnd, lParam);

      case EM_SETFORMAT:
         DateSetFieldFormat (hWnd, lParam);
         break;

      case EM_GETFORMAT:
         DateGetFieldFormat(hWnd, lParam);
         break;

      case WM_LBUTTONDBLCLK:
//         PostMessage(hWnd, DM_POSTCALENDAR, 0, 0L);
         EditDataCallCalendar(hWnd);
         return (0);

      case WM_SETFOCUS:
         if (GetWindowDateField(hWnd))
            {
            if (!(GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC))
               SendMessage(hWnd, EM_SETCARETPOS, 0, 0L);

            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
            }

         break;

      case WM_KILLFOCUS:
      // fix for 9570 -scl
      if( !GetParent((HWND)wParam) )
	  {
		 HWND hwnd = hWnd;
         do
		 {
           hwnd = GetParent(hwnd);
		   if( hwnd == (HWND)wParam )
			   return 0;
		 } while(hwnd);
	  }
         if (GetWindowDateField(hWnd))
            {
            if (!DateFinalValidate (hWnd))
               NotifyParent (hWnd, EN_INVALIDDATA);

            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage,
                                         wParam, lParam);
            }

         break;

      case WM_VSCROLL:
#ifdef WIN32
         DateVScroll(hWnd, LOWORD(wParam));
#else
         DateVScroll(hWnd, wParam);
#endif
         break;

		/* RFW - 6/3/09 - 19184 - Changed it to WM_NCDESTROY
      case WM_DESTROY:
		*/
      case WM_NCDESTROY:
         DateDestroyField (hWnd);
         if (IsWindow(hWnd))
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case DM_POSTCALENDAR:
         EditDataCallCalendar(hWnd);
         return (0);

	  case DM_SETENHANCEDCALENDAR:
		  {
         LPDATEFIELD lpField = LockDateField(hWnd);
         lpField->fEnhancedCalendar = (BOOL)lParam;
         UnlockDateField(hWnd);
		 break;
		  }
      case DM_SETTWODIGITYEARMAX:
         {
         LPDATEFIELD lpField = LockDateField(hWnd);
         lpField->nTwoDigitYearMax = (short)lParam;
         UnlockDateField(hWnd);
         }
         break;

      case DM_GETTWODIGITYEARMAX:
         {
         LPDATEFIELD lpField = LockDateField(hWnd);
         lReturnCode = lpField->nTwoDigitYearMax;
         UnlockDateField(hWnd);
         }
         break;

      default:
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;
   }

   return lReturnCode;
}


void EditDataCallCalendar(HWND hWnd)
{
BOOL fModified;
LPDATEFIELD lpField = LockDateField(hWnd);

EditSuspendNotify(hWnd, TRUE);
fModified = PopupCalendar(hWnd, lpField->fEnhancedCalendar);
UnlockDateField(hWnd);
EditSuspendNotify(hWnd, FALSE);

if (fModified)
   {
   EditFieldModifed(hWnd);
   NotifyParent(hWnd, EN_CHANGE);
   }
}
