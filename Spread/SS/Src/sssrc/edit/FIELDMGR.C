/*
$Revision:   1.13  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/FIELDMGR.C_V  $
 * 
 *    Rev 1.13   12 Jun 1990 14:08:00   Dirk
 * No change.
 * 
 *    Rev 1.12   29 May 1990  9:08:38   Randall
 * Made external objects "static".
 * 
 *    Rev 1.11   28 May 1990 13:51:10   Randall
 * Continued to split into several modules.
 * 
 *    Rev 1.10   25 May 1990 15:48:58   Randall
 * Split some functions off into OverMode.c and HiLight.c
 * 
 *    Rev 1.7   18 May 1990 17:12:44   Randall
 * Added ClearSelection function to support WM_CLEAR
 * 
 *    Rev 1.6   16 May 1990 17:34:38   Randall
 * Put global bMouseMove in EditField structure: Now one per field.
 * 
 *    Rev 1.5   15 May 1990 15:24:30   Randall
 * Added parent notification: EN_SETFOCUS, EN_KILLFOCUS, EN_UPDATE, EN_CHANGE
 * 
 *    Rev 1.4   15 May 1990 10:55:30   Randall
 * Check if Mouse button is down when field loses the focus.
 * 
 *    Rev 1.3   14 May 1990 17:40:14   Randall
 * Changed NotifyOutOfMemory to the more general NotifyParent
 * 
 *    Rev 1.2   14 May 1990 15:00:38   Randall
 * Send EN_ERRSPACE if Alloc fails.
 * 
 *    Rev 1.1   14 May 1990 11:14:22   Randall
 * Added AdjustLength to get me out of a spot in DateMgr.c>DateDynamicValidate
 * 
 *    Rev 1.0   11 May 1990 16:10:38   Bruce
 * Log file removed from the server. Started again.
 * 
 *    Rev 1.10   09 May 1990 15:10:48   Randall
 * Added concept of local and global overstrike modes.
 * 
 *    Rev 1.9   07 May 1990 12:58:04   Randall
 * Fixed up highlight deselection on ProcessCharacter.
 * 
 *    Rev 1.8   07 May 1990 10:18:08   Randall
 * Initialized bRemembered on create.
 * 
 *    Rev 1.7   04 May 1990 11:34:06   Randall
 * Now automatically selects the first WS_TABSTOP on WM_INITDIALOG.
 * 
 *    Rev 1.6   03 May 1990 17:54:26   Randall
 * WM_GETSEL and WM_SETSEL
 * 
 *    Rev 1.5   02 May 1990 19:02:10   Randall
 * Rewrote highlighting.
 * 
 *    Rev 1.4   01 May 1990 10:47:50   Randall
 * Temporarily removed bOnCreate stuff pending proper handling of WM_INITDIALOG
 * 
 *    Rev 1.2   24 Apr 1990 20:48:50   Randall
 * Improved highlighting in overstrike mode.
 * 
 *    Rev 1.0   23 Apr 1990 18:40:10   Sonny
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOMINMAX

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <toolbox.h>
#include "..\classes\wintools.h"
#include "..\libgen.h"
#include "editfld.h"

//----------------------------Internal Functions----------------------------//
extern void EditPaintField (HWND hWnd, HDC hDC, LPEDITFIELD lpField);
extern void EditTextOut(HWND hWnd, LPEDITFIELD lpField, HDC hDC, HBRUSH hBrush,
                        HFONT hFont, RECT Rect, int x, int y, LPCTSTR lpszText, short nCnt,
                        BOOL fRightJustify, short iStart, short iEnd);
static void FreeField(HWND);
short       Edit_GetRightVisCharPos(LPEDITFIELD lpField, short nLeftChar);
//--------------------------------------------------------------------------//

BOOL CreateField (HWND hWnd, LPARAM lParam)

{
   LPEDITFIELD lpField;
   HANDLE      hField;
   LONG        lStyle;
   BOOL        fRetCode = FALSE;

   if (!(hField = GlobalAlloc (GHND, (DWORD) sizeof(EDITFIELD))))
   {
      NotifyParent (hWnd, EN_ERRSPACE);
   }

   else  
   {
      lpField = (LPEDITFIELD) GlobalLock (hField);
      if (lpField)
      {
         SetWindowEditField (hWnd, hField);

         lpField->hWnd =            hWnd;
         lpField->lpszString[0] =   '\0';
         lpField->nChars =          0;
         lpField->nMaxChars =       -1;
         lpField->bModified =       0;
         lpField->iCurrentPos =     0;
         lpField->bHighlighted =    FALSE;
         lpField->iHiEnd = lpField->iHiStart = 0;
         lpField->bProcessingBackspace = FALSE;
         lpField->bProcessingDelete = FALSE;

         lpField->f3D = FALSE;
         lpField->Edit3DFormat.dShadowSize = 1;
         lpField->Edit3DFormat.ColorShadow = GetSysColor(COLOR_BTNSHADOW);
         lpField->Edit3DFormat.ColorInsideShadow = GetSysColor(COLOR_BTNSHADOW);
         lpField->Edit3DFormat.ColorHighlight = RGBCOLOR_WHITE;
         lpField->Edit3DFormat.ColorInsideShadow = GetSysColor(COLOR_BTNFACE);
         lpField->Edit3DFormat.ColorInsideHighlight = RGBCOLOR_BLACK;

         GetFontWidth(hWnd, NULL);

         lStyle = ((LPCREATESTRUCT)lParam)->style;

         if (lStyle & ES_STATIC)
            {
            lStyle &= ~WS_TABSTOP;
            ((LPCREATESTRUCT)lParam)->style = lStyle;
            SetWindowLong(hWnd, GWL_STYLE, lStyle);
            }

         if (lStyle & ES_3D)
            lpField->f3D = TRUE;

         lpField->xRightNonClientXtra = 0;
         lpField->bOverstrike = (lStyle & ES_OVERSTRIKE) != 0;
#ifdef SS_OCX
         lpField->fAutoHScroll = TRUE;
#else
         lpField->fAutoHScroll = (lStyle & ES_AUTOHSCROLL) != 0;
#endif // SS_OCX

         lpField->bHAlign = (lStyle & ES_RIGHT) ? EDIT_HALIGN_RIGHT : EDIT_HALIGN_LEFT;
			lpField->bHAlignActual = lpField->bHAlign;
         lpField->fPassWord = (BYTE)((lStyle & ES_PASSWORD) != 0);

         if ((lStyle & ES_UPPERCASE) != 0)
            lpField->nCase = UPPERCASE;
         else if ((lStyle & ES_LOWERCASE) != 0)
            lpField->nCase = LOWERCASE;
         else
            lpField->nCase = '\0';

         lpField->bMouseDown = FALSE;
         ShiftUp (hWnd);

         SetInsertMode (hWnd, GLOBAL);

         GlobalUnlock (hField);
         fRetCode = TRUE;
      }
      else
         NotifyParent (hWnd, EN_ERRSPACE);
   }

   return(fRetCode);
}


void  OpenField (HWND hWnd)

// Called in response to WM_SETFOCUS only.
{
LPEDITFIELD lpField;

NotifyParent (hWnd, EN_SETFOCUS);

if (!(lpField = LockField(hWnd)))
   return;

if (lpField->bOverstrike)           // Force the field to Overstrike
   SetOverstrikeMode (hWnd, LOCAL);
else                                // Depends on tha state of the screen
   if (GetOverstrikeMode (hWnd, GLOBAL))
      SetOverstrikeMode (hWnd, LOCAL);
   else
      SetInsertMode (hWnd, LOCAL);

if (EditGetHAlign(lpField) == EDIT_HALIGN_RIGHT)
	EditScrollEnd(lpField);
else
	EditScrollHome(lpField);

EditScrollCurrentPosIntoView(lpField);
PositionCaret (hWnd, lpField, lpField->iCurrentPos);
lpField->bHighlighted = FALSE;

UnlockField (hWnd);
}


void HighlightField(HWND hWnd, LPEDITFIELD lpField)
{
HWND hWndFocus;
HDC  hDC;

hWndFocus = GetFocus();

if (hWndFocus == hWnd)
   HideCaret(hWnd);

hDC = GetDC(hWnd);
EditPaintField (hWnd, hDC, lpField);
ReleaseDC(hWnd, hDC);

if (hWndFocus == hWnd)
   ShowCaret(hWnd);
}


void PaintField(HWND hWnd, HDC hDC)

{
LPEDITFIELD lpField;
PAINTSTRUCT ps;
HDC         hDCOrig;

hDCOrig = hDC;

lpField = LockField (hWnd);

if (!hDCOrig)
   hDC = BeginPaint (hWnd, &ps);

EditPaintField (hWnd, hDC, lpField);

if (GetFocus() == hWnd)       // This field has the focus
   {
   if (lpField->bHighlighted)
      {
      // Give the sub-class an opportunity to do any initial caret
      // positioning required, e.g., float fields need the caret at
      // the decimal point.

      SendMessage (hWnd, EM_EDIT_POSITIONAFTERPAINT, 0, 0L);
      }
   else
      PositionCaret (hWnd, lpField, lpField->iCurrentPos);
   }

if (!hDCOrig)
   EndPaint (hWnd, &ps);

UnlockField (hWnd);
}


void EditPaintField (HWND hWnd, HDC hDC, LPEDITFIELD lpField)
{
HBITMAP hBitmap;
HBITMAP hBitmapOld;
HBRUSH  hBrush;
HBRUSH  hBrushOld;
HBRUSH  hBrushNew = 0;
RECT    RectClient;
RECT    Rect;
HDC     hDCMemory;
TCHAR   szBuffer[MAXFIELD];
int     xPos;
short   i;

GetClientRect(hWnd, &RectClient);
CopyRect(&Rect, &RectClient);

#ifdef SS_V40
if (lpField->fAutoHScroll)
	{
	if (lpField->bHAlign == EDIT_HALIGN_RIGHT)
		{
		if (Edit_GetRightVisCharPos(lpField, 0) == lpField->nChars - 1)
			{
			lpField->bHAlignActual = EDIT_HALIGN_RIGHT;
			lpField->nLeftChar = 0;
			}
		// RFW - 7/25/03
		else if (lpField->iCurrentPos >= lpField->nChars - 1)
			{
			lpField->bHAlignActual = EDIT_HALIGN_RIGHT;
			lpField->nLeftChar = 0;
			}
		else if (lpField->iCurrentPos == EditGetLastValidCaretPos(lpField))
			{
			lpField->bHAlignActual = EDIT_HALIGN_RIGHT;
			lpField->nLeftChar = 0;
			}
//			lpField->bHAlignActual = EDIT_HALIGN_RIGHT;
		else
			lpField->bHAlignActual = EDIT_HALIGN_LEFT;
		}
	}
#endif // SS_V40

/*******************************************************
* Create Bitmap to select to the memory device context
*******************************************************/

