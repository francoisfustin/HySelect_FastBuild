/*********************************************************
* CHECKBOX.C
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

#define OEMRESOURCE 
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "toolbox.h"
#include "fphdc.h"
#include "checkbox.h"
#include "superbtn.h"
#include "wintools.h"
#include "..\spread\ss_w32s.h"
#ifdef SS_V80
#include "uxtheme.h"
#include "ss_theme.h"
#endif

/**********************
* Function prototypes
**********************/

static LPCHECKBOX       CheckBoxInit(GLOBALHANDLE FAR *hGlobal);
BOOL                    CheckBoxSetText(HWND hWnd, LPCTSTR Text);
void                    CheckBoxPaintPict(HWND hWnd, LPCHECKBOX lpCheckBox,
                                          LPRECT lpRect, HDC hDCDev, HDC hDC,
                                          HBRUSH hBrush, BOOL fPaint,
                                          BOOL fUseDib, LPRECT lpRectOrig);
void                    CheckBoxPaintText(HWND hWnd, LPCHECKBOX lpCheckBox,
                                          LPRECT lpRect, LPRECT lpRectText,
                                          HDC hDCDev, HDC hDC, HBRUSH hBrush,
                                          BOOL fPaint, BOOL fUseDib,
                                          LPRECT lpRectOrig);
void                    CheckBoxGetPicture(HWND hWnd, LPCHECKBOX lpCheckBox,
                                           LPTBBTNPICT lpPict);
void                    CheckBoxPickPicture(LPCHECKBOX lpCheckBox,
                                            LPTBBTNPICT lpPict,
                                            WORD wPictType);
BOOL                    CheckBoxChoosePicture(LPCHECKBOX lpCheckBox,
                                              LPTBBTNPICT lpPict,
                                              WORD wPictType);
LPTBBTNPICT             CheckBoxGetPicturePtr(LPCHECKBOXPICTS lpPicts,
                                              WORD wPictType);
void                    CheckBoxSetDisabled(HWND hWnd, LPCHECKBOX lpCheckBox,
                                            BOOL fValue);
void                    CheckBoxSetPictMax(LPCHECKBOXPICTS lpPicts,
                                           LPTBBTNPICT lpPict);

GLOBALHANDLE  hCheckBoxDefPicts = 0;
short         dCheckBoxDefPictCnt = 0;
short         dCheckBoxDefPictLoadCnt = 0;


extern HANDLE hDynamicInst;


BOOL RegisterCheckBox(hInstance, lpszClassName)

HANDLE     hInstance;
LPCTSTR    lpszClassName;
{
WNDCLASS wc;

wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
wc.lpfnWndProc   = (WNDPROC)tbCheckBoxWndFn;
wc.cbClsExtra    = 0;
#if defined(_WIN64) || defined(_IA64)
wc.cbWndExtra    = 12;
#else
wc.cbWndExtra    = 8;
#endif
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = lpszClassName;

if (!RegisterClass((LPWNDCLASS)&wc))
   return (FALSE);

/*
CheckBoxLoadDefBitmaps(hInstance);
*/

return (TRUE);
}

BOOL DLLENTRY tbCheckBoxLoadDef(HANDLE hInst, WORD wButtonType, LPARAM lPict)
{
return (tbxCheckBoxLoadDef(&hCheckBoxDefPicts, &dCheckBoxDefPictCnt, hInst,
                           wButtonType, lPict));
}


BOOL DLLENTRY tbCheckBoxUnloadDefs(HANDLE hInst)
{
return (tbxCheckBoxUnloadDefs(&hCheckBoxDefPicts, &dCheckBoxDefPictCnt,
                             hInst));
}


BOOL DLLENTRY tbCheckBoxGetPict(HANDLE hInst, LPCHECKBOXPICTS lpDefPict)
{
return (tbxCheckBoxGetPict(&hCheckBoxDefPicts, &dCheckBoxDefPictCnt,
                           hInst, lpDefPict));
}


BOOL DLLENTRY tbxCheckBoxLoadDef(GLOBALHANDLE FAR *lphDefPicts,
                                 LPSHORT dDefPictCnt, HANDLE hInst,
                                 WORD wButtonType, LPARAM lPict)
{
LPCHECKBOXPICTS lpDefPicts;
BOOL            fRet = TRUE;
short           dFound = -1;
short           i;

if (*dDefPictCnt)
   {
   lpDefPicts = (LPCHECKBOXPICTS)GlobalLock(*lphDefPicts);

   for (i = 0; dFound == -1 && i < *dDefPictCnt; i++)
      if (lpDefPicts[i].hInst == hInst)
         dFound = i;

   GlobalUnlock(*lphDefPicts);

   if (dFound == -1)
      {
      if (!(*lphDefPicts = GlobalReAlloc(*lphDefPicts,
                                         (long)(sizeof(CHECKBOXPICTS) *
                                         (*dDefPictCnt + 1)),
                                         GMEM_MOVEABLE | GMEM_ZEROINIT |
                                         GMEM_SHARE)))
         return (FALSE);

      dFound = *dDefPictCnt;
      (*dDefPictCnt)++;
      }
   }

else
   {
   if (!(*lphDefPicts = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT |
                                    GMEM_SHARE, (long)sizeof(CHECKBOXPICTS))))
      return (FALSE);

   dFound = *dDefPictCnt;
   (*dDefPictCnt)++;
   }

lpDefPicts = (LPCHECKBOXPICTS)GlobalLock(*lphDefPicts);

lpDefPicts[dFound].hInst = hInst;

if (lPict >= OBM_LFARROWI && lPict <= OBM_OLD_CLOSE)
	hInst = 0;

CheckBoxSetPict(0, hInst, &lpDefPicts[dFound], wButtonType, lPict);

GlobalUnlock(*lphDefPicts);
return (fRet);
}


BOOL DLLENTRY tbxCheckBoxUnloadDefs(GLOBALHANDLE FAR *lphDefPicts,
                                    LPSHORT dDefPictCnt, HANDLE hInst)
{
LPCHECKBOXPICTS lpDefPicts;
short           dFound = -1;
short           i;

if (*dDefPictCnt && *lphDefPicts)
   {
   lpDefPicts = (LPCHECKBOXPICTS)GlobalLock(*lphDefPicts);

   for (i = 0; dFound == -1 && i < *dDefPictCnt; i++)
      if (lpDefPicts[i].hInst == hInst)
         dFound = i;

   if (dFound != -1)
      {
      CheckBoxFreePict(&lpDefPicts[dFound].PictUp);
      CheckBoxFreePict(&lpDefPicts[dFound].PictDown);
      CheckBoxFreePict(&lpDefPicts[dFound].PictGray);
      CheckBoxFreePict(&lpDefPicts[dFound].PictFocusUp);
      CheckBoxFreePict(&lpDefPicts[dFound].PictFocusDown);
      CheckBoxFreePict(&lpDefPicts[dFound].PictFocusGray);
      CheckBoxFreePict(&lpDefPicts[dFound].PictDisabledUp);
      CheckBoxFreePict(&lpDefPicts[dFound].PictDisabledDown);
      CheckBoxFreePict(&lpDefPicts[dFound].PictDisabledGray);

      for (i = dFound; i < *dDefPictCnt - 1; i++)
         _fmemcpy(&lpDefPicts[dFound], &lpDefPicts[dFound + 1],
                  sizeof(CHECKBOXPICTS));

      GlobalUnlock(*lphDefPicts);

      if ((*dDefPictCnt) == 1)
         {
         GlobalFree(*lphDefPicts);
         *lphDefPicts = 0;
         }

      else
         if (!(*lphDefPicts = GlobalReAlloc(*lphDefPicts,
                                   (long)(sizeof(CHECKBOXPICTS) *
                                   (*dDefPictCnt - 1)),
                                   GMEM_MOVEABLE | GMEM_ZEROINIT |
                                   GMEM_SHARE)))
            return (FALSE);

      (*dDefPictCnt)--;
      }

   else
      GlobalUnlock(*lphDefPicts);

   }

return (TRUE);
}


