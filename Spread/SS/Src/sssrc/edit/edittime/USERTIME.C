/*
$Revision:   1.1  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITTIME/USERTIME.C_V  $
 *
 *    Rev 1.1   12 Jun 1990 14:09:02   Dirk
 * No change.
 *
 *    Rev 1.0   04 Jun 1990 14:53:46   Bruce
 * Initial revision.
 *
 *    Rev 1.14   24 May 1990 16:13:28   Steve
 * fixed bug where 12hour+"pm" at nHour=12 went to 24, not 12 as it should
 *
 *    Rev 1.13   24 May 1990 12:01:12   Randall
 * Added TimeGetFormat
 *
 *    Rev 1.12   24 May 1990 10:09:34   Steve
 * TimeHMSToString now fixed so TIME struct not modified, only temp variable
 *
 *    Rev 1.11   23 May 1990 15:44:40   Randall
 *
 *    Rev 1.10   23 May 1990 15:08:34   Steve
 * fixed TineHMSToString to set 0 -->12am if 12 hour time
 *
 *    Rev 1.9   23 May 1990 12:12:42   Randall
 *
 *    Rev 1.8   23 May 1990  9:53:44   Steve
 * changed Time functions to use LPTIME structure rather than h, m, s integers
 *
 *    Rev 1.7   22 May 1990 15:46:02   Randall
 * Added TimeSetFormat
 *
 *    Rev 1.6   22 May 1990 12:45:12   Randall
 * No change.
 *
 *    Rev 1.5   21 May 1990 16:23:42   Randall
 * Added TimeSetRange
 *
 *    Rev 1.4   21 May 1990 13:29:26   Randall
 *
 *    Rev 1.3   17 May 1990 16:54:06   Steve
 * TimeStringIsValid can accept upper and lowercase am or pm in input
 *
 *    Rev 1.2   17 May 1990 12:40:38   Steve
 * stricter validation of input string for DateDMYToString()
 *
 *    Rev 1.1   16 May 1990 15:38:04   Steve
 * redone DateDMYToString().... added LOTS of Debugs
 *
 *    Rev 1.0   11 May 1990 16:12:42   Bruce
 * Initial revision.
 *
 *    Rev 1.1   10 May 1990 13:29:30   Steve
 *
 *    Rev 1.0   10 May 1990  8:56:16   Randall
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT          // to include our struct defns and func defns

#include <ctype.h>         // Needed for isdigit() checking below
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <toolbox.h>
#include <fptools.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "edittime.h"
//#ifdef SPREAD_JPN
#include "..\..\..\vbx\stringrc.h"          //- Added by Charles Feng at FarPoint (SPREAD_JPN)
//#endif
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "edittime\edittime.h"
<-BORLAND*/
#endif

extern TCHAR szHourGlobal[3];
extern TCHAR szMinuteGlobal[3];
extern TCHAR szSecondGlobal[3];
extern TCHAR szAmGlobal[5];
extern TCHAR szPmGlobal[5];

BOOL IsDbcsTimeSeparator(TCHAR cSeparator);


void TimeGetDefFormat(LPTIMEFORMAT lpTimeFormat);