hBitmap = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,
                                 Rect.bottom - Rect.top);
hDCMemory = CreateCompatibleDC(hDC);

/*********************************
* Create a memory device context
*********************************/

hBitmapOld = SelectObject(hDCMemory, hBitmap);
SetMapMode(hDCMemory, MM_TEXT);

#ifdef WIN32
hBrush = FORWARD_WM_CTLCOLOREDIT(GetParent(hWnd), hDC, hWnd, SendMessage);
#else
#ifdef TBPRO
hBrush = FORWARD_WM_CTLCOLOR(GetParent(hWnd), hDC, hWnd, CTLCOLOR_TBEDIT,
                             SendMessage);
#else
hBrush = FORWARD_WM_CTLCOLOR(GetParent(hWnd), hDC, hWnd, CTLCOLOR_EDIT,
                             SendMessage);
#endif // TBPRO
#endif // WIN32

SetBkColor(hDCMemory, GetBkColor(hDC));
SetTextColor(hDCMemory, GetTextColor(hDC));

if (!hBrush)
   {
   hBrushNew = CreateSolidBrush(GetBkColor(hDC));
   hBrush = hBrushNew;
   }

hBrushOld = SelectObject(hDCMemory, hBrush);

/************************************
* Fill the hDC background with the
* current background of the display
************************************/

