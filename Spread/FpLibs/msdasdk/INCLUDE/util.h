//=--------------------------------------------------------------------------=
// Util.H
//=--------------------------------------------------------------------------=
// Copyright  1995  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
// contains utilities that we will find useful.
//
#ifndef _UTIL_H_

#include "Globals.H"



//=--------------------------------------------------------------------------=
// Misc Helper Stuff
//=--------------------------------------------------------------------------=
//

HWND      GetParkingWindow(void);
HINSTANCE GetResourceHandle(LCID lcid = 0);	// Optional LCID param
						// If not used or zero, we use
						// the g_


// an array of common OLE automation data types and their sizes [in bytes]
//
extern const BYTE g_rgcbDataTypeSize [];


//=--------------------------------------------------------------------------=
// miscellaneous [useful] numerical constants
//=--------------------------------------------------------------------------=
// the length of a guid once printed out with -'s, leading and trailing bracket,
// plus 1 for NULL
//
#define GUID_STR_LEN    40


//=--------------------------------------------------------------------------=
// allocates a temporary buffer that will disappear when it goes out of scope
// NOTE: be careful of that -- make sure you use the string in the same or
// nested scope in which you created this buffer. people should not use this
// class directly.  use the macro(s) below.
//
class TempBuffer {
  public:
    TempBuffer(ULONG cBytes) {
        m_pBuf = (cBytes <= 120) ? &m_szTmpBuf : HeapAlloc(g_hHeap, 0, cBytes);
        m_fHeapAlloc = (cBytes > 120);
    }
    ~TempBuffer() {
        if (m_pBuf && m_fHeapAlloc) HeapFree(g_hHeap, 0, m_pBuf);
    }
    void *GetBuffer() {
        return m_pBuf;
    }

  private:
    void *m_pBuf;
    // we'll use this temp buffer for small cases.
    //
    char  m_szTmpBuf[120];
    unsigned m_fHeapAlloc:1;
};

//=--------------------------------------------------------------------------=
// string helpers.
//
// given and ANSI String, copy it into a wide buffer.
// be careful about scoping when using this macro!
//
// how to use the below two macros:
//
//  ...
//  LPSTR pszA;
//  pszA = MyGetAnsiStringRoutine();
//  MAKE_WIDEPTR_FROMANSI(pwsz, pszA);
//  MyUseWideStringRoutine(pwsz);
//  ...
//
// similarily for MAKE_ANSIPTR_FROMWIDE.  note that the first param does not
// have to be declared, and no clean up must be done.
//
#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname = (lstrlen(ansistr) + 1) * sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, ansistr, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()

// * 2 for DBCS handling in below length computation
//
#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (lstrlenW(widestr) + 1) * 2 * sizeof(char); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname.GetBuffer()

#define STR_BSTR   0
#define STR_OLESTR 1
#define BSTRFROMANSI(x)    (BSTR)MakeWideStrFromAnsi((LPSTR)(x), STR_BSTR)
#define OLESTRFROMANSI(x)  (LPOLESTR)MakeWideStrFromAnsi((LPSTR)(x), STR_OLESTR)
#define BSTRFROMRESID(x)   (BSTR)MakeWideStrFromResourceId(x, STR_BSTR)
#define OLESTRFROMRESID(x) (LPOLESTR)MakeWideStrFromResourceId(x, STR_OLESTR)
#define COPYOLESTR(x)      (LPOLESTR)MakeWideStrFromWide(x, STR_OLESTR)
#define COPYBSTR(x)        (BSTR)MakeWideStrFromWide(x, STR_BSTR) // Embedded NULLs not supported

inline BSTR DuplicateBSTR(BSTR bstr) { 
    return SysAllocStringLen(bstr, SysStringLen(bstr)); }

LPWSTR MakeWideStrFromAnsi(LPSTR, BYTE bType);
LPWSTR MakeWideStrFromResourceId(WORD, BYTE bType);
LPWSTR MakeWideStrFromWide(LPWSTR, BYTE bType);


// takes a GUID, and a pointer to a buffer, and places the string form of the
// GUID in said buffer.
//
int StringFromGuidA(REFIID, LPSTR);


//=--------------------------------------------------------------------------=
// registry helpers.
//
// takes some information about an Automation Object, and places all the
// relevant information about it in the registry.
//
BOOL RegSetMultipleValues(HKEY hkey, ...);
BOOL RegisterUnknownObject(LPCSTR pszObjectName, LPCSTR pszLabelName, REFCLSID riidObject, BOOL fAptThreadSafe);
BOOL RegisterAutomationObject(LPCSTR pszLibName, LPCSTR pszObjectName, LPCSTR pszLabelName, long lObjVer, long lMajorVersion, long lMinorVersion, REFCLSID riidLibrary, REFCLSID riidObject, BOOL fAptThreadSafe);
BOOL RegisterControlObject(LPCSTR pszLibName, LPCSTR pszObjectName, LPCSTR pszLabelName, long lObjMajVer, long lObjMinVer, long lMajorVersion, long lMinorVersion, REFCLSID riidLibrary, REFCLSID riidObject, DWORD dwMiscStatus, WORD wToolboxBitmapId, BOOL fAptThreadSafe);
BOOL UnregisterUnknownObject(REFCLSID riidObject, BOOL *pfAllRemoved);
BOOL UnregisterAutomationObject(LPCSTR pszLibName, LPCSTR pszObjectName, long lVersion, REFCLSID riidObject);
#define UnregisterControlObject UnregisterAutomationObject

BOOL UnregisterTypeLibrary(REFCLSID riidLibrary);

// Register/UnregisterUnknownObject helpers to help prevent us from blowing away specific keys
//
BOOL ExistInprocServer(HKEY hkCLSID, char *pszCLSID);
BOOL ExistImplementedCategories(REFCLSID riid);

// deletes a key in the registr and all of it's subkeys
//
BOOL DeleteKeyAndSubKeys(HKEY hk, LPCSTR pszSubKey);

// Path of Windows\Help directory.
//
UINT GetHelpFilePath(char *pszPath, UINT cbPath);

// Helper function for registration
//
void _MakePath(LPSTR pszFull, const char * pszName, LPSTR pszOut);

//=--------------------------------------------------------------------------=
// conversion helpers.
//
void        HiMetricToPixel(const SIZEL *pSizeInHiMetric, SIZEL *pSizeinPixels);
void        PixelToHiMetric(const SIZEL *pSizeInPixels, SIZEL *pSizeInHiMetric);


//=--------------------------------------------------------------------------=
// This is a version macro so that versioning can be done in text and binary
// streams.
//
#define VERSION(x,y) MAKELONG(y,x)

#define _UTIL_H_
#endif // _UTIL_H_
