#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editpic.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "editpic\editpic.h"
<-BORLAND*/
#endif

//#ifdef  BUGS
// Bug-001
extern void SS_BeepLowlevel(HWND hwnd);
//#endif

static LPPICFIELD PicLockField(HWND);
static void       PicUnlockField(HWND);
static void       PicFreeField(HWND);
static BOOL       PicIsFmtMask(LPCTSTR szMask, short iPos);
static BOOL       PicIsMaskChar(WORD Mask);
static WORD       PicValidateKey(HWND hWnd, LPPICFIELD lpFieldPic,
                                 LPCTSTR szMask, short iPos, WORD Key);

#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// Thare are 2 functions we changed for handling DBCS.
// 1. We add iPos parameter to know whether the iPos position of the lpStr is KANJI 
//    lead byte or not. 
// 2. We changed char to WORD because one KANJI has two bytes.
// Charles
static WORD       PicxValidateKey(HWND hWnd, LPPICFIELD lpFieldPic,
                                  WORD wMask, WORD Key);
static void       PicReplaceCharacter(HWND hWnd, LPEDITFIELD lpField,
                                      TCHAR cChar);
#else
static WORD       PicxValidateKey(HWND hWnd, LPPICFIELD lpFieldPic,
                                  WORD wMask, WORD Key, LPCTSTR lpStr, int iPos);
static void       PicReplaceCharacter(HWND hWnd, LPEDITFIELD lpField,
                                      WORD wParam);
#endif

static short      PicGetMaskLen(LPCTSTR szMask);
static WORD       PicGetMaskChar(LPCTSTR szMask, short iPos);
static void       PicClearHighlight(HWND hWnd, LPTSTR lpszText);
#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE) // Added by BOC FMH
static void       PicReplaceText(HWND hWnd, LPEDITFIELD lpField,
                                 LPCTSTR lpszText, short dPosStart,
                                 short dPosEnd);
#endif // Added by BOC FMH

#if  defined(SPREAD_JPN) && !( defined(UNICODE) || defined(_UNICODE) )
// There are 2 functions we added  for handling DBCS.
// 1. Whether the Pos position of the lpstr string is the lead byte of KANJI or not.
// 2. Overwrite a character(SBC or DBC) with Byte1 and Byte2 at the Pos position of
//    lpstr string. If Byte2 is zero, Byte1 is a SBC.
// Charles
static int        IsKanJiType(LPCTSTR lpstr,int Pos);
static void       OverWriteStringAtCurrentPos(LPTSTR lpstr,int Pos,
                                              BYTE Byte1,BYTE Byte2);
#endif

BOOL PicCreateField(HWND hWnd, LPARAM lParam)
{
LPPICFIELD lpField;
HANDLE     hPic;
BOOL       fRet = FALSE;
LONG       lStyle;

if (!(hPic = GlobalAlloc(GHND, sizeof(PICFIELD))))
   NotifyParent(hWnd, EN_ERRSPACE);

else
   {
   if (lpField = (LPPICFIELD)GlobalLock(hPic))
      {
      SetDefWndProc(hWnd, GetTBEditClassProc());
      SetWindowPicField(hWnd, hPic);

      lStyle = ((LPCREATESTRUCT)lParam)->style |= ES_OVERSTRIKE;

      GlobalUnlock(hPic);
      fRet = TRUE;
      }
   else
      NotifyParent(hWnd, EN_ERRSPACE);
   }

return (fRet);
}


void PicInitializeField(HWND hWnd, LPARAM lParam)
{
LPPICFIELD lpField;

lpField = PicLockField(hWnd);

SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), (LPARAM)TRUE);
PicSetMask(hWnd, (LPTSTR)((LPCREATESTRUCT)lParam)->lpszName);

PicUnlockField(hWnd);
}


void PicSetMask(HWND hWnd, LPCTSTR szMaskNew)
{
LPPICFIELD   lpField;
GLOBALHANDLE hMaskTemp;
LPTSTR       szMask;
LPTSTR       szMaskTemp;
LPTSTR       lpPtr;
LPTSTR       lpszDefText = 0;
LPTSTR       lpszDefTextNew = 0;
LPTSTR       lpszSave = 0;
long         Len;
long         lMaskLen = 0;
TCHAR        cSave;
short        iCaretPos = 0;
short        i;

lpField = PicLockField(hWnd);

if (lpField->hMask)
   {
   GlobalFree(lpField->hMask);
   lpField->hMask = 0;
   }

if (lpField->hDefText)
   {
   GlobalFree(lpField->hDefText);
   lpField->hDefText = 0;
   }

if (szMaskNew)
   {
   if (lpszDefTextNew = StrpBrk(szMaskNew, _TEXT("\t\n")))
      {
      cSave = *lpszDefTextNew;
      lpszSave = lpszDefTextNew;
      *lpszDefTextNew = '\0';
      lpszDefTextNew++;

		lMaskLen = PicGetMaskLen(szMaskNew);
      if (lMaskLen > lstrlen(lpszDefTextNew))
         {
         *lpszSave = cSave;
         PicUnlockField(hWnd);
         return;
         }
      }

   Len = lstrlen(szMaskNew);
   if (lstrlen(szMaskNew) >= MAXFIELD)
      Len = min(lstrlen(szMaskNew), MAXFIELD - 2);

   if (lpField->hMask = GlobalAlloc(GHND, (Len + 1) * sizeof(TCHAR)))
      {
      szMask = (LPTSTR)GlobalLock(lpField->hMask);
      MemCpy(szMask, szMaskNew, (WORD)(Len * sizeof(TCHAR)));

      /***********
      * Set Text
      ***********/

      if (lpszDefTextNew)
         {
         Len = lMaskLen;
         if (lstrlen(lpszDefTextNew) >= MAXFIELD)
            Len = min(lstrlen(lpszDefTextNew), MAXFIELD - 2);

         if (lpField->hDefText = GlobalAlloc(GHND, (Len + 1) * sizeof(TCHAR)))
            {
            lpszDefText = (LPTSTR)GlobalLock(lpField->hDefText);
            MemCpy(lpszDefText, lpszDefTextNew, (WORD)(Len * sizeof(TCHAR)));
            SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)lpszDefText);
            GlobalUnlock(lpField->hDefText);
            }
         }

      else if (hMaskTemp = GlobalAlloc(GHND, (lstrlen(szMask) + 1)
                                             * sizeof(TCHAR)))
         {
         szMaskTemp = (LPTSTR)GlobalLock(hMaskTemp);

         /*
         for (i = 0; i < PicGetMaskLen(szMask); i++)
            if (PicIsFmtMask(szMask, i))
               szMaskTemp[i] = ' ';
            else
               szMaskTemp[i] = PicGetMaskChar(szMask, i);
         */

         for (i = 0, lpPtr = szMask; *lpPtr; )
            {
            if (*lpPtr == PICFMT_CONTROL && (PicIsMaskChar(*(lpPtr + 1)) ||
                *(lpPtr + 1) == PICFMT_CONTROL))
               {
               lpPtr++;
               szMaskTemp[i++] = *lpPtr;
               }

            else if (PicIsMaskChar(*lpPtr))
               szMaskTemp[i++] = ' ';
            else
               szMaskTemp[i++] = *lpPtr;

//#ifdef  SPREAD_JPN
// We changed for handling DBCS. Charles
// We can get rid of this preprocessor.
            if (_TIsDBCSLeadByte((BYTE)*lpPtr))
                {
                    lpPtr++;
                    szMaskTemp[i++] = *lpPtr;
                }
//#endif
            if (*lpPtr)
                lpPtr++;
            }

         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szMaskTemp);
         GlobalUnlock(hMaskTemp);
         GlobalFree(hMaskTemp);
         }

      /*
      while (iCaretPos < PicGetMaskLen(szMask) - 1 &&
             !PicIsFmtMask(szMask, iCaretPos))
         iCaretPos++;
      */
      for (lpPtr = szMask; *lpPtr; )
         {
         if (*lpPtr == PICFMT_CONTROL)
            lpPtr++;
         else if (PicIsMaskChar(*lpPtr))
            break;
         else
            iCaretPos++;

//#ifdef  SPREAD_JPN
// We changed for handling DBCS. Charles
// We can get rid of this preprocessor.
         if (_TIsDBCSLeadByte((BYTE)*lpPtr))
            lpPtr++;
//#endif
         if (*lpPtr)
            lpPtr++;
         }

      Edit_SetSel(hWnd, 0, 0);
      SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);

      if (lpszSave)
         *lpszSave = cSave;

      GlobalUnlock(lpField->hMask);
      }

   else
      NotifyParent(hWnd, EN_ERRSPACE);
   }
else
   {
   SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)"");
   SendMessage(hWnd, EM_SETCARETPOS, 0, 0L);
   }

PicUnlockField(hWnd);
}


BOOL PicSetText(HWND hWnd, LPCTSTR lpszText, LPTSTR lpszTextFmt, BOOL fSetValue)
{
LPPICFIELD lpField;
BOOL       Ret = FALSE;

lpField = PicLockField(hWnd);

Ret = PicFmtText(hWnd, lpField->hDefText, lpField->hMask, lpszText,
                 lpszTextFmt, fSetValue);

PicUnlockField(hWnd);
return (Ret);
}


BOOL DLLENTRY PicFmtText(HWND hWnd, GLOBALHANDLE hDefText, GLOBALHANDLE hMask,
                         LPCTSTR lpszText, LPTSTR lpszTextFmt, BOOL fSetValue)
{
LPTSTR      lpMask;
LPTSTR      lpDefText = NULL;
LPTSTR      lpPtr;
LPPICFIELD  lpField = 0;
WORD        wKey;
BOOL        Ret = FALSE;
BOOL        fValid = TRUE;
short       At;
short       i;
TCHAR       szMask[MAXFIELD + 1];

_fmemset(szMask, '\0', sizeof(szMask));

if (hWnd)
   lpField = PicLockField(hWnd);

if ((!lpszText || !(*lpszText)) && hMask)
   {
   if (hDefText)
      {
      lpMask = (LPTSTR)GlobalLock(hDefText);

      if (lstrlen(lpMask))
         {
         if (lpszTextFmt)
            StrnCpy(lpszTextFmt, lpMask, MAXFIELD);
         else
            SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)lpMask);

         Ret = TRUE;
         }

      GlobalUnlock(hDefText);
      }

   else
      {
      lpMask = (LPTSTR)GlobalLock(hMask);

      if (lstrlen(lpMask))
         {
         for (i = 0, lpPtr = lpMask; *lpPtr; )
            {
            if (*lpPtr == PICFMT_CONTROL && (PicIsMaskChar(*(lpPtr + 1)) ||
                *(lpPtr + 1) == PICFMT_CONTROL))
               {
               lpPtr++;
               szMask[i++] = *lpPtr;
               }

            else if (PicIsMaskChar(*lpPtr))
               szMask[i++] = ' ';
            else
               szMask[i++] = *lpPtr;

//#ifdef  SPREAD_JPN
// We changed for handling DBCS. Charles
// We can get rid of this preprocessor.
            if (_TIsDBCSLeadByte((BYTE)*lpPtr))
              {
               lpPtr++;
               szMask[i++] = *lpPtr;
              }
//#endif
            if (*lpPtr)
               lpPtr++;
            }

         if (lpszTextFmt)
            StrnCpy(lpszTextFmt, szMask, MAXFIELD);
         else
            SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szMask);

         Ret = TRUE;
         }
      else
         CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, WM_SETTEXT, 0,
                        (LPARAM)(LPCTSTR)"");

      GlobalUnlock(hMask);
      }

   if (hWnd)
      PicProcessKey(hWnd, VK_HOME, FALSE);
   }

