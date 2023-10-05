/****************************************************************************
* FPATLSTM.CPP  -  Implementation of FarPoint's ATL Functions for 
*                  converting from VC project with MFC-built OCX to
*                  VC project with ATL-built OCX.
*
* Copyright (C) 1991-1998 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
****************************************************************************/
#include "stdafx.h"
#include "fpatl.h"

#include "fpatlstm.h"
#include "fpxprop.h"

#include <malloc.h>
#include <ole2.h>
#include <oleauto.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------
// Stock property mask
//-----------------------------------------------------------------------
#define STOCKPROP_BACKCOLOR     0x00000001
#define STOCKPROP_CAPTION       0x00000002
#define STOCKPROP_FONT          0x00000004
#define STOCKPROP_FORECOLOR     0x00000008
#define STOCKPROP_TEXT          0x00000010
#define STOCKPROP_BORDERSTYLE   0x00000020
#define STOCKPROP_ENABLED       0x00000040
#define STOCKPROP_APPEARANCE    0x00000080


//#define new DEBUG_NEW

#define TRACE0  ATLTRACE

//-----------------------------------------------------------------------
//
// Typedef's
//
//-----------------------------------------------------------------------

// This structure is used to create a lookup table of property names and
// dispid pairs.  It provides a fast way of finding dispid's for
// a property name. 
typedef struct 
{
  LPTSTR propName;
  DISPID dispid;
} PROPNAME_ID_PAIR, * LPPROPNAME_ID_PAIR;

// This structure is used to pass several parameters to a couple of
// functions which need all of these values. 
typedef struct
{
  LPPROPNAME_ID_PAIR pPropList;          // required, sorted list of props
  int                iPropCt;            // required, count of above list
  A_LPFPATL_OLDPROP  pOldPropList;       // optional, this may be null
  int                iOldPropCt;         // optional, count of above list
  LPSTREAM           pstmSrc;            // required, stream to read from
  LPSTREAM           pstmDest;           // required, stream to write persist props
  LPSTREAM           pstmOldPropDest;    // optional, stream to write "old" props
} READPROP_PARAMS, *LPREADPROP_PARAMS;

//-----------------------------------------------------------------------
//
// Function forward declarations
//
//-----------------------------------------------------------------------
extern "C"
{
  int __cdecl ComparePropNames(const void *elem1, const void *elem2);
  int __cdecl CompareOldPropNames(const void *elem1, const void *elem2);
}
static HRESULT CopyOldPropStream(CfpAtlBase *pfpAtlCtl, 
  LPSTREAM pstmDest, LPSTREAM pstmOldProps);

static void FreePropNameList(LPPROPNAME_ID_PAIR pList, int iCt);
static void FreeOldPropList(A_LPFPATL_OLDPROP pList, int iCt);
static BOOL BuildPropNameList(ATL_PROPMAP_ENTRY *pPropList,
  LPPROPNAME_ID_PAIR *ppRetList, int *pRetCt);
static BOOL BuildOldPropList(LPFPATL_OLDPROP pOldProps,
  A_LPFPATL_OLDPROP *ppRetList, int *pRetCt);
static BOOL LookupPropName(LPPROPNAME_ID_PAIR pPropList, int iPropCt,
  LPCTSTR tstrPropName, SHORT *psRetDispid);
static BOOL LookupOldPropName(A_LPFPATL_OLDPROP pPropList, 
  int iPropCt, LPCTSTR tstrPropName, SHORT *psRetDispid);

static HRESULT CreateStream(LPSTREAM *ppstmRet);
static BOOL ReadStockProps(LPREADPROP_PARAMS pReadPms);

static  HRESULT ReadFromStreamIntoVariant(LPSTREAM pStream, CComVariant *pvar,
  VARTYPE vtRead);

// Flags for "iConvertFlags" parameter on ReadAndConvertProp() use
// FPCNVT_FLAG_NONE and FPCNVT_FLAG_ENUM.
static HRESULT ReadAndConvertProp(LPCTSTR tstrPropName, VARTYPE vtType,
  int iConvertFlags, CComVariant *pvar, 
  LPREADPROP_PARAMS pReadPms);

static HRESULT ReadStringLength(LPSTREAM pstm, UINT *puiLen);



