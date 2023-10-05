/*
$Revision:   1.5  $
*/

/***
*atof.c - convert char string to floating point number
*
*Purpose:
*   Converts a character string into a floating point number.
*
*******************************************************************************/

#include <windows.h>
#include <toolbox.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include "output.h"

extern int DLLENTRY StringReplaceCh(char *szStr, register WORD ch);

/***
*void DLLENTRY StringToFloat(LPCTSTR nptr, LPDOUBLE lpdfVal) - convert string to floating point number
*
*Purpose:
*   atof recognizes an optional string of tabs and spaces, then
*   an optional sign, then a string of digits optionally
*   containing a decimal point, then an optional e or E followed
*   by an optionally signed integer, and converts all this to
*   to a floating point number.  The first unrecognized
*   character ends the string.
*
*Entry:
*   nptr - pointer to string to convert
*
*Exit:
*   returns floating point value of character representation
*
*Exceptions:
*
*******************************************************************************/

/*
void DLLENTRY StringToFloat(LPCTSTR nptr, LPDOUBLE lpdfVal)
{
struct lconv *plconv;
char cvtb[100];

#if defined(_UNICODE)
_fmemset(cvtb, '\0', CVTBUFSIZE);
wcstombs((char*)cvtb, nptr, min(StrLen(nptr),100));
#else
lstrcpyn(cvtb, nptr, 99);
#endif

plconv = localeconv();

if (plconv->decimal_point[0] != 0 && plconv->decimal_point[0] != '.')
   StringReplaceCh(cvtb, MAKEWORD(plconv->decimal_point[0], '.'));

cvtb[99] = '\0';
*lpdfVal = atof((char*)cvtb);
}
*/
void DLLENTRY StringToFloat(LPCTSTR nptr, LPDOUBLE lpdfVal)
{
struct lconv *plconv;
char cvtb[CVTBUFSIZE];

#if defined(_UNICODE)
_fmemset(cvtb, '\0', CVTBUFSIZE);
wcstombs((char*)cvtb, nptr, min(StrLen(nptr),CVTBUFSIZE));
#else
lstrcpyn(cvtb, nptr, CVTBUFSIZE - 1);
#endif

plconv = localeconv();

if (plconv->decimal_point[0] != 0 && plconv->decimal_point[0] != '.')
   StringReplaceCh(cvtb, MAKEWORD(plconv->decimal_point[0], '.'));

cvtb[CVTBUFSIZE - 1] = '\0';
*lpdfVal = atof((char*)cvtb);
}


int DLLENTRY StringReplaceCh(char *szStr, register WORD ch)
{
   char *szPtr = szStr;

   while (*szStr)
   {
      if (*szStr == HIBYTE(ch))
         *szStr = LOBYTE(ch);
      ++szStr;
   }
   return((int)(szStr - szPtr));
}
