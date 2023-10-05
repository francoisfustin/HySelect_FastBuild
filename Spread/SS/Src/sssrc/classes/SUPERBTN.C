/*********************************************************
* SUPERBTN.C
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
#include "toolbox.h"
#include "fphdc.h"
#include "superbtn.h"
#include "wintools.h"
#ifdef SS_V80
#include "uxtheme.h"
#include "ss_theme.h"
#include <winuser.h>
#endif
#include "..\spread\ss_w32s.h"

#define DIVUP(Var1, Var2)  (((Var1) + (Var2) - 1) / (Var2))
#define RECTWIDTH(lpRect)  ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect) ((lpRect)->bottom - (lpRect)->top)

/**********************
* Function prototypes
**********************/

static LPSUPERBTN SuperBtnInit(GLOBALHANDLE FAR *hGlobal);
BOOL              SuperBtnSetText(HWND hWnd, LPTSTR Text);
static void       SuperBtnSetPtr(HWND hWnd, GLOBALHANDLE hGlobal);
void              SetDefPushButton(HWND hWnd);
BOOL DLLENTRY     SuperBtnFocusRect(HDC hDC, LPRECT lpRect, HBRUSH hBrush);
void              SuperBtnFreeStretch(LPSUPERBTN lpSuperBtn);
void              SuperBtnGetCurrColor(LPSUPERBTNCOLOR lpColor,
                                       LPSUPERBTN lpSuperBtn);

extern HANDLE hDynamicInst;

BOOL RegisterSuperBtn(hInstance)

HANDLE   hInstance;
{
WNDCLASS wc;

wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
wc.lpfnWndProc   = tbSuperBtnWndFn;
wc.cbClsExtra    = 0;
#if defined(_WIN64) || defined(_IA64)
wc.cbWndExtra    = 10;
#else
wc.cbWndExtra    = 6;
#endif
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = _T("tbSuperBtn");

if (!RegisterClass((LPWNDCLASS)&wc))
   return (FALSE);

return (TRUE);
}


static LPSUPERBTN SuperBtnInit(hGlobal)

GLOBALHANDLE FAR *hGlobal;
{
LPSUPERBTN        lpSuperBtn;
HBITMAP           hBitmap;
/*
static WORD FAR   Bits[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
*/

if (!(*hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                             (long)sizeof(SUPERBTN))))
   return (NULL);

lpSuperBtn = (LPSUPERBTN)GlobalLock(*hGlobal);

lpSuperBtn->ShadowSize           = 2;
lpSuperBtn->BtnFrameOffset       = 3;
lpSuperBtn->ButtonType           = SUPERBTN_TYPE_NORMAL;
lpSuperBtn->Color.Color          = RGBCOLOR_DEFAULT;
lpSuperBtn->Color.ColorBorder    = RGBCOLOR_DEFAULT;
lpSuperBtn->Color.ColorShadow    = RGBCOLOR_DEFAULT;
#if (WINVER >= 0x030a)
lpSuperBtn->Color.ColorHighlight = RGBCOLOR_DEFAULT;
#else
lpSuperBtn->Color.ColorHighlight = RGBCOLOR_WHITE;
#endif
lpSuperBtn->Color.ColorText      = RGBCOLOR_DEFAULT;

#ifndef SS_V21
{
WORD wLoWordVersion = LOWORD(GetVersion());

lpSuperBtn->fIs95 = (HIBYTE(wLoWordVersion) >= 95 ||
                     LOBYTE(wLoWordVersion) >= 4);
}
#endif
#ifdef SS_V80
lpSuperBtn->Enhanced = FALSE;
#endif

hBitmap = CreateBitmap(8, 8, 1, 1,
          "\x55\x55\xaa\xaa\x55\x55\xaa\xaa\x55\x55\xaa\xaa\x55\x55\xaa\xaa");
lpSuperBtn->hBrushFocus = CreatePatternBrush(hBitmap);
DeleteObject(hBitmap);

return (lpSuperBtn);
}


WINENTRY tbSuperBtnWndFn(hWnd, Msg, wParam, lParam)

