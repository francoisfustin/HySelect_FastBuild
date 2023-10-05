/*
$Revision:   1.2  $
*/

/*
$Log:   G:/DRLIBS/TOOLBOX/EDIT/EDITDATE/STATIC.C_V  $
 * 
 *    Rev 1.2   10 Jul 1990 10:03:40   Sonny
 * Fixed a bug in DateIsInRange (if range is not define then the function
 * returns without unlocking its memory block!) Randallyou will pay for this!
 * 
 *    Rev 1.1   12 Jun 1990 14:09:28   Dirk
 * No change.
 * 
 *    Rev 1.0   28 May 1990 15:34:02   Randall
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <tchar.h>
#include <ctype.h>
#include <string.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "editdate.h"
#include "calmonth.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editdate\editdate.h"
//borland #include "editdate\calmonth.h"
<-BORLAND*/
#endif

//- Added functions to support DBCS separators (Masanori Iwasa)
static BOOL CheckSeparators(LPTSTR szDate, LPDATEFORMAT datefmt);
int GetDateLength(LPDATEFORMAT datefmt);

//- Global variables for DBCS separators (Masanori Iwasa)
extern TCHAR    szYearGlobal[8];
extern TCHAR    szMonthGlobal[8];
extern TCHAR    szDayGlobal[8];

//--------------------------------------------------------------------------//

/*
BOOL DLLENTRY DateStringIsValid( LPCTSTR lpszDateString, LPDATEFORMAT datefmt)
{
   return (xDateStringIsValid(lpszDateString, datefmt, TRUE));
}
*/

BOOL DLLENTRY DateStringIsValidEx(LPCTSTR lpszDateString, LPDATEFORMAT datefmt,
                                  short nTwoDigitYearMax)
{
   return (xDateStringIsValid(lpszDateString, datefmt, TRUE,
                              TWODIGITYEARMAX_DEF));
}

//--------------------------------------------------------------------------//

BOOL DLLENTRY xDateStringIsValid(LPCTSTR lpszDateString, LPDATEFORMAT datefmt,
                                 BOOL fCheckYear, short nTwoDigitYearMax)
{
   TCHAR    szDate[ DATELENGTH];
   int      nLength;
   TB_DATE  date;

   // Check overall string length
   nLength = StrLen( lpszDateString);
   if ( nLength != GetDateLength(datefmt)) return FALSE;

   // Check the number of delimiters or for Japanese separators (Masanori Iwasa)
   if (!CheckSeparators((LPTSTR)lpszDateString, datefmt)) return FALSE;

   // Check for embedded blanks
   if ( datefmt->cSeparator != ' ')
      if ( StrChr( lpszDateString, ' '))
         return FALSE;

   // Copy into local string
   StrCpy( szDate, lpszDateString);

   /* Break out the individual date components */
   DateStringToDMYEx(szDate, &date, datefmt, nTwoDigitYearMax);
   if (date.nYear < 0 || date.nMonth < 0 || date.nDay < 0) return FALSE;
    
   if (fCheckYear)
       if (date.nYear < LOYEAR || date.nYear > HIYEAR) return FALSE;
    
   if (date.nMonth < 1 || date.nMonth > 12) return FALSE;

   if (date.nDay < 1 || date.nDay > CalMonth[date.nMonth - 1].nDays[ ISLEAP(date.nYear)]) return FALSE;

   return TRUE;
}
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// DateIntIsValid  - check integers d, m, y for valid date     spm
//////////////////////////////////////////////////////////////////////////////
BOOL DLLENTRY DateIntIsValid( LPTB_DATE da)
{
   // Check Year:
   if ( da->nYear < 0)
      return FALSE;
   // Check Month:
   if ( da->nMonth < 1 || da->nMonth > 12)
      return FALSE;
   // Check Days:
   if ( ( da->nDay < 1) || ( da->nDay > CalMonth[ da->nMonth - 1].nDays[ ISLEAP( da->nYear)]))
      return FALSE;
   // Ok, passed all above, so it's valid:
   return TRUE;
}  //// End of DateIntIsValid function ////

//--------------------------------------------------------------------------//

LONG  DateStaticValidate (HWND hWnd, LPARAM* lParam)

