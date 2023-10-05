/***********
* SSPRVW.H
***********/

#ifndef SSPRVW_H
#define SSPRVW_H

#include "fptools.h"
#include "fpmemmgr.h"
#include "fppinfo.h"
#include "prvwapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/************
* SSPreview
************/

enum 
   {                      
   // VB properties
   PVPROP_VBX_CTLNAME,
   PVPROP_VBX_INDEX,
   PVPROP_VBX_LEFT,
   PVPROP_VBX_TOP,
   PVPROP_VBX_WIDTH,
   PVPROP_VBX_HEIGHT,
   PVPROP_VBX_MOUSEPOINTER,
   PVPROP_VBX_TABINDEX,
   PVPROP_VBX_TABSTOP,
   PVPROP_VBX_DRAGICON,
   PVPROP_VBX_DRAGMODE,
   PVPROP_VBX_ENABLED,
   PVPROP_VBX_PARENT,
   PVPROP_VBX_TAG,
   PVPROP_VBX_VISIBLE,
   PVPROP_VBX_BORDERSTYLE,
   PVPROP_VBX_HELPCONTEXTID,

   PVPROP_ABOUTBOX,
   PVPROP_ALLOWUSERZOOM,
   PVPROP_GRAYAREACOLOR,
   PVPROP_GRAYAREAMARGINH,
   PVPROP_GRAYAREAMARGINTYPE,
   PVPROP_GRAYAREAMARGINV,
   PVPROP_HWNDSPREAD,
   PVPROP_PAGEBORDERCOLOR,
   PVPROP_PAGEBORDERWIDTH,
   PVPROP_PAGESHADOWCOLOR,
   PVPROP_PAGESHADOWWIDTH,
   PVPROP_PAGEVIEWPERCENTAGE,
   PVPROP_PAGEVIEWTYPE,
   PVPROP_SCROLLBARH,
   PVPROP_SCROLLBARV,
   PVPROP_SCROLLINCH,
   PVPROP_SCROLLINCV,
   PVPROP_PAGEMULTICNTH,
   PVPROP_PAGEMULTICNTV,
   PVPROP_PAGEGUTTERH,
   PVPROP_PAGEGUTTERV,
   PVPROP_ZOOMSTATE,
   PVPROP_PAGECURRENT,
   PVPROP_PAGESPERSCREEN,
   PVPROP_PAGEPERCENTAGEACTUAL,
#ifdef NO_MFC
   PVPROP_MOUSEICON,
   PVPROP_OLEDROPMODE,
#endif
#ifdef SS_V35
   PVPROP_SCRIPTENHANCED,
#endif
//#ifdef PP4
//   PVPROP_PAGEHEIGHT,
//   PVPROP_PAGEWIDTH,
//#endif
   PVPROP_PROPCNT
   };

extern FP_PROPLIST PVPropList[PVPROP_PROPCNT];
#ifdef SS_V80
#ifdef _UNICODE
#define FPCLASS_SSPREVIEW     _T("fpUSpreadPreview80")
#else
#define FPCLASS_SSPREVIEW     _T("fpSpreadPreview80")
#endif

#elif SS_V75

#ifdef _UNICODE
#define FPCLASS_SSPREVIEW     _T("fpUSpreadPreview75")
#else
#define FPCLASS_SSPREVIEW     _T("fpSpreadPreview75")
#endif

#elif SS_V70

#if (defined(FP_ADB))
#define FPCLASS_SSPREVIEW     _T("fpADOSpreadPreview70")
#elif (defined(FP_OCX))
#define FPCLASS_SSPREVIEW     _T("fpOCXSpreadPreview70")
#elif (defined(WIN32))
#define FPCLASS_SSPREVIEW     _T("fpSpreadPreview70")
#else
#define FPCLASS_SSPREVIEW     "fp16SpreadPreview70"
#endif

#elif SS_V40

#if (defined(FP_ADB))
#define FPCLASS_SSPREVIEW     _T("fpADOSpreadPreview60")
#elif (defined(FP_OCX))
#define FPCLASS_SSPREVIEW     _T("fpOCXSpreadPreview60")
#elif (defined(WIN32))
#define FPCLASS_SSPREVIEW     _T("fpSpreadPreview60")
#else
#define FPCLASS_SSPREVIEW     "fp16SpreadPreview60"
#endif

#elif defined(SS_V35)

#if (defined(FP_ADB))
#define FPCLASS_SSPREVIEW     _T("fpADOSpreadPreview35")
#elif (defined(FP_OCX))
#define FPCLASS_SSPREVIEW     _T("fpOCXSpreadPreview35")
#elif (defined(WIN32))
#define FPCLASS_SSPREVIEW     _T("fpSpreadPreview35")
#else
#define FPCLASS_SSPREVIEW     "fp16SpreadPreview35"
#endif