///////////////////////////////////////////////////////////////////////////////
// TimeStringIsValid  - check time string for valid ranges     Steve M.
// NOTE: some double-checking is done, ie. the string is tested and so are
// flags in the TIMEINFO structure, only if they concur then return TRUE.
// NOTE: I really don't like this function.. it's real spaghetti code with
// a few logical loopholes. I should have written it in a totally different way!
///////////////////////////////////////////////////////////////////////////////
BOOL DLLENTRY TimeStringIsValid( LPCTSTR lpszTimeString, LPTIMEFORMAT timefmt)
{
   TCHAR szTimeUpper[ TIMELENGTH];  // The input string, but uppercased
   TCHAR szTmpStr[ TIMELENGTH];     // Temporary string
   int   xStart;                    // Checking location
   int   MaxLength;                 // Max Length.
   int   Separator;                 // Separator type
#ifdef _UNICODE
   int   LenTable[2][4] = { {  8, 5,  8,  8 },
                            { 11, 8, 11, 11 }  };
   int   nCharWidth = 1;
#else
   int   LenTable[2][4] = { {  8, 5, 10, 10 },
                            { 11, 8, 13, 13 }  };
   int   nCharWidth = 2;
#endif
   int   iTmpInt, iMaxHour;         // Temporary stuff

   // Copy the whole input time string to an uppercase string:
   lstrcpyn( szTimeUpper, lpszTimeString, TIMELENGTH);

   Separator = ( IsDbcsTimeSeparator(timefmt->cSeparator) ) ? 1 : 0;

   MaxLength = LenTable[Separator][timefmt->b24Hour];
   if (timefmt->bSeconds)
		{
      MaxLength += 3;      //- Calc string length.
#ifndef _UNICODE
		if (Separator)
			MaxLength++;
#endif
		}

   if (StrLen(szTimeUpper) > MaxLength)      //- Check the string length according
      return FALSE;                          //  to the selected hour time.

/* RFW - 7/23/08 - 22784
#ifdef	SPREAD_JPN
*/
   //- Sometimes, szAmGlobal and szPmGlobal are not initialized when 
   //  Spread kernal call into this sub routine. Fengwei
   //
   LoadString(hDynamicInst, IDS_TIME_AM + LANGUAGE_BASE, szAmGlobal, STRING_SIZE(szAmGlobal));
   LoadString(hDynamicInst, IDS_TIME_PM + LANGUAGE_BASE, szPmGlobal, STRING_SIZE(szPmGlobal));
   //-
//#endif

   if (timefmt->b24Hour != 1)
      {
      if ((StriStr(szTimeUpper, _T("am")  ) != NULL) ||
          (StriStr(szTimeUpper, _T("pm")  ) != NULL) ||
          (StrStr(szTimeUpper,(LPTSTR)szAmGlobal) != NULL) || 
          (StrStr(szTimeUpper,(LPTSTR)szPmGlobal) != NULL) )
         iMaxHour = 12;
      else
         return FALSE;
      }
   else
      iMaxHour = 23;
   
   //- Calculate the starting position. ( HH:**..... )
   xStart = (timefmt->b24Hour == 3) ? 1 + (nCharWidth * 2) : 0;

   //- Check the Hour section.
   szTmpStr[0] = *(szTimeUpper + xStart);
   szTmpStr[1] = *(szTimeUpper + xStart + 1);
   szTmpStr[2] = 0;
   iTmpInt = StringToInt(szTmpStr);
   iTmpInt = (iTmpInt >= ((timefmt->b24Hour >= 1) ? 0 : 1) && iTmpInt <= iMaxHour) ? 0 : 1;
   
   // 24hr HOUR range: 0-23  (0:Next  1: Return)
   if (iTmpInt)               
      // 12hr HOUR range: 1-12   
      return FALSE;                                                         

   //- Check for ':' or 'Žž'.
   if (szTimeUpper[xStart + 2] != 
       szHourGlobal[0] && (TUCHAR)szTimeUpper[xStart + 2] != timefmt->cSeparator )
      return FALSE;
#ifndef _UNICODE
   if (Separator && szTimeUpper[xStart + 3] != szHourGlobal[1])
      return FALSE;
#endif

   //- Next **:MM......
	xStart += 3;
#ifndef _UNICODE
	if (Separator)
		xStart++;
#endif

   //- Check the Minute section.
   szTmpStr[0] = *(szTimeUpper + xStart);
   szTmpStr[1] = *(szTimeUpper + xStart + 1);
   szTmpStr[2] = 0;
   iTmpInt = StringToInt(szTmpStr);
   iTmpInt = (iTmpInt >= 0 && iTmpInt <= 59) ? 0 : 1;
   
   // Check minutes range  (0:Next  1: Return)
   if (iTmpInt)               
      return FALSE;

   //- Check for '•ª' if separator is '?'.
   if (Separator && szTimeUpper[xStart + 2] != szMinuteGlobal[0])
      return FALSE;

#ifndef _UNICODE
	if (Separator && szTimeUpper[xStart + 3] != szMinuteGlobal[1])
      return FALSE;
#endif

   if (timefmt->bSeconds)
      {
      if (!Separator && (TUCHAR)szTimeUpper[xStart + 2] != timefmt->cSeparator)
         return FALSE;

      //- Next **:**:SS....
		xStart += 3;
#ifndef _UNICODE
		if (Separator)
			xStart++;
#endif

      //- Check the Minute section.
      szTmpStr[0] = *(szTimeUpper + xStart);
      szTmpStr[1] = *(szTimeUpper + xStart + 1);
      szTmpStr[2] = 0;
      iTmpInt = StringToInt(szTmpStr);
      iTmpInt = (iTmpInt >= 0 && iTmpInt <= 59) ? 0 : 1;
      
      // Check seconds range  (0:Next  1: Return)
      if (iTmpInt)            
         return FALSE;

      //- Check for '•b' if separator is '?'.
	   if (Separator && szTimeUpper[xStart + 2] != szSecondGlobal[0])
	      return FALSE;

#ifndef _UNICODE
		if (Separator && szTimeUpper[xStart + 3] != szSecondGlobal[1])
	      return FALSE;
#endif
      }

      //- Check the AM,PM,Œß‘O,ŒßŒã section.
      if(timefmt->b24Hour != 1)
      {
      
      //- Next **:**:** ....(space)
		xStart += 2;
#ifndef _UNICODE
		if (Separator)
			xStart += 2;
#else
		if (Separator)
			xStart++;
#endif

      //- Next byte should be a space. Adjusting need if 24Hour type is 3.
      if ((timefmt->b24Hour == 3 && szTimeUpper[nCharWidth*2]      != ' ') ||
          (timefmt->b24Hour != 3 && szTimeUpper[xStart] != ' '))
         return FALSE;

      //- Next **:**:** [a,p]m....
      xStart = (timefmt->b24Hour == 3) ? 0 : xStart + 1;

      if (timefmt->b24Hour == 0)
         {
         if ((tolower(szTimeUpper[xStart])   != 0x61 ||           //- a
              tolower(szTimeUpper[xStart])   != 0x70) &&          //- p
              tolower(szTimeUpper[xStart+1]) != 0x6d)             //- m
            return FALSE;
         }
      else
         {
#ifdef _UNICODE
         if (szTimeUpper[xStart + 0] != szAmGlobal[0] &&
             szTimeUpper[xStart + 0] != szPmGlobal[0])
            return FALSE;
         if (szTimeUpper[xStart + 1] != szAmGlobal[1] &&
             szTimeUpper[xStart + 1] != szPmGlobal[1])
            return FALSE;
#else
         if (szTimeUpper[xStart]     != szAmGlobal[0] &&          //- Œß
             szTimeUpper[xStart + 1] != szAmGlobal[1])
            return FALSE;
         if (szTimeUpper[xStart + 2] != szAmGlobal[2] &&          //- ‘O,Œã (1)
             szTimeUpper[xStart + 2] != szPmGlobal[2])
            return FALSE;
         if (szTimeUpper[xStart + 3] != szAmGlobal[3] &&          //- ‘O,Œã (2)
             szTimeUpper[xStart + 3] != szPmGlobal[3])
            return FALSE;
#endif
         }
      }

   return TRUE;   
}  //// End of function TimeStringIsValid ////


