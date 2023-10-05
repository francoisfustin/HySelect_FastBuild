/*********************************************************
* SS_EDIT.C
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
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <process.h>
//#ifdef SPREAD_JPN
#ifdef WIN32
  #include <imm.h>
#else
  #include <ime.h>
#endif
//#endif
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_cell.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_main.h"
#include "ss_scrol.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_w32s.h"
#include "..\classes\checkbox.h"
#include "..\classes\superbtn.h"
#include "..\edit\editfld.h"

#ifdef SS_VB
//#include <vbapi.h>
#define VBAPI_H 1
#endif

#ifdef SS_USEAWARE
#include "utools.h"
#endif

static FARPROC lpfnEditProc = 0;
static FARPROC lpfnEditHScrollProc = 0;
static FARPROC lpfnPicProc = 0;
static FARPROC lpfnDateProc = 0;
static FARPROC lpfnTimeProc = 0;
static FARPROC lpfnIntegerProc = 0;
static FARPROC lpfnFloatProc = 0;
#ifdef SS_V40
static FARPROC lpfnCurrencyProc = 0;
static FARPROC lpfnNumberProc = 0;
static FARPROC lpfnPercentProc = 0;
#endif // SS_V40
#ifdef SS_V70
static FARPROC lpfnScientificProc = 0;
#endif // SS_V70
static FARPROC lpfnButtonProc = 0;
static FARPROC lpfnCheckBoxProc = 0;
static FARPROC lpfnComboBoxProc = 0;
static FARPROC lpfnListBoxProc = 0;

FARPROC  lpfnDeskTopProc;
FARPROC  lpfnSSDeskTopProc;

BOOL     SS_UnRegisterEditHScroll(HANDLE hInstance);
WINENTRY SS_EditProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SS_EditHScrollProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SSx_EditProc(FARPROC lpfnProc, LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd,
                      UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT  SS_EditPaste(FARPROC lpfnProc, LPSPREADSHEET lpSS, HWND hWnd, UINT Msg,
                      WPARAM wParam, LPARAM lParam);
WINENTRY SS_PicProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SS_DateProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SS_TimeProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SS_IntegerProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SS_FloatProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#ifdef SS_V40
WINENTRY SS_CurrencyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SS_NumberProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SS_PercentProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif // SS_V40
#ifdef SS_V70
WINENTRY SS_ScientificProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif // SS_V70
LRESULT  SS_BaseProc(FARPROC lpfnProc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL     SS_RegistertbSuperBtn(HANDLE hInstance);
BOOL     SS_UnRegistertbSuperBtn(HANDLE hInstance);
WINENTRY SS_ButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL     SS_RegistertbCheckBox(HANDLE hInstance);
BOOL     SS_UnRegistertbCheckBox(HANDLE hInstance);
WINENTRY SS_CheckBoxProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL     SS_UnRegisterListBox(HANDLE hInstance);
WINENTRY SS_ComboBoxProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT  SSx_ComboBoxProc(LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINENTRY SS_ListBoxProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT  SSx_ListBoxProc(LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT  SS_EditProcessTab(HWND hWndSS, UINT Msg, WPARAM wParam,
                           LPARAM lParam, WORD wShift);
void     SS_ShowEditField(LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd);
void     SS_EditPopupMenu(HWND hWndSS, HWND hWnd, WORD xPos, WORD yPos);

//- This macro was made for easy comparison on Character set. (Masanori Iwasa)
#define CHARSET_TURNON_IME(Edit) \
                (Edit.ChrSet == SS_CHRSET_ALLCHR_IME || \
                 Edit.ChrSet == SS_CHRSET_KNJONLY_IME)

#define IDM_CUT    1100
#define IDM_COPY   1101
#define IDM_PASTE  1102
#define IDM_DELETE 1103
#define IDM_SELECT 1104

// This is used because of a bug in XP when using manifests
/*
BOOL SS_GetClassInfo(LPCSTR lpszClassName, LPWNDCLASS lpwc)
{
BOOL fRet = TRUE;

if (!GetClassInfo(NULL, lpszClassName, lpwc))
	{
	HWND hWndCtrl;

   hWndCtrl = CreateWindow(lpszClassName, NULL, WS_POPUP, 0, 0, 0, 0, 0, 0, hDynamicInst, NULL);

	if (hWndCtrl)
		{
		_fmemset(lpwc, '\0', sizeof(WNDCLASS));

		lpwc->style = GetClassLong(hWndCtrl, GCL_STYLE);
		lpwc->lpfnWndProc = (WNDPROC)GetClassLong(hWndCtrl, GCL_WNDPROC);
		lpwc->cbClsExtra = GetClassLong(hWndCtrl, GCL_CBCLSEXTRA);
		lpwc->cbWndExtra = GetClassLong(hWndCtrl, GCL_CBWNDEXTRA);
		lpwc->hInstance = (HINSTANCE)GetClassLong(hWndCtrl, GCL_HMODULE);
		lpwc->hCursor = (HICON)GetClassLong(hWndCtrl, GCL_HCURSOR);

		DestroyWindow(hWndCtrl);
		}
	else
		fRet = FALSE;
	}

return (fRet);
}
*/

#if 0 // Shannon
void SS_LogMsg(LPTSTR lpStr)
{
OFSTRUCT    OfStruct;
HFILE       hFile;
TCHAR       Buffer[200];
LPTSTR      Dir = _T("\\fp.log");
/*
static TCHAR Dir[144];

if (!Dir[0])
   {
   GetSystemDirectory(Dir, STRING_SIZE(Dir));
   lstrcpy(&Dir[lstrlen(Dir)], _T("\\fp.log"));
   }

#ifdef _UNICODE  // if unicode, convert WCHAR string to CHAR string for 
                // functions which take LPSTR (not LPWSTR).
   {
     CHAR szBuff[STRING_SIZE(Dir)];
     WideCharToMultiByte(CP_ACP, 0, Dir, -1, szBuff, sizeof(szBuff);
     strcpy((LPSTR)Dir, szBuff);
   }
#endif
*/

if (OpenFile((LPSTR)Dir, &OfStruct, OF_EXIST) == HFILE_ERROR)
   {
   hFile = OpenFile((LPSTR)Dir, &OfStruct, OF_CREATE | OF_WRITE);
   _lclose(hFile);
   }

hFile = OpenFile((LPSTR)Dir, &OfStruct, OF_WRITE);

lstrcpy(Buffer, lpStr);

// Add a time stamp to the log
{
TIMEFORMAT TimeFormat = {TRUE, TRUE, ':', FALSE};

lstrcat(Buffer, _T(", "));
TimeGetCurrentTime(&Buffer[lstrlen(Buffer)], &TimeFormat);
}

lstrcpy(&Buffer[lstrlen(Buffer)], _T("\r\n"));

_llseek(hFile, 0, 2);
_lwrite(hFile, Buffer, lstrlen(Buffer)*sizeof(TCHAR));
_lclose(hFile);
}
#endif // Shannon


BOOL SS_RegisterEdit(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBEdit, &wc))
   {
   lpfnEditProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_EditProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSEdit;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }

SS_RegisterEditHScroll(hInstance);
return bRet;
}


BOOL SS_UnRegisterEdit(HANDLE hInstance)
{
BOOL bRet = FALSE;

bRet = UnregisterClass(SSClassNames[dClassNameIndex].SSEdit, hInstance);
bRet = SS_UnRegisterEditHScroll(hInstance);

return (bRet);
}


BOOL SS_RegisterEditHScroll(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     Ret = FALSE;

if (lpfnEditHScrollProc)
	return (TRUE);

#ifdef SS_USEAWARE
if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBEdit, &wc))
#else
if (GetClassInfo(NULL, _T("Edit"), &wc))
#endif
   {
   lpfnEditHScrollProc = (FARPROC)wc.lpfnWndProc;

	// RFW - 9/2/03 - This solves the problem with the cursor
	// overflowing to the next line for small row heights.
	wc.style &= ~CS_PARENTDC;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_EditHScrollProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSEditHScroll;

   Ret = RegisterClass(&wc);
   }

return (Ret);
}


BOOL SS_UnRegisterEditHScroll(HANDLE hInstance)
{
if (lpfnEditHScrollProc)
	return (UnregisterClass(SSClassNames[dClassNameIndex].SSEditHScroll, hInstance));

return (TRUE);
}


WINENTRY SS_EditProc(hWnd, Msg, wParam, lParam)

HWND          hWnd;
UINT          Msg;
WPARAM        wParam;
LPARAM        lParam;
{
HWND          hWndSS = GetParent(hWnd);
LPSPREADSHEET lpSS = SS_SheetLockActive(hWndSS);
LRESULT       lRet;

lRet = SSx_EditProc(lpfnEditProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam);
SS_SheetUnlockActive(hWndSS);
return (lRet);
}


WINENTRY SS_EditHScrollProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
HWND          hWndSS = GetParent(hWnd);
LPSPREADSHEET lpSS = SS_SheetLockActive(hWndSS);
LRESULT       lRet = 0;

switch (Msg)
   {
#if 0
   case WM_KEYDOWN:
      {
      if (SS_IsActionKey(lpSS->lpBook, SS_KBA_CLEAR, (WORD)wParam))
         {
         WPARAM wMsgLastPrev = lpSS->wMsgLast;

         SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);
         #ifdef SS_VB
         if( WM_KEYDOWN == lpSS->wMsgLast )
         #endif
            {
            SetWindowText(hWnd, _T(""));
            SendMessage(hWnd, EM_SETMODIFY, TRUE, 0L);
            }

         lpSS->wMsgLast = wMsgLastPrev;
         return (0);
         }
      }
      break;
#endif

   case WM_CHAR:
      // BJO 26Jun97 TEL5701 - Begin fix
      if (!SSGetBool(GetParent(hWnd), SSB_AUTOCLIPBOARD) &&
          (3 == wParam ||  // <Ctl><C> copy
           22 == wParam || // <Ctl><V> paste
           24 == wParam || // <Ctl><X> cut
           26 == wParam))  // <Ctl><Z> undo
         return 0;
      // BJO 26Jun97 TEL5701 - End fix
      // BJO 16Oct96 JAP5662 - Begin fix
      if (wParam <= 31 &&  // control character
          SSGetBool(GetParent(hWnd), SSB_NOBEEP) &&
          wParam != 3 &&   // <Ctl><C> copy
          wParam != 8 &&   // backspace
          wParam != 9 &&   // tab
          wParam != 13 &&  // carriage return
          wParam != 22 &&  // <Ctl><V> paste
          wParam != 24 &&  // <Ctl><X> cut
          wParam != 26 )   // <Ctl><Z> undo
         return 0;
      // BJO 16Oct96 JAP5662 - End fix
      break;

   }

lRet = SSx_EditProc(lpfnEditHScrollProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam);
SS_SheetUnlockActive(hWndSS);
return (lRet);
}


int GetSetIMEMode(HWND hWnd, BOOL fGet, BOOL fStatus)
{
   int          nRet=0;   
#ifdef SPREAD_JPN
#ifdef WIN32

   HIMC    hImc;

   //- Get Ime Context handle
   hImc = ImmGetContext(hWnd);
    
   if(fGet)
      nRet = ImmGetOpenStatus(hImc);
   else
// 96' 6/20 Modified by BOC Gao. for a bug.
//      ImmSetOpenStatus(hImc, TRUE);
      ImmSetOpenStatus(hImc, fStatus);
// --------------------------------------<<

   //- Release the Ime Context handle.
   ImmReleaseContext(hWnd, hImc);

#else

   HANDLE       hMem;
   LPIMESTRUCT  lpIME;
    
   hMem = GlobalAlloc((GMEM_MOVEABLE | GMEM_LOWER), (LONG)(sizeof(IMESTRUCT)));

   lpIME = (LPIMESTRUCT)GlobalLock(hMem);
   lpIME->fnc = fGet ? IME_GETOPEN : IME_SETOPEN;
   lpIME->wParam = fStatus;             //- Set uses this only.
   SendIMEMessage(NULL, (LONG)hMem);
   nRet = fGet ? lpIME->wParam : 0;

   GlobalUnlock(hMem);
   GlobalFree(hMem);  

#endif
#endif
   return (nRet);
}


WINENTRY SSx_EditProc(FARPROC lpfnProc, LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd,
                      UINT Msg, WPARAM wParam, LPARAM lParam)
{
SS_CELLTYPE CellType;
WORD        wShift = 0;
BOOL        fCtrlState;
LRESULT     lRet;

if (Msg == WM_COPY || Msg == WM_CUT)
   lRet = 0;

switch (Msg)
   {
   case WM_SETFOCUS:
      if(lpSS->lpBook->EditModeOn || lpSS->lpBook->EditModeTurningOn) {
            SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);

         if (CellType.Type == SS_TYPE_EDIT && CHARSET_TURNON_IME(CellType.Spec.Edit) )
            GetSetIMEMode(hWnd, FALSE, TRUE);
      }
      break;

    //- Automatically turns off the IME when character set is set to 5 or 6.
   case WM_KILLFOCUS:
      if(lpSS->lpBook->EditModeOn || lpSS->lpBook->EditModeTurningOff) {
         SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);

         if (CellType.Type == SS_TYPE_EDIT && GetSetIMEMode(hWnd, TRUE, FALSE) &&
                                CHARSET_TURNON_IME(CellType.Spec.Edit) )
            GetSetIMEMode(hWnd, FALSE, FALSE);
		//Add by BOC 99.7.6 (hyt)-----------------------------------------
		//for IME caret not destroy at 95&98
		#ifdef SPREAD_JPN
		  else
		  {
			  HIMC hImc = ImmGetContext(hWnd);
		  	  if(hImc)
			  {
					ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0); 
					ImmReleaseContext(hWnd, hImc);
			  }
		  }
		#endif
		//-----------------------------------------------------------------
      }
      break;

   case WM_PASTE:
//SS_LogMsg("Test 2"); // Shannon
      return (SS_EditPaste(lpfnProc, lpSS, hWnd, Msg, wParam, lParam));

   case WM_KEYDOWN:
		{
      BOOL fAutoClipboard = SSGetBool(GetParent(hWnd), SSB_AUTOCLIPBOARD);
		BOOL fControlDown = HIBYTE(GetKeyState(VK_CONTROL));

      if ((wParam == VK_INSERT && HIBYTE(GetKeyState(VK_SHIFT))) ||
          (fControlDown && (wParam == 'v' || wParam == 'V')))
         {
         if (fAutoClipboard)
            return (SS_EditPaste(lpfnProc, lpSS, hWnd, Msg, wParam, lParam));
         else
            {
            SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);
            return 0;
            }
         }
		/*
		else if (fControlDown && (wParam == 'x' || wParam == 'X'))
			{
			if (fAutoClipboard)
				SSClipboardCut(GetParent(hWnd));

         SS_SendMessageToVB(GetParent(hWnd), Msg, wParam, lParam);
			return (0);
			}

		else if (fControlDown && (wParam == 'c' || wParam == 'C'))
			{
			if (fAutoClipboard)
				SSClipboardCopy(GetParent(hWnd));

         SS_SendMessageToVB(GetParent(hWnd), Msg, wParam, lParam);
			return (0);
			}
		*/

      if (fControlDown && (wParam == 'c' || wParam == 'C'))
         break;
		}

      switch (wParam)
         {
         case VK_UP:
         case VK_DOWN:
            if (GetWindowLong(hWnd, GWL_STYLE) & ES_MULTILINE)
               {
               fCtrlState = HIBYTE(GetKeyState(VK_CONTROL));

               wShift = 0;

               if (HIBYTE(GetKeyState(VK_SHIFT)))
                  wShift |= 0x01;

               if (HIBYTE(GetKeyState(VK_CONTROL)))
                  wShift |= 0x02;

               if ((lpSS->lpBook->fArrowsExitEditMode && !fCtrlState &&
                    !HIBYTE(GetKeyState(VK_SHIFT))) ||
                   (!lpSS->lpBook->fArrowsExitEditMode && fCtrlState))
                  {
                  SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, 0L);
                  return (0);
                  }

               if (SS_SendMsg(lpSS->lpBook, lpSS, SSM_KEYDOWN,
                              GetWindowID(GetParent(hWnd)),
                              MAKELONG(wParam, wShift)))
                  return (0);

               return (CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam));
               }

            break;

         case VK_LEFT:
         case VK_RIGHT:
            if (GetWindowLong(hWnd, GWL_STYLE) &
                (ES_MULTILINE | ES_AUTOHSCROLL))
               {
               fCtrlState = HIBYTE(GetKeyState(VK_CONTROL));

               if ((lpSS->lpBook->fArrowsExitEditMode && !fCtrlState &&
                    !HIBYTE(GetKeyState(VK_SHIFT))) ||
                   (!lpSS->lpBook->fArrowsExitEditMode && fCtrlState))
                  {
                  SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
                  return (0);
                  }

               else if (lpSS->lpBook->fArrowsExitEditMode && fCtrlState)
                  {
                  BYTE pbKeyState[256];
                  BYTE bControlState;

                  GetKeyboardState(pbKeyState);
                  bControlState = pbKeyState[VK_CONTROL];
                  pbKeyState[VK_CONTROL] = 0;
                  SetKeyboardState(pbKeyState);
                  CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam);
                  pbKeyState[VK_CONTROL] = bControlState;
                  SetKeyboardState(pbKeyState);
                  return (0);
                  }

               SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

               return (CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam));
               }

            break;

         case VK_RETURN:
            if (GetWindowLong(hWnd, GWL_STYLE) & ES_WANTRETURN)
               {
               if (HIBYTE(GetKeyState(VK_SHIFT)))
                  wShift |= 0x01;

               if (HIBYTE(GetKeyState(VK_CONTROL)))
                  wShift |= 0x02;

               if (SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN,
                                         GetWindowID(hWndSS),
                                         MAKELONG(wParam, wShift)))
                  return (0);

               return (CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam));
               }

            break;
         }

      break;

	// RFW - 3/5/07 - 19759
	/* This fix appeared to break more than it fixed.
	case WM_IME_ENDCOMPOSITION:
      if (GetWindowLong(hWnd, GWL_STYLE) & ES_WANTRETURN)
         {
         if (HIBYTE(GetKeyState(VK_SHIFT)))
            wShift |= 0x01;

         if (HIBYTE(GetKeyState(VK_CONTROL)))
            wShift |= 0x02;

         if (SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN, GetWindowID(hWndSS),
                                   MAKELONG(VK_RETURN, wShift)))
            return (0);

         return (CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam));
         }

      SS_SendMessageToVB(hWndSS, SSM_KEYDOWN, VK_RETURN, MAKELONG(VK_RETURN, wShift));
      PostMessage(hWndSS, SSM_PROCESSENTERKEY, 0, 0L);
      return (TRUE);
	*/

#ifdef _UNICODE
	case WM_IME_CHAR: // RFW - 2/27/04 - 13757
		Msg = WM_CHAR;
#endif

   case WM_CHAR:
#ifdef	SPREAD_JPN
	  //- Wei Feng for Incident 183
		if(lpSS->lpBook->EditModeOn)
			SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);

      if (CellType.Type == SS_TYPE_EDIT)
			{
			if(CellType.Spec.Edit.Len <= 0)
				{
				SS_Beep(lpSS->lpBook);
			   return 0;
		      }
	      }
#endif

      if (lpSS->fFormulaMode)
         _fmemcpy(&CellType, &lpSS->lpBook->FormulaCellType,
                  sizeof(SS_CELLTYPE));
      else
			{
			/*
			long  lSel = (long)SendMessage(hWnd, EM_GETSEL, 0, 0);
			BOOL  fHighlighted = (HIWORD(lSel) > LOWORD(lSel));
			*/
	
         SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);

			/*
			// RFW - 9/4/03 - 1812
			if (GetWindowTextLength(hWnd) >= CellType.Spec.Edit.Len && !fHighlighted)
				{
				SS_Beep(lpSS->lpBook);
			   return 0;
		      }
			*/
			}

      if (SSGetBool(GetParent(hWnd), SSB_AUTOCLIPBOARD) &&
          (3 == wParam ||  // <Ctl><C> copy
           22 == wParam || // <Ctl><V> paste
           24 == wParam || // <Ctl><X> cut
           26 == wParam))  // <Ctl><Z> undo
         lRet = SS_DefWindowProc(lpfnProc, lpSS, hWndSS, hWnd, Msg, wParam,
                                 lParam);

      else if (wParam == '\r' && (GetWindowLong(hWnd, GWL_STYLE) & ES_WANTRETURN))
      {
         if (!SS_SendMsg(lpSS->lpBook, lpSS, SSM_KEYPRESS, GetWindowID(hWndSS),
                         (LPARAM)(LPVOID)&wParam) && wParam != 0)
            lRet = CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam);
         else
            lRet = 0;
      }
      else
         {
         //- Check to see if it was a DBCS character. (lRet = 2 is Kanji)
         lRet = SS_EditIsValidKey(lpSS->lpBook, &CellType, (short)wParam);
         if (lRet == 2)
            {
#ifndef _UNICODE
            MSG    KMsg;
            //- Peek the next character out of the queue for correct behavior.
            PeekMessage(&KMsg, hWnd, Msg, Msg, PM_REMOVE);
#endif
            return (0);
            }
         else if (lRet == 1 || wParam == '\b' || wParam == VK_RETURN)
            {
            if (CellType.Spec.Edit.ChrCase == SS_CASE_UCASE)
               #if defined(_WIN64) || defined(_IA64)
               wParam = (WORD)CharUpper((LPTSTR)MAKELONG_PTR(wParam, 0));
               #elif defined(WIN32)
               wParam = (WORD)CharUpper((LPTSTR)MAKELONG(wParam, 0));
               #else
               wParam = (WORD)(long)AnsiUpper((LPTSTR)MAKELONG(wParam, 0));
               #endif

            else if (CellType.Spec.Edit.ChrCase == SS_CASE_LCASE)
               #if defined(_WIN64) || defined(_IA64)
               wParam = (WORD)CharLower((LPTSTR)MAKELONG_PTR(wParam, 0));
               #elif defined(WIN32)
               wParam = (WORD)CharLower((LPTSTR)MAKELONG(wParam, 0));
               #else
               wParam = (WORD)(long)AnsiLower((LPTSTR)MAKELONG(wParam, 0));
               #endif

// Yagi No.7 Wei Feng 1997/9/12
#ifdef	SPREAD_JPN
			  {
				BOOL bRet = TRUE;
				bRet = lpSS->lpBook->fAutoClipboard;
				if (wParam == 3 || wParam == 24 || wParam == 22)
					if (!bRet)
						return 0;
			  }
#endif

            lRet = SS_DefWindowProc(lpfnProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam);
            }

// 96' 6/24 Added by BOC Gao. for an unknow bug.
		 else if ( wParam == 22)
            lRet = SS_DefWindowProc(lpfnProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam);
// -------------------------<<

         else
           return (0);
         }

#ifdef SS_GP
      {
      TBGLOBALHANDLE hText;
      LPSS_CELL      lpCell;
      LPSS_CELLTYPE  lpCellType;
      SS_CELLTYPE    CellTypeTemp;
      POINT          Point;
      RECT           RectCurrent;
      RECT           Rect;
      LPTSTR         lpszText;
      short          dLen;
      short          x;
      short          y;
      short          cx;
      short          cy;

      SS_GetClientRect(lpSS->lpBook, &Rect);

      lpCell = SS_LockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);

      lpCellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell,
                                       lpSS->Col.EditAt, lpSS->Row.EditAt);

      SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL,
                      lpSS->Col.EditAt, lpSS->Row.EditAt, &x, &y, &cx, &cy);

      if (x + cx > Rect.right)
         cx = Rect.right - x;

      if (y + cy > Rect.bottom)
         cy = Rect.bottom - y;

      GetWindowRect(hWnd, &RectCurrent);
      Point.x = 0;
      Point.y = 0;
      ClientToScreen(GetParent(hWnd), &Point);
      OffsetRect(&RectCurrent, -Point.x, -Point.y);

      dLen = lpCellType->Spec.Edit.Len;

      if (hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                (dLen + 1) * sizeof(TCHAR)))
         {
         lpszText = (LPTSTR)tbGlobalLock(hText);
         _fmemset(lpszText, '\0', (dLen + 1) * sizeof(TCHAR));
         GetWindowText(hWnd, lpszText, dLen + 1);
         tbGlobalUnlock(hText);
         }

      {
      BOOL fModify;

      fModify = (BOOL)SendMessage(hWnd, EM_GETMODIFY, 0, 0L);
      SS_SizeExpandingEdit(lpSS, hWnd, Rect, lpCellType, x, y, cx, cy,
                           &RectCurrent, hText);
      SendMessage(hWnd, EM_SETMODIFY, (WORD)fModify, 0L);
      }

      if (hText)
         tbGlobalFree(hText);

      SS_UnlockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);
      }
#endif
      return (lRet);
   }

return (SS_DefWindowProc(lpfnProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam));
}


