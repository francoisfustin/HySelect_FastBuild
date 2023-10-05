/*********************************************************
* SS_SPICT.C          
*
* Copyright (C) 1991-1995 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <string.h>

#ifdef SS_OCX

#ifndef WIN32
#include <ole2.h>
#include <olenls.h>
#include <dispatch.h>
#endif
#include <ole2.h>                                      //DBOCX
#include <olectl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <math.h>
#include "toolbox.h"
#include "fphdc.h"
#include "..\classes\wintools.h"

#include "spread.h"
#include "ss_spict.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_span.h"
#include "ss_user.h"

#ifdef SS_V70
#include "..\..\..\fplibs\CxImage\CxImage\fpimages.h"
#include "..\..\..\fplibs\CxImage\CxImage\ximage.h"
#endif

#ifdef SS_V80
#include "ss_gradient.h"
#endif
//--------------------------------------------------------------------
//
//  The CreateOcxPict() function creates an OCX picture interface
//  from the given bitmap or icon.
//

#ifndef SS_NOSAVE

#ifdef SS_OCX
LPVOID CreateOcxPict(HANDLE hPicture, BOOL bIsBitmap, BOOL bIsIcon)
{
PICTDESC pictDesc;
LPPICTURE lpPict = NULL;

if (hPicture)
   {
   if (bIsBitmap)
      {
      pictDesc.cbSizeofstruct = sizeof(pictDesc);
      pictDesc.picType = PICTYPE_BITMAP;
      pictDesc.bmp.hbitmap = (HBITMAP)hPicture;
      pictDesc.bmp.hpal = NULL;
      OleCreatePictureIndirect(&pictDesc, &IID_IPicture, TRUE, &(LPVOID)lpPict);
      }
   else if (bIsIcon)
      {
      pictDesc.cbSizeofstruct = sizeof(pictDesc);
      pictDesc.picType = PICTYPE_ICON;
      pictDesc.icon.hicon = (HICON)hPicture;
      OleCreatePictureIndirect(&pictDesc, &IID_IPicture, TRUE, &(LPVOID)lpPict);
      }
   }
return (LPVOID)lpPict;
}
#endif  //not SS_NOSAVE
#endif  //SS_OCX


#define BTN_HALIGNPIC_LEFT   0
#define BTN_HALIGNPIC_CENTER 1
#define BTN_HALIGNPIC_RIGHT  2

#define BTN_VALIGNPIC_TOP    0
#define BTN_VALIGNPIC_CENTER 1
#define BTN_VALIGNPIC_BOTTOM 2


void SS_PaintPicture(LPSPREADSHEET lpSS, HDC hDC, HDC hDCDev, LPRECT lpRect, double dfScaleX, double dfScaleY,
                     HPALETTE hPal, HANDLE hPict, long lStyle, COLORREF clrBack, HBRUSH hBrush, BOOL bBackDiff,
                     BOOL IsLastCol, BOOL IsLastRow, BOOL fSelAlpha)
{
HBITMAP    hBitmapOld;
HPALETTE   hPalOld = 0;
HPALETTE   hPalMemOld = 0;
HDC        hDCMemory;
BYTE       bAlignPictH = BTN_HALIGNPIC_LEFT;
BYTE       bAlignPictV = BTN_VALIGNPIC_TOP;
RECT       RectOrig;
RECT       RectPict;
short      nPictWidth = 0;
short      nPictHeight = 0;
short      nPictWidthOrig;
short      nPictHeightOrig;
short      x = 0;
short      y = 0;
short      nRectWidth;
short      nRectHeight;
BOOL       fDeletePict;
#if SS_V80
BOOL       isTransparent = FALSE;
#endif
SaveDC(hDC);

clrBack = SS_TranslateColor(clrBack);  // BJO 20May96 JAP4167

if (hBrush && (!(lStyle & VPS_STRETCH) || (lStyle & VPS_MAINTAINSCALE) || !hPict))
{
   RECT rcBackgrnd;
   SS_CopyCellRect(lpSS, &rcBackgrnd, lpRect, NULL, bBackDiff, IsLastCol, IsLastRow);

#ifdef SS_V80
	if (fSelAlpha)
		fpAlphaFill(hDC, rcBackgrnd, hBrush, SS_TranslateColor(lpSS->lpBook->clrHighlightAlphaBlend), lpSS->lpBook->bHighlightAlphaBlend);
	else
#endif
	   FillRect(hDC, &rcBackgrnd, hBrush);
}

IntersectClipRect(hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);

if (lStyle & VPS_ICON)
   {
   nPictWidth = (short)tbGetIconWidth(hPict);
   nPictHeight = (short)tbGetIconHeight(hPict);
   }
else if (hPict)
   {
   BITMAP bm;

   GetObject(hPict, sizeof(BITMAP), (LPVOID)&bm);
   nPictWidth = (short)bm.bmWidth;
   nPictHeight = (short)bm.bmHeight;
#if SS_V80
/*   {
      BITMAPV5HEADER bmpv5;
      
      if( GetObject(hPict, sizeof(BITMAPV5HEADER), (LPVOID)&bmpv5) && bmpv5.bV5Intent )
         isTransparent = TRUE;
   }
   */
