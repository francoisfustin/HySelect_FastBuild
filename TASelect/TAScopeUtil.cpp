#include "stdafx.h"
#include <Windows.h>
#include "TASelect.h"
#include "global.h"
#include "TAScopeUtil.h"


bool CTAScopeUtil::CheckAndCreateTAScopeFolders()
{
	// Create directory if needed.
	CString TAScopeFolder = TASApp.GetDocumentsFolderForDoc();
	
	// Create _TASCOPE_DIRECTORY directory if needed.
	// By default 'TAScopeFolder' = "c:\User\xxx\Documents" (or in the sub folder of a USB kit).
	// Ex: strTSCPath = "c:\Users\FFustin\Documents\HySelect\tascope".
	CString strTSCPath = TAScopeFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _TASCOPE_DIRECTORY );
	int err = SHCreateDirectoryEx( NULL, (LPCTSTR)strTSCPath, NULL );
	
	if ( !( err == ERROR_SUCCESS || err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS ) )
	{
		return false;
	}

	// Create _TASCOPE_Localized directory if needed.
	// Ex: strTSCLocPath = "c:\Users\FFustin\Documents\HySelect\tascope\int".
	CString strTSCLocPath = strTSCPath + CString( _T("\\") ) + TASApp.GetTADBKey();
	err = SHCreateDirectoryEx( NULL, (LPCTSTR)strTSCLocPath, NULL );

	if( !( err == ERROR_SUCCESS || err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS ) )
	{
		return false;
	}

	// Create _TASCOPE_Maintenance_DIRECTORY directory if needed.
	// Ex: strTSCMaintPath = "c:\Users\FFustin\Documents\HySelect\tascope\int\Maintenance".
	CString strTSCMaintPath = strTSCLocPath + CString( _T("\\") ) + CString( _TASCOPE_MAINTENANCE_DIRECTORY );
	err = SHCreateDirectoryEx( NULL, (LPCTSTR)strTSCMaintPath, NULL );
	
	if( !( err == ERROR_SUCCESS || err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS ) )
	{
		return false;
	}

	// Create _TASCOPE_DATA_DIRECTORY directory if needed.
	// Ex: strTSCDataPath = "c:\Users\FFustin\Documents\HySelect\tascope\int\Data".
	CString strTSCDataPath = strTSCLocPath + CString( _T("\\") ) + CString( _TASCOPE_DATA_DIRECTORY );
	err = SHCreateDirectoryEx( NULL, (LPCTSTR)strTSCDataPath, NULL );
	
	if( !( err == ERROR_SUCCESS || err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS ) )
	{
		return false;
	}

	// Create _TASCOPE_BACKUP_DIRECTORY directory if needed.
	// Ex: strTSCBackcupPath = "c:\Users\FFustin\Documents\HySelect\tascope\int\Backup".
	CString strTSCBackcupPath = strTSCLocPath + CString( _T("\\") ) + CString( _TASCOPE_BACKUP_DIRECTORY );
	err = SHCreateDirectoryEx( NULL, (LPCTSTR)strTSCBackcupPath, NULL );
	
	if( !( err == ERROR_SUCCESS || err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS ) )
	{
		return false;
	}

	return true;
}

CString CTAScopeUtil::GetTAScopeFolder()
{
	// Verify folder existence.
	if( false == CheckAndCreateTAScopeFolders() )
	{
		return _T("");
	}

	CString TAScopeFolder = TASApp.GetDocumentsFolderForDoc();
	CString strTSCPath = TAScopeFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _TASCOPE_DIRECTORY );

	// Ex: strTSCPath = "c:\Users\FFustin\Documents\HySelect\tascope".
	return strTSCPath;
}

CString CTAScopeUtil::GetTAScopeDataFolder()
{
	// Verify folder existence.
	if( false == CheckAndCreateTAScopeFolders() )
	{
		return _T("");
	}

	CString TAScopeFolder = TASApp.GetDocumentsFolderForDoc();
	CString strTSCPath = TAScopeFolder + CString( _HYSELECT_NAME_BCKSLASH) + CString( _TASCOPE_DIRECTORY );
	CString strTSCLocPath = strTSCPath + CString( _T("\\") ) + TASApp.GetTADBKey();
	CString strTSCBackupPath = strTSCLocPath + CString( _T("\\") ) + CString( _TASCOPE_DATA_DIRECTORY );

	// Ex: strTSCBackupPath = "c:\Users\FFustin\Documents\HySelect\tascope\int\Data".
	return strTSCBackupPath;
}

CString CTAScopeUtil::GetTAScopeBackupFolder()
{
	// Verify folder existence.
	if( false == CheckAndCreateTAScopeFolders() )
	{
		return _T("");
	}

	CString TAScopeFolder = TASApp.GetDocumentsFolderForDoc();
	CString strTSCPath = TAScopeFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _TASCOPE_DIRECTORY );
	CString strTSCLocPath = strTSCPath + CString( _T("\\") ) + TASApp.GetTADBKey();
	CString strTSCDataPath = strTSCLocPath + CString( _T("\\") ) + CString( _TASCOPE_BACKUP_DIRECTORY );

	// Ex: strTSCDataPath = "c:\Users\FFustin\Documents\HySelect\tascope\int\Backup".
	return strTSCDataPath;
}


CString CTAScopeUtil::GetTAScopeMaintenanceFolder()
{
	// Verify folder existence.
	if( false == CheckAndCreateTAScopeFolders() )
	{
		return _T("");
	}

	CString TAScopeFolder = TASApp.GetDocumentsFolderForDoc();
	CString strTSCPath = TAScopeFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _TASCOPE_DIRECTORY );
	CString strTSCLocPath = strTSCPath + CString( _T("\\") ) + TASApp.GetTADBKey();
	CString strTSCMaintPath = strTSCLocPath + CString( _T("\\") ) + CString( _TASCOPE_MAINTENANCE_DIRECTORY );

	// Ex: strTSCMaintPath = "c:\Users\FFustin\Documents\HySelect\tascope\int\Maintenance".
	return strTSCMaintPath;
}

CString CTAScopeUtil::GetTAScopeLocHFTPath()
{
	// Verify folder existence.
	if( false == CheckAndCreateTAScopeFolders() )
	{
		return _T("");
	}

	CString TAScopeFolder = TASApp.GetDocumentsFolderForDoc();
	CString strTSCPath = TAScopeFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _TASCOPE_DIRECTORY );
	CString strTSCLocPath = strTSCPath + CString( _T("\\") ) + TASApp.GetTADBKey();
	CString strTSCHFTPath = strTSCLocPath + CString( _T("\\") ) + CString( _TASCOPE_HFT );

	// Ex: strTSCMaintPath = "c:\Users\FFustin\Documents\HySelect\tascope\int\hft-tascope.txt".
	return strTSCHFTPath;
}

CString CTAScopeUtil::GetTAScopeMaintHFTPath()
{
	CString strTSCSFTPath = GetTAScopeMaintenanceFolder() + CString(_T("\\")) + CString( _TASCOPE_HFT );

	// Ex: strTSCMaintPath = "c:\Users\FFustin\Documents\HySelect\tascope\int\Maintenance\hft-tascope.txt".
	return strTSCSFTPath;
}

CString CTAScopeUtil::CompleteTargetPath( CString Target )
{
	// Variable.
	CString TargetDir;
	CString DocumentFolder = TASApp.GetDocumentsFolderForDoc();

	// Make the CString lower.
	Target.MakeLower();

	int pos = Target.Find(L"$documents");
	
	if( pos != -1) // The file must exist in the Document folder
	{
		// Replace $documents by the right path.
		Target.Replace( L"$documents", DocumentFolder );
		TargetDir = Target;
	}

	return TargetDir;
}
