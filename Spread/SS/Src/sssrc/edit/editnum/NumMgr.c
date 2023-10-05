
#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <float.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <toolbox.h>
#include <fptools.h>

#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editnum.h"

#define NUM_BUFFSIZE 330

extern void SS_BeepLowlevel(HWND hwnd);

static void Num_CopyDecDigits(LPTSTR lpszDest, LPTSTR lpszSrc, int iLen);
static int Num_GetDecimalPos(LPEDITNUMINFO lpInfo, LPTSTR szText);
static void Num_SetCaretLeftOfDec(LPEDITNUMINFO lpInfo, LPTSTR szText);
static void Num_SetCaretRightOfDec(LPEDITNUMINFO lpInfo, LPTSTR szText);
static void Num_SetValueInternal(LPEDITNUMINFO lpInfo, double dfVal);
BOOL Num_IsValInRange(LPEDITNUMINFO lpInfo, double dfVal);
static double dfZero = 0.0;
SSNUM_INTLSETTINGS NumIntlSettings;
BOOL fNumIntlSettingsSet = FALSE;


BOOL Num_SetRange(LPEDITNUMINFO lpInfo, double dfMinVal, double dfMaxVal)
{
if (dfMinVal <= dfMaxVal)
	{
   lpInfo->dfMaxVal = dfMaxVal;
   lpInfo->dfMinVal = dfMinVal;
   return(TRUE);
	}

return(FALSE);
}


void Num_InitUserFormat(LPSSNUM_FORMAT lpFormat)
{
_fmemset(lpFormat, '\0', sizeof(SSNUM_FORMAT));
lpFormat->bLeadingZero = 0;
lpFormat->bNegCurrencyStyle = 0;
lpFormat->bPosCurrencyStyle = 0;
lpFormat->bNegStyle = 0;
lpFormat->bFractionalWidth = -1;
}


void Num_GetDisplayFormat(LPSSNUM_FORMAT lpFormatSrc, LPSSNUM_FORMAT lpFormat)
{
*lpFormat = *lpFormatSrc;

NumGetSystemIntlSettings(NULL);

if (lpFormat->fCurrency)
	{
	if (!lpFormat->szDecimal[0])
		lstrcpy(lpFormat->szDecimal, NumIntlSettings.mon_decimal_point);
	if (!lpFormat->szSeparator[0])
		lstrcpy(lpFormat->szSeparator, NumIntlSettings.mon_thousands_sep);
	if (!lpFormat->szCurrency[0])
		lstrcpy(lpFormat->szCurrency, NumIntlSettings.currency_symbol);

	if (lpFormat->bFractionalWidth == -1)
		lpFormat->bFractionalWidth = NumIntlSettings.frac_digits;
	}
else
	{
	if (!lpFormat->szDecimal[0])
		lstrcpy(lpFormat->szDecimal, NumIntlSettings.decimal_point);
	if (!lpFormat->szSeparator[0])
		lstrcpy(lpFormat->szSeparator, NumIntlSettings.thousands_sep);
	lpFormat->szCurrency[0] = '\0';

	if (lpFormat->bFractionalWidth == -1)
		lpFormat->bFractionalWidth = 0;
	}

if (lpFormat->bLeadingZero == 0)
	lpFormat->bLeadingZero = NumIntlSettings.fLeadingZero;
else
	lpFormat->bLeadingZero--;

if (lpFormat->bNegStyle == 0)
	lpFormat->bNegStyle = NumIntlSettings.bNegStyle;
else
	lpFormat->bNegStyle--;

if (lpFormat->bNegCurrencyStyle == 0)
	lpFormat->bNegCurrencyStyle = NumIntlSettings.bNegCurrencyStyle;
else
	lpFormat->bNegCurrencyStyle--;

if (lpFormat->bPosCurrencyStyle == 0)
	lpFormat->bPosCurrencyStyle = NumIntlSettings.bPosCurrencyStyle;
else
	lpFormat->bPosCurrencyStyle--;
}


void Num_GetValue(LPEDITNUMINFO lpInfo, LPDOUBLE lpdfVal)
{
TCHAR szVal[NUM_BUFFSIZE];

szVal[0] = '\0';
SendMessage(lpInfo->hWnd, WM_GETTEXT, NUM_BUFFSIZE, (LPARAM)(LPTSTR)szVal);
Num_StringToValue(&lpInfo->Format, szVal, lpdfVal);
}


