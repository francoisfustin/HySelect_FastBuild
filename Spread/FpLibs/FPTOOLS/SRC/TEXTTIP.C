/***********************************************************************
* TEXTTIP.C
*
* Copyright (C) 1998 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
*
* RAP01 - 9114                                                  07.23.01
***********************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fptools.h"
#include "fpmemmgr.h"
#include <tchar.h>
#include "texttip.h"

#define TT_PROP  _T("fpTextTip")
#define TT_CLASS _T("fpTextTip")

#define TT_TIMER1ID    111

#define TTMx_MOUSEMOVE 0x0652
#define TTMx_MOUSEDOWN 0x0653
#ifdef TT_V2
#define TTMx_MOUSEUP   0x0655
#endif
#define TTMx_KEYDOWN   0x0654
#define TTM_RESETHOOKS 0x0656

#define TT_HIDE_NO        0
#define TT_HIDE_TILLNEW   1
#define TT_HIDE_TILLMOUSE 2

#define TT_IsLButtonDown() ((GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? \
                             VK_RBUTTON : VK_LBUTTON) & 0x8000) != 0)
#define TT_IsRButtonDown() ((GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? \
                             VK_LBUTTON : VK_RBUTTON) & 0x8000) != 0)
#define TT_NullID(lpID)    (lpID)->lID1 = 0, (lpID)->lID2 = 0
#ifdef TT_V2
#define TT_IsEqualID(lpID1, lpID2) ((lpID1)->lID1 == (lpID2)->lID1 && \
                                    (lpID1)->lID2 == (lpID2)->lID2 && \
                                    (lpID1)->lID3 == (lpID2)->lID3)
#define TT_SetEqual(lpID1, lpID2) ((lpID1)->lID1 = (lpID2)->lID1, \
                                   (lpID1)->lID2 = (lpID2)->lID2, \
                                   (lpID1)->lID3 = (lpID2)->lID3)
#else
#define TT_IsEqualID(lpID1, lpID2) ((lpID1)->lID1 == (lpID2)->lID1 && \
                                    (lpID1)->lID2 == (lpID2)->lID2)
#define TT_SetEqual(lpID1, lpID2) ((lpID1)->lID1 = (lpID2)->lID1, \
                                   (lpID1)->lID2 = (lpID2)->lID2)
#endif
#define TT_StatusFocus(lpTT) (lpTT->wStatus == FP_TT_STATUS_FIXEDFOCUSONLY || \
                              lpTT->wStatus == FP_TT_STATUS_FLOATINGFOCUSONLY)
#define TT_StatusFixed(lpTT) (lpTT->wStatus == FP_TT_STATUS_FIXED || \
                              lpTT->wStatus == FP_TT_STATUS_FIXEDFOCUSONLY)
#define TT_StatusFloating(lpTT) (lpTT->wStatus == FP_TT_STATUS_FLOATING || \
                                 lpTT->wStatus == FP_TT_STATUS_FLOATINGFOCUSONLY)
#define TT_YFLOATINGOFFSET (GetSystemMetrics(SM_CYCURSOR) / 2)

typedef struct tagTEXTTIP
   {
   HWND     hWnd;
   HWND     hWndParent;
   BOOL     fActive;
   long     lDelay;
   long     lDelayNext;
   WORD     wStatus;
#ifdef TT_V2
   WORD     wScrollTipStatus;
#endif
   BOOL     fTimer1Sent;
   BYTE     bHideOnMouseDown;
   BYTE     bHideOnKeyDown;
   DWORD    dwTickCount;
   FP_TT_ID ID;
   FP_TT_ID IDHideTillNew;
   COLORREF clrBack;
   COLORREF clrFore;
   HGLOBAL  hText;
   LOGFONT  LogFont;
   WORD     wMultiLine;
   } TEXTTIP, FAR *LPTEXTTIP;

typedef struct tagTEXTTIPHOOK
{
#ifndef WIN32
   HOOKPROC lpfnMouseHookProc;
   HOOKPROC lpfnKBHookProc;
#endif
   HHOOK hHookMouse;
   HHOOK hHookKB;
   DWORD dwThreadId;
   HGLOBAL hHookWndList;
   long lHookWndCnt;
} TEXTTIPHOOK, FAR *LPTEXTTIPHOOK;


#define TEXTTIPHOOKS_MAX -1
#define TT_HANDLE HGLOBAL
#define ttGlobalAlloc(dwBytes)         GlobalAlloc(GHND, dwBytes)
#define ttGlobalReAlloc(hMem, dwBytes) GlobalReAlloc((HGLOBAL)hMem, dwBytes, GHND)
#define ttGlobalLock(hMem)             GlobalLock((HGLOBAL)hMem)
#define ttGlobalUnlock(hMem)           GlobalUnlock((HGLOBAL)hMem)
#define ttGlobalFree(hMem)             GlobalFree((HGLOBAL)hMem)

// Each process will load an instance of these variables
static TT_HANDLE   hTextTipHooks = 0; // handle to TEXTTIPHOOK*
static long        lHookCnt = 0;
static short       nfpTextTipWindowCnt = -1;

/**********************
* Function prototypes
**********************/

LRESULT WINAPI _export TT_Proc(HWND hWnd, WORD Msg, WPARAM wParam,
                                LPARAM lParam);
