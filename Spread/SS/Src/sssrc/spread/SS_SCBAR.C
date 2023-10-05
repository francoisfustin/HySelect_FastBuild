/*********************************************************
* SS_SCBAR.C
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "spread.h"
#include "fphdc.h"
#include "ss_scbar.h"
#include "..\classes\wintools.h"
#ifdef SS_V80
#include "ss_draw.h"
#include "uxtheme.h"
#include "ss_theme.h"
#define V_TRIANGLE_WIDTH  9
#define V_TRIANGLE_HEIGHT  5
#define H_TRIANGLE_WIDTH  5
#define H_TRIANGLE_HEIGHT  9
#define THUMBGRIP_HEIGHT  7
#define THUMBGRIP_WIDTH  7
#endif

#define THUMBWIDTH(lpScrlBar)  (lpScrlBar->rectThumb.right - lpScrlBar->rectThumb.left)
#define THUMBHEIGHT(lpScrlBar) (lpScrlBar->rectThumb.bottom - lpScrlBar->rectThumb.top)
#define THUMBSIZE(lpScrlBar)   (lpScrlBar->fIsHorizontal ? THUMBWIDTH(lpScrlBar) : THUMBHEIGHT(lpScrlBar))

/**********************
* Function prototypes
**********************/

#ifndef SS_NOSCBAR

static LPSCRLBAR ScrlBarInit(HWND hWnd, BOOL isHorizontal);
LRESULT CALLBACK _export tbScrlBarWndFn(HWND hWnd, UINT Msg,
                                          WPARAM wParam, LPARAM lParam);
void             ScrlBarPaint(HWND hWnd, HDC hDC, LPRECT lpRectUpdate);

void             ScrlBarPaintItem(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar,
                                  LPRECT lpRect, LPRECT lpRectUpdate,
                                  LPSCRLBARITEM lpScrlBarItem,
                                  LPSCRLBARITEM lpScrlBarItem2, BOOL fBtnDown,
                                  BOOL fBtnDown2, LPINT lpdTop, LPINT lpdBottom);
void             ScrlBarPaintItem2(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar,
                                   LPRECT lpRect, BOOL fIsEnabled,
                                   BOOL fBtnDown, LPHBITMAP lphBitmap,
                                   LPTSTR lpBitmapName,
                                   LPHBITMAP lphBitmapDisabled,
                                   LPTSTR lpBitmapDisabledName);
int              ScrlBarGetButton(HWND hWnd, LPSCRLBAR lpScrlBar,
                                  LPRECT lpRectClient, LPRECT lpRectBtn,
                                  int yMouse);
int              ScrlBarGetBtn2(LPSCRLBAR lpScrlBar, LPRECT lpRectClient,
                                LPRECT lpRectBnt, LPINT lpdTop,
                                LPINT lpdBottom, int wTop, int wBottom,
                                int yMouse, int dHeight, long lStyle);
#ifdef SS_V80
void					ScrlBarGetThumbRect(LPSCRLBAR lpScrlBar, LPRECT lpRectClient, LPRECT lpRectThumb);
void             UpdatePositions(HWND hWnd);
long			 GetPositionFromPoint(HWND hWnd, long mouseLoc);
void             UpdateThumbPosition(HWND hWnd, long newPos);
void			 UpdatePageSize(HWND hWnd, long pageSize, BOOL bRedraw);
void			 ScrlBarPaintVisualStyleThumb(HWND hWnd, HDC hdc, LPRECT lpRect, BOOL fIsEnabled, BOOL fBtnDown, BOOL isHorizontal);
void             ScrlBarPaintThumb(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar, LPRECT lpRectClient);

void             ScrlBarPaintItemEnhanced(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar,
                                  LPRECT lpRect, LPRECT lpRectUpdate,
                                  BOOL fBtnDown, BOOL fBtnDown2, 
								  LPINT lpdTop, LPINT lpdBottom,
								  int type);
void		     ScrlBarPaintItemEnhanced2(HWND hWnd, HDC hDC, LPRECT lpRect, BOOL fIsEnabled, BOOL fBtnDown, int type, BOOL fHorizontal, BOOL fThumb);
void             ScrlBarPaintItemVisualStyles(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar,
                                  LPRECT lpRect, LPRECT lpRectUpdate,
                                  BOOL fBtnDown, BOOL fBtnDown2, 
								  LPINT lpdTop, LPINT lpdBottom,
								  int type);
void		     ScrlBarPaintItemVisualStyles2(HWND hWnd, HDC hDC, LPRECT lpRect, BOOL fIsEnabled, BOOL fBtnDown, int type, BOOL fIsHorizontal);
void             ScrlBarPaintTrackVisualStyles(HWND hWnd, HDC hDC, LPRECT lpRect);
#endif
static void      ScrlBarSetPtr(HWND hWnd, GLOBALHANDLE hGlobal);
static LPSCRLBAR ScrlBarLock(HWND hWnd);
static void      ScrlBarUnlock(HWND hWnd);
static void      ScrlBarFree(HWND hWnd);

extern HANDLE hDynamicInst;


BOOL RegisterScrlBar(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS |
                   CS_CLASSDC;
wc.lpfnWndProc   = tbScrlBarWndFn;
wc.cbClsExtra    = 0;
#if defined(_WIN64) || defined(_IA64)
wc.cbWndExtra    = 8;
#else
wc.cbWndExtra    = 4;
#endif
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = SSClassNames[dClassNameIndex].TBScrlBar;
if (RegisterClass(&wc))
   bRet = TRUE;
return bRet;
}


BOOL UnRegisterScrlBar(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].TBScrlBar, hInstance));
}


static LPSCRLBAR ScrlBarInit(HWND hWnd, BOOL isHorizontal)
{
GLOBALHANDLE hGlobal;
LPSCRLBAR    lpScrlBar;

if (!(hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                            (long)sizeof(SCRLBAR))))
   return (NULL);

ScrlBarSetPtr(hWnd, hGlobal);

lpScrlBar = (LPSCRLBAR)GlobalLock(hGlobal);

#ifdef SS_V21
lpScrlBar->fIs95 = FALSE;
#else
lpScrlBar->fIs95 = SS_IsWindowsV95();
#endif

#ifdef SS_V80
lpScrlBar->lCurPos = 1;
lpScrlBar->fTrackMouse = TRUE;
lpScrlBar->fMouseIsDown = FALSE;
lpScrlBar->fIsHorizontal = isHorizontal;
#endif


lpScrlBar->dBtnDown = -1;

#ifdef SS_V80
if (isHorizontal)
{
lpScrlBar->dWidth = GetSystemMetrics(SM_CYHSCROLL) - lpScrlBar->nStyle;
if (lpScrlBar->fIs95)
   lpScrlBar->dWidth++;
lpScrlBar->ItemUp.lpBmpName = SBR_LF;
lpScrlBar->ItemUp.lpBmpNameD = SBR_LFD;
lpScrlBar->ItemDn.lpBmpName = SBR_RT;
lpScrlBar->ItemDn.lpBmpNameD = SBR_RTD;
lpScrlBar->ItemPageUp.lpBmpName = SBR_PAGELF;
lpScrlBar->ItemPageUp.lpBmpNameD = SBR_PAGELFD;
lpScrlBar->ItemPageDn.lpBmpName = SBR_PAGERT;
lpScrlBar->ItemPageDn.lpBmpNameD = SBR_PAGERTD;
lpScrlBar->ItemHome.lpBmpName = SBR_HOMEH;
lpScrlBar->ItemHome.lpBmpNameD = SBR_HOMEHD;
lpScrlBar->ItemEnd.lpBmpName = SBR_ENDH;
lpScrlBar->ItemEnd.lpBmpNameD = SBR_ENDHD;
}
else
#endif
{
#if SS_V80
lpScrlBar->dHeight = GetSystemMetrics(SM_CYVSCROLL) - lpScrlBar->nStyle;
#else
lpScrlBar->dHeight = GetSystemMetrics(SM_CYVSCROLL) - 2;
#endif

if (lpScrlBar->fIs95)
   lpScrlBar->dHeight++;
lpScrlBar->ItemUp.lpBmpName = SBR_UP;
lpScrlBar->ItemUp.lpBmpNameD = SBR_UPD;
lpScrlBar->ItemDn.lpBmpName = SBR_DN;
lpScrlBar->ItemDn.lpBmpNameD = SBR_DND;
lpScrlBar->ItemPageUp.lpBmpName = SBR_PAGEUP;
lpScrlBar->ItemPageUp.lpBmpNameD = SBR_PAGEUPD;
lpScrlBar->ItemPageDn.lpBmpName = SBR_PAGEDN;
lpScrlBar->ItemPageDn.lpBmpNameD = SBR_PAGEDND;
lpScrlBar->ItemHome.lpBmpName = SBR_HOME;
lpScrlBar->ItemHome.lpBmpNameD = SBR_HOMED;
lpScrlBar->ItemEnd.lpBmpName = SBR_END;
lpScrlBar->ItemEnd.lpBmpNameD = SBR_ENDD;
}
return (lpScrlBar);
}


LRESULT CALLBACK _export tbScrlBarWndFn(hWnd, Msg, wParam, lParam)

