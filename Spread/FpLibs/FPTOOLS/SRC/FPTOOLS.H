#ifndef FPTOOLS_H
#define FPTOOLS_H

// SCP 12/5/96
#if defined(FP_OCX)         // Prefix for OCX functions
#define FP_PREFIX(x)  X_##x
#elif (defined(FP_DLL) && !defined(FP_OCX))           // Prefix for DLL functions
#define FP_PREFIX(x)  x
#elif defined(FP_VB)          // Prefix for VBX functions
#define FP_PREFIX(x)  V_##x   
#endif

#if (defined(FP_OCX) || defined(FP_DLL))
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		//exclude conflicts with VBAPI.H in WIN32
#endif
#endif

#include "windows.h"
#ifndef RC_INVOKED
#include <math.h>
#endif

#include "fpconvrt.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

// NOFPFONT
// NOFPGLOBAL
// NOFPTXTOUT
// NOFP3DTEXT
// NOFPHELP  
// NOFPFILE
// NOFPINTL

//
// Property Values for BOOL properties:
//
#ifdef FP_DLL
#define PROP_TRUE  TRUE                // C's TRUE  (= 1)
#else
#define PROP_TRUE  (-1)                // Visual Basic's "True" = -1
#endif


//
// FarPoint Messages
//
//
#define REDRAW_OFF   0
#define REDRAW_ON    1
#define REDRAW_QUERY 2

#define CTLT_GET       0
#define CTLT_SET       1

#define CTLT_NORMAL    0
#define CTLT_READONLY  1
#define CTLT_STATIC    2
#define CTLT_BUTTON    3

#define FPM_REDRAW                           (WM_USER+100)
#define FPM_GETNCINFO                        (WM_USER+101)
#define FPM_SETNCINFO                        (WM_USER+102)
#define FPM_BUTTONHIT                        (WM_USER+103)
#define FPM_INVALIDDATA                      (WM_USER+104)
#define FPM_INVALIDPOST                      (WM_USER+105)
#define FPM_AUTOADVANCE                      (WM_USER+106)
#define FPM_AUTOBEEP                         (WM_USER+107)
#define FPM_USERSELECT                       (WM_USER+109)
#define FPM_SETDEFAULT                       (WM_USER+110)
#define FPM_SETNULL                          (WM_USER+111)
#define FPM_CONTROLTYPE                      (WM_USER+112)
#define FPM_NCPAINT                          (WM_USER+113)
//A261, A1003...
#define FPM_GETALLOWMODIFIED                 (WM_USER+114)
#define FPM_SETALLOWMODIFIED                 (WM_USER+115)   
//A261, A1003

// NOTE: The following are used for the OCX controls.
#define FPM_LOADPROPERTY                     (WM_USER + 116)
#define FPM_SAVEPROPERTY                     (WM_USER + 117)

#define FPN_BUTTONMIN                        9001
#define FPN_BUTTONMAX                        9002
#define FPN_CHANGE                           EN_CHANGE
#define FPN_INSERTMODE                       9004
#define FPN_OVERWRITEMODE                    9005
#define FPN_INVALIDCHAR                      9006
#define FPN_INVALIDACTION                    9007
#define FPN_INVALIDMOVE                      9008
#define FPN_EXCEEDLIMIT                      9009
#define FPN_AUTOADVANCEPREV                  9011
#define FPN_AUTOADVANCENEXT                  9012
#define FPN_AUTOADVANCEDOWN                  9013
#define FPN_AUTOADVANCEUP                    9014
#define FPN_OUTOFRANGE                       9015
#define FPN_SETCLOCK                         9016
#define FPN_LMOUSECLICK                      9017
#define FPN_RMOUSECLICK                      9018
#define FPN_CLICK                            9019
#define FPN_BUTTONHIT                        9020
#define FPN_INVALIDDATA                      9021
#define FPN_POPUP                            9022
#define FPN_TIMER                            9023
#define FPN_ALARM                            9024
#define FPN_MOVEHAND                         9025
#define FPN_CALCHANGE						 9026
#define FPN_CALDBLCLK						 9027
#define FPN_CALZOOM    						 9028
#define FPN_CALVIEWCHANGE    				 9029
#define FPN_NOPOP							 9030
#define FPN_PBRWBEGINUPDATE                  9031
#define FPN_PBRWENDUPDATE                    9032
//#ifdef NEWDATE
#define FPN_POPCLOSE						 9033
//#endif

typedef struct tagClockSet
   {
   int new_hr, new_mn, new_sc;
   int old_hr, old_mn, old_sc;
   } CLOCKSET, FAR * LPCLOCKSET;


#define FPM_USER                             (WM_USER+125)

//
// Useful Macros
//

#ifdef WINENTRY
#undef WINENTRY
#endif

#ifdef DLGENTRY
#undef DLGENTRY
#endif

#if _WIN64
#define WINENTRY LRESULT FP_API
#else
#define WINENTRY LONG FP_API
#endif
#define DLGENTRY BOOL FP_API

typedef short FAR *LPSHORT;

typedef VOID huge *HPVOID;
typedef char huge *HPSTR;
typedef BYTE huge *HPBYTE;

#ifndef VS7
#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif
#endif

#define SEGMENT(lp)         (*((unsigned *)&(lp) + 1))
#define OFFSET(lp)          (*((unsigned *)&(lp)))

typedef double FAR * LPDOUBLE;
typedef BOOL FAR * LPBOOL;           
//
// Integer conversion
//
#define FP_INT_MAX       (32767)         /* max integer supported - rdw */
#define FP_INT_MIN       (-32767)        /* min integer supported - rdw */
#define FP_INT_OVERFLOW  (FP_INT_MIN-1)  /* StringToInt overflow  - rdw */
#define FP_LONG_MAX      (2147483647)    /* max long supported    - rdw */
#define FP_LONG_MIN      (-2147483647)   /* min long supported    - rdw */
#define FP_LONG_OVERFLOW (FP_LONG_MIN-1) /* StringToLong overflow - rdw */

//
// Communication functions
//
//    fpPostMessageToParent    FPTOOLS06.C
//    fpNotifyParent           FPTOOLS07.C
//    fpSendMessageToParent    FPTOOLS08.C
//    fpSendTextToClipboard    FPTOOLS09.C
//    fpGetTextFromClipboard   FPTOOLS10.C
//

long   FPLIB fpPostMessageToParent  (HWND hWnd, WORD wMsg, WPARAM wParam, LPARAM lParam);
#ifdef _WIN64
LONG_PTR FPLIB fpSendMessageToParent  (HWND hWnd, WORD wMsg, WPARAM wParam, LPARAM lParam);
LONG_PTR FPLIB fpNotifyParent         (HWND hWnd, WORD wNotifyMsg, BOOL fSuspend, BOOL FAR *lpbIsHwndValid); //A849
#else
long FPLIB fpSendMessageToParent  (HWND hWnd, WORD wMsg, WPARAM wParam, LPARAM lParam);  
long   FPLIB fpNotifyParent         (HWND hWnd, WORD wNotifyMsg, BOOL fSuspend, BOOL FAR *lpbIsHwndValid); //A849
#endif
BOOL   FPLIB fpSendTextToClipboard  (HWND hWnd, LPTSTR lpszText, UINT dLen);
HANDLE FPLIB fpGetTextFromClipboard (HWND hWnd);

// 
// Miscellaneous Functions
//
//    fpDrawShadows        FPTOOLS03.C
//    fpCenterWindow       FPTOOLS04.C
//    fpDoubleToString     FPTOOLS05.C
//
//    fpAllocateData       FPTOOLS11.C  (smartheap)
//    fpStringToInt        FPTOOLS12.C
//    SysRead              FPTOOLS14.C
//	  fpStringToValue      FPTOOLS12.C

void FPLIB fpCenterWindow (HWND hWnd);

#if defined(_WIN64) || defined(_IA64)
int DLLENTRY fpSysRead (LONG_PTR fileHandle, LPVOID buffer, unsigned bytesToRead, LPWORD bytesRead);
#else
int DLLENTRY SysRead (int fileHandle, LPVOID buffer, unsigned bytesToRead, LPWORD bytesRead);
#endif

LPTSTR FPLIB fpDoubleToString (LPTSTR lpszString, double value, short dPlaces,
   TCHAR cDecimal, TCHAR cSeparator, BOOL fLeadingZero, BOOL fFixedPoint,
   BOOL fNegSign, BOOL fFormatted, short dCurrencyStyle, 
   LPTSTR lpszCurrencySymbol);
