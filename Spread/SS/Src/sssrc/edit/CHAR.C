/*
$Revision:   1.3  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/CHAR.C_V  $
 * 
 *    Rev 1.3   12 Jun 1990 14:07:54   Dirk
 * No change.
 * 
 *    Rev 1.2   06 Jun 1990 16:17:58   Randall
 * Ensure FindNearestCharacter doesn't return > end of string.
 * 
 *    Rev 1.1   29 May 1990  9:09:44   Randall
 * Made external objects "static".
 * 
 *    Rev 1.0   28 May 1990 13:50:58   Randall
 * Initial revision.
*/


//---------*---------*---------*---------*---------*---------*---------*-----
// 
// UPDATE LOG:
//
//		RWP01	10.18.96	<JIS3305> Can't turn off the beep sound
//	
//---------*---------*---------*---------*---------*---------*---------*-----


#define  NOCOMM
#define  NOKANJI

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>
#include "..\libgen.h"
#include "editfld.h"

//RWP01a
extern void SS_Beep2(HWND hWnd);
#ifdef SPREAD_JPN
extern void SS_BeepLowlevel(HWND hWnd);
#endif
//RWP01a

//----------------------------Internal Functions----------------------------//
//- This function was changed in order to intergrate DBCS characters.
//  (LPINT) parameter was added, and it is used to check for DBCS
//  first byte. (Masanori Iwasa)
static BOOL CheckFit (LPEDITFIELD, TCHAR);
//--------------------------------------------------------------------------//

HFONT ProcessEditSetFont(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   LPEDITFIELD lpField;
 
   lpField = LockField (hWnd);
   if (lpField)
   {
      if (wParam != 0 || lpField->hFont != 0)
      {
         if (GetFontWidth(hWnd, (HFONT)wParam) && (BOOL)lParam)
            InvalidateRect(hWnd, NULL, TRUE);    // repaint with the new font
      }
      UnlockField (hWnd);
   }
   return (HFONT)wParam;
}


HFONT ProcessEditGetFont(HWND hWnd)
{
LPEDITFIELD lpField;
HFONT       hFont = 0;

lpField = LockField (hWnd);
if (lpField)
   {
   hFont = lpField->hFont;
   UnlockField (hWnd);
   }

return (hFont);
}

//--------------------------------------------------------------------------//

BOOL  GetFontWidth(HWND hWnd, HFONT hFont)
{
   HDC         hDC;
   TEXTMETRIC  tm;
   //int         nCharWidths[N_PRINTABLE_CHARS];
   LPEDITFIELD lpField;
   BOOL        fRetCode = FALSE;
   register    int   i;
   SIZE        Size;

   lpField = LockField (hWnd);
   if (lpField)
   {
      hDC = GetDC (hWnd);
      if (hDC)
      {
         lpField->hFont = hFont;
         if (hFont)                 // a font is given so select it
            SelectObject(hDC, hFont);
         // else use the system font

         GetTextMetrics(hDC, &tm);
         lpField->yHeight = tm.tmHeight;
         //- This structure member was added to support DBCS characters.
         //  (Masanori Iwasa)
         lpField->nMaxCharWidth = tm.tmMaxCharWidth;
         lpField->tmOverhang = tm.tmOverhang;

//         GetCharWidth (hDC, ' ', N_PRINTABLE_CHARS, nCharWidths);
//         for (i = 0; i < N_PRINTABLE_CHARS - ' '; ++i)
//            lpField->nCharWidthTable[i] = (BYTE)nCharWidths[i] -
//                                          (BYTE)tm.tmOverhang;

         {
         TCHAR Buffer[1 + 1];

         Buffer[1] = '\0';
         for (i = ' '; i < N_PRINTABLE_CHARS; ++i)
            {
            Buffer[0] = (TCHAR)(i);
            GetTextExtentPoint(hDC, Buffer, 1, &Size);
            lpField->nCharWidthTable[i - ' '] = (BYTE)Size.cx -
                                                (BYTE)tm.tmOverhang;
            }
         }

         /*
         Buffer[1] = '\0';

         for (i = ' '; i < N_PRINTABLE_CHARS; ++i)
            {
            Buffer[0] = (TCHAR)(i);
            lpField->nCharWidthTable[i - ' '] = (BYTE)LOWORD(GetTextExtent(hDC,
                                                Buffer, 1));
            }
         */

         ReleaseDC(hWnd, hDC);
         fRetCode = TRUE;
      }

      UnlockField (hWnd);
   }
   return(fRetCode);
}

//--------------------------------------------------------------------------//

void ReplaceCharacter (HWND hWnd, LPEDITFIELD lpField, TCHAR cChar)

{
   TCHAR lpszLocal[MAXFIELD + 1];

   StrCpy (lpszLocal, lpField->lpszString);
   lpszLocal[lpField->iCurrentPos] = cChar;
   if (ValidateText(hWnd, lpszLocal, lpField->iCurrentPos))
   {
		int iChangeMade = lstrcmp(lpField->lpszString, lpszLocal);

		if (iChangeMade)
			{
			StrCpy (lpField->lpszString, lpszLocal);
			NotifyParent (hWnd, EN_UPDATE);
			}

      RightCaret (hWnd, lpField);

		if (iChangeMade)
			{
	      InvalidateRect(hWnd, NULL, TRUE);
	      UpdateWindow(hWnd);
	      EditFieldModifed(hWnd);
	      NotifyParent(hWnd, EN_CHANGE);
			}
   }
}

