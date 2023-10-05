/*
$Revision:   1.1  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/TEXT.C_V  $
 * 
 *    Rev 1.1   12 Jun 1990 14:08:32   Dirk
 * No change.
 * 
 *    Rev 1.0   28 May 1990 13:50:06   Randall
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOMINMAX

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <toolbox.h>
#include "..\libgen.h"
#include "editfld.h"

//--------------------------------------------------------------------------//

void SetFieldText (HWND hWnd, LPARAM lParam)
{
   LPEDITFIELD lpField;
   LPTSTR lpszText;

   if (lParam)
      lpszText = (LPTSTR) lParam;
   else
      lpszText = _T("");

   lpField = LockField (hWnd);

	// RFW - 5/27/99 - SPR_CEL_004_003
	if (lstrcmp(lpField->lpszString, lpszText))
		{
		if (lpField->bHighlighted) DeselectText (hWnd, lpField);

		StrnCpy (lpField->lpszString, lpszText, MAXFIELD);

		if (lpField->nMaxChars != -1)
			lpField->lpszString[lpField->nMaxChars] = '\0';

		if (lpField->nCase == UPPERCASE)       // Force case?
			StrUpr (lpField->lpszString);
		else if (lpField->nCase == LOWERCASE)
			StrLwr (lpField->lpszString);

		lpField->nChars = StrLen(lpField->lpszString);
		NotifyParent (hWnd, EN_UPDATE);

		if (lpField->iCurrentPos >= lpField->nChars)
			PositionCaret(hWnd, lpField, lpField->nChars);

		InvalidateRect (hWnd, NULL, TRUE);
		/*
		UpdateWindow(hWnd);
		*/

		EditFieldModifed(hWnd);
		NotifyParent (hWnd, EN_CHANGE);
		}

   if (IsWindow(hWnd))
      UnlockField (hWnd);
}

//--------------------------------------------------------------------------//

short GetFieldText(HWND hWnd, WPARAM wParam, LPARAM lParam)

{
   LPEDITFIELD lpField;
   LPTSTR      lpszText;
   short       dLen = 0;
   short       dLen2 = 0;

   if (lParam)
      {
      lpszText = (LPTSTR)lParam;

      lpField = LockField (hWnd);

      dLen = lstrlen(lpField->lpszString);
      dLen2 = dLen + 1;

      if (wParam > 0)
        {
        dLen = min(dLen, (short)wParam);
        dLen2 = min(dLen2, (short)wParam);
        }

      _fmemcpy(lpszText, lpField->lpszString, dLen2 * sizeof(TCHAR));

      UnlockField (hWnd);
      }

   return (dLen);
}

//--------------------------------------------------------------------------//

LONG GetTextLength (HWND hWnd)

{
   LPEDITFIELD lpField;
   LONG lTextLength;

   lpField = LockField (hWnd);
/*
   lTextLength = lpField->nChars;
*/

   lTextLength = lstrlen(lpField->lpszString);

   UnlockField (hWnd);

   return lTextLength;
}

//--------------------------------------------------------------------------//

void AdjustLength (HWND hWnd)

{
   LPEDITFIELD lpField;

   lpField = LockField (hWnd);

   lpField->nChars = StrLen (lpField->lpszString);

   UnlockField (hWnd);
}


void InsertText(HWND hWnd, LPEDITFIELD lpField, LPCTSTR lpNewText)

{
   TCHAR lpszLocal[MAXFIELD + 1];
   int   nTextLen;

   MemSet(lpszLocal, '\0', sizeof(lpszLocal));

   nTextLen = StrLen(lpNewText);
   nTextLen = min(nTextLen, MAXFIELD - lstrlen(lpField->lpszString));

   MemCpy(lpszLocal, lpField->lpszString, (WORD)(max(0, lpField->iCurrentPos) * sizeof(TCHAR)));
   MemCpy(&lpszLocal[lstrlen(lpszLocal)], lpNewText, (WORD)(nTextLen * sizeof(TCHAR)));
   StrCat(&lpszLocal[lstrlen(lpszLocal)],
          &lpField->lpszString[lpField->iCurrentPos]);

   if (lpField->nMaxChars != -1)
      lpszLocal[lpField->nMaxChars] = '\0';

   if (ValidateText(hWnd, lpszLocal, lpField->iCurrentPos))
      {
      StrCpy (lpField->lpszString, lpszLocal);
      lpField->nChars = StrLen(lpField->lpszString);
      NotifyParent (hWnd, EN_UPDATE);
      /*
      PositionCaret (hWnd, lpField, lpField->iCurrentPos + nTextLen);
      */
      InvalidateRect (hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      EditFieldModifed(hWnd);
      NotifyParent (hWnd, EN_CHANGE);
      }
}
