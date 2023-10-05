#pragma once
class CTAScopeUtil
{
public:
	static bool CheckAndCreateTAScopeFolders();
	static CString GetTAScopeFolder();
	static CString GetTAScopeDataFolder();
	static CString GetTAScopeBackupFolder();
	static CString GetTAScopeMaintenanceFolder();
	static CString GetTAScopeLocHFTPath();
	static CString GetTAScopeMaintHFTPath();
	static CString CompleteTargetPath(CString Target);
};