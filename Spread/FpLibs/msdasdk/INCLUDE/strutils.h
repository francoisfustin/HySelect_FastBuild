//=--------------------------------------------------------------------------=
// STRUTILS.H:	Definitions for string helpers and conversions
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _STRUTILS_H_
#define _STRUTILS_H_

// This macros generates a temporary ANSI multibyte string
// from a WIDE string
//
#define MAKE_MULTIBYTESTRING(name,arg) \
	MULTIBYTESTRING _Temp_Obj_##name(arg); \
	const char* name = _Temp_Obj_##name

// String APIs
//
int lstrncmp(const char *sz1, const char *sz2, int cChars);
int lstrncmpi(const char *sz1, const char *sz2, int cChars);

int bstrlen(const BSTR bstr);
int bstrcmp(const BSTR bstr1, const BSTR bstr2);
int bstrncmp(const BSTR bstr1, const BSTR bstr2, int cChars);
int bstrcmpi(const BSTR bstr1, const BSTR bstr2);
int bstrncmpi(const BSTR bstr1, const BSTR bstr2, int cChars);

BSTR bstrcpy(BSTR& bstrDst, const BSTR bstrSrc);
BSTR bstrncpy(BSTR& bstrDst, const BSTR bstrSrc, int cChars);

int wstrlen(const WCHAR *wsz);
int wstrcmp(const WCHAR *wsz1, const WCHAR *wsz2);
int wstrncmp(const WCHAR *wsz1, const WCHAR *wsz2, int cChars);
int wstrcmpi(const WCHAR *wsz1, const WCHAR *wsz2);
int wstrncmpi(const WCHAR *wsz1, const WCHAR *wsz2, int cChars);

WCHAR *wstrcpy(WCHAR *wszDst, const WCHAR *wszSrc);
WCHAR *wstrncpy(WCHAR *wszDst, const WCHAR *wszSrc, int cChars);

char *stralloc(const char *sz);
char *stralloc(const BSTR bstr);
char *stralloc(const WCHAR *wsz);
WCHAR *wstralloc(const WCHAR *wsz);
WCHAR *wstralloc(const BSTR bstr);
WCHAR *wstralloc(const WCHAR *wsz);
BSTR bstralloc(const BSTR bstr);
BSTR bstralloc(const char *sz);
BSTR bstralloc(const WCHAR *wsz);

int LoadWideString(HINSTANCE hInstance, UINT uID, WCHAR *pwcBuf, int cbBufMax);

/////////////////////////////////////////////////////////////////////////////
// MULTIBYTESTRING
//
struct MULTIBYTESTRING
{
	MULTIBYTESTRING(const BSTR bstr);
	MULTIBYTESTRING(const WCHAR *wsz);
	~MULTIBYTESTRING();

	char *sz;

	operator const char*() {return (const char*)sz;}
};

#endif // !defined(_STRUTILS_H_)
