#include "fptools.h"
#include "string.h"
                                    
static fDoubleClick = FALSE;

LONG FPLIB vbGenericProc (HCTL hCtl, HWND hWnd, WORD Msg, 
    LPWORD lpwParam, LPLONG lplParam, LONG (*lpCallBack[])(), 
    LONG (*lpDataAware)(), short EditEvent[], 
    BOOL FAR *lpfCallDef )
{
   VBPROPPARAMS PropParams;
   WORD wParam = *lpwParam;
   LONG lParam = *lplParam;
   FPCONTROL fpTemp = {hWnd, hCtl, 0L, 0L};
   
   *lpfCallDef = FALSE;
   
   switch (Msg)
      {
      case WM_CREATE:
         // VBM_CREATED message is not posted in VC
         if (fpVBGetVersion() < VB200_VERSION && fpVBGetMode((LONG)NULL) == MODE_DESIGN)
            PostMessage(hWnd, FPM_REDRAW, REDRAW_ON, TRUE);
         break;

      case WM_CHAR:
         if (EditEvent[VB_EVENT_KEYPRESS] != -1)
         {
         PARAMS1 params = { lpwParam, 0L };

         fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_KEYPRESS], &params);
         if (!(*lpwParam) || !IsWindow(hWnd))
            return 0L;
         }
         break;

      case WM_SYSKEYDOWN:                                    //A521
      case WM_KEYDOWN:
         if (EditEvent[VB_EVENT_KEYDOWN] != -1)
         {
         WORD    wShift = 0;
         PARAMS2 params = { &wShift, lpwParam, 0L };

         if (HIBYTE(GetKeyState(VK_SHIFT)))
            wShift |= 0x01;

         if (HIBYTE(GetKeyState(VK_CONTROL)))
            wShift |= 0x02;

         if (lParam & 0x20000000L)        // Alt
            wShift |= 0x04;

         fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_KEYDOWN], &params);
         if (!(*lpwParam) || !IsWindow(hWnd))
            return 0L;
         }
         break;  // was return 0 to remove double firing of key down events  CTF 1/12/95

      case WM_ENABLE:
      case WM_KEYUP:
      case WM_MOUSEMOVE:
         fpVBDefControlProc((LPFPCONTROL)&fpTemp, Msg, wParam, lParam);
         if (!IsWindow(hWnd))
            return 0;
         break;                       

      case WM_SETFOCUS:
      case WM_KILLFOCUS:
         if (fpVBGetMode((LONG)NULL) == MODE_DESIGN)
            return (0);
         else
            fpVBDefControlProc((LPFPCONTROL)&fpTemp, Msg, wParam, lParam);
         break;                       

      case WM_NCHITTEST:
      case WM_MOUSEACTIVATE:
         if (fpVBGetMode((LONG)NULL) == MODE_DESIGN)
            return fpVBDefControlProc((LPFPCONTROL)&fpTemp, Msg, wParam, lParam);
         break;                       

      //case VBM_LOADED:
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
   
      case VBM_WANTSPECIALKEY:    
         // only process key down to avoid duplicate messaging for auto advance   A601
         if (GetKeyState(wParam) >> 15)                                                                  //A601
           if (SendMessage (hWnd, FPM_AUTOADVANCE, wParam, 0L))        
              return 0;
         break;

      case VBM_DATA_AVAILABLE:
      case VBM_DATA_REQUEST:
         if (lpDataAware)
            return (lpDataAware (hWnd, hCtl, Msg, lParam));
         break;
         
      case FPM_BUTTONHIT:
         {
         short dIndex, dDir;                     
         PARAMS2 params = { &dIndex, &dDir, 0L };
         switch (wParam)
            {                                // Loyal Begin Changes
            case FPB_LEFT:      dDir = 1; break;
            case FPB_RIGHT:     dDir = 2; break;
            case FPB_UP :       dDir = 3; break;
            case FPB_DOWN:      dDir = 4; break;
            case FPB_POPUP:      dDir = 5; break;
            case FPB_DROPDOWN:  dDir = 6; break;
            }                                // Loyal End Changes
         dIndex = *(short FAR *)lParam;

         fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_BUTTONHIT], &params); 
         *(short far *)lParam = dIndex;
         return dDir;
         }
         break;

      case WM_LBUTTONDBLCLK:
      case WM_RBUTTONDBLCLK:
         {
         short dClick = (Msg == WM_LBUTTONDBLCLK ? 1 : 2);
         PARAMS1 params = { &dClick, 0L };
         fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_DOUBLECLICK], &params); 
         if (!dClick || !IsWindow(hWnd))
            return 0L;
         fDoubleClick = TRUE;
         }
         break;


      case WM_LBUTTONUP:
      case WM_LBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_RBUTTONDOWN:
         fpVBDefControlProc((LPFPCONTROL)&fpTemp, Msg, wParam, lParam);
         if (!IsWindow(hWnd))
            return 0;
         break;
