/*    PortTool v2.2     FPDIB.C          */

#include "fptools.h"
#include "fphuge.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <math.h>


#define MAXREAD                  32767
#define BFT_BITMAP               0x4d42 /* 'BM' */

#define PALVERSION               0x300
#define MAXPALETTE               256   /* max. # supported palette entries */

#define WIDTHBYTES(i)            ((i+31)/32*4)
#define ISDIB(bft)               ((bft) == BFT_BITMAP)
#define BOUND(x, Size, Min, Max) ((x) < (Min) ? (Min) : ((x) > ((Max) - \
                                 (Size)) ? ((Max) - (Size)) : (x)))

void   InitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr, DWORD dwWidth,
                            DWORD dwHeight, int nBPP);
LPSTR  FindDIBBits(LPSTR lpbi);
int    PalEntriesOnDevice(HDC hDC);

long fpGetIconHeight(HICON hIcon);
long fpGetIconWidth(HICON hIcon);

HBITMAP FPLIB BitmapFromDib(HANDLE hDib, HPALETTE hpal, BOOL fPaletteIncluded)
{
LPBITMAPINFOHEADER lpbi;
HPALETTE           hpalT;
HDC                hDC;
HBITMAP            hbm;
short              dOffset;

if (!hDib)
   return (NULL);

lpbi = (VOID FAR *)GlobalLock(hDib);

hDC = GetDC(NULL);

if (hpal)
   {
   hpalT = SelectPalette(hDC, hpal, TRUE);
   RealizePalette(hDC);     // GDI Bug...????
   }

if (fPaletteIncluded)
   dOffset = PaletteSize(lpbi);
else
   dOffset = 0;

hbm = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)lpbi, (LONG)CBM_INIT,
                     (LPSTR)lpbi + lpbi->biSize + dOffset,
                     (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

if (hpal)
   SelectPalette(hDC, hpalT, TRUE);

ReleaseDC(NULL, hDC);
GlobalUnlock(hDib);

return (hbm);
}


HANDLE FPLIB OpenDIB(LPSTR szFile, HPSTR BmpResourceData, long BmpResourceDataSize)
{
unsigned           fh = 0;
BITMAPINFOHEADER   bi;
LPBITMAPINFOHEADER lpbi;
DWORD              dwLen = 0;
DWORD              dwBits;
HANDLE             hDib;
HANDLE             h;
OFSTRUCT           of;
long               BmpResourceDataAt = 0;

/* Open the file and read the DIB information */

if (!BmpResourceData)

   if ((fh = OpenFile(szFile, &of, OF_READ)) == -1)
      return (NULL);

if (!(hDib = ReadDibBitmapInfo(fh, BmpResourceData, BmpResourceDataSize,
                               &BmpResourceDataAt)))
   {
   if (!BmpResourceData)
      _lclose(fh);

   return (NULL);
   }

DibInfo(hDib, &bi);

/* Calculate the memory needed to hold the DIB */

dwBits = bi.biSizeImage;
dwLen  = bi.biSize + (DWORD)PaletteSize(&bi) + dwBits;

/* Try to increase the size of the bitmap info. buffer to hold the DIB */

if (!(h = GlobalReAlloc(hDib, dwLen, GHND)))
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

   DibLRead(fh, (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi),
            dwBits, BmpResourceData, BmpResourceDataSize,
            &BmpResourceDataAt);
   GlobalUnlock(hDib);
   }

if (!BmpResourceData)
   _lclose(fh);

return (hDib);
}


GLOBALHANDLE FPLIB ReadDibBitmapInfo(int fh, HPSTR BmpResourceData, long BmpResourceDataSize, 
                               long FAR *BmpResourceDataAt)
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

if (fh == -1)
   return NULL;

/* Reset file pointer and read file header */

if (!BmpResourceData)
   {
   off = _llseek(fh, 0L, SEEK_CUR);
   if (sizeof(bf) != _lread (fh, (LPSTR)&bf, sizeof(bf)))
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
      _llseek (fh, off, SEEK_SET);
   else
      *BmpResourceDataAt = off;
   }

if (!BmpResourceData)
   {
   if (sizeof(bi) != _lread(fh, (LPSTR)&bi, sizeof(bi)))
      return FALSE;
   }