BOOL Num_StringToValue(LPSSNUM_FORMAT lpFormat, LPCTSTR szString, LPDOUBLE lpdfVal)
{
SSNUM_FORMAT Format;
TCHAR        szVal[NUM_BUFFSIZE];
short        dCurrencyLen;
short        dDecimalLen;
short        dSeparatorLen;
short        i, j;
short        nParenCnt = 0;
BOOL         fNeg = FALSE;
BOOL         fRet = FALSE;

Num_GetDisplayFormat(lpFormat, &Format);

dCurrencyLen = lstrlen(Format.szCurrency);
dDecimalLen = lstrlen(Format.szDecimal);
dSeparatorLen = lstrlen(Format.szSeparator);

// parse prefix
i = 0; j = 0;         
while (szString[i])
   {
   if (szString[i] == ' ')
      i++;
   else if (szString[i] == '(')
		{
      fNeg = TRUE, i++;
		nParenCnt++;
		}
   else if (szString[i] == *NumIntlSettings.negative_sign)
      fNeg = TRUE, i++;
   else if (dCurrencyLen && _tcsnicmp(&szString[i], Format.szCurrency, dCurrencyLen) == 0)
      i += dCurrencyLen;
   else
      break;
   }

while (szString[i] == ' ')
	i++;

// parse value
while (szString[i])
   {
   if (isdigit(szString[i]))
      szVal[j++] = szString[i++];
   else if (tolower(szString[i]) == 'e')
		{
      szVal[j++] = szString[i++];
	   if (szString[i] == '+' || szString[i] == '-')
	      szVal[j++] = szString[i++];
		}
   else if (dDecimalLen && _tcsnicmp(&szString[i], Format.szDecimal, dDecimalLen) == 0)
      {
      i += dDecimalLen;
      szVal[j++] = '.';
      }
   else if (dSeparatorLen && _tcsnicmp(&szString[i], Format.szSeparator, dSeparatorLen) == 0)
      i += dSeparatorLen;
	// If there is a negative signe anywhere in the text, make the
	// number negative and ignore the negative sign.
   else if (szString[i] == *NumIntlSettings.negative_sign)
      fNeg = TRUE, i++;
   else if (szString[i] == '(')
		{
      fNeg = TRUE, i++;
		nParenCnt++;
		}
   else if (szString[i] == ')')
		{
      i++;
		nParenCnt--;
		}
	/*
   else if (szString[i] == ' ')
      i++;
	*/
   else
      break;
   }

// postfix
while (szString[i])
   {
   if (szString[i] == ' ')
      i++;
   else if (szString[i] == ')')
		{
      i++;
		nParenCnt--;
		}
   else if (szString[i] == *NumIntlSettings.negative_sign)
      i++, fNeg = TRUE;
   else if (dCurrencyLen && _tcsnicmp(&szString[i], Format.szCurrency, dCurrencyLen) == 0)
      i += dCurrencyLen;
   else
      break;
   }

if (nParenCnt != 0)
	;
else if (szString[i] == (char)0)
   {
//	char *lpTemp;

   szVal[j] = (char)0;

   StringToFloat(szVal, lpdfVal);
//	*lpdfVal = fpstrtod(szVal, &lpTemp, '.');
   if (fNeg)
		*lpdfVal *= -1;

	fRet = TRUE;
   }
else
	*lpdfVal = dfZero;

return (fRet);
}


// This function returns the formatted float string via szString.

BOOL Num_ValueToString(LPSSNUM_FORMAT lpFormat, double dfVal, LPTSTR szString)
{
SSNUM_FORMAT Format;
TCHAR szText[NUM_BUFFSIZE];

Num_GetDisplayFormat(lpFormat, &Format);

fpIntlDoubleToString(szText, dfVal, Format.bFractionalWidth, Format.szDecimal,
                     Format.fShowSeparator ? Format.szSeparator : NULL,
                     Format.bLeadingZero, TRUE, Format.bNegStyle, FALSE,
                     MAKEWORD(Format.bNegCurrencyStyle, Format.bPosCurrencyStyle),
							Format.fShowCurrencySymbol ? Format.szCurrency : _T(""),
                     NumIntlSettings.negative_sign, Format.fCurrency);

lstrcpy(szString, szText);

return TRUE;
}


BOOL Num_ProcessWMCreate(HWND hWnd, LPARAM lParam)
{
HANDLE         hMem;
LPEDITNUMINFO  lpTmp;
LONG           style;
BOOL           fRet = FALSE;

if (!(hMem = GlobalAlloc(GHND, (DWORD)sizeof(EDITNUMINFO))))
   NotifyParent(hWnd, EN_ERRSPACE);
else
	{
   // Set the parent handler at offset 4 of the window structure
   SetDefWndProc(hWnd, GetTBEditClassProc());
   SetWindowNumField(hWnd, hMem);
   lpTmp = tbMemLock(hMem);

   lpTmp->fEmpty = TRUE;
   lpTmp->dfMinVal = -DBL_MAX;
   lpTmp->dfMaxVal = DBL_MAX;
   lpTmp->hWnd = hWnd;
   lpTmp->hWndParent = ((LPCREATESTRUCT)lParam)->hwndParent;
   // make the style to be right justified
   style = ((LPCREATESTRUCT)lParam)->style;
   style |= ES_RIGHT;

	if (!fNumIntlSettingsSet)
		{
		NumGetSystemIntlSettings(&NumIntlSettings);
		fNumIntlSettingsSet = TRUE;
		}

	Num_InitUserFormat(&lpTmp->Format);
   lpTmp->iPrevStrLen = 0;
   ((LPCREATESTRUCT)lParam)->style = lpTmp->style = style;
   SetWindowLong(hWnd, GWL_STYLE, style);
   fRet = TRUE;

   tbMemUnlock(hMem);
	}

return(fRet);
}


BOOL Num_CheckRange(LPEDITNUMINFO lpInfo, double dfVal)
{
BOOL fValid = FALSE;

if (lpInfo->dfMinVal <= dfVal && dfVal <= lpInfo->dfMaxVal)
   fValid = TRUE;

return(fValid);
}


void Num_ProcessWMDestroy(HWND hWnd)
{
HANDLE hMem;

if (IsWindow(hWnd) && (hMem = GetWindowNumField(hWnd)))
   {
   LPEDITNUMINFO lpNumField;

   lpNumField = (LPEDITNUMINFO)GlobalLock(hMem);

   if (lpNumField->hWndSpinBtn)
      DestroyWindow(lpNumField->hWndSpinBtn);

   GlobalUnlock(hMem);

   GlobalFree(hMem);
   SetWindowNumField(hWnd, 0);
   }
}


BOOL Num_ProcessWMChar(HWND hWnd, WPARAM wParam, LPARAM lParam)

