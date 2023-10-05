/*********************************************
* ssprvw.c
*
* Copyright (c) 1998 - FarPoint Technologies
* All rights reserved.
*********************************************/
#include <windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fptools.h"
#include "ssprvw.h"
#include <tchar.h>

/**********************
* Function prototypes
**********************/

LRESULT WINAPI _export fpSSPreviewWndFn(HWND hWnd, UINT uMessage, WPARAM wParam,
                                     LPARAM lParam);
void                SSPreviewHScroll(LPSSPREVIEW lpSSPreview, WORD wParam,
                                     LONG lParam);
void                SSPreviewVScroll(LPSSPREVIEW lpSSPreview, WORD wParam,
                                     LONG lParam);
short               SSPreviewCalcMargin(LPSSPREVIEW lpSSPreview,
                                        int iSize, long lNormal,
                                        short nMargin, short nPageCnt,
                                        long lGutter);
long                SSPreviewIsMouseInPage(LPSSPREVIEW lpSSPreview,
                                           short xMouse, short yMouse,
                                           LPRECT lpPageRect);

extern HANDLE fpInstance;

#define CALCMARGIN(Screen, Normal, Margin, Cnt) (short)((long)(Screen) / ((((long)(Normal) * (long)(Cnt)) / (long)(max(1, Margin))) + (long)(Cnt) + 1))
#define CALCGUTTERMARGIN(Screen, Normal, Margin, Cnt, Gutter) (short)(((long)(Margin) * ((long)(Screen) - (((long)(Cnt) - 1) * (long)(Gutter)))) / (((long)(Normal) * (long)(Cnt)) + (2L * (long)(Margin))))


/***********************************************************************
* Name:        RegisterSSPreview -
*
* Usage:       BOOL RegisterSSPreview(hInstance)
*                 HANDLE   hInstance - 
*
* Description: 
*
* Return:      
***********************************************************************/

BOOL SSPreviewRegister(HINSTANCE hInst)
{
WNDCLASS wc;
HDC hDC;

wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
wc.lpfnWndProc   = (WNDPROC)fpSSPreviewWndFn;
wc.cbClsExtra    = 0;
#if defined(_WIN64) || defined(_IA64)
wc.cbWndExtra    = 8;
#else
wc.cbWndExtra    = 4;
#endif
wc.hInstance     = hInst;
wc.hIcon         = NULL;
wc.hCursor       = NULL;
wc.hbrBackground = 0;
wc.lpszMenuName  = NULL;
wc.lpszClassName = FPCLASS_SSPREVIEW;

if (!RegisterClass((LPWNDCLASS)&wc))
   return (FALSE);
hDC = GetDC(GetDesktopWindow());
dxPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
dyPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSY);
ReleaseDC(GetDesktopWindow(), hDC);

return (TRUE);
}


BOOL SSPreviewUnregister(HINSTANCE hInst)
{
return (UnregisterClass(FPCLASS_SSPREVIEW, hInst));
}


GLOBALHANDLE SSPreviewCreate (HWND hWnd, LONG Style)
{
GLOBALHANDLE hGlobal;
LPSSPREVIEW  lpSSPreview;
HDC          hDC;

if (!(hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                            (long)sizeof(SSPREVIEW))))
   return (NULL);

lpSSPreview = (LPSSPREVIEW)GlobalLock(hGlobal);

#ifdef NO_MFC
   fpPM_Init(0);
#endif

hDC = GetDC(hWnd);
lpSSPreview->nPixelsPerInchX = GetDeviceCaps(hDC, LOGPIXELSX);
lpSSPreview->nPixelsPerInchY = GetDeviceCaps(hDC, LOGPIXELSY);
ReleaseDC(hWnd, hDC);

lpSSPreview->hWnd                = hWnd;
lpSSPreview->fAllowUserZoom      = TRUE;
lpSSPreview->GrayAreaColor       = RGBCOLOR_DARKGRAY;
lpSSPreview->lGrayAreaMarginH    = SSPRVW_MARGINDEF;
lpSSPreview->wGrayAreaMarginType = SSPRVW_MARGINTYPE_NORMAL;
lpSSPreview->lGrayAreaMarginV    = SSPRVW_MARGINDEF;
lpSSPreview->hWndSpread          = 0;
lpSSPreview->PageBorderColor     = RGBCOLOR_DARKBLUE;
lpSSPreview->nPageBorderWidth    = 2;
lpSSPreview->PageShadowColor     = RGBCOLOR_BLACK;
lpSSPreview->nPageShadowWidth    = 2;
lpSSPreview->nPageViewPercentage = 100;
lpSSPreview->wPageViewType       = SSPRVW_VIEWTYPE_WHOLEPAGE;
lpSSPreview->wZoomState          = SSPRVW_ZOOMSTATE_OUT;
lpSSPreview->wScrollBarH         = SSPRVW_SCROLLBAR_AUTO;
lpSSPreview->wScrollBarV         = SSPRVW_SCROLLBAR_AUTO;
lpSSPreview->lScrollIncH         = SSPRVW_SCROLLINCHDEF;
lpSSPreview->lScrollIncV         = SSPRVW_SCROLLINCVDEF;
lpSSPreview->lScrollPosV         = 1;
lpSSPreview->fShowMargins        = FALSE;
lpSSPreview->nPageMultiCntH      = 1;
lpSSPreview->nPageMultiCntV      = 1;
lpSSPreview->lPageBeg            = 1;
lpSSPreview->lPageEnd            = 100;
lpSSPreview->lPageGutterH        = -1;
lpSSPreview->lPageGutterV        = -1;
lpSSPreview->lPageCurrent        = 1;
lpSSPreview->fPageWidth          = (float)8.5;
lpSSPreview->fPageHeight         = (float)11.0;

