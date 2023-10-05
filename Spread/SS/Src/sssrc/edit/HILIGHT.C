/*
$Revision:   1.3  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/HILIGHT.C_V  $
 * 
 *    Rev 1.3   12 Jun 1990 14:08:36   Dirk
 * 
 *    Rev 1.2   29 May 1990  9:09:06   Randall
 * Made external objects "static".
 * 
 *    Rev 1.1   28 May 1990 13:51:40   Randall
 * 
 *    Rev 1.0   25 May 1990 15:49:52   Randall
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOMINMAX

#define  DRV_EDIT       // to include our struct defns and func defns

#define  LEFT  1
#define  RIGHT 2

#include <windows.h>
#include <toolbox.h>
#include <stdlib.h>
#include <string.h>
#include "..\libgen.h"
#include "editfld.h"

//static BOOL  bShiftDown;  // The shift key is currently held down.
//static int   iAnchor;     // Start, or anchor, position for highlighting
//static int   iPrevious;   // Previous caret position for highlighting
//static BOOL  bFirstMove;  // True if this is the first move of a highlighting
                            //    operation in an OVERSTRIKE field

//----------------------------Internal Functions----------------------------//
//- This function was changed in order to intergrate DBCS characters.
//  (BOOL) parameter was added. (Masanori Iwasa)
static void UpdateSelectedRegion (HWND, LPEDITFIELD, BOOL);
//--------------------------------------------------------------------------//

void SetSelection (HWND hWnd, int iStart, int iEnd)

{
   LPEDITFIELD lpField;
#if defined(_WIN64) || defined(_IA64)
   INT_PTR iTemp;
#else
   int iTemp;
#endif

   if (lpField = LockField (hWnd))
      {
      // Limit the range to the current field contents

      if (iEnd > lpField->nChars || iEnd <= -1)
         iEnd = lpField->nChars;

      // Swap them if they are in the wrong order
      if (iEnd < (int)iStart)
      {
         iTemp = iStart;
         iStart = iEnd;
         iEnd = (int)iTemp;
      }
//Add by BOC 99.7.19 (hyt)---------------------------
//Limit the range to the current field contents
	  if (iEnd > lpField->nChars || iEnd <= -1)
         iEnd = lpField->nChars;
//---------------------------------------------------

      // If start and end are the same, turn off selection
      if (iStart == iEnd)
         {
         DeselectText (hWnd, lpField);
         lpField->iHiStart = (int)iStart;
         lpField->iHiEnd = (int)iEnd;
         }

      // Store them in the field's data structure
      else
      {
         lpField->iHiStart = (int)iStart;
         lpField->iHiEnd = (int)iEnd;

         if (GetOverstrikeMode (hWnd, LOCAL))
            lpField->iAnchor = lpField->iHiEnd;
         else
            lpField->iAnchor = lpField->iHiStart;
      }

      // Position the caret appropriately
      if (GetOverstrikeMode (hWnd, LOCAL))
         PositionCaret (hWnd, lpField, lpField->iHiStart);
      else
         PositionCaret (hWnd, lpField, lpField->iHiEnd);

      if (lpField->iHiStart == lpField->iHiEnd)
         lpField->bHighlighted = FALSE;
      else
         lpField->bHighlighted = TRUE;

      UnlockField (hWnd);

      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      }
}

//--------------------------------------------------------------------------//

LONG GetSelection (HWND hWnd)

{
   LONG lStartEnd;
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   if (lpField->bHighlighted)
      lStartEnd = MAKELONG (lpField->iHiStart, lpField->iHiEnd);

   // If there is currently no selection return the caret position.
   else
      lStartEnd = MAKELONG (lpField->iCurrentPos, lpField->iCurrentPos);

   UnlockField (hWnd);

   return lStartEnd;
}

//--------------------------------------------------------------------------//

void ClearSelection (HWND hWnd)

{
   LPEDITFIELD lpField;

   if (!GetOverstrikeMode (hWnd, LOCAL))
   {
      lpField = LockField (hWnd);

      if (lpField->bHighlighted)
         DeleteSelectedText (hWnd, lpField);

      UnlockField (hWnd);
   }

   return;
}


//--------------------------------------------------------------------------//

void MouseDown (HWND hWnd, LPARAM lParam)

{
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   if (lpField->bHighlighted) DeselectText (hWnd, lpField);
   lpField->iCurrentPos =
      FindNearestCharacter (lpField, (short)LOWORD(lParam), FALSE);

   SetCaretXPos (hWnd, CaretPosOfCharacter (lpField, lpField->iCurrentPos));

   SetCapture (hWnd);
   lpField->bMouseDown = TRUE;

   lpField->iAnchor = lpField->iCurrentPos;
   lpField->iHiEnd = lpField->iHiStart = lpField->iAnchor;
   lpField->iPrevious = lpField->iAnchor;
   lpField->bFirstMove = TRUE;

#if 0
   HideCaret (hWnd);             // There is a bug here: The HideCaret
                                 // seems to work only on the first 
                                 // highlighting operation.
#endif
   UnlockField (hWnd);
}

//--------------------------------------------------------------------------//

void MouseMove (HWND hWnd, LPARAM lParam)

{
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   if (lpField->bMouseDown)
   {
      lpField->iCurrentPos =
         FindNearestCharacter (lpField, (short)LOWORD(lParam),
                              !GetOverstrikeMode (hWnd, LOCAL));

      if (lpField->iCurrentPos != lpField->iPrevious)
         {
         UpdateSelectedRegion (hWnd, lpField, TRUE);
         SetCaretXPos(hWnd, CaretPosOfCharacter(lpField,
                      lpField->iCurrentPos));
         }
   }

   UnlockField (hWnd);
}

//--------------------------------------------------------------------------//

void MouseUp (HWND hWnd)

{
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   lpField->bMouseDown = FALSE;
   lpField->bFirstMove = FALSE;
   ReleaseCapture ();
   SetCaretXPos (hWnd, CaretPosOfCharacter (lpField, lpField->iCurrentPos));

   /*
   ShowCaret (hWnd);
   */

   UnlockField (hWnd);

   return;
}

