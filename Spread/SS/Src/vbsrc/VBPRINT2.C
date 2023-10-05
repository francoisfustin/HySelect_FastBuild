/*********************************************************
* VBPRINT2.C
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "toolbox.h"
#include "..\sssrc\spread\spread.h"
#include "fphdc.h"
#include "vbmsg.h"
#include "vbprint2.h"
#include "vbdattim.h"
#include "vbsscnvt.h"                                //DBOCX
#include "..\ssprvw\prvwapi.h"
#ifdef tm
asdfasdsd
#endif
#include <time.h>


PRINTABORTTABLE PrintAbortTable[PRINTABORTTABLE_MAX];
short           PrintAbortTableCnt;
void            vbSSDisablePrintAbortItem(HWND hWndDlg);


short vbSpreadPrintHeaderFooter(HDC hDC, GLOBALHANDLE hText, short nPageNum,
                                float fPrintMarginRight, BOOL fCalc,
                                LPSS_PRINTINFO lpPrintInfo)
{
GLOBALHANDLE hString;
LPTSTR       lpszString;
LPTSTR       lpszText;
LPTSTR       lpStart;
LPTSTR       Ptr;
LPTSTR       PtrTemp;
HFONT        hFont[10];
HFONT        hFontOrig;
HFONT        hFontTemp;
HFONT        hFontCurrent;
HFONT        hFontCreated = 0;
LOGFONT      LogFont;
BOOL         fNewLine = FALSE;
BOOL         fPrint = FALSE;
BOOL         fCreateFont = FALSE;
BOOL         fDeleteFont = FALSE;
int          x = 0;
int          y = 0;
int          Len;
int          dNameLen;
int          Size;
int          Width;
int          HeightRow;
int          Height;
short        dJust = SS_TEXT_LEFT;
short        dJustOld;
short        dJustTemp;
short        dHorzPrinterRes;
short        dVertPrinterRes;
int          dLeft;
int          dRight;
int          i;
float        fTemp;
SIZE         TextSize;
POINT        Point;
#ifdef SS_V80
LONG		 Color;
BOOL         bUseColor = FALSE;
#endif
double       dfScalePreviewX = 1.0;
double       dfScalePreviewY = 1.0;
BOOL         fIsPreview = (lpPrintInfo && lpPrintInfo->fIsPreview);
HDC          hDCPrinter = (fIsPreview ? lpPrintInfo->hDCPrinter : hDC);

if (hText)
   {
   dHorzPrinterRes = GetDeviceCaps(hDCPrinter, HORZRES);
   dVertPrinterRes = GetDeviceCaps(hDCPrinter, VERTRES);

   if (fIsPreview)
      {
      dfScalePreviewX = (double)(lpPrintInfo->RectPreview.right -
                                 lpPrintInfo->RectPreview.left) /
                                 (double)dHorzPrinterRes;
      dfScalePreviewY = (double)(lpPrintInfo->RectPreview.bottom -
                                 lpPrintInfo->RectPreview.top) /
                                 (double)dVertPrinterRes;
      dLeft = lpPrintInfo->RectPrinter.left;
      dRight = lpPrintInfo->RectPrinter.right;
      }

   else
      {
      GetViewportOrgEx(hDC, &Point);
      dLeft = Point.x;

      fTemp = (float)GetDeviceCaps(hDC, LOGPIXELSX);

      if (fPrintMarginRight == 0)
         fTemp /= (float)16; // Default to 1/16 of an inch
      else
         fTemp *= fPrintMarginRight;

      dRight = dHorzPrinterRes - (short)fTemp;
      }

   dHorzPrinterRes = dRight - dLeft;
   GetTextExtentPoint(hDCPrinter, _T("0"), 1, &TextSize);
   Height = TextSize.cy;
   HeightRow = Height;

   _fmemset(hFont, '\0', sizeof(hFont));

   hFontOrig = SelectObject(hDCPrinter, GetStockObject(SYSTEM_FONT));
   hFontCurrent = hFontOrig;
   SelectObject(hDCPrinter, hFontCurrent);
   hFont[0] = hFontCurrent;

   GetObject(hFont[0], sizeof(LOGFONT), &LogFont);
   LogFont.lfCharSet = DEFAULT_CHARSET;

   lpszText = (LPTSTR)GlobalLock(hText);

   Ptr = lpszText;
   lpStart = lpszText;
   dJustOld = dJust;

   do
      {
      Ptr = StrChr(Ptr, '/');
      fCreateFont = FALSE;

      if (Ptr)
         {
         Len = Ptr - lpStart;
         Ptr++;

         switch (tolower(*Ptr))
            {
            case 'n':                  // Newline
               fPrint = TRUE;
               fNewLine = TRUE;
               Ptr++;
               dJust = SS_TEXT_LEFT;
               break;

            case 'l':                  // Left justify
               fPrint = TRUE;
               Ptr++;
               dJust = SS_TEXT_LEFT;
               break;

            case 'r':                  // Right justify
               fPrint = TRUE;
               Ptr++;
               dJust = SS_TEXT_RIGHT;
               break;

            case 'c':                  // Center
               fPrint = TRUE;
               Ptr++;
#ifdef SS_V80
               if (tolower(*Ptr) == 'l')
                  {
				    fPrint = FALSE;
					bUseColor = TRUE;
                    Ptr++;

                     if (*Ptr == '\"')
                        {
                        Ptr++;

                        if (PtrTemp = StrChr(Ptr, '\"'))
                           {
                           Color = StringToLong(Ptr);

                           Ptr = PtrTemp + 1;
						   lpStart = Ptr;
                           }
                        }
                  }
			   else
#endif
				dJust = SS_TEXT_CENTER;
               break;

            case 'p':                  // Page number
            case '/':
               fPrint = FALSE;
               Ptr++;
               if (tolower(*Ptr) == 'c')
                  {
                  if (!fCalc && lpPrintInfo->lXtra == 0)
                     {
                     PRVW_INFO PrvwInfo;

                     SendMessage(lpPrintInfo->hWndSpread, PRVWM_GETINFO, 0,
                                 (LONG)(LPVOID)&PrvwInfo);
                     lpPrintInfo->lXtra = PrvwInfo.lPageEnd;
                     }

                  Ptr++;
                  }

               break;

#ifdef SS_V70
				case 'd':
               fPrint = FALSE;
					if (Ptr[1] == 'l' || Ptr[1] == 'L' || Ptr[1] == 's' || Ptr[1] == 'S')
						Ptr += 2;
					else if (_tcsnicmp(Ptr, _T("date"), 4) == 0)
						Ptr += 4;
					break;

				case 't':
               fPrint = FALSE;
					if (Ptr[1] == 'l' || Ptr[1] == 'L' || Ptr[1] == 's' || Ptr[1] == 'S')
						Ptr += 2;
					else if (_tcsnicmp(Ptr, _T("time"), 4) == 0)
						Ptr += 4;
					break;
#endif

            case 'f':                  // Font
               dJust = 0;
               fPrint = TRUE;
               Ptr++;

               switch (tolower(*Ptr))
                  {
                  case 'n':            // Font name
                     Ptr++;

                     if (*Ptr == '\"')
                        {
                        Ptr++;

                        if (PtrTemp = StrChr(Ptr, '\"'))
                           {
                           dNameLen = (short)((long)PtrTemp - (long)Ptr);

                           if (dNameLen < LF_FACESIZE)
                              {
                              _fmemset(LogFont.lfFaceName, '\0', LF_FACESIZE * sizeof(TCHAR));
                              _fmemcpy(LogFont.lfFaceName, Ptr, dNameLen * sizeof(TCHAR));
                              fCreateFont = TRUE;
                              }

                           Ptr = PtrTemp + 1;
                           }
                        }

                     break;

                  case 'z':            // Font size
                     Ptr++;

                     if (*Ptr == '\"')
                        {
                        Ptr++;

                        if (PtrTemp = StrChr(Ptr, '\"'))
                           {
                           Size = StringToInt(Ptr);

                           if (Size > 0)
                              {
                              LogFont.lfHeight = PT_TO_PIXELS_EX(hDCPrinter, Size);
                              LogFont.lfWidth = 0;
                              fCreateFont = TRUE;
                              }

                           Ptr = PtrTemp + 1;
                           }
                        }

                     break;

                  case 'b':            // Font bold
                     Ptr++;

                     if (*Ptr == '0')
                        LogFont.lfWeight = FW_NORMAL;
                     else
                        LogFont.lfWeight = FW_BOLD;

                     Ptr++;
                     fCreateFont = TRUE;
                     break;

                  case 'i':            // Font italics
                     Ptr++;
                     LogFont.lfItalic = *Ptr - '0';
                     Ptr++;
                     fCreateFont = TRUE;
                     break;

                  case 'u':            // Font underline
                     Ptr++;
                     LogFont.lfUnderline = *Ptr - '0';
                     Ptr++;
                     fCreateFont = TRUE;
                     break;

                  case 'k':            // Font strikethrough
                     Ptr++;
                     LogFont.lfStrikeOut = *Ptr - '0';
                     Ptr++;
                     fCreateFont = TRUE;
                     break;

                  case 's':            // Font save
                     Ptr++;

                     if (*Ptr >= '1' && *Ptr <= '9')
                        {
                        hFont[*Ptr - '0'] = hFontCurrent;
                        hFontCreated = 0;
                        }

                     Ptr++;
                     break;

                  default:
                     if (*Ptr >= '0' && *Ptr <= '9')
                        {
                        if (hFont[*Ptr - '0'])
                           {
                           fDeleteFont = TRUE;
                           hFontCurrent = hFont[*Ptr - '0'];
                           }

                        Ptr++;
                        }

                     break;
                  }

               break;
            }
         }

      else
         {
         fPrint = TRUE;
         Len = lstrlen(lpStart);
         }

      if (fPrint)
         {
         dJustTemp = dJustOld;

         if (dJust)
            dJustOld = dJust;

         if (Len)
            {
            if (!fCalc)
               {
               hString = vbSSCreatePrintString(lpStart, (short)Len, nPageNum,
                                               lpPrintInfo->lXtra, lpPrintInfo);

               if (hString)
                  {
                  lpszString = (LPTSTR)GlobalLock(hString);
                  Len = lstrlen(lpszString);

                  switch (dJustTemp)
                     {
                     case SS_TEXT_LEFT:
                        x = 0;
                        break;

                     case SS_TEXT_CENTER:
                        GetTextExtentPoint(hDCPrinter, lpszString, Len, &TextSize);
                        Width = TextSize.cx;
                        x = (dHorzPrinterRes - Width) / 2;
                        break;

                     case SS_TEXT_RIGHT:
                        GetTextExtentPoint(hDCPrinter, lpszString, Len, &TextSize);
                        Width = TextSize.cx;
                        x = dHorzPrinterRes - Width;
                        break;
                     }

                  {
                  HFONT hFontOld;

                  // Scale the font if Print Preview

                  if (hDCPrinter != hDC)
                     {
                     TEXTMETRIC tm;
                     LOGFONT    LogFontTemp;
							HFONT      hFontCurr;

                     _fmemset(&LogFontTemp, '\0', sizeof(LOGFONT));
					      hFontCurr = (HFONT)SelectObject(hDCPrinter, GetStockObject(SYSTEM_FONT));
                     GetObject(hFontCurr, sizeof(LOGFONT), &LogFontTemp);
							SelectObject(hDCPrinter, hFontCurr);
							/* RFW - 8/25/04 - 15076
                     LogFontTemp.lfHeight = -(int)(((double)LogFontTemp.lfHeight *
                                                  dfScalePreviewY));
							*/
                     LogFontTemp.lfHeight = (int)(((double)LogFontTemp.lfHeight *
                                                  dfScalePreviewY));
                     LogFontTemp.lfWidth = 0;
                     LogFontTemp.lfCharSet = DEFAULT_CHARSET;
                     hFontTemp = CreateFontIndirect(&LogFontTemp);
                     hFontOld = SelectObject(hDC, hFontTemp);

                     // If the font is not scaled small enough then
                     // have Windows choose any font of this size.

                     GetTextMetrics(hDC, &tm);

                     if (tm.tmAscent > abs(LogFontTemp.lfHeight))
                        {
                        LogFontTemp.lfPitchAndFamily = tm.tmPitchAndFamily & 0xF0;
                        if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH)
                           LogFont.lfPitchAndFamily &= ~FIXED_PITCH;
                        else
                           LogFont.lfPitchAndFamily |= FIXED_PITCH;

                        LogFontTemp.lfCharSet = tm.tmCharSet;
                        LogFontTemp.lfFaceName[0] = '\0';

                        SelectObject(hDC, hFontOld);
                        DeleteObject(hFontTemp);
                        hFontTemp = CreateFontIndirect(&LogFontTemp);
                        SelectObject(hDC, hFontTemp);
                        }
                     }

