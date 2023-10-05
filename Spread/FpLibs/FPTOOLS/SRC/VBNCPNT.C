#include "fptools.h"
#include "fpncpnt.h"
#include "fppinfo.h"
#include <tchar.h>

// This function has VB specific function calls.
//
long FPLIB FP_VBButtonWidth (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(LONG FAR *)lpPP->lParam = VBXPixelsToTwips(ncInfo.dButtonWidth);
   else
      {
      ncInfo.dButtonWidth = VBXTwipsToPixels(lpPP->lParam);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}




TCHAR sz3DStyles[] =
   {
   _T("0 - None\0")
   _T("1 - Lowered\0")
   _T("2 - Raised\0")
   };

PROPINFO NCProp3DInsideStyle =
   {
   _T("ThreeDInsideStyle"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, sz3DStyles, 2
   };

PROPINFO NCProp3DInsideHighlightColor =
   {
   _T("ThreeDInsideHighlightColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_BTNFACE, NULL, 0
   };

PROPINFO NCProp3DInsideShadowColor =
   {
   _T("ThreeDInsideShadowColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_WINDOWFRAME, NULL, 0
   };

PROPINFO NCProp3DInsideShadowSize =
   {
   _T("ThreeDInsideWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };

PROPINFO NCProp3DOutsideStyle =
   {
   _T("ThreeDOutsideStyle"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, sz3DStyles, 2
   };

PROPINFO NCProp3DOutsideHighlightColor =
   {         
   _T("ThreeDOutsideHighlightColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
#ifdef WIN32  //KAM
   0, 0, 0x80000000 | COLOR_BTNHIGHLIGHT, NULL, 0
#else
   0, 0, RGBCOLOR_WHITE, NULL, 0
#endif
   };

PROPINFO NCProp3DOutsideShadowColor =
   {
   _T("ThreeDOutsideShadowColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_BTNSHADOW, NULL, 0
   };

PROPINFO NCProp3DOutsideShadowSize =
   {
   _T("ThreeDOutsideWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };

PROPINFO NCProp3DBevelSize =
   {
   _T("ThreeDFrameWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };

PROPINFO NCPropBorderColor =
   {                          
   _T("BorderColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_WINDOWFRAME, NULL, 0
   };

PROPINFO NCPropNCBkColor =
   {                          
   _T("BorderGrayAreaColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, RGBCOLOR_DEFAULT, NULL, 0
   };

TCHAR szBorderStyles[] =
   {
   _T("0 - No Border\0")
   _T("1 - Single Line\0")
   _T("2 - Rounded\0")
   _T("3 - Dash\0")
   _T("4 - Dot\0")
   _T("5 - Dash Dot\0")
   _T("6 - Dash Dot Dot\0")
   };

PROPINFO NCPropBorderStyle =
   {
   _T("BorderStyle"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, szBorderStyles, 6
   };

/*
CHAR szScrollBarsTypes[] =
   {
   "0 - None\0"
   "1 - Horizontal\0"
   "2 - Vertical\0"
   "3 - Both\0"
   };

PROPINFO NCPropScrollBars =
   {
   "ScrollBars",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, szScrollBarsTypes, 3
   };

PROPINFO NCPropScrollIncrement =
   {
   "ScrollIncrement",
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 3, NULL, 0
   };
*/

TCHAR szButtonTypes[] =
   {
   _T("0 - None\0")
   _T("1 - Spin\0")
   _T("2 - Pop Up\0")
   _T("3 - Drop Down\0")
   _T("4 - Slide\0")
   };

PROPINFO NCPropButtonStyle =
   {
   _T("ButtonStyle"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, szButtonTypes, 4
   };

PROPINFO NCPropButtonDisable =
   {
   _T("ButtonDisable"),
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, FALSE, NULL, 0
   };

PROPINFO NCPropButtonWidth =
   {
   _T("ButtonWidth"),
   DT_XSIZE | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };

PROPINFO NCPropButtonMin =
   {
   _T("ButtonMin"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };

PROPINFO NCPropButtonMax =
   {
   _T("ButtonMax"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 100, NULL, 0
   };

PROPINFO NCPropButtonIndex =
   {
   _T("ButtonIndex"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };

PROPINFO NCPropButtonIncrement =
   {
   _T("ButtonIncrement"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };

PROPINFO NCPropButtonWrap =
   {
   _T("ButtonWrap"),
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, -1, NULL, 0
   };

PROPINFO NCPropButtonDefaultAction =
   {
   _T("ButtonDefaultAction"),
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, -1, NULL, 0
   };

PROPINFO NCPropButtonHide =
   {
   _T("ButtonHide"),
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, FALSE, NULL, 0
   };

PROPINFO NCPropButtonColor =
   {                          
   _T("ButtonColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef | fpPF_fNewSaveData,
   0, 0, 0x80000000 | COLOR_BTNFACE, NULL, 0
   };

TCHAR szButtonAlign[] =
   {
   _T("0 - Right\0")
   _T("1 - Left\0")
   };

PROPINFO NCPropButtonAlign =
   {                          
   _T("ButtonAlign"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef | fpPF_fNewSaveData,
   0, 0, 0, szButtonAlign, 1
   };

PROPINFO NCPropBorderSize =
   {
   _T("BorderWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };

PROPINFO NCPropOnFocusInvert3D =               // Old
   {
   _T("OnFocusInvert3D"),
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, FALSE, NULL, 0
   };

PROPINFO NCPropThreeDOnFocusInvert =
   {
   _T("ThreeDOnFocusInvert"),
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, FALSE, NULL, 0
   };

PROPINFO NCPropOnFocusShadow =                   // Old
   {
   _T("OnFocusShadow"),
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, FALSE, NULL, 0
   };

TCHAR szBorderDropShadow[] =
   {
   _T("0 - None\0")
   _T("1 - Always\0")
   _T("2 - On Focus\0")
   };

PROPINFO NCPropBorderDropShadow =
   {
   _T("BorderDropShadow"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, szBorderDropShadow, 2
   };

PROPINFO NCPropDropShadowColor =                 // Old
   {                          
   _T("DropShadowColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0x80000000 | COLOR_BTNSHADOW, NULL, 0
   };

PROPINFO NCPropBorderDropShadowColor =
   {                          
   _T("BorderDropShadowColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_BTNSHADOW, NULL, 0
   };

PROPINFO NCPropDropShadowSize =                  // Old
   {
   _T("DropShadowWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 3, NULL, 0
   };

PROPINFO NCPropBorderDropShadowWidth =
   {
   _T("BorderDropShadowWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 3, NULL, 0
   };

PROPINFO NCPropThreeDFrameColor =
   {                          
   _T("ThreeDFrameColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_BTNFACE, NULL, 0
   };

TCHAR szAppearance[] =
   {
   _T("0 - Custom\0")
   _T("1 - Flat\0")
   _T("2 - 3-D\0")
   _T("3 - 3-D with Border\0")
   };

PROPINFO NCPropAppearance =
   {
   _T("Appearance"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, szAppearance, 3
   };

// ThreeD properties using common Border property prefix

PROPINFO NCPropBorderInnerStyle =
   {
   _T("BorderInnerStyle"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, sz3DStyles, 2
   };

PROPINFO NCPropBorderInnerHighlightColor =
   {
   _T("BorderInnerHighlightColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_BTNFACE, NULL, 0
   };

PROPINFO NCPropBorderInnerShadowColor =
   {
   _T("BorderInnerShadowColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_WINDOWFRAME, NULL, 0
   };

PROPINFO NCPropBorderInnerWidth =
   {
   _T("BorderInnerWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };

PROPINFO NCPropBorderOuterStyle =
   {
   _T("BorderOuterStyle"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, sz3DStyles, 2
   };

PROPINFO NCPropBorderOuterHighlightColor =
   {         
   _T("BorderOuterHighlightColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, RGBCOLOR_WHITE, NULL, 0
   };

PROPINFO NCPropBorderOuterShadowColor =
   {
   _T("BorderOuterShadowColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_BTNSHADOW, NULL, 0
   };

PROPINFO NCPropBorderOuterWidth =
   {
   _T("BorderOuterWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };

PROPINFO NCPropBorderFrameWidth =
   {
   _T("BorderFrameWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };

PROPINFO NCPropBorderOutlineColor =
   {                          
   _T("BorderOutlineColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_WINDOWFRAME, NULL, 0
   };

PROPINFO NCPropBorderFrameColor =
   {                          
   _T("BorderFrameColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0x80000000 | COLOR_BTNFACE, NULL, 0
   };

PROPINFO NCPropBorderOutlineWidth =
   {
   _T("BorderOutlineWidth"),
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };

PROPINFO NCPropBorderOutlineStyle =
   {
   _T("BorderOutlineStyle"),
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, szBorderStyles, 6
   };


PROPINFO NCPropBackColor =
   {
   _T("BackColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal,
   0, 0, COLOR_INIT(COLOR_WINDOW), 0, 0
   };

PROPINFO NCPropForeColor =
   {
   _T("ForeColor"),
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal,
   0, 0, COLOR_INIT(COLOR_WINDOWTEXT), 0, 0
   };