lpSSPreview->wPageViewTypePrev   = lpSSPreview->wPageViewType;
lpSSPreview->nPageMultiCntHPrev  = lpSSPreview->nPageMultiCntH;
lpSSPreview->nPageMultiCntVPrev  = lpSSPreview->nPageMultiCntV;

lpSSPreview->hCursorZoomIn       = LoadCursor(fpInstance,
                                              _T("SSPREVIEWZOOMINCURSOR"));
lpSSPreview->hCursorZoomOut      = LoadCursor(fpInstance,
                                              _T("SSPREVIEWZOOMOUTCURSOR"));

#ifdef NO_MFC
lpSSPreview->idMouseIcon		 = 0;
#endif

#ifdef SS_V35
lpSSPreview->bScriptEnhanced	 = FALSE;
#endif


GlobalUnlock(hGlobal);

return (hGlobal);
}


void SSPreviewDestroy(HWND hWnd, GLOBALHANDLE hGlobal)
{
LPSSPREVIEW lpSSPreview;

if (hGlobal)
   {
   lpSSPreview = (LPSSPREVIEW)GlobalLock(hGlobal);
   SSPreviewSrcClose(lpSSPreview);
#ifdef NO_MFC
	   fpPM_Free(0);
#endif
  SSPreviewFreePagesInMem(lpSSPreview);
   DestroyCursor(lpSSPreview->hCursorZoomIn);
   DestroyCursor(lpSSPreview->hCursorZoomOut);
   GlobalUnlock(hGlobal);
   GlobalFree(hGlobal);
   }
}


LRESULT WINAPI _export fpSSPreviewWndFn(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
BOOL bCallDefWndProc = FALSE;
LRESULT lRet = 0L;

switch (uMessage)
   {
   case WM_NCCREATE:
      {
      GLOBALHANDLE hSSPreview;
      hSSPreview = SSPreviewCreate (hWnd, GetWindowLong(hWnd, GWL_STYLE));
#if defined(_WIN64) || defined(_IA64)
      SetWindowLongPtr (hWnd, 0, (LONG_PTR)hSSPreview);
#else
      SetWindowLong (hWnd, 0, (long)hSSPreview);
#endif
      }
      bCallDefWndProc = TRUE;
      break;

   case WM_DESTROY:
      {
      GLOBALHANDLE hSSPreview;
      if (IsWindow(hWnd) && (hSSPreview = 
#if defined(_WIN64) || defined(_IA64)
		  (GLOBALHANDLE)GetWindowLongPtr(hWnd, 0)))
#else
		  (GLOBALHANDLE)GetWindowLong(hWnd, 0)))
#endif
         {
         SSPreviewDestroy (hWnd, hSSPreview);
#if defined(_WIN64) || defined(_IA64)
         SetWindowLongPtr (hWnd, 0, 0);
#else
         SetWindowLong (hWnd, 0, 0L);
#endif
         }
      bCallDefWndProc = TRUE;
      }
      break;

   default:
      {
      GLOBALHANDLE hSSPreview;
      LPSSPREVIEW lpSSPreview;
#if defined(_WIN64) || defined(_IA64)
      hSSPreview  = (GLOBALHANDLE)GetWindowLongPtr (hWnd, 0);
#else
      hSSPreview  = (GLOBALHANDLE)GetWindowLong (hWnd, 0);
#endif

		if (hSSPreview)
			{
			lpSSPreview = (LPSSPREVIEW)GlobalLock (hSSPreview);
         lRet = fpSSPreviewMain(lpSSPreview, hWnd, uMessage, wParam, lParam, &bCallDefWndProc);
	      GlobalUnlock (hSSPreview);
			}
      }
   }

if (bCallDefWndProc)
   lRet = (DefWindowProc(hWnd, uMessage, wParam, lParam));

return (lRet);
}


