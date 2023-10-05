/*********************************************************
* SPINBTN.C
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
#include <windowsx.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <process.h>
#include "toolbox.h"
#include "fphdc.h"
#include "spinbtn.h"
#include "wintools.h"
#ifdef SS_V80
#include "uxtheme.h"
#include "ss_theme.h"
#endif
static BOOL      fVisualStyles;

#define SPINBTN_ISARROW(Style, Arrow) ((Style & 0x03) == Arrow)

#define SPINBTN_SHADOWSIZE   1
#define SPINBTN_LINESIZE     1
#define SPINBTN_XOFFSET_DEF  6
#define SPINBTN_YOFFSET_DEF  4

//#define SPINBTN_TIMEDELAY    50
//#define SPINBTN_INITIALDELAY 120
#define SPINBTN_TIMEDELAY    100
#define SPINBTN_INITIALDELAY 200

#define SPINBTN_UP           0
#define SPINBTN_DOWN         1
#define SPINBTN_LEFT         2
#define SPINBTN_RIGHT        3

#define SPINBTN_XOFFSET(lpRect) min(SPINBTN_XOFFSET_DEF, max(2, (lpRect->right - lpRect->left - 5) / 2))
#define SPINBTN_YOFFSET(lpRect) min(SPINBTN_YOFFSET_DEF, max(1, (lpRect->bottom - lpRect->top - 5) / 2))

#define SPINBTN_XOFFSET_UPDN(lpRect) min(SPINBTN_YOFFSET_DEF, max(1, (lpRect->right - lpRect->left - 5) / 2))
#define SPINBTN_YOFFSET_UPDN(lpRect) min(SPINBTN_XOFFSET_DEF, max(2, (lpRect->bottom - lpRect->top - 5) / 2))

WINENTRY                tbSpinBtnWndFn(HWND hWnd, UINT Msg, WPARAM wParam,
                                       LPARAM lParam);
BOOL                    SpinBtnPaint(HWND hWnd, HDC hDC);
short                   SpinBtnSetUpArrow(LPPOINT lpPoints, LPRECT Rect);
short                   SpinBtnSetDownArrow(LPPOINT lpPoints, LPRECT Rect);
short                   SpinBtnSetRightArrow(LPPOINT lpPoints, LPRECT Rect);
short                   SpinBtnSetLeftArrow(LPPOINT lpPoints, LPRECT Rect);
long                    SpinBtnGetStyle(HWND hWnd);
short                   SpinBtnGetScrollArea(LPRECT RectClient,
                                             LPRECT RectTemp, long Style,
                                             LPARAM lParam);
void                    SpinBtnDrawUpArrow(HDC hDC, LPRECT Rect);
void                    SpinDrawArrow(HWND hWnd, HDC hDC, LPPOINT lpPoints,
                                      short dHighlight);
void                    Spin_GetUpDnArrowRect(LPRECT lpRectArrow, LPRECT lpRect);
void                    Spin_GetLtRtArrowRect(LPRECT lpRectArrow, LPRECT lpRect);

#ifdef SS_V80
void				SpinBtnVisualStylePaint(HWND hWnd, HDC hDC, RECT Rect, long Style, short scrollDirection, BOOL mouseOver);
#endif
BOOL RegisterSpinBtn(hInstance, lpszClassName)

HANDLE     hInstance;
LPCTSTR    lpszClassName;
{
WNDCLASS wc;
BOOL     bRet = TRUE;

wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc   = tbSpinBtnWndFn;
wc.cbClsExtra    = 0;
wc.cbWndExtra    = 2;
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = lpszClassName;
if (!RegisterClass(&wc))
   bRet = FALSE;

wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc   = tbSpinBtnWndFn;
wc.cbClsExtra    = 0;
wc.cbWndExtra    = 2;
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = _T("tbSpinButton");
if (!RegisterClass(&wc))
   bRet = FALSE;

return bRet;
}


WINENTRY tbSpinBtnWndFn(hWnd, Msg, wParam, lParam)

HWND        hWnd;
UINT        Msg;
WPARAM      wParam;
LPARAM      lParam;
{
PAINTSTRUCT Paint;
HDC         hDC;
HDC         hDCMemory;
HBITMAP     hBitmapOld;
HBITMAP     hBitmapDC;
RECT        RectClient;
RECT        Rect;
RECT        RectTemp;
DWORD       dwTime;
long        Style;
short       ScrollDirection;

switch (Msg)
   {
   case WM_CREATE:
      SetWindowWord(hWnd, 0, FALSE);
      return (0);

   case WM_GETDLGCODE:
      return (DLGC_WANTARROWS);

   case EM_SETSPINSTYLE:
	   fVisualStyles = (BOOL)lParam;
	   return (0);
   case WM_KEYDOWN:
      Style = SpinBtnGetStyle(hWnd);

      switch (wParam)
         {
         case VK_UP:
         case VK_LEFT:
            if (SPINBTN_ISARROW(Style, SBNS_UPARROW) ||
                SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
               if (Style & SBNS_HORIZONTAL)
                  FORWARD_WM_HSCROLL(hWnd, hWnd, SB_LINEUP, 0,
                                     tbSendMessageToParent);
               else
                  FORWARD_WM_VSCROLL(hWnd, hWnd, SB_LINEUP, 0,
                                     tbSendMessageToParent);                             
            return (0);

         case VK_DOWN:
         case VK_RIGHT:
            if (SPINBTN_ISARROW(Style, SBNS_DOWNARROW) ||
                SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
               if (Style & SBNS_HORIZONTAL)
                  FORWARD_WM_HSCROLL(hWnd, hWnd, SB_LINEDOWN, 0,
                                     tbSendMessageToParent);
               else
                  FORWARD_WM_VSCROLL(hWnd, hWnd, SB_LINEDOWN, 0,
                                     tbSendMessageToParent);
            return (0);
         }

      return (0);

   case WM_LBUTTONDOWN:
		{
		long button;
		BOOL fFirstTime;

      if (GetSystemMetrics(SM_SWAPBUTTON))
			button = VK_RBUTTON;
		else
			button = VK_LBUTTON;

      Style = SpinBtnGetStyle(hWnd);
      GetClientRect(hWnd, &RectClient);

      ScrollDirection = SpinBtnGetScrollArea(&RectClient, &RectTemp, Style,
                                             lParam);

      /***********************************
      * Invert the appropriate Rectangle
      ***********************************/

      hDC = fpGetDC(hWnd);
