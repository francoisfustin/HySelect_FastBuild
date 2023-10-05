#ifndef FPNCPNT_H
#define FPNCPNT_H

#include "windows.h"
#include "fptools.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif

#define SPINCLASS(version) "fpSpin" #version

//
// NonClient Styles
//
#define FP_NCDROPSHADOW      0x00000001L
#define FP_NCROUND           0x00000002L
#define FP_NCBORDER          0x00000004L
#define FP_NC3DINVERT        0x00000008L

#define FP_NC3DINMASK        0x00000070L
#define FP_NC3DINLOWER       0x00000010L
#define FP_NC3DINRAISE       0x00000020L
#define FP_NC3DINCHISEL      0x00000040L

#define FP_NCDROPSHADOWFOCUS 0x00000080L

#define FP_NC3DOUTMASK       0x00000700L
#define FP_NC3DOUTLOWER      0x00000100L
#define FP_NC3DOUTRAISE      0x00000200L
#define FP_NC3DOUTCHISEL     0x00000400L

#define FP_NCUPDATE          0x00000800L

#define FP_NCBUTTONWRAP          0x00001000L
#define FP_NCBUTTONHIDE          0x00002000L
#define FP_NCBUTTONDISABLED      0x00004000L
#define FP_NCBUTTONDEFAULTACTION 0x00008000L

#define FP_LBUTTON(info)  (short)((info->lStyle & 0xFF000000) >> 24)
#define FP_RBUTTON(info)  (short)((info->lStyle & 0x00FF0000) >> 16)
#define FP_NCBUTTON       0xFFFF0000L

#define FP_NCSPIN         0x00130000L
#define FP_NCPOPUP        0x00110000L
#define FP_NCDROPDOWN     0x00120000L
#define FP_NCSLIDE        0x24180000L

#define FPNC_REDRAW_BORDER     0x0001
#define FPNC_REDRAW_3D         0x0002
#define FPNC_REDRAW_BTNLEFT    0x0004
#define FPNC_REDRAW_BTNRIGHT   0x0008
#define FPNC_REDRAW_DROPSHADOW 0x0010
#define FPNC_REDRAW_POST       0x8000
#define FPNC_REDRAW_REFRESH    0x7FFF

#define FPNC_APPEARANCE_CUSTOM   0
#define FPNC_APPEARANCE_FLAT     1
#define FPNC_APPEARANCE_3D       2
#define FPNC_APPEARANCE_3DBORDER 3

//
// NonClient Structure
//
typedef struct tagFPNCINFO
   {
   // styles
   LONG     lStyle;
   
   // border
   BYTE     bBorderSize;
   
   FPCOLORID idColorBorder;
   FPCOLORID idColorBk;
   
   COLORREF colorBorder; 
   COLORREF colorBk;
   BYTE     bPenStyleNoFocus; 
   BYTE     bPenStyleOnFocus;
   
   // drop shadow
   BYTE     bDropShadowWidth;
   COLORREF colorDropShadow;
   FPCOLORID idColorDropShadow;
   
   // 3D
   FPCOLORID idClr3DFrame;
   FPCOLORID idClr3DOutShadow;
   FPCOLORID idClr3DInShadow;
   FPCOLORID idClr3DOutHiLite;
   FPCOLORID idClr3DInHiLite;

   COLORREF clr3DFrame;
   COLORREF color3DOutShadow;
   COLORREF color3DInShadow;
   COLORREF color3DOutHiLite; 
   COLORREF color3DInHiLite;

   BYTE     bInShadowWidth;
   BYTE     bOutShadowWidth;
   BYTE     bFrameWidth;
   BYTE     bAppearance;
   
   // buttons
   short    dButtonMin;
   short    dButtonMax;
   short    dButtonIndex;
   short    dButtonIncrement;
   short    dButtonWidth;
   short    dButtonState;
   
   COLORREF clrButton;
   FPCOLORID idClrButton;

//#ifdef BTNALIGN
   BOOL    fButtonLeft;
//#endif   
//   BOOL    fSuspend;

   } FPNCINFO, FAR *LPFPNCINFO;


//
// NonClient Functions
//
void FPLIB fpSetButtonStyle (HWND hWnd, HANDLE hInst, LONG lStyle, 
   LPFPNCINFO info);

LONG FPLIB fpWMNCCalcSize (HWND hWnd, LPFPNCINFO lpInfo, BOOL fCalcValidRects, LPNCCALCSIZE_PARAMS lpCalc);
void FPLIB fpWMNCPaint (HWND hWnd, LPFPNCINFO info, short redraw, HDC hDCInput);
LONG FPLIB fpNCLeftButtonDown (HWND hWnd, LPFPNCINFO lpNCInfo, POINT point);
void FPLIB fpForceNCUpdate (HWND hWnd, LPFPNCINFO lpNCInfo, short redraw);
void FPLIB fpNCInit(LPFPNCINFO lpzNCInfo, BOOL fSet);
LONG FPLIB fpNCHitTest (HWND hWnd, LONG code, LPFPNCINFO lpNCInfo, LPPOINT point, BOOL fStatic);

// style macros
#define NC_GET3DINSIDESTYLE(info) \
   (info.lStyle & FP_NC3DINMASK)
#define NC_SET3DINSIDESTYLE(info, style) \
   ((info.lStyle & ~FP_NC3DINMASK) | style)
#define NC_GET3DOUTSIDESTYLE(info) \
   (info.lStyle & FP_NC3DOUTMASK)
#define NC_SET3DOUTSIDESTYLE(info, style) \
   ((info.lStyle & ~FP_NC3DOUTMASK) | style)

#define NC_SETSTYLE(info, style, state) \
   (info.lStyle = (info.lStyle & ~style) | (state ? style : 0))


#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#ifdef __cplusplus
}
#endif    

#endif // FPNCPNT_H
