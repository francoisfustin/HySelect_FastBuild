/*********************************************************
* TBDIB.C
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
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <math.h>
#include "toolbox.h"
#include "fphdc.h"
#include "wintools.h"

#define MAXREAD                  32767
#define BFT_BITMAP               0x4d42 /* 'BM' */

#define PALVERSION               0x300
#define MAXPALETTE               256   /* max. # supported palette entries */

#define WIDTHBYTES(i)            ((i+31)/32*4)
#define ISDIB(bft)               ((bft) == BFT_BITMAP)
#define BOUND(x, Size, Min, Max) ((x) < (Min) ? (Min) : ((x) > ((Max) - \
				 (Size)) ? ((Max) - (Size)) : (x)))

void  tbInitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr, DWORD dwWidth,
			   DWORD dwHeight, int nBPP);
LPBYTE tbFindDIBBits(LPBYTE lpbi);
int   tbPalEntriesOnDevice(HDC hDC);
#if defined(_WIN64) || defined(_IA64)
static HPVOID tbMemHugeCpy(HPVOID hpvMem1, HPVOID hpvMem2, LONG_PTR lSize);
#else
static HPVOID tbMemHugeCpy(HPVOID hpvMem1, HPVOID hpvMem2, long lSize);
#endif

HBITMAP tbBitmapFromDib(hDib, hpal, fPaletteIncluded)

HANDLE             hDib;
HPALETTE           hpal;
BOOL               fPaletteIncluded;
{
LPBITMAPINFOHEADER lpbi;
HPALETTE           hpalT;
HDC                hDC;
HBITMAP            hbm;
short              dOffset;

if (!hDib)
   return (NULL);

lpbi = (VOID FAR *)GlobalLock(hDib);

hDC = fpGetDC(NULL);

if (hpal)
   {
   hpalT = SelectPalette(hDC, hpal, TRUE);
   RealizePalette(hDC);     // GDI Bug...????
   }

if (fPaletteIncluded)
   dOffset = tbPaletteSize(lpbi);
else
   dOffset = 0;

hbm = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)lpbi, (LONG)CBM_INIT,
		     (LPBYTE)lpbi + lpbi->biSize + dOffset,
		     (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

if (hpal)
   SelectPalette(hDC, hpalT, TRUE);

ReleaseDC(NULL, hDC);
GlobalUnlock(hDib);

return (hbm);
}


HANDLE tbOpenDIB(szFile, BmpResourceData, BmpResourceDataSize)

LPCTSTR            szFile;
HPBYTE             BmpResourceData;
long               BmpResourceDataSize;
{
BITMAPINFOHEADER   bi;
LPBITMAPINFOHEADER lpbi;
DWORD              dwLen = 0;
DWORD              dwBits;
HANDLE             hDib;
HANDLE             h;
#if defined(WIN32)
HANDLE             fh = 0;
#else
HFILE              fh = 0;
OFSTRUCT           of;
#endif
long               BmpResourceDataAt = 0;

/* Open the file and read the DIB information */

if (!BmpResourceData)
{
   #if defined(WIN32)
   fh = CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                   NULL);
   if (INVALID_HANDLE_VALUE == fh)
   #else
   fh = OpenFile(szFile, &of, OF_READ);
   if (HFILE_ERROR == fh)
   #endif
      return (NULL);
}

if (!(hDib = tbReadDibBitmapInfo(fh, BmpResourceData, BmpResourceDataSize,
			       &BmpResourceDataAt)))
   {
   if (!BmpResourceData)
      #if defined(WIN32)
      CloseHandle(fh);
      #else
      _lclose(fh);
      #endif

   return (NULL);
   }

tbDibInfo(hDib, &bi);

/* Calculate the memory needed to hold the DIB */

dwBits = bi.biSizeImage;
dwLen  = bi.biSize + (DWORD)tbPaletteSize(&bi) + dwBits;

if (BmpResourceDataSize > 0 && dwLen > (DWORD)BmpResourceDataSize)
   {
   GlobalFree(hDib);
   hDib = NULL;
   }

/* Try to increase the size of the bitmap info. buffer to hold the DIB */

else if (!(h = GlobalReAlloc(hDib, dwLen, GHND)))
   {
   GlobalFree(hDib);
   hDib = NULL;
   }
else
   hDib = h;

/* Read in the bits */

if (hDib)
   {
   lpbi = (VOID FAR *)GlobalLock(hDib);
   tbDibLRead(fh, (LPBYTE)lpbi + (WORD)lpbi->biSize + tbPaletteSize(lpbi),
	          dwBits, BmpResourceData, BmpResourceDataSize,
	          &BmpResourceDataAt);
   GlobalUnlock(hDib);
   }

if (!BmpResourceData)
   #if defined(WIN32)
   CloseHandle(fh);
   #else
   _lclose(fh);
   #endif

return (hDib);
}


GLOBALHANDLE tbReadDibBitmapInfo(fh, BmpResourceData, BmpResourceDataSize,
			       BmpResourceDataAt)

#if defined(WIN32)
HANDLE             fh;
#else
int                fh;
#endif
HPBYTE             BmpResourceData;
long               BmpResourceDataSize;
long FAR          *BmpResourceDataAt;
{
DWORD              off;
HANDLE             hbi = NULL;
int                size;
int                i;
WORD               nNumColors;
RGBQUAD FAR       *pRgb;
BITMAPINFOHEADER   bi;
BITMAPCOREHEADER   bc;
LPBITMAPINFOHEADER lpbi;
BITMAPFILEHEADER   bf;
DWORD              dwWidth = 0;
DWORD              dwHeight = 0;
WORD               wPlanes, wBitCount;
#if defined(WIN32)
DWORD              BytesRead;
#endif

#if defined(WIN32)
if (INVALID_HANDLE_VALUE == fh)
#else
if (HFILE_ERROR == fh)
#endif
   return NULL;

/* Reset file pointer and read file header */

if (!BmpResourceData)
   {
   #if defined(WIN32)
   off = SetFilePointer(fh, 0, NULL, FILE_CURRENT);
   if (!ReadFile(fh, &bf, sizeof(bf), &BytesRead, NULL) ||
       sizeof(bf) != BytesRead)
   #else
   off = _llseek(fh, 0L, SEEK_CUR);
   if (sizeof(bf) != _lread (fh, (LPBYTE)&bf, sizeof(bf)))
   #endif
      return (0);
   }
else
   {
   off = *BmpResourceDataAt;

   if (off + sizeof(bf) > (DWORD)BmpResourceDataSize)
      return (0);

   _fmemcpy(&bf, &BmpResourceData[*BmpResourceDataAt], sizeof(bf));
   *BmpResourceDataAt += sizeof(bf);
   }

/* Do we have an RC HEADER? */

if (!ISDIB(bf.bfType))
   {
   bf.bfOffBits = 0L;

   if (!BmpResourceData)
      #if defined(WIN32)
      SetFilePointer(fh, off, NULL, FILE_BEGIN);
      #else
      _llseek (fh, off, SEEK_SET);
      #endif
   else
      *BmpResourceDataAt = off;
   }

if (!BmpResourceData)
   {
   #if defined(WIN32)
   if (!ReadFile(fh, &bi, sizeof(bi), &BytesRead, NULL) ||
       sizeof(bi) != BytesRead)
   #else
   if (sizeof(bi) != _lread(fh, (LPBYTE)&bi, sizeof(bi)))
   #endif
      return FALSE;
   }
else
   {
   if (*BmpResourceDataAt + (long)sizeof(bi) > BmpResourceDataSize)
      return (0);

   _fmemcpy(&bi, &BmpResourceData[*BmpResourceDataAt], sizeof(bi));
   *BmpResourceDataAt += sizeof(bi);
   }

nNumColors = tbDibNumColors(&bi);

/* Check the nature (BITMAPINFO or BITMAPCORE) of the info. block
 * and extract the field information accordingly. If a BITMAPCOREHEADER,
 * transfer it's field information to a BITMAPINFOHEADER-style block
 */

switch (size = (int)bi.biSize)
   {
   case sizeof(BITMAPINFOHEADER):
      break;

   case sizeof (BITMAPCOREHEADER):
      bc = *(BITMAPCOREHEADER FAR *)&bi;

      dwWidth   = (DWORD)bc.bcWidth;
      dwHeight  = (DWORD)bc.bcHeight;
      wPlanes   = bc.bcPlanes;
      wBitCount = bc.bcBitCount;

      bi.biSize          = sizeof(BITMAPINFOHEADER);
      bi.biWidth         = dwWidth;
      bi.biHeight        = dwHeight;
      bi.biPlanes        = wPlanes;
      bi.biBitCount      = wBitCount;

      bi.biCompression   = BI_RGB;
      bi.biSizeImage     = 0;
      bi.biXPelsPerMeter = 0;
      bi.biYPelsPerMeter = 0;
      bi.biClrUsed       = nNumColors;
      bi.biClrImportant  = nNumColors;

      if (!BmpResourceData)
         #if defined(WIN32)
         SetFilePointer(fh, sizeof(BITMAPCOREHEADER) - sizeof(BITMAPINFOHEADER),
                        NULL, FILE_CURRENT);
         #else
         _llseek(fh, (LONG)sizeof(BITMAPCOREHEADER) - sizeof(BITMAPINFOHEADER),
                 SEEK_CUR);
         #endif
      else
	 *BmpResourceDataAt += sizeof(BITMAPCOREHEADER) -
			       sizeof(BITMAPINFOHEADER);

      break;

   default:
      /* Not a DIB! */
      return (NULL);
  }

/*  Fill in some default values if they are zero */

if (bi.biSizeImage == 0)
   bi.biSizeImage = WIDTHBYTES ((DWORD)bi.biWidth * bi.biBitCount) *
		    bi.biHeight;

if (bi.biClrUsed == 0)
   bi.biClrUsed = tbDibNumColors(&bi);

/* Allocate for the BITMAPINFO structure and the color table. */

if (!(hbi = GlobalAlloc(GHND, (LONG)bi.biSize + nNumColors * sizeof(RGBQUAD))))
   return (NULL);

lpbi = (VOID FAR *)GlobalLock(hbi);
*lpbi = bi;

/* Get a pointer to the color table */

pRgb = (RGBQUAD FAR *)((LPBYTE)lpbi + bi.biSize);
if (nNumColors)
   {
   if (size == sizeof(BITMAPCOREHEADER))
      {
      /* Convert a old color table (3 byte RGBTRIPLEs) to a new
       * color table (4 byte RGBQUADs)
       */

      if (!BmpResourceData)
         #if defined(WIN32)
         ReadFile(fh, pRgb, nNumColors * sizeof(RGBTRIPLE), &BytesRead, NULL);
         #else
         _lread (fh, (LPBYTE)pRgb, nNumColors * sizeof(RGBTRIPLE));
         #endif
      else
	 {
	 if (*BmpResourceDataAt + ((long)nNumColors * (long)sizeof(RGBTRIPLE)) >
	     BmpResourceDataSize)
	    return (0);

	 tbMemHugeCpy(pRgb, &BmpResourceData[*BmpResourceDataAt],
		      nNumColors * sizeof(RGBTRIPLE));
	 *BmpResourceDataAt += nNumColors * sizeof(RGBTRIPLE);
	 }

      for (i = nNumColors - 1; i >= 0; i--)
	 {
	 RGBQUAD rgb;

	 rgb.rgbRed      = ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
	 rgb.rgbBlue     = ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
	 rgb.rgbGreen    = ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
	 rgb.rgbReserved = (BYTE)0;

	 pRgb[i] = rgb;
	 }
      }

   else
      {
      if (!BmpResourceData)
         #if defined(WIN32)
         ReadFile(fh, pRgb, nNumColors * sizeof(RGBQUAD), &BytesRead, NULL);
         #else
         _lread (fh, (LPBYTE)pRgb, nNumColors * sizeof(RGBQUAD));
         #endif
      else
	 {
	 if (*BmpResourceDataAt + (long)(nNumColors * sizeof(RGBQUAD)) >
	     BmpResourceDataSize)
	    return (0);

	 tbMemHugeCpy(pRgb, &BmpResourceData[*BmpResourceDataAt],
		      nNumColors * sizeof(RGBQUAD));
	 *BmpResourceDataAt += nNumColors * sizeof(RGBQUAD);
	 }
      }
   }

if (bf.bfOffBits != 0L)
   if (!BmpResourceData)
      #if defined(WIN32)
      SetFilePointer(fh, off + bf.bfOffBits, NULL, FILE_BEGIN);
      #else
      _llseek(fh, off + bf.bfOffBits, SEEK_SET);
      #endif
   else
      *BmpResourceDataAt = off + bf.bfOffBits;

GlobalUnlock(hbi);
return (hbi);
}


BOOL tbDibInfo(hbi, lpbi)

HANDLE             hbi;
LPBITMAPINFOHEADER lpbi;
{
if (hbi)
   {
   *lpbi = *(LPBITMAPINFOHEADER)GlobalLock(hbi);

   /* fill in the default fields */
   if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
      {
      if (lpbi->biSizeImage == 0L)
	 lpbi->biSizeImage =
	    WIDTHBYTES(lpbi->biWidth*lpbi->biBitCount) * lpbi->biHeight;

      if (lpbi->biClrUsed == 0L)
	 lpbi->biClrUsed = tbDibNumColors (lpbi);
      }

   GlobalUnlock (hbi);
   return (TRUE);
   }

return (FALSE);
}


DWORD PASCAL tbDibLRead(hFile, pBuffer, Len, BmpResourceData,
		      BmpResourceDataSize, BmpResourceDataAt)

#if defined(WIN32)
HANDLE     hFile;
#else
int        hFile;
#endif
VOID far  *pBuffer;
DWORD      Len;
HPBYTE     BmpResourceData;
long       BmpResourceDataSize;
long FAR  *BmpResourceDataAt;
{
DWORD      LenTotal = Len;
BYTE huge *hpBuffer = pBuffer;
#if defined(WIN32)
DWORD      BytesRead;
#endif

while (Len > (DWORD)MAXREAD)
   {
   if (!BmpResourceData)
      {
      #if defined(WIN32)
      if (!ReadFile(hFile, hpBuffer, MAXREAD, &BytesRead, NULL) ||
          MAXREAD != BytesRead)
      #else
      if (_lread(hFile, (LPBYTE)hpBuffer, (WORD)MAXREAD) != MAXREAD)
      #endif
         return (0);
      }
   else
      {
      tbMemHugeCpy(hpBuffer, &BmpResourceData[*BmpResourceDataAt],
                   (WORD)MAXREAD);
      *BmpResourceDataAt += MAXREAD;
      }

   Len -= MAXREAD;
   hpBuffer += MAXREAD;
   }

if (!BmpResourceData)
   {
   #if defined(WIN32)
   if (!ReadFile(hFile, hpBuffer, Len, &BytesRead, NULL) ||
       BytesRead != Len)
   #else
   if (_lread(hFile, (LPBYTE)hpBuffer, (WORD)Len) != (WORD)Len)
   #endif
      return (0);
   }
else
   {
   tbMemHugeCpy(hpBuffer, &BmpResourceData[*BmpResourceDataAt], (short)Len);
   *BmpResourceDataAt += Len;
   }

return (LenTotal);
}


HPALETTE tbCreateDibPalette(hbi)

HANDLE hbi;
{
    HPALETTE hpal;

    if (!hbi)
	return NULL;
    hpal = tbCreateBIPalette((LPBITMAPINFOHEADER)GlobalLock(hbi));
    GlobalUnlock(hbi);
    return hpal;
}


HPALETTE tbCreateBIPalette (lpbi)

LPBITMAPINFOHEADER lpbi;
{
    HLOCAL              hPalMem;
    LOGPALETTE          *pPal;
    HPALETTE            hpal = NULL;
    WORD                nNumColors;
    BYTE                red;
    BYTE                green;
    BYTE                blue;
    int                 i;
    RGBQUAD        FAR *pRgb;

    if (!lpbi)
	return NULL;

    if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
	return NULL;

    /* Get a pointer to the color table and the number of colors in it */
    pRgb = (RGBQUAD FAR *)((LPBYTE)lpbi + (WORD)lpbi->biSize);
    nNumColors = tbDibNumColors(lpbi);

    if (nNumColors){
	/* Allocate for the logical palette structure */
	hPalMem = LocalAlloc(LPTR,sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
	pPal = (LOGPALETTE*)LocalLock(hPalMem);
	if (!pPal)
	    return NULL;

	pPal->palNumEntries = nNumColors;
	pPal->palVersion    = PALVERSION;

	/* Fill in the palette entries from the DIB color table and
	 * create a logical color palette.
	 */
    for (i = 0; i < (short)nNumColors; i++){
	    pPal->palPalEntry[i].peRed   = pRgb[i].rgbRed;
	    pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
	    pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
	    pPal->palPalEntry[i].peFlags = (BYTE)0;
	}
	hpal = CreatePalette(pPal);
	LocalFree(hPalMem);
    }
    else if (lpbi->biBitCount == 24){
	/* A 24 bitcount DIB has no color table entries so, set the number of
	 * to the maximum value (256).
	 */
	nNumColors = MAXPALETTE;
	hPalMem = LocalAlloc(LPTR,sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
	pPal = (LOGPALETTE*)LocalLock(hPalMem);
	if (!pPal)
	    return NULL;

	pPal->palNumEntries = nNumColors;
	pPal->palVersion    = PALVERSION;

	red = green = blue = 0;

	/* Generate 256 (= 8*8*4) RGB combinations to fill the palette
	 * entries.
	 */
    for (i = 0; i < (short)pPal->palNumEntries; i++){
	    pPal->palPalEntry[i].peRed   = red;
	    pPal->palPalEntry[i].peGreen = green;
	    pPal->palPalEntry[i].peBlue  = blue;
	    pPal->palPalEntry[i].peFlags = (BYTE)0;

	    if (!(red += 32))
		if (!(green += 32))
		    blue += 64;
	}
	hpal = CreatePalette(pPal);
	LocalFree(hPalMem);
    }
    return hpal;
}


HANDLE tbDibFromBitmap (hbm, biStyle, biBits, hpal, lpdwLen, lpdPaletteSize)

HBITMAP              hbm;
DWORD                biStyle;
WORD                 biBits;
HPALETTE             hpal;
LPDWORD              lpdwLen;
LPSHORT              lpdPaletteSize;
{
BITMAP               bm;
BITMAPINFOHEADER     bi;
BITMAPINFOHEADER FAR *lpbi;
DWORD                dwLen;
HANDLE               hdib;
HANDLE               h;
HDC                  hdc;
short                dPaletteSize;

if (!hbm)
    return NULL;

if (hpal == NULL)
    hpal = GetStockObject(DEFAULT_PALETTE);

GetObject(hbm,sizeof(bm),(LPBYTE)&bm);

if (biBits == 0)
    biBits =  bm.bmPlanes * bm.bmBitsPixel;

bi.biSize               = sizeof(BITMAPINFOHEADER);
bi.biWidth              = bm.bmWidth;
bi.biHeight             = bm.bmHeight;
bi.biPlanes             = 1;
bi.biBitCount           = biBits;
bi.biCompression        = biStyle;
bi.biSizeImage          = 0;
bi.biXPelsPerMeter      = 0;
bi.biYPelsPerMeter      = 0;
bi.biClrUsed            = 0;
bi.biClrImportant       = 0;

dPaletteSize  = tbPaletteSize(&bi);
dwLen  = bi.biSize + dPaletteSize;

hdc = fpGetDC(NULL);
hpal = SelectPalette(hdc,hpal,TRUE);
     RealizePalette(hdc);

hdib = GlobalAlloc(GHND,dwLen);

if (!hdib){
    SelectPalette(hdc,hpal,TRUE);
    ReleaseDC(NULL,hdc);
    return NULL;
}

lpbi = (VOID FAR *)GlobalLock(hdib);

*lpbi = bi;

/*  call GetDIBits with a NULL lpBits param, so it will calculate the
 *  biSizeImage field for us
 */
GetDIBits(hdc, hbm, 0, (WORD)bi.biHeight,
    NULL, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

bi = *lpbi;
GlobalUnlock(hdib);

/* If the driver did not fill in the biSizeImage field, make one up */
if (bi.biSizeImage == 0){
    bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

    if (biStyle != BI_RGB)
	bi.biSizeImage = (bi.biSizeImage * 3) / 2;
}

/*  realloc the buffer big enough to hold all the bits */
dwLen = bi.biSize + dPaletteSize + bi.biSizeImage;
if (h = GlobalReAlloc(hdib,dwLen,0))
    hdib = h;
else{
    GlobalFree(hdib);
    hdib = NULL;

    SelectPalette(hdc,hpal,TRUE);
    ReleaseDC(NULL,hdc);
    return hdib;
}

/*  call GetDIBits with a NON-NULL lpBits param, and actualy get the
 *  bits this time
 */
lpbi = (VOID FAR *)GlobalLock(hdib);

if (GetDIBits( hdc,
	       hbm,
	       0,
	       (WORD)bi.biHeight,
	       (LPBYTE)lpbi + (WORD)lpbi->biSize + dPaletteSize,
	       (LPBITMAPINFO)lpbi, DIB_RGB_COLORS) == 0){
     GlobalUnlock(hdib);
     hdib = NULL;
     SelectPalette(hdc,hpal,TRUE);
     ReleaseDC(NULL,hdc);
     return NULL;
}

bi = *lpbi;
GlobalUnlock(hdib);

*lpdwLen = dwLen;

SelectPalette(hdc,hpal,TRUE);
ReleaseDC(NULL,hdc);

if (lpdPaletteSize)
   *lpdPaletteSize = dPaletteSize;

return (hdib);
}


HANDLE tbBitmapToDIB(HBITMAP hBitmap, HPALETTE hPal)
{
BITMAP             Bitmap;
BITMAPINFOHEADER   bmInfoHdr;
LPBITMAPINFOHEADER lpbmInfoHdr;
LPBYTE             lpBits;
HDC                hMemDC;
HANDLE             hDIB;
HPALETTE           hOldPal = NULL;

if (!hBitmap)
   return NULL;

if (!GetObject (hBitmap, sizeof (Bitmap), (LPBYTE) &Bitmap))
   return NULL;

tbInitBitmapInfoHeader (&bmInfoHdr,
		      Bitmap.bmWidth,
		      Bitmap.bmHeight,
		      Bitmap.bmPlanes * Bitmap.bmBitsPixel);


   // Now allocate memory for the DIB.  Then, set the BITMAPINFOHEADER
   //  into this memory, and find out where the bitmap bits go.

hDIB = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) +
		           tbPaletteSize(&bmInfoHdr) + bmInfoHdr.biSizeImage);

if (!hDIB)
   return NULL;

lpbmInfoHdr  = (LPBITMAPINFOHEADER) GlobalLock (hDIB);
*lpbmInfoHdr = bmInfoHdr;
lpBits       = tbFindDIBBits((LPBYTE)lpbmInfoHdr);


   // Now, we need a DC to hold our bitmap.  If the app passed us
   //  a palette, it should be selected into the DC.

hMemDC = fpGetDC (NULL);

if (hPal)
   {
   hOldPal = SelectPalette (hMemDC, hPal, TRUE);
   RealizePalette (hMemDC);
   }

if (!GetDIBits(hMemDC, hBitmap, 0, Bitmap.bmHeight, lpBits,
	       (LPBITMAPINFO)lpbmInfoHdr, DIB_RGB_COLORS))
   {
   GlobalUnlock (hDIB);
   GlobalFree (hDIB);
   hDIB = NULL;
   }
else
   GlobalUnlock (hDIB);

if (hOldPal)
   SelectPalette (hMemDC, hOldPal, TRUE);

ReleaseDC (NULL, hMemDC);

return (hDIB);
}


void tbInitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr, DWORD dwWidth,
			  DWORD dwHeight, int nBPP)
{
_fmemset (lpBmInfoHdr, 0, sizeof (BITMAPINFOHEADER));

lpBmInfoHdr->biSize      = sizeof (BITMAPINFOHEADER);
lpBmInfoHdr->biWidth     = dwWidth;
lpBmInfoHdr->biHeight    = dwHeight;
lpBmInfoHdr->biPlanes    = 1;

if (nBPP <= 1)
   nBPP = 1;
else if (nBPP <= 4)
   nBPP = 4;
else if (nBPP <= 8)
   nBPP = 8;
else
   nBPP = 24;

lpBmInfoHdr->biBitCount  = nBPP;
lpBmInfoHdr->biSizeImage = WIDTHBYTES (dwWidth * nBPP) * dwHeight;
}