#ifdef SS_V80
	  if (fVisualStyles && SS_IsThemeActive())
	  {
		  SpinBtnVisualStylePaint(hWnd, hDC, RectClient, Style, ScrollDirection, TRUE);
	  }
	  else
#endif
		InvertRect(hDC, &RectTemp);
      ReleaseDC(hWnd, hDC);

      SetCapture(hWnd);

      SetWindowWord(hWnd, 0, TRUE);
      dwTime = GetTickCount();

      while (dwTime + SPINBTN_INITIALDELAY > GetTickCount() &&
             (GetAsyncKeyState(button) & 0x8000) != 0)
         ;

		fFirstTime = TRUE;
      for (;;)
         {
         if (dwTime + SPINBTN_TIMEDELAY <= GetTickCount() || fFirstTime)
            {
            switch (ScrollDirection)
               {
               case SPINBTN_UP:
                  FORWARD_WM_VSCROLL(hWnd, hWnd, SB_LINEUP, 0,
                                     tbSendMessageToParent);
                  break;

               case SPINBTN_DOWN:
                  FORWARD_WM_VSCROLL(hWnd, hWnd, SB_LINEDOWN, 0,
                                     tbSendMessageToParent);
                  break;

               case SPINBTN_LEFT:
                  FORWARD_WM_HSCROLL(hWnd, hWnd, SB_LINEUP, 0,
                                     tbSendMessageToParent);
                  break;

               case SPINBTN_RIGHT:
                  FORWARD_WM_HSCROLL(hWnd, hWnd, SB_LINEDOWN, 0,
                                     tbSendMessageToParent);
                  break;
               }

            dwTime = GetTickCount();
            }

         if (GetSystemMetrics(SM_SWAPBUTTON))
            {
            if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
               break;
            }
         else
            if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
               break;

			fFirstTime = FALSE;
         }

      ReleaseCapture();
      SetWindowWord(hWnd, 0, FALSE);

      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
		}
      return (0);

   case WM_ERASEBKGND:
      return (1);

