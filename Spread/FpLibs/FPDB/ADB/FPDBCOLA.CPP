/*************************************************************************\
* FPDBCOLA.CPP                                                             *
*                                                                         *
* Copyright (C) 1991-1998 - FarPoint Technologies, Inc.                   *
* All Rights Reserved.                                                    *
*                                                                         *
* No part of this source code may be copied, modified or reproduced       *
* in any form without retaining the above copyright notice.  This         *
* source code, or source code derived from it, may not be redistributed   *
* without express written permission of FarPoint Technologies.            *
*                                                                         *
\*************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#ifndef NO_MFC
#include <afxctl.h>         // MFC support for OLE Custom Controls
#else  //NO_MFC
#define FP_EXCLUDE_ATL
#include "stdafx.h"
//ATL #include "fpatl.h"
#endif

#include "vbinterf.h"                                                                                                                          

#include "fpconvrt.h"
#include "fptstr.h"                             

#include "fpdbutil.h"		  
#include "fpdbcola.h"
#include "fpoledb.h"		        // get defines for error codes

#include <limits.h>
#include <malloc.h>

#ifndef WIN32  // define for WIN16
#define SysStringByteLen(bstr) SysStringLen(bstr)
#endif

static USHORT fpDBTypeToVarType(USHORT usDBType);
static HRESULT fpdbConvertBuffer(LPVOID pvData, ULONG ulInputByteLen, 
  ULONG *pulRetByteLen, UINT uSrcCharSize, UINT uDestCharSize);

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::CfpDBColumn()
 *
 * DESCRIPTION:
 *
 *   Constructor for the class.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBColumn::CfpDBColumn()
{
  // Initialize members
  Init();
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::~CfpDBColumn()
 *
 * DESCRIPTION:
 *
 *   Destructor for the class.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBColumn::~CfpDBColumn()
{
  // Clear members. FALSE indicates: "don't initialize members"
  ClearOwnMembers(FALSE);  
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::ClearOwnMembers()
 *
 * DESCRIPTION:
 *
 *   Clears its own member variables which have allocated data.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBColumn::ClearOwnMembers(BOOL fInit /* = TRUE*/ )
{
  // Clear tstrName (if set)
  if (tstrName)
    fpSysFreeTString(tstrName);

  // Initialize members if specified (default)
  if (fInit)
    Init();
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::Init()
 *
 * DESCRIPTION:
 *
 *   Initializes the member variables.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBColumn::Init()
{
  // Initialize member variables
  tstrName = NULL;
  uMetaFlags = 0;
  m_fNewRead = FPDB_COL_NOFETCH;
  m_pSeqStmGet = NULL;
  m_hReadAccessor  = NULL;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::Clear()
 *
 * DESCRIPTION:
 *
 *   Clear its self, including its parent class.   
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBColumn::Clear(void)
{
  // Clear COLUMNINFO portion of "this"
  ((COLUMNINFO*)this)->Clear();

  // Clear our own members
  ClearOwnMembers();
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::operator=(const CfpDBColumn&)
 *
 * DESCRIPTION:
 *
 *   Used to make this object the same as the one passed in.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBColumn& CfpDBColumn::operator=(const CfpDBColumn& other)
{
  if (this != &other)
  {
    // Copy COLUMNINFO portion of "other"
    *this = *((COLUMNINFO*)&other);
    
    // Copy members not set by the above assignment
    m_obStatus     = other.m_obStatus;    
    m_obLength     = other.m_obLength;    
    m_obValue      = other.m_obValue;     
    m_wTypeBind    = other.m_wTypeBind;   
    m_fBookmarkCol = other.m_fBookmarkCol;
    m_fNewRead     = other.m_fNewRead;
    m_cbReadPos    = other.m_cbReadPos;
    m_ulStreamLen  = other.m_ulStreamLen;
  }
  return *this;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::operator=(const COLUMNINFO&)
 *
 * DESCRIPTION:
 *
 *   Used to make this object the same as the object passed in, though,
 *   the object passed in is not the same class type.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBColumn& CfpDBColumn::operator=(const COLUMNINFO& other)
{
  if (this != &other)
  {
    // Clear our own members
    ClearOwnMembers();

    // Copy "other" to COLUMNINFO portion of "this"
    *((COLUMNINFO*)this) = other;

    // Make a LPTSTR copy of LPWSTR "pwszName".
    if (pwszName)
      tstrName = fpSysAllocTStringFromBSTR(pwszName);

    // Convert OLEDB flags to VB3 flags
    if (dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH)
      uMetaFlags |= DB_FIXEDFIELD;
    else
      uMetaFlags |= DB_VARIABLEFIELD;

    if (dwFlags & DBCOLUMNFLAGS_ISROWVER)
      uMetaFlags |= DB_AUTOINCRFIELD;

// removed 10/16/07 for 20995 -scl
//    if (!(dwFlags & DBCOLUMNFLAGS_WRITE)) // 20449 -scl
//      uMetaFlags |= DB_AUTOINCRFIELD;

    if (dwFlags & (DBCOLUMNFLAGS_WRITE|DBCOLUMNFLAGS_WRITEUNKNOWN))
      uMetaFlags |= DB_UPDATABLEFIELD;

    if (dwFlags & DBCOLUMNFLAGS_ISNULLABLE)
      uMetaFlags |= UDB_NULLABLE;
  }
  return *this;
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpFillColBindRec()
 *
 * DESCRIPTION:
 *
 *   Fill the fields in the DBBINDING structure for this Column.
 *   Other parameters are for Inline and Out-of-line buffer offsets.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpDBColumn::fpFillColBindRec(DBBINDING *pColBind, ULONG *pulInOffset,
  LPBLOB *pBlobTmp, BOOL fBookmarkCol /* = FALSE*/ )
{
  ULONG remainder;

  // Column ID
  pColBind->iOrdinal = iOrdinal;

  pColBind->dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;

  // DataType
  pColBind->wType = wType;

  // Maximum length of the buffer
  pColBind->cbMaxLen = ulColumnSize;

  // If type DBTYPE_DBTIMESTAMP, get as string
  if (wType == DBTYPE_DBTIMESTAMP)
  {
    // For now, we will read this type as DBTYPE_DATE (see farther down).
//  pColBind->wType = DBTYPE_STR;
//  pColBind->cbMaxLen = 64;
  }
  // If type NUMERIC, look for specific cases.
  else if (wType == DBTYPE_NUMERIC)
  {
    // If scale (# of digits to the right of the decimal pt) is 0 and
    // precision is less than 10 or 5, then use I4 or I2.
    if (bScale == 0)
    {
      if (bPrecision < 5)        // use I2
      {
        pColBind->wType = DBTYPE_I2;
        pColBind->cbMaxLen = 2;
      }
      else if (bPrecision < 10)  // use I4
      {
        pColBind->wType = DBTYPE_I4;
        pColBind->cbMaxLen = 4;
      }
    }
    // Else, bScale is not zero, see if we can use R4 or R8
    else
    {
      if (bPrecision < 7)        // use R4
      {
        pColBind->wType = DBTYPE_R4;
        pColBind->cbMaxLen = 4;
      }
      else if (bPrecision < 16)  // use R8
      {
        pColBind->wType = DBTYPE_R8;
        pColBind->cbMaxLen = 8;
      }
      else if (bPrecision == 19 && bScale == 4) // use CY (Currency)
      {
        pColBind->wType = DBTYPE_CY;
        pColBind->cbMaxLen = 8;
      }
    }
  }

  // returned info flags
  pColBind->obStatus  = *pulInOffset;
  *pulInOffset       += sizeof(DBSTATUS); 

  if (fBookmarkCol)
  {
    // if col size if greater than 4 bytes OR 
	// not fixed length and not of type I4 (and not UI4), get as array.
    if (ulColumnSize > sizeof(ULONG) || 
        (!(dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH) && 
		 wType != DBTYPE_I4 && wType != DBTYPE_UI4))
    {
      pColBind->wType = DBTYPE_VECTOR | DBTYPE_UI1;
      pColBind->cbMaxLen = sizeof(BLOB);
    }
	else
    {
      pColBind->wType = DBTYPE_UI4;
      pColBind->cbMaxLen = sizeof(ULONG);
    }

    pColBind->dwPart  &= ~DBPART_LENGTH; // remove length flag.
    pColBind->obValue  = *pulInOffset;
    *pulInOffset      += pColBind->cbMaxLen;
  }
    // Not bookmark column. Is it "long" (large amount of data)?
  else if (dwFlags & DBCOLUMNFLAGS_ISLONG)
  {
    // alloc/Realloc blob to add room for DBOBJECT
    DBOBJECT *pDbObject;
    ULONG cbSize = (*pBlobTmp ? (*pBlobTmp)->cbSize+sizeof(DBOBJECT) : 
      sizeof(ULONG)+sizeof(DBOBJECT));
    *pBlobTmp = (LPBLOB)(*pBlobTmp ? realloc(*pBlobTmp, cbSize) :
      malloc(cbSize));
    // Return if allocate failed
    if (!(*pBlobTmp))
      return FALSE;
    (*pBlobTmp)->cbSize = cbSize;  // update blob size
    // Setup DBOBJECT
    pDbObject = (DBOBJECT *)((LPBYTE)(*pBlobTmp) + cbSize - sizeof(DBOBJECT));
    pDbObject->dwFlags = STGM_READ;
    pDbObject->iid = IID_ISequentialStream; 
    // Setup "pColBind"
    pColBind->obLength  = *pulInOffset;  // returned len
    *pulInOffset       += sizeof(ULONG);
    pColBind->pObject  = pDbObject;
    pColBind->obValue  = *pulInOffset;  // returned len
    pColBind->cbMaxLen = sizeof(IUnknown *);
    *pulInOffset      += pColBind->cbMaxLen;
    pColBind->wType    = DBTYPE_IUNKNOWN;
    m_cbReadPos        = 0;
    m_ulStreamLen      = (ULONG)-1;  // un-initialized indicator
  }
  else // Else, not bookmark and not "IS_LONG"
  {
    switch (pColBind->wType)
    {
      case DBTYPE_NUMERIC:
      case DBTYPE_VARNUMERIC:
      case DBTYPE_I8:
      case DBTYPE_UI8:
      case DBTYPE_DECIMAL:  // Use STRING for these types
      case DBTYPE_GUID:
		if (pColBind->wType == DBTYPE_VARNUMERIC)
          pColBind->cbMaxLen     = 257;  // max 255, allow sign and null-term
	    else  
          pColBind->cbMaxLen     = 50;
        
		pColBind->wType        = DBTYPE_STR;
        // Fall-Thru

      // if variable-len data, set offset into Inline buffer
      case DBTYPE_STR:    // DBTYPE_CHARS ?
      case DBTYPE_WSTR:   // DBTYPE_WCHARS ?
      case DBTYPE_BYTES:
        pColBind->obLength  = *pulInOffset;  // returned len
        *pulInOffset       += sizeof(ULONG);
        pColBind->cbMaxLen += 1;             // add 1 to len for null-term.
        if (pColBind->wType == DBTYPE_WSTR)  // if WSTR, len = len * 2
          pColBind->cbMaxLen *= sizeof(WCHAR);
        pColBind->obValue   = *pulInOffset;  // returned data
        *pulInOffset       += pColBind->cbMaxLen;
        break;
  
      case DBTYPE_BSTR:
        pColBind->dwPart      &= ~DBPART_LENGTH; // remove length flag.
        pColBind->cbMaxLen     = sizeof(BSTR);   // set max len
        pColBind->obValue      = *pulInOffset;   // returned data
        *pulInOffset          += sizeof(BSTR);
        break;
  
      case DBTYPE_VARIANT:  // Using type DBVARIANT
        pColBind->obLength     = *pulInOffset;   // returned len
        *pulInOffset          += sizeof(ULONG);
        pColBind->cbMaxLen     = sizeof(VARIANT);
        pColBind->obValue      = *pulInOffset;   // DBVARIANT structure
        *pulInOffset          += sizeof(VARIANT);// sizeof(DBVARIANT)
        break;

      case DBTYPE_DBDATE:   // Using type DBTYPE_DATE
      case DBTYPE_DBTIME:   // Using type DBTYPE_DATE
      case DBTYPE_DBTIMESTAMP:
        pColBind->wType        = DBTYPE_DATE;    // Use this type instead.
        pColBind->cbMaxLen     = sizeof(double);
        pColBind->obValue      = *pulInOffset;
        *pulInOffset          += sizeof(double);
        pColBind->dwPart      &= ~DBPART_LENGTH; // remove length flag.
        break;


      default:         // All others are fixed-length
        pColBind->dwPart      &= ~DBPART_LENGTH; // remove length flag.
        pColBind->obValue      = *pulInOffset;
        *pulInOffset          += pColBind->cbMaxLen;
    } // switch
  } // if-else bookmark

  // Save some of the values set (for retrieving the data after the fetch)
  m_obStatus  = pColBind->obStatus;
  m_obLength  = pColBind->obLength;
  m_obValue   = pColBind->obValue;
  m_wTypeBind = pColBind->wType;
  m_dwPart    = pColBind->dwPart;
  m_cbMaxLen  = pColBind->cbMaxLen;

  m_fBookmarkCol = fBookmarkCol;

  m_fNewRead  = FPDB_COL_NOFETCH;

  m_fSet = 0;
  m_pSetData = NULL;
  m_pstmSet = NULL;

  m_wTypeBindSet = pColBind->wType;
  // If type DBTYPE_DBTIMESTAMP, when setting, use DBTYPE_DATE
  if (wType == DBTYPE_DBTIMESTAMP)
    m_wTypeBindSet = DBTYPE_DATE;
  if (wType == DBTYPE_VARIANT)  // if variant, use BSTR's for updating.
    m_wTypeBindSet = DBTYPE_BSTR;

  // Round "*pulInOffset" to an 8-byte boundary.
  remainder = *pulInOffset % 8;
  *pulInOffset += (remainder == 0 ? 0 : 8 - remainder);

  return TRUE;
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpFillSetDataBinding()
 *
 * DESCRIPTION:
 *
 *   Fill the fields in the DBBINDING structure for this Column.
 *   Other parameters are for Inline and Out-of-line buffer offsets.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpDBColumn::fpFillSetDataBinding(DBBINDING *pColBind, 
  LPBLOB *pBlobTmp)
{
  // Column ID
  pColBind->iOrdinal = iOrdinal;

  pColBind->obStatus = m_obStatus;
  pColBind->obLength = m_obLength;
  pColBind->obValue  = m_obValue;
  pColBind->wType    = m_wTypeBindSet;
  pColBind->dwPart   = m_dwPart;
  pColBind->cbMaxLen = m_cbMaxLen;

  // Remove length flag for all except the following
  switch (pColBind->wType)
  {
    case DBTYPE_STR:
    case DBTYPE_WSTR:
    case DBTYPE_BYTES:
      pColBind->dwPart |= DBPART_LENGTH;  // required for these
      break;
    default:
      pColBind->dwPart &= ~DBPART_LENGTH; 
  }

  // Is it "long" (large amount of data)?
  if (dwFlags & DBCOLUMNFLAGS_ISLONG)
  {
    // alloc/Realloc blob to add room for DBOBJECT
    DBOBJECT *pDbObject;
    ULONG cbSize = (*pBlobTmp ? (*pBlobTmp)->cbSize+sizeof(DBOBJECT) : 
      sizeof(ULONG)+sizeof(DBOBJECT));
    *pBlobTmp = (LPBLOB)(*pBlobTmp ? realloc(*pBlobTmp, cbSize) :
      malloc(cbSize));
    // Return if allocate failed
    if (!(*pBlobTmp))
      return FALSE;
    (*pBlobTmp)->cbSize = cbSize;  // update blob size
    // Setup DBOBJECT
    pDbObject = (DBOBJECT *)((LPBYTE)(*pBlobTmp) + cbSize - sizeof(DBOBJECT));
    pDbObject->dwFlags = STGM_WRITE;
    pDbObject->iid = IID_ISequentialStream; 
    pColBind->pObject  = pDbObject;
  }

  return TRUE;
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpFreeFetchData()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
void CfpDBColumn::fpFreeFetchData(LPSTR pszData, BOOL fZeroMem /* = TRUE*/)
{
  DBSTATUS dwInfo;
  LPSTR psz;

  // if already freed, return
  if (m_fNewRead <= FPDB_COL_FETCHFREED)
    return;

  dwInfo = DBSTATUS_S_OK;
  // Retrieve INFO from the Fetch buffer
  if (m_dwPart & DBPART_STATUS)
    dwInfo = *(DBSTATUS FAR *)(pszData + m_obStatus);

  // if the field value is null, just return
  if (dwInfo == DBSTATUS_S_ISNULL)
    return;

  psz = pszData + m_obValue;

  // If type is something that needs freeing, do it.
  if (m_wTypeBind & DBTYPE_VECTOR)          // type: BLOB
  {
    BLOB *pBlob = (BLOB *)psz;

    // Free callee allocated buffer
    CoTaskMemFree(pBlob->pBlobData);
  }
  else if (m_wTypeBind == DBTYPE_BSTR)      // type: BSTR
  {
    BSTR *bstr = (BSTR *)psz;
    if (bstr && *bstr)
      SysFreeString(*bstr);
  }
  else if (m_wTypeBind == DBTYPE_VARIANT)   // type: VARIANT
  {
    VARIANT *lpvar = (VARIANT *)psz;
    if (lpvar && lpvar->vt != VT_EMPTY)
      VariantClear(lpvar);
  }
  else if (m_wTypeBind == DBTYPE_IUNKNOWN)  // type: IUNKNOWN
  {
    m_cbReadPos = 0;                        // reset read position
    m_ulStreamLen = (ULONG)-1;
    if (m_pSeqStmGet)                       // If stream, release
    {
      ULONG ul = m_pSeqStmGet->Release();
      m_pSeqStmGet = NULL;
    }
  }

  if (fZeroMem)
    ZeroMemory(psz, m_cbMaxLen);      // clear memory
  m_fNewRead = FPDB_COL_FETCHFREED;    // set flag

  return;
} //FreeFetchData()


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpPostFetchSetup()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
void CfpDBColumn::fpPostFetchSetup( LPSTR pszData)
{
  // set flag
  m_fNewRead = FPDB_COL_POSTFETCH;

  // If type is IUNKNOWN (Stream), then initialize the stream ptr.
  if (m_wTypeBind == DBTYPE_IUNKNOWN)  // type: IUNKNOWN
  {
    // Retrieve INFO from the Fetch buffer, if value not null, proceed.
    if (m_dwPart & DBPART_STATUS)
    {
      DWORD dwInfo = *(DBSTATUS FAR *)(pszData + m_obStatus);
      // If not null, save ptr to seq stream used for gets.
      if (dwInfo != DBSTATUS_S_ISNULL && dwInfo != 0xBAADF00D)
        m_pSeqStmGet = *(ISequentialStream **)(pszData + m_obValue);
    }
  }
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpGetBookmark()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
HRESULT CfpDBColumn::fpGetBookmark(LPSTR pszData, CBookmark *pBmk)
{
  HRESULT hr = S_OK;
  
  pszData += m_obValue;  // Add data offset to buffer

  if (!m_fBookmarkCol)
    return S_FALSE;

  if (m_wTypeBind == DBTYPE_UI4)
  {
    // Bookmark data is formatted as a ULONG
	hr = pBmk->Set(sizeof(ULONG), (BYTE*)pszData);
  }
  else
  {
    BLOB *pBlob = (BLOB *)pszData;

    // Bookmark data is formatted as a byte vector
    hr = pBmk->Set(pBlob->cbSize, (BYTE*)pBlob->pBlobData);

    // Free callee allocated buffer
    // CoTaskMemFree(pBlob->pBlobData);
  }

  return hr;
}





/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpGetDataValue()
 *
 * DESCRIPTION:
 *
 *   This function copies the data for the field from the "Fetch Buffer".
 *
 * PARAMETERS:
 *
 *   pszFetchBuff .. Ptr to fetch buffer.
 *   usVBDataType ... Format for the returned data (DT_LONG, DT_HSZ, etc).
 *   ulChunkOffset .. For types DBTYPE_CHAR and VT_BLOB, gets chunk at offset.
 *   ulChunkSize .... Sizeof chunk to get.
 *   lplData  ....... Returned data. Either the value or ptr to string. 
 *                    NOTE:  If NULL is passed, then only the length is
 *                    returned.
 *   lpuFlags ....... Returned flags.
 *   lpCursor ....... Ptr to Cursor.  Used when retrieving BLOB values.
 *
 * RETURNS: (ULONG)
 *
 *   0 ............. if no data (or null)
 *   Len of data ... If successful (does not include null terminator)
 *   (ULONG)-1 ..... Error (general)
 *   (ULONG)-2 ..... Error (Fetch buffer needs to be reloaded)
 *
 **************************************************************************/
ULONG CfpDBColumn::fpGetDataValue( LPSTR pszFetchBuff, USHORT usVBDataType,
  ULONG ulChunkOffset, ULONG ulChunkSize,
  LONG FAR *lplData, USHORT FAR *lpuFlags)
{
  DBSTATUS dwInfo = DBSTATUS_S_OK;
  LPSTR pszData = NULL;
  ULONG dwVarLen = (ULONG)-1;
  ULONG ulReturn = (ULONG)-1;
  DBTYPE wDataType;

  if (lplData)
    *lplData = 0;

  // Retrieve INFO from the Fetch buffer
  if (m_dwPart & DBPART_STATUS)
    dwInfo = *(DBSTATUS FAR *)(pszFetchBuff + m_obStatus);

  // if the field value is null, set flag and return
  // 9/3/99 -- if status is unavailable or permission is denied, return a NULL -scl & scp
  if (dwInfo == DBSTATUS_S_ISNULL || dwInfo == DBSTATUS_E_UNAVAILABLE || dwInfo == DBSTATUS_E_PERMISSIONDENIED)
  {
    *lpuFlags = DA_fNull;
    return (ULONG)0;
  }

  // get ptrs to data and its length
  if (m_dwPart & DBPART_LENGTH)
     dwVarLen = *(ULONG FAR *)(pszFetchBuff + m_obLength);
  else
     dwVarLen = m_cbMaxLen;

  pszData = pszFetchBuff + m_obValue;

  wDataType = m_wTypeBind & ~DBTYPE_ARRAY;

  switch (wDataType)
  {
    case DBTYPE_UI1+DBTYPE_VECTOR:  // possible BLOB
    case DBTYPE_BSTR:
    case DBTYPE_STR:
    case DBTYPE_WSTR:
      ulReturn = fpGetStringValue( pszData, usVBDataType, wDataType, 
                   ulChunkOffset, ulChunkSize, lplData, dwVarLen);
      break;

    case DBTYPE_IUNKNOWN:
      ulReturn = fpGetStreamValue(pszData, usVBDataType, dwVarLen, 
                   ulChunkOffset, ulChunkSize, lplData, lpuFlags);
      break;

    case DBTYPE_VARIANT:
      ulReturn = fpGetDBVariantValue(pszData, usVBDataType, wDataType,
                                      dwVarLen, lplData);
      break;


    default:  // Retrieving a non-string value
      ulReturn = fpGetNonStringValue( pszData, usVBDataType, wDataType,
                                      dwVarLen, lplData);
  } //switch (m_ulType)

  return ulReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpGetNonStringValue()
 *
 * DESCRIPTION:
 *
 * RETURNS: (ULONG)
 *
 *   0 ............. if no data (or null)
 *   Len of data ... If successful (does not include null terminator)
 *   (ULONG)-1 ..... Error (general)
 *   (ULONG)-2 ..... Error (Fetch buffer needs to be reloaded)
 *
 **************************************************************************/
ULONG CfpDBColumn::fpGetNonStringValue( LPSTR pszGetData, USHORT usVBDataType,
  WORD wDataType, ULONG dwVarLen, LONG FAR *lplData)
{
  ULONG ulReturn = (ULONG)-1;
  LONG  ulTmp = 0;
  LBLOCK tmpLBlock = NULL;

  // If control is requesting data as type DATE OR type REAL8,
  // get "double" value and return those 8 bytes in an allocated HLSTR.
  if (usVBDataType == DT_FP_DATETIME || usVBDataType == DT_FP_REAL8)
  {
    ulReturn = (ULONG)sizeof(double);
    // If caller wants the data besides the length...
    if (lplData)
    {
      // Retrieve value (as double)
      double dblVal; 
      // If control wants a REAL8 but we don't have one, use float
      if (usVBDataType == DT_FP_REAL8 && wDataType != DBTYPE_R8)
        dblVal = (double)*(float *)pszGetData;
      else // use value as a double
        dblVal = *(double *)pszGetData;
      // Allocate HLSTR memory and copy 8 bytes into it.
      *lplData = (LONG)LBlockAllocLen(NULL, sizeof(double));
      if (*lplData)
        *(double *)*lplData = dblVal;
    }
  }
  else  // else, requested type is not DT_FP_REAL8 or DT_FP_DATETIME
  {
    ulReturn = (ULONG)dwVarLen;
    // If caller wants the data besides the length...
    if (lplData)
    {
      // If requesting data as a REAL (float) and data field type is 
      // REAL8 (double), convert from double to float.
      if (usVBDataType == DT_REAL && wDataType == DBTYPE_R8)
      {
        *(float *)lplData = (float)*(double *)pszGetData;
        ulReturn = (ULONG)sizeof(float);
      }
      // If requesting the data as a string...
      else if ((usVBDataType == DT_HSZ) || (usVBDataType == DT_HLSTR))
      {
        // Convert data to string.
        LPTSTR tstr = (LPTSTR)fpConvertTypeToTSTR((LPVOID)pszGetData, 
                         (USHORT)wDataType);
        if (tstr)
        {
          // Copy string to desired string type.
          if (usVBDataType == DT_HSZ)
            *lplData = (LONG)fpDBCreateHsz( (LPTSTR)tstr);
          else
            *lplData = (LONG)LBlockAllocXStrToXStr(tstr,
                         sizeof(TCHAR), sizeof(TCHAR));
          fpSysFreeTString(tstr);
        } //if tstr
      }
      else
      {
          ulTmp = (ULONG)0;
          _fmemcpy( (LPVOID)&ulTmp, pszGetData, (size_t)dwVarLen);
        *(ULONG FAR *)lplData = ulTmp;
      }
    }
  }

  return ulReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpGetDBVariantValue()
 *
 * DESCRIPTION:
 *
 * RETURNS: (ULONG)
 *
 *   0 ............. if no data (or null)
 *   Len of data ... If successful (does not include null terminator)
 *   (ULONG)-1 ..... Error (general)
 *   (ULONG)-2 ..... Error (Fetch buffer needs to be reloaded)
 *
 **************************************************************************/
ULONG CfpDBColumn::fpGetDBVariantValue( LPSTR pszGetData, USHORT usVBDataType,
  WORD wDataType, ULONG dwVarLen, LONG FAR *lplData)
{
  ULONG ulReturn = (ULONG)-1;
  LONG  ulTmp = 0;
  LBLOCK tmpLBlock = NULL;

  {
    LPTSTR tstr;
    VARIANT *lpvar;

    ulReturn = (ULONG)dwVarLen;
    // If caller wants the data besides the length...
    if (lplData)
    {
      // If not expecting a string, return error
      _ASSERT((usVBDataType == DT_HSZ) || (usVBDataType == DT_HLSTR));
      if ((usVBDataType != DT_HSZ) && (usVBDataType != DT_HLSTR))
        return -1;

      // Get ptr to variant structure.
      lpvar = (VARIANT *)pszGetData;

      // Convert data to string.
      if (lpvar->vt == VT_BSTR)
        tstr = fpSysAllocTStringFromBSTR(lpvar->bstrVal);
      else
        tstr = (LPTSTR)fpConvertTypeToTSTR((LPVOID)&lpvar->lVal, 
                       (USHORT)lpvar->vt);
      if (tstr)
      {
        // Copy string to desired string type.
        if (usVBDataType == DT_HSZ)
          *lplData = (LONG)fpDBCreateHsz( (LPTSTR)tstr);
        else
          *lplData = (LONG)LBlockAllocXStrToXStr(tstr,
                       sizeof(TCHAR), sizeof(TCHAR));
        fpSysFreeTString(tstr);
      } //if tstr
    }
  }

  return ulReturn;
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpGetStreamValue()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: (ULONG)
 *
 *   0 ............. if no data (or null)
 *   Len of data ... If successful (does not include null terminator)
 *   (ULONG)-1 ..... Error (general)
 *   (ULONG)-2 ..... Error (Fetch buffer needs to be reloaded)
 *
 **************************************************************************/
ULONG CfpDBColumn::fpGetStreamValue( LPSTR pszGetData, USHORT usVBDataType,
  ULONG dwVarLen, ULONG ulChunkOffset, ULONG ulChunkSize, LONG FAR *lplData, 
  USHORT FAR *lpuFlags)
{
  ULONG ulReturn = (ULONG)-1;
  HRESULT hr;
  UINT  uSrcCharSize;
  UINT  uDestCharSize;
  ULONG ulAjustedOffset;
  ULONG ulAjustedSize;
  LBLOCK tmpLBlock = NULL;
  ISequentialStream *pSeqStm;
  ULONG cbBytesRead = 0;

  // If the length given is 4 bytes (size of a ptr), then it is possible
  // that the length is invalid.  In that case, call the other function
  // which is not dependent on the given length.
  if (dwVarLen == sizeof(LPUNKNOWN))
    return fpGetStreamValueWithoutLen( pszGetData, usVBDataType,
      dwVarLen, ulChunkOffset, ulChunkSize, lplData, lpuFlags);

  // Set size of "source" chars (either WORD or BYTE chars).
  uSrcCharSize = 
    (ULONG)(wType == DBTYPE_WSTR ? sizeof(WCHAR) : sizeof(BYTE));

  // If only the size of requested (not the actual data), just return
  // the size.
  if (lplData == NULL)
    return ((ULONG)dwVarLen / uSrcCharSize);

  // Get ptr from data.
  pSeqStm = m_pSeqStmGet;

  // If ptr is NULL, set flag that there is no data and return.
  if (!pSeqStm)
  {
    *lpuFlags = DA_fNull;
    return (ULONG)0;
  }

  ulAjustedOffset = ulChunkOffset * uSrcCharSize; // scale the Offset
  // If the current position of the Stream is not the same as the
  // requested offset, return a code indicating such (the stream will
  // get re-read from scratch).
  if (m_cbReadPos != ulAjustedOffset)
    return (ULONG)-2;

  ulAjustedSize = ulChunkSize * uSrcCharSize;     // scale the chunk

  // adjust for offset & size
  if (ulAjustedOffset && (ulAjustedOffset < (ULONG)dwVarLen))
    dwVarLen -= ulAjustedOffset;     // decrease size

  if (ulAjustedSize && (dwVarLen > ulAjustedSize))
    dwVarLen = ulAjustedSize;

  // Determine the destination char size (TCHAR or BYTE)
  uDestCharSize = sizeof(TCHAR);
  if (wType == DBTYPE_BYTES || wType == (DBTYPE_UI1+DBTYPE_VECTOR))
    uDestCharSize = sizeof(BYTE);

  // Allocate buffer of size "dwVarLen"
  if (dwVarLen > 0)
  {
    tmpLBlock = LBlockAllocLen(NULL, (LONG)dwVarLen);
    if (tmpLBlock)
    {
      // Read the data from the Stream
	   hr = pSeqStm->Read((void *)tmpLBlock, dwVarLen, &cbBytesRead);
	   if ((hr == S_OK || hr == DB_S_ENDOFROWSET) && cbBytesRead)
	   {
		  // If read char sizes don't match, convert to proper type (size)
		  if (uSrcCharSize != uDestCharSize)
		  {
		    LBLOCK tmpLBlock2 = LBlockAllocXLenToXLen(tmpLBlock,
			    (UINT)(dwVarLen / uSrcCharSize), uSrcCharSize, uDestCharSize);
		    // Use new block and free old one.  NOTE: if allocation fails,
		    // tmpLBlock2 is null, so old block will be freed, and tmpLBlock
		    // will equal NULL.
		    LBlockFree(tmpLBlock);
		    tmpLBlock = tmpLBlock2;
		    if (!tmpLBlock)  // if null, return memory error
			   return (ULONG)-1;
		  }

		  // We have "some" or all of the data, copy to desired string type.
		  if (usVBDataType == DT_HSZ)
		  {
		    *lplData = (LONG)fpDBCreateHsz((LPTSTR)tmpLBlock);
		    LBlockFree(tmpLBlock);
		  }
		  else   
		   *lplData = (LONG)tmpLBlock;
		  ulReturn = (ULONG)dwVarLen / uSrcCharSize;
	   }
	   else // else, read failed, free memory block
		  LBlockFree(tmpLBlock);
    }
  }

  // increment current position
  m_cbReadPos += cbBytesRead;
  
  return ulReturn;
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpGetStreamValueWithoutLen()
 *
 * DESCRIPTION:
 *
 *   Get stream value without a valid length.  In this case, the length
 *   that is given by the ADO is 4 bytes.  That length of 4 may or
 *   may not be valid.  In some cases, the length refers to the length
 *   of the IUnknown ptr (4 bytes), in other cases, the length may be
 *   valid, there may be only 4 bytes of data.
 *
 *   To determine the length, we will get the data using the Read()
 *   function until that function returns that there is no more data.
 *
 * PARAMETERS:
 *
 * RETURNS: (ULONG)
 *
 *   0 ............. if no data (or null)
 *   Len of data ... If successful (does not include null terminator)
 *   (ULONG)-1 ..... Error (general)
 *   (ULONG)-2 ..... Error (Fetch buffer needs to be reloaded)
 *
 **************************************************************************/
ULONG CfpDBColumn::fpGetStreamValueWithoutLen( LPSTR pszGetData, USHORT usVBDataType,
  ULONG dwVarLen, ULONG ulChunkOffset, ULONG ulChunkSize, LONG FAR *lplData, 
  USHORT FAR *lpuFlags)
{
#define FPDB_STREAM_READ_SIZE  50000

  ULONG ulReturn = (ULONG)-1;
  HRESULT hr;
  UINT  uSrcCharSize;
  UINT  uDestCharSize;
  ULONG ulAjustedOffset;
  ULONG ulAjustedSize;
  LBLOCK tmpLBlock = NULL;
  ISequentialStream *pSeqStm;
  ULONG cbBytesRead = 0;
  ULONG cbBytesToRead;
  ULONG cbCharsRead;
  ULONG cbAllocSize;
  BOOL  fLenOnly = (lplData == NULL);
  BOOL  fGetStreamLen = FALSE;
  BOOL  fFreeLBlock = TRUE;

  // Set size of "source" chars (either WORD or BYTE chars).
  uSrcCharSize = 
    (ULONG)(wType == DBTYPE_WSTR ? sizeof(WCHAR) : sizeof(BYTE));

  // If caller only wants the length and we know it, then return the
  // length
  if (fLenOnly && m_ulStreamLen != (ULONG)-1)
    return (m_ulStreamLen / uSrcCharSize);

  // Get ptr from data.
  pSeqStm = m_pSeqStmGet;

  // If ptr is NULL, set flag that there is no data and return.
  if (!pSeqStm)
  {
    *lpuFlags = DA_fNull;
    return (ULONG)0;
  }

  ulAjustedOffset = ulChunkOffset * uSrcCharSize; // scale the Offset
  // If the current position of the Stream is not the same as the
  // requested offset, return a code indicating such (the stream will
  // get re-read from scratch).
  if (m_cbReadPos != ulAjustedOffset)
    return (ULONG)-2;

  ulAjustedSize = ulChunkSize * uSrcCharSize;     // scale the chunk

  cbBytesToRead = FPDB_STREAM_READ_SIZE;
  // If requesting data and given a size, allocate to that size.
  if (!fLenOnly && ulAjustedSize)
    cbBytesToRead = ulAjustedSize;

  // Determine the destination char size (TCHAR or BYTE)
  uDestCharSize = sizeof(TCHAR);
  if (wType == DBTYPE_BYTES || wType == (DBTYPE_UI1+DBTYPE_VECTOR))
    uDestCharSize = sizeof(BYTE);

  // Since we are possibly converting (from WCHARS/CHARS to TCHARS) 
  // the buffer as we read it, the allocation size should be large enough
  // for both the number of bytes to read and the number of bytes of
  // the converted buffer.  Use the "largest" size.
  cbAllocSize = cbBytesToRead;
  if (uDestCharSize > uSrcCharSize) // If true, Dest must be 2 and Src is 1.
    cbAllocSize *= (uDestCharSize / uSrcCharSize); // multiply the size by 2.

  // Allocate a buffer which will be used to read the data into.
  tmpLBlock = LBlockAllocLen(NULL, cbAllocSize); 
  if (!tmpLBlock)
    return (ULONG)-1;  // not enough memory

  // If only getting the length, then simply call reads until no more
  // data to count.
  if (fLenOnly)
  {
    m_ulStreamLen = 0;  // initialize
    do // loop the reads
    {
      // Read the data from the Stream
      hr = pSeqStm->Read((void *)tmpLBlock, cbBytesToRead, &cbBytesRead);
      // If read was successful...
      if ((hr == S_OK || hr == DB_S_ENDOFROWSET) && cbBytesRead)
      {
        m_ulStreamLen += cbBytesRead;
        m_cbReadPos += cbBytesRead;
      }
    } while (hr == S_OK && cbBytesToRead == cbBytesRead);

    // Return the length
    ulReturn = (ULONG)m_ulStreamLen / uSrcCharSize;
  }

  // Else, getting data. With each read, get data and realloc buffer if needed.
  else
  {
    ULONG ulTotalRead = 0;  // total number of chars read (chars not bytes) 
    BOOL  fLoop;

    do // loop the reads
    {
      fLoop = FALSE;   // assume no loop, but that may change
      // Read the data from the Stream
      hr = pSeqStm->Read((void *)(tmpLBlock+ulTotalRead), cbBytesToRead, &cbBytesRead);
      // If read was successful...
      if ((hr == S_OK || hr == DB_S_ENDOFROWSET) && cbBytesRead)
      {
        cbCharsRead = cbBytesRead;  // default to "same"

        // If data is coming in different sized data, convert data.
        if (uSrcCharSize != uDestCharSize)
        {
          hr = fpdbConvertBuffer((tmpLBlock+ulTotalRead), 
            cbBytesRead, &cbCharsRead, uSrcCharSize, uDestCharSize);
          // if error (memory error), free block and return
          if (hr != S_OK)
          {
            LBlockFree(tmpLBlock);
            return (ULONG)-1;
          }
        }

        // Adjust counters
        ulTotalRead += cbCharsRead;  // count in chars
        m_cbReadPos += cbBytesRead;  // count in bytes
        // if getting the data and there is more, reallocate buffer.
        if (hr == S_OK && cbBytesToRead == cbBytesRead &&
            ulAjustedSize == 0)
        {
          if (LBlockReAllocLen(&tmpLBlock, NULL, ulTotalRead + cbAllocSize))
            fLoop = TRUE;
        } 
        else // adjust size of block to reflect the length of the data.
        {
          LBlockReAllocLen(&tmpLBlock, NULL, ulTotalRead);
        }
      }
    } while (fLoop);

    // If the stream size is uninitialized AND caller is getting 
    // all of the data at once, then save length of stream.
    if (m_ulStreamLen == (ULONG)-1 && ulAjustedSize == 0)
      m_ulStreamLen = m_cbReadPos;

    // We have "some" or all of the data, copy to desired string type.
    if (usVBDataType == DT_HSZ)
      *lplData = (LONG)fpDBCreateHszLen((LPTSTR)tmpLBlock, 
                                        ulTotalRead/sizeof(TCHAR));
    else  // else, return allocated block
    {
      *lplData = (LONG)tmpLBlock;
      fFreeLBlock = FALSE;
    }

    // Return the length
    ulReturn = ulTotalRead;
  } // if-else getting data

  if (fFreeLBlock)
    LBlockFree(tmpLBlock);
  
  return ulReturn;
}

/***************************************************************************
 *
 * FUNCTION:  fpConvertFromWSTR()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: (HRESULT)
 *
 **************************************************************************/
static HRESULT fpdbConvertBuffer(LPVOID pvData, ULONG ulInputByteLen, 
  ULONG *pulRetByteLen, UINT uSrcCharSize, UINT uDestCharSize)
{
  ULONG ulByteLen;

  // If read WCHAR's, convert to TCHAR's
  LBLOCK tmpLBlock = LBlockAllocXLenToXLen(pvData,
     (UINT)(ulInputByteLen / uSrcCharSize), uSrcCharSize, uDestCharSize);

  *pulRetByteLen = 0;

  // If allocation failed, return error
  if (tmpLBlock == NULL && ulInputByteLen != 0)
    return E_OUTOFMEMORY;

  ulByteLen = LBlockLen(tmpLBlock);  // Get new length

  // If dest char size is 2 (WCHAR), then exclude extra null byte.
  if (uDestCharSize == sizeof(WCHAR) && ulByteLen)
    ulByteLen -= 1;
  // Copy data from new block to original block.
  memcpy(pvData, tmpLBlock, ulByteLen);

  *pulRetByteLen = ulByteLen;        // Return new size.

  // Free the temp block
  LBlockFree(tmpLBlock);

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpGetStringValue()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: (ULONG)
 *
 *   0 ............. if no data (or null)
 *   Len of data ... If successful (does not include null terminator)
 *   (ULONG)-1 ..... Error (general)
 *   (ULONG)-2 ..... Error (Fetch buffer needs to be reloaded)
 *
 **************************************************************************/
ULONG CfpDBColumn::fpGetStringValue( LPSTR pszGetData, USHORT usVBDataType,
  WORD wDataType, ULONG ulChunkOffset, ULONG ulChunkSize,
  LONG FAR *lplData, DWORD dwVarLen)
{
  LPSTR pszData = NULL;
  ULONG ulReturn = (ULONG)-1;
  LONG  ulTmp = 0;
  UINT  uSrcCharSize;
  UINT  uDestCharSize;
  ULONG ulAjustedOffset;
  ULONG ulAjustedSize;
  LBLOCK tmpLBlock = NULL;


  uSrcCharSize = sizeof(BYTE);
  uDestCharSize = sizeof(TCHAR);

  // If not one of these types, then assert and return.
  switch (wDataType)
  {
    case DBTYPE_UI1+DBTYPE_VECTOR:  // possible BLOB
    case DBTYPE_BSTR:
    case DBTYPE_STR:
    case DBTYPE_WSTR:
      break;  // these are ok.

    default:
      ASSERT(0);  // Other types currently are not supported!
      return (ULONG)-1;
  }

  pszData = pszGetData;        // use pszData to ptr to data.

  // If type is a WSTR, then "source char" size is 2-bytes.
  if (wDataType == DBTYPE_WSTR)
  {
    uSrcCharSize = sizeof(WCHAR);
  }
  // Else, if type is a BSTR, get the ptr to the data and the length.
  else if (wDataType == DBTYPE_BSTR)
  {
    // for BSTR, pszData points to BSTR.
    BSTR *bstr = (BSTR *)pszGetData;
    if (bstr && *bstr)
    {
      dwVarLen = SysStringByteLen(*bstr);
      pszData = (LPSTR)*bstr;
    }
    uSrcCharSize = (ULONG)sizeof(OLECHAR);
    uDestCharSize = sizeof(TCHAR);
  }
  // Else, if type is a VECTOR, data is in a BLOB.
  else if (wDataType == (DBTYPE_UI1+DBTYPE_VECTOR))
  {
    BLOB *pBlob = (BLOB *)pszGetData;
    dwVarLen = pBlob->cbSize;           // get blob len
    pszData = (LPSTR)pBlob->pBlobData; // get ptr to blob data
    uDestCharSize = sizeof(BYTE);
  }

  ulAjustedOffset = ulChunkOffset * uSrcCharSize;
  ulAjustedSize = ulChunkSize * uSrcCharSize;
  // adjust for offset & size
  if (ulAjustedOffset && (ulAjustedOffset < (ULONG)dwVarLen))
  {
     pszData += ulAjustedOffset;     // move ptr
     dwVarLen -= ulAjustedOffset;     // decrease size
  }

  if (ulAjustedSize && (dwVarLen > ulAjustedSize))
     dwVarLen = ulAjustedSize;

  // switch on VB data type
  switch (usVBDataType)
  {
    case DT_HLSTR:
    case DT_HSZ:
      if (lplData)
      {
         // copy from buffer TO LBlock, then, if HSZ, call fpDBCreateHsz()
         // so that the "types" match.  
         tmpLBlock = LBlockAllocXLenToXLen(pszData,
           (UINT)(dwVarLen / uSrcCharSize), uSrcCharSize, uDestCharSize);
         if ((usVBDataType == DT_HSZ) && tmpLBlock)
         {
            *lplData = (LONG)fpDBCreateHsz((LPTSTR)tmpLBlock);
            LBlockFree(tmpLBlock);
         }
         else   
            *lplData = (LONG)tmpLBlock;
      }
      ulReturn = (ULONG)dwVarLen / uSrcCharSize;
      break;

    default:
      // Add support for this VB type!!!
      ASSERT(0);
      break;
  } //switch (usVBDataType)

  return ulReturn;
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpSetDataValue()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 *   usVBDataType ... Format for the returned data (DT_LONG, DT_HSZ, etc).
 *   ulChunkOffset .. For types DBTYPE_CHAR and VT_BLOB, gets chunk at offset.
 *   ulChunkSize .... Sizeof chunk to get.
 *   lData  ......... Either the value or ptr to string. 
 *   uFlags ......... Flags (indicates NULL value)
 *
 * RETURNS:
 *
 *   0 if successful, else ERROR
 *
 **************************************************************************/
LONG CfpDBColumn::fpSetDataValue( LPSTR pszSetBuffer, USHORT usVBDataType, 
  ULONG ulChunkOffset, ULONG ulChunkSize, LONG lData, UINT uFlags)
{
  LPSTR pszSetData;
  USHORT usDBtype;

  // if AutoIncrement OR not updatable
  if (uMetaFlags & DB_AUTOINCRFIELD || !(uMetaFlags & DB_UPDATABLEFIELD))
  {
    // Cannot update field, it is SET when added and can only be deleted!
    return (LONG)FPDB_ERR_CANTSETAUTOINCR;
  }

  // Are we setting it to NULL?
  if (uFlags & DA_fNull)
  {
     *(DBSTATUS FAR *)(pszSetBuffer + m_obStatus) = DBSTATUS_S_ISNULL;
     m_fSet = FPDB_DATA_SET;
     return (LONG)0;                          // Bind is ready, return
  }
  else
    *(DBSTATUS FAR *)(pszSetBuffer + m_obStatus) = DBSTATUS_S_OK;


  usDBtype = LOWORD(fpGetVBDataType());
  pszSetData = pszSetBuffer + m_obValue;

  // if we are given a string AND the target type is not a string...
  if ((usDBtype != DATA_VT_MEMO) && (usDBtype != DATA_VT_TEXT) &&
      (usDBtype != DATA_VT_BINARY))
  {
    return fpSetNonStringValue(pszSetData, usVBDataType, usDBtype, lData);
  } 
  // else we have a string type field and are using "regular" binding
  else if (m_wTypeBindSet != DBTYPE_IUNKNOWN)
  {
    // Specifying chunks is NOT SUPPORTED for field which are not "large"
    _ASSERT((ulChunkOffset == 0) && (ulChunkSize == 0)); 
    return fpSetStringValue(pszSetBuffer, usVBDataType, usDBtype, lData);
  } 
  // else using IStream
  else 
  {
    return fpSetStreamValue(pszSetData, usVBDataType, lData,
      ulChunkOffset, ulChunkSize);
  }

  _ASSERT(0);  // Unknown condition!!!
  return -1;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpSetNonStringValue()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   0 if successful, else ERROR
 *
 **************************************************************************/
LONG CfpDBColumn::fpSetNonStringValue( LPSTR pszSetData, USHORT usVBDataType, 
  USHORT usDBtype, LONG lData)
{
  LONG lSize;

  // if we are given a string AND the target type is not a string,
  // convert to binary type.
  if ((usVBDataType == DT_HLSTR) || (usVBDataType == DT_HSZ))
  {
    lSize = fpConvertTSTRtoType( (LPTSTR)lData, pszSetData, 
              fpDBTypeToVarType(m_wTypeBindSet));
    if (lSize <= 0)
       return (LONG)FPDB_ERR_CONVERTSTRINGTOTYPE;
    m_fSet = FPDB_DATA_SET;
  } 
  // else, some type of "fixed-len binary type" and not given a string
  else
  {
    lSize = 0L;
    switch (usVBDataType)
    {
      case DT_SHORT:  lSize = sizeof(short);  break; // 0x02
      case DT_BOOL :  lSize = sizeof(BOOL);   break; // 0x04
      case DT_LONG :  lSize = sizeof(long);   break; // 0x03
      case DT_REAL :  lSize = sizeof(float);  break; // 0x07
      case DT_FP_REAL8 :  lSize = sizeof(double);  break; // 0x0E
      case DT_FP_DATETIME :  lSize = sizeof(double);  break; // 0x0F
      default:
        ASSERT(0);    //Add support!!!
        return (LONG)FPDB_ERR_NOTIMPLEMENTED;
    }
    // ptr to data
    if (lSize > 0)
    {
      // If size is greater than 4 bytes, use lData as a ptr instead of 
      // holding the data itself.
      memcpy(pszSetData, 
             (lSize > sizeof(long) ? (LPVOID)lData : &lData), 
             (size_t)lSize);
      m_fSet = FPDB_DATA_SET;
    }
  }

  return 0;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpSetStringValue()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   0 if successful, else ERROR
 *
 **************************************************************************/
LONG CfpDBColumn::fpSetStringValue( LPSTR pszSetBuffer, USHORT usVBDataType, 
  USHORT usDBtype, LONG lData)
{
  LONG lSize;
  UINT uDestCharSize;
  LPSTR pszSetData = pszSetBuffer + m_obValue;

  _ASSERT(lData);   // we must have data.

  // Determine the size of the destination chars.
  uDestCharSize = sizeof(BYTE);  // assume byte, prove otherwise.
  if (m_wTypeBindSet == DBTYPE_WSTR || m_wTypeBindSet == DBTYPE_BSTR)
    uDestCharSize = sizeof(WCHAR);

  switch (m_wTypeBindSet)
  {
    case DBTYPE_BSTR:  // allocate buffer and make "pszData" point to it.
      m_pSetData = fpSysAllocBSTRFromTString((LPTSTR)lData);
      if (m_pSetData)
      {
        *(void **)pszSetData = m_pSetData;
        m_fSet = FPDB_DATA_SET | FPDB_DATA_ALLOCATED;
      }
      break;

    case DBTYPE_STR:  // Copy data to space already provided in "pszData".
    case DBTYPE_WSTR:
    case DBTYPE_BYTES:
      if (usVBDataType == DT_HLSTR)
        lSize = LBlockLen((LBLOCK)lData);
      else
        lSize = (LONG)fpSysTStringLen((FPTSTR)lData)*sizeof(TCHAR);

      // TO DO: Be conscious of char widths, convert if necessary.
      if (m_wTypeBindSet != DBTYPE_BYTES && uDestCharSize != sizeof(TCHAR))
      {
        // Allocate a LBlock, then copy data from it to the buffer.
        LBLOCK tmpLBlock = LBlockAllocXLenToXLen((LPVOID)lData,
           (UINT)(lSize / sizeof(TCHAR)), sizeof(TCHAR), uDestCharSize);

        if (tmpLBlock)
        {
          lSize = LBlockLen(tmpLBlock);  // get new size.
          // If writing out WCHARS, exclude extra null char (odd byte)
          if (m_wTypeBindSet == DBTYPE_WSTR && (lSize % sizeof(WCHAR)))
            lSize -= 1;  // remove odd byte
          memcpy(pszSetData, tmpLBlock, min((ULONG)lSize, m_cbMaxLen));
          LBlockFree(tmpLBlock);
        }
      }
      else // just copy data.
        memcpy(pszSetData, (LPVOID)lData, min((ULONG)lSize, m_cbMaxLen));

      // Set the length for this type.
      *(ULONG FAR *)(pszSetBuffer + m_obLength) = min((ULONG)lSize, m_cbMaxLen);

      m_fSet = FPDB_DATA_SET;
      break;
  } // switch

  return 0;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpSetStreamValue()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 *   usVBDataType ... Format for the returned data (DT_LONG, DT_HSZ, etc).
 *   ulChunkOffset .. For types DBTYPE_CHAR and VT_BLOB, gets chunk at offset.
 *   ulChunkSize .... Sizeof chunk to get.
 *   lData  ......... Either the value or ptr to string. 
 *
 * RETURNS:
 *
 *   0 if successful, else ERROR
 *
 **************************************************************************/
LONG CfpDBColumn::fpSetStreamValue( LPSTR pszSetData, USHORT usVBDataType, 
  LONG lData, ULONG ulChunkOffset, ULONG ulChunkSize)
{
  HRESULT hr;
  LARGE_INTEGER dliMove = {0};
  ULARGE_INTEGER uliNewPos;
  ULONG ulCurPos = 0;
  BOOL  fJustCreated = FALSE;
  ULONG ulByteCt;
  LBLOCK tmpLBlock = NULL;
  UINT uSrcCharSize;
  UINT uDestCharSize;

  // If we don't already have a stream, create an empty stream.  
  if (!m_pstmSet)
  {
    IStream *pTmpStm = NULL;
    
    // Call CreateStreamOnHGlobal() to create a stream which will be
    // passed to a new CfpDbStream object.
    hr = CreateStreamOnHGlobal(NULL, TRUE, &pTmpStm);
    if (hr != S_OK || !pTmpStm)
      return FPDB_ERR_LBLOCKALLOC;

    // Use our own IStream implementation. 
    // Note: CfpDbStream will add a reference to pTmpStm.
    m_pstmSet = new CfpDbStream(pTmpStm);
    pTmpStm->Release();        // release this reference.

    // Set the data buffer so that it contains the IStream ptr.
    *(LPSTREAM *)pszSetData = m_pstmSet;

    // Set flag that indicates we have a stream.
    m_fSet |= FPDB_STREAM_CREATED | FPDB_DATA_SET;
    fJustCreated = TRUE;
    m_pstmSet->AddRef();
  }

  // Get the current position of the stream (if just created then its 0)
  if (!fJustCreated)
    if (m_pstmSet->Seek(dliMove, STREAM_SEEK_CUR, &uliNewPos) == S_OK)
      ulCurPos = uliNewPos.LowPart;
    
  // Set the current position to the specified offset.
  if (ulChunkOffset != ulCurPos)
  {
    dliMove.LowPart = ulChunkOffset;
    hr = m_pstmSet->Seek(dliMove, STREAM_SEEK_SET, &uliNewPos);
    _ASSERT(hr == S_OK);   // Could not seek to specified offset!!!
    if (hr != S_OK)
      return FPDB_ERR_LBLOCKALLOC;   // NOTE: May not be the true reason.
  }

  // If both the chunk and offset parameters are 0, then assume that
  // the control wants all of the data to be written out, so determine
  // the size of the data.
  if (!ulChunkSize && !ulChunkOffset)
    if (usVBDataType == DT_HLSTR)
      ulChunkSize = (ULONG)LBlockLen((LBLOCK)lData);
    else
      ulChunkSize = (ULONG)fpSysTStringByteLen((FPTSTR)lData);

  // Write data to stream.
  if (ulChunkSize)
  {
    // Get size of "dest" chars (either WORD or BYTE chars).
    uDestCharSize = 
      (ULONG)(wType == DBTYPE_WSTR ? sizeof(WCHAR) : sizeof(BYTE));

    // Determine the source char size (TCHAR or BYTE)
    uSrcCharSize = sizeof(TCHAR);
    if (wType == DBTYPE_BYTES || wType == (DBTYPE_UI1+DBTYPE_VECTOR))
      uSrcCharSize = sizeof(BYTE);

    // If char sizes don't match, convert buffer.
    if (uSrcCharSize != uDestCharSize)
    {
      // Copy from buffer TO LBlock with convert chars.
      tmpLBlock = LBlockAllocXLenToXLen((LPVOID)lData,
           (UINT)ulChunkSize, uSrcCharSize, uDestCharSize);
      if (!tmpLBlock)
        return FPDB_ERR_LBLOCKALLOC;
      ulChunkSize = LBlockLen(tmpLBlock);  // set new length
// 19696 (don't mess with ulChunkSize) -scl
      // if converting TO WCHARs,  subtract extra null char.
//      if (uDestCharSize == sizeof(WCHAR) && ulChunkSize) 
//        ulChunkSize -= 1;
      lData = (LONG)tmpLBlock;  // make lData ptr to new buffer
    }

    // write data to stream
    hr = m_pstmSet->Write((void *)lData, ulChunkSize, &ulByteCt);

    // if we allocated a buffer, free it.
    if (tmpLBlock)
      LBlockFree(tmpLBlock);

    // Look for errors writing to the stream.
    _ASSERT(hr == S_OK);
    if (hr != S_OK)
      return FPDB_ERR_LBLOCKALLOC;
  }

  return 0;
}





/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpGetVBDataType()
 *
 * DESCRIPTION:
 *
 *   VB Data types:
 *   --------------
 *   DATA_VT_BOOL
 *   DATA_VT_VAR_BYTE
 *   DATA_VT_INTEGER
 *   DATA_VT_LONG
 *   DATA_VT_CURRENCY
 *   DATA_VT_SINGLE
 *   DATA_VT_DOUBLE
 *   DATA_VT_DATETIME
 *   DATA_VT_TEXT
 *   DATA_VT_BINARY
 *   DATA_VT_MEMO
 *
 * PARAMETERS:
 *
 * RETURNS:  VB Data type (USHORT)
 *
 **************************************************************************/
DWORD CfpDBColumn::fpGetVBDataType(void)
{
  WORD retType = (WORD)0;          // old support
  WORD extendedRetType = (WORD)0;  // new support
  WORD wTmpType = wType;

  // If type is NUMERIC, type being used may actually be different,
  // in that case, use "m_wTypeBind" field.
  if (wType == DBTYPE_NUMERIC)
    wTmpType = m_wTypeBind;

  // Remove these flags from type
  wTmpType &= ~(DBTYPE_VECTOR | DBTYPE_ARRAY);

  switch (wTmpType)
  {

    case DBTYPE_I2:     /* 2   */
    case DBTYPE_UI2:    /* 18  */  retType = DATA_VT_INTEGER;  break; 

//  case DBTYPE_HCHAPTER: /* 136 */
    case DBTYPE_ERROR:
    case DBTYPE_I4:     /* 3   */
    case DBTYPE_UI4:    /* 19  */  retType = DATA_VT_LONG;     break; 

    case DBTYPE_R4:     /* 4   */  retType = DATA_VT_SINGLE;   break; 
    case DBTYPE_CY:     /* 6   */  retType = DATA_VT_CURRENCY; break; 
    case DBTYPE_BOOL:   /* 11  */  retType = DATA_VT_BOOL;     break; 

    case DBTYPE_R8:     /* 5   */  
      retType = DATA_VT_DOUBLE;
      break; 


    case DBTYPE_DATE:        // 7   
	case DBTYPE_DBDATE:      // 133
	case DBTYPE_DBTIME:      // 134
	case DBTYPE_DBTIMESTAMP: // 135
      retType = DATA_VT_DATETIME;
      break; 

    case DBTYPE_BSTR: // = 8
	case DBTYPE_WSTR: // = 130
      // if MAX LEN > (255 * 2), then MEMO, else TEXT
      if ((ulColumnSize > (ULONG)255) || (dwFlags & DBCOLUMNFLAGS_ISLONG))
      {
         retType = DATA_VT_MEMO;     
         extendedRetType = UDATA_VT_WMEMO;    // WCHARs 
      }
      else
      {
         retType = DATA_VT_TEXT;
         extendedRetType = UDATA_VT_WTEXT;     // WCHARs
      }
      break; 

	case DBTYPE_STR:  // = 129
      // if MAX LEN > 255 or not Data-type binding, then MEMO, else TEXT
      if ((ulColumnSize > (ULONG)255) || (dwFlags & DBCOLUMNFLAGS_ISLONG))
         retType = DATA_VT_MEMO;     
      else
         retType = DATA_VT_TEXT;
      break; 

//  case DBTYPE_BLOB:   /* 65 */
    case DBTYPE_BYTES: // = 128
      retType = DATA_VT_BINARY;
      break; 

	case DBTYPE_UI1:  // = 17
      if (wType & (DBTYPE_VECTOR | DBTYPE_ARRAY))
        retType = DATA_VT_BINARY;
      else
        retType = DATA_VT_VAR_BYTE;
      break;

    // These types are supported as STRINGs to avoid lost precision.
	case DBTYPE_I8:          // 20
	case DBTYPE_UI8:         // 21
    case DBTYPE_DECIMAL:
    case DBTYPE_NUMERIC:
    case DBTYPE_GUID:
	case DBTYPE_VARNUMERIC:
      retType = DATA_VT_TEXT;
      break;

    case DBTYPE_VARIANT:     // 12 - Default to text.
      retType = DATA_VT_TEXT;
      break;

    // Types not yet translated or not supported:
	case DBTYPE_EMPTY:       // 0
	case DBTYPE_NULL:        // 1
    default:
      _ASSERT(0);  // VERIFY THIS CODE!!!
      if (wType & (DBTYPE_VECTOR | DBTYPE_ARRAY))
        retType = DATA_VT_BINARY;
      else
      {  
        // These types need translation or don't have VB equivalents.
        _ASSERT(0);
      }
      break; 
  }

  // if extendedRetType is 0, set it to the regular value
  if (!extendedRetType)
    extendedRetType = retType; 

  return (DWORD)MAKELONG( retType, extendedRetType);
}




/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpPrepareSetState()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBColumn::fpPrepareSetState(void)
{
  // If this column is writing out a Stream, set the current position
  // to the beginning
  if (m_pstmSet)
  {
    LARGE_INTEGER dliMove = {0};
    HRESULT hr = m_pstmSet->Seek(dliMove, STREAM_SEEK_SET, NULL);
    _ASSERT(hr == S_OK);   // Could not seek to specified offset!!!
  }
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpClearSetState()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBColumn::fpClearSetState(LPSTR pszSetBuffer)
{
  if (m_pSetData)
  {
    if (m_fSet & FPDB_DATA_ALLOCATED)
      fpSysFreeTString((FPTSTR)m_pSetData);
    m_pSetData = NULL;
  }

  if (m_pstmSet)
  {
    m_pstmSet->Release();  // this should be the only reference
    m_pstmSet = NULL;
  }

  m_fSet = 0;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpHasStreamSetData()
 *
 * DESCRIPTION:
 *
 *   Returns whether this column is writing out a Stream.
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 *   TRUE if this column has stream data set, otherwise FALSE.
 *
 **************************************************************************/
BOOL CfpDBColumn::fpHasStreamSetData(void)
{
  // Return whether this column is writing out a Stream.
  return (m_pstmSet != NULL);
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpIsStreamCol()
 *
 * DESCRIPTION:
 *
 *   Returns TRUE if the column uses streams for the data (ISLONG).
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 *   TRUE or FALSE
 *
 **************************************************************************/
BOOL CfpDBColumn::fpIsStreamCol(void)
{
  // Returns TRUE if dwFlags contains this bit.
  return ((dwFlags & DBCOLUMNFLAGS_ISLONG) != 0);
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::fpNeedsOwnAccessor()
 *
 * DESCRIPTION:
 *
 *   Returns TRUE if the column needs a seperate accessor to write
 *   the data.
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 *   TRUE or FALSE
 *
 **************************************************************************/
BOOL CfpDBColumn::fpNeedsOwnAccessor(void)
{
  // Currently, there are no column types (other than streams) which
  // required their own accessor.
  return FALSE;
}


/***************************************************************************
 *
 * FUNCTION:  fpDBTypeToVarType()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
static USHORT fpDBTypeToVarType(USHORT usDBType)
{
  USHORT usRet = VT_EMPTY;

  switch (usDBType)
  {
    case DBTYPE_DATE:  usRet = VT_DATE;  break;
    case DBTYPE_I2:    usRet = VT_I2;    break;
    case DBTYPE_I4:    usRet = VT_I4;    break;
    case DBTYPE_R4:    usRet = VT_R4;    break;
    case DBTYPE_R8:    usRet = VT_R8;    break;
    case DBTYPE_CY:    usRet = VT_CY;    break;
    case DBTYPE_BOOL:  usRet = VT_BOOL;  break;
    case DBTYPE_UI1:   usRet = VT_UI1;   break;

    default:       // Note: default of -1 is returned
      ASSERT(0);   // Add support for this type!!!
      break;
  }

  return usRet;
}
