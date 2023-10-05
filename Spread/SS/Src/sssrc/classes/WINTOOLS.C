/*********************************************************
* WINTOOLS.C
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
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <process.h>
#include <limits.h>
#include <math.h>
#include "toolbox.h"
#include "wintools.h"
#if SS_V80
#include "ss_gradient.h"
#endif
#if 0
void LogMsg(Str, ...)

TCHAR  *Str;
{
FILE   *fp;
TCHAR   Buffer[150 + 1];
va_list ArgPtr;

va_start(ArgPtr, Str);
vsprintf(Buffer, Str, ArgPtr);
va_end(ArgPtr);

fp = fopen("Msg.log", "a");
fprintf(fp, "%s\n", Buffer);
fclose(fp);
}
#endif


void DLLENTRY DlgBoxCenter(hWnd)

HWND  hWnd;
{
short x;
short y;
short width;
short height;
RECT  Rect;

/***********************************
* Query width and height of screen
***********************************/

width = GetSystemMetrics(SM_CXSCREEN);
height = GetSystemMetrics(SM_CYSCREEN);

/***************************************
* Query width and height of dialog box
***************************************/

GetWindowRect(hWnd, &Rect);

/**********************************
* Center dialog box within screen
**********************************/

x = (short)((width - (Rect.right - Rect.left)) / 2);
y = (short)((height - (Rect.bottom - Rect.top)) / 2);

SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


LRESULT tbPostMessageToParent(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
return (PostMessage(GetParent(hWnd), Msg, wParam, lParam));
}


LRESULT tbSendMessageToParent(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
SendMessage(hWnd, Msg, wParam, lParam);
return (SendMessage(GetParent(hWnd), Msg, wParam, lParam));
}


void tbDrawShadows(hDC, lpRect, iShadowSize, ColorShadow, ColorHighlight,
                   fShadowIn, fShadowButton)

HDC      hDC;
LPRECT   lpRect;
short    iShadowSize;
COLORREF ColorShadow;
COLORREF ColorHighlight;
BOOL     fShadowIn;
BOOL     fShadowButton;
{
HBRUSH   hBrush;
HBRUSH   hBrushOld;
short    i;

iShadowSize = (short)min(iShadowSize, (lpRect->right - lpRect->left) / 2);
iShadowSize = (short)min(iShadowSize, (lpRect->bottom - lpRect->top) / 2);

/**********************
* Create left and top
**********************/

hBrush = CreateSolidBrush(fShadowIn ? ColorShadow : ColorHighlight);
hBrushOld = SelectObject(hDC, hBrush);

/*******
* Left
*******/

for (i = 0; i < iShadowSize; i++)
   PatBlt(hDC, lpRect->left + i, lpRect->top, 1, lpRect->bottom -
          lpRect->top - i, PATCOPY);

/******
* Top
******/

for (i = 0; i < iShadowSize; i++)
   PatBlt(hDC, lpRect->left, lpRect->top + i, lpRect->right -
          lpRect->left - i, 1, PATCOPY);

SelectObject(hDC, hBrushOld);
DeleteObject(hBrush);

/**************************
* Create right and bottom
**************************/

if (!fShadowButton || !fShadowIn)
   {
   hBrush = CreateSolidBrush(fShadowIn ? ColorHighlight : ColorShadow);
   hBrushOld = SelectObject(hDC, hBrush);

   /********
   * Right
   ********/

   for (i = 0; i < iShadowSize; i++)
      PatBlt(hDC, lpRect->right - i - 1, lpRect->top + i, 1, lpRect->bottom -
             lpRect->top - i, PATCOPY);

   /*********
   * Bottom
   *********/

   for (i = 0; i < iShadowSize; i++)
      PatBlt(hDC, lpRect->left + i, lpRect->bottom - i - 1, lpRect->right -
             lpRect->left - i, 1, PATCOPY);

   SelectObject(hDC, hBrushOld);
   DeleteObject(hBrush);
   }
}


void tbShrinkRect(lpRect, x, y)

LPRECT lpRect;
short  x;
short  y;
{
lpRect->left += x;
lpRect->top += y;
lpRect->right -= x;
lpRect->bottom -= y;
}