#endif
   }

nPictWidthOrig = nPictWidth;
nPictHeightOrig = nPictHeight;

if (lStyle & SSS_ALIGN_BOTTOM)
   bAlignPictV = BTN_VALIGNPIC_BOTTOM;
else if (lStyle & SSS_ALIGN_VCENTER)
   bAlignPictV = BTN_VALIGNPIC_CENTER;
else
   bAlignPictV = BTN_VALIGNPIC_TOP;

if (lStyle & SSS_ALIGN_RIGHT)
   bAlignPictH = BTN_HALIGNPIC_RIGHT;
else if (lStyle & SSS_ALIGN_CENTER)
   bAlignPictH = BTN_HALIGNPIC_CENTER;
else
   bAlignPictH = BTN_HALIGNPIC_LEFT;

/*
if (lStyle & VPS_CENTER)
   {
   bAlignPictH = BTN_HALIGNPIC_CENTER;
   bAlignPictV = BTN_VALIGNPIC_CENTER;
   }
*/

if (!hPict)
   {
   RestoreDC(hDC, -1);
   return;
   }

if ((lStyle & VPS_STRETCH) && !(lStyle & VPS_MAINTAINSCALE))
   {
   bAlignPictH = BTN_HALIGNPIC_LEFT;
   bAlignPictV = BTN_VALIGNPIC_TOP;
   }

CopyRect(&RectOrig, lpRect);

/****************************
* Determine size of picture
****************************/

if (lStyle & VPS_STRETCH)
   {
   if (!(lStyle & VPS_MAINTAINSCALE))
      {
      nPictWidth = (short)(lpRect->right - lpRect->left);
      nPictHeight = (short)(lpRect->bottom - lpRect->top);
      }

   else
      {
      nPictHeight = (short)(((long)(lpRect->right - lpRect->left) *
                             (long)nPictHeight) / (long)nPictWidth);

      if (nPictHeight <= lpRect->bottom - lpRect->top)
         nPictWidth = (short)(lpRect->right - lpRect->left);

      else
         {
         nPictWidth = (short)(((long)(lpRect->bottom - lpRect->top) *
                               (long)nPictWidthOrig) /
                               (long)nPictHeightOrig);
         nPictHeight = (short)(lpRect->bottom - lpRect->top);
         }
      }

   nPictWidthOrig = nPictWidth;
   nPictHeightOrig = nPictHeight;
   }

else if (dfScaleX > 0.0 && dfScaleY > 0.0 &&
        (dfScaleX != 1.0 || dfScaleY != 1.0))
   {
   nPictWidth = (short)((double)nPictWidth * dfScaleX);
   nPictHeight = (short)((double)nPictHeight * dfScaleY);
   }

nRectWidth = (short)(RectOrig.right - RectOrig.left);
nRectHeight = (short)(RectOrig.bottom - RectOrig.top);

if (bAlignPictH == BTN_HALIGNPIC_RIGHT)
   x = nRectWidth - nPictWidth;
else if (bAlignPictH == BTN_HALIGNPIC_CENTER)
   x = (nRectWidth - nPictWidth) >> 1;

if (bAlignPictV == BTN_VALIGNPIC_BOTTOM)
   y = nRectHeight - nPictHeight;