LPTSTR FPLIB fpIntlDoubleToString (LPTSTR lpszString, double value, short dPlaces,
   LPTSTR lpszDecimal, LPTSTR lpszSeparator, BOOL fLeadingZero, BOOL fFixedPoint,
   BYTE bNegSign, BOOL fFormatted, WORD wCurrencyStyle, LPTSTR lpszCurrencySymbol, LPTSTR lpszNegSymbol,
   BOOL fIsCurrency);

LPTSTR  FPLIB fpStrRev(LPTSTR lpS1);
LPTSTR  FPLIB fpIntToString(int nVal, LPTSTR lpStr);
LPTSTR  FPLIB fpLongToString(long lVal, LPTSTR lpStr);
int     FPLIB fpStringToInt(LPTSTR lpStr);
long    FPLIB fpStringToLong(LPTSTR lpStr, int base);

DWORD FPLIB fpAllocateData (HANDLE FAR* hData, DWORD dwOldSize, DWORD dwLength);

void FPLIB fpDrawShadows(HDC hDC, LPRECT lpRect, short iShadowSize, 
   COLORREF ColorShadow, COLORREF ColorHighlight, BOOL fShadowIn, 
   BOOL fShadowButton);


#ifndef NOFP3DTEXT
//
// FarPoint 3D Text Utilities
//
//    fp3DTextInit         FPTOOL01
//    fpDrawText3D         FPTOOL02
//

#define TEXT3D_UP           1 // Etched
#define TEXT3D_DOWN         2 // Embossed
#define TEXT3D_MOREUP       3 // Etched w/ shading
#define TEXT3D_MOREDOWN     4 // Embossed w/ shading

typedef short FPCOLORID, FAR *LPFPCOLORID;

typedef struct tagTEXT3DINFO
   {
   COLORREF colorShadow;
   COLORREF colorHilite;
   FPCOLORID idClrHiLite;
   FPCOLORID idClrShadow;
   BYTE     bVOffset;
   BYTE     bHOffset;
   BYTE     bStyle;
   } TEXT3DINFO, FAR *LPTEXT3DINFO;


void FPLIB fp3DTextInit(LPTEXT3DINFO lp3DText);

BOOL FPLIB fpDrawText3D (HDC hDC, LPTSTR lpszText, short dLen, LPRECT lpRect,
   WORD wFormat, LPTEXT3DINFO lp3DText);

#endif // NOFP3DTEXT


#ifndef NOFPGLOBAL
//
// FarPoint Global Data Manager - FPTOOLS15.C
//
//    fpCreateGlobalData 
//    fpDestroyGlobalData 
//    fpLockGlobalData 
//    fpUnlockGlobalData 
//

typedef struct fpGlobalDataManager
{

   GLOBALHANDLE hData;         // handle to FPGLOBALDATA array
   UINT fuUse;                 // usage count
   UINT fuN;                   // number of FPGLOBALDATA structures
#ifdef FP_APARTMENT	
   CRITICAL_SECTION	cs; // for Apartment model threading -scl
#endif
} FPGLOBALDATAMANAGER, FAR *LPFPGLOBALDATAMANAGER;

BOOL   FPLIB fpCreateGlobalData (LPFPGLOBALDATAMANAGER lpDM, HINSTANCE hInstance, UINT fuSize);
BOOL   FPLIB fpDestroyGlobalData (LPFPGLOBALDATAMANAGER lpDM, HINSTANCE hInstance, void (*fpDestroy)(GLOBALHANDLE, UINT));
LPVOID FPLIB fpLockGlobalData (LPFPGLOBALDATAMANAGER lpDM, HINSTANCE hInstance, UINT FAR *lpfuN);
void   FPLIB fpUnlockGlobalData (LPFPGLOBALDATAMANAGER lpDM, HINSTANCE hInstance);

#endif // NOFPGLOBAL                                          

#ifndef NOFPFONT
//
// FarPoint Font Management Utilities - FPTOOL16.C
//
//    fpGetFontTable
//    fpReleaseFontTable 
//    fpGetLogFont 
//    fpGetFont 
//    fpReleaseFont 
//
//    fpGetFontLogFont 
//    fpGetFontHandle 
//
//    fpLockCharWidths 
//    fpUnlockCharWidths 
//    fpGetTextWidth 
//    fpGetCharWidth 
//

typedef int FAR *LPCHARWIDTHS;
typedef short FPFONT;

FPFONT FPLIB fpRotateFont(HDC hDC, HFONT hFont, int lfEscapement);
// fix for JUS47 -scl
FPFONT FPLIB fpRotateFontEx(HINSTANCE hInst, HDC hDC, HFONT hFont, int lfEscapement);
// fix for JUS47 -scl


BOOL FPLIB      fpCM_Init(void);
BOOL FPLIB      fpCM_InitEx(HINSTANCE hInst);
void FPLIB      fpCM_Free(void);
void FPLIB      fpCM_FreeEx(HINSTANCE hInst);
FPCOLORID FPLIB fpCM_AddItem(COLORREF clr);
FPCOLORID FPLIB fpCM_AddItemEx(HINSTANCE hInst, COLORREF clr);
void FPLIB      fpCM_FreeItem(FPCOLORID fpColorID);
void FPLIB      fpCM_FreeItemEx(HINSTANCE hInst, FPCOLORID fpColorID);
void FPLIB      fpCM_RefItem(FPCOLORID fpColorID);
void FPLIB      fpCM_RefItemEx(HINSTANCE hInst, FPCOLORID fpColorID);
void FPLIB      fpCM_ChangeItem(LPFPCOLORID lpfpColorID, COLORREF clr);
void FPLIB      fpCM_ChangeItemEx(HINSTANCE hInst, LPFPCOLORID lpfpColorID, COLORREF clr);
COLORREF FPLIB  fpCM_GetItem(FPCOLORID fpColorID, LPHANDLE lphBrush,
                                LPHANDLE lphPen);
COLORREF FPLIB  fpCM_GetItemEx(HINSTANCE hInst, FPCOLORID fpColorID, LPHANDLE lphBrush,
                                LPHANDLE lphPen);
COLORREF FPLIB  fpCM_LockItem(FPCOLORID fpColorID, LPHANDLE lphBrush,
                                 LPHANDLE lphPen);
COLORREF FPLIB  fpCM_LockItemEx(HINSTANCE hInst, FPCOLORID fpColorID, LPHANDLE lphBrush,
                                 LPHANDLE lphPen);
void FPLIB      fpCM_UnlockItem(FPCOLORID fpColorID);
void FPLIB      fpCM_UnlockItemEx(HINSTANCE hInst, FPCOLORID fpColorID);
UINT FPLIB      fpCM_GetCount(void);
UINT FPLIB      fpCM_GetCountEx(HINSTANCE hInst);
void FPLIB      fpCM_SysColorChange(void);
void FPLIB      fpCM_SysColorChangeEx(HINSTANCE hInst);


// Support for Setting Tabstops                                     //A419
typedef struct                                                      //A419
{                                                                   //A419
  UINT     ct;                       // number of tab stops         //A419
  int FAR *lpStops;                  // list of tabstops            //A419
} FPTABSTOPS, FAR * LPFPTABSTOPS;                                   //A419

typedef HANDLE FPTABS, FAR * LPFPTABS; // Handle for Tabstops //A419

#define NOFONT (-1)

BOOL   FPLIB fpGetFontTable (void);
BOOL   FPLIB fpGetFontTableEx (HINSTANCE hInst);
void   FPLIB fpReleaseFontTable (void);
void   FPLIB fpReleaseFontTableEx (HINSTANCE hInst);

FPFONT FPLIB fpGetFont (HFONT hFont);
FPFONT FPLIB fpGetFontEx (HINSTANCE hInst, HFONT hFont);
FPFONT FPLIB fpGetFontFromHDC (HFONT hFont, HDC hdc);
FPFONT FPLIB fpGetFontFromHDCEx (HINSTANCE hInst, HFONT hFont, HDC hdc);
FPFONT FPLIB fpGetLogFontPrim(LOGFONT FAR *lpLogFont, HDC hdc);
FPFONT FPLIB fpGetLogFontPrimEx(HINSTANCE hInst, LOGFONT FAR *lpLogFont, HDC hdc);
FPFONT FPLIB fp_GetLogFontPrim(LOGFONT FAR *lpLogFont, HDC hdc,
                                  HFONT FAR *lphFont);