HWND         hWnd;
UINT         Msg;
WPARAM       wParam;
LPARAM       lParam;
{
LPSCRLBAR    lpScrlBar;
PAINTSTRUCT  Paint;
HBITMAP      hBitmapOld;
HBITMAP      hBitmapDC;
HDC          hDCMemory;
HDC          hDC;
HDC          hDCOrig;
RECT         Rect;
RECT         RectBtn;
RECT         RectClient;
POINT        ptCurPos;
DWORD        dwTime;
short        dRet;
BOOL         fCancel = FALSE;

switch (Msg)
   {
   /********************
   * Create new window
   ********************/

   case WM_CREATE:
	   {
	  LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
      lpScrlBar = ScrlBarInit(hWnd, *(LPBOOL)lpCS->lpCreateParams);

      ScrlBarUnlock(hWnd);
      return (0);
	   }
#ifdef SS_V80
   case WM_SIZE:
	   {
		UpdatePositions(hWnd);
	   }
	   break;
   case SBR_SETPAGESIZE:
	   {
		lpScrlBar = ScrlBarLock(hWnd);
		if (lpScrlBar->lCntPage != (long)wParam)
			UpdatePageSize(hWnd, (long)wParam, (BOOL)lParam);
        ScrlBarUnlock(hWnd);
	   }
	   break;
   case SBM_SETRANGE:
	   lpScrlBar = ScrlBarLock(hWnd);
	   if (lpScrlBar->lLastPos != (long)lParam || lpScrlBar->lFirstPos != (long)wParam)
	   {
		lpScrlBar->lLastPos = (long)lParam;
		lpScrlBar->lFirstPos = (long)wParam;
		InvalidateRect(hWnd, NULL, TRUE);
	   }
	   ScrlBarUnlock(hWnd);
				   
	   break;
   case SBM_SETPOS:
	   {
	   lpScrlBar = ScrlBarLock(hWnd);
	   if (!(BOOL)lParam)
		UpdatePositions(hWnd);
	   //if (lpScrlBar->lCurPos != (long)wParam)
	   //{
		   UpdateThumbPosition(hWnd, (long)wParam);
	   //}
	   ScrlBarUnlock(hWnd);
	   }
	   break;

   case SBR_SETSTYLE:
	   lpScrlBar = ScrlBarLock(hWnd);
	   if (lpScrlBar->nStyle != (short)wParam)
	   {
		lpScrlBar->nStyle = (short)wParam;
		lpScrlBar->dWidth = GetSystemMetrics(SM_CYHSCROLL) - lpScrlBar->nStyle;
		if (lpScrlBar->fIs95)
			lpScrlBar->dWidth++;
		lpScrlBar->dHeight = GetSystemMetrics(SM_CYVSCROLL) - lpScrlBar->nStyle;
		if (lpScrlBar->fIs95)
			lpScrlBar->dHeight++;

		InvalidateRect(hWnd, NULL, TRUE);
	   }
	   ScrlBarUnlock(hWnd);
	   break;
   case SBR_TRACKMOUSE:
	   lpScrlBar = ScrlBarLock(hWnd);
	   lpScrlBar->fTrackMouse = (BOOL)wParam;
	   ScrlBarUnlock(hWnd);
	   break;

   case WM_MOUSELEAVE:
       lpScrlBar = ScrlBarLock(hWnd);
	   SetRectEmpty(&(lpScrlBar->mouseBtnRect));
       ScrlBarUnlock(hWnd);
	   InvalidateRect(hWnd, NULL, FALSE);
	   break;

   case WM_MOUSEMOVE:
	   {
	   TRACKMOUSEEVENT tme;
	   long mouseLoc;	   
       lpScrlBar = ScrlBarLock(hWnd);
	   if (!lpScrlBar->fMouseIsDown)
	   {
		   if (IsRectEmpty(&(lpScrlBar->mouseBtnRect)) && lpScrlBar->nStyle != 0)
		   {
			 tme.cbSize = sizeof(TRACKMOUSEEVENT);
			 tme.dwFlags = TME_LEAVE;
			 tme.hwndTrack = hWnd;
			 tme.dwHoverTime = HOVER_DEFAULT;
			 _TrackMouseEvent(&tme);

		   }
			if (lpScrlBar->fIsHorizontal)
				mouseLoc = LOWORD(lParam);
			else
				mouseLoc = HIWORD(lParam);
			GetClientRect(hWnd, &RectClient);
			dRet = ScrlBarGetButton(hWnd, lpScrlBar, &RectClient, &RectBtn,
									   mouseLoc);
        
			if (!EqualRect(&RectBtn, &(lpScrlBar->mouseBtnRect)))
			{
				RECT tempRect = lpScrlBar->mouseBtnRect;
				lpScrlBar->mouseBtnRect = RectBtn;
				if (!IsRectEmpty(&tempRect))
					InvalidateRect(hWnd, &tempRect, FALSE);
				InvalidateRect(hWnd, &RectBtn, FALSE);			
			}
	   }
		ScrlBarUnlock(hWnd);
		

	   }
	   break;
#endif
   case WM_LBUTTONDOWN:
   case WM_LBUTTONDBLCLK:
		{
		MSG MsgTemp;
		long mouseLoc;

		lpScrlBar = ScrlBarLock(hWnd);

      ptCurPos.x = LOWORD(lParam);
      ptCurPos.y = HIWORD(lParam);

      GetClientRect(hWnd, &RectClient);

      if (!lpScrlBar->fIs95)
         InflateRect(&RectClient, -1, -1);

		if (lpScrlBar->fIsHorizontal)
			mouseLoc = ptCurPos.x;
		else
			mouseLoc = ptCurPos.y;

      if ((dRet = ScrlBarGetButton(hWnd, lpScrlBar, &RectClient, &RectBtn, mouseLoc)) == -1)
         return (0);

      SetCapture(hWnd);

      if (PtInRect(&RectBtn, ptCurPos))
         {
         if (lpScrlBar->dBtnDown == -1)
            {
            lpScrlBar->dBtnDown = dRet;
            InvalidateRect(hWnd, &RectBtn, FALSE);
            UpdateWindow(hWnd);
            }

#ifdef SS_V80
			if (dRet != 4 && dRet != 5)
				{
				if (lpScrlBar->fIsHorizontal)
					FORWARD_WM_HSCROLL(hWnd, hWnd, dRet, 0, tbSendMessageToParent);
				else if (!lpScrlBar->fIsHorizontal && dRet != 4 && dRet != 5)
					FORWARD_WM_VSCROLL(hWnd, hWnd, dRet, 0, tbSendMessageToParent);
				}
#else
         FORWARD_WM_VSCROLL(hWnd, hWnd, dRet, 0, tbSendMessageToParent);
#endif
         }

      dwTime = GetTickCount();

#ifdef SS_V80
		if (dRet != 4 && dRet != 5)
#endif // SS_V80
			while (dwTime + SCRLBAR_INITIALDELAY > GetTickCount())
				{
				if (GetSystemMetrics(SM_SWAPBUTTON))
					{
					if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
						{
						fCancel = TRUE;
						break;
						}
					}
				else
					if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
						{
						fCancel = TRUE;
						break;
						}
				}

#ifdef SS_V80
		lpScrlBar->ptOffset = max(0, mouseLoc - lpScrlBar->ptCur);
#endif

		for (; !fCancel; )
         {
#ifdef SS_V80
			int dRetTemp;
			lpScrlBar->fMouseIsDown = TRUE;
#endif

#ifdef SS_V80
         if (dwTime + SCRLBAR_TIMEDELAY <= GetTickCount() || (dRet == 4 || dRet == 5))
#else
         if (dwTime + SCRLBAR_TIMEDELAY <= GetTickCount())
#endif
				{
#ifdef SS_V80
            if (PtInRect(&RectBtn, ptCurPos) || (dRet == 4 || dRet == 5))
#else
            if (PtInRect(&RectBtn, ptCurPos))
#endif
               {

               if (lpScrlBar->dBtnDown == -1)
                  {
                  lpScrlBar->dBtnDown = dRet;
                  InvalidateRect(hWnd, &RectBtn, FALSE);
                  UpdateWindow(hWnd);
                  }

 #ifdef SS_V80
					if (dRet == SB_THUMBPOSITION || dRet == SB_THUMBTRACK)
						{
						long lCurPosOld = lpScrlBar->lCurPos;
						dRet = SB_THUMBTRACK;
						lpScrlBar->lCurPos = GetPositionFromPoint(hWnd, mouseLoc);
						if (lCurPosOld == lpScrlBar->lCurPos)
							InvalidateRect(hWnd, NULL, FALSE);
						}

					if (lpScrlBar->fIsHorizontal)
						FORWARD_WM_HSCROLL(hWnd, hWnd, dRet, lpScrlBar->lCurPos, tbSendMessageToParent);
					else
						FORWARD_WM_VSCROLL(hWnd, hWnd, dRet, lpScrlBar->lCurPos, tbSendMessageToParent);

					if (!lpScrlBar->fTrackMouse)
						UpdateThumbPosition(hWnd, lpScrlBar->lCurPos);
#else
					FORWARD_WM_VSCROLL(hWnd, hWnd, dRet, 0, tbSendMessageToParent);
#endif
					}

            else
               {
               if (lpScrlBar->dBtnDown != -1)
                  {
                  lpScrlBar->dBtnDown = -1;
                  InvalidateRect(hWnd, &RectBtn, FALSE);
                  UpdateWindow(hWnd);
                  }
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

         GetCursorPos(&ptCurPos);
         ScreenToClient(hWnd, &ptCurPos);

#ifdef SS_V80
			if (lpScrlBar->fIsHorizontal)
				mouseLoc = ptCurPos.x;
			else
				mouseLoc = ptCurPos.y;

			dRetTemp = ScrlBarGetButton(hWnd, lpScrlBar, &RectClient, &RectBtn, mouseLoc);

			if (dRetTemp == SB_THUMBPOSITION || dRetTemp == SB_THUMBTRACK)
				dRetTemp = SB_THUMBTRACK;

			/* RFW - 11/14/08
			if (dRet != dRetTemp && dRetTemp == SB_THUMBTRACK)
			*/
			if (dRet != dRetTemp && (dRet == SB_PAGEUP || dRet == SB_PAGEDOWN))
				fCancel = TRUE;
#endif

			// RFW - 12/4/02 - 11325
			while (PeekMessage(&MsgTemp, NULL, 0, 0, PM_REMOVE))
				{
				TranslateMessage(&MsgTemp);
				DispatchMessage(&MsgTemp);
				}
			}

		ReleaseCapture();
#ifdef SS_V80
		UpdateThumbPosition(hWnd, lpScrlBar->lCurPos);
#endif


#ifdef SS_V80
		lpScrlBar->fMouseIsDown = FALSE;

		if (dRet == 5)
			{
			if (lpScrlBar->fIsHorizontal)
				FORWARD_WM_HSCROLL(hWnd, hWnd, SB_THUMBPOSITION, lpScrlBar->lCurPos, tbSendMessageToParent);
			else
				FORWARD_WM_VSCROLL(hWnd, hWnd, SB_THUMBPOSITION, lpScrlBar->lCurPos, tbSendMessageToParent);
			}

		InvalidateRect(hWnd, &(lpScrlBar->rectThumb), TRUE);	
#endif
      lpScrlBar->dBtnDown = -1;
      InvalidateRect(hWnd, &RectBtn, TRUE);
      UpdateWindow(hWnd);
      ScrlBarUnlock(hWnd);
		}
      return (0);

   case WM_CLOSE:
      DestroyWindow(hWnd);
      return (0);

   /*********************************************************
   * This message is received when the window is destroyed,
   * either under program control or external direction
   *********************************************************/

   case WM_DESTROY:
      lpScrlBar = ScrlBarLock(hWnd);

      if (lpScrlBar->ItemUp.hBmp)
         DeleteObject(lpScrlBar->ItemUp.hBmp);

      if (lpScrlBar->ItemUp.hBmpD)
         DeleteObject(lpScrlBar->ItemUp.hBmpD);

      if (lpScrlBar->ItemDn.hBmp)
         DeleteObject(lpScrlBar->ItemDn.hBmp);

      if (lpScrlBar->ItemDn.hBmpD)
         DeleteObject(lpScrlBar->ItemDn.hBmpD);

      if (lpScrlBar->ItemPageUp.hBmp)
         DeleteObject(lpScrlBar->ItemPageUp.hBmp);

      if (lpScrlBar->ItemPageUp.hBmpD)
         DeleteObject(lpScrlBar->ItemPageUp.hBmpD);

      if (lpScrlBar->ItemPageDn.hBmp)
         DeleteObject(lpScrlBar->ItemPageDn.hBmp);

      if (lpScrlBar->ItemPageDn.hBmpD)
         DeleteObject(lpScrlBar->ItemPageDn.hBmpD);

      if (lpScrlBar->ItemHome.hBmp)
         DeleteObject(lpScrlBar->ItemHome.hBmp);

      if (lpScrlBar->ItemHome.hBmpD)
         DeleteObject(lpScrlBar->ItemHome.hBmpD);

      if (lpScrlBar->ItemEnd.hBmp)
         DeleteObject(lpScrlBar->ItemEnd.hBmp);

      if (lpScrlBar->ItemEnd.hBmpD)
         DeleteObject(lpScrlBar->ItemEnd.hBmpD);

      ScrlBarUnlock(hWnd);
      ScrlBarFree(hWnd);
      return (0);

   case WM_ERASEBKGND:
      return (TRUE);

#if (WINVER >= 0x0400)
   // New to Windows 95
   case WM_PRINT:
		{
         HDC hdc = (HDC)wParam; 
         ULONG uFlags = (ULONG)lParam; 

         if (uFlags & (ULONG)PRF_CLIENT && (uFlags & ~(ULONG)PRF_CHECKVISIBLE || IsWindowVisible(hWnd)))
         {
            RECT rc;

            GetClientRect(hWnd, &rc);
            ScrlBarPaint(hWnd, hdc, &rc);
         }
      }
      break;
#endif

   case WM_PAINT:
      GetClientRect(hWnd, &Rect);
      hDC = hDCOrig = (HDC)wParam;
      if (hDCOrig)
         CopyRect(&Paint.rcPaint, &Rect);
      else
         hDC = fpBeginPaint(hWnd, &Paint);

      /*********************************
      * Create a memory device context
      *********************************/

      hBitmapDC = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,
                                         Rect.bottom - Rect.top);

      hDCMemory = CreateCompatibleDC(hDC);
      hBitmapOld = SelectObject(hDCMemory, hBitmapDC);

      ScrlBarPaint(hWnd, hDCMemory, &Paint.rcPaint);

      /*******************************************************
      * Copy the memory device context bitmap to the display
      *******************************************************/

      BitBlt(hDC, Paint.rcPaint.left, Paint.rcPaint.top, Paint.rcPaint.right -
             Paint.rcPaint.left, Paint.rcPaint.bottom - Paint.rcPaint.top,
             hDCMemory, Paint.rcPaint.left, Paint.rcPaint.top, SRCCOPY);

      /***********
      * Clean up
      ***********/

      SelectObject(hDCMemory, hBitmapOld);
      DeleteDC(hDCMemory);
      DeleteObject(hBitmapDC);

      if (!hDCOrig)
         EndPaint(hWnd, &Paint);
      return (0);
   }

return (DefWindowProc(hWnd, Msg, wParam, lParam));
}

#ifdef SS_V80

void ScrlBarGetThumbRect(LPSCRLBAR lpScrlBar, LPRECT lpRectClient, LPRECT lpRectThumb)
{
	double factor;
	long lThumbSize;

	if (lpScrlBar->lLastPos == 0)
		factor = 1;
	else
		factor = (double)lpScrlBar->lCntPage / (double)lpScrlBar->lLastPos;

	if (lpScrlBar->fIsHorizontal)
	{		
		long width = lpRectClient->right - lpRectClient->left - (2*lpScrlBar->dWidth);
		lThumbSize = max((long)((double)width * factor), THUMBGRIP_WIDTH);
		lpRectThumb->top = lpRectClient->top;
		lpRectThumb->bottom = lpRectClient->bottom;
		lpRectThumb->left = max(lpScrlBar->ptCur, lpScrlBar->ptStart);
		lpRectThumb->right = lpRectThumb->left + lThumbSize;
	}
	else
	{
		long height = lpRectClient->bottom - lpRectClient->top - (2*lpScrlBar->dHeight);
		lThumbSize = max((long)((double)height * factor), THUMBGRIP_HEIGHT);
		lpRectThumb->left = lpRectClient->left;
		lpRectThumb->right = lpRectClient->right;
		lpRectThumb->top = max(lpScrlBar->ptCur, lpScrlBar->ptStart);
		lpRectThumb->bottom = lpRectThumb->top + lThumbSize;
	}
}


void ScrlBarPaintThumb(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar, LPRECT lpRectClient)
{
	 RECT RectTemp;
 	 int i = 0;
    short nStyle = lpScrlBar->nStyle;
    LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);

    if( nStyle == 1 )
    {
       HTHEME hTheme = SS_OpenThemeData(hWnd, L"ScrollBar");
       if( hTheme )
          SS_CloseThemeData(hTheme);
       else
          nStyle = 0;
    }

	if (lStyle & SBRS_HOMEEND)
		i++;
	if (lStyle & SBRS_PAGEUPDN)
		i++;
	if (lStyle & SBRS_UPDN)
		i++;

	ScrlBarGetThumbRect(lpScrlBar, lpRectClient, &RectTemp);
	lpScrlBar->rectThumb = RectTemp;

   switch (nStyle)
	{
	case 0:
		ScrlBarPaintItem2(hWnd, hDC, lpScrlBar, &RectTemp, IsWindowEnabled(hWnd),
                     lpScrlBar->dBtnDown == SB_THUMBPOSITION, NULL, NULL, NULL, NULL);
		break;
	case 1:
        ScrlBarPaintVisualStyleThumb(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd), lpScrlBar->dBtnDown == SB_THUMBPOSITION, lpScrlBar->fIsHorizontal);
		break;
	case 2: 
		ScrlBarPaintItemEnhanced2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
			lpScrlBar->dBtnDown == SB_THUMBPOSITION, lpScrlBar->dBtnDown == SB_THUMBPOSITION, lpScrlBar->fIsHorizontal, TRUE);
		break;
	}

}

