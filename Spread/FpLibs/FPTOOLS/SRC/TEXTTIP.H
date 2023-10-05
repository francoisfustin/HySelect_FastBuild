/************
* TEXTTIP.H
************/

#ifndef TEXTTIP_H
#define TEXTTIP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

// -> fix for GIC21056 -scl
#define TT_PROP_FOCUS _T("fpTextTipFocus")
// <- fix for GIC21056 -scl

#define FPM_TT_FETCH 0x0771
#define FPM_TT_SCAN  0x0772

#define FP_TT_STATUS_OFF               0
#define FP_TT_STATUS_FIXED             1
#define FP_TT_STATUS_FLOATING          2
#define FP_TT_STATUS_FIXEDFOCUSONLY    3
#define FP_TT_STATUS_FLOATINGFOCUSONLY 4

#define FP_TT_ITEM_DELAY     0x0001
#define FP_TT_ITEM_STATUS    0x0002
#define FP_TT_ITEM_ACTIVE    0x0004
#define FP_TT_ITEM_BACKCOLOR 0x0008
#define FP_TT_ITEM_FORECOLOR 0x0010
#define FP_TT_ITEM_FONT      0x0020
#define FP_TT_ITEM_SCROLLTIPSTATUS 0x0040
#define FP_TT_ITEM_ALL       0xffff

#define FP_TT_XALIGN_AUTO   0
#define FP_TT_XALIGN_LEFT   1
#define FP_TT_XALIGN_RIGHT  2
#define FP_TT_XALIGN_CENTER 3

#define FP_TT_YALIGN_AUTO   0
#define FP_TT_YALIGN_TOP    1
#define FP_TT_YALIGN_BOTTOM 2
#define FP_TT_YALIGN_CENTER 3

#define FP_TT_MULTILINE_SINGLE 0
#define FP_TT_MULTILINE_MULTI  1
#define FP_TT_MULTILINE_AUTO   2

#define FP_TT_TEXTMAX 1024

#ifdef TT_V2
#define FP_TT_CELLNOTE_WIDTH  130
#define FP_TT_CELLNOTE_HEIGHT 135
#endif

typedef struct tagFP_TT_INFO
   {
   long     lDelay;
   WORD     wStatus;
   BOOL     fActive;
   COLORREF clrBack;
   COLORREF clrFore;
   LOGFONT  LogFont;
#ifdef TT_V2
   BOOL     fShowScrollTips;
#endif
   } FP_TT_INFO, FAR *LPFP_TT_INFO;

typedef struct tagFP_TT_ID
   {
   long lID1;
   long lID2;
#ifdef TT_V2
   long lID3;
#endif
   } FP_TT_ID, FAR *LPFP_TT_ID;

typedef struct tagFP_TT_SCAN
   {
   short    nMouseX;
   short    nMouseY;
   FP_TT_ID ID;
   } FP_TT_SCAN, FAR *LPFP_TT_SCAN;

typedef struct tagFP_TT_FETCH
   {
   FP_TT_SCAN Scan;
   BOOL       fShow;
   WORD       wMultiLine;
   WORD       wAlignX;
   WORD       wAlignY;
   short      xOffset;
   short      yOffset;
   short      nWidth;
   RECT       Rect;
   TCHAR      szText[FP_TT_TEXTMAX + 1];
	HGLOBAL    hText;  // If this member is non-empty it is used instead of szText.
                      // The app does not need to free it.  TextTip will free it.
   } FP_TT_FETCH, FAR *LPFP_TT_FETCH;

/**********************
* Function prototypes
**********************/

BOOL FPLIB fpTextTipSetInfo(HWND hWndParent, WORD wItem, LPFP_TT_INFO lpInfo);
BOOL FPLIB fpTextTipGetInfo(HWND hWndParent, LPFP_TT_INFO lpInfo);
void FPLIB fpTextTipTerminate(HWND hWndParent);
#ifdef TT_V2
HWND FPLIB fpTextTipGetHwnd(HWND hWndParent);
void fpTextTipSetScrollTip(HWND hWndParent, BOOL fScrollTip);
BOOL fpTextTipGetScrollTip(HWND hWndParent);
void fpTextTipHideScrollTip(HWND hWndParent);
#endif

#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#ifdef __cplusplus
}
#endif

#endif
