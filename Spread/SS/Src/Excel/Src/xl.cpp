/*********************************************************
* XL.cpp
*
* Copyright (C) 1999 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* UPDATE LOG:
* -----------
*	RAP	- 11.19.98
*********************************************************/
#include "windows.h"
#include "spread.h"
#include "xl.h"
#include "xlbiff.h"
#include "biff.h"

/***********************************************************************
* Name:   xl_LoadFile - Open the file and read its contents. After completion,
*                       release the stream (clean-up), release the storage, and
*                       uninitialize the COM library if needed.
*
*
* Usage:  short FP_API xl_LoadFile(LPCTSTR szXLFileName, LPSSXL lpSSXL,
*                                  LPSPREADSHEET lpSS, int nSheetNum,
*                                  short sPass, BOOL fCheckOnly)
*            szXLFileName - the file to open & load
*            lpSSXL - pointer to the Excel spreadsheet structure 
*                     (used for conversion) 
*            lpSS - pointer to the Spread control
*            nSheetNum - sheet number of the sheet to load
*            sPass - which time through this function. (1 or 2)
*            fCheckOnly - check to verify if this file is a valid
*                         structured storage model(SSM) file and the
*                         Excel section is in the correct BIFF format.
*
* Return: short - Success or Failure
***********************************************************************/
short FP_API xl_LoadFile(LPCTSTR szXLFileName, LPSSXL lpSSXL, LPSS_BOOK lpBook, int nSheetNum, short sPass, BOOL fCheckOnly)
{
  IStorage *pstg = NULL;
  IStream  *pstm = NULL;
  BOOL     fUninit = TRUE;
  HRESULT  hr = 0;

// The IStorage & IStream methods require WCHAR strings!
#ifndef _UNICODE
  WCHAR szFileName[MAX_PATH + 1]; 
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szXLFileName, -1, szFileName, MAX_PATH);
#else //UNICODE
  TCHAR    szFileName[MAX_PATH + 1];
  wcscpy(szFileName, szXLFileName);
#endif //UNICODE

  hr = ssm_Initialize();
  if (hr == S_FALSE) //S_FALSE means the COM Library is already initialized.
  {
    fUninit = FALSE; //Therefore, don't uninitialize it at the end.   
    hr = S_OK;
  }
 	else if (hr != S_OK) 
    return 1;

  //!!!RAP!!! use "Workbook" for the stream name -- look into this!
  hr = ssm_OpenStorageObject(szFileName, L"Workbook", &pstg, &pstm, TRUE);
	if (hr != S_OK)
  {
    ssm_CleanUp(pstm);
    if (pstg)
      pstg->Release();
    if (fUninit)
      ssm_UnInitialize();
    if (STG_E_SHAREVIOLATION == hr ||  STG_E_LOCKVIOLATION == hr)
      return 2;
    else
      return 1;
  }

  if (!fCheckOnly)
  {
    hr = ssm_StreamRead(pstm, lpSSXL, lpBook, nSheetNum, sPass);
	  if (hr != S_OK)
    { 
      ssm_CleanUp(pstm);
      if (pstg)
        pstg->Release();
      if (fUninit)
        ssm_UnInitialize();
      return 1;
    }
  }

  hr = ssm_CleanUp(pstm);
	if (hr != S_OK)
  {
    if (pstg)
      pstg->Release();
    if (fUninit)
      ssm_UnInitialize();
    return 1;
  }

  // Release the IStorage pointer to force the file to be flushed.
  pstg->Release();

  if (fUninit)
    hr = ssm_UnInitialize();
	if (hr != S_OK)
    return 1;

  return 0;  
}

/***********************************************************************
* Name:   xl_SaveFile - Open the file or create the file if it does not
*                       already exist. Create the stream with the provided
*                       name. Write the converted data to the stream. After
*                       completion, release the stream (clean-up), release 
*                       the storage, and uninitialize the COM library if needed.
*                       
* Usage:  short FP_API xl_SaveFile(LPCTSTR szXLFileName, LPCTSTR szSheetName,
*                                  LPVOID lp, LPTBGLOBALHANDLE lpghBuffer,
*                                  LPLONG lplBufferLen, BOOL fToFile)
*            szXLFileName - the file name to save to
*            szSheetName - the sheet name for the sheet to be saved
*            lp - the converted data buffer.
*
* Return: short - Success or Failure
***********************************************************************/
short FP_API xl_SaveFile(LPCTSTR szXLFileName, LPCTSTR szSheetName, LPVOID lp, 
                         LPTBGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen, BOOL fToFile, SHORT sFlags)
{
  IStorage *pstg = NULL;
  IStream  *pstm = NULL;
  BOOL     fUninit = TRUE;
  HRESULT  hr = 0;

  if (fToFile)
  {
    // The IStorage & IStream methods require WCHAR strings!
#ifndef _UNICODE
    WCHAR szFileName[MAX_PATH + 1]; 
    memset(szFileName, 0, (MAX_PATH+1)*sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szXLFileName, -1, szFileName, MAX_PATH);
#else //UNICODE
    TCHAR    szFileName[MAX_PATH + 1];
    memset(szFileName, 0, (MAX_PATH+1)*sizeof(TCHAR));
    wcscpy(szFileName, szXLFileName);
#endif //UNICODE

    hr = ssm_Initialize();
    if (hr == S_FALSE) //S_FALSE means the COM Library is already initialized.
    {
      fUninit = FALSE; //Therefore, don't uninitialize it at the end.   
      hr = S_OK;
    }
 	  else if (hr != S_OK) 
      return 1;

//    hr = ssm_OpenStorageObject(szFileName, L"Workbook", &pstg, &pstm, FALSE);
//	  if (hr != S_OK)
//    {
//      if (hr == S_FALSE || hr == STG_E_FILENOTFOUND)
//      {
//        //!!!RAP!!! use "Workbook" for the stream name -- look into this!
//        hr = ssm_CreateStorageObject(szFileName, L"Workbook", &pstg, &pstm);
//        if (hr != S_OK)
//        {
//          if (fUninit)
//            ssm_UnInitialize();
//          return 1;
//        }
//      }
//      else
//      {
//        if (fUninit)
//          ssm_UnInitialize();
//        return 1;
//      }
//   }
    hr = ssm_CreateStorageObject(szFileName, L"Workbook", &pstg, &pstm);
    if (hr != S_OK)
    {
      if (fUninit)
        ssm_UnInitialize();
      return 1;
    }

  } //fToFile

  hr = ssm_StreamWrite(pstm, lp, szSheetName, lpghBuffer, lplBufferLen, fToFile, sFlags);

  if (fToFile)
  {
  	if (hr != S_OK)
    {
	    ssm_UnInitialize();
      return 1;
    }

    hr = ssm_CleanUp(pstm);
	  if (hr != S_OK)
    {
	    ssm_UnInitialize();
      return 1;
    }
  
    // Release the IStorage pointer to force the file to be flushed.
    pstg->Release();

    hr = ssm_UnInitialize();
	  if (hr != S_OK)
      return 1;
  } //fToFile

  return 0;
}