BOOL                TT_Register(HANDLE hInstance);
BOOL                TT_Unregister(HANDLE hInstance);
void                TT_SetHooks(HWND hWnd);
void                TT_ReleaseHooks(HWND hWnd);
LRESULT CALLBACK TT_MouseHook(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TT_KBHook(int nCode, WPARAM wParam, LPARAM lParam);
void                TT_HookSendMessage(HGLOBAL hHookWndList, long lHookWndCnt, WORD wMsg, WPARAM wParam, LPARAM lParam);
void                TT_Hide(LPTEXTTIP lpTT, BYTE bHideOn);
LPTEXTTIP           TT_LockWithParent(HWND hWndParent);
void                TT_UnlockWithParent(HWND hWndParent);
LPTEXTTIP           TT_Lock(HWND hWnd);
void                TT_Unlock(HWND hWnd);
void                TT_Paint(HWND hWnd, HDC hDC, LPTEXTTIP lpTT, LPRECT lpRect);
BOOL                TT_IsMouseInWindow(HWND hWnd, LPARAM lParam);
BOOL                TT_IsWindowSibling(HWND hWndSelf, HWND hWndFocus);


BOOL FPLIB fpTextTipSetInfo(HWND hWndParent, WORD wItem, LPFP_TT_INFO lpInfo)
{
LPTEXTTIP lpTT;
BOOL      fRet = TRUE;

if (lpTT = TT_LockWithParent(hWndParent))
   {
   if (wItem & FP_TT_ITEM_DELAY)
      {
      lpTT->lDelay = lpInfo->lDelay;
      lpTT->lDelayNext = lpInfo->lDelay;
      }

   if ((wItem & FP_TT_ITEM_BACKCOLOR) || (wItem & FP_TT_ITEM_FORECOLOR) ||
       (wItem & FP_TT_ITEM_FONT))
      {
      if (wItem & FP_TT_ITEM_BACKCOLOR)
         lpTT->clrBack = lpInfo->clrBack;

      if (wItem & FP_TT_ITEM_FORECOLOR)
         lpTT->clrFore = lpInfo->clrFore;

      if (wItem & FP_TT_ITEM_FONT)
         _fmemcpy(&lpTT->LogFont, &lpInfo->LogFont, sizeof(LOGFONT));

      if (lpTT->hWnd && IsWindowVisible(lpTT->hWnd))
         InvalidateRect(lpTT->hWnd, NULL, TRUE);
      }

   if ((wItem & FP_TT_ITEM_STATUS) || (wItem & FP_TT_ITEM_ACTIVE) || wItem & FP_TT_ITEM_SCROLLTIPSTATUS)
      {
      if (wItem & FP_TT_ITEM_STATUS)
         lpTT->wStatus = lpInfo->wStatus;
      if (wItem & FP_TT_ITEM_ACTIVE)
         lpTT->fActive = lpInfo->fActive;
#ifdef TT_V2
      if (wItem & FP_TT_ITEM_SCROLLTIPSTATUS)
         lpTT->wScrollTipStatus = lpInfo->fShowScrollTips;
#endif

#ifdef TT_V2
      if (lpTT->fActive && (lpTT->wStatus || lpTT->wScrollTipStatus) && !lpTT->hWnd)
#else
      if (lpTT->fActive && lpTT->wStatus && !lpTT->hWnd)
#endif
         {
         HGLOBAL hTT = GetProp(hWndParent, TT_PROP);

//         if (nfpTextTipWindowCnt == -1)
				{
            TT_Register(fpInstance);
//            nfpTextTipWindowCnt = 0;
				}

         lpTT->hWnd = CreateWindowEx(WS_EX_TOPMOST | 0x80L, TT_CLASS, NULL,
                                     WS_POPUP | WS_CLIPSIBLINGS | WS_BORDER,
                                     0, 0, 0, 0, 0, 0, fpInstance, &hTT);
         nfpTextTipWindowCnt++;
         }
#ifdef TT_V2
      else if ((!lpTT->fActive || (!lpTT->wStatus && !lpTT->wScrollTipStatus)) && lpTT->hWnd)
#else
      else if ((!lpTT->fActive || !lpTT->wStatus) && lpTT->hWnd)
#endif
         {
         DestroyWindow(lpTT->hWnd);
         lpTT->hWnd = 0;
         }
      }

   TT_UnlockWithParent(hWndParent);
   }

return (fRet);
}

#ifdef TT_V2
HWND FPLIB fpTextTipGetHwnd(HWND hWndParent)
{
LPTEXTTIP lpTT;
HWND      hwndRet;

if (lpTT = TT_LockWithParent(hWndParent))
   {
   hwndRet = lpTT->hWnd;
   TT_UnlockWithParent(hWndParent);
   }

return (hwndRet);
}

BOOL fpTextTipGetScrollTip(HWND hWndParent)
{
LPTEXTTIP lpTT;
BOOL      fRet = FALSE;

if (lpTT = TT_LockWithParent(hWndParent))
   {
   fRet = lpTT->ID.lID3;
   TT_UnlockWithParent(hWndParent);
   }

return fRet;
}

void fpTextTipSetScrollTip(HWND hWndParent, BOOL fScrollTip)
{
LPTEXTTIP lpTT;

if (lpTT = TT_LockWithParent(hWndParent))
   {
   lpTT->ID.lID3 = 2; // ScrollTip.
   TT_UnlockWithParent(hWndParent);
   }
}

void fpTextTipHideScrollTip(HWND hWndParent)
{
LPTEXTTIP lpTT;

if (lpTT = TT_LockWithParent(hWndParent))
   {
   if (2 == lpTT->ID.lID3) //ScrollTip
      {
      TT_Hide(lpTT, lpTT->bHideOnMouseDown);
      lpTT->ID.lID3 = 0;
      }

   TT_UnlockWithParent(hWndParent);
   }
}

#endif

BOOL FPLIB fpTextTipGetInfo(HWND hWndParent, LPFP_TT_INFO lpInfo)
{
LPTEXTTIP lpTT;
BOOL      fRet = TRUE;

if (lpTT = TT_LockWithParent(hWndParent))
   {
   lpInfo->lDelay = lpTT->lDelay;
   lpInfo->wStatus = lpTT->wStatus;
   lpInfo->fActive = lpTT->fActive;
   lpInfo->clrBack = lpTT->clrBack;
   lpInfo->clrFore = lpTT->clrFore;
   _fmemcpy(&lpInfo->LogFont, &lpTT->LogFont, sizeof(LOGFONT));
#ifdef TT_V2
   lpInfo->fShowScrollTips = FALSE;    
#endif
    
   TT_UnlockWithParent(hWndParent);
   }

return (fRet);
}


void FPLIB fpTextTipTerminate(HWND hWndParent)
{
HGLOBAL hTT = GetProp(hWndParent, TT_PROP);

if (hTT)
   {
   LPTEXTTIP lpTT = (LPTEXTTIP)GlobalLock(hTT);

   if (lpTT->hWnd)
      DestroyWindow(lpTT->hWnd);

   GlobalUnlock(hTT);
   GlobalFree(hTT);

   RemoveProp(hWndParent, TT_PROP);

   if (nfpTextTipWindowCnt == 0)
		{
      TT_Unregister(fpInstance);
//      nfpTextTipWindowCnt = -1;
		}
   }
}


BOOL TT_Register(HANDLE hInstance)
{
WNDCLASS wc;

wc.style         = CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc   = (WNDPROC)TT_Proc;
wc.cbClsExtra    = 0;
#if defined(_WIN64) || defined(_IA64)
wc.cbWndExtra    = 8;
#else
wc.cbWndExtra    = 4;
#endif
wc.hInstance     = hInstance;
wc.hIcon         = 0;
wc.hCursor       = LoadCursor(0, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = TT_CLASS;

return (RegisterClass((LPWNDCLASS)&wc));
}


BOOL TT_Unregister(HANDLE hInstance)
{
UnregisterClass(TT_CLASS, hInstance);
return TRUE;
}


LRESULT WINAPI _export TT_Proc(HWND hWnd, WORD Msg, WPARAM wParam, LPARAM lParam)
{
LPTEXTTIP lpTT;
switch (Msg)
   {
   case WM_NCCREATE:
#if defined(_WIN64) || defined(_IA64)
      SetWindowLongPtr(hWnd, 0,
                    (LONG_PTR)*(HGLOBAL FAR *)((LPCREATESTRUCT)lParam)->lpCreateParams);
#else
      SetWindowLong(hWnd, 0,
                    (LONG)*(HGLOBAL FAR *)((LPCREATESTRUCT)lParam)->lpCreateParams);
#endif

      fpInitTextOut(hWnd);
      TT_SetHooks(hWnd);
      break;

   case WM_GETDLGCODE:
      return (DLGC_STATIC);

   case WM_NCHITTEST:
      return (HTTRANSPARENT);

/*
   case WM_NCHITTEST:
      {
      LPBLN lpBln;

      lpBln = (LPBLN)GlobalLock((HGLOBAL)GetWindowLong(hWnd, 0));

      if (IsWindowVisible(hWnd) && BLN_ISCTL(lpBln->Ctl))
         {
         RECT  RectTemp;
         POINT pt;

         Bln_GetControlRect(&lpBln->Ctl, &RectTemp);

         pt.x = LOWORD(lParam);
         pt.y = HIWORD(lParam);

         if (PtInRect(&RectTemp, pt))
            return (HTTRANSPARENT);
         }
      }

      break;
*/

   case WM_WININICHANGE:
      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case WM_MOUSEACTIVATE:
      return (MA_NOACTIVATEANDEAT);

   case WM_SETTEXT:
      lpTT = TT_Lock(hWnd);

      if (lpTT->hText)
         {
         GlobalFree(lpTT->hText);
         lpTT->hText = 0;
         }

      if (lParam && *(LPTSTR)lParam)
         {
         lpTT->hText = GlobalAlloc(GHND, (lstrlen((LPTSTR)lParam)+1)*sizeof(TCHAR));

         if (lpTT->hText)
            {
            LPTSTR lpszText = (LPTSTR)GlobalLock(lpTT->hText);
            lstrcpy(lpszText, (LPTSTR)lParam);
            GlobalUnlock(lpTT->hText);
            }
         }

      TT_Unlock(hWnd);
      return (0);

   case WM_ERASEBKGND:
      return (TRUE);

   case WM_PAINT:
      {
      PAINTSTRUCT ps;
      RECT        RectClient;
		HDC         hDC;

      lpTT = TT_Lock(hWnd);
      GetClientRect(hWnd, &RectClient);

      if (wParam)
         hDC = (HDC)wParam;
      else
			{                                       
         BeginPaint(hWnd, &ps);
         hDC = ps.hdc;
			}

		if (hDC)
         {
			HDC     hDCMemory;
			HBITMAP hBitmapOld;
			HBITMAP hBitmapDC;

			// Create a memory device context
			hBitmapDC = CreateCompatibleBitmap(hDC, RectClient.right - RectClient.left,
														  RectClient.bottom - RectClient.top);

			hDCMemory = CreateCompatibleDC(hDC);
			hBitmapOld = SelectObject(hDCMemory, hBitmapDC);

			// Draw Text
         TT_Paint(hWnd, hDCMemory, lpTT, &RectClient);

			// Copy the memory device context bitmap to the display

			BitBlt(hDC, RectClient.left, RectClient.top, RectClient.right - RectClient.left,
					 RectClient.bottom - RectClient.top, hDCMemory, 0, 0, SRCCOPY);

			/***********
			* Clean up
			***********/

			SelectObject(hDCMemory, hBitmapOld);
			DeleteDC(hDCMemory);
			DeleteObject(hBitmapDC);
         }

		if (!wParam)
         EndPaint(hWnd, &ps);

      TT_Unlock(hWnd);
      return (0);
      }

   case WM_DESTROY:
      lpTT = TT_Lock(hWnd);

      TT_ReleaseHooks(hWnd);
      nfpTextTipWindowCnt--;

      if (lpTT->hText)
         {
         GlobalFree(lpTT->hText);
         lpTT->hText = 0;
         }

      fpKillTextOut(hWnd);
      TT_Unlock(hWnd);
      break;

   case WM_TIMER:
      lpTT = TT_Lock(hWnd);

      if (wParam == TT_TIMER1ID)
         KillTimer(hWnd, TT_TIMER1ID);

// removed 10/10/07 -scl
// this change breaks fpCombo and makes the text tip fail to show when the focus is not on Spread's window.
		// RFW - 10/26/05 - 17280
//		if (!TT_IsWindowSibling(lpTT->hWndParent, GetFocus()) /*&& GetParent(lpTT->hWndParent) != GetDesktopWindow()*/) // 20568 -scl
//         TT_NullID(&lpTT->ID);
//        else 
		if (lpTT->hWndParent && lpTT->fTimer1Sent)
         {
// -> fix for GIC21056 -scl
         HWND hWndFocus = GetFocus();
         HWND hWndFocusCompare = GetProp(lpTT->hWndParent, TT_PROP_FOCUS);

			/* RFW - 3/16/05
         if ((lpTT->bHideOnMouseDown && TT_IsLButtonDown()) ||
			*/
         if ((lpTT->bHideOnMouseDown && (TT_IsLButtonDown() || TT_IsRButtonDown())) ||
             (TT_StatusFocus(lpTT) && hWndFocus != lpTT->hWndParent && (!hWndFocus || hWndFocus != hWndFocusCompare))
#ifdef TT_V2
             || !lpTT->wStatus
#endif
             )
            TT_NullID(&lpTT->ID);
// <- fix for GIC21056 -scl

         else if (!IsWindowVisible(hWnd))
            {
            FP_TT_FETCH Fetch;
            POINT       ptPos;
				LPTSTR      lpszFetchText;
#ifdef TT_V2
            BOOL        fCellNote = FALSE;

            if (lpTT->ID.lID3)
               fCellNote = TRUE;
#endif
            GetCursorPos(&ptPos);
            ScreenToClient(lpTT->hWndParent, &ptPos);

            _fmemset(&Fetch, '\0', sizeof(FP_TT_FETCH));

            Fetch.Scan.nMouseX = (short)ptPos.x;
            Fetch.Scan.nMouseY = (short)ptPos.y;
//            Fetch.Scan.ID = lpTT->ID;
            TT_SetEqual(&(Fetch.Scan.ID), &(lpTT->ID));
            Fetch.fShow = TRUE;
            Fetch.wMultiLine = lpTT->wMultiLine;
            Fetch.wAlignX = FP_TT_XALIGN_AUTO;
            Fetch.wAlignY = FP_TT_YALIGN_AUTO;
            Fetch.xOffset = 0;
            Fetch.yOffset = 0;
#ifdef TT_V2
            Fetch.nWidth = (fCellNote?FP_TT_CELLNOTE_WIDTH:dyPixelsPerInch * 2);
#else
            Fetch.nWidth = dyPixelsPerInch * 2;
#endif
            SendMessage(lpTT->hWndParent, FPM_TT_FETCH, (WPARAM)hWnd,
                        (LPARAM)(LPVOID)&Fetch);

            if (!IsWindow(hWnd))
               return (0);

            lpTT->wMultiLine = Fetch.wMultiLine;

				if (Fetch.hText)
					lpszFetchText = (LPTSTR)GlobalLock(Fetch.hText);
				else
					lpszFetchText = Fetch.szText;

            if (Fetch.fShow && *lpszFetchText)
               {
               LPTSTR lpszText;
               HFONT  hFont = CreateFontIndirect(&lpTT->LogFont);
               HFONT  hFontOld;
               RECT   Rect;
               HDC    hDC = GetDC(hWnd);
               short  nScreenWidth = GetSystemMetrics(78);
               short  nScreenHeight = GetSystemMetrics(79);
               short  x;
               short  y;

					// SCL - 3/23/04 - 13932
					if (nScreenHeight == 0)
						nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
					if (nScreenHeight == 0)
						nScreenHeight = GetDeviceCaps(hDC, VERTRES);
					if (nScreenWidth == 0)
						nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
					if (nScreenWidth == 0)
						nScreenWidth = GetDeviceCaps(hDC, HORZRES);

               GetClientRect(lpTT->hWndParent, &Rect);
               IntersectRect(&Fetch.Rect, &Rect, &Fetch.Rect);

               if (hFont)
                  hFontOld = SelectObject(hDC, hFont);

               SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)lpszFetchText);

               if (TT_StatusFloating(lpTT))
                  {
                  x = (short)(ptPos.x + Fetch.xOffset);
                  y = (short)(ptPos.y + Fetch.yOffset + TT_YFLOATINGOFFSET);

                  if (Fetch.wAlignX == FP_TT_XALIGN_AUTO)
                     Fetch.wAlignX = FP_TT_XALIGN_CENTER;
                  }
               else
                  {
#ifdef TT_V2
//RAP01a>>
                  if (fCellNote)
                  {
                    POINT pt;
                    GetCursorPos(&pt);
                    x = (short)(pt.x + GetSystemMetrics(SM_CXCURSOR)/2);
                    y = (short)(pt.y + (int)((double)GetSystemMetrics(SM_CYCURSOR)/1.5));
                  }
                  else
                  {
//<< RAP01a
#endif
                  if (Fetch.wAlignX == FP_TT_XALIGN_RIGHT)
                     x = (short)(Fetch.Rect.right - Fetch.xOffset);
                  else if (Fetch.wAlignX == FP_TT_XALIGN_CENTER)
                     x = (short)((Fetch.Rect.right - Fetch.Rect.left) + Fetch.xOffset);
                  else
                     x = (short)(Fetch.Rect.left + Fetch.xOffset);

                  if (Fetch.wAlignY == FP_TT_YALIGN_BOTTOM)
                     y = (short)(Fetch.Rect.bottom - Fetch.yOffset);
                  else if (Fetch.wAlignY == FP_TT_YALIGN_CENTER)
                     y = (short)((Fetch.Rect.bottom - Fetch.Rect.top) + Fetch.yOffset);
                  else
                     y = (short)(Fetch.Rect.top + Fetch.yOffset);
                  }
#ifdef TT_V2
                  } //RAP01a
#endif

               lpszText = (LPTSTR)GlobalLock(lpTT->hText);

               // Check to see if the string has an
               // embedded CR or LF

               if (Fetch.wMultiLine == FP_TT_MULTILINE_AUTO)
                  {
                  LPTSTR p = lpszText;

                  for ( ; *p; p = CharNext(p))
                     {
                     if (*p == (TCHAR)'\r' || *p == (TCHAR)'\n')
                        {
                        Fetch.wMultiLine = FP_TT_MULTILINE_MULTI;
                        break;
                        }
                     }
                  }

               if (Fetch.wMultiLine != FP_TT_MULTILINE_MULTI)
                  {
                  short nTextWidth;

#ifdef WIN32
						{
                  SIZE sizeRect;
						int iLen = lstrlen(lpszText);
						ABCFLOAT abcf;

                  GetTextExtentPoint32(hDC, lpszText, iLen, &sizeRect);
                  nTextWidth = (WORD)sizeRect.cx + 1 + 2;

						// RFW - 2/5/04 - 4880
						if (iLen)
							if (GetCharABCWidthsFloat(hDC, lpszText[iLen - 1], lpszText[iLen - 1], &abcf) &&
								 abcf.abcfC < 0)
								nTextWidth += -(int)abcf.abcfC;
						}
#else
                  DWORD dwExtent = GetTextExtent(hDC, lpszText, lstrlen(lpszText));
                  nTextWidth = LOWORD(dwExtent) + 1 + 2;
#endif

                  if (Fetch.wMultiLine == FP_TT_MULTILINE_AUTO)
                     {
                     short xTemp;

                     ptPos.x = x;
                     ptPos.y = y;
                     ClientToScreen(lpTT->hWndParent, &ptPos);
                     xTemp = (short)ptPos.x;

                     if ((Fetch.wAlignX == FP_TT_XALIGN_RIGHT &&
                          nTextWidth <= xTemp) ||
                         (Fetch.wAlignX == FP_TT_XALIGN_CENTER &&
                          (nTextWidth / 2) <= min(xTemp, nScreenWidth - xTemp)) ||
                         ((Fetch.wAlignX == FP_TT_XALIGN_AUTO ||
                           Fetch.wAlignX == FP_TT_XALIGN_LEFT) &&
                          nTextWidth <= nScreenWidth - xTemp))
                        {
                        Fetch.nWidth = nTextWidth;
                        Fetch.wMultiLine = FP_TT_MULTILINE_SINGLE;
                        }
                     }
                  else
                     Fetch.nWidth = nTextWidth;
                  }

               GlobalUnlock(lpTT->hText);

               if (Fetch.nWidth > 0)
                  {
						// RFW - 9/27/07 - 21160
		            int iScreenLeft = GetSystemMetrics(76); // SM_XVIRTUALSCREEN
		            int iScreenTop = GetSystemMetrics(77);  // SM_YVIRTUALSCREEN

                  short nHeight;
						/* redundant
						short nScreenWidth = GetSystemMetrics(78);

						if (nScreenWidth == 0)
							nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
						if (nScreenWidth == 0)
							nScreenWidth = GetDeviceCaps(hDC, HORZRES);
						*/
                  Fetch.nWidth = min(Fetch.nWidth, nScreenWidth);

                  if (Fetch.wMultiLine != FP_TT_MULTILINE_SINGLE)
                     {
                     FPDRAWTEXT dt;
                     RECT       Rect;
                     LPTSTR     lpszText = (LPTSTR)GlobalLock(lpTT->hText);
#ifdef TT_V2
//                     short     nCellNoteWidth;
                     short     nCellNoteHeight;

                     if (fCellNote)
                     {  
//                       nCellNoteWidth = FP_TT_CELLNOTE_WIDTH;
                       nCellNoteHeight = FP_TT_CELLNOTE_HEIGHT;
                     }
#endif
                     _fmemset(&dt, '\0', sizeof(FPDRAWTEXT));
// -> fix for TIB6807 -scl
//                     SetRect(&Rect, 0, 0, Fetch.nWidth, 0);
                     SetRect(&Rect, 0, 0, Fetch.nWidth - 2, 0);
// <- fix for TIB6807 -scl

// RFW - 2/1/02 - 9754
//#ifdef TT_V2
//                     if (fCellNote)
//{
//Fetch.nWidth = nCellNoteWidth;
//                        Rect.right = nCellNoteWidth;
//}
//#endif
                     fpDrawText(hWnd, hDC, lpszText, -1, &Rect,
                                DT_WORDBREAK | DT_CALCRECT, &dt);
#ifdef TT_V2
                     if (fCellNote)
                        {
                        if (Rect.bottom < FP_TT_CELLNOTE_HEIGHT)
                           Rect.bottom = nCellNoteHeight;     
                        }
#endif
                     nHeight = (short)(Rect.bottom - Rect.top + 2);

                     {
                     short nTextWidth;
#ifdef WIN32
                     SIZE sizeRect;
                     GetTextExtentPoint32(hDC, lpszText, lstrlen(lpszText), &sizeRect);
                     nTextWidth = (WORD)sizeRect.cx + 1 + 2;
#else
                     DWORD dwExtent = GetTextExtent(hDC, lpszText, lstrlen(lpszText));
                     nTextWidth = LOWORD(dwExtent) + 1 + 2;
#endif

#ifdef TT_V2
                     if (!fCellNote)
                        Fetch.nWidth = min(Fetch.nWidth, nTextWidth);
#endif
                     }

                     GlobalUnlock(lpTT->hText);
                     }
                  else
                     {
                     TEXTMETRIC tm;

                     GetTextMetrics(hDC, &tm);
                     nHeight = (short)(tm.tmHeight + 2);
                     }

                  ptPos.x = x;
                  ptPos.y = y;
#ifdef TT_V2
                  if (TT_StatusFloating(lpTT) || !fCellNote)
                    ClientToScreen(lpTT->hWndParent, &ptPos);
#else
                  ClientToScreen(lpTT->hWndParent, &ptPos);
#endif
                  x = (short)ptPos.x;
                  y = (short)ptPos.y;

                  if (Fetch.wAlignX == FP_TT_XALIGN_RIGHT)
                     x = max(iScreenLeft, x - Fetch.nWidth);
                  else if (Fetch.wAlignX == FP_TT_XALIGN_CENTER)
                     x = max(iScreenLeft, x - (Fetch.nWidth / 2));
                  else if (Fetch.wAlignX == FP_TT_XALIGN_AUTO)
                     {
                     if (x + Fetch.nWidth > nScreenWidth)
                        /* RFW - 4/19/00 - SCS9319
                        if (Fetch.Rect.right - Fetch.xOffset - Fetch.nWidth >= 0)
                           x = Fetch.Rect.right - Fetch.xOffset - Fetch.nWidth;
                        */
                        x = nScreenWidth - Fetch.nWidth;
                     }

                  if (Fetch.wAlignY == FP_TT_YALIGN_BOTTOM)
                     y = max(iScreenTop, y - nHeight);
                  else if (Fetch.wAlignY == FP_TT_YALIGN_CENTER)
                     y = max(iScreenTop, y - (nHeight / 2));
                  else if (Fetch.wAlignY == FP_TT_YALIGN_AUTO)
                     {
                     if (y + nHeight > nScreenHeight)
                        /* RFW - 4/19/00 - SCS9319
                        {
                        if (TT_StatusFloating(lpTT))
                           {
                           if (ptPos.y - Fetch.yOffset - TT_YFLOATINGOFFSET - nHeight >= 0)
                              y = ptPos.y - Fetch.yOffset - TT_YFLOATINGOFFSET - nHeight;
                           }

                        else if (Fetch.Rect.bottom - Fetch.yOffset - nHeight >= 0)
                           y = Fetch.Rect.bottom - Fetch.yOffset - nHeight;
                        }
                        */
                        y = max(0, nScreenHeight - nHeight);
                     }

                  x = min(x, nScreenWidth - Fetch.nWidth);
                  y = min(y, max(0, nScreenHeight - nHeight));
                  SetWindowPos(hWnd, HWND_TOPMOST, x, y, Fetch.nWidth,
                               min(nHeight, nScreenHeight - y), SWP_NOACTIVATE);
//                               nHeight, SWP_NOZORDER | SWP_NOACTIVATE);
                  ShowWindow(hWnd, SW_SHOWNOACTIVATE);
						// RFW - 8/9/04 - 15023 - SetCapture was added to fix
						// 14510, but it broke 15023, so I changed the capture to hWndParent.
						// RFW - 6/16/04 - 14510
						// SetCapture(lpTT->hWnd);
						SetCapture(lpTT->hWndParent);
                  }

               if (hFont)
                  {
                  SelectObject(hDC, hFontOld);
                  DeleteObject(hFont);
                  }

               ReleaseDC(hWnd, hDC);
               }


				if (Fetch.hText)
					{
					GlobalUnlock(Fetch.hText);
					GlobalFree(Fetch.hText);
					}
            }
         }

      lpTT->fTimer1Sent = FALSE;
      TT_Unlock(hWnd);
      return (0);

   case TTMx_MOUSEMOVE:
      lpTT = TT_Lock(hWnd);

      if (lpTT->hWndParent)
         {
         BOOL fShow;
// -> fix for GIC21056 -scl
         HWND hWndFocus = GetFocus();
         HWND hWndFocusCompare = GetProp(lpTT->hWndParent, TT_PROP_FOCUS);
#ifdef TT_V2
         BOOL       fTipModeChange = FALSE; //cellnote to texttip or back.
         FP_TT_SCAN Scan;

         if (2 == lpTT->ID.lID3) //ScrollTip
            return 0;
#endif
         fShow = (((HWND)wParam == lpTT->hWndParent || (HWND)wParam == hWnd) &&
                  IsWindowEnabled(lpTT->hWndParent) &&
                  TT_IsMouseInWindow(lpTT->hWndParent, lParam) &&
                  (!TT_StatusFocus(lpTT) || hWndFocus == lpTT->hWndParent || hWndFocus == hWndFocusCompare) &&
                  (!lpTT->bHideOnMouseDown || !TT_IsLButtonDown()));
// <- fix for GIC21056 -scl

         if (fShow)
            {
#ifndef TT_V2
            FP_TT_SCAN Scan;
#endif
            POINT      ptPos;

            GetCursorPos(&ptPos);
            ScreenToClient(lpTT->hWndParent, &ptPos);

            _fmemset(&Scan, '\0', sizeof(FP_TT_SCAN));
            Scan.nMouseX = (short)ptPos.x;
            Scan.nMouseY = (short)ptPos.y;

            SendMessage(lpTT->hWndParent, FPM_TT_SCAN, (WPARAM)hWnd,
                        (LPARAM)(LPVOID)&Scan);

#ifdef TT_V2
            if (Scan.ID.lID3 != lpTT->ID.lID3)
               fTipModeChange = TRUE;
#endif
            if (!TT_IsEqualID(&Scan.ID, &lpTT->ID))
               fShow = FALSE;

            TT_SetEqual(&lpTT->ID, &Scan.ID);
            }
         if (!fShow)
            {
            if (IsWindowVisible(hWnd))
               TT_Hide(lpTT, TT_HIDE_TILLMOUSE);

            if (lpTT->fTimer1Sent)
               {
               lpTT->fTimer1Sent = FALSE;
               KillTimer(hWnd, TT_TIMER1ID);
               }
#ifdef TT_V2
            if (fTipModeChange)
               {
               BOOL fDone = FALSE;

               TT_SetEqual(&lpTT->ID, &Scan.ID);
               if (lpTT->dwTickCount)
                  {
//                  if (GetTickCount() <= lpTT->dwTickCount +
//                      (DWORD)lpTT->lDelayNext)
                     {
                     PostMessage(hWnd, WM_TIMER, 0, 0);
                     lpTT->fTimer1Sent = TRUE;
                     fDone = TRUE;
                     }

                  lpTT->dwTickCount = 0;
                  }

               if (!lpTT->fTimer1Sent && !fDone)
                  {
                  SetTimer(hWnd, TT_TIMER1ID, (Scan.ID.lID3?50:(UINT)lpTT->lDelay), NULL);
                  lpTT->fTimer1Sent = TRUE;
                  }
               }
#endif
            }
         else if (fShow && !IsWindowVisible(hWnd))
            {
            BOOL fDone = FALSE;
            if (lpTT->dwTickCount)
               {
               if (GetTickCount() <= lpTT->dwTickCount +
                   (DWORD)lpTT->lDelayNext)
                  {
                  PostMessage(hWnd, WM_TIMER, 0, 0);
                  lpTT->fTimer1Sent = TRUE;
                  fDone = TRUE;
                  }

               lpTT->dwTickCount = 0;
               }

            if (!lpTT->fTimer1Sent && !fDone)
               {
               SetTimer(hWnd, TT_TIMER1ID, (UINT)lpTT->lDelay, NULL);
               lpTT->fTimer1Sent = TRUE;
               }
            }
         }

      TT_Unlock(hWnd);
      return (0);

   case TTMx_MOUSEDOWN:
      lpTT = TT_Lock(hWnd);

      if (lpTT->bHideOnMouseDown)
         TT_Hide(lpTT, lpTT->bHideOnMouseDown);

      TT_Unlock(hWnd);
      return (0);
#ifdef TT_V2
   case TTMx_MOUSEUP:
      lpTT = TT_Lock(hWnd);
      
      if (2 == lpTT->ID.lID3) //ScrollTip
      {
        TT_Hide(lpTT, lpTT->bHideOnMouseDown);
        lpTT->ID.lID3 = 0;
      }

      TT_Unlock(hWnd);
      return (0);
#endif
   case TTMx_KEYDOWN:
      lpTT = TT_Lock(hWnd);

      if (lpTT->bHideOnKeyDown)
         TT_Hide(lpTT, lpTT->bHideOnKeyDown);

      TT_Unlock(hWnd);
      return (0);

   case TTM_RESETHOOKS:
      TT_SetHooks(NULL);
	  return (0);
   }

return (DefWindowProc(hWnd, Msg, wParam, lParam));
}

