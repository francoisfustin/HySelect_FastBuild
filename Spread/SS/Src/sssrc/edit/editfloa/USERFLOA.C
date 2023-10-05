/*
$Revision:   1.2  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/EDITFLOA/USERFLOA.C_V  $
 *
 *    Rev 1.2   04 Dec 1990 13:40:14   Dirk
 * No change.
 *
 *    Rev 1.1   12 Jun 1990 14:10:26   Dirk
 * No change.
 *
 *    Rev 1.0   04 Jun 1990 14:56:00   Bruce
 * Initial revision.
 *
 *    Rev 1.8   28 May 1990 15:58:30   Sonny
 *
 *    Rev 1.7   23 May 1990 18:16:04   Sonny
 *
 *    Rev 1.6   23 May 1990 16:51:06   Sonny
 * No change.
 *
 *    Rev 1.5   22 May 1990 17:02:24   Sonny
 * No change.
 *
 *    Rev 1.4   21 May 1990 13:27:48   Randall
 *
 *    Rev 1.3   18 May 1990 11:49:42   Randall
 * Changed %e format to %f in FloatSetRange string
 *
 *    Rev 1.2   17 May 1990 17:29:10   Randall
 * Check class in SetFloatRange
 *
 *    Rev 1.1   15 May 1990 17:05:26   Sonny
 * No change.
 *
 *    Rev 1.0   11 May 1990 16:17:48   Bruce
 * Initial revision.
 *
 *    Rev 1.0   07 May 1990 16:16:22   Randall
 * Initial revision.
*/

#define  NOCOMM             // COMM driver routines
#define  NOKANJI            // Kanji support stuff.

#define  DRV_EDIT       // to include our struct defns and func defns

#include <ctype.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editfloa.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "editfloa\editfloa.h"
<-BORLAND*/
#endif

//--------------------------------------------------------------------------//

BOOL DLLENTRY FloatSetRange (HWND hWnd, double Min, double Max)

{
   TCHAR szString[800 + 1];

   if (Min > Max) return FALSE;
   StrPrintf (szString, _T("%f %f"), Min, Max);

   return
      (BOOL) SendMessage (hWnd, EM_SETRANGE, 0, (LPARAM) (LPTSTR) szString);
}


BOOL  DLLENTRY FloatGetValue(HWND hWnd, LPDOUBLE lpDouble)
{
#ifdef  BUGS
// Bug-002
TCHAR  szVal[330];
#else
TCHAR  szVal[40];
#endif

   LPTSTR szPtr = szVal;

   LockFloatHeader(lpInfo, hInfo, hWnd);
   if (lpInfo)
   {
      szPtr[0] = '\0';
#ifdef  BUGS
// Bug-002
      SendMessage(hWnd, WM_GETTEXT, 329, (LPARAM) (LPTSTR)szVal);
#else
      SendMessage(hWnd, WM_GETTEXT, 39, (LONG) (LPTSTR)szVal);
#endif

      if (InSet(lpInfo->style, FS_MONEY))
         StrDeleteCh(szVal, lpInfo->cCurrencySign);
      if (InSet(lpInfo->style, FS_SEPARATOR))
         StrDeleteCh(szVal, lpInfo->cSeparator);
      StrReplaceCh(szVal, MAKEWORD('.', lpInfo->cDecimalSign));
      UnlockFloatHeader(hInfo);
      StringToFloat(szVal, lpDouble);
      return(TRUE);
   }
   return(FALSE);
}


BOOL DLLENTRY FloatSetValue(HWND hWnd, double dfDouble)
{
#ifdef  BUGS
// Bug-002
   TCHAR  szVal[330];
#else
   TCHAR  szVal[40];
#endif

   LPTSTR szPtr = szVal;
   BYTE  bFractionalWidth;

   LockFloatHeader(lpInfo, hInfo, hWnd);

   if (lpInfo)
   {
      if (lpInfo->bFractionalWidth == 0)
         bFractionalWidth = 1;
      else
         bFractionalWidth = lpInfo->bFractionalWidth;

      StrPrintf(szVal, _T("%.*f"), (short)bFractionalWidth, dfDouble);
      if (lpInfo->cDecimalSign != '.')
         StrReplaceCh(szVal, MAKEWORD(lpInfo->cDecimalSign, '.'));

      if (lpInfo->bFractionalWidth == 0)
         szVal[lstrlen(szVal) - 2] = '\0';

      UnlockFloatHeader(hInfo);
      SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szVal);
      return(TRUE);
   }
   return(FALSE);
}


BOOL DLLENTRY FloatSetFormat (HWND hWnd, LPFLOATFORMAT ff)
{
SendMessage(hWnd, EM_SETFORMAT, 0, (LPARAM)ff);
return (TRUE);
}