LRESULT SS_EditPaste(FARPROC lpfnProc, LPSPREADSHEET lpSS, HWND hWnd, UINT Msg,
                  WPARAM wParam, LPARAM lParam)
{
GLOBALHANDLE hText;
GLOBALHANDLE hTextOld;
SS_CELLCOORD CellUL;
SS_CELLCOORD CellLR;
SS_CELLTYPE  CellType;
LPTSTR       lpszText;
LPTSTR       lpszTextTemp;
LRESULT      lRet = 0;
TCHAR        cChar;
short        nDest;
short        nLen;
short        i;

// if (lpSS->fAutoClipboard)  RFW - 4/18/00 - GIC11594
   {
   /**************************
   * Retrieve Clipboard text
   **************************/
//SS_LogMsg("Test 1"); // Shannon

   if (hTextOld = SS_GetTextFromClipboard(hWnd))
      {
      if (lpSS->fFormulaMode)
         _fmemcpy(&CellType, &lpSS->lpBook->FormulaCellType,
                  sizeof(SS_CELLTYPE));
      else
         SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);

      lpszText = (LPTSTR)GlobalLock(hTextOld);

      CellUL.Col = lpSS->Col.EditAt;
      CellUL.Row = lpSS->Row.EditAt;
      CellLR.Col = lpSS->Col.EditAt;
      CellLR.Row = lpSS->Row.EditAt;

      if (!SS_ValidatePasteData(GetParent(hWnd), lpSS, lpszText, &CellUL,
                                &CellLR, TRUE))
         {
         GlobalUnlock(hTextOld);
         return (0);
         }

      nLen = lstrlen(lpszText);
		// RFW - 8/23/04 - 14696
      if (CellType.Spec.Edit.Len == 0)
			nLen = 0;

      GlobalUnlock(hTextOld);

      if (hText = GlobalAlloc(GHND, (nLen + 1) * sizeof(TCHAR)))
         {
         lpszText = (LPTSTR)GlobalLock(hText);

         lpszTextTemp = (LPTSTR)GlobalLock(hTextOld);
         lstrcpyn(lpszText, lpszTextTemp, nLen + 1);
         GlobalUnlock(hTextOld);

         for (i = 0, nDest = 0; i < nLen; i++)
            {
            int nRet; 

            cChar = lpszText[i];

            if (CellType.Style & (ES_AUTOHSCROLL | ES_MULTILINE) &&
                (cChar == '\r' || cChar == '\n'))
               nRet = 1;
            else
               nRet = SS_EditIsValidKey(lpSS->lpBook, &CellType, lpszText[i]);

            if (nRet == 1)
               {
               if (CellType.Spec.Edit.ChrCase == SS_CASE_UCASE)
                  #if defined(_WIN64) || defined(_IA64)
                  cChar = (TCHAR)CharUpper((LPTSTR)MAKELONG_PTR((WORD)cChar, 0));
                  #elif defined(WIN32)
                  cChar = (TCHAR)CharUpper((LPTSTR)MAKELONG((WORD)cChar, 0));
                  #else
                  cChar = (TCHAR)(long)AnsiUpper((LPTSTR)MAKELONG((WORD)cChar, 0));
                  #endif

               else if (CellType.Spec.Edit.ChrCase == SS_CASE_LCASE)
                  #if defined(_WIN64) || defined(_IA64)
                  cChar = (TCHAR)CharLower((LPTSTR)MAKELONG_PTR((WORD)cChar, 0));
                  #elif defined(WIN32)
                  cChar = (TCHAR)CharLower((LPTSTR)MAKELONG((WORD)cChar, 0));
                  #else
                  cChar = (TCHAR)(long)AnsiLower((LPTSTR)MAKELONG((WORD)cChar, 0));
                  #endif

//#ifdef SPREAD_JPN
// 96' 6/24 Modified by BOC Gao. for Japanese processing
				if ( _TIsDBCSLeadByte((BYTE)cChar) )
				{
					if ( i + 1 < nLen ) 
					 {
	                     lpszText[nDest++] = cChar;
	                     lpszText[nDest++] = lpszText[i+1];
					 }
				}
				else
                   lpszText[nDest++] = cChar;
// -------------------------<<
//#else
//               lpszText[nDest++] = cChar;
//#endif
               }
// Yagi No.5 Wei Feng 1997/9/10
//#ifdef  SPREAD_JPN
				if( _TIsDBCSLeadByte((BYTE)cChar) )
					i++;
//#endif
            }

         lpszText[nDest] = '\0';
         GlobalUnlock(hText);

         if (OpenClipboard(hWnd))
            {
            EmptyClipboard();
            #if defined(UNICODE) || defined(_UNICODE)
            SetClipboardData(CF_UNICODETEXT, hText);
            #else
            SetClipboardData(CF_TEXT, hText);
            #endif
            CloseClipboard();

            lRet = SS_DefWindowProc(lpfnProc, lpSS, GetParent(hWnd), hWnd, Msg,
                                    wParam, lParam);

            if (OpenClipboard(hWnd))
               {
               EmptyClipboard();
               #if defined(UNICODE) || defined(_UNICODE)
               SetClipboardData(CF_UNICODETEXT, hTextOld);
               #else
               SetClipboardData(CF_TEXT, hTextOld);
               #endif
               CloseClipboard();
               }

            return (lRet);
            }
         }
      }
   }
//else
//   SS_SendMessageToVB(GetParent(hWnd), Msg, wParam, lParam);

return (lRet);
}


//------------------------------------------------------------
// This functions return value was changed from BOOL
// to INT in order to support DBCS. (Masanori Iwasa)
//------------------------------------------------------------
int SS_EditIsValidKey(LPSS_BOOK lpBook, LPSS_CELLTYPE CellType, short Key)
{

// for YAGI No. 4 Wei feng 1997/09/08
// RFW - 8/24/01 #ifdef	SPREAD_JPN
	if(CellType->Type == SS_TYPE_EDIT)
	{
		if(Key == 3 || Key == 24 || Key == 26)
			return TRUE;
	}
// RFW - 8/24/01 #endif

switch (CellType->Type)
   {
   //- CAUTION ------------------------------------------------------------
   //    SS_TYPE_EDIT return values
   //          0: Bad char   1: Ok   2: Kanji 1st byte (Bad char)
   //----------------------------------------------------------------------
   case SS_TYPE_EDIT:
      switch (CellType->Spec.Edit.ChrSet)
         {
         case SS_CHRSET_CHR:
         //- New character set supported in JPN version. (Masanori Iwasa)
         case SS_CHRSET_ALLCHR_IME:
            /*
            if (isascii(Key))
               return (TRUE);
            */

            return (TRUE);

         case SS_CHRSET_ALPHA:
#ifndef _UNICODE
            if (IsDBCSLeadByte((BYTE)Key))
#else
      if(HIBYTE(Key))
#endif
               return (2);
            else if (IsCharAlpha((TCHAR)Key) || Key == ' ')
               return (TRUE);

            break;

         case SS_CHRSET_ALPHANUM:
#ifndef _UNICODE
            if (IsDBCSLeadByte((BYTE)Key))
#else
      if(HIBYTE(Key))
#endif
               return (2);
				/* RFW - 6/25/04
            else if (IsCharAlphaNumeric((TCHAR)Key) || Key == ' ' || Key == '.' ||
                     Key == ',' || Key == '-')
				*/
            else if (IsCharAlphaNumeric((TCHAR)Key) || Key == ' ' ||
                     Key == lpBook->WinFloatFormat.cDecimalSign ||
                     Key == lpBook->WinFloatFormat.cSeparator || Key == '-')
               return (TRUE);

            break;

         case SS_CHRSET_NUM:
//Add by BOC 99.7.30 (hyt)------------------------------------
//for ensure can't input DBCS or single byte katakana
#ifndef _UNICODE
			if (IsDBCSLeadByte((BYTE)Key))
#else
			if(HIBYTE(Key))
#endif
               return (2);
//#ifdef SPREAD_JPN
			//Is KataKana
			if((BYTE)Key>=0xA1 && (BYTE)Key<= 0xDF)
				return (2);
//#endif
//--------------------------------------------------------------

				/* RFW - 6/25/04
            if (isdigit(Key) || Key == '.' || Key == '-')
				*/
            if (isdigit(Key) || Key == lpBook->WinFloatFormat.cDecimalSign ||
                Key == '-')
               return (TRUE);

            break;

         //- These input character sets were support in the Japanese
         //  version for DBCS only or auto FEP control. (Masanori Iwasa)
         case SS_CHRSET_KNJONLY:
         case SS_CHRSET_KNJONLY_IME:
#ifndef _UNICODE
            if(IsDBCSLeadByte((BYTE)Key) || Key == ' ')
#else
            if(HIBYTE(Key) || Key == ' ')
#endif
               return (TRUE);

            break;
         }

      break;
   }

return (FALSE);
}


#ifndef SS_NOCT_PIC

BOOL SS_RegisterPic(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBPic, &wc))
   {
   lpfnPicProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_PicProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSPic;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterPic(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSPic, hInstance));
}


WINENTRY SS_PicProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
return (SS_BaseProc(lpfnPicProc, hWnd, Msg, wParam, lParam));
}

#endif


#ifndef SS_NOCT_DATE

BOOL SS_RegisterDate(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBDate, &wc))
   {
   lpfnDateProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_DateProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSDate;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterDate(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSDate, hInstance));
}


WINENTRY SS_DateProc(hWnd, Msg, wParam, lParam)

HWND          hWnd;
UINT          Msg;
WPARAM        wParam;
LPARAM        lParam;
{
HWND          hWndSS = GetParent(hWnd);
LPSPREADSHEET lpSS;
LONG          lValue = 0L;
LRESULT       lRet;
LONG          lGlobalKey;
BOOL          fCalendar = FALSE;
BOOL          fCurrentDate = FALSE;

switch (Msg)
   {
#ifdef SS_CLIPBOARDCUTTOCOPY
   case WM_CUT:
      Msg = WM_COPY;
      break;
#endif

   case WM_KEYDOWN:
      fCalendar = SSIsActionKey(hWndSS, SS_KBA_POPUP, (WORD)wParam);
      fCurrentDate = SSIsActionKey(hWndSS, SS_KBA_CURRENT, (WORD)wParam);

      if (fCurrentDate)
         {
         GetToolBoxGlobal(TBG_CURRENTDATETIME, &lGlobalKey);
         SetToolBoxGlobal(TBG_CURRENTDATETIME, (long)wParam);
         }

#ifdef SS_CLIPBOARDCUTTOCOPY
      if ((HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'x' ||
           wParam == 'X')) ||
          (wParam == VK_DELETE && HIBYTE(GetKeyState(VK_SHIFT))))
         {
         ClipboardCopy(hWnd);
         return (0);
         }
#endif

#ifdef SS_GP
      if (wParam == VK_DELETE)
         {
         lRet = SendMessage(hWnd, EM_GETSEL, 0, 0L);

         if (LOWORD(lRet) < HIWORD(lRet))
            return (0);
         }
#endif

      break;

   case WM_LBUTTONDBLCLK:
		{
		WORD wKey;
      if (SSGetActionKey(hWndSS, SS_KBA_POPUP, NULL, NULL, &wKey) && wKey)
			fCalendar = TRUE;
		else
			return (0);
		}
      break;

   default:
      break;
   }

lpSS = SS_SheetLockActive(hWndSS);

if (fCalendar)
   {
   lpSS->lpBook->wMessageBeingSent++;
   if (Msg == WM_KEYDOWN)
      {
      GetToolBoxGlobal(TBG_POPUPCALENDAR, &lGlobalKey);
      SetToolBoxGlobal(TBG_POPUPCALENDAR, (long)wParam);
      }
   }

lRet = SS_DefWindowProc(lpfnDateProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam);

if (fCalendar)
   {
   lpSS->lpBook->wMessageBeingSent--;
   if (Msg == WM_KEYDOWN)
      SetToolBoxGlobal(TBG_POPUPCALENDAR, lGlobalKey);
   }

if (fCurrentDate)
   SetToolBoxGlobal(TBG_CURRENTDATETIME, lGlobalKey);

SS_SheetUnlockActive(hWndSS);
return (lRet);
}

#endif


#ifndef SS_NOCT_TIME

BOOL SS_RegisterTime(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBTime, &wc))
   {
   lpfnTimeProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_TimeProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSTime;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterTime(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSTime, hInstance));
}


WINENTRY SS_TimeProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
LRESULT lRet;
long lGlobalKey;
BOOL fCurrentTime = FALSE;

switch (Msg)
   {
   case WM_KEYDOWN:
      fCurrentTime = SSIsActionKey(GetParent(hWnd), SS_KBA_CURRENT, (WORD)wParam);

      if (fCurrentTime)
         {
         GetToolBoxGlobal(TBG_CURRENTDATETIME, &lGlobalKey);
         SetToolBoxGlobal(TBG_CURRENTDATETIME, (long)wParam);
         }

      break;

   default:
      break;
   }

lRet = SS_BaseProc(lpfnTimeProc, hWnd, Msg, wParam, lParam);

if (fCurrentTime)
   SetToolBoxGlobal(TBG_CURRENTDATETIME, lGlobalKey);

return (lRet);
}

#endif


#ifndef SS_NOCT_INT

BOOL SS_RegisterInteger(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBInteger, &wc))
   {
   lpfnIntegerProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_IntegerProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSInteger;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterInteger(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSInteger, hInstance));
}


WINENTRY SS_IntegerProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
return (SS_BaseProc(lpfnIntegerProc, hWnd, Msg, wParam, lParam));
}

#endif


BOOL SS_RegisterFloat(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBFloat, &wc))
   {
   lpfnFloatProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_FloatProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSFloat;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterFloat(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSFloat, hInstance));
}


WINENTRY SS_FloatProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
return (SS_BaseProc(lpfnFloatProc, hWnd, Msg, wParam, lParam));
}


#ifdef SS_V40

BOOL SS_RegisterCurrency(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBCurrency, &wc))
   {
   lpfnCurrencyProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_CurrencyProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSCurrency;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterCurrency(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSCurrency, hInstance));
}


WINENTRY SS_CurrencyProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
return (SS_BaseProc(lpfnCurrencyProc, hWnd, Msg, wParam, lParam));
}


BOOL SS_RegisterNumber(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBNumber, &wc))
   {
   lpfnNumberProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_NumberProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSNumber;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterNumber(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSNumber, hInstance));
}


WINENTRY SS_NumberProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
return (SS_BaseProc(lpfnNumberProc, hWnd, Msg, wParam, lParam));
}