FPFONT FPLIB fp_GetLogFontPrimEx(HINSTANCE hInst, LOGFONT FAR *lpLogFont, HDC hdc,
                                  HFONT FAR *lphFont);
FPFONT FPLIB fpGetLogFont (LOGFONT FAR *lpLogFont);
FPFONT FPLIB fpGetLogFontEx (HINSTANCE hInst, LOGFONT FAR *lpLogFont);
void   FPLIB fpReleaseFont (FPFONT fpFont);
void   FPLIB fpReleaseFontEx (HINSTANCE hInst, FPFONT fpFont);
void   FPLIB fpRefFont (FPFONT fpFont);
void   FPLIB fpRefFontEx (HINSTANCE hInst, FPFONT fpFont);
HFONT  FPLIB fpLockFont (FPFONT fpFont, LOGFONT FAR * lpLogFont);
HFONT  FPLIB fpLockFontEx (HINSTANCE hInst, FPFONT fpFont, LOGFONT FAR * lpLogFont);

BOOL   FPLIB fpGetFontLogFont (FPFONT fpFont, LOGFONT FAR *lpLogFont);
BOOL   FPLIB fpGetFontLogFontEx (HINSTANCE hInst, FPFONT fpFont, LOGFONT FAR *lpLogFont);
void   FPLIB fpGetHFontLogFont (HFONT hFont, LOGFONT FAR *lpLogFont);
HFONT  FPLIB fpGetFontHandle (FPFONT fpFont);
HFONT  FPLIB fpGetFontHandleEx (HINSTANCE hInst, FPFONT fpFont);

LPCHARWIDTHS FPLIB fpLockCharWidths (FPFONT fpFont);
LPCHARWIDTHS FPLIB fpLockCharWidthsEx (HINSTANCE hInst, FPFONT fpFont);
void   FPLIB fpUnlockCharWidths (FPFONT fpFont);
void   FPLIB fpUnlockCharWidthsEx (HINSTANCE hInst, FPFONT fpFont);

BOOL   FPLIB fpCreateTabHnd (LPFPTABSTOPS lpTabStops, LPFPTABS lpTabHnd); //A419
void   FPLIB fpReleaseTabHnd (FPTABS fpTabHnd);                 //A419
LPFPTABSTOPS FPLIB fpLockTabStops (FPTABS fpTabHnd);            //A419
void   FPLIB fpUnlockTabStops (FPTABS fpTabHnd);                //A419
void   FPLIB fpConvertTabsToPixels (FPTABS fpTabHnd, FPFONT fpFont); //A419

int    FPLIB fpGetTextWidth (FPFONT fpFont, LPTSTR lpsz, short width);
int    FPLIB fpGetTextWidthEx (HINSTANCE hInst, FPFONT fpFont, LPTSTR lpsz, short width);
int    FPLIB fpGetCharWidth (FPFONT fpFont, TCHAR ch);
int    FPLIB fpGetCharWidthEx (HINSTANCE hInst, FPFONT fpFont, TCHAR ch);
short  FPLIB fpDBCSGetCharWidth( HDC hDC, LPTSTR lpPtr, LPCHARWIDTHS lpCharWidth, long lLastChar);

int    FPLIB fpGetTabbedTextWidth (FPFONT fpFont, LPTSTR lpsz, short length, HWND hwnd, LPFPTABSTOPS lpTabstops); //A419
int    FPLIB fpGetTabbedTextWidthEx (HINSTANCE hInst, FPFONT fpFont, LPTSTR lpsz, short length, HWND hwnd, LPFPTABSTOPS lpTabstops); //A419
int    FPLIB fpGetTabbedTextWidthForDC (HDC hdc, LPTSTR lpsz, short length, LPFPTABSTOPS lpTabstops);  //A419

void   FPLIB fp3dTextOut (HDC hdc, int x, int y, UINT fuOpt, LPRECT lprc, LPCTSTR lpsz, UINT cbString, LPINT lpDx, LPTEXT3DINFO lp3d);
// Generic function which supports text with tab chars
void   FPLIB fpGen3dTextOut (HDC hdc, int x, int y, UINT fuOpt, LPRECT lprc, LPCTSTR lpsz, UINT cbString, LPINT lpDx, LPTEXT3DINFO lp3d, LPFPTABSTOPS lpTabstops, int tabOrigin); //A419

int    FPLIB fpGetFontHeight     (FPFONT fpFont);
int    FPLIB fpGetFontHeightEx   (HINSTANCE hInst, FPFONT fpFont);
int    FPLIB fpGetFontOverhang   (FPFONT fpFont);
int    FPLIB fpGetFontOverhangEx (HINSTANCE hInst, FPFONT fpFont);
int    FPLIB fpGetFontLineHeight (FPFONT fpFont);
int    FPLIB fpGetFontLineHeightEx (HINSTANCE hInst, FPFONT fpFont);
int    FPLIB fpGetFontAscent     (FPFONT fpFont);
int    FPLIB fpGetFontAscentEx   (HINSTANCE hInst, FPFONT fpFont);
int    FPLIB fpGetFontTextMetric (FPFONT fpFont, TEXTMETRIC FAR *lptm);
int    FPLIB fpGetFontTextMetricEx (HINSTANCE hInst, FPFONT fpFont, TEXTMETRIC FAR *lptm);

// Private information

#define GSHARE (GMEM_SHARE | GMEM_ZEROINIT | GMEM_MOVEABLE)

typedef struct fpFontEntry
{

   HFONT				hFont;
   LOGFONT				LogFont;
   TEXTMETRIC			tm;
   GLOBALHANDLE			hCharWidths;   
   LONG					lastUsed;
   UINT					useCount;

} FPFONTENTRY, FAR * LPFPFONTENTRY;

extern FPGLOBALDATAMANAGER dmFontTable;

#endif // NOFPFONT


#ifndef NOFPBUTTON
//
// FarPoint Button Utilities - FPTOOL17.C
//
//    fpHitText
//    fpButtonPaint
//    fpButtonRect
//    fpButtonWidth
//    fpButtonDown
//

// button flags
//
#define FPB_BUTTON        0x000F
#define FPB_SPIN          0x0003
#define FPB_POPUP         0x0001
#define FPB_DROPDOWN      0x0002
#define FPB_SLIDE         0x000C
#define FPB_LEFT          0x0004
#define FPB_RIGHT         0x0008
// Loyal Begin Changes
#define FPB_UP            0x0005
#define FPB_DOWN          0x0006
// Loyal End Changes

#define FPB_SPEED         0x0C00
#define FPB_SPEEDLEFT     0x0400
#define FPB_SPEEDRIGHT    0x0800

#define FPB_BORDER        0x00F0
#define FPB_BORDERLEFT    0x0010
#define FPB_BORDERRIGHT   0x0020
#define FPB_BORDERTOP     0x0040
#define FPB_BORDERBOTTOM  0x0080

#define FPB_HITUP         1
#define FPB_HITDOWN       2
#define FPB_HITLEFT       3
#define FPB_HITRIGHT      4

#define FPB_TIMEDELAY     50
#define FPB_INITIALDELAY  120


short FPLIB fpHitTest     (HWND hWnd, LPPOINT lpPoint, short style, BOOL fLeft, BOOL fSpin, short dButtonWidth, LPRECT r);
BOOL  FPLIB fpButtonPaint (HDC hDC, RECT rect, short style, BOOL fEnabled, COLORREF clrButton);

void  FPLIB fpDrawEllipseEx(HDC hDC, LPRECT lprc, BOOL fEnabled, COLORREF foreColor);
void  FPLIB fpDrawArrowEx(HDC hDC, LPPOINT lpPoints, short dHighlight, BOOL fEnabled, COLORREF foreColor);
void  FPLIB fpDrawDoubleArrowEx(HDC hDC, LPPOINT lpPoints, short dHighlight, BOOL fEnabled, COLORREF foreColor);

// use FPB_xxxx flags for styleArrow param to denote arrow type
short FPLIB fpSetAnyArrow(LPPOINT lpPoints, LPRECT lpRect, short styleArrow);

void  FPLIB fpButtonRect  (LPRECT buttonRect, HWND hWnd, BOOL fLeft, BOOL fSpin, short dButtonWidth);
short FPLIB fpButtonWidth (HWND hWnd, short dButtonWidth, BOOL fSpin, short dHeight);
void  FPLIB fpButtonDown  (HDC hDC, LPRECT lprc, short style, COLORREF clrButton);

#endif // NOFPBUTTON
                      
