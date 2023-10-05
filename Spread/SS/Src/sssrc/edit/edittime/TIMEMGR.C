/*
$Log:   G:/DRLIBS/TOOLBOX/EDIT/EDITTIME/TIMEMGR.C_V  $
 *
 *    Rev 1.18   10 Jul 1990 10:13:36   Sonny
 * Fixed a bug in TimeIsInRange (Forgetting to unlock if time range values
 * are not defined)
 *
 *    Rev 1.17   12 Jun 1990 14:08:50   Dirk
 * No change.
 *
 *    Rev 1.16   29 May 1990  9:19:20   Randall
 * Made external objects "static".
 *
 *    Rev 1.15   24 May 1990 17:34:22   Randall
 * Added LoadCurrentTime
 *
 *    Rev 1.14   24 May 1990 12:00:50   Randall
 * Added TimeGetFormat
 *
 *    Rev 1.13   23 May 1990 16:49:34   Sonny
 *
 *    Rev 1.12   23 May 1990 12:13:28   Randall
 *
 *    Rev 1.11   23 May 1990  9:18:10   Randall
 * No change.
 *
 *    Rev 1.10   22 May 1990 15:44:48   Randall
 * Added Support for TimeSetFormat
 *
 *    Rev 1.9   22 May 1990 12:44:14   Randall
 * Added Range setting and checking
 *
 *    Rev 1.8   21 May 1990 13:29:36   Randall
 *
 *    Rev 1.7   17 May 1990 18:05:58   Randall
 *
 *    Rev 1.6   16 May 1990 16:16:44   Randall
 * Fixed bug in InitializeTimeField: Unlocked too soon.
 *
 *    Rev 1.5   16 May 1990 16:12:58   Randall
 *
 *    Rev 1.4   15 May 1990 15:26:20   Randall
 *
 *    Rev 1.3   14 May 1990 17:41:00   Randall
 *
 *    Rev 1.2   14 May 1990 15:02:36   Randall
 * Notify parent if Alloc fails
 *
 *    Rev 1.1   14 May 1990 11:18:06   Randall
 * Changed some StrnCpy's to StrnnCpy's
 *
 *    Rev 1.0   11 May 1990 16:12:32   Bruce
 * Initial revision.
 *
 *    Rev 1.2   10 May 1990 10:39:30   Randall
 * Modified to use new "Time..." functions
 *
 *    Rev 1.1   30 Apr 1990 13:20:32   Bruce
 *
 *    Rev 1.0   23 Apr 1990 18:42:36   Sonny
 * Initial revision.
*/

//Modify by BOC 1999.4.12(hyt)------------------------------
//FORWARD_WM_CHAR not work correct
//so replace with PostMessage
//----------------------------------------------------------

#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\classes\wintools.h"
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "edittime.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\classes\wintools.h"
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "edittime\edittime.h"
<-BORLAND*/
#endif

extern TCHAR szHourGlobal[3];
extern TCHAR szMinuteGlobal[3];
extern TCHAR szSecondGlobal[3];
extern TCHAR szAmGlobal[5];
extern TCHAR szPmGlobal[5];

BOOL IsDbcsTimeSeparator(TCHAR cSeparator);

//#ifdef  BUGS
// Bug-001
extern  void    SS_BeepLowlevel(HWND hwnd);
//#endif

//--------------------------------------------------------------------------//
static void        FreeTimeField     (HWND);
//--------------------------------------------------------------------------//

BOOL TimeCreateField (HWND hWnd, LPARAM lParam)

{
   LPTIMEFIELD lpField;
   HANDLE      hTime;
   TCHAR       szTemp[2];
   BOOL        fRet = FALSE;
   LONG        lStyle;

   if (!(hTime = GlobalAlloc (GHND, (DWORD) sizeof(TIMEFIELD))))
   {
      NotifyParent (hWnd, EN_ERRSPACE);
   }

   else
   {
      lpField = (LPTIMEFIELD) GlobalLock (hTime);
      if (lpField)
      {
         SetDefWndProc(hWnd, GetTBEditClassProc());
         SetWindowTimeField (hWnd, hTime);

         if (((LPCREATESTRUCT)lParam)->style & WS_DISABLED)
            FORWARD_WM_ENABLE(hWnd, 0, PostMessage);

         lStyle = ((LPCREATESTRUCT) lParam)->style |= ES_OVERSTRIKE;

         lpField->hParent =    ((LPCREATESTRUCT) lParam)->hwndParent;

         lpField->tf.bSeconds =   (lStyle & TS_SECONDS) != 0;

         GetProfileString (_T("intl"), _T("sTime"), DEFAULT_SEPARATOR, szTemp, 2);
         lpField->tf.cSeparator = szTemp[0];

         lpField->lRangeMin = UNDEFINED;
         lpField->lRangeMax = UNDEFINED;
         fRet = TRUE;
         GlobalUnlock (hTime);
      }
      else
         NotifyParent (hWnd, EN_ERRSPACE);
   }

   return(fRet);
}

//--------------------------------------------------------------------------//

