/* output1.c */
/*
$Revision:   1.7  $
*/


#define  USE_STACK

#include "output.h"
#include <stdlib.h>
#include <tchar.h>

double   val;

static  int  InsertComma(TCHAR far *szVal, int cSep, int cDecimal);
static void _tbcftof(double near *lpdfVal, TCHAR NEAR* cvtb, short Right);
static void _tbcftoe(double near *lpdfVal, TCHAR NEAR* cvtb, short Right,
                     BOOL fUpperCase);

#pragma warning(disable : 4700)
int FAR PASCAL _output(LPTSTR fp, LPCTSTR format, va_list FAR* lpParms) //, int (*putc)(), int (*write)())
{
   int            (far pascal *putc)(int, TCHAR far * far *) = _mputc;
   int            (far pascal *write)(TCHAR far * far*, TCHAR far *, int) = _mwrite;
   TCHAR    far   *bufptr = NULL;
   int            c;
   int            padchar;
   register int   count;
   int            ljflag;
   int            longflag;
   int            Nflag;
   int            Fflag;
   register int   length;
   int            flag;
   int            left;
   int            right;
   int            width;
   TCHAR    far   *null = _T("(null)");
   TCHAR    cvtb[CVTBUFSIZE];

#if   defined(USE_STACK)
   TCHAR          buffer[MAX_STRLEN+1];
#else
   PSTR           buffer;
   HANDLE         hbuffer;
#endif

   FLAGS          flags;
   TCHAR          cSep;
   TCHAR          cDecimal;
   int            fSeparatorSupplied;
   void           (far pascal *enc)(TCHAR far *, int, LPFLAGS);

   count = 0;
   // remove this if buffer is a stack array var
#if   !defined(USE_STACK)
   /*
   buffer = LocalAlloc(NONZEROLPTR, (MAX_STRLEN + 1) * sizeof(TCHAR));
   */
   hbuffer = LocalAlloc(NONZEROLHND, (MAX_STRLEN + 1) * sizeof(TCHAR));
   buffer = LocalLock(hbuffer);
#endif
   while (c = *format)
   {
      if (c != '%')
      {
         if (c == '\\' && *(format + 1) == 'n')
         {   format++;
             c = *format;
             if (c == 'n')
                 (*putc)('\n', (TCHAR far * far *)fp);
            count++;
            format++;
         } else
         {
             (*putc)(c, (TCHAR far * far *)fp);
             count++;
             format++;
        }
         continue;
      }
      bufptr = buffer;
      ljflag = longflag = Nflag = Fflag = 0;
      flags.signflag = 0;
      flags.plusflag = 0;
      flags.blankflag = 0;
      flags.poundflag = 0;
      flags.upperflag = 0;
      flags.farflag = 0;
      flags.precision = -1;
      fSeparatorSupplied = 0;
      moreflags:
      {
         switch (*(++format))
         {
            case '-':
               ljflag++;
               goto moreflags;

            case '+':
               flags.plusflag++;
               goto moreflags;

            case ' ':
               flags.blankflag++;
               goto moreflags;

            case '#':
               flags.poundflag++;
               goto moreflags;

         } /* case */
      } /* none */
      if (*format == '0')
      {
         padchar = '0';
         ++format;
      }
      else
         padchar = ' ';
      if (*format == '*')
      {
         ++format;
         if ((width = va_arg(*lpParms, int)) < 0)
         {
            width = -width;
            ++ljflag;
         }
      }
      else
      {
         width = TBatoi(&format);
      }
      if (*format == '.')
      {
         ++format;
         if (*format == '*')
         {
            ++format;
            flags.precision = va_arg(*lpParms, int);
         }
         else
            flags.precision = TBatoi(&format);
      }
      enc = _eint;
      if (((c = *format++) == 'l') || c == 'L')
      {
         c = *format++;
         enc = _elong;
         longflag++;
      }
      else
      if (c == 'h')
         c = *format;
      else
      if (c == 'N')
      {
         Nflag++;
         c = *format++;
      }
      else
      if (c == 'S')        // separator supplied
      {
         fSeparatorSupplied++;
         c = *format++;
      }
      switch (c)
      {
         case 'i':
         case 'd':
            flags.signflag++;    // fall through
         case 'u':
            if (longflag)
               flags.ival = va_arg(*lpParms, long);
            else
               flags.ival = (int)va_arg(*lpParms, int);
            (*enc)(buffer, 10, &flags);
            length = StrLen(bufptr);
            break;

         case 'o':
            if (longflag)
               flags.ival = va_arg(*lpParms, long);
            else
               flags.ival = (int)va_arg(*lpParms, int);
            (*enc)(buffer, 8, &flags);
            length = StrLen(bufptr);
            break;

         case 'X':
            flags.upperflag++;
         case 'x':
            if (longflag)
               flags.ival = va_arg(*lpParms, long);
            else
               flags.ival = (int)va_arg(*lpParms, int);
            (*enc)(buffer, 16, &flags);
            length = StrLen(bufptr);
            break;

         case 'p':
            flags.precision = 4;
            flags.upperflag++;
            flags.poundflag = 0;
            if (Nflag)
            {
               flags.ival = (int)va_arg(*lpParms, int);
               _eint(buffer, 16, &flags);
               length = 4;
            }
            else
            {
               flags.farflag++;
               bufptr = va_arg(*lpParms, TCHAR far *); // Pragma at top gets rid of this warning(C4700) which is causee by a compiler bug
               flags.ival = (int)FP_SEG(bufptr);
               _eint(buffer, 16, &flags);
               buffer[4] = ':';
               flags.ival = FP_OFF(bufptr);
               _eint(buffer + 5, 16, &flags);
               length = 9;
               flags.farflag = 0;
            }
            bufptr = buffer;
            break;

         case 's':
            if (Nflag)
               bufptr = va_arg(*lpParms, TCHAR near *);
            else
               bufptr = va_arg(*lpParms, TCHAR far *);
            if (bufptr == NULL)
            {
               bufptr = null;
               length = 6;
               break;
            }
            length = StrLen(bufptr);
            if (length >= MAX_STRLEN)
            {
               bufptr = null;
               length = 6;
            }
            if (flags.precision >= 0 && length > flags.precision)
               length = flags.precision;
            break;

         case 'c':
            buffer[0] = (TCHAR)va_arg(*lpParms, int);
            buffer[1] = '\0';
            length = 1;
            break;

         case 'm':            // money
            right = 0;
            if (fSeparatorSupplied)
            {
               cSep = (TCHAR)va_arg(*lpParms, int);
               cDecimal = (TCHAR)va_arg(*lpParms, int);
            }
            else        // default to English separator
            {
               cSep = ',';
               cDecimal = '.';
            }
            // BJO 16Aug96 SCS3591 - Before fix
            //// if the precision is supplied then it is a double
            //if (flags.precision > 0)
            //{
            //   right = flags.precision;
            //   val = va_arg(parms, double);
            //   _tbcftof(&val, cvtb, (short)right);
            //}
            //else  // otherwise we assume it is a long
            //{
            //   LongToString(va_arg(parms, long), cvtb);
            //}
            // BJO 16Aug96 SCS3591 - Begin fix
            right = flags.precision;
            val = va_arg(*lpParms, double);
            _tbcftof(&val, cvtb, (short)right);
            // BJO 16Aug96 SCS3591 - End fix
            length = InsertComma(cvtb, cSep, cDecimal);
            StrCpy(bufptr, cvtb);
            break;

         case 'E':
            flags.upperflag++;
         case 'e':
            flag = 1;
            goto reals;

         case 'G':
            flags.upperflag++;
         case 'g':
            flag = 2;
            if (flags.precision < 0)
               flags.precision = 6;
            goto reals;

         case 'f':
            flag = 0;
            reals:
               if (flags.plusflag)
                  flag |= 8;
               else
               if (flags.blankflag)
                  flag |= 136;
               if (flags.precision < 0)
//-Fengwei
// Because there is no fractionwidth parameter when call 
// FloatSetRange funciton, so there is no better way to 
// inform this routine's fractionwidth, so give it max
// fractionwidth of double type, otherwise, the min and max
// will be 10.0000000 after calling FlaotSetRange with the 
// min and max is 9.999999.
// 
#ifdef	SPREAD_JPN
				  right = 16;
#else
                  right = 6;
#endif
               else
                  right = flags.precision;
               left = 1;
               val = va_arg(*lpParms, double);
               if (flag & 1)
               {
                  _tbcftoe((double near *)&val, (TCHAR near *)cvtb, (short)right, flags.upperflag);
               }
               else
               if (flag & 2)     // '%G' option
               {
                  if ((val > 2.8114749767e14) || (val < 1.0 / 2.8114749767e14))   // must use 'E' option
                     _tbcftoe((double near *)&val, (TCHAR near *)cvtb, (short)right, flags.upperflag);
                  else     // we can use '%f' option
                     _tbcftof((double near *)&val, (TCHAR near *)cvtb, (short)right);
               }
               else
                  _tbcftof((double near *)&val, (TCHAR near *)cvtb, (short)right);
               if (flag & 8)     // plus flag is on
               {
                  if (cvtb[0] == '-')  // a negative sign
                     StrCpy(bufptr, cvtb);
                  else
                  if (cvtb[0] == '+')  // a negative sign
                     StrCpy(bufptr, cvtb);
                  else // it must be positive
                  {
                     *bufptr = '+';
                     StrCpy(bufptr + 1, cvtb);
                  }
               }
               else
                  StrCpy(bufptr, cvtb);
#ifdef   NEVER
               length = 0;
               while (c = buffer[length])
               {
                  if (flags.poundflag == 0)
                  {
                     // if there is a decimal marker and precision is
                     // given as zero then do not copy it
                     if (c == '.' && flags.precision == 0)
                     {
                        StrCpy(&buffer[length], &buffer[length + 1]);
                        continue;
                     }
                  }
                  // if an exponent is present and it needs to be
                  // upper flag then convert it
                  if (c == 'e' && flags.upperflag)
                     buffer[length] = 'E';
                  // otherwise convert to lowercase, if necessary
                  else
                  if (c == 'E' && !flags.upperflag)
                     buffer[length] = 'e';
                  ++length;
               }
#endif
               length = StrLen(bufptr);
               break;

         case 'n':   // write the current buffer count
            *(va_arg(*lpParms, int far *)) = count;

         default:    // must be a normal character, write it to the buffer
            (*putc)(c, (TCHAR far * far *)fp);
            count++;
            continue;
      } /* case */
      width -= length;
      if (!ljflag)
      {
         if (padchar == '0' && (*bufptr == '-' || *bufptr == '+'))
         {
            --length;
            (*putc)(*bufptr++, (TCHAR far * far *)fp);
            count++;
         }
         while (width-- > 0)
         {
            (*putc)(padchar, (TCHAR far * far *)fp);
            count++;
         }
      }
      (*write)((TCHAR far * far *)fp, bufptr, length);
      count += length;
      while (width-- >0)
      {
         (*putc)(padchar, (TCHAR far * far *)fp);
         count++;
      }
   } // while

#if   !defined(USE_STACK)
   // remove this if buffer is a stack array var
   LocalUnlock(hbuffer);
   LocalFree(hbuffer);
   /*
   LocalFree((HANDLE)buffer);
   */
#endif
   return(count);
}