#else // version 3.0:

#if (defined(FP_ADB))
#define FPCLASS_SSPREVIEW     _T("fpADOSpreadPreview")
#elif (defined(FP_OCX))
#define FPCLASS_SSPREVIEW     _T("fpOCXSpreadPreview")
#elif (defined(WIN32))
#define FPCLASS_SSPREVIEW     _T("fpSpreadPreview")
#else
#define FPCLASS_SSPREVIEW     "fp16SpreadPreview"
#endif

#endif

#define CLASS_ABOUTBOX                "AboutBoxClass"

#define SPM_OPENABOUTBOX               WM_USER
#define SPM_SETPROPERTY                (WM_USER + 1)
#define SPM_GETPROPERTY                (WM_USER + 2)
#ifdef NO_MFC
#define PP_MOUSEICONCHANGED         (WM_USER + 3)
#endif

#define SPN_PAGECHANGE                 1
#define SPN_ZOOM                       2

#define SSPRVW_MARGINTYPE_NORMAL       0
#define SSPRVW_MARGINTYPE_ACTUAL       1

#define SSPRVW_VIEWTYPE_WHOLEPAGE      0
#define SSPRVW_VIEWTYPE_NORMALSIZE     1
#define SSPRVW_VIEWTYPE_PERCENTAGE     2
#define SSPRVW_VIEWTYPE_PAGEWIDTH      3
#define SSPRVW_VIEWTYPE_PAGEHEIGHT     4
#define SSPRVW_VIEWTYPE_MULTIPLEPAGES  5

#define SSPRVW_MARGINDEF               720
#define SSPRVW_SCROLLINCHDEF           360
#define SSPRVW_SCROLLINCVDEF           360

#define SSPRVW_ZOOMSTATE_INDETERMINATE 0
#define SSPRVW_ZOOMSTATE_IN            1
#define SSPRVW_ZOOMSTATE_OUT           2
#define SSPRVW_ZOOMSTATE_SWITCH        3

#define SSPRVW_SCROLLBAR_SHOW          0
#define SSPRVW_SCROLLBAR_AUTO          1
#define SSPRVW_SCROLLBAR_HIDE          2

typedef HBITMAP FAR *LPHBITMAP;

#define TWIPSTOPIXELSX(lpSSPreview, lTwips) (short)(((long)lpSSPreview->nPixelsPerInchX * lTwips) / 1440)
#define TWIPSTOPIXELSY(lpSSPreview, lTwips) (short)(((long)lpSSPreview->nPixelsPerInchY * lTwips) / 1440)

#define PAGEWIDTH(lpSSPreview, lpRect, nMargin, nPageCnt, nGutter)  (max(0, ((lpRect)->right - (lpRect)->left - (((nPageCnt) - 1) * (nGutter)) - (2 * (nMargin)) - (nPageCnt * ((lpSSPreview->nPageBorderWidth * 2) + lpSSPreview->nPageShadowWidth))) / (nPageCnt)))
#define PAGEHEIGHT(lpSSPreview, lpRect, nMargin, nPageCnt, nGutter) (max(0, ((lpRect)->bottom - (lpRect)->top - (((nPageCnt) - 1) * (nGutter)) - (2 * (nMargin)) - (nPageCnt * ((lpSSPreview->nPageBorderWidth * 2) + lpSSPreview->nPageShadowWidth))) / (nPageCnt)))

#define PAGECNT(lpSSPreview) ((lpSSPreview)->lPageEnd - (lpSSPreview)->lPageBeg + 1)

#define DIVUP(A, B) (((A) + (B) - 1) / (B))

#define PV_ALLOWMARGINRESIZE(lpSSPreview) (lpSSPreview->fAllowMarginResize && lpSSPreview->fShowMargins)