LRESULT fpSSPreviewMain(LPSSPREVIEW lpSSPreview, HWND hWnd, UINT Msg, WPARAM wParam,
                     LPARAM lParam, BOOL FAR *lpfCallDef)
{
PAINTSTRUCT Paint;
HRGN        hRgn;
HBITMAP     hBitmapOld;
HBITMAP     hBitmapDC;
HDC         hDCMemory;
HDC         hDC;
RECT        Rect;
LRESULT     lRet = 0;

*lpfCallDef = FALSE;

/* RFW - 10/4/07 - 20859
if (fpMouseWheel(hWnd, Msg, wParam, lParam, 0, NULL))
   return (TRUE);
*/
if (fpMouseWheelEx(hWnd, Msg, wParam, lParam, 0, NULL, SSPreviewIsScrollPartial(lpSSPreview) ? 0 : 1))
   return (TRUE);

switch (Msg)
   {
   /******************************
   * The window has been resized
   ******************************/

   case WM_SIZE:
      {
      RECT RectClient;

      SetRect(&RectClient, 0, 0, LOWORD(lParam), HIWORD(lParam));
      if (!EqualRect(&RectClient, &lpSSPreview->RectClient))
         {
         SSPreviewCalcPage(lpSSPreview);
         CopyRect(&lpSSPreview->RectClient, &RectClient);
         }
      }

      break;

   case WM_GETDLGCODE:
		return (DLGC_WANTARROWS);

   case WM_HSCROLL:
#ifdef WIN32
      SSPreviewHScroll(lpSSPreview, LOWORD(wParam), (LONG)HIWORD(wParam));
#else
      SSPreviewHScroll(lpSSPreview, wParam, lParam);
#endif
      break;

   case WM_VSCROLL:
#ifdef WIN32
      SSPreviewVScroll(lpSSPreview, LOWORD(wParam), (LONG)HIWORD(wParam));
#else
      SSPreviewVScroll(lpSSPreview, wParam, lParam);
#endif
      break;

	case WM_KEYDOWN:
		switch (wParam)
			{
			case VK_UP:
		      SSPreviewVScroll(lpSSPreview, SB_LINEUP, 0);
				break;

			case VK_DOWN:
		      SSPreviewVScroll(lpSSPreview, SB_LINEDOWN, 0);
				break;

			case VK_NEXT:
		      SSPreviewVScroll(lpSSPreview, SB_PAGEDOWN, 0);
				break;

			case VK_PRIOR:
		      SSPreviewVScroll(lpSSPreview, SB_PAGEUP, 0);
				break;

			case VK_HOME:
		      SSPreviewVScroll(lpSSPreview, SB_TOP, 0);
				break;

			case VK_END:
		      SSPreviewVScroll(lpSSPreview, SB_BOTTOM, 0);
				break;
			}

		break;

/*
   case WM_MOUSEMOVE:
      if (lpSSPreview->fAllowUserZoom)
         {
         HCURSOR hCursor;

         if (SSPreviewIsMouseInPage(lpSSPreview, LOWORD(lParam),
                                    HIWORD(lParam), NULL))
            {
            if (lpSSPreview->wZoomState == SSPRVW_ZOOMSTATE_IN ||
                (lpSSPreview->wZoomState == SSPRVW_ZOOMSTATE_INDETERMINATE &&
                 lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE &&
                 lpSSPreview->nPageViewPercentage >= 100))
               hCursor = lpSSPreview->hCursorZoomOut;
            else
               hCursor = lpSSPreview->hCursorZoomIn;

#ifdef WIN32
            SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCursor);
#else
            SetClassWord(hWnd, GCW_HCURSOR, (WORD)hCursor);
#endif 
            SetCursor(hCursor);
            }
         else
            {
            hCursor = LoadCursor(NULL, IDC_ARROW);
#ifdef WIN32
            SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCursor);
#else
            SetClassWord(hWnd, GCW_HCURSOR, (WORD)hCursor);
#endif 
            }
         }

      break;
*/

   case WM_NCHITTEST:
#ifdef FP_OCX
      if (lpSSPreview->fDesignTime)
         return (HTCLIENT);
#endif

      if (lpSSPreview->fAllowUserZoom)
         {
         POINT Point;

         Point.x = LOWORD(lParam);
         Point.y = HIWORD(lParam);

         ScreenToClient(hWnd, &Point);
         lpSSPreview->hCursorCurrent = 0;

         if (SSPreviewIsMouseInPage(lpSSPreview, (short)Point.x, (short)Point.y,
                                    NULL))
            {
            if (lpSSPreview->wZoomState == SSPRVW_ZOOMSTATE_IN ||
                (lpSSPreview->wZoomState == SSPRVW_ZOOMSTATE_INDETERMINATE &&
                 lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE &&
                 lpSSPreview->nPageViewPercentage >= 100))
               lpSSPreview->hCursorCurrent = lpSSPreview->hCursorZoomOut;
            else
               lpSSPreview->hCursorCurrent = lpSSPreview->hCursorZoomIn;
            }
         }
		else
			lpSSPreview->hCursorCurrent = 0;

      break;

   case WM_SETCURSOR:
      if (wParam == (WPARAM)hWnd && !lpSSPreview->fDesignTime)
         {
         if (lpSSPreview->hCursorCurrent)
            {
#if defined(_WIN64) || defined(_IA64)
            SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)lpSSPreview->hCursorCurrent);
#elif WIN32
            SetClassLong(hWnd, GCL_HCURSOR, (LONG)lpSSPreview->hCursorCurrent);
#else
            SetClassWord(hWnd, GCW_HCURSOR, (WORD)lpSSPreview->hCursorCurrent);
#endif 
            SetCursor(lpSSPreview->hCursorCurrent);
            }
#ifdef FP_VB
         else if (VBDefControlProc(lpSSPreview->hCtl, hWnd, Msg, wParam, lParam))
            {
            SetClassWord(hWnd, GCW_HCURSOR, GetCursor());
            return (1);
            }
#endif
         else if (SendMessage(GetParent(hWnd), Msg, wParam, lParam))
            return (1);
/*         else if (lpSSPreview->fAllowUserZoom)
            {
				HCURSOR hCursor;
				if (lpSSPreview->wZoomState == SSPRVW_ZOOMSTATE_IN)
					hCursor = lpSSPreview->hCursorZoomIn;
				else
					hCursor = lpSSPreview->hCursorZoomOut;
	#ifdef WIN32
				SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCursor);
	#else
				SetClassWord(hWnd, GCW_HCURSOR, hCursor);
	#endif 
				SetCursor(hCursor);
			 }
*/
         else
            {
            HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
#if defined(_WIN64) || defined(_IA64)
            SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)hCursor);
