/*********************************************************
* XLloadl.cpp
*
* Copyright (C) 1999 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* UPDATE LOG:
* -----------
*	RAP	- 11.19.98
* RAP01 - GIC9513 - Add support for LABEL records                    9.13.99             
* RAP02 - gic10940 - date celltype not using regional settings.      12.10.99
* RAP03 - GIC12259 - Add support for '@' format                      09.11.00
* RAP04 - 8832 - When importing strings, if 0xa(linefeed) occurs, 
*                it needs to be preceded by a 0xd(carriage return).  05.23.01
* RAP05 - 9007 - col width not correct when applied from SD          06.06.01
* RAP06 - Changes for UNICODE build                                  10.04.01
* RAP07 - changes to loading ROW and COLINFO to set celltype info    1.9.02 
*********************************************************/

#if defined(_WIN64) || defined(_IA64)
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "tchar.h"
#include "ctype.h"
#include "io.h"
#include "math.h"
#include <stdio.h>
#include "windows.h"

#include "spread.h"
#include "xl.h"
#include "xlbiff.h"
#include "xlutl.h"
#include "biff.h"
#include "ss_alloc.h"
#include "ss_main.h"

extern "C" SS_FONTID SS_InitFont(LPSS_BOOK lpBook, HFONT hFont, BOOL fDeleteFont,
                                 BOOL fCreateFont, LPBOOL lpfDeleteFontObject);
LPSS_CELLTYPE DLLENTRY SS_XlSetTypeEdit(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style, int Len,
                                        short ChrSet, short ChrCase);


void DisplayError(HRESULT hr, LPTSTR lptstr);

BOOL FP_API bif_GetXF(TBGLOBALHANDLE gh, WORD wIndex, LPxXF8 lpxf)
{
  LPxXF8 lpxfS = (LPxXF8)tbGlobalLock(gh);
  BOOL   Ret = FALSE;

  memcpy(lpxf, &(lpxfS[wIndex]), sizeof(xXF8));

  tbGlobalUnlock(gh);

  return Ret;
}

BOOL bif_GetFONT(TBGLOBALHANDLE gh, WORD wIndex, LPxFONT lpfont)
{
  LPxFONT lpfontS = (LPxFONT)tbGlobalLock(gh);
  BOOL  Ret = FALSE;

  if (wIndex >= 0x04) //the font index 0x04 is not used in Excel BIFF files.
    wIndex--;

  memcpy(lpfont, &(lpfontS[wIndex]), sizeof(xFONT));

  tbGlobalUnlock(gh);

  return Ret;
}

BOOL FP_API bif_GetFont(TBGLOBALHANDLE gh, WORD wIndex, LPHFONT lphf, LPCOLORREF lpColor)
{
  HDC      hDC = GetDC(NULL);
  xFONT    font;
  LPBYTE   lpb;
  TCHAR    tcFontName[LF_FACESIZE];
  long     lHeight;
  BOOL     Ret = FALSE;

  memset(tcFontName, 0, LF_FACESIZE*sizeof(TCHAR));

  bif_GetFONT(gh, wIndex, (LPxFONT)&font);
  
  *lpColor = xl_ColorFromIndex(font.wicv);
  lpb = (LPBYTE)tbGlobalLock(font.ghFontName); 
  if (font.bStrType) // the grbit is true == highbyte
  {
#ifndef _UNICODE
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)lpb, -1, tcFontName,
                        LF_FACESIZE, NULL, NULL);
#else
    memcpy(tcFontName, lpb, font.bFontNameLen*sizeof(WCHAR));
#endif
  }
  else
  {
#ifndef _UNICODE
    memcpy(tcFontName, lpb, font.bFontNameLen);
#else
#endif
  }

  tbGlobalUnlock(font.ghFontName);

  lHeight = -MulDiv((font.wHeight + (font.wHeight%20?20:0))/20, GetDeviceCaps(hDC, LOGPIXELSY), 72);

  *lphf = CreateFont(lHeight,
                     0,            //width
                     0,            //escapement
                     0,            //orientation
                     font.wBoldStyle, //weight
                     font.fItalic, //italic
                     font.bUls?TRUE:FALSE, //underline
                     font.fStrikeout,  //strikeout
                       font.bCharSet, //charset
//                    OUT_TT_PRECIS, //output precision
                     OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS,  //clipping precision
                     DEFAULT_QUALITY,      //quality
                     DEFAULT_PITCH | FF_DONTCARE,//font.bFamily,  //pitch & family
                     tcFontName);

  ReleaseDC(NULL, hDC);

  return Ret;
}

BOOL bif_GetFORMAT(LPSSXL lpssXL, LPSPREADSHEET lpSS, WORD wIndex, LPTSTR lptstr, LPWORD lpwType, LPFMT FAR *plpf, LPVOID lpData)
{
  BOOL Ret = FALSE;
  LPFMT lpf = *plpf;

#ifdef SS_V40
  lpf->bLeadingZero = 1;
#endif

//20157 >>
#ifdef SS_V40
//    if (*lpwType == NUMBERTYPE || *lpwType == PERCENTTYPE || *lpwType == CURRENCYTYPE)
  {  
    lpf->Min = -999999999999.99;
    lpf->Max = 999999999999.99;
  }
#endif
#ifdef SS_V70
  if (*lpwType == SCIENTIFICTYPE)
  {
    lpf->Min = -1.7E308;
    lpf->Max = 1.7E308;
  }
#endif
//<< 20157

#ifdef SPREAD_JPN // for far east version of Excel
  if (wIndex <= 0x40)
#else
  if (wIndex <= 0x31)
#endif
  {
    switch(wIndex)
    {
      //XF: Extended Format internally saved formats
      //Index to internal format (ifmt)
#ifdef SS_V40
      case 0x00:
        lstrcpy(lptstr, XF00h);
        if (lpData)
        {
          lpf->bLeadingZero = 2;
          lpf->Right = 5;
          *lpwType = NUMBERTYPE;
        }
        else
          *lpwType = 0;
      break;
      case 0x01:
        lpf->Right = 0;
        *lpwType = NUMBERTYPE;
        lstrcpy(lptstr, XF01h);
      break;
      case 0x02:
        lpf->Right = 2;
        lpf->bLeadingZero = 2;
        *lpwType = NUMBERTYPE;
        lstrcpy(lptstr, XF02h);
      break;
      case 0x03:
        lpf->Right = 0;
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->currencyfmt.szCurrency, _T("\\"));
        *lpwType = CURRENCYTYPE;
#else
        *lpwType = NUMBERTYPE;
#endif
        lstrcpy(lptstr, XF03h);
      break;
      case 0x04:
        lpf->Right = 2;

        lpf->fShowSeparator = TRUE;
        lpf->bLeadingZero = 2;
#ifdef SPREAD_JPN
        lstrcpy(lpf->currencyfmt.szCurrency, _T("\\"));
        *lpwType = CURRENCYTYPE;
#else
        *lpwType = NUMBERTYPE;
#endif
        lstrcpy(lptstr, XF04h);
      break;
      case 0x05:
        lpf->Right = 0;
        lpf->fShowSeparator = TRUE;
        lpf->bLeadingZero = 2;
        lpf->bNegStyle = 1;
#ifdef SPREAD_JPN
        lstrcpy(lpf->currencyfmt.szCurrency, _T("\\"));
        lpf->bNegStyle = 14;
        lpf->currencyfmt.bPosStyle = 4;
#endif
        *lpwType = CURRENCYTYPE;
        lstrcpy(lptstr, XF05h);
      break;
      case 0x06:
        lpf->Right = 0;
        lpf->fShowSeparator = TRUE;
        lpf->bLeadingZero = 2;
        lpf->bNegStyle = 1;
        lpf->fNegRed = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->currencyfmt.szCurrency, _T("\\"));
        lpf->bNegStyle = 14;
        lpf->currencyfmt.bPosStyle = 4;
#endif
        *lpwType = CURRENCYTYPE;
        lstrcpy(lptstr, XF06h);
      break;
      case 0x07:
        lpf->Right = 2;
        lpf->fShowSeparator = TRUE;
        lpf->bLeadingZero = 2;
        lpf->bNegStyle = 1;
#ifdef SPREAD_JPN
        lstrcpy(lpf->currencyfmt.szCurrency, _T("\\"));
        lpf->bNegStyle = 14;
        lpf->currencyfmt.bPosStyle = 4;
#endif
        *lpwType = CURRENCYTYPE;
        lstrcpy(lptstr, XF07h);
      break;
      case 0x08:
        lpf->Right = 2;
        lpf->fShowSeparator = TRUE;
        lpf->bLeadingZero = 2;
        lpf->bNegStyle = 1;
        lpf->fNegRed = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->currencyfmt.szCurrency, _T("\\"));
        lpf->bNegStyle = 14;
        lpf->currencyfmt.bPosStyle = 4;
#endif
        *lpwType = CURRENCYTYPE;
        lstrcpy(lptstr, XF08h);
      break;
      case 0x09:
        *lpwType = PERCENTTYPE;
        lpf->Right = 0;
        lpf->bLeadingZero = 2;
      break;
      case 0x0a:
        *lpwType = PERCENTTYPE;
        lpf->Right = 2;
        lpf->bLeadingZero = 2;
      break;
#ifdef SS_V70
      case 0x0b:
        *lpwType = SCIENTIFICTYPE;
        lpf->Right = 2;
      break;
      case 0x30:
        *lpwType = SCIENTIFICTYPE;
        lpf->Right = 1;
      break;
#else
      case 0x0b:
      case 0x30:
        *lpwType = NUMBERTYPE;
        lpf->Right = 2;
        xl_LogFile(lpSS->lpBook, LOG_FORMATSCIENTIFIC, 0, 0, _T("0.00E+00"));
        lstrcpy(lptstr, XF0bh);
      break;
#endif
      case 0x0c:
        *lpwType = NUMBERTYPE;
        lpf->Right = 5;
        xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, _T("#\\ ?/?"));
        lstrcpy(lptstr, XF0ch);
      break;
      case 0x0d:
        *lpwType = NUMBERTYPE;
        lpf->Right = 9;
        xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, _T("#\\ ??/??"));
        lstrcpy(lptstr, XF0dh);
      break;

      case 0x17:
        lpf->Right = 0;
        lpf->fShowSeparator = TRUE;
        lpf->bLeadingZero = 2;
        lpf->bNegStyle = 1;
#ifdef SPREAD_JPN
//        lstrcpy(lpf->currencyfmt.szCurrency, _T("\\"));
        lstrcpy(lpf->currencyfmt.szCurrency, _T("$"));
#endif
        *lpwType = CURRENCYTYPE;
        lstrcpy(lptstr, XF05h);
      break;
      case 0x18:
        lpf->Right = 0;
        lpf->fShowSeparator = TRUE;
        lpf->bLeadingZero = 2;
        lpf->bNegStyle = 1;
        lpf->fNegRed = TRUE;
#ifdef SPREAD_JPN
//        lstrcpy(lpf->currencyfmt.szCurrency, _T("\\"));
        lstrcpy(lpf->currencyfmt.szCurrency, _T("$"));
#endif
        *lpwType = CURRENCYTYPE;
        lstrcpy(lptstr, XF06h);
      break;

#else //!SS_V40

      case 0x00:
        lstrcpy(lptstr, XF00h);
        if (lpData)
        {
          lpf->floatfmt.nDigitsLeft = 9;
          lpf->floatfmt.nDigitsRight = 5;
          lpf->floatfmt.fDigitLimit = TRUE;
          *lpwType = FLOATTYPE;
        }
        else
          *lpwType = 0;
      break;
      case 0x01:
        lpf->floatfmt.nDigitsLeft = 14;
        lpf->floatfmt.nDigitsRight = 0;
        lpf->floatfmt.fDigitLimit = TRUE;
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF01h);
      break;
      case 0x02:
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF02h);
      break;
      case 0x03:
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.nDigitsLeft = 14;
        lpf->floatfmt.fSeparator = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->floatfmt.szCurrency, _T("\\"));
#endif
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF03h);
      break;
      case 0x04:
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->floatfmt.szCurrency, _T("\\"));
#endif
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF04h);
      break;
      case 0x05:
        lpf->floatfmt.nDigitsLeft = 14;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->floatfmt.szCurrency, _T("\\"));
#endif
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF05h);
      break;
      case 0x06:
        lpf->floatfmt.nDigitsLeft = 14;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->floatfmt.szCurrency, _T("\\"));
#endif
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF06h);
      break;
      case 0x07:
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->floatfmt.szCurrency, _T("\\"));
#endif
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF07h);
      break;
      case 0x08:
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
#ifdef SPREAD_JPN
        lstrcpy(lpf->floatfmt.szCurrency, _T("\\"));
#endif
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF08h);
      break;
      case 0x09:
        *lpwType = FLOATTYPE;
        lpf->floatfmt.nDigitsLeft = 9;
        lpf->floatfmt.nDigitsRight = 5;
        lpf->floatfmt.fDigitLimit = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATPERCENT, 0, 0, _T("0%"));
        lstrcpy(lptstr, XF09h);
      break;
      case 0x0a:
        *lpwType = FLOATTYPE;
        lpf->floatfmt.nDigitsLeft = 9;
        lpf->floatfmt.nDigitsRight = 5;
        lpf->floatfmt.fDigitLimit = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATPERCENT, 0, 0, _T("0.00%"));
        lstrcpy(lptstr, XF0ah);
      break;
      case 0x0b:
      case 0x30:
        *lpwType = FLOATTYPE;
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATSCIENTIFIC, 0, 0, _T("0.00E+00"));
        lstrcpy(lptstr, XF0bh);
        break;
      case 0x0c:
        *lpwType = FLOATTYPE;
        lpf->floatfmt.nDigitsLeft = 9;
        lpf->floatfmt.nDigitsRight = 5;
        lpf->floatfmt.fDigitLimit = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, _T("#\\ ?/?"));
        lstrcpy(lptstr, XF0ch);
      break;
      case 0x0d:
        *lpwType = FLOATTYPE;
        lpf->floatfmt.nDigitsLeft = 5;
        lpf->floatfmt.nDigitsRight = 9;
        lpf->floatfmt.fDigitLimit = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, _T("#\\ ??/??"));
        lstrcpy(lptstr, XF0dh);
      break;