/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ Func: static int InsertComma(TCHAR far *szVal, int cSep, int cDecimal) ³
³                                                                        ³
³ Desc: this function is for output function only. Its use is to insert  ³
³       thousand separator and decimal character in the string szVal.    ³
³       So please make sure szVal is big enough to accomodate the entire ³
³       string                                                           ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/

static int InsertComma(TCHAR far *szVal, int cSep, int cDecimal)
{
   TCHAR szFraction[30];        // contains the fractional part, if any
   TCHAR szInteger[CVTBUFSIZE]; // contains the integer part, if any
   int   ch;                    // current character read from the org string
   int   index;                 // general index loop
   int   iLen;                  // general string length
   int   iStrLen;               // the length returned to the caller
   int   frac = 1;              // this is zero if there is no fraction
   int   charcount;
   LPTSTR szDotPos1;
   LPTSTR szDotPos2 = NULL;

   szInteger[0] = szFraction[0] = '\0';
   szDotPos1 = szVal;
   // look for the '.' character. this is assuming that the decimal separator
   // is a '.'
   while (*szDotPos1 && *szDotPos1 != '.')
      ++szDotPos1;
   if (!*szDotPos1)     // can't find it so set frac to 0
      frac = 0;
   else
      szDotPos2 = szDotPos1;
   // now copy the integer part in reverse and in the process insert the
   // thousand separator
   charcount = 0;
   for (index = 0; szDotPos1 > szVal; ++index)
   {
      szInteger[index] = *--szDotPos1;
      charcount++;
      if ((charcount % 3) == 0)        // we need to insert a separator
         szInteger[++index] = (TCHAR)cSep;
   }
   // now szInteger is comma separated but in reverse
   if (szInteger[index - 1] == '-' && szInteger[index - 2] == (TCHAR)cSep)
   {
      szInteger[index - 2] = '-';      // this is necessary if cSep is the
      --index;                         // 2nd last character
   }
   else
   if (szInteger[index - 1] == (TCHAR)cSep)
      --index;                         // this is necessary if cSep is the last char
   szInteger[index] = '\0';
   iLen = index - 1;
   if (frac)
   {
      index = 0;     // there is a fraction so copy it
      while ((ch = *++szDotPos2) && index < 30)
      {
         szFraction[index++] = (TCHAR)ch;
      }
      szFraction[index] = '\0';
   }
   // now copy it back to szVal
   iStrLen = iLen;
   while (iLen >= 0)    // copy the integer part to szVal in reverse
      *szVal++ = szInteger[iLen--];
   if (frac)
   {
      *szVal++ = (TCHAR)cDecimal;
      iLen = index;
      index = 0;
      while (index < iLen)       // copy the fractional part not in reverse!
         *szVal++ = szFraction[index++];
      iStrLen += index + 1;
   }
   *szVal = '\0';
   return(iStrLen + 1);
}