/***************************************************************************
 *
 * FUNCTION:  fpAtlConvertMfcStream()
 *
 * DESCRIPTION:
 *
 *   Converts a set of property values from an IStream saved with a
 *   control using MFC to an IStream used by a control using ATL.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
HRESULT __cdecl  fpAtlConvertMfcStream( 
  ATL_PROPMAP_ENTRY* pMap, CfpAtlBase *pfpAtlCtl,
  LPSTREAM *ppstmDest, LPSTREAM pstmSrc, 
  LPFP_PROPLIST lpPropList, SHORT nPropListCnt, WORD wFlags,
  LPFPATL_CNVT_PROP lpAddlPropList, LPFPATL_OLDPROP lpOldProps)
{
  SHORT   i;
  BOOL    fIsOldFileVer = wFlags & FPX_OLDVER;
  HRESULT hr;
  BOOL    fResult;
  BOOL    fIgnoreProp;
  VARTYPE vtType;
  int     iCnvtFlags;
  CComVariant var;
  LPSTREAM pstmDest = NULL;
  READPROP_PARAMS readPms;

  // Initialize parameter structure
  memset(&readPms, 0, sizeof(readPms));
  readPms.pstmSrc = pstmSrc;

  // Create (or reset) a stream to copy the properties to.
  hr = CreateStream(ppstmDest);
  if (hr != S_OK)
    return hr;
  readPms.pstmDest = pstmDest = *ppstmDest;

  // Create a sorted list of persist property names (& indexes).
  fResult = BuildPropNameList(pMap, &readPms.pPropList, &readPms.iPropCt);
  _ASSERT(fResult);  // Failed to create sorted prop list!
  if (!fResult)
    return S_FALSE;

  // Create a sorted list of OLD properties
  if (lpOldProps)
  {
    // Create (or reset) a stream to copy the old properties to.
    hr = CreateStream(&readPms.pstmOldPropDest);
    if (hr != S_OK)
      return hr;

    // Build the lookup table (list) of "old props".
    fResult = BuildOldPropList(lpOldProps, &readPms.pOldPropList, 
      &readPms.iOldPropCt);
    _ASSERT(fResult);  // Failed to create sorted prop list!
    if (!fResult)
      return S_FALSE;
  }

  // Read the persist stock properties
  fResult = ReadStockProps(&readPms);

  if (fResult)
  {
    // Read all other properties
    for (i = 0; i < nPropListCnt; i++)
    {
      // If this is a BLOB property, load the blob...
      if (!(lpPropList[i].wEnvironment & FP_NO_OCX) &&
          (lpPropList[i].wEnvironment & FP_BLOB))
      {
        HGLOBAL hBlob = 0;
        int iPropId;
  
        // PX_Blob(pPX, lpPropList[i].pPropInfo->npszName, hBlob, 0);
        hr = pfpAtlCtl->fpAtlLoadBlob(pstmSrc, &hBlob);
        _ASSERT(hr == S_OK);
        if (hr != S_OK)
          break;
  
        if (pfpAtlCtl->fpAtlHasBlobData(&iPropId, NULL) && hBlob)
        {
          SHORT sIndex = FPATL_STM_BLOBPROP;
          hr = pstmDest->Write(&sIndex, sizeof(SHORT), NULL); // write index
          if (hr == S_OK)
            hr = pfpAtlCtl->fpAtlSaveBlob(pstmDest, hBlob);
        }
        if (hBlob)
          GlobalFree(hBlob);
      }
      // Else if, this is a save/load prop, load it...
      else if (lpPropList[i].bDataType >= 0 &&
               !(lpPropList[i].wEnvironment & FP_NO_OCX) &&
               lpPropList[i].pPropInfo < fpPPROPINFO_STD_LAST &&
              (
               (!(lpPropList[i].pPropInfo->fl & fpPF_fNoShow) &&
                (lpPropList[i].pPropInfo->fl & fpPF_fSaveData) &&
  			  // if loading an old file version, load props with fLoadDataOnly 
  			  // unless fpFP_fNewSaveData is set.  (this flag is for props
  			  // that have fSaveData, but are not in the old version of the control 
  			  // -- these props must be skipped when loading the old format) -scl
  			  (!fIsOldFileVer || ((lpPropList[i].pPropInfo->fl & fpPF_fNewSaveData) != fpPF_fNewSaveData))) &&
                ((fIsOldFileVer || 
  			   (lpPropList[i].pPropInfo->fl & fpPF_fLoadDataOnly) != fpPF_fLoadDataOnly))))
      {
//      lDefValue = 0;
//      if (fIsLoading)
//        if (lpPropList[i].pPropInfo->fl & fpPF_fDefVal)
//          lDefValue = lpPropList[i].pPropInfo->dataDefault;
  
        fIgnoreProp = FALSE;
        iCnvtFlags = FPCNVT_FLAG_NONE;
  
        switch (lpPropList[i].pPropInfo->fl & fpPF_datatype)
        {
           case DT_HSZ:
           case fpDT_HLSTR:
              vtType = VT_BSTR; 
              break;
  
           case DT_ENUM:
              vtType = VT_I2;              // Read as VT_I2
              iCnvtFlags = FPCNVT_FLAG_ENUM; // Write as VT_I4.
              break;
  
           case DT_SHORT:
              vtType = VT_I2;
              break;
  
           case DT_COLOR:
           case DT_LONG:
           case DT_XPOS:
           case DT_XSIZE:
           case DT_YPOS:
           case DT_YSIZE:
              vtType = VT_I4;
              break;
  
           case DT_BOOL:
              vtType = VT_BOOL;  // or VT_BOOL ?
              break;
  
           case DT_PICTURE:
              vtType = VT_UNKNOWN;
              break;
  
           case DT_REAL:
              vtType = VT_R4;  // or VT_BOOL ?
              break;
  
           default:
             fIgnoreProp = TRUE;
             break; 
           }
  
           // Process prop-type set by switch. The following function
           // read the prop value from the source stream and writes
           // it to one of the destination stream (if the prop is 
           // still supported in this version).
           if (!fIgnoreProp)
           {
             hr = ReadAndConvertProp(
               lpPropList[i].pPropInfo->npszName, vtType, iCnvtFlags,
               &var, &readPms);
             if (hr != S_OK)
               break; // end for-loop
           }
        }
  
    } // for loop
  } // if fResult

  // If specified, read any additional properties not found in
  // the "pinfo" list.  These props are specified in the list:
  // "lpAddlPropList".
  if (lpAddlPropList && hr == S_OK)
  {
    LPFPATL_CNVT_PROP tmpEntry = lpAddlPropList;
    while (tmpEntry->tstrPropName && hr == S_OK)
    {
      hr = ReadAndConvertProp(
        tmpEntry->tstrPropName, tmpEntry->vtType, tmpEntry->iCnvtFlags,
        &var, &readPms);
      if (hr == S_OK)
        ++tmpEntry;  // next property in list
    }
  }

  // If we have a stream with "old props", copy them to dest stream and
  // free the "old props" stream.
  if (readPms.pstmOldPropDest)
  {
    if (hr == S_OK)
      hr = CopyOldPropStream(pfpAtlCtl, pstmDest, readPms.pstmOldPropDest);

    // Regardless of "hr" value, release "old prop" stream.
    readPms.pstmOldPropDest->Release();
  }

  // if successful, write 0xFFFF as dispid for "terminating" dispid.
  if (hr == S_OK)
  {
    SHORT sIndex = FPATL_STM_NULLPROP;
    hr = pstmDest->Write(&sIndex, sizeof(SHORT), NULL); // write index
  }

  // free prop list created by BuildOldPropList()
  if (readPms.pOldPropList)
    FreeOldPropList(readPms.pOldPropList, readPms.iOldPropCt);

  // Free prop list created by BuildPropNameList()
  if (readPms.pPropList)
    FreePropNameList(readPms.pPropList, readPms.iPropCt);

  return fResult;
} // fpAtlConvertMfcStream()



/***************************************************************************
 *
 * FUNCTION:  CopyOldPropStream()
 *
 * DESCRIPTION:
 *
 *   Copies the properties from one stream to another.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
static HRESULT CopyOldPropStream(CfpAtlBase *pfpAtlCtl, 
  LPSTREAM pstmDest, LPSTREAM pstmOldProps)
{
  HRESULT hr;
  SHORT sIndex = FPATL_STM_NULLPROP;

  // Write 0xFFFF as dispid for "terminating" dispid.
  hr = pstmOldProps->Write(&sIndex, sizeof(SHORT), NULL); // write index
  if (hr == S_OK)
  {
    // Set stream position to the beginning
    LARGE_INTEGER li;
    LISet32(li, 0);
    hr = pstmOldProps->Seek(li, STREAM_SEEK_SET, NULL);
  }

  // copy props from pstmOldProps to pstmDest
  if (hr == S_OK)
    hr = pfpAtlCtl->fpAtlCopyPropStream(pstmDest, pstmOldProps);

  return hr;
}


/***************************************************************************
 *
 * FUNCTION:  FreePropNameList()
 *
 * DESCRIPTION:
 *
 *   Frees the list.  First, it frees each prop name, then it frees
 *   the list itself.
 *
 * PARAMETERS:
 *
 * RETURNS:  void
 *
 **************************************************************************/