typedef HWND FAR *LPHWND;


void TT_SetHooks(HWND hWnd)
{
   LPTEXTTIPHOOK lpTextTipHooks;
   DWORD dwThreadId = GetCurrentThreadId();

   if( hTextTipHooks )
   { // check whether this thead has a hook set -scl
      int i;
      BOOL fFound = FALSE;

	   lpTextTipHooks = ttGlobalLock(hTextTipHooks);

      for( i = 0; i < lHookCnt; i++ )
      {
         if( lpTextTipHooks[i].dwThreadId == dwThreadId )
         {
            fFound = TRUE;
            break;
         }
      }
      if( fFound )
      {
         if( hWnd ) // add hWnd to this hook's list -scl
         {
            if (lpTextTipHooks[i].lHookWndCnt == 0)
               lpTextTipHooks[i].hHookWndList = GlobalAlloc(GHND, sizeof(HWND) * (lpTextTipHooks[i].lHookWndCnt + 1));
            else
               lpTextTipHooks[i].hHookWndList = GlobalReAlloc(lpTextTipHooks[i].hHookWndList, sizeof(HWND) * (lpTextTipHooks[i].lHookWndCnt + 1), GHND);

            if (lpTextTipHooks[i].hHookWndList)
            {
               LPHWND lphWnd = (LPHWND)GlobalLock(lpTextTipHooks[i].hHookWndList);

               lphWnd[lpTextTipHooks[i].lHookWndCnt] = hWnd;
               GlobalUnlock(lpTextTipHooks[i].hHookWndList);
               lpTextTipHooks[i].lHookWndCnt++;
            }
            else
               lpTextTipHooks[i].lHookWndCnt = 0;
         }
	      ttGlobalUnlock(hTextTipHooks);
         return; // already set hook on this thead -scl
      }

      ttGlobalUnlock(hTextTipHooks);
   }

	if (TEXTTIPHOOKS_MAX == -1 || lHookCnt < TEXTTIPHOOKS_MAX)
	{
		// need to set hook on this thead
		if( lHookCnt == 0 )
			hTextTipHooks = ttGlobalAlloc(sizeof(TEXTTIPHOOK));
		else
			hTextTipHooks = ttGlobalReAlloc(hTextTipHooks, sizeof(TEXTTIPHOOK) * (lHookCnt + 1));

		if (hTextTipHooks)
		{
			lpTextTipHooks = ttGlobalLock(hTextTipHooks);
			lpTextTipHooks[lHookCnt].dwThreadId = dwThreadId;
		#if WIN32
			lpTextTipHooks[lHookCnt].hHookMouse = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)TT_MouseHook, 0, dwThreadId);
			lpTextTipHooks[lHookCnt].hHookKB = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)TT_KBHook, 0, dwThreadId);
		#else
			lpfnMouseHookProc = (HOOKPROC)MakeProcInstance((FARPROC)TT_MouseHook, fpInstance);
			lpfnKBHookProc = (HOOKPROC)MakeProcInstance((FARPROC)TT_KBHook, fpInstance);
			lpTextTipHooks[lHookCnt].hHookMouse = SetWindowsHookEx(WH_MOUSE, lpfnMouseHookProc, 0, dwThreadId);
			lpTextTipHooks[lHookCnt].hHookKB = SetWindowsHookEx(WH_KEYBOARD, lpfnKBHookProc, 0, dwThreadId);
		#endif
			lpTextTipHooks[lHookCnt].hHookWndList = GlobalAlloc(GHND, sizeof(HWND));
			if( lpTextTipHooks[lHookCnt].hHookWndList )
			{
				LPHWND lphWnd = (LPHWND)GlobalLock(lpTextTipHooks[lHookCnt].hHookWndList);
				lphWnd[0] = hWnd;
				GlobalUnlock(lpTextTipHooks[lHookCnt].hHookWndList);
				lpTextTipHooks[lHookCnt].lHookWndCnt = 1;
			}

			ttGlobalUnlock(hTextTipHooks);
			lHookCnt++;
		}
		else
			lHookCnt = 0;
	}
}