///////////////////////////////////////////////////////////////////////////////
// TimeStringToHMS  -- Convert validated string to integers according to the
// TIMEFORMAT structure.  This has been rewritten.            Steve M.  8/5/90
///////////////////////////////////////////////////////////////////////////////
LPTIME DLLENTRY TimeStringToHMS( LPCTSTR lpszTimeString, LPTIME ti, LPTIMEFORMAT timefmt)
{
   TCHAR szTmpStr[ TIMELENGTH];

   int   xStart = 0;
   int   nLength;

   // Length of current string.
   nLength = StrLen(lpszTimeString);

   // Starting Position for Hour.
#ifdef _UNICODE
   xStart = (timefmt->b24Hour == 3) ? 3 : 0;
#else
   xStart = (timefmt->b24Hour == 3) ? 5 : 0;
#endif
   ti->nHour = 0;
   ti->nMinute = 0;
   ti->nSecond = 0;

   // Convert the Hours in the string ("HH:..!??**") to integer:

   if (nLength >= xStart + 2)
      {
      szTmpStr[0] = *(lpszTimeString + xStart);
      szTmpStr[1] = *(lpszTimeString + xStart + 1);

      szTmpStr[ 2] = '\0';
      ti->nHour = StringToInt( szTmpStr);

      switch(timefmt->b24Hour){
         case 0: // 12:00 am
            if (ti->nHour == 12) ti->nHour = 0;
            if ((StriStr(lpszTimeString, _T("p") ) != NULL) ||  // it's pm now
/*RAP*/         (!StrCmp(lpszTimeString, szPmGlobal)))
                ti->nHour += 12;
            break;

         case 2:
         case 3:
#ifdef _UNICODE
            if (StriStr(lpszTimeString, szPmGlobal) != NULL && ti->nHour != 12)
#else
            if (StriStr(lpszTimeString, &szPmGlobal[2]) != NULL && ti->nHour != 12)
#endif
                ti->nHour += 12;
      }

      // Convert the Minutes in the string ("..:MM!??**") to integer:

      // Starting Position for Minute.
      xStart += 3;
#ifndef _UNICODE
      if (IsDbcsTimeSeparator(timefmt->cSeparator))
			xStart++;
#endif

      if (nLength >= xStart + 2)
         {
         szTmpStr[0] = *(lpszTimeString + xStart);
         szTmpStr[1] = *(lpszTimeString + xStart + 1);
         szTmpStr[ 2] = '\0';
         ti->nMinute = StringToInt( szTmpStr);

         // Structure says whether Seconds are included or not: ("..:..:SS**") :
         // Starting Position for Minute.
	      xStart += 3;
#ifndef _UNICODE
	      if (IsDbcsTimeSeparator(timefmt->cSeparator))
				xStart++;
#endif

         if ( timefmt->bSeconds && nLength >= xStart + 2)
         {
            szTmpStr[0] = *(lpszTimeString + xStart);
            szTmpStr[1] = *(lpszTimeString + xStart + 1);
            szTmpStr[2] = '\0';
            ti->nSecond = StringToInt( szTmpStr);
         }
         else
            ti->nSecond = 0;  // No Seconds in string, so zero it
         }
      }

   return ti;
}  //// End of function TimeStringToHMS ////


