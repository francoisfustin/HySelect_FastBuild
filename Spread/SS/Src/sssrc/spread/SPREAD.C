/*********************************************************
* SPREAD.C
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <malloc.h>
#include "..\..\..\..\fplibs\fptools\src\fptools.h"
#include "spread.h"
#include "ss_alloc.h"
#include "ss_main.h"
#include "..\classes\wintools.h"

#ifdef SS_NO_USE_SH
BOOL SS_BookUnlock(HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
return (hWnd && IsWindow(hWnd) ? GlobalUnlock((GLOBALHANDLE)GetWindowLongPtr(hWnd, 0)) : FALSE);
#else
return (hWnd && IsWindow(hWnd) ? GlobalUnlock((GLOBALHANDLE)GetWindowLong(hWnd, 0)) : FALSE);
#endif
}

BOOL SS_BookUnlockSheetIndex(LPSS_BOOK lpBook, short nIndex)
{
BOOL ret = FALSE;	// RMA fix for uninit var issue
LPGLOBALHANDLE lpSheets;

if (lpBook && lpBook->hSheets && nIndex >= 0 && nIndex < lpBook->nSheetCnt)
	{
	lpSheets = (LPGLOBALHANDLE)GlobalLock(lpBook->hSheets);
	if (lpSheets)
	{
	    ret = GlobalUnlock(lpSheets[nIndex]);
	    GlobalUnlock(lpBook->hSheets);
	}
}

return ret;
}

BOOL SS_BookUnlockSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookUnlockSheetIndex(lpBook, lpBook->nSheet));

return FALSE;
}

BOOL SS_BookUnlockActiveSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookUnlockSheetIndex(lpBook, lpBook->nActiveSheet));

return FALSE;
}

BOOL SS_SheetUnlock(HWND hWnd)
{
BOOL ret = FALSE;
LPSS_BOOK lpBook = SS_BookLock(hWnd);
if (lpBook)
{
    ret = SS_BookUnlockSheet(lpBook);
    SS_BookUnlock(hWnd);
}
return ret;
}

BOOL SS_SheetUnlockIndex(HWND hWnd, short nIndex)
{
BOOL ret = FALSE;
LPSS_BOOK lpBook = SS_BookLock(hWnd);
if (lpBook)
{
    ret = SS_BookUnlockSheetIndex(lpBook, nIndex);
    SS_BookUnlock(hWnd);
}
return ret;
}

BOOL SS_SheetUnlockActive(HWND hWnd)
{
BOOL ret=FALSE;
LPSS_BOOK lpBook = SS_BookLock(hWnd);
if (lpBook)
{
    ret = SS_BookUnlockActiveSheet(lpBook);
    SS_BookUnlock(hWnd);
}
return ret;
}
#endif // SS_NO_USE_SH

#if defined(FP_SPLASHEXPIRE)
BOOL WINAPI _export fpSplashDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif

// CODESENT  
#if !defined(CS_OFF) //&& !defined(SS_V75) && !defined(SS_V80)
#if defined(FP_VB) || defined(FP_OCX)
#include "..\..\..\..\fplibs\codesent\src\codesent.h"
#include "..\..\..\..\fplibs\codesent\src\csentdef.h"
#endif
#else
// define FP_SPLASHEXPIRE for this -scl
//BOOL WINAPI _export fpSplashDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif
// CODESENT  

#ifdef SS_V21
BOOL CheckRuntimeLicense(LPSTR lpszCaption, LPSTR lpszLicenseFileName);
#endif

SS_COLORTABLE  ColorTable = {0};
SS_GLOBALTABLE FontTable = {0};
#if defined(SS_V30) && defined(WIN32)
SS_GLOBALTABLE ExcelTable = {0};
#endif
short          dSpreadInstCnt = 0;
/*
TBGLOBALHANDLE hColorTable = 0;
short          dColorTableCnt = 0;
short          dColorTableAllocCnt = 0;
short          dColorTableUseCnt = 0;
TBGLOBALHANDLE hFontTable = 0;
short          dFontTableCnt = 0;
short          dFontTableAllocCnt = 0;
*/

short dClassNameIndex = 0;