//--------------------------------------------------------------------------//

void ShiftDown (HWND hWnd)

// For future reference: 
//    There is a small bug. If you start hihglighting by one means, 
//    via the mouse or WM_SETSEL for example, then try to extend the
//    highlighted region using shift and arrow keys, the highlighting
//    stuffs up.  Its because all the initial stuff: iAnchor, iPrevious,
//    bFirstMove, was never set.

{
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   lpField->bShiftDown = TRUE;

   if (!lpField->bHighlighted)
   {
      lpField->iAnchor = lpField->iCurrentPos;
      lpField->iHiEnd = lpField->iHiStart = lpField->iAnchor;
      lpField->iPrevious = lpField->iAnchor;
      lpField->bFirstMove = TRUE;
   }

   UnlockField (hWnd);
}

//--------------------------------------------------------------------------//

void ShiftMove (HWND hWnd)

{
   LPEDITFIELD lpField = LockField (hWnd);

   if (lpField->bShiftDown)
   {
      if (lpField->iCurrentPos != lpField->iPrevious)
         {
         UpdateSelectedRegion (hWnd, lpField, FALSE);
         SetCaretXPos(hWnd, CaretPosOfCharacter(lpField,
                      lpField->iCurrentPos));
         }

   }

   UnlockField (hWnd);
}

//--------------------------------------------------------------------------//

void ShiftUp(HWND hWnd)

{
   LPEDITFIELD lpField = LockField (hWnd);
   lpField->bShiftDown = FALSE;
   lpField->bFirstMove = FALSE;
   UnlockField (hWnd);
}

//--------------------------------------------------------------------------//

BOOL IsShiftDown(HWND hWnd)

{
   LPEDITFIELD lpField = LockField (hWnd);
   BOOL        fRet;

   fRet = lpField->bShiftDown;
   UnlockField (hWnd);
   return (fRet);
}

//--------------------------------------------------------------------------//

//--------------------------------------------------------------------------
// The parameter count changed for this function. Check the declaration
// for more details. (Masanori Iwasa)
//--------------------------------------------------------------------------
void UpdateSelectedRegion (HWND hWnd, LPEDITFIELD lpField, BOOL bMouseMove)
// There are three stages to this operation:
//    1. Figure out which part of the currently selected region needs
//       to be inverted
//    2. Invert it
//    3. Figure out exactly which characters are now inverted.
//
// For INSERT mode this is easy because:
//    o The caret is always to the left of the current character
//    o Therefore the pivot position is always to the left of the 
//      anchor character
//    o And, we always want to highlight up to BUT not including the
//      extreem right hand, or LAST character (indexed by iEnd);
//
// For OVERSTRIKE mode it is mode difficult:
//    o The caret is underneath the current character
//    o Therefore, the pivot position is to the LEFT of the anchor 
//      character if the first move was to the RIGHT, and to the RIGHT 
//      of the anchor character if the first move was to the LEFT.
//    o We want to avoid inverting either the FIRST or the LAST character
//      of the new sub-region (and therefore of the whole selected region)
//      depending on various criteria.  These include which side of the 
//      anchor you currently are, which side of the anchor the pivot
//      position is, and which direction you're going.

