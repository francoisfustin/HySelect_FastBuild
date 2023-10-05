/*
$Revision:   1.1  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITDATE/DYNAMIC1.C_V  $
 * 
 *    Rev 1.1   12 Jun 1990 14:09:34   Dirk
 * No change.
 * 
 *    Rev 1.0   28 May 1990 15:34:12   Randall
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
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <tchar.h>
#include <toolbox.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editdate.h"
#include "calmonth.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "editdate\editdate.h"
//borland #include "editdate\calmonth.h"
<-BORLAND*/
#endif

//RWP01a
#ifdef SPREAD_JPN
extern void SS_BeepLowlevel(HWND hWnd);
#else
extern void SS_Beep2(HWND hWnd);
#endif
//RWP01a


/**********************
* Function Prototypes
**********************/

BOOL  MatchMonthAbbrev(LPTSTR lpszAbbrev, BOOL FAR *bUnique);
BOOL  DateValidateDate(HWND hWnd, LPTSTR lpszDate, short iPos, BOOL bCentury,
                       TUCHAR cSeparator, int nFormat, short First,
                       short Second, short Third, LPDATEEDITINFO lpEditInfo);
BOOL  DateValidateDD(HWND hWnd, LPTSTR lpszDate, LPTSTR lpszString, short iPos,
                     TCHAR ch, BOOL bCentury, TUCHAR cSeparator, int nFormat,
                     BOOL AdvanceCaret, LPDATEEDITINFO lpEditInfo);
BOOL  DateValidateMM(HWND hWnd, LPTSTR lpszDate, LPTSTR lpszString, short iPos,
                     TCHAR ch, BOOL bCentury, TUCHAR cSeparator, int nFormat,
                     BOOL AdvanceCaret, LPDATEEDITINFO lpEditInfo);
BOOL  DateValidateMON(HWND hWnd, LPTSTR lpszDate, LPTSTR lpszString, short iPos,
                      TCHAR ch, BOOL bCentury, TUCHAR cSeparator, int nFormat,
                      BOOL AdvanceCaret, LPDATEEDITINFO lpEditInfo);
BOOL  DateValidateYY(HWND hWnd, LPTSTR lpszDate, LPTSTR lpszString, short iPos,
                     TCHAR ch, BOOL bCentury, TUCHAR cSeparator, int nFormat,
                     BOOL AdvanceCaret, LPDATEEDITINFO lpEditInfo);
//- CAUTION -------------------------------------------------------------+
//- To intergrate the Japanese separators, (char cSeparator) was added   |
//  as the first parameter. (Masanori Iwasa)                             |
//-----------------------------------------------------------------------+-----------------
short DateGetDaysPerMonth(TUCHAR cSeparator, LPCTSTR lpszDate, int nFormat,
                          BOOL bCentury, short nTwoDigitYearMax);
//-----------------------------------------------------------------------------------------

void  DatePutChar(HWND hWnd, LPTSTR lpszString, TCHAR ch, short iPos,
                  LPDATEEDITINFO lpEditInfo);

//- CAUTION -------------------------------------------------------------+
//- To intergrate the Japanese separators, (LPSTR lpKanji) and           |
//  (BOOL bAdvanceCaret) were added as the fifth and sixth parameter.    }
// (Masanori Iwasa)                                                      |
//-----------------------------------------------------------------------+-----------------
void  DateAdvanceCaret(HWND hWnd, LPTSTR lpszDate, short iPos, TUCHAR cSeparator,
                       LPTSTR lpKanji, BOOL bAdvanceCaret, LPDATEEDITINFO lpEditInfo);
//-----------------------------------------------------------------------------------------

//- The following functions were added to intergrate Japanese Era
//  and separators etc.. (Masanori Iwasa)
BOOL DateValidateNengo(HWND hWnd, LPTSTR lpszDate, short iPos, TCHAR ch, BOOL bCentury, TUCHAR cSeparator,
                        short nFormat, LPDATEEDITINFO lpEditInfo, int NengoLen);
void DatePutStr(HWND hWnd, LPTSTR lpszString, LPTSTR lpStr, short iPos,
                LPDATEEDITINFO lpEditInfo);
BOOL DateValidateCentury(HWND hWnd, TCHAR cDigit1, TCHAR cDigit2);

extern TCHAR     szYearGlobal[8];            //- Global Year variable (Masanori Iwasa)
extern TCHAR     szMonthGlobal[8];           //- Global Month variable
extern TCHAR     szDayGlobal[8];             //- Global Day variable


LRESULT DateDynamicValidate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
short        iPos = (short)wParam;
LPTSTR       lpszString = (LPTSTR)lParam;
LPDATEFIELD  lpField;
DATEFORMAT   df;
DATEEDITINFO EditInfo;
LPEDITFIELD  lpEditField;
TUCHAR       cSeparator;
int          nLength;
int          nFormat;
TCHAR        szStringTemp[LOCAL_LENGTH];
BOOL         bCentury;
BOOL         Ok = TRUE;
int          LenLimit;

if (lpszString && *lpszString == '\0')
   return (TRUE);

if (iPos >= lstrlen(lpszString))
   return (DateStaticValidate(hWnd, &lParam));

lpField = LockDateField (hWnd);
bCentury = lpField->df.bCentury;
nFormat = lpField->df.nFormat;
cSeparator = lpField->df.cSeparator;
MemCpy(&df, &lpField->df, sizeof(DATEFORMAT));

lpEditField = LockField(hWnd);

if (!lpszString)
   lpszString = lpEditField->lpszString;

EditInfo.nChars = lpEditField->nChars;
EditInfo.iCurrentPos = lpEditField->iCurrentPos;

StrnnCpy (szStringTemp, lpszString, LOCAL_LENGTH);

nLength = StrLen(szStringTemp);

