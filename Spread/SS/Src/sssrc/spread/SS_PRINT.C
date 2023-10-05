/*********************************************************
* SS_PRINT.C
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
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include <string.h>

#ifndef WIN32
#include <print.h>
#else
#include <winspool.h>
#endif

#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_draw.h"
#include "ss_font.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_span.h"
#include "ss_user.h"
#include "ss_virt.h"
#include "ss_win.h"

#ifdef SS_VB
//#include <vbapi.h>
#include "..\..\vbsrc\vbspread.h"
#include "..\..\vbsrc\vbprint2.h"
#endif

void vbSSReplacePrintAbortItem(HWND hWndSpread, HDC hOldDC, HDC hNewDC);

#ifndef SS_NOPRINT

#if SS_V70
#define SS_PROCESS_PAGEBREAKS(lpPFormat) ((lpPFormat)->pf.wScalingMethod != SS_PRINTSCALINGMETHOD_BESTFIT)
#else
#define SS_PROCESS_PAGEBREAKS(lpPFormat) TRUE
#endif

/*
typedef struct tagBANDINFOSTRUCT
   {
   BOOL    fGraphics;
   BOOL    fText;
   RECT    rcGraphics;
   } BANDINFOSTRUCT, FAR* LPBI;
*/

#define PRINTERINFO_MAXSIZE 400
TCHAR szPrinter[PRINTERINFO_MAXSIZE];

#ifdef SS_UTP
TCHAR szUtpDevice[120] = {0};
TCHAR szUtpDriver[50] = {0};
TCHAR szUtpOutput[50] = {0};
#endif

extern HANDLE hDynamicInst;

#define SCALE_LIMIT 0.6

BOOL SS_PrintDrawPageOfRows(LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                            LPRECT lpRectBand, SS_PRINTFUNC lpfnPrintCallBack,
                            HFONT hFont);
BOOL SS_PrintDrawPageOfCols(LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                            LPRECT lpRectBand, SS_PRINTFUNC lpfnPrintCallBack,
                            HFONT hFont);
BOOL SS_PrintDrawPage(LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                      LPRECT lpRectBand, SS_PRINTFUNC lpfnPrintCallBack,
                      HFONT hFont);
BOOL CALLBACK SS_PrintAbortProc(HDC hPrnDC, int iCode);
void SS_PrintCalcPrinter(HDC hDC, LPSSX_PRINTFORMAT lpPFormat,
                         LPRECT lpRectPage);
void SS_PrintInit(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat,
                  LPSS_PAGEFORMAT lpPageFormat, WORD wAction,
                  LPRECT lpRectPreview);
HDC SS_PrintSmartPrint(LPSPREADSHEET lpSS, HDC hDC,
                       LPSSX_PRINTFORMAT lpPFormat, LPTSTR szDevice,
                       LPTSTR szDriver, LPTSTR szOutput, LPRECT lpRectPage,
                       FARPROC lpfnAbortProc, LPBOOL lpbContinue, LPDEVMODE lpDevModeIn);
BOOL SS_PrintBestFit(LPSPREADSHEET lpSS, HDC hDC, LPSSX_PRINTFORMAT lpPFormat,
                     LPRECT lpRectPage);
HDC SS_PrintSetOrientation(LPSPREADSHEET lpSS, HDC hDC,
                           LPSSX_PRINTFORMAT lpPFormat, LPTSTR szDevice,
                           LPTSTR szDriver, LPTSTR szOutput, LPRECT lpRectPage,
                           WORD wPrintOrient, LPDEVMODE lpDevModeIn, BOOL fForceOrientation);
SS_COORD SS_PrintCalcCols(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat,
                          LPRECT lpRect, SS_COORD ColAt);
SS_COORD SS_PrintCalcRows(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat,
                          LPRECT lpRect, SS_COORD RowAt, BOOL fIgnorePageBreaks);
void SS_PrintBorder(HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRectPrint,
                    LPRECT lpRectBand);
void SS_PrintLineTo(HDC hDC, int x, int y, LPSSX_PRINTFORMAT lpPFormat);
#if defined(_WIN64) || defined(_IA64)
void SS_PrintHeader(LPSPREADSHEET lpSS, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                    SS_PRINTFUNC lpfnPrintCallBack, long lPage, LONG_PTR lAppData,
                    HFONT hFont, WORD wType);
BOOL SS_PrintPage(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                  LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRectBand,
                  LPRECT lpRectPrint, LPRECT lpRectUpdate,
                  SS_PRINTFUNC lpfnPrintCallBack, long lPage, LONG_PTR lAppData,
                  HFONT hFont, SS_COORD ColAt, SS_COORD RowAt);
#else
void SS_PrintHeader(LPSPREADSHEET lpSS, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                    SS_PRINTFUNC lpfnPrintCallBack, long lPage, long lAppData,
                    HFONT hFont, WORD wType);
BOOL SS_PrintPage(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                  LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRectBand,
                  LPRECT lpRectPrint, LPRECT lpRectUpdate,
                  SS_PRINTFUNC lpfnPrintCallBack, long lPage, long lAppData,
                  HFONT hFont, SS_COORD ColAt, SS_COORD RowAt);
#endif

#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSPrint(HWND hWnd, LPCTSTR lpszPrintJobName,
                      LPSS_PRINTFORMAT lpPrintFormat, SS_PRINTFUNC lpfnPrintProc,
                      LONG_PTR lAppData, ABORTPROC lpfnAbortProc )
#else
BOOL DLLENTRY SSPrint(HWND hWnd, LPCTSTR lpszPrintJobName,
                      LPSS_PRINTFORMAT lpPrintFormat, SS_PRINTFUNC lpfnPrintProc,
                      LONG lAppData, ABORTPROC lpfnAbortProc )
#endif
{
LPSPREADSHEET   lpSS = SS_SheetLock(hWnd);
SS_PAGEFORMAT   PageFormat;
LPSS_PAGEFORMAT lpPageFormat = &PageFormat;
BOOL            fRet;

if (lpPrintFormat)
	SS_PrintFormat2PageFormat(lpPageFormat, lpPrintFormat);
else
   lpPageFormat = &lpSS->PrintOptions.pf;

fRet = SS_Print(lpSS, lpszPrintJobName, lpPageFormat, lpfnPrintProc,
                lAppData, lpfnAbortProc, SS_PRINT_ACTION_PRINT, 0, NULL,
                NULL, NULL, NULL, NULL, NULL);

SS_SheetUnlock(hWnd);
return (fRet);
}

#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSPrintPreview(HWND hWnd, LPCTSTR lpszPrintJobName,
                      LPSS_PRINTFORMAT lpPrintFormat,
                      SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData,
                      ABORTPROC lpfnAbortProc )
#else
BOOL DLLENTRY SSPrintPreview(HWND hWnd, LPCTSTR lpszPrintJobName,
                      LPSS_PRINTFORMAT lpPrintFormat,
                      SS_PRINTFUNC lpfnPrintProc, LONG lAppData,
                      ABORTPROC lpfnAbortProc )
#endif
{
LPSPREADSHEET   lpSS = SS_SheetLock(hWnd);
SS_PAGEFORMAT   PageFormat;
LPSS_PAGEFORMAT lpPageFormat = &PageFormat;
FLOAT dHorzResPixels, dVertResPixels; 
//FLOAT dHorzPixelByInch, dVertPixelByInch;
BOOL            fRet;
	RECT RectPage;

if (lpPrintFormat)
	SS_PrintFormat2PageFormat(lpPageFormat, lpPrintFormat);
else
   lpPageFormat = &lpSS->PrintOptions.pf;
	
	dHorzResPixels = (FLOAT)GetDeviceCaps( lpSS->PrintOptions.pf.hDCPrinter, HORZRES );
	dVertResPixels = (FLOAT)GetDeviceCaps( lpSS->PrintOptions.pf.hDCPrinter, VERTRES );
//	dHorzPixelByInch = (FLOAT)GetDeviceCaps( lpSS->PrintOptions.pf.hDCPrinter, LOGPIXELSX );
//	dVertPixelByInch = (FLOAT)GetDeviceCaps( lpSS->PrintOptions.pf.hDCPrinter, LOGPIXELSY );

	RectPage.left = 0;
	RectPage.top = 0;
	RectPage.right = dHorzResPixels;
	RectPage.bottom = dVertResPixels;

// 	lpPageFormat->fMarginLeft = 0;
// 	lpPageFormat->fMarginTop = 0;
// 	lpPageFormat->fMarginRight = ( dHorzResPixels - lpPageFormat->fMarginRight ) / dHorzPixelByInch;
// 	lpPageFormat->fMarginBottom = ( dVertResPixels - lpPageFormat->fMarginBottom ) / dVertPixelByInch;

	lpPageFormat->dPrintType |= SS_PRINTTYPE_PAGERANGE;

   fRet = SS_Print(lpSS, NULL, lpPageFormat, lpfnPrintProc,
					lAppData, lpfnAbortProc, SS_PRINT_ACTION_PREVIEW,
					lpPageFormat->hDCPrinter, &RectPage, NULL, NULL, NULL, NULL, NULL);

SS_SheetUnlock(hWnd);
return (fRet);
}

#if defined(_WIN64) || defined(IA64)
BOOL SS_Print(LPSPREADSHEET lpSS, LPCTSTR lpszPrintJobName, LPSS_PAGEFORMAT lpPageFormat,
              SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData, ABORTPROC lpfnAbortProc,
              WORD wAction, HDC hDCPreview, LPRECT lpRectPreview,
              LPSS_PRINTPAGECALC lpPrintPageCalc, LPDEVMODE lpDevModeIn, LPCTSTR lpszDriver,
              LPCTSTR lpszDevice, LPCTSTR lpszOutput)
#else
BOOL SS_Print(LPSPREADSHEET lpSS, LPCTSTR lpszPrintJobName, LPSS_PAGEFORMAT lpPageFormat,
              SS_PRINTFUNC lpfnPrintProc, LONG lAppData, ABORTPROC lpfnAbortProc,
              WORD wAction, HDC hDCPreview, LPRECT lpRectPreview,
              LPSS_PRINTPAGECALC lpPrintPageCalc, LPDEVMODE lpDevModeIn, LPCTSTR lpszDriver,
              LPCTSTR lpszDevice, LPCTSTR lpszOutput)
