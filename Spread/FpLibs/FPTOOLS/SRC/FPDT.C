/*    PortTool v2.2     FPDT.C          */

#include "fptools.h"
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>

#if (defined(FP_OCX) || defined(FP_VB))

BOOL FPLIB DoubleToFPdatetime (long lObject, double dfVal, LPFPDATE lpfpDate, LPFPTIME lpfpTime)
{
   // The double consists of 2 parts, "days.secs", 
   //   days:  number of since 1/1/1900.  
   //   secs:  number of secs into day.
   //
   
   TCHAR   sz[9];
   FPDATE  fpDate;
   FPTIME  fpTime;

   *sz = (TCHAR)'\0';
   if ((long)dfVal != 0)
      {         
      // if this is used for OCX, the first parameter must be (LONG) cast of OCX object pointer
      fpVBFormat((LPFPCONTROL)lObject, VT_R8, &dfVal, _T("yyyymmdd"), sz, STRING_SIZE(sz));
      fpDate = fpStringToDate(sz, DATEF_YYYYMMDD);
      // if this is used for OCX, the first parameter must be (LONG) cast of OCX object pointer
      fpVBFormat((LPFPCONTROL)lObject, VT_R8, &dfVal, _T("hhmmss"), sz, STRING_SIZE(sz));
      fpTime = fpStringToTime(sz, TSC_INTERNAL); 
      }
   else
      {
      fpDate = fpNODATE;
      fpTime = fpNOTIME;
      }
      
   *lpfpDate = fpDate;
   *lpfpTime = fpTime;
   
   return TRUE;
}

BOOL FPLIB DBdatetimeToFPdatetime (long lObject, LPDATAACCESS lpda, 
  LPFPDATE lpfpDate, LPFPTIME lpfpTime)
{
   VARIANT Variant;
   TCHAR   sz[9];
   VALUE   Value;
   double  dfVal;
   FPDATE  fpDate;
   FPTIME  fpTime;
   ERR     rc;
   FPCONTROL fpTemp = {0, 0, lObject, 0L};

   Value.hlstr = (HLSTR)lpda->lData;

   dfVal = 0.0;
   _fmemset(&Variant, '\0', sizeof(VARIANT));
   fpVBSetVariantValue(&Variant, VT_STRING, &Value);
   rc = fpVBCoerceVariant(&Variant, VT_DATE, &dfVal);

   *sz = (TCHAR)'\0';

   if (rc == (ERR)0) //  ((long)dfVal != 0)
      {
      // if this is used for OCX, the first parameter must be (LONG) cast of OCX object pointer
      fpVBFormat(&fpTemp, VT_R8, &dfVal, _T("yyyymmdd"), sz, STRING_SIZE(sz));
      fpDate = fpStringToDate(sz, DATEF_YYYYMMDD);
      // if this is used for OCX, the first parameter must be (LONG) cast of OCX object pointer
      fpVBFormat(&fpTemp, VT_R8, &dfVal, _T("hhmmss"), sz, STRING_SIZE(sz));
      fpTime = fpStringToTime(sz, TSC_INTERNAL); 
      }
   else
      {
      fpDate = fpNODATE;
      fpTime = fpNOTIME;
      }
      
   *lpfpDate = fpDate;
   *lpfpTime = fpTime;
   
   return TRUE;
}
#endif  //not FP_DLL

void FPLIB FPdatetimeToDouble (FPDATE fpDate, FPTIME fpTime, double FAR *dfVal)
{
   long    lJulian;

   //
   // VB date is the number of days since 01/01/1990
   // Convert fpDate to VB date
   //
   //                                              julian(12/31/1899)+1
   lJulian = (fpDate == fpNODATE ? 0L : fpDateToJulian(fpDate) - 693593); 
   if (lJulian >= 0)              //km  bug scs6678 
      *dfVal = (double)lJulian +
            (double)(fpTime == fpNOTIME ? 0L : ((double)fpTime / 86400000L));
   else
      *dfVal = -((double)labs(lJulian) + 
            (double)(fpTime == fpNOTIME ? 0L : ((double)fpTime / 86400000L)));

   return;
}