static void FreePropNameList(LPPROPNAME_ID_PAIR pList, int iCt)
{
  // free tstr's
  for (int i = 0; i < iCt; i++)
    if (pList[i].propName)
      fpSysFreeTString(pList[i].propName);
  // free list
  free(pList);
}

/***************************************************************************
 *
 * FUNCTION:  FreeOldPropList()
 *
 * DESCRIPTION:
 *
 *   Frees the list; in this case, only the list itself needs to be freed.
 *
 * PARAMETERS:
 *
 * RETURNS:  void
 *
 **************************************************************************/
static void FreeOldPropList(A_LPFPATL_OLDPROP pList, int iCt)
{
  // free list
  free(pList);
}



/***************************************************************************
 *
 * FUNCTION:  BuildPropNameList()
 *
 * DESCRIPTION:
 *
 *   Builds a list of properties from the "persistent" property map
 *   (defined with "BEGIN_PROPERTY_MAP()") along with the dispid's.
 *   The prop names are converted from OLESTR's (WCHAR's) to LPTSTR's.
 *   This list is then sorted by property name for quick lookups.
 *
 * PARAMETERS:
 *
 * RETURNS:  TRUE is successful
 *
 **************************************************************************/
BOOL BuildPropNameList(ATL_PROPMAP_ENTRY *pMap,
  LPPROPNAME_ID_PAIR *ppRetList, int *pRetCt)
{
  int iAllocCt = 200;
  int iCt = 0;
  LPPROPNAME_ID_PAIR retList = 
    (LPPROPNAME_ID_PAIR)malloc(iAllocCt * sizeof(PROPNAME_ID_PAIR));

  _ASSERT(retList);  // List could not be allocated!
  if (!retList)
    return FALSE;

  for(int i = 0; pMap[i].pclsidPropPage != NULL; i++)
  {
	if (pMap[i].szDesc == NULL)
	  continue;

    // if current list is full, increase size
    if (iAllocCt == iCt)
    {
      LPPROPNAME_ID_PAIR pSaveOld = retList;
      iAllocCt += 50;
      retList = (LPPROPNAME_ID_PAIR)realloc(retList, 
        iAllocCt * sizeof(PROPNAME_ID_PAIR));
      if (!retList)
      {
        _ASSERT(0); // relloc failed!
        // free tstr's and the memory block
        FreePropNameList(pSaveOld, iCt);
        return FALSE;
      }  
    }  

    // Put propName and index into structure.  
    retList[iCt].propName = fpSysAllocTStringFromBSTR((LPVOID)pMap[i].szDesc);
    retList[iCt].dispid = pMap[i].dispid;
    ++iCt;
  } // for loop

  // Sort list by propName and return list, count and TRUE.
  qsort( retList, iCt, sizeof(PROPNAME_ID_PAIR), ComparePropNames);
  *ppRetList = retList;
  *pRetCt = iCt;

  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  BuildOldPropList()
 *
 * DESCRIPTION:
 *
 *   Builds the list of properties from the list of "Old props".
 *   This list is sorted by property name for quick lookups.
 *
 * PARAMETERS:
 *
 * RETURNS:  TRUE if successful.
 *
 **************************************************************************/
BOOL BuildOldPropList(LPFPATL_OLDPROP pOldProps,
  A_LPFPATL_OLDPROP *ppRetList, int *pRetCt)
{
  int iAllocCt = 50;
  int iCt = 0;
  A_LPFPATL_OLDPROP retList = 
    (A_LPFPATL_OLDPROP)malloc(iAllocCt * sizeof(LPFPATL_OLDPROP));

  _ASSERT(retList);  // List could not be allocated!
  if (!retList)
    return FALSE;

  for(int i = 0; pOldProps[i].tstrPropName != NULL; i++)
  {
    // if current list is full, increase size
    if (iAllocCt == iCt)
    {
      A_LPFPATL_OLDPROP pSaveOld = retList;
      iAllocCt += 20;
      retList = (A_LPFPATL_OLDPROP)realloc(retList, 
        iAllocCt * sizeof(LPFPATL_OLDPROP));
      if (!retList)
      {
        _ASSERT(0); // relloc failed!
        // free the memory block
        FreeOldPropList(pSaveOld, iCt);
        return FALSE;
      }  
    }  

    // Put ptr to structure in list.  
    retList[iCt] = &(pOldProps[i]);
    ++iCt;
  } // for loop

  // Sort list by propName and return list, count and TRUE.
  qsort( retList, iCt, sizeof(LPFPATL_OLDPROP), CompareOldPropNames);
  *ppRetList = retList;
  *pRetCt = iCt;

  return TRUE;
}


/***************************************************************************
 *
 * FUNCTION:  ComparePropNames()
 *
 * DESCRIPTION:
 *
 *   Compares two PROPNAME_ID_PAIR structures (actually their prop names).
 *
 * PARAMETERS:
 *
 * RETURNS:  results of lstrcmpi()
 *
 **************************************************************************/
int __cdecl ComparePropNames(const void *elem1, const void *elem2)
{
  LPPROPNAME_ID_PAIR p1 = (LPPROPNAME_ID_PAIR)elem1;
  LPPROPNAME_ID_PAIR p2 = (LPPROPNAME_ID_PAIR)elem2;

  return (lstrcmpi(p1->propName, p2->propName));
}

/***************************************************************************
 *
 * FUNCTION:  CompareOldPropNames()
 *
 * DESCRIPTION:
 *
 *   Compares two LPFPATL_OLDPROP structures (actually their prop names).
 *
 * PARAMETERS:
 *
 * RETURNS:  results of lstrcmpi()
 *
 **************************************************************************/
int __cdecl CompareOldPropNames(const void *elem1, const void *elem2)
{
  LPFPATL_OLDPROP p1 = *(LPFPATL_OLDPROP *)elem1;
  LPFPATL_OLDPROP p2 = *(LPFPATL_OLDPROP *)elem2;

  return (lstrcmpi(p1->tstrPropName, p2->tstrPropName));
}

/***************************************************************************
 *
 * FUNCTION:  LookupPropName()
 *
 * DESCRIPTION:
 *
 *   Finds the prop name in the list and returns the dispid for that
 *   prop.  This looks in the passed list (PROPNAME_ID_PAIR).
 *
 * PARAMETERS:
 *
 * RETURNS:  TRUE if successful (prop name found)
 *
 **************************************************************************/
static BOOL LookupPropName(LPPROPNAME_ID_PAIR pPropList, int iPropCt,
  LPCTSTR tstrPropName, SHORT *psRetDispid)
{
  //void *bsearch(const void *key, const void *base, size_t num, size_t width, int ( __cdecl *compare ) ( const void *elem1, const void *elem2 ) );
  PROPNAME_ID_PAIR propTmp = {(LPTSTR)tstrPropName, 0};
  LPPROPNAME_ID_PAIR pPropFound = (LPPROPNAME_ID_PAIR)bsearch(&propTmp, 
    pPropList, iPropCt, sizeof(PROPNAME_ID_PAIR), ComparePropNames);

  if (!pPropFound)
    return FALSE;

  *psRetDispid = (SHORT)pPropFound->dispid;
  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  LookupOldPropName()
 *
 * DESCRIPTION:
 *
 *   Finds the prop name in the "old props" list and returns the dispid 
 *   for that prop.  This looks in the passed list (FPATL_OLDPROP).
 *
 * PARAMETERS:
 *
 * RETURNS:  TRUE if successful (prop name found)
 *
 **************************************************************************/
static BOOL LookupOldPropName(A_LPFPATL_OLDPROP pPropList, 
  int iPropCt, LPCTSTR tstrPropName, SHORT *psRetDispid)
{
  //void *bsearch(const void *key, const void *base, size_t num, size_t width, int ( __cdecl *compare ) ( const void *elem1, const void *elem2 ) );
  FPATL_OLDPROP propTmp = {(LPCTSTR)tstrPropName, 0, 0};
  LPFPATL_OLDPROP ppropPtr = &propTmp;
  LPFPATL_OLDPROP *pPropFound = (LPFPATL_OLDPROP *)bsearch(&ppropPtr, 
    pPropList, iPropCt, sizeof(LPFPATL_OLDPROP), CompareOldPropNames);

  if (!pPropFound)
    return FALSE;

  *psRetDispid = (SHORT)((*pPropFound)->dispid);
  return TRUE;
}


/***************************************************************************
 *
 * FUNCTION:  ReadFromStreamIntoVariant()
 *
 * DESCRIPTION:
 *
 *   Read the prop value from the stream, given the prop type.  The
 *   prop value is returned in the passed variant.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
static HRESULT ReadFromStreamIntoVariant(LPSTREAM pStream, CComVariant *pvar,
  VARTYPE vtRead)
{
	_ASSERTE(pStream != NULL);
	HRESULT hr;
	int cbRead = 0;

    // If variant is not empty, empty the current value.
    if (pvar->vt != VT_EMPTY && pvar->vt != VT_NULL)
      pvar->Clear();

    // Based on the type... 
	switch (vtRead)
	{
	case VT_UNKNOWN:  // If this type (usually Font or Picture), load
	case VT_DISPATCH: // the object into variant.
		{
            BYTE bFlag;
			pvar->punkVal = NULL;
            pvar->vt = vtRead;
            // read first byte, if 0xFF, then skip the loading
            hr = pStream->Read(&bFlag, sizeof(BYTE), NULL);
            if (hr != S_OK || bFlag == 0xFF)
              return hr;

			hr = OleLoadFromStream(pStream, 
				(vtRead == VT_UNKNOWN) ? IID_IUnknown : IID_IDispatch, 
				(void**)&pvar->punkVal);
			if (hr == REGDB_E_CLASSNOTREG)
				hr = S_OK;
			return S_OK;   // no further processing.
		}
	case VT_BOOL:
	case VT_UI1:
	case VT_I1:
		cbRead = sizeof(BYTE);     // set read size
		break;
	case VT_I2:
	case VT_UI2:
		cbRead = sizeof(SHORT);    // set read size
		break;
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		cbRead = sizeof(long);     // set read size
		break;
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		cbRead = sizeof(double);   // set read size
		break;
	default:
		break;                     // default, don't set any read size
	}
	if (cbRead != 0)               // If size was set, read those bytes.
    {
	  hr = pStream->Read((void*) &pvar->bVal, cbRead, NULL);

      // if successful, set the vartype (->vt)...
      if (hr == S_OK)
      {
        pvar->vt = vtRead;
        // if type is VT_BOOL, then convert 1 byte to a long
        if (vtRead == VT_BOOL)
          pvar->lVal = (pvar->bVal ? TRUE : FALSE);
      }
      return hr;  // Return here.
    }

    // At this point, assume VT_BSTR. Read BSTR as MFC did in old OCX.
	UINT nNewLen;
    hr = ReadStringLength(pStream, &nNewLen); // First, read string len.
    if (hr != S_OK)
      return hr;

    // If len is -1, then this indicates that the string is a UNICODE
    // string...
	if (nNewLen == (UINT)-1)
	{
      // Read len of UNICODE string
      hr = ReadStringLength(pStream, &nNewLen);
	  _ASSERT(hr == S_OK);
      if (hr != S_OK)
        return hr;

      // Read UNICODE string
	  pvar->vt = VT_BSTR;
      pvar->bstrVal = SysAllocStringLen(NULL, nNewLen);
	  hr = pStream->Read(pvar->bstrVal, nNewLen * sizeof(OLECHAR), NULL);
      if (hr != S_OK)
        return hr;
	}
    else // not UNICODE string
    {
      BSTR bstr = NULL;
      // Create buffer of "len" size.
      LPSTR lpsz = (LPSTR)fpSysAllocTStringByteLen(NULL, nNewLen);
      if (!lpsz)
        return E_OUTOFMEMORY;

      // Read string into allocated buffer
	  hr = pStream->Read(lpsz, nNewLen, NULL);
      if (hr != S_OK)
        return hr;

      *pvar = (LPCSTR)lpsz;  // assign value to variant
      fpSysFreeTString((LPTSTR)lpsz); // free allocated buffer
    }

    return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  ReadStockProps()
 *
 * DESCRIPTION:
 *
 *   This reads the MFC "Stock properties" which may have been saved
 *   by MFC automatically.
 *
 * PARAMETERS:
 *
 * RETURNS:  TRUE if successful
 *
 **************************************************************************/
static BOOL ReadStockProps(LPREADPROP_PARAMS pReadPms)
{
  CComVariant var;
  DWORD dwPersistMask = 0;
  ULONG cbRead;

  // Read the DWORD which indicates which stock props were saved.
  // PX_ULong(pPX, _T("_StockProps"), dwPersistMask);
  HRESULT hr = pReadPms->pstmSrc->Read(&dwPersistMask, sizeof(DWORD), &cbRead);
  _ASSERT(hr == S_OK);
  if (hr != S_OK)
    return FALSE;

  // For each property:
  //
  //   1. Read prop value from source stream.
  //   2. Lookup prop dispid for that property
  //   3. Write prop dispid and prop value to dest stream

  if (dwPersistMask & STOCKPROP_CAPTION)
  {
    // PX_String(pPX, _T("Caption"), strText, _T(""));
    hr = ReadAndConvertProp(_T("Caption"), VT_BSTR, FPCNVT_FLAG_NONE, 
      &var, pReadPms);
    if (hr != S_OK)
      return FALSE;
  }
  if (dwPersistMask & STOCKPROP_TEXT)
  {
  // PX_String(pPX, _T("Text"), strText, _T(""));
    hr = ReadAndConvertProp(_T("Text"), VT_BSTR, FPCNVT_FLAG_NONE,
      &var, pReadPms);
    if (hr != S_OK)
      return FALSE;
  }

  if (dwPersistMask & STOCKPROP_FORECOLOR)
  {
    // PX_Color(pPX, _T("ForeColor"), m_clrForeColor, AmbientForeColor());
      hr = ReadAndConvertProp(_T("ForeColor"), VT_I4, FPCNVT_FLAG_NONE,
        &var, pReadPms);
      if (hr != S_OK)
        return FALSE;
  }

  if (dwPersistMask & STOCKPROP_BACKCOLOR)
  {
    // PX_Color(pPX, _T("BackColor"), m_clrBackColor, AmbientBackColor());
      hr = ReadAndConvertProp(_T("BackColor"), VT_I4, FPCNVT_FLAG_NONE,
        &var, pReadPms);
      if (hr != S_OK)
        return FALSE;
  }

  if (dwPersistMask & STOCKPROP_FONT)
  {
    // PX_Font(pPX, _T("Font"), m_font, NULL, pFontDispAmbient);
      hr = ReadAndConvertProp(_T("Font"), VT_UNKNOWN, FPCNVT_FLAG_NONE,
        &var, pReadPms);
      if (hr != S_OK)
        return FALSE;
  }

  if (dwPersistMask & STOCKPROP_BORDERSTYLE)
  {
    // PX_Short(pPX, _T("BorderStyle"), m_sBorderStyle, 0);
      hr = ReadAndConvertProp(_T("BorderStyle"), VT_I2, FPCNVT_FLAG_ENUM,
       &var, pReadPms);
      if (hr != S_OK)
        return FALSE;
  }

  if (dwPersistMask & STOCKPROP_ENABLED)
  {
    // PX_Bool(pPX, _T("Enabled"), m_bEnabled, TRUE);
      hr = ReadAndConvertProp(_T("Enabled"), VT_BOOL, FPCNVT_FLAG_NONE, 
        &var, pReadPms);
      if (hr != S_OK)
        return FALSE;
  }

  if (dwPersistMask & STOCKPROP_APPEARANCE)
  {
    // PX_Short(pPX, _T("Appearance"), m_sAppearance, 0);
      hr = ReadAndConvertProp(_T("Appearance"), VT_I2, FPCNVT_FLAG_NONE, 
        &var, pReadPms);
      if (hr != S_OK)
        return FALSE;
  }

  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CreateStream()
 *
 * DESCRIPTION:
 *
 *   Creates (or resets) a stream.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
static HRESULT CreateStream(LPSTREAM *ppstmRet)
{
  LPSTREAM pstmTmp = *ppstmRet;
  HRESULT  hr = S_FALSE;

  // If we have a stream, reset it.
  if (pstmTmp)
  {
    LARGE_INTEGER li;

    LISet32(li, 0);
    hr = pstmTmp->Seek(li, STREAM_SEEK_SET, NULL);
    _ASSERT(hr == S_OK);
    if (hr != S_OK)
    {
      pstmTmp->Release();
      pstmTmp = NULL;
    }
  }

  // if no stream, create one.
  if (!pstmTmp)
  {
    hr = CreateStreamOnHGlobal(NULL, TRUE, ppstmRet);
    _ASSERT(hr == S_OK);
    if (hr != S_OK)
      return E_OUTOFMEMORY;
    pstmTmp = *ppstmRet;
  }

  return hr;
}



/***************************************************************************
 *
 * FUNCTION:  ReadAndConvertProp()
 *
 * DESCRIPTION:
 *
 *   Given a prop's name, type and "flags", read the prop value from
 *   the source stream and write it to one of the two destination streams.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
static HRESULT ReadAndConvertProp(LPCTSTR tstrPropName, VARTYPE vtType,
  int iConvertFlags, CComVariant *pvar, 
  LPREADPROP_PARAMS pReadPms)
{
  // 1. Read prop value from source stream.
  // 2. Lookup prop dispid for that property
  // 3. Write prop dispid and prop value to dest stream
  HRESULT hr = S_FALSE;

  // 1. Read prop value from source stream.
  if (ReadFromStreamIntoVariant(pReadPms->pstmSrc, pvar, vtType) == S_OK)
  {
    // 2. Lookup prop dispid for that property
    LPSTREAM pstmDest = pReadPms->pstmDest; // default destination
    SHORT sdispid;
    BOOL  fResult = LookupPropName(pReadPms->pPropList, 
      pReadPms->iPropCt, tstrPropName, &sdispid);
    hr = S_OK;

    // if not found in "Persist" prop list, try "Old props" list
    if (!fResult)
    {
      fResult = LookupOldPropName(pReadPms->pOldPropList, 
        pReadPms->iOldPropCt, tstrPropName, &sdispid);
      if (fResult)
        pstmDest = pReadPms->pstmOldPropDest; // use as destination stream
    }

    _ASSERT(fResult);  // Not serious but prop does not persist
                       // in ATL version!
    if (fResult)
    {
      // If any conversion flags specified, then process here.
      if (iConvertFlags & FPCNVT_FLAG_ENUM)
      {
        // Convert "enum" from 2-byte value to 4-byte value.
        pvar->lVal = (long)pvar->iVal;
        pvar->vt = VT_I4;
      }

	  // 3. Write prop dispid and prop value to dest stream
	  hr = pstmDest->Write(&sdispid, sizeof(SHORT), NULL);
	  if (FAILED(hr))
		return hr;
      hr = pvar->WriteToStream(pstmDest);
    }
  }

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  ReadStringLength()
 *
 * DESCRIPTION:
 *
 *   Reads the string length from the stream.  Returns string length 
 *   or -1 if UNICODE string is found in the archive.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
static HRESULT ReadStringLength(LPSTREAM pstm, UINT *puiLen)
{
  DWORD nNewLen;
  WORD wLen;
  BYTE bLen;

  // attempt BYTE length first
  HRESULT hr = pstm->Read(&bLen, sizeof(BYTE), NULL);

  *puiLen = 0;

  if (hr != S_OK)
    return hr;

  // If 1st byte is not 0xFF, then this is the length, return. 
  if (bLen < 0xff)
  {
    *puiLen = (UINT)bLen;
    return S_OK;
  }

  // At this point, read the next WORD.
  // attempt WORD length
  hr = pstm->Read(&wLen, sizeof(WORD), NULL);
  if (hr != S_OK)
    return hr;

  // If this WORD is 0xFFFE, then this is a UNICODE string, return -1 as len. 
  if (wLen == 0xfffe)
  {
    // UNICODE string prefix (length will follow)
    *puiLen = (UINT)-1;
    return S_OK;
  }
  // Else if, this WORD is 0xFFFF, then next DWORD is the length.
  else if (wLen == 0xffff)
  {
    // read DWORD of length
    hr = pstm->Read(&nNewLen, sizeof(DWORD), NULL);
    if (hr == S_OK)
      *puiLen = (UINT)nNewLen;
  }
  // Else, this WORD is the length, return it.
  else
    *puiLen = (UINT)wLen;

  return S_OK;  // return SUCCESSFUL.
}
