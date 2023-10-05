// jb - 1.may.01 --------------------------------------------------------------
//
// Take the XML code out of Spread4 and put it in Spread3.  Was going to put
// it in the web.lib, but just too many conflicts/changes....
//
//
//
#ifdef SS_V35

// fix for bug 10020 -scl
// find and replace all occurances of "BUFSIZE" with "BUFSIZ*10"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include "spread.h"
#define STRINGS_RESOURCE 1
#include "..\..\web\src\web.h"
#include "ss_virt.h"
#include "ss_draw.h"
#include "ss_xml.h"
#include "ss_cell.h"
#include "ss_data.h"
#include "ss_doc.h"
#include "ss_user.h"

#ifdef SPREAD_JPN
#include <mbctype.h>
#endif

//GAB 2/17/2005 Added to fix bug 15752
#define MAX_BUFLEN	256

// forward declaration
static BOOL EncodeTag(LPTSTR pszTag, int bufSize);

static BOOL IsBaseChar(WORD c)
{
  if( (0x41 <= c && c <= 0x5A) ||
      (0x61 <= c && c <= 0x7A) ||
      (0xC0 <= c && c <= 0xD6) ||
      (0xD8 <= c && c <= 0xF6) ||
      (0xF8 <= c && c <= 0xFF) ||
      (0x100 <= c && c <= 0x131) ||
      (0x134 <= c && c <= 0x13E) ||
      (0x141 <= c && c <= 0x148) ||
      (0x14A <= c && c <= 0x17E) ||
      (0x180 <= c && c <= 0x1C3) ||
      (0x1CD <= c && c <= 0x1F0) ||
      (0x1F4 <= c && c <= 0x1F5) ||
      (0x1FA <= c && c <= 0x217) ||
      (0x250 <= c && c <= 0x2A8) ||
      (0x2BB <= c && c <= 0x2C1) ||
      (0x386 == c) ||
      (0x388 <= c && c <= 0x38A) ||
      (0x38C == c) ||
      (0x38E <= c && c <= 0x3A1) ||
      (0x3A3 <= c && c <= 0x3CE) ||
      (0x3D0 <= c && c <= 0x3D6) ||
      (0x3DA == c) ||
      (0x3DC == c) ||
      (0x3DE == c) ||
      (0x3E0 == c) ||
      (0x3E2 <= c && c <= 0x3F3) ||
      (0x401 <= c && c <= 0x40C) ||
      (0x40E <= c && c <= 0x44F) ||
      (0x451 <= c && c <= 0x45C) ||
      (0x45E <= c && c <= 0x481) ||
      (0x490 <= c && c <= 0x4C4) ||
      (0x4C7 <= c && c <= 0x4C8) ||
      (0x4CB <= c && c <= 0x4CC) ||
      (0x4D0 <= c && c <= 0x4EB) ||
      (0x4EE <= c && c <= 0x4F5) ||
      (0x4F8 <= c && c <= 0x4F9) ||
      (0x531 <= c && c <= 0x556) ||
      (0x559 == c) ||
      (0x561 <= c && c <= 0x586) ||
      (0x5D0 <= c && c <= 0x5EA) ||
      (0x5F0 <= c && c <= 0x5F2) ||
      (0x621 <= c && c <= 0x63A) ||
      (0x641 <= c && c <= 0x64A) ||
      (0x671 <= c && c <= 0x6B7) ||
      (0x6BA <= c && c <= 0x6BE) ||
      (0x6C0 <= c && c <= 0x6CE) ||
      (0x6D0 <= c && c <= 0x6D3) ||
      (0x6D5 == c) ||
      (0x6E5 <= c && c <= 0x6E6) ||
      (0x905 <= c && c <= 0x939) ||
      (0x93D == c) ||
      (0x958 <= c && c <= 0x961) ||
      (0x985 <= c && c <= 0x98C) ||
      (0x98F <= c && c <= 0x990) ||
      (0x993 <= c && c <= 0x9A8) ||
      (0x9AA <= c && c <= 0x9B0) ||
      (0x9B2 == c) ||
      (0x9B6 <= c && c <= 0x9B9) ||
      (0x9DC <= c && c <= 0x9DD) ||
      (0x9DF <= c && c <= 0x9E1) ||
      (0x9F0 <= c && c <= 0x9F1) ||
      (0xA05 <= c && c <= 0xA0A) ||
      (0xA0F <= c && c <= 0xA10) ||
      (0xA13 <= c && c <= 0xA28) ||
      (0xA2A <= c && c <= 0xA30) ||
      (0xA32 <= c && c <= 0xA33) ||
      (0xA35 <= c && c <= 0xA36) ||
      (0xA38 <= c && c <= 0xA39) ||
      (0xA59 <= c && c <= 0xA5C) ||
      (0xA5E == c) ||
      (0xA72 <= c && c <= 0xA74) ||
      (0xA85 <= c && c <= 0xA8B) ||
      (0xA8D == c) ||
      (0xA8F <= c && c <= 0xA91) ||
      (0xA93 <= c && c <= 0xAA8) ||
      (0xAAA <= c && c <= 0xAB0) ||
      (0xAB2 <= c && c <= 0xAB3) ||
      (0xAB5 <= c && c <= 0xAB9) ||
      (0xABD == c) ||
      (0xAE0 == c) ||
      (0xB05 <= c && c <= 0xB0C) ||
      (0xB0F <= c && c <= 0xB10) ||
      (0xB13 <= c && c <= 0xB28) ||
      (0xB2A <= c && c <= 0xB30) ||
      (0xB32 <= c && c <= 0xB33) ||
      (0xB36 <= c && c <= 0xB39) ||
      (0xB3D == c) ||
      (0xB5C <= c && c <= 0xB5D) ||
      (0xB5F <= c && c <= 0xB61) ||
      (0xB85 <= c && c <= 0xB8A) ||
      (0xB8E <= c && c <= 0xB90) ||
      (0xB92 <= c && c <= 0xB95) ||
      (0xB99 <= c && c <= 0xB9A) ||
      (0xB9C == c) ||
      (0xB9E <= c && c <= 0xB9F) ||
      (0xBA3 <= c && c <= 0xBA4) ||
      (0xBA8 <= c && c <= 0xBAA) ||
      (0xBAE <= c && c <= 0xBB5) ||
      (0xBB7 <= c && c <= 0xBB9) ||
      (0xC05 <= c && c <= 0xC0C) ||
      (0xC0E <= c && c <= 0xC10) ||
      (0xC12 <= c && c <= 0xC28) ||
      (0xC2A <= c && c <= 0xC33) ||
      (0xC35 <= c && c <= 0xC39) ||
      (0xC60 <= c && c <= 0xC61) ||
      (0xC85 <= c && c <= 0xC8C) ||
      (0xC8E <= c && c <= 0xC90) ||
      (0xC92 <= c && c <= 0xCA8) ||
      (0xCAA <= c && c <= 0xCB3) ||
      (0xCB5 <= c && c <= 0xCB9) ||
      (0xCDE == c) ||
      (0xCE0 <= c && c <= 0xCE1) ||
      (0xD05 <= c && c <= 0xD0C) ||
      (0xD0E <= c && c <= 0xD10) ||
      (0xD12 <= c && c <= 0xD28) ||
      (0xD2A <= c && c <= 0xD39) ||
      (0xD60 <= c && c <= 0xD61) ||
      (0xE01 <= c && c <= 0xE2E) ||
      (0xE30 == c) ||
      (0xE32 <= c && c <= 0xE33) ||
      (0xE40 <= c && c <= 0xE45) ||
      (0xE81 <= c && c <= 0xE82) ||
      (0xE84 == c) ||
      (0xE87 <= c && c <= 0xE88) ||
      (0xE8A == c) ||
      (0xE8D == c) ||
      (0xE94 <= c && c <= 0xE97) ||
      (0xE99 <= c && c <= 0xE9F) ||
      (0xEA1 <= c && c <= 0xEA3) ||
      (0xEA5 == c) ||
      (0xEA7 == c) ||
      (0xEAA <= c && c <= 0xEAB) ||
      (0xEAD <= c && c <= 0xEAE) ||
      (0xEB0 == c) ||
      (0xEB2 <= c && c <= 0xEB3) ||
      (0xEBD == c) ||
      (0xEC0 <= c && c <= 0xEC4) ||
      (0xF40 <= c && c <= 0xF47) ||
      (0xF49 <= c && c <= 0xF69) ||
      (0x10A0 <= c && c <= 0x10C5) ||
      (0x10D0 <= c && c <= 0x10F6) ||
      (0x1100 == c) ||
      (0x1102 <= c && c <= 0x1103) ||
      (0x1105 <= c && c <= 0x1107) ||
      (0x1109 == c) ||
      (0x110B <= c && c <= 0x110C) ||
      (0x110E <= c && c <= 0x1112) ||
      (0x113C == c) ||
      (0x113E == c) ||
      (0x1140 == c) ||
      (0x114C == c) ||
      (0x114E == c) ||
      (0x1150 == c) ||
      (0x1154 <= c && c <= 0x1155) ||
      (0x1159 == c) ||
      (0x115F <= c && c <= 0x1161) ||
      (0x1163 == c) ||
      (0x1165 == c) ||
      (0x1167 == c) ||
      (0x1169 == c) ||
      (0x116D <= c && c <= 0x116E) ||
      (0x1172 <= c && c <= 0x1173) ||
      (0x1175 == c) ||
      (0x119E == c) ||
      (0x11A8 == c) ||
      (0x11AB == c) ||
      (0x11AE <= c && c <= 0x11AF) ||
      (0x11B7 <= c && c <= 0x11B8) ||
      (0x11BA == c) ||
      (0x11BC <= c && c <= 0x11C2) ||
      (0x11EB == c) ||
      (0x11F0 == c) ||
      (0x11F9 == c) ||
      (0x1E00 <= c && c <= 0x1E9B) ||
      (0x1EA0 <= c && c <= 0x1EF9) ||
      (0x1F00 <= c && c <= 0x1F15) ||
      (0x1F18 <= c && c <= 0x1F1D) ||
      (0x1F20 <= c && c <= 0x1F45) ||
      (0x1F48 <= c && c <= 0x1F4D) ||
      (0x1F50 <= c && c <= 0x1F57) ||
      (0x1F59 == c) ||
      (0x1F5B == c) ||
      (0x1F5D == c) ||
      (0x1F5F <= c && c <= 0x1F7D) ||
      (0x1F80 <= c && c <= 0x1FB4) ||
      (0x1FB6 <= c && c <= 0x1FBC) ||
      (0x1FBE == c) ||
      (0x1FC2 <= c && c <= 0x1FC4) ||
      (0x1FC6 <= c && c <= 0x1FCC) ||
      (0x1FD0 <= c && c <= 0x1FD3) ||
      (0x1FD6 <= c && c <= 0x1FDB) ||
      (0x1FE0 <= c && c <= 0x1FEC) ||
      (0x1FF2 <= c && c <= 0x1FF4) ||
      (0x1FF6 <= c && c <= 0x1FFC) ||
      (0x2126 == c) ||
      (0x212A <= c && c <= 0x212B) ||
      (0x212E == c) ||
      (0x2180 <= c && c <= 0x2182) ||
      (0x3041 <= c && c <= 0x3094) ||
      (0x30A1 <= c && c <= 0x30FA) ||
      (0x3105 <= c && c <= 0x312C) ||
      (0xAC00 <= c && c <= 0xD7A3) )
      return TRUE;
  return FALSE;
}