#endif
      case 0x0e:
        lpf->datefmt.bCentury = TRUE;
        lpf->datefmt.cSeparator = (TCHAR)'/';
        lpf->datefmt.bSpin = FALSE;
#ifdef SPREAD_JPN
        lpf->datefmt.nFormat = IDF_YYMMDD;
#else
        lpf->datefmt.nFormat = IDF_MMDDYY;
#endif
        *lpwType = DATETYPE;
        lstrcpy(lptstr, XF0eh);
      break;
      case 0x0f:
        lpf->datefmt.bCentury = FALSE;
        lpf->datefmt.cSeparator = (TCHAR)'-';
        lpf->datefmt.nFormat = IDF_DDMONYY;
        lpf->datefmt.bSpin = FALSE;
        *lpwType = DATETYPE;
        lstrcpy(lptstr, XF0fh);
      break;
      case 0x10:  
        lpf->datefmt.bCentury = FALSE;
        lpf->datefmt.cSeparator = (TCHAR)'-';
//#ifdef SPREAD_JPN //xxrap see if this format is supported in Spread
        lpf->datefmt.nFormat = IDF_MMDD;
        lpf->datefmt.bSpin = FALSE;
        *lpwType = DATETYPE;
        lstrcpy(lptstr, XF10h);
      break;
      case 0x11:
        lpf->datefmt.bCentury = FALSE;
        lpf->datefmt.cSeparator = (TCHAR)'-';
//#ifdef SPREAD_JPN//xxrap see if this format is supported in Spread
        lpf->datefmt.nFormat = IDF_YYMM;
        lpf->datefmt.bSpin = FALSE;
        *lpwType = DATETYPE;
        lstrcpy(lptstr, XF11h);
      break;
      case 0x12:
        lpf->timefmt.cSeparator = (TCHAR)':';
        *lpwType = TIMETYPE;
        lstrcpy(lptstr, XF12h);
      break;
      case 0x13:
        lpf->timefmt.bSeconds = TRUE;
        lpf->timefmt.cSeparator = (TCHAR)':';
        *lpwType = TIMETYPE;
        lstrcpy(lptstr, XF13h);
      break;
      case 0x14:
        lpf->timefmt.b24Hour = TRUE;
        lpf->timefmt.cSeparator = (TCHAR)':';
        *lpwType = TIMETYPE;
        lstrcpy(lptstr, XF14h);
      break;
      case 0x15:
        lpf->timefmt.b24Hour = TRUE;
        lpf->timefmt.bSeconds = TRUE;
        lpf->timefmt.cSeparator = (TCHAR)':';
        *lpwType = TIMETYPE;
        lstrcpy(lptstr, XF15h);
      break;
      case 0x16:
        lpf->datefmt.bCentury = FALSE;
        lpf->datefmt.cSeparator = (TCHAR)'/';
        lpf->datefmt.nFormat = IDF_MMDDYY;
        lpf->datefmt.bSpin = FALSE;
        *lpwType = DATETYPE;
        lstrcpy(lptstr, XF16h);
      break;
      //Add by BOC 99.5.25 (hyt)
	  //for support far east excel version
//#ifdef SPREAD_JPN
	    case 0x19:
	    case 0x1a:
#ifdef SS_V40
        lpf->Right = 2;
        lpf->fShowSeparator = TRUE;
        lpf->fNegRed = TRUE;
        lpf->bNegStyle = 1;
		    lstrcpy(lpf->currencyfmt.szCurrency, _T("$"));
        *lpwType = CURRENCYTYPE;
#else
	      lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
#endif
		    if(wIndex==0x19)
          lstrcpy(lptstr, XF19h);
    		else
          lstrcpy(lptstr, XF1ah);
  		break;
	    case 0x1e:
		    lpf->datefmt.bCentury = FALSE;
        lpf->datefmt.cSeparator = (TCHAR)'/';
        lpf->datefmt.nFormat = IDF_MMDDYY;
        lpf->datefmt.bSpin = FALSE;
        *lpwType = DATETYPE;
        lstrcpy(lptstr, XF1eh);
	    break;
	    case 0x1f:
		    lpf->datefmt.bCentury = TRUE;
        lpf->datefmt.cSeparator = (TCHAR)'?';
        lpf->datefmt.nFormat = IDF_YYMMDD;
        lpf->datefmt.bSpin = FALSE;
        *lpwType = DATETYPE;
        lstrcpy(lptstr, XF1fh);
	    break;
	    case 0x20:
		    lpf->timefmt.cSeparator = (TCHAR)'?';
  		  lpf->timefmt.b24Hour = TRUE;
  	  	lpf->timefmt.bSeconds = FALSE;
        *lpwType = TIMETYPE;
        lstrcpy(lptstr, XF20h);
	    break;
	    case 0x21:
  		  lpf->timefmt.cSeparator = (TCHAR)'?';
  	  	lpf->timefmt.b24Hour = TRUE;
	  	  lpf->timefmt.bSeconds = TRUE;
        *lpwType = TIMETYPE;
        lstrcpy(lptstr, XF21h);
  	  break;
//#endif //SPREAD_JPN
  	  case 0x25:
#ifdef SS_V40
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
        lpf->Right = 0;
        *lpwType = NUMBERTYPE;
  #ifdef SPREAD_JPN
        lpf->bNegStyle = 2;
  #else
        lpf->bNegStyle = 1;
  #endif
#else
        lpf->floatfmt.nDigitsLeft = 14;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
#endif
        lstrcpy(lptstr, XF25h);
      break;
      case 0x26:
#ifdef SS_V40
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
        lpf->Right = 0;
        lpf->fNegRed = TRUE;
        *lpwType = NUMBERTYPE;
  #ifdef SPREAD_JPN
        lpf->bNegStyle = 2;
  #else
        lpf->bNegStyle = 1;
  #endif
#else
        lpf->floatfmt.nDigitsLeft = 14;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, XF26h);
#endif
      break;
      case 0x27:
#ifdef SS_V40
        lpf->Right = 2;
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
        *lpwType = NUMBERTYPE;
#ifdef SPREAD_JPN
        lpf->bNegStyle = 2;
#else
        lpf->bNegStyle = 1;
#endif
#else
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
#endif
        lstrcpy(lptstr, XF27h);
      break;
      case 0x28:
#ifdef SS_V40
        lpf->Right = 2;
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
        lpf->fNegRed = TRUE;
        *lpwType = NUMBERTYPE;
#ifdef SPREAD_JPN
        lpf->bNegStyle = 2;
#else
        lpf->bNegStyle = 1;
#endif
#else
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
#endif
        lstrcpy(lptstr, XF28h);
      break;


//!!!RAP - 0x29-0x2c --> "_" as first char in defined string...
/*
      case 0x29:
        *lpwType = FLOATTYPE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, &wIndex);
        lstrcpy(lptstr, "XF29h");
      break;
      case 0x2a:
        *lpwType = FLOATTYPE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, &wIndex);
        lstrcpy(lptstr, "XF2ah");
      break;
      case 0x2b:
        *lpwType = FLOATTYPE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, &wIndex);
        lstrcpy(lptstr, "XF2bh");
      break;
      case 0x2c:
        lpf->floatfmt.nDigitsLeft = 4;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, "XF2ch");
      break;
*/
#ifdef SS_V40
      case 0x29:
        lpf->Right = 0;
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
        *lpwType = NUMBERTYPE;
#ifdef SPREAD_JPN
        lpf->bNegStyle = 4;
#else
        lpf->bNegStyle = 1;
#endif
        lstrcpy(lptstr, _T("XF2ch"));
      break;
      case 0x2a:
        lpf->Right = 0;
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
#ifdef SPREAD_JPN
        lpf->currencyfmt.bPosStyle = 4;
        lpf->bNegStyle = 14;
#else
        lpf->bNegStyle = 15;
#endif
        *lpwType = CURRENCYTYPE;
        lstrcpy(lptstr, _T("XF2ch"));
      break;
      case 0x2b:
        lpf->Right = 2;
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
#ifdef SPREAD_JPN
        lpf->bNegStyle = 4;
#else
        lpf->bNegStyle = 1;
#endif
        *lpwType = NUMBERTYPE;
        lstrcpy(lptstr, _T("XF2bh"));
      break;
      case 0x2c:
        lpf->Right = 2;
        lpf->bLeadingZero = 2;
        lpf->fShowSeparator = TRUE;
#ifdef SPREAD_JPN
        lpf->currencyfmt.bPosStyle = 4;
        lpf->bNegStyle = 14;
#else
        lpf->bNegStyle = 15;
#endif
        *lpwType = CURRENCYTYPE;
        lstrcpy(lptstr, _T("XF2ch"));
      break;
#else
      case 0x29:
        lpf->floatfmt.nDigitsLeft = 14;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, _T("XF2ch"));
      break;
      case 0x2a:
        lpf->floatfmt.nDigitsLeft = 14;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, _T("XF2ch"));
      break;
      case 0x2b:
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, _T("XF2bh"));
      break;
      case 0x2c:
        lpf->floatfmt.nDigitsLeft = 12;
        lpf->floatfmt.nDigitsRight = 2;
        lpf->floatfmt.fDigitLimit = TRUE;
        lpf->floatfmt.fCurrency = TRUE;
        lpf->floatfmt.fSeparator = TRUE;
        *lpwType = FLOATTYPE;
        lstrcpy(lptstr, _T("XF2ch"));
      break;
#endif
    case 0x2d:
      lpf->timefmt.b24Hour = TRUE;
      lpf->timefmt.bSeconds = TRUE;
      lpf->timefmt.cSeparator = (TCHAR)':';
      *lpwType = TIMETYPE;
      lstrcpy(lptstr, XF2dh);
    break;
    case 0x2e:
      lpf->timefmt.b24Hour = TRUE;
      lpf->timefmt.bSeconds = TRUE;
      lpf->timefmt.cSeparator = (TCHAR)':';
      *lpwType = TIMETYPE;
      lstrcpy(lptstr, XF2eh);
    break;
    case 0x2f:
      lpf->timefmt.b24Hour = TRUE;
      lpf->timefmt.bSeconds = TRUE;
      lpf->timefmt.cSeparator = (TCHAR)':';
      *lpwType = TIMETYPE;
      lstrcpy(lptstr, XF2fh);
    break;
    case 0x31:
      *lpwType = STRINGTYPE; //RAP03a
//        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, _T("@"));
      lstrcpy(lptstr, XF31h);
     break;
	  //Add by BOC 99.5.25 (hyt)
	  //for support far east excel version
//#ifdef SPREAD_JPN
	  case 0x37:
	  	lpf->datefmt.bCentury = TRUE;
      lpf->datefmt.cSeparator = (TCHAR)'?';
      lpf->datefmt.nFormat = IDF_YYMM;
      lpf->datefmt.bSpin = FALSE;
      *lpwType = DATETYPE;
      lstrcpy(lptstr, XF37h);
	  break;
	  case 0x38:
  		lpf->datefmt.bCentury = TRUE;
      lpf->datefmt.cSeparator = (TCHAR)'?';
      lpf->datefmt.nFormat = IDF_MMDD;
      lpf->datefmt.bSpin = FALSE;
      *lpwType = DATETYPE;
      lstrcpy(lptstr, XF38h);
	  break;
	  case 0x39:
		  lpf->datefmt.bCentury = FALSE;
      lpf->datefmt.cSeparator = (TCHAR)'.';
      lpf->datefmt.nFormat = IDF_NYYMMDD;
      lpf->datefmt.bSpin = FALSE;
      *lpwType = DATETYPE;
      lstrcpy(lptstr, XF39h);
	  break;
	  case 0x3a:
      lpf->datefmt.bCentury = FALSE;
      lpf->datefmt.cSeparator = (TCHAR)'?';
      lpf->datefmt.nFormat = IDF_NNNNYYMMDD;
      lpf->datefmt.bSpin = FALSE;
      *lpwType = DATETYPE;
      lstrcpy(lptstr, XF3ah);
	  break;
//#endif //SPREAD_JPN
    } //switch
/* 20157
#ifdef SS_V40
//    if (*lpwType == NUMBERTYPE || *lpwType == PERCENTTYPE || *lpwType == CURRENCYTYPE)
    {  
      lpf->Min = -999999999999.99;
      lpf->Max = 999999999999.99;
    }
#endif
#ifdef SS_V70
    if (*lpwType == SCIENTIFICTYPE)
    {
      lpf->Min = -1.7E308;
      lpf->Max = 1.7E308;
    }
#endif
*/
  } //if
  else
  {
    LPxFORMAT lpformatS = (LPxFORMAT)tbGlobalLock(lpssXL->ghFormats);
    xFORMAT   format;
    DWORD     dw;
    DWORD     dwZeroCount = 0;
    BOOL      bFound = FALSE;
  
    memset((LPxFORMAT)&format, 0, sizeof(xFORMAT));

    for (dw=0; dw<lpssXL->dwFormatCount; dw++)
    {
      if (lpformatS[dw].wIndex == wIndex)
      {
        memcpy((LPxFORMAT)&format, &(lpformatS[dw]), sizeof(xFORMAT));
        bFound = TRUE;
        break;
      }
    }
    if (!bFound && wIndex < lpssXL->dwFormatCount)
    {
      for (dw=0; dw<lpssXL->dwFormatCount; dw++)
      {
        if (lpformatS[dw].wIndex == 0)
        {
          if (dwZeroCount == wIndex)
          {
            memcpy((LPxFORMAT)&format, &(lpformatS[dw]), sizeof(xFORMAT));
            bFound = TRUE;
            break;
          }
          dwZeroCount++;
        }
      }
    }

    if (bFound)
    {
      LPBIFFSTR lpbfstr = (LPBIFFSTR)tbGlobalLock(format.ghFormat);
      LPTSTR lpszFormat = (LPTSTR)((LPBYTE)lpbfstr + sizeof(BIFFSTR)-1);
      memcpy(lptstr, lpszFormat, lpbfstr->wCharCount*sizeof(TCHAR));
      lptstr[lpbfstr->wCharCount] = 0;
      xl_PreParseFormat(lpSS, lptstr, lpf, lpwType);
      if (!*lpwType)
      {
        xl_ParseFormat(lpSS, lptstr, lpf, lpwType, lpData);
      }    

      tbGlobalUnlock(format.ghFormat);
    }

    tbGlobalUnlock(lpssXL->ghFormats);
  } // else

  return Ret;
}