#if (defined(FP_VB) || defined(FP_OCX))
//SCP 5/20/96:  This function is only valid for VBX, for the
//              OCX, is it implemented (and renamed) in a source file
//              specific to each OCX.
void FPLIB FPdatetimeToDBdatetime (long lObject, LPDATAACCESS lpda, FPDATE fpDate, FPTIME fpTime)
{
#ifdef FP_OCX
  return;
#else  //FP_VB
   char    szBuffer[65];
   double  dfVal = 0.0;
   long    lJulian;
   VARIANT Variant;
   HSZ     hszText;
   VALUE   Value;
   FPCONTROL fpTemp = {0, lpda->hctlData, 0L, 0L};

   //
   // Retrieve the old value to preserve old date and time if needed
   // (VB cannot return a variant, so the string)
   //
   lpda->usDataType = DT_HLSTR;
   lpda->lData = 0L;
   lpda->sAction = DATA_FIELDVALUE;
   if (fpVBSendControlMsg((LPFPCONTROL)&fpTemp, VBM_DATA_GET, 0, (LONG)lpda))
      return;

   //
   // Convert the string to a variant and get
   // the double value (the date is stored in
   // the integer part, the time in the fractional
   // part - double precision required)
   //
   if (!(lpda->fs & DA_fNull))
   {
     Value.hlstr = (HLSTR)lpda->lData;
     _fmemset(&Variant, '\0', sizeof(VARIANT));
     fpVBSetVariantValue(&Variant, VT_STRING, &Value);
     fpVBCoerceVariant(&Variant, VT_DATE, &dfVal);
   }
   
   // Free the HLSTR we got from fpVBSendControlMsg(GET).  
   // NOTE: Even though the GET may have returned "DA_fNull", it may have
   // allocated a HLSTR!
   if (lpda->lData)
     fpVBDestroyHlstr((HLSTR)lpda->lData);

   //
   // VB date is the number of days since 01/01/1990
   // Convert fpDate to VB date
   //
   //                                              julian(12/31/1899)+1
   lJulian = (fpDate == fpNODATE ? 0L : fpDateToJulian(fpDate) - 693593); 

   // 
   // subtract old date (preserving time data)
   // and add new date
   //
   if (fpTime == fpNOTIME)
      dfVal = dfVal - (double)(long)dfVal + (double)lJulian;
   else if (fpDate == fpNODATE)
      dfVal = (double)(long)dfVal + (double)fpTime / 86400000L; // ((double)fpSC(fpTime) / 100000);
   else
      dfVal = (double)lJulian + (double)fpTime / 86400000L;

   //
   // VB is picky about the date/time format it will accept
   //
   {
   char FAR *p1, FAR *p2, szFormat[65];
   // build format string
   _fstrcpy(szFormat, fpGetIntl_sShortDate());
   _fstrcat(szFormat, " hh");
   _fstrcat(szFormat, fpGetIntl_sTime()); 
   _fstrcat(szFormat, "nn");
   _fstrcat(szFormat, fpGetIntl_sTime()); 
   _fstrcat(szFormat, "ss");
   // force four digit year (yyyy)
   p1 = _fstrtok(szFormat,"yY");
   p2 = _fstrtok(NULL, "yY");
   _fstrcpy(szBuffer,(p2?p1:""));
   _fstrcat(szBuffer,"yyyy");
   _fstrcat(szBuffer,(p2?p2:p1));
   // build date string in the format VB wants
   _fstrcpy(szFormat,szBuffer);

   fpTemp.hCtl = (HCTL)lObject;
   fpVBFormat((LPFPCONTROL)&fpTemp, VT_R8, &dfVal, szFormat, szBuffer, sizeof(szBuffer));
   }
   hszText = fpVBCreateHsz((LPFPCONTROL)&fpTemp, szBuffer);
   
   lpda->lData = (long)hszText;
   lpda->usDataType = DT_HSZ;

   return;
#endif //if-else FP_OCX
}
 