#endif
{
#ifndef WIN32
BANDINFOSTRUCT  BandInfo;
#endif
SSX_PRINTFORMAT PFormat;
SS_PRINTFUNC    lpfnPrintCallBack;
FARPROC         lpfnAbortProcTemp = NULL;
RECT            RectPage;
RECT            RectBand;
HFONT           hFontOld;
HFONT           hFontDefPrinter = 0;
HPEN            hPenOld;
HPEN            hPen;
HFONT           hFont;
LPSS_FONT       Font;
SS_FONT         FontTemp;
SS_FONTID       FontId;
HDC             hDC = 0;
HDC             hDCPrinter = 0;
RECT            Rect;
BOOL            fRet = FALSE;
BOOL            fBandInfoDevice = 0;
BOOL            fNextBandDevice = 0;
LPTSTR          szDevice = NULL;
LPTSTR          szDriver = NULL;
LPTSTR          szOutput = NULL;
BOOL            bContinue = TRUE;
DOCINFO         docInfo;
BOOL            fEnableWindow = FALSE;
BOOL            fAbort = FALSE;

#ifdef SS_UTP
if (szUtpDriver[0] && szUtpOutput[0])
   {
   szDevice = szUtpDevice;
   szDriver = szUtpDriver;
   szOutput = szUtpOutput;
   }
#endif

if (!lpPageFormat)
   lpPageFormat = &lpSS->PrintOptions.pf;

lpfnPrintCallBack = (SS_PRINTFUNC)lpfnPrintProc;

if (lpPageFormat->hDCPrinter)
   {
   hDC = lpPageFormat->hDCPrinter;
   }
else
   {
	if (lpDevModeIn)
		{
		szDriver = (LPTSTR)lpszDriver;
		szDevice = (LPTSTR)lpszDevice;
		szOutput = (LPTSTR)lpszOutput;
		}
	else
      {
      GetProfileString(_T("Windows"), _T("device"), _T(",,,"), szPrinter, PRINTERINFO_MAXSIZE);

      szDevice = _ftcstok(szPrinter, _T(",\0"));
      szDriver = _ftcstok(NULL, _T(",\0"));
      szOutput = _ftcstok(NULL, _T(",\0"));
      }

#ifdef WIN32
   if (szDevice && szDriver)
      {
#if 0 // RFW 10/31/97 because of a problem with a user using an A4 cartridge
      LPDEVMODE lpDevMode = NULL;
      DEVMODE   DevModeNew;

      if (lpPageFormat->wOrientation != SS_PRINTORIENT_DEFAULT)
         {
         _fmemset(&DevModeNew, '\0', sizeof(DEVMODE));
         lstrcpy(DevModeNew.dmDeviceName, szDevice);
//         DevModeNew.dmSpecVersion = DM_SPECVERSION;
//         DevModeNew.dmSize = sizeof(DEVMODE);
         DevModeNew.dmSpecVersion = 0x0320;
         DevModeNew.dmSize = sizeof(DEVMODE);
#if (WINVER >= 0x0400)
         DevModeNew.dmSize -= 48;
#endif
         DevModeNew.dmFields = DM_ORIENTATION;
         DevModeNew.dmOrientation = (lpPageFormat->wOrientation ==
                                     SS_PRINTORIENT_PORTRAIT ?
                                     DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE);
         lpDevMode = &DevModeNew;
         }
#endif

      hDC = CreateDC(szDriver, szDevice, NULL, lpDevModeIn);
      }
#else
   if (szDevice && szDriver && szOutput)
      hDC = CreateDC(szDriver, szDevice, szOutput, NULL);
#endif
   }

if (hDC)
   {
   lpSS->fPrintingInProgress++;

   if (wAction == SS_PRINT_ACTION_PRINT ||
       wAction == SS_PRINT_ACTION_OWNERPRINT)
      {
#ifndef WIN32
      int nEscapeCode;

      nEscapeCode = NEXTBAND;
      fNextBandDevice = Escape(hDC, QUERYESCSUPPORT, sizeof(int),
                               (LPVOID)&nEscapeCode, NULL);
      nEscapeCode = BANDINFO;
      fBandInfoDevice = Escape(hDC, QUERYESCSUPPORT, sizeof(int),
                               (LPVOID)&nEscapeCode, NULL);
      BandInfo.fGraphics = TRUE;
      BandInfo.fText = TRUE;
#endif
		if (IsWindowEnabled(lpSS->lpBook->hWnd))
			{
			EnableWindow(lpSS->lpBook->hWnd, FALSE);
			fEnableWindow = TRUE;
			}
      }

   /**************************
   * Initialize for printing
   **************************/

   _fmemset(&PFormat, '\0', sizeof(SSX_PRINTFORMAT));
   SS_PrintInit(lpSS, &PFormat, lpPageFormat, wAction, lpRectPreview);

   if (!lpPageFormat->hDCPrinter)
      if (StrStr(szDevice, _T("HP")) && StrStr(szDevice, _T("4000")) &&
          StrStr(szDevice, _T("PCL 6")))
        PFormat.wPrinterType = SS_PRINTER_HP4000PCL6;

   if (lpSS->fVirtualMode &&
       !(PFormat.pf.dPrintType & SS_PRINTTYPE_DOWNTHENOVER))
      PFormat.pf.dPrintType |= SS_PRINTTYPE_OVERTHENDOWN;

   /***************************************************
   * Determine if this PostScript printer has scaling
   ***************************************************/

	PFormat.dfScale = 1.0;

#ifdef SS_V70
	if (SS_PRINTSCALINGMETHOD_ZOOM == lpPageFormat->wScalingMethod &&
       lpPageFormat->ZoomFactor != 0)
		PFormat.dfScale = lpPageFormat->ZoomFactor;
#endif // SS_V70

   /*
   if ((hInstDevCap = LoadLibrary(szDriver)) > HINSTANCE_ERROR)
      {
      if ((FARPROC)lpfnDevCap = GetProcAddress(hInstDevCap,
                                               "DeviceCapabilities"))
         if ((*lpfnDevCap)(szDevice, szOutput, 0, NULL, &DevMode) != -1)
            if (DevMode.dmScale != 0 && DevMode.dmScale != 100)
               PFormat.dfScale = (double)DevMode.dmScale / 100.0;

      FreeLibrary(hInstDevCap);
      }
   */

   /********************************
   * Get information about printer
   ********************************/

   PFormat.hDCPreview = hDCPreview;
   SS_PrintCalcPrinter(hDC, &PFormat, &RectPage);

	if (!lpDevModeIn)
	   hDC = SS_PrintSetOrientation(lpSS, hDC, &PFormat, szDevice,
	                                szDriver, szOutput, &RectPage,
	                                lpPageFormat->wOrientation, lpDevModeIn, FALSE);
   hDCPrinter = hDC;

   /******************************************
   * Set default font to default spread font
   ******************************************/

   FontId = SSx_RetrieveFontId(lpSS, NULL, NULL, NULL, -1, -1);
   Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);

   PFormat.wAction = SS_PRINT_ACTION_PRINT;
   hFont = SS_CreatePrinterFont(lpSS, hDCPrinter, Font, FontId, &PFormat);
   PFormat.wAction = wAction;
   hFontOld = SelectObject(hDC, hFont);
   hFontDefPrinter = hFont;

   if (wAction == SS_PRINT_ACTION_PREVIEW)
      {
      Font->hFontPrinter = 0;
      hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, &PFormat);
      }

   if (lpfnPrintCallBack)
      {
#ifdef SS_V30
      PFormat.AppHeaderLen = (*lpfnPrintCallBack)(hDC, SS_PRINTCMD_HEADERLEN,
                                                  0, lAppData, NULL);
      PFormat.AppFooterLen = (*lpfnPrintCallBack)(hDC, SS_PRINTCMD_FOOTERLEN,
                                                  0, lAppData, NULL);
#else
      PFormat.AppHeaderLen = (*lpfnPrintCallBack)(hDC, SS_PRINTCMD_HEADERLEN,
                                                  0, lAppData);
      PFormat.AppFooterLen = (*lpfnPrintCallBack)(hDC, SS_PRINTCMD_FOOTERLEN,
                                                  0, lAppData);
#endif
      PFormat.AppHeaderLenOrig = PFormat.AppHeaderLen;
      PFormat.AppFooterLenOrig = PFormat.AppFooterLen;
      SS_PrintCalcPrinter(hDC, &PFormat, &RectPage);
      }

   if (wAction == SS_PRINT_ACTION_PRINT && !lpfnAbortProc)
      {
      lpfnAbortProc = MakeProcInstance(SS_PrintAbortProc, (HINSTANCE)hDynamicInst);
      lpfnAbortProcTemp = (FARPROC)lpfnAbortProc;
      }

   /**********************************************************
   * Add the logic for determining if the spreadsheet should
   * print in landscape and if it should scale itself
   **********************************************************/

#ifdef SS_V70
   if (PFormat.pf.wScalingMethod == SS_PRINTSCALINGMETHOD_SMARTPRINT)
#else
   if (PFormat.pf.fSmartPrint)
#endif // SS_V70
      {
      SelectObject(hDC, hFontOld);
      hDC = SS_PrintSmartPrint(lpSS, hDC, &PFormat, szDevice, szDriver,
                               szOutput, &RectPage, (FARPROC)lpfnAbortProc, &bContinue, lpDevModeIn);
      hDCPrinter = hDC;

      PFormat.wAction = SS_PRINT_ACTION_PRINT;
      hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, &PFormat);
      PFormat.wAction = wAction;
      hFontOld = SelectObject(hDC, hFont);
      hFontDefPrinter = hFont;

      if (wAction == SS_PRINT_ACTION_PREVIEW)
         {
         Font->hFontPrinter = 0;
         hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, &PFormat);
         }
      }

#ifdef SS_V70
   else if (PFormat.pf.wScalingMethod == SS_PRINTSCALINGMETHOD_BESTFIT)
      {
      SelectObject(hDC, hFontOld);
      SS_PrintBestFit(lpSS, hDC, &PFormat, &RectPage);

      PFormat.wAction = SS_PRINT_ACTION_PRINT;
      hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, &PFormat);
      PFormat.wAction = wAction;
      hFontOld = SelectObject(hDC, hFont);
      hFontDefPrinter = hFont;

      if (wAction == SS_PRINT_ACTION_PREVIEW)
         {
         Font->hFontPrinter = 0;
         hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, &PFormat);
         }
      }
#endif // SS_V70

   CopyRect(&RectBand, &RectPage);
#ifndef WIN32
   CopyRect(&BandInfo.rcGraphics, &RectPage);
#endif

   if (wAction == SS_PRINT_ACTION_PREVIEW)
      hDC = hDCPreview;

   if (PFormat.xPrintArea <= 0 || PFormat.yPrintArea <= 0)
      fRet = FALSE;

   else if (wAction == SS_PRINT_ACTION_NEXTCOL)
      {
      lpPrintPageCalc->lPageCnt = SS_PrintCalcCols(lpSS, &PFormat,
                                                   &Rect, PFormat.ColLeft);
      if (lpPrintPageCalc->lPageCnt >= PFormat.ColsMax)
         lpPrintPageCalc->lPageCnt = -1;
      }

   else if (wAction == SS_PRINT_ACTION_NEXTROW)
      {
      lpPrintPageCalc->lPageCnt = SS_PrintCalcRows(lpSS, &PFormat,
                                                   &Rect, PFormat.RowTop, FALSE);
      if (lpPrintPageCalc->lPageCnt >= PFormat.RowsMax)
         lpPrintPageCalc->lPageCnt = -1;
      }

   else
      {
      _fmemset(&docInfo, '\0', sizeof(docInfo));
      docInfo.cbSize = sizeof(docInfo);
      docInfo.lpszDocName = lpszPrintJobName;
      docInfo.lpszOutput = NULL;

	  if (bContinue &&
          ((wAction != SS_PRINT_ACTION_PRINT && wAction != SS_PRINT_ACTION_OWNERPRINT) ||
            (wAction == SS_PRINT_ACTION_OWNERPRINT || (SetAbortProc(hDC, lpfnAbortProc) > 0 && StartDoc(hDC, &docInfo) > 0))))
         {
         /******************
         * Set up defaults
         ******************/

         if (wAction != SS_PRINT_ACTION_PAGECALC)
            {
            if (!PFormat.pf.fDrawColors)
               SetTextColor(hDC, RGBCOLOR_BLACK);

            hPen = CreatePen(PS_SOLID, 1, RGBCOLOR_BLACK);
            hPenOld = SelectObject(hDC, hPen);
            }

         /**************
         * Print pages
         **************/

         PFormat.lPage = 1;

         PFormat.ColLeftOrig = PFormat.ColLeft;
         PFormat.RowTopOrig = PFormat.RowTop;
         PFormat.lpSS = lpSS;
         PFormat.hWnd = lpSS->lpBook->hWnd;
         PFormat.hDC = hDC;
         PFormat.lAppData = lAppData;
         PFormat.fNextBandDevice = fNextBandDevice;
         PFormat.wAction = wAction;

         if (PFormat.pf.dPrintType & SS_PRINTTYPE_OVERTHENDOWN)
            fRet = SS_PrintDrawPageOfRows(&PFormat, &Rect, &RectBand,
                                          lpfnPrintCallBack, hFont);
         else
            fRet = SS_PrintDrawPageOfCols(&PFormat, &Rect, &RectBand,
                                          lpfnPrintCallBack, hFont);

         if (wAction != SS_PRINT_ACTION_PAGECALC)
            {
            SelectObject(hDC, hPenOld);
            DeleteObject(hPen);
            }

         if (wAction == SS_PRINT_ACTION_PRINT)
            {
            if (fRet == -1)
					{
               AbortDoc(hDC);
					fAbort = TRUE;
					}
            else
               EndDoc(hDC);
            }

         if (fRet == -1)
            fRet = FALSE;
         else
            fRet = TRUE;
         }
      }

   SelectObject(hDC, hFontOld);

   if (wAction == SS_PRINT_ACTION_PREVIEW)
      DeleteObject(hFontDefPrinter);