FillRect(hDCMemory, &Rect, hBrush);

if (lpField->f3D)
   {
   tbDrawShadows(hDCMemory, &Rect, lpField->Edit3DFormat.dShadowSize,
                 lpField->Edit3DFormat.ColorShadow,
                 lpField->Edit3DFormat.ColorHighlight,
                 !(lpField->Edit3DFormat.wStyle & EDIT3D_OUT), FALSE);

   tbShrinkRect(&Rect, lpField->Edit3DFormat.dShadowSize,
                lpField->Edit3DFormat.dShadowSize);

   if (lpField->Edit3DFormat.wStyle & EDIT3D_INSIDEBORDER)
      {
      tbDrawShadows(hDCMemory, &Rect, 1, lpField->Edit3DFormat.ColorInsideShadow,
                    lpField->Edit3DFormat.ColorInsideHighlight, TRUE, FALSE);

      tbShrinkRect(&Rect, 1, 1);
      }
   }

EditGetEditRect(lpField, &Rect);

if (lpField->hFont)
   SelectObject(hDCMemory, lpField->hFont);

if (lpField->fPassWord)
   {
   for (i = 0; i < lstrlen(lpField->lpszString); i++)
      szBuffer[i] = '*';

   szBuffer[lstrlen(lpField->lpszString)] = '\0';
   }