else
   {
   if (hMask)
      {
      lpMask = (LPTSTR)GlobalLock(hMask);
      if (hDefText)
         lpDefText = (LPTSTR)GlobalLock(hDefText);

      if (fSetValue)
         {
         for (i = 0, At = 0, lpPtr = lpMask; *lpPtr; i++)
            {
            if (hDefText)
               szMask[i] = lpDefText[i];
            else
               szMask[i] = *lpPtr;

            if (*lpPtr == PICFMT_CONTROL && (PicIsMaskChar(*(lpPtr + 1)) ||
                *(lpPtr + 1) == PICFMT_CONTROL))
               {
               lpPtr++;
               if (!hDefText)
                  szMask[i] = *lpPtr;
               }
            else if (PicIsMaskChar(*lpPtr))
               {
               if (At >= lstrlen(lpszText))
                  szMask[i] = ' ';

               else
                  {
#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// We changed for handling DBCS, also pass the current 
// position in the string to the function for checking.
// Charles
                  if (!PicxValidateKey(hWnd, lpField, *lpPtr, lpszText[At]) &&
#else
                  if (!PicxValidateKey(hWnd, lpField, *lpPtr, lpszText[At], lpszText, At) &&
#endif
                      ((hDefText && lpszText[At] != lpDefText[i]) ||
                      (!hDefText && lpszText[At] != ' ')))
                     {
                     fValid = FALSE;
                     break;
                     }

                  else
                     szMask[i] = lpszText[At];

                  At++;
                  }
               }

//#ifdef  SPREAD_JPN
// We changed for handling DBCS. Charles
// We can get rid of this preprocessor.
            if (_TIsDBCSLeadByte((BYTE)*lpPtr))
             {
              lpPtr++;
              i++;
              if (hDefText)
                 szMask[i] = lpDefText[i];
              else
                 szMask[i] = *lpPtr;
             }
//#endif
            if (*lpPtr)
              lpPtr++;
            }

         lpszText = szMask;
         }

      else if (lstrlen(lpMask) == 0)
         fValid = FALSE;

      else
         /*
         for (i = 0, lpPtr = lpMask; i < lstrlen(lpszText) && *lpPtr; i++)
            {
            if (*lpPtr == PICFMT_CONTROL)
               lpPtr++;

            else if (PicIsMaskChar(*lpPtr))
               {
               if (!PicxValidateKey(hWnd, lpField, *lpPtr, lpszText[i]))
                  {
                  if ((hDefText && lpszText[i] != lpDefText[i]) ||
                      (!hDefText && lpszText[i] != ' '))
                     {
                     fValid = FALSE;
                     break;
                     }
                  }
               }

            else if (*lpPtr != lpszText[i])
               {
               fValid = FALSE;
               break;
               }

            if (*lpPtr)
               lpPtr++;
            }
         */
         {
         for (i = 0, lpPtr = lpMask; *lpPtr; i++)
            {
            if (hDefText)
               szMask[i] = lpDefText[i];
            else
               szMask[i] = *lpPtr;

            if (*lpPtr == PICFMT_CONTROL && (PicIsMaskChar(*(lpPtr + 1)) ||
                *(lpPtr + 1) == PICFMT_CONTROL))
               {
               lpPtr++;
               if (!hDefText)
                  szMask[i] = *lpPtr;
               }
            else if (PicIsMaskChar(*lpPtr))
               {
               if (i >= lstrlen(lpszText))
                  szMask[i] = ' ';

               else
                  {

#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// We changed for handling DBCS, also pass the current position
// to the fuction for checking.
// Charles
                  if (!(wKey = PicxValidateKey(hWnd, lpField, *lpPtr,
                     lpszText[i])) &&
#else
                  if (!(wKey = PicxValidateKey(hWnd, lpField, *lpPtr,
                        lpszText[i], lpszText, i)) &&
#endif
                      ((hDefText && lpszText[i] != lpDefText[i]) ||
                      (!hDefText && lpszText[i] != ' ')))
                     {
                     fValid = FALSE;
                     break;
                     }

                    else if (wKey == 0 && lpszText[i] == ' ')
                      szMask[i] = ' ';

                  else
                     szMask[i] = (TCHAR)wKey;
                  }
               }

//#ifdef  SPREAD_JPN
// We changed for handling DBCS. Charles
// We can get rid of this preprocessor.
            if (_TIsDBCSLeadByte((BYTE)*lpPtr))
             {
              lpPtr++;
              i++;
              if (hDefText)
                 szMask[i] = lpDefText[i];
              else
                 szMask[i] = *lpPtr;
             }
//#endif
            if (*lpPtr)
              lpPtr++;
            }

         lpszText = szMask;
         }

      if (hDefText)
         GlobalUnlock(hDefText);

      GlobalUnlock(hMask);
      }
   else
      lpszText = _T("");

   if (fValid)
      Ret = TRUE;

   if (lpszTextFmt)
      {
      if (fValid)
         StrnCpy(lpszTextFmt, lpszText, MAXFIELD);
      }
   else
      {
      if (!fValid)
         NotifyParent(hWnd, EN_INVALIDSETTEXT);
      else
         CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, WM_SETTEXT, 0,
                        (LPARAM)lpszText);
      }
   }

if (hWnd)
   PicUnlockField(hWnd);

return (Ret);
}


BOOL PicGetMask(HWND hWnd, LPTSTR szMask, WORD Len)
{
LPPICFIELD  lpField;
LPTSTR      szMaskTemp;
BOOL        Ret = FALSE;

lpField = PicLockField(hWnd);

if (lpField->hMask)
   {
   szMaskTemp = (LPTSTR)GlobalLock(lpField->hMask);

   if (Len == 0)
      lstrcpy(szMask, szMaskTemp);
   else
      StrnCpy(szMask, szMaskTemp, Len);

   GlobalUnlock(lpField->hMask);
   Ret = TRUE;
   }

PicUnlockField(hWnd);
return (Ret);
}


void PicDestroyField(HWND hWnd)
{
PicFreeField(hWnd);
}


LPPICFIELD PicLockField(HWND hWnd)
{
HANDLE hField;

if (hField = GetWindowPicField(hWnd))
   return ((LPPICFIELD)GlobalLock(hField));
else
   return (NULL);
}


void PicUnlockField(HWND hWnd)
{
HANDLE hField;

if (hField = GetWindowPicField(hWnd))
   GlobalUnlock(hField);
}


void PicFreeField(HWND hWnd)
{
HANDLE     hField;
LPPICFIELD lpField;

if (hField = GetWindowPicField(hWnd))
   {
   lpField = PicLockField(hWnd);

   if (lpField->hMask)
      GlobalFree(lpField->hMask);

   PicUnlockField(hWnd);
   tbMemFree(hField);
   }
}


BOOL PicIsFmtMask(szMask, iPos)

LPCTSTR szMask;
short   iPos;
{
WORD Mask;

if (iPos >= lstrlen(szMask))
   return (0);

#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// We changed for handling '&' which is a KANJI mask character.
// I think we should add a preprocessor here.
// Charles
if ((Mask = PicGetMaskChar(szMask, iPos)) >= 256)
   return (PicIsMaskChar((WORD)(Mask - 256)));
#else
if ((Mask = PicGetMaskChar(szMask, iPos)) >= 256) {
   if(Mask==256)
      return (TRUE);
   if(Mask==257)
      return (TRUE);
   return (PicIsMaskChar((WORD)(Mask - 256)));
}
#endif
return (FALSE);
}


BOOL PicIsMaskChar(Mask)

WORD Mask;
{
Mask = toupper(Mask);

#if  defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// We changed for handling KANJI mask character '&'.
// I think we should add a preprocessor here.
// Charles
if (Mask == PICFMT_NUM || Mask == PICFMT_ALPHA || Mask == PICFMT_ASCII ||
    Mask == PICFMT_ALPHANUM || Mask == PICFMT_LOWER || Mask == PICFMT_UPPER ||
    Mask == PICFMT_HEX || Mask == PICFMT_CALLBACK || Mask == PICFMT_KANJI)
#else
if (Mask == PICFMT_NUM || Mask == PICFMT_ALPHA || Mask == PICFMT_ASCII ||
    Mask == PICFMT_ALPHANUM || Mask == PICFMT_LOWER || Mask == PICFMT_UPPER ||
    Mask == PICFMT_HEX || Mask == PICFMT_CALLBACK)
#endif
   return (TRUE);

return (FALSE);
}


WORD PicValidateKey(hWnd, lpFieldPic, szMask, iPos, Key)

HWND       hWnd;
LPPICFIELD lpFieldPic;
LPCTSTR    szMask;
short      iPos;
WORD       Key;
{
WORD       Mask;

if (Key == '\b')
   return (0);

if ((Mask = PicGetMaskChar(szMask, iPos)) >= 256)

#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// We changed for handling DBCS.  Charles
   return (PicxValidateKey(hWnd, lpFieldPic, (WORD)(Mask - 256), Key));
#else
   return (PicxValidateKey(hWnd, lpFieldPic, (WORD)(Mask - 256), Key, 0l, 0));
#endif

return (0);
}

#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// We changed for handling DBCS. Charles
WORD PicxValidateKey(hWnd, lpFieldPic, wMask, Key)