// RFW - 12/17/03 - 12993
//   if (hDCPrinter && hDCPrinter != lpPageFormat->hDCPrinter)
   if (hDCPrinter && hDCPrinter != lpPageFormat->hDCPrinter && !fAbort)
      DeleteDC(hDCPrinter);

   if (lpSS->lPrintColWidthListCnt)
      {
      tbGlobalFree(lpSS->hPrintColWidthList);
      lpSS->lPrintColWidthListCnt = 0;
      lpSS->hPrintColWidthList = 0;
      }

#ifdef SS_UTP
   if (lpSS->lPrintRowHeightListCnt)
      {
      tbGlobalFree(lpSS->hPrintRowHeightList);
      lpSS->lPrintRowHeightListCnt = 0;
      lpSS->hPrintRowHeightList = 0;
      }
#endif

   if (wAction == SS_PRINT_ACTION_PRINT || wAction == SS_PRINT_ACTION_OWNERPRINT)
      {
		if (fEnableWindow)
			EnableWindow(lpSS->lpBook->hWnd, TRUE);

      if (lpfnAbortProcTemp)
         FreeProcInstance(lpfnAbortProcTemp);
      }

   lpSS->fPrintingInProgress--;

   if (wAction == SS_PRINT_ACTION_PAGECALC)
      {
      lpPrintPageCalc->lPageCnt = PFormat.lPage - 1;
      lpPrintPageCalc->fPageWidth = (float)PFormat.dHorzPrinterRes /
                                     (float)PFormat.TextWidthPrinter;
      lpPrintPageCalc->fPageHeight = (float)PFormat.dVertPrinterRes /
                                      (float)PFormat.TextHeightPrinter;
      }

   if (!lpSS->fPrintingInProgress)
      SS_DestroyPrinterFonts(lpSS);
   }

else if (wAction == SS_PRINT_ACTION_PAGECALC)
   {
   lpPrintPageCalc->lPageCnt = 0;
   lpPrintPageCalc->fPageWidth = 0.0F;
   lpPrintPageCalc->fPageHeight = 0.0F;
   }

return (fRet);
}


BOOL SS_PrintDrawPageOfRows(LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                            LPRECT lpRectBand, SS_PRINTFUNC lpfnPrintCallBack,
                            HFONT hFont)
{
BOOL fRet;

/*************************
* Draw each page of rows
*************************/

lpPFormat->RowTop = lpPFormat->RowTopOrig;
lpPFormat->RowAt = lpPFormat->RowTop;

do
   {
   if (lpPFormat->pf.dPrintType & SS_PRINTTYPE_OVERTHENDOWN)
      SetRectEmpty(lpRect);

   lpPFormat->RowAt = SS_PrintCalcRows(lpPFormat->lpSS, lpPFormat, lpRect,
                                       lpPFormat->RowAt, FALSE);

   if (lpPFormat->pf.dPrintType & SS_PRINTTYPE_OVERTHENDOWN)
      fRet = SS_PrintDrawPageOfCols(lpPFormat, lpRect, lpRectBand,
                                    lpfnPrintCallBack, hFont);
   else
      fRet = SS_PrintDrawPage(lpPFormat, lpRect, lpRectBand,
                              lpfnPrintCallBack, hFont);

   if (fRet != TRUE)
      break;

   // RFW - KEM16 - 5/17/99
   else if (lpPFormat->fAtVirtualEnd &&
            lpPFormat->RowAt >= lpPFormat->lpSS->Virtual.VTop +
            lpPFormat->lpSS->Virtual.VPhysSize)
      break;

   lpPFormat->RowTop = lpPFormat->RowAt;
   } while (lpPFormat->RowTop <= lpPFormat->RowsMax - 1);

return (fRet);
}


BOOL SS_PrintDrawPageOfCols(LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                            LPRECT lpRectBand, SS_PRINTFUNC lpfnPrintCallBack,
                            HFONT hFont)
{
BOOL fRet;

/*************************
* Draw each page of cols
*************************/

lpPFormat->ColLeft = lpPFormat->ColLeftOrig;
lpPFormat->ColAt = lpPFormat->ColLeft;

do
   {
   if (!(lpPFormat->pf.dPrintType & SS_PRINTTYPE_OVERTHENDOWN))
      SetRectEmpty(lpRect);

   lpPFormat->ColAt = SS_PrintCalcCols(lpPFormat->lpSS, lpPFormat, lpRect,
                                       lpPFormat->ColAt);

   if (!(lpPFormat->pf.dPrintType & SS_PRINTTYPE_OVERTHENDOWN))
      fRet = SS_PrintDrawPageOfRows(lpPFormat, lpRect, lpRectBand,
                                    lpfnPrintCallBack, hFont);
   else
      fRet = SS_PrintDrawPage(lpPFormat, lpRect, lpRectBand,
                              lpfnPrintCallBack, hFont);

   if (fRet != TRUE)
      break;

   lpPFormat->ColLeft = lpPFormat->ColAt;
   } while (lpPFormat->ColLeft <= lpPFormat->ColsMax - 1);

return (fRet);
}


BOOL SS_PrintDrawPage(LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                      LPRECT lpRectBand, SS_PRINTFUNC lpfnPrintCallBack,
                      HFONT hFont)
{
RECT  RectUpdate;
short dRet;
BOOL  fRet = TRUE;
int   DCState1;

/****************
* Draw the page
****************/

if (lpPFormat->wAction != SS_PRINT_ACTION_PAGECALC)
   {
   _fmemcpy(&RectUpdate, lpRect, sizeof(RECT));

	/* RFW - 6/2/04 - 14268
   if (!(lpPFormat->pf.dPrintType & SS_PRINTTYPE_PAGERANGE) ||
       lpPFormat->lPage >= lpPFormat->pf.nPageStart ||
       -1 == lpPFormat->pf.nPageStart)
	*/

   if (!(lpPFormat->pf.dPrintType & SS_PRINTTYPE_PAGERANGE) ||
       ((lpPFormat->lPage >= lpPFormat->pf.nPageStart ||
         -1 == lpPFormat->pf.nPageStart) && 
        (lpPFormat->lPage <= lpPFormat->pf.nPageEnd ||
         -1 == lpPFormat->pf.nPageEnd)))
      {
      RECT RectPrint;

      DCState1 = SaveDC(lpPFormat->hDC);
      SetWindowOrgEx(lpPFormat->hDC, 0, 0, NULL);

#ifdef SS_V30
      CopyRect(&RectPrint, &RectUpdate);
#else
      SetRect(&RectPrint,
              SS_SCREEN2PRINTERX(lpPFormat, RectUpdate.left),
              SS_SCREEN2PRINTERY(lpPFormat, RectUpdate.top),
              SS_SCREEN2PRINTERX(lpPFormat, RectUpdate.right),
              SS_SCREEN2PRINTERY(lpPFormat, RectUpdate.bottom));
#endif

      OffsetRect(&RectPrint, lpPFormat->x, lpPFormat->y);

#ifdef SS_V70
		if (lpPFormat->pf.fCenterOnPageH)
			OffsetRect(&RectPrint,
                    (lpPFormat->xPrintArea - (RectPrint.right - RectPrint.left)) / 2,
                    0);

		if (lpPFormat->pf.fCenterOnPageV)
			OffsetRect(&RectPrint, 0,
                    (lpPFormat->yPrintArea - (RectPrint.bottom - RectPrint.top)) / 2);
#endif // SS_V70

      /********************************************************
      * Check to see if the spreadsheet is in virtual mode.
      * If so check to see if more data needs to be requested
      * from the application
      ********************************************************/

// SPRD011 Wei Feng 1997/9/5
#ifndef SPREAD_JPN
      #ifndef SS_NOVIRTUAL
      {
      LPSPREADSHEET lpSS = lpPFormat->lpSS;
      
      if (lpSS->fVirtualMode && lpSS->Virtual.VMax != 0 && lpSS->Row.Max > 0 &&
          !lpPFormat->fAtVirtualEnd)
         {
         while ((lpPFormat->RowTop < lpSS->Virtual.VTop ||
                lpPFormat->RowAt >= lpSS->Virtual.VTop +
                lpSS->Virtual.VPhysSize) && !lpPFormat->fAtVirtualEnd)
            {
            lpSS->Row.UL = lpPFormat->RowTop;
            lpPFormat->fAtVirtualEnd = SS_VQueryData(lpSS, lpPFormat->RowAt);
            lpPFormat->RowAt = SS_PrintCalcRows(lpPFormat->lpSS, lpPFormat,
                                                lpRect, lpPFormat->RowTop, FALSE);
            SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);

            if (lpSS->Virtual.VMax && lpSS->Virtual.VTop +
                lpSS->Virtual.VPhysSize - lpSS->Row.HeaderCnt >= lpSS->Virtual.VMax)
               lpPFormat->fAtVirtualEnd = TRUE;
            }
         }
      }
      #endif
#endif
      
      if (fRet && lpPFormat->wAction == SS_PRINT_ACTION_PRINT)
         {
         if (StartPage(lpPFormat->hDC) <= 0)
            fRet = -1;
         }

      /******************
      * Perform Banding
      ******************/

      if (fRet == TRUE)
         {
         while (!lpPFormat->fNextBandDevice || ((dRet = Escape(lpPFormat->hDC,
                NEXTBAND, 0, NULL, (LPVOID)lpRectBand)) > 0 &&
                !IsRectEmpty(lpRectBand)))
            {
            fRet = SS_PrintPage(lpPFormat->hWnd, lpPFormat->hDC, lpPFormat->lpSS,
                                lpPFormat, lpRectBand, &RectPrint, &RectUpdate,
                                lpfnPrintCallBack, lpPFormat->lPage,
                                lpPFormat->lAppData, hFont, lpPFormat->ColAt,
                                lpPFormat->RowAt);

            if (!lpPFormat->fNextBandDevice)
               break;
            }

         if (lpPFormat->wAction == SS_PRINT_ACTION_PRINT)
            {
            int rc;

            rc = EndPage(lpPFormat->hDC);

            if (rc < (lpPFormat->fNextBandDevice ? -1 : 0))
               fRet = -1;
            }
         }

      SetViewportOrgEx(lpPFormat->hDC, lpPFormat->x, lpPFormat->y, NULL);
      RestoreDC(lpPFormat->hDC, DCState1);
      }
   }

if (fRet == TRUE)
   {
   lpPFormat->lPage++;

   if (lpPFormat->wAction != SS_PRINT_ACTION_PAGECALC &&
       (lpPFormat->pf.dPrintType & SS_PRINTTYPE_PAGERANGE) &&
       lpPFormat->lPage > lpPFormat->pf.nPageEnd &&
       -1 != lpPFormat->pf.nPageEnd)
      fRet = FALSE;

   /* RFW - KEM16 - 5/17/99
   else if (lpPFormat->fAtVirtualEnd &&
            lpPFormat->RowAt >= lpPFormat->lpSS->Virtual.VTop +
            lpPFormat->lpSS->Virtual.VPhysSize)
      fRet = FALSE;
   */
   }

return (fRet);
}