static BOOL IsIdeographic(WORD c)
{
  if( (0x4E00 <= c && c <= 0x9FA5) ||
      (0x3007 == c) ||
      (0x3021 <= c && c <= 0x3029) )
      return TRUE;
  return FALSE;
}

static BOOL IsCombiningChar(WORD c)
{
  if( (0x300 <= c && c <= 0x345) ||
      (0x360 <= c && c <= 0x361) ||
      (0x483 <= c && c <= 0x486) ||
      (0x591 <= c && c <= 0x5A1) ||
      (0x5A3 <= c && c <= 0x5B9) ||
      (0x5BB <= c && c <= 0x5BD) ||
      (0x5BF == c) ||
      (0x5C1 <= c && c <= 0x5C2) ||
      (0x5C4 == c) ||
      (0x64B <= c && c <= 0x652) ||
      (0x670 == c) ||
      (0x6D6 <= c && c <= 0x6DC) ||
      (0x6DD <= c && c <= 0x6DF) ||
      (0x6E0 <= c && c <= 0x6E4) ||
      (0x6E7 <= c && c <= 0x6E8) ||
      (0x6EA <= c && c <= 0x6ED) ||
      (0x901 <= c && c <= 0x903) ||
      (0x93C == c) ||
      (0x93E <= c && c <= 0x94C) ||
      (0x94D == c) ||
      (0x951 <= c && c <= 0x954) ||
      (0x962 <= c && c <= 0x963) ||
      (0x981 <= c && c <= 0x983) ||
      (0x9BC == c) ||
      (0x9BE == c) ||
      (0x9BF == c) ||
      (0x9C0 <= c && c <= 0x9C4) ||
      (0x9C7 <= c && c <= 0x9C8) ||
      (0x9CB <= c && c <= 0x9CD) ||
      (0x9D7 == c) ||
      (0x9E2 <= c && c <= 0x9E3) ||
      (0xA02 == c) ||
      (0xA3C == c) ||
      (0xA3E == c) ||
      (0xA3F == c) ||
      (0xA40 <= c && c <= 0xA42) ||
      (0xA47 <= c && c <= 0xA48) ||
      (0xA4B <= c && c <= 0xA4D) ||
      (0xA70 <= c && c <= 0xA71) ||
      (0xA81 <= c && c <= 0xA83) ||
      (0xABC == c) ||
      (0xABE <= c && c <= 0xAC5) ||
      (0xAC7 <= c && c <= 0xAC9) ||
      (0xACB <= c && c <= 0xACD) ||
      (0xB01 <= c && c <= 0xB03) ||
      (0xB3C == c) ||
      (0xB3E <= c && c <= 0xB43) ||
      (0xB47 <= c && c <= 0xB48) ||
      (0xB4B <= c && c <= 0xB4D) ||
      (0xB56 <= c && c <= 0xB57) ||
      (0xB82 <= c && c <= 0xB83) ||
      (0xBBE <= c && c <= 0xBC2) ||
      (0xBC6 <= c && c <= 0xBC8) ||
      (0xBCA <= c && c <= 0xBCD) ||
      (0xBD7 == c) ||
      (0xC01 <= c && c <= 0xC03) ||
      (0xC3E <= c && c <= 0xC44) ||
      (0xC46 <= c && c <= 0xC48) ||
      (0xC4A <= c && c <= 0xC4D) ||
      (0xC55 <= c && c <= 0xC56) ||
      (0xC82 <= c && c <= 0xC83) ||
      (0xCBE <= c && c <= 0xCC4) ||
      (0xCC6 <= c && c <= 0xCC8) ||
      (0xCCA <= c && c <= 0xCCD) ||
      (0xCD5 <= c && c <= 0xCD6) ||
      (0xD02 <= c && c <= 0xD03) ||
      (0xD3E <= c && c <= 0xD43) ||
      (0xD46 <= c && c <= 0xD48) ||
      (0xD4A <= c && c <= 0xD4D) ||
      (0xD57 == c) ||
      (0xE31 == c) ||
      (0xE34 <= c && c <= 0xE3A) ||
      (0xE47 <= c && c <= 0xE4E) ||
      (0xEB1 == c) ||
      (0xEB4 <= c && c <= 0xEB9) ||
      (0xEBB <= c && c <= 0xEBC) ||
      (0xEC8 <= c && c <= 0xECD) ||
      (0xF18 <= c && c <= 0xF19) ||
      (0xF35 == c) ||
      (0xF37 == c) ||
      (0xF39 == c) ||
      (0xF3E == c) ||
      (0xF3F == c) ||
      (0xF71 <= c && c <= 0xF84) ||
      (0xF86 <= c && c <= 0xF8B) ||
      (0xF90 <= c && c <= 0xF95) ||
      (0xF97 == c) ||
      (0xF99 <= c && c <= 0xFAD) ||
      (0xFB1 <= c && c <= 0xFB7) ||
      (0xFB9 == c) ||
      (0x20D0 <= c && c <= 0x20DC) ||
      (0x20E1 == c) ||
      (0x302A <= c && c <= 0x302F) ||
      (0x3099 == c) ||
      (0x309A == c) )
      return TRUE;
  return FALSE;
}

static BOOL IsDigit(WORD c)
{
  if( (0x30 <= c && c <= 0x39) ||
      (0x660 <= c && c <= 0x669) ||
      (0x6F0 <= c && c <= 0x6F9) ||
      (0x966 <= c && c <= 0x96F) ||
      (0x9E6 <= c && c <= 0x9EF) ||
      (0xA66 <= c && c <= 0xA6F) ||
      (0xAE6 <= c && c <= 0xAEF) ||
      (0xB66 <= c && c <= 0xB6F) ||
      (0xBE7 <= c && c <= 0xBEF) ||
      (0xC66 <= c && c <= 0xC6F) ||
      (0xCE6 <= c && c <= 0xCEF) ||
      (0xD66 <= c && c <= 0xD6F) ||
      (0xE50 <= c && c <= 0xE59) ||
      (0xED0 <= c && c <= 0xED9) ||
      (0xF20 <= c && c <= 0xF29) )
      return TRUE;
  return FALSE;
}

static BOOL IsExtender(WORD c)
{
  if( (0xB7 == c) ||
      (0x2D0 <= c && c <= 0x2D1) ||
      (0x387 == c) ||
      (0x640 == c) ||
      (0xE46 == c) ||
      (0xEC6 == c) ||
      (0x3005 == c) ||
      (0x3031 <= c && c <= 0x3035) ||
      (0x303D <= c && c <= 0x309E) ||
      (0x30FC <= c && c <= 0x30FE) )
      return TRUE;
  return FALSE;
}

static BOOL IsLetter(WORD c)
{
  return IsBaseChar(c) || IsIdeographic(c);
}

static BOOL IsValidNameStartChar(WORD c)
{
  return c == (WCHAR)':' || 
         c == (WCHAR)'_' || 
         IsLetter(c);
}