HWND        hWnd;
LPPICFIELD  lpFieldPic;
WORD        wMask;
WORD        Key;
#else
WORD PicxValidateKey(hWnd, lpFieldPic, wMask, Key, lpszText, iPos)

HWND        hWnd;
LPPICFIELD  lpFieldPic;
WORD        wMask;
WORD        Key;
LPCTSTR     lpszText;
int         iPos;
#endif
{
PICCALLBACK PicCallBack;

if (Key == '\b')
   return (0);

wMask = toupper(wMask);

switch (wMask)
   {
   case PICFMT_NUM:
	   //Modify By BOC 99.6.16 (hyt)------------
	   //for if input DBCS isdigit will generate access violation error
       //if (!isdigit(Key) && Key != '.')
	   //-----------------------------------------
	   if ((Key & 0xff00) || (!isdigit(Key) && Key != '.'))
         Key = 0;
	   

      break;

   case PICFMT_ALPHA:
//GAB 07/28/04 Changed per Okuda's request to allow half wide characters only.
// Note: This was done to Grapecity build only because our help files states:
// A-Z, a-z plus any alphabetic character from larger character sets such as ANSI or Unicode 
#if  defined(SPREAD_JPN) && ( defined(UNICODE) || defined(_UNICODE) )
      if (!IsCharAlpha((TCHAR)Key) || !((0x0041 <= Key && Key <= 0x005A) ||(0x0061 <= Key && Key <= 0x007A)))
#else
	  if (!IsCharAlpha((TCHAR)Key))
#endif
         Key = 0;

      break;

   case PICFMT_ALPHANUM:
//GAB 07/28/04 Changed per Okuda's request to allow half wide characters only
//      if (!IsCharAlphaNumeric((TCHAR)Key) && Key != '.')
      if (!IsCharAlphaNumeric((TCHAR)Key) && Key != '.' || !((0x002E == Key) || (0x0030 <= Key && Key <= 0x0039) || (0x0041 <= Key && Key <= 0x005A) ||(0x0061 <= Key && Key <= 0x007A)))
		  Key = 0;

      break;

#if  defined(SPREAD_JPN) && !( defined(UNICODE) || defined(_UNICODE) )
// We handle '&' KANJI mask here.
// I think we should add a preprocessor here.
// Charles
   case 0:        // the last PICFMT_KANJI
      if(Key!=0x20)
        Key=0;

      break;


   case PICFMT_KANJI:
      if (Key == 0x20) 
         break;

      if (lpszText == 0l) 
      {
         if(!_TIsDBCSLeadByte((BYTE)Key))
           Key = 0;
      }
      else
         if (IsKanJiType(lpszText,iPos)==0) 
            Key = 0;

      break;

   case 1:        // the last 'X'
      if(_TIsDBCSLeadByte((BYTE)Key))
         Key=0;

      break;
#endif

//GAB 08/02/04
#if defined(UNICODE) || defined(_UNICODE)
   case PICFMT_KANJI:
      if (Key == 0x20) 
         break;

	  if ((( 0x21 <= Key && Key <= 0x7e) || (0xff61 <= Key && Key <= 0xff9f)))
           Key = 0;
      break;
#endif

   case PICFMT_ASCII:
      /*
      if (!isascii(Key))
         Key = 0;
      */

      break;

   case PICFMT_LOWER:
//GAB 07/28/04 Changed per Okuda's request to allow half wide characters only
//      if (IsCharAlpha((TCHAR)Key))
      if (IsCharAlpha((TCHAR)Key) && ((0x0041 <= Key && Key <= 0x005A) ||(0x0061 <= Key && Key <= 0x007A)))
         #if defined(_WIN64) || defined(_IA64)
         Key = (WORD)CharLower((LPTSTR)MAKELONG_PTR(Key, 0));
         #elif defined(WIN32)
         Key = (WORD)CharLower((LPTSTR)MAKELONG(Key, 0));
         #else
         Key = (WORD)(long)AnsiLower((LPTSTR)MAKELONG(Key, 0));
         #endif
      else
         Key = 0;

      break;

   case PICFMT_UPPER:
//GAB 07/28/04 Changed per Okuda's request to allow half wide characters only
//      if (IsCharAlpha((TCHAR)Key))
      if (IsCharAlpha((TCHAR)Key) && ((0x0041 <= Key && Key <= 0x005A) ||(0x0061 <= Key && Key <= 0x007A)))
         #if defined(_WIN64) || defined(_IA64)
         Key = (WORD)CharUpper((LPTSTR)MAKELONG_PTR(Key, 0));
         #elif defined(WIN32)
         Key = (WORD)CharUpper((LPTSTR)MAKELONG(Key, 0));
         #else
         Key = (WORD)(long)AnsiUpper((LPTSTR)MAKELONG(Key, 0));
         #endif
      else
         Key = 0;

      break;

   case PICFMT_HEX:
      #if defined(_WIN64) || defined(_IA64)
      if (!((Key >= '0' && Key <= '9') ||
            ((WORD)CharUpper((LPTSTR)MAKELONG_PTR(Key,0)) >= 'A' &&
             (WORD)CharUpper((LPTSTR)MAKELONG_PTR(Key,0)) <= 'F')))
      #elif defined(WIN32)
      if (!((Key >= '0' && Key <= '9') ||
            ((WORD)CharUpper((LPTSTR)MAKELONG(Key,0)) >= 'A' &&
             (WORD)CharUpper((LPTSTR)MAKELONG(Key,0)) <= 'F')))
      #else
      if (!((Key >= '0' && Key <= '9') ||
            ((WORD)(long)AnsiUpper((LPTSTR)MAKELONG(Key,0)) >= 'A' &&
             (WORD)(long)AnsiUpper((LPTSTR)MAKELONG(Key,0)) <= 'F')))
      #endif
         Key = 0;

      break;

   case PICFMT_CALLBACK:
      if (hWnd)
         if (lpFieldPic->lpfnCallBack)
            {
            PicCallBack = (PICCALLBACK)lpFieldPic->lpfnCallBack;

            Key = (*PicCallBack)(hWnd, (WORD)GetDlgCtrlID(hWnd),
                  (short)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L), (short)Key);
            }
         else
            Key = 0;

      break;

   default:
      Key = 0;
   }

return (Key);
}


BOOL PicProcessKey(hWnd, Key, fShift)

HWND         hWnd;
WORD         Key;
BOOL         fShift;
{
LPPICFIELD   lpFieldPic;
LPEDITFIELD  lpField;
short        iCaretPos;
LPCTSTR      szMask;
LPCTSTR      szDefText;
BOOL         Ret = 0;
TCHAR        szText[MAXFIELD + 1];

#if  defined(SPREAD_JPN) && !( defined(UNICODE) || defined(_UNICODE) )
// We added for saving a KANJI's type.
// 1 --- the lead byte of KANJI.
// 2 --- the second byte of KANJI. 
int         CharType;
#endif

lpField = LockField(hWnd);
lpFieldPic = PicLockField(hWnd);
SendMessage(hWnd, WM_GETTEXT, sizeof(szText), (LPARAM)(LPTSTR)szText);
iCaretPos = (short)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);