long GetPositionFromPoint(HWND hWnd, long mouseLoc)
{
   LPSCRLBAR lpScrlBar = ScrlBarLock(hWnd);
   long      newPos = -1;
   double    dVal;
   
   if (lpScrlBar->lLastPos == 0)
	   return 0;

	dVal = ((double)lpScrlBar->trackSize/(double)lpScrlBar->lLastPos);

	mouseLoc = mouseLoc - lpScrlBar->ptOffset;
	if (mouseLoc < lpScrlBar->ptBase)
		newPos = 1;
	else
	{
		double tmpD = (mouseLoc - lpScrlBar->ptBase) / dVal;
//		newPos = (long)ceil(tmpD) - 1;
		newPos = (long)tmpD + 1;
	}

	mouseLoc = max(mouseLoc, lpScrlBar->ptBase);
	lpScrlBar->ptCur = min(mouseLoc, lpScrlBar->ptLast - THUMBSIZE(lpScrlBar));

   ScrlBarUnlock(hWnd);
   return (long)min(newPos, (long)SS_SBMAX);
}

void UpdatePositions(HWND hWnd)
{
	RECT RectClient;
	LONG lStyle;
	int i = 0;

    LPSCRLBAR lpScrlBar = ScrlBarLock(hWnd);
	GetClientRect(hWnd, &RectClient);
	lStyle = GetWindowLong(hWnd, GWL_STYLE);

	if (lStyle & SBRS_HOMEEND)
		i++;
	if (lStyle & SBRS_PAGEUPDN)
		i++;
	if (lStyle & SBRS_UPDN)
		i++;


	if (lpScrlBar->fIsHorizontal)
		{
		lpScrlBar->ptBase = RectClient.left + (lpScrlBar->dWidth*i);
		lpScrlBar->ptCur = RectClient.left + (lpScrlBar->dWidth*i);
		lpScrlBar->trackSize = RectClient.right - RectClient.left - (lpScrlBar->dWidth*(2*i));
		lpScrlBar->ptLast = RectClient.right - (lpScrlBar->dWidth*i);
		lpScrlBar->ptStart = lpScrlBar->ptCur;
		}
	else
		{
		lpScrlBar->ptBase = RectClient.top + (lpScrlBar->dHeight*i);
		lpScrlBar->ptCur = RectClient.top + (lpScrlBar->dHeight*i);
		lpScrlBar->trackSize = RectClient.bottom - RectClient.top - (lpScrlBar->dHeight*(2*i));
		lpScrlBar->ptLast = RectClient.bottom - (lpScrlBar->dHeight*i);
		lpScrlBar->ptStart = lpScrlBar->ptCur;
		}
	ScrlBarUnlock(hWnd);

}

void UpdateThumbPosition(HWND hWnd, long newPos)
{
	LPSCRLBAR lpScrlBar = ScrlBarLock(hWnd);
	RECT      RectClient;
	RECT      RectThumb;
	double    dVal;

	GetClientRect(hWnd, &RectClient);
  	ScrlBarGetThumbRect(lpScrlBar, &RectClient, &RectThumb);
	InvalidateRect(hWnd, &RectThumb, FALSE);

   if (lpScrlBar->lLastPos == 0)
	   return;

	dVal = ((double)lpScrlBar->trackSize/(double)lpScrlBar->lLastPos);

   {	 
	 int thumbHeight = THUMBSIZE(lpScrlBar);
	 int pos = (newPos-1) - (lpScrlBar->lFirstPos-1);
	 int pt = lpScrlBar->ptBase + (int)(max(0, ceil(dVal * pos)));

	 if (pt > lpScrlBar->ptLast - thumbHeight || newPos == lpScrlBar->lLastPos - lpScrlBar->lCntPage + 1)
		 pt = lpScrlBar->ptLast - thumbHeight;

	 lpScrlBar->ptCur = pt;
   }

   lpScrlBar->lCurPos = newPos;

  	ScrlBarGetThumbRect(lpScrlBar, &RectClient, &RectThumb);
   InvalidateRect(hWnd, &RectThumb, TRUE);
   ScrlBarUnlock(hWnd);
}

void UpdatePageSize(HWND hWnd, long pageSize, BOOL bRedraw)
{
   LPSCRLBAR lpScrlBar = ScrlBarLock(hWnd);
   lpScrlBar->lCntPage = pageSize;
   if (bRedraw)
	InvalidateRect(hWnd, NULL, TRUE);
   ScrlBarUnlock(hWnd);
}
#endif