{
   int iStart, iEnd;
   static int nAnchorPivotSide;   // For OVERSTRIKE mode: which side of
                             // the anchor character the pivot position is on.
   BOOL bOverstrike;              // The field is in overstrike mode.

   // Figure out which part of the currently selected region 
   // needs to be inverted. 
   if (lpField->iCurrentPos > lpField->iPrevious)
   {
      iStart = lpField->iPrevious;
      iEnd = lpField->iCurrentPos;
   }
   else
   {
      iStart = lpField->iCurrentPos;
      iEnd = lpField->iPrevious;
   }

   bOverstrike = GetOverstrikeMode (hWnd, LOCAL);

   // OVERSTRIKE mode.
   // The given range usually needs trimming at one end or another.
   if (bOverstrike)
   {
      if (lpField->bFirstMove)          // No trimming required but register which
      {                        //    we are heading in initially.
         if (lpField->iCurrentPos > lpField->iPrevious)
            nAnchorPivotSide = LEFT;
         else
            nAnchorPivotSide = RIGHT;

         lpField->bFirstMove = FALSE;
      }

      else
      {

         // Currently to the RIGHT of anchor.
         // Usually don't want to invert the FIRST character
         if (lpField->iCurrentPos > lpField->iAnchor)
         {
            if (lpField->iPrevious > lpField->iAnchor ||
               (lpField->iPrevious == lpField->iAnchor && nAnchorPivotSide == LEFT))
               ++iStart;
         }

         // Currently to the LEFT of anchor.
         // Usually don't want to invert the LAST character
         else if (lpField->iCurrentPos < lpField->iAnchor)
         {
            if (lpField->iPrevious < lpField->iAnchor ||
               (lpField->iPrevious == lpField->iAnchor && nAnchorPivotSide == RIGHT))
               --iEnd;
         }

         // Currently ON the anchor. What we invert depends which 
         // direction we are moving, and which side of the anchor 
         // character the pivot is on.
         else
         {
            if (lpField->iCurrentPos < lpField->iPrevious && // Moving left
                nAnchorPivotSide == LEFT)
                  ++iStart;

            else if (lpField->iCurrentPos > lpField->iPrevious && // Moving right
                     nAnchorPivotSide == RIGHT)
                  --iEnd;
         }
      }
   }

   // INSERT mode.
   // Never include the last character.
   if (!bOverstrike) --iEnd;

   // Invert the new sub-region

/*
   Invert (hWnd, lpField, iStart, iEnd);
*/

   // Figure out exactly which characters are now inverted and update
   // our internal records
   if (lpField->iCurrentPos > lpField->iAnchor)
   {
      if (IsCharDbcs(lpField, lpField->iAnchor) == 2) lpField->iAnchor++;
      if (IsCharDbcs(lpField, lpField->iCurrentPos) == 1)
          {
          if (lpField->iCurrentPos > lpField->iPrevious || bMouseMove)
              lpField->iCurrentPos++;
          else
              lpField->iCurrentPos--;
          }
      lpField->iHiStart = lpField->iAnchor;
      lpField->iHiEnd = lpField->iCurrentPos + 1;
   }
   else 
   {
      if (IsCharDbcs(lpField, lpField->iAnchor) == 1) lpField->iAnchor--;
      if (IsCharDbcs(lpField, lpField->iCurrentPos) == 2)
          {
          if (lpField->iCurrentPos < lpField->iPrevious || bMouseMove)
              lpField->iCurrentPos--;
          else
              lpField->iCurrentPos++;
          }
      lpField->iHiStart = lpField->iCurrentPos;
      lpField->iHiEnd = lpField->iAnchor + 1;
   }

   // OVERSTRIKE mode: Don't include the anchor character if we are 
   // on the same side of it as the pivot position.
// Modified by BOC FMH 1996.07.09. -------------------->>
//   for miss selection of KANJI string by mouse.
//#ifdef SPREAD_JPN
//   if (bOverstrike)
//      if (lpField->iCurrentPos < lpField->iAnchor && nAnchorPivotSide == LEFT)
//         --lpField->iHiEnd;
//      else if (lpField->iCurrentPos > lpField->iAnchor && nAnchorPivotSide == RIGHT)
//         ++lpField->iHiStart;
//#endif

   // BJO 24Apr97 SCS5100 - Before fix
   //bOverstrike = !(BOOL)SendMessage(hWnd, EM_CARETPOS, lpField->iHiEnd, 1l);
   // BJO 24Apr97 SCS5100 - Begin fix
   bOverstrike = !(BOOL)SendMessage(hWnd, EM_CARETPOS, lpField->iHiEnd - 1, 1l);
   // BJO 24Apr97 SCS5100 - End fix

   // INSERT mode: Never include the last character.
   if (!bOverstrike) --lpField->iHiEnd;

   lpField->bHighlighted = lpField->iHiEnd > lpField->iHiStart;

   HighlightField(hWnd, lpField);

   lpField->iPrevious = lpField->iCurrentPos;
}

