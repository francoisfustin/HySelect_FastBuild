#include "windows.h"
#include "string.h"
#include "fptools.h"
#include "fpmemmgr.h"
#include "vbssprvw.h"
#include "ssprvw.h"

extern HANDLE fpInstance;
extern PROPINFO PropNULL;

extern void VBSS_WinHelp(HWND hWnd, long lIndex);
extern void VBSS_About(HWND hWndSS);

PPROPINFO PV_Properties_VB1[PVPROP_PROPCNT + 1];
PPROPINFO PV_Properties[PVPROP_PROPCNT + 1];

MODEL PV_Model;
MODEL PV_Model_VB1;

LONG FAR PASCAL _export vbFPSSPreviewProc(HCTL hCtl, HWND hWnd, WORD Msg,
                                          WORD wParam, LONG lParam);
static long FP_SSPreviewCaption (LPVBPROPPARAMS lpPP, BOOL fGetProp);


HWND NEAR HwndInitAboutBox(VOID);
LONG _export FAR PASCAL AboutBoxProc(HWND hWnd, UINT msg, WPARAM wParam,
                                     LPARAM lParam);
long fpVBSetProperty(HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (*lpPropProc)(LPVOID, WPARAM, LPARAM),
                     LPFP_PROPLIST lpPropList);
long fpVBGetProperty(HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (*lpPropProc)(LPVOID, WPARAM, LPARAM),
                     LPFP_PROPLIST lpPropList);
void VBPV_InitModel(USHORT usVersion);