BOOL CALLBACK SS_PrintAbortProc(HDC hPrnDC, int iCode)
{
MSG Msg;

while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
   {
   TranslateMessage(&Msg);
   DispatchMessage(&Msg);
   }

return (TRUE);
}


void SS_PrintCalcPrinter(HDC hDC, LPSSX_PRINTFORMAT lpPFormat,
                         LPRECT lpRectPage)
{
short dMinOffsetFactor = 16;           // Allow a 1/16" for default margin
float fTemp;

lpPFormat->dfScalePreviewX = 1.0;
lpPFormat->dfScalePreviewY = 1.0;

lpPFormat->dHorzPrinterRes = GetDeviceCaps(hDC, HORZRES);
lpPFormat->dVertPrinterRes = GetDeviceCaps(hDC, VERTRES);

lpPFormat->TextWidthPrinter = GetDeviceCaps(hDC, LOGPIXELSX);
lpPFormat->TextHeightPrinter = GetDeviceCaps(hDC, LOGPIXELSY);

/************************************************
* Calculate printable area - The multiplication
* has been divided into multiple lines because
* of an MSC 6.0 compiler bug
************************************************/

if (lpPFormat->pf.fMarginLeft != 0)
   {
   fTemp = (float)lpPFormat->TextWidthPrinter;
   fTemp *= lpPFormat->pf.fMarginLeft;
   lpPFormat->x = (short)fTemp;
   }
else
   lpPFormat->x = lpPFormat->TextWidthPrinter / dMinOffsetFactor;

if (lpPFormat->pf.fMarginRight != 0)
   {
   fTemp = (float)lpPFormat->TextWidthPrinter;
   fTemp *= lpPFormat->pf.fMarginRight;
   }
else
   fTemp = (float)(lpPFormat->TextWidthPrinter / dMinOffsetFactor);

lpPFormat->xPrintArea = lpPFormat->dHorzPrinterRes - (short)fTemp -
                        lpPFormat->x;
#ifdef SS_V30
lpPFormat->cx = lpPFormat->xPrintArea;
#else
lpPFormat->cx = SS_PRINTER2SCREENX(lpPFormat, lpPFormat->xPrintArea);
#endif

if (lpPFormat->pf.fMarginTop != 0)
   {
   fTemp = (float)lpPFormat->TextHeightPrinter;
   fTemp *= lpPFormat->pf.fMarginTop;
   lpPFormat->y = (short)fTemp;
   }
else
   lpPFormat->y = lpPFormat->TextHeightPrinter / dMinOffsetFactor;

lpPFormat->y += lpPFormat->AppHeaderLenOrig;

if (lpPFormat->pf.fMarginBottom != 0)
   {
   fTemp = (float)lpPFormat->TextHeightPrinter;
   fTemp *= lpPFormat->pf.fMarginBottom;
   }
else
   fTemp = (float)(lpPFormat->TextHeightPrinter / dMinOffsetFactor);

lpPFormat->yPrintArea = lpPFormat->dVertPrinterRes - (short)fTemp -
                        lpPFormat->y - lpPFormat->AppFooterLenOrig;
#ifdef SS_V30
lpPFormat->cy = lpPFormat->yPrintArea;
#else
lpPFormat->cy = SS_PRINTER2SCREENY(lpPFormat, lpPFormat->yPrintArea);
#endif

// This is somewhat of a cludge to get around a problems when OwnerPrint is used and the
// device is not a printer.
lpPFormat->dHorzPrinterRes = max(lpPFormat->dHorzPrinterRes, lpPFormat->xPrintArea + lpPFormat->x);
lpPFormat->dVertPrinterRes = max(lpPFormat->dVertPrinterRes, lpPFormat->yPrintArea + lpPFormat->y);

SetRect(lpRectPage, 0, 0, lpPFormat->dHorzPrinterRes,
        lpPFormat->dVertPrinterRes);

if (lpPFormat->wAction == SS_PRINT_ACTION_PREVIEW)
   {
   lpPFormat->PrintInfo.fIsPreview = TRUE;
   lpPFormat->PrintInfo.hDCPrinter = hDC;
   lpPFormat->PrintInfo.RectPreview = lpPFormat->RectPreview;
   SetRect(&lpPFormat->PrintInfo.RectPrinter, lpPFormat->x,
           lpPFormat->y - lpPFormat->AppHeaderLenOrig,
           lpPFormat->x + lpPFormat->xPrintArea,
           lpPFormat->y + lpPFormat->yPrintArea - lpPFormat->AppHeaderLenOrig -
                          lpPFormat->AppFooterLenOrig);
   }

if (lpPFormat->wAction == SS_PRINT_ACTION_PREVIEW)
   {
   lpPFormat->dfScalePreviewX = (double)(lpPFormat->RectPreview.right -
                                         lpPFormat->RectPreview.left) /
                                         (double)lpPFormat->dHorzPrinterRes;
   lpPFormat->dfScalePreviewY = (double)(lpPFormat->RectPreview.bottom -
                                         lpPFormat->RectPreview.top) /
                                         (double)lpPFormat->dVertPrinterRes;

   lpPFormat->x = (short)((double)lpPFormat->x * lpPFormat->dfScalePreviewX);
   lpPFormat->y = (short)((double)lpPFormat->y * lpPFormat->dfScalePreviewY);
   lpPFormat->xPrintArea = (short)((double)lpPFormat->xPrintArea *
                           lpPFormat->dfScalePreviewX);
   lpPFormat->yPrintArea = (short)((double)lpPFormat->yPrintArea *
                           lpPFormat->dfScalePreviewY);
   lpRectPage->right = (short)((double)lpRectPage->right *
                       lpPFormat->dfScalePreviewX);
   lpRectPage->bottom = (short)((double)lpRectPage->bottom *
                        lpPFormat->dfScalePreviewY);
   lpPFormat->AppHeaderLen = (short)((double)lpPFormat->AppHeaderLenOrig *
                                     lpPFormat->dfScalePreviewY);
   lpPFormat->AppFooterLen = (short)((double)lpPFormat->AppFooterLenOrig *
                                     lpPFormat->dfScalePreviewY);
   lpPFormat->cx = lpPFormat->xPrintArea;
   lpPFormat->cy = lpPFormat->yPrintArea;
   }
}


HDC SS_OpenPrinter(void)
{
TCHAR  szPrinter[PRINTERINFO_MAXSIZE];
LPTSTR szDevice;
LPTSTR szDriver;
LPTSTR szOutput;
HDC    hDC = 0;

GetProfileString(_T("Windows"), _T("device"), _T(",,,"), szPrinter, PRINTERINFO_MAXSIZE);

szDevice = _ftcstok(szPrinter, _T(",\0"));
//szDriver = _ftcstok(NULL, _T(", \0"));
//szOutput = _ftcstok(NULL, _T(", \0"));
szDriver = _ftcstok(NULL, _T(",\0"));
szOutput = _ftcstok(NULL, _T(",\0"));

if (szDevice && szDriver && szOutput)
   hDC = CreateDC(szDriver, szDevice, szOutput, NULL);

return (hDC);
}


void SS_PrintInit(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat,
                  LPSS_PAGEFORMAT lpPageFormat, WORD wAction,
                  LPRECT lpRectPreview)
{
LPSS_CELL lpCellTemp;
SS_COORD  ColsMaxTemp;
SS_COORD  i;
SS_COORD  j;
HDC       hDCScreen;
BOOL      fOverflow;

_fmemcpy(&lpPFormat->pf, lpPageFormat, sizeof(SS_PAGEFORMAT));
if (lpPFormat->pf.nPageStart != -1)
	lpPFormat->pf.nPageStart = max(1, lpPFormat->pf.nPageStart);

lpPFormat->wAction = wAction;

lpPFormat->PrintInfo.hWndSpread = lpSS->lpBook->hWnd;
lpPFormat->PrintInfo.lXtra = 0;

if (lpRectPreview)
   CopyRect(&lpPFormat->RectPreview, lpRectPreview);

#ifdef SS_V70
	if (lpPFormat->pf.fSmartPrint)
		lpPFormat->pf.wScalingMethod = SS_PRINTSCALINGMETHOD_SMARTPRINT;
#endif // SS_V70

/*******************************
* Get information about screen
*******************************/

hDCScreen = SS_GetDC(lpSS->lpBook);

lpPFormat->TextWidthScreen = GetDeviceCaps(hDCScreen, LOGPIXELSX);
lpPFormat->TextHeightScreen = GetDeviceCaps(hDCScreen, LOGPIXELSY);

SS_ReleaseDC(lpSS->lpBook, hDCScreen);

/******************************
* Determine max rows and cols
******************************/

if (lpPFormat->pf.fUseDataMax && !lpSS->fVirtualMode)
   {
#ifdef SS_V40
	SS_COORD lrCol, lrRow;
#endif // SS_V40
	
   lpPFormat->ColsMax = max(lpSS->Col.DataCnt, lpSS->Col.HeaderCnt);
   lpPFormat->RowsMax = max(lpSS->Row.DataCnt, lpSS->Row.HeaderCnt);

   /*****************************************************************
   * If CellOverflow is on, the far right columns that contain data
   * may extend more than one column furthur to the right.  This
   * code will include the cells that have text overflowing onto
   * them when the fUseDataMax flag is TRUE.
   *****************************************************************/

   if (lpSS->lpBook->fAllowCellOverflow)
      {
      if (lpPFormat->ColsMax < SS_GetColCnt(lpSS) - 1)
         {
         ColsMaxTemp = lpPFormat->ColsMax;

         for (i = 0; i < SS_GetRowCnt(lpSS); i++)
            for (j = ColsMaxTemp; j < SS_GetColCnt(lpSS); j++)
               {
               fOverflow = FALSE;

               if (lpCellTemp = SS_LockCellItem(lpSS, j, i))
                  {
                  if ((lpCellTemp->Data.bOverflow & SS_OVERFLOW_LEFT))
                     fOverflow = TRUE;

                  SS_UnlockCellItem(lpSS, j, i);
                  }

               if (fOverflow)
                  lpPFormat->ColsMax = max(lpPFormat->ColsMax, j + 1);
               else
                  break;
               }
         }
      }
#ifdef SS_V40
	if (SS_GetSpanLREx(lpSS, &lrCol, &lrRow, lpPFormat->ColsMax - 1, lpPFormat->RowsMax - 1))
		{
		lpPFormat->ColsMax = max(lpPFormat->ColsMax, lrCol + 1);
		lpPFormat->RowsMax = max(lpPFormat->RowsMax, lrRow + 1);
		}
#endif // SS_V40
   }

else
   {
   lpPFormat->ColsMax = SS_GetColCnt(lpSS);
   lpPFormat->RowsMax = SS_GetRowCnt(lpSS);
   }

if (lpPFormat->pf.dPrintType & SS_PRINTTYPE_CURRENTPAGE)
   {
   lpPFormat->ColLeft = lpSS->Col.UL;
   lpPFormat->RowTop = lpSS->Row.UL;
	/* RFW - 4/17/00 - GIC11571
   lpPFormat->ColRight = lpSS->Col.LRAllVis;
   lpPFormat->RowBottom = lpSS->Row.LRAllVis;
	*/
   lpPFormat->ColRight = SS_GetRightVisCell(lpSS, lpSS->Col.UL);
   lpPFormat->RowBottom = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);
   }

