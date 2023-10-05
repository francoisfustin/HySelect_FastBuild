/*
$Revision:   1.8  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/EDITFLOA/EDITFLOA.C_V  $
 *
 *    Rev 1.8   04 Dec 1990 13:40:38   Dirk
 * No change.
 *
 *    Rev 1.7   12 Jun 1990 14:10:12   Dirk
 * No change.
 *
 *    Rev 1.6   24 May 1990 16:22:06   Sonny
 *
 *    Rev 1.5   23 May 1990 18:15:42   Sonny
 * No change.
 *
 *    Rev 1.4   23 May 1990 16:50:44   Sonny
 *
 *    Rev 1.3   22 May 1990 17:01:58   Sonny
 * No change.
 *
 *    Rev 1.2   21 May 1990 13:27:38   Randall
 * No change.
 *
 *    Rev 1.1   15 May 1990 17:05:08   Sonny
 * No change.
 *
 *    Rev 1.0   11 May 1990 16:17:26   Bruce
 * Initial revision.
 *
 *    Rev 1.3   09 May 1990 15:23:26   Randall
 * Correctly position caret on WM_SETFOCUS and VK_DELETE (when highlighted)
 *
 *    Rev 1.2   01 May 1990 10:39:02   Sonny
 *
 *    Rev 1.1   30 Apr 1990 13:21:00   Bruce
 * No change.
 *
 *    Rev 1.0   23 Apr 1990 18:41:30   Sonny
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <windowsx.h>

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

WINENTRY EditFloatWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)

{
#if defined(_WIN64) || defined(_IA64)
   LRESULT  lReturnCode = TRUE;
#else
   LONG  lReturnCode = TRUE;
#endif

   switch (uMessage)
   {
      case WM_CREATE:   // Send the default picture text
         EditSuspendNotify(hWnd, TRUE);
         SendFloatPictureText(hWnd);
         EditSuspendNotify(hWnd, FALSE);
         lReturnCode = 0l;
         break;

      case WM_CHAR:
			if (wParam == 0x03)  // ctrl-C
	         return (CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam));

         if (wParam < 0x20)
            return(0l);       // don't pass it on

         if (!ProcessFloatWMChar(hWnd, wParam, lParam))
            return(0l);       // don't pass it on

         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

		/* RFW - 6/3/09 - 19184 - Changed it to WM_NCDESTROY
      case WM_DESTROY:
		*/
      case WM_NCDESTROY:
         ProcessFloatWMDestroy(hWnd);
         if (IsWindow(hWnd))
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case EM_VALIDATE:
         lReturnCode = ProcessFloatEMValidate(hWnd, wParam, lParam);
         return(lReturnCode);

      case WM_SETTEXT:
         lReturnCode = ProcessFloatWMSetText(hWnd, wParam, (LPTSTR)lParam,
                                             FALSE, FALSE);
         break;

      case WM_KILLFOCUS:
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         ProcessFloatWMKillFocus(hWnd);
         break;

      case WM_SETFOCUS:
         if (GetWindowFloatField(hWnd))
            {
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
            if (!(GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC))
               {
               SetInitialCaretPosition (hWnd);
#ifdef TBPRO
               Edit_SetSel(hWnd, 0, -1);
#endif
               }
            }

         return(lReturnCode);

      case EM_CARETPOS:
         lReturnCode = ProcessFloatEMCaretPos(hWnd, wParam, lParam);
         break;

      case WM_WININICHANGE:
         ProcessWMIniChange(hWnd, wParam, lParam);
         break;

      case EM_SETRANGE:
         return(ProcessFloatEMRange(hWnd, wParam, lParam));

      case EM_SETFORMAT:
         return(ProcessEMSetFormat(hWnd, wParam, lParam));

      case EM_GETFORMAT:
         return(ProcessEMGetFormat(hWnd, lParam));

      case EM_EDIT_POSITIONAFTERPAINT:
         SetInitialCaretPosition (hWnd);
         break;

      case EM_RESETPREVLEN:
         FloatResetPrevLen(hWnd);
         break;

      case EM_REPLACETEXT:
         FloatReplaceText(hWnd, lParam);
         return (TRUE);

      case WM_KEYDOWN:
         if (wParam == VK_INSERT && !HIBYTE(GetKeyState(VK_CONTROL)) &&
             !IsShiftDown(hWnd))
         {
//#ifdef  BUGS
// Bug-001
            SS_BeepLowlevel(hWnd);
//#else
//            MessageBeep(1);
//#endif
            return(0l);
         }
         if (ProcessFloatWMKeyDown(hWnd, wParam))
            return(1l);    // Don't send it to FieldMgr

         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

		// RFW - 2/2/05 - 15673
		case WM_KEYUP:
         ProcessFloatWMKeyUp(hWnd, wParam);
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
			break;

      case WM_LBUTTONDBLCLK:
         Edit_SetSel(hWnd, 0, -1);
         break;

      case WM_NCCREATE:
         if (!ProcessFloatWMCreate(hWnd, lParam))
            return(0l);    // fall through

       default:
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
   }
   return(lReturnCode);
}