else
   lstrcpy(szBuffer, lpField->lpszString);

xPos = EditGetStartXPos(lpField);
EditTextOut(hWnd, lpField, hDCMemory, hBrush, lpField->hFont, Rect, xPos,
            Rect.top, szBuffer, (short)lstrlen(szBuffer),
            EditGetHAlign(lpField) == EDIT_HALIGN_RIGHT, -1, -1);

/*******************************************************
* Copy the memory device context bitmap to the display
*******************************************************/

// The following code will Exclude the Spin button if there is one.
{
HWND hWndChild = GetWindow(hWnd, GW_CHILD);

if (hWndChild)
   {
   RECT RectParent;
   RECT RectChild;

   GetWindowRect(hWnd, &RectParent);
   GetWindowRect(hWndChild, &RectChild);

   ExcludeClipRect(hDC,
                   RectChild.left - RectParent.left,
                   RectChild.top - RectParent.top,
                   RectChild.right - RectParent.left,
                   RectChild.bottom - RectParent.top);
   }
}

BitBlt(hDC, RectClient.left, RectClient.top, RectClient.right - RectClient.left,
       RectClient.bottom - RectClient.top, hDCMemory, RectClient.left,
       RectClient.top, SRCCOPY);

/***********
* Clean up
***********/

SelectObject(hDCMemory, hBrushOld);
SelectObject(hDCMemory, hBitmapOld);
DeleteDC(hDCMemory);
DeleteObject(hBitmap);

if (hBrushNew)
   DeleteObject(hBrushNew);
}


void EditTextOut(HWND hWnd, LPEDITFIELD lpField, HDC hDC, HBRUSH hBrush,
                 HFONT hFont, RECT Rect, int x, int y, LPCTSTR lpszText, short nCnt,
                 BOOL fRightJustify, short iStart, short iEnd)
{
HBRUSH      hBrushTemp;
WORD        wFormat;
RECT        RectTemp;
RECT        RectTempHi;
BOOL        fIsEnabled;

MemCpy(&RectTemp, &Rect, sizeof(RECT));

/****************
* Draw the text
****************/

if (hFont)
   SelectObject(hDC, hFont);

fIsEnabled = IsWindowEnabled(hWnd);

if (fIsEnabled || GetBkColor(hDC) != GetSysColor(COLOR_GRAYTEXT))
   {
#ifdef SS_V40
//	if (lpField->fAutoHScroll)
//		nCnt = min(nCnt, EditGetRightVisCharPos(lpField) + 1);
#endif // SS_V40

   if (!fIsEnabled)
      SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));

   if (fRightJustify)
      wFormat = DT_TOP | DT_SINGLELINE | DT_RIGHT | DT_NOPREFIX;
   else
      wFormat = DT_TOP | DT_SINGLELINE | DT_LEFT | DT_NOPREFIX;

   DrawText(hDC, &lpszText[lpField->nLeftChar], nCnt - lpField->nLeftChar, &RectTemp, wFormat);

   /*****************
   * Draw Highlight
   *****************/

   if (GetFocus() == hWnd && lpField->bHighlighted)
      {
		int iSaveDC = SaveDC(hDC);
		IntersectClipRect(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);
      CopyRect(&RectTempHi, &RectTemp);

      iStart = max(lpField->iHiStart, lpField->nLeftChar);
      iEnd = min(lpField->iHiEnd, nCnt);

      RectTempHi.left =  LeftOf(lpField, iStart);
// 96' 6/26 Modofied by BOC Gao. for Mask select processing.
#ifdef SPREAD_JPN
      RectTempHi.right = RightOf(lpField, iEnd);
// --------------------------<<
#else
      RectTempHi.right = RightOf(lpField, iEnd - 1);
#endif
      RectTempHi.bottom =  Rect.top + lpField->yHeight + 1;

      wFormat = DT_TOP | DT_SINGLELINE | DT_LEFT | DT_NOPREFIX;

      SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
      SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));

      DrawText(hDC, &lpszText[iStart], iEnd - iStart, &RectTempHi,
               wFormat);
		RestoreDC(hDC, iSaveDC);
      }
   }