void ScrlBarPaint(HWND hWnd, HDC hDC, LPRECT lpRectUpdate)
{
LPSCRLBAR lpScrlBar;
RECT      RectClient;
HPEN      hPenLine;
HPEN      hPenOld;
HBRUSH    hBrush;
HBRUSH    hBrushOld;
long      lStyle;
int       dTop = 0;
int       dBottom;
BOOL      fDeleteBrush = FALSE;
#if SS_V80
short     nStyle;
HTHEME    hTheme;
#endif

lpScrlBar = ScrlBarLock(hWnd);

#if SS_V80
nStyle = lpScrlBar->nStyle;
if( nStyle == 1)
{
	if (hTheme = SS_OpenThemeData(hWnd, L"ScrollBar") )
	{
	SS_CloseThemeData(hTheme);
	hTheme = NULL;
	}
	else
		nStyle = 0;
}
#endif

GetClientRect(hWnd, &RectClient);

/*******************
* Clear Background
*******************/


/*{
COLORREF  clrBack;

if (IsWindowEnabled(hWnd))
   clrBack = GetSysColor(COLOR_SCROLLBAR);
else
   clrBack = RGBCOLOR_WHITE;

hBrush = CreateSolidBrush(clrBack);
}*/


#ifdef SS_V80
  //if (lpScrlBar->nStyle == 2)
  if( nStyle == 2 )
  {
	 hBrush = CreateSolidBrush(lpScrlBar->clrTrack);
     fDeleteBrush = TRUE;
     hPenLine = CreatePen(PS_SOLID, 1, lpScrlBar->clrTrack);
  }
  else 
  {
#endif
 if (IsWindowEnabled(hWnd))
   {
#ifdef WIN32
   hBrush = (HBRUSH)SendMessage(GetParent(hWnd), WM_CTLCOLORSCROLLBAR,
                                (WPARAM)hDC, (LPARAM)hWnd);
#else
   hBrush = (HBRUSH)SendMessage(GetParent(hWnd), WM_CTLCOLOR, (WPARAM)hDC,
                                MAKELONG(hWnd, CTLCOLOR_SCROLLBAR));
#endif
   }
else
	{
   hBrush = CreateSolidBrush(RGBCOLOR_WHITE);
   fDeleteBrush = TRUE;
	}

if (lpScrlBar->fIs95)
   hPenLine = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
else
   hPenLine = CreatePen(PS_SOLID, 1, RGBCOLOR_BLACK);
#ifdef SS_V80
  }
#endif
hBrushOld = SelectObject(hDC, hBrush);
hPenOld = SelectObject(hDC, hPenLine);
#ifdef SS_V80
//if (lpScrlBar->nStyle == 1)
if( nStyle == 1 )
   ScrlBarPaintTrackVisualStyles(hWnd, hDC, &RectClient);
else
#endif
Rectangle(hDC, RectClient.top, RectClient.left, RectClient.right,
          RectClient.bottom);
SelectObject(hDC, hPenOld);
DeleteObject(hPenLine);
SelectObject(hDC, hBrushOld);

if (fDeleteBrush)
   DeleteObject(hBrush);

if (!lpScrlBar->fIs95)
   InflateRect(&RectClient, -1, -1);

#ifdef SS_V80
if (nStyle == 2)
{
	if (lpScrlBar->fIsHorizontal)
	{
		InflateRect(&RectClient, -1, 0);
		dTop = RectClient.left;
	}
	else
	{
	   InflateRect(&RectClient, 0, -1);	
	   dTop = RectClient.top;
	}
}
if (lpScrlBar->fIsHorizontal)
  dBottom = RectClient.right;
else
  dBottom = RectClient.bottom;
#else
dBottom = RectClient.bottom;
#endif
hPenLine = CreatePen(PS_SOLID, 1, RGBCOLOR_BLACK);
hPenOld = SelectObject(hDC, hPenLine);

lStyle = GetWindowLong(hWnd, GWL_STYLE);

#ifdef SS_V80
if (lStyle & SBRS_THUMB)
	ScrlBarPaintThumb(hWnd, hDC, lpScrlBar, &RectClient);
#endif
if (lStyle & SBRS_HOMEEND)
#ifdef SS_V80
  //if (lpScrlBar->nStyle == 2)
  if( nStyle == 2 )
   ScrlBarPaintItemEnhanced(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    lpScrlBar->dBtnDown == SB_TOP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_BOTTOM ? TRUE : FALSE,
                    &dTop, &dBottom, 1);
  //else if (lpScrlBar->nStyle == 1)
  else if (nStyle == 1)
   ScrlBarPaintItemVisualStyles(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    lpScrlBar->dBtnDown == SB_TOP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_BOTTOM ? TRUE : FALSE,
                    &dTop, &dBottom, 1);
else
#endif
   ScrlBarPaintItem(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    &lpScrlBar->ItemHome, &lpScrlBar->ItemEnd,
                    lpScrlBar->dBtnDown == SB_TOP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_BOTTOM ? TRUE : FALSE,
                    &dTop, &dBottom);

if (lStyle & SBRS_PAGEUPDN)
#ifdef SS_V80
  //if (lpScrlBar->nStyle == 2)
  if (nStyle == 2)
   ScrlBarPaintItemEnhanced(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    lpScrlBar->dBtnDown == SB_PAGEUP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_PAGEDOWN ? TRUE : FALSE,
                    &dTop, &dBottom, 2);
  //else if (lpScrlBar->nStyle == 1)
  else if (nStyle == 1)
   ScrlBarPaintItemVisualStyles(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    lpScrlBar->dBtnDown == SB_PAGEUP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_PAGEDOWN ? TRUE : FALSE,
                    &dTop, &dBottom, 2);
 else
#endif
   ScrlBarPaintItem(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    &lpScrlBar->ItemPageUp, &lpScrlBar->ItemPageDn,
                    lpScrlBar->dBtnDown == SB_PAGEUP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_PAGEDOWN ? TRUE : FALSE,
                    &dTop, &dBottom);

if (lStyle & SBRS_UPDN)
#ifdef SS_V80
  //if (lpScrlBar->nStyle == 2)
  if (nStyle == 2)
   ScrlBarPaintItemEnhanced(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    lpScrlBar->dBtnDown == SB_LINEUP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_LINEDOWN ? TRUE : FALSE,
                    &dTop, &dBottom, 3);
  //else if (lpScrlBar->nStyle == 1)
  else if (nStyle == 1)
   ScrlBarPaintItemVisualStyles(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    lpScrlBar->dBtnDown == SB_LINEUP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_LINEDOWN ? TRUE : FALSE,
                    &dTop, &dBottom, 3);
 else
#endif
   ScrlBarPaintItem(hWnd, hDC, lpScrlBar, &RectClient, lpRectUpdate,
                    &lpScrlBar->ItemUp, &lpScrlBar->ItemDn,
                    lpScrlBar->dBtnDown == SB_LINEUP ? TRUE : FALSE,
                    lpScrlBar->dBtnDown == SB_LINEDOWN ? TRUE : FALSE,
                    &dTop, &dBottom);

SelectObject(hDC, hPenOld);
DeleteObject(hPenLine);

ScrlBarUnlock(hWnd);
}


void ScrlBarPaintItem(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar, LPRECT lpRect,
                      LPRECT lpRectUpdate, LPSCRLBARITEM lpScrlBarItem,
                      LPSCRLBARITEM lpScrlBarItem2, BOOL fBtnDown,
                      BOOL fBtnDown2, LPINT lpdTop, LPINT lpdBottom)
{
RECT  RectTemp;
RECT  RectInt;
int   dTop;
int   dBottom;
#ifdef SS_V80
int dLeft;
int dRight;
#endif


#ifdef SS_V80
if (lpScrlBar->fIsHorizontal)
{
SetRect(&RectTemp, *lpdTop, lpRect->top, *lpdTop + lpScrlBar->dWidth, lpRect->bottom);

if (RectTemp.right > lpRect->left + ((lpRect->right - lpRect->left) / 2))
   RectTemp.right = lpRect->left + ((lpRect->right - lpRect->left) / 2);

dLeft = RectTemp.right;

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
   ScrlBarPaintItem2(hWnd, hDC, lpScrlBar, &RectTemp, IsWindowEnabled(hWnd),
                     fBtnDown, &lpScrlBarItem->hBmp,
                     lpScrlBarItem->lpBmpName, &lpScrlBarItem->hBmpD,
                     lpScrlBarItem->lpBmpNameD);
if (!lpScrlBar->fIs95)
   {
   if (dLeft >= lpRectUpdate->left && dLeft < lpRectUpdate->right)
      {
      MoveToEx(hDC, dLeft, lpRect->top, NULL);
      LineTo(hDC, dLeft, lpRect->bottom);
      }

   dLeft += 1;
   }

if (dLeft + lpScrlBar->dWidth <= lpRect->left +
    ((lpRect->right - lpRect->left) / 2))
   *lpdTop = dLeft;

dRight = *lpdBottom;
dRight -= lpScrlBar->dWidth;
dRight = max(dRight, lpRect->left + ((lpRect->right - lpRect->left) / 2));
SetRect(&RectTemp, dRight, lpRect->top, dRight +
        lpScrlBar->dWidth, lpRect->bottom);

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
{

   ScrlBarPaintItem2(hWnd, hDC, lpScrlBar, &RectTemp, IsWindowEnabled(hWnd),
                     fBtnDown2, &lpScrlBarItem2->hBmp,
                     lpScrlBarItem2->lpBmpName, &lpScrlBarItem2->hBmpD,
                     lpScrlBarItem2->lpBmpNameD);
}
if (!lpScrlBar->fIs95)
   {
   dRight -= 1;

   if (dRight >= lpRectUpdate->left && dRight < lpRectUpdate->right)
      {
      MoveToEx(hDC, dRight, lpRect->top, NULL);
      LineTo(hDC, dRight, lpRect->bottom);
      }
   }

if (dRight - lpScrlBar->dWidth >= lpRect->left +
    ((lpRect->right - lpRect->left) / 2))
   *lpdBottom = dRight;
}
else
{
#endif
SetRect(&RectTemp, lpRect->left, *lpdTop, lpRect->right, *lpdTop +
        lpScrlBar->dHeight);

if (RectTemp.bottom > lpRect->top + ((lpRect->bottom - lpRect->top) / 2))
   RectTemp.bottom = lpRect->top + ((lpRect->bottom - lpRect->top) / 2);

dTop = RectTemp.bottom;

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
   ScrlBarPaintItem2(hWnd, hDC, lpScrlBar, &RectTemp, IsWindowEnabled(hWnd),
                     fBtnDown, &lpScrlBarItem->hBmp,
                     lpScrlBarItem->lpBmpName, &lpScrlBarItem->hBmpD,
                     lpScrlBarItem->lpBmpNameD);
if (!lpScrlBar->fIs95)
   {
   if (dTop >= lpRectUpdate->top && dTop < lpRectUpdate->bottom)
      {
      MoveToEx(hDC, lpRect->left, dTop, NULL);
      LineTo(hDC, lpRect->right, dTop);
      }

   dTop += 1;
   }

if (dTop + lpScrlBar->dHeight <= lpRect->top +
    ((lpRect->bottom - lpRect->top) / 2))
   *lpdTop = dTop;

dBottom = *lpdBottom;
dBottom -= lpScrlBar->dHeight;
dBottom = max(dBottom, lpRect->top + ((lpRect->bottom - lpRect->top) / 2));
SetRect(&RectTemp, lpRect->left, dBottom, lpRect->right, dBottom +
        lpScrlBar->dHeight);

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
{

   ScrlBarPaintItem2(hWnd, hDC, lpScrlBar, &RectTemp, IsWindowEnabled(hWnd),
                     fBtnDown2, &lpScrlBarItem2->hBmp,
                     lpScrlBarItem2->lpBmpName, &lpScrlBarItem2->hBmpD,
                     lpScrlBarItem2->lpBmpNameD);
}
if (!lpScrlBar->fIs95)
   {
   dBottom -= 1;

   if (dBottom >= lpRectUpdate->top && dBottom < lpRectUpdate->bottom)
      {
      MoveToEx(hDC, lpRect->left, dBottom, NULL);
      LineTo(hDC, lpRect->right, dBottom);
      }
   }

if (dBottom - lpScrlBar->dHeight >= lpRect->top +
    ((lpRect->bottom - lpRect->top) / 2))
   *lpdBottom = dBottom;
#ifdef SS_V80
}
#endif
}

