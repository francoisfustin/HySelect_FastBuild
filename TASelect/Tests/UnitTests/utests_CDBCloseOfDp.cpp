#include "stdafx.h"

#include <string>
#include "utests_base.h"
#include "utests_CDBCloseOffDp.h"
#include "DataBObj.h"
#include "TASelect.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_CDBCloseOffDp, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_CDBCloseOffDp::GetCloseOffDp()
{
	CArray<CString, CString> arTables;
	arTables.Add( _T("CTRLVALV_TAB") );
	arTables.Add( _T("BALCTRLVALV_TAB") );
	arTables.Add( _T("PICTRLVALV_TAB") );
	arTables.Add( _T("DPCBALCTRLVALV_TAB") );

	bool bAtLeastOneOutput = false;
	CString strFileName = _T("GetCloseOffDp.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );

	for( int iLoopTab = 0; iLoopTab < arTables.GetCount(); iLoopTab++ )
	{
		IDPTR TabIDPtr = GetpTADB()->Get( arTables.GetAt( iLoopTab ) );
		CTable *pclTable = dynamic_cast<CTable *>( ( CData * )TabIDPtr.MP );
		CPPUNIT_ASSERT( pclTable );
		int iValveCount = 0;

		for( IDPTR ValveIDPtr = pclTable->GetFirst(); _T( '\0' ) != *ValveIDPtr.ID; ValveIDPtr = pclTable->GetNext() )
		{
			iValveCount++;
			CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( ( CData * )ValveIDPtr.MP );
			CPPUNIT_ASSERT( pclControlValve );

			CDB_CloseOffChar *pclCloseOffChar = dynamic_cast<CDB_CloseOffChar *>( ( CData * )pclControlValve->GetCloseOffCharIDPtr().MP );

			if( NULL == pclCloseOffChar )
			{
				continue;
			}

			CTable *pclActuatorGroup = dynamic_cast<CTable *>( ( CData * )pclControlValve->GetActuatorGroupIDPtr().MP );

			if( NULL == pclActuatorGroup )
			{
				continue;
			}

			CString strCirLin = ( CDB_CloseOffChar::eOpenType::Linear == pclCloseOffChar->GetOpenType() ) ? _T("Linear") : _T("Circular");
			CString strLimitType = ( CDB_CloseOffChar::eLimitType::CloseOffDp == pclCloseOffChar->GetLimitType() ) ? _T("CloseOffDp") : _T("InletPressure");

			for( IDPTR ActuatorIDPtr = pclActuatorGroup->GetFirst(); _T( '\0' ) != *ActuatorIDPtr.ID; ActuatorIDPtr = pclActuatorGroup->GetNext() )
			{
				CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( ( CData * )ActuatorIDPtr.MP );
				CPPUNIT_ASSERT( pclActuator );

				double dMaxForceTorque = pclActuator->GetMaxForceTorque();

				if( 0.0 == dMaxForceTorque )
				{
					continue;
				}

				double dCloseOffDp = pclCloseOffChar->GetCloseOffDp( dMaxForceTorque );

				if( DBL_MIN == dCloseOffDp )
				{
					if( false == bAtLeastOneOutput )
					{
						pOutf->WriteTxtLine( _T("Table; Number; Valve ID; V. name; V. Close off char. Lin/Cir; V. C. Type; V. C. Pts; V. C. DpMin; V. C. DpMax; V. C. Force/Torque Min; V. C. Force/Torque max.; Actuator ID; Act. name; Act. MaxForceTorque\n") );
					}

					CString message;

					message.Format( _T("%s; %i; %s; %s; %s; %s; %i; %f; %f; %f; %f; %s; %s; %f\n"), 
						arTables.GetAt( iLoopTab ), 
						iValveCount, 
						CString( pclControlValve->GetIDPtr().ID ), 
						CString( pclControlValve->GetName() ), 
						strCirLin,
						strLimitType,
						pclCloseOffChar->GetPointsInCharacteristic(),
						pclCloseOffChar->GetCloseOffDpMin(),
						pclCloseOffChar->GetCloseOffDpMax(),
						pclCloseOffChar->GetForceTorqueMin(),
						pclCloseOffChar->GetForceTorqueMax(),
						CString( pclActuator->GetIDPtr().ID ), 
						CString( pclActuator->GetName() ),
						dMaxForceTorque );

					pOutf->WriteTxtLine( ( LPCTSTR )message );
					bAtLeastOneOutput = true;
				}
			}
		}
	}

	if( true == bAtLeastOneOutput )
	{
		CPPUNIT_FAIL( "Some actuators haven't enough force/torque to close valve. See the 'GetCloseOffDp.txt' file." );
	}

	utests_Init::CloseTxtFile( pOutf );
}
