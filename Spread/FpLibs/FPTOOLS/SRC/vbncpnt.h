#ifndef VBNCPNT_H
#define VBNCPNT_H

#include "fptools.h"

#include "fpncprop.h"             // include the Function Prototypes

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

// specific to VB 
//
long FPLIB FP_VBButtonWidth (LPVBPROPPARAMS lpPP, BOOL fGetProp);          


// 3D
extern PROPINFO NCProp3DOutsideStyle;
extern PROPINFO NCProp3DOutsideShadowSize;
extern PROPINFO NCProp3DOutsideShadowColor;
extern PROPINFO NCProp3DOutsideHighlightColor;
extern PROPINFO NCProp3DInsideStyle;
extern PROPINFO NCProp3DInsideShadowSize;
extern PROPINFO NCProp3DInsideShadowColor;
extern PROPINFO NCProp3DInsideHighlightColor;
extern PROPINFO NCProp3DBevelSize;
// Border
extern PROPINFO NCPropBorderSize;
extern PROPINFO NCPropBorderStyle;
extern PROPINFO NCPropBorderColor;
extern PROPINFO NCPropNCBkColor;
// Spin
extern PROPINFO NCPropButtonStyle;
extern PROPINFO NCPropButtonDisable;
extern PROPINFO NCPropButtonMin;
extern PROPINFO NCPropButtonMax;
extern PROPINFO NCPropButtonWrap;
extern PROPINFO NCPropButtonHide;
extern PROPINFO NCPropButtonIncrement;
extern PROPINFO NCPropButtonIndex;
extern PROPINFO NCPropButtonWidth;
extern PROPINFO NCPropButtonWidth;
extern PROPINFO NCPropButtonDefaultAction;
// Scroll Bars
extern PROPINFO NCPropScrollBars;
extern PROPINFO NCPropScrollIncrement;
// Onfocus
extern PROPINFO NCPropOnFocusShadow;
extern PROPINFO NCPropOnFocusInvert3D;
extern PROPINFO NCPropDropShadowColor;
extern PROPINFO NCPropDropShadowSize;

extern PROPINFO NCPropThreeDFrameColor;

extern PROPINFO NCPropBorderInnerStyle;
extern PROPINFO NCPropBorderInnerHighlightColor;
extern PROPINFO NCPropBorderInnerShadowColor;
extern PROPINFO NCPropBorderInnerWidth;
extern PROPINFO NCPropBorderOuterStyle;
extern PROPINFO NCPropBorderOuterHighlightColor;
extern PROPINFO NCPropBorderOuterShadowColor;
extern PROPINFO NCPropBorderOuterWidth;
extern PROPINFO NCPropBorderFrameWidth;
extern PROPINFO NCPropBorderOutlineColor;
extern PROPINFO NCPropBorderFrameColor;
extern PROPINFO NCPropBorderOutlineWidth;
extern PROPINFO NCPropBorderOutlineStyle;

extern PROPINFO NCPropThreeDOnFocusInvert;
extern PROPINFO NCPropBorderDropShadow;
extern PROPINFO NCPropBorderDropShadowColor;
extern PROPINFO NCPropBorderDropShadowWidth;
extern PROPINFO NCPropAppearance;


extern PROPINFO NCPropBackColor;
extern PROPINFO NCPropForeColor;
extern PROPINFO NCPropButtonColor;
extern PROPINFO NCPropButtonAlign;

#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#ifdef __cplusplus
}
#endif    

#endif   // ifndef