/*
         {
         RECT rect;
         GetClientRect(hWnd, &rect);
         if (PtInRect(&rect, MAKEPOINT(lParam)))
            {
            short dClick = (Msg == WM_LBUTTONUP ? 1 : 2);
            PARAMS1 params = { &dClick, 0L };
            VBFireEvent(hCtl, EditEvent[VB_EVENT_CLICK], &params);
            if (!dClick || !IsWindow(hWnd))
               return 0L;
            } 
         }
*/

      case FPM_INVALIDDATA:
         {
         long hNextWnd;
         PARAMS1 params;
               
         hNextWnd = *(long far *)lParam;
         params.lpParam1 = &hNextWnd;
         params.Index = 0L;
         fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_INVALIDDATA], &params);
         *(long far *)lParam = hNextWnd;
         //if ((hNextWnd != 0) && (IsWindow(hWnd)))
         //   PostMessage (hWnd, FPM_INVALIDPOST, 0, hNextWnd);
         //else
         //   return 0;
         }
         break;
      
      case WM_COMMAND:
         switch (HIWORD(lParam))
            {
            case FPN_CHANGE:
               if (fpVBGetVersion() > VB100_VERSION)
                  fpVBSetControlFlags((LPFPCONTROL)&fpTemp, CTLFLG_DATACHANGED, CTLFLG_DATACHANGED);
               fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_CHANGE], NULL);
               if (!IsWindow(hWnd))         
                  return 0;
               break;
               
            case FPN_INSERTMODE:
            case FPN_OVERWRITEMODE:
                {
                short dMode = (HIWORD(lParam) == FPN_INSERTMODE ? 0 : 1);
                PARAMS1 params = { &dMode, 0L };
                fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_CHANGEMODE], &params); 
                // mapping mode for uniqueness from true and false return values         //A671
                if (dMode == 0)                                                                                         //A671
                  dMode = FPN_INSERTMODE;                                                                 //A671
                else                                                                                                          //A671
                  dMode = FPN_OVERWRITEMODE;                                                         //A671
                  
                return (LONG)dMode;
                }
            
            case FPN_INVALIDCHAR:
            case FPN_INVALIDACTION:
            case FPN_INVALIDMOVE: 
            case FPN_EXCEEDLIMIT:
            case FPN_BUTTONMIN:
            case FPN_BUTTONMAX:
            case FPN_OUTOFRANGE:
               {
               SHORT dAction, dBeep;
               PARAMS2 params = { &dBeep, &dAction, 0L };
               dBeep = (SendMessage (hWnd, FPM_AUTOBEEP, 0, 0L) ? (-1) : 0);
               switch (HIWORD(lParam))
                  {
                  case FPN_INVALIDCHAR:   dAction = 1; break;
                  case FPN_INVALIDMOVE:   dAction = 2; break;
                  case FPN_INVALIDACTION: dAction = 3; break;
                  case FPN_EXCEEDLIMIT:   dAction = 4; break;
                  case FPN_BUTTONMIN:     dAction = 5; break;
                  case FPN_BUTTONMAX:     dAction = 6; break;
                  case FPN_OUTOFRANGE:    dAction = 7; break;
				  case FPN_NOPOP:		  dAction = 8; break;
                  }
               fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_INVALIDACTION], &params);
               return (LONG)dBeep;
               }
            
            case FPN_CLICK:
            case FPN_LMOUSECLICK:
            case FPN_RMOUSECLICK:
               {
               short dClick = (HIWORD(lParam) == FPN_RMOUSECLICK ? 2 : 1);
               RECT rect;
               POINT pt;
               if (fDoubleClick)
                  {
                  fDoubleClick = FALSE;
                  return 0;
                  }
               GetWindowRect(hWnd, &rect);
               GetCursorPos(&pt);
               if (PtInRect(&rect, pt) || HIWORD(lParam) == FPN_CLICK)
                  {
                  PARAMS1 params = { &dClick, 0L };
                  fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_CLICK], &params); 
                  }
               else 
                  dClick = 0;   
               return dClick;
               }
            
            case FPN_AUTOADVANCENEXT:
            case FPN_AUTOADVANCEPREV:
            case FPN_AUTOADVANCEUP:
            case FPN_AUTOADVANCEDOWN:
               {  
               SHORT dDirection, dAdvance = -1;  
               PARAMS2 params = { &dAdvance, &dDirection, 0L };

               // next - 0, prev - 1
               switch (HIWORD(lParam))
                  {
                  case FPN_AUTOADVANCENEXT:
                     dDirection = 1; break;
                  case FPN_AUTOADVANCEPREV:
                     dDirection = 2; break;
                  case FPN_AUTOADVANCEDOWN:
                     dDirection = 3; break;
                  case FPN_AUTOADVANCEUP:
                     dDirection = 4; break;
                  }
               fpVBFireEvent((LPFPCONTROL)&fpTemp, EditEvent[VB_EVENT_ADVANCE], &params);
               // return 0 to advance, non-zero to cancel  
               return (dAdvance ? 0 : 1);
               }
            }
         break;
      }
   
   *lpfCallDef = TRUE;
   return 0;
}                             