void TimeInitializeField (HWND hWnd, LPARAM lParam)

{
   LPTIMEFIELD lpField;
   TIME        Time;
   TCHAR       szTime[TIMELENGTH];

   lpField = LockTimeField (hWnd);


   if (((LPCREATESTRUCT)lParam)->lpszName)
      StrnnCpy(szTime, (LPTSTR)((LPCREATESTRUCT)lParam)->lpszName,
               sizeof(szTime));
   else
      szTime[0] = '\0';

   if (StrCmp (StrTrim (szTime), _T("")) == 0)
   {
      SysGetTime (&Time.nHour, &Time.nMinute, &Time.nSecond);
      TimeHMSToString (&Time, szTime, &lpField->tf);
   }

   if (GetWindowLong(hWnd, GWL_STYLE) & TS_SPIN)
      {
      lpField->tf.bSpin = TRUE;
      TimeResetSpinBtn(hWnd, TRUE);
      }

   // Send the string to the Edit Field processor
   if (TimeStringIsValid (szTime, &lpField->tf))
      SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szTime);

   UnlockTimeField (hWnd);
}

//--------------------------------------------------------------------------//

BOOL TimeSetValidRange (HWND hWnd, LPARAM lParam)

{
   LPCTSTR     lpszString = (LPCTSTR) lParam;
   LPCTSTR     Ptr;
   LPTIMEFIELD lpField;
   LONG        lMin = 0L;
   LONG        lMax = 0L;
   BOOL        Ok;

   lpField = LockTimeField (hWnd);

   /*
   StrScanf (lpszString, "%ld %ld", (LPLONG) &lMin, (LPLONG) &lMax);
   */
   lMin = StringToLong(lpszString);

   if (Ptr = StrChr(lpszString, ' '))
      lMax = StringToLong(Ptr + 1);

   if (lMin <= lMax)
   {
      lpField->lRangeMin = lMin;
      lpField->lRangeMax = lMax;
      Ok = TRUE;
   }

   else
      Ok = FALSE;

   UnlockTimeField (hWnd);

   return Ok;
}

//--------------------------------------------------------------------------//
// Changed by Bill Sheng, BOC 1995/01/30
void TimeSetFieldFormat (HWND hWnd, LPARAM lParam)

{
   LPTIMEFIELD lpField;

   lpField = LockTimeField (hWnd);

   // Store the new format
   MemCpy(&lpField->tf, (LPTIMEFORMAT)lParam, sizeof(TIMEFORMAT));
   TimeResetSpinBtn(hWnd, lpField->tf.bSpin);
   SetWindowText(hWnd, _T(""));

   InvalidateRect(hWnd, NULL, TRUE);
   UpdateWindow(hWnd);

   UnlockTimeField (hWnd);
}

//--------------------------------------------------------------------------//

LONG TimeGetFieldFormat (HWND hWnd, LPARAM lParam)

{
   LPTIMEFIELD  lpField;

   lpField = LockTimeField (hWnd);

   MemCpy((LPTSTR)lParam, (LPTSTR)&lpField->tf, sizeof(TIMEFORMAT));

   UnlockTimeField (hWnd);

   return (TRUE);
}

//--------------------------------------------------------------------------//

void  TimeDestroyField (HWND hWnd)

{
   FreeTimeField (hWnd);
}

//--------------------------------------------------------------------------//