else if (bAlignPictV == BTN_VALIGNPIC_CENTER)
   y = (nRectHeight - nPictHeight) >> 1;

SetRect(&RectPict, x, y, x + nPictWidth, y + nPictHeight);
OffsetRect(&RectPict, RectOrig.left, RectOrig.top);

hPict = SSx_PictGetBitmap(hDCDev, &RectOrig, hPal, hPict, lStyle, clrBack, FALSE,
                          &fDeletePict);

x = (short)RectPict.left;
y = (short)RectPict.top;

if (dfScaleX > 0.0 && dfScaleY > 0.0 && (dfScaleX != 1.0 || dfScaleY != 1.0))
   {
   HANDLE   hDIB;
   HPALETTE hPalette;

   if (hPal)
      hDIB = tbBitmapToDIB(hPict, hPal);
   else
      {
      hPalette = tbGetSystemPalette();
      hDIB = tbBitmapToDIB(hPict, hPalette);
      DeleteObject(hPalette);
      }

   SetStretchBltMode(hDC, COLORONCOLOR);
   tbStretchDibBlt(hDC, x, y, nPictWidth, nPictHeight, hDIB, 0,
                   0, nPictWidthOrig, nPictHeightOrig, SRCCOPY);
   }

else
   {
   if (hPal)
      {
      hPalOld = SelectPalette(hDC, hPal, 0);
      RealizePalette(hDC);
      }

   hDCMemory = CreateCompatibleDC(hDCDev);

   if (hPal)
      {
      hPalMemOld = SelectPalette(hDCMemory, hPal, 0);
      RealizePalette(hDCMemory);
      }

   hBitmapOld = SelectObject(hDCMemory, hPict);

#if SS_V80
//   if( isTransparent && !fpAlphaBlend(hDC, x, y, nPictWidth, nPictHeight, hDCMemory, 0, 0, nPictWidth, nPictHeight) )
#endif
   // Copy the memory device context bitmap to the display
   StretchBlt(hDC, x, y, nPictWidth, nPictHeight, hDCMemory,
              0, 0, nPictWidth, nPictHeight, SRCCOPY);

   if (hPalOld)
      {
      SelectPalette(hDC, hPalOld, TRUE);
      RealizePalette(hDC);
      }

   if (hPalMemOld)
      {
      SelectPalette(hDCMemory, hPalMemOld, TRUE);
      RealizePalette(hDCMemory);
      }

   SelectObject(hDCMemory, hBitmapOld);
   DeleteDC(hDCMemory);
   }

if (fDeletePict)
   DeleteObject(hPict);

RestoreDC(hDC, -1);
}

#if _DEBUG
const LPCTSTR DLL_NOT_FOUND = _T("Could not find FpImage.dll.");
#else
const LPCTSTR DLL_NOT_FOUND = _T("Could not find FpImaged.dll.");
#endif

