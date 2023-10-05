// ssocx.cpp : Implementation of CSsocxApp and DLL registration.

#include "stdafx.h"
#include "ssocx.h"
#ifdef WIN32
#include <objsafe.h>
#include <oaidl.h>
#endif

#ifdef SS_V30
#if !defined(SS_V80) && !defined(SS_V70)
#include "..\..\..\fplibs\fptools\jpeg\jpeglib.h"
#endif
#include "..\sssrc\spread\ss_html.h"
#endif

#ifndef SS_OLDCALC
#include "..\sssrc\calc\cal_mem.h"
#endif

#ifdef FP_OLEDRAG
#include "..\..\..\fplibs\fptools\src\fpdatobj.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


CSsocxApp NEAR theApp;

#if defined(SS_SPREADOCX)
  #if defined(_UNICODE)
    #if defined(SPREAD_JPN) && defined(SS_V70)
      // ADO Unicode fpSpread 7.0 Japan typelib -scl
      const GUID CDECL BASED_CODE _tlid =
     { 0xf13c99c1, 0x4d73, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #elif SS_V40
      #ifdef FP_ADB // ADO Unicode fpSpread 6.0 typelib -scl
        // {B9411660-10E6-4a53-BE96-7FED334704FA}
        const GUID CDECL BASED_CODE _tlid =
        { 0xb9411660, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
      #else // non-ADO Unicode fpSpread 6.0 typelib -scl
        // {B9411661-10E6-4a53-BE96-7FED334704FA}
        const GUID CDECL BASED_CODE _tlid =
		{ 0xA993A02F, 0x5A82, 0x4550, { 0x93, 0x62, 0x84, 0xCE, 0x83, 0xF6, 0x3E, 0xE3 } } ;
        //{ 0xb9411661, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
      #endif // FP_ADB
    #elif SS_V30
      #ifdef FP_ADB // ADO Unicode fpSpread 3.0 typelib -scl
        // {7D797BE0-4D76-11d2-B8B2-0000C00A958C}
        const GUID CDECL BASED_CODE _tlid =
            { 0x7d797be0, 0x4d76, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      #else // non-ADO Unicode fpSpread 3.0 typelib (same as 2.5) -scl
        const GUID CDECL BASED_CODE _tlid =
            { 0xf5ca1ac0, 0xa9d8, 0x11cf, { 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19 } };
      #endif // FP_ADB
    #else // non-ADO Unicode vaSpread 2.5 typelib
      const GUID CDECL BASED_CODE _tlid =
          { 0xf5ca1ac0, 0xa9d8, 0x11cf, { 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19 } };
    #endif // SS_V30
  #else // not Unicode -scl
    #if defined(SS_V40) && !defined(SPREAD_JPN)
      #ifdef FP_ADB // ADO ANSI fpSpread 4.0 typelib (same as 3.5)
        // {F856EC8B-F03C-4515-BDC6-64CBD617566A}
        const GUID CDECL BASED_CODE _tlid =
        { 0xf856ec8b, 0xf03c, 0x4515, { 0xbd, 0xc6, 0x64, 0xcb, 0xd6, 0x17, 0x56, 0x6a } };
      #else // non-ADO ANSI fpSpread 4.0 typelib (same as 3.5) -scl
        // {FDAC2480-F4ED-4632-AA78-DCA210A74E49}
        const GUID CDECL BASED_CODE _tlid =
        { 0xfdac2480, 0xf4ed, 0x4632, { 0xaa, 0x78, 0xdc, 0xa2, 0x10, 0xa7, 0x4e, 0x49 } };
      #endif // FP_ADB
    #elif defined(SS_V40) && defined(SPREAD_JPN) // Japan 6.0
      #ifdef FP_ADB // ADO ANSI fpSpread 6.0 typelib (same as 3.0) -scl
        // {F13C99C0-4D73-11d2-B8B2-0000C00A958C}
        const GUID CDECL BASED_CODE _tlid =
        { 0xf13c99c0, 0x4d73, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      #else // non-ADO ANSI fpSpread 6.0 typelib (same as 3.0) -scl
        const GUID CDECL BASED_CODE _tlid =
            { 0xb02f3647, 0x766b, 0x11ce, { 0xaf, 0x28, 0xc3, 0xa2, 0xfb, 0xe7, 0x6a, 0x13 } };
      #endif // FP_ADB
    #elif SS_V35
      #ifdef FP_ADB // ADO ANSI fpSpread 3.5 typelib
        // {F856EC8B-F03C-4515-BDC6-64CBD617566A}
        const GUID CDECL BASED_CODE _tlid =
        { 0xf856ec8b, 0xf03c, 0x4515, { 0xbd, 0xc6, 0x64, 0xcb, 0xd6, 0x17, 0x56, 0x6a } };
      #else // non-ADO ANSI fpSpread 3.5 typelib (same as 2.5) -scl
        // {FDAC2480-F4ED-4632-AA78-DCA210A74E49}
        const GUID CDECL BASED_CODE _tlid =
        { 0xfdac2480, 0xf4ed, 0x4632, { 0xaa, 0x78, 0xdc, 0xa2, 0x10, 0xa7, 0x4e, 0x49 } };
      #endif // FP_ADB
    #elif SS_V30
      #ifdef FP_ADB // ADO ANSI fpSpread 3.0 typelib
        // {F13C99C0-4D73-11d2-B8B2-0000C00A958C}
        const GUID CDECL BASED_CODE _tlid =
        { 0xf13c99c0, 0x4d73, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      #else // non-ADO ANSI fpSpread 3.0 typelib (same as 2.5) -scl
        const GUID CDECL BASED_CODE _tlid =
            { 0xb02f3647, 0x766b, 0x11ce, { 0xaf, 0x28, 0xc3, 0xa2, 0xfb, 0xe7, 0x6a, 0x13 } };
      #endif // FP_ADB
    #else // non-ADO ANSI vaSpread 2.5 typelib
      const GUID CDECL BASED_CODE _tlid =
          { 0xb02f3647, 0x766b, 0x11ce, { 0xaf, 0x28, 0xc3, 0xa2, 0xfb, 0xe7, 0x6a, 0x13 } };
    #endif // SS_V30
  #endif // _UNICODE
#elif defined(SS_GRIDOCX)
  #ifdef SS_V30
    #ifdef FP_ADB // ADO ANSI fpGrid 3.0 typelib -scl
      // {FE6B1DC0-4D77-11d2-B8B2-0000C00A958C}
      const GUID CDECL BASED_CODE _tlid =
      { 0xfe6b1dc0, 0x4d77, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #else // non-ADO ANSI fpGrid 3.0 typelib (same as 2.5) -scl
      const GUID CDECL BASED_CODE _tlid =
              { 0x4b687083, 0xf21e, 0x11ce, { 0x86, 0x47, 0xd9, 0x52, 0x18, 0x5a, 0xe9, 0x18 } };
    #endif // FP_ADB
  #else // non-ADO ANSI vaGrid 2.5 typelib -scl
    const GUID CDECL BASED_CODE _tlid =
            { 0x4b687083, 0xf21e, 0x11ce, { 0x86, 0x47, 0xd9, 0x52, 0x18, 0x5a, 0xe9, 0x18 } };
  #endif // SS_V30
#elif defined(SS_RSOCX)
  #ifdef SS_V30
    #ifdef FP_ADB // ADO ANSI RSSpread 3.0 typelib -scl
      // {BF089B30-4D78-11d2-B8B2-0000C00A958C}
      const GUID CDECL BASED_CODE _tlid =
      { 0xbf089b30, 0x4d78, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #else // non-ADO ANSI RSSpread 3.0 typelib (same as 2.5) -scl
      const GUID CDECL BASED_CODE _tlid =
          { 0x5a276520, 0xc536, 0x11cf, { 0x87, 0x9a, 0, 0xaa, 0, 0xbc, 0x8f, 0x75 } };
    #endif // FP_ADB
  #else // non-ADO ANSI RSSpread 2.5 typelib -scl
    const GUID CDECL BASED_CODE _tlid =
        { 0x5a276520, 0xc536, 0x11cf, { 0x87, 0x9a, 0, 0xaa, 0, 0xbc, 0x8f, 0x75 } };
  #endif // SS_V30
#endif

#if SS_V80
  const WORD _wVerMajor = 8;
  const WORD _wVerMinor = 0;
#elif SS_V70
  const WORD _wVerMajor = 7;
  const WORD _wVerMinor = 0;
#elif SS_V40
  const WORD _wVerMajor = 6;
  const WORD _wVerMinor = 0;
#elif SS_V35
  const WORD _wVerMajor = 3;
  const WORD _wVerMinor = 5;
#elif SS_V30
  const WORD _wVerMajor = 3;
  const WORD _wVerMinor = 0;
#else
  const WORD _wVerMajor = 2;
  const WORD _wVerMinor = 5;
#endif // SS_V30


////////////////////////////////////////////////////////////////////////////
// Following includes and declarations copied from SSW.C file.

#include <toolbox.h>

#define  DRV_EDIT

#include "..\sssrc\libgen.h"
#include "..\sssrc\spread\spread.h"
#include "..\sssrc\classes\wintools.h"
#include "..\sssrc\classes\viewpict.h"
#include "..\sssrc\spread\ss_heap.h"

#include "..\..\..\fplibs\fptools\src\fpconvrt.h"
#ifdef SS_V30
#include "..\ssprvw\ssprvw.h"
#endif

static BOOL RegisterLibClass(HINSTANCE);
static BOOL UnregisterLibClass(HINSTANCE hInst);
static BOOL DynamicInit = FALSE;

extern "C" {

OMEM_POOL tbStringPool;
OMEM_POOL tbOmemPoolCol;
OMEM_POOL tbOmemPoolRow;
OMEM_POOL tbOmemPoolCell;
HINSTANCE hDynamicInst;
HANDLE    fpInstance;

LPTSTR     lpszSpinClassName = VA_SPINBTN_CLASS_NAME;

int WINAPI              DialogBoxProc(HINSTANCE, LPCTSTR, HWND, DLGPROC);

struct tagToolBoxGlobal
   {
   int ClearField;
   int CurrentDateTimeKey;
   int PopupCalendarKey;
   } ToolBoxGlobal = {0 ,0, 0};
//   } ToolBoxGlobal = {VK_F2, VK_F3, VK_F4};

#ifdef WIN32
int   SmartHeap_malloc = 0;
short dxPixelsPerInch;
short dyPixelsPerInch;
#else
int __cdecl SmartHeap_malloc = 0;
short PASCAL dxPixelsPerInch;
short PASCAL dyPixelsPerInch;
#endif

}

                           
////////////////////////////////////////////////////////////////////////////
// CSsocxApp::InitInstance - DLL initialization

BOOL CSsocxApp::InitInstance()
{
  BOOL bInit = COleControlModule::InitInstance();

  if (bInit)
  {
//- Wei Feng add for working arround a bug in MFC
#ifdef	SPREAD_JPN
#ifndef	WIN32
	extern CParkingWnd* _gpWndParking;
	if(_gpWndParking == NULL)
    	_gpWndParking = new CParkingWnd;
	//OutputDebugString("InitInstance\n");
#endif
#endif

    HDC hDC = GetDC(GetDesktopWindow());
    dxPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);;
    dyPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSY);;
    ReleaseDC(GetDesktopWindow(), hDC);

#if 0
    {
      // Beta software time bomb. Invalid after 01/Feb/1996.
      // tm = {sec,min,hour,day,mon(0-11),year(0-99),wDay,yDay,isDST}
      tm tmBomb = {0, 0, 0, 1, 1, 96, 0, 0, 0};
      time_t timeBomb;
      time_t timeNow;
      timeBomb = mktime(&tmBomb);
      timeNow = time(NULL);
      if( difftime(timeNow, timeBomb) > 0.0 )
      {
        #if defined(SS_SPREADOCX)
        MessageBox(0, _T("SSOCX.OCX beta software has expired."), _T("Warning"), MB_OK);
        #elif defined(SS_GRIDOCX)
        MessageBox(0, _T("FPGRID beta software has expired."), _T("Warning"), MB_OK);
        #elif defined(SS_RSOCX)
        MessageBox(0, _T("RSSPRD32.OCX beta software has expired."), _T("Warning"), MB_OK);
        #else
        MessageBox(0, _T("Beta software has expired."), _T("Warning"), MB_OK);
        #endif
        return FALSE;
      }
    }
#endif

//    hDynamicInst = AfxGetInstanceHandle();
    hDynamicInst = m_hInstance;
    fpInstance = m_hInstance;
    DynamicInit = TRUE;
#ifndef SS_NO_USE_SH
    MemRegisterTask();
#endif
    RegisterLibClass(hDynamicInst);
  }

  return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CSsocxApp::ExitInstance - DLL termination

int CSsocxApp::ExitInstance()
{
  // TODO: Add your own module termination code here.
//- Wei Feng add for working arround a bug in MFC
#ifdef	SPREAD_JPN
#ifndef	WIN32
	extern CParkingWnd* _gpWndParking;
    if(_gpWndParking != NULL)
	{
    	delete _gpWndParking;
		_gpWndParking = NULL;
	}
	//OutputDebugString("ExitInstance\n");
#endif
#endif

  UnregisterLibClass(hDynamicInst);
#ifndef SS_NO_USE_SH
  MemUnregisterTask();
#endif

  return COleControlModule::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{ 
  AFX_MANAGE_STATE(_afxModuleAddrThis);

//  if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
  if (!AfxOleRegisterTypeLib(hDynamicInst, _tlid))
    return ResultFromScode(SELFREG_E_TYPELIB);

  if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
    return ResultFromScode(SELFREG_E_CLASS);

  return NOERROR;
}

// copied from AfxOleUnregisterTypeLib in Vc98\Mfc\Src\ctlreg.cpp

#include "afxconv.h"

#define GUID_CCH    39  // Characters in string form of guid, including '\0'
#define TYPELIBWIN   _T("win32")
#define TYPELIBWIN_2 _T("win16")

inline BOOL RegDeleteKeySucceeded(LONG error)
{
	return (error == ERROR_SUCCESS) || (error == ERROR_BADKEY) ||
		(error == ERROR_FILE_NOT_FOUND);
}

// Under Win32, a reg key may not be deleted unless it is empty.
// Thus, to delete a tree,  one must recursively enumerate and
// delete all of the sub-keys.

LONG RecursiveRegDeleteKey(HKEY hParentKey, LPTSTR szKeyName)
{
	// one implementation for everybody
	return AfxDelRegTreeHelper(hParentKey, szKeyName);
}

void UnregisterInterfaces(ITypeLib* pTypeLib)
{
	TCHAR szKey[128];
	lstrcpy(szKey, _T("Interface\\"));
	LPTSTR pszGuid = szKey + lstrlen(szKey);

	int cTypeInfo = pTypeLib->GetTypeInfoCount();

	for (int i = 0; i < cTypeInfo; i++)
	{
		TYPEKIND tk;
		if (SUCCEEDED(pTypeLib->GetTypeInfoType(i, &tk)) &&
			(tk == TKIND_DISPATCH || tk == TKIND_INTERFACE))
		{
			ITypeInfo* pTypeInfo = NULL;
			if (SUCCEEDED(pTypeLib->GetTypeInfo(i, &pTypeInfo)))
			{
				TYPEATTR* pTypeAttr;
				if (SUCCEEDED(pTypeInfo->GetTypeAttr(&pTypeAttr)))
				{
#if defined(_UNICODE) || defined(OLE2ANSI)
					StringFromGUID2(pTypeAttr->guid, pszGuid, GUID_CCH);
#else
					WCHAR wszGuid[39];
					StringFromGUID2(pTypeAttr->guid, wszGuid, GUID_CCH);
					_wcstombsz(pszGuid, wszGuid, GUID_CCH);
#endif
					RecursiveRegDeleteKey(HKEY_CLASSES_ROOT, szKey);
					pTypeInfo->ReleaseTypeAttr(pTypeAttr);
				}

				pTypeInfo->Release();
			}
		}
	}
}

BOOL UnregisterTypeLib(REFGUID tlid, WORD wVerMajor,
	WORD wVerMinor, LCID lcid = 0)
{
	USES_CONVERSION;

	// Load type library before unregistering it.
	ITypeLib* pTypeLib = NULL;
	if (wVerMajor != 0)
	{
		if (FAILED(LoadRegTypeLib(tlid, wVerMajor, wVerMinor, lcid, &pTypeLib)))
			pTypeLib = NULL;
	}

	// Format typelib guid as a string
	OLECHAR szTypeLibID[GUID_CCH];
	int cchGuid = ::StringFromGUID2(tlid, szTypeLibID, GUID_CCH);

	ASSERT(cchGuid == GUID_CCH);    // Did StringFromGUID2 work?
	if (cchGuid != GUID_CCH)
		return FALSE;

	TCHAR szKeyTypeLib[_MAX_PATH];
	BOOL bSurgical = FALSE;
	LONG error = ERROR_SUCCESS;

	wsprintf(szKeyTypeLib, _T("TYPELIB\\%s"), OLE2CT(szTypeLibID));

	HKEY hKeyTypeLib;
	if (RegOpenKey(HKEY_CLASSES_ROOT, szKeyTypeLib, &hKeyTypeLib) ==
		ERROR_SUCCESS)
	{
		int iKeyVersion = 0;
		HKEY hKeyVersion;
		TCHAR szVersion[_MAX_PATH];

  	BOOL bSurgicalVersion = FALSE;
		// Iterate through all installed versions of the control

		while (RegEnumKey(hKeyTypeLib, iKeyVersion, szVersion, _MAX_PATH) ==
			ERROR_SUCCESS)
		{
			hKeyVersion = NULL;
// moved up ^- above while loop -scl
//			BOOL bSurgicalVersion = FALSE;

			if (RegOpenKey(hKeyTypeLib, szVersion, &hKeyVersion) !=
				ERROR_SUCCESS)
			{
				++iKeyVersion;
				continue;
			}

// fix inserted here -scl
      TCHAR pszMajor[_MAX_PATH];
      LPTSTR pszMinor;

      lstrcpy(pszMajor, szVersion);
      _tcstok(pszMajor, _T("."));
      pszMinor = pszMajor + lstrlen(pszMajor) + 1;
      if( wVerMajor != (WORD)_ttoi(pszMajor) || 
          wVerMinor != (WORD)_ttoi(pszMinor) )
			{
				++iKeyVersion;
  			bSurgical = bSurgicalVersion = TRUE;
				continue;
			}

			int iKeyLocale = 0;
			HKEY hKeyLocale;
			TCHAR szLocale[_MAX_PATH];

			// Iterate through all registered locales for this version

			while (RegEnumKey(hKeyVersion, iKeyLocale, szLocale, _MAX_PATH) ==
				ERROR_SUCCESS)
			{
				// Don't remove HELPDIR or FLAGS keys.
				if ((lstrcmpi(szLocale, _T("HELPDIR")) == 0) ||
					(lstrcmpi(szLocale, _T("FLAGS")) == 0))
				{
					++iKeyLocale;
					continue;
				}

				hKeyLocale = NULL;

				if (RegOpenKey(hKeyVersion, szLocale, &hKeyLocale) !=
					ERROR_SUCCESS)
				{
					++iKeyLocale;
					continue;
				}

				// Check if a 16-bit key is found when unregistering 32-bit
				HKEY hkey;
				if (RegOpenKey(hKeyLocale, TYPELIBWIN_2, &hkey) ==
					ERROR_SUCCESS)
				{
					RegCloseKey(hkey);

					// Only remove the keys specific to the 32-bit version
					// of control, leaving things intact for 16-bit version.
					error = RecursiveRegDeleteKey(hKeyLocale, TYPELIBWIN);
					bSurgicalVersion = TRUE;
					RegCloseKey(hKeyLocale);
				}
				else
				{
					// Delete everything for this locale.
					RegCloseKey(hKeyLocale);
					if (RecursiveRegDeleteKey(hKeyVersion, szLocale) ==
						ERROR_SUCCESS)
					{
						// Start over again, so we don't skip anything.
						iKeyLocale = 0;
						continue;
					}
				}
				++iKeyLocale;
			}
			RegCloseKey(hKeyVersion);

			if (bSurgicalVersion)
			{
				bSurgical = TRUE;
			}
			else
			{
				if (RecursiveRegDeleteKey(hKeyTypeLib, szVersion) ==
					ERROR_SUCCESS)
				{
					// Start over again, to make sure we don't skip anything.
					iKeyVersion = 0;
					continue;
				}
			}

			++iKeyVersion;
		}
		RegCloseKey(hKeyTypeLib);
	}

	if (!bSurgical)
		error = RecursiveRegDeleteKey(HKEY_CLASSES_ROOT, szKeyTypeLib);

	if (RegDeleteKeySucceeded(error))
	{
		// If type library was unregistered successfully, then also unregister
		// interfaces.
		if (pTypeLib != NULL)
		{
			ITypeLib* pDummy = NULL;
			if (FAILED(LoadRegTypeLib(tlid, wVerMajor, wVerMinor, lcid, &pDummy)))
				UnregisterInterfaces(pTypeLib);
			else
				pDummy->Release();

			pTypeLib->Release();
		}
	}

	return RegDeleteKeySucceeded(error);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
  AFX_MANAGE_STATE(_afxModuleAddrThis);

  if (!UnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
    return ResultFromScode(SELFREG_E_TYPELIB);

  if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
    return ResultFromScode(SELFREG_E_CLASS);
    return NOERROR;

}

/////////////////////////////////////////////////////////////////////////////

static BOOL RegisterLibClass(HINSTANCE hInst)
{
WNDCLASS WndClass;
BOOL     fOk = TRUE;

MemSet(&WndClass, 0, sizeof(WNDCLASS));

WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditGeneralWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor(NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_GENERAL_CLASS_NAME;
if (!RegisterClass(&WndClass))
   fOk = FALSE;

#ifndef SS_NOCT_DATE
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditDateWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_DATE_CLASS_NAME;
if (!RegisterClass(&WndClass))
   fOk = FALSE;
#endif

#ifndef SS_NOCT_TIME
WndClass.style         = CS_GLOBALCLASS;
WndClass.lpfnWndProc   = (WNDPROC)EditTimeWndProc;
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_TIME_CLASS_NAME;
WndClass.cbWndExtra    = 2 * sizeof (HANDLE) + sizeof(FARPROC);
if (!RegisterClass(&WndClass))
   fOk = FALSE;
#endif

#ifndef SS_NOCT_INT
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditIntegerWndProc;
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_INTEGER_CLASS_NAME;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
if (!RegisterClass(&WndClass))
   fOk = FALSE;
#endif

#ifndef SS_NOCT_FLOAT
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditFloatWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_FLOAT_CLASS_NAME;
if (!RegisterClass(&WndClass))
   fOk = FALSE;
#endif

#ifndef SS_NOCT_PIC
/* RFW - 9/11/07 - 20794
WndClass.style         = CS_GLOBALCLASS;
*/
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditPicWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_PIC_CLASS_NAME;
if (!RegisterClass(&WndClass))
   fOk = FALSE;
#endif

#ifdef SS_V40
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditNumWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_NUM_CLASS_NAME;
if (!RegisterClass(&WndClass))
   fOk = FALSE;
#endif

// The Calendar Class:
#ifndef SS_NOCALENDAR
#ifndef SS_NOCT_DATE
//WndClass.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW | CS_GLOBALCLASS;
WndClass.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
WndClass.lpfnWndProc   = (WNDPROC)CalendarWndProc;
WndClass.cbClsExtra    = 0;
WndClass.cbWndExtra    = sizeof (HANDLE);
WndClass.hInstance     = hInst;
WndClass.hIcon         = NULL;
WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_CALENDAR_CLASS_NAME;
if (!RegisterClass(&WndClass))
   fOk = FALSE;
#endif
#endif

//if (!RegisterViewPict(hInst, _T(VA_VIEWPICT_CLASS_NAME)))
//   fOk = FALSE;
#ifndef SSP_NO_TYPESPIN
if (!RegisterSpinBtn(hInst, VA_SPINBTN_CLASS_NAME))
   fOk = FALSE;
#endif
if (!RegisterSpreadSheet(hInst, VA_SPREAD_CLASS_NAME, 0))
   fOk = FALSE;

#ifdef SS_V30
fOk = SSPreviewRegister(hInst);
#endif

return fOk;
}


static BOOL UnregisterLibClass(HINSTANCE hInst)
{
BOOL fOk = TRUE;

fOk = UnRegisterSpreadSheet(hInst, VA_SPREAD_CLASS_NAME);

fOk = UnregisterClass(VA_GENERAL_CLASS_NAME, hInst);

#ifndef SS_NOCT_DATE
fOk = UnregisterClass(VA_DATE_CLASS_NAME, hInst);
#endif

#ifndef SS_NOCT_TIME
fOk = UnregisterClass(VA_TIME_CLASS_NAME, hInst);
#endif

#ifndef SS_NOCT_INT
fOk = UnregisterClass(VA_INTEGER_CLASS_NAME, hInst);
#endif

#ifndef SS_NOCT_FLOAT
fOk = UnregisterClass(VA_FLOAT_CLASS_NAME, hInst);
#endif

#ifndef SS_NOCT_PIC
fOk = UnregisterClass(VA_PIC_CLASS_NAME, hInst);
#endif

#ifdef SS_V40
fOk = UnregisterClass(VA_NUM_CLASS_NAME, hInst);
#endif
// The Calendar Class:
#ifndef SS_NOCALENDAR
#ifndef SS_NOCT_DATE
fOk = UnregisterClass(VA_CALENDAR_CLASS_NAME, hInst);
#endif
#endif

//fOk = UnregisterClass(_T(VA_VIEWPICT_CLASS_NAME), hInst);

#ifndef SSP_NO_TYPESPIN
fOk = UnregisterClass(VA_SPINBTN_CLASS_NAME, hInst);
fOk = UnregisterClass(_T("tbSpinButton"), hInst);
#endif

#ifdef SS_V30
SSPreviewUnregister(hInst);
#endif

// return value of unregistering Spread
return fOk;

}


BOOL DLLENTRY SetToolBoxGlobal(int nID, LONG lValue)
{
switch (nID)
   {
   case TBG_CLEARFIELD:
      ToolBoxGlobal.ClearField = (int) lValue;
      return TRUE;

   case TBG_CURRENTDATETIME:
      ToolBoxGlobal.CurrentDateTimeKey = (int) lValue;
      return TRUE;

   case TBG_POPUPCALENDAR:
      ToolBoxGlobal.PopupCalendarKey = (int) lValue;
      return TRUE;

   default:
      return FALSE;
   }
}

BOOL DLLENTRY GetToolBoxGlobal (int nID, LPLONG lplValue)
{
switch (nID)
   {
   case TBG_CLEARFIELD:
      *lplValue = (LONG) ToolBoxGlobal.ClearField;
      return TRUE;

   case TBG_CURRENTDATETIME:
      *lplValue = (LONG) ToolBoxGlobal.CurrentDateTimeKey;
      return TRUE;

   case TBG_POPUPCALENDAR:
      *lplValue = (LONG) ToolBoxGlobal.PopupCalendarKey;
      return TRUE;

   default:
      return FALSE;
   }
}


void SpreadInit(HWND hWnd)
{
}

/*
short ViewGifLoadImage(HWND hWnd, LPVIEWPICT_PICTURE Picture,
                       LPCTSTR szGIFFile, LPBYTE GifResData,
                       long GifResDataSize)
{
return (0);
}
*/

short ViewPcxLoadImage(HWND hWnd, LPVIEWPICT_PICTURE Picture,
                       LPCTSTR FileName, LPBYTE PcxResourceData,
                       long PcxResourceDataSize)
{
return (0);
}


int WINAPI DialogBoxProc(HINSTANCE hInst, LPCTSTR lpszDlg, HWND hWnd,
                         DLGPROC DlgProc)
{
return (DialogBox(hInst, lpszDlg, hWnd, DlgProc));
}

#if (defined(SS_NOCALENDAR) || defined(SS_NOCT_DATE))
BOOL DLLENTRY PopupCalendar(HWND hDateWnd, BOOL fEnhanced)
{
return (0);
}
#endif

/*
// dual-interface attempt? not used -scl
#if 0 
/////////////////////////////////////////////////////////////////////////////
// _DSpread properties

short _DSpread::GetAction()
{
  short result;
  GetProperty(0x1, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetAction(short propVal)
{
  SetProperty(0x1, VT_I2, propVal);
}

long _DSpread::GetActiveCol()
{
  long result;
  GetProperty(0x2, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetActiveCol(long propVal)
{
  SetProperty(0x2, VT_I4, propVal);
}

long _DSpread::GetActiveRow()
{
  long result;
  GetProperty(0x3, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetActiveRow(long propVal)
{
  SetProperty(0x3, VT_I4, propVal);
}

BOOL _DSpread::GetAllowCellOverflow()
{
  BOOL result;
  GetProperty(0x4, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetAllowCellOverflow(BOOL propVal)
{
  SetProperty(0x4, VT_BOOL, propVal);
}

BOOL _DSpread::GetAllowDragDrop()
{
  BOOL result;
  GetProperty(0x5, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetAllowDragDrop(BOOL propVal)
{
  SetProperty(0x5, VT_BOOL, propVal);
}

BOOL _DSpread::GetAllowMultiBlocks()
{
  BOOL result;
  GetProperty(0x6, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetAllowMultiBlocks(BOOL propVal)
{
  SetProperty(0x6, VT_BOOL, propVal);
}

BOOL _DSpread::GetAllowUserFormulas()
{
  BOOL result;
  GetProperty(0x7, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetAllowUserFormulas(BOOL propVal)
{
  SetProperty(0x7, VT_BOOL, propVal);
}

BOOL _DSpread::GetArrowsExitEditMode()
{
  BOOL result;
  GetProperty(0x8, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetArrowsExitEditMode(BOOL propVal)
{
  SetProperty(0x8, VT_BOOL, propVal);
}

BOOL _DSpread::GetAutoCalc()
{
  BOOL result;
  GetProperty(0x9, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetAutoCalc(BOOL propVal)
{
  SetProperty(0x9, VT_BOOL, propVal);
}

BOOL _DSpread::GetAutoClipboard()
{
  BOOL result;
  GetProperty(0xa, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetAutoClipboard(BOOL propVal)
{
  SetProperty(0xa, VT_BOOL, propVal);
}

BOOL _DSpread::GetAutoSize()
{
  BOOL result;
  GetProperty(0xb, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetAutoSize(BOOL propVal)
{
  SetProperty(0xb, VT_BOOL, propVal);
}

BOOL _DSpread::GetBlockMode()
{
  BOOL result;
  GetProperty(0xc, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetBlockMode(BOOL propVal)
{
  SetProperty(0xc, VT_BOOL, propVal);
}

short _DSpread::GetButtonDrawMode()
{
  short result;
  GetProperty(0xd, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetButtonDrawMode(short propVal)
{
  SetProperty(0xd, VT_I2, propVal);
}

BOOL _DSpread::GetCalcDependencies()
{
  BOOL result;
  GetProperty(0xe, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetCalcDependencies(BOOL propVal)
{
  SetProperty(0xe, VT_BOOL, propVal);
}

short _DSpread::GetCellBorderStyle()
{
  short result;
  GetProperty(0x10, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetCellBorderStyle(short propVal)
{
  SetProperty(0x10, VT_I2, propVal);
}

short _DSpread::GetCellBorderType()
{
  short result;
  GetProperty(0x11, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetCellBorderType(short propVal)
{
  SetProperty(0x11, VT_I2, propVal);
}

short _DSpread::GetCellType()
{
  short result;
  GetProperty(0x12, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetCellType(short propVal)
{
  SetProperty(0x12, VT_I2, propVal);
}

BOOL _DSpread::GetChangeMade()
{
  BOOL result;
  GetProperty(0x13, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetChangeMade(BOOL propVal)
{
  SetProperty(0x13, VT_BOOL, propVal);
}

CString _DSpread::GetClip()
{
  CString result;
  GetProperty(0x14, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetClip(LPCTSTR propVal)
{
  SetProperty(0x14, VT_BSTR, propVal);
}

CString _DSpread::GetClipValue()
{
  CString result;
  GetProperty(0x15, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetClipValue(LPCTSTR propVal)
{
  SetProperty(0x15, VT_BSTR, propVal);
}

long _DSpread::GetCol()
{
  long result;
  GetProperty(0x16, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetCol(long propVal)
{
  SetProperty(0x16, VT_I4, propVal);
}

long _DSpread::GetCol2()
{
  long result;
  GetProperty(0x17, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetCol2(long propVal)
{
  SetProperty(0x17, VT_I4, propVal);
}

short _DSpread::GetColHeaderDisplay()
{
  short result;
  GetProperty(0x18, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetColHeaderDisplay(short propVal)
{
  SetProperty(0x18, VT_I2, propVal);
}

BOOL _DSpread::GetColHidden()
{
  BOOL result;
  GetProperty(0x19, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetColHidden(BOOL propVal)
{
  SetProperty(0x19, VT_BOOL, propVal);
}

BOOL _DSpread::GetColPageBreak()
{
  BOOL result;
  GetProperty(0x1a, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetColPageBreak(BOOL propVal)
{
  SetProperty(0x1a, VT_BOOL, propVal);
}

long _DSpread::GetColsFrozen()
{
  long result;
  GetProperty(0x1b, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetColsFrozen(long propVal)
{
  SetProperty(0x1b, VT_I4, propVal);
}

short _DSpread::GetCursorStyle()
{
  short result;
  GetProperty(0x1c, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetCursorStyle(short propVal)
{
  SetProperty(0x1c, VT_I2, propVal);
}

short _DSpread::GetCursorType()
{
  short result;
  GetProperty(0x1d, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetCursorType(short propVal)
{
  SetProperty(0x1d, VT_I2, propVal);
}

long _DSpread::GetDataColCnt()
{
  long result;
  GetProperty(0x1e, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetDataColCnt(long propVal)
{
  SetProperty(0x1e, VT_I4, propVal);
}

CString _DSpread::GetDataField()
{
  CString result;
  GetProperty(0x1f, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetDataField(LPCTSTR propVal)
{
  SetProperty(0x1f, VT_BSTR, propVal);
}

BOOL _DSpread::GetDataFillEvent()
{
  BOOL result;
  GetProperty(0x20, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDataFillEvent(BOOL propVal)
{
  SetProperty(0x20, VT_BOOL, propVal);
}

long _DSpread::GetDataRowCnt()
{
  long result;
  GetProperty(0x21, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetDataRowCnt(long propVal)
{
  SetProperty(0x21, VT_I4, propVal);
}

BOOL _DSpread::GetDAutoCellTypes()
{
  BOOL result;
  GetProperty(0x22, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDAutoCellTypes(BOOL propVal)
{
  SetProperty(0x22, VT_BOOL, propVal);
}

BOOL _DSpread::GetDAutoFill()
{
  BOOL result;
  GetProperty(0x23, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDAutoFill(BOOL propVal)
{
  SetProperty(0x23, VT_BOOL, propVal);
}

BOOL _DSpread::GetDAutoHeadings()
{
  BOOL result;
  GetProperty(0x24, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDAutoHeadings(BOOL propVal)
{
  SetProperty(0x24, VT_BOOL, propVal);
}

BOOL _DSpread::GetDAutoSave()
{
  BOOL result;
  GetProperty(0x25, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDAutoSave(BOOL propVal)
{
  SetProperty(0x25, VT_BOOL, propVal);
}

BOOL _DSpread::GetDAutoSizeCols()
{
  BOOL result;
  GetProperty(0x26, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDAutoSizeCols(BOOL propVal)
{
  SetProperty(0x26, VT_BOOL, propVal);
}

long _DSpread::GetDestCol()
{
  long result;
  GetProperty(0x27, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetDestCol(long propVal)
{
  SetProperty(0x27, VT_I4, propVal);
}

long _DSpread::GetDestRow()
{
  long result;
  GetProperty(0x28, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetDestRow(long propVal)
{
  SetProperty(0x28, VT_I4, propVal);
}

BOOL _DSpread::GetDInformActiveRowChange()
{
  BOOL result;
  GetProperty(0x29, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDInformActiveRowChange(BOOL propVal)
{
  SetProperty(0x29, VT_BOOL, propVal);
}

BOOL _DSpread::GetDisplayColHeaders()
{
  BOOL result;
  GetProperty(0x2a, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDisplayColHeaders(BOOL propVal)
{
  SetProperty(0x2a, VT_BOOL, propVal);
}

BOOL _DSpread::GetDisplayRowHeaders()
{
  BOOL result;
  GetProperty(0x2b, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetDisplayRowHeaders(BOOL propVal)
{
  SetProperty(0x2b, VT_BOOL, propVal);
}

short _DSpread::GetEditEnterAction()
{
  short result;
  GetProperty(0x2c, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetEditEnterAction(short propVal)
{
  SetProperty(0x2c, VT_I2, propVal);
}

BOOL _DSpread::GetEditMode()
{
  BOOL result;
  GetProperty(0x2d, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetEditMode(BOOL propVal)
{
  SetProperty(0x2d, VT_BOOL, propVal);
}

BOOL _DSpread::GetEditModePermanent()
{
  BOOL result;
  GetProperty(0x2e, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetEditModePermanent(BOOL propVal)
{
  SetProperty(0x2e, VT_BOOL, propVal);
}

BOOL _DSpread::GetEditModeReplace()
{
  BOOL result;
  GetProperty(0x2f, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetEditModeReplace(BOOL propVal)
{
  SetProperty(0x2f, VT_BOOL, propVal);
}

short _DSpread::GetFileNum()
{
  short result;
  GetProperty(0x30, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetFileNum(short propVal)
{
  SetProperty(0x30, VT_I2, propVal);
}

short _DSpread::GetFloatDefCurrencyChar()
{
  short result;
  GetProperty(0x31, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetFloatDefCurrencyChar(short propVal)
{
  SetProperty(0x31, VT_I2, propVal);
}

short _DSpread::GetFloatDefDecimalChar()
{
  short result;
  GetProperty(0x32, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetFloatDefDecimalChar(short propVal)
{
  SetProperty(0x32, VT_I2, propVal);
}

short _DSpread::GetFloatDefSepChar()
{
  short result;
  GetProperty(0x33, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetFloatDefSepChar(short propVal)
{
  SetProperty(0x33, VT_I2, propVal);
}

CString _DSpread::GetFormula()
{
  CString result;
  GetProperty(0x34, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetFormula(LPCTSTR propVal)
{
  SetProperty(0x34, VT_BSTR, propVal);
}

BOOL _DSpread::GetGridShowHoriz()
{
  BOOL result;
  GetProperty(0x37, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetGridShowHoriz(BOOL propVal)
{
  SetProperty(0x37, VT_BOOL, propVal);
}

BOOL _DSpread::GetGridShowVert()
{
  BOOL result;
  GetProperty(0x38, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetGridShowVert(BOOL propVal)
{
  SetProperty(0x38, VT_BOOL, propVal);
}

BOOL _DSpread::GetGridSolid()
{
  BOOL result;
  GetProperty(0x39, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetGridSolid(BOOL propVal)
{
  SetProperty(0x39, VT_BOOL, propVal);
}

short _DSpread::GetHDCPrinter()
{
  short result;
  GetProperty(0x3a, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetHDCPrinter(short propVal)
{
  SetProperty(0x3a, VT_I2, propVal);
}

short _DSpread::GetInterfaceDesigner()
{
  short result;
  GetProperty(0x3b, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetInterfaceDesigner(short propVal)
{
  SetProperty(0x3b, VT_I2, propVal);
}

BOOL _DSpread::GetIsBlockSelected()
{
  BOOL result;
  GetProperty(0x3c, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetIsBlockSelected(BOOL propVal)
{
  SetProperty(0x3c, VT_BOOL, propVal);
}

long _DSpread::GetLeftCol()
{
  long result;
  GetProperty(0x3d, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetLeftCol(long propVal)
{
  SetProperty(0x3d, VT_I4, propVal);
}

BOOL _DSpread::GetLock()
{
  BOOL result;
  GetProperty(0x3e, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetLock(BOOL propVal)
{
  SetProperty(0x3e, VT_BOOL, propVal);
}

long _DSpread::GetMaxCols()
{
  long result;
  GetProperty(0x41, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetMaxCols(long propVal)
{
  SetProperty(0x41, VT_I4, propVal);
}

long _DSpread::GetMaxRows()
{
  long result;
  GetProperty(0x42, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetMaxRows(long propVal)
{
  SetProperty(0x42, VT_I4, propVal);
}

double _DSpread::GetMaxTextCellHeight()
{
  double result;
  GetProperty(0x43, VT_R8, (void*)&result);
  return result;
}

void _DSpread::SetMaxTextCellHeight(double propVal)
{
  SetProperty(0x43, VT_R8, propVal);
}

double _DSpread::GetMaxTextCellWidth()
{
  double result;
  GetProperty(0x44, VT_R8, (void*)&result);
  return result;
}

void _DSpread::SetMaxTextCellWidth(double propVal)
{
  SetProperty(0x44, VT_R8, propVal);
}

BOOL _DSpread::GetMoveActiveOnFocus()
{
  BOOL result;
  GetProperty(0x45, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetMoveActiveOnFocus(BOOL propVal)
{
  SetProperty(0x45, VT_BOOL, propVal);
}

long _DSpread::GetMultiSelCount()
{
  long result;
  GetProperty(0x46, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetMultiSelCount(long propVal)
{
  SetProperty(0x46, VT_I4, propVal);
}

long _DSpread::GetMultiSelIndex()
{
  long result;
  GetProperty(0x47, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetMultiSelIndex(long propVal)
{
  SetProperty(0x47, VT_I4, propVal);
}

BOOL _DSpread::GetNoBeep()
{
  BOOL result;
  GetProperty(0x48, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetNoBeep(BOOL propVal)
{
  SetProperty(0x48, VT_BOOL, propVal);
}

BOOL _DSpread::GetNoBorder()
{
  BOOL result;
  GetProperty(0x49, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetNoBorder(BOOL propVal)
{
  SetProperty(0x49, VT_BOOL, propVal);
}

short _DSpread::GetOperationMode()
{
  short result;
  GetProperty(0x4a, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetOperationMode(short propVal)
{
  SetProperty(0x4a, VT_I2, propVal);
}

short _DSpread::GetPosition()
{
  short result;
  GetProperty(0x4b, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetPosition(short propVal)
{
  SetProperty(0x4b, VT_I2, propVal);
}

CString _DSpread::GetPrintAbortMsg()
{
  CString result;
  GetProperty(0x4c, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetPrintAbortMsg(LPCTSTR propVal)
{
  SetProperty(0x4c, VT_BSTR, propVal);
}

BOOL _DSpread::GetPrintBorder()
{
  BOOL result;
  GetProperty(0x4d, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetPrintBorder(BOOL propVal)
{
  SetProperty(0x4d, VT_BOOL, propVal);
}

BOOL _DSpread::GetPrintColHeaders()
{
  BOOL result;
  GetProperty(0x4e, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetPrintColHeaders(BOOL propVal)
{
  SetProperty(0x4e, VT_BOOL, propVal);
}

BOOL _DSpread::GetPrintColor()
{
  BOOL result;
  GetProperty(0x4f, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetPrintColor(BOOL propVal)
{
  SetProperty(0x4f, VT_BOOL, propVal);
}

CString _DSpread::GetPrintFooter()
{
  CString result;
  GetProperty(0x50, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetPrintFooter(LPCTSTR propVal)
{
  SetProperty(0x50, VT_BSTR, propVal);
}

BOOL _DSpread::GetPrintGrid()
{
  BOOL result;
  GetProperty(0x51, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetPrintGrid(BOOL propVal)
{
  SetProperty(0x51, VT_BOOL, propVal);
}

CString _DSpread::GetPrintHeader()
{
  CString result;
  GetProperty(0x52, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetPrintHeader(LPCTSTR propVal)
{
  SetProperty(0x52, VT_BSTR, propVal);
}

CString _DSpread::GetPrintJobName()
{
  CString result;
  GetProperty(0x53, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetPrintJobName(LPCTSTR propVal)
{
  SetProperty(0x53, VT_BSTR, propVal);
}

long _DSpread::GetPrintMarginBottom()
{
  long result;
  GetProperty(0x54, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetPrintMarginBottom(long propVal)
{
  SetProperty(0x54, VT_I4, propVal);
}

long _DSpread::GetPrintMarginLeft()
{
  long result;
  GetProperty(0x55, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetPrintMarginLeft(long propVal)
{
  SetProperty(0x55, VT_I4, propVal);
}

long _DSpread::GetPrintMarginRight()
{
  long result;
  GetProperty(0x56, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetPrintMarginRight(long propVal)
{
  SetProperty(0x56, VT_I4, propVal);
}

long _DSpread::GetPrintMarginTop()
{
  long result;
  GetProperty(0x57, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetPrintMarginTop(long propVal)
{
  SetProperty(0x57, VT_I4, propVal);
}

short _DSpread::GetPrintPageEnd()
{
  short result;
  GetProperty(0x58, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetPrintPageEnd(short propVal)
{
  SetProperty(0x58, VT_I2, propVal);
}

short _DSpread::GetPrintPageStart()
{
  short result;
  GetProperty(0x59, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetPrintPageStart(short propVal)
{
  SetProperty(0x59, VT_I2, propVal);
}

BOOL _DSpread::GetPrintRowHeaders()
{
  BOOL result;
  GetProperty(0x5a, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetPrintRowHeaders(BOOL propVal)
{
  SetProperty(0x5a, VT_BOOL, propVal);
}

BOOL _DSpread::GetPrintShadows()
{
  BOOL result;
  GetProperty(0x5b, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetPrintShadows(BOOL propVal)
{
  SetProperty(0x5b, VT_BOOL, propVal);
}

short _DSpread::GetPrintType()
{
  short result;
  GetProperty(0x5c, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetPrintType(short propVal)
{
  SetProperty(0x5c, VT_I2, propVal);
}

BOOL _DSpread::GetPrintUseDataMax()
{
  BOOL result;
  GetProperty(0x5d, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetPrintUseDataMax(BOOL propVal)
{
  SetProperty(0x5d, VT_BOOL, propVal);
}

BOOL _DSpread::GetProcessTab()
{
  BOOL result;
  GetProperty(0x5e, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetProcessTab(BOOL propVal)
{
  SetProperty(0x5e, VT_BOOL, propVal);
}

BOOL _DSpread::GetProtect()
{
  BOOL result;
  GetProperty(0x5f, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetProtect(BOOL propVal)
{
  SetProperty(0x5f, VT_BOOL, propVal);
}

BOOL _DSpread::GetReDraw()
{
  BOOL result;
  GetProperty(0x60, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetReDraw(BOOL propVal)
{
  SetProperty(0x60, VT_BOOL, propVal);
}

BOOL _DSpread::GetRestrictCols()
{
  BOOL result;
  GetProperty(0x61, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetRestrictCols(BOOL propVal)
{
  SetProperty(0x61, VT_BOOL, propVal);
}

BOOL _DSpread::GetRestrictRows()
{
  BOOL result;
  GetProperty(0x62, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetRestrictRows(BOOL propVal)
{
  SetProperty(0x62, VT_BOOL, propVal);
}

BOOL _DSpread::GetRetainSelBlock()
{
  BOOL result;
  GetProperty(0x63, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetRetainSelBlock(BOOL propVal)
{
  SetProperty(0x63, VT_BOOL, propVal);
}

long _DSpread::GetRow()
{
  long result;
  GetProperty(0x64, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetRow(long propVal)
{
  SetProperty(0x64, VT_I4, propVal);
}

long _DSpread::GetRow2()
{
  long result;
  GetProperty(0x65, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetRow2(long propVal)
{
  SetProperty(0x65, VT_I4, propVal);
}

short _DSpread::GetRowHeaderDisplay()
{
  short result;
  GetProperty(0x66, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetRowHeaderDisplay(short propVal)
{
  SetProperty(0x66, VT_I2, propVal);
}

BOOL _DSpread::GetRowHidden()
{
  BOOL result;
  GetProperty(0x67, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetRowHidden(BOOL propVal)
{
  SetProperty(0x67, VT_BOOL, propVal);
}

BOOL _DSpread::GetRowPageBreak()
{
  BOOL result;
  GetProperty(0x68, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetRowPageBreak(BOOL propVal)
{
  SetProperty(0x68, VT_BOOL, propVal);
}

long _DSpread::GetRowsFrozen()
{
  long result;
  GetProperty(0x69, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetRowsFrozen(long propVal)
{
  SetProperty(0x69, VT_I4, propVal);
}

BOOL _DSpread::GetScrollBarExtMode()
{
  BOOL result;
  GetProperty(0x6a, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetScrollBarExtMode(BOOL propVal)
{
  SetProperty(0x6a, VT_BOOL, propVal);
}

BOOL _DSpread::GetScrollBarMaxAlign()
{
  BOOL result;
  GetProperty(0x6b, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetScrollBarMaxAlign(BOOL propVal)
{
  SetProperty(0x6b, VT_BOOL, propVal);
}

short _DSpread::GetScrollBars()
{
  short result;
  GetProperty(0x6c, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetScrollBars(short propVal)
{
  SetProperty(0x6c, VT_I2, propVal);
}

BOOL _DSpread::GetScrollBarShowMax()
{
  BOOL result;
  GetProperty(0x6d, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetScrollBarShowMax(BOOL propVal)
{
  SetProperty(0x6d, VT_BOOL, propVal);
}

long _DSpread::GetSelBlockCol()
{
  long result;
  GetProperty(0x6e, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetSelBlockCol(long propVal)
{
  SetProperty(0x6e, VT_I4, propVal);
}

long _DSpread::GetSelBlockCol2()
{
  long result;
  GetProperty(0x6f, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetSelBlockCol2(long propVal)
{
  SetProperty(0x6f, VT_I4, propVal);
}

long _DSpread::GetSelBlockRow()
{
  long result;
  GetProperty(0x70, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetSelBlockRow(long propVal)
{
  SetProperty(0x70, VT_I4, propVal);
}

long _DSpread::GetSelBlockRow2()
{
  long result;
  GetProperty(0x71, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetSelBlockRow2(long propVal)
{
  SetProperty(0x71, VT_I4, propVal);
}

short _DSpread::GetSelectBlockOptions()
{
  short result;
  GetProperty(0x72, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetSelectBlockOptions(short propVal)
{
  SetProperty(0x72, VT_I2, propVal);
}

long _DSpread::GetSelLength()
{
  long result;
  GetProperty(0x73, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetSelLength(long propVal)
{
  SetProperty(0x73, VT_I4, propVal);
}

long _DSpread::GetSelModeIndex()
{
  long result;
  GetProperty(0x74, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetSelModeIndex(long propVal)
{
  SetProperty(0x74, VT_I4, propVal);
}

long _DSpread::GetSelModeSelCount()
{
  long result;
  GetProperty(0x75, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetSelModeSelCount(long propVal)
{
  SetProperty(0x75, VT_I4, propVal);
}

BOOL _DSpread::GetSelModeSelected()
{
  BOOL result;
  GetProperty(0x76, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetSelModeSelected(BOOL propVal)
{
  SetProperty(0x76, VT_BOOL, propVal);
}

long _DSpread::GetSelStart()
{
  long result;
  GetProperty(0x77, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetSelStart(long propVal)
{
  SetProperty(0x77, VT_I4, propVal);
}

CString _DSpread::GetSelText()
{
  CString result;
  GetProperty(0x78, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetSelText(LPCTSTR propVal)
{
  SetProperty(0x78, VT_BSTR, propVal);
}

short _DSpread::GetSortBy()
{
  short result;
  GetProperty(0x7c, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetSortBy(short propVal)
{
  SetProperty(0x7c, VT_I2, propVal);
}

long _DSpread::GetStartingColNumber()
{
  long result;
  GetProperty(0x7d, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetStartingColNumber(long propVal)
{
  SetProperty(0x7d, VT_I4, propVal);
}

long _DSpread::GetStartingRowNumber()
{
  long result;
  GetProperty(0x7e, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetStartingRowNumber(long propVal)
{
  SetProperty(0x7e, VT_I4, propVal);
}

long _DSpread::GetTopRow()
{
  long result;
  GetProperty(0x7f, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetTopRow(long propVal)
{
  SetProperty(0x7f, VT_I4, propVal);
}

short _DSpread::GetTypeButtonAlign()
{
  short result;
  GetProperty(0x80, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeButtonAlign(short propVal)
{
  SetProperty(0x80, VT_I2, propVal);
}

short _DSpread::GetTypeButtonShadowSize()
{
  short result;
  GetProperty(0x87, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeButtonShadowSize(short propVal)
{
  SetProperty(0x87, VT_I2, propVal);
}

CString _DSpread::GetTypeButtonText()
{
  CString result;
  GetProperty(0x88, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypeButtonText(LPCTSTR propVal)
{
  SetProperty(0x88, VT_BSTR, propVal);
}

short _DSpread::GetTypeButtonType()
{
  short result;
  GetProperty(0x8a, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeButtonType(short propVal)
{
  SetProperty(0x8a, VT_I2, propVal);
}

BOOL _DSpread::GetTypeCheckCenter()
{
  BOOL result;
  GetProperty(0x8b, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeCheckCenter(BOOL propVal)
{
  SetProperty(0x8b, VT_BOOL, propVal);
}

CString _DSpread::GetTypeCheckText()
{
  CString result;
  GetProperty(0x8c, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypeCheckText(LPCTSTR propVal)
{
  SetProperty(0x8c, VT_BSTR, propVal);
}

short _DSpread::GetTypeCheckTextAlign()
{
  short result;
  GetProperty(0x8d, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeCheckTextAlign(short propVal)
{
  SetProperty(0x8d, VT_I2, propVal);
}

short _DSpread::GetTypeComboBoxCount()
{
  short result;
  GetProperty(0x8e, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeComboBoxCount(short propVal)
{
  SetProperty(0x8e, VT_I2, propVal);
}

short _DSpread::GetTypeComboBoxCurSel()
{
  short result;
  GetProperty(0x8f, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeComboBoxCurSel(short propVal)
{
  SetProperty(0x8f, VT_I2, propVal);
}

BOOL _DSpread::GetTypeComboBoxEditable()
{
  BOOL result;
  GetProperty(0x90, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeComboBoxEditable(BOOL propVal)
{
  SetProperty(0x90, VT_BOOL, propVal);
}

short _DSpread::GetTypeComboBoxIndex()
{
  short result;
  GetProperty(0x91, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeComboBoxIndex(short propVal)
{
  SetProperty(0x91, VT_I2, propVal);
}

CString _DSpread::GetTypeComboBoxList()
{
  CString result;
  GetProperty(0x92, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypeComboBoxList(LPCTSTR propVal)
{
  SetProperty(0x92, VT_BSTR, propVal);
}

CString _DSpread::GetTypeComboBoxString()
{
  CString result;
  GetProperty(0x93, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypeComboBoxString(LPCTSTR propVal)
{
  SetProperty(0x93, VT_BSTR, propVal);
}

BOOL _DSpread::GetTypeDateCentury()
{
  BOOL result;
  GetProperty(0x94, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeDateCentury(BOOL propVal)
{
  SetProperty(0x94, VT_BOOL, propVal);
}

short _DSpread::GetTypeDateFormat()
{
  short result;
  GetProperty(0x95, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeDateFormat(short propVal)
{
  SetProperty(0x95, VT_I2, propVal);
}

CString _DSpread::GetTypeDateMax()
{
  CString result;
  GetProperty(0x96, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypeDateMax(LPCTSTR propVal)
{
  SetProperty(0x96, VT_BSTR, propVal);
}

CString _DSpread::GetTypeDateMin()
{
  CString result;
  GetProperty(0x97, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypeDateMin(LPCTSTR propVal)
{
  SetProperty(0x97, VT_BSTR, propVal);
}

short _DSpread::GetTypeDateSeparator()
{
  short result;
  GetProperty(0x98, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeDateSeparator(short propVal)
{
  SetProperty(0x98, VT_I2, propVal);
}

short _DSpread::GetTypeEditCharCase()
{
  short result;
  GetProperty(0x99, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeEditCharCase(short propVal)
{
  SetProperty(0x99, VT_I2, propVal);
}

short _DSpread::GetTypeEditCharSet()
{
  short result;
  GetProperty(0x9a, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeEditCharSet(short propVal)
{
  SetProperty(0x9a, VT_I2, propVal);
}

long _DSpread::GetTypeEditLen()
{
  long result;
  GetProperty(0x9b, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetTypeEditLen(long propVal)
{
  SetProperty(0x9b, VT_I4, propVal);
}

BOOL _DSpread::GetTypeEditMultiLine()
{
  BOOL result;
  GetProperty(0x9c, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeEditMultiLine(BOOL propVal)
{
  SetProperty(0x9c, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypeEditPassword()
{
  BOOL result;
  GetProperty(0x9d, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeEditPassword(BOOL propVal)
{
  SetProperty(0x9d, VT_BOOL, propVal);
}

short _DSpread::GetTypeFloatCurrencyChar()
{
  short result;
  GetProperty(0x9e, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeFloatCurrencyChar(short propVal)
{
  SetProperty(0x9e, VT_I2, propVal);
}

short _DSpread::GetTypeFloatDecimalChar()
{
  short result;
  GetProperty(0x9f, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeFloatDecimalChar(short propVal)
{
  SetProperty(0x9f, VT_I2, propVal);
}

short _DSpread::GetTypeFloatDecimalPlaces()
{
  short result;
  GetProperty(0xa0, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeFloatDecimalPlaces(short propVal)
{
  SetProperty(0xa0, VT_I2, propVal);
}

double _DSpread::GetTypeFloatMax()
{
  double result;
  GetProperty(0xa1, VT_R8, (void*)&result);
  return result;
}

void _DSpread::SetTypeFloatMax(double propVal)
{
  SetProperty(0xa1, VT_R8, propVal);
}

double _DSpread::GetTypeFloatMin()
{
  double result;
  GetProperty(0xa2, VT_R8, (void*)&result);
  return result;
}

void _DSpread::SetTypeFloatMin(double propVal)
{
  SetProperty(0xa2, VT_R8, propVal);
}

BOOL _DSpread::GetTypeFloatMoney()
{
  BOOL result;
  GetProperty(0xa3, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeFloatMoney(BOOL propVal)
{
  SetProperty(0xa3, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypeFloatSeparator()
{
  BOOL result;
  GetProperty(0xa4, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeFloatSeparator(BOOL propVal)
{
  SetProperty(0xa4, VT_BOOL, propVal);
}

short _DSpread::GetTypeFloatSepChar()
{
  short result;
  GetProperty(0xa5, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeFloatSepChar(short propVal)
{
  SetProperty(0xa5, VT_I2, propVal);
}

short _DSpread::GetTypeHAlign()
{
  short result;
  GetProperty(0xa6, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeHAlign(short propVal)
{
  SetProperty(0xa6, VT_I2, propVal);
}

long _DSpread::GetTypeIntegerMax()
{
  long result;
  GetProperty(0xa7, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetTypeIntegerMax(long propVal)
{
  SetProperty(0xa7, VT_I4, propVal);
}

long _DSpread::GetTypeIntegerMin()
{
  long result;
  GetProperty(0xa8, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetTypeIntegerMin(long propVal)
{
  SetProperty(0xa8, VT_I4, propVal);
}

long _DSpread::GetTypeIntegerSpinInc()
{
  long result;
  GetProperty(0xa9, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetTypeIntegerSpinInc(long propVal)
{
  SetProperty(0xa9, VT_I4, propVal);
}

BOOL _DSpread::GetTypeIntegerSpinWrap()
{
  BOOL result;
  GetProperty(0xaa, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeIntegerSpinWrap(BOOL propVal)
{
  SetProperty(0xaa, VT_BOOL, propVal);
}

long _DSpread::GetTypeOwnerDrawStyle()
{
  long result;
  GetProperty(0xab, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetTypeOwnerDrawStyle(long propVal)
{
  SetProperty(0xab, VT_I4, propVal);
}

CString _DSpread::GetTypePicDefaultText()
{
  CString result;
  GetProperty(0xac, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypePicDefaultText(LPCTSTR propVal)
{
  SetProperty(0xac, VT_BSTR, propVal);
}

CString _DSpread::GetTypePicMask()
{
  CString result;
  GetProperty(0xad, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypePicMask(LPCTSTR propVal)
{
  SetProperty(0xad, VT_BSTR, propVal);
}

BOOL _DSpread::GetTypePictCenter()
{
  BOOL result;
  GetProperty(0xae, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypePictCenter(BOOL propVal)
{
  SetProperty(0xae, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypePictMaintainScale()
{
  BOOL result;
  GetProperty(0xaf, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypePictMaintainScale(BOOL propVal)
{
  SetProperty(0xaf, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypePictStretch()
{
  BOOL result;
  GetProperty(0xb1, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypePictStretch(BOOL propVal)
{
  SetProperty(0xb1, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypeSpin()
{
  BOOL result;
  GetProperty(0xb2, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeSpin(BOOL propVal)
{
  SetProperty(0xb2, VT_BOOL, propVal);
}

short _DSpread::GetTypeTextAlignVert()
{
  short result;
  GetProperty(0xb3, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeTextAlignVert(short propVal)
{
  SetProperty(0xb3, VT_I2, propVal);
}

BOOL _DSpread::GetTypeTextPrefix()
{
  BOOL result;
  GetProperty(0xb4, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeTextPrefix(BOOL propVal)
{
  SetProperty(0xb4, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypeTextShadow()
{
  BOOL result;
  GetProperty(0xb5, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeTextShadow(BOOL propVal)
{
  SetProperty(0xb5, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypeTextShadowIn()
{
  BOOL result;
  GetProperty(0xb6, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeTextShadowIn(BOOL propVal)
{
  SetProperty(0xb6, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypeTextWordWrap()
{
  BOOL result;
  GetProperty(0xb7, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeTextWordWrap(BOOL propVal)
{
  SetProperty(0xb7, VT_BOOL, propVal);
}

BOOL _DSpread::GetTypeTime24Hour()
{
  BOOL result;
  GetProperty(0xb8, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeTime24Hour(BOOL propVal)
{
  SetProperty(0xb8, VT_BOOL, propVal);
}

CString _DSpread::GetTypeTimeMax()
{
  CString result;
  GetProperty(0xb9, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypeTimeMax(LPCTSTR propVal)
{
  SetProperty(0xb9, VT_BSTR, propVal);
}

CString _DSpread::GetTypeTimeMin()
{
  CString result;
  GetProperty(0xba, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetTypeTimeMin(LPCTSTR propVal)
{
  SetProperty(0xba, VT_BSTR, propVal);
}

BOOL _DSpread::GetTypeTimeSeconds()
{
  BOOL result;
  GetProperty(0xbb, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetTypeTimeSeconds(BOOL propVal)
{
  SetProperty(0xbb, VT_BOOL, propVal);
}

short _DSpread::GetTypeTimeSeparator()
{
  short result;
  GetProperty(0xbc, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetTypeTimeSeparator(short propVal)
{
  SetProperty(0xbc, VT_I2, propVal);
}

short _DSpread::GetUnitType()
{
  short result;
  GetProperty(0xbd, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetUnitType(short propVal)
{
  SetProperty(0xbd, VT_I2, propVal);
}

short _DSpread::GetUserResize()
{
  short result;
  GetProperty(0xbe, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetUserResize(short propVal)
{
  SetProperty(0xbe, VT_I2, propVal);
}

short _DSpread::GetUserResizeCol()
{
  short result;
  GetProperty(0xbf, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetUserResizeCol(short propVal)
{
  SetProperty(0xbf, VT_I2, propVal);
}

short _DSpread::GetUserResizeRow()
{
  short result;
  GetProperty(0xc0, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetUserResizeRow(short propVal)
{
  SetProperty(0xc0, VT_I2, propVal);
}

CString _DSpread::GetValue()
{
  CString result;
  GetProperty(0xc1, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetValue(LPCTSTR propVal)
{
  SetProperty(0xc1, VT_BSTR, propVal);
}

long _DSpread::GetVirtualCurRowCount()
{
  long result;
  GetProperty(0xc2, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetVirtualCurRowCount(long propVal)
{
  SetProperty(0xc2, VT_I4, propVal);
}

long _DSpread::GetVirtualCurTop()
{
  long result;
  GetProperty(0xc3, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetVirtualCurTop(long propVal)
{
  SetProperty(0xc3, VT_I4, propVal);
}

long _DSpread::GetVirtualMaxRows()
{
  long result;
  GetProperty(0xc4, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetVirtualMaxRows(long propVal)
{
  SetProperty(0xc4, VT_I4, propVal);
}

BOOL _DSpread::GetVirtualMode()
{
  BOOL result;
  GetProperty(0xc5, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetVirtualMode(BOOL propVal)
{
  SetProperty(0xc5, VT_BOOL, propVal);
}

long _DSpread::GetVirtualOverlap()
{
  long result;
  GetProperty(0xc6, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetVirtualOverlap(long propVal)
{
  SetProperty(0xc6, VT_I4, propVal);
}

long _DSpread::GetVirtualRows()
{
  long result;
  GetProperty(0xc7, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetVirtualRows(long propVal)
{
  SetProperty(0xc7, VT_I4, propVal);
}

BOOL _DSpread::GetVirtualScrollBuffer()
{
  BOOL result;
  GetProperty(0xc8, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetVirtualScrollBuffer(BOOL propVal)
{
  SetProperty(0xc8, VT_BOOL, propVal);
}

long _DSpread::GetVisibleCols()
{
  long result;
  GetProperty(0xc9, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetVisibleCols(long propVal)
{
  SetProperty(0xc9, VT_I4, propVal);
}

long _DSpread::GetVisibleRows()
{
  long result;
  GetProperty(0xca, VT_I4, (void*)&result);
  return result;
}

void _DSpread::SetVisibleRows(long propVal)
{
  SetProperty(0xca, VT_I4, propVal);
}

BOOL _DSpread::GetVScrollSpecial()
{
  BOOL result;
  GetProperty(0xcb, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetVScrollSpecial(BOOL propVal)
{
  SetProperty(0xcb, VT_BOOL, propVal);
}

short _DSpread::GetVScrollSpecialType()
{
  short result;
  GetProperty(0xcc, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetVScrollSpecialType(short propVal)
{
  SetProperty(0xcc, VT_I2, propVal);
}

short _DSpread::GetBorderStyle()
{
  short result;
  GetProperty(0xfffffe08, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetBorderStyle(short propVal)
{
  SetProperty(0xfffffe08, VT_I2, propVal);
}

BOOL _DSpread::GetEnabled()
{
  BOOL result;
  GetProperty(0xfffffdfe, VT_BOOL, (void*)&result);
  return result;
}

void _DSpread::SetEnabled(BOOL propVal)
{
  SetProperty(0xfffffdfe, VT_BOOL, propVal);
}

short _DSpread::GetHWnd()
{
  short result;
  GetProperty(0xfffffdfd, VT_I2, (void*)&result);
  return result;
}

void _DSpread::SetHWnd(short propVal)
{
  SetProperty(0xfffffdfd, VT_I2, propVal);
}

CString _DSpread::GetText()
{
  CString result;
  GetProperty(0xfffffdfb, VT_BSTR, (void*)&result);
  return result;
}

void _DSpread::SetText(LPCTSTR propVal)
{
  SetProperty(0xfffffdfb, VT_BSTR, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// _DSpread operations

double _DSpread::GetColWidth(long lCol)
{
  double result;
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
    lCol);
  return result;
}

void _DSpread::SetColWidth(long lCol, double newValue)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_R8;
  InvokeHelper(0xcd, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
     lCol, newValue);
}

double _DSpread::GetMaxTextColWidth(long lCol)
{
  double result;
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xce, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
    lCol);
  return result;
}

void _DSpread::SetMaxTextColWidth(long lCol, double newValue)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_R8;
  InvokeHelper(0xce, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
     lCol, newValue);
}

double _DSpread::GetMaxTextRowHeight(long lRow)
{
  double result;
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xcf, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
    lRow);
  return result;
}

void _DSpread::SetMaxTextRowHeight(long lRow, double newValue)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_R8;
  InvokeHelper(0xcf, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
     lRow, newValue);
}

double _DSpread::GetRowHeight(long lRow)
{
  double result;
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xd0, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
    lRow);
  return result;
}

void _DSpread::SetRowHeight(long lRow, double newValue)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_R8;
  InvokeHelper(0xd0, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
     lRow, newValue);
}

long _DSpread::GetSortKey(short nIndex)
{
  long result;
  static BYTE BASED_CODE parms[] =
    VTS_I2;
  InvokeHelper(0xd1, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, parms,
    nIndex);
  return result;
}

void _DSpread::SetSortKey(short nIndex, long nNewValue)
{
  static BYTE BASED_CODE parms[] =
    VTS_I2 VTS_I4;
  InvokeHelper(0xd1, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
     nIndex, nNewValue);
}

short _DSpread::GetSortKeyOrder(short nIndex)
{
  short result;
  static BYTE BASED_CODE parms[] =
    VTS_I2;
  InvokeHelper(0xd2, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, parms,
    nIndex);
  return result;
}

void _DSpread::SetSortKeyOrder(short nIndex, short nNewValue)
{
  static BYTE BASED_CODE parms[] =
    VTS_I2 VTS_I2;
  InvokeHelper(0xd2, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
     nIndex, nNewValue);
}

BOOL _DSpread::AddCustomFunction(LPCTSTR FunctionName, short ParameterCnt)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_BSTR VTS_I2;
  InvokeHelper(0xd4, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    FunctionName, ParameterCnt);
  return result;
}

double _DSpread::CFGetDoubleParam(short Param)
{
  double result;
  static BYTE BASED_CODE parms[] =
    VTS_I2;
  InvokeHelper(0xd5, DISPATCH_METHOD, VT_R8, (void*)&result, parms,
    Param);
  return result;
}

long _DSpread::CFGetLongParam(short Param)
{
  long result;
  static BYTE BASED_CODE parms[] =
    VTS_I2;
  InvokeHelper(0xd6, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    Param);
  return result;
}

BOOL _DSpread::CFGetParamInfo(short Param, short* Type, short* Status)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_I2 VTS_PI2 VTS_PI2;
  InvokeHelper(0xd7, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Param, Type, Status);
  return result;
}

CString _DSpread::CFGetStringParam(short Param)
{
  CString result;
  static BYTE BASED_CODE parms[] =
    VTS_I2;
  InvokeHelper(0xd8, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
    Param);
  return result;
}

void _DSpread::CFSetResult(VARIANT* Var)
{
  static BYTE BASED_CODE parms[] =
    VTS_PVARIANT;
  InvokeHelper(0xd9, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Var);
}

CString _DSpread::ColNumberToLetter(long HeaderNumber)
{
  CString result;
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xda, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
    HeaderNumber);
  return result;
}

void _DSpread::ColWidthToTwips(float Width, long* Twips)
{
  static BYTE BASED_CODE parms[] =
    VTS_R4 VTS_PI4;
  InvokeHelper(0xdb, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Width, Twips);
}

void _DSpread::GetBottomRightCell(long* Col, long* Row)
{
  static BYTE BASED_CODE parms[] =
    VTS_PI4 VTS_PI4;
  InvokeHelper(0xdc, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row);
}

BOOL _DSpread::GetCellDirtyFlag(long Col, long Row)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0xdd, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Col, Row);
  return result;
}

void _DSpread::GetCellFromScreenCoord(long* Col, long* Row, long x, long y)
{
  static BYTE BASED_CODE parms[] =
    VTS_PI4 VTS_PI4 VTS_I4 VTS_I4;
  InvokeHelper(0xde, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, x, y);
}

BOOL _DSpread::GetCellPos(long Col, long Row, long* x, long* y, long* Width, long* Height)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4;
  InvokeHelper(0xdf, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Col, Row, x, y, Width, Height);
  return result;
}

void _DSpread::GetClientArea(long* Width, long* Height)
{
  static BYTE BASED_CODE parms[] =
    VTS_PI4 VTS_PI4;
  InvokeHelper(0xe0, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Width, Height);
}

long _DSpread::GetColItemData(long Col)
{
  long result;
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xe1, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    Col);
  return result;
}

BOOL _DSpread::GetDataFillData(VARIANT* Var, short VarType)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_PVARIANT VTS_I2;
  InvokeHelper(0xe2, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Var, VarType);
  return result;
}

void _DSpread::GetFirstValidCell(long* Col, long* Row)
{
  static BYTE BASED_CODE parms[] =
    VTS_PI4 VTS_PI4;
  InvokeHelper(0xe3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row);
}

long _DSpread::GetItemData()
{
  long result;
  InvokeHelper(0xe4, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
  return result;
}

void _DSpread::GetLastValidCell(long* Col, long* Row)
{
  static BYTE BASED_CODE parms[] =
    VTS_PI4 VTS_PI4;
  InvokeHelper(0xe5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row);
}

long _DSpread::GetMultiSelItem(long SelPrev)
{
  long result;
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xe6, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    SelPrev);
  return result;
}

long _DSpread::GetRowItemData(long Row)
{
  long result;
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xe7, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
    Row);
  return result;
}

BOOL _DSpread::GetText(long Col, long Row, VARIANT* Var)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_PVARIANT;
  InvokeHelper(0xe8, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Col, Row, Var);
  return result;
}

BOOL _DSpread::IsCellSelected(long Col, long Row)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0xe9, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Col, Row);
  return result;
}

BOOL _DSpread::IsFormulaValid(LPCTSTR Formula)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_BSTR;
  InvokeHelper(0xea, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Formula);
  return result;
}

BOOL _DSpread::IsVisible(long Col, long Row, BOOL Partial)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_BOOL;
  InvokeHelper(0xeb, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Col, Row, Partial);
  return result;
}

BOOL _DSpread::LoadFromFile(LPCTSTR FileName)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_BSTR;
  InvokeHelper(0xec, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    FileName);
  return result;
}

BOOL _DSpread::LoadTabFile(LPCTSTR FileName)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_BSTR;
  InvokeHelper(0xed, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    FileName);
  return result;
}

void _DSpread::RowHeightToTwips(long Row, float Height, long* Twips)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_R4 VTS_PI4;
  InvokeHelper(0xee, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Row, Height, Twips);
}

BOOL _DSpread::SaveTabFile(LPCTSTR FileName)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_BSTR;
  InvokeHelper(0xef, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    FileName);
  return result;
}

BOOL _DSpread::SaveToFile(LPCTSTR FileName, BOOL DataOnly)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_BSTR VTS_BOOL;
  InvokeHelper(0xf0, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    FileName, DataOnly);
  return result;
}

void _DSpread::SetColItemData(long Col, long Value)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0xf1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Value);
}

BOOL _DSpread::SetDataFillData(VARIANT* Var)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_PVARIANT;
  InvokeHelper(0xf2, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Var);
  return result;
}

BOOL _DSpread::SetCellDirtyFlag(long Col, long Row, BOOL Dirty)
{
  BOOL result;
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_BOOL;
  InvokeHelper(0xf3, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
    Col, Row, Dirty);
  return result;
}

void _DSpread::SetItemData(long Value)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4;
  InvokeHelper(0xf4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Value);
}

void _DSpread::SetRowItemData(long Row, long Value)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0xf5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Row, Value);
}

void _DSpread::SetText(long Col, long Row, VARIANT* Var)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_PVARIANT;
  InvokeHelper(0xf6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, Var);
}

void _DSpread::TwipsToColColWidth(long Twips, float* ColWidth)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_PR4;
  InvokeHelper(0xf7, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Twips, ColWidth);
}

void _DSpread::TwipsToRowHeight(long Row, long Twips, float* RowHeight)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_PR4;
  InvokeHelper(0xf8, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Row, Twips, RowHeight);
}

void _DSpread::Refresh()
{
  InvokeHelper(0xfffffdda, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DSpread::AboutBox()
{
  InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// _DSpreadEvents properties

/////////////////////////////////////////////////////////////////////////////
// _DSpreadEvents operations

void _DSpreadEvents::Advance(BOOL AdvanceNext)
{
  static BYTE BASED_CODE parms[] =
    VTS_BOOL;
  InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     AdvanceNext);
}

void _DSpreadEvents::BlockSelected(long BlockCol, long BlockRow, long BlockCol2, long BlockRow2)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I4 VTS_I4;
  InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     BlockCol, BlockRow, BlockCol2, BlockRow2);
}

void _DSpreadEvents::ButtonClicked(long Col, long Row, long ButtonDown)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I4;
  InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, ButtonDown);
}

void _DSpreadEvents::Change(long Col, long Row)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row);
}

void _DSpreadEvents::Click(long Col, long Row)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row);
}

void _DSpreadEvents::ColWidthChange(long Col1, long Col2)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col1, Col2);
}

void _DSpreadEvents::CustomFunction(LPCTSTR FunctionName, short ParameterCnt, long Col, long Row, short* Status)
{
  static BYTE BASED_CODE parms[] =
    VTS_BSTR VTS_I2 VTS_I4 VTS_I4 VTS_PI2;
  InvokeHelper(0x7, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     FunctionName, ParameterCnt, Col, Row, Status);
}

void _DSpreadEvents::DataAddNew()
{
  InvokeHelper(0x8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DSpreadEvents::DataColConfig(long Col, LPCTSTR DataField, short DataType)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_BSTR VTS_I2;
  InvokeHelper(0x9, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, DataField, DataType);
}

void _DSpreadEvents::DataFill(long Col, long Row, short DataType, BOOL fGetData, short* Cancel)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I2 VTS_BOOL VTS_PI2;
  InvokeHelper(0xa, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, DataType, fGetData, Cancel);
}

void _DSpreadEvents::DblClick(long Col, long Row)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0xb, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row);
}

void _DSpreadEvents::DragDropBlock(long Col, long Row, long Col2, long Row2, long NewCol, long NewRow, long NewCol2, long NewRow2, BOOL Overwrite, short* Action, BOOL* DataOnly, BOOL* Cancel)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BOOL VTS_PI2 VTS_PBOOL VTS_PBOOL;
  InvokeHelper(0xc, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, Col2, Row2, NewCol, NewRow, NewCol2, NewRow2, Overwrite, Action, DataOnly, Cancel);
}

void _DSpreadEvents::DrawItem(long Col, long Row, short hDC, long Left, long Top, long Right, long Bottom, long Style)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4;
  InvokeHelper(0xd, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, hDC, Left, Top, Right, Bottom, Style);
}

void _DSpreadEvents::EditError(long Col, long Row, short EditError)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I2;
  InvokeHelper(0xe, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, EditError);
}

void _DSpreadEvents::EditMode(long Col, long Row, short Mode, BOOL ChangeMade)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I2 VTS_BOOL;
  InvokeHelper(0xf, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, Mode, ChangeMade);
}

void _DSpreadEvents::EnterRow(long Row, long RowIsLast)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0x10, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Row, RowIsLast);
}

void _DSpreadEvents::LeaveCell(long Col, long Row, long NewCol, long NewRow, BOOL* Cancel)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL;
  InvokeHelper(0x11, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row, NewCol, NewRow, Cancel);
}

void _DSpreadEvents::LeaveRow(long Row, BOOL RowWasLast, BOOL RowChanged, BOOL AllCellsHaveData, long NewRow, long NewRowIsLast, BOOL* Cancel)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_BOOL VTS_BOOL VTS_BOOL VTS_I4 VTS_I4 VTS_PBOOL;
  InvokeHelper(0x12, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Row, RowWasLast, RowChanged, AllCellsHaveData, NewRow, NewRowIsLast, Cancel);
}

void _DSpreadEvents::PrintAbort(BOOL* Abort)
{
  static BYTE BASED_CODE parms[] =
    VTS_PBOOL;
  InvokeHelper(0x13, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Abort);
}

void _DSpreadEvents::QueryAdvance(BOOL AdvanceNext, BOOL* Cancel)
{
  static BYTE BASED_CODE parms[] =
    VTS_BOOL VTS_PBOOL;
  InvokeHelper(0x14, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     AdvanceNext, Cancel);
}

void _DSpreadEvents::QueryData(long Row, long RowsNeeded, long* RowsLoaded, short Direction, BOOL* AtTop, BOOL* AtBottom)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_PI4 VTS_I2 VTS_PBOOL VTS_PBOOL;
  InvokeHelper(0x15, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Row, RowsNeeded, RowsLoaded, Direction, AtTop, AtBottom);
}

void _DSpreadEvents::RightClick(short ClickType, long Col, long Row, long MouseX, long MouseY)
{
  static BYTE BASED_CODE parms[] =
    VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_I4;
  InvokeHelper(0x16, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     ClickType, Col, Row, MouseX, MouseY);
}

void _DSpreadEvents::RowHeightChange(long Row1, long Row2)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0x17, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Row1, Row2);
}

void _DSpreadEvents::SelChange(long BlockCol, long BlockRow, long BlockCol2, long BlockRow2, long CurCol, long CurRow)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4;
  InvokeHelper(0x18, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     BlockCol, BlockRow, BlockCol2, BlockRow2, CurCol, CurRow);
}

void _DSpreadEvents::TopLeftChange(long OldLeft, long OldTop, long NewLeft, long NewTop)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4 VTS_I4 VTS_I4;
  InvokeHelper(0x19, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     OldLeft, OldTop, NewLeft, NewTop);
}

void _DSpreadEvents::UserFormulaEntered(long Col, long Row)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0x1a, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Col, Row);
}

void _DSpreadEvents::VirtualClearData(long Row, long RowsBeingCleared)
{
  static BYTE BASED_CODE parms[] =
    VTS_I4 VTS_I4;
  InvokeHelper(0x1b, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Row, RowsBeingCleared);
}

void _DSpreadEvents::KeyDown(short* KeyCode, short Shift)
{
  static BYTE BASED_CODE parms[] =
    VTS_PI2 VTS_I2;
  InvokeHelper(0xfffffda6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     KeyCode, Shift);
}

void _DSpreadEvents::KeyPress(short* KeyAscii)
{
  static BYTE BASED_CODE parms[] =
    VTS_PI2;
  InvokeHelper(0xfffffda5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     KeyAscii);
}

void _DSpreadEvents::KeyUp(short* KeyCode, short Shift)
{
  static BYTE BASED_CODE parms[] =
    VTS_PI2 VTS_I2;
  InvokeHelper(0xfffffda4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     KeyCode, Shift);
}

void _DSpreadEvents::MouseDown(short Button, short Shift, long x, long y)
{
  static BYTE BASED_CODE parms[] =
    VTS_I2 VTS_I2 VTS_I4 VTS_I4;
  InvokeHelper(0xfffffda3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Button, Shift, x, y);
}

void _DSpreadEvents::MouseMove(short Button, short Shift, long x, long y)
{
  static BYTE BASED_CODE parms[] =
    VTS_I2 VTS_I2 VTS_I4 VTS_I4;
  InvokeHelper(0xfffffda2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Button, Shift, x, y);
}

void _DSpreadEvents::MouseUp(short Button, short Shift, long x, long y)
{
  static BYTE BASED_CODE parms[] =
    VTS_I2 VTS_I2 VTS_I4 VTS_I4;
  InvokeHelper(0xfffffda1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Button, Shift, x, y);
}

#ifdef SS_V30
// jb - 03 sep 98 Exporting spread to HTML
//
BOOL _DSpread::ExportToHTML (LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile)
{
	BOOL result;
	static BYTE BASED_CODE parms[] = VTS_BSTR VTS_BOOL VTS_BSTR;

	InvokeHelper (0x130, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms, lpszFileName, bAppendFlag, lpszLogFile);

	return result;
}

BOOL _DSpread::ExportRangeToHTML (long col, long row, long col2, long row2, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile)
{
	BOOL result;
	static BYTE BASED_CODE parms[] = VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_BOOL VTS_BSTR;

	InvokeHelper (0x131, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms, col, row, col2, row2, lpszFileName, bAppendFlag, lpszLogFile);

	return result;
}

#ifdef FP_OLEDRAG
void _DSpread::OLEDrag()
{
	InvokeHelper (903, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DSpreadEvents::OLECompleteDrag(long* Effect)
{
	static BYTE BASED_CODE parms[] =
		VTS_PI4;
  InvokeHelper(904, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     Effect);
}

void _DSpreadEvents::OLEDragDrop(fpDataObject* DataObject, long* Effect, short* Button, short* Shift, float* X, float* Y)
{
	static BYTE BASED_CODE parms[] =
		VTS_DISPATCH VTS_PI4 VTS_PI2 VTS_PI2 VTS_PR4;
  InvokeHelper(905, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
     DataObject, Effect, Button, Shift, X, Y);
}

void _DSpreadEvents::OLEDragOver(fpDataObject* DataObject, long* Effect, short* Button, short* Shift, float* X, float* Y, short* State)
{
	static BYTE BASED_CODE parms[] =
		VTS_DISPATCH VTS_PI4 VTS_PI2 VTS_PI2 VTS_PR4 VTS_PR4 VTS_PI2;
	InvokeHelper(906, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		DataObject, Effect, Button, Shift, X, Y, State);
}

void _DSpreadEvents::OLEGiveFeedback(long* Effect, BOOL* DefaultCursors)
{
	static BYTE BASED_CODE parms[] =
		VTS_PI4 VTS_PBOOL;
	InvokeHelper(907, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		Effect, DefaultCursors);
}

void _DSpreadEvents::OLESetData(fpDataObject* DataObject, short* DataFormat)
{
	static BYTE BASED_CODE parms[] =
		VTS_DISPATCH VTS_PI2;
	InvokeHelper(908, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		DataObject, DataFormat);
}

void _DSpreadEvents::OLEStartDrag(fpDataObject* DataObject, long* AllowedEffects)
{
	static BYTE BASED_CODE parms[] =
		VTS_DISPATCH VTS_PI4;
	InvokeHelper(909, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		DataObject, AllowedEffects);
}
#endif // FP_OLEDRAG
#endif

#endif // dual interface attempt
*/