// 
// Use to include Aware/VBX (1.0) controls in the Utopia version of 
// Spread/VBX (2.1).  This header will include AWARE.H and FPTOOLS.H.
// Both must be on the INCLUDE path or this file must be modified.
// This header also resolves conflicts between FPTOOLS.H and TOOLBOX.H
// and defines prototypes for functions not found in FPTOOLS.H or
// AWARE.H.
//
#ifndef UTOOLS_H
#define UTOOLS_H

// 
// Avoid warning messages generated because the macros are defined
// in both FPTOOLS.H and TOOLBOX.H.  
//
#undef  RGBCOLOR_DEFAULT
#undef  EN_INVALIDCHAR

//
// Avoid multiple include of VBAPI.H
//
#define VBAPI_H 1

#undef  RGBCOLOR_DEFAULT
#undef  EN_INVALIDCHAR

//
// use TOOLBOX.H definition
//
#define RGBCOLOR_DEFAULT (COLORREF)-1L 

//
// Aware/VBX "C" window procedures
//
// wrtmsg.c
extern WINENTRY wrTextWndProc
   (HWND hWnd, unsigned uMessage, WORD wParam, LONG lParam);
// wrmmsg.c
extern WINENTRY wrMaskWndProc
   (HWND hWnd, unsigned uMessage, WORD wParam, LONG lParam);
// wrdmsg.c
extern WINENTRY wrDoubleWndProc
   (HWND hWnd, unsigned uMessage, WORD wParam, LONG lParam);
// wrymsg.c
extern WINENTRY wrDateWndProc
   (HWND hWnd, unsigned uMessage, WORD wParam, LONG lParam);
//
// Aware/VBX DateTime Utilities
//
// wrytext.c
BOOL wryStringToDateTime (LPFPDATE lpfpDate, LPFPTIME lpfpTime,
   LPSTR lpNewData, LPSTR lpszOrder, DWORD dwStyle, 
   FPDATE fpDefDate, FPDATE fpMinDate, FPDATE fpMaxDate,
   FPTIME fpDefTime, FPTIME fpMinTime, FPTIME fpMaxTime,
   BOOL f24Hour, LPSTR lpszAM, LPSTR lpszPM, int fFlags);
// wryintl.c
void DLLENTRY wryDateWinFormat (LPSTR lpszText, FPDATE fpDate, FPTIME fpTime, 
   LPSTR lpszDateFormat, BOOL f24Hour, LPSTR lpszAM, LPSTR lpszPM);

#endif

