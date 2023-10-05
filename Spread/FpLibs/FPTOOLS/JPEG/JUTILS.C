/*
 * jutils.c
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains tables and miscellaneous utility routines needed
 * for both compression and decompression.
 * Note we prefix all global names with "j" to minimize conflicts with
 * a surrounding application.
 */

 // jb - 30.jun.00
 //
 // Removed the reliance on fptools.h, moved the BITMAP2JPEG structure
 // to jpeglib.h and replaced FPLIB with _stdcall
 //

#define JPEG_INTERNALS
#include <tchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wchar.h>

#include "jinclude.h"
#include "jpeglib.h"
#define FPCOLOR(c)         ((c &  0x80000000) ? GetSysColor((int)(short)c) : c);

/*
 * jpeg_natural_order[i] is the natural-order position of the i'th element
 * of zigzag order.
 *
 * When reading corrupted data, the Huffman decoders could attempt
 * to reference an entry beyond the end of this array (if the decoded
 * zero run length reaches past the end of the block).  To prevent
 * wild stores without adding an inner-loop test, we put some extra
 * "63"s after the real entries.  This will cause the extra coefficient
 * to be stored in location 63 of the block, not somewhere random.
 * The worst case would be a run-length of 15, which means we need 16
 * fake entries.
 */

const int jpeg_natural_order[DCTSIZE2+16] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
 63, 63, 63, 63, 63, 63, 63, 63, /* extra entries for safety in decoder */
 63, 63, 63, 63, 63, 63, 63, 63
};


/*
 * Arithmetic utilities
 */

long jdiv_round_up (long a, long b)
/* Compute a/b rounded up to next integer, ie, ceil(a/b) */
/* Assumes a >= 0, b > 0 */
{
  return (a + b - 1L) / b;
}


long jround_up (long a, long b)
/* Compute a rounded up to next multiple of b, ie, ceil(a/b)*b */
/* Assumes a >= 0, b > 0 */
{
  a += b - 1L;
  return a - (a % b);
}


/* On normal machines we can apply MEMCOPY() and MEMZERO() to sample arrays
 * and coefficient-block arrays.  This won't work on 80x86 because the arrays
 * are FAR and we're assuming a small-pointer memory model.  However, some
 * DOS compilers provide far-pointer versions of memcpy() and memset() even
 * in the small-model libraries.  These will be used if USE_FMEM is defined.
 * Otherwise, the routines below do it the hard way.  (The performance cost
 * is not all that great, because these routines aren't very heavily used.)
 */

#define FMEMCOPY(dest,src,size)	MEMCOPY(dest,src,size)
#define FMEMZERO(target,size)	MEMZERO(target,size)

void jcopy_sample_rows (JSAMPARRAY input_array, int source_row,
		   JSAMPARRAY output_array, int dest_row,
		   int num_rows, JDIMENSION num_cols)
/* Copy some rows of samples from one place to another.
 * num_rows rows are copied from input_array[source_row++]
 * to output_array[dest_row++]; these areas may overlap for duplication.
 * The source and destination arrays must be at least as wide as num_cols.
 */
{
  register JSAMPROW inptr, outptr;
#ifdef FMEMCOPY
  register size_t count = (size_t) (num_cols * SIZEOF(JSAMPLE));
#else
  register JDIMENSION count;
#endif
  register int row;

  input_array += source_row;
  output_array += dest_row;

  for (row = num_rows; row > 0; row--) {
    inptr = *input_array++;
    outptr = *output_array++;
#ifdef FMEMCOPY
    FMEMCOPY(outptr, inptr, count);
#else
    for (count = num_cols; count > 0; count--)
      *outptr++ = *inptr++;	/* needn't bother with GETJSAMPLE() here */
#endif
  }
}


void jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row,
		 JDIMENSION num_blocks)
/* Copy a row of coefficient blocks from one place to another. */
{
#ifdef FMEMCOPY
  FMEMCOPY(output_row, input_row, num_blocks * (DCTSIZE2 * SIZEOF(JCOEF)));
#else
  register JCOEFPTR inptr, outptr;
  register long count;

  inptr = (JCOEFPTR) input_row;
  outptr = (JCOEFPTR) output_row;
  for (count = (long) num_blocks * DCTSIZE2; count > 0; count--) {
    *outptr++ = *inptr++;
  }
#endif
}


void jzero_far (void * target, size_t bytestozero)
/* Zero out a chunk of FAR memory. */
/* This might be sample-array data, block-array data, or alloc_large data. */
{
#ifdef FMEMZERO
  FMEMZERO(target, bytestozero);
#else
  register TCHAR *ptr = (TCHAR *) target;
  register size_t count;

  for (count = bytestozero; count > 0; count--) {
    *ptr++ = 0;
  }
#endif
}

