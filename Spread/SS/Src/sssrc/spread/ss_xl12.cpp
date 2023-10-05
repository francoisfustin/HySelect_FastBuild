/*********************************************************
* SS_XL12.CPP
*
* Copyright (C) 2009 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#if defined(SS_V80) && defined(XL12)

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"
#include "spread.h"

#ifdef SS_OCX
   #ifdef _UNICODE
      #import "FarPoint.Spread8U.Excel2007.tlb" named_guids no_namespace raw_interfaces_only
   #else // !_UNICODE
      #import "FarPoint.Spread8.Excel2007.tlb" named_guids no_namespace raw_interfaces_only
   #endif // !_UNICODE
#elif SS_DLL
   #if _WIN64
      #if _UNICODE
         #import "FarPoint.Spread8UDLL64.Excel2007.tlb" named_guids no_namespace raw_interfaces_only
      #else // !_UNICODE
         #import "FarPoint.Spread8DLL64.Excel2007.tlb" named_guids no_namespace raw_interfaces_only
      #endif // !_UNICODE
   #elif _IA64
      #if _UNICODE
         #import "FarPoint.Spread8UDLLIA64.Excel2007.tlb" named_guids no_namespace raw_interfaces_only
      #else // !_UNICODE
         #import "FarPoint.Spread8DLLIA64.Excel2007.tlb" named_guids no_namespace raw_interfaces_only
      #endif // !_UNICODE
   #else // WIN32
      #if _UNICODE
         #import "FarPoint.Spread8UDLL32.Excel2007.tlb" named_guids no_namespace raw_interfaces_only
      #else // !_UNICODE
         #import "FarPoint.Spread8DLL32.Excel2007.tlb" named_guids no_namespace raw_interfaces_only
      #endif // !_UNICODE
   #endif // WIN32
#endif // SS_DLL

void DisplayError(HRESULT hr, LPTSTR lptstr);

#pragma warning(disable:4268)
#include "mscoree.h"
extern "C"
{
typedef HRESULT (__stdcall *COR_BIND_TO_RUNTIME_EX)(LPCWSTR pwszVersion, LPCWSTR pwszBuildFlavor, DWORD startupFlags, REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv);
}
ICorRuntimeHost* pRuntimeHost = NULL;

BOOL xl_LoadFramework()
{  // Bind to the runtime.
   if(pRuntimeHost == NULL)
   {
      int flags = 0x1 << 1; // STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN;
      flags |= /*STARTUP_TRIM_GC_COMMIT*/ 0x80000;
      HINSTANCE hInstMscoree = LoadLibrary(_T("mscoree.dll"));
      if( hInstMscoree != NULL )
      {
         COR_BIND_TO_RUNTIME_EX pCorBindToRuntimeEx = (COR_BIND_TO_RUNTIME_EX)GetProcAddress(hInstMscoree, "CorBindToRuntimeEx");
         if( pCorBindToRuntimeEx )
         {
            HRESULT hrCorBind = pCorBindToRuntimeEx(
            NULL,   // Load the latest CLR version available
            L"wks", // Workstation GC ("wks" or "svr" overrides)
            flags,  
            __uuidof(CorRuntimeHost), __uuidof(ICorRuntimeHost),
            (void**)&pRuntimeHost);

            // Now, start the CLR.
            if(!FAILED( hrCorBind ))
            {
               HRESULT hrStart = pRuntimeHost->Start();
               if(!FAILED(hrStart))
                  return TRUE;
            }
         }
      }
      return FALSE;
   }
   else
      return TRUE;
}

extern "C"
{

BOOL xl_OpenExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short sheet, short excelSheet, LPCTSTR logFileName)
{
  HRESULT hr = CoInitialize(NULL);
  if (S_OK != hr && S_FALSE != hr)
  {
    CoUninitialize();
    DisplayError(hr, _T("CoInitialize"));
    return FALSE;
  }
  if( !xl_LoadFramework() )
     return FALSE;
  IFpExcelStub* excelPtr;
  hr = CoCreateInstance(CLSID_FpExcelStub, NULL, CLSCTX_INPROC_SERVER, IID_IFpExcelStub, (LPVOID*)&excelPtr);
  
  if(S_OK != hr)
  {
    CoUninitialize();
    DisplayError(hr, _T("CoCreateInstance"));
    return FALSE;
  }
  
#ifndef _UNICODE
  WCHAR szFileName[MAX_PATH + 1]; 
  WCHAR szLogFileName[MAX_PATH + 1];
  WCHAR szPassword[MAX_PATH + 1];
  memset(szFileName, 0, (MAX_PATH+1)*sizeof(WCHAR));
  memset(szLogFileName, 0, (MAX_PATH+1)*sizeof(WCHAR));
  memset(szPassword, 0, (MAX_PATH+1)*sizeof(WCHAR));
  if( fileName )
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, fileName, -1, szFileName, MAX_PATH);
  if( logFileName )
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, logFileName, -1, szLogFileName, MAX_PATH);
  if( password )
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, password, -1, szPassword, MAX_PATH);
#else //UNICODE
  TCHAR    szFileName[MAX_PATH + 1];
  TCHAR    szLogFileName[MAX_PATH + 1];
  TCHAR    szPassword[MAX_PATH + 1];
  memset(szFileName, 0, (MAX_PATH+1)*sizeof(TCHAR));
  memset(szLogFileName, 0, (MAX_PATH+1)*sizeof(TCHAR));
  memset(szPassword, 0, (MAX_PATH+1)*sizeof(TCHAR));
  if( fileName )
    wcsncpy(szFileName, fileName, MAX_PATH);
  if( logFileName )
    wcsncpy(szLogFileName, logFileName, MAX_PATH);
  if( password )
    wcsncpy(szPassword, password, MAX_PATH);
#endif //UNICODE
  
  VARIANT_BOOL ret;
  BSTR strFileName = SysAllocString(szFileName);
  BSTR strLogFileName = SysAllocString(szLogFileName);
  BSTR strPassword = SysAllocString(szPassword);
#if defined(_WIN64) || defined(_IA64)
  hr = excelPtr->OpenExcelEx((LONG_PTR)spread, strFileName, strLogFileName, sheet, excelSheet, strPassword, &ret);
#else
  hr = excelPtr->OpenExcelEx((long)spread, strFileName, strLogFileName, sheet, excelSheet, strPassword, &ret);
#endif
  SysFreeString(strFileName);
  SysFreeString(strLogFileName);
  SysFreeString(strPassword);
  if(S_OK != hr)
  {
    DisplayError(hr, _T("OpenExcel2007File"));
    return FALSE;
  }
  
  excelPtr->Release();
  CoUninitialize();
  return (BOOL)ret;
}

short xl_SaveExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short flags, LPCTSTR logFileName)
{
  HRESULT hr = CoInitialize(NULL);
  if (S_OK != hr && S_FALSE != hr)
  {
    CoUninitialize();
    DisplayError(hr, _T("CoInitialize"));
    return FALSE;
  }
  if( !xl_LoadFramework() )
     return FALSE;
  IFpExcelStub* excelPtr;
  hr = CoCreateInstance(CLSID_FpExcelStub, NULL, CLSCTX_INPROC_SERVER, IID_IFpExcelStub, (LPVOID*)&excelPtr);
  
  if(S_OK != hr)
  {
    CoUninitialize();
    DisplayError(hr, _T("CoCreateInstance"));
    return FALSE;
  }
  
#ifndef _UNICODE
  WCHAR szFileName[MAX_PATH + 1]; 
  WCHAR szLogFileName[MAX_PATH + 1];
  WCHAR szPassword[MAX_PATH + 1];
  memset(szFileName, 0, (MAX_PATH+1)*sizeof(WCHAR));
  memset(szLogFileName, 0, (MAX_PATH+1)*sizeof(WCHAR));
  memset(szPassword, 0, (MAX_PATH+1)*sizeof(WCHAR));
  if( fileName )
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, fileName, -1, szFileName, MAX_PATH);
  if( logFileName )
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, logFileName, -1, szLogFileName, MAX_PATH);
  if( password )
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, password, -1, szPassword, MAX_PATH);
#else //UNICODE
  TCHAR    szFileName[MAX_PATH + 1];
  TCHAR    szLogFileName[MAX_PATH + 1];
  TCHAR    szPassword[MAX_PATH + 1];
  memset(szFileName, 0, (MAX_PATH+1)*sizeof(TCHAR));
  memset(szLogFileName, 0, (MAX_PATH+1)*sizeof(TCHAR));
  memset(szPassword, 0, (MAX_PATH+1)*sizeof(TCHAR));
  if( fileName )
    wcsncpy(szFileName, fileName, MAX_PATH);
  if( logFileName )
    wcsncpy(szLogFileName, logFileName, MAX_PATH);
  if( password )
    wcsncpy(szPassword, password, MAX_PATH);