#ifndef FPNOTXTOUT
//void  FPLIB fpGradientFill(HDC hdc, RECT rect, HBRUSH hBrushTop, HBRUSH hBrushBottom, int direction);
//void FPLIB fpAlphaFill(HDC hdc, RECT rect, HBRUSH hBrush, COLORREF clrHighlight, BYTE alpha);

// FarPoint TextOut Functions - FPTOOL18
//
//    fpDrawText
//    fpWordWrap
//    fpProcessLine
//    fpDrawFocusRect
//    fpExtTextOut  
//    fpSelTextOut
//    fpInitTextOut
//    fpKillTextOut
//

// Special TextOut
typedef struct tagLineData
{

   LPTSTR lpsz;
   UINT  len;
   UINT  bc;
   long  w;

} FPLINEDATA, FAR * LPFPLINEDATA;

typedef struct tagDrawText
{
   // input
   UINT       fuLineSpacing;
   LPTSTR     lpLineData;
   UINT       fuStyle;
   TEXT3DINFO z3DInfo;
   // output
   UINT       cMnemonic;
   BYTE       bRetIsTextTooLong;       // This flag is returned and is true
                                       // if the text is too long to fit in
                                       // the given triangle
   LPCHARWIDTHS lpCharWidths;          // Optional, array of char widths for font SCP 12/31/98
	DWORD      dwFontLanguageInfo;
} FPDRAWTEXT, FAR *LPFPDRAWTEXT;

typedef struct tagWordWrap
{
   // input
   LPTSTR  lpszFirst;
   UINT   tmBreakChar;                              
   long   pageWidth;
   FPFONT fpFont;
   HWND   hWnd;                                             //A419 
   FPTABS tabHnd;                                           //A419
   // output
   LPTSTR lpszNext;
   UINT  dLength;
   UINT  dBreakCount;
   long  textWidth;
   UINT  fuMnemonic;
   long  dWidth;
   // internal state
   UINT  cb;
   LPCHARWIDTHS lpCharWidths; // Optional, array of char widths for font SCP 12/31/98
	DWORD dwFontLanguageInfo;
// enhancement SEL8276
	BOOL	fDrawEllipses;	// flag for drawing ellipses for single-line text too long to fit
    long    dEllipsesWidth; // width of ellipses in dc font
// enhancement SEL8276
} FPWORDWRAP, FAR *LPFPWORDWRAP;


#define DT_VJUSTIFY      0x2000
#define DT_JUSTIFY       0x4000

#define DTX_3D           0x0001
#define DTX_FOCUSRECT    0x0002
#define DTX_SAVEDATA     0x0004
#define DTX_GRAYTEXT     0x0008
#define DTX_ROTATE       0x0010
#define DTX_FLIP         0x0020
#define DTX_TRANSPARENT  0x0040
#define DTX_VERTICAL_RTL 0x0080
#define DTX_VERTICAL_LTR 0x0100
// enhancement SEL8276
#define DTX_ELLIPSES	 0x0200
// enhancement SEL8276

#define DTERR_NORECT     1
#define DTERR_MEMOUT     2

#define ETO_MNEMONIC     0x0100
#define ETO_MNSHOW       0x1000

// Function Prototypes
//
void  FPLIB fpInitTextOut (HWND hWnd);
void  FPLIB fpInitTextOutEx (HINSTANCE hInst, HWND hWnd);

void  FPLIB fpKillTextOut (HWND hWnd);
void  FPLIB fpKillTextOutEx (HINSTANCE hInst, HWND hWnd);

int   FPLIB fpDrawText (HWND hWnd, HDC hdc, LPTSTR lpsz, int cb, LPRECT lprc, UINT fuFormat, LPFPDRAWTEXT lpdt);
int   FPLIB fpDrawTextEx (HINSTANCE hInstance, HWND hWnd, HDC hdc, LPTSTR lpsz, int cb, LPRECT lprc, UINT fuFormat, LPFPDRAWTEXT lpdt);

LPTSTR FPLIB fpWordWrap (HDC hdc, LPFPWORDWRAP lpww, TEXTMETRIC FAR *lptm);

LPTSTR FPLIB fpProcessLine(HDC hdc, LPFPWORDWRAP lpww, BOOL fVertical,
                             UINT fuFormat, TEXTMETRIC FAR *lptm);

void  FPLIB fpExtTextOut (HDC hdc, int x, int y, UINT fuOptions, 
     LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx,
     LPTEXT3DINFO lp3d, LPFPDRAWTEXT lpdt);

// fix for JUS49 -scl
void  FPLIB fpExtTextOutEx (HINSTANCE hInst, HDC hdc, int x, int y, UINT fuOptions, 
     LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx,
     LPTEXT3DINFO lp3d, LPFPDRAWTEXT lpdt);
// fix for JUS49 -scl

void  FPLIB fpSelTextOut (HDC hdc, int x, int y, UINT fuOptions,
     LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx, 
     LPTEXT3DINFO lp3d, short nFirst, short nLast,
     COLORREF colorText, COLORREF colorBack);

void  FPLIB fpSelTabbedTextOut (HDC hdc, int x, int y, UINT fuOptions,
     LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx, 
     LPTEXT3DINFO lp3d, short nFirst, short nLast,
     COLORREF colorText, COLORREF colorBack);

//Generic (w/ & w/o tabs) version of the 2 functions above
void  FPLIB fpGenSelTextOut (HDC hdc, int x, int y, UINT fuOptions,
     LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx, 
     LPTEXT3DINFO lp3d, short nFirst, short nLast,
     COLORREF colorText, COLORREF colorBack, LPFPTABSTOPS lpTabstops);

void  FPLIB fpDrawFocusRect (HDC hdc, LPRECT lprc);

#endif // NOFPTXTOUT

#ifndef NOFPCALC
//
// FarPoint Calculation Utilities
//                  
//    fpAdjustRect      FPTOOL19
//    fpIsStyle         FPTOOL20
//    fpComputeHAlign   FPTOOL21 (unused)
//    fpComputeVAlign   FPTOOL22
//    
//

#define fpShrinkRect(lpRect, x, y) InflateRect(lpRect, -(x), -(y));

#define fpLM(margin) ((margin & 0xF000) >> 12)
#define fpRM(margin) ((margin & 0x0F00) >>  8)
#define fpTM(margin) ((margin & 0x00F0) >>  4)
#define fpBM(margin) (margin & 0x000F)

#define fpWIN_WIDTH(rect)  (int)((rect).right-(rect).left+1)
#define fpWIN_HEIGHT(rect) (int)((rect).bottom-(rect).top+1)
#define fpEXT_WIDTH(ext)   (int)(LOWORD(ext))
#define fpEXT_HEIGHT(ext)  (int)(HIWORD(ext))

#define fpLEFT_X(rc, txtWidth)                                      \
   (rc.left)

#define fpCENTER_X(rc, txtWidth)                                    \
   (rc.left + ((fpWIN_WIDTH(rc) - (int)txtWidth - 1)) / 2)

#define fpRIGHT_X(rc, txtWidth)                                     \
   (rc.left + (fpWIN_WIDTH(rc) - (int)txtWidth - 1))

#define fpTOP_Y(rc, txtHeight)                                      \
   (rc.top)                                                      
   
#define fpCENTER_Y(rc, txtHeight)                                   \
   (rc.top + (((fpWIN_HEIGHT(rc) - (int)txtHeight) / 2) - 1))

#define fpBOTTOM_Y(rc, txtHeight)                                   \
   (rc.top + ((fpWIN_HEIGHT(rc) - (int)txtHeight) - 1))

#define fpVISIBLE_LENGTH(p_dwLength, p_dwFirst, p_dwLast)           \
   (UINT)(p_dwLength ? p_dwLast-p_dwFirst+1 : 0L)

#define fpIS_EXTSTYLE(p_dwExtStyle, p_dwFlags)                      \
   (BOOL)((p_dwExtStyle & p_dwFlags) != 0L)
                              
#define fpIS_STATE(p_dwState, p_dwFlags)                            \
   (BOOL)((p_dwState & p_dwFlags)    != 0L)

#define fpIS_STYLE(p_hWnd, p_dwStyle)                               \
   (BOOL)(fpIsStyle (p_hWnd, p_dwStyle))


// function prototypes
//
void  FPLIB fpAdjustRect (UINT uMargin, LPRECT rect);
LPPOINT FPLIB fpMakePoint( LPPOINT lpPoint, DWORD dwXY);

