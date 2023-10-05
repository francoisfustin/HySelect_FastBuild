/*
$Revision:   1.5  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITGEN/EDITGEN.C_V  $
 * 
 *    Rev 1.5   12 Jun 1990 14:11:02   Dirk
 * No change.
 * 
 *    Rev 1.4   04 Jun 1990 12:43:32   Randall
 * Fixed bug wherein WM_NCCREATE fell through into WM_DESTROY
 * 
 *    Rev 1.3   29 May 1990  9:38:38   Randall
 * 
 *    Rev 1.2   29 May 1990  9:20:18   Sonny
 * 
 *    Rev 1.1   21 May 1990 13:28:18   Randall
 * No change.
 * 
 *    Rev 1.0   11 May 1990 16:16:42   Bruce
 * Initial revision.
 * 
 *    Rev 1.0   07 May 1990 10:19:50   Randall
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "editgen.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editgen\editgen.h"
<-BORLAND*/
#endif
#include "..\..\spread\ss_w32s.h"

WINENTRY  EditGeneralWndProc
(
   HWND   hWnd,
   UINT   uMessage,
   WPARAM wParam,
   LPARAM lParam
)

{
#if defined(_WIN64) || defined(_IA64)
   LRESULT  lReturnCode = 0l;
#else
   LONG     lReturnCode = 0l;
#endif

   switch (uMessage)
   {
      case WM_NCCREATE:
         if (!GenCreateField(hWnd, lParam))
            return(0l);
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

      case WM_CREATE:
         EditSuspendNotify(hWnd, TRUE);
         GenInitializeField(hWnd, lParam);
         EditSuspendNotify(hWnd, FALSE);
         break;

		/* RFW - 6/3/09 - 19184 - Changed it to WM_NCDESTROY
      case WM_DESTROY:
		*/
      case WM_NCDESTROY:
         GenDestroyField(hWnd);
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;

#ifdef SS_WIN32S
      case EM_LIMITTEXT_WIN32S:
#endif      
      case EM_LIMITTEXT:
         GenLimitText(hWnd, wParam);
         break;

      default:
         lReturnCode = CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, uMessage, wParam, lParam);
         break;
   }

   return lReturnCode;
}

