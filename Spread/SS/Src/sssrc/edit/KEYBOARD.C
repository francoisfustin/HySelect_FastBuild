/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/KEYBOARD.C_V  $
 * 
 *    Rev 1.2   10 Aug 1990 15:37:54   Randall
 * Added processing of TBG_CLEARFIELD
 * 
 * 
 *    Rev 1.1   12 Jun 1990 14:08:12   Dirk
 * No change.
 * 
 *    Rev 1.0   28 May 1990 13:50:38   Randall
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
#define  NOMINMAX

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <tchar.h>
#include <toolbox.h>
#include <stdlib.h>
#include "..\libgen.h"
#include "editfld.h"

//RWP01a
#ifdef SPREAD_JPN
extern void SS_BeepLowlevel(HWND hWnd);
#else
extern void SS_Beep2(HWND hWnd);
#endif
//RWP01a

//--------------------------------------------------------------------------//

TCHAR ProcessCharacter(HWND hWnd, WPARAM wParam)

{
LPEDITFIELD lpField;
TCHAR       cChar;
TCHAR       szNewText[2];

if (wParam == 3) // <ctrl>-C
   ClipboardCopy(hWnd);

#if 0
if (wParam >= ' ' && wParam <= '~') // Printable character.
#endif
//if (wParam >= ' ' && wParam < N_PRINTABLE_CHARS) // Printable character.
if (wParam >= ' ') // Printable character.
   {
   lpField = LockField (hWnd);

   if (lpField->nCase == UPPERCASE)       // Force case?
      #if defined(_WIN64) || defined(_IA64)
      cChar = (TCHAR)CharUpper((LPTSTR)MAKELONG_PTR(wParam, 0));
      #elif defined(WIN32)
      cChar = (TCHAR)CharUpper((LPTSTR)MAKELONG(wParam, 0));
      #else
      cChar = (TCHAR)(long)AnsiUpper((LPTSTR)MAKELONG(wParam, 0));
      #endif
   else if (lpField->nCase == LOWERCASE)
      #if defined(_WIN64) || defined(_IA64)
      cChar = (TCHAR)CharLower((LPTSTR)MAKELONG_PTR(wParam, 0));
      #elif defined(WIN32)
      cChar = (TCHAR)CharLower((LPTSTR)MAKELONG(wParam, 0));
      #else
      cChar = (TCHAR)(long)AnsiLower((LPTSTR)MAKELONG(wParam, 0));
      #endif
   else
      cChar = (TCHAR)wParam;

   if (GetOverstrikeMode (hWnd, LOCAL))     // Overstrike mode
      {
/*    RFW - 5/13/02 - RUN_CEL_005_008
      if (lpField->bHighlighted) DeselectText (hWnd, lpField);
      if (lpField->iCurrentPos < lpField->nChars)
         ReplaceCharacter (hWnd, lpField, cChar);
*/
      if (lpField->iCurrentPos < lpField->nChars)
			{
	      if (lpField->bHighlighted)
				{
				szNewText[0] = cChar;
				szNewText[1] = 0;
				ReplaceSelectedText (hWnd, lpField, szNewText);
				DeselectText (hWnd, lpField);
				}
			else
				ReplaceCharacter (hWnd, lpField, cChar);
			}
      else
         {
         if (EditGetHAlign(lpField) == EDIT_HALIGN_RIGHT)
#ifdef SPREAD_JPN
// 96' 6/26 Modified by BOC Gao. changed MessageBeep to SS_BeepLowlevel
            SS_BeepLowlevel(hWnd);
// ------------------------<<
#else

//RWP01c
//---------*---------
//          MessageBeep (MB_OK);    // No you don't!
//---------*---------
			SS_Beep2(hWnd);			// No you don't!
//RWP01c

#endif
         else
            InsertCharacter (hWnd, lpField, cChar);
         }
      }

   else                 // Insert mode
      {
      if (lpField->bHighlighted)             // SubField currently selected.
         {
         szNewText[0] = cChar;
         szNewText[1] = 0;
         ReplaceSelectedText (hWnd, lpField, szNewText);
         }
      else
         InsertCharacter (hWnd, lpField, cChar);
      }

   if (!IsWindow(hWnd))
      return (0);

   UnlockField (hWnd);
   }

else                       // Unprintable!
   cChar = (TCHAR) wParam;

return (cChar);
}


WORD ProcessKeystroke (HWND hWnd, WPARAM wParam)
{
LPEDITFIELD lpField;
WORD        wKey;
LONG        lSel;
TCHAR       lpszLocal[MAXFIELD + 1];

wKey = (WORD)wParam;

lpField = LockField(hWnd);

if (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'x' || wParam == 'X'))
   ClipboardCut(hWnd);

else if (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'c' || wParam == 'C'))
   ClipboardCopy(hWnd);

else if (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'v' || wParam == 'V'))
   ClipboardPaste(hWnd);

