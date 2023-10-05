
#define  NOCOMM
#define  NOKANJI

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <windowsx.h>

#include <toolbox.h>

#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editnum.h"

extern void SS_BeepLowlevel(HWND hwnd);

WINENTRY EditNumWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)

{
#if defined(_WIN64) || defined(_IA64)
   LRESULT lReturnCode = TRUE;
#else
   LONG lReturnCode = TRUE;
#endif

   switch (uMessage)
   {
      case WM_CREATE:   // Send the default picture text
			{
			LockNumHeader(lpInfo, hInfo, hWnd);
			if (lpInfo)
				{
				EditSuspendNotify(hWnd, TRUE);
				Num_SetZero(lpInfo);
				EditSuspendNotify(hWnd, FALSE);
				UnlockNumHeader(hInfo);
				}
			}
			lReturnCode = 0;
         break;

      case WM_SIZE:
         Num_ProcessWMSize(hWnd, lParam);
         return (0);

      case WM_ENABLE:
         Num_ProcessWMEnable(hWnd, wParam);
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_VSCROLL:
         Num_ProcessWMVScroll(hWnd, LOWORD(wParam));
         break;

      case WM_CHAR:
			if (wParam == 0x03)  // ctrl-C
	         return (CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam));

         if (wParam < 0x20)
            return(0l);       // don't pass it on

         if (!Num_ProcessWMChar(hWnd, wParam, lParam))
            return(0l);       // don't pass it on

         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

		/* RFW - 6/3/09 - 19184 - Changed it to WM_NCDESTROY
      case WM_DESTROY:
		*/
      case WM_NCDESTROY:
         Num_ProcessWMDestroy(hWnd);
         if (IsWindow(hWnd))
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case EM_VALIDATE:
         lReturnCode = Num_ProcessEMValidate(hWnd, wParam, lParam);
         return(lReturnCode);

      case WM_SETTEXT:
         lReturnCode = Num_ProcessWMSetText(hWnd, wParam, (LPTSTR)lParam);
         break;

      case WM_KILLFOCUS:
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         Num_ProcessWMKillFocus(hWnd);
         break;

      case WM_SETFOCUS:
         if (GetWindowNumField(hWnd))
            {
            lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
            if (!(GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC))
               {
               Num_SetInitialCaretPosition(hWnd);
#ifdef TBPRO
               Edit_SetSel(hWnd, 0, -1);
#endif
               }
            }

         return(lReturnCode);

      case EM_CARETPOS:
         lReturnCode = Num_ProcessEMCaretPos(hWnd, wParam, lParam);
         break;

      case WM_WININICHANGE:
         Num_ProcessWMIniChange(hWnd, wParam, lParam);
         break;

      case EM_SETFORMAT:
         return(Num_ProcessEMSetFormat(hWnd, wParam, lParam));

      case EM_GETFORMAT:
         return(Num_ProcessEMGetFormat(hWnd, lParam));

      case EM_EDIT_POSITIONAFTERPAINT:
         Num_SetInitialCaretPosition(hWnd);
         break;

      case EM_RESETPREVLEN:
         Num_ResetPrevLen(hWnd);
         break;

      case EM_REPLACETEXT:
         Num_ReplaceText(hWnd, lParam);
         return (TRUE);

      case WM_KEYDOWN:
         if (wParam == VK_INSERT && !HIBYTE(GetKeyState(VK_CONTROL)) &&
             !IsShiftDown(hWnd))
				{
            SS_BeepLowlevel(hWnd);
            return(0l);
				}

         if (Num_ProcessWMKeyDown(hWnd, wParam))
            return(1l);    // Don't send it to FieldMgr

         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

		// RFW - 8/13/04 - 15035
		case WM_KEYUP:
         Num_ProcessWMKeyUp(hWnd, wParam);
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
			break;

      case WM_LBUTTONDBLCLK:
         Edit_SetSel(hWnd, 0, -1);
         break;

      case WM_NCCREATE:
         if (!Num_ProcessWMCreate(hWnd, lParam))
            return(0l);    // fall through

       default:
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
   }
   return(lReturnCode);
}

