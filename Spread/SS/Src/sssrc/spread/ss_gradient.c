//--------------------------------------------------------------------
//
//  File: ss_gradient.c
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
#if SS_V80

#include <windows.h>
#include <tchar.h>

typedef BOOL (__stdcall *GRADIENT_FILL)(HDC,PTRIVERTEX,ULONG,PVOID,ULONG,ULONG);
typedef BOOL (__stdcall *ALPHA_BLEND)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
#define MSIMG _T("msimg32.dll")

static HINSTANCE hInstMsimg = NULL;
static GRADIENT_FILL pGradientFill = NULL;
static ALPHA_BLEND pAlphaBlend = NULL;

#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA                0x01
#endif

BOOL SS_LoadMsImg()
{
   hInstMsimg = LoadLibrary(MSIMG);
   if( hInstMsimg != NULL )
   {
      pGradientFill = (GRADIENT_FILL)GetProcAddress(hInstMsimg, "GradientFill");
      pAlphaBlend = (ALPHA_BLEND)GetProcAddress(hInstMsimg, "AlphaBlend");
   }
   else 
      return FALSE;
   return TRUE;
}

void SS_UnloadMsImg()
{
   if( hInstMsimg )
   {
      FreeLibrary(hInstMsimg);
      hInstMsimg = NULL;
      pGradientFill = NULL;
      pAlphaBlend = NULL;
   }
}

void fpGradientFill(HDC hdc, RECT rect, HBRUSH hBrushTop, HBRUSH hBrushBottom, int direction)
{
   if( pGradientFill != NULL )
   {
	   GRADIENT_RECT gRect;
	   TRIVERTEX vertex[2] ;
	   LOGBRUSH logbrush1, logbrush2;

	   GetObject(hBrushTop, sizeof(LOGBRUSH), (LPVOID)&logbrush1);
	   GetObject(hBrushBottom, sizeof(LOGBRUSH), (LPVOID)&logbrush2);
	   vertex[0].x     = rect.left;
	   vertex[0].y     = rect.top;
	   vertex[0].Red   = (GetRValue(logbrush1.lbColor) << 8);
	   vertex[0].Green = (GetGValue(logbrush1.lbColor) << 8);
	   vertex[0].Blue  = (GetBValue(logbrush1.lbColor) << 8);
	   vertex[0].Alpha = 0x0000;

	   vertex[1].x     = rect.right;
	   vertex[1].y     = rect.bottom; 
	   vertex[1].Red   = (GetRValue(logbrush2.lbColor) << 8);
	   vertex[1].Green = (GetGValue(logbrush2.lbColor) << 8);
	   vertex[1].Blue  = (GetBValue(logbrush2.lbColor) << 8);
	   vertex[1].Alpha = 0x0000;

	   // Create a GRADIENT_RECT structure that
	   // references the TRIVERTEX vertices.
	   gRect.UpperLeft  = 0;
	   gRect.LowerRight = 1;

	   // Draw a shaded rectangle.
	   //GradientFill(hdc, vertex, 2, &gRect, 1, direction);
      pGradientFill(hdc, vertex, 2, &gRect, 1, direction);
   }
   else
      FillRect(hdc, &rect, hBrushTop);
}


void fpAlphaFill(HDC hdc, RECT rect, HBRUSH hBrush, COLORREF clrHighlight, BYTE alpha)
{
   if( pGradientFill )
   {
	   GRADIENT_RECT gRect;
	   TRIVERTEX vertex[2] ;
	   LOGBRUSH logbrush;
	   int r1, g1, b1;
	   int r2, g2, b2;
	   int rNew, gNew, bNew;

      GetObject(hBrush, sizeof(LOGBRUSH), (LPVOID)&logbrush);

	   r1 = GetRValue(logbrush.lbColor);
	   g1 = GetGValue(logbrush.lbColor);
	   b1 = GetBValue(logbrush.lbColor);

	   r2 = GetRValue(clrHighlight);
	   g2 = GetGValue(clrHighlight);
	   b2 = GetBValue(clrHighlight);

	   rNew = (r2 * alpha + r1 * (255 - alpha)) / 255;
	   gNew = (g2 * alpha + g1 * (255 - alpha)) / 255;
	   bNew = (b2 * alpha + b1 * (255 - alpha)) / 255;

	   vertex[0].x     = rect.left;
	   vertex[0].y     = rect.top;
	   vertex[0].Red   = (rNew << 8);
	   vertex[0].Green = (gNew << 8);
	   vertex[0].Blue  = (bNew << 8);
	   vertex[0].Alpha = 0;

	   vertex[1].x     = rect.right;
	   vertex[1].y     = rect.bottom; 
	   vertex[1].Red   = vertex[0].Red;
	   vertex[1].Green = vertex[0].Green;
	   vertex[1].Blue  = vertex[0].Blue;
	   vertex[1].Alpha = vertex[0].Alpha;

	   // Create a GRADIENT_RECT structure that
	   // references the TRIVERTEX vertices.
	   gRect.UpperLeft  = 0;
	   gRect.LowerRight = 1;

	   // Draw a shaded rectangle.
	   //GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
      pGradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
   }
   else
      FillRect(hdc, &rect, hBrush);
}

BOOL fpAlphaBlend(HDC hDC, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hDCSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc)
{
   BOOL ret = FALSE;
//   char buff[1000];

   if( pAlphaBlend )
   {
      BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
      ret = pAlphaBlend(hDC, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hDCSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, bf);
   }
   else
      StretchBlt(hDC, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hDCSrc,
              nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);
//   sprintf(buff, "ret: %d\r\nXOrgDest: %d YOrgDest: %d WDest: %d HDest: %d\r\nXOrgSrc: %d YOrgSrc: %d WSrc:  %d HSrc:  %d\r\n", ret, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc);
//   MessageBox(NULL, buff, "", MB_OK);
   return ret;
}
#endif // SS_V80