// ----------------------------------------------------------------------------
// Pretty self explanatory.  Got this from FPtools to return number of
// colors for the bitmap.
//
WORD DibNumColors (LPVOID pv)
{
	int bits;
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
	{	if (lpbi->biClrUsed != 0)
		{	return ((WORD)lpbi->biClrUsed);
		}

		bits = lpbi->biBitCount;
	}
	else
	{	bits = lpbc->bcBitCount;
	}

	switch (bits)
	{	case 1:
			return 2;
		case 4:
			return 16;
		case 8:
			return 256;
		default:

      /* A 24 bitcount DIB has no color table */
		return 0;
	}
}

// ----------------------------------------------------------------------------
// Get the palette size.
//
WORD PaletteSize (LPVOID pv)
{
	LPBITMAPINFOHEADER lpbi;
	WORD               NumColors;

	lpbi      = (LPBITMAPINFOHEADER)pv;
	NumColors = DibNumColors(lpbi);

	if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
	{	return (NumColors * sizeof(RGBTRIPLE));
	}
	else
	{	return (NumColors * sizeof(RGBQUAD));
	}
}

// ==========================================================================
// jb - 27.jan.99 --------------------------------------------------------------------------------
//
// The colorref is blue, green, red so reverse the blue and red for RGB
//
void reverseRGB (JSAMPROW *row_pointer, JDIMENSION width)
{
	JDIMENSION	i;
	BYTE		temptr;

	for (i=0; i <= width * 3; i += 3)
	{	temptr = row_pointer[0][i];
		row_pointer[0][i] = row_pointer[0][i+2];
		row_pointer[0][i+2] = temptr;
	}
}

// Main jpeg processing.
//
BOOL WriteJPEG(TCHAR *jpegfile, LPBITMAP2JPEG pBitmap2JPEG)
{
	BOOL bRet = TRUE;

	// This struct contains the JPEG compression parameters and pointers to
	// working space (which is allocated as needed by the JPEG library).
	// It is possible to have several such structures, representing multiple
	// compression/decompression processes, in existence at once.  We refer
	// to any one struct (and its associated working data) as a "JPEG object".
	// 
	struct jpeg_compress_struct cinfo;

	// pointer to JSAMPLE row[s]
	//
	JSAMPROW row_pointer[1];
	FILE *outfile;

	// Physical row width in image buffer.  
	// The reverse_scanlines is used to coerce the writing of scanlines from the 
	// bottom up for our bitmap.
	//
	int row_stride;
	int reverse_scanlines;

	__try 
	{	// Now we can initialize the JPEG compression object.
		//
		jpeg_create_compress (&cinfo);

		// Use the library-supplied code to send compressed data to a
		// stdio stream.  You can also write your own code to do something else.
		//
		if ((outfile = _tfopen (jpegfile, _T("wb"))) == NULL) 
		{	RaiseException (0, 1, 0, NULL);
		}

		jpeg_stdio_dest (&cinfo, outfile);

		// Set parameters for compression
		// First we supply a description of the input image.
		// Four fields of the cinfo struct must be filled in:
		// 
		cinfo.image_width  = pBitmap2JPEG->image_width;
		cinfo.image_height = pBitmap2JPEG->image_height;

		// Number of color components per pixel and the type of colorspace of input image.
		//
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;

		// Now use the library's routine to set default compression parameters.
		// (You must set at least cinfo.in_color_space before calling this,
		// since the defaults depend on the source color space.)  Also, set optimize on
		// for a smaller jpeg file.  (I've found no noticible loss in quality)
		// 
		jpeg_set_defaults (&cinfo);
		cinfo.optimize_coding = TRUE;

		// Set non-default parameters, like quality (quantization table) scaling  
		// (we want 100 for the maximum quality, otherwise some junk floats around in the
		// areas that have large areas of that same color, even at 75 quality)
		// 
		jpeg_set_quality (&cinfo, 100, TRUE);

		// Start the jpeg compressor !
		//
		jpeg_start_compress (&cinfo, TRUE);

		// The length of each row in bytes is always a multiple of 4
		// Lets send the bottom scanline (height-1 to 0) first since we are working
		// with bitmaps.
		//
		row_stride = ((pBitmap2JPEG->image_width * 24 + 31) & ~31) >> 3;
		reverse_scanlines = pBitmap2JPEG->image_height - 1;

		while (reverse_scanlines >= 0) 
		{	// To keep things simple, we pass one scanline per call; you can pass
			// more if you wish.
			// jpeg_write_scanlines expects an array of pointers to scanlines.
			//
			row_pointer[0] = &pBitmap2JPEG->image_buffer[reverse_scanlines * row_stride];
			reverseRGB (row_pointer, cinfo.image_width);
			(void) jpeg_write_scanlines (&cinfo, row_pointer, 1);
			reverse_scanlines--;
		}

		jpeg_finish_compress (&cinfo);
		fclose (outfile);
		jpeg_destroy_compress (&cinfo);
	}

	// Over 130 possible points of failure in the jpeg code.  Catch any of the errors
	// and return false, "failed to create jpeg file" will be written to the log file.
	//
	__except (EXCEPTION_EXECUTE_HANDLER )
	{	bRet = FALSE;
	}

	return bRet;
}