#ifdef SS_V80
				  if (bUseColor)
					  SetTextColor(hDC, Color);
#endif
                  TextOut(hDC, (int)((double)x * dfScalePreviewX),
                          (int)((double)y * dfScalePreviewY),
                          lpszString, Len);

                  if (hDCPrinter != hDC)
                     {
                     SelectObject(hDC, hFontOld);
                     DeleteObject(hFontTemp);
                     }
                  }

                  GetTextExtentPoint(hDCPrinter, lpszString, Len, &TextSize);
                  x += TextSize.cx;

                  GlobalUnlock(hString);

                  dJustOld = dJust;
                  }

               GlobalFree(hString);
               }

            GetTextExtentPoint(hDCPrinter, _T("X"), 1, &TextSize);
            HeightRow = max(HeightRow, TextSize.cy);
            }

         lpStart = Ptr;
         }

      if (fCreateFont)
         {
         if (hFontTemp = CreateFontIndirect(&LogFont))
            {
            SelectObject(hDCPrinter, hFontOrig);

            if (hFontCreated)
               DeleteObject(hFontCreated);

            hFontCreated = hFontTemp;
            hFontCurrent = hFontCreated;
            }
         }

      SelectObject(hDCPrinter, hFontCurrent);

      if (fDeleteFont)
         {
         fDeleteFont = FALSE;

         if (hFontCreated)
            {
            DeleteObject(hFontCreated);
            hFontCreated = 0;
            }
         }

      if (fNewLine)
         {
         y += HeightRow;
         HeightRow = Height;
         }

      fPrint = FALSE;
      fNewLine = FALSE;

      } while (Ptr);

   y += HeightRow;

   GlobalUnlock(hText);

   SelectObject(hDCPrinter, hFont[0]);

   if (hFontCreated)
      DeleteObject(hFontCreated);

   for (i = 1; i < 10; i++)
      if (hFont[i])
         DeleteObject(hFont[i]);
   }