#elif WIN32
            SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCursor);
#else
            SetClassWord(hWnd, GCW_HCURSOR, hCursor);
#endif 
            SetCursor(hCursor);
            }

         return (0);
         }

      break;

   case WM_LBUTTONDOWN:
   case WM_LBUTTONDBLCLK:
      if (lpSSPreview->fAllowUserZoom)
         {
         RECT RectPage;
         long lPage;

         if (lPage = SSPreviewIsMouseInPage(lpSSPreview, LOWORD(lParam),
                                            HIWORD(lParam), &RectPage))
            {
            SSPreviewSetPageCurrent(lpSSPreview, lPage);
            SSPreviewZoom(lpSSPreview);
            InvalidateRect(hWnd, NULL, TRUE);

            // Move the page to where it was clicked on

            if (lpSSPreview->wZoomState == SSPRVW_ZOOMSTATE_IN)
               {
               double dfPercentX = (double)(LOWORD(lParam) - RectPage.left) /
                                   (double)(RectPage.right - RectPage.left);
               double dfPercentY = (double)(HIWORD(lParam) - RectPage.top) /
                                   (double)(RectPage.bottom - RectPage.top);

               if (lpSSPreview->lScrollMaxH)
                  {
                  lpSSPreview->lScrollPosH = (long)((double)lpSSPreview->lScrollMaxH *
                                                    dfPercentX);
                  SetScrollPos(lpSSPreview->hWnd, SB_HORZ,
                               (short)lpSSPreview->lScrollPosH, TRUE);
                  }

               if (lpSSPreview->lScrollMaxV)
                  {
                  lpSSPreview->lScrollPosV = (long)((double)lpSSPreview->lScrollMaxV *
                                                    dfPercentY);
                  SetScrollPos(lpSSPreview->hWnd, SB_VERT,
                               (short)lpSSPreview->lScrollPosV, TRUE);
                  }
               }
            }
         }

      break;

   case SPM_SETPROPERTY:
      return (PV_SetProp(lpSSPreview, (WORD)wParam, lParam));

   case SPM_GETPROPERTY:
      return (PV_GetProp(lpSSPreview, (WORD)wParam, lParam));

   case WM_CLOSE:
      DestroyWindow(hWnd);
      return (0);

   case WM_NCPAINT:
	   {
		if (!IsWindowVisible(hWnd))
         return (0);
	   }
      break;

   case WM_ERASEBKGND:
      return (TRUE);

#if (WINVER >= 0x0400)
   // New to Windows 95
   case WM_PRINT:
      if (!(lParam & PRF_CLIENT))
         return (0);
#endif
   case WM_PAINT:
      GetClientRect(hWnd, &Rect);

      if (wParam)
         {
         hDC = (HDC)wParam;
         CopyRect(&Paint.rcPaint, &Rect);
         }
      else
         hDC = BeginPaint(hWnd, &Paint);

      /*********************************
      * Create a memory device context
      *********************************/

      if (!lpSSPreview->fNoMemDCPaint)
         {
			HDC hDCScreen = GetDC(0);
         hBitmapDC = CreateCompatibleBitmap(hDCScreen, Rect.right - Rect.left,
                                            Rect.bottom - Rect.top);
			ReleaseDC(0, hDCScreen);
         hDCMemory = CreateCompatibleDC(0);
         hBitmapOld = SelectObject(hDCMemory, hBitmapDC);

         hRgn = CreateRectRgnIndirect(&Rect);
         SelectClipRgn(hDCMemory, hRgn);
         DeleteObject(hRgn);
         }
      else
         hDCMemory = hDC;

     SSPreviewPaint(lpSSPreview, hWnd, hDC, hDCMemory, &Rect);

      if (!lpSSPreview->fNoMemDCPaint)
         SelectClipRgn(hDCMemory, 0);

      /*******************************************************
      * Copy the memory device context bitmap to the display
      *******************************************************/

      if (!lpSSPreview->fNoMemDCPaint)
         {
         BitBlt(hDC, Paint.rcPaint.left, Paint.rcPaint.top, Paint.rcPaint.right -
              Paint.rcPaint.left, Paint.rcPaint.bottom - Paint.rcPaint.top,
              hDCMemory, Paint.rcPaint.left, Paint.rcPaint.top, SRCCOPY);

         /***********
         * Clean up
         ***********/

         SelectObject(hDCMemory, hBitmapOld);
         DeleteDC(hDCMemory);
         DeleteObject(hBitmapDC);
         }

      if (!wParam)
         EndPaint(hWnd, &Paint);

      return (0);
   }