if ((HIBYTE(GetKeyState(VK_CONTROL)) && (Key == 'v' || Key == 'V')) ||
    (Key == VK_INSERT && fShift))
   {
   PicClipboardPaste(hWnd);
   return (TRUE);
   }

if ((HIBYTE(GetKeyState(VK_CONTROL)) && (Key == 'x' || Key == 'X')) ||
    (Key == VK_DELETE && fShift))
   {
   PicClipboardCut(hWnd);
   return (TRUE);
   }

if (lpFieldPic->hMask)
   {
   szMask = (LPTSTR)GlobalLock(lpFieldPic->hMask);

   switch (Key)
      {
      case VK_LEFT:
      case VK_UP:
      case VK_BACK:
         while (iCaretPos > 0 && !PicIsFmtMask(szMask, --iCaretPos))
            ;

			/* RFW - 6/18/04 - 14581
         if (iCaretPos > 0 || PicIsFmtMask(szMask, iCaretPos))
			*/
         if (iCaretPos > 0 || PicIsFmtMask(szMask, iCaretPos) ||
             (lpField->bHighlighted && Key == VK_BACK))
            {
            if (Key == VK_BACK)
               {
               if (lpField->bHighlighted)
                  PicClearHighlight(hWnd, szText);

					else
						{
#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// We changed for  overwrite a KANJI in a string. Charles
						if (lpFieldPic->hDefText)
							{
							szDefText = (LPTSTR)GlobalLock(lpFieldPic->hDefText);
							szText[iCaretPos] = szDefText[iCaretPos];
							GlobalUnlock(lpFieldPic->hDefText);
							}
						else
							szText[iCaretPos] = ' ';
#else
						if (lpFieldPic->hDefText)
							{
							szDefText = (LPTSTR)GlobalLock(lpFieldPic->hDefText);
							CharType=IsKanJiType(szDefText,iCaretPos);
							switch(CharType){
								case 1:
									OverWriteStringAtCurrentPos(szText,iCaretPos,
											  szDefText[iCaretPos],szDefText[iCaretPos+1]);
									break;
								case 2:
									OverWriteStringAtCurrentPos(szText,iCaretPos-1,
											  szDefText[iCaretPos-1],szDefText[iCaretPos]);
									break;
								default:
									OverWriteStringAtCurrentPos(szText,iCaretPos,szDefText[iCaretPos],0);
								}
							GlobalUnlock(lpFieldPic->hDefText);
							}
						else
							OverWriteStringAtCurrentPos(szText,iCaretPos,' ',0);
#endif
						}
               EditFieldModifed(hWnd);
               SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szText);
               }

            SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);
            }

         if (fShift && Key != VK_BACK)
            ShiftMove(hWnd);
         else
            DeselectText(hWnd, lpField);

         Ret = TRUE;
         break;

      case VK_RIGHT:
      case VK_DOWN:
         while (iCaretPos < PicGetMaskLen(szMask) &&
                !PicIsFmtMask(szMask, ++iCaretPos))
            ;

         SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);

         if (fShift)
            ShiftMove(hWnd);
         else
            DeselectText(hWnd, lpField);

         Ret = TRUE;
         break;

      case VK_HOME:
         iCaretPos = 0;
         while (iCaretPos < PicGetMaskLen(szMask) - 1 &&
                !PicIsFmtMask(szMask, iCaretPos))
            iCaretPos++;

         if (PicIsFmtMask(szMask, iCaretPos))
            SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);

         if (fShift)
            ShiftMove(hWnd);
         else
            DeselectText(hWnd, lpField);

         Ret = TRUE;
         break;

      case VK_END:
         iCaretPos = PicGetMaskLen(szMask);
         /*
         while (iCaretPos > 0 && !PicIsFmtMask(szMask, iCaretPos))
            iCaretPos--;

         if (PicIsFmtMask(szMask, iCaretPos))
         */
            SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);

         if (fShift)
            ShiftMove(hWnd);
         else
            DeselectText(hWnd, lpField);

         Ret = TRUE;
         break;

      case VK_DELETE:
         if (!IsShiftDown(hWnd))
            {
            if (lpField->bHighlighted)
               {
               iCaretPos = lpField->iHiStart;
               PicClearHighlight(hWnd, szText);
               SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szText);
               DeselectText(hWnd, lpField);
               SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0L);
               }

            else if (iCaretPos < PicGetMaskLen(szMask))
               {

#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// We changed for  overwrite a KANJI in a string. Charles
               if (lpFieldPic->hDefText)
                  {
                  szDefText = (LPTSTR)GlobalLock(lpFieldPic->hDefText);
                  szText[iCaretPos] = szDefText[iCaretPos];
                  GlobalUnlock(lpFieldPic->hDefText);
                  }
               else
                  szText[iCaretPos] = ' ';
#else
               if (lpFieldPic->hDefText)
                  {
                  szDefText = (LPTSTR)GlobalLock(lpFieldPic->hDefText);
                  CharType=IsKanJiType(szDefText,iCaretPos);
                  switch(CharType){
                     case 1:
                        OverWriteStringAtCurrentPos(szText,iCaretPos,
                                szDefText[iCaretPos],szDefText[iCaretPos+1]);
                        break;
                     case 2:
                        OverWriteStringAtCurrentPos(szText,iCaretPos-1,
                                szDefText[iCaretPos-1],szDefText[iCaretPos]);
                        break;
                     default:
                        OverWriteStringAtCurrentPos(szText,iCaretPos,szDefText[iCaretPos],0);
                     }
                  GlobalUnlock(lpFieldPic->hDefText);
                  }
               else
                  OverWriteStringAtCurrentPos(szText,iCaretPos,' ',0);
#endif

               SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szText);
               EditFieldModifed(hWnd);
               }


            Ret = TRUE;
            }

         break;
      }

   GlobalUnlock(lpFieldPic->hMask);
   }

PicUnlockField(hWnd);
UnlockField(hWnd);
return (Ret);
}