///////////////////////////////////////////////////////////////////////////////
// TimeHMSToString  -- Convert integers to a string according to the
// format supplied in the TIMEFORMAT structure and return it     Steve M.
///////////////////////////////////////////////////////////////////////////////
LPTSTR DLLENTRY TimeHMSToString( LPTIME ti, LPTSTR lpszTimeString, LPTIMEFORMAT timefmt)
{
   TCHAR szTmpStr[ TIMELENGTH];
   TCHAR szInternal[ TIMELENGTH];
   int   iTmpHour = 0;
   BOOL  fPOSTMERIDIAN = FALSE;

   // Convert the integer for Hours. Must use a copy of the structure's variable,
   // as it is modified:
   iTmpHour = ti->nHour;

   fPOSTMERIDIAN = FALSE;

   switch(timefmt->b24Hour){
      case 0:
         if (iTmpHour == 0)
             iTmpHour = 12;
         else if (iTmpHour >= 12){
             fPOSTMERIDIAN = TRUE;
             if (iTmpHour > 12) iTmpHour -= 12;
         }
         break;

      case 2:
      case 3:
         if (iTmpHour >= 12){
             fPOSTMERIDIAN = TRUE;
             if (iTmpHour > 12) iTmpHour -= 12;
         }
   }
 
   szInternal[0] = 0;               //- Does the string intializing here !

   if (timefmt->b24Hour == 3)
      {
      if (fPOSTMERIDIAN) 
         StrCpy( szInternal, (LPTSTR)szPmGlobal);

      else
         StrCpy( szInternal, (LPTSTR)szAmGlobal);
      
      StrCat( szInternal, _T(" "));        

      }

   IntToString( iTmpHour, szTmpStr);
   if ( iTmpHour < 10)
      StrCat( szInternal, _T("0"));
   StrCat( szInternal, szTmpStr);

   // Append the Separator:
   if ( IsDbcsTimeSeparator(timefmt->cSeparator) )       //- 255 embedded char.
      StrCpy( szTmpStr,(LPTSTR)szHourGlobal );
   else
      {
      szTmpStr[0] = timefmt->cSeparator;
      szTmpStr[1] = 0;
      }

   StrCat( szInternal, szTmpStr);

   // Convert the integer Minutes:
   IntToString( ti->nMinute, szTmpStr);
   if ( ti->nMinute < 10)
      StrCat( szInternal, _T("0"));
   StrCat( szInternal, szTmpStr);

   if ( IsDbcsTimeSeparator(timefmt->cSeparator) )       //- 255 embedded char.
      StrCat( szInternal,(LPTSTR)szMinuteGlobal );

   // Append the Separator, but only if Seconds are to follow, add Seconds:
   if ( timefmt->bSeconds)
   {
      if ( !IsDbcsTimeSeparator(timefmt->cSeparator) )
         {
         szTmpStr[0] = timefmt->cSeparator;
         szTmpStr[1] = 0;
         StrCat( szInternal, szTmpStr);
         }
      // Convert the integer Seconds:
      IntToString( ti->nSecond, szTmpStr);
      if ( ti->nSecond < 10)
         StrCat( szInternal, _T("0"));     // Pad with zero if < 10 Seconds
      StrCat( szInternal, szTmpStr);
      
      if ( IsDbcsTimeSeparator(timefmt->cSeparator) )       //- 255 embedded char.
         StrCat( szInternal, (LPTSTR)szSecondGlobal );
   }


   if ( timefmt->b24Hour == 0 || timefmt->b24Hour == 2 )
   {
      if ( fPOSTMERIDIAN)
      {
         if ( timefmt->b24Hour == 2 )
         {
            StrCat( szInternal, _T(" ") );
            StrCat( szInternal, (LPTSTR)szPmGlobal );
         }
         else
         {
// RAP            StrCat( szInternal, _T(" pm"));
//Modify by BOC 99.5.11 (hyt)--------------------------
//for szPmGlobal contain japanese char in jpn version
#ifdef SPREAD_JPN
			StrCat( szInternal, _T(" pm"));
#else
            TCHAR buffer[100];
            if (!lstrlen(szPmGlobal))
            {
              StrCpy( buffer, fpGetIntl_s2359());
              if (StriCmp(buffer, _T("pm")))
                StrCat( szInternal, _T(" pm"));
              else
                StrCpy( szPmGlobal, buffer);
            }
            StrCat( szInternal, _T(" ") );
            _ftcsncat( szInternal, szPmGlobal, 2);
#endif
         }
      }
      else 
      {
         if ( timefmt->b24Hour == 2 )
         {
            StrCat( szInternal, _T(" ") );
            StrCat( szInternal, (LPTSTR)szAmGlobal);
         }
         else
         {
// RAP           StrCat( szInternal, _T(" am"));
//Modify by BOC 99.5.11 (hyt)--------------------------
//for szPmGlobal contain japanese char in jpn version
#ifdef SPREAD_JPN
			StrCat( szInternal, _T(" am"));
#else
            TCHAR buffer[100];
            if (!lstrlen(szAmGlobal))
            {
              StrCpy( buffer, fpGetIntl_s1159());
              if (StriCmp(buffer, _T("am")))
                StrCat( szInternal, _T(" am"));
              else
                StrCpy( szAmGlobal, buffer);
            }
            StrCat( szInternal, _T(" ") );
            _ftcsncat( szInternal, szAmGlobal, 2);
#endif
//-------------------------------------------------------
         }
      }
   }

   // Ok, built up the internal string... copy it out and return it:
   StrCpy( lpszTimeString, szInternal);
   return lpszTimeString;
}  //// End of Function TimeHMSToString ////