switch(nFormat){
     case IDF_DDMONYY:
         LenLimit = bCentury ? 11 : 9;
#ifndef _UNICODE
         if (IsDbcsSeparator(cSeparator)) LenLimit += 4;
#else
         if (IsDbcsSeparator(cSeparator)) LenLimit += 1;
#endif
         break;

     case IDF_DDMMYY:
     case IDF_MMDDYY:
     case IDF_YYMMDD:
         LenLimit = bCentury ? 10 : 8;
#ifndef _UNICODE
         if (IsDbcsSeparator(cSeparator)) LenLimit += 4;
#else
         if (IsDbcsSeparator(cSeparator)) LenLimit += 1;
#endif
         break;

     //------------------------------------------------------------------------
     // Date format types from here handle Era etc.. (Masanori Iwasa)
     //------------------------------------------------------------------------
     case IDF_YYMM:
         LenLimit = bCentury ? 7 : 5;
         if (IsDbcsSeparator(cSeparator)) LenLimit += 3;
         break;
         
     case IDF_MMDD:
         LenLimit = (IsDbcsSeparator(cSeparator)) ? 8 : 5;
         break;
         
     case IDF_NYYMMDD:
#ifdef _UNICODE
         LenLimit = (IsDbcsSeparator(cSeparator)) ? 10 : 9;
#else
         LenLimit = (IsDbcsSeparator(cSeparator)) ? 13 : 9;
#endif
         break;

     case IDF_NNYYMMDD:
#ifdef _UNICODE
         LenLimit = (IsDbcsSeparator(cSeparator)) ? 13 : 9;
#else
         LenLimit = (IsDbcsSeparator(cSeparator)) ? 14 : 10;
#endif
         break;
     case IDF_NNNNYYMMDD:
#ifdef _UNICODE
         LenLimit = (IsDbcsSeparator(cSeparator)) ? 14 : 10;
#else
         LenLimit = (IsDbcsSeparator(cSeparator)) ? 16 : 12;
#endif
         break;
     }           
 
if (nLength > LenLimit)
	Ok = FALSE;

// RFW - 8/17/04 - 15047
// I removed this fix for 13844 because it broke 15047.
// RFW - 8/13/04 - 13844
//if (nLength == LenLimit)
//   return (DateStaticValidate(hWnd, &lParam));

if (Ok)
   {
   switch (nFormat)
      {
      case IDF_DDMONYY:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_DD, DATE_MON, DATE_YY, &EditInfo);
         break;

      case IDF_DDMMYY:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_DD, DATE_MM, DATE_YY, &EditInfo);
         break;

      case IDF_MMDDYY:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_MM, DATE_DD, DATE_YY, &EditInfo);
         break;

      case IDF_YYMMDD:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_YY, DATE_MM, DATE_DD, &EditInfo);
         break;

      //------------------------------------------------------------------------
      // Date format types from here handle Era etc.. (Masanori Iwasa)
      //------------------------------------------------------------------------
      case IDF_YYMM:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_YY, DATE_MM, DATE_NULL, &EditInfo);
         break;

      case IDF_MMDD:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_MM, DATE_DD, DATE_NULL, &EditInfo);
         break;

      case IDF_NYYMMDD:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_NYY, DATE_MM, DATE_DD, &EditInfo);
         break;

      case IDF_NNYYMMDD:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_NNYY, DATE_MM, DATE_DD, &EditInfo);
         break;

      case IDF_NNNNYYMMDD:
         Ok = DateValidateDate(hWnd, szStringTemp, iPos, bCentury, cSeparator,
                               nFormat, DATE_NNNNYY, DATE_MM, DATE_DD, &EditInfo);
         break;
      }
   }

if (Ok)
   if (xDateStringIsValid(szStringTemp, &df, FALSE, lpField->nTwoDigitYearMax))
      Ok = DateIsInRange(hWnd, szStringTemp, &df);

UnlockDateField(hWnd);

if (!Ok)
// Modified by BOC FMH 1996.07.15. ----------------->>
//#ifdef MASA
#ifdef SPREAD_JPN
//--------------------<<
// 96' 6/26 Modified by BOC Gao. changed MessageBeep to SS_BeepLowlevel
            SS_BeepLowlevel(hWnd);
// ------------------------<<
#else

//RWP01c
//--------*--------
// MessageBeep (MB_OK);
//--------*--------
   SS_Beep2(hWnd);
//RWP01c

#endif
else
   {
   lstrcpy(lpszString, szStringTemp);
	// RFW - 6/16/04 - 14547 - I move this line above SetCaretPosition
   lpEditField->nChars = EditInfo.nChars;
   SetCaretPosition(hWnd, EditInfo.iCurrentPos);
   }

UnlockField(hWnd);

return ((LONG)Ok);
}


BOOL MatchMonthAbbrev (LPTSTR lpszAbbrev, BOOL FAR *bUnique)

{
   int   nChar;
   int   iMonth;
   int   iMatch;
   int   nMatches;

   nChar = StrLen (lpszAbbrev);
//   #ifdef WIN32
//   lpszAbbrev = CharUpper(lpszAbbrev);
//   #else
//   lpszAbbrev = AnsiUpper(lpszAbbrev);
//   #endif

   for (iMonth = 0, nMatches = 0; iMonth < 12; ++iMonth)
      if (StrnCmpi (lpszAbbrev, CalMonth[iMonth].szName, nChar) == 0)
      {
         if (++nMatches == 1)
            iMatch = iMonth;
         else
            break;
      };

   switch (nMatches)
   {
      case 0:     // Not Found.
         break;

      case 1:     // Found and unique.
         *bUnique = TRUE;
         StrnnCpy (lpszAbbrev, CalMonth[iMatch].szName, 4);
         break;

      default:    // Found but not unique.
         *bUnique = FALSE;
         StrnnCpy (lpszAbbrev, CalMonth[iMatch].szName, 4);
         break;
      }

   return nMatches > 0;
}


