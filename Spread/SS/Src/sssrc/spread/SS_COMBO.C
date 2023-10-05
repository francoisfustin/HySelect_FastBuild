/*********************************************************
* SS_COMBO.C
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

#ifndef WINVER
#define WINVER  0x0500      /* version 5.0 */
#endif /* !WINVER */

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <process.h>
#include "spread.h"
#include "fphdc.h"
#include "list.h"    // this file has been added to fplibs/fptools/src - CTF
#ifdef SS_V80
#include "uxtheme.h"
#endif

#ifndef SS_NOCT_COMBO

#ifdef SS_V30
//#include "list.h"  // this file is already included above - CTF
#include "combo.h"   // this file has been added to fplibs/fptools/src - CTF
#endif
#include "ss_alloc.h"
#include "ss_draw.h"
#include "ss_font.h"
#include "ss_main.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_w32s.h"
#include "..\edit\editfld.h"
#include "..\classes\wintools.h"
#include "zmouse.h"

#define SS_CB_ISDROPDOWN(Style) ((Style & 0x03) == SS_CB_DROPDOWN)

#if 0

#ifdef WIN32

#define GWL_LISTBOX  0
#define GWW_BTNDOWN  4
#define GWL_HWNDEDIT 6
#define GWW_CURSEL   10
#define GWW_MAX      12
#define SSCB_GetListhWnd(hWnd) ((HWND)GetWindowLong(hWnd, GWL_LISTBOX))
#define SSCB_GetEdithWnd(hWnd) ((HWND)GetWindowLong(hWnd, GWL_HWNDEDIT))
#define SetWindowListBox(hWnd, hWndCtl) \
            SetWindowLong(hWnd, GWL_LISTBOX, (LONG)hWndCtl)
#define SetWindowEdit(hWnd, hWndCtl) \
            SetWindowLong(hWnd, GWL_HWNDEDIT, (LONG)hWndCtl)

#else

#define GWW_LISTBOX  0
#define GWW_BTNDOWN  2
#define GWW_HWNDEDIT 4
#define GWW_CURSEL   6
#define GWW_MAX      8
#define SSCB_GetListhWnd(hWnd) ((HWND)GetWindowWord(hWnd, GWW_LISTBOX))
#define SSCB_GetEdithWnd(hWnd) ((HWND)GetWindowWord(hWnd, GWW_HWNDEDIT))
#define SetWindowListBox(hWnd, hWndCtl) \
            SetWindowWord(hWnd, GWW_LISTBOX, (WORD)hWndCtl)
#define SetWindowEdit(hWnd, hWndCtl) \
            SetWindowWord(hWnd, GWW_HWNDEDIT, (WORD)hWndCtl)

#endif

#endif

#define SS_COMBOAUTOSEARCHTEXT_MAX 50

typedef struct tagSS_COMBO
   {
   HWND    hWndList;
   HWND    hWndListActive;
   HWND    hWndEdit;
   HWND    hWndListProCombo;
   BOOL    fBtnDown;
   long    nCurSel;
   long    lHorzExtent;
   BOOL    fInternalENChange;
   BOOL    fIgnoreSelChange;
#ifdef SS_V40
	TCHAR   szAutoSearchText[SS_COMBOAUTOSEARCHTEXT_MAX + 1];
#endif // SS_V40
#ifdef SS_V80
	BOOL MouseOver;
#endif
   } SS_COMBO, FAR *LPSS_COMBO;

/*
#define SSCB_GetCurSel(hWnd) ((short)GetWindowWord(hWnd, GWW_CURSEL))
#define SSCB_SetCurSel(hWnd, dCurSel) \
            SetWindowWord(hWnd, GWW_CURSEL, (WORD)dCurSel)
*/

extern HANDLE  hDynamicInst;
extern FARPROC lpfnDeskTopProc;
extern FARPROC lpfnSSDeskTopProc;

//#ifdef SPREAD_JPN
// JPNFIX0002 - (Masanori Iwasa)
FARPROC     lpfnWndHookProc = 0;
HHOOK       hWndHook;
HWND        hWndCombo;

LRESULT  _export CALLBACK HandleWnd(int Code, WPARAM wParam, LPARAM lParam);
//#endif

WINENTRY   tbSSComboBoxWndFn(HWND hWnd, UINT Msg, WPARAM wParam,
                             LPARAM lParam);
LRESULT    SS_ComboBoxWndFn(HWND hWnd, LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, LPARAM lParam);
void       SS_ComboPaint(HWND hWndCombo, LPSPREADSHEET lpSS, HDC hDC, LPRECT lpRect);
WINENTRY   SS_EditComboBoxProc(HWND hWnd, UINT Msg, WPARAM wParam,
                               LPARAM lParam);
BOOL       SS_RegisterEditComboBox(HANDLE hInstance);
BOOL       SS_UnRegisterEditComboBox(HANDLE hInstance);
void       SS_ComboSubclassDesktop(HWND hWnd);
void       SS_ComboUnSubclassDesktop(void);
LPSS_COMBO SSCB_Lock(HWND hWnd);
void       SSCB_Unlock(HWND hWnd);
HWND       SSCB_GetListhWnd(HWND hWndCombo);
HWND       SSCB_GetEdithWnd(HWND hWndCombo);
long       SSCB_GetCurSel(HWND hWndCombo);
void       SSCB_SetCurSel(HWND hWndCombo, long nCurSel);
BOOL       SSCB_GetBtnDown(HWND hWndCombo);
void       SSCB_SetBtnDown(HWND hWndCombo, BOOL fBtnDown);
HGLOBAL    SSCB_GetTextFromList(HWND hWnd, long dSel);
void       SSCB_ClearAutoSearchText(HWND hWnd);
void       SS_GetMonitorRect(LPRECT lprcCtl, LPRECT lprcMonitor);

static FARPROC lpfnEditComboBoxProc = 0;


BOOL tbSSRegisterComboBox(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = TRUE;

wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
wc.lpfnWndProc   = (WNDPROC)tbSSComboBoxWndFn;
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
wc.lpszClassName = SSClassNames[dClassNameIndex].TBComboBox;
if (!RegisterClass((LPWNDCLASS)&wc))
   bRet = FALSE;
SS_RegisterEditComboBox(hInstance);
return bRet;
}


BOOL tbSSUnRegisterComboBox(HANDLE hInstance)
{
SS_UnRegisterEditComboBox(hInstance);
return (UnregisterClass(SSClassNames[dClassNameIndex].TBComboBox, hInstance));
}

//#ifdef SPREAD_JPN
//---------------------------------------------------------------------
// This mouse hook routine rolls up the combo box list window.
// Subclass wasn't used because of some other problems.
// JPNFIX0002 - (Masanori Iwasa)
//---------------------------------------------------------------------
LRESULT  _export CALLBACK HandleWnd(int Code, WPARAM wParam, LPARAM lParam)
{
    HWND    hWnd = ((LPMOUSEHOOKSTRUCT)lParam)->hwnd;
    LRESULT lResult;

    if(Code < 0)
        return CallNextHookEx(hWndHook, Code, wParam, lParam);

    if(Code == HC_ACTION)
        {
        //- Don't check for mouse click in combo box related windows.
        if (hWnd != hWndCombo && hWnd != SSCB_GetListhWnd(hWndCombo) &&
            GetParent(hWnd) != SSCB_GetListhWnd(hWndCombo))
            {
            if (wParam == WM_LBUTTONDOWN || wParam == WM_NCLBUTTONDOWN ||
                            wParam == WM_LBUTTONDBLCLK)
                {
                lResult = CallNextHookEx(hWndHook, Code, wParam, lParam);
                //- Roll up the listbox and update the cell.
                SendMessage(hWndCombo, CB_SHOWDROPDOWN, FALSE, 0L);
                InvalidateRect(hWndCombo, NULL, FALSE);
                return (lResult);
                }
            }
        }
    return CallNextHookEx(hWndHook, Code, wParam, lParam);
}
//#endif


WINENTRY tbSSComboBoxWndFn(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPSPREADSHEET lpSS;
HWND          hWndSS = GetParent(hWnd);
LRESULT       lRet;

lpSS = SS_SheetLockActive(hWndSS);
lRet = SS_ComboBoxWndFn(hWnd, lpSS, Msg, wParam, lParam);
SS_SheetUnlockActive(hWndSS);

return (lRet);
}


