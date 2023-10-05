/*************************************************************************\
* FPDBMFC.CPP - FarPoint Data Binding source file for ADO Binding.                                                            *
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

// *********************************************************************
// ****                                                             ****
// ****  IMPORTANT NOTE: Include this .CPP file in your project     ****
// ****                  ONLY if your control uses MFC (class       ****
// ****                  derived from COleControl).                 ****
// ****                                                             ****
// *********************************************************************
#ifdef FP_ADB

#include <afxctl.h>         // MFC support for OLE Custom Controls

#include "fptools.h"
#include "fpoledb.h"

#include "fpdbmfc.h"



/***************************************************************************
 *
 * FUNCTION:  fpMFCGetListOfColumnNames()
 *
 * DESCRIPTION:
 *
 *   The intended use of this function is to return a list of 
 *   columns names for the DataField property when it is requested
 *   via IPerPropertyBrowsing::GetPredefinedStrings().
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 **************************************************************************/
BOOL __cdecl fpMFCGetListOfColumnNames(CfpOleBinding *pfpOleBindObject,
  CStringArray *pStringArray, CDWordArray *pCookieArray)
{
  UINT    i = 0;
  LPCTSTR pColName;
  DWORD   dwCookie;

  // If both lists are NULL, return FALSE;
  if (!pStringArray && !pCookieArray)
    return FALSE;

  // Loop until the column name is a NULL ptr
  do 
  {
    // Get column name at this index.
    pColName = pfpOleBindObject->GetColumnNameAtIndex(i, &dwCookie);
    if (pColName)     // if column name is not null...
    {
      if (pStringArray)
        pStringArray->Add(pColName);
      if (pCookieArray)
        pCookieArray->Add(dwCookie);
      ++i;
    }
  } while (pColName);

  // Return TRUE if we have at least one column name.
  return (i != 0);   
}


/***************************************************************************
 *
 * FUNCTION:  fpMFCGetListOfDataMemberNames()
 *
 * DESCRIPTION:
 *
 *   The intended use of this function is to return a list of 
 *   DataMember names for the DataMember property when it is requested
 *   via IPerPropertyBrowsing::GetPredefinedStrings().
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 **************************************************************************/
BOOL __cdecl fpMFCGetListOfDataMemberNames(
  CfpOleBinding *pfpOleBindObject, CStringArray *pStringArray, 
  CDWordArray *pCookieArray)
{
  HRESULT hr;
  BSTR    bstr;
  LONG    lCt;
  LONG    i;
  CALPOLESTR rgOleStr = {0};
  CADWORD    rgCookie = {0};

  // If both lists are NULL, return FALSE;
  if (!pStringArray && !pCookieArray)
    return FALSE;

  // Get the list of DataMember names and stuff them in the
  // StringArray and CookieArray.

  hr = pfpOleBindObject->GetListOfDataMemberNames(&rgOleStr, &rgCookie);
 
  if (hr != S_OK)
    return FALSE;

  // Loop until we have copied every string.
  lCt = (LONG)rgOleStr.cElems;
  for (i = 0; i < lCt; i++)
  {
    // For each string, copy the DataMember name to the StringArray
    // and free the BSTR.
    bstr = rgOleStr.pElems[i];
    if (pStringArray)
      pStringArray->Add(bstr);
    if (bstr)
      SysFreeString(bstr);

    // If we were given a CookieArray, add the cookie to it.
    if (pCookieArray)
      pCookieArray->Add(rgCookie.pElems[i]);
  }

  // Free the allocate arrays.
  if (rgOleStr.pElems)
    CoTaskMemFree(rgOleStr.pElems);
  if (rgCookie.pElems)
    CoTaskMemFree(rgCookie.pElems);

  // Return TRUE if we have at least one column name.
  return (i != 0);   
}


#endif // FP_ADB