// This function's purpose is to intercept and reject invalid characters
// and to process the minus and plus signs and the decimal point.
//
// If a plus, minus, or decimal point are detected, they
// are processed here and the function returns FALSE, preventing the
// character from being passed on to the superclass.
{
LPEDITFIELD  lpField;
SSNUM_FORMAT Format;
BOOL         fDigit = isdigit((int)wParam);
BOOL         fRetCode = TRUE;
int          iCaretPos;
int          iDecimalPos;
WORD         wLen;
double       dfVal;
LONG         lSel;
TCHAR        cPosSign = ((*NumIntlSettings.positive_sign && !isdigit(*NumIntlSettings.positive_sign)) ? *NumIntlSettings.positive_sign : '+');
TCHAR        cNegSign = *NumIntlSettings.negative_sign;

LockNumHeader(lpInfo, hInfo, hWnd);

lpField = LockField(hWnd);

if (lpInfo)
	{
	Num_GetDisplayFormat(&lpInfo->Format, &Format);

   // RFW - 3/10/03 - 11402
   // if (GetKeyState(VK_DECIMAL) & 0x8000)
	if (VK_DECIMAL == lpInfo->wKeyDownVirtualKeyCode || VK_DECIMAL == wParam)
      wParam = (WPARAM)*Format.szDecimal;

   if (wParam != VK_BACK && !fDigit && wParam != (WPARAM)*NumIntlSettings.negative_sign &&
       wParam != (WPARAM)cPosSign && (TUCHAR)wParam != *Format.szDecimal)
		{
      SS_BeepLowlevel(hWnd);
	   if (lpInfo->hWndParent)
		   NotifyParent(hWnd, EN_INVALIDCHAR);
      fRetCode = FALSE;
		}

   // Highlighted Text
	// RFW - 1/22/04 - 13619
   if (fRetCode && lpField->bHighlighted)
		{
      wLen = lstrlen(lpField->lpszString);
      lSel = (LONG)SendMessage(hWnd, EM_GETSEL, 0, 0l);
      if (LOWORD(lSel) == 0 && (HIWORD(lSel)) == wLen)
			{
			BOOL fSuspend = lpField->fSuspendNotify;

         DeselectText(hWnd, lpField);
			lpField->fSuspendNotify = TRUE;
			Num_SetZero(lpInfo);
			Num_SetCaretLeftOfDec(lpInfo, lpField->lpszString);
			lpField->fSuspendNotify = fSuspend;
			}
		}

	if (fRetCode && GetOverstrikeMode(hWnd, FALSE) && !isdigit(lpField->lpszString[lpField->iCurrentPos]) &&
       wParam != (WPARAM)cNegSign && wParam != (WPARAM)cPosSign)
		{
      SS_BeepLowlevel(hWnd);
      fRetCode = FALSE;
		}

   if (fRetCode)
		{
      // Minus Sign
      if (wParam == (WPARAM)cNegSign)         // the user wants to negate the value
			{
			int iDecimalPosBefore;
			int iDecimalPosAfter;

			// RFW - 6/15/04 - 14523
         // I moved these 2 lines before lpInfo->fNegative is set.
         iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);   // Pos before dec point
			iDecimalPosBefore = Num_GetDecimalPos(lpInfo, lpField->lpszString);

         if (lpInfo->dfMinVal >= dfZero)        // min val is greater than zero
            SS_BeepLowlevel(hWnd);

         else if (lpInfo->dfVal == dfZero)      // Field contains zero
            {
            lpInfo->fNegative = !lpInfo->fNegative;

            if (Num_IsValInRange(lpInfo, lpInfo->dfVal))
				   {
/*
					if (!lpInfo->fNegative)
						{
						if (iCaretPos > 0)
							--iCaretPos;      // going + so decrease the count
						}
					else
						++iCaretPos;         // going - so increase the count
*/
               EditFieldModifed(hWnd);
               Num_SetValueInternal(lpInfo, lpInfo->dfVal);
					iDecimalPosAfter = Num_GetDecimalPos(lpInfo, lpField->lpszString);
               SendMessage(hWnd, EM_SETCARETPOS, iCaretPos - (iDecimalPosBefore - iDecimalPosAfter), 0L);
					}

            // else out of range
            else
               SS_BeepLowlevel(hWnd);
            }
         else
				{
/* 
           if (lpInfo->dfVal < dfZero)
					{
               if (iCaretPos > 0)
                  --iCaretPos;      // going + so decrease the count
					}
            else
               ++iCaretPos;         // going - so increase the count
*/
            dfVal = -lpInfo->dfVal;
            if (Num_IsValInRange(lpInfo, dfVal)) // RFW - 6/27/93
					{
               EditFieldModifed(hWnd);
               Num_SetValueInternal(lpInfo, dfVal);
					iDecimalPosAfter = Num_GetDecimalPos(lpInfo, lpField->lpszString);
               SendMessage(hWnd, EM_SETCARETPOS, iCaretPos - (iDecimalPosBefore - iDecimalPosAfter), 0L);
//					Num_SetCaretLeftOfDec(lpInfo, lpField->lpszString);
					}

            // else out of range
            else
               SS_BeepLowlevel(hWnd);
				}
         fRetCode = FALSE;
			}

      // Plus Sign
      else if (wParam == (WPARAM)cPosSign)
			{
         lpInfo->fNegative = FALSE;

         if (lpInfo->dfMaxVal < dfZero)        // max val is less than zero
            SS_BeepLowlevel(hWnd);

         else if (lpInfo->dfVal == dfZero)      // Field contains zero
            {
            Edit_SetSel(hWnd, 0, 0);
				Num_SetZero(lpInfo);
            }

         else if (lpInfo->dfVal < dfZero)
				{
            dfVal = -lpInfo->dfVal;
            if (Num_IsValInRange(lpInfo, dfVal))// RFW - 6/27/93
					{
               EditFieldModifed(hWnd);
	            Num_SetValueInternal(lpInfo, dfVal);
					Num_SetCaretLeftOfDec(lpInfo, lpField->lpszString);
					}

            // else out of range
            else
               SS_BeepLowlevel(hWnd); 
				}

         fRetCode = FALSE;
			}

      // Decimal Point
      else if ((TUCHAR)wParam == *Format.szDecimal)
			{
			if (Format.bFractionalWidth)
				{
				// Is the field highlighted ?
				wLen = StrLen(lpField->lpszString);
				lSel = (LONG) SendMessage(hWnd, EM_GETSEL, 0, 0L);
				if (LOWORD(lSel) == 0 && (HIWORD(lSel)) == wLen)
					{
					if (lpInfo->dfVal == dfZero)     // Just deselect it.
						{
						iDecimalPos = Num_GetDecimalPos(lpInfo, lpField->lpszString);
						Edit_SetSel(hWnd, iDecimalPos, iDecimalPos);
						}
					else
						Num_SetZero(lpInfo);
					}

				else
					{
					iDecimalPos = Num_GetDecimalPos(lpInfo, lpField->lpszString);
					iCaretPos = (int) SendMessage(hWnd, EM_GETCARETPOS, 0, 0L);
					if (iCaretPos != iDecimalPos)
						SS_BeepLowlevel(hWnd);  
					else
						SendMessage(hWnd, EM_SETCARETPOS, iDecimalPos+_tcsclen(Format.szDecimal), 0l);
					}
				}
         fRetCode = FALSE;
			}
		}

   UnlockNumHeader(hInfo);
	}