void tbDrawIcon(hDC, lpRect, Color, x, y, hIcon)

HDC        hDC;
LPRECT     lpRect;
COLORREF   Color;
short      x;
short      y;
HICON      hIcon;
{
HBITMAP    hBitmap;
HBITMAP    hBitmapOld;
HDC        hDCMemory;
HBRUSH     hBrush;
RECT       Rect;
int        Width;
int        Height;
int        OrgX;
int        OrgY;

Width = tbGetIconWidth(hIcon);
Height = tbGetIconHeight(hIcon);

/*******************************************************
* Create Bitmap to select to the memory device context
*******************************************************/

hBitmap = CreateCompatibleBitmap(hDC, Width, Height);

/*********************************
* Create a memory device context
*********************************/

hDCMemory = CreateCompatibleDC(hDC);
hBitmapOld = SelectObject(hDCMemory, hBitmap);
SetMapMode(hDCMemory, MM_TEXT);

/*****************************************
* Fill the hDCMemory background with the
* current background of the display
*****************************************/

hBrush = CreateSolidBrush(Color);
SetRect(&Rect, 0, 0, Width, Height);
FillRect(hDCMemory, &Rect, hBrush);
DeleteObject(hBrush);

/*********************************************
* Draw the icon on the memory device context
*********************************************/

#ifdef WIN32
DrawIconEx(hDCMemory, 0, 0, hIcon, Width, Height, 0, NULL, DI_NORMAL);
#else
DrawIcon(hDCMemory, 0, 0, hIcon);
#endif

if (x == SHRT_MIN)
   x = (short)(lpRect->left + (((lpRect->right - lpRect->left) - Width) / 2));

if (y == SHRT_MIN)
   y = (short)(lpRect->top + (((lpRect->bottom - lpRect->top) - Height) / 2));

Width = min(Width, lpRect->right - lpRect->left);
Height = min(Height, lpRect->bottom - lpRect->top);

if (x < lpRect->left)
   OrgX = lpRect->left - x;
else
   OrgX = 0;

if (y < lpRect->top)
   OrgY = lpRect->top - y;
else
   OrgY = 0;

/*******************************************************
* Copy the memory device context bitmap to the display
*******************************************************/

BitBlt(hDC, max(x, lpRect->left), max(y, lpRect->top), Width, Height,
       hDCMemory, OrgX, OrgY, SRCCOPY);

/***********
* Clean up
***********/

SelectObject(hDCMemory, hBitmapOld);
DeleteDC(hDCMemory);
DeleteObject(hBitmap);
}


void tbDrawBitmap(hDC, hBitmap, lpRect, x, y, dwRop)

HDC     hDC;
HBITMAP hBitmap;
LPRECT  lpRect;
#if defined(_WIN64) || defined(_IA64)
long    x;
long    y;
#else
short   x;
short   y;
#endif
DWORD   dwRop;
{
BITMAP  bm;
HBITMAP hBitmapOld;
HDC     hDCMem;
POINT   ptSize;
POINT   ptOrg;
#if SS_V80
BOOL    isTransparent = FALSE;
#endif

hDCMem = CreateCompatibleDC(hDC);
hBitmapOld = SelectObject(hDCMem, hBitmap);
SetMapMode(hDCMem, GetMapMode(hDC));

GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm);
#if SS_V80
/*
{
   BITMAPV5HEADER bmpv5;

   if( GetObject(hBitmap, sizeof(BITMAPV5HEADER), (LPVOID)&bmpv5) && bmpv5.bV5Intent )
      isTransparent = TRUE;
}
*/
#endif
ptSize.x = min(bm.bmWidth, lpRect->right - lpRect->left);
ptSize.y = min(bm.bmHeight, lpRect->bottom - lpRect->top);

/* RFW 12/15
if (x < 0)
   ptOrg.x = -x;
else
   ptOrg.x = 0;

if (y < 0)
   ptOrg.y = -y;
else
   ptOrg.y = 0;
*/

if (x < lpRect->left)
   ptOrg.x = lpRect->left - x;
else
   ptOrg.x = 0;

if (y < lpRect->top)
   ptOrg.y = lpRect->top - y;
else
   ptOrg.y = 0;