BOOL bif_FixupString(TBGLOBALHANDLE ghStrS, TBGLOBALHANDLE FAR *lpghStrD)
{
  BOOL      Ret = FALSE;
  LPBIFFSTR lpbfstr = (LPBIFFSTR)tbGlobalLock(ghStrS);
  BOOL      bWChar = lpbfstr->fHighByte;

  if (bWChar)
  {
    LPTSTR lptstr;
    
    *lpghStrD = tbGlobalAlloc(GHND, lpbfstr->wCharCount);
    lptstr = (LPTSTR)tbGlobalLock(*lpghStrD);

    // If the BIFF String is stored as a WCHAR String convert the WCHAR string 
    // to a MultiByte string.
#ifndef _UNICODE
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)&lpbfstr->bString,
                        -1, lptstr, lpbfstr->wCharCount, NULL, NULL);
#else
    memcpy(lptstr, lpbfstr->bString, lpbfstr->wCharCount*sizeof(TCHAR));        
#endif

    lptstr[lpbfstr->wCharCount-1] = 0;

    tbGlobalUnlock(*lpghStrD);
  }
  else
  {
    bif_LoadData((TBGLOBALHANDLE FAR *)lpghStrD, 
                (unsigned short)(lpbfstr->wCharCount*sizeof(TCHAR)), 
                (LPBYTE)lpbfstr->bString, TRUE, bWChar, 0);
  }

  tbGlobalUnlock(ghStrS);

  return Ret;
}

BOOL bif_GetStringFromSST(LPzSST lpsst, DWORD dwIndex, TBGLOBALHANDLE FAR *lpgh)
{
  DWORD        dw = 0;
  int          nLen = 0; 
  LPBYTE       lpReturnStr = NULL;
//  LPBYTE       lpBiffStr = NULL;
  int          *pnString = 0;
  BOOL         Ret = FALSE;

  if (dwIndex >= lpsst->dwStrCount)
    return TRUE;

  pnString = (int *)tbGlobalLock(lpsst->ghStrings);

  for (dw=0; dw<dwIndex; dw++)
  {
    if (pnString)
      pnString = (int *)LPOFFSET(pnString, (*pnString + sizeof(int)));
    else
      pnString++;
  }     

  nLen = *pnString;
  Alloc(lpgh, nLen + sizeof(TCHAR));
  lpReturnStr = (LPBYTE)tbGlobalLock(*lpgh);

  pnString++;
  memcpy(lpReturnStr, pnString, nLen);
  tbGlobalUnlock(*lpgh);

  return Ret;
}

BOOL xl_FormatPicType(double num, LPTSTR lptstrFormat, BOOL fMask)
{
  BOOL  Ret = FALSE;
  short sLen = (short)lstrlen(lptstrFormat);
  TCHAR szVal[100];
  int   iVal = 0;
  int   i;
  
  _stprintf(szVal, _T("%f.0"), num);
  
  for (i=0; i<sLen; i++)
  {
    switch(lptstrFormat[i])
    {
      case (TCHAR)'0':
      case (TCHAR)'#':
        if (fMask)
        {
          lptstrFormat[i] = (TCHAR)'9';
        }
      break;
      case (TCHAR)'9':
        if (!fMask)
        {
          lptstrFormat[i] = szVal[iVal];
          iVal++;
        }
      break;
      case (TCHAR)'\\':
        lstrcpy(&lptstrFormat[i], &lptstrFormat[i+1]);
        sLen--;
      break;
      default:
        if (!fMask)
        {
          lstrcpy(&lptstrFormat[i], &lptstrFormat[i+1]);
          i--;
          sLen--;
        }
        break;
    } //switch
  } //for

  return Ret;
}

BOOL bif_LoadCell(LPSSXL lpssXL, LPSPREADSHEET lpSS, LPyCELLVAL lpcell)
{
  SS_CELLTYPE ct;
  LPCTSTR     lpctstr;
  TCHAR       szFormat[256];
  HFONT       hFont = 0;
  xXF8        xf;
  FMT         f;
  LPFMT       lpf = (LPFMT)&f;
  WORD        wType = 0;
  DWORD       dwStyle = 0;
  BOOL        Ret = FALSE;;

  memset(lpf, 0, sizeof(FMT));
  memset(&xf, 0, sizeof(xXF8));

  if (!lpssXL->fStringFmlaFlag)
  {
    WORD wRow = (WORD)SSGetMaxRows(lpSS->lpBook->hWnd);
    WORD wCol = (WORD)SSGetMaxCols(lpSS->lpBook->hWnd);
      
    if ((short)lpcell->wRow != SS_ALLROWS && lpcell->wRow+1 > wRow) //RAP07a
//RAP07d    if (lpcell->wRow+1 > wRow)
      SSSetMaxRows(lpSS->lpBook->hWnd, lpcell->wRow+1);
 
    if (lpssXL->dwXFCount)
    {
      WORD wBorderType = SS_BORDERTYPE_NONE;
      COLORREF color;
      if ((short)lpcell->wCol != SS_ALLCOLS && lpcell->wCol+1 > wCol) //RAP07a
//RAP07d    if (lpcell->wCol+1 > wCol)
        SSSetMaxCols(lpSS->lpBook->hWnd, lpcell->wCol+1);

      bif_GetXF(lpssXL->ghXFs, lpcell->wixfe, (LPxXF8)&xf);
      if ((short)lpcell->wRow != SS_ALLROWS && (short)lpcell->wCol != SS_ALLCOLS)//RAP07a
      { //RAP07a
        //SetCellFont
        bif_GetFont(lpssXL->ghFonts, xf.wifnt, (LPHFONT)&hFont, (LPCOLORREF)&color);
        if (hFont)
        {
          LPLOGFONT lplf = (LPLOGFONT)tbGlobalLock(lpssXL->ghDefaultLogFont);
          LOGFONT lf;
          memset(&lf, 0, sizeof(LOGFONT));
          GetObject(hFont, sizeof(LOGFONT), &lf);
          int len = lstrlen(lf.lfFaceName);
          memset(&(lf.lfFaceName[len]), 0, (LF_FACESIZE-len)*sizeof(TCHAR));

//18495          if (memcmp(&lf, lplf, sizeof(LOGFONT)))
          if (lplf != NULL && memcmp(&lf, lplf, sizeof(LOGFONT))) //18495
            SSSetFont(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, hFont, TRUE);
          else
            DeleteObject(hFont);

          tbGlobalUnlock(lpssXL->ghDefaultLogFont);
        }
        
        if (xf.fls)
          SSSetColor(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, xl_ColorFromIndex(xf.fiBackColor), color);
        else
          SSSetColor(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, RGBCOLOR_DEFAULT, color);
      } //RAP07a

      //SetCellLock
      SSSetLock(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1), 
                            ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), (BOOL)xf.fLocked);

#ifdef SS_V40
      if (xf.trot == 255)
        dwStyle |= SSS_TEXTORIENT_VERT_LTR;
      else if (xf.trot == 90) 
        dwStyle |= SSS_TEXTORIENT_UP;
      else if (xf.trot == 180) 
        dwStyle |= SSS_TEXTORIENT_DOWN;
#endif

    //SetHAlign
    //---------------Changed by dean 2000/02/28--------------
    //-To fix bug GEN_001_003 When importing a Excel file exported by SPREAD, all the
    //-string alignment will show on the Right side.
      if (xf.fAlign == 1)
        dwStyle |= ES_LEFT | ES_LEFTALIGN; // 15569 -- ES_LEFTALIGN will be removed if the cell is determined to be an edit cell
      else if (xf.fAlign == 2)
        dwStyle |= ES_CENTER;
      else if (xf.fAlign == 3)
        dwStyle |= ES_RIGHT;
      else if (lpcell->wCellType == xlLABELSST || lpcell->wCellType == xlBLANK)
        dwStyle |= ES_LEFT | ES_LEFTALIGN;  // 15569 -- ES_LEFTALIGN will be removed if the cell is determined to be an edit cell
      else if (lpcell->wCellType != xlFORMULA && (lpcell->wCellType != xlNUMBER || lpcell->wCellType != xlRK))
        dwStyle |= ES_RIGHT;

      //----------------------------------------------------------
      //SetVAlign
      dwStyle |= (xf.fVAlign==0)?SSS_ALIGN_TOP:(xf.fVAlign==1)?SSS_ALIGN_VCENTER:(xf.fVAlign==2)?SSS_ALIGN_BOTTOM:
                 (xf.fVAlign==3)?SSS_ALIGN_BOTTOM:SSS_ALIGN_TOP;
      //SetBorders
      wBorderType = (WORD)((xf.dgLeft?SS_BORDERTYPE_LEFT:0) | (xf.dgRight?SS_BORDERTYPE_RIGHT:0) |
                         (xf.dgTop?SS_BORDERTYPE_TOP:0) | (xf.dgBottom?SS_BORDERTYPE_BOTTOM:0));

//    if ((short)lpcell->wCol != SS_ALLCOLS)
      {
  	    //Modify by BOC 99.7.14 (hyt)--------------------------------------------------------------
	      //for every border have different type line 
	      static WORD awConvert[]={SS_BORDERTYPE_NONE,SS_BORDERSTYLE_FINE_SOLID,SS_BORDERSTYLE_SOLID,
		       SS_BORDERSTYLE_FINE_DASH,SS_BORDERSTYLE_FINE_DASH,SS_BORDERSTYLE_SOLID,SS_BORDERSTYLE_SOLID,
		       SS_BORDERSTYLE_FINE_DOT,SS_BORDERSTYLE_DASH,SS_BORDERSTYLE_FINE_DASH_DOT,
		       SS_BORDERSTYLE_DASHDOT,SS_BORDERSTYLE_FINE_DASH_DOT_DOT,SS_BORDERSTYLE_DASHDOTDOT,
		       SS_BORDERSTYLE_SOLID};

        if (xf.fAtrBdr)
        {
	        WORD awBorderStyle[4];
	        short aicv[4];

	        awBorderStyle[0]=awBorderStyle[1]=awBorderStyle[2]=awBorderStyle[3]=SS_BORDERTYPE_NONE;
  	      aicv[0] = aicv[1] = aicv[2] = aicv[3] =  -1;

	        if(xf.dgLeft >= 0 && xf.dgLeft <= 0xD)
  	      {
	  	      awBorderStyle[0] = awConvert[xf.dgLeft];
		        aicv[0] = xf.icvLeft;
	        }

	        if(xf.dgRight >= 0 && xf.dgRight <= 0xD)
	        {
		        awBorderStyle[1] = awConvert[xf.dgRight];
		        aicv[1] = xf.icvRight;
	        }

	        if(xf.dgTop >= 0 && xf.dgTop <= 0xD)
	        {
		        awBorderStyle[2] = awConvert[xf.dgTop];
		        aicv[2] = xf.icvTop;
	        }

	        if(xf.dgBottom >= 0 && xf.dgBottom <= 0xD)
	        {
		        awBorderStyle[3] = awConvert[xf.dgBottom];
		        aicv[3] = xf.icvBottom;
	        }

          if(aicv[0]==aicv[1] && aicv[0]==aicv[2] && aicv[0]==aicv[3] && 
		        awBorderStyle[0]== awBorderStyle[1] && awBorderStyle[0]== awBorderStyle[2] &&
		        awBorderStyle[0]== awBorderStyle[3])
	        {
		        if (aicv[0] == -1 || xl_ColorFromIndex(aicv[0]) == RGBCOLOR_DEFAULT)
			        color = 0x00000000;
		        else
			        color = xl_ColorFromIndex(aicv[0]);
		        SSSetBorder(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                        ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), wBorderType, awBorderStyle[0], color);
	        }
	        else
	        {
		        static WORD awBorderType[]={SS_BORDERTYPE_LEFT,SS_BORDERTYPE_RIGHT,
			        SS_BORDERTYPE_TOP,SS_BORDERTYPE_BOTTOM};
		        for(int i=0;i<4;i++)
		        {
			        if (aicv[i] == -1 || xl_ColorFromIndex(aicv[i]) == RGBCOLOR_DEFAULT)
				        color = 0x00000000;
			        else
				        color = xl_ColorFromIndex(aicv[i]);
			        if(awBorderStyle[i]!=SS_BORDERTYPE_NONE)
				        SSSetBorder(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                        ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), 
				        awBorderType[i], awBorderStyle[i], color);
		        }
	        }
        }
      }
    }