static BOOL IsValidNameChar(WORD c)
{
  return c == (WCHAR)':' || 
         c == (WCHAR)'_' || 
         IsLetter(c) || 
         IsDigit(c) || 
         IsCombiningChar(c) || 
         IsExtender(c);
}

// jb - 16.may.01 ------------------------------------------------------------------
//
// Write the information message to logfile specified from Export method.
//
static void writemsg2log (SS_XML *pXML)
{
	FILE *fp;

	if (pXML->pszLogFile != 0)
	{	fp = _tfopen (pXML->pszLogFile, _T("a"));
		if (!fp) return;
		_fputts(pXML->tcBuf, fp);
		fclose (fp);
	}
	pXML->tcBuf[0] = 0;

	return;
}

// jb - 16.may.01 --------------------------------------------------------------
//
// Write information to the log file about what we intend to do.
//
static HeaderLog_XML (SS_XML *pXML)
{
	TCHAR tcFormat[BUFSIZ*10];
	TCHAR tcTimeBuf[32];

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD1_XML, tcFormat, BUFSIZ*10-1);
	_stprintf (pXML->tcBuf, tcFormat, getdatetimestring (tcTimeBuf), pXML->pXMLName);
	writemsg2log (pXML);

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD3, tcFormat, BUFSIZ*10-1);
	_stprintf (pXML->tcBuf, tcFormat);
	writemsg2log (pXML);
	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD4, tcFormat, BUFSIZ*10-1);
	_stprintf (pXML->tcBuf, tcFormat);
	writemsg2log (pXML);
}

// jb - 16.may.01 --------------------------------------------------------------
//
// Write information to the log file about what we intend to do.
//
static HeaderLog_XMLBUFF (SS_XML *pXML)
{
	TCHAR tcFormat[BUFSIZ*10];
	TCHAR tcTimeBuf[32];

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD1_XMLBUFF, tcFormat, BUFSIZ*10-1);
	_stprintf (pXML->tcBuf, tcFormat, getdatetimestring (tcTimeBuf));
	writemsg2log (pXML);

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD3, tcFormat, BUFSIZ*10-1);
	_stprintf (pXML->tcBuf, tcFormat);
	writemsg2log (pXML);
	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD4, tcFormat, BUFSIZ*10-1);
	_stprintf (pXML->tcBuf, tcFormat);
	writemsg2log (pXML);
}