LRESULT SS_ComboBoxWndFn(HWND hWnd, LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, LPARAM lParam)
{
SS_COLORTBLITEM BackColorTblItem;
SS_COLORTBLITEM ForeColorTblItem;
LPSS_COMBO      lpCombo;
SS_CELLTYPE     CellType;
PAINTSTRUCT     Paint;
GLOBALHANDLE    hText;
GLOBALHANDLE    hCombo;
TEXTMETRIC      fm;
HBITMAP         hBitmapOld;
HBITMAP         hBitmapDC;
LPSS_CELL       Cell;
LPSS_FONT       Font;
LPTSTR          lpText;
SS_FONT         FontTemp;
HFONT           hFontOld;
HDC             hDCMemory;
HDC             hDC;
RECT            Rect;
HWND            hWndCtrl;
HWND            hWndEdit;
//GAB - The following 5 variables were changed to long to fix JUW150
// I also changed all of the places an assignment was cast to short
long			    dItemCnt;
long			    dCurSel;
long			    dCnt;
long			    dSel;
long			    dSelOld;
short           dLen;
short           dHeight;
short           dItemsShown;
short           x;
short           y;

switch (Msg)
   {
   case WM_CREATE:
      if (hCombo = GlobalAlloc(GHND, sizeof(SS_COMBO)))
         {
#if defined(_WIN64) || defined(_IA64)
         SetWindowLongPtr(hWnd, 0, (LONG_PTR)hCombo);
#else
         SetWindowLong(hWnd, 0, (LONG)hCombo);
#endif
         lpCombo = SSCB_Lock(hWnd);

			SS_RegisterListBox(hDynamicInst);

   //      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSListBox, NULL,
   //                              WS_BORDER | WS_CHILD | WS_VSCROLL, 0, 0, 0, 0,
   //                              GetDesktopWindow(), 0, hDynamicInst, NULL);
//#ifdef SS_V30
//         hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSListBox, NULL,
//                                 LBS_NOINTEGRALHEIGHT | WS_BORDER | WS_POPUP | WS_VSCROLL | WS_HSCROLL,
//                                 0, 0, 0, 0, hWnd, 0, hDynamicInst, NULL);
//#else
         hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSListBox, NULL,
                                 LBS_NOINTEGRALHEIGHT | WS_BORDER | WS_POPUP | WS_VSCROLL,
                                 0, 0, 0, 0, hWnd, 0, hDynamicInst, NULL);
//#endif

         lpCombo->hWndList = hWndCtrl;
         lpCombo->hWndListActive = lpCombo->hWndList;
         SetProp(lpCombo->hWndList, _T("HWNDSPREAD"), GetParent(hWnd));
         SetProp(lpCombo->hWndList, _T("HWNDCOMBO"), hWnd);

         SendMessage(lpCombo->hWndList, WM_SETFONT, (WPARAM)SendMessage(hWnd, WM_GETFONT,
                     0, 0L), 0L);

			SS_RegisterEditComboBox(hDynamicInst);
         if (lpCombo->hWndEdit = CreateWindow(SSClassNames[dClassNameIndex].SSEditComboBox,
                                 NULL, WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0,
                                 hWnd, 0, hDynamicInst, NULL))
            SendMessage(lpCombo->hWndEdit, EM_LIMITTEXT, 150, 0L);

         SSCB_Unlock(hWnd);
         }

      break;

   case WM_SYSKEYDOWN:
      hWndCtrl = SSCB_GetListhWnd(hWnd);

      if ((dCnt = (long)SendMessage(hWndCtrl, LB_GETCOUNT, 0, 0L)) > 0)
         {
         if (((lParam >> 29) & 0x01) && (wParam == VK_UP || wParam == VK_DOWN))
            if (!SSCB_GetBtnDown(hWnd))
               {
               lpSS->lpBook->EditModeTurningOn = TRUE;
               SendMessage(hWnd, CB_SHOWDROPDOWN, TRUE, 0L);
               lpSS->lpBook->EditModeTurningOn = FALSE;
               }
            else
               {
               SendMessage(hWnd, CB_SHOWDROPDOWN, FALSE, 0L);
               InvalidateRect(hWnd, NULL, FALSE);
               UpdateWindow(hWnd);
               }
         }

      break;

   case WM_KEYDOWN:
      hWndCtrl = SSCB_GetListhWnd(hWnd);

      SS_RetrieveCellType(lpSS, &CellType, NULL,
                          lpSS->Col.EditAt, lpSS->Row.EditAt);

#ifdef SS_V40
		if (CellType.Spec.ComboBox.AutoSearch == LB_AUTOSEARCH_MULTIPLECHAR &&
			 (wParam == VK_CLEAR || wParam == VK_ESCAPE || wParam == VK_PRIOR ||
			  wParam == VK_NEXT || wParam == VK_END || wParam == VK_HOME ||
			  wParam == VK_LEFT || wParam == VK_UP || wParam == VK_RIGHT ||
			  wParam == VK_DOWN))
			SSCB_ClearAutoSearchText(hWnd);

		if (wParam == VK_BACK)
			{
			LPSS_COMBO lpCombo = SSCB_Lock(hWnd);
 
			if (lpCombo->hWndList == lpCombo->hWndListActive)
				{
				if (CellType.Spec.ComboBox.AutoSearch == LB_AUTOSEARCH_MULTIPLECHAR &&
					 *lpCombo->szAutoSearchText)
					{
					LPTSTR lpszLastChar;
					long lSearchIndex;
					long lSel = (long)SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);

					lpszLastChar = CharPrev(lpCombo->szAutoSearchText, &lpCombo->szAutoSearchText[lstrlen(lpCombo->szAutoSearchText)]);
					if (lpszLastChar)
						*lpszLastChar = '\0';

					if (lpCombo->szAutoSearchText[0] == 0)
						lSearchIndex = 0;
					else
						{
						SendMessage(hWndCtrl, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)lpCombo->szAutoSearchText);
						lSearchIndex = (long)SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
						}
					if (lSel != lSearchIndex)
						{
						FORWARD_WM_COMMAND(hWnd, GetWindowID(hWndCtrl), hWndCtrl,
												 LBN_SELCHANGE, SendMessage);
						InvalidateRect(hWnd, NULL, FALSE);
						UpdateWindow(hWnd);
						}
					}

				SSCB_Unlock(hWnd);
				return (0);
				}

			SSCB_Unlock(hWnd);
			}
#endif // SS_V40

		// RFW - 6/25/04 - 14438
      if (SS_IsActionKey(lpSS->lpBook, SS_KBA_CLEAR, (WORD)wParam))
         {
         SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)-1, 0L);
         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)_TEXT(""));
			InvalidateRect(hWnd, NULL, TRUE);
         return (0);
         }

		// RFW - 6/25/04 - 14438
		if (wParam == VK_F4)
			{
			if (SSCB_GetBtnDown(hWndCombo))
				SendMessage(hWndCombo, CB_SHOWDROPDOWN, FALSE, 0L);
			else
				SendMessage(hWndCombo, CB_SHOWDROPDOWN, TRUE, 0L);
			}

      if (SSCB_GetBtnDown(hWnd))
         {
         if (wParam == VK_ESCAPE)
            SendMessage(hWnd, CB_SHOWDROPDOWN, FALSE, 0L);
         else
            {
            lpSS->lpBook->fComboKeyDown = TRUE;
            SendMessage(hWndCtrl, Msg, wParam, lParam);
            lpSS->lpBook->fComboKeyDown = FALSE;
            }

         return (0);
         }

      if ((dCnt = (long)SendMessage(hWndCtrl, LB_GETCOUNT, 0, 0L)) > 0)
         {
         dSel = (long)SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
         dSelOld = dSel;

         switch (wParam)
            {
            case VK_UP:
            case VK_LEFT:
               if (dSel != 0)
                  {
                  dSel = max(dSel - 1, 0);
                  SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)dSel, 0L);
                  }

               break;

            case VK_DOWN:
            case VK_RIGHT:
               if (dSel != dCnt - 1)
                  {
                  dSel++;
                  SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)dSel, 0L);
                  }

               break;

            case VK_HOME:
               if (dSel != 0)
                  {
                  dSel = 0;
                  SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)dSel, 0L);
                  }

               break;

            case VK_END:
               if (dSel != dCnt - 1)
                  {
                  dSel = dCnt - 1;
                  SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)dSel, 0L);
                  }
               break;

            case VK_BACK:
					// fix for #8976 -scl
               SendMessage(hWndCtrl, Msg, wParam, lParam);
               break;
            }

         if (dSelOld != dSel)
            {
            if (hText = SSCB_GetTextFromList(hWnd, dSel))
               {
               HWND hWndSS = GetParent(hWnd);
// fix for bug 4913
				   LPSS_COMBO lpCombo = SSCB_Lock(hWnd);

               lpText = (LPTSTR)GlobalLock(hText);

               SS_RetrieveCellType(lpSS, &CellType, NULL,
                                   lpSS->Col.EditAt, lpSS->Row.EditAt);

               if (SS_CB_ISDROPDOWN(CellType.Style))
                  {
                  hWndEdit = SSCB_GetEdithWnd(hWnd);
                  SetWindowText(hWndEdit, lpText);
                  }
				   else if( !SendMessage(hWnd, CB_GETDROPPEDSTATE, 0, 0L)) 
				   // fire the EditChange event if list is NOT dropped down
#ifdef WIN32
					   SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetWindowID(hWnd), CBN_EDITCHANGE), (LPARAM)hWnd);
#else
					   SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)GetWindowID(hWnd), MAKELPARAM(hWnd, CBN_EDITCHANGE));