BOOL DateValidateDate
(
HWND           hWnd,
LPTSTR         lpszDate,
short          iPos,
BOOL           bCentury,
TUCHAR         cSeparator,
int            nFormat,
short          First,
short          Second,
short          Third,
LPDATEEDITINFO lpEditInfo
)
{
LPTSTR         lpszString;
short          Formats[3];
short          i;
TCHAR          ch;
BOOL           AdvanceCaret;
BOOL           Ok = TRUE;
int            NengoLen;

Formats[0] = First;
Formats[1] = Second;
Formats[2] = Third;

lpszString = lpszDate;

ch = lpszString[iPos];

for (i = 0; Ok && i < 3; i++)
   {
   if (i == 2)
      AdvanceCaret = FALSE;
   else
      AdvanceCaret = TRUE;

   switch (Formats[i])
      {
      case DATE_DD:
         if (iPos >= lpszString - lpszDate && iPos < lpszString - lpszDate + 2)
            Ok = DateValidateDD(hWnd, lpszDate, lpszString,
                                (short)(iPos - (short)(lpszString - lpszDate)), ch,
                                bCentury, cSeparator, nFormat, AdvanceCaret,
                                lpEditInfo);

         lpszString += 2;
         break;

      case DATE_YY:
         if ((bCentury && iPos >= lpszString - lpszDate &&
              iPos < lpszString - lpszDate + 4) ||
             (!bCentury && iPos >= lpszString - lpszDate &&
              iPos < lpszString - lpszDate + 2))
            Ok = DateValidateYY(hWnd, lpszDate, lpszString,
                                (short)(iPos - (short)(lpszString - lpszDate)), ch,
                                bCentury, cSeparator, nFormat, AdvanceCaret,
                                lpEditInfo);

         if (bCentury)
            lpszString += 4;
         else
            lpszString += 2;

         break;

      case DATE_MM:
         if (iPos >= lpszString - lpszDate && iPos < lpszString - lpszDate + 2)
            Ok = DateValidateMM(hWnd, lpszDate, lpszString,
                                (short)(iPos - (short)(lpszString - lpszDate)), ch,
                                bCentury, cSeparator, nFormat, AdvanceCaret,
                                lpEditInfo);

         lpszString += 2;
         break;

      case DATE_MON:
         if (iPos >= lpszString - lpszDate && iPos < lpszString - lpszDate + 3)
            Ok = DateValidateMON(hWnd, lpszDate, lpszString,
                                 (short)(iPos - (short)(lpszString - lpszDate)), ch,
                                 bCentury, cSeparator, nFormat, AdvanceCaret,
                                 lpEditInfo);

         lpszString += 3;
         break;

      //------------------------------------------------------------------------
      // Date format types from here handle Era etc.. (Masanori Iwasa)
      //------------------------------------------------------------------------
      case DATE_NYY:
      case DATE_NNYY:
      case DATE_NNNNYY:
          //- (3)DATE_NYY  (4)DATE_NNYY  (6)DATE_NNNNYY
          //- The numbers in the brackets are calculated with the following algorithim
//#ifdef _UNICODE
//         NengoLen = (Formats[i] / DATE_NNNNYY) + (Formats[i] / 2);
//#else
//         NengoLen = (Formats[i] / DATE_NNNNYY) + Formats[i] - 2;
//#endif
#ifdef _UNICODE
        if (Formats[i] == DATE_NNNNYY) 
          NengoLen = 4;
        else // DATE_NYY || DATE_NNYY
          NengoLen = 3;
#else
        if (Formats[i] == DATE_NYY) NengoLen = 3;
        else if (Formats[i] == DATE_NNYY) NengoLen = 4;
        else if (Formats[i] == DATE_NNNNYY) NengoLen = 6;
#endif

        if (iPos >= lpszString - lpszDate && iPos < lpszString - lpszDate + NengoLen)
            Ok = DateValidateNengo(hWnd, lpszDate, iPos, ch,
                                   bCentury, cSeparator, (short)nFormat,
                                   lpEditInfo, NengoLen);

         lpszString += NengoLen;
         break;

      }
      if (iPos == lpszString - lpszDate) Ok = FALSE;

#ifndef UNICODE
      if (iPos == lpszString - lpszDate + 1 && IsDbcsSeparator(cSeparator)) Ok = FALSE;
#endif
    
      if (Formats[i] != DATE_NULL){
          lpszString++;
#ifndef UNICODE
          if (IsDbcsSeparator(cSeparator)) lpszString++;
#endif
      }
   }

return (Ok);
}