BOOL SS_RegisterPercent(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBPercent, &wc))
   {
   lpfnPercentProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_PercentProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSPercent;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterPercent(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSPercent, hInstance));
}


WINENTRY SS_PercentProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
return (SS_BaseProc(lpfnPercentProc, hWnd, Msg, wParam, lParam));
}

#endif // SS_V40


#ifdef SS_V70
BOOL SS_RegisterScientific(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (GetClassInfo(NULL, _T("Edit"), &wc))
   {
   lpfnScientificProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_ScientificProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSScientific;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterScientific(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].SSScientific, hInstance));
}


void SS_GetScientificDecimal(LPSS_CELLTYPE lpCellType, LPTSTR lpszDecimal)
{
if (lpCellType->Spec.Scientific.szDecimal[0])
	lstrcpy(lpszDecimal, lpCellType->Spec.Scientific.szDecimal);
else
	{
	SSNUM_INTLSETTINGS IntlSettings;

	NumGetSystemIntlSettings(&IntlSettings);
	lstrcpy(lpszDecimal, IntlSettings.decimal_point);
	}
}


void SSx_GetScientificDecimal(LPTSTR lpszDecimalIn, LPTSTR lpszDecimalOut)
{
if (lpszDecimalIn && *lpszDecimalIn)
	lstrcpy(lpszDecimalOut, lpszDecimalIn);
else
	{
	SSNUM_INTLSETTINGS IntlSettings;

	NumGetSystemIntlSettings(&IntlSettings);
	lstrcpy(lpszDecimalOut, IntlSettings.decimal_point);
	}
}


BOOL SS_ScientificCheckRange(LPSS_CELLTYPE lpCellType, LPTSTR lpszText, BOOL fEditMode)
{
double dfVal;
double dfZero = 0.0;
double dfMin = lpCellType->Spec.Scientific.Min;
double dfMax = lpCellType->Spec.Scientific.Max;

if (!lpszText || !*lpszText)
	return (TRUE);

dfVal = SS_ScientificToFloat(lpszText, lpCellType);

if (dfMin <= dfVal && dfVal <= dfMax)
   return (TRUE);

if (fEditMode)
	{
	//  for case the range is 500 to 5000
	//  we must return TRUE for 0 to 500
	if (dfMin > dfZero &&
		 dfVal >= dfZero && dfVal < dfMin)
		return (TRUE);

	// converse
	if (dfMax < dfZero &&
		 dfVal <= dfZero && dfVal > dfMax)
		return (TRUE);
	}

return (FALSE);
}


WINENTRY SS_ScientificProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LRESULT lRet;

switch (Msg)
	{
	case WM_SETTEXT:
		{
		HWND          hWndSS = GetParent(hWnd);
		LPSPREADSHEET lpSS = SS_SheetLockActive(hWndSS);
		SS_CELLTYPE   CellType;

      SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);
		SS_SheetUnlockActive(hWndSS);

		if (lParam && *(LPTSTR)lParam && !SS_ScientificCheckRange(&CellType, (LPTSTR)lParam, FALSE))
			return (FALSE);

		lstrcpyn(lpSS->lpBook->szScientificBuffer, (LPTSTR)lParam ? (LPTSTR)lParam : (LPTSTR)"", SCIENTIFIC_BUFFER_SIZE);
		}
		break;

	case WM_KEYDOWN:
		{
		HWND      hWndSS = GetParent(hWnd);
		LPSS_BOOK lpBook = SS_BookLock(hWndSS);

		SendMessage(hWnd, EM_GETSEL, (WPARAM)&lpBook->dwScientificSel1, (WPARAM)&lpBook->dwScientificSel2);
		SS_BookUnlock(hWndSS);
		}
		break;

	case WM_CHAR:
		{
		HWND          hWndSS = GetParent(hWnd);
		LPSPREADSHEET lpSS;
		SS_CELLTYPE   CellType;
		TCHAR         szDecimal[5];

      if (!SSGetBool(GetParent(hWnd), SSB_AUTOCLIPBOARD) &&
          (3 == wParam ||  // <Ctl><C> copy
           22 == wParam || // <Ctl><V> paste
           24 == wParam || // <Ctl><X> cut
           26 == wParam))  // <Ctl><Z> undo
         return 0;

      if (wParam <= 31 &&  // control character
          SSGetBool(GetParent(hWnd), SSB_NOBEEP) &&
          wParam != 3 &&   // <Ctl><C> copy
          wParam != 8 &&   // backspace
          wParam != 9 &&   // tab
          wParam != 13 &&  // carriage return
          wParam != 22 &&  // <Ctl><V> paste
          wParam != 24 &&  // <Ctl><X> cut
          wParam != 26 )   // <Ctl><Z> undo
         return 0;

		lpSS = SS_SheetLockActive(hWndSS);
      SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);
		SS_GetScientificDecimal(&CellType, szDecimal);
		SS_SheetUnlockActive(hWndSS);

		if ('.' == wParam)
			wParam = (WPARAM)*szDecimal;

		// Only allow 1 e
		if (tolower((int)wParam) == 'e')
			{
			TCHAR szBuffer[100 + 1];
			GetWindowText(hWnd, szBuffer, 100);
			if (_tcscspn(szBuffer, _T("eE")) != (size_t)lstrlen(szBuffer))
				{
				SS_Beep(lpSS->lpBook);
				return (0);
				}
			}

      else if (isdigit((int)wParam) || (TUCHAR)wParam == szDecimal[0] || wParam == '-' || wParam == '+' ||
               wParam == '\b' || wParam == VK_RETURN)
         ;

      else if (SSGetBool(GetParent(hWnd), SSB_AUTOCLIPBOARD) &&
               (3 == wParam ||  // <Ctl><C> copy
                22 == wParam || // <Ctl><V> paste
                24 == wParam || // <Ctl><X> cut
                26 == wParam))  // <Ctl><Z> undo
			;

		else
			{
			SS_Beep(lpSS->lpBook);
			return (0);
			}

		lpSS = SS_SheetLockActive(hWndSS);
		lRet = SS_BaseProc(lpfnScientificProc, hWnd, Msg, wParam, lParam);
		lpSS->fIgnoreEditChange = FALSE;
		SS_SheetUnlockActive(hWndSS);
		}
		return (lRet);
	}

return (SS_BaseProc(lpfnScientificProc, hWnd, Msg, wParam, lParam));
}
#endif // SS_V70


LRESULT SS_BaseProc(FARPROC lpfnProc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
HWND hWndSS = GetParent(hWnd);
LPSPREADSHEET lpSS = SS_SheetLockActive(hWndSS);
LRESULT lRet;

#if SS_V80
if( lpSS == NULL )
{
   hWndSS = GetProp(hWnd, _T("HWNDSPREAD"));
   lpSS = SS_SheetLockActive(hWndSS);
}
#endif

lRet = SS_DefWindowProc(lpfnProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam);

SS_SheetUnlockActive(hWndSS);
return (lRet);
}


#ifndef SS_NOCT_BUTTON

BOOL SS_RegistertbSuperBtn(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
wc.lpfnWndProc   = (WNDPROC)tbSuperBtnWndFn;
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
wc.lpszClassName = SSClassNames[dClassNameIndex].TBSuperBtn;
if (RegisterClass(&wc))
   bRet = TRUE;
return bRet;
}


BOOL SS_UnRegistertbSuperBtn(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].TBSuperBtn, hInstance));
}


BOOL SS_RegisterButton(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;                              

SS_RegistertbSuperBtn(hInstance);
if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBSuperBtn, &wc))
   {
   lpfnButtonProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_ButtonProc;
   wc.hInstance = hInstance;
   wc.hCursor = NULL;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSSuperBtn;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterButton(HANDLE hInstance)
{
SS_UnRegistertbSuperBtn(hInstance);
return (UnregisterClass(SSClassNames[dClassNameIndex].SSSuperBtn, hInstance));
}

BOOL SSButton_OnSetCursor(HWND hwndBtn, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
HWND hwndSS;
LPSPREADSHEET lpSS;
HCURSOR hCursor;
BOOL bRet = FALSE;

hwndSS = GetParent(hwndBtn);
if (lpSS = SS_SheetLockActive(hwndSS))
   {
   hCursor = lpSS->lpBook->CursorButton.hCursor;
   switch ((LPARAM)hCursor)
      {
      case SS_CURSOR_DEFAULT:
         if (FORWARD_WM_SETCURSOR(GetParent(hwndSS), hwndCursor, codeHitTest, msg, SendMessage))
            {
            SS_SheetUnlockActive(hwndSS);
            return TRUE;
            }
         hCursor = lpSS->lpBook->CursorDefault.hCursor;
         break;
      case (LPARAM)SS_CURSOR_ARROW:
         hCursor = LoadCursor(NULL, IDC_ARROW);
         break;
      case (LPARAM)SS_CURSOR_DEFCOLRESIZE:
         hCursor = lpSS->lpBook->hCursorResizeV;
         break;
      case (LPARAM)SS_CURSOR_DEFROWRESIZE:
         hCursor = lpSS->lpBook->hCursorResizeH;
         break;
      }
   switch ((LPARAM)hCursor)
      {
      case SS_CURSOR_DEFAULT:
         hCursor = lpSS->lpBook->hCursorPointer;
         break;
      case (LPARAM)SS_CURSOR_ARROW:
         hCursor = LoadCursor(NULL, IDC_ARROW);
         break;
      case (LPARAM)SS_CURSOR_DEFCOLRESIZE:
         hCursor = lpSS->lpBook->hCursorResizeV;
         break;
      case (LPARAM)SS_CURSOR_DEFROWRESIZE:
         hCursor = lpSS->lpBook->hCursorResizeH;
         break;
       }
   SetCursor(hCursor);
   bRet = TRUE;
   SS_SheetUnlockActive(hwndSS);
   }
return FALSE;
}

WINENTRY SS_ButtonProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
if (Msg == BM_SETSTYLE)
   return (0);

switch (Msg)
   {
   case BM_SETSTYLE:
      return 0;
   case WM_SETCURSOR:
      return HANDLE_WM_SETCURSOR(hWnd, wParam, lParam, SSButton_OnSetCursor);
   case WM_KEYDOWN:
      switch (wParam)
         {
         case VK_RETURN:
				{
				HWND hWndSS = GetParent(hWnd);

            SS_SendMsgActiveSheet(hWndSS, WM_COMMAND, MAKEWPARAM(GetWindowID(hWnd), BN_CLICKED), (LPARAM)hWnd);
//				FORWARD_WM_COMMAND(hWndSS, GetWindowID(hWnd),
//										 hWnd, BN_CLICKED, SS_SendMsg);
				if (SS_IsDestroyed(hWndSS) || !IsWindow(hWnd))
					return 0;
				}
           break;
         case VK_UP:
         case VK_DOWN:
            SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, 0L);
            return (0);
         case VK_LEFT:
         case VK_RIGHT:
            SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, 0L);
            return (0);
			}

      break;
   }

return (SS_BaseProc(lpfnButtonProc, hWnd, Msg, wParam, lParam));
}

#endif


#ifndef SS_NOCT_CHECK

BOOL SS_RegistertbCheckBox(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

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
wc.lpszClassName = SSClassNames[dClassNameIndex].TBCheckBox;
if (RegisterClass(&wc))
   return bRet = TRUE;

return bRet;
}


BOOL SS_UnRegistertbCheckBox(HANDLE hInstance)
{
return (UnregisterClass(SSClassNames[dClassNameIndex].TBCheckBox, hInstance));
}


BOOL SS_RegisterCheckBox(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

SS_RegistertbCheckBox(hInstance);
if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBCheckBox, &wc))
   {
   lpfnCheckBoxProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_CheckBoxProc;
   wc.hInstance = hInstance;
   wc.hCursor = NULL;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSCheckBox;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterCheckBox(HANDLE hInstance)
{
SS_UnRegistertbCheckBox(hInstance);
return (UnregisterClass(SSClassNames[dClassNameIndex].SSCheckBox, hInstance));
}


WINENTRY SS_CheckBoxProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
switch (Msg)
   {
//   case BM_SETSTYLE:
//     return 0;

   case WM_KEYDOWN:
      switch (wParam)
         {
         case VK_RETURN:
           SS_SendMsgCommandActiveSheet(GetParent(hWnd), BN_CLICKED, FALSE);
           break;
         case VK_UP:
         case VK_DOWN:
            SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, 0L);
            return (0);
         case VK_LEFT:
         case VK_RIGHT:
#ifdef SS_UTP
            FORWARD_WM_COMMAND(GetParent(hWnd), GetWindowID(hWnd), hWnd,
                               wParam == VK_LEFT ? EN_AUTOADVANCEPREV :
                               EN_AUTOADVANCENEXT, SendMessage);
#else
            SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, 0L);
#endif
            return (0);
         }

      break;
   
   case WM_SETCURSOR:
      return HANDLE_WM_SETCURSOR(hWnd, wParam, lParam, SSButton_OnSetCursor);
   }

return (SS_BaseProc(lpfnCheckBoxProc, hWnd, Msg, wParam, lParam));
}

#endif


#ifndef SS_NOCT_COMBO