// jb - 03.jan.99 -------------------------------------------------------------
//
// BITMAP INFO HEADER
// Contains information about the dimensions and color format of a DIB
//
void SetBitmapInfoHeader (LPBITMAPINFOHEADER lpbmih, BITMAP *bm)
{
	lpbmih->biSize   = sizeof (BITMAPINFOHEADER);
	lpbmih->biWidth  = bm->bmWidth;
	lpbmih->biHeight = bm->bmHeight;
	lpbmih->biPlanes = 1;
	lpbmih->biBitCount = 24;
	lpbmih->biCompression = BI_RGB;

	return;
}

// jb - 03.jan.99 -------------------------------------------------------------
//
// BITMAP FILE HEADER
// Must start with the bitmap FILE header.
// Calculate the offset the actual bitmap bits will be in the file.
// It's the Bitmap file header plus the DIB header, plus the size of the color table.
//
static void SetBitmapFileHeader (BITMAPFILEHEADER *bmfHdr, DWORD dwDIBSize, LPBITMAPINFOHEADER lpbmih)
{
	bmfHdr->bfType = DIB_HEADER_MARKER;  // "BM"
	bmfHdr->bfSize = dwDIBSize + sizeof (BITMAPFILEHEADER);
	bmfHdr->bfReserved1 = 0;
	bmfHdr->bfReserved2 = 0;
	bmfHdr->bfOffBits = (DWORD)sizeof (BITMAPFILEHEADER) + lpbmih->biSize + PaletteSize((LPSTR)lpbmih);

	return;
}

// jb - 03.jan.99 -------------------------------------------------------------
//
// Make the bitcount 24 bit (true color) for JPEG processing.
//
BOOL Bitmap4JPEG (LPBITMAP2JPEG pBitmap2JPEG)
{
	BOOL		bRet = TRUE;
	HPALETTE	hPalOld;
	long		lPaletteSize = 0;	// Since this bitmap will be 24-bit, no palette.
	long		lBitmap2JPEGSize;
	BITMAPFILEHEADER	bmfHdr;		// Header for Bitmap file
	BITMAPINFOHEADER	bitmapinfo;	// Body bits for Bitmap
	HBITMAP		hbmTemp = pBitmap2JPEG->hBmp;
	BITMAP		bm;

	pBitmap2JPEG->ghMem = 0;

	if (pBitmap2JPEG->hBmp)
	{	HDC hDC = GetDC (0);

		if (pBitmap2JPEG->hPal)
		{	hPalOld = SelectPalette (hDC, pBitmap2JPEG->hPal, TRUE);
			RealizePalette (hDC);
		}

		GetObject (pBitmap2JPEG->hBmp, sizeof(bm), &bm);

		// BITMAPINFOHEADER - Contains information about the dimensions and color format of a DIB
		//
		SetBitmapInfoHeader (&bitmapinfo, &bm);
		pBitmap2JPEG->image_width = bm.bmWidth; 
		pBitmap2JPEG->image_height = bm.bmHeight;

		GetDIBits (hDC, pBitmap2JPEG->hBmp, 0, (UINT)&bitmapinfo.biHeight, NULL, (LPBITMAPINFO)&bitmapinfo, DIB_RGB_COLORS);
		lBitmap2JPEGSize = sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER) + bitmapinfo.biSizeImage;

		// BITMAPFILEHEADER - Must start with the bitmap FILE header.
		//
		SetBitmapFileHeader (&bmfHdr, lBitmap2JPEGSize, &bitmapinfo);

		pBitmap2JPEG->ghMem = GlobalAlloc(GHND, lBitmap2JPEGSize);
		pBitmap2JPEG->lpBitmap = (LPVOID)GlobalLock(pBitmap2JPEG->ghMem);
		pBitmap2JPEG->lSize = lBitmap2JPEGSize;

		memcpy(pBitmap2JPEG->lpBitmap, &bmfHdr, sizeof (BITMAPFILEHEADER));
		memcpy((LPBYTE)pBitmap2JPEG->lpBitmap + sizeof (BITMAPFILEHEADER), &bitmapinfo, sizeof (BITMAPINFOHEADER));
		GetDIBits (hDC, hbmTemp, 0, (UINT)labs (bitmapinfo.biHeight), 
			       (LPBYTE)pBitmap2JPEG->lpBitmap + sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER) 
				   + lPaletteSize, (LPBITMAPINFO)&bitmapinfo, DIB_RGB_COLORS);

		if (pBitmap2JPEG->hPal)
		{	SelectPalette(hDC, hPalOld, TRUE);
		}

		ReleaseDC (0, hDC);
	}
	else
	{	bRet = FALSE;
	}

	return bRet;
}