#endif //UNICODE

  short ret;
  BSTR strFileName = SysAllocString(szFileName);
  BSTR strLogFileName = SysAllocString(szLogFileName);
  BSTR strPassword = SysAllocString(szPassword);
#if defined(_WIN64) || defined(_IA64)
  hr = excelPtr->SaveExcelEx((LONG_PTR)spread, strFileName, strLogFileName, (short)(flags | 16), strPassword, &ret);
#else
  hr = excelPtr->SaveExcelEx((long)spread, strFileName, strLogFileName, (short)(flags | 16), strPassword, &ret);
#endif
  SysFreeString(strFileName);
  SysFreeString(strLogFileName);
  SysFreeString(strPassword);
  if(S_OK != hr)
  {
    DisplayError(hr, _T("SaveExcel2007File"));
    return FALSE;
  }

  excelPtr->Release();
  CoUninitialize();
  return (BOOL)ret;
}

short xl_IsExcel2007File(LPFPCONTROL spread, LPCTSTR fileName)
{
  HRESULT hr = CoInitialize(NULL);
  if (S_OK != hr && S_FALSE != hr)
  {
    CoUninitialize();
    DisplayError(hr, _T("CoInitialize"));
    return FALSE;
  }
  if( !xl_LoadFramework() )
     return FALSE;
  IFpExcelStub* excelPtr;
  hr = CoCreateInstance(CLSID_FpExcelStub, NULL, CLSCTX_INPROC_SERVER, IID_IFpExcelStub, (LPVOID*)&excelPtr);
  
  if(S_OK != hr)
  {
    CoUninitialize();
    DisplayError(hr, _T("CoCreateInstance"));
    return FALSE;
  }
#ifndef _UNICODE
  WCHAR szFileName[MAX_PATH + 1]; 
  memset(szFileName, 0, (MAX_PATH+1)*sizeof(WCHAR));
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, fileName, -1, szFileName, MAX_PATH);
#else //UNICODE
  TCHAR    szFileName[MAX_PATH + 1];
  memset(szFileName, 0, (MAX_PATH+1)*sizeof(TCHAR));
  wcscpy(szFileName, fileName);
#endif //UNICODE

  VARIANT_BOOL ret;
  BSTR strFileName = SysAllocString(szFileName);
#if defined(_WIN64) || defined(_IA64)
  hr = excelPtr->IsExcelFile((LONG_PTR)spread, strFileName, &ret);
#else
  hr = excelPtr->IsExcelFile((long)spread, strFileName, &ret);
#endif
  SysFreeString(strFileName);
  if(S_OK != hr)
  {
    DisplayError(hr, _T("IsExcel2007File"));
    return FALSE;
  }

  excelPtr->Release();
  CoUninitialize();
  return ret;
}

BOOL xl_IsExcel2007Supported()
{
  HRESULT hr = CoInitialize(NULL);
  if (S_OK != hr && S_FALSE != hr)
  {
    CoUninitialize();
    return FALSE;
  }
  if( !xl_LoadFramework() )
     return FALSE;
  IFpExcelStub* excelPtr;
  hr = CoCreateInstance(CLSID_FpExcelStub, NULL, CLSCTX_INPROC_SERVER, IID_IFpExcelStub, (LPVOID*)&excelPtr);
  
  if(S_OK != hr)
  {
    CoUninitialize();
    return FALSE;
  }
  excelPtr->Release();
  CoUninitialize();
  return TRUE;
}

} // extern "C"

#endif // SS_V80 && XL12

