/*
$Revision:   1.9  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/EDITFLD.C_V  $
 * 
 *    Rev 1.9   07 Aug 1990 17:11:22   Randall
 * No change.
 * 
 *    Rev 1.8   22 Jun 1990 17:25:14   Dirk
 * 
 *    Rev 1.7   12 Jun 1990 14:07:48   Dirk
 * No change.
 * 
 *    Rev 1.6   04 Jun 1990 12:42:22   Randall
 * Moved WM_NCCREATE to top of switch
 * 
 *    Rev 1.5   23 May 1990 16:46:20   Sonny
 * 
 *    Rev 1.4   21 May 1990 13:26:32   Randall
 * 
 *    Rev 1.3   18 May 1990 17:13:18   Randall
 * Process WM_CLEAR
 * 
 *    Rev 1.2   15 May 1990 15:24:18   Randall
 * No change.
 * 
 *    Rev 1.1   14 May 1990 11:15:54   Randall
 * Added AdjustLength
 * 
 *    Rev 1.0   11 May 1990 16:10:58   Bruce
 * Initial revision.
 * 
 *    Rev 1.3   02 May 1990 19:01:14   Randall
 * Removed unused lParam from MouseUp call
 * 
 *    Rev 1.2   30 Apr 1990 13:20:04   Bruce
 * No change.
 * 
 *    Rev 1.1   23 Apr 1990 20:09:30   Sonny
 * No change.
 * 
 *    Rev 1.0   23 Apr 1990 18:40:18   Sonny
 * Initial revision.
*/

//#define  NOCOMM
//#define  NOKANJI
#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <windowsx.h>
#include <toolbox.h>

#include "..\libgen.h"
#include "editfld.h"
#include "..\spread\ss_w32s.h"

WINENTRY EditFieldWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN64) || defined(_IA64)
   LRESULT  lReturnCode = 0l;
#else
   long  lReturnCode = 0l;