//--------------------------------------------------------------------------//
BOOL DLLENTRY TimeSetRange( HWND hWnd, LPTIME tiMin, LPTIME tiMax)

{
   LONG  lMin, lMax;       // Seconds since midnight
   TCHAR szString[40];

   lMin = TimeHMSToSeconds ( tiMin);
   lMax = TimeHMSToSeconds ( tiMax);

   if (lMin > lMax) return FALSE;
#if 0
   GetClassName (hWnd, szString, 40);
   if (StrCmp (szString, TIME_CLASS_NAME) != 0) return FALSE;
#endif
   StrPrintf (szString, _T("%ld %ld"), lMin, lMax);

   return
      (BOOL) SendMessage (hWnd, EM_SETRANGE, 0, (LPARAM)(LPTSTR) szString);
}

//--------------------------------------------------------------------------//

LPTIME DLLENTRY TimeSecondsToHMS( LONG lSeconds, LPTIME ti)
{
   ti->nHour =   (int) (lSeconds / 3600L);
   ti->nMinute = (int) ((lSeconds % 3600L) / 60L);
   ti->nSecond = (int) (lSeconds % 60L);

   return ti;
}

//--------------------------------------------------------------------------//

BOOL DLLENTRY TimeSetFormat (HWND hWnd, LPTIMEFORMAT tf)

