#pragma once
namespace WinSys
{
	#define ASSERT_RETURN {ASSERT(0); return;}
	#define ASSERTA_RETURN(arg) {ASSERT(0); return(arg);}
	#define ASSERT_CONTINUE {ASSERT(0); continue;}
	/////////////////////////////////////////////////////////////////////////////
	//Display a dialog box with error message from GetLastError
	/////////////////////////////////////////////////////////////////////////////
	void DisplayGetLastError(LPTSTR FuncName);
	/*++ 
	Routine Description: This routine returns TRUE if the caller's process 
	is a member of the Administrators local group. Caller is NOT expected 
	to be impersonating anyone and is expected to be able to open its own 
	process and process token. 
	Arguments: None. 
	Return Value: 
	   TRUE - Caller has Administrators local group. 
	   FALSE - Caller does not have Administrators local group. --
	*/ 
	BOOL IsUserAdmin(VOID);
	// Test if administrator right are needed for accessing file stored under the installation folder.
	bool ElevationNeeded(LPTSTR fn);

	bool CopyOrReplaceFile(LPCTSTR source, LPCTSTR destination);
	void MoveFile(LPCTSTR source, LPCTSTR destination);
	void DeleteFile(LPCTSTR filename);
	// Delete all files in directory
	void CleanupDirectory(LPCTSTR dirpath);
	bool CreateFullDirectoryPath( LPCTSTR DirPath );
}