void TT_ReleaseHooks(HWND hWnd)
{
   LPHWND lphWnd;
   long  i;
   long  j;
   long  k;
   BOOL   fFoundHook = FALSE;
   BOOL   fFoundWnd = FALSE;
   LPTEXTTIPHOOK lpTextTipHooks;
   DWORD  dwThreadId = GetCurrentThreadId();
   
   lpTextTipHooks = ttGlobalLock(hTextTipHooks);
   if( !lpTextTipHooks )
      return;
   for( i = 0, fFoundHook = FALSE; i < lHookCnt && !fFoundHook; i++ )
   {
      if( lpTextTipHooks[i].dwThreadId == dwThreadId )
      {
         fFoundHook = TRUE;
         break;
      }
   }
   if( fFoundHook )
   {
      lphWnd = (LPHWND)GlobalLock(lpTextTipHooks[i].hHookWndList);
      if( !lphWnd )
         return;
      for (j = 0, fFoundWnd = FALSE; j < lpTextTipHooks[i].lHookWndCnt && !fFoundWnd; j++)
      {
         if (lphWnd[j] == hWnd)
         {
            for (k = j; k < lpTextTipHooks[i].lHookWndCnt - 1; k++)
               lphWnd[k] = lphWnd[k + 1];
            
            fFoundWnd = TRUE;
         }
      }
      
      GlobalUnlock(lpTextTipHooks[i].hHookWndList);
      
      if (fFoundWnd)
      {
         lpTextTipHooks[i].lHookWndCnt--;
         
         if (lpTextTipHooks[i].lHookWndCnt > 0)
            lpTextTipHooks[i].hHookWndList = GlobalReAlloc(lpTextTipHooks[i].hHookWndList, sizeof(HWND) * lpTextTipHooks[i].lHookWndCnt, GHND);
         else if (lpTextTipHooks[i].hHookWndList)
         {
            GlobalFree(lpTextTipHooks[i].hHookWndList);
            lpTextTipHooks[i].hHookWndList = NULL;
         }
         
         if( /*processId == GetCurrentProcessId() &&*/ lpTextTipHooks[i].lHookWndCnt == 0 )
         {
            if (lpTextTipHooks[i].hHookMouse)
            {
               UnhookWindowsHookEx(lpTextTipHooks[i].hHookMouse);
#ifndef WIN32
               FreeProcInstance(lpTextTipHooks[i].lpfnMouseHookProc);
#endif
               lpTextTipHooks[i].hHookMouse = 0;
            }
            
            if (lpTextTipHooks[i].hHookKB)
            {
               UnhookWindowsHookEx(lpTextTipHooks[i].hHookKB);
#ifndef WIN32
               FreeProcInstance(lpTextTipHooks[i].lpfnKBHookProc);
#endif
               lpTextTipHooks[i].hHookKB = 0;
            }
            
            if( i < lHookCnt - 1 )
               _fmemcpy(&lpTextTipHooks[i], &lpTextTipHooks[i+1], sizeof(TEXTTIPHOOK) * (lHookCnt - i - 1));
            lHookCnt--;
            ttGlobalUnlock(hTextTipHooks);
            if( lHookCnt == 0 )
            {
               ttGlobalFree(hTextTipHooks);
               hTextTipHooks = NULL;
            }
            else
               hTextTipHooks = ttGlobalReAlloc(hTextTipHooks, sizeof(TEXTTIPHOOK) * lHookCnt);

				return;
            //processId = 0;
            //{
            //   HWND temp = FindWindow(TT_CLASS, NULL);
            //   if( temp == hWnd )
            //	   temp = FindWindowEx(NULL, hWnd, TT_CLASS, NULL);
            //   if( temp != NULL )
            //	SendMessage(FindWindow(TT_CLASS, NULL), TTM_RESETHOOKS, 0, 0);
            //}
         }
         
      }
   }

   ttGlobalUnlock(hTextTipHooks);
}