//#endif //rap
  }

  switch (lpcell->wCellType)
  {
    case xlFORMULA:
    {
      TCHAR       szFormula[2000]; //must be same value as "#define FORMULA_PART_MAX 2000" in ss_xlparse.c.
      LPyyFORMULA fx = (LPyyFORMULA)tbGlobalLock(lpcell->ghCell);
      SS_CELLTYPE ct;
      BYTE        bType = 0;

      struct _grbit
      {
        BYTE valType;
        BYTE res1;
        BYTE val;
        BYTE res2[3];
        short mask;
      }grbit;

      Ret = xl_ParseFormula(lpSS, lpssXL, fx, (LPTSTR)szFormula, lpcell, (LPBYTE)&bType, lpf);
      if (Ret == TRUE)
      {
        bType = 0; //signal an error in the formula parsing.
      }
      else if (Ret == 2)
      {
        //ptgExp was encountered.  This means a shared formula is used. Check if the correct
        //shared formula has been loaded.  If so, load the formula using the shared formula; 
        //otherwise, save the formula info in the Excel SS structure and break to 
        //parse & apply the shared formula.
//        Ret = 0;
        if (xl_FindShrFmla(lpssXL, lpcell) == 0)
        {
          // The Shared Formula is already loaded. Fixup the structure and load the formula
          bif_LoadCell(lpssXL, lpSS, lpcell);
          Ret = 0;
        }
        else
        {
          memcpy(&(lpssXL->ShrFmlaCell), lpcell,  sizeof(yCELLVAL));
        }
        break;
      }
      SSGetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);

//19808 >>
      if (lpssXL->dwXFCount)
      {
        memset(lpf, 0, sizeof(FMT));
        szFormat[0] = 0;
        bif_GetFORMAT(lpssXL, lpSS, xf.wifmt, (LPTSTR)szFormat, (LPWORD)&wType, (LPFMT FAR *)&lpf, NULL);
      }
      if (wType == INTEGERTYPE)
      {
        wType = FLOATTYPE;
      }
      
      if (wType == 0)
        wType = FLOATTYPE; 
      if (bType != STRINGTYPE)
        bType = (BYTE)wType;
//<< 19808
      
      memcpy(&grbit, &fx->dblNum, sizeof(double));
      if ((grbit.mask & 0xFFFF)==0xFFFF && grbit.valType == 0)
      {
        bType = STRINGTYPE;

        lpssXL->fStringFmlaFlag = TRUE;
        lpssXL->StringFmlaCell.wCol = lpcell->wCol;
        lpssXL->StringFmlaCell.wRow = lpcell->wRow;

        if (xf.fAlign == 0)
          dwStyle |= ES_LEFTALIGN;
      }
      else if (Ret != TRUE)
      {
        if (xf.fAlign == 0)
          dwStyle |= ES_RIGHT;
        SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, fx->dblNum);
      }

      if (ct.Type == SS_TYPE_EDIT)
      {
        BOOL fSetCellType = TRUE;

        if (lpssXL->dwXFCount)
        {
          if (lpssXL->dwFormatCount)
          {
            double dbl = 0.0;
            memset(lpf, 0, sizeof(FMT));
            wType = 0;
            bif_GetFORMAT(lpssXL, lpSS, xf.wifmt, (LPTSTR)szFormat, (LPWORD)&wType, (LPFMT FAR *)&lpf, &dbl);
            if (bType != STRINGTYPE)
              bType = (BYTE)wType;
          }
        }

#ifndef SS_V40
        if (bType == INTEGERTYPE)
        {
          SSSetTypeIntegerExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, -32768, 32767, FALSE, 0L);
        }
        else if (bType == FLOATTYPE)
        {
          SS_FLOATFORMAT ssff;

          SSGetDefFloatFormat(lpSS->lpBook->hWnd, (LPSS_FLOATFORMAT)&ssff);

          if (lpf)
          {
            if (lpf->floatfmt.szCurrency[0] != 0 && lstrcmp(lpf->floatfmt.szCurrency, _T("\\")))
              ssff.cCurrencySign = lpf->floatfmt.szCurrency[0];
            dwStyle |= (lpf->floatfmt.fCurrency?FS_MONEY:0) + (lpf->floatfmt.fSeparator?FS_SEPARATOR:0);
            if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
              dwStyle |= ES_LEFTALIGN;                          
   
            SSSetTypeFloatExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, 
                              (WORD)(lpf->floatfmt.fDigitLimit?lpf->floatfmt.nDigitsLeft:8), 
                              (WORD)lpf->floatfmt.nDigitsRight,
                              (double)(lpf->floatfmt.fDigitLimit?-(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MIN_FLOAT),
                              (double)(lpf->floatfmt.fDigitLimit?(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MAX_FLOAT),
                              (LPSS_FLOATFORMAT)&ssff);
          }
          else
            fSetCellType = FALSE;
        }
        else if (bType == DATETYPE)
#else
        if (bType == DATETYPE)
#endif
        {
          SS_DATE  dateMin = {1,1,1900};
    			SS_DATE  dateMax = {31,12,3000};
          
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;                          
          SSSetTypeDate(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (lpf?(LPSS_DATEFORMAT)&(lpf->datefmt):NULL),
                        (LPSS_DATE)&dateMin, (LPSS_DATE)&dateMax);
        }
        else if (bType == STRINGTYPE)
        {
          //Hey, that's what it is already!
          if (dwStyle == (ES_LEFTALIGN & SSS_ALIGN_BOTTOM)) //19750
            fSetCellType = FALSE;
          SS_XlSetTypeEdit(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, ct.Spec.Edit.Len, ct.Spec.Edit.ChrSet, ct.Spec.Edit.ChrCase);
        }
        else if (bType == TIMETYPE)
        {
          SS_TIME timeMin = {0,0,0};
          SS_TIME timeMax = {23,59,59};

          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;                          
          SSSetTypeTime(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (lpf?(LPSS_TIMEFORMAT)&(lpf->timefmt):NULL),
                        (LPSS_TIME)&timeMin, (LPSS_TIME)&timeMax);
        }
#ifdef SS_V40      
        else if (bType == CURRENCYTYPE)
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;                                      

          SSSetTypeCurrencyEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                              lpf->fShowSeparator, TRUE, NULL, NULL, lpf->currencyfmt.szCurrency,
                              lpf->bLeadingZero, lpf->bNegStyle, lpf->currencyfmt.bPosStyle,
                              FALSE, FALSE, 1.1);
        }
        else if (bType == PERCENTTYPE)
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN; 
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;                                      

          SSSetTypePercentEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                             lpf->szDecimal, lpf->bLeadingZero, lpf->bNegStyle, FALSE, FALSE, 1.1);
        }
        else if (bType == NUMBERTYPE || bType == FLOATTYPE)
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;                          
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;                                      

          SSSetTypeNumberEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                            lpf->fShowSeparator, NULL, NULL, lpf->bLeadingZero, lpf->bNegStyle,
                            FALSE, FALSE, 1.1);
        }
#endif // SS_V40
#ifdef SS_V70
        else if (bType == SCIENTIFICTYPE)
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;                                      

          SSSetTypeScientific(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                              lpf->szDecimal);

        }
#endif // SS_V70

        if (fSetCellType)
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
      }
 
      if (Ret == 0) //if there was not an error processing the formula
      {
        SSSetFormula(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPTSTR)szFormula, TRUE);
      }  
      tbGlobalUnlock(lpcell->ghCell);
    }
    break;
//RAP01a >>    
    case xlLABEL:
    {
      LPBIFFSTR      lpBiffStr = (LPBIFFSTR)tbGlobalLock(lpcell->ghCell);
      TBGLOBALHANDLE ghString = (TBGLOBALHANDLE)0;
      LPTSTR         lpszString = NULL;

#ifndef _UNICODE
		  if(lpBiffStr->fHighByte)
		  {
			  LPWSTR lpwstr=new WCHAR[lpBiffStr->wCharCount+1];
			  int StringLen;
			  memcpy(lpwstr,lpBiffStr->bString,lpBiffStr->wCharCount*sizeof(WCHAR));
			  StringLen=WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,lpwstr,lpBiffStr->wCharCount,NULL,0,NULL,NULL);
			  Alloc(&ghString,StringLen+1);
			  lpszString=(LPTSTR)tbGlobalLock(ghString);
			  WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,lpwstr,lpBiffStr->wCharCount,lpszString,StringLen+1,NULL,NULL);
			  delete[] lpwstr;
		  }
		  else
		  {
			  Alloc(&ghString, (lpBiffStr->wCharCount+1)*sizeof(TCHAR));
			  lpszString = (LPTSTR)tbGlobalLock(ghString);
			  memcpy(lpszString, lpBiffStr->bString, lpBiffStr->wCharCount);      
		  }
#else
		  Alloc(&ghString, (lpBiffStr->wCharCount+1)*sizeof(TCHAR));
		  lpszString = (LPTSTR)tbGlobalLock(ghString);
		  memcpy(lpszString, lpBiffStr->bString, lpBiffStr->wCharCount*sizeof(TCHAR));      

		  if (!lpBiffStr->fHighByte)
		  {
        TBGLOBALHANDLE ghMBString = 0;
        LPSTR lpszMBString = NULL;
        Alloc(&ghMBString, (lpBiffStr->wCharCount+1)*sizeof(CHAR));
        lpszMBString = (LPSTR)tbGlobalLock(ghMBString);
			  memcpy(lpszMBString, lpBiffStr->bString, lpBiffStr->wCharCount);      

        MultiByteToWideChar(CP_ACP, 0, lpszMBString, -1, lpszString, lpBiffStr->wCharCount+1);
        tbGlobalUnlock(ghMBString);
        tbGlobalFree(ghMBString);
		  }
#endif
      if (lpssXL->dwXFCount)
      {
        dwStyle |= (xf.fWrap==1)?ES_MULTILINE:ES_AUTOHSCROLL;
      }
        
      if (!lpssXL->fStringFmlaFlag)
      {

//18796d #ifdef SS_V70
        SS_XlSetTypeEdit(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, 32000, 
                         SS_CHRSET_CHR, SS_CASE_NOCASE); //xxRAP:hardcoded values
//18796d #else
//18796d         SS_XlSetTypeEdit(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, 60, 
//18796d                          SS_CHRSET_CHR, SS_CASE_NOCASE); //xxRAP:hardcoded values
//18796d #endif
        SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
      }
      else
        lpssXL->fStringFmlaFlag = FALSE;

      if(!(dwStyle & ES_MULTILINE))
		    SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpszString);
	    else
	    {
		    TBGLOBALHANDLE ghTemp = 0;
		    int lLen = lstrlen(lpszString);
        Alloc(&ghTemp, (lLen+1)*2);
		    LPTSTR lpstr = (LPTSTR)tbGlobalLock(ghTemp);
		    memset(lpstr,0,(lLen+1)*2);
		    LPTSTR p1 = (LPTSTR)lpszString;
		    LPTSTR p2 = lpstr;
		    TCHAR ch=0; //RAP06a
		    while(*p1!=0)
		    {
#ifndef _UNICODE  
			    if(IsDBCSLeadByte((BYTE)*p1))
			    {
			        *p2++ = *p1++;
			        *p2 = *p1;
				    ch =0;
			    }
			    else
			    {
#endif
				    if(*p1 == 0x0a && ch != 0x0d)
				    {
					    *p2++ = 0x0d;
					    *p2 = 0x0a;
				    }
				    else
					    *p2 = *p1;
				    ch = *p1;
#ifndef _UNICODE
			    }
#endif
			    p1++;
			    p2++;
//<< RAP06a
		    }
		    SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpstr);
        tbGlobalUnlock(ghTemp);
        tbGlobalFree(ghTemp);
	    }
// <<RAP04a
      
      tbGlobalUnlock(ghString);
      tbGlobalFree(ghString);
      tbGlobalUnlock(lpcell->ghCell);
    }
    break;
//RAP01a <<    
    case xlLABELSST:
    {
      LPyyLABELSST   lplabelsst = (LPyyLABELSST)tbGlobalLock(lpcell->ghCell);
      TBGLOBALHANDLE gh = (TBGLOBALHANDLE)0;
      BOOL           ret;

      if (dwStyle & ES_LEFTALIGN)
        dwStyle &= ~ES_LEFTALIGN;

      if (lpssXL->dwXFCount)
      {
        dwStyle |= (xf.fWrap==1)?ES_MULTILINE:ES_AUTOHSCROLL;
      }
        
      ret = bif_GetStringFromSST((LPzSST)&lpssXL->sst, lplabelsst->dwisst, (TBGLOBALHANDLE FAR *)&gh);

#ifdef SS_V70
      SS_XlSetTypeEdit(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, 32000, 
                       SS_CHRSET_CHR, SS_CASE_NOCASE); //xxRAP:hardcoded values
#else
      SS_XlSetTypeEdit(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, 60, 
                       SS_CHRSET_CHR, SS_CASE_NOCASE); //xxRAP:hardcoded values
#endif
      SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);

      if (ret)
      {
        tbGlobalUnlock(lpcell->ghCell);
        break;
      }

      lpctstr = (LPCTSTR)tbGlobalLock(gh);
  
//Modify by BOC 99.8.25 (hyt)-------------------------
//for multi-line should convert 0x0a t0 0x0d0x0a
/*
#ifndef SPREAD_JPN //xxrap test this
	  SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpctstr);
#else
*/
	  if(!(dwStyle & ES_MULTILINE))
		  SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpctstr);
	  else
	  {
		  TBGLOBALHANDLE ghTemp = 0;
		  int lLen = lstrlen(lpctstr);
      int i;
      
      for (i=0; i<lLen; i++)
      {
  	    if (lpctstr[i] == 0x0a && (i==0 || lpctstr[i-1] != 0x0d))
          lLen++;
      }

      Alloc(&ghTemp, (lLen+1)*2);
		  LPTSTR lpstr = (LPTSTR)tbGlobalLock(ghTemp);
		  memset(lpstr,0,(lLen+1)*2);
		  LPTSTR p1 = (LPTSTR)lpctstr;
		  LPTSTR p2 = lpstr;
//RAP06d		    char ch=0;
      TCHAR ch=0; //RAP06a
	    while(*p1!=0)
	    {
/*RAP06d
        if(IsDBCSLeadByte((BYTE)*p1))
			  {
			    *p2++ = *p1++;
			    *p2 = *p1;
				  ch =0;
			  }
			  else
			  {
			    if(*p1 == 0x0a && ch != 0x0d)
			    {
				    *p2++ = 0x0d;
				    *p2 = 0x0a;
			    }
			    else
				    *p2 = *p1;
			    ch = *p1;
			  }
			  p1++;
			  p2++;
RAP06d*/
//RAP06a >>
          
#ifndef _UNICODE  
        if(IsDBCSLeadByte((BYTE)*p1))
		    {
	        *p2++ = *p1++;
	        *p2 = *p1;
			    ch =0;
		    }
		    else
		    {
#endif
			    if(*p1 == 0x0a && ch != 0x0d)
			    {
				    *p2++ = 0x0d;
				    *p2 = 0x0a;
			    }
			    else
				    *p2 = *p1;
			    ch = *p1;
#ifndef _UNICODE
		    }
#endif
		    p1++;
		    p2++;
//<< RAP06a
	    }

		  SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpstr);
      tbGlobalUnlock(ghTemp);
      tbGlobalFree(ghTemp);
	  }
