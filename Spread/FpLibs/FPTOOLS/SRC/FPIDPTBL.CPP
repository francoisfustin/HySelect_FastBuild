/***********************************************************************
* FPIDPTBL.CPP
*
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#if (defined(FP_OCX) && !defined(NO_MFC))
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
//#include "ctlcore.h"
#include "stdafx.h"
#endif

#include "fpconvrt.h"
#include "fppinfo.h"
#include "fpidptbl.h"

#include <malloc.h>                      // declares _fmalloc(), etc.


// These defines are declared in FPPINFO.H.  They declare static structures
// as the ("name") prefixed with "_" (ex. _std_fontname)
SPROPINFO_STD_FONTNAME(std_fontname);  
SPROPINFO_STD_FONTBOLD(std_fontbold);  
SPROPINFO_STD_FONTITALIC(std_fontitalic);
SPROPINFO_STD_FONTSTRIKE(std_fontstrike);
SPROPINFO_STD_FONTUNDER(std_fontunder); 
SPROPINFO_STD_FONTSIZE(std_fontsize);  

#ifdef WIN32
static TCHAR fpidptbl_NullString[] = _T("");
#endif

// -----------------------------------------------------------------------
// Returns one of the following:
//
//   - SAME ptr if it's not a Standard Property (determined by "anding"
//       PPROPINFO_STD_LAST).
//   - Ptr to static structure if it's a Standard Property.
//   - NULL if no structure for this standard property.
//
static LPPROPINFO GetPropStruct(LPPROPINFO lpPropInfo)
{
  LPPROPINFO lpRet = NULL;

  // If not a standard property and address is valid...
  if (((DWORD)lpPropInfo & (DWORD)PPROPINFO_STD_LAST) != 
      (DWORD)PPROPINFO_STD_LAST)
    return lpPropInfo;

  // standard property, see if we have a static structure for it
  switch ((UINT)lpPropInfo)
  {
   case (UINT)PPROPINFO_STD_FONTNAME:   lpRet = &std_fontname;    break;
   case (UINT)PPROPINFO_STD_FONTBOLD:   lpRet = &std_fontbold;    break;
   case (UINT)PPROPINFO_STD_FONTITALIC: lpRet = &std_fontitalic;  break;
   case (UINT)PPROPINFO_STD_FONTSTRIKE: lpRet = &std_fontstrike;  break;
   case (UINT)PPROPINFO_STD_FONTUNDER:  lpRet = &std_fontunder;   break;
   case (UINT)PPROPINFO_STD_FONTSIZE:   lpRet = &std_fontsize;    break;
  }

  return lpRet;
}


// Constructor
CfpIDispatchTbl::CfpIDispatchTbl()
{
  Init();
}

/*
// Constructor which builds the IDispatchTable.  If an error
// occurs, "m_dispatchTbl" will be NULL.
CfpIDispatchTbl::CfpIDispatchTbl(FP_PROPLIST FAR *propList, 
  int iListCt, AFX_PMSG pfnDefGet, AFX_PMSG pfnDefSet, 
  const AFX_DISPMAP_ENTRY FAR *defDispatchMap, int iDefCt)
{
  long h;

  Init();
  h = BuildDispatchMap(propList, iListCt, pfnDefGet, pfnDefSet, 
        defDispatchMap, iDefCt);

  ASSERT(h == S_OK);
  if ((h != S_OK) && m_dispatchTbl)
  {
    _ffree(m_dispatchTbl);
    m_dispatchTbl = NULL;
  }

  return;
}
*/

// The "True" initializer (used by constructors)
void CfpIDispatchTbl::Init()
{
  m_dispatchTbl = NULL;
  m_iTblSizeCt = 0;            // number of entries allocated
  m_iLastEntryIndex  = -1;     // last entry in use
  m_iLastDispidIndex = -1;     // last entry with a valid DispId

  m_PropIdTbl = NULL;          // array of PropID's indexed by (DispID-1)
  m_PropIdTblCt = 0;           // number of entries in table 
}

// Destructor
CfpIDispatchTbl::~CfpIDispatchTbl()
{
  if (m_dispatchTbl)
    _ffree(m_dispatchTbl);
  if (m_PropIdTbl)
    _ffree(m_PropIdTbl);
}