#endif
   TCHAR cCharacter;
   WORD  wKey;

   switch (uMessage)
   {
      // WM_...   Windows Messages.

      case WM_NCCREATE:
         if (CreateField (hWnd, lParam))
            lReturnCode = DefWindowProc(hWnd, uMessage, wParam, lParam);
         else
            lReturnCode = 0l;
         break;

      case WM_SIZE:
			ProcessSize(hWnd, lParam);
         break;

		/* RFW - 6/3/09 - 19184 - Changed it to WM_NCDESTROY
      case WM_DESTROY:
		*/
      case WM_NCDESTROY:
         DestroyField (hWnd);
         return (DefWindowProc(hWnd, uMessage, wParam, lParam));
         break;

      case WM_CHAR:
         cCharacter = ProcessCharacter (hWnd, wParam);
         break;

      case WM_KEYDOWN:
         wKey = ProcessKeystroke (hWnd, wParam);
         break;

      case WM_KEYUP:
         wKey = ProcessKeyUp (hWnd, wParam);
         break;

      case WM_ENABLE:
         InvalidateRect(hWnd, NULL, TRUE);
         UpdateWindow(hWnd);
         break;

      case WM_ERASEBKGND:
         return (TRUE);

      case WM_PAINT:
         PaintField(hWnd, (HDC)wParam);
         break;

      case WM_GETDLGCODE:
         if (GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC)
            lReturnCode = DLGC_STATIC;
         else
            lReturnCode = EDIT_FIELD_CODES;

         break;

      case WM_NCHITTEST:
         if (GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC)
            return (HTTRANSPARENT);

         return (DefWindowProc(hWnd, uMessage, wParam, lParam));

      case WM_SETFONT:
         ProcessEditSetFont(hWnd, wParam, lParam);
         break;

      case WM_GETFONT:
         return (LRESULT)ProcessEditGetFont(hWnd);

      case WM_MOUSEACTIVATE:
         if (!(GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC))
            SetFocus (hWnd);

         break;

      case WM_LBUTTONDOWN:
         MouseDown (hWnd, lParam);
         break;

      case WM_LBUTTONDBLCLK:
         Edit_SetSel(hWnd, 0, -1);
         break;

      case WM_MOUSEMOVE:
         MouseMove (hWnd, lParam);
         break;

      case WM_LBUTTONUP:
         MouseUp (hWnd);
         break;

      case WM_SETFOCUS:
         if (GetWindowEditField(hWnd))
            {
            if (!(GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC))
               OpenField (hWnd);
            }

         break;

      case WM_KILLFOCUS:
         CloseField (hWnd);
         break;

      case WM_SETTEXT:
         SetFieldText (hWnd, lParam);
         break;

      case WM_GETTEXT:
         lReturnCode = (long)GetFieldText(hWnd, wParam, lParam);
         break;

      case WM_GETTEXTLENGTH:
         lReturnCode = GetTextLength (hWnd);
         break;

      case WM_CLEAR:
         ClearSelection(hWnd);
         lReturnCode = 1;
         break;

      case WM_CUT:
         ClipboardCut(hWnd);
         lReturnCode = 1;
         break;

      case WM_COPY:
         ClipboardCopy(hWnd);
         lReturnCode = 1;
         break;

      case WM_PASTE:
         ClipboardPaste(hWnd);
         lReturnCode = 1;
         break;

      // EM_...   Windows dialog edit class messages.

#ifdef SS_WIN32S
      case EM_GETMODIFY_WIN32S:
#endif
      case EM_GETMODIFY:
         lReturnCode = GetModify(hWnd);
         break;

#ifdef SS_WIN32S
      case EM_SETMODIFY_WIN32S:
#endif
      case EM_SETMODIFY:
         SetModify(hWnd, (BOOL)wParam);
         break;

#ifdef SS_WIN32S
      case EM_SETSEL_WIN32S:
         SetSelection(hWnd, (int)LOWORD(lParam), (int)HIWORD(lParam));
         break;
#endif

      case EM_SETSEL:
#ifdef WIN32
         SetSelection (hWnd, (int)wParam, (int)lParam);
#else
         SetSelection (hWnd, (int)LOWORD(lParam), (int)HIWORD(lParam));
#endif
         break;

#ifdef SS_WIN32S
      case EM_REPLACESEL_WIN32S:
#endif
      case EM_REPLACESEL:
         lReturnCode = ReplaceSel(hWnd, lParam);
         break;

#ifdef SS_WIN32S
      case EM_GETSEL_WIN32S:
#endif
      case EM_GETSEL:
         lReturnCode = GetSelection (hWnd);
         break;


      // Additional EM_... messages defined by us, in TOOLBOX.H

      case EM_VALIDATE:
         lReturnCode = (LONG) TRUE;
         break;

      case EM_CARETPOS:
         lReturnCode = CaretPosition(hWnd, wParam, lParam);
         break;

      case EM_SETCARETPOS:
         SetCaretPosition (hWnd, wParam);
         break;

      case EM_GETCARETPOS:
         lReturnCode = (LONG) GetCaretPosition (hWnd);
         break;

      // Private ToolBox messages defined in LIBGEN.H

      case EM_EDIT_POSITIONAFTERPAINT:
         PositionCaretAfterPaint (hWnd);
         break;

      case EM_EDIT_ADJUSTLENGTH:
         AdjustLength (hWnd);
         break;

      case EM_GET3D:
         lReturnCode = EditGet3DParams(hWnd, lParam);
         break;

      case EM_SET3D:
         EditSet3DParams(hWnd, wParam, lParam);
         break;

		case EM_GETRECT:
			EditGetRect(hWnd, lParam);
			break;

		case EM_CHARFROMPOS:
			lReturnCode = EditGetCharFromPos(hWnd, lParam);
			break;

      default:
         return (DefWindowProc(hWnd, uMessage, wParam, lParam));
   }

   return lReturnCode;
}