HWND           hWnd;
UINT           Msg;
WPARAM         wParam;
LPARAM         lParam;
{
GLOBALHANDLE   hGlobal;
LPSUPERBTN     lpSuperBtn;
LPCREATESTRUCT lpcsData;
PAINTSTRUCT    Paint;
HBITMAP        hBitmapOld;
HFONT          hFont;
LPTSTR         Text;
RECT           Rect;
HDC            hDCMemory;
HDC            hDC;
long           lRet;
short          Len;
POINT          ptPos;

switch (Msg)
   {
   case WM_CREATE:
      lpSuperBtn = SuperBtnInit(&hGlobal);
      SuperBtnSetPtr(hWnd, hGlobal);

      lpSuperBtn->lStyle = GetWindowLong(hWnd, GWL_STYLE);

      lpcsData = (LPCREATESTRUCT)lParam;
      SuperBtnSetText(hWnd, (LPTSTR)lpcsData->lpszName);

      Rect.left = lpcsData->x;
      Rect.top = lpcsData->y;
      Rect.right = lpcsData->x + lpcsData->cx;
      Rect.bottom = lpcsData->y + lpcsData->cy;

      /*******************************************************
      * Create Bitmap to select to the memory device context
      *******************************************************/

      hDC = fpGetDC(hWnd);
      lpSuperBtn->hBitmapDC = CreateCompatibleBitmap(hDC, Rect.right -
                                                     Rect.left,
                                                     Rect.bottom - Rect.top);
      ReleaseDC(hWnd, hDC);

      GlobalUnlock(hGlobal);
      return (0);

   case WM_GETDLGCODE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      if (lpSuperBtn->lStyle & SBS_DEFPUSHBUTTON)
         lRet = DLGC_DEFPUSHBUTTON;

      /*
      else if ((lpSuperBtn->lStyle & SBS_NOPOINTERFOCUS) && GetFocus() != hWnd)
         lRet = DLGC_STATIC;
      */
      else
         lRet = DLGC_UNDEFPUSHBUTTON;

      lRet |= DLGC_BUTTON;

      GlobalUnlock(hGlobal);
      return (lRet);

   case WM_WININICHANGE:
      InvalidateRect(hWnd, NULL, TRUE);
      break;

#ifdef SS_WIN32S
   case BM_SETSTYLE_WIN32S:
#endif
   case BM_SETSTYLE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      /*
      if (!(lpSuperBtn->lStyle & SBS_NOPOINTERFOCUS))
      */
         {
         lpSuperBtn->lStyle &= ~SBS_DEFPUSHBUTTON;

         if ((long)wParam == BS_DEFPUSHBUTTON)
            lpSuperBtn->lStyle |= SBS_DEFPUSHBUTTON;
         }

      GlobalUnlock(hGlobal);

      if (lParam)
         InvalidateRect(hWnd, NULL, TRUE);

      return (0);

   case SBM_SETCOLOR:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      _fmemcpy(&lpSuperBtn->Color, (LPSUPERBTNCOLOR)lParam,
               sizeof(SUPERBTNCOLOR));
      SuperBtnFreeStretch(lpSuperBtn);
      GlobalUnlock(hGlobal);

      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      return (TRUE);

   case SBM_GETCOLOR:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      _fmemcpy((LPSUPERBTNCOLOR)lParam, &lpSuperBtn->Color,
               sizeof(SUPERBTNCOLOR));
      GlobalUnlock(hGlobal);
      return (TRUE);

   case SBM_SETPICT:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lRet = SuperBtnSetPict(hWnd, 0, &lpSuperBtn->Pict, (char)wParam, lParam);
      GlobalUnlock(hGlobal);
      return (lRet);

   case SBM_SETPICT_DISABLED:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lRet = SuperBtnSetPict(hWnd, 0, &lpSuperBtn->PictDisabled, (char)wParam,
                             lParam);
      GlobalUnlock(hGlobal);
      return (lRet);

   case SBM_SETPICT_BTNDOWN:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lRet = SuperBtnSetPict(hWnd, 0, &lpSuperBtn->PictBtnDown, (char)wParam,
                             lParam);
      GlobalUnlock(hGlobal);
      return (lRet);

   case SBM_SETBUTTONTYPE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lpSuperBtn->ButtonType = (short)wParam;
      GlobalUnlock(hGlobal);
      return (0);

   case SBM_SETBUTTONSTATE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      if (lpSuperBtn->ButtonDown != (short)wParam)
         {
         lpSuperBtn->ButtonDown = (BOOL)wParam;
         InvalidateRect(hWnd, NULL, FALSE);
         UpdateWindow(hWnd);

         FORWARD_WM_COMMAND(hWnd, GetDlgCtrlID(hWnd), hWnd, BN_CLICKED,
                            tbSendMessageToParent);
         }

      GlobalUnlock(hGlobal);
      return (0);

#ifdef SS_WIN32S
   case BM_SETSTATE_WIN32S:
#endif
   case BM_SETSTATE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      if (lpSuperBtn->ButtonDown != (short)wParam)
         {
         lpSuperBtn->ButtonDown = (BOOL)wParam;
         InvalidateRect(hWnd, NULL, FALSE);
         UpdateWindow(hWnd);
         }

      GlobalUnlock(hGlobal);
      return (0);

#ifdef SS_WIN32S
   case BM_GETSTATE_WIN32S:
#endif
   case BM_GETSTATE:
   case SBM_GETBUTTONSTATE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lRet = (long)lpSuperBtn->ButtonDown;
      GlobalUnlock(hGlobal);
      return (lRet);

   case SBM_SETSHADOWSIZE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lpSuperBtn->ShadowSize = (short)wParam;
      GlobalUnlock(hGlobal);
      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      return (0);

   case SBM_GETSHADOWSIZE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lRet = (long)lpSuperBtn->ShadowSize;
      GlobalUnlock(hGlobal);
      return (lRet);

   case SBM_SETSTRETCH:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lpSuperBtn->fStretch = (BOOL)wParam;
      SuperBtnFreeStretch(lpSuperBtn);
      GlobalUnlock(hGlobal);

      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      return (TRUE);

   case SBM_GETSTRETCH:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lRet = (long)lpSuperBtn->fStretch;
      GlobalUnlock(hGlobal);
      return (lRet);

   case WM_SIZE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      GetClientRect(hWnd, &Rect);

      SuperBtnFreeStretch(lpSuperBtn);

      if (lpSuperBtn->hBitmapDC)
         DeleteObject(lpSuperBtn->hBitmapDC);

      /*******************************************************
      * Create Bitmap to select to the memory device context
      *******************************************************/

      hDC = fpGetDC(hWnd);
      lpSuperBtn->hBitmapDC = CreateCompatibleBitmap(hDC, Rect.right -
                                                     Rect.left, Rect.bottom -
                                                     Rect.top);
      ReleaseDC(hWnd, hDC);

      GlobalUnlock(hGlobal);
      break;

   case WM_CLOSE:
      DestroyWindow(hWnd);
      return (0);

   /*********************************************************
   * This message is received when the window is destroyed,
   * either under program control or external direction
   *********************************************************/

	/* RFW - 5/12/06
   case WM_DESTROY:
	*/
   case WM_NCDESTROY:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      DeleteObject(lpSuperBtn->hBrushFocus);

      if (lpSuperBtn->hText)
         GlobalFree(lpSuperBtn->hText);

      SuperBtnFreePict(&lpSuperBtn->Pict);
      SuperBtnFreePict(&lpSuperBtn->PictDisabled);
      SuperBtnFreePict(&lpSuperBtn->PictBtnDown);

      if (lpSuperBtn->hBitmapDC)
         DeleteObject(lpSuperBtn->hBitmapDC);

      GlobalUnlock(hGlobal);
      GlobalFree(hGlobal);

      return (0);

   case WM_SETFONT:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      lpSuperBtn->hFont = (HFONT)wParam;
      SuperBtnFreeStretch(lpSuperBtn);
      GlobalUnlock(hGlobal);

      if (lParam)
         {
         InvalidateRect(hWnd, NULL, TRUE);
         UpdateWindow(hWnd);
         }

      return (0);

   case WM_GETFONT:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      if (lpSuperBtn->hFont)
         hFont = lpSuperBtn->hFont;

      else
         {
         hDC = fpGetDC(hWnd);
         hFont = SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
         SelectObject(hDC, hFont);
         ReleaseDC(hWnd, hDC);
         }

      GlobalUnlock(hGlobal);
      return (LRESULT)hFont;

   case WM_KEYDOWN:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
#ifdef SPREAD_JPN
      if (wParam == VK_SPACE && !lpSuperBtn->MouseCaptureOn
		// BUG SPR_JPN_001_018 (1-1)
		// A Standard Button Does Not Respond To Space Key When It Is In Japanese IME Mode. 
		// (It Discards IME Status Window, But Still Allow Hot Keys To Change IME Mode)
		// Button In Spread Should Also Disable ButtonClicked Event When In Japanese IME Mode.
		// But ButtonClicked Event Will Be Fired At The First Time.
		// This Is Because At First Time, Keyborad Messages Are Sent To Main Spread Window,
		// But Are Not Sent To Button Window Directly As It Does Later.
		// Then Spread Window Sends WM_KEYDOWN To Button Window When Disposing WM_CHAR Message,
		// And It Use The Same Parameters Of WM_CHAR. (In SSx_CellEditModeOn() In SS_EMODE.C)
		// But In Japanese IME Mode, WM_KEYDOWN And WM_CHAR Have Different Parameters.
		// Here We Judge Whether It Is In Japanese IME Mode Or Not,
		// If In Japanese IME Mode, Do Nothing Even A Spece Key Is Send From WM_KEYDOWN.
		// Modified By HaHa 1999.11.5
		&& !ImmGetOpenStatus(ImmGetContext(hWnd)))
#else
      if (wParam == VK_SPACE && !lpSuperBtn->MouseCaptureOn)
#endif

         {
         if (lpSuperBtn->ButtonType == SUPERBTN_TYPE_NORMAL &&
             !lpSuperBtn->ButtonDown)
            {
            lpSuperBtn->ButtonDown = TRUE;
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
            }

         else if (lpSuperBtn->ButtonType == SUPERBTN_TYPE_2STATE)
            {
            lpSuperBtn->ButtonDown = !lpSuperBtn->ButtonDown;

            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);

            FORWARD_WM_COMMAND(hWnd, GetDlgCtrlID(hWnd), hWnd, BN_CLICKED,
                               tbSendMessageToParent);
            }
         }

      GlobalUnlock(hGlobal);
      return (0);

   case WM_KEYUP:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      if (wParam == VK_SPACE && !lpSuperBtn->MouseCaptureOn &&
          lpSuperBtn->ButtonDown &&
          lpSuperBtn->ButtonType == SUPERBTN_TYPE_NORMAL)
         {
         lpSuperBtn->ButtonDown = FALSE;
         InvalidateRect(hWnd, NULL, FALSE);
         UpdateWindow(hWnd);

         FORWARD_WM_COMMAND(hWnd, GetDlgCtrlID(hWnd), hWnd, BN_CLICKED,
                            tbSendMessageToParent);
         }

      GlobalUnlock(hGlobal);
      return (0);

   case WM_MOUSEACTIVATE:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      if (!(lpSuperBtn->lStyle & SBS_NOPOINTERFOCUS))
         {
         SetFocus(hWnd);
         SetDefPushButton(hWnd);
         }

      GlobalUnlock(hGlobal);
      return (0);

   case WM_LBUTTONDOWN:
   case WM_LBUTTONDBLCLK:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      if (!lpSuperBtn->MouseCaptureOn &&
          lpSuperBtn->ButtonType == SUPERBTN_TYPE_NORMAL)
         {
         SetCapture(hWnd);
         lpSuperBtn->MouseCaptureOn = TRUE;
         lpSuperBtn->ButtonDown = TRUE;

         InvalidateRect(hWnd, NULL, FALSE);
         UpdateWindow(hWnd);
         }

      else if (!lpSuperBtn->MouseCaptureOn &&
          lpSuperBtn->ButtonType == SUPERBTN_TYPE_2STATE)
         {
         lpSuperBtn->ButtonDown = !lpSuperBtn->ButtonDown;

         InvalidateRect(hWnd, NULL, FALSE);
         UpdateWindow(hWnd);

         
         FORWARD_WM_COMMAND(hWnd, GetDlgCtrlID(hWnd), hWnd, BN_CLICKED,
                            tbSendMessageToParent);
         }

      GlobalUnlock(hGlobal);
      return (0);

   case WM_LBUTTONUP:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      if (lpSuperBtn->MouseCaptureOn &&
          lpSuperBtn->ButtonType == SUPERBTN_TYPE_NORMAL)
         {
         lpSuperBtn->ButtonDown = FALSE;
         GetClientRect(hWnd, &Rect);

         lpSuperBtn->MouseCaptureOn = FALSE;
         ReleaseCapture();

         ptPos.x = LOWORD(lParam);
         ptPos.y = HIWORD(lParam);
         if (PtInRect(&Rect, ptPos))
            {
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);

            FORWARD_WM_COMMAND(hWnd, GetWindowID(hWnd), hWnd, BN_CLICKED,
                               tbSendMessageToParent);
            }
         }

      if (IsWindow(hWnd))
         GlobalUnlock(hGlobal);

      return (0);
#ifdef SS_V80
   case WM_MOUSELEAVE:
       lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
	   lpSuperBtn->MouseOver = FALSE;
       GlobalUnlock(hGlobal);
	   InvalidateRect(hWnd, NULL, FALSE);
	   break;
