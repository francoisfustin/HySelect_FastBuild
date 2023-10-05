#define NOCOMM
#include <windows.h>
#include <string.h>
#include "fptools.h"
#include "fpmemmgr.h"
#include "vbssprvw.h"
#include "ssprvw.h"
#include <tchar.h>

#define OFFSETIN(struc,field) ((USHORT)&(((struc *)0)->field))

static PROPINFO PVProp_AboutBox =
   {
   _T("(About)"),
   DT_HSZ | PF_fGetData | PF_fSetData | PF_fSetMsg,
   OFFSETIN(VBFPSSPREVIEW, hszAbout),
   0, 0, NULL, 0
   };

static PROPINFO PVProp_AllowUserZoom =
   {
   _T("AllowUserZoom"),
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, -1, 0, 0
   };

static PROPINFO PVProp_GrayAreaColor =
   {
   _T("GrayAreaColor"),
   DT_COLOR | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, RGBCOLOR_DARKGRAY, 0, 0
   };

static PROPINFO PVProp_GrayAreaMarginH =
   {
   _T("GrayAreaMarginH"),
   DT_XPOS | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, SSPRVW_MARGINDEF, 0, 0
   };

TCHAR szGrayAreaMarginType[] =
   {
   _T("0 - Scaled\0")
   _T("1 - Actual\0")
   };

static PROPINFO PVProp_GrayAreaMarginType =
   {
   _T("GrayAreaMarginType"),
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, szGrayAreaMarginType, 1
   };

static PROPINFO PVProp_GrayAreaMarginV =
   {
   _T("GrayAreaMarginV"),
   DT_YPOS | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, SSPRVW_MARGINDEF, 0, 0
   };

static PROPINFO PVProp_hWndSpread =
   {
   _T("hWndSpread"),
   DT_LONG | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, 0, 0
   };

static PROPINFO PVProp_PageBorderColor =
   {
   _T("PageBorderColor"),
   DT_COLOR | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, RGBCOLOR_DARKBLUE, 0, 0
   };

static PROPINFO PVProp_PageBorderWidth =
   {
   _T("PageBorderWidth"),
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 2, 0, 0
   };

static PROPINFO PVProp_PageShadowColor =
   {
   _T("PageShadowColor"),
   DT_COLOR | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, RGBCOLOR_BLACK, 0, 0
   };

static PROPINFO PVProp_PageShadowWidth =
   {
   _T("PageShadowWidth"),
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 2, 0, 0
   };

static PROPINFO PVProp_PageViewPercentage =
   {
   _T("PageViewPercentage"),
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 100, 0, 0
   };

TCHAR szPageViewType[] =
   {
   _T("0 - Whole Page\0")
   _T("1 - Normal Size\0")
   _T("2 - Percentage\0")
   _T("3 - Page Width\0")
   _T("4 - Page Height\0")
   _T("5 - Multiple Pages\0")
   };

static PROPINFO PVProp_PageViewType =
   {
   _T("PageViewType"),
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, szPageViewType, 5
   };

TCHAR szScrollBar[] =
   {
   _T("0 - Show\0")
   _T("1 - Auto\0")
   _T("2 - Hide\0")
   };

static PROPINFO PVProp_ScrollBarH =
   {
   _T("ScrollBarH"),
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, szScrollBar, 2
   };

static PROPINFO PVProp_ScrollBarV =
   {
   _T("ScrollBarV"),
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, szScrollBar, 2
   };

static PROPINFO PVProp_ScrollIncH =
   {
   _T("ScrollIncH"),
   DT_XSIZE | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, SSPRVW_SCROLLINCHDEF, 0, 0
   };

static PROPINFO PVProp_ScrollIncV =
   {
   _T("ScrollIncV"),
   DT_YSIZE | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, SSPRVW_SCROLLINCVDEF, 0, 0
   };

/*
static PROPINFO PVProp_ShowMargins =
   {
   _T("ShowMargins"),
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, 0, 0
   };
*/

PROPINFO PropNULL =
   {
   _T("Null"),
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, 0, 0
   };

static PROPINFO PVProp_PageMultiCntH =
   {
   _T("PageMultiCntH"),
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, 0, 0
   };

static PROPINFO PVProp_PageMultiCntV =
   {
   _T("PageMultiCntV"),
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, 0, 0
   };

static PROPINFO PVProp_PageGutterH =
   {
   _T("PageGutterH"),
   DT_XSIZE | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, -1, 0, 0
   };

static PROPINFO PVProp_PageGutterV =
   {
   _T("PageGutterV"),
   DT_YSIZE | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, -1, 0, 0
   };