//#endif		//SPREAD_JPN
//-------------------------------------------------------------------       
      tbGlobalUnlock(gh);
      tbGlobalFree(gh);
      tbGlobalUnlock(lpcell->ghCell);
    }
    break;
    case xlNUMBER:
    {
      LPyyNUMBER  lpNum = (LPyyNUMBER)tbGlobalLock(lpcell->ghCell);

//19817      wType = FLOATTYPE;  //19564

      if (lpssXL->dwXFCount)
      {
        memset(lpf, 0, sizeof(FMT));
        szFormat[0] = 0;
        bif_GetFORMAT(lpssXL, lpSS, xf.wifmt, (LPTSTR)szFormat, (LPWORD)&wType, (LPFMT FAR *)&lpf, (LPVOID)&(lpNum->dblNumber));
      }
      if (wType == INTEGERTYPE &&
          (lpNum->dblNumber > (double)SS_MAX_INTEGER || lpNum->dblNumber < (double)SS_MIN_INTEGER))
      {
        wType = FLOATTYPE;
      }
      
//19817 >>
      if (wType == 0)
        wType = FLOATTYPE;  //19564
//<< 19817

      switch (wType)
      {
//RAP03a >>
        case STRINGTYPE:
           { // 25147 -scl
          char  szNumber[51]; //MUST BE a CHAR array because _gcvt is ANSI-only.
          short nLen;

          memset(szNumber, 0, 51*sizeof(char));
          _gcvt(lpNum->dblNumber, 10, (char *)szNumber); //_gcvt is ANSI-only; therefore, szNumber must be char *.
          nLen = (short)strlen(szNumber);
          // _gcvt() supresses zeros following the decimal, so you can get values like: 1.  100.  1000.
          // we need to check if the last character is a '.'.
          if ((char)'.' == ((char *)szNumber)[nLen-1])
            ((char *)szNumber)[nLen-1] = 0;

#ifdef _UNICODE
          // If unicode, the convert the ANSI chars to Wide chars.
          WCHAR wcszNumber[51];
          memset(wcszNumber, 0, 51*sizeof(WCHAR));
          MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szNumber, -1, wcszNumber, 50);
//          wcscpy(szNumber, wcszNumber);
          SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)wcszNumber);
#else
          SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)szNumber);
#endif
        }
        break;
#ifndef SS_V40
//RAP03a <<
        case INTEGERTYPE:
        {
          xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&lpNum->dblNumber);

          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;                          

          SSSetTypeIntegerExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, -32768, 32767, FALSE, 0L);
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);
          SSSetInteger(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (long)lpNum->dblNumber);
        }
        break;
        case 0:
        case FLOATTYPE:
        {
          BOOL fDigitsNonDefault = FALSE;
          double dblMin = 0.0;
          double dblMax = 0.0;

          if ((long)lpNum->dblNumber <= SS_MAX_FLOAT && (long)lpNum->dblNumber >= SS_MIN_FLOAT)
          {
            if (xf.wifmt == 0)  //General Format
            {
              short sLeftSide = xlGetDigitsLeft(lpNum->dblNumber);
              if (sLeftSide > 9) //9.5 is the default for General Format.
              {
                lpf->floatfmt.nDigitsLeft = sLeftSide;
                lpf->floatfmt.nDigitsRight = 14 - sLeftSide;
                fDigitsNonDefault = TRUE;
              }  
            }
          }

          if (lpf != NULL)
          {
            SS_FLOATFORMAT ssff;
            SSGetDefFloatFormat(lpSS->lpBook->hWnd, (LPSS_FLOATFORMAT)&ssff);
            if (lpf->floatfmt.szCurrency[0] != 0)
              ssff.cCurrencySign = lpf->floatfmt.szCurrency[0];
            dwStyle |= (lpf->floatfmt.fCurrency?FS_MONEY:0) + (lpf->floatfmt.fSeparator?FS_SEPARATOR:0);
            if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
              dwStyle |= ES_LEFTALIGN;                          

            dblMin = (double)(lpf->floatfmt.fDigitLimit?-(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MIN_FLOAT);
            dblMax = (double)(lpf->floatfmt.fDigitLimit?(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MAX_FLOAT);
            SSSetTypeFloatExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, 
                              (WORD)((lpf->floatfmt.fDigitLimit || fDigitsNonDefault)?lpf->floatfmt.nDigitsLeft:9), 
                              (WORD)lpf->floatfmt.nDigitsRight,
                              dblMin,
                              dblMax,
//                              (double)(lpf->floatfmt.fDigitLimit?-(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MIN_FLOAT),
//                              (double)(lpf->floatfmt.fDigitLimit?(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MAX_FLOAT),
                              (LPSS_FLOATFORMAT)&ssff);
          }
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);

          if (lpNum->dblNumber > SS_MAX_FLOAT || lpNum->dblNumber < SS_MIN_FLOAT)
          {
            xl_LogFile(lpSS->lpBook, LOG_FLOATOUTOFBOUNDS, lpcell->wCol+1, lpcell->wRow+1, (long *)&lpNum->dblNumber);
            break;          
          }
          if (lpNum->dblNumber >= dblMin && lpNum->dblNumber <= dblMax)
            SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpNum->dblNumber);
        }
        break;
#else //SS_V40
        case CURRENCYTYPE:
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;                          
   
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypeCurrencyEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                              lpf->fShowSeparator, TRUE, NULL, NULL, lpf->currencyfmt.szCurrency,
                              lpf->bLeadingZero, lpf->bNegStyle, lpf->currencyfmt.bPosStyle,
                              FALSE, FALSE, 1.1);
          
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);

          SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpNum->dblNumber);
          
          if (lpf->Right == 0)
          {
            double dblMod = fmod( lpNum->dblNumber, 1.0 );
            if (dblMod)
              xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&lpNum->dblNumber);
          }
        }
        break;
        case PERCENTTYPE:
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN; 

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypePercentEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                             lpf->szDecimal, lpf->bLeadingZero, lpf->bNegStyle, FALSE, FALSE, 1.1);

          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);

          SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpNum->dblNumber);
          if (lpf->Right == 0)
          {
            double dblMod = fmod( lpNum->dblNumber, 1.0 );
            if (dblMod)
              xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&lpNum->dblNumber);
          }
        }
        break;
        case 0:
          lpf->Right = 5;
        case FLOATTYPE:
        case NUMBERTYPE:
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;                          

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypeNumberEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                            lpf->fShowSeparator, NULL, NULL, lpf->bLeadingZero, lpf->bNegStyle,
                            FALSE, FALSE, 1.1);

          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);

          SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpNum->dblNumber);
          if (lpf->Right == 0)
          {
            double dblMod = fmod( lpNum->dblNumber, 1.0 );
            if (dblMod)
              xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&lpNum->dblNumber);
          }
        }
        break;
#endif
#ifdef SS_V70
        case SCIENTIFICTYPE:
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;                                      

          SSSetTypeScientific(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                              lpf->szDecimal);

          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);

          SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, lpNum->dblNumber);
          if (lpf->Right == 0)
          {
            double dblMod = fmod( lpNum->dblNumber, 1.0 );
            if (dblMod)
              xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&lpNum->dblNumber);
          }
        }
        break;
#endif // SS_V70
        case DATETYPE:
        {
          SS_DATE  ssdate;
          SS_DATE  dateMin = {1,1,1900};
		  //Modify by BOC 99.6.9 (hyt)-------------
		  //for incorrect order
			SS_DATE  dateMax = {31,12,3000};
          //SS_DATE  dateMax = {12,31,3000};
		  //----------------------------------------
          TCHAR    szBuffer[50];
          short dDaysInMonth[] = 
                  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
          BOOL     fSetDate = TRUE;
           
          if (lpf)
          {
            switch (lpf->datefmt.nFormat)
            {
              case IDF_DDMONYY:
                dwStyle |= DS_DDMONYY;
              break;
              case IDF_DDMMYY:
                dwStyle |= DS_DDMMYY;
              break;
              case IDF_MMDDYY:
                dwStyle |= DS_MMDDYY;
              break;
              case IDF_YYMMDD:
                dwStyle |= DS_YYMMDD;
              break;
            }
          }
          SSSetTypeDate(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (lpf?(LPSS_DATEFORMAT)&(lpf->datefmt):NULL),   //RAP02c
                        (LPSS_DATE)&dateMin, (LPSS_DATE)&dateMax);
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);

          memset((LPSS_DATE)&ssdate, 0, sizeof(SS_DATE));


          // Spread dates start at 1/1/1AD. Excel dates start
          // at either 1/1/1900 or 1/1/1904. Set the
          // correct start date for the date conversion.
          DateSetJulianStart((short)(lpssXL->w1904?1904:1900)); 
          // If the date stream is less than 366 days and the
          // Excel time format is 1900, back off only one day.
          // This is due to the oddity of Excel incorrectly
          // treating 1900 as a leap year.
          if (!lpssXL->w1904)
          {
            if (lpNum->dblNumber <= 366)
              lpNum->dblNumber--;
            else
              lpNum->dblNumber -= 2;
          }  
          DateJulianToDMY((long)lpNum->dblNumber, (LPSS_DATE)&ssdate);

          if (ssdate.nYear == 1900)
          {
            if (ssdate.nMonth == 3 && ssdate.nDay == 1)
            {
              //Our way to handle Excel's oddity of treating
              //1990 as a leap year.
              fSetDate = FALSE;
              xl_LogFile(lpSS->lpBook, LOG_FEB291900, lpcell->wCol+1, lpcell->wRow+1, NULL);
            }
            else if (ssdate.nMonth > 2) 
            {
              // Due to the "oddity," the rest of the year
              // is off by one day.
              ssdate.nDay--;
              if (ssdate.nDay < 1)
              {
                ssdate.nMonth--;
                ssdate.nDay += dDaysInMonth[ssdate.nMonth-1];
              }
            }
          }
          if (fSetDate)
          {
            if (lpf)
            {
  		        switch (lpf->datefmt.nFormat)
              {
                case IDF_YYMM:
  		  	        _stprintf(szBuffer, _T("%04u%02u"), ssdate.nYear, ssdate.nMonth);
                  break;
                case IDF_MMDD:
				          _stprintf(szBuffer, _T("%02u%02u"), ssdate.nMonth,ssdate.nDay);
                  break;
                default:
#ifdef SPREAD_JPN
                  _stprintf(szBuffer, _T("%04u%02u%02u"), ssdate.nYear, ssdate.nMonth, ssdate.nDay);
#else
                  _stprintf(szBuffer, _T("%02u%02u%04u"), ssdate.nMonth, ssdate.nDay, ssdate.nYear);
#endif
                  break;
              }
            }
            else
#ifdef SPREAD_JPN
              _stprintf(szBuffer, _T("%04u%02u%02u"), ssdate.nYear, ssdate.nMonth, ssdate.nDay);
#else
        			_stprintf(szBuffer, _T("%02u%02u%04u"), ssdate.nMonth, ssdate.nDay, ssdate.nYear);
#endif
            SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)szBuffer);
          }
        }  
        break;
        case TIMETYPE:
        {
          SS_TIME  sstime;
          SS_TIME  timeMin = {0,0,0};
          SS_TIME  timeMax = {23,59,59};
          TCHAR    szBuffer[50];
                       
          SSSetTypeTime(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (LPSS_TIMEFORMAT)&(lpf->timefmt), (LPSS_TIME)&timeMin, (LPSS_TIME)&timeMax);
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);

          memset(&sstime, 0, sizeof(SS_TIME));
          xl_TimeToHMS(lpNum->dblNumber, (LPSS_TIME)&sstime);
          _stprintf(szBuffer, _T("%02u%02u%02u"), sstime.nHour, sstime.nMinute, sstime.nSecond);
          SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)szBuffer);
        }
        break;
        case MASKTYPE:
        {
          xl_FormatPicType(lpNum->dblNumber, szFormat, TRUE);
          SSSetTypePic(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (LPCTSTR)szFormat);
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);

          xl_FormatPicType(lpNum->dblNumber, szFormat, FALSE);
          SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)szFormat);
        }
        default:
        break;
      }
      tbGlobalUnlock(lpcell->ghCell);
    }
    break;
    case xlRK:
    {
      LPyyRK      lpRK = (LPyyRK)tbGlobalLock(lpcell->ghCell);
      double      dbl;
      BOOL        bFloat = FALSE;

//19817      wType = FLOATTYPE;  //19564
 
      dbl = xl_NumFromRk(lpRK->dwRK, (BOOL FAR *)&bFloat);
      
      if (lpssXL->dwXFCount)
      {
        if (lpssXL->dwFormatCount)
        {
          memset(lpf, 0, sizeof(FMT));
          bif_GetFORMAT(lpssXL, lpSS, xf.wifmt, (LPTSTR)szFormat, (LPWORD)&wType, (LPFMT FAR *)&lpf, (LPVOID)&dbl);

          if (wType == INTEGERTYPE &&
            ((long)dbl > SS_MAX_INTEGER || (long)dbl < SS_MIN_INTEGER))
          {
            wType = FLOATTYPE;
          }                       
        }
      }

//19817 >>
      if (wType == 0)
        wType = FLOATTYPE;  //19564
//<< 19817

      switch (wType)
      {
//RAP03a >>
        case STRINGTYPE:
           { // 25147 -scl
          char  szNumber[51]; //MUST BE a CHAR array because _gcvt is ANSI-only.
          short nLen;

          memset(szNumber, 0, 51*sizeof(char));
          _gcvt(dbl, 10, (char *)szNumber); //_gcvt is ANSI-only; therefore, szNumber must be char *.
          nLen = (short)strlen(szNumber);
          if ((char)'.' == ((char *)szNumber)[nLen-1])
            ((char *)szNumber)[nLen-1] = 0;

#ifdef _UNICODE
         // If unicode, the convert the ANSI chars to Wide chars.
         WCHAR wcszNumber[51];
         memset(wcszNumber, 0, 51*sizeof(WCHAR));
         MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szNumber, -1, wcszNumber, 50);
//          wcscpy(szNumber, wcszNumber);
          SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)wcszNumber);