LONG  FPLIB fpIsStyle (HWND hWnd, DWORD dwStyle);

// note: hardcoded style bits
//
short FPLIB fpComputeVAlign 
   (DWORD dwExtStyle, UINT yAvgFont, UINT uMargin, LPRECT r);


//
// Colors
//
// define for fpDrawTransparentBitmap to stop transparency 
#define NOT_TRANSPARENT      (COLORREF)-1
#ifndef RGBCOLOR_DEFAULT
#define RGBCOLOR_DEFAULT     (0x80000000L | COLOR_INACTIVEBORDER)
#endif
#define RGBCOLOR_WHITE       RGB(255, 255, 255)
#define RGBCOLOR_BLUE        RGB(0, 0, 255)
#define RGBCOLOR_RED         RGB(255, 0, 0)
#define RGBCOLOR_PINK        RGB(255, 0, 255)
#define RGBCOLOR_GREEN       RGB(0, 255, 0)
#define RGBCOLOR_CYAN        RGB(0, 255, 255)
#define RGBCOLOR_YELLOW      RGB(255, 255, 0)
#define RGBCOLOR_BLACK       RGB(0, 0, 0)
#define RGBCOLOR_DARKGRAY    RGB(128, 128, 128)
#define RGBCOLOR_DARKBLUE    RGB(0, 0, 128)
#define RGBCOLOR_DARKRED     RGB(128, 0, 0)
#define RGBCOLOR_DARKPINK    RGB(255, 0, 128)
#define RGBCOLOR_DARKGREEN   RGB(0, 128, 0)
#define RGBCOLOR_DARKCYAN    RGB(0, 128, 128)
#define RGBCOLOR_BROWN       RGB(128, 128, 64)
#define RGBCOLOR_PALEGRAY    RGB(192, 192, 192)

#define RGBCOLOR_PURPLE      RGB(192, 0, 192)
#define RGBCOLOR_DARKPURPLE  RGB(128, 0, 128)
#define RGBCOLOR_DARKYELLOW  RGB(128, 128, 0)

#define FPCOLOR(c)         ((c &  0x80000000) ? GetSysColor((int)(short)c) : c)
//#define FPCOLOR(c)         ((c != RGBCOLOR_DEFAULT && (c &  0x80000000)) ? GetSysColor((int)(short)c) : c)
#define COLOR_FIXUP(c)     (c == 0x80000014 ? GetSysColor((int)(short)c) : c)
#define COLOR_INIT(c)      (0x80000000 | c)

#endif // NOFPCALC

//#ifndef NOFPPROP
// Property Management Utilities
//

//#define FPDT_BOOL               0
//#define FPDT_SHORT      1
//#define FPDT_LONG               2
//#define FPDT_COLOR      3
//#define FPDT_STRING     4
//#define FPDT_XPOS               5
//#define FPDT_XSIZE      6
//#define FPDT_YPOS               7
//#define FPDT_YSIZE      8
//#define FPDT_PICT               9

//typedef LPSTR FPSTRINGHANDLE;
//LPSTR   fpLockString(FPSTRINGHANDLE);
//VOID  fpUnlockString(FPSTRINGHANDLE);

//typedef struct fppropparams
//{
//   HWND     hWnd;
//       HCTL         hCtl;
//   WORD         nDataType;
//   LPVOID       lpData;
//   WORD         nParams;
//   LPLONG       lpParams;                 
//} FPPROPPARAMS, FAR* LPFPPROPPARAMS;

//typedef long FPPROPHANDLER(LPFPPROPPARAMS,LPVOID,BOOL);

//typedef struct fpproptableentry
//{
//    short   nPropType;
//    FPPROPHANDLER   *lpfnHandler;
//    BYTE    nParams;
//} FPPROPTABLEENTRY, FAR *LPFPPROPTABLEENTRY; 

//typedef struct fpproptable
//{
//    short           nPropCount;
//    FPPROPTABLEENTRY entry[];
//} FPPROPTABLE, FAR* LPFPPROPTABLE;

//#endif

#ifndef NOFPVBTOOLS
//
// VBX Building Utilities
//
//    vbProcessTextExt
//

#define VB1 "10"         // append to class names for C++ environments
#define VB3 ""           // append to class names for VB environments

typedef struct vbpropparams
   {
   HCTL       hCtl;
   HWND       hWnd;
   WORD       Msg;
   WPARAM     wParam;
   LPARAM     lParam;
   } VBPROPPARAMS, FAR *LPVBPROPPARAMS;

typedef struct tagPARAMS1
{
   LPVOID lpParam1;
   LPVOID Index;
} PARAMS1, FAR * LPPARAMS1;

typedef struct tagPARAMS2
{
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
} PARAMS2, FAR * LPPARAMS2;

typedef struct tagPARAMS3
{
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
} PARAMS3, FAR * LPPARAMS3;

typedef struct tagPARAMS4
{
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
} PARAMS4, FAR * LPPARAMS4;

typedef struct tagPARAMS6
{
   LPVOID lpParam6;
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
} PARAMS6, FAR * LPPARAMS6;    

typedef struct tagPARAMS7
{
   LPVOID lpParam7;
   LPVOID lpParam6;
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
} PARAMS7, FAR * LPPARAMS7;    

typedef struct tagPARAMS8
{
   LPVOID lpParam8;
   LPVOID lpParam7;
   LPVOID lpParam6;
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
} PARAMS8, FAR * LPPARAMS8;    

typedef struct tagPARAMS9
{
   LPVOID lpParam9;
   LPVOID lpParam8;
   LPVOID lpParam7;
   LPVOID lpParam6;
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
} PARAMS9, FAR * LPPARAMS9;    

extern EVENTINFO EventAdvance;
extern EVENTINFO EventChange;
extern EVENTINFO EventClick;
extern EVENTINFO EventDoubleClick;
extern EVENTINFO EventButtonHit;
extern EVENTINFO EventInvalidData;
extern EVENTINFO EventInvalidAction;
extern EVENTINFO EventPopUp;
extern EVENTINFO EventKeyDown;                                       //A521
extern EVENTINFO EventKeyPress;  
extern EVENTINFO EventPopUpClose;
                                     //A521

extern PROPINFO  fpProphWnd;
extern PROPINFO  fpPropRedraw;
extern PROPINFO  PropertyNULL;  
extern PROPINFO  fpPropControlType;
extern PROPINFO  fpPropMMControlType;
enum 
   {
   VB_EVENT_CHANGE,        // 0
   VB_EVENT_CHANGEMODE,    // 1
   VB_EVENT_INVALIDACTION, // 2
   VB_EVENT_INVALIDDATA,   // 3
   VB_EVENT_ADVANCE,       // 4
   VB_EVENT_CLICK,         // 5
   VB_EVENT_DOUBLECLICK,   // 6
   VB_EVENT_BUTTONHIT,     // 7
   VB_EVENT_KEYPRESS,      // 8
   VB_EVENT_KEYDOWN,       // 9

   VB_EVENT_COUNT
   };

long FPLIB vbProcessTextExt
   (LPVBPROPPARAMS lpPP, BOOL fGetProp, WORD MsgGet, WORD MsgGetLen, WORD MsgSet, BOOL fLenMsg);

LONG FPLIB vbGenericProc (HCTL hCtl, HWND hWnd, WORD Msg, LPWORD lpwParam, 
    LPLONG lplParam, LONG (*lpCallBack[])(), LONG (*lpDataAware)(), 
    short EditEvent[], BOOL FAR *lpfCallDef ); 

LONG FPLIB vbBasicProc (HCTL hCtl, HWND hWnd, WORD Msg, WORD wParam, 
    LONG lParam, LONG (*lpCallBack[])(), LONG (*lpDataAware)(), 
    BOOL FAR *lpfCallDef ); 