UnlockField(hWnd);
return(fRetCode);
}


LONG Num_ProcessEMValidate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
LPEDITFIELD  lpField;
SSNUM_FORMAT Format;
LPTSTR       lpszString = (LPTSTR)lParam;
LPTSTR       szVal;
double       dfVal;
BOOL         fRetCode = FALSE;
int          iLen, iLenNew, iLenOrig;
int          iCaretPos;
int          iDistFromRHS;

LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
	{
   lpField = LockField(hWnd);
	Num_GetDisplayFormat(&lpInfo->Format, &Format);

   if (lParam)
		szVal = lpszString;
   else
      szVal = lpField->lpszString;

	iLenOrig = lstrlen(szVal);
   iLen = StrLen(lpField->lpszString);
   iDistFromRHS = iLen - (int)wParam;

   if (lpField->bProcessingBackspace && lpField->bHighlighted)
      iDistFromRHS = iLen - lpField->iHiEnd;
   else if (lpField->bProcessingDelete && lpField->bHighlighted)
      iDistFromRHS = iLen - lpField->iHiEnd;
   else if (lpField->bProcessingDelete)
      iDistFromRHS = iLen - (int)wParam - 1;
   else
      iDistFromRHS = iLen - (int)wParam;

	dfVal = 0.0;
	if (Num_StringToValue(&lpInfo->Format, szVal, &dfVal))
		// now check for ranges
		if (Num_IsValInRange(lpInfo, dfVal))
			fRetCode = TRUE;

   if (fRetCode)
		{
		if (dfVal == dfZero)    // if empty then we must post a message to the parent
			{
			if (!lpInfo->fEmpty)
				{
				if (lpInfo->hWndParent)
					NotifyParent(hWnd, EN_FIELDEMPTY);
				lpInfo->fEmpty = TRUE;
				}
			}

		else if (lpInfo->fEmpty)
			{
			NotifyParent(hWnd, EN_FIELDNONEMPTY);
			lpInfo->fEmpty = FALSE;
			}

      // Format the new string

      lpInfo->dfVal = dfVal;

		/* RFW - 2/27/07 - 19939
      if (dfVal == dfZero && lpInfo->fNegative)
         Num_ValueToString(&lpInfo->Format, -HUGE_VAL, lpszString);
      else
         Num_ValueToString(&lpInfo->Format, dfVal, lpszString);
		*/
		/* RFW - 5/16/07 - 20340
      if (dfVal == dfZero && lpInfo->fNegative)
			lpInfo->fNegative = FALSE;

      Num_ValueToString(&lpInfo->Format, dfVal, lpszString);
		*/

      if (dfVal == dfZero && lpInfo->fNegative && (lpField->bProcessingBackspace || lpField->bProcessingDelete))
			{
			lpInfo->fNegative = FALSE;
	      Num_ValueToString(&lpInfo->Format, dfVal, lpszString);
			}
      else if (dfVal == dfZero && lpInfo->fNegative)
         Num_ValueToString(&lpInfo->Format, -HUGE_VAL, lpszString);
      else
         Num_ValueToString(&lpInfo->Format, dfVal, lpszString);

		iLenNew = lstrlen(lpszString);
		lpInfo->iPrevStrLen = iLenNew;

      // Reposition the caret
      iCaretPos = StrLen(lpszString) - iDistFromRHS;
		iCaretPos = max(0, iCaretPos);
//      SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0l);

//	   iCaretPos = SendMessage(hWnd, EM_GETCARETPOS, 0, 0l);
		if (iLenOrig < iLenNew)
			lpField->iCurrentPos += iLenNew - iLenOrig;
//	      SendMessage(hWnd, EM_SETCARETPOS, iCaretPos + 1, 0l);
		else if (iLenOrig > iLenNew)
			lpField->iCurrentPos -= iLenOrig - iLenNew;
//	      SendMessage(hWnd, EM_SETCARETPOS, iCaretPos - 1, 0l);

      lpField->nChars = lstrlen(lpszString);
		// RFW - 7/25/03 - 12306 
		lpField->iCurrentPos = max(0, lpField->iCurrentPos);
		lpField->iCurrentPos = min(lpField->nChars + 1, lpField->iCurrentPos);
		}
   else
      SS_BeepLowlevel(hWnd);

   UnlockField(hWnd);
   UnlockNumHeader(hInfo);
	}

return(fRetCode);
}


void Num_SetValue(LPEDITNUMINFO lpInfo, double dfVal)
{
if (dfVal == dfZero)
   lpInfo->fNegative = FALSE;

Num_SetValueInternal(lpInfo, dfVal);

if (lpInfo->fEmpty)
	{
   NotifyParent(lpInfo->hWnd, EN_FIELDNONEMPTY);
   lpInfo->fEmpty = FALSE;
	}
}