#else
          SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)szNumber);
#endif
        }
        break;
#ifndef SS_V40
//RAP03a <<
        case INTEGERTYPE:
        {
          xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&dbl);

          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;                          

          SSSetTypeIntegerExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, -32768, 32767, FALSE, 0L);
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
          SSSetInteger(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (long)dbl);
        }
        break;
        case FLOATTYPE:
        {
          BOOL   fDigitsNonDefault = FALSE;
          double dblMin = 0.0;
          double dblMax = 0.0;

          //This may be needed later, but for now, 9.5 for General format seems like a good idea.
          if (bFloat || lpf->floatfmt.nDigitsRight ||
              (long)dbl > SS_MAX_INTEGER || (long)dbl < SS_MIN_INTEGER)
          {
            if (dbl <= SS_MAX_FLOAT && dbl >= SS_MIN_FLOAT)
            {
              if (xf.wifmt == 0)  //General Format
              {
                short sLeftSide = xlGetDigitsLeft(dbl);
                if (sLeftSide > 9) //9.5 is the default for General Format.
                {
                   lpf->floatfmt.nDigitsLeft = sLeftSide;
                   lpf->floatfmt.nDigitsRight = 14 - sLeftSide;
                   fDigitsNonDefault = TRUE;
                }
              }
            }

            if (lpf != NULL)
            {
              SS_FLOATFORMAT ssff;
              SSGetDefFloatFormat(lpSS->lpBook->hWnd, (LPSS_FLOATFORMAT)&ssff);
              if (lpf->floatfmt.szCurrency[0] != 0)
                ssff.cCurrencySign = lpf->floatfmt.szCurrency[0];
              dwStyle |= (lpf->floatfmt.fCurrency?FS_MONEY:0) + (lpf->floatfmt.fSeparator?FS_SEPARATOR:0);
              if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
                dwStyle |= ES_LEFTALIGN;                          
              dblMin = (double)(lpf->floatfmt.fDigitLimit?-(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MIN_FLOAT);
              dblMax = (double)(lpf->floatfmt.fDigitLimit?(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MAX_FLOAT);
              SSSetTypeFloatExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle,
                                (WORD)((lpf->floatfmt.fDigitLimit || fDigitsNonDefault)?lpf->floatfmt.nDigitsLeft:9), 
                                (WORD)lpf->floatfmt.nDigitsRight,
                                dblMin, dblMax,
                                (LPSS_FLOATFORMAT)&ssff);
            }  
            SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
            if (dbl > SS_MAX_FLOAT || dbl < SS_MIN_FLOAT)
            {
              xl_LogFile(lpSS->lpBook, LOG_FLOATOUTOFBOUNDS, lpcell->wCol+1, lpcell->wRow+1, (long *)&dbl);
              break;          
            }
            if (dbl >= dblMin && dbl <= dblMax)
              SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, dbl);
          }
          else
          {
            xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&dbl);
            SSSetTypeIntegerExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, -32768, 32767, FALSE, 0L);
            SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
            SSSetInteger(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (long)dbl);
          }
        }
        break;
#else //SS_V40
        case CURRENCYTYPE:
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypeCurrencyEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                              lpf->fShowSeparator, TRUE, NULL, NULL, lpf->currencyfmt.szCurrency,
                              lpf->bLeadingZero, lpf->bNegStyle, lpf->currencyfmt.bPosStyle,
                              FALSE, FALSE, 1.1);
          
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
          SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, dbl);
          if (lpf->Right == 0)
          {
            double dblMod = fmod( dbl, 1.0 );
            if (dblMod)
              xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&dbl);
          }
        }
        break;
        case PERCENTTYPE:
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN; 

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypePercentEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                             lpf->szDecimal, lpf->bLeadingZero, lpf->bNegStyle, FALSE, FALSE, 1.1);

          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
          SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, dbl);
          if (lpf->Right == 0)
          {
            double dblMod = fmod( dbl, 1.0 );
            if (dblMod)
              xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&dbl);
          }
        }
        break;
        case FLOATTYPE:
        case NUMBERTYPE:
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;  

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypeNumberEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                            lpf->fShowSeparator, NULL, NULL, lpf->bLeadingZero, lpf->bNegStyle,
                            FALSE, FALSE, 1.1);

          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
          SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, dbl);
          if (lpf->Right == 0)
          {
            double dblMod = fmod( dbl, 1.0 );
            if (dblMod)
              xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&dbl);
          }
        }
        break;
#endif
#ifdef SS_V70
        case SCIENTIFICTYPE:
        {
          if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
            dwStyle |= ES_LEFTALIGN;
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;                                      

          SSSetTypeScientific(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                              lpf->szDecimal);

          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);

          SSSetFloat(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, dbl);
          if (lpf->Right == 0)
          {
            double dblMod = fmod( dbl, 1.0 );
            if (dblMod)
              xl_LogFile(lpSS->lpBook, LOG_INTEGERCELLTYPE, lpcell->wCol+1, lpcell->wRow+1, (long *)&dbl);
          }
        }
        break;
#endif // SS_V70
        case DATETYPE:
        {
          SS_DATE  ssdate;
#ifdef SPREAD_JPN
          SS_DATE  dateMin = {1,1,1900};
#else
		  SS_DATE  dateMin = {1,1,1};
#endif
		  //Modify by BOC 99.6.17 (hyt)-------------
		  //for incorrect order
			SS_DATE  dateMax = {31,12,9999};
          //SS_DATE  dateMax = {12,31,9999};
		  //----------------------------------------

          TCHAR    szBuffer[50];
          short    dDaysInMonth[] = 
                    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
          BOOL     fSetDate = TRUE;
                       
          if (lpf)
          {
            switch (lpf->datefmt.nFormat)
            {
              case IDF_DDMONYY:
                dwStyle |= DS_DDMONYY;
              break;
              case IDF_DDMMYY:
                dwStyle |= DS_DDMMYY;
              break;
              case IDF_MMDDYY:
                dwStyle |= DS_MMDDYY;
              break;
              case IDF_YYMMDD:
                dwStyle |= DS_YYMMDD;
              break;
              default:
#ifdef SPREAD_JPN
                dwStyle |= DS_YYMMDD;
#else
                dwStyle |= DS_MMDDYY;
#endif
              break;
            }
          }              
          memset((LPSS_DATE)&ssdate, 0, sizeof(SS_DATE));
          
          // Spread dates start at 1/1/1AD. Excel dates start
          // at either 1/1/1900 or 1/1/1904. Set the
          // correct start date for the date conversion.
          DateSetJulianStart((short)(lpssXL->w1904?1904:1900)); 
          // If the date stream is less than 366 days and the
          // Excel time format is 1900, back off only one day.
          // This is due to the oddity of Excel incorrectly
          // treating 1900 as a leap year.
          if (!lpssXL->w1904)
          {
            if (dbl <= 366)
              dbl--;
            else
              dbl -= 2;
          }  
          DateJulianToDMY((long)dbl, (LPSS_DATE)&ssdate);

          if (ssdate.nYear == 1900)
          {
            if (ssdate.nMonth == 3 && ssdate.nDay == 1)
            {
              //Our way to handle Excel's oddity of treating
              //1990 as a leap year.
              fSetDate = FALSE;
              xl_LogFile(lpSS->lpBook, LOG_FEB291900, lpcell->wCol+1, lpcell->wRow+1, NULL);
            }
            else if (ssdate.nMonth > 2) 
            {
              // Due to the "oddity," the rest of the year is off by one day.
              ssdate.nDay--;
              if (ssdate.nDay < 1)
              {
                ssdate.nMonth--;
                ssdate.nDay += dDaysInMonth[ssdate.nMonth-1];
              }
            }
          }
          SSSetTypeDate(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (lpf?(LPSS_DATEFORMAT)&(lpf->datefmt):NULL), //RAP02c
                        (LPSS_DATE)&dateMin, (LPSS_DATE)&dateMax);
          if (!lpf)
          {
            ct.Spec.Date.Format.bCentury = TRUE;
          }
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);
          if (fSetDate)
          {
/*
			//Modify by BOC 99.4.28 (hyt)---------------------------
			//for japanese version date format should be YYYYMMDD
#ifdef SPREAD_JPN
			if (lpf)
      {
        switch (lpf->datefmt.nFormat)
	  		{
		  		case IDF_YYMM:
			  		_stprintf(szBuffer, _T("%04u%02u"), ssdate.nYear, ssdate.nMonth);
				  break;
  				case IDF_MMDD:
	  				_stprintf(szBuffer, _T("%02u%02u"), ssdate.nMonth,ssdate.nDay);
		  		break;
			  	default:
				    _stprintf(szBuffer, _T("%04u%02u%02u"), ssdate.nYear, ssdate.nMonth, ssdate.nDay);
			  }
      }
      else
        _stprintf(szBuffer, _T("%04u%02u%02u"), ssdate.nYear, ssdate.nMonth, ssdate.nDay);
#else
			_stprintf(szBuffer, _T("%02u%02u%04u"), ssdate.nMonth, ssdate.nDay, ssdate.nYear);
#endif
			//--------------------------------------------------------
*/
			      if (lpf)
            {
              switch (lpf->datefmt.nFormat)
	  		      {
		  		      case IDF_YYMM:
			  		      _stprintf(szBuffer, _T("%04u%02u"), ssdate.nYear, ssdate.nMonth);
				        break;
  				      case IDF_MMDD:
	  				      _stprintf(szBuffer, _T("%02u%02u"), ssdate.nMonth,ssdate.nDay);
		  		      break;
			  	      default:
#ifdef SPREAD_JPN
                  _stprintf(szBuffer, _T("%04u%02u%02u"), ssdate.nYear, ssdate.nMonth, ssdate.nDay);
#else
            			_stprintf(szBuffer, _T("%02u%02u%04u"), ssdate.nMonth, ssdate.nDay, ssdate.nYear);
#endif
			            break;
              }
            }
            else
#ifdef SPREAD_JPN
              _stprintf(szBuffer, _T("%04u%02u%02u"), ssdate.nYear, ssdate.nMonth, ssdate.nDay);
#else
        			_stprintf(szBuffer, _T("%02u%02u%04u"), ssdate.nMonth, ssdate.nDay, ssdate.nYear);
#endif
            SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)szBuffer);
          }
        }  
        break;
        case TIMETYPE:
        {
          SS_TIME  sstime;
          SS_TIME  timeMin = {0,0,0};
          SS_TIME  timeMax = {23,59,59};
          TCHAR    szBuffer[50];
                     
          SSSetTypeTime(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (LPSS_TIMEFORMAT)&(lpf->timefmt), (LPSS_TIME)&timeMin, (LPSS_TIME)&timeMax);
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1,
                        (LPSS_CELLTYPE)&ct);

          memset((LPSS_DATE)&sstime, 0, sizeof(SS_TIME));
          xl_TimeToHMS(dbl, (LPSS_TIME)&sstime);
          _stprintf(szBuffer, _T("%02u%02u%02u"), sstime.nHour, sstime.nMinute, sstime.nSecond);
          SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)szBuffer);
        }
        break;
        case MASKTYPE:
        {
          xl_FormatPicType(dbl, szFormat, TRUE);
          SSSetTypePic(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (LPCTSTR)szFormat);
          SSSetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);

          xl_FormatPicType(dbl, szFormat, FALSE);
          SSSetValue(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPCTSTR)szFormat);
        }
      }
      tbGlobalUnlock(lpcell->ghCell);
    }
    break;
    case xlBLANK:
    {
      if (lpssXL->dwXFCount)
      {
        if (lpssXL->dwFormatCount)
        {
          memset(lpf, 0, sizeof(FMT));
          bif_GetFORMAT(lpssXL, lpSS, xf.wifmt, (LPTSTR)szFormat, (LPWORD)&wType, (LPFMT FAR *)&lpf, NULL);
        }
      }
      switch (wType)
      {
#ifndef SS_V40
        case FLOATTYPE:
        {
// Hatakeyama@boc.co.jp 11/14-2000 ADD if (lpf != NULL)
// Fixed to prevent errors when the 'lpf' reference and NULL check order are wrong.
         if (lpf != NULL)	{
          if (lpf->floatfmt.nDigitsRight)
          {
// Hatakeyama@boc.co.jp 11/14-2000 DELETE if (lpf != NULL)
//            if (lpf != NULL)
//            {
              SS_FLOATFORMAT ssff;
              SSGetDefFloatFormat(lpSS->lpBook->hWnd, (LPSS_FLOATFORMAT)&ssff);
              if (lpf->floatfmt.szCurrency[0] != 0)
                ssff.cCurrencySign = lpf->floatfmt.szCurrency[0];
              dwStyle |= (lpf->floatfmt.fCurrency?FS_MONEY:0) + (lpf->floatfmt.fSeparator?FS_SEPARATOR:0);
              if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
                dwStyle |= ES_LEFTALIGN;  
              SSSetTypeFloatExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle,
                                (WORD)(lpf->floatfmt.fDigitLimit?lpf->floatfmt.nDigitsLeft:8), 
                                (WORD)(lpf->floatfmt.fDigitLimit?lpf->floatfmt.nDigitsRight:2),
                                (double)(lpf->floatfmt.fDigitLimit?-(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MIN_FLOAT),
                                (double)(lpf->floatfmt.fDigitLimit?(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MAX_FLOAT),
                                (LPSS_FLOATFORMAT)&ssff);
//            }  
            SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                      ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                          (LPSS_CELLTYPE)&ct);
          }
          else
          {
            SSSetTypeIntegerExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, -32768, 32767, FALSE, 0L);
            SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                      ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                          (LPSS_CELLTYPE)&ct);
          }
          tbGlobalUnlock(lpcell->ghCell);
		      }	// if (lpf != NULL)
        }
        break;
