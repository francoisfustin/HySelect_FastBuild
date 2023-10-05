#include "stdafx.h"
#include <Windows.h>
#include <strsafe.h>
#include <VersionHelpers.h>
#include "accctrl.h"
#include "aclapi.h"
#include "WinSysUtil.h"

using namespace WinSys;
namespace WinSys
{
	/////////////////////////////////////////////////////////////////////////////
	//Display a dialog box with error message from GetLastError
	/////////////////////////////////////////////////////////////////////////////
	void DisplayGetLastError( LPTSTR lpszFunction )
	{
		// Retrieve the system error message for the last-error code

		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			(LPTSTR)&lpMsgBuf,
			0, NULL );

		// Display the error message and exit the process

		lpDisplayBuf = (LPVOID)LocalAlloc( LMEM_ZEROINIT,
			( lstrlen( (LPCTSTR)lpMsgBuf ) + lstrlen( (LPCTSTR)lpszFunction ) + 40 ) * sizeof( TCHAR ) );
		StringCchPrintf( (LPTSTR)lpDisplayBuf,
			LocalSize( lpDisplayBuf ) / sizeof( TCHAR ),
			TEXT( "%s failed with error %d: %s" ),
			lpszFunction, dw, lpMsgBuf );
		MessageBox( NULL, (LPCTSTR)lpDisplayBuf, TEXT( "Error" ), MB_OK );

		LocalFree( lpMsgBuf );
		LocalFree( lpDisplayBuf );
		//ExitProcess(dw); 
	}

	//void DisplayGetLastError(LPCTSTR FuncName)
	//{
	/*	LPVOID lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,
		0, NULL );
		std::wstring str;
		str.FormatMessage(_T(" failed with error %d: %s"),dw, lpMsgBuf);
		str = FuncName + str;
		MessageBox(NULL, str.c_str(), _T("Error"), MB_OK);
		LocalFree(lpMsgBuf);
		*/
	//}
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
	BOOL IsUserAdmin( VOID )
	{
		// At least Windows2000;
		if( false == IsWindowsVersionOrGreater( 5, 0, 0 ) )
		{
			return true;			   // below that return true administrator mode!	
		}

		BOOL b;
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup;
		b = AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup );
		if( b )
		{
			if( !CheckTokenMembership( NULL, AdministratorsGroup, &b ) )
			{
				b = FALSE;
			}
			FreeSid( AdministratorsGroup );
		}
		return( b );
	}
	// Test if administrator rights are needed for accessing file stored under the installation folder.
	bool ElevationNeeded( wchar_t* fn )
	{
		// At least Windows2000;
		if( false == IsWindowsVersionOrGreater( 5, 0, 0 ) )
		{
			return true;			  // below that return true user mode!	
		}

		bool bRetval = true;
		HANDLE hToken = NULL;
		PSID pSIDAdmin = NULL;
		PSID pSIDEveryone = NULL;
		PACL pACL = NULL;
		SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
		SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
		const int NUM_ACES = 2;
		EXPLICIT_ACCESS ea[NUM_ACES];
		DWORD dwRes;

		// Specify the DACL to use.
		// Create a SID for the Everyone group.
		if( !AllocateAndInitializeSid( &SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSIDEveryone ) )
		{
			//AfxMessageBox(_T("AllocateAndInitializeSid (Everyone) error"));
			//printf("AllocateAndInitializeSid (Everyone) error %u\n",GetLastError());
			goto Cleanup;
		}

		// Create a SID for the BUILTIN\Administrators group.
		if( !AllocateAndInitializeSid( &SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pSIDAdmin ) )
		{
			//AfxMessageBox(_T("AllocateAndInitializeSid (Admin) error"));
			//printf("AllocateAndInitializeSid (Admin) error %u\n",GetLastError());
			goto Cleanup;
		}

		ZeroMemory( &ea, NUM_ACES * sizeof( EXPLICIT_ACCESS ) );

		// Set read access for Everyone.
		ea[0].grfAccessPermissions = GENERIC_READ;
		ea[0].grfAccessMode = SET_ACCESS;
		ea[0].grfInheritance = NO_INHERITANCE;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[0].Trustee.ptstrName = (LPTSTR)pSIDEveryone;

		// Set full control for Administrators.
		ea[1].grfAccessPermissions = GENERIC_ALL;
		ea[1].grfAccessMode = SET_ACCESS;
		ea[1].grfInheritance = NO_INHERITANCE;
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
		ea[1].Trustee.ptstrName = (LPTSTR)pSIDAdmin;

		if( ERROR_SUCCESS != SetEntriesInAcl( NUM_ACES, ea, NULL, &pACL ) )
		{
			//		printf("Failed SetEntriesInAcl\n");
			goto Cleanup;
		}

		// Try to modify the object's DACL.
		// (LPTSTR)(LPCTSTR)fn,		 // name of the object
		//	SE_FILE_OBJECT,              // type of object
		//	DACL_SECURITY_INFORMATION,   // change only the object's DACL
		//	NULL, NULL,                  // do not change owner or group
		//	pACL,                        // DACL specified
		//	NULL);                       // do not change SACL
		dwRes = SetNamedSecurityInfo( (LPTSTR)(LPCTSTR)fn, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL );

		if( ERROR_SUCCESS == dwRes )
		{
			bRetval = false;
			// No more processing needed.
			goto Cleanup;
		}
		else //if (dwRes != ERROR_ACCESS_DENIED)
		{
			goto Cleanup;
		}

		Cleanup:
		if( pSIDAdmin )
			FreeSid( pSIDAdmin );
		if( pSIDEveryone )
			FreeSid( pSIDEveryone );
		if( pACL )
			LocalFree( pACL );
		if( hToken )
			CloseHandle( hToken );
		return bRetval;
	}

	bool CopyOrReplaceFile( LPCTSTR source, LPCTSTR destination )
	{
		// Copy a file (source) to an other place (destination)
		// if the destination file exists then remove it.

		ASSERT( source&&destination );

		// Implicit conversion of LPCTSTR to CString.
		CString src, dest;
		src = source;
		dest = destination;

		// if the destination file exists then remove it.
		try
		{
			CFileStatus fs;
			if( CFile::GetStatus( destination, fs ) )
			{				// remove the file if exists.
				fs.m_attribute = CFile::normal;
				CFile::SetStatus( destination, fs );
				CFile::Remove( dest );
			}
		}
		catch( CFileException *pclFileExecption )
		{
			// If we set a 'try-catch' mechanism we have to delete the class passed as argument otherwise
			// we will get memory leaks.
			pclFileExecption->Delete();

			DebugBreak();
		}

		// Copy the file (source) with the destination name.
		try
		{
			//copy file
			CopyFile( source, destination, FALSE );
			CFileStatus fs;
			if( CFile::GetStatus( destination, fs ) )
			{				// change file status to writable.
				fs.m_attribute &= !CFile::readOnly;
				CFile::SetStatus( destination, fs );
			}
			else
				return false;
		}
		catch( CFileException *pclFileExecption )
		{
			// If we set a 'try-catch' mechanism we have to delete the class passed as argument otherwise
			// we will get memory leaks.
			pclFileExecption->Delete();

			DebugBreak();
			return false;
		}

		return true;
	}

	void MoveFile( LPCTSTR source, LPCTSTR destination )
	{
		// Move a file (source) to an other place (destination)
		// if the destination file exists then remove it.

		ASSERT( source&&destination );

		// Implicit conversion of LPCTSTR to CString.
		CString src, dest;
		src = source;
		dest = destination;

		// if the destination file exists then remove it.
		try
		{
			CFileStatus fs;
			if( CFile::GetStatus( dest, fs ) )
			{				// remove file if exists.
				fs.m_attribute &= !0x01;
				CFile::SetStatus( dest, fs );
				CFile::Remove( dest );
			}
		}
		catch( CFileException *pclFileExecption )
		{
			// If we set a 'try-catch' mechanism we have to delete the class passed as argument otherwise
			// we will get memory leaks.
			pclFileExecption->Delete();

			DebugBreak();
		}


		// Rename the file with the destination name.
		try
		{			// move file.
			CFile::Rename( src, dest );
			CFileStatus fs;
			if( CFile::GetStatus( dest, fs ) )
			{				// Change file status to writable.
				fs.m_attribute &= !0x01;
				CFile::SetStatus( dest, fs );
			}
		}
		catch( CFileException *pclFileExecption )
		{
			// If we set a 'try-catch' mechanism we have to delete the class passed as argument otherwise
			// we will get memory leaks.
			pclFileExecption->Delete();

			DebugBreak();
		}
	}

	// Delete a file
	void DeleteFile( LPCTSTR filename )
	{
		CFileStatus fs;
		try
		{
			CFile::GetStatus( filename, fs );
			fs.m_attribute = 0;
			CFile::SetStatus( filename, fs );
			CFile::Remove( filename );
		}
		catch( CFileException *pclFileExecption )
		{
			// If we set a 'try-catch' mechanism we have to delete the class passed as argument otherwise
			// we will get memory leaks.
			pclFileExecption->Delete();
		}
	}

	// Delete all files in directory
	void CleanupDirectory( LPCTSTR dirpath )
	{
		CFileFind finder;
		CString DirPath = dirpath;
		if( DirPath.ReverseFind( '\\' ) != DirPath.GetLength() - 1 )
			DirPath += CString( _T("\\") );

		BOOL bWorking = finder.FindFile( DirPath + _T("*.*") );
		while( bWorking )
		{
			bWorking = finder.FindNextFile();
			try
			{
				CFileStatus fs;
				if( finder.IsDots() ) continue;
				CString str = DirPath + finder.GetFileName();
				CFile::GetStatus( (LPCTSTR)str, fs );
				fs.m_attribute = 0;

				CFile::SetStatus( (LPCTSTR)str, fs );
				CFile::Remove( (LPCTSTR)str );
			}
			catch( CFileException *pclFileExecption )
			{
				// If we set a 'try-catch' mechanism we have to delete the class passed as argument otherwise
				// we will get memory leaks.
				pclFileExecption->Delete();
			}
		}
		finder.Close();
	}

	// Create directory including intermediate directory
	bool CreateFullDirectoryPath( LPCTSTR DirPath )
	{
		CString strDirPath = DirPath;
		CString path;

		bool fFolderCreated = true;
		// Verify the first char of the Target is not "\\"
		// Otherwise the find function for the string will not
		// work properly
		if( strDirPath[0] == '\\' ) strDirPath = strDirPath.Mid( 1 );

		// Verify the Target is placed in the current HySelect folder
		int pos = strDirPath.Find( '\\' );
		CString NetworkFolder;
		if( strDirPath.Find( '\\' ) != -1 )
		{
			// Variables
			int pos;
			// Test if we are working on a Network folder or on a RAID disk
			// we get a case where the Doc file was located on D:\\Documents\... on a RAID disk
			pos = strDirPath.Find( _T("\\\\") );
			if( pos >= 0 )
			{
				if( pos == 0 )
				{
					strDirPath.Delete( 0, 2 );
					NetworkFolder = _T("\\\\");
				}
				else
				{
					NetworkFolder = strDirPath.Left( pos ) + _T("\\\\");
					strDirPath.Delete( 0, pos + 2 );
				}
			}
			else
				NetworkFolder = _T( "" );
			// Create the folders if they don't exist
			while( ( pos = strDirPath.Find( '\\' ) ) > 0 && fFolderCreated )
			{
				path += strDirPath.Left( pos ) + '\\';
				CString PartialPath = NetworkFolder.IsEmpty() ? _T("\\\\?\\") : NetworkFolder;
				PartialPath += path;
				if( !::CreateDirectory( PartialPath, NULL ) )
				{
					DWORD dw = GetLastError();
					fFolderCreated = ( dw != ERROR_PATH_NOT_FOUND );	// React only when path is not found
				}
				else
					fFolderCreated = TRUE;
				strDirPath = strDirPath.Mid( pos + 1 );
			}
			// Complete the path with the file 
			// name for the MoveFile function
			CString fullPath = NetworkFolder.IsEmpty() ? _T("\\\\?\\") : NetworkFolder;
			fullPath += path;
			path = fullPath + strDirPath;
		}
		return true;
	}

}