BOOL SS_RegisterComboBox(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

SS_RegisterListBox(hInstance);
tbSSRegisterComboBox(hInstance);
if (GetClassInfo(NULL, SSClassNames[dClassNameIndex].TBComboBox, &wc))
   {
   lpfnComboBoxProc = (FARPROC)wc.lpfnWndProc;
   wc.style |= CS_GLOBALCLASS;
   wc.lpfnWndProc = (WNDPROC)SS_ComboBoxProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSComboBox;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }
return bRet;
}


BOOL SS_UnRegisterComboBox(HANDLE hInstance)
{
SS_UnRegisterListBox(hInstance);
tbSSUnRegisterComboBox(hInstance);
return (UnregisterClass(SSClassNames[dClassNameIndex].SSComboBox, hInstance));
}


WINENTRY SS_ComboBoxProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
HWND hWndSS = GetParent(hWnd);
LPSPREADSHEET lpSS = SS_SheetLockActive(hWndSS);
LRESULT lRet;

lRet = SSx_ComboBoxProc(lpSS, hWndSS, hWnd, Msg, wParam, lParam);

SS_SheetUnlockActive(hWndSS);
return (lRet);
}


LRESULT SSx_ComboBoxProc(LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPMEASUREITEMSTRUCT lpmis;
LPDRAWITEMSTRUCT    lpdis;
SS_COLORTBLITEM     ForeColorTblItem;
SS_COLORTBLITEM     BackColorTblItem;
TEXTMETRIC          tm;
COLORREF            BkColorOld;
COLORREF            ColorOld;
HBRUSH              hBrush;
LPTSTR              lpData;
short               Shift;
LRESULT             lRet;
HDC                 hDC;

switch (Msg)
   {
   case WM_SETFOCUS:
      /*
      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      */
      break;

   case WM_KILLFOCUS:
		if (GetCapture() == hWnd)
			ReleaseCapture();

      break;

   case WM_KEYDOWN:
      Shift = 0;

      SS_ShowEditField(lpSS, hWndSS, hWnd);

      if (HIBYTE(GetKeyState(VK_SHIFT)))
         Shift |= 0x01;

      if (HIBYTE(GetKeyState(VK_CONTROL)))
         Shift |= 0x02;

#ifdef SS_GP
      if (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'z' || wParam == 'Z'))
         wParam = VK_ESCAPE;
#endif

      switch (wParam)
         {
         case VK_ESCAPE:
            if (SS_SendMsg(lpSS->lpBook, lpSS, SSM_KEYDOWN, GetWindowID(hWndSS),
                           MAKELONG(wParam, Shift)))
               return (0);

            lpSS->EscapeInProgress = TRUE;

            SS_PostSetEditMode(lpSS, FALSE, 0L);
            return (TRUE);

         case VK_RETURN:
            if (!SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN,
                            GetWindowID(hWndSS),
                            MAKELONG(wParam, Shift)))
               PostMessage(hWndSS, SSM_PROCESSENTERKEY, 0, 0L);

            return (TRUE);

         case VK_F1:
            SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
            return (0);

         case VK_RIGHT:
         case VK_LEFT:
				{
            BOOL fCtrlState = HIBYTE(GetKeyState(VK_CONTROL));

				// RFW - 6/10/04 - 14343
            if ((lpSS->lpBook->fArrowsExitEditMode && !fCtrlState &&
                 !HIBYTE(GetKeyState(VK_SHIFT))) ||
                (!lpSS->lpBook->fArrowsExitEditMode && fCtrlState))
               {
               SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
               return (0);
               }

            if (SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN,
                                      GetWindowID(hWndSS),
                                      MAKELONG(wParam, Shift)))
               return (0);
				}
            break;

         case VK_HOME:
         case VK_END:
         case VK_UP:
         case VK_DOWN:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               {
               SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
               return (0);
               }

            if (SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN,
                                      GetWindowID(hWndSS),
                                      MAKELONG(wParam, Shift)))
               return (0);

            break;

         case VK_TAB:
            return (SS_EditProcessTab(hWndSS, Msg, wParam, lParam, Shift));

         default:
            if (SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN,
                                      GetWindowID(hWndSS),
                                      MAKELONG(wParam, Shift)))
               return (0);
         }

      lpSS->lpBook->fComboKeyDown = TRUE;

      lRet = CallWindowProc((WNDPROC)lpfnComboBoxProc, hWnd, Msg, wParam, lParam);
      lpSS->lpBook->fComboKeyDown = FALSE;
		return (lRet);

   case WM_MEASUREITEM:
      lpmis = (LPMEASUREITEMSTRUCT)lParam;

      hDC = fpGetDC(hWnd);
      GetTextMetrics(hDC, &tm);
      ReleaseDC(hWnd, hDC);
      lpmis->itemHeight = tm.tmHeight;

      return (0);

#ifdef SS_VB
// RFW - 8/24/01 #ifdef SPREAD_JPN
   //------------------------------------------------------------
   // DO NOT TAKE OUT THIS PREPROCESSOR.
   // This was added to bypass the character handling bug in
   // Visual Basic 2.0J.
   // JPNFIX0004 - (Masanori Iwasa)
   //------------------------------------------------------------
   case WM_CHAR:
      return (CallWindowProc((WNDPROC)lpfnComboBoxProc, hWnd, Msg, wParam, lParam));
// RFW - 8/24/01 #endif
#endif

   case WM_DRAWITEM:
      lpdis = (LPDRAWITEMSTRUCT)lParam;

      if (((lpdis->itemAction & ODA_DRAWENTIRE) || (lpdis->itemAction &
          ODA_SELECT)) && lpdis->itemData)
         {
         lpData = (LPTSTR)lpdis->itemData;

         if (lpdis->itemState & ODS_SELECTED)
            {
            BkColorOld = SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
            ColorOld = SetTextColor(lpdis->hDC,
                                    GetSysColor(COLOR_HIGHLIGHTTEXT));
            }

         else
            {
            SS_GetColorTblItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt,
                               &BackColorTblItem, &ForeColorTblItem);

            BkColorOld = SetBkColor(lpdis->hDC, SS_TranslateColor(BackColorTblItem.Color));
            ColorOld = SetTextColor(lpdis->hDC, SS_TranslateColor(ForeColorTblItem.Color));
            }

         hBrush = CreateSolidBrush(GetBkColor(lpdis->hDC));
         FillRect(lpdis->hDC, &lpdis->rcItem, hBrush);
         DeleteObject(hBrush);

         SetBkMode(lpdis->hDC, TRANSPARENT);

         TextOut(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpData,
                 lstrlen(lpData));

         SetBkColor(lpdis->hDC, BkColorOld);
         SetTextColor(lpdis->hDC, ColorOld);
         }

      if (lpdis->itemState & ODS_SELECTED)
         DrawFocusRect(lpdis->hDC, &lpdis->rcItem);

      return (0);

   }

lRet = SS_DefWindowProc(lpfnComboBoxProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam);

return (lRet);
}


BOOL SS_RegisterListBox(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = FALSE;

if (lpfnListBoxProc)
	return (TRUE);

if (GetClassInfo(NULL, _T("ListBox"), &wc))
   {
   lpfnListBoxProc = (FARPROC)wc.lpfnWndProc;
//   wc.style |= CS_GLOBALCLASS | CS_PARENTDC;
   wc.style |= CS_OWNDC;
   wc.lpfnWndProc = (WNDPROC)SS_ListBoxProc;
   wc.hInstance = hInstance;
   wc.lpszClassName = SSClassNames[dClassNameIndex].SSListBox;
   if (RegisterClass(&wc))
      bRet = TRUE;
   }

return bRet;
}


BOOL SS_UnRegisterListBox(HANDLE hInstance)
{
if (lpfnListBoxProc)
	return (UnregisterClass(SSClassNames[dClassNameIndex].SSListBox, hInstance));

return (TRUE);
}


WINENTRY SS_ListBoxProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
HWND hWndSS = GetProp(hWnd, _T("HWNDSPREAD"));
LPSPREADSHEET lpSS = SS_SheetLockActive(hWndSS);
LRESULT lRet;

lRet = SSx_ListBoxProc(lpSS, hWndSS, hWnd, Msg, wParam, lParam);

SS_SheetUnlockActive(hWndSS);
return (lRet);
}


LRESULT SSx_ListBoxProc(LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
HFONT      hFontOld;
POINT      Point;
RECT       Rect;
HWND       hWndTemp;
short      Shift;
#if defined(_WIN64) || defined(_IA64)
long       dCurSel;
#else
short      dCurSel;
#endif
short      dSel;
short      dCount;
HDC        hDC;
TEXTMETRIC tm;
LRESULT    lRet;

hWndTemp = GetProp(hWnd, _T("HWNDCOMBO"));

switch (Msg)
   {
   case WM_SETFOCUS:
      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      break;

   case WM_MOUSEACTIVATE:
      return (MA_NOACTIVATE);

   case WM_LBUTTONDOWN:
      hDC = fpGetDC(hWnd);
      hFontOld = SelectObject(hDC, (HANDLE)SendMessage(hWnd,
                              WM_GETFONT, 0, 0L));
      GetTextMetrics(hDC, &tm);
      SelectObject(hDC, hFontOld);
      ReleaseDC(hWnd, hDC);

      dCurSel = (HIWORD(lParam) / tm.tmHeight) +
                (short)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0L);

      dCurSel = min(dCurSel, (short)SendMessage(hWnd, LB_GETCOUNT, 0, 0L) - 1);

      SetCapture(hWnd);

		/* RFW - 7/22/03 - 12291
      if (dCurSel != (short)SendMessage(hWnd, LB_GETCURSEL, 0, 0L))
         {
         SendMessage(hWnd, LB_SETCURSEL, dCurSel, 0L);

         hWndTemp = GetProp(hWnd, _T("HWNDCOMBO"));
         FORWARD_WM_COMMAND(hWndTemp, GetWindowID(hWnd), hWnd, LBN_SELCHANGE,
                            SendMessage);
         }
		*/

      if (dCurSel != (short)SendMessage(hWnd, LB_GETCURSEL, 0, 0L))
         SendMessage(hWnd, LB_SETCURSEL, dCurSel, 0L);

      hWndTemp = GetProp(hWnd, _T("HWNDCOMBO"));
      FORWARD_WM_COMMAND(hWndTemp, GetWindowID(hWnd), hWnd, LBN_SELCHANGE,
                         SendMessage);
      return (0);

   case WM_LBUTTONUP:
		// RFW - 4/28/02 - BUG 001-001
		if (GetCapture() == hWnd)
	      PostMessage(hWndSS, SSM_PROCESSENTERKEY, 0, 0L);
      ReleaseCapture();
      return (0);

   case WM_MOUSEMOVE:
		/* RFW - 4/25/05 - 16137
      if (wParam & MK_LBUTTON)
		*/
      if ((wParam & MK_LBUTTON) && IsWindowVisible(hWnd))
         {
         hWndTemp = GetCapture();
         if (hWndTemp && hWndTemp != hWnd)
            {
            SendMessage(hWndTemp, WM_LBUTTONUP, 0, 0L);
            SendMessage(hWnd, WM_LBUTTONDOWN, 0, 0L);
            }
         else
            {
            GetClientRect(hWnd, &Rect);
            hDC = fpGetDC(hWnd);
            hFontOld = SelectObject(hDC, (HANDLE)SendMessage(hWnd,
                                    WM_GETFONT, 0, 0L));
            GetTextMetrics(hDC, &tm);
            SelectObject(hDC, hFontOld);
            ReleaseDC(hWnd, hDC);

#ifdef SPREAD_JPN
            //- JPNFIX0011 - (Masanori Iwasa)
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
#endif

            for (;;)
               {
					// BUG SPR_CEL_003_008 (1-1)
					// The List Box Here Is The List Of Combo Box.
					// When User Drag Mouse In The List Box,
					// It Will Do The Loop Until Mouse Release.
					// During This Time, It Does Not Respond To Other Message.
					// This Will Cause Bug SPR_CEL_003_008.
					// Check For ESC Key Here, If Press, Exit The Loop, 
					// Then It Can Respond To The ESC Key.
					// Modified By HaHa 1999.10.27
					if (GetAsyncKeyState(VK_ESCAPE))
						break;

               if (GetCapture() != hWnd)
                  break;

               if (GetSystemMetrics(SM_SWAPBUTTON))
                  {
                  if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
                     break;
                  }
               else
                  if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
                     break;

               GetCursorPos(&Point);
               ScreenToClient(hWnd, &Point);

               dSel = (short)SendMessage(hWnd, LB_GETCURSEL, 0, 0L);
               dCount = (short)SendMessage(hWnd, LB_GETCOUNT, 0, 0L);

               if (Point.y < 0)
                  {
                  if (dSel > 0)
                     SendMessage(hWnd, WM_KEYDOWN, VK_UP, 0L);
                  }

               else if (Point.y > Rect.bottom)
                  {
                  if (dSel < dCount - 1)
                     SendMessage(hWnd, WM_KEYDOWN, VK_DOWN, 0L);
                  }

               else
                  {
                  dCurSel = (Point.y / tm.tmHeight) +
                            (short)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0L);

                  dCurSel = min(dCurSel, dCount - 1);

                  if (dCurSel != dSel)
                     {
                     SendMessage(hWnd, LB_SETCURSEL, dCurSel, 0L);

                     hWndTemp = GetProp(hWnd, _T("HWNDCOMBO"));
                     FORWARD_WM_COMMAND(hWndTemp, GetWindowID(hWnd), hWnd,
                                        LBN_SELCHANGE, SendMessage);
                     }
                  }
               }
            }
         }

      break;
   
   case WM_CHAR:
      dCurSel = (short)SendMessage(hWnd, LB_GETCURSEL, 0, 0L);
      if (dCurSel == LB_ERR)
         {
         TCHAR szFind[2];
         szFind[0] = (TCHAR)wParam;
         szFind[1] = '\0';
         SendMessage(hWnd, LB_SELECTSTRING, (WPARAM)-1,
                     (LPARAM)(LPTSTR)szFind);
         return 0;
         }
      break;

   case WM_KEYDOWN:
      Shift = 0;

      SS_ShowEditField(lpSS, hWndSS, hWnd);

      if (HIBYTE(GetKeyState(VK_SHIFT)))
         Shift |= 0x01;

      if (HIBYTE(GetKeyState(VK_CONTROL)))
         Shift |= 0x02;