*lpfCallDef = TRUE;
return (lRet);
}


void SSPreviewGetMargins(LPSSPREVIEW lpSSPreview, LPSHORT lpnMarginH,
                         LPSHORT lpnMarginV)
{
RECT  RectClient;
short nMarginH;
short nMarginV;
short nGutterH;
short nGutterV;
long  lNormalPageV;
long  lNormalPageH;

GetClientRect(lpSSPreview->hWnd, &RectClient);

nMarginH = TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lGrayAreaMarginH);
nMarginV = TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lGrayAreaMarginV);

if (lpSSPreview->lPageGutterH == -1)
   nGutterH = nMarginH;
else
   nGutterH = TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lPageGutterH);

if (lpSSPreview->lPageGutterV == -1)
   nGutterV = nMarginV;
else
   nGutterV = TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lPageGutterV);

if (lpSSPreview->wGrayAreaMarginType == SSPRVW_MARGINTYPE_NORMAL)
   {
   lNormalPageV = (long)((double)lpSSPreview->fPageHeight *
                         (double)lpSSPreview->nPixelsPerInchY);
   lNormalPageH = (long)((double)lpSSPreview->fPageWidth *
                         (double)lpSSPreview->nPixelsPerInchX);

   switch (lpSSPreview->wPageViewType)
      {
      case SSPRVW_VIEWTYPE_WHOLEPAGE:
         nMarginV = SSPreviewCalcMargin(lpSSPreview, RectClient.bottom -
                                        RectClient.top, lNormalPageV,
                                        nMarginV, 1,
                                        lpSSPreview->lPageGutterV);
         nMarginH = SSPreviewCalcMargin(lpSSPreview, RectClient.right -
                                        RectClient.left, lNormalPageH,
                                        nMarginH, 1,
                                        lpSSPreview->lPageGutterH);
         break;

      case SSPRVW_VIEWTYPE_NORMALSIZE:
         break;

      case SSPRVW_VIEWTYPE_PERCENTAGE:
         nMarginV = nMarginV * lpSSPreview->nPageViewPercentage / 100;
         nMarginH = nMarginH * lpSSPreview->nPageViewPercentage / 100;
         break;

      case SSPRVW_VIEWTYPE_PAGEWIDTH:
         nMarginH = SSPreviewCalcMargin(lpSSPreview, RectClient.right -
                                        RectClient.left, lNormalPageH,
                                        nMarginH, 1,
                                        lpSSPreview->lPageGutterH);
         nMarginV = (short)((long)nMarginV * (long)PAGEWIDTH(lpSSPreview,
                    &RectClient, nMarginH, 1, nGutterH) / lNormalPageH);

         break;

      case SSPRVW_VIEWTYPE_PAGEHEIGHT:
         nMarginV = SSPreviewCalcMargin(lpSSPreview, RectClient.bottom -
                                        RectClient.top, lNormalPageV,
                                        nMarginV, 1,
                                        lpSSPreview->lPageGutterV);
         nMarginH = (short)((long)nMarginH * (long)PAGEHEIGHT(lpSSPreview,
                    &RectClient, nMarginV, 1, nGutterV) / lNormalPageV);
         break;

      case SSPRVW_VIEWTYPE_MULTIPLEPAGES:
         nMarginV = SSPreviewCalcMargin(lpSSPreview, RectClient.bottom -
                                        RectClient.top, lNormalPageV,
                                        nMarginV,
                                        lpSSPreview->nPageMultiCntV,
                                        lpSSPreview->lPageGutterV);
         nMarginH = SSPreviewCalcMargin(lpSSPreview, RectClient.right -
                                        RectClient.left,
                                        lNormalPageH, nMarginH,
                                        lpSSPreview->nPageMultiCntH,
                                        lpSSPreview->lPageGutterH);
         break;
      }
   }

if (lpnMarginH)
   *lpnMarginH = nMarginH;

if (lpnMarginV)
   *lpnMarginV = nMarginV;
}


void SSPreviewGetPageSize(LPSSPREVIEW lpSSPreview, LPRECT lpRect,
                          short nMarginH, short nMarginV)
{
RECT  RectClient;
short nWidth = -1;
short nHeight = -1;
short nPercentageH = -1;
short nPercentageV = -1;
short nGutterH;
short nGutterV;
long  lNormalPageV;
long  lNormalPageH;

GetClientRect(lpSSPreview->hWnd, &RectClient);

lNormalPageV = (long)((double)lpSSPreview->fPageHeight *
                      (double)lpSSPreview->nPixelsPerInchY);
lNormalPageH = (long)((double)lpSSPreview->fPageWidth *
                      (double)lpSSPreview->nPixelsPerInchX);

if (lpSSPreview->lPageGutterH == -1)
   nGutterH = nMarginH;
else
   nGutterH = TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lPageGutterH);