long FPLIB FP_Redraw (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FP_hWnd   (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_ControlType (LPVBPROPPARAMS lpPP, BOOL fGetProp);

COLORREF fpGetVBColor(LONG color);

/*****************
* DataSourcehWnd
*****************/

#define FP_DATASOURCEHWNDPROP _T("hWndDataSource")

extern PROPINFO fpPropDataSourcehWnd;

extern long fpVB_DataSourcehWnd(LPVBPROPPARAMS lpPP, BOOL fGetProp);
extern void fpVB_DestroyDataSourcehWnd(HWND hWnd);

#endif // NOFPVBTOOLS

#ifndef NOFPINTL

void   FPLIB fpResetIntlDate();
void   FPLIB fpResetIntlNumeric();
LPTSTR FPLIB fpGetIntl_sCurrency();
int    FPLIB fpGetIntl_iCurrency();
int    FPLIB fpGetIntl_iCurrDigits();
int    FPLIB fpGetIntl_iNegCurr();

LPTSTR FPLIB fpGetIntl_sThousand();
LPTSTR FPLIB fpGetIntl_sDecimal();
int    FPLIB fpGetIntl_iDigits();
int    FPLIB fpGetIntl_iLZero();

LPTSTR FPLIB fpGetIntl_sShortDate();
LPTSTR FPLIB fpGetIntl_sLongDate();
LPTSTR FPLIB fpGetIntl_sDate();
int    FPLIB fpGetIntl_iTime();
LPTSTR FPLIB fpGetIntl_sTime();
LPTSTR FPLIB fpGetIntl_s1159();
LPTSTR FPLIB fpGetIntl_s2359();
int    FPLIB fpGetIntl_iTLZero();
LPTSTR FPLIB fpGetIntl_sTimeFormat();
#if (WINVER >= 0x0400)  //WIN95 or NT
LPTSTR FPLIB fpGetIntl_sMonThousandSep();
LPTSTR FPLIB fpGetIntl_sMonDecimalSep();
#endif //WINVER
#endif  // NOFPINTL

#ifndef NOFPFILE

// file functions
int      DLLENTRY ErrorNo           (void);
BOOL     DLLENTRY FileClose         (int File);
BOOL     DLLENTRY FileExists        (LPSTR);
BOOL     DLLENTRY FileGetc          (LPSTR, int);
BOOL     DLLENTRY FileGetw          (LPINT, int);
BOOL     DLLENTRY FileOpen          (LPSTR Name, int OFlag, int PMode, LPINT File);
#if defined(_WIN64) || defined(_IA64)
WORD     DLLENTRY fpFileRead        (LPVOID lpbuff, WORD wItemSize, WORD wItemCount, int iFH);
#else
WORD     DLLENTRY FileRead          (LPVOID lpbuff, WORD wItemSize, WORD wItemCount, int iFH);
#endif
BOOL     DLLENTRY FileSeek          (int, LONG, WORD);

#endif  // NOFPFILE

#ifndef NOFPHELP

BOOL FPLIB fpWinHelp (LPTSTR lpszHelpFile, HWND hWnd, WORD wParam, PPROPINFO Properties[], PEVENTINFO Events[], int fLastVB, long lOffset);

#endif // NOFPHELP

#ifndef NOFPABOUT

extern PROPINFO  fpPropAbout;


#ifndef NOFPVBTOOLS
   long FP_About (LPVBPROPPARAMS lpPP, BOOL fGetProp);
#endif

short fpVersion ();

#define FPM_SHOWWINDOW 9999
#define IDBMP_FARPOINT 999

#define CLASS_FPABOUT _T("fpAbout")

#define IDC_CTLNAME 100
#define IDC_BITMAP  101

#define IDS_MODEL_VERSION 0x8001
#define IDS_ABOUT_VERSION 0x8002

typedef struct tagAboutBox
{
                       
   HINSTANCE hInstance;
   LPSTR     lpszCtlName;
   LPSTR     lpszRcName;
   COLORREF  dlgColor;
   COLORREF  backColor;
   COLORREF  textColor;
   int       bmpId;

} FPABOUT, FAR * LPFPABOUT;

typedef struct tagAboutPurchaseInfo
{
  char serialNumber[64];  
  char productKey[64];
  DWORD productID;
} PURCHASEINFO, FAR * LPPURCHASEINFO;

void fpRegisterAboutProc(HINSTANCE hInstance);
//long fpInitAboutPopUp 
//   (HWND hWnd, LPSTR lpszName, LPSTR lpszDlgRC, int idBmp);
#ifndef NO_MFC
long fpInitAboutPopUp(HWND hWnd, LPPURCHASEINFO lpPurchase);
#else
long fpInitAboutPopUp(LPFPCONTROL lpObject, LPPURCHASEINFO lpPurchase);
#endif

#endif // NOFPABOUT                    

#ifndef NOFPTIME

typedef DWORD FPTIME, FAR *LPFPTIME;

// time is expressed as a long integer representing milliseconds
//
#define fpTIME(hr, mn, sc, hs) ((long)hr*3600000L+(long)mn*60000L+(long)sc*1000L+(long)hs)
#define fpHR(tm)               (((double)(tm))/(double)3600000.0)
#define fpMN(tm)               (((double)(tm))/(double)60000.0)
#define fpSC(tm)               (((double)(tm))/(double)1000.0)
#define fpHS(tm)               (((double)(tm))/(double)10.0)  
#define fpMS(tm)               (((double)(tm)))                

#define fpHRTOMS(hr)           ((long)((hr)*3600000L))
#define fpMNTOMS(mn)           ((long)((mn)*60000L))
#define fpSCTOMS(sc)           ((long)((sc)*1000L))
#define fpHSTOMS(hs)           ((long)((hs)*10L))
  
#define fpHOUR(fpTime)         (int)(fpHR(fpTime))
#define fpMIN(fpTime)          (int)(fpMN(fpTime-fpHRTOMS((long)fpHR(fpTime))))
#define fpSEC(fpTime)          (int)(fpSC(fpTime-fpMNTOMS((long)fpMN(fpTime))))

#define fpNOTIME               ((DWORD)-1)

#define fpMAXTIME   fpHRTOMS(100)
#define fpMAXHR     (1000)
#define fpMAXMN     (59)
#define fpMAXSC     (59)
#define fpMAXHS     (99)

#define TSC_HHMMSS   0
#define TSC_HOURS    1
#define TSC_MINUTES  2
#define TSC_SECONDS  3
#define TSC_MSECS    4
#define TSC_INTERNAL 5

#define IsNum(c)    (c >= (TCHAR)'0' && c <= (TCHAR)'9')
#define NotNum(c)   (c < (TCHAR)'0' || c > (TCHAR)'9')
#define IsTime(tm)  (tm != fpNOTIME ? TRUE : FALSE);
// #define IsTime(tm) (tm != fpNOTIME);
// #define NotTime(tm) (tm == fpNOTIME);

FPTIME FPLIB fpStringToTime (LPTSTR lpsz, UINT fScale);
LPTSTR FPLIB fpTimeToString (FPTIME fpTime, UINT fScale, LPTSTR lpsz);
FPTIME FPLIB fpGetTime ();
BOOL   FPLIB fpTimeInRange (FPTIME fpTime, FPTIME fpMin, FPTIME fpMax);

#endif // NOFPTIME

#ifndef NOFPDATE

extern short dDaysInMonth[12]; 
extern short dJulianDays[13];

#define ISLEAP(y) (y%4 == 0 && y%100 != 0 || y%400 == 0)
#define DAYSINMONTH(y, m) (dDaysInMonth[m-1] + ((m == 2 && ISLEAP(y)) ? 1 : 0))
typedef long FPDATE, FAR *LPFPDATE;

typedef struct tagDateParts
{

   char  day;
   char  month;
   short year;

} FPDATEPARTS, FAR * LPFPDATEPARTS;

#define fpYEAR(fpDate)  ((LPFPDATEPARTS)&fpDate)->year
#define fpMONTH(fpDate) ((LPFPDATEPARTS)&fpDate)->month
#define fpDAY(fpDate)   ((LPFPDATEPARTS)&fpDate)->day
#define fpISLEAP(fpDate) ISLEAP(fpYEAR(fpDate))
#define fpDAYSINMONTH(fpDate) \
   DAYSINMONTH(fpYEAR(fpDate), fpMONTH(fpDate))
#define fpSET_DATE(fpDate, year, month, day) \
   fpYEAR(fpDate) = (short)year, fpMONTH(fpDate) = (char)month, fpDAY(fpDate) = (char)day
#define fpNODATE 0L

#define DATEF_YYYYMMDD 0
#define DATEF_MMDDYYYY 1

LONG   FPLIB fpDateToJulian (FPDATE fpDate);
FPDATE FPLIB fpJulianToDate( long julian, int startingFromYear);
short  FPLIB fpDateWeekDay  (FPDATE fpDate);
FPDATE FPLIB fpTodaysDate   ();
BOOL   FPLIB fpDateInRange  (FPDATE fpDate, FPDATE fpMin, FPDATE fpMax);
FPDATE FPLIB fpStringToDate (LPTSTR lpsz, WORD fFormat);
BOOL   FPLIB fpDateIsValid  (FPDATE fpDate);
BOOL   FPLIB fpYMDIsValid   (int y, int m, int d);
LPTSTR FPLIB fpDateToString (FPDATE fpDate, LPTSTR lpsz, WORD fFormat);

#endif // NOFPDATE

#ifndef NOFPDT

typedef struct tagDateTime
{
   FPDATE fpDate;
   FPTIME fpTime;
} FPDATETIME, FAR * LPFPDATETIME;

BOOL FPLIB DBdatetimeToFPdatetime (long lObject, LPDATAACCESS lpda, LPFPDATE lpfpDate, LPFPTIME lpfpTime);
void FPLIB FPdatetimeToDBdatetime (long lObject, LPDATAACCESS lpda, FPDATE fpDate, FPTIME lpTime);
void FPLIB FPdatetimeToDBtext (HCTL hCtl, LPDATAACCESS lpda, FPDATE fpDate, FPTIME lpTime, BOOL bSaveBoth);

void FPLIB FPdatetimeToDouble (FPDATE fpDate, FPTIME fpTime, double FAR *dfVal);
BOOL FPLIB DoubleToFPdatetime (long lObject, double dfVal, LPFPDATE lpfpDate, LPFPTIME lpfpTime);
void FPLIB StringToFPdatetime (LPTSTR lpsz, LPFPDATE lpfpDate, LPFPTIME lpfpTime);
void FPLIB FPdatetimeToString (FPDATE fpDate, FPTIME fpTime, LPTSTR buffer, WORD bufferSize);

#endif // NOFPDT


#ifndef NOFPGRAPHC

typedef unsigned char fpGraphicType;

#define fpBITMAP    0
#define fpICON      1
#define fpMETAFILE  2

extern void FPLIB fpDrawIcon(HDC hDC, HICON hIcon, LPRECT lpRect,
                              short x, short y, COLORREF Color);

extern void FPLIB fpDrawBitmap(HDC hDC, HBITMAP hBitmap, LPRECT lpRect,
                                short x, short y);

extern HBITMAP FPLIB fpCreateStretchedIcon(HDC hDC, LPRECT lpRect, COLORREF Color,
                                            HICON hIcon, BOOL fProportionally);

extern HBITMAP FPLIB fpCreateStretchedBitmap(HDC hDC, HBITMAP hBitmap, LPRECT lpRect,
                                              BOOL fProportionally, HPALETTE hPal);

extern WORD FPLIB fpSetDIBitsToDevice(HDC hDC, HANDLE hDib, LPRECT lpRect,
                                       short x, short y);

extern HDC FPLIB fpGetPrinterDC(void);

HBITMAP FPLIB BitmapFromDib(HANDLE hdib, HPALETTE hpal, BOOL fPaletteIncluded);

HANDLE FPLIB OpenDIB(LPSTR szFile, HPSTR BmpResourceData, long BmpResourceDataSize);

GLOBALHANDLE FPLIB ReadDibBitmapInfo(int fh, HPSTR BmpResourceData, long BmpResourceDataSize,
                                      long FAR *BmpResourceDataAt);

WORD FPLIB PaletteSize(VOID FAR *pv);

WORD FPLIB DibNumColors(VOID FAR *pv);

BOOL FPLIB DibInfo(HANDLE hbi, LPBITMAPINFOHEADER lpbi);

DWORD PASCAL DibLRead(int hFile, VOID FAR *pBuffer, DWORD Len, HPSTR BmpResourceData, 
   long BmpResourceDataSize, long FAR *BmpResourceDataAt);

HPALETTE FPLIB CreateDibPalette(HANDLE hbi);

HPALETTE FPLIB CreateBIPalette(LPBITMAPINFOHEADER lpbi);

HANDLE FPLIB DibFromBitmap(HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal, LPDWORD lpdwLen,
   LPSHORT lpdPaletteSize);

HANDLE FPLIB BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal);

BOOL FPLIB StretchDibBlt(HDC hDC, int x, int y, int dx, int dy, HANDLE hDib,
                          int x0, int y0, int dx0, int dy0, LONG Rop);

HPALETTE FPLIB GetSystemPalette(void);

GLOBALHANDLE FPLIB SaveDIBToBuffer(GLOBALHANDLE hdib, LPLONG lplBufferSize);

HBITMAP FPLIB fpIconToBitmap(HWND hWnd, HICON hIcon, COLORREF Color);

HBITMAP FPLIB fpMetaFileToBitmap(HWND hWnd, HANDLE hMeta, COLORREF Color);

void FPLIB fpDrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart,
                                    short yStart, COLORREF cTransparentColor, DWORD dwPaintFlag, HPALETTE hPalette);