BOOL DLLENTRY tbxCheckBoxGetPict(GLOBALHANDLE FAR *lphDefPicts,
                                 LPSHORT dDefPictCnt, HANDLE hInst,
                                 LPCHECKBOXPICTS lpDefPict)
{
LPCHECKBOXPICTS lpDefPicts;
BOOL            fRet = TRUE;
short           dFound = -1;
short           i;

if (*dDefPictCnt)
   {
   lpDefPicts = (LPCHECKBOXPICTS)GlobalLock(*lphDefPicts);

   for (i = 0; dFound == -1 && i < *dDefPictCnt; i++)
      if (lpDefPicts[i].hInst == hInst)
         dFound = i;

   if (dFound != -1)
      {
      _fmemcpy(lpDefPict, &lpDefPicts[dFound], sizeof(CHECKBOXPICTS));
      fRet = TRUE;
      }
   else
      _fmemset(lpDefPict, '\0', sizeof(CHECKBOXPICTS));

   GlobalUnlock(*lphDefPicts);
   }

return (fRet);
}


void DLLENTRY CheckBoxLoadDefBitmaps(HANDLE hInst)
{
if (dCheckBoxDefPictLoadCnt == 0)
   {
	BOOL fLoadFromRC = TRUE;

#ifdef SS_V70
	HBITMAP hBitmap = LoadBitmap(0, (LPCTSTR)OBM_CHECKBOXES);

	if (hBitmap)
		{
		BITMAP Bitmap;

      GetObject(hBitmap, sizeof(BITMAP), (LPTSTR)&Bitmap);

		// There are 4 columns by 3 rows of pictures

		/* RFW - 1/26/07 - 19755
		if (Bitmap.bmWidth == 52 && Bitmap.bmHeight == 39)
			{
			HBITMAP hBitmaps[7];
			short   i;
			HDC     hDCSrc, hDCDest;
			HBITMAP hBitmapSrcOld, hBitmapDestOld;
			POINT   Points[7] =
				{
				{0,0},   // 0 - Up
				{13,0},  // 1 - Down
				{26,0},  // 2 - Focus Up
				{39,0},  // 3 - Focus Down
				{13,26}, // 4 - Disabled Up
				{26,26}, // 5 - Disabled Down
				{39,26}  // 6 - Grayed Down
				};
		*/

		if (Bitmap.bmWidth > 0 && Bitmap.bmHeight > 0)
			{
			HBITMAP hBitmaps[7];
			short   i;
			HDC     hDCSrc, hDCDest;
			HBITMAP hBitmapSrcOld, hBitmapDestOld;
			int     bmWidth = Bitmap.bmWidth / 4;
			int     bmHeight = Bitmap.bmHeight / 3;
			POINT   Points[7] =
				{
				{0,0},                      // 0 - Up
				{bmWidth,0},                // 1 - Down
				{bmWidth * 2,0},            // 2 - Focus Up
				{bmWidth * 3,0},            // 3 - Focus Down
				{bmWidth,bmHeight * 2},     // 4 - Disabled Up
				{bmWidth * 2,bmHeight * 2}, // 5 - Disabled Down
				{bmWidth * 3,bmHeight * 2}  // 6 - Grayed Down
				};

			hDCSrc = CreateCompatibleDC(0);
			hBitmapSrcOld = SelectObject(hDCSrc, hBitmap);
			hDCDest = CreateCompatibleDC(0);

			for (i = 0; i < 7; i++)
				{
				hBitmaps[i] = CreateBitmap(13, 13, Bitmap.bmPlanes,
                                       Bitmap.bmBitsPixel, NULL);
				hBitmapDestOld = SelectObject(hDCDest, hBitmaps[i]);
				BitBlt(hDCDest, 0, 0, 13, 13, hDCSrc, Points[i].x, Points[i].y, SRCCOPY);
				SelectObject(hDCDest, hBitmapDestOld);
				}

			SelectObject(hDCSrc, hBitmapSrcOld);
			// RFW - 1/26/05 - 15587
			DeleteObject(hDCSrc);
			DeleteObject(hDCDest);

			tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_UP | BT_HANDLE | BT_FREEHANDLE, (LPARAM)hBitmaps[0]);
			tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_DOWN | BT_HANDLE | BT_FREEHANDLE, (LPARAM)hBitmaps[1]);
			tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_FOCUSUP | BT_HANDLE | BT_FREEHANDLE, (LPARAM)hBitmaps[2]);
			tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_FOCUSDOWN | BT_HANDLE | BT_FREEHANDLE, (LPARAM)hBitmaps[3]);
			tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_DISABLEDUP | BT_HANDLE | BT_FREEHANDLE, (LPARAM)hBitmaps[4]);
			tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_DISABLEDDOWN | BT_HANDLE | BT_FREEHANDLE, (LPARAM)hBitmaps[5]);
			tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_GRAY | BT_HANDLE | BT_FREEHANDLE, (LPARAM)hBitmaps[6]);
			fLoadFromRC = FALSE;
			}

		DeleteObject(hBitmap);
		}
#endif // SS_V70

	if (fLoadFromRC)
		{
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_UP,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_Up"));
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_DOWN,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_Down"));
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_GRAY,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_Gray"));
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_FOCUSUP,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_FocusUp"));
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_FOCUSDOWN,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_FocusDown"));
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_FOCUSGRAY,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_FocusGray"));
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_DISABLEDUP,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_DisabledUp"));
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_DISABLEDDOWN,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_DisabledDown"));
		tbCheckBoxLoadDef(hInst, BT_BITMAP | BT_PICT_DISABLEDGRAY,
								(LPARAM)(LPCTSTR)_TEXT("tbCB_DisabledGray"));
		}
   }

dCheckBoxDefPictLoadCnt++;
}


void DLLENTRY CheckBoxUnloadDefBitmaps()
{
dCheckBoxDefPictLoadCnt--;

if (dCheckBoxDefPictLoadCnt == 0)
   tbCheckBoxUnloadDefs(hDynamicInst);
}


static LPCHECKBOX CheckBoxInit(hGlobal)

GLOBALHANDLE FAR *hGlobal;
{
LPCHECKBOX        lpCheckBox;
static WORD FAR   Bits[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

if (!(*hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                             (long)sizeof(CHECKBOX))))
   return (NULL);

lpCheckBox = (LPCHECKBOX)GlobalLock(*hGlobal);

lpCheckBox->hBitmapFocus = CreateBitmap(8, 8, 1, 1, (LPVOID)Bits);
lpCheckBox->hBrushFocus  = CreatePatternBrush(lpCheckBox->hBitmapFocus);
#ifdef SS_V80
lpCheckBox->Enhanced = FALSE;
#endif

return (lpCheckBox);
}


WINENTRY tbCheckBoxWndFn(hWnd, Msg, wParam, lParam)

HWND           hWnd;
UINT           Msg;
WPARAM         wParam;
LPARAM         lParam;
{
return (tbxCheckBoxWndFn(hWnd, Msg, wParam, lParam));
}


WINENTRY tbxCheckBoxWndFn(hWnd, Msg, wParam, lParam)