TCHAR szZoomState[] =
   {
   _T("0 - Undefined\0")
   _T("1 - In\0")
   _T("2 - Out\0")
   _T("3 - Switch\0")
   };

static PROPINFO PVProp_ZoomState =
   {
   _T("ZoomState"),
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, szZoomState, 3
   };

static PROPINFO PVProp_PageCurrent =
   {
   _T("PageCurrent"),
   DT_LONG | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, 0, 0
   };

static PROPINFO PVProp_PagesPerScreen =
   {
   _T("PagesPerScreen"),
   DT_SHORT | PF_fGetMsg | PF_fNoRuntimeW | PF_fNoShow,
   0, 0, 0, 0, 0
   };

static PROPINFO PVProp_PagePercentageActual =
   {
   _T("PagePercentageActual"),
   DT_SHORT | PF_fGetMsg | PF_fNoRuntimeW | PF_fNoShow,
   0, 0, 0, 0, 0
   };
#ifdef NO_MFC
PROPINFO PVProp_MouseIcon =
   {
   "MouseIcon",
   DT_PICTURE | PF_fSetMsg | PF_fGetMsg | 
   PF_fSaveMsg |
   PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 0, 0, 0
   };
static char szOLEDropMode[] =
	{
	"0 - None\0"
	"1 - Manual\0"
	"2 - Automatic\0"
	};

PROPINFO PVProp_OLEDropMode = 
{
	"OLEDropMode",
	DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
    PF_fNoInitDef | 
#ifndef FP_OLEDRAG
	PF_fNoShow |
#endif	   
    fpPF_fNewSaveData,
	0, 0, 0, szOLEDropMode, 2
};	
#endif

#ifdef SS_V35
static PROPINFO PVProp_ScriptEnhanced =
   {
   _T("ScriptEnhanced"),
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, -1, 0, 0
   };

#endif


/*#ifdef PP4
static PROPINFO PVProp_PageHeight =
   {
   _T("PageHeight"),
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, 0, 0
   };
static PROPINFO PVProp_PageWidth =
   {
   _T("PageWidth"),
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal |
   PF_fNoInitDef,
   0, 0, 1, 0, 0
   };
#endif
   */
/****************
* Property List
****************/