BOOL Num_ProcessWMSetText(HWND hWnd, WPARAM wParam, LPCTSTR lParam)
{
LPCTSTR  lpszStr = lParam;
BOOL     fRetCode = TRUE;
double   dfVal = dfZero;

LockNumHeader(lpInfo, hInfo, hWnd);
if (lpszStr)
	{
	Num_StringToValue(&lpInfo->Format, lpszStr, &dfVal);

   if (dfVal != dfZero)
      fRetCode = Num_CheckRange(lpInfo, dfVal);
	}

if (fRetCode)
	Num_SetValue(lpInfo, dfVal);

UnlockNumHeader(hInfo);

return (fRetCode);
}


/************************************************************************
* Func: void  Num_ProcessWMKillFocus(HWND hWnd)
*
* Desc: In kill focus we check whether the val in the window is still
*       valid, and notify the parent if not
************************************************************************/

void Num_ProcessWMKillFocus(HWND hWnd)
{
double dfVal;

LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
	{
	Num_GetValue(lpInfo, &dfVal);
   if (!Num_CheckRange(lpInfo, dfVal))
		{
      if (lpInfo->hWndParent)
         NotifyParent(hWnd, EN_INVALIDDATA);
		}

   UnlockNumHeader(hInfo);
	}
}


void Num_SetZero(LPEDITNUMINFO lpInfo)
{
EditFieldModifed(lpInfo->hWnd);
lpInfo->fNegative = FALSE;
Num_SetValueInternal(lpInfo, dfZero);
}


BOOL Num_ProcessEMCaretPos(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
int  iDecimalPos = -1;
BOOL fRet = 0;

LockNumHeader(lpInfo, hInfo, hWnd);

iDecimalPos = Num_GetDecimalPos(lpInfo, NULL);

if (iDecimalPos != -1 && wParam > (WORD)iDecimalPos)
	{
   if (lParam)
      fRet = 0;
   else if (!GetOverstrikeMode(hWnd, FALSE))
      SetOverstrikeMode(hWnd, FALSE);
	}
else
	{
   if (lParam)
      fRet = 1;
   else if (GetOverstrikeMode(hWnd, FALSE))
      SetInsertMode(hWnd, FALSE);
	}

UnlockNumHeader(hInfo);
return (fRet);
}


BOOL Num_ProcessWMKeyDown(HWND hWnd, WPARAM wParam)
{
SSNUM_FORMAT Format;
#if defined(_WIN64) || defined(_IA64)
LRESULT      lSel;
#else
LONG         lSel;
#endif
BOOL         fRetCode = FALSE;
TCHAR        szText[NUM_BUFFSIZE];
int          iCaretPos;
int          iLen;
int          i;

LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
	{
	lpInfo->wKeyDownVirtualKeyCode = wParam;

   if ((wParam == VK_DELETE && !IsShiftDown(hWnd)) || wParam == VK_BACK)
		{
		Num_GetDisplayFormat(&lpInfo->Format, &Format);

		lSel = SendMessage(hWnd, EM_GETSEL, 0, 0l);
		// check if all of the text is hilited. if it is then we delete the whole thing
		iLen = (int)SendMessage(hWnd, WM_GETTEXT, NUM_BUFFSIZE, (LPARAM)(LPTSTR)szText);
		if (LOWORD(lSel) == 0 && (HIWORD(lSel)) == (WORD)iLen)
			{
			Edit_SetSel(hWnd, 0, 0);         // Turn off highlight
			Num_SetZero(lpInfo);
			fRetCode = TRUE;
			}
		else
			{
			iCaretPos = (int)SendMessage(hWnd, EM_GETCARETPOS, 0, 0);

			if (GetOverstrikeMode(hWnd, LOCAL))     // Overstrike mode
				{
				if (isdigit(szText[iCaretPos]))
					{
					szText[iCaretPos] = '0';
					SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szText);
					SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0l);
					}
				}

			if (wParam == VK_BACK)
				--iCaretPos;

			if (iCaretPos < NUM_BUFFSIZE)
				{
				if (LOWORD(lSel) < HIWORD(lSel))
					{
					fRetCode = TRUE;

					for (i = (short)LOWORD(lSel); i < (short)HIWORD(lSel); i++)
						{
						if (isdigit(szText[i]))
							fRetCode = FALSE;

						else if ((TUCHAR)szText[i] == *Format.szDecimal)
							{
							fRetCode = TRUE;
							break;
							}
						}
					}

				else if (wParam == VK_BACK && GetOverstrikeMode(hWnd, LOCAL))
					{
					SendMessage(hWnd, EM_SETCARETPOS, iCaretPos, 0l);
					fRetCode = TRUE;
					}

				else
					fRetCode = !isdigit(szText[iCaretPos]);
				}

			// check if we are deleting a zero before a '.'
			if (!fRetCode && iCaretPos < iLen && iCaretPos >= 0)
				{
				if (szText[iCaretPos] == '0' && (iCaretPos == 0 ||
					 (!isdigit(szText[iCaretPos-1]) &&
					  szText[iCaretPos-1] != *Format.szSeparator)))
					fRetCode = TRUE;
				}
			}
		}

   UnlockNumHeader(hInfo);
	}

return(fRetCode);
}


// RFW - 8/13/04 - 15035
void Num_ProcessWMKeyUp(HWND hWnd, WPARAM wParam)
{
LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
	{
	lpInfo->wKeyDownVirtualKeyCode = 0;
   UnlockNumHeader(hInfo);
	}
}


void Num_SetInitialCaretPosition(HWND hWnd)
{
LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
	{
	Num_SetCaretLeftOfDec(lpInfo, NULL);
   UnlockNumHeader(hInfo);
	}
}


void Num_ProcessWMIniChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
/*
double dfVal;

LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
	{
   Num_GetValue(lpInfo, &dfVal);
	NumGetSystemIntlSettings(&NumIntlSettings);
	fNumIntlSettingsSet = TRUE;
   Num_SetValueInternal(lpInfo, dfVal);
   UnlockNumHeader(hInfo);
	}
*/
}


