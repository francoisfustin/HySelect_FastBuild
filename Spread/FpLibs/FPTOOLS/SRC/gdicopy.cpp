/***********************************************************************
* gdicopy.cpp -- Functions to create copies of drag-and-drop data
*
* Copyright (C) 1991-1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/
#ifdef SS40
#include <windows.h>
#include "gditools.h"
#include "gdicopy.h"
#else
#include "gdicopy.h"
#include "fptools.h"
#endif

// creates a new global handle that contains the same data as an existing one
HRESULT CopyHGlobal(HGLOBAL hGlobal, HGLOBAL* phRet)
{
	if (!phRet)
		return E_POINTER;

	if (!hGlobal)
	{
		*phRet = 0;
		return S_OK;
	}

	long	lByteLen = GlobalSize(hGlobal);
	void*	lpSrc = GlobalLock(hGlobal);
	void*	lpDest;
	
	if (!lpSrc)
		return E_UNEXPECTED;

	if (*phRet = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, lByteLen))
	{
		lpDest = GlobalLock(*phRet);
		memcpy(lpDest, lpSrc, lByteLen);
		GlobalUnlock(*phRet);
		GlobalUnlock(hGlobal);
	}
	else
	{
		GlobalUnlock(hGlobal);
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

// creates a new HBITMAP from an existing HBITMAP
HRESULT CopyHBitmap(HBITMAP hBitmap, HBITMAP* phRet)
{
	if (!phRet)
		return E_POINTER;

	if (!hBitmap)
		*phRet = 0;
	else
	{
		HANDLE	hDib;
		HPALETTE hPal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);

		hDib = BitmapToDIB(hBitmap, hPal);
		*phRet = BitmapFromDib(hDib, hPal, TRUE);
		GlobalFree(hDib);
	}
	return S_OK;
}

// creates a new HPALETTE from an existing HPALETTE
HRESULT CopyHPalette(HPALETTE hPal, HPALETTE* phRet)
{	
	if (!phRet)
		return E_POINTER;

	if (!hPal)
		*phRet = 0;
	else
	{	// copy the palette
		UINT nEntries = GetPaletteEntries(hPal, 0, 0, NULL);
		if (!nEntries)
			return E_UNEXPECTED;
		GLOBALHANDLE hLogPal = GlobalAlloc(GMEM_DDESHARE, sizeof(LOGPALETTE) + (nEntries - 1) * sizeof(PALETTEENTRY));
		if (!hLogPal)
			return E_OUTOFMEMORY;
		LPLOGPALETTE pLogPal = (LPLOGPALETTE)GlobalLock(hLogPal);
		pLogPal->palVersion = 0x300;
		pLogPal->palNumEntries = (WORD)nEntries;
		if (GetPaletteEntries(hPal, 0, nEntries, pLogPal->palPalEntry))
		{
			GlobalUnlock(hLogPal);
			GlobalFree(hLogPal);
			return E_UNEXPECTED;
		}
		*phRet = CreatePalette(pLogPal);
		GlobalUnlock(hLogPal);
		GlobalFree(hLogPal);
	}
	return S_OK;
}

// creates a new HMETAFILEPICT from an existing HMETAFILEPICT
// (creates a new HMETAFILE for the new HMETAFILEPICT, too)
HRESULT CopyHMetaFilePict(HMETAFILEPICT hMetaFilePict, HMETAFILEPICT* phRet)
{
	if (!phRet)
		return E_POINTER;

	if (!hMetaFilePict)
		*phRet = 0;
	else
	{
		LPMETAFILEPICT pMetaSrc;	// source METAFILEPICT struct
		LPMETAFILEPICT pMetaDest;	// destination METAFILEPICT struct
		LPBYTE		   pMetaData;	// metafile data
		UINT		   nSize;		// size of metafile data
		
		pMetaSrc = (LPMETAFILEPICT)GlobalLock(hMetaFilePict);
		if (!pMetaSrc)
			return DV_E_TYMED;
		// get the size of the metafile data
		nSize = GetMetaFileBitsEx(pMetaSrc->hMF, 0, NULL);
		if (!nSize)
			return E_UNEXPECTED;
		// allocate memory for this data
		pMetaData = new BYTE[nSize];
		if (!pMetaData)
		{
			GlobalUnlock(hMetaFilePict);
			return E_OUTOFMEMORY;
		}
		// allocate memory for pMetaDest
		*phRet = GlobalAlloc(GMEM_DDESHARE, sizeof(METAFILEPICT));
		if (!*phRet)
		{
			GlobalUnlock(hMetaFilePict);
			delete pMetaData;
			return E_OUTOFMEMORY;
		}
		pMetaDest = (LPMETAFILEPICT)GlobalLock(*phRet);
		// get the metafile data
		GetMetaFileBitsEx(pMetaSrc->hMF, nSize, (LPVOID)pMetaData);
		// create a new metafile using this data
		pMetaDest->hMF = SetMetaFileBitsEx(nSize, pMetaData);
		// copy the other characteristics of the metafile to the new hMetaFilePict
		pMetaDest->mm = pMetaSrc->mm;
		pMetaDest->xExt = pMetaSrc->xExt;
		pMetaDest->yExt = pMetaSrc->yExt;
		// cleanup
		GlobalUnlock(hMetaFilePict);
		GlobalUnlock(*phRet);
		delete pMetaData;
	}
	return S_OK;
}

// creates a new HENHMETAFILE from an existing HMENHMETAFILE
HRESULT CopyHEnhMetaFile(HENHMETAFILE hEnhMetaFile, HENHMETAFILE* phRet)
{
	if (!phRet)
		return E_POINTER;

	if (!hEnhMetaFile)
		*phRet = 0;
	else
	{
		LPBYTE		   pMetaData;	// metafile data
		UINT		   nSize;		// size of metafile data
		
		// get the size of the metafile data
		nSize = GetEnhMetaFileBits(hEnhMetaFile, 0, NULL);
		if (!nSize)
			return E_UNEXPECTED;
		// allocate memory for this data
		pMetaData = new BYTE[nSize];
		if (!pMetaData)
		{
			GlobalUnlock(hEnhMetaFile);
			return E_OUTOFMEMORY;
		}
		// get the metafile data
		GetEnhMetaFileBits(hEnhMetaFile, nSize, pMetaData);
		// create a new hEnhMetaFile using this data
		*phRet = SetEnhMetaFileBits(nSize, pMetaData);
		// cleanup
		delete pMetaData;
	}
	return S_OK;
}

