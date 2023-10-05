/*********************************************
* pvpaint.c
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

/**********************
* Function prototypes
**********************/

void                    SSPreviewHatchRect(HDC hDC, LPSSPREVIEW lpSSPreview,
                                           LPRECT lpRect);
void                    SSPreviewDrawPage(HDC hDC, HDC hDCDev, LPSSPREVIEW lpSSPreview,
                                          LPRECT lpRectClient, LPRECT lpRect,
                                          long lPageNum, LPHBITMAP lphBitmap);
void                    SSPreviewDrawMargins(HDC hDC, LPSSPREVIEW lpSSPreview,
                                             LPRECT lpRectClient,
                                             LPRECT lpRect);


/***********************************************************************
* Name:        SSPreviewPaint -
*
* Usage:       void SSPreviewPaint(hWnd)
*                 HWND         hWnd - Window Handle
*
* Description: 
*
* Return:      
***********************************************************************/

void SSPreviewPaint(LPSSPREVIEW lpSSPreview, HWND hWnd, HDC hDCDev, HDC hDC,
                    LPRECT lpRectClient)
{
LPHBITMAP lphBitmap;
RECT      RectTemp;
RECT      Rect;
short     nMarginH;
short     nMarginV;
short     nCntH;
short     nCntV;
#if defined(_WIN64) || defined(_IA64)
long      nOffsetH;
long      nOffsetV;
long     nGutterH;
long     nGutterV;
#else
short     nOffsetH;
short     nOffsetV;
short     nGutterH;
short     nGutterV;
#endif
short     x;
short     y;
short     nScrollIncH;
short     nScrollIncV = 0;
long      lPageNum = lpSSPreview->lPageCurrent;

if (lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_MULTIPLEPAGES)
   {
   if (lpSSPreview->lScrollPosV > 0)
      lPageNum = ((lpSSPreview->lScrollPosV - 1) *
                  SSPreviewGetPagesPerScreen(lpSSPreview)) + lpSSPreview->lPageBeg;
   else
      lPageNum = lpSSPreview->lPageBeg;
   }

/*******************
* Clear Background
*******************/

SSPreviewHatchRect(hDC, lpSSPreview, lpRectClient);

/*********************************
* Calculate margin and page size
*********************************/

SSPreviewGetMargins(lpSSPreview, &nMarginH, &nMarginV);
SSPreviewGetPageSize(lpSSPreview, &Rect, nMarginH, nMarginV);
Rect.right += lpSSPreview->nPageBorderWidth * 2;
Rect.bottom += lpSSPreview->nPageBorderWidth * 2;

nScrollIncH = max(TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lScrollIncH), 1);
nScrollIncH *= (short)lpSSPreview->lScrollPosH;

if ((lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_NORMALSIZE ||
    lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE ||
    lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PAGEWIDTH) &&
    (Rect.bottom + nMarginV + (2 * lpSSPreview->nPageBorderWidth) +
    lpSSPreview->nPageShadowWidth > lpRectClient->bottom))
   {
   nScrollIncV = max(TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lScrollIncV), 1);
   nScrollIncV *= (short)lpSSPreview->lScrollPosV;
   }

/****************************************************************
* Set if the table that keeps pages in memory needs reallocated
****************************************************************/

if (lpSSPreview->hWndSpread && (!lpSSPreview->hPagesInMem ||
    lPageNum != lpSSPreview->lPagesInMemBeg))
   {
   SSPreviewFreePagesInMem(lpSSPreview);

   lpSSPreview->lPagesInMemBeg = lPageNum;
   lpSSPreview->lPagesInMemEnd = lpSSPreview->lPagesInMemBeg +
                                 SSPreviewGetPagesPerScreen(lpSSPreview) - 1;

   lpSSPreview->hPagesInMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                          sizeof(HBITMAP) *
                                          (lpSSPreview->lPagesInMemEnd -
                                          lpSSPreview->lPagesInMemBeg + 1));
   }

if (lpSSPreview->hPagesInMem)
   lphBitmap = (LPHBITMAP)GlobalLock(lpSSPreview->hPagesInMem);

/****************
* Print Page(s)
****************/

