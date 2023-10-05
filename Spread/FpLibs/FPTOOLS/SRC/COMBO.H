/**********
* COMBO.H
**********/

#ifndef COMBO_H
#define COMBO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LP30	// classnames for version 3.0:

#ifdef FP_VB
#define CB_CLASSNAME_LIST "fpComboListVBX30"
#define CB_CLASSNAME_EDIT "fpComboEditVBX30"
#elif defined (FP_OCX)
// gic8774 -scl
#ifdef FP_ADB
#define CB_CLASSNAME_LIST "fpComboAdoListOCX30"
#define CB_CLASSNAME_EDIT "fpComboAdoEditOCX30"
#else
#define CB_CLASSNAME_LIST "fpComboListOCX30"
#define CB_CLASSNAME_EDIT "fpComboEditOCX30"
#endif
// gic8774 -scl
#else
#ifdef WIN32
#define CB_CLASSNAME_LIST "fpComboListDLL3230"
#define CB_CLASSNAME_EDIT "fpComboEditDLL3230"
#else
#define CB_CLASSNAME_LIST "fpComboListDLL1630"
#define CB_CLASSNAME_EDIT "fpComboEditDLL1630"
#endif
#endif

#else	// classnames for version 2.0:

#ifdef FP_VB
#define CB_CLASSNAME_LIST "fpComboListVBX20"
#define CB_CLASSNAME_EDIT "fpComboEditVBX20"
#elif defined (FP_OCX)
// gic8774 -scl
#ifdef FP_ADB
#define CB_CLASSNAME_LIST "fpComboAdoListOCX20"
#define CB_CLASSNAME_EDIT "fpComboAdoEditOCX20"
#else
#define CB_CLASSNAME_LIST "fpComboListOCX20"
#define CB_CLASSNAME_EDIT "fpComboEditOCX20"
#endif
// gic8774 -scl
#else
#define CB_CLASSNAME_LIST "fpComboListDLL20"
#define CB_CLASSNAME_EDIT "fpComboEditDLL20"
#endif

#endif

#define CB_ID_LIST 1561
#define CB_ID_EDIT 1562

#define FPCN_CLICK             123

#define CBM_GETLISTHWND        0x0700
#define CBM_ADJUSTCOMBOHEIGHT  0x0701
#define CBM_SENDKEYDOWN        0x0702
#define CBM_SENDKEYPRESS       0x0703
#define CBM_SETIGNORESELCHANGE 0x0704
#define CBMx_RESIZE            0x0705
#define CBMx_POSTSETTEXT       0x0706
#define CBMx_GETSTRUCT         0x0707
#define CBMx_REFRESHLIST       0x0708

#ifdef LP30
#define CB_DROPDOWNCOMBO_GAP -2
#else
#define CB_DROPDOWNCOMBO_GAP 7
#endif

#define CB_EDIT_MAXTEXT      150

#define CB_ISEDIT(lpCombo) (lpCombo->bStyle == CB_STYLE_DROPDOWN_COMBO || \
                            lpCombo->bStyle == CB_STYLE_SIMPLE_COMBO)
#define CB_ISBUTTON(lpCombo) (lpCombo->bStyle != CB_STYLE_SIMPLE_COMBO)

#if defined(FP_VB) || defined(FP_OCX)

typedef struct tagCB_BOUND
   {
   HCTL  hCtlDataControl;
   HCTL  hCtlDataControlConnected;
   HWND  hWndDataControl;
   BOOL  fBoundActive;
   BYTE  bFieldType;
   short nFieldNum;
   } CB_BOUND, FAR *LPCB_BOUND;

#endif