#ifdef SS_V80
void ScrlBarEnhancedScrollBarColors(HWND hWnd, COLORREF clrTrackColor, COLORREF clrArrowColor, COLORREF clrUpperNormalStartColor, COLORREF clrUpperNormalEndColor,
										   COLORREF clrLowerNormalStartColor, COLORREF clrLowerNormalEndColor, COLORREF clrUpperPushedStartColor, COLORREF clrUpperPushedEndColor,
										   COLORREF clrLowerPushedStartColor, COLORREF clrLowerPushedEndColor, COLORREF clrUpperHoverStartColor, COLORREF clrUpperHoverEndColor,
										   COLORREF clrLowerHoverStartColor, COLORREF clrLowerHoverEndColor, COLORREF clrHoverButtonBorderColor, COLORREF clrButtonBorderColor)
{
   LPSCRLBAR lpScrlBar = ScrlBarLock(hWnd);
   if (lpScrlBar != NULL)
   {
   lpScrlBar->clrTrack = clrTrackColor;
   lpScrlBar->clrArrow = clrArrowColor;
   lpScrlBar->clrUpperNormalStart = clrUpperNormalStartColor;
   lpScrlBar->clrUpperNormalEnd = clrUpperNormalEndColor;
   lpScrlBar->clrLowerNormalStart = clrLowerNormalStartColor;
   lpScrlBar->clrLowerNormalEnd = clrLowerNormalEndColor;
   lpScrlBar->clrUpperHoverStart = clrUpperHoverStartColor;
   lpScrlBar->clrUpperHoverEnd = clrUpperHoverEndColor;
   lpScrlBar->clrLowerHoverStart = clrLowerHoverStartColor;
   lpScrlBar->clrLowerHoverEnd = clrLowerHoverEndColor;
   lpScrlBar->clrUpperPushedStart = clrUpperPushedStartColor;
   lpScrlBar->clrUpperPushedEnd = clrUpperPushedEndColor;
   lpScrlBar->clrLowerPushedStart = clrLowerPushedStartColor;
   lpScrlBar->clrLowerPushedEnd = clrLowerPushedEndColor;
   lpScrlBar->clrHoverBorderColor = clrHoverButtonBorderColor;
   lpScrlBar->clrBorderColor = clrButtonBorderColor;
   InvalidateRect(hWnd, NULL, TRUE);
   }
   ScrlBarUnlock(hWnd);
}

void ScrlBarPaintItemEnhanced(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar, LPRECT lpRect,
                      LPRECT lpRectUpdate, BOOL fBtnDown,
                      BOOL fBtnDown2, LPINT lpdTop, LPINT lpdBottom, int type)
{
RECT  RectTemp;
RECT  RectInt;
int   dTop;
int   dBottom;
int   dLeft;
int   dRight;

if (lpScrlBar->fIsHorizontal)
{
SetRect(&RectTemp, *lpdTop, lpRect->top, *lpdTop +
        lpScrlBar->dWidth, lpRect->bottom);

if (RectTemp.right > lpRect->left + ((lpRect->right - lpRect->left) / 2))
   RectTemp.right = lpRect->left + ((lpRect->right - lpRect->left) / 2);

dLeft = RectTemp.right;

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
	ScrlBarPaintItemEnhanced2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
	fBtnDown, type, lpScrlBar->fIsHorizontal, FALSE);

if (!lpScrlBar->fIs95)
   {
   if (dLeft >= lpRectUpdate->left && dLeft < lpRectUpdate->right)
      {
      MoveToEx(hDC, dLeft, lpRect->top, NULL);
      LineTo(hDC, dLeft, lpRect->bottom);
      }

   dLeft += 1;
   }

if (dLeft + lpScrlBar->dWidth <= lpRect->left +
    ((lpRect->right - lpRect->left) / 2))
   *lpdTop = dLeft;

dRight = *lpdBottom;
dRight -= lpScrlBar->dWidth;
dRight = max(dRight, lpRect->left + ((lpRect->right - lpRect->left) / 2));
SetRect(&RectTemp, dRight , lpRect->top, dRight +
        lpScrlBar->dWidth, lpRect->bottom);

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
	ScrlBarPaintItemEnhanced2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
                     fBtnDown2, type+3, lpScrlBar->fIsHorizontal, FALSE);

if (!lpScrlBar->fIs95)
   {
   dRight -= 1;

   if (dRight >= lpRectUpdate->left && dRight < lpRectUpdate->right)
      {
      MoveToEx(hDC, dRight, lpRect->top, NULL);
      LineTo(hDC, dRight, lpRect->bottom);
      }
   }

if (dRight - lpScrlBar->dWidth >= lpRect->left +
    ((lpRect->right - lpRect->left) / 2))
   *lpdBottom = dRight;
}
else
{
SetRect(&RectTemp, lpRect->left, *lpdTop, lpRect->right, *lpdTop +
        lpScrlBar->dHeight);

if (RectTemp.bottom > lpRect->top + ((lpRect->bottom - lpRect->top) / 2))
   RectTemp.bottom = lpRect->top + ((lpRect->bottom - lpRect->top) / 2);

dTop = RectTemp.bottom;

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
	ScrlBarPaintItemEnhanced2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
	fBtnDown, type, lpScrlBar->fIsHorizontal, FALSE);

if (!lpScrlBar->fIs95)
   {
   if (dTop >= lpRectUpdate->top && dTop < lpRectUpdate->bottom)
      {
      MoveToEx(hDC, lpRect->left, dTop, NULL);
      LineTo(hDC, lpRect->right, dTop);
      }

   dTop += 1;
   }

if (dTop + lpScrlBar->dHeight <= lpRect->top +
    ((lpRect->bottom - lpRect->top) / 2))
   *lpdTop = dTop;

dBottom = *lpdBottom;
dBottom -= lpScrlBar->dHeight;
dBottom = max(dBottom, lpRect->top + ((lpRect->bottom - lpRect->top) / 2));
SetRect(&RectTemp, lpRect->left, dBottom, lpRect->right, dBottom +
        lpScrlBar->dHeight);

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
	ScrlBarPaintItemEnhanced2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
                     fBtnDown2, type+3, lpScrlBar->fIsHorizontal, FALSE);

if (!lpScrlBar->fIs95)
   {
   dBottom -= 1;

   if (dBottom >= lpRectUpdate->top && dBottom < lpRectUpdate->bottom)
      {
      MoveToEx(hDC, lpRect->left, dBottom, NULL);
      LineTo(hDC, lpRect->right, dBottom);
      }
   }

if (dBottom - lpScrlBar->dHeight >= lpRect->top +
    ((lpRect->bottom - lpRect->top) / 2))
   *lpdBottom = dBottom;

}
}

void  ScrlBarPaintTrackVisualStyles(HWND hWnd, HDC hDC, LPRECT lpRect)
{
	BOOL isHorizontal = FALSE;
	int part = 0;
	int state = 1;
	HTHEME hTheme = SS_OpenThemeData(hWnd, L"ScrollBar");
	if (isHorizontal)
		part = 5;
	else 
		part = 7;
	SS_DrawThemeBackground( hTheme, hDC, part, state, lpRect, lpRect);
	SS_CloseThemeData(hTheme);

}
void ScrlBarPaintItemVisualStyles(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar, LPRECT lpRect,
                      LPRECT lpRectUpdate, BOOL fBtnDown,
                      BOOL fBtnDown2, LPINT lpdTop, LPINT lpdBottom, int type)
{
RECT  RectTemp;
RECT  RectInt;
int   dTop;
int   dBottom;

if (lpScrlBar->fIsHorizontal)
{
SetRect(&RectTemp, *lpdTop, lpRect->top, *lpdTop +
        lpScrlBar->dWidth,  lpRect->bottom);

if (RectTemp.right > lpRect->left + ((lpRect->right - lpRect->left) / 2))
   RectTemp.right = lpRect->left + ((lpRect->right - lpRect->left) / 2);

dTop = RectTemp.right;

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
	ScrlBarPaintItemVisualStyles2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
	fBtnDown, type, lpScrlBar->fIsHorizontal);

if (!lpScrlBar->fIs95)
   {
   if (dTop >= lpRectUpdate->left && dTop < lpRectUpdate->right)
      {
      MoveToEx(hDC,dTop, lpRect->top, NULL);
      LineTo(hDC, dTop, lpRect->bottom );
      }

   dTop += 1;
   }

if (dTop + lpScrlBar->dWidth <= lpRect->left +
    ((lpRect->right - lpRect->left) / 2))
   *lpdTop = dTop;

dBottom = *lpdBottom;
dBottom -= lpScrlBar->dWidth;
dBottom = max(dBottom, lpRect->left + ((lpRect->right - lpRect->left) / 2));
SetRect(&RectTemp, dBottom, lpRect->top, dBottom +
        lpScrlBar->dWidth, lpRect->bottom);

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
	ScrlBarPaintItemVisualStyles2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
                     fBtnDown2, type+3, lpScrlBar->fIsHorizontal);

if (!lpScrlBar->fIs95)
   {
   dBottom -= 1;

   if (dBottom >= lpRectUpdate->left && dBottom < lpRectUpdate->right)
      {
      MoveToEx(hDC, dBottom, lpRect->top, NULL);
      LineTo(hDC, dBottom, lpRect->bottom);
      }
   }

if (dBottom - lpScrlBar->dWidth >= lpRect->left +
    ((lpRect->right - lpRect->left) / 2))
   *lpdBottom = dBottom;
}
else
{
SetRect(&RectTemp, lpRect->left, *lpdTop, lpRect->right, *lpdTop +
        lpScrlBar->dHeight);

if (RectTemp.bottom > lpRect->top + ((lpRect->bottom - lpRect->top) / 2))
   RectTemp.bottom = lpRect->top + ((lpRect->bottom - lpRect->top) / 2);

dTop = RectTemp.bottom;

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
	ScrlBarPaintItemVisualStyles2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
	fBtnDown, type, lpScrlBar->fIsHorizontal);

if (!lpScrlBar->fIs95)
   {
   if (dTop >= lpRectUpdate->top && dTop < lpRectUpdate->bottom)
      {
      MoveToEx(hDC, lpRect->left, dTop, NULL);
      LineTo(hDC, lpRect->right, dTop);
      }

   dTop += 1;
   }

if (dTop + lpScrlBar->dHeight <= lpRect->top +
    ((lpRect->bottom - lpRect->top) / 2))
   *lpdTop = dTop;

dBottom = *lpdBottom;
dBottom -= lpScrlBar->dHeight;
dBottom = max(dBottom, lpRect->top + ((lpRect->bottom - lpRect->top) / 2));
SetRect(&RectTemp, lpRect->left, dBottom, lpRect->right, dBottom +
        lpScrlBar->dHeight);

if (IntersectRect(&RectInt, &RectTemp, lpRectUpdate))
	ScrlBarPaintItemVisualStyles2(hWnd, hDC, &RectTemp, IsWindowEnabled(hWnd),
                     fBtnDown2, type+3, lpScrlBar->fIsHorizontal);