{
   LPARAM  lParam;

   lParam = (LPARAM) tf;

   SendMessage (hWnd, EM_SETFORMAT, 0, lParam);

   return TRUE;
}

//--------------------------------------------------------------------------//

BOOL DLLENTRY TimeGetFormat (HWND hWnd, LPTIMEFORMAT tf)
{
if (hWnd)
   SendMessage(hWnd, EM_GETFORMAT, 0, (LPARAM)tf);
else
   TimeGetDefFormat(tf);

return (TRUE);
}


void DLLENTRY TimeGetCurrentTime(LPTSTR lpszTime, LPTIMEFORMAT lptf)
{
TIME Time;

SysGetTime (&Time.nHour, &Time.nMinute, &Time.nSecond);
TimeHMSToString (&Time, lpszTime, lptf);
}


void TimeGetDefFormat(lpTimeFormat)

LPTIMEFORMAT lpTimeFormat;
{
TCHAR szTemp[2];

lpTimeFormat->b24Hour = (BOOL)GetProfileInt(_T("intl"), _T("iTime"), 0);
lpTimeFormat->bSeconds = FALSE;

GetProfileString(_T("intl"), _T("sTime"), _T(":"), szTemp, 2);
lpTimeFormat->cSeparator = szTemp[0];
lpTimeFormat->bSpin = 0;
}

//--------------------------------------------------------------------------//

LONG  DLLENTRY TimeHMSToSeconds ( LPTIME ti)
{
   return
      (LONG)ti->nHour * 3600L + (LONG)(ti->nMinute * 60) + (LONG)ti->nSecond;
}

//-----------------------------------------------------------------------------
// Check to see if the separator is DBCS or not. (Masanori Iwasa)
//-----------------------------------------------------------------------------
BOOL IsDbcsTimeSeparator(TCHAR cSeparator)
{
    int     nChar = (BYTE)szHourGlobal[0];

    //- Checks to see if the loaded resource is DBCS. If it's not
    //  that means the VBX/OCX is a English version. Character '?'
    //  should not be a reserved in the English version.
#ifndef _UNICODE // RFW - 5/24/05 - 16281
    if(!_TIsDBCSLeadByte((BYTE)nChar))
        return FALSE;
#endif
    
    //- If the separator is the defined keyword ('?'), that means
    //  the DBCS separator is being used.
    if(cSeparator == KANJISEPARATOR)
        return TRUE;

    return FALSE;
}