#if SS_V80
//if( isTransparent && !fpAlphaBlend(hDC, max(x, lpRect->left), max(y, lpRect->top), bm.bmWidth - ptOrg.x, bm.bmHeight - ptOrg.y,
//       hDCMem, ptOrg.x, ptOrg.y, bm.bmWidth - ptOrg.x, bm.bmHeight - ptOrg.y) )
#endif
BitBlt(hDC, max(x, lpRect->left), max(y, lpRect->top), ptSize.x, ptSize.y,
       hDCMem, ptOrg.x, ptOrg.y, dwRop);

SelectObject(hDCMem, hBitmapOld);
DeleteDC(hDCMem);
}


HBITMAP tbCreateStretchedIcon(HDC hDC, LPRECT lpRect, COLORREF Color,
                              HICON hIcon, BOOL fProportionally)
{
HBITMAP    hBitmap;
HBITMAP    hBitmapNew;
HBITMAP    hBitmapOld;
HDC        hDCMemory;
HBRUSH     hBrush;
RECT       Rect;
int        Width;
int        Height;

Width = tbGetIconWidth(hIcon);
Height = tbGetIconHeight(hIcon);

/*******************************************************
* Create Bitmap to select to the memory device context
*******************************************************/

hBitmap = CreateCompatibleBitmap(hDC, Width, Height);

/*********************************
* Create a memory device context
*********************************/

hDCMemory = CreateCompatibleDC(hDC);
hBitmapOld = SelectObject(hDCMemory, hBitmap);
SetMapMode(hDCMemory, MM_TEXT);

/*****************************************
* Fill the hDCMemory background with the
* current background of the display
*****************************************/

hBrush = CreateSolidBrush(Color);
SetRect(&Rect, 0, 0, Width, Height);
FillRect(hDCMemory, &Rect, hBrush);
DeleteObject(hBrush);

/*********************************************
* Draw the icon on the memory device context
*********************************************/

#ifdef WIN32
DrawIconEx(hDCMemory, 0, 0, hIcon, Width, Height, 0, NULL, DI_NORMAL);
#else
DrawIcon(hDCMemory, 0, 0, hIcon);
#endif

SelectObject(hDCMemory, hBitmapOld);
DeleteDC(hDCMemory);

if (Width == lpRect->right - lpRect->left && Height == lpRect->bottom -
    lpRect->top)
   hBitmapNew = hBitmap;

else
   {
   /*********************
   * Stretch the bitmap
   *********************/

   hBitmapNew = tbCreateStretchedBitmap(hDC, hBitmap, lpRect, fProportionally, 0);

   /***********
   * Clean up
   ***********/

   DeleteObject(hBitmap);
   }

return (hBitmapNew);
}


HBITMAP tbCreateStretchedBitmap(HDC hDC, HBITMAP hBitmap, LPRECT lpRect,
                                BOOL fProportionally, HPALETTE hPal)
{
BITMAPINFOHEADER bi;
BITMAP           bm;
HPALETTE         hPalSrcOld;
HPALETTE         hPalDestOld;
HBITMAP          hBitmapSrcOld;
HBITMAP          hBitmapDestOld;
HBITMAP          hBitmapDest;
HDC              hDCMemSrc;
HDC              hDCMemDest;
short            nWidth;
short            nHeight;

// fix for 14914 -scl
if(!GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm))
   return NULL;

if (fProportionally)
   {
   nHeight = (short)(((long)(lpRect->right - lpRect->left) *
                      (long)bm.bmHeight) / (long)bm.bmWidth);

   if (nHeight <= lpRect->bottom - lpRect->top)
      nWidth = (short)(lpRect->right - lpRect->left);

   else
      {
      nWidth = (short)(((long)(lpRect->bottom - lpRect->top) *
                        (long)bm.bmWidth) / (long)bm.bmHeight);
      nHeight = (short)(lpRect->bottom - lpRect->top);
      }
   }
else
   {
   nWidth = (short)(lpRect->right - lpRect->left);
   nHeight = (short)(lpRect->bottom - lpRect->top);
   }

if (nWidth == bm.bmWidth * 2 && nHeight == bm.bmHeight * 2)
   {
   nWidth--;
   nHeight--;
   }

