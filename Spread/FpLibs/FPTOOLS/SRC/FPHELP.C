//
// FarPoint Help Utilities
//
#include "fptools.h"
#include "string.h"
#include <tchar.h>

BOOL FPLIB fpWinHelp (LPTSTR lpszHelpFile, HWND hWnd, WORD wParam, PPROPINFO Properties[], PEVENTINFO Events[], int fLastVB, long lOffset)
{
   BYTE index = HIBYTE(wParam);
   BYTE helpType = LOBYTE(wParam);
   HGLOBAL hString;

   switch (helpType)
      {
      case VBHELP_PROP:
         if (Properties[index] < PPROPINFO_STD_LAST)
            {
//            WinHelp(hWnd, lpszHelpFile, HELP_CONTEXT, lOffset+HIBYTE(wParam));

            hString = (HGLOBAL)GlobalAlloc(GHND, (lstrlen(Properties[index]->npszName) + 16) * sizeof(TCHAR));

            if (hString != NULL)
            {
               LPTSTR lpHelpTopic = (LPTSTR)GlobalLock (hString);
               if (lpHelpTopic)
               {                          
                  lstrcpy(lpHelpTopic, Properties[index]->npszName);
                  lstrcat(lpHelpTopic, _T(" property"));
#if defined(_WIN64) || defined(_IA64)
                  WinHelp(hWnd, lpszHelpFile, HELP_KEY, (ULONG_PTR)(LPTSTR)lpHelpTopic);
#else
                  WinHelp(hWnd, lpszHelpFile, HELP_KEY, (DWORD)(LPTSTR)lpHelpTopic);
#endif
                  GlobalUnlock(hString);
               }                                   
               GlobalFree(hString);
            }
            return TRUE;
            }
         break;
      case VBHELP_EVT:
         if (Events[index] < PEVENTINFO_STD_LAST)
         {
             if ((!lstrcmpi(Events[index]->npszName, _T("KeyPress")))   ||            //A521
                 (!lstrcmpi(Events[index]->npszName, _T("KeyDown"))))               //A521
               break;                                                                                     //A521
               
            hString = (HGLOBAL)GlobalAlloc(GHND, (lstrlen(Events[index]->npszName) + 16) * sizeof(TCHAR));

            if (hString != NULL)
            {
               LPTSTR lpHelpTopic = (LPTSTR)GlobalLock (hString);
               if (lpHelpTopic)
               {                          
                  lstrcpy(lpHelpTopic, Events[index]->npszName);
                  lstrcat(lpHelpTopic, _T(" event"));
                  
#if defined(_WIN64) || defined(_IA64)
                  WinHelp(hWnd, lpszHelpFile, HELP_KEY, (ULONG_PTR)(LPTSTR)lpHelpTopic);
#else
                  WinHelp(hWnd, lpszHelpFile, HELP_KEY, (DWORD)(LPTSTR)lpHelpTopic);
#endif
                  GlobalUnlock(hString);
               }                                   
               GlobalFree(hString);
            }
            return TRUE;
         }
         break;   
      case VBHELP_CTL:
         WinHelp(hWnd, lpszHelpFile, HELP_CONTEXT, lOffset);
         return TRUE;
      }
   return FALSE;
}