LRESULT CALLBACK TT_MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
LPTEXTTIPHOOK lpTextTipHooks;
LPMOUSEHOOKSTRUCT lpMouseHookParam = (LPMOUSEHOOKSTRUCT)lParam;
//LRESULT           lRet;
static POINT      PtPrev = {0, 0};
DWORD dwThreadId = GetCurrentThreadId();
int i;
HHOOK hHookMouse;

lpTextTipHooks = (LPTEXTTIPHOOK)ttGlobalLock(hTextTipHooks);
for( i = 0; i < lHookCnt; i++ )
{
   if( lpTextTipHooks[i].dwThreadId == dwThreadId )
   {
      hHookMouse = lpTextTipHooks[i].hHookMouse;
      break;
   }
}
ttGlobalUnlock(hTextTipHooks);

// RFW - 8/20/03
if (nCode < HC_ACTION)
	return (CallNextHookEx(hHookMouse, nCode, wParam, lParam));

if (nCode >= 0 && (wParam == WM_MOUSEMOVE || wParam == WM_NCMOUSEMOVE))
   {
//   lRet = CallNextHookEx(hHookMouse, nCode, wParam, lParam);

   if (PtPrev.x == lpMouseHookParam->pt.x &&
       PtPrev.y == lpMouseHookParam->pt.y)
      //return (lRet);
	  return CallNextHookEx(hHookMouse, nCode, wParam, lParam);

//   if (lRet == 0 && lpMouseHookParam->wHitTestCode > 0)
   if (lpMouseHookParam->wHitTestCode > 0)
      TT_HookSendMessage(lpTextTipHooks[i].hHookWndList, lpTextTipHooks[i].lHookWndCnt, TTMx_MOUSEMOVE, (WPARAM)lpMouseHookParam->hwnd,
                          MAKELPARAM(lpMouseHookParam->pt.x,
                          lpMouseHookParam->pt.y));

   PtPrev = lpMouseHookParam->pt;
   }