return ((short)((double)y * dfScalePreviewY + 0.5));
}


GLOBALHANDLE vbSSCreatePrintString(LPTSTR lpStart, short Len, short nPageNum,
                                   long lPageCount, LPSS_PRINTINFO lpPrintInfo)
{
GLOBALHANDLE hString = 0;
LPTSTR       Ptr;
LPTSTR       lpszString;
LPTSTR       lpszStringPtr;
TCHAR        Buffer[20];
TCHAR        szPageCount[20];
short        NewLen = Len;
#ifdef SS_V70
TCHAR        szDateShort[64];
TCHAR        szDateLong[64];
TCHAR        szTimeShort[64];
TCHAR        szTimeLong[64];

szDateShort[0] = '\0';
szDateLong[0] = '\0';
szTimeShort[0] = '\0';
szTimeLong[0] = '\0';
#endif // SS_V70

wsprintf(Buffer, _T("%d"), nPageNum);
wsprintf(szPageCount, _T("%ld"), lPageCount);

Ptr = lpStart;
while (Ptr = StrChr(Ptr, '/'))
   {
   if (Ptr - lpStart >= Len)
      break;

   Ptr++;

   if (tolower(*Ptr) == 'p')
      {
      Ptr++;
      if (tolower(*Ptr) == 'c')
         {
         Ptr++;
         NewLen += lstrlen(szPageCount);
         }
      else
         NewLen += lstrlen(Buffer);
      }

#ifdef SS_V70
	else if (_tcsnicmp(Ptr, _T("date"), 4) == 0 || _tcsnicmp(Ptr, _T("ds"), 2) == 0 ||
            _tcsnicmp(Ptr, _T("dl"), 2) == 0)
		{
		LPSS_BOOK lpBook;

		lpBook = SS_BookLock(lpPrintInfo->hWndSpread);

		if (lpBook)
			{
			/* RFW - -8/5/04 - 15009/14443
			struct tm *ptmToday;
			time_t     lTime;

			_tzset();
			time(&lTime);
			ptmToday=localtime(&lTime);

			if (_tcsnicmp(Ptr, _T("dl"), 2) == 0)
				{
				_tcsftime(szDateLong, sizeof(szDateLong) - 1, _T("%#x"), ptmToday);
		      NewLen += lstrlen(szDateLong);
				}
			else
				{
				_tcsftime(szDateShort, sizeof(szDateShort) - 1, _T("%x"), ptmToday);
		      NewLen += lstrlen(szDateShort);
				}
			*/

			TB_DATE Date;
			double dfVal;
			struct tm *ptmToday;
			time_t     lTime;

			_tzset();
			time(&lTime);
			ptmToday=localtime(&lTime);

			SysGetDate (&Date.nDay, &Date.nMonth, &Date.nYear);

			dfVal = (double)(vbDateDMYToJulian(&Date) + 2);
			if (_tcsnicmp(Ptr, _T("dl"), 2) == 0)
				{
				if (!ssVBFormat((long)lpBook->lpOleControl, VT_R8, &dfVal, _T("Long Date"), szDateLong,
					             sizeof(szDateLong) - 1))
					_tcsftime(szDateLong, sizeof(szDateLong) - 1, _T("%#x"), ptmToday);
		      NewLen += lstrlen(szDateLong);
				}
			else
				{
				if (!ssVBFormat((long)lpBook->lpOleControl, VT_R8, &dfVal, _T("Short Date"), szDateShort,
					             sizeof(szDateShort) - 1))
					_tcsftime(szDateShort, sizeof(szDateShort) - 1, _T("%x"), ptmToday);
		      NewLen += lstrlen(szDateShort);
				}
			}

		if (_tcsnicmp(Ptr, _T("date"), 4) == 0)
			Ptr += 4;
		else
			Ptr += 2;

		SS_BookUnlock(lpPrintInfo->hWndSpread);
		}

	else if (_tcsnicmp(Ptr, _T("time"), 4) == 0 || _tcsnicmp(Ptr, _T("ts"), 2) == 0 ||
            _tcsnicmp(Ptr, _T("tl"), 2) == 0)
		{
		LPSS_BOOK lpBook;

		lpBook = SS_BookLock(lpPrintInfo->hWndSpread);

		if (lpBook)
			{
			/* RFW - -8/5/04 - 15009/14443
			struct tm *ptmToday;
			time_t     lTime;

			_tzset();
			time(&lTime);
			ptmToday=localtime(&lTime);

			if (_tcsnicmp(Ptr, _T("ts"), 2) == 0)
				{
				_tcsftime(szTimeShort, sizeof(szTimeShort) - 1, _T("%X"), ptmToday);
		      NewLen += lstrlen(szTimeShort);
				}
			else
				{
				_tcsftime(szTimeLong, sizeof(szTimeLong) - 1, _T("%X %p"), ptmToday);
		      NewLen += lstrlen(szTimeLong);
				}
			*/

			TIME Time;
			double dfVal;
			struct tm *ptmToday;
			time_t     lTime;

			_tzset();
			time(&lTime);
			ptmToday=localtime(&lTime);

	      SysGetTime (&Time.nHour, &Time.nMinute, &Time.nSecond);

         dfVal = (double)(((long)Time.nHour * 3600L) +
                          ((long)Time.nMinute * 60L) +
                           (long)Time.nSecond);
         dfVal = dfVal * 0.99999999 / 86399.0;
			if (_tcsnicmp(Ptr, _T("ts"), 2) == 0)
				{
				if (!ssVBFormat((long)lpBook->lpOleControl, VT_R8, &dfVal, _T("Short Time"), szTimeShort,
					             sizeof(szTimeShort) - 1))
					_tcsftime(szTimeShort, sizeof(szTimeShort) - 1, _T("%X"), ptmToday);
		      NewLen += lstrlen(szTimeShort);
				}
			else
				{
				if (!ssVBFormat((long)lpBook->lpOleControl, VT_R8, &dfVal, _T("Long Time"), szTimeLong,
					             sizeof(szTimeLong) - 1))
					_tcsftime(szTimeLong, sizeof(szTimeLong) - 1, _T("%X %p"), ptmToday);
		      NewLen += lstrlen(szTimeLong);
				}
			}

		if (_tcsnicmp(Ptr, _T("time"), 4) == 0)
			Ptr += 4;
		else
			Ptr += 2;

		SS_BookUnlock(lpPrintInfo->hWndSpread);
		}

/*
		{
		TCHAR szFormat[64];
		TB_DATE Date;

		GetProfileString(_T("Intl"), _T("sShortDate"), "M/d/yyyy", szFormat,
                       sizeof(szFormat));
		SysGetDate (&Date.nDay, &Date.nMonth, &Date.nYear);
		DateWinFormat(szTime, &Date, szFormat, NULL);
      NewLen += lstrlen(szTime);

		Ptr += 4;
		}
*/
#endif // SS_V70

   else if (*Ptr == '/')
      {
      NewLen--;
      Ptr++;
      }
   }

if (NewLen && (hString = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                     (NewLen + 1) * sizeof(TCHAR))))
   {
   lpszString = (LPTSTR)GlobalLock(hString);

   for (Ptr = lpStart, lpszStringPtr = lpszString; Ptr < lpStart + Len; )
      {
      if (*Ptr == '/' && tolower(*(Ptr + 1)) == 'p')
         {
         Ptr++;
         if (tolower(*(Ptr + 1)) == 'c')
            {
            Ptr++;
            lstrcpy(lpszStringPtr, szPageCount);
            lpszStringPtr += lstrlen(szPageCount);
            }
         else
            {
            lstrcpy(lpszStringPtr, Buffer);
            lpszStringPtr += lstrlen(Buffer);
            }
         }

#ifdef SS_V70
		else if (*Ptr == '/' && _tcsnicmp(Ptr + 1, _T("ds"), 2) == 0)
			{
         lstrcpy(lpszStringPtr, szDateShort);
			lpszStringPtr += lstrlen(szDateShort);
			Ptr += 2;
			}

		else if (*Ptr == '/' && _tcsnicmp(Ptr + 1, _T("dl"), 2) == 0)
			{
         lstrcpy(lpszStringPtr, szDateLong);
			lpszStringPtr += lstrlen(szDateLong);
			Ptr += 2;
			}

		else if (*Ptr == '/' && _tcsnicmp(Ptr + 1, _T("date"), 4) == 0)
			{
         lstrcpy(lpszStringPtr, szDateShort);
			lpszStringPtr += lstrlen(szDateShort);
			Ptr += 4;
			}

		else if (*Ptr == '/' && _tcsnicmp(Ptr + 1, _T("ts"), 2) == 0)
			{
         lstrcpy(lpszStringPtr, szTimeShort);
			lpszStringPtr += lstrlen(szTimeShort);
			Ptr += 2;
			}

		else if (*Ptr == '/' && _tcsnicmp(Ptr + 1, _T("tl"), 2) == 0)
			{
         lstrcpy(lpszStringPtr, szTimeLong);
			lpszStringPtr += lstrlen(szTimeLong);
			Ptr += 2;
			}

		else if (*Ptr == '/' && _tcsnicmp(Ptr + 1, _T("time"), 4) == 0)
			{
         lstrcpy(lpszStringPtr, szTimeLong);
			lpszStringPtr += lstrlen(szTimeLong);
			Ptr += 4;
			}

#endif // SS_V70

      else if (*Ptr == '/' && *(Ptr + 1) == '/')
         {
         *lpszStringPtr = '/';
         lpszStringPtr++;
         Ptr++;
         }

      else
         {
         *lpszStringPtr = *Ptr;
         lpszStringPtr++;
         }

      Ptr++;
      }

   GlobalUnlock(hString);
   }