#endif

					SSCB_Unlock(hWnd);
					if (lpCombo->hWndList == lpCombo->hWndListActive)
						SS_SendMsgCoords(lpSS, SSM_COMBOSELCHANGE,
                                   GetDlgCtrlID(hWndSS),
                                   lpSS->Col.EditAt, lpSS->Row.EditAt);

               GlobalUnlock(hText);
               GlobalFree(hText);
               }

            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
            }
         }

      break;

   case WM_CHAR:
      {
      long lSel;
		if ((int)wParam < 32)
			return (0);

		lpCombo = SSCB_Lock(hWnd);

      SS_RetrieveCellType(lpSS, &CellType, NULL,
                          lpSS->Col.EditAt, lpSS->Row.EditAt);

      if (SS_CB_ISDROPDOWN(CellType.Style) || lpCombo->hWndList != lpCombo->hWndListActive)
			{
			if (SS_CB_ISDROPDOWN(CellType.Style))
				hWndCtrl = SSCB_GetEdithWnd(hWnd);
			else
				hWndCtrl = SSCB_GetListhWnd(hWnd);

			lpSS->lpBook->fComboKeyDown = TRUE;
			if(!_TIsDBCSLeadByte((BYTE)wParam)) // RFW - 2/21/-07 - 19761
#if defined(SPREAD_JPN) && !defined(_UNICODE) // RFW - 2/21/-07 - 19761
				PostMessage(hWndCtrl, Msg, wParam, lParam);
#else
				SendMessage(hWndCtrl, Msg, wParam, lParam);
#endif
			else
				{
				MSG nMsg;
				PeekMessage(&nMsg, hWnd, WM_CHAR, WM_CHAR, PM_REMOVE);
				PostMessage(hWndCtrl, Msg, wParam, lParam);
				PostMessage(hWndCtrl, Msg, nMsg.wParam, nMsg.lParam);
				}
			lpSS->lpBook->fComboKeyDown = FALSE;
			}
      else
			{
         TCHAR szFind[SS_COMBOAUTOSEARCHTEXT_MAX + 1];
			long lSelNew = -1;
			long lSelOld;
			short nAt = 0;
         hWndCtrl = SSCB_GetListhWnd(hWnd);
			lSel = (long)SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
			lSelOld = lSel;
			szFind[0] = '\0';

#ifdef SS_V40
			if (CellType.Spec.ComboBox.AutoSearch == SS_COMBOBOX_AUTOSEARCH_NONE)
				nAt = -1;

			else if (CellType.Spec.ComboBox.AutoSearch == SS_COMBOBOX_AUTOSEARCH_MULTIPLECHAR)
				{
				lstrcpy(szFind, lpCombo->szAutoSearchText);
				nAt = lstrlen(szFind);

				// RFW - 6/23/04 - 14671
				if (nAt == 0)
					lSel = -1;

				if (lSel >= 0)
					lSel--;
				}

			else if (CellType.Spec.ComboBox.AutoSearch == SS_COMBOBOX_AUTOSEARCH_SINGLECHARGREATER)
				;

			else // SS_COMBO_AUTOSEARCH_SINGLECHAR
				;
#endif // SS_V40

			if (nAt >= 0)
				{
				if(!IsDBCSLeadByte((BYTE)wParam))
					szFind[nAt++] = (TCHAR)wParam;
				else
					{
					MSG nMsg;
					PeekMessage(&nMsg, hWnd, WM_CHAR, WM_CHAR, PM_REMOVE);
					szFind[nAt++] = (TCHAR)wParam;
					szFind[nAt++] = (TCHAR)nMsg.wParam;
					}

				szFind[nAt] = '\0';

#ifdef SS_V40
				if (CellType.Spec.ComboBox.AutoSearch == SS_COMBOBOX_AUTOSEARCH_SINGLECHARGREATER)
					{
					if (CellType.Spec.ComboBox.hItems)
						{
						LPTSTR lpItemsStart = (LPTSTR)tbGlobalLock(CellType.Spec.ComboBox.hItems);
						LPTSTR lpItems = lpItemsStart;
						long   lStart;
						long   i;
						BOOL   fBegin = FALSE;

						/* RFW - 6/23/04 - 14669
						for (i = 0; lSelNew == -1 && i < CellType.Spec.ComboBox.dItemCnt; i++)
						*/
						lStart = (lSel < CellType.Spec.ComboBox.dItemCnt - 1 ? lSel + 1 : 0);
						i = 0;
						do
							{
							if (i == lStart)
								fBegin = TRUE;

							if (fBegin && _tcsnicmp(szFind, lpItems, nAt) <= 0)
								{
								lSelNew = i;
		                  SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)lSelNew, 0L);
								if (lSelOld != lSelNew)
									FORWARD_WM_COMMAND(hWnd, GetWindowID(hWndCtrl), hWndCtrl,
															 LBN_SELCHANGE, SendMessage);
								}

							if (i < CellType.Spec.ComboBox.dItemCnt - 1)
								{
								i++;
								lpItems += lstrlen(lpItems) + 1;
								}
							else
								{
								i = 0;
								lpItems = lpItemsStart;
								}

							} while (lSelNew == -1 && (!fBegin || i != lStart));

						tbGlobalUnlock(CellType.Spec.ComboBox.hItems);
						}
					}
				else
#endif // SS_V40
					{
					lSelNew = (long)SendMessage(hWndCtrl, LB_SELECTSTRING, (WPARAM)(lSel == LB_ERR ? -1 : lSel),
												(LPARAM)(LPTSTR)szFind);
					if (lSelNew != (long)LB_ERR && lSelOld != lSelNew)
						FORWARD_WM_COMMAND(hWnd, GetWindowID(hWndCtrl), hWndCtrl,
												 LBN_SELCHANGE, SendMessage);
					}

#ifdef SS_V40
				if (lSelNew != LB_ERR && CellType.Spec.ComboBox.AutoSearch == SS_COMBOBOX_AUTOSEARCH_MULTIPLECHAR)
					lstrcpy(lpCombo->szAutoSearchText, szFind);
#endif // SS_V40

				if (lSel != lSelNew)
					{
					InvalidateRect(hWnd, NULL, FALSE);
					UpdateWindow(hWnd);
					}
				}
			}

		SSCB_Unlock(hWnd);
      }
	   return (0);

   case WM_GETFONT:
      Cell = SS_LockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);
      Font = SS_RetrieveFont(lpSS, &FontTemp, Cell, lpSS->Col.EditAt, lpSS->Row.EditAt);
      SS_UnlockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);
      return ((LRESULT)Font->hFont);

   case WM_SETFOCUS:
      SSCB_ClearAutoSearchText(hWnd);

//#ifdef SPREAD_JPN
      //- JPNFIX0002 - (Masanori Iwasa)
      hWndCombo = hWnd;
//#endif

      hWndCtrl = SSCB_GetListhWnd(hWnd);
      SetProp(hWndCtrl, _T("HWNDSPREAD"), GetParent(hWnd));

      SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt,
                          lpSS->Row.EditAt);

      if (SS_CB_ISDROPDOWN(CellType.Style))
         {
         GetClientRect(hWnd, &Rect);

         hWndCtrl = SSCB_GetEdithWnd(hWnd);

         Cell = SS_LockCellItem(lpSS, lpSS->Col.EditAt,
                                lpSS->Row.EditAt);
         Font = SS_RetrieveFont(lpSS, &FontTemp, Cell,
                                lpSS->Col.EditAt, lpSS->Row.EditAt);
         SS_UnlockCellItem(lpSS, lpSS->Col.EditAt,
                           lpSS->Row.EditAt);

         SendMessage(hWndCtrl, WM_SETFONT, (WPARAM)Font->hFont, 0L);

         dLen = (short)DefWindowProc(hWnd, WM_GETTEXTLENGTH, 0, 0L);

         if (hText = GlobalAlloc(GHND, (dLen + 1) * sizeof(TCHAR)))
            {
            lpText = (LPTSTR)GlobalLock(hText);

            DefWindowProc(hWnd, WM_GETTEXT, dLen + 1, (LPARAM)lpText);
            SendMessage(hWndCtrl, WM_SETTEXT, 0, (LPARAM)lpText);

            GlobalUnlock(hText);
            GlobalFree(hText);
            }

			/* RFW - 8/9/03
         MoveWindow(hWndCtrl, Rect.left + 1, Rect.top + 1, Rect.right -
                    lpSS->lpBook->dComboButtonBitmapWidth - 2, Rect.bottom - 1,
                    TRUE);
			*/
         MoveWindow(hWndCtrl, Rect.left + 2, Rect.top + 1, Rect.right -
                    lpSS->lpBook->dComboButtonBitmapWidth - 3, Rect.bottom - 1,
                    TRUE);

         /*
         ShowWindow(hWndCtrl, SW_SHOWNOACTIVATE);
         */
         ShowWindow(hWndCtrl, SW_SHOWNORMAL);
         UpdateWindow(hWndCtrl);
         SetFocus(hWndCtrl);

         if (lpSS->lpBook->fEditModeReplace)
            {
            Edit_SetSel(hWndCtrl, 0, dLen);
#ifdef WIN32
            Edit_ScrollCaret(hWndCtrl);
#endif
            }
         else
            {
            Edit_SetSel(hWndCtrl, dLen, dLen);
#ifdef WIN32
            Edit_ScrollCaret(hWndCtrl);
#endif
            }
         }

      else
         {
         SS_ComboSubclassDesktop(GetParent(hWnd));

#ifdef SPREAD_JPN
         //- JPNFIX0003 - (Masanori Iwasa)

         SendMessage(GetParent(hWnd), CB_SHOWDROPDOWN, FALSE, 0l);
#endif
         hWndCtrl = SSCB_GetEdithWnd(hWnd);
         ShowWindow(hWndCtrl, SW_HIDE);
         }

      break;

   case WM_KILLFOCUS:
      SSCB_ClearAutoSearchText(hWnd);
      if (!lpSS->lpBook->EditModeTurningOn)
         {
         SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt,
                             lpSS->Row.EditAt);
         hWndCtrl = SSCB_GetListhWnd(hWnd);
         if (GetFocus() != hWndCtrl)
            {
            SendMessage(hWnd, CB_SHOWDROPDOWN, FALSE, 0L);
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
            }

         if (GetCapture() == hWnd)
            ReleaseCapture();

         if (!SS_CB_ISDROPDOWN(CellType.Style))
            SS_ComboUnSubclassDesktop();
         }

      break;

#if 0
	// RFW - 8/8/05 - 16327
   case WM_SETTEXT:
      SS_RetrieveCellType(lpSS, &CellType, NULL,
                          lpSS->Col.EditAt, lpSS->Row.EditAt);

      if (SS_CB_ISDROPDOWN(CellType.Style))
         {
         hWndEdit = SSCB_GetEdithWnd(hWnd);
         return (SendMessage(hWndEdit, Msg, wParam, lParam));
         }

      break;
#endif

   case WM_GETTEXT:
   case WM_GETTEXTLENGTH:
      SS_RetrieveCellType(lpSS, &CellType, NULL,
                          lpSS->Col.EditAt, lpSS->Row.EditAt);

      if (SS_CB_ISDROPDOWN(CellType.Style))
         {
         hWndEdit = SSCB_GetEdithWnd(hWnd);
         return (SendMessage(hWndEdit, Msg, wParam, lParam));
         }

      break;

   case WM_DESTROY:
      lpCombo = SSCB_Lock(hWnd);
      if( lpCombo )
      {
         SS_ComboUnSubclassDesktop();
		   if (lpCombo->hWndList)
			   {
			   RemoveProp(lpCombo->hWndList, _T("HWNDSPREAD"));
			   RemoveProp(lpCombo->hWndList, _T("HWNDCOMBO"));
			   DestroyWindow(lpCombo->hWndList);
			   }

		   if (lpCombo->hWndEdit)
			   DestroyWindow(lpCombo->hWndEdit);

         SSCB_Unlock(hWnd);

#if defined(_WIN64) || defined(_IA64)
         if (hCombo = (HGLOBAL)GetWindowLongPtr(hWnd, 0))
#else
         if (hCombo = (HGLOBAL)GetWindowLong(hWnd, 0))
#endif
            GlobalFree(hCombo);
      }
      break;

	// RFW - 5/18/07
	case WM_MOUSEWHEEL:
      if (SSCB_GetBtnDown(hWnd))
			SendMessage(SSCB_GetListhWnd(hWnd), Msg, wParam, lParam);

		break;


   case WM_MOUSEACTIVATE:
      if (GetParent(GetFocus()) != hWnd)
         SetFocus(hWnd);

      return (0);

   case WM_LBUTTONDOWN:
   case WM_LBUTTONDBLCLK:
      SSCB_ClearAutoSearchText(hWnd);
      GetClientRect(hWnd, &Rect);

      x = LOWORD(lParam);
      y = HIWORD(lParam);

      SS_RetrieveCellType(lpSS, &CellType, NULL,
                          lpSS->Col.EditAt, lpSS->Row.EditAt);

      if (!SS_CB_ISDROPDOWN(CellType.Style) ||
          (x >= Rect.right - lpSS->lpBook->dComboButtonBitmapWidth &&
          x < Rect.right && y > 0 && y < Rect.bottom))
         {
         if (!SSCB_GetBtnDown(hWnd))
            {
            lpSS->lpBook->EditModeTurningOn = TRUE;
            SendMessage(hWnd, CB_SHOWDROPDOWN, TRUE, 0L);
            lpSS->lpBook->EditModeTurningOn = FALSE;
            }
         else
            {
            SendMessage(hWnd, CB_SHOWDROPDOWN, FALSE, 0L);
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
            }
         }

      break;

   case WM_SHOWWINDOW:
      if (!wParam)
         SendMessage(hWnd, CB_SHOWDROPDOWN, FALSE, 0L);

      break;

