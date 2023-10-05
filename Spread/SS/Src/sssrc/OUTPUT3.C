/*
$Revision:   1.3  $
*/

#include "output.h"

void  far pascal _elong(TCHAR far *bufptr, int base, LPFLAGS lpFlags)
{
   TCHAR        buffer[15];
   TCHAR far   *ptr = buffer;
   int         i;
   long        l;

   l = lpFlags->ival;
   if (lpFlags->precision == 0 && l == 0)
      *bufptr = '\0';
   else
   {
      if (base == 10)
      {
         if (lpFlags->signflag)
         {
            if (l < 0)
            {
               *bufptr++ = '-';
               l = -l;
            }
            else
            if (lpFlags->plusflag)
               *bufptr++ = '+';
            else
            if (lpFlags->blankflag)
               *bufptr++ = ' ';
         }
         _ldtoa(l, &ptr);
      }
      else
      if (base == 16)
      {
         if (lpFlags->poundflag && l)
         {
            *bufptr++ = '0';
            *bufptr++ = (TCHAR)(lpFlags->upperflag ? 'X' : 'x');
         }
         _lhtoa(l , &ptr, lpFlags->upperflag);
      }
      else
      {
         if (lpFlags->poundflag && l)
            *bufptr++ = '0';
         _lotoa(l, &ptr);
      }
      *ptr = '\0';
      for (i = lpFlags->precision - StrLen(buffer); i > 0; i--)
         *bufptr++ = '0';
      StrCpy(bufptr, buffer);
   }
}

void  far pascal _ldtoa(unsigned long l, TCHAR far * far *bufptr)
{
   if (l < 10)
      *(*bufptr)++ = (TCHAR)(l + '0');
   else
   {
      _ldtoa(l / 10, bufptr);
      _ldtoa(l % 10, bufptr);
   }
}

void  far pascal _lhtoa(unsigned long l, TCHAR far * far *bufptr, int upperflag)
{
   register int   start;
   register int   i;
   int            digit[9];

   digit[8] = 1;
   for (i = 7; i >= 0; i--)
   {
      digit[i] = (int)(l & 0xf);
      l >>= 4;
   }
   start = 0;
   while (digit[start] == 0)
      start++;
   if (start == 8)
      *(*bufptr)++ = '0';
   else
   {
      for (i = start; i < 8; i++)
      {
         if (digit[i] < 10)
            *(*bufptr)++ = (TCHAR)(digit[i] + '0');
         else
            *(*bufptr)++ = (TCHAR)(upperflag ? digit[i] + 55 : digit[i] + 87);
      }
   }
}

void  far pascal _lotoa(long l, TCHAR far * far *bufptr)

{
   register int   start;
   register int   i;
   int            digit[12];

   digit[11] = 1;
   for (i = 10; i > 0; i--)
   {
      digit[i]  = (int)(l & 7);
      l = l >> 3;
   }
   digit[0] = (int)(l & 3);
   start = 0;
   while (digit[start] == 0)
      start++;
   if (start == 11)
      *(*bufptr)++ = '0';
   else
   {
      for ( i = start; i < 11; i++)
         *(*bufptr)++ = (TCHAR)(digit[i] + '0');
   }
}

far pascal _mputc(int c, TCHAR far * far *fp)
{
   *(*fp)++ = (TCHAR)c;
   return(c);
}

far pascal _mwrite(TCHAR far * far *fp, TCHAR far *s, int n)
{
   register int   i;

   for (i = n; i > 0; --i)
      *(*fp)++ = *s++;
   return(n);
}