HANDLE SS_LoadPicture(HWND hWnd, LPCTSTR lpszFileName, long lStyle,
                      HPALETTE FAR *lphPal)
{
HANDLE hResource;
HANDLE hFindResource;
HANDLE hRet = 0;
HANDLE hDib = 0;
LPBYTE ResourceData = NULL;
long   ResourceDataSize = 0;
#ifdef SS_V70
char path[MAX_PATH] = {0};
#endif

if (!lpszFileName)
   return (0);

if (!(lStyle & VPS_HANDLE) && !lpszFileName[0])
   return (0);

#ifdef SS_V70
#ifdef _UNICODE
WideCharToMultiByte(CP_ACP, 0, lpszFileName, lstrlen(lpszFileName), path, MAX_PATH, NULL, NULL);
#else
strncpy(path, lpszFileName, __min(strlen(lpszFileName), MAX_PATH));
#endif
if( lStyle & VPS_RESOURCE )
{
#ifdef _DEBUG
  HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  FPLOADRESIMAGEPROC pFPLoadResImage = hInstDll ? (FPLOADRESIMAGEPROC)GetProcAddress(hInstDll, "FPLoadResImage") : NULL;
//  if( !pFPLoadResImage )
//  {
//    MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
//    return NULL;
//  }
  if( pFPLoadResImage )
    hRet = pFPLoadResImage(GetWindowInstance(hWnd), path, "VIEWPICT", CXIMAGE_FORMAT_BMP);
  if( !hRet && pFPLoadResImage )
    hRet = pFPLoadResImage(GetWindowInstance(hWnd), path, "BITMAP", CXIMAGE_FORMAT_BMP);
  if( !hRet && pFPLoadResImage )
  {
    int len = (int)strlen(path);
#ifdef _MBCS
    int n = _mbscspn(path, "|");
#else
    int n = (int)strcspn(path, "|");
#endif

    if( len > 0 && n < len )
    {
#ifdef _MBCS
      LPSTR lpszResName = (LPSTR)malloc(len * 2 + 1);
#else
      LPSTR lpszResName = (LPSTR)malloc(len + 1);
#endif
      LPSTR lpszResType = &lpszResName[n+1];
      strcpy(lpszResName, path);
      lpszResName[n] = 0;
      hRet = pFPLoadResImage(GetWindowInstance(hWnd), lpszResName, lpszResType, CXIMAGE_FORMAT_UNKNOWN); 
      free(lpszResName);
    }
  }
  if( hInstDll )
    FreeLibrary(hInstDll);
}
else
{
#ifdef _DEBUG
  HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  FPLOADIMAGEPROC pFPLoadImage = hInstDll ? (FPLOADIMAGEPROC)GetProcAddress(hInstDll, "FPLoadImage") : NULL;
//  if( !pFPLoadImage )
//  {
//    MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
//    return NULL;
//  }
  if( pFPLoadImage )
    hRet = pFPLoadImage(path, CXIMAGE_FORMAT_UNKNOWN);
  if( hInstDll )
    FreeLibrary(hInstDll);
}

if( hRet )
  return hRet;
#endif

if (!(lStyle & VPS_BMP) 
	&& !(lStyle & VPS_ICON))
   return (0);

if ((lStyle & VPS_RESOURCE) && (lStyle & VPS_ICON))
   hRet = LoadIcon(GetWindowInstance(hWnd), lpszFileName);

else if (lStyle & VPS_BMP)
   {
   if (lStyle & VPS_RESOURCE)
      {
      if (!(hFindResource = FindResource(GetWindowInstance(hWnd),
                                         lpszFileName, _T("VIEWPICT"))))
         if (!(hFindResource = FindResource(GetWindowInstance(hWnd),
                                            lpszFileName, _T("BITMAP"))))
            return (0);

      if (!(hResource = LoadResource(GetWindowInstance(hWnd), hFindResource)))
         return (0);

      else
         {
         ResourceDataSize = SizeofResource(GetWindowInstance(hWnd),
                                           hFindResource);
         ResourceData = LockResource(hResource);
         }
      }

   {
   HANDLE       hBmp = 0;

   if (hDib = tbOpenDIB(lpszFileName, ResourceData, ResourceDataSize))
      {
      // BJO 23Sep96 JAP5457 - Before fix
      //*lphPal = tbCreateDibPalette(hDib);
      //hRet = tbBitmapFromDib(hDib, *lphPal, TRUE);
      // BJO 23Sep96 JAP5457 - Begin fix
      if (lphPal)
         {
         *lphPal = tbCreateDibPalette(hDib);
         hRet = tbBitmapFromDib(hDib, *lphPal, TRUE);
         }
      else
         {
         hRet = tbBitmapFromDib(hDib, NULL, TRUE);
         }
      // BJO 23Sep96 JAP5457 - End fix
      GlobalFree(hDib);
      }
   }

   if (lStyle & VPS_RESOURCE)
      {
      UnlockResource(hResource);
      FreeResource(hResource);
      }
   }

return (hRet);
}