void _tbcftof(double NEAR *lpdfVal, TCHAR NEAR* cvtb, short Right)
{
int   Dec;
int   Sign;
short i;

// BJO 03Sep96 ARR243 - Begin fix, remove LongToString() processing
//if (Right <= 0)
//   LongToString((long)*lpdfVal, cvtb);
//
//else
// BJO 03Sep96 ARR243 - End fix
   {
   FloatToFString(cvtb, *lpdfVal, Right, &Dec, &Sign);

   if (Dec <= 0)
      {
      Dec = abs(Dec);

      for (i = 0; i < Dec; i++)
         StrInsertChar('0', cvtb, 0);

      StrInsertChar('.', cvtb, 0);
      StrInsertChar('0', cvtb, 0);
      }
   else
      StrInsertChar('.', cvtb, Dec);

   if (Sign)
      StrInsertChar('-', cvtb, 0);
   }
}


void _tbcftoe(double near *lpdfVal, TCHAR NEAR* cvtb, short Right, BOOL fUpperCase)
{
int   Dec;
int   Sign;
short i;

if (Right <= 0)
   LongToString((long)*lpdfVal, cvtb);

else
   {
   FloatToEString(cvtb, *lpdfVal, Right, &Dec, &Sign);

   if (Dec <= 0)
      {
      Dec = abs(Dec);

      for (i = 0; i < Dec; i++)
         StrInsertChar('0', cvtb, 0);

      StrInsertChar('.', cvtb, 0);
      StrInsertChar('0', cvtb, 0);
      }
   else
      StrInsertChar('.', cvtb, Dec);

   if (Sign)
      StrInsertChar('-', cvtb, 0);
   }
}