LONG FAR PASCAL _export vbFPSSPreviewProc(HCTL hCtl, HWND hWnd, WORD Msg,
                                        WORD wParam, LONG lParam)
{
BOOL fCallDef = TRUE;
LONG lRet = 0;
LONG lCode = 0;

switch (Msg)
   {
   case WM_NCCREATE:
      {
      LPVBFPSSPREVIEW lpVBSSPreview = VBDerefControl(hCtl);
      if (lpVBSSPreview)
         {
         LPSSPREVIEW lpSSPreview;

         lpVBSSPreview->hClassData = SSPreviewCreate(hWnd, GetWindowLong(hWnd,
                                                     GWL_STYLE));
         lpVBSSPreview->hWnd = hWnd;

         lpSSPreview = (LPSSPREVIEW)GlobalLock(lpVBSSPreview->hClassData);
         lpSSPreview->hCtl = hCtl;
         lpSSPreview->fDesignTime = (VBGetMode() == MODE_DESIGN);
         GlobalUnlock (lpVBSSPreview->hClassData);
         }
      }

      break;

   case WM_DESTROY:
      {
      LPVBFPSSPREVIEW lpVBSSPreview = VBDerefControl(hCtl);
      if (lpVBSSPreview)
         {
         SSPreviewDestroy (hWnd, lpVBSSPreview->hClassData);
         lpVBSSPreview->hClassData = NULL;
         }
      }
      break;

   case WM_CTLCOLOR:
      return (SendMessage(GetParent(hWnd), Msg, wParam, lParam));

   case VBM_HELP:
      switch (LOBYTE(wParam))
         {
         case VBHELP_PROP:
            if (HIBYTE(wParam) != PVPROP_VBX_CTLNAME &&
                HIBYTE(wParam) != PVPROP_VBX_INDEX &&
                HIBYTE(wParam) != PVPROP_VBX_LEFT &&
                HIBYTE(wParam) != PVPROP_VBX_TOP &&
                HIBYTE(wParam) != PVPROP_VBX_WIDTH &&
                HIBYTE(wParam) != PVPROP_VBX_HEIGHT &&
                HIBYTE(wParam) != PVPROP_VBX_MOUSEPOINTER &&
                HIBYTE(wParam) != PVPROP_VBX_TABINDEX &&
                HIBYTE(wParam) != PVPROP_VBX_TABSTOP &&
                HIBYTE(wParam) != PVPROP_VBX_DRAGICON &&
                HIBYTE(wParam) != PVPROP_VBX_DRAGMODE &&
                HIBYTE(wParam) != PVPROP_VBX_ENABLED &&
                HIBYTE(wParam) != PVPROP_VBX_PARENT &&
                HIBYTE(wParam) != PVPROP_VBX_TAG &&
                HIBYTE(wParam) != PVPROP_VBX_VISIBLE &&
                HIBYTE(wParam) != PVPROP_VBX_HELPCONTEXTID)
               {
               VBSS_WinHelp(hWnd, 8100L + (DWORD)HIBYTE(wParam));
               return (0);
               }

            break;

         case VBHELP_EVT:
            VBSS_WinHelp(hWnd, 8300 + HIBYTE(wParam));
            return (0);

         case VBHELP_CTL:
            VBSS_WinHelp(hWnd, 8000);
            return (0);
         }

      break;

   case VBM_INITPROPPOPUP:
      if (wParam == PVPROP_ABOUTBOX)
         return ((LONG)HwndInitAboutBox());
      break;

   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      if (VBGetMode() == MODE_DESIGN)
         return (0);
      else
         VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam);
      break;

   case WM_MOUSEACTIVATE:
      if (VBGetMode() == MODE_DESIGN)
         return (0);
      break;

   case VBM_SETPROPERTY:
      {
      LPVBFPSSPREVIEW lpVBSSPreview = VBDerefControl(hCtl);

      if (lpVBSSPreview)
         {
         if (lpVBSSPreview->hClassData)
            {
            LPSSPREVIEW lpSSPreview = (LPSSPREVIEW)GlobalLock(lpVBSSPreview->hClassData);
               {
               lRet = fpVBSetProperty(hCtl, hWnd, lpSSPreview, wParam, lParam,
                                      PV_SetProp, PVPropList);
               GlobalUnlock (lpVBSSPreview->hClassData);
               }
            }
         }
      }

      return (lRet);

   case VBM_GETPROPERTY:
      {
      LPVBFPSSPREVIEW lpVBSSPreview = VBDerefControl(hCtl);

      if (lpVBSSPreview)
         {
         if (lpVBSSPreview->hClassData)
            {
            LPSSPREVIEW lpSSPreview = (LPSSPREVIEW)GlobalLock(lpVBSSPreview->hClassData);
               {
               lRet = fpVBGetProperty(hCtl, hWnd, lpSSPreview, wParam, lParam,
                                      PV_GetProp, PVPropList);
               GlobalUnlock (lpVBSSPreview->hClassData);
               }
            }
         }
      }

      return (lRet);

   case WM_COMMAND:
      if (HIWORD(lParam) == SPN_PAGECHANGE)
         {
         VBPARAMS1 Params1;
         long      lPageCurrent;

         lPageCurrent = SendMessage(hWnd, SPM_GETPROPERTY,
                                    PVPROP_PAGECURRENT, 0L);

         Params1.lpParam1 = &lPageCurrent;
         VBFireEvent(hCtl, IEVENT_FPSSPREVIEW_PAGECHANGE, &Params1);
         }

      else if (HIWORD(lParam) == SPN_ZOOM)
         VBFireEvent(hCtl, IEVENT_FPSSPREVIEW_ZOOM, NULL);

      break;

   default:
      {
      LPVBFPSSPREVIEW lpVBSSPreview = VBDerefControl(hCtl);

      if (Msg == WM_NCHITTEST)
         if (VBGetMode() == MODE_DESIGN)
            return (VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam));

      if (lpVBSSPreview)
         {
         if (fCallDef && lpVBSSPreview->hClassData)
            {
            LPSSPREVIEW lpSSPreview = (LPSSPREVIEW)GlobalLock (lpVBSSPreview->hClassData);
            if (lpSSPreview)
               {
               lCode = fpSSPreviewMain (lpSSPreview, hWnd, Msg, wParam, lParam, &fCallDef);
               GlobalUnlock (lpVBSSPreview->hClassData);
               }
            }
         }
      }
      break;
   }

if (fCallDef)
   lCode = VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam);

return lCode;
}

WORD ParamPageChange[] = {ET_I4};

static EVENTINFO EventPageChange =
   {
   "PageChange",
   1,
   2,
   ParamPageChange,
   "Page As Long"
   };

static EVENTINFO EventZoom =
   {
   "Zoom",
   0,
   0,
   NULL,
   ""
   };