else
   {
   hBrushTemp = CreateSolidBrush(GetTextColor(hDC));
   GrayString(hDC, hBrushTemp, NULL, (DWORD)lpszText[lpField->nLeftChar], 0, x, y, 0, 0);
   DeleteObject(hBrushTemp);
   }
}


void CloseField (HWND hWnd)

// Called in response to WM_KILLFOCUS only.
{
LPEDITFIELD lpField;

if (lpField = LockField(hWnd))
   {
   if (lpField->bMouseDown)
      MouseUp (hWnd);

   DestroyCaret();
   DeselectText(hWnd, lpField);
   ShiftUp(hWnd);

   UnlockField(hWnd);
   }

NotifyParent(hWnd, EN_KILLFOCUS);
}


void DestroyField (HWND hWnd)
{
FreeField(hWnd);
}


long NotifyParent (HWND hWnd, int NotificationCode)
{
LPEDITFIELD lpField;
long        lRet = 0;

if (lpField = LockField(hWnd))
   {
   if (!lpField->fSuspendNotify)
      {
      FORWARD_WM_COMMAND(hWnd, GetWindowID(hWnd), hWnd,
                         NotificationCode, SendMessage);
      FORWARD_WM_COMMAND(GetParent(hWnd), GetWindowID(hWnd), hWnd,
                         NotificationCode, SendMessage);
      }

   UnlockField(hWnd);
   }

return (lRet);
}


void EditSuspendNotify(HWND hWnd, BOOL fSuspend)
{
LPEDITFIELD lpField;
long        lRet = 0;

if (lpField = LockField(hWnd))
   {
   lpField->fSuspendNotify = fSuspend;
   UnlockField(hWnd);
   }
}

//--------------------------------------------------------------------------//
//   These functions are local, from the functions above this line only.    //
//--------------------------------------------------------------------------//

LPEDITFIELD LockField (HWND hWnd)
{
HANDLE hField;

if (hField = GetWindowEditField (hWnd))
   return (LPEDITFIELD) GlobalLock (hField);
else
   return NULL;
}


void UnlockField (HWND hWnd)

{
HANDLE hField;

if (hField = GetWindowEditField (hWnd))
   GlobalUnlock (hField);
}


void FreeField (HWND hWnd)

{
LPEDITFIELD lpField;
HANDLE      hField;

if (IsWindow(hWnd) && (hField = GetWindowEditField(hWnd)))
   {
   lpField = (LPEDITFIELD)GlobalLock(hField);

   if (lpField->hBitmapCaret)
      DeleteObject(lpField->hBitmapCaret);

   GlobalUnlock(hField);
   GlobalFree (hField);
   SetWindowEditField (hWnd, 0);
   }
}


BOOL ValidateText(HWND hWnd, LPCTSTR lpszString, int nPos)
{
return ((BOOL)SendMessage(hWnd, EM_VALIDATE, nPos, (LPARAM) lpszString));
}


void EditFieldModifed(HWND hWnd)
{
LPEDITFIELD lpField;

if (lpField = LockField(hWnd))
   {
   lpField->bModified = TRUE;
   UnlockField(hWnd);
   }
}


BOOL GetModify(HWND hWnd)
{
LPEDITFIELD lpField;
BOOL         Modify = FALSE;

if (lpField = LockField(hWnd))
   {
   Modify = lpField->bModified;
   UnlockField(hWnd);
   }

return (Modify);
}


void SetModify(HWND hWnd, BOOL Modify)
{
LPEDITFIELD lpField;

if (lpField = LockField(hWnd))
   {
   lpField->bModified = Modify;
   UnlockField(hWnd);
   }
}


BOOL ReplaceSel(hWnd, lParam)

HWND        hWnd;
LPARAM      lParam;
{
LPEDITFIELD lpField;
BOOL        Ret = FALSE;

lpField = LockField (hWnd);

if (lParam)
   {
   ReplaceSelectedText(hWnd, lpField, (LPCTSTR)lParam);
   return (TRUE);
   }

UnlockField(hWnd);
return (Ret);
}


BOOL EditSet3DParams(hWnd, wParam, lParam)