else if (lpPFormat->pf.dPrintType & SS_PRINTTYPE_CELLRANGE)
   {
	SS_COORD x1CellRange = lpPFormat->pf.x1CellRange;
   SS_COORD x2CellRange = lpPFormat->pf.x2CellRange;
   SS_COORD y1CellRange = lpPFormat->pf.y1CellRange;
   SS_COORD y2CellRange = lpPFormat->pf.y2CellRange;

	SS_AdjustCellCoords(lpSS, &x1CellRange, &y1CellRange);
	SS_AdjustCellCoords(lpSS, &x2CellRange, &y2CellRange);

   lpPFormat->ColLeft = max(lpSS->Col.HeaderCnt + lpSS->Col.Frozen, x1CellRange);

   if (x2CellRange <= 0)
      lpPFormat->ColRight = lpPFormat->ColsMax - 1;
   else
      lpPFormat->ColRight = x2CellRange;

   lpPFormat->RowTop = max(lpSS->Row.HeaderCnt + lpSS->Row.Frozen, y1CellRange);

   if (y2CellRange <= 0)
      lpPFormat->RowBottom = lpPFormat->RowsMax - 1;
   else
      lpPFormat->RowBottom = y2CellRange;
   }

else
   {
   lpPFormat->ColLeft = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
   lpPFormat->ColRight = lpPFormat->ColsMax - 1;
   lpPFormat->RowTop = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
   lpPFormat->RowBottom = lpPFormat->RowsMax - 1;
   }

lpPFormat->ColsMax = min(lpPFormat->ColsMax, lpPFormat->ColRight + 1);
lpPFormat->RowsMax = min(lpPFormat->RowsMax, lpPFormat->RowBottom + 1);
}


HDC SS_PrintSmartPrint(LPSPREADSHEET lpSS, HDC hDC,
                       LPSSX_PRINTFORMAT lpPFormat, LPTSTR szDevice,
                       LPTSTR szDriver, LPTSTR szOutput, LPRECT lpRectPage,
                       FARPROC lpfnAbortProc, LPBOOL lpbContinue, LPDEVMODE lpDevModeIn)
{
TBGLOBALHANDLE hPrintColWidthList;
long           *hpPrintColWidthList;
SS_COORD       i;
double         dfColWidth;
long           dMaxTextColWidth;
long           dColWidthTotal;
long           Width;
long           WidthScreen;
long           dWidthTemp;
HDC            hOldDC = hDC;
BOOL           bContinue = TRUE;

/**********************************************************
* Add the logic for determining if the spreadsheet should
* print in landscape and if it should scale itself
**********************************************************/

#ifdef SS_UTP
if (lpSS->fPrintExpandMultiLine)
   {
   HPSHORT hpPrintRowHeightList;
   double  dfRowHeight;
   short   dMaxTextRowHeight;
   short   dHeightTemp;

   if (lpSS->hPrintRowHeightList = tbGlobalAlloc(GMEM_MOVEABLE |
                                                 GMEM_ZEROINIT,
                                                 lpSS->Row.DataCnt *
                                                 sizeof(short)))
      {
      hpPrintRowHeightList = (HPSHORT)tbGlobalLock(
                                      lpSS->hPrintRowHeightList);

      for (i = 0; i < lpSS->Row.DataCnt; i++)
         {
         dHeightTemp = SS_GetRowHeightInPixelsEx(lpSS, i, lpPFormat);

         if (i >= lpSS->Row.HeaderCnt)
            {
            SSGetMaxTextRowHeight(hWnd, i, &dfRowHeight);
            dMaxTextRowHeight = SSRowHeightToLogUnits(hWnd, i,
                                                      dfRowHeight);

            if (dMaxTextRowHeight > dHeightTemp)
               dHeightTemp = dMaxTextRowHeight;
            }

         hpPrintRowHeightList[i] = dHeightTemp;
         }

      tbGlobalUnlock(lpSS->hPrintRowHeightList);
      lpSS->lPrintRowHeightListCnt = lpSS->Row.DataCnt;
      }
   }
#endif

for (WidthScreen = 0, i = 0; i < lpPFormat->ColsMax; i++)
#ifdef SS_UTP
   if (i != lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
#endif
      WidthScreen += SS_GetColWidthInPixels(lpSS, i);

#ifdef SS_V30
Width = SS_SCREEN2PRINTERX(lpPFormat, WidthScreen);
#else
Width = WidthScreen;
#endif

if (Width > lpPFormat->cx)
   {
   // Turn on landscape mode

   if ((lpDevModeIn && lpDevModeIn->dmOrientation != DMORIENT_LANDSCAPE) ||
       (!lpDevModeIn && lpPFormat->pf.wOrientation != SS_PRINTORIENT_LANDSCAPE))
      {
      hDC = SS_PrintSetOrientation(lpSS, hDC, lpPFormat, szDevice,
                                   szDriver, szOutput, lpRectPage,
                                   SS_PRINTORIENT_LANDSCAPE, lpDevModeIn, FALSE);

#ifdef SS_V30
      Width = SS_SCREEN2PRINTERX(lpPFormat, WidthScreen);
#else
      Width = WidthScreen;
#endif
      }

#if (defined(SS_VB) || defined(SS_OCX))
   vbSSReplacePrintAbortItem(lpSS->lpBook->hWnd, hOldDC, hDC);
#endif

   // If the text will not fit on the page in landscape mode,
   // scale the image up to SCALE_LIMIT.  If it still doesnt
   // fit, then try resizing the columns

   if (Width > lpPFormat->cx)
      {
      if ((double)lpPFormat->cx / (double)Width >= SCALE_LIMIT)
         {
         lpPFormat->dfScale = (double)lpPFormat->cx / (double)Width;
         SS_PrintCalcPrinter(hDC, lpPFormat, lpRectPage);
         }
      else
         {
         if (hPrintColWidthList = tbGlobalAlloc(GMEM_MOVEABLE |
                                                GMEM_ZEROINIT,
                                                lpPFormat->ColsMax *
                                                sizeof(long)))
            {
            hpPrintColWidthList = (long*)tbGlobalLock(hPrintColWidthList);
            dColWidthTotal = 0;

            for (i = 0; bContinue && i < lpPFormat->ColsMax; i++)
               {
#ifdef SS_UTP
               if (i == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
                  dWidthTemp = 0;
               else
#endif
                  dWidthTemp = SS_GetColWidthInPixels(lpSS, i);

               if (i >= lpSS->Col.HeaderCnt)
                  {
                  SS_GetMaxTextColWidth(lpSS, i, &dfColWidth);
						if (dfColWidth > 0)
							dMaxTextColWidth = SSColWidthToLogUnits(lpSS->lpBook->hWnd,
									                                  dfColWidth) + 1;
						else
							dMaxTextColWidth = 0;

                  if (dMaxTextColWidth < dWidthTemp)
                     dWidthTemp = dMaxTextColWidth;
                  }

               hpPrintColWidthList[i] = dWidthTemp;

               dColWidthTotal += hpPrintColWidthList[i];
               
               if (lpfnAbortProc)
                  bContinue = ((ABORTPROC)lpfnAbortProc)(hDC,0);
               }

            tbGlobalUnlock(hPrintColWidthList);

				// RFW - 4/29/02 - 10017
            if (SS_SCREEN2PRINTERX(lpPFormat, dColWidthTotal) <= lpPFormat->cx)
               {
               lpSS->lPrintColWidthListCnt = lpPFormat->ColsMax;
               lpSS->hPrintColWidthList = hPrintColWidthList;
               }
            else
					// Everything failed, let's reset.
					{
               tbGlobalFree(hPrintColWidthList);

					// RFW - 9/2/05 - 16805
					if ((lpDevModeIn && lpDevModeIn->dmOrientation != DMORIENT_LANDSCAPE) ||
						 (!lpDevModeIn && lpPFormat->pf.wOrientation != SS_PRINTORIENT_LANDSCAPE))
						hDC = SS_PrintSetOrientation(lpSS, hDC, lpPFormat, szDevice,
															  szDriver, szOutput, lpRectPage,
															  SS_PRINTORIENT_PORTRAIT, lpDevModeIn, TRUE);
					}
            }
         }
      }
   }

if (lpbContinue)
   *lpbContinue = bContinue;

return (hDC);
}


#ifdef SS_V70


BOOL SS_PrintTestHFit(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat, long lMaxPages)
{
SS_COORD ColAt;
long     lPages = 0;

ColAt = lpPFormat->ColLeft;

do
   {
   ColAt = SS_PrintCalcCols(lpSS, lpPFormat, NULL, ColAt);
	lPages++;
   } while (ColAt <= lpPFormat->ColsMax - 1 && lPages <= lMaxPages);

return (lPages <= lMaxPages);
}


BOOL SS_PrintTestVFit(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat, long lMaxPages)
{
SS_COORD RowAt;
long     lPages = 0;

RowAt = lpPFormat->RowTop;

do
   {
   RowAt = SS_PrintCalcRows(lpSS, lpPFormat, NULL, RowAt, TRUE);
	lPages++;
   } while (RowAt <= lpPFormat->RowsMax - 1 && lPages <= lMaxPages);

return (lPages <= lMaxPages);
}


BOOL SS_PrintBestFit(LPSPREADSHEET lpSS, HDC hDC, LPSSX_PRINTFORMAT lpPFormat,
                     LPRECT lpRectPage)
{
BOOL fRet = FALSE;

if (lpPFormat->pf.wScalingMethod == SS_PRINTSCALINGMETHOD_BESTFIT)
   {
	double   dfTolerance = 0.02;
	double   dfScaleOrig = lpPFormat->dfScale;
	/* RFW - 5/29/07 - 20312
	SS_COORD lPageBreakCnt = 0;
	LPSS_COL lpCol;
	SS_COORD ColAt;
	LPSS_ROW lpRow;
	SS_COORD RowAt;
	BOOL     fBreak;
	*/

	/* RFW - 5/29/07 - 20312
	// RFW - 2/5/07 - 19803
	for (ColAt = 0, fBreak = FALSE; ColAt < lpPFormat->ColsMax && !fBreak; ColAt++)
		{
		if (lpCol = SS_LockColItem(lpSS, ColAt))
			{
			if (lpCol->fPageBreak)
				{
				lPageBreakCnt++;
				if (lPageBreakCnt >= lpPFormat->pf.nBestFitPagesWide)
					{
					lpPFormat->ColsMax = ColAt;
					fBreak = TRUE;
					}
				}

			SS_UnlockColItem(lpSS, ColAt);
			}
		}
	*/

	if (!SS_PrintTestHFit(lpSS, lpPFormat, lpPFormat->pf.nBestFitPagesWide))
		{
		double dfHi = lpPFormat->dfScale;
		double dfLow = 0.0;
		double dfScaleOld;

		while (TRUE)
			{
			dfScaleOld = lpPFormat->dfScale;
			lpPFormat->dfScale = dfLow + ((dfHi - dfLow) / 2.0);

			if (SS_PrintTestHFit(lpSS, lpPFormat, lpPFormat->pf.nBestFitPagesWide))
				{
				if (fabs(lpPFormat->dfScale - dfScaleOld) <= dfTolerance)
					break;

				dfLow = lpPFormat->dfScale;
				}
			else
				dfHi = lpPFormat->dfScale;
			}
		}

	/* RFW - 5/29/07 - 20312
	// RFW - 2/5/07 - 19803
	lPageBreakCnt = 0;
	for (RowAt = 0, fBreak = FALSE; RowAt < lpPFormat->RowsMax && !fBreak; RowAt++)
		{
		if (lpRow = SS_LockRowItem(lpSS, RowAt))
			{
			if (lpRow->fPageBreak)
				{
				lPageBreakCnt++;
				if (lPageBreakCnt >= lpPFormat->pf.nBestFitPagesWide)
					{
					lpPFormat->RowsMax = RowAt;
					fBreak = TRUE;
					}
				}

			SS_UnlockRowItem(lpSS, RowAt);
			}
		}
	*/

	if (!SS_PrintTestVFit(lpSS, lpPFormat, lpPFormat->pf.nBestFitPagesTall))
		{
		double dfHi = lpPFormat->dfScale;
		double dfLow = 0.0;
		double dfScaleOld;

		while (TRUE)
			{
			dfScaleOld = lpPFormat->dfScale;
			lpPFormat->dfScale = dfLow + ((dfHi - dfLow) / 2.0);

			if (SS_PrintTestVFit(lpSS, lpPFormat, lpPFormat->pf.nBestFitPagesTall))
				{
				if (fabs(lpPFormat->dfScale - dfScaleOld) <= dfTolerance)
					break;

				dfLow = lpPFormat->dfScale;
				}
			else
				dfHi = lpPFormat->dfScale;
			}
		}

   if (dfScaleOrig != lpPFormat->dfScale)
      {
      SS_PrintCalcPrinter(hDC, lpPFormat, lpRectPage);
		fRet = TRUE;
      }
   }

return (fRet);
}

#if 0
BOOL SS_PrintBestFit(LPSPREADSHEET lpSS, HDC hDC, LPSSX_PRINTFORMAT lpPFormat,
                     LPRECT lpRectPage)
{
BOOL fRet = FALSE;

if (lpPFormat->pf.wScalingMethod == SS_PRINTSCALINGMETHOD_BESTFIT)
   {
	SS_COORD i;
	long     Width, Height;
	long     WidthScreen, HeightScreen;
	long     lDesiredWidth, lDesiredHeight;

   for (WidthScreen = 0, i = 0; i < lpPFormat->ColsMax; i++)
      WidthScreen += SS_GetColWidthInPixels(lpSS, i);

   for (HeightScreen = 0, i = 0; i < lpPFormat->RowsMax; i++)
      HeightScreen += SS_GetRowHeightInPixels(lpSS, i);

   Width = SS_SCREEN2PRINTERX(lpPFormat, WidthScreen);
   Height = SS_SCREEN2PRINTERY(lpPFormat, HeightScreen);

	lDesiredWidth = lpPFormat->cx * lpPFormat->pf.nBestFitPagesWide;
	lDesiredHeight = (lpPFormat->cy - 1) * lpPFormat->pf.nBestFitPagesTall;

   if (Width > lDesiredWidth || Height > lDesiredHeight)
      {
		lpPFormat->dfScale = min((double)lDesiredWidth / (double)Width,
				                   (double)lDesiredHeight / (double)Height);
      SS_PrintCalcPrinter(hDC, lpPFormat, lpRectPage);
		fRet = TRUE;
      }
   }

return (fRet);
}
#endif

#endif // SS_V70


HDC SS_PrintSetOrientation(LPSPREADSHEET lpSS, HDC hDC,
                           LPSSX_PRINTFORMAT lpPFormat, LPTSTR szDevice,
                           LPTSTR szDriver, LPTSTR szOutput, LPRECT lpRectPage,
                           WORD wPrintOrient, LPDEVMODE lpDevModeIn, BOOL fForceOrientation)
{
int dmOrientation = (wPrintOrient == SS_PRINTORIENT_PORTRAIT ?
                     DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE);

// Turn on landscape mode
if (wPrintOrient != SS_PRINTORIENT_DEFAULT)
   {
	if (lpDevModeIn)
		{
      if (lpDevModeIn->dmOrientation != dmOrientation)
         {
			/* RFW - 11/21/06 - 19624
         lpDevModeIn->dmFields = DM_ORIENTATION;
			*/
         lpDevModeIn->dmFields = DM_ORIENTATION | DM_PAPERSIZE;
         lpDevModeIn->dmOrientation = dmOrientation;

			// Read new printer values
			if (ResetDC(hDC, lpDevModeIn))
				{
				if (lpPFormat->dHorzPrinterRes != GetDeviceCaps(hDC, HORZRES))
					{
					SS_PrintCalcPrinter(hDC, lpPFormat, lpRectPage);
					return (hDC);
					}
				}
			}
		}

#if 0 // I took this out because of bug GRB7372.  A call to ResetDC
	   // really needs a DevMode structure set up with DocumentProperties
	   // and since I do not have the Device Name when the hDCPrinter is
	   // passed in, I cannot call DocumentProperties.

#ifdef SS_V30
   if (lpPFormat->pf.hDCPrinter)
      {
      DEVMODE DevMode;

      _fmemset(&DevMode, '\0', sizeof(DEVMODE));
      DevMode.dmSpecVersion = DM_SPECVERSION;
      DevMode.dmSize = sizeof(DEVMODE);
      DevMode.dmFields = DM_ORIENTATION;
      DevMode.dmOrientation = dmOrientation;

      // Read new printer values
      if (ResetDC(hDC, &DevMode))
         {
         if (lpPFormat->dHorzPrinterRes != GetDeviceCaps(hDC, HORZRES))
            {
            SS_PrintCalcPrinter(hDC, lpPFormat, lpRectPage);
            return (hDC);
            }
         }
      }

#endif // SS_V30
#endif

   else if (!lpPFormat->pf.hDCPrinter)
      { 
#ifdef WIN32
      TBGLOBALHANDLE hDevMode;
      LPDEVMODE      lpDevMode;
      short          dLen;
      DEVMODE        DevModeTemp;
      HANDLE         hPrinter;

      if (!OpenPrinter(szDevice, &hPrinter, NULL))
         return (HDC)NULL;
      
      _fmemset(&DevModeTemp, '\0', sizeof(DEVMODE));

#ifdef _UNICODE
      dLen = (short)DocumentProperties(lpSS->lpBook->hWnd, hPrinter, szDevice,
                                       NULL, (LPDEVMODE)NULL,(WORD)0);
#else
      dLen = (short)DocumentProperties(lpSS->lpBook->hWnd, hPrinter, szDevice,
                                       (LPDEVMODE)&DevModeTemp, (LPDEVMODE)NULL,(WORD)0);
#endif

      if (hDevMode = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   (long)dLen))
         {
         lpDevMode = (LPDEVMODE)tbGlobalLock(hDevMode);

         DocumentProperties(lpSS->lpBook->hWnd, hPrinter, (LPTSTR)szDevice, lpDevMode,
                            (LPDEVMODE)NULL, (WORD)DM_OUT_BUFFER);

         if (lpDevMode->dmOrientation != dmOrientation || fForceOrientation)
            {
//            lpDevMode->dmFields = DM_ORIENTATION;
            lpDevMode->dmFields |= DM_ORIENTATION;
            lpDevMode->dmOrientation = dmOrientation;

            DocumentProperties(lpSS->lpBook->hWnd, hPrinter, (LPTSTR)szDevice, lpDevMode,
                               lpDevMode, (WORD)(DM_IN_BUFFER | DM_OUT_BUFFER));

				// RFW - 4/8/04 - This fixes a problem with some drivers.
				if ((lpDevMode->dmFields & DM_COPIES) && lpDevMode->dmCopies == 0)
					lpDevMode->dmCopies = 1;

            if (!lpPFormat->pf.hDCPrinter)
               DeleteDC(hDC);

            hDC = CreateDC(szDriver, szDevice, NULL, lpDevMode);

            // Read new printer values
            SS_PrintCalcPrinter(hDC, lpPFormat, lpRectPage);
            }

         tbGlobalUnlock(hDevMode);
         tbGlobalFree(hDevMode);
         }

      ClosePrinter(hPrinter);
#else  // !defined(WIN32)
      TBGLOBALHANDLE hDevMode;
      LPDEVMODE      lpDevMode;
      FARPROC        lpfn;
      HANDLE         hDrv;
      short          dLen;
      TCHAR          sz[50];

      lstrcpy(sz, szDriver);
      lstrcat(sz, _T(".drv"));
      if ((UINT)(hDrv = LoadLibrary(sz)) >= 32)
         {
         if (lpfn = GetProcAddress(hDrv, "ExtDeviceMode"))
            {
            DEVMODE DevModeTemp;

            _fmemset(&DevModeTemp, '\0', sizeof(DEVMODE));

            dLen = (short)(*lpfn)(hWnd, hDrv, (LPDEVMODE)&DevModeTemp,
                                (LPTSTR)szDevice, (LPTSTR)szOutput, (LPDEVMODE)NULL,
                                (LPTSTR)NULL, (WORD)0);

            if (hDevMode = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                         (long)dLen))
               {
               lpDevMode = (LPDEVMODE)tbGlobalLock(hDevMode);

               (*lpfn)(hWnd, hDrv, lpDevMode, (LPTSTR)szDevice,
                       (LPTSTR)szOutput, (LPDEVMODE)NULL, (LPTSTR)NULL,
                       (WORD)DM_OUT_BUFFER);

               if (lpDevMode->dmOrientation != dmOrientation)
                  {
                  DEVMODE DevModeNew;
                  int     dmOrientationSave = lpDevMode->dmOrientation;

                  _fmemset(&DevModeNew, '\0', sizeof(DEVMODE));
                  lstrcpy(DevModeNew.dmDeviceName, szDevice);
                  DevModeNew.dmSpecVersion = DM_SPECVERSION;
                  DevModeNew.dmSize = sizeof(DEVMODE);
                  DevModeNew.dmFields = DM_ORIENTATION;
                  DevModeNew.dmOrientation = dmOrientation;

                  (*lpfn)(hWnd, hDrv, lpDevMode, (LPTSTR)szDevice,
                          (LPTSTR)szOutput, (LPDEVMODE)&DevModeNew,
                          (LPTSTR)NULL, (WORD)(DM_MODIFY | DM_COPY | DM_UPDATE));

                  if (!lpPFormat->pf.hDCPrinter)
                     DeleteDC(hDC);

                  hDC = CreateDC(szDriver, szDevice, szOutput, lpDevMode);


                  _fmemset(&DevModeNew, '\0', sizeof(DEVMODE));
                  lstrcpy(DevModeNew.dmDeviceName, szDevice);
                  DevModeNew.dmSpecVersion = DM_SPECVERSION;
                  DevModeNew.dmSize = sizeof(DEVMODE);
                  DevModeNew.dmFields = DM_ORIENTATION;
                  DevModeNew.dmOrientation = dmOrientationSave;

                  (*lpfn)(hWnd, hDrv, lpDevMode, (LPTSTR)szDevice,
                          (LPTSTR)szOutput, (LPDEVMODE)&DevModeNew,
                          (LPTSTR)NULL, (WORD)(DM_MODIFY | DM_COPY | DM_UPDATE));

                  // Read new printer values

                  SS_PrintCalcPrinter(hDC, lpPFormat, lpRectPage);
                  }

               tbGlobalUnlock(hDevMode);
               tbGlobalFree(hDevMode);
               }
            }

         FreeLibrary(hDrv);
         }
#endif  // defined(WIN32)

      }
   }

return (hDC);
}