#endif

   case WM_MOUSEMOVE:
	   {

#ifdef SS_V80
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
#endif
      if (wParam & MK_LBUTTON)
         {
         lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

         if (lpSuperBtn->MouseCaptureOn)
            {
            GetClientRect(hWnd, &Rect);
            
            ptPos.x = LOWORD(lParam);
            ptPos.y = HIWORD(lParam);
            if (PtInRect(&Rect, ptPos))
               {
               if (!lpSuperBtn->ButtonDown)
                  {
                  lpSuperBtn->ButtonDown = TRUE;
                  InvalidateRect(hWnd, NULL, FALSE);
                  UpdateWindow(hWnd);
                  }
               }

            else
               {
               if (lpSuperBtn->ButtonDown)
                  {
                  lpSuperBtn->ButtonDown = FALSE;
                  InvalidateRect(hWnd, NULL, FALSE);
                  UpdateWindow(hWnd);
                  }
               }
            }

         GlobalUnlock(hGlobal);
         }
	  }
      break;

   case WM_SETFOCUS:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      if (lpSuperBtn->ButtonType == SUPERBTN_TYPE_NORMAL)
         lpSuperBtn->ButtonDown = FALSE;

      GlobalUnlock(hGlobal);
      InvalidateRect(hWnd, NULL, FALSE);
      return (0);

   case WM_KILLFOCUS:
      InvalidateRect(hWnd, NULL, TRUE);
      return (0);

   case WM_ENABLE:
      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      return (0);

   case WM_SETTEXT:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);
      SuperBtnFreeStretch(lpSuperBtn);
      SuperBtnSetText(hWnd, (LPTSTR)lParam);
      GlobalUnlock(hGlobal);
      return (0);

   case WM_GETTEXT:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      if (lpSuperBtn->hText)
         {
         Text = (LPTSTR)GlobalLock(lpSuperBtn->hText);
         Len = lstrlen(Text);
         GlobalUnlock(lpSuperBtn->hText);

         Len = min(Len + 1, (short)wParam);
         _fmemcpy((LPTSTR)lParam, Text, Len * sizeof(TCHAR));
         }

      else
         Len = 0;

      GlobalUnlock(hGlobal);
      return (Len);

   case WM_GETTEXTLENGTH:
      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      if (lpSuperBtn->hText)
         {
         Text = (LPTSTR)GlobalLock(lpSuperBtn->hText);
         Len = lstrlen(Text);
         GlobalUnlock(lpSuperBtn->hText);
         }

      else
         Len = 0;

      GlobalUnlock(hGlobal);
      return (Len);

   case WM_ERASEBKGND:
      return (TRUE);

   case WM_PAINT:
      hDC = fpBeginPaint(hWnd, &Paint);

      lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

      /*********************************
      * Create a memory device context
      *********************************/

      hDCMemory = CreateCompatibleDC(hDC);
      hBitmapOld = SelectObject(hDCMemory, lpSuperBtn->hBitmapDC);

      SetMapMode(hDCMemory, MM_TEXT);
      SetBkMode(hDCMemory, TRANSPARENT);

      GetClientRect(hWnd, &Rect);
      SuperBtnPaint(hWnd, lpSuperBtn, hDCMemory, &Rect, IsWindowEnabled(hWnd),
                    GetFocus() == hWnd, TRUE, FALSE, NULL, TRUE);
      GlobalUnlock(hGlobal);

      /*******************************************************
      * Copy the memory device context bitmap to the display
      *******************************************************/

      BitBlt(hDC, Rect.left, Rect.top, Rect.right - Rect.left,
             Rect.bottom - Rect.top, hDCMemory,  0, 0, SRCCOPY);

      /***********
      * Clean up
      ***********/

      SelectObject(hDCMemory, hBitmapOld);
      DeleteDC(hDCMemory);

      EndPaint(hWnd, &Paint);
      return (0);
   }

return (DefWindowProc(hWnd, Msg, wParam, lParam));
}


void SuperBtnFreePict(Pict)

LPTBBTNPICT Pict;
{
if (Pict->hPict && !Pict->HandleProvided)
   {
   if (Pict->PictType == SUPERBTN_PICT_BITMAP)
      DeleteObject(Pict->hPict);
   else if (Pict->PictType == SUPERBTN_PICT_ICON)
      DestroyIcon(Pict->hPict);
   }

if (Pict->hPictStretch)
   DeleteObject(Pict->hPictStretch);
}


BOOL SuperBtnSetText(hWnd, Text)

HWND         hWnd;
LPTSTR       Text;
{
GLOBALHANDLE hGlobal;
GLOBALHANDLE hTextTemp;
LPSUPERBTN   lpSuperBtn;
LPTSTR       TextTemp;
long         Style;
short        At;
short        i;

if (!Text)
   Text = _T("");

if (!(hTextTemp = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                              (lstrlen(Text) + 1) * sizeof(TCHAR))))
   return (FALSE);

lpSuperBtn = SuperBtnGetPtr(hWnd, &hGlobal);

TextTemp = (LPTSTR)GlobalLock(hTextTemp);

Style = lpSuperBtn->lStyle;
if ((Style & SBS_TEXTICON) || (Style & SBS_TEXTBITMAP))
   {
   for (i = 0, At = 0; i < lstrlen(Text); i++)
      if (Text[i] != '&')
         TextTemp[At++] = Text[i];

   if (Style & SBS_TEXTICON)
      SendMessage(hWnd, SBM_SETPICT, SUPERBTN_PICT_ICON, (LPARAM)TextTemp);

   else if (Style & SBS_TEXTBITMAP)
      SendMessage(hWnd, SBM_SETPICT, SUPERBTN_PICT_BITMAP, (LPARAM)TextTemp);
   }

else
   Style |= SBS_TEXTTEXT;

if (lpSuperBtn->hText)
   {
   GlobalFree(lpSuperBtn->hText);
   lpSuperBtn->hText = 0;
   }

if (Style & SBS_TEXTTEXT)
   {
   lstrcpy(TextTemp, Text);
   GlobalUnlock(hTextTemp);
   lpSuperBtn->hText = hTextTemp;
   }
else
   {
   GlobalUnlock(hTextTemp);
   GlobalFree(hTextTemp);
   }

GlobalUnlock(hGlobal);
return (TRUE);
}


void SuperBtnPaint(HWND hWnd, LPSUPERBTN lpSuperBtn, HDC hDC, LPRECT lpRect,
                   BOOL fIsEnabled, BOOL fFocus, BOOL fRoundCorners,
                   BOOL fUseDib, LPRECT lpRectOrig, BOOL fIsButton)
{
SUPERBTNCOLOR Color;
GLOBALHANDLE  hTextTemp;
TEXTMETRIC    fm;
COLORREF      clrText;
COLORREF      clrBk;
BITMAP        bm;
HBRUSH        hBrush;
HBRUSH        hBrushOld;
HBRUSH        hBrushBackground;
HPEN          hPenBorder;
HPEN          hPenOld;
HFONT         hFontOld = 0;
HANDLE        hDIB;
HPALETTE      hPalette;
HBITMAP       hPict;
LPTBBTNPICT   lpPict;
RECT          Rect;
RECT          RectTemp;
RECT          RectPict;
RECT          RectText;
LPTSTR        Text;
LPTSTR        TextTemp;
long          Style;
BOOL          ButtonIn;
BOOL          fDeletePict = FALSE;
double        dfScaleX;
double        dfScaleY;
short         BorderSize = 0;
short         TextWidth;
short         TextHeight;
short         PictWidth;
short         PictHeight;
short         PictWidthNew;
short         PictHeightNew;
short         PictWidthOrig;
short         PictHeightOrig;
short         Len;
short         At;
short         x;
short         y;
short         xOrig;
short         yOrig;
short         i;
int           iMapModeOld;
SIZE          Size;
#ifdef SS_V80
int prevBkMode;
HTHEME hTheme = NULL;
BOOL visualStyles = lpSuperBtn->Enhanced && SS_IsThemeActive();
#endif
CopyRect(&Rect, lpRect);



Style = lpSuperBtn->lStyle;

#ifndef SS_V70
Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);
Style |= SSS_ALIGN_CENTER;
Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
Style |= SSS_ALIGN_VCENTER;
#endif // SS_V70

/*************************************
* Fill the background with the
* current background of the display
*************************************/
SuperBtnGetCurrColor(&Color, lpSuperBtn);
clrText = SetTextColor(hDC, Color.ColorText);
clrBk = SetBkColor(hDC, Color.Color);
if (lpSuperBtn->hFont)
   hFontOld = SelectObject(hDC, lpSuperBtn->hFont);

#ifdef SS_V80
if (visualStyles)
{
int nFlags = 0;
LPWSTR lpWStr = NULL;
int oldDC = SaveDC(hDC);
int nlen = 0;
tbShrinkRect(&Rect, 1, 1);
if (fIsButton)
	hTheme = SS_OpenThemeData(hWnd, L"Button");
else
	hTheme = SS_OpenThemeData(hWnd, L"ComboBox");
SS_DrawThemeBackground( hTheme, hDC, 1, lpSuperBtn->ButtonDown ? 3 : lpSuperBtn->MouseOver ? 2 : 1, &Rect, &Rect);
/*if (lpSuperBtn->hText)
   {
	Text = (LPTSTR)GlobalLock(lpSuperBtn->hText);
	Len = lstrlen(Text);
// 22880 -scl
#ifndef _UNICODE
	nlen = MultiByteToWideChar(CP_ACP, 0, Text, -1, NULL, 0);
    //lpWStr = (LPWSTR) SysAllocStringLen(NULL, nlen - 1);
    lpWStr = (LPWSTR)malloc(nlen * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, Text, Len, lpWStr, nlen);
#else
   lpWStr = (LPWSTR)Text;
#endif

	if (Style & SSS_ALIGN_LEFT)
	 nFlags = DT_LEFT;
	else if (Style & SSS_ALIGN_RIGHT)
	 nFlags = DT_RIGHT;
	else 
	 nFlags = DT_CENTER;

	if (Style & SSS_ALIGN_TOP)
	 nFlags |= DT_TOP;
	else if (Style & SSS_ALIGN_BOTTOM)
	 nFlags |= DT_BOTTOM;
	else
	 nFlags |= DT_VCENTER;

	nFlags |= DT_SINGLELINE;

	DrawThemeText(hTheme, hDC, 1, lpSuperBtn->ButtonDown ? 3 : lpSuperBtn->MouseOver ? 2 : 1, lpWStr,Len, nFlags, !fIsEnabled ? DTT_GRAYED : 0, &Rect);
	if (lpSuperBtn->hText)
	   GlobalUnlock(lpSuperBtn->hText);
	//SysFreeString(lpWStr);
// 22880 -scl
#ifndef _UNICODE 
	free(lpWStr);
#endif

}*/

#ifdef SS_V80
if (visualStyles)
{
SS_CloseThemeData(hTheme);
RestoreDC(hDC, oldDC);
prevBkMode = SetBkMode(hDC, TRANSPARENT);
}
#endif
if (!fIsButton)
  return;	
}
else
{
#endif
hBrushBackground = CreateSolidBrush(Color.Color);
FillRect(hDC, &Rect, hBrushBackground);
DeleteObject(hBrushBackground);


/***********************
* Paint Outside border
***********************/
if (!lpSuperBtn->fIs95)
   BorderSize = 1;

if (Style & SBS_DEFPUSHBUTTON)
   BorderSize++;

if (BorderSize)
   {
   hBrushOld = SelectObject(hDC, GetStockObject(NULL_BRUSH));
   hPenBorder = CreatePen(PS_SOLID, 1, Color.ColorBorder);
   hPenOld = SelectObject(hDC, hPenBorder);

   for (i = 0; i < BorderSize; i++)
      {
      if (fRoundCorners)
         RoundRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom, 2, 2);
      else
         Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

      tbShrinkRect(&Rect, 1, 1);
      }

   SelectObject(hDC, hBrushOld);
   SelectObject(hDC, hPenOld);
   DeleteObject(hPenBorder);
   }