HWND   hWnd;
WPARAM wParam;
LPARAM lParam;
{
LPEDITFIELD    lpField;
RECT           RectClient;
BOOL           fRet = TRUE;

lpField = LockField (hWnd);

lpField->f3D = (BOOL)wParam;

if (lParam)
   _fmemcpy(&lpField->Edit3DFormat, (LPEDIT3DFORMAT)lParam,
            sizeof(EDIT3DFORMAT));

GetClientRect(hWnd, &RectClient);
SendMessage(hWnd, WM_SIZE, 0, MAKELONG(RectClient.right,
            RectClient.bottom));

InvalidateRect(hWnd, NULL, TRUE);
UpdateWindow(hWnd);

UnlockField(hWnd);
return (fRet);
}


BOOL EditGet3DParams(hWnd, lParam)

HWND           hWnd;
LPARAM         lParam;
{
LPEDITFIELD    lpField;
BOOL           fRet = FALSE;

lpField = LockField (hWnd);

if (lParam)
   _fmemcpy((LPVOID)lParam, &lpField->Edit3DFormat, sizeof(EDIT3DFORMAT));

fRet = lpField->f3D;

UnlockField(hWnd);
return (fRet);
}


void ProcessSize(HWND hWnd, LPARAM lParam)
{
LPEDITFIELD lpField;

if (!(lpField = LockField(hWnd)))
   return;

EditScrollHome(lpField);

if (EditGetHAlign(lpField) == EDIT_HALIGN_RIGHT)
	EditScrollEnd(lpField);

UnlockField (hWnd);
}


int EditGetStartXPos(LPEDITFIELD lpField)
{
RECT EditRect;
int  xStart;

EditGetEditRect(lpField, &EditRect);

if (EditGetHAlign(lpField) == EDIT_HALIGN_RIGHT)
	xStart = EditRect.right - FieldLength(lpField, lpField->nLeftChar);
else
   xStart = EditRect.left;

return (xStart);
}


void EditGetEditRect(LPEDITFIELD lpField, LPRECT lpRect)
{
GetClientRect(lpField->hWnd, lpRect);
InflateRect(lpRect, -XMARGIN, -YMARGIN);

if (lpField->f3D)
   {
   InflateRect(lpRect, -lpField->Edit3DFormat.dShadowSize, -lpField->Edit3DFormat.dShadowSize);
   if (lpField->Edit3DFormat.wStyle & EDIT3D_INSIDEBORDER)
      InflateRect(lpRect, -1, -1);
   }

lpRect->right -= lpField->xRightNonClientXtra;
}


void EditScrollCurrentPosIntoView(LPEDITFIELD lpField)
{
#ifdef SS_V40
EditScrollCharIntoView(lpField, (short)lpField->iCurrentPos);
#endif // SS_V40
}


void EditScrollCharIntoView(LPEDITFIELD lpField, short nViewChar)
{
#ifdef SS_V40
if (lpField->fAutoHScroll)
	{
	nViewChar = max(0, min(nViewChar, lpField->nChars - 1));

	if (nViewChar < lpField->nLeftChar)
		EditScroll(lpField, nViewChar);

	else if (lpField->bHAlign == EDIT_HALIGN_RIGHT && lpField->bHAlignActual == EDIT_HALIGN_LEFT &&
            nViewChar == max(0, lpField->nChars - 1))
		{
		lpField->bHAlignActual = EDIT_HALIGN_RIGHT;
		lpField->nLeftChar = 0;
		InvalidateRect(lpField->hWnd, NULL, TRUE);
		UpdateWindow(lpField->hWnd);
//		EditScroll(lpField, 0);
		}

	else if (nViewChar > EditGetRightVisCharPos(lpField))
		{
		RECT  Rect;
		short nChar = nViewChar + 1;
		short nInc = 5;
		int   iWidth;
		int   iMaxWidth;
		int   iLen = lstrlen(lpField->lpszString);

		EditGetEditRect(lpField, &Rect);
		iMaxWidth = Rect.right - Rect.left;

		if (iLen > 0)
			{
			while (nChar > 0)
				{
				nChar = max(0, nChar - nInc);
				iWidth = GetDBCSWidth(lpField, nChar, nViewChar - nChar + 1);

				if (iWidth > iMaxWidth)
					{
					while (iWidth > iMaxWidth && nChar < nViewChar)
						{
						nChar++;
						iWidth = GetDBCSWidth(lpField, nChar, nViewChar - nChar + 1);
						}

					break;
					}
				}
			}
		else
			nChar = 0;

		EditScroll(lpField, nChar);
		}

	else if (lpField->bHAlignActual == EDIT_HALIGN_RIGHT &&
            Edit_GetRightVisCharPos(lpField, nViewChar) < lpField->nChars - 1)
		{
		lpField->bHAlignActual = EDIT_HALIGN_LEFT;
		lpField->nLeftChar = nViewChar;
		InvalidateRect(lpField->hWnd, NULL, TRUE);
		UpdateWindow(lpField->hWnd);
		}
	}
#endif // SS_V40
}


