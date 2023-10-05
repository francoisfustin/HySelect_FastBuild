/*    PortTool v2.2     FPTOOL12.C          */

#include "fptools.h"
#include "ctype.h"
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

/*
 *    atoi for far strings
 */
int FPLIB fpStringToInt(LPTSTR lpStr)
   {
   int  iVal;
   BOOL fNeg;

   while (*lpStr == (TCHAR)' ')
      lpStr++;

   lpStr += (fNeg = *lpStr == (TCHAR)'-');

   iVal = 0;
   while(_istdigit (*lpStr))
      {
      if (iVal < FP_INT_MAX / 10)
         iVal = iVal*10 + (int)(*lpStr++ - (TCHAR)'0');
      else if (iVal == FP_INT_MAX / 10)
         {
         iVal = iVal*10;
         if (iVal <= (long)FP_INT_MAX - (long)(*lpStr - (TCHAR)'0'))
            iVal = iVal + (int)(*lpStr++ - (TCHAR)'0');
         else // overflow detected
            return FP_INT_OVERFLOW;
         }
      else // overflow detected
         return FP_INT_OVERFLOW;
      }
   return fNeg ? -iVal : iVal;
}


/*
 *    atol for far strings
 */
long FPLIB fpStringToLong(LPTSTR lpStr, int base)
   {
   long lCurr;
   long val = 0;
   BOOL bIsNeg = FALSE;

//   int len = lstrlen(lpStr);

   if (*lpStr == '-')
      {
      bIsNeg = TRUE;
      *lpStr++; 
      }
   
   while (_istalnum(*lpStr))
      {
      if (_istalpha(*lpStr))
         {
         lCurr = (long)toascii(toupper(*lpStr)) - 55;
         if (lCurr > base - 1)
            return val; 
         }
      else
         {
         lCurr = (long)*lpStr - 48;          
         }
    
//      val += (long)(l * pow(base,len-1));   // this was replaced because of 
                                              // floating point problem - CTF
      if (val == 0)
         val = lCurr;
      else
         val = lCurr + (val * (long)base);

      *lpStr++; 
//      len--;
      }
   
   return (bIsNeg ? (-1L * val) : val);

   }

/*
 *		strrev for far strings
 */
LPTSTR	FPLIB fpStrRev(LPTSTR lpS1)
{
   LPTSTR	lpS = lpS1;
   LPTSTR   lpS2 = lpS1 + lstrlen(lpS1);
   TCHAR    c;

   while (lpS1 < lpS2)
   {
      c = *lpS1;
      *lpS1++ = *--lpS2;
      *lpS2 = c;
   }

	return lpS;
}
/*
 *    itoa for far strings
 */
LPTSTR   FPLIB fpIntToString(int nVal, LPTSTR lpStr)
{
   LPTSTR lpS = lpStr;
   BOOL   bNeg;

   if (bNeg = nVal < 0)
      nVal = -nVal;
   while (nVal || lpS == lpStr)
   {
      *lpStr++ = (TCHAR)(nVal % 10 + (int)'0');
      nVal /= 10;
   }
   if (bNeg)
      *lpStr++ = '-';
   *lpStr = '\0';

   return fpStrRev(lpS);
}

/*
 *    ltoa for far strings
 */
LPTSTR FPLIB fpLongToString(long lVal, LPTSTR lpStr)
{
   LPTSTR lpS = lpStr;
   BOOL   bNeg;

   if (bNeg = lVal < 0)
      lVal = -lVal;
   while (lVal || lpS == lpStr)
   {
      *lpStr++ = (TCHAR)(lVal % 10 + (long)'0');
      lVal /= 10;
   }
   if (bNeg)
      *lpStr++ = (TCHAR)'-';
   *lpStr = (TCHAR)'\0';

   return fpStrRev(lpS);
}