else
   {
   if (*BmpResourceDataAt + (int)sizeof(bi) > BmpResourceDataSize)
      return (0);

   _fmemcpy(&bi, &BmpResourceData[*BmpResourceDataAt], sizeof(bi));
   *BmpResourceDataAt += sizeof(bi);
   }

nNumColors = DibNumColors(&bi);

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
         _llseek(fh, (LONG)sizeof(BITMAPCOREHEADER) - sizeof(BITMAPINFOHEADER),
                 SEEK_CUR);
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
   bi.biClrUsed = DibNumColors(&bi);

/* Allocate for the BITMAPINFO structure and the color table. */

if (!(hbi = GlobalAlloc(GHND, (LONG)bi.biSize + nNumColors * sizeof(RGBQUAD))))
   return (NULL);

lpbi = (VOID FAR *)GlobalLock(hbi);
*lpbi = bi;

/* Get a pointer to the color table */

pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + bi.biSize);
if (nNumColors)
   {
   if (size == sizeof(BITMAPCOREHEADER))
      {
      /* Convert a old color table (3 byte RGBTRIPLEs) to a new
       * color table (4 byte RGBQUADs)
       */

      if (!BmpResourceData)
         _lread (fh, (LPSTR)pRgb, nNumColors * sizeof(RGBTRIPLE));
      else
         {
         if (*BmpResourceDataAt + (nNumColors * (int)sizeof(RGBTRIPLE)) >
             BmpResourceDataSize)
            return (0);

         fpMemHugeCpy(pRgb, &BmpResourceData[*BmpResourceDataAt],
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
         _lread (fh, (LPSTR)pRgb, nNumColors * sizeof(RGBQUAD));
      else
         {
         if (*BmpResourceDataAt + (nNumColors * (int)sizeof(RGBQUAD)) >
             BmpResourceDataSize)
            return (0);

         fpMemHugeCpy(pRgb, &BmpResourceData[*BmpResourceDataAt],
                      nNumColors * sizeof(RGBQUAD));
         *BmpResourceDataAt += nNumColors * sizeof(RGBQUAD);
         }
      }
   }

if (bf.bfOffBits != 0L)
   if (!BmpResourceData)
      _llseek(fh, off + bf.bfOffBits, SEEK_SET);
   else
      *BmpResourceDataAt = off + bf.bfOffBits;

GlobalUnlock(hbi);
return (hbi);
}


BOOL FPLIB DibInfo(HANDLE hbi, LPBITMAPINFOHEADER lpbi)
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
         lpbi->biClrUsed = DibNumColors (lpbi);
      }

   GlobalUnlock (hbi);
   return (TRUE);
   }

return (FALSE);
}


DWORD PASCAL DibLRead(hFile, pBuffer, Len, BmpResourceData,
                      BmpResourceDataSize, BmpResourceDataAt)

int        hFile;
VOID FAR  *pBuffer;
DWORD      Len;
HPSTR      BmpResourceData;
long       BmpResourceDataSize;
long FAR  *BmpResourceDataAt;
{
DWORD      LenTotal = Len;
BYTE huge *hpBuffer = pBuffer;


while (Len > (DWORD)MAXREAD)
   {
   if (!BmpResourceData)
      {

      if (_lread(hFile, (LPSTR)hpBuffer, (UINT)MAXREAD) != (UINT)MAXREAD)
         return (0);
      }
   else
      {
      fpMemHugeCpy(hpBuffer, &BmpResourceData[*BmpResourceDataAt],
                   (WORD)MAXREAD);

      *BmpResourceDataAt += MAXREAD;
      }


   Len -= MAXREAD;

   hpBuffer += MAXREAD;
   }

if (!BmpResourceData)
   {

   if (_lread(hFile, (LPSTR)hpBuffer, (UINT)Len) != (UINT)Len)
      return (0);
   }
else
   {
   fpMemHugeCpy(hpBuffer, &BmpResourceData[*BmpResourceDataAt], (short)Len);
   *BmpResourceDataAt += Len;
   }

return (LenTotal);
}


HPALETTE FPLIB CreateDibPalette(HANDLE hbi)
{
    HPALETTE hpal;

    if (!hbi)
   return NULL;
    hpal = CreateBIPalette((LPBITMAPINFOHEADER)GlobalLock(hbi));
    GlobalUnlock(hbi);
    return hpal;
}


HPALETTE FPLIB CreateBIPalette (LPBITMAPINFOHEADER lpbi)
{
    HGLOBAL             hglbPal;
    LPLOGPALETTE        pPal;
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

    pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)lpbi->biSize);
    nNumColors = DibNumColors(lpbi);

    if (nNumColors)
       {
       /* Allocate for the logical palette structure */
       hglbPal = GlobalAlloc(GPTR, sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
       
       if (!hglbPal)
          return NULL;

       pPal = (LPLOGPALETTE)GlobalLock(hglbPal);

       if (!pPal)
          return NULL;
 
       pPal->palNumEntries = nNumColors;
       pPal->palVersion    = PALVERSION;

       /* Fill in the palette entries from the DIB color table and
        * create a logical color palette.
        */
       for (i = 0; i < (short)nNumColors; i++)
           {
           pPal->palPalEntry[i].peRed   = pRgb[i].rgbRed;
           pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
           pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
           pPal->palPalEntry[i].peFlags = (BYTE)0;
           }
       hpal = CreatePalette(pPal);
       GlobalUnlock(hglbPal);
       GlobalFree(hglbPal);
       }
    else if (lpbi->biBitCount == 24)
       {
       /* A 24 bitcount DIB has no color table entries so, set the number of
        * to the maximum value (256).
        */
       nNumColors = MAXPALETTE;
       hglbPal = GlobalAlloc(GPTR, sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
       
       if (!hglbPal)
          return NULL;

       pPal = (LPLOGPALETTE)GlobalLock(hglbPal);

       if (!pPal)
          return NULL;
 

       pPal->palNumEntries = nNumColors;
       pPal->palVersion    = PALVERSION;

       red = green = blue = 0;

       /* Generate 256 (= 8*8*4) RGB combinations to fill the palette
        * entries.
        */
       for (i = 0; i < (short)pPal->palNumEntries; i++)
           {
           pPal->palPalEntry[i].peRed   = red;
           pPal->palPalEntry[i].peGreen = green;
           pPal->palPalEntry[i].peBlue  = blue;
           pPal->palPalEntry[i].peFlags = (BYTE)0;
           
           if (!(red += 32))
              if (!(green += 32))
                 blue += 64;
           }
       hpal = CreatePalette(pPal);
       GlobalUnlock(hglbPal);
       GlobalFree(hglbPal);
    }
    return hpal;
}


HANDLE FPLIB DibFromBitmap (HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal, 
                      LPDWORD lpdwLen, LPSHORT lpdPaletteSize)
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

GetObject(hbm,sizeof(bm),(LPSTR)&bm);

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

dPaletteSize  = PaletteSize(&bi);
dwLen  = bi.biSize + dPaletteSize;

hdc = GetDC(NULL);
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

GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight,
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
               (UINT)bi.biHeight,
               (LPSTR)lpbi + (WORD)lpbi->biSize + dPaletteSize,
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


HANDLE FPLIB BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal)
{
BITMAP             Bitmap;
BITMAPINFOHEADER   bmInfoHdr;
LPBITMAPINFOHEADER lpbmInfoHdr;
LPSTR              lpBits;
HDC                hMemDC;
HANDLE             hDIB;
HPALETTE           hOldPal = NULL;

if (!hBitmap)
   return NULL;

if (!GetObject (hBitmap, sizeof (Bitmap), (LPSTR) &Bitmap))
   return NULL;

InitBitmapInfoHeader (&bmInfoHdr,
                      Bitmap.bmWidth,
                      Bitmap.bmHeight,
                      Bitmap.bmPlanes * Bitmap.bmBitsPixel);


   // Now allocate memory for the DIB.  Then, set the BITMAPINFOHEADER
   //  into this memory, and find out where the bitmap bits go.

hDIB = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) +
                   PaletteSize(&bmInfoHdr) + bmInfoHdr.biSizeImage);

if (!hDIB)
   return NULL;