BOOL Num_ProcessEMSetFormat(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
LPSSNUM_FORMAT nf = (LPSSNUM_FORMAT)lParam;
double         dfVal;
SSNUM_FORMAT   Format;
TCHAR          szText[NUM_BUFFSIZE];
LPTSTR         lpszTemp;


LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
   {
   Num_GetValue(lpInfo, &dfVal);
	lpInfo->Format = *nf;
	Num_ResetSpinBtn(lpInfo);

   Num_SetValueInternal(lpInfo, dfVal);

   // Calc the size of the Suffix

	Num_GetDisplayFormat(&lpInfo->Format, &Format);

	// Calc the size of the Suffix for Negative numbers
	fpIntlDoubleToString(szText, -1.0, 0, Format.szDecimal,
								Format.fShowSeparator ? Format.szSeparator : NULL,
								Format.bLeadingZero, TRUE, Format.bNegStyle, FALSE,
								MAKEWORD(Format.bNegCurrencyStyle, Format.bPosCurrencyStyle),
								Format.fShowCurrencySymbol ? Format.szCurrency : _T(""),
								NumIntlSettings.negative_sign, Format.fCurrency);

	if (lpszTemp = _tcsstr(szText, _T("1")))
		lpInfo->nWidthSufNeg = (BYTE)(lstrlen(szText) - (lpszTemp - szText) - 1); // RFW - 6/14/04 - 14577
	else
		lpInfo->nWidthSufNeg = 0;

	// Calc the size of the Suffix for Positive numbers
	fpIntlDoubleToString(szText, 1.0, 0, Format.szDecimal,
								Format.fShowSeparator ? Format.szSeparator : NULL,
								Format.bLeadingZero, TRUE, Format.bNegStyle, FALSE,
								MAKEWORD(Format.bNegCurrencyStyle, Format.bPosCurrencyStyle),
								Format.fShowCurrencySymbol ? Format.szCurrency : _T(""),
								NumIntlSettings.negative_sign, Format.fCurrency);

	if (lpszTemp = _tcsstr(szText, _T("1")))
		lpInfo->nWidthSufPos = (BYTE)(lstrlen(szText) - (lpszTemp - szText) - 1); // RFW - 6/14/04 - 14577
	else
		lpInfo->nWidthSufPos = 0;

   UnlockNumHeader(hInfo);
   return (TRUE);
   }

return (FALSE);
}


BOOL Num_ProcessEMGetFormat(HWND hWnd, LPARAM lParam)
{
LPSSNUM_FORMAT ff;

LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
   {
   ff = (LPSSNUM_FORMAT)lParam;
	*ff = lpInfo->Format;
   UnlockNumHeader(hInfo);
   return (TRUE);
   }

return (FALSE);
}


static int Num_GetDecimalPos(LPEDITNUMINFO lpInfo, LPTSTR szText)
{
LPEDITFIELD  lpField = NULL;
SSNUM_FORMAT Format;
LPTSTR       lpszTemp;
LPTSTR       lpszStart;
int          iDecimalPos;
int          iCurrencyLen;

if (!szText)
	{
   lpField = LockField(lpInfo->hWnd);
   szText = lpField->lpszString;
	}

Num_GetDisplayFormat(&lpInfo->Format, &Format);

// RFW - 12/15/03 - 13099
iCurrencyLen = lstrlen(Format.szCurrency);
if (iCurrencyLen && _tcsnicmp(szText, Format.szCurrency, iCurrencyLen) == 0)
	lpszStart = szText + iCurrencyLen;
else
	lpszStart = szText;

if (Format.szDecimal[0] && (lpszTemp = _tcsstr(lpszStart, Format.szDecimal)))
   iDecimalPos = lstrlen(szText) - lstrlen(lpszTemp);
else
	{
   iDecimalPos = lstrlen(szText);
	if ((lpInfo->dfVal == dfZero && lpInfo->fNegative) || lpInfo->dfVal < dfZero)
		iDecimalPos -= lpInfo->nWidthSufNeg;
	else
		iDecimalPos -= lpInfo->nWidthSufPos;
	}

if (lpField)
   UnlockField(lpInfo->hWnd);

return (iDecimalPos);
}


// Set caret in front of the decimal place

static void Num_SetCaretLeftOfDec(LPEDITNUMINFO lpInfo, LPTSTR szText)
{
int iDecimalPos;

iDecimalPos = Num_GetDecimalPos(lpInfo, szText);
SendMessage(lpInfo->hWnd, EM_SETCARETPOS, iDecimalPos, 0l);

if (GetOverstrikeMode(lpInfo->hWnd, FALSE))
   SetInsertMode(lpInfo->hWnd, FALSE);
}


// Set caret right of the decimal place

static void Num_SetCaretRightOfDec(LPEDITNUMINFO lpInfo, LPTSTR szText)
{
int iDecimalPos;

iDecimalPos = Num_GetDecimalPos(lpInfo, szText) + 1;

if (iDecimalPos <= lstrlen(szText))
	{
	SendMessage(lpInfo->hWnd, EM_SETCARETPOS, iDecimalPos, 0l);

	if (!GetOverstrikeMode(lpInfo->hWnd, FALSE))
		SetOverstrikeMode(lpInfo->hWnd, FALSE);
	}
}


static void Num_SetValueInternal(LPEDITNUMINFO lpInfo, double dfVal)
{
LPEDITFIELD  lpField;
TCHAR szText[NUM_BUFFSIZE];
int   iDistFromRHS;

lpField = LockField(lpInfo->hWnd);

lpInfo->dfVal =  dfVal;
if (dfVal == dfZero)
	{
	if (lpInfo->fNegative)
		dfVal = -HUGE_VAL; // -0.0

   Num_ValueToString(&lpInfo->Format, dfVal, szText);

	if (!GetOverstrikeMode(lpInfo->hWnd, FALSE))
		Num_SetCaretLeftOfDec(lpInfo, lpField->lpszString);
   }

else
   Num_ValueToString(&lpInfo->Format, dfVal, szText);

iDistFromRHS = lstrlen(lpField->lpszString) - lpField->iCurrentPos;

lpInfo->iPrevStrLen = lstrlen(szText);
CallWindowProc((WNDPROC)GetDefWndProc(lpInfo->hWnd), lpInfo->hWnd, WM_SETTEXT, 0, (LPARAM)szText);

// RFW - 6/19/02 - RUN_CEL_005_010
//lpField->iCurrentPos = lstrlen(lpField->lpszString) - iDistFromRHS;
PositionCaret(lpInfo->hWnd, lpField, lstrlen(lpField->lpszString) - iDistFromRHS);

UnlockField(lpInfo->hWnd);
}