#ifdef WIN32
   case WM_CTLCOLOREDIT:
   case WM_CTLCOLORLISTBOX:
#else
   case WM_CTLCOLOR:
#endif
      SS_GetColorTblItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt,
                         &BackColorTblItem, &ForeColorTblItem);

      SetBkColor((HDC)wParam, SS_TranslateColor(BackColorTblItem.Color));
      SetTextColor((HDC)wParam, SS_TranslateColor(ForeColorTblItem.Color));

      return ((LRESULT)BackColorTblItem.hBrush);

   case CB_LIMITTEXT:
      return (SendMessage(SSCB_GetEdithWnd(hWnd), EM_LIMITTEXT, wParam, lParam));

#ifdef SS_WIN32S
   case CB_RESETCONTENT_WIN32S:
   case CB_ADDSTRING_WIN32S:
   case CB_GETCURSEL_WIN32S:
   case CB_SETCURSEL_WIN32S:
#endif
   case CB_RESETCONTENT:
   case CB_ADDSTRING:
   case CB_GETCURSEL:
   case CB_SETCURSEL:
      hWndCtrl = SSCB_GetListhWnd(hWnd);
      lpCombo = SSCB_Lock(hWnd);

      if (Msg == CB_RESETCONTENT || Msg == CB_RESETCONTENT_WIN32S)
         {
         lpCombo->lHorzExtent = 0;
         Msg = LB_RESETCONTENT;
         }

      else if (Msg == CB_ADDSTRING || Msg == CB_ADDSTRING_WIN32S)
         {
         if (lParam)
            {
            HDC   hDC = GetDC(hWndCtrl);
				// RFW - 8/4/04 - 14999
            //HFONT hFontOld = (HFONT)SelectObject(hDC,
            //                 (HFONT)SendMessage(hWndCtrl, WM_GETFONT, 0, 0));
            HFONT hFontOld;
            long  lExtent;
            SIZE  Size;

				Cell = SS_LockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);
				Font = SS_RetrieveFont(lpSS, &FontTemp, Cell, lpSS->Col.EditAt, lpSS->Row.EditAt);
				SS_UnlockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);
				hFontOld = (HFONT)SelectObject(hDC, Font->hFont);

            GetTextExtentPoint(hDC, (LPTSTR)lParam, lstrlen((LPTSTR)lParam), &Size);
            lExtent = Size.cx + 4;
            SelectObject(hDC, hFontOld);
            ReleaseDC(hWndCtrl, hDC);

            lpCombo->lHorzExtent = max(lpCombo->lHorzExtent, lExtent);
            }

         Msg = LB_ADDSTRING;
         }

      else if (Msg == CB_GETCURSEL || Msg == CB_GETCURSEL_WIN32S)
         Msg = LB_GETCURSEL;

      else if (Msg == CB_SETCURSEL || Msg == CB_SETCURSEL_WIN32S)
         Msg = LB_SETCURSEL;

      SSCB_Unlock(hWnd);

		// RFW - 8/4/06 - 19129
		if (!IsWindow(hWndCtrl))
			return (-1);

      return (SendMessage(hWndCtrl, Msg, wParam, lParam));

   case CB_GETDROPPEDSTATE:
      return (SSCB_GetBtnDown(hWnd));

#ifdef SS_WIN32S
   case CB_SHOWDROPDOWN_WIN32S:
#endif
   case CB_SHOWDROPDOWN:

      if (wParam == FALSE)
         {
         if (SSCB_GetBtnDown(hWnd))
            {
            HWND hWndSS = GetParent(hWnd);

            SSCB_SetBtnDown(hWnd, FALSE);
// fix for bug 9416 -scl
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);

            if (hWndCtrl = SSCB_GetListhWnd(hWnd))
               {
					/* RFW - 3/3/04 - 13801
               ShowWindow(hWndCtrl, SW_HIDE);
					*/
					/* RFW - 5/27/04 - 14308
               SetWindowPos(hWndCtrl, HWND_NOTOPMOST, 0, 0, 0, 0,
                            SWP_NOACTIVATE | SWP_NOMOVE |
                            SWP_NOSIZE | SWP_HIDEWINDOW);
					*/
               SetWindowPos(hWndCtrl, HWND_NOTOPMOST, 0, 0, 0, 0,
                            SWP_NOACTIVATE | SWP_NOMOVE |
                            SWP_NOSIZE | SWP_HIDEWINDOW |
                            SWP_NOOWNERZORDER);

               dCurSel = (long)SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
               SS_SendMsgComboCloseUp(lpSS,
                  (short)(dCurSel != SSCB_GetCurSel(hWnd) ? dCurSel : -1),
                  lpSS->Col.EditAt, lpSS->Row.EditAt);
               }
            }

         return 0;
         }

      else if (!SSCB_GetBtnDown(hWnd))
         {
         HWND        hWndSS = GetParent(hWnd);
         SS_CELLTYPE CellTypeOld;

         if (GetFocus() != hWnd && GetFocus() != SSCB_GetEdithWnd(hWnd))
            return (0);

         SS_RetrieveCellType(lpSS, &CellTypeOld, NULL,
                             lpSS->Col.EditAt, lpSS->Row.EditAt);

			lpSS->fComboListChanged = FALSE;
         SS_SendMsgCoords(lpSS, SSM_COMBODROPDOWN,
                          GetDlgCtrlID(hWndSS),
                          lpSS->Col.EditAt, lpSS->Row.EditAt);

         SS_RetrieveCellType(lpSS, &CellType, NULL,
                             lpSS->Col.EditAt, lpSS->Row.EditAt);

/* RFW - 8/8/03 - 12335
         if (CellTypeOld.Spec.ComboBox.hItems !=
             CellType.Spec.ComboBox.hItems)
*/
			if (lpSS->fComboListChanged)
            {
            SendMessage(hWnd, CB_RESETCONTENT, 0, 0L);

            if (CellType.Spec.ComboBox.hItems)
               {
               LPTSTR lpItems = (LPTSTR)tbGlobalLock(CellType.Spec.ComboBox.hItems);
               long   i;

               for (i = 0; i < CellType.Spec.ComboBox.dItemCnt; i++)
                  {
                  SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)lpItems);
                  lpItems += lstrlen(lpItems) + 1;
                  }

               tbGlobalUnlock(CellType.Spec.ComboBox.hItems);
               }
            }

         hWndCtrl = SSCB_GetListhWnd(hWnd);
         lpCombo = SSCB_Lock(hWnd);

         dCnt = (long)SendMessage(hWndCtrl, LB_GETCOUNT, 0, 0L);

         // Check if using ListPro's combo then if Virtual
         // Mode, do a Refresh.

         dItemsShown = CellType.Spec.ComboBox.dMaxRows > 0 ?
                       CellType.Spec.ComboBox.dMaxRows : 6 ;

         if (lpCombo->hWndList != lpCombo->hWndListActive &&
             lpCombo->hWndListProCombo &&
             dCnt < dItemsShown && SendMessage(hWndCtrl, LBM_GETPROPERTY,
             LBPROP_VirtualMode, 0))
            {
            LRESULT lPageSize = SendMessage(hWndCtrl, LBM_GETPROPERTY,
                                         LBPROP_VirtualPageSize, 0);

            SendMessage(hWndCtrl, LBM_SETPROPERTY, LBPROP_VirtualPageSize, dItemsShown);
            SendMessage(hWndCtrl, LBM_SETPROPERTY, LBPROP_Action, LB_ACTION_VIRTUALREFRESH);
            SendMessage(hWndCtrl, LBM_SETPROPERTY, LBPROP_VirtualPageSize, lPageSize);
            dCnt = (long)SendMessage(hWndCtrl, LB_GETCOUNT, 0, 0L);
            }

         if (dCnt > 0)
            {
            short dScreenHeight;
            short dScreenWidth;
				short dScreenOrgX;
				short dScreenOrgY;
            short dWidth;
            long  lHorzExtent = 0;
            RECT  RectCombo[2];
				RECT  rcMonitor;

            // HWND  hWndSS = GetParent(hWnd);

            SSCB_SetBtnDown(hWnd, TRUE);

            /* RFW - 7/16/99 - Moved this to above the code that checks
               if there are any items in the list.  That way the user
               can populate the list if it is empty.
            SS_SendMsgCoords(hWndSS, SSM_COMBODROPDOWN,
                             GetDlgCtrlID(hWndSS),
                             lpSS->Col.EditAt - lpSS->Col.HeaderCnt + 1,
                             lpSS->Row.EditAt - lpSS->Row.HeaderCnt + 1);
            */

            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);

            hWndCtrl = SSCB_GetListhWnd(hWnd);

            EnableWindow(hWndCtrl, TRUE); // BJO 5Apr96 SEL3639

            Cell = SS_LockCellItem(lpSS, lpSS->Col.EditAt,
                                   lpSS->Row.EditAt);
            Font = SS_RetrieveFont(lpSS, &FontTemp, Cell, lpSS->Col.EditAt,
                                   lpSS->Row.EditAt);
            SS_UnlockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);

            SendMessage(hWndCtrl, WM_SETFONT, (WPARAM)Font->hFont, 0L);

            dItemCnt = (long)SendMessage(hWndCtrl, LB_GETCOUNT, 0, 0L);

            GetWindowRect(hWnd, &Rect);
            CopyRect(&RectCombo[1], &Rect);
            Rect.right++;

            hDC = fpGetDC(hWndCtrl);
            hFontOld = SelectObject(hDC, Font->hFont);
            GetTextMetrics(hDC, &fm);

            SS_RetrieveCellType(lpSS, &CellType, NULL,
                                lpSS->Col.EditAt, lpSS->Row.EditAt);

            dItemsShown = (short)min(dItemCnt, CellType.Spec.ComboBox.dMaxRows > 0 ?
                                     CellType.Spec.ComboBox.dMaxRows : 6);
            dHeight = ((short)fm.tmHeight * dItemsShown) + 2;
            SelectObject(hDC, hFontOld);

				/*
            dScreenHeight = GetSystemMetrics(79); // SM_CYVIRTUALSCREEN
            dScreenWidth = GetSystemMetrics(78);  // SM_CXVIRTUALSCREEN
            // RFW - 3/10/04 - 8048
            //dScreenHeight = GetSystemMetrics(SM_CYSCREEN);
            //dScreenWidth = GetSystemMetrics(SM_CXSCREEN);
				// SCL - 3/23/04 - 13932
				if (dScreenHeight == 0)
					dScreenHeight = GetSystemMetrics(SM_CYSCREEN);
				if (dScreenHeight == 0)
					dScreenHeight = GetDeviceCaps(hDC, VERTRES);
				if (dScreenWidth == 0)
					dScreenWidth = GetSystemMetrics(SM_CXSCREEN);
				if (dScreenWidth == 0)
					dScreenWidth = GetDeviceCaps(hDC, HORZRES);
				*/

				SS_GetMonitorRect(&RectCombo[1], &rcMonitor);
				dScreenOrgX = (short)rcMonitor.left;
				dScreenOrgY = (short)rcMonitor.top;
				dScreenWidth = (short)(rcMonitor.right - rcMonitor.left);
				dScreenHeight = (short)(rcMonitor.bottom - rcMonitor.top);

            ReleaseDC(hWndCtrl, hDC);

            if (lpCombo->hWndList == lpCombo->hWndListActive)
               lHorzExtent = lpCombo->lHorzExtent;
            else if (lpCombo->hWndListProCombo)
					/* RFW - 45/1/05 - 16170
               lHorzExtent = (long)SendMessage(lpCombo->hWndListActive, 0x0615, 0, 0); // LBMx_GETACTUALHEXTENT
					*/
               lHorzExtent = (long)SendMessage(lpCombo->hWndListActive, 0x0615, 0, 0) + 4; // LBMx_GETACTUALHEXTENT

            if (CellType.Spec.ComboBox.dComboWidth != SS_COMBOWIDTH_CELLWIDTH)
               {
               if (CellType.Spec.ComboBox.dComboWidth == SS_COMBOWIDTH_AUTORIGHT ||
                   CellType.Spec.ComboBox.dComboWidth == SS_COMBOWIDTH_AUTOLEFT)
                  {
                  dWidth = (short)min((long)dScreenWidth, lHorzExtent + 2);
                  if (dItemsShown < dItemCnt)
                     dWidth += GetSystemMetrics(SM_CXVSCROLL);
                  }
               else
                  dWidth = abs(CellType.Spec.ComboBox.dComboWidth);

               if (CellType.Spec.ComboBox.dComboWidth > 0)
                  Rect.left = max(Rect.left, Rect.right - dWidth);
               else
						{
						// RFW - 9/27/07 - 21160
                  Rect.left = max(dScreenOrgX, Rect.right - dWidth);
						}

	            Rect.right = min(Rect.left + dWidth, max(Rect.right, dScreenOrgX + dScreenWidth));
               }

   #if 0
   #ifdef SS_V30
            if ((Rect.right - Rect.left) - 2 -
                (dItemsShown < dItemCnt ? GetSystemMetrics(SM_CXVSCROLL) - 1 : 0) <
                lHorzExtent)
               {
               dHeight += GetSystemMetrics(SM_CYHSCROLL) - 1;
               ShowScrollBar(hWndCtrl, SB_HORZ, TRUE);
               }
            else
               ShowScrollBar(hWndCtrl, SB_HORZ, FALSE);
   #endif
   #endif

            dHeight = min(dHeight, dScreenHeight); 

            if (Rect.bottom + dHeight >= dScreenOrgY + dScreenHeight)
					{
					// RFW - 9/27/07 - 21160
               Rect.top = max(Rect.top - dHeight, dScreenOrgY);
					}
            else
               Rect.top = Rect.bottom;

            Rect.bottom = min(Rect.top + dHeight, dScreenOrgY + dScreenHeight);

            lpCombo = SSCB_Lock(hWnd);

            dCurSel = (long)SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
              
   #ifndef SPREAD_JPN
            //- JPNFIX0021 - (Masanori Iwasa)
   #ifndef SS_V30
            if (dCurSel == LB_ERR)
   #endif
               {
   #endif // SPREAD_JPN
               dLen = (short)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0L);
               if (hText = GlobalAlloc(GHND, (dLen+1) * sizeof(TCHAR)))
                  {
						long lCurSelTemp;

                  lpText = (LPTSTR)GlobalLock(hText);
                  SendMessage(hWnd, WM_GETTEXT, dLen+1, (LPARAM)lpText);

                  if (lpCombo->hWndList != hWndCtrl)
                     {
                     if (lpCombo->hWndListProCombo)
                        {
                        short nColEdit = (short)SendMessage(lpCombo->hWndListProCombo,
                                                            CBM_GETPROPERTY,
                                                            CBPROP_ColumnEdit, 0);
                        SendMessage(hWndCtrl, LBM_SETPROPERTY,
                                    LBPROP_ColumnSearch, (LPARAM)nColEdit);
                        }
                     }

                  lCurSelTemp = (long)SendMessage(hWndCtrl, LB_FINDSTRINGEXACT,
                                                  (WPARAM)(dCurSel == -1 ? -1 : dCurSel - 1),
                                                  (LPARAM)lpText);

						if (lCurSelTemp != dCurSel)
							dCurSel = (long)SendMessage(hWndCtrl, LB_FINDSTRINGEXACT,
                                                 (WPARAM)-1, (LPARAM)lpText);

                  GlobalUnlock(hText);
                  GlobalFree(hText);
                  }
   #ifndef SPREAD_JPN
               //- JPNFIX0021 - (Masanori Iwasa)
               }
   #endif