else
   {
//   lRet = CallNextHookEx(hHookMouse, nCode, wParam, lParam);

//   if (lRet == 0 && nCode >= 0 && (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN ||
   if (nCode >= 0 && (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN ||
       wParam == WM_LBUTTONDBLCLK || wParam == WM_RBUTTONDBLCLK))
      TT_HookSendMessage(lpTextTipHooks[i].hHookWndList, lpTextTipHooks[i].lHookWndCnt, TTMx_MOUSEDOWN, wParam, (LPARAM)lpMouseHookParam->hwnd);

   /***********************************************
   * If the Mouse Button is released over another
   * control, then redisplay the balloon.
   ***********************************************/

//   else if (lRet == 0 && nCode >= 0 && (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP))
   else if (nCode >= 0 && (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP))
      {
      HWND  hWndFoundTemp;
      POINT Pt = lpMouseHookParam->pt;

      hWndFoundTemp = WindowFromPoint(Pt);
      if (hWndFoundTemp != lpMouseHookParam->hwnd)
         TT_HookSendMessage(lpTextTipHooks[i].hHookWndList, lpTextTipHooks[i].lHookWndCnt, TTMx_MOUSEMOVE, (WPARAM)hWndFoundTemp,
                             MAKELPARAM(Pt.x, Pt.y));
#ifdef TT_V2
      TT_HookSendMessage(lpTextTipHooks[i].hHookWndList, lpTextTipHooks[i].lHookWndCnt, TTMx_MOUSEUP, (WPARAM)hWndFoundTemp,
                         MAKELPARAM(Pt.x, Pt.y));
#endif
      }

//   lRet = CallNextHookEx(hHookMouse, nCode, wParam, lParam);
   }