if (lpSSPreview->lPageGutterV == -1)
   nGutterV = nMarginV;
else
   nGutterV = TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lPageGutterV);

switch (lpSSPreview->wPageViewType)
   {
   case SSPRVW_VIEWTYPE_WHOLEPAGE:
      nWidth = (short)PAGEWIDTH(lpSSPreview, &RectClient, nMarginH, 1, nGutterH);
      nHeight = (short)PAGEHEIGHT(lpSSPreview, &RectClient, nMarginV, 1, nGutterV);
      break;

   case SSPRVW_VIEWTYPE_NORMALSIZE:
      nWidth = (short)lNormalPageH;
      nHeight = (short)lNormalPageV;
      break;

   case SSPRVW_VIEWTYPE_PERCENTAGE:
      nHeight = (short)(lNormalPageV * (long)lpSSPreview->nPageViewPercentage /
                        100L);
      break;

   case SSPRVW_VIEWTYPE_PAGEWIDTH:
      nWidth = (short)PAGEWIDTH(lpSSPreview, &RectClient, nMarginH, 1, nGutterH);
      break;

   case SSPRVW_VIEWTYPE_PAGEHEIGHT:
      nHeight = (short)PAGEHEIGHT(lpSSPreview, &RectClient, nMarginV, 1, nGutterV);
      break;

   case SSPRVW_VIEWTYPE_MULTIPLEPAGES:
      nWidth = (short)PAGEWIDTH(lpSSPreview, &RectClient, nMarginH,
                         lpSSPreview->nPageMultiCntH, nGutterH);
      nHeight = (short)PAGEHEIGHT(lpSSPreview, &RectClient, nMarginV,
                         lpSSPreview->nPageMultiCntV, nGutterV);
      break;
   }

if (nWidth != -1)
   nPercentageH = (short)((long)nWidth * 100L / lNormalPageH);

if (nHeight != -1)
   nPercentageV = (short)((long)nHeight * 100L / lNormalPageV);

if (nWidth == -1 || (nPercentageV != -1 && nPercentageH > nPercentageV))
   nWidth = (short)((long)nPercentageV * lNormalPageH / 100L);

if (nHeight == -1 || (nPercentageH != -1 && nPercentageV > nPercentageH))
   nHeight = (short)((long)nPercentageH * lNormalPageV / 100L);

lpRect->left = 0;
lpRect->right = nWidth;
lpRect->top = 0;
lpRect->bottom = nHeight;
}


short SSPreviewGetPagesPerScreen(LPSSPREVIEW lpSSPreview)
{
short nPageCnt = 1;

if (lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_MULTIPLEPAGES)
   nPageCnt = lpSSPreview->nPageMultiCntH * lpSSPreview->nPageMultiCntV;

return (nPageCnt);
}


void SSPreviewCalcPage(LPSSPREVIEW lpSSPreview)
{
SSPreviewFreePagesInMem(lpSSPreview);
SSPreviewSetScrollBars(lpSSPreview);
}


short SSPreviewCalcMargin(LPSSPREVIEW lpSSPreview, int iSize, long lNormal,
                          short nMargin, short nPageCnt, long lGutter)
{
short nMarginNew;
short nGutter;

if (lGutter != -1 && nPageCnt > 1)
   {
   nGutter = TWIPSTOPIXELSX(lpSSPreview, lGutter);
   nMarginNew = CALCGUTTERMARGIN(iSize, lNormal, nMargin, nPageCnt, nGutter);
   }
else
   nMarginNew = CALCMARGIN(iSize, lNormal, nMargin, nPageCnt);

return (nMarginNew);
}


void SSPreviewZoom(LPSSPREVIEW lpSSPreview)
{
WORD wZoomState;
WORD wZoomStateOld = lpSSPreview->wZoomState;

if (lpSSPreview->wZoomState == SSPRVW_ZOOMSTATE_IN)
   wZoomState = SSPRVW_ZOOMSTATE_OUT;
else
   wZoomState = SSPRVW_ZOOMSTATE_IN;

SendMessage(lpSSPreview->hWnd, SPM_SETPROPERTY, PVPROP_ZOOMSTATE, wZoomState);

if (wZoomStateOld == SSPRVW_ZOOMSTATE_INDETERMINATE)
   lpSSPreview->wPageViewTypePrev = SSPRVW_VIEWTYPE_WHOLEPAGE;
}