extern HANDLE hDynamicInst;

#ifdef SS_V80

#ifdef SS_W
#if defined(_WIN64) || defined(_IA64)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR64DU80")
   #else
   #define SSPREFIX "SPR64D80"
   #endif
#else
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32DU80")
   #else
   #define SSPREFIX "SPR32D80"
   #endif
#endif // ! ( defined(_WIN64) || defined(_IA64) )
#elif (defined(FP_ADB))
   #ifdef SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPRJ32AU80")
   #else
   #define SSPREFIX "SPRJ32A80"
   #endif
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32AU80")
   #else
   #define SSPREFIX "SPR32A80"
   #endif
   #endif // SPREAD_JPN
#elif (defined(SS_SPREADOCX))
   #ifdef SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPRJ32XU80")
   #else
   #define SSPREFIX "SPRJ32X80"
   #endif
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32XU80")
   #else
   #define SSPREFIX "SPR32X80"
   #endif
   #endif // SPREAD_JPN
#endif
#elif SS_V75

   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR64DU75")
   #else
   #define SSPREFIX "SPR64D75"
   #endif

#elif defined(SS_V70)

#ifdef SS_W

   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32DU70")
   #else
   #define SSPREFIX "SPR32D70"
   #endif

#elif (defined(FP_ADB))

   #ifdef SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPRJ32AU70")
   #else
   #define SSPREFIX "SPRJ32A70"
   #endif
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32AU70")
   #else
   #define SSPREFIX "SPR32A70"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_SPREADOCX))

   #ifdef SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPRJ32XU70")
   #else
   #define SSPREFIX "SPRJ32X70"
   #endif
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32XU70")
   #else
   #define SSPREFIX "SPR32X70"
   #endif
   #endif // SPREAD_JPN
#endif

#elif defined(SS_V40)

#ifdef SS_W

   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32DU60")
   #else
   #define SSPREFIX "SPR32D60"
   #endif

#elif (defined(FP_ADB))

   #ifdef SPREAD_JPN
   #define SSPREFIX "SPRJ32A60"
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32AU60")
   #else
   #define SSPREFIX "SPR32A60"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_SPREADOCX))

   #ifdef SPREAD_JPN
   #define SSPREFIX "SPRJ32X60"
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SPR32XU60")
   #else
   #define SSPREFIX "SPR32X60"
   #endif
   #endif // SPREAD_JPN
#endif

#elif defined(SS_V30)

// jb - 21 sep 98 I changed ss32d30 to SPR32D30 and ss32x30 to SPR32X30
// What about Unicode ?
// GAB 10/6 Changed SSPREFIX from SPR30D32 to SPR32D30 and SS16D30 to SPR16D30 
#ifdef SS_W

   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SS3UD30")
   #else
   #define SSPREFIX "SPR32D30"
   #endif
   #else
   #define SSPREFIX "SPR16D30"
   #endif

#elif (defined(FP_ADB))

   #ifdef SPREAD_JPN
   #define SSPREFIX "SPRJ32A30"
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SS3UA30")
   #else
   #define SSPREFIX "SPR32A30"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_SPREADOCX))

   #ifdef SPREAD_JPN
   #define SSPREFIX "SPRJ32X30"
   #else // SPREAD_JPN
   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SS3UX30")
   #else
   #define SSPREFIX "SPR32X30"
   #endif
   #else
   #define SSPREFIX "SPR16X30"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_GRIDOCX))

   #if defined(WIN32) || defined(_WIN32)
   #define SSPREFIX "Grid32Ocx"
   #else
   #define SSPREFIX "Grid16Ocx"
   #endif

#elif (defined(SS_RSDLL))

   #define SSPREFIX "RS32D30"

#elif (defined(SS_RSOCX))

   #define SSPREFIX "RSSprd32"

#elif (defined(SS_GP))
   #define SSPREFIX "gpSS"

#elif (defined(SS_BC))
   #define SSPREFIX "SVBX2BC_SS"

#elif (defined(SS_SPREADVBXPP))
   #define SSPREFIX "SVBXPP2_SS"

#elif (defined(SS_GRID))
   #define SSPREFIX "FPGRID30_SS"

#elif (defined(SS_MASKTEXT))
   #define SSPREFIX "MT_SS"

#elif (defined(SS_V21))
   #define SSPREFIX "SVBX21_SS"

#else
   #define SSPREFIX "SSVBX30"
#endif

#else // SS_V30

#ifdef SS_W

   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SS3UD25")
   #else
   #define SSPREFIX "SS32D25"
   #endif
   #else
   #define SSPREFIX "SS16D25"
   #endif

#elif (defined(FP_ADB))

   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SS3UA25")
   #else
   #define SSPREFIX "SS32A25"
   #endif
   #else
   #define SSPREFIX "SS16A25"
   #endif

#elif (defined(SS_SPREADOCX))

   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSPREFIX _T("SS3UX25")
   #else
   #define SSPREFIX "SS32X25"
   #endif
   #else
   #define SSPREFIX "SS16X25"
   #endif

#elif (defined(SS_GRIDOCX))

   #if defined(WIN32) || defined(_WIN32)
   #define SSPREFIX "Grid32Ocx"
   #else
   #define SSPREFIX "Grid16Ocx"
   #endif

#elif (defined(SS_RSDLL))

   #define SSPREFIX "RS32D25"

#elif (defined(SS_RSOCX))

   #define SSPREFIX "RSSprd32"

#elif (defined(SS_GP))
   #define SSPREFIX "gpSS"

#elif (defined(SS_BC))
   #define SSPREFIX "SVBX2BC_SS"

#elif (defined(SS_SPREADVBXPP))
   #define SSPREFIX "SVBXPP2_SS"

#elif (defined(SS_GRID))
   #define SSPREFIX "FPGRID25_SS"

#elif (defined(SS_MASKTEXT))
   #define SSPREFIX "MT_SS"

#elif (defined(SS_V21))
   #define SSPREFIX "SVBX21_SS"

#else
   #define SSPREFIX "SSVBX25_SS"
#endif

#endif // SS_V30

#define SS_CLASS(szClassName) SSPREFIX szClassName

SS_CLASSNAMES SSClassNames[1] =
   {
   VA_GENERAL_CLASS_NAME,
   SS_CLASS(_T("Edit")),
   SS_CLASS(_T("EditHScroll")),
   SS_CLASS(_T("EditComboBox")),
   VA_PIC_CLASS_NAME,
   SS_CLASS(_T("Pic")),
   VA_DATE_CLASS_NAME,
   SS_CLASS(_T("Date")),
   VA_TIME_CLASS_NAME,
   SS_CLASS(_T("Time")),
   VA_INTEGER_CLASS_NAME,
   SS_CLASS(_T("Integer")),
   VA_FLOAT_CLASS_NAME,
   SS_CLASS(_T("Float")),
#ifdef SS_V40
   VA_NUM_CLASS_NAME,
   SS_CLASS(_T("Currency")),
   VA_NUM_CLASS_NAME,
   SS_CLASS(_T("Number")),
   VA_NUM_CLASS_NAME,
   SS_CLASS(_T("Percent")),
#endif // SS_V40
#ifdef SS_V70
   SS_CLASS(_T("Scientific")),
#endif // SS_V70
   SS_CLASS(_T("tbSuperBtn")),
   SS_CLASS(_T("SuperBtn")),
   SS_CLASS(_T("tbComboBox")),
   SS_CLASS(_T("ComboBox")),
   SS_CLASS(_T("ListBox")),
   SS_CLASS(_T("tbCheckBox")),
   SS_CLASS(_T("CheckBox")),
   SS_CLASS(_T("ScrlBar")),
   };

#ifdef SS_UTP
#define UCLIPPING
#include "clipping.h"
#endif


BOOL RegisterSpreadSheet(HANDLE hInstance, LPCTSTR lpszClassName,
                         short dClassIndex)
{
WNDCLASS wc;
BOOL     bRet = TRUE;

#ifdef SS_UTP
INIT_CLIPPING;
#endif

// dClassNameIndex = dClassIndex;

/**************************************
* Allocate memory for class structure
**************************************/

wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
wc.lpfnWndProc   = (WNDPROC)SpreadSheetProc;
wc.cbClsExtra    = 0;
wc.cbWndExtra    = 8;
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = NULL;
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = lpszClassName;
if (!RegisterClass(&wc))
   bRet = FALSE;

if (!SS_RegisterEdit(hInstance))
   bRet = FALSE;

#ifndef SS_NOCT_PIC
if (!SS_RegisterPic(hInstance))
   bRet = FALSE;
#endif

#ifndef SS_NOCT_DATE
if (!SS_RegisterDate(hInstance))
   bRet = FALSE;
#endif

#ifndef SS_NOCT_INT
if (!SS_RegisterInteger(hInstance))
   bRet = FALSE;
#endif

if (!SS_RegisterFloat(hInstance))
   bRet = FALSE;

#ifdef SS_V40
if (!SS_RegisterCurrency(hInstance))
   bRet = FALSE;

if (!SS_RegisterNumber(hInstance))
   bRet = FALSE;

if (!SS_RegisterPercent(hInstance))
   bRet = FALSE;
#endif // SS_V40
#ifdef SS_V70
if (!SS_RegisterScientific(hInstance))
	bRet = FALSE;
#endif // SS_V70

#ifndef SS_NOCHILDSCROLL
#ifndef SS_NOSCBAR
if (!RegisterScrlBar(hInstance))
   bRet = FALSE;
#endif
#endif

#ifndef SS_NOCT_TIME
if (!SS_RegisterTime(hInstance))
   bRet = FALSE;
#endif

#ifndef SS_NOCT_BUTTON
if (!SS_RegisterButton(hInstance))
   bRet = FALSE;
#endif

#ifndef SS_NOCT_CHECK
if (!SS_RegisterCheckBox(hInstance))
   bRet = FALSE;
#endif

#ifndef SS_NOCT_COMBO
if (!SS_RegisterComboBox(hInstance))
   bRet = FALSE;
#endif

return bRet;
}


BOOL UnRegisterSpreadSheet(HANDLE hInstance, LPCTSTR lpszClassName)
{
BOOL fOk;

/**************************************
* Allocate memory for class structure
**************************************/

fOk = UnregisterClass(lpszClassName, hInstance);

fOk = SS_UnRegisterEdit(hInstance);

#ifndef SS_NOCT_PIC
fOk = SS_UnRegisterPic(hInstance);
#endif

#ifndef SS_NOCT_DATE
fOk = SS_UnRegisterDate(hInstance);
#endif

#ifndef SS_NOCT_INT
fOk = SS_UnRegisterInteger(hInstance);
#endif

fOk = SS_UnRegisterFloat(hInstance);

#ifdef SS_V40
fOk = SS_UnRegisterCurrency(hInstance);
fOk = SS_UnRegisterNumber(hInstance);
fOk = SS_UnRegisterPercent(hInstance);
#ifdef SS_V70
fOk = SS_UnRegisterScientific(hInstance);
#endif // SS_V70
#endif // SS_V40

#ifndef SS_NOCHILDSCROLL
#ifndef SS_NOSCBAR
fOk = UnRegisterScrlBar(hInstance);
#endif
#endif

#ifndef SS_NOCT_TIME
fOk = SS_UnRegisterTime(hInstance);
#endif

#ifndef SS_NOCT_BUTTON
fOk = SS_UnRegisterButton(hInstance);
#endif

#ifndef SS_NOCT_CHECK
fOk = SS_UnRegisterCheckBox(hInstance);
#endif

#ifndef SS_NOCT_COMBO
fOk = SS_UnRegisterComboBox(hInstance);
#endif


return (fOk);
}


BOOL fpIsTrialVersion()
{
#ifdef FP_SPLASHEXPIRE
   return TRUE;
#else 
   return FALSE;
#endif   
}


WORD fpSplashIsExpired(HINSTANCE hInst)
{
// CODESENT  
#ifndef CS_OFF
#if defined(SS_V80) && defined(FP_DLL)
#if FP_SPLASHEXPIRE
// time bomb for Spread v8 DLL beta
FPDATE fpDate;
fpSET_DATE(fpDate, 2009, 12, 02);
if (fpTodaysDate() >= fpDate)
   {
	 DialogBoxParam(hInst, _T("SplashExpire"), 0, (DLGPROC)fpSplashDlgProc, 0L);
     return (FPSPLASH_EXPIRED);
   }
return FPSPLASH_OK;
#else
return FPSPLASH_OK;
#endif

#elif SS_V75
#ifdef FP_SPLASHEXPIRE
// time bomb for Spread 64-bit v7.5 beta
FPDATE fpDate;
fpSET_DATE(fpDate, 2008, 10, 30);
if (fpTodaysDate() > fpDate)
   {
	 DialogBoxParam(hInst, _T("SplashExpire"), 0, (DLGPROC)fpSplashDlgProc, 0L);
     return (FPSPLASH_EXPIRED);
   }
#endif
return FPSPLASH_OK;
#elif defined(FP_VB) || defined(FP_OCX)
   int   retVal = 0;
    short nType;
    char  buffer[64];
    BOOL  bLoop = TRUE;
    WORD  wReturn = FPSPLASH_EXPIRED;
    
#ifdef SS_V80
#ifdef FP_SPLASHEXPIRE
// time bomb for Spread v8 beta OCX
FPDATE fpDate;
fpSET_DATE(fpDate, 2008, 12, 02);
if (fpTodaysDate() >= fpDate)
   {
	 DialogBoxParam(hInst, _T("SplashExpire"), 0, (DLGPROC)fpSplashDlgProc, 0L);
     return (FPSPLASH_EXPIRED);
   }
#endif
#endif

    if (!fpInstance)
       fpInstance = hInst;
    while (bLoop)
       {
       retVal = SoftwareSecureOpen(SS_SECURE_PRODUCT_NAME, SS_SECURE_PREFIX, SS_SECURE_PRODUCT_CODE, 
                                   SS_SECURE_VERSION_MAJOR, SS_SECURE_VERSION_MINOR, SS_SECURE_VERSION_REV,
                                   SS_SECURE_PRODUCT_STRING, &nType, 
#ifdef FP_BETA
                                   TRUE,
#else
                                   FALSE, 
#endif
                                   fpInstance);

       if (retVal == OPEN_CANCEL)
          bLoop = FALSE;
       else if (retVal == OPEN_EXPIRED)
          bLoop = FALSE;
       else if ((retVal == OPEN_PURCHASE) ||
                (retVal == OPEN_PURCHASE_NEW))
          {
          _fmemset((LPVOID)buffer, '\0', sizeof(buffer));

          if (nType == USAGE_TYPE_OTHER)
            buffer[0] = '~';

          if (SoftwareSecureUnlock(SS_SECURE_PRODUCT_NAME,
                             (LPSTR)buffer, SS_SECURE_PRODUCT_CODE,
                             (HINSTANCE)fpInstance, 
                             0, 
                             NULL,
#ifdef WIN32
                             TRUE) == TRUE)  
#else
                             FALSE) == TRUE)  
#endif

             {
             char *pLocBuff = NULL;

             if (pLocBuff = strchr(buffer, '~'))
                {
                pLocBuff[0] = '\0';
                SoftwareSecureSetSerialNumber(SS_SECURE_PRODUCT_NAME, SS_SECURE_PRODUCT_CODE,
                                              (LPSTR)buffer, ++pLocBuff);
                }
             else
                SoftwareSecureSetSerialNumber(SS_SECURE_PRODUCT_NAME, SS_SECURE_PRODUCT_CODE,
                                             (LPSTR)buffer, NULL);

             bLoop = FALSE;
             wReturn = FPSPLASH_OK;
             }
          else
             {
             SoftwareSecureClose(SS_SECURE_PRODUCT_NAME);
             }
          }
       else    // agree to evaluation license
          {
          // key is valid
          bLoop = FALSE;
          wReturn = FPSPLASH_OK;
          }  
       }

    SoftwareSecureClose(SS_SECURE_PRODUCT_NAME);
    return (wReturn);
#else
    return (FPSPLASH_NONE);
#endif
// CODESENT  
#else
static nCallCnt = 0;

#ifdef FP_SPLASHEXPIRE
FPDATE fpDate;
//GAB DLL EXPIRE - This is where the expiration date is set for the DLLs
fpSET_DATE(fpDate, 2008, 12, 02);
if (fpTodaysDate() >= fpDate)
   {
    #ifdef SS_VB
	fpVBDialogBoxParam(0, hInst, "SplashExpire", (FARPROC)fpSplashDlgProc, 0L);
    #elif (defined(WIN32) && defined(FP_OCX)) || defined(SS_V80)
	DialogBoxParam(hInst, _T("SplashExpire"), 0, (FARPROC)fpSplashDlgProc, 0L);
    #endif
   
   return (FPSPLASH_EXPIRED);
   }

if (nCallCnt > 0)
   return (FPSPLASH_OK);

nCallCnt++;

#ifdef SS_VB
fpVBDialogBoxParam(0, hInst, "SplashScreen", (FARPROC)fpSplashDlgProc, 0L);
#elif (defined(WIN32) && defined(FP_OCX)) || defined(SS_V80)
DialogBoxParam(hInst, _T("SplashScreen"), 0, (FARPROC)fpSplashDlgProc, 0L);
#endif
return (FPSPLASH_OK);

#else
return (FPSPLASH_NONE);
#endif

#endif
}

#if defined(FP_SPLASHEXPIRE)

BOOL WINAPI _export fpSplashDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
static HBRUSH hBrush;

switch (Msg)
   {
   case WM_INITDIALOG:
      hBrush = CreateSolidBrush(RGBCOLOR_PALEGRAY);

      SetTimer(hDlg, 1, 15000, 0);
      DlgBoxCenter(hDlg);
      return (TRUE);

#ifndef WIN32
   case WM_CTLCOLOR:
      if (HIWORD(lParam) == CTLCOLOR_DLG)
         return (hBrush);

      else if (HIWORD(lParam) == CTLCOLOR_STATIC)
         {
         SetTextColor((HDC)wParam, RGBCOLOR_BLACK);
         SetBkColor((HDC)wParam, RGBCOLOR_PALEGRAY);
         return (hBrush);
         }

      return (0);
#else
   case WM_CTLCOLORDLG:
      return (long)(hBrush);

   case WM_CTLCOLORSTATIC:
      SetTextColor((HDC)wParam, RGBCOLOR_BLACK);
      SetBkColor((HDC)wParam, RGBCOLOR_PALEGRAY);
      return (long)(hBrush);
#endif

   case WM_TIMER:
      EndDialog(hDlg, TRUE);
      return (0);

   #if (defined(WIN32) && defined(FP_OCX))
   case WM_ACTIVATE:
      if (LOWORD(wParam) == WA_INACTIVE)
	  { 
		  EndDialog(hDlg, TRUE);
		  return (0);
	  }
	  break;
  #endif
   
   case WM_DESTROY:
      KillTimer(hDlg, 1);
      DeleteObject(hBrush);
      break;

   case WM_COMMAND:
      switch (WM_CMD_ID(wParam))
         {
         case IDOK:
         case IDCANCEL:
            EndDialog(hDlg, TRUE);
            return (TRUE);
         }

      break;

   }

return (FALSE);
}

#endif

#ifndef SS_DLL

BOOL SS_CheckLicense(LPTSTR lpszFileName, LPTSTR lpszLicName, BOOL fRuntime)
{
#if 0
#ifdef SS_V21
if (!fRuntime)
   if (!CheckRuntimeLicense(lpszFileName, lpszLicName))
      return (FALSE);
#else
WORD wSplash;

if (!fRuntime)
   wSplash = fpSplashIsExpired(hDynamicInst);
else
   wSplash = FPSPLASH_OK;
      

if (wSplash == FPSPLASH_NONE)
   {
   if (!fRuntime)
#ifdef CS_OFF
      if (!fpCheckRuntimeLicense(lpszFileName, lpszLicName, SS_LIC_ID, 0,
                                 FALSE))
#endif
         return (FALSE);
   }

else if (wSplash == FPSPLASH_EXPIRED)
   return (FALSE);

#endif
#endif

return (TRUE);
}

#endif