HBITMAP SS_PictGetBitmap(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                         LPBOOL lpfDeletePict)
{
SS_COLORTBLITEM ColorTblItem;
SS_CELLTYPE     CellType;
HBITMAP         hBitmap;
RECT            Rect;
HDC             hDC = GetDC(0);
#ifdef SS_V40
WORD wSpan;             // cell span
SS_COORD lColAnchor;    // span anchor column
SS_COORD lRowAnchor;    // span anchor row
SS_COORD lNumCols;      // span number of columns
SS_COORD lNumRows;      // span number of rows
#endif

SS_GetColorTblItem(lpSS, Col, Row, &ColorTblItem, NULL);

if (lpSS->LockColor.BackgroundId && SS_GetLock(lpSS, Col, Row, TRUE))
   SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

Rect.left = 0;
Rect.top = 0;
Rect.right = SS_GetColWidthInPixels(lpSS, Col);
Rect.bottom = SS_GetRowHeightInPixels(lpSS, Row);

#ifdef SS_V40
wSpan = SS_GetCellSpan(lpSS, Col, Row, &lColAnchor, &lRowAnchor, &lNumCols, &lNumRows);

if( SS_SPAN_ANCHOR == wSpan )
{ // add spanned column widths and row heights
  long l;

  for( l = Col + 1; l < Col + lNumCols; l++ )
    if( !SS_IsColHidden(lpSS, l) )
      Rect.right += SS_GetColWidthInPixels(lpSS, l);

  for( l = Row + 1; l < Row + lNumRows; l++ )
    if( !SS_IsRowHidden(lpSS, l) )
      Rect.bottom += SS_GetRowHeightInPixels(lpSS, l);
}
#endif

// fix for 9337 -scl

if( CellType.Spec.ViewPict.hPictName )
  hBitmap = SSx_PictGetBitmap(hDC, &Rect, CellType.Spec.ViewPict.hPal,
                              (HANDLE)CellType.Spec.ViewPict.hPictName,
                              CellType.Style, ColorTblItem.Color, TRUE,
                              lpfDeletePict);
else
  hBitmap = NULL;

ReleaseDC(0, hDC);

return (hBitmap);
}


#if 0
HBITMAP SS_PictGetBitmap(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                         LPBOOL lpfDeletePict)
{
SS_COLORTBLITEM ColorTblItem;
SS_CELLTYPE     CellType;
HBITMAP         hBitmap;
RECT            Rect;
HDC             hDC = GetDC(0);

SS_GetColorTblItem(lpSS, Col, Row, &ColorTblItem, NULL);

if (lpSS->LockColor.BackgroundId && SS_GetLock(lpSS, Col, Row, TRUE))
   SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

Rect.left = 0;
Rect.top = 0;
Rect.right = SS_GetColWidthInPixels(lpSS, Col);
Rect.bottom = SS_GetRowHeightInPixels(lpSS, Row);

wSpan = SS_GetCellSpan(lpSS, Col, Row, &lColAnchor, &lRowAnchor, &lNumCols, &lNumRows);
if( SS_SPAN_ANCHOR == wSpan )
{ // add spanned column widths and row heights
  long l;
  for( l = Col + 1; l < Col + lNumCols; l++ )
    if( !SS_IsColHidden(lpSS, l) )
      Rect.right += SS_GetColWidthInPixels(lpSS, l);
  for( l = Row + 1; l < Row + lNumRows; l++ )
    if( !SS_IsRowHidden(lpSS, l) )
      Rect.bottom += SS_GetRowHeightInPixels(lpSS, l);
}
// fix for 9337 -scl
if( CellType.Spec.ViewPict.hPictName )
  hBitmap = SSx_PictGetBitmap(hDC, &Rect, CellType.Spec.ViewPict.hPal,
                              (HANDLE)CellType.Spec.ViewPict.hPictName,
                              CellType.Style, ColorTblItem.Color, TRUE,
                              lpfDeletePict);
else
  hBitmap = NULL;

ReleaseDC(0, hDC);

return (hBitmap);
}
#endif