// 21093 -scl
// (moved down below SendMessage(hWnd, CB_SETCURSEL, (WPARAM)dCurSel, 0L))
//          if (dCurSel <= dItemsShown)
//             SendMessage(hWndCtrl, LB_SETTOPINDEX, 0, 0L);
// 			else if( dCurSel + dItemsShown > dCnt )
//				SendMessage(hWndCtrl, LB_SETTOPINDEX, (WPARAM)(dCnt - dItemsShown), 0L);
//			else
//				SendMessage(hWndCtrl, LB_SETTOPINDEX, (WPARAM)dCurSel, 0L);

            // Check to see if it is the default list that is being used.
            if (lpCombo->hWndList == hWndCtrl)
               {
               if (dItemsShown < dItemCnt)
                  ShowScrollBar(hWndCtrl, SB_VERT, TRUE);
               else
                  ShowScrollBar(hWndCtrl, SB_VERT, FALSE);

               MoveWindow(hWndCtrl, Rect.left, Rect.top, Rect.right - Rect.left,
                          Rect.bottom - Rect.top, TRUE);

               SetWindowPos(hWndCtrl, HWND_TOPMOST, 0, 0, 0, 0,
                            SWP_NOACTIVATE | SWP_NOMOVE |
                            SWP_NOSIZE | SWP_SHOWWINDOW);
               }
            else
               {
               RectCombo[0] = Rect;
               SendMessage(hWndCtrl, 0x0612,        // LBMx_SIZEDROPDOWN
                           (WPARAM)(CellType.Spec.ComboBox.dMaxRows > 0 ?
                           CellType.Spec.ComboBox.dMaxRows : 6),
                           (LPARAM)(LPVOID)RectCombo);
               }

            SSCB_Unlock(hWnd);

//            SendMessage(hWnd, LB_SETCURSEL, (WPARAM)dCurSel, 0L);
				lpCombo->fIgnoreSelChange = TRUE; // RFW - 5/18/07 - 20390
            SendMessage(hWnd, CB_SETCURSEL, (WPARAM)dCurSel, 0L);
            SSCB_SetCurSel(hWnd, dCurSel);
				lpCombo->fIgnoreSelChange = FALSE; // RFW - 5/18/07 - 20390
// 21093 -scl
// (moved from above)
            if (dCurSel <= dItemsShown)
               SendMessage(hWndCtrl, LB_SETTOPINDEX, 0, 0L);
			else if( dCurSel + dItemsShown > dCnt )
				SendMessage(hWndCtrl, LB_SETTOPINDEX, (WPARAM)(dCnt - dItemsShown), 0L);
			else
				SendMessage(hWndCtrl, LB_SETTOPINDEX, (WPARAM)dCurSel, 0L);

            ReleaseCapture();
            }

         SSCB_Unlock(hWnd);
         }

      break;

   case CB_GETEDITSEL:
      return (SendMessage(SSCB_GetEdithWnd(hWnd), EM_GETSEL, wParam, lParam));

   case CB_SETEDITSEL:
      #if defined(WIN32)
      return (SendMessage(SSCB_GetEdithWnd(hWnd), EM_SETSEL, (WPARAM)(short)LOWORD(lParam), (LPARAM)(short)HIWORD(lParam)));
      #else
      return (SendMessage(SSCB_GetEdithWnd(hWnd), EM_SETSEL, wParam, lParam));
      #endif

   case SS_CB_REPLACESEL:
      return (SendMessage(SSCB_GetEdithWnd(hWnd), EM_REPLACESEL, wParam, lParam));

	/*
   case CBM_SETIGNORESELCHANGE:
      {
      HWND hWndSS = GetParent(hWnd);
      lpSS->fCBIgnoreSelChange = (BOOL)wParam;
      }
		break;
	*/

   case WM_COMMAND:
      {
#ifdef WIN32
      WORD wNotifyCode = HIWORD(wParam);
      WORD wID         = LOWORD(wParam);
      HWND hWndCtl     = (HWND)lParam;
#else  //not WIN32  
      WORD wNotifyCode = HIWORD(lParam);
      WORD wID         = wParam;
      HWND hWndCtl     = LOWORD(lParam);
#endif
      HWND hWndList = SSCB_GetListhWnd(hWnd);
      HWND hWndEdit = SSCB_GetEdithWnd(hWnd);

      if (hWndCtl == hWndEdit) // Edit
         {
         lpCombo = SSCB_Lock(hWnd);

         if (wNotifyCode == EN_CHANGE && !lpCombo->fInternalENChange)
            {
            WPARAM wParam;
            LPARAM lParam;
            short dLen = (short)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0L);
            if (hText = GlobalAlloc(GHND, (dLen+1) * sizeof(TCHAR)))
               {
               short nColEdit;

               lpText = (LPTSTR)GlobalLock(hText);
               SendMessage(hWnd, WM_GETTEXT, dLen+1, (LPARAM)lpText);

					if (lpCombo->hWndList != hWndList && lpCombo->hWndListProCombo) // ListPro
						{
                  nColEdit = (short)SendMessage(lpCombo->hWndListProCombo,
                                                CBM_GETPROPERTY,
                                                CBPROP_ColumnEdit, 0);
                  SendMessage(hWndList, LBM_SETPROPERTY,
                              LBPROP_ColumnSearch, (LPARAM)nColEdit);

#if defined(SS_V40) && defined(_UNICODE)
                  // translate string from unicode to ANSI
                  {
                    LPSTR buf = malloc((dLen+1)*sizeof(char));
                    if( buf )
                    {
                      memset(buf, 0, (dLen+1)*sizeof(char));
                      WideCharToMultiByte(CP_ACP, 0, lpText, dLen, buf, dLen, NULL, NULL);
                      strcpy((char*)lpText, buf);
                      free(buf);
                    }
                  }
#endif
                  dCurSel = (long)SendMessage(hWndList, LB_FINDSTRING,
                                              (WPARAM)-1, (LPARAM)lpText);

                  lpCombo->fIgnoreSelChange = TRUE;
                  SendMessage(hWndList, LB_SETCURSEL, (WPARAM)-1, 0L);
//                  lpCombo->lSelIndex = -1;
                  if (dCurSel >= 0)
                     {
//                     lpList->lRectDotPos = dCurSel;
                     SendMessage(hWndList, LBM_SETPROPERTY,
                                 LBPROP_SearchIndex, (LPARAM)dCurSel);
                     SendMessage(hWndList, LB_SETTOPINDEX, dCurSel, 0L);
//                     LB_SetTopIndex(lpList, min(lSearchIndex,
//                                    LB_RowGetLastPageAllVis(lpList)));
                     }

                  else
                     SendMessage(hWndList, LBM_SETPROPERTY,
                                 LBPROP_SearchIndex, (LPARAM)-1);

                  lpCombo->fIgnoreSelChange = FALSE;
                  }
#ifdef SS_V40
					else // Internal drop down
						{
						LRESULT lSelOld, lSelNew;

						lSelOld = (long)SendMessage(hWndList, LB_GETCURSEL, 0, 0L);
						/* RFW - 12/7/03 - 12868
						lSelNew = SendMessage(hWndList, LB_SELECTSTRING, (WPARAM)(lSelOld == LB_ERR ? -1 : lSelOld),
													(LPARAM)(LPTSTR)lpText);
						*/
						lSelNew = SendMessage(hWndList, LB_SELECTSTRING, (WPARAM)(lSelOld == LB_ERR ? -1 : lSelOld - 1),
													(LPARAM)(LPTSTR)lpText);
//						if (lSelNew != LB_ERR && lSelOld != lSelNew)
//							FORWARD_WM_COMMAND(hWnd, GetWindowID(hWndList), hWndList,
//													 LBN_SELCHANGE, SendMessage);
						}
#endif // SS_V40

               GlobalUnlock(hText);
               GlobalFree(hText);
					}

            #ifdef WIN32
              wParam = MAKEWPARAM(GetWindowID(hWnd), CBN_EDITCHANGE);
              lParam = (LPARAM)hWnd;
            #else
              wParam = (WPARAM)GetWindowID(hWnd);
              lParam = MAKELPARAM(hWnd, CBN_EDITCHANGE);
            #endif

            return (SendMessage(GetParent(hWnd), WM_COMMAND, wParam, lParam));
            }

         SSCB_Unlock(hWnd);
         }
      else // List
         {
         lpCombo = SSCB_Lock(hWnd);

			/* RFW - 2/28/04 - 15810
			// fix for bug 4913 -scl
         if (wNotifyCode == LBN_SELCHANGE && !lpCombo->fIgnoreSelChange
			 && lpCombo->hWndList == lpCombo->hWndListActive ||
                      lpSS->lpBook->fComboKeyDown  ||
							 // At some point in time this code replaced the above line.  I'm not sure why.  I commented
							 // it out because it causes bug 9489 RFW 1/14/02
							 // (lpSS->fComboKeyDown && SendMessage(hWnd, CB_GETDROPPEDSTATE, 0, 0L)) ||
                      (GetAsyncKeyState(VK_LBUTTON) & 0x8000))
			*/

			/* RFW - 2/10/06 - 18177
         if (wNotifyCode == LBN_SELCHANGE && !lpCombo->fIgnoreSelChange &&
             (lpCombo->hWndList == lpCombo->hWndListActive || lpSS->lpBook->fComboKeyDown  ||
              (GetAsyncKeyState(VK_LBUTTON) & 0x8000)))
			*/
			// RFW - 5/18/07
         if (wNotifyCode == LBN_SELCHANGE && !lpCombo->fIgnoreSelChange &&
             (lpCombo->hWndList == lpCombo->hWndListActive || lpSS->lpBook->fComboKeyDown  ||
              (GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) & 0x8000)))
            {
            SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)_TEXT(""));
            hWndCtrl = SSCB_GetListhWnd(hWnd);

            if ((dSel = (long)SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L)) != LB_ERR)
               {
               if (hText = SSCB_GetTextFromList(hWnd, dSel))
                  {
                  HWND hWndSS = GetParent(hWnd);

                  lpText = (LPTSTR)GlobalLock(hText);

                  SS_RetrieveCellType(lpSS, &CellType, NULL,
                                      lpSS->Col.EditAt, lpSS->Row.EditAt);

                  hWndEdit = SSCB_GetEdithWnd(hWnd);

                  lpCombo->fInternalENChange = TRUE;
                  if (SS_CB_ISDROPDOWN(CellType.Style))
                     SetWindowText(hWndEdit, lpText);
                  else
                     FORWARD_WM_COMMAND(hWnd, GetWindowID(hWndEdit), hWndEdit,
                                        EN_CHANGE, SendMessage);
                  lpCombo->fInternalENChange = FALSE;

                  // If the ListPro combo box is being used and this
                  // message is sent after the mouse has been released
                  // then do not send SSM_COMBOSELCHANGE
/*
                  lpCombo = SSCB_Lock(hWnd);
                  if (lpCombo->hWndList == lpCombo->hWndListActive ||
                      lpSS->fComboKeyDown ||
                      (GetAsyncKeyState(VK_LBUTTON) & 0x8000))
*/
                     SS_SendMsgCoords(lpSS, SSM_COMBOSELCHANGE,
                                      GetDlgCtrlID(hWndSS),
                                      lpSS->Col.EditAt, lpSS->Row.EditAt);
//                  SSCB_Unlock(hWnd);

                  GlobalUnlock(hText);
                  GlobalFree(hText);
                  }
               }

            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            }

         SSCB_Unlock(hWnd);
         }
      }
      return (0);

   case WM_ERASEBKGND:
      return (TRUE);

 #ifdef SS_V80
   case WM_MOUSELEAVE:
	   {
         lpCombo = SSCB_Lock(hWnd);
	     lpCombo->MouseOver = FALSE;
         SSCB_Unlock(hWnd);
	     InvalidateRect(hWnd, NULL, FALSE);
	   }
	   break;
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

      SetRect(&Rect, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top);
      SS_ComboPaint(hWnd, lpSS, hDCMemory, &Rect);

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

      EndPaint(hWnd, &Paint);
      return (0);

   case SS_CBM_SETDROPDOWNHWND:
      lpCombo = SSCB_Lock(hWnd);

      lpCombo->hWndListProCombo = 0;
      lpCombo->hWndListActive = lpCombo->hWndList;

      if (lParam && IsWindow((HWND)lParam))
         {
         TCHAR szBuffer[30];

         GetClassName((HWND)lParam, szBuffer, sizeof(szBuffer));
         _ftcslwr(szBuffer);

         if (_ftcsstr(szBuffer, _T("fp")) && _ftcsstr(szBuffer, _T("combo")))
            {
            HWND hWndListActive = (HWND)SendMessage((HWND)lParam, 0x0700, 0, 0);

            if (hWndListActive && IsWindow(hWndListActive))
               {
               lpCombo->hWndListProCombo = (HWND)lParam;
               lpCombo->hWndListActive = hWndListActive;
               SendMessage(lpCombo->hWndListActive, 0x0611, 0, (LPARAM)hWnd); // LBMx_SETOWNERHWND
               }
            }

#if 0
         else if (_ftcsstr(szBuffer, _T("fplist")))
            {
            long lStyle;

            lpCombo->hWndListActive = (HWND)lParam;
            lStyle = GetWindowLong(lpCombo->hWndListActive, GWL_STYLE);
            SendMessage(lpCombo->hWndListActive, 0x0611, 0, (LPARAM)hWnd); // LBMx_SETOWNERHWND

            SetWindowPos(lpCombo->hWndListActive, 0, -2000, 0, 0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            SetParent(lpCombo->hWndListActive, 0);
            SetWindowLong(lpCombo->hWndListActive, GWL_STYLE, (lStyle & ~WS_CHILD) |
                          WS_POPUP);
            SetWindowPos(lpCombo->hWndListActive, 0, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_NOZORDER |
                         SWP_NOACTIVATE);
            }
#endif
         }

      SSCB_Unlock(hWnd);
      return (0);

   }

// fix for bug #9067 -scl
//if( Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST )
// ^- I found that this really messes with EditModePermanent, 
// so I restricted it to WM_MOUSEMOVE
if( Msg == WM_MOUSEMOVE )
{
   POINT pt = {LOWORD(lParam), HIWORD(lParam)};
   HWND hwndparent = GetParent(hWnd);

 #ifdef SS_V80
   GetClientRect(hWnd, &Rect);
   SS_RetrieveCellType(lpSS, &CellType, NULL,
                          lpSS->Col.EditAt, lpSS->Row.EditAt);
   lpCombo = SSCB_Lock(hWnd);
   if ((pt.x >= Rect.right - lpSS->lpBook->dComboButtonBitmapWidth &&
       pt.x < Rect.right && pt.y > 0 && pt.y < Rect.bottom))
        {
	   TRACKMOUSEEVENT tme;
	   if (lpCombo->MouseOver == FALSE)
	   {
		 lpCombo->MouseOver = TRUE;
		 tme.cbSize = sizeof(TRACKMOUSEEVENT);
		 tme.dwFlags = TME_LEAVE;
		 tme.hwndTrack = hWnd;
		 tme.dwHoverTime = HOVER_DEFAULT;
		 _TrackMouseEvent(&tme);

  		InvalidateRect(hWnd, NULL, FALSE);
	   }
   }
   else
	   lpCombo->MouseOver = FALSE;
   SSCB_Unlock(hWnd);
	   
#endif
    MapWindowPoints(hWnd, hwndparent, &pt, 1);
    PostMessage(hwndparent, Msg, wParam, MAKELONG(pt.x, pt.y));
}
return (DefWindowProc(hWnd, Msg, wParam, lParam));
}


void SS_ComboPaint(HWND hWndCombo, LPSPREADSHEET lpSS, HDC hDC, LPRECT lpRect)
{
SS_COLORTBLITEM BackColorTblItem;
SS_CELLTYPE     CellType;
GLOBALHANDLE    hText;
HBRUSH          hBrush;
LPTSTR          lpText;
HFONT           hFontOld;
LPSS_FONT       Font;
LPSS_CELL       Cell;
SS_FONT         FontTemp;
short           dLen;

#ifdef SS_V80
BOOL fUseVisualStyles = (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1);
BOOL fMouseOver = FALSE;
LPSS_COMBO lpCombo = SSCB_Lock(hWndCombo);
fMouseOver = lpCombo->MouseOver;
SSCB_Unlock(hWndCombo);
#endif
SS_GetColorTblItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt,
                   &BackColorTblItem, NULL);

FillRect(hDC, lpRect, BackColorTblItem.hBrush);

#ifdef SS_V80
SS_DrawComboBtn(hWndCombo, lpSS, hDC, lpRect,
                lpSS->lpBook->dComboButtonBitmapWidth,
                (BOOL)SSCB_GetBtnDown(hWndCombo), fMouseOver, fUseVisualStyles );
#else
SS_DrawComboBtn(hWndCombo, lpSS, hDC, lpRect,
                lpSS->lpBook->dComboButtonBitmapWidth,
                (BOOL)SSCB_GetBtnDown(hWndCombo));
#endif

SS_RetrieveCellType(lpSS, &CellType, NULL,
                    lpSS->Col.EditAt, lpSS->Row.EditAt);

if (!SS_CB_ISDROPDOWN(CellType.Style))
   {
   dLen = (short)SendMessage(hWndCombo, WM_GETTEXTLENGTH, 0, 0L);

   if (hText = GlobalAlloc(GHND, (dLen + 1) * sizeof(TCHAR)))
      {
      lpText = (LPTSTR)GlobalLock(hText);

      SendMessage(hWndCombo, WM_GETTEXT, dLen + 1, (LPARAM)lpText);

      Cell = SS_LockCellItem(lpSS, lpSS->Col.EditAt,
                             lpSS->Row.EditAt);
      Font = SS_RetrieveFont(lpSS, &FontTemp, Cell, lpSS->Col.EditAt,
                             lpSS->Row.EditAt);
      SS_UnlockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);

      hFontOld = SelectObject(hDC, Font->hFont);

      SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
      SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));

      lpRect->left += lpSS->lpBook->dXMargin;
      lpRect->top += lpSS->lpBook->dYMargin;
      lpRect->right -= lpSS->lpBook->dComboButtonBitmapWidth;

      hBrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
      FillRect(hDC, lpRect, hBrush);
      DeleteObject(hBrush);

      DrawText(hDC, lpText, -1, lpRect, DT_LEFT | DT_NOPREFIX);

      SelectObject(hDC, hFontOld);

      GlobalUnlock(hText);
      GlobalFree(hText);
      }
   }
}


BOOL SS_RegisterEditComboBox(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (lpfnEditComboBoxProc)
	return (TRUE);

if (GetClassInfo(NULL, _T("Edit"), &wc))
   {
   lpfnEditComboBoxProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_EditComboBoxProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSEditComboBox;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterEditComboBox(HANDLE hInstance)
{
if (lpfnEditComboBoxProc)
	return (UnregisterClass(SSClassNames[dClassNameIndex].SSEditComboBox, hInstance));

return (TRUE);
}


WINENTRY SS_EditComboBoxProc(hWnd, Msg, wParam, lParam)

HWND          hWnd;
UINT          Msg;
WPARAM        wParam;
LPARAM        lParam;
{
switch (Msg)
   {
   case WM_SYSKEYDOWN:
      tbSSComboBoxWndFn(GetParent(hWnd), Msg, wParam, lParam);
      return (0);

   case WM_KEYDOWN:
      switch (wParam)
         {
         case VK_DELETE:
            SendMessage(GetParent(hWnd), CB_SETCURSEL, (WPARAM)-1, 0L);
            break;

         case VK_UP:
         case VK_DOWN:
            tbSSComboBoxWndFn(GetParent(hWnd), Msg, wParam, lParam);
				// RFW - 4/18/05 - 16021
		      SS_SendMessageToVB(GetParent(GetParent(hWnd)), Msg, wParam, lParam);
            return (0);

			// RFW - 6/25/04 - 14438
			case VK_F4:
				{
				HWND hWndCombo = GetParent(hWnd);

				if (SSCB_GetBtnDown(hWndCombo))
					SendMessage(hWndCombo, CB_SHOWDROPDOWN, FALSE, 0L);
				else
					SendMessage(hWndCombo, CB_SHOWDROPDOWN, TRUE, 0L);
				}
         }

      break;

#ifdef _UNICODE
	case WM_IME_CHAR: // RFW - 2/8/05 - 15320
		Msg = WM_CHAR;
#endif

   case WM_CHAR:
      if (wParam != VK_RETURN)
         SendMessage(GetParent(hWnd), CB_SETCURSEL, (WPARAM)-1, 0L);

		// RFW - 6/28/02 If Enter key is pressed while list is down, then put selected item in edit box.
		else
			{
	      LPSS_COMBO lpCombo = SSCB_Lock(GetParent(hWnd));
			HWND       hWndList = SSCB_GetListhWnd(GetParent(hWnd));

         if (lpCombo->hWndList == lpCombo->hWndListActive && // Not ListPro
			    SSCB_GetBtnDown(GetParent(hWnd)))
				{
				long lSel = (long)SendMessage(hWndList, LB_GETCURSEL, 0, 0L);

				if (lSel != -1)
					FORWARD_WM_COMMAND(GetParent(hWnd), GetWindowID(hWndList), hWndList,
											 LBN_SELCHANGE, SendMessage);
				}

	      SSCB_Unlock(GetParent(hWnd));
			}

      break;

   case WM_SETFOCUS:
//#ifdef SPREAD_JPN
      //- JPNFIX0002 - (Masanori Iwasa)
      hWndCombo = GetParent(hWnd);
//#endif
      if (SSCB_GetListhWnd(GetParent(hWnd)))
         SS_ComboSubclassDesktop(GetParent(GetParent(hWnd)));
      break;

   case WM_KILLFOCUS:
      SS_ComboUnSubclassDesktop();
//      MoveWindow(hWnd, 0, 0, 0, 0, FALSE);
//      ShowWindow(hWnd, SW_HIDE);
      if ((HWND)lParam != GetParent(hWnd))
         {
         HWND hWndCombo = GetParent(hWnd);

         if (SSCB_GetBtnDown(hWndCombo))
            {
            SendMessage(hWndCombo, CB_SHOWDROPDOWN, FALSE, 0L);
            InvalidateRect(hWndCombo, NULL, FALSE);
            }
         }

      break;
   }

// fix for bug #9067 -scl
//if( Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST )
// ^- I found that this really messes with EditModePermanent, 
// so I restricted it to WM_MOUSEMOVE
if( Msg == WM_MOUSEMOVE )
	{
   POINT pt = {LOWORD(lParam), HIWORD(lParam)};
   HWND hwndparent = GetParent(hWnd);
   MapWindowPoints(hWnd, hwndparent, &pt, 1);
   PostMessage(hwndparent, Msg, wParam, MAKELONG(pt.x, pt.y));
	}

{
HWND hWndSS = GetParent(GetParent(hWnd));
LPSPREADSHEET lpSS = SS_SheetLockActive(hWndSS);
LRESULT lRet;

lRet = SS_DefWindowProc(lpfnEditComboBoxProc, lpSS, hWndSS,
                         hWnd, Msg, wParam, lParam);

SS_SheetUnlockActive(hWndSS);
return (lRet);
}
}


void SS_ComboSubclassDesktop(HWND hWnd)
{
// Note: Subclassing of the desktop is only allowed in 16-bit windows.
#ifndef WIN32
if (!lpfnDeskTopProc)
   {
   HWND hWndDesktop;

   /******************************
   * Subclass the desktop window
   ******************************/

   hWndDesktop = GetDesktopWindow();

   lpfnSSDeskTopProc = MakeProcInstance((FARPROC)SS_DeskTopProc,
                                        hDynamicInst);
   lpfnDeskTopProc = (FARPROC)GetWindowLong(hWndDesktop, GWL_WNDPROC);

   SetWindowLong(hWndDesktop, GWL_WNDPROC, (long)lpfnSSDeskTopProc);
   SetProp(hWndDesktop, "HWNDSPREAD", hWnd);
   }
#endif

//#ifdef SPREAD_JPN
//- JPNFIX0002 - (Masanori Iwasa)
if (!lpfnWndHookProc)
   {
   lpfnWndHookProc = MakeProcInstance((FARPROC)HandleWnd, hDynamicInst);
#ifdef WIN32
   hWndHook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)lpfnWndHookProc, 
                        hDynamicInst, GetCurrentThreadId());
#else
   hWndHook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)lpfnWndHookProc, 
                        hDynamicInst, (HTASK)NULL);
#endif
   }
//#endif
}


void SS_ComboUnSubclassDesktop(void)
{
// Note: Subclassing of the desktop is only allowed in 16-bit windows.
#ifndef WIN32
if (lpfnDeskTopProc)
   {
   HWND hWndDesktop;

   hWndDesktop = GetDesktopWindow();
   SetWindowLong(hWndDesktop, GWL_WNDPROC, (long)lpfnDeskTopProc);
   RemoveProp(hWndDesktop, "HWNDSPREAD");
   lpfnDeskTopProc = 0;
   }
#endif

//#ifdef SPREAD_JPN
//- JPNFIX0002 - (Masanori Iwasa)
if(lpfnWndHookProc)
   {
   UnhookWindowsHookEx(hWndHook);
   FreeProcInstance(lpfnWndHookProc);
   lpfnWndHookProc = 0;
   }
//#endif
}


LPSS_COMBO SSCB_Lock(HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
HGLOBAL    hCombo = (HGLOBAL)GetWindowLongPtr(hWnd, 0);
#else
HGLOBAL    hCombo = (HGLOBAL)GetWindowLong(hWnd, 0);
#endif
LPSS_COMBO lpCombo = NULL;

if (hCombo)
   lpCombo = (LPSS_COMBO)GlobalLock(hCombo);

return (lpCombo);
}


void SSCB_Unlock(HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
HGLOBAL hCombo = (HGLOBAL)GetWindowLongPtr(hWnd, 0);
#else
HGLOBAL hCombo = (HGLOBAL)GetWindowLong(hWnd, 0);
#endif
if (hCombo)
   GlobalUnlock(hCombo);
}


HWND SSCB_GetListhWnd(HWND hWndCombo)
{
LPSS_COMBO lpCombo;
HWND       hWndList = 0;

if (lpCombo = SSCB_Lock(hWndCombo))
   {
   hWndList = lpCombo->hWndListActive;
   SSCB_Unlock(hWndCombo);
   }

return (hWndList);
}


HWND SSCB_GetEdithWnd(HWND hWndCombo)
{
LPSS_COMBO lpCombo;
HWND       hWndEdit = 0;

if (lpCombo = SSCB_Lock(hWndCombo))
   {
   hWndEdit = lpCombo->hWndEdit;
   SSCB_Unlock(hWndCombo);
   }

return (hWndEdit);
}


long SSCB_GetCurSel(HWND hWndCombo)
{
LPSS_COMBO lpCombo;
long       nCurSel = 0;

if (lpCombo = SSCB_Lock(hWndCombo))
   {
   nCurSel = lpCombo->nCurSel;
   SSCB_Unlock(hWndCombo);
   }

return (nCurSel);
}


void SSCB_SetCurSel(HWND hWndCombo, long nCurSel)
{
LPSS_COMBO lpCombo;

if (lpCombo = SSCB_Lock(hWndCombo))
   {
   lpCombo->nCurSel = nCurSel;
   SSCB_Unlock(hWndCombo);
   }
}


BOOL SSCB_GetBtnDown(HWND hWndCombo)
{
LPSS_COMBO lpCombo;
BOOL       fBtnDown = 0;

if (lpCombo = SSCB_Lock(hWndCombo))
   {
   fBtnDown = lpCombo->fBtnDown;
   SSCB_Unlock(hWndCombo);
   }

return (fBtnDown);
}


void SSCB_SetBtnDown(HWND hWndCombo, BOOL fBtnDown)
{
LPSS_COMBO lpCombo;

if (lpCombo = SSCB_Lock(hWndCombo))
   {
   lpCombo->fBtnDown = fBtnDown;
   SSCB_Unlock(hWndCombo);
   }
}


HGLOBAL SSCB_GetTextFromList(HWND hWnd, long dSel)
{
LPSS_COMBO lpCombo = SSCB_Lock(hWnd);
HGLOBAL    hText;
short      dLen;
short      nColEdit;
HWND       hWndCtrl = lpCombo->hWndListActive;

if (lpCombo->hWndList == lpCombo->hWndListActive)
   dLen = (short)SendMessage(hWndCtrl, LB_GETTEXTLEN, (WPARAM)dSel, 0L);
else
   {
   nColEdit = (short)SendMessage(lpCombo->hWndListProCombo, CBM_GETPROPERTY,
                                 CBPROP_ColumnEdit, 0);
   dLen = (short)SendMessage(hWndCtrl, LBMx_GETCOLTEXTLEN, nColEdit, 0L);
   }

if (hText = GlobalAlloc(GHND, (dLen + 1) * sizeof(TCHAR)))
   {
   LPTSTR lpText = (LPTSTR)GlobalLock(hText);

   if (lpCombo->hWndList == lpCombo->hWndListActive)
      SendMessage(hWndCtrl, LB_GETTEXT, (WPARAM)dSel, (LPARAM)lpText);
   else
      SendMessage(hWndCtrl, LBMx_GETCOLTEXT, nColEdit, (LPARAM)lpText);

#if defined(SS_V40) && defined(_UNICODE)
   // translate string from ANSI to Unicode
// fix for bug 9639 -scl
   if( lpCombo->hWndList != lpCombo->hWndListActive )
   {
     LPTSTR buf = malloc((dLen+1)*sizeof(TCHAR));
     if( buf )
     {
       memset(buf, 0, (dLen+1)*sizeof(TCHAR));
       MultiByteToWideChar(CP_ACP, 0, (char*)lpText, dLen, buf, dLen);
       lstrcpy(lpText, buf);
       free(buf);
     }
   }
#endif

   SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)lpText);

   GlobalUnlock(hText);
   }

SSCB_Unlock(hWnd);
return (hText);
}

// fix for bug #8154 -scl
// just like above function, except this one ALWAYS expects a listpro control,
// hWnd is handle to the listpro fpCombo, and returns a TBGLOBALHANDLE instead of a HGLOBAL
TBGLOBALHANDLE SSCB_GetTextFromListEx(HWND hWnd, long dSel)
{
TBGLOBALHANDLE hText;
short          dLen;
short          nColEdit;
TCHAR          szIndex[16];

nColEdit = (short)SendMessage(hWnd, CBM_GETPROPERTY,
                             CBPROP_ColumnEdit, 0);
dLen = (short)SendMessage(hWnd, LBMx_GETCOLTEXTLEN, nColEdit, 0L) + 1;

wsprintf(szIndex, _T("%d"), dSel);

dLen += lstrlen(szIndex) + 1;

if (hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dLen * sizeof(TCHAR)))
   {
   LPTSTR lpText = (LPTSTR)tbGlobalLock(hText);
   SendMessage(hWnd, LBMx_GETCOLTEXT, nColEdit, (LPARAM)lpText);
#if defined(SS_V40) && defined(_UNICODE)
   // translate string from ANSI to Unicode
   {
     LPTSTR buf = malloc(dLen * sizeof(TCHAR));
     if( buf )
     {
       memset(buf, 0, (dLen+1)*sizeof(TCHAR));
       MultiByteToWideChar(CP_ACP, 0, (char*)lpText, dLen, buf, dLen);
       lstrcpy(lpText, buf);
       free(buf);
     }
   }
#endif
   lstrcpy(&lpText[lstrlen(lpText)+1], szIndex);
   tbGlobalUnlock(hText);
   }
return (hText);
}



void SSCB_ClearAutoSearchText(HWND hWnd)
{
#ifdef SS_V40
LPSS_COMBO lpCombo = SSCB_Lock(hWnd);

lpCombo->szAutoSearchText[0] = '\0';

SSCB_Unlock(hWnd);
#endif // SS_V40
}

typedef struct tagSS_MONITOR
   {
	RECT rcMonitor;
	RECT rcCtl;
	long lPixelsInView;
   } SS_MONITOR, FAR *LPSS_MONITOR;


BOOL CALLBACK SS_MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
LPSS_MONITOR lpMonitor = (LPSS_MONITOR)dwData;
RECT         rcTemp;

IntersectRect(&rcTemp, lprcMonitor, &lpMonitor->rcCtl);

if (rcTemp.right - rcTemp.left > 0 && rcTemp.bottom - rcTemp.top > 0)
	{
	long lPixels = (rcTemp.right - rcTemp.left) * (rcTemp.bottom - rcTemp.top);

	if (lPixels > lpMonitor->lPixelsInView)
		{
		lpMonitor->lPixelsInView = lPixels;
		CopyRect(&lpMonitor->rcMonitor, lprcMonitor);
		}
	}

return (TRUE);
}

void SS_GetMonitorRect(LPRECT lprcCtl, LPRECT lprcMonitor)
{
SS_MONITOR Monitor;
BOOL ret = FALSE;
HINSTANCE hUser;
FARPROC pEnumDisplayMonitors;

_fmemset(&Monitor, '\0', sizeof(SS_MONITOR));
CopyRect(&Monitor.rcCtl, lprcCtl);
// 25068 -scl
//EnumDisplayMonitors(NULL, NULL, SS_MonitorEnumProc, (LPARAM)&Monitor);
hUser = LoadLibrary(_T("user32.dll"));
if( hUser )
{
   pEnumDisplayMonitors = GetProcAddress(hUser, "EnumDisplayMonitors");
   if( pEnumDisplayMonitors )
      ret = (BOOL)pEnumDisplayMonitors(NULL, NULL, SS_MonitorEnumProc, (LPARAM)&Monitor);
   FreeLibrary(hUser);
}
if (ret && Monitor.rcMonitor.right - Monitor.rcMonitor.left > 0 && Monitor.rcMonitor.bottom - Monitor.rcMonitor.top > 0)
	CopyRect(lprcMonitor, &Monitor.rcMonitor);

else
	{
	short dScreenOrgX = (short)GetSystemMetrics(76);   // SM_XVIRTUALSCREEN
	short dScreenOrgY = (short)GetSystemMetrics(77);   // SM_YVIRTUALSCREEN
   short dScreenHeight = (short)GetSystemMetrics(79); // SM_CYVIRTUALSCREEN
   short dScreenWidth = (short)GetSystemMetrics(78);  // SM_CXVIRTUALSCREEN

	if (dScreenHeight == 0)
		dScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	if (dScreenWidth == 0)
		dScreenWidth = GetSystemMetrics(SM_CXSCREEN);

	if (dScreenHeight == 0 || dScreenWidth == 0)
		{
		HDC hDC = GetDC(NULL);

		if (dScreenHeight == 0)
			dScreenHeight = GetDeviceCaps(hDC, VERTRES);
		if (dScreenWidth == 0)
			dScreenWidth = GetDeviceCaps(hDC, HORZRES);

		ReleaseDC(NULL, hDC);
		}

	SetRect(lprcMonitor, dScreenOrgX, dScreenOrgY, dScreenOrgX + dScreenWidth, dScreenOrgY + dScreenHeight);
	}
}


#else

BOOL tbSSRegisterComboBox(HANDLE hInstance)
{
return (TRUE);
}

#endif
