/************************************************************************
 * FPTSTR.H  FarPoint's SysAlloc functions for TSTR's (LPTSTR)
 *
 *           Unlike BSTR's, TSTR's are dependent on UNICODE setting 
 *           (not these other variables: WIN32, MSVC20, MSVC40.)
 *
 ************************************************************************/
#ifndef FPTSTR_H
#define FPTSTR_H

//#ifdef NO_MFC
//#ifdef __T
//#undef __T
//#endif
//#endif    // NO_MFC

#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

// Use this typedef (FPTSTR) because "LPTSTR" is not defined in all environments. 
#ifndef _FPTSTR_DEFINED
typedef TCHAR FAR *FPTSTR;
typedef const TCHAR FAR *FPCTSTR;
#define _FPTSTR_DEFINED
#endif

//NOTE: "FPLIB_()" is defined in fpconvrt.h.
FPLIB_(FPTSTR)       fpSysAllocTString(const TCHAR FAR*);
FPLIB_(int)          fpSysReAllocTString(FPTSTR FAR*, const TCHAR FAR*);
FPLIB_(FPTSTR)       fpSysAllocTStringLen(const TCHAR FAR*, unsigned int);
FPLIB_(int)          fpSysReAllocTStringLen(FPTSTR FAR*, const TCHAR FAR*, unsigned int);
FPLIB_(void)         fpSysFreeTString(FPTSTR);
FPLIB_(unsigned int) fpSysTStringLen(FPTSTR);

FPLIB_(unsigned int) fpSysTStringByteLen(FPTSTR fptstr);
FPLIB_(FPTSTR)       fpSysAllocTStringByteLen(const char FAR* psz, unsigned int len);

// Creates new TSTR from BSTR, may convert WCHAR to CHAR (doesn't free BSTR).
FPLIB_(FPTSTR)       fpSysAllocTStringFromBSTR(LPVOID bstr);

// Creates new BSTR from TSTR, may convert CHAR to WCHAR (doesn't free TSTR).
FPLIB_(LPVOID)       fpSysAllocBSTRFromTString(FPTSTR fptstr);

#ifdef __cplusplus
}
#endif

#if (!defined(WIN32) && defined(FP_VB)) //WIN16 & VBX
typedef LPSTR PTSTR, LPTSTR;
typedef LPCSTR LPCTSTR;
#endif

#endif // FPTSTR.H
 