LONG  TimeDynamicValidate (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   int         iPos = (int)wParam;
   LPTSTR      lpszString = (LPTSTR) lParam;
   LPTIMEFIELD lpField;
   LPEDITFIELD lpEditField;
   int         nLength;
   TCHAR       ch;
   TCHAR       cSeparator;
//- Masanori Iwasa   94' 01/12 BOC
   int         b12Hour, b24Hour;

   BOOL        bSeconds;
   BOOL        Ok = TRUE;

#ifdef	SPREAD_JPN
   // Fengwei
   TCHAR	   ch1 = szAmGlobal[1];
   TCHAR	   ch2 = szAmGlobal[2];
   TCHAR       ch3 = szAmGlobal[3];
#endif


//- Added in by Masanori Iwasa-- 94' 01/13 BOC
//    New structure for handling TIME input.
//
//          01234567890123456      In the USA mode, 'MasaFixTable' is used to
//     JPN  12Žž01•ª01•b ŒßŒã      reference the location, in order to check
//     USA  12: 01: 01:  am        the inputted character validation.

#ifdef _UNICODE
   int         nCharWidth = 1;
#else
   int         nCharWidth = 2;
#endif
   int         nPointer;
   int         InputLength;
   int         MasaFixTable[13] = { 0, 1, 2, 4, 5, 6, 8, 9, 12, 13, 14, 15, 16 };
#ifdef _UNICODE
   int         LenTable[2][4] = { { 11, 8, 11, 11 },
                                  {  8, 5,  8,  8 }  };
#else
   int         LenTable[2][4] = { { 11, 8, 13, 13 },
                                  {  8, 5, 10, 10 }  };
#endif

   if (lpszString && *lpszString == '\0')
      return (TRUE);

   lpField = LockTimeField (hWnd);
   b24Hour =      lpField->tf.b24Hour;
   b12Hour =      !b24Hour;
   bSeconds =     lpField->tf.bSeconds;
   cSeparator =   lpField->tf.cSeparator;
   UnlockTimeField (hWnd);

   lpEditField = LockField(hWnd);

   if (!lpszString)
      lpszString = lpEditField->lpszString;

   nLength = StrLen (lpszString);

   InputLength = LenTable[( IsDbcsTimeSeparator(cSeparator) ) ? 0 : 1][b24Hour];

   if (bSeconds)
		{
      InputLength += 3;
#ifndef _UNICODE
		if (IsDbcsTimeSeparator(cSeparator))
			InputLength++;
#endif
		}

   if (iPos >= nLength)
      return (TimeStaticValidate(hWnd, &lParam));

	// Modified by Masanori Iwasa BOC
   if (b24Hour == 3 && nLength > 1)
      {
      if (iPos < (nCharWidth * 2))
         nPointer = 13 + iPos;
      else
			{
#ifndef _UNICODE
         if (IsDbcsTimeSeparator(cSeparator))
            nPointer = (iPos == (nCharWidth * 2)) ? 12 : iPos - 1 - (nCharWidth * 2);
         else
#endif
				{
	         if (IsDbcsTimeSeparator(cSeparator) && iPos == 11)
					nPointer = 10;
				else
					nPointer = (iPos == (nCharWidth * 2)) ? 12 : MasaFixTable[iPos-1-(nCharWidth * 2)];
				}
			}
      }
   else
      {
#ifndef _UNICODE
      if ( IsDbcsTimeSeparator(cSeparator) )
	      {
         nPointer = iPos;
         if (!bSeconds && b24Hour != 1 && nPointer > 7)  //- Adjust when no seconds.
            nPointer = 12 + iPos - 8;
         }
      else
#endif
         {
#ifdef _UNICODE
         if (IsDbcsTimeSeparator(cSeparator) && iPos == 8)
				nPointer = 10;
         else if (IsDbcsTimeSeparator(cSeparator) && iPos == 9)
				nPointer = 12;
         else if (IsDbcsTimeSeparator(cSeparator) && iPos == 10)
				nPointer = 13;
         else if (IsDbcsTimeSeparator(cSeparator) && iPos == 11)
				nPointer = 14;
			else
#endif
	         nPointer = MasaFixTable[iPos];

         if (!bSeconds && b24Hour != 1 && iPos > 4)  //- Adjust when no seconds.
            nPointer = 12 + iPos - 5;
         }
      }

   if (nLength > InputLength)
      Ok = FALSE;

   if (Ok)
   {
#ifdef SPREAD_JPN
		// BUG SPR_JPN_001_016 (2-1)
		// When User Input Time, It Will Generate WM_CHAR Messages For Each Character
		// By Calling SpreadSheetProc() Function In OnChar() In SSOCXCTL.CPP.
		// It Also Post WM_CHAR Messages To Add Separators To The Time Stirng In Current Function.
		// Evidencely, The Separators Should Be Put In Proper Position In The Time String.
		// But Unfortunately, When Input Time By Using IME,
		// It Will Send The Whole Time String And The Separators Will Be Added To The End Of The String.
		// This Will Cause BUG SPR_JPN_001_016 
		// Following Codes Save All WM_CHAR Messages Currently In Message Queue And Remove Them,
		// After Add Separators, These Messages Will Be Recovered At The End Of The "If" Clause
		// By The Way, Date Input Has Different Processing As Time, So Date Input Is Correct
		// Modified By HaHa 1999.10.25
		MSG Message;
		HGLOBAL MessageArrayHandle=NULL;
		LPMSG lpMessage;
		int TempMessageCount=0;
		int MessageCount=0;

		// BUG Refix SPR_JPN_001_016 (1-1)
		// When Using Japanese Time Separator, After Remove A WM_CHAR Message Then Post It,
		// The Result Is Different. The Posted Message Does Not Work. (Only On NT)
		// Maybe It Depends On Send Time Of Message, By Which System Match Two Bytes Of A Charactor.
		// So Don't Dispose Double Byte Charactor Here, Because A Separator Only Followed By A Number.
		// Modified By HaHa 1999.12.14
		if (lpszString[iPos]>='0' && lpszString[iPos]<='9')
			while (PeekMessage(&Message,hWnd,WM_CHAR,WM_CHAR,PM_REMOVE))
			{
				MessageCount++;
				if (!MessageArrayHandle)
					MessageArrayHandle=GlobalAlloc(GHND,sizeof(MSG)*MessageCount);
				else
					MessageArrayHandle=GlobalReAlloc(MessageArrayHandle,sizeof(MSG)*MessageCount,GHND);
				lpMessage=(LPMSG)GlobalLock(MessageArrayHandle);
				lpMessage[MessageCount-1]=Message;
				GlobalUnlock(MessageArrayHandle);
			}
#endif

      ch = lpszString[iPos];

      switch (nPointer)
      {

         case 0:
// Japanese version added value 2 and 3 to TypeTimeSeparator property.
            if (b24Hour == 3 && nLength < 2)
               {
               lpszString[iPos] = szAmGlobal[0];
#ifdef	SPREAD_JPN
			   // Fengwei
			   if(ch == 'p' || ch == 'P')
			   {
					ch1 = szPmGlobal[1],
					ch2 = szPmGlobal[2],
					ch3 = szPmGlobal[3];
			   }
               PostMessage(hWnd, WM_CHAR, (TUCHAR)ch1, 0);
               PostMessage(hWnd, WM_CHAR, (TUCHAR)ch2, 0);
               PostMessage(hWnd, WM_CHAR, (TUCHAR)ch3, 0);
#else
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[1], 0);
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[2], 0);
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[3], 0);
#endif
               PostMessage(hWnd, WM_CHAR, ' ', 0);
			   //Modify By BOC 99.6.16 (hyt)-----------------------------
			   //for not send duplicate message
			   if(ch!='a' && ch != 'A' && ch!='p' && ch!='P')
				  PostMessage(hWnd, WM_CHAR, (TUCHAR)ch, 0);
			   //--------------------------------------------------------
               }
            else if (ch < '0' || ch > '9')
               Ok = FALSE;
            else
               {
               if (b24Hour != 1)
                  {
                  if (ch > '1') Ok = FALSE;
                  }
               else
                  {
                  if (ch > '2') Ok = FALSE;
                  }
               }
            break;

         case 1:
            if (ch < '0' || ch > '9' ) Ok = FALSE;

            if (b24Hour == 0 && lpszString[iPos-1] == '0' && ch == '0')
               Ok = FALSE;
            if (b24Hour != 1 && lpszString[iPos-1] == '1' && ch > '2')
               Ok = FALSE;
            if (b24Hour == 1 && lpszString[iPos-1] == '2' && ch > '3')
               Ok = FALSE;

            if ( ( !IsDbcsTimeSeparator(cSeparator) ) && Ok)
               PostMessage(hWnd, WM_CHAR, (TUCHAR)cSeparator, 0);
            else if (Ok)
               {
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szHourGlobal[0], 0);
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szHourGlobal[1], 0);
               }
            break;

         case 2:
            if ( ( !IsDbcsTimeSeparator(cSeparator) ) && ch != cSeparator) Ok = FALSE;