/**************
* Draw Shadow
**************/

if (lpSuperBtn->fIs95)
   {
   short nSize = max(1, lpSuperBtn->ShadowSize / 2);

   if (lpSuperBtn->ButtonDown)
      tbDrawShadows(hDC, &Rect, nSize, Color.ColorShadow,
                    Color.ColorShadow, FALSE, FALSE);

   else
      {
#ifdef SS_V35
      tbDrawShadows(hDC, &Rect, nSize, RGBCOLOR_BLACK, Color.ColorHighlight,
                    FALSE, TRUE);
      tbShrinkRect(&Rect, nSize, nSize);
      nSize = max(1, DIVUP(lpSuperBtn->ShadowSize, 2));
      tbDrawShadows(hDC, &Rect, nSize, Color.ColorShadow, Color.Color,
                    FALSE, TRUE);
#else
      tbDrawShadows(hDC, &Rect, nSize, RGBCOLOR_BLACK, Color.Color,
                    FALSE, TRUE);
      tbShrinkRect(&Rect, nSize, nSize);
      nSize = max(1, DIVUP(lpSuperBtn->ShadowSize, 2));
      tbDrawShadows(hDC, &Rect, nSize, Color.ColorShadow, Color.ColorHighlight,
                    FALSE, TRUE);
#endif
      }

   tbShrinkRect(&Rect, nSize, nSize);
   }

else
   {
   tbDrawShadows(hDC, &Rect, lpSuperBtn->ShadowSize,
                 Color.ColorShadow, Color.ColorHighlight,
                 (BOOL)(lpSuperBtn->ButtonDown ? 1 : 0), TRUE);

   tbShrinkRect(&Rect, lpSuperBtn->ShadowSize, lpSuperBtn->ShadowSize);
   }

/*************
* Draw Frame
*************/

if ((Style & SBS_BTNFRAMEIN) || (Style & SBS_BTNFRAMEOUT))
   {
   tbShrinkRect(&Rect, lpSuperBtn->BtnFrameOffset, lpSuperBtn->BtnFrameOffset);

   /************************
   * Draw Shadow for Frame
   ************************/

   ButtonIn = (BOOL)((Style & SBS_BTNFRAMEOUT) ? 0 : 1);
   if (!(Style & SBS_NOINVERTFRAME) && lpSuperBtn->ButtonDown)
      ButtonIn = !ButtonIn;

   tbDrawShadows(hDC, &Rect, lpSuperBtn->ShadowSize,
                 Color.ColorShadow,
                 Color.ColorHighlight, ButtonIn, FALSE);

   tbShrinkRect(&Rect, lpSuperBtn->ShadowSize, lpSuperBtn->ShadowSize);
   }


#ifdef SS_V80
}
#endif

if (lpSuperBtn->hFont)
   hFontOld = SelectObject(hDC, lpSuperBtn->hFont);
/**********************************
* Determine Picture and Text size
**********************************/

if (lpSuperBtn->hText)
   {
   Text = (LPTSTR)GlobalLock(lpSuperBtn->hText);
   Len = lstrlen(Text);
   }
else
	{
	Text = _T("");
   Len = 0;
	}

_fmemcpy(&RectPict, &Rect, sizeof(RECT));
_fmemcpy(&RectText, &Rect, sizeof(RECT));

lpPict = &lpSuperBtn->Pict;

if (!fIsEnabled && lpSuperBtn->PictDisabled.PictType)
   lpPict = &lpSuperBtn->PictDisabled;

else if (lpSuperBtn->ButtonDown && lpSuperBtn->PictBtnDown.PictType)
   lpPict = &lpSuperBtn->PictBtnDown;

GetTextMetrics(hDC, &fm);

if (fUseDib)
   {
   dfScaleX = (double)(lpRect->right - lpRect->left) /
              (double)(lpRectOrig->right - lpRectOrig->left);

   dfScaleY = (double)(lpRect->bottom - lpRect->top) /
              (double)(lpRectOrig->bottom - lpRectOrig->top);

   PictWidthNew = (short)(lpPict->PictWidth * dfScaleX);
   PictHeightNew = (short)(lpPict->PictHeight * dfScaleY);
   }

else
   {
   PictWidthNew = lpPict->PictWidth;
   PictHeightNew = lpPict->PictHeight;
   }

GetTextExtentPoint(hDC, Text, lstrlen(Text), &Size);
TextWidth = (short)Size.cx;
if ((Style & SBS_TEXTFRAMEIN) || (Style & SBS_TEXTFRAMEOUT))
   TextWidth += 8;

PictWidth = PictWidthNew;
if ((Style & SBS_PICTFRAMEIN) || (Style & SBS_PICTFRAMEOUT))
   PictWidth += 8;

TextHeight = (short)fm.tmHeight;
if ((Style & SBS_TEXTFRAMEIN) || (Style & SBS_TEXTFRAMEOUT))
   TextHeight += 8;

/* RFW - 4/20/05 - 16043
PictHeight = lpPict->PictHeight;
*/
PictHeight = PictHeightNew;
if ((Style & SBS_PICTFRAMEIN) || (Style & SBS_PICTFRAMEOUT))
   PictHeight += 8;

RectText.right = min(RectText.right, RectText.left + TextWidth);
RectPict.right = min(RectPict.right, RectPict.left + PictWidth);
RectText.bottom = min(RectText.bottom, RectText.top + TextHeight);
RectPict.bottom = min(RectPict.bottom, RectPict.top + PictHeight);

if (Len && lpPict->PictType)
   {
   if ((Style & SBS_TEXTLEFT_PICTRIGHT) || (Style & SBS_TEXTRIGHT_PICTLEFT))
      {
		if (RECTWIDTH(&RectText) + RECTWIDTH(&RectPict) > RECTWIDTH(&Rect))
			{
			/* RFW - 7/17/08 - 22579
			RectText.right = RectText.left + (RECTWIDTH(&RectText) + RECTWIDTH(&RectPict) - RECTWIDTH(&Rect)) / 2;
			*/
			RectText.right = RectText.left + (int)((double)TextWidth * ((double)RECTWIDTH(&Rect) / (double)(TextWidth + PictWidth)));
			RectPict.right = RectPict.left + RECTWIDTH(&Rect) - RECTWIDTH(&RectText);
			}

		if ((Style & SSS_ALIGN_LEFT) || (Style & SSS_ALIGN_RIGHT))
			{
			if (Style & SBS_TEXTLEFT_PICTRIGHT)
				OffsetRect(&RectPict, RectText.right - RectPict.left, 0);
			else
				OffsetRect(&RectText, RectPict.right - RectText.left, 0);

			if (Style & SSS_ALIGN_RIGHT)
				{
				OffsetRect(&RectText, RECTWIDTH(&Rect) - RECTWIDTH(&RectText) - RECTWIDTH(&RectPict), 0);
				OffsetRect(&RectPict, RECTWIDTH(&Rect) - RECTWIDTH(&RectText) - RECTWIDTH(&RectPict), 0);
				}
			}

		else // (Style & SSS_ALIGN_CENTER)
			{
			x = (short)((RECTWIDTH(&Rect) - RECTWIDTH(&RectText) - RECTWIDTH(&RectPict)) / 3);

			if (Style & SBS_TEXTLEFT_PICTRIGHT)
				{
				OffsetRect(&RectText, x, 0);
				OffsetRect(&RectPict, RectText.right + x - RectPict.left, 0);
				}
			else
				{
				OffsetRect(&RectPict, x, 0);
				OffsetRect(&RectText, RectPict.right + x - RectText.left, 0);
				}
			}

		if (Style & SSS_ALIGN_TOP)
			;

		else if (Style & SSS_ALIGN_BOTTOM)
			{
			OffsetRect(&RectText, 0, RECTHEIGHT(&Rect) - RECTHEIGHT(&RectText));
			OffsetRect(&RectPict, 0, RECTHEIGHT(&Rect) - RECTHEIGHT(&RectPict));
			}

		else // (Style & SSS_ALIGN_VCENTER)
			{
			OffsetRect(&RectText, 0, (RECTHEIGHT(&Rect) - RECTHEIGHT(&RectText)) / 2);
			OffsetRect(&RectPict, 0, (RECTHEIGHT(&Rect) - RECTHEIGHT(&RectPict)) / 2);
			}
      }

   else
      {
		if (RECTHEIGHT(&RectText) + RECTHEIGHT(&RectPict) > RECTHEIGHT(&Rect))
			{
			/* RFW - 7/17/08 - 22579
			RectText.bottom = RectText.top + (RECTHEIGHT(&RectText) + RECTHEIGHT(&RectPict) - RECTHEIGHT(&Rect)) / 2;
			*/
			RectText.bottom = RectText.top + (int)((double)TextHeight * ((double)RECTHEIGHT(&Rect) / (double)(TextHeight + PictHeight)));
			RectPict.bottom = RectPict.top + RECTHEIGHT(&Rect) - RECTHEIGHT(&RectText);
			}

		if ((Style & SSS_ALIGN_TOP) || (Style & SSS_ALIGN_BOTTOM))
			{
			if (Style & SBS_TEXTTOP_PICTBOT)
				OffsetRect(&RectPict, 0, RectText.bottom - RectPict.top);
			else
				OffsetRect(&RectText, 0, RectPict.bottom - RectText.top);

			if (Style & SSS_ALIGN_BOTTOM)
				{
				OffsetRect(&RectText, 0, RECTHEIGHT(&Rect) - RECTHEIGHT(&RectText) - RECTHEIGHT(&RectPict));
				OffsetRect(&RectPict, 0, RECTHEIGHT(&Rect) - RECTHEIGHT(&RectText) - RECTHEIGHT(&RectPict));
				}
			}

		else // (Style & SSS_ALIGN_CENTER)
			{
			y = (short)((RECTHEIGHT(&Rect) - RECTHEIGHT(&RectText) - RECTHEIGHT(&RectPict)) / 3);

			if (Style & SBS_TEXTTOP_PICTBOT)
				{
				OffsetRect(&RectText, 0, y);
				OffsetRect(&RectPict, 0, RectText.bottom + y - RectPict.top);
				}
			else
				{
				OffsetRect(&RectPict, 0, y);
				OffsetRect(&RectText, 0, RectPict.bottom + y - RectText.top);
				}
			}

		if (Style & SSS_ALIGN_LEFT)
			;

		else if (Style & SSS_ALIGN_RIGHT)
			{
			OffsetRect(&RectText, RECTWIDTH(&Rect) - RECTWIDTH(&RectText), 0);
			OffsetRect(&RectPict, RECTWIDTH(&Rect) - RECTWIDTH(&RectPict), 0);
			}

		else // (Style & SSS_ALIGN_CENTER)
			{
			OffsetRect(&RectText, (RECTWIDTH(&Rect) - RECTWIDTH(&RectText)) / 2, 0);
			OffsetRect(&RectPict, (RECTWIDTH(&Rect) - RECTWIDTH(&RectPict)) / 2, 0);
			}
      }
   }