//--------------------------------------------------------------------------//

//--------------------------------------------------------------------------
// The parameter count changed for this function. Check the declaration
// for more details. (Masanori Iwasa)
//--------------------------------------------------------------------------
void InsertCharacter (HWND hWnd, LPEDITFIELD lpField, TCHAR cChar)
{
   TCHAR lpszLocal[MAXFIELD + 1];
#ifdef SPREAD_JPN
//Modify by BOC 99.4.15(hyt)--------------
//I don't know why check the width
//But if check it sometimes will just input one part of conent like "am"
//Only input "a"
//   if (!CheckFit (lpField, cChar))
   if (FALSE)
//----------------------------------------
// 96' 6/26 Modified by BOC Gao.
            SS_BeepLowlevel(hWnd);
// ------------------------<<
#else

   if (!CheckFit (lpField, cChar))
      SS_Beep2(hWnd);					// No more room in field.

#endif
   else
   {

#ifdef SPREAD_JPN
// 97' 5/16 Modified by BOC Gao. Incident 426
// Float type cell with separater, Zengaku input ‚P‚Q‚R‚S‚T‚U
// using F8 to change it to Hangaku, then you can not get 
// a 123,456 but 123,564. I just clear out the stacked messages.
	  MSG msg;
	  while ( PeekMessage(&msg, hWnd, EM_SETCARETPOS, EM_SETCARETPOS, PM_REMOVE ))
		  SendMessage(hWnd, EM_SETCARETPOS, msg.wParam, msg.lParam);
// ---------------------------<<
#endif

      StrCpy (lpszLocal, lpField->lpszString);

      StrInsertChar (cChar, lpszLocal, lpField->iCurrentPos);

      lpField->bCharBeingInserted = TRUE;
      if (ValidateText(hWnd, lpszLocal, lpField->iCurrentPos))
      {
         StrCpy (lpField->lpszString, lpszLocal);
		 //Modify By BOC 99.4.30 (hyt)---------------------
		 //for can't ensure always insert one char
		 //Date Type cell press "s,h,t,m" will convert to other string
         //++lpField->nChars;
			lpField->nChars = lstrlen(lpszLocal);
		 //-------------------------------------------------
         NotifyParent (hWnd, EN_UPDATE);
         RightCaret (hWnd, lpField);
         InvalidateRect (hWnd, NULL, TRUE);
         UpdateWindow(hWnd);
         EditFieldModifed(hWnd);
         NotifyParent (hWnd, EN_CHANGE);
         if (!IsWindow(hWnd))
            return;
      }

      lpField->bCharBeingInserted = FALSE;
   }

   return;
}

//--------------------------------------------------------------------------//

int FieldLength (LPEDITFIELD lpField, short nStartPos)
{
// 96' 6/26 Modified by BOC Gao. for processing DBCS char.
	int  xLength = 0;

	if ( lpField->nChars )
		xLength = GetDBCSWidth(lpField, nStartPos, lpField->nChars - nStartPos);
   
   return xLength;
}


int GetStringLength(HDC hDC, LPCTSTR lpString, short Length)
{
SIZE Size;

GetTextExtentPoint(hDC, lpString, Length, &Size);
return Size.cx;
}


int EditGetCharFromPos(HWND hWnd, LPARAM lParam)
{
   LPEDITFIELD lpField;
   int iRet;

   lpField = LockField (hWnd);
   iRet = FindNearestCharacter (lpField, (short)LOWORD(lParam), FALSE);
   UnlockField (hWnd);

	return (iRet);
}


//--------------------------------------------------------------------------//

int FindNearestCharacter (LPEDITFIELD lpField, int xRawPos, BOOL bRoundUp)

{
   int   xStart;
   int   xPos;
   int   nCharWidth;
   int   iChar;
   int   nLimit;

   xStart = EditGetStartXPos(lpField);
   nLimit = EditGetLastValidCaretPos(lpField);

// 96' 6/26 Modified by BOC Gao. for processing DBCS char.
   for (iChar = lpField->nLeftChar, xPos = xStart; iChar < nLimit; ++iChar)
      {
		int Kanji = IsCharDbcs(lpField, iChar);
      if ( Kanji == 1 )
         nCharWidth = GetDBCSWidth(lpField, iChar, 2) / 2;
		else if ( Kanji == 2 )
         nCharWidth = GetDBCSWidth(lpField, iChar - 1, 2) / 2;
      else
         nCharWidth = GetDBCSWidth(lpField, iChar, 1);

      if (xPos + nCharWidth  > xRawPos)
         break;
      xPos += nCharWidth;
      }

   if (bRoundUp)     // Round up to the next character if closer to it.
      if (xRawPos - xPos > (xPos + nCharWidth) - xRawPos)
         ++iChar;

   return min (iChar, nLimit);
}