#else
        case CURRENCYTYPE:
        {
          
          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          
   
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypeCurrencyEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                              lpf->fShowSeparator, TRUE, NULL, NULL, lpf->currencyfmt.szCurrency,
                              lpf->bLeadingZero, lpf->bNegStyle, lpf->currencyfmt.bPosStyle,
                              FALSE, FALSE, 1.1);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), (LPSS_CELLTYPE)&ct);
        }
        break;
        case PERCENTTYPE:
        {
          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypePercentEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                             lpf->szDecimal, lpf->bLeadingZero, lpf->bNegStyle, FALSE, FALSE, 1.1);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), (LPSS_CELLTYPE)&ct);
        }
        break;
        case FLOATTYPE:
        case NUMBERTYPE:
        {
          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypeNumberEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                            lpf->fShowSeparator, NULL, NULL, lpf->bLeadingZero, lpf->bNegStyle,
                            FALSE, FALSE, 1.1);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), (LPSS_CELLTYPE)&ct);
        }
        break;
#endif        
        case DATETYPE:
        {
          SS_DATE  dateMin = {1,1,1};
          SS_DATE  dateMax = {31, 12,9999};

          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          

          if (lpf)
          {
            switch (lpf->datefmt.nFormat)
            {
              case IDF_DDMONYY:
                dwStyle |= DS_DDMONYY;
              break;
              case IDF_DDMMYY:
                dwStyle |= DS_DDMMYY;
              break;
              case IDF_MMDDYY:
                dwStyle |= DS_MMDDYY;
              break;
              case IDF_YYMMDD:
                dwStyle |= DS_YYMMDD;
              break;
            }
          }
          else 
            dwStyle |= DS_DDMMYY;

          SSSetTypeDate(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (lpf?(LPSS_DATEFORMAT)&(lpf->datefmt):NULL),  //RAP02c
                        (LPSS_DATE)&dateMin, (LPSS_DATE)&dateMax);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                        (LPSS_CELLTYPE)&ct);
        }  
        break;
        case TIMETYPE:
        {
          SS_TIME  timeMin = {0,0,0};
          SS_TIME  timeMax = {23,59,59};

          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          
                     
          SSSetTypeTime(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (LPSS_TIMEFORMAT)&(lpf->timefmt), (LPSS_TIME)&timeMin, (LPSS_TIME)&timeMax);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                        (LPSS_CELLTYPE)&ct);
        }
        break;
        default:
        {
          if (xf.fAlign == 0 || xf.fAlign == 1)
            dwStyle |= ES_LEFT;  
          if (dwStyle & ES_LEFTALIGN)
            dwStyle &= ~ES_LEFTALIGN;

          if (lpssXL->dwXFCount)
          {
            dwStyle |= (xf.fWrap==1)?ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN:ES_AUTOHSCROLL;
          }
      
#ifdef SS_V70
          SS_XlSetTypeEdit(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, 32000, 
                           SS_CHRSET_CHR, SS_CASE_NOCASE); //xxRAP:hardcoded values
#else
          SS_XlSetTypeEdit(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, 60, 
                           SS_CHRSET_CHR, SS_CASE_NOCASE); //xxRAP:hardcoded values
#endif
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                        ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                        (LPSS_CELLTYPE)&ct);
        }
        break;
      }
    }
    break;
    case xlBOOLERR:
    {
#ifdef SS_V40
      LPyyBOOLERR lpBE = (LPyyBOOLERR)tbGlobalLock(lpcell->ghCell);
#endif
      if (lpssXL->dwXFCount)
      {
        if (lpssXL->dwFormatCount)
        {
          memset(lpf, 0, sizeof(FMT));
          bif_GetFORMAT(lpssXL, lpSS, xf.wifmt, (LPTSTR)szFormat, (LPWORD)&wType, (LPFMT FAR *)&lpf, NULL);
        }
      }
      switch (wType)
      {
#ifndef SS_V40
        case FLOATTYPE:
        default:
        {
// Hatakeyama@boc.co.jp 11/14-2000 ADD if (lpf != NULL)
// Fixed to prevent errors when the 'lpf' reference and NULL check order are wrong.
         if (lpf != NULL)	{
          if (lpf->floatfmt.nDigitsRight)
          {
// Hatakeyama@boc.co.jp 11/14-2000 DELETE if (lpf != NULL)
//            if (lpf != NULL)
//            {
            SS_FLOATFORMAT ssff;
            SSGetDefFloatFormat(lpSS->lpBook->hWnd, (LPSS_FLOATFORMAT)&ssff);
            if (lpf->floatfmt.szCurrency[0] != 0)
              ssff.cCurrencySign = lpf->floatfmt.szCurrency[0];
            dwStyle |= (lpf->floatfmt.fCurrency?FS_MONEY:0) + (lpf->floatfmt.fSeparator?FS_SEPARATOR:0);
            if (!(dwStyle & ES_CENTER) && !(dwStyle & ES_RIGHT))
              dwStyle |= ES_LEFTALIGN;
            SSSetTypeFloatExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle,
                              (WORD)(lpf->floatfmt.fDigitLimit?lpf->floatfmt.nDigitsLeft:8), 
                              (WORD)(lpf->floatfmt.fDigitLimit?lpf->floatfmt.nDigitsRight:2),
                              (double)(lpf->floatfmt.fDigitLimit?-(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MIN_FLOAT),
                              (double)(lpf->floatfmt.fDigitLimit?(pow(10,lpf->floatfmt.nDigitsLeft) - pow(10,-lpf->floatfmt.nDigitsRight)):SS_MAX_FLOAT),
                              (LPSS_FLOATFORMAT)&ssff);
//            }  
            SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                          ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                          (LPSS_CELLTYPE)&ct);
          }
          else
          {
            SSSetTypeIntegerExt(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, -32768, 32767, FALSE, 0L);
            SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                      ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                          (LPSS_CELLTYPE)&ct);
          }
          tbGlobalUnlock(lpcell->ghCell);
		      }	// if (lpf != NULL)
        }
        break;
#else
        case CURRENCYTYPE:
        {
          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          
   
          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypeCurrencyEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                              lpf->fShowSeparator, TRUE, NULL, NULL, lpf->currencyfmt.szCurrency,
                              lpf->bLeadingZero, lpf->bNegStyle, lpf->currencyfmt.bPosStyle,
                              FALSE, FALSE, 1.1);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), (LPSS_CELLTYPE)&ct);
          if (!lpBE->bIsError)
            SSSetFormula(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (lpBE->bFlag?_T("TRUE()"):_T("FALSE()")), TRUE);
        }
        break;
        case PERCENTTYPE:
        {
          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypePercentEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                             lpf->szDecimal, lpf->bLeadingZero, lpf->bNegStyle, FALSE, FALSE, 1.1);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), (LPSS_CELLTYPE)&ct);
          if (!lpBE->bIsError)
            SSSetFormula(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (lpBE->bFlag?_T("TRUE()"):_T("FALSE()")), TRUE);
        }
        break;
        case FLOATTYPE:
        case NUMBERTYPE:
        default:
        {
          TCHAR szFormula[100];

          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          

          if (lpf->fNegRed)
            dwStyle |= SSS_NEGRED;

          SSSetTypeNumberEx(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, lpf->Right, lpf->Min, lpf->Max,
                            lpf->fShowSeparator, NULL, NULL, lpf->bLeadingZero, lpf->bNegStyle,
                            FALSE, FALSE, 1.1);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1), (LPSS_CELLTYPE)&ct);
          if (lpBE->bIsError)
          {
            switch (lpBE->bFlag)
            {
              case 0:
                _tcscpy(szFormula, _T("#NULL!"));
                break;
              case 7:
                _tcscpy(szFormula, _T("#DIV/0!"));
                break;
              case 15:
                _tcscpy(szFormula, _T("#VALUE!"));
                break;
              case 23:
                _tcscpy(szFormula, _T("#REF!"));
                break;
              case 29:
                _tcscpy(szFormula, _T("#NAME?"));
                break;
              case 36:
                _tcscpy(szFormula, _T("#NUM!"));
                break;
              case 42:
                _tcscpy(szFormula, _T("#N/A"));
                break;
            }
          }
          else
          {
            if (lpBE->bFlag)
              _tcscpy(szFormula, _T("TRUE()"));
            else
              _tcscpy(szFormula, _T("FALSE()"));
          }

          SSSetFormula(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, szFormula, TRUE);
        }
        break;
#endif        
        case DATETYPE:
        {
          SS_DATE  dateMin = {1,1,1};
          SS_DATE  dateMax = {31, 12,9999};

          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          

          if (lpf)
          {
            switch (lpf->datefmt.nFormat)
            {
              case IDF_DDMONYY:
                dwStyle |= DS_DDMONYY;
              break;
              case IDF_DDMMYY:
                dwStyle |= DS_DDMMYY;
              break;
              case IDF_MMDDYY:
                dwStyle |= DS_MMDDYY;
              break;
              case IDF_YYMMDD:
                dwStyle |= DS_YYMMDD;
              break;
            }
          }
          else 
            dwStyle |= DS_DDMMYY;

          SSSetTypeDate(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (lpf?(LPSS_DATEFORMAT)&(lpf->datefmt):NULL),  //RAP02c
                        (LPSS_DATE)&dateMin, (LPSS_DATE)&dateMax);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                        (LPSS_CELLTYPE)&ct);
        }  
        break;
        case TIMETYPE:
        {
          SS_TIME  timeMin = {0,0,0};
          SS_TIME  timeMax = {23,59,59};

          if (xf.fAlign == 0)
            dwStyle |= ES_RIGHT;                          
                     
          SSSetTypeTime(lpSS->lpBook->hWnd, (LPSS_CELLTYPE)&ct, dwStyle, (LPSS_TIMEFORMAT)&(lpf->timefmt), (LPSS_TIME)&timeMin, (LPSS_TIME)&timeMax);
          SSSetCellType(lpSS->lpBook->hWnd, ((short)lpcell->wCol==SS_ALLCOLS?SS_ALLCOLS:lpcell->wCol+1),
                                    ((short)lpcell->wRow==SS_ALLROWS?SS_ALLROWS:lpcell->wRow+1),
                        (LPSS_CELLTYPE)&ct);
        }
        break;
      }

      tbGlobalUnlock(lpcell->ghCell);
    }
    break;

    default:
      #ifdef _DEBUG
      DisplayError(S_FALSE, _T("xlloadl:bif_LoadCell"));
      #endif
    break;

  } //end switch

  return Ret;
}