#if kmV8
   case WM_MOUSELEAVE:
	   SpinBtnVisualStylePaint(hWnd, hDC, Rect, Style, -1, FALSE);
	   break;

   case WM_MOUSEMOVE:
	   {

	   TRACKMOUSEEVENT tme;
       lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
	   if (lpSuperBtn->MouseOver == FALSE)
	   {
		 lpSuperBtn->MouseOver = TRUE;
		 tme.cbSize = sizeof(TRACKMOUSEEVENT);
		 tme.dwFlags = TME_LEAVE;
		 tme.hwndTrack = hWnd;
		 tme.dwHoverTime = HOVER_DEFAULT;
		 _TrackMouseEvent(&tme);

  		InvalidateRect(hWnd, NULL, FALSE);
	   }
		GlobalUnlock(hGlobal);
	   }
#endif
   case WM_PAINT:
      hDC = fpBeginPaint(hWnd, &Paint);
      GetClientRect(hWnd, &Rect);

      /*********************************
      * Create a memory device context
      *********************************/

      hBitmapDC = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,
                                         Rect.bottom - Rect.top);

      hDCMemory = CreateCompatibleDC(hDC);
      hBitmapOld = SelectObject(hDCMemory, hBitmapDC);

      if (SpinBtnPaint(hWnd, hDCMemory))
         {
         /*******************************************************
         * Copy the memory device context bitmap to the display
         *******************************************************/

         BitBlt(hDC, Paint.rcPaint.left, Paint.rcPaint.top,
                Paint.rcPaint.right - Paint.rcPaint.left,
                Paint.rcPaint.bottom - Paint.rcPaint.top, hDCMemory,
                Paint.rcPaint.left, Paint.rcPaint.top, SRCCOPY);
         }

      /***********
      * Clean up
      ***********/

      SelectObject(hDCMemory, hBitmapOld);
      DeleteDC(hDCMemory);
      DeleteObject(hBitmapDC);

      EndPaint(hWnd, &Paint);
      return (0);
   }

return (DefWindowProc(hWnd, Msg, wParam, lParam));
}