switch (lpSSPreview->wPageViewType)
   {
   case SSPRVW_VIEWTYPE_NORMALSIZE:
      OffsetRect(&Rect, nMarginH, nMarginV);
      OffsetRect(&Rect, -nScrollIncH, -nScrollIncV);
      SSPreviewDrawPage(hDC, hDCDev, lpSSPreview, lpRectClient, &Rect, lPageNum,
                        lphBitmap);
      break;

   case SSPRVW_VIEWTYPE_WHOLEPAGE:
   case SSPRVW_VIEWTYPE_PERCENTAGE:
   case SSPRVW_VIEWTYPE_PAGEWIDTH:
   case SSPRVW_VIEWTYPE_PAGEHEIGHT:
      OffsetRect(&Rect,
                 max(nMarginH, (lpRectClient->right - lpRectClient->left -
                 Rect.right) / 2),
                 max(nMarginV, (lpRectClient->bottom - lpRectClient->top -
                 Rect.bottom) / 2));
      OffsetRect(&Rect, -nScrollIncH, -nScrollIncV);

      SSPreviewDrawPage(hDC, hDCDev, lpSSPreview, lpRectClient, &Rect, lPageNum,
                        lphBitmap);
      break;

   case SSPRVW_VIEWTYPE_MULTIPLEPAGES:
      nCntH = lpSSPreview->nPageMultiCntH;
      nCntV = lpSSPreview->nPageMultiCntV;

      if (lpSSPreview->lPageGutterH == -1)
         {
         nOffsetH = max(nMarginH, (lpRectClient->right - lpRectClient->left -
                        (Rect.right * nCntH)) / (nCntH + 1));
         nGutterH = nOffsetH;
         }
      else
         {
         nGutterH = TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lPageGutterH);
         nOffsetH = max(nMarginH, (lpRectClient->right - lpRectClient->left -
                        (Rect.right * nCntH) - ((nCntH - 1) * nGutterH)) / 2);
         }

      if (lpSSPreview->lPageGutterV == -1)
         {
         nOffsetV = max(nMarginV, (lpRectClient->bottom - lpRectClient->top -
                        (Rect.bottom * nCntV)) / (nCntV + 1));
         nGutterV = nOffsetV;
         }
      else
         {
         nGutterV = TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lPageGutterV);
         nOffsetV = max(nMarginV, (lpRectClient->bottom - lpRectClient->top -
                        (Rect.bottom * nCntV) - ((nCntV - 1) * nGutterV)) / 2);
         }

      for (y = 0; y < nCntV && lPageNum <= lpSSPreview->lPageEnd; y++)
         for (x = 0; x < nCntH && lPageNum <= lpSSPreview->lPageEnd; x++,
              lPageNum++)
            {
            CopyRect(&RectTemp, &Rect);
            OffsetRect(&RectTemp, nOffsetH + (x * nGutterH) +
                       (x * Rect.right), nOffsetV + (y * nGutterV) +
                       (y * Rect.bottom));
            SSPreviewDrawPage(hDC, hDCDev, lpSSPreview, lpRectClient, &RectTemp,
                              lPageNum, lphBitmap);
            }

      break;
   }

if (lpSSPreview->hPagesInMem)
   GlobalUnlock(lpSSPreview->hPagesInMem);
}


void SSPreviewHatchRect(HDC hDC, LPSSPREVIEW lpSSPreview, LPRECT lpRect)
{
HBRUSH hBrush = CreateSolidBrush(FPCOLOR(lpSSPreview->GrayAreaColor));
HBRUSH hBrushOld = SelectObject(hDC, hBrush);

FillRect(hDC, lpRect, hBrush);

SelectObject(hDC, hBrushOld);
DeleteObject(hBrush);
}