return (hString);
}


DLGENTRY vbSSPrintAbortProc(HDC hDCPrn, int nCode)
{
PRINTABORTTABLE PrintAbortItem;
MSG             Msg;

while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
   {
   if (vbSSGetPrintAbortItem(&PrintAbortItem, hDCPrn, 0))
      {
      if (!IsDialogMessage(PrintAbortItem.hWndDlg, &Msg))
         {
         TranslateMessage(&Msg);
         DispatchMessage(&Msg);
         }

      if (!PrintAbortItem.fDisabled)
         {
#if !defined(WIN32) && defined(SS_OCX)
         if (!PrintAbortItem.fAbort)
#endif
         SendMessage(PrintAbortItem.hWndSpread, SSM_FIREEVENTPRINTABORT, 0,
                     (long)(BOOL FAR*)&PrintAbortItem.fAbort);

         if (PrintAbortItem.fAbort)
            {
            ShowWindow(PrintAbortItem.hWndDlg, SW_HIDE);
            vbSSDisablePrintAbortItem(PrintAbortItem.hWndDlg);
            vbSSCancelPrintAbortItem(PrintAbortItem.hWndDlg);  // BJO 22Jul97 JAP7507 - Added
//            DestroyWindow(PrintAbortItem.hWndDlg);
//            vbSSDeletePrintAbortItem(PrintAbortItem.hWndDlg);
            }
         }

      if (PrintAbortItem.fAbort)
         return (FALSE);
      }

   else
      {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
      }
   }

return (TRUE);
}


