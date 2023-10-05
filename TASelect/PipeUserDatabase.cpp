#include "stdafx.h"

#include "TASelect.h"
#include "PipeUserDatabase.h"
#include "DataBObj.h"


CPipeUserDatabase::CPipeUserDatabase() 
	: CDataBase( CDataBase::PipeUserDatabase )
{
	m_bSaveDB = true;
}

void CPipeUserDatabase::CopyFrom( CDataBase *pFromDB, bool bIgnoreFixed )
{
	if( NULL == pFromDB )
	{
		ASSERT_RETURN;
	}

	// Loop on pipe series tables to introduce them in pToDB.
	_CopySeries( GetPipeTab( pFromDB, false ), GetPipeTab(), bIgnoreFixed );
}

void CPipeUserDatabase::CopyPipeSeriesFrom( std::vector<CString> &vecPipeSeries, CDataBase *pFromDB )
{
	// HYS-1590: Allow to copy pipe series in the pipe user database.
	// It is essentially used by the 'CTADatastruct::Read' method.

	CTable *pclSrcPipeTable = GetPipeTab( pFromDB, false );
	
	if( NULL == pclSrcPipeTable )
	{
		ASSERT_RETURN;
	}

	for( auto &iter : vecPipeSeries )
	{
		if( NULL == dynamic_cast<CTable *>( pclSrcPipeTable->GetDB()->Get( iter ).MP ) )
		{
			ASSERT_CONTINUE;
		}

		// First, create the pipe series table.
		IDPTR PipeSeriesIDPtr = _NULL_IDPTR;
		GetPipeTab()->GetDB()->CreateObject( PipeSeriesIDPtr, CLASS( CTable ), (LPCTSTR)iter );

		if( NULL == dynamic_cast<CTable *>( PipeSeriesIDPtr.MP ) )
		{
			ASSERT_RETURN;
		}

		// Insert the pipe series table in the current pipe user database.
		GetPipeTab()->Insert( PipeSeriesIDPtr );

		CTable *pclSrcPipeSeriesTab = dynamic_cast<CTable *>( pclSrcPipeTable->GetDB()->Get( iter ).MP );
		CTable *pclDstPipeSeriesTab = dynamic_cast<CTable *>( PipeSeriesIDPtr.MP );
		pclSrcPipeSeriesTab->Copy( pclDstPipeSeriesTab );

		// Now copy the pipes in this pipe series table.
		_CopyPipes( pclSrcPipeSeriesTab, pclDstPipeSeriesTab );
	}
}

void CPipeUserDatabase::CopyPipeSeriesTo( std::vector<CTable *> &vecPipeSeries, CTable *pclTableWhereToCopy )
{
	// HYS-1590: Allow to copy some user pipe series from the vector to a table.
	// It is essentially used by the 'CTADatastruct::AddUsedUserPipesBeforeSavingProject' method.

	if( NULL == pclTableWhereToCopy )
	{
		ASSERT_RETURN;
	}

	for( auto &iter : vecPipeSeries )
	{
		// First, create the pipe series table.
		CString PipeSerieID = iter->GetIDPtr().ID;
		IDPTR PipeSeriesIDPtr = _NULL_IDPTR;
		pclTableWhereToCopy->GetDB()->CreateObject( PipeSeriesIDPtr, CLASS( CTable ), (LPCTSTR)PipeSerieID );
		ASSERT( _NULL_IDPTR != PipeSeriesIDPtr );

		// Insert the pipe series table in the current pipe user database.
		pclTableWhereToCopy->Insert( PipeSeriesIDPtr );

		CTable *pPipeSeriesTab = dynamic_cast<CTable *>( PipeSeriesIDPtr.MP );
		iter->Copy( pPipeSeriesTab );

		// Now copy the pipes in this pipe series table.
		CTable *pclPipeSeries = (CTable *)( PipeSeriesIDPtr.MP );
		_CopyPipes( iter, pclPipeSeries );
	}
}

void CPipeUserDatabase::CopyTo( CDataBase *pToDB, bool bIgnoreFixed )
{
	if( NULL == pToDB )
	{
		ASSERT( 0 );
		return;
	}

	// Loop on pipe series tables to introduce them in pToDB.
	_CopySeries( GetPipeTab(), GetPipeTab( pToDB ), bIgnoreFixed );
}

