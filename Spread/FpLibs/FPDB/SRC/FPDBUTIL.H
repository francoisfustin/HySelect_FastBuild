/*************************************************************************\
* FPDBUTIL.H  :  Utility functions for CfpOleBinding class                *
*                                                                         *
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.                   *
* All Rights Reserved.                                                    *
*                                                                         *
* No part of this source code may be copied, modified or reproduced       *
* in any form without retaining the above copyright notice.  This         *
* source code, or source code derived from it, may not be redistributed   *
* without express written permission of FarPoint Technologies.            *
*                                                                         *
\*************************************************************************/
#ifndef FPDBUTIL_H
#define FPDBUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(NO_MFC) && defined(__cplusplus))
 // NO_MFC && NOT C++
#include "fpdblist.h"
#endif // NO_MFC

typedef LPBYTE LBLOCK;     // Ptr to data, length at Addr-4 (sizeof long)

#ifdef WIN32
typedef LPWSTR LPDBWSTR;
// Use OCXCHAR/LPOCXSTR inplace of OLECHAR/LPOLESTR
typedef WCHAR  FPOCHAR, FAR *LPFPOSTR;
#define _FPOCHAR_DEFINED
#else

#ifndef __cplusplus
#define LPCTSTR LPCSTR     // this definition was taken from AFXVER_.H
#endif 

typedef USHORT WCHAR;
typedef WCHAR FAR *LPDBWSTR;
// Use OCXCHAR/LPOCXSTR inplace of OLECHAR/LPOLESTR
typedef char  FPOCHAR, FAR *LPFPOSTR;
#define _FPOCHAR_DEFINED
#endif

void DLLENTRY LBlockSetStringPool( DWORD memPool);

LBLOCK LBlockAllocString(LPCSTR lpcstr);
BOOL   LBlockReAllocString(LBLOCK FAR *lpLBlock, LPCSTR lpcstr);
LBLOCK LBlockAllocTString(LPCTSTR lpctstr);
// Allocates From XStr (LPWSTR or LPSTR) to LPSTR
LBLOCK LBlockAllocXStrToStr(const LPVOID lpxstr, UINT XCharSize);
// Allocates From LPSTR to XStr (LPWSTR or LPSTR)
LBLOCK LBlockAllocStrToXStr(const LPSTR lpstr, UINT XCharSize);
// Allocates from XStr (either LPSTR or LPWSTR) to XStr
LBLOCK LBlockAllocXStrToXStr(const LPVOID lpxstr, UINT XSrcCharSize, 
  UINT XDestCharSize);

LBLOCK DLLENTRY LBlockAllocLen(const LPBYTE pData, LONG lSize);
BOOL   LBlockReAllocLen(LBLOCK FAR *lpLBlock, const LPBYTE pData, 
         LONG lSize);
// Allocates From X-Byte data (LPWORD or LPBYTE) to LPBYTE.
LBLOCK LBlockAllocFromXLen(const LPVOID lpxData, LONG lLen, UINT XCharSize);
// Allocates From LPBYTE data to X-byte data (LPWORD or LPBYTE).
LBLOCK LBlockAllocToXLen(const LPBYTE lpbData, LONG lLen, UINT XCharSize);
// Allocates From X-Byte data to X-Byte data. Char sizes must be 1 or 2.
// Len is number of X-Bytes (either number of WORDS or BYTES)
LBLOCK LBlockAllocXLenToXLen(const LPVOID lpSrcData, LONG lLen, 
  UINT uSrcXCharSize, UINT uDestXCharSize);


void   LBlockFree(LBLOCK LBlock);
BOOL   LBlockAdjustXLen( LBLOCK LBlock, LONG lLen, UINT XCharSize);
LONG   LBlockLen(LBLOCK LBlock);
LBLOCK LBlockClone(LBLOCK LBlock);

BOOL LBlockConvertCharType( LBLOCK FAR *lpLBlock, LONG lLen,
  UINT uSrcCharSize, UINT uDestCharSize);


LPTSTR fpDBCreateHsz(LPTSTR lpszString);
LPTSTR fpDBCreateHszLen(LPTSTR lpszString, UINT cbLen);
LPTSTR fpDBDestroyHsz(LPTSTR hszParm);

int fpConvertTSTRtoType( LPTSTR tstr, LPVOID lpDest, USHORT usDestType);
LPTSTR fpConvertTypeToTSTR( LPVOID lpSource, USHORT usSourceType);

#ifdef __cplusplus
BOOL CopySafeArrayToLBlock(SAFEARRAY FAR *psa, LBLOCK FAR *lpLBlock);
#endif

#ifdef __cplusplus
}
#endif

//*************************************************************************
//****                        DEBUGGING INFO                           ****
//*************************************************************************
#if (defined(_DEBUG) && defined(__cplusplus))
class CfpNotifyEvents : public CObject
{
public:
  CfpNotifyEvents(void);
  virtual ~CfpNotifyEvents(void);
  BOOL Flush(void);

#ifdef FP_ADB
  BOOL Add( LPSTR type, DWORD eReason, BOOL fIgnoringEvents = FALSE);
  void PrivateAdd( LPSTR prefix, DWORD eReason, LPSTR suffix);
#else  // not FP_ADB
  BOOL Add( LPSTR type, ULONG cReason, LPVOID rgReasons, 
    BOOL fIgnoringEvents = FALSE);
  void PrivateAdd( LPSTR prefix, LPVOID rgReason, LPSTR suffix);
#endif

public:
#define NE_MAX_ROWS  50
#define NE_MAX_COLS  50
  int m_ct;
  int m_entrynum;
  char m_list[NE_MAX_ROWS][NE_MAX_COLS];
};
#endif // _DEBUG && __cplusplus

#endif  // ifndef FPDBUTIL_H
