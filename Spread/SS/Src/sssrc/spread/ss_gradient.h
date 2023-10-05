//--------------------------------------------------------------------
//
//  File: ss_gradient.h
//
//  Description: Gradient drawing methods
//
//  Copyright (c) 2008 by FarPoint Technologies, Inc.
//
//  All rights reserved.  No part of this source code may be
//  copied, modified or reproduced in any form without retaining
//  the above copyright notice.  This source code, or source code
//  derived from it, may not be redistributed without express
//  written permission of FarPoint Technologies, Inc.
//
//--------------------------------------------------------------------
#ifdef SS_V80

#ifndef SS_GRADIENT_H
#define SS_GRADIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#if (WINVER < 0x0500)
typedef struct {
        DWORD        bV5Size;
        LONG         bV5Width;
        LONG         bV5Height;
        WORD         bV5Planes;
        WORD         bV5BitCount;
        DWORD        bV5Compression;
        DWORD        bV5SizeImage;
        LONG         bV5XPelsPerMeter;
        LONG         bV5YPelsPerMeter;
        DWORD        bV5ClrUsed;
        DWORD        bV5ClrImportant;
        DWORD        bV5RedMask;
        DWORD        bV5GreenMask;
        DWORD        bV5BlueMask;
        DWORD        bV5AlphaMask;
        DWORD        bV5CSType;
        CIEXYZTRIPLE bV5Endpoints;
        DWORD        bV5GammaRed;
        DWORD        bV5GammaGreen;
        DWORD        bV5GammaBlue;
        DWORD        bV5Intent;
        DWORD        bV5ProfileData;
        DWORD        bV5ProfileSize;
        DWORD        bV5Reserved;
} BITMAPV5HEADER, FAR *LPBITMAPV5HEADER, *PBITMAPV5HEADER;

// Values for bV5CSType
#define PROFILE_LINKED          'LINK'
#define PROFILE_EMBEDDED        'MBED'
#endif

BOOL SS_LoadMsImg();
void SS_UnloadMsImg();
void fpGradientFill(HDC hdc, RECT rect, HBRUSH hBrushTop, HBRUSH hBrushBottom, int direction);
void fpAlphaFill(HDC hdc, RECT rect, HBRUSH hBrush, COLORREF clrHighlight, BYTE alpha);
BOOL fpAlphaBlend(HDC hDC, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hDCSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc);

#ifdef __cplusplus
}
#endif

#endif // !SS_GRADIENT_H

#endif // SS_V80