BOOL DateValidateDD
(
HWND           hWnd,
LPTSTR         lpszDate,
LPTSTR         lpszString,
short          iPos,
TCHAR          ch,
BOOL           bCentury,
TUCHAR         cSeparator,
int            nFormat,
BOOL           AdvanceCaret,
LPDATEEDITINFO lpEditInfo
)
{
LPDATEFIELD    lpField;
short          Day;
short          DaysPerMonth;
TCHAR          Buffer[2 + 1];
BOOL           Ok = TRUE;

lpField = LockDateField (hWnd);

switch (iPos)
   {
   case 0:
      if (ch < '0' || ch > '9')
         Ok = FALSE;

      else
         {
         if (ch > '3' ||
				 (ch == '3' && DateGetDaysPerMonth(cSeparator, lpszDate, nFormat,
                                               bCentury, lpField->nTwoDigitYearMax) < 30))
            {
            DatePutChar(hWnd, lpszString, '0', 0, lpEditInfo);
            DatePutChar(hWnd, lpszString, ch, 1, lpEditInfo);

// Sp25_001 [2-1]
// Why Mr Feng Change These Code %( It Will Generate A Bug :( 
// When Date Format Is 'yy/mm/dd', There Will Be A Extra '/' After 'dd'!
// So Comment 2 Place Of Mr Feng's Code For SPRD019 In These Function :)
// -- HaHa 1998.4.21
// SPRD019 Wei Feng 1997/10/15
//#ifdef	SPREAD_JPN
//                 if (nFormat != IDF_MMDD)
//#else
			//Modify BOC 99.6.25 (hyt)--------------------------------
			//for not input dbcsseparator
            //if (AdvanceCaret)
            //     if (!IsDbcsSeparator(cSeparator) && nFormat != IDF_MMDD)
			if((AdvanceCaret && nFormat != IDF_MMDD) || IsDbcsSeparator(cSeparator) )
			//--------------------------------------------------------
//#endif
                     DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 2),
                                cSeparator, szDayGlobal, AdvanceCaret, lpEditInfo);
            }

			/* RFW - 3/10/04 - 13835
         else if (ch == '3')
			*/
         else if (ch == '2' || ch == '3')
            {
            DaysPerMonth = DateGetDaysPerMonth(cSeparator, lpszDate, nFormat,
                                               bCentury, lpField->nTwoDigitYearMax);
            Day = DateGetDay(cSeparator, lpszDate, nFormat, bCentury);

// Sp25_001 [2-2]
// See Detail In Sp25_001 [2-1]
// -- HaHa 1998.4.21
// SPRD019 Wei Feng 1997/10/17
//#ifdef	SPREAD_JPN
//			if (DaysPerMonth < 30)
//			{
//	            DatePutChar(hWnd, lpszString, '0', 0, lpEditInfo);
//		        DatePutChar(hWnd, lpszString, ch, 1, lpEditInfo);
//
//		        DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 2),
//                               cSeparator, szDayGlobal, AdvanceCaret, lpEditInfo);
//			}
//			else if (DaysPerMonth == 30)
//			{
//	            DatePutChar(hWnd, lpszString, ch, 0, lpEditInfo);
//		        DatePutChar(hWnd, lpszString, '0', 1, lpEditInfo);
//
//		        DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 2),
//                               cSeparator, szDayGlobal, AdvanceCaret, lpEditInfo);
//			}
//#else
            if (Day > 0 && DaysPerMonth > 0 && Day > DaysPerMonth)
               {
               wsprintf(Buffer, _T("%02d"), DaysPerMonth);
               DatePutChar(hWnd, lpszString, Buffer[0], 0, lpEditInfo);
               DatePutChar(hWnd, lpszString, Buffer[1], 1, lpEditInfo);
               }
//#endif
            }

         else if (ch == '0' && lpszString[1] == '0')
            DatePutChar(hWnd, lpszString, '1', 1, lpEditInfo);

         }

      break;

   case 1:
      if (ch < '0' || ch > '9')
         Ok = FALSE;
      else
         {
         DaysPerMonth = DateGetDaysPerMonth(cSeparator, lpszDate, nFormat,
                                            bCentury, lpField->nTwoDigitYearMax);
         Day = DateGetDay(cSeparator, lpszDate, nFormat, bCentury);

         if (Day == 0 || (DaysPerMonth > 0 && Day > DaysPerMonth))
            Ok = FALSE;
         else if ((AdvanceCaret && nFormat != IDF_MMDD) || IsDbcsSeparator(cSeparator) )
              DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 2),
                               cSeparator, szDayGlobal, AdvanceCaret, lpEditInfo);
         }

      break;
   }

    //- Check the nengo range if nessecary. (Masanori Iwasa)
    PostMessage(hWnd, UM_CHECKNENGO, 0, 0l);

UnlockDateField(hWnd);

return (Ok);
}


BOOL DateValidateMM
(
HWND    hWnd,
LPTSTR  lpszDate,
LPTSTR  lpszString,
short   iPos,
TCHAR   ch,
BOOL    bCentury,
TUCHAR  cSeparator,
int     nFormat,
BOOL    AdvanceCaret,
LPDATEEDITINFO lpEditInfo
)
{
short Day;
short DaysPerMonth;
BOOL  Ok = TRUE;

switch (iPos)
   {
   case 0:
      if (ch < '0' || ch > '9')
         Ok = FALSE;

      else
         {
         if (ch > '1')
            {
            DatePutChar(hWnd, lpszString, '0', 0, lpEditInfo);
            DatePutChar(hWnd, lpszString, ch, 1, lpEditInfo);

//#ifdef  SPREAD_JPN
			// SPRD019 Wei Feng 1997/9/26
			if (nFormat != IDF_YYMM)
//#else
//           if (!IsDbcsSeparator(cSeparator) && nFormat != IDF_YYMM)
//#endif
                DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 2),
                             cSeparator, szMonthGlobal, TRUE, lpEditInfo);
            }

         else if (ch == '1' && lpszString[1] > '2')
            DatePutChar(hWnd, lpszString, '2', 1, lpEditInfo);

         else if (ch == '0' && lpszString[1] == '0')
            DatePutChar(hWnd, lpszString, '1', 1, lpEditInfo);
         }

      break;

   case 1:
      if (ch < '0' || ch > '9')
         Ok = FALSE;
      else
         {
         if (lpszString[0] == '0' && ch == '0')
            Ok = FALSE;
         else if (lpszString[0] == '1' && ch > '2')
            Ok = FALSE;
         else if ((AdvanceCaret && nFormat != IDF_YYMM) || IsDbcsSeparator(cSeparator))
              DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 2),
                        cSeparator, szMonthGlobal, AdvanceCaret, lpEditInfo);
         }

      break;
   }

if (Ok)
   {
   LPDATEFIELD lpField = LockDateField (hWnd);

   DaysPerMonth = DateGetDaysPerMonth(cSeparator, lpszDate, nFormat, bCentury,
                                      lpField->nTwoDigitYearMax);
   Day = DateGetDay(cSeparator, lpszDate, nFormat, bCentury);

   if (DaysPerMonth > 0 && Day > 0 && Day > DaysPerMonth)
      DateSetDay(hWnd, cSeparator, lpszDate, nFormat, bCentury, DaysPerMonth, lpEditInfo);

   UnlockDateField(hWnd);
   }

    //- Check the nengo range if nessecary. (Masanori Iwasa)
    PostMessage(hWnd, UM_CHECKNENGO, 0, 0l);

