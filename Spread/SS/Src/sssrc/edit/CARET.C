/*
$Revision:   1.2  $
*/
/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/CARET.C_V  $
 * 
 *    Rev 1.2   12 Jun 1990 14:08:08   Dirk
 * No change.
 * 
 *    Rev 1.1   30 May 1990  9:36:38   Randall
 * Don't move the caret if you haven't got the Focus!
 * 
 *    Rev 1.0   28 May 1990 13:50:26   Randall
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOMINMAX

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>
#include "..\libgen.h"
#include "editfld.h"

#define  CARET_BITMAP_WIDTH   8
#define  CARET_BITMAP_HEIGHT  4
#define  CARET_HALF_WIDTH     (CARET_BITMAP_WIDTH/2)
static BITMAP  Bitmap = {0, CARET_BITMAP_WIDTH, CARET_BITMAP_HEIGHT, 2, 1, 1};
static BYTE    byBits[] =
{
   0x18, 0x00,       //    XX   
   0x3C, 0x00,       //   XXXX  
   0x7E, 0x00,       //  XXXXXX 
   0xFF, 0x00        // XXXXXXXX
};

//--------------------------------------------------------------------------//

BOOL CaretPosition(HWND hWnd, WPARAM nChar, LPARAM lParam)
{
BOOL fRet = 0;

if (lParam)
   {
   if (GetOverstrikeMode (hWnd, LOCAL))
      fRet = FALSE;
   else
      fRet = TRUE;
   }

return (fRet);
}


void SetCaretPosition (HWND hWnd, WPARAM nChar)

{
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   PositionCaret (hWnd, lpField, (int)nChar);

   UnlockField (hWnd);

   return;
}

//--------------------------------------------------------------------------//

int GetCaretPosition (HWND hWnd)

{
   int xPos;
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   xPos = lpField->iCurrentPos;

   UnlockField (hWnd);

   return xPos;
}


//--------------------------------------------------------------------------//

int CaretPosOfCharacter (LPEDITFIELD lpField, int nChar)

{
   int   xStart;
   int   xPos;
   int   nKanjiChk = 0;

   xStart = EditGetStartXPos(lpField);

// 96' 6/26 Modified by BOC Gao.
// for Japanese processing.
	xPos = xStart;
    if ( nChar > 0 )
	{
		if ( IsCharDbcs(lpField, nChar - 1) == 1 )
		{
			xPos += GetDBCSWidth(lpField, lpField->nLeftChar, nChar - 1 - lpField->nLeftChar);
			xPos += GetDBCSWidth(lpField, nChar - 1, 2) / 2 + 1;
		}
		else 
			xPos += GetDBCSWidth(lpField, lpField->nLeftChar, nChar - lpField->nLeftChar);
	}

	if (GetOverstrikeMode (lpField->hWnd,LOCAL))
	{
		if (nChar < lpField->nChars)
			nKanjiChk = IsCharDbcs(lpField, nChar);

		if ( nKanjiChk == 1 )
			xPos += GetDBCSWidth(lpField, nChar, 2) / 4;
		else if ( nKanjiChk == 2 )
			xPos += GetDBCSWidth(lpField, nChar - 1, 2) / 4;
		else
			{
			if (nChar < lpField->nChars)
				xPos += GetDBCSWidth(lpField, nChar, 1) / 2;
			else
				{
				TEXTMETRIC tm;
				HFONT  hOldFont;

				HDC hDC = GetDC(lpField->hWnd);

				hOldFont = SelectObject(hDC, lpField->hFont);
				GetTextMetrics(hDC, &tm);
				SelectObject(hDC, hOldFont);

				ReleaseDC(lpField->hWnd, hDC);

				xPos += tm.tmAveCharWidth / 2;
				}
			}
	}
	else if (EditGetHAlign(lpField) == EDIT_HALIGN_RIGHT)
		xPos -= lpField->tmOverhang;

   return xPos;
}

//-------------------------------------------------------------------------//

void HomeCaret(HWND hWnd, LPEDITFIELD lpField)
{
   PositionCaret(hWnd, lpField, 0);
//   lpField->iCurrentPos = 0;
//	EditScrollHome(lpField);
//   SetCaretXPos (hWnd, CaretPosOfCharacter(lpField, lpField->iCurrentPos));
}

//--------------------------------------------------------------------------//

void  EndCaret (HWND hWnd, LPEDITFIELD lpField)
{
   PositionCaret(hWnd, lpField, EditGetLastValidCaretPos(lpField));

//   lpField->iCurrentPos = EditGetLastValidCaretPos(lpField);
//	EditScrollEnd(lpField);
//   SetCaretXPos (hWnd, CaretPosOfCharacter (lpField, lpField->iCurrentPos));
}

//--------------------------------------------------------------------------//

void  LeftCaret (HWND hWnd, LPEDITFIELD lpField)