void Num_ResetPrevLen(HWND hWnd)
{
LockNumHeader(lpInfo, hInfo, hWnd);
lpInfo->iPrevStrLen = 0;
UnlockNumHeader(hInfo);
}


void Num_ReplaceText(HWND hWnd, LPARAM lParam)
{
TCHAR        lpszLocal[MAXFIELD + 1];
TCHAR        lpszLocalNew[MAXFIELD + 1];
LPEDITFIELD  lpField;
LPTSTR       lpNewText = (LPTSTR)lParam;
LPTSTR       Ptr;
SSNUM_FORMAT Format;
short        dDecPosNew = -1;
short        dDecPosString = -1;
short        iHiStart;
short        iHiEnd;
short        nAt;
BOOL         fHighlightLeftOfDec = FALSE;

LockNumHeader(lpInfo, hInfo, hWnd);
lpField = LockField(hWnd);

Num_GetDisplayFormat(&lpInfo->Format, &Format);

for (nAt = 0; *lpNewText; lpNewText++)
	{
	if (isdigit(*lpNewText))
		lpszLocalNew[nAt++] = *lpNewText;
	else if (*lpNewText == *Format.szDecimal)
		{
	   dDecPosNew = nAt;
		lpszLocalNew[nAt++] = *lpNewText;
		}
	else if (*lpNewText == '-')
		lpszLocalNew[nAt++] = *lpNewText;
	}

lpszLocalNew[nAt] = '\0';
_fmemset(lpszLocal, '\0', sizeof(lpszLocal));

/**********************************************
* Determine if old string has a decimal place
**********************************************/

// RFW - 12/15/03 - 13099
iHiStart = lpField->iHiStart;
if (Format.szDecimal[0])
	{
	int iCurrencyLen = lstrlen(Format.szCurrency);
	if (iCurrencyLen && _tcsnicmp(lpField->lpszString, Format.szCurrency, iCurrencyLen) == 0)
		iHiStart = max(iHiStart, iCurrencyLen);
	}

if (Format.szDecimal[0] && (Ptr = _tcsstr(&lpField->lpszString[iHiStart], Format.szDecimal)))
   {
   fHighlightLeftOfDec = TRUE;

   if (lpField->iHiEnd > Ptr - (LPTSTR)lpField->lpszString)
      dDecPosString = (short)(Ptr - lpField->lpszString);
   }

StrnnCpy(lpszLocal, lpField->lpszString, lpField->iHiStart + 1);

iHiEnd = min(lpField->iHiEnd, (int)lstrlen(lpField->lpszString) - 1);

if (Format.bFractionalWidth == 0 || (fHighlightLeftOfDec && dDecPosString == -1))
   {
   StrnCat(lpszLocal, lpszLocalNew, dDecPosNew >= 0 ? dDecPosNew :
           lstrlen(lpszLocalNew));
   }

else if (fHighlightLeftOfDec && dDecPosString >= 0)
   {
   StrnCat(lpszLocal, lpszLocalNew, dDecPosNew >= 0 ? dDecPosNew :
           lstrlen(lpszLocalNew));

   lpszLocal[lstrlen(lpszLocal)] = *Format.szDecimal;

   if (dDecPosNew >= 0)
     Num_CopyDecDigits(&lpszLocal[lstrlen(lpszLocal)],
                       &lpszLocalNew[dDecPosNew + 1], iHiEnd - dDecPosString);
   }

else if (!fHighlightLeftOfDec)
   Num_CopyDecDigits(&lpszLocal[lstrlen(lpszLocal)],
                     &lpszLocalNew[dDecPosNew + 1], iHiEnd - lpField->iHiStart);

lpField->iCurrentPos = lstrlen(lpszLocal);

if (lpField->iHiEnd < (int)lstrlen(lpField->lpszString))
  StrCat(lpszLocal, &lpField->lpszString[lpField->iHiEnd]);

if (Format.bFractionalWidth && lpField->iCurrentPos == lstrlen(lpszLocal))
   lpField->iCurrentPos--;

lstrcpy((LPTSTR)lParam, lpszLocal);

UnlockField(hWnd);
UnlockNumHeader(hInfo);
}


// This function was added to handle copying text that have an
// inconsistent number of decimal places

static void Num_CopyDecDigits(LPTSTR lpszDest, LPTSTR lpszSrc, int iLen)
{
_fmemset(lpszDest, '0', iLen);
lpszDest[iLen] = '\0';

while (iLen-- > 0 && *lpszSrc && isdigit(*lpszSrc))
   *lpszDest++ = *lpszSrc++;
}


// This function is used while editing and we need to check
// the validity of a range.

BOOL Num_IsValInRange(LPEDITNUMINFO lpInfo, double dfVal)
{
BOOL fRet = FALSE;

if (lpInfo->dfMinVal <= dfVal && dfVal <= lpInfo->dfMaxVal)
   fRet = TRUE;

//  for case the range is 500 to 5000
//  we must return TRUE for 0 to 500
if (!fRet && lpInfo->dfMinVal > dfZero &&
    dfVal >= dfZero && dfVal < lpInfo->dfMinVal)
   fRet = TRUE;

// converse
if (!fRet && lpInfo->dfMaxVal < dfZero &&
    dfVal <= dfZero && dfVal > lpInfo->dfMaxVal)
   fRet = TRUE;

return (fRet);
}