CTable *CPipeUserDatabase::GetPipeTab( CDataBase *pFromDB, bool bCreateIfMissing )
{
	HYSELECT_TRY
	{
		CTable *pPipeTab = NULL;
		CDataBase *pDB = pFromDB;

		// if CDataBase* is missing use 'this'
		if( NULL == pDB )
		{
			pDB = this;
		}

		if( true == bCreateIfMissing && _T('\0') == *( (*pDB).Access().Get( _T("PIPE_TAB") ).ID ) )
		{
			IDPTR IDPtrTo;
			pDB->CreateObject( IDPtrTo, CLASS( CTable ), _T("PIPE_TAB") );

			pPipeTab = dynamic_cast<CTable *>( IDPtrTo.MP );
			pPipeTab->SetName( _T("Pipes") );

			pDB->Access().Insert( IDPtrTo );
		}

		pPipeTab = (CTable *)( (*pDB).Access().Get( _T("PIPE_TAB") ).MP );

		if( true == bCreateIfMissing && NULL == pPipeTab )
		{
			HYSELECT_THROW( _T("Internal error: 'PIPE_TAB' table has not been created.") );
		}
		
		return pPipeTab;
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CPipeUserDatabase::GetPipeTab'.") )
}

void CPipeUserDatabase::CheckPipeTab( CDataBase *pFromDB )
{
	CTable *pPipeTab = GetPipeTab( pFromDB );

	if( NULL == pPipeTab )
	{
		return;
	}

	for( IDPTR PSIDPtr = pPipeTab->GetFirst(); _T('\0') != *PSIDPtr.ID; PSIDPtr = pPipeTab->GetNext() )
	{
		Extend( &PSIDPtr );
		CTable *PSTab = dynamic_cast<CTable *>( PSIDPtr.MP );

		// For each series scan each pipe.
		for( IDPTR PIDPtr = PSTab->GetFirst(); _T('\0') != *PIDPtr.ID; PIDPtr = PSTab->GetNext() )
		{
			Extend( &PIDPtr );
		}
	}
}

void CPipeUserDatabase::DeletePipeTab( CDataBase *pFromDB )
{
	CDataBase *pDB = pFromDB;

	if( NULL == pDB )
	{
		pDB = this;
	}

	CTable *pPipeTab = GetPipeTab( pDB, false );

	if( NULL == pPipeTab )
	{
		return;
	}

	pDB->DeleteObjectRecursive( pPipeTab, true );
}

void CPipeUserDatabase::SetSaveDB( bool bSaveDB )
{
	m_bSaveDB = bSaveDB;
}

bool CPipeUserDatabase::GetSaveDB()
{
	return m_bSaveDB;
}

void CPipeUserDatabase::Export( CDataBase *pFromDB )
{
	HYSELECT_TRY
	{
		IDPTR IDPtr;
		CreateObject( IDPtr, CLASS( CDB_String ), L"VALIDITY" );

		CDB_String *pValStr = dynamic_cast<CDB_String *>( IDPtr.MP );
		pValStr->SetString( L"^&aéze'r(t-yèu_içoàp)^" );
		
		Access().Insert( IDPtr );

		// 'true' to not export the fixed pipes (The ones that are defined in the "Pipedb.txt").
		// We then export only the user pipes.
		CopyFrom( pFromDB, true );
		CheckPipeTab();
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CPipeUserDatabase::Export'.") )
}

void CPipeUserDatabase::Write( OUTSTREAM outf )
{
	if( true == m_bSaveDB )
	{
		CDataBase::Write( outf );
	}
}

void CPipeUserDatabase::Read( INPSTREAM inpf )
{
	HYSELECT_TRY
	{
		m_bSaveDB = true;
		CDataBase::Read( inpf );
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CPipeUserDatabase::Read'.") )
}

bool CPipeUserDatabase::CheckImportValidity()
{
	// Read Validity string.
	CDB_String *pVal = dynamic_cast<CDB_String *>( Access().Get( L"VALIDITY" ).MP );

	if( NULL != pVal )
	{
		if( 0 == _tcscmp( pVal->GetString(), L"^&aéze'r(t-yèu_içoàp)^" ) )
		{
			return true;
		}
	}

	return false;
}

bool CPipeUserDatabase::RenameOldPipeIDs( CDataBase *pFromDB )
{
	// HYS-1590: Old ID form "{AAAA}_GEN_ID" is now converted to "AAAA_UPIPS_OLD" or "AAAA_UPIPE_OLD".
	// See more details in the Jira card.

	// Even if we also verify the 'pFromDB' parameter in the "GetPipeTab" method, we need it below
	// when calling the "pFromDB->GetpDataList()" method.
	if( NULL == pFromDB )
	{
		pFromDB = this;
	}

	if( NULL == GetPipeTab( pFromDB ) )
	{
		ASSERTA_RETURN( false );
	}

	// We need to memorize all the IDs to change because we can't change during the loop.
	std::vector<std::pair<CString, int>> vecAllChanges;

	CTable *pclPipeTable = GetPipeTab( pFromDB );

	for( IDPTR PipeSeriesIDPtr = pclPipeTable->GetFirst( CLASS( CTable ) ); _NULL_IDPTR != PipeSeriesIDPtr; PipeSeriesIDPtr = pclPipeTable->GetNext() )
	{
		if( true == IsPipeIDOld( CString( PipeSeriesIDPtr.ID ) ) )
		{
			vecAllChanges.push_back( std::pair<CString, int>( CString( PipeSeriesIDPtr.ID ), (int)Convert_PipeSeries ) );
		}

		CTable *pclTablePipeSeries = dynamic_cast<CTable *>( PipeSeriesIDPtr.MP );

		if( NULL == pclTablePipeSeries )
		{
			ASSERT_CONTINUE;
		}

		for( IDPTR PipeIDPtr = pclTablePipeSeries->GetFirst( CLASS( CDB_Pipe ) ); _T('\0') != *PipeIDPtr.ID; PipeIDPtr = pclTablePipeSeries->GetNext() )
		{
			if( true == IsPipeIDOld( CString( PipeIDPtr.ID ) ) )
			{
				vecAllChanges.push_back( std::pair<CString, int>( CString( PipeIDPtr.ID ), (int)Convert_Pipe ) );
			}
		}
	}

	CDataList *pclTADSDataList = pFromDB->GetpDataList();

	// Now we can rename. BUT: All objects are in a 'CTADatastruct::CDataList' class that saves objects in "m_ppDataArray" in alphabetical
	// order. So, we can't just rename the ID. We need to remove the object from the "CTADatastruct::CDataList", rename it and insert it back again.
	// If object to rename is a pipe series: we need also to remove it from its parent ("PIPE_TAB" table) and insert it back again.
	// If object to rename is a pipe: we need also to remove from its parent (Pipe series table) and insert it back again AND we need to change the owner ID!
	for( auto &iter : vecAllChanges )
	{
		IDPTR IDPtr = pclTADSDataList->Remove( iter.first );
		CString strOldID = IDPtr.ID;

		// We can't change the object ID right now. Why ?
		// Because if we do it now, the ID in the parent table ("PIPE_TAB" table for a pipe series object or pipe series table for a pipe) will be also 
		// changed (Because the parent table points to the same object).
		// So, if we need to remove the object from the parent table to insert it back again at the right position, the 'CDataList::Remove'
		// method will not find the object. This method uses a dichotomic method to search object before to remove it. And because we have changed 
		// the ID, the 'CDataList::m_ppDataArray' of the parent does not contain anymore a list alphabetically sorted. So the 'CDataList::Remove' will
		// not find anymore the object and will fire an ASSERT.

		// Remove also the object from its parent table.
		// For a pipe series it is the 'PIPE_TAB' table.
		// For a pipe it is the pipe series table.
		CTable *pclParentTable = dynamic_cast<CTable *>( IDPtr.PP );

		if( NULL == pclParentTable || NULL == pclParentTable->GetpDataList() )
		{
			ASSERT_CONTINUE;
		}

		// Retrieve the 'ParentTable::CDataList' object in which is saved the current object.
		CDataList *pclParentTableDataList = pclParentTable->GetpDataList();

		// Remove the object from its parent table.
		IDPTR IDPtrInParentTable = pclParentTableDataList->Remove( strOldID );

		// Change the old ID.
		CString strNewID = RenameOneOldPipeID( iter.first, iter.second );
		
		if( true == strNewID.IsEmpty() )
		{
			ASSERT_CONTINUE;
		}

		// Change 'ID' in the 'IDPtr' object to well replace the object in 'm_ppDataArray' when calling the 'CTADastruct::CDataList::Insert' method.
		_tcscpy( IDPtr.ID, strNewID );

		// Change 'ID' in the 'IDPtrInParentTable' object to well replace the object in 'm_ppDataArray' when calling the 'ParentTable::CDataList::Insert' method.
		_tcscpy( IDPtrInParentTable.ID, strNewID );

		// Now, we need also to change the 'ID' in the object itself (CData::m_Identificator.ID).
		CData *pclData = (CData *)( IDPtr.MP );
		pclData->ChangeID( strNewID.GetBuffer() );

		// Insert the object back in 'CTADatastruct::CDataList'. Because the ID has changed, the position in the 'CTADatastruct::CDataList' will change.
		pclTADSDataList->Insert( IDPtr );

		// Insert the object back in 'ParentTable::CDataList'.
		pclParentTableDataList->Insert( IDPtrInParentTable );

		if( (int)Convert_Pipe == iter.second )
		{
			// If object is a pipe, we need also to change the ID in the 'CData::m_pOwners'.
			CString strOwnerNewID = RenameOneOldPipeID( pclData->m_pOwners->ID, (int)Convert_PipeSeries );

			if( true == strOwnerNewID.IsEmpty() )
			{
				ASSERT_CONTINUE;
			}
			
			_tcscpy( pclData->m_pOwners->ID, strOwnerNewID );
		}
	}

	return ( 0 == (int)vecAllChanges.size() ) ? false : true;
}

CString CPipeUserDatabase::RenameOneOldPipeID( CString strOldPipeID, int iConvertWhat )
{
	if( false == IsPipeIDOld( strOldPipeID ) )
	{
		return _T("");
	}

	CString strNew = strOldPipeID.Left( 5 ).Right( 4 );
	strNew += ( Convert_PipeSeries == iConvertWhat ) ? _T("_UPIPS_OLD") : _T("_UPIPE_OLD");

	return strNew;
}

bool CPipeUserDatabase::IsPipeIDOld( CString strPipeID )
{
	if( true == strPipeID.IsEmpty() )
	{
		return false;
	}

	// "{AAAA}_GEN_ID"
	if( 13 != strPipeID.GetLength() )
	{
		return false;
	}

	if( 0 != strPipeID.Right( 7 ).CompareNoCase( _T("_GEN_ID") ) )
	{
		return false;
	}

	return true;
}

bool CPipeUserDatabase::FindDuplicate( CDataBase *pFromDB )
{
	if( NULL == pFromDB )
	{
		ASSERTA_RETURN( false );
	}

	// Get a pointer to PIPE_TAB table.
	CTable *pThisPipeTab = GetPipeTab();

	// Get a pointer to PIPE_TAB table from pFromDB.
	CTable *pPipeTab = GetPipeTab( pFromDB );

	// Loops on pipe series.
	// To verify if the pipes are not already present in other DB.
	IDPTR SerieIDPtr = _NULL_IDPTR;
	IDPTR PipeIDPtr = _NULL_IDPTR;

	for( SerieIDPtr = pThisPipeTab->GetFirst( CLASS( CTable ) ); _T('\0') != *SerieIDPtr.ID; SerieIDPtr = pThisPipeTab->GetNext() )
	{
		CTable *pTab = dynamic_cast<CTable *>( SerieIDPtr.MP );

		if( _T('\0') != *pPipeTab->Get( SerieIDPtr.ID ).ID )
		{
			return true;
		}

		for( PipeIDPtr = pTab->GetFirst( CLASS( CDB_Pipe ) ); _T('\0') != *PipeIDPtr.ID; PipeIDPtr = pTab->GetNext() )
		{
			if( _T('\0') != *pPipeTab->Get( PipeIDPtr.ID ).ID )
			{
				return true;
			}
		}
	}

	return false;
}

void CPipeUserDatabase::RemoveFixed()
{
	// Get a pointer to PIPE_TAB table from m_PIPEDB.
	CTable *pPipeTab = GetPipeTab();

	// Loop on Pipe series tables.
	for( IDPTR IDPtr = pPipeTab->GetFirst( CLASS( CTable ) ); _T('\0') != *IDPtr.ID; )
	{
		CTable *pTab = dynamic_cast<CTable *>( IDPtr.MP );

		// Check if this is a table of fixed pipes.
		if( false == dynamic_cast<CDB_Pipe *>( pTab->GetFirst( CLASS( CDB_Pipe ) ).MP )->IsFixed() )
		{
			IDPtr = pPipeTab->GetNext();
			continue;
		}

		// Loop on existing pipes in Pipe series table pTab, remove them from the table and delete them from m_USERDB.
		for (IDPTR PipeIDPtr = pTab->GetFirst( CLASS( CDB_Pipe ) ); _T('\0') != *PipeIDPtr.ID; )
		{
			IDPTR PipeIDPtrNext = pTab->GetNext();
			pTab->Remove( PipeIDPtr );
			VERIFY( DeleteObject( PipeIDPtr ) );

			PipeIDPtr = PipeIDPtrNext;
		}

		// Remove the Pipe series table from the Pipe table and delete it from m_USERDB.
		IDPTR IDPtrNext = pPipeTab->GetNext();
		pPipeTab->Remove( IDPtr );
		VERIFY( DeleteObject( IDPtr ) );

		IDPtr = IDPtrNext;
	}
}

// HYS-935: Because bug in autosave with previous HySelect version, we can have bad "pipes.db". 
void CPipeUserDatabase::CleanPipesDB()
{
	if( NULL == GetpDataList() )
	{
		return;
	}

	// The 'pipes.db' file can contain fixed pipe series or/and pipes (Fixed are the ones that are in the 
	// 'CTADatastruct' class (in tadb.db)).

	std::vector<CData * >OrphanList;
	IDPTR IDPtrPrevious;
	
	for( IDPTR IDPtr = GetpDataList()->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = GetpDataList()->GetNext() )
	{
		if( NULL != dynamic_cast<CTable *>( IDPtr.MP ) )
		{
			// Okay, it's a table. Check first if this one is already existing in the 'CTADatabase'.
			if( NULL == dynamic_cast<CTable *>( TASApp.GetpTADB()->Get( IDPtr.ID ).MP ) )
			{
				continue;
			}

			// Yes this table is already existing. Check now if it's well a table belonging to the 'PIPE_TAB'.
			CTable *pTable = (CTable *)( TASApp.GetpTADB()->Get( IDPtr.ID ).MP );
			
			if( NULL == dynamic_cast<CTable *>( pTable->GetIDPtr().PP ) )
			{
				continue;
			}

			pTable = (CTable *)( pTable->GetIDPtr().PP );
			
			if( 0 != CString( pTable->GetName() ).Compare( _T("PIPE_TAB") ) )
			{
				continue;
			}

			// Add this object to be deleted.
			OrphanList.push_back( IDPtr.MP );
		}
		else if( NULL != dynamic_cast<CDB_Pipe *>( IDPtr.MP ) )
		{
			// Okay, it's a pipe. Check first if this one is already existing in the 'CTADatabase'.
			if( NULL == dynamic_cast<CDB_Pipe *>( TASApp.GetpTADB()->Get( IDPtr.ID ).MP ) )
			{
				continue;
			}

			// Add this object to be deleted.
			OrphanList.push_back( IDPtr.MP );
		}
	}
	
	for( int i = 0; i < (int)OrphanList.size(); i++ )
	{
		IDPTR IDPtr = OrphanList.at( i )->GetIDPtr();
		DeleteObject( IDPtr, true );
	}
}

void CPipeUserDatabase::ResetDeletedFlag( std::vector<CString> &vecPipeSeries )
{
	// Get a pointer to PIPE_TAB table from m_PIPEDB.
	CTable *pclPipeTable = GetPipeTab();

	for( auto &iter : vecPipeSeries )
	{
		CTable *pclPipeSeries = dynamic_cast<CTable *>( pclPipeTable->GetDB()->Get( iter ).MP );
		
		if( NULL == pclPipeSeries )
		{
			ASSERT_CONTINUE;
		}

		pclPipeSeries->SetDeleted( false );

		// Loop on existing pipes in this pipe series table and reset deleted flag.
		for( IDPTR PipeIDPtr = pclPipeSeries->GetFirst( CLASS( CDB_Pipe ) ); _T('\0') != *PipeIDPtr.ID; )
		{
			CDB_Pipe *pclPipe = dynamic_cast<CDB_Pipe *>( PipeIDPtr.MP );

			if( NULL == pclPipe )
			{
				ASSERT_CONTINUE;
			}

			pclPipe->SetDeleted( false );
			PipeIDPtr = pclPipeSeries->GetNext();
		}
	}
}

void CPipeUserDatabase::_CopySeries( CTable *pFrom, CTable *pTo, bool bIgnoreFixed )
{
	HYSELECT_TRY
	{
		if( NULL == pFrom )
		{
			// Can be NULL if there are no pipe series in the database (It was the case before).
			return;
		}
		else if( NULL == pTo )
		{
			HYSELECT_THROW( _T("Internal error: 'pTo' argument can't be NULL.") );
		}

		// Run all pipe series from 'pFrom'.
		for( IDPTR IDPtr = pFrom->GetFirst( CLASS( CTable ) ); _T('\0') != *IDPtr.ID; IDPtr = pFrom->GetNext() )
		{
			CTable *pclPipeSeriesTableSource = dynamic_cast<CTable *>( IDPtr.MP );
			CDB_Pipe *pclFirstPipeInPipeSeries = dynamic_cast<CDB_Pipe *>( pclPipeSeriesTableSource->GetFirst( CLASS( CDB_Pipe ) ).MP );
			
			if( NULL == pclFirstPipeInPipeSeries )
			{
				HYSELECT_THROW( _T("Internal error: There is no pipe in the '%s' pipe table."), IDPtr.ID );
			}

			if( true == bIgnoreFixed && true == pclFirstPipeInPipeSeries->IsFixed() )
			{
				// No need to add fixed pipes (These pipes are the ones defined in the "Pipedb.txt".
				continue;
			}

			IDPTR PipeSeriesIDPtr;
			pTo->GetDB()->CreateObject( PipeSeriesIDPtr, CLASS( CTable ), IDPtr.ID );

			CTable *pclPipeSeriesTableDest = dynamic_cast<CTable *>( PipeSeriesIDPtr.MP );
			pclPipeSeriesTableSource->Copy( pclPipeSeriesTableDest );

			pTo->Insert( PipeSeriesIDPtr );
		
			_CopyPipes( pclPipeSeriesTableSource, pclPipeSeriesTableDest );
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CPipeUserDatabase::_CopySeries'.") )
}

void CPipeUserDatabase::_CopyPipes( CTable *pFrom, CTable *pTo )
{
	HYSELECT_TRY
	{
		CDataBase *pToDB = pTo->GetDB();

		// Loop on existing pipes in Pipe series table pTab and introduce them in pToDB.
		for( IDPTR FromPipeIDPtr = pFrom->GetFirst( CLASS( CDB_Pipe ) ); _T('\0') != *FromPipeIDPtr.ID; FromPipeIDPtr = pFrom->GetNext() )
		{
			IDPTR PipeIDPtr;
			pToDB->CreateObject( PipeIDPtr, CLASS( CDB_Pipe ), FromPipeIDPtr.ID );

			CDB_Pipe *pPipe = dynamic_cast<CDB_Pipe *>( PipeIDPtr.MP );
			CDB_Pipe *pFromPipe = dynamic_cast<CDB_Pipe *>( FromPipeIDPtr.MP );
		
			// Copy properties from pFromDB into the new Pipe in pToDB.
			pFromPipe->Copy( pPipe );

			pTo->Insert( PipeIDPtr );

			// Force availability (bug in previous version).
			pPipe->SetAvailable( TRUE );
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CPipeUserDatabase::_CopyPipes'.") )
}