// jb - 02.nov.98 ------------------------------------------------------------------
//
// Write out 
//
void write_XML (SS_XML *pXML)
{
	LPTSTR pszBuf;
// fix for bug 9293 -scl
  LPWSTR wstr = NULL;
  LPSTR str = NULL;
  long len = 0;

  if( pXML->fp == NULL )
    return;

  pszBuf = GlobalLock (pXML->ghBuf);

//GAB 1/15/04 - modified for unicode
#if defined(UNICODE) || defined(_UNICODE)
	len = WideCharToMultiByte(CP_UTF8, 0, pszBuf, -1, NULL, 0, NULL, NULL);
	str = (LPSTR)malloc(len * sizeof(char));
	if( str )
	{
	  memset(str, 0, len * sizeof(char));
	  WideCharToMultiByte(CP_UTF8, 0, pszBuf, -1, str, len, NULL, NULL);
  	  fputs(str, pXML->fp);
	  free(str);
	}
#else
  len = (long)strlen(pszBuf);
  len = MultiByteToWideChar(CP_ACP, 0, pszBuf, -1, NULL, 0);
  wstr = (LPWSTR)malloc(len * sizeof(WCHAR));
  if( wstr )
  {
	memset(wstr, 0, len * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, pszBuf, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	str = (LPSTR)malloc(len * sizeof(char));
	if( str )
	{
	  memset(str, 0, len * sizeof(char));
	  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
  	  fputs(str, pXML->fp);
	  free(str);
	}
	free(wstr);
  }
#endif

  GlobalUnlock (pXML->ghBuf);
  GlobalFree (pXML->ghBuf);

  return;
}

// jb - 25.aug.99 ---------------------------------------------------------
//
// Compliments of Robby, needed to implement virtual mode in the event a customer
// will export during virtual mode.
//
void virtualmode (LPSPREADSHEET lpSS, SS_COORD *lRowStart, SS_COORD *lRowEnds)
{
    BOOL fNoVirtual = TRUE;
    BOOL fContinueProcessing = TRUE;
    BOOL fAtVirtualEnd = FALSE;
	SS_COORD lRowAt = *lRowStart;

	while (fContinueProcessing)
	{	if (lpSS->Virtual.VMax != 0 && lpSS->Row.Max > 0 && !fAtVirtualEnd)
		{	fNoVirtual = FALSE;
			*lRowStart = lRowAt;

			while ((*lRowStart < lpSS->Virtual.VTop || 
					*lRowStart >= lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize) && 
					!fAtVirtualEnd)
			{	lpSS->Row.UL = lRowAt;
				fContinueProcessing = TRUE;
				fAtVirtualEnd = SS_VQueryData(lpSS, lRowAt);

				if (lpSS->Virtual.VMax != -1 && 
					lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - lpSS->Row.HeaderCnt >= lpSS->Virtual.VMax)
				{	fAtVirtualEnd = TRUE;
				}
			}

			*lRowEnds += lpSS->Virtual.VSize;
		}
		else
		{	fContinueProcessing = FALSE;
		}
	}

	return;
}

// jb - 05.nov.98 ---------------------------------------------------------
//
// If the user just specified a file without the path, then get the current working
// directory and put the filename on.  If the forget a filename, use "default"
// with extension passed in.
//
LPTSTR ValidateFileName (LPCTSTR lpcFileName, TCHAR *tcPath, LPCTSTR lpcExt)
{
   LPTSTR pValidName;
	TCHAR tcBuf[BUFSIZ*10+1];
	size_t uSize;

	TCHAR tcDrive[_MAX_DRIVE], 
		  tcDir[_MAX_DIR], 
		  tcBaseName[_MAX_FNAME], 
		  tcExt[_MAX_EXT];
	
	memset (tcBuf, 0, BUFSIZ*10);
	tcDrive[0] = tcDir[0] = tcBaseName[0] = tcExt[0] = 0;
	_tsplitpath (lpcFileName, tcDrive, tcDir, tcBaseName, tcExt);

	// Do we have a drive letter ?
	//
	if (tcDrive[0] != 0)
	{	_tcsncpy (tcBuf, tcDrive, 10);
	}

	// If we don't have a directory, then forget the drive.  We are going to use the
	// current working directory.
	//
	if (tcDir[0] == 0)
	{	_tgetcwd (tcBuf, _MAX_PATH);
		_tcscat (tcBuf, _T("\\"));
	}

	// Add our path to the drive letter if there is a drive letter, otherwise,
	// just add the path.
	//
	else
	{	if (tcBuf[0] != 0)
		{	_tcscat (tcBuf, tcDir);
		}
		else
		{	_tcscpy (tcBuf, tcDir);
		}
	}
		
	// Get our path so we can use it for stuff we may need later.
	//
   if (tcPath)
	   _tcsncpy (tcPath, tcBuf, _MAX_PATH);
	
	// Lets see if we have a file name.
	//
	if (tcBaseName[0] == 0)
	{	return NULL;
	}

	// We have a filename so put it on our path.  If there is no extension, use passed in
	// extension.  Make sure we have a dot before the extension.
	//
	else
	{	_tcscat (tcBuf, tcBaseName);
		if (tcExt[0] != 0)
		{	if (tcExt[0] != L'.')
			{	_tcscat (tcBuf, _T("."));
				_tcscat (tcBuf, tcExt);
			}
			else
			{	_tcscat (tcBuf, tcExt);
			}
		}
		else
		{	if (*lpcExt != L'.')
			{	_tcscat (tcBuf, _T("."));
				_tcscat (tcBuf, lpcExt);
			}
			else
			{	_tcscat (tcBuf, lpcExt);
			}
		}
	}

//GAB	10/17/01 Modified for Unicode
//		uSize = (_tcslen (tcBuf) + 1) * sizeof(TCHAR);
//		*ppValidName = (LPTSTR)calloc (1, uSize);
	uSize = _tcslen (tcBuf);
	pValidName = (LPTSTR)calloc (1, (uSize + 1) * sizeof(TCHAR));
	if (!pValidName)
	{	return NULL;
	}

	_tcsncpy (pValidName, tcBuf, uSize);
//GAB	10/17/01 Modified for Unicode
//		(pValidName)[uSize-1] = 0;
	(pValidName)[uSize] = 0;
	
	return pValidName;
}

// jb - 18.jan.01 -------------------------------------------------------------
//
// Clean up our resources we allocated.
//
static BOOL CleanUp (SS_XML *pXML)
{
	BOOL bRet = FALSE;
	short i;

	if (pXML->fp != NULL)
	{	fclose (pXML->fp);
		bRet = TRUE;
	}
	
	// Free the XML data tags first.
	//
	for (i = 0; i < MAXNUM_TAGS; i++)
	{	if (pXML->Tags[i].pDataTags)
		{	free (pXML->Tags[i].pDataTags);
		}
		else
		{	break;
		}
	}

	if (pXML->pPath)
	{	free (pXML->pPath);
	}

	if (pXML->pXMLName)
	{	free (pXML->pXMLName);
	}

	if (pXML->ghLogFile)
	{	GlobalFree (pXML->ghLogFile);
	}

	free (pXML);

	return bRet;
}

// jb - 6.sep.00 --------------------------------------------------------------
//
// Broke up the init so would not be passing so many parameters...
//
static void InitXML (SS_XML *pXML, LPCTSTR pszRoot, LPCTSTR pszCollection,
long lFlags, LPCTSTR pcszLogFile)
{
	int i;
	int len;
//GAB 2/17/2005 - 15752  TCHAR buf[90];
	TCHAR buf[MAX_BUFLEN];

	pXML->pPath = NULL;
	pXML->pXMLName = NULL;
	pXML->bFirstPass = TRUE;
	pXML->lFlags = lFlags;
	pXML->nBufSize = 4096;
	pXML->ghBuf = GlobalAlloc (GHND, pXML->nBufSize);
	pXML->iNumTags = 0;
	if (pcszLogFile == NULL || _tcslen(pcszLogFile) == 0)
	{	pXML->pszLogFile = 0;
		pXML->ghLogFile = 0;
	}


// GAB 10/17/01 Modified for Unicode

	else
	{	len = (int)_tcslen (pcszLogFile);
		pXML->ghLogFile = GlobalAlloc (GHND, (len + 1) * sizeof(TCHAR));
		pXML->pszLogFile = GlobalLock (pXML->ghLogFile);
		_tcsncpy (pXML->pszLogFile, pcszLogFile, len);
		pXML->pszLogFile[len] = 0;
	}


	// If the method passed in a root or collection, use it.
	//
	if (pszRoot)
	{	if (_tcslen (pszRoot) > 0)
		{	
      _tcscpy(buf, pszRoot);
//GAB 2/17/2005 - 15752      EncodeTag(buf, 90);
      EncodeTag(buf, MAX_BUFLEN);

      _tcscpy (pXML->root, _T("<"));
//GAB 2/17/2005 - 15752			_tcsncat (pXML->root, buf, 89);
	  _tcsncat (pXML->root, buf, MAX_BUFLEN - 1);
			_tcscat (pXML->root, CLOSECR);
		}

		// If no root or collection, then default.
		//
		else
		{	_tcscpy (pXML->root, SPREAD);
		}
	}

	// If no root or collection, then default.
	//
	else
	{	_tcscpy (pXML->root, SPREAD);
	}

	if (pszCollection)
	{	if (_tcslen (pszCollection)  > 0)
		{	
      _tcscpy(buf, pszCollection);
//GAB 2/17/2005 - 15752      EncodeTag(buf, 90);
      EncodeTag(buf, MAX_BUFLEN);
      _tcscpy (pXML->collection, _T("\t<"));
//GAB 2/17/2005 - 15752			_tcsncat (pXML->collection, buf, 89);
	  _tcsncat (pXML->collection, buf, MAX_BUFLEN - 1);

			_tcscat (pXML->collection, CLOSECR);
		}
		else
		{	_tcscpy (pXML->collection, TABROW);
		}
	}

	// Just have a collection of row nodes...the user can change as they
	// see fit.  Just the data nodes will be unique.
	//
	else
	{	_tcscpy (pXML->collection, TABROW);
	}

	if (pXML->pszLogFile != 0)
	{
//GAB 10/18/01 Modified for Unicode
		//remove (pXML->pszLogFile);
		_tremove (pXML->pszLogFile);
	}

	for (i = 0; i < MAXNUM_TAGS; i++)
	{	pXML->Tags[i].pDataTags = NULL;
	}

	return;
}

/*
static void InitXML (SS_XML *pXML, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR pcszLogFile)
{
	int i;
	int len;

	pXML->pPath = NULL;
	pXML->pXMLName = NULL;
	pXML->bFirstPass = TRUE;
	pXML->lFlags = lFlags;
	pXML->nBufSize = 4096;
	pXML->ghBuf = GlobalAlloc (GHND, pXML->nBufSize);
	pXML->iNumTags = 0;
	if (pcszLogFile == NULL || _tcslen(pcszLogFile) == 0)
	{	pXML->pszLogFile = 0;
		pXML->ghLogFile = 0;
	}
	else
	{	len = (_tcslen (pcszLogFile) + 1) * sizeof (TCHAR);
		pXML->ghLogFile = GlobalAlloc (GHND, len);
		pXML->pszLogFile = GlobalLock (pXML->ghLogFile);
		_tcsncpy (pXML->pszLogFile, pcszLogFile, len-1);
		pXML->pszLogFile[len-1] = 0;
	}

	// If the method passed in a root or collection, use it.
	//
	if (pszRoot)
	{	if (_tcslen (pszRoot) > 0)
		{	_tcscpy (pXML->root, _T("<"));
			_tcsncat (pXML->root, pszRoot, 89);
			_tcscat (pXML->root, CLOSECR);
		}

		// If no root or collection, then default.
		//
		else
		{	_tcscpy (pXML->root, SPREAD);
		}
	}

	// If no root or collection, then default.
	//
	else
	{	_tcscpy (pXML->root, SPREAD);
	}

	if (pszCollection)
	{	if (_tcslen (pszCollection)  > 0)
		{	_tcscpy (pXML->collection, _T("\t<"));
			_tcsncat (pXML->collection, pszCollection, 89);
			_tcscat (pXML->collection, CLOSECR);
		}
		else
		{	_tcscpy (pXML->collection, TABROW);
		}
	}

	// Just have a collection of row nodes...the user can change as they
	// see fit.  Just the data nodes will be unique.
	//
	else
	{	_tcscpy (pXML->collection, TABROW);
	}

	if (pXML->pszLogFile != 0)
	{	remove (pXML->pszLogFile);
	}

	for (i = 0; i < MAXNUM_TAGS; i++)
	{	pXML->Tags[i].pDataTags = NULL;
	}

	return;
}
*/
// jb - 4.apr.01 --------------------------------------------------------------
//
// Load the XML buffer or write it to a file.
//
static void LoadBuffer (SS_XML *pXML)
{
	LPTSTR pszBuf = NULL;
	int nBufLen, nMoreDataLen;
	
	if (!pXML->bFirstPass)
	{	pszBuf = GlobalLock (pXML->ghBuf);
		nBufLen = (int)_tcslen (pszBuf);
		GlobalUnlock (pXML->ghBuf);
		nMoreDataLen = (int)_tcslen(pXML->tcBuf);

		// We started with 4K for our buffer.  If we go over that, then give our buffer
		// another 4K
		//
		if ((nMoreDataLen + nBufLen + 1) * (int)sizeof(TCHAR) > pXML->nBufSize)
		{	pXML->nBufSize += 4096;
  		pXML->ghBuf = GlobalReAlloc (pXML->ghBuf, pXML->nBufSize, GHND);
		}
	}

	pszBuf = (LPTSTR)GlobalLock (pXML->ghBuf);

	if (pXML->bFirstPass)
	{	
// fix for japan bug (missing encoding attribute) -scl
#ifdef SS_V40
    _tcscpy (pszBuf, _T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"));
#else
    _tcscpy (pszBuf, _T("<?xml version=\"1.0\" ?>\n"));
#endif
		_tcscat (pszBuf, pXML->tcBuf);
		pXML->bFirstPass = FALSE;
	}
	else
	{	_tcscat (pszBuf, pXML->tcBuf);
	}
	GlobalUnlock (pXML->ghBuf);
	memset (pXML->tcBuf, 0, BUFSIZ*10);

	return;
	
}

// jb - 6.sep.00 --------------------------------------------------------------
//
// Make an ending tag.  Address would become </Address> that ends the tag.
//
static void EndTag (TCHAR *tcBuf, TCHAR *node, short numtabs)
{
	TCHAR *ptr;

	// The root, main node.
	//
	if (!numtabs || numtabs == 2)
	{	_tcscpy (tcBuf, _T("</"));
	}

	// The collection
	//
	else if (numtabs == 1)
	{	_tcscpy (tcBuf, _T("\t</"));
	}

	ptr = _tcschr (node, 60);

	if (ptr)
	{	ptr++;
//GAB 2/17/2005 - 15752	_tcsncat (tcBuf, ptr, 89);
		_tcsncat (tcBuf, ptr, MAX_BUFLEN - 1);
	}

	// If it is an ending tag on the datatag, put a carriage return.
	//
	if (numtabs == 2)
	{	_tcscat (tcBuf, _T("\n"));
	}

	return;
}

// jb - 18.sep.00 --------------------------------------------------------------
//
// Store our data tag in buffer.  If there is no datatag, (column headers are blank), then
// default to <Col1>, <Col2> etc.
//
static void StoreDataTag (SS_XML *pXML, LPTSTR pszText)
{
 	if (pszText != NULL)
 	{	_tcscpy (pXML->tcBuf, TAB2);
//GAB 2/17/2005 15752 		_tcsncat (pXML->tcBuf, pszText, 89);
 		_tcsncat (pXML->tcBuf, pszText, MAX_BUFLEN - 1);
 	}
 	else
 	{	_tcscpy (pXML->tcBuf, _T("\t\t<Col"));
 	}

	return;
}

// jb - 27.oct.98 ---------------------------------------------------------
//
// We want the data in the column header.  The user may have no text
// in the header, numbers, or letters.  Also, they may have altered the 
// StartingColumnNumber.  For a blank header, we put &nbsp; 
// 
// removed / replaced with getheaderdata() in web.cpp -scl
/*
static BOOL getcolumnheaderdata (LPSPREADSHEET lpSS, SS_COORD lCol, LPTSTR pBuf)
{
	WORD wCol;
	TCHAR tcLocalBuf[16];
	SS_COORD StartRowNum, StartColNum;

	SS_GetStartingNumbers(lpSS, &StartColNum, &StartRowNum);
	lCol += (StartColNum -1);

	wCol = SS_GetColHeaderDisplay(lpSS);
	if (wCol == SS_HEADERDISPLAY_BLANK ||
		wCol == SS_HEADERDISPLAY_NUMBERS)
	{ 	_tcscpy (pBuf, (LPTSTR)_itot(lCol, tcLocalBuf, 10));
	}

	else if (wCol == SS_HEADERDISPLAY_LETTERS)
	{	SS_DrawFormatHeaderLetter (pBuf, lCol);
	}

	else
		*pBuf = 0;

	return TRUE;
}
*/
// Encodes the tag characters that are invalid:
// i.e. "_xFFDB_"
static BOOL EncodeTag(LPTSTR pszTag, int bufSize)
{
  TCHAR buf[8];
// 15348 -scl
#ifndef _UNICODE
  WCHAR u[2];
#endif
  WORD c;
  int n, len, replen;
  BOOL bLeadByte;

  len = lstrlen(pszTag);
  if( !len )
    return FALSE;
// 15348 -scl
#if _UNICODE
   bLeadByte = FALSE;
   n = 1;
   c = pszTag[0];
   if( !IsValidNameStartChar(c) )
   {  // encode character
      replen = 6;
      if( len + replen < bufSize )
      {
         memcpy(&pszTag[replen], pszTag, len * sizeof(TCHAR));
         _stprintf(buf, _T("_x%04x_"), (int)c);
         memcpy(pszTag, buf, 7 * sizeof(TCHAR));
         len += replen;
         pszTag[len] = 0;
         n = 7;
      }
      else // no room to insert encoded value
         return FALSE;
   }
   for( ; n < len; n++ )
   { // encode characters if necessary
      c = pszTag[n];
      if( !IsValidNameChar(c) )
      { // encode character
         replen = 6;
         if( len+replen < bufSize )
         {
            memcpy(&pszTag[n+replen], &pszTag[n], (len-n) * sizeof(TCHAR));
            _stprintf(buf, _T("_x%04x_"), (int)c);
            memcpy(&pszTag[n], buf, 7 * sizeof(TCHAR));
            len += replen;
            pszTag[len] = 0;
            n += 6;
         }
         else // no room to insert encoded value
            return FALSE;
      }
   }
#else
  bLeadByte = IsDBCSLeadByte(((char*)pszTag)[0]);
  if( bLeadByte )
    n = 2;
  else
    n = 1;
  if( MultiByteToWideChar(CP_ACP, 0, (char*)(&pszTag[0]), bLeadByte ? 2 : 1, u, 2) )
  {
    c = u[0];
    if( !IsValidNameStartChar(c) )
    {  // encode character
      replen = (bLeadByte ? 5 : 6);
      if( len + replen < bufSize )
      {
         memcpy(&pszTag[replen], pszTag, len * sizeof(TCHAR));
        _stprintf(buf, _T("_x%04x_"), (int)c);
        memcpy(pszTag, buf, 7 * sizeof(TCHAR));
        len += replen;
        pszTag[len] = 0;
        n = 7;
      }
      else // no room to insert encoded value
        return FALSE;
    }
  }
  for( ; n < len; bLeadByte ? n+=2 : n++ )
  { // encode characters if necessary
    bLeadByte = IsDBCSLeadByte(((char*)pszTag)[n]);
    if( MultiByteToWideChar(CP_ACP, 0, (char*)(&pszTag[n]), bLeadByte ? 2 : 1, u, 2) )
    {
      c = u[0];
      if( !IsValidNameChar(c) )
      { // encode character
        replen = (bLeadByte ? 5 : 6);
        if( len+replen < bufSize )
        {
         memcpy(&pszTag[n+replen], &pszTag[n], (len-n) * sizeof(TCHAR));
         _stprintf(buf, _T("_x%04x_"), (int)c);
          memcpy(&pszTag[n], buf, 7 * sizeof(TCHAR));
          len += replen;
          pszTag[len] = 0;
          n += (bLeadByte ? 5 : 6);
        }
        else // no room to insert encoded value
          return FALSE;
      }
    }
  }
#endif
  return TRUE;
}
// jb - 18.sep.00 --------------------------------------------------------------
//
// Need the data tag names from the column headers.  If there are no column headers, we will 
// default to col1, col2, -- coln  Build this dynamically, since we do not no how many data
// tags we have.
//
//	<root>
//		<col1>
//			<unique_datatag>some data</unique_datatag>
//		</col>
//		<col2>
//			<...>etc</...>
//		</col2>
//	</root>
//
static BOOL GetDataTags (LPSPREADSHEET lpSS, PWEB_COORD pWebCOORD, SS_XML *pXML)
{
	BOOL bRet = TRUE;
	int i, start = lpSS->Col.HeaderCnt;
	size_t uSize;
	TCHAR pszBuf[BUFSIZ*10];
  WORD c;

	// We want the top row of column headers for our header names.
	//
	if (pWebCOORD->lColStart >= lpSS->Col.HeaderCnt)
	{	start = pWebCOORD->lColStart;
	}

	for (i = start; i <= pWebCOORD->lColEnd; i++)
	{  	// If there is no data in the header, then get the letters or numbers.  If
		// the headers are blank, default to numbers.
		//
#ifdef SS_V40
// fix for bug 9850 -scl
//    if (!SS_GetDataEx (lpSS, i, lpSS->Row.lHeaderDisplayIndex, pszBuf, -1))
    if (!SS_GetDataEx (lpSS, i, -1 == lpSS->Row.lHeaderDisplayIndex ? lpSS->Row.HeaderCnt - 1 : lpSS->Row.lHeaderDisplayIndex, pszBuf, -1) || !*pszBuf )
#else
    if (!SS_GetDataEx (lpSS, i, 0, pszBuf, -1) || !*pszBuf )
#endif
		{	
//      getcolumnheaderdata (lpSS, i, pszBuf);
      c = lpSS->ColHeaderDisplay;
      lpSS->ColHeaderDisplay = SS_HEADERDISPLAY_LETTERS;
      getheaderdata(lpSS, i, FALSE, pszBuf);
      lpSS->ColHeaderDisplay = c;
		}

// fix for bug 9753/9912 -scl
// check for a valid XML tag
    EncodeTag(pszBuf, BUFSIZ*10);
		// Put our data tag in the buffer.
		//
		StoreDataTag (pXML, pszBuf);

		// We add extra bytes for the possiblity of 2 tabs, opening and closing brackets, and a null.
		//
//GAB 10/17/01 Modified for Unicode
//			uSize = (_tcslen (pXML->tcBuf) + 10) * sizeof (TCHAR);
//			pXML->Tags[pXML->iNumTags].pDataTags = (LPTSTR)calloc (1, uSize);
//			memset (pXML->Tags[pXML->iNumTags].pDataTags, 0, uSize);
		uSize = _tcslen (pXML->tcBuf);
		pXML->Tags[pXML->iNumTags].pDataTags = (LPTSTR)calloc (1, (uSize + 10) * sizeof (TCHAR));
		memset (pXML->Tags[pXML->iNumTags].pDataTags, 0, uSize + 10);

		if (!pXML->Tags[pXML->iNumTags].pDataTags)
		{	bRet = FALSE;
			break;
		}

		// Two tabs to indent for data tags.
		//
		if (!pszBuf)
		{ 
//GAB 	10/17/01 Modified for Unicode
			//_tcsncpy (pXML->Tags[pXML->iNumTags].pDataTags, pXML->tcBuf, uSize - 11);
			_tcsncpy (pXML->Tags[pXML->iNumTags].pDataTags, pXML->tcBuf, uSize);
			_tcscat (pXML->Tags[pXML->iNumTags].pDataTags, _itot (i, pXML->tcBuf, 10));
		}
		else
		{	_tcsncat (pXML->Tags[pXML->iNumTags].pDataTags, pXML->tcBuf, uSize);
		}

		// For the data tags, don't put a carriage return at the end...
		//
		_tcscat (pXML->Tags[pXML->iNumTags].pDataTags, _T(">"));
		pXML->iNumTags++;
	}

	return bRet;
}

// jb - 10.sep.00 -------------------------------------------------------------
//
// Write out the data tag.  Take into account the range with the offset variable.
// We set offset to one since we don't use column 1, but we store tags starting
// in the array 0.  At some point, may need to re-visit how we want to address
// multiple row headers.
//
static void SaveDataTag (SS_XML *pXML, PWEB_COORD pWebCOORD, LPTSTR pszText, int idx)
{
	if (pszText && _tcslen(pszText))
	{	if (pXML->Tags[idx].pDataTags != 0)
		{	_tcsncpy (pXML->tcBuf, pXML->Tags[idx].pDataTags, MAXLEN_TAGS);
			_tcsncat (pXML->tcBuf, pszText, BUFSIZ*10 - MAXLEN_TAGS - 2);
			LoadBuffer (pXML);
			EndTag (pXML->tcBuf, pXML->Tags[idx].pDataTags, 2);
			LoadBuffer (pXML);
		}
	}

	return;
}

// jb - 18.sep.00 --------------------------------------------------------------
//
// Need our XML name to write too, and the path.
//
static BOOL InitXMLName (SS_XML *pXML, LPCTSTR pszFile)
{
	TCHAR tcFormat[BUFSIZ*10];
	BOOL bRet = FALSE;
	size_t uSize;

	// ValidateFileName () will always return a valid filename.  If none was specified, it returns
	// the current directory with "default" for the basename and whatever was specified for the extension.
	//
	pXML->pXMLName = ValidateFileName (pszFile, pXML->tcBuf, _T(".xml"));
	if (pXML->pXMLName == NULL)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FILENOTFOUND, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pszFile);
		writemsg2log (pXML);
		return bRet;
	}
/*
	uSize = (_tcslen (pXML->tcBuf) + 1) * sizeof(TCHAR);
	pXML->pPath = (LPTSTR)calloc (1, uSize);
	if (!pXML->pPath)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_DIRECTORYNOTFOUND, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pszFile);
		writemsg2log (pXML);
		return bRet;
	}
	_tcsncpy (pXML->pPath, pXML->tcBuf, uSize);
	pXML->pPath[uSize-1] = 0;
*/
// GAB 10/17/01 modified for Unicode
	uSize = _tcslen (pXML->tcBuf);
	pXML->pPath = (LPTSTR)calloc (1, (uSize + 1) * sizeof(TCHAR));
	if (!pXML->pPath)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_DIRECTORYNOTFOUND, tcFormat, BUFSIZ*10);
		_stprintf (pXML->tcBuf, tcFormat, pszFile);
		writemsg2log (pXML);
		return bRet;
	}
	_tcsncpy (pXML->pPath, pXML->tcBuf, uSize);
	pXML->pPath[uSize] = 0;


	return TRUE;
}