void Num_ProcessWMSize(HWND hWnd, LPARAM lParam)
{
LPEDITFIELD lpField;

LockNumHeader(lpInfo, hInfo, hWnd);
lpField = LockField(hWnd);

if (lpInfo->hWndSpinBtn)
   Num_ResetSpinBtn(lpInfo);
else
   lpField->xRightNonClientXtra = 0;

UnlockField(hWnd);
UnlockNumHeader(hInfo);
}

void Num_ResetSpinBtn(LPEDITNUMINFO lpInfo)
{
LPEDITFIELD lpField;
RECT        Rect;
short       SBWidth = 0;
short       SBHeight = 0;
BOOL        fShowSpin = lpInfo->Format.fSpin;
BOOL        fDone = FALSE;

lpField = LockField(lpInfo->hWnd);

GetClientRect(lpInfo->hWnd, &Rect);

if (fShowSpin)
   {
	SBHeight = (short)min(Rect.bottom - Rect.top, (lpField->yHeight * 3) / 2);
   SBWidth = (SBHeight * 2) / 3;
   }

lpField->xRightNonClientXtra = SBWidth;

if (lpField->f3D)
   {
   InflateRect(&Rect, -lpField->Edit3DFormat.dShadowSize,
               -lpField->Edit3DFormat.dShadowSize);

   if (lpField->Edit3DFormat.wStyle & EDIT3D_INSIDEBORDER)
      InflateRect(&Rect, -1, -1);
   }

if (fShowSpin && !lpInfo->hWndSpinBtn)
   {
   lpInfo->hWndSpinBtn = CreateWindow(lpszSpinClassName, NULL, WS_CHILD |
                                      WS_VISIBLE | SBNS_BORDERLEFT,
                                      Rect.right - SBWidth, Rect.top,
                                      SBWidth, SBHeight, lpInfo->hWnd, 0,
                                      GetWindowInstance(lpInfo->hWnd), NULL);

  if (GetWindowLong(lpInfo->hWnd, GWL_STYLE) & ES_STATIC)
      EnableWindow(lpInfo->hWndSpinBtn, FALSE);
   }

else if (!fShowSpin && lpInfo->hWndSpinBtn)
   {
   DestroyWindow(lpInfo->hWndSpinBtn);
   lpInfo->hWndSpinBtn = 0;
   }

else if (fShowSpin && lpInfo->hWndSpinBtn)
   MoveWindow(lpInfo->hWndSpinBtn, Rect.right - SBWidth, Rect.top, SBWidth,
              SBHeight, TRUE);

else
	fDone = TRUE;

if (lpInfo->hWndSpinBtn)
   SendMessage(lpInfo->hWndSpinBtn , EM_SETSPINSTYLE, 0, (LPARAM)lpField->fEnhanced);

if (!fDone)
	{
	InvalidateRect(lpInfo->hWnd, NULL, TRUE);
	UpdateWindow(lpInfo->hWnd);
	}

UnlockField(lpInfo->hWnd);
}


void Num_ProcessWMVScroll(HWND hWnd, int nScrollCode)
{
double dfNumOld;
double dfNum;
double dfInc;

LockNumHeader(lpInfo, hInfo, hWnd);

if (!lpInfo->hWndSpinBtn || (nScrollCode != SB_LINEDOWN && nScrollCode != SB_LINEUP))
   {
   UnlockNumHeader(hInfo);
   return;
   }

Num_GetValue(lpInfo, &dfNum);
dfNumOld = dfNum;

if (GetOverstrikeMode(hWnd, LOCAL))     // Overstrike mode
	dfInc = lpInfo->Format.dfSpinInc - (double)(long)lpInfo->Format.dfSpinInc;
else
	dfInc = (double)(long)lpInfo->Format.dfSpinInc;

switch (nScrollCode)
   {
   // RDW - Rewrite to prevent overflow
   case SB_LINEUP:
		// RFW - 6/13/04 - 14563
		if (dfNum + dfInc < lpInfo->dfMinVal)
         dfNum = lpInfo->dfMinVal;
      else if (dfNum < lpInfo->dfMaxVal - dfInc)
         dfNum += dfInc;
      else if (dfNum < lpInfo->dfMaxVal)
         dfNum = lpInfo->dfMaxVal;
      else if (lpInfo->Format.fSpinWrap) // && dfNum = lpInfo->dfMaxVal (implied)
         dfNum = lpInfo->dfMinVal;
      break;

   // RDW - Rewrite to prevent overflow
   case SB_LINEDOWN:
      if (dfNum > lpInfo->dfMinVal + dfInc)
         dfNum -= dfInc;
      else if (dfNum > lpInfo->dfMinVal)
         dfNum = lpInfo->dfMinVal;
      else if (lpInfo->Format.fSpinWrap) // && dfNum = lpInfo->dfMinVal (implied)
         dfNum = lpInfo->dfMaxVal;
      break;
   }

if (dfNumOld != dfNum)
   {
	LPEDITFIELD lpField = LockField(hWnd);
	BOOL fOverstrike = GetOverstrikeMode(lpInfo->hWnd, FALSE);

   EditFieldModifed(hWnd);
   Num_SetValue(lpInfo, dfNum);
	if (!fOverstrike)
		Num_SetCaretLeftOfDec(lpInfo, lpField->lpszString);
//	else
//		Num_SetCaretRightOfDec(lpInfo, lpField->lpszString);

	UnlockField(hWnd);
   UpdateWindow(hWnd);
   }

UnlockNumHeader(hInfo);
}


void Num_ProcessWMEnable(HWND hWnd, WPARAM wParam)
{
LockNumHeader(lpInfo, hInfo, hWnd);

if (lpInfo->hWndSpinBtn)
   EnableWindow(lpInfo->hWndSpinBtn, (BOOL)wParam);

UnlockNumHeader(hInfo);
}