//return (lRet);
return CallNextHookEx(hHookMouse, nCode, wParam, lParam);
}


/* RFW - 8/20/03
LRESULT _export CALLBACK TT_KBHook(int nCode, WPARAM wParam, DWORD lParam)
{
if (nCode >= 0)
   TT_HookSendMessage(TTMx_KEYDOWN, wParam, lParam);

return (CallNextHookEx(hHookKB, nCode, wParam, lParam));
}
*/


LRESULT CALLBACK TT_KBHook(int nCode, WPARAM wParam, LPARAM lParam)
{
LPTEXTTIPHOOK lpTextTipHooks;
DWORD dwThreadId = GetCurrentThreadId();
int i;
HHOOK hHookKB;
LRESULT lRet;

lpTextTipHooks = (LPTEXTTIPHOOK)ttGlobalLock(hTextTipHooks);
for( i = 0; i < lHookCnt; i++ )
{
   if( lpTextTipHooks[i].dwThreadId == dwThreadId )
   {
      hHookKB = lpTextTipHooks[i].hHookKB;
      break;
   }
}
ttGlobalUnlock(hTextTipHooks);

lRet = CallNextHookEx(hHookKB, nCode, wParam, lParam);

//	if( nCode < HC_ACTION )
//		return CallNextHookEx(hHookKB, nCode, wParam, lParam);
if (nCode == HC_ACTION)
   TT_HookSendMessage(lpTextTipHooks[i].hHookWndList, lpTextTipHooks[i].lHookWndCnt, TTMx_KEYDOWN, wParam, lParam);

return (lRet);
//return CallNextHookEx(hHookKB, nCode, wParam, lParam);
}


void TT_HookSendMessage(HGLOBAL hHookWndList, long lHookWndCnt, WORD wMsg, WPARAM wParam, LPARAM lParam)
{
if (hHookWndList)
   {
   LPHWND lphWnd = (LPHWND)GlobalLock(hHookWndList);
   long  i;

   for (i = 0; i < lHookWndCnt; i++)
		if (lphWnd[i])
			// RFW - 1/7/05 - 15362
			//SendMessage(lphWnd[i], wMsg, wParam, lParam);
			TT_Proc(lphWnd[i], wMsg, wParam, lParam);

   GlobalUnlock(hHookWndList);
   }
}