#ifdef SS_V80
void SpinBtnVisualStylePaint(HWND hWnd, HDC hDC, RECT Rect, long Style, short scrollDirection, BOOL mouseOver)
{
    HTHEME hTheme = NULL;
	short state = 1;
	hTheme = SS_OpenThemeData(hWnd, L"Spin");
    
	if (Style & SBNS_VERTICAL)
    {
	  if (SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
      {
      Rect.top += (Rect.bottom - Rect.top) / 2;

      /******************
      * Draw Down Arrow
      ******************/
	  SS_DrawThemeBackground( hTheme, hDC, 2, scrollDirection == SPINBTN_DOWN ? 3 : mouseOver ? 2 : 1, &Rect, &Rect);


      /****************
      * Draw Up Arrow
      ****************/

      Rect.bottom = Rect.top + 1;
      Rect.top = 0;

	  SS_DrawThemeBackground( hTheme, hDC, 1, scrollDirection == SPINBTN_UP ? 3 : mouseOver ? 2 : 1, &Rect, &Rect);
      }

   else
      {
      if (SPINBTN_ISARROW(Style, SBNS_UPARROW))
			SS_DrawThemeBackground( hTheme, hDC, 1, scrollDirection == SPINBTN_UP ? 3 : mouseOver ? 2 : 1, &Rect, &Rect);
      else
			SS_DrawThemeBackground( hTheme, hDC, 2, scrollDirection == SPINBTN_DOWN ? 3 : mouseOver ? 2 : 1, &Rect, &Rect);

      }
	  
   /*else
      {
      if (SPINBTN_ISARROW(Style, SBNS_UPARROW))
         dHighlight = SpinBtnSetUpArrow(lpPoints, &Rect);
      else
         dHighlight = SpinBtnSetDownArrow(lpPoints, &Rect);

      SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight);
      }
	  */
	  
   }

   else                                   // Horizontal
   {
    if (SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
      {
      Rect.left += (Rect.right - Rect.left) / 2;

      /*******************
      * Draw Right Arrow
      *******************/
	  SS_DrawThemeBackground( hTheme, hDC, 3, scrollDirection == SPINBTN_RIGHT ? 3 : mouseOver ? 2 : 1, &Rect, &Rect);

      /******************
      * Draw Left Arrow
      ******************/

      Rect.right = Rect.left + 1;
      Rect.left = 0;
	  SS_DrawThemeBackground( hTheme, hDC, 4, scrollDirection == SPINBTN_LEFT ? 3 : mouseOver ? 2 : 1, &Rect, &Rect);

      }

   else
      {
      if (SPINBTN_ISARROW(Style, SBNS_UPARROW))
			SS_DrawThemeBackground( hTheme, hDC, 4, scrollDirection == SPINBTN_LEFT ? 3 : mouseOver ? 2 : 1, &Rect, &Rect);
      else
			SS_DrawThemeBackground( hTheme, hDC, 3, scrollDirection == SPINBTN_RIGHT ? 3 : mouseOver ? 2 : 1, &Rect, &Rect);

      }
   /*else
      {
      if (SPINBTN_ISARROW(Style, SBNS_UPARROW))
         dHighlight = SpinBtnSetLeftArrow(lpPoints, &Rect);
      else
         dHighlight = SpinBtnSetRightArrow(lpPoints, &Rect);

      SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight);
      }
	  */
	  
   }	
	
	SS_CloseThemeData(hTheme);
}

#endif 
BOOL SpinBtnPaint(hWnd, hDC)

HWND         hWnd;
HDC          hDC;
{
HBRUSH       hBrush;
RECT         Rect;
RECT         RectTemp;
POINT        lpPoints[5];
long         Style;
short        dHighlight;
#if defined(_WIN64) || defined(_IA64)
long         x;
long         y;
#else
short        x;
short        y;
#endif

#ifdef SS_V80
BOOL visualStyles = fVisualStyles && SS_IsThemeActive();
#endif

if (GetWindowWord(hWnd, 0))
   return (FALSE);

SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
SetBkColor(hDC, GetSysColor(COLOR_WINDOW));

GetClientRect(hWnd, &Rect);
Style = SpinBtnGetStyle(hWnd);

#ifdef SS_V80
if (visualStyles)
{
	SpinBtnVisualStylePaint(hWnd, hDC, Rect, Style, -1, FALSE);
}
else
{
#endif
/*******************
* Erase background
*******************/

//hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
//hBrush = CreateSolidBrush(RGBCOLOR_PALEGRAY);
hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
FillRect(hDC, &Rect, hBrush);
DeleteObject(hBrush);

hBrush = GetStockObject(BLACK_BRUSH);

if (Style & SBNS_BORDERLEFT)
   {
   MoveToEx(hDC, 0, 0, NULL);
   LineTo(hDC, 0, Rect.bottom);
   Rect.left++;
   }

if (Style & SBNS_BORDERRIGHT)
   {
   MoveToEx(hDC, Rect.right - 1, 0, NULL);
   LineTo(hDC, Rect.right - 1, Rect.bottom);
   Rect.right--;
   }

if (Style & SBNS_BORDERTOP)
   {
   MoveToEx(hDC, 0, 0, NULL);
   LineTo(hDC, Rect.right, 0);
   Rect.top++;
   }

if (Style & SBNS_BORDERBOTTOM)
   {
   MoveToEx(hDC, 0, Rect.bottom - 1, NULL);
   LineTo(hDC, Rect.right, Rect.bottom - 1);
   Rect.bottom--;
   }

if (!(Style & (SBNS_BORDERLEFT | SBNS_BORDERRIGHT | SBNS_BORDERTOP |
      SBNS_BORDERBOTTOM | SBNS_BORDERNONE)))
   {
   FrameRect(hDC, &Rect, hBrush);
   tbShrinkRect(&Rect, 1, 1);
   }

/*
tbDrawShadows(hDC, &Rect, 1, GetSysColor(COLOR_BTNSHADOW), RGBCOLOR_WHITE,
              FALSE, FALSE);
*/

tbDrawShadows(hDC, &Rect, 1, RGBCOLOR_DARKGRAY, RGBCOLOR_WHITE, FALSE, FALSE);

/*******************************
* Draw Horizontal Shadow Lines
*******************************/

if ((Style & SBNS_VERTICAL) && (SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW)))
   {
   y = (Rect.bottom - Rect.top) / 2;

   RectTemp.left = Rect.left;
   RectTemp.right = Rect.right;
   RectTemp.top = Rect.top + y;
   RectTemp.bottom = Rect.top + y + SPINBTN_LINESIZE;

   /*
   hBrush = CreateSolidBrush(COLOR_BTNSHADOW);
   */

   hBrush = CreateSolidBrush(RGBCOLOR_DARKGRAY);
   FillRect(hDC, &RectTemp, hBrush);
   DeleteObject(hBrush);
   }

/*****************************
* Draw Vertical Shadow Lines
*****************************/

if ((Style & SBNS_HORIZONTAL) && SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
   {
   x = (Rect.right - Rect.left) / 2;

   RectTemp.left = Rect.left + x;
   RectTemp.right = Rect.left + x + SPINBTN_LINESIZE;
   RectTemp.top = Rect.top;
   RectTemp.bottom = Rect.bottom;

   /*
   hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
   */
   hBrush = CreateSolidBrush(RGBCOLOR_DARKGRAY);
   FillRect(hDC, &RectTemp, hBrush);
   DeleteObject(hBrush);
   }

/**************
* Draw Arrows
**************/

if (Style & SBNS_VERTICAL)
   {
   if (SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
      {
      Rect.top += (Rect.bottom - Rect.top) / 2;

      /******************
      * Draw Down Arrow
      ******************/

      dHighlight = SpinBtnSetDownArrow(lpPoints, &Rect);
      SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight);

      /****************
      * Draw Up Arrow
      ****************/

      Rect.bottom = Rect.top + 1;
      Rect.top = 0;

      dHighlight = SpinBtnSetUpArrow(lpPoints, &Rect);
      SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight);
      }

   else
      {
      if (SPINBTN_ISARROW(Style, SBNS_UPARROW))
         dHighlight = SpinBtnSetUpArrow(lpPoints, &Rect);
      else
         dHighlight = SpinBtnSetDownArrow(lpPoints, &Rect);

      SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight);
      }
   }

else                                   // Horizontal
   {
   if (SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
      {
      Rect.left += (Rect.right - Rect.left) / 2;

      /*******************
      * Draw Right Arrow
      *******************/

      dHighlight = SpinBtnSetRightArrow(lpPoints, &Rect);
      SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight);

      /******************
      * Draw Left Arrow
      ******************/

      Rect.right = Rect.left + 1;
      Rect.left = 0;

      dHighlight = SpinBtnSetLeftArrow(lpPoints, &Rect);
      SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight);
      }

   else
      {
      if (SPINBTN_ISARROW(Style, SBNS_UPARROW))
         dHighlight = SpinBtnSetLeftArrow(lpPoints, &Rect);
      else
         dHighlight = SpinBtnSetRightArrow(lpPoints, &Rect);

      SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight);
      }
   }