// This is the routine we change a lot.
TCHAR PicProcessCharacter(HWND hWnd, WPARAM wParam)
{
LPEDITFIELD lpField;
LPPICFIELD  lpFieldPic;
LPTSTR      szMask;
TCHAR       cChar;

#if  defined(SPREAD_JPN)
// For saving the character(SBC or DBC). Charles
WORD          wWord;
#endif

lpField = LockField(hWnd);
lpFieldPic = PicLockField(hWnd);

if (lpFieldPic->hMask)
   {
#if  !defined(SPREAD_JPN)
// We moved the code to behind.
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
#else
// For localizaton, we removed code before and added the code to behind.
   cChar = (TCHAR)wParam;
#endif

// RFW - 10/25/02 - 11112
//   if (lpField->bHighlighted)
//      DeselectText(hWnd, lpField);
	if (lpField->bHighlighted)
		{
		TCHAR szText[MAXFIELD + 1];

      SendMessage(hWnd, WM_GETTEXT, sizeof(szText), (LPARAM)(LPTSTR)szText);
		PicClearHighlight(hWnd, szText);
      DeselectText(hWnd, lpField);
      SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szText);
		}

   szMask = (LPTSTR)GlobalLock(lpFieldPic->hMask);

#if  !defined(SPREAD_JPN)
// Changed for handling DBCS. Charles
   if (!(cChar = (TCHAR)PicValidateKey(hWnd, lpFieldPic, szMask,
                                               (short)lpField->iCurrentPos,
                                               (WORD)(_TUCHAR)cChar)))
      {
#else
   wWord = wParam;
   if (!PicValidateKey(hWnd, lpFieldPic, szMask, (short)lpField->iCurrentPos,
                       (WORD)cChar))
      {
      cChar = 0;
#endif
      if (!NotifyParent(hWnd, EN_INVALIDCHAR))

//#ifdef  BUGS
// Bug-001
         SS_BeepLowlevel(hWnd);
//#else
//         MessageBeep(MB_OK);
//#endif

      }
   else if (lpField->iCurrentPos < PicGetMaskLen(szMask))
      {
#if  defined(SPREAD_JPN)
// Moving the code before to here.
// 96' 6/19 Added by BOC Gao. (added "wWord =" to match the following DBCS decision.)
      WORD Mask = PicGetMaskChar(szMask, (short)lpField->iCurrentPos) - 256;
		Mask = toupper(Mask); // RFW - -8/3/04 - 14917
      if(Mask == PICFMT_LOWER)
          #if defined(WIN32)
          wWord = cChar = (TCHAR)CharLower((LPTSTR)MAKELONG(wParam, 0));
          #else
          wWord = cChar = (TCHAR)(long)AnsiLower((LPTSTR)MAKELONG(wParam, 0));
          #endif
      else if(Mask == PICFMT_UPPER)
          #if defined(WIN32)
          wWord = cChar = (TCHAR)CharUpper((LPTSTR)MAKELONG(wParam, 0));
          #else
          wWord = cChar = (TCHAR)(long)AnsiUpper((LPTSTR)MAKELONG(wParam, 0));
          #endif
#endif

//GAB 8/9/04
#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
//#if  !defined(SPREAD_JPN)
// Changed for Handling DBCS.
      PicReplaceCharacter(hWnd, lpField, cChar);
#else
      PicReplaceCharacter(hWnd, lpField, wWord);
      if(_TIsDBCSLeadByte((BYTE)wWord))
         lpField->iCurrentPos++;
#endif
      PicProcessKey(hWnd, VK_RIGHT, FALSE);
      }

   GlobalUnlock(lpFieldPic->hMask);
   }

PicUnlockField(hWnd);
UnlockField(hWnd);

return (cChar);
}

#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// Changed for DBCS
void PicReplaceCharacter(HWND hWnd, LPEDITFIELD lpField, TCHAR cChar)
#else
void PicReplaceCharacter(HWND hWnd, LPEDITFIELD lpField, WORD wParam)
#endif
{
TCHAR lpszLocal[MAXFIELD + 1];

#if  defined(SPREAD_JPN) && !(defined(UNICODE) || defined(_UNICODE))
// Get the FirstByte and the second byte of wParam.
BYTE FirstByte,SecondByte;

FirstByte=(BYTE)wParam;
SecondByte=(BYTE)(wParam>>8);
#endif

StrCpy(lpszLocal, lpField->lpszString);

#if  !defined(SPREAD_JPN) || defined(UNICODE) || defined(_UNICODE)
// Changed for handling DBCS.
lpszLocal[lpField->iCurrentPos] = cChar;
#else
if(!_TIsDBCSLeadByte(FirstByte))
   SecondByte=0;
OverWriteStringAtCurrentPos(lpszLocal,lpField->iCurrentPos,FirstByte,SecondByte);
#endif

StrCpy(lpField->lpszString, lpszLocal);
NotifyParent(hWnd, EN_UPDATE);

InvalidateRect(hWnd, NULL, TRUE);
EditFieldModifed(hWnd);
NotifyParent(hWnd, EN_CHANGE);
}


short PicGetMaskLen(szMask)

LPCTSTR szMask;
{
short Len = 0;
short i;

for (i = 0; i < lstrlen(szMask); )
   {
   Len++;
//GAB 8/9/04 Bug 15002
#if  !defined(SPREAD_JPN) || defined(_UNICODE) || defined(UNICODE)
//#ifndef SPREAD_JPN
// Changed for handling DBCS.
// We can get rid of this preprocessor.
   if (szMask[i++] == PICFMT_CONTROL)
#else
   if (_TIsDBCSLeadByte((BYTE)szMask[i]))
   {
      i+=2;
      Len++;
   }
   else if (szMask[i++] == PICFMT_CONTROL)
#endif
      if (PicIsMaskChar(szMask[i]) || szMask[i] == PICFMT_CONTROL)
         i++;
   }

return (Len);
}


//???? Unicode problem:
//???? PicGetMaskChar() will not work when char > 255.
WORD PicGetMaskChar(szMask, iPos)

LPCTSTR szMask;
short   iPos;
{
WORD Char = 0;
short At = 0;
short i;

for (i = 0, At = 0; i < lstrlen(szMask) && At <= iPos; i++, At++)
   {
#ifdef  SPREAD_JPN
// Add for handling DBCS.
// We can get rid of this preprocessor.
   if (_TIsDBCSLeadByte((BYTE)szMask[i]))
     {
     Char = 0;
     i++;
     At++;
     continue;
     }
#endif
   if (PicIsMaskChar(szMask[i]))
      Char = (unsigned char)szMask[i] + 256;

   else if (szMask[i] == PICFMT_CONTROL)
      {
      if (PicIsMaskChar(szMask[i + 1]) || szMask[i + 1] == PICFMT_CONTROL)
         i++;

      Char = (unsigned char)szMask[i];
      }

   else
      Char = (unsigned char)szMask[i];
   }

#if  defined(SPREAD_JPN) && !(defined(UNICODE) || defined(_UNICODE))
//- Added for '&' and 'X' KANJI mask character
// I think we should add preprocessor here.

   if((Char-256)=='&'){    // check the last '&'
      if(szMask[i]!='&') Char=256;
      }

   if((Char-256)=='X' || (Char-256)=='x'){   // cheak the last 'X'
      if(!(szMask[i]=='X' || szMask[i]=='x')) Char=257;
      }
#endif

return (Char);
}


