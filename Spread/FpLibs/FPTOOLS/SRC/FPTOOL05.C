/*    PortTool v2.2     FPTOOL05.C          */

#include "fptools.h"

#include "string.h"
#include "stdlib.h"
#include "float.h" 
#include "ctype.h"
#include "math.h" 
#include "stdio.h"
#include <tchar.h>

LPTSTR fp_fcvt(double dfValue, int dPlaces, LPINT lpiPos, LPINT lpiSign);

// get prefix for currency string, based on intl currency codes                                                                     
static TCHAR* BeginCurrency (int iSign, LPTSTR sCurrency, short iCurr, int iNegCurr, LPTSTR lpszNegSymbol)
{
   static TCHAR currBuf[16];
   
   *currBuf = (TCHAR)0;
   if (iSign)
      {        
      switch (iNegCurr)
         {
         case 0:  lstrcat(currBuf, _T("("));
                  lstrcat(currBuf, sCurrency);
                  break;
                  
         case 1:  lstrcat(currBuf, lpszNegSymbol);
         case 3:  lstrcat(currBuf, sCurrency);
                  break;

         case 2:  lstrcat(currBuf, sCurrency);
         case 5:
         case 8:  lstrcat(currBuf, lpszNegSymbol);
                  break;
         
         case 4:  lstrcat(currBuf, _T("("));
                  break;
                  
         case 9:  lstrcat(currBuf, lpszNegSymbol);
                  lstrcat(currBuf, sCurrency);
                  lstrcat(currBuf, _T(" "));
                  break;
                  
         case 10:
         case 13:
                  break;
                  
         case 11: lstrcat(currBuf, sCurrency);
                  lstrcat(currBuf, _T(" "));
                  break;
                         
		   case 12: lstrcat(currBuf, sCurrency);
                  lstrcat(currBuf, _T(" "));
                  lstrcat(currBuf, lpszNegSymbol);
		 		      break;
		 
		   case 14: lstrcat(currBuf, _T("("));
                  lstrcat(currBuf, sCurrency);
                  lstrcat(currBuf, _T(" "));
                  break;
		 
		   case 15: lstrcat(currBuf, _T("("));
                  break;
		 		  
         default: break;
         
         }     
      }
   else
      {
      switch (iCurr)
         {
         case 0:
            lstrcpy(currBuf, sCurrency);
            break;
         case 2:  
            lstrcpy(currBuf, sCurrency);
            lstrcat(currBuf, _T(" "));
            break;
         default:
            lstrcpy(currBuf, _T(""));
         }
      }
      
   return currBuf;
}

// get suffix for currency string, based on intl currency codes                                                               
static TCHAR* EndCurrency (int iSign, LPTSTR sCurrency, short iCurr, int iNegCurr, LPTSTR lpszNegSymbol)
{
   static TCHAR currBuf[16];
   
   *currBuf = (TCHAR)0;
   if (iSign)
      {        
      switch (iNegCurr)
         {
         case 4:  lstrcat(currBuf, sCurrency);
         case 0:  lstrcat(currBuf, _T(")"));
                  break;
                  
         case 7:  lstrcat(currBuf, sCurrency);
         case 3:  
         case 11: lstrcat(currBuf, lpszNegSymbol);
                  break;
                  
         case 6:  lstrcat(currBuf, lpszNegSymbol);
         case 5:  lstrcat(currBuf, sCurrency);
                  break;
                  
         case 8:  lstrcat(currBuf, _T(" "));
                  lstrcat(currBuf, sCurrency);
                  break;
         
         case 10: lstrcat(currBuf, _T(" "));
         		   lstrcat(currBuf, sCurrency);
         		   lstrcat(currBuf, lpszNegSymbol);
                  break;
         
         case 13: lstrcat(currBuf, lpszNegSymbol);
                  lstrcat(currBuf, _T(" "));
         		   lstrcat(currBuf, sCurrency);
         		   break;
          
         case 14: lstrcat(currBuf, _T(")"));
         		   break;
         		
         case 15: lstrcat(currBuf, _T(" "));
         		   lstrcat(currBuf, sCurrency);
         		   lstrcat(currBuf, _T(")"));
         		   break;
         
         default: break;
         
         }     
      }
   else
      {
      switch (iCurr)
         {
         case 3:  lstrcpy(currBuf, _T(" "));
         case 1:  lstrcat(currBuf, sCurrency);
                  break;
                  
         default: lstrcpy(currBuf, _T(""));
         }
      } 
   return currBuf;
}                                                                     
                                                               