void FPLIB fpDrawTransparentBitmapEx(HDC hdc, HBITMAP hBitmap, short xStart, short yStart, 
                                    short nWidth, short nHeight, COLORREF cTransparentColor, 
                                    DWORD dwPaintFlag, HPALETTE hPalette);

void FPLIB fpRenderMetaFile(HDC hDC, HMETAFILE hMeta, LPRECT lpRect, short x, short y, COLORREF Color);
                                

#endif // NOFPGRAPHC

#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#ifndef NOFPVBX
//
// Implement VBINITCC as follows:
//
// BOOL FPLIB VBINITCC(USHORT usVersion, BOOL fRuntime)
// { 
//    if (!fRuntime)
//       {
//       if (!fpCheckRuntimeLicense("FPTEXT.VBX", "FPTEXT.LIC"))
//          return (FALSE);
//       }
//    return (fpRegisterLibClass(fpInstance));
// }
//
extern HANDLE fpInstance;    
extern PROPINFO PropertyNULL; 
      
typedef char huge * HPSTR;

BOOL FPLIB fpCheckRuntimeLicense (LPTSTR lpszVBX, LPTSTR lpszLIC,
   DWORD dwProductID, FPDATE fpTimeOut, BOOL bCreateLicense);
void FPLIB fpDisplayRuntimeLicenseMsg (LPTSTR lpszVBX);

#else

extern HANDLE fpInstance;

#endif // NOFPVBX          

#ifdef _UNICODE
#define fpDBCSGetChar(lpPtr) (*(unsigned short FAR *)lpPtr)
#define STRING_SIZE(x) (sizeof(x)/sizeof(TCHAR)) // size of buffer in UNICODE chars

#else // not UNICODE

#define fpDBCSGetChar(lpPtr) (unsigned short)(IsDBCSLeadByte(*lpPtr) ? (((unsigned short)*(lpPtr) << 8) | (unsigned short)lpPtr[1]) : (unsigned short)*lpPtr)
#define STRING_SIZE(x) sizeof(x)
#ifndef CharNext  // if not defined, define it as AnsiNext
#define CharNext AnsiNext
#define CharPrev AnsiPrev
#endif

//#define fpDBCSGetCharWidth(hDC, lpPtr, lpCharWidth) (short)(IsDBCSLeadByte(*lpPtr) ? (GetTextExtent(hDC, lpPtr, 2)) : lpCharWidth[(unsigned char)*lpPtr])
#endif


// Returns FALSE if date is no longer valid (ie. product has expired!).
BOOL FPLIB fpValidateExpiration( FPDATE fpDate, LPTSTR lpszProductName, 
                                    LPTSTR lpszMessage);

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

/************************************************************************
* Structure for getting HUGE Text strings (HPSTR)
************************************************************************/
typedef struct fp_hptext_info
{
  HPSTR   hpsz;
  DWORD   maxlen;
} FP_HPTEXT_INFO, FAR *LPFP_HPTEXT_INFO;



/************************************************************************
* Structure for getting Text strings (LPSTR)
************************************************************************/
typedef struct fp_text_info
{
  LPTSTR  lpsz;
  UINT    maxlen;
} FP_TEXT_INFO, FAR *LPFP_TEXT_INFO;

/***********************************************
* Structure for getting & setting the Picture
**********************************************/
typedef struct fp_pict_info
{
  short   pictType;
#define FP_PICT_TYPE_NONE                 0
#define FP_PICT_TYPE_BITMAP               1
#define FP_PICT_TYPE_METAFILE             2 
#define FP_PICT_TYPE_ICON                 3

  short   hndType;
#define FP_HNDTYPE_HANDLE                 0 /* for HBITMAP, HICON, HMETAFILE */
#define FP_HNDTYPE_RESID                  1
#define FP_HNDTYPE_RESNAME                2

  HPALETTE hPalette;

  union
  {
    HANDLE         handle;                  /* HBITMAP, HICON, or HMETAFILE  */
    WORD           resID;
    FP_TEXT_INFO   resName;                 /* When GETTING property,        */
  } u;                                      /* ptr to buffer for name.       */
  
  int xExt;                                 /* horiz. metafile extent        */
  int yExt;                                 /* vert. metafile extent         */

} FP_PICT_INFO, FAR *LPFP_PICT_INFO;