if (!lpScrlBar->fIs95)
   {
   dBottom -= 1;

   if (dBottom >= lpRectUpdate->top && dBottom < lpRectUpdate->bottom)
      {
      MoveToEx(hDC, lpRect->left, dBottom, NULL);
      LineTo(hDC, lpRect->right, dBottom);
      }
   }

if (dBottom - lpScrlBar->dHeight >= lpRect->top +
    ((lpRect->bottom - lpRect->top) / 2))
   *lpdBottom = dBottom;
}
}


void Draw_Up_Triangle(HDC hdc, POINT topLeft, COLORREF arrowColor)
{
	RECT rc;
	int x;
	HBRUSH hBrush = CreateSolidBrush(arrowColor);

    int lineHeight = 1;
    int inc = 1;
    for (x = 0; x < V_TRIANGLE_WIDTH; x++)
        {
		 rc.left = topLeft.x + x;
		 rc.top = topLeft.y + V_TRIANGLE_HEIGHT - lineHeight;
		 rc.right = rc.left + 1;
		 rc.bottom = rc.top + lineHeight;

         FillRect(hdc, &rc, hBrush);
         lineHeight += inc;
         if (V_TRIANGLE_HEIGHT == lineHeight)
            inc = -1;
        }
	DeleteObject(hBrush);
}

void Draw_Down_Triangle(HDC hdc, POINT topLeft, COLORREF arrowColor)
{

 	RECT rc;
	int x;
	HBRUSH hBrush = CreateSolidBrush(arrowColor);
	int lineHeight = 1;
    int inc = 1;
    for (x = 0; x < V_TRIANGLE_WIDTH; x++)
        {
		  rc.left = topLeft.x + x;
		  rc.top = topLeft.y;
		  rc.right = rc.left + 1;
 		  rc.bottom = rc.top + lineHeight;
          FillRect(hdc, &rc, hBrush);
          lineHeight += inc;
          if (V_TRIANGLE_HEIGHT == lineHeight)
            inc = -1;
        }
	DeleteObject(hBrush);
 }
void Draw_Left_Triangle(HDC hdc, POINT topLeft, COLORREF arrowColor)
{
	RECT rc;
	int y, x2, y2;
	HBRUSH hBrush = CreateSolidBrush(arrowColor);
	int lineWidth = 1;
    int inc = 1;
    for (y = 0; y < H_TRIANGLE_HEIGHT; y++)
        {
		 rc.left = topLeft.x + H_TRIANGLE_WIDTH - lineWidth;
		 rc.top = topLeft.y + y;
		 rc.right = rc.left + lineWidth;
 		 rc.bottom = rc.top + 1;
         FillRect(hdc, &rc, hBrush);
         x2 = topLeft.x + H_TRIANGLE_WIDTH - lineWidth;
         y2 = topLeft.y + y;
         lineWidth += inc;
         if (H_TRIANGLE_WIDTH == lineWidth)
            inc = -1;
        }
	DeleteObject(hBrush);
}
void Draw_Right_Triangle(HDC hdc, POINT topLeft, COLORREF arrowColor)
{
	RECT rc;
	int y;
	HBRUSH hBrush = CreateSolidBrush(arrowColor);
	int lineWidth = 1;
    int inc = 1;
    for (y = 0; y < H_TRIANGLE_HEIGHT; y++)
        {
 		 rc.left = topLeft.x;
		 rc.top = topLeft.y + y;
		 rc.right = rc.left + lineWidth;
 		 rc.bottom = rc.top + 1;
          FillRect(hdc, &rc, hBrush);
          lineWidth += inc;
          if (H_TRIANGLE_WIDTH == lineWidth)
            inc = -1;
        }
	DeleteObject(hBrush);
}