/**********************
* Set up the Dest hDC
**********************/

hDCMemDest = CreateCompatibleDC(hDC);
if (hPal)
   {
   hPalDestOld = SelectPalette(hDCMemDest, hPal, TRUE);
   RealizePalette(hDCMemDest);
   }

tbGetBitMapInfoHeader(hBitmap, &bi, nWidth, nHeight);

if (!(hBitmapDest = CreateDIBitmap(hDC, &bi, 0L, NULL, NULL, 0)))
   return (0);

//hBitmapDest = CreateCompatibleBitmap(hDC, nWidth, nHeight);

hBitmapDestOld = SelectObject(hDCMemDest, hBitmapDest);
SetMapMode(hDCMemDest, GetMapMode(hDC));
SetStretchBltMode(hDCMemDest, COLORONCOLOR);

/*********************
* Set up the Src hDC
*********************/

hDCMemSrc = CreateCompatibleDC(hDC);

if (hPal)
   {
   hPalSrcOld = SelectPalette(hDCMemSrc, hPal, TRUE);
   RealizePalette(hDCMemSrc);
   }

hBitmapSrcOld = SelectObject(hDCMemSrc, hBitmap);
SetMapMode(hDCMemSrc, GetMapMode(hDC));

StretchBlt(hDCMemDest, 0, 0, nWidth, nHeight, hDCMemSrc, 0, 0, bm.bmWidth,
           bm.bmHeight, SRCCOPY);

SelectObject(hDCMemDest, hBitmapDestOld);
SelectObject(hDCMemSrc, hBitmapSrcOld);
if (hPal)
   {
   SelectPalette(hDCMemDest, hPalDestOld, TRUE);
   SelectPalette(hDCMemSrc, hPalSrcOld, TRUE);
   }
DeleteDC(hDCMemDest);
DeleteDC(hDCMemSrc);

return (hBitmapDest);
}


void tbGetBitMapInfoHeader(HBITMAP hBitmap, LPBITMAPINFOHEADER lpBI,
                           short nWidth, short nHeight)
{
BITMAP Bitmap;
double dfTemp;

_fmemset(lpBI, '\0', sizeof(BITMAPINFOHEADER));

GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);

lpBI->biSize = sizeof(BITMAPINFOHEADER);
lpBI->biWidth = nWidth;
lpBI->biHeight = nHeight;
lpBI->biPlanes = 1;
lpBI->biBitCount = Bitmap.bmPlanes * Bitmap.bmBitsPixel;
lpBI->biCompression = BI_RGB;
dfTemp = pow(2.0,(double)lpBI->biBitCount * (double)lpBI->biPlanes);
lpBI->biClrUsed = (short)dfTemp;
}


WORD tbSetDIBitsToDevice(hDC, hDib, lpRect, x, y)

HDC                hDC;
HANDLE             hDib;
LPRECT             lpRect;
short              x;
short              y;
{
LPBITMAPINFOHEADER lpbi;
LPBYTE             pBuf;
POINT              ptSize;
POINT              ptOrg;
WORD               wRet;

lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);

pBuf = (LPBYTE)lpbi + *(LPDWORD)lpbi + tbPaletteSize(lpbi);

ptSize.x = min((int)lpbi->biWidth, lpRect->right - lpRect->left);
ptSize.y = min((int)lpbi->biHeight, lpRect->bottom - lpRect->top);

if (x < 0)
   ptOrg.x = -x;
else
   ptOrg.x = 0;

if (y < 0)
   ptOrg.y = -y;
else
   ptOrg.y = 0;

wRet = SetDIBitsToDevice(hDC, max(x, lpRect->left), max(y, lpRect->top),
                         ptSize.x, ptSize.y, ptOrg.x, ptOrg.y, 0,
                         (WORD)lpbi->biHeight, pBuf, (LPBITMAPINFO)lpbi,
                         DIB_RGB_COLORS);

GlobalUnlock(hDib);
GlobalFree(hDib);

return (wRet);
}


BOOL tbStretchDibBlt(hDC, x, y, dx, dy, hDib, x0, y0, dx0, dy0, Rop)