// jq - 12.nov.98 ------------------------------------------------------------------
//
// Get the type of 'type' from a particular cell.
//
static BOOL gettypeoftype (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LONG nType)
{
	BOOL bRet = FALSE;
	SS_CELLTYPE cellType;

	if (SS_RetrieveCellType (lpSS, &cellType, NULL, lCol, lRow))
	{	if (cellType.Type == nType)
		{	bRet = TRUE;
		}
	}

	return bRet;
}

// jb - 04.feb.99 ----------------------------------------------------------------------------
//
// We need to know if user has some pictures that are in the last cell (col or row) because
// the data count will not recognize this.
// 
void getothercnt (LPSPREADSHEET lpSS, SS_COORD *lColCnt, SS_COORD *lRowCnt)
{
	SS_CELLTYPE CellType;
	SS_COORD lRow, lCol;
	SS_COORD lMaxPictRow, lMaxPictCol;
	SS_COORD lMaxRows = SS_GetMaxRows (lpSS) + lpSS->Row.HeaderCnt - 1;
	SS_COORD lMaxCols = SS_GetMaxCols (lpSS) + lpSS->Col.HeaderCnt - 1;

	lMaxPictRow = lMaxPictCol = 0;

	for (lRow = lpSS->Row.HeaderCnt; lRow <= lMaxRows; lRow++)
	{	for (lCol = lpSS->Col.HeaderCnt; lCol <= lMaxCols; lCol++)
		{	if (gettypeoftype (lpSS, lCol, lRow, SS_TYPE_PICTURE))
			{	SS_RetrieveCellType (lpSS, &CellType, NULL, lCol, lRow);

				// Don't count an empty picture cell.
				//
				if (CellType.Spec.ViewPict.hPictName)
				{	if (lMaxPictCol < lCol)
					{	lMaxPictCol = lCol;
					}
					lMaxPictRow = lRow;
				}
			}

			// Check for 'checkboxes'
			//
			if (gettypeoftype (lpSS, lCol, lRow, SS_TYPE_CHECKBOX))
			{	SS_RetrieveCellType (lpSS, &CellType, NULL, lCol, lRow);

				if (lMaxPictCol < lCol)
				{	lMaxPictCol = lCol;
				}
				lMaxPictRow = lRow;
			}
		}
	}

	if (*lColCnt < lMaxPictCol) *lColCnt = lMaxPictCol;
	if (*lRowCnt < lMaxPictRow) *lRowCnt = lMaxPictRow;

	return;
}