// jb - 29.jan.99 -------------------------------------------------------------------------------
//
// free the jpeg memory.
//
void CleanUpJPEG (LPBITMAP2JPEG pBitmap2JPEG)
{
	GlobalUnlock(pBitmap2JPEG->ghMem);
	GlobalFree	(pBitmap2JPEG->ghMem);

	return;
}

// jb - 27.jan.99 -------------------------------------------------------------------------------
//
// Copy all the bitmap bits (after the file and info header) into the image_buffer.
// Then, call the jpeg compressor (write_JPEG_file) to make our file.
//
BOOL WriteJPEG2Disk(TCHAR *jpegname, LPBITMAP2JPEG pBitmap2JPEG)
{
	BOOL bRet;
	HGLOBAL	ghDibMem;
	int iPixelOffset;

	iPixelOffset = sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER);
	ghDibMem = GlobalAlloc (GHND, pBitmap2JPEG->lSize +1);
	pBitmap2JPEG->image_buffer = (JSAMPLE*)GlobalLock(ghDibMem);
	memcpy (pBitmap2JPEG->image_buffer, (JSAMPLE *)&(((LPBYTE)pBitmap2JPEG->lpBitmap)[iPixelOffset]), pBitmap2JPEG->lSize - iPixelOffset);

	bRet = WriteJPEG (jpegname, pBitmap2JPEG);

	CleanUpJPEG (pBitmap2JPEG);

	if (ghDibMem)
	{	GlobalUnlock (ghDibMem);
		GlobalFree   (ghDibMem);
	}

	return bRet;
}

// Called from ssbkhtml.c for jpeg support as well as Chris's chart.
//
//
BOOL _stdcall SaveBitmap2JPEG(HBITMAP hBmp, TCHAR *jpegname)
{
	BITMAP2JPEG bmpjpg;
  
	memset(&bmpjpg, '\0', sizeof(BITMAP2JPEG));
	bmpjpg.hBmp = hBmp; 
	Bitmap4JPEG(&bmpjpg);

	return WriteJPEG2Disk(jpegname, &bmpjpg);
}

// 04.jun.01 ------------------------------------------------------------------
//
// Save the jpeg to a buffer.  
// If buffer is NULL, then return size of the bitmap in lSize.  
//
BYTE * _stdcall SaveBitmap2Buffer (HBITMAP hBmp, BYTE *buf, long *lSize)
{
	BITMAP2JPEG bmpjpg;
	FILE *fp;
	TCHAR tmpJPEG[16];
	struct _stat fileinfo;

  	*lSize = 0;
	_tcscpy (tmpJPEG, _T("tmpfile"));
	memset (&bmpjpg, '\0', sizeof(BITMAP2JPEG));
	bmpjpg.hBmp = hBmp; 

	// Make sure the bitmap passed in is 24 bit.
	//
	Bitmap4JPEG (&bmpjpg);

	// If buf is NULL, we just want to return the size of the jpeg.
	// Leave JPEG so second time around, we can get it and stuff it into the
	// buffer, then delete it.  Worst case senerio, we have one tmpJPEG file
	// that is stranded if not called again.
	//
	if (!buf)
	{	if (WriteJPEG2Disk (tmpJPEG, &bmpjpg))
		{	
//GAB 12/20/01 changed for Unicode
		// *lSize = _stat (tmpJPEG, &fileinfo);
		  *lSize = _tstat (tmpJPEG, &fileinfo);
		}
		return NULL;
	}

	// Might as well get the size again..  Read the JPEG into the buf then
	// delete the temporary JPEG file.
	//
	else
	{	
//GAB 12/20/01 changed for Unicode
		//*lSize = _stat (tmpJPEG, &fileinfo);
		*lSize = _tstat (tmpJPEG, &fileinfo);
		if (fp = _tfopen (tmpJPEG, _T("rb")))
		{	fread (buf, 1, *lSize, fp);
			fclose (fp);
//GAB 12/20/01 changed for Unicode
			//remove (tmpJPEG);
			_tremove (tmpJPEG);
		}
		else
		{	return NULL;
		}
	}

	return buf;
}