#ifdef SS_V80
}
#endif
return (TRUE);
}


short SpinBtnSetUpArrow(lpPoints, Rect)

LPPOINT lpPoints;
LPRECT  Rect;
{
RECT RectArrow;

Spin_GetUpDnArrowRect(&RectArrow, Rect);

lpPoints[0].x = RectArrow.left;
lpPoints[0].y = RectArrow.bottom - 1;

lpPoints[1].x = RectArrow.left + ((RectArrow.right - RectArrow.left) / 2);
lpPoints[1].y = RectArrow.top;

lpPoints[2].x = RectArrow.right - 1;
lpPoints[2].y = RectArrow.bottom - 1;

lpPoints[3].x = RectArrow.left;
lpPoints[3].y = RectArrow.bottom - 1;

lpPoints[4].x = lpPoints[1].x;
lpPoints[4].y = lpPoints[1].y + ((lpPoints[0].y - lpPoints[1].y) / 2);

return (2);
}


short SpinBtnSetDownArrow(lpPoints, Rect)

LPPOINT lpPoints;
LPRECT  Rect;
{
RECT RectArrow;

Spin_GetUpDnArrowRect(&RectArrow, Rect);

lpPoints[0].x = RectArrow.left;
lpPoints[0].y = RectArrow.top;

lpPoints[1].x = RectArrow.right - 1;
lpPoints[1].y = RectArrow.top;

lpPoints[2].x = RectArrow.left + ((RectArrow.right - RectArrow.left) / 2);
lpPoints[2].y = RectArrow.bottom - 1;

lpPoints[3].x = RectArrow.left;
lpPoints[3].y = RectArrow.top;

lpPoints[4].x = lpPoints[2].x;
lpPoints[4].y = lpPoints[1].y + ((lpPoints[2].y - lpPoints[1].y) / 2);

return (1);
}