typedef struct tagFP_PICT
   {
   BYTE     bType;
   short    nWidth;
   short    nHeight;
   HANDLE   hPict;
   HPALETTE hPal;
   BOOL     fDeletePict;
   HBITMAP  hPictStretch;
   short    nPictStretchWidth;
   short    nPictStretchHeight;
#if (defined(FP_VB) || defined(FP_OCX)) && (!defined(FP_BTNDES) || defined(BM20) || defined(FP_VB)) // for 16-bit OCX button designer -scl
   HANDLE     hVBPic;
#endif
   } FP_PICT, FAR *LPFP_PICT;

typedef WORD FPPICTID, FAR *LPFPPICTID;

BOOL FPLIB      fpPM_Init(HANDLE hInst);
void FPLIB      fpPM_Free(HANDLE hInst);
BOOL FPLIB      fpPM_SetProp(HANDLE hInst, HWND hWnd, BYTE bCtlType, LPFPPICTID lpidPict,
                                long lParam);
HANDLE FPLIB    fpPM_GetProp(HANDLE hInst, BYTE bCtlType, FPPICTID idPict, long lParam);
void FPLIB      fpPM_FreeItem(HANDLE hInst, FPPICTID idPict);
void FPLIB      fpPM_RefItem(HANDLE hInst, FPPICTID idPict);
BOOL FPLIB      fpPM_GetItem(HANDLE hInst, FPPICTID idPict, LPFP_PICT lpPict);
LPFP_PICT FPLIB fpPM_LockItem(HANDLE hInst, FPPICTID idPict);
#define            fpPM_UnlockItem(hInst, idPict) TRUE
FPPICTID FPLIB  fpPM_Load(HANDLE hInst, HPBYTE lpMem, long lMemLen, BYTE bCtlType);
HGLOBAL FPLIB   fpPM_Save(HANDLE hInst, FPPICTID idPict, LPLONG lplSize);
UINT FPLIB      fpPM_GetCount(HANDLE hInst);
long FPLIB      fpPM_GetRefCount(HANDLE hInst, FPPICTID idPict);
void FPLIB      fpPM_SetRefCount(HANDLE hInst, FPPICTID idPict, long lRefCnt);
void FPLIB      fpPM_UnrefItem(HANDLE hInst, FPPICTID idPict);
                                
// Picture Manager utility functions                                

BOOL     FPLIB fpPM_IsPicture(HANDLE hInst, FPPICTID idPict);
BYTE     FPLIB fpPM_GetPictType(HANDLE hInst, FPPICTID idPict);
HPALETTE FPLIB fpPM_GetPalette(HANDLE hInst, FPPICTID idPict);
long     FPLIB fpPM_GetHeight(HANDLE hInst, FPPICTID idPict);
long     FPLIB fpPM_GetWidth(HANDLE hInst, FPPICTID idPict);
HBITMAP  FPLIB fpPM_GetStretchBitmap(HANDLE hInst, FPPICTID idPict);
long     FPLIB fpPM_GetStretchHeight(HANDLE hInst, FPPICTID idPict);
long     FPLIB fpPM_GetStretchWidth(HANDLE hInst, FPPICTID idPict);
void     FPLIB fpPM_SetPalette(HANDLE hInst, FPPICTID idPict, HPALETTE hPal);
void     FPLIB fpPM_SetHeight(HANDLE hInst, FPPICTID idPict, long lHeight);
void     FPLIB fpPM_SetWidth(HANDLE hInst, FPPICTID idPict, long lWidth);

// MouseWheel
BOOL FPLIB fpMouseWheel(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam,
                         HWND hWndVScroll, HWND hWndHScroll);
BOOL FPLIB fpMouseWheelEx(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam,
                          HWND hWndVScroll, HWND hWndHScroll, int iLinesToScroll);
#ifndef _WIN32	// makes changes to mousewheel code compile in 16-bit -scl
#define SBM_GETPOS                  0x00E1 /*not in win3.1 */
#define SBM_GETRANGE                0x00E3 /*not in win3.1 */
#endif

HBITMAP  FPLIB fpRotateBitmap(HDC hDCDev, HBITMAP hBitmap, HPALETTE hPal);

/************************************************************************
* Structure for getting/setting properties which have an Index
************************************************************************/
typedef struct fp_indexprop_info
{
  short    index;
  union
  {
    FP_HPTEXT_INFO htValue;
    FP_TEXT_INFO   tValue;
    short    FAR  *lpnValue;
    long     FAR  *lplValue;
    BOOL     FAR  *lpbValue;
    float    FAR  *lpfValue;
    COLORREF FAR  *lpcValue;
    FP_PICT_INFO   picValue;
  } u;

} FP_INDEXPROP_INFO, FAR *LPFP_INDEXPROP_INFO;

/************************************************************************
* Structure for getting/setting properties which have an Index and Text
************************************************************************/
typedef struct fp_indextext_info
{
  int     index;
  HPSTR   lpsz;
  DWORD    maxlen;
} FP_INDEXTEXT_INFO, FAR *LPFP_INDEXTEXT_INFO;

/************************************************************************
* Structure for getting length of properties which have an Index and Text
************************************************************************/
typedef struct fp_indextextlen_info
{
  int        index;
  short FAR *lpLength;
} FP_INDEXTEXTLEN_INFO, FAR *LPFP_INDEXTEXTLEN_INFO;

#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#define FP_GETPROP_OK          0            /* good return code              */
#define FP_GETPROP_FAILED     -1            /* bad return code               */

#define FP_SETPROP_OK          0            /* good return code              */
#define FP_SETPROP_FAILED     -1            /* bad return code               */

#define ERR_INVALIDBUFFER     -2
#define ERR_INVALIDINDEX      -3

#define FP_CMDPARAM_TYPE_NONE     0
#define FP_CMDPARAM_TYPE_PARAM1   1
#define FP_CMDPARAM_TYPE_PARAM2   2
#define FP_CMDPARAM_TYPE_PARAM3   3
#define FP_CMDPARAM_TYPE_PARAM4   4


/**************************************************************************/
/* WIN16/WIN32-Independent MACROS                                         */
/**************************************************************************/
#ifdef WIN32

#ifdef _WIN64
#define GETGW_HINSTANCE(hWnd)   ((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE))
#else
#define GETGW_HINSTANCE(hWnd)   ((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE))
#endif
#define GETGW_ID(hWnd)          GetWindowLong(hWnd, GWL_ID)
#define GETGW_USERDATA(hWnd)    GetWindowLong(hWnd, GWL_USERDATA)
#define GETGC_CURSOR(hWnd)      GetClassLong(hWnd, GCL_HCURSOR)
#define SETGC_CURSOR(hWnd, newVal)      SetClassLong(hWnd, GCL_HCURSOR, (LONG)newVal)
#define SETGW_ID(hWnd, Value)   SetWindowLong(hWnd, GWL_ID, Value)

#else  //not WIN32

#define GETGW_HINSTANCE(hWnd)   ((HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE))
#define GETGW_ID(hWnd)          GetWindowWord(hWnd, GWW_ID)
#define GETGW_USERDATA(hWnd)    GetWindowWord(hWnd, GWW_USERDATA)
#define GETGC_CURSOR(hWnd)      GetClassWord(hWnd, GCW_HCURSOR)
#define SETGC_CURSOR(hWnd, newVal)      SetClassWord(hWnd, GCW_HCURSOR, (WORD)newVal)
#define SETGW_ID(hWnd, Value)   SetWindowWord(hWnd, GWW_ID, Value)

#endif //not WIN32

// workaround for bug in strtod() in MSVCRT.DLL
double FPLIB fpstrtod(LPCTSTR str, char FAR** ppchar, const char decimal);

#define RC_WIDTH(lpRect) ((LPRECT)lpRect)->right - ((LPRECT)lpRect)->left
#define RC_HEIGHT(lpRect) ((LPRECT)lpRect)->bottom - ((LPRECT)lpRect)->top

#ifdef __cplusplus
}
#endif    

#endif // FPTOOLS_H  
