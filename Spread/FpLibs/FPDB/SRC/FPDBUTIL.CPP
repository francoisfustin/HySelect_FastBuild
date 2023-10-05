/*************************************************************************\
* FPDBUTIL.CPP                                                            *
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
#if ((defined(FP_OCX) || defined(FP_DLL)) && defined(WIN32) && !defined(NO_MFC))
#define WIN32_LEAN_AND_MEAN		//exclude conflicts with VBAPI.H in WIN32
#endif

#ifndef NO_MFC
#include <afxctl.h>         // MFC support for OLE Custom Controls
#else  //NO_MFC
#include "stdafx.h"
//ATL$ #include "fpatl.h"
#include "fptools.h"
#include "fptstr.h"
#endif

#include <windows.h>
#include <windowsx.h>

#include "fpconvrt.h"
#include "fptstr.h"

#include <limits.h>
#include <malloc.h>
#include <stdio.h>

#ifndef NO_SH   // !smrtheap

#ifdef __cplusplus

#include "smrtheap.hpp"
#else
#include "smrtheap.h"
#endif

#else
typedef void *MEM_POOL;
#endif    // NO_SH

#include "fpdbutil.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WSIZE sizeof(WORD)
#define BSIZE sizeof(BYTE)

typedef struct SLBLOCKtag   // Struct-LBLOCK
{
  LONG lLen;                // Len EXCLUDING null terminator
  BYTE data[2];             // Data block begins here, add 2 bytes for null
} SLBLOCK, FAR *LPSLBLOCK;

static MEM_POOL fpDBStringPool = NULL;
static ULONG    fpDBSetPoolCt = 0;

void DLLENTRY LBlockSetStringPool( DWORD memPool)
{
  // if the pool is being "reset" decrement count and if 0, set pool to null
  if (memPool == (DWORD)0)
  {
    if (fpDBSetPoolCt)
    {
      --fpDBSetPoolCt;
      if (!fpDBSetPoolCt)    // if 0
        fpDBStringPool = NULL;
    }
  }
  else   // setting pool...
  {
    ++fpDBSetPoolCt;
    if (fpDBSetPoolCt == (ULONG)1)   //first one
      fpDBStringPool = (MEM_POOL)memPool;
  }
}


LBLOCK LBlockAllocString(LPCSTR lpcstr)
{
  LONG len;

  if (!lpcstr)
     return NULL;

  len = (LONG)_fstrlen(lpcstr);
  return LBlockAllocLen((LPBYTE)lpcstr, len);
}

BOOL   LBlockReAllocString(LBLOCK FAR *lpLBlock, LPCSTR lpcstr)
{
  LONG len;

  if (!lpcstr)
     return FALSE;

  len = (LONG)_fstrlen(lpcstr);
  return LBlockReAllocLen( lpLBlock, (LPBYTE)lpcstr, len);
}

// Copies from TSTR (either LPSTR or LPWSTR) to single-byte string
//
LBLOCK LBlockAllocTString(LPCTSTR lpctstr)
{
  return LBlockAllocXStrToStr((const LPVOID)lpctstr, sizeof(TCHAR));
}



// Copies from XString (either LPSTR or LPWSTR) to single-byte string
//
LBLOCK LBlockAllocXStrToStr(const LPVOID lpxstr, UINT XCharSize)
{
  LBLOCK LBlock = NULL;
  LONG   lLen;
  LPDBWSTR lpwstr;

  if (!lpxstr)
     return NULL;

  if ((XCharSize != BSIZE) && (XCharSize != WSIZE))
  {
     ASSERT(0);     // Char size not supported!!! Only sizes 1 & 2
     return NULL;
  }

  if (XCharSize == BSIZE)
     return LBlockAllocString((LPCSTR)lpxstr);

  // else WCHARs...
  lpwstr = (LPDBWSTR)lpxstr;

  // get len
  {
    register long l = 0;
    while (lpwstr[l])
      ++l;
    lLen = l;
  }

  // create buffer and copy
  return LBlockAllocFromXLen( lpwstr, lLen, XCharSize);
}


// Allocates from XString (either LPSTR or LPWSTR) to XString
//
LBLOCK LBlockAllocXStrToXStr(const LPVOID lpxstr, UINT XSrcCharSize, UINT XDestCharSize)
{
  LONG   lLen;

  if (!lpxstr)
     return NULL;

  // get len
  if (XSrcCharSize == BSIZE)
  {
    // BYTEs
    lLen = (LONG)_fstrlen((LPSTR)lpxstr);
  }
  else if (XSrcCharSize == WSIZE)
  {
     // WCHARs...
    register long l = 0;
    register LPDBWSTR lpwstr = (LPDBWSTR)lpxstr;
    while (lpwstr[l])
      ++l;
    lLen = l;
  }
  else
  {
    ASSERT(0);     // Char size not supported!!! Only sizes 1 & 2
    return NULL;
  }
     
  // create buffer and copy
  return LBlockAllocXLenToXLen( lpxstr, lLen, XSrcCharSize, XDestCharSize);
}

// Copies from Single-byte string to XString (either LPSTR or LPWSTR)
//
LBLOCK LBlockAllocStrToXStr(const LPSTR lpstr, UINT XCharSize)
{
  LONG   lLen;

  if (!lpstr)
     return NULL;

  if ((XCharSize != BSIZE) && (XCharSize != WSIZE))
  {
     ASSERT(0);     // Char size not supported!!! Only sizes 1 & 2
     return NULL;
  }

  if (XCharSize == BSIZE)
     return LBlockAllocString(lpstr);

  // get len
  lLen = (LONG)_fstrlen(lpstr);

  return LBlockAllocToXLen( (const LPBYTE)lpstr, lLen, XCharSize);
}


// After allocation, a null-terminator is put at end-of-block and
// also at either the end-of-data or beginning of block (if no data).
// returns ptr to new block.
LBLOCK DLLENTRY LBlockAllocLen(const LPBYTE pData, LONG lSize)
{
  LONG lallocsize;
  LPSLBLOCK lpSLBlock;
  LBLOCK    LBlock = NULL;

  // add overhead to size of block and check its size
  lallocsize = lSize + sizeof(SLBLOCK);
#ifdef WIN32
  if ((lSize < 0) || (lallocsize < 0))
     return NULL;
#else
  if ((ULONG)lallocsize > (LONG)USHRT_MAX)
     return NULL;
#endif
  
  // allocate block
#ifndef NO_SH                                        // !smrtheap
  // Use SmartHeap instead of malloc() & free()
  lpSLBlock = (LPSLBLOCK)MemAllocPtr(fpDBStringPool, (DWORD)lallocsize,
     TRUE);
#else
  lpSLBlock = (LPSLBLOCK)_fmalloc((size_t)lallocsize);
#endif                                                 // !smrtheap

  if (lpSLBlock)
  {
    // set-up structure
    lpSLBlock->lLen = lSize; 
    LBlock = &lpSLBlock->data[0];       // temp ptr to "data"
    *(LPWORD)&LBlock[lSize] = (WORD)0;  // null at end of block
    // copy data
    if (pData)
       _fmemcpy( LBlock, pData, (size_t)lSize);
    else
       *(LPWORD)LBlock = (WORD)0;      // no data, null at beginning
  }
  
  return LBlock;                       // return data portion of allocation
}



// After allocation, a null-terminator is put at end-of-block and
// also at the end-of-data (if data).
// returns ptr to new block.
BOOL   LBlockReAllocLen(LBLOCK FAR *lpLBlock, const LPBYTE pData, 
         LONG lSize)
{
  LONG lallocsize;
  LPSLBLOCK lpSLBlock;
  LBLOCK    LBlock = NULL;

  if (!lpLBlock)
     return FALSE;

  // Get ptr to SLBOCK structure
  lpSLBlock = (LPSLBLOCK)(*lpLBlock - sizeof(LONG));

  // add overhead to size of block and check its size
  lallocsize = lSize + sizeof(SLBLOCK);
#ifdef WIN32
  if ((lSize < 0) || (lallocsize < 0))
     return FALSE;
#else
  if ((ULONG)lallocsize > (LONG)USHRT_MAX)
     return FALSE;
#endif
  
  // allocate block
#ifndef NO_SH                                    // !smrtheap
  lpSLBlock = (LPSLBLOCK)MemReAllocPtr((LPVOID)lpSLBlock,
                           (DWORD)lallocsize, TRUE);
#else
  lpSLBlock = (LPSLBLOCK)_frealloc((LPVOID)lpSLBlock, (size_t)lallocsize);
#endif                                           // !smrtheap  

  if (lpSLBlock)
  {
    // set-up structure
    lpSLBlock->lLen = lSize; 
    LBlock = &lpSLBlock->data[0];          // temp ptr to "data"
    *(LPWORD)&LBlock[lSize] = (WORD)0;  // null at end of block
    // copy data
    if (pData)
       _fmemcpy( LBlock, pData, (size_t)lSize);
    *lpLBlock = LBlock;
  }
  
  return TRUE;
}

// Copies from X-byte data (either LPBYTE or LPWORD) to single-byte buffer.
//
// Len is number of X-bytes (if X-byte is a WORD, then Len is half
// the number of actual bytes).
LBLOCK LBlockAllocFromXLen(const LPVOID lpxData, LONG lLen, UINT XCharSize)
{
  LPBYTE lpStrOut;
  int rc;

  if (lpxData == NULL)
    return NULL;

  if ((XCharSize != BSIZE) && (XCharSize != WSIZE))
  {
     ASSERT(0);     // Char size not supported!!! Only sizes 1 & 2
     return NULL;
  }

  if (XCharSize == BSIZE)
     return LBlockAllocLen((const LPBYTE)lpxData, lLen);

  // create buffer and copy (WSIZE to BSIZE)
#ifndef WIN32   //if WIN16
  lpStrOut = (LPBYTE)LBlockAllocLen( NULL, lLen);
  if (lpStrOut)
  {
    // copy LPWSTR to LBlock. one "char" at a time
    LPWORD lpwData = (LPWORD)lpxData;
    for (register long l = 0; l < lLen; l++)
      lpStrOut[l] = (BYTE)(lpwData[l]);
  }

#else //if WIN32

  // allocate same size as lpxData in case every char is MultiByte.
  lpStrOut = (LPBYTE)LBlockAllocLen(NULL, (lLen * 2));
  lpStrOut[0] = (BYTE)'\0';

  rc = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)lpxData, -1, (LPSTR)lpStrOut,
                           (int)(lLen * 2) + 1, NULL, NULL);
  // realloc to new size if not same (note: rc = num copied including null)
  if (rc && ((LONG)rc != (lLen * 2)))
  {
    LPBYTE strTmp;
    --rc;           // exclude null in re-calc
    strTmp = LBlockAllocLen((const LPBYTE)lpStrOut, (LONG)rc);
    strTmp[rc] = (BYTE)'\0';
    LBlockFree((LBLOCK)lpStrOut);
    lpStrOut = strTmp;
  }
#endif

  return (LBLOCK)lpStrOut;
}


// Copies from Single-byte buffer to X-byte data (either LPBYTE or LPWORD)
//
// Len is number of bytes in source buffer
//
LBLOCK LBlockAllocToXLen(const LPBYTE lpbData, LONG lLen, UINT XCharSize)
{
  LPWORD lpStrOut;
  int rc;

  if (lpbData == NULL)
	 return NULL;

  if ((XCharSize != BSIZE) && (XCharSize != WSIZE))
  {
     ASSERT(0);     // Char size not supported!!! Only sizes 1 & 2
     return NULL;
  }

  if (XCharSize == BSIZE)
     return LBlockAllocLen(lpbData, lLen);

  // Create buffer and copy (from BSIZE to WSIZE).

#ifndef WIN32  //if WIN16
  lpStrOut = (LPWORD)LBlockAllocLen( NULL, lLen * WSIZE);
  if (lpStrOut)
  {
    // copy lpData to LBlock (as LPWORD)
    for (register long l = 0; l < lLen; l++)
      lpStrOut[l] = (WORD)(lpbData[l]);
  }
#else  //WIN32

  // fix for 16257 -scl
  lLen = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpbData, -1, NULL, 0) - 1;
  lpStrOut = (LPWORD)LBlockAllocLen(NULL, lLen * 2);
  ASSERT(lpStrOut != NULL);
  if (!lpStrOut)
    return NULL;

  lpStrOut[0] = (WORD)0;
  rc = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpbData, -1, (LPWSTR)lpStrOut, (int)lLen);
  // realloc to new size if not same (note: rc = num copied including null)
  if (rc && ((LONG)rc != lLen))
  {
    LPWORD wstrTmp;
    --rc;           // exclude null in re-calc
// fix for 14248 -scl
//    wstrTmp = (LPWORD)LBlockAllocLen((const LPBYTE)lpStrOut, ((LONG)rc * 2) + 1);
    wstrTmp = (LPWORD)LBlockAllocLen((const LPBYTE)lpStrOut, ((LONG)rc * 2));
//    wstrTmp[rc] = (WORD)0;
    LBlockFree((LBLOCK)lpStrOut);
    lpStrOut = wstrTmp;
  }
#endif

  return (LBLOCK)lpStrOut;
}


// Copies from X-byte data to X-byte data (either LPBYTE or LPWORD)
//
// Len is number of X-bytes in source buffer
//
LBLOCK LBlockAllocXLenToXLen(const LPVOID lpSrcData, LONG lLen, 
  UINT uSrcXCharSize, UINT uDestXCharSize)
{
  // Following combinations:
  //   1. Source: BYTE   Dest: BYTE   (matching types)
  //   2. Source: BYTE   Dest: WORD 
  //   3. Source: WORD   Dest: BYTE 
  //   4. Source: WORD   Dest: WORD   (matching types)
  //
  // Anything else is an error

  // This takes care of cases 1 & 2.  NOTE: if destCharSize is not 1 or 2,
  // the function call will produce an error.
  if (uSrcXCharSize == BSIZE)
    return LBlockAllocToXLen((const LPBYTE)lpSrcData, lLen, uDestXCharSize);

  // Case 3.
  if ((uSrcXCharSize == WSIZE) && (uDestXCharSize == BSIZE))
    return LBlockAllocFromXLen(lpSrcData, lLen, WSIZE);

  // Case 4.
  if ((uSrcXCharSize == WSIZE) && (uDestXCharSize == WSIZE))
    return LBlockAllocLen((const LPBYTE)lpSrcData, lLen * WSIZE);

  // Else, parameters are invalid, the only allowed char sizes are 1 or 2 !
  ASSERT(0);
  return NULL;
}


void LBlockFree(LBLOCK LBlock)
{
  if (!LBlock)
     return;
  // get ptr to SLBLOCK structure and free it
#ifndef NO_SH                                    // !smrtheap
  MemFreePtr((LPVOID)(LBlock - sizeof(LONG)));
#else
  _ffree((LPVOID)(LPSLBLOCK)(LBlock - sizeof(LONG)));
#endif                                           // !smrtheap
}

// This function adjusts the length member and terminates the
// data with a null.  NOTE: If XCharSize is 2, lLen is number of WORDS
// and data is terminated with 2 null bytes.
// 
BOOL LBlockAdjustXLen( LBLOCK LBlock, LONG lLen, UINT XCharSize)
{
  BOOL fReturn = FALSE;
  LPLONG lplLen;

  if (!LBlock)
     return FALSE;

  if ((XCharSize == BSIZE) || (XCharSize == WSIZE))
  {
    lplLen = &(((LPSLBLOCK)(LBlock - sizeof(LONG)))->lLen);
    if (XCharSize == WSIZE)
       lLen *= WSIZE;

    if (*lplLen <= lLen)
    {
      *lplLen = lLen;
      *(LPWORD)&LBlock[lLen] = (WORD)0;
      fReturn = TRUE;
    }
  }

  return fReturn;
}



LONG   LBlockLen(LBLOCK LBlock)
{
  if (!LBlock)
     return 0;

  // Get ptr to SLBLOCK structure and return the len
  return ((LPSLBLOCK)(LBlock - sizeof(LONG)))->lLen;
}

LBLOCK LBlockClone(LBLOCK LBlock)
{
  if (!LBlock)
     return NULL;

  return LBlockAllocLen( (LPBYTE)LBlock, LBlockLen(LBlock));
}

BOOL LBlockConvertCharType( LBLOCK FAR *lpLBlock, LONG lLen,
  UINT uSrcCharSize, UINT uDestCharSize)
{
  LBLOCK tmpBlock;

  if (!lpLBlock || !*lpLBlock)
    return FALSE;

  // if the types are the same, just return
  if (uSrcCharSize == uDestCharSize)
    return TRUE;

  // Different sizes, use "special" alloc function
  tmpBlock = LBlockAllocXLenToXLen( *lpLBlock, lLen, uSrcCharSize,
                                    uDestCharSize);
  // if it failed, return FALSE
  if (tmpBlock == NULL)
    return FALSE;

  // else, free old block and use new block
  LBlockFree(*lpLBlock);
  *lpLBlock = tmpBlock;

  return TRUE;
/*
  // If converting from WORDs to BYTEs, starting from index 0, move the
  // lo-byte to its byte location.  When finished, re-allocate LBlock to 
  // half the len.
  if ((uSrcCharSize == WSIZE) && (uDestCharSize == BSIZE))
  {
    register LPWORD lpwSrc = (LPWORD)*lpLBlock;
    register LPBYTE lpbDest = *lpLBlock;
    for (register long l = 0; l < lLen; l++)
    {
      *lpbDest = (BYTE)*lpwSrc;
      ++lpbDest;
      ++lpwSrc;
    }
    return LBlockReAllocLen( lpLBlock, NULL, lLen);
  }

  // Else if converting from BYTEs to WORDs, re-allocate buffer to twice
  // its size, and starting at index lLen-1, move bytes to word locations.
  if ((uSrcCharSize == BSIZE) && (uDestCharSize == WSIZE))
  {
    if (!LBlockReAllocLen(lpLBlock, NULL, lLen * WSIZE))
      return FALSE;

    register LPBYTE lpbSrc = &((*lpLBlock)[lLen - 1]);
    register LPWORD lpwDest = (LPWORD)lpbSrc;
    for (register long l = (lLen - 1); l >= 0; l--)
    {
      *lpwDest = (WORD)*lpbSrc;
      ++lpbSrc;
      ++lpwDest;
    }
    return TRUE;
  }
  // bad parameters
  ASSERT(0);
  return FALSE;
*/
}