//--------------------------------------------------------------------------//

int LeftOf (LPEDITFIELD lpField, int nChar)

{
   int   xStart;
   int   xPos;

   xStart = EditGetStartXPos(lpField);

// 96' 6/26 Modified by BOC Gao. for processing DBCS char.
   xPos = xStart;
   if (nChar)
   {
      if ( IsCharDbcs(lpField, nChar - 1) == 1 )
	  {
		 xPos += GetDBCSWidth(lpField, lpField->nLeftChar, nChar - 1 - lpField->nLeftChar);
         xPos += GetDBCSWidth(lpField, nChar - 1, 2) / 2 + 1;
	  }
	  else 
		 xPos += GetDBCSWidth(lpField, lpField->nLeftChar, nChar - lpField->nLeftChar);
   }

   return xPos;
}

//--------------------------------------------------------------------------//

int RightOf (LPEDITFIELD lpField, int nChar)

{
   int   xStart;
   int   xPos;

   xStart = EditGetStartXPos(lpField);

// 96' 6/26 Modified by BOC Gao. for processing DBCS char.
   xPos = xStart;
   if ( nChar)
   {
      if ( IsCharDbcs(lpField, nChar - 1) == 1 )
	  {
		 xPos += GetDBCSWidth(lpField, 0, nChar - 1);
         xPos += GetDBCSWidth(lpField, nChar - 1, 2) / 2 + 1;
	  }
	  else 
		 xPos += GetDBCSWidth(lpField, 0, nChar);
    }

	{
		int Kanji = IsCharDbcs(lpField, nChar);
		if ( Kanji == 1 )
			xPos += GetDBCSWidth(lpField, nChar, 2) / 4;
		else if ( Kanji == 2 )
			xPos += GetDBCSWidth(lpField, nChar - 1, 2) / 4;
		else
#ifdef SPREAD_JPN
			xPos += GetDBCSWidth(lpField, nChar, 1) / 2;
#else
			xPos += GetDBCSWidth(lpField, nChar, 1);
#endif
	}

   return xPos;
}

//--------------------------------------------------------------------------//

int GetSingleCharWidth (LPEDITFIELD lpField, TCHAR cCharacter)

{
   if (lpField->fPassWord && cCharacter != ' ')
      cCharacter = '*';

   return lpField->nCharWidthTable[(BYTE)cCharacter - ' '];
}

//--------------------------------------------------------------------------//

//--------------------------------------------------------------------------
// The parameter count changed for this function. Check the declaration
// for more details. (Masanori Iwasa)
//--------------------------------------------------------------------------
BOOL CheckFit (LPEDITFIELD lpField, TCHAR cChar)
{
if (!lpField->fAutoHScroll)
	{
	RECT EditRect;
   int  nChar = cChar;
   int  xWidth;

#ifndef _UNICODE
   if (lpField->KType)
      {
      lpField->KType = FALSE;
      xWidth = lpField->nMaxCharWidth / 2;
      }
   else if (_TIsDBCSLeadByte((BYTE)nChar))
      {
      lpField->KType = TRUE;
      xWidth = lpField->nMaxCharWidth;
      }
   else
#endif
      xWidth = GetSingleCharWidth(lpField, cChar);

	EditGetEditRect(lpField, &EditRect);
	if (xWidth + FieldLength(lpField, 0) > EditRect.right)
		return (FALSE);
	}

if (lpField->nMaxChars != -1 && lpField->nChars >= lpField->nMaxChars)
   return (FALSE);

return (TRUE);
}


//--------------------------------------------------------------------------------
// Checks to see if the specified location happens to be on a DBCS first byte.
// (Masanori Iwasa)
//--------------------------------------------------------------------------------
int IsCharDbcs ( LPEDITFIELD lpField, int Last )
{
#ifndef _UNICODE
   int   i;
   int   nChar;

   for ( i = 0; i <= Last; i++ )
      {
      nChar = (BYTE)lpField->lpszString[i];
      if ( _TIsDBCSLeadByte((BYTE)nChar) )
         if ( i >= Last - 1 ) 
            return Last - i + 1;
         else
            i++;
      }
#endif

   return FALSE;
}

// 96' 6/25 Added by BOC MASA & Gao. 
// - for get DBCS width
int GetDBCSWidth(LPEDITFIELD lpField, int nStart, int nLength)
{
#ifdef WIN32
    SIZE	nSize;
#endif
	int   nCnt;
	HFONT hOldFont;
	HDC   hDC;

	if (nLength <= 0)
		return (0);

	hDC = GetDC(lpField->hWnd);

	hOldFont = SelectObject(hDC, lpField->hFont);
#ifdef WIN32
	GetTextExtentPoint(hDC, lpField->lpszString + nStart, nLength, &nSize);
	nCnt = (int)nSize.cx;
#else
	nCnt = (int)GetTextExtent(hDC, lpField->lpszString + nStart, nLength);
#endif

	SelectObject(hDC, hOldFont);

	ReleaseDC(lpField->hWnd, hDC);

	return nCnt;
}
// --------------------------------<<