HBITMAP SSx_PictGetBitmap(HDC hDC, LPRECT lpRect, HPALETTE hPal, HANDLE hPict,
                          long lStyle, COLORREF clrBack, BOOL fCrop,
                          LPBOOL lpfDeletePict)
{
HBITMAP hPictStretch = 0;

if (lStyle & VPS_STRETCH)
   {
   if (lStyle & VPS_ICON)
      hPictStretch = tbCreateStretchedIcon(hDC, lpRect, clrBack,
                     hPict, (BOOL)((lStyle & VPS_MAINTAINSCALE) != 0));
   else
      {
      HPALETTE hPalOld;

      if (hPal)
         {
         hPalOld = SelectPalette(hDC, hPal, 0);
         RealizePalette(hDC);
         }

      hPictStretch = tbCreateStretchedBitmap(hDC, hPict, lpRect,
                     (BOOL)((lStyle & VPS_MAINTAINSCALE) != 0), hPal);

      if (hPal)
         {
         SelectPalette(hDC, hPalOld, TRUE);
         RealizePalette(hDC);
         }
      }
   }

else if (lStyle & VPS_ICON)
   {
   RECT RectIcon;

   SetRect(&RectIcon, 0, 0, tbGetIconWidth(hPict), tbGetIconHeight(hPict));
   hPictStretch = tbCreateStretchedIcon(hDC, &RectIcon, clrBack, hPict, FALSE);
   }

if (hPictStretch)
   hPict = hPictStretch;

// Determine if the picture needs cropped

if (fCrop)
   {
   BITMAP bm;

   GetObject(hPict, sizeof(BITMAP), (LPVOID)&bm);

   if (bm.bmWidth > lpRect->right - lpRect->left ||
       bm.bmHeight > lpRect->bottom - lpRect->top)
      {
      BITMAPINFOHEADER bi;
      HBITMAP          hBitmapDestOld;
      HBITMAP          hBitmapSrcOld;
      HBITMAP          hBitmapDest;
      HPALETTE         hPalMemSrcOld;
      HPALETTE         hPalMemDestOld;
      HDC              hDCMemSrc;
      HDC              hDCMemDest;
      int              x = 0;
      int              y = 0;
      int              iRectWidth = min(lpRect->right - lpRect->left, bm.bmWidth);
      int              iRectHeight = min(lpRect->bottom - lpRect->top, bm.bmHeight);
      int              iPictWidth = bm.bmWidth;
      int              iPictHeight = bm.bmHeight;

      if (lStyle & SSS_ALIGN_RIGHT)
         x = iRectWidth - iPictWidth;
      else if (lStyle & SSS_ALIGN_CENTER)
         x = (iRectWidth - iPictWidth) >> 1;

      if (lStyle & SSS_ALIGN_BOTTOM)
         y = iRectHeight - iPictHeight;
      else if (lStyle & SSS_ALIGN_VCENTER)
         y = (iRectHeight - iPictHeight) >> 1;

      hDCMemSrc = CreateCompatibleDC(hDC);
      hDCMemDest = CreateCompatibleDC(hDC);

      if (hPal)
         {
         hPalMemSrcOld = SelectPalette(hDCMemSrc, hPal, 0);
         RealizePalette(hDCMemSrc);
         hPalMemDestOld = SelectPalette(hDCMemDest, hPal, 0);
         RealizePalette(hDCMemDest);
         }

      hBitmapSrcOld = SelectObject(hDCMemSrc, hPict);

      // Create Dest Bitmap

      tbGetBitMapInfoHeader(hPict, &bi, (short)iRectWidth, (short)iRectHeight);

      if (!(hBitmapDest = CreateDIBitmap(hDC, &bi, 0L, NULL, NULL, 0)))
         return (0);

      hBitmapDestOld = SelectObject(hDCMemDest, hBitmapDest);

      // Copy the memory device context bitmap to the display
      BitBlt(hDCMemDest, 0, 0, iRectWidth, iRectHeight, hDCMemSrc,
             -x, -y, SRCCOPY);

      if (hPalMemSrcOld)
         SelectPalette(hDCMemSrc, hPalMemSrcOld, TRUE);

      SelectObject(hDCMemSrc, hBitmapSrcOld);
      DeleteDC(hDCMemSrc);

      if (hPalMemDestOld)
         SelectPalette(hDCMemDest, hPalMemDestOld, TRUE);

      SelectObject(hDCMemDest, hBitmapDestOld);
      DeleteDC(hDCMemDest);

      if (hPictStretch)
         DeleteObject(hPictStretch);

      hPictStretch = hBitmapDest;
      hPict = hBitmapDest;
      }
   }

if (lpfDeletePict)
   *lpfDeletePict = (BOOL)(hPictStretch != 0);

return (hPict);
}