long SSPreviewIsMouseInPage(LPSSPREVIEW lpSSPreview, short xMouse, short yMouse,
                            LPRECT lpPageRect)
{
POINT Point;
RECT  RectClient;
RECT  RectTemp;
RECT  Rect;
short nMarginH;
short nMarginV;
short nCntH;
short nCntV;
short nOffsetH;
short nOffsetV;
short x;
short y;
short nScrollIncH;
short nScrollIncV;
short nGutterH;
short nGutterV;
long  lPageNum = lpSSPreview->lPageCurrent;
long  lPage = 0;

Point.x = xMouse;
Point.y = yMouse;

GetClientRect(lpSSPreview->hWnd, &RectClient);

if (PtInRect(&RectClient, Point))
   {
   /*********************************
   * Calculate margin and page size
   *********************************/

   SSPreviewGetMargins(lpSSPreview, &nMarginH, &nMarginV);
   SSPreviewGetPageSize(lpSSPreview, &Rect, nMarginH, nMarginV);
   Rect.right += lpSSPreview->nPageBorderWidth * 2;
   Rect.bottom += lpSSPreview->nPageBorderWidth * 2;

   nScrollIncH = max(TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lScrollIncH), 1);
   nScrollIncV = max(TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lScrollIncV), 1);
   nScrollIncH *= (short)lpSSPreview->lScrollPosH;
   nScrollIncV *= (short)lpSSPreview->lScrollPosV;

   switch (lpSSPreview->wPageViewType)
      {
      case SSPRVW_VIEWTYPE_NORMALSIZE:
         OffsetRect(&Rect, nMarginH, nMarginV);
         OffsetRect(&Rect, -nScrollIncH, -nScrollIncV);
         if (PtInRect(&Rect, Point))
            lPage = lpSSPreview->lPageCurrent;

         break;

      case SSPRVW_VIEWTYPE_WHOLEPAGE:
      case SSPRVW_VIEWTYPE_PERCENTAGE:
      case SSPRVW_VIEWTYPE_PAGEWIDTH:
      case SSPRVW_VIEWTYPE_PAGEHEIGHT:
         OffsetRect(&Rect,
                    max(nMarginH, (RectClient.right - RectClient.left -
                    Rect.right) / 2),
                    max(nMarginV, (RectClient.bottom - RectClient.top -
                    Rect.bottom) / 2));

         if (Rect.right + lpSSPreview->nPageShadowWidth > RectClient.right)
            OffsetRect(&Rect, -nScrollIncH, 0);

         if (Rect.bottom + lpSSPreview->nPageShadowWidth > RectClient.bottom)
            OffsetRect(&Rect, 0, -nScrollIncV);

         if (PtInRect(&Rect, Point))
            lPage = lpSSPreview->lPageCurrent;

         break;

      case SSPRVW_VIEWTYPE_MULTIPLEPAGES:
         nCntH = lpSSPreview->nPageMultiCntH;
         nCntV = lpSSPreview->nPageMultiCntV;

         if (lpSSPreview->lScrollPosV > 0)
            lPageNum = ((lpSSPreview->lScrollPosV - 1) *
                        SSPreviewGetPagesPerScreen(lpSSPreview)) + lpSSPreview->lPageBeg;
         else
            lPageNum = lpSSPreview->lPageBeg;

         if (lpSSPreview->lPageGutterH == -1)
            {
            nOffsetH = (short)max(nMarginH, (RectClient.right - RectClient.left -
                           (Rect.right * nCntH)) / (nCntH + 1));
            nGutterH = nOffsetH;
            }
         else
            {
            nGutterH = TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lPageGutterH);
            nOffsetH = (short)max(nMarginH, (RectClient.right - RectClient.left -
                           (Rect.right * nCntH) - ((nCntH - 1) * nGutterH)) / 2);
            }

         if (lpSSPreview->lPageGutterV == -1)
            {
            nOffsetV = (short)max(nMarginV, (RectClient.bottom - RectClient.top -
                           (Rect.bottom * nCntV)) / (nCntV + 1));
            nGutterV = nOffsetV;
            }
         else
            {
            nGutterV = TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lPageGutterV);
            nOffsetV = (short)max(nMarginV, (RectClient.bottom - RectClient.top -
                           (Rect.bottom * nCntV) - ((nCntV - 1) * nGutterV)) / 2);
            }

         for (y = 0; y < nCntV && lPageNum <= lpSSPreview->lPageEnd && !lPage;
              y++)
            for (x = 0; x < nCntH && lPageNum <= lpSSPreview->lPageEnd &&
                 !lPage; x++, lPageNum++)
               {
               CopyRect(&RectTemp, &Rect);
               OffsetRect(&RectTemp, nOffsetH + (x * nGutterH) +
                          (x * Rect.right), nOffsetV + (y * nGutterV) +
                          (y * Rect.bottom));

               if (PtInRect(&RectTemp, Point))
                  {
                  lPage = lPageNum;
                  CopyRect(&Rect, &RectTemp);
                  }
               }

         break;
      }
   }

if (lpPageRect)
   CopyRect(lpPageRect, &Rect);

return (lPage);
}


void SSPreviewSetPageCurrent(LPSSPREVIEW lpSSPreview, long lPageCurrent)
{
if (lpSSPreview->lPageCurrent != lPageCurrent)
	{
	lpSSPreview->lPageCurrent = lPageCurrent;
	SSPreviewSetScrollBars(lpSSPreview);
	PV_SendWMCommand(lpSSPreview, SPN_PAGECHANGE);
	}
}


