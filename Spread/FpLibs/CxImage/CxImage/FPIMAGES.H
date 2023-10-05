/*********************************************************
* IMAGES.H          
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

#ifndef FPSAVE_H
#define FPSAVE_H

#define IMAGE_FORMAT_UNKNOWN  0
#define IMAGE_FORMAT_BMP     1
#define IMAGE_FORMAT_GIF     2
#define IMAGE_FORMAT_JPG     3
#define IMAGE_FORMAT_PNG     4
#define IMAGE_FORMAT_ICO     5
#define IMAGE_FORMAT_TIF     6
#define IMAGE_FORMAT_TGA     7
#define IMAGE_FORMAT_PCX     8
#define IMAGE_FORMAT_WBMP    9
#define IMAGE_FORMAT_WMF     10

#ifdef __cplusplus
extern "C" {
#endif

HANDLE __stdcall FPLoadImage(LPCSTR lpszFileName, short nFormat);
HANDLE __stdcall FPLoadImageBuffer(HGLOBAL hGlobal, long lSize, short nFormat);
HANDLE __stdcall FPLoadResImage(HINSTANCE hInstance, LPCSTR lpszResName, LPCSTR lpszResType, short nFormat);
BOOL __stdcall FPSaveImage(HANDLE hImage, HPALETTE hPal, LPCSTR lpszFileName, short nFormat);
BOOL __stdcall FPSaveImageBuffer(HANDLE hImage, HPALETTE hPal, short nFormat, HGLOBAL *phGlobal, long *plSize);
// 18027 -scl
//BOOL IsIcon(HANDLE hImage);
//BOOL IsBitmap(HANDLE hImage);
//BOOL SaveIconToBuffer(HICON hIcon, HGLOBAL *phGlobal, long *plSize);
//HICON TryLoadIconFromBuffer(HGLOBAL hGlobal, long lSize);
BOOL __stdcall IsIcon(HANDLE hImage);
BOOL __stdcall IsBitmap(HANDLE hImage);
BOOL __stdcall SaveIconToBuffer(HICON hIcon, HGLOBAL *phGlobal, long *plSize);
HICON __stdcall TryLoadIconFromBuffer(HGLOBAL hGlobal, long lSize);

typedef HANDLE (_stdcall *FPLOADIMAGEPROC)(LPCSTR, short);
typedef HANDLE (_stdcall *FPLOADIMAGEBUFFERPROC)(HGLOBAL, long, short);
typedef HANDLE (_stdcall *FPLOADRESIMAGEPROC)(HINSTANCE, LPCSTR, LPCSTR, short);
typedef BOOL (FAR __stdcall *FPSAVEIMAGEPROC)(HANDLE, HPALETTE, LPCSTR, short);
//typedef BOOL (_stdcall *FPSAVEIMAGEPROC)(HANDLE, HPALETTE, LPCSTR, short);
typedef BOOL (_stdcall *FPSAVEIMAGEBUFFERPROC)(HANDLE, HPALETTE, short, HGLOBAL*, long*);
typedef BOOL (_stdcall *ISICONPROC)(HANDLE);
typedef BOOL (_stdcall *ISBITMAPPROC)(HANDLE);
typedef BOOL (_stdcall *SAVEICONTOBUFFERPROC)(HICON, HGLOBAL*, long*);
typedef HICON (_stdcall *TRYLOADICONFROMBUFFERPROC)(HGLOBAL, long);

#ifdef __cplusplus
}
#endif    

#endif
