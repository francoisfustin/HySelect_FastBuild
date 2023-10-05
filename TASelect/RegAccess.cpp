#include "stdafx.h"
#include <Windows.h>
//#include <stdio.h>
//#include <strsafe.h>
#include "TaSelect.h"
#include "RegAccess.h"


CRegAccess::CRegAccess(void)
{
}


CRegAccess::~CRegAccess(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// My access to the registry 
void CRegAccess::RegistryAccessError(int err)
{

	// To Display a error message
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
}
// Write a string 
bool CRegAccess::SetRegString(CString path,CString field,CString Val,bool bVerbose/*=true*/)
{
	HKEY subKey;

	int err;
	if(err=::RegOpenKeyEx(HKEY_CURRENT_USER,(LPCTSTR)path,0,KEY_WRITE,&subKey)) 
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	if(err=::RegSetValueEx(subKey,field,0,REG_SZ,(const unsigned char *)(LPCTSTR)Val,(Val.GetLength()+1)*sizeof(TCHAR)))
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	if(err=::RegCloseKey(subKey))
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	return true;
}

// Write an Integer
bool CRegAccess::SetRegInt(CString path,CString field,int Val,bool bVerbose/*=true*/)
{
	HKEY subKey;
	int err;
	if(err=::RegOpenKeyEx(HKEY_CURRENT_USER,(LPCTSTR)path,0,KEY_WRITE,&subKey)) 
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	if(err=::RegSetValueEx(subKey,field,0,REG_DWORD,(const unsigned char *)&Val,sizeof(REG_DWORD)))
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	if(err=::RegCloseKey(subKey))
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	return true;
}
bool CRegAccess::SetRegBinData(CString path,CString field,unsigned char *pbuf, UINT len,bool bVerbose/*=true*/)
{
	HKEY subKey;
	int err;
	if(err=::RegOpenKeyEx(HKEY_CURRENT_USER,(LPCTSTR)path,0,KEY_WRITE,&subKey)) 
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	if(err=::RegSetValueEx(subKey,field,0,REG_BINARY,(const unsigned char *)pbuf,len))
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	if(err=::RegCloseKey(subKey))
	{
		if (bVerbose) RegistryAccessError(err);
		return false;
	}
	return true;

}
// Read a string 
CString CRegAccess::GetRegString(CString path,CString field,CString defVal,bool bVerbose/*=true*/)
{
	HKEY subKey;
	TCHAR data[256];
	DWORD	Size=sizeof(data);

	memset(data,0,Size/sizeof(TCHAR));
	int err;
	if(err=::RegOpenKeyEx(HKEY_CURRENT_USER,(LPCTSTR)path,0,KEY_READ,&subKey)) 
	{
		if (bVerbose) RegistryAccessError(err);
		return defVal;
	}
	if(err=::RegQueryValueEx(subKey,field,0,NULL,(unsigned char *)data,&Size))
	{
		if (bVerbose) RegistryAccessError(err);
		return defVal;
	}
	if(err=::RegCloseKey(subKey))
	{
		if (bVerbose) RegistryAccessError(err);
		return defVal;
	}
	return data;
}

// Read an Integer
int	CRegAccess::GetRegInt(CString path,CString field,int defVal,bool bVerbose/*=true*/)
{
	HKEY subKey;
	TCHAR	data[256];
	DWORD	Size=sizeof(data);

	memset(data,0,Size);
	int err;
	if(err=::RegOpenKeyEx(HKEY_CURRENT_USER,(LPCTSTR)path,0,KEY_READ,&subKey)) 
	{
		if (bVerbose) RegistryAccessError(err);
		return (defVal);
	}
	if(err=::RegQueryValueEx(subKey,field,0,NULL,(unsigned char *)data,&Size))
	{
		if (bVerbose) RegistryAccessError(err);
		return (defVal);
	}
	if(err=::RegCloseKey(subKey))
	{
		if (bVerbose) RegistryAccessError(err);
		return (defVal);
	}
	return (*(int*)data);
}

// Read a binary buffer
bool CRegAccess::GetRegBinData(CString path,CString field,unsigned char *pBuf, UINT *plen,bool bVerbose/*=true*/)
{
	HKEY subKey;
	TCHAR	data[256];
	DWORD	Size=sizeof(data);

	int err;
	if(err=::RegOpenKeyEx(HKEY_CURRENT_USER,(LPCTSTR)path,0,KEY_READ,&subKey)) 
	{
		if (bVerbose) RegistryAccessError(err);
		return (false);
	}
	DWORD type = REG_BINARY;
	if(err=::RegQueryValueEx(subKey,field,0,&type,(unsigned char *)data,&Size))
	{
		if (bVerbose) RegistryAccessError(err);
		return (false);
	}
	else 
	{
		memcpy(pBuf,data,Size);
		*plen=Size;
	}
	if(err=::RegCloseKey(subKey))
	{
		if (bVerbose) RegistryAccessError(err);
		return (false);
	}
	return (true);
}

LONG CRegAccess::RegCopyKey(HKEY SrcKey, HKEY TrgKey, LPWSTR TrgSubKeyName)
{

	HKEY    SrcSubKey;
	HKEY    TrgSubKey;
	int    ValEnumIndx=0;
	int    KeyEnumIndx=0;
	WCHAR    ValName[MAX_PATH+1];
	WCHAR    KeyName[MAX_PATH+1];
	DWORD    size;    
	DWORD    VarType;
	DWORD    BuffSize;
	unsigned char*    Buff=NULL;
	LONG    Err;
	DWORD    KeyDisposition;
	FILETIME LastWriteTime; 

	if (NULL == SrcKey || NULL == TrgKey) 
		return -1;
	// create target key
	if (::RegCreateKeyEx(TrgKey,TrgSubKeyName,NULL,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&TrgSubKey,&KeyDisposition) != ERROR_SUCCESS)
		return GetLastError();

	do {
		do {
			// read value from source key
			Err = ERROR_NOT_ENOUGH_MEMORY;
			Err = RegQueryInfoKey(SrcKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &BuffSize, NULL, NULL);                   
			BuffSize = __max(BuffSize, 1024); // just to be safe min size is 1024
			do {                         
				AllocBuff(&Buff,BuffSize);
				size=MAX_PATH+1;
				Err = RegEnumValue(SrcKey,ValEnumIndx,ValName,&size,NULL,&VarType,Buff,&BuffSize);
				if ((Err != ERROR_SUCCESS) && (Err != ERROR_NO_MORE_ITEMS))
					Err = GetLastError();
			} while (Err == ERROR_NOT_ENOUGH_MEMORY);

			// done copying this key
			if (Err == ERROR_NO_MORE_ITEMS)
				break;

			// unknown error return
			if (Err != ERROR_SUCCESS)
				goto quit_err;

			// write value to target key
			if (RegSetValueEx(TrgSubKey,ValName,NULL,VarType,Buff,BuffSize) != ERROR_SUCCESS)
				goto quit_get_err;

			// read next value
			ValEnumIndx++;
		} while (true);

		// free buffer
		FreeBuff(&Buff);

		// if copying under the same 
		// key avoid endless recursions
		do {
			// enum sub keys
			size=MAX_PATH+1;
			Err = RegEnumKeyEx(SrcKey,KeyEnumIndx++,KeyName,&size,NULL,NULL,NULL,&LastWriteTime);
		} while ((SrcKey == TrgKey) && 	!_wcsnicmp(KeyName,TrgSubKeyName,wcslen(KeyName)) && (Err == ERROR_SUCCESS));

		// done copying this key        
		if (Err == ERROR_NO_MORE_ITEMS)
			break;

		// unknown error return
		if (Err != ERROR_SUCCESS)
			goto quit_get_err;

		// open the source subkey
		if (RegOpenKeyEx(SrcKey,KeyName,NULL,KEY_ALL_ACCESS,&SrcSubKey) != ERROR_SUCCESS)
			goto quit_get_err;

		// recurs with the subkey
		if ((Err = RegCopyKey(SrcSubKey, TrgSubKey,KeyName)) != ERROR_SUCCESS)
			break;

		if (RegCloseKey(SrcSubKey) != ERROR_SUCCESS)
			goto quit_get_err;
	} while (true);

	// normal quit

quit_err:
	FreeBuff(&Buff);
	RegCloseKey(TrgSubKey);
	if (Err == ERROR_NO_MORE_ITEMS)
		return ERROR_SUCCESS;    
	else
		return Err;

	// abnormal quit
quit_get_err:
	FreeBuff(&Buff);
	RegCloseKey(TrgSubKey);
	return GetLastError();
}

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL CRegAccess::RegDelnodeRecurse (HKEY hKeyRoot, CString SubKey)
{
    CString StrEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.
    lResult = RegDeleteKey(hKeyRoot, (LPCTSTR)SubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    lResult = RegOpenKeyEx (hKeyRoot,(LPCTSTR)SubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        if (lResult == ERROR_FILE_NOT_FOUND) 
		{
            //printf("Key not found.\n");
            return TRUE;
        } 
        else 
		{
            //printf("Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.
	if (SubKey.GetAt(SubKey.GetLength()-1) != '\\')
		SubKey += _T("\\");
  
    // Enumerate the keys
    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) 
    {
        do 
		{
            StrEnd = SubKey + szName;
            if (!RegDelnodeRecurse(hKeyRoot, StrEnd)) 
			{
                break;
            }
            dwSize = MAX_PATH;
            lResult = RegEnumKeyEx(hKey, 0,  szName, &dwSize, NULL, NULL, NULL, &ftWrite);
        } while (lResult == ERROR_SUCCESS);
    }


    RegCloseKey (hKey);

    // Try again to delete the key.
	SubKey.Delete(SubKey.GetLength()-1);		// Remove '\'
    lResult = RegDeleteKey(hKeyRoot, SubKey);
    if (lResult == ERROR_SUCCESS) 
        return TRUE;
    return FALSE;
}

void CRegAccess::VerifyAndRepairQATRegistryEntry()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Following code is needed to recover Quick access toolbar (QAT).
	// Resource ID are stored into the registry (Workspace) if these ID doesn't exist anymore (refactoring!)
	// the QAT is not correctly displayed
	BYTE  cData[100];
	DWORD size = 100;
	CString tas =  _T("Software\\")+GetTASRegistryKey();
	CString path = tas + CString(_HYSELECT_NAME_BCKSLASH) + _T("Workspace\\MFCRibbonBar-59398");
	HKEY key;
	DWORD dwRet = ::RegOpenKeyEx(HKEY_CURRENT_USER,(LPCTSTR)path,0,KEY_ALL_ACCESS,&key);
	if (ERROR_SUCCESS == dwRet)
	{
		dwRet = RegQueryValueEx(key,_T("QuickAccessToolbarCommands"),NULL,NULL,(LPBYTE)cData,(LPDWORD)&size);
		if (ERROR_SUCCESS == dwRet || ERROR_MORE_DATA == dwRet)
		{
			int iRegID = cData[2] + cData[3]*256; 
			if (iRegID != ID_APPMENU_NEW)		// Wrong ID or resource changed!
			{
				path = tas + CString(_T("\\")) + CString( _HYSELECT_NAME );
				HKEY key;
				DWORD dwRet = ::RegOpenKeyEx(HKEY_CURRENT_USER,(LPCTSTR)path,0,KEY_ALL_ACCESS,&key);
				RegDelnodeRecurse(key,_T("Workspace"));
			}
		}
	}
}

CString CRegAccess::GetTASRegistryKey()
{
#define TAHK	_T("TA Hydronics\\")
#define IMIHEK	_T("IMI Hydronic Engineering\\")
	CString OldTasKey = TAHK+TASApp.GetTADBKey();
	CString TasKey = IMIHEK+TASApp.GetTADBKey();
	CString SWKey = _T("Software\\");
	HKEY hKey;
	return TasKey;

	// Try to open IMI Hydronic Engineering key
	CString tmpKey = SWKey + TasKey;
	LONG lResult = RegOpenKeyEx (HKEY_CURRENT_USER, tmpKey, 0, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS)
		return TasKey;
	else if (lResult != ERROR_FILE_NOT_FOUND)
		return TasKey;

	// Not found, try to found OldTasKey
	tmpKey = SWKey + OldTasKey;
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, tmpKey, 0, KEY_READ, &hKey);
	if (lResult != ERROR_SUCCESS) 
	{
		// Not found neither force usage of new TasKey
		return TasKey;
	}
	// Old Key found rename it to new name
	// Renaming doesn't exist so copy and delete...
	HKEY SrcKey;
	HKEY TrgKey;
	
	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, tmpKey, 0, KEY_READ, &SrcKey);
	if (lResult == ERROR_SUCCESS) 
	{
		lResult = RegOpenKeyEx(HKEY_CURRENT_USER, SWKey, 0, KEY_READ, &TrgKey);
		if (lResult == ERROR_SUCCESS) 
		{
			if (RegCopyKey(SrcKey, TrgKey, (LPWSTR)(LPCTSTR)TasKey) == ERROR_SUCCESS)
			{
				// Delete Src key
				//tmpKey = SWKey + TAAK;
				//lResult = RegOpenKeyEx(HKEY_CURRENT_USER, tmpKey, 0, KEY_READ, &SrcKey);
				//if (lResult == ERROR_SUCCESS) 
				//{
				//	RegDelnodeRecurse(SrcKey,TASApp.GetTADBKey());
				//}
				// Copy Common key if exist
				tmpKey = SWKey + TAHK + _T("TA Select Common");
				lResult = RegOpenKeyEx(HKEY_CURRENT_USER, tmpKey, 0, KEY_READ, &SrcKey);
				if (lResult == ERROR_SUCCESS) 
				{
					tmpKey = SWKey + IMIHEK;
					lResult = RegOpenKeyEx(HKEY_CURRENT_USER, tmpKey, 0, KEY_READ, &TrgKey);
					if (lResult == ERROR_SUCCESS) 
					{
						RegCopyKey(SrcKey, TrgKey, (LPWSTR) _T("HySelect Common"));
					}
				}
			}
		}
	}
	return TasKey;
}