else
	{
   // Adjust horizontal alignment
	if (Style & SSS_ALIGN_LEFT)
		;

	else if (Style & SSS_ALIGN_RIGHT)
		{
		OffsetRect(&RectText, RECTWIDTH(&Rect) - RECTWIDTH(&RectText), 0);
		OffsetRect(&RectPict, RECTWIDTH(&Rect) - RECTWIDTH(&RectPict), 0);
		}

	else // (Style & SSS_ALIGN_CENTER)
		{
		OffsetRect(&RectText, (RECTWIDTH(&Rect) - RECTWIDTH(&RectText)) / 2, 0);
		OffsetRect(&RectPict, (RECTWIDTH(&Rect) - RECTWIDTH(&RectPict)) / 2, 0);
		}

   // Adjust vertical alignment
	if (Style & SSS_ALIGN_TOP)
		;

	else if (Style & SSS_ALIGN_BOTTOM)
		{
		OffsetRect(&RectText, 0, RECTHEIGHT(&Rect) - RECTHEIGHT(&RectText));
		OffsetRect(&RectPict, 0, RECTHEIGHT(&Rect) - RECTHEIGHT(&RectPict));
		}

	else // (Style & SSS_ALIGN_VCENTER)
		{
		OffsetRect(&RectText, 0, (RECTHEIGHT(&Rect) - RECTHEIGHT(&RectText)) / 2);
		OffsetRect(&RectPict, 0, (RECTHEIGHT(&Rect) - RECTHEIGHT(&RectPict)) / 2);
		}
	}

/***************
* Draw Picture
***************/
//Modify By BOC 99.6.29 (hyt)------------------------------
//for draw error when not enough height
//if (lpPict->PictType && RectPict.right > RectPict.left)
if (lpPict->PictType && RectPict.right > RectPict.left && RectPict.bottom>RectPict.top)
//---------------------------------------------------------
   {
   if (lpSuperBtn->fStretch)
      {
      if (!lpPict->hPictStretch)
         {
         if (lpPict->PictType == SUPERBTN_PICT_ICON)
            lpPict->hPictStretch = tbCreateStretchedIcon(hDC, &RectPict,
                                   Color.Color, lpPict->hPict, FALSE);
         else
            lpPict->hPictStretch = tbCreateStretchedBitmap(hDC, lpPict->hPict,
                                                           &RectPict, FALSE, 0);
         }

      GetObject(lpPict->hPictStretch, sizeof(BITMAP), (LPTSTR)&bm);

      if (fUseDib)
         {
         bm.bmWidth = (short)(bm.bmWidth * dfScaleX);
         bm.bmHeight = (short)(bm.bmHeight * dfScaleY);
         }

      x = (short)(RectPict.left + (RectPict.right - RectPict.left - bm.bmWidth) / 2);
      y = (short)(RectPict.top + (RectPict.bottom - RectPict.top - bm.bmHeight) / 2);
      }

   else
      {
      x = (short)(RectPict.left + (RectPict.right - RectPict.left - PictWidthNew) / 2);
      y = (short)(RectPict.top + (RectPict.bottom - RectPict.top - PictHeightNew) / 2);

      /****************************
      * Draw Frame around picture
      ****************************/

      if ((Style & SBS_PICTFRAMEIN) || (Style & SBS_PICTFRAMEOUT))
         {
         RectTemp.left = max(x - 4, RectPict.left);
         RectTemp.right = min(x + PictWidthNew + 4, RectPict.right);
         RectTemp.top = max(y - 4, RectPict.top);
         RectTemp.bottom = min(y + PictHeightNew + 4, RectPict.bottom);

         ButtonIn = (BOOL)((Style & SBS_PICTFRAMEOUT) ? 0 : 1);
         if (!(Style & SBS_NOINVERTFRAME) && lpSuperBtn->ButtonDown)
            ButtonIn = !ButtonIn;

         tbDrawShadows(hDC, &RectTemp, lpSuperBtn->ShadowSize,
                       Color.ColorShadow,
                       Color.ColorHighlight, ButtonIn, FALSE);
         }
      }

   if (lpSuperBtn->ButtonDown && !lpSuperBtn->PictBtnDown.PictType)
      {
      x++;
      y++;
      }

   if (fUseDib)
      {
      hPict = 0;

      if (lpPict->hPictStretch)
         hPict = lpPict->hPictStretch;

      else if (lpPict->hPict)
         switch (lpPict->PictType)
            {
            case SUPERBTN_PICT_ICON:
               SetRect(&RectTemp, 0, 0, tbGetIconWidth(lpPict->hPict), tbGetIconHeight(lpPict->hPict));
               hPict = tbCreateStretchedIcon(hDC, &RectTemp, Color.Color,
                                             lpPict->hPict, FALSE);
               fDeletePict = TRUE;
               break;

            case SUPERBTN_PICT_BITMAP:
               hPict = lpPict->hPict;
               break;
            }

      hPalette = tbGetSystemPalette();
      hDIB = tbBitmapToDIB(hPict, hPalette);
      DeleteObject(hPalette);

      SetStretchBltMode(hDC, COLORONCOLOR);
      iMapModeOld = SetMapMode(hDC, MM_TEXT);

      x = (short)max(RectPict.left, x);
      y = (short)max(RectPict.top, y);

      PictWidthOrig = min(lpPict->PictWidth, (short)((RectPict.right - x) /
                          dfScaleX));
      PictHeightOrig = min(lpPict->PictHeight, (short)((RectPict.bottom - y) /
                           dfScaleY));

      xOrig = max((lpPict->PictWidth - (short)((RectPict.right - x) /
                   dfScaleX)) / 2, 0);
      yOrig = max((lpPict->PictHeight - (short)((RectPict.bottom - y) /
                   dfScaleY)) / 2, 0);

      tbStretchDibBlt(hDC, x, y, min(RectPict.right - x, PictWidthNew),
                      min(RectPict.bottom - y, PictHeightNew), hDIB,
                      xOrig, yOrig, PictWidthOrig, PictHeightOrig, SRCCOPY);

      if (fDeletePict)
         DeleteObject(hPict);

      SetMapMode(hDC, iMapModeOld);
      }

   else
      {
      if (lpPict->hPictStretch)
         tbDrawBitmap(hDC, lpPict->hPictStretch, &RectPict, x, y, SRCCOPY);

      else
         switch (lpPict->PictType)
            {
            case SUPERBTN_PICT_ICON:
               if (lpPict->hPict)
                  tbDrawIcon(hDC, &RectPict, Color.Color, x, y,
                             lpPict->hPict);

               break;

            case SUPERBTN_PICT_BITMAP:
               if (lpPict->hPict)
               {
                  SaveDC(hDC);
                  IntersectClipRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);
                  tbDrawBitmap(hDC, lpPict->hPict, &RectPict, x, y, SRCCOPY);
                  RestoreDC(hDC, -1);
               }
               break;
            }
      }
   }

/************
* Draw Text
************/