return (Ok);
}


BOOL DateValidateMON
(
HWND    hWnd,
LPTSTR  lpszDate,
LPTSTR  lpszString,
short   iPos,
TCHAR   ch,
BOOL    bCentury,
TUCHAR  cSeparator,
int     nFormat,
BOOL    AdvanceCaret,
LPDATEEDITINFO lpEditInfo
)
{
TCHAR szTemp[4];
short Day;
short DaysPerMonth;
BOOL  bUnique;
BOOL  Ok = TRUE;

switch (iPos)
   {
   case 0:
      szTemp[0] = lpszString[0];
      szTemp[1] = 0;
      if (MatchMonthAbbrev(szTemp, &bUnique))
         {
         DatePutChar(hWnd, lpszString, szTemp[0], 0, lpEditInfo);
         DatePutChar(hWnd, lpszString, szTemp[1], 1, lpEditInfo);
         DatePutChar(hWnd, lpszString, szTemp[2], 2, lpEditInfo);

         if (bUnique)
            DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate +
                             3), (TUCHAR)(AdvanceCaret ? cSeparator : 0), szMonthGlobal, TRUE,
                             lpEditInfo);

         // A bit of a fudge for MAR and MAY. It seemed less confusing
         // than trying to make the code general enough to accommodate
         // months where the first character (eg 'M') uniquely
         // determined the second character ('A') but not the third.
         // This is the only such case there ever will be.

         #if defined(_WIN64) || defined(_IA64)
         else if ((TCHAR)CharUpper((LPTSTR)MAKELONG_PTR(szTemp[0], 0)) == 'M' &&
                  (TCHAR)CharUpper((LPTSTR)MAKELONG_PTR(szTemp[1], 0)) == 'A')
         #elif defined(WIN32)
         else if ((TCHAR)CharUpper((LPTSTR)MAKELONG(szTemp[0], 0)) == 'M' &&
                  (TCHAR)CharUpper((LPTSTR)MAKELONG(szTemp[1], 0)) == 'A')
         #else
         else if ((TCHAR)(long)AnsiUpper((LPTSTR)MAKELONG(szTemp[0], 0)) == 'M' &&
                  (TCHAR)(long)AnsiUpper((LPTSTR)MAKELONG(szTemp[1], 0)) == 'A')
         #endif
            lpEditInfo->iCurrentPos = (short)(lpszString - lpszDate) + 1;
         }
      else
         Ok = FALSE;

      break;

   case 1:
      szTemp[0] = lpszString[0];
      szTemp[1] = lpszString[1];
      szTemp[2] = '\0';
      if (MatchMonthAbbrev (szTemp, &bUnique))
         {
         DatePutChar(hWnd, lpszString, szTemp[0], 0, lpEditInfo);
         DatePutChar(hWnd, lpszString, szTemp[1], 1, lpEditInfo);
         DatePutChar(hWnd, lpszString, szTemp[2], 2, lpEditInfo);

         if (bUnique)
            DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate +
                             3), (TUCHAR)(AdvanceCaret ? cSeparator : 0), szMonthGlobal, TRUE,
                             lpEditInfo);
         }
      else
         Ok = FALSE;

      break;

   case 2:
      szTemp[0] = lpszString[0];
      szTemp[1] = lpszString[1];
      szTemp[2] = lpszString[2];
      szTemp[3] = 0;
      if (MatchMonthAbbrev (szTemp, &bUnique))
         DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate +
                          3), (TUCHAR)(AdvanceCaret ? cSeparator : 0), szMonthGlobal, TRUE,
                          lpEditInfo);
      else
         Ok = FALSE;

      break;
   }

if (Ok)
   {
   LPDATEFIELD lpField = LockDateField (hWnd);
   DaysPerMonth = DateGetDaysPerMonth(cSeparator, lpszDate, nFormat, bCentury,
                                      lpField->nTwoDigitYearMax);
   Day = DateGetDay(cSeparator, lpszDate, nFormat, bCentury);

   if (DaysPerMonth > 0 && Day > 0 && Day > DaysPerMonth)
      DateSetDay(hWnd, cSeparator, lpszDate, nFormat, bCentury, DaysPerMonth, lpEditInfo);

   UnlockDateField(hWnd);
   }

return (Ok);
}


BOOL DateValidateYY
(
HWND    hWnd,
LPTSTR  lpszDate,
LPTSTR  lpszString,
short   iPos,
TCHAR   ch,
BOOL    bCentury,
TUCHAR  cSeparator,
int     nFormat,
BOOL    AdvanceCaret,
LPDATEEDITINFO lpEditInfo
)
{
short Day;
short DaysPerMonth;
BOOL  Ok = TRUE;

switch (iPos)
   {
	/* RFW - 1/10/05 - Add validatation for dynamic century
   case 0:
      if (bCentury)
         {
         if (ch < '0' || ch > HIYEAR1)
            Ok = FALSE;
         }
      else
         if (ch < '0' || ch > '9')
            Ok = FALSE;

      break;

   case 1:
      if (ch < '0' || ch > '9')
         Ok = FALSE;
      else
         if (bCentury && lpszString[0] == HIYEAR1 && ch > HIYEAR2)
            Ok = FALSE;

      if (Ok && !bCentury && (AdvanceCaret || IsDbcsSeparator(cSeparator)))
         DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 2),
                          cSeparator, szYearGlobal, AdvanceCaret, lpEditInfo);
      break;
	*/

   case 0:
      if (ch < '0' || ch > '9')
         Ok = FALSE;
      else if (bCentury)
			Ok = DateValidateCentury(hWnd, ch, '\0');

      break;

   case 1:
      if (ch < '0' || ch > '9')
         Ok = FALSE;
      else if (bCentury)
			Ok = DateValidateCentury(hWnd, lpszString[0], ch);

      if (Ok && !bCentury && (AdvanceCaret || IsDbcsSeparator(cSeparator)))
         DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 2),
                          cSeparator, szYearGlobal, AdvanceCaret, lpEditInfo);
      break;

   case 2:
      if (ch < '0' || ch > '9')
         Ok = FALSE;

      break;

   case 3:
      if (ch < '0' || ch > '9')
         Ok = FALSE;
      else
         if (lpszString[0] == '0' && lpszString[1] == '0' &&
             lpszString[2] == '0' && ch == '0')    // Year 0000 not allowed
            Ok = FALSE;

      if (Ok && (AdvanceCaret || IsDbcsSeparator(cSeparator)))
         DateAdvanceCaret(hWnd, lpszDate, (short)(lpszString - lpszDate + 4),
                          cSeparator, szYearGlobal, AdvanceCaret, lpEditInfo);
      break;
   }