// jb - 14.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 for the first column and row coordinate.  This means we 
// are exporting the entire spreadsheet.  SS_ALLCOLS and SS_ALLROWS
//
BOOL entirespreadsheet (SS_COORD lCol, SS_COORD lRow)
{
	BOOL fRet = FALSE;

	if (lCol == SS_ALLCOLS && lRow == SS_ALLROWS) fRet = TRUE;
	return fRet;
}

// jb - 17.sep.98 ---------------------------------------------------------------
//
// Check to see if the coordinates passed in are greater than -1
//
static BOOL validatecoordinates (SS_COORD Col1, SS_COORD Row1, SS_COORD Col2, SS_COORD Row2, SS_XML *pXML)
{
	TCHAR tcFormat[BUFSIZ*10];
	BOOL bRet = TRUE;

	pXML->tcBuf[0] = 0;

	// No such coordinate less than -1.
	//
	if (Col1 < -1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_COL1INVALID, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, Col1, Row1);
	}

	else if (Col2 < -1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_COL2INVALID, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, Col1, Row1, Col2);
	}

	else if (Row1 < -1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ROW1INVALID, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, Col1, Row1);
	}

	else if (Row2 < -1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ROW2INVALID, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, Col1, Row1, Row2);
	}

	else if (Col1 > Col2)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_COLRANGEERROR, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, Col1, Row1, Col2);
	}

	else if (Row1 > Row2)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ROWRANGEERROR, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, Col1, Row1, Row2);
	}

	if (pXML->tcBuf[0] != 0)
	{ 	writemsg2log (pXML);
		bRet = FALSE;
	}

	return bRet;
}

// jb - 15.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 (SS_ALLCOLS) for the second column coordinate.  If
// so, then user wants to do a range of column cells.
//
BOOL columnrange (SS_COORD lCol, SS_COORD lRow)
{
	BOOL fRet = FALSE;

	if (lCol == SS_ALLCOLS && lRow >= 0) fRet = TRUE;
	return fRet;
}

// jb - 15.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 (SS_ALLROWS) for the second row coordinate.  If so,
// then user wants to do a range of rows.
//
BOOL rowrange (SS_COORD lCol, SS_COORD lRow)
{	
	BOOL fRet = FALSE;
	if (lCol >= 0 && lRow == SS_ALLROWS) fRet = TRUE;

	return fRet;
}

// jb - 15.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 (SS_ALLCOLS) for the first column coordinate.  If
// so, then user wants to do ALL of the column cells.
//
BOOL allcolumns (SS_COORD lCol, SS_COORD lRow)
{
	BOOL fRet = FALSE;

	// -1, 3
	//
	if (lCol == SS_ALLCOLS && lRow >= 0) fRet = TRUE;
	return fRet;
}

// jb - 15.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 (SS_ALLROWS) for the first row coordinate.  If so,
// then user wants to do a ALL of the rows.
//
BOOL allrows (SS_COORD lCol, SS_COORD lRow)
{	
	BOOL fRet = FALSE;
	if (lCol >= 0 && lRow == SS_ALLROWS) fRet = TRUE;

	return fRet;
}

