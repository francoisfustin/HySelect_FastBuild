/*
$Revision:   1.3  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITDATE/USERDATE.C_V  $
 * 
 *    Rev 1.3   12 Jun 1990 14:09:22   Dirk
 * No change.
 * 
 *    Rev 1.2   07 Jun 1990 10:09:46   Steve
 * fixed YYMMDD year value for DateStringToDMY() function
 * 
 *    Rev 1.1   06 Jun 1990 10:36:18   Steve
 * break statement missing from DDMMYY block in DateStringToDMY() function.
 * 
 *    Rev 1.0   04 Jun 1990 15:14:50   Bruce
 * Initial revision.
 * 
 *    Rev 1.0   04 Jun 1990 14:54:38   Bruce
 * Initial revision.
 * 
 *    Rev 1.16   28 May 1990 15:25:52   Randall
 * Moved DateStringIsValid and DateIntIsValid in to Static.c to reduce size of o
 * 
 *    Rev 1.15   24 May 1990 11:20:56   Steve
 * DateDMYToString years changed to use modulo, not subtraction
 * 
 *    Rev 1.14   24 May 1990 11:02:32   Randall
 * Added DateSet(Get)Format
 * 
 *    Rev 1.13   23 May 1990 16:48:28   Sonny
 * No change.
 * 
 *    Rev 1.12   23 May 1990  9:19:54   Randall
 * No change.
 * 
 *    Rev 1.11   22 May 1990 17:23:36   Steve
 * changed Date functions to use LPDATE structure rather than d, m, y integers
 * 
 *    Rev 1.10   22 May 1990 14:45:16   Steve
 * removed addition of THISCENTURY to year value if bCentury set.
 * 
 *    Rev 1.9   21 May 1990 16:22:02   Randall
 * Added DateSetRange
 * 
 *    Rev 1.8   21 May 1990 13:26:44   Randall
 * 
 *    Rev 1.7   17 May 1990  9:56:00   Steve
 * Changed DateDMYToString() to print years properly
 * 
 *    Rev 1.6   16 May 1990 15:42:10   Steve
 * using define for string of THISCENTURY
 * 
 *    Rev 1.5   14 May 1990 17:44:46   Randall
*/

///////////////////////////////////////////////////////////////////////////////
/// DATEUTIL.C -- new version of Date Functions with Randall's spec changes ///
/// most of this done originally by Jeff Smartt                             ///
/// other functions - Randall Smallwood and a few by       Steve M. 23/4/90 ///
///////////////////////////////////////////////////////////////////////////////

#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <tchar.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "editdate.h"
#include "calmonth.h"
#include "..\editfld.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editdate\editdate.h"
//borland #include "editdate\calmonth.h"
//borland #include "editfld.h"
<-BORLAND*/
#endif

short DateJulianStart = LOYEAR;

void DateGetDefFormat(LPDATEFORMAT lpDateFormat);

//- CAUTION ------------------------------------------------------------------------+
// This section sets up Japanese specific strings and date value so that the era    |
// support can be implemented. (Masanori Iwasa)                                     |
//----------------------------------------------------------------------------------+

//-                    TAISHO     MEIJI     SHOWA    HEISEI    CURRENT
//-                  ----------+---------+---------+---------+----------
LONG  NengoLimit[] = { 682158,   698188,   703449,   726109,   999999};             
int   NengoYear[]  = {   1868,     1912,     1926,     1989,     9999};
int   NengoMonth[] = {      9,        7,       12,        1          };
int   NengoDay[]   = {      8,       30,       25,        8          };
TCHAR NengoName[12][8];

//- Global variables for DBCS separators (Masanori Iwasa)
extern TCHAR     szYearGlobal[8];
extern TCHAR     szMonthGlobal[8];
extern TCHAR     szDayGlobal[8];


//////////////////////////////////////////////////////////////////////////////
// DateStringToDMY  - Convert a Drover ToolBox Date string (from a formatted
// edit field) using the info in the DATEFORMAT structure into the integers
// d, m and y. This string is guaranteed to be a valid date.
// The DATEFORMAT cSeparator character is skipped over entirely.     spm
//////////////////////////////////////////////////////////////////////////////
#ifdef SPREAD_JPN
LPTB_DATE DLLENTRY DateStringToDMY(LPCTSTR lpszDateString, LPTB_DATE da,
                                   LPDATEFORMAT datefmt)
{
//return (DateStringToDMYEx(lpszDateString, da, datefmt, nTwoDigitYearMax,
//                          TWODIGITYEARMAX_DEF));
	return (DateStringToDMYEx(lpszDateString, da, datefmt, 
                          TWODIGITYEARMAX_DEF));

}
#endif


LPTB_DATE DLLENTRY DateStringToDMYEx(LPCTSTR lpszDateString, LPTB_DATE da,
                                     LPDATEFORMAT datefmt, short nTwoDigitYearMax)
{
   TCHAR szTmpStr[ DATELENGTH];     // temp string buffer for conversion
   int   iMonthPos;                 // loop index for month -> integer

   int   iStartPos = 0;             // Used to adjust pointer in date string
                                    // This was needed because of DBCS separator
   BOOL  bChangeYear = TRUE;        // Used whether the year should change or not.
                                    // MMDD, Era Format will be FALSE
                                    // (Masanori Iwasa)

   MemSet(da, '\0', sizeof(TB_DATE));

   //- CAUTION ------------------------------------------------------------+
   // In order to support Japanese separators, we had to use a variable    |
   // as a pointer instead of accessing the string address directly.       |
   // The iStartPos variable is used as the pointer. (Masanori Iwasa)      |
   //----------------------------------------------------------------------+

   // Select processing according to DATEFORMAT's nFormat specifier:
   switch (datefmt->nFormat)
   {
    case IDF_DDMONYY:  // Eg.  23-MAR-90 style
         // Convert "DD-......" part:
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        da->nDay = StringToInt( szTmpStr);
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
        // Convert "...MON-..." part:
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = *(lpszDateString + iStartPos++);
        szTmpStr[ 3] = 0;
        // Find "MON" string in global CalMonth array (it WILL be there):
        for ( iMonthPos = 0; iMonthPos < 12; iMonthPos++)
            if ( !StriCmp( szTmpStr, CalMonth[ iMonthPos].szName))
               da->nMonth = iMonthPos + 1;
        // Convert ".......YY" part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        if (datefmt->bCentury){
            szTmpStr[ 2] = *(lpszDateString + iStartPos++);
            szTmpStr[ 3] = *(lpszDateString + iStartPos++);
            szTmpStr[ 4] = 0;
            }
        da->nYear = StringToInt( szTmpStr);
        break;

    case IDF_DDMMYY:   // Eg. 23-03-90 style
         // Convert "DD-......" part:
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        da->nDay = StringToInt( szTmpStr);
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
        // Convert "...MM-..." part:
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        da->nMonth = StringToInt( szTmpStr);
        // Convert ".......YY" part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        if (datefmt->bCentury){
            szTmpStr[ 2] = *(lpszDateString + iStartPos++);
            szTmpStr[ 3] = *(lpszDateString + iStartPos++);
            szTmpStr[ 4] = 0;
            }
        da->nYear = StringToInt( szTmpStr);
        break;

    case IDF_MMDDYY:   // Eg. 03-23-90 style
         // Convert "MM-......" part:
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        da->nMonth = StringToInt( szTmpStr);
        // Convert "...DD-..." part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        da->nDay = StringToInt( szTmpStr);
        // Convert ".......YY" part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        if (datefmt->bCentury){
            szTmpStr[ 2] = *(lpszDateString + iStartPos++);
            szTmpStr[ 3] = *(lpszDateString + iStartPos++);
            szTmpStr[ 4] = 0;
            }
        da->nYear = StringToInt( szTmpStr);
        break;

    case IDF_YYMMDD:   // Eg. 90-03-23 style
        // Convert ".......YY" part:
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        if (datefmt->bCentury){
            szTmpStr[ 2] = *(lpszDateString + iStartPos++);
            szTmpStr[ 3] = *(lpszDateString + iStartPos++);
            szTmpStr[ 4] = 0;
            }
        da->nYear = StringToInt( szTmpStr);
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        da->nMonth = StringToInt( szTmpStr);
        // Convert "...DD-..." part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        da->nDay = StringToInt( szTmpStr);
        break;

    //- CAUTION : Current version doesn't support this !!!!
    //case IDF_MONDDYY:
    //    break;

    //---------------------------------------------------------------------------
    // Date input formats that support Japanese Era (Masanori Iwasa)
    //---------------------------------------------------------------------------
    case IDF_YYMM:    // Eg. 90-03 style
         // Convert "YY-..." part:
        szTmpStr[ 0] = *(lpszDateString + iStartPos++);
        szTmpStr[ 1] = *(lpszDateString + iStartPos++);
        szTmpStr[ 2] = 0;
        if (datefmt->bCentury){
            szTmpStr[ 2] = *(lpszDateString + iStartPos++);
            szTmpStr[ 3] = *(lpszDateString + iStartPos++);
            szTmpStr[ 4] = 0;
            }
         da->nYear = StringToInt( szTmpStr);
         // Convert "...MM" part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
         szTmpStr[ 0] = *(lpszDateString + iStartPos++);
         szTmpStr[ 1] = *(lpszDateString + iStartPos);
         szTmpStr[ 2] = 0;
         da->nMonth = StringToInt( szTmpStr);
         da->nDay = 1;
         break;

    case IDF_MMDD:   // Eg. 03-23 style
         SysGetDate(&da->nMonth, &da->nDay, &da->nYear);
         // Convert "MM-..." part:
         szTmpStr[ 0] = *(lpszDateString + iStartPos++);
         szTmpStr[ 1] = *(lpszDateString + iStartPos++);
         szTmpStr[ 2] = 0;
         da->nMonth = StringToInt( szTmpStr);
         // Convert "....DD" part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
         szTmpStr[ 0] = *(lpszDateString + iStartPos++);
         szTmpStr[ 1] = *(lpszDateString + iStartPos);
         szTmpStr[ 2] = 0;
         da->nDay = StringToInt( szTmpStr);
         bChangeYear = FALSE;
         break;

    case IDF_NYYMMDD:       // Eg. H06봏01뙉06볷 style
    case IDF_NNYYMMDD:      // Eg. 빟06봏01뙉06볷 style
    case IDF_NNNNYYMMDD:    // Eg. 빟맟06봏01뙉06볷 style
         //- (3)IDF_NYYMMDD  (4)IDF_NNYYMMDD  (6)IDF_NNNNYYMMDD
         //- The numbers in the brackets are calculated with the following algorithim
         if (datefmt->nFormat == IDF_NYYMMDD)
             iStartPos = 3;
         else if (datefmt->nFormat == IDF_NNYYMMDD)
         {
#ifdef _UNICODE
             iStartPos = 3;
#else
             iStartPos = 4;
#endif
         }
         else if (datefmt->nFormat == IDF_NNNNYYMMDD)
         {
#ifdef _UNICODE
             iStartPos = 4;
#else
             iStartPos = 6;
#endif
         }
         // Convert "YY-......" part:
         da->nYear = GetYearFromNengo((LPTSTR)lpszDateString, iStartPos);
         // Convert "...MM-..." part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
         szTmpStr[ 0] = *(lpszDateString + iStartPos++);
         szTmpStr[ 1] = *(lpszDateString + iStartPos++);
         szTmpStr[ 2] = 0;
         da->nMonth = StringToInt( szTmpStr);
         // Convert "......DD" part:
#ifdef _UNICODE
         iStartPos ++;
#else
         iStartPos += (IsDbcsSeparator(datefmt->cSeparator)) ? 2 : 1;
#endif
         szTmpStr[ 0] = *(lpszDateString + iStartPos++);
         szTmpStr[ 1] = *(lpszDateString + iStartPos);
         szTmpStr[ 2] = 0;
         da->nDay = StringToInt( szTmpStr);
         bChangeYear = FALSE;                   //- FALSE because own calc for era.
         break;
   }

   //- Japanese Era has it's own calc algorithem, so it sets
   //  bChangedYear to FALSE and jumps over. (Masanori Iwasa)
   //  The original line doesn't check for bChangeYear.
   if ( !datefmt->bCentury && bChangeYear )
      {
      short nCentury = (short)(nTwoDigitYearMax / 100 * 100);
      short nTwoDigit = nTwoDigitYearMax - nCentury;

      if (da->nYear <= nTwoDigit)
         da->nYear += nCentury;
      else
         da->nYear += nCentury - 100;

/* RFW - 8/26/98
      if (da->nYear < 60)
         da->nYear += THISCENTURY + 100;
      else
         da->nYear += THISCENTURY;
*/
      }

   return da;
}  //// End of DateStringToDMY function /////


///////////////////////////////////////////////////////////////////////////////
// DateDMYToString  - Convert integers d,m,y to a string according to the 
// DATEFORMAT info contents.                                            spm
// modified to use DATE structure for Day, Month, Year 22/5/90
///////////////////////////////////////////////////////////////////////////////
LPTSTR DLLENTRY DateDMYToString(LPTB_DATE da, LPTSTR lpszDateOutString,
                                LPDATEFORMAT datefmt)
{
   TCHAR szInternal[ DATELENGTH];      // For building the string internally
   TCHAR szTmpStr[ DATELENGTH];        // temporary string buffer
   int   iTmpYear;                     // Temp var for subtracting THISCENTURY

   switch (datefmt->nFormat)
   {
      case IDF_DDMONYY:     // Eg. 23,3,90 ===> "23-MAR-90"
         // Convert nDay to string and add separator:
         wsprintf(szInternal, _T("%02d"), da->nDay);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = '\0';
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCpy(szTmpStr, szDayGlobal);
         StrCat( szInternal, szTmpStr);
         // Look up nMonth string in CalMonth[] and append to Internal string:
         StrCat( szInternal, CalMonth[ da->nMonth - 1].szName);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = '\0';
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCpy(szTmpStr, szMonthGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert iYear to string:
         iTmpYear = da->nYear;
         if ( !datefmt->bCentury)
            iTmpYear %= 100;
         wsprintf(szTmpStr, _T("%02d"), iTmpYear);
         StrCat( szInternal, szTmpStr);
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCat(szInternal, szYearGlobal);
         break;

      case IDF_DDMMYY:      // Eg. 23,3,90 ===> "23-03-90"
         // Convert nDay to string and add separator:
         // Add leading zero, if needed:
         wsprintf(szInternal, _T("%02d"), da->nDay);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = '\0';
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCpy(szTmpStr, szDayGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert iMonth to string and add separator:
         // Add leading zero, if needed:
         wsprintf(szTmpStr, _T("%02d"), da->nMonth);
         StrCat( szInternal, szTmpStr);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = '\0';
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCpy(szTmpStr, szMonthGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert iYear to string:
         iTmpYear = da->nYear;
         if ( !datefmt->bCentury)
            iTmpYear %= 100;
         wsprintf(szTmpStr, _T("%02d"), iTmpYear);
         StrCat( szInternal, szTmpStr);
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCat(szInternal, szYearGlobal);
         break;
 
      case IDF_MMDDYY:      // Eg. 3,23,90 ===> "03-23-90"
         // Convert nMonth to string and add separator:
         wsprintf(szInternal, _T("%02d"), da->nMonth);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = '\0';
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCpy(szTmpStr, szMonthGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert nDay to string and add separator:
         // Add leading zero, if needed:
         wsprintf(szTmpStr, _T("%02d"), da->nDay);
         StrCat( szInternal, szTmpStr);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = '\0';
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCpy(szTmpStr, szDayGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert nYear to string:
         iTmpYear = da->nYear;
         if ( !datefmt->bCentury)
            iTmpYear %= 100;
         wsprintf(szTmpStr, _T("%02d"), iTmpYear);
         StrCat( szInternal, szTmpStr);
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCat(szInternal, szYearGlobal);
         break;
 
      case IDF_YYMMDD:      // Eg. 90,23,3 ===> "90-23-03"
         // Convert nYear to string and add separator:
         iTmpYear = da->nYear;
         if ( !datefmt->bCentury)
            iTmpYear %= 100;
         wsprintf(szInternal, _T("%02d"), iTmpYear);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = '\0';
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCpy(szTmpStr, szYearGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert nMonth to string and add separator:
         // Add leading zero, if needed:
         wsprintf(szTmpStr, _T("%02d"), da->nMonth);
         StrCat( szInternal, szTmpStr);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = '\0';
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCpy(szTmpStr, szMonthGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert nDay to string and append to internal string:
         // Add leading zero, if needed:
         wsprintf(szTmpStr, _T("%02d"), da->nDay);
         StrCat( szInternal, szTmpStr);
         if(IsDbcsSeparator(datefmt->cSeparator))
             StrCat(szInternal, szDayGlobal);
         break;

      //- CAUTION : Current version doesn't support this !!!!
      //case IDF_MONDDYY:
      //   break;

      //---------------------------------------------------------------------------
      // Date input formats that support Japanese Era (Masanori Iwasa)
      //---------------------------------------------------------------------------
      case IDF_YYMM:        // Eg. 90,3 ===> "90-03"
         // Convert nYear to string and add separator:
         iTmpYear = da->nYear;
         if ( !datefmt->bCentury)
            iTmpYear %= 100;
         wsprintf(szInternal, _T("%02d"), iTmpYear);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = 0;
         if(IsDbcsSeparator(datefmt->cSeparator))
            StrCpy(szTmpStr, szYearGlobal );
         StrCat( szInternal, szTmpStr);
         // Convert nMonth to string and add separator:
         // Add leading zero, if needed:
         wsprintf(szTmpStr, _T("%02d"), da->nMonth);
         StrCat( szInternal, szTmpStr);
         // Append Separator:
         if(IsDbcsSeparator(datefmt->cSeparator))
            StrCat( szInternal, szMonthGlobal);
         break;

      case IDF_MMDD:        // Eg. 23,3 ===> "23-03"
         // Convert nMonth to string and add separator:
         // Add leading zero, if needed:
         wsprintf(szInternal, _T("%02d"), da->nMonth);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = 0;
         if(IsDbcsSeparator(datefmt->cSeparator))
            StrCpy(szTmpStr, szMonthGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert nDay to string and append to internal string:
         // Add leading zero, if needed:
         wsprintf(szTmpStr, _T("%02d"), da->nDay);
         StrCat( szInternal, szTmpStr);
         if(IsDbcsSeparator(datefmt->cSeparator))
            StrCat( szInternal, szDayGlobal);
         break;

      case IDF_NYYMMDD:     // Eg. 90,23,3 ===> "H02-23-03"
      case IDF_NNYYMMDD:    // Eg. 90,23,3 ===> "빟02-23-03"
      case IDF_NNNNYYMMDD:  // Eg. 90,23,3 ===> "빟맟02-23-03"
         // Convert nYear to string and add separator:
         GetNengoFromDate(da, szTmpStr, datefmt->nFormat - IDF_NYYMMDD);
         StrCpy(szInternal, szTmpStr);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = 0;

         if(IsDbcsSeparator(datefmt->cSeparator))
           StrCpy(szTmpStr, szYearGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert nMonth to string and add separator:
         // Add leading zero, if needed:
         wsprintf(szTmpStr, _T("%02d"), da->nMonth);
         StrCat( szInternal, szTmpStr);
         // Append Separator:
         szTmpStr[ 0] = datefmt->cSeparator;
         szTmpStr[ 1] = 0;
         if(IsDbcsSeparator(datefmt->cSeparator))
            StrCpy(szTmpStr, szMonthGlobal);
         StrCat( szInternal, szTmpStr);
         // Convert nDay to string and append to internal string:
         // Add leading zero, if needed:
         wsprintf(szTmpStr, _T("%02d"), da->nDay);
         StrCat( szInternal, szTmpStr);
         if(IsDbcsSeparator(datefmt->cSeparator))
            StrCat( szInternal, szDayGlobal);
         break;
   }
   // Ok, internal string built, copy it out and return a pointer to it:
   StrCpy( lpszDateOutString, szInternal);
   return lpszDateOutString;
}  //// End of function DateDMYToString ////


///////////////////////////////////////////////////////////////////////////////
// DateAddDays  - Add lDays to the base date and place new value in NewDate spm
///////////////////////////////////////////////////////////////////////////////
LPTB_DATE DLLENTRY DateAddDays( LPTB_DATE da, LONG lDays)
{
   long  lOldJulianDate;              // Temporary variable
   long  lNewJulianDate;              // Temporary variable

   //// Convert the base date to Julian, add days, convert back:
   lOldJulianDate = DateDMYToJulian( da);
   lNewJulianDate = lOldJulianDate + lDays;
   DateJulianToDMY( lNewJulianDate, da);
   return da;
}


///////////////////////////////////////////////////////////////////////////////
// DateSubtractDates  - return number of days difference between 2 dates
// NOTE: difference = First Date - Second Date                           spm
///////////////////////////////////////////////////////////////////////////////
long DLLENTRY DateSubtractDates( LPTB_DATE daFirst, LPTB_DATE daSecond)
{
   long  lFirstJulianDate;                // Tempory variable
   long  lSecondJulianDate;               // Temporary variable


   lFirstJulianDate = DateDMYToJulian( daFirst);
   lSecondJulianDate = DateDMYToJulian( daSecond);

   //// Return the Difference:
   return ( lFirstJulianDate - lSecondJulianDate);
}  //// End of function DateSubtractDates ////
///////////////////////////////////////////////////////////////////////////////

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
� Function: DateDMYToJulian                                                  �
�                                                                            �
� Desc: Takes a day, month, and year and returns the number of days that     �
�       have elapsed since DateJulianStart                                   �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
long DLLENTRY DateDMYToJulian( LPTB_DATE da)
{
   int   i;
   long  julian = 0L;
   
   for ( i = DateJulianStart ; i < da->nYear ; i++)
   {
      julian += 365;
      if ( ISLEAP( i))
         julian++;
   }
   for ( i = 0 ; i < ( da->nMonth - 1) ; i++)
      julian += CalMonth[ i].nDays[ ISLEAP( da->nYear)];
   julian += ( da->nDay - 1);
   return julian;
}

//--------------------------------------------------------------------------//
 
BOOL DLLENTRY DateSetRange( HWND hWnd, LPTB_DATE daMin, LPTB_DATE daMax)
{
   LONG  lMin, lMax;       // Julian Dates
   TCHAR szString[60];

   daMin->nYear = max(daMin->nYear, LOYEAR);
   daMax->nYear = min(daMax->nYear, HIYEAR);

   lMin = DateDMYToJulian ( daMin);
   lMax = DateDMYToJulian ( daMax);

   if (lMin > lMax) return FALSE;
#if 0
   GetClassName (hWnd, szString, 40);
   if (StrCmp (szString, DATE_CLASS_NAME) != 0) return FALSE;
#endif
   StrPrintf (szString, _T("%ld %ld"), lMin, lMax);

   return
      (BOOL) SendMessage (hWnd, EM_SETRANGE, 0, (LPARAM)(LPTSTR) szString);
}

//--------------------------------------------------------------------------//

BOOL DLLENTRY DateSetFormat (HWND hWnd, LPDATEFORMAT df)

{
   LPARAM  lParam;

   lParam = (LPARAM) df;

   SendMessage (hWnd, EM_SETFORMAT, 0, lParam);

   return TRUE;
}

//--------------------------------------------------------------------------//

BOOL DLLENTRY DateGetFormat (HWND hWnd, LPDATEFORMAT df)

{
if (hWnd)
   SendMessage(hWnd, EM_GETFORMAT, 0, (LPARAM)df);
else
   DateGetDefFormat(df);

return (TRUE);
}


void DLLENTRY DateGetCurrentDate(LPTSTR lpszDate, LPDATEFORMAT lpdf)
{
TB_DATE Date;

SysGetDate (&Date.nDay, &Date.nMonth, &Date.nYear);
DateDMYToString (&Date, lpszDate, lpdf);
}


void DateGetDefFormat(lpDateFormat)

LPDATEFORMAT lpDateFormat;
{
TCHAR        szTemp[20];
LPTSTR       Ptr;

switch ((BOOL)GetProfileInt(_T("intl"), _T("iDate"), -1))
   {
   case -1:
   case 0:
      lpDateFormat->nFormat = IDF_MMDDYY;
      break;

   case 1:
      lpDateFormat->nFormat = IDF_DDMMYY;
      break;

   case 2:
      lpDateFormat->nFormat = IDF_YYMMDD;
      break;
   }

GetProfileString (_T("intl"), _T("sDate"), _T("/"), szTemp, 2);
if (szTemp[0])
   lpDateFormat->cSeparator = szTemp[0];
else                                   // Windows seems to return NULL
   lpDateFormat->cSeparator = ' ';           //    if you specified a space.

GetProfileString(_T("intl"), _T("sShortDate"), _T("M/d/yy"), szTemp, 11);

lpDateFormat->bCentury = 0;

if ((Ptr = StrChr(szTemp, 'y')) || (Ptr = StrChr(szTemp, 'Y')))
   if (StriStr(Ptr, _T("yyyy")))
      lpDateFormat->bCentury = 1;

lpDateFormat->bSpin = 0;
}


BOOL DLLENTRY DateSetDMY(HWND hWnd, LPTB_DATE lpDate)
{
LPDATEFIELD lpFieldDate;
TCHAR       szDate[DATELENGTH];
BOOL        fRet = FALSE;

if (lpDate)
   {
   lpFieldDate = LockDateField(hWnd);

   if (DateDMYToString(lpDate, szDate, &lpFieldDate->df))
      fRet = TRUE;

   SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szDate);

   UnlockDateField(hWnd);
   }

return (fRet);
}


BOOL DLLENTRY DateGetDMY(HWND hWnd, LPTB_DATE lpDate)
{
LPDATEFIELD lpFieldDate;
LPEDITFIELD lpField;
BOOL        fRet = FALSE;

if (lpDate)
   {
   lpField = LockField(hWnd);
   lpFieldDate = LockDateField(hWnd);

   if (DateStringToDMYEx(lpField->lpszString, lpDate, &lpFieldDate->df,
                         lpFieldDate->nTwoDigitYearMax))
      fRet = TRUE;

   UnlockDateField(hWnd);
   UnlockField(hWnd);
   }

return (fRet);
}


void DLLENTRY DateSetJulianStart(short dYear)
{
DateJulianStart = dYear;
}


short DLLENTRY DateGetJulianStart(void)
{
return (DateJulianStart);
}


/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
쿑unction: DateJulianToDMY                                           �
�                                                                    �
쿏esc: Given a number of days from DateJulianStart, set d, m, and y  �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
LPTB_DATE DLLENTRY DateJulianToDMY( long jul, LPTB_DATE da)
{
   long l;
   
   l = jul;
   da->nYear = DateJulianStart;
   while ( ( l >= 365L && !ISLEAP( da->nYear)) || ( l >= 366L && ISLEAP( da->nYear)))
   {
      l -= 365L;
      if ( ISLEAP( da->nYear))
         l -= 1L;
      da->nYear++;
   }
   da->nMonth = 0;
   while ((int) l >= CalMonth[ da->nMonth].nDays[ ISLEAP( da->nYear)])
   {
      l -= (long)CalMonth[ da->nMonth].nDays[ ISLEAP( da->nYear)];
      da->nMonth++;
   }
   da->nMonth++;
   da->nDay = (int)l + 1;

   return da;
}


//-----------------------------------------------------------------------------
// Appends the Era name and year to the given string. (Masanori Iwasa)
//-----------------------------------------------------------------------------
void GetNengoFromDate(LPTB_DATE lpDate, LPTSTR szTmpStr, int nNengoType)
{
    LONG    lJulian;
    TCHAR   szBuff[3];
    int     i, iYear;

	//Modify by BOC 99.5.25 (hyt)------------------
	//for read execl sheet will change DateJulianStart
	//so get Julian value not correct
	short DateJulianStartSave;
	
	DateJulianStartSave = DateJulianStart;

	DateJulianStart = LOYEAR;

	lJulian = DateDMYToJulian(lpDate);

	DateJulianStart = DateJulianStartSave;
	//-------------------------------------------------
    
    //- Check to see if out of the Era range. If it is, 
    //  set 1868-09-08 as the default date.
    if (lJulian < NengoLimit[0])
        {
        lpDate->nYear = 1868;
        lpDate->nMonth = 9;
        lpDate->nDay = 8;
        }
//Add by BOC 99.7.22 (hyt) for not over 2087/12/31 (H99.12.31)
	else if(lJulian>762260)
	{
		lpDate->nYear = 2087;
		lpDate->nMonth = 12;
		lpDate->nDay = 31;
	}
//-----------------------------------------------------	

    //- Searches for the right Era info.
    for(i = 1; i <= 4; i++)
        if (lJulian < NengoLimit[i])
            {
            //- Pickup the right name according to specified type
            lstrcpy(szTmpStr, (LPTSTR)NengoName[nNengoType * 4 + i - 1]);
            //- Calculate the year to Japanese year
            iYear = lpDate->nYear - NengoYear[i - 1] + 1;
            if (iYear > 99) iYear = 99;
            wsprintf(szBuff, _T("%02d"), iYear);
            lstrcat(szTmpStr, szBuff);
            break;
            }
}


//-----------------------------------------------------------------------------
// Get the year in European format from the given string (Masanori Iwasa)
//    NengoType ->  0-NYYMMDD   1-NNYYMMDD   4-NNNNYYMMDD
//-----------------------------------------------------------------------------
int GetYearFromNengo(LPCTSTR lpDateStr, int NengoType)
{
    TCHAR   szEra[DATELENGTH];
    TCHAR   szYear[DATELENGTH];
    int     i;

    //- Retrieves the Era name and year from the string
    lstrcpy(szEra, lpDateStr);
    lstrcpy(szYear, lpDateStr + NengoType - 2);
    szEra[NengoType - 2] = '\0';
    szYear[2] = '\0';        

    //- Finds the matching Era and returns the Era base year
    //  with the year count retrieved from the string
    for (i = 0; i <= 11; i++)
        {
        if (!lstrcmp(szEra, (LPTSTR)NengoName[i]))
            return(NengoYear[i % 4]+StringToInt(szYear) - 1);
        }

	return (0);
}


//-----------------------------------------------------------------------------
// Initialize the date value in the Japanese Era format. (Masanori Iwasa)
//-----------------------------------------------------------------------------
void SetNengoInit(HWND hwnd, LPTSTR lpszDate, int NengoType, int Nengo, LPDATEEDITINFO lpEditInfo, BOOL bCurrentDate)
{
    TB_DATE     Date;
    LPDATEFIELD lpField;

    lpField = LockDateField (hwnd);

    if (bCurrentDate)
        SysGetDate (&Date.nDay, &Date.nMonth, &Date.nYear);
    else{
        Date.nDay   = NengoDay[Nengo];
        Date.nMonth = NengoMonth[Nengo];
        Date.nYear  = NengoYear[Nengo];
        }
        
    DateDMYToString (&Date, lpszDate, &lpField->df);
    lpEditInfo->nChars = lstrlen(lpszDate);
    lpEditInfo->iCurrentPos = NengoType;
    SetCaretPosition(hwnd, NengoType);

    UnlockDateField (hwnd);
}


//-----------------------------------------------------------------------------
// Check the range for Japanese Era type. (Masanori Iwasa)
//-----------------------------------------------------------------------------
void CheckNengoRange(HWND hwnd)
{
    TB_DATE     Date;
    LPDATEFIELD lpField;
    LPEDITFIELD lpEditField;
    TCHAR       DateStr[DATELENGTH], ResultDateStr[DATELENGTH];

    lpField = LockDateField (hwnd);

    switch (lpField->df.nFormat){
        case IDF_NYYMMDD:
        case IDF_NNYYMMDD:
        case IDF_NNNNYYMMDD:
            lpEditField = LockField(hwnd);
            lstrcpy((LPTSTR)DateStr, lpEditField->lpszString);
            UnlockField(hwnd);
            DateStringToDMYEx(DateStr, &Date, &lpField->df,
                              lpField->nTwoDigitYearMax);
            DateDMYToString (&Date, ResultDateStr, &lpField->df);
            if (lstrcmp(DateStr, (LPTSTR)ResultDateStr))
                SendMessage (hwnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)ResultDateStr);
        }

    UnlockDateField (hwnd);
}


//-----------------------------------------------------------------------------
// Scroll the Era up or down (Masanori Iwasa)
//-----------------------------------------------------------------------------
void NengoScroll(LPTB_DATE lpDate, int iDirection)
{
    LONG    lJulian;
    int     i;

    lJulian = DateDMYToJulian(lpDate);
    for(i = 1; i <= 4; i ++)
        if (lJulian < NengoLimit[i]) break;
    i += iDirection - 1;
    if (i < 0 || i > 3) return;
    lJulian = NengoLimit[i];
    DateJulianToDMY( lJulian, lpDate);
}


//-----------------------------------------------------------------------------
// Check to see if the separator is DBCS or not. (Masanori Iwasa)
//-----------------------------------------------------------------------------
BOOL IsDbcsSeparator(TUCHAR cSeparator)
{
#ifndef _UNICODE
    int     nChar = (BYTE)szYearGlobal[0];

    //- Checks to see if the loaded resource is DBCS. If it's not
    //  that means the VBX/OCX is a English version. Character '?'
    //  should not be a reserved in the English version.
    if(!_TIsDBCSLeadByte((BYTE)nChar))
        return FALSE;
    
    //- If the separator is the defined keyword ('?'), that means
    //  the DBCS separator is being used.
    if(cSeparator == KANJISEPARATOR)
        return TRUE;
///rap 6/30/2004 - 14219
#else
  #ifdef SPREAD_JPN
    if (cSeparator == KANJISEPARATOR) return TRUE;
  #endif
#endif
///rap 6/30/2004 - 14219

    return FALSE;
}
