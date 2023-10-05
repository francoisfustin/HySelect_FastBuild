/*    PortTool v2.2     FPTIME.C          */

#include "fptools.h"
#include <tchar.h>

#ifdef WIN32
//WIN32
FPTIME FPLIB fpGetTime ()
{
   FPTIME fpTime;
   SYSTEMTIME systime;
   
   GetLocalTime( &systime);
   
   fpTime = fpTIME(systime.wHour, systime.wMinute, systime.wSecond, 0);

   return fpTime;
}

#else  //not WIN32
FPTIME FPLIB fpGetTime ()
{
   extern void FAR PASCAL DOS3Call();
   FPTIME fpTime;
   short  hr, mn, sc;
   LPVOID lpHour = &hr, lpMinute = &mn, lpSecond = &sc;
   __asm
   {
   mov   ah, 2ch
   call  DOS3Call
   les   bx, lpHour
   xor   ax, ax
   mov   al, ch
   mov   es: WORD PTR[bx], ax

   les   bx, lpMinute
   mov   al, cl
   mov   es: WORD PTR[bx], ax

   les   bx, lpSecond
   mov   al, dh
   mov   es: WORD PTR[bx], ax
   }
   fpTime = fpTIME(hr, mn, sc, 0);

   return fpTime;
}
#endif //not WIN32

// Convert HHHH:MM:SS:NN string to fpTIME
// returns fpNOTIME if the value is out-of-range or
// the string cannot be parsed
//
static FPTIME fpHHMMSSToTime (LPTSTR lpsz)
{
   FPTIME time = 0, sum;
   int n = 4;
   
   if (!lpsz)			//km  sel8398
	   return fpNOTIME;

   if (NotNum (*lpsz))
      return fpNOTIME;

   while (*lpsz && n--)
      {
      sum = 0;
      while (*lpsz && IsNum (*lpsz) && sum < 1000)
         {
         sum = sum*10 + (*lpsz-(TCHAR)'0');
         lpsz = CharNext(lpsz);
         }
      switch (n)
         {
         case 3: if (sum > fpMAXHR) return fpNOTIME;
                 sum = fpHRTOMS(sum); break;
         case 2: if (sum > fpMAXMN) return fpNOTIME;
                 sum = fpMNTOMS(sum); break;
         case 1: if (sum > fpMAXSC) return fpNOTIME;
                 sum = fpSCTOMS(sum); break;
         case 0: if (sum > fpMAXHS) return fpNOTIME;
                 sum = fpHSTOMS(sum);
                 break;
         }
      if (fpMAXTIME - time < sum)
         return fpNOTIME;
      if (n) while (*lpsz && NotNum (*lpsz))
         lpsz = CharNext (lpsz);
      time += sum;
      }
   return (*lpsz == 0 ? time : fpNOTIME);
}

FPTIME FPLIB fpStringToTime (LPTSTR lpsz, UINT fScale)
{
   FPTIME ones, tens, time = 0;
   int h, m, s;
   double sum = 0.0;
   
   if (fScale == TSC_INTERNAL)
      {
      while (*lpsz && IsNum (*lpsz))
         {
         time = time*10 + (*lpsz-(TCHAR)'0');
         lpsz = CharNext (lpsz);
         }
      h = (int)(time/10000L);
      if (h > fpMAXHR) return fpNOTIME;
      time -= (h*10000L);
      m = (int)(time/100L);
      if (m > fpMAXMN) return fpNOTIME;
      time -= (m*100L);
      s = (int)time;
      if (s > fpMAXSC) return fpNOTIME;
      time = fpTIME(h,m,s,0);
      }
   else if (fScale == TSC_HHMMSS)
      time = fpHHMMSSToTime (lpsz);
   else
      {
      ones = 0;
      while (*lpsz && IsNum (*lpsz) && sum < fpMAXTIME)
         {
         ones = ones*10 + (*lpsz-(TCHAR)'0');
         lpsz = CharNext(lpsz);
         }
      sum = ones;
      if (*lpsz == (TCHAR)'.')
         {
         lpsz = CharNext(lpsz);
         ones = 0; tens = 1;
         while (*lpsz && IsNum (*lpsz) && tens < 100000000)
            {
            ones = ones*10 + (*lpsz-(TCHAR)'0');
            lpsz = CharNext (lpsz);
            tens = tens*10;
            }
         if (tens)
            sum += (double)ones / (double)(tens);
         }   
      
      if (*lpsz && tens < 100000000) return fpNOTIME;
      
      switch (fScale)
         {
         case TSC_HOURS:   
            time = (FPTIME)(sum > (double)fpHR(fpMAXTIME) ? fpNOTIME : fpHRTOMS(sum));
            break;
         case TSC_MINUTES:
            time = (FPTIME)(sum > (double)fpMN(fpMAXTIME) ? fpNOTIME : fpMNTOMS(sum));
            break;
         case TSC_SECONDS:
            time = (FPTIME)(sum > (double)fpSC(fpMAXTIME) ? fpNOTIME : fpSCTOMS(sum));
            break;
         case TSC_MSECS:
            time = (FPTIME)(sum > (double)fpMS(fpMAXTIME) ? fpNOTIME : sum);
            break;
         }
      }
   
   return time;
}   

//A1053...
void FPLIB fpFormatTimeParts(UINT fScale, LPTSTR lpsz, int hr, int mn, int sc, int hs)
{                                   
      if (fScale == TSC_INTERNAL)
        wsprintf (lpsz, _T("%02d%02d%02d"), hr, mn, sc);
      else
        wsprintf (lpsz, _T("%02d:%02d:%02d.%02d"), hr, mn, sc, hs);

}         
//A1053

LPTSTR FPLIB fpTimeToString (FPTIME fpTime, UINT fScale, LPTSTR lpsz)
{
  double value;
  
   if ((fScale == TSC_HHMMSS) || (fScale == TSC_INTERNAL))   //A1053
      {
      int hr, mn, sc, hs;

      hr = (int)(fpTime / 3600000L);                         //A1053     
      fpTime -= fpHRTOMS((long)hr);
      mn = (int)fpMN(fpTime);
      fpTime -= fpMNTOMS((long)mn);
      sc = (int)fpSC(fpTime);
      fpTime -= fpSCTOMS((long)sc);
      hs = (int)fpHS(fpTime);

      fpFormatTimeParts(fScale, lpsz, hr, mn, sc, hs);       //A1053

      }
   else 
      {
      switch (fScale)
         {
         case TSC_HOURS:   
            value = fpHR ((double)fpTime);
            break;
         case TSC_MINUTES:
            value = fpMN ((double)fpTime);
            break;
         case TSC_SECONDS:
            value = fpSC ((double)fpTime);
            break;
         case TSC_MSECS:
            value = fpMS ((double)fpTime);
            break;
         }
      fpDoubleToString (lpsz, value, 15, (TCHAR)'.', 0, FALSE, FALSE,
         TRUE, FALSE, 0, NULL);
      }
   return lpsz;
}

BOOL FPLIB fpTimeInRange (FPTIME fpTime, FPTIME fpMin, FPTIME fpMax)
{
   if (fpMin && fpTime < fpMin)
      return FALSE;
   else if (fpMax && fpTime > fpMax)
      return FALSE;
   return TRUE;
}