if (Len)
   {
   _fmemcpy(&RectTemp, &RectText, sizeof(RECT));
   TextHeight = DrawText(hDC, Text, -1, &RectTemp, DT_CALCRECT | DT_TOP |
                         DT_LEFT | DT_SINGLELINE);
   TextWidth = (short)(RectTemp.right - RectTemp.left);
   TextHeight = (short)fm.tmHeight;

   x = (short)(RectText.left + max((RectText.right - RectText.left -
       TextWidth) / 2, 0));

   y = (short)(RectText.top + max((RectText.bottom - RectText.top -
       TextHeight) / 2, 0));

   /*************************
   * Draw Frame around text
   *************************/

   if ((Style & SBS_TEXTFRAMEIN) || (Style & SBS_TEXTFRAMEOUT))
      {
      RectTemp.left = max(x - 4, RectText.left);
      RectTemp.right = min(x + TextWidth + 4, RectText.right);
      RectTemp.top = max(y - 4, RectText.top);
      RectTemp.bottom = min(y + TextHeight + 4, RectText.bottom);

      ButtonIn = (BOOL)((Style & SBS_TEXTFRAMEOUT) ? 0 : 1);
      if (!(Style & SBS_NOINVERTFRAME) && lpSuperBtn->ButtonDown)
         ButtonIn = !ButtonIn;

      tbDrawShadows(hDC, &RectTemp, lpSuperBtn->ShadowSize,
                    Color.ColorShadow,
                    Color.ColorHighlight, ButtonIn, FALSE);
      }

   RectText.left = x;
   RectText.top = y;
   RectText.right = min(x + TextWidth, RectText.right);
   RectText.bottom = min(y + TextHeight, RectText.bottom);

   SetTextColor(hDC, Color.ColorText);
   SetBkColor(hDC, Color.Color);

   if (lpSuperBtn->ButtonDown)
      OffsetRect(&RectText, 1, 1);

   SaveDC(hDC);
   IntersectClipRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

   if (fIsEnabled)
      DrawText(hDC, Text, -1, &RectText, DT_TOP | DT_LEFT | DT_SINGLELINE);
   else
      {
      if (hTextTemp = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  (lstrlen(Text) + 1) * sizeof(TCHAR)))
         {
         TextTemp = (LPTSTR)GlobalLock(hTextTemp);

         for (i = 0, At = 0; i < lstrlen(Text); i++)
            if (Text[i] != '&')
               TextTemp[At++] = Text[i];

         hBrush = CreateSolidBrush(Color.ColorText);
         GrayString(hDC, hBrush, NULL, (LPARAM)TextTemp, 0, x, y, 0, 0);
         DeleteObject(hBrush);

         GlobalUnlock(hTextTemp);
         GlobalFree(hTextTemp);
         }
      }

   RestoreDC(hDC, -1);
   }

if (lpSuperBtn->hText)
   GlobalUnlock(lpSuperBtn->hText);

if (fFocus)
   {
   if (Len && RectText.bottom > RectText.top && RectText.right > RectText.left)
      {
      RectText.bottom += 1;
      RectText.left -= 1;
      RectText.right += 1;

      SuperBtnFocusRect(hDC, &RectText, lpSuperBtn->hBrushFocus);
      }
   else if (Rect.right - Rect.left > 10 &&
            Rect.bottom - Rect.top > 10)
      {
      if (lpSuperBtn->ButtonDown)
         OffsetRect(&Rect, 1, 1);

      tbShrinkRect(&Rect, 1, 1);
      SuperBtnFocusRect(hDC, &Rect, lpSuperBtn->hBrushFocus);
      }
   }

if (hFontOld)
   SelectObject(hDC, hFontOld);

#if SS_V80
SetBkMode(hDC, prevBkMode);
#endif

clrText = SetTextColor(hDC, clrText);
clrBk = SetBkColor(hDC, clrBk);

}


#if 0
void SuperBtnPaint(HWND hWnd, LPSUPERBTN lpSuperBtn, HDC hDC, LPRECT lpRect,
                   BOOL fIsEnabled, BOOL fFocus, BOOL fRoundCorners,
                   BOOL fUseDib, LPRECT lpRectOrig, BOOL fIsButton)
{
SUPERBTNCOLOR Color;
GLOBALHANDLE  hTextTemp;
TEXTMETRIC    fm;
COLORREF      clrText;
COLORREF      clrBk;
BITMAP        bm;
HBRUSH        hBrush;
HBRUSH        hBrushOld;
HBRUSH        hBrushBackground;
HPEN          hPenBorder;
HPEN          hPenOld;
HFONT         hFontOld = 0;
HANDLE        hDIB;
HPALETTE      hPalette;
HBITMAP       hPict;
LPTBBTNPICT   lpPict;
RECT          Rect;
RECT          RectTemp;
RECT          RectPict;
RECT          RectText;
LPTSTR        Text;
LPTSTR        TextTemp;
long          Style;
BOOL          ButtonIn;
BOOL          fDeletePict = FALSE;
double        dfScaleX;
double        dfScaleY;
short         BorderSize = 0;
short         TextWidth;
short         TextHeight;
short         PictWidth;
short         PictHeight;
short         PictWidthNew;
short         PictHeightNew;
short         PictWidthOrig;
short         PictHeightOrig;
short         Len;
short         At;
short         x;
short         y;
short         xOrig;
short         yOrig;
short         i;
int           iMapModeOld;
SIZE          Size;

CopyRect(&Rect, lpRect);

Style = lpSuperBtn->lStyle;

/*************************************
* Fill the background with the
* current background of the display
*************************************/

SuperBtnGetCurrColor(&Color, lpSuperBtn);

hBrushBackground = CreateSolidBrush(Color.Color);
FillRect(hDC, &Rect, hBrushBackground);
DeleteObject(hBrushBackground);

clrText = SetTextColor(hDC, Color.ColorText);
clrBk = SetBkColor(hDC, Color.Color);

if (lpSuperBtn->hFont)
   hFontOld = SelectObject(hDC, lpSuperBtn->hFont);

/***********************
* Paint Outside border
***********************/

if (!lpSuperBtn->fIs95)
   BorderSize = 1;

if (Style & SBS_DEFPUSHBUTTON)
   BorderSize++;

if (BorderSize)
   {
   hBrushOld = SelectObject(hDC, GetStockObject(NULL_BRUSH));
   hPenBorder = CreatePen(PS_SOLID, 1, Color.ColorBorder);
   hPenOld = SelectObject(hDC, hPenBorder);

   for (i = 0; i < BorderSize; i++)
      {
      if (fRoundCorners)
         RoundRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom, 2, 2);
      else
         Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

      tbShrinkRect(&Rect, 1, 1);
      }

   SelectObject(hDC, hBrushOld);
   SelectObject(hDC, hPenOld);
   DeleteObject(hPenBorder);
   }

/**************
* Draw Shadow
**************/

if (lpSuperBtn->fIs95)
   {
   short nSize = max(1, lpSuperBtn->ShadowSize / 2);

   if (lpSuperBtn->ButtonDown)
      tbDrawShadows(hDC, &Rect, nSize, Color.ColorShadow,
                    Color.ColorShadow, FALSE, FALSE);

   else
      {
#ifdef SS_V35
      tbDrawShadows(hDC, &Rect, nSize, RGBCOLOR_BLACK, Color.ColorHighlight,
                    FALSE, TRUE);
      tbShrinkRect(&Rect, nSize, nSize);
      nSize = max(1, DIVUP(lpSuperBtn->ShadowSize, 2));
      tbDrawShadows(hDC, &Rect, nSize, Color.ColorShadow, Color.Color,
                    FALSE, TRUE);
#else
      tbDrawShadows(hDC, &Rect, nSize, RGBCOLOR_BLACK, Color.Color,
                    FALSE, TRUE);
      tbShrinkRect(&Rect, nSize, nSize);
      nSize = max(1, DIVUP(lpSuperBtn->ShadowSize, 2));
      tbDrawShadows(hDC, &Rect, nSize, Color.ColorShadow, Color.ColorHighlight,
                    FALSE, TRUE);
#endif
      }

   tbShrinkRect(&Rect, nSize, nSize);
   }

else
   {
   tbDrawShadows(hDC, &Rect, lpSuperBtn->ShadowSize,
                 Color.ColorShadow, Color.ColorHighlight,
                 (BOOL)(lpSuperBtn->ButtonDown ? 1 : 0), TRUE);

   tbShrinkRect(&Rect, lpSuperBtn->ShadowSize, lpSuperBtn->ShadowSize);
   }

/*************
* Draw Frame
*************/

if ((Style & SBS_BTNFRAMEIN) || (Style & SBS_BTNFRAMEOUT))
   {
   tbShrinkRect(&Rect, lpSuperBtn->BtnFrameOffset, lpSuperBtn->BtnFrameOffset);

   /************************
   * Draw Shadow for Frame
   ************************/

   ButtonIn = (BOOL)((Style & SBS_BTNFRAMEOUT) ? 0 : 1);
   if (!(Style & SBS_NOINVERTFRAME) && lpSuperBtn->ButtonDown)
      ButtonIn = !ButtonIn;

   tbDrawShadows(hDC, &Rect, lpSuperBtn->ShadowSize,
                 Color.ColorShadow,
                 Color.ColorHighlight, ButtonIn, FALSE);

   tbShrinkRect(&Rect, lpSuperBtn->ShadowSize, lpSuperBtn->ShadowSize);
   }

/**********************************
* Determine Picture and Text size
**********************************/

if (lpSuperBtn->hText)
   {
   Text = (LPTSTR)GlobalLock(lpSuperBtn->hText);
   Len = lstrlen(Text);
   }
else
   Len = 0;

_fmemcpy(&RectPict, &Rect, sizeof(RECT));
_fmemcpy(&RectText, &Rect, sizeof(RECT));

lpPict = &lpSuperBtn->Pict;

if (!fIsEnabled && lpSuperBtn->PictDisabled.PictType)
   lpPict = &lpSuperBtn->PictDisabled;