if (Ok)
   {
   LPDATEFIELD lpField = LockDateField (hWnd);

   DaysPerMonth = DateGetDaysPerMonth(cSeparator, lpszDate, nFormat, bCentury,
                                      lpField->nTwoDigitYearMax);
   Day = DateGetDay(cSeparator, lpszDate, nFormat, bCentury);

   if (DaysPerMonth > 0 && Day > 0 && Day > DaysPerMonth)
      DateSetDay(hWnd, cSeparator, lpszDate, nFormat, bCentury, DaysPerMonth, lpEditInfo);

   UnlockDateField(hWnd);
   }

return (Ok);
}


short DateGetDaysPerMonth(TUCHAR cSeparator, LPCTSTR lpszDate, int nFormat,
                          BOOL bCentury, short nTwoDigitYearMax)
{
short nMonth;
short nYear;
short Index;

if (nYear = DateGetYear(cSeparator, lpszDate, nFormat, bCentury, nTwoDigitYearMax))
   {
   if (nYear == -1)
      Index = 1;
   else
      Index = ISLEAP(nYear);
   }
else
   Index = 0;

if ((nMonth = DateGetMonth(cSeparator, lpszDate, nFormat, bCentury)) == 0)   
   return (31);

return (CalMonth[nMonth - 1].nDays[Index]);
}


short DaysPerMonth(short nMonth, short nYear)
{
short Index;

Index = ISLEAP(nYear);
return (CalMonth[nMonth - 1].nDays[Index]);
}


short DateGetDay
(
TUCHAR  cSeparator,
LPCTSTR lpszDate,
int     nFormat,
BOOL    bCentury
)
{
short   Offset = 0;
TCHAR   Buffer[2 + 1];
//static TCHAR   Buffer[2 + 1];

    switch (nFormat){
        case IDF_DDMONYY:
        case IDF_DDMMYY:
            break;

        case IDF_MMDDYY:
        case IDF_MMDD:
            Offset = 3;
#ifndef UNICODE // RFW - 5/24/05 - 16280
            if (IsDbcsSeparator(cSeparator)) Offset += 1;
#endif
            break;

        case IDF_YYMMDD:
            Offset = 6;
            if (bCentury) Offset += 2;
#ifndef UNICODE // RFW - 5/24/05 - 16280
            if (IsDbcsSeparator(cSeparator)) Offset += 2;
#endif
            break;

        //------------------------------------------------------------------------
        // Date format types from here handle Era etc.. (Masanori Iwasa)
        //------------------------------------------------------------------------
        case IDF_YYMM:
            return (1);

        case IDF_NYYMMDD:
#ifdef UNICODE
            Offset = 7;
#else
            Offset = (IsDbcsSeparator(cSeparator)) ? 9 : 7;
#endif
            break;
        case IDF_NNYYMMDD:
#ifdef UNICODE
            Offset = (IsDbcsSeparator(cSeparator)) ? 8 : 7;
#else
            Offset = (IsDbcsSeparator(cSeparator)) ? 10 : 8;
#endif
            break;
                    
        case IDF_NNNNYYMMDD:
#ifdef UNICODE
            Offset = (IsDbcsSeparator(cSeparator)) ? 9 : 8;
#else
            Offset = (IsDbcsSeparator(cSeparator)) ? 12 : 10;
#endif
            break;
        }
      
if (lstrlen(lpszDate) < Offset + 2)
   return (0);

MemCpy(Buffer, &lpszDate[Offset], 2 * sizeof(TCHAR));
Buffer[2] = '\0';

return (StringToInt(Buffer));
}


BOOL DateSetDay
(
HWND    hWnd,
TUCHAR  cSeparator,
LPTSTR  lpszDate,
int     nFormat,
BOOL    bCentury,
short   nDay,
LPDATEEDITINFO lpEditInfo
)
{
TCHAR Buffer[2 + 1];
short Offset = 0;

    switch(nFormat){
        case IDF_DDMONYY:
        case IDF_DDMMYY:
            break;

        case IDF_MMDDYY:
        case IDF_MMDD:
            Offset = (IsDbcsSeparator(cSeparator)) ? 4 : 3;
            break;

        case IDF_YYMMDD:
            Offset = 6;
            if (bCentury) Offset += 2;
            if (IsDbcsSeparator(cSeparator)) Offset += 2;
            break;

        //------------------------------------------------------------------------
        // Date format types from here handle Era etc.. (Masanori Iwasa)
        //------------------------------------------------------------------------
        case IDF_YYMM:
            return (FALSE);

        case IDF_NYYMMDD:
            Offset = (IsDbcsSeparator(cSeparator)) ? 9 : 7;
            break;
                    
        case IDF_NNYYMMDD:
//GAB 3/2/05 - Bug 15794
#ifdef _UNICODE
            Offset = (IsDbcsSeparator(cSeparator)) ? 9 : 7;
#else
            Offset = (IsDbcsSeparator(cSeparator)) ? 10 : 8;
#endif
            break;
                    
        case IDF_NNNNYYMMDD:
//GAB 3/2/05 - Bug 15794
#ifdef _UNICODE
            Offset = (IsDbcsSeparator(cSeparator)) ?  10 : 8;
#else
            Offset = (IsDbcsSeparator(cSeparator)) ? 12 : 10;
#endif
          break;
        }

if (lstrlen(lpszDate) < Offset + 2)
   return (0);

wsprintf(Buffer, _T("%02d"), nDay);
DatePutChar(hWnd, lpszDate, Buffer[0], Offset, lpEditInfo);
DatePutChar(hWnd, lpszDate, Buffer[1], (short)(Offset + 1), lpEditInfo);
return (TRUE);
}