BOOL vbSSGetPrintAbortItem(lpPrintAbortItem, hDC, hWndDlg)

LPPRINTABORTTABLE lpPrintAbortItem;
HDC               hDC;
HWND              hWndDlg;
{
short             i;

for (i = 0; i < PrintAbortTableCnt; i++)
   if ((hDC && PrintAbortTable[i].hDC == hDC) ||
       (hWndDlg && PrintAbortTable[i].hWndDlg == hWndDlg))
      {
      _fmemcpy(lpPrintAbortItem, &PrintAbortTable[i], sizeof(PRINTABORTTABLE));
      return (TRUE);
      }

return (FALSE);
}


void vbSSCancelPrintAbortItem(hWndDlg)

HWND  hWndDlg;
{
short i;

for (i = 0; i < PrintAbortTableCnt; i++)
   if (PrintAbortTable[i].hWndDlg == hWndDlg)
      PrintAbortTable[i].fAbort = TRUE;
}


void vbSSDisablePrintAbortItem(HWND hWndDlg)
{
short i;

for (i = 0; i < PrintAbortTableCnt; i++)
   if (PrintAbortTable[i].hWndDlg == hWndDlg)
      PrintAbortTable[i].fDisabled = TRUE;
}


void vbSSDeletePrintAbortItem(hWndDlg)