// Allocates m_dispatchTbl to the specified number of entries.
long CfpIDispatchTbl::AllocDispatchTbl(int numOfEntries)
{
  size_t allocSize;
  BOOL   fRealloc = (m_dispatchTbl != NULL);

#ifndef NO_MFC
  ASSERT(numOfEntries > 0);
#else
  ASSERT(numOfEntries > 0, 0);
#endif

  if (numOfEntries <= 0)
    return STG_E_INVALIDPARAMETER;

  allocSize = numOfEntries * sizeof(AFX_DISPMAP_ENTRY);

  // if ptr is not already null, use realloc(), else use malloc()
  if (fRealloc)
    m_dispatchTbl = (AFX_DISPMAP_ENTRY FAR *)_frealloc(m_dispatchTbl, allocSize);
  else
    m_dispatchTbl = (AFX_DISPMAP_ENTRY FAR *)_fmalloc(allocSize);

  // if alloc failed, return error, else return S_OK.
  if (!m_dispatchTbl)
  {
    m_iTblSizeCt = 0;
    return STG_E_INSUFFICIENTMEMORY;
  }

  // Initialize new entries
  if (numOfEntries > m_iTblSizeCt)
  {
    _fmemset( &m_dispatchTbl[m_iTblSizeCt], 0, 
              (numOfEntries - m_iTblSizeCt) * sizeof(AFX_DISPMAP_ENTRY));
#ifdef WIN32
    // Initialize name to ptr to "".
    for (int i = m_iTblSizeCt; i < numOfEntries; i++)
      m_dispatchTbl[i].lpszName = (LPCTSTR)fpidptbl_NullString;
#endif      
  }

  m_iTblSizeCt = numOfEntries;
  return S_OK;
}

// Adds one FP_PROPLIST entry to the dispatch table.
long CfpIDispatchTbl::AddPropEntry(FP_PROPLIST FAR *pPropEntry, 
  WORD wPropID, AFX_PMSG pfnDefGet, AFX_PMSG pfnDefSet)
{
  AFX_DISPMAP_ENTRY tmpEntry = {0};
  WORD vt;
  long h = S_OK;
  int  index;
  LPPROPINFO lpPropInfo;

  // if empty dispatch table entry
  if (pPropEntry->wEnvironment & FP_NULLDPTBL)
  {
    index = (int)pPropEntry->sDispid;
    if (pPropEntry->sDispid > 0) // If DispID is greater than 0, index is DispId-1
      --index; 
    h = AddDispatchEntry(NULL, index);
    return h;
  }

  // if no dispID OR not Ocx prop OR no dispatch table entry, just return.
  if (pPropEntry->sDispid == 0 ||
      (pPropEntry->wEnvironment & FP_NO_OCX) ||
      (pPropEntry->wEnvironment & FP_NODPTBL))
    return h;

  // "pPropEntry->pPropInfo" could actually be a standard property flag;
  // this function will return either a structure ptr or NULL.
  lpPropInfo = GetPropStruct(pPropEntry->pPropInfo);
  if (!lpPropInfo)
    return S_OK;                  // no property info

  tmpEntry.lDispID = (long)pPropEntry->sDispid; //DISPID_UNKNOWN ?  // DISPID (may be DISPID_UNKNOWN)

#ifdef WIN32
  tmpEntry.lpszName = (LPCTSTR)lpPropInfo->npszName;
  tmpEntry.flags = afxDispCustom;     // flags (e.g. stock/custom)
  if (lpPropInfo->fl & PF_fPropArray)
    tmpEntry.lpszParams = (LPCSTR)VTS_I2; // member parameter description
#else //WIN16
  _fstrncpy(tmpEntry.szName, lpPropInfo->npszName, sizeof(tmpEntry.szName));
  if (lpPropInfo->fl & PF_fPropArray)
    _fstrcpy((LPSTR)tmpEntry.pbParams, VTS_I2);  // member parameter description
#endif

  vt = VT_EMPTY;    
  switch (lpPropInfo->fl & PF_datatype)
  {
    case DT_HSZ:     vt = VT_BSTR;         break;
    case DT_SHORT:   vt = VT_I2;           break;
    case DT_LONG:    vt = VT_I4;           break;
    case DT_BOOL:    vt = VT_BOOL;         break;
    case DT_COLOR:   vt = VT_COLOR;        break;
    case DT_ENUM:    vt = VT_I2;           break;
    case DT_REAL:    vt = VT_R4;           break;
    case DT_XPOS:    vt = VT_XPOS_PIXELS;  break;
    case DT_XSIZE:   vt = VT_XSIZE_PIXELS; break;
    case DT_YPOS:    vt = VT_YPOS_PIXELS;  break;
    case DT_YSIZE:   vt = VT_YSIZE_PIXELS; break;
    case DT_PICTURE: vt = VT_PICTURE;      break;
    case DT_HLSTR:   vt = VT_BSTR;         break;
    case DT_OBJECT:  vt = VT_DISPATCH;     break;
    case fpDT_FONT:  vt = VT_FONT;         break;
  }
  tmpEntry.vt = vt;                // return value type / or type of property
  tmpEntry.pfn = pfnDefGet;           // normal member On<membercall> or, OnGet<property>
  tmpEntry.pfnSet = pfnDefSet;        // special member for OnSet<property>
  tmpEntry.nPropOffset = (size_t)0;   // property offset

  index = (int)tmpEntry.lDispID - 1; 
  h = AddDispatchEntry((const AFX_DISPMAP_ENTRY FAR *)&tmpEntry, index);

  // Also, add to PropIdTbl lookup table at "index" (DispId - 1)
  if (h == S_OK)
    h = AddPropId(index, wPropID);
  
  return h;
}