#ifdef SS_GP
      if (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'z' || wParam == 'Z'))
         wParam = VK_ESCAPE;
#endif

      switch (wParam)
         {
         case VK_ESCAPE:
            if (SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN, GetWindowID(hWndSS),
                                      MAKELONG(wParam, Shift)))
               return (0);

            lpSS->EscapeInProgress = TRUE;
            SS_PostSetEditMode(lpSS, FALSE, 0L);
            return (TRUE);

         case VK_RETURN:
            if (SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN, GetWindowID(hWndSS),
                                      MAKELONG(wParam, Shift)))
               return (0);

            PostMessage(hWndSS, SSM_PROCESSENTERKEY, 0, 0L);
            return (TRUE);

         case VK_F1:
            SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
            return (0);

         case VK_RIGHT:
         case VK_LEFT:
         case VK_UP:
         case VK_DOWN:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               {
               SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
               return (0);
               }

/* RFW - 6/30/99 - KeyDown being fired twice
            if (SS_SendMsg(hWndSS, SSM_KEYDOWN, GetWindowID(hWndSS),
                           MAKELONG(wParam, Shift)))
               return (0);
*/

            break;

         case VK_TAB:
            return (SS_EditProcessTab(hWndSS, Msg, wParam, lParam,
                    Shift));

         }

		{
      short dCurSel = (short)SendMessage(hWnd, LB_GETCURSEL, 0, 0L);

      lRet = CallWindowProc((WNDPROC)lpfnListBoxProc, hWnd, Msg, wParam, lParam);

		// RFW - 3/6/04 - 13809
      if (dCurSel != (short)SendMessage(hWnd, LB_GETCURSEL, 0, 0L))
			{
	      hWndTemp = GetProp(hWnd, _T("HWNDCOMBO"));
	      FORWARD_WM_COMMAND(hWndTemp, GetWindowID(hWnd), hWnd, LBN_SELCHANGE,
	                         SendMessage);
			}
		}

      return (lRet);
   }

lRet = SS_DefWindowProc(lpfnListBoxProc, lpSS, hWndSS, hWnd, Msg, wParam, lParam);

if (Msg == WM_LBUTTONUP)
   lRet = SS_DefWindowProc(lpfnListBoxProc, lpSS, hWndSS, hWnd,
                           WM_KEYDOWN, VK_RETURN, 0L);

return (lRet);
}


WINENTRY SS_DeskTopProc(hWnd, Msg, wParam, lParam)

HWND            hWnd;
UINT            Msg;
WPARAM          wParam;
LPARAM          lParam;
{
LPSPREADSHEET   lpSS;
SS_COLORTBLITEM ForeColorTblItem;
SS_COLORTBLITEM BackColorTblItem;
HWND            hWndSS;

switch (Msg)
   {
#ifdef WIN32
   case WM_CTLCOLORLISTBOX:
#else
   case WM_CTLCOLOR:
#endif
      if (hWndSS = GetProp(hWnd, _T("HWNDSPREAD")))
         {
         lpSS = SS_SheetLockActive(hWndSS);

         SS_GetColorTblItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt,
                            &BackColorTblItem, &ForeColorTblItem);

         SS_SheetUnlockActive(hWndSS);

         SetTextColor((HDC)wParam, SS_TranslateColor(ForeColorTblItem.Color));

         SetBkColor((HDC)wParam, SS_TranslateColor(BackColorTblItem.Color));
         return ((LRESULT)BackColorTblItem.hBrush);
         }

      break;

   }

return (CallWindowProc((WNDPROC)lpfnDeskTopProc, hWnd, Msg, wParam, lParam));
}

#endif


LRESULT SS_DefWindowProc(FARPROC lpfnProc, LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd,
                      UINT Msg, WPARAM wParam, LPARAM lParam)
{
SS_CELLCOORD CellUL;
SS_CELLCOORD CellLR;
WORD         wShift;
BOOL         fCtrlState;
long         lRet = 0;
long         lRetTemp = 0;

switch (Msg)
   {
   case WM_GETDLGCODE:
      {
      long lExtra = 0;

      if (lpSS->lpBook->fEditModePermanent || lpSS->lpBook->fProcessTab)
         lExtra = DLGC_WANTTAB;

      return (CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam) |
              DLGC_WANTALLKEYS | lExtra);
      }

   case WM_PASTE:
      CellUL.Col = lpSS->Col.EditAt;
      CellUL.Row = lpSS->Row.EditAt;
      CellLR.Col = lpSS->Col.EditAt;
      CellLR.Row = lpSS->Row.EditAt;

      if (!SS_ValidatePasteData(hWndSS, lpSS, NULL, &CellUL, &CellLR,
                                TRUE))
         return (0);
//SS_LogMsg("Test 3"); // Shannon

      break;

   case WM_SYSKEYDOWN:
      SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);
      break;

   case WM_KEYDOWN:
      wShift = 0;
//SS_LogMsg("Test 4"); // Shannon

      lpSS->lpBook->fIsActionKey = FALSE;

      SS_ShowEditField(lpSS, hWndSS, hWnd);

      if (HIBYTE(GetKeyState(VK_SHIFT)))
         wShift |= 0x01;

      if (HIBYTE(GetKeyState(VK_CONTROL)))
         wShift |= 0x02;

      if (lParam & 0x20000000L)        // Alt
         wShift |= 0x04;

#ifdef SS_GP
      if (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'z' || wParam == 'Z'))
         wParam = VK_ESCAPE;
#endif

      if ((wParam == VK_INSERT && HIBYTE(GetKeyState(VK_SHIFT))) ||
          (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'v' ||
          wParam == 'V')))
         {
         CellUL.Col = lpSS->Col.EditAt;
         CellUL.Row = lpSS->Row.EditAt;
         CellLR.Col = lpSS->Col.EditAt;
         CellLR.Row = lpSS->Row.EditAt;

         if (!SS_ValidatePasteData(hWndSS, lpSS, NULL, &CellUL,
                                   &CellLR, TRUE))
            return (0);
         }

      /*********************************************************
      * Check to see if the clipboard processing is turned off
      *********************************************************/

      if (!lpSS->lpBook->fAutoClipboard)
         {
         if ((wParam == VK_INSERT && (HIBYTE(GetKeyState(VK_SHIFT)) ||
              HIBYTE(GetKeyState(VK_CONTROL)))) || (wParam == VK_DELETE &&
              HIBYTE(GetKeyState(VK_SHIFT))) ||
              (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'v' ||
              wParam == 'V' || wParam == 'c' || wParam == 'C' ||
              wParam == 'x' || wParam == 'X')))
            {
            SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);
            return (0);
            }
         }

	  //Add By BOC 99.4.22(hyt)-------------------------------------
	  //for Checkbox type and Button type cell not implement clear action
	  {
	  SS_CELLTYPE cellType;
	  BOOL bProcessClr = TRUE;
	  if (SS_RetrieveCellType(lpSS, &cellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt))
	  {
		  if( (SS_TYPE_CHECKBOX == cellType.Type || SS_TYPE_BUTTON == cellType.Type) && cellType.ControlID )
			  bProcessClr = FALSE;
	  }

      //if (SS_IsActionKey(lpSS->lpBook, SS_KBA_CLEAR, (WORD)wParam))
      // RFW - 7/5/00 - GIC12161
      // if (bProcessClr && SS_IsActionKey(lpSS->lpBook, SS_KBA_CLEAR, (WORD)toupper(wParam)))
      if (bProcessClr && SS_IsActionKey(lpSS->lpBook, SS_KBA_CLEAR, (WORD)wParam))
	  //----------------------------------------------------------------------
         {
         WPARAM wMsgLastPrev = lpSS->lpBook->wMsgLast;

         SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);
         #ifdef SS_VB
         if( WM_KEYDOWN == lpSS->wMsgLast )
         #endif
            {
            SetWindowText(hWnd, _T(""));
            SendMessage(hWnd, EM_SETMODIFY, TRUE, 0L);
            }

         lpSS->lpBook->wMsgLast = wMsgLastPrev;
         lpSS->lpBook->fIsActionKey = TRUE;
         return (0);
         }
	  }

      switch (wParam)
         {
         case VK_ESCAPE:
            SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

            if (!IsWindow(hWndSS))
               return (0);

            lpSS->EscapeInProgress = TRUE;
            SS_PostSetEditMode(lpSS, FALSE, 0L);
            return (TRUE);

         case VK_RETURN:
            SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

//            if (SS_SendMsg(hWndSS, SSM_KEYDOWN, GetWindowID(hWndSS),
//                           MAKELONG(wParam, wShift)))
//               return (0);

            PostMessage(hWndSS, SSM_PROCESSENTERKEY, 0, 0L);
            return (TRUE);

#ifdef SS_UTP
         case VK_INSERT:
            SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

            if (!HIBYTE(GetKeyState(VK_SHIFT)) &&
                !HIBYTE(GetKeyState(VK_CONTROL)))
               {
               SS_SendMsgCommand(hWndSS, NULL, SSN_EDITMODEINS, FALSE);
               return (0);
               }

            break;
#endif

         case VK_UP:
         case VK_DOWN:
#ifdef SS_UTP
            break;
#else
            SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
            return (0);
#endif

         case VK_PRIOR:
         case VK_NEXT:
         case VK_F1:
            SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
            return (0);

         case VK_LEFT:
         case VK_RIGHT:
            fCtrlState = HIBYTE(GetKeyState(VK_CONTROL));

            if ((lpSS->lpBook->fArrowsExitEditMode && !fCtrlState &&
                 !HIBYTE(GetKeyState(VK_SHIFT))) ||
                (!lpSS->lpBook->fArrowsExitEditMode && fCtrlState))
               {
               SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
               return (0);
               }

            SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

//            if (SS_SendMsg(hWndSS, SSM_KEYDOWN, GetWindowID(hWndSS),
//                           MAKELONG(wParam, wShift)))
//               return (0);

            break;

         case VK_HOME:
         case VK_END:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               {
               SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
               return (0);
               }

            SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);
            break;

         case VK_TAB:
            return (SS_EditProcessTab(hWndSS, Msg, wParam, lParam,
                    wShift));

			// Process the popup menu shortcut key
			case VK_APPS:
				{
	         SS_CELLTYPE cellType;
				int x, y, cx, cy;

		      SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL,
				                lpSS->Col.EditAt, lpSS->Row.EditAt, &x, &y, &cx, &cy);
	         SS_RetrieveCellType(lpSS, &cellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);
				// Show popup menu
				if (lpfnProc != lpfnEditHScrollProc && SS_TYPE_BUTTON != cellType.Type)
					SS_EditPopupMenu(hWndSS, hWnd, (WORD)(x + (cx / 2)), (WORD)(y + (cy / 2)));
				}

				return (0);


         default:
            SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);
            break;
         }
//SS_LogMsg("Test 5"); // Shannon

      break;

   case WM_KEYUP:
      SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

//      wShift = 0;
//
//      if (HIBYTE(GetKeyState(VK_SHIFT)))
//         wShift |= 0x01;
//
//      if (HIBYTE(GetKeyState(VK_CONTROL)))
//         wShift |= 0x02;
//
//      if (lParam & 0x20000000L)        // Alt
//         wShift |= 0x04;
//
//      if (SS_SendMsg(hWndSS, SSM_KEYUP, GetWindowID(hWndSS),
//                     MAKELONG(wParam, wShift)))
//         return (0);

      break;

   case WM_CHAR:
      lRet = -1;

