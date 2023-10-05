/*
$Revision:   1.9  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/EDITINT/EDITINT.C_V  $
 * 
 *    Rev 1.9   04 Dec 1990 13:43:58   Dirk
 * No change.
 * 
 *    Rev 1.8   12 Jun 1990 14:10:38   Dirk
 * No change.
 * 
 *    Rev 1.7   06 Jun 1990 17:50:46   Randall
 * Initialize to "0" on create
 * 
 *    Rev 1.6   29 May 1990  9:58:46   Randall
 * Pass KILLFOCUS on to EditFieldWndProc
 * 
 *    Rev 1.5   25 May 1990 17:57:54   Sonny
 * No change.
 * 
 *    Rev 1.4   24 May 1990 17:24:12   Sonny
 * 
 *    Rev 1.3   23 May 1990 16:49:52   Sonny
 * 
 *    Rev 1.2   21 May 1990 13:28:56   Randall
 * No change.
 * 
 *    Rev 1.1   15 May 1990 17:04:22   Sonny
 * 
 *    Rev 1.0   11 May 1990 16:14:08   Bruce
 * Initial revision.
 * 
 *    Rev 1.1   30 Apr 1990 13:21:22   Bruce
 * No change.
 * 
 *    Rev 1.0   23 Apr 1990 18:42:02   Sonny
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
// to include our struct defns and func defns

#define  DRV_EDIT

#include <windows.h>
#include <windowsx.h>
#include <toolbox.h>

// local to the library

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "editint.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editint\editint.h"
<-BORLAND*/
#endif

WINENTRY EditIntegerWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN64) || defined(_IA64)
   LRESULT  lRetCode;
#else
   LONG  lRetCode;
#endif

   switch (uMessage)
   {
      case WM_CREATE:
         EditSuspendNotify(hWnd, TRUE);
         ProcessIntWMCreate(hWnd, lParam);
         IntWMSetText(hWnd, (LPARAM)(LPCTSTR)"0");
         EditSuspendNotify(hWnd, FALSE);
         lRetCode = 0l;
         break;

      case WM_SIZE:
         IntSize(hWnd, lParam);
         return (0);

      case WM_ENABLE:
         IntEnable(hWnd, wParam);
         lRetCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_CHAR:
			if (wParam == 0x03)  // ctrl-C
	         return (CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam));

         if (wParam < 0x20)
            return(0l);       // don't pass it on

         if (!ProcessIntWMChar(hWnd, wParam, lParam))
            return(0l);       // don't pass it on
         lRetCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

		/* RFW - 6/3/09 - 19184 - Changed it to WM_NCDESTROY
      case WM_DESTROY:
		*/
      case WM_NCDESTROY:
         ProcessIntWMDestroy(hWnd);
         lRetCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_SETTEXT:
         if (!ProcessIntWMSetText(hWnd, 0, lParam))
            return(1l);

         lRetCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_KILLFOCUS:
         lRetCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         ProcessIntWMKillFocus(hWnd, wParam);
         break;

      case WM_SETFOCUS:
         if (GetWindowIntField(hWnd))
            {
            lRetCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage,
                                      wParam, lParam);
            if (!(GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC))
               {
               ProcessIntWMSetFocus(hWnd);
#ifdef TBPRO
               Edit_SetSel(hWnd, 0, -1);
#endif
               }
            }

         break;

      case WM_VSCROLL:
#ifdef WIN32
         IntVScroll(hWnd, LOWORD(wParam));
#else
         IntVScroll(hWnd, wParam);
#endif
         break;

      case EM_VALIDATE:
         lRetCode = ProcessIntEMValidate(hWnd, wParam, lParam);
         return(lRetCode);

      case EM_SETRANGE:
         return(ProcessIntEMRange(hWnd, wParam, lParam));

     case WM_KEYDOWN:
         if (!ProcessIntWMKeyDown(hWnd, wParam))
            return(0l);
         lRetCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_LBUTTONDBLCLK:
         Edit_SetSel(hWnd, 0, -1);
         break;

      case WM_NCCREATE:
         if (!ProcessIntWMNCCreate(hWnd, lParam))
         {
            lRetCode = 0l;
            break;
         } // fall through

      default:
         lRetCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
   }
   return(lRetCode);
}