void SSPreviewSrcOpen(LPSSPREVIEW lpSSPreview)
{
PRVW_OPEN PrvwOpen;

if (lpSSPreview->hWndSpread)
   {
   if (SendMessage(lpSSPreview->hWndSpread, PRVWM_OPEN, (WPARAM)lpSSPreview->hWnd,
                   (LPARAM)(LPVOID)&PrvwOpen))
      {
      lpSSPreview->fAllowMarginResize = PrvwOpen.fAllowMarginResize;

      if (!lpSSPreview->fAllowMarginResize && lpSSPreview->fShowMargins)
         lpSSPreview->fShowMargins = FALSE;

      SSPreviewSrcGetInfo(lpSSPreview);
      }
   else
      lpSSPreview->hWndSpread = 0;
   }
}


void SSPreviewSrcClose(LPSSPREVIEW lpSSPreview)
{
if (lpSSPreview->hWndSpread)
   {
   SendMessage(lpSSPreview->hWndSpread, PRVWM_CLOSE, (WPARAM)lpSSPreview->hWnd, 0);
   lpSSPreview->hWndSpread = 0;
   }
}


void SSPreviewSrcGetInfo(LPSSPREVIEW lpSSPreview)
{
PRVW_INFO PrvwInfo;

#ifdef PP40
PrvwInfo.hDCScreen = GetDC(lpSSPreview->hWnd);
#endif
if (lpSSPreview->hWndSpread)
   {
   PrvwInfo.hDCScreen = GetDC(lpSSPreview->hWndSpread);
   if (SendMessage(lpSSPreview->hWndSpread, PRVWM_GETINFO, (WPARAM)lpSSPreview->hWnd,
                   (LPARAM)(LPVOID)&PrvwInfo))
      {
      if (PrvwInfo.lPageEnd - PrvwInfo.lPageBeg + 1 <= 0)
         lpSSPreview->hWndSpread = 0;
      else
         {
         lpSSPreview->lPageBeg = PrvwInfo.lPageBeg;
         lpSSPreview->lPageEnd = PrvwInfo.lPageEnd;
         _fmemcpy(&lpSSPreview->Margins, &PrvwInfo.Margins,
                  sizeof(PRVW_MARGINS));
         lpSSPreview->fPageWidth = PrvwInfo.fPageWidth;
         lpSSPreview->fPageHeight = PrvwInfo.fPageHeight;
         lpSSPreview->lPageCurrent = lpSSPreview->lPageBeg;
         SSPreviewCalcPage(lpSSPreview);
         }
      }

   ReleaseDC(lpSSPreview->hWndSpread, PrvwInfo.hDCScreen);
   }
}


void SSPreviewFreePagesInMem(LPSSPREVIEW lpSSPreview)
{
LPHBITMAP lphBitmap;
short     i;

if (lpSSPreview->hPagesInMem)
   {
   lphBitmap = (LPHBITMAP)GlobalLock(lpSSPreview->hPagesInMem);

   for (i = 0; i <= lpSSPreview->lPagesInMemEnd - lpSSPreview->lPagesInMemBeg;
        i++)
      if (lphBitmap[i])
         DeleteObject(lphBitmap[i]);

   GlobalUnlock(lpSSPreview->hPagesInMem);
   GlobalFree(lpSSPreview->hPagesInMem);
   lpSSPreview->hPagesInMem = 0;
   }
}


LPSSPREVIEW PV_Lock(HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
GLOBALHANDLE hSSPreview  = (GLOBALHANDLE)GetWindowLongPtr (hWnd, 0);
#else
GLOBALHANDLE hSSPreview  = (GLOBALHANDLE)GetWindowLong (hWnd, 0);
#endif
if (hSSPreview)
   return ((LPSSPREVIEW)GlobalLock(hSSPreview));
else
   return (0);
}


void PV_Unlock(HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
GLOBALHANDLE hSSPreview  = (GLOBALHANDLE)GetWindowLongPtr (hWnd, 0);
#else
GLOBALHANDLE hSSPreview  = (GLOBALHANDLE)GetWindowLong (hWnd, 0);
#endif

if (hSSPreview)
   GlobalUnlock(hSSPreview);
}


LRESULT PV_SendWMCommand(LPSSPREVIEW lpSSPreview, WORD wNotification)
{
WORD id = (WORD)GETGW_ID(lpSSPreview->hWnd);

#ifdef WIN32
   return(PV_SendMessage(lpSSPreview, WM_COMMAND, MAKELONG(id, wNotification),
            (LPARAM)lpSSPreview->hWnd));
#else
   return(PV_SendMessage(lpSSPreview, WM_COMMAND, id,
            MAKELONG((WORD)lpSSPreview->hWnd, wNotification)));
#endif
}


LRESULT PV_SendMessage(LPSSPREVIEW lpSSPreview, UINT Msg, WPARAM wParam,
                    LPARAM lParam)
{
#if defined (FP_VB) || defined(FP_OCX)
BOOL bSelfMsg = TRUE;
#else
BOOL bSelfMsg = FALSE;
#endif

if (bSelfMsg)
   return (SendMessage(lpSSPreview->hWnd, Msg, wParam, lParam));
else
   return (SendMessage(GetParent(lpSSPreview->hWnd), Msg, wParam, lParam));
}