void PicClearHighlight(HWND hWnd, LPTSTR lpszText)
{
LPEDITFIELD lpField;
LPPICFIELD  lpFieldPic;
LPTSTR      lpszMask;
LPTSTR      szDefText;
short       i;

lpField = LockField(hWnd);
lpFieldPic = PicLockField(hWnd);

if (lpField->bHighlighted && lpFieldPic->hMask)
   {
   lpszMask = (LPTSTR)GlobalLock(lpFieldPic->hMask);

   if (lpFieldPic->hDefText)
      {
      szDefText = (LPTSTR)GlobalLock(lpFieldPic->hDefText);
      for (i = lpField->iHiStart; i < lpField->iHiEnd; i++)
         if (PicIsFmtMask(lpszMask, i))
            lpszText[i] = szDefText[i];

      GlobalUnlock(lpFieldPic->hDefText);
      }

   else
      for (i = lpField->iHiStart; i < lpField->iHiEnd; i++)
         if (PicIsFmtMask(lpszMask, i))
            lpszText[i] = ' ';

   GlobalUnlock(lpFieldPic->hMask);
   EditFieldModifed(hWnd);
   }

UnlockField(hWnd);
PicUnlockField(hWnd);
}


void PicSetCallBack(HWND hWnd, FARPROC lpfnCallBack)
{
LPPICFIELD lpFieldPic;

lpFieldPic = PicLockField(hWnd);
lpFieldPic->lpfnCallBack = lpfnCallBack;
PicUnlockField(hWnd);
}


void PicSetSelection(HWND hWnd, int iStart, int iEnd)
{
   LPEDITFIELD lpField;
   LPPICFIELD  lpPicField;
#if defined(_WIN64) || defined(_IA64)
   INT_PTR iTemp;
#else
   int iTemp;
#endif
   LPTSTR szMask;
   int iCaretPos;

   lpField = LockField(hWnd);

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

   // Limit the range to the current field contents

   if (iEnd > lpField->nChars)
      iEnd = lpField->nChars;

   // If start and end are the same, turn off selection
   if (iStart == iEnd)
      DeselectText(hWnd, lpField);

   // Store them in the field's data structure
	/* RFW - 7/29/04 - 14863
   else
   {
      lpField->iHiStart = iStart;
      lpField->iHiEnd = iEnd;
   }
	*/
   lpField->iHiStart = (int)iStart;
   lpField->iHiEnd = (int)iEnd;

   // Position the caret appropriately

   lpPicField = PicLockField(hWnd);

   iCaretPos = lpField->iHiStart;

   if (lpPicField->hMask)
      {
      szMask = (LPTSTR)GlobalLock(lpPicField->hMask);

      while (iCaretPos < PicGetMaskLen(szMask) - 1 &&
             !PicIsFmtMask(szMask, (short)iCaretPos))
         iCaretPos++;

      GlobalUnlock(lpPicField->hMask);
      }

   PicUnlockField(hWnd);

   PositionCaret(hWnd, lpField, iCaretPos);

   if (lpField->iHiStart == lpField->iHiEnd)
      lpField->bHighlighted = FALSE;
   else
      lpField->bHighlighted = TRUE;

   UnlockField(hWnd);

   InvalidateRect(hWnd, NULL, TRUE);
   UpdateWindow(hWnd);
}


BOOL DLLENTRY PicSetValue(HWND hWnd, LPCTSTR lpString)
{
return (PicSetText(hWnd, lpString, NULL, TRUE));
}


BOOL DLLENTRY PicIsNULL(HWND hWnd)
{
TCHAR lpszLocal[MAXFIELD + 1];
BOOL  fRet = TRUE;

if (PicGetValue(hWnd, lpszLocal))
	{
	StrTrim(lpszLocal);
	if (lpszLocal[0])
		fRet = FALSE;
	}

return (fRet);
}


BOOL DLLENTRY PicGetValue(HWND hWnd, LPTSTR lpValString)
{
LPEDITFIELD lpField;
LPPICFIELD  lpFieldPic;
LPTSTR      lpszPicMask;
BOOL        fRet = FALSE;

if (lpValString)
   {
   lpField = LockField(hWnd);
   lpFieldPic = PicLockField(hWnd);
   *lpValString = '\0';

   if (lpFieldPic->hMask)
      {
      lpszPicMask = (LPTSTR)GlobalLock(lpFieldPic->hMask);
      fRet = PicToValue(lpField->lpszString, lpszPicMask, lpValString);
      GlobalUnlock(lpFieldPic->hMask);
      }

   PicUnlockField(hWnd);
   UnlockField(hWnd);
   }

return (fRet);
}


BOOL DLLENTRY PicToValue(LPCTSTR lpszPicFmt, LPCTSTR lpszPicMask,
                         LPTSTR lpszPicValue)
{
short At;
short i;

if (!lpszPicFmt || !lpszPicMask || !lpszPicValue)
   return (FALSE);

if (lstrlen(lpszPicFmt) > PicGetMaskLen(lpszPicMask))
   return (FALSE);

for (i = 0, At = 0; i < lstrlen(lpszPicFmt); i++)
   if (PicIsFmtMask(lpszPicMask, i))
      lpszPicValue[At++] = lpszPicFmt[i];

lpszPicValue[At++] = '\0';

return (TRUE);
}


void PicClipboardPaste(HWND hWnd)
{
LPEDITFIELD  lpField;
GLOBALHANDLE hText;
LPTSTR       lpszText;

lpField = LockField(hWnd);

if (hText = GetTextFromClipboard(hWnd))
   {
   lpszText = (LPTSTR)GlobalLock(hText);

//GAB 8/9/04 Added new define to fix bug 1002
#if !defined(SPREAD_JPN) || defined(_UNICODE) || defined(UNICODE)
   // Changed for handling DBCS
   if (lpField->bHighlighted)
      {
      lpField->bHighlighted = FALSE;
      PicReplaceText(hWnd, lpField, lpszText, (short)lpField->iHiStart,
                     (short)(lpField->iHiEnd - 1));
      }
   else
      PicReplaceText(hWnd, lpField, lpszText, (short)lpField->iCurrentPos,
                     (short)-1);
#else
{
int   i;
int   iLen = lstrlen(lpszText);
WORD  wWord;

      for (i = 0; i < iLen; i++) {
         if (!lpszText[i]) break;
         wWord = 0;
         wWord = (BYTE)lpszText[i];
         if (_TIsDBCSLeadByte((BYTE)lpszText[i])) {
            i++;
            wWord += (BYTE)lpszText[i] << 8;
         }
         if (!PicProcessCharacter(hWnd, wWord)) break;
      }
  }
#endif

   GlobalUnlock(hText);
   GlobalFree(hText);
   }

UnlockField(hWnd);
}