static PEVENTINFO FPSSPreviewEvents[] =
   {
   PEVENTINFO_STD_CLICK,
   PEVENTINFO_STD_DBLCLICK,
   PEVENTINFO_STD_DRAGDROP,
   PEVENTINFO_STD_DRAGOVER,
   PEVENTINFO_STD_GOTFOCUS,
   PEVENTINFO_STD_KEYDOWN,
   PEVENTINFO_STD_KEYPRESS,
   PEVENTINFO_STD_KEYUP,
   PEVENTINFO_STD_LOSTFOCUS,
   PEVENTINFO_STD_MOUSEDOWN,
   PEVENTINFO_STD_MOUSEMOVE,
   PEVENTINFO_STD_MOUSEUP,

   &EventPageChange,
   &EventZoom,
   NULL
   };

MODEL PV_Model =
   {
   VB200_VERSION,                  // VB version being used
   MODEL_fFocusOk | MODEL_fArrows, // MODEL flags
   (PCTLPROC)vbFPSSPreviewProc,    // Control procedure
   CS_VREDRAW | CS_HREDRAW,        // Class style
   0,                              // Default Window style
   sizeof(VBFPSSPREVIEW),          // Size of VBFPSSPREVIEW structure
   IDBMP_FPSSPREVIEW,              // Palette bitmap ID
   VBDEFAULT_SSPREVIEW,            // Default control name
   VBCLASS_SSPREVIEW,              // Visual SSPreview class name
   NULL,                           // Parent class name
   PV_Properties,                  // Property information table
   FPSSPreviewEvents,              // Event information table
   PVPROP_ABOUTBOX,
   0,
   PVPROP_ABOUTBOX,
   4                                // 1-17-94
   };     

   
BOOL VBSSPreviewRegister(HANDLE hInstance)
{
USHORT usVersion = VBGetVersion();
BOOL   fRet;

if (VBGetMode() == MODE_DESIGN)
   {
   WNDCLASS class;

   class.style = 0;
   class.lpfnWndProc = AboutBoxProc;
   class.cbClsExtra = 0;
   class.cbWndExtra = 0;
   class.hInstance = fpInstance;
   class.hIcon = NULL;
   class.hCursor = NULL;
   class.hbrBackground = NULL;
   class.lpszMenuName = NULL;
   class.lpszClassName = CLASS_ABOUTBOX;
   RegisterClass(&class);
   }

VBPV_InitModel(usVersion);

if (usVersion <= VB100_VERSION)
   fRet = VBRegisterModel(hInstance, &PV_Model_VB1);
else
   fRet = VBRegisterModel(hInstance, &PV_Model);

return (fRet);
}

   
void VBPV_InitModel(USHORT usVersion)
{
NPPROPLIST npProps;
short      i;

if (usVersion < VB200_VERSION)
   npProps = PV_Properties_VB1;
else
   npProps = PV_Properties;

for (i = 0; i < PVPROP_PROPCNT; i++)
   {
   if (PVPropList[i].pPropInfo < fpPPROPINFO_STD_LAST &&
       PVPropList[i].bDataType != -1)
      PVPropList[i].bDataType = (BYTE)(PVPropList[i].pPropInfo->fl &
                                        PF_datatype);

   npProps[i] = (PPROPINFO)PVPropList[i].pPropInfo;
   }

if (usVersion < VB200_VERSION)
   {
   npProps[PVPROP_VBX_HELPCONTEXTID] = &PropNULL;

   for (i = 0; i < PVPROP_PROPCNT; i++)
      if (npProps[i]  < PPROPINFO_STD_LAST)
         npProps[i]->fl &= ~PF_fNoRuntimeR;
   }

/**********************************
* Set up ListBox Model Structures
**********************************/

_fmemcpy(&PV_Model_VB1, &PV_Model, sizeof(MODEL));

PV_Model_VB1.usVersion = VB100_VERSION;
PV_Model_VB1.npproplist = PV_Properties_VB1;
}


//---------------------------------------------------------------------------
// Create our property popup-window.  Since we want to put up a dialog, this
// window never becomes visible.  Instead, when asked to become visible, it
// will post a message to itself, reminding it to put up our dialog.
//
// NOTE: May return NULL!
//---------------------------------------------------------------------------
HWND NEAR HwndInitAboutBox(VOID)
{
return (CreateWindow(CLASS_ABOUTBOX, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL,
                     fpInstance, NULL));
}

