#pragma once
class CRegAccess
{
public:
	CRegAccess(void);
	virtual ~CRegAccess(void);
	static void RegistryAccessError(int err);
	static bool SetRegString(CString path,CString field,CString Val,bool bVerbose=true);
	static bool SetRegInt(CString path,CString field,int Val,bool bVerbose=true);
	static bool SetRegBinData(CString path,CString field,unsigned char *pbuf, UINT len,bool bVerbose=true);
	static CString GetRegString(CString path,CString field,CString defVal,bool bVerbose=true);
	static int GetRegInt(CString path,CString field,int defVal,bool bVerbose=true);
	static bool GetRegBinData(CString path,CString field,unsigned char *pBuf, UINT *plen,bool bVerbose=true);
	static LONG RegCopyKey(HKEY SrcKey, HKEY TrgKey, LPWSTR TrgSubKeyName);
	static CString GetTASRegistryKey();
	static void VerifyAndRepairQATRegistryEntry();
protected:
	static __forceinline void FreeBuff(unsigned char** Buff)
	{
		if (*Buff) 
		{
			free(*Buff);
			*Buff = NULL;
		}
	};

	static __forceinline void AllocBuff(unsigned char** Buff,DWORD BuffSize)
	{
		FreeBuff(Buff);
		*Buff = (unsigned char*)malloc(BuffSize);
	};
	static BOOL RegDelnodeRecurse (HKEY hKeyRoot, CString lpSubKey);
};