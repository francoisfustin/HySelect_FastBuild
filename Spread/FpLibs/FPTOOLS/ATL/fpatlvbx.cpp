/****************************************************************************
* FPATLVBX.CPP  -  Implementation of FarPoint's ATL Functions for 
*                  converting from VBX project to OCX.
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

#include "fpatlvbx.h"

#include <malloc.h>
#include <ole2.h>
#include <oleauto.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define new DEBUG_NEW

#define TRACE0  ATLTRACE

#define FPATL_FONTNAME_INDEX        0 
#define FPATL_FONTBOLD_INDEX        1
#define FPATL_FONTITALIC_INDEX      2
#define FPATL_FONTSIZE_INDEX        3
#define FPATL_FONTSTRIKETHRU_INDEX  4
#define FPATL_FONTUNDERLINE_INDEX   5
#define FPATL_FONTPROP_CT           6

static TCHAR *fontprops[FPATL_FONTPROP_CT] = {
_T("FontName"),
_T("FontBold"),
_T("FontItalic"),
_T("FontSize"),
_T("FontStrikethru"),
_T("FontUnderline") };


static HRESULT fpLoadFontPropsetTofpStream(CfpPropertySection* ppsec,
  LPSTREAM pStmDest, SHORT sFontPropIndex);


CfpPropString::operator LPCTSTR() const
{
  return (LPCTSTR)m_tstr;
}

CfpPropString::CfpPropString()
  : CfpString((LPTSTR)NULL)
{
  m_dwValue = 0;
}

CfpPropString::CfpPropString(LPCTSTR lpsz)
  : CfpString((LPTSTR)lpsz)
{
  m_dwValue = 0;
}

CfpPropString::CfpPropString(LPCTSTR key, DWORD dwValue)
  : CfpString((LPTSTR)key)
{
  m_dwValue = dwValue;
}

int CfpPropString::GetLength() const
{
  int iRet = 0;

  if (m_tstr)
    iRet = _tcslen(m_tstr);

  return iRet;
}

BOOL CfpMapStringToPtr::Lookup(LPCTSTR key, DWORD& rValue) const
{
  int ct = GetSize();
  int i;
  BOOL fFound = FALSE;
  CfpPropString *pPropStr;
  LPCTSTR pPropName;

  for (i = 0; i < ct; i++)
  {
    pPropStr = (CfpPropString *)GetAt(i);  // get prop object
    pPropName = (pPropStr ? (LPCTSTR)*pPropStr : NULL); // get prop name
    // if not null AND first char in name matches key AND strings match...
    if (pPropName && *pPropName == *key && _tcscmp(pPropName, key) == 0)
    {
      rValue = pPropStr->m_dwValue;  // return stored value
      fFound = TRUE;
      break; // end loop
    }
  }  

  return fFound;
}

BOOL CfpMapStringToPtr::Add(LPCTSTR key, DWORD dwValue)
{
  return CObArray::Add(new CfpPropString(key, dwValue));
}
  
POSITION CfpMapStringToPtr::GetStartPosition() const
{
  // if count is 0, return 0, else return "first index" (1)
  return (POSITION)(GetSize() ? 0 : 1);
}

void CfpMapStringToPtr::GetNextAssoc(POSITION& rNextPosition,
  CfpPropString& rKey, DWORD& rValue) const
{
  // Note: rNextPosition is actually "index + 1"
  int ct = GetSize();
  int index = (int)rNextPosition - 1;
  
  if (!rNextPosition || index >= ct)
  {
    rNextPosition = (POSITION)0;
    return;
  }

  rKey = *(CfpPropString *)GetAt(index);
  rValue = (rKey ? rKey.m_dwValue : 0);

  rNextPosition = 
    (POSITION)((int)rNextPosition < ct ? ((int)rNextPosition + 1) : 0);
}


/////////////////////////////////////////////////////////////////////////////
// Helper functions used by CfpProperty, CfpPropertySection, CfpPropertySet classes

static LPVOID CountPrefixedStringA(LPCSTR lpsz)
{
	DWORD cb = (lstrlenA(lpsz) + 1);
	LPDWORD lp = (LPDWORD)malloc((int)cb + sizeof(DWORD));
	if (lp)
	{
		*lp = cb;
		lstrcpyA((LPSTR)(lp+1), lpsz);
	}

	return (LPVOID)lp;
}


#ifndef OLE2ANSI
static LPVOID CountPrefixedStringW(LPCWSTR lpsz)
{
	DWORD cb = (wcslen(lpsz) + 1);
	LPDWORD lp = (LPDWORD)malloc((int)cb * sizeof(WCHAR) + sizeof(DWORD));
	if (lp)
	{
		*lp = cb;
		wcscpy((LPWSTR)(lp+1), lpsz);
	}

	return (LPVOID)lp;
}
#endif

#ifdef _UNICODE
#define CountPrefixedStringT CountPrefixedStringW
#else
#define CountPrefixedStringT CountPrefixedStringA
#endif


#ifdef _UNICODE

#define MAX_STRLEN 1024

static LPBYTE ConvertStringProp(LPBYTE pbProp, DWORD dwType, ULONG nReps,
	size_t cbCharSize)
{
	LPBYTE pbResult = NULL; // Return value
	ULONG cbResult = 0;     // Number of bytes in pbResult
	LPBYTE pbBuffer;        // Temporary holding space
	ULONG cchOrig;          // Number of characters in original string
	ULONG cchCopy;          // Number of characters to copy
	ULONG cbCopy;           // Number of bytes to copy
	LPBYTE pbResultNew;     // Used for realloc of pbResult

	pbBuffer = (LPBYTE)malloc(MAX_STRLEN * cbCharSize);
	if (pbBuffer == NULL)
		return NULL;

	// If it's a vector, the count goes first.
	if (dwType & VT_VECTOR)
	{
		pbResult = (LPBYTE)malloc(sizeof(DWORD));
		if (pbResult == NULL)
		{
			free(pbBuffer);
			return NULL;
		}
		*(LPDWORD)pbResult = nReps;
		cbResult = sizeof(DWORD);
	}

	while (nReps--)
	{
		cchOrig = *(LPDWORD)pbProp;
		pbProp += sizeof(DWORD);

		// Convert multibyte string to Unicode.
		if (cbCharSize == sizeof(WCHAR))
		{
			cchCopy = _mbstowcsz((LPWSTR)pbBuffer, (LPSTR)pbProp,
				min(cchOrig, MAX_STRLEN));
		}
		else
		{
			cchCopy = _wcstombsz((LPSTR)pbBuffer, (LPWSTR)pbProp,
				min(cchOrig, MAX_STRLEN));
		}

		// Allocate space to append string.
		cbCopy = cchCopy * cbCharSize;
		pbResultNew = (LPBYTE)realloc(pbResult, cbResult + sizeof(DWORD) +
			cbCopy);

		// If allocation failed, cleanup and return NULL;
		if (pbResultNew == NULL)
		{
			free(pbResult);
			free(pbBuffer);
			return NULL;
		}

		pbResult = pbResultNew;

		// Copy character count and converted string into place,
		// then update the total size.
		memcpy(pbResult + cbResult, (LPBYTE)&cchCopy, sizeof(DWORD));
		memcpy(pbResult + cbResult + sizeof(DWORD), pbBuffer, cbCopy);
		cbResult += sizeof(DWORD) + cbCopy;

		// Advance to the next vector element
		pbProp += (cchOrig * cbCharSize);
	}

	free(pbBuffer);
	return pbResult;
}

#endif // _UNICODE

/////////////////////////////////////////////////////////////////////////////
// Implementation of the CfpProperty class

CfpProperty::CfpProperty()
{
	m_dwPropID = 0;

	m_dwType = VT_EMPTY;
	m_pValue = NULL;       // must init to NULL
}

CfpProperty::CfpProperty(DWORD dwID, const LPVOID pValue, DWORD dwType)
{
	m_dwPropID = dwID;
	m_dwType = dwType;
	m_pValue = NULL;       // must init to NULL
	Set(dwID, pValue, dwType);
}

CfpProperty::~CfpProperty()
{
	FreeValue();
}

BOOL CfpProperty::Set(DWORD dwID, const LPVOID pValue, DWORD dwType)
{
	m_dwType = dwType;
	m_dwPropID = dwID;

	return Set(pValue);
}

BOOL CfpProperty::Set(const LPVOID pValue, DWORD dwType)
{
	m_dwType = dwType;
	return Set(pValue);
}

BOOL CfpProperty::Set(const  LPVOID pVal)
{
	ULONG           cb;
	ULONG           cbItem;
	ULONG           cbValue;
	ULONG           nReps;
	LPBYTE          pCur;
	LPVOID          pValue = pVal;
	DWORD           dwType = m_dwType;
	LPVOID          pValueOrig = NULL;

	if (m_pValue != NULL)
	{
		FreeValue();
	}

	if (pValue == NULL || m_dwType == 0)
		return TRUE;

	// Given pValue, determine how big it is
	// Then allocate a new buffer for m_pValue and copy...
	nReps = 1;
	cbValue = 0;
	pCur = (LPBYTE)pValue;
	if (m_dwType & VT_VECTOR)
	{
		// The next DWORD is a count of the elements
		nReps = *(LPDWORD)pValue;
		cb = sizeof(nReps);
		pCur += cb;
		cbValue += cb;
		dwType &= ~VT_VECTOR;
	}
	else
	{
		// If we get any of the string-like types,
		// and we are not a vector create a count-prefixed
		// buffer.
		switch (dwType)
		{
			case VT_LPSTR:          // null terminated string
				pValueOrig = pValue;
				pValue = CountPrefixedStringA((LPSTR)pValueOrig);
				pCur = (LPBYTE)pValue;
				break;

			case VT_BSTR:           // binary string
			case VT_STREAM:         // Name of the stream follows
			case VT_STORAGE:        // Name of the storage follows
			case VT_STREAMED_OBJECT:// Stream contains an object
			case VT_STORED_OBJECT:  // Storage contains an object
				pValueOrig = pValue;
				pValue = CountPrefixedStringT((LPTSTR)pValueOrig);
				pCur = (LPBYTE)pValue;
				break;

#ifndef OLE2ANSI
			case VT_LPWSTR:         // UNICODE string
				pValueOrig = pValue;
				pValue = CountPrefixedStringW((LPWSTR)pValueOrig);
				pCur = (LPBYTE)pValue;
				break;
#endif
		}
	}

	// Since a value can be made up of a vector (VT_VECTOR) of
	// items, we first seek through the value, picking out
	// each item, getting it's size.
	//
	cbItem = 0;        // Size of the current item
	while (nReps--)
	{
		switch (dwType)
		{
			case VT_EMPTY:          // nothing
				cbItem = 0;
				break;

			case VT_I2:             // 2 byte signed int
			case VT_BOOL:           // True=-1, False=0
				cbItem = 2;
				break;

			case VT_I4:             // 4 byte signed int
			case VT_R4:             // 4 byte real
				cbItem = 4;
				break;

			case VT_R8:             // 8 byte real
			case VT_CY:             // currency
			case VT_DATE:           // date
			case VT_I8:             // signed 64-bit int
			case VT_FILETIME:       // FILETIME
				cbItem = 8;
				break;

			case VT_CLSID:          // A Class ID
				cbItem = sizeof(CLSID);
				break;

#ifndef _UNICODE
			case VT_BSTR:           // binary string
			case VT_STREAM:         // Name of the stream follows
			case VT_STORAGE:        // Name of the storage follows
			case VT_STREAMED_OBJECT:// Stream contains an object
			case VT_STORED_OBJECT:  // Storage contains an object
			case VT_STREAMED_PROPSET:// Stream contains a propset
			case VT_STORED_PROPSET: // Storage contains a propset
#endif // _UNICODE
			case VT_LPSTR:          // null terminated string
			case VT_BLOB_OBJECT:    // Blob contains an object
			case VT_BLOB_PROPSET:   // Blob contains a propset
			case VT_BLOB:           // Length prefixed bytes
			case VT_CF:             // Clipboard format
				// Get the DWORD that gives us the size, making
				// sure we increment cbValue.
				cbItem = *(LPDWORD)pCur;
				cb = sizeof(cbItem);
				pCur += cb;
				cbValue += cb;
				break;

#ifdef _UNICODE
			case VT_BSTR:           // binary string
			case VT_STREAM:         // Name of the stream follows
			case VT_STORAGE:        // Name of the storage follows
			case VT_STREAMED_OBJECT:// Stream contains an object
			case VT_STORED_OBJECT:  // Storage contains an object
			case VT_STREAMED_PROPSET:// Stream contains a propset
			case VT_STORED_PROPSET: // Storage contains a propset
#endif // _UNICODE
			case VT_LPWSTR:         // UNICODE string
				cbItem = *(LPDWORD)pCur * sizeof(WCHAR);
				cb = sizeof(cbItem);
				pCur += cb;
				cbValue += cb;
				break;

			default:
				if (pValueOrig)
					free(pValue);
				return FALSE;
		}

		// Seek to the next item
		pCur += cbItem;
		cbValue += cbItem;
	}

	if (NULL == AllocValue(cbValue))
	{
		TRACE0("CfpProperty::AllocValue failed");
		return FALSE;
	}
	memcpy(m_pValue, pValue, (int)cbValue);

	if (pValueOrig)
		free(pValue);

	return TRUE;
}

LPVOID CfpProperty::Get()
{   return Get((DWORD*)NULL);   }

LPVOID CfpProperty::Get(DWORD* pcb)
{
	DWORD   cb;
	LPBYTE  p = NULL;

	p = (LPBYTE)m_pValue;

	// m_pValue points to a Property "Value" which may
	// have size information included...
	switch (m_dwType)
	{
		case VT_EMPTY:          // nothing
			cb = 0;
			break;

		case VT_I2:             // 2 byte signed int
		case VT_BOOL:           // True=-1, False=0
			cb = 2;
			break;

		case VT_I4:             // 4 byte signed int
		case VT_R4:             // 4 byte real
			cb = 4;
			break;

		case VT_R8:             // 8 byte real
		case VT_CY:             // currency
		case VT_DATE:           // date
		case VT_I8:             // signed 64-bit int
		case VT_FILETIME:       // FILETIME
			cb = 8;
			break;

#ifndef _UNICODE
		case VT_BSTR:           // binary string
		case VT_STREAM:         // Name of the stream follows
		case VT_STORAGE:        // Name of the storage follows
		case VT_STREAMED_OBJECT:// Stream contains an object
		case VT_STORED_OBJECT:  // Storage contains an object
		case VT_STREAMED_PROPSET:// Stream contains a propset
		case VT_STORED_PROPSET: // Storage contains a propset
#endif // UNICODE
		case VT_LPSTR:          // null terminated string
		case VT_CF:             // Clipboard format
			// Read the DWORD that gives us the size, making
			// sure we increment cbValue.
			cb = *(LPDWORD)p;
			p += sizeof(DWORD);
			break;

		case VT_BLOB:           // Length prefixed bytes
		case VT_BLOB_OBJECT:    // Blob contains an object
		case VT_BLOB_PROPSET:   // Blob contains a propset
			// Read the DWORD that gives us the size.
			cb = *(LPDWORD)p;
			break;

#ifdef _UNICODE
		case VT_BSTR:           // binary string
		case VT_STREAM:         // Name of the stream follows
		case VT_STORAGE:        // Name of the storage follows
		case VT_STREAMED_OBJECT:// Stream contains an object
		case VT_STORED_OBJECT:  // Storage contains an object
		case VT_STREAMED_PROPSET:// Stream contains a propset
		case VT_STORED_PROPSET: // Storage contains a propset
#endif // _UNICODE
		case VT_LPWSTR:         // UNICODE string
			cb = *(LPDWORD)p * sizeof(WCHAR);
			p += sizeof(DWORD);
			break;

		case VT_CLSID:          // A Class ID
			cb = sizeof(CLSID);
			break;

		default:
			return NULL;
	}
	if (pcb != NULL)
		*pcb = cb;

	return p;
}

BOOL CfpProperty::Get(CComVariant *pvar)
{
	LPBYTE  p = NULL;
    BOOL fRet = TRUE;

	p = (LPBYTE)m_pValue;

    // clear variant
    pvar->Clear();

	// m_pValue points to a Property "Value" which may
	// have size information included...
	switch (m_dwType)
	{
		case VT_EMPTY:          // nothing
			break;

		case VT_I2:             // 2 byte signed int
		case VT_BOOL:           // True=-1, False=0
            *pvar = *(short*)p;
            pvar->vt = (USHORT)m_dwType;// set type
			break;

		case VT_I4:             // 4 byte signed int
            *pvar = *(long*)p;
			break;

		case VT_R4:             // 4 byte real
            *pvar = *(float*)p;
			break;

	    case VT_R8:
	    case VT_CY:
	    case VT_DATE:
		case VT_I8:             // signed 64-bit int
		case VT_FILETIME:       // FILETIME
            *pvar = *(double*)p;
            pvar->vt = (USHORT)m_dwType;// set type
			break;

		case VT_BSTR:           // binary string
		case VT_LPSTR:          // null terminated string
		case VT_CF:             // Clipboard format
			// Read the DWORD that gives us the size, making
			// sure we increment cbValue.
//			cb = *(LPDWORD)p;
			p += sizeof(DWORD);  // skip size
            *pvar = (LPCSTR)p;
			break;
/*
		case VT_BLOB:           // Length prefixed bytes
		case VT_BLOB_OBJECT:    // Blob contains an object
		case VT_BLOB_PROPSET:   // Blob contains a propset
			// Read the DWORD that gives us the size.
			cb = *(LPDWORD)p;
			break;
*/

		default:
            return FALSE;
	}

	return fRet;
}