FP_PROPLIST PVPropList[PVPROP_PROPCNT] =
{
-1, (PFP_PROPINFO)PPROPINFO_STD_CTLNAME,             FP_NO_OCX, 0, // PVPROP_VBX_CTLNAME
-1, (PFP_PROPINFO)PPROPINFO_STD_INDEX,               FP_NO_OCX, 0, // PVPROP_VBX_INDEX
-1, (PFP_PROPINFO)PPROPINFO_STD_LEFT,                FP_NO_OCX, 0, // PVPROP_VBX_LEFT
-1, (PFP_PROPINFO)PPROPINFO_STD_TOP,                 FP_NO_OCX, 0, // PVPROP_VBX_TOP
-1, (PFP_PROPINFO)PPROPINFO_STD_WIDTH,               FP_NO_OCX, 0, // PVPROP_VBX_WIDTH
-1, (PFP_PROPINFO)PPROPINFO_STD_HEIGHT,              FP_NO_OCX, 0, // PVPROP_VBX_HEIGHT
-1, (PFP_PROPINFO)PPROPINFO_STD_MOUSEPOINTER,        FP_NO_OCX, 0, // PVPROP_VBX_MOUSEPOINTER
-1, (PFP_PROPINFO)PPROPINFO_STD_TABINDEX,            FP_NO_OCX, 0, // PVPROP_VBX_TABINDEX
-1, (PFP_PROPINFO)PPROPINFO_STD_TABSTOP,             FP_NO_OCX, 0, // PVPROP_VBX_TABSTOP
-1, (PFP_PROPINFO)PPROPINFO_STD_DRAGICON,            FP_NO_OCX, 0, // PVPROP_VBX_DRAGICON
-1, (PFP_PROPINFO)PPROPINFO_STD_DRAGMODE,            FP_NO_OCX, 0, // PVPROP_VBX_DRAGMODE
-1, (PFP_PROPINFO)PPROPINFO_STD_ENABLED,             FP_NO_OCX, 0, // PVPROP_VBX_ENABLED
-1, (PFP_PROPINFO)PPROPINFO_STD_PARENT,              FP_NO_OCX, 0, // PVPROP_VBX_PARENT
-1, (PFP_PROPINFO)PPROPINFO_STD_TAG,                 FP_NO_OCX, 0, // PVPROP_VBX_TAG
-1, (PFP_PROPINFO)PPROPINFO_STD_VISIBLE,             FP_NO_OCX, 0, // PVPROP_VBX_VISIBLE
-1, (PFP_PROPINFO)PPROPINFO_STD_BORDERSTYLEOFF,      FP_NO_OCX, 0, // PVPROP_VBX_VISIBLE
-1, (PFP_PROPINFO)PPROPINFO_STD_HELPCONTEXTID,       FP_NO_OCX, 0, // PVPROP_VBX_HELPCONTEXTID

-1,  (PFP_PROPINFO)&PVProp_AboutBox,                 FP_NO_OCX, 0, // PVPROP_AboutBox
0,  (PFP_PROPINFO)&PVProp_AllowUserZoom,             0,         1, // PVPROP_AllowUserZoom
0,  (PFP_PROPINFO)&PVProp_GrayAreaColor,             0,         2, // PVPROP_GrayAreaColor
0,  (PFP_PROPINFO)&PVProp_GrayAreaMarginH,           0,         3, // PVPROP_GrayAreaMarginH
0,  (PFP_PROPINFO)&PVProp_GrayAreaMarginType,        0,         4, // PVPROP_GrayAreaMarginType
0,  (PFP_PROPINFO)&PVProp_GrayAreaMarginV,           0,         5, // PVPROP_GrayAreaMarginV
0,  (PFP_PROPINFO)&PVProp_hWndSpread,                0,         6, // PVPROP_hWndSpread
0,  (PFP_PROPINFO)&PVProp_PageBorderColor,           0,         7, // PVPROP_PageBorderColor
0,  (PFP_PROPINFO)&PVProp_PageBorderWidth,           0,         8, // PVPROP_PageBorderWidth
0,  (PFP_PROPINFO)&PVProp_PageShadowColor,           0,         9, // PVPROP_PageShadowColor
0,  (PFP_PROPINFO)&PVProp_PageShadowWidth,           0,         10, // PVPROP_PageShadowWidth
0,  (PFP_PROPINFO)&PVProp_PageViewPercentage,        0,         11, // PVPROP_PageViewPercentage
0,  (PFP_PROPINFO)&PVProp_PageViewType,              0,         12, // PVPROP_PageViewType
0,  (PFP_PROPINFO)&PVProp_ScrollBarH,                0,         13, // PVPROP_ScrollBars
0,  (PFP_PROPINFO)&PVProp_ScrollBarV,                0,         14, // PVPROP_ScrollBars
0,  (PFP_PROPINFO)&PVProp_ScrollIncH,                0,         15, // PVPROP_ScrollIncH
0,  (PFP_PROPINFO)&PVProp_ScrollIncV,                0,         16, // PVPROP_ScrollIncV
0,  (PFP_PROPINFO)&PVProp_PageMultiCntH,             0,         17, // PVPROP_PageMultiCntH
0,  (PFP_PROPINFO)&PVProp_PageMultiCntV,             0,         18, // PVPROP_PageMultiCntV
0,  (PFP_PROPINFO)&PVProp_PageGutterH,               0,         19, // PVPROP_PageGutterH
0,  (PFP_PROPINFO)&PVProp_PageGutterV,               0,         20, // PVPROP_PageGutterV
0,  (PFP_PROPINFO)&PVProp_ZoomState,                 0,         21, // PVPROP_ZoomState
0,  (PFP_PROPINFO)&PVProp_PageCurrent,               0,         22, // PVPROP_PageCurrent
0,  (PFP_PROPINFO)&PVProp_PagesPerScreen,            0,         23, // PVPROP_PagesPerScreen
0,  (PFP_PROPINFO)&PVProp_PagePercentageActual,      0,         24, // PVPROP_PagePercentageActual
#ifdef NO_MFC
0,  (PFP_PROPINFO)&PVProp_MouseIcon,           FP_ALL | FP_NOTINBLOB, -522, // DAYPROP_MouseIcon (ATL only, using DISPID_MOUSEICON)
0,  (PFP_PROPINFO)&PVProp_OLEDropMode,              FP_NO_VB | FP_NO_DLL | FP_NOTINBLOB, 52, // BTNPROP_OLEDropMode
#endif
#ifdef SS_V35
0,  (PFP_PROPINFO)&PVProp_ScriptEnhanced,            0,         25, // PVProp_ScriptEnhanced
#endif
/*#ifdef PP4
0,  (PFP_PROPINFO)&PVProp_PageHeight,      0,         25, // PVPROP_PagePercentageActual
0,  (PFP_PROPINFO)&PVProp_PageWidth,      0,         26, // PVPROP_PagePercentageActual
#endif*/
};