BOOL bif_SetRowProps(LPSSXL lpssXL, LPSPREADSHEET lpSS, LPyROW lpRow, BOOL fRowHeightOnly)
{
  double dblTwips;
  double dblPixels;
  HDC    hDC = GetDC(lpSS->lpBook->hWnd);
  WORD   wBorderType = 0;
  WORD   wBorderStyle = 0;
  short  icv = 0;
  BOOL   Ret = FALSE;
  
  //SetMaxRows
  if (lpRow->wRow+1 > SSGetMaxRows(lpSS->lpBook->hWnd))
  {
    SSSetMaxRows(lpSS->lpBook->hWnd, lpRow->wRow+1);
  }

  //SetRowHeight
  dblTwips = (lpRow->wyRow/20) * TWIP_PER_PT;
  dblPixels = MulDiv((int)dblTwips, GetDeviceCaps(hDC, LOGPIXELSY), 1440);

  SSSetRowHeightInPixels(lpSS->lpBook->hWnd, lpRow->wRow+1, (WORD)dblPixels);
  
  if (fRowHeightOnly) //15436, 19989
  {
    ReleaseDC(lpSS->lpBook->hWnd, hDC); //20121
    return true;      //15436, 19989
  }
  
  if (lpRow->fGhostDirty)
  {
    //SetRowFont
    xXF8     xf;
    HFONT    hFont = 0;
    COLORREF color;
    WORD     wixfe = (WORD)(lpRow->wixfe & 0x0FFF); // only the loworder 12 bits are used for the XF index.

    bif_GetXF(lpssXL->ghXFs, wixfe, (LPxXF8)&xf);

    bif_GetFont(lpssXL->ghFonts, xf.wifnt, (LPHFONT)&hFont, (LPCOLORREF)&color);

    if (hFont)
    {
      LPLOGFONT lplf = (LPLOGFONT)tbGlobalLock(lpssXL->ghDefaultLogFont);
      LOGFONT  lf;
      memset(&lf, 0, sizeof(LOGFONT));
      GetObject(hFont, sizeof(LOGFONT), &lf);
      int len = lstrlen(lf.lfFaceName);
      memset(&(lf.lfFaceName[len]), 0, (LF_FACESIZE-len)*sizeof(TCHAR));
        
//18495      if (memcmp(&lf, lplf, sizeof(LOGFONT)))
      if (lplf != NULL && memcmp(&lf, lplf, sizeof(LOGFONT))) //18495
        SSSetFont(lpSS->lpBook->hWnd, SS_ALLCOLS, lpRow->wRow+1, hFont, TRUE);
      else
        DeleteObject(hFont);

      tbGlobalUnlock(lpssXL->ghDefaultLogFont);
    }

    if (lpRow->res3 == 1) //this nice little undocumented treat specifies to use the row color.
      SSSetColor(lpSS->lpBook->hWnd, SS_ALLCOLS, lpRow->wRow+1, (xf.fls?xl_ColorFromIndex(xf.fiBackColor):-1), color);

    //SetBorders
    wBorderType = (WORD)((xf.dgLeft?SS_BORDERTYPE_LEFT:0) | (xf.dgRight?SS_BORDERTYPE_RIGHT:0) |
                         (xf.dgTop?SS_BORDERTYPE_TOP:0) | (xf.dgBottom?SS_BORDERTYPE_BOTTOM:0));

    if (xf.dgLeft == 0x01 || xf.dgRight == 0x01 || xf.dgTop == 0x01 || xf.dgBottom == 0x01)
    {
      wBorderStyle = SS_BORDERSTYLE_FINE_SOLID;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x02 || xf.dgRight == 0x02 || xf.dgTop == 0x02 || xf.dgBottom == 0x02)
    {
      wBorderStyle = SS_BORDERSTYLE_SOLID;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x03 || xf.dgRight == 0x03 || xf.dgTop == 0x03 || xf.dgBottom == 0x03)
    {
      wBorderStyle = SS_BORDERSTYLE_DASH;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x04 || xf.dgRight == 0x04 || xf.dgTop == 0x04 || xf.dgBottom == 0x04)
    {
      wBorderStyle = SS_BORDERSTYLE_DOT;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x05 || xf.dgRight == 0x05 || xf.dgTop == 0x05 || xf.dgBottom == 0x05)
    {
      wBorderStyle = SS_BORDERSTYLE_SOLID;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x06 || xf.dgRight == 0x06 || xf.dgTop == 0x06 || xf.dgBottom == 0x06)
    { 
      wBorderStyle = SS_BORDERSTYLE_SOLID;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x07 || xf.dgRight == 0x07 || xf.dgTop == 0x07 || xf.dgBottom == 0x07)
    {
      wBorderStyle = SS_BORDERSTYLE_FINE_DOT;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x08 || xf.dgRight == 0x08 || xf.dgTop == 0x08 || xf.dgBottom == 0x08)
    {
      wBorderStyle = SS_BORDERSTYLE_DASH;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x09 || xf.dgRight == 0x09 || xf.dgTop == 0x09 || xf.dgBottom == 0x09)
    {
      wBorderStyle = SS_BORDERSTYLE_FINE_DASH_DOT;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x0a || xf.dgRight == 0x0a || xf.dgTop == 0x0a || xf.dgBottom == 0x0a)
    {
      wBorderStyle = SS_BORDERSTYLE_DASHDOT;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x0b || xf.dgRight == 0x0b || xf.dgTop == 0x0b || xf.dgBottom == 0x0b)
    {
      wBorderStyle = SS_BORDERSTYLE_FINE_DASH_DOT_DOT;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x0c || xf.dgRight == 0x0c || xf.dgTop == 0x0c || xf.dgBottom == 0x0c)
    {
      wBorderStyle = SS_BORDERSTYLE_DASHDOTDOT;
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (xf.dgLeft == 0x0d || xf.dgRight == 0x0d || xf.dgTop == 0x0d || xf.dgBottom == 0x0d)
    {
  	//Modify by BOC 99.7.9 (hyt)------------------------------
	  //Slanted dash-dot	should convert to Solid
      //wBorderStyle = SS_BORDERSTYLE_DASHDOT;
		  wBorderStyle = SS_BORDERSTYLE_SOLID;
  	//--------------------------------------------------------
      icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
    }
    if (icv == -1 || xl_ColorFromIndex(icv) == RGBCOLOR_DEFAULT)
      color = 0x00000000;
    else
      color = xl_ColorFromIndex(icv);
    SSSetBorder(lpSS->lpBook->hWnd, SS_ALLCOLS, lpRow->wRow+1, wBorderType, wBorderStyle, color); 
  
  

    //RAP07a >>
    // if fGhostDirty == TRUE, thie wixfe element of the ROW record is the XF index.
    yCELLVAL cell;
    memset(&cell, 0, sizeof(yCELLVAL));
    cell.wCellType = xlBLANK;
    cell.wCol = (WORD)SS_ALLCOLS;
    cell.wixfe = wixfe;
    cell.wRow = lpRow->wRow;
    bif_LoadCell(lpssXL, lpSS, &cell);
    //<< RAP07a

  }

  //SSShowRow
  if (lpRow->fDyZero)
    SSShowRow(lpSS->lpBook->hWnd, lpRow->wRow+1, FALSE); 

  ReleaseDC(lpSS->lpBook->hWnd, hDC);

  return Ret;
}

BOOL bif_SetColProps(LPSSXL lpssXL, LPSPREADSHEET lpSS, LPyCOLINFO lpCol)
{
  DWORD         dwRange;
  xXF8          xf;
  HDC           hDC = GetDC(lpSS->lpBook->hWnd);
  COLORREF      color;
  HFONT         hFont = SSGetFont(lpSS->lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS);     
  HFONT         hFontOld = (HFONT)SelectObject(hDC, hFont);
  RECT          rc;
  int           CharWidth = 0;
  BOOL          bSetFont = TRUE; //15103
  BOOL          Ret = FALSE;

  memset(&xf, 0, sizeof(xXF8));

  //SetMaxCols
  if (lpCol->wColLast+1 > SSGetMaxCols(lpSS->lpBook->hWnd))
  {
    SSSetMaxCols(lpSS->lpBook->hWnd, lpCol->wColLast+1);
  }

  bif_GetXF(lpssXL->ghXFs, lpCol->wixfe, (LPxXF8)&xf);
  bif_GetFont(lpssXL->ghFonts, xf.wifnt, (LPHFONT)&hFont, (LPCOLORREF)&color);

  if (hFont)
  {
    LPLOGFONT lplf = (LPLOGFONT)tbGlobalLock(lpssXL->ghDefaultLogFont);
    LOGFONT lf;
    GetObject(hFont, sizeof(LOGFONT), &lf);
  
//18495    if (memcmp(&lf, lplf, sizeof(LOGFONT)))
    if (lplf != NULL && memcmp(&lf, lplf, sizeof(LOGFONT))) //18495
    {
      bSetFont = FALSE; //15103
      DeleteObject(hFont);
    }
      
    tbGlobalUnlock(lpssXL->ghDefaultLogFont);
  }

	memset(&rc,0,sizeof(RECT));

  SIZE size;
  GetTextExtentPoint32(hDC, _T("0"), 1, &size);
  
  CharWidth = size.cx;

  if(CharWidth<=0)
	  CharWidth=1;

  if (lpCol->wColFirst == 0 && lpCol->wColLast == 256)
  {
    // Assume this format is set for -1, -1 (entire sheet)
    yCELLVAL cell;
    memset(&cell, 0, sizeof(yCELLVAL));
    cell.wCellType = xlBLANK;
    cell.wCol = (USHORT)SS_ALLCOLS;
    cell.wixfe = lpCol->wixfe;
    cell.wRow = (WORD)SS_ALLROWS;
    bif_LoadCell(lpssXL, lpSS, &cell);
  }

  for (dwRange=lpCol->wColFirst; dwRange<=lpCol->wColLast; dwRange++)
  {
    double dblColWidthPixels = (double)MulDiv(lpCol->wdxCol, CharWidth, 256);
//    double dblColWidth;
    
//    SS_CalcPixelsToColWidth(lpSS, dwRange+1, (int)(dblColWidthPixels), &dblColWidth);

    //SetColFont
    if (bSetFont) //15103
      SSSetFont(lpSS->lpBook->hWnd, dwRange+1, SS_ALLROWS, hFont, FALSE);
    //SetColWidth
//19561    SSSetColWidthInPixels(lpSS->lpBook->hWnd, dwRange+1, (int)dblColWidthPixels);
    SSSetColWidthInPixels(lpSS->lpBook->hWnd, dwRange+1, (int)dblColWidthPixels+1); //19561
    //SSShowCol
    SSShowCol(lpSS->lpBook->hWnd, dwRange+1, lpCol->fHidden?FALSE:TRUE); 
    
    if (xf.fls && xf.fiBackColor != 0x40)
      SSSetColor(lpSS->lpBook->hWnd, dwRange+1, SS_ALLROWS, (xf.fls?xl_ColorFromIndex(xf.fiBackColor):-1), color);

//RAP07a >>
//    yCELLVAL cell;
//    memset(&cell, 0, sizeof(yCELLVAL));
//    cell.wCellType = xlBLANK;
//    cell.wCol = (USHORT)dwRange;
//    cell.wixfe = lpCol->wixfe;
//    cell.wRow = (WORD)SS_ALLROWS;
//    bif_LoadCell(lpssXL, lpSS, &cell);
//<< RAP07a
  
    if (xf.fAtrBdr)
    {
      //SetBorders
      short    icv = 0;
      WORD     wBorderStyle = 0;
      WORD     wBorderType = (WORD)((xf.dgLeft?SS_BORDERTYPE_LEFT:0) | (xf.dgRight?SS_BORDERTYPE_RIGHT:0) |
                                (xf.dgTop?SS_BORDERTYPE_TOP:0) | (xf.dgBottom?SS_BORDERTYPE_BOTTOM:0));
      COLORREF color = 0;

      if (xf.dgLeft == 0x01 || xf.dgRight == 0x01 || xf.dgTop == 0x01 || xf.dgBottom == 0x01)
      {
        wBorderStyle = SS_BORDERSTYLE_FINE_SOLID;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x02 || xf.dgRight == 0x02 || xf.dgTop == 0x02 || xf.dgBottom == 0x02)
      {
        wBorderStyle = SS_BORDERSTYLE_SOLID;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x03 || xf.dgRight == 0x03 || xf.dgTop == 0x03 || xf.dgBottom == 0x03)
      {
        wBorderStyle = SS_BORDERSTYLE_DASH;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x04 || xf.dgRight == 0x04 || xf.dgTop == 0x04 || xf.dgBottom == 0x04)
      {
        wBorderStyle = SS_BORDERSTYLE_DOT;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x05 || xf.dgRight == 0x05 || xf.dgTop == 0x05 || xf.dgBottom == 0x05)
      {
        wBorderStyle = SS_BORDERSTYLE_SOLID;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x06 || xf.dgRight == 0x06 || xf.dgTop == 0x06 || xf.dgBottom == 0x06)
      { 
        wBorderStyle = SS_BORDERSTYLE_SOLID;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x07 || xf.dgRight == 0x07 || xf.dgTop == 0x07 || xf.dgBottom == 0x07)
      {
        wBorderStyle = SS_BORDERSTYLE_FINE_DOT;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x08 || xf.dgRight == 0x08 || xf.dgTop == 0x08 || xf.dgBottom == 0x08)
      {
        wBorderStyle = SS_BORDERSTYLE_DASH;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x09 || xf.dgRight == 0x09 || xf.dgTop == 0x09 || xf.dgBottom == 0x09)
      {
        wBorderStyle = SS_BORDERSTYLE_FINE_DASH_DOT;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x0a || xf.dgRight == 0x0a || xf.dgTop == 0x0a || xf.dgBottom == 0x0a)
      {
        wBorderStyle = SS_BORDERSTYLE_DASHDOT;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x0b || xf.dgRight == 0x0b || xf.dgTop == 0x0b || xf.dgBottom == 0x0b)
      {
        wBorderStyle = SS_BORDERSTYLE_FINE_DASH_DOT_DOT;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x0c || xf.dgRight == 0x0c || xf.dgTop == 0x0c || xf.dgBottom == 0x0c)
      {
        wBorderStyle = SS_BORDERSTYLE_DASHDOTDOT;
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (xf.dgLeft == 0x0d || xf.dgRight == 0x0d || xf.dgTop == 0x0d || xf.dgBottom == 0x0d)
      {
  	  //Modify by BOC 99.7.9 (hyt)------------------------------
	    //Slanted dash-dot	should convert to Solid
        //wBorderStyle = SS_BORDERSTYLE_DASHDOT;
		    wBorderStyle = SS_BORDERSTYLE_SOLID;
  	  //--------------------------------------------------------
        icv = (short)(xf.dgLeft?xf.icvLeft:(xf.dgRight?xf.icvRight:(xf.dgTop?xf.icvTop:xf.icvBottom)));
      }
      if (icv == -1 || xl_ColorFromIndex(icv) == RGBCOLOR_DEFAULT)
        color = 0x00000000;
      else
        color = xl_ColorFromIndex(icv);
      SSSetBorder(lpSS->lpBook->hWnd, dwRange+1, SS_ALLROWS, wBorderType, wBorderStyle, color); 
    }
  }

  SelectObject(hDC, hFontOld);

  if (bSetFont)
      DeleteObject(hFont);

  ReleaseDC(lpSS->lpBook->hWnd, hDC);

  return Ret;
}


LPSS_CELLTYPE DLLENTRY SS_XlSetTypeEdit(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style, int Len,
                                        short ChrSet, short ChrCase)
{
//RFW - 8/19/08 - 14744
// This code will always make sure that one of these flags are set.
// When one of these flags is set the Windows edit control is used instead of the internal one.

if ((Style & (ES_MULTILINE | ES_AUTOHSCROLL)) == 0)
	Style = Style | ES_AUTOHSCROLL;

return (SSSetTypeEdit(hWnd, lpCellType, Style, Len, ChrSet, ChrCase));
}
