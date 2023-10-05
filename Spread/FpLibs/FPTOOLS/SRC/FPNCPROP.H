/**************************************************************************
*
* FPNCPROP_H:
*
* Functions associated with Non-Client Properties (pulled from VBNCPNT.H).
*
* NOTE:  Though these functions use the VBPROPPARAMS structure, the
*        functions are used for both VBX's and DLL's.  The "hCtl" field
*        should NOT be used in these functions.
*
**************************************************************************/
#ifndef FPNCPROP_H
#define FPNCPROP_H

//***#include "fptools.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

// 3D
long FPLIB FP_3DInsideStyle           (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_3DInsideHighlightColor  (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_3DInsideShadowColor     (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_3DInsideShadowSize      (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_3DOutsideStyle          (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_3DOutsideHighlightColor (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_3DOutsideShadowColor    (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_3DOutsideShadowSize     (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_3DBevelSize             (LPVBPROPPARAMS lpPP, BOOL fGetProp);
// border
long FPLIB FP_BorderColor             (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_NCBkColor               (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_BorderSize              (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_BorderStyle             (LPVBPROPPARAMS lpPP, BOOL fGetProp);
// onfocus
long FPLIB FP_OnFocusInvert3D         (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_OnFocusShadow           (LPVBPROPPARAMS lpPP, BOOL fGetProp);           
// drop shadow
long FPLIB FP_DropShadowColor         (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_DropShadowSize          (LPVBPROPPARAMS lpPP, BOOL fGetProp);
// scroll bars               
long FPLIB FP_ScrollBars              (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_ScrollIncrement         (LPVBPROPPARAMS lpPP, BOOL fGetProp);
// Buttons
long FPLIB FP_ButtonStyle             (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonMin               (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonMax               (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonIncrement         (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonIndex             (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonWrap              (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonDisabled          (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonHide              (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonWidth             (LPVBPROPPARAMS lpPP, BOOL fGetProp);          
long FPLIB FP_ButtonDefaultAction     (LPVBPROPPARAMS lpPP, BOOL fGetProp);         
long FPLIB FP_ThreeDFrameColor        (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_BorderDropShadow        (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_Appearance              (LPVBPROPPARAMS lpPP, BOOL fGetProp);

long FPLIB FP_ButtonColor        (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_ButtonAlign        (LPVBPROPPARAMS lpPP, BOOL fGetProp);

#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#ifdef __cplusplus
}
#endif    

#endif