else if (lpSuperBtn->ButtonDown && lpSuperBtn->PictBtnDown.PictType)
   lpPict = &lpSuperBtn->PictBtnDown;

GetTextMetrics(hDC, &fm);

if (fUseDib)
   {
   dfScaleX = (double)(lpRect->right - lpRect->left) /
              (double)(lpRectOrig->right - lpRectOrig->left);

   dfScaleY = (double)(lpRect->bottom - lpRect->top) /
              (double)(lpRectOrig->bottom - lpRectOrig->top);

   PictWidthNew = (short)(lpPict->PictWidth * dfScaleX);
   PictHeightNew = (short)(lpPict->PictHeight * dfScaleY);
   }

else
   {
   PictWidthNew = lpPict->PictWidth;
   PictHeightNew = lpPict->PictHeight;
   }

if (Len && lpPict->PictType)
   {
   if ((Style & SBS_TEXTLEFT_PICTRIGHT) || (Style & SBS_TEXTRIGHT_PICTLEFT))
      {
      GetTextExtentPoint(hDC, Text, lstrlen(Text), &Size);
      TextWidth = (short)Size.cx;
      if ((Style & SBS_TEXTFRAMEIN) || (Style & SBS_TEXTFRAMEOUT))
         TextWidth += 8;

      PictWidth = PictWidthNew;
      if ((Style & SBS_PICTFRAMEIN) || (Style & SBS_PICTFRAMEOUT))
         PictWidth += 8;

		x = (Rect.right - Rect.left - TextWidth - PictWidth) / 3;

      if (Style & SBS_TEXTLEFT_PICTRIGHT)
         {
         RectText.right = Rect.left + x + TextWidth + x;
		 //- Wei Feng for Incident 171
         if ((RectPict.right - RectPict.left) > PictWidth)
            RectPict.left = RectText.right - x;
         }
      else
         {
         if ((RectPict.right - RectPict.left) > PictWidth)
            RectPict.right = Rect.left + x + PictWidth + x;
         RectText.left = RectPict.right - x;
         }
      }

   else
      {
      TextHeight = (short)fm.tmHeight;
      if ((Style & SBS_TEXTFRAMEIN) || (Style & SBS_TEXTFRAMEOUT))
         TextHeight += 8;

      PictHeight = lpPict->PictHeight;
      if ((Style & SBS_PICTFRAMEIN) || (Style & SBS_PICTFRAMEOUT))
         PictHeight += 8;

      y = max(0, (Rect.bottom - Rect.top - TextHeight - PictHeight) / 3);

      if (Style & SBS_TEXTTOP_PICTBOT)
         {
         RectText.bottom = Rect.top + y + TextHeight + y;
         RectPict.top = RectText.bottom - y;
         RectText.bottom = min(RectText.bottom, Rect.bottom);
         }
      else
         {
         RectPict.bottom = Rect.top + y + PictHeight + y;
         RectText.top = RectPict.bottom - y;
         RectPict.bottom = min(RectPict.bottom, Rect.bottom);
         }
      }
   }

/***************
* Draw Picture
***************/
//Modify By BOC 99.6.29 (hyt)------------------------------
//for draw error when not enough height
//if (lpPict->PictType && RectPict.right > RectPict.left)
if (lpPict->PictType && RectPict.right > RectPict.left && RectPict.bottom>RectPict.top)
//---------------------------------------------------------
   {
   if (lpSuperBtn->fStretch)
      {
      if (!lpPict->hPictStretch)
         {
         if (lpPict->PictType == SUPERBTN_PICT_ICON)
            lpPict->hPictStretch = tbCreateStretchedIcon(hDC, &RectPict,
                                   Color.Color, lpPict->hPict, FALSE);
         else
            lpPict->hPictStretch = tbCreateStretchedBitmap(hDC, lpPict->hPict,
                                                           &RectPict, FALSE, 0);
         }

      GetObject(lpPict->hPictStretch, sizeof(BITMAP), (LPTSTR)&bm);

      if (fUseDib)
         {
         bm.bmWidth = (short)(bm.bmWidth * dfScaleX);
         bm.bmHeight = (short)(bm.bmHeight * dfScaleY);
         }

      x = RectPict.left + (RectPict.right - RectPict.left - bm.bmWidth) / 2;
      y = RectPict.top + (RectPict.bottom - RectPict.top - bm.bmHeight) / 2;
      }

   else
      {
      x = RectPict.left + (RectPict.right - RectPict.left - PictWidthNew) / 2;
      y = RectPict.top + (RectPict.bottom - RectPict.top - PictHeightNew) / 2;

      /****************************
      * Draw Frame around picture
      ****************************/

      if ((Style & SBS_PICTFRAMEIN) || (Style & SBS_PICTFRAMEOUT))
         {
         RectTemp.left = max(x - 4, RectPict.left);
         RectTemp.right = min(x + PictWidthNew + 4, RectPict.right);
         RectTemp.top = max(y - 4, RectPict.top);
         RectTemp.bottom = min(y + PictHeightNew + 4, RectPict.bottom);

         ButtonIn = (BOOL)((Style & SBS_PICTFRAMEOUT) ? 0 : 1);
         if (!(Style & SBS_NOINVERTFRAME) && lpSuperBtn->ButtonDown)
            ButtonIn = !ButtonIn;

         tbDrawShadows(hDC, &RectTemp, lpSuperBtn->ShadowSize,
                       Color.ColorShadow,
                       Color.ColorHighlight, ButtonIn, FALSE);
         }
      }

   if (lpSuperBtn->ButtonDown && !lpSuperBtn->PictBtnDown.PictType)
      {
      x++;
      y++;
      }

   if (fUseDib)
      {
      hPict = 0;

      if (lpPict->hPictStretch)
         hPict = lpPict->hPictStretch;

      else if (lpPict->hPict)
         switch (lpPict->PictType)
            {
            case SUPERBTN_PICT_ICON:
               SetRect(&RectTemp, 0, 0, tbGetIconWidth(lpPict->hPict), tbGetIconHeight(lpPict->hPict));
               hPict = tbCreateStretchedIcon(hDC, &RectTemp, Color.Color,
                                             lpPict->hPict, FALSE);
               fDeletePict = TRUE;
               break;

            case SUPERBTN_PICT_BITMAP:
               hPict = lpPict->hPict;
               break;
            }

      hPalette = tbGetSystemPalette();
      hDIB = tbBitmapToDIB(hPict, hPalette);
      DeleteObject(hPalette);

      SetStretchBltMode(hDC, COLORONCOLOR);
      iMapModeOld = SetMapMode(hDC, MM_TEXT);

      x = max(RectPict.left, x);
      y = max(RectPict.top, y);

      PictWidthOrig = min(lpPict->PictWidth, (short)((RectPict.right - x) /
                          dfScaleX));
      PictHeightOrig = min(lpPict->PictHeight, (short)((RectPict.bottom - y) /
                           dfScaleY));

      xOrig = max((lpPict->PictWidth - (short)((RectPict.right - x) /
                   dfScaleX)) / 2, 0);
      yOrig = max((lpPict->PictHeight - (short)((RectPict.bottom - y) /
                   dfScaleY)) / 2, 0);

      tbStretchDibBlt(hDC, x, y, min(RectPict.right - x, PictWidthNew),
                      min(RectPict.bottom - y, PictHeightNew), hDIB,
                      xOrig, yOrig, PictWidthOrig, PictHeightOrig, SRCCOPY);

      if (fDeletePict)
         DeleteObject(hPict);

      SetMapMode(hDC, iMapModeOld);
      }

   else
      {
      if (lpPict->hPictStretch)
         tbDrawBitmap(hDC, lpPict->hPictStretch, &RectPict, x, y, SRCCOPY);

      else
         switch (lpPict->PictType)
            {
            case SUPERBTN_PICT_ICON:
               if (lpPict->hPict)
                  tbDrawIcon(hDC, &RectPict, Color.Color, x, y,
                             lpPict->hPict);

               break;

            case SUPERBTN_PICT_BITMAP:
               if (lpPict->hPict)
                  tbDrawBitmap(hDC, lpPict->hPict, &RectPict, x, y, SRCCOPY);

               break;
            }
      }
   }

/************
* Draw Text
************/

if (Len)
   {
   _fmemcpy(&RectTemp, &RectText, sizeof(RECT));
   TextHeight = DrawText(hDC, Text, -1, &RectTemp, DT_CALCRECT | DT_TOP |
                         DT_LEFT | DT_SINGLELINE);
   TextWidth = (short)(RectTemp.right - RectTemp.left);
   TextHeight = (short)fm.tmHeight;

   x = RectText.left + max((RectText.right - RectText.left -
       TextWidth) / 2, 0);

   y = RectText.top + max((RectText.bottom - RectText.top -
       TextHeight) / 2, 0);

   /*************************
   * Draw Frame around text
   *************************/

   if ((Style & SBS_TEXTFRAMEIN) || (Style & SBS_TEXTFRAMEOUT))
      {
      RectTemp.left = max(x - 4, RectText.left);
      RectTemp.right = min(x + TextWidth + 4, RectText.right);
      RectTemp.top = max(y - 4, RectText.top);
      RectTemp.bottom = min(y + TextHeight + 4, RectText.bottom);

      ButtonIn = (BOOL)((Style & SBS_TEXTFRAMEOUT) ? 0 : 1);
      if (!(Style & SBS_NOINVERTFRAME) && lpSuperBtn->ButtonDown)
         ButtonIn = !ButtonIn;

      tbDrawShadows(hDC, &RectTemp, lpSuperBtn->ShadowSize,
                    Color.ColorShadow,
                    Color.ColorHighlight, ButtonIn, FALSE);
      }

   RectText.left = x;
   RectText.top = y;
   RectText.right = min(x + TextWidth, RectText.right);
   RectText.bottom = min(y + TextHeight, RectText.bottom);

   SetTextColor(hDC, Color.ColorText);
   SetBkColor(hDC, Color.Color);

   if (lpSuperBtn->ButtonDown)
      OffsetRect(&RectText, 1, 1);

   SaveDC(hDC);
   IntersectClipRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

   if (fIsEnabled)
      DrawText(hDC, Text, -1, &RectText, DT_TOP | DT_LEFT | DT_SINGLELINE);
   else
      {
      if (hTextTemp = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  (lstrlen(Text) + 1) * sizeof(TCHAR)))
         {
         TextTemp = (LPTSTR)GlobalLock(hTextTemp);

         for (i = 0, At = 0; i < lstrlen(Text); i++)
            if (Text[i] != '&')
               TextTemp[At++] = Text[i];

         hBrush = CreateSolidBrush(Color.ColorText);
         GrayString(hDC, hBrush, NULL, (DWORD)TextTemp, 0, x, y, 0, 0);
         DeleteObject(hBrush);

         GlobalUnlock(hTextTemp);
         GlobalFree(hTextTemp);
         }
      }

   RestoreDC(hDC, -1);
   }