LPBYTE tbFindDIBBits(LPBYTE lpbi)
{
return (lpbi + *(LPDWORD)lpbi + tbPaletteSize((LPBITMAPINFOHEADER)lpbi));
}


HPALETTE tbGetSystemPalette(void)
{
HDC           hDC;
HPALETTE      hPal = NULL;
HANDLE        hLogPal;
LPLOGPALETTE  lpLogPal;
int           i, nColors;

// Find out how many palette entries we want.

hDC = fpGetDC(NULL);
if (!hDC)
   return (NULL);

nColors = tbPalEntriesOnDevice(hDC);
ReleaseDC(NULL, hDC);

   // Allocate room for the palette and lock it.

hLogPal = GlobalAlloc(GHND, sizeof (LOGPALETTE) +
		      nColors * sizeof (PALETTEENTRY));

if (!hLogPal)
   return (NULL);

lpLogPal = (LPLOGPALETTE)GlobalLock(hLogPal);

lpLogPal->palVersion    = PALVERSION;
lpLogPal->palNumEntries = nColors;

for (i = 0; i < nColors; i++)
   {
   lpLogPal->palPalEntry[i].peBlue  = 0;
   *((LPWORD) (&lpLogPal->palPalEntry[i].peRed)) = i;
   lpLogPal->palPalEntry[i].peFlags = PC_EXPLICIT;
   }

// Go ahead and create the palette.  Once it's created,
//  we no longer need the LOGPALETTE, so free it.

hPal = CreatePalette(lpLogPal);

GlobalUnlock(hLogPal);
GlobalFree(hLogPal);

return (hPal);
}