HWND  hWndDlg;
{
short i;
short j;

for (i = 0; i < PrintAbortTableCnt; i++)
   if (PrintAbortTable[i].hWndDlg == hWndDlg)
      {
      for (j = i; j < PrintAbortTableCnt - 1; j++)
         _fmemcpy(&PrintAbortTable[j] , &PrintAbortTable[j + 1],
                  sizeof(PRINTABORTTABLE));

      PrintAbortTableCnt--;
      }

}


void vbSSReplacePrintAbortItem(HWND hWndSpread, HDC hOldDC, HDC hNewDC)
{
short i;

for (i = 0; i < PrintAbortTableCnt; i++)
   if (PrintAbortTable[i].hWndSpread == hWndSpread
       && PrintAbortTable[i].hDC == hOldDC)
      {
      PrintAbortTable[i].hDC = hNewDC;
      return;
      }
}


void vbSSAddPrintAbortItem(hWndDlg, hDC)

HWND hWndDlg;
HDC  hDC;
{
short i;

for (i = 0; i < PrintAbortTableCnt; i++)
   if (PrintAbortTable[i].hWndDlg == hWndDlg)
      {
      PrintAbortTable[i].hDC = hDC;
      return;
      }

if (PrintAbortTableCnt < PRINTABORTTABLE_MAX)
   {
   PrintAbortTable[PrintAbortTableCnt].hDC = hDC;
   PrintAbortTable[PrintAbortTableCnt].hWndDlg = hWndDlg;
   PrintAbortTable[PrintAbortTableCnt].hWndSpread = GetProp(hWndDlg,
                                                            _T("hWndSpread"));
   PrintAbortTable[PrintAbortTableCnt].fAbort = FALSE;
   PrintAbortTable[PrintAbortTableCnt].fDisabled = FALSE;
   PrintAbortTableCnt++;
   }
}