LPTSTR FPLIB fpDoubleToString (LPTSTR lpszString, double value, short dPlaces,
   TCHAR cDecimal, TCHAR cSeparator, BOOL fLeadingZero, BOOL fFixedPoint,
   BOOL fNegSign, BOOL fFormatted, short dCurrencyStyle, LPTSTR lpszCurrencySymbol)
{
	TCHAR szDecimal[2] = { cDecimal, (TCHAR)0 };
	TCHAR szSeparator[2] = { cSeparator, (TCHAR)0 };

	return fpIntlDoubleToString(lpszString, value, dPlaces, szDecimal, szSeparator, fLeadingZero,
		       fFixedPoint, (BYTE)fNegSign, fFormatted, (WORD)dCurrencyStyle, lpszCurrencySymbol, NULL,
             lpszCurrencySymbol != NULL);
}


LPTSTR FPLIB fpIntlDoubleToString (LPTSTR lpszString, double value, short dPlaces,
   LPTSTR lpszDecimal, LPTSTR lpszSeparator, BOOL fLeadingZero, BOOL fFixedPoint,
   BYTE bNegSign, BOOL fFormatted, WORD wCurrencyStyle, LPTSTR lpszCurrencySymbol, LPTSTR lpszNegSymbol,
   BOOL fIsCurrency)
{
   LPTSTR lpszDouble, lpszFirst = lpszString;
   TCHAR szBuffer[330];
   static int iSign;
   static int iPos;
	int iPastDecPos = 0;

	if (!lpszNegSymbol || !*lpszNegSymbol)
		lpszNegSymbol = _T("-");

   if (value == -HUGE_VAL)
      {
      int i=0;
      for (; i < dPlaces; i++)
         szBuffer[i] = (TCHAR)'0';
      szBuffer[i] = (TCHAR)0;
      lpszDouble = szBuffer;
      iSign = TRUE;
      iPos = 0;
      }
   else
   {
     // fp_fcvt doesn't round correctly, so we have to help it a little.
//     value += (1.0 / pow(10, max(dPlaces + 1, 5)));
     // lpszDouble = fp_fcvt (value, (int)dPlaces, &iPos, &iSign);
     lpszDouble = (LPTSTR)_fcvt (value, dPlaces, &iPos, &iSign);
     if (!lpszDouble || *lpszDouble == '\0')
        iSign = 0;

#ifdef _UNICODE
     // If unicode, the convert the ANSI chars to Wide chars.
     MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpszDouble, -1, szBuffer, STRING_SIZE(szBuffer));
     
     lpszDouble = szBuffer;  // let "lpszDouble" point to szBuffer
#endif
   }

   *lpszString = (TCHAR)0;
   //  copy prefix chars into buffer                                                  
   if (fIsCurrency)
      {
      lstrcpy (lpszString, BeginCurrency (iSign, lpszCurrencySymbol,
         (short)(wCurrencyStyle >> 8), (wCurrencyStyle & 0x00FF), lpszNegSymbol));
      }
   else if (iSign)
      {
      switch (bNegSign)               
         {
         case 0: lstrcat(lpszString, _T("(")); break;
         case 1: lstrcat(lpszString, lpszNegSymbol); break;
         case 2: lstrcat(lpszString, lpszNegSymbol);
         		  lstrcat(lpszString, _T(" ")); break;
         case 3: 
         case 4: break;
         }
      }

