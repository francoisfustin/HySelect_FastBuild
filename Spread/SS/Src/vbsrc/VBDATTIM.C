/*********************************************************
* VBDATTIM.C
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
//#include <vbapi.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "toolbox.h"
//#include "vbmisc.h"
#include "vbdattim.h"
#include <tchar.h>

static short DaysPerMonth[2][12] =
   {
   {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
   {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
   };


BOOL DateValidate(lpszText, lpDate)

LPTSTR  lpszText;
LPTB_DATE lpDate;
{
if (lstrlen(lpszText) != 8)
   return (FALSE);

DateValueToDMY(lpszText, lpDate);

return (DateIsValid((short)lpDate->nMonth, (short)lpDate->nDay, (short)lpDate->nYear));
}

// Modify By BOC 99.6.15 (hyt)--Just get from spread 2.5J sourcecode--------------------
// Begin modify - by BOC(Tang) - for bug03(Tang Tian Qin) - 07/04/1996 ------------------------->>
// Cause: Date format of japan verion is "yyyymmdd", so this function need some change.
// Note: bug03 can be correct by modifying function SS_DateToString in Ss_type.c(2998),
//       but it will change format of date value of spread to english style, and something wrong
//		 with setting typedatemax and typedatemin, so I think make changing here is better.
void DateValueToDMY(lpszText, lpDate)

LPTSTR  lpszText;
LPTB_DATE lpDate;
{
TCHAR   szBuffer[4 + 1];

if (lstrlen(lpszText) != 8)
   return;

#ifdef SPREAD_JPN
	_fmemcpy(szBuffer, &lpszText[4], 2);
	szBuffer[2] = 0;
	lpDate->nMonth = StringToInt(szBuffer);

	_fmemcpy(szBuffer, &lpszText[6], 2);
	szBuffer[2] = 0;
	lpDate->nDay = StringToInt(szBuffer);

	_fmemcpy(szBuffer, &lpszText[0], 4);
	szBuffer[4] = 0;
	lpDate->nYear = StringToInt(szBuffer);
#else
	_fmemcpy(szBuffer, &lpszText[0], 2*sizeof(TCHAR));
	szBuffer[2] = 0;
	lpDate->nMonth = StringToInt(szBuffer);

	_fmemcpy(szBuffer, &lpszText[2], 2*sizeof(TCHAR));
	szBuffer[2] = 0;
	lpDate->nDay = StringToInt(szBuffer);

	_fmemcpy(szBuffer, &lpszText[4], 4*sizeof(TCHAR));
	szBuffer[4] = 0;
	lpDate->nYear = StringToInt(szBuffer);
#endif
}


BOOL DateToString(LPTSTR lpszText, LPTB_DATE lpDate)
{
#ifdef SPREAD_JPN
	wsprintf(lpszText, _T("%04d%02d%02d"), lpDate->nYear, lpDate->nMonth,
         lpDate->nDay);
#else
	wsprintf(lpszText, _T("%02d%02d%04d"), lpDate->nMonth, lpDate->nDay,
         lpDate->nYear);
#endif
return (TRUE);
}
//End Modify-------------------------------------------------------------

short DateIsLeapYear(Year)

short Year;
{
if (((Year % 4) == 0 && (Year % 100) != 0) ||
    ((Year % 100) == 0 && (Year % 400) == 0))
   return (TRUE);
else
   return (FALSE);
}


short DateDaysPerMonth(Month, Year)

short        Month;
short        Year;
{
if (Month <= 0 || Month > 12)
   return (0);

if (Year < 1900)
   Year += Year >= 50 ? 1900 : 2000;

return (DaysPerMonth[DateIsLeapYear(Year)][Month - 1]);
}


long vbDateDMYToJulian(LPTB_DATE lpDate)
{
long  Julian = 0L;
short i;
// If year is less than 1900, result is negative.
int   iDir = (lpDate->nYear >= 1900 ? 1 : -1);
   
for (i = (iDir == 1 ? 1900 : 1899); i != lpDate->nYear; i += iDir)
   {
   Julian += 365;
   if (DateIsLeapYear(i))
      Julian++;
   }

for (i = (iDir == 1 ? 0 : 11); i != (lpDate->nMonth - 1) ; i += iDir)
   Julian += DaysPerMonth[DateIsLeapYear((short)lpDate->nYear)][i];

// If iDir is neg, subtract day from num of days in month
Julian += 
  (iDir == 1 ? (lpDate->nDay - 1)
   : DaysPerMonth[DateIsLeapYear((short)lpDate->nYear)][lpDate->nMonth-1] - lpDate->nDay);

// If iDir is neg, add 1 to julian and then change its sign. (We add 1
// because we are calc the number of days from 12/31/1899 instead of
// 1/1/1900.
if (iDir == -1)
  Julian = -(Julian + 1);

return (Julian);
}


LPTB_DATE vbDateJulianToDMY(long jul, LPTB_DATE da)
{
   int iDir = (jul >= 0 ? 1 : -1);  // if jul is neg, subtract years.
   long l;
   BOOL fIsLeapYear;

   // If "jul" is positive use "jul". 
   // Else, use absolute value of "jul" - 1. The -1 is because we are
   // starting with 12/31/1899 instead of 1/1/1900.
   l = (iDir == 1 ? jul : (-jul) - 1);
   da->nYear = (iDir == 1 ? 1900 : 1899); // if neg, start with 1899
   while ( (l >= 365L && !DateIsLeapYear((short)da->nYear)) ||
           (l >= 366L && DateIsLeapYear((short)da->nYear)) )
      {
      l -= 365L;
      if (DateIsLeapYear((short)da->nYear))
         l -= 1L;
      da->nYear += iDir; // add/sub 1
      }
   da->nMonth = (iDir == 1 ? 0 : 11); // start at JAN or DEC.
   fIsLeapYear = DateIsLeapYear((short)da->nYear);

   while (l >= (long)(DaysPerMonth[fIsLeapYear][da->nMonth]))
      {
      l -= (long)(DaysPerMonth[fIsLeapYear][da->nMonth]);
      da->nMonth += iDir; // add/sub 1
      }
   // Get the "l"th day of the month. If dir is neg, subtract "l" from
   // the num of days in the month.
   da->nDay = (int)(iDir == 1 ? (int)l + 1 : 
                    (int)(DaysPerMonth[fIsLeapYear][da->nMonth]) - l);

   da->nMonth++;  // change from DaysPerMonth index to month number.

   return da;
}


BOOL DateIsValid(Month, Day, Year)

short Month;
short Day;
short Year;
{
if (Month < 1 || Month > 12)
   return (FALSE);

if (Day < 1 || Day > DateDaysPerMonth(Month, Year))
   return (FALSE);

return (TRUE);
}


BOOL TimeValidate(lpszText, lpTime)

LPTSTR lpszText;
LPTIME lpTime;
{
TCHAR  szBuffer[2 + 1];

if (lstrlen(lpszText) != 4 && lstrlen(lpszText) != 6)
   return (FALSE);

szBuffer[2] = 0;

_fmemcpy(szBuffer, &lpszText[0], 2*sizeof(TCHAR));
lpTime->nHour = StringToInt(szBuffer);

if (lpTime->nHour < 0 || lpTime->nHour > 23)
   return (FALSE);

_fmemcpy(szBuffer, &lpszText[2], 2*sizeof(TCHAR));
lpTime->nMinute = StringToInt(szBuffer);

if (lpTime->nMinute < 0 || lpTime->nMinute > 59)
   return (FALSE);

if (lstrlen(lpszText) == 6)
   {
   _fmemcpy(szBuffer, &lpszText[4], 2*sizeof(TCHAR));
   lpTime->nSecond = StringToInt(szBuffer);

   if (lpTime->nSecond < 0 || lpTime->nSecond > 59)
      return (FALSE);
   }

return (TRUE);
}


BOOL TimeToString(LPTSTR lpszText, LPTIME lpTime)
{
wsprintf(lpszText, _T("%02d%02d%02d"), lpTime->nHour, lpTime->nMinute,
         lpTime->nSecond);

return (TRUE);
}


#ifndef SS_OCX

void DateDefault(lpDateFormat)

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

if (Ptr = StrChr(szTemp, 'y'))
   if (StriStr(Ptr, _T("yyyy")))
      lpDateFormat->bCentury = 1;

lpDateFormat->bSpin = 0;
}


void TimeDefault(lpTimeFormat)

LPTIMEFORMAT lpTimeFormat;
{
TCHAR         szTemp[2];

lpTimeFormat->b24Hour = (BOOL)GetProfileInt(_T("intl"), _T("iTime"), 0);
lpTimeFormat->bSeconds = FALSE;

GetProfileString(_T("intl"), _T("sTime"), _T(":"), szTemp, 2);
lpTimeFormat->cSeparator = szTemp[0];
lpTimeFormat->bSpin = 0;
}
#endif // SS_OCX