void TT_Hide(LPTEXTTIP lpTT, BYTE bHideOn)
{
if (IsWindowVisible(lpTT->hWnd))
   {
   MSG Msg;

	// RFW - 8/9/04 - 15023 - SetCapture was added to fix
	// 14510, but it broke 15023, so I changed the capture to hWndParent.
	// RFW - 6/16/04 - 14510
	//if (GetCapture() == lpTT->hWnd)
	//	ReleaseCapture();
	if (GetCapture() == lpTT->hWndParent)
		ReleaseCapture();

   if (bHideOn == TT_HIDE_TILLNEW)
      lpTT->IDHideTillNew = lpTT->ID;

   ShowWindow(lpTT->hWnd, SW_HIDE);
   TT_NullID(&lpTT->ID);
//   lpTT->dwTickCount = GetTickCount();

   while (PeekMessage(&Msg, 0, WM_PAINT, WM_PAINT, PM_REMOVE))
      {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
      }
   }
}


LPTEXTTIP TT_LockWithParent(HWND hWndParent)
{
HGLOBAL   hTT = GetProp(hWndParent, TT_PROP);
LPTEXTTIP lpTT;

if (!hTT)
   {
   if (hTT = GlobalAlloc(GHND, sizeof(TEXTTIP)))
      {
      lpTT = (LPTEXTTIP)GlobalLock(hTT);

      lpTT->hWndParent = hWndParent;
      lpTT->fActive = PROP_TRUE;
      lpTT->wMultiLine = FP_TT_MULTILINE_AUTO;
      lpTT->lDelay = 500;
      lpTT->lDelayNext = 500;
//Modify by BOC 99.7.19 (hyt)---------------------------
//for The ActiveX control uses the ToolTip colours as the 
//default colours for the background and text
#ifdef WIN32
      lpTT->clrBack = COLOR_INIT(COLOR_INFOBK);
      lpTT->clrFore = COLOR_INIT(COLOR_INFOTEXT);
#else
	  lpTT->clrBack = COLOR_INIT(COLOR_WINDOW);
      lpTT->clrFore = COLOR_INIT(COLOR_WINDOWTEXT);
#endif
//---------------------------------------------------
      lpTT->bHideOnMouseDown = TT_HIDE_TILLMOUSE;
      lpTT->bHideOnKeyDown = TT_HIDE_TILLMOUSE;

      lpTT->LogFont.lfWeight = FW_NORMAL;
      lpTT->LogFont.lfCharSet = DEFAULT_CHARSET;
	  //Modify by Boc 1999.4.26(hyt)------------------------
	  //for support japanese version
#ifdef WIN32
	  if((GetSystemDefaultLCID() & 0x1ff)==0x11)
	  {
		  lpTT->LogFont.lfHeight = PT_TO_PIXELS(9);
		  lstrcpy(lpTT->LogFont.lfFaceName, _T("‚l‚r ‚oƒSƒVƒbƒN"));
	  }
	  else
#endif
	  {
	      lpTT->LogFont.lfHeight =  PT_TO_PIXELS(8);
		  lstrcpy(lpTT->LogFont.lfFaceName, _T("MS Sans Serif"));
	  }
 
      SetProp(hWndParent, TT_PROP, hTT);
      }
   }

else
   lpTT = (LPTEXTTIP)GlobalLock(hTT);

return (lpTT);
}


void TT_UnlockWithParent(HWND hWndParent)
{
HGLOBAL hTT = GetProp(hWndParent, TT_PROP);

if (hTT)
   GlobalUnlock(hTT);
}


LPTEXTTIP TT_Lock(HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
return ((LPTEXTTIP)GlobalLock((HGLOBAL)GetWindowLongPtr(hWnd, 0)));
#else
return ((LPTEXTTIP)GlobalLock((HGLOBAL)GetWindowLong(hWnd, 0)));
#endif
}


void TT_Unlock(HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
GlobalUnlock((HGLOBAL)GetWindowLongPtr(hWnd, 0));
#else
GlobalUnlock((HGLOBAL)GetWindowLong(hWnd, 0));
#endif
}


void TT_Paint(HWND hWnd, HDC hDC, LPTEXTTIP lpTT, LPRECT lpRect)
{
HBRUSH hBrush;
HFONT  hFont;

#ifdef TT_V2
if (2 == lpTT->ID.lID3) // ScrollTip
  hBrush = CreateSolidBrush(0x00FFFFFF); //white
else
  hBrush = CreateSolidBrush(FPCOLOR(lpTT->clrBack));
#else
hBrush = CreateSolidBrush(FPCOLOR(lpTT->clrBack));
#endif

FillRect(hDC, lpRect, hBrush);
DeleteObject(hBrush);

if (lpTT->hText)
   {
   FPDRAWTEXT dt;
//   COLORREF   clrForeOld = SetTextColor(hDC, 0); //black
   COLORREF   clrForeOld = SetTextColor(hDC, FPCOLOR(lpTT->clrFore));
   LPTSTR     lpText = (LPTSTR)GlobalLock(lpTT->hText);
   HFONT      hFontOld;
   WORD       wFormat = 0;
    
   if (hFont = CreateFontIndirect(&lpTT->LogFont))
      hFontOld = SelectObject(hDC, hFont);

   _fmemset(&dt, '\0', sizeof(FPDRAWTEXT));

   if (lpTT->wMultiLine == FP_TT_MULTILINE_SINGLE)
      wFormat |= DT_SINGLELINE;
   else
      wFormat |= DT_WORDBREAK;

   wFormat |= DT_NOPREFIX;

   dt.fuStyle |= DTX_TRANSPARENT;

   fpDrawText(hWnd, hDC, lpText, -1, lpRect, wFormat, &dt);

   if (hFont)
      {
      SelectObject(hDC, hFontOld);
      DeleteObject(hFont);
      }

   GlobalUnlock(lpTT->hText);
   SetTextColor(hDC, clrForeOld);
   }
}


BOOL TT_IsMouseInWindow(HWND hWnd, LPARAM lParam)
{
RECT Rect;

GetWindowRect(hWnd, &Rect);

#ifdef WIN32
{
POINT pt;

/* RFW - 5/31/07 - 20536
pt.x = LOWORD(lParam);
pt.y = HIWORD(lParam);
*/
pt.x = (short)LOWORD(lParam);
pt.y = (short)HIWORD(lParam);

return PtInRect(&Rect, pt);
}
#else
return (PtInRect(&Rect, MAKEPOINT(lParam)));
#endif
}


HWND TT_GetParent(HWND hWnd)
{
return ((GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) ?
        GetParent(hWnd) : GetWindow(hWnd, GW_OWNER));
}



BOOL TT_IsDescendant(HWND hWndParent, HWND hWndChild)
// helper for detecting whether child descendent of parent
{
if (IsWindow(hWndParent) && IsWindow(hWndChild))
	{
	do
		{
		if (hWndParent == hWndChild)
			return TRUE;

		hWndChild = TT_GetParent(hWndChild);
		} while (hWndChild != NULL);
	}

return FALSE;
}


HWND TT_GetTopLevelWindow(HWND hWnd)
{
HWND hWndTop;

do
	{
	hWndTop = hWnd;
	hWnd = GetParent(hWnd);
	} while (hWnd != NULL);

return hWndTop;
}


BOOL TT_IsWindowSibling(HWND hWndSelf, HWND hWndFocus)
{
if (hWndSelf == hWndFocus || TT_IsDescendant(hWndSelf, hWndFocus))
	return (TRUE);

return (TT_IsDescendant(TT_GetTopLevelWindow(hWndSelf), hWndFocus));
}