// 96' 5/20 BOC Gao. for a bug.
//            if ( ( IsDbcsTimeSeparator(cSeparator) ) == '?' && (BYTE)ch != (BYTE)szHourGlobal[0]) Ok = FALSE;
            if (  IsDbcsTimeSeparator(cSeparator)  && ch != szHourGlobal[0]) Ok = FALSE;
// ---------------------------<<
            break;

         case 3:
            if ((TCHAR)ch != (TCHAR)szHourGlobal[1]) Ok = FALSE;
            break;

         case 4:
            if (ch < '0' || ch > '5') Ok = FALSE;
            break;

         case 5:
            if (ch < '0' || ch > '9' ) Ok = FALSE;
            if ( IsDbcsTimeSeparator(cSeparator) && Ok)
               {
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szMinuteGlobal[0], 0);
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szMinuteGlobal[1], 0);
               }
            else if (bSeconds && Ok)
               PostMessage(hWnd, WM_CHAR, (TUCHAR)cSeparator, 0);

            if ((b24Hour == 0 || b24Hour == 2) && !bSeconds && Ok)
                  PostMessage(hWnd, WM_CHAR, ' ', 0);  
            break;

         case 6:
            if ( (!IsDbcsTimeSeparator(cSeparator)) && ch != cSeparator) Ok = FALSE;
            if ( (IsDbcsTimeSeparator(cSeparator)) && (TCHAR)ch != (TCHAR)szMinuteGlobal[0]) Ok = FALSE;
            break;

         case 7:
            if ((TCHAR)ch != (TCHAR)szMinuteGlobal[1] ) Ok = FALSE;
            break;

         case 8:
            if (ch < '0' || ch > '5') Ok = FALSE;
            break;

         case 9:
            if (ch < '0' || ch > '9' ) Ok = FALSE;
            if ( (IsDbcsTimeSeparator(cSeparator)) && Ok)
               {
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szSecondGlobal[0], 0);
#ifndef _UNICODE
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szSecondGlobal[1], 0);
#endif
               }

            if ((b24Hour == 0 || b24Hour == 2) && Ok)
               PostMessage(hWnd, WM_CHAR, ' ', 0);
            break;

         case 10:
            if ( (!IsDbcsTimeSeparator(cSeparator)) && ch != cSeparator) Ok = FALSE;
            if ( (IsDbcsTimeSeparator(cSeparator)) && (TCHAR)ch != (TCHAR)szSecondGlobal[0]) Ok = FALSE;
            break;

         case 11:
            if ((TCHAR)ch != (TCHAR)szSecondGlobal[1]) Ok = FALSE;
            break;

         case 12:
            if (ch != ' ') Ok = FALSE;
            break;

         case 13:
            if (ch != 'a' && ch != 'A' && ch != 'p' && ch != 'P' &&
               (TCHAR)ch != (TCHAR)szAmGlobal[0] )
               Ok = FALSE;

				// RFW - 7/1/04 - 14586
            else if ((ch == 'a' || ch == 'A') && lpField->lRangeMin >= 46800)
               Ok = FALSE;

            if (b24Hour == 0 && Ok)
               {
               lpszString[iPos] = lpszString[iPos] | 0x20;
               PostMessage(hWnd, WM_CHAR, 'm', 0);
               }
            else if (b24Hour != 1 && Ok)
               {
               lpszString[iPos] = szAmGlobal[0];
#ifndef _UNICODE
               PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[1], 0);