//GAB 12/21/01   lpszString = lpszString + strlen(lpszString);
   lpszString = lpszString + _tcslen(lpszString);
   
   // iPos contains the index value where the decimal point should go
   if (iPos <= 0)
      {
      if (fLeadingZero || (value == 0.0 && dPlaces == 0))
         lstrcat(lpszString, _T("0"));
      }
   else
      {  
      // Copy Integer portion with or without separators to buffer         
      if ((iPos > 3) && (lpszSeparator))
         {
         short iNumSeps = (iPos - 1) / 3;
         short i = iPos % 3, startPos; 

         if (!i) i = 3;                  
          _ftcsncat(lpszString, lpszDouble, i);
         startPos = i;
         for(i=1; i <= iNumSeps; i++)
            {
            lstrcat(lpszString, lpszSeparator);    
            _ftcsncat(lpszString, &lpszDouble[startPos], 3);
            startPos = startPos + 3;
            }
         }     
      else
         _ftcsncat(lpszString, lpszDouble, iPos);
      }
      
   // check to see if there is a fractional portion
   if (dPlaces > 0)
      {
		if (!lpszDecimal)
			lpszDecimal = _T(".");
      // Copy decimal point and fractional part of number   
      // RFW - 7/28/03 - 12113
		iPastDecPos = lstrlen(lpszString);
      lstrcat(lpszString, lpszDecimal);
      if (iPos < 0)
         {
         // slow, but will do for now... rdw
         int i = max(-dPlaces, iPos);
         while (i < 0)
            {
            lstrcat(lpszString, _T("0"));
            i++;
            }
         lstrcat(lpszString, lpszDouble); 
         }
      else
         lstrcat(lpszString, &lpszDouble[iPos]);
      }
         
   // remove trailing zeros
   if (fFormatted)
      lpszString[lstrlen(lpszString)-1] = (TCHAR)0;
   else if (!fFixedPoint)
      {
      short dI = 0, dLast = 0;

		if (iPastDecPos)
			{
	      dLast = iPastDecPos;
			dI = dLast;

	      while (lpszString[dI])
		      {
			   if (lpszString[dI] != (TCHAR)'0')
				   dLast = dI;
				dI++;   
				}

	      // RFW - 7/28/03 - 12113
			if (lpszString[dLast] && _tcsncmp(&lpszString[dLast], lpszDecimal, lstrlen(lpszDecimal)) != 0)
//			if (lpszString[dLast] && strncmp(&lpszString[dLast], lpszDecimal, lstrlen(lpszDecimal)) != 0)
				lpszString[dLast+1] = (TCHAR)0;
			else
				lpszString[dLast] = (TCHAR)0;   
			}

      else if (value == 0.0)
         {
         lpszString[0] = (TCHAR)'0';
         lpszString[1] = (TCHAR)0;
         }
      }   

   // copy postfix characters info buffer
   if (fIsCurrency)
      {
      lstrcat (lpszString, EndCurrency (iSign, lpszCurrencySymbol,
         (short)(wCurrencyStyle >> 8), (wCurrencyStyle & 0x00FF), lpszNegSymbol));
     }
   else if (iSign)
      {
      switch (bNegSign)               
         {
         case 0: lstrcat(lpszString, _T(")")); break;
         case 1: 
         case 2: break;
         case 3: lstrcat(lpszString, lpszNegSymbol); break;
         case 4: lstrcat(lpszString, _T(" "));
                 lstrcat(lpszString, lpszNegSymbol); break;
         }
      }
   return lpszFirst;                               
}


LPTSTR fp_fcvt(double dfValue, int dPlaces, LPINT lpiPos, LPINT lpiSign)
{
LPSTR         lpszDouble;
double        y;
static double n;
static int    iSign;
static int    iPos;
static TCHAR  szTemp[60];
static int    iNoSign;

y = modf(dfValue, &n);

// Do the conversion in chars (bytes) then convert string to WCHARS (WORDS).
lpszDouble = _fcvt(n, 0, &iPos, &iSign);
*lpiPos = iPos;
_fstrcpy((LPSTR)szTemp, lpszDouble);
lpszDouble = _fcvt(y, dPlaces, &iPos, &iNoSign);
*lpiSign = iSign;
_fstrcat((LPSTR)szTemp, lpszDouble);

#ifdef _UNICODE
  // If unicode, convert buffer to WCHARs
  {
    TCHAR szBuffer[60];
    int  rc = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTemp, -1, 
                                  szBuffer, STRING_SIZE(szBuffer));
    // Copy convert string to "szTemp".
    lstrcpy(szTemp, szBuffer);
  }
#endif

return (szTemp);
}