// jb - 27.jul.98 ----------------------------------------------------------------------------
//
// Need to normalize the Row column for start, stop positions with spread.  Validate
// the coordinate ranges.
//
BOOL columnrowcoordinates (LPSPREADSHEET lpSS, PWEB_COORD pWebCOORD, SS_XML *pXML)
{
	BOOL bRet = TRUE;
	SS_COORD colcnt, rowcnt;
	SS_COORD c, r;
	TCHAR tcFormat[BUFSIZ*10];

	pXML->tcBuf[0] = 0;
	pXML->bColRange = FALSE;
	pXML->bRowRange = FALSE;

	// Get the last maximum col/row number with data.
	//
	SS_GetDataCnt (lpSS, &colcnt, &rowcnt);
// fix for 14272 -scl
//  colcnt += (lpSS->Row.HeaderCnt - 1);
//  rowcnt += (lpSS->Col.HeaderCnt - 1);
  colcnt += (lpSS->Col.HeaderCnt - 1);
  rowcnt += (lpSS->Row.HeaderCnt - 1);
   rowcnt--;
   colcnt--;
	getothercnt (lpSS, &colcnt, &rowcnt);
	pXML->rowcnt = rowcnt;
	pXML->colcnt = colcnt;

	// Are we doing the entire spreadsheet ?  This happens when ExportToHTML is called.
	// -1, -1  ---> 0, 0, maxcol, maxrow
	//
	if (entirespreadsheet(pWebCOORD->lColStart, pWebCOORD->lRowStart))
	{	pWebCOORD->lColStart = 0;
		pWebCOORD->lRowStart = 0;
		pWebCOORD->lColEnd = colcnt;
		pWebCOORD->lRowEnd = rowcnt;
	}

	// We are doing ALL columns: -1, 3	---> 0, 3, maxcol, row
	//
	if (allcolumns (pWebCOORD->lColStart, pWebCOORD->lRowStart))
	{	pWebCOORD->lColStart = 0;
		pWebCOORD->lColEnd = colcnt;
	}

	// We are doing ALL rows: 3, -1	--->3, 0, col, maxrow
	//
	else if (allrows (pWebCOORD->lColStart, pWebCOORD->lRowStart))
	{	pWebCOORD->lRowStart = 0;
		pWebCOORD->lRowEnd = rowcnt;
	}

	// We are doing a column range, --->0, 0, maxcol, row
	//
	if (columnrange (pWebCOORD->lColEnd, pWebCOORD->lRowEnd))
	{	pWebCOORD->lColEnd = colcnt;
	}

	// We are doing a column range,  --->0, 0, col, maxrow
	//
	if (rowrange (pWebCOORD->lColEnd, pWebCOORD->lRowEnd))
	{	pWebCOORD->lRowEnd = rowcnt;
	}

	// Make sure the coordinates make sense.  Not < -1, first coord greater than second etc.
	//
	if (!validatecoordinates (pWebCOORD->lColStart, pWebCOORD->lRowStart, pWebCOORD->lColEnd, pWebCOORD->lRowEnd, pXML))
	{	return FALSE;
	}

	// Make sure we don't try to export over what the maximum columns/rows are.
	//
	c = SS_GetMaxCols(lpSS);
	r = SS_GetMaxRows(lpSS);
#ifdef SS_V40
  r += lpSS->Row.HeaderCnt - 1;
  c += lpSS->Col.HeaderCnt - 1;
#endif
	if (lpSS->fVirtualMode)
	{	// In virtual mode, rows gets maxed to 1,000,000 which makes the export
		// seem like it is hung when all it is really doing is looping 1/2 a billion times...
		//
//		virtualmode (lpSS, &pWebCOORD->lRowStart, &pWebCOORD->lRowEnd);
//		if (r > 500)
//		{	r = 500;
//		}
	  if( pWebCOORD->lRowStart < lpSS->Virtual.VTop )
	  {
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_PRECEEDEDVTOPROW, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pWebCOORD->lRowStart, lpSS->Virtual.VTop, pWebCOORD->lRowStart, lpSS->Virtual.VTop);
		pWebCOORD->lRowStart = lpSS->Virtual.VTop;
	  }
	  if( pWebCOORD->lRowEnd > lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize )
	  {
		r = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize;
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXCEEDEDVBOTTOMROW, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pWebCOORD->lRowEnd, r, pWebCOORD->lRowEnd, r);
	  }
	}

	// If the second column passed in is greater than the maxcol, make it EQ to 
	// maxcol and write a message in the log file.
	//
	if (c < pWebCOORD->lColEnd)
	{	pWebCOORD->lColEnd = c;
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXCEEDEDMAXCOL, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pWebCOORD->lColStart, pWebCOORD->lRowStart, pWebCOORD->lColEnd, c, pWebCOORD->lColEnd, c);
	}

	// If the second row passed in is greater than the maxrow, make it EQ to 
	// maxrow and write a message in the log file.
	//
	if (r < pWebCOORD->lRowEnd)
	{	pWebCOORD->lRowEnd = r;
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXCEEDEDMAXROW, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pWebCOORD->lColStart, pWebCOORD->lRowStart, pWebCOORD->lRowEnd, r, pWebCOORD->lRowEnd, r);
	}

	if (pWebCOORD->lColEnd > colcnt) pWebCOORD->lColEnd = colcnt;
	if (pWebCOORD->lRowEnd > rowcnt) pWebCOORD->lRowEnd = rowcnt;

	if (pWebCOORD->lColStart >= lpSS->Col.HeaderCnt)
	{	pXML->bColRange = TRUE;
	}

	if (pWebCOORD->lRowStart >= lpSS->Row.HeaderCnt)
	{	pXML->bRowRange = TRUE;
	}

	if (pXML->tcBuf[0] != 0)
	{ 	writemsg2log (pXML);
	}

	return bRet;
}

// jb - 02.nov.98 ---------------------------------------------------------
//
// This is not a column or row header.  It is a cell and it is either empty or
// we could not get any data from it because of the type of cell.  Write this
// information to the log file.
//
static BYTE checkbox0 (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
	SS_CELLTYPE CellType;
	BYTE bType;

	SS_RetrieveCellType(lpSS, &CellType, NULL, lCol, lRow);
	bType = CellType.Type;

	if (bType == SS_TYPE_CHECKBOX)
	{	return bType;
	}

	return 0;
}

// jb - 18.sep.00 -------------------------------------------------------------
//					  
// This function will actually save each sheet that is specified, not the entire book with all the sheets.
// In the future, we may make this save all sheets to one file or to seperate files.
//
static int SaveXML (LPSPREADSHEET lpSS, SS_XML *pXML, PWEB_COORD pWebCOORD, long lFlags)
{
	BOOL nRet = 0;
	SS_COORD lCol, lRow;
	TCHAR pszBuf[BUFSIZ*10];
	size_t uSize = 0;
	int startcol, startrow, endcol, endrow;
	int idx = 0;
	TCHAR tcFormat[BUFSIZ*10];

  
	// Normalize the column/row coordinates.  ex.  SS_ALLCOLS would be col = 0, 
	// col2 = maxdatacols.
	//
	columnrowcoordinates (lpSS, pWebCOORD, pXML);

	// These are the tags like <fname>, <lname> etc.  Store them dynamically in the XML
	// structure.
	//
	if (!GetDataTags (lpSS, pWebCOORD, pXML))
// fix for bug 9903 -scl
//	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_NOXMLTAGS, pszBuf, BUFSIZ*10-1);
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_NOXMLTAGS, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat);
		writemsg2log (pXML);
		return 0;
	}

	// Start off with our root node.  ex <AddressBook>
	//
	_tcscpy (pXML->tcBuf, pXML->root);
  	LoadBuffer (pXML);

	startcol = lpSS->Col.HeaderCnt;
	startrow = lpSS->Row.HeaderCnt;
	endrow = pWebCOORD->lRowEnd;
	endcol = pWebCOORD->lColEnd;

	if (pWebCOORD->lColStart >= lpSS->Col.HeaderCnt)
	{	startcol = pWebCOORD->lColStart;
		endcol++;
	}

	if (pWebCOORD->lRowStart >= lpSS->Row.HeaderCnt)
	{	startrow = pWebCOORD->lRowStart;
	}

	// Start going through spread, getting rows and columns of cell data.
	//
	for (lRow = startrow; lRow <= endrow; lRow++)
	{  	idx = 0;
		// Now, write our collection node. ex. <Address>
		//
		_tcscpy (pXML->tcBuf, pXML->collection);
  		LoadBuffer (pXML);

		// Get our data
		//
		for (lCol = startcol; lCol <= endcol; lCol++)
		{	if (lFlags & SS_EXPORTXML_UNFORMATTED) 
			{ 	SS_GetValueEx (lpSS, lCol, lRow, pszBuf, BUFSIZ*10);
			}
			else
			{	SS_GetDataEx (lpSS, lCol, lRow, pszBuf, BUFSIZ*10);
			}

			if (lRow >= lpSS->Row.HeaderCnt && lCol >= lpSS->Col.HeaderCnt)
			{	if (checkbox0 (lpSS, lCol, lRow))
				{	if (*pszBuf == 0)
					{	_tcscpy (pszBuf, (LPTSTR)_T("0"));
					}
				}
			}

			dontshowpassword (lpSS, lCol, lRow, pszBuf);
// fix for bug 9753 -scl
      look4specialcharacters(pszBuf, BUFSIZ*10);
			SaveDataTag (pXML, pWebCOORD, pszBuf, idx);
			idx++;
		}

		// Ending of the collection tag. ex. </Address>
		//
		EndTag (pXML->tcBuf, pXML->collection, 1);
		LoadBuffer (pXML);
	}

	// Ending of only root tag.  ex. </AddressBook>
	//
	EndTag (pXML->tcBuf, pXML->root, 0);
	LoadBuffer (pXML);

	return 1;
}

// jb - 6.sep.00 --------------------------------------------------------------
//
// Initialize our coordinates.
//
void InitCoords (PWEB_COORD pWebCOORD, SS_COORD lCol, SS_COORD lCol2, SS_COORD lRow, SS_COORD lRow2)
{
	pWebCOORD->lColStart	= lCol;
	pWebCOORD->lColEnd		= lCol2;
	pWebCOORD->lRowStart	= lRow;
	pWebCOORD->lRowEnd		= lRow2;
	pWebCOORD->bFlag		= FALSE;

	return;
}