SS_COORD SS_PrintCalcCols(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat,
                          LPRECT lpRect, SS_COORD ColAt)
{
LPSS_COL lpCol;
SS_COORD ColAtOld;
SS_COORD ColHeaderAt;
SS_COORD TempColAt;
double   dfColPosAt;
double   dfWidth;
BOOL     fBreak;

/*********************
* Calc width of cols
*********************/

dfColPosAt = -1;
ColHeaderAt = 0;
if (lpRect)
	lpRect->right = 0;
ColAtOld = ColAt;
fBreak = FALSE;

do
   {
   if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      TempColAt = ColHeaderAt;
   else
      TempColAt = ColAt;

#ifdef SS_UTP
   if (TempColAt == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
       dfWidth = 0;
   else
#endif
      {
      dfWidth = (double)SS_GetColWidthInPixels(lpSS, TempColAt);
#ifdef SS_V30
      dfWidth = SS_DFSCREEN2PRINTERX(lpPFormat, dfWidth);
#endif
      }

   if ((int)(dfColPosAt + dfWidth) >= lpPFormat->cx && ColAt > ColAtOld)
      break;

   if (lpPFormat->pf.fShowRowHeaders || ColHeaderAt >= lpSS->Col.HeaderCnt)
      {
      dfColPosAt += dfWidth;
		if (lpRect)
			lpRect->right = (int)dfColPosAt + 1;
      }

   if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      ColHeaderAt++;
   else
      {
      if (lpCol = SS_LockColItem(lpSS, ColAt + 1))
         {
         if (lpCol->fPageBreak && SS_PROCESS_PAGEBREAKS(lpPFormat))
            fBreak = TRUE;

         SS_UnlockColItem(lpSS, ColAt + 1);
         }

      ColAt++;

      if (fBreak)
         break;
      }

	// RFW - -7/28/04 - 14885
   } while (TempColAt < lpPFormat->ColsMax - 1);

if (lpRect)
	lpRect->right = min(lpRect->right, lpPFormat->cx);

return (ColAt);
}


SS_COORD SS_PrintCalcRows(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat,
                          LPRECT lpRect, SS_COORD RowAt, BOOL fIgnorePageBreaks)
{
LPSS_ROW lpRow;
SS_COORD RowAtOld;
SS_COORD RowHeaderAt;
SS_COORD TempRowAt;
double   dfRowPosAt;
double   dfHeight;
BOOL     fBreak;

/**********************
* Calc height of rows
**********************/

dfRowPosAt = -1;
RowHeaderAt = 0;
if (lpRect)
	lpRect->bottom = 0;
RowAtOld = RowAt;
fBreak = FALSE;

do
   {
   if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      TempRowAt = RowHeaderAt;
   else
      {
      TempRowAt = RowAt;

// SPRD011 Wei Feng
#ifndef	SPREAD_JPN
      // BJO 22Mar96 SEL3492 - Begin fix
      #ifndef SS_NOVIRTUAL
      if (lpSS->fVirtualMode && lpSS->Virtual.VMax != 0 && lpSS->Row.Max > 0 &&
          !lpPFormat->fAtVirtualEnd)
         {
         while ((lpPFormat->RowTop < lpSS->Virtual.VTop ||
                TempRowAt >= lpSS->Virtual.VTop +
                lpSS->Virtual.VPhysSize) && !lpPFormat->fAtVirtualEnd)
            {
            lpSS->Row.UL = lpPFormat->RowTop;
            lpPFormat->fAtVirtualEnd = SS_VQueryData(lpSS, TempRowAt);
            SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
            if (lpSS->Virtual.VMax != -1 && lpSS->Virtual.VTop +
                lpSS->Virtual.VPhysSize - lpSS->Row.HeaderCnt >= lpSS->Virtual.VMax)
               lpPFormat->fAtVirtualEnd = TRUE;
            }
         }
      #endif
      // BJO 22Mar96 SEL3492 - End fix
#endif
      }

   dfHeight = (double)SS_GetRowHeightInPixels(lpSS, TempRowAt);
#ifdef SS_V30
   dfHeight = SS_DFSCREEN2PRINTERY(lpPFormat, dfHeight);
#endif

   if ((int)(dfRowPosAt + dfHeight) >= lpPFormat->cy && RowAt > RowAtOld)
      break;

   if (lpPFormat->pf.fShowColHeaders || RowHeaderAt >= lpSS->Row.HeaderCnt)
      {
      dfRowPosAt += dfHeight;
		if (lpRect)
			lpRect->bottom = (int)dfRowPosAt + 1;
      }

   if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      RowHeaderAt++;
   else
      {
		if (!fIgnorePageBreaks)
			{
			if (lpRow = SS_LockRowItem(lpSS, TempRowAt + 1))
				{
				if (lpRow->fPageBreak && SS_PROCESS_PAGEBREAKS(lpPFormat))
					fBreak = TRUE;

				SS_UnlockRowItem(lpSS, TempRowAt + 1);
				}
			}

      RowAt++;

      if (fBreak)
         break;
      }

   } while (TempRowAt < lpPFormat->RowsMax - 1);

if (lpRect)
	lpRect->bottom = min(lpRect->bottom, lpPFormat->cy);

return (RowAt);
}


void SS_PrintBorder(HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRectPrint,
                    LPRECT lpRectBand)
{
RECT RectTemp;
RECT RectDest;

/**************
* Draw Border
**************/

if (lpPFormat->pf.fDrawBorder)
   {
   SetViewportOrgEx(hDC, 0, 0, NULL);

   // BJO 23Mar96 SEL3492 - The right and bottom borders were off by
   //                       a pixel.

   /******************
   * Draw top border
   ******************/

   if (lpRectBand->top <= lpRectPrint->top &&
       lpRectBand->bottom >= lpRectPrint->top)
      {
      MoveToEx(hDC, lpRectPrint->left, lpRectPrint->top, NULL);
      //LineTo(hDC, lpRectPrint->right - 1, lpRectPrint->top);
      SS_PrintLineTo(hDC, lpRectPrint->right, lpRectPrint->top, lpPFormat);
      }

   /*******************
   * Draw left border
   *******************/

   SetRect(&RectTemp, lpRectPrint->left, lpRectPrint->top,
           lpRectPrint->left + 1, lpRectPrint->bottom);
   if (IntersectRect(&RectDest, lpRectBand, &RectTemp))
      {
      MoveToEx(hDC, lpRectPrint->left, max(lpRectBand->top,
               lpRectPrint->top), NULL);
      //LineTo(hDC, lpRectPrint->left, min(lpRectBand->bottom,
      //       lpRectPrint->bottom));
      SS_PrintLineTo(hDC, lpRectPrint->left, min(lpRectBand->bottom,
                     lpRectPrint->bottom + 1), lpPFormat);
      }

   /********************
   * Draw right border
   ********************/

   SetRect(&RectTemp, lpRectPrint->right - 1, lpRectPrint->top,
           lpRectPrint->right, lpRectPrint->bottom);
   if (IntersectRect(&RectDest, lpRectBand, &RectTemp))
      {
      //MoveToEx(hDC, lpRectPrint->right - 1, max(lpRectBand->top,
      //         lpRectPrint->top), NULL);
      //LineTo(hDC, lpRectPrint->right - 1,
      //       min(lpRectBand->bottom, lpRectPrint->bottom));
      MoveToEx(hDC, lpRectPrint->right, max(lpRectBand->top,
               lpRectPrint->top), NULL);
      SS_PrintLineTo(hDC, lpRectPrint->right,
                     min(lpRectBand->bottom, lpRectPrint->bottom + 1),
                     lpPFormat);
      }

   /*********************
   * Draw bottom border
   *********************/

   if (lpRectBand->top <= lpRectPrint->bottom &&
       lpRectBand->bottom >= lpRectPrint->bottom)
      {
      //MoveToEx(hDC, lpRectPrint->left, lpRectPrint->bottom - 1, NULL);
      //LineTo(hDC, lpRectPrint->right - 1,
      //       lpRectPrint->bottom - 1);
      MoveToEx(hDC, lpRectPrint->left, lpRectPrint->bottom, NULL);
      SS_PrintLineTo(hDC, lpRectPrint->right, lpRectPrint->bottom, lpPFormat);
      }

   SetViewportOrgEx(hDC, lpPFormat->x + 1, lpPFormat->y + 1, NULL);
   }

else
   SetViewportOrgEx(hDC, lpPFormat->x, lpPFormat->y, NULL);
}


#if 0
#pragma pack(1)
typedef struct tagPRECT_STRUCT
   {
   POINT ptPosition;
   POINT ptSize;
   WORD  wStyle;
   WORD  wPattern;
   } PRECT_STRUCT, FAR *LPPRECT_STRUCT;
#pragma pack()

void SS_PrintDrawLine(HDC hDC, int iLeft, int iTop, int iWidth, int iHeight)
{
int iEscNum = DRAWPATTERNRECT;

if (TRUE || Escape(hDC, QUERYESCSUPPORT, sizeof(iEscNum), (LPCSTR)&iEscNum, NULL))
   {
   PRECT_STRUCT InData;

   InData.ptPosition.x = iLeft;
   InData.ptPosition.y = iTop;
   InData.ptSize.x = iWidth;
   InData.ptSize.y = iHeight;
   InData.wStyle = 0;
   InData.wPattern = 0;

   Escape(hDC, DRAWPATTERNRECT, sizeof(PRECT_STRUCT), (LPCSTR)&InData, NULL);
   }
else
   {
   MoveToEx(hDC, iLeft, iTop, NULL);
   LineTo(hDC, iWidth == 1 ? iLeft : iLeft + iWidth,
               iHeight == 1 ? iTop : iTop + iHeight);
//   PatBlt(hDC, iLeft, iTop, iWidth, iHeight, BLACKNESS);
   }
}


void SS_PrintBorder(HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRectPrint,
                    LPRECT lpRectBand)
{
RECT RectTemp;
RECT RectDest;

/**************
* Draw Border
**************/

if (lpPFormat->pf.fDrawBorder)
   {
   SetViewportOrgEx(hDC, 0, 0, NULL);

   // BJO 23Mar96 SEL3492 - The right and bottom borders were off by
   //                       a pixel.

   /******************
   * Draw top border
   ******************/

   if (lpRectBand->top <= lpRectPrint->top &&
       lpRectBand->bottom >= lpRectPrint->top)
      {
      SS_PrintDrawLine(hDC, lpRectPrint->left, lpRectPrint->top,
                       lpRectPrint->right - lpRectPrint->left, 1);
      }

   /*******************
   * Draw left border
   *******************/

   SetRect(&RectTemp, lpRectPrint->left, lpRectPrint->top,
           lpRectPrint->left + 1, lpRectPrint->bottom);
   if (IntersectRect(&RectDest, lpRectBand, &RectTemp))
      {
      SS_PrintDrawLine(hDC, lpRectPrint->left, max(lpRectBand->top, lpRectPrint->top),
                       1, min(lpRectBand->bottom, lpRectPrint->bottom + 1) -
                       max(lpRectBand->top, lpRectPrint->top));
      }

   /********************
   * Draw right border
   ********************/

   SetRect(&RectTemp, lpRectPrint->right - 1, lpRectPrint->top,
           lpRectPrint->right, lpRectPrint->bottom);
   if (IntersectRect(&RectDest, lpRectBand, &RectTemp))
      {
      SS_PrintDrawLine(hDC, lpRectPrint->right, max(lpRectBand->top, lpRectPrint->top),
                       1, min(lpRectBand->bottom, lpRectPrint->bottom + 1) -
                       max(lpRectBand->top, lpRectPrint->top));
      }

   /*********************
   * Draw bottom border
   *********************/

   if (lpRectBand->top <= lpRectPrint->bottom &&
       lpRectBand->bottom >= lpRectPrint->bottom)
      {
      SS_PrintDrawLine(hDC, lpRectPrint->left, lpRectPrint->bottom,
                       lpRectPrint->right - lpRectPrint->left, 1);
      }

   SetViewportOrgEx(hDC, lpPFormat->x + 1, lpPFormat->y + 1, NULL);
   }

else
   SetViewportOrgEx(hDC, lpPFormat->x, lpPFormat->y, NULL);
}
#endif


void SS_PrintLineTo(HDC hDC, int x, int y, LPSSX_PRINTFORMAT lpPFormat)
{
#if 0
POINT             ptCurPos;
int               x1;
int               y1;
int               xOrig;
int               yOrig;
int               dx = 1;
int               dy = 1;
short             i;

dx = max(SS_SCREEN2PRINTERX(lpPFormat, 1), 1);
dy = max(SS_SCREEN2PRINTERY(lpPFormat, 1), 1);

xOrig = x;
yOrig = y;

GetCurrentPositionEx(hDC, &ptCurPos);

x1 = ptCurPos.x;
y1 = ptCurPos.y;

if (y == y1)
   {
   for (i = 0; i < dy; i++)
      {
      LineTo(hDC, x, y + i);
      MoveToEx(hDC, x1, y1 + i + 1, NULL);
      }

   MoveToEx(hDC, x, y1, NULL);
   }
else
   {
   for (i = 0; i < dx; i++)
      {
      LineTo(hDC, x + i, y);
      MoveToEx(hDC, x1 + i + 1, y1, NULL);
      }

   MoveToEx(hDC, x1, y, NULL);
   }
#else
LineTo(hDC, x, y);
#endif
}

#if defined(_WIN64) || defined(_IA64)
void SS_PrintHeader(LPSPREADSHEET lpSS, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                    SS_PRINTFUNC lpfnPrintCallBack, long lPage, LONG_PTR lAppData,
                    HFONT hFont, WORD wType)
#else
void SS_PrintHeader(LPSPREADSHEET lpSS, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect,
                    SS_PRINTFUNC lpfnPrintCallBack, long lPage, long lAppData,
                    HFONT hFont, WORD wType)
#endif
{
HFONT hFontTemp;
int   DCState;

if (lpfnPrintCallBack && ((wType == SS_PRINTCMD_PRINTHEADER &&
    lpRect->top <= lpPFormat->y && lpPFormat->AppHeaderLen > 1) ||
   (wType == SS_PRINTCMD_PRINTFOOTER &&
    lpRect->bottom - lpPFormat->y >= lpPFormat->yPrintArea &&
    lpPFormat->AppFooterLen > 1)))
   {
   int y;
   int iHeight;

   DCState = SaveDC(hDC);
   SetWindowOrgEx(hDC, 0, 0, NULL);

   if (wType == SS_PRINTCMD_PRINTHEADER)
      {
      y = lpPFormat->y - lpPFormat->AppHeaderLen;
      iHeight = lpPFormat->AppHeaderLen;
      }
   else
      {
      y = lpPFormat->y + lpPFormat->yPrintArea;
      iHeight = lpPFormat->AppFooterLen;
      }

   SetViewportOrgEx(hDC, lpPFormat->x, y, NULL);

   IntersectClipRect(hDC, 0, 0, lpPFormat->cx, iHeight);

   hFontTemp = SelectObject(hDC, hFont);

#ifdef SS_V30
   lPage += (lpSS->PrintOptions.pf.lFirstPageNumber - 1);
   (*lpfnPrintCallBack)(hDC, wType, (short)lPage, lAppData, &lpPFormat->PrintInfo);
#else
   (*lpfnPrintCallBack)(hDC, wType, (short)lPage, lAppData);
#endif

   SelectObject(hDC, hFontTemp);
   RestoreDC(hDC, DCState);
   }
}

#if defined(_WIN64) || defined(_IA64)
BOOL SS_PrintPage(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                  LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRectBand,
                  LPRECT lpRectPrint, LPRECT lpRectUpdate,
                  SS_PRINTFUNC lpfnPrintCallBack, long lPage, LONG_PTR lAppData,
                  HFONT hFont, SS_COORD ColAt, SS_COORD RowAt)
#else
BOOL SS_PrintPage(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                  LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRectBand,
                  LPRECT lpRectPrint, LPRECT lpRectUpdate,
                  SS_PRINTFUNC lpfnPrintCallBack, long lPage, long lAppData,
                  HFONT hFont, SS_COORD ColAt, SS_COORD RowAt)
#endif
{
RECT RectTemp;
RECT RectBandTemp;
HRGN hRgn;
BOOL fRet = TRUE;
int  iDCSave;
int  x, y;

#ifdef SS_V70
x = lpRectPrint->left;
y = lpRectPrint->top;
#else
x = lpPFormat->x;
y = lpPFormat->y;
#endif // SS_V70

/**************
* Draw Header
**************/

SS_PrintHeader(lpSS, hDC, lpPFormat, lpRectBand, lpfnPrintCallBack,
               lPage, lAppData, hFont, SS_PRINTCMD_PRINTHEADER);

if (lpRectBand->bottom >= lpPFormat->y)
   {
   /**************
   * Draw Border
   **************/

//   SS_PrintBorder(hDC, lpPFormat, lpRectPrint, lpRectBand);

	if (lpPFormat->pf.fDrawBorder)
		SetViewportOrgEx(hDC, x + 1, y + 1, NULL);
	else
		SetViewportOrgEx(hDC, x, y, NULL);

#ifdef SS_V30
   RectBandTemp.left = max(lpRectBand->left - x, 0);
   RectBandTemp.top = max(lpRectBand->top - y, 0);
   RectBandTemp.right = max(lpRectBand->right - x, 0);
   RectBandTemp.bottom = max(lpRectBand->bottom - y, 0);
#else
   RectBandTemp.left = SS_PRINTER2SCREENX((lpPFormat),
                       max(lpRectBand->left - x, 0));
   RectBandTemp.top = SS_PRINTER2SCREENY((lpPFormat),
                      max(lpRectBand->top - y, 0));
   RectBandTemp.right = SS_PRINTER2SCREENX((lpPFormat),
                        max(lpRectBand->right - x, 0));
   RectBandTemp.bottom = SS_PRINTER2SCREENY((lpPFormat),
                         max(lpRectBand->bottom - y, 0));
#endif

   IntersectRect(&RectTemp, lpRectPrint, lpRectBand);

   iDCSave = SaveDC(hDC);
//   hRgn = CreateRectRgnIndirect(lpRectBand);
   // RFW - 8/6/01 - 9154
   RectTemp.top -= 1;
   RectTemp.left -= 1;
	// RFW - 10/21/02 - 11087
	RectTemp.right++;
   hRgn = CreateRectRgnIndirect(&RectTemp);
	SelectClipRgn(hDC, hRgn);
   DeleteObject(hRgn);

   IntersectRect(&RectTemp, lpRectUpdate, &RectBandTemp);

   // RFW - 5/27/99
   // Paint color background
   if (lpPFormat->pf.fDrawColors)
      {
      SS_COLORTBLITEM   BackColorTblItem;

      SS_GetColorItem(&BackColorTblItem, lpSS->Color.BackgroundId);
      if (lpPFormat->pf.fDrawBorder)
         {
         RectTemp.right -= 2;
         RectTemp.bottom--;
         }

      FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);
      }

   SS_Draw(hWnd, hDC, 0, lpSS, lpRectUpdate, &RectTemp, lpPFormat->ColLeft,
           lpPFormat->RowTop, ColAt - 1, RowAt - 1, ColAt, RowAt,
           lpPFormat);

   RestoreDC(hDC, iDCSave);

   /**************
   * Draw Border
   **************/

   SS_PrintBorder(hDC, lpPFormat, lpRectPrint, lpRectBand);
   }

/**************
* Draw Footer
**************/

SS_PrintHeader(lpSS, hDC, lpPFormat, lpRectBand, lpfnPrintCallBack,
               lPage, lAppData, hFont, SS_PRINTCMD_PRINTFOOTER);

return (fRet);
}