DWORD  CfpProperty::GetType()
{   return m_dwType;  }

void   CfpProperty::SetType(DWORD dwType)
{   m_dwType = dwType; }

DWORD CfpProperty::GetID()
{   return m_dwPropID;   }

void CfpProperty::SetID(DWORD dwPropID)
{    m_dwPropID = dwPropID;   }

LPVOID CfpProperty::GetRawValue()
{   return m_pValue; }

/***************************************************************************
 *
 * FUNCTION:  CfpProperty::WriteToStream()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful
 *
 **************************************************************************/
BOOL CfpProperty::WriteToStream(IStream* pIStream)
{
	ULONG           cb;
	ULONG           cbTotal; // Total size of the whole value
	DWORD           dwType = m_dwType;
	DWORD           nReps;
	LPBYTE          pValue;
	LPBYTE          pCur;
	BOOL            bSuccess = FALSE;
	BYTE            b = 0;

	nReps = 1;
	pValue = (LPBYTE)m_pValue;
	pCur = pValue;
	cbTotal = 0;
	if (m_dwType & VT_VECTOR)
	{
		// Value is a DWORD count of elements followed by
		// that many repititions of the value.
		//
		nReps = *(LPDWORD)pCur;
		cbTotal = sizeof(DWORD);
		pCur += cbTotal;
		dwType &= ~VT_VECTOR;
	}

#ifdef _UNICODE
	switch (dwType)
	{
		case VT_BSTR:           // binary string
		case VT_STREAM:         // Name of the stream follows
		case VT_STORAGE:        // Name of the storage follows
		case VT_STREAMED_OBJECT:// Stream contains an object
		case VT_STORED_OBJECT:  // Storage contains an object
		case VT_STREAMED_PROPSET:// Stream contains a propset
		case VT_STORED_PROPSET: // Storage contains a propset
			pValue = ConvertStringProp(pCur, m_dwType, nReps, sizeof(char));
			if (m_dwType & VT_VECTOR)
				pCur = pValue + sizeof(DWORD);
			break;
	}
#endif // _UNICODE

	// Figure out how big the data is.
	while (nReps--)
	{
		switch (dwType)
		{
			case VT_EMPTY:          // nothing
				cb = 0;
				break;

			case VT_I2:             // 2 byte signed int
			case VT_BOOL:           // True=-1, False=0
				cb = 2;
				break;

			case VT_I4:             // 4 byte signed int
			case VT_R4:             // 4 byte real
				cb = 4;
				break;

			case VT_R8:             // 8 byte real
			case VT_CY:             // currency
			case VT_DATE:           // date
			case VT_I8:             // signed 64-bit int
			case VT_FILETIME:       // FILETIME
				cb = 8;
				break;

			case VT_LPSTR:          // null terminated string
			case VT_BSTR:           // binary string
			case VT_STREAM:         // Name of the stream follows
			case VT_STORAGE:        // Name of the storage follows
			case VT_STREAMED_OBJECT:// Stream contains an object
			case VT_STORED_OBJECT:  // Storage contains an object
			case VT_STREAMED_PROPSET:// Stream contains a propset
			case VT_STORED_PROPSET: // Storage contains a propset
			case VT_BLOB:           // Length prefixed bytes
			case VT_BLOB_OBJECT:    // Blob contains an object
			case VT_BLOB_PROPSET:   // Blob contains a propset
			case VT_CF:             // Clipboard format
				cb = sizeof(DWORD) + *(LPDWORD)pCur;
				break;

			case VT_LPWSTR:         // UNICODE string
				cb = sizeof(DWORD) + (*(LPDWORD)pCur * sizeof(WCHAR));
				break;

			case VT_CLSID:          // A Class ID
				cb = sizeof(CLSID);
				break;

			default:
				return FALSE;
		}

		pCur += cb;
		cbTotal+= cb;
	}

	// Write the type
	pIStream->Write((LPVOID)&m_dwType, sizeof(m_dwType), &cb);
	if (cb != sizeof(m_dwType))
		goto Cleanup;

	// Write the value
	pIStream->Write((LPVOID)pValue, cbTotal, &cb);
	if (cb != cbTotal)
		goto Cleanup;

	// Make sure we are 32 bit aligned
	cbTotal = (((cbTotal + 3) >> 2) << 2) - cbTotal;
	while (cbTotal--)
	{
		pIStream->Write((LPVOID)&b, 1, &cb);
		if (cb != sizeof(BYTE))
			goto Cleanup;
	}

	bSuccess = TRUE;

Cleanup:
	if (pValue != m_pValue)
		free(pValue);

	return bSuccess;
}