#endif
               if (ch == 'a' || ch == 'A' )
                  {
#ifdef _UNICODE
						PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[1], 0);
#else
                  PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[2], 0);
                  PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[3], 0);
#endif
#ifdef	SPREAD_JPN
				  // Fengwei
				  //Modify by BOC 99.6.16 (hyt)------------------
				  //for not send invalid message when at special mode
				  if(b24Hour!=2)
					PostMessage(hWnd, WM_CHAR, ' ', 0);
				  //-----------------------------------------------
#endif
                  }
               else
                  {
#ifdef _UNICODE
						PostMessage(hWnd, WM_CHAR, (TUCHAR)szPmGlobal[1], 0);
#else
                  PostMessage(hWnd, WM_CHAR, (TUCHAR)szPmGlobal[2], 0);
                  PostMessage(hWnd, WM_CHAR, (TUCHAR)szPmGlobal[3], 0);
#endif
#ifdef	SPREAD_JPN
				  // Fengwei
				  //Modify by BOC 99.6.16 (hyt)------------------
				  //for not send invalid message when at special mode
				  if(b24Hour!=2)
					PostMessage(hWnd, WM_CHAR, ' ', 0);
				  //-----------------------------------------------
#endif
                  }
               }
            break;

         case 14:
            if (b24Hour == 0)
               {
               if (ch != 'm' && ch != 'M')
                  Ok = FALSE;
               }
            else if (b24Hour != 1 )
               {
               if (ch == 'a' || ch == 'A' || ch == szAmGlobal[1])
                  {
                  lpszString[iPos] = szAmGlobal[1];
				  //Modify by BOC 99.6.16 (hyt)---------------------------
				  //for avoid send duplicate message
				  if(ch == 'a' || ch == 'A')
				  {
					  PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[2], 0);
					  PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[3], 0);
				  }
				  //-------------------------------------------------------
                  }
               else if (ch == 'p' || ch == 'P' || ch == szPmGlobal[1])
                  {
                  lpszString[iPos] = szPmGlobal[1];
				  //Modify by BOC 99.6.16 (hyt)---------------------------
				  //for avoid send duplicate message
				  if(ch == 'p' || ch == 'P')
				  {
					  PostMessage(hWnd, WM_CHAR, (TUCHAR)szPmGlobal[2], 0);
					  PostMessage(hWnd, WM_CHAR, (TUCHAR)szPmGlobal[3], 0);
				  }
				  //-------------------------------------------------------
                  }
               else
                  Ok = FALSE;
               }
            break;

         case 15:
            if (ch == 'a' || ch == 'A' || ch == szAmGlobal[2])
               {
               lpszString[iPos] = szAmGlobal[2];
			   //Modify by BOC 99.6.16 (hyt)---------------------------
			   //for avoid send duplicate message
			   if(ch == 'a' || ch == 'A')
			   {
					PostMessage(hWnd, WM_CHAR, (TUCHAR)szAmGlobal[3], 0);
			   }
			   //--------------------------------------------------------
               }
            else if (ch == 'p' || ch == 'P' || ch == szPmGlobal[2])
               {
               lpszString[iPos] = szPmGlobal[2];
			   //Modify by BOC 99.6.16 (hyt)---------------------------
			   //for avoid send duplicate message
			   if(ch == 'p' || ch == 'P')
			   {
				   //Modify By BOC 99.6.16 (hyt)---------------------------
				   //for not just send second byte of "Œã",if send will generate incorrect result
#ifdef SPREAD_JPN
				   PostMessage(hWnd, WM_CHAR, (TUCHAR)szPmGlobal[2], 0);
#endif
                    PostMessage(hWnd, WM_CHAR, (TUCHAR)szPmGlobal[3], 0);
			   }
			   //-------------------------------------------------------
               }
            else
               Ok = FALSE;
            break;

         case 16:
            if (ch == 'a' || ch == 'A' || ch == szAmGlobal[3])
               {
               lpszString[iPos-1] = szAmGlobal[2];
               lpszString[iPos] = szAmGlobal[3];
               }
            else if (ch == 'p' || ch == 'P' || ch == szPmGlobal[3])
               {
               lpszString[iPos-1] = szPmGlobal[2];
               lpszString[iPos] = szPmGlobal[3];
               }
            else
               Ok = FALSE;
            break;

         default:
            Ok = FALSE;
            break;

      }