void EditScrollHome(LPEDITFIELD lpField)
{
#ifdef SS_V40
if (lpField->nLeftChar > 0)
	EditScroll(lpField, 0);
#endif // SS_V40
}


void EditScrollEnd(LPEDITFIELD lpField)
{
#ifdef SS_V40
EditScrollCharIntoView(lpField, (short)(lpField->nChars - 1));
#endif // SS_V40
}


void EditScrollLeft(LPEDITFIELD lpField)
{
#ifdef SS_V40
if (lpField->nLeftChar > 0)
	EditScroll(lpField, (short)(lpField->nLeftChar - 1));
#endif // SS_V40
}


void EditScrollRight(LPEDITFIELD lpField)
{
#ifdef SS_V40
if (lpField->nChars - 1 > EditGetRightVisCharPos(lpField))
	EditScroll(lpField, (short)(lpField->nLeftChar + 1));
#endif // SS_V40
}


short EditGetRightVisCharPos(LPEDITFIELD lpField)
{
if (lpField->bHAlignActual == EDIT_HALIGN_RIGHT)
	return (max(0, lpField->nChars - 1));
else
	return (Edit_GetRightVisCharPos(lpField, lpField->nLeftChar));
}


short Edit_GetRightVisCharPos(LPEDITFIELD lpField, short nLeftChar)
{
short nRet = 0;

// Determine the last visible character.  The most accurate way to determine the width
// of a string is use the whole string, not just a character at a time, because some fonts
// depend on what chars are next to each other.  Since speed is not of concern in an
// edit control, we will do just that.

#ifdef SS_V40
if (lpField->fAutoHScroll)
	{
	RECT  Rect;
	short nChar = nLeftChar - 1;
	short nInc = 5;
	int   iWidth;
	int   iMaxWidth;

	EditGetEditRect(lpField, &Rect);
	iMaxWidth = Rect.right - Rect.left;

	while (nChar < lpField->nChars - 1)
		{
		nChar = min(lpField->nChars - 1, nChar + nInc);
		iWidth = GetDBCSWidth(lpField, nLeftChar, nChar - nLeftChar + 1);

		if (iWidth > iMaxWidth)
			{
			while (iWidth > iMaxWidth && nChar > 0)
				{
				nChar--;
				iWidth = GetDBCSWidth(lpField, nLeftChar, nChar - nLeftChar + 1);
				}

			break;
			}
		}

	/* RFW - 8/12/03
	nRet = nChar;
	*/
	nRet = max(nChar, nLeftChar);
	}
#endif // SS_V40

return (nRet);
}


void EditScroll(LPEDITFIELD lpField, short nNewLeftChar)
{
#ifdef SS_V40
if (lpField->fAutoHScroll)
	if (nNewLeftChar != lpField->nLeftChar)
		{
		lpField->nLeftChar = nNewLeftChar;
		InvalidateRect(lpField->hWnd, NULL, TRUE);
		UpdateWindow(lpField->hWnd);
		}
#endif // SS_V40
}


BYTE EditGetHAlign(LPEDITFIELD lpField)
{
return (lpField->bHAlignActual);
}


short EditGetLastValidCaretPos(LPEDITFIELD lpField)
{
short nCharPos;

if (lpField->bHAlign == EDIT_HALIGN_RIGHT && GetOverstrikeMode (lpField->hWnd, LOCAL))
   nCharPos = max(lpField->nChars - 1, 0);
else
   nCharPos = lpField->nChars;

return (nCharPos);
}


void EditGetRect(HWND hWnd, LPARAM lParam)
{
LPEDITFIELD lpField;
LPRECT      lpRect = (LPRECT)lParam;

lpField = LockField (hWnd);

EditGetEditRect(lpField, lpRect);

UnlockField(hWnd);
}