//--------------------------------------------------------------------------//

void DeselectText (HWND hWnd, LPEDITFIELD lpField)

{
   if (lpField->bHighlighted)
   {
/*
      Invert (hWnd, lpField, lpField->iHiStart, lpField->iHiEnd - 1);
*/

      lpField->iHiEnd = lpField->iHiStart = lpField->iCurrentPos;
      lpField->bHighlighted = FALSE;

      HighlightField(hWnd, lpField);
   }
   else
      lpField->iHiEnd = lpField->iHiStart = lpField->iCurrentPos;
}

//--------------------------------------------------------------------------//

void DeleteSelectedText (HWND hWnd, LPEDITFIELD lpField)

{
   TCHAR lpszLocal[MAXFIELD + 1];

   if (lpField->bHighlighted)
   {
      StrCpy(lpszLocal, lpField->lpszString);
      StrRemove(lpszLocal, lpField->iHiStart, min(lstrlen(lpszLocal),
                lpField->iHiEnd) - 1);
      if (ValidateText(hWnd, lpszLocal, lpField->iCurrentPos))
      {
         StrCpy (lpField->lpszString, lpszLocal);
         lpField->nChars = StrLen(lpField->lpszString);
         NotifyParent (hWnd, EN_UPDATE);
// RFW - 11/17/01         lpField->nChars -= lpField->iHiEnd - lpField->iHiStart;
         lpField->nChars = max(lpField->nChars, 0);
         PositionCaret (hWnd, lpField, lpField->iHiStart);
         lpField->bHighlighted = FALSE;
         InvalidateRect (hWnd, NULL, TRUE);
         UpdateWindow(hWnd);
         EditFieldModifed(hWnd);
         NotifyParent (hWnd, EN_CHANGE);
      }
   }
}

//--------------------------------------------------------------------------//

void ReplaceSelectedText (HWND hWnd, LPEDITFIELD lpField, LPCTSTR lpNewText)

{
   TCHAR lpszLocal[MAXFIELD + 1];
   short dCurrentPos, dValidateStart;
   int   nTextLen, iStrLen;

   if (lpField->bHighlighted)
   {
      nTextLen = min(lstrlen(lpNewText), MAXFIELD);

      _fmemcpy(lpszLocal, lpNewText, nTextLen * sizeof(TCHAR));
      lpszLocal[nTextLen] = '\0';

      if (!SendMessage(hWnd, EM_REPLACETEXT, 0, (LPARAM)(LPTSTR)lpszLocal))
         {
         StrnnCpy (lpszLocal, lpField->lpszString, lpField->iHiStart+1);
         StrCat (lpszLocal, lpNewText);
			if (lpField->iHiEnd < lstrlen(lpField->lpszString))
				StrCat (lpszLocal, &lpField->lpszString[lpField->iHiEnd]);

			dValidateStart = lpField->iHiStart;
         dCurrentPos = lpField->iHiStart + nTextLen;
         }
      else
			{
			dValidateStart = lpField->iHiStart;
         dCurrentPos = lpField->iCurrentPos;
			}

      SendMessage(hWnd, EM_RESETPREVLEN, 0, 0L);

		/* RFW - 5/10/05 - 15408
      if (ValidateText(hWnd, lpszLocal, dCurrentPos))
		*/
		
		iStrLen = lstrlen(lpszLocal);
      if (ValidateText(hWnd, lpszLocal, dValidateStart))
      {
			MSG msg;
			PeekMessage(&msg, hWnd, EM_SETCARETPOS, EM_SETCARETPOS, PM_REMOVE); // RFW - 12/27/05 - 17882
         StrCpy (lpField->lpszString, lpszLocal);
         lpField->nChars = StrLen(lpField->lpszString);
         NotifyParent (hWnd, EN_UPDATE);
			/* RFW - 6/14/04 - 14590
         PositionCaret (hWnd, lpField, lpField->iHiStart + nTextLen);
			*/
			/* RFW - 6/18/07 - 20391
         PositionCaret (hWnd, lpField, min(lpField->nChars, lpField->iHiStart + nTextLen));
			*/
         PositionCaret (hWnd, lpField, min(lpField->nChars, lpField->iHiStart + nTextLen + (lstrlen(lpszLocal) - iStrLen)));
         lpField->bHighlighted = FALSE;
         InvalidateRect (hWnd, NULL, TRUE);
         UpdateWindow(hWnd);
         EditFieldModifed(hWnd);
         NotifyParent (hWnd, EN_CHANGE);
      }
   }
}