else
   {
   switch (wKey)
      {
      case VK_LEFT:
      case VK_UP:
         if (lpField->iCurrentPos > 0)
            LeftCaret (hWnd, lpField);

         if (IsShiftDown (hWnd))
            ShiftMove (hWnd);
         else
            DeselectText (hWnd, lpField);

         break;

      case VK_RIGHT:
      case VK_DOWN:
         if (lpField->iCurrentPos < lpField->nChars)
            RightCaret (hWnd, lpField);

         if (IsShiftDown (hWnd))
            ShiftMove (hWnd);
         else
            DeselectText (hWnd, lpField);

         break;

      case VK_HOME:
         HomeCaret (hWnd, lpField);

         if (IsShiftDown (hWnd))
            ShiftMove (hWnd);
         else
            DeselectText (hWnd, lpField);

         break;

      case VK_END:
         EndCaret (hWnd, lpField);

         if (IsShiftDown (hWnd))
            ShiftMove (hWnd);
         else
            DeselectText (hWnd, lpField);

         break;

      case VK_BACK:
         lpField->bProcessingBackspace = TRUE;

         if (GetOverstrikeMode (hWnd, LOCAL))   // Like VK_LEFT but ignore control key
            {
            if (lpField->iCurrentPos > 0)
               LeftCaret (hWnd, lpField);

            if (IsShiftDown (hWnd))
               ShiftMove (hWnd);
            else
               DeselectText (hWnd, lpField);
            }

         else if (lpField->bHighlighted)
            {
            DeleteSelectedText (hWnd, lpField);
            }

         else if (lpField->iCurrentPos > 0)
            {
            StrCpy (lpszLocal, lpField->lpszString);
            StrDeleteChar (lpszLocal, lpField->iCurrentPos - 1);
            if (ValidateText(hWnd, lpszLocal, lpField->iCurrentPos))
               {
               StrCpy (lpField->lpszString, lpszLocal);
               lpField->nChars = lstrlen(lpField->lpszString);
               NotifyParent (hWnd, EN_UPDATE);
               LeftCaret (hWnd, lpField);
               InvalidateRect (hWnd, NULL, TRUE);
               UpdateWindow(hWnd);
               EditFieldModifed(hWnd);
               NotifyParent (hWnd, EN_CHANGE);
               }
            }

         lpField->bProcessingBackspace = FALSE;

         break;

      case VK_DELETE:
         lpField->bProcessingDelete = TRUE;

         if (IsShiftDown(hWnd))
            ClipboardCut(hWnd);

         else
            {
            if (GetOverstrikeMode (hWnd, LOCAL))
               {
               if (lpField->bHighlighted)
                  {
                  lSel = (LONG) SendMessage(hWnd, EM_GETSEL, 0, 0l);
                  if (LOWORD(lSel) == 0 && (HIWORD(lSel)) >=
                      (WORD)lpField->nChars)
                     DeleteSelectedText (hWnd, lpField);
                  else
                     {
                     DeselectText (hWnd, lpField);
                     #ifdef SPREAD_JPN
                     // 96' 6/26 Modified by BOC Gao. changed MessageBeep to SS_BeepLowlevel
                     SS_BeepLowlevel(hWnd);
                     // ------------------------<<
                     #else
                     //RWP01c
                     //---------*---------
                     //MessageBeep (MB_OK);    // You can't do that 'ere, mate.
                     //---------*---------
                     SS_Beep2(hWnd);		 // You can't do that 'ere, mate.
                     //RWP01c
                     #endif
                     }
                  }

               else
                  #ifdef SPREAD_JPN
                  // 96' 6/26 Modified by BOC Gao. changed MessageBeep to SS_BeepLowlevel
                  SS_BeepLowlevel(hWnd);
                  // ------------------------<<
                  #else
                  //RWP01c
                  //---------*---------
                  //                MessageBeep (MB_OK);    // You can't do that 'ere, mate.
                  //---------*---------
                  SS_Beep2(hWnd);		  // You can't do that 'ere, mate.
                  //RWP01c
                  #endif

               }

            else if (lpField->bHighlighted)
               {
               DeleteSelectedText (hWnd, lpField);
               }

            else if (lpField->iCurrentPos < lpField->nChars)
               {
               StrCpy (lpszLocal, lpField->lpszString);
               StrDeleteChar (lpszLocal, lpField->iCurrentPos);
               if (ValidateText(hWnd, lpszLocal, lpField->iCurrentPos))
                  {
                  StrCpy (lpField->lpszString, lpszLocal);
			         lpField->nChars = StrLen(lpField->lpszString);
                  NotifyParent (hWnd, EN_UPDATE);
                  InvalidateRect (hWnd, NULL, TRUE);
                  UpdateWindow(hWnd);
                  EditFieldModifed(hWnd);
                  NotifyParent (hWnd, EN_CHANGE);
                  }
               }
            }

         lpField->bProcessingDelete = FALSE;

         break;

      case VK_INSERT:
         if (IsShiftDown(hWnd))
            ClipboardPaste(hWnd);

         else if (HIBYTE(GetKeyState(VK_CONTROL)))
            ClipboardCopy(hWnd);

         else
            {
            if (lpField->bHighlighted)
               DeselectText (hWnd, lpField);
            if (GetOverstrikeMode (hWnd, GLOBAL))
               SetInsertMode (hWnd, GLOBAL);
            else
               SetOverstrikeMode (hWnd, GLOBAL);
            }

         break;

      case VK_SHIFT:
         ShiftDown (hWnd);
         break;

      default:
         // Check for signal to clear the field
         if (Edit_IsActionKey(SS_KBA_CLEAR, wKey))
            {
            EditFieldModifed(hWnd);
            SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)_TEXT(""));
            SendMessage(hWnd, EM_SETMODIFY, TRUE, 0L);
            }

         break;
      }
   }

