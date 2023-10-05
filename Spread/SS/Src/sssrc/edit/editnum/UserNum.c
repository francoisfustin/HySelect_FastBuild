
#define  NOCOMM             // COMM driver routines
#define  NOKANJI            // Kanji support stuff.

#define  DRV_EDIT       // to include our struct defns and func defns

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <locale.h>
#include <toolbox.h>

#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editnum.h"

extern SSNUM_INTLSETTINGS NumIntlSettings;
extern BOOL fNumIntlSettingsSet;

//--------------------------------------------------------------------------//
#ifdef SS_V80
void NumSetSpinStyle (HWND hWnd, BOOL fEnhanced)

{
	LPEDITFIELD lpField = LockField(hWnd);
	lpField->fEnhanced = fEnhanced;
	UnlockField(hWnd);
}
#endif

BOOL NumSetRange(HWND hWnd, double Min, double Max)
{
BOOL fRet;

LockNumHeader(lpInfo, hInfo, hWnd);
fRet = Num_SetRange(lpInfo, Min, Max);
UnlockNumHeader(hInfo);
return (fRet);
}


BOOL NumGetValue(HWND hWnd, LPDOUBLE lpDouble)
{
BOOL fRet = FALSE;

LockNumHeader(lpInfo, hInfo, hWnd);
if (lpInfo)
	{
   Num_GetValue(lpInfo, lpDouble);
   UnlockNumHeader(hInfo);
   fRet = TRUE;
	}

return (fRet);
}


BOOL NumSetValue(HWND hWnd, double dfDouble)
{
BOOL fRet = FALSE;

LockNumHeader(lpInfo, hInfo, hWnd);

if (lpInfo)
	{
	if (Num_CheckRange(lpInfo, dfDouble))
		{
		Num_SetValue(lpInfo, dfDouble);
	   fRet = TRUE;
		}

   UnlockNumHeader(hInfo);
	}

return (fRet);
}


BOOL NumSetFormat(HWND hWnd, LPSSNUM_FORMAT lpff)
{
SendMessage(hWnd, EM_SETFORMAT, 0, (LPARAM)lpff);
return (TRUE);
}


BOOL NumStringToValue(LPSSNUM_FORMAT lpFormat, LPCTSTR szString, LPDOUBLE lpdfVal)
{
return (Num_StringToValue(lpFormat, szString, lpdfVal));
}


BOOL NumValueToString(LPSSNUM_FORMAT lpFormat, double dfVal, LPTSTR szString)
{
return (Num_ValueToString(lpFormat, dfVal, szString));
}


void NumGetDisplayFormat(LPSSNUM_FORMAT lpFormatSrc, LPSSNUM_FORMAT lpFormat)
{
Num_GetDisplayFormat(lpFormatSrc, lpFormat);
}


void NumGetSystemIntlSettings(LPSSNUM_INTLSETTINGS lpIntlSettings)
{
if (!fNumIntlSettingsSet)
	{
	NumReadSystemIntlSettings(&NumIntlSettings);
	fNumIntlSettingsSet = TRUE;
	}

if (lpIntlSettings)
	*lpIntlSettings = NumIntlSettings;
}


BOOL NumReadSystemIntlSettings(LPSSNUM_INTLSETTINGS lpIntlSettings)
{
struct lconv *plconv = NULL;
TCHAR szBuffer[10];
TCHAR *lpszLocale;
TCHAR *lpszLocaleSave;
BOOL fRet = TRUE;

lpszLocale = _tsetlocale(LC_ALL, NULL);
lpszLocaleSave = _tcsdup(lpszLocale);
_tsetlocale(LC_ALL, _T(""));
plconv = localeconv();

_fmemset(lpIntlSettings, '\0', sizeof(SSNUM_INTLSETTINGS));

//GAB 12/13/01 Modified for Unicode
#if defined(UNICODE) || defined(_UNICODE)
// Number related values
  MultiByteToWideChar(CP_ACP, 0, plconv->decimal_point, -1, lpIntlSettings->decimal_point, SSNUM_DECCHAR_SIZE + 1);
  MultiByteToWideChar(CP_ACP, 0, plconv->thousands_sep, -1, lpIntlSettings->thousands_sep, SSNUM_SEPCHAR_SIZE + 1);
  MultiByteToWideChar(CP_ACP, 0, plconv->grouping, -1, lpIntlSettings->grouping, SSNUM_GROUPING_SIZE + 1);

// Currency related values
  MultiByteToWideChar(CP_ACP, 0, plconv->currency_symbol, -1, lpIntlSettings->currency_symbol, SSNUM_CURCHAR_SIZE + 1);
  MultiByteToWideChar(CP_ACP, 0, plconv->mon_decimal_point, -1, lpIntlSettings->mon_decimal_point, SSNUM_DECCHAR_SIZE + 1);
  MultiByteToWideChar(CP_ACP, 0, plconv->mon_thousands_sep, -1, lpIntlSettings->mon_thousands_sep, SSNUM_SEPCHAR_SIZE + 1);
  MultiByteToWideChar(CP_ACP, 0, plconv->mon_grouping, -1, lpIntlSettings->mon_grouping, SSNUM_GROUPING_SIZE + 1);

  MultiByteToWideChar(CP_ACP, 0, plconv->positive_sign, -1, lpIntlSettings->positive_sign, SSNUM_POSSIGN_SIZE + 1);
  MultiByteToWideChar(CP_ACP, 0, plconv->negative_sign, -1, lpIntlSettings->negative_sign, SSNUM_NEGSIGN_SIZE + 1);
#else
// Number related values
  lstrcpyn(lpIntlSettings->decimal_point, plconv->decimal_point, SSNUM_DECCHAR_SIZE + 1);
  lstrcpyn(lpIntlSettings->thousands_sep, plconv->thousands_sep, SSNUM_SEPCHAR_SIZE + 1);
  lstrcpyn(lpIntlSettings->grouping, plconv->grouping, SSNUM_GROUPING_SIZE + 1);

// Currency related values
  lstrcpyn(lpIntlSettings->currency_symbol, plconv->currency_symbol, SSNUM_CURCHAR_SIZE + 1);
  lstrcpyn(lpIntlSettings->mon_decimal_point, plconv->mon_decimal_point, SSNUM_DECCHAR_SIZE + 1);
  lstrcpyn(lpIntlSettings->mon_thousands_sep, plconv->mon_thousands_sep, SSNUM_SEPCHAR_SIZE + 1);
  lstrcpyn(lpIntlSettings->mon_grouping, plconv->mon_grouping, SSNUM_GROUPING_SIZE + 1);

  lstrcpyn(lpIntlSettings->positive_sign, plconv->positive_sign, SSNUM_POSSIGN_SIZE + 1);
  lstrcpyn(lpIntlSettings->negative_sign, plconv->negative_sign, SSNUM_NEGSIGN_SIZE + 1);
#endif

lpIntlSettings->frac_digits = plconv->frac_digits;

_tsetlocale(LC_ALL, lpszLocaleSave);
free(lpszLocaleSave);

//GAB 12/10/01 Changed the atoi functions to TCHAR routine _ttoi
GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, szBuffer, 10);
lpIntlSettings->fLeadingZero = _ttoi(szBuffer);

GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER, szBuffer, 10);
lpIntlSettings->bNegStyle = _ttoi(szBuffer);

GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGCURR, szBuffer, 10);
lpIntlSettings->bNegCurrencyStyle = _ttoi(szBuffer);

GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, szBuffer, 10);
lpIntlSettings->bPosCurrencyStyle = _ttoi(szBuffer);

return (fRet);
}
