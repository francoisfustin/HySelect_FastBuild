/*************************************************************************\
* FPDBCOL.CPP                                                             *
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

#ifdef STRICT
#undef STRICT
#endif

#ifndef NO_MFC
#include <afxctl.h>         // MFC support for OLE Custom Controls
#else  //NO_MFC
#include "stdafx.h"
#include "fpatl.h"
#endif

//#include "stdafx.h"
#include "vbinterf.h"

#include "fpconvrt.h"
#include "fptstr.h"                             

#include "fpdbutil.h"		  
#include "fpdbcol.h"
#include "fpoledb.h"		        // get defines for error codes

#include <limits.h>
#include <malloc.h>

#ifndef WIN32  // define for WIN16
#define SysStringByteLen(bstr) SysStringLen(bstr)
#endif

//--------------------------------------------------------------------------
// CfpDBColumn
CfpDBColumn::CfpDBColumn()
{
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::CfpDBColumn( w/parameters )
 *
 * DESCRIPTION:
 *
 *   Constructor with initialization parameters.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBColumn::CfpDBColumn(LPCTSTR szName, DBCOLUMNID ColID, ULONG ulType, 
                           ULONG ulMaxLen, DWORD dwBindType, 
                           int iIndex, UINT uFlags)
{
	m_strDataField  = szName ? fpSysAllocTString(szName) : NULL;
	m_ColID         = ColID;
	m_ulType        = ulType; 
    m_ulMaxLen      = ulMaxLen;
    m_dwBindType    = dwBindType;
//    m_ulEntryMaxLen = ulEntryMaxLen;
    m_iIndex        = iIndex;
    m_uFlags        = uFlags;

    // Binding info
    m_obInfo        = DB_NOVALUE;
    m_obVarDataLen  = DB_NOVALUE;
    m_obData        = DB_NOVALUE;
    m_dwBindTypeUsed = 0;
    m_dwDataTypeUsed = 0;

    m_fNewRead      = FPDB_COL_NOFETCH;
    m_lpBlob        = NULL;
    m_ulReadSize    = (ULONG)0;

    m_lpStream      = NULL;
    m_lpWriteBlob   = NULL;

}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::FillColBindRec()
 *
 * DESCRIPTION:
 *
 *   Fill the fields in the DBCOLUMNBINDING structure for this Column.
 *   Other parameters are for Inline and Out-of-line buffer offsets.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpDBColumn::FillColBindRec( DBCOLUMNBINDING *pColBind, 
  ULONG *pulInOffset, ULONG *pulOutOffset, BOOL fRemoteDataCtl, BOOL fUseEntryId)
{
  // NOTE:  Restrict variable len's to 10k (for the time being)
  ULONG ulMaxLen = ((m_ulMaxLen > 10240) ? 10240 : m_ulMaxLen);
#ifdef USEVARIANT
  BOOL  fUseVariant = TRUE;
#else
  BOOL  fUseVariant = FALSE;
#endif

  // Column ID
  pColBind->columnID = m_ColID;    

  pColBind->dwDataType = m_ulType;

  // Binding Method: given a choice between DEFAULT and ENTRY_ID, 
  // use ENTRY_ID for most flexibility
  switch (m_dwBindType)
  {
	case DBBINDTYPE_ENTRYID:  // = 1
      pColBind->dwBinding = DBBINDING_ENTRYID;
      pColBind->dwDataType = VT_BLOB;
      break;

	case DBBDINTYPE_EITHER:   // = 2
	case DBBINDTYPE_BOTH:     // = 3
      // If given a choice, it must be a Memo-type field, use DEFAULT and 
      // VT_BSTR.
#ifdef KEEPTHIS  // SCP 5/8/96: This is the right way BUT EntryId's don't work with text
      if (fUseEntryId)
      {
        pColBind->dwBinding = DBBINDING_ENTRYID;
        pColBind->dwDataType = VT_BLOB;
      }
      else
      {
        pColBind->dwBinding = DBBINDING_DEFAULT;
        pColBind->dwDataType = VT_BSTR;
      }
#endif  //0
      pColBind->dwBinding = DBBINDING_DEFAULT;
      // Since EntryId's don't work properly for text, use bstr's
      if (fUseEntryId)
        pColBind->dwDataType = VT_BSTR;
      break;

    case DBBINDTYPE_DATA:     // = 0
    default:
      pColBind->dwBinding = DBBINDING_DEFAULT;
  }

  // DataType
  if (pColBind->dwBinding == DBBINDING_DEFAULT)
  {
     switch (pColBind->dwDataType)
     {
       case VT_LPSTR:   // map VT_LPSTR's to DBTYPE's
         if (!fRemoteDataCtl)
           pColBind->dwDataType = DBTYPE_CHARS;
         break;

       case VT_LPWSTR:  
         if (!fRemoteDataCtl)
           pColBind->dwDataType = DBTYPE_WCHARS;
         break;

       case VT_BSTR:
         // if not Memo-type OR not fUseEntryId, use allocated buffer
         if ((m_dwBindType == DBBINDTYPE_DATA) || !fUseEntryId)
         {
  		   if (sizeof(TCHAR) == sizeof(WORD))
             pColBind->dwDataType = DBTYPE_WCHARS;
           else
             pColBind->dwDataType = DBTYPE_CHARS;
         }
         break;

       default:
         pColBind->dwDataType = m_ulType;
     }
   
     // If Remote DC, ignore above setting and use LPxSTR
     if (fRemoteDataCtl)
     {
	   if ((sizeof(TCHAR) == sizeof(WORD)) && (m_ulType != DBTYPE_BYTES))
         pColBind->dwDataType = VT_LPWSTR;
       else
         pColBind->dwDataType = VT_LPSTR;
     }
  
     // If using type DBVARIANT, ignore settings from above
     if (fUseVariant && !fRemoteDataCtl)
     {
       pColBind->dwBinding  = DBBINDING_VARIANT;
       pColBind->dwDataType = m_ulType;
     }
  }

  // returned info flags
  pColBind->obInfo = *pulInOffset;
  *pulInOffset    += sizeof(DWORD); 

  switch ((pColBind->dwBinding == DBBINDING_VARIANT) ? 
           DBTYPE_ANYVARIANT : pColBind->dwDataType)
  {
    // if variable-len data, set offset into Inline buffer
    case DBTYPE_CHARS:
    case DBTYPE_WCHARS:
    case DBTYPE_BYTES:
      pColBind->obVarDataLen = *pulInOffset;  // returned len
      *pulInOffset          += sizeof(DWORD);
      pColBind->cbMaxLen     = ulMaxLen+2;        // set max len
      pColBind->obData       = *pulInOffset;  // returned data
      *pulInOffset          += pColBind->cbMaxLen;
      break;

    case VT_BSTR:
      pColBind->obVarDataLen = DB_NOVALUE;  
      pColBind->cbMaxLen     = DB_NOMAXLENGTH;        // set max len
      pColBind->obData       = *pulInOffset;  // returned data
      *pulInOffset          += sizeof(DWORD);
      break;

    case VT_LPSTR:
    case VT_LPWSTR:
      pColBind->obVarDataLen = *pulInOffset;  // returned len
      *pulInOffset          += sizeof(DWORD);
      pColBind->cbMaxLen     = ulMaxLen+2;    // set max len
      pColBind->obData       = *pulInOffset;  // returned data
      *pulInOffset          += sizeof(DWORD);
      *pulOutOffset         += pColBind->cbMaxLen;
      break;
      
    case VT_BLOB:     // this is variable len, but we get an EntryID (which 
                      // is also variable len)
      pColBind->obVarDataLen = *pulInOffset;  // returned len
      *pulInOffset          += sizeof(DWORD);
      pColBind->cbMaxLen     = DB_NOMAXLENGTH;
      pColBind->obData       = *pulInOffset;   // BLOB structure
      *pulInOffset          += sizeof(BLOB);   // sizeof(BLOB)
      *pulOutOffset         += 100; // storage for IStream in out-bound mem
      break;

    case DBTYPE_ANYVARIANT:  // Using type DBVARIANT
      pColBind->obVarDataLen = *pulInOffset;  // returned len
      *pulInOffset          += sizeof(DWORD);
      pColBind->cbMaxLen     = DB_NOMAXLENGTH;
      pColBind->obData       = *pulInOffset;   // DBVARIANT structure
      *pulInOffset          += sizeof(DBVARIANT); // sizeof(DBVARIANT)
//    *pulOutOffset         += XXX; // set aside storage???
      break;

    default:         // All others are fixed-length
      pColBind->obVarDataLen = DB_NOVALUE;
      pColBind->cbMaxLen     = DB_NOMAXLENGTH;
      pColBind->obData       = *pulInOffset;
      *pulInOffset          += ulMaxLen;           // max len should = len
  }

  // Save some of the values set (for retrieving the data after the fetch)
  m_obInfo       = pColBind->obInfo;
  m_obVarDataLen = pColBind->obVarDataLen;
  m_obData       = pColBind->obData;
  m_dwBindTypeUsed = pColBind->dwBinding;
  m_dwDataTypeUsed = pColBind->dwDataType;

  m_fNewRead       = FPDB_COL_NOFETCH;

  return TRUE;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::FillColBindDbVariant()
 *
 * DESCRIPTION:
 *
 *   Fill the fields in the DBCOLUMNBINDING structure for this Column.
 *   Other parameters are for Inline and Out-of-line buffer offsets.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpDBColumn::FillColBindDbVariant( DBCOLUMNBINDING *pColBind, 
  ULONG *pulInOffset, ULONG *pulOutOffset)
{

  // if the type is VT_ARRAY + VT_UI1 (0x2011), then it is a BINARY field,
  // don't use DBVariants (it won't work).
  if (m_ulType == (VT_ARRAY + VT_UI1))
    return FillColBindRec( pColBind, pulInOffset, pulOutOffset, TRUE, FALSE);

  // Column ID
  pColBind->columnID = m_ColID;    
  pColBind->dwDataType = m_ulType;
  pColBind->dwBinding  = DBBINDING_VARIANT;

  // returned info flags
  pColBind->obInfo = *pulInOffset;
  *pulInOffset    += sizeof(DWORD); 

  pColBind->obVarDataLen = *pulInOffset;  // returned len
  *pulInOffset          += sizeof(DWORD);
  pColBind->cbMaxLen     = DB_NOMAXLENGTH;
  pColBind->obData       = *pulInOffset;   // DBVARIANT structure
  *pulInOffset          += sizeof(DBVARIANT); // sizeof(DBVARIANT)
//*pulOutOffset         += XXX; // set aside storage???

  // Save some of the values set (for retrieving the data after the fetch)
  m_obInfo       = pColBind->obInfo;
  m_obVarDataLen = pColBind->obVarDataLen;
  m_obData       = pColBind->obData;
  m_dwBindTypeUsed = pColBind->dwBinding;
  m_dwDataTypeUsed = pColBind->dwDataType;

  m_fNewRead       = FPDB_COL_NOFETCH;

  return TRUE;
}




/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::GetVBDataType()
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
DWORD CfpDBColumn::GetVBDataType(void)
{
  WORD retType = (WORD)0;          // old support
  WORD extendedRetType = (WORD)0;  // new support

  switch (m_ulType)
  {

    case DBTYPE_I2:     /* 2   */
    case DBTYPE_UI2:    /* 504 */  retType = DATA_VT_INTEGER;  break; 

    case DBTYPE_I4:     /* 3   */
    case DBTYPE_UI4:    /* 505 */  retType = DATA_VT_LONG;     break; 

    case DBTYPE_R4:     /* 4   */  retType = DATA_VT_SINGLE;   break; 
    case DBTYPE_R8:     /* 5   */  retType = DATA_VT_DOUBLE;   break; 
    case DBTYPE_CY:     /* 6   */  retType = DATA_VT_CURRENCY; break; 
    case DBTYPE_DATE:   /* 7   */  retType = DATA_VT_DATETIME; break; 
    case DBTYPE_BOOL:   /* 11  */  retType = DATA_VT_BOOL;     break; 

    case DBTYPE_LPWSTR: /* 31  */ 
    case DBTYPE_WCHARS: /* 510 */
      // if MAX LEN > (255 * 2), then MEMO, else TEXT
      if ((m_ulMaxLen > (ULONG)(255 * 2)) || (m_dwBindType != DBBINDTYPE_DATA))
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

    case VT_BSTR:       /* 8   */ 
    case DBTYPE_LPSTR:  /* 30  */ 
    case DBTYPE_CHARS:  /* 509 */  
      // if MAX LEN > 255 or not Data-type binding, then MEMO, else TEXT
      if ((m_ulMaxLen > (ULONG)255) || (m_dwBindType != DBBINDTYPE_DATA))
         retType = DATA_VT_MEMO;     
      else
         retType = DATA_VT_TEXT;
      break; 

    case DBTYPE_BLOB:   /* 65 */
    case DBTYPE_BYTES:  /* 508 */  retType = DATA_VT_BINARY;   break; 

    // Types not yet translated or not supported:
	case DBTYPE_EMPTY:       // 0
	case DBTYPE_NULL:        // 1
	case DBTYPE_I8:          // 20
	case DBTYPE_HRESULT:     // 25
	case DBTYPE_FILETIME:    // 64
	case DBTYPE_UUID:        // 72
	case DBTYPE_UI8:         // 506
	case DBTYPE_COLUMNID:    // 507
	case DBTYPE_ANYVARIANT:  // 51
    default:
      if (m_ulType & VT_ARRAY)
        retType = DATA_VT_BINARY;
      else
      {  
        // These types need translation or don't have VB equivalents.
        ASSERT(0);
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
 * FUNCTION:  CfpDBColumn::GetDataValue()
 *
 * DESCRIPTION:
 *
 *   This function copies the data for the field from the "Fetch Buffer".
 *
 * PARAMETERS:
 *
 *   lpszFetchBuff .. Ptr to fetch buffer.
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
 *   FFFFFFFF ...... If error (-1) 
 *
 **************************************************************************/
ULONG CfpDBColumn::GetDataValue( LPSTR lpszFetchBuff, USHORT usVBDataType,
  ULONG ulChunkOffset, ULONG ulChunkSize,
  LONG FAR *lplData, UINT FAR *lpuFlags, ICursor FAR *lpCursor)
{
  DWORD dwInfo = 0;
  LPSTR lpszData = NULL;
  DWORD dwVarLen = (DWORD)-1;
  ULONG ulReturn = (ULONG)-1;
  LONG  ulTmp = 0;
  HRESULT h;
  UINT  uSrcCharSize;
  UINT  uDestCharSize;
  ULONG ulAjustedOffset;
  ULONG ulAjustedSize;
  LBLOCK tmpLBlock = NULL;
  DBVARIANT FAR *lpdbVar;
  DWORD dwDataType;

  if (lplData)
    *lplData = 0;

  // Retrieve INFO from the Fetch buffer
  if (m_obInfo != DB_NOVALUE)
     dwInfo = *(DWORD FAR *)(lpszFetchBuff + m_obInfo);

  // if the field value is null, set flag and return
  if ((dwInfo == DB_NULL) || (dwInfo == DB_EMPTY) || (m_obData == DB_NOVALUE))
  {
    *lpuFlags = DA_fNull;
    return (ULONG)0;
  }

  // get ptrs to data and its length
  lpszData = lpszFetchBuff + m_obData;
  if (m_obVarDataLen != DB_NOVALUE)
     dwVarLen = *(DWORD FAR *)(lpszFetchBuff + m_obVarDataLen);
  else
     dwVarLen = m_ulMaxLen;

  uSrcCharSize = (ULONG)sizeof(BYTE);
  dwDataType = m_dwDataTypeUsed;

  // If DBVARIANT was used, set lpszData to data in DBVARIANT structure.
  if (m_dwBindTypeUsed == DBBINDING_VARIANT)
  {
    lpdbVar = (DBVARIANT FAR *)lpszData;
    GetDBVariantData(lpdbVar, &dwDataType, (LPVOID FAR *)&lpszData, &dwVarLen);
    if (lpszData == NULL)
    {
      *lpuFlags = DA_fNull;
      return (ULONG)0;
    }
  }

  switch (dwDataType)
  {
    case DBTYPE_WCHARS:
      uSrcCharSize = (ULONG)sizeof(WORD);  // FALL-THRU!
    case DBTYPE_CHARS:
    case DBTYPE_BYTES:
    case VT_BSTR:
      if (m_dwDataTypeUsed == VT_BSTR)
      {
        // for BSTR, lpszData currently points to BSTR, make lpszData 
        // point to actual data
        BSTR bstr = ((m_dwBindTypeUsed == DBBINDING_VARIANT) ? 
          (BSTR)lpszData : *(BSTR FAR *)lpszData);
        if (bstr)
        {
          lpszData = (LPSTR)bstr;
          dwVarLen = SysStringByteLen(bstr);
        }
        uSrcCharSize = (ULONG)sizeof(OLECHAR);
      }
      ulAjustedOffset = ulChunkOffset * uSrcCharSize;
      ulAjustedSize = ulChunkSize * uSrcCharSize;
      // adjust for offset & size
      if (ulAjustedOffset && (ulAjustedOffset < (ULONG)dwVarLen))
      {
         lpszData += ulAjustedOffset;     // move ptr
         dwVarLen -= ulAjustedOffset;     // decrease size
      }

      if (ulAjustedSize && (dwVarLen > ulAjustedSize))
         dwVarLen = ulAjustedSize;

      // Set the destination char size (assuming TCHAR)
      uDestCharSize = (m_dwDataTypeUsed == DBTYPE_BYTES) ? 
        sizeof(BYTE) : sizeof(TCHAR);

      // switch on VB data type
      switch (usVBDataType)
      {
        case DT_HLSTR:
        case DT_HSZ:
          if (lplData)
          {
             // copy from buffer TO LBlock, then, if HSZ, call fpDBCreateHsz()
             // so that the "types" match.  
             tmpLBlock = LBlockAllocXLenToXLen(lpszData,
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
      break;

    case VT_BLOB:
      // If the read is new, set the BLOB ptr
      if (m_fNewRead == FPDB_COL_POSTFETCH)
      {
        //m_lpBlob = (BLOB FAR *)lpszData; 
        m_ulReadSize = 0;         // info and set
//      m_fNewRead = FALSE;

        this->BlobQuerySize(lpCursor);
        m_fNewRead = FPDB_COL_BLOB_INFO;
      }
      // If no data, return 0, else if no ptr, just get length, 
      // else if we have data in the buffer and it's less than the offset...
      if (!m_ulReadSize)
      {
         ulReturn = (ULONG)0;
      }
      else if (ulChunkOffset < m_ulReadSize)
      {
         if (!lplData)         // just get len (from offset)
            ulReturn = m_ulReadSize - ulChunkOffset;
         else
         {
            UINT uSrcCharSize;
            UINT uDestCharSize;

            ulReturn = this->GetBlobDataValue( lpCursor, &tmpLBlock, 
                         ulChunkOffset, ulChunkSize);
            // Get char sizes for converting CHAR types. First source
            uSrcCharSize = sizeof(BYTE);
            if ((m_ulType == VT_LPWSTR) || (m_ulType == DBTYPE_WCHARS))
              uSrcCharSize = sizeof(WORD);
#ifdef WIN32
            if (m_ulType == VT_BSTR)  // for WIN32, VT_BSTR (Memo) gives the data
              uSrcCharSize = sizeof(WORD); // as WCHARs
#endif
            // Get char sizes for destination
            if ((m_ulType == DBTYPE_BYTES) || (m_ulType == DBTYPE_BLOB))
              uDestCharSize = sizeof(BYTE);
            else
              uDestCharSize = sizeof(TCHAR);

            // convert buffer to desired type (LPBYTE or LPWORD)
            if (tmpLBlock)
            {
              LBlockConvertCharType( &tmpLBlock, ulReturn, 
                uSrcCharSize, uDestCharSize);
              if ((usVBDataType == DT_HSZ) && tmpLBlock)
              {
                 *lplData = (LONG)fpDBCreateHsz((LPTSTR)tmpLBlock);
                 LBlockFree(tmpLBlock);
              }
              else   
                 *lplData = (LONG)tmpLBlock;
            }
         }
      }
      break; //VT_BLOB

    case VT_DATE:                  // convert DATE type to user's choice
        ulReturn = (ULONG)sizeof(DATE); // should be 8 bytes
        if (lplData)               // if value is wanted
        {
           if ((usVBDataType == DT_HSZ) || (usVBDataType == DT_HLSTR))
           {
             VARIANTARG varData1, varData2;
             VariantInit(&varData1);
             varData1.vt = VT_DATE;
             varData1.date = *(DATE FAR *)lpszData;
             VariantInit(&varData2);
             h = VariantChangeType(&varData2, &varData1, (USHORT)0, VT_BSTR);
             ASSERT(h == S_OK);         // Could not convert DATE to BSTR!
             if ((h == S_OK) && varData2.bstrVal)
             {
               if (usVBDataType == DT_HSZ)
               {
                  LPTSTR tstr = fpSysAllocTStringFromBSTR(varData2.bstrVal);
                  if (tstr)
                  {
                    *lplData = (LONG)fpDBCreateHszLen((LPTSTR)tstr, 
                                       fpSysTStringLen(tstr));
                    fpSysFreeTString(tstr);
                  }
               }
               else
                  *lplData = (LONG)LBlockAllocXLenToXLen( varData2.bstrVal,
                    SysStringLen(varData2.bstrVal), sizeof(OLECHAR), sizeof(TCHAR));
               // free variant's BSTR
               SysFreeString(varData2.bstrVal);
             }
             else
                ulReturn = (ULONG)-1;
           }
           else if (usVBDataType == DT_FP_DATETIME)
           {
             // If control is requesting data as type DATE, get a double
             // value and return those 8 bytes in an allocated HLSTR.
             *lplData = (LONG)LBlockAllocLen(NULL, sizeof(DATE));
             // copy the double to the new buffer
             if (*lplData)
               *(DATE FAR *)*lplData = *(DATE FAR *)lpszData;
             else
               ulReturn = (ULONG)-1;
           }
           else
           {
             // Add support for converting DATE to YOUR type!
             ASSERT(0);
           }
        }
      break;  // VT_DATE

    default:
      // If requesting data as a REAL (float) and data field type is 
      // REAL8 (double), convert from double to float.
      if (usVBDataType == DT_REAL && dwDataType == DBTYPE_R8)
      {
        if (lplData)
          *(float FAR *)lplData = (float)*(double FAR *)lpszData;
        ulReturn = (ULONG)sizeof(float);
      }
      // Either requesting as a string OR data len is 4 bytes or less
      else if ((dwVarLen <= sizeof(LONG)) ||
          (usVBDataType == DT_HSZ) || (usVBDataType == DT_HLSTR))
      {
         ulReturn = (ULONG)dwVarLen;
         if (lplData)
         {
            if ((usVBDataType == DT_HSZ) || (usVBDataType == DT_HLSTR))
            {
              FPTSTR tstr;

              // If we need to "fix" the value read, use ulTmp as
              // the data buffer.
              if (FixReadValue(&ulTmp, lpszData, m_dwDataTypeUsed, dwVarLen))
                lpszData = (LPSTR)&ulTmp;
              tstr = (FPTSTR)fpConvertTypeToTSTR((LPVOID)lpszData, 
                               (USHORT)dwDataType);
              if (tstr)
              {
                if (usVBDataType == DT_HSZ)
                  *lplData = (LONG)fpDBCreateHsz(tstr);
                else
                  *lplData = (LONG)LBlockAllocXStrToXStr(tstr,
                               sizeof(TCHAR), sizeof(TCHAR));
                fpSysFreeTString(tstr);
              } //if tstr
            }
            else
            {
              // If we DON'T have to "fix" the value read, copy the data
              // into ulTmp.  NOTE: If the value gets "fixed", the value
              // is set in "ulTmp" so we don't have to copy it.
              if (!FixReadValue(&ulTmp, lpszData, m_dwDataTypeUsed, dwVarLen))
              {
                ulTmp = (ULONG)0;
                _fmemcpy( (LPVOID)&ulTmp, lpszData, (size_t)dwVarLen);
              }
              *(ULONG FAR *)lplData = ulTmp;
            }
         }
      }
  } //switch (m_ulType)

  return ulReturn;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::GetDataValueFromRDC() - Remote Data Control
 *
 * DESCRIPTION:
 *
 *   This function copies the data for the field from the "Fetch Buffer".
 *
 * PARAMETERS:
 *
 *   lpszFetchBuff .. Ptr to fetch buffer.
 *   usVBDataType ... Format for the returned data (DT_LONG, DT_HSZ, etc).
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
 *   FFFFFFFF ...... If error (-1) 
 *
 **************************************************************************/
ULONG CfpDBColumn::GetDataValueFromRDC( LPSTR lpszFetchInline, 
  LPSTR lpszFetchOutline, USHORT usVBDataType,
  LONG FAR *lplData, UINT FAR *lpuFlags, ICursor FAR *lpCursor)
{
  DWORD  dwInfo = 0;
  LPSTR  lpszData = NULL;
  DWORD  dwVarLen = (DWORD)-1;
  ULONG  ulReturn = (ULONG)-1;
  USHORT usType = 0;
  UINT   uSrcCharSize;
  UINT   uDestCharSize;
  LBLOCK tmpLBlock = NULL;
  char   buffer[16];   // 16 bytes should be enough for any binary value
  LPTSTR tstr = NULL;
  
  *lplData = 0;

  // Retrieve INFO from the Fetch buffer
  if (m_obInfo != DB_NOVALUE)
     dwInfo = *(DWORD FAR *)(lpszFetchInline + m_obInfo);

  // if the field value is null, set flag and return
  if ((dwInfo == DB_NULL) || (dwInfo == DB_EMPTY) || (m_obData == DB_NOVALUE))
  {
    *lpuFlags = DA_fNull;
    return (ULONG)0;
  }

  // get ptrs to data and its length
//dwDataOffset = *(DWORD FAR *)(lpszFetchInline + m_obData);
//lpszData = lpszFetchOutline + dwDataOffset;
  lpszData = *(LPSTR FAR *)(lpszFetchInline + m_obData);

  if (m_obVarDataLen != DB_NOVALUE)
     dwVarLen = *(DWORD FAR *)(lpszFetchInline + m_obVarDataLen);
  else
     dwVarLen = m_ulMaxLen;

  //$Rdc bug: Len includes the null-terminator, but it's not always there!
  if (dwVarLen > 0)
    --dwVarLen;

  // Set the source char size
  if (m_dwDataTypeUsed == VT_LPWSTR)
    uSrcCharSize = (ULONG)sizeof(WORD);
  else
    uSrcCharSize = (ULONG)sizeof(BYTE);

  // Set the destination char size (assuming TCHAR)
  if (m_ulType == DBTYPE_BYTES)
   uDestCharSize = sizeof(BYTE);
  else
   uDestCharSize = sizeof(TCHAR);

  // switch on VB data type
  switch (usVBDataType)
  {
    case DT_HLSTR:
    case DT_HSZ:
      if (lplData)
      {
         // copy from buffer TO LBlock, then, if HSZ, call fpDBCreateHsz()
         // so that the "types" match.  
         tmpLBlock = LBlockAllocXLenToXLen(lpszData,
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
      // Convert from string to type
      // NOTE: We assume the Source Char size matches TCHAR's size
      //       (see FillColBindRec()).

      switch (usVBDataType)
      {
        case DT_SHORT:  usType = VT_I2;  break;
        case DT_LONG:   usType = VT_I4;  break;
        case DT_BOOL:   usType = VT_I2;  break;
        case DT_REAL:   usType = VT_R4;  break;
        case DT_FP_DATETIME: usType = VT_DATE;  break;
        default:        usType = VT_I4;  break;
      }
      // string may not be null-terminated, create new string if not
      if (lpszData[dwVarLen] != '\0')
      {
        tstr = fpSysAllocTStringByteLen(lpszData, dwVarLen);
        lpszData = (LPSTR)tstr;
      }
      ulReturn = fpConvertTSTRtoType( (LPTSTR)lpszData, &buffer, usType);
      if (lplData)
      {
        if ((ulReturn <= sizeof(LONG)) && (ulReturn > 0))
        {
          _fmemcpy( lplData, buffer, (size_t)ulReturn);
        }
        // if control wants data as DT_FP_DATETIME, allocate HLSTR and
        // copy double (8 bytes) to that HLSTR.
        else if (usVBDataType == DT_FP_DATETIME)
        {
          *lplData = (LONG)LBlockAllocLen(NULL, sizeof(DATE));
          // copy the double to the new buffer
          if (*lplData)
            *(DATE FAR *)*lplData = *(DATE FAR *)buffer;
          else
            ulReturn = (ULONG)-1;
        }
        else
          ulReturn = (ULONG)-1;
      }
      if (tstr)
        fpSysFreeTString(tstr);
      break; //default:

  } //switch (usVBDataType)

  return ulReturn;
} //GetDataValueFromRDC()


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::FreeFetchData()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
void CfpDBColumn::FreeFetchData( LPSTR lpszFetchBuff)
{
  DBVARIANT FAR *lpdbVar;
  DWORD dwDataType;
  LPSTR lpszData;
  DWORD dwInfo;
  DWORD dwVarLen;

  // if already freed, return
  if (m_fNewRead <= FPDB_COL_FETCHFREED)
    return;

  dwInfo = DB_NOINFO;
  // Retrieve INFO from the Fetch buffer
  if (m_obInfo != DB_NOVALUE)
    dwInfo = *(DWORD FAR *)(lpszFetchBuff + m_obInfo);

  // if the field value is not null, free the bstr
  if ((dwInfo == DB_NULL) || (dwInfo == DB_EMPTY) ||
       (m_obData == DB_NOVALUE))
    return;

  dwDataType = m_dwDataTypeUsed;
  lpszData = lpszFetchBuff + m_obData;

  // If DBVARIANT was used, set lpszData to data in DBVARIANT structure.
  if (m_dwBindTypeUsed == DBBINDING_VARIANT)
  {
    lpdbVar = (DBVARIANT FAR *)lpszData;
    GetDBVariantData(lpdbVar, &dwDataType, (LPVOID FAR *)&lpszData, &dwVarLen);
    if (lpszData == NULL)
      dwDataType = DBTYPE_NULL;
  }

  // if type VT_BSTR and we have data, free the BSTR
  if (dwDataType == VT_BSTR)
  {
    BSTR FAR *pbstr;

    if (m_dwBindTypeUsed == DBBINDING_VARIANT)
      pbstr = (BSTR FAR *)&lpszData;
    else
      pbstr = (BSTR FAR *)lpszData;

    if (*pbstr)
      SysFreeString(*pbstr);
    *pbstr = NULL;
  }

  m_fNewRead = FPDB_COL_FETCHFREED;

  return;
} //FreeFetchData()


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::PostFetchSetup()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
void CfpDBColumn::PostFetchSetup( LPSTR lpszFetchBuff)
{
  // set-up ptrs to BLOB's
  if (m_dwBindTypeUsed == DBBINDING_ENTRYID)
  {
     if (m_obData == DB_NOVALUE)
        m_lpBlob = NULL;
     else
        m_lpBlob = (BLOB FAR *)(lpszFetchBuff + m_obData);

     m_ulReadSize = 0;         // set later if needed
  }

  // set flag
  m_fNewRead = FPDB_COL_POSTFETCH;

  return;
} //PostFetchSetup()


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::BlobGetInterface()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Returns ptr to IStream if successful, else NULL
 *
 *   NOTE:  BE SURE TO CALL "lpIStream->Release()" WHEN FINISHED!!!
 *
 **************************************************************************/
IStream FAR *CfpDBColumn::BlobGetInterface( ICursor FAR *lpCursor)
{
  IEntryID FAR *lpIEntryID;
  IStream  FAR *lpIStream;
  HRESULT h;

  if (!m_lpBlob)
     return NULL;

  // Get interface ptrs
  h = lpCursor->QueryInterface( IID_IEntryID, (LPVOID FAR *)&lpIEntryID);
  ASSERT(h == S_OK); // Error, cursor has no IEntryID interface!!!
  if ((h != S_OK) || !lpIEntryID)
  {
     lpIEntryID = NULL;
  }
  else
  {
     h = lpIEntryID->GetInterface( m_lpBlob->cbSize, 
           m_lpBlob->pBlobData, 0, IID_IStream, (IUnknown **)&lpIStream);
     ASSERT(h == S_OK); // Error, cannot get IStream interface from IEntryID!!!
     if ((h != S_OK) || !lpIStream)
        lpIStream = NULL;
  }

  // release interface to IEntryID
  if (lpIEntryID)
    lpIEntryID->Release();

  return lpIStream;
} // BlobGetInterface()

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::BlobGetWriteInterface()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Returns ptr to IStream if successful, else NULL
 *
 *   NOTE:  BE SURE TO CALL "lpIStream->Release()" WHEN FINISHED!!!
 *
 **************************************************************************/
// *** Following code was dumped because of a MICROSOFT-admitted bug:  ***
// *** GetColumn() is suppose to return a NEW EntryId when adding      ***
// *** a new row; however NO EntryId is returned!!!                    ***
/*
IStream FAR *CfpDBColumn::BlobGetWriteInterface( 
  ICursorUpdateARow FAR *lpUpdateCursor)
{
  IEntryID FAR *lpIEntryID;
  IStream  FAR *lpIStream;
  HRESULT h;
  DBBINDPARAMS BindParams;
  HLSTR    tmpbuff = NULL;

  if (m_lpWriteBlob)
  {
     tmpbuff = m_lpWriteBlob;
     fpDBDestroyHlstr(tmpbuff);
     m_lpWriteBlob = NULL;
  }

  // Binding method and DataType
  BindParams.cbMaxLen     = 100;
  BindParams.dwBinding    = DBBINDING_ENTRYID;
  BindParams.dwDataType   = DBTYPE_BLOB; 
  BindParams.cbVarDataLen = DB_NOVALUE;
  BindParams.dwInfo       = (DWORD)0;
  tmpbuff = fpDBCreateHlstr(NULL, 100);      // allocate a 100 bytes
  _fmemset( tmpbuff, 0, 100);
  BindParams.pData = tmpbuff; 

  h = lpUpdateCursor->GetColumn( &m_ColID, &BindParams, NULL);
  ASSERT(h == S_OK);
  
  if (h == S_OK)
  {
     m_lpWriteBlob = (LPBLOB)BindParams.pData;

     // Get interface ptrs
     h = lpUpdateCursor->QueryInterface( IID_IEntryID, (LPVOID FAR *)&lpIEntryID);
     ASSERT(h == S_OK); // Error, cursor has no IEntryID interface!!!
     if ((h != S_OK) || !lpIEntryID)
     {
        lpIEntryID = NULL;
     }
     else
     {
        h = lpIEntryID->GetInterface( m_lpWriteBlob->cbSize, 
              m_lpWriteBlob->pBlobData, 0, IID_IStream, (IUnknown **)&lpIStream);
        ASSERT(h == S_OK); // Error, cannot get IStream interface from IEntryID!!!
        if ((h != S_OK) || !lpIStream)
           lpIStream = NULL;
     }
  }

  // release interface to IEntryID
  if (lpIEntryID)
    lpIEntryID->Release();

  // if no Blob ptr but allocated buffer, free buffer
  if ((m_lpWriteBlob == NULL) && tmpbuff)
     fpDBDestroyHlstr(tmpbuff);

  return lpIStream;
} // BlobGetWriteInterface()
*/


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::BlobQuerySize()
 *
 * DESCRIPTION:
 *
 *   Gets information about the BLOB just read.  If successful, it
 *   sets member variables.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpDBColumn::BlobQuerySize(ICursor FAR *lpCursor)
{
  // If the read is new OR the BLOB ptr is NULL, return error
  IStream  FAR *lpIStream;
  BOOL     fReturn = FALSE;
  HRESULT  h;

  // if no blob ptr, just return
  if (!m_lpBlob)
     return FALSE;

  lpIStream = BlobGetInterface(lpCursor);
  if (lpIStream)
  {
     LARGE_INTEGER  li;
     ULARGE_INTEGER lu;
     LISet32(li, 0);
     h = lpIStream->Seek(li, (DWORD)STREAM_SEEK_END, &lu);
     ASSERT(h == S_OK); // Error, cannot get Info about Stream
     if (h == S_OK)
     {
       // check for overflow
       if (lu.HighPart)
          m_ulReadSize = ULONG_MAX;
       else
          m_ulReadSize = lu.LowPart;

       fReturn = TRUE;
     }

     // free interface
     lpIStream->Release();
  }  // if lpIStream

  return fReturn;
} // BlobSetStatistics


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::GetBlobDataValue()
 *
 * DESCRIPTION:
 *
 *   Gets data value for VT_BLOB/DBTYPE_BLOB types.
 *
 * PARAMETERS:
 *
 *   lpCursor ...... Ptr to Cursor.  Used when retrieving BLOB values.
 *
 * RETURNS:
 *
 **************************************************************************/
ULONG CfpDBColumn::GetBlobDataValue( ICursor FAR *lpCursor, 
  LPVOID hlstrRetData, ULONG ulOffset, ULONG ulSize)
{
  IStream  FAR *lpIStream = NULL;
  LARGE_INTEGER li;
  ULONG ulLenToRead;
  ULONG ulLenRead;
  ULONG ulReturn = (ULONG)-1;
  HLSTR hlstr;
  HRESULT h;

  // get interface ptr
  lpIStream = this->BlobGetInterface(lpCursor);
  if (!lpIStream)
     return ulReturn;

  // Read data from interface
  if (m_ulReadSize && (ulOffset < m_ulReadSize))
  {
     LISet32(li, ulOffset);
     h = lpIStream->Seek(li, (DWORD)STREAM_SEEK_SET, NULL);
     ASSERT(h == S_OK); // Error, IStream cannot be set to beginning!!!
     if (h == S_OK)
     {
       // if size to read is 0 or negative, read all.
       ulLenToRead = ( ulSize < 1 ? (m_ulReadSize - ulOffset) : ulSize);
// Restrict read to nnnk
#ifdef WIN32
#define BLOB_READ_MAX  (ULONG)(512 * 1024)  // 512k max (Big!)
#else
#define BLOB_READ_MAX  (ULONG)0xFFF0        // 64k - 16 bytes for overhead
#endif
       if (ulLenToRead > BLOB_READ_MAX)
          ulLenToRead = BLOB_READ_MAX;
       // buffer for read
       hlstr = LBlockAllocLen( NULL, ulLenToRead);
       // read data into buffer
       h = lpIStream->Read( hlstr, ulLenToRead, &ulLenRead); 
       ASSERT(h == S_OK);
       // resize buffer if necessary
       if (h == S_OK)
       {
          if (ulLenRead != ulLenToRead)
             LBlockReAllocLen( (LBLOCK FAR *)&hlstr, NULL, ulLenRead);
          *(HLSTR FAR *)hlstrRetData = hlstr;
          ulReturn = ulLenRead;
       }
       else
       {  // read failed, free buffer
          LBlockFree((LBLOCK)hlstr);
       }
     }  // if seek OK
  } // if (size of data)

  lpIStream->Release();
  
  return ulReturn;
} // GetBlobDataValue() 

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::WriteDataValue()
 *
 * DESCRIPTION:
 *
 *   Gets data value for VT_BLOB/DBTYPE_BLOB types.
 *
 * PARAMETERS:
 *
 *   lpCursor ...... Ptr to Cursor.  Used when retrieving BLOB values.
 *
 * RETURNS:
 *
 **************************************************************************/
ULONG CfpDBColumn::WriteDataValue( ICursor FAR *lpCursor, 
  LPVOID hlstrData, ULONG ulOffset, ULONG ulSize)
{
  IStream  FAR *lpIStream = NULL;
  LARGE_INTEGER li;
  ULARGE_INTEGER lu;
  ULONG ulLenToWrite = 0;
  ULONG ulLenWrite;
  ULONG ulReturn = (ULONG)-1;
  HRESULT h;

  // get interface ptr
  lpIStream = this->BlobGetInterface(lpCursor);
  if (!lpIStream)
     return ulReturn;

  // Seek to Offset
  LISet32(li, ulOffset);
  h = lpIStream->Seek(li, (DWORD)STREAM_SEEK_SET, NULL);
  ASSERT(h == S_OK); // Error, Cannot seek IStream to offset
  if (h == S_OK)
  {
     // if size to write is 0 or negative, write all.
     ulLenToWrite = (ulSize < 1 ? (ULONG)LBlockLen((LBLOCK)hlstrData) : ulSize);
     // write data into IStream
     h = lpIStream->Write( hlstrData, ulLenToWrite, &ulLenWrite); 
     ASSERT(h == S_OK);
     // Set new size
     if (h == S_OK)
     {
/* //SetSize Work-a-round: currently, IStream::SetSize() is NOT SUPPORTED!!!
        ASSERT(ulLenWrite == ulLenToWrite);  // difference in counts
        LISet32( lu, (ulOffset + ulLenWrite));
        h = lpIStream->SetSize(lu);
        ASSERT(h == S_OK);                   // Error setting IStream size
*/
        // test if WRITE was successful
        LISet32(li, 0);
        h = lpIStream->Seek(li, (DWORD)STREAM_SEEK_SET, NULL);
        ULISet32(lu, 0);
        h = lpIStream->Seek(li, (DWORD)STREAM_SEEK_END, &lu);
        h = lpIStream->Commit(STGC_OVERWRITE);
        ulReturn = ulLenWrite;
     }
  }  // if seek OK

  lpIStream->Release();
  
  return ulReturn;
} // WriteDataValue() 


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::SetDataValue()
 *
 * DESCRIPTION:
 *
 *   This function fills the DBBINDPARAMS structure used by 
 *   ICursorUpdateARow::SetColumn().  
 *
 * PARAMETERS:
 *
 *   lpBindParams ... Ptr to DBBINDPARAMS structure.
 *   usVBDataType ... Format for the returned data (DT_LONG, DT_HSZ, etc).
 *   ulChunkOffset .. For types DBTYPE_CHAR and VT_BLOB, gets chunk at offset.
 *   ulChunkSize .... Sizeof chunk to get.
 *   lData  ......... Either the value or ptr to string. 
 *   uFlags ......... Flags (indicates NULL value)
 *   lpCursor ....... Ptr to Cursor.  Used when referencing BLOB values.
 *
 * RETURNS:
 *
 *   0 if successful, else ERROR
 *
 **************************************************************************/
LONG CfpDBColumn::SetDataValue( DBBINDPARAMS FAR *lpBindParams, 
  USHORT usVBDataType, ULONG ulChunkOffset, ULONG ulChunkSize,
  LONG lData, UINT uFlags, ICursorUpdateARow FAR *lpUpdateCursor,
  BOOL FAR *lpbMemAllocated)
{
  LPSTR  lpszData = NULL;
  LONG  lReturn = (LONG)0;
  USHORT usDBtype;
  LONG   lSize = 0;
  char   buffer[20];
  BOOL   bAllocateMem = FALSE;
  BLOB   blob;                             
  BSTR   bstr = (BSTR)NULL;
  
  *lpbMemAllocated = FALSE;

  // Max len as retrieved from Meta Data
  lpBindParams->cbMaxLen = m_ulMaxLen;
  
  // Binding method and DataType
  lpBindParams->dwBinding  = m_dwBindTypeUsed;     // ENTRY_ID or DEFAULT
  lpBindParams->dwDataType = m_dwDataTypeUsed; 

  lpBindParams->dwInfo = DB_NOINFO;
  // Are we setting it to NULL?
  if ((uFlags & DA_fNull) && (m_dwBindTypeUsed == DBBINDING_DEFAULT))
  {
     lpBindParams->dwInfo = DB_NULL;
     return (LONG)0;                          // Bind is ready, return
  }                           
  // The two remaining fields in lpBindParams (cbVarDataLen & pData)
  // depend on the data's type and value.
  // Convert from usVBDataType to DB's type

  usDBtype = LOWORD(GetVBDataType());
  // if AutoIncrement...
  if (m_uFlags & DB_AUTOINCRFIELD)
  {
    // Cannot update field, it is SET when added and can only be deleted!
    //fReturn = FALSE;
    lReturn = (LONG)FPDB_ERR_CANTSETAUTOINCR;
  }
  // else if we are given a string AND the target type is not a string,
  // convert to binary type.
  else if (((usVBDataType == DT_HLSTR) || (usVBDataType == DT_HSZ)) &&
      ((usDBtype != DATA_VT_MEMO) && (usDBtype != DATA_VT_TEXT) &&
       (usDBtype != DATA_VT_BINARY)))
  {
    lSize = fpConvertTSTRtoType( (LPTSTR)lData, &buffer, 
              (USHORT)m_dwDataTypeUsed);
    //ASSERT(lSize > 0);    // Could not convert from string to type!!!
    if (lSize <= 0)
       lReturn = (LONG)FPDB_ERR_CONVERTSTRINGTOTYPE;
    else
    {   
       lpszData = &buffer[0];
       bAllocateMem = TRUE;
    }
  } 
  // else if some type of "fixed-len binary type" and not given a string
  else if ((usDBtype != DATA_VT_MEMO) && (usDBtype != DATA_VT_TEXT) &&
           (usDBtype != DATA_VT_BINARY))
  {
//  lpBindParams->dwInfo = DB_NULL;
    lSize = 0L;
    switch (usVBDataType)
    {
      case DT_SHORT:  lSize = sizeof(short);  break; // 0x02
      case DT_BOOL :  lSize = sizeof(BOOL);   break; // 0x04
      case DT_LONG :  lSize = sizeof(long);   break; // 0x03
      case DT_REAL :  lSize = sizeof(float);  break; // 0x07
      case DT_FP_DATETIME: lSize = sizeof(DATE);  break; // 0x0F
      default:
        ASSERT(0);    //Add support!!!
        lReturn = (LONG)FPDB_ERR_NOTIMPLEMENTED;
    }
    // ptr to data
    if (lSize > 0)
    {
       // If lSize is greater than 4 bytes, treat lData as ptr to the
       // value, else lData itself is the value.
       lpszData = (lSize > 4 ? (LPSTR)lData: (LPSTR)&lData);
       bAllocateMem = TRUE;
    }
  }
  // else we have a variable-len type field and are using "Default" binding
  else if (m_dwBindTypeUsed != DBBINDING_ENTRYID)
  {                                        
    if (usVBDataType == DT_HLSTR)
       lSize = LBlockLen((LBLOCK)lData);
    else
       lSize = (LONG)fpSysTStringByteLen((FPTSTR)lData);
    if (ulChunkSize && ((ULONG)lSize > ulChunkSize))
       lSize = (LONG)ulChunkSize;

    // NOTE: If WCHARS, we will use type VT_BSTR because SetColumn() returns 
    // an error, "E_OUTOFMEMORY", when we use WCHARS.
    if (m_dwDataTypeUsed == DBTYPE_WCHARS)
      lpBindParams->dwDataType = VT_BSTR;

#ifndef WIN32  // for WIN16, use BSTR
    lpBindParams->dwDataType = VT_BSTR;
#endif

    if (lpBindParams->dwDataType == VT_BSTR)
    {
      if (lData)
      {
        // Since this may be a chunk of data, null terminate it, allocate,
        // then restore original char.
        UINT uIndex = lSize / sizeof(TCHAR);
        TCHAR saveChar = ((LPTSTR)lData)[uIndex];
        ((LPTSTR)lData)[uIndex] = (TCHAR)0;
        bstr = (BSTR)fpSysAllocBSTRFromTString((LPTSTR)lData);
        ((LPTSTR)lData)[uIndex] = saveChar;
      }
      lpBindParams->dwDataType = VT_BSTR;                
      // allocate 4 bytes to hold ptr to BSTR
      lSize = sizeof(BSTR);
      lpszData = (LPSTR)LBlockAllocLen( (LPBYTE)&bstr, sizeof(BSTR));
      *lpbMemAllocated = TRUE;
    }
    else // not VT_BSTR
    {   
      lpszData = (LPSTR)lData;
    }
  } 
  // else using ENTRY_ID type binding
  else 
  {
/* //SetSize Work-a-round:
    ULONG ulLenWritten;
    HLSTR hlstr = (HLSTR)lData;
    if (uFlags & DA_fNull)
    {
       hlstr = fpDBCreateHlstr( "", 0);
       ulChunkOffset = 0; 
       ulChunkSize = 0;
    }
    ulLenWritten = WriteDataValue( (ICursor FAR *)lpUpdateCursor, hlstr, 
                     ulChunkOffset, ulChunkSize);
    if (ulLenWritten < ulChunkSize)
       fReturn = FALSE;

    if (uFlags & DA_fNull)
       fpDBDestroyHlstr( hlstr);
    lpszData = (LPSTR)(m_lpBlob->pBlobData);
    lSize = (LONG)m_lpBlob->cbSize;
*/
    //SetSize...
    if (uFlags & DA_fNull)
    {
       blob.cbSize = 0;
       blob.pBlobData = NULL;
    }
    else
    {
       if (usVBDataType == DT_HLSTR)
          blob.cbSize = (ULONG)LBlockLen((LBLOCK)lData);
       else
          blob.cbSize = (ULONG)fpSysTStringByteLen((LPTSTR)lData);
       blob.pBlobData = (BYTE FAR *)lData;
    }
    lpszData = (LPSTR)&blob;
    lSize = sizeof(blob);
    bAllocateMem = TRUE;
    lpBindParams->dwBinding  = DBBINDING_DEFAULT;     // ENTRY_ID or DEFAULT
    lpBindParams->dwDataType = VT_BLOB; 
    //SetSize
  }

  // set data parameters
  if ((lSize > 0) && lpszData)
  {
     if (bAllocateMem)
     {
        lpBindParams->pData = LBlockAllocLen(NULL, lSize);
        ASSERT(lpBindParams->pData);  // Could not alloc block of this size!!!
        if (lpBindParams->pData)
        {
           _fmemcpy(lpBindParams->pData, lpszData, (size_t)lSize); 
           *lpbMemAllocated = TRUE;
        }
        else
        {
           lReturn = (LONG)FPDB_ERR_LBLOCKALLOC;
        }
     }
     else 
        lpBindParams->pData = lpszData;

     lpBindParams->cbVarDataLen = lSize;
  }

  return lReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::~CfpDBColumn()
 *
 * DESCRIPTION:
 *
 *   Destructor
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBColumn::~CfpDBColumn()
{
  HLSTR hlstr;

  if (m_lpWriteBlob)
  {
     hlstr = m_lpWriteBlob;
     LBlockFree((LBLOCK)hlstr);
  }
  if (m_strDataField)
    fpSysFreeTString(m_strDataField);
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::FixReadValue()
 *
 * DESCRIPTION:
 *
 *   Fixes inconsistencies with OLE DB.  There are a few problems, see
 *   the following code for the list of problems.
 *
 * PARAMETERS:
 *
 * RETURNS:  Returns TRUE if a fix was performed, else FALSE.
 *
 **************************************************************************/
BOOL CfpDBColumn::FixReadValue(LONG FAR *lplDataDest, LPSTR lpszDataSrc, 
  DWORD dwDataType, DWORD dwLenRead)
{
  // if VT_I4 but didn't read 4 bytes, convert to 4 bytes.
  if ((dwDataType == VT_I4) && (dwLenRead != sizeof(LONG)))
  {
    // I have not seen this case (len = 1) but we'll code for it anyways.
    if (dwLenRead == sizeof(char))
      *lplDataDest = (long)*(char FAR *)lpszDataSrc;  // given len = 1
    else
      *lplDataDest = (long)*(short FAR *)lpszDataSrc; // given len >= 2
    return TRUE;
  }

  // if VT_I2 but didn't read 2 bytes, convert to 2 bytes.
  if ((dwDataType == VT_I2) && (dwLenRead == sizeof(char)))
  {
    *lplDataDest = (long)*(BYTE FAR *)lpszDataSrc;  // given len = 1
    return TRUE;
  }

  // If given 4 bytes for 2 byte field, use only 2 bytes.
  if ((dwLenRead > sizeof(short)) &&
      ((dwDataType == VT_BOOL) || (dwDataType == VT_I2)))
  {
    *lplDataDest = (long)*(short FAR *)lpszDataSrc; // given len > 2, use 2.
    return TRUE;
  }

  return FALSE;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::GetDBVariantData()
 *
 * DESCRIPTION:
 *
 *   If using DBVARIANT to retrieve data, return the "data type" and
 *   a ptr to the data in the DBVARIANT structure.
 *
 * PARAMETERS:
 *
 * RETURNS:  Returns TRUE if successful
 *
 **************************************************************************/
BOOL CfpDBColumn::GetDBVariantData(DBVARIANT FAR *lpdbVar, 
  DWORD FAR *dwRetDataType, LPVOID FAR *lpvRetData, DWORD FAR *dwRetVarLen)
{
  *lpvRetData = NULL;

  if (!lpdbVar)
    return FALSE;
 
  *dwRetDataType = (DWORD)lpdbVar->vt;

  switch((DWORD)lpdbVar->vt)
  {
    case DBTYPE_EMPTY:
    case DBTYPE_NULL:
      break;            // leave ptr null

    case VT_BSTR:       // 8
    case DBTYPE_LPSTR:  // 30
    case DBTYPE_LPWSTR: // 31
    case DBTYPE_BYTES:  // 508
    case DBTYPE_CHARS:  // 509
    case DBTYPE_WCHARS: // 510
      *lpvRetData = (LPVOID)(lpdbVar->pszVal);
	  if (lpdbVar->pszVal)
		*dwRetVarLen = (lpdbVar->vt == VT_BSTR ? 
           SysStringByteLen(lpdbVar->bstrVal) : _fstrlen(lpdbVar->pszVal));
      break;

    case VT_CY:
      *lpvRetData = (LPVOID)&(lpdbVar->cyVal);
      *dwRetVarLen = sizeof(CY);
	  break;

    case VT_DECIMAL: // We are getting this type from Oracle when using RDC.
      *lpvRetData = (LPVOID)&(((VARIANT FAR *)lpdbVar)->decVal);
      *dwRetVarLen = sizeof(DECIMAL);
	  break;
      
    default:            // this includes: VT_I2, VT_I4, VT_BLOB, etc.
      *lpvRetData = (LPVOID)&(lpdbVar->lVal);
      *dwRetVarLen = sizeof(long);
  }

  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::SetDataValueForRDC()
 *
 * DESCRIPTION:
 *
 *   This function fills the DBBINDPARAMS structure used by 
 *   ICursorUpdateARow::SetColumn().  
 *
 * PARAMETERS:
 *
 *   lpBindParams ... Ptr to DBBINDPARAMS structure.
 *   usVBDataType ... Format for the returned data (DT_LONG, DT_HSZ, etc).
 *   ulChunkOffset .. For types DBTYPE_CHAR and VT_BLOB, gets chunk at offset.
 *   ulChunkSize .... Sizeof chunk to get.
 *   lData  ......... Either the value or ptr to string. 
 *   uFlags ......... Flags (indicates NULL value)
 *   lpCursor ....... Ptr to Cursor.  Used when referencing BLOB values.
 *
 * RETURNS:
 *
 *   0 if successful, else ERROR
 *
 **************************************************************************/
LONG CfpDBColumn::SetDataValueForRDC( DBBINDPARAMS FAR *lpBindParams, 
  USHORT usVBDataType, /*ULONG ulChunkOffset, ULONG ulChunkSize, */
  LONG lData, UINT uFlags, ICursorUpdateARow FAR *lpUpdateCursor,
  BOOL FAR *lpbMemAllocated)
{
  LONG lResult;
  DBVARIANT dbvar;
  SAFEARRAY safeArr;
  BOOL fSafeArrUsed = FALSE;
  BLOB FAR *lpBlob;

  lResult = SetDataValue( lpBindParams, usVBDataType, 0L, 0L,
    lData, uFlags, lpUpdateCursor, lpbMemAllocated);

  // if SetDataValue() returned an error, then return the error.
  if (lResult != 0L)
    return lResult;

  lpBindParams->dwBinding  = DBBINDING_VARIANT;

  // Move result into a DBVariant structure.  This structure should be
  // allocated with the rest of the data, so re-allocate the block of
  // memory and move the current data to the end of the block, then 
  // treat the beginning of the block as the DBVariant.  In the DBVariant,
  // store the data type and a ptr to the data or the value itself.

  if (lpBindParams->pData || (uFlags & DA_fNull))
  {
    ULONG ulNewSize;
    ULONG ulReallocSize = 0L;
    LPSTR lpRealloc = (*lpbMemAllocated ? (LPSTR)lpBindParams->pData : NULL);

    VariantInit((VARIANTARG FAR *)&dbvar);
    dbvar.vt = (uFlags & DA_fNull ? VT_NULL : (VARTYPE)lpBindParams->dwDataType);

    switch (lpBindParams->dwDataType)
    {
       case VT_BSTR:
         dbvar.bstrVal = *(BSTR *)lpBindParams->pData;
         break;
       case DBTYPE_I2:
       case DBTYPE_UI2:
         dbvar.iVal = *(short FAR *)lpBindParams->pData;
         break;
       case DBTYPE_I4:
       case DBTYPE_UI4:
         dbvar.lVal = *(long FAR *)lpBindParams->pData;
         break;
       case DBTYPE_R4:
         dbvar.fltVal = *(float FAR *)lpBindParams->pData;
         break;
       case DBTYPE_BOOL:
         dbvar.xbool = *(VARIANT_BOOL FAR *)lpBindParams->pData;
         break;
       case DBTYPE_R8:
         dbvar.dblVal = *(double FAR *)lpBindParams->pData;
         break;
       case DBTYPE_CY:
         dbvar.cyVal = *(CY FAR *)lpBindParams->pData;
         break;
       case DBTYPE_DATE:
         dbvar.date = *(DATE FAR *)lpBindParams->pData;
         break;
       case DBTYPE_WCHARS:
       case DBTYPE_CHARS:
         dbvar.pszVal = (LPSTR)lpBindParams->pData;
         break;
       case DBTYPE_BLOB:
         // for BLOB, copy data from old blob into dbvar.blob .
         lpBlob = (BLOB FAR *)lpBindParams->pData;
         safeArr.cDims = 1;
         safeArr.fFeatures = FADF_EMBEDDED;
         safeArr.cbElements = sizeof(BYTE);
         safeArr.cLocks = 0;

         safeArr.pvData = (void HUGEP *)lpBlob->pBlobData;
         safeArr.rgsabound[0].cElements = lpBlob->cbSize;
         safeArr.rgsabound[0].lLbound = 0;
         fSafeArrUsed = TRUE;
         ulReallocSize = sizeof(safeArr);
//       dbvar.vt = VT_ARRAY + VT_UI1;
         dbvar.vt = VT_SAFEARRAY;
         break;
       default:  // VT_NULL
         break;  
    }
    // create new block
    ulNewSize = sizeof(dbvar) + ulReallocSize;
    if (lpRealloc)
      LBlockReAllocLen((LBLOCK FAR *)&lpBindParams->pData, NULL, ulNewSize);
    else
      lpBindParams->pData = LBlockAllocLen(NULL, ulNewSize);
    ASSERT(lpBindParams->pData);
    if (!lpBindParams->pData)
      return -1L;
    // move previous data "down" by DBVARIANT to make room for that struct.
    if (lpRealloc && ulReallocSize)
    {
      // if SafeArray was used, copy it into the buffer and make dbvar.parray
      // point to it.
      if (fSafeArrUsed)
      {
        // set ptr to new safe array in block
        dbvar.parray = 
          (SAFEARRAY FAR *) ((LPSTR)lpBindParams->pData + sizeof(dbvar));
        _fmemcpy(dbvar.parray, &safeArr, sizeof(safeArr));
      }
      else // else not safearray, copy old data at beginning of block to the end.
        _fmemmove((LPSTR)lpBindParams->pData + sizeof(dbvar),
                  lpBindParams->pData, (size_t)ulReallocSize);
    }
    // copy struct to beginning of block.
    _fmemcpy(lpBindParams->pData, &dbvar, sizeof(dbvar));
    *lpbMemAllocated = TRUE;
  }

  return 0L;
}