{
	if (lpField->iCurrentPos > 0)
		{
		EditScrollCurrentPosIntoView(lpField);
		if (lpField->iCurrentPos <= lpField->nLeftChar)
			EditScrollLeft(lpField);

		SetCaretXPos (hWnd, CaretPosOfCharacter (lpField, --lpField->iCurrentPos));
		}
}

//--------------------------------------------------------------------------//

void  RightCaret (HWND hWnd, LPEDITFIELD lpField)

{
   // Right justified overstrike fields...
   //    ...don't let them position past the last character.
   if (lpField->iCurrentPos >= EditGetLastValidCaretPos(lpField))
      return;

	lpField->iCurrentPos++;
	EditScrollCurrentPosIntoView(lpField);

   SetCaretXPos (hWnd, CaretPosOfCharacter (lpField, lpField->iCurrentPos));
}

//--------------------------------------------------------------------------//

void  PositionCaret (HWND hWnd, LPEDITFIELD lpField, int nChar)

{           
//#ifndef _WIN64 // removed from 64-bit / WPARAM is unsigned, so nChar is always < -1 -scl
   // RAP 6/26/96 fix for JIS3302 -- a -2 was being passed to SetCaretXPos()
   // resulting in a GPF...                    
   if (nChar < -1)
     return;
   // RAP 6/26/96                     
//#endif
   lpField->iCurrentPos = (int)nChar;
	EditScrollCurrentPosIntoView(lpField);
   SetCaretXPos (hWnd, CaretPosOfCharacter (lpField, lpField->iCurrentPos));
}

//--------------------------------------------------------------------------//

void SetCaretXPos (HWND hWnd, int xPos)

{
   if (GetFocus() == hWnd)       // This field has the focus
      {
	   LPEDITFIELD lpField;
		RECT        EditRect;

	   lpField = LockField (hWnd);
		EditGetEditRect(lpField, &EditRect);

      if (GetOverstrikeMode (hWnd, LOCAL))
         SetCaretPos (xPos - CARET_HALF_WIDTH, lpField->yHeight + EditRect.top -
                      CARET_BITMAP_HEIGHT + 2);
      else
         SetCaretPos (xPos, EditRect.top + 1);

      SendMessage(hWnd, EM_CARETPOS, lpField->iCurrentPos, 0l);

	   UnlockField(hWnd);
      }

}

//--------------------------------------------------------------------------//

int GetCaretXPos (HWND hWnd)

{
   POINT Point;
   int   xPos;

   GetCaretPos (&Point);
   xPos = Point.x;

   if (GetOverstrikeMode (hWnd, LOCAL))
      xPos += CARET_HALF_WIDTH;

   return xPos;
}

//--------------------------------------------------------------------------//

void CreateModeCaret (HWND hWnd, LPEDITFIELD lpField)

{
   int     xWidth, yHeight;
   HBITMAP hBitmap;   

   DestroyCaret ();

   if (GetOverstrikeMode (hWnd, LOCAL))
   {
      xWidth = CARET_BITMAP_WIDTH;
      yHeight = CARET_BITMAP_HEIGHT;

      if (!lpField->hBitmapCaret)
         {
         Bitmap.bmBits = (LPVOID) byBits;
         lpField->hBitmapCaret = CreateBitmapIndirect(&Bitmap);
         }

      hBitmap = lpField->hBitmapCaret;

      if (lpField->nChars > 0 && lpField->iCurrentPos == lpField->nChars)
         --lpField->iCurrentPos;
   }
   else
   {
      xWidth = 1;
      yHeight = lpField->yHeight + 1;
      hBitmap = NULL;
   }

   CreateCaret (hWnd, hBitmap, xWidth, yHeight);
   SetCaretXPos (hWnd, CaretPosOfCharacter (lpField, lpField->iCurrentPos));
   if (GetFocus() == hWnd)       // This field has the focus
      ShowCaret (hWnd);
   
   return;
}

//--------------------------------------------------------------------------//

void PositionCaretAfterPaint (HWND hWnd)

// This routine positions the caret to the default position after
// a highlighted field is painted.  In practice this is only neccessary
// for the first field in a dialog box when it gets painted on initialization.
// In all other cases the caret gets positioned correctly by the sub-class
// on WM_SETFOCUS.  On dialog initialization, however, the WM_SETFOCUS is
// received before the WM_PAINT.  The WM_PAINT must give the subclass a 
// chance to do any special caret positioning, e.g., to the decimal point
// in a float field.  (See PaintField in fieldmgr.c)
//
// This function provides the default positioning if the subclass chose
// not to process the EM_EDIT_POSITIONAFTERPAINT message.

{
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   if (GetOverstrikeMode (hWnd, LOCAL))
      PositionCaret (hWnd, lpField, lpField->iHiStart);
   else
      PositionCaret (hWnd, lpField, lpField->iHiEnd);

   UnlockField (hWnd);
}