{
   LPDATEFIELD lpField;
   int         nLength;
   BOOL        bAddZero;
   BOOL        Ok = TRUE;
   DATEFORMAT  df;

   static      TCHAR szLocal[LOCAL_LENGTH];

   lpField = LockDateField (hWnd);
   df = lpField->df;

   StrnnCpy (szLocal, (LPCTSTR)*lParam, LOCAL_LENGTH);
   nLength = StrLen (StrTrim(szLocal));

   // Allow the user to set it to the null string
   if (nLength == 0)
      Ok = TRUE;

   else
      {
      // If we're only one character short,
      //    assume its because the leading 0 has been omitted
      bAddZero = (nLength == GetDateLength(&df) - 1) ? TRUE : FALSE;

      if (bAddZero)
      {
         StrInsertChar ('0', szLocal, 0);
         ++nLength;
      }

      Ok = DateStringIsValidEx(szLocal, &df, lpField->nTwoDigitYearMax);

      if (Ok)
         Ok = DateIsInRange (hWnd, szLocal, &df);

      if (Ok)
         *lParam = (LPARAM) (LPTSTR) szLocal;    // Replace the original string.
      }

   UnlockDateField (hWnd);

   return (LONG) Ok;
}

//--------------------------------------------------------------------------//

BOOL  DateFinalValidate (HWND hWnd)
{
   LPDATEFIELD lpField;
   BOOL        Ok;
   DATEFORMAT  df;

   TCHAR       szLocal[LOCAL_LENGTH];

   lpField = LockDateField (hWnd);
   _fmemcpy(&df, &lpField->df, sizeof(DATEFORMAT));

   MemSet(szLocal, '\0', sizeof(szLocal));
   SendMessage(hWnd, WM_GETTEXT, LOCAL_LENGTH, (LPARAM)(LPTSTR)szLocal);

   Ok = DateStringIsValidEx(szLocal, &df, lpField->nTwoDigitYearMax);

   if (Ok)
      Ok = DateIsInRange (hWnd, szLocal, &df);

   // BUG SPR_EVN_001_005 (2-1)
   // When User Leaves A Date Or Time Cell Without Input Anything (szLocal Is Empty),
   // Set Ok To TRUE To Prevent EditError Event Be Fired.
   // Modified By HaHa 1999.12.6
   if (!StrLen(szLocal)) Ok=TRUE;

   UnlockDateField (hWnd);

   return Ok;
}

//--------------------------------------------------------------------------//

BOOL DateIsInRange (HWND hWnd, LPTSTR lpszString, LPDATEFORMAT lpdf)

{
   LPDATEFIELD lpField;
   TB_DATE     Date;
   LONG        lDate;

   lpField = LockDateField (hWnd);
   if (lpField)
   {
#if 0
      // Decode the date to be tested
      DateStringToDMY (lpszString, &Date, lpdf);
      lDate = DateDMYToJulian (&Date);

      if (lpField->lRangeMin != UNDEFINED && lpField->lRangeMax != UNDEFINED)
           if (lDate < lpField->lRangeMin)
               lDate = lpField->lRangeMin;
           else if (lDate > lpField->lRangeMax)
               lDate = lpField->lRangeMax;

      DateJulianToDMY (lDate, &Date);
      DateDMYToString (&Date, lpszString, lpdf);
#endif

      // Decode the date to be tested
      DateStringToDMYEx(lpszString, &Date, lpdf, lpField->nTwoDigitYearMax);
      lDate = DateDMYToJulian (&Date);

      if (lpField->lRangeMin != UNDEFINED && lpField->lRangeMax != UNDEFINED)
           {
           if (lDate < lpField->lRangeMin)
               {
#ifndef SPREAD_JPN
               if (!lpField->fStrictValidate)
                  {
						TB_DATE DateTemp;
                  DateJulianToDMY(lpField->lRangeMin, &DateTemp);
                  if (Date.nYear < DateTemp.nYear)
                     {
                     Date.nYear = DateTemp.nYear;
                     lDate = DateDMYToJulian(&Date);
                     }
                  }
#endif //!SPREAD_JPN
				//------------------------------------------------------
               if (lDate < lpField->lRangeMin)
                  lDate = lpField->lRangeMin;
               }

           if (lDate > lpField->lRangeMax)
               {
#ifndef SPREAD_JPN
               if (!lpField->fStrictValidate)
                  {
						TB_DATE DateTemp;
                  DateJulianToDMY(lpField->lRangeMax, &DateTemp);
                  if (Date.nYear > DateTemp.nYear)
                     {
                     Date.nYear = DateTemp.nYear;
                     lDate = DateDMYToJulian(&Date);
                     }
                  }
#endif //!SPREAD_JPN
				//------------------------------------------------------

               if (lDate > lpField->lRangeMax)
                  lDate = lpField->lRangeMax;
               }

           lDate = max(lDate, lpField->lRangeMin);
           }

      DateJulianToDMY (lDate, &Date);
      DateDMYToString (&Date, lpszString, lpdf);

      UnlockDateField (hWnd);
   }
   return TRUE;
}