HWND           hWnd;
UINT           Msg;
WPARAM         wParam;
LPARAM         lParam;
{
GLOBALHANDLE   hGlobal;
HDC            hDCMemory;
PAINTSTRUCT    Paint;
HBITMAP        hBitmapOld;
LPCHECKBOX     lpCheckBox;
HFONT          hFont;
LPTSTR         Text;
RECT           Rect;
HDC            hDC;
long           lRet;
short          Len;
POINT          ptPos;

switch (Msg)
   {
   case WM_CREATE:
      CheckBoxLoadDefBitmaps(hDynamicInst);
      CheckBoxCreate(hWnd, lParam);
      return (0);

   case WM_GETDLGCODE:
      return (DLGC_BUTTON);

   case WM_ENABLE:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      CheckBoxSetDisabled(hWnd, lpCheckBox, (BOOL)!wParam);
      GlobalUnlock(hGlobal);
      return (0);

   case WM_SETFOCUS:
      if (lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal))
         {
         CheckBoxSetFocus(hWnd, lpCheckBox, TRUE);
         GlobalUnlock(hGlobal);
         }

      return (0);

   case WM_KILLFOCUS:
      if (lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal))
         {
         CheckBoxSetFocus(hWnd, lpCheckBox, FALSE);
         CheckBoxSetButtonDown(hWnd, lpCheckBox, FALSE);
         GlobalUnlock(hGlobal);
         }

      return (0);

   case WM_KEYDOWN:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      if (wParam == VK_SPACE && !lpCheckBox->fMouseCaptureOn &&
          !(HIBYTE(HIWORD(lParam)) & 0x0040))
         CheckBoxSetButtonDown(hWnd, lpCheckBox, TRUE);

      GlobalUnlock(hGlobal);
      break;

   case WM_CHAR:
      if (wParam != VK_SPACE && wParam != VK_TAB && wParam != VK_SHIFT &&
          wParam != VK_CONTROL)
         FORWARD_WM_COMMAND(hWnd, GetDlgCtrlID(hWnd), hWnd,
                            BN_CHECKBOXTYPING, tbPostMessageToParent);

      break;

   case WM_USER + 99:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      CheckBoxSetButtonDown(hWnd, lpCheckBox, FALSE);

      if ((lpCheckBox->lStyle & 0x0F) == BS_AUTO3STATE)
         SendMessage(hWnd, BM_SETCHECK, (lpCheckBox->fChecked + 1) % 3, 0L);

      else if ((lpCheckBox->lStyle & 0x0F) == BS_AUTOCHECKBOX)
         CheckBoxSetCheck(hWnd, lpCheckBox, !lpCheckBox->fChecked);

      else if ((lpCheckBox->lStyle & 0x0F) == BS_AUTORADIOBUTTON)
         SendMessage(hWnd, BM_SETCHECK, !lpCheckBox->fChecked, 0L);

      if (lpCheckBox->fFocus)
         FORWARD_WM_COMMAND(hWnd, GetDlgCtrlID(hWnd), hWnd, BN_CLICKED,
                            tbPostMessageToParent);

      GlobalUnlock(hGlobal);
      break;

   case WM_KEYUP:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      if (wParam == VK_SPACE && !lpCheckBox->fMouseCaptureOn &&
          lpCheckBox->fButtonDown)
         {
         CheckBoxSetButtonDown(hWnd, lpCheckBox, FALSE);

         if (((lpCheckBox->lStyle & 0x0F) == BS_AUTOCHECKBOX) ||
             ((lpCheckBox->lStyle & 0x0F) == BS_AUTORADIOBUTTON))
            CheckBoxSetCheck(hWnd, lpCheckBox, !lpCheckBox->fChecked);

         else if ((lpCheckBox->lStyle & 0x0F) == BS_AUTO3STATE)
            SendMessage(hWnd, BM_SETCHECK, (lpCheckBox->fChecked + 1) % 3, 0L);

         if (lpCheckBox->fFocus)
            FORWARD_WM_COMMAND(hWnd, GetDlgCtrlID(hWnd), hWnd,
                               BN_CLICKED, tbPostMessageToParent);
         }

      GlobalUnlock(hGlobal);
      break;

   case WM_MOUSEACTIVATE:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      SetFocus(hWnd);
      GlobalUnlock(hGlobal);
      break;

   case WM_LBUTTONDOWN:
   case WM_LBUTTONDBLCLK:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      if (!lpCheckBox->fMouseCaptureOn)
         {
         SetCapture(hWnd);
         lpCheckBox->fMouseCaptureOn = TRUE;
         CheckBoxSetButtonDown(hWnd, lpCheckBox, TRUE);
         }

      GlobalUnlock(hGlobal);
      break;

   case WM_LBUTTONUP:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      if (lpCheckBox->fMouseCaptureOn)
         {
         ReleaseCapture();
         lpCheckBox->fMouseCaptureOn = FALSE;

         CheckBoxSetButtonDown(hWnd, lpCheckBox, FALSE);
         GetClientRect(hWnd, &Rect);

         ptPos.x = LOWORD(lParam);
         ptPos.y = HIWORD(lParam);
         if (PtInRect(&Rect, ptPos))
            {
            if (((lpCheckBox->lStyle & 0x0F) == BS_AUTOCHECKBOX) ||
                ((lpCheckBox->lStyle & 0x0F) == BS_AUTORADIOBUTTON))
               CheckBoxSetCheck(hWnd, lpCheckBox, !lpCheckBox->fChecked);

            else if ((lpCheckBox->lStyle & 0x0F) == BS_AUTO3STATE)
               SendMessage(hWnd, BM_SETCHECK, (lpCheckBox->fChecked + 1) % 3, 0L);

            FORWARD_WM_COMMAND(hWnd, GetWindowID(hWnd), hWnd,
                               BN_CLICKED, tbPostMessageToParent);
            }
         }

      GlobalUnlock(hGlobal);
      break;

#ifdef SS_V80
   case WM_MOUSELEAVE:
       lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
	   lpCheckBox->MouseOver = FALSE;
       GlobalUnlock(hGlobal);
	   InvalidateRect(hWnd, NULL, FALSE);
	   break;
#endif


   case WM_MOUSEMOVE:
	   {
#ifdef SS_V80
	   TRACKMOUSEEVENT tme;
       lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
	   if (lpCheckBox->MouseOver == FALSE && lpCheckBox->Enhanced)
	   {
		 lpCheckBox->MouseOver = TRUE;
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
         lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

         if (lpCheckBox->fMouseCaptureOn)
            {
            GetClientRect(hWnd, &Rect);

            ptPos.x = LOWORD(lParam);
            ptPos.y = HIWORD(lParam);
            if (PtInRect(&Rect, ptPos))
               CheckBoxSetButtonDown(hWnd, lpCheckBox, TRUE);
            else
               CheckBoxSetButtonDown(hWnd, lpCheckBox, FALSE);
            }

         GlobalUnlock(hGlobal);
         }
	   }
      break;

   case BM_SETPICT:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      if (lpCheckBox->fUseDefPicts)
         {
         _fmemset(&lpCheckBox->Picts, '\0', sizeof(CHECKBOXPICTS));
         lpCheckBox->fUseDefPicts = FALSE;
         }

      CheckBoxSetPict(hWnd, 0, &lpCheckBox->Picts, (WORD)wParam, lParam);
      GlobalUnlock(hGlobal);
      return (0);

#ifdef SS_WIN32S
   case BM_SETSTATE_WIN32S:
#endif
   case BM_SETSTATE:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      CheckBoxSetCheck(hWnd, lpCheckBox, (BOOL)wParam);
      GlobalUnlock(hGlobal);
      return (0);

#ifdef SS_WIN32S
   case BM_SETCHECK_WIN32S:
#endif
   case BM_SETCHECK:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      CheckBoxSetCheck(hWnd, lpCheckBox, (BOOL)wParam);

      GlobalUnlock(hGlobal);
      return (0);

#ifdef SS_WIN32S
   case BM_GETSTATE_WIN32S:
#endif
   case BM_GETSTATE:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      lRet = (long)lpCheckBox->fChecked;
      GlobalUnlock(hGlobal);
      return (lRet);

#ifdef SS_WIN32S
   case BM_GETCHECK_WIN32S:
#endif
   case BM_GETCHECK:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      lRet = (long)lpCheckBox->fChecked;

      GlobalUnlock(hGlobal);
      return (lRet);

#ifdef SS_WIN32S
   case BM_SETSTYLE_WIN32S:
#endif
   case BM_SETSTYLE:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
#ifdef TBPRO
      lpCheckBox->lStyle = (long)wParam;
#else
      lpCheckBox->lStyle = (long)lParam;
#endif
      GlobalUnlock(hGlobal);

#ifdef TBPRO
      if (lParam)
         InvalidateRect(hWnd, NULL, TRUE);
#endif

      return (0);

   case WM_SIZE:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      GetClientRect(hWnd, &Rect);

      if (lpCheckBox->hBitmapDC)
         DeleteObject(lpCheckBox->hBitmapDC);

      /*******************************************************
      * Create Bitmap to select to the memory device context
      *******************************************************/

      hDC = fpGetDC(hWnd);
      lpCheckBox->hBitmapDC = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,
                                                   Rect.bottom - Rect.top);
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

   case WM_DESTROY:
		break;

	/* RFW - 5/12/06
   case WM_DESTROY:
	*/
   case WM_NCDESTROY:
      if (lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal))
         {
         DeleteObject(lpCheckBox->hBrushFocus);
         DeleteObject(lpCheckBox->hBitmapFocus);

         if (lpCheckBox->hText)
            GlobalFree(lpCheckBox->hText);

         if (!lpCheckBox->fUseDefPicts)
            {
            CheckBoxFreePict(&lpCheckBox->Picts.PictUp);
            CheckBoxFreePict(&lpCheckBox->Picts.PictDown);
            CheckBoxFreePict(&lpCheckBox->Picts.PictGray);
            CheckBoxFreePict(&lpCheckBox->Picts.PictFocusUp);
            CheckBoxFreePict(&lpCheckBox->Picts.PictFocusDown);
            CheckBoxFreePict(&lpCheckBox->Picts.PictFocusGray);
            CheckBoxFreePict(&lpCheckBox->Picts.PictDisabledUp);
            CheckBoxFreePict(&lpCheckBox->Picts.PictDisabledDown);
            CheckBoxFreePict(&lpCheckBox->Picts.PictDisabledGray);
            }

         if (lpCheckBox->hBitmapDC)
            DeleteObject(lpCheckBox->hBitmapDC);

         GlobalUnlock(hGlobal);
         GlobalFree(hGlobal);
         CheckBoxSetPtr(hWnd, 0);
         }

      CheckBoxUnloadDefBitmaps();
      return (0);

   case WM_SETFONT:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);
      lpCheckBox->hFont = (HFONT)wParam;
      GlobalUnlock(hGlobal);

      if (lParam)
         {
         InvalidateRect(hWnd, NULL, TRUE);
         UpdateWindow(hWnd);
         }

      return (0);

   case WM_GETFONT:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      if (lpCheckBox->hFont)
         hFont = lpCheckBox->hFont;

      else
         {
         hDC = fpGetDC(hWnd);
         hFont = SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
         SelectObject(hDC, hFont);
         ReleaseDC(hWnd, hDC);
         }

      GlobalUnlock(hGlobal);
      return (LRESULT)hFont;

   case WM_SETTEXT:
      CheckBoxSetText(hWnd, (LPTSTR)lParam);
      return (0);

   case WM_GETTEXT:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      if (lpCheckBox->hText)
         {
         Text = (LPTSTR)GlobalLock(lpCheckBox->hText);
         Len = lstrlen(Text);

         Len = min(Len + 1, (short)wParam);
         _ftcsncpy((LPTSTR)lParam, Text, Len);
         }

      else
         Len = 0;

      GlobalUnlock(hGlobal);
      return (Len);

   case WM_GETTEXTLENGTH:
      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      if (lpCheckBox->hText)
         {
         Text = (LPTSTR)GlobalLock(lpCheckBox->hText);
         Len = lstrlen(Text);
         }

      else
         Len = 0;

      GlobalUnlock(hGlobal);
      return (Len);

   case WM_ERASEBKGND:
      return (TRUE);

   case WM_PAINT:
      hDC = fpBeginPaint(hWnd, &Paint);
      GetClientRect(hWnd, &Rect);

      lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

      /*********************************
      * Create a memory device context
      *********************************/

      hDCMemory = CreateCompatibleDC(hDC);
      hBitmapOld = SelectObject(hDCMemory, lpCheckBox->hBitmapDC);
      SetMapMode(hDCMemory, MM_TEXT);
      SetBkMode(hDCMemory, TRANSPARENT);

      CheckBoxPaint(hWnd, lpCheckBox, hDCMemory, &Rect, FALSE, NULL, TRUE);

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

      GlobalUnlock(hGlobal);
      EndPaint(hWnd, &Paint);
      return (0);
   }

return (DefWindowProc(hWnd, Msg, wParam, lParam));
}


void CheckBoxFreePict(Pict)

LPTBBTNPICT Pict;
{
if (Pict->hPict && Pict->PictType == SUPERBTN_PICT_BITMAP &&
    !Pict->HandleProvided)
   DeleteObject(Pict->hPict);
}


BOOL CheckBoxSetText(hWnd, Text)

HWND         hWnd;
LPCTSTR      Text;
{
GLOBALHANDLE hGlobal;
GLOBALHANDLE hTextTemp = 0;
LPCHECKBOX   lpCheckBox;
LPTSTR       TextTemp;

if (Text && *Text)
	{
	if (!(hTextTemp = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
											(lstrlen(Text) + 1) * sizeof(TCHAR))))
		return (FALSE);

	TextTemp = (LPTSTR)GlobalLock(hTextTemp);
	lstrcpy(TextTemp, Text);
	GlobalUnlock(hTextTemp);
	}

lpCheckBox = CheckBoxGetPtr(hWnd, &hGlobal);

if (lpCheckBox->hText)
	GlobalFree(lpCheckBox->hText);

lpCheckBox->hText = hTextTemp;

GlobalUnlock(hGlobal);

return (TRUE);
}

void CheckBoxPaint(HWND hWnd, LPCHECKBOX lpCheckBox, HDC hDC, LPRECT lpRect,
                   BOOL fUseDib, LPRECT lpRectOrig, BOOL fEraseBkgrnd)
{
COLORREF clrText;
COLORREF clrBk;
HBRUSH   hBrush = INVALID_HANDLE_VALUE;
RECT     RectTemp;

/*****************************************
* Fill the hDCMemory background with the
* current background of the display
*****************************************/

if (fEraseBkgrnd)
   {
   clrText = GetTextColor(hDC);
   clrBk = GetBkColor(hDC);

   #ifdef WIN32
   hBrush = FORWARD_WM_CTLCOLORBTN(GetParent(hWnd), hDC, hWnd, SendMessage);
   #else
   hBrush = FORWARD_WM_CTLCOLOR(GetParent(hWnd), hDC, hWnd, CTLCOLOR_BTN,
                                SendMessage);
   #endif

   FillRect(hDC, lpRect, hBrush);
   }

_fmemcpy(&RectTemp, lpRect, sizeof(RECT));

CheckBoxPaintPict(hWnd, lpCheckBox, &RectTemp, hDC, hDC, hBrush, FALSE,
                  fUseDib, lpRectOrig);
CheckBoxPaintText(hWnd, lpCheckBox, lpRect, &RectTemp, hDC, hDC, hBrush, FALSE,
                  fUseDib, lpRectOrig);

lpCheckBox->cPaint = CHECKBOX_PAINTALL;

if (fEraseBkgrnd)
   {
   SetTextColor(hDC, clrText);
   SetBkColor(hDC, clrBk);
   }
}


void CheckBoxPaintPict(hWnd, lpCheckBox, lpRect, hDCDev, hDC,
                       hBrush, fPaint, fUseDib, lpRectOrig)

HWND       hWnd;
LPCHECKBOX lpCheckBox;
LPRECT     lpRect;
HDC        hDCDev;
HDC        hDC;
HBRUSH     hBrush;
BOOL       fPaint;
BOOL       fUseDib;
LPRECT     lpRectOrig;
{
TBBTNPICT  Pict;
LPTSTR     lpText;
HANDLE     hDIB;
HPALETTE   hPalette;
HBITMAP    hPict;
RECT       RectControl;
RECT       RectPict;
RECT       RectTemp;
HFONT      hFontOld = 0;
BOOL       fDeletePict = FALSE;
short      PictWidth;
short      PictHeight;
short      PictWidthOrig;
short      PictHeightOrig;
short      nTextLen;
short      xOrig;
short      yOrig;
short      Width;
#if defined(_WIN64) || defined(_IA64)
int      x;
int      y;
#else
short      x;
short      y;
#endif
long       lStyle = lpCheckBox->lStyle;
double     dfScaleX;
double     dfScaleY;
int        iMapModeOld;

#ifdef SS_V80
HTHEME hTheme = NULL;
BOOL visualStyles = lpCheckBox->Enhanced && SS_IsThemeActive();
int iSave = SaveDC(hDC);
IntersectClipRect(hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
#endif

if (lpCheckBox->hFont)
   hFontOld = SelectObject(hDC, lpCheckBox->hFont);

_fmemcpy(&RectPict, lpRect, sizeof(RECT));
_fmemcpy(&RectControl, lpRect, sizeof(RECT));

/***************************
* Determine picture to use
***************************/

CheckBoxGetPicture(hWnd, lpCheckBox, &Pict);

if (fUseDib)
   {
   dfScaleX = (double)(lpRect->right - lpRect->left) /
              (double)(lpRectOrig->right - lpRectOrig->left);

   dfScaleY = (double)(lpRect->bottom - lpRect->top) /
              (double)(lpRectOrig->bottom - lpRectOrig->top);

   PictWidth = (short)(Pict.PictWidth * dfScaleX);
   PictHeight = (short)(Pict.PictHeight * dfScaleY);
   }

else
   {
   PictWidth = Pict.PictWidth;
   PictHeight = Pict.PictHeight;
   }

//if (!(lStyle & (SSS_ALIGN_CENTER | SSS_ALIGN_RIGHT)))
//	lStyle = SSS_ALIGN_LEFT;

//Modify by BOC 99.4.22 (hyt)--------------------------------------------
//When BS_LEFTTEXT && SSS_ALIGN_LEFT still need cal width
//if (lStyle & (BS_CENTER | SSS_ALIGN_CENTER | SSS_ALIGN_RIGHT))
if ((lStyle & (BS_CENTER | SSS_ALIGN_CENTER | SSS_ALIGN_RIGHT)) ||
	(lStyle & (BS_LEFTTEXT | SSS_ALIGN_LEFT)) == (BS_LEFTTEXT | SSS_ALIGN_LEFT))
   {
   _fmemcpy(&RectTemp, lpRect, sizeof(RECT));
   Width = 0;

   if (lpCheckBox->hText)
      {
      lpText = (LPTSTR)GlobalLock(lpCheckBox->hText);
		if (*lpText)
			{
			DrawText(hDC, lpText, -1, &RectTemp, DT_TOP | DT_LEFT | DT_SINGLELINE |
				      DT_CALCRECT);

			Width = (short)(RectTemp.right - RectTemp.left);
			}

		GlobalUnlock(lpCheckBox->hText);
      }

   if (fUseDib)
      Width += (short)((Pict.PictWidth + (Width ? BTNTEXTPICT_INC : 0)) *
                        dfScaleX);
   else
      Width += Pict.PictWidth + (Width ? BTNTEXTPICT_INC : 0);

    if((lStyle & (BS_LEFTTEXT | SSS_ALIGN_LEFT)) == (BS_LEFTTEXT | SSS_ALIGN_LEFT))
	   RectPict.right = min(RectPict.right,RectPict.left + Width);
    else
    {
	   if (lStyle & (BS_CENTER | SSS_ALIGN_CENTER))
		  RectPict.left += max((RectPict.right - RectPict.left) - Width, 0) / 2;
	   else
		  RectPict.left = max(RectPict.right - Width, RectPict.left);

	   RectPict.right = min(RectPict.right, RectPict.left + Width);
    }
   }
//----------------------------------------------------------------------------------
_fmemcpy(lpRect, &RectPict, sizeof(RECT));

if (lStyle & BS_LEFTTEXT)
   RectPict.left = RectPict.right - PictWidth;
else
   RectPict.right = RectPict.left + PictWidth;

/***************
* Draw Picture
***************/
#ifdef SS_V80
if (dCheckBoxDefPictCnt)
   {
	LPCHECKBOXPICTS lpDefPicts;
	int i;
	WORD wPictType;
    BOOL b3State;

	lpDefPicts = (LPCHECKBOXPICTS)GlobalLock(hCheckBoxDefPicts);

    b3State = ((lpCheckBox->lStyle & 0x0F) == BS_3STATE) ||
          ((lpCheckBox->lStyle & 0x0F) == BS_AUTO3STATE);

    if (lpCheckBox->fDisabled)
    {
		if (b3State && 2 == lpCheckBox->fChecked)
			wPictType = BT_PICT_DISABLEDGRAY;
		else if (lpCheckBox->fChecked)
			wPictType = BT_PICT_DISABLEDDOWN;
		else
			wPictType = BT_PICT_DISABLEDUP;
	}

	else if (lpCheckBox->fButtonDown)
	{
		if (b3State && 2 == lpCheckBox->fChecked)
			wPictType = BT_PICT_FOCUSGRAY;
		else if (lpCheckBox->fChecked)
			wPictType = BT_PICT_FOCUSDOWN;
		else
			wPictType = BT_PICT_FOCUSUP;
	}

	else
	{
		if (b3State && 2 == lpCheckBox->fChecked)
			wPictType = BT_PICT_GRAY;
		else if (lpCheckBox->fChecked)
			wPictType = BT_PICT_DOWN;
		else
			wPictType = BT_PICT_UP;
	}
	
	for (i = 0; visualStyles == TRUE && i < dCheckBoxDefPictCnt; i++)
		switch(wPictType)
		{
		   case BT_PICT_DISABLEDGRAY:
				if (lpDefPicts[i].PictDisabledGray.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;
		   case BT_PICT_DISABLEDDOWN:
				if (lpDefPicts[i].PictDisabledDown.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;
		   case BT_PICT_DISABLEDUP:
				if (lpDefPicts[i].PictDisabledUp.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;
		   case BT_PICT_FOCUSGRAY:
				if (lpDefPicts[i].PictFocusGray.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;
		   case BT_PICT_FOCUSDOWN:
				if (lpDefPicts[i].PictFocusDown.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;
		   case BT_PICT_FOCUSUP:
				if (lpDefPicts[i].PictFocusUp.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;
		   case BT_PICT_GRAY:
				if (lpDefPicts[i].PictGray.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;
		   case BT_PICT_DOWN:
				if (lpDefPicts[i].PictDown.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;
		   case BT_PICT_UP:
				if (lpDefPicts[i].PictUp.hPict != Pict.hPict)
					visualStyles = FALSE;
			   break;


		}

	GlobalUnlock(hCheckBoxDefPicts);
}


if (visualStyles)
{
	int state = 0;
	BOOL b3State = ((lpCheckBox->lStyle & 0x0F) == BS_3STATE) ||
          ((lpCheckBox->lStyle & 0x0F) == BS_AUTO3STATE);

	if (lpCheckBox->fChecked)
	{
		if (lpCheckBox->fDisabled || (b3State && 2 == lpCheckBox->fChecked))
			state = 8;
		else if (lpCheckBox->MouseOver && lpCheckBox->fFocus && !lpCheckBox->fButtonDown)
			state = 6;
		else if (lpCheckBox->fButtonDown)
			state = 7;
		else 
			state = 5;
	}
	else
	{
		if (lpCheckBox->fDisabled)
			state = 4;
		else if (lpCheckBox->MouseOver && lpCheckBox->fFocus && !lpCheckBox->fButtonDown)
			state = 2;
		else if (lpCheckBox->fButtonDown)
			state = 3;
		else 
			state = 1;
	}
hTheme = SS_OpenThemeData(hWnd, L"Button");
SS_DrawThemeBackground( hTheme, hDC, 3, state, &RectPict, &RectPict);
SS_CloseThemeData(hTheme);
}
else
{
#endif


if (Pict.PictType)
   {
   x = RectPict.left + (RectPict.right - RectPict.left - PictWidth) / 2;

//   if (lStyle & BS_CENTER)
   if (lStyle & SSS_ALIGN_VCENTER)
      y = (short)(RectPict.top + (RectPict.bottom - RectPict.top - PictHeight) / 2);
   else if (lStyle & SSS_ALIGN_TOP)
      y = (short)RectPict.top;
   else if (lStyle & SSS_ALIGN_BOTTOM)
      y = (short)max(RectPict.top, RectPict.bottom - PictHeight);
   else
      y = (short)(RectPict.top + (RectPict.bottom - RectPict.top - PictHeight) / 2);

   if (fUseDib)
      {
      hPict = 0;

      if (Pict.hPict)
         switch (Pict.PictType)
            {
            case SUPERBTN_PICT_ICON:
               SetRect(&RectTemp, 0, 0, tbGetIconWidth(Pict.hPict), tbGetIconHeight(Pict.hPict));
               hPict = tbCreateStretchedIcon(hDC, &RectTemp, GetBkColor(hDC),
                                             Pict.hPict, FALSE);
               fDeletePict = TRUE;
               break;

            case SUPERBTN_PICT_BITMAP:
               hPict = Pict.hPict;
               break;
            }

      hPalette = tbGetSystemPalette();
      hDIB = tbBitmapToDIB(hPict, hPalette);
      DeleteObject(hPalette);

      SetStretchBltMode(hDC, COLORONCOLOR);
      iMapModeOld = SetMapMode(hDC, MM_TEXT);

      x = max(RectPict.left, x);
      y = max(RectPict.top, y);

      PictWidthOrig = min(Pict.PictWidth, (short)((RectPict.right - x) /
                          dfScaleX));
      PictHeightOrig = min(Pict.PictHeight, (short)((RectPict.bottom - y) /
                           dfScaleY));

      xOrig = max((Pict.PictWidth - (short)((RectPict.right - x) /
                   dfScaleX)) / 2, 0);
      yOrig = max((Pict.PictHeight - (short)((RectPict.bottom - y) /
                   dfScaleY)) / 2, 0);

      tbStretchDibBlt(hDC, x, y, min(RectPict.right - x, PictWidth),
                      min(RectPict.bottom - y, PictHeight), hDIB,
                      xOrig, yOrig, PictWidthOrig, PictHeightOrig, SRCCOPY);

      if (fDeletePict)
         DeleteObject(hPict);

      SetMapMode(hDC, iMapModeOld);
      }

   else
      {
      RectPict.left = max(RectPict.left, RectControl.left);
      RectPict.right = min(RectPict.right, RectControl.right);
      switch (Pict.PictType)
         {
         case SUPERBTN_PICT_ICON:
            if (Pict.hPict)
               tbDrawIcon(hDC, &RectPict, GetBkColor(hDC), x, y, Pict.hPict);

            break;

         case SUPERBTN_PICT_BITMAP:
            if (Pict.hPict)
               tbDrawBitmap(hDC, Pict.hPict, &RectPict, x, y, SRCCOPY);

            break;
         }

      nTextLen = 0;
      if (lpCheckBox->hText)
         {
         lpText = (LPTSTR)GlobalLock(lpCheckBox->hText);
         nTextLen = lstrlen(lpText);
         GlobalUnlock(lpCheckBox->hText);
         }

      /*********************************************************
      * If no text and we have focus, then draw the focus rect
      *********************************************************/

      if (lpCheckBox->fFocus && !nTextLen)
         {
         RectTemp.left = max(RectPict.left - 2, RectControl.left);
         RectTemp.right = min(RectPict.right + 2, RectControl.right);
/*
         RectTemp.top = max(((RectPict.bottom - RectPict.top -
                            PictHeight) / 2) - 2, RectControl.top);
//         RectTemp.bottom = min(RectPict.bottom + 2, RectControl.bottom);
         RectTemp.bottom = min(RectTemp.top + 2 + PictHeight + 2,
                               RectControl.bottom);
*/
         RectTemp.top = max(y - 2, RectControl.top);
         RectTemp.bottom = min(y + PictHeight + 2,
                               RectControl.bottom);
         FrameRect(hDC, &RectTemp, lpCheckBox->hBrushFocus);
         }
      }
   }

if (hFontOld)
   SelectObject(hDC, hFontOld);

/*******************************************************
* Copy the memory device context bitmap to the display
*******************************************************/

if (fPaint)
   BitBlt(hDCDev, RectPict.left, RectPict.top, RectPict.right - RectPict.left,
          RectPict.bottom - RectPict.top, hDC, RectPict.left,
          RectPict.top, SRCCOPY);
#ifdef SS_V80
}
RestoreDC(hDC, iSave);
#endif
}


void CheckBoxPaintText(hWnd, lpCheckBox, lpRect, lpRectText, hDCDev, hDC,
                       hBrush, fPaint, fUseDib, lpRectOrig)

HWND         hWnd;
LPCHECKBOX   lpCheckBox;
LPRECT       lpRect;
LPRECT       lpRectText;
HDC          hDCDev;
HDC          hDC;
HBRUSH       hBrush;
BOOL         fPaint;
BOOL         fUseDib;
LPRECT       lpRectOrig;
{
GLOBALHANDLE hTextTemp;
TEXTMETRIC   fm;
HFONT        hFontOld = 0;
HBRUSH       hBrushTemp;
RECT         RectTemp;
RECT         RectText;
LPTSTR       Text;
LPTSTR       TextTemp;
#if defined(_WIN64) || defined(_IA64)
long         PictWidth;
long         TextWidth;
long         TextHeight;
long         Len;
long         At;
long         x;
long         y;
long         i;
#else
short        PictWidth;
short        TextWidth;
short        TextHeight;
short        Len;
short        At;
short        x;
short        y;
short        i;
#endif
int          iBkMode;
TBBTNPICT  Pict;

if (lpCheckBox->hFont)
   hFontOld = SelectObject(hDC, lpCheckBox->hFont);

/**********************************
* Determine Picture and Text size
**********************************/

if (lpCheckBox->hText)
   {
   Text = (LPTSTR)GlobalLock(lpCheckBox->hText);
   Len = lstrlen(Text);
   }
else
   Len = 0;

_fmemcpy(&RectText, lpRectText, sizeof(RECT));

GetTextMetrics(hDC, &fm);
//Modify by BOC 99.4.22 (hyt)-------------------------------
//should use actual picture size not max
/*if (fUseDib)
   PictWidth = (short)((lpCheckBox->Picts.dMaxPictWidth + BTNTEXTPICT_INC) *
                      ((double)(lpRect->right - lpRect->left) /
                      (double)(lpRectOrig->right - lpRectOrig->left)));
else
   PictWidth = lpCheckBox->Picts.dMaxPictWidth + BTNTEXTPICT_INC;
*/
/***************************
* Determine picture to use
***************************/
CheckBoxGetPicture(hWnd, lpCheckBox, &Pict);

if (fUseDib)
   {
   double dfScaleX = (double)(lpRect->right - lpRect->left) /
              (double)(lpRectOrig->right - lpRectOrig->left);

   PictWidth = (short)(Pict.PictWidth * dfScaleX) + BTNTEXTPICT_INC;
   }

else
   PictWidth = Pict.PictWidth + BTNTEXTPICT_INC;
//--------------------------------------------------------

if (Len)
   {
   _fmemcpy(&RectTemp, &RectText, sizeof(RECT));
   DrawText(hDC, Text, -1, &RectTemp, DT_TOP | DT_LEFT | DT_SINGLELINE |
            DT_CALCRECT);
   TextWidth = (short)(RectTemp.right - RectTemp.left);
   }
else
   TextWidth = 0;

if (lpCheckBox->lStyle & BS_LEFTTEXT)
   {
   RectText.right = RectText.right - PictWidth;
   RectText.left = max(RectText.left, RectText.right - TextWidth);
   }

else
   {
   RectText.left = RectText.left + PictWidth;
   RectText.right = min(RectText.right, RectText.left + TextWidth);
   }

/************
* Draw Text
************/

#ifdef  BUGS
// Bug-006
iBkMode = SetBkMode(hDC, TRANSPARENT);
#else
iBkMode = SetBkMode(hDC, OPAQUE);
#endif

if (Len)
   {
   _fmemcpy(&RectTemp, &RectText, sizeof(RECT));
   TextHeight = (short)fm.tmHeight;

   x = RectText.left + max((RectText.right - RectText.left -
       TextWidth) / 2, 0);

//   if (lpCheckBox->lStyle & BS_CENTER)
   if (lpCheckBox->lStyle & SSS_ALIGN_VCENTER)
      y = (short)(RectText.top + max((RectText.bottom - RectText.top -
                  TextHeight) / 2, 0));
   else if (lpCheckBox->lStyle & SSS_ALIGN_TOP)
      y = (short)RectText.top;
   else if (lpCheckBox->lStyle & SSS_ALIGN_BOTTOM)
      y = (short)max(RectText.top, RectText.bottom - TextHeight);
   else
      y = (short)(RectText.top + max((RectText.bottom - RectText.top -
                  TextHeight) / 2, 0));

   RectText.left = x;
   RectText.top = y;
   RectText.right = min(x + TextWidth, RectText.right);
   RectText.bottom = min(y + TextHeight, RectText.bottom);

   if (!lpCheckBox->fDisabled)
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

         hBrushTemp = CreateSolidBrush(GetTextColor(hDC));
         GrayString(hDC, hBrushTemp, NULL, (LPARAM)TextTemp, 0, x, y, 0, 0);
         DeleteObject(hBrushTemp);

         GlobalUnlock(hTextTemp);
         GlobalFree(hTextTemp);
         }
      }

   }

if (lpCheckBox->hText)
   GlobalUnlock(lpCheckBox->hText);

if (lpCheckBox->fFocus && Len)
   {
   RectText.bottom += 1;
   RectText.left -= 1;
   RectText.right += 1;
   FrameRect(hDC, &RectText, lpCheckBox->hBrushFocus);
   }

if (hFontOld)
   SelectObject(hDC, hFontOld);

/*******************************************************
* Copy the memory device context bitmap to the display
*******************************************************/

if (fPaint)
   BitBlt(hDCDev, RectText.left, RectText.top, RectText.right - RectText.left,
          RectText.bottom - RectText.top, hDC, RectText.left,
          RectText.top, SRCCOPY);

SetBkMode(hDC, iBkMode);
}


void CheckBoxGetPicture(hWnd, lpCheckBox, lpPict)

HWND        hWnd;
LPCHECKBOX  lpCheckBox;
LPTBBTNPICT lpPict;
{
WORD wPictType;
BOOL b3State;

b3State = ((lpCheckBox->lStyle & 0x0F) == BS_3STATE) ||
          ((lpCheckBox->lStyle & 0x0F) == BS_AUTO3STATE);

if (lpCheckBox->fDisabled)
   {
   if (b3State && 2 == lpCheckBox->fChecked)
      wPictType = BT_PICT_DISABLEDGRAY;
   else if (lpCheckBox->fChecked)
      wPictType = BT_PICT_DISABLEDDOWN;
   else
      wPictType = BT_PICT_DISABLEDUP;
   }

else if (lpCheckBox->fButtonDown)
   {
   if (b3State && 2 == lpCheckBox->fChecked)
      wPictType = BT_PICT_FOCUSGRAY;
   else if (lpCheckBox->fChecked)
      wPictType = BT_PICT_FOCUSDOWN;
   else
      wPictType = BT_PICT_FOCUSUP;
   }

else
   {
   if (b3State && 2 == lpCheckBox->fChecked)
      wPictType = BT_PICT_GRAY;
   else if (lpCheckBox->fChecked)
      wPictType = BT_PICT_DOWN;
   else
      wPictType = BT_PICT_UP;
   }

CheckBoxPickPicture(lpCheckBox, lpPict, wPictType);
}


void CheckBoxPickPicture(lpCheckBox, lpPict, wPictType)

LPCHECKBOX  lpCheckBox;
LPTBBTNPICT lpPict;
WORD        wPictType;
{
WORD       *lpwPriority;
BOOL        fFound;
short       dPriorityCnt;
short       i;
static WORD wPriorityNormal[]=
   {
   BT_PICT_UP,
   BT_PICT_DOWN,
   BT_PICT_GRAY,
   };
static WORD wPriorityFocus[]=
   {
   BT_PICT_UP,
   BT_PICT_FOCUSUP,
   BT_PICT_DOWN,
   BT_PICT_FOCUSDOWN,
   BT_PICT_GRAY,
   BT_PICT_FOCUSGRAY,
   };
static WORD wPriorityDisabled[]=
   {
   BT_PICT_UP,
   BT_PICT_DISABLEDUP,
   BT_PICT_DOWN,
   BT_PICT_DISABLEDDOWN,
   BT_PICT_GRAY,
   BT_PICT_DISABLEDGRAY,
   };

if (lpCheckBox->fDisabled)
   {
   lpwPriority = wPriorityDisabled;
   dPriorityCnt = sizeof(wPriorityDisabled) / sizeof(WORD);
   }
else if (lpCheckBox->fButtonDown)
   {
   lpwPriority = wPriorityFocus;
   dPriorityCnt = sizeof(wPriorityFocus) / sizeof(WORD);
   }
else
   {
   lpwPriority = wPriorityNormal;
   dPriorityCnt = sizeof(wPriorityNormal) / sizeof(WORD);
   }

for (i = dPriorityCnt - 1, fFound = FALSE; i >= 0; i--)
   {
   if (lpwPriority[i] == wPictType)
      fFound = TRUE;

   if (fFound)
      if (CheckBoxChoosePicture(lpCheckBox, lpPict, lpwPriority[i]))
         break;
   }

if (fFound < 0)
   _fmemcpy(lpPict, &lpCheckBox->Picts.PictUp, sizeof(TBBTNPICT));
}


BOOL CheckBoxChoosePicture(lpCheckBox, lpPict, wPictType)

LPCHECKBOX  lpCheckBox;
LPTBBTNPICT lpPict;
WORD        wPictType;
{
LPTBBTNPICT lpPictTemp;

lpPictTemp = CheckBoxGetPicturePtr(&lpCheckBox->Picts, wPictType);

if (lpPictTemp->PictType)
   {
   _fmemcpy(lpPict, lpPictTemp, sizeof(TBBTNPICT));
   return (TRUE);
   }

_fmemset(lpPict, '\0', sizeof(TBBTNPICT));
return (FALSE);
}


LPTBBTNPICT CheckBoxGetPicturePtr(lpPicts, wPictType)

LPCHECKBOXPICTS lpPicts;
WORD            wPictType;
{
LPTBBTNPICT     lpPict;

switch (wPictType)
   {
   case BT_PICT_UP:
      lpPict = &lpPicts->PictUp;
      break;

   case BT_PICT_DOWN:
      lpPict = &lpPicts->PictDown;
      break;

   case BT_PICT_GRAY:
      lpPict = &lpPicts->PictGray;
      break;

   case BT_PICT_FOCUSUP:
      lpPict = &lpPicts->PictFocusUp;
      break;

   case BT_PICT_FOCUSDOWN:
      lpPict = &lpPicts->PictFocusDown;
      break;

   case BT_PICT_FOCUSGRAY:
      lpPict = &lpPicts->PictFocusGray;
      break;

   case BT_PICT_DISABLEDUP:
      lpPict = &lpPicts->PictDisabledUp;
      break;

   case BT_PICT_DISABLEDDOWN:
      lpPict = &lpPicts->PictDisabledDown;
      break;
  
   case BT_PICT_DISABLEDGRAY:
      lpPict = &lpPicts->PictDisabledGray;
      break;
   }

return (lpPict);
}


void CheckBoxSetPtr(hWnd, hGlobal)

HWND         hWnd;
GLOBALHANDLE hGlobal;
{
#if defined(_WIN64) || defined(_IA64)
SetWindowLongPtr(hWnd, 4, (LONG_PTR)hGlobal);
#else
SetWindowLong(hWnd, 4, (LONG)hGlobal);
#endif
}


LPCHECKBOX CheckBoxGetPtr(hWnd, hGlobal)

HWND              hWnd;
GLOBALHANDLE FAR *hGlobal;
{
#if defined(_WIN64) || defined(_IA64)
if (!(*hGlobal = (GLOBALHANDLE)GetWindowLongPtr(hWnd, 4)))
#else
if (!(*hGlobal = (GLOBALHANDLE)GetWindowLong(hWnd, 4)))
#endif
   return (NULL);

return ((LPCHECKBOX)GlobalLock(*hGlobal));
}


void CheckBoxSetDisabled(hWnd, lpCheckBox, fValue)

HWND       hWnd;
LPCHECKBOX lpCheckBox;
BOOL       fValue;
{
if (lpCheckBox->fDisabled != fValue)
   {
   lpCheckBox->fDisabled = fValue;
   InvalidateRect(hWnd, NULL, FALSE);
   }
}


void CheckBoxSetButtonDown(hWnd, lpCheckBox, fValue)

HWND       hWnd;
LPCHECKBOX lpCheckBox;
BOOL       fValue;
{
if (lpCheckBox->fButtonDown != fValue)
   {
   lpCheckBox->cPaint = CHECKBOX_PAINTPICT;
   lpCheckBox->fButtonDown = fValue;
   InvalidateRect(hWnd, NULL, FALSE);
   }
}


void CheckBoxSetCheck(hWnd, lpCheckBox, fValue)

HWND       hWnd;
LPCHECKBOX lpCheckBox;
BOOL       fValue;
{
if (lpCheckBox->fChecked != fValue)
   {
   lpCheckBox->cPaint = CHECKBOX_PAINTPICT;
   lpCheckBox->fChecked = fValue;
   InvalidateRect(hWnd, NULL, FALSE);
   }
}


void CheckBoxSetFocus(hWnd, lpCheckBox, fValue)

HWND       hWnd;
LPCHECKBOX lpCheckBox;
BOOL       fValue;
{
if (lpCheckBox->fFocus != fValue)
   {
   lpCheckBox->fFocus = fValue;
   InvalidateRect(hWnd, NULL, FALSE);
   UpdateWindow(hWnd);
   }
}


void CheckBoxSetPict(HWND hWnd, HINSTANCE hInst, LPCHECKBOXPICTS lpPicts,
                     WPARAM wParam, LPARAM lParam)
{
LPTBBTNPICT lpPict;
WORD        wPictType;

lpPict = CheckBoxGetPicturePtr(lpPicts, (WORD)(wParam &
                               (BT_PICT_UP | BT_PICT_DOWN | BT_PICT_GRAY |
                                BT_PICT_FOCUSUP | BT_PICT_FOCUSDOWN |
                                BT_PICT_FOCUSGRAY | BT_PICT_DISABLEDUP |
                                BT_PICT_DISABLEDDOWN |
                                BT_PICT_DISABLEDGRAY
                                )));

wPictType = 0;

if (wParam & BT_BITMAP)
   wPictType |= SUPERBTN_PICT_BITMAP;

if (wParam & BT_ICON)
   wPictType |= SUPERBTN_PICT_ICON;

if (wParam & BT_HANDLE)
   wPictType |= SUPERBTN_PICT_HANDLE;

#ifdef SS_V70
if (wParam & BT_FREEHANDLE)
	wPictType|= BT_FREEHANDLE;
#endif // SS_V70

SuperBtnSetPict(hWnd, hInst, lpPict, wPictType, lParam);

lpPicts->dMaxPictWidth = 0;
lpPicts->dMaxPictHeight = 0;

CheckBoxSetPictMax(lpPicts, &lpPicts->PictUp);
CheckBoxSetPictMax(lpPicts, &lpPicts->PictDown);
CheckBoxSetPictMax(lpPicts, &lpPicts->PictGray);
CheckBoxSetPictMax(lpPicts, &lpPicts->PictFocusUp);
CheckBoxSetPictMax(lpPicts, &lpPicts->PictFocusDown);
CheckBoxSetPictMax(lpPicts, &lpPicts->PictFocusGray);
CheckBoxSetPictMax(lpPicts, &lpPicts->PictDisabledUp);
CheckBoxSetPictMax(lpPicts, &lpPicts->PictDisabledDown);
CheckBoxSetPictMax(lpPicts, &lpPicts->PictDisabledGray);
}


void CheckBoxSetPictMax(LPCHECKBOXPICTS lpPicts, LPTBBTNPICT lpPict)
{
if (lpPict->PictType)
   {
   lpPicts->dMaxPictWidth = max(lpPicts->dMaxPictWidth, lpPict->PictWidth);
   lpPicts->dMaxPictHeight = max(lpPicts->dMaxPictHeight, lpPict->PictHeight);
   }
}


long CheckBoxCreate(HWND hWnd, LPARAM lParam)
{
GLOBALHANDLE   hGlobal;
LPCREATESTRUCT lpcsData;
LPCHECKBOX     lpCheckBox;
RECT           Rect;
HDC            hDC;

lpCheckBox = CheckBoxInit(&hGlobal);
CheckBoxSetPtr(hWnd, hGlobal);

tbCheckBoxGetPict(hDynamicInst, &lpCheckBox->Picts);
tbCheckBoxGetPict(GetWindowInstance(hWnd), &lpCheckBox->Picts);

lpCheckBox->fUseDefPicts = TRUE;
lpCheckBox->dCtlColorCode = CTLCOLOR_BTN;

lpCheckBox->lStyle = GetWindowLong(hWnd, GWL_STYLE);

lpcsData = (LPCREATESTRUCT)lParam;
CheckBoxSetText(hWnd, (LPTSTR)lpcsData->lpszName);

GetClientRect(hWnd, &Rect);

/*******************************************************
* Create Bitmap to select to the memory device context
*******************************************************/

hDC = fpGetDC(hWnd);
lpCheckBox->hBitmapDC = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,
                                             Rect.bottom - Rect.top);
ReleaseDC(hWnd, hDC);

if (((LPCREATESTRUCT)lParam)->style & WS_DISABLED)
    lpCheckBox->fDisabled = TRUE;

GlobalUnlock(hGlobal);

return (0);
}
