/*************
* CHECKBOX.H
*************/

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "superbtn.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TBCHECKBOX "tbCheckBox"

#define BN_CHECKBOXTYPING 20

#define CHECKBOX_PAINTALL  0
#define CHECKBOX_PAINTPICT 1
#define CHECKBOX_PAINTTEXT 2

#define BTNTEXTPICT_INC 6

typedef struct checkboxpicts
   {
   HANDLE        hInst;
   short         dMaxPictWidth;
   short         dMaxPictHeight;
   TBBTNPICT     PictUp;
   TBBTNPICT     PictDown;
   TBBTNPICT     PictGray;
   TBBTNPICT     PictFocusUp;
   TBBTNPICT     PictFocusDown;
   TBBTNPICT     PictFocusGray;
   TBBTNPICT     PictDisabledUp;
   TBBTNPICT     PictDisabledDown;
   TBBTNPICT     PictDisabledGray;
   } CHECKBOXPICTS, FAR *LPCHECKBOXPICTS;

typedef struct checkbox
   {
   GLOBALHANDLE  hText;
   HBRUSH        hBrushFocus;
   HBITMAP       hBitmapFocus;
   HBITMAP       hBitmapDC;
   HFONT         hFont;
   BYTE          cPaint;
   short         dCtlColorCode;
   long          lStyle;
   BOOL          fDisabled;
   BOOL          fFocus;
   BOOL          fChecked;
   BOOL          fButtonDown;
   BOOL          fMouseCaptureOn;
   BOOL          fUseDefPicts;
#ifdef SS_V80
   BOOL           MouseOver;
   BOOL			  Enhanced;
#endif
   CHECKBOXPICTS Picts;
   } CHECKBOX, FAR *LPCHECKBOX;

/**********************
* Function prototypes
**********************/

void DLLENTRY           CheckBoxLoadDefBitmaps(HANDLE hInst);
void DLLENTRY           CheckBoxUnloadDefBitmaps(void);
BOOL DLLENTRY           tbCheckBoxGetPict(HANDLE hInst,
                                          LPCHECKBOXPICTS lpDefPict);
WINENTRY                tbCheckBoxWndFn(HWND hWnd, UINT Msg, WPARAM wParam,
                                        LPARAM lParam);
WINENTRY                tbxCheckBoxWndFn(HWND hWnd, UINT Msg, WPARAM wParam,
                                         LPARAM lParam);
void                    CheckBoxSetPtr(HWND hWnd, GLOBALHANDLE hGlobal);
LPCHECKBOX              CheckBoxGetPtr(HWND hWnd, GLOBALHANDLE FAR *hGlobal);
void                    CheckBoxSetButtonDown(HWND hWnd, LPCHECKBOX lpCheckBox,
                                              BOOL fValue);
void                    CheckBoxSetCheck(HWND hWnd, LPCHECKBOX lpCheckBox,
                                         BOOL fValue);
void                    CheckBoxSetFocus(HWND hWnd, LPCHECKBOX lpCheckBox,
                                         BOOL fValue);
void                    CheckBoxFreePict(LPTBBTNPICT Pict);
void                    CheckBoxSetPict(HWND hWnd, HINSTANCE hInst,
                                        LPCHECKBOXPICTS lpPicts,
                                        WPARAM wParam, LPARAM lParam);

BOOL DLLENTRY           tbxCheckBoxLoadDef(GLOBALHANDLE FAR *lphDefPicts,
                                           LPSHORT dDefPictCnt, HANDLE hInst,
                                           WORD wButtonType, LPARAM lPict);
BOOL DLLENTRY           tbxCheckBoxUnloadDefs(GLOBALHANDLE FAR *lphDefPicts,
                                              LPSHORT dDefPictCnt,
                                              HANDLE hInst);
BOOL DLLENTRY           tbxCheckBoxGetPict(GLOBALHANDLE FAR *lphDefPicts,
                                           LPSHORT dDefPictCnt, HANDLE hInst,
                                           LPCHECKBOXPICTS lpDefPict);
void                    RadioBtnUnloadDefBitmaps(void);
void                    CheckBoxPaint(HWND hWnd, LPCHECKBOX lpCheckBox,
                                      HDC hDC, LPRECT lpRect, BOOL fUseDib,
                                      LPRECT lpRectOrig, BOOL fEraseBkgrnd);
long                    CheckBoxCreate(HWND hWnd, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif
