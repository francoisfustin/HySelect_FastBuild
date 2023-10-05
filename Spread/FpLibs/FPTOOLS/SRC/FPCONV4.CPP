/***************************************************************************/
/* FPCONV.CPP   FarPoint Support for VBX functions in non-VB environments */
/***************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#include <string.h>

#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#endif

#ifdef WIN32
#include <afxpriv.h>
#include "tchar.h"
#else

#define WM_NOTIFY                       0x004E

#define USES_CONVERSION int _convert; _convert

#define T2OLE  
#define OLE2T

#define NON_STANDARD_INDEX  12

#endif

#include "vbinterf.h"
// following lines taken from VBINTERF.H
#include <initguid.h>

#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FP_OCX

LPTSTR FPLIB fpStr2OLE(LPTSTR lpszInput)                          
{
return (fpStr2OLEByteLen(lpszInput, lpszInput ? lstrlen(lpszInput) : 0));
}            


LPTSTR FPLIB fpStr2OLEByteLen(LPTSTR lpszInput, int iCharCt)
{
#ifdef FP_OCX
//  USES_CONVERSION;
//  return T2OLE(lpString);
#if (defined(WIN32) && !defined(_UNICODE))
/*
  int iCharCt = lstrlen((LPTSTR)lpszInput);
  BSTR lpStrOut = SysAllocStringByteLen(NULL, (iCharCt * 2) + 1);
  for (register int i = 0; i <= iCharCt; i++)
    ((WORD *)lpStrOut)[i] = (WORD)lpszInput[i];

  // Subtract first byte of null-terminator from len so len excludes
  // both null chars
  *(int *)(((LPSTR)lpStrOut)-sizeof(int)) -= 1;
*/
  LPWSTR lpStrOut;
  int rc;

  if (lpszInput == NULL)
	 return NULL;

  lpStrOut = (LPWSTR)SysAllocStringByteLen(NULL, (iCharCt * 2) + 1);
  ASSERT(lpStrOut != NULL);
  if (!lpStrOut)
    return NULL;

  lpStrOut[0] = (WORD)0;
  rc = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpszInput, -1, lpStrOut, iCharCt+1);
  // realloc to new size if not same (note: rc = num copied including null)
  if (rc && (rc != iCharCt))
  {
    LPWSTR wstrTmp;
    --rc;           // exclude null in re-calc
    wstrTmp = (LPWSTR)SysAllocStringByteLen((LPCSTR)lpStrOut, (rc * 2) + 1);
    wstrTmp[rc] = (WORD)0;
    SysFreeString((BSTR)lpStrOut);
    lpStrOut = wstrTmp;
  }

  // Subtract first byte of null-terminator from len so len excludes
  // both null chars
  *(int *)(((LPSTR)lpStrOut)-sizeof(int)) -= 1;
#else
  BSTR lpStrOut = SysAllocString((OLECHAR FAR *)lpszInput);
#endif
  return (LPTSTR)lpStrOut;
#else
  return lpString;
#endif
}            


LPTSTR FPLIB fpOLE2Str(LPTSTR lpOleInput)                          
{
#ifdef FP_OCX
//  USES_CONVERSION;
//  return T2OLE(lpString);
#if (defined(WIN32) && !defined(_UNICODE))
/*
  int iCharCt = wcslen((WORD *)lpOleInput);
  BSTR lpStrOut = SysAllocStringByteLen(NULL, (iCharCt / 2));
  for (register int i = 0; i <= iCharCt; i++)
    ((BYTE *)lpStrOut)[i] = (BYTE) (((WORD *)lpOleInput)[i]) ;
*/
  int iLen;
  LPSTR lpStrOut;
  int rc;

  if (lpOleInput == NULL)
    return NULL;

  /* RFW - 5/28/04 - 14248
  //iCharCt = wcslen((WORD FAR *)lpOleInput);
  iCharCt = wcslen((wchar_t FAR *)lpOleInput); //scl
  // allocate same size as OleInput in case every char is MultiByte.
  iLen = iCharCt * 2;
  */

  iLen = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)lpOleInput, -1, NULL, 0,
                             NULL, NULL) - 1;

  lpStrOut = (LPSTR)SysAllocStringByteLen(NULL, iLen);
  lpStrOut[0] = '\0';

  rc = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)lpOleInput, -1, lpStrOut,
                           iLen + 1, NULL, NULL);
  // realloc to new size if not same (note: rc = num copied including null)
  if (rc && (rc < iLen + 1))
  {
    LPSTR strTmp;
    --rc;           // exclude null in re-calc
    strTmp = (LPSTR)SysAllocStringByteLen((LPCSTR)lpStrOut, rc);
    strTmp[rc] = '\0';
    SysFreeString((BSTR)lpStrOut);
    lpStrOut = strTmp;
  }
#else
  BSTR lpStrOut = SysAllocString((BSTR)lpOleInput);
#endif
  return (LPTSTR)lpStrOut;
#else
  return lpOleInput;
#endif  //FP_OCX
}            




/***************************************************************************
 *
 * FUNCTION:  fpPatchVBVariant()
 *
 * DESCRIPTION:
 *
 *   VB 4.0 has a bug with VT_BSTR's (VT_STRING) in VARIANT structures.
 *   The bug only applies when the OCX is returning data to VB 4.0; data
 *   passed from VB 4.0 to the OCX is not affected! 
 *   (ie. OCX-->VB40 bug;  VB40-->OCX ok)
 *
 *   The CHARs of the BSTR have to be converted to WCHARs and the len
 *   of the BSTR has to be adjusted to include the number of BYTES in
 *   the string, but EXCLUDING both null-terminating bytes.
 *
 *   This should only affect OCXs for WIN32 without Unicode, not WIN16.
 *
 * PARAMETERS:
 *
 *   VARIANT FAR *lpVariant
 *
 * RETURNS:
 *
 *   void
 *
 **************************************************************************/
void FPLIB fpPatchVBVariant(VARIANT FAR *lpVariant)                          
{
#if (defined(WIN32) && !defined(_UNICODE) && defined(OLE2ANSI))
  // if not a BSTR, no patch is necessary, just return.
  if ((lpVariant->vt != VT_BSTR) || (lpVariant->bstrVal == NULL))
     return;

  BSTR bstrOld = lpVariant->bstrVal;
  int iCharCt = SysStringLen(bstrOld);
  BSTR bstrNew = SysAllocStringByteLen(NULL, (iCharCt * 2) + 1);
  for (register int i = 0; i <= iCharCt; i++)
    ((WORD *)bstrNew)[i] = (WORD)bstrOld[i];

  // Subtract first byte of null-terminator from len so len excludes
  // both null chars
  *(int *)(((LPSTR)bstrNew)-sizeof(int)) -= 1;

  // Replace current BSTR with new BSTR
  SysFreeString(bstrOld);
  lpVariant->bstrVal = bstrNew;
#endif  //WIN32 & not Unicode

  return;
}
       
#endif  //if FP_OCX

#ifdef __cplusplus
}
#endif
