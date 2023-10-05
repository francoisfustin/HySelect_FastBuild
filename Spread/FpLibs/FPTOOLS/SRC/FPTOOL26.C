#include "fptools.h"

LONG FPLIB vbBasicProc (HCTL hCtl, HWND hWnd, WORD Msg, WORD wParam, 
    LONG lParam, LONG (*lpCallBack[])(), LONG (*lpDataAware)(), 
    BOOL FAR *lpfCallDef )
{
   VBPROPPARAMS PropParams;
   FPCONTROL fpTemp = {hWnd, hCtl, 0L, 0L};

   *lpfCallDef = FALSE;
   
   switch (Msg)
      {
      case WM_CREATE:
         // VBM_CREATED message is not posted in VC
         if (fpVBGetVersion() < VB200_VERSION)
            SendMessage(hWnd, FPM_REDRAW, REDRAW_ON, TRUE);
         break;

      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_CHAR:
      case WM_LBUTTONDOWN:
      case WM_MOUSEMOVE:
         fpVBDefControlProc((LPFPCONTROL)&fpTemp, Msg, wParam, lParam);
         break;                       

      case WM_SETFOCUS:
      case WM_KILLFOCUS:
         if (fpVBGetMode((LONG)NULL) == MODE_DESIGN)
            return (0);
         else
            fpVBDefControlProc((LPFPCONTROL)&fpTemp, Msg, wParam, lParam);
         break;                       

      case WM_MOUSEACTIVATE:
         if (fpVBGetMode((LONG)NULL) == MODE_DESIGN)
            return (0);
         break;                       

      case VBM_LOADED:
      case VBM_CREATED:            
         SendMessage(hWnd, FPM_REDRAW, REDRAW_ON, TRUE);
         break;
         
      case VBM_SETPROPERTY:
      case VBM_GETPROPERTY:
         if(lpCallBack[wParam])
            {
            PropParams.hCtl   = hCtl;
            PropParams.hWnd   = hWnd;
            PropParams.Msg    = Msg;
            PropParams.wParam = wParam;
            PropParams.lParam = lParam;

            return ((lpCallBack[wParam])((LPVBPROPPARAMS)&PropParams,
               Msg == VBM_SETPROPERTY ? FALSE : TRUE));
            }
         break;
   
      case VBM_DATA_AVAILABLE:
      case VBM_DATA_REQUEST:
         if (lpDataAware)
            return (lpDataAware (hWnd, hCtl, Msg, lParam));
         break;
         
      }
   *lpfCallDef = TRUE;
   return 0;
}                             
