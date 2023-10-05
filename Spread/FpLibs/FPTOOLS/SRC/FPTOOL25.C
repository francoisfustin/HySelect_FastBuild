/*    PortTool v2.2     FPTOOL25.C          */

#include "fptools.h"
#include "string.h"
#include <tchar.h>

PROPINFO fpPropRedraw =
   {
   _T("Redraw"),
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };

long FPLIB FP_Redraw (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   if (fGetProp)
      *(int FAR *)lpPP->lParam = (SendMessage(lpPP->hWnd, FPM_REDRAW, REDRAW_QUERY, 0L) ? PROP_TRUE : 0);
   else
      {
      if ((int)lpPP->lParam)
         SendMessage(lpPP->hWnd, FPM_REDRAW, REDRAW_ON, TRUE);
      else
         SendMessage(lpPP->hWnd, FPM_REDRAW, REDRAW_OFF, 0);
      }
   return (0);
}

PROPINFO fpProphWnd =                                              // `p`n
   {
   _T("hWnd"),                                                       // `p`n
   DT_SHORT | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,              // `p`n
   0, 0, 0, NULL, 0                                                  // `p`n
   };

long FP_hWnd(LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
if (fGetProp)
   {
   *(HWND FAR *)lpPP->lParam = lpPP->hWnd;
   }
else
   {
   }

return (0);
}