//--------------------------------------------------------------------------//

BOOL DLLENTRY FloatGetFormat (HWND hWnd, LPFLOATFORMAT ff)
{
TCHAR szText[5];

if (hWnd == 0)
   {
   GetProfileString(WININTL, WIN_THOUSAND, WIN_DEFTHOUSAND, szText, 2);
   ff->cSeparator = szText[0];
   GetProfileString(WININTL, WIN_DECIMAL, WIN_DEFDECIMAL, szText, 2);
   ff->cDecimalSign = szText[0];
   GetProfileString(WININTL, WIN_CURRENCY, WIN_DEFCURRENCY, szText, 2);
   ff->cCurrencySign = szText[0];
   }

else
   SendMessage(hWnd, EM_GETFORMAT, 0, (LPARAM)ff);

return (TRUE);
}


BOOL DLLENTRY FloatSetMask(HWND hWnd, LPCTSTR szMask)
{
double dfVal;
BOOL   Ret;

LockFloatHeader(lpInfo, hInfo, hWnd);

FloatGetValue(hWnd, &dfVal);
Ret = GetFloatPicture(lpInfo, szMask);
FloatSetValue(hWnd, dfVal);

UnlockFloatHeader(hInfo);
return (Ret);
}


BOOL DLLENTRY FloatGetRange(HWND hWnd, LPDOUBLE lpMin, LPDOUBLE lpMax)

{
   LockFloatHeader(lpInfo, hInfo, hWnd);

   *lpMin = lpInfo->dfMinVal;
   *lpMax = lpInfo->dfMaxVal;

   UnlockFloatHeader(hInfo);

   return (TRUE);
}


void DLLENTRY FloatSetStyle(HWND hWnd, long Style)
{
double dfVal;

LockFloatHeader(lpInfo, hInfo, hWnd);

if (lpInfo)
   {
   lpInfo->style &= ~FS_MONEY;
   lpInfo->style &= ~FS_SEPARATOR;

   lpInfo->style |= Style;
   UnlockFloatHeader(hInfo);

   FloatGetValue(hWnd, &dfVal);
   FloatSetValue(hWnd, dfVal);
   }
}


#ifdef TBPRO

LPTSTR DLLENTRY FloatFormatString(double Val, LPTSTR szBuffer,
                                  BOOL fSeparator, BOOL fMoney,
                                  short iDecimalPlaces)
{
TCHAR         szText[3];
FLOATFORMAT   Format;

MemSet(&Format, '\0', sizeof(FLOATFORMAT));

if (fSeparator)
   {
   GetProfileString(WININTL, WIN_THOUSAND, WIN_DEFTHOUSAND, szText, 2);
   Format.cSeparator = szText[0];
   }

if (fMoney)
   {
   GetProfileString(WININTL, WIN_CURRENCY, WIN_DEFCURRENCY, szText, 2);
   Format.cCurrencySign = szText[0];
   }

return (FloatFormatStringExt(Val, szBuffer, &Format, iDecimalPlaces));
}


LPTSTR DLLENTRY FloatFormatStringExt(double Val, LPTSTR szBuffer,
                                     LPFLOATFORMAT lpFormat,
                                     short iDecimalPlaces)
{
EDITFLOATINFO Info;
TCHAR         szText[3];

if (!lpFormat->cDecimalSign)
   {
   GetProfileString(WININTL, WIN_DECIMAL, WIN_DEFDECIMAL, szText, 2);
   lpFormat->cDecimalSign = szText[0];
   }

_fmemset(&Info, '\0', sizeof(EDITFLOATINFO));

Info.cSeparator = lpFormat->cSeparator;
Info.cDecimalSign = lpFormat->cDecimalSign;
Info.cCurrencySign = lpFormat->cCurrencySign;

Info.bFractionalWidth = (BYTE)iDecimalPlaces;

if (lpFormat->cSeparator)
   Info.style |= FS_SEPARATOR;

if (lpFormat->cCurrencySign)
   Info.style |= FS_MONEY;

FloatFormat(&Info, &Val, szBuffer);

return (szBuffer);
}


BOOL DLLENTRY FloatGetMask(HWND hWnd, LPTSTR szMask)
{
short dAt = 0;
short i;
BOOL  Ret = TRUE;

LockFloatHeader(lpInfo, hInfo, hWnd);

for (i = 0; i < (short)lpInfo->bIntegerWidth; i++)
   szMask[dAt++] = '9';

if (lpInfo->bFractionalWidth)
   {
   szMask[dAt++] = '.';
   for (i = 0; i < (short)lpInfo->bFractionalWidth; i++)
      szMask[dAt++] = '9';
   }

szMask[dAt] = '\0';

UnlockFloatHeader(hInfo);
return (Ret);
}

#endif