void SSPreviewDrawPage(HDC hDC, HDC hDCDev, LPSSPREVIEW lpSSPreview, LPRECT lpRectClient,
                       LPRECT lpRect, long lPageNum, LPHBITMAP lphBitmap)
{
HBRUSH hBrush;
HBRUSH hBrushOld;
HPEN   hPen;
HPEN   hPenOld;
RECT   RectTemp;

/**************************
* Draw Border around Page
**************************/

hPen = CreatePen(PS_INSIDEFRAME, lpSSPreview->nPageBorderWidth,
                 FPCOLOR(lpSSPreview->PageBorderColor));
hPenOld = SelectObject(hDC, hPen);

hBrush = CreateSolidBrush(RGBCOLOR_WHITE);
hBrushOld = SelectObject(hDC, hBrush);

Rectangle(hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);

SelectObject(hDC, hBrushOld);
DeleteObject(hBrush);

SelectObject(hDC, hPenOld);
DeleteObject(hPen);

/**************************
* Draw Shadow around Page
**************************/

if (lpSSPreview->nPageShadowWidth > 0)
   {
   hBrush = CreateSolidBrush(FPCOLOR(lpSSPreview->PageShadowColor));

   SetRect(&RectTemp, lpRect->right, lpRect->top + 2, lpRect->right +
           lpSSPreview->nPageShadowWidth, lpRect->bottom +
           lpSSPreview->nPageShadowWidth);
   FillRect(hDC, &RectTemp, hBrush);

   SetRect(&RectTemp, lpRect->left + 2, lpRect->bottom, lpRect->right,
           lpRect->bottom + lpSSPreview->nPageShadowWidth);
   FillRect(hDC, &RectTemp, hBrush);

   DeleteObject(hBrush);
   }

if (lpSSPreview->hWndSpread)
   {
   PRVW_PAGE PrvwPage;
   HBITMAP   hBitmapDC;
   HBITMAP   hBitmapOld;
   HDC       hDCMemory;
   HRGN      hRgn;
   RECT      RectPage;

//   SetRect(&RectPage, 0, 0, lpRect->right - lpRect->left, lpRect->bottom -
//           lpRect->top);
   SetRect(&RectPage, 0, 0,
           lpRect->right - lpRect->left - (lpSSPreview->nPageBorderWidth * 2),
           lpRect->bottom - lpRect->top - (lpSSPreview->nPageBorderWidth * 2));

   if (lphBitmap && lphBitmap[lPageNum - lpSSPreview->lPagesInMemBeg])
      {
      hDCMemory = CreateCompatibleDC(hDCDev);
      hBitmapOld = SelectObject(hDCMemory,
                   lphBitmap[lPageNum - lpSSPreview->lPagesInMemBeg]);

      /*******************************************************
      * Copy the memory device context bitmap to the display
      *******************************************************/

//      BitBlt(hDC, lpRect->left, lpRect->top, lpRect->right - lpRect->left,
//             lpRect->bottom - lpRect->top, hDCMemory, 0, 0, SRCCOPY);
      BitBlt(hDC, lpRect->left + lpSSPreview->nPageBorderWidth,
             lpRect->top + lpSSPreview->nPageBorderWidth,
             RectPage.right - RectPage.left,
             RectPage.bottom - RectPage.top, hDCMemory, 0, 0, SRCCOPY);

      /***********
      * Clean up
      ***********/

      SelectObject(hDCMemory, hBitmapOld);
      DeleteDC(hDCMemory);
      }

   else
      {
      /*********************************
      * Create a memory device context
      *********************************/

      hBitmapDC = CreateCompatibleBitmap(hDCDev, RectPage.right - RectPage.left,
                                         RectPage.bottom - RectPage.top);

      hDCMemory = CreateCompatibleDC(hDCDev);
      hBitmapOld = SelectObject(hDCMemory, hBitmapDC);

      hRgn = CreateRectRgnIndirect(&RectPage);
      SelectClipRgn(hDCMemory, hRgn);
      DeleteObject(hRgn);

      hBrush = CreateSolidBrush(RGBCOLOR_WHITE);
      FillRect(hDCMemory, &RectPage, hBrush);
      DeleteObject(hBrush);

      PrvwPage.hDC = hDCMemory;
      CopyRect(&PrvwPage.Rect, &RectPage);
      PrvwPage.lPage = lPageNum;

      SendMessage(lpSSPreview->hWndSpread, PRVWM_PRINTPAGE, (WPARAM)lpSSPreview->hWnd,
                  (LPARAM)(LPVOID)&PrvwPage);

      SelectClipRgn(hDCMemory, 0);

      /*******************************************************
      * Copy the memory device context bitmap to the display
      *******************************************************/

//      BitBlt(hDC, lpRect->left, lpRect->top, lpRect->right - lpRect->left,
//             lpRect->bottom - lpRect->top, hDCMemory, 0, 0, SRCCOPY);
      BitBlt(hDC, lpRect->left + lpSSPreview->nPageBorderWidth,
             lpRect->top + lpSSPreview->nPageBorderWidth,
             RectPage.right - RectPage.left,
             RectPage.bottom - RectPage.top, hDCMemory, 0, 0, SRCCOPY);

      /***********
      * Clean up
      ***********/

      SelectObject(hDCMemory, hBitmapOld);
      DeleteDC(hDCMemory);

      if (lphBitmap)
         lphBitmap[lPageNum - lpSSPreview->lPagesInMemBeg] = hBitmapDC;
      else
         DeleteObject(hBitmapDC);

      if (PV_ALLOWMARGINRESIZE(lpSSPreview))
         SSPreviewDrawMargins(hDC, lpSSPreview, lpRectClient, lpRect);
      }
   }
}


void SSPreviewDrawMargins(HDC hDC, LPSSPREVIEW lpSSPreview,
                          LPRECT lpRectClient, LPRECT lpRect)
{
}
