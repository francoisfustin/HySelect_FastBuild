/*    PortTool v2.2     FPDATE.C          */

#include "fptools.h"
#include "string.h"    
#include <stdio.h>
#include <tchar.h>

short dDaysInMonth[] = 
   { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

short dJulianDays[] =
   { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
   
LONG FPLIB fpDateToJulian(FPDATE fpDate)
{
   long  yr = fpYEAR(fpDate)-1;
   long  julian = 0L;
   
   julian =  (yr * 365) + ((yr / 4) - (yr / 100)) + (yr / 400);
   julian += dJulianDays[fpMONTH(fpDate)-1] +
                (fpMONTH(fpDate) > 2 && fpISLEAP(fpDate) ? 1 : 0);
   julian += fpDAY(fpDate) - 1;

   return julian;
}


FPDATE FPLIB fpJulianToDate( long julian, int startingFromYear)
{
   long  yr;
   int  mn;
   long  dy;
   FPDATE fpDate;
   long l;
   
   // if julian is negative, adjust starting year.
   
   l = julian;
   yr = (long)startingFromYear;
   
   // if julian is negative, adjust starting year.
   if (l < 0)
      {
      while ((yr > 0) && (l < 0))
        {
        --yr;
        l += (ISLEAP(yr) ? 366L : 365L);
        }
      }
   
   // if still negative, return 0.
   if (l < 0)
     return( (FPDATE)0);
     
   while ( ( l >= 365L && !ISLEAP(yr)) || ( l >= 366L && ISLEAP(yr)))
   {
      l -= 365L;
      if ( ISLEAP(yr))
         l -= 1L;
      yr++;
   }
   mn = 0;
   while ( (int)l >= DAYSINMONTH(yr, (mn+1)) )
   {
      l -= (long)DAYSINMONTH(yr, (mn+1));
      mn++;
   }
   mn++;
   dy = (int)l + 1;

   fpSET_DATE(fpDate, yr, mn, dy);

   return fpDate;
}


// 0 = Sunday ...
//
short FPLIB fpDateWeekDay (FPDATE fpDate)
{
   return (short)((fpDateToJulian (fpDate) + 1) % 7L);
}

#ifdef WIN32
//WIN32
FPDATE FPLIB fpTodaysDate ()
{
   FPDATE fpDate = 0;
   SYSTEMTIME systime;
   
   GetLocalTime( &systime);  //km sel6760
   
   
   fpSET_DATE(fpDate, systime.wYear, systime.wMonth, systime.wDay);

   return fpDate;
}
#else  //not WIN32
FPDATE FPLIB fpTodaysDate ()
{
   extern void FAR PASCAL DOS3Call();
   FPDATE fpDate = 0;
   short  day, month, year;
   LPVOID lpDay = &day, lpMonth = &month, lpYear = &year;
   __asm
   {
      mov   ah, 2ah
      call  DOS3Call
      les   bx, lpYear
      mov   es:WORD PTR [bx], cx
      les   bx, lpMonth
      xor   ax, ax
      mov   al, dh
      mov   es: WORD PTR[bx], ax
      les   bx, lpDay
      mov   al, dl
      mov   es: WORD PTR[bx], ax
   }
   fpSET_DATE(fpDate, year, month, day);

   return fpDate;
}
#endif //not WIN32


BOOL FPLIB fpDateInRange (FPDATE fpDate, FPDATE fpMin, FPDATE fpMax)
{
   BOOL bInRange = TRUE;
   if (fpMin)
      if (fpDate < fpMin)
         bInRange = FALSE;
   if (bInRange && fpMax)
      if (fpDate > fpMax)
         bInRange = FALSE;
   return bInRange;
}

BOOL FPLIB fpYMDIsValid (int y, int m, int d)
{
   if (y < 10000)
      if ((unsigned)(m-1) < 12)
         {
         int dMax = dDaysInMonth[m-1] + 
            (m == 2 && ISLEAP(y) ? 1 : 0);
         if ((unsigned)(d-1) < (unsigned)dMax)
            return TRUE;
         } 
   return FALSE;                           
}   

BOOL FPLIB fpDateIsValid (FPDATE fpDate)
{
   return fpYMDIsValid(fpYEAR(fpDate), fpMONTH(fpDate), fpDAY(fpDate));
}

FPDATE FPLIB fpStringToDate(LPTSTR lpsz, WORD fFormat)
{
   FPDATE fpDate = fpNODATE;
   long date = 0;
   int y=0, m=0, d=0;
   
   if (lpsz)			//km sel8398
   {
      while (*lpsz && IsNum (*lpsz))
      {
      date = date*10 + (*lpsz-(TCHAR)'0');
      lpsz = CharNext(lpsz);
      }
   }
   if (fFormat == DATEF_YYYYMMDD)
      {
      y = (int)(date / 10000);
      date -= (long)y * 10000L;
      m = (int)(date / 100);
      date -= m*100;
      d = (int)date;
      if (fpYMDIsValid(y, m, d))
         fpSET_DATE(fpDate, y, m, d);
      }
   else if (fFormat == DATEF_MMDDYYYY)
      {
      m = (int)(date / 1000000);
      date -= (long)m * 1000000L;
      d = (int)(date / 10000);
      date -= d*10000;
      y = (int)date;
      if (fpYMDIsValid(y, m, d))
         fpSET_DATE(fpDate, y, m, d);
      }   
   return fpDate;
}


LPTSTR FPLIB fpDateToString(FPDATE fpDate, LPTSTR lpsz, WORD fFormat)
{
   TCHAR tmpstr[9]; 
   
   if (fFormat == DATEF_YYYYMMDD)
      {
      wsprintf( tmpstr, _T("%04d%02d%02d"),
               (int)fpYEAR(fpDate), (int)fpMONTH(fpDate), (int)fpDAY(fpDate));

      lstrcpy( lpsz, tmpstr);      
      }
   
   return lpsz;
}