short DateGetYear(TUCHAR cSeparator, LPCTSTR lpszDate, int nFormat,
                  BOOL bCentury, short nTwoDigitYearMax)
{
short        Year;
short        Len;
short        Offset = 0;
TCHAR        Buffer[4 + 1];
//static TCHAR Buffer[4 + 1];
TB_DATE      Date;

    switch (nFormat){
        case IDF_YYMMDD:
        case IDF_YYMM:
            break;

        case IDF_DDMONYY:
            Offset = (IsDbcsSeparator(cSeparator)) ? 9 : 7;
            break;

        case IDF_DDMMYY:
        case IDF_MMDDYY:
            Offset = (IsDbcsSeparator(cSeparator)) ? 8 : 6;
            break;

        //------------------------------------------------------------------------
        // Date format types from here handle Era etc.. (Masanori Iwasa)
        //------------------------------------------------------------------------
        case IDF_MMDD:
            SysGetDate(&Date.nMonth, &Date.nDay, &Date.nYear);
            return Date.nYear;

        case IDF_NYYMMDD:
        case IDF_NNYYMMDD:
        case IDF_NNNNYYMMDD:
//GAB 3/2/05 - Bug 15794
#ifdef _UNICODE
        if (nFormat == IDF_NNNNYYMMDD) 
          Offset = 4;
        else // DATE_NYY || DATE_NNYY
          Offset = 3;
#else
        if (nFormat == IDF_NYYMMDD) Offset = 3;
        else if (nFormat == IDF_NNYYMMDD) Offset = 4;
        else if (nFormat == IDF_NNNNYYMMDD) Offset = 6;
#endif
//GAB 3/2/05 - Bug 15794
//          Offset = (nFormat / IDF_NNNNYYMMDD) + (nFormat - 103);
            if (lstrlen(lpszDate) < Offset)
                 return (-1);

            return (GetYearFromNengo(lpszDate, Offset));
        }

Len = bCentury ? 4 : 2;

if (lstrlen(lpszDate) < Offset + Len)
   return (-1);

MemCpy(Buffer, &lpszDate[Offset], (WORD)(Len * sizeof(TCHAR)));
Buffer[Len] = '\0';

Year = StringToInt(Buffer);

if (Len == 2)
   {
   short nCentury = (short)(nTwoDigitYearMax / 100 * 100);
   short nTwoDigit = nTwoDigitYearMax - nCentury;

   if (Year <= nTwoDigit)
      Year += nCentury;
   else
      Year += nCentury - 100;

/* RFW - 8/26/98
   if (Year >= 60)
      Year += 1900;
   else
      Year += 2000;
*/
   }

return (Year);
}


short DateGetMonth
(
TUCHAR       cSeparator,
LPCTSTR      lpszDate,
int          nFormat,
BOOL         bCentury
)
{
short        Offset = 0;
short        iMonth;
TCHAR        Buffer[3 + 1];
//static TCHAR Buffer[3 + 1];

    switch (nFormat){
        case IDF_MMDDYY:
        case IDF_MMDD:
            break;

        case IDF_YYMMDD:
        case IDF_YYMM:
            Offset = (bCentury) ? 5 : 3;
#ifndef _UNICODE // RFW - 2/20/07 - 19768
            if (IsDbcsSeparator(cSeparator)) Offset ++;
#endif
            break;

        case IDF_DDMMYY:
            Offset = (IsDbcsSeparator(cSeparator)) ? 4 : 3;
            break;

        case IDF_DDMONYY:
            Offset = (IsDbcsSeparator(cSeparator)) ? 4 : 3;
            if (lstrlen(lpszDate) < Offset + 3) return (0);

				// RFW - 7/26/04 - 14940
            //MemCpy(Buffer, &lpszDate[Offset], 3);
            lstrcpyn(Buffer, &lpszDate[Offset], 4);
            Buffer[3] = '\0';

            for (iMonth = 0; iMonth < 12; iMonth++)
                if (StrCmp(Buffer, CalMonth[iMonth].szName) == 0)
                    return (iMonth + 1);
            return (0);

        //------------------------------------------------------------------------
        // Date format types from here handle Era etc.. (Masanori Iwasa)
        //------------------------------------------------------------------------
        case IDF_NYYMMDD:
            Offset = (IsDbcsSeparator(cSeparator)) ? 5 : 4;
            break;
                    
        case IDF_NNYYMMDD:
//GAB 3/2/05 - Bug 15794
#ifdef _UNICODE
            Offset = (IsDbcsSeparator(cSeparator)) ? 5 : 4;
#else 
            Offset = (IsDbcsSeparator(cSeparator)) ? 6 : 5;
#endif
            break;
                    
        case IDF_NNNNYYMMDD:
//GAB 3/2/05 - Bug 15794
#ifdef _UNICODE
           Offset = (IsDbcsSeparator(cSeparator)) ? 6 : 5;
#else
           Offset = (IsDbcsSeparator(cSeparator)) ? 8 : 7;
#endif
            break;
        }

if (lstrlen(lpszDate) < Offset + 2)
   return (0);

MemCpy(Buffer, &lpszDate[Offset], 2 * sizeof(TCHAR));
Buffer[2] = '\0';

return (StringToInt(Buffer));
}


void DatePutChar(hWnd, lpszString, ch, iPos, lpEditInfo)

