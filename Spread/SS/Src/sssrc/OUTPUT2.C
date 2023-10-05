#include "output.h"

/*
$Revision:   1.3  $
*/

int   far pascal TBatoi(const TCHAR far  * far *format)
{
   const TCHAR  far *str;

   register int   val;
   register int   count;
   int  neg;

   str = *format;
   count = 0;
   while (isspace(*str))
   {
      str++;
      count++;
   }

   str += (neg = *str == '-'); 

   val = 0;
   while(isdigit(*str))
   {
      val = val * 10 + * str++ - '0';
      count++;
   }
   if (neg)
   {
      count++;
      val = -val;
   }
   *format += count;
   return(val);
}

void far pascal _eint(TCHAR far *bufptr, int base, LPFLAGS lpFlags)
{
   int         i;
   TCHAR       buffer[9];
   TCHAR  far  *ptr = buffer;
   
   i = (int)lpFlags->ival;
   if (lpFlags->precision == 0 && i == 0)
      *bufptr = '\0';
   else
   {
      if (base == 10)
      {
         if (lpFlags->signflag)
         {
            if (i < 0)
            {
               *bufptr++ = '-';
               i= -i;
            }
            else
            if (lpFlags->plusflag)
               *bufptr++ = '+';
            else
            if (lpFlags->blankflag)
               *bufptr++ = ' ';
         }
         _dtoa(i, &ptr);
      }
      else
      if (base == 16)
      {
         if (lpFlags->poundflag & i)
         {
            *bufptr++ = '0';
            *bufptr++ = (TCHAR)(lpFlags->upperflag ? 'X' : 'x');
         }
         _htoa(i, &ptr, lpFlags->upperflag);
      }
      else
      {
         if (lpFlags->poundflag && i)
         {
            *bufptr++ = '\0';
         }
         _otoa(i, &ptr);
      }
      *ptr = '\0';
      for (i = lpFlags->precision - StrLen(buffer); i > 0; i--)
         *bufptr++ = '0';
      StrCpy(bufptr, buffer);
   }
}

void far pascal  _dtoa(unsigned n, TCHAR far * far *bufptr)
{
   if (n < 10)
      *(*bufptr)++ = (TCHAR)(n + '0');
   else
   {
      _dtoa(n / 10, bufptr);
      _dtoa(n % 10, bufptr);
   }
}

void far pascal  _htoa(unsigned n, TCHAR far * far *bufptr, int upperflag)
{
   if (n < 16)
   {
      if (n < 10)
         *(*bufptr)++ = (TCHAR)(n + '0');
      else
         *(*bufptr)++ = (TCHAR)(upperflag ? n + 55 : n + 87);
   }
   else
   {
      _htoa(n / 16, bufptr, upperflag);
      _htoa(n % 16, bufptr, upperflag);
   }
}

void  far pascal _otoa(unsigned n, TCHAR far * far *bufptr)
{
   if (n < 8)
      *(*bufptr)++ = (TCHAR)(n + '0');
   else
   {
      _otoa(n / 8, bufptr);
      _otoa(n % 8, bufptr);
   }
}