// jb - 17.may.01 -------------------------------------------------------------
//
// Write ending information to the log file.
//
static void write2log (SS_XML *pXML, WEB_COORD *pWEB_COORD, int nRet)
{
	TCHAR tcFormat[BUFSIZ*10];

	if (pXML->bColRange)
	{	pWEB_COORD->lColStart--;
	}

	if (pXML->bRowRange)
	{	pWEB_COORD->lRowStart--;
	}

	if (nRet == 2)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_CANTOPENFILE, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pXML->pXMLName);
		writemsg2log (pXML);
	}

	if (nRet == 1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_SUCCESSEXPORT_XML, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pWEB_COORD->lColStart+1, pWEB_COORD->lColEnd, pWEB_COORD->lRowStart+1, pWEB_COORD->lRowEnd, pXML->pXMLName);
		writemsg2log (pXML);

		LoadString((HINSTANCE)fpInstance, IDS_LOGS_FOOTER_XML, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat);
		writemsg2log (pXML);
		return;
	}
	else if (nRet == 0)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FAILEDEXPORT, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pXML->pXMLName);
		writemsg2log (pXML);

		LoadString((HINSTANCE)fpInstance, IDS_LOGS_FOOTER_XML, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat);
		writemsg2log (pXML);
		return;
	}

	// XML buffer strings
	//
	if (nRet == 3)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FAILEDBUFF, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat);
	}

	if (nRet == 4)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_CANTALLOCBUFFER, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat);
	}

	else
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_SUCCESSBUFF, tcFormat, BUFSIZ*10-1);
		_stprintf (pXML->tcBuf, tcFormat, pWEB_COORD->lColStart+1, pWEB_COORD->lColEnd, pWEB_COORD->lRowStart+1, pWEB_COORD->lRowEnd);
	}
	writemsg2log (pXML);

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FOOTER_XMLBUFF, tcFormat, BUFSIZ*10-1);
	_stprintf (pXML->tcBuf, tcFormat);
	writemsg2log (pXML);

	return;
}

// jb - 2.may.01 --------------------------------------------------------------
//
// Export spread data to XML, store in a file
//
BOOL SS_ExportRangeToXML (LPSPREADSHEET lpSS, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR pszFile, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR pcszLogFile)
{
	int nRet = 0;
	SS_XML *pXML;
	WEB_COORD webcoord;
	
	// No need to proceed if our filename is invalid.
	//
	if (!*pszFile)
	{	return nRet;
	}

	// Initialize xml and coordinate structures.
	//
  	InitCoords (&webcoord, lColStart, lColEnd, lRowStart, lRowEnd);
	pXML = calloc (1, (sizeof(SS_XML) * sizeof(TCHAR)));
	InitXML (pXML, pszRoot, pszCollection, lFlags, pcszLogFile);

	// Validate our XML name.
	//
	if (!InitXMLName (pXML, pszFile))
	{	return nRet;
	}

	HeaderLog_XML (pXML);

	// Save our XML data.
	if (pXML->fp = _tfopen (pXML->pXMLName, _T("w+")))
	{  	
//GAB 1/15/2004 Add these bytes to identify file as UTF-8
    fputc(0xEF,pXML->fp);
    fputc(0xBB,pXML->fp);
    fputc(0xBF,pXML->fp);
    nRet = SaveXML (lpSS, pXML, &webcoord, lFlags);
	}
	else
	{	nRet = 2;
	}

// fix for bug 9753 -scl
  if( nRet )
	  write_XML (pXML);
  else
  {
    fclose (pXML->fp);
    pXML->fp = NULL;
  }
	write2log (pXML, &webcoord, nRet);
	return CleanUp (pXML);
}

// jb - 2.may.01 --------------------------------------------------------------
//
// Export a range of spread cells to XML, store in a file
//
BOOL DLLENTRY SSExportRangeToXML (HWND hWnd, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR lpszFileName, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR pszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet;

	lpSS = SS_SheetLock (hWnd);
	SS_AdjustCellRangeCoords(lpSS, &lColStart, &lRowStart, &lColEnd, &lRowEnd);
	bRet = SS_ExportRangeToXML (lpSS, lColStart, lRowStart, lColEnd, lRowEnd, lpszFileName, pszRoot, pszCollection, lFlags, pszLogFile);
	SS_SheetUnlock (hWnd);

	return bRet;
}

// jb - 2.may.01 --------------------------------------------------------------
//
// Export spread data to XML, store in a file
//
BOOL DLLENTRY SSExportToXML (HWND hWnd, LPCTSTR pszFileName, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR pszLogFile)
{	
	return SSExportRangeToXML (hWnd, SS_ALLCOLS, SS_ALLROWS, 0, 0, pszFileName, pszRoot, pszCollection, lFlags, pszLogFile);
}

// jb - 2.may.01 --------------------------------------------------------------
//
// Export spread data to XML, store in a buffer
//
HGLOBAL SS_ExportRangeToXMLBuffer (LPSPREADSHEET lpSS, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR pcszLogFile)
{
	int nRet;
	SS_XML *pXML;
	WEB_COORD webcoord;
	int nSize;
	LPTSTR pszBuff = NULL;
	LPTSTR pszRetBuff = NULL;
	HGLOBAL hXMLBuff;
	LPWSTR wstr = NULL;
	LPSTR str = NULL;
	long len = 0;

  	InitCoords (&webcoord, lColStart, lColEnd, lRowStart, lRowEnd);
	pXML = calloc (1, (sizeof(SS_XML) * sizeof(TCHAR)));
	InitXML (pXML, pszRoot, pszCollection, lFlags, pcszLogFile);
	HeaderLog_XMLBUFF (pXML);

	nRet = SaveXML (lpSS, pXML, &webcoord, lFlags);
	if (!nRet)
	{	write2log (pXML, &webcoord, 3);
		hXMLBuff = NULL;
	}
	else
	{	pszBuff = GlobalLock (pXML->ghBuf);
//GAB 10/18/01 Modified for Unicode
//		nSize = (_tcslen (pszBuff) + 1) * sizeof (TCHAR);
//	 	hXMLBuff = GlobalAlloc (GHND, nSize);
	 	nSize = (int)_tcslen (pszBuff) + 1;
	 	hXMLBuff = GlobalAlloc (GHND, nSize * 2 * sizeof (TCHAR));
		if (hXMLBuff)
		{

		  pszRetBuff = GlobalLock (hXMLBuff);

//GAB Modified for Unicode
#if defined(UNICODE) || defined(_UNICODE)
// fix for 14217 -scl
//	len = WideCharToMultiByte(CP_UTF8, 0, pszBuff, -1, NULL, 0, NULL, NULL);
//	str = (LPSTR)malloc(len * sizeof(char));
//	if( str )
//	{
//	  memset(str, 0, len * sizeof(char));
//	  WideCharToMultiByte(CP_UTF8, 0, pszBuff, -1, str, len, NULL, NULL);
//  	  fputs(str, pXML->fp);
	  _tcscpy (pszRetBuff, pszBuff);
	  free(str);
//	}
#else
// fix for bug 9293 -scl
		  len = MultiByteToWideChar(CP_ACP, 0, pszBuff, -1, NULL, 0) + 1;
		  wstr = (LPWSTR)malloc(len * sizeof(WCHAR));
	      if( wstr )
	      {
			  memset(wstr, 0, len * sizeof(WCHAR));
			  MultiByteToWideChar(CP_ACP, 0, pszBuff, -1, wstr, len);
			  len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL) + 1;
			  str = malloc(len * 2 * sizeof(char));
			  if( str )
			  {
				  memset(str, 0, len * 2 * sizeof(char));
				  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
				  _tcscpy (pszRetBuff, (LPTSTR)str);
				  free(str);
			  }
			  free(wstr);
		  }
#endif
		  GlobalUnlock (hXMLBuff);
		  nRet = 5;	
		}
		else
		{	hXMLBuff = NULL;
			nRet = 4;
		}
		GlobalUnlock (pXML->ghBuf);
		write2log (pXML, &webcoord, nRet);
	}

	CleanUp (pXML);
	return hXMLBuff;
}

// jb - 2.may.01 --------------------------------------------------------------
//
// Export a range of spread cells to XML, store in a file
//
BOOL DLLENTRY SSExportRangeToXMLBuffer (HWND hWnd, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR pszRoot, LPCTSTR pszCollection, HGLOBAL *hXMLBuff, long lFlags, LPCTSTR pszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet = TRUE;

	lpSS = SS_SheetLock (hWnd);
	SS_AdjustCellRangeCoords(lpSS, &lColStart, &lRowStart, &lColEnd, &lRowEnd);
	hXMLBuff = SS_ExportRangeToXMLBuffer (lpSS, lColStart, lRowStart, lColEnd, lRowEnd, pszRoot, pszCollection, lFlags, pszLogFile);
	SS_SheetUnlock (hWnd);

	if (*hXMLBuff == NULL)
	{	bRet = FALSE;
	}

	return bRet;
}

// jb - 2.may.01 --------------------------------------------------------------
//
// Export spread data to XML, store in a file
//
BOOL DLLENTRY SSExportToXMLBuffer (HWND hWnd, LPCTSTR pszRoot, LPCTSTR pszCollection, HGLOBAL *hXMLBuff, long lFlags, LPCTSTR pszLogFile)
{	
	BOOL bRet = TRUE;

	bRet = SSExportRangeToXMLBuffer (hWnd, SS_ALLCOLS, SS_ALLROWS, 0, 0, pszRoot, pszCollection, hXMLBuff, lFlags, pszLogFile);

	return bRet;
}
#endif

