#pragma once

#include "..\ZipArchive\ZipArchive.h"
//#include "taselect.h"
#include "utilities.h"


class CTender
{
public:
	CTender();
	~CTender();

	// *** PUBLIC ***
	void SetZipPath( const TCHAR *zipPath );

	// returns -1 if no tender text are found
	int GetTenderTxt( const _string &artRef, _string &tenderTxt, bool checkIndex = true );

	void GetFileBufferFromIndex( unsigned int i, void **buffer );

	int GetTenderTxtFromIndex( const _string &artRef, _string &tenderTxt );
	int GetTenderTxtFromPath( const _string &path, _string &tenderTxt );
	// returns -1 if no tender text are found
	// tender ID can be overrided by the index file
	int GetTenderID( const _string &artRef );

	void RemoveUnusedTender();
	std::vector<_string> GetArtNumTenderVector();
	std::vector<_string> GetArtNumVector();
	_string GetTextFromRTF( const _string &rtf ) const;
	bool IsTenderNeeded();

	// For test unit to allow access to protected methods.
	bool PublicOpenZip() { return OpenZip(); }

private:
	// *** PRIVATE ***
	// Need to be called once to load article numbers
	bool OpenZip();

	friend class utests_Tender;
	// *** VARIABLES ***
	TCHAR *m_zipPath;
	CZipArchive m_zip;
	int m_iIndexNumIndex;
	std::vector<_string> m_vArtNumIndex;
};