DLGENTRY vbSSPrintDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
PAINTSTRUCT       Paint;
LPPRINTABORTPARAM lpPrintAbortParam;
TBGLOBALHANDLE    hAbortMsg;
TBGLOBALHANDLE    hAbortMsgTemp;
LPTSTR            lpszAbortMsg;
LPTSTR            lpszAbortMsgTemp;
LPTSTR            PtrOld;
LPTSTR            Ptr;
RECT              Rect;
RECT              RectClient;
RECT              RectButton;
HDC               hDC;
short             dRowCnt = 0;
int               dWidth = 0;
int               dHeight;
int               dHeightClient;
int               BkModeOld;
SIZE              TextSize;

switch (uMsg)
   {
   case WM_INITDIALOG:
      EnableMenuItem(GetSystemMenu(hWnd, FALSE), SC_CLOSE, MF_GRAYED);

      /***************************
      * Calculate text area size
      ***************************/

      if (lParam)
         {
         lpPrintAbortParam = (LPPRINTABORTPARAM)lParam;
         hAbortMsg = lpPrintAbortParam->hAbortMsg;
         SetProp(hWnd, _T("AbortMsg"), (HANDLE)hAbortMsg);
         SetProp(hWnd, _T("hWndSpread"), lpPrintAbortParam->hWndSpread);

         lpszAbortMsg = (LPTSTR)tbGlobalLock(hAbortMsg);

         hDC = fpGetDC(hWnd);

         Ptr = lpszAbortMsg;
         PtrOld = Ptr;
         dRowCnt = 1;
         while (Ptr = StrChr(Ptr, '/'))
            {
            Ptr++;

            if (tolower(*Ptr) == 'n')
               {
               //dWidth = max(dWidth, (short)LOWORD(GetTextExtent(hDC, PtrOld,
               //             Ptr - PtrOld - 1)));
               GetTextExtentPoint(hDC, PtrOld, Ptr - PtrOld - 1, &TextSize);
               dWidth = max(dWidth, TextSize.cx);
               dRowCnt++;
               Ptr++;
               PtrOld = Ptr;
               }
            }

         GetTextExtentPoint(hDC, PtrOld, lstrlen(PtrOld), &TextSize);
         dWidth = max(dWidth, TextSize.cx);

         GetTextExtentPoint(hDC, _T("1"), 1, &TextSize);
         dHeight = TextSize.cy;
         ReleaseDC(hWnd, hDC);

         GetWindowRect(GetDlgItem(hWnd, IDOK), &RectButton);
         GetWindowRect(hWnd, &Rect);
         GetClientRect(hWnd, &RectClient);

         dWidth = max(dWidth, RectButton.right - RectButton.left) +
                      (PRINTABORTDLG_HMARGIN * 2) + (Rect.right -
                      Rect.left - RectClient.right);

         dHeightClient = PRINTABORTDLG_VMARGIN1 + (dHeight * dRowCnt) +
                         PRINTABORTDLG_VMARGIN2 + (RectButton.bottom -
                         RectButton.top) + PRINTABORTDLG_VMARGIN3;

         dHeight = dHeightClient + (Rect.bottom - Rect.top -
                   RectClient.bottom);

         SetWindowPos(hWnd, NULL, 0, 0, dWidth, dHeight,
                      SWP_NOMOVE | SWP_NOZORDER);

         GetClientRect(hWnd, &RectClient);
         dWidth = RectClient.right - (RectButton.right - RectButton.left);
         dHeight = RectClient.bottom - (RectButton.bottom - RectButton.top);

         SetWindowPos(GetDlgItem(hWnd, IDOK), NULL, dWidth / 2,
                      dHeightClient - PRINTABORTDLG_VMARGIN3 -
                      (RectButton.bottom - RectButton.top), 0, 0,
                      SWP_NOSIZE | SWP_NOZORDER);

         if (!SendMessage(lpPrintAbortParam->hWndSpread,
                          SSM_FIREEVENTPRINTMSGOPEN, (WPARAM)hWnd, (LPARAM)lpszAbortMsg))
            {
            DlgBoxCenter(hWnd);
            ShowWindow(hWnd, SW_SHOWNORMAL);
            UpdateWindow(hWnd);
            }
           

         tbGlobalUnlock(hAbortMsg);
         }

      return (0);

   case WM_PAINT:
      hDC = fpBeginPaint(hWnd, (LPPAINTSTRUCT)&Paint);
      GetClientRect(hWnd, &RectClient);

      GetTextExtentPoint(hDC, _T("1"), 1, &TextSize);
      dHeight = TextSize.cy;

      hAbortMsg = (TBGLOBALHANDLE)GetProp(hWnd, _T("AbortMsg"));
      lpszAbortMsg = (LPTSTR)tbGlobalLock(hAbortMsg);

      BkModeOld = SetBkMode(hDC, TRANSPARENT);

      if (hAbortMsgTemp = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                        (lstrlen(lpszAbortMsg) + 1) * sizeof(TCHAR)))
         {
         lpszAbortMsgTemp = (LPTSTR)tbGlobalLock(hAbortMsgTemp);
         lstrcpy(lpszAbortMsgTemp, lpszAbortMsg);

         Ptr = lpszAbortMsgTemp;
         PtrOld = Ptr;
         while (Ptr = StrChr(Ptr, '/'))
            {
            Ptr++;

            if (tolower(*Ptr) == 'n')
               {
               //dWidth = LOWORD(GetTextExtent(hDC, PtrOld, Ptr - PtrOld - 1));
               GetTextExtentPoint(hDC, PtrOld, Ptr - PtrOld - 1, &TextSize);
               dWidth = TextSize.cx;
               TextOut(hDC, (RectClient.right - dWidth) / 2,
                       PRINTABORTDLG_VMARGIN1 + (dRowCnt * dHeight), PtrOld,
                       Ptr - PtrOld - 1);
               dRowCnt++;
               Ptr++;
               PtrOld = Ptr;
               }

            else if (tolower(*Ptr) == '/')
               lstrcpy(Ptr, Ptr + 1);
            }

         //dWidth = LOWORD(GetTextExtent(hDC, PtrOld, lstrlen(PtrOld)));
         GetTextExtentPoint(hDC, PtrOld, lstrlen(PtrOld), &TextSize);
         dWidth = TextSize.cx;
         TextOut(hDC, (RectClient.right - dWidth) / 2, PRINTABORTDLG_VMARGIN1 +
                 (dRowCnt * dHeight), PtrOld, lstrlen(PtrOld));

         tbGlobalUnlock(hAbortMsgTemp);
         tbGlobalFree(hAbortMsgTemp);
         }

      SetBkMode(hDC, BkModeOld);

      tbGlobalUnlock(hAbortMsg);

      EndPaint(hWnd, &Paint);
      return (0);

   case WM_DESTROY:
      hAbortMsg = (TBGLOBALHANDLE)GetProp(hWnd, _T("AbortMsg"));
      lpszAbortMsg = (LPTSTR)tbGlobalLock(hAbortMsg);
      SendMessage(GetProp(hWnd, _T("hWndSpread")), SSM_FIREEVENTPRINTMSGCLOSE,
                  (WPARAM)hWnd, (LPARAM)lpszAbortMsg);
      tbGlobalUnlock(hAbortMsg);

      RemoveProp(hWnd, _T("AbortMsg"));
      RemoveProp(hWnd, _T("hWndSpread"));
      break;

   case WM_COMMAND:
      ShowWindow(hWnd, SW_HIDE);
      vbSSCancelPrintAbortItem(hWnd);
      return (TRUE);
   }

return (FALSE);
}