if (lpSuperBtn->hText)
   GlobalUnlock(lpSuperBtn->hText);

if (fFocus)
   {
   if (Len && RectText.bottom > RectText.top && RectText.right > RectText.left)
      {
      RectText.bottom += 1;
      RectText.left -= 1;
      RectText.right += 1;

      SuperBtnFocusRect(hDC, &RectText, lpSuperBtn->hBrushFocus);
      }
   else if (Rect.right - Rect.left > 10 &&
            Rect.bottom - Rect.top > 10)
      {
      if (lpSuperBtn->ButtonDown)
         OffsetRect(&Rect, 1, 1);

      tbShrinkRect(&Rect, 1, 1);
      SuperBtnFocusRect(hDC, &Rect, lpSuperBtn->hBrushFocus);
      }
   }

if (hFontOld)
   SelectObject(hDC, hFontOld);

clrText = SetTextColor(hDC, clrText);
clrBk = SetBkColor(hDC, clrBk);
}
#endif // 0


static void SuperBtnSetPtr(hWnd, hGlobal)

HWND         hWnd;
GLOBALHANDLE hGlobal;
{
#if defined(_WIN64) || defined(_IA64)
  SetWindowLongPtr(hWnd, 2, (LONG_PTR)hGlobal);
#else
  SetWindowLong(hWnd, 2, (LONG)hGlobal);
#endif
}
 

LPSUPERBTN SuperBtnGetPtr(hWnd, hGlobal)

HWND              hWnd;
GLOBALHANDLE FAR *hGlobal;
{
#if defined(_WIN64) || defined(_IA64)
*hGlobal = (GLOBALHANDLE)GetWindowLongPtr(hWnd, 2);
#else
*hGlobal = (GLOBALHANDLE)GetWindowLong(hWnd, 2);
#endif
return ((LPSUPERBTN)GlobalLock(*hGlobal));
}


void SetDefPushButton(hWnd)

HWND hWnd;
{
HWND hWndTemp;

for (hWndTemp = GetWindow(GetParent(hWnd), GW_CHILD); hWndTemp;
     hWndTemp = GetWindow(hWndTemp, GW_HWNDNEXT))
   if (hWndTemp != hWnd &&
       SendMessage(hWndTemp, WM_GETDLGCODE, 0, 0L) & DLGC_DEFPUSHBUTTON)
      SendMessage(hWndTemp, BM_SETSTYLE, (short)BS_PUSHBUTTON, 1L);

if (SendMessage(hWnd, WM_GETDLGCODE, 0, 0L) & DLGC_UNDEFPUSHBUTTON)
   SendMessage(hWnd, BM_SETSTYLE, (short)BS_DEFPUSHBUTTON, 1L);
}


BOOL SuperBtnSetPict(hWnd, hInst, Pict, wPictType, lPict)

HWND        hWnd;
HANDLE      hInst;
LPTBBTNPICT Pict;
WORD        wPictType;
LPARAM      lPict;
{
BITMAP      Bitmap;
BOOL        fRet = FALSE;

if (hWnd && !hInst)
   hInst = GetWindowInstance(hWnd);

if (Pict->hPict)
   {
   SuperBtnFreePict(Pict);
   Pict->hPict = 0;
   }

Pict->PictType = SUPERBTN_PICT_NONE;
Pict->HandleProvided = FALSE;

if (wPictType & SUPERBTN_PICT_HANDLE)
	{
#ifdef SS_V70
	if (!(wPictType & BT_FREEHANDLE))
#endif // SS_V70
		Pict->HandleProvided = TRUE;
	}

if (wPictType & SUPERBTN_PICT_ICON)
   {
   fRet = TRUE;

   if (wPictType & SUPERBTN_PICT_HANDLE)
      Pict->hPict = (HANDLE)lPict;
   else if (!(Pict->hPict = LoadIcon(hInst, (LPTSTR)lPict)))
      fRet = FALSE;

   if (Pict->hPict)
      {
      Pict->PictType = SUPERBTN_PICT_ICON;
      Pict->PictWidth = (short)tbGetIconWidth(Pict->hPict);
      Pict->PictHeight = (short)tbGetIconHeight(Pict->hPict);
      }
   }

else if (wPictType & SUPERBTN_PICT_BITMAP)
   {
   fRet = TRUE;

   if (wPictType & SUPERBTN_PICT_HANDLE)
      Pict->hPict = (HANDLE)lPict;
   else if (!(Pict->hPict = LoadBitmap(hInst, (LPTSTR)lPict)))
   {
	   DWORD dwError = GetLastError();
		fRet = FALSE;
   }

   if (Pict->hPict)
      {
      Pict->PictType = SUPERBTN_PICT_BITMAP;

      GetObject(Pict->hPict, sizeof(BITMAP), (LPTSTR)&Bitmap);
      Pict->PictWidth = (short)Bitmap.bmWidth;
      Pict->PictHeight = (short)Bitmap.bmHeight;
      }
   }

if (hWnd)
   {
   InvalidateRect(hWnd, NULL, TRUE);
   UpdateWindow(hWnd);
   }

return (fRet);
}


BOOL DLLENTRY SuperBtnFocusRect(HDC hDC, LPRECT lpRect, HBRUSH hBrush)
{
int      iOldROP;
RECT     Rect;
HBRUSH   hOldBrush;
HPEN     hOldPen;
COLORREF TextColorOld;

TextColorOld = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

iOldROP = SetROP2(hDC, R2_XORPEN);
hOldBrush = SelectObject(hDC, hBrush);
UnrealizeObject(hBrush);
hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));

/************************
* Draw the dotted lines
************************/

/********************
* Draw the top line
********************/

Rect.bottom = Rect.top = lpRect->top;
Rect.bottom += 2;
Rect.left = lpRect->left;
Rect.right = lpRect->right;
Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

/***********************
* Draw the bottom line
***********************/

Rect.bottom = Rect.top = lpRect->bottom;
Rect.top -= 2;
Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

/*********************************
* Draw do the left vertical line
*********************************/

Rect.top = lpRect->top + 1;
Rect.bottom = lpRect->bottom;
Rect.right = Rect.left = lpRect->left;
Rect.right += 2;
Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

/*******************************
* Draw the right vertical line
*******************************/

Rect.right = Rect.left = lpRect->right;
Rect.left -= 2;
Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

/***********
* Clean up
***********/

SetTextColor(hDC, TextColorOld);
SetROP2(hDC, iOldROP);
SelectObject(hDC, hOldPen);
SelectObject(hDC, hOldBrush);
return(TRUE);
}


void SuperBtnFreeStretch(LPSUPERBTN lpSuperBtn)
{
if (lpSuperBtn->Pict.hPictStretch)
   {
   DeleteObject(lpSuperBtn->Pict.hPictStretch);
   lpSuperBtn->Pict.hPictStretch = 0;
   }

if (lpSuperBtn->PictDisabled.hPictStretch)
   {
   DeleteObject(lpSuperBtn->PictDisabled.hPictStretch);
   lpSuperBtn->PictDisabled.hPictStretch = 0;
   }

if (lpSuperBtn->PictBtnDown.hPictStretch)
   {
   DeleteObject(lpSuperBtn->PictBtnDown.hPictStretch);
   lpSuperBtn->PictBtnDown.hPictStretch = 0;
   }
}


void SuperBtnGetCurrColor(LPSUPERBTNCOLOR lpColor, LPSUPERBTN lpSuperBtn)
{
_fmemcpy(lpColor, &lpSuperBtn->Color, sizeof(SUPERBTNCOLOR));

if (lpColor->Color == RGBCOLOR_DEFAULT)
   lpColor->Color = GetSysColor(COLOR_BTNFACE);

if (lpColor->ColorBorder == RGBCOLOR_DEFAULT)
   lpColor->ColorBorder = GetSysColor(COLOR_BTNTEXT);

if (lpColor->ColorShadow == RGBCOLOR_DEFAULT)
   lpColor->ColorShadow = GetSysColor(COLOR_BTNSHADOW);

if (lpColor->ColorText == RGBCOLOR_DEFAULT)
   lpColor->ColorText = GetSysColor(COLOR_BTNTEXT);

#if (WINVER >= 0x030a)
if (lpColor->ColorHighlight == RGBCOLOR_DEFAULT)
   lpColor->ColorHighlight = GetSysColor(COLOR_BTNHIGHLIGHT);
#endif
}
