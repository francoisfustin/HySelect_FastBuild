/****************************************************************************/
/* FPTSTR.CPP   FarPoint Support for FPTSTR's                               */
/****************************************************************************/

#include <string.h>
#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
//#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#endif

#include "fpconvrt.h"
#include "fptstr.h"                             

#ifdef __cplusplus
extern "C" {
#endif

FPLIB_(FPTSTR) fpSysAllocTString(const TCHAR FAR *fptstr)
{
// If WIN16 or UNICODE, simply use matching BSTR function
#if (!defined(WIN32) || defined(_UNICODE))
  return SysAllocString(fptstr);
//else WIN32 and not Unicode, use BYTE version of BSTR function
#else
  return (FPTSTR)SysAllocStringByteLen(fptstr, _ftcslen(fptstr));
#endif  
}

FPLIB_(int) fpSysReAllocTString(FPTSTR FAR *lp_fptstr, 
  const TCHAR FAR *fptstr)
{
// If WIN16 or UNICODE, simply use matching BSTR function
#if (!defined(WIN32) || defined(_UNICODE))
  return SysReAllocString(lp_fptstr, fptstr);
//else WIN32 and not Unicode, use BYTE version of BSTR function
#else
  return fpSysReAllocTStringLen(lp_fptstr, fptstr, _ftcslen(fptstr));
#endif  
}

FPLIB_(FPTSTR) fpSysAllocTStringLen(const TCHAR FAR *fptstr,
  unsigned int len)
{
// If WIN16 or UNICODE, simply use matching BSTR function
#if (!defined(WIN32) || defined(_UNICODE))
  return SysAllocStringLen(fptstr, len);
//else WIN32 and not Unicode, use BYTE version of BSTR function
#else
  return (FPTSTR)SysAllocStringByteLen(fptstr, len);
#endif  
}

FPLIB_(int) fpSysReAllocTStringLen(FPTSTR FAR *lp_fptstr, 
  const TCHAR FAR *fptstr, unsigned int len)
{
// If WIN16 or UNICODE, simply use matching BSTR function
#if (!defined(WIN32) || defined(_UNICODE))
   return SysReAllocStringLen(lp_fptstr, fptstr, len);
//else WIN32 and not Unicode, use BYTE version of BSTR function
#else
   // allocate new string
   FPTSTR fstrTmp = (FPTSTR)SysAllocStringByteLen(fptstr, len);
   // if allocation failed, return FALSE.
   if (!fstrTmp)
     return FALSE;
   // free current BSTR, set ptr to new BSTR and return TRUE.
   fpSysFreeTString(*lp_fptstr);
   *lp_fptstr = fstrTmp;
   return TRUE;
#endif  
}

FPLIB_(void) fpSysFreeTString(FPTSTR fptstr)
{
  SysFreeString((BSTR)fptstr);  // no difference between BSTR & TCHAR
}

FPLIB_(unsigned int) fpSysTStringLen(FPTSTR fptstr)
{
// If WIN16 or UNICODE, simply use matching BSTR function
#if (!defined(WIN32) || defined(_UNICODE))
  return SysStringLen(fptstr);
//else WIN32 and not Unicode, use BYTE version of BSTR function
#else
  return SysStringByteLen((BSTR)fptstr);
#endif  
}

FPLIB_(unsigned int) fpSysTStringByteLen(FPTSTR fptstr)
{
#ifdef WIN32
  return SysStringByteLen((BSTR)fptstr);
#else
  return SysStringLen(fptstr);
#endif  
}

FPLIB_(FPTSTR) fpSysAllocTStringByteLen(const char FAR* psz, unsigned int len)
{
#ifdef WIN32
  return (FPTSTR)SysAllocStringByteLen(psz, len);
#else
  return SysAllocStringLen(psz, len);
#endif  
}

// Creates new TSTR from BSTR, may convert WCHAR to CHAR if TCHAR != OLECHAR.
// NOTE: Doesn't free BSTR.
FPLIB_(FPTSTR) fpSysAllocTStringFromBSTR(LPVOID bstr)
{
// If WIN16 or UNICODE, simply use matching BSTR function
#if (!defined(WIN32) || defined(_UNICODE) || defined(OLE2ANSI))
  return SysAllocString((BSTR)bstr);

//if TCHAR != OLECHAR, use conversion func.
#else
  return (FPTSTR)fpOLE2Str((LPSTR)bstr); // NOTE: func uses SysAllocStringByteLen()
#endif  
}

// Creates new BSTR from TSTR, may convert CHAR to WCHAR if TCHAR != OLECHAR.
// NOTE: Doesn't free TSTR.
FPLIB_(LPVOID) fpSysAllocBSTRFromTString(FPTSTR fptstr)
{
// If TCHAR == OLECHAR, simply use matching BSTR function
#if (!defined(WIN32) || defined(_UNICODE) || defined(OLE2ANSI))
  return (LPVOID)SysAllocString((BSTR)fptstr);

//else TCHAR != OLECHAR, use conversion func.
#else
  return (LPVOID)fpStr2OLE(fptstr); // NOTE: func uses SysAllocStringByteLen()
#endif  
}

#ifdef __cplusplus
}
#endif
         
#endif //FP_OCX