#ifdef SPREAD_JPN
		// BUG SPR_JPN_001_016 (2-2)
		// Recovered WM_CHAR Messages For Time String
		// Modified By HaHa 1999.10.25
		while (TempMessageCount<MessageCount)
		{
			lpMessage=(LPMSG)GlobalLock(MessageArrayHandle);
			Message=lpMessage[TempMessageCount];
			PostMessage(hWnd,Message.message,Message.wParam,Message.lParam);
			GlobalUnlock(MessageArrayHandle);
			TempMessageCount++;
		}
		if (MessageArrayHandle)
			GlobalFree(MessageArrayHandle);
#endif
   }

   if (Ok && nLength == InputLength) // If it's full length now
       Ok = TimeIsInRange(hWnd, lpszString, &lpField->tf); //Ok must be OK!

   if (!Ok)
//#ifdef  BUGS
// Bug-001
      SS_BeepLowlevel(hWnd);  
//#else
//      MessageBeep (MB_OK);
//#endif

   UnlockField(hWnd);
   return (LONG) Ok;
}

//--------------------------------------------------------------------------//

LONG  TimeStaticValidate (HWND hWnd, LPARAM* lParam)

{
   LPTIMEFIELD lpField;
   int         nLength;
   BOOL        bAddZero;
   BOOL        Ok = TRUE;
   TIMEFORMAT  tf;

   static      TCHAR szLocal[LOCAL_LENGTH];

//- Added by Masanori Iwasa-- 94' 01/15 BOC
#ifdef _UNICODE
   int         LenTable[2][4] = { {  7, 4,  7,  7 },
                                  { 10, 7, 10, 10 }  };
#else
   int         LenTable[2][4] = { {  7, 4,  9,  9 },
                                  { 10, 7, 12, 12 }  };
#endif
   int         CalcPos;

   if (!(*lParam))
      return (TRUE);

   lpField = LockTimeField (hWnd);
   tf = lpField->tf;
   UnlockTimeField (hWnd);

   StrnnCpy (szLocal, (LPTSTR) *lParam, LOCAL_LENGTH);
   nLength = StrLen (StrTrim(szLocal));

   // Allow the user to set it to the null string
   if (nLength == 0)
      return (TRUE);

   // If we're only one character short,
   //    assume its because the leading 0 has been omitted
   bAddZero = FALSE;

//- Changed by Masanori Iwasa-- 94' 01/17 BOC
   CalcPos = LenTable[( IsDbcsTimeSeparator(tf.cSeparator) ) ? 1 : 0][tf.b24Hour];
   if (tf.bSeconds)
      CalcPos += ( IsDbcsTimeSeparator(tf.cSeparator) ) ? 4 : 3;

   if (nLength == CalcPos) bAddZero = TRUE;

   if (bAddZero)
   {
//- Changed by Masanori Iwasa-- 94' 01/17 BOC
      if (tf.b24Hour == 3)
         StrInsertChar (szAmGlobal[0], szLocal, 0);
      else
         StrInsertChar ('0', szLocal, 0);

      ++nLength;
   }

   Ok = TimeStringIsValid (szLocal, &tf);

   if (Ok)
      Ok = TimeIsInRange (hWnd, szLocal, &tf);

   if (Ok)
      *lParam = (LPARAM) (LPTSTR) szLocal;    // Replace the original string.

   return (LONG) Ok;
}

//--------------------------------------------------------------------------//

BOOL  TimeFinalValidate (HWND hWnd)
{
   LPTIMEFIELD lpField;
   BOOL        Ok;
   TIMEFORMAT  tf;

   TCHAR       szLocal[LOCAL_LENGTH];

   lpField = LockTimeField (hWnd);
   tf = lpField->tf;
   UnlockTimeField (hWnd);

   SendMessage (hWnd, WM_GETTEXT, LOCAL_LENGTH, (LPARAM) (LPTSTR) szLocal);

   Ok = TimeStringIsValid (szLocal, &tf);

   if (Ok)
      Ok = TimeIsInRange (hWnd, szLocal, &tf);

   // BUG SPR_EVN_001_005 (2-2)
   // Modified By HaHa 1999.12.6
   if (!StrLen(szLocal)) Ok=TRUE;

   return Ok;
}

//--------------------------------------------------------------------------//
BOOL TimeIsInRange (HWND hWnd, LPCTSTR lpszString, LPTIMEFORMAT lptf)
{
   LPTIMEFIELD lpField;
   TIME        Time;
   LONG        lTime;
   BOOL        Ok = FALSE;

   lpField = LockTimeField (hWnd);
   if (lpField)
   {
      // Check to see if a range has been set
      if (lpField->lRangeMin != UNDEFINED && lpField->lRangeMax != UNDEFINED)
      {
         // Decode the time to be tested
         TimeStringToHMS (lpszString, &Time, lptf);
         lTime = TimeHMSToSeconds (&Time);

         // Check against the range
         Ok = lpField->lRangeMin <= lTime && lTime <= lpField->lRangeMax;
      }
      else
         Ok = TRUE;
      UnlockTimeField (hWnd);
   }
   return Ok;
}