void FPLIB FPdatetimeToDBtext (HCTL hCtl, LPDATAACCESS lpda, FPDATE fpDate, FPTIME fpTime, BOOL bSaveBoth)
{
   TCHAR   szBufDate[34];
   TCHAR   szBufTime[34];
   TCHAR   szBuffer[68];
   double  dfVal = 0.0;
   long    lJulian;
   VARIANT Variant;
   HSZ     hszText;
   VALUE   Value;
   FPDATE fpDate2 = fpNODATE;
   FPTIME fpTime2 =  fpNOTIME;                           
   FPCONTROL fpTemp = {0, lpda->hctlData, 0L, 0L};
                              
   //
   // Retrieve the old value to preserve old date and time if needed
   // (VB cannot return a variant, so the string)
   //
   lpda->usDataType = DT_HLSTR;
   lpda->lData = 0L;
   lpda->sAction = DATA_FIELDVALUE;
   if (fpVBSendControlMsg((LPFPCONTROL)&fpTemp, VBM_DATA_GET, 0, (LONG)lpda))
      return;
   //SCP 5/20/96 - Fix this for OCX!!!
   DBdatetimeToFPdatetime(0L, lpda, &fpDate2, &fpTime2);
   //
   // Convert the string to a variant and get
   // the double value (the date is stored in
   // the integer part, the time in the fractional
   // part - double precision required)
   //
   Value.hlstr = (HLSTR)lpda->lData;
   _fmemset(&Variant, '\0', sizeof(VARIANT));
   fpVBSetVariantValue(&Variant, VT_STRING, &Value);
   fpVBCoerceVariant(&Variant, VT_DATE, &dfVal);

   //
   // VB date is the number of days since 01/01/1990
   // Convert fpDate to VB date
   //
   //                                              julian(12/31/1899)+1
   lJulian = (fpDate == fpNODATE ? 0L : fpDateToJulian(fpDate) - 693593); 

   // 
   // subtract old date (preserving time data)
   // and add new date
   //
   if (fpTime == fpNOTIME)
      dfVal = dfVal - (double)(long)dfVal + (double)lJulian;
   else if (fpDate == fpNODATE)
      dfVal = (double)(long)dfVal + (double)fpTime / 86400000L; // ((double)fpSC(fpTime) / 100000);
   else
      dfVal = (double)lJulian + (double)fpTime / 86400000L;

   //
   // VB is picky about the date/time format it will accept
   //
   {
   TCHAR szFormat1[34]  = {(TCHAR)'\0'};
   TCHAR szFormat2[34]  = {(TCHAR)'\0'};
   // build format string
   // build date/time string in the format VB wants   
   lstrcpy(szFormat1, _T("m"));
   lstrcat(szFormat1, fpGetIntl_sDate()); 
   lstrcat(szFormat1, _T("d"));
   lstrcat(szFormat1, fpGetIntl_sDate());
   lstrcat(szFormat1, _T("yyyy"));
 
   lstrcpy(szFormat2, _T("hh"));
   lstrcat(szFormat2, fpGetIntl_sTime()); 
   lstrcat(szFormat2, _T("nn"));
   lstrcat(szFormat2, fpGetIntl_sTime()); 
   lstrcat(szFormat2, _T("ss"));

   fpTemp.hCtl = hCtl;
   fpVBFormat((LPFPCONTROL)&fpTemp, VT_R8, &dfVal, szFormat1, szBufDate, sizeof(szBufDate));   

   fpVBFormat((LPFPCONTROL)&fpTemp, VT_R8, &dfVal, szFormat2, szBufTime, sizeof(szBufTime));   
   if (!bSaveBoth)                        // save both date and time
     {  
       if (fpDate != fpNODATE)
       {                              
          lstrcpy(szBuffer, szBufDate);
          if (fpTime2 != fpNOTIME)     // db field had a date portion to be rewritten untouched
          {
             lstrcat(szBuffer, _T(" "));
             lstrcat(szBuffer, szBufTime);
          }   
             
       }
       else if (fpTime != fpNOTIME)
       {  
          if (fpDate2 != fpNODATE)
          {
             lstrcpy(szBuffer, szBufDate);
             lstrcat(szBuffer, _T(" "));
             lstrcat(szBuffer, szBufTime);
          }
          else             
             lstrcpy(szBuffer, szBufTime);
       }  
     }
     else
     {
        lstrcpy(szBuffer, szBufDate);
        lstrcat(szBuffer, _T(" "));
        lstrcat(szBuffer, szBufTime);
     }

   }
   hszText = fpVBCreateHsz((LPFPCONTROL)&fpTemp, szBuffer);
   
   lpda->lData = (long)hszText;
   lpda->usDataType = DT_HSZ;

   return;
}
#endif  //not FP_DLL


void FPLIB FPdatetimeToString (FPDATE fpDate, FPTIME fpTime,
                                  LPTSTR buffer, WORD bufferSize)
{
   long    days;
   long    secs;
   TCHAR   tmpstr[22];
   //
   // VB date is the number of days since 01/01/1900
   // Convert fpDate to string
   //
   //                                              julian(12/31/1899)+1
   days = (fpDate == fpNODATE ? 0L : fpDateToJulian(fpDate) - 693593L); 
   secs = (fpTime == fpNOTIME ? 0L : ((long)fpTime / 1000L));
   wsprintf( tmpstr, _T("%ld.%ld"), days, secs);

   if (buffer)
     lstrcpyn( buffer, tmpstr, (int)bufferSize);
     
   return;
}

void FPLIB StringToFPdatetime (LPTSTR lpsz, LPFPDATE lpfpDate, 
                                  LPFPTIME lpfpTime)
{
   long    days;
   long    secs;
   int i;
   long    sign = 1L;
   TCHAR    *stopstring;
   double  fsecs = 0.0;

   //
   // VB date is the number of days since 01/01/1900
   // Convert fpDate to string
   //
   //                                              julian(12/31/1899)+1
   // Parse date (days) from string
   days = 0L;
   i = 0;
   // skip blanks
   while (lpsz[i] == ' ')
     ++i;
     
   // check for minus sign
   if (lpsz[i] == '-')
     {
     sign = -1;
     ++i;
     }
     
   while ((lpsz[i] >= '0') && (lpsz[i] <= '9'))
     {
     days = (days * 10L) + (long)(lpsz[i] - '0');
     ++i;
     }
     
   // include sign
   days *= sign;
     
   // Parse time (seconds) from string
   secs = 0;  
   if (lpsz[i] == '.')
     {
     /*++i;
     while ((lpsz[i] >= '0') && (lpsz[i] <= '9'))
       {
       secs = (secs * 10L) + (long)(lpsz[i] - '0');
       ++i;
       }*/
	 fsecs = _tcstod(&lpsz[i], &stopstring);
     }  
     
   // NOTE: Days are since 12/30/1899 (equal to 0), 1/1/1900 is 2 (see VB).
   *lpfpDate = fpJulianToDate (days - 2, 1900);
   
   // fpTime is number of milliseconds
   //*lpfpTime = (FPTIME)(secs * 1000L);

   //*lpfpTime = (FPTIME)secs; 
   *lpfpTime = (FPTIME)(fsecs*86400000L);
   return;
}