int tbPalEntriesOnDevice(HDC hDC)
{
int nColors;

// Find out the number of palette entries on this
//  defice.

nColors = GetDeviceCaps (hDC, SIZEPALETTE);


// For non-palette devices, we'll use the # of system
// colors for our palette size.

if (!nColors)
   nColors = GetDeviceCaps (hDC, NUMCOLORS);

return (nColors);
}


GLOBALHANDLE tbSaveDIBToBuffer(GLOBALHANDLE hdib, LPLONG lplBufferSize)
{
BITMAPFILEHEADER   hdr;
LPBITMAPINFOHEADER lpbi;
GLOBALHANDLE       hBuffer = 0;
HPBYTE             hpBuffer;
HPBYTE             hpbi;

if (hdib)
   {
   hpbi = (HPBYTE)GlobalLock(hdib);

   lpbi = (LPBITMAPINFOHEADER)hpbi;

   /* Fill in the fields of the file header */
   hdr.bfType          = BFT_BITMAP;
   hdr.bfSize          = (DWORD)(GlobalSize(hdib) + sizeof(BITMAPFILEHEADER));
   hdr.bfReserved1     = 0;
   hdr.bfReserved2     = 0;
   hdr.bfOffBits       = (DWORD)sizeof(BITMAPFILEHEADER) + lpbi->biSize +
			 tbPaletteSize(lpbi);

   if (hBuffer = GlobalAlloc(GHND, (LONG)hdr.bfSize))
      {
      hpBuffer = (HPBYTE)GlobalLock(hBuffer);

      /* Write the file header */
      tbMemHugeCpy(hpBuffer, (LPBYTE)&hdr, sizeof(BITMAPFILEHEADER));
      hpBuffer += sizeof(BITMAPFILEHEADER);

      /* Write the DIB header and the bits */
      tbMemHugeCpy(hpBuffer, hpbi, GlobalSize(hdib));

      GlobalUnlock(hBuffer);

      if (lplBufferSize)
	 *lplBufferSize = (LONG)hdr.bfSize;
      }

   GlobalUnlock(hdib);
   }

return (hBuffer);
}


HBITMAP tbIconToBitmap(HWND hWnd, HICON hIcon, COLORREF Color)
{
HBITMAP    hBitmap;
HBITMAP    hBitmapOld;
HDC        hDC;
HDC        hDCMemory;
RECT       Rect;
HBRUSH     hBrush;
int        Width;
int        Height;

Width = tbGetIconWidth(hIcon);
Height = tbGetIconHeight(hIcon);

hDC = fpGetDC(hWnd);

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

ReleaseDC(hWnd, hDC);

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

/***********
* Clean up
***********/

SelectObject(hDCMemory, hBitmapOld);
DeleteDC(hDCMemory);

return (hBitmap);
}

#if defined(_WIN64) || defined(_IA64)
static HPVOID tbMemHugeCpy(HPVOID hpvMem1, HPVOID hpvMem2, LONG_PTR lSize)
#else
static HPVOID tbMemHugeCpy(HPVOID hpvMem1, HPVOID hpvMem2, long lSize)
#endif
{
HPBYTE hpMem1 = hpvMem1;
HPBYTE hpMem2 = hpvMem2;
HPBYTE p = hpvMem1;

while(lSize--)
   *hpMem1++ = *hpMem2++;

return (p);
}
