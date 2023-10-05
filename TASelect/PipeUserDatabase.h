#pragma once
#include "DataBase.h"


class CPipeUserDatabase : public CDataBase
{
public:
	CPipeUserDatabase();
	virtual ~CPipeUserDatabase() {}

	// Copy the 'PIPE_TAB' from an another DB.
	// If 'bIgnoreFixed' is 'true' we copy only user pipes.
	// Fixed pipes are those that are in the "Pipedb.txt".
	void CopyFrom( CDataBase *pToDB, bool bIgnoreFixed = false );

	// HYS-1590: Allow to copy pipe series in the pipe user database.
	// It is essentially used by the 'CTADatastruct::Read' method.
	// param: 'vecPipeSeries' contains a list of ID of the pipe series to copy.
	// param: If 'pFromDB' is not defined, we will retrieve object to copy from the current database (In 'CPipeUserDatabase').
	//        If 'pFromDB' is defined, we will retrieve object to copy in this database instead of the current one.
	void CopyPipeSeriesFrom( std::vector<CString> &vecPipeSeries, CDataBase *pFromDB );

	// HYS-1590: Allow to copy some user pipe series from the vector to a table.
	// It is essentially used by the 'CTADatastruct::AddUsedUserPipesBeforeSavingProject' method.
	void CopyPipeSeriesTo( std::vector<CTable *> &vecPipeSeries, CTable *pclTableWhereToCopy );

	// Copy the 'PIPE_TAB' into an another DB.
	// If 'bIgnoreFixed' is 'true' we copy only user pipes.
	// Fixed pipes are those that are in the "Pipedb.txt".
	void CopyTo( CDataBase *pToDB, bool bIgnoreFixed = false );

	// Get a pointer to the CTable 'PIPE_TAB'
	// if pFromDB is NULL then 'this' is used
	// By default, create the 'PIPE_TAB' if missing
	CTable *GetPipeTab( CDataBase *pFromDB = NULL, bool bCreateIfMissing = true );

	// Loop on series and pipes to see if each IDPTR are valid.
	void CheckPipeTab( CDataBase *pFromDB = NULL );

	// Delete 'PIPE_TAB'.
	// if pFromDB is NULL then 'this' is used.
	void DeletePipeTab( CDataBase *pFromDB = NULL );

	// If false then the DB will not be saved to the disk.
	void SetSaveDB( bool bSaveDB );

	// Get flag if DB need to be saved to the disk.
	bool GetSaveDB();

	// Write exp file to export non locked pipes.
	void Export( CDataBase *pDB );

	// Override write to avoid writing if pipe are used temporarily.
	void Write( OUTSTREAM outf );

	// Override read to reset the save flag.
	void Read( INPSTREAM inpf );

	// Check Validity string from import.
	bool CheckImportValidity();

	// HYS-1590: Old ID form "{AAAA}_GEN_ID" is now converted to "AAAA_UPIPS_OLD" or "AAAA_UPIPE_OLD".
	// If 'pFromDB' is NULL we work on the pipe table in 'CPipeUserDatabase' otherwise we work on
	// the pipe table found in 'pFromDB'.
	// Returns 'true' if some IDs have been changed, otherwise return 'false'.
	bool RenameOldPipeIDs( CDataBase *pFromDB = NULL );

	// Convert old ID to the new.
	enum
	{
		Convert_PipeSeries,
		Convert_Pipe
	};
	CString RenameOneOldPipeID( CString strOldPipeID, int iConvertWhat );

	// Check if the pipe series or pipe ID is an old one (Like "{AAAA}_GEN_ID").
	bool IsPipeIDOld( CString strPipeID );

	// Check if similar series or pipes are found in an another DB.
	bool FindDuplicate( CDataBase *pFromDB );

	// Remove fixed pipes in PIPE_TAB. Must be done before saving.
	void RemoveFixed();

	// Allow to temporarily marked deleted pipes as available.
	void ResetDeletedFlag( std::vector<CString> &vecPipeSeries );

	// HYS-935: Because bug in autosave with previous HySelect version, we can have bad "pipes.db". 
	void CleanPipesDB( void );

// Private methods.
private:
	// Pass 2 'PIPE_TAB' and this method copy all series and pipes.
	void _CopySeries( CTable *pFrom, CTable *pTo, bool bIgnoreFixed = false );

	// Pass 2 pipes series and copy pipes from one series to the other.
	void _CopyPipes( CTable *pFrom, CTable *pTo );

// Private variables.
private:
	// If we load temporarily some pipe we don't want to keep.
	bool m_bSaveDB;
};

