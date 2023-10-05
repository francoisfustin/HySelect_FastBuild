#define  NOCOMM
#define  NOKANJI

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "editpic.h"
#include "..\editfld.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editpic\editpic.h"
//borland #include "editfld.h"
<-BORLAND*/
#endif

//#ifdef  BUGS
// Bug-001
void SS_BeepLowlevel(HWND hWnd);
//#endif

WINENTRY  EditPicWndProc
(
HWND   hWnd,
UINT   uMessage,
WPARAM wParam,
LPARAM lParam
)

{
switch (uMessage)
   {
   case WM_NCCREATE:
      if (!PicCreateField(hWnd, lParam))
         return (0);

      break;

   case WM_CREATE:
      EditSuspendNotify(hWnd, TRUE);
      PicInitializeField(hWnd, lParam);
      EditSuspendNotify(hWnd, FALSE);
      return (0);

   case WM_KEYDOWN:
      if (wParam == VK_INSERT && !HIBYTE(GetKeyState(VK_CONTROL)) &&
          !IsShiftDown(hWnd))
         {
//#ifdef  BUGS
// Bug-001
         SS_BeepLowlevel(hWnd);
//#else
//         MessageBeep(MB_OK);
//#endif
         return (0);
         }

      else if (PicProcessKey(hWnd, (WORD)wParam, IsShiftDown(hWnd)))
         return (0);

      break;

   case WM_CHAR:
		// RFW - 8/21/07 - 20795
		if (wParam == 0x03)  // ctrl-C
	      return (CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam));

      if (wParam < 0x20)
         return(0l);       // don't pass it on

//#ifdef  SPREAD_JPN
// We can get rid of this preprocessor
      {
      MSG KanJi_Msg;

          if (_TIsDBCSLeadByte((BYTE)wParam)) {
              PeekMessage(&KanJi_Msg,hWnd,WM_CHAR,WM_CHAR,PM_REMOVE);
              wParam = wParam + (KanJi_Msg.wParam << 8);
          }
      }
//#endif

      PicProcessCharacter(hWnd, wParam);
      return (0);

   case WM_SETTEXT:
      PicSetText(hWnd, (LPTSTR)lParam, NULL, FALSE);
      return (0);

   case WM_SETFOCUS:
      if (GetWindowPicField(hWnd))
         PicProcessKey(hWnd, VK_HOME, FALSE);

      break;

   case WM_KILLFOCUS:
      break;

   case WM_CUT:
      PicClipboardCut(hWnd);
      return (1);

   case WM_PASTE:
      PicClipboardPaste(hWnd);
      return (1);

   case EM_SETSEL:
#ifdef WIN32
      PicSetSelection(hWnd, (int)wParam, (int)lParam);
#else
      PicSetSelection(hWnd, (int)LOWORD(lParam), (int)HIWORD(lParam));
#endif
      return (0);

   case EM_EDIT_POSITIONAFTERPAINT:
      return(0);

   case EPM_SETMASK:
      PicSetMask(hWnd, (LPTSTR)lParam);
      break;

   case EPM_GETMASK:
      PicGetMask(hWnd, (LPTSTR)lParam, (WORD)wParam);
      break;

   case EPM_SETCALLBACK:
      PicSetCallBack(hWnd, (FARPROC)lParam);
      break;

   case EPM_SETVALUE:
      PicSetText(hWnd, (LPTSTR)lParam, NULL, TRUE);
      return (0);

   case EPM_GETVALUE:
      PicGetValue(hWnd, (LPTSTR)lParam);
      return (0);

	/* RFW - 6/3/09 - 19184 - Changed it to WM_NCDESTROY
   case WM_DESTROY:
	*/
   case WM_NCDESTROY:
      PicDestroyField(hWnd);
      break;

   default:
      break;
   }

return (CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam));
}
