#include "stdafx.h"

#include "utests_base.h"
#include "utests_SelectActuator.h"
#include "DataStruct.h"
#include "TADataBase.h"
#include "TASelect.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_SelectActuator, CPPUNIT_TEST_CATEGORYNAME_ACTUATORS );

void utests_SelectActuator::getActuatorList()
{
	CTable *pTab = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	CString strFileName = _T("ActList.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );

	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( IDPtr.MP );

		if( NULL == pActuator  || false == pActuator->IsSelectable(true) )
		{
			continue;
		}

		CString str;
		CString strName = pActuator->GetName();
		CString strDescription = pActuator->GetComment();
		CString strArtNumber = pActuator->GetArtNum();
		str.Format( _T("%s;%s;%s;%s\n"), IDPtr.ID, strName, strDescription, strArtNumber );
		pOutf->WriteTxtLine( ( LPCTSTR )str );
	}

	utests_Init::CloseTxtFile( pOutf );

	// Compare with reference file.
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );
}

void utests_SelectActuator::getCVActuatorList()
{
	CTable *pTab = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("CTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CString strFileName = _T("CTRLVALV_TAB.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );

	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ControlValve *pValve = dynamic_cast<CDB_ControlValve *>( IDPtr.MP );

		if( NULL == pValve || false == pValve->IsSelectable(true) )
		{
			continue;
		}

		CTable *pActrTab = static_cast< CTable *>( pValve->GetActuatorGroupIDPtr().MP );
		ASSERT( NULL != pTab );

		if( NULL != pActrTab )
		{
			// For each actuator present into the actuator group table, verify the existence into the ActrFullList.
			for( IDPTR idptra = pActrTab->GetFirst(); NULL != idptra.MP; idptra = pActrTab->GetNext( idptra.MP ) )
			{

				CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( idptra.MP );

				if( NULL == pActuator || false == pActuator->IsSelectable(true) )
				{
					continue;
				}

				CString str;
				str.Format( _T("%s;%s;%s;%s\n"), pValve->GetIDPtr().ID, pValve->GetFamilyID(), pValve->GetSizeID(), pActuator->GetIDPtr().ID );
				pOutf->WriteTxtLine( ( LPCTSTR )str );

			}
		}

	}

	utests_Init::CloseTxtFile( pOutf );

	// Compare with reference file.
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );
}

void utests_SelectActuator::getBCVActuatorList()
{
	CTable *pTab = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("PICTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CString strFileName = _T("PICTRLVALV_TAB.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );

	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ControlValve *pValve = dynamic_cast<CDB_ControlValve *>( IDPtr.MP );

		if( NULL == pValve || false == pValve->IsSelectable(true) )
		{
			continue;
		}

		CTable *pActrTab = static_cast< CTable *>( pValve->GetActuatorGroupIDPtr().MP );
		ASSERT( NULL != pTab );

		if( NULL != pActrTab )
		{
			// For each actuator present into the actuator group table, verify the existence into the ActrFullList.
			for( IDPTR idptra = pActrTab->GetFirst(); NULL != idptra.MP; idptra = pActrTab->GetNext( idptra.MP ) )
			{

				CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( idptra.MP );

				if( NULL == pActuator || false == pActuator->IsSelectable(true) )
				{
					continue;
				}

				CString str;
				str.Format( _T("%s;%s;%s;%s\n"), pValve->GetIDPtr().ID, pValve->GetFamilyID(), pValve->GetSizeID(), pActuator->GetIDPtr().ID );
				pOutf->WriteTxtLine( ( LPCTSTR )str );

			}
		}

	}

	utests_Init::CloseTxtFile( pOutf );

	// Compare with reference file.
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );
}

void utests_SelectActuator::getTRVActuatorList()
{
	// HYS-1774: We also added FLCTRLVALV_TAB and TRINSERT_TAB.
	int iTabCount = 0;
	CTable *pTabTrv = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("TRVALV_TAB") ).MP );
	ASSERT( NULL != pTabTrv );
	iTabCount++;
	CTable *pTabFLC = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("FLCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTabFLC );
	iTabCount++;
	CTable *pTabInsert = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("TRINSERT_TAB") ).MP );
	ASSERT( NULL != pTabInsert );
	iTabCount++;

	CString strFileName = _T("TRVALV_TAB.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );
	
	int i = 0;
	while( i < iTabCount )
	{
		CTable* pTab = NULL;

		switch( i ) {
		case 0:
			pTab = pTabTrv;
			break;

		case 1:
			pTab = pTabFLC;
			break;

		case 2:
			pTab = pTabInsert;
			break;
		}

		if( NULL != pTab )
		{
			for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
			{
				CDB_ControlValve* pValve = dynamic_cast<CDB_ControlValve*>(IDPtr.MP);

				if( NULL == pValve || false == pValve->IsSelectable( true ) )
				{
					continue;
				}

				CTable* pActrTab = static_cast<CTable*>(pValve->GetActuatorGroupIDPtr().MP);
				ASSERT( NULL != pTab );

				if( NULL != pActrTab )
				{
					// For each actuator present into the actuator group table, verify the existence into the ActrFullList.
					for( IDPTR idptra = pActrTab->GetFirst(); NULL != idptra.MP; idptra = pActrTab->GetNext( idptra.MP ) )
					{

						CDB_Actuator* pActuator = dynamic_cast<CDB_Actuator*>(idptra.MP);

						if( NULL == pActuator || false == pActuator->IsSelectable( true ) )
						{
							continue;
						}

						CString str;
						str.Format( _T( "%s;%s;%s;%s\n" ), pValve->GetIDPtr().ID, pValve->GetFamilyID(), pValve->GetSizeID(), pActuator->GetIDPtr().ID );
						pOutf->WriteTxtLine( (LPCTSTR)str );

					}
				}

			}
		}
		i++;
	}

	utests_Init::CloseTxtFile( pOutf );

	// Compare with reference file.
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );
}

void utests_SelectActuator::getBALActuatorList()
{
	CTable *pTab = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("BALCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CString strFileName = _T("BALCTRLVALV_TAB.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );

	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ControlValve *pValve = dynamic_cast<CDB_ControlValve *>( IDPtr.MP );

		if( NULL == pValve || false == pValve->IsSelectable(true) )
		{
			continue;
		}

		CTable *pActrTab = static_cast< CTable *>( pValve->GetActuatorGroupIDPtr().MP );
		ASSERT( NULL != pTab );

		if( NULL != pActrTab )
		{
			// For each actuator present into the actuator group table, verify the existence into the ActrFullList.
			for( IDPTR idptra = pActrTab->GetFirst(); NULL != idptra.MP; idptra = pActrTab->GetNext( idptra.MP ) )
			{

				CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( idptra.MP );

				if( NULL == pActuator || false == pActuator->IsSelectable(true) )
				{
					continue;
				}

				CString str;
				str.Format( _T("%s;%s;%s;%s\n"), pValve->GetIDPtr().ID, pValve->GetFamilyID(), pValve->GetSizeID(), pActuator->GetIDPtr().ID );
				pOutf->WriteTxtLine( ( LPCTSTR )str );

			}
		}

	}

	utests_Init::CloseTxtFile( pOutf );

	// Compare with reference file.
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );
}

void utests_SelectActuator::getDPCActuatorList()
{
	CTable *pTab = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("DPCBALCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CString strFileName = _T("DPCBALCTRLVALV_TAB.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );

	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ControlValve *pValve = dynamic_cast<CDB_ControlValve *>( IDPtr.MP );

		if( NULL == pValve || false == pValve->IsSelectable(true) )
		{
			continue;
		}

		CTable *pActrTab = static_cast< CTable *>( pValve->GetActuatorGroupIDPtr().MP );
		ASSERT( NULL != pTab );

		if( NULL != pActrTab )
		{
			// For each actuator present into the actuator group table, verify the existence into the ActrFullList.
			for( IDPTR idptra = pActrTab->GetFirst(); NULL != idptra.MP; idptra = pActrTab->GetNext( idptra.MP ) )
			{

				CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( idptra.MP );

				if( NULL == pActuator || false == pActuator->IsSelectable(true) )
				{
					continue;
				}

				CString str;
				str.Format( _T("%s;%s;%s;%s\n"), pValve->GetIDPtr().ID, pValve->GetFamilyID(), pValve->GetSizeID(), pActuator->GetIDPtr().ID );
				pOutf->WriteTxtLine( ( LPCTSTR )str );

			}
		}

	}

	utests_Init::CloseTxtFile( pOutf );

	// Compare with reference file.
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS ), CString( __FILE__ ), strFileName );
}

void utests_SelectActuator::verifyCVActuatorSet()
{
	verifyActuatorSet( _T("CVACTSET_TAB" ), _T( "CTRLVALV_TAB") );
}

void utests_SelectActuator::verifyPICVActuatorSet()
{
	verifyActuatorSet( _T("PICVACTSET_TAB" ), _T( "PICTRLVALV_TAB") );
}

void utests_SelectActuator::verifyBCVActuatorSet()
{
	verifyActuatorSet( _T("BCVACTSET_TAB" ), _T( "BALCTRLVALV_TAB") );
}

void utests_SelectActuator::verifyActuatorSet( TCHAR *actSet, TCHAR *valveTab )
{
	CTable *pSet = static_cast<CTable *>( TASApp.GetpTADB()->Get( actSet ).MP );
	ASSERT( NULL != pSet );
	std::string setErrorList;

	bool setExistSeparately = false;

	for( IDPTR IDPtrSet = pSet->GetFirst(); _T('\0') != *IDPtrSet.ID; IDPtrSet = pSet->GetNext() )
	{
		CDB_Set *pSet = dynamic_cast<CDB_Set *>( IDPtrSet.MP );
		CPPUNIT_ASSERT( NULL != pSet );

		if( NULL == pSet || false == pSet->IsSelectable( true ) )
		{
			continue;
		}

		IDPTR valveInSet = pSet->GetFirstIDPtr();
		IDPTR actuatorInSet = pSet->GetSecondIDPtr();

		CTable *pTab = static_cast<CTable *>( TASApp.GetpTADB()->Get( valveTab ).MP );
		CPPUNIT_ASSERT( NULL != pTab );

		setExistSeparately = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			if( 0 != _tcscmp( valveInSet.ID, IDPtr.ID ) )
			{
				continue;
			}

			CDB_ControlValve *pValve = dynamic_cast<CDB_ControlValve *>( IDPtr.MP );
			CPPUNIT_ASSERT( NULL != pValve );

			if( NULL == pValve )
			{
				continue;
			}

			CTable *pActrTab = static_cast<CTable *>( pValve->GetActuatorGroupIDPtr().MP );
			CPPUNIT_ASSERT( NULL != pTab );

			if( NULL != pActrTab )
			{
				// For each actuator present into the actuator group table, verify the existence into the ActrFullList.
				for( IDPTR idptra = pActrTab->GetFirst(); NULL != idptra.MP; idptra = pActrTab->GetNext( idptra.MP ) )
				{
					if( 0 != _tcscmp( actuatorInSet.ID, idptra.ID ) )
					{
						continue;
					}

					CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( idptra.MP );
					CPPUNIT_ASSERT( NULL != pActuator );

					if( NULL == pActuator )
					{
						continue;
					}

					setExistSeparately = true;
					break;
				}
			}

		}

		if( false == setExistSeparately )
		{
			CString setID = pSet->GetName();

			setID += " ( ";
			setID += pSet->GetIDPtr().ID;
			setID += " - ";
			setID += pSet->GetArtNum();
			setID += " ) : ";
			setID += valveInSet.ID;
			setID += " + ";
			setID += actuatorInSet.ID;
			setID += "\n";

			CT2CA toAnsi( setID );

			setErrorList += toAnsi;
		}
	}

	CPPUNIT_ASSERT_MESSAGE( setErrorList, 0 == setErrorList.length() );
}
