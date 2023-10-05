/*    PortTool v2.2     FPTOOL31.C          */

#include "fptools.h"
#include "string.h"
#include <tchar.h>

#ifndef FP_DLL  //not FP_DLL
TCHAR szControlTypes[] =                                                // `p`e
   {
   _T("0 - Normal\0")                                                      // `p`e
   _T("1 - Read Only\0")                                                   // `p`e
   _T("2 - Static\0")
   _T("3 - Button Edit\0")
   };
                                                                       // `p`n
PROPINFO fpPropControlType =                                           // `p`n
   {
   _T("ControlType"),                                                  // `p`n
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |     // `p`n
   PF_fNoInitDef,                                                      // `p`n
   0, 0, 0, szControlTypes, 3                                          // `p`n
   };

TCHAR szMMControlTypes[] =                                                // `p`e
   {
   _T("0 - Normal\0")                                                      // `p`e
   _T("1 - Read Only\0")                                                   // `p`e
   _T("2 - Static\0")
   };

PROPINFO fpPropMMControlType =                                           // `p`n
   {
   _T("ControlType"),                                                      // `p`n
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |     // `p`n
   PF_fNoInitDef,                                                      // `p`n
   0, 0, 0, szMMControlTypes, 2                                          // `p`n
   };
#endif  //not FP_DLL


long FPLIB FP_ControlType (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   if (fGetProp)
      {
      *(int FAR *)lpPP->lParam = (int)SendMessage(lpPP->hWnd, FPM_CONTROLTYPE, CTLT_GET, 0L);
      }
   else
      {
      SendMessage(lpPP->hWnd, FPM_CONTROLTYPE, CTLT_SET, (int)lpPP->lParam);
      }
   return (0);
}