lpbmInfoHdr  = (LPBITMAPINFOHEADER) GlobalLock (hDIB);
*lpbmInfoHdr = bmInfoHdr;
lpBits       = FindDIBBits((LPSTR)lpbmInfoHdr);


   // Now, we need a DC to hold our bitmap.  If the app passed us
   //  a palette, it should be selected into the DC.

hMemDC = GetDC (NULL);

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


void InitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr, DWORD dwWidth,
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


LPSTR FindDIBBits(LPSTR lpbi)
{
return (lpbi + *(LPDWORD)lpbi + PaletteSize((LPBITMAPINFOHEADER)lpbi));
}


HPALETTE FPLIB GetSystemPalette(void)
{
HDC           hDC;
HPALETTE      hPal = NULL;
HANDLE        hLogPal;
LPLOGPALETTE  lpLogPal;
int           i, nColors;

// Find out how many palette entries we want.

hDC = GetDC(NULL);
if (!hDC)
   return (NULL);

nColors = PalEntriesOnDevice(hDC);
ReleaseDC(NULL, hDC);

if (nColors <= 0)
  return (NULL);
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


int PalEntriesOnDevice(HDC hDC)
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


GLOBALHANDLE FPLIB SaveDIBToBuffer(GLOBALHANDLE hdib, LPLONG lplBufferSize)
{
BITMAPFILEHEADER   hdr;
LPBITMAPINFOHEADER lpbi;
GLOBALHANDLE       hBuffer = 0;
HPSTR              hpBuffer;
HPSTR              hpbi;

if (hdib)
   {
   hpbi = (HPSTR)GlobalLock(hdib);

   lpbi = (LPBITMAPINFOHEADER)hpbi;

   /* Fill in the fields of the file header */
   hdr.bfType          = BFT_BITMAP;
   hdr.bfSize          = (DWORD)GlobalSize(hdib) + sizeof(BITMAPFILEHEADER);
   hdr.bfReserved1     = 0;
   hdr.bfReserved2     = 0;
   hdr.bfOffBits       = (DWORD)sizeof(BITMAPFILEHEADER) + lpbi->biSize +
                         PaletteSize(lpbi);

   if (hBuffer = GlobalAlloc(GHND, (LONG)hdr.bfSize))
      {
      hpBuffer = (HPSTR)GlobalLock(hBuffer);

      /* Write the file header */
      fpMemHugeCpy(hpBuffer, (LPSTR)&hdr, sizeof(BITMAPFILEHEADER));
      hpBuffer += sizeof(BITMAPFILEHEADER);

      /* Write the DIB header and the bits */
      fpMemHugeCpy(hpBuffer, hpbi, GlobalSize(hdib));

      GlobalUnlock(hBuffer);

      if (lplBufferSize)
         *lplBufferSize = (LONG)hdr.bfSize;
      }

   GlobalUnlock(hdib);
   }

return (hBuffer);
}


HBITMAP FPLIB fpIconToBitmap(HWND hWnd, HICON hIcon, COLORREF Color)
{
HBITMAP    hBitmap;
HBITMAP    hBitmapOld;
HDC        hDC;
HDC        hDCMemory;
RECT       Rect;
HBRUSH     hBrush;
short      Width;
short      Height;

Width = (short)fpGetIconWidth(hIcon);
Height = (short)fpGetIconHeight(hIcon);

hDC = GetDC(hWnd);

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
DrawIconEx(hDCMemory, 0, 0,
           hIcon, Width, Height,
           0,
           NULL, DI_NORMAL);
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

HBITMAP FPLIB fpMetaFileToBitmap(HWND hWnd, HANDLE hMeta, COLORREF Color)
{
HBITMAP    hBitmap;
HBITMAP    hBitmapOld;
HDC        hDC;
HDC        hDCMemory;
RECT       Rect;
HBRUSH     hBrush;
short      Width;
short      Height;

hDC = GetDC(hWnd);

GetClientRect(hWnd, &Rect);

Width = (short)(Rect.right - Rect.left);
Height = (short)(Rect.bottom - Rect.top);

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

fpRenderMetaFile(hDCMemory, hMeta, &Rect, 0, 0, Color);

/***********
* Clean up
***********/

SelectObject(hDCMemory, hBitmapOld);
DeleteDC(hDCMemory);

return (hBitmap);
}
