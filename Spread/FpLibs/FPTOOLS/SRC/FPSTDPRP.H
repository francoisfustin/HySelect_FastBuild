/**************************************************************************
*
* FPSTDPRP_H:
*
* Functions associated with "Standard VBX Properties" but used in DLL's.
*
* NOTE:  Though these functions use the VBPROPPARAMS structure, the
*        functions are used for both VBX's and DLL's.  The "hCtl" field
*        should NOT be used in these functions.
*
**************************************************************************/
#ifndef FPSTDPRP_H
#define FPSTDPRP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

//Misc
long FPLIB FP_Enabled         (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_Visible         (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_TabStop         (LPVBPROPPARAMS lpPP, BOOL fGetProp);

//Size & Position
long FPLIB FP_Height          (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_Left            (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_Top             (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_Width           (LPVBPROPPARAMS lpPP, BOOL fGetProp);

// Font Properties...
long FPLIB FP_FontBold        (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_FontItalic      (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_FontStrikeThru  (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_FontUnderline   (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_FontSize        (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_FontName        (LPVBPROPPARAMS lpPP, BOOL fGetProp);


#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#ifdef __cplusplus
}
#endif    

#endif