/***************************************************************************
 *
 * FUNCTION:  CfpProperty::ReadFromStream()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful
 *
 **************************************************************************/
BOOL CfpProperty::ReadFromStream(IStream* pIStream)
{
	ULONG           cb;
	ULONG           cbItem;
	ULONG           cbValue;
	DWORD           dwType;
	ULONG           nReps;
	ULONG           iReps;
	LPSTREAM        pIStrItem;
	LARGE_INTEGER   li;

	// All properties are made up of a type/value pair.
	// The obvious first thing to do is to get the type...
	pIStream->Read((LPVOID)&m_dwType, sizeof(m_dwType), &cb);
	if (cb != sizeof(m_dwType))
		return FALSE;

	dwType = m_dwType;
	nReps = 1;
	cbValue = 0;
	if (m_dwType & VT_VECTOR)
	{
		// The next DWORD in the stream is a count of the
		// elements
		pIStream->Read((LPVOID)&nReps, sizeof(nReps), &cb);
		if (cb != sizeof(nReps))
			return FALSE;
		cbValue += cb;
		dwType &= ~VT_VECTOR;
	}

	// Since a value can be made up of a vector (VT_VECTOR) of
	// items, we first seek through the value, picking out
	// each item, getting it's size.  We use a cloned
	// stream for this (pIStrItem).
	// We then use our pIStream to read the entire 'blob' into
	// the allocated buffer.
	//
	cbItem = 0;        // Size of the current item
	pIStream->Clone(&pIStrItem);
	ASSERT(pIStrItem != NULL);
	iReps = nReps;
	while (iReps--)
	{
		switch (dwType)
		{
			case VT_EMPTY:          // nothing
				cbItem = 0;
				break;

			case VT_I2:             // 2 byte signed int
			case VT_BOOL:           // True=-1, False=0
				cbItem = 2;
				break;

			case VT_I4:             // 4 byte signed int
			case VT_R4:             // 4 byte real
				cbItem = 4;
				break;

			case VT_R8:             // 8 byte real
			case VT_CY:             // currency
			case VT_DATE:           // date
			case VT_I8:             // signed 64-bit int
			case VT_FILETIME:       // FILETIME
				cbItem = 8;
				break;

			case VT_LPSTR:          // null terminated string
			case VT_BSTR:           // binary string
			case VT_STREAM:         // Name of the stream follows
			case VT_STORAGE:        // Name of the storage follows
			case VT_STREAMED_OBJECT:// Stream contains an object
			case VT_STORED_OBJECT:  // Storage contains an object
			case VT_STREAMED_PROPSET:// Stream contains a propset
			case VT_STORED_PROPSET: // Storage contains a propset
			case VT_BLOB:           // Length prefixed bytes
			case VT_BLOB_OBJECT:    // Blob contains an object
			case VT_BLOB_PROPSET:   // Blob contains a propset
			case VT_CF:             // Clipboard format
				// Read the DWORD that gives us the size, making
				// sure we increment cbValue.
				pIStream->Read((LPVOID)&cbItem, sizeof(cbItem), &cb);
				if (cb != sizeof(cbItem))
					return FALSE;
				LISet32(li, -(LONG)cb);
				pIStream->Seek(li, STREAM_SEEK_CUR, NULL);
				cbValue += cb;
				break;

			case VT_LPWSTR:         // UNICODE string
				pIStream->Read((LPVOID)&cbItem, sizeof(cbItem), &cb);
				if (cb != sizeof(cbItem))
					return FALSE;
				LISet32(li, -(LONG)cb);
				pIStream->Seek(li, STREAM_SEEK_CUR, NULL);
				cbValue += cb;
				cbItem *= sizeof(WCHAR);
				break;

			case VT_CLSID:          // A Class ID
				cbItem = sizeof(CLSID);
				break;

			default:
				pIStrItem->Release();
				return FALSE;
		}

		// Seek to the next item
		LISet32(li, cbItem);
		pIStrItem->Seek(li, STREAM_SEEK_CUR, NULL);
		cbValue += cbItem;
	}

	pIStrItem->Release();

#ifdef _UNICODE
	LPBYTE pTmp;

	switch (dwType)
	{
		case VT_BSTR:           // binary string
		case VT_STREAM:         // Name of the stream follows
		case VT_STORAGE:        // Name of the storage follows
		case VT_STREAMED_OBJECT:// Stream contains an object
		case VT_STORED_OBJECT:  // Storage contains an object
		case VT_STREAMED_PROPSET:// Stream contains a propset
		case VT_STORED_PROPSET: // Storage contains a propset
			pTmp = (LPBYTE)malloc((int)cbValue);
			pIStream->Read(pTmp, cbValue, &cb);
			m_pValue = ConvertStringProp(pTmp, m_dwType, nReps, sizeof(WCHAR));
			free(pTmp);
			break;

		default:
#endif // _UNICODE
			// Allocate cbValue bytes
			if (NULL == AllocValue(cbValue))
				return FALSE;

			// Read the buffer from pIStream
			pIStream->Read(m_pValue, cbValue, &cb);
			if (cb != cbValue)
				return FALSE;
#ifdef _UNICODE
			break;
	}
#endif // _UNICODE

	// Done!
	return TRUE;
}


LPVOID CfpProperty::AllocValue(ULONG cb)
{
	return m_pValue = malloc((int)cb);
}


void CfpProperty::FreeValue()
{
	if (m_pValue != NULL)
	{
		free(m_pValue);
		m_pValue = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Implementation of the CfpPropertySection class

CfpPropertySection::CfpPropertySection()
{
	m_FormatID = GUID_NULL;
	m_SH.cbSection = 0;
	m_SH.cProperties = 0;
}

CfpPropertySection::CfpPropertySection(CLSID FormatID)
{
	m_FormatID = FormatID;
	m_SH.cbSection = 0;
	m_SH.cProperties = 0;
}

CfpPropertySection::~CfpPropertySection()
{
	RemoveAll();
	return;
}

CLSID CfpPropertySection::GetFormatID()
{   return m_FormatID; }

void CfpPropertySection::SetFormatID(CLSID FormatID)
{   m_FormatID = FormatID; }

BOOL CfpPropertySection::Set(DWORD dwPropID, LPVOID pValue, DWORD dwType)
{
	CfpProperty* pProp = GetProperty(dwPropID);
	if (pProp == NULL)
	{
		if ((pProp = new CfpProperty(dwPropID, pValue, dwType)) != NULL)
			AddProperty(pProp);
		return (pProp != NULL);
	}

	pProp->Set(dwPropID, pValue, dwType);
	return TRUE;
}

BOOL CfpPropertySection::Set(DWORD dwPropID, LPVOID pValue)
{
	// Since no dwType was specified, the property is assumed
	// to exist.   Fail if it does not.
	CfpProperty* pProp = GetProperty(dwPropID);
	if (pProp != NULL && pProp->m_dwType)
	{
		pProp->Set(dwPropID, pValue, pProp->m_dwType);
		return TRUE;
	}
	else
		return FALSE;
}

LPVOID CfpPropertySection::Get(DWORD dwPropID)
{   return Get(dwPropID, (DWORD*)NULL);  }

LPVOID CfpPropertySection::Get(DWORD dwPropID, DWORD* pcb)
{
	CfpProperty* pProp = GetProperty(dwPropID);
	if (pProp)
		return pProp->Get(pcb);
	else
		return NULL;
}

void CfpPropertySection::Remove(DWORD dwID)
{
	POSITION pos = m_PropList.GetHeadPosition();
	CfpProperty* pProp;
	while (pos != NULL)
	{
		POSITION posRemove = pos;
		pProp = (CfpProperty*)m_PropList.GetNext(pos);
		if (pProp->m_dwPropID == dwID)
		{
			m_PropList.RemoveAt(posRemove);
			delete pProp;
			m_SH.cProperties--;
			return;
		}
	}
}

void CfpPropertySection::RemoveAll()
{
	POSITION pos = m_PropList.GetHeadPosition();
	while (pos != NULL)
		delete (CfpProperty*)m_PropList.GetNext(pos);
	m_PropList.RemoveAll();
	m_SH.cProperties = 0;
}


CfpProperty* CfpPropertySection::GetProperty(DWORD dwPropID)
{
	POSITION pos = m_PropList.GetHeadPosition();
	CfpProperty* pProp;
	while (pos != NULL)
	{
		pProp= (CfpProperty*)m_PropList.GetNext(pos);
		if (pProp->m_dwPropID == dwPropID)
			return pProp;
	}
	return NULL;
}

void CfpPropertySection::AddProperty(CfpProperty* pProp)
{
	m_PropList.AddTail(pProp);
	m_SH.cProperties++;
}

DWORD CfpPropertySection::GetSize()
{   return m_SH.cbSection; }

DWORD CfpPropertySection::GetCount()
{   return m_PropList.GetCount();  }

CfpPropObList* CfpPropertySection::GetList()
{   return &m_PropList;  }

BOOL CfpPropertySection::WriteToStream(IStream* pIStream)
{
	// Create a dummy property entry for the name dictionary (ID == 0).
	Set(0, NULL, VT_EMPTY);

	ULONG           cb;
	ULARGE_INTEGER  ulSeekOld;
	ULARGE_INTEGER  ulSeek;
	LPSTREAM        pIStrPIDO;
	PROPERTYIDOFFSET  pido;
	LARGE_INTEGER   li;

	// The Section header contains the number of bytes in the
	// section.  Thus we need  to go back to where we should
	// write the count of bytes
	// after we write all the property sets..
	// We accomplish this by saving the seek pointer to where
	// the size should be written in ulSeekOld
	m_SH.cbSection = 0;
	m_SH.cProperties = m_PropList.GetCount();
	LISet32(li, 0);
	pIStream->Seek(li, STREAM_SEEK_CUR, &ulSeekOld);

	pIStream->Write((LPVOID)&m_SH, sizeof(m_SH), &cb);
	if (sizeof(m_SH) != cb)
	{
		TRACE0("Write of section header failed (1).\n");
		return FALSE;
	}

	if (m_PropList.IsEmpty())
	{
		TRACE0("Warning: Wrote empty property section.\n");
		return TRUE;
	}

	// After the section header is the list of property ID/Offset pairs
	// Since there is an ID/Offset pair for each property and we
	// need to write the ID/Offset pair as we write each property
	// we clone the stream and use the clone to access the
	// table of ID/offset pairs (PIDO)...
	//
	pIStream->Clone(&pIStrPIDO);

	// Now seek pIStream past the PIDO list
	//
	LISet32(li,  m_SH.cProperties * sizeof(PROPERTYIDOFFSET));
	pIStream->Seek(li, STREAM_SEEK_CUR, &ulSeek);

	// Now write each section to pIStream.
	CfpProperty* pProp = NULL;
	POSITION pos = m_PropList.GetHeadPosition();
	while (pos != NULL)
	{
		// Get next element (note cast)
		pProp = (CfpProperty*)m_PropList.GetNext(pos);

		if (pProp->m_dwPropID != 0)
		{
			// Write it
			if (!pProp->WriteToStream(pIStream))
			{
				pIStrPIDO->Release();
				return FALSE;
			}
		}
		else
		{
			if (!WriteNameDictToStream(pIStream))
			{
				pIStrPIDO->Release();
				return FALSE;
			}
		}

		// Using our cloned stream write the Format ID / Offset pair
		// The offset to this property is the current seek pointer
		// minus the pointer to the beginning of the section
		pido.dwOffset = ulSeek.LowPart - ulSeekOld.LowPart;
		pido.propertyID = pProp->m_dwPropID;
		pIStrPIDO->Write((LPVOID)&pido, sizeof(pido), &cb);
		if (sizeof(pido) != cb)
		{
			TRACE0("Write of 'pido' failed\n");
			pIStrPIDO->Release();
			return FALSE;
		}

		// Get the seek offset after the write
		LISet32(li, 0);
		pIStream->Seek(li, STREAM_SEEK_CUR, &ulSeek);
	}

	pIStrPIDO->Release();

	// Now go back to ulSeekOld and write the section header.
	// Size of section is current seek point minus old seek point
	//
	m_SH.cbSection = ulSeek.LowPart - ulSeekOld.LowPart;

	// Seek to beginning of this section and write the section header.
	LISet32(li, ulSeekOld.LowPart);
	pIStream->Seek(li, STREAM_SEEK_SET, NULL);
	pIStream->Write((LPVOID)&m_SH, sizeof(m_SH), &cb);
	if (sizeof(m_SH) != cb)
	{
		TRACE0("Write of section header failed (2).\n");
		return FALSE;
	}

	// Now seek to end of of the now written section
	LISet32(li, ulSeek.LowPart);
	pIStream->Seek(li, STREAM_SEEK_SET, NULL);

	return TRUE;
}

BOOL CfpPropertySection::ReadFromStream(IStream* pIStream,
	LARGE_INTEGER liPropSet)
{
	ULONG               cb;
	PROPERTYIDOFFSET    pido;
	ULONG               cProperties;
	LPSTREAM            pIStrPIDO;
	ULARGE_INTEGER      ulSectionStart;
	LARGE_INTEGER       li;
	CfpProperty*          pProp;

	if (m_SH.cProperties || !m_PropList.IsEmpty())
		RemoveAll();

	// pIStream is pointing to the beginning of the section we
	// are to read.  First there is a DWORD that is the count
	// of bytes in this section, then there is a count
	// of properties, followed by a list of propertyID/offset pairs,
	// followed by type/value pairs.
	//
	LISet32(li, 0);
	pIStream->Seek(li, STREAM_SEEK_CUR, &ulSectionStart);
	pIStream->Read((LPVOID)&m_SH, sizeof(m_SH), &cb);
	if (cb != sizeof(m_SH))
		return FALSE;

	// Now we're pointing at the first of the PropID/Offset pairs
	// (PIDOs).   To get to each property we use a cloned stream
	// to stay back and point at the PIDOs (pIStrPIDO).  We seek
	// pIStream to each of the Type/Value pairs, creating CProperites
	// and so forth as we go...
	//
	pIStream->Clone(&pIStrPIDO);

	cProperties = m_SH.cProperties;
	while (cProperties--)
	{
		pIStrPIDO->Read((LPVOID)&pido, sizeof(pido), &cb);
		if (cb != sizeof(pido))
		{
			pIStrPIDO->Release();
			return FALSE;
		}

		// Do a seek from the beginning of the property set.
		LISet32(li, ulSectionStart.LowPart + pido.dwOffset);
		pIStream->Seek(liPropSet, STREAM_SEEK_SET, NULL);
		pIStream->Seek(li, STREAM_SEEK_CUR, NULL);

		// Now pIStream is at the type/value pair
		if (pido.propertyID != 0)
		{
			pProp = new CfpProperty(pido.propertyID, NULL, 0);
			pProp->ReadFromStream(pIStream);
			m_PropList.AddTail(pProp);
		}
		else
		{
			ReadNameDictFromStream(pIStream);
		}
	}

	pIStrPIDO->Release();

	return TRUE;
}

BOOL CfpPropertySection::GetID(LPCOLESTR pszName, DWORD* pdwPropID)
{
  LPTSTR tstr = NULL;
  BOOL fRet;

  if (pszName)
    tstr = fpSysAllocTStringFromBSTR((LPVOID)pszName);

  fRet = GetID((LPCTSTR)tstr, pdwPropID);

  if (tstr)
    fpSysFreeTString(tstr);  // free allocated string

  return fRet;
}


BOOL CfpPropertySection::GetID(LPCTSTR pszName, DWORD* pdwPropID)
{
	CfpPropString strName((LPTSTR)pszName);
	strName.MakeLower();        // Dictionary stores all names in lowercase

	DWORD pvID;
	if (m_NameDict.Lookup((LPCTSTR)strName, pvID))
	{
		*pdwPropID = (DWORD)pvID;
		return TRUE;
	}

	// Failed to find entry in dictionary
	return FALSE;
}

BOOL CfpPropertySection::SetName(DWORD dwPropID, LPCTSTR pszName)
{
	BOOL bSuccess = TRUE;
	CfpPropString strName((LPTSTR)pszName);

	DWORD pDummy;
    int bNameExists;

	strName.MakeLower();        // Dictionary stores all names in lowercase
	bNameExists = m_NameDict.Lookup((LPCTSTR)strName, pDummy);

	ASSERT(!bNameExists);  // Property names must be unique.

	if (bNameExists)
    {
		TRACE0("Failed to add entry to dictionary.\n");
		bSuccess = FALSE;
    }
	else
		m_NameDict.Add((LPCTSTR)strName, dwPropID);
	
	return bSuccess;
}

struct DICTENTRYHEADER
{
	DWORD dwPropID;
	DWORD cb;
};

struct DICTENTRY
{
	DICTENTRYHEADER hdr;
	char sz[256];
};

BOOL CfpPropertySection::ReadNameDictFromStream(IStream* pIStream)
{
	ULONG cb;
	ULONG cbRead = 0;

	// Read dictionary header (count).
	ULONG cProperties = 0;
	pIStream->Read((LPVOID)&cProperties, sizeof(cProperties), &cb);
	if (sizeof(cProperties) != cb)
	{
		TRACE0("Read of dictionary header failed.\n");
		return FALSE;
	}

	ULONG iProp;
	DICTENTRY entry;

	for (iProp = 0; iProp < cProperties; iProp++)
	{
		// Read entry header (dwPropID, cch).
		if (FAILED(pIStream->Read((LPVOID)&entry, sizeof(DICTENTRYHEADER),
			&cbRead)) ||
			(sizeof(DICTENTRYHEADER) != cbRead))
		{
			TRACE0("Read of dictionary entry failed.\n");
			return FALSE;
		}

		// Read entry data (name).

		cb = entry.hdr.cb;

		if (FAILED(pIStream->Read((LPVOID)&entry.sz, cb, &cbRead)) ||
			(cbRead != cb))
		{
			TRACE0("Read of dictionary entry failed.\n");
			return FALSE;
		}

		LPTSTR pszName;

#ifdef _UNICODE
		// Persistent form is always ANSI/DBCS.  Convert to Unicode.
		WCHAR wszName[256];
		_mbstowcsz(wszName, entry.sz, 256);
		pszName = wszName;
#else // _UNICODE
		pszName = entry.sz;
#endif // _UNICODE

		// Section's "name" appears first in list and has dwPropID == 0.
		if ((iProp == 0) && (entry.hdr.dwPropID == 0))
			m_strSectionName = pszName;             // Section name
		else
			SetName(entry.hdr.dwPropID, pszName);   // Some other property
	}

	return TRUE;
}

static BOOL WriteNameDictEntry(IStream* pIStream, DWORD dwPropID, CfpPropString& strName)
{
	ULONG cb;
	ULONG cbWritten = 0;
	DICTENTRY entry;

	entry.hdr.dwPropID = dwPropID;
	entry.hdr.cb = min(strName.GetLength() + 1, 255);
#ifdef _UNICODE
	// Persistent form is always ANSI/DBCS.  Convert from Unicode.
	_wcstombsz(entry.sz, (LPCWSTR)strName, 256);
#else // _UNICODE
	memcpy(entry.sz, (LPCSTR)strName, (size_t)entry.hdr.cb);
#endif // _UNICODE

	cb = sizeof(DICTENTRYHEADER) + entry.hdr.cb;

	if (FAILED(pIStream->Write((LPVOID)&entry, cb, &cbWritten)) ||
		(cbWritten != cb))
	{
		TRACE0("Write of dictionary entry failed.\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CfpPropertySection::WriteNameDictToStream(IStream* pIStream)
{
	ULONG cb;

	// Write dictionary header (count).
	ULONG cProperties = m_NameDict.GetSize() + 1;
	pIStream->Write((LPVOID)&cProperties, sizeof(cProperties), &cb);
	if (sizeof(cProperties) != cb)
	{
		TRACE0("Write of dictionary header failed.\n");
		return FALSE;
	}

	POSITION pos;
	CfpPropString strName;
	DWORD pvID;

	// Write out section's "name" with dwPropID == 0 first
	if (!WriteNameDictEntry(pIStream, 0, m_strSectionName))
		return FALSE;

	// Enumerate contents of dictionary and write out (dwPropID, cb, name).
	pos = m_NameDict.GetStartPosition();
	while (pos != NULL)
	{
		m_NameDict.GetNextAssoc(pos, strName, pvID);
		if (!WriteNameDictEntry(pIStream, pvID, strName))
			return FALSE;
	}

	return TRUE;
}

BOOL CfpPropertySection::SetSectionName(LPCTSTR pszName)
{
	m_strSectionName = pszName;
	return TRUE;
}

LPCTSTR CfpPropertySection::GetSectionName()
{
	return (LPCTSTR)m_strSectionName;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation of the CfpPropertySet class

CfpPropertySet::CfpPropertySet()
{
	m_PH.wByteOrder = 0xFFFE;
	m_PH.wFormat = 0;
	m_PH.dwOSVer = (DWORD)MAKELONG(LOWORD(GetVersion()), 2);
	m_PH.clsID =  GUID_NULL;
	m_PH.cSections = 0;

}

CfpPropertySet::CfpPropertySet(CLSID clsID)
{
	m_PH.wByteOrder = 0xFFFE;
	m_PH.wFormat = 0;
	m_PH.dwOSVer = (DWORD)MAKELONG(LOWORD(GetVersion()), 2);
	m_PH.clsID = clsID;
	m_PH.cSections = 0;
}

CfpPropertySet::~CfpPropertySet()
{   RemoveAll();  }

BOOL CfpPropertySet::Set(CLSID FormatID, DWORD dwPropID, LPVOID pValue, DWORD dwType)
{
	CfpPropertySection* pSect = GetSection(FormatID);
	if (pSect == NULL)
	{
		if ((pSect = new CfpPropertySection(FormatID)) != NULL)
			AddSection(pSect);
	}
	pSect->Set(dwPropID, pValue, dwType);
	return TRUE;
}

BOOL CfpPropertySet::Set(CLSID FormatID, DWORD dwPropID, LPVOID pValue)
{
	// Since there is no dwType, we have to assume that the property
	// already exists.  If it doesn't, fail.
	CfpPropertySection* pSect = GetSection(FormatID);
	if (pSect != NULL)
		return pSect->Set(dwPropID, pValue);
	else
		return FALSE;
}

LPVOID CfpPropertySet::Get(CLSID FormatID, DWORD dwPropID, DWORD* pcb)
{
	CfpPropertySection* pSect = GetSection(FormatID);
	if (pSect)
		return pSect->Get(dwPropID, pcb);
	else
		return NULL;
}

LPVOID CfpPropertySet::Get(CLSID FormatID, DWORD dwPropID)
{   return Get(FormatID, dwPropID, (DWORD*)NULL); }

void CfpPropertySet::Remove(CLSID FormatID, DWORD dwPropID)
{
	CfpPropertySection*  pSect = GetSection(FormatID);
	if (pSect)
		pSect->Remove(dwPropID);
}

void CfpPropertySet::Remove(CLSID FormatID)
{
	CfpPropertySection* pSect;
	POSITION posRemove = m_SectionList.GetHeadPosition();
	POSITION pos = posRemove;
	while (posRemove != NULL)
	{
		pSect = (CfpPropertySection*)m_SectionList.GetNext(pos);
		if (IsEqualCLSID(pSect->m_FormatID, FormatID))
		{
			m_SectionList.RemoveAt(posRemove);
			delete pSect;
			m_PH.cSections--;
			return;
		}
		posRemove = pos;
	}
}

void CfpPropertySet::RemoveAll()
{
	POSITION pos = m_SectionList.GetHeadPosition();
	while (pos != NULL)
	{
		delete (CfpPropertySection*)m_SectionList.GetNext(pos);
	}
	m_SectionList.RemoveAll();
	m_PH.cSections = 0;
}

CfpPropertySection* CfpPropertySet::GetSection(CLSID FormatID)
{
	POSITION pos = m_SectionList.GetHeadPosition();
	CfpPropertySection* pSect;
	while (pos != NULL)
	{
		pSect = (CfpPropertySection*)m_SectionList.GetNext(pos);
		if (IsEqualCLSID(pSect->m_FormatID, FormatID))
			return pSect;
	}
	return NULL;
}

CfpPropertySection* CfpPropertySet::AddSection(CLSID FormatID)
{
	CfpPropertySection* pSect = GetSection(FormatID);
	if (pSect)
		return pSect;

	pSect = new CfpPropertySection(FormatID);
	if (pSect)
		AddSection(pSect);
	return pSect;
}

void CfpPropertySet::AddSection(CfpPropertySection* pSect)
{
	m_SectionList.AddTail(pSect);
	m_PH.cSections++;
}

CfpProperty* CfpPropertySet::GetProperty(CLSID FormatID, DWORD dwPropID)
{
	CfpPropertySection* pSect = GetSection(FormatID);
	if (pSect)
		return pSect->GetProperty(dwPropID);
	else
		return NULL;
}

void CfpPropertySet::AddProperty(CLSID FormatID, CfpProperty* pProp)
{
	CfpPropertySection* pSect = GetSection(FormatID);
	if (pSect)
		pSect->AddProperty(pProp);
}

WORD CfpPropertySet::GetByteOrder()
{   return m_PH.wByteOrder;  }

WORD CfpPropertySet::GetFormatVersion()
{   return m_PH.wFormat;  }

void CfpPropertySet::SetFormatVersion(WORD wFmtVersion)
{   m_PH.wFormat = wFmtVersion;  }

DWORD CfpPropertySet::GetOSVersion()
{   return m_PH.dwOSVer;  }

void CfpPropertySet::SetOSVersion(DWORD dwOSVer)
{   m_PH.dwOSVer = dwOSVer;  }

CLSID CfpPropertySet::GetClassID()
{   return m_PH.clsID;  }

void CfpPropertySet::SetClassID(CLSID clsID)
{   m_PH.clsID = clsID;  }

DWORD CfpPropertySet::GetCount()
{   return m_SectionList.GetCount();  }

CfpPropObList* CfpPropertySet::GetList()
{   return &m_SectionList;  }


/***************************************************************************
 *
 * FUNCTION:  CfpPropertySet::WriteToStream()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful
 *
 **************************************************************************/
BOOL CfpPropertySet::WriteToStream(IStream* pIStream)
{
	LPSTREAM        pIStrFIDO;
	FORMATIDOFFSET  fido;
	ULONG           cb;
	ULARGE_INTEGER  ulSeek;
	LARGE_INTEGER   li;

	// Write the Property List Header
	m_PH.cSections = m_SectionList.GetCount();
	pIStream->Write((LPVOID)&m_PH, sizeof(m_PH), &cb);
	if (sizeof(m_PH) != cb)
	{
		TRACE0("Write of Property Set Header failed.\n");
		return FALSE;
	}

	if (m_SectionList.IsEmpty())
	{
		TRACE0("Warning: Wrote empty property set.\n");
		return TRUE;
	}

	// After the header is the list of Format ID/Offset pairs
	// Since there is an ID/Offset pair for each section and we
	// need to write the ID/Offset pair as we write each section
	// we clone the stream and use the clone to access the
	// table of ID/offset pairs (FIDO)...
	//
	pIStream->Clone(&pIStrFIDO);

	// Now seek pIStream past the FIDO list
	//
	LISet32(li, m_PH.cSections * sizeof(FORMATIDOFFSET));
	pIStream->Seek(li, STREAM_SEEK_CUR, &ulSeek);

	// Write each section.
	CfpPropertySection*   pSect = NULL;
	POSITION            pos = m_SectionList.GetHeadPosition();
	while (pos != NULL)
	{
		// Get next element (note cast)
		pSect = (CfpPropertySection*)m_SectionList.GetNext(pos);

		// Write it
		if (!pSect->WriteToStream(pIStream))
		{
			pIStrFIDO->Release();
			return FALSE;
		}

		// Using our cloned stream write the Format ID / Offset pair
		fido.formatID = pSect->m_FormatID;
		fido.dwOffset = ulSeek.LowPart;
		pIStrFIDO->Write((LPVOID)&fido, sizeof(fido), &cb);
		if (sizeof(fido) != cb)
		{
			TRACE0("Write of 'fido' failed.\n");
			pIStrFIDO->Release();
			return FALSE;
		}

		// Get the seek offset (for pIStream) after the write
		LISet32(li, 0);
		pIStream->Seek(li, STREAM_SEEK_CUR, &ulSeek);
	}

	pIStrFIDO->Release();

	return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpPropertySet::ReadFromStream()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful
 *
 **************************************************************************/
BOOL CfpPropertySet::ReadFromStream(IStream* pIStream)
{
	ULONG               cb;
	FORMATIDOFFSET      fido;
	ULONG               cSections;
	LPSTREAM            pIStrFIDO;
	CfpPropertySection*   pSect;
	LARGE_INTEGER       li;
	LARGE_INTEGER       liPropSet;

	// Save the stream position at which the property set starts.
	LARGE_INTEGER liZero = {0,0};
	pIStream->Seek(liZero, STREAM_SEEK_CUR, (ULARGE_INTEGER*)&liPropSet);

	if (m_PH.cSections || !m_SectionList.IsEmpty())
		 RemoveAll();

	// The stream starts like this:
	//  wByteOrder   wFmtVer   dwOSVer   clsID  cSections
	// Which is nice, because our PROPHEADER is the same!
	pIStream->Read((LPVOID)&m_PH, sizeof(m_PH), &cb);
	if (cb != sizeof(m_PH))
		return FALSE;

	// Now we're pointing at the first of the FormatID/Offset pairs
	// (FIDOs).   To get to each section we use a cloned stream
	// to stay back and point at the FIDOs (pIStrFIDO).  We seek
	// pIStream to each of the sections, creating CProperitySection
	// and so forth as we go...
	//
	pIStream->Clone(&pIStrFIDO);

	cSections = m_PH.cSections;
	while (cSections--)
	{
		pIStrFIDO->Read((LPVOID)&fido, sizeof(fido), &cb);
		if (cb != sizeof(fido))
		{
			pIStrFIDO->Release();
			return FALSE;
		}

		// Do a seek from the beginning of the property set.
		LISet32(li, fido.dwOffset);
		pIStream->Seek(liPropSet, STREAM_SEEK_SET, NULL);
		pIStream->Seek(li, STREAM_SEEK_CUR, NULL);

		// Now pIStream is at the type/value pair
		pSect = new CfpPropertySection;
		pSect->SetFormatID(fido.formatID);
		pSect->ReadFromStream(pIStream, liPropSet);
		m_SectionList.AddTail(pSect);
	}

	pIStrFIDO->Release();
	return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  fpLoadPropsetTofpStream()
 *
 * DESCRIPTION:
 *
 *   Load the properties from the PropSet (source stream) and copy 
 *   the values that we care about into the output stream (destination).
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
HRESULT __cdecl fpLoadPropsetTofpStream(LPSTREAM *pRetStmDest, LPSTREAM pStmSrc,
  ATL_PROPMAP_ENTRY* pMap, CfpAtlBase *pfpAtlCtl)
{
	USES_CONVERSION;
    HRESULT hr;
    SHORT dispid;
	CComVariant var;
    HGLOBAL hBlob = NULL;
    LPBLOB  lpBlob = NULL;
    int iPropId = -1;
    LPCTSTR tstrBlobProp = NULL;
    LPSTREAM pStmDest = *pRetStmDest;
    DWORD dwPropId;
    CfpProperty *pProp;

	CfpPropertySet pset;
	if (!pset.ReadFromStream(pStmSrc))
	{
		TRACE0("CPropertySet::ReadFromStream failed.\n");
		return FALSE;
	}

	CfpPropertySection* ppsec = pset.GetSection(CLSID_ConvertVBX);
	if (ppsec == NULL)
	{
		TRACE0("CLSID_PersistPropset section not found in property set.\n");
		return FALSE;
	}


    // If we have a stream, reset it.
    if (pStmDest)
    {
      LARGE_INTEGER li;

      LISet32(li, 0);
      hr = pStmDest->Seek(li, STREAM_SEEK_SET, NULL);
      _ASSERT(hr == S_OK);
      if (hr != S_OK)
      {
        pStmDest->Release();
        pStmDest = *pRetStmDest = NULL;
      }
    }
    // if no stream, create one.
    if (!pStmDest)
    {
      hr = CreateStreamOnHGlobal(NULL, TRUE, pRetStmDest);
      _ASSERT(hr == S_OK);
      if (hr != S_OK)
        return E_OUTOFMEMORY;
      pStmDest = *pRetStmDest;
    }

    // For each property in PropMap, read value from bag/stream and
    // write to Stream.
	for(int i = 0; pMap[i].pclsidPropPage != NULL; i++)
	{
	  if (pMap[i].szDesc == NULL)
		continue;

      // If this is the font property, try to read the font properties.
      if (_wcsicmp(pMap[i].szDesc, OLESTR("Font")) == 0)
      {
        // If the propset has a the font props, get font props.
        if (ppsec->GetID((LPCTSTR)"Fontname", &dwPropId))
          hr = fpLoadFontPropsetTofpStream(ppsec, pStmDest, (SHORT)pMap[i].dispid);

        continue;  // move on to next prop
      }

      // Lookup up property name in PropSet (actual Property section),
      // if not found, then this property was not in the form file,
      // just continue.
      if (!ppsec->GetID((LPCOLESTR)pMap[i].szDesc, &dwPropId))
		continue;  // prop not found, just continue with next property.

	  pProp = ppsec->GetProperty(dwPropId);
      if (!pProp)
        continue;

      // get property value
      if (pProp->Get(&var))
      {
        // write DISPID to stream and then property value.
        dispid = (SHORT)pMap[i].dispid;
        hr = pStmDest->Write(&dispid, sizeof(SHORT), NULL); // write index
		if (FAILED(hr))
		  break;
        hr = var.WriteToStream(pStmDest); // write prop value
		if (FAILED(hr))
		  break;
      } // if get 
	}

    // if successful and control uses blob data, get data and write to
    // stream.
    if (hr == S_OK && pfpAtlCtl->fpAtlHasBlobData(&iPropId, &tstrBlobProp)
        && tstrBlobProp)
    {
      ULONG len = 0;
      BYTE *pData;

      // get blob data
      if (ppsec->GetID((LPCTSTR)tstrBlobProp, &dwPropId) &&
          (pProp = ppsec->GetProperty(dwPropId)) &&
          (pData = (BYTE *)pProp->Get(&len)) && len)
      {
        // Read the data into an allocated blob.
        hBlob = GlobalAlloc(GMEM_MOVEABLE, len + sizeof(ULONG));
        if (hBlob == NULL)
          return CTL_E_OUTOFMEMORY;
        
        lpBlob = (LPBLOB)GlobalLock(hBlob);
        _ASSERT(lpBlob);
        if (!lpBlob)                    // if Lock failed, return error
          hr = CTL_E_OUTOFMEMORY;
        else                            // else proceed to retrieve the data.
        {
          // Read data into blob. (Note: pData contains the size)
          memcpy(lpBlob, pData, sizeof(ULONG) + len);
          GlobalUnlock(hBlob);
        }

        // if something failed and we have a Global handle, free the handle.
        if (hr != S_OK && hBlob)
        {
          GlobalFree(hBlob);
          hBlob = NULL;
        }
      }  

      if (hr == S_OK && hBlob)
      {
        dispid = FPATL_STM_BLOBPROP;
        hr = pStmDest->Write(&dispid, sizeof(SHORT), NULL); // write index
        if (hr == S_OK)
          pfpAtlCtl->fpAtlSaveBlob(pStmDest, hBlob);
      }
      if (hBlob)
        GlobalFree(hBlob);

      hr = S_OK;
    }

    // if successful, write 0xFFFF as index for "terminating" map index.
    if (hr == S_OK)
    {
      dispid = FPATL_STM_NULLPROP;
      hr = pStmDest->Write(&dispid, sizeof(SHORT), NULL); // write index
    }

	return hr;
}


/***************************************************************************
 *
 * FUNCTION:  fpLoadFontPropsetTofpStream()
 *
 * DESCRIPTION:
 *
 *   This loads all of the properties associated with a font from 
 *   a PropSet and writes it back out to a stream.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
static HRESULT fpLoadFontPropsetTofpStream(CfpPropertySection* ppsec,
  LPSTREAM pStmDest, SHORT sFontDispid)
{
  HRESULT hr = S_OK;
  CComVariant var;
  int iPropId = -1;
  DWORD dwPropId;
  CfpProperty *pProp;
  FONTDESC   fontDesc = {0};
  IFont     *pFontNew = NULL;
  IFontDisp *pDispFont = NULL;

    // initialize structure
    fontDesc.cbSizeofstruct = sizeof(fontDesc);
    fontDesc.sCharset = DEFAULT_CHARSET;
    fontDesc.sWeight = FW_NORMAL;
    fontDesc.cySize.Lo = 78000;

    // For each property in PropMap, read value from bag/stream and
    // write to Stream.
	for(int i = 0; i < FPATL_FONTPROP_CT; i++)
	{
      // Lookup up property name in PropSet (actual Property section),
      // if not found, then this property was not in the form file,
      // just continue.
      if (!ppsec->GetID(fontprops[i], &dwPropId))
		continue;  // prop not found, just continue with next property.

	  pProp = ppsec->GetProperty(dwPropId);
      if (!pProp)
        continue;

      // get property value
      if (pProp->Get(&var))
      {
        switch (i)
        {
          case FPATL_FONTNAME_INDEX:
            fontDesc.lpstrName = var.bstrVal;
            var.bstrVal = NULL;  // prevent str from being freed
            break;
          case FPATL_FONTBOLD_INDEX:
            fontDesc.sWeight = (var.boolVal ? FW_BOLD : FW_NORMAL);
            break;
          case FPATL_FONTITALIC_INDEX:
            fontDesc.fItalic = (var.boolVal ? TRUE : FALSE);
            break;
          case FPATL_FONTSIZE_INDEX:
            {
              double dblVal;
              switch (var.vt)
              {
                case VT_R4: dblVal = (double)var.fltVal; break;
                case VT_R8: dblVal = var.dblVal; break;
                case VT_I4: dblVal = (double)var.lVal; break;
                case VT_I2: dblVal = (double)var.iVal; break;
                default: dblVal = 7.8; // default value
              } // switch
            fontDesc.cySize.Lo = (long)(dblVal * 10000.0);
            }
            break;
          case FPATL_FONTSTRIKETHRU_INDEX:
            fontDesc.fStrikethrough = (var.boolVal ? TRUE : FALSE);
            break;
          case FPATL_FONTUNDERLINE_INDEX:
            fontDesc.fUnderline = (var.boolVal ? TRUE : FALSE);
            break;
        } // switch
      } // if get 
	} // for i

    // if no name, default it.
    if (!fontDesc.lpstrName)
      fontDesc.lpstrName = SysAllocString(OLESTR("MS Sans Serif"));

	if (OleCreateFontIndirect(&fontDesc, IID_IFont,
			(LPVOID *)&pFontNew) == S_OK && pFontNew)
    {
      // if we get a new IFontDisp ptr, write font to stream
	  if (pFontNew->QueryInterface(IID_IFontDisp, (void**)&pDispFont) == S_OK)
      {
        var = (IDispatch*)pDispFont; // put font in variant
        hr = pStmDest->Write(&sFontDispid, sizeof(SHORT), NULL); // write index
		if (hr == S_OK)
          hr = var.WriteToStream(pStmDest); // write prop value
        var.Clear(); // clear font before we release it
        pDispFont->Release(); // release from Query
      }
      pFontNew->Release();   // release from OleCreateFont, free font object
    }

    if (fontDesc.lpstrName)
      SysFreeString(fontDesc.lpstrName);

	return hr;
}


/***************************************************************************
 *
 * FUNCTION:  fpAtlVbxSaveFromfpStream()
 *
 * DESCRIPTION:
 *
 *   Given a [source] stream of properties (dispid-value pairs), 
 *   write-out to a [destination] stream all the "elements" needed
 *   for a "persist" stream.
 *
 *   The stream written-out as the following elements:
 *
 *     1. Version number (DWORD)
 *     2. Extents (2 LONG's for width & height).
 *     3. Multiple property dispid & value pairs.
 *     4. "Null" dispid to indicate end of dispid-value pairs.
 *
 * PARAMETERS:
 *
 * RETURNS:  S_OK if successful, otherwise some error code.
 *
 **************************************************************************/
HRESULT __cdecl fpAtlVbxSaveFromfpStream(LPSTREAM *ppstmSrc, LPSTREAM pStmDest,
  BOOL fClearDirty, DWORD dwCtlVersion, SIZE *psizeExtent, 
  CfpAtlBase *pfpAtlCtl)
{
  HRESULT hr;
  LARGE_INTEGER li;

  // Set the source stream ptr to the beginning.
  LISet32(li, 0);
  hr = (*ppstmSrc)->Seek(li, STREAM_SEEK_SET, NULL);
  _ASSERT(hr == S_OK);
  if (hr != S_OK)
    return S_OK;

  // Write-out (copy to stream) the version number and extents (size)
  hr = pStmDest->Write(&dwCtlVersion, sizeof(DWORD), NULL); // write version
  if (FAILED(hr))
    return hr;
  hr = pStmDest->Write(psizeExtent, sizeof(SIZE), NULL); // write extent
  if (FAILED(hr))
    return hr;

  // Write-out (copy to stream) the property/value pairs.
  hr = pfpAtlCtl->fpAtlCopyPropStream(pStmDest, *ppstmSrc);

  // if successful, write the "NULL terminate" property
  if (hr == S_OK)
  {
    SHORT dispid = FPATL_STM_NULLPROP;
    hr = pStmDest->Write(&dispid, sizeof(SHORT), NULL); // write index
  }

  return hr;
}