HDC                hDC;
int                x;
int                y;
int                dx;
int                dy;
HANDLE             hDib;
int                x0;
int                y0;
int                dx0;
int                dy0;
LONG               Rop;
{
LPBITMAPINFOHEADER lpbi;
LPBYTE             pBuf;
BOOL               fRet;

lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);

pBuf = (LPBYTE)lpbi + *(LPDWORD)lpbi + tbPaletteSize(lpbi);

fRet = StretchDIBits(hDC, x, y, dx, dy, x0, y0, dx0, dy0,
                     pBuf, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS, Rop);

GlobalUnlock(hDib);
GlobalFree(hDib);

return (fRet);
}


HDC tbGetPrinterDC(void)
{
static TCHAR szPrinter[80];
TCHAR       *szDevice;
TCHAR       *szDriver;
TCHAR       *szOutput;

GetProfileString(_T("Windows"), _T("device"), _T(",,,"), szPrinter, 80);

if ((szDevice = _tcstok(szPrinter, _T(","))) &&
    (szDriver = _tcstok(NULL, _T(","))) &&
    (szOutput = _tcstok(NULL, _T(","))))
   return (CreateDC(szDriver, szDevice, szOutput, NULL));

return (0);
}


WORD tbPaletteSize(pv)

VOID FAR          *pv;
{
LPBITMAPINFOHEADER lpbi;
WORD               NumColors;

lpbi      = (LPBITMAPINFOHEADER)pv;
NumColors = tbDibNumColors(lpbi);

if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
   return (NumColors * sizeof(RGBTRIPLE));
else
   return (NumColors * sizeof(RGBQUAD));
}


WORD tbDibNumColors(pv)

VOID FAR          *pv;
{
int                bits;
LPBITMAPINFOHEADER lpbi;
LPBITMAPCOREHEADER lpbc;

lpbi = ((LPBITMAPINFOHEADER)pv);
lpbc = ((LPBITMAPCOREHEADER)pv);

/*  With the BITMAPINFO format headers, the size of the palette
 *  is in biClrUsed, whereas in the BITMAPCORE - style headers, it
 *  is dependent on the bits per pixel ( = 2 raised to the power of
 *  bits/pixel).
 */

if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
   {
   if (lpbi->biClrUsed != 0)
      return ((WORD)lpbi->biClrUsed);

   bits = lpbi->biBitCount;
   }

else
   bits = lpbc->bcBitCount;

switch (bits)
   {
   case 1:
      return (2);
   case 4:
      return (16);
   case 8:
      return (256);
   default:
      /* A 24 bitcount DIB has no color table */
      return (0);
   }
}


long tbGetIconWidth(HICON hIcon)
{
long lRet = GetSystemMetrics(SM_CXICON);

#ifdef WIN32
ICONINFO iconInfo;
BITMAP bm;

if(hIcon && GetIconInfo(hIcon, &iconInfo))
{
  if (iconInfo.hbmColor)
     {
     GetObject(iconInfo.hbmColor, sizeof(BITMAP), (LPSTR)&bm);
     lRet = bm.bmWidth;
     } 
  else    // monochrome, and hbmMask is AND and XOR bitmap
     {
     GetObject(iconInfo.hbmMask, sizeof(BITMAP), (LPSTR)&bm);           
     lRet = bm.bmWidth / 2;
     }
  DeleteObject(iconInfo.hbmMask);
  DeleteObject(iconInfo.hbmColor);
}
#endif

return lRet;
}


long tbGetIconHeight(HICON hIcon)
{
long lRet = GetSystemMetrics(SM_CYICON);

#ifdef WIN32
ICONINFO iconInfo;
BITMAP bm;

if(hIcon && GetIconInfo(hIcon, &iconInfo))
{
  if (iconInfo.hbmColor)
     {
     GetObject(iconInfo.hbmColor, sizeof(BITMAP), (LPSTR)&bm);
     lRet = bm.bmHeight;
     } 
  else    // monochrome, and hbmMask is AND and XOR bitmap
     {
     GetObject(iconInfo.hbmMask, sizeof(BITMAP), (LPSTR)&bm);           
     lRet = bm.bmHeight / 2;
     }
  DeleteObject(iconInfo.hbmMask);
  DeleteObject(iconInfo.hbmColor);
}
#endif

return lRet;
}