FPTSTR fpDBCreateHsz(FPTSTR lpszString)
{
//  return (LPSTR)SysAllocString((LPTSTR)lpszString);
  return (FPTSTR)fpSysAllocTString((FPTSTR)lpszString);
}

FPTSTR fpDBCreateHszLen( FPTSTR lpszString, UINT cbLen) 
{
//  return (LPSTR)SysAllocStringLen((LPTSTR)lpszString, cbLen);
  return (FPTSTR)fpSysAllocTStringLen((FPTSTR)lpszString, cbLen);
}

FPTSTR fpDBDestroyHsz(FPTSTR hszParm)
{
  fpSysFreeTString((FPTSTR)hszParm);
  return NULL;
}


// Returns the size of the returned data (ex. VT_R4 returns 4)
// or -1 if failed.
int fpConvertTSTRtoType( FPTSTR tstr, LPVOID lpDest, USHORT usDestType)
{
   VARIANTARG varData1, varData2;
   HRESULT h = (HRESULT)-1;
   int iReturn = -1;

   if (*(FPTSTR)tstr)
   {
     BSTR bstr = (BSTR)fpSysAllocBSTRFromTString((LPTSTR)tstr);
     VariantInit(&varData2);
     VariantInit(&varData1);
     varData1.vt = VT_BSTR;
     varData1.bstrVal = (BSTR)bstr;
     h = VariantChangeType(&varData2, &varData1, (USHORT)0, usDestType);
     if (bstr)
       fpSysFreeTString((LPTSTR)bstr);
   }
//   ASSERT(h == S_OK);         // Could not convert BSTR to DATE!
   if (h == S_OK)
   {
     switch (usDestType)
     {
       case VT_DATE:
         *(double FAR *)lpDest = (double)varData2.date;
         iReturn = sizeof(double);
         break;

       case VT_I2:
         *(short FAR *)lpDest = varData2.iVal;
         iReturn = sizeof(short);
         break;

       case VT_I4:
         *(long FAR *)lpDest = varData2.lVal;
         iReturn = sizeof(long);
         break;

       case VT_R4:
         *(float FAR *)lpDest = varData2.fltVal;
         iReturn = sizeof(float);
         break;

       case VT_R8:
         *(double FAR *)lpDest = varData2.dblVal;
         iReturn = sizeof(double);
         break;

       case VT_CY:
         *(CY FAR *)lpDest = varData2.cyVal;
         iReturn = sizeof(CY);
       break;


       case VT_BOOL:
#ifdef WIN32
         *(VARIANT_BOOL FAR *)lpDest = varData2.boolVal;
#else
         *(VARIANT_BOOL FAR *)lpDest = varData2.bool;
#endif
         iReturn = sizeof(VARIANT_BOOL);
         break;

       case VT_UI1:
#ifdef WIN32
         *(BYTE FAR *)lpDest = varData2.bVal;
#else
         *(BYTE FAR *)lpDest = (BYTE)varData2.iVal;
#endif
         iReturn = sizeof(BYTE);
         break;

       default:       // Note: default of -1 is returned
         ASSERT(0);   // Add support for this type!!!
         break;
     }
   }

   return iReturn;
}


// Returns the TSTR to the string 
// or NULL if failed.
FPTSTR fpConvertTypeToTSTR( LPVOID lpSource, USHORT usSourceType)
{
   VARIANTARG varData1, varData2;
   HRESULT h;
   FPTSTR tstr = NULL;

   VariantInit(&varData1);
   VariantInit(&varData2);

   switch (usSourceType)
   {
     case VT_DATE:
       (double)varData1.date = *(double FAR *)lpSource;
       break;

     case VT_UI1:
#ifdef WIN32     
       varData1.bVal = *(BYTE FAR *)lpSource;
#else // WIN16 - 16bit does not have a "bVal" member
       *(BYTE FAR *)&varData1.iVal = *(BYTE FAR *)lpSource;
#endif       
       break;

     case VT_I2:
       varData1.iVal = *(short FAR *)lpSource;
       break;

     case VT_I4:
       varData1.lVal = *(long FAR *)lpSource;
       break;

     case VT_R4:
       varData1.fltVal = *(float FAR *)lpSource;
       break;

     case VT_R8:
       varData1.dblVal = *(double FAR *)lpSource;
       break;

     case VT_CY:
       varData1.cyVal = *(CY FAR *)lpSource;
       break;

     case VT_BOOL:
#ifdef WIN32
       varData1.boolVal = *(VARIANT_BOOL FAR *)lpSource;
#else
       varData1.bool = *(VARIANT_BOOL FAR *)lpSource;
#endif
       break;

     case VT_DECIMAL:
       varData1.decVal = *(DECIMAL FAR *)lpSource;
	   break;

     default:       // Note: default of -1 is returned
       ASSERT(0);   // Add support for this type!!!
       return NULL;
   }

   varData1.vt = usSourceType;
   h = VariantChangeType(&varData2, &varData1, (USHORT)0, VT_BSTR);

   // if it worked, change BSTR to TSTR
   if ((h == S_OK) && varData2.bstrVal)
   {
     tstr = (FPTSTR)fpSysAllocTStringFromBSTR(varData2.bstrVal);
     SysFreeString(varData2.bstrVal);
   }

   return (FPTSTR)tstr;
}

#ifdef __cplusplus
BOOL CopySafeArrayToLBlock(SAFEARRAY FAR *psa, LBLOCK FAR* lpLBlock)
{ 
  LONG lsize = (LONG)0;
  if (SafeArrayGetUBound( psa, 1, &lsize) == S_OK)
     ++lsize;                  // UpperBound + 1 = Length

  if (lsize)
    *lpLBlock = LBlockAllocLen((LPBYTE)psa->pvData, lsize);

  return ((lsize > (LONG)0) && *lpLBlock);

}
#endif  //__cplusplus

#ifdef __cplusplus
}
#endif

#if (defined(_DEBUG) && defined(__cplusplus))

#ifndef __OCDB_H_
#include <ocdbid.h>
#include <olebind.h>
#include <ocdb.h>
#endif

#ifdef FP_ADB
  #include "fpoledba.h"
#else
  #include "fpdbnote.h"
#endif

CfpNotifyEvents::CfpNotifyEvents(void)
{
}

CfpNotifyEvents::~CfpNotifyEvents(void)
{
}

BOOL CfpNotifyEvents::Flush()
{
  m_ct = 0;
  m_entrynum = 1;
  memset( m_list, 0, sizeof(m_list));
  return TRUE;
}

#ifdef FP_ADB
BOOL CfpNotifyEvents::Add( LPSTR type, DWORD eReason,
  BOOL fIgnoringEvents)
{
  CHAR prefix[20];
  LPSTR suffix;

  sprintf( (LPSTR)prefix, "%2d %s ", m_entrynum, type);
  if (fIgnoringEvents)
    suffix = " I";
  else
    suffix = "";

  PrivateAdd( (LPSTR)prefix, eReason, suffix);

  ++m_entrynum;
  if (m_entrynum > 99)
     m_entrynum = 1;

  return TRUE;
}

#else // not FP_ADB
BOOL CfpNotifyEvents::Add( LPSTR type, ULONG cReasons, LPVOID rgReasons,
  BOOL fIgnoringEvents)
{
  CHAR prefix[20];
  LPSTR suffix;
  int  i;
  DBNOTIFYREASON FAR *rgReasonsArray = (DBNOTIFYREASON FAR *)rgReasons; 

  sprintf( (LPSTR)prefix, "%2d %s ", m_entrynum, type);
  if (fIgnoringEvents)
    suffix = " I";
  else
    suffix = "";

  for (i = 0; i < (int)cReasons; i++)
  {
    if (rgReasons)
      PrivateAdd( (LPSTR)prefix, (LPVOID)(&rgReasonsArray[i]), suffix);
    else
      PrivateAdd( (LPSTR)prefix, NULL, suffix);
  }

  ++m_entrynum;
  if (m_entrynum > 99)
     m_entrynum = 1;

  return TRUE;
}
#endif // if-else FP_ADB


#ifdef FP_ADB
void CfpNotifyEvents::PrivateAdd( LPSTR prefix, DWORD eReason, 
  LPSTR suffix)
{
  LPSTR s;
  // if list is full, trash first 6 rows
  if (m_ct == NE_MAX_ROWS)
  {
    m_ct -= 6;
    memmove( m_list, &m_list[6][0], m_ct * NE_MAX_COLS);
    memset( &m_list[m_ct][0], 0, (NE_MAX_ROWS - m_ct) * NE_MAX_COLS);
  } 

  s = "";
  switch (eReason)
  {
    case DBREASON_ROWSET_RELEASE:      //  1
      s = "Rowset_release (1)"; break;
    case DBREASON_COLUMN_SET:          //  2
      s = "Column_set (2)"; break;
    case DBREASON_COLUMN_RECALCULATED: //  3
      s = "Column_recalc (3)"; break;
    case DBREASON_ROW_DELETE:          //  6
      s = "Row_delete (6)"; break;
    case DBREASON_ROW_INSERT:          //  8
      s = "Row_insert (8)"; break;
    case DBREASON_ROW_UPDATE:          // 13
      s = "Row_update (13)"; break;
    case DBREASON_ROWSET_CHANGED:      // 14
      s = "Rowset_changed (14)"; break;

    // These reasons are defined in OLE DB 1.5 & 2.0. My documentation
    // does not explain these (OLEDB 1.1 doc), so for now, we will
    // assert.
    case DBREASON_ROWPOSITION_CHANGED:        // 15  (ver 1.5)
      s = "Position_changes (15)"; break;
    case DBREASON_ROWPOSITION_CHAPTERCHANGED: // 16  (ver 1.5)
      s = "Position_chapter (16)"; break;
    case DBREASON_ROWPOSITION_CLEARED:        // 17  (ver 1.5)
      s = "Position_cleared (17)"; break;
    case DBREASON_ROW_ASYNCHINSERT:           // 18  (ver 1.5)
      s = "Row_asynchinsert (18)"; break;

    case DBREASON_ROWSET_ROWSADDED:           // 19  (ver 2.0)
      s = "Rowset_rowsadded (19)"; break;
    case DBREASON_ROWSET_POPULATIONCOMPLETE:  // 20  (ver 2.0)
      s = "Rowset_popul_complete (20)"; break;
    case DBREASON_ROWSET_POPULATIONSTOPPED:   // 21  (ver 2.0)
      s = "Rowset_popul_stopped (21)"; break;

    case DBREASON_ROWSET_FETCHPOSITIONCHANGE: //  0
      s = "SKIP-Rowset_fetchposchange (0)"; break;
    case DBREASON_ROW_ACTIVATE:               //  4
      s = "SKIP-Row_activate (4)"; break;
    case DBREASON_ROW_RELEASE:                //  5
      s = "SKIP-Row_release (5)"; break;
    case DBREASON_ROW_FIRSTCHANGE:            //  7
      s = "SKIP-Row_firstchange (7)"; break;
    case DBREASON_ROW_RESYNCH:                //  9
      s = "SKIP-Row_resynch (9)"; break;
    case DBREASON_ROW_UNDOCHANGE:             // 10
      s = "SKIP-Row_undochange (10)"; break;
    case DBREASON_ROW_UNDOINSERT:             // 11
      s = "SKIP-Row_undo_insert (11)"; break;
    case DBREASON_ROW_UNDODELETE:             // 12
      s = "SKIP-Row_undodelete (12)"; break;

    // User-defined DBREASONs to trigger special VBM_DATA messages 
    // not represented in the current list of DBREASONs.
    case UDBREASON_INIT_CONNECT:
      s = "U_Connect"; break;
    case UDBREASON_DISCONNECT:
      s = "U_Disconnect"; break;
    case UDBREASON_SOURCECHANGED:
      s = "U_SourceChanged"; break;
    case UDBREASON_SELF_REFRESH:
      s = "U_SelfRefresh"; break;

    default:
      s = "Unknown";
  }

  sprintf( (LPSTR)m_list[m_ct], "%s%s%s", prefix, s, suffix);
  ++m_ct;
}

#else // not FP_ADB
void CfpNotifyEvents::PrivateAdd( LPSTR prefix, LPVOID rgReason, 
  LPSTR suffix)
{
  LPSTR s;
  // if list is full, trash first 6 rows
  if (m_ct == NE_MAX_ROWS)
  {
    m_ct -= 6;
    memmove( m_list, &m_list[6][0], m_ct * NE_MAX_COLS);
    memset( &m_list[m_ct][0], 0, (NE_MAX_ROWS - m_ct) * NE_MAX_COLS);
  } 

  s = "";
  if (rgReason)
  {
    switch (((DBNOTIFYREASON FAR *)rgReason)->dwReason)
    {
      case DBREASON_DELETED:
        s = "Deleted"; break;
      case DBREASON_INSERTED:
        s = "Inserted"; break;
      case DBREASON_MODIFIED:
        s = "Modified"; break;
      case DBREASON_REMOVEDFROMCURSOR:
        s = "Removedfromcursor"; break;
      case DBREASON_MOVEDINCURSOR:
        s = "Movedincursor"; break;
      case DBREASON_MOVE:
        s = "Move"; break;
      case DBREASON_FIND:
        s = "Find"; break;
      case DBREASON_NEWINDEX:
        s = "Newindex"; break;
      case DBREASON_ROWFIXUP:
        s = "Rowfixup"; break;
      case DBREASON_RECALC:
        s = "Recalc"; break;
      case DBREASON_REFRESH:
        s = "Refresh"; break;
      case DBREASON_NEWPARAMETERS:
        s = "Newparameters"; break;
      case DBREASON_SORTCHANGED:
        s = "Sortchanged"; break;
      case DBREASON_FILTERCHANGED:
        s = "Filterchanged"; break;
      case DBREASON_QUERYSPECCHANGED:
        s = "Queryspecchanged"; break;
      case DBREASON_SEEK:
        s = "Seek"; break;
      case DBREASON_PERCENT:
        s = "Percent"; break;
      case DBREASON_FINDCRITERIACHANGED:
        s = "Findcriteriachanged"; break;
      case DBREASON_SETRANGECHANGED:
        s = "Setrangechanged"; break;
      case DBREASON_ADDNEW:
        s = "Addnew"; break;
      case DBREASON_MOVEPERCENT:
        s = "Movepercent"; break;
      case DBREASON_BEGINTRANSACT:
        s = "Begintransact"; break;
      case DBREASON_ROLLBACK:
        s = "Rollback"; break;
      case DBREASON_COMMIT:
        s = "Commit"; break;
      case DBREASON_CLOSE:
        s = "Close"; break;
      case DBREASON_BULK_ERROR:
        s = "Bulk_error"; break;
      case DBREASON_BULK_NOTTRANSACTABLE:
        s = "Bulk_nottransactable"; break;
      case DBREASON_BULK_ABOUTTOEXECUTE:
        s = "Bulk_abouttoexecute"; break;
      case DBREASON_CANCELUPDATE:
        s = "Cancelupdate"; break;
      case DBREASON_SETCOLUMN:
        s = "Setcolumn"; break;
      case DBREASON_EDIT:
        s = "Edit"; break;

      // User-defined DBREASONs to trigger special VBM_DATA messages 
      // not represented in the current list of DBREASONs.
      case UDBREASON_INIT_CONNECT:
        s = "U_Connect"; break;
      case UDBREASON_DISCONNECT:
        s = "U_Disconnect"; break;
      case UDBREASON_SOURCECHANGED:
        s = "U_SourceChanged"; break;
      case UDBREASON_SELF_REFRESH:
        s = "U_SelfRefresh"; break;

      default:
        s = "Unknown";
    }
  }

  sprintf( (LPSTR)m_list[m_ct], "%s%s%s", prefix, s, suffix);
  ++m_ct;
}
#endif // if-else FP_ADB


#endif //_DEBUG && __cplusplus