short SpinBtnSetRightArrow(lpPoints, Rect)

LPPOINT lpPoints;
LPRECT  Rect;
{
RECT RectArrow;

Spin_GetLtRtArrowRect(&RectArrow, Rect);

lpPoints[0].x = RectArrow.left;
lpPoints[0].y = RectArrow.top;

lpPoints[1].x = RectArrow.right - 1;
lpPoints[1].y = RectArrow.top + ((RectArrow.bottom - RectArrow.top) / 2);

lpPoints[2].x = RectArrow.left;
lpPoints[2].y = RectArrow.bottom - 1;

lpPoints[3].x = RectArrow.left;
lpPoints[3].y = RectArrow.top;

lpPoints[4].x = lpPoints[0].x + ((lpPoints[1].x - lpPoints[0].x) / 2);
lpPoints[4].y = lpPoints[1].y;

return (1);
}


short SpinBtnSetLeftArrow(lpPoints, Rect)

LPPOINT lpPoints;
LPRECT  Rect;
{
RECT RectArrow;

Spin_GetLtRtArrowRect(&RectArrow, Rect);

lpPoints[0].x = RectArrow.right - 1;
lpPoints[0].y = RectArrow.top;

lpPoints[1].x = RectArrow.right - 1;
lpPoints[1].y = RectArrow.bottom - 1;

lpPoints[2].x = RectArrow.left;
lpPoints[2].y = RectArrow.top + ((RectArrow.bottom - RectArrow.top) / 2);

lpPoints[3].x = RectArrow.right - 1;
lpPoints[3].y = RectArrow.top;

lpPoints[4].x = lpPoints[2].x + ((lpPoints[1].x - lpPoints[2].x) / 2);
lpPoints[4].y = lpPoints[2].y;

return (0);
}


