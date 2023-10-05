/*********************************************************
* IMAGES.CPP          
*
* Copyright (C) 1991-2003 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
*********************************************************/

#ifdef STRICT
#undef STRICT
#endif

#include <windows.h>
#include <tchar.h>
#include "ximage.h"
#include "fpimages.h"

extern "C"
{

HANDLE __stdcall FPLoadImage(LPCTSTR lpszFileName, short nFormat)
{
  HANDLE hRet = NULL;

  if( nFormat == CXIMAGE_FORMAT_ICO || nFormat == CXIMAGE_FORMAT_UNKNOWN )
    hRet = LoadImage(NULL, lpszFileName, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
  if( hRet == NULL && (nFormat == CXIMAGE_FORMAT_BMP || nFormat == CXIMAGE_FORMAT_UNKNOWN) )
    hRet = LoadImage(NULL, lpszFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  if( hRet == NULL )
  {
    CxImage image;
	  try
	  {
      if( lpszFileName && image.Load((char*)lpszFileName, nFormat) )
        hRet = image.MakeBitmap();
    }
	  catch(...)
	  {
	  }
  }
  return hRet;
}

HANDLE __stdcall FPLoadImageBuffer(HGLOBAL hGlobal, long lSize, short nFormat)
{
  BYTE* pBuffer;
  HANDLE hRet = NULL;

  if( pBuffer = (BYTE*)GlobalLock(hGlobal) )
  {
    if( nFormat == CXIMAGE_FORMAT_ICO || nFormat == CXIMAGE_FORMAT_UNKNOWN )
      hRet = TryLoadIconFromBuffer(hGlobal, lSize);
    if( hRet == NULL )
    {
      try
      {
        CxImage image(pBuffer, lSize, nFormat);
        hRet = image.MakeBitmap();
      }
      catch(...)
      {
      }
    }
  }
  return hRet;
}

HANDLE __stdcall FPLoadResImage(HINSTANCE hInstance, LPCTSTR lpszResName, LPCTSTR lpszResType, short nFormat)
{
  HANDLE hRet = NULL;
  LPTSTR lpszType = NULL;

  if( lpszResType && HIWORD(lpszResType) && *lpszResType )
  {
    lpszType = (LPTSTR)malloc((lstrlen(lpszResType) + 1) * sizeof(TCHAR));
    lstrcpy(lpszType, lpszResType);
    _tcsupr(lpszType);
  }
  if( (lpszResType == RT_ICON || (lpszType && (!_tcscmp(lpszType, _T("#3")) || !_tcscmp(lpszType, _T("ICON"))))) && (nFormat == CXIMAGE_FORMAT_ICO || nFormat == CXIMAGE_FORMAT_UNKNOWN) )
    hRet = LoadImage(hInstance, lpszResName, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
  if( hRet == NULL && ((lpszResType == RT_BITMAP || (lpszType && (!_tcscmp(lpszType, _T("#2")) || !_tcscmp(lpszType, _T("BITMAP"))))) && (nFormat == CXIMAGE_FORMAT_BMP || nFormat == CXIMAGE_FORMAT_UNKNOWN)) )
    hRet = LoadImage(hInstance, lpszResName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
  if( hRet == NULL )
  {
    CxImage image;
    HRSRC hResource = FindResource(hInstance, lpszResName, lpszResType);
	  try
	  {
      if( hResource && image.LoadResource((HRSRC)hResource, nFormat, hInstance) )
        hRet = image.MakeBitmap();
    }
	  catch(...)
	  {
	  }
  }
  if( lpszType )
    free(lpszType);
  return hRet;
}

BOOL FAR __stdcall FPSaveImage(HANDLE hImage, HPALETTE hPal, LPCTSTR lpszFileName, short nFormat)
{
  CxImage image;
  WORD bpp;
  BOOL ret = FALSE;

  if( lpszFileName )
  {
    if( IsIcon(hImage) )
    {
	    try
	    {
        image.CreateFromHICON((HICON)hImage);
        bpp = image.GetBpp();
// GAB -  We will not be supporting pgx format 
        /*
        if( nFormat == CXIMAGE_FORMAT_PGX && !image.IsGrayScale() )
          image.GrayScale();
        else if( nFormat == CXIMAGE_FORMAT_WBMP && bpp > 1 )
          image.DecreaseBpp(1, FALSE);
        */
        if( nFormat == CXIMAGE_FORMAT_WBMP && bpp > 1 )
          image.DecreaseBpp(1, FALSE);
        ret = (BOOL)image.Save(lpszFileName, nFormat);
      }
	    catch(...)
	    {
	    }
    }
    else if( IsBitmap(hImage) )
    {
	    try
	    {
        image.CreateFromHBITMAP((HBITMAP)hImage);
        bpp = image.GetBpp();
// GAB -  We will not be supporting pgx format 
        /*
        if( nFormat == CXIMAGE_FORMAT_PGX && !image.IsGrayScale() )
          image.GrayScale();
        else if( nFormat != CXIMAGE_FORMAT_WBMP )
        */
		if( nFormat == CXIMAGE_FORMAT_WBMP && bpp > 1 )
          image.DecreaseBpp(1, FALSE);
        else if( nFormat == CXIMAGE_FORMAT_GIF && bpp > 8 )
		  image.DecreaseBpp(8, FALSE);
		else if( nFormat != CXIMAGE_FORMAT_WBMP && nFormat != CXIMAGE_FORMAT_GIF )
		{
		  if( bpp < 24 )
		    image.IncreaseBpp(24);
		  else if( bpp > 24 )
		    image.DecreaseBpp(24, FALSE);
		}
        ret = (BOOL)image.Save(lpszFileName, nFormat);
      }
	    catch(...)
	    {
	    }
    }
  }
  return ret;
}

BOOL __stdcall FPSaveImageBuffer(HANDLE hImage, HPALETTE hPal, short nFormat, HGLOBAL *phGlobal, long *plSize)
{
  CxImage image;
  WORD bpp;
  BOOL ret = FALSE;
  BYTE *pBuffer = NULL, *pTemp = NULL;

  if( phGlobal && plSize )
  {
    if( IsIcon(hImage) )
    {
      if( nFormat == CXIMAGE_FORMAT_ICO )
        return SaveIconToBuffer((HICON)hImage, phGlobal, plSize);
      else
      {
	      try
	      {
          image.CreateFromHICON((HICON)hImage);
          bpp = image.GetBpp();
// GAB -  We will not be supporting pgx format 
          /*
          if( nFormat == CXIMAGE_FORMAT_PGX && !image.IsGrayScale() )
            image.GrayScale();
          else if( nFormat == CXIMAGE_FORMAT_WBMP && bpp > 1 )
            image.DecreaseBpp(1, FALSE);
          */
          if( nFormat == CXIMAGE_FORMAT_WBMP && bpp > 1 )
            image.DecreaseBpp(1, FALSE);
          ret = image.Encode(pBuffer, *plSize, nFormat);
        }
	      catch(...)
	      {
	      }
      }
    }
    else if( IsBitmap(hImage) )
    {
	    try
	    {
        image.CreateFromHBITMAP((HBITMAP)hImage);
        bpp = image.GetBpp();
// GAB -  We will not be supporting pgx format 
        /*
        if( nFormat == CXIMAGE_FORMAT_PGX && !image.IsGrayScale() )
          image.GrayScale();
        else if( nFormat != CXIMAGE_FORMAT_WBMP )
        */
        if( nFormat != CXIMAGE_FORMAT_WBMP )
        {
          if( bpp < 24 )
            image.IncreaseBpp(24);
          else if( bpp > 24 )
            image.DecreaseBpp(24, FALSE);
        }
        else if( bpp > 1 )
          image.DecreaseBpp(1, FALSE);
        ret = image.Encode(pBuffer, *plSize, nFormat);
      }
	    catch(...)
	    {
	    }
    }
    if( ret )
    {
      *phGlobal = GlobalAlloc(GHND, *plSize);
      pTemp = (BYTE*)GlobalLock(*phGlobal);
      if( pTemp )
      {
        memcpy(pTemp, pBuffer, *plSize);
        GlobalUnlock(phGlobal);
      }
      else
        ret = FALSE;
      free(pBuffer);
    }
  }
  return ret;
}

// 18027 -scl
//BOOL SaveIconToBuffer(HICON hIcon, HGLOBAL *phGlobal, long *plSize)
BOOL __stdcall SaveIconToBuffer(HICON hIcon, HGLOBAL *phGlobal, long *plSize)
{
  BOOL ret = FALSE;
  ICONINFO iconinfo;
  BYTE *pBufferAnd, *pBufferXor, *pTemp;
  long lSizeAnd, lSizeXor = 0;
  CxImage image;

  if( phGlobal && plSize && GetIconInfo(hIcon, &iconinfo) )
  {
    image.CreateFromHBITMAP(iconinfo.hbmMask);
    image.Encode(pBufferAnd, lSizeAnd, CXIMAGE_FORMAT_BMP);
    if( iconinfo.hbmColor )
    {
      image.Clear();
      image.CreateFromHBITMAP(iconinfo.hbmColor);
      image.Encode(pBufferXor, lSizeXor, CXIMAGE_FORMAT_BMP);
    }
    *phGlobal = GlobalAlloc(GHND, 2 * sizeof(long) + lSizeAnd + lSizeXor);
    if( pTemp = (BYTE*)GlobalLock(*phGlobal) )
    {
      *(LPLONG)pTemp = lSizeAnd;
      memcpy(pTemp + sizeof(long), pBufferAnd, lSizeAnd);
      *(LPLONG)(pTemp + sizeof(long) + lSizeAnd) = lSizeXor;
      if( lSizeXor )
        memcpy(pTemp + 2 * sizeof(long) + lSizeAnd, pBufferXor, lSizeXor);
      GlobalUnlock(*phGlobal);
      *plSize = 2 * sizeof(long) + lSizeAnd + lSizeXor;
      ret = TRUE;
    }
    free(pBufferAnd);
    if( iconinfo.hbmColor )
      free(pBufferXor);
  }
  return ret;
}

// 18027 -scl
//HICON TryLoadIconFromBuffer(HGLOBAL hGlobal, long lSize)
HICON __stdcall TryLoadIconFromBuffer(HGLOBAL hGlobal, long lSize)
{
  HICON hRet = NULL;
  ICONINFO iconinfo = {TRUE, 0, 0, 0, 0};
  BYTE *pBufferAnd, *pBufferXor, *pTemp;
  long lSizeAnd = 0, lSizeXor = 0;

  if( lSize <= 2 * sizeof(long) )
    return NULL;
  if( pTemp = (BYTE*)GlobalLock(hGlobal) )
  {
    CxImage image;

    lSizeAnd = *(LPLONG)pTemp;
    pBufferAnd = pTemp + sizeof(long);
    if( image.Decode(pBufferAnd, lSizeAnd, CXIMAGE_FORMAT_BMP) )
    {
      iconinfo.hbmMask = image.MakeBitmap();
      if( lSize > (long)(lSizeAnd + 2 * sizeof(long)) )
      {
        lSizeXor = *(LPLONG)(pTemp + sizeof(long) + lSizeAnd);
        if( lSizeXor )
        {
          pBufferXor = pTemp + 2 * sizeof(long) + lSizeAnd;
          image.Clear();
          if( image.Decode(pBufferXor, lSizeXor, CXIMAGE_FORMAT_BMP) )
            iconinfo.hbmColor = image.MakeBitmap();
        }
      }
      hRet = CreateIconIndirect(&iconinfo);
    }
  }
  return hRet;
}

// 18027 -scl
//BOOL IsIcon(HANDLE hImage)
BOOL __stdcall IsIcon(HANDLE hImage)
{
  ICONINFO iconinfo;

  if( GetIconInfo((HICON)hImage, &iconinfo) )
  {
    DeleteObject(iconinfo.hbmColor);
    DeleteObject(iconinfo.hbmMask);
    return TRUE;
  }
  return FALSE;
}

// 18027 -scl
//BOOL IsBitmap(HANDLE hImage)
BOOL __stdcall IsBitmap(HANDLE hImage)
{
  if( GetObject(hImage, sizeof(BITMAP), NULL) )
	  return TRUE;
  return FALSE;
}

} // extern "C"