UnlockField(hWnd);
return (wKey);
}


BOOL SendTextToClipboard(hWnd, lpszText, dLen)

HWND    hWnd;
LPCTSTR lpszText;
short   dLen;
{
HANDLE hMem;
LPTSTR lpszMem;
BOOL   fRet = FALSE;

if (hMem = GlobalAlloc(GHND, (dLen + 1) * sizeof(TCHAR)))
   {
   lpszMem = (LPTSTR)GlobalLock(hMem);
   MemCpy(lpszMem, lpszText, (WORD)(dLen * sizeof(TCHAR)));
   GlobalUnlock(hMem);

   OpenClipboard(hWnd);
   EmptyClipboard();

   #if defined(UNICODE) || defined(_UNICODE)
   SetClipboardData(CF_UNICODETEXT, hMem);
   #else
   SetClipboardData(CF_TEXT, hMem);
   #endif
   CloseClipboard();

   fRet = TRUE;
   }

return (fRet);
}


HANDLE GetTextFromClipboard(hWnd)

HWND   hWnd;
{
HANDLE hMem;
HANDLE hText = 0;
LPTSTR lpszMem;
LPTSTR lpszText;

OpenClipboard(hWnd);

#if defined(UNICODE) || defined(_UNICODE)
if (hMem = GetClipboardData(CF_UNICODETEXT))
#else
if (hMem = GetClipboardData(CF_TEXT))
#endif
   {
   lpszMem = (LPTSTR)GlobalLock(hMem);

   if (lstrlen(lpszMem))
      if (hText = GlobalAlloc(GHND, (lstrlen(lpszMem) + 1) * sizeof(TCHAR)))
         {
         lpszText = (LPTSTR)GlobalLock(hText);
         StrCpy(lpszText, lpszMem);
         GlobalUnlock(hText);
         }

   GlobalUnlock(hMem);
   }

CloseClipboard();

return (hText);
}


WORD ProcessKeyUp(HWND hWnd, WPARAM wParam)
{
LPEDITFIELD lpField;
WORD        wKey;

wKey = (WORD)wParam;

lpField = LockField (hWnd);

switch (wKey)
   {
   case VK_SHIFT:
      ShiftUp (hWnd);
      break;

   default:
      break;
   }

UnlockField(hWnd);
return (wKey);
}


void ClipboardCopy(HWND hWnd)
{
LPEDITFIELD lpField;

lpField = LockField(hWnd);

if (lpField->bHighlighted)
   SendTextToClipboard(hWnd, &lpField->lpszString[lpField->iHiStart],
                       (short)(lpField->iHiEnd - lpField->iHiStart));

UnlockField (hWnd);
}


void ClipboardCut(HWND hWnd)
{
LPEDITFIELD lpField;

lpField = LockField (hWnd);

if (lpField->bHighlighted)
   if (SendTextToClipboard(hWnd, &lpField->lpszString[lpField->iHiStart],
                           (short)(lpField->iHiEnd - lpField->iHiStart)))
      DeleteSelectedText(hWnd, lpField);

UnlockField (hWnd);
}


void ClipboardPaste(HWND hWnd)
{
LPEDITFIELD  lpField;
GLOBALHANDLE hText;
LPTSTR       lpszText;

lpField = LockField (hWnd);

if (hText = GetTextFromClipboard(hWnd))
   {
   lpszText = (LPTSTR)GlobalLock(hText);

   if (lpField->bHighlighted)
      ReplaceSelectedText(hWnd, lpField, lpszText);
   else
      InsertText(hWnd, lpField, lpszText);

   GlobalUnlock(hText);
   GlobalFree(hText);
   }

if (IsWindow(hWnd))
   UnlockField (hWnd);
}


BOOL Edit_IsActionKey(WORD wAction, WORD wKey)
{
BOOL fRet = FALSE;

if (wAction < SS_KBA_CNT)
   {
   long lValue;

   switch (wAction)
      {
      case SS_KBA_CLEAR:
         GetToolBoxGlobal(TBG_CLEARFIELD, &lValue);
         break;

      case SS_KBA_CURRENT:
         GetToolBoxGlobal(TBG_CURRENTDATETIME, &lValue);
         break;

      case SS_KBA_POPUP:
         GetToolBoxGlobal(TBG_POPUPCALENDAR, &lValue);
         break;
      }

   if ((wKey == 0 && lValue) || wKey == (WORD)lValue)
      fRet = TRUE;
   }

return (fRet);
}