typedef struct tagCOMBO
   {
#if defined(FP_VB) || defined(FP_OCX)
   CB_BOUND       Bound;
   BOOL           fIgnoreDataChange;
   FPCONTROL      fpCtrl;
#endif
   HANDLE         fpInst;
   BOOL           fDesignTime;
   BYTE           bCtlType;
   HWND           hWnd;
   HWND           hWndList;
   HWND           hWndEdit;
   BOOL           fListDown;
   BOOL           fVB;
   long           lStyle;
   short          nBitmapDropDownWidth;
   short          nFontHeight;
   FPNCINFO       zNCInfoEdit;

   FPGLOBALHANDLE hDataSourceList;
   FPGLOBALHANDLE hDataFieldList;
   FPGLOBALHANDLE hText;
   short          nColumnEdit;
   short          nColumnBound;
   BYTE           bStyle;
   BYTE           bApplyTo;
   short          nMaxDrop;
   long           lListWidth;
   long           lEditHeight;
   COLORREF       clrGrayArea;
   long           lListLeftOffset;
   short          nEditHeightInPixels;
   short          nEditWidthInPixels;
   short          nListLeftOffsetInPixels;
   short          nListWidthInPixels;
   RECT           RectClientEdit;
   HBRUSH         hBrushBack;
   COLORREF       clrBack;
   COLORREF       clrFore;
   BOOL           fMouseCapture;
   BOOL           fButtonDepressed;
   BOOL           fIgnoreSelChange;
   BOOL           fIgnoreEditChange;
   BOOL           fLoaded;
   FPFONT         fpFont;
   long           lSelIndex;
   BOOL           fAdjustComboHeightSent;
   short          nComboGap;
   short          nMaxEditLen;
   short          nEditOffsetLeft;
   short          nEditOffsetTop;
   short          nEditOffsetRight;
   short          nEditOffsetBottom;
   HWND           hWndOwner;
   BOOL           fEnableKeyEvents;
   BOOL           fEnableMouseEvents;
   BOOL           fIsVisible;
   BOOL           fRefreshListSent;
   FPPICTID		  idMouseIcon;
#ifdef FP_DLL
   UINT           helpContextId;
#endif   
#ifdef LP30
// enhancement GIL2276 -scl
// IntelliMouse support
//   HWND           hWndVScrollWheel;
// enhancement GIL2276 -scl
   WNDPROC	      pEditWndProc;
   short		  nListPosition;
   BYTE			  bButtonThreeDAppearance;
   BYTE	          bButtonThreeDAppearanceDef;
   BOOL			  fIsNT;
   BOOL			  fIsWin2K;
   BYTE			  fNoRedraw;
   BYTE			  fAutoSearchFill;
   short		  nAutoSearchFillDelay;
   BOOL       fAutoMenu;
   BYTE       bEditAlignH;
   BYTE       bEditAlignV;
   BOOL       fEditAlignVChanging;
   BOOL       fAnimate;
   BOOL       fAllowAnimate;
   BOOL       fEditMouseActivate;
#endif
	BOOL       fInternalResize;
   } COMBO, FAR *LPCOMBO;

// cbprops.c

long PASCAL CB_SetProp(LPCOMBO lpCombo, WORD wProp, LPARAM lParam);
long PASCAL CB_GetProp(LPCOMBO lpCombo, WORD wProp, long lParam);
void        CB_SetBackColor(LPCOMBO lpCombo, COLORREF clrBack);
void        CB_SetForeColor(LPCOMBO lpCombo, COLORREF clrFore);

// combo.c

LONG WINAPI _export ComboProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL           CB_Register(HANDLE hInstance, LPSTR lpszClassName);
BOOL           CBx_Register(HANDLE hInstance);
BOOL           CBx_RegisterEdit(HANDLE hInstance);
LONG           CB_Main(LPCOMBO lpCombo, HWND hWnd, UINT Msg, WPARAM wParam,
                       LPARAM lParam, LPBOOL lpfCallDef);
FPGLOBALHANDLE CB_Create(HWND hWnd, LPCREATESTRUCT lParam);
BOOL           CBx_Create(HWND hWnd, long lStyle, LPCOMBO lpCombo);
void           CB_Destroy(FPGLOBALHANDLE hCombo);
void           CBx_Destroy(LPCOMBO lpCombo);
void           CB_AdjustComboHeight(LPCOMBO lpCombo);
void           CB_PerformAdjustComboHeight(LPCOMBO lpCombo);
long           CB_SendMessage(LPCOMBO lpCombo, UINT Msg, WPARAM wParam,
                              LPARAM lParam);
long           CB_SendWMCommand(LPCOMBO lpCombo, WORD wNotification);
LPCOMBO        CB_Lock(HWND hWnd);
#define        CB_Unlock(hWnd)
LPLISTBOX      CBLB_Lock(HWND hWnd);
#define        CBLB_Unlock(hWnd)
WORD           CB_MouseOverArea(LPCOMBO lpCombo, short x, short y);
FPGLOBALHANDLE CB_GetText(LPCOMBO lpCombo);
#ifdef LP30
void CB_GetEditRectActual(LPCOMBO lpCombo, LPRECT lpRect);
#endif

BOOL DLLENTRY CB_OCXGetMessageReflect(LPCOMBO lpCombo);

#ifdef LP30
void		  CB_GetButtonRect(LPCOMBO lpCombo, LPRECT lpRect);

#define CB_BUTTONTHREEDAPPEARANCE_WIN95		1
#define CB_BUTTONTHREEDAPPEARANCE_WINDOWS	2
#define CB_ISWIN95BUTTON(lpCombo) (CB_BUTTONTHREEDAPPEARANCE_WIN95 == lpCombo->bButtonThreeDAppearance || \
									(!lpCombo->bButtonThreeDAppearance && \
									CB_BUTTONTHREEDAPPEARANCE_WIN95 == lpCombo->bButtonThreeDAppearanceDef))
#endif

short ComboCalcListRect(LPLISTBOX lpList, RECT *pListRect, RECT *pClientRect, BOOL bShowingList, BOOL *pfOpenUp);

#ifdef __cplusplus
}
#endif    

#endif