typedef struct sspreview
   {
   HWND         hWnd;
#ifdef FP_VB
   HCTL         hCtl;
#endif
   BOOL         fDesignTime;
   BOOL         fAllowUserZoom;
   COLORREF     GrayAreaColor;
   long         lGrayAreaMarginH;
   WORD         wGrayAreaMarginType;
   long         lGrayAreaMarginV;
   HWND         hWndSpread;
   COLORREF     PageBorderColor;
   short        nPageBorderWidth;
   COLORREF     PageShadowColor;
   short        nPageShadowWidth;
   short        nPageViewPercentage;
   WORD         wPageViewType;
   WORD         wScrollBarH;
   WORD         wScrollBarV;
   long         lScrollIncH;
   long         lScrollIncV;
   BOOL         fShowMargins;
   short        nPixelsPerInchX;
   short        nPixelsPerInchY;
   long         lScrollPosH;
   long         lScrollPosV;
   long         lScrollMaxH;
   long         lScrollMaxV;
   short        nPageMultiCntH;
   short        nPageMultiCntV;
   long         lPageBeg;
   long         lPageEnd;
   long         lPageCurrent;
   long         lPageGutterH;
   long         lPageGutterV;
   HCURSOR      hCursorZoomIn;
   HCURSOR      hCursorZoomOut;
   HCURSOR      hCursorCurrent;
   WORD         wZoomState;
   WORD         wPageViewTypePrev;
   short        nPageMultiCntHPrev;
   short        nPageMultiCntVPrev;
   BOOL         fAllowMarginResize;
   PRVW_MARGINS Margins;
   float        fPageWidth;
   float        fPageHeight;
   GLOBALHANDLE hPagesInMem;
   long         lPagesInMemBeg;
   long         lPagesInMemEnd;
   RECT         RectClient;
   BOOL         fNoMemDCPaint;
#ifdef NO_MFC   
   short		idMouseIcon;
#endif
#ifdef SS_V35
   BOOL         bScriptEnhanced;
#endif
   } SSPREVIEW, FAR *LPSSPREVIEW;

BOOL         SSPreviewRegister(HINSTANCE hInst);
BOOL         SSPreviewUnregister(HINSTANCE hInst);
LRESULT      fpSSPreviewMain(LPSSPREVIEW lpSSPreview, HWND hWnd, UINT Msg,
                             WPARAM wParam, LPARAM lParam, BOOL FAR *lpfCallDef);
GLOBALHANDLE SSPreviewCreate(HWND hWnd, LONG Style);
void         SSPreviewDestroy(HWND hWnd, GLOBALHANDLE hGlobal);
void         SSPreviewGetMargins(LPSSPREVIEW lpSSPreview, LPSHORT lpnMarginH,
                                 LPSHORT lpnMarginV);
void         SSPreviewGetPageSize(LPSSPREVIEW lpSSPreview, LPRECT lpRect,
                                  short nMarginH, short nMarginV);
short        SSPreviewGetPagesPerScreen(LPSSPREVIEW lpSSPreview);
void         SSPreviewCalcPage(LPSSPREVIEW lpSSPreview);
void         SSPreviewZoom(LPSSPREVIEW lpSSPreview);
void         SSPreviewSetPageCurrent(LPSSPREVIEW lpSSPreview,
                                     long lPageCurrent);
void         SSPreviewSrcOpen(LPSSPREVIEW lpSSPreview);
void         SSPreviewSrcClose(LPSSPREVIEW lpSSPreview);
void         SSPreviewSrcGetInfo(LPSSPREVIEW lpSSPreview);
void         SSPreviewFreePagesInMem(LPSSPREVIEW lpSSPreview);
LPSSPREVIEW  PV_Lock(HWND hWnd);
void         PV_Unlock(HWND hWnd);
LRESULT      PV_SendWMCommand(LPSSPREVIEW lpSSPreview, WORD wNotification);
LRESULT      PV_SendMessage(LPSSPREVIEW lpSSPreview, UINT Msg, WPARAM wParam,
                            LPARAM lParam);

// pvpaint

void         SSPreviewPaint(LPSSPREVIEW lpSSPreview, HWND hWnd, HDC hDCDev,
                            HDC hDC, LPRECT lpRectClient);

// pvprops

LRESULT PASCAL  PV_SetProp(LPSSPREVIEW lpSSPreview, WORD wProp, LPARAM lParam);
LRESULT PASCAL  PV_GetProp(LPSSPREVIEW lpSSPreview, WORD wProp, LPARAM lParam);

// pvscroll

void         SSPreviewSetScrollBars(LPSSPREVIEW lpSSPreview);
void         SSPreviewHScroll(LPSSPREVIEW lpSSPreview, WORD wParam,
                              LONG lParam);
void         SSPreviewVScroll(LPSSPREVIEW lpSSPreview, WORD wParam,
                              LONG lParam);

BOOL         SSPreviewIsScrollPartial(LPSSPREVIEW lpSSPreview);

#ifdef PP40
WORD fpSplashIsExpired(HINSTANCE);
BOOL fpIsTrialVersion();

#define FPSPLASH_NONE    0
#define FPSPLASH_OK      1
#define FPSPLASH_EXPIRED 2
#endif

#ifdef __cplusplus
}
#endif

#endif