//---------------------------------------------------------------------
// Check to see if right amount of separators etc.. (Masanori Iwasa)
//---------------------------------------------------------------------
BOOL CheckSeparators(LPTSTR szDate, LPDATEFORMAT datefmt)
{
    int    SeparatorNum[] = {2, 2, 2, 2, 1, 1, 2, 2, 2};
    int    Weights[] = {76, 76, 80, 88, 37, 47, 88, 88, 88};
    LPTSTR szTemp = szDate;
    TCHAR  Temp[3] = _T("  ");
    int    SepNum = 0, SepWeight = 0, TotalWeight = 0;

    if (!IsDbcsSeparator(datefmt->cSeparator))
        {
        szTemp += (datefmt->nFormat == IDF_NNNNYYMMDD) ? 4 : 2;

        for(; *szTemp; szTemp++)
            if ((TUCHAR)*szTemp == datefmt->cSeparator) SepNum++;
        if (SepNum == SeparatorNum[datefmt->nFormat - IDF_DDMONYY])
            return TRUE;
        else
            return FALSE;
        }
    else{
        SepNum++;

#ifndef UNICODE
        for(; szTemp[0] && szTemp[1]; szTemp++){
            Temp[0] = szTemp[0];
            Temp[1] = szTemp[1];
            if (!lstrcmp(Temp, szYearGlobal)) SepWeight = 11;
            if (!lstrcmp(Temp, szMonthGlobal)) SepWeight = 13;
            if (!lstrcmp(Temp, szDayGlobal)) SepWeight = 17;
///rap 6/30/2004 - 14219
#else
        for(; szTemp[0]; szTemp++){
            Temp[0] = szTemp[0];
            if (Temp[0] == szYearGlobal[0]) SepWeight = 11;
            if (Temp[0] == szMonthGlobal[0]) SepWeight = 13;
            if (Temp[0] == szDayGlobal[0]) SepWeight = 17;
#endif
///rap 6/30/2004 - 14219
            if (SepWeight){
                TotalWeight += SepNum++ * SepWeight;
                SepWeight = 0;
                }
            }
            if (TotalWeight == Weights[datefmt->nFormat - IDF_DDMONYY])
                return TRUE;
            else
                return FALSE;
        }                               
}


//---------------------------------------------------------------------
// Get length of date string according to date types. (Masanori Iwasa)
//---------------------------------------------------------------------
int GetDateLength(LPDATEFORMAT datefmt)
{
    int LenLimit;

    switch(datefmt->nFormat){
        case IDF_DDMONYY:
            LenLimit = datefmt->bCentury ? 11 : 9;
#ifdef UNICODE
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 1;
#else
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 4;
#endif
            break;

        case IDF_DDMMYY:
        case IDF_MMDDYY:
        case IDF_YYMMDD:
            LenLimit = datefmt->bCentury ? 10 : 8;
#ifdef UNICODE
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 1;
#else
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 4;
#endif
            break;

        //---------------------------------------------------------------------------
        // Date input formats that support Japanese Era (Masanori Iwasa)
        //---------------------------------------------------------------------------
        case IDF_YYMM:
            LenLimit = datefmt->bCentury ? 7 : 5;
#ifdef UNICODE
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 1;
#else
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 3;
#endif
            break;
            
        case IDF_MMDD:
            LenLimit = 5;
#ifdef UNICODE
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 1;
#else
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 3;
#endif
            break;
            
        case IDF_NYYMMDD:
            LenLimit = 9;
#ifdef UNICODE
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 1;
#else
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 4;
#endif
            break;

        case IDF_NNYYMMDD:
//            LenLimit = 8 + (2 / sizeof(TCHAR)) + (IsDbcsSeparator(datefmt->cSeparator) ? 4 : 0);
            LenLimit = 9;
#ifdef UNICODE
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 1;
#else
            LenLimit += (IsDbcsSeparator(datefmt->cSeparator)) ? 5 : 1;
#endif
            break;

        case IDF_NNNNYYMMDD:
//rap 6/30/2004 - 14219
//            LenLimit = 8 + (4 / sizeof(TCHAR)) + (IsDbcsSeparator(datefmt->cSeparator) ? 4 : 0);
            LenLimit = 10;
#ifdef UNICODE
            if (IsDbcsSeparator(datefmt->cSeparator)) LenLimit += 1;
#else
            LenLimit += (IsDbcsSeparator(datefmt->cSeparator)) ? 6 : 2;
#endif
            break;
        }

    return LenLimit;
}           