//GAB 8/9/04 Added new define to fix bug 15002
#if  !defined(SPREAD_JPN) || defined(_UNICODE) || defined(UNICODE)
void PicReplaceText(HWND hWnd, LPEDITFIELD lpField, LPCTSTR lpszText,
                    short dPosStart, short dPosEnd)
{
LPPICFIELD lpFieldPic;
LPTSTR     lpszDefText;
LPTSTR     lpMask;
LPTSTR     lpPtr;
TCHAR      lpszLocal[MAXFIELD + 1];
short      At;
short      i;

lpFieldPic = PicLockField(hWnd);

StrCpy(lpszLocal, lpField->lpszString);

lpMask = (LPTSTR)GlobalLock(lpFieldPic->hMask);

for (i = 0, At = 0, lpPtr = lpMask; *lpPtr; i++)
   {
   if (dPosEnd != -1 && i > dPosEnd)
      break;

   if (*lpPtr == PICFMT_CONTROL)
      lpPtr++;

   else if (i >= dPosStart)
      {
      if (PicIsMaskChar(*lpPtr))
         {
         if (At < lstrlen(lpszText))
            {
//GAB 8/9/04 Bug 15002
#if  !defined(SPREAD_JPN) || defined(_UNICODE) || defined(UNICODE)
// Changed for handling DBCS.
			 WORD Key; //GAB 2/11/2005 Bug 15680
            if (Key = PicxValidateKey(hWnd, lpFieldPic, (WORD)*lpPtr,
                                (WORD)lpszText[At]))
#else
            if (Key = PicxValidateKey(hWnd, lpFieldPic, (WORD)*lpPtr,
                                (WORD)lpszText[At], lpszText, At))
#endif
//             lpszLocal[i] = lpszText[At++];
//GAB 2/11/2005 Bug 15680
			{
               lpszLocal[i] = (TCHAR)Key;
				At++;
			}
            }
#ifdef  BUGS
// Bug-005
         else if(i <= lstrlen(lpField->lpszString)) ;
#endif
         else if (lpFieldPic->hDefText)
            {
            lpszDefText = (LPTSTR)GlobalLock(lpFieldPic->hDefText);
            lpszLocal[i] = lpszDefText[i];
            GlobalUnlock(lpFieldPic->hDefText);
            }
         else
            lpszLocal[i] = ' ';
         }

      else if (*lpPtr == lpszText[At])
//GAB 8/9/04 Bug 15002
#if  !defined(SPREAD_JPN) || defined(_UNICODE) || defined(UNICODE)
//#ifndef SPREAD_JPN 
// Changed for handling DBCS.
// We can get rid of this preprocessor.
         At++;
#else
        {
         if (_TIsDBCSLeadByte((BYTE)*lpPtr) && (*(lpPtr+1) == lpszText[At]))
            At++;
        }
#endif
      }

// changed for handling DBCS.
// If a mask character is a KANJI then we should move 2 bytes, such as: "\Š¿"
// We can get rid of this preprocessor.
   if (_TIsDBCSLeadByte((BYTE)*lpPtr))
    {
      lpPtr++;
      i++;
    }
    if (*lpPtr) lpPtr++;

   }

lpField->iCurrentPos = i;

lstrcpy(lpField->lpszString, lpszLocal);
lpField->nChars = lstrlen(lpField->lpszString);
NotifyParent(hWnd, EN_UPDATE);
PositionCaret(hWnd, lpField, lpField->iCurrentPos);

InvalidateRect(hWnd, NULL, TRUE);
UpdateWindow(hWnd);

EditFieldModifed(hWnd);
NotifyParent(hWnd, EN_CHANGE);

GlobalUnlock(lpFieldPic->hMask);
PicUnlockField(hWnd);
}
#endif // Added by BOC FMH


void PicClipboardCut(HWND hWnd)
{
LPEDITFIELD lpField;
TCHAR       szText[MAXFIELD + 1];

lpField = LockField(hWnd);

if (lpField->bHighlighted)
   if (SendTextToClipboard(hWnd, &lpField->lpszString[lpField->iHiStart],
                           (short)(lpField->iHiEnd - lpField->iHiStart)))
      {
      SendMessage(hWnd, WM_GETTEXT, sizeof(szText), (LPARAM)(LPTSTR)szText);
      PicClearHighlight(hWnd, szText);
      EditFieldModifed(hWnd);
      SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szText);
      }

UnlockField(hWnd);
}


BOOL DLLENTRY PicFormatText(HWND hWnd, LPCTSTR lpszDefText, LPCTSTR lpszMask,
                            LPTSTR lpszText, LPTSTR lpszTextFmt,
                            BOOL fTextUnformatted)
{
GLOBALHANDLE hDefText = 0;
GLOBALHANDLE hMask = 0;
LPPICFIELD   lpField;
LPTSTR       lpTemp;
BOOL         Ret = FALSE;

if (hWnd)
   {
   lpField = PicLockField(hWnd);

   Ret = PicFmtText(0, lpField->hDefText, lpField->hMask, lpszText,
                    lpszTextFmt, fTextUnformatted);

   PicUnlockField(hWnd);
   }

else
   {
   if (lpszDefText && lstrlen(lpszDefText))
      if (hDefText = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                 (lstrlen(lpszDefText) + 1) * sizeof(TCHAR)))
         {
         lpTemp = (LPTSTR)GlobalLock(hDefText);
         lstrcpy(lpTemp, lpszDefText);
         GlobalUnlock(hDefText);
         }

   if (lpszMask && lstrlen(lpszMask))
      if (hMask = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                              (lstrlen(lpszMask) + 1) * sizeof(TCHAR)))
         {
         lpTemp = (LPTSTR)GlobalLock(hMask);
         lstrcpy(lpTemp, lpszMask);
         GlobalUnlock(hMask);
         }

   Ret = PicFmtText(0, hDefText, hMask, lpszText, lpszTextFmt,
                    fTextUnformatted);

   if (hDefText)
      GlobalFree(hDefText);

   if (hMask)
      GlobalFree(hMask);
   }

return (Ret);
}

#if  defined(SPREAD_JPN) && !(defined(UNICODE) || defined(_UNICODE))
// There are two functions we added.
// IsKanjiType(...): return the [Pos] position in the [lpstr] string.
//                   1 --- The character is a lead byte.
//                   2 --- The character is a second byte.
//
int IsKanJiType(lpstr,Pos)

LPCTSTR lpstr;
int    Pos;
{
int CharType,i;

for(i=0;i<=Pos;i++){
   CharType=0;
   if(_TIsDBCSLeadByte((BYTE)lpstr[i])){
      CharType=1;
      i++;
      if(i==Pos) CharType=2;
      }
   }
   return CharType;
}

//OverWriteStringAtCurrentPos(...): 
// the function will overwrite the Character(DBC, SBC) at the
// Pos position of lpstr string with Byte1 and Byte2.
// If Byte2 is zero, the byte1 is a SBC, only one character is
// overwrited.(Charles)
void OverWriteStringAtCurrentPos(lpstr,Pos,Byte1,Byte2)

LPTSTR lpstr;
int   Pos;
BYTE  Byte1,Byte2;
{
int Type;

Type=IsKanJiType(lpstr,Pos);
switch(Type){
   case 1:
      lpstr[Pos]=' ';
      lpstr[Pos+1]=' ';
      break;
   case 2:
      lpstr[Pos-1]=' ';
      lpstr[Pos]=' ';
      break;
   }
if(Byte2!=0){
    Type=IsKanJiType(lpstr,Pos+1);
    if(Type==1){
      lpstr[Pos+1]=' ';
      lpstr[Pos+2]=' ';
      }
    lpstr[Pos+1]=Byte2;
   }
lpstr[Pos]=Byte1;
}
#endif
