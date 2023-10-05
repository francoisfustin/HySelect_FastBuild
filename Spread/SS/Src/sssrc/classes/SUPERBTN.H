/*************
* SUPERBTN.H
*************/

#ifndef SUPERBTN_H
#define SUPERBTN_H

#ifdef __cplusplus
extern "C" {
#endif

#define TBSUPERBTN "tbSuperBtn"

typedef struct tbbtnpict
   {
   BYTE   PictType;
   short  PictWidth;
   short  PictHeight;
   HANDLE hPict;
   HANDLE hPictStretch;
   BOOL   HandleProvided;
   } TBBTNPICT, FAR *LPTBBTNPICT;

typedef struct superbtn
   {
   GLOBALHANDLE  hText;
   HBRUSH        hBrushFocus;
   HBITMAP       hBitmapDC;
   HFONT         hFont;
   long          lStyle;
   short         ShadowSize;
   short         BtnFrameOffset;
   short         ButtonType;
   BOOL          ButtonDown;
   BOOL          MouseCaptureOn;
   BOOL          fStretch;
   TBBTNPICT     Pict;
   TBBTNPICT     PictBtnDown;
   TBBTNPICT     PictDisabled;
   SUPERBTNCOLOR Color;
   BOOL          fIs95;
#ifdef SS_V80
   BOOL          MouseOver;
   BOOL			 Enhanced;
#endif
   } SUPERBTN, FAR *LPSUPERBTN;

/**********************
* Function prototypes
**********************/

WINENTRY tbSuperBtnWndFn(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void     SuperBtnPaint(HWND hWnd, LPSUPERBTN lpSuperBtn, HDC hDC, LPRECT lpRect,
                       BOOL fIsEnabled, BOOL fFocus, BOOL fRoundCorners,
                       BOOL fUseDib, LPRECT lpRectOrig, BOOL fIsButton);
BOOL     SuperBtnSetPict(HWND hWnd, HANDLE hInst, LPTBBTNPICT Pict,
                         WORD wPictType, LPARAM lPict);
void     SuperBtnFreePict(LPTBBTNPICT Pict);
extern LPSUPERBTN SuperBtnGetPtr(HWND hWnd, GLOBALHANDLE FAR *hGlobal);

#ifdef __cplusplus
}
#endif

#endif