//--------------------------------------------------------------------------//

LPTIMEFIELD LockTimeField (HWND hWnd)

{
   HANDLE hField;

   if (hField = GetWindowTimeField (hWnd))
      return (LPTIMEFIELD) GlobalLock (hField);
   else
      return NULL;
}

//--------------------------------------------------------------------------//

void UnlockTimeField (HWND hWnd)

{
   HANDLE hField;

   if (hField = GetWindowTimeField (hWnd))
      GlobalUnlock (hField);
}

//--------------------------------------------------------------------------//

void FreeTimeField (HWND hWnd)

{
   LPTIMEFIELD lpTimeField;
   HANDLE      hField;

   if (hField = GetWindowTimeField (hWnd))
      {
      lpTimeField = (LPTIMEFIELD)GlobalLock(hField);

      if (lpTimeField->hWndSpinBtn)
         DestroyWindow(lpTimeField->hWndSpinBtn);

      GlobalUnlock(hField);
      tbMemFree(hField);
      SetWindowTimeField(hWnd, 0);
      }
}

//--------------------------------------------------------------------------//

void  LoadCurrentTime (HWND hWnd)

{
   LPTIMEFIELD lpField;
   TIME        Time;
   TCHAR       szTime[TIMELENGTH];

   lpField = LockTimeField (hWnd);

   SysGetTime (&Time.nHour, &Time.nMinute, &Time.nSecond);
   TimeHMSToString (&Time, szTime, &lpField->tf);

   SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPTSTR)szTime);

   UnlockTimeField (hWnd);

   return;
}


void TimeSize(HWND hWnd, LPARAM lParam)
{
LPTIMEFIELD lpTimeField;
LPEDITFIELD lpField;

lpTimeField = LockTimeField(hWnd);
lpField = LockField(hWnd);

if (lpTimeField->hWndSpinBtn)
   TimeResetSpinBtn(hWnd, TRUE);

else
   lpField->xRightNonClientXtra = 0;

UnlockField(hWnd);
UnlockTimeField(hWnd);
}

#ifdef SS_V80
void TimeSetSpinStyle (HWND hWnd, BOOL fEnhanced)

{
	LPEDITFIELD lpField = LockField(hWnd);
	lpField->fEnhanced = fEnhanced;
	UnlockField(hWnd);
}
#endif

void TimeResetSpinBtn(HWND hWnd, BOOL fShowSpin)
{
LPTIMEFIELD lpTimeField;
LPEDITFIELD lpField;
RECT        Rect;
short       SBWidth = 0;
short       SBHeight = 0;

lpTimeField = LockTimeField(hWnd);
lpField = LockField(hWnd);

GetClientRect(hWnd, &Rect);

if (fShowSpin)
   {
//   SBHeight = (short)(Rect.bottom - Rect.top);
   SBHeight = (short)min(Rect.bottom - Rect.top, (lpField->yHeight * 3) / 2);
   SBWidth = (SBHeight * 2) / 3;
   }

lpField->xRightNonClientXtra = SBWidth;

if (lpField->f3D)
   {
   tbShrinkRect(&Rect, lpField->Edit3DFormat.dShadowSize,
                lpField->Edit3DFormat.dShadowSize);

   if (lpField->Edit3DFormat.wStyle & EDIT3D_INSIDEBORDER)
      tbShrinkRect(&Rect, 1, 1);
   }

if (fShowSpin && !lpTimeField->hWndSpinBtn)
   {
   lpTimeField->hWndSpinBtn = CreateWindow(lpszSpinClassName, NULL, WS_CHILD |
                                           WS_VISIBLE | SBNS_BORDERLEFT,
                                           Rect.right - SBWidth, Rect.top,
                                           SBWidth, SBHeight, hWnd, 0,
                                           GetWindowInstance(hWnd), NULL);

   if (GetWindowLong(hWnd, GWL_STYLE) & ES_STATIC)
      EnableWindow(lpTimeField->hWndSpinBtn, FALSE);
   }

else if (!fShowSpin && lpTimeField->hWndSpinBtn)
   {
   DestroyWindow(lpTimeField->hWndSpinBtn);
   lpTimeField->hWndSpinBtn = 0;
   }

else if (fShowSpin && lpTimeField->hWndSpinBtn)
   MoveWindow(lpTimeField->hWndSpinBtn, Rect.right - SBWidth, Rect.top,
              SBWidth, SBHeight, TRUE);

if (lpTimeField->hWndSpinBtn)
  SendMessage(lpTimeField->hWndSpinBtn , EM_SETSPINSTYLE, 0, (LPARAM)lpField->fEnhanced);

UnlockField(hWnd);
UnlockTimeField(hWnd);
}