void ScrlBarPaintItemEnhanced2(HWND hWnd, HDC hdc, LPRECT lpRect, BOOL fIsEnabled, BOOL fBtnDown, int type, BOOL isHorizontal, BOOL isThumb)
{
		HBRUSH borderBrush;
        RECT rc;		
		int x;
		int y;
		int width ;
		int height;
		RECT topRC; 
		RECT bottomRC;
		RECT destRC;
		RECT oldBtnRect;
		RECT newBtnRect;
		BOOL mouseOver = FALSE;	
	    BOOL enabled = TRUE;
		
        LPSCRLBAR lpScrlBar = ScrlBarLock(hWnd);
		
		HBRUSH disBrush = CreateSolidBrush(RGB(192,192,192));
		HBRUSH upperBrush1;
		HBRUSH upperBrush2;
		HBRUSH bottomBrush1;
		HBRUSH bottomBrush2;

		if (isHorizontal)
			lpRect->bottom--;
		else
			lpRect->right--;
			
		
		rc.left = lpRect->left;
		rc.top = lpRect->top;
		rc.bottom = lpRect->bottom;
		rc.right = lpRect->right;
		x = rc.left;
		y = rc.top;
		width = rc.right-rc.left;
		height = rc.bottom-rc.top;

		newBtnRect = *lpRect;
	    oldBtnRect = lpScrlBar->mouseBtnRect;
		InflateRect(&oldBtnRect, -1, -1);
		InflateRect(&newBtnRect, -1, -1);
		IntersectRect(&destRC, &oldBtnRect, &newBtnRect);
		mouseOver = !IsRectEmpty(&destRC);

		if (isHorizontal)
		{
			topRC.left = rc.left;
			topRC.right = rc.right;
			topRC.top = rc.top;
			topRC.bottom = rc.top + (height/2);
			
			bottomRC.left = rc.left;
			bottomRC.top = rc.top + (height/2); 
			bottomRC.right = rc.right;
			bottomRC.bottom = rc.bottom;
		}
		else
		{
			topRC.left = rc.left;
			topRC.right = rc.left + (width/2);
			topRC.top = rc.top;
			topRC.bottom = rc.bottom;

			bottomRC.left = rc.left + (width/2);
			bottomRC.right = rc.right;
			bottomRC.top = rc.top;
			bottomRC.bottom = rc.bottom;
		}

        if (fBtnDown )
        {
		 upperBrush1 = CreateSolidBrush(lpScrlBar->clrUpperPushedStart);
		 upperBrush2 = CreateSolidBrush(lpScrlBar->clrUpperPushedEnd);
		 bottomBrush1 = CreateSolidBrush(lpScrlBar->clrLowerPushedStart);
		 bottomBrush2 = CreateSolidBrush(lpScrlBar->clrLowerPushedEnd);
        }
       else if (mouseOver)
        {
		 upperBrush1 = CreateSolidBrush(lpScrlBar->clrUpperHoverStart);
		 upperBrush2 = CreateSolidBrush(lpScrlBar->clrUpperHoverEnd);
		 bottomBrush1 = CreateSolidBrush(lpScrlBar->clrLowerHoverStart);
		 bottomBrush2 = CreateSolidBrush(lpScrlBar->clrLowerHoverEnd);
        }
       else
        {
		 upperBrush1 = CreateSolidBrush(lpScrlBar->clrUpperNormalStart);
		 upperBrush2 = CreateSolidBrush(lpScrlBar->clrUpperNormalEnd);
		 bottomBrush1 = CreateSolidBrush(lpScrlBar->clrLowerNormalStart);
		 bottomBrush2 = CreateSolidBrush(lpScrlBar->clrLowerNormalEnd);
        }

       if (!enabled)
        {
		  fpGradientFill(hdc, topRC, disBrush, disBrush, isHorizontal ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
          fpGradientFill(hdc, bottomRC, disBrush, disBrush, isHorizontal ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
        }
        else
        {
          fpGradientFill(hdc, topRC, upperBrush1, upperBrush2, isHorizontal ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
          fpGradientFill(hdc, bottomRC, bottomBrush1, bottomBrush2, isHorizontal ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
        }
		DeleteObject(disBrush);
		DeleteObject(upperBrush1);
		DeleteObject(upperBrush2);
		DeleteObject(bottomBrush1);
		DeleteObject(bottomBrush2);

		//hPen = CreatePen(PS_SOLID, 1, RGB(255,255,255));
		//hPenOld = SelectObject(hdc, hPen);

        //SS_DrawLine(hdc, x, y + height - 2, x + width - 1, y + height - 2);
        //SS_DrawLine(hdc, x, y + 1, x + width - 1, y + 1);

		//SelectObject(hdc, hPenOld);
		//DeleteObject(hPen);

		if (mouseOver)
			borderBrush = CreateSolidBrush(lpScrlBar->clrHoverBorderColor);
		else
			borderBrush = CreateSolidBrush(lpScrlBar->clrBorderColor);

       FrameRect(hdc, &rc, borderBrush);
	    DeleteObject(borderBrush);
        
        if (width > 2 && height > 2 && !isThumb)
        {
	      RECT arrowRect = {0,0,0,0};
          /*
          HBRUSH enabledBrush = CreateSolidBrush(lpScrlBar->clrArrow);
          HBRUSH lightBrush = GetSysColorBrush(COLOR_3DHIGHLIGHT);
          HBRUSH darkBrush = GetSysColorBrush(COLOR_3DSHADOW);
          */
		  double dWidth; 
          double dHeight; 
		  POINT topLeft;
		
		  if (isHorizontal)
		  {
			dWidth = ceil((double)(width - H_TRIANGLE_WIDTH) / 2.0f);
			dHeight = ceil((double)(height - H_TRIANGLE_HEIGHT) / 2.0f);
		  }
		  else
		  {
		    dWidth = ceil((double)(width - V_TRIANGLE_WIDTH) / 2.0f);
		    dHeight = ceil((double)(height - V_TRIANGLE_HEIGHT) / 2.0f);
		  }

		  topLeft.x = rc.left + (int)dWidth;
		  topLeft.y = rc.top + (int)dHeight;

		  switch (type)
		  {
		  case 1:
			  if (isHorizontal)
			  {
				  Draw_Left_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
			      SS_DrawLine(hdc, topLeft.x - 1, topLeft.y, topLeft.x - 1, topLeft.y + H_TRIANGLE_HEIGHT );
			  }
			  else
			  {
				  Draw_Up_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
				  SS_DrawLine(hdc, topLeft.x, topLeft.y - 1, topLeft.x + V_TRIANGLE_WIDTH , topLeft.y - 1);
			  }
			  break;
		  case 2:
			  if (isHorizontal)
			  {
				  topLeft.x -= 3;
				  Draw_Left_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
				  SS_DrawLine(hdc, topLeft.x + H_TRIANGLE_WIDTH + 1, topLeft.y , topLeft.x + H_TRIANGLE_WIDTH + 1, topLeft.y + H_TRIANGLE_HEIGHT);
				  SS_DrawLine(hdc, topLeft.x + H_TRIANGLE_WIDTH + 3, topLeft.y, topLeft.x + H_TRIANGLE_WIDTH + 3, topLeft.y + H_TRIANGLE_HEIGHT );
			  }
			  else
			  {
				  topLeft.y -= 3;
				  Draw_Up_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
				  SS_DrawLine(hdc, topLeft.x, topLeft.y + V_TRIANGLE_HEIGHT + 1, topLeft.x + V_TRIANGLE_WIDTH , topLeft.y + V_TRIANGLE_HEIGHT + 1);
				  SS_DrawLine(hdc, topLeft.x, topLeft.y + V_TRIANGLE_HEIGHT + 3, topLeft.x + V_TRIANGLE_WIDTH , topLeft.y + V_TRIANGLE_HEIGHT + 3);
			  }
			  break;
		  case 3:
			  if (isHorizontal)
			  {
				  topLeft.x -= 1;
				  Draw_Left_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
			  }
			  else
			  {
				  topLeft.y -= 1;
				  Draw_Up_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
			  }
			  break;
		  case 4:
			  if (isHorizontal)
			  {
				  topLeft.x -=1;
				  Draw_Right_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
				  SS_DrawLine(hdc, topLeft.x + H_TRIANGLE_WIDTH, topLeft.y, topLeft.x + H_TRIANGLE_WIDTH, topLeft.y + H_TRIANGLE_HEIGHT );
			  }
			  else
			  {
				  topLeft.y -= 1;
  				  Draw_Down_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
				  SS_DrawLine(hdc, topLeft.x, topLeft.y + V_TRIANGLE_HEIGHT, topLeft.x + V_TRIANGLE_WIDTH , topLeft.y + V_TRIANGLE_HEIGHT);
			  }
			  break;
		  case 5:
			  if (isHorizontal)
			  {
				  topLeft.x +=2;
				  Draw_Right_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
                  SS_DrawLine(hdc, topLeft.x - 2, topLeft.y, topLeft.x - 2, topLeft.y + H_TRIANGLE_HEIGHT );
				  SS_DrawLine(hdc, topLeft.x - 4, topLeft.y, topLeft.x - 4, topLeft.y + H_TRIANGLE_HEIGHT );
			  }
			  else
			  {
				  topLeft.y += 2;
  				  Draw_Down_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
				  SS_DrawLine(hdc, topLeft.x, topLeft.y - 2, topLeft.x + V_TRIANGLE_WIDTH , topLeft.y - 2);
				  SS_DrawLine(hdc, topLeft.x, topLeft.y - 4, topLeft.x + V_TRIANGLE_WIDTH , topLeft.y - 4);
			  }
			  break;
		  case 6:			  
			  if (isHorizontal)
				  Draw_Right_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
			  else
  				  Draw_Down_Triangle(hdc, topLeft, lpScrlBar->clrArrow);
			  break;

		  }
          //DeleteObject(enabledBrush);
        }
		//hPen = CreatePen(PS_SOLID, 1, RGB(146, 161, 189));
		//hPenOld = SelectObject(hdc, hPen);
        //SS_DrawLine(hdc, x, y, x + width, y);
        //SS_DrawLine(hdc, x, y + height - 1, x + width, y + height - 1);
		//SelectObject(hdc, hPenOld);
		//DeleteObject(hPen);

		ScrlBarUnlock(hWnd);

}

void ScrlBarPaintVisualStyleThumb(HWND hWnd, HDC hdc, LPRECT lpRect, BOOL fIsEnabled, BOOL fBtnDown, BOOL isHorizontal)
{
		int state = 0;
	    BOOL enabled = TRUE;
		RECT destRC;
		RECT oldBtnRect;
		RECT newBtnRect;
		BOOL mouseOver = FALSE;	

		HTHEME hTheme = SS_OpenThemeData(hWnd, L"ScrollBar");	
	
        LPSCRLBAR lpScrlBar = ScrlBarLock(hWnd);
		
		newBtnRect = *lpRect;
	    oldBtnRect = lpScrlBar->mouseBtnRect;
		InflateRect(&oldBtnRect, -1, -1);
		InflateRect(&newBtnRect, -1, -1);
		IntersectRect(&destRC, &oldBtnRect, &newBtnRect);
		mouseOver = !IsRectEmpty(&destRC);
		ScrlBarUnlock(hWnd);
		
   	    state = !enabled ? 4 : fBtnDown ? 3 : mouseOver ? 2 : 1;
		SS_DrawThemeBackground( hTheme, hdc, isHorizontal ? 2 : 3, state, lpRect, lpRect);
		SS_CloseThemeData(hTheme);


}
void ScrlBarPaintItemVisualStyles2(HWND hWnd, HDC hdc, LPRECT lpRect, BOOL fIsEnabled, BOOL fBtnDown, int type, BOOL isHorizontal)
{
		int state = 0;
	    BOOL enabled = TRUE;
		RECT destRC;
		RECT oldBtnRect;
		RECT newBtnRect;
		BOOL mouseOver = FALSE;	
		HRESULT res = S_OK;
		HGLOBAL gh1 = 0;
		LPWSTR lpwstrFile = NULL;
		HGLOBAL gh2 = 0;
		LPWSTR lpwstrColor = NULL;
		HGLOBAL gh3 = 0;
		LPWSTR lpwstrSize = NULL;
		POINT pt;
		HPEN hPen;
		HPEN hPenOld;
		COLORREF clr;
        
		HTHEME hTheme = SS_OpenThemeData(hWnd, L"ScrollBar");
		
	
        LPSCRLBAR lpScrlBar = ScrlBarLock(hWnd);
		
		newBtnRect = *lpRect;
	    oldBtnRect = lpScrlBar->mouseBtnRect;
		InflateRect(&oldBtnRect, -1, -1);
		InflateRect(&newBtnRect, -1, -1);
		IntersectRect(&destRC, &oldBtnRect, &newBtnRect);
		mouseOver = !IsRectEmpty(&destRC);
		ScrlBarUnlock(hWnd);
		
		if (type <= 3)
		{
			state = !enabled ? 4 : fBtnDown ? 3 : mouseOver ? 2 : 1;
			if (isHorizontal)
				state += 8;
		}
		else
		{
			state = !enabled ? 8 : fBtnDown ? 7 : mouseOver ? 6 : 5;
			if (isHorizontal)
				state += 8;

		}

		SS_DrawThemeBackground( hTheme, hdc, 1, state, lpRect, lpRect);

  
		if (type != 3 && type != 6)
		{
			gh1 = GlobalAlloc(GHND,255);
			lpwstrFile = (LPWSTR)GlobalLock(gh1);
			gh2 = GlobalAlloc(GHND,255);
			lpwstrColor = (LPWSTR)GlobalLock(gh2);
			gh3 = GlobalAlloc(GHND,255);
			lpwstrSize = (LPWSTR)GlobalLock(gh3);
			res = SS_GetCurrentThemeName(lpwstrFile, 255, lpwstrColor, 255, lpwstrSize, 255);
			SS_GetThemePosition(hTheme, 1, fBtnDown ? 11 : 9, 3409, &pt );

			if (lstrcmpW((LPCWSTR)lpwstrColor, L"Metallic")== 0)
				clr = RGB(0,0,0);
			else if (lstrcmpW((LPCWSTR)lpwstrColor, L"HomeStead") == 0)
				clr = RGB(255,255,255);
			else if (lstrcmpW((LPCWSTR)lpwstrColor, L"NormalColor") == 0)
				clr = RGB(77,97,133);
			else
				clr = RGB(0,0,0);

			hPen = CreatePen(PS_SOLID, 1, clr);
			hPenOld = SelectObject(hdc, hPen);

			if (isHorizontal)
			{
				if (type == 1)
					SS_DrawLine(hdc, newBtnRect.left + 2, newBtnRect.top + 2, newBtnRect.left +  2 , newBtnRect.bottom - 2);
				else if (type == 2)
				{
					SS_DrawLine(hdc, newBtnRect.right - 2, newBtnRect.top + 2, newBtnRect.right - 2, newBtnRect.bottom-2);
					SS_DrawLine(hdc, newBtnRect.right - 4, newBtnRect.top + 2, newBtnRect.right - 2, newBtnRect.bottom-2);
				}
				else if (type == 4)
					SS_DrawLine(hdc, newBtnRect.right - 4, newBtnRect.top + 2, newBtnRect.right - 4, newBtnRect.bottom-2);
				else if (type == 5)
				{
					SS_DrawLine(hdc, newBtnRect.left, newBtnRect.top + 2, newBtnRect.left, newBtnRect.bottom - 2);
					SS_DrawLine(hdc, newBtnRect.left + 2, newBtnRect.top + 2, newBtnRect.left + 2, newBtnRect.bottom - 2);
				}
			}
			else
			{
				if (type == 1)
					SS_DrawLine(hdc, newBtnRect.left + 2, newBtnRect.top+2, newBtnRect.right-2, newBtnRect.top+2);
				else if (type == 2)
				{
					SS_DrawLine(hdc, newBtnRect.left + 2, newBtnRect.bottom-2, newBtnRect.right-2, newBtnRect.bottom-2);
					SS_DrawLine(hdc, newBtnRect.left + 2, newBtnRect.bottom-4, newBtnRect.right-2, newBtnRect.bottom-4);
				}
				else if (type == 4)
					SS_DrawLine(hdc, newBtnRect.left + 2, newBtnRect.bottom-4, newBtnRect.right-2, newBtnRect.bottom-4);
				else if (type == 5)
				{
					SS_DrawLine(hdc, newBtnRect.left + 2, newBtnRect.top, newBtnRect.right-2, newBtnRect.top);
					SS_DrawLine(hdc, newBtnRect.left + 2, newBtnRect.top+2, newBtnRect.right-2, newBtnRect.top+2);
				}
			}

			SelectObject(hdc, hPenOld);
			DeleteObject(hPen);

			GlobalUnlock(gh1);
			GlobalFree(gh1);
			GlobalUnlock(gh2);
			GlobalFree(gh2);
			GlobalUnlock(gh3);
			GlobalFree(gh3);
		}
		SS_CloseThemeData(hTheme);

}
#endif
void ScrlBarPaintItem2(HWND hWnd, HDC hDC, LPSCRLBAR lpScrlBar, LPRECT lpRect,
                       BOOL fIsEnabled, BOOL fBtnDown, LPHBITMAP lphBitmap,
                       LPTSTR lpBitmapName, LPHBITMAP lphBitmapDisabled,
                       LPTSTR lpBitmapDisabledName)
{
BITMAP  bm;
HBITMAP hBitmap;
HBRUSH  hBrushBackground;
RECT    Rect;
short   dOffset = 0;
#if defined(_WIN64) || defined(_IA64)
long    x;
long    y;
#else
short   x;
short   y;
#endif

CopyRect(&Rect, lpRect);

/*************************************
* Fill the background with the
* current background of the display
*************************************/

hBrushBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
FillRect(hDC, &Rect, hBrushBackground);
DeleteObject(hBrushBackground);

/**************
* Draw Shadow
**************/

if (lpScrlBar->fIs95)
   {
   if (fBtnDown)
      {
      dOffset = 1;
      tbDrawShadows(hDC, &Rect, 1, GetSysColor(COLOR_BTNSHADOW),
                    GetSysColor(COLOR_BTNSHADOW), FALSE, FALSE);
      }
   else
      {
      tbDrawShadows(hDC, &Rect, 1, RGBCOLOR_BLACK, GetSysColor(COLOR_BTNFACE),
                    FALSE, TRUE);
      tbShrinkRect(&Rect, 1, 1);
      tbDrawShadows(hDC, &Rect, 1, GetSysColor(COLOR_BTNSHADOW),
                    GetSysColor(COLOR_BTNHIGHLIGHT), FALSE, TRUE);
      }
   }

else
   {
   if (fBtnDown)
      {
      dOffset = 1;
      tbDrawShadows(hDC, &Rect, 1, GetSysColor(COLOR_BTNSHADOW),
                    GetSysColor(COLOR_BTNFACE), TRUE, FALSE);
      }
   else
      {
      tbDrawShadows(hDC, &Rect, 1, GetSysColor(COLOR_BTNSHADOW),
                    GetSysColor(COLOR_BTNHIGHLIGHT), FALSE, TRUE);
      tbShrinkRect(&Rect, 1, 1);
      tbDrawShadows(hDC, &Rect, 1, GetSysColor(COLOR_BTNSHADOW),
                    GetSysColor(COLOR_BTNFACE), FALSE, TRUE);
      }
   }

tbShrinkRect(&Rect, 1, 1);

if (lphBitmap != NULL && lphBitmapDisabled != NULL)
{
	if (fIsEnabled)
	   {
	   if (!*lphBitmap)
		  *lphBitmap = LoadBitmap(hDynamicInst, lpBitmapName);

	   hBitmap = *lphBitmap;
	   }
	else 
	   {
	   if (!*lphBitmapDisabled)
		  *lphBitmapDisabled = LoadBitmap(hDynamicInst, lpBitmapDisabledName);

	   hBitmap = *lphBitmapDisabled;
	   }

	GetObject(hBitmap, sizeof(BITMAP), &bm);

	/**************
	* Draw Bitmap
	**************/

	x = (Rect.left + (Rect.right - Rect.left - bm.bmWidth) / 2) + dOffset;
	y = (Rect.top + (Rect.bottom - Rect.top - bm.bmHeight) / 2) + dOffset;

	tbDrawBitmap(hDC, hBitmap, &Rect, x, y, SRCAND);
}
}


int ScrlBarGetButton(HWND hWnd, LPSCRLBAR lpScrlBar, LPRECT lpRectClient,
                     LPRECT lpRectBtn, int yMouse)
{
long  lStyle;
int   dRet = -1;
int   dRetTemp;
int   dTop = 0;
int   dBottom = lpRectClient->bottom;
short length = lpScrlBar->dHeight;
int   dEnd = lpRectClient->bottom;

#ifdef SS_V80
if (lpScrlBar->fIsHorizontal)
{
	dBottom = lpRectClient->right;
	length = lpScrlBar->dWidth;
	dEnd = lpRectClient->right;
}

#endif

lStyle = GetWindowLong(hWnd, GWL_STYLE);

if (lStyle & SBRS_HOMEEND)
   dRet = ScrlBarGetBtn2(lpScrlBar, lpRectClient, lpRectBtn, &dTop, &dBottom,
                         SB_TOP, SB_BOTTOM, yMouse, length, lStyle);

if (lStyle & SBRS_PAGEUPDN)
   if ((dRetTemp = ScrlBarGetBtn2(lpScrlBar, lpRectClient, lpRectBtn, &dTop,
                                  &dBottom, SB_PAGEUP, SB_PAGEDOWN, yMouse, length, lStyle)) !=
                                  -1)
      dRet = dRetTemp;

if (lStyle & SBRS_UPDN)
   if ((dRetTemp = ScrlBarGetBtn2(lpScrlBar, lpRectClient, lpRectBtn, &dTop,
                                  &dBottom, SB_LINEUP, SB_LINEDOWN, yMouse, length, lStyle)) !=
                                  -1)
      dRet = dRetTemp;
#ifdef SS_V80
if (dRet == -1)
{
   if (lpScrlBar->fIsHorizontal && (lStyle & SBRS_THUMB))
   {
	   if (yMouse < lpScrlBar->rectThumb.left)
		   dRet = SB_PAGEUP;
	   else if (yMouse > lpScrlBar->rectThumb.right)
		   dRet = SB_PAGEDOWN;
   lpRectBtn->bottom = lpRectClient->bottom;
   lpRectBtn->top = lpRectClient->top;
   lpRectBtn->left = yMouse-2;
   lpRectBtn->right = yMouse+2;
   }
   else if (lStyle & SBRS_THUMB)
   {
	   if (yMouse < lpScrlBar->rectThumb.top)
		   dRet = SB_PAGEUP;
	   else if (yMouse > lpScrlBar->rectThumb.bottom)
		   dRet = SB_PAGEDOWN;
   lpRectBtn->bottom = yMouse+2;
   lpRectBtn->top = yMouse-2;
   lpRectBtn->left = lpRectClient->left;
   lpRectBtn->right = lpRectClient->right;
   }

}
#else
if (dRet == -1 && (lStyle & SBRS_PAGEUPDN))
{
   if ((dRetTemp = ScrlBarGetBtn2(lpScrlBar, lpRectClient, lpRectBtn, &dTop,
                                  &dBottom, SB_PAGEUP, SB_PAGEDOWN, yMouse, (dEnd / 2) - dTop, lStyle)) !=
                                  -1)
		{
      dRet = dRetTemp;
#ifdef SS_V80
		if (lpScrlBar->fIsHorizontal)
		{
		if (dRet == SB_PAGEUP)
			lpRectBtn->right = lpRectClient->right / 2;
		else
			lpRectBtn->left = lpRectClient->right / 2;
		}
		else
		{
#endif
		if (dRet == SB_PAGEUP)
			lpRectBtn->bottom = lpRectClient->bottom / 2;
		else
			lpRectBtn->top = lpRectClient->bottom / 2;
#ifdef SS_V80
		}
#endif
		}
}
#endif

return (dRet);
}


int ScrlBarGetBtn2(LPSCRLBAR lpScrlBar, LPRECT lpRectClient,
                   LPRECT lpRectBtn, LPINT lpdTop, LPINT lpdBottom,
                   int wTop, int wBottom, int yMouse, int dHeight, long lStyle)
{
int dRet = -1;

#ifdef SS_V80
if (lpScrlBar->fIsHorizontal)
{
if (yMouse >= *lpdTop && yMouse <= min(*lpdTop + dHeight,
    lpRectClient->left + ((lpRectClient->right - lpRectClient->left) / 2)))
   {
   SetRect(lpRectBtn, *lpdTop, lpRectClient->top, *lpdTop + dHeight + 1, lpRectClient->bottom);
   dRet = (short)wTop;
   }

if (*lpdTop + dHeight + 1 + dHeight <=
    lpRectClient->left + ((lpRectClient->right - lpRectClient->left) / 2))
   {
   *lpdTop += dHeight;
   if (!lpScrlBar->fIs95)
      *lpdTop += 1;
   }

if (dRet == -1)
   {
   if (yMouse >= max(*lpdBottom - dHeight - 1,
       lpRectClient->left + ((lpRectClient->right - lpRectClient->left) / 2)) &&
       yMouse < *lpdBottom)
      {
      SetRect(lpRectBtn, *lpdBottom - dHeight -
              1, lpRectClient->top, *lpdBottom, lpRectClient->bottom);
      dRet = (short)wBottom;
      }

   if (*lpdBottom - dHeight + 1 - dHeight >
       lpRectClient->left + ((lpRectClient->right - lpRectClient->left) / 2))
      {
      *lpdBottom -= dHeight;
      if (!lpScrlBar->fIs95)
         *lpdBottom -= 1;
      }
   }

if (dRet == -1)
{
	if (lStyle & SBRS_THUMB)
	{
		if (yMouse > lpScrlBar->rectThumb.left && yMouse <  lpScrlBar->rectThumb.right)
		{
			dRet = SB_THUMBPOSITION;
			SetRect(lpRectBtn,lpScrlBar->rectThumb.left, lpScrlBar->rectThumb.top, lpScrlBar->rectThumb.right, lpScrlBar->rectThumb.bottom);
		}
	}
}

}
else
{
#endif

if (yMouse >= *lpdTop && yMouse <= min(*lpdTop + dHeight,
    lpRectClient->top + ((lpRectClient->bottom - lpRectClient->top) / 2)))
   {
   SetRect(lpRectBtn, lpRectClient->left, *lpdTop, lpRectClient->right,
           *lpdTop + dHeight + 1);
   dRet = (short)wTop;
   }

if (*lpdTop + dHeight + 1 + dHeight <=
    lpRectClient->top + ((lpRectClient->bottom - lpRectClient->top) / 2))
   {
   *lpdTop += dHeight;
   if (!lpScrlBar->fIs95)
      *lpdTop += 1;
   }

if (dRet == -1)
   {
   if (yMouse >= max(*lpdBottom - dHeight - 1,
       lpRectClient->top + ((lpRectClient->bottom - lpRectClient->top) / 2)) &&
       yMouse < *lpdBottom)
      {
      SetRect(lpRectBtn, lpRectClient->left, *lpdBottom - dHeight -
              1, lpRectClient->right, *lpdBottom);
      dRet = (short)wBottom;
      }

   if (*lpdBottom - dHeight + 1 - dHeight >
       lpRectClient->top + ((lpRectClient->bottom - lpRectClient->top) / 2))
      {
      *lpdBottom -= dHeight;
      if (!lpScrlBar->fIs95)
         *lpdBottom -= 1;
      }
   }

#ifdef SS_V80
if (dRet == -1)
	{
	if (yMouse > lpScrlBar->rectThumb.top && yMouse <  lpScrlBar->rectThumb.bottom)
	{
        SetRect(lpRectBtn,lpScrlBar->rectThumb.left, lpScrlBar->rectThumb.top, lpScrlBar->rectThumb.right, lpScrlBar->rectThumb.bottom);
		dRet = SB_THUMBPOSITION;
	}
	}
}
#endif

return (dRet);
}


static void ScrlBarSetPtr(hWnd, hGlobal)

HWND         hWnd;
GLOBALHANDLE hGlobal;
{
#if defined(_WIN64) || defined(_IA64)
SetWindowLongPtr(hWnd, 0, (LONG_PTR)hGlobal);
#else
SetWindowLong(hWnd, 0, (long)hGlobal);
#endif
}
 

static LPSCRLBAR ScrlBarLock(hWnd)

HWND hWnd;
{
#if defined(_WIN64) || defined(_IA64)
return ((LPSCRLBAR)GlobalLock((HANDLE)GetWindowLongPtr(hWnd, 0)));
#else
return ((LPSCRLBAR)GlobalLock((HANDLE)GetWindowLong(hWnd, 0)));
#endif
}
 

static void ScrlBarUnlock(hWnd)

HWND hWnd;
{
#if defined(_WIN64) || defined(_IA64)
GlobalUnlock((HANDLE)GetWindowLongPtr(hWnd, 0));
#else
GlobalUnlock((HANDLE)GetWindowLong(hWnd, 0));
#endif
}
 

static void ScrlBarFree(hWnd)

HWND hWnd;
{
#if defined(_WIN64) || defined(_IA64)
GlobalFree((HANDLE)GetWindowLongPtr(hWnd, 0));
#else
GlobalFree((HANDLE)GetWindowLong(hWnd, 0));
#endif
}

#else


BOOL RegisterScrlBar(HANDLE hInstance)
{
return (TRUE);
}

#endif