//---------------------------------------------------------------------------
// We asked to show ourself, remain invisible and post a CM_OPENABOUT to
// ourself.  When we receive this message, open the dialog box.
//---------------------------------------------------------------------------
LONG _export FAR PASCAL AboutBoxProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
switch (msg)
   {
   case WM_SHOWWINDOW:
      if (wParam)
         {
         PostMessage(hWnd, SPM_OPENABOUTBOX, 0, 0L);
         return 0L;
         }
      break;

   case SPM_OPENABOUTBOX:
      VBSS_About(hWnd);
      return 0L;
   }

return DefWindowProc(hWnd, msg, wParam, lParam);
}


long fpVBSetProperty(HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (*lpPropProc)(LPVOID, WPARAM, LPARAM),
                     LPFP_PROPLIST lpPropList)
{
long lRet = 0;

switch (lpPropList[wParam].bDataType)
   {
   case DT_SHORT:
   case DT_BOOL:
   case DT_ENUM:
      lRet = lpPropProc(lpStruct, wParam, (long)(short)lParam);
      break;

   case DT_REAL:
      lRet = lpPropProc(lpStruct, wParam, (long)(LPVOID)&lParam);
      break;

   case DT_HSZ:
   case DT_LONG:
   case DT_COLOR:
   case DT_XPOS:
   case DT_XSIZE:
   case DT_YPOS:
   case DT_YSIZE:
   case DT_HLSTR:
      lRet = lpPropProc(lpStruct, wParam, lParam);
      break;

   case DT_PICTURE:
      lRet = lpPropProc(lpStruct, wParam, (long)(HPIC)lParam);
      break;

   default:
      return (VBDefControlProc(hCtl, hWnd, VBM_SETPROPERTY, wParam, lParam));
   }

return (lRet);
}


long fpVBGetProperty(HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (*lpPropProc)(LPVOID, WPARAM, LPARAM),
                     LPFP_PROPLIST lpPropList)
{
long lRet = 0;

switch (lpPropList[wParam].bDataType)
   {
   case DT_HSZ:
      lRet = lpPropProc(lpStruct, wParam, 0);

      if (lRet)
         {
         LPSTR lpszText = (LPSTR)fpGlobalLock((FPGLOBALHANDLE)lRet);
         *(HSZ FAR *)lParam = VBCreateHsz((_segment)hCtl, lpszText);
         fpGlobalUnlock((FPGLOBALHANDLE)lRet);
         fpGlobalFree((FPGLOBALHANDLE)lRet);
         }
      else
         *(HSZ FAR *)lParam = VBCreateHsz((_segment)hCtl, "");

      break;

   case DT_SHORT:
   case DT_ENUM:
      *(short FAR *)lParam = (short)lpPropProc(lpStruct, wParam, 0);
      break;

   case DT_BOOL:
      *(BOOL FAR *)lParam = (BOOL)(lpPropProc(lpStruct, wParam, 0) ? -1 :
                                              FALSE);
      break;

   case DT_REAL:
      lpPropProc(lpStruct, wParam, lParam);
      break;

   case DT_LONG:
   case DT_COLOR:
   case DT_XPOS:
   case DT_XSIZE:
   case DT_YPOS:
   case DT_YSIZE:
      *(long FAR *)lParam = lpPropProc(lpStruct, wParam, 0);
      break;

   case DT_PICTURE:
      *(HPIC FAR *)lParam = (HPIC)lpPropProc(lpStruct, wParam, 0);
      break;

   case DT_HLSTR:
      {
      HLSTR  hlstrStr;
      LPSTR  lpszStr;
      USHORT uLen;

      hlstrStr = (HLSTR)lpPropProc(lpStruct, wParam, 0);

      if (hlstrStr)
         {
         lpszStr = VBDerefHlstrLen(hlstrStr, &uLen);
         hlstrStr = VBCreateHlstr(lpszStr, uLen);
         }

      *(HLSTR FAR *)lParam = hlstrStr;
      }
      break;

   default:
      return (VBDefControlProc(hCtl, hWnd, VBM_GETPROPERTY, wParam, lParam));
   }

return (0);
}