void TimeVScroll(HWND hWnd, int nScrollCode)
{
LPEDITFIELD lpField;
LPTIMEFIELD lpTimeField;
TIME        Time;
TCHAR       szTime[MAXFIELD + 1];
long        lSeconds;
short       iCursorPos;

lpTimeField = LockTimeField(hWnd);

if (!lpTimeField->hWndSpinBtn || (nScrollCode != SB_LINEDOWN &&
    nScrollCode != SB_LINEUP))
   {
   UnlockTimeField(hWnd);
   return;
   }

lpField = LockField(hWnd);

if ((iCursorPos = TimeGetCursorPos(hWnd, (short)lpField->iCurrentPos)) == -1)
   {
   UnlockField(hWnd);
   UnlockTimeField(hWnd);
   return;
   }

TimeStringToHMS(lpField->lpszString, &Time, &lpTimeField->tf);
lSeconds = TimeHMSToSeconds(&Time);

// If there is nothing entered, then start at 0(12:00am)
if (*lpField->lpszString) // RFW - 7/1/04 - 14587
	{
	switch (iCursorPos)
		{
		case TIME_HH:
			if (nScrollCode == SB_LINEDOWN)
				lSeconds -= 3600;
			else
				lSeconds += 3600;

			break;

		case TIME_MM:
			if (nScrollCode == SB_LINEDOWN)
				lSeconds -= 60;
			else
				lSeconds += 60;

			break;

		case TIME_SS:
			if (nScrollCode == SB_LINEDOWN)
				lSeconds--;
			else
				lSeconds++;

			break;

		case TIME_AM:
			if (nScrollCode == SB_LINEDOWN)
				lSeconds -= 43200;
			else
				lSeconds += 43200;

			break;
		}
	}

if (lSeconds >= 0 && lSeconds < 86400)
   {
	// RFW - 6/14/04 - 14585
	if (lpTimeField->lRangeMin != UNDEFINED && lSeconds < lpTimeField->lRangeMin)
		lSeconds = lpTimeField->lRangeMin;

	if (lpTimeField->lRangeMax != UNDEFINED && lSeconds > lpTimeField->lRangeMax)
		lSeconds = lpTimeField->lRangeMax;

   TimeSecondsToHMS(lSeconds, &Time);
   TimeHMSToString(&Time, szTime, &lpTimeField->tf);
   SetWindowText(hWnd, szTime);
   UpdateWindow(hWnd);
   }

UnlockField(hWnd);
UnlockTimeField(hWnd);
}


short TimeGetCursorPos(HWND hWnd, short iPos)
{
LPTIMEFIELD lpField;
short       iCursorPos = -1;

//- Added in by Masanori Iwasa-- 94' 01/15 BOC
int         Sep,xStart;
int         PosTbl[3][5] = { { 0, 3, 6, 6,  9 },
                             { 0, 4, 8, 9, 13 },
                             { 2, 0, 4, 0,  0 }  };

lpField = LockTimeField (hWnd);

//- Added in by Masanori Iwasa-- 94' 01/15 BOC

   //- Get separator type and starting position.
   Sep = ( IsDbcsTimeSeparator(lpField->tf.cSeparator) ) ? TRUE : FALSE;

#ifdef _UNICODE
   xStart = (lpField->tf.b24Hour == 3) ? 3 : 0;
#else
   xStart = (lpField->tf.b24Hour == 3) ? 5 : 0;
#endif

   if (iPos >= PosTbl[Sep][0] + xStart && iPos < PosTbl[Sep][0] + xStart + 2)
      iCursorPos = TIME_HH;
   else if (iPos >= PosTbl[Sep][1] + xStart && iPos < PosTbl[Sep][1] + xStart + 2)
      iCursorPos = TIME_MM;
   else if (lpField->tf.bSeconds &&
      iPos >= PosTbl[Sep][2] + xStart && iPos < PosTbl[Sep][2] + xStart + 2)
      iCursorPos = TIME_SS;
   else
      {
      if (lpField->tf.b24Hour == 3)
         {
         if (iPos >= 0 && iPos < 4)
            iCursorPos = TIME_AM;
         }
      else if (lpField->tf.b24Hour != 1)
         {
         if (iPos >= PosTbl[lpField->tf.bSeconds & 1][Sep + 3] &&
             iPos < (PosTbl[lpField->tf.bSeconds & 1][Sep + 3] + PosTbl[2][lpField->tf.b24Hour]))
            iCursorPos = TIME_AM;
         }
      }

UnlockTimeField(hWnd);
return (iCursorPos);
}


BOOL TimeProcessWMKeyDown(HWND hWnd, WPARAM wParam)
{
BOOL fRet = FALSE;

switch (wParam)
   {
   case VK_DOWN:
      TimeVScroll(hWnd, SB_LINEDOWN);
      fRet = TRUE;
      break;

   case VK_UP:
      TimeVScroll(hWnd, SB_LINEUP);
      fRet = TRUE;
      break;
   }

return (fRet);
}


void TimeEnable(HWND hWnd, WPARAM wParam)
{
LPTIMEFIELD lpInfo;

lpInfo = LockTimeField(hWnd);

if (lpInfo->hWndSpinBtn)
   EnableWindow(lpInfo->hWndSpinBtn, (BOOL)wParam);

UnlockTimeField(hWnd);
}