// Adds one AFX_DISPMAP_ENTRY entry to the dispatch table.
long CfpIDispatchTbl::AddDispatchEntry(const AFX_DISPMAP_ENTRY FAR *pDispEntry,
  int index)
{
  BOOL fIndexValid = (index >= 0);

  // if index is invalid (negative), determine where to put it...
  if (!fIndexValid)
    index = m_iLastEntryIndex + 1;

  // if index is greater than size allocated, expand it.
  if (index >= m_iTblSizeCt)
  {
    long h = AllocDispatchTbl(index + 1);
    if (h != S_OK)
      return h;
  }

  // Copy entry to table if one was passed. If adding a blank entry, this
  // may be null.
  if (pDispEntry)
    _fmemcpy( &m_dispatchTbl[index], pDispEntry, sizeof(AFX_DISPMAP_ENTRY));

  // if a valid index was specified, record in "m_iLastDispidIndex"
  if (fIndexValid && (index > m_iLastDispidIndex))
    m_iLastDispidIndex = index;
    
  // record last (greatest) entry in use
  if (index > m_iLastEntryIndex)
    m_iLastEntryIndex = index;

  return S_OK;
}


// Adds one PropID to m_PropIdTbl.
long CfpIDispatchTbl::AddPropId(int index, WORD wPropID)
{
  BOOL fIndexValid = (index >= 0);

  // if index is invalid (negative), just return (no error).
  if (!fIndexValid)
    return S_OK;

  // if index is greater than size allocated, expand it.
  if (index >= m_PropIdTblCt)
  {
    size_t size = (index + 1) * sizeof(WORD);
    WORD FAR *tmpPtr = (WORD FAR *)_frealloc((LPVOID)m_PropIdTbl, size);
    if (!tmpPtr)
      return STG_E_INSUFFICIENTMEMORY;
  
    m_PropIdTbl = tmpPtr;
    m_PropIdTblCt = index + 1;
  }

  m_PropIdTbl[index] = wPropID;

  return S_OK;
}


// Builds the new IDispatch table (m_dispatchTbl) given the 2 lists
long CfpIDispatchTbl::BuildDispatchMap(FP_PROPLIST FAR *propList, 
  int iListCt, AFX_PMSG pfnDefGet, AFX_PMSG pfnDefSet, 
  const AFX_DISPMAP_ENTRY FAR *defDispatchMap, int iDefCt)
{
  long h;
  int index;
  int i;

  // Allocate at least the size of the two lists together
  h = AllocDispatchTbl(iListCt + iDefCt);
  if (h != S_OK)
    return h;

  // Allocate PropIdTbl as the size of only iListCt (because it is the
  // only list which contains the PropId info).
  if (iListCt)
  {
    size_t size = iListCt * sizeof(WORD);
    m_PropIdTbl = (WORD FAR *)_fmalloc(size);
    if (!m_PropIdTbl)
      return STG_E_INSUFFICIENTMEMORY;
    // initialize to 0xffff
    _fmemset( m_PropIdTbl, 0xff, size);
    m_PropIdTblCt = iListCt;
  }

  // Put entries from propList into dispatchTbl
  for (i = 0; i < iListCt; i++)
  {
    h = AddPropEntry(&propList[i], (WORD)i /*<-PropID*/, 
          pfnDefGet, pfnDefSet);
    if (h != S_OK)
      return h;        // BREAK from loop & return error
  }

  // Append (or replace) these entries from defDispatchMap into 
  // dispatchTbl
  for (i = 0; i < iDefCt; i++)
  {
    // if dispid is > 0, put at dispid - 1, else append to table
    index = (defDispatchMap[i].lDispID > 0L) ? 
              (int)defDispatchMap[i].lDispID - 1 : -1;
    h = AddDispatchEntry(&defDispatchMap[i], index);
    if (h != S_OK)
      return h;        // BREAK from loop & return error
  }

  // Re-alloc table to "final" size (leave 1 extra for loops which may
  // go too far looking for the table's last entry).
  // NOTE: m_iLastEntryIndex + 1 (changes index to count) + 1 (extra entry)
  if (h == S_OK && ((m_iLastEntryIndex + 2) < m_iTblSizeCt))
    h = AllocDispatchTbl(m_iLastEntryIndex + 2);

  return h;
}

#endif //FP_OCX && MFC
;
