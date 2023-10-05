/*    PortTool v2.2     FPGRAPHC.C          */


#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <process.h>
#include <tchar.h>

#include "fptools.h"

#define WIDTHBYTES(i)            ((i+31)/32*4)

void fpGetBitMapInfoHeader(HBITMAP hBitmap, LPBITMAPINFOHEADER lpBI,
                           short nWidth, short nHeight);


long fpGetIconWidth(HICON hIcon)
{
long lRet = GetSystemMetrics(SM_CXICON);

#ifdef WIN32
ICONINFO iconInfo;
BITMAP bm;

if(GetIconInfo(hIcon, &iconInfo))
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

long fpGetIconHeight(HICON hIcon)
{
long lRet = GetSystemMetrics(SM_CYICON);

#ifdef WIN32
ICONINFO iconInfo;
BITMAP bm;

if(GetIconInfo(hIcon, &iconInfo))
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

void FPLIB fpDrawIcon(HDC hDC, HICON hIcon, LPRECT lpRect, short x, short y, COLORREF Color)
{
HBITMAP    hBitmap;
HBITMAP    hBitmapOld;
HDC        hDCMemory;
HBRUSH     hBrush;
RECT       Rect;
short      Width;
short      Height;

Width  = (short)fpGetIconWidth(hIcon);
Height = (short)fpGetIconHeight(hIcon);

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

hBrush = CreateSolidBrush(FPCOLOR(Color));
SetRect(&Rect, 0, 0, Width, Height);
FillRect(hDCMemory, &Rect, hBrush);
DeleteObject(hBrush);

/*********************************************
* Draw the icon on the memory device context
*********************************************/

#ifdef WIN32
DrawIconEx(hDCMemory, 0, 0,
           hIcon, Width, Height,
           0,
           NULL, DI_NORMAL);
#else
DrawIcon(hDCMemory, 0, 0, hIcon);
#endif

/*******************************************************
* Copy the memory device context bitmap to the display
*******************************************************/

BitBlt(hDC, lpRect->left, lpRect->top, Width, Height,
       hDCMemory, 0, 0, SRCCOPY);

/***********
* Clean up
***********/

SelectObject(hDCMemory, hBitmapOld);
DeleteDC(hDCMemory);
DeleteObject(hBitmap);
}

void FPLIB fpDrawBitmap(HDC hDC, HBITMAP hBitmap, LPRECT lpRect, short x, short y)
{
BITMAP  bm;
HBITMAP hBitmapOld;
HDC     hDCMem;
POINT   ptSize;
POINT   ptOrg;

hDCMem = CreateCompatibleDC(hDC);
hBitmapOld = SelectObject(hDCMem, hBitmap);
SetMapMode(hDCMem, GetMapMode(hDC));

GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

ptSize.x = min(bm.bmWidth, lpRect->right - lpRect->left);
ptSize.y = min(bm.bmHeight, lpRect->bottom - lpRect->top);

if (x < 0)
   ptOrg.x = -x;
else
   ptOrg.x = 0;

if (y < 0)
   ptOrg.y = -y;
else
   ptOrg.y = 0;

BitBlt(hDC, max(x, lpRect->left), max(y, lpRect->top), ptSize.x, ptSize.y,
       hDCMem, ptOrg.x, ptOrg.y, SRCCOPY);

SelectObject(hDCMem, hBitmapOld);
DeleteDC(hDCMem);
}


HBITMAP FPLIB fpCreateStretchedIcon(HDC hDC, LPRECT lpRect, COLORREF Color,
                              HICON hIcon, BOOL fProportionally)
{
HBITMAP    hBitmap;
HBITMAP    hBitmapNew;
HBITMAP    hBitmapOld;
HDC        hDCMemory;
HBRUSH     hBrush;
RECT       Rect;
short      Width;
short      Height;

Width = (short)fpGetIconWidth(hIcon);
Height = (short)fpGetIconHeight(hIcon);

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

hBrush = CreateSolidBrush(FPCOLOR(Color));
SetRect(&Rect, 0, 0, Width, Height);
FillRect(hDCMemory, &Rect, hBrush);
DeleteObject(hBrush);

/*********************************************
* Draw the icon on the memory device context
*********************************************/

#ifdef WIN32
DrawIconEx(hDCMemory, 0, 0,
           hIcon, Width, Height,
           0,
           NULL, DI_NORMAL);
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

   hBitmapNew = fpCreateStretchedBitmap(hDC, hBitmap, lpRect, fProportionally, 0);

   /***********
   * Clean up
   ***********/

   DeleteObject(hBitmap);
   }

return (hBitmapNew);
}


HBITMAP FPLIB fpCreateStretchedBitmap(HDC hDC, HBITMAP hBitmap, LPRECT lpRect,
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

GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

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

hBitmapDestOld = SelectObject(hDCMemDest, hBitmap);

fpGetBitMapInfoHeader(hBitmap, &bi, nWidth, nHeight);

if (!(hBitmapDest = CreateDIBitmap(hDC, &bi, 0L, NULL, NULL, 0)))
   return (0);

//hBitmapDest = CreateCompatibleBitmap(hDC, nWidth, nHeight);

SelectObject(hDCMemDest, hBitmapDest);
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


void fpGetBitMapInfoHeader(HBITMAP hBitmap, LPBITMAPINFOHEADER lpBI,
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


WORD FPLIB fpSetDIBitsToDevice(HDC hDC, HANDLE hDib, LPRECT lpRect, short x, short y)
{
LPBITMAPINFOHEADER lpbi;
LPSTR              pBuf;
POINT              ptSize;
POINT              ptOrg;
WORD               wRet;

lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);

pBuf = (LPSTR)lpbi + *(LPDWORD)lpbi + PaletteSize(lpbi);

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

/*    PortTool v2.2     5/10/1995    20:40          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
wRet = (WORD)SetDIBitsToDevice(hDC, max(x, lpRect->left), max(y, lpRect->top),
                         ptSize.x, ptSize.y, ptOrg.x, ptOrg.y, 0,
                         (UINT)lpbi->biHeight, pBuf, (LPBITMAPINFO)lpbi,
                         DIB_RGB_COLORS);

GlobalUnlock(hDib);
GlobalFree(hDib);

return (wRet);
}


BOOL FPLIB StretchDibBlt(HDC hDC, int x, int y, int dx, int dy, HANDLE hDib, 
                          int x0, int y0, int dx0, int dy0, LONG Rop)
{
LPBITMAPINFOHEADER lpbi;
LPSTR              pBuf;
BOOL               fRet;

lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);

pBuf = (LPSTR)lpbi + *(LPDWORD)lpbi + PaletteSize(lpbi);

fRet = StretchDIBits(hDC, x, y, dx, dy, x0, y0, dx0, dy0,
                     pBuf, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS, Rop);
                                                 
GlobalUnlock(hDib);
GlobalFree(hDib);

return (fRet);
}


HDC FPLIB fpGetPrinterDC(void)
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


WORD FPLIB PaletteSize(LPVOID pv)
{
LPBITMAPINFOHEADER lpbi;
WORD               NumColors;

lpbi      = (LPBITMAPINFOHEADER)pv;
NumColors = DibNumColors(lpbi);

if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
   return (NumColors * sizeof(RGBTRIPLE));
else
   return (NumColors * sizeof(RGBQUAD));
}


WORD FPLIB DibNumColors(LPVOID pv)
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

/////////////////////////////////////////////////////////
void FPLIB fpDrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart,
                                    short yStart, COLORREF cTransparentColor, DWORD dwPaintFlag, HPALETTE hPalette)
{
   fpDrawTransparentBitmapEx(hdc, hBitmap, xStart, yStart, 0, 0,
                             cTransparentColor, dwPaintFlag, hPalette);
}

void FPLIB fpDrawTransparentBitmapEx(HDC hdc, HBITMAP hBitmap, short xStart, short yStart, 
                                    short nWidth, short nHeight, COLORREF cTransparentColor, 
                                    DWORD dwPaintFlag, HPALETTE hPalette)
{
   BITMAP     bm;
   COLORREF   cColor;
   HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
   HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
   HPALETTE   hpalBackOld, hpalObjectOld, hpalMemOld, hpalSaveOld;
   HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
   POINT      ptSize;

   hdcTemp = CreateCompatibleDC(hdc);
   SelectObject(hdcTemp, hBitmap);   // Select the bitmap
   if (hPalette)
      {
      SelectPalette(hdcTemp, hPalette, 0);
      RealizePalette(hdcTemp);
      }
      
   if (cTransparentColor != NOT_TRANSPARENT)
      {
      if (GetNearestColor(hdcTemp, cTransparentColor) != cTransparentColor)
         cTransparentColor =  NOT_TRANSPARENT;
      }
 
  GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
   
   ptSize.x = (nWidth ? nWidth : bm.bmWidth);      // Get width of bitmap
   ptSize.y = (nHeight ? nHeight : bm.bmHeight);   // Get height of bitmap

   DPtoLP(hdcTemp, &ptSize, 1);                    // Convert from device
                                                   // to logical points
 
   if (cTransparentColor != NOT_TRANSPARENT)   // true transparency
   {
      // Create some DCs to hold temporary data.
      hdcBack   = CreateCompatibleDC(hdc);
      hdcObject = CreateCompatibleDC(hdc);
      hdcMem    = CreateCompatibleDC(hdc);
      hdcSave   = CreateCompatibleDC(hdc);
 
      // Create a bitmap for each DC. DCs are required for a number of
      // GDI functions.
 
      // Monochrome DC
      bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
 
      // Monochrome DC
      bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
 
      bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
      bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
 
      // Each DC must select a bitmap object to store pixel data.
      bmBackOld   = SelectObject(hdcBack, bmAndBack);
      bmObjectOld = SelectObject(hdcObject, bmAndObject);
      bmMemOld    = SelectObject(hdcMem, bmAndMem);
      bmSaveOld   = SelectObject(hdcSave, bmSave);
 
      if (hPalette)
         {
         hpalBackOld   = SelectPalette(hdcBack, hPalette, 0);
         hpalObjectOld = SelectPalette(hdcObject, hPalette, 0);
         hpalMemOld    = SelectPalette(hdcMem, hPalette, 0);
         hpalSaveOld   = SelectPalette(hdcSave, hPalette, 0);

         RealizePalette(hdcBack);
         RealizePalette(hdcObject);
         RealizePalette(hdcMem);
         RealizePalette(hdcSave);
         }
         
      // Set proper mapping mode.
      SetMapMode(hdcTemp, GetMapMode(hdc));
 
      // Save the bitmap sent here, because it will be overwritten.
      BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);
 
      // Set the background color of the source DC to the color.
      // contained in the parts of the bitmap that should be transparent
      cColor = SetBkColor(hdcTemp, FPCOLOR(cTransparentColor));
 
      // Create the object mask for the bitmap by performing a BitBlt
      // from the source bitmap to a monochrome bitmap.
      BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
             SRCCOPY);
 
      // Set the background color of the source DC back to the original
      // color.
      SetBkColor(hdcTemp, FPCOLOR(cColor));
 
      // Create the inverse of the object mask.
      BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
             NOTSRCCOPY);
 
      // Copy the background of the main DC to the destination.
      BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
             SRCCOPY);
 
      // Mask out the places where the bitmap will be placed.
      BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);
 
      // Mask out the transparent colored pixels on the bitmap.
      BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);
 
      // XOR the bitmap with the background on the destination DC.
      BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);
 
      // Copy the destination to the screen.
      BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
             SRCCOPY);
 
      // Place the original bitmap back into the bitmap sent here.
      BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);
 
      // Delete the memory bitmaps.
      DeleteObject(SelectObject(hdcBack, bmBackOld));
      DeleteObject(SelectObject(hdcObject, bmObjectOld));
      DeleteObject(SelectObject(hdcMem, bmMemOld));
      DeleteObject(SelectObject(hdcSave, bmSaveOld));
 
      if (hPalette)
         {
         SelectPalette(hdcBack, hpalBackOld, 0);
         SelectPalette(hdcObject, hpalObjectOld, 0);
         SelectPalette(hdcMem, hpalMemOld, 0);
         SelectPalette(hdcSave, hpalSaveOld, 0);

         RealizePalette(hdcBack);
         RealizePalette(hdcObject);
         RealizePalette(hdcMem);
         RealizePalette(hdcSave);
         }

      // Delete the memory DCs.
      DeleteDC(hdcMem);
      DeleteDC(hdcBack);
      DeleteDC(hdcObject);
      DeleteDC(hdcSave);
   }
   else                // just render the bitmap
   {
      // if NOT_TRANSPARENT use the dwPaintFlag to determine paint style
      if (!dwPaintFlag)
         dwPaintFlag = SRCCOPY;
         
      BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcTemp, 0, 0,
             dwPaintFlag);
   }
   
   DeleteDC(hdcTemp);
}

 
void FPLIB fpRenderMetaFile(HDC hDC, HMETAFILE hMeta, LPRECT lpRect, short x, short y, COLORREF Color)
{

  if (hMeta) 
     {
	 SIZE sizeTemp;
	 POINT pointTemp;

     SetMapMode(hDC, MM_ANISOTROPIC);

     SetWindowOrgEx(hDC, 0, 0, &pointTemp);
     SetWindowExtEx(hDC, lpRect->right, lpRect->bottom, &sizeTemp);

     SetViewportOrgEx(hDC, 0, 0, &pointTemp);
     SetViewportExtEx(hDC, lpRect->right, lpRect->bottom, &sizeTemp);
#ifdef WIN32    
     if (PlayMetaFile(hDC, hMeta) == 0);
       PlayEnhMetaFile(hDC, (HENHMETAFILE)hMeta, lpRect);
#else
     PlayMetaFile(hDC, hMeta);
#endif
     }
}