#if defined(SS_V30) && defined(SS_DLL)

void SS_PrintPreviewGetInfo(LPSPREADSHEET lpSS, LPPRVW_INFO lpPrvwInfo)
{
SS_PRINTPAGECALC PrintPageCalc;

if (lpPrvwInfo)
   {
   lpPrvwInfo->Margins.fMarginLeft = lpSS->PrintOptions.pf.fMarginLeft;
   lpPrvwInfo->Margins.fMarginTop = (float)lpSS->PrintOptions.pf.fMarginTop;
   lpPrvwInfo->Margins.fMarginRight = (float)lpSS->PrintOptions.pf.fMarginRight;
   lpPrvwInfo->Margins.fMarginBottom = (float)lpSS->PrintOptions.pf.fMarginBottom;

   SS_Print(lpSS, NULL, &lpSS->PrintOptions.pf, lpSS->PrintOptions.lpfnPrintProc,
            lpSS->PrintOptions.lAppData, 0, SS_PRINT_ACTION_PAGECALC, 0, NULL,
            &PrintPageCalc, NULL, NULL, NULL, NULL);

   if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_PAGERANGE)
		{
		if (lpSS->PrintOptions.pf.nPageStart == -1)
	      lpPrvwInfo->lPageBeg = 1;
		else
			lpPrvwInfo->lPageBeg = max(1, min(PrintPageCalc.lPageCnt, lpSS->PrintOptions.pf.nPageStart));

		if (lpSS->PrintOptions.pf.nPageEnd == -1)
		   lpPrvwInfo->lPageEnd = PrintPageCalc.lPageCnt;
		else
			lpPrvwInfo->lPageEnd = min(lpSS->PrintOptions.pf.nPageEnd,  PrintPageCalc.lPageCnt);
		}
   else
		{
      lpPrvwInfo->lPageBeg = 1;
	   lpPrvwInfo->lPageEnd = PrintPageCalc.lPageCnt;
		}

   lpPrvwInfo->fPageWidth = PrintPageCalc.fPageWidth;
   lpPrvwInfo->fPageHeight = PrintPageCalc.fPageHeight;
   }
}


void SS_PrintPreviewPrintPage(LPSPREADSHEET lpSS, LPPRVW_PAGE lpPrvwPage)
{
if (lpPrvwPage)
   {
   SS_PAGEFORMAT PageFormat;

   PageFormat = lpSS->PrintOptions.pf;
   PageFormat.dPrintType |= SS_PRINTTYPE_PAGERANGE;
   PageFormat.nPageStart = (short)lpPrvwPage->lPage;
   PageFormat.nPageEnd = (short)lpPrvwPage->lPage;

   SS_Print(lpSS, NULL, &PageFormat, lpSS->PrintOptions.lpfnPrintProc,
            lpSS->PrintOptions.lAppData, 0, SS_PRINT_ACTION_PREVIEW,
            lpPrvwPage->hDC, &lpPrvwPage->Rect, NULL, NULL, NULL, NULL, NULL);
   }
}

#endif // defined(SS_V30) && defined(SS_DLL)


BOOL SS_SetPrintHeader(LPSPREADSHEET lpSS, LPTSTR lpszText)
{
if (lpSS->PrintOptions.pf.hHeader)
	{
	GlobalFree(lpSS->PrintOptions.pf.hHeader);
	lpSS->PrintOptions.pf.hHeader = 0;
	}

if (lpszText && *lpszText)
	{
	if (lpSS->PrintOptions.pf.hHeader = GlobalAlloc(GHND, (lstrlen(lpszText) + 1) * sizeof(TCHAR)))
		{
		LPTSTR lpszTemp = (LPTSTR)GlobalLock(lpSS->PrintOptions.pf.hHeader);
		lstrcpy(lpszTemp, lpszText);
		GlobalUnlock(lpSS->PrintOptions.pf.hHeader);
		}
	}

return (TRUE);
}


BOOL SS_SetPrintFooter(LPSPREADSHEET lpSS, LPTSTR lpszText)
{
if (lpSS->PrintOptions.pf.hFooter)
	{
	GlobalFree(lpSS->PrintOptions.pf.hFooter);
	lpSS->PrintOptions.pf.hFooter = 0;
	}

if (lpszText && *lpszText)
	{
	if (lpSS->PrintOptions.pf.hFooter = GlobalAlloc(GHND, (lstrlen(lpszText) + 1) * sizeof(TCHAR)))
		{
		LPTSTR lpszTemp = (LPTSTR)GlobalLock(lpSS->PrintOptions.pf.hFooter);
		lstrcpy(lpszTemp, lpszText);
		GlobalUnlock(lpSS->PrintOptions.pf.hFooter);
		}
	}

return (TRUE);
}


#else


BOOL DLLENTRY SSPrint(hWnd, lpszPrintJobName, lpPageFormat, lpfnPrintProc,
                      lAppData, lpfnAbortProc)

HWND            hWnd;
LPCTSTR         lpszPrintJobName;
LPSS_PAGEFORMAT lpPageFormat;
FARPROC         lpfnPrintProc;
LONG            lAppData;
FARPROC         lpfnAbortProc;
{
return (TRUE);
}
#endif