HWND           hWnd;
LPTSTR         lpszString;
TCHAR          ch;
short          iPos;
LPDATEEDITINFO lpEditInfo;
{
if (iPos < lstrlen(lpszString))
   lpszString[iPos] = ch;
else
   {
   lpszString[iPos] = ch;
   lpszString[iPos + 1] = '\0';

   lpEditInfo->nChars++;
   }
}


void DateAdvanceCaret
(
HWND           hWnd,
LPTSTR         lpszDate,
short          iPos,
TUCHAR         cSeparator,
LPTSTR         lpKanji,
BOOL           bAdvanceCaret,
LPDATEEDITINFO lpEditInfo
)
{
//- This was modified so that Japanese separator can be supported (Masanori Iwasa)
if (cSeparator){
    if (!IsDbcsSeparator(cSeparator))
        DatePutChar(hWnd, lpszDate, cSeparator, iPos, lpEditInfo);
    else{
        DatePutStr(hWnd, lpszDate, lpKanji, iPos, lpEditInfo);
        iPos += lstrlen(lpKanji) - 1;
        }
    }               

lpEditInfo->iCurrentPos = iPos;
SetCaretPosition(hWnd, iPos);
}


//--------------------------------------------------------------------------------
// Sets a DBCS separator into the string. (Masanori Iwasa)
//--------------------------------------------------------------------------------
void DatePutStr(HWND hWnd, LPTSTR lpszString, LPTSTR lpKanji, short iPos, LPDATEEDITINFO lpEditInfo)
{
    if (iPos >= lstrlen(lpszString) - 1){
#ifndef UNICODE // RFW - 5/24/05 - 16280
        lpszString[iPos + 2] = 0;
#else
        lpszString[iPos + 1] = 0;
#endif
        lpEditInfo->nChars++;
        if (iPos == lstrlen(lpszString))
            lpEditInfo->nChars++;
        }
    lpszString[iPos] = lpKanji[0];
#ifndef UNICODE
    lpszString[iPos + 1] = lpKanji[1];
#endif
}


//--------------------------------------------------------------------------------
// Check if the inputted character matches with the Era name. (Masanori Iwasa) 
//--------------------------------------------------------------------------------
BOOL DateValidateNengo
(
HWND           hWnd,
LPTSTR         lpszDate,
short          iPos,
TCHAR          ch,
BOOL           bCentury,
TUCHAR         cSeparator,
short          nFormat,
LPDATEEDITINFO lpEditInfo,
int            NengoLen
)
{
    short Day;
    short DaysPerMonth;
    BOOL  Ok = TRUE;
    int   NengoType;

    NengoType = NengoLen - 3;
    
    if (iPos < NengoLen - 2){
        switch(ch){
            case 'M':
            case 'm':
                SetNengoInit(hWnd, lpszDate, NengoType, 0, lpEditInfo, FALSE);
                return (TRUE);
                 
            case 'T':
            case 't':
                SetNengoInit(hWnd, lpszDate, NengoType, 1, lpEditInfo, FALSE);
                return (TRUE);

            case 'S':
            case 's':
                SetNengoInit(hWnd, lpszDate, NengoType, 2, lpEditInfo, FALSE);
                return (TRUE);

            case 'H':
            case 'h':
                SetNengoInit(hWnd, lpszDate, NengoType, 3, lpEditInfo, FALSE);
                return (TRUE);

            default:
                if (lstrlen(lpszDate) == 1){
                    SetNengoInit(hWnd, lpszDate, NengoType, 3, lpEditInfo, TRUE);
                    return (TRUE);
                    }
                else
                    return (FALSE);
            }
        }
    else{
        switch (iPos - NengoLen + 2){

            case 0:
                if (ch < '0' || ch > '9') Ok = FALSE;
                break;

            case 1:
                if (ch < '0' || ch > '9') Ok = FALSE;
                if (Ok)
                    DateAdvanceCaret(hWnd, lpszDate, (short)NengoLen,
                          cSeparator, szYearGlobal, TRUE, lpEditInfo);
                break;
            }
        }

    if (Ok)
        {
        LPDATEFIELD lpField = LockDateField (hWnd);

        DaysPerMonth = DateGetDaysPerMonth(cSeparator, lpszDate, nFormat, bCentury,
                                           lpField->nTwoDigitYearMax);
        Day = DateGetDay(cSeparator, lpszDate, nFormat, bCentury);

        if (DaysPerMonth > 0 && Day > 0 && Day > DaysPerMonth)
           DateSetDay(hWnd, cSeparator, lpszDate, nFormat, bCentury, DaysPerMonth, lpEditInfo);

        UnlockDateField(hWnd);
        }

    PostMessage(hWnd, UM_CHECKNENGO, 0, 0l);

    return (Ok);
}


BOOL DateValidateCentury(HWND hWnd, TCHAR cDigit1, TCHAR cDigit2)
{
LPDATEFIELD lpField = LockDateField (hWnd);
TB_DATE     Date;
TCHAR       szBuffer[2 + 1];
int         iCentury, iRange;
BOOL        fOneDigit = (cDigit2 == 0);
BOOL        fRet = TRUE;

szBuffer[0] = cDigit1;
szBuffer[1] = cDigit2;
szBuffer[2] = 0;

iCentury = StringToInt(szBuffer);

if (lpField->lRangeMin != UNDEFINED)
	{
   DateJulianToDMY(lpField->lRangeMin, &Date);
	iRange = (fOneDigit ? (Date.nYear / 1000) : (Date.nYear / 100));
	fRet = (BOOL)(iCentury >= iRange);
	}
else
	fRet = TRUE;

if (fRet)
	{
	 if (lpField->lRangeMax != UNDEFINED)
		{
		DateJulianToDMY(lpField->lRangeMax, &Date);
		iRange = (fOneDigit ? (Date.nYear / 1000) : (Date.nYear / 100));
		}
	else
		iRange = (fOneDigit ? (HIYEAR / 1000) : (HIYEAR / 100));

	fRet = (BOOL)(iCentury <= iRange);
	}

UnlockDateField(hWnd);
return (fRet);
}