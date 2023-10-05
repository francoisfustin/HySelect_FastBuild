/////////////////////////////////////////////////////////////////////////////
//	Class for the management of a HostFileTable (table structure
//  of download files)
/////////////////////////////////////////////////////////////////////////////
//
// File Table
// each line must be formatted as follows :
//     source;target;version;size;minAppversion;date;reqAppVersion //comments
// The field separator is ';'.
// Characters after // are not considered, used for comments.
// Space and tab characters are ignored.
//
// source : is the filename on the server with eventually a relative path
// target : is the filename on the local machine with the relative (to the installation directory) path
// version : version number must be greater than 0
// size : file size must be greater than 0
// minAppVersion : files can be used only when the TA application has a version >= to this value
// date : file date only used for user information
// reqAppVersion : files must be downloaded only when the TA application has a version < than this value
//
/////////////////////////////////////////////////////////////////////////////
#pragma once 

#define DEFREQAPPVERSION _T("99.99.99.99")

// Define the number of digits to be displayed for each file.
#define FILETABLE_VERSIONDIGITNBR_HH_APP		4			// TA-Scope firmware.
#define FILETABLE_VERSIONDIGITNBR_HH_WIRELESS	2			// Wireless Atmel & Dresden firmware.
#define FILETABLE_VERSIONDIGITNBR_HH_DB			3			// TA-Scope database.
#define FILETABLE_VERSIONDIGITNBR_DPS_APP		2			// DpS and DpS-Visio firmware.

class CFileTable
{
public:
	CFileTable();

public:
	POSITION GetActivePosition() { return m_ActivePos; }
	bool SetActivePosition( POSITION pos );

	// Delete all items in the FileTable.
	void DeleteAllItems();

	// Read the text file named FileName and complete the file table.
	// Returns true if everything is ok.
	bool ReadFile( TCHAR *ptcFileName );

	// Retrieve the number of elements.
	int GetCount()
	{
		return m_List.GetCount();
	}
	
	// Sweep the list and activate the current element.
	// Returns true if one gets to the end.
	bool GetFirst();

	bool GetNext();

	// Retrieve the info of the active element.
	LPCTSTR GetSource()
	{
		return m_Active.m_tcSource;
	}

	LPCTSTR GetTarget()
	{
		return m_Active.m_tcTarget;
	}

	double GetVersion()
	{
		return m_Active.m_dVersion;
	}

	unsigned long GetSize()
	{
		return m_Active.m_ulSize;
	}

	double GetminAppVersion()
	{
		return m_Active.m_dMinAppVersion;
	}

	double GetreqAppVersion()
	{
		return m_Active.m_dReqAppVersion;
	}
	
	CTime GetDate()
	{
		return m_Active.m_Date;
	}

	CString GetDateAsString();

	unsigned int GetCRC32()
	{
		return m_Active.m_uiCRC;
	}
	
	bool FindSource( TCHAR *ptcTarget );
	bool FindTarget( TCHAR *ptcTarget );
	bool FindTargetWoPath( TCHAR *ptcTarget );
	
	// Add an entry to the table.
	void Add( TCHAR *ptcSource, TCHAR *ptcTarget, double dVersion, unsigned long ulSize, double dMinAppVersion, double dReqAppVersion, 
			CTime date, unsigned int uiCRC );
	
	// Transform a x field version string (X.Y.Z.n) into a double.
	// !!! The first and second fields (X and Y in the above line example).
	// !!! can be made of only one digit. The third field (Z) is limited to 99.
	double VersionStringToDouble( CString str );
	
	// Transform a double into a 3 field version string (X.Y.Z).
	// !!! The first and second fields (X and Y in the above line example).
	// !!! can be made of only one digit. The third field (Z) is limited to 99.
	CString VersionDoubleToString( double dVal, int iNbrField );

	bool CompareVersionFileTable( CFileTable *pOldFileTable, CFileTable *pNewFileTable, double dCurrentAppVersion );

	void SetreqAppVersion( double dVal )
	{
		m_Active.m_dReqAppVersion = dVal;
		m_List.SetAt( m_ActivePos, m_Active );
	}

	void SetVersion( double dVal )
	{
		m_Active.m_dVersion = dVal;
		m_List.SetAt( m_ActivePos, m_Active );
	}

	void SetSize( unsigned long size )
	{
		m_Active.m_ulSize = size;
		m_List.SetAt( m_ActivePos, m_Active );
	}

	void SetDate( CTime dt )
	{
		m_Active.m_Date = dt;
		m_List.SetAt( m_ActivePos, m_Active );
	}

	void SetCRC( unsigned int crc )
	{
		m_Active.m_uiCRC = crc;
		m_List.SetAt( m_ActivePos, m_Active );
	}

	void SetSource( CString src )
	{
		_tcscpy_s( m_Active.m_tcSource, ( LPCTSTR )src );
		m_List.SetAt( m_ActivePos, m_Active );
	}

	void SetTarget( CString src )
	{
		_tcscpy_s( m_Active.m_tcTarget, ( LPCTSTR )src );
		m_List.SetAt( m_ActivePos, m_Active );
	}

	// Empty the table.
	void _MakeEmpty()
	{
		m_List.RemoveAll();
		m_pos = 0;
		memset(&m_Active, 0, sizeof(m_Active));
	}

	void OrderList();

private:
	struct SRecord
	{
		struct SRecord()
		{
			m_tcSource[0] = _T('\0');
			m_tcTarget[0] = _T('\0');
			m_dVersion = 0.0;
			m_ulSize = 0;
			m_dMinAppVersion = 0.0;
			m_dReqAppVersion = 0.0;
			m_Date = CTime( 0 );
			m_uiCRC = 0;
		}

		TCHAR m_tcSource[512];
		TCHAR m_tcTarget[512];
		double m_dVersion;
		unsigned long m_ulSize;
		double m_dMinAppVersion;
		double m_dReqAppVersion;					// First application version requiring this entry of the FileTable
		CTime m_Date;
		unsigned int m_uiCRC;
	};
	
	CList<SRecord, SRecord &> m_List;
	POSITION m_pos, m_ActivePos;
	SRecord m_Active;

	// Retrieve the "record" of which the target file corresponds.
	// to this of the argument. Return true if the element is found.
	bool _FindTarget( TCHAR *ptcTarget );

	// Read a line of a text file, deleting each space or \t
	// Characters following // are ignored.
	bool _ReadTextLine( CFile &f, CString &str );
};