long SpinBtnGetStyle(hWnd)

HWND hWnd;
{
long Style;

Style = GetWindowLong(hWnd, GWL_STYLE);

if ((Style & SBNS_HORIZONTAL) == 0)
   Style |= SBNS_VERTICAL;

if ((SPINBTN_ISARROW(Style, SBNS_UPARROW)) == 0 &&
    (SPINBTN_ISARROW(Style, SBNS_DOWNARROW)) == 0)
   Style |= SBNS_UPDOWNARROW;

return (Style);
}


short SpinBtnGetScrollArea(RectClient, RectTemp, Style, lParam)

LPRECT RectClient;
LPRECT RectTemp;
long   Style;
LPARAM lParam;
{
short  x;
short  y;

x = LOWORD(lParam);
y = HIWORD(lParam);

_fmemcpy(RectTemp, RectClient, sizeof(RECT));

RectTemp->top++;
RectTemp->bottom--;
RectTemp->left++;
RectTemp->right--;

if (Style & SBNS_VERTICAL)
   {
   if (SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
      {
      if (y <= RectTemp->bottom / 2)
         {
         RectTemp->bottom /= 2;
         return (SPINBTN_UP);
         }

      else
         {
         RectTemp->top = (RectTemp->bottom / 2) + 1;
         return (SPINBTN_DOWN);
         }
      }

   else if (SPINBTN_ISARROW(Style, SBNS_UPARROW))
      return (SPINBTN_UP);

   else if (SPINBTN_ISARROW(Style, SBNS_DOWNARROW))
      return (SPINBTN_DOWN);
   }

/*************
* Horizontal
*************/

else
   {
   if (SPINBTN_ISARROW(Style, SBNS_UPDOWNARROW))
      {
      if (x <= RectTemp->right / 2)
         {
         RectTemp->right /= 2;
         return (SPINBTN_LEFT);
         }

      else
         {
         RectTemp->left = (RectTemp->right / 2) + 1;
         return (SPINBTN_RIGHT);
         }
      }

   else if (SPINBTN_ISARROW(Style, SBNS_UPARROW))
      return (SPINBTN_LEFT);

   else if (SPINBTN_ISARROW(Style, SBNS_DOWNARROW))
      return (SPINBTN_RIGHT);
   }

return (0);
}


void SpinDrawArrow(hWnd, hDC, lpPoints, dHighlight)

HWND    hWnd;
HDC     hDC;
LPPOINT lpPoints;
short   dHighlight;
{
HPEN    hPen;
HPEN    hPenOld;
HPEN    hPenOldTemp;
HBRUSH  hBrush;
HBRUSH  hBrushOld;
short   i;

if (IsWindowEnabled(hWnd))
   {
   hBrush = CreateSolidBrush(RGBCOLOR_BLACK);
   hPen = CreatePen(PS_SOLID, 1, RGBCOLOR_BLACK);
   hPenOld = SelectObject(hDC, hPen);

   MoveToEx(hDC, lpPoints[0].x, lpPoints[0].y, NULL);

   for (i = 1; i < 4; i++)
      LineTo(hDC, lpPoints[i].x, lpPoints[i].y);

   hBrushOld = SelectObject(hDC, hBrush);
   FloodFill(hDC, lpPoints[4].x, lpPoints[4].y, RGBCOLOR_BLACK);
   SelectObject(hDC, hBrushOld);

   SelectObject(hDC, hPenOld);
   DeleteObject(hBrush);
   DeleteObject(hPen);
   }

else
   {
   hPen = CreatePen(PS_SOLID, 1, RGBCOLOR_DARKGRAY);
   hPenOld = SelectObject(hDC, hPen);

   MoveToEx(hDC, lpPoints[0].x, lpPoints[0].y, NULL);

   for (i = 1; i < 4; i++)
      {
      if (i == dHighlight + 1)
         hPenOldTemp = SelectObject(hDC, GetStockObject(WHITE_PEN));

      LineTo(hDC, lpPoints[i].x, lpPoints[i].y);

      if (i == dHighlight + 1)
         SelectObject(hDC, hPenOldTemp);
      }

   SelectObject(hDC, hPenOld);
   DeleteObject(hPen);
   }
}


void Spin_GetUpDnArrowRect(LPRECT lpRectArrow, LPRECT lpRect)
{
/*
short xOffset = SPINBTN_XOFFSET_UPDN(lpRect);
short yOffset = SPINBTN_YOFFSET_UPDN(lpRect);

*lpRectArrow = *lpRect;

lpRectArrow->left += xOffset;
lpRectArrow->top += yOffset;
lpRectArrow->right -= xOffset;
lpRectArrow->bottom -= yOffset;
*/
//short nWidth = lpRect->right - lpRect->left - 5;
//short nHeightMax = lpRect->bottom - lpRect->top - 5;
#if defined(_WIN64) || defined(_IA64)
long nWidth = (lpRect->right - lpRect->left) * 3 / 5;
long nHeightMax = (lpRect->bottom - lpRect->top) * 3 / 5;
long nHeight;
#else
short nWidth = (lpRect->right - lpRect->left) * 3 / 5;
short nHeightMax = (lpRect->bottom - lpRect->top) * 3 / 5;
short nHeight;
#endif

nWidth = (((nWidth - 1) / 2) * 2) + 1; // Ensure an odd number
nHeight = (nWidth / 2) + 1;

for (; nHeight > nHeightMax && nHeight > 2; nWidth -= 2, nHeight--)
	;

SetRect(lpRectArrow, 0, 0, nWidth, nHeight);
OffsetRect(lpRectArrow, ((lpRect->right - lpRect->left) - (lpRectArrow->right - lpRectArrow->left)) / 2,
           ((lpRect->bottom - lpRect->top) - (lpRectArrow->bottom - lpRectArrow->top)) / 2);
OffsetRect(lpRectArrow, lpRect->left, lpRect->top);
}


void Spin_GetLtRtArrowRect(LPRECT lpRectArrow, LPRECT lpRect)
{
/*
short xOffset = SPINBTN_XOFFSET(lpRect);
short yOffset = SPINBTN_YOFFSET(lpRect);

*lpRectArrow = *lpRect;

lpRectArrow->left += xOffset;
lpRectArrow->top += yOffset;
lpRectArrow->right -= xOffset;
lpRectArrow->bottom -= yOffset;
*/
//short nWidthMax = lpRect->right - lpRect->left - 5;
//short nHeight = lpRect->bottom - lpRect->top - 5;
#if defined(_WIN64) || defined(_IA64)
long nWidthMax = (lpRect->right - lpRect->left) * 3 / 5;
long nHeight = (lpRect->bottom - lpRect->top) * 3 / 5;
long nWidth;
#else
short nWidthMax = (lpRect->right - lpRect->left) * 3 / 5;
short nHeight = (lpRect->bottom - lpRect->top) * 3 / 5;
short nWidth;
#endif

nHeight = (((nHeight - 1) / 2) * 2) + 1; // Ensure an odd number
nWidth = (nHeight / 2) + 1;

for (; nWidth > nWidthMax && nWidth > 2; nHeight -= 2, nWidth--)
	;

SetRect(lpRectArrow, 0, 0, nWidth, nHeight);
OffsetRect(lpRectArrow, ((lpRect->right - lpRect->left) - (lpRectArrow->right - lpRectArrow->left)) / 2,
           ((lpRect->bottom - lpRect->top) - (lpRectArrow->bottom - lpRectArrow->top)) / 2);
OffsetRect(lpRectArrow, lpRect->left, lpRect->top);
}
