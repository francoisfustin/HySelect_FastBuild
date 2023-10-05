//--------------------------------------------------------------------
//
//  File: ftdattim.c
//
//  Description: Date and time routines
//
//  Copyright (c) 1999 by FarPoint Technologies, Inc.
//
//  All rights reserved.  No part of this source code may be
//  copied, modified or reproduced in any form without retaining
//  the above copyright notice.  This source code, or source code
//  derived from it, may not be redistributed without express
//  written permission of FarPoint Technologies, Inc.
//
//--------------------------------------------------------------------

#include <windows.h>
#include <math.h>

#include "ftdattim.h"


//--------------------------------------------------------------------
//
//  The following variables contain the number of days in a month,
//  a year, a 4 year period, a 100 year period, and a 400 year
//  period.  For the two element arrays, element 0 is for regular
//  years (or periods) and element 1 is for leap years (or periods).
//

const int nDaysMon[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                             {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
const long nDaysYear[2] = {365, 366};
const long nDays4Years[2] = {1460, 1461};
const long nDays100Years[2] = {36524, 36525};
const long nDays400Years = 146097;

//--------------------------------------------------------------------
//
//  The following constant represents December 30, 1899 as days
//  in the AD era (assuming a year zero and no pre-Gregorian
//  calendar adjustments).  This constant is used in conversions
//  between doubles and dates (year, month, day).
//

#define BASE_DATE 693959

//--------------------------------------------------------------------
//
//  The following function converts a double to a date (year, month,
//  day).  The whole part of the double represents the number of
//  days since December 30, 1899.  For example, 2.0 represents
//  January 01, 1900.
//
//  In order to simplify calculations, the double is first converted
//  to the number of days in the AD era (assumming a year zero and
//  no pre-Gregorian calendar adjustments).  It is then converted
//  to a date.
//

BOOL ftDoubleToDate(double dfVal, short *pnYear, short *pnMon, short *pnDay)
{
  long lVal = (long)dfVal + BASE_DATE;
  int nYear = 0;
  int nMon = 0;
  BOOL bLeap = TRUE;

  for( ; lVal >= nDays400Years; nYear += 400 )
    lVal -= nDays400Years;
  for( ; lVal >= nDays100Years[bLeap]; nYear += 100, bLeap = FALSE )
    lVal -= nDays100Years[bLeap];
  for( ; lVal >= nDays4Years[bLeap]; nYear += 4, bLeap = TRUE )
    lVal -= nDays4Years[bLeap];
  for( ; lVal >= nDaysYear[bLeap]; nYear += 1, bLeap = FALSE )
    lVal -= nDaysYear[bLeap];
  for( ; lVal >= nDaysMon[bLeap][nMon]; nMon += 1 )
    lVal -= nDaysMon[bLeap][nMon];
  if( pnYear )
    *pnYear = nYear;
  if( pnMon )
    *pnMon = nMon + 1;
  if( pnDay )
    *pnDay = (short)(lVal + 1);
  return TRUE;
}



//--------------------------------------------------------------------
//
//  The following function converts a date (year, month, day) to
//  a double.  The whole part of the double represents the number
//  of days since December 30, 1899.  For example, 2.0 represents
//  January 01, 1900.
//
//  In order to simplify calculations, the date is first converted
//  to the number of days in the AD era (assumming a year zero and
//  no pre-Gregorian calendar adjustments).  It is then converted
//  to the number of days since December 30, 1899.
//

BOOL ftDateToDouble(int nYear, int nMon, int nDay, double *pdfVal)
{
  BOOL bLeap = nYear%4==0 && (nYear%100!=0 || nYear%400==0);
  long lVal;
  int i;

  // Verify that the month and day are valid, if not, return FALSE.
  if (nMon < 1 || nMon > 12 || nDay < 1 || nDay > nDaysMon[bLeap][nMon-1])
  {
    *pdfVal = 0.0;    // reset date and return FALSE
    return FALSE;
  }

  lVal = (nYear)*365 + (nYear+3)/4 - (nYear+99)/100 + (nYear+399)/400;
  for( i = 0; i < nMon - 1; i++ )
    lVal += nDaysMon[bLeap][i];
  lVal += nDay - 1;
  *pdfVal = lVal - BASE_DATE;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The following function converts a double to a time (hour, minute
//  second).  The fractional part of the double represents the
//  fraction of a day.  For example, 0.25 represents 6:00 AM.
//
//  Note: Leap seconds are not handled.
//

BOOL ftDoubleToTime(double dfVal, short *pnHour, short *pnMin, short *pnSec)
{
  double dfInt;
  long lVal = (long)(fabs(modf(dfVal,&dfInt)) * 24.0 * 60.0 * 60.0 + 0.5);

  if( pnHour )
    *pnHour = (short)(lVal / 60 / 60);
  if( pnMin )
    *pnMin = (short)(lVal / 60 % 60);
  if( pnSec )
    *pnSec = (short)(lVal % 60);
  return TRUE;
}


//--------------------------------------------------------------------
//
//  The following function converts a double to a time (hour, minute,
//  second, and milliseconds).  The fractional part of the double 
//  represents the fraction of a day.  For example, 0.25 represents 
//  6:00 AM.
//
//  Note: Leap seconds are not handled.
//
BOOL ftDoubleToTimeEx(double dfVal, short *pnHour, short *pnMin, 
  short *pnSec, short *pnMilliSec)
{
  double dfInt;
  long lVal =        // number of milliseconds
    (long)(fabs(modf(dfVal,&dfInt)) * 24.0 * 60.0 * 60.0 * 1000.0 + 0.5);

  if( pnHour )
    *pnHour = (short)(lVal / (60 * 60 * 1000));
  if( pnMin )
    *pnMin = (short)(lVal / (60 * 1000) % 60);
  if( pnSec )
    *pnSec = (short)(lVal / 1000 % 60);
  if( pnMilliSec )
    *pnMilliSec = (short)(lVal % 1000);

  return TRUE;
}

//--------------------------------------------------------------------
//
//  The following function converts a time (hour, minute, second) to
//  a double.  The fractional part of the double represents the
//  fraction of a day.  For example, 0.25 represents 6:00 AM.
//
//  Note: Leap seconds are not handled.
//

BOOL ftTimeToDouble(int nHour, int nMin, int nSec, double *pdfVal)
{
  long lVal;
  
  if (nHour == -1)
    nHour = 0;
  if (nMin == -1)
    nMin = 0;
  if (nSec == -1)
    nSec = 0;

  lVal = (long)nHour * 60 * 60 + (long)nMin * 60 + nSec;
  *pdfVal = (double)lVal / (24.0 * 60.0 * 60.0);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The following function converts a double (representing a date)
//  to a weekday (0=Sun, 1=Mon, ..., 6=Sat).
//

void ftDoubleToWeekday(double dfVal, short *pnWeekday)
{
  long lVal = (long)dfVal + BASE_DATE;

  *pnWeekday = (short)((lVal + 6) % 7);
}


//--------------------------------------------------------------------
// Returns the last day of the month for the given month/year pair.
// The year is optional, but if not specified (-1) and the month is
// February, then the maximum day allowed for the month (on Leap Year)
// is returned as the last day of the month (29).
//
// If an invalid month is specified, then -1 is returned.
//
int LastDayOfMonth(int iMonth, int iYear)
{
  int iLeapYear = FALSE;

  // If invalid month, return -1.
  if (iMonth < 1 || iMonth > 12)
    return -1;

  // If the month specified is Febraury and the year is specified,
  // then determine if the year is Leap Year.
  if (iMonth == 2 && iYear > -1)
    iLeapYear = 
     ((iYear % 4 == 0 && (iYear % 100 != 0 || iYear % 400 == 0)) ? 1 : 0);

  return nDaysMon[iLeapYear][iMonth-1];
}