//#ifdef SS_VB
//      SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);
//#endif
	  {
//Modify by BOC 99.6.25 (hyt)----------------------------
//for BUG:01341 With KeyPress event,  if you replace the 
//value of KeyAscii parameter so SBCS becomes DBCS, the 
//characters either do not display or become garbled.
//#ifdef SPREAD_JPN
		  WPARAM wParamSave = wParam;
//#endif
  		// BUG SPR_EVN_001_003 (1-1)
		// There Are Three Windows Associated With A ComboBox Cell, A Combo, An Edit And A List.
		// When User Press A Key On Combo, The Combo Receive A WM_CHAR Message First.
		// It Will Send A SSM_KEYPRESS Message To Fire A KeyPress Event.
		// After That, The Combo Pass The WM_CHAR Message To The Edit.
		// If The ComboBox Cell Is Not Editable, It Pass The Message To List.
		// The Edit Or List Will Also Send SSM_KEYPRESS Message Here.
		// This Will Cause KeyPress Event Be Fired Twice.
		// When The ComboBox Cell Is In Editing (With Cursor), 
		// The WM_CHAR Message Will Be Sent To Edit Directly,
		// But Not Send To Combo. In This Case, KeyPress Event Only Be Fired Once.
		// If The ComboBox Cell In Uneditable And The List Is Empty,
		// The List Will Not Send SSM_KEYPRESS Message,
		// At This Time, The KeyPress Event Only Be Fired Once Too.
		// To Avoid KeyPress Event Be Fired Twice, Following Code Is Added
		// It Will Judge Whether The SSM_KEYPRESS Message Need Be Sent Or Not
		// Modified By HaHa 1999.10.29

		// Definition Of The Two Fucntions To Use
		// HWND SSCB_GetEdithWnd(HWND hWndCombo);
		// HWND SSCB_GetListhWnd(HWND hWndCombo);
		
		// Whether Send SSM_KEYPRESS Message
		// BOOL SendMsg=TRUE;
		
		// Current Window Is A Combo With Edit Or List
		// BUG SPR_EVN_001_003 Refix (1-1)
		// When Message Not Send To Combo (To Edit, List Or Other Cell),
		// It May Get Some Invalid Window Handle By Using The Two Functions.
		// Use IsWindow Funtion To Judge Whether Window Is Valid.
		// Modified By HaHa 1999.12.17
		//if (IsWindow(SSCB_GetEdithWnd(hWnd)) || IsWindow(SSCB_GetListhWnd(hWnd)))
		//if (SSCB_GetEdithWnd(hWnd) || SSCB_GetListhWnd(hWnd))
		//{
			// Get Type Of The Cell
			//SS_CELLTYPE CellType;
			//SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);
			//if ((CellType.Style & 0x03) == SS_CB_DROPDOWN) // SS_CB_ISDROPDOWN(CellType.Style)
				// It Is Combo With Edit, The Message Will Be Disposed By Edit, So Discard Here
				//SendMsg=FALSE;
			//else if ((short)SendMessage(SSCB_GetListhWnd(hWnd), LB_GETCURSEL, 0, 0L) == LB_ERR)
				// It Is Combo With List, The List Is Empty And Will Not Disposed The Message, Do Here
				//SendMsg=TRUE;
			//else
				// It Is Combo With List, The Message Will Be Disposed By List, So Discard Here
				//SendMsg=FALSE;
		//}

		// Old Behavior To Send SSM_KEYPRESS Message
		if (!lpSS->lpBook->EditModeTurningOn && SS_SendMsg(lpSS->lpBook, lpSS, SSM_KEYPRESS,
		    GetWindowID(hWndSS), (LPARAM)(LPVOID)&wParam) ||
            wParam == 0)
            lRet = 0;

		// Send Message SSM_KEYPRESS Message If Need
		//if (!lpSS->lpBook->EditModeTurningOn &&
		//	SendMsg &&
		//	SS_SendMsg(hWndSS, SSM_KEYPRESS, GetWindowID(hWndSS), (long)(LPVOID)&wParam) ||
		//	wParam == 0)
		//	lRet = 0;

//#ifdef SPREAD_JPN
#ifndef _UNICODE
		if(wParam>0xff && wParam != wParamSave)
		{
			  USHORT nChar1,nChar2;
			  nChar1 = (USHORT)(wParam >> 8);
			  nChar2 = (USHORT)(wParam & 0x00ff);
			  PostMessage(hWnd,WM_CHAR,nChar1,0);
			  PostMessage(hWnd,WM_CHAR,nChar2,0);
			  lRet = 0;
		}
#endif
//#endif
	  }

      //Add By BOC 99.4.22(hyt)-------------------------------------
      //for Checkbox type and Button type cell not implement clear action

      if (lpSS->lpBook->fIsActionKey)
         {
         SS_CELLTYPE cellType;
         BOOL bProcessClr = TRUE;
         if (SS_RetrieveCellType(lpSS, &cellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt))
            {
            if( (SS_TYPE_CHECKBOX == cellType.Type || SS_TYPE_BUTTON == cellType.Type) && cellType.ControlID )
               bProcessClr = FALSE;
            }

         if (bProcessClr)
            lRet = 0;

	      lpSS->lpBook->fIsActionKey = FALSE; // RFW - 2/9/06 - 18127
         }

      if (wParam == '\r' || wParam == '\t')
         lRet = 1;

      else if (wParam == 27)           // Do not send escape key to control
         lRet = 0;

      else
         {
         SS_ShowEditField(lpSS, hWndSS, hWnd);

#ifdef SS_UTP
         if (lpSS->fProhibitTypingWhenSel)
				if (SS_IsBlockSelected(lpSS) &&
					 !(wParam == 3 || wParam == 22 || wParam == 24))
					if (!SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_TYPINGPROHIBITED, FALSE))
						lRet = 0;
#endif // SS_UTP
         }

      if (lRet != -1)
         return (lRet);

      break;

   case WM_SETFOCUS:
      CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam);

      if (!lpSS->lpBook->EditModeOn && !lpSS->lpBook->EditModeTurningOn)
         SetFocus(hWndSS);

      // BJO 19Nov96 SEL6570 - Begin fix
      //else if(lpSS->lpBook->EditModeOn && !lpSS->lpBook->EditModeTurningOff &&
      //        lpSS->fKillFocusReceived)
      //   SetFocus(hWndSS);
      // BJO 19Nov96 SEL6570 - End fix

      return (0);


   case WM_KILLFOCUS:
      // fix for 9570 -scl
      if( wParam && !GetParent((HWND)wParam) )
			{
			HWND hwnd = hWnd;
			do
				{
				hwnd = GetParent(hwnd);
				if( hwnd == (HWND)wParam )
					return 0;
				} while(hwnd);
			}

		{
		LPSS_BOOK lpBook = lpSS->lpBook;
      CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam);

      lpBook->hWndFocusPrev = hWnd;

      if (!lpBook->EditModeTurningOn && !lpBook->EditModeTurningOff &&
          !lpBook->wMessageBeingSent)
         SendMessage(hWndSS, Msg, wParam, lParam);
		}
      return (0);

   case WM_RBUTTONUP:
   case WM_RBUTTONDOWN:
   case WM_RBUTTONDBLCLK:
      {
      SS_RBUTTON RButton;
      POINT      point;
      int        x;
      int        y;

      if (Msg == WM_RBUTTONUP)
         RButton.RButtonType = RBUTTONTYPE_UP;
      else if (Msg == WM_RBUTTONDOWN)
         RButton.RButtonType = RBUTTONTYPE_DOWN;
      else if (Msg == WM_RBUTTONDBLCLK)
         RButton.RButtonType = RBUTTONTYPE_DBLCLK;

      point.x = LOWORD(lParam);
      point.y = HIWORD(lParam);
      ClientToScreen(hWnd, &point);
      ScreenToClient(hWndSS, &point);
      RButton.xPos = (short)point.x;
      RButton.yPos = (short)point.y;

      SS_GetCellFromPixel(lpSS, &RButton.Col, &RButton.Row, &x, &y,
                          RButton.xPos, RButton.yPos);

      if (RButton.Col != -2 && RButton.Row != -2)
         {
         SS_CELLTYPE cellType;

			SS_AdjustCellCoordsOut(lpSS, &RButton.Col, &RButton.Row);
         SS_SendMsg(lpSS->lpBook, lpSS, SSM_RBUTTON, GetWindowID(hWndSS),
                    (LPARAM)(LPSS_RBUTTON)&RButton);

			// RFW - 2/2/05 - 15670
			if (GetFocus() == hWnd)
				{
				SS_RetrieveCellType(lpSS, &cellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);

				// Show popup menu
				SS_AdjustCellCoords(lpSS, &RButton.Col, &RButton.Row);
				/* RFW - 2/2/05 - 15670
				if (RButton.Col == lpSS->Col.CurAt && RButton.Row == lpSS->Row.CurAt &&
					 lpfnProc != lpfnEditHScrollProc && SS_TYPE_BUTTON != cellType.Type)
				*/
				/* RFW - 10/18/05 - 17263
				if (RButton.Col == lpSS->Col.CurAt && RButton.Row == lpSS->Row.CurAt &&
					 lpfnProc != lpfnEditHScrollProc && SS_TYPE_BUTTON != cellType.Type &&
                Msg == WM_RBUTTONUP)
				*/
				if (RButton.Col == lpSS->Col.CurAt && RButton.Row == lpSS->Row.CurAt &&
					 lpfnProc != lpfnEditHScrollProc && SS_TYPE_BUTTON != cellType.Type &&
                SS_TYPE_CHECKBOX != cellType.Type && SS_TYPE_COMBOBOX != cellType.Type &&
                Msg == WM_RBUTTONUP)
					SS_EditPopupMenu(hWndSS, hWnd, RButton.xPos, RButton.yPos);
				}
         }
      }

      if (GetFocus() != hWnd)
         return (0);

      break;

   case WM_MOUSEMOVE:
#ifdef SS_VB
      {
      POINT point;
      point.x = LOWORD(lParam);
      point.y = HIWORD(lParam);
      ClientToScreen(hWnd, &point);
      ScreenToClient(hWndSS, &point);
      SS_SendMessageToVB(hWndSS, Msg, wParam,
                         MAKELPARAM(point.x,point.y));
      }
#endif
      break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
			{
			case IDM_CUT:
				SendMessage(hWnd, WM_CUT, 0, 0);
				return (0);
			case IDM_COPY:
				SendMessage(hWnd, WM_COPY, 0, 0);
				return (0);
			case IDM_PASTE:
				SendMessage(hWnd, WM_PASTE, 0, 0);
				return (0);
			case IDM_DELETE:
				SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, 0);
				return (0);
			case IDM_SELECT:
				Edit_SetSel(hWnd, 0, -1);
				return (0);
			}

		break;
   }

if (!IsWindow(hWnd))
   return (0);

return (CallWindowProc((WNDPROC)lpfnProc, hWnd, Msg, wParam, lParam));
}


LRESULT SS_EditProcessTab(HWND hWndSS, UINT Msg, WPARAM wParam,
                          LPARAM lParam, WORD wShift)
{
BOOL fProcessTab = SSGetBool(hWndSS, SSB_PROCESSTAB);

if (fProcessTab)
   {
   SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

//               if (SS_SendMsg(hWndSS, SSM_KEYDOWN,
//                              GetWindowID(hWndSS),
//                              MAKELONG(wParam, wShift)))
//                  return (0);

   PostMessage(hWndSS, SSM_PROCESSTAB, wParam, lParam);
   }

else
   {
   SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

//               if (SS_SendMsg(hWndSS, SSM_KEYDOWN,
//                              GetWindowID(hWndSS),
//                              MAKELONG(wParam, wShift)))
//                  return (0);

   if (HIBYTE(GetKeyState(VK_SHIFT)))
      PostMessage(GetParent(hWndSS), WM_NEXTDLGCTL, 1, 0L);
   else
      PostMessage(GetParent(hWndSS), WM_NEXTDLGCTL, 0, 0L);
   }

return (TRUE);
}


void SS_ShowEditField(LPSPREADSHEET lpSS, HWND hWndSS, HWND hWnd)
{
if (lpSS->lpBook->fEditModePermanent && !lpSS->fEditModeDataChange)
   {
   SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
   lpSS->fEditModeDataChange = TRUE;
   }
}


void SS_ProcessEnterKey(LPSPREADSHEET lpSS)
{
LPSS_BOOK lpBook = lpSS->lpBook;
HWND      hWndSS = lpBook->hWnd;

if (HIBYTE(GetKeyState(VK_SHIFT)))
   return;

#ifndef SS_UTP
//#ifdef SPREAD_JPN
//- JPNFIX0012 - (Masanori Iwasa)
if(!lpBook->fEditModePermanent)
//#endif
    SendMessage(hWndSS, SSM_SETEDITMODE, FALSE, 0L);

// BJO 05Feb97 JOK1592 - Begin fix
else
   {
   HWND hwndCombo;
   SS_CELLTYPE cellType;
   if (SS_RetrieveCellType(lpSS, &cellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt))
      {
      if( SS_TYPE_COMBOBOX == cellType.Type && cellType.ControlID )
         if( hwndCombo = SS_GetControlhWnd(lpSS, cellType.ControlID) )
			{
            ComboBox_ShowDropdown(hwndCombo, FALSE);
			}
      }
   }
// BJO 05Feb97 JOK1592 - Edn fix

// BJO 12Sep96 TEL3550 - Begin fix - remove SourceSafe version 19 change
//else
//{
//   LPSS_CELL      lpCell;
//   LPSS_CELLTYPE  CellType;
//   SS_CELLTYPE    CellTypeTemp;
//   
//   lpCell = SS_LockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);
//   CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell,
//                                  lpSS->Col.EditAt, lpSS->Row.EditAt);
//   if (CellType->Type == SS_TYPE_COMBOBOX)
//      SetFocus(hWndSS);
////         InvalidateRect(hWndCtrl, NULL, FALSE);
////         UpdateWindow(hWndCtrl);
////         ShowWindow(hWndCtrl, SW_HIDE);
//
////         SS_UpdateWindow(lpBook);
//   if (lpCell)
//      SS_UnlockCellItem(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);
//
//}
//BJO 12Sep96 TEL3550 - End fix
lpSS = SS_BookLockActiveSheet(lpBook);
#endif

switch (lpBook->wEnterAction)
   {
   case SS_ENTERACTION_UP:
      SS_KeyDown(lpSS, VK_UP, 0L, TRUE);
//      SendMessage(hWndSS, WM_KEYDOWN, VK_UP, 0L);
      break;

   case SS_ENTERACTION_DOWN:
      SS_KeyDown(lpSS, VK_DOWN, 0L, TRUE);
//      SendMessage(hWndSS, WM_KEYDOWN, VK_DOWN, 0L);
      break;

   case SS_ENTERACTION_LEFT:
      SS_KeyDown(lpSS, VK_LEFT, 0L, TRUE);
//      SendMessage(hWndSS, WM_KEYDOWN, VK_LEFT, 0L);
      break;

   case SS_ENTERACTION_RIGHT:
      SS_KeyDown(lpSS, VK_RIGHT, 0L, TRUE);
//      SendMessage(hWndSS, WM_KEYDOWN, VK_RIGHT, 0L);
      break;

   case SS_ENTERACTION_NEXT:
      SendMessage(hWndSS, SSM_NEXTCELL, 0, 0L);
      break;

   case SS_ENTERACTION_PREV:
      SendMessage(hWndSS, SSM_PREVCELL, 0, 0L);
      break;

   case SS_ENTERACTION_NEXTROW:
      SendMessage(hWndSS, SSM_NEXTROW, 0, 0L);
      break;
   }
}


LRESULT SS_SendMessageToVB(HWND hWndSS, UINT Msg, WPARAM wParam,
                           LPARAM lParam)
{
LPSPREADSHEET lpSS = SS_SheetLockActive(hWndSS);
long          lRet = 0;

#ifdef SS_VB
if (!lpSS->lpBook->EditModeTurningOn)
   {
   lpSS->fMsgSentToVB = TRUE;
   lRet = SendMessage(hWndSS, Msg, wParam, lParam);
   if (!IsWindow(hWndSS))
      return (0);

   lpSS->fMsgSentToVB = FALSE;
   }

#else
WORD wShift = 0;

// RFW - 5/4/99 - SPR_EVN_001_004
// I added this line to fix a bug in which multiple KeyDown events were
// being fired.  I'm a little concerned it may break something, but
// I think we're OK.
if (!lpSS->lpBook->EditModeTurningOn)
   {
   if (HIBYTE(GetKeyState(VK_SHIFT)))
      wShift |= 0x01;

   if (HIBYTE(GetKeyState(VK_CONTROL)))
      wShift |= 0x02;

   if (lParam & 0x20000000L)        // Alt
      wShift |= 0x04;

   // BJO 07Jul97 JOK2946 - Before fix
   //if (SS_SendMsg(hWndSS, Msg == WM_KEYDOWN ? SSM_KEYDOWN : SSM_KEYUP,
   //               GetWindowID(hWndSS), MAKELONG(wParam, wShift)))
   //   lRet = 0;
   // BJO 07Jul97 JOK2946 - Begin fix
   if (SSx_SendMsg(lpSS->lpBook, lpSS, Msg == WM_KEYDOWN ? SSM_KEYDOWN : SSM_KEYUP,
                   GetWindowID(hWndSS), MAKELONG(wParam, wShift)))
      lRet = 0;
   // BJO 07Jul97 JOK2946 - End fix
   }

#endif

SS_SheetUnlockActive(hWndSS);
return (lRet);
}


BOOL SS_EditIsClipboardEmpty(HWND hWnd)
{
HANDLE hMem;
BOOL   fIsEmpty = TRUE;

OpenClipboard(hWnd);

#if defined(UNICODE) || defined(_UNICODE)
if (hMem = GetClipboardData(CF_UNICODETEXT))
#else
if (hMem = GetClipboardData(CF_TEXT))
#endif
   {
	LPTSTR lpszMem = (LPTSTR)GlobalLock(hMem);
	fIsEmpty = (*lpszMem == 0);
   GlobalUnlock(hMem);
   }

CloseClipboard();
return (fIsEmpty);
}


void SS_EditPopupMenu(HWND hWndSS, HWND hWnd, WORD xPos, WORD yPos)
{
POINT ptCurrent;
HMENU hmenuPopUp;
BOOL  fStatic = (GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC);
BOOL  fIsPasteValid = !SS_EditIsClipboardEmpty(hWnd);
long  lSel = (long)SendMessage(hWnd, EM_GETSEL, 0, 0);
BOOL  fHighlighted = (HIWORD(lSel) > LOWORD(lSel));
long  lTextLen = (long)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);

ptCurrent.x = xPos;
ptCurrent.y = yPos;

hmenuPopUp = CreatePopupMenu();

#ifdef SPREAD_JPN
AppendMenu(hmenuPopUp, fHighlighted && !fStatic ? MF_ENABLED : MF_GRAYED, IDM_CUT, _T("切り取り(&T)"));
AppendMenu(hmenuPopUp, fHighlighted ? MF_ENABLED : MF_GRAYED, IDM_COPY, _T("コピー(&C)"));
AppendMenu(hmenuPopUp, !fStatic && fIsPasteValid ? MF_ENABLED : MF_GRAYED, IDM_PASTE, _T("貼り付け(&P)"));
AppendMenu(hmenuPopUp, fHighlighted && !fStatic ? MF_ENABLED : MF_GRAYED, IDM_DELETE, _T("削除(&D)"));

AppendMenu(hmenuPopUp, MF_SEPARATOR, 0, _T(""));
AppendMenu(hmenuPopUp, lTextLen > 0 ? MF_ENABLED : MF_GRAYED, IDM_SELECT, _T("すべて選択(&A)")); 
#else
AppendMenu(hmenuPopUp, fHighlighted && !fStatic ? MF_ENABLED : MF_GRAYED, IDM_CUT, _T("Cu&t"));
AppendMenu(hmenuPopUp, fHighlighted ? MF_ENABLED : MF_GRAYED, IDM_COPY, _T("&Copy"));
AppendMenu(hmenuPopUp, !fStatic && fIsPasteValid ? MF_ENABLED : MF_GRAYED, IDM_PASTE, _T("&Paste"));
AppendMenu(hmenuPopUp, fHighlighted && !fStatic ? MF_ENABLED : MF_GRAYED, IDM_DELETE, _T("&Delete"));

AppendMenu(hmenuPopUp, MF_SEPARATOR, 0, _T(""));
AppendMenu(hmenuPopUp, lTextLen > 0 ? MF_ENABLED : MF_GRAYED, IDM_SELECT, _T("Select &All")); 
#endif // SPREAD_JPN

ClientToScreen(hWndSS, &ptCurrent);
TrackPopupMenu(hmenuPopUp, TPM_LEFTALIGN | TPM_TOPALIGN, ptCurrent.x, ptCurrent.y, 0, hWnd, NULL);
}

#if SS_V80
#define HWNDSPREAD _T("HWNDSPREAD")

WINENTRY SS_CustomProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
  HWND hSpread = (HWND)GetProp(hWnd, HWNDSPREAD);
  LPSPREADSHEET lpSS = SS_SheetLock(hSpread);
  FARPROC lpfnCustomProc = (FARPROC)SS_GetControlWndProc(lpSS, hWnd);
  switch( Msg )
  {
  case WM_DESTROY:
     {
#if SS_OCX
        long lpdisp = (long)GetProp(hWnd, _T("PDISPATCH"));
        if( lpdisp )
        {
           fpDispatchRelease(lpdisp);
           SetProp(hWnd, _T("PDISPATCH"), NULL);
        }
#endif
        if( lpSS )
          //SS_CellEditModeOff(lpSS, (HWND)-1);
          SS_CT_UnsubclassEditor(lpSS, hWnd);
     }
     break;
  case WM_KEYDOWN:
     {
        WORD dlgCode = (WORD)CallWindowProc((WNDPROC)lpfnCustomProc, hWnd, WM_GETDLGCODE, 0, 0);
        HWND hWndSS = lpSS->lpBook->hWnd;
        WORD fCtrlState;
        WORD wShift;

        switch( wParam )
        {
         case VK_UP:
         case VK_DOWN:
            if (dlgCode & DLGC_WANTARROWS || dlgCode & DLGC_WANTALLKEYS)
               {
               fCtrlState = HIBYTE(GetKeyState(VK_CONTROL));

               wShift = 0;

               if (HIBYTE(GetKeyState(VK_SHIFT)))
                  wShift |= 0x01;

               if (HIBYTE(GetKeyState(VK_CONTROL)))
                  wShift |= 0x02;

               if ((lpSS->lpBook->fArrowsExitEditMode && !fCtrlState &&
                    !HIBYTE(GetKeyState(VK_SHIFT))) ||
                   (!lpSS->lpBook->fArrowsExitEditMode && fCtrlState))
                  {
                  SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
                  return (0);
                  }

               if (SS_SendMsg(lpSS->lpBook, lpSS, SSM_KEYDOWN,
                              GetWindowID(hWndSS),
                              MAKELONG(wParam, wShift)))
                  return (0);

               return (CallWindowProc((WNDPROC)lpfnCustomProc, hWnd, Msg, wParam, lParam));
               }

            break;

         case VK_LEFT:
         case VK_RIGHT:
            if (dlgCode & DLGC_WANTARROWS || dlgCode & DLGC_WANTALLKEYS)
               {
               fCtrlState = HIBYTE(GetKeyState(VK_CONTROL));

               if ((lpSS->lpBook->fArrowsExitEditMode && !fCtrlState &&
                    !HIBYTE(GetKeyState(VK_SHIFT))) ||
                   (!lpSS->lpBook->fArrowsExitEditMode && fCtrlState))
                  {
                  SendMessage(hWndSS, WM_KEYDOWN, wParam, 0L);
                  return (0);
                  }

               else if (lpSS->lpBook->fArrowsExitEditMode && fCtrlState)
                  {
                  BYTE pbKeyState[256];
                  BYTE bControlState;

                  GetKeyboardState(pbKeyState);
                  bControlState = pbKeyState[VK_CONTROL];
                  pbKeyState[VK_CONTROL] = 0;
                  SetKeyboardState(pbKeyState);
                  CallWindowProc((WNDPROC)lpfnCustomProc, hWnd, Msg, wParam, lParam);
                  pbKeyState[VK_CONTROL] = bControlState;
                  SetKeyboardState(pbKeyState);
                  return (0);
                  }

               SS_SendMessageToVB(hWndSS, Msg, wParam, lParam);

               return (CallWindowProc((WNDPROC)lpfnCustomProc, hWnd, Msg, wParam, lParam));
               }

            break;

         case VK_RETURN:
            if (GetWindowLong(hWnd, GWL_STYLE) & ES_WANTRETURN)
               {
               if (HIBYTE(GetKeyState(VK_SHIFT)))
                  wShift |= 0x01;

               if (HIBYTE(GetKeyState(VK_CONTROL)))
                  wShift |= 0x02;

               if (SS_SendMsgActiveSheet(hWndSS, SSM_KEYDOWN,
                                         GetWindowID(hWndSS),
                                         MAKELONG(wParam, wShift)))
                  return (0);

               return (CallWindowProc((WNDPROC)lpfnCustomProc, hWnd, Msg, wParam, lParam));
               }

            break;
        }
     }
  }
  return (SS_BaseProc(lpfnCustomProc, hWnd, Msg, wParam, lParam));
}


void SS_CT_SubclassEditor(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   HWND hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);
   FARPROC wndProc;

   if( IsWindow(hWndCtrl) )
   {
      SetProp(hWndCtrl, HWNDSPREAD, lpSS->lpBook->hWnd);
#if defined(_WIN64) || defined(_IA64)
      wndProc = (FARPROC)SetWindowLongPtr(hWndCtrl, GWLP_WNDPROC, (LONG_PTR)&SS_CustomProc);
#else
      wndProc = (FARPROC)SetWindowLong(hWndCtrl, GWL_WNDPROC, (long)&SS_CustomProc);
#endif
      if( wndProc != SS_CustomProc ) // 27026 -scl
         SS_SetControlWndProc(lpSS, hWndCtrl, wndProc);
   }
}

void SS_CT_UnsubclassEditor(LPSPREADSHEET lpSS, HWND hWndCtrl)
{
   if( IsWindow(hWndCtrl) )
   {
      FARPROC wndProc = (FARPROC)SS_GetControlWndProc(lpSS, hWndCtrl);

      RemoveProp(hWndCtrl, HWNDSPREAD);
#if defined(_WIN64) || defined(_IA64)
      SetWindowLongPtr(hWndCtrl, GWLP_WNDPROC, (LONG_PTR)wndProc);
#else
      SetWindowLong(hWndCtrl, GWL_WNDPROC, (long)wndProc);
#endif
      SS_SetControlWndProc(lpSS, hWndCtrl, NULL);
      if( !lpSS->lpBook->EditModeTurningOff )
         PostMessage(lpSS->lpBook->hWnd, SSM_RESUBCLASSEDITOR, 0, 0);
   }
}


#endif
