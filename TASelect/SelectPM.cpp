#include "stdafx.h"
#include <map>
#include <algorithm>    // std::sort
#include <vector>       // std::vector


#include "TASelect.h"
#include "MainFrm.h"
#include "DataBase.h"
#include "ProductSelectionParameters.h"
#include "SelectPM.h"
#include "PMPressureGaugeHelper.h"
#include "DlgLeftTabSelManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CPMWQSelectionPreferences.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPMWQPrefs::Reset( void )
{
	ZeroMemory( m_arPMWQPrefState, PMWQP_Last * sizeof( PMWQPrefState ) );
}

bool CPMWQPrefs::CompareTo( CPMWQPrefs *pclPMWQSelectionPreferences )
{
	if( NULL == pclPMWQSelectionPreferences )
	{
		return false;
	}

	for( int iLoop = PMWQP_First; iLoop < PMWQP_Last; iLoop++ )
	{
		if( m_arPMWQPrefState[iLoop].m_bChecked != pclPMWQSelectionPreferences->IsChecked( (PMWQPreference)iLoop ) )
		{
			return false;
		}

		if( m_arPMWQPrefState[iLoop].m_bDisabled != pclPMWQSelectionPreferences->IsDisabled( (PMWQPreference)iLoop ) )
		{
			return false;
		}
	}

	return true;
}

void CPMWQPrefs::CopyFrom( CPMWQPrefs *pclPMWQSelectionPreferences )
{
	if( NULL == pclPMWQSelectionPreferences )
	{
		return;
	}

	Reset();

	for( int iLoop = PMWQP_First; iLoop < PMWQP_Last; iLoop++ )
	{
		m_arPMWQPrefState[iLoop].m_bChecked = pclPMWQSelectionPreferences->IsChecked( (PMWQPreference)iLoop );
		m_arPMWQPrefState[iLoop].m_bDisabled = pclPMWQSelectionPreferences->IsDisabled( (PMWQPreference)iLoop );
	}
}

void CPMWQPrefs::SetChecked( PMWQPreference ePMWQPreference, bool bSet )
{
	m_arPMWQPrefState[ePMWQPreference].m_bChecked = bSet;
}

void CPMWQPrefs::SetDisabled( PMWQPreference ePMWQPreference, bool bSet )
{
	// HYS-1537 : We don't set m_bDisabled to false because we should retreive this state
	// when the the option become enabled.
	m_arPMWQPrefState[ePMWQPreference].m_bDisabled = bSet;
}

void CPMWQPrefs::SetRedundancy( wchar_t *pID )
{
	if( NULL == pID )
	{
		return;
	}

	if( 0 == _tcscmp( pID, _T("PM_REDDCY_NONE" ) ) )
	{
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked = false;
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked = false;
	}
	else if( 0 == _tcscmp( pID, _T("PM_REDDCY_PUMPCOMP" ) ) )
	{
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked = true;
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked = false;
	}
	else if( 0 == _tcscmp( pID, _T("PM_REDDCY_TECBOX" ) ) )
	{
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked = false;
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked = true;
	}
	else if( 0 == _tcscmp( pID, _T("PM_REDDCY_ALL" ) ) )
	{
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked = true;
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked = true;
	}
	else
	{
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked = false;
		m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked = false;
	}
}

CString CPMWQPrefs::GetRedundancy( void )
{
	CString strID = _T("");
	// HYS-1537 : Look the disable state
	if( ( false == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked || true == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bDisabled )
		&& ( false == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked || true == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bDisabled ) )
	{
		strID = _T("PM_REDDCY_NONE");
	}
	else if( ( true == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked && false == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bDisabled )
			&& ( false == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked || true == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bDisabled ) )
	{
		strID = _T("PM_REDDCY_PUMPCOMP");
	}
	else if( ( true == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked && false == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bDisabled )
			&& ( false == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked || true == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bDisabled ) )
	{
		strID = _T("PM_REDDCY_TECBOX");
	}
	else if( ( true == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bChecked && false == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyTecBox].m_bDisabled )
			&& ( true == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bChecked && false == m_arPMWQPrefState[CPMWQPrefs::GeneralRedundancyPumpComp].m_bDisabled ) )
	{
		strID = _T("PM_REDDCY_ALL");
	}

	return strID;
}

bool CPMWQPrefs::IsRedundancyIDMatch( wchar_t *pID )
{
	bool bReturn = false;
	
	// HYS-1537 : Look the disable state
	if( 0 == _tcscmp( pID, _T("PM_REDDCY_NONE") ) 
		&& ( false == IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) || true == IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp ) )
		&& ( false == IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) || true == IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) ) )
	{
		bReturn = true;
	}
	else if( 0 == _tcscmp( pID, _T("PM_REDDCY_PUMPCOMP") ) 
		&& ( true == IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) && false == IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp ) )
		&& ( false == IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) || true == IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) ) )
	{
		bReturn = true;
	}
	else if( 0 == _tcscmp( pID, _T("PM_REDDCY_TECBOX") ) 
		&& ( true == IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) && false == IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) )
		&& ( false == IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) || true == IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp ) ) )
	{
		bReturn = true;
	}
	else if( 0 == _tcscmp( pID, _T("PM_REDDCY_ALL") ) 
		&& ( true == IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) && false == IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) )
		&& ( true == IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) && false == IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp ) ) )
	{
		bReturn = true;
	}

	return bReturn;
}

bool CPMWQPrefs::IsChecked( PMWQPreference ePMWQPreference )
{
	return m_arPMWQPrefState[ePMWQPreference].m_bChecked;
}

bool CPMWQPrefs::IsDisabled( PMWQPreference ePMWQPreference )
{
	return m_arPMWQPrefState[ePMWQPreference].m_bDisabled;
}

#define CPMPMWQSELECTIONPREFERENCES_VERSION		5
// Version 5: 2019-01-22 - HYS-984: the 'DegassingEcoAutomatic' and 'DegassingVacusplit' enums are no more needed
//                                  and 'DegassingCoolingVersion' is renamed into 'PumpDegassingCoolingVersion'.
// Version 4: 2019-01-21 - HYS-866: the 'TransferoIntegratedBufferVessel' enum is no more needed.
// Version 3: 'm_strGeneralRendundancyID' replaced by 2 bool variables.
// Version 2: 'm_fTransferoIntegratedBufferVessel' added.
void CPMWQPrefs::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPMPMWQSELECTIONPREFERENCES_VERSION;
	outf.write( (char *)&Version, sizeof( Version ) );

	outf.write( (char *)&m_arPMWQPrefState[GeneralCombinedInOneDevice].m_bChecked, sizeof(bool) );

	// From version 3, 'm_strGeneralRendundancyID' is no more exist.
	WriteString( outf, _T("") );

	outf.write( (char *)&m_arPMWQPrefState[CompressoInternalCoating].m_bChecked, sizeof(bool) );
	outf.write( (char *)&m_arPMWQPrefState[CompressoExternalAir].m_bChecked, sizeof(bool) );
	outf.write( (char *)&m_arPMWQPrefState[PumpDegassingCoolingVersion].m_bChecked, sizeof(bool) );
	outf.write( (char *)&m_arPMWQPrefState[WaterMakeupBreakTank].m_bChecked, sizeof(bool) );
	outf.write( (char *)&m_arPMWQPrefState[WaterMakeupDutyStandBy].m_bChecked, sizeof(bool) );

	// Version 3.
	outf.write( (char *)&m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bChecked, sizeof(bool) );
	outf.write( (char *)&m_arPMWQPrefState[GeneralRedundancyTecBox].m_bChecked, sizeof(bool) );
}

bool CPMWQPrefs::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPMPMWQSELECTIONPREFERENCES_VERSION;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CPMPMWQSELECTIONPREFERENCES_VERSION )
	{
		return false;
	}

	inpf.read( (char *)&m_arPMWQPrefState[GeneralCombinedInOneDevice].m_bChecked, sizeof(bool) );

	CString strGeneralRedundancyID;

	if( false == ReadString( inpf, strGeneralRedundancyID ) )
	{
		return false;
	}

	if( Version < 3 )
	{
		if( 0 == strGeneralRedundancyID.Compare( _T("PM_REDDCY_NONE") ) )
		{
			m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bChecked = false;
			m_arPMWQPrefState[GeneralRedundancyTecBox].m_bChecked = false;
		}
		else if( 0 == strGeneralRedundancyID.Compare( _T("PM_REDDCY_PUMPCOMP") ) )
		{
			m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bChecked = true;
			m_arPMWQPrefState[GeneralRedundancyTecBox].m_bChecked = false;
		}
		else if( 0 == strGeneralRedundancyID.Compare( _T("PM_REDDCY_TECBOX") ) )
		{
			m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bChecked = true;
			m_arPMWQPrefState[GeneralRedundancyTecBox].m_bChecked = false;
		}
	}

	inpf.read( (char *)&m_arPMWQPrefState[CompressoInternalCoating].m_bChecked, sizeof(bool) );
	inpf.read( (char *)&m_arPMWQPrefState[CompressoExternalAir].m_bChecked, sizeof(bool) );

	if( Version < 5 )
	{
		// HYS-984: the 'DegassingEcoAutomatic' and 'DegassingVacusplit' enums are no more needed
		//         and 'DegassingCoolingVersion' is renamed into 'PumpDegassingCoolingVersion'.

		bool bDummy;
		inpf.read( (char *)&bDummy, sizeof(bool) );		// DegassingCoolingVersion
		inpf.read( (char *)&bDummy, sizeof(bool) );		// DegassingEcoAutomatic
		inpf.read( (char *)&bDummy, sizeof(bool) );		// DegassingVacusplit
	}
	else
	{
		inpf.read( (char *)&m_arPMWQPrefState[PumpDegassingCoolingVersion].m_bChecked, sizeof(bool) );
	}
	
	inpf.read( (char *)&m_arPMWQPrefState[WaterMakeupBreakTank].m_bChecked, sizeof(bool) );
	inpf.read( (char *)&m_arPMWQPrefState[WaterMakeupDutyStandBy].m_bChecked, sizeof(bool) );

	if( Version < 2 )
	{
		return true;
	}

	if( Version < 4 )
	{
		// HYS-866: No more need of the 'TransferoIntegratedBufferVessel' variable.
		bool bDummy;
		inpf.read( (char *)&bDummy, sizeof(bool) );
	}

	if( Version < 3 )
	{
		return true;
	}
	
	inpf.read( (char *)&m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bChecked, sizeof(bool) );
	inpf.read( (char *)&m_arPMWQPrefState[GeneralRedundancyTecBox].m_bChecked, sizeof(bool) );

	return true;
}

void CPMWQPrefs::WriteToRegistry( CString strSectionName )
{
	// HYS-1537 : Save Heating preferences
	CSelectPMList* pclHeatingPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Heating );
	bool bCheckedHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralCombinedInOneDevice].m_bChecked;
	bool bDisabledHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralCombinedInOneDevice].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_GeneralCombinedInOneDeviceChecked_H" ), ( true == bCheckedHeating ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_GeneralCombinedInOneDeviceDisabled_H" ), ( true == bDisabledHeating ) ? 1 : 0 );
	bCheckedHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bChecked;
	bDisabledHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyPumpCompChecked_H" ), ( true == bCheckedHeating ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyPumpCompDisabled_H" ), ( true == bDisabledHeating ) ? 1 : 0 );
	bCheckedHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyTecBox].m_bChecked;
	bDisabledHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyTecBox].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyTecBoxChecked_H" ), ( true == bCheckedHeating ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyTecBoxDisabled_H" ), ( true == bDisabledHeating ) ? 1 : 0 );
	bCheckedHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoInternalCoating].m_bChecked;
	bDisabledHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoInternalCoating].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_CompressoInternalCoatingChecked_H" ), ( true == bCheckedHeating ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_CompressoInternalCoatingDisabled_H" ), ( true == bDisabledHeating ) ? 1 : 0 );
	bCheckedHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoExternalAir].m_bChecked;
	bDisabledHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoExternalAir].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_CompressoExternalAirChecked_H" ), ( true == bCheckedHeating ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_CompressoExternalAirDisabled_H" ), ( true == bDisabledHeating ) ? 1 : 0 );
	bCheckedHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[PumpDegassingCoolingVersion].m_bChecked;
	bDisabledHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[PumpDegassingCoolingVersion].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_PumpDegassingCoolingVersionChecked_H" ), ( true == bCheckedHeating ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_PumpDegassingCoolingVersionDisabled_H" ), ( true == bDisabledHeating ) ? 1 : 0 );
	bCheckedHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupBreakTank].m_bChecked;
	bDisabledHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupBreakTank].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupBreakTankChecked_H" ), ( true == bCheckedHeating ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupBreakTankDisabled_H" ), ( true == bDisabledHeating ) ? 1 : 0 );
	bCheckedHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupDutyStandBy].m_bChecked;
	bDisabledHeating = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupDutyStandBy].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupDutyStandByChecked_H" ), ( true == bCheckedHeating ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupDutyStandByDisabled_H" ), ( true == bDisabledHeating ) ? 1 : 0 );

	// Save Cooling preferences
	CSelectPMList* pclCoolingPMList =  pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Cooling );
	bool bCheckedCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralCombinedInOneDevice].m_bChecked;
	bool bDisabledCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralCombinedInOneDevice].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralCombinedInOneDeviceChecked_C"), ( true == bCheckedCooling ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralCombinedInOneDeviceDisabled_C"), ( true == bDisabledCooling ) ? 1 : 0 );
	bCheckedCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bChecked;
	bDisabledCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralRedundancyPumpCompChecked_C"), ( true == bCheckedCooling ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralRedundancyPumpCompDisabled_C"), ( true == bDisabledCooling ) ? 1 : 0 );
	bCheckedCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyTecBox].m_bChecked;
	bDisabledCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyTecBox].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralRedundancyTecBoxChecked_C"), ( true == bCheckedCooling ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralRedundancyTecBoxDisabled_C"), ( true == bDisabledCooling ) ? 1 : 0 );
	bCheckedCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoInternalCoating].m_bChecked;
	bDisabledCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoInternalCoating].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_CompressoInternalCoatingChecked_C"), ( true == bCheckedCooling ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_CompressoInternalCoatingDisabled_C"), ( true == bDisabledCooling ) ? 1 : 0 );
	bCheckedCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoExternalAir].m_bChecked;
	bDisabledCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoExternalAir].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_CompressoExternalAirChecked_C"), ( true == bCheckedCooling ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_CompressoExternalAirDisabled_C"), ( true == bDisabledCooling ) ? 1 : 0 );
	bCheckedCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[PumpDegassingCoolingVersion].m_bChecked;
	bDisabledCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[PumpDegassingCoolingVersion].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_PumpDegassingCoolingVersionChecked_C"), ( true == bCheckedCooling ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_PumpDegassingCoolingVersionDisabled_C"), ( true == bDisabledCooling ) ? 1 : 0 );
	bCheckedCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupBreakTank].m_bChecked;
	bDisabledCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupBreakTank].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_WaterMakeupBreakTankChecked_C"), ( true == bCheckedCooling ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_WaterMakeupBreakTankDisabled_C"), ( true == bDisabledCooling ) ? 1 : 0 );
	bCheckedCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupDutyStandBy].m_bChecked;
	bDisabledCooling = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupDutyStandBy].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_WaterMakeupDutyStandByChecked_C"), ( true == bCheckedCooling ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_WaterMakeupDutyStandByDisabled_C"), ( true == bDisabledCooling ) ? 1 : 0 );

	// Save Solar preferences
	CSelectPMList* pclSolarPMList =  pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Solar );
	bool bCheckedSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralCombinedInOneDevice].m_bChecked;
	bool bDisabledSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralCombinedInOneDevice].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralCombinedInOneDeviceChecked_S"), ( true == bCheckedSolar ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralCombinedInOneDeviceDisabled_S"), ( true == bDisabledSolar ) ? 1 : 0 );
	bCheckedSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bChecked;
	bDisabledSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyPumpComp].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralRedundancyPumpCompChecked_S"), ( true == bCheckedSolar ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralRedundancyPumpCompDisabled_S"), ( true == bDisabledSolar ) ? 1 : 0 );
	bCheckedSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyTecBox].m_bChecked;
	bDisabledSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[GeneralRedundancyTecBox].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralRedundancyTecBoxChecked_S"), ( true == bCheckedSolar ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_GeneralRedundancyTecBoxDisabled_S"), ( true == bDisabledSolar ) ? 1 : 0 );
	bCheckedSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoInternalCoating].m_bChecked;
	bDisabledSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoInternalCoating].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_CompressoInternalCoatingChecked_S"), ( true == bCheckedSolar ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_CompressoInternalCoatingDisabled_S"), ( true == bDisabledSolar ) ? 1 : 0 );
	bCheckedSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoExternalAir].m_bChecked;
	bDisabledSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[CompressoExternalAir].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_CompressoExternalAirChecked_S"), ( true == bCheckedSolar ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_CompressoExternalAirDisabled_S"), ( true == bDisabledSolar ) ? 1 : 0 );
	bCheckedSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[PumpDegassingCoolingVersion].m_bChecked;
	bDisabledSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[PumpDegassingCoolingVersion].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_PumpDegassingCoolingVersionChecked_S"), ( true == bCheckedSolar ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_PumpDegassingCoolingVersionDisabled_S"), ( true == bDisabledSolar ) ? 1 : 0 );
	bCheckedSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupBreakTank].m_bChecked;
	bDisabledSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupBreakTank].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_WaterMakeupBreakTankChecked_S"), ( true == bCheckedSolar ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_WaterMakeupBreakTankDisabled_S"), ( true == bDisabledSolar ) ? 1 : 0 );
	bCheckedSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupDutyStandBy].m_bChecked;
	bDisabledSolar = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->m_arPMWQPrefState[WaterMakeupDutyStandBy].m_bDisabled;
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_WaterMakeupDutyStandByChecked_S"), ( true == bCheckedSolar ) ? 1 : 0 );
	::AfxGetApp()->WriteProfileInt( strSectionName, _T("PMWQPrefs_WaterMakeupDutyStandByDisabled_S"), ( true == bDisabledSolar ) ? 1 : 0 );
}

void CPMWQPrefs::ReadFromRegistry( CString strSectionName )
{
	if( NULL != pDlgLeftTabSelManager )
	{
		// Read Heating preferences saved
		CSelectPMList* pclHeatingPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Heating );
		CPMWQPrefs* pclHeatingPrefs = pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		int iHeatingChecked = 0;
		int iHeatingDisabled = 0;
		iHeatingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralCombinedInOneDeviceChecked_H" ), iHeatingChecked );
		pclHeatingPrefs->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, ( 1 == iHeatingChecked ) ? true : false );
		iHeatingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralCombinedInOneDeviceDisabled_H" ), iHeatingDisabled );
		pclHeatingPrefs->SetDisabled( CPMWQPrefs::GeneralCombinedInOneDevice, ( 1 == iHeatingDisabled ) ? true : false );
		iHeatingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyPumpCompChecked_H" ), iHeatingChecked );
		pclHeatingPrefs->SetChecked( CPMWQPrefs::GeneralRedundancyPumpComp, ( 1 == iHeatingChecked ) ? true : false );
		iHeatingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyPumpCompDisabled_H" ), iHeatingDisabled );
		pclHeatingPrefs->SetDisabled( CPMWQPrefs::GeneralRedundancyPumpComp, ( 1 == iHeatingDisabled ) ? true : false );
		iHeatingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyTecBoxChecked_H" ), iHeatingChecked );
		pclHeatingPrefs->SetChecked( CPMWQPrefs::GeneralRedundancyTecBox, ( 1 == iHeatingChecked ) ? true : false );
		iHeatingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyTecBoxDisabled_H" ), iHeatingDisabled );
		pclHeatingPrefs->SetDisabled( CPMWQPrefs::GeneralRedundancyTecBox, ( 1 == iHeatingDisabled ) ? true : false );
		iHeatingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoInternalCoatingChecked_H" ), iHeatingChecked );
		pclHeatingPrefs->SetChecked( CPMWQPrefs::CompressoInternalCoating, ( 1 == iHeatingChecked ) ? true : false );
		iHeatingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoInternalCoatingDisabled_H" ), iHeatingDisabled );
		pclHeatingPrefs->SetDisabled( CPMWQPrefs::CompressoInternalCoating, ( 1 == iHeatingDisabled ) ? true : false );
		iHeatingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoExternalAirChecked_H" ), iHeatingChecked );
		pclHeatingPrefs->SetChecked( CPMWQPrefs::CompressoExternalAir, ( 1 == iHeatingChecked ) ? true : false );
		iHeatingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoExternalAirDisabled_H" ), iHeatingDisabled );
		pclHeatingPrefs->SetDisabled( CPMWQPrefs::CompressoExternalAir, ( 1 == iHeatingDisabled ) ? true : false );
		iHeatingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_PumpDegassingCoolingVersionChecked_H" ), iHeatingChecked );
		pclHeatingPrefs->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, ( 1 == iHeatingChecked ) ? true : false );
		iHeatingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_PumpDegassingCoolingVersionDisabled_H" ), iHeatingDisabled );
		pclHeatingPrefs->SetDisabled( CPMWQPrefs::PumpDegassingCoolingVersion, ( 1 == iHeatingDisabled ) ? true : false );
		iHeatingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupBreakTankChecked_H" ), iHeatingChecked );
		pclHeatingPrefs->SetChecked( CPMWQPrefs::WaterMakeupBreakTank, ( 1 == iHeatingChecked ) ? true : false );
		iHeatingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupBreakTankDisabled_H" ), iHeatingDisabled );
		pclHeatingPrefs->SetDisabled( CPMWQPrefs::WaterMakeupBreakTank, ( 1 == iHeatingDisabled ) ? true : false );
		iHeatingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupDutyStandByChecked_H" ), iHeatingChecked );
		pclHeatingPrefs->SetChecked( CPMWQPrefs::WaterMakeupDutyStandBy, ( 1 == iHeatingChecked ) ? true : false );
		iHeatingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupDutyStandByDisabled_H" ), iHeatingDisabled );
		pclHeatingPrefs->SetDisabled( CPMWQPrefs::WaterMakeupDutyStandBy, ( 1 == iHeatingDisabled ) ? true : false );

		// Read Cooling preferences saved
		CSelectPMList* pclCoolingPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Cooling );
		CPMWQPrefs* pclCoolingPrefs = pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		int iCoolingChecked = 0;
		int iCoolingDisabled = 0;
		iCoolingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralCombinedInOneDeviceChecked_C" ), iCoolingChecked );
		pclCoolingPrefs->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, ( 1 == iCoolingChecked ) ? true : false );
		iCoolingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralCombinedInOneDeviceDisabled_C" ), iCoolingDisabled );
		pclCoolingPrefs->SetDisabled( CPMWQPrefs::GeneralCombinedInOneDevice, ( 1 == iCoolingDisabled ) ? true : false );
		iCoolingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyPumpCompChecked_C" ), iCoolingChecked );
		pclCoolingPrefs->SetChecked( CPMWQPrefs::GeneralRedundancyPumpComp, ( 1 == iCoolingChecked ) ? true : false );
		iCoolingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyPumpCompDisabled_C" ), iCoolingDisabled );
		pclCoolingPrefs->SetDisabled( CPMWQPrefs::GeneralRedundancyPumpComp, ( 1 == iCoolingDisabled ) ? true : false );
		iCoolingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyTecBoxChecked_C" ), iCoolingChecked );
		pclCoolingPrefs->SetChecked( CPMWQPrefs::GeneralRedundancyTecBox, ( 1 == iCoolingChecked ) ? true : false );
		iCoolingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyTecBoxDisabled_C" ), iCoolingDisabled );
		pclCoolingPrefs->SetDisabled( CPMWQPrefs::GeneralRedundancyTecBox, ( 1 == iCoolingDisabled ) ? true : false );
		iCoolingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoInternalCoatingChecked_C" ), iCoolingChecked );
		pclCoolingPrefs->SetChecked( CPMWQPrefs::CompressoInternalCoating, ( 1 == iCoolingChecked ) ? true : false );
		iCoolingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoInternalCoatingDisabled_C" ), iCoolingDisabled );
		pclCoolingPrefs->SetDisabled( CPMWQPrefs::CompressoInternalCoating, ( 1 == iCoolingDisabled ) ? true : false );
		iCoolingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoExternalAirChecked_C" ), iCoolingChecked );
		pclCoolingPrefs->SetChecked( CPMWQPrefs::CompressoExternalAir, ( 1 == iCoolingChecked ) ? true : false );
		iCoolingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoExternalAirDisabled_C" ), iCoolingDisabled );
		pclCoolingPrefs->SetDisabled( CPMWQPrefs::CompressoExternalAir, ( 1 == iCoolingDisabled ) ? true : false );
		iCoolingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_PumpDegassingCoolingVersionChecked_C" ), iCoolingChecked );
		pclCoolingPrefs->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, ( 1 == iCoolingChecked ) ? true : false );
		iCoolingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_PumpDegassingCoolingVersionDisabled_C" ), iCoolingDisabled );
		pclCoolingPrefs->SetDisabled( CPMWQPrefs::PumpDegassingCoolingVersion, ( 1 == iCoolingDisabled ) ? true : false );
		iCoolingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupBreakTankChecked_C" ), iCoolingChecked );
		pclCoolingPrefs->SetChecked( CPMWQPrefs::WaterMakeupBreakTank, ( 1 == iCoolingChecked ) ? true : false );
		iCoolingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupBreakTankDisabled_C" ), iCoolingDisabled );
		pclCoolingPrefs->SetDisabled( CPMWQPrefs::WaterMakeupBreakTank, ( 1 == iCoolingDisabled ) ? true : false );
		iCoolingChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupDutyStandByChecked_C" ), iCoolingChecked );
		pclCoolingPrefs->SetChecked( CPMWQPrefs::WaterMakeupDutyStandBy, ( 1 == iCoolingChecked ) ? true : false );
		iCoolingDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupDutyStandByDisabled_C" ), iCoolingDisabled );
		pclCoolingPrefs->SetDisabled( CPMWQPrefs::WaterMakeupDutyStandBy, ( 1 == iCoolingDisabled ) ? true : false );

		// Read Solar preferences saved
		CSelectPMList* pclSolarPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Solar );
		CPMWQPrefs* pclSolarPrefs = pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		int iSolarChecked = 0;
		int iSolarDisabled = 0;
		iSolarChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralCombinedInOneDeviceChecked_S" ), iSolarChecked );
		pclSolarPrefs->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, ( 1 == iSolarChecked ) ? true : false );
		iSolarDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralCombinedInOneDeviceDisabled_S" ), iSolarDisabled );
		pclSolarPrefs->SetDisabled( CPMWQPrefs::GeneralCombinedInOneDevice, ( 1 == iSolarDisabled ) ? true : false );
		iSolarChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyPumpCompChecked_S" ), iSolarChecked );
		pclSolarPrefs->SetChecked( CPMWQPrefs::GeneralRedundancyPumpComp, ( 1 == iSolarChecked ) ? true : false );
		iSolarDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyPumpCompDisabled_S" ), iSolarDisabled );
		pclSolarPrefs->SetDisabled( CPMWQPrefs::GeneralRedundancyPumpComp, ( 1 == iSolarDisabled ) ? true : false );
		iSolarChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyTecBoxChecked_S" ), iSolarChecked );
		pclSolarPrefs->SetChecked( CPMWQPrefs::GeneralRedundancyTecBox, ( 1 == iSolarChecked ) ? true : false );
		iSolarDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_GeneralRedundancyTecBoxDisabled_S" ), iSolarDisabled );
		pclSolarPrefs->SetDisabled( CPMWQPrefs::GeneralRedundancyTecBox, ( 1 == iSolarDisabled ) ? true : false );
		iSolarChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoInternalCoatingChecked_S" ), iSolarChecked );
		pclSolarPrefs->SetChecked( CPMWQPrefs::CompressoInternalCoating, ( 1 == iSolarChecked ) ? true : false );
		iSolarDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoInternalCoatingDisabled_S" ), iSolarDisabled );
		pclSolarPrefs->SetDisabled( CPMWQPrefs::CompressoInternalCoating, ( 1 == iSolarDisabled ) ? true : false );
		iSolarChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoExternalAirChecked_S" ), iSolarChecked );
		pclSolarPrefs->SetChecked( CPMWQPrefs::CompressoExternalAir, ( 1 == iSolarChecked ) ? true : false );
		iSolarDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_CompressoExternalAirDisabled_S" ), iSolarDisabled );
		pclSolarPrefs->SetDisabled( CPMWQPrefs::CompressoExternalAir, ( 1 == iSolarDisabled ) ? true : false );
		iSolarChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_PumpDegassingCoolingVersionChecked_S" ), iSolarChecked );
		pclSolarPrefs->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, ( 1 == iSolarChecked ) ? true : false );
		iSolarDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_PumpDegassingCoolingVersionDisabled_S" ), iSolarDisabled );
		pclSolarPrefs->SetDisabled( CPMWQPrefs::PumpDegassingCoolingVersion, ( 1 == iSolarDisabled ) ? true : false );
		iSolarChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupBreakTankChecked_S" ), iSolarChecked );
		pclSolarPrefs->SetChecked( CPMWQPrefs::WaterMakeupBreakTank, ( 1 == iSolarChecked ) ? true : false );
		iSolarDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupBreakTankDisabled_S" ), iSolarDisabled );
		pclSolarPrefs->SetDisabled( CPMWQPrefs::WaterMakeupBreakTank, ( 1 == iSolarDisabled ) ? true : false );
		iSolarChecked = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupDutyStandByChecked_S" ), iSolarChecked );
		pclSolarPrefs->SetChecked( CPMWQPrefs::WaterMakeupDutyStandBy, ( 1 == iSolarChecked ) ? true : false );
		iSolarDisabled = ( ProjectType )::AfxGetApp()->GetProfileInt( strSectionName, _T( "PMWQPrefs_WaterMakeupDutyStandByDisabled_S" ), iSolarDisabled );
		pclSolarPrefs->SetDisabled( CPMWQPrefs::WaterMakeupDutyStandBy, ( 1 == iSolarDisabled ) ? true : false );

		// Copy current prefs in CDS_Datastruct::CPMWQPrefs
		if( TASApp.GetpTADS()->GetpIndSelParameter()->GetPMApplicationType() == Heating )
		{
			CopyFrom( pclHeatingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() );
		}
		else if( TASApp.GetpTADS()->GetpIndSelParameter()->GetPMApplicationType() == Cooling )
		{
			CopyFrom( pclCoolingPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() );
		}
		else if( TASApp.GetpTADS()->GetpIndSelParameter()->GetPMApplicationType() == Solar )
		{
			CopyFrom( pclSolarPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CPMWQPrefsValidation.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPMWQPrefsValidation::CPMWQPrefsValidation()
{
	m_mapTecBoxEntries[CDB_TecBox::etbtCompresso] = IV_Compresso;
	m_mapTecBoxEntries[CDB_TecBox::etbtTransfero] = IV_Transfero;
	m_mapTecBoxEntries[CDB_TecBox::etbtTransferoTI] = IV_TransferoTI;
	m_mapTecBoxEntries[CDB_TecBox::etbtVento] = IV_Vento;
	m_mapTecBoxEntries[CDB_TecBox::etbtPleno] = IV_Pleno;

	m_mapVesselEntries[CDB_Vessel::eVsslType_Statico] = IV_Statico;
	m_mapVesselEntries[CDB_Vessel::eVsslType_StatMbr] = IV_StaticoMembrane;

	m_vecFunctions.push_back( DWFCT_None );
	m_vecFunctions.push_back( DWFCT_Degassing );
	m_vecFunctions.push_back( DWFCT_WaterMakeUpStd );
	m_vecFunctions.push_back( DWFCT_WaterMakeUpSoftDesal );
	m_vecFunctions.push_back( DWFCT_Degassing | DWFCT_WaterMakeUpStd );
	m_vecFunctions.push_back( DWFCT_Degassing | DWFCT_WaterMakeUpSoftDesal );

	m_bInitialiazed = false;
}

void CPMWQPrefsValidation::Init()
{
	if( true == m_bInitialiazed )
	{
		return;
	}

	PMWQPrefArray rPrefArray;
	ZeroMemory( &rPrefArray, sizeof( PMWQPrefArray ) );

	mapCombinedPreferences mapCombPrefs;

	for( int iLoop = 0; iLoop < (int)m_vecFunctions.size(); iLoop++ )
	{
		mapCombPrefs[iLoop] = rPrefArray;
	}

	for( int iLoop = IV_First; iLoop <= IV_Last; iLoop++ )
	{
		m_vecValidateOptions.push_back( mapCombPrefs );
	}

	m_bInitialiazed = true;
}

void CPMWQPrefsValidation::CopyFrom( CPMWQPrefsValidation *pclPMWQPrefsValidation )
{
	if( NULL == pclPMWQPrefsValidation )
	{
		return;
	}

	m_vecValidateOptions.clear();
	m_vecValidateOptions = *( pclPMWQPrefsValidation->GetVecValidateOptions() );
}

bool CPMWQPrefsValidation::IsAtLeastOneProductExist( IndexVector eIndexVector, int iFunction )
{
	if( IV_Undefined == eIndexVector )
	{
		return false;
	}

	if( 0 == m_vecValidateOptions[eIndexVector].count( iFunction ) )
	{
		ASSERT( 0 );
		return false;
	}

	return m_vecValidateOptions[eIndexVector][iFunction].m_bAtLeastOneExist;
}

bool CPMWQPrefsValidation::IsAtLeastOneTecBoxHasMultiFct( CDB_TecBox::TecBoxType eTecBox )
{
	if( 0 == m_mapTecBoxEntries.count( eTecBox ) )
	{
		return false;
	}

	IndexVector eIV = m_mapTecBoxEntries[eTecBox];
	bool bMultiFct = false;

	for( int iLoop = 0; iLoop < (int)m_vecFunctions.size(); iLoop++ )
	{
		int iFunction = m_vecFunctions[iLoop];

		if( 0 != m_vecValidateOptions[eIV].count( iFunction ) && true == m_vecValidateOptions[eIV][iFunction].m_arPMWQPrefs[CPMWQPrefs::GeneralCombinedInOneDevice] )
		{
			bMultiFct = true;
			break;
		}
	}

	return bMultiFct;
}

void CPMWQPrefsValidation::SetValidateOptionFlag( CDB_Product *pclPMProduct, int iFunction, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet )
{
	SetValidateOptionFlag( _GetIndexVector( pclPMProduct ), iFunction, ePMWQPref, bSet );
}

void CPMWQPrefsValidation::SetValidateOptionFlag( CDB_Product *pclPMProduct, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet )
{
	SetValidateOptionFlag( _GetIndexVector( pclPMProduct ), ePMWQPref, bSet );
}

void CPMWQPrefsValidation::SetValidateOptionFlag( IndexVector eIndexVector, int iFunction, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet )
{
	if( IV_Undefined == eIndexVector )
	{
		return;
	}

	if( 0 == m_vecValidateOptions[eIndexVector].count( iFunction ) )
	{
		ASSERT_RETURN;
	}

	m_vecValidateOptions[eIndexVector][iFunction].m_arPMWQPrefs[ePMWQPref] = bSet;
	m_vecValidateOptions[eIndexVector][iFunction].m_bAtLeastOneExist = true;
}

void CPMWQPrefsValidation::SetValidateOptionFlag( IndexVector eIndexVector, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet )
{
	for( int iLoop = 0; iLoop < (int)m_vecFunctions.size() ; iLoop++ )
	{
		SetValidateOptionFlag( eIndexVector, m_vecFunctions[iLoop], ePMWQPref, bSet );
	}
}

void CPMWQPrefsValidation::PopulateValidateOptionFlag( IndexVector eIndexVector, CPMWQPrefs::PMWQPreference ePMWQPref )
{
	if( IV_Undefined == eIndexVector )
	{
		return;
	}

	// We don't populate on the first combination.
	for( int iLoop = 1; iLoop < (int)m_vecFunctions.size(); iLoop++ )
	{
		int iFunction = m_vecFunctions[iLoop];

		if( 0 != m_vecValidateOptions[eIndexVector].count( iFunction ) && false == m_vecValidateOptions[eIndexVector][iFunction].m_arPMWQPrefs[CPMWQPrefs::GeneralCombinedInOneDevice] )
		{
			m_vecValidateOptions[eIndexVector][iFunction].m_arPMWQPrefs[ePMWQPref] = true;
		}
	}
}

bool CPMWQPrefsValidation::GetValidateOptionFlag( CDB_Product *pclPMProduct, int iFunctions, CPMWQPrefs::PMWQPreference ePMWQPref )
{
	return GetValidateOptionFlag( _GetIndexVector( pclPMProduct ), iFunctions, ePMWQPref );
}

bool CPMWQPrefsValidation::GetValidateOptionFlag( IndexVector eIndexVector, int iFunctions, CPMWQPrefs::PMWQPreference ePMWQPref )
{
	if( IV_Undefined == eIndexVector )
	{
		return false;
	}

	if( 0 == m_vecValidateOptions[eIndexVector].count( iFunctions ) )
	{
		return false;
	}

	return m_vecValidateOptions[eIndexVector][iFunctions].m_arPMWQPrefs[ePMWQPref];
}

CPMWQPrefsValidation::IndexVector CPMWQPrefsValidation::_GetIndexVector( CDB_Product *pclPMProduct )
{
	if( NULL == dynamic_cast<CDB_Vessel*>( pclPMProduct ) && NULL == dynamic_cast<CDB_TecBox*>( pclPMProduct ) )
	{
		return IV_Undefined;
	}

	IndexVector eIV;

	if( NULL != dynamic_cast<CDB_Vessel*>( pclPMProduct ) )
	{
		CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel*>( pclPMProduct );
		
		if( 0 == m_mapVesselEntries.count( pclVessel->GetVesselType() ) )
		{
			return IV_Undefined;
		}

		eIV = m_mapVesselEntries[pclVessel->GetVesselType()];
	}
	else
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox*>( pclPMProduct );
		
		if( 0 == m_mapTecBoxEntries.count( pclTecBox->GetTecBoxType() ) )
		{
			return IV_Undefined;
		}

		eIV = m_mapTecBoxEntries[pclTecBox->GetTecBoxType()];
	}

	return eIV;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CHeatGeneratorInput.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHeatGeneratorList::CompareTo( CHeatGeneratorList *pclHeatGeneratorInput )
{
	if( NULL == pclHeatGeneratorInput )
	{
		return false;
	}

	if( m_dTotalHeatCapacity != pclHeatGeneratorInput->GetTotalHeatCapacity() )
	{
		return false;
	}

	if( m_dTotalContent != pclHeatGeneratorInput->GetTotalContent() )
	{
		return false;
	}

	if( m_bHeatGeneratorProtect != pclHeatGeneratorInput->GetHeatHeneratorProtect() )
	{
		return false;
	}
	
	vecHeatGeneratorList *pclHeatGeneratorList = pclHeatGeneratorInput->GetHeatGeneratorList();

	if( m_vecHeatGeneratorList.size() != pclHeatGeneratorList->size() )
	{
		return false;
	}

	for( int iLoop = 0; iLoop < (int)m_vecHeatGeneratorList.size(); iLoop++ )
	{
		if( 0 != m_vecHeatGeneratorList[iLoop].m_strHeatGeneratorTypeID.Compare( pclHeatGeneratorList->at( iLoop ).m_strHeatGeneratorTypeID ) )
		{
			return false;
		}

		if( m_vecHeatGeneratorList[iLoop].m_dHeatCapacity != pclHeatGeneratorList->at( iLoop ).m_dHeatCapacity )
		{
			return false;
		}

		if( m_vecHeatGeneratorList[iLoop].m_dContent != pclHeatGeneratorList->at( iLoop ).m_dContent )
		{
			return false;
		}

		if( m_vecHeatGeneratorList[iLoop].m_bManual != pclHeatGeneratorList->at( iLoop ).m_bManual )
		{
			return false;
		}
	}
	
	return true;
}

void CHeatGeneratorList::CopyFrom( CHeatGeneratorList *pclHeatGeneratorInput )
{
	if( NULL == pclHeatGeneratorInput )
	{
		return;
	}

	Reset();
	m_dTotalHeatCapacity = pclHeatGeneratorInput->GetTotalHeatCapacity();
	m_dTotalContent = pclHeatGeneratorInput->GetTotalContent();
	m_bHeatGeneratorProtect = pclHeatGeneratorInput->GetHeatHeneratorProtect();
	m_vecHeatGeneratorList = *( pclHeatGeneratorInput->GetHeatGeneratorList() );
}

#define CPHEATGENERATORLIST_VERSION		1
void CHeatGeneratorList::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPHEATGENERATORLIST_VERSION;
	outf.write( (char *)&Version, sizeof( Version ) );

	outf.write( (char *)&m_dTotalHeatCapacity, sizeof( m_dTotalHeatCapacity ) );
	outf.write( (char *)&m_dTotalContent, sizeof( m_dTotalContent ) );
	outf.write( (char *)&m_bHeatGeneratorProtect, sizeof( m_bHeatGeneratorProtect ) );

	int iSize = m_vecHeatGeneratorList.size();
	outf.write( (char *)&iSize, sizeof( iSize ) );

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			WriteString( outf, m_vecHeatGeneratorList[i].m_strHeatGeneratorTypeID );
			outf.write( (char *)&m_vecHeatGeneratorList[i].m_dHeatCapacity, sizeof( m_vecHeatGeneratorList[i].m_dHeatCapacity ) );
			outf.write( (char *)&m_vecHeatGeneratorList[i].m_dContent, sizeof( m_vecHeatGeneratorList[i].m_dContent ) );
			outf.write( (char *)&m_vecHeatGeneratorList[i].m_bManual, sizeof( m_vecHeatGeneratorList[i].m_bManual ) );
		}
	}
}

bool CHeatGeneratorList::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPHEATGENERATORLIST_VERSION;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CPHEATGENERATORLIST_VERSION )
	{
		return false;
	}

	inpf.read( (char *)&m_dTotalHeatCapacity, sizeof( m_dTotalHeatCapacity ) );
	inpf.read( (char *)&m_dTotalContent, sizeof( m_dTotalContent ) );
	inpf.read( (char *)&m_bHeatGeneratorProtect, sizeof( m_bHeatGeneratorProtect ) );

	int iSize;
	inpf.read( (char *)&iSize, sizeof( iSize ) );
	m_vecHeatGeneratorList.clear();

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			HeatGeneratorSave rHeatGeneratorSave;
			TCHAR tcID[_ID_LENGTH + 1];

			if( false == ReadString( inpf, tcID, sizeof( tcID ) ) )
			{
				return false;
			}

			rHeatGeneratorSave.m_strHeatGeneratorTypeID = CString( tcID );
			inpf.read( (char *)&rHeatGeneratorSave.m_dHeatCapacity, sizeof( rHeatGeneratorSave.m_dHeatCapacity ) );
			inpf.read( (char *)&rHeatGeneratorSave.m_dContent, sizeof( rHeatGeneratorSave.m_dContent ) );
			inpf.read( (char *)&rHeatGeneratorSave.m_bManual, sizeof( rHeatGeneratorSave.m_bManual ) );
			m_vecHeatGeneratorList.push_back( rHeatGeneratorSave );
		}
	}

	return true;
}

bool CHeatGeneratorList::AddHeatGenerator( CString strHeatGeneratorTypeID, double dHeatCapacity, double dContent )
{
	if( dHeatCapacity < 0.0 )
	{
		return false;
	}

	HeatGeneratorSave rHeatGeneratorSave;
	rHeatGeneratorSave.m_strHeatGeneratorTypeID = strHeatGeneratorTypeID;
	rHeatGeneratorSave.m_dHeatCapacity = dHeatCapacity;
	rHeatGeneratorSave.m_bManual = ( dContent != -1.0 );

	if( -1.0 == dContent )
	{
		dContent = _ComputeWaterContent( strHeatGeneratorTypeID, dHeatCapacity );
	}

	rHeatGeneratorSave.m_dContent = dContent;
	m_vecHeatGeneratorList.push_back( rHeatGeneratorSave );

	m_dTotalHeatCapacity += dHeatCapacity;
	m_dTotalContent += dContent;

	return true;
}

bool CHeatGeneratorList::RemoveHeatGenerator( int iIndex )
{
	if( iIndex >= ( int )m_vecHeatGeneratorList.size() )
	{
		return false;
	}

	if( -1 == iIndex || 0 == m_vecHeatGeneratorList.size() )
	{
		Reset();
	}
	else
	{
		m_vecHeatGeneratorList.erase( m_vecHeatGeneratorList.begin() + iIndex );

		// To avoid rounding error, we recompute total content at each time.
		m_dTotalHeatCapacity = 0.0;
		m_dTotalContent = 0.0;
		HeatGeneratorSave *pclHeatGenerator = GetFirstHeatGenerator();

		while( NULL != pclHeatGenerator )
		{
			m_dTotalHeatCapacity += pclHeatGenerator->m_dHeatCapacity;

			if( true == pclHeatGenerator->m_bManual )
			{
				m_dTotalContent += pclHeatGenerator->m_dContent;
			}
			else
			{
				m_dTotalContent += _ComputeWaterContent( pclHeatGenerator->m_strHeatGeneratorTypeID, pclHeatGenerator->m_dHeatCapacity );
			}

			pclHeatGenerator = GetNextHeatGenerator();
		}
	}

	return true;
}

bool CHeatGeneratorList::UpdateHeatGenerator( int iIndex, CString strHeatGeneratorTypeID, double dHeatCapacity, double dContent )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecHeatGeneratorList.size() )
	{
		return false;
	}

	m_dTotalHeatCapacity -= m_vecHeatGeneratorList[iIndex].m_dHeatCapacity;
	m_dTotalContent -= m_vecHeatGeneratorList[iIndex].m_dContent;

	m_vecHeatGeneratorList[iIndex].m_strHeatGeneratorTypeID = strHeatGeneratorTypeID;
	m_vecHeatGeneratorList[iIndex].m_dHeatCapacity = dHeatCapacity;
	m_vecHeatGeneratorList[iIndex].m_dContent = dContent;
	m_vecHeatGeneratorList[iIndex].m_bManual = ( dContent != -1.0 );

	if( -1.0 == dContent )
	{
		m_vecHeatGeneratorList[iIndex].m_dContent = _ComputeWaterContent( strHeatGeneratorTypeID, dHeatCapacity );
	}

	m_dTotalHeatCapacity += m_vecHeatGeneratorList[iIndex].m_dHeatCapacity;
	m_dTotalContent += m_vecHeatGeneratorList[iIndex].m_dContent;

	return true;
}

CHeatGeneratorList::HeatGeneratorSave *CHeatGeneratorList::GetHeatGenerator( int iIndex )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecHeatGeneratorList.size() )
	{
		return NULL;
	}

	return &m_vecHeatGeneratorList[iIndex];
}

CHeatGeneratorList::HeatGeneratorSave *CHeatGeneratorList::GetFirstHeatGenerator( void )
{
	if( 0 == m_vecHeatGeneratorList.size() )
	{
		return NULL;
	}

	m_vecHeatGeneratorListIter = m_vecHeatGeneratorList.begin();
	return &( *m_vecHeatGeneratorListIter );
}

CHeatGeneratorList::HeatGeneratorSave *CHeatGeneratorList::GetNextHeatGenerator( void )
{
	if( 0 == m_vecHeatGeneratorList.size() )
	{
		return NULL;
	}

	if( m_vecHeatGeneratorList.end() == m_vecHeatGeneratorListIter )
	{
		return NULL;
	}

	m_vecHeatGeneratorListIter++;

	if( m_vecHeatGeneratorList.end() == m_vecHeatGeneratorListIter )
	{
		return NULL;
	}

	return &( *m_vecHeatGeneratorListIter );
}

double CHeatGeneratorList::_ComputeWaterContent( CString strHeatGeneratorTypeID, double dHeatCapacity )
{
	double dWaterContent = dHeatCapacity;

	if( 0 == strHeatGeneratorTypeID.Compare( _T("PM_HG_STEELBOILER") ) )
	{
		dWaterContent *= 2.5;
	}
	else if( 0 == strHeatGeneratorTypeID.Compare( _T("PM_HG_CASTIRBOILER") ) )
	{
		dWaterContent *= 1.5;
	}
	else if( 0 == strHeatGeneratorTypeID.Compare( _T("PM_HG_BOILER") ) )
	{
		dWaterContent *= 0.2;
	}
	else if( 0 == strHeatGeneratorTypeID.Compare( _T("PM_HG_HEATEXCHANGER") ) )
	{
		dWaterContent *= 0.6;
	}
	else if( 0 == strHeatGeneratorTypeID.Compare( _T("PM_HG_CHP") ) )
	{
		dWaterContent *= 0.6;
	}
	else if( 0 == strHeatGeneratorTypeID.Compare( _T("PM_HG_HEATPUMP") ) )
	{
		dWaterContent *= 0.6;
	}
	else
	{
		ASSERTA_RETURN( 0.0 );
	}

	// Remark: factor below are in 'l/kW'.
	return dWaterContent / 1000000.0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CColdGeneratorList.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CColdGeneratorList::CompareTo( CColdGeneratorList *pclColdGeneratorInput )
{
	if( NULL == pclColdGeneratorInput )
	{
		return false;
	}

	if( m_dTotalColdCapacity != pclColdGeneratorInput->GetTotalColdCapacity() )
	{
		return false;
	}

	if( m_dTotalContent != pclColdGeneratorInput->GetTotalContent() )
	{
		return false;
	}

	if( m_dTempOutdoor != pclColdGeneratorInput->GetTempOutdoor() )
	{
		return false;
	}
	
	vecColdGeneratorList *pclColdGeneratorList = pclColdGeneratorInput->GetColdGeneratorList();

	if( m_vecColdGeneratorList.size() != pclColdGeneratorList->size() )
	{
		return false;
	}

	for( int iLoop = 0; iLoop < (int)m_vecColdGeneratorList.size(); iLoop++ )
	{
		if( 0 != m_vecColdGeneratorList[iLoop].m_strColdGeneratorTypeID.Compare( pclColdGeneratorList->at( iLoop ).m_strColdGeneratorTypeID ) )
		{
			return false;
		}

		if( m_vecColdGeneratorList[iLoop].m_dColdCapacity != pclColdGeneratorList->at( iLoop ).m_dColdCapacity )
		{
			return false;
		}

		if( m_vecColdGeneratorList[iLoop].m_dContent != pclColdGeneratorList->at( iLoop ).m_dContent )
		{
			return false;
		}

		if( m_vecColdGeneratorList[iLoop].m_dSupplyTemp != pclColdGeneratorList->at( iLoop ).m_dSupplyTemp )
		{
			return false;
		}

		if( m_vecColdGeneratorList[iLoop].m_bManual != pclColdGeneratorList->at( iLoop ).m_bManual )
		{
			return false;
		}

		if( m_vecColdGeneratorList[iLoop].m_bManualTemp != pclColdGeneratorList->at( iLoop ).m_bManualTemp )
		{
			return false;
		}

		if( m_vecColdGeneratorList[iLoop].m_lQuantity != pclColdGeneratorList->at( iLoop ).m_lQuantity )
		{
			return false;
		}
	}
	
	return true;
}

void CColdGeneratorList::CopyFrom( CColdGeneratorList *pclColdGeneratorInput )
{
	if( NULL == pclColdGeneratorInput )
	{
		return;
	}

	Reset();
	m_dTotalColdCapacity = pclColdGeneratorInput->GetTotalColdCapacity();
	m_dTotalContent = pclColdGeneratorInput->GetTotalContent();
	m_dTempOutdoor = pclColdGeneratorInput->GetTempOutdoor();
	m_vecColdGeneratorList = *( pclColdGeneratorInput->GetColdGeneratorList() );
}

#define CPCOLDGENERATORLIST_VERSION		2
void CColdGeneratorList::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPCOLDGENERATORLIST_VERSION;
	outf.write( (char *)&Version, sizeof( Version ) );

	outf.write( (char *)&m_dTotalColdCapacity, sizeof( m_dTotalColdCapacity ) );
	outf.write( (char *)&m_dTotalContent, sizeof( m_dTotalContent ) );
	outf.write( (char *)&m_dTempOutdoor, sizeof( m_dTempOutdoor ) );

	int iSize = m_vecColdGeneratorList.size();
	outf.write( (char *)&iSize, sizeof( iSize ) );

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			WriteString( outf, m_vecColdGeneratorList[i].m_strColdGeneratorTypeID );
			outf.write( (char *)&m_vecColdGeneratorList[i].m_dColdCapacity, sizeof( m_vecColdGeneratorList[i].m_dColdCapacity ) );
			outf.write( (char *)&m_vecColdGeneratorList[i].m_dContent, sizeof( m_vecColdGeneratorList[i].m_dContent ) );
			outf.write( (char *)&m_vecColdGeneratorList[i].m_bManual, sizeof( m_vecColdGeneratorList[i].m_bManual ) );
			outf.write( (char *)&m_vecColdGeneratorList[i].m_dSupplyTemp, sizeof( m_vecColdGeneratorList[i].m_dSupplyTemp ) );
			outf.write( (char *)&m_vecColdGeneratorList[i].m_bManualTemp, sizeof( m_vecColdGeneratorList[i].m_bManualTemp ) );
			outf.write( (char *)&m_vecColdGeneratorList[i].m_lQuantity, sizeof( m_vecColdGeneratorList[i].m_lQuantity ) );
		}
	}
}

bool CColdGeneratorList::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPCOLDGENERATORLIST_VERSION;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CPCOLDGENERATORLIST_VERSION )
	{
		return false;
	}

	inpf.read( (char *)&m_dTotalColdCapacity, sizeof( m_dTotalColdCapacity ) );
	inpf.read( (char *)&m_dTotalContent, sizeof( m_dTotalContent ) );
	inpf.read( (char *)&m_dTempOutdoor, sizeof( m_dTempOutdoor ) );

	int iSize;
	inpf.read( (char *)&iSize, sizeof( iSize ) );
	m_vecColdGeneratorList.clear();

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			ColdGeneratorSave rColdGeneratorSave;
			TCHAR tcID[_ID_LENGTH + 1];

			if( false == ReadString( inpf, tcID, sizeof( tcID ) ) )
			{
				return false;
			}

			rColdGeneratorSave.m_strColdGeneratorTypeID = CString( tcID );
			inpf.read( (char *)&rColdGeneratorSave.m_dColdCapacity, sizeof( rColdGeneratorSave.m_dColdCapacity ) );
			inpf.read( (char *)&rColdGeneratorSave.m_dContent, sizeof( rColdGeneratorSave.m_dContent ) );
			inpf.read( (char *)&rColdGeneratorSave.m_bManual, sizeof( rColdGeneratorSave.m_bManual ) );
			inpf.read( (char *)&rColdGeneratorSave.m_dSupplyTemp, sizeof( rColdGeneratorSave.m_dSupplyTemp ) );
			inpf.read( (char *)&rColdGeneratorSave.m_bManualTemp, sizeof( rColdGeneratorSave.m_bManualTemp ) );

			if ( Version >= 2 )
			{
				inpf.read((char *)&rColdGeneratorSave.m_lQuantity, sizeof(rColdGeneratorSave.m_lQuantity));
			}
			else
			{
				rColdGeneratorSave.m_lQuantity = 1;
			}

			m_vecColdGeneratorList.push_back( rColdGeneratorSave );
		}
	}

	return true;
}

bool CColdGeneratorList::AddColdGenerator( CString strColdGeneratorTypeID, double dColdCapacity, double dContent, double dSupplyTemp )
{
	if( dColdCapacity < 0.0 )
	{
		return false;
	}

	ColdGeneratorSave rColdGeneratorSave;
	rColdGeneratorSave.m_strColdGeneratorTypeID = strColdGeneratorTypeID;
	rColdGeneratorSave.m_dColdCapacity = dColdCapacity;
	rColdGeneratorSave.m_bManual = ( dContent != -1.0 );
	rColdGeneratorSave.m_bManualTemp = ( dSupplyTemp != -1.0 );
	rColdGeneratorSave.m_dSupplyTemp = dSupplyTemp;
	// HYS-938: open saved System volume corrupt the Total content static field
	rColdGeneratorSave.m_lQuantity = 1;

	if( -1.0 != dContent )
	{
		dContent = _ComputeWaterContent( strColdGeneratorTypeID, dColdCapacity, dSupplyTemp );
	}

	rColdGeneratorSave.m_dContent = dContent;
	m_vecColdGeneratorList.push_back( rColdGeneratorSave );

	m_dTotalColdCapacity += dColdCapacity;
	m_dTotalContent += dContent;

	return true;
}

bool CColdGeneratorList::RemoveColdGenerator( int iIndex )
{
	if( iIndex >= ( int )m_vecColdGeneratorList.size() )
	{
		return false;
	}

	if( -1 == iIndex || m_vecColdGeneratorList.size() == 0 )
	{
		Reset();
	}
	else
	{
		m_vecColdGeneratorList.erase( m_vecColdGeneratorList.begin() + iIndex );

		// To avoid rounding error, we recompute total content at each time.
		m_dTotalColdCapacity = 0.0;
		m_dTotalContent = 0.0;
		ColdGeneratorSave *pclColdGenerator = GetFirstColdGenerator();

		while( NULL != pclColdGenerator )
		{
			m_dTotalColdCapacity += pclColdGenerator->m_dColdCapacity;

			if( true == pclColdGenerator->m_bManual )
			{
				m_dTotalContent += pclColdGenerator->m_dContent;
			}
			else
			{
				m_dTotalContent += _ComputeWaterContent( pclColdGenerator->m_strColdGeneratorTypeID, pclColdGenerator->m_dColdCapacity,
					pclColdGenerator->m_dSupplyTemp );
			}

			pclColdGenerator = GetNextColdGenerator();
		}
	}

	return true;
}

bool CColdGeneratorList::UpdateColdGenerator( int iIndex, CString strColdGeneratorTypeID, double dColdCapacity, double dContent, double dSupplyTemp,
		bool bManualTemp, long lQuantity )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecColdGeneratorList.size() )
	{
		return false;
	}

	m_dTotalColdCapacity -= (m_vecColdGeneratorList[iIndex].m_dColdCapacity * m_vecColdGeneratorList[iIndex].m_lQuantity);
	// HYS-938: multiply by the quantity
	m_dTotalContent -= (m_vecColdGeneratorList[iIndex].m_dContent * m_vecColdGeneratorList[iIndex].m_lQuantity );

	m_vecColdGeneratorList[iIndex].m_strColdGeneratorTypeID = strColdGeneratorTypeID;
	m_vecColdGeneratorList[iIndex].m_dColdCapacity = dColdCapacity;
	m_vecColdGeneratorList[iIndex].m_dContent = dContent;
	m_vecColdGeneratorList[iIndex].m_bManual = ( dContent != -1.0 );
	m_vecColdGeneratorList[iIndex].m_bManualTemp = bManualTemp;
	m_vecColdGeneratorList[iIndex].m_dSupplyTemp = dSupplyTemp;
	m_vecColdGeneratorList[iIndex].m_lQuantity = lQuantity;

	if( -1.0 == dContent )
	{
		m_vecColdGeneratorList[iIndex].m_dContent = _ComputeWaterContent( strColdGeneratorTypeID, dColdCapacity, dSupplyTemp );
	}

	m_dTotalColdCapacity += (m_vecColdGeneratorList[iIndex].m_dColdCapacity * m_vecColdGeneratorList[iIndex].m_lQuantity);
	m_dTotalContent += (m_vecColdGeneratorList[iIndex].m_dContent * m_vecColdGeneratorList[iIndex].m_lQuantity);

	return true;
}

CColdGeneratorList::ColdGeneratorSave *CColdGeneratorList::GetColdGenerator( int iIndex )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecColdGeneratorList.size() )
	{
		return NULL;
	}

	return &m_vecColdGeneratorList[iIndex];
}

CColdGeneratorList::ColdGeneratorSave *CColdGeneratorList::GetFirstColdGenerator()
{
	if( 0 == m_vecColdGeneratorList.size() )
	{
		return NULL;
	}

	m_vecColdGeneratorListIter = m_vecColdGeneratorList.begin();
	return &( *m_vecColdGeneratorListIter );
}

CColdGeneratorList::ColdGeneratorSave *CColdGeneratorList::GetNextColdGenerator()
{
	if( 0 == m_vecColdGeneratorList.size() )
	{
		return NULL;
	}

	if( m_vecColdGeneratorList.end() == m_vecColdGeneratorListIter )
	{
		return NULL;
	}

	m_vecColdGeneratorListIter++;

	if( m_vecColdGeneratorList.end() == m_vecColdGeneratorListIter )
	{
		return NULL;
	}

	return &( *m_vecColdGeneratorListIter );
}

void CColdGeneratorList::SetSupplyTemperature( double dSupplyTemperature )
{
	// Run all existing cold generators to update water content.
	for( vecColdGeneratorIter iter = m_vecColdGeneratorList.begin(); iter != m_vecColdGeneratorList.end(); iter++ )
	{
		if( false == iter->m_bManualTemp )
		{
			iter->m_dSupplyTemp = dSupplyTemperature;
			double dNewContent = _ComputeWaterContent( iter->m_strColdGeneratorTypeID, iter->m_dColdCapacity, iter->m_dSupplyTemp );
			iter->m_dContent = dNewContent;
		}
	}
}

void CColdGeneratorList::SetTempOutdoor( double dTempOutdoor )
{ 
	m_dTempOutdoor = dTempOutdoor;

	// Run all existing cold generators to update water content.
	for( vecColdGeneratorIter iter = m_vecColdGeneratorList.begin(); iter != m_vecColdGeneratorList.end(); iter++ )
	{
		if( false == iter->m_bManual )
		{
			double dNewContent = _ComputeWaterContent( iter->m_strColdGeneratorTypeID, iter->m_dColdCapacity, iter->m_dSupplyTemp );
			iter->m_dContent = dNewContent;
		}
	}
}

double CColdGeneratorList::_ComputeWaterContent( CString strColdGeneratorTypeID, double dColdCapacity, double dSupplyTemp )
{
	double dWaterContent = 0.0;
	double dCoeff1 = 0.0293 * dSupplyTemp + 0.7951;
	double dCoeff2 = -0.0136 * m_dTempOutdoor + 1.4621;

	// Check the "Volume of cooling system by Karoly - 2015-03-06.xlsx" file.

	if( 0 == strColdGeneratorTypeID.Compare( _T("PM_CG_CHILLER_PHE") ) )
	{
		// Taken from "CHILLER!P8".
		dWaterContent = ( 0.1068 * ( dColdCapacity / 1000.0 / ( dCoeff1 * dCoeff2 ) ) ) / 1000.0;
	}
	else if( 0 == strColdGeneratorTypeID.Compare( _T("PM_CG_CHILLER_TUBE") ) )
	{
		// Taken from "CHILLER!P12".
		dWaterContent = ( 0.3 * ( dColdCapacity / 1000.0 / ( dCoeff1 * dCoeff2 ) ) + 50 ) / 1000.0;
	}
	else
	{
		ASSERTA_RETURN( 0.0 );
	}

	if( 0 == dColdCapacity )
	{
		return 0.0;
	}
	else
	{
		return dWaterContent;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CConsumersList.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHeatConsumersList::CompareTo( CHeatConsumersList *pclConsumersList )
{
	if( NULL == pclConsumersList )
	{
		return false;
	}

	if( m_dTotalHeatCapacity != pclConsumersList->GetTotalHeatCapacity() )
	{
		return false;
	}

	if( m_dBufferContent != pclConsumersList->GetBufferContent() )
	{
		return false;
	}

	vecConsumersList *pclHeatConsumerList = pclConsumersList->GetConsumersList();

	if( m_vecConsumersList.size() != pclHeatConsumerList->size() )
	{
		return false;
	}

	for( int iLoop = 0; iLoop < (int)m_vecConsumersList.size(); iLoop++ )
	{
		if( 0 != m_vecConsumersList[iLoop].m_strConsumersTypeID.Compare( pclHeatConsumerList->at( iLoop ).m_strConsumersTypeID ) )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dHeatCapacity != pclHeatConsumerList->at( iLoop ).m_dHeatCapacity )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dContent != pclHeatConsumerList->at( iLoop ).m_dContent )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_bManualContent != pclHeatConsumerList->at( iLoop ).m_bManualContent )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dSupplyTemp != pclHeatConsumerList->at( iLoop ).m_dSupplyTemp )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dReturnTemp != pclHeatConsumerList->at( iLoop ).m_dReturnTemp )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_bManualTemp != pclHeatConsumerList->at( iLoop ).m_bManualTemp )
		{
			return false;
		}
	}
	
	return true;
}

void CHeatConsumersList::CopyFrom( CHeatConsumersList *pclConsumersList )
{
	if( NULL == pclConsumersList )
	{
		return;
	}

	Reset();
	m_dTotalHeatCapacity = pclConsumersList->GetTotalHeatCapacity();
	m_dBufferContent = pclConsumersList->GetBufferContent();
	m_vecConsumersList = *( pclConsumersList->GetConsumersList() );
}

#define CPCONSUMERSLIST_VERSION		1
void CHeatConsumersList::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPCONSUMERSLIST_VERSION;
	outf.write( (char *)&Version, sizeof( Version ) );

	outf.write( (char *)&m_dTotalHeatCapacity, sizeof( m_dTotalHeatCapacity ) );
	outf.write( (char *)&m_dBufferContent, sizeof( m_dBufferContent ) );

	int iSize = m_vecConsumersList.size();
	outf.write( (char *)&iSize, sizeof( iSize ) );

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			WriteString( outf, m_vecConsumersList[i].m_strConsumersTypeID );
			outf.write( (char *)&m_vecConsumersList[i].m_dHeatCapacity, sizeof( m_vecConsumersList[i].m_dHeatCapacity ) );
			outf.write( (char *)&m_vecConsumersList[i].m_dContent, sizeof( m_vecConsumersList[i].m_dContent ) );
			outf.write( (char *)&m_vecConsumersList[i].m_bManualContent, sizeof( m_vecConsumersList[i].m_bManualContent ) );
			outf.write( (char *)&m_vecConsumersList[i].m_dSupplyTemp, sizeof( m_vecConsumersList[i].m_dSupplyTemp ) );
			outf.write( (char *)&m_vecConsumersList[i].m_dReturnTemp, sizeof( m_vecConsumersList[i].m_dReturnTemp ) );
			outf.write( (char *)&m_vecConsumersList[i].m_bManualTemp, sizeof( m_vecConsumersList[i].m_bManualTemp ) );
		}
	}
}

bool CHeatConsumersList::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPCONSUMERSLIST_VERSION;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CPCONSUMERSLIST_VERSION )
	{
		return false;
	}

	inpf.read( (char *)&m_dTotalHeatCapacity, sizeof( m_dTotalHeatCapacity ) );
	inpf.read( (char *)&m_dBufferContent, sizeof( m_dBufferContent ) );

	int iSize;
	inpf.read( (char *)&iSize, sizeof( iSize ) );
	m_vecConsumersList.clear();

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			ConsumersSave rConsumersSave;
			TCHAR tcID[_ID_LENGTH + 1];

			if( false == ReadString( inpf, tcID, sizeof( tcID ) ) )
			{
				return false;
			}

			rConsumersSave.m_strConsumersTypeID = CString( tcID );
			inpf.read( (char *)&rConsumersSave.m_dHeatCapacity, sizeof( rConsumersSave.m_dHeatCapacity ) );
			inpf.read( (char *)&rConsumersSave.m_dContent, sizeof( rConsumersSave.m_dContent ) );
			inpf.read( (char *)&rConsumersSave.m_bManualContent, sizeof( rConsumersSave.m_bManualContent ) );
			inpf.read( (char *)&rConsumersSave.m_dSupplyTemp, sizeof( rConsumersSave.m_dSupplyTemp ) );
			inpf.read( (char *)&rConsumersSave.m_dReturnTemp, sizeof( rConsumersSave.m_dReturnTemp ) );
			inpf.read( (char *)&rConsumersSave.m_bManualTemp, sizeof( rConsumersSave.m_bManualTemp ) );
			m_vecConsumersList.push_back( rConsumersSave );
		}
	}

	return true;
}

bool CHeatConsumersList::AddConsumers( CString strConsumersTypeID, double dHeatCapacity, double dContent, double dSupplyTemperature, 
		double dReturnTemperature, bool bManualTemp )
{
	if( dHeatCapacity < 0.0 )
	{
		return false;
	}

	ConsumersSave rConsumersSave;
	rConsumersSave.m_strConsumersTypeID = strConsumersTypeID;
	rConsumersSave.m_dHeatCapacity = dHeatCapacity;
	rConsumersSave.m_bManualContent = ( dContent != -1.0 );

	if( -1.0 == dContent )
	{
		dContent = _ComputeWaterContent( strConsumersTypeID, dHeatCapacity, dSupplyTemperature, dReturnTemperature );
	}

	rConsumersSave.m_dContent = dContent;
	rConsumersSave.m_dSupplyTemp = dSupplyTemperature;
	rConsumersSave.m_dReturnTemp = dReturnTemperature;
	rConsumersSave.m_bManualTemp = bManualTemp;
	m_vecConsumersList.push_back( rConsumersSave );

	m_dTotalHeatCapacity += dHeatCapacity;

	return true;
}

bool CHeatConsumersList::RemoveConsumers( int iIndex )
{
	if( iIndex >= ( int )m_vecConsumersList.size() )
	{
		return false;
	}

	if( -1 == iIndex )
	{
		Reset();
	}
	else
	{
		m_dTotalHeatCapacity -= m_vecConsumersList[iIndex].m_dHeatCapacity;
		m_vecConsumersList.erase( m_vecConsumersList.begin() + iIndex );
	}

	return true;
}

bool CHeatConsumersList::UpdateConsumers( int iIndex, CString strConsumersTypeID, double dHeatCapacity, double dContent, double dSupplyTemperature, 
		double dReturnTemperature, bool bManualTemp )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecConsumersList.size() )
	{
		return false;
	}

	m_dTotalHeatCapacity -= m_vecConsumersList[iIndex].m_dHeatCapacity;

	m_vecConsumersList[iIndex].m_strConsumersTypeID = strConsumersTypeID;
	m_vecConsumersList[iIndex].m_dHeatCapacity = dHeatCapacity;
	m_vecConsumersList[iIndex].m_dContent = dContent;
	m_vecConsumersList[iIndex].m_bManualContent = ( dContent != -1.0 );
	m_vecConsumersList[iIndex].m_dSupplyTemp = dSupplyTemperature;
	m_vecConsumersList[iIndex].m_dReturnTemp = dReturnTemperature;
	m_vecConsumersList[iIndex].m_bManualTemp = bManualTemp;

	if( -1.0 == dContent )
	{
		m_vecConsumersList[iIndex].m_dContent = _ComputeWaterContent( strConsumersTypeID, dHeatCapacity, dSupplyTemperature,
				dReturnTemperature );
	}

	m_dTotalHeatCapacity += m_vecConsumersList[iIndex].m_dHeatCapacity;

	return true;
}

CHeatConsumersList::ConsumersSave *CHeatConsumersList::GetConsumers( int iIndex )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecConsumersList.size() )
	{
		return NULL;
	}

	return &m_vecConsumersList[iIndex];
}

CHeatConsumersList::ConsumersSave *CHeatConsumersList::GetFirstConsumers( void )
{
	if( 0 == m_vecConsumersList.size() )
	{
		return NULL;
	}

	m_vecConsumersListIter = m_vecConsumersList.begin();
	return &( *m_vecConsumersListIter );
}

CHeatConsumersList::ConsumersSave *CHeatConsumersList::GetNextConsumers( void )
{
	if( 0 == m_vecConsumersList.size() )
	{
		return NULL;
	}

	if( m_vecConsumersList.end() == m_vecConsumersListIter )
	{
		return NULL;
	}

	m_vecConsumersListIter++;

	if( m_vecConsumersList.end() == m_vecConsumersListIter )
	{
		return NULL;
	}

	return &( *m_vecConsumersListIter );
}

void CHeatConsumersList::SetSupplyTemperature( double dSupplyTemperature )
{
	// Run all existing cold generators to update water content.
	for( vecConsumersListIter iter = m_vecConsumersList.begin(); iter != m_vecConsumersList.end(); iter++ )
	{
		if( false == iter->m_bManualTemp )
		{
			iter->m_dSupplyTemp = dSupplyTemperature;

			double dNewContent = _ComputeWaterContent( iter->m_strConsumersTypeID, iter->m_dHeatCapacity, iter->m_dSupplyTemp,
					iter->m_dReturnTemp );

			iter->m_dContent = dNewContent;
		}
	}
}

void CHeatConsumersList::SetReturnTemperature( double dReturnTemperature )
{
	// Run all existing cold generators to update water content.
	for( vecConsumersListIter iter = m_vecConsumersList.begin(); iter != m_vecConsumersList.end(); iter++ )
	{
		if( false == iter->m_bManualTemp )
		{
			iter->m_dReturnTemp = dReturnTemperature;

			double dNewContent = _ComputeWaterContent( iter->m_strConsumersTypeID, iter->m_dHeatCapacity, iter->m_dSupplyTemp,
					iter->m_dReturnTemp );

			iter->m_dContent = dNewContent;
		}
	}
}

void CHeatConsumersList::VerifyConsHeatingTemperatureValues( double dSupplyTemperature, double dReturnTemperature, bool *pbSupplyTempOK, bool *pbReturnTempOK )
{
	if( ( NULL == pbSupplyTempOK ) || ( NULL == pbReturnTempOK ) )
	{
		return;
	}

	*pbSupplyTempOK = true;
	*pbReturnTempOK = true;

	if( dSupplyTemperature <= dReturnTemperature )
	{
		*pbSupplyTempOK = false;
		*pbReturnTempOK = false;
	}

	// This condition is only because the interpolation equation (See in the '_ComputeWaterContent' method).
	if( ( dReturnTemperature + dSupplyTemperature ) / 2.0 <= 20.0 )
	{
		*pbReturnTempOK = false;
		*pbSupplyTempOK = false;
	}
}

double CHeatConsumersList::_ComputeWaterContent( CString strConsumersTypeID, double dHeatCapacity, double dSupplyTemperature, double dReturnTemperature )
{
	double num1 = 120.0;
	double num2 = 20.0;
	bool pbSupplyTempOK = true;
	bool pbReturnTempOK = true;

	VerifyConsHeatingTemperatureValues( dSupplyTemperature, dReturnTemperature, &pbSupplyTempOK, &pbReturnTempOK );
	if( pbSupplyTempOK == false || pbReturnTempOK == false )
	{
		return 0.0;
	}

	if( 0 == strConsumersTypeID.Compare( _T("PM_C_FLOORHEATING") ) )
	{
		num1 = 50.0;
	}

	double dWaterContentFromHeatEmitter;
	double dPowerWaterContentFromHeatEmitter;

	if( 0 == strConsumersTypeID.Compare( _T("PM_C_CASTIRONRAD") ) )
	{
		dWaterContentFromHeatEmitter = 9.2;
		dPowerWaterContentFromHeatEmitter = 1.33;
	}
	else if( 0 == strConsumersTypeID.Compare( _T("PM_C_STEELTUBERAD") ) )
	{
		dWaterContentFromHeatEmitter = 12.7;
		dPowerWaterContentFromHeatEmitter = 1.33;
	}
	else if( 0 == strConsumersTypeID.Compare( _T("PM_C_PANELRAD") ) )
	{
		dWaterContentFromHeatEmitter = 4.2;
		dPowerWaterContentFromHeatEmitter = 1.29;
	}
	else if( 0 == strConsumersTypeID.Compare( _T("PM_C_CONVECTOR") ) )
	{
		dWaterContentFromHeatEmitter = 1.7;
		dPowerWaterContentFromHeatEmitter = 1.5;
	}
	else if( 0 == strConsumersTypeID.Compare( _T("PM_C_VENTSYSTEM") ) )
	{
		dWaterContentFromHeatEmitter = 1.0;
		dPowerWaterContentFromHeatEmitter = 1.5;
	}
	else if( 0 == strConsumersTypeID.Compare( _T("PM_C_FLOORHEATING") ) )
	{
		dWaterContentFromHeatEmitter = 13.2;
		dPowerWaterContentFromHeatEmitter = 1.0;
	}
	else
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dContent = dHeatCapacity * dWaterContentFromHeatEmitter;
	dContent *= pow( ( num1 / ( dSupplyTemperature + dReturnTemperature - 40.0 ) ), dPowerWaterContentFromHeatEmitter );
	dContent += ( dHeatCapacity * ( 2.3 * ( num2 / ( dSupplyTemperature - dReturnTemperature ) ) ) );

	// Remark: factor above are in 'l/kW'.
	if( IsNaN( dContent ) )
	{
		return 0.0;
	}
	else
	{
		return dContent / 1000000.0;
	}
}

double CHeatConsumersList::GetTotalContent( double dSupplyTemperature, double dReturnTemperature )
{
	double totalContent = 0.0;

	for( UINT i = 0; i < m_vecConsumersList.size(); i++ )
	{
		if( m_vecConsumersList[i].m_bManualContent == false )
		{
			// Use provided temperature (e.g. : Floor Heating uses Lower Supply Temp.)
			m_vecConsumersList[i].m_dSupplyTemp = ( m_vecConsumersList[i].m_bManualTemp ? m_vecConsumersList[i].m_dSupplyTemp :
					dSupplyTemperature );
			
			m_vecConsumersList[i].m_dReturnTemp = ( m_vecConsumersList[i].m_bManualTemp ? m_vecConsumersList[i].m_dReturnTemp :
					dReturnTemperature );
			
			m_vecConsumersList[i].m_dContent = _ComputeWaterContent( m_vecConsumersList[i].m_strConsumersTypeID,
					m_vecConsumersList[i].m_dHeatCapacity, m_vecConsumersList[i].m_dSupplyTemp, m_vecConsumersList[i].m_dReturnTemp );
		}

		totalContent += m_vecConsumersList[i].m_dContent;
	}

	return totalContent + m_dBufferContent;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CColdConsumersList.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CColdConsumersList::CompareTo( CColdConsumersList *pclConsumersList )
{
	if( NULL == pclConsumersList )
	{
		return false;
	}

	if( m_dTotalColdConsCapacity != pclConsumersList->GetTotalColdCapacity() )
	{
		return false;
	}

	if( m_dBufferContent != pclConsumersList->GetBufferContent() )
	{
		return false;
	}

	if( m_dMaxAirInputTempForAHU != pclConsumersList->GetMaxAirInputTempForAHU() )
	{
		return false;
	}

	vecConsumersList *pclColdConsumerList = pclConsumersList->GetConsumersList();

	if( m_vecConsumersList.size() != pclColdConsumerList->size() )
	{
		return false;
	}

	for( int iLoop = 0; iLoop < (int)m_vecConsumersList.size(); iLoop++ )
	{
		if( 0 != m_vecConsumersList[iLoop].m_strConsumersTypeID.Compare( pclColdConsumerList->at( iLoop ).m_strConsumersTypeID ) )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dColdCapacity != pclColdConsumerList->at( iLoop ).m_dColdCapacity )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dContent != pclColdConsumerList->at( iLoop ).m_dContent )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_bManualContent != pclColdConsumerList->at( iLoop ).m_bManualContent )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dSupplyTemp != pclColdConsumerList->at( iLoop ).m_dSupplyTemp )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dReturnTemp != pclColdConsumerList->at( iLoop ).m_dReturnTemp )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_bManualTemp != pclColdConsumerList->at( iLoop ).m_bManualTemp )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dRoomTemp != pclColdConsumerList->at( iLoop ).m_dRoomTemp )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_lQuantity != pclColdConsumerList->at( iLoop ).m_lQuantity )
		{
			return false;
		}

		if( m_vecConsumersList[iLoop].m_dSurface != pclColdConsumerList->at( iLoop ).m_dSurface )
		{
			return false;
		}
	}
	
	return true;
}

void CColdConsumersList::CopyFrom( CColdConsumersList *pclConsumersList )
{
	if( NULL == pclConsumersList )
	{
		return;
	}

	Reset();
	m_pTADS = pclConsumersList->GetpTADS();
	m_dTotalColdConsCapacity = pclConsumersList->GetTotalColdCapacity();
	m_dBufferContent = pclConsumersList->GetBufferContent();
	m_dMaxAirInputTempForAHU = pclConsumersList->GetMaxAirInputTempForAHU();
	m_vecConsumersList = *( pclConsumersList->GetConsumersList() );
}

#define CPCOLDCONSUMERSLIST_VERSION		4
// Version 4: 2019-09-04: 'm_dMaxAirInputTempForAHU' added that is not the max. outdoor temperature.
// Version 3: 2018-07-10: 'm_dOutTemp' variable in the 'ConsumersSave' structure doesn't exist anymore.
void CColdConsumersList::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPCOLDCONSUMERSLIST_VERSION;
	outf.write( (char *)&Version, sizeof( Version ) );

	outf.write( (char *)&m_dTotalColdConsCapacity, sizeof( m_dTotalColdConsCapacity ) );
	outf.write( (char *)&m_dBufferContent, sizeof( m_dBufferContent ) );

	int iSize = m_vecConsumersList.size();
	outf.write( (char *)&iSize, sizeof( iSize ) );

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			WriteString( outf, m_vecConsumersList[i].m_strConsumersTypeID );
			outf.write( (char *)&m_vecConsumersList[i].m_dColdCapacity, sizeof( m_vecConsumersList[i].m_dColdCapacity ) );
			outf.write( (char *)&m_vecConsumersList[i].m_dContent, sizeof( m_vecConsumersList[i].m_dContent ) );
			outf.write( (char *)&m_vecConsumersList[i].m_bManualContent, sizeof( m_vecConsumersList[i].m_bManualContent ) );
			outf.write( (char *)&m_vecConsumersList[i].m_dSupplyTemp, sizeof( m_vecConsumersList[i].m_dSupplyTemp ) );
			outf.write( (char *)&m_vecConsumersList[i].m_dReturnTemp, sizeof( m_vecConsumersList[i].m_dReturnTemp ) );
			outf.write( (char *)&m_vecConsumersList[i].m_bManualTemp, sizeof( m_vecConsumersList[i].m_bManualTemp ) );
			outf.write( (char *)&m_vecConsumersList[i].m_dRoomTemp, sizeof( m_vecConsumersList[i].m_dRoomTemp ) );
			outf.write( (char *)&m_vecConsumersList[i].m_lQuantity, sizeof( m_vecConsumersList[i].m_lQuantity ) );
			outf.write( (char *)&m_vecConsumersList[i].m_dSurface, sizeof( m_vecConsumersList[i].m_dSurface ) );
		 }
	}

	// Version 4.
	outf.write( (char *)&m_dMaxAirInputTempForAHU, sizeof( m_dMaxAirInputTempForAHU ) );
}

bool CColdConsumersList::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPCOLDCONSUMERSLIST_VERSION;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CPCOLDCONSUMERSLIST_VERSION )
	{
		return false;
	}

	inpf.read( (char *)&m_dTotalColdConsCapacity, sizeof( m_dTotalColdConsCapacity ) );
	inpf.read( (char *)&m_dBufferContent, sizeof( m_dBufferContent ) );

	int iSize;
	inpf.read( (char *)&iSize, sizeof( iSize ) );
	m_vecConsumersList.clear();

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			ConsumersSave rConsumersSave;
			TCHAR tcID[_ID_LENGTH + 1];

			if( false == ReadString( inpf, tcID, sizeof( tcID ) ) )
			{
				return false;
			}

			rConsumersSave.m_strConsumersTypeID = CString( tcID );
			inpf.read( (char *)&rConsumersSave.m_dColdCapacity, sizeof( rConsumersSave.m_dColdCapacity ) );
			inpf.read( (char *)&rConsumersSave.m_dContent, sizeof( rConsumersSave.m_dContent ) );
			inpf.read( (char *)&rConsumersSave.m_bManualContent, sizeof( rConsumersSave.m_bManualContent ) );
			inpf.read( (char *)&rConsumersSave.m_dSupplyTemp, sizeof( rConsumersSave.m_dSupplyTemp ) );
			inpf.read( (char *)&rConsumersSave.m_dReturnTemp, sizeof( rConsumersSave.m_dReturnTemp ) );
			inpf.read( (char *)&rConsumersSave.m_bManualTemp, sizeof( rConsumersSave.m_bManualTemp ) );

			if( 2 == Version )
			{
				inpf.read( (char *)&rConsumersSave.m_dRoomTemp, sizeof( rConsumersSave.m_dRoomTemp ) );
				double dNop;
				inpf.read( (char *)&dNop, sizeof( dNop ) );
				inpf.read( (char *)&rConsumersSave.m_lQuantity, sizeof( rConsumersSave.m_lQuantity ) );
				inpf.read( (char *)&rConsumersSave.m_dSurface, sizeof( rConsumersSave.m_dSurface ) );
			}
			else if( Version > 2 )
			{
				inpf.read( (char *)&rConsumersSave.m_dRoomTemp, sizeof( rConsumersSave.m_dRoomTemp ) );
				inpf.read( (char *)&rConsumersSave.m_lQuantity, sizeof( rConsumersSave.m_lQuantity ) );
				inpf.read( (char *)&rConsumersSave.m_dSurface, sizeof( rConsumersSave.m_dSurface ) );
			}
			else
			{
				rConsumersSave.m_dRoomTemp = 20.0;
				rConsumersSave.m_lQuantity = 1;
				rConsumersSave.m_dSurface = 1.0;
			}

			m_vecConsumersList.push_back( rConsumersSave );
		}
	}

	if( Version < 4 )
	{
		return true;
	}

	// Version 4.
	inpf.read( (char *)&m_dMaxAirInputTempForAHU, sizeof( m_dMaxAirInputTempForAHU ) );

	return true;
}

bool CColdConsumersList::AddConsumers( CString strConsumersTypeID, double dColdCapacity, double dContent, double dSupplyTemperature, 
		double dReturnTemperature, bool bManualTemp, double dRoomTemperature, long lQuantity, double dSurface )
{
	if( dColdCapacity < 0.0 )
	{
		return false;
	}

	ConsumersSave rConsumersSave;
	rConsumersSave.m_strConsumersTypeID = strConsumersTypeID;
	rConsumersSave.m_dColdCapacity = dColdCapacity;
	rConsumersSave.m_bManualContent = ( dContent != -1.0 );

	if( -1.0 == dContent )
	{
		dContent = _ComputeWaterContent( 
				strConsumersTypeID,
				dColdCapacity,
				dSupplyTemperature,
				dReturnTemperature,
				dRoomTemperature,
				lQuantity,
				dSurface );
	}

	rConsumersSave.m_dContent = dContent;
	rConsumersSave.m_dSupplyTemp = dSupplyTemperature;
	rConsumersSave.m_dReturnTemp = dReturnTemperature;
	rConsumersSave.m_bManualTemp = bManualTemp;
	rConsumersSave.m_lQuantity = lQuantity;
	rConsumersSave.m_dSurface = dSurface;
	m_vecConsumersList.push_back( rConsumersSave );

	m_dTotalColdConsCapacity += ( dColdCapacity * lQuantity );

	return true;
}

bool CColdConsumersList::RemoveConsumers( int iIndex )
{
	if( iIndex >= ( int )m_vecConsumersList.size() )
	{
		return false;
	}

	if( -1 == iIndex )
	{
		Reset();
	}
	else
	{
		m_dTotalColdConsCapacity -= (m_vecConsumersList[iIndex].m_dColdCapacity*m_vecConsumersList[iIndex].m_lQuantity);
		m_vecConsumersList.erase( m_vecConsumersList.begin() + iIndex );
	}

	return true;
}

bool CColdConsumersList::UpdateConsumers( int iIndex, CString strConsumersTypeID, double dColdCapacity, double dContent, double dSupplyTemperature, 
		double dReturnTemperature, bool bManualTemp, double dRoomTemperature, long lQuantity, double dSurface )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecConsumersList.size() )
	{
		return false;
	}

	m_dTotalColdConsCapacity -= (m_vecConsumersList[iIndex].m_dColdCapacity * m_vecConsumersList[iIndex].m_lQuantity);

	m_vecConsumersList[iIndex].m_strConsumersTypeID = strConsumersTypeID;
	m_vecConsumersList[iIndex].m_dColdCapacity = dColdCapacity;
	m_vecConsumersList[iIndex].m_dContent = dContent;
	m_vecConsumersList[iIndex].m_bManualContent = ( dContent != -1.0 );
	m_vecConsumersList[iIndex].m_dSupplyTemp = dSupplyTemperature;
	m_vecConsumersList[iIndex].m_dReturnTemp = dReturnTemperature;
	m_vecConsumersList[iIndex].m_bManualTemp = bManualTemp;
	m_vecConsumersList[iIndex].m_lQuantity = lQuantity;

	if( 0 == strConsumersTypeID.Compare( _T("PM_C_SURFACECOOLING") ) )
	{
		m_vecConsumersList[iIndex].m_dSurface = dSurface;
	}
	else
	{
		m_vecConsumersList[iIndex].m_dRoomTemp = dRoomTemperature;
	}

	if( -1.0 == dContent )
	{
		m_vecConsumersList[iIndex].m_dContent = _ComputeWaterContent( 
				strConsumersTypeID,
				dColdCapacity,
				dSupplyTemperature,
				dReturnTemperature,
				dRoomTemperature,
				lQuantity,
				dSurface );
	}

	m_dTotalColdConsCapacity += ( m_vecConsumersList[iIndex].m_dColdCapacity * m_vecConsumersList[iIndex].m_lQuantity );

	return true;
}

CColdConsumersList::ConsumersSave *CColdConsumersList::GetConsumer( int iIndex )
{
	if( iIndex < 0 || iIndex >= (int)m_vecConsumersList.size() )
	{
		return NULL;
	}

	return &m_vecConsumersList[iIndex];
}

CColdConsumersList::ConsumersSave *CColdConsumersList::GetFirstConsumers( void )
{
	if( 0 == m_vecConsumersList.size() )
	{
		return NULL;
	}

	m_vecConsumersListIter = m_vecConsumersList.begin();
	return &( *m_vecConsumersListIter );
}

CColdConsumersList::ConsumersSave *CColdConsumersList::GetNextConsumers( void )
{
	if( 0 == m_vecConsumersList.size() )
	{
		return NULL;
	}

	if( m_vecConsumersList.end() == m_vecConsumersListIter )
	{
		return NULL;
	}

	m_vecConsumersListIter++;

	if( m_vecConsumersList.end() == m_vecConsumersListIter )
	{
		return NULL;
	}

	return &( *m_vecConsumersListIter );
}

void CColdConsumersList::SetSupplyTemperature( double dSupplyTemperature )
{
	// Run all existing cold generators to update water content.
	for( vecConsumersListIter iter = m_vecConsumersList.begin(); iter != m_vecConsumersList.end(); iter++ )
	{
		if( false == iter->m_bManualTemp )
		{
			iter->m_dSupplyTemp = dSupplyTemperature;

			double dNewContent = _ComputeWaterContent( iter->m_strConsumersTypeID, iter->m_dColdCapacity, iter->m_dSupplyTemp,
				iter->m_dReturnTemp, iter->m_dRoomTemp, iter->m_lQuantity, iter->m_dSurface );

			iter->m_dContent = dNewContent;
		}
	}
}

void CColdConsumersList::SetReturnTemperature( double dReturnTemperature )
{
	// Run all existing cold generators to update water content.
	for( vecConsumersListIter iter = m_vecConsumersList.begin(); iter != m_vecConsumersList.end(); iter++ )
	{
		if( false == iter->m_bManualTemp )
		{
			iter->m_dReturnTemp = dReturnTemperature;

			double dNewContent = _ComputeWaterContent( iter->m_strConsumersTypeID, iter->m_dColdCapacity, iter->m_dSupplyTemp,
				iter->m_dReturnTemp, iter->m_dRoomTemp, iter->m_lQuantity, iter->m_dSurface );

			iter->m_dContent = dNewContent;
		}
	}
}

void CColdConsumersList::SetMaxAirInputTempForAHU( double dMaxAirInputTempForAHU )
{
	m_dMaxAirInputTempForAHU = dMaxAirInputTempForAHU;

	// Run all existing cold consumers to update water content.
	for( vecConsumersListIter iter = m_vecConsumersList.begin(); iter != m_vecConsumersList.end(); iter++ )
	{
		if( false == iter->m_bManualContent )
		{
			double dNewContent = _ComputeWaterContent( iter->m_strConsumersTypeID, iter->m_dColdCapacity, iter->m_dSupplyTemp,
				iter->m_dReturnTemp, iter->m_dRoomTemp, iter->m_lQuantity, iter->m_dSurface );

			iter->m_dContent = dNewContent;
		}
	}
}

void CColdConsumersList::VerifyConsCoolingTemperatureValues( CString strConsumersTypeID, double dSupplyTemperature, double dReturnTemperature,
	double dRoomTemperature, bool *pbSupplyTempOK, bool *pbReturnTempOK, bool *pbRoomTempOK )
{
	if( ( NULL == pbSupplyTempOK ) || ( NULL == pbReturnTempOK ) || ( NULL == pbRoomTempOK ) )
	{
		return;
	}

	*pbSupplyTempOK = true;
	*pbReturnTempOK = true;
	*pbRoomTempOK = true;

	if( dSupplyTemperature >= dReturnTemperature )
	{
		*pbSupplyTempOK = false;
		*pbReturnTempOK = false;
	}

	if( 0 != strConsumersTypeID.Compare( _T("PM_C_SURFACECOOLING") ) )
	{

		if( dReturnTemperature >= dRoomTemperature )
		{
			*pbReturnTempOK = false;
			*pbRoomTempOK = false;
		}

		if( dSupplyTemperature >= dRoomTemperature )
		{
			*pbSupplyTempOK = false;
			*pbRoomTempOK = false;
		}
	}

	if( 0 == strConsumersTypeID.Compare( _T("PM_C_FANCOIL") ) )
	{
		if( dSupplyTemperature >= ( dRoomTemperature - 1 ) )
		{
			*pbSupplyTempOK = false;
			*pbRoomTempOK = false;
		}
		
		if( dSupplyTemperature >= ( dReturnTemperature - 1 ) )
		{
			*pbSupplyTempOK = false;
			*pbReturnTempOK = false;
		}
	}
	else if( 0 == strConsumersTypeID.Compare( _T("PM_C_AIRHANDLERUNIT") ) )
	{
		if( dSupplyTemperature >= m_dMaxAirInputTempForAHU )
		{
			*pbSupplyTempOK = false;
		}
		
		if( dReturnTemperature >= m_dMaxAirInputTempForAHU )
		{
			*pbReturnTempOK = false;
		}
	}
}

double CColdConsumersList::_ComputeWaterContent( CString strConsumersTypeID, double dColdCapacity, double dSupplyTemperature, 
		double dReturnTemperature, double dRoomTemperature, long lQuantity, double dSurface )
{
	bool bSupplyTempOK = true;
	bool bReturnTempOK = true;
	bool bRoomTempOK = true;
	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dContent = 0.0;

	// Check the "Volume of cooling system by Karoly - 2015-03-06.xlsx" file.

	if( dColdCapacity <= 0.0 && 0 != strConsumersTypeID.Compare( _T("PM_C_SURFACECOOLING") ) )
	{
		return dContent;
	}

	// verify temperature
	VerifyConsCoolingTemperatureValues( strConsumersTypeID, dSupplyTemperature, dReturnTemperature, dRoomTemperature, &bSupplyTempOK, &bReturnTempOK, &bRoomTempOK );
	
	if( bSupplyTempOK == false || bReturnTempOK == false || bRoomTempOK == false )
	{
		return 0.0;
	}

	if( 0 == strConsumersTypeID.Compare( _T("PM_C_FANCOIL") ) )
	{
		double dDt1 = dRoomTemperature - 1.0 - dSupplyTemperature;
		double dDt2 = dRoomTemperature - dReturnTemperature;
		double dDtln = ( dDt1 - dDt2 ) / log( dDt1 / dDt2 );
		double dDtlnBasic = 13.9042;
		double dRatio = dDtln / dDtlnBasic;

		// Taken from "FC!Q6"
		double dUnit = ( 0.55 * ( dColdCapacity / 1000.0 / dRatio ) + 0.15 ) / 1000.0;
		
		// Piping.
		double dSPam = m_pTADS->GetpTechParams()->GetPipeTargDp();
		double dDt = dReturnTemperature - dSupplyTemperature;
		double dCoeff1 = 5.0488 * pow( dSPam, -0.35 );
		double dCoeff2 = 2.8349 * pow( dDt, -0.643 );
		
		// Taken from "PIPE!O7".
		double dPiping = ( 17 * dColdCapacity / 1000.0 * dCoeff1 * dCoeff2 ) / 1000.0;
		
		dContent = dUnit + dPiping;
		dContent *= lQuantity;
	}
	else if( 0 == strConsumersTypeID.Compare( _T("PM_C_AIRHANDLERUNIT") ) )
	{
		// HYS-958: To refer to the Excel sheet from Karoly Dt1 = Tairout - Tr and Dt2 = Tairin - Ts
		// HYS-958: 'm_dTempOutdoor' becomes 'm_dMaxAirInputTempForAHU'.
		double dDt1 = m_dMaxAirInputTempForAHU - dReturnTemperature;
		double dDt2 = dRoomTemperature - dSupplyTemperature;

		// HYS-1356: If 'dDt1' and 'dDt2' are the same, we need to add 1e-7 to have a result.
		if( dDt1 == dDt2 )
		{
			dDt1 += 1e-7;
		}

		double dDtln = ( dDt1 - dDt2 ) / log( dDt1 / dDt2 );
		double dDtlnBasic = 19.8491;
		double dRatio = dDtln / dDtlnBasic;

		// Taken from "AHU!S6".
		double dUnit = ( 0.345 * ( dColdCapacity / 1000.0 / dRatio )  + 0.4184 ) / 1000.0;
		
		// Piping.
		double dSPam = m_pTADS->GetpTechParams()->GetPipeTargDp();
		double dDt = dReturnTemperature - dSupplyTemperature;
		double dCoeff5 = 5.0488 * pow( dSPam, -0.35 );
		double dCoeff6 = 2.8349 * pow( dDt, -0.643 );
		
		// Taken from "PIPE!R60".
		double dPiping = ( 7.8 * dColdCapacity * dCoeff5 * dCoeff6 / 1000.0 ) / 1000.0;

		dContent = dUnit + dPiping;
		dContent *= lQuantity;
	}
	else if( 0 == strConsumersTypeID.Compare( _T("PM_C_SURFACECOOLING") ) )
	{
		// Taken from "SURFACE!I10".
		dContent = dSurface / 1000.0;
		
		// Piping.
		double dSPam = m_pTADS->GetpTechParams()->GetPipeTargDp();
		double dDt = dReturnTemperature - dSupplyTemperature;
		double dCoeff3 = 5.0488 * pow( dSPam, -0.35 );
		double dCoeff4 = 2.1006 * pow( dDt, -0.672 );

		// Taken from "PIPE!R34".
		double dPiping = ( 26 * dColdCapacity / 1000.0 * dCoeff3 * dCoeff4 ) / 1000.0;
		
		dContent += dPiping;
		dContent *= lQuantity;
	}
	else
	{
		ASSERTA_RETURN( 0.0 );
	}


	// Remark: factor above are in 'l/kW'.
	if( true == IsNaN( dContent ) )
	{
		return 0.0;
	}
	else
	{
		return dContent;
	}
}

double CColdConsumersList::GetTotalContent( double dSupplyTemperature, double dReturnTemperature )
{
	double totalContent = 0.0;

	for( UINT i = 0; i < m_vecConsumersList.size(); i++ )
	{
		if( m_vecConsumersList[i].m_bManualContent == false )
		{
			// Use provided temperature (e.g. : Floor Heating uses Lower Supply Temp.)
			m_vecConsumersList[i].m_dSupplyTemp = ( m_vecConsumersList[i].m_bManualTemp ? m_vecConsumersList[i].m_dSupplyTemp :
					dSupplyTemperature );
			
			m_vecConsumersList[i].m_dReturnTemp = ( m_vecConsumersList[i].m_bManualTemp ? m_vecConsumersList[i].m_dReturnTemp :
					dReturnTemperature );
			
			m_vecConsumersList[i].m_dContent = _ComputeWaterContent( 
					m_vecConsumersList[i].m_strConsumersTypeID,
					m_vecConsumersList[i].m_dColdCapacity,
					m_vecConsumersList[i].m_dSupplyTemp,
					m_vecConsumersList[i].m_dReturnTemp,
					m_vecConsumersList[i].m_dRoomTemp,
					m_vecConsumersList[i].m_lQuantity,
					m_vecConsumersList[i].m_dSurface);
		}

		totalContent += m_vecConsumersList[i].m_dContent;
	}

	return totalContent + m_dBufferContent;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CPipesList.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CPipeList::CompareTo( CPipeList *pclPipeList )
{
	if( NULL == pclPipeList )
	{
		return false;
	}

	if( m_dTotalContent != pclPipeList->GetTotalContent() )
	{
		return false;
	}

	vecPipeList *pvecPipeList = pclPipeList->GetPipeList();

	if( m_vecPipeList.size() != pvecPipeList->size() )
	{
		return false;
	}

	for( int iLoop = 0; iLoop < (int)m_vecPipeList.size(); iLoop++ )
	{
		if( 0 != m_vecPipeList[iLoop].m_strPipeSerieID.Compare( pvecPipeList->at( iLoop ).m_strPipeSerieID ) )
		{
			return false;
		}

		if( 0 != m_vecPipeList[iLoop].m_strPipeSizeID.Compare( pvecPipeList->at( iLoop ).m_strPipeSizeID ) )
		{
			return false;
		}

		if( m_vecPipeList[iLoop].m_dLength != pvecPipeList->at( iLoop ).m_dLength )
		{
			return false;
		}

		if( m_vecPipeList[iLoop].m_dContent != pvecPipeList->at( iLoop ).m_dContent )
		{
			return false;
		}
	}

	return true;
}

void CPipeList::CopyFrom( CPipeList *pclPipeList )
{
	if( NULL == pclPipeList )
	{
		return;
	}

	Reset();
	m_dTotalContent = pclPipeList->GetTotalContent();
	m_vecPipeList = *( pclPipeList->GetPipeList() );
}

#define CPPIPELIST_VERSION		1
void CPipeList::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPPIPELIST_VERSION;
	outf.write( (char *)&Version, sizeof( Version ) );

	outf.write( (char *)&m_dTotalContent, sizeof( m_dTotalContent ) );

	int iSize = m_vecPipeList.size();
	outf.write( (char *)&iSize, sizeof( iSize ) );

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			WriteString( outf, ( LPCTSTR )m_vecPipeList[i].m_strPipeSerieID );
			WriteString( outf, ( LPCTSTR )m_vecPipeList[i].m_strPipeSizeID );
			outf.write( (char *)&m_vecPipeList[i].m_dLength, sizeof( m_vecPipeList[i].m_dLength ) );
			outf.write( (char *)&m_vecPipeList[i].m_dContent, sizeof( m_vecPipeList[i].m_dContent ) );
		}
	}
}

bool CPipeList::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPPIPELIST_VERSION;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CPPIPELIST_VERSION )
	{
		return false;
	}

	inpf.read( (char *)&m_dTotalContent, sizeof( m_dTotalContent ) );

	int iSize;
	inpf.read( (char *)&iSize, sizeof( iSize ) );
	m_vecPipeList.clear();

	if( iSize > 0 )
	{
		for( int i = 0; i < iSize; i++ )
		{
			PipeSave rPipeSave;

			if( false == ReadString( inpf, rPipeSave.m_strPipeSerieID ) )
			{
				return false;
			}

			if( false == ReadString( inpf, rPipeSave.m_strPipeSizeID ) )
			{
				return false;
			}

			inpf.read( (char *)&rPipeSave.m_dLength, sizeof( rPipeSave.m_dLength ) );
			inpf.read( (char *)&rPipeSave.m_dContent, sizeof( rPipeSave.m_dContent ) );
			m_vecPipeList.push_back( rPipeSave );
		}
	}

	return true;
}

bool CPipeList::PipeExistInDB()
{
	for( UINT i = 0; i < m_vecPipeList.size(); i++ )
	{
		CTable *pipeSerie = ( CTable * )TASApp.GetpPipeDB()->Get( m_vecPipeList[i].m_strPipeSerieID ).MP;

		if( NULL == pipeSerie )
		{
			return false;
		}

		CDB_Pipe *pipe = ( CDB_Pipe * )TASApp.GetpPipeDB()->Get( m_vecPipeList[i].m_strPipeSizeID ).MP;

		if( NULL == pipe )
		{
			return false;
		}
	}

	return true;
}

bool CPipeList::AddPipe( CString strPipeSeriesID, CString strPipeSizeID, double dLength )
{
	if( 0 == strPipeSeriesID.GetLength() || 0 == strPipeSizeID.GetLength() || dLength < 0.0 )
	{
		return false;
	}

	PipeSave rPipeSave;
	rPipeSave.m_strPipeSerieID = strPipeSeriesID;
	rPipeSave.m_strPipeSizeID = strPipeSizeID;
	rPipeSave.m_dLength = dLength;

	double dContent = _ComputeWaterContent( strPipeSizeID, dLength );

	rPipeSave.m_dContent = dContent;
	m_vecPipeList.push_back( rPipeSave );

	m_dTotalContent += dContent;

	return true;
}

bool CPipeList::RemovePipe( int iIndex )
{
	if( iIndex >= ( int )m_vecPipeList.size() )
	{
		return false;
	}

	if( -1 == iIndex )
	{
		Reset();
	}
	else
	{
		m_dTotalContent -= m_vecPipeList[iIndex].m_dContent;
		m_vecPipeList.erase( m_vecPipeList.begin() + iIndex );
	}

	return true;
}

bool CPipeList::UpdatePipe( int iIndex, CString strPipeSerieID, CString strPipeSizeID, double dLength )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecPipeList.size() )
	{
		return false;
	}

	m_dTotalContent -= m_vecPipeList[iIndex].m_dContent;

	m_vecPipeList[iIndex].m_strPipeSerieID = strPipeSerieID;
	m_vecPipeList[iIndex].m_strPipeSizeID = strPipeSizeID;
	m_vecPipeList[iIndex].m_dLength = dLength;
	m_vecPipeList[iIndex].m_dContent = _ComputeWaterContent( strPipeSizeID, dLength );

	m_dTotalContent += m_vecPipeList[iIndex].m_dContent;

	return true;
}

CPipeList::PipeSave *CPipeList::GetPipe( int iIndex )
{
	if( iIndex < 0 || iIndex >= ( int )m_vecPipeList.size() )
	{
		return NULL;
	}

	return &m_vecPipeList[iIndex];
}

CPipeList::PipeSave *CPipeList::GetFirstPipe( void )
{
	if( 0 == m_vecPipeList.size() )
	{
		return NULL;
	}

	m_vecPipeListIter = m_vecPipeList.begin();
	return &( *m_vecPipeListIter );
}

CPipeList::PipeSave *CPipeList::GetNextPipe( void )
{
	if( 0 == m_vecPipeList.size() )
	{
		return NULL;
	}

	if( m_vecPipeList.end() == m_vecPipeListIter )
	{
		return NULL;
	}

	m_vecPipeListIter++;

	if( m_vecPipeList.end() == m_vecPipeListIter )
	{
		return NULL;
	}

	return &( *m_vecPipeListIter );
}

double CPipeList::_ComputeWaterContent( CString strPipeID, double dLength )
{
	CDB_Pipe *pipe = ( CDB_Pipe * )TASApp.GetpPipeDB()->Get( strPipeID ).MP;

	if( !pipe )
	{
		return 0.0;
	}

	return pipe->GetFluidVolumeByMeter() * dLength;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CPMInputUser.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPMInputUser::CPMInputUser()
{
	m_pTADB = NULL;
	m_pTADS = NULL;
	
	m_eApplicationType = ProjectType::Heating;		// Common to heating, cooling and solar.
	m_strPressureMaintenanceTypeID = "";			// Common to heating, cooling and solar.
	m_strWaterMakeUpTypeID = "";					// Common to heating, cooling and solar.
	m_iDegassingChecked = 0;						// Common to heating, cooling and solar.
	m_strNormID = "";								// Common to heating, cooling and solar.
	m_iPzChecked = 0;								// Common to heating, cooling and solar.
	m_dPz = 0.0;									// Common to heating, cooling and solar.
	m_dStaticHeight = 0.0;							// Common to heating, cooling and solar.
	m_dSystemVolume = 0.0;							// Common to heating, cooling and solar.
	m_dSolarCollectorVolume = 0.0;
	m_dStorageTankVolume = 0.0;						// HYS-1534: Common to heating, cooling and solar.
	m_dStorageTankMaxTemp = 0.0;						// HYS-1534: Common to heating, cooling and solar.
	m_dInstalledPower = 0.0;						// Common to heating, cooling and solar.
	m_dSafetyPressValve = 0.0;						// Common to heating, cooling and solar.
	m_iSafetyValveLocationChecked = BST_UNCHECKED;			// HYS-1083. Common to heating, cooling and solar.
	m_dSafetyValveLocation = 0.0;							// HYS-1083. Common to heating, cooling and solar.
	m_dSafetyTempLimiter = 0.0;						// Common to heating and solar.
	m_dMinTemperature = 0.0;						// Min to heating and solar, Max for Cooling.
	m_fCheckMinTemperature = false;					// For Cooling (true if user entered a min temperature, false if it's the same as supply).
	m_dMaxTemperature = 0.0;						// For cooling.
	m_dSupplyTemperature = 0.0;						// For heating and solar.
	m_dReturnTemperature = 0.0;						// For heating and solar.
	m_dFillTemperature = 0.0;						// Only for cooling.
	m_ePressOn = PressurON::poPumpSuction;			// Common to heating, cooling and solar.
	m_dPumpHead = 0.0;								// Common to heating, cooling and solar.
	m_dMaxWidth = 0.0;								// Common to heating, cooling and solar.
	m_dMaxHeight = 0.0;								// Common to heating, cooling and solar.
	m_dDegassingPressureConnectPoint = 0.0;
	m_dDegassingMaxTempConnectPoint = 0.0;
	m_dWaterMakeUpNetworkPN = 0.0;
	m_dWaterMakeUpWaterTemp = 0.0;
	m_dWaterMakeUpWaterHardness = 0.0;
	m_clPMWQSelectionPreferences.Reset();
	m_clHeatGeneratorList.Reset();
	m_clHeatConsumersList.Reset();
	m_clHeatingPipeList.Reset();

	m_mapPMTypeMatchIDEnum[_T("PMT_ALL_TAB")] = MaintenanceType::MT_All;
	m_mapPMTypeMatchIDEnum[_T("PMT_EXPVSSL_TAB")] = MaintenanceType::MT_ExpansionVessel;
	m_mapPMTypeMatchIDEnum[_T("PMT_EXPVSSLMBR_TAB")] = MaintenanceType::MT_ExpansionVesselWithMembrane;
	m_mapPMTypeMatchIDEnum[_T("PMT_WITHCOMPRESS_TAB")] = MaintenanceType::MT_WithCompressor;
	m_mapPMTypeMatchIDEnum[_T("PMT_WITHPUMP_TAB")] = MaintenanceType::MT_WithPump;
	m_mapPMTypeMatchIDEnum[_T("PMT_NONE_TAB")] = MaintenanceType::MT_None;

	m_dSystemVolumeBackup = 0.0;
	m_dInstalledPowerBackup = 0.0;

	// Working variables.
	m_bSolarContentMultiplierFactorWarningDisplayed = false;
}

bool CPMInputUser::CompareTo( CPMInputUser *pclInputUser )
{
	if( NULL == pclInputUser )
	{
		return false;
	}

	if( false == m_WC.Compare( *pclInputUser->GetpWC(), true ) )
	{
		return false;
	}

	if( m_eApplicationType != pclInputUser->GetApplicationType() )
	{
		return false;
	}

	if( 0 != _tcscmp( m_strPressureMaintenanceTypeID, pclInputUser->GetPressureMaintenanceTypeID() ) )
	{
		return false;
	}

	if( 0 != _tcscmp( m_strWaterMakeUpTypeID, pclInputUser->GetWaterMakeUpTypeID() ) )
	{
		return false;
	}

	if( m_iDegassingChecked != pclInputUser->GetDegassingChecked() )
	{
		return false;
	}

	if( 0 != _tcscmp( m_strNormID, pclInputUser->GetNormID() ) )
	{
		return false;
	}

	if( m_dStaticHeight != pclInputUser->GetStaticHeight() )
	{
		return false;
	}

	if( m_iPzChecked != pclInputUser->GetPzChecked() )
	{
		return false;
	}

	if( m_dPz != pclInputUser->GetPz() )
	{
		return false;
	}

	if( m_dSystemVolume != pclInputUser->GetSystemVolume() )
	{
		return false;
	}

	if( m_dSolarCollectorVolume != pclInputUser->GetSolarCollectorVolume() )
	{
		return false;
	}

	if( m_dStorageTankVolume != pclInputUser->GetStorageTankVolume() )
	{
		return false;
	}

	if( m_dStorageTankMaxTemp != pclInputUser->GetStorageTankMaxTemp() )
	{
		return false;
	}

	if( m_dInstalledPower != pclInputUser->GetInstalledPower() )
	{
		return false;
	}

	if( m_dSafetyPressValve != pclInputUser->GetSafetyValveResponsePressure() )
	{
		return false;
	}

	// HYS-1083.
	if( m_iSafetyValveLocationChecked != pclInputUser->GetSafetyValveLocationChecked() )
	{
		return false;
	}

	// HYS-1083.
	if( m_dSafetyPressValve != pclInputUser->GetSafetyValveResponsePressure() )
	{
		return false;
	}

	if( m_dSafetyTempLimiter != pclInputUser->GetSafetyTempLimiter() )
	{
		return false;
	}

	if( m_dMaxTemperature != pclInputUser->GetMaxTemperature() )
	{
		return false;
	}

	if( m_dSupplyTemperature != pclInputUser->GetSupplyTemperature() )
	{
		return false;
	}

	if( m_dReturnTemperature != pclInputUser->GetReturnTemperature() )
	{
		return false;
	}

	if( m_dMinTemperature != pclInputUser->GetMinTemperature() )
	{
		return false;
	}

	if( m_fCheckMinTemperature != pclInputUser->GetCheckMinTemperature() )
	{
		return false;
	}

	if( m_dFillTemperature != pclInputUser->GetFillTemperature() )
	{
		return false;
	}

	if( m_ePressOn != pclInputUser->GetPressOn() )
	{
		return false;
	}

	if( m_dPumpHead != pclInputUser->GetPumpHead() )
	{
		return false;
	}

	if( m_dMaxWidth != pclInputUser->GetMaxWidth() )
	{
		return false;
	}

	if( m_dMaxHeight != pclInputUser->GetMaxHeight() )
	{
		return false;
	}

	if( m_dDegassingMaxTempConnectPoint != pclInputUser->GetDegassingMaxTempConnectPoint() )
	{
		return false;
	}

	if( m_dDegassingPressureConnectPoint != pclInputUser->GetDegassingPressureConnectPoint() )
	{
		return false;
	}

	if( m_dWaterMakeUpNetworkPN != pclInputUser->GetWaterMakeUpNetworkPN() )
	{
		return false;
	}

	if( m_dWaterMakeUpWaterTemp != pclInputUser->GetWaterMakeUpWaterTemp() )
	{
		return false;
	}

	if( m_dWaterMakeUpWaterHardness != pclInputUser->GetWaterMakeUpWaterHardness() )
	{
		return false;
	}

	if( false == m_clPMWQSelectionPreferences.CompareTo( pclInputUser->GetPMWQSelectionPreferences() ) )
	{
		return false;
	}

	if( false == m_clHeatGeneratorList.CompareTo( pclInputUser->GetHeatGeneratorList() ) )
	{
		return false;
	}

	if( false == m_clColdGeneratorList.CompareTo( pclInputUser->GetColdGeneratorList() ) )
	{
		return false;
	}

	if( false == m_clHeatConsumersList.CompareTo( pclInputUser->GetHeatConsumersList() ) )
	{
		return false;
	}

	if( false == m_clColdConsumersList.CompareTo( pclInputUser->GetColdConsumersList() ) )
	{
		return false;
	}

	if( false == m_clHeatingPipeList.CompareTo( pclInputUser->GetHeatingPipeList() ) )
	{
		return false;
	}

	if( false == m_clCoolingPipeList.CompareTo( pclInputUser->GetCoolingPipeList() ) )
	{
		return false;
	}

	return true;
}

void CPMInputUser::CopyFrom( CPMInputUser *pclInputUserFrom )
{
	if( NULL == pclInputUserFrom )
	{
		return;
	}

	m_pTADB = pclInputUserFrom->GetpTADB();
	m_pTADS = pclInputUserFrom->GetpTADS();

	m_WC = *pclInputUserFrom->GetpWC();
	m_eApplicationType = pclInputUserFrom->GetApplicationType();
	m_strPressureMaintenanceTypeID = pclInputUserFrom->GetPressureMaintenanceTypeID();
	m_strWaterMakeUpTypeID = pclInputUserFrom->GetWaterMakeUpTypeID();
	m_iDegassingChecked = pclInputUserFrom->GetDegassingChecked();
	m_strNormID = pclInputUserFrom->GetNormID();
	m_dStaticHeight = pclInputUserFrom->GetStaticHeight();
	m_iPzChecked = pclInputUserFrom->GetPzChecked();
	m_dPz = pclInputUserFrom->GetPz();
	m_dSystemVolume = pclInputUserFrom->GetSystemVolume();
	m_dSolarCollectorVolume = pclInputUserFrom->GetSolarCollectorVolume();
	m_iSolarCollectorMultiplierFactorChecked = pclInputUserFrom->GetSolarCollectorMultiplierFactorChecked();
	m_dSolarCollectorMultiplierFactor = pclInputUserFrom->GetSolarCollectorMultiplierFactor();
	m_dStorageTankVolume = pclInputUserFrom->GetStorageTankVolume();
	m_dStorageTankMaxTemp = pclInputUserFrom->GetStorageTankMaxTemp();
	m_dInstalledPower = pclInputUserFrom->GetInstalledPower();
	m_dSafetyPressValve = pclInputUserFrom->GetSafetyValveResponsePressure();

	// HYS-1083.
	m_iSafetyValveLocationChecked = pclInputUserFrom->GetSafetyValveLocationChecked();
	m_dSafetyValveLocation = pclInputUserFrom->GetSafetyValveLocation();

	m_dSafetyTempLimiter = pclInputUserFrom->GetSafetyTempLimiter();
	m_dMinTemperature = pclInputUserFrom->GetMinTemperature();
	m_fCheckMinTemperature = pclInputUserFrom->GetCheckMinTemperature();
	m_dMaxTemperature = pclInputUserFrom->GetMaxTemperature();
	m_dSupplyTemperature = pclInputUserFrom->GetSupplyTemperature();
	m_dReturnTemperature = pclInputUserFrom->GetReturnTemperature();
	m_dFillTemperature = pclInputUserFrom->GetFillTemperature();
	m_ePressOn = pclInputUserFrom->GetPressOn();
	m_dPumpHead = pclInputUserFrom->GetPumpHead();
	m_dMaxWidth = pclInputUserFrom->GetMaxWidth();
	m_dMaxHeight = pclInputUserFrom->GetMaxHeight();
	m_dDegassingMaxTempConnectPoint = pclInputUserFrom->GetDegassingMaxTempConnectPoint();
	m_dDegassingPressureConnectPoint = pclInputUserFrom->GetDegassingPressureConnectPoint();
	m_dWaterMakeUpNetworkPN = pclInputUserFrom->GetWaterMakeUpNetworkPN();
	m_dWaterMakeUpWaterTemp = pclInputUserFrom->GetWaterMakeUpWaterTemp();
	m_dWaterMakeUpWaterHardness = pclInputUserFrom->GetWaterMakeUpWaterHardness();

	m_clPMWQSelectionPreferences.CopyFrom( pclInputUserFrom->GetPMWQSelectionPreferences() );
	m_clHeatGeneratorList.CopyFrom( pclInputUserFrom->GetHeatGeneratorList() );
	m_clColdGeneratorList.CopyFrom( pclInputUserFrom->GetColdGeneratorList() );
	m_clHeatConsumersList.CopyFrom( pclInputUserFrom->GetHeatConsumersList() );
	m_clColdConsumersList.CopyFrom( pclInputUserFrom->GetColdConsumersList() );
	m_clHeatingPipeList.CopyFrom( pclInputUserFrom->GetHeatingPipeList() );
	m_clCoolingPipeList.CopyFrom( pclInputUserFrom->GetCoolingPipeList() );
}

#define CPMINPUTUSER_VERSION	12
// Version 12: 2021-01-27 - HYS-1583: "PM_NORM_SWKI" ID has been changed into "PM_NORM_SWKI_HE301_01" (HYS-1565) -> Need to check for old project !!!!
// Version 11: 2020-11-24 - HYS-1534: variables added for storage tank and solar collector.
// Version 10: 2019-07-26 - HYS-1083: 'm_iSafetyValveLocationChecked' and 'm_dSafetyValveLocation' added.
// Version 9: 2018-07-04 - 'm_iPzChecked' added.
// Version 8: 2017-06-30 - 'm_clCoolingPipeList' added.
// Version 7: 2015-08-27 - 'm_dWaterMakeUpHardness' added.
// Version 6: 2015-08-10 - 'm_iWaterMakeUpChecked' replaced by 'm_strWaterMakeUpTypeID'.
// Version 5: add 'm_clColdGeneratorList' & 'm_clColdConsumersList'.
// Version 4: add 'm_dDegassingPressureConnectPoint'.
// Version 3: add 'm_fCheckMinTemperature'.
void CPMInputUser::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPMINPUTUSER_VERSION;
	outf.write( (char *)&Version, sizeof( Version ) );

	// Infos.
	outf.write( (char *)&m_eApplicationType, sizeof( m_eApplicationType ) );
	WriteString( outf, ( LPCTSTR )m_strPressureMaintenanceTypeID );
	
	// Version 6.
	WriteString( outf, ( LPCTSTR )m_strWaterMakeUpTypeID );
	
	outf.write( (char *)&m_iDegassingChecked, sizeof( m_iDegassingChecked ) );
	WriteString( outf, ( LPCTSTR )m_strNormID );
	WriteDouble( outf, m_dSystemVolume );
	WriteDouble( outf, m_dSolarCollectorVolume );
	WriteDouble( outf, m_dInstalledPower );
	WriteDouble( outf, m_dStaticHeight );
	WriteDouble( outf, m_dSafetyPressValve );
	WriteDouble( outf, m_dSafetyTempLimiter );
	WriteDouble( outf, m_dMinTemperature );
	WriteDouble( outf, m_dMaxTemperature );
	WriteDouble( outf, m_dSupplyTemperature );
	WriteDouble( outf, m_dReturnTemperature );
	WriteDouble( outf, m_dFillTemperature );
	outf.write( (char *)&m_ePressOn, sizeof( m_ePressOn ) );
	WriteDouble( outf, m_dPumpHead );
	WriteDouble( outf, m_dPz );					// Version 9: NPSH becomes Pz.
	WriteDouble( outf, m_dMaxWidth );
	WriteDouble( outf, m_dMaxHeight );
	WriteDouble( outf, m_dDegassingMaxTempConnectPoint );
	WriteDouble( outf, m_dWaterMakeUpNetworkPN );
	WriteDouble( outf, m_dWaterMakeUpWaterTemp );

	// Write the 'PMWQSelectionPreferences' structure.
	m_clPMWQSelectionPreferences.Write( outf );

	// Write the system volume variables.
	m_clHeatGeneratorList.Write( outf );
	m_clHeatConsumersList.Write( outf );
	m_clHeatingPipeList.Write( outf );

	// Version 3.
	outf.write( (char *)&m_fCheckMinTemperature, sizeof( m_fCheckMinTemperature ) );

	// Version 4.
	outf.write( (char *)&m_dDegassingPressureConnectPoint, sizeof( m_dDegassingPressureConnectPoint ) );

	// Version 5.
	m_clColdGeneratorList.Write(outf);
	m_clColdConsumersList.Write(outf);

	// Version 7.
	WriteDouble( outf, m_dWaterMakeUpWaterHardness );

	// Version 8.
	m_clCoolingPipeList.Write( outf );

	// Version 9.
	outf.write( (char *)&m_iPzChecked, sizeof( m_iPzChecked ) );

	// Version 10. HYS-1083: 'm_iSafetyValveLocationChecked' and 'm_dSafetyValveLocation' added.
	outf.write( (char *)&m_iSafetyValveLocationChecked, sizeof( m_iSafetyValveLocationChecked ) );
	WriteDouble( outf, m_dSafetyValveLocation );

	// Version 11. HYS-1534: variables added for storage tank and solar collector.
	WriteDouble( outf, m_dSolarCollectorVolume );
	outf.write( (char *)&m_iSolarCollectorMultiplierFactorChecked, sizeof( m_iSolarCollectorMultiplierFactorChecked ) );
	WriteDouble( outf, m_dSolarCollectorMultiplierFactor );
	WriteDouble( outf, m_dStorageTankVolume );
	WriteDouble( outf, m_dStorageTankMaxTemp );
}

bool CPMInputUser::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPMINPUTUSER_VERSION;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CPMINPUTUSER_VERSION )
	{
		return false;
	}

	inpf.read( (char *)&m_eApplicationType, sizeof( m_eApplicationType ) );

	if( false == ReadString( inpf, m_strPressureMaintenanceTypeID ) )
	{
		return false;
	}

	if( Version < 6 ) 
	{
		int iDummy = 0;
		inpf.read( (char *)&iDummy, sizeof( iDummy ) );
	}
	else
	{
		if( false == ReadString( inpf, m_strWaterMakeUpTypeID ) )
		{
			return false;
		}
	}

	inpf.read( (char *)&m_iDegassingChecked, sizeof( m_iDegassingChecked ) );

	if( false == ReadString( inpf, m_strNormID ) )
	{
		return false;
	}

	// HYS-1583: "PM_NORM_SWKI" ID has been changed into "PM_NORM_SWKI_HE301_01" (HYS-1565) -> Need to check for old project !!!!
	if( Version < 12 )
	{
		if( 0 == m_strNormID.CompareNoCase( _T("PM_NORM_SWKI") ) )
		{
			m_strNormID = _T("PM_NORM_SWKI_HE301_01");
		}
	}

	m_dSystemVolume = ReadDouble( inpf );
	m_dSolarCollectorVolume = ReadDouble( inpf );
	m_dInstalledPower = ReadDouble( inpf );
	m_dStaticHeight = ReadDouble( inpf );
	m_dSafetyPressValve = ReadDouble( inpf );
	m_dSafetyTempLimiter = ReadDouble( inpf );
	m_dMinTemperature = ReadDouble( inpf );

	if( 2 <= Version )
	{
		m_dMaxTemperature = ReadDouble( inpf );
	}

	m_dSupplyTemperature = ReadDouble( inpf );
	m_dReturnTemperature = ReadDouble( inpf );
	m_dFillTemperature = ReadDouble( inpf );
	inpf.read( (char *)&m_ePressOn, sizeof( m_ePressOn ) );
	m_dPumpHead = ReadDouble( inpf );

	if( Version < 9 )
	{
		// In version < 9 it was the NPSH that we don't use anymore.
		double dNop = ReadDouble( inpf );
	}
	else
	{
		m_dPz = ReadDouble( inpf );
	}

	m_dMaxWidth = ReadDouble( inpf );
	m_dMaxHeight = ReadDouble( inpf );
	m_dDegassingMaxTempConnectPoint = ReadDouble( inpf );
	m_dWaterMakeUpNetworkPN = ReadDouble( inpf );
	m_dWaterMakeUpWaterTemp = ReadDouble( inpf );

	// Read the 'PMWQSelectionPreferences' structure.
	m_clPMWQSelectionPreferences.Read( inpf );

	// Read the system volume variables.
	m_clHeatGeneratorList.Read( inpf );
	m_clHeatConsumersList.Read( inpf );
	m_clHeatingPipeList.Read( inpf );

	// By default we copy 'm_clHeatingPipeList' in 'm_clCoolingPipeList'. Before version 8 we had only one container for
	// pipe.
	m_clCoolingPipeList.CopyFrom( &m_clHeatingPipeList );

	if( Version < 3 )
	{
		return true;
	}

	inpf.read( (char *)&m_fCheckMinTemperature, sizeof( m_fCheckMinTemperature ) );

	// In version 4.4.0.1 & 4.4.0.2 there was a bug about version. We added a new variable 'm_dDegassingPressureConnectPoint' but
	// we forgot to change version number from 3 to 4. Thus if we open a 4.4.0.1 or 4.4.0.2 tsp file, we must read 'm_dDegassingPressureConnectPoint'.
	// BUT in the 4.4.0.1 version there was a bug when writing version. Because the app name was changed from "TASelect4" into "HySelect".
	// But to recover the application version, we called 'CModuleVersion::GetFileVersionInfo' with "TaSelect4.exe". Thus 'GetAppVersionStr' will returns NULL.
	bool bReadDegPressConPt = false;

	if( 0 == TASApp.GetpTADS()->GetDBFileHeader()->GetAppVersionStr().Compare( _T( "" ) )
			&& 0 == TASApp.GetpTADS()->GetDBFileHeader()->GetAppName().Compare( _T("HySelect") ) )
	{
		// Version 4.4.0.1.
		bReadDegPressConPt = true;
	}
	else if( 0 == TASApp.GetpTADS()->GetDBFileHeader()->GetAppVersionStr().Compare( _T("4.4.0.2") ) )
	{
		// Version 4.4.0.2.
		bReadDegPressConPt = true;
	}
	else if( Version >= 4 )
	{
		bReadDegPressConPt = true;
	}

	if( true == bReadDegPressConPt )
	{
		inpf.read( (char *)&m_dDegassingPressureConnectPoint, sizeof( m_dDegassingPressureConnectPoint ) );
	}

	if( Version < 5 )
	{
		return true;
	}

	m_clColdGeneratorList.Read( inpf );
	m_clColdConsumersList.Read( inpf );

	if( Version < 7 )
	{
		return true;
	}

	m_dWaterMakeUpWaterHardness = ReadDouble( inpf );

	if( Version < 8 )
	{
		return true;
	}

	m_clCoolingPipeList.Read( inpf );

	if( Version < 9 )
	{
		return true;
	}

	inpf.read( (char *)&m_iPzChecked, sizeof( m_iPzChecked ) );

	if( Version < 10 )
	{
		return true;
	}

	// Version 10. HYS-1083: 'm_iSafetyValveLocationChecked' and 'm_dSafetyValveLocation' added.
	inpf.read( (char *)&m_iSafetyValveLocationChecked, sizeof( m_iSafetyValveLocationChecked ) );
	m_dSafetyValveLocation = ReadDouble( inpf );

	if( Version < 11 )
	{
		return true;
	}

	// Version 11. HYS-1534: variables added for storage tank and solar collector.
	m_dSolarCollectorVolume = ReadDouble( inpf );
	inpf.read( (char *)&m_iSolarCollectorMultiplierFactorChecked, sizeof( m_iSolarCollectorMultiplierFactorChecked ) );
	m_dSolarCollectorMultiplierFactor = ReadDouble( inpf );
	m_dStorageTankVolume = ReadDouble( inpf );
	m_dStorageTankMaxTemp = ReadDouble( inpf );

	return true;
}

double CPMInputUser::GetNominalVolumeInterVssl( double dVesselTempMin, double dVesselTempMax )
{
	// For heating/solar expansion factor is computed for the temperature range from the max admissible temperature for the vessel
	// to the return temperature.

	// For cooling expansion factor is computed for the temperature range from the minimal temperature that can be reached in the system to the 
	// minimum admissible temperature for the vessel (same remark as above).

	// HYS-1494: It is not depending of the fact that we are in heating/solar or cooling.
	// We need to verify what is the return temperature and minimum temperature.
	// If one of these temperatures (Or both) is below the vessel limit we need to compute the expansion from this temperature to the limit of the vessel.

	double dNominalVolumeReturn = 0.0;

	if( GetReturnTemperature() > dVesselTempMax )
	{
		double dExpansionFactor = m_WC.GetExpansionCoeff( dVesselTempMax, GetReturnTemperature() );

		// Expansion volume: Ve = e * Vs.
		double dExpandedVolume = GetSystemVolume() * dExpansionFactor;

		dNominalVolumeReturn = dExpandedVolume;

		if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
		{
			dNominalVolumeReturn += ( GetStorageTankVolume() * dExpansionFactor );
		}

		if( Solar == GetApplicationType() )
		{
			dNominalVolumeReturn += GetSolarCollectorSecurityVolume();
		}

		dNominalVolumeReturn += GetDegassingWaterReserve();
	}

	double dNominalVolumeMin = 0.0;

	if( GetMinTemperature() < dVesselTempMin )
	{
		double dExpansionFactor = m_WC.GetExpansionCoeff( GetMinTemperature(), dVesselTempMin );

		// Expansion volume: Ve = e * Vs.
		double dExpandedVolume = GetSystemVolume() * dExpansionFactor;

		dNominalVolumeMin = dExpandedVolume;

		if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
		{
			dNominalVolumeMin += ( GetStorageTankVolume() * dExpansionFactor );
		}

		if( Solar == GetApplicationType() )
		{
			dNominalVolumeMin += GetSolarCollectorSecurityVolume();
		}

		dNominalVolumeMin += GetDegassingWaterReserve();
	}

	return max( dNominalVolumeReturn, dNominalVolumeMin );
}

double CPMInputUser::GetMinimumInitialPressure()
{
	// GetMinimumPressure = p0.
	// HYS-1022: 0.3 bar margin for initial pressure (EN 12828-2014)
	return ( GetMinimumPressure() + m_pTADS->GetpTechParams()->GetInitialPressureMargin() );
}

double CPMInputUser::ComputeInitialPressure( double dTotalVesselVolume, double dWaterReserve )
{
	if( dWaterReserve >= dTotalVesselVolume )
	{
		return -1.0;
	}

	// Minimum pressure p0.
	double dMinimumPressureP0 = GetMinimumPressure();
	double dInitialPressure = ( dMinimumPressureP0 + 1e5 ) * dTotalVesselVolume;
	double dDenominator = dTotalVesselVolume - dWaterReserve;

	dDenominator -= GetDegassingWaterReserve();

	if( dDenominator > 0.0 )
	{
		dInitialPressure = ( dInitialPressure / dDenominator ) - 1e5;
	}
	else
	{
		dInitialPressure = -1.0;
	}

	if( dInitialPressure <= 0.0 )
	{
		dInitialPressure = -1.0;
		ASSERT( 0 );
	}

	return dInitialPressure;
}

double CPMInputUser::GetLowestPressure( double dWaterReserve, double dTotalVesselVolume )
{
	if( Cooling != GetApplicationType() )
	{
		return -1;
	}

	// We have an additional test to apply. Effectively minimum required pressure allow us to push a water reserver in the
	// vessel. In cooling, this is done at the filling temperature that is often above the supply and return temperatures.
	// But when the system starts, the temperature will decrease to reach the operating temperature. And in certain
	// circumstances this pressure can be lower than the limit that is p0 + 0.3 bar.
	double dTempStart = GetMinTemperature();
	double dTempEnd = GetFillTemperature();
	double dTempStep = ( dTempEnd - dTempStart ) / 100.0;
	double dTempIntermediate = dTempStart;
	double dLowerPressure = DBL_MAX;

	for( int iLoopTemp = 0; iLoopTemp <= 100; ++iLoopTemp )
	{
		double dIntermediatePressure = GetIntermediatePressure( dTempIntermediate, dWaterReserve, dTotalVesselVolume );

		if( dIntermediatePressure < dLowerPressure )
		{
			dLowerPressure = dIntermediatePressure;
		}

		dTempIntermediate += dTempStep;

		if( dTempIntermediate > dTempEnd )
		{
			dTempIntermediate = dTempEnd;
		}
	}

	return dLowerPressure;
}

double CPMInputUser::ComputeContractionVolume( void )
{
	// Compute contraction volume, difference between filling temperature and minimum temperature.
	// Remark: in the left panel we have the possibility to enable or not the 'Min. temperature'.
	//         If it is disabled, the minimum temperature is initialized with the supply temperature (the lower temperature).
	//         If is is enabled, the minimum temperature is initialized with the freezing point and user can change it.
	//         Thus, we can in both case use the minimum temperature.
	double dContr = GetContractionCoefficient();
	return ( GetSystemVolume() * dContr );
}

double CPMInputUser::GetContractionCoefficient()
{
	return m_WC.GetExpansionCoeff( GetFillTemperature(), GetMinTemperature() );
}

bool CPMInputUser::UsePressureVolumeLimit()
{
	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( false );
	}

	// HYS-1909 (HYS-1198): the pressure.volume limit when selected is applicable for all norms.
	return m_pTADS->GetpTechParams()->GetUseVesselPressureVolumeLimit();
}

double CPMInputUser::GetWaterReserve( double dTotalVesselVolume )
{
	// HYS-1022: 'Vwr' becomes 'Vwr,min' and 'Vwr,opt' becomes 'Vwr'.

	// Other norm than SWKI HE301-01:
	//   Net volume (Vn) = Ve,tot + VDK + Vv + Vwr,min.
	//   Nominal volume (VN) = Vn * pf
	//   Vessel volume >= VN
	//   Vwr,lav = Vvessel/pf - Ve,tot - VDK -Vv - Vwr,min.		-> Vwr,lav (Water reserve left available).
	//   Vwr,lav + Vwr,min = Vvessel/pf - Ve,tot - VDK - Vv		-> Vwr = Vwr,min + Vwr,lav
	//
	// SWKI HE301-01 norm:
	//   Net volume (Vn) = Ve,tot + VDK + Vv -> "Vwr,min" is already included in "Ve,tot" because the X factor (Ve,tot = Vs.e.X + Ve,sto)
	//   Nominal volume (VN) = Vn * pf
	//   Vessel volume >= VN
	//   Vwr,lav = Vvessel/pf - Ve,tot - VDK - Vv
	//   Vwr,lav + Vwr,min = Vvessel/pf - Ve,tot - VDK - Vv + Vwr,min
	//   Vwr = Vvessel/pf - Ve,tot - VDK - Vv + Vwr,min

	double dVwr = ( dTotalVesselVolume / GetPressureFactor() ) - GetTotalExpansionVolume() - GetSolarCollectorSecurityVolume() - GetDegassingWaterReserve();

	// If we are in the SWKI HE301-01 (See notes above).
	if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		dVwr += GetMinimumWaterReserve();
	}

	// Remark: For the cooling, Ve at the tmax temperature will include the Vctr.
	//
    //     ////////////            ////////////        ////////////        ////////////
	//     /          /            /          /	       /          /	       /          /
	//     /          /            /          /	       /          /	       /          /
	//     /          /            /          /	       /          /	       /----------/
	//     /----------/            /          /	       /          /	       /          /
	//     /   Vctr   /      --->  /----------/	 --->  /          /	 --->  /   Ve     /
	//     /          /            /   Vctr   /	       /          /	       /          /
	//     /----------/            /----------/	       /----------/	       /----------/
	//     /   Vwr    /            /   Vwr    /	       /   Vwr    /	       /   Vwr    /
	//     /----------/            /----------/	       /----------/	       /----------/
	//     /    Vv    /            /    Vv    /	       /    Vv    /	       /    Vv    /
	//     ////////////            ////////////	       ////////////	       ////////////
	//
	//     pa (tfill = 30°C)       p (ts = 6°C)       p (tmin = -24°C)     p (tmax = 45°C)

	return dVwr;
}

double CPMInputUser::GetWaterReservePushed( double dTotalVesselVolume, double dPressure )
{
	double dVwr = ( dPressure - GetMinimumPressure() ) * dTotalVesselVolume;
	dVwr /= ( dPressure + 1e5 );
	dVwr -= GetDegassingWaterReserve();

	return dVwr;
}

double CPMInputUser::GetIntermediatePressure( double dIntermediateTemperature, double dWaterReserve, double dTotalVesselVolume )
{
	double dSystemIntermediateExpansionCoefficient = 0.0;

	// HYS-1104: To draw curves, we take only the EN12828 norm for the expansion between the min. temperature and the intermediate temperature.
	dSystemIntermediateExpansionCoefficient = m_WC.GetExpansionCoeff( GetMinTemperature(), dIntermediateTemperature );

	// System expansion volume [Ve] = e * Vs.
	double dIntermediateExpansionVolume = GetSystemVolume() * dSystemIntermediateExpansionCoefficient;

	// HYS-1534: pay attention, in SWKI HE301-01 norm, the system volume doesn't contain storage tank volume if defined.
	if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) && GetStorageTankVolume() > 0.0 )
	{
		dIntermediateExpansionVolume += ( GetStorageTankVolume() * dSystemIntermediateExpansionCoefficient );
	}

	double dIntermediatePressure = -1.0;
	double dVentoReserve = GetDegassingWaterReserve();

	double dDenominator = dTotalVesselVolume - dWaterReserve - dVentoReserve - dIntermediateExpansionVolume;

	// Stop as soon as the denominator is lower or equal to 0. Effectively, if water reserve and expanded volume is bigger than
	// the vessel volume, it's not possible!!
	if( dDenominator > 0.0 )
	{	
		double dMinimumPressureP0 = GetMinimumPressure();
		dIntermediatePressure = ( dMinimumPressureP0 + 1e5 ) * dTotalVesselVolume;
		dIntermediatePressure /= dDenominator;
		dIntermediatePressure -= 1e5;
	}

	return dIntermediatePressure;
}

double CPMInputUser::GetPressureFactor( bool bForTecBox )
{
	double dVesselPressureFactor = 0.0;

	if( true == bForTecBox )
	{
		dVesselPressureFactor = 1.1;
	}
	else
	{
		if( true == IsNorm( ProjectType::Heating, PressurisationNorm::PN_DM11275 ) )
		{
			dVesselPressureFactor = ( GetFinalPressure() + 1e5 ) / ( GetFinalPressure() - ( GetMinimumPressure() + 3e4 ) );
		}
		else
		{
			dVesselPressureFactor = ( GetFinalPressure() + 1e5 ) / ( GetFinalPressure() - GetMinimumPressure() );
		}
	}

	return dVesselPressureFactor;
}

CPMInputUser::MaintenanceType CPMInputUser::GetPressureMaintenanceType()
{
	MaintenanceType eMaintenanceType = MT_Unknown;

	if( 0 != m_mapPMTypeMatchIDEnum.count( m_strPressureMaintenanceTypeID ) )
	{
		eMaintenanceType = m_mapPMTypeMatchIDEnum[m_strPressureMaintenanceTypeID];
	}

	return eMaintenanceType;
}

double CPMInputUser::GetDegassingWaterReserve() const
{
	double dDegassingWaterReserve = 0.0;

	if( NULL != m_pTADS && NULL != m_pTADS->GetpTechParams() && BST_CHECKED == GetDegassingChecked() )
	{
		dDegassingWaterReserve = m_pTADS->GetpTechParams()->GetDegassingWaterRerserveForVessel();
	}

	return dDegassingWaterReserve;
}

PressurisationNorm CPMInputUser::GetNorm() const
{
	PressurisationNorm ePMNorm = PN_None;

	if( 0 == GetNormID().Compare( _T("PM_NORM_EN12828") ) )
	{
		ePMNorm = PN_EN12828;
	}
	else if( 0 == GetNormID().Compare( _T("PM_NORM_EN12953") ) )
	{
		ePMNorm = PN_EN12953;
	}
	else if( 0 == GetNormID().Compare( _T("PM_NORM_SWKI_HE301_01") ) )
	{
		ePMNorm = PN_SWKIHE301_01;
	}
	else if( 0 == GetNormID().Compare( _T("PM_NORM_DM11275") ) )
	{
		ePMNorm = PN_DM11275;
	}
	else if( 0 == GetNormID().Compare( _T("PM_NORM_NONE") ) )
	{
		ePMNorm = PN_None;
	}

	return ePMNorm;
}

bool CPMInputUser::IsNorm( ProjectType eProjectType, PressurisationNorm ePMNorm )
{
	bool bReturn = ( ePMNorm == GetNorm() ) ? true : false;
	
	if( ProjectType::All != eProjectType )
	{
		bReturn &= ( eProjectType == GetApplicationType() ) ? true : false;
	}
	
	return bReturn;
}

double CPMInputUser::GetSystemVolume()
{
	double dSystemVolume = m_dSystemVolume;

	if( Heating == m_eApplicationType && true == IfSysVolExtDefExist() )
	{
		dSystemVolume = m_clHeatGeneratorList.GetTotalContent();
		dSystemVolume += m_clHeatConsumersList.GetTotalContent( GetSupplyTemperature(), GetReturnTemperature() );
		dSystemVolume += m_clHeatingPipeList.GetTotalContent();
	}
	else if( Cooling == m_eApplicationType && true == IfSysVolExtDefExist() )
	{
		dSystemVolume = m_clColdGeneratorList.GetTotalContent();
		dSystemVolume += m_clColdConsumersList.GetTotalContent( GetSupplyTemperature(), GetReturnTemperature() );
		dSystemVolume += m_clCoolingPipeList.GetTotalContent();
	}

	return dSystemVolume;
}

double CPMInputUser::GetSolarCollectorMultiplierFactor()
{
	// Default value.
	double dSolarCollectorMultiplierFactor = 1.1;

	if( true == IsNorm( ProjectType::Solar, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// In the SWKI HE301-01 norm, the user can change the multiplier factor.
		dSolarCollectorMultiplierFactor = m_dSolarCollectorMultiplierFactor;
	}

	return dSolarCollectorMultiplierFactor;
}

double CPMInputUser::GetInstalledPower()
{
	double dInstalledPower = m_dInstalledPower;

	if( Heating == m_eApplicationType && 0 != m_clHeatGeneratorList.GetTotalHeatCapacity() )
	{
		dInstalledPower = m_clHeatGeneratorList.GetTotalHeatCapacity();
	}
	else if( Cooling == m_eApplicationType && 0 != m_clColdGeneratorList.GetTotalColdCapacity() )
	{
		dInstalledPower = m_clColdGeneratorList.GetTotalColdCapacity();
	}

	return dInstalledPower;
}

double CPMInputUser::GetXFactorSWKI()
{
	double dXFactorSWKI = 0.0;

	if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		if( GetInstalledPower() <= 10000.0 )
		{
			dXFactorSWKI = 3.0;
		}
		else if( GetInstalledPower() > 10000.0 && GetInstalledPower() < 150000.0 )
		{
			dXFactorSWKI = ( 87000.0 - ( 0.3 * GetInstalledPower() ) ) / 28000.0;
		}
		else if( GetInstalledPower() >= 150000.0 )
		{
			dXFactorSWKI = 1.5;
		}
	}

	return dXFactorSWKI;
}

double CPMInputUser::GetSystemExpansionCoefficient()
{
	double dExpansionCoefficient = 0.0;

	// Expansion factor [e]
	if( true == IsNorm( ProjectType::Heating, PressurisationNorm::PN_DM11275 ) )
	{
		double n = 0.31 + 0.00039 * GetSafetyTempLimiter() * GetSafetyTempLimiter();
		dExpansionCoefficient = n / 100.0;
	}
	else
	{
		// For heating/solar, expansion coefficient is computed for the temperature range from the min. temperature (the admissible minimum temperature that
		// the system can reach when no more heat generators run) to the supply temperature.
		// For cooling, expansion coefficient is computed for the temperature range from the min. temperature to the max. temperature (the admissible maximum
		// temperature that the system can reach when no more cooling units run).
		if( ProjectType::Heating == GetApplicationType() || ProjectType::Solar == GetApplicationType() )
		{
			double dTemperatureForExpansionCoefficient = GetSupplyTemperature();		//EN12828; EN12953

			if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
			{
				dTemperatureForExpansionCoefficient -= ( ( GetSupplyTemperature() - GetReturnTemperature() ) / 2 );	// SWKI HE301-01
			}

			if( true == IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12953 ) && dTemperatureForExpansionCoefficient > 105 )
			{
				dExpansionCoefficient = ( dTemperatureForExpansionCoefficient * dTemperatureForExpansionCoefficient * 0.00039 + 0.31 ) / 100.0;
			}
			else
			{
				dExpansionCoefficient = m_WC.GetExpansionCoeff( GetMinTemperature(), dTemperatureForExpansionCoefficient );
			}
		}
		else
		{
			if( true == IsNorm( ProjectType::Cooling, PressurisationNorm::PN_SWKIHE301_01 ) )
			{
				// HYS-1534: In cooling, where the max. temperature will not exceed 40 °C, the coefficient e is determined 
				// between the lowest possible temperature and 40 °C (standstill of the system). If above 40 °C we take the Tmax.
				dExpansionCoefficient = m_WC.GetExpansionCoeff( max( 40.0, GetMaxTemperature() ), GetMinTemperature() );
			}
			else
			{
				dExpansionCoefficient = m_WC.GetExpansionCoeff( GetMaxTemperature(), GetMinTemperature() );
			}
		}
	}

	return dExpansionCoefficient;
}

double CPMInputUser::GetSystemExpansionVolume()
{
	double dSystemExpansionVolume = GetSystemVolume() * GetSystemExpansionCoefficient();

	if( true == IsNorm( ProjectType::All, PN_SWKIHE301_01 ) )
	{
		// Expansion volume [Ve] = e * Vs * X.
		dSystemExpansionVolume *= GetXFactorSWKI();
	}

	return dSystemExpansionVolume;
}

double CPMInputUser::GetSolarCollectorSecurityVolume()
{
	return ( GetSolarCollectorVolume() * GetSolarCollectorMultiplierFactor() );
}

double CPMInputUser::GetStorageTankExpansionCoefficient()
{
	double dStorageExpansionCoefficient = 0.0;

	if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		dStorageExpansionCoefficient = m_WC.GetExpansionCoeff( GetStorageTankMaxTemp(), GetMinTemperature() );
	}

	return dStorageExpansionCoefficient;
}

double CPMInputUser::GetStorageTankExpansionVolume()
{
	double dStorageExansionVolume = 0.0;

	if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// Ve,sto = Vsto * esto.
		dStorageExansionVolume = GetStorageTankVolume() * GetStorageTankExpansionCoefficient();
	}

	return dStorageExansionVolume;
}

double CPMInputUser::GetTotalExpansionVolume()
{
	double dTotalExansionVolume = GetSystemExpansionVolume();

	if( PressurisationNorm::PN_SWKIHE301_01 == GetNorm() )
	{
		// In SWKI HE301-01, user can define a storage tank volume that must be taken into account for the expansion.
		dTotalExansionVolume += GetStorageTankExpansionVolume();
	}

	return dTotalExansionVolume;
}

double CPMInputUser::GetVesselNetVolume()
{
	double dVesselNetVolume = GetTotalExpansionVolume();

	if( ProjectType::Solar == GetApplicationType() )
	{
		dVesselNetVolume += GetSolarCollectorSecurityVolume();
	}

	if( PressurisationNorm::PN_SWKIHE301_01 != GetNorm() )
	{
		// Water reserve in SWKI HE301-01 is included in the expansion volume (See the 'GetSystemExpansionVolume' method).
		dVesselNetVolume += GetMinimumWaterReserve();
	}

	dVesselNetVolume += GetDegassingWaterReserve();

	return dVesselNetVolume;
}

double CPMInputUser::GetNominalVolumeForPaEN12828( double dTotalVesselVolume )
{
	// See the "PM - Computing required nominal volume to satisfy EN12828 norm.docx" document in the doc folder of the HySelect solution.
	double dVwrEN = ( 3e4 * dTotalVesselVolume ) / ( GetMinimumPressure() + 1.3e5 );

	// Note here we don't call 'GetTotalExpansionVolume' because this method will already return the minimum water reserve and reserve for degassing if exist
	// and we don't need them in this formula.
	double dVNreq = ( GetSystemVolume() * GetSystemExpansionCoefficient() ) + dVwrEN;

	if( Solar == GetApplicationType() )
	{
		dVNreq += GetSolarCollectorSecurityVolume();
	}

	dVNreq *= GetPressureFactor();
	return dVNreq;
}

double CPMInputUser::GetMinimumPressure( bool bVerifyPz )
{
	double dMinimumPressurep0 = 0.0;

	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( 0.0 );
	}
	
	double dPst = 0.0;

	// HYS-1126: Use simplified calculation for pst as Hst/10 when it is defined in tech param
	if( true == m_pTADS->GetpTechParams()->GetUseSimplyPst() )
	{
		dPst = m_dStaticHeight / 10;
		
		// Convert in Pa
		dPst = dPst * 1.0e5;
	}
	else
	{
		// HYS-1022: We take now the media density at tmin instead of ts.
		dPst = m_WC.GetDens( m_dMinTemperature ) * 9.81 * m_dStaticHeight;
	}
	
	// Minimum pressure p0.
	dMinimumPressurep0 = ( PressurON::poPumpDischarge == GetPressOn() ) ? m_dPumpHead : 0.0;

	// HYS-1407 : SWIKI norm min. pressure margin is 0.3 bar
	if( false == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// Minimum pressure margin is 0.2 bar (Margin over static height as per EN 12828-2014).
		dMinimumPressurep0 += dPst + GetVaporPressure() + m_pTADS->GetpTechParams()->GetMinPressureMargin();
	}
	else
	{
		// Minimum pressure margin is 0.3 bar.
		dMinimumPressurep0 += dPst + GetVaporPressure() + 3e+4;
	}
	if( true == bVerifyPz && BST_CHECKED == m_iPzChecked )
	{
		dMinimumPressurep0 = max( dMinimumPressurep0, GetPz() );
	}

	return dMinimumPressurep0;
}

double CPMInputUser::GetMinimumWaterReserve()
{
	double dMinimumWaterReserve = 0.0;

	// Water reserve.
	if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// Even if we don't display Vwr, we need to initialize it to compute the required minimum pressure (pa).
		// Vwr = (X-1) * e * Vs
		dMinimumWaterReserve = ( GetXFactorSWKI() - 1.0 ) * GetSystemExpansionCoefficient() * GetSystemVolume();
	}
	else
	{
		// Vwr = max(5e-3 * Vs , 3e-3).
		dMinimumWaterReserve = max( 5e-3 * GetSystemVolume(), 3e-3 );
	}

	return dMinimumWaterReserve;
}

double CPMInputUser::GetVesselNominalVolume( bool bForTecBox )
{
	// HYS-1083: Note for the DM 1.12.75 norm
	// Nominal volume is computed with Vn >= Ve / ( 1 - P1/P2 )
		
	// P1 = absolute pressure in bar, to which the gas cushion is preloaded, pressure which can't be lower at the hydrostatic pressure 
	// at the point where the vessel is installed (or at the reintegration pressure of the group of filling). This initial pressure value 
	// can't be lower than 1.5 bar.

	// P2 = absolute pressure setting of the safety valve, in bar, decreased by an amount corresponding to height difference between
	// the expansion vessel and the safety valve, if the latter is placed lower or increased if placed higher.
	// P2 = pSV – (Hvessel – Hpsv)*rho*g if Hpsv < Hvessel or P2 = pSV + (Hvessel – Hpsv)*rho*g

	// Vn > Ve / ( 1 - p0 / pe ) ) > Ve / ( pe - p0 ) / pe > ( pe / ( pe - p0 ) ) * Ve > pf * Ve.
	// Thus it's the same as for EN 12828.

	return GetVesselNetVolume() * GetPressureFactor( bForTecBox );
}

double CPMInputUser::GetVaporPressure()
{
	return m_WC.GetVaporPressure( GetSafetyTempLimiter() );
}

double CPMInputUser::GetEqualizingVolumetricFlow()
{
	// dVD compensation flow [l/kWh]
	//double dVD = 0.0058 * ( GetMaxTemperature() + GetMinTemperature() ) / 2 + 0.094;
	double dVD = 0.0058 * GetSupplyTemperature() + 0.094;
	dVD = max( 0.384, dVD );
	dVD = dVD / 3.6e9; // [m3/Ws]
	return dVD;
}

double CPMInputUser::GetVD()
{
	return ( GetEqualizingVolumetricFlow() * GetInstalledPower() );
}

CPipeList *CPMInputUser::GetPipeList()
{
	CPipeList *pclPipeList;

	if( Heating == m_eApplicationType || Solar == m_eApplicationType )
	{
		pclPipeList = &m_clHeatingPipeList;
	}
	else
	{
		pclPipeList = &m_clCoolingPipeList;
	}

	return pclPipeList;
}


double CPMInputUser::GetTotalHeatWaterContent()
{
	double dTotalContent = m_clHeatGeneratorList.GetTotalContent();
	dTotalContent += m_clHeatConsumersList.GetTotalContent( m_dSupplyTemperature, m_dReturnTemperature );
	dTotalContent += m_clHeatingPipeList.GetTotalContent();

	return dTotalContent;
}

double CPMInputUser::GetTotalHeatInstalledPower()
{
	return m_clHeatGeneratorList.GetTotalHeatCapacity();
}

double CPMInputUser::GetTotalColdWaterContent()
{
	double dTotalContent = m_clColdGeneratorList.GetTotalContent();
	dTotalContent += m_clColdConsumersList.GetTotalContent( m_dSupplyTemperature, m_dReturnTemperature );
	dTotalContent += m_clCoolingPipeList.GetTotalContent();

	return dTotalContent;
}

double CPMInputUser::GetTotalColdInstalledPower()
{
	return m_clColdGeneratorList.GetTotalColdCapacity();
}

bool CPMInputUser::CheckPSV( double *pdPSVLimit, double dCurrentPSV )
{
	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL != pdPSVLimit )
	{
		*pdPSVLimit = 0.0;
	}
	double dPSVLimit = 0.0;

	if( MT_ExpansionVessel == GetPressureMaintenanceType() || MT_ExpansionVesselWithMembrane == GetPressureMaintenanceType() )
	{
		dPSVLimit = GetMinimumRequiredPSVRounded(MT_ExpansionVessel);
	}
	else if( MT_WithCompressor == GetPressureMaintenanceType() )
	{
		dPSVLimit = GetMinimumRequiredPSVRounded(MT_WithCompressor);
	}
	else if( MT_WithPump == GetPressureMaintenanceType() )
	{
		dPSVLimit = GetMinimumRequiredPSVRounded(MT_WithPump);
	}
	else if ( MT_All == GetPressureMaintenanceType() )
	{
		dPSVLimit = (GetMinimumRequiredPSVRounded(MT_ExpansionVessel) > GetMinimumRequiredPSVRounded(MT_WithCompressor)) ? GetMinimumRequiredPSVRounded(MT_WithCompressor) : GetMinimumRequiredPSVRounded(MT_ExpansionVessel);
		
		if (dPSVLimit > GetMinimumRequiredPSVRounded(MT_WithPump))
		{
			dPSVLimit = GetMinimumRequiredPSVRounded(MT_WithPump);
		}
	}

	if( NULL != pdPSVLimit )
	{
		*pdPSVLimit = dPSVLimit;
	}


	bool bPSVError = false;

	if( -1.0 == dCurrentPSV )
	{
		bPSVError = ( GetSafetyValveResponsePressure() < dPSVLimit ) ? false : true;
	}
	else
	{
		bPSVError = ( dCurrentPSV < dPSVLimit ) ? false : true;
	}

	return bPSVError;
}

double CPMInputUser::GetMinimumRequiredPSVRounded( MaintenanceType eMaintenanceType )
{
	double dMinPSV = GetMinimumRequiredPSVRaw( eMaintenanceType );
	
	// HYS-662: we must round dMinPSV to the upper limit to have the same behavior between
	// error tooltip (leftTab, computedData sheet) and error message (after click on Suggest)

	// We force min PSV to be accurate with two decimals. The method used is:
	//  minPSV  | minPSV | (A)-(B) | (C)*100 |  (D)  | (D)-(E) |
	// (double) | (int)  |   (C)   |   (D)   | (int) |         |
	//   (A)    |  (B)   |         |         |  (E)  |         |
	// ---------+--------+---------+---------+-------+---------+
	// 5,01     | 5      | 0,01    | 1,00    | 1     | 0       | =0  -> no change
	// 5,19     | 5      | 0,19    | 19,00   | 19    | 0       | =0  -> no change
	// 5,144    | 5      | 0,144   | 14,40   | 14    | 0,40    | <>0 -> dMinPSV = (B) + ( (E) + 1 ) / 100
	
	double dMinPSVCU = CDimValue::SItoCU( _U_PRESSURE, dMinPSV );
	int iMinPSV_B = (int)dMinPSVCU;
	double dDiff_C = dMinPSVCU - (double)iMinPSV_B;
	double dMul100_D = dDiff_C * 100.0;
	int iMul100_E = (int)dMul100_D;

	if( dMul100_D - (double)iMul100_E > 0.0 )
	{
		double dNewMinPSVCU = (double)iMinPSV_B + ( (double)iMul100_E + 1.0 )  / 100.0;
		dMinPSV = CDimValue::CUtoSI(_U_PRESSURE, dNewMinPSVCU );
	}

	return dMinPSV;
}

double CPMInputUser::GetMinimumRequiredPSVRaw( MaintenanceType eMaintenanceType )
{
	double dMinPressureP0 = GetMinimumPressure();
	double dMinPSV = dMinPressureP0;

	switch(eMaintenanceType)
	{
		case MT_ExpansionVessel:

			// These verifications are done in regards to the 'Pflichtenheft_offline_Algorithmus_10620.xlsx' documentation in tab 'Data Versions'.
			if( true == IsNorm( ProjectType::Heating, PressurisationNorm::PN_SWKIHE301_01 ) )
			{
				if( GetInstalledPower() <= 4.0e6 )
				{
					dMinPSV = ( dMinPressureP0 + 5.0e4 ) * 1.3;
				}
				else
				{
					dMinPSV = ( dMinPressureP0 + 9.0e4 ) * 1.3;
				}
			}
			else if( ProjectType::Heating == GetApplicationType() && ( true == IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12828 ) || true == IsNorm( ProjectType::Heating, PressurisationNorm::PN_DM11275 ) ) )
			{
				if( GetInstalledPower() <= 4.0e6 )
				{
					if( dMinPressureP0 > 4.0e5 )
					{
						dMinPSV = ( dMinPressureP0 + 5.0e4 ) / 9.0e4 * 1e5;
					}
					else
					{
						dMinPSV = dMinPressureP0 + 1.0e5;
					}
				}
				else
				{
					if( dMinPressureP0 > 3.8e5 )
					{
						dMinPSV = ( dMinPressureP0 + 7.0e4 ) / 9.0e4 * 1e5;
					}
					else
					{
						dMinPSV = dMinPressureP0 + 1.2e5;
					}
				}
			}
			else
			{
				// No norm ... old code.
				double dpe = dMinPressureP0 + 5.0e+4;

				// pe = PSV - max( 0.1*PSV; 0.5). the minimum value is thus 0.5 => PSV >= pe + 0.5 (1)
				// pe = PSV - 0.1*PSV = 0.9 * PSV => PSV = pe / 0.9 (2)
				// (1) & (2) => pe/0.9 >= pe + 0.5 => pe/0.9 - pe >= 0.5
				double dASV = max( (dpe / 0.9) - dpe, 50000);
				dMinPSV = dpe + dASV;
			}

			break;

		case MT_WithCompressor:
			dMinPSV = max( dMinPressureP0 + 1e5, ( dMinPressureP0 + 5e4 ) / 0.9 );
			break;

		case MT_WithPump:
			dMinPSV = max( dMinPressureP0 + 1.2e5, ( dMinPressureP0 + 7e4 ) / 0.9 );
			break;
	}

	return dMinPSV;
}

double CPMInputUser::GetTargetPressureForTecBox( CDB_TecBox::TecBoxType TbT )
{
	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	// Remark: Target pressure is the pressure just in the middle of the initial pressure and the final pressure.
	//         The tolerances of pressure control of Compresso (+/-0.1bar) and Transfero (+/-0.2bar) are set in the technical parameters in
	//         the local DB.

	double dTargetPressure = GetMinimumInitialPressure();

	if( CDB_TecBox::etbtCompresso == TbT )
	{
		dTargetPressure += m_pTADS->GetpTechParams()->GetPSetupCompresso();
	}
	else if( CDB_TecBox::etbtTransfero == TbT || CDB_TecBox::etbtTransferoTI == TbT )
	{
		dTargetPressure += m_pTADS->GetpTechParams()->GetPSetupTransfero();
	}
	else
	{
		ASSERT( 0 );
	}

	return dTargetPressure;
}

double CPMInputUser::GetFinalPressure()
{
	double dFinalPressure = 0.0;

	// HYS-1083: If the safety valve is not located at the place as the vessel, we must take into account
	// the difference. See the 'PM - Max height between vessel and pSV.docx' documentation for more details.
	double dP2 = GetSafetyValveResponsePressure();

	if( BST_CHECKED == m_iSafetyValveLocationChecked )
	{
		dP2 += m_dSafetyValveLocation * m_WC.GetDens( GetSafetyTempLimiter() ) * 9.81;
	}

	// Final Pressure pe.
	if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// HYS-1534.
		if( GetSafetyValveResponsePressure() <= 300000.0 )
		{
			dFinalPressure = dP2 / 1.3;
		}
		else
		{
			dFinalPressure = dP2 / 1.15;
		}
	}
	else if( true == IsNorm( Heating, PressurisationNorm::PN_DM11275 ) )
	{
		dFinalPressure = dP2;
	}
	else
	{
		dFinalPressure = dP2 - max( 0.1 * dP2, 5e4 );
	}

	return dFinalPressure;
}

double CPMInputUser::GetFinalPressure( CDB_TecBox::TecBoxType TbT )
{
	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dFinalPressure = GetTargetPressureForTecBox( TbT );

	if( CDB_TecBox::etbtCompresso == TbT )
	{
		dFinalPressure += m_pTADS->GetpTechParams()->GetPSetupCompresso();
	}
	else if( CDB_TecBox::etbtTransfero == TbT || CDB_TecBox::etbtTransferoTI == TbT )
	{
		dFinalPressure += m_pTADS->GetpTechParams()->GetPSetupTransfero();
	}
	else
	{
		ASSERT( 0 );
	}

	return dFinalPressure;
}

double CPMInputUser::GetMaxWeight( double dVesselWeight, double dVesselVolume, double dMinTemperature )
{
	CDB_AdditCharacteristic *pclAdditCharacteristic = m_WC.GetpAdditChar();
	ASSERT( NULL != pclAdditCharacteristic );
	
	if( NULL == pclAdditCharacteristic )
	{
		return -1.0;
	}

	double dDensity = 0.0;
	double dKinVisc = 0.0;
	double dSpecifHeat = 0.0;
	double dVaporPressure = 0.0;
	pclAdditCharacteristic->GetAdditProp( m_WC.GetPcWeight(), dMinTemperature, &dDensity, &dKinVisc, &dSpecifHeat, &dVaporPressure );

	double dMaxWeight = dVesselWeight + dDensity * dVesselVolume;
	return dMaxWeight;
}

double CPMInputUser::GetTotalHardnessOfSystem( void )
{
	if( NULL == m_pTADB )
	{
		ASSERTA_RETURN( -1.0 );
	}

	if( ProjectType::Heating != m_eApplicationType )
	{
		return -1.0;
	}

	CDB_TotalHardnessCharacteristic *pclTHC = dynamic_cast<CDB_TotalHardnessCharacteristic *>( m_pTADB->Get( _T("THCHAR_VDI2014_B1") ).MP );

	if( NULL == pclTHC || m_dSystemVolume <= 0.0 || m_dInstalledPower <= 0.0 )
	{
		return -1.0;
	}

	double dSpecificVolume = m_dSystemVolume;
	double dLowerPower = m_dInstalledPower;

	if( m_clHeatGeneratorList.GetHeatGeneratorCount() > 0 )
	{
		for( int iLoopGen = 0; iLoopGen < m_clHeatGeneratorList.GetHeatGeneratorCount(); iLoopGen++ )
		{
			CHeatGeneratorList::HeatGeneratorSave *pHeatGenerator = m_clHeatGeneratorList.GetHeatGenerator( iLoopGen );

			if( NULL == pHeatGenerator )
			{
				continue;
			}

			if( pHeatGenerator->m_dHeatCapacity < dLowerPower )
			{
				dLowerPower = pHeatGenerator->m_dHeatCapacity;
			}
		}
	}

	dSpecificVolume /= dLowerPower;

	return pclTHC->GetTotalHardnessOfSystem( m_dInstalledPower, dSpecificVolume );
}

void CPMInputUser::SetpTADS( CTADatastruct *pTADS )
{ 
	m_pTADS = pTADS;
	m_clColdConsumersList.SetpTADS( pTADS );
}

bool CPMInputUser::CheckInputData( std::vector<std::pair<int, int>> *pveciRetCode )
{
	if( NULL == pveciRetCode )
	{
		return false;
	}

	if( true == IsPressurisationSystemExist() )
	{
		// Verify safety pressure valve.
		// These verifications are done in regards to the 'Pflichtenheft_offline_Algorithmus_10620.xlsx' documentation.
		if( false == CheckPSV( NULL ) )
		{
			pveciRetCode->push_back( std::pair<int, int>( ePM_InvalidPSV, 0 ) );
		}

		// Power.
		/*
		if( true == IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) && GetInstalledPower() <= 0 )
		{
			pveciRetCode->push_back( std::pair<int, int>( ePM_InputEmpty, RACIE_InstalledPower ) );
		}
		*/

		if( PressurON::poPumpDischarge == GetPressOn() && m_dPumpHead <= 0.0 )
		{
			pveciRetCode->push_back( std::pair<int, int>( ePM_InputEmpty, RACIE_PumpHead ) );
		}

		// Check if fields are well filled.
		if( m_dSystemVolume <= 0.0 )
		{
			pveciRetCode->push_back( std::pair<int, int>( ePM_InputEmpty, RACIE_WaterContent ) );
		}

		if( ProjectType::Solar == m_eApplicationType && m_dSolarCollectorVolume <= 0.0 )
		{
			pveciRetCode->push_back( std::pair<int, int>( ePM_InputEmpty, RACIE_SolarContent ) );
		}
	}
	else
	{
		if( BST_CHECKED == m_iDegassingChecked && m_dSystemVolume <= 0.0 )
		{
			pveciRetCode->push_back( std::pair<int, int>( ePM_InputEmpty, RACIE_WaterContent ) );
		}
	}

	bool bWaterMakeUpAvailable = ( 0 != CString( m_strWaterMakeUpTypeID ).Compare( _T("WMUP_TYPE_NONE") ) ) ? true : false;
	
	if( true == bWaterMakeUpAvailable && m_dWaterMakeUpNetworkPN <= 0.0 )
	{
		pveciRetCode->push_back( std::pair<int, int>( ePM_InputEmpty, RACIE_WaterMakeUpPSN ) );
	}

	return ( 0 == ( int )pveciRetCode->size() ) ? true : false;
}

void CPMInputUser::SetSupplyTemperature( double dSupplyTemperature )
{ 
	m_dSupplyTemperature = dSupplyTemperature;

	// Also update the water char.
	m_WC.SetTemp( m_dSupplyTemperature );
	m_WC.UpdateFluidData( m_dSupplyTemperature );

	// Update this temperature for the generators and consumers.
	switch( m_eApplicationType )
	{
		case ProjectType::Heating:
			m_clHeatGeneratorList.SetSupplyTemperature( m_dSupplyTemperature );
			m_clHeatConsumersList.SetSupplyTemperature( m_dSupplyTemperature );

		case ProjectType::Cooling:
			m_clColdGeneratorList.SetSupplyTemperature( m_dSupplyTemperature );
			m_clColdConsumersList.SetSupplyTemperature( m_dSupplyTemperature );
			break;
	}
}

void CPMInputUser::SetReturnTemperature( double dReturnTemperature )
{ 
	m_dReturnTemperature = dReturnTemperature;


	// Update this temperature for the generators and consumers.
	switch( m_eApplicationType )
	{
		case ProjectType::Heating:
			// HYS-1022: The Vento is connected on the return.
			// HYS-1201: In Cooling mode the m_dDegassingMaxTempConnectPoint is max temp.
			m_dDegassingMaxTempConnectPoint = dReturnTemperature;
			m_clHeatGeneratorList.SetReturnTemperature( m_dReturnTemperature );
			m_clHeatConsumersList.SetReturnTemperature( m_dReturnTemperature );

		case ProjectType::Cooling:
			m_clColdGeneratorList.SetReturnTemperature( m_dReturnTemperature );
			m_clColdConsumersList.SetReturnTemperature( m_dReturnTemperature );
			break;
	}
}

void CPMInputUser::SetMaxTemperature( double dMaxTemperature )
{ 
	m_dMaxTemperature = dMaxTemperature;

	// HYS-1201: In Cooling mode the m_dDegassingMaxTempConnectPoint is max temp.
	m_dDegassingMaxTempConnectPoint = dMaxTemperature;

	// Update this temperature for the cooling generators.
	m_clColdGeneratorList.SetTempOutdoor( dMaxTemperature );
	m_clColdConsumersList.SetMaxAirInputTempForAHU( dMaxTemperature );
}

void CPMInputUser::SetPMWQSelectionPreferences( CPMWQPrefs *pclPMWQSelectionPreferences )
{
	if( NULL == pclPMWQSelectionPreferences )
	{
		return;
	}

	m_clPMWQSelectionPreferences.CopyFrom( pclPMWQSelectionPreferences );
}

bool CPMInputUser::IfSysVolExtDefExist()
{
	if( Heating == GetApplicationType() )
	{
		if( 0 != m_clHeatGeneratorList.GetTotalContent() )
		{
			return true;
		}

		if( 0 != m_clHeatConsumersList.GetTotalContent( GetSupplyTemperature(), GetReturnTemperature() ) )
		{
			return true;
		}

		if( 0 != m_clHeatingPipeList.GetTotalContent() )
		{
			return true;
		}
	}
	else if( Cooling == GetApplicationType() )
	{
		if( 0 != m_clColdGeneratorList.GetTotalContent() )
		{
			return true;
		}

		if( 0 != m_clColdConsumersList.GetTotalContent(GetSupplyTemperature(), GetReturnTemperature() ) )
		{
			return true;
		}

		if( 0 != m_clCoolingPipeList.GetTotalContent() )
		{
			return true;
		}
	}
	else if( Solar == GetApplicationType() )
	{
		// Nothing for the moment.
	}

	return false;
}

bool CPMInputUser::IfInstPowExtDefExist()
{
	if( Heating == GetApplicationType() )
	{
		if( 0 != m_clHeatGeneratorList.GetTotalHeatCapacity() )
		{
			return true;
		}
	}
	else if( Cooling == GetApplicationType() )
	{
		if( 0 != m_clColdGeneratorList.GetTotalColdCapacity() )
		{
			return true;
		}
	}

	return false;
}

IDPTR CPMInputUser::GetExpansionPipeSizeIDPtr( CDB_Product *pclProduct, double dPipelength )
{
	IDPTR idptrPipeSize = _NULL_IDPTR;
	double dPower = GetInstalledPower();
	double dHst = GetStaticHeight();

	CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclProduct );
	CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pclProduct );

	// Hard coded function should be reviewed as soon as logic behind one selection is clarified.
	// This is directly taken from the 'Planning and calculation' documentation.
	if( NULL != pclVessel || ( NULL != pclTecBox && CDB_TecBox::TecBoxType::etbtCompresso == pclTecBox->GetTecBoxType() ) )
	{
		if( PressurisationNorm::PN_DM11275 == GetNorm() )
		{
			double dDiameter = sqrt( m_dInstalledPower / 1163 ) / 1000.0;

			CTable *pTab = TASApp.GetpPipeDB()->GetPipeTab();
			ASSERT( NULL != pTab );

			CTable *pclPipeSeriesSteelGen = static_cast<CTable *>( pTab->Get( L"STEEL_GEN" ).MP );
			ASSERT( NULL != pclPipeSeriesSteelGen );

			CDB_Pipe *pclBestPipe = NULL;
			double dBestDelta = DBL_MAX;

			for( IDPTR IDPtrPipe = pclPipeSeriesSteelGen->GetFirst(); _T('\0') != *IDPtrPipe.ID; IDPtrPipe = pclPipeSeriesSteelGen->GetNext() )
			{
				CDB_Pipe *pclPipe = static_cast<CDB_Pipe *>( IDPtrPipe.MP );
				ASSERT( NULL != pclPipe );

				if( pclPipe->GetIntDiameter() < dDiameter )
				{
					continue;
				}

				if( pclPipe->GetIntDiameter() - dDiameter < dBestDelta )
				{
					dBestDelta = pclPipe->GetIntDiameter() - dDiameter;
					pclBestPipe = pclPipe;
				}
			}

			idptrPipeSize = pclBestPipe->GetSizeIDPtr( m_pTADB );
		}
		else
		{
			if( Heating == m_eApplicationType || Solar == m_eApplicationType )
			{
				if( PressurisationNorm::PN_EN12828 == GetNorm() || PressurisationNorm::PN_EN12953 == GetNorm() )
				{
					if( dPower <= 1e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_20") );
					}
					else if( dPower <= 1.7e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_25") );
					}
					else if( dPower <= 3e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
					}
					else if( dPower <= 3.9e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_40") );
					}
					else if( dPower <= 6e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_50") );
					}
					else if( dPower <= 11e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_65") );
					}
					else if( dPower <= 15e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_80") );
					}
					else //if( dPower <= 23e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_100") );
					}
				}
				else if( PressurisationNorm::PN_SWKIHE301_01 == GetNorm() )
				{
					if( dPower <= 0.3e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_20") );
					}
					else if( dPower <= 0.6e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_25") );
					}
					else if( dPower <= 0.9e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
					}
					else if( dPower <= 1.4e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_40") );
					}
					else if( dPower <= 3e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_50") );
					}
					else if( dPower <= 6e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_65") );
					}
					else if( dPower <= 9e6 )
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_80") );
					}
					else
					{
						idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_100") );
					}
				}
			}
			else
			{
				// Cooling.
				if( dPower <= 1.6e6 )
				{
					idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_20") );
				}
				else if( dPower <= 2.7e6 )
				{
					idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_25") );
				}
				else if( dPower <= 4.8e6 )
				{
					idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
				}
				else if( dPower <= 6.3e6 )
				{
					idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_40") );
				}
				else if( dPower <= 9.6e6 )
				{
					idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_50") );
				}
				else if( dPower <= 18.1e6 )
				{
					idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_65") );
				}
				else if( dPower <= 24.6e6 )
				{
					idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_80") );
				}
				else //if( dPower <= 36.8e6 )
				{
					idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_100") );
				}
			}
		}
	}
	else if( NULL != pclTecBox && CDB_TecBox::TecBoxType::etbtTransfero == pclTecBox->GetTecBoxType() 
			&& 0 == CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TV_C") ) )
	{
		CString strID = pclTecBox->GetIDPtr().ID;
		
		if( dPipelength <= 10 )
		{
			if( 0 == strID.Compare( _T("TRANS_TVC_4_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_4_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_25") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_4_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_4_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_4_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_4_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 13.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_6_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_6_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_25") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_6_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_6_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 23.0 ) ? _T("DN_40" ) : _T( "DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_6_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_6_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 25.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_8_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_8_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_25") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_8_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_8_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 24.0 ) ? _T("DN_40" ) : _T( "DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_8_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_8_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 34.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_10_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_10_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_25") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_10_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_10_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 40.0 ) ? _T("DN_40" ) : _T( "DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_10_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_10_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 52.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_14_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_14_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_25") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_14_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_14_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_14_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_14_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 80.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
		}
		else //if ( dPipelength <= 30 )
		{
			if( 0 == strID.Compare( _T("TRANS_TVC_4_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_4_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_4_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_4_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_4_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_4_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_50") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_6_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_6_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_6_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_6_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 26.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_6_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_6_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 22.0 ) ? _T("DN_65" ) : _T( "DN_50") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_8_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_8_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_8_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_8_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 28.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_8_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_8_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 30.0 ) ? _T("DN_65" ) : _T( "DN_50") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_10_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_10_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_10_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_10_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 45.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_10_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_10_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 48.0 ) ? _T("DN_65" ) : _T( "DN_50") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_14_1E") ) || 0 == strID.Compare( _T("TRANS_TVC_14_1EC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( _T("DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_14_1EH") ) || 0 == strID.Compare( _T("TRANS_TVC_14_1EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 80.0 ) ? _T("DN_40" ) : _T( "DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVC_14_2EH") ) || 0 == strID.Compare( _T("TRANS_TVC_14_2EHC") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 70.0 ) ? _T("DN_65" ) : _T( "DN_50") );
			}
		}
	}
	else if( NULL != pclTecBox && CDB_TecBox::TecBoxType::etbtTransfero == pclTecBox->GetTecBoxType() 
			&& 0 == CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TVI_C") ) )
	{
		CString strID = pclTecBox->GetIDPtr().ID;
		
		if( dPipelength <= 10 )
		{
			if( 0 == strID.Compare( _T("TRANS_TVIC_19_1EH") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 88.0 ) ? _T("DN_40" ) : _T( "DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVIC_19_2EH") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 87.0 ) ? _T("DN_65" ) : _T( "DN_50") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVIC_25_1EH") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 136.0 ) ? _T("DN_40" ) : _T( "DN_32") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVIC_25_2EH") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 136.0 ) ? _T("DN_65" ) : _T( "DN_50") );
			}
		}
		else
		{
			if( 0 == strID.Compare( _T("TRANS_TVIC_19_1EH") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 101.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVIC_19_2EH") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 134.0 ) ? _T("DN_65" ) : _T( "DN_50") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVIC_25_1EH") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 150.0 ) ? _T("DN_50" ) : _T( "DN_40") );
			}
			else if( 0 == strID.Compare( _T("TRANS_TVIC_25_2EH") ) )
			{
				idptrPipeSize = TASApp.GetpTADB()->Get( ( dHst < 188.0 ) ? _T("DN_65" ) : _T( "DN_50") );
			}
		}
	}
	else if( NULL != pclTecBox && CDB_TecBox::TecBoxType::etbtTransferoTI == pclTecBox->GetTecBoxType() 
			&& 0 == CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TIC") ) )
	{
		CString strID = pclTecBox->GetIDPtr().ID;
		CString strClass = CString( strID.GetAt( strID.ReverseFind( _T('_') ) - 1 ) );

		if( 0 == strClass.Compare( _T("0") ) )
		{
			idptrPipeSize = TASApp.GetpTADB()->Get( ( dPipelength <= 10.0 ) ? _T("DN_50" ) : _T( "DN_65") );
		}
		else if( 0 == strClass.Compare( _T("1") ) )
		{
			idptrPipeSize = TASApp.GetpTADB()->Get( ( dPipelength <= 10.0 ) ? _T("DN_65" ) : _T( "DN_80") );
		}
		else if( 0 == strClass.Compare( _T("2") ) )
		{
			idptrPipeSize = TASApp.GetpTADB()->Get( ( dPipelength <= 10.0 ) ? _T("DN_80" ) : _T( "DN_100") );
		}
		else if( 0 == strClass.Compare( _T("3") ) )
		{
			idptrPipeSize = TASApp.GetpTADB()->Get( ( dPipelength <= 10.0 ) ? _T("DN_100" ) : _T( "DN_125") );
		}
	}

	return idptrPipeSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CSelectedPMBase.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSelectedPMBase::CSelectedPMBase( SelectedType eSelectedType )
{
	m_eSelectedType = eSelectedType;
	m_IDPtr = _NULL_IDPTR;
	m_ulFlags = 0;
	m_CompatibilityFlag = CDB_Vessel::ePMComp_None;
	m_dPriceIndex = 0.0;
	m_bRejected = false;
	m_vecRejectedReasons.clear();
	m_mapFlagsErrorCode.clear();
	m_dDelta = 0.0;
	m_dqNmax = 0.0;
}

void CSelectedPMBase::AddRejectedReasons( std::vector<RejectedReasons> &vecRejectedReasons, bool bVerifyDuplication )
{
	if( false == bVerifyDuplication )
	{
		m_vecRejectedReasons.insert( m_vecRejectedReasons.begin() + m_vecRejectedReasons.size(), 
				vecRejectedReasons.begin(), vecRejectedReasons.end() );
	}
	else
	{
		for( auto &iterSrc : vecRejectedReasons ) 
		{
			if( m_vecRejectedReasons.end() == std::find( m_vecRejectedReasons.begin(), m_vecRejectedReasons.end(), iterSrc ) )
			{
				m_vecRejectedReasons.push_back( iterSrc );
			}
		}
	}
}

void CSelectedPMBase::ClearRejectedReasons()
{
	m_vecRejectedReasons.clear();
	m_bRejected = false;
}

int CSelectedPMBase::GetRejectReasonCount( void )
{
	int iSize = (int)m_vecRejectedReasons.size();

	if( iSize < 2 )
	{
		return iSize;
	}

	int iCount = 0;

	for( auto &iter : m_vecRejectedReasons )
	{
		if( RRF_DisplayOnlyWhenAlone != iter.m_eFlag )
		{
			iCount++;
		}
	}

	return iCount;
}

CString CSelectedPMBase::GetFirstRejectedReason( int *piIDIerror )
{
	m_vecRejectedReasonsIter = m_vecRejectedReasons.begin();
	return GetNextRejectedReason( piIDIerror );
}

CString CSelectedPMBase::GetNextRejectedReason( int *piIDIerror )
{
	int iIDError = 1;
	CString strRejectedReason = _T("");

	while( m_vecRejectedReasonsIter != m_vecRejectedReasons.end() && true == strRejectedReason.IsEmpty() )
	{
		if( m_vecRejectedReasons.size() < 2 || RRF_DisplayOnlyWhenAlone != (*m_vecRejectedReasonsIter).m_eFlag )
		{
			strRejectedReason = (*m_vecRejectedReasonsIter).m_strErrorMsg;

			if( NULL != piIDIerror )
			{
				*piIDIerror = (*m_vecRejectedReasonsIter).m_iIDError;
			}
		}

		m_vecRejectedReasonsIter++;
	}

	return strRejectedReason;
}

void CSelectedPMBase::SetFlag( Flags eFlag, bool bSet, int iErrorCode )
{
	unsigned long ulFlag = ( unsigned long )eFlag;

	if( true == bSet )
	{
		// Set.
		m_ulFlags |= ulFlag;

		if( -1 != iErrorCode )
		{
			m_mapFlagsErrorCode[ eFlag ] = iErrorCode;
		}
	}
	else
	{
		// Clear.
		m_ulFlags &= ~ulFlag;

		if( -1 != iErrorCode )
		{
			if( m_mapFlagsErrorCode.count( eFlag ) > 0 )
			{
				m_mapFlagsErrorCode.erase( eFlag );
			}
		}
	}
}

bool CSelectedPMBase::GetFlag( Flags eFlag, int *piErrorCode )
{
	bool bReturn = ( 0 == ( eFlag & m_ulFlags ) ) ? false : true ;

	if( NULL != piErrorCode )
	{
		if( m_mapFlagsErrorCode.count( eFlag ) > 0 )
		{
			*piErrorCode = m_mapFlagsErrorCode[eFlag];
		}
	}

	return bReturn;
}

int CSelectedPMBase::GetFlagError( Flags eFlag )
{
	int iError = -1;

	if( m_mapFlagsErrorCode.count( eFlag ) > 0 )
	{
		iError = m_mapFlagsErrorCode[eFlag];
	}

	return iError;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CSelectedVsl.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSelectedVssl::CSelectedVssl()
	: CSelectedPMBase( SelectedType::ST_Vessel )
{
	m_iNbreOfVsslNeeded = 0;
	m_dMinimumInitialPressure = 0;
	m_dInitialPressure = 0;
	m_dWaterReserve = 0;
	m_bPrimaryIntegrated = false;
	m_dSecVesselPriceIndex2 = 0.0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CSelectedTransfero
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IDPTR CSelectedTransfero::GetBufferVesselIDPtr( CString strVesselName )
{
	if( 0 == (int)m_mapBufferVesselList.size() )
	{
		return _NULL_IDPTR;
	}

	IDPTR BufferVesselIDPtr = _NULL_IDPTR;
	// HYS-1025:
	for( auto &it : m_mapBufferVesselList )
	{
		if( NULL == m_mapBufferVesselList.at( it.first ).m_pclVessel )
		{
			continue;
		}

		if( 0 == strVesselName.Compare( m_mapBufferVesselList.at( it.first ).m_pclVessel->GetName() ) )
		{
			BufferVesselIDPtr = m_mapBufferVesselList.at( it.first ).m_pclVessel->GetIDPtr();
			break;
		}
	}

	return BufferVesselIDPtr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CSelectedWaterTreatmentCombination.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSelectedWaterTreatmentCombination::CSelectedWaterTreatmentCombination() 
		: CSelectedPMBase( SelectedType::ST_WaterTreatmentCombination )
{
	m_pclSelectedFirst = new CSelectedPMBase( SelectedType::ST_Pleno );
	m_pclSelectedSecond = new CSelectedPMBase( SelectedType::ST_Pleno );
	m_pclWTCombination = NULL;
}

CSelectedWaterTreatmentCombination::~CSelectedWaterTreatmentCombination()
{
	if( NULL != m_pclSelectedFirst )
	{
		delete m_pclSelectedFirst;
	}

	if( NULL != m_pclSelectedSecond )
	{
		delete m_pclSelectedSecond;
	}
}

void CSelectedWaterTreatmentCombination::ClearAllFlags( void )
{
	CSelectedPMBase::ClearAllFlags();

	if( NULL != m_pclSelectedFirst )
	{
		m_pclSelectedFirst->ClearAllFlags();
	}

	if( NULL != m_pclSelectedSecond )
	{
		m_pclSelectedSecond->ClearAllFlags();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CSelectedPMList.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSelectPMList::CSelectPMList()
{
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pclInputUser = new CPMInputUser;
}

CSelectPMList::~CSelectPMList()
{
	if( NULL != m_pclInputUser )
	{
		delete m_pclInputUser;
	}

	m_pclInputUser = NULL;
	Clean();
}

void CSelectPMList::SetpTADB( CTADatabase *pTADB )
{ 
	if( NULL == m_pclInputUser )
	{
		ASSERT_RETURN;
	}

	m_pTADB = pTADB;
	m_pclInputUser->SetpTADB( pTADB );
}

void CSelectPMList::SetpTADS( CTADatastruct *pTADS )
{ 
	if( NULL == m_pclInputUser )
	{
		ASSERT_RETURN;
	}

	m_pTADS = pTADS;
	m_pclInputUser->SetpTADS( pTADS );
}

void CSelectPMList::Clean( void )
{
	m_vSelectedPMlist.clear();
	
	//HYS 843: clear rejected
	m_vRejectedPMlist.clear();

	for( int i = PT_First; i < PT_Last; ++i )
	{
		_CleanVecKeyPMBase( (ProductType)i );
		m_ariSelectedProductsIndex[i] = -1;
		m_ariRejectedProductsIndex[i] = -1;

		m_arsPriceData[i].m_pclSelectedPMBase = NULL;
		m_arsPriceData[i].m_dLowestPrice = DBL_MAX;
	}

	for( int i = BPIS_First; i < BPIS_Last; ++i )
	{
		m_arsBestPriceIndex[i].m_pclSelectedPMBase = NULL;
		m_arsBestPriceIndex[i].m_dLowestPrice = DBL_MAX;
		m_arsBestPriceIndex[i].m_dBestPriceIndex = DBL_MAX;
	}
}

int CSelectPMList::Select()
{
	if( NULL == m_pclInputUser || NULL == m_pTADB || NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	Clean();

	// What's the requested pressure maintenance?
	if( CPMInputUser::MT_ExpansionVessel == m_pclInputUser->GetPressureMaintenanceType()
			|| CPMInputUser::MT_All == m_pclInputUser->GetPressureMaintenanceType() )
	{
		// Don't compute vessel if the minimum required PSV is above the PSV.
		double dPSVLimit = m_pclInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MT_ExpansionVessel );

		if( m_pclInputUser->GetSafetyValveResponsePressure() >= dPSVLimit )
		{
			std::map< int, CDB_StringID * > mapType;
			std::map< int, CDB_StringID * >::iterator mapIt;
			CTableOrdered *pTabO = ( CTableOrdered * )( m_pTADB->Get( _T("PMT_EXPVSSL_TAB") ).MP );

			for( IDPTR idptr = pTabO->GetFirst(); NULL != idptr.MP; idptr = pTabO->GetNext( idptr.MP ) )
			{
				CDB_StringID *pstrID = (CDB_StringID *)( idptr.MP );
				mapType[_ttoi( pstrID->GetIDstr() )] = pstrID;
			}

			// Map contains 'VSSLTYPE_STAT' and 'VSSLTYPE_AQUA' ordered.
			vecSelectePMBase vecSelectedList;
			vecSelectePMBase vecRejectedList;

			// HYS-1003: For each type, we will do 2 loops: one with user choice for 'pressure.volume' limit and one other
			// by unlocking the 'pressure.volume' limit if this one is active.
			bool bStop = false;

			for( int iLoop = 0; iLoop < 2 && false == bStop; iLoop++ )
			{
				if( false == m_pclInputUser->UsePressureVolumeLimit() && 1 == iLoop )
				{
					// If 'pressure.volume' limit is not set, we don't need to test.
					continue;
				}

				if( 1 == iLoop )
				{
					Clean();
				}

				bool bUnlockPressureVolumeLimit = ( 0 == iLoop ) ? false: true;

				for( mapIt = mapType.begin(); mapIt != mapType.end() && false == bStop; ++mapIt )
				{
					// Remark: '_SelectVessel' returns now the total vessel found (Selected and rejected).
					// All vessels are for the moment saved only in the 'm_vecSelectedList'.
					_SelectVessel( CDB_Vessel::ePMComp_Statico, mapIt->second, bUnlockPressureVolumeLimit );

					// Sort and store them in the map.
					if( mapType.begin() == mapIt )
					{
						// Here the method will put vessels in 'm_arvecSelectedProducts' and 'm_arvecRejectedProducts'.
						// Remark: if there is a result in the selected list, we keep it and quit this 'for' loop.
						_SortVsslListAndInsertInVector( ProductType::PT_ExpansionVessel, true, bUnlockPressureVolumeLimit );
					}
					else
					{
						// Here the method will put vessels in 'vecSelectedList' and 'vecRejectedList'.
						// Remark: we don't find vessel with the first loop. We try to find vessels with other vessel types.
						_SortVsslListAndInsertInVector( ProductType::PT_ExpansionVessel, true, bUnlockPressureVolumeLimit, &vecSelectedList, &vecRejectedList );
					}
			
					if( mapType.begin() == mapIt )
					{
						// We are with the first vessel type.

						if( true == ( IsProductExist( ProductType::PT_ExpansionVessel, WhichList::WL_Selected ) ) )
						{
							// If we found a product, it's OK we can stop.

							// HYS-867: Flag these solutions to warn user that there are not conform to the SWKI HE301-01.
							if( 1 == iLoop )
							{
								CSelectedPMBase *pclSelectedPMBase = GetFirstProduct( ProductType::PT_ExpansionVessel, WhichList::WL_Selected );

								while( NULL != pclSelectedPMBase )
								{
									pclSelectedPMBase->SetFlag( CSelectedPMBase::eVslFlags3000lbar, true );
									pclSelectedPMBase = GetNextProduct( ProductType::PT_ExpansionVessel, WhichList::WL_Selected );
								}
							}

							bStop = true;
							break;
						}
					}
					else
					{
						// We are with the next vessel types.

						if( vecSelectedList.size() > 0 )
						{
							// Case: Application type=Heating, Norm=SWKI (Split vessel option activated), HST=20m, VS=5m3, Qs=50kW, PSV=4bar, TAZ=100°C, 
							//       Ts=80°C, Tr=70°C and tsmin=5°C. Because the vessels must respect the pressure.volume limit rule, there is no solution
							//       with the first vessel type (Statico). But it exists vessel of 700l in the Aquapresso vessel type.
							//       In this case, we have to show Aquapresso in the selected list and let Statico in the rejected list to allow user to check
							//       why Statico have been rejected.

							// We found a product.
							// We must to clean ONLY 'm_arvecSelectedProducts' to keep the reason why the first vessel type has been rejected.
							// And we copy ONLY 'vecSelectedList' in 'm_arvecSelectedProducts'.
							_CleanVecKeyPMBase( ProductType::PT_ExpansionVessel, &m_arvecSelectedProducts[ProductType::PT_ExpansionVessel] );
							m_arvecSelectedProducts[ProductType::PT_ExpansionVessel] = vecSelectedList;
							
							bStop = true;
							break;
						}
						else
						{
							// We don't found a product with the current vessel type.
							// We must clean 'vecSelectedList' and 'vecRejectedList' and continue with the next vessel type.
							_CleanVecKeyPMBase( ProductType::PT_ExpansionVessel, &vecSelectedList, &vecRejectedList );
						}
					}
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	// Expansion vessel membrane

	if( CPMInputUser::MT_ExpansionVesselWithMembrane == m_pclInputUser->GetPressureMaintenanceType()
			|| CPMInputUser::MT_All == m_pclInputUser->GetPressureMaintenanceType() )
	{
		// Don't compute vessel if the minimum required PSV is above the PSV.
		double dPSVLimit = m_pclInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MT_ExpansionVessel );

		if( m_pclInputUser->GetSafetyValveResponsePressure() >= dPSVLimit )
		{
			CTableOrdered *pTabO = (CTableOrdered *)( m_pTADB->Get( _T("PMT_EXPVSSLMBR_TAB") ).MP );

			// HYS-1577: the Squeeze and MN vessels are now phase out. We keep the code in case of.
			// HYS-1657: The MN vessels are released for now.
			if( NULL != pTabO )
			{
				IDPTR idptr = pTabO->GetFirst();

				_SelectVessel( CDB_Vessel::ePMComp_Statico, (CDB_StringID *)idptr.MP );
				_SortVsslListAndInsertInVector( ProductType::PT_ExpansionVesselMembrane, true );
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Expansion vessel with Compressor.
	// Don't use Compresso in solar!
	if( ProjectType::Solar != m_pclInputUser->GetApplicationType() &&
		( CPMInputUser::MT_WithCompressor == m_pclInputUser->GetPressureMaintenanceType()
			|| CPMInputUser::MT_All == m_pclInputUser->GetPressureMaintenanceType() ) )
	{
		// Don't compute vessel if the minimum required PSV is above the PSV.
		double dPSVLimit = m_pclInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MT_WithCompressor );

		if( m_pclInputUser->GetSafetyValveResponsePressure() >= dPSVLimit )
		{
			CStringArray arTypeID;
			arTypeID.Add( _T("TBXTYPE_COMPRESSO") );

			_SelectTecBox( &arTypeID, CDB_TecBox::etbtCompresso );
			_SortCompressoAndInsertInVector( WhichList::WL_Selected );

			if( 0 != m_vRejectedPMlist.size() )
			{
				_SortCompressoAndInsertInVector( WhichList::WL_Rejected );
			}

			// HYS-982: Special case for the SWKI HE301-01 norm and Switzerland.
			// HYS-2082: Do not set the best compresso for CH region if there is no selected compresso.
			if( ( 0 == TASApp.GetTADBKey().CompareNoCase( _T("CH") ) ) && 0 < m_arvecSelectedProducts[ProductType::PT_Compresso].size() )
					//&& true == m_pclInputUser->UsePressureVolumeLimit() )
			{
				_SetBestCompressoTecBox();
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Expansion vessel with Transfero
	// Don't use Transfero in solar!
	if( ProjectType::Solar != m_pclInputUser->GetApplicationType() &&
		( CPMInputUser::MT_WithPump == m_pclInputUser->GetPressureMaintenanceType()
			|| CPMInputUser::MT_All == m_pclInputUser->GetPressureMaintenanceType() ) )
	{
		// Don't compute vessel if the minimum required PSV is above the PSV.
		double dPSVLimit = m_pclInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MT_WithPump );

		if( m_pclInputUser->GetSafetyValveResponsePressure() >= dPSVLimit )
		{
			CStringArray arTypeID;

			arTypeID.Add( _T("TBXTYPE_TRANSFERO_T") );
			arTypeID.Add( _T("TBXTYPE_TRANSFERO_TI") );

			_SelectTecBox( &arTypeID, CDB_TecBox::etbtTransfero );

			// HYS-981: Check module H for Transfero TV Connect.
			_CheckHModuleForTransferoTV();

			_SortTransferoAndInsertInVector( WhichList::WL_Selected );

			if( 0 != m_vRejectedPMlist.size() )
			{
				_SortTransferoAndInsertInVector( WhichList::WL_Rejected );
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Pleno or/and Vento alone
	if( CPMInputUser::MT_None == m_pclInputUser->GetPressureMaintenanceType() )
	{
		SelectVentoPleno();
	}

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		ComputePriceIndex();
	}

	return 1;
}

bool CSelectPMList::IsErrorTemperature( CDB_Product *pclProduct, int *piTempErrorCode )
{
	if( NULL == m_pclInputUser || NULL == pclProduct )
	{
		return false;
	}

	if( NULL != piTempErrorCode )
	{
		*piTempErrorCode = 0;
	}

	// Test Tmax.
	bool fError = false;

	if( m_pclInputUser->GetReturnTemperature() > pclProduct->GetTmax() )
	{
		fError = true;

		if( NULL != piTempErrorCode )
		{
			*piTempErrorCode |= CSelectedPMBase::TEC_SystemReturnTempBiggerTBMaxTemp;
		}
	}

	// Test Tmin.
	// Remark: in the left panel of the cooling we have the possibility to enable or not the 'Min. temperature'.
	//         If it is disabled, the minimum temperature is initialized with the supply temperature (the lower temperature).
	//         If is is enabled, the minimum temperature is initialized with the freezing point and user can change it.
	//         Thus, we can in both case use the minimum temperature.
	if( m_pclInputUser->GetMinTemperature() < pclProduct->GetTmin() )
	{
		fError = true;

		if( NULL != piTempErrorCode )
		{
			*piTempErrorCode |= CSelectedPMBase::TEC_SystemMinTempLowerTBMinTemp;
		}
	}

	return fError;
}

int CSelectPMList::GetIntermVsslList( CSelectedVssl *pSelectedVssl )
{
	if( NULL == pSelectedVssl || NULL == m_pTADB )
	{
		ASSERTA_RETURN( -1 );
	}

	CDB_Vessel *pCDBVssl = dynamic_cast<CDB_Vessel *>( pSelectedVssl->GetProductIDPtr().MP );
	CDB_StringID *pTypeID = dynamic_cast<CDB_StringID *>( m_pTADB->Get( _T("VSSLTYPE_INTERM") ).MP );

	_CleanVecKeyPMBase( ProductType::PT_IntermediateVessel );

	int iCount = _SelectVessel( pSelectedVssl->GetCompatibilityFlag(), pTypeID, false, pSelectedVssl );

	if( 0 != iCount )
	{
		// Vessel exist store them in map
		_SortVsslListAndInsertInVector( ProductType::PT_IntermediateVessel, true );
	}

	return m_arvecSelectedProducts[ProductType::PT_IntermediateVessel].size();
}

int CSelectPMList::SelectCompressoVessel( CSelectedPMBase *pclSelectedCompresso )
{
	if( NULL == m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pclSelectedCompresso || NULL == dynamic_cast<CDB_TecBox*>( pclSelectedCompresso->GetProductIDPtr().MP ) )
	{
		return 0;
	}

	CDB_TecBox *pclCompresso = dynamic_cast<CDB_TecBox*>( pclSelectedCompresso->GetProductIDPtr().MP );

	double dTecBoxHeight = 0.0;

	// When the tech box is installed on the vessel top we have to take into account supplementary height
	if( CDB_TecBox::eilVesselTop == ( pclCompresso->GetInstallationLayout() & CDB_TecBox::eilVesselTop ) )
	{
		dTecBoxHeight =  pclCompresso->GetHeight();
	}

	_CleanVecKeyPMBase( ProductType::PT_CompressoVessel );

	IDPTR idptr = m_pTADB->Get( _T("VSSLTYPE_PRIM") );

	if( true == pclCompresso->IsVariantIntegratedPrimaryVessel() )
	{
		// Case of Simply Compresso for example.
		_VerifyIntegratedVessel( pclCompresso );
	}
	else
	{	
		_SelectVessel( CDB_Vessel::ePMComp_Compresso, (CDB_StringID *)idptr.MP, false, NULL, dTecBoxHeight, pclCompresso );
	}

	_SortVsslListAndInsertInVector( ProductType::PT_CompressoVessel, false );

	int iSize = (int)m_arvecSelectedProducts[ProductType::PT_CompressoVessel].size();

	if( 0.0 != m_pTADB->GetPrice( pclCompresso->GetArtNum( true ) ) )
	{
		vecSelectePMBase *pvecPrimaryVesselList = &m_arvecSelectedProducts[ProductType::PT_CompressoVessel];

		for( vecSelectePMBaseIter iter = pvecPrimaryVesselList->begin(); iter != pvecPrimaryVesselList->end(); iter++ )
		{
			double dTotalPrice = m_pTADB->GetPrice( pclCompresso->GetArtNum( true ) );
			
			if( NULL == *iter || NULL == dynamic_cast<CSelectedVssl*>( *iter ) )
			{
				continue;
			}

			CSelectedVssl *pclSelectedPrimaryVessel = dynamic_cast<CSelectedVssl*>( *iter );
			CDB_Vessel *pclPrimaryVessel = dynamic_cast<CDB_Vessel*>( pclSelectedPrimaryVessel->GetProductIDPtr().MP );

			if( NULL == pclPrimaryVessel )
			{
				continue;
			}

			// Case for example for the Simply Compresso. The primary vessel is integrated in the TecBox. In this case,
			// we don't add price of the primary vessel but only for the secondary ones if needed.
			if( false == pclSelectedPrimaryVessel->IsPrimaryIntegrated() )
			{
				dTotalPrice += m_pTADB->GetPrice( pclPrimaryVessel->GetArtNum( true ) );
			}

			double dSecVesselPrice = 0.0;

			if( pclSelectedPrimaryVessel->GetNbreOfVsslNeeded() > 1 )
			{
				CDB_Vessel *pclSecondaryVessel = dynamic_cast<CDB_Vessel*>( pclPrimaryVessel->GetSecVsslIDPtr().MP );

				if( NULL == pclSecondaryVessel )
				{
					continue;
				}

				dSecVesselPrice = m_pTADB->GetPrice( pclSecondaryVessel->GetArtNum( true ) );
			}

			double dTotalSecVesselPrice = dSecVesselPrice * ( pclSelectedPrimaryVessel->GetNbreOfVsslNeeded() - 1 );
			dTotalPrice += dTotalSecVesselPrice;

			// Can be 0 in the case of the primary vessel is integrated.
			pclSelectedPrimaryVessel->SetPriceIndex( m_pTADB->GetPrice( pclPrimaryVessel->GetArtNum( true ) ) / dTotalPrice * 100.0 );

			if( dSecVesselPrice > 0.0 )
			{
				pclSelectedPrimaryVessel->SetSecVesselPriceIndex( dTotalSecVesselPrice / dTotalPrice * 100.0 );
			}
		}
	}

	return iSize;
}

int CSelectPMList::SelectTransferoVessel( CSelectedPMBase *pSelectedTransfero )
{
	if( NULL == m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pSelectedTransfero || NULL == dynamic_cast<CDB_TecBox*>( pSelectedTransfero->GetProductIDPtr().MP ) )
	{
		return 0;
	}

	CDB_TecBox* pclTransfero = dynamic_cast<CDB_TecBox*>( pSelectedTransfero->GetProductIDPtr().MP );
	double dTecBoxHeight = 0.0;

	// When the tech box is installed on the vessel top we have to take into account supplementary height
	if( CDB_TecBox::eilVesselTop == ( pclTransfero->GetInstallationLayout() & CDB_TecBox::eilVesselTop ) )
	{
		dTecBoxHeight = pclTransfero->GetHeight();
	}

	_CleanVecKeyPMBase( ProductType::PT_TransferoVessel );

	IDPTR idptr = m_pTADB->Get( _T("VSSLTYPE_PRIM") );

	CDB_Vessel::ePMCompFlags CompFlags = CDB_Vessel::ePMComp_None;

	if( CDB_TecBox::etbtTransfero == pclTransfero->GetTecBoxType() )
	{
		CompFlags = CDB_Vessel::ePMComp_Transfero;
	}
	else if( CDB_TecBox::etbtTransferoTI == pclTransfero->GetTecBoxType() )
	{
		CompFlags = CDB_Vessel::ePMComp_TransferoTI;
	}
	else
	{
		ASSERT( 0 );
		return 0;
	}

	_SelectVessel( CompFlags, (CDB_StringID *)idptr.MP, false, NULL, dTecBoxHeight, pclTransfero );
	_SortVsslListAndInsertInVector( ProductType::PT_TransferoVessel, false );

	int iSize = (int)m_arvecSelectedProducts[ProductType::PT_TransferoVessel].size();

	if( 0.0 != m_pTADB->GetPrice( pclTransfero->GetArtNum( true ) ) )
	{
		vecSelectePMBase *pvecPrimaryVesselList = &m_arvecSelectedProducts[ProductType::PT_TransferoVessel];
		for( vecSelectePMBaseIter iter = pvecPrimaryVesselList->begin(); iter != pvecPrimaryVesselList->end(); iter++ )
		{
			double dTotalPrice = m_pTADB->GetPrice( pclTransfero->GetArtNum( true ) );
			
			if( NULL == *iter || NULL == dynamic_cast<CSelectedVssl*>( *iter ) )
			{
				continue;
			}

			CSelectedVssl *pclSelectedPrimaryVessel = dynamic_cast<CSelectedVssl*>( *iter );
			CDB_Vessel *pclPrimaryVessel = dynamic_cast<CDB_Vessel*>( pclSelectedPrimaryVessel->GetProductIDPtr().MP );

			if( NULL == pclPrimaryVessel || 0.0 == m_pTADB->GetPrice( pclPrimaryVessel->GetArtNum( true ) ) )
			{
				continue;
			}

			dTotalPrice += m_pTADB->GetPrice( pclPrimaryVessel->GetArtNum( true ) );
			double dSecVesselPrice = 0.0;

			if( pclSelectedPrimaryVessel->GetNbreOfVsslNeeded() > 1 )
			{
				CDB_Vessel *pclSecondaryVessel = dynamic_cast<CDB_Vessel*>( pclPrimaryVessel->GetSecVsslIDPtr().MP );

				if( NULL == pclSecondaryVessel || 0.0 == m_pTADB->GetPrice( pclSecondaryVessel->GetArtNum( true ) ) )
				{
					continue;
				}

				dSecVesselPrice = m_pTADB->GetPrice( pclSecondaryVessel->GetArtNum( true ) );
			}

			dTotalPrice += ( dSecVesselPrice * pclSelectedPrimaryVessel->GetNbreOfVsslNeeded() );

			pclSelectedPrimaryVessel->SetPriceIndex( m_pTADB->GetPrice( pclPrimaryVessel->GetArtNum( true ) ) / dTotalPrice * 100.0 );

			if( dSecVesselPrice > 0.0 )
			{
				pclSelectedPrimaryVessel->SetSecVesselPriceIndex( dSecVesselPrice / dTotalPrice * 100.0 );
			}
		}
	}

	return iSize;
}

void CSelectPMList::SelectVentoPleno( CSelectedPMBase *pSelectedPM, bool bOnlyPleno )
{
	// I merge the 'SelectPleno' and 'SelectVento' into only one method. It's just to be sure that the Vento
	// selection is done before the Pleno.

	// With pressurisation system, the Vento and Pleno can be selected with vessel or TecBox (Compresso or Transfero). Thus in this
	// case to fill Pleno we check with the current vessel or selected (pSelectedPM is not null and fOnlyPleno is false).

	// Without pressurisation system, if user wants both water make-up and degassing, we must choose Pleno in regards to the current Vento
	// selected (pSelectedPM is not null and fOnlyPleno is true).

	if( false == bOnlyPleno )
	{
		// Don't select Vento in solar for Compresso and Transfero.
		if( ProjectType::Solar != m_pclInputUser->GetApplicationType() ||
				( CPMInputUser::MaintenanceType::MT_All != m_pclInputUser->GetPressureMaintenanceType()
					&& CPMInputUser::MaintenanceType::MT_WithCompressor != m_pclInputUser->GetPressureMaintenanceType()
					&& CPMInputUser::MaintenanceType::MT_WithPump != m_pclInputUser->GetPressureMaintenanceType() ) )
		{
			_SelectVento( pSelectedPM );
		}
	}

	_SelectPleno( pSelectedPM );
}

int CSelectPMList::SelectPlenoRefill( CDB_TBPlenoVento *pclSelectedPleno, bool bAtLeastOneDeviceSelectedHasVacuumDegassingFunction, CDB_Set *pclWTCombination )
{
	if( NULL == m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	_CleanVecKeyPMBase( ProductType::PT_PlenoRefill );
	m_vSelectedPMlist.clear();
	m_vRejectedPMlist.clear();

	if( NULL == m_pclInputUser || ( NULL == pclSelectedPleno && NULL == pclWTCombination ) )
	{
		return 0;
	}

	// If user has not asked softening or desalination, we don't show Pleno Refill.
	if( 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_NONE") ) 
			|| 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_STD") ) )
	{
		return 0;
	}

	CDB_RuledTable *pclRefillGroup = NULL;

	if( NULL == pclWTCombination )
	{
		pclRefillGroup = dynamic_cast<CDB_RuledTable *>( m_pTADB->Get( pclSelectedPleno->GetRefillGroupIDPtr().ID ).MP );
	}
	else
	{
		pclRefillGroup = dynamic_cast<CDB_RuledTable *>( m_pTADB->Get( pclWTCombination->GetAccGroupIDPtr().ID ).MP );
	}

	if( NULL == pclRefillGroup )
	{
		return 0;
	}

	bool bWorkForSoftening = ( 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_WITHSOFT") ) ) ? true : false;
	bool bWorkForDesalination = ( 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_WITHDESAL") ) ) ? true : false;

	CSelectedPMBase::RejectedReasons rRejectedReasons;
	std::vector<CSelectedPMBase::RejectedReasons> vecRejectedReasons;

	for( IDPTR PRIDPtr = pclRefillGroup->GetFirst(); NULL != PRIDPtr.MP; PRIDPtr = pclRefillGroup->GetNext() )
	{
		vecRejectedReasons.clear();
		CDB_PlenoRefill *pclPlenoRefill = dynamic_cast<CDB_PlenoRefill *>( PRIDPtr.MP );

		if( NULL == pclPlenoRefill )
		{
			continue;
		}

		if( false == pclPlenoRefill->IsSelectable( true ) )
		{
			continue;
		}

		if( true == bWorkForSoftening && ( CDB_PlenoRefill::ePRFunc_Soft != ( CDB_PlenoRefill::ePRFunc_Soft & pclPlenoRefill->GetFunctions() ) ) )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMPRSE_SOFT_NOSOFTFUNCTION;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMPRSE_SOFT_NOSOFTFUNCTION );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		if( true == bWorkForDesalination && ( CDB_PlenoRefill::ePRFunc_Desalin != ( CDB_PlenoRefill::ePRFunc_Desalin & pclPlenoRefill->GetFunctions() ) ) )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMPRSE_DESAL_NODESALFUNCTION;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMPRSE_DESAL_NODESALFUNCTION );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		if( true == bAtLeastOneDeviceSelectedHasVacuumDegassingFunction && false == pclPlenoRefill->IsDegasserCompatible() )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMPRSE_NOTCOMPATIBLEWITHDEGASSER;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMPRSE_NOTCOMPATIBLEWITHDEGASSER );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		// Check the water network pressure.
		if( m_pclInputUser->GetWaterMakeUpNetworkPN() < pclPlenoRefill->GetMinSupplyWaterPressure() )
		{
			CString strPN = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetWaterMakeUpNetworkPN(), true );
			CString strPmin = WriteCUDouble( _U_PRESSURE, pclPlenoRefill->GetMinSupplyWaterPressure(), true );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPFSE_PNBELOWPMIN, strPN, strPmin );
			rRejectedReasons.m_iIDError = IDS_SELECTPMPFSE_PNBELOWPMIN;
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		if( m_pclInputUser->GetWaterMakeUpNetworkPN() > pclPlenoRefill->GetMaxSupplyWaterPressure() )
		{
			CString strPN = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetWaterMakeUpNetworkPN(), true );
			CString strPmin = WriteCUDouble( _U_PRESSURE, pclPlenoRefill->GetMaxSupplyWaterPressure(), true );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPFSE_PNABOVEPMAX, strPN, strPmin );
			rRejectedReasons.m_iIDError = IDS_SELECTPMPFSE_PNABOVEPMAX;
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		CSelectedPMBase *pSelectedPlenoRefill = new CSelectedPMBase( CSelectedPMBase::ST_PlenoRefill );

		if( NULL == pSelectedPlenoRefill )
		{
			continue;
		}

		pSelectedPlenoRefill->SetProductIDPtr( PRIDPtr );

		// Check temperature.
		if( m_pclInputUser->GetWaterMakeUpWaterTemp() < pclPlenoRefill->GetTmin() )
		{
			pSelectedPlenoRefill->SetFlag( CSelectedPMBase::ePRefillWMUpMinTemp, true );
		}
		else if( m_pclInputUser->GetWaterMakeUpWaterTemp() > pclPlenoRefill->GetTmax() )
		{
			pSelectedPlenoRefill->SetFlag( CSelectedPMBase::ePRefillWMUpMaxTemp, true );
		}

		// Add Selected object to the list.
		if( 0 == (int)vecRejectedReasons.size() )
		{
			m_vSelectedPMlist.push_back( pSelectedPlenoRefill );
		}
		else
		{
			pSelectedPlenoRefill->SetRejected();
			pSelectedPlenoRefill->AddRejectedReasons( vecRejectedReasons );
			m_vRejectedPMlist.push_back( pSelectedPlenoRefill );
		}
	}

	if( 0 != ( int )m_vSelectedPMlist.size() )
	{
		_SortPlenoRefillAndInsertInVector( WhichList::WL_Selected );
	}

	if( 0 != m_vRejectedPMlist.size() )
	{
		// Rejected product exist in the vector.
		_SortPlenoRefillAndInsertInVector( WhichList::WL_Rejected );
	}

	return m_arvecSelectedProducts[PT_PlenoRefill].size();
}

bool CSelectPMList::IsProductExist( ProductType eProductType, WhichList eWhichList )
{
	bool fExist = false;

	if( WhichList::WL_Selected == eWhichList )
	{
		fExist = ( 0 != m_arvecSelectedProducts[eProductType].size() ) ? true : false;
	}
	else
	{
		fExist = ( 0 != m_arvecRejectedProducts[eProductType].size() ) ? true : false;
	}

	return fExist;
}

bool CSelectPMList::IsProductExistInBothList( ProductType eProductType )
{
	bool fExist = false;

	fExist = ( 0 != m_arvecSelectedProducts[eProductType].size() ) ? true : false;

	if( false == fExist )
	{
		fExist = ( 0 != m_arvecRejectedProducts[eProductType].size() ) ? true : false;
	}

	return fExist;
}

CSelectedPMBase *CSelectPMList::GetBestProduct( ProductType eProductType )
{
	CSelectedPMBase *pclBest = GetFirstProduct( eProductType, WhichList::WL_Selected );
	
	while( NULL != pclBest && false == pclBest->IsFlagSet( CSelectedPMBase::eBest ) )
	{
		pclBest = GetNextProduct( eProductType, WhichList::WL_Selected );
	}

	return pclBest;
}

CSelectedPMBase *CSelectPMList::GetFirstProduct( ProductType eProductType, WhichList eWhichList )
{
	if( WhichList::WL_Selected == eWhichList )
	{
		m_ariSelectedProductsIndex[eProductType] = 0;
	}
	else
	{
		m_ariRejectedProductsIndex[eProductType] = 0;
	}

	return GetNextProduct( eProductType, eWhichList );
}

CSelectedPMBase *CSelectPMList::GetNextProduct( ProductType eProductType, WhichList eWhichList )
{
	CSelectedPMBase *pSelectedPMBase = NULL;

	if( WhichList::WL_Selected == eWhichList )
	{
		int iIndex = m_ariSelectedProductsIndex[eProductType];

		if( iIndex < (int)m_arvecSelectedProducts[eProductType].size() )
		{
			pSelectedPMBase = (CSelectedPMBase *)m_arvecSelectedProducts[eProductType][iIndex];
			m_ariSelectedProductsIndex[eProductType]++;
		}
	}
	else
	{
		int iIndex = m_ariRejectedProductsIndex[eProductType];

		if( iIndex < (int)m_arvecRejectedProducts[eProductType].size() )
		{
			pSelectedPMBase = (CSelectedPMBase *)m_arvecRejectedProducts[eProductType][iIndex];
			m_ariRejectedProductsIndex[eProductType]++;
		}
	}

	return pSelectedPMBase;
}

CSelectedPMBase *CSelectPMList::GetPrevProduct( ProductType eProductType, WhichList eWhichList )
{
	CSelectedPMBase *pSelectedPMBase = NULL;

	if( WhichList::WL_Selected == eWhichList )
	{
		int iIndex = m_ariSelectedProductsIndex[eProductType];

		if( iIndex >= 0 )
		{
			pSelectedPMBase = (CSelectedPMBase *)m_arvecSelectedProducts[eProductType][iIndex];
			m_ariSelectedProductsIndex[eProductType]--;
		}
	}
	else
	{
		int iIndex = m_ariRejectedProductsIndex[eProductType];

		if( iIndex >= 0 )
		{
			pSelectedPMBase = (CSelectedPMBase *)m_arvecRejectedProducts[eProductType][iIndex];
			m_ariRejectedProductsIndex[eProductType]--;
		}
	}

	return pSelectedPMBase;
}

CSelectedPMBase *CSelectPMList::GetLastProduct( ProductType eProductType, WhichList eWhichList )
{
	if( WhichList::WL_Selected == eWhichList )
	{
		m_ariSelectedProductsIndex[eProductType] = (int)m_arvecSelectedProducts[eProductType].size() - 1;
	}
	else
	{
		m_ariRejectedProductsIndex[eProductType] = (int)m_arvecRejectedProducts[eProductType].size() - 1;
	}

	return GetPrevProduct( eProductType, eWhichList );
}

void CSelectPMList::ComputePriceIndex( void )
{
	bool fProductExist = false;

	for( int i = PT_First; i < PT_Last && false == fProductExist; i++ )
	{
		if( m_arvecSelectedProducts[i].size() != 0 )
		{
			fProductExist = true;
		}
	}

	if( false == fProductExist )
	{
		return;
	}

	_ComputePriceIndex( true );
	_ComputePriceIndex( false );
}

double CSelectPMList::GetBestPriceIndex( BestPriceIndexSolution eWhichSolution )
{
	double dBestPriceIndex = 0.0;

	if( NULL != m_arsBestPriceIndex[eWhichSolution].m_pclSelectedPMBase )
	{
		dBestPriceIndex = m_arsBestPriceIndex[eWhichSolution].m_dBestPriceIndex;

		if( DBL_MAX == dBestPriceIndex )
		{
			dBestPriceIndex = 0.0;
		}
	}

	return dBestPriceIndex;
}

CSelectedPMBase *CSelectPMList::GetBestPriceIndexProduct( BestPriceIndexSolution eWhichSolution )
{
	return m_arsBestPriceIndex[eWhichSolution].m_pclSelectedPMBase;
}

CDlgPMTChartHelper *CSelectPMList::GetTransferoCurves( CDB_TecBox *pclTransferoReference, double dPmanSI, double dqNSI, CWnd *pWnd )
{
	if( NULL == pclTransferoReference || NULL == m_pclInputUser )
	{
		return NULL;
	}

	CRankEx clTransferoTecBoxList;
	int iNbrTransferoTecBox = _GetTransferoTecBoxInSameGroup( &clTransferoTecBoxList, pclTransferoReference );

	if( 0 == iNbrTransferoTecBox )
	{
		return NULL;
	}

	CDlgPMTChartHelper *pclDlgPMTCharHelper = new CDlgPMTChartHelper();

	if( NULL == pclDlgPMTCharHelper )
	{
		ASSERTA_RETURN( NULL );
	}

	if( FALSE == pclDlgPMTCharHelper->Create( CDlgPMTChartHelper::IDD, pWnd ) )
	{
		ASSERTA_RETURN( NULL );
	}

	pclDlgPMTCharHelper->DrawTransferoCurves( pclTransferoReference, dPmanSI, dqNSI, clTransferoTecBoxList, m_pclInputUser );
	return pclDlgPMTCharHelper;
}

CDlgPMTChartHelper *CSelectPMList::GetCompressoCurves( CDB_TecBox *pclCompressoReference, double dPmanSI, double dqNSI, CWnd *pWnd, int iNbrOfDevices )
{
	if( NULL == pclCompressoReference || NULL == m_pclInputUser )
	{
		return NULL;
	}

	CRankEx clCompressoCurveList;
	int iNbrCompressoCurve = _GetCompressoCurveInSameGroup( &clCompressoCurveList, pclCompressoReference );

	if( 0 == iNbrCompressoCurve )
	{
		return NULL;
	}

	CDlgPMTChartHelper *pclDlgPMTCharHelper = new CDlgPMTChartHelper();

	if( NULL == pclDlgPMTCharHelper )
	{
		ASSERTA_RETURN( NULL );
	}

	if( FALSE == pclDlgPMTCharHelper->Create( CDlgPMTChartHelper::IDD, pWnd ) )
	{
		ASSERTA_RETURN( NULL );
	}

	if( true == pclCompressoReference->IsVariantExternalAir() )
	{
		pclDlgPMTCharHelper->DrawCompressoCXCurves( pclCompressoReference, dPmanSI, dqNSI, clCompressoCurveList, m_pclInputUser, iNbrOfDevices );
	}
	else
	{
		pclDlgPMTCharHelper->DrawCompressoCurves( pclCompressoReference, dPmanSI, dqNSI, clCompressoCurveList, m_pclInputUser );
	}

	return pclDlgPMTCharHelper;
}

CDlgPMTChartHelper *CSelectPMList::GetExpansionVesselCurves( CSelectedVssl *pclSelectedVessel, CWnd *pWnd )
{
	if( NULL == pclSelectedVessel || NULL == pclSelectedVessel->GetProductIDPtr().MP
			|| NULL == dynamic_cast<CDB_Vessel *>( pclSelectedVessel->GetProductIDPtr().MP )
			|| NULL == m_pclInputUser )
	{
		return NULL;
	}

	CDlgPMTChartHelper *pclDlgPMTCharHelper = new CDlgPMTChartHelper();

	if( NULL == pclDlgPMTCharHelper )
	{
		ASSERTA_RETURN( NULL );
	}

	if( FALSE == pclDlgPMTCharHelper->Create( CDlgPMTChartHelper::IDD, pWnd ) )
	{
		ASSERTA_RETURN( NULL );
	}

	pclDlgPMTCharHelper->DrawExpansionVesselCurves( pclSelectedVessel, m_pclInputUser );
	return pclDlgPMTCharHelper;
}

HBITMAP CSelectPMList::GetPressureGauge( double dp0, double dpa, double dpe, double dPSV, CWnd *pWnd )
{
	CPMPressureGaugeHelper clPMPressureGaugeHelper;
	HBITMAP hBitmap = clPMPressureGaugeHelper.GetPressureGauge( dp0, dpa, dpe, dPSV, pWnd );
	return hBitmap;
}

bool CSelectPMList::_CompareVssl( CSelectedPMBase *i, CSelectedPMBase *j )
{
	// This method is called before check other parameters in the '_SortVsslListAndInsertInVector' method.

	// Product key match,
	CSelectedVssl *pclFirstVssl = dynamic_cast <CSelectedVssl *>( i );
	CSelectedVssl *pclSecondVssl = dynamic_cast <CSelectedVssl *>( j );

	if( NULL != pclFirstVssl  && NULL != pclSecondVssl )
	{
		CDB_Vessel *pFirstProd = dynamic_cast<CDB_Vessel *>( pclFirstVssl->GetProductIDPtr().MP );
		CDB_Vessel *pSecondProd = dynamic_cast<CDB_Vessel *>( pclSecondVssl->GetProductIDPtr().MP );

		if( pFirstProd->GetPriorityLevel() < pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the lowest number for the priority level, it is the first.
			return true;
		}
		else if( pFirstProd->GetPriorityLevel() > pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the biggest number for the priority level, it is the second.
			return false;
		}

		// Type.
		CDB_StringID *pFirstStrID = (CDB_StringID *)pFirstProd->GetTypeIDPtr().MP;
		CDB_StringID *pSecondStrID = (CDB_StringID *)pSecondProd->GetTypeIDPtr().MP;

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}

		// Nominal volume.
		if( pFirstProd->GetNominalVolume() < pSecondProd->GetNominalVolume() )
		{
			return true;
		}
		else if( pFirstProd->GetNominalVolume() > pSecondProd->GetNominalVolume() )
		{
			return false;
		}

		// Test PMax
		if( pFirstProd->GetPmaxmax() < pSecondProd->GetPmaxmax() )
		{
			return true;
		}
		else if( pFirstProd->GetPmaxmax() > pSecondProd->GetPmaxmax() )
		{
			return false;
		}

		// VNDelta
		if( pclFirstVssl->GetDelta() < pclSecondVssl->GetDelta() )
		{
			return true;
		}
		else if( pclFirstVssl->GetDelta() > pclSecondVssl->GetDelta() )
		{
			return false;
		}

		// Family.
		pFirstStrID = (CDB_StringID *)pFirstProd->GetFamilyIDPtr().MP;
		pSecondStrID = (CDB_StringID *)pSecondProd->GetFamilyIDPtr().MP;

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	ASSERT( 0 );
	return false;
}

bool CSelectPMList::_CompareVsslAfter( CSelectedPMBase *i, CSelectedPMBase *j )
{
	// This method is called at the end of the '_SortVsslListAndInsertInVector' method to resort the final list.
	// It is almost the same of the '_CompareVsslAfter' except we inverse 'Pmax' and nominal volume.

	// Product key match.
	CSelectedVssl *pclFirstVssl = dynamic_cast <CSelectedVssl *>( i );
	CSelectedVssl *pclSecondVssl = dynamic_cast <CSelectedVssl *>( j );

	if( NULL != pclFirstVssl  && NULL != pclSecondVssl )
	{
		CDB_Vessel *pFirstProd = dynamic_cast<CDB_Vessel *>( pclFirstVssl->GetProductIDPtr().MP );
		CDB_Vessel *pSecondProd = dynamic_cast<CDB_Vessel *>( pclSecondVssl->GetProductIDPtr().MP );

		if( pFirstProd->GetPriorityLevel() < pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the lowest number for the priority level, it is the first.
			return true;
		}
		else if( pFirstProd->GetPriorityLevel() > pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the biggest number for the priority level, it is the second.
			return false;
		}

		// Type.
		CDB_StringID *pFirstStrID = (CDB_StringID *)pFirstProd->GetTypeIDPtr().MP;
		CDB_StringID *pSecondStrID = (CDB_StringID *)pSecondProd->GetTypeIDPtr().MP;

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}

		// Test PMax
		if( pFirstProd->GetPmaxmax() < pSecondProd->GetPmaxmax() )
		{
			return true;
		}
		else if( pFirstProd->GetPmaxmax() > pSecondProd->GetPmaxmax() )
		{
			return false;
		}

		// Nominal volume.
		if( pFirstProd->GetNominalVolume() < pSecondProd->GetNominalVolume() )
		{
			return true;
		}
		else if( pFirstProd->GetNominalVolume() > pSecondProd->GetNominalVolume() )
		{
			return false;
		}

		// VNDelta
		if( pclFirstVssl->GetDelta() < pclSecondVssl->GetDelta() )
		{
			return true;
		}
		else if( pclFirstVssl->GetDelta() > pclSecondVssl->GetDelta() )
		{
			return false;
		}

		// Family.
		pFirstStrID = (CDB_StringID *)pFirstProd->GetFamilyIDPtr().MP;
		pSecondStrID = (CDB_StringID *)pSecondProd->GetFamilyIDPtr().MP;

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	ASSERT( 0 );
	return false;
}

bool CSelectPMList::_CompareTecBoxVessel( CSelectedPMBase *i, CSelectedPMBase *j )
{
	// Product key match,
	CSelectedVssl *pclFirstVssl = dynamic_cast <CSelectedVssl *>( i );
	CSelectedVssl *pclSecondVssl = dynamic_cast <CSelectedVssl *>( j );

	if( NULL != pclFirstVssl  && NULL != pclSecondVssl )
	{
		CDB_Vessel *pFirstProd = dynamic_cast<CDB_Vessel *>( pclFirstVssl->GetProductIDPtr().MP );
		CDB_Vessel *pSecondProd = dynamic_cast<CDB_Vessel *>( pclSecondVssl->GetProductIDPtr().MP );

		if( pFirstProd->GetPriorityLevel() < pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the lowest number for the priority level, it is the first.
			return true;
		}
		else if( pFirstProd->GetPriorityLevel() > pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the biggest number for the priority level, it is the second.
			return false;
		}

		// Type.
		CDB_StringID *pFirstStrID = (CDB_StringID *)pFirstProd->GetTypeIDPtr().MP;
		CDB_StringID *pSecondStrID = (CDB_StringID *)pSecondProd->GetTypeIDPtr().MP;

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}

		// Nominal volume.
		if( pFirstProd->GetNominalVolume() < pSecondProd->GetNominalVolume() )
		{
			return true;
		}
		else if( pFirstProd->GetNominalVolume() > pSecondProd->GetNominalVolume() )
		{
			return false;
		}

		// Test PMax
		// For Compresso and Transfero, we show vessel with the biggest vessel followed by the lowest.
		// And we want for example that the vessel PN 10 are displayed after PN 6. This is why
		// we inverse the condition for the comparison 
		if( pFirstProd->GetPmaxmax() > pSecondProd->GetPmaxmax() )
		{
			return true;
		}
		else if( pFirstProd->GetPmaxmax() < pSecondProd->GetPmaxmax() )
		{
			return false;
		}

		// VNDelta
		if( pclFirstVssl->GetDelta() < pclSecondVssl->GetDelta() )
		{
			return true;
		}
		else if( pclFirstVssl->GetDelta() > pclSecondVssl->GetDelta() )
		{
			return false;
		}

		// Family.
		// For Compresso and Transfero, we show vessel with the biggest vessel followed by the lowest.
		// And we want for example that the CG vessel family are displayed after CU. This is why
		// we inverse the condition for the comparison 
		pFirstStrID = (CDB_StringID *)pFirstProd->GetFamilyIDPtr().MP;
		pSecondStrID = (CDB_StringID *)pSecondProd->GetFamilyIDPtr().MP;

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	ASSERT( 0 );
	return false;
}

bool CSelectPMList::_CompareCompressoTransfero( CSelectedPMBase *pclFirstTecBox, CSelectedPMBase *pclSecondTecBox )
{
	// Remark: this methods must return 'true' if 'pclFirstTecBox' must be placed before 'pclSecondTecBox'.
	if( NULL != pclFirstTecBox && NULL != pclSecondTecBox )
	{
		CDB_TecBox *pFirstProd = dynamic_cast<CDB_TecBox *>( pclFirstTecBox->GetProductIDPtr().MP );
		CDB_TecBox *pSecondProd = dynamic_cast<CDB_TecBox *>( pclSecondTecBox->GetProductIDPtr().MP );

		if( pFirstProd->GetPriorityLevel() < pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the lowest number for the priority level, it is the first.
			return true;
		}
		else if( pFirstProd->GetPriorityLevel() > pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the biggest number for the priority level, it is the second.
			return false;
		}

		bool fFirstTecBoxErr = pclFirstTecBox->GetFlag( CSelectedPMBase::eTemperature );
		bool fSecondTecBoxErr = pclSecondTecBox->GetFlag( CSelectedPMBase::eTemperature );

		if( true == fFirstTecBoxErr || true == fSecondTecBoxErr )
		{
			// Keep selection without temperature issue
			if( true == fFirstTecBoxErr && false == fSecondTecBoxErr )
			{
				return false;
			}

			if( false == fFirstTecBoxErr && true == fSecondTecBoxErr )
			{
				return true;
			}
		}

		// Type.
		CDB_StringID *pFirstStrID = (CDB_StringID *)( pFirstProd->GetTypeIDPtr().MP );
		CDB_StringID *pSecondStrID = (CDB_StringID *)( pSecondProd->GetTypeIDPtr().MP );

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}

		// Family
		pFirstStrID = (CDB_StringID *)pFirstProd->GetFamilyIDPtr().MP;
		pSecondStrID = (CDB_StringID *)pSecondProd->GetFamilyIDPtr().MP;

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}

		// Pump/Compressor number.
		// Models without pump or compressors are pushed at the end of the list
		if( 0 == pFirstProd->GetNbrPumpCompressor() && 0 != pSecondProd->GetNbrPumpCompressor() )
		{
			return false;
		}

		if( 0 != pFirstProd->GetNbrPumpCompressor() && 0 == pSecondProd->GetNbrPumpCompressor() )
		{
			return true;
		}
		else //( 0 != pFirstProd->GetNbrPumpCompressor() && 0 != pFirstProd->GetNbrPumpCompressor() )
		{
			if( pFirstProd->GetNbrPumpCompressor() < pSecondProd->GetNbrPumpCompressor() )
			{
				return true;
			}
			else if( pFirstProd->GetNbrPumpCompressor() > pSecondProd->GetNbrPumpCompressor() )
			{
				return false;
			}
		}

		// Test PMax.
		// it should be > Final Pressure but idealy < GetSafetyPressValve() to relax air before water
		if( pFirstProd->GetPmaxmax() < pSecondProd->GetPmaxmax() )
		{
			return true;
		}
		else if( pFirstProd->GetPmaxmax() > pSecondProd->GetPmaxmax() )
		{
			return false;
		}

		// Installation layout ! lower cost?
		if( pFirstProd->GetInstallationLayout() < pSecondProd->GetInstallationLayout() )
		{
			return true;
		}
		else if( pFirstProd->GetInstallationLayout() > pSecondProd->GetInstallationLayout() )
		{
			return false;
		}

		// Delta.
		// Don't test delta if one of both TecBox has no pump or Compressor (no curve, no delta -> see '_SelectTecBox').
		if( 0 != pFirstProd->GetNbrPumpCompressor() && 0 != pSecondProd->GetNbrPumpCompressor() )
		{
			if( pclFirstTecBox->GetDelta() < pclSecondTecBox->GetDelta() )
			{
				return true;
			}
			else if( pclFirstTecBox->GetDelta() > pclSecondTecBox->GetDelta() )
			{
				return false;
			}

			if( pclFirstTecBox->GetqNmax() < pclSecondTecBox->GetqNmax() )
			{
				return true;
			}
			else if( pclFirstTecBox->GetqNmax() > pclSecondTecBox->GetqNmax() )
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	return false;
}

bool CSelectPMList::_CompareBufferVessel( CSelectedTransfero::BufferVesselData rFirstBufferVesselData, CSelectedTransfero::BufferVesselData rSecondBufferVesselData )
{
	// Remark: this methods must return 'true' if 'pFirstBufferVesselData' must be placed before 'pSecondBufferVesselData'.
	CDB_Vessel *pFirstVessel = rFirstBufferVesselData.m_pclVessel;
	CDB_Vessel *pSecondVessel = rSecondBufferVesselData.m_pclVessel;
	// HYS-1025:
	if( NULL != pFirstVessel && NULL != pSecondVessel )
	{
		// Type.
		CDB_StringID *pFirstStrID = (CDB_StringID *)( pFirstVessel->GetFamilyIDPtr().MP );
		CDB_StringID *pSecondStrID = (CDB_StringID *)( pSecondVessel->GetFamilyIDPtr().MP );

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}

		// PN.
		if( pFirstVessel->GetPmaxmax() < pSecondVessel->GetPmaxmax() )
		{
			return true;
		}
		else if( pFirstVessel->GetPmaxmax() > pSecondVessel->GetPmaxmax() )
		{
			return false;
		}

		// Volume.
		if( pFirstVessel->GetNominalVolume() < pSecondVessel->GetNominalVolume() )
		{
			return true;
		}
		else if( pFirstVessel->GetNominalVolume() > pSecondVessel->GetNominalVolume() )
		{
			return false;
		}
	}

	return false;
}

void CSelectPMList::GetTransferoBufferVesselList( CDB_TecBox *pTB, std::map<int, CSelectedTransfero::BufferVesselData> *pmapBufVssl )
{
	if( NULL == m_pTADB || NULL == pmapBufVssl )
	{
		ASSERT_RETURN;
	}
	
	pmapBufVssl->clear();

	if( true == pTB->IsVariantIntegratedBufferVessel() )
	{
		// If the buffer is integrated, we don't add it as a separated article.
		return;
	}

	std::vector<CSelectedTransfero::BufferVesselData> vecList;

	// Check first if there is not a local table defined that links buffer vessel to device.
	// For example it has been asked in UK to be able to choose between 2 buffer vessels.
	IDPTR BufVsslTabIDPtr = m_pTADB->Get( _T("PMBUFFERVSSL_TAB") );
	CTable *pTabBufVssl = dynamic_cast<CTable *>( BufVsslTabIDPtr.MP );
	CString strTecBoxFamilyID = pTB->GetFamilyID();

	if( NULL != pTabBufVssl )
	{
		// In the redirection buffer vessel table try to found select Transfero.
		for( IDPTR IDPtr = pTabBufVssl->GetFirst(); NULL != IDPtr.MP; IDPtr = pTabBufVssl->GetNext( IDPtr.MP ) )
		{
			CTableOrdered *pTabOrd = dynamic_cast<CTableOrdered *>( IDPtr.MP );
			IDPTR BufferVesselIDPtr = pTabOrd->Get( pTB->GetIDPtr().ID );
				
			if( _NULL_IDPTR != BufferVesselIDPtr )
			{
				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel*>( m_pTADB->Get( pTabOrd->GetName() ).MP );

				if( NULL == pclVessel || pclVessel->GetPmaxmax() < m_pclInputUser->GetSafetyValveResponsePressure() )
				{
					continue;
				}
				
				CSelectedTransfero::BufferVesselData rBufferVesselData;
				rBufferVesselData.m_bOnRequest = false;
				rBufferVesselData.m_pclVessel = pclVessel;

				vecList.push_back( rBufferVesselData );
			}
		}
	}

	if( 0 == (int)vecList.size() )
	{
		if( NULL != dynamic_cast<CTable *>( pTB->GetIntegratedVesselIDPtr().MP ) )
		{
			// We need to check if buffer vessel PN is not below the PSV.
			CTable *pTabBufVssl = dynamic_cast<CTable *>( pTB->GetIntegratedVesselIDPtr().MP );

			if( NULL != pTabBufVssl )
			{
				int iCountBufferPN25 = 0;
				int iCountBufferBelowPN25 = 0;
				std::multimap<double, CSelectedTransfero::BufferVesselData> mapBufferVessels;

				for( IDPTR IDPtr = pTabBufVssl->GetFirst(); NULL != IDPtr.MP; IDPtr = pTabBufVssl->GetNext( IDPtr.MP ) )
				{
					CDB_Vessel *pclBufferVessel = dynamic_cast<CDB_Vessel *>( IDPtr.MP );

					if( NULL == pclBufferVessel )
					{
						continue;
					}
				
					if( pclBufferVessel->GetPmaxmax() < m_pclInputUser->GetSafetyValveResponsePressure() )
					{
						continue;
					}

					// HYS-1402 : Use IsVolumeRespectsPressureVolumeLimit() function to consider GetSafetyValveResponsePressure > 2 bar
					if( false == m_pclInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 )
							|| ( CSelectPMList::eAcceptVessel == 
									IsVolumeRespectsPressureVolumeLimit( pclBufferVessel, m_pclInputUser->GetSafetyValveResponsePressure(), true ) ) )
					{
						// HYS-1311: Particular case for Transfero TVI Connect.
						// Below or equal to 10 bar we can show both (SH 150/300.25) for TVI.19 & TVI.25.
						// Above to 10 bar we must show only SH 300 for TVI.19 & TVI.25.
						if( 0 == strTecBoxFamilyID.Compare( _T("TBXFAM_TRANSFERO_TVI_C") ) && m_pclInputUser->GetSafetyValveResponsePressure() > 1e6 
								&& 0 != CString( pclBufferVessel->GetIDPtr().ID ).Compare( _T("STAT_SH300_25") ) )
						{
							continue;
						}

						CSelectedTransfero::BufferVesselData rBufferVesselData;
						rBufferVesselData.m_bOnRequest = false;
						rBufferVesselData.m_pclVessel = pclBufferVessel;
						mapBufferVessels.insert( std::pair<double, CSelectedTransfero::BufferVesselData>( pclBufferVessel->GetPmaxmax(), rBufferVesselData ) );

						if( 2.5e6 == pclBufferVessel->GetPmaxmax() )
						{
							iCountBufferPN25++;
						}
						else
						{
							iCountBufferBelowPN25++;
						}
					}
				}

				// HYS-1478: We show SH 150/300.25 only if there is no other solution.
				bool bErasePN25 = ( iCountBufferPN25 > 0 && iCountBufferBelowPN25 > 0 ) ? true : false;
				
				for( auto &iter : mapBufferVessels )
				{
					if( true == bErasePN25 && 2.5e6 == iter.first )
					{
						continue;
					}

					vecList.push_back( iter.second );
				}
			}
		}
		else if( 0 == strTecBoxFamilyID.Compare( _T("TBXFAM_TRANSFERO_T") )
				&& 0 != strTecBoxFamilyID.Compare( _T("TBXFAM_TRANSFERO_TV_C") )
				&& 0 != strTecBoxFamilyID.Compare( _T("TBXFAM_TRANSFERO_TVI_C") ) )
		{
			CString strBufferVesselID( _T("") );

			if( m_pclInputUser->GetSafetyValveResponsePressure() <= 3e5 )
			{
				strBufferVesselID = _T("STAT_SD35_3");
			}
			else if( m_pclInputUser->GetSafetyValveResponsePressure() <= 10e+5 )
			{
				strBufferVesselID = _T("STAT_SD35_10");
			}

			CSelectedTransfero::BufferVesselData rBufferVesselData;
			rBufferVesselData.m_bOnRequest = false;
			rBufferVesselData.m_pclVessel = dynamic_cast<CDB_Vessel *>( (CData *)( m_pTADB->Get( strBufferVesselID ).MP ) );

			vecList.push_back( rBufferVesselData );
		}
		else if( 0 == strTecBoxFamilyID.Compare( _T("TBXFAM_TRANSFERO_TIC") ) )
		{
			double dVD = m_pclInputUser->GetVD();
		
			// Identify in witch BUFVSSLTYPESEL_TAB is include this TecBox.
			CTable *pBufVsslTypeTab = dynamic_cast <CTable *>( m_pTADB->Get( _T("BUFVSSLTYPESEL_TAB") ).MP );

			if( NULL == pBufVsslTypeTab )
			{
				ASSERT_RETURN;
			}

			CDB_BufferVesselTI *pBuffVesselTIList = NULL;
			IDPTR idptrBVTab;
			bool bFound = false;

			for( idptrBVTab = pBufVsslTypeTab->GetFirst(); NULL != idptrBVTab.MP; idptrBVTab = pBufVsslTypeTab->GetNext( idptrBVTab.MP ) )
			{
				CTable *pTab = dynamic_cast<CTable *>( idptrBVTab.MP );
				
				if( NULL == pTab )
				{
					// Get the object BV_TRANS_TI with buffer vessel id and correspondence.
					pBuffVesselTIList = dynamic_cast<CDB_BufferVesselTI *>( idptrBVTab.MP );
					continue;
				}
				
				if( NULL != pTab->Get( pTB->GetIDPtr().ID ).MP )
				{
					bFound = true;
					break;
				}
			}

			if( false == bFound )
			{
				ASSERT_RETURN;
			}

			CSelectedTransfero::BufferVesselData rBufferVesselData;
			rBufferVesselData.m_bOnRequest = false;
			rBufferVesselData.m_pclVessel = NULL;
			CString strBufferVesselID( _T("") );

			if( NULL == pBuffVesselTIList || NULL == pBuffVesselTIList->GetStringList() )
			{
				ASSERT_RETURN;
			}

			int iNbuff = (int)pBuffVesselTIList->GetStringList()->size() / 2;
			int indStr = 0;

			for( int i = 0; i < iNbuff; i++ )
			{
				CString strID = pBuffVesselTIList->GetStringList()->at( indStr++ );

				if( false == idptrBVTab.IDMatch( (LPCTSTR)strID ) )
				{
					indStr++;
					continue;
				}

				if( m_pclInputUser->GetSafetyValveResponsePressure() <= pBuffVesselTIList->GetVectPSVList().at( i ).second )
				{
					bool bBufferVesselFound = false;

					if( dVD <= pBuffVesselTIList->GetVectVDList().at(i).second
							&& 0 == StringCompare( _T("<"), pBuffVesselTIList->GetVectVDList().at( i ).first ) )
					{
						strBufferVesselID = pBuffVesselTIList->GetStringList()->at( indStr );
						bBufferVesselFound = true;
					}
					else if( dVD > pBuffVesselTIList->GetVectVDList().at( i ).second
							&& 0 == StringCompare( _T(">"), pBuffVesselTIList->GetVectVDList().at( i ).first ) )
					{
						strBufferVesselID = pBuffVesselTIList->GetStringList()->at( indStr );
						bBufferVesselFound = true;
					}
					
					// HYS-1593 : For TI apply the rule of HYS-1402
					if( true == bBufferVesselFound && 0 != StringCompare( _T( "" ), strBufferVesselID ) )
					{
						CDB_Vessel *pclBufferVessel = dynamic_cast<CDB_Vessel *>( (CData *)( m_pTADB->Get( strBufferVesselID ).MP ) );

						// HYS-1402 : Use IsVolumeRespectsPressureVolumeLimit() function to consider GetSafetyValveResponsePressure > 2 bar
						if( false == m_pclInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 )
								|| CSelectPMList::eAcceptVessel == IsVolumeRespectsPressureVolumeLimit( pclBufferVessel, m_pclInputUser->GetSafetyValveResponsePressure(), true ) )
						{
							break;
						}
					}
				}
				
				indStr++;
			}
			
			if( 0 == StringCompare( _T(""), strBufferVesselID ) )
			{
				rBufferVesselData.m_bOnRequest = true;
			}

			if( false == rBufferVesselData.m_bOnRequest )
			{
				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( (CData *)( m_pTADB->Get( strBufferVesselID ).MP ) );

				if( NULL != pclVessel )
				{
					rBufferVesselData.m_pclVessel = pclVessel;
				}
			}

			vecList.push_back( rBufferVesselData );
		}
	}

	std::sort( vecList.begin(), vecList.end(), CSelectPMList::_CompareBufferVessel );
	int iKey = 0;

	for( auto &iter : vecList )
	{
		pmapBufVssl->insert( std::pair<int, CSelectedTransfero::BufferVesselData>(iKey++, iter ) );
	}
}

CSelectPMList::CheckPressureVolume CSelectPMList::IsVolumeRespectsPressureVolumeLimit( CDB_Vessel *pclVessel, double dPSVS, bool bForBufferVessel )
{
	CSelectPMList::CheckPressureVolume eReturn = CheckPressureVolume::eNotApplicable;

	// HYS-999: pressure.volume limit not applicable for Transfero vessel.
	if( ( false == bForBufferVessel ) 
		&& ( CDB_Vessel::ePMComp_Transfero == ( pclVessel->GetCompatibilityflags() & CDB_Vessel::ePMComp_Transfero )
			|| CDB_Vessel::ePMComp_TransferoTI == ( pclVessel->GetCompatibilityflags() & CDB_Vessel::ePMComp_TransferoTI ) ) )
	{
		eReturn = CheckPressureVolume::eNotApplicable;
	}
	else if( true == m_pclInputUser->UsePressureVolumeLimit() )
	{
		if( dPSVS <= 2e+5 )
		{
			eReturn = CSelectPMList::eAcceptVessel;
		}
		else if( pclVessel->GetNominalVolume() * dPSVS <= m_pTADS->GetpTechParams()->GetPressureVolumeLimit() )
		{
			eReturn = CSelectPMList::eAcceptVessel;
		}
		else
		{
			eReturn = CSelectPMList::eRejectVessel;
		}
	}
	else
	{
		eReturn = CSelectPMList::eAcceptVessel;
	}

	return eReturn;
}

bool CSelectPMList::_ComparePlenoVento( CSelectedPMBase *pclFirstPlenoVento, CSelectedPMBase *pclSecondPlenoVento )
{
	// Remark: this methods must return 'true' if 'pclFirstPlenoVento' must be placed before 'pclSecondPlenoVento'.
	if( NULL != pclFirstPlenoVento && NULL != pclSecondPlenoVento )
	{
		CDB_TBPlenoVento *pFirstProd = dynamic_cast<CDB_TBPlenoVento *>( pclFirstPlenoVento->GetProductIDPtr().MP );
		CDB_TBPlenoVento *pSecondProd = dynamic_cast<CDB_TBPlenoVento *>( pclSecondPlenoVento->GetProductIDPtr().MP );

		if( pFirstProd->GetPriorityLevel() < pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the lowest number for the priority level, it is the first.
			return true;
		}
		else if( pFirstProd->GetPriorityLevel() > pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the biggest number for the priority level, it is the second.
			return false;
		}

		// Type.
		CDB_StringID *pFirstStrID = (CDB_StringID *)( pFirstProd->GetTypeIDPtr().MP );
		CDB_StringID *pSecondStrID = (CDB_StringID *)( pSecondProd->GetTypeIDPtr().MP );

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}

		// Installation layout ! lower cost?
		if( pFirstProd->GetInstallationLayout() < pSecondProd->GetInstallationLayout() )
		{
			return true;
		}
		else if( pFirstProd->GetInstallationLayout() > pSecondProd->GetInstallationLayout() )
		{
			return false;
		}

		// Test PMax.
		// it should be > Final Pressure but idealy < GetSafetyPressValve() to relax air before water
		if( pFirstProd->GetPmaxmax() < pSecondProd->GetPmaxmax() )
		{
			return true;
		}
		else if( pFirstProd->GetPmaxmax() > pSecondProd->GetPmaxmax() )
		{
			return false;
		}

		// Family
		pFirstStrID = (CDB_StringID *)pFirstProd->GetFamilyIDPtr().MP;
		pSecondStrID = (CDB_StringID *)pSecondProd->GetFamilyIDPtr().MP;

		if( _ttoi( pFirstStrID->GetIDstr() ) < _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return true;
		}

		if( _ttoi( pFirstStrID->GetIDstr() ) > _ttoi( pSecondStrID->GetIDstr() ) )
		{
			return false;
		}
		//HYS-1004: Sort vento by taking into account the working pressure range
		if( pFirstProd->GetWorkingPressRangeMin() < pSecondProd->GetWorkingPressRangeMin() )
		{
			return true;
		}
		else if( pFirstProd->GetWorkingPressRangeMin() > pSecondProd->GetWorkingPressRangeMin() )
		{
			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	ASSERTA_RETURN( false );
}

bool CSelectPMList::_ComparePlenoRefill( CSelectedPMBase *pclFirstPlenoRefill, CSelectedPMBase *pclSecondPlenoRefill )
{
	// Remark: this methods must return 'true' if 'pclFirstPlenoRefill' must be placed before 'pclSecondPlenoRefill'.
	if( NULL != pclFirstPlenoRefill && NULL != pclSecondPlenoRefill )
	{
		CDB_PlenoRefill *pFirstProd = dynamic_cast<CDB_PlenoRefill *>( pclFirstPlenoRefill->GetProductIDPtr().MP );
		CDB_PlenoRefill *pSecondProd = dynamic_cast<CDB_PlenoRefill *>( pclSecondPlenoRefill->GetProductIDPtr().MP );

		if( pFirstProd->GetPriorityLevel() < pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the lowest number for the priority level, it is the first.
			return true;
		}
		else if( pFirstProd->GetPriorityLevel() > pSecondProd->GetPriorityLevel() )
		{
			// If the first one has the biggest number for the priority level, it is the second.
			return false;
		}

		if( pFirstProd->GetCapacity() < pSecondProd->GetCapacity() )
		{
			return true;
		}

		if( pFirstProd->GetCapacity() > pSecondProd->GetCapacity() )
		{
			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	ASSERTA_RETURN( false );
}

int CSelectPMList::_SelectVessel( int iPMCompatibilityFlag, CDB_StringID *pTypeID, bool bUnlockPressureVolumeLimit,
		CSelectedVssl *pclBaseVesselForIntermediate, double dTecBoxHeight, CDB_TecBox *pCurrentTBSelected )
{
	if( NULL == m_pclInputUser || NULL == m_pTADB || NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	if( NULL != pclBaseVesselForIntermediate && NULL == dynamic_cast<CDB_Vessel*>( pclBaseVesselForIntermediate->GetProductIDPtr().MP ) )
	{
		return -1;
	}

	m_vSelectedPMlist.clear();
	m_vRejectedPMlist.clear();

	std::vector<CSelectedPMBase::RejectedReasons> vecRejectedReasons;

	CTable *pTabVssl = (CTable *)( m_pTADB->Get( L"VSSL_TAB" ).MP );

	bool bPrimaryVessel = ( 0 == CString( _T("VSSLTYPE_PRIM") ).Compare( pTypeID->GetIDPtr().ID ) );
	bool bForTecBox = ( CDB_Vessel::ePMComp_Statico != ( CDB_Vessel::ePMComp_Statico & iPMCompatibilityFlag ) );

	if( true == bForTecBox && true == bPrimaryVessel && NULL == pCurrentTBSelected )
	{
		// We don't allow primary vessel selection for tecbox if we don't know the tecbox!
		ASSERTA_RETURN( -1 );
	}

	// HYS-1374
	bool bTecBoxOnTopOnly = false;

	if( true == bForTecBox && true == bPrimaryVessel )
	{
		if( CDB_TecBox::eilVesselTop == ( pCurrentTBSelected->GetInstallationLayout() & CDB_TecBox::eilVesselTop ) )
		{
			bTecBoxOnTopOnly = true;
		}
	}
	

	double dVsslPmax = 0;
	double dSmallestMinVesselVolumeForDegassing = GetSmallestMinVesselVolumeForDegassing();

	for( IDPTR IDPtr = pTabVssl->GetFirst(); NULL != IDPtr.MP; IDPtr = pTabVssl->GetNext( IDPtr.MP ) )
	{
		CSelectedPMBase::RejectedReasons rRejectedReasons;
		vecRejectedReasons.clear();
		CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( IDPtr.MP );

		if( NULL == pclVessel )
		{
			continue;
		}

		if( false == pclVessel->IsSelectable( true ) )
		{
			continue;
		}

		// Do not take vessel that are integrated in a TecBox (as primary vessel!).
		if( true == pclVessel->IsPrimaryVesselIntegrated() )
		{
			continue;
		}

		// Test compatibility flag.
		if( NULL != pclBaseVesselForIntermediate )
		{
			if( false == pclVessel->IsCompatibleOR( iPMCompatibilityFlag ) )
			{
				continue;
			}
		}
		else if( false == pclVessel->IsCompatibleAND( iPMCompatibilityFlag ) )
		{
			continue;
		}

		// HYS-1374: If tecbox can be installed only on a vessel, check also if this vessel is compatible.
		if( true == bTecBoxOnTopOnly && false == pclVessel->IsAcceptTecBoxOnTop() )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_XAT_XTP_CT_XVT_NOTECBOXONTOP;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMVSE_XAT_XTP_CT_XVT_NOTECBOXONTOP );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		// If user asks for expansion vessel with internal coating only, we must check this special compatibility flag.
		// Remark: it's only for the Compresso!
		// HYS-1537 : Look the disable state
		if( true == m_pclInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::CompressoInternalCoating )
				&& false == m_pclInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::CompressoInternalCoating )
				&& CDB_Vessel::ePMCompFlags::ePMComp_Compresso == ( iPMCompatibilityFlag & CDB_Vessel::ePMCompFlags::ePMComp_Compresso )
				&& CDB_Vessel::ePMCompFlags::ePMComp_VsslIntCoating != ( pclVessel->GetCompatibilityflags() & CDB_Vessel::ePMCompFlags::ePMComp_VsslIntCoating ) )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_INTCOAT_C_XVT_NOINTCOATVARIANT;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMVSE_INTCOAT_C_XVT_NOINTCOATVARIANT );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		if( pclVessel->GetTypeIDPtr().MP != pTypeID )
		{
			continue;
		}

		bool bCHVersion = ( 0 == TASApp.GetTADBKey().CompareNoCase( _T("CH") ) ) ? true : false;
		double dVesselPS = ( true == bCHVersion ) ? pclVessel->GetPSch() : pclVessel->GetPmaxmax();

		if( true == bCHVersion && 0.0 == dVesselPS )
		{
			dVesselPS = pclVessel->GetPmaxmax();
		}
		
		// Keep a trace of Pmax.
		// HYS-1040: if GetPsch() returns 0 take GetPmaxmax() value
		if( 0 == dVesselPS && true == bCHVersion )
		{
			dVesselPS = pclVessel->GetPmaxmax();
		}

		if( dVsslPmax < dVesselPS )
		{
			dVsslPmax = dVesselPS;
		}

		CSelectedVssl *pclSelectedVsl = new CSelectedVssl();
		pclSelectedVsl->SetCompatibilityFlag( (CDB_Vessel::ePMCompFlags)( iPMCompatibilityFlag & CDB_Vessel::ePMComp_Mask ) );
		pclSelectedVsl->SetProductIDPtr( pclVessel->GetIDPtr() );

		// Test vessel nominal volume.
		double dRequestedNominalVolume = 0.0;

		if( NULL == pclBaseVesselForIntermediate )
		{
			dRequestedNominalVolume = m_pclInputUser->GetVesselNominalVolume( bForTecBox );
		}
		else
		{
			CDB_Vessel *pclBaseVessel = dynamic_cast<CDB_Vessel*>( pclBaseVesselForIntermediate->GetProductIDPtr().MP );
			dRequestedNominalVolume = m_pclInputUser->GetNominalVolumeInterVssl( pclBaseVessel->GetTmin(), pclBaseVessel->GetTmax() );
		}

		dRequestedNominalVolume -= ( dRequestedNominalVolume / 100.0 * m_pTADS->GetpTechParams()->GetVesselSelectionRangePercentage() );
		int iNVessel = 1;

		// Check also the minimum vessel volume needed if degassing is asked.
		if( BST_CHECKED == m_pclInputUser->GetDegassingChecked() &&
			( 0 == CString( _T("VSSLTYPE_STAT") ).Compare( pTypeID->GetIDPtr().ID ) 
				|| 0 == CString( _T("VSSLTYPE_STATMEMBR") ).Compare( pTypeID->GetIDPtr().ID ) ) )
		{
			if( dRequestedNominalVolume < dSmallestMinVesselVolumeForDegassing )
			{
				dRequestedNominalVolume = dSmallestMinVesselVolumeForDegassing;
			}
		}

		if( pclVessel->GetNominalVolume() < dRequestedNominalVolume )
		{
			// Try to combine several vessel.
			iNVessel = ( INT )ceil( dRequestedNominalVolume / pclVessel->GetNominalVolume() );
		}

		pclSelectedVsl->SetNbreOfVsslNeeded( iNVessel );

		// Verify Pmax, should be greater than Pz // for Transfero vessel is set to atmospheric pressure
		if( 0 != CString( pclVessel->GetFamilyID() ).Compare( _T("VSSLFAM_TU") ) 
				&& 0 != CString( pclVessel->GetFamilyID() ).Compare( _T("VSSLFAM_TUE") )
				&& 0 != CString( pclVessel->GetFamilyID() ).Compare( _T("VSSLFAM_TG") )
				&& 0 != CString( pclVessel->GetFamilyID() ).Compare( _T("VSSLFAM_TGE") ) 
				&& 0 != CString( pclVessel->GetFamilyID() ).Compare( _T("VSSLFAM_TGI") )
				&& 0 != CString( pclVessel->GetFamilyID() ).Compare( _T("VSSLFAM_TGIE") ) 
				&& 0 != CString( pclVessel->GetFamilyID() ).Compare( _T("VSSLFAM_TGIH") ) )
		{
			if( m_pclInputUser->GetSafetyValveResponsePressure() > dVesselPS )
			{
				CString strPS = WriteCUDouble( _U_PRESSURE, dVesselPS, true );
				CString strPSV = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetSafetyValveResponsePressure(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVSE_XTP_VVMC_XVT_PSBELOWPSV, strPS, strPSV );
				rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_XTP_VVMC_XVT_PSBELOWPSV;
				vecRejectedReasons.push_back( rRejectedReasons );
			}
		}

		if( CDB_Vessel::ePMComp_Compresso == ( iPMCompatibilityFlag & CDB_Vessel::ePMComp_Compresso ) )
		{
			// Additional test for SWKI HE301-01: PSCH = maximum allowed pressure for Switzerland: pressure up to which
			// the expansion vessel must not subject to authorization, according to Swiss directive SICC 93-1.
			// It is in fact the limit of the pressure.volume if activated in the technical parameters.
			// In the Switzerland catalog version, we have a PSCH column for the CU/CG Compresso vessels.

			// HYS-1003: We can force to derogate to this rule. It can happen that there is no result with big installation for example. In this case
			// we will show results with a warning.
			if( false == bUnlockPressureVolumeLimit )
			{
				// Don't use vessel that has its PSch bigger than the PStecbox.
				// And don't use bigger vessel that has its nominal volume multiplied by PStecbox bigger the pressure.volume limit.
				// HYS-1402 : Use IsVolumeRespectsPressureVolumeLimit() function to consider GetPmaxmax > 2 bar
				if( NULL != pCurrentTBSelected && CSelectPMList::eRejectVessel ==
					IsVolumeRespectsPressureVolumeLimit( pclVessel, pCurrentTBSelected->GetPmaxmax() ) )
				{
					// Set the text for the 'pressure.volume' unit.
					CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
					CString strUnit = pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str();
					strUnit += CString( _T(".") ) + pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str();

					// Value in 'localdb.txt' is set in SI units (i.e. Pascal.m3). To display value with the current unit, we must convert value SI to
					// CU in 2 phases. First one for the pressure, and second one for the volume.
					double dPressVolLimitSI = pclVessel->GetNominalVolume() * pCurrentTBSelected->GetPmaxmax();
					double dPressVolLimitCUP = CDimValue::SItoCU( _U_PRESSURE, dPressVolLimitSI );
					double dPressVolLimitCUPV = CDimValue::SItoCU( _U_VOLUME, dPressVolLimitCUP );
					CString strPresVolVessel = WriteCUDouble( _U_NODIM, dPressVolLimitCUPV );
					strPresVolVessel += _T(" ") + strUnit;
				
					CString strPresVolLimit = m_pTADS->GetpTechParams()->GetPressureVolumeLimitCU().c_str();
				
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVSE_PRESSVOL_XPM_XVT_PRESSVOLLIMITREACHED, strPresVolVessel, strPresVolLimit );
					rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_PRESSVOL_XPM_XVT_PRESSVOLLIMITREACHED;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
			}
		}
		else if( NULL == pclBaseVesselForIntermediate )
		{
			// HYS-1003: We can force to derogate to this rule. It can happen that there is no result with big installation for example. In this case
			// we will show results with a warning.
			if( false == bUnlockPressureVolumeLimit )
			{
				// Don't use vessel that has its nominal volume multiply by PSV bigger than the value in technical parameter.
				// HYS-1402 : Use IsVolumeRespectsPressureVolumeLimit() function to consider GetSafetyValveResponsePressure > 2 bar
				if( CSelectPMList::eRejectVessel == 
					IsVolumeRespectsPressureVolumeLimit( pclVessel, m_pclInputUser->GetSafetyValveResponsePressure() ) )
				{
					// Set the text for the 'pressure.volume' unit.
					CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
					CString strUnit = pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str();
					strUnit += CString( _T(".") ) + pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str();

					// Value in 'localdb.txt' is set in SI units (i.e. Pascal.m3). To display value with the current unit, we must convert value SI to
					// CU in 2 phases. First one for the pressure, and second one for the volume.
					double dPressVolLimitSI = pclVessel->GetNominalVolume() * m_pclInputUser->GetSafetyValveResponsePressure();
					double dPressVolLimitCUP = CDimValue::SItoCU( _U_PRESSURE, dPressVolLimitSI );
					double dPressVolLimitCUPV = CDimValue::SItoCU( _U_VOLUME, dPressVolLimitCUP );
					CString strPresVolVessel = WriteCUDouble( _U_NODIM, dPressVolLimitCUPV );
					strPresVolVessel += _T(" ") + strUnit;
				
					CString strPresVolLimit = m_pTADS->GetpTechParams()->GetPressureVolumeLimitCU().c_str();
				
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVSE_PRESSVOL_XPM_XVT_PRESSVOLLIMITREACHED, strPresVolVessel, strPresVolLimit );
					rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_PRESSVOL_XPM_XVT_PRESSVOLLIMITREACHED;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
			}

			if( CDB_Vessel::ePMComp_Statico == ( CDB_Vessel::ePMComp_Statico & iPMCompatibilityFlag ) )
			{
				pclSelectedVsl->SetInitialNbreOfVsslNeeded( iNVessel );

				// Compute minimum initial pressure.
				// Remark: first with the minimum water reserve.
				double dPaMin = m_pclInputUser->ComputeInitialPressure( pclVessel->GetNominalVolume() * iNVessel, m_pclInputUser->GetMinimumWaterReserve() );
			
				// HYS-1022: 0.3 bar margin for initial pressure (EN 12828-2014)
				// HYS-1116: Pa,min must respect the EN12828 norm: pa,min >= p0 + 0.3 bar.
				// HYS-1198: It's only for heating !!
				if( m_pclInputUser->IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12828 )
					&& dPaMin < m_pclInputUser->GetMinimumPressure() + m_pTADS->GetpTechParams()->GetInitialPressureMargin() )
				{
					// Pamin must satisfy the rule >= p0 + 0.3.
					dPaMin = m_pclInputUser->GetMinimumPressure() + m_pTADS->GetpTechParams()->GetInitialPressureMargin();
					bool bFound = false;
					
					// We must increase number of vessel until we satisfy the EN12828 norm.
					do 
					{
						double dVwrOptimized = m_pclInputUser->GetWaterReserve( pclVessel->GetNominalVolume() * iNVessel );
						double dVwrPushed = m_pclInputUser->GetWaterReservePushed( pclVessel->GetNominalVolume() * iNVessel, dPaMin );

						if( dVwrPushed <= dVwrOptimized )
						{
							bFound = true;
						}
						else
						{
							iNVessel++;
						}

					}while( false == bFound );
				}

				pclSelectedVsl->SetNbreOfVsslNeeded( iNVessel );
				pclSelectedVsl->SetMinimumInitialPressure( dPaMin );

				// Compute the water reserve (It's the optimized one).
				// If we are in the cooling mode, the 'GetWaterReserve' method will remove the contracted volume needed.
				double dVwr = m_pclInputUser->GetWaterReserve( pclVessel->GetNominalVolume() * iNVessel );
				pclSelectedVsl->SetWaterReserve( dVwr );

				// Compute initial pressure.
				double dPa = m_pclInputUser->ComputeInitialPressure( pclVessel->GetNominalVolume() * iNVessel, dVwr );
				pclSelectedVsl->SetInitialPressure( dPa );
			}
		}

		// Store delta between requested volume and vessel nominal volume.
		double dDeltaVN = ( pclVessel->GetNominalVolume() * iNVessel ) - dRequestedNominalVolume;
		pclSelectedVsl->SetDelta( dDeltaVN );

		// Testing diameter.
		double dDiameter = pclVessel->GetDiameter();
		bool bSecDiameter = false;

		if( true == bPrimaryVessel && iNVessel > 1 && NULL != pclVessel->GetAssociatedSecondaryVessel() )
		{
			// In some case secondary vessel is higher than primary vessel.
			double dSecDiameter = pclVessel->GetAssociatedSecondaryVessel()->GetDiameter();

			if( dSecDiameter > dDiameter )
			{
				dDiameter = dSecDiameter;
				bSecDiameter = true;
			}
		}

		if( m_pclInputUser->GetMaxWidth() > 0 && dDiameter > m_pclInputUser->GetMaxWidth() )
		{
			CString strDiameter = WriteCUDouble( _U_LENGTH, dDiameter, true );
			CString strMaxWidth = WriteCUDouble( _U_LENGTH, m_pclInputUser->GetMaxWidth(), true );

			// Adapt message in regards to the concerned vessel.
			int iID = ( false == bSecDiameter ) ? IDS_SELECTPMVSE_XTP_XPM_XVT_DIAMTOOHIGH : IDS_SELECTPMVSE_XTP_XPM_XVT_DIAMTOOHIGHSEC;
			FormatString( rRejectedReasons.m_strErrorMsg, iID, strDiameter, strMaxWidth );
			
			rRejectedReasons.m_iIDError = iID;
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		// Testing height if needed.
		if( m_pclInputUser->GetMaxHeight() > 0.0 )
		{
			double dTotalHeight = pclVessel->GetHeight();
			bool bHeightWithTecBox = false;

			// Check if we need a TecBox.
			if( NULL != pCurrentTBSelected && 0.0 != dTecBoxHeight 
					&& CDB_Vessel::ePMComp_Compresso == ( CDB_Vessel::ePMComp_Compresso & iPMCompatibilityFlag  )
					&& true == pclVessel->IsAcceptTecBoxOnTop()
					&& true == pCurrentTBSelected->IsInstallationLayoutVesselTop() )
			{
				// Add height of tech box if fixed on top of main vessel.
				dTotalHeight += dTecBoxHeight;
				bHeightWithTecBox = true;
			}

			bool bHeightSec = false;

			// If we are on the primary vessel by there is at least one secondary vessel...
			if( true == bPrimaryVessel && iNVessel > 1 )
			{
				CDB_Vessel *pSecVssl = pclVessel->GetAssociatedSecondaryVessel();

				// In some case secondary vessel is higher than primary vessel.
				if( NULL != pSecVssl )
				{
					double dSecHeight = pSecVssl->GetHeight();

					if( dSecHeight > dTotalHeight )
					{
						dTotalHeight = dSecHeight;
						bHeightSec = true;
					}
				}
			}

			// Check the possibility to stand-up the vessel (With TecBox if available).
			double dHeightNeeded = sqrt( pow( dTotalHeight, 2 ) + pow( dDiameter, 2 ) );

			if( dHeightNeeded > m_pclInputUser->GetMaxHeight() )
			{
				CString strHeight = WriteCUDouble( _U_LENGTH, dHeightNeeded, true );
				CString strMaxHeight = WriteCUDouble( _U_LENGTH, m_pclInputUser->GetMaxHeight(), true );

				// We can have here three cases: 
				// bHeightWithTecBox | bHeightSec | 
				// ------------------+------------+
				//       false       |   false    | The concerned vessel is the main one.
				//       true        |   false    | The concerned vessel is the main one with a TecBox on it.
				//       false       |   true     | The secondary vessel is higher than the primary.
				//       true        |   true     | The concerned vessel is the secondary one (Higher in this case than the primary vessel AND TecBox).
				int iID = 0;

				if( false == bHeightWithTecBox && false == bHeightSec )
				{
					// Main vessel.
					iID = IDS_SELECTPMVSE_XTP_XPM_XVT_HEIGHTTOOHIGH;
				}
				else if( true == bHeightWithTecBox && false == bHeightSec )
				{
					// Main vessel + TecBox.
					iID = IDS_SELECTPMVSE_XTP_XPM_XVT_HEIGHTTOOHIGHTECBOX;
				}
				else
				{
					// Secondary vessel.
					iID = IDS_SELECTPMVSE_XTP_XPM_XVT_HEIGHTTOOHIGHSEC;
				}

				FormatString( rRejectedReasons.m_strErrorMsg, iID, strHeight, strMaxHeight );
				rRejectedReasons.m_iIDError = iID;
				vecRejectedReasons.push_back( rRejectedReasons );
			}
		}

		// Check now the temperatures.
		int iTempErrorCode;

		if( true == IsErrorTemperature( pclVessel, &iTempErrorCode ) )
		{
			// HYS-1437: when it's an intermediate vessel, we can choose it if the return temperature is bigger that the limit of the vessel.
			if( ( CSelectedPMBase::TEC_SystemReturnTempBiggerTBMaxTemp == ( CSelectedPMBase::TEC_SystemReturnTempBiggerTBMaxTemp | iTempErrorCode ) )
					&& CDB_Vessel::VsslType::eVsslType_Interm == pclVessel->GetVesselType() )
			{
				CString strReturnTemp = WriteCUDouble( _U_TEMPERATURE, m_pclInputUser->GetReturnTemperature(), true );
				CString strVesselIntermediaireMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclVessel->GetTmax(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVSE_HS_NOTP_V_I_RETURNTEMPTOOHIGH, strReturnTemp, strVesselIntermediaireMaxTemp );
				rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_HS_NOTP_V_I_RETURNTEMPTOOHIGH;
				vecRejectedReasons.push_back( rRejectedReasons );
			}
			else
			{
				pclSelectedVsl->SetFlag( CSelectedPMBase::eTemperature, true, iTempErrorCode );
			}
		}

		// HYS-1428: Special case for Transfero TI. When TAZ > 110 °C we must use TGI...H as primary vessel.
		if( CDB_Vessel::ePMComp_TransferoTI == ( CDB_Vessel::ePMComp_TransferoTI & iPMCompatibilityFlag ) 
				&& CDB_Vessel::VsslType::eVsslType_Prim == pclVessel->GetVesselType() )
		{
			if( m_pclInputUser->GetSafetyTempLimiter() > m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953() )
			{
				if( 0 != IDcmp( _T("VSSLFAM_TGIH"), pclVessel->GetFamilyID() ) )
				{
					CString strSafetyTempLimiter = WriteCUDouble( _U_TEMPERATURE, m_pclInputUser->GetSafetyTempLimiter(), true );
					CString strSafetyTempLimiterLimit = WriteCUDouble( _U_TEMPERATURE, m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953(), true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPTBSE_HS_NOTP_T_XP_TEMPTOHIGHFORTHISVESSEL, strSafetyTempLimiter, strSafetyTempLimiterLimit );
					rRejectedReasons.m_iIDError = IDS_SELECTPTBSE_HS_NOTP_T_XP_TEMPTOHIGHFORTHISVESSEL;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
			}
		}

		if( vecRejectedReasons.size() > 0 )
		{
			pclSelectedVsl->AddRejectedReasons( vecRejectedReasons );
		}

		// We put all vessels in the 'm_vSelectedPMList' to allow us to keep vessel that are bigger (At least two sizes above) that the best one.
		m_vSelectedPMlist.push_back( pclSelectedVsl );
	}

	return ( m_vSelectedPMlist.size() );
}

double CSelectPMList::GetSmallestMinVesselVolumeForDegassing()
{
	double dSmallestMinVesselVolumeForDegassing = DBL_MAX;
	CTable *pTab = (CTable *)( m_pTADB->Get( _T("TECBOX_TAB") ).MP );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_TBPlenoVento *pclTecBoxVento = dynamic_cast<CDB_TBPlenoVento *>( idptr.MP );

		if( NULL == pclTecBoxVento )
		{
			continue;
		}

		if( false == pclTecBoxVento->IsSelectable( true ) )
		{
			continue;
		}

		// Don't take degassing and water make-up that are combined with a Compresso or a Transfero. These ones are selected in
		// the '_SelectTecBox' method.
		if( true == pclTecBoxVento->IsFctCompressor() || true == pclTecBoxVento->IsFctPump() )
		{
			continue;
		}

		// If TecBox has not the degassing property, we reject it.
		if( false == pclTecBoxVento->IsFctDegassing() )
		{
			continue;
		}

		double dMinVesselVolume = pclTecBoxVento->GetMinimumVesselVolume( m_pclInputUser->GetFinalPressure() );

		if( dMinVesselVolume < dSmallestMinVesselVolumeForDegassing )
		{
			dSmallestMinVesselVolumeForDegassing = dMinVesselVolume;
		}
	}

	return dSmallestMinVesselVolumeForDegassing;
}

int CSelectPMList::_VerifyIntegratedVessel( CDB_TecBox *pclCompresso )
{
	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	// This is a particular case. Simply Compresso has already an integrated vessel. If this TecBox has been selected, it means
	// that we have already check if the integrated vessel is correct (with '_CheckSimplyCompressoIntegratedVessel').
	// Here, we check if we need the secondary vessel.

	if( NULL == m_pclInputUser || NULL == pclCompresso || NULL == dynamic_cast<CDB_Vessel *>( pclCompresso->GetIntegratedVesselIDPtr().MP ) )
	{
		return -1;
	}

	CDB_Vessel *pclIntegratedVessel = dynamic_cast<CDB_Vessel *>( pclCompresso->GetIntegratedVesselIDPtr().MP );

	m_vSelectedPMlist.clear();
	m_vRejectedPMlist.clear();

	CString strReason;
	std::vector<CString> vecRejectedReasons;
	vecRejectedReasons.clear();

	// Test vessel nominal volume.
	double dRequestedNominalVolume = m_pclInputUser->GetVesselNominalVolume( true );
	dRequestedNominalVolume -= (dRequestedNominalVolume / 100.0 * m_pTADS->GetpTechParams()->GetVesselSelectionRangePercentage() );

	int iNVessel = ( INT )ceil( dRequestedNominalVolume / pclIntegratedVessel->GetNominalVolume() );
	double dDeltaVN = ( pclIntegratedVessel->GetNominalVolume() * iNVessel ) - dRequestedNominalVolume;

	if( iNVessel > ( pclCompresso->GetMaximumSecondaryVessels() + 1 ) )
	{
		// Must not happen. Because this case if filtered out when calling the '_CheckSimplyCompressoIntegratedVessel' method in
		// '_SelectTecBox'.
		return -1;
	}

	CSelectedVssl *pclSelectedVsl = new CSelectedVssl();
	pclSelectedVsl->SetCompatibilityFlag( CDB_Vessel::ePMComp_Compresso );
	pclSelectedVsl->SetProductIDPtr( pclIntegratedVessel->GetIDPtr() );
	pclSelectedVsl->SetDelta( dDeltaVN );
	pclSelectedVsl->SetNbreOfVsslNeeded( iNVessel );
	pclSelectedVsl->SetPrimaryIntegrated( true );

	int iTempErrorCode;

	if( true == IsErrorTemperature( pclIntegratedVessel, &iTempErrorCode ) )
	{
		pclSelectedVsl->SetFlag( CSelectedPMBase::eTemperature, true, iTempErrorCode );
	}

	// Add selected object to the list.
	m_vSelectedPMlist.push_back( pclSelectedVsl );

	// Only one solution exist for Simply Compresso.
	return 1;
}

int CSelectPMList::_SelectTecBox( CStringArray *parTypeID, CDB_TecBox::TecBoxType Tbt )
{
	if( NULL == m_pclInputUser || NULL == m_pclInputUser->GetPMWQSelectionPreferences() || NULL == m_pTADB || NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CPMWQPrefs *pclPMWQSelectionPrefs = m_pclInputUser->GetPMWQSelectionPreferences();

	m_vSelectedPMlist.clear();
	m_vRejectedPMlist.clear();

	std::vector<CSelectedPMBase::RejectedReasons> vecRejectedReasons;

	// 'dEqualizingVolumetricFlow' compensation flow [m3/Ws].
	double dEqualizingVolumetricFlow = m_pclInputUser->GetEqualizingVolumetricFlow();

	// Working point qN = dEqualizingVolumetricFlow * Q.
	double dqN = m_pclInputUser->GetVD();
	double dPman = m_pclInputUser->GetTargetPressureForTecBox( Tbt );

	// We will pass through all TecBox to find TecBox with a compliant working curve.
	CTable *pTab = (CTable *)( m_pTADB->Get( _T("TECBOX_TAB") ).MP );

	// Prepare some variables.
	bool bDegassingWanted = ( BST_CHECKED == m_pclInputUser->GetDegassingChecked() ) ? true : false;
	bool bWaterMakeupWanted = ( 0 != CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_NONE") ) ) ? true : false;

	CString strCombined = TASApp.LoadLocalizedString( IDS_RBN_PS_PMWQ_ONEDEVICE );

	for( int iar = 0; iar < parTypeID->GetCount(); iar++ )
	{
		CDB_StringID *pTypeID = (CDB_StringID *)( m_pTADB->Get( ( LPCTSTR ) parTypeID->GetAt( iar ) ).MP );

		// Sanity.
		if( NULL == pTab || NULL == pTypeID )
		{
			return 0;
		}

		// Loop on available TecBox.
		for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
		{
			CSelectedPMBase::RejectedReasons rRejectedReasons;
			vecRejectedReasons.clear();
			CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( idptr.MP );

			if( NULL == pclTecBox )
			{
				continue;
			}

			if( pTypeID != pclTecBox->GetTypeIDPtr().MP )
			{
				continue;
			}

			if( false == pclTecBox->IsSelectable( true ) )
			{
				continue;
			}

			if( ProjectType::Heating == m_pclInputUser->GetApplicationType() )
			{
				if(	m_pclInputUser->GetSafetyTempLimiter() > m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953() )		// Check TAZ
				{
					// HYS-1480 : Show rejected product in rejected list with a tooltip message
					// When we are working with TAZ > 110, only Transfero TI Connect are allowed
					if( 0 != CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TIC") ) )
					{
						CString strTemp = CteEMPTY_STRING;
						CString strSafetyTempLimiter = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSAFETYTEMP );
						CString strSafetyTempLimiterLimit = WriteCUDouble( _U_TEMPERATURE, m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953(), true );
						FormatString( strTemp, TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_H_ERRORTEMPLIMIT ), strSafetyTempLimiter, strSafetyTempLimiterLimit );
						rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_H_ERRORTEMPLIMIT;
						rRejectedReasons.m_strErrorMsg = strTemp;
						vecRejectedReasons.push_back( rRejectedReasons );
					}
				}
			}

			// Included functions.
			bool bTecBoxFctDegassing = pclTecBox->IsFctDegassing();
			bool bTecBoxFctWaterMakeup = pclTecBox->IsFctWaterMakeUp();
			bool bTecBoxFctCompressor = pclTecBox->IsFctCompressor();
			bool bTecBoxFctPump = pclTecBox->IsFctPump();

			// Excluded functions.
			bool bTecBoxFctExcludedDegassing = pclTecBox->IsFctExcludedDegassing();
			bool bTecBoxFctExcludedWaterMakeup = pclTecBox->IsFctExcludedWaterMakeUp();

			bool bTecBoxVariantCooling = pclTecBox->IsVariantCooling();
			bool bTecBoxVariantBreakTank = pclTecBox->IsVariantBreakTank();
			bool bTecBoxVariantExtAir = pclTecBox->IsVariantExternalAir();
			bool bTecBoxVariantIntBufferVessel = pclTecBox->IsVariantIntegratedBufferVessel();
			bool bTecBoxVariantHighFlowTemp = pclTecBox->IsVariantHighFlowTemperature();

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// General preferences.

			// If user wants a redundancy on compressors/pumps, we need to reject TecBox that have only one Compressor/pump.
			// HYS-1537 : Look the disable state
			if( true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) 
				&& pclTecBox->GetNbrPumpCompressor() < 2
				&& false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp ) )
			{
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_REDPC_XPM_XVP_HASONECOMP;
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_REDPC_XPM_XVP_HASONECOMP );
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// Don't show cooling version if we are in heating or solar.
			if( ProjectType::Cooling != m_pclInputUser->GetApplicationType() && true == bTecBoxVariantCooling  )
			{
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_HS_XTP_XPM_XVP_COOLVARIANT;
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_HS_XTP_XPM_XVP_COOLVARIANT );
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Pump-based pressure maintenance preferences.

			// HYS-866: The 'Integrated buffer vessel' option is no more needed.

			// HYS-1712: Add verification for Tranfero TVI and TI Connect (see Pflichtenheft_selectP_Hyselect_Algorithmus 20210415.xlsx).
			if( ProjectType::Cooling == m_pclInputUser->GetApplicationType() 
				&& ( 0 == CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TV_C") )
				|| 0 == CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TVI_C") ) 
				|| 0 == CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TIC") ) ) )
			{
				// HYS-981: We don't take Transfero TV Connect without cooling version if we are in cooling AND if user has asked to get
				//          Transfero for cooling version.
				// HYS-1537 : Look the disable state
				if( true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion )
				    && false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion )
					&& false == bTecBoxVariantCooling )
				{
					// This device has not the cooling insulation with condensation water protection.
					rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_C_XTP_xPM_XVP_NOCOOLVARIANT );
					rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_C_XTP_xPM_XVP_NOCOOLVARIANT;
					vecRejectedReasons.push_back( rRejectedReasons );
				}

				// HYS-981: We don't take Transfero TV connect with cooling version if it is not needed.
				// HYS-1537 : Look the disable state
				if( ( false == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion )
					  || true == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion ) )
					&& true == bTecBoxVariantCooling )
				{
					// The cooling insulation with condensation water protection is not needed here.
					rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_C_XTP_XPM_XVP_COOLVARIANTNOTNEEDED );
					rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_C_XTP_XPM_XVP_COOLVARIANTNOTNEEDED;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Compressor-based pressure maintenance preferences.

			// If user wants exclusively Compressor working with external compressed air, we must exclude others.
			// HYS-1537 : Look the disable state
			if( true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::CompressoExternalAir )
			    && false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::CompressoExternalAir ) 
				&& true == bTecBoxFctCompressor
				&& false == bTecBoxVariantExtAir )
			{
				// This device does not work with external compressed air.
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_CX_C_XVP_NOTEXTERNALAIR );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_CX_C_XVP_NOTEXTERNALAIR;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Degassing preferences.
			if( true == bDegassingWanted && true == bTecBoxFctExcludedDegassing )
			{
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_COMBINED_XPM_V_DEGEXCLUDED );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_COMBINED_XPM_V_DEGEXCLUDED;
				vecRejectedReasons.push_back( rRejectedReasons );
			}
			
			// HYS-1537 : Look the disable state.
			if( true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice )
			    && false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) 
				&& true == bDegassingWanted
				&& false == bTecBoxFctDegassing )
			{
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_COMBINED_XPM_V_NODEGFCT );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_COMBINED_XPM_V_NODEGFCT;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// In cooling, if user wants degassing with cooling version BUT the TecBox has not this variant, we can only take the current TecBox
			// if user doesn't want combined function in one device. TV, TPV and CPV (in normal version) can't be accepted but well T & TI. In this case the
			// Vento will be selected alone when user will choose the Transfero.
			// HYS-1537 : Look the disable state.
			if( true == bDegassingWanted && ProjectType::Cooling == m_pclInputUser->GetApplicationType() && false == bTecBoxVariantCooling
					&& true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion ) 
					&& false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion ) 
					&& true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice )
					&& false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) )
			{
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_C_COMBINEDVCV_XPM_V_NOCOOLVARIANT );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_C_COMBINEDVCV_XPM_V_NOCOOLVARIANT;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Water make-up preferences.
			// HYS-1832: If the tecbox has bTecBoxFctExcludedWaterMakeup = true but degassing function selected then the water make-up
			// could be possible with the vento.
			if( true == bWaterMakeupWanted && true == bTecBoxFctExcludedWaterMakeup && false == bDegassingWanted )
			{
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_COMBINED_XPM_P_WMUPEXCLUDED );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_COMBINED_XPM_P_WMUPEXCLUDED;
				vecRejectedReasons.push_back( rRejectedReasons );
			}
			
			// HYS-1537 : Look the disable state.
			if( true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) && true == bWaterMakeupWanted
					&& false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice )
					&& false == bTecBoxFctWaterMakeup )
			{
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_COMBINED_XPM_P_NOPFCT );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_COMBINED_XPM_P_NOPFCT;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// If user wants break-tank for water make-up, we must exclude others.
			// HYS-1537 : Look the disable state.
			if( true == bWaterMakeupWanted && true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::WaterMakeupBreakTank )
					&& false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::WaterMakeupBreakTank )
					&& false == bTecBoxVariantBreakTank && true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice )
					&& false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) )
			{
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_COMBINEDPBKTK_XPM_P_NOBKTKVARIANT );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_COMBINEDPBKTK_XPM_P_NOBKTKVARIANT;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// If user wants duty and stand-by of make-up pumps, we must exclude others.
			// HYS-1537 : Look the disable state.
			if( true == bWaterMakeupWanted && true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy )
					&& false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::WaterMakeupDutyStandBy )
					&& pclTecBox->GetNbrPumpCompressor() < 2 && true == pclPMWQSelectionPrefs->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice )
					&& false == pclPMWQSelectionPrefs->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) )
			{
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_PDUTYSTBY_XPM_P_NODUTYSTBYVARIANT );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_PDUTYSTBY_XPM_P_NODUTYSTBYVARIANT;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


			if( m_pclInputUser->GetSafetyValveResponsePressure() > pclTecBox->GetPmaxmax() )
			{
				CString strPS = WriteCUDouble( _U_PRESSURE, pclTecBox->GetPmaxmax(), true );
				CString strPSV = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetSafetyValveResponsePressure(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_PSBELOWPSV, strPSV, strPS );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_PSBELOWPSV;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			if( m_pclInputUser->GetMaxWidth() > 0 && pclTecBox->GetWidth() > m_pclInputUser->GetMaxWidth()
					&& pclTecBox->GetDepth() > m_pclInputUser->GetMaxWidth() )
			{
				CString strWidth = WriteCUDouble( _U_LENGTH, pclTecBox->GetWidth() );
				CString strDepth = WriteCUDouble( _U_LENGTH, pclTecBox->GetDepth() );
				CString strWidthDepth = strWidth + _T("/") + strDepth;
				CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
				CString strUnit = pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str();
				strWidthDepth += _T(" ") + strUnit;
				CString strMaxWidth = WriteCUDouble( _U_LENGTH, m_pclInputUser->GetMaxWidth(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_WIDTHTOOHIGH, strWidthDepth, strMaxWidth );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_WIDTHTOOHIGH;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			if( m_pclInputUser->GetMaxHeight() > 0 && pclTecBox->GetHeight() > m_pclInputUser->GetMaxHeight() )
			{
				CString strHeight = WriteCUDouble( _U_LENGTH, pclTecBox->GetHeight(), true );
				CString strMaxHeight = WriteCUDouble( _U_LENGTH, m_pclInputUser->GetMaxHeight(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HEIGHTTOHIGH, strHeight, strMaxHeight );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HEIGHTTOHIGH;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// HYS-599: We have also curve for Compresso CX
			double dPDelta = DBL_MAX;
			double dqNmax = DBL_MAX;
			CDB_TBCurve *pCurve = dynamic_cast<CDB_TBCurve *>( pclTecBox->GetPumpComprCurveIDPtr().MP );

			if( NULL == pCurve )
			{
				continue;
			}

			bool bRejected = false;

			// Test qNmax.
			// HYS-599: For Compresso CX (With external compressed air) we can put devices in parallel.
			if( false == bTecBoxVariantExtAir && dqN > pCurve->GetqNmax() )
			{
				CString strqN = WriteCUDouble( _U_FLOW, dqN, true );
				CString strqNMax = WriteCUDouble( _U_FLOW, pCurve->GetqNmax(), true );

				// "Computed volumetric flow exceed qNmax of this TecBox (%1 > %2)."
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_COMPUTEDQNBIGGERQNMAX, strqN, strqNMax );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_COMPUTEDQNBIGGERQNMAX;
				vecRejectedReasons.push_back( rRejectedReasons );
				bRejected = true;
			}

			if( dPman > pCurve->GetPmax() )
			{
				// Test Pmax.
				CString strPMan = WriteCUDouble( _U_PRESSURE, dPman, true );
				CString strPMax = WriteCUDouble( _U_PRESSURE, pCurve->GetPmax(), true );

				// "Computed target pressure is above maximum pressure allowed for this TecBox (%1 > %2)."
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_COMPUTEDPMANBIGGERPMAX, strPMan, strPMax );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_COMPUTEDPMANBIGGERPMAX;
				vecRejectedReasons.push_back( rRejectedReasons );
				bRejected = true;
			}
			else if( dPman < pCurve->GetPmin() )
			{
				// Test Pmin.
				CString strPMin = WriteCUDouble( _U_PRESSURE, dPman, true );
				CString strPMax = WriteCUDouble( _U_PRESSURE, pCurve->GetPmin(), true );

				// "Computed target pressure is below the minimum pressure allowed for this TecBox (%1 < %2)."
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_COMPUTEDPMANLOWERPMIN, strPMin, strPMax );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_COMPUTEDPMANLOWERPMIN;
				vecRejectedReasons.push_back( rRejectedReasons );
				bRejected = true;
			}
			else 
			{
				// Test working point.
				if( false == bTecBoxVariantExtAir )
				{
					if( dPman > pCurve->GetBoundedY( dqN, 0 ) || ( 2 == pCurve->GetFuncNumber() && dPman < pCurve->GetBoundedY( dqN, 1) ) )
					{
						// We have only one curve defined for Compresso and 2 for Transfero.
						// Remark: the curve 0 is the above curve and the curve 1 is the below curve.

						// "Computed working point is not covered by this TecBox."
						rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_WORKPTBELOWCURVE );
						rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_WORKPTBELOWCURVE;
						vecRejectedReasons.push_back( rRejectedReasons );
						bRejected = true;
					}
				}
				else
				{
					// HYS-599: We have now also curves for Compresso CX. For CX-6 and CX-10 we have two curves, and for CX-16 three.
					// We test only on the last curve.
					if( dPman > pCurve->GetBoundedY( dqN, pCurve->GetFuncNumber() - 1 ) )
					{
						// "Computed working point is not covered by this TecBox."
						rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_WORKPTBELOWCURVE );
						rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_WORKPTBELOWCURVE;
						vecRejectedReasons.push_back( rRejectedReasons );
						bRejected = true;
					}
				}
			}

			if( false == bRejected )
			{
				dPDelta = min( dPDelta, pCurve->GetPmax() );
				dqNmax = pCurve->GetqNmax();
			}

			if( true == pclTecBox->IsVariantIntegratedPrimaryVessel() )
			{
				// Case of the Simply Compresso for example. We need to check if the integrated primary vessel match with
				// the system.
				_CheckSimplyCompressoIntegratedVessel( pclTecBox, &vecRejectedReasons );
			}

			CSelectedPMBase *pSelTecBox = NULL;

			if( Tbt == CDB_TecBox::etbtCompresso )
			{
				CSelectedCompresso *pSelCompresso = new CSelectedCompresso;

				if( NULL == pSelCompresso )
				{
					continue;
				}

				pSelTecBox = pSelCompresso;

				// HYS-599: Check how many Compresso CX we need in parallel.
				if( true == bTecBoxVariantExtAir )
				{
					// HYS-1121 : Verify Nul value before divided
					if( dqN != 0 && pCurve->GetqNmax() != 0 )
					{
						pSelCompresso->SetCompressoCXNumber( (int)ceil( dqN / pCurve->GetqNmax() ) );
					}
					else
					{
						pSelCompresso->SetCompressoCXNumber( 1 );
					}
				}
			}
			else
			{
				CSelectedTransfero *pSelTransfero = new CSelectedTransfero;

				if( NULL == pSelTransfero )
				{
					continue;
				}

				pSelTecBox = pSelTransfero;

				std::map<int, CSelectedTransfero::BufferVesselData> mapBufferVesselList;
				GetTransferoBufferVesselList( pclTecBox, &mapBufferVesselList );
				pSelTransfero->SetBufferVesselList( mapBufferVesselList );
			}

			int iTempErrorCode;

			if( true == IsErrorTemperature( pclTecBox, &iTempErrorCode ) )
			{
				pSelTecBox->SetFlag( CSelectedPMBase::eTemperature, true, iTempErrorCode );
			}

			if( true == bTecBoxFctWaterMakeup )
			{
				// Check water make-up temperature.
				// Remark: for Transfero/Compresso we don't have the min. admissible temperature for the integrated water make-up part.
				if( m_pclInputUser->GetWaterMakeUpWaterTemp() > pclTecBox->GetTmaxWaterMakeUp() )
				{
					pSelTecBox->SetFlag( CSelectedPMBase::ePlenoWMUpMaxTemp, true );
				}
			}

			pSelTecBox->SetProductIDPtr( idptr );
			pSelTecBox->SetDelta( dPDelta );
			pSelTecBox->SetqNax( dqNmax );

			if( 0 == (int)vecRejectedReasons.size() )
			{
				m_vSelectedPMlist.push_back( pSelTecBox );
			}
			else
			{
				pSelTecBox->SetRejected();
				pSelTecBox->AddRejectedReasons( vecRejectedReasons );
				m_vRejectedPMlist.push_back( pSelTecBox );
			}
		}
	}

	return m_vSelectedPMlist.size();
}

int CSelectPMList::_SelectVento( CSelectedPMBase *pSelectedPM )
{
	if( NULL == m_pclInputUser || NULL == m_pTADB || NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( 0 );
	}

	_CleanVecKeyPMBase( ProductType::PT_Vento );
	m_vSelectedPMlist.clear();
	m_vRejectedPMlist.clear();

	std::vector<CSelectedPMBase::RejectedReasons> vecRejectedReasons;

	if( BST_UNCHECKED == m_pclInputUser->GetDegassingChecked() )
	{
		return 0;
	}

	CTable *pTab = ( CTable * )( m_pTADB->Get( _T("TECBOX_TAB") ).MP );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	// If there is a TecBox selected but the product IDPTR is NULL, we exit.
	if( NULL != pSelectedPM && _NULL_IDPTR == pSelectedPM->GetProductIDPtr() )
	{
		return 0;
	}

	CPMWQPrefs *pclPMWQSelectionPreferences = m_pclInputUser->GetPMWQSelectionPreferences();
	ASSERT( NULL != pclPMWQSelectionPreferences );

	if( NULL == pclPMWQSelectionPreferences )
	{
		return 0;
	}

	CDB_TecBox *pclSelectedTecBox = NULL;
	
	if( NULL != pSelectedPM )
	{
		pclSelectedTecBox = dynamic_cast<CDB_TecBox *>( pSelectedPM->GetProductIDPtr().MP );
	}

	// If Vento is selected with a TecBox (Compresso or Transfero)
	if( NULL != pclSelectedTecBox )
	{
		if( true == pclSelectedTecBox->IsFctDegassing() )
		{
			// If selected TecBox has degassing device integrated, we don't select Vento.
			return 0;
		}

		if( true == pclSelectedTecBox->IsFctExcludedDegassing() )
		{
			// The current TecBox can't work with an external degassing device.
			return 0;
		}
	}

	// Prepare some variables to help.
	bool bAllAreBiggerThanMaxNumberOfVentoInParallel = true;
	int iLowerMaxNumberOfVentoInParallel = INT_MAX;
	bool bVentoAlone = ( NULL == pSelectedPM ) ? true : false;
	CSelectedPMBase::SelectedType eSelectedType = ( false == bVentoAlone ) ? pSelectedPM->GetSelectedType() :
			CSelectedPMBase::ST_Undefined;
	bool bSelectedTypeTrComp = ( CSelectedPMBase::ST_Compresso == eSelectedType
								 || CSelectedPMBase::ST_Transfero == eSelectedType ) ? true : false;

	int iFunction = CPMWQPrefsValidation::DWFCT_Degassing;
	bool bWaterMakeupWanted = ( 0 != m_pclInputUser->GetWaterMakeUpTypeID().Compare( _T("WMUP_TYPE_NONE") ) ) ? true : false;

	if( true == bWaterMakeupWanted )
	{
		if( 0 != m_pclInputUser->GetWaterMakeUpTypeID().Compare( _T("WMUP_TYPE_STD") ) )
		{
			iFunction |= CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;
		}
		else
		{
			iFunction |= CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal;
		}
	}


	CString strCombined = TASApp.LoadLocalizedString( IDS_RBN_PS_PMWQ_ONEDEVICE );

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CSelectedPMBase::RejectedReasons rRejectedReasons;
		vecRejectedReasons.clear();
		CDB_TBPlenoVento *pclTecBoxVento = dynamic_cast<CDB_TBPlenoVento *>( idptr.MP );

		if( NULL == pclTecBoxVento )
		{
			continue;
		}

		if( false == pclTecBoxVento->IsSelectable( true ) )
		{
			continue;
		}

		// Don't take degassing and water make-up that are combined with a Compresso or a Transfero. These ones are selected in
		// the '_SelectTecBox' method.
		if( true == pclTecBoxVento->IsFctCompressor() )
		{
			continue;
		}

		if( true == pclTecBoxVento->IsFctPump() )
		{
			continue;
		}

		// If TecBox has not the degassing property, we reject it.
		bool bTecBoxFctDegassing = pclTecBoxVento->IsFctDegassing();

		if( false == bTecBoxFctDegassing )
		{
			continue;
		}

		bool bTecBoxFctWaterMakeup = pclTecBoxVento->IsFctWaterMakeUp();
		bool bTecBoxVariantCooling = pclTecBoxVento->IsVariantCooling();
		bool bTecBoxVariantEcoAuto = pclTecBoxVento->IsVariantEcoAuto();
		bool bTecBoxVariantBreakTank = pclTecBoxVento->IsVariantBreakTank();
		bool bTecBoxVariantVacuumDeg = pclTecBoxVento->IsVariantVacuumDegassing();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// General preferences.

// 		// If user doesn't want water make-up and the current TecBox has this function, we reject it.
// 		if( false == fWaterMakeupWanted && true == fTecBoxFctWaterMakeup )
// 		{
// 			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMVTSE_XTP_XPM_NOP_HASWMKUPFCT );
// 			vecRejectedReasons.push_back( rRejectedReasons );
// 		}

		// If Vento is selected alone (without pressurisation) and if user wants water make-up in one combined TecBox but the current 
		// TecBox has not this function, we reject it.
		// HYS-1537 : Look the disable state
		if( true == bVentoAlone && true == bWaterMakeupWanted && true == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) 
			&& false == pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) && false == bTecBoxFctWaterMakeup )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_COMBINED_XPM_P_NOWMKUPFCT;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMVTSE_COMBINED_XPM_P_NOWMKUPFCT );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

// 		// If user wants degassing AND water make-up but not combined and the current TecBox is a VP we reject it.
// 		if( true == fWaterMakeupWanted && false == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice )
// 			&& true == fTecBoxFctDegassing && true == fTecBoxFctWaterMakeup )
// 		{
// 				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMVTSE_NOTP_XPM_P_VPCOMBINED );
// 				vecRejectedReasons.push_back( rRejectedReasons );
// 			}
// 		}

		if( ProjectType::Cooling == m_pclInputUser->GetApplicationType() )
		{
			// HYS-1006: We don't take Vento Connect without cooling version if we are in cooling AND if user has asked to get
			//          Vento for cooling version.
			// HYS-1537 : Look the disable state
			if( true == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion )
			   && false == pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion )
					&& false == bTecBoxVariantCooling )
			{
				// 
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_C_XTP_xPM_XVP_NOCOOLVARIANT;
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_C_XTP_xPM_XVP_NOCOOLVARIANT );
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// HYS-1006: We don't take Vento connect with cooling version if it is not needed.
			// HYS-1537 : Look the disable state
			if( ( false == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion )
				|| true == pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion ) )
				&& true == bTecBoxVariantCooling )
			{
				rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_C_XTP_XPM_XVP_VENTOCOOLVARIANTNOTNEEDED  );
				rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_C_XTP_XPM_XVP_VENTOCOOLVARIANTNOTNEEDED ;
				vecRejectedReasons.push_back( rRejectedReasons );
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Pump & Degassing preferences.

		// If user wants a Vento with cooling insulation with condensation water protection but the current Vento has not
		// this variant we reject it.
		// HYS-1537 : Look the disable state
		if( true == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion ) && false == bTecBoxVariantCooling 
			&& false == pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion ) )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_VCV_XPM_XP_NOCOOLVARIANT;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMVTSE_VCV_XPM_XP_NOCOOLVARIANT );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Water make-up preferences.

		// If user wants 'Break tank' and the current TecBox has not this variant, we reject it.
		// HYS-1537 : Look the disable state
		if( true == bWaterMakeupWanted && true == bTecBoxFctWaterMakeup	&& true == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::WaterMakeupBreakTank )
			&& false == pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::WaterMakeupBreakTank ) && false == bTecBoxVariantBreakTank )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_COMBINEDPBKTK_XPM_P_PNOBKTKVARIANT;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMVTSE_COMBINEDPBKTK_XPM_P_PNOBKTKVARIANT );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		// If user wants redundancy on Compressor/pump, we keep only VP.2 products.
		// HYS-1537 : Look the disable state
		if( true == bWaterMakeupWanted && true == bTecBoxFctWaterMakeup	&& true == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy )
			&& false == pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::WaterMakeupDutyStandBy ) && pclTecBoxVento->GetNbrPumpCompressor() < 2 )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_COMBINEDPDSTBY_XPM_P_NODSTBYVARIANT;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMVTSE_COMBINEDPDSTBY_XPM_P_NODSTBYVARIANT );
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Check that response pressure valve is not to high in regards to PS of the Vento.
		if( m_pclInputUser->GetSafetyValveResponsePressure() > pclTecBoxVento->GetPmaxmax() )
		{
			CString strPS = WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetPmaxmax(), true );
			CString strPSV = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetSafetyValveResponsePressure(), true );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_PSBELOWPSV, strPS, strPSV );
			rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_PSBELOWPSV;
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		// Check dimensions.
		if( m_pclInputUser->GetMaxWidth() > 0.0 && pclTecBoxVento->GetWidth() > m_pclInputUser->GetMaxWidth()
				&& pclTecBoxVento->GetDepth() > m_pclInputUser->GetMaxWidth() )
		{
			CString strWidth = WriteCUDouble( _U_LENGTH, pclTecBoxVento->GetWidth() );
			CString strDepth = WriteCUDouble( _U_LENGTH, pclTecBoxVento->GetDepth() );
			CString strWidthDepth = strWidth + _T("/") + strDepth;
			CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
			CString strUnit = pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str();
			strWidthDepth += _T(" ") + strUnit;
			CString strMaxWidth = WriteCUDouble( _U_LENGTH, m_pclInputUser->GetMaxWidth(), true );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_WITHTOOHIGH, strWidthDepth, strMaxWidth );
			rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_WITHTOOHIGH;
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		if( m_pclInputUser->GetMaxHeight() > 0.0 && pclTecBoxVento->GetHeight() > m_pclInputUser->GetMaxHeight() )
		{
			CString strHeight = WriteCUDouble( _U_LENGTH, pclTecBoxVento->GetHeight(), true );
			CString strMaxHeight = WriteCUDouble( _U_LENGTH, m_pclInputUser->GetMaxHeight(), true );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_HEIGHTTOHIGH, strHeight, strMaxHeight );
			rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_HEIGHTTOHIGH;
			vecRejectedReasons.push_back( rRejectedReasons );
		}

		// VAmin <= VS -> VAmin not actually in the DB.

		// Check if the installation volume VI is not too big.
		int iNbreOfVento = 1;

		if( pclTecBoxVento->GetSystemVolume() < m_pclInputUser->GetSystemVolume() )
		{
			iNbreOfVento = ( int )ceil( m_pclInputUser->GetSystemVolume() / pclTecBoxVento->GetSystemVolume() );
		}

		if( iNbreOfVento <= m_pTADS->GetpTechParams()->GetMaxNumberOfVentoInParallel() )
		{
			bAllAreBiggerThanMaxNumberOfVentoInParallel = false;
		}

		iLowerMaxNumberOfVentoInParallel = min( iNbreOfVento, iLowerMaxNumberOfVentoInParallel );

		// Check combination possibility.
		switch( eSelectedType )
		{
			case CSelectedPMBase::ST_Undefined:
				if( m_pclInputUser->GetDegassingPressureConnectPoint() < pclTecBoxVento->GetWorkingPressRangeMin() )
				{
					CString strPConnectPt = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetDegassingPressureConnectPoint(), true );
					CString strDPpMin = WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetWorkingPressRangeMin(), true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_PATCONPTBELOWDPPMIN, strPConnectPt, strDPpMin );
					rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_PATCONPTBELOWDPPMIN;
					vecRejectedReasons.push_back( rRejectedReasons );
				}

				if( m_pclInputUser->GetDegassingPressureConnectPoint() > pclTecBoxVento->GetWorkingPressRangeMax() )
				{
					CString strPConnectPt = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetDegassingPressureConnectPoint(), true );
					CString strDPpMax = WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetWorkingPressRangeMax(), true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_PATCONPTABOVEDPPMAX, strPConnectPt, strDPpMax );
					rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_PATCONPTABOVEDPPMAX;
					vecRejectedReasons.push_back( rRejectedReasons );
				}

				break;

			case CSelectedPMBase::ST_Vessel:
			{
				// Test for degassing.
				CSelectedVssl *pSelectedVessel = dynamic_cast<CSelectedVssl *>( pSelectedPM );
				ASSERT( NULL != pSelectedVessel );

				if( NULL == pSelectedVessel || NULL == dynamic_cast<CDB_Vessel*>( pSelectedVessel->GetProductIDPtr().MP ) )
				{
					continue;
				}

				if( pSelectedVessel->GetInitialPressure() < pclTecBoxVento->GetWorkingPressRangeMin() )
				{
					CString strPaopt = WriteCUDouble( _U_PRESSURE, pSelectedVessel->GetInitialPressure(), true );
					CString strDPpMin = WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetWorkingPressRangeMin(), true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_VVM_XP_PAOPTBELOWDPPMIN, strPaopt, strDPpMin );
					rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_VVM_XP_PAOPTBELOWDPPMIN;
					vecRejectedReasons.push_back( rRejectedReasons );
				}

				if( m_pclInputUser->GetFinalPressure() > pclTecBoxVento->GetWorkingPressRangeMax() )
				{
					CString strPe = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetFinalPressure(), true );
					CString strDPpMax = WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetWorkingPressRangeMax(), true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_VVM_XP_PEABOVEDPPMAX, strPe, strDPpMax );
					rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_VVM_XP_PEABOVEDPPMAX;
					vecRejectedReasons.push_back( rRejectedReasons );
				}

				// Test for the minimum vessel volume required for this Vento.
				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel*>( pSelectedVessel->GetProductIDPtr().MP );
				double dMinVesselVolume = pclTecBoxVento->GetMinimumVesselVolume( m_pclInputUser->GetFinalPressure() );

				if( -1.0 != dMinVesselVolume && pclVessel->GetNominalVolume() < dMinVesselVolume )
				{
					CString strVol = WriteCUDouble( _U_VOLUME, dMinVesselVolume, true );
					// This Vento requires a minimum vessel volume of %1.
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_VVM_XP_MINVESSELVOL, strVol);
					rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_VVM_XP_MINVESSELVOL;
					vecRejectedReasons.push_back( rRejectedReasons );
				}

				break;
			}

			case CSelectedPMBase::ST_Compresso:
			case CSelectedPMBase::ST_Transfero:
			{
				// Test for degassing.
				if( m_pclInputUser->GetMinimumInitialPressure() < pclTecBoxVento->GetWorkingPressRangeMin() )
				{
					CString strPa = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetMinimumInitialPressure(), true );
					CString strDPpMin = WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetWorkingPressRangeMin(), true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_CT_XP_PABELOWDPPMIN, strPa, strDPpMin );
					rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_CT_XP_PABELOWDPPMIN;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
				
				if( m_pclInputUser->GetFinalPressure( pclSelectedTecBox->GetTecBoxType() ) > pclTecBoxVento->GetWorkingPressRangeMax() )
				{
					CString strPe = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetFinalPressure( pclSelectedTecBox->GetTecBoxType() ), true );
					CString strDPpMax = WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetWorkingPressRangeMax(), true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_CT_XP_PEABOVEDPPMAX, strPe, strDPpMax );
					rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_CT_XP_PEABOVEDPPMAX;
					vecRejectedReasons.push_back( rRejectedReasons );
				}

				break;
			}
		}

		CSelectedVento *pSelectedVento = new CSelectedVento();

		if( NULL == pSelectedVento )
		{
			continue;
		}

		// Check the max temperature for the inlet of the degassing.
		if( true == bVentoAlone )
		{
			if( m_pclInputUser->GetDegassingMaxTempConnectPoint() > pclTecBoxVento->GetTmax() )
			{
				rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_TEMPTOHIGH;

				// Max. temperature at connection point of the vacuum degasser is above\r\nthe max. temperature of this device (%1 > %2).
				CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, m_pclInputUser->GetDegassingMaxTempConnectPoint(), true );
				CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclTecBoxVento->GetTmax(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_TEMPTOHIGH, strTemp1, strTemp2 );

				vecRejectedReasons.push_back( rRejectedReasons );
			}
			else if( m_pclInputUser->GetDegassingMaxTempConnectPoint() < pclTecBoxVento->GetTmin() )
			{
				CSelectedPMBase::RejectedReasons rRejectedReasons;
				rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_TEMPTOLOO;

				// The water make-up temperature is below the minimu admissible temperature for this product (%1 < %2).
				CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, m_pclInputUser->GetDegassingMaxTempConnectPoint(), true );
				CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclTecBoxVento->GetTmin(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_TEMPTOLOO, strTemp1, strTemp2 );

				vecRejectedReasons.push_back( rRejectedReasons );
			}

		}
		// Check temperature for water make-up if exist.
		if( true == bWaterMakeupWanted && true == bTecBoxFctWaterMakeup )
		{
			// Here we are with a VP!
			// Remark: for Vento we don't have the min. admissible temperature for the integrated water make-up part.
			if( m_pclInputUser->GetWaterMakeUpWaterTemp() > pclTecBoxVento->GetTmaxWaterMakeUp() )
			{
				pSelectedVento->SetFlag( CSelectedPMBase::ePlenoWMUpMaxTemp, true );
			}
		}

		// Check min temperature.
		// For the moment, we don't have min temperature when Pleno is selected without pressurization.
		if( CSelectedPMBase::ST_Undefined != eSelectedType )
		{
			if( ProjectType::Heating == m_pclInputUser->GetApplicationType()
				|| ProjectType::Solar == m_pclInputUser->GetApplicationType() )
			{
				if( m_pclInputUser->GetMinTemperature() < pclTecBoxVento->GetTmin() )
				{
					pSelectedVento->SetFlag( CSelectedPMBase::eVentoMinTemp, true );
				}
			}
			else
			{
				if( m_pclInputUser->GetReturnTemperature() < pclTecBoxVento->GetTmin() )
				{
					pSelectedVento->SetFlag( CSelectedPMBase::eVentoMinTemp, true );
				}
			}
		}

		pSelectedVento->SetNumberOfVento( iNbreOfVento );
		pSelectedVento->SetProductIDPtr( idptr );

		// Add Selected object to the list.
		if( 0 == (int)vecRejectedReasons.size() )
		{
			m_vSelectedPMlist.push_back( pSelectedVento );
		}
		else
		{
			pSelectedVento->SetRejected();
			pSelectedVento->AddRejectedReasons( vecRejectedReasons );
			m_vRejectedPMlist.push_back( pSelectedVento );
		}
	}

	if( 0 != ( int )m_vSelectedPMlist.size() )
	{
		_SortVentoAndInsertInVector( WhichList::WL_Selected, bAllAreBiggerThanMaxNumberOfVentoInParallel,
				iLowerMaxNumberOfVentoInParallel );
	}

	if( 0 != m_vRejectedPMlist.size() )
	{
		// Rejected product exist in the vector.
		_SortVentoAndInsertInVector( WhichList::WL_Rejected, bAllAreBiggerThanMaxNumberOfVentoInParallel,
				iLowerMaxNumberOfVentoInParallel );
	}

	return m_arvecSelectedProducts[PT_Vento].size();
}

bool CSelectPMList::_IsSelectedVP()
{
	if( false == IsProductExist( ProductType::PT_Vento ) )
	{
		return false;
	}

	CSelectedPMBase *pclSelectedVento = GetFirstProduct( ProductType::PT_Vento );

	if( NULL == pclSelectedVento )
	{
		return false;
	}

	CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedVento->GetProductIDPtr().MP );

	if( NULL == pclVento )
	{
		return false;
	}

	// If the first selected Vento is combined with a Pleno, we are sure that all the other ones are also combined.
	// Because if user chooses 'Combined in one device' and there is at least one 'VP', we reject all the 'V'. If there is no 'VP', we 
	// display thus only 'V'.
	// If user does not choose 'Combined in one device' and there is at least one 'V', we reject all the 'VP'. If there is no 'V' we 
	// display thus only 'VP'.
	return ( pclVento->GetNumberOfFunctions() > 1 ) ? true : false;
}

void CSelectPMList::_CheckHModuleForTransferoTV()
{
	// Special requirement for the new Transfero Connect.

	if( 0 == m_vSelectedPMlist.size() )
	{
		return;
	}

	// First step: run all Transfero to check if we have only H module.
	// If all are Transfero are with H module, it means that we need the H module for the "high flow" property.
	// In this case we need the H module even if temperatures are above 5°C and below 70°C.
	// If it's not the case we only need the H module for the "high temperature" property.
	// In this case we will effectively check with the return temperatures.

	bool bAtLeastOneWithoutHModule = false;

	for( auto &iter : m_vSelectedPMlist )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( iter->GetProductIDPtr().MP );

		if( NULL == pclTecBox )
		{
			ASSERT( 0 );
			continue;
		}

		if( 0 == CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TV_C") )
				&& false == pclTecBox->IsVariantHighFlowTemperature() )
		{
			bAtLeastOneWithoutHModule = true;
			break;
		}
	}

	if( false == bAtLeastOneWithoutHModule )
	{
		// All Transfero TV Connect selected are with the H module.
		return;
	}
	
	std::vector<CSelectedPMBase::RejectedReasons> vecRejectedReasons;
	vecSelectePMBase vecSelectedPMListCopy = m_vSelectedPMlist;
	m_vSelectedPMlist.clear();

	// Second step: we check return temperature.
	for( auto &iter : vecSelectedPMListCopy )
	{
		CSelectedPMBase::RejectedReasons rRejectedReasons;
		vecRejectedReasons.clear();

		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( iter->GetProductIDPtr().MP );

		if( NULL == pclTecBox )
		{
			ASSERT( 0 );
			m_vSelectedPMlist.push_back( iter );
			continue;
		}

		if( 0 != CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TV_C") ) )
		{
			m_vSelectedPMlist.push_back( iter );
			continue;
		}

		bool bTecBoxVariantHighFlowTemp = pclTecBox->IsVariantHighFlowTemperature();

		if( ProjectType::Heating == m_pclInputUser->GetApplicationType() 
				|| ProjectType::Solar == m_pclInputUser->GetApplicationType() )
		{
			// Vento limit is 90°C and the bag limit is 70°C.
			// Until 70°C no special need.
			// Between 70°C and 90°C it's ok for the Vento but not the bag of the vessel. We need to choose
			// Transfero with the H module (and intermediate vessel are mandatory).
			// Above 90°C the H module is mandatory and the Vento function MUST BE disabled.

			if( false == bTecBoxVariantHighFlowTemp )
			{
				if( m_pclInputUser->GetReturnTemperature() > 70.0 )
				{
					CString strTReturn = WriteCUDouble( _U_TEMPERATURE, m_pclInputUser->GetReturnTemperature(), true );
					CString strTMax = WriteCUDouble( _U_TEMPERATURE, pclTecBox->GetTmax(), true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HMODULENEEDED, strTReturn, strTMax );
					rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HMODULENEEDED;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
			}
			else
			{
				if( m_pclInputUser->GetReturnTemperature() <= 70.0 )
				{
					// This Transfero model contains a H module that is not needed here.
					rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HMODULENOTNEEDED );
					rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HMODULENOTNEEDED;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
			}
		}
		else
		{
			// We need to check also for the minimum temperature.
			// Vento limit is 0°C and the bag limit is 5°C.
			// If minimum temperature is below 5°C we need to choose a Transfero with a H module (and intermediate vessel 
			// are mandatory).
			// Below 0°C the H module is mandatory and the Vento function MUST BE disabled.

			if( false == bTecBoxVariantHighFlowTemp )
			{
				if( m_pclInputUser->GetMinTemperature() < 5.0 )
				{
					CString strTMinTemp = WriteCUDouble( _U_TEMPERATURE, m_pclInputUser->GetMinTemperature(), true );
					CString strTMin = WriteCUDouble( _U_TEMPERATURE, 5.0, true );
					FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HMODULENEEDED2, strTMinTemp, strTMin );
					rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HMODULENEEDED2;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
			}
			else
			{
				if( m_pclInputUser->GetMinTemperature() >= 5.0 )
				{
					// This Transfero model contains a H module that is not needed here.
					rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HMODULENOTNEEDED );
					rRejectedReasons.m_iIDError = IDS_SELECTPMTBSE_XAT_XTP_XPM_XVP_HMODULENOTNEEDED;
					vecRejectedReasons.push_back( rRejectedReasons );
				}
			}
		}

		if( 0 == (int)vecRejectedReasons.size() )
		{
			m_vSelectedPMlist.push_back( iter );
		}
		else
		{
			iter->SetRejected();
			iter->AddRejectedReasons( vecRejectedReasons );
			m_vRejectedPMlist.push_back( iter );
		}
	}
}

int CSelectPMList::_SelectPleno( CSelectedPMBase *pSelectedPM )
{
	if( NULL == m_pclInputUser || NULL == m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	_CleanVecKeyPMBase( ProductType::PT_Pleno );
	m_vSelectedPMlist.clear();
	m_vRejectedPMlist.clear();

	std::vector<CSelectedPMBase::RejectedReasons> vecRejectedReasons;

	// If user doesn't want a water make-up device, we exit.
	if( 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_NONE") ) )
	{
		return 0;
	}

	// If we can't access the TecBox table, we exit.
	CTable *pclTecBoxTable = (CTable *)( m_pTADB->Get( _T("TECBOX_TAB") ).MP );
	ASSERT( NULL != pclTecBoxTable );

	if( NULL == pclTecBoxTable )
	{
		return 0;
	}

	// If there is a TecBox selected but the product IDPTR is NULL, we exit.
	if( NULL != pSelectedPM && _NULL_IDPTR == pSelectedPM->GetProductIDPtr() )
	{
		return 0;
	}

	CPMWQPrefs *pclPMWQSelectionPreferences = m_pclInputUser->GetPMWQSelectionPreferences();
	ASSERT( NULL != pclPMWQSelectionPreferences );

	if( NULL == pclPMWQSelectionPreferences )
	{
		return 0;
	}

	// Prepare some variables to help.
	bool bPlenoAlone = ( NULL == pSelectedPM ) ? true : false;
	CSelectedPMBase::SelectedType eSelectedType = ( false == bPlenoAlone ) ? pSelectedPM->GetSelectedType() :
			CSelectedPMBase::ST_Undefined;

	// 2015-08-10: New with Connect products: in 'CDB_TecBox' there is a new parameter 'PlenoID' that identifies a table
	//             of Pleno if defined.
	// 2015-09-07: Also in the 'CDB_Vessel'.
	CTable *pclPlenoTable = NULL;
	bool bPlenoTableFromGroupID = false;
	CTable *pclWaterTreatmentCombinationTable = NULL;
	CDB_TecBox *pclSelectedTecBox = NULL;
	CDB_Vessel *pclSelectedVessel = NULL;

	if( false == bPlenoAlone )
	{
		pclSelectedTecBox = dynamic_cast<CDB_TecBox *>( pSelectedPM->GetProductIDPtr().MP );

		if( NULL != pclSelectedTecBox )
		{
			pclPlenoTable = dynamic_cast<CTable*>( pclSelectedTecBox->GetPlenoIDPtr().MP );
			pclWaterTreatmentCombinationTable = dynamic_cast<CTable*>( pclSelectedTecBox->GetWaterTreatmentCombIDPtr().MP );
			bPlenoTableFromGroupID = true;
		}
		else
		{
			pclSelectedVessel = dynamic_cast<CDB_Vessel*>( pSelectedPM->GetProductIDPtr().MP );

			if( NULL != pclSelectedVessel )
			{
				pclPlenoTable = dynamic_cast<CTable*>( pclSelectedVessel->GetPlenoIDPtr().MP );
				bPlenoTableFromGroupID = true;
			}
		}
	}

	if( NULL == pclPlenoTable )
	{
		pclPlenoTable = pclTecBoxTable;
	}

	if( true == bPlenoAlone )
	{
		if( BST_CHECKED == m_pclInputUser->GetDegassingChecked() )
		{
			// We need to check if in the selected Vento there is only VP or V.
			if( true == _IsSelectedVP() )
			{
				return 0;
			}
		}
	}
	else if( NULL != pclSelectedTecBox )
	{
		// Case of Pleno selected with a TecBox (Compresso or Transfero).

		if( true == pclSelectedTecBox->IsFctWaterMakeUp() )
		{
			// If selected TecBox has water make-up device integrated, we don't select Pleno.
			return 0;
		}

		if( true == pclSelectedTecBox->IsFctExcludedWaterMakeUp() )
		{
			// The current TecBox can't work with an eternal water make-up device.
			return 0;
		}

		if( BST_CHECKED == m_pclInputUser->GetDegassingChecked() )
		{
			// We need to check if in the selected Vento there is only VP or V.
			if( true == _IsSelectedVP() )
			{
				return 0;
			}
		}
	}

	// Simply take the list from the 'CTable'.
	bool bWorkForStandard = ( 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_STD") ) ) ? true : false;

	// Special treatment if user asks a water treatment with softening or desalination and there is a table for the different water treatment 
	// combinations.
	if( false == bWorkForStandard && NULL != pclWaterTreatmentCombinationTable )
	{
		CTableSet *pclPlenoRefillCombTable = dynamic_cast<CTableSet *>( m_pTADB->Get( _T("PLENOREFILLCOMB_TAB") ).MP );

		if( NULL == pclPlenoRefillCombTable )
		{
			ASSERTA_RETURN( 0 );
		}

		for( IDPTR PRCombIDPtr = pclWaterTreatmentCombinationTable->GetFirst(); _T('\0') != *PRCombIDPtr.ID; PRCombIDPtr = pclWaterTreatmentCombinationTable->GetNext() )
		{
			CDB_Set *pclPRCombination = dynamic_cast<CDB_Set*>( pclPlenoRefillCombTable->Get( PRCombIDPtr.ID ).MP );

			if( NULL == pclPRCombination )
			{
				ASSERT_CONTINUE;
			}

			// HYS-1419.
			// If one product of the combination is not available (Filtered out for a country), we by-pass it.
			if( pclPRCombination->GetFirstIDPtr().ID != _T('\0') && 0 != _tcsicmp( _T("NOTHING_ID"), pclPRCombination->GetFirstIDPtr().ID ) )
			{
				if( false == (CData*)( pclPRCombination->GetFirstIDPtr().MP )->IsAvailable() )
				{
					continue;
				}
			}

			if( pclPRCombination->GetSecondIDPtr().ID != _T('\0') && 0 != _tcsicmp( _T("NOTHING_ID"), pclPRCombination->GetSecondIDPtr().ID ) )
			{
				if( false == (CData*)( pclPRCombination->GetSecondIDPtr().MP )->IsAvailable() )
				{
					continue;
				}
			}

			CSelectedWaterTreatmentCombination *pWTCombination = new CSelectedWaterTreatmentCombination();

			if( NULL == pWTCombination )
			{
				continue;
			}

			bool bAtLeastOneRejected = false;

			for( int iLoopProduct = 0; iLoopProduct < 2; ++iLoopProduct )
			{
				vecRejectedReasons.clear();
				IDPTR ProductIDPtr = ( 0 == iLoopProduct ) ? pclPRCombination->GetFirstIDPtr() : pclPRCombination->GetSecondIDPtr();

				if( _T('\0') == *ProductIDPtr.ID || 0 == _tcsicmp( _T("NOTHING_ID"), ProductIDPtr.ID ) )
				{
					continue;
				}
				
				// Pay attention here we use 'pclTecBoxTable' to have all Pleno.
				CDB_TBPlenoVento *pTecBoxPleno = dynamic_cast<CDB_TBPlenoVento*>( pclTecBoxTable->Get( pclPRCombination->GetFirstIDPtr().ID ).MP ); 

				if( NULL == pTecBoxPleno )
				{
					continue;
				}
				
				if( false == _SelectPlenoHelper( pTecBoxPleno, pSelectedPM, vecRejectedReasons ) )
				{
					continue;
				}

				CSelectedPMBase *pSelectedPleno = ( 0 == iLoopProduct ) ? pWTCombination->GetSelectedFirst() : pWTCombination->GetSelectedSecond();

				if( NULL == pSelectedPleno )
				{
					continue;
				}

				pSelectedPleno->SetProductIDPtr( ProductIDPtr );

				// Add Selected object to the list.
				if( 0 != (int)vecRejectedReasons.size() )
				{
					pSelectedPleno->SetRejected();
					pSelectedPleno->AddRejectedReasons( vecRejectedReasons );
					bAtLeastOneRejected = true;
				}
			}

			pWTCombination->SetWTCombination( pclPRCombination );

			vecSelectePMBase *pvecWhereToInsert = NULL;

			// Add Selected object to the list.
			if( false == bAtLeastOneRejected )
			{
				pvecWhereToInsert = &m_arvecSelectedProducts[ProductType::PT_Pleno];
			}
			else
			{
				pWTCombination->SetRejected();
				pWTCombination->ClearAllFlags();
				pvecWhereToInsert = &m_arvecRejectedProducts[ProductType::PT_Pleno];
			}

			pvecWhereToInsert->push_back( pWTCombination );
		}
	}
	else
	{
		for( IDPTR PlenoIDPtr = pclPlenoTable->GetFirst(); '\0' != *PlenoIDPtr.ID; PlenoIDPtr = pclPlenoTable->GetNext() )
		{
			vecRejectedReasons.clear();

			CDB_TBPlenoVento *pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento*>( PlenoIDPtr.MP );

			if( NULL == pclTecBoxPleno )
			{
				continue;
			}

			// It can be here Pleno or Vento. We reject Vento.
			if( true == pclTecBoxPleno->IsFctDegassing() )
			{
				continue;
			}

			if( false == _SelectPlenoHelper( pclTecBoxPleno, pSelectedPM, vecRejectedReasons ) )
			{
				continue;
			}

			CSelectedPMBase *pSelectedPleno = new CSelectedPMBase( CSelectedPMBase::ST_Pleno );

			if( NULL == pSelectedPleno )
			{
				continue;
			
			}

			if( false == bPlenoAlone )
			{
				if( m_pclInputUser->GetWaterMakeUpWaterTemp() < pclTecBoxPleno->GetTmin() )
				{
					pSelectedPleno->SetFlag( CSelectedPMBase::ePlenoWMUpMinTemp, true );
				}
				else if( m_pclInputUser->GetWaterMakeUpWaterTemp() > pclTecBoxPleno->GetTmaxWaterMakeUp() )
				{
					pSelectedPleno->SetFlag( CSelectedPMBase::ePlenoWMUpMaxTemp, true );
				}
			}

			pSelectedPleno->SetProductIDPtr( PlenoIDPtr );

			// Add Selected object to the list.
			if( 0 == (int)vecRejectedReasons.size() )
			{
				m_vSelectedPMlist.push_back( pSelectedPleno );
			}
			else
			{
				pSelectedPleno->SetRejected();
				pSelectedPleno->AddRejectedReasons( vecRejectedReasons, false );
				m_vRejectedPMlist.push_back( pSelectedPleno );
			}
		}

		if( 0 != ( int )m_vSelectedPMlist.size() )
		{
			_SortPlenoAndInsertInVector( WhichList::WL_Selected );
		}

		if( 0 != m_vRejectedPMlist.size() )
		{
			// Rejected product exist in the vector.
			_SortPlenoAndInsertInVector( WhichList::WL_Rejected );
		}
	}

	return m_arvecSelectedProducts[PT_Pleno].size();
}


int CSelectPMList::SelectPlenoProtec( CSelectedPMBase *pSelectedPM )
{
	if( NULL == m_pclInputUser || NULL == m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}
	bool IsPlenoSelected = false;
	if( NULL != pSelectedPM && CSelectedPMBase::SelectedType::ST_Pleno == pSelectedPM->GetSelectedType() )
	{
		IsPlenoSelected = true;
	}

	_CleanVecKeyPMBase( ProductType::PT_Pleno_Protec );

	m_vSelectedPMlist.clear();
	m_vRejectedPMlist.clear();

	std::vector<CSelectedPMBase::RejectedReasons> vecRejectedReasons;

	// If user doesn't want a water make-up device, we exit.
	if( 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_NONE") ) )
	{
		return 0;
	}

	// If we can't access the TecBox table, we exit.
	CTable *pclTecBoxTable = (CTable *)( m_pTADB->Get( _T("TECBOX_TAB") ).MP );
	ASSERT( NULL != pclTecBoxTable );

	if( NULL == pclTecBoxTable )
	{
		return 0;
	}

	// If there is a TecBox selected but the product IDPTR is NULL, we exit.
	if( NULL != pSelectedPM && _NULL_IDPTR == pSelectedPM->GetProductIDPtr() )
	{
		return 0;
	}

	CPMWQPrefs *pclPMWQSelectionPreferences = m_pclInputUser->GetPMWQSelectionPreferences();
	ASSERT( NULL != pclPMWQSelectionPreferences );

	if( NULL == pclPMWQSelectionPreferences )
	{
		return 0;
	}

	// Prepare some variables to help.
	bool bPlenoAlone = ( NULL == pSelectedPM ) ? true : false;
	CSelectedPMBase::SelectedType eSelectedType = ( false == bPlenoAlone ) ? pSelectedPM->GetSelectedType() :
			CSelectedPMBase::ST_Undefined;

	// 2015-08-10: New with Connect products: in 'CDB_TecBox' there is a new parameter 'PlenoID' that identifies a table
	//             of Pleno if defined.
	// 2015-09-07: Also in the 'CDB_Vessel'.
	CTable *pclPlenoTable = NULL;
	bool bPlenoTableFromGroupID = false;
	CTable *pclWaterTreatmentCombinationTable = NULL;
	CDB_TecBox *pclSelectedTecBox = NULL;
	CDB_Vessel *pclSelectedVessel = NULL;
	
	if( false == bPlenoAlone )
	{
		pclSelectedTecBox = dynamic_cast<CDB_TecBox *>( pSelectedPM->GetProductIDPtr().MP );

		if( NULL != pclSelectedTecBox )
		{
			pclPlenoTable = dynamic_cast<CTable*>( pclSelectedTecBox->GetPlenoIDPtr().MP );
			pclWaterTreatmentCombinationTable = dynamic_cast<CTable*>( pclSelectedTecBox->GetWaterTreatmentCombIDPtr().MP );
			bPlenoTableFromGroupID = true;
		}
		else
		{
			pclSelectedVessel = dynamic_cast<CDB_Vessel*>( pSelectedPM->GetProductIDPtr().MP );

			if( NULL != pclSelectedVessel )
			{
				pclPlenoTable = dynamic_cast<CTable*>( pclSelectedVessel->GetPlenoIDPtr().MP );
				bPlenoTableFromGroupID = true;
			}
		}
	}

	if( NULL == pclPlenoTable )
	{
		pclPlenoTable = pclTecBoxTable;
	}

	if( true == bPlenoAlone )
	{
		if( BST_CHECKED == m_pclInputUser->GetDegassingChecked() )
		{
			// We need to check if in the selected Vento there is only VP or V.
			if( true == _IsSelectedVP() )
			{
				return 0;
			}
		}
	}
	else if( NULL != pclSelectedTecBox )
	{
		// Case of Pleno selected with a TecBox (Compresso or Transfero).

		if( true == pclSelectedTecBox->IsFctWaterMakeUp() && false == IsPlenoSelected )
		{
			// If selected TecBox has water make-up device integrated, we don't select Pleno.
			return 0;
		}

		if( true == pclSelectedTecBox->IsFctExcludedWaterMakeUp() )
		{
			// The current TecBox can't work with an eternal water make-up device.
			return 0;
		}

		if( BST_CHECKED == m_pclInputUser->GetDegassingChecked() )
		{
			// We need to check if in the selected Vento there is only VP or V.
			if( true == _IsSelectedVP() )
			{
				return 0;
			}
		}
	}

	// Simply take the list from the 'CTable'.
	bool bWorkForStandard = ( 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_STD") ) ) ? true : false;

	// Special treatment if user asks a water treatment with softening or desalination and there is a table for the different water treatment 
	// combinations.
	if( false == bWorkForStandard && NULL != pclWaterTreatmentCombinationTable )
	{
		CTableSet *pclPlenoRefillCombTable = dynamic_cast<CTableSet *>( m_pTADB->Get( _T("PLENOREFILLCOMB_TAB") ).MP );

		if( NULL == pclPlenoRefillCombTable )
		{
			ASSERTA_RETURN( 0 );
		}

		for( IDPTR PRCombIDPtr = pclWaterTreatmentCombinationTable->GetFirst(); _T('\0') != *PRCombIDPtr.ID; PRCombIDPtr = pclWaterTreatmentCombinationTable->GetNext() )
		{
			CDB_Set *pclPRCombination = dynamic_cast<CDB_Set*>( pclPlenoRefillCombTable->Get( PRCombIDPtr.ID ).MP );

			if( NULL == pclPRCombination )
			{
				ASSERT_CONTINUE;
			}

			CSelectedWaterTreatmentCombination *pWTCombination = new CSelectedWaterTreatmentCombination();

			if( NULL == pWTCombination )
			{
				continue;
			}

			bool bAtLeastOneRejected = false;

			for( int iLoopProduct = 0; iLoopProduct < 2; ++iLoopProduct )
			{
				vecRejectedReasons.clear();
				IDPTR ProductIDPtr = ( 0 == iLoopProduct ) ? pclPRCombination->GetFirstIDPtr() : pclPRCombination->GetSecondIDPtr();

				if( _T('\0') == *ProductIDPtr.ID || 0 == _tcsicmp( _T("NOTHING_ID"), ProductIDPtr.ID ) )
				{
					continue;
				}
				
				// Pay attention here we use 'pclTecBoxTable' to have all Pleno.
				CDB_TBPlenoVento *pTecBoxPleno = dynamic_cast<CDB_TBPlenoVento*>( pclTecBoxTable->Get( pclPRCombination->GetFirstIDPtr().ID ).MP ); 

				if( NULL == pTecBoxPleno )
				{
					continue;
				}
				
				if( false == _SelectPlenoHelper( pTecBoxPleno, pSelectedPM, vecRejectedReasons ) )
				{
					continue;
				}

				CSelectedPMBase *pSelectedPleno = ( 0 == iLoopProduct ) ? pWTCombination->GetSelectedFirst() : pWTCombination->GetSelectedSecond();

				if( NULL == pSelectedPleno )
				{
					continue;
				}

				pSelectedPleno->SetProductIDPtr( ProductIDPtr );

				// Check temperature.
				if( m_pclInputUser->GetWaterMakeUpWaterTemp() < pTecBoxPleno->GetTmin() )
				{
					pSelectedPleno->SetFlag( CSelectedPMBase::ePlenoWMUpMinTemp, true );
				}
				else if( m_pclInputUser->GetWaterMakeUpWaterTemp() > pTecBoxPleno->GetTmaxWaterMakeUp() )
				{
					pSelectedPleno->SetFlag( CSelectedPMBase::ePlenoWMUpMaxTemp, true );
				}

				// Add Selected object to the list.
				if( 0 != (int)vecRejectedReasons.size() )
				{
					pSelectedPleno->SetRejected();
					pSelectedPleno->AddRejectedReasons( vecRejectedReasons );
					bAtLeastOneRejected = true;
				}
			}

			pWTCombination->SetWTCombination( pclPRCombination );

			vecSelectePMBase *pvecWhereToInsert = NULL;

			// Add Selected object to the list.
			if( false == bAtLeastOneRejected )
			{
				pvecWhereToInsert = &m_arvecSelectedProducts[ProductType::PT_Pleno_Protec];
			}
			else
			{
				pWTCombination->SetRejected();
				pWTCombination->ClearAllFlags();
				pvecWhereToInsert = &m_arvecRejectedProducts[ProductType::PT_Pleno_Protec];
			}

			pvecWhereToInsert->push_back( pWTCombination );
		}
	}
	else
	{
		for( IDPTR PlenoIDPtr = pclPlenoTable->GetFirst(); '\0' != *PlenoIDPtr.ID; PlenoIDPtr = pclPlenoTable->GetNext() )
		{
			vecRejectedReasons.clear();

			CDB_TBPlenoVento *pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento*>( PlenoIDPtr.MP );

			if( NULL == pclTecBoxPleno )
			{
				continue;
			}

			// It can be here Pleno or Vento. We reject Vento.
			if( true == pclTecBoxPleno->IsFctDegassing() )
			{
				continue;
			}

			if( false == _SelectPlenoHelper( pclTecBoxPleno, pSelectedPM, vecRejectedReasons ) )
			{
				continue;
			}

			CSelectedPMBase *pSelectedPleno = new CSelectedPMBase( CSelectedPMBase::ST_Pleno );

			if( NULL == pSelectedPleno )
			{
				continue;
			}

			pSelectedPleno->SetProductIDPtr( PlenoIDPtr );

			// Check temperature.
			if( m_pclInputUser->GetWaterMakeUpWaterTemp() < pclTecBoxPleno->GetTmin() )
			{
				pSelectedPleno->SetFlag( CSelectedPMBase::ePlenoWMUpMinTemp, true );
			}
			else if( m_pclInputUser->GetWaterMakeUpWaterTemp() > pclTecBoxPleno->GetTmaxWaterMakeUp() )
			{
				pSelectedPleno->SetFlag( CSelectedPMBase::ePlenoWMUpMaxTemp, true );
			}

			// Add Selected object to the list.
			if( 0 == (int)vecRejectedReasons.size() )
			{
				m_vSelectedPMlist.push_back( pSelectedPleno );
			}
			else
			{
				pSelectedPleno->SetRejected();
				pSelectedPleno->AddRejectedReasons( vecRejectedReasons, false );
				m_vRejectedPMlist.push_back( pSelectedPleno );
			}
		}

		if( 0 != ( int )m_vSelectedPMlist.size() )
		{
			_SortPlenoAndInsertInVector( WhichList::WL_Selected, PT_Pleno_Protec );
		}

		if( 0 != m_vRejectedPMlist.size() )
		{
			// Rejected product exist in the vector.
			_SortPlenoAndInsertInVector( WhichList::WL_Rejected, PT_Pleno_Protec );
		}
	}

	return m_arvecSelectedProducts[PT_Pleno_Protec].size();
}

bool CSelectPMList::_SelectPlenoHelper( CDB_TBPlenoVento *pclTecBoxPleno, CSelectedPMBase *pSelectedPM, std::vector<CSelectedPMBase::RejectedReasons> &vecRejectedReasons )
{
	if( NULL == pclTecBoxPleno )
	{
		return false;
	}

	if( false == pclTecBoxPleno->IsSelectable( true ) )
	{
		return false;
	}

	CSelectedPMBase::RejectedReasons rRejectedReasons;
	bool bPlenoAlone = ( NULL == pSelectedPM ) ? true : false;

	CSelectedPMBase::SelectedType eSelectedType = ( false == bPlenoAlone ) ? pSelectedPM->GetSelectedType() : CSelectedPMBase::ST_Undefined;
	CPMWQPrefs *pclPMWQSelectionPreferences = m_pclInputUser->GetPMWQSelectionPreferences();

	bool bTecBoxVariantBreakTank = pclTecBoxPleno->IsVariantBreakTank();

	bool bWorkForStandard = ( 0 == CString( m_pclInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_STD") ) ) ? true : false;

	// HYS-1121: Pleno can have an optionnal breaktank
	CTable *pclPlenoTable = dynamic_cast<CTable*>( pclTecBoxPleno->GetPlenoIDPtr().MP );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Water make-up preferences.

	if( true == bWorkForStandard && CDB_TBPlenoVento::ePMPR_RefillMandatory == pclTecBoxPleno->GetRefillable() )
	{
		// If current Pleno can work only with water treatment module, we can't select it if user asks the water make-up standard (without treatment).
		rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_WMUPSTD_XPM_XPT_REFILLMANDATORY;
		rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMPSE_WMUPSTD_XPM_XPT_REFILLMANDATORY );
		vecRejectedReasons.push_back( rRejectedReasons );
	}

	if( false == bWorkForStandard && CDB_TBPlenoVento::ePMPR_RefillNone == pclTecBoxPleno->GetRefillable() )
	{
		// HYS-1121: Pleno can have an optionnal Pleno Refill
		bool bRefillOptional = false;
		if( NULL != pclPlenoTable )
		{
			for( IDPTR PlenoIDPtr = pclPlenoTable->GetFirst(); '\0' != *PlenoIDPtr.ID; PlenoIDPtr = pclPlenoTable->GetNext() )
			{
				CDB_TBPlenoVento *pclTecBoxOptional = dynamic_cast<CDB_TBPlenoVento*>( PlenoIDPtr.MP );

				if( NULL == pclTecBoxOptional )
				{
					continue;
				}
				if( CDB_TBPlenoVento::ePMPR_RefillNone != pclTecBoxOptional->GetRefillable() )
				{
					bRefillOptional = true;
					break;
				}
			}
		}
		if( false == bRefillOptional )
		{
			// If current Pleno can't work will water treatment module, we can't select if if user asks a water treatment (softening or desalination).
			rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_WMUPSOFTDESAL_XPM_XPT_REFILLNONE;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMPSE_WMUPSOFTDESAL_XPM_XPT_REFILLNONE );
			vecRejectedReasons.push_back( rRejectedReasons );
		}
	}

	// If user wants 'Break tank' and the current TecBox has not this variant, we reject it.
	// HYS-1537 : Look the disable state
	if( true == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::WaterMakeupBreakTank ) && false == bTecBoxVariantBreakTank 
		&& false == pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::WaterMakeupBreakTank ) )
	{
		// HYS-1121: Pleno can have an optionnal breaktank
		bool bBreakTankOptional = false;
		if( NULL != pclPlenoTable )
		{
			for( IDPTR PlenoIDPtr = pclPlenoTable->GetFirst(); '\0' != *PlenoIDPtr.ID; PlenoIDPtr = pclPlenoTable->GetNext() )
			{
				CDB_TBPlenoVento *pclTecBoxOptional = dynamic_cast<CDB_TBPlenoVento*>( PlenoIDPtr.MP );

				if( NULL == pclTecBoxOptional )
				{
					continue;
				}
				if( true == pclTecBoxOptional->IsVariantBreakTank() )
				{
					bBreakTankOptional = true;
					break;
				}
			}
		}
		if( false == bBreakTankOptional )
		{
			rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_PBKTK_XPM_XPT_NOBKTKVARIANT;
			rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMPSE_PBKTK_XPM_XPT_NOBKTKVARIANT );
			vecRejectedReasons.push_back( rRejectedReasons );
		}
	}

	// If user wants redundancy on Compressor/pump, we keep only PI.2 products.
	// HYS-1537 : Look the disable state
	if( true == pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy ) && pclTecBoxPleno->GetNbrPumpCompressor() < 2 
		&& false == pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::WaterMakeupDutyStandBy ) )
	{
		rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_PDTYSTBY_XPM_XPT_NODTYSTBYVARIANT;
		rRejectedReasons.m_strErrorMsg = TASApp.LoadLocalizedString( IDS_SELECTPMPSE_PDTYSTBY_XPM_XPT_NODTYSTBYVARIANT );
		vecRejectedReasons.push_back( rRejectedReasons );
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Check physical data.
	if( m_pclInputUser->GetMaxWidth() > 0.0 && pclTecBoxPleno->GetWidth() > m_pclInputUser->GetMaxWidth()
			&& pclTecBoxPleno->GetDepth() > m_pclInputUser->GetMaxWidth() )
	{
		CString strWidth = WriteCUDouble( _U_LENGTH, pclTecBoxPleno->GetWidth() );
		CString strDepth = WriteCUDouble( _U_LENGTH, pclTecBoxPleno->GetDepth() );
		CString strWidthDepth = strWidth + _T("/") + strDepth;
		CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
		CString strUnit = pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str();
		strWidthDepth += _T(" ") + strUnit;
		CString strMaxWidth = WriteCUDouble( _U_LENGTH, m_pclInputUser->GetMaxWidth(), true );
		FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_XPM_XPT_WIDTHTOOHIGH, strWidthDepth, strMaxWidth );
		rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_XPM_XPT_WIDTHTOOHIGH;
		vecRejectedReasons.push_back( rRejectedReasons );
	}

	if( m_pclInputUser->GetMaxHeight() > 0.0 && pclTecBoxPleno->GetHeight() > m_pclInputUser->GetMaxHeight() )
	{
		CString strHeight = WriteCUDouble( _U_LENGTH, pclTecBoxPleno->GetHeight(), true );
		CString strMaxHeight = WriteCUDouble( _U_LENGTH, m_pclInputUser->GetMaxHeight(), true );
		FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_XPM_XPT_HEIGHTTOOHIGH, strHeight, strMaxHeight );
		rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_XPM_XPT_HEIGHTTOOHIGH;
		vecRejectedReasons.push_back( rRejectedReasons );
	}

	// VAmin <= VS -> VAmin not actually in the DB.

	// Check combination possibility.
	switch( eSelectedType )
	{
		case CSelectedPMBase::ST_Undefined:
			if( m_pclInputUser->GetWaterMakeUpNetworkPN() > pclTecBoxPleno->GetPmaxmax() )
			{
				CString strPSN = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetWaterMakeUpNetworkPN(), true );
				CString strPS = WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetPmaxmax(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPFSE_PNABOVEPMAX, strPSN, strPS );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPFSE_PNABOVEPMAX;
				vecRejectedReasons.push_back( rRejectedReasons );
			}
			break;

		case CSelectedPMBase::ST_Vessel:
		{
			// Check that response pressure valve is not to high in regards to PS only for Pleno without pump.
			if( 0 == pclTecBoxPleno->GetNbrPumpCompressor() && pclTecBoxPleno->GetPmaxmax() < m_pclInputUser->GetSafetyValveResponsePressure() )
			{
				CString strPS = WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetPmaxmax(), true );
				CString strPSV = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetSafetyValveResponsePressure(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_VVM_PI_PSBELOWPSV, strPS, strPSV );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_VVM_PI_PSBELOWPSV;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			CSelectedVssl *pSelectedVessel = dynamic_cast<CSelectedVssl *>( pSelectedPM );
			ASSERT( NULL != pSelectedVessel );

			if( NULL == pSelectedVessel )
			{
				return false;
			}

			// Check the pressure water network only for Pleno without pump.
			if( 0 == pclTecBoxPleno->GetNbrPumpCompressor() 
				&& ( m_pclInputUser->GetWaterMakeUpNetworkPN() < m_pclInputUser->GetMinimumPressure() + 1.7e5 ) )
			{
				CString strPSN = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetWaterMakeUpNetworkPN(), true );
				CString strPo1_7 = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetMinimumPressure() + 1.7e5, true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_VVM_PI_PNSTOOLOW, strPSN, strPo1_7 );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_VVM_PI_PNSTOOLOW;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			if( 0 == pclTecBoxPleno->GetNbrPumpCompressor() && ( m_pclInputUser->GetWaterMakeUpNetworkPN() > 1e6 ) )
			{
				CString strPSN = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetWaterMakeUpNetworkPN(), true );
				CString str10bar = WriteCUDouble( _U_PRESSURE, 1e6, true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_VVM_PI_PNSTOOHIGH, strPSN, str10bar );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_VVM_PI_PNSTOOHIGH;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// Check that Pa is well in the DPp range of the Pleno with pump.
			if( pclTecBoxPleno->GetNbrPumpCompressor() >= 1 && pSelectedVessel->GetInitialPressure() < pclTecBoxPleno->GetWorkingPressRangeMin() )
			{
				CString strPaopt = WriteCUDouble( _U_PRESSURE, pSelectedVessel->GetInitialPressure(), true );
				CString strDPpMin = WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetWorkingPressRangeMin(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_VVM_PI96_PABELOWDPPMIN, strPaopt, strDPpMin );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_VVM_PI96_PABELOWDPPMIN;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			if( pclTecBoxPleno->GetNbrPumpCompressor() >= 1 && pSelectedVessel->GetInitialPressure() > pclTecBoxPleno->GetWorkingPressRangeMax() )
			{
				CString strPaopt = WriteCUDouble( _U_PRESSURE, pSelectedVessel->GetInitialPressure(), true );
				CString strDPpMax = WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetWorkingPressRangeMax(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_VVM_PI96_PAABOVEDPPMAX, strPaopt, strDPpMax );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_VVM_PI96_PAABOVEDPPMAX;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			break;
		}

		case CSelectedPMBase::ST_Compresso:
		case CSelectedPMBase::ST_Transfero:

			// Check the pressure water network only for Pleno without pump connected on a system with a Compresso.
			// Remark: In this case the Pleno is directly connected to the system.
			if( CSelectedPMBase::ST_Compresso == eSelectedType
					&& 0 == pclTecBoxPleno->GetNbrPumpCompressor()
					&& m_pclInputUser->GetWaterMakeUpNetworkPN() < m_pclInputUser->GetMinimumPressure() + 1.9e5 )
			{
				CString strPSN = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetWaterMakeUpNetworkPN(), true );
				CString strPo1_9 = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetMinimumPressure() + 1.9e5, true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_NOTP_C_P_PNSTOOLOW, strPSN, strPo1_9 );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_NOTP_C_P_PNSTOOLOW;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// For a Transfero the minimum value is set to 2 bar.
			// Remark: In this case the Pleno is connected to the TecBox.
			if( CSelectedPMBase::ST_Transfero == eSelectedType
					&& 0 == pclTecBoxPleno->GetNbrPumpCompressor()
					&& m_pclInputUser->GetWaterMakeUpNetworkPN() < 2e5 )
			{
				CString strPSN = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetWaterMakeUpNetworkPN(), true );
				CString strPo1_9 = WriteCUDouble( _U_PRESSURE, 2e5, true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_NOTP_T_P_PNSTOOLOW, strPSN, strPo1_9 );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_NOTP_T_P_PNSTOOLOW;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// For Compresso and Transfero the upper limit is 10bar.
			if( 0 == pclTecBoxPleno->GetNbrPumpCompressor() && m_pclInputUser->GetWaterMakeUpNetworkPN() > 1e6 )
			{
				CString strPSN = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetWaterMakeUpNetworkPN(), true );
				CString str10bar = WriteCUDouble( _U_PRESSURE, 1e6, true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_NOTP_C_P_PNSTOOHIGH, strPSN, str10bar );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_NOTP_C_P_PNSTOOHIGH;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			// Check that Pa is well in the DPp range of the Pleno with pump.
			CDB_TecBox::TecBoxType eTecBoxType = ( (CDB_TecBox*)pSelectedPM->GetProductIDPtr().MP )->GetTecBoxType();

			if( pclTecBoxPleno->GetNbrPumpCompressor() >= 1 && m_pclInputUser->GetMinimumInitialPressure() < pclTecBoxPleno->GetWorkingPressRangeMin() )
			{
				CString strPa = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetMinimumInitialPressure(), true );
				CString strDPpMin = WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetWorkingPressRangeMin(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_PBKTK_CT_PI96_PABELOWDPPMIN, strPa, strDPpMin );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_PBKTK_CT_PI96_PABELOWDPPMIN;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			if( pclTecBoxPleno->GetNbrPumpCompressor() >= 1 && m_pclInputUser->GetFinalPressure( eTecBoxType ) > pclTecBoxPleno->GetWorkingPressRangeMax() )
			{
				CString strPe = WriteCUDouble( _U_PRESSURE, m_pclInputUser->GetFinalPressure( eTecBoxType ), true );
				CString strDPpMax = WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetWorkingPressRangeMax(), true );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_PBKTK_CT_PI96_PEABOVEDPPMAX, strPe, strDPpMax );
				rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_PBKTK_CT_PI96_PEABOVEDPPMAX;
				vecRejectedReasons.push_back( rRejectedReasons );
			}

			break;
	}

	// Check temperature.
	// HYS-1492: If Pleno is selected alone and the water make-up temperature is not valid, this Pleno is in the rejected list with the error
	if( true == bPlenoAlone )
	{
		if( m_pclInputUser->GetWaterMakeUpWaterTemp() < pclTecBoxPleno->GetTmin() )
		{
			CSelectedPMBase::RejectedReasons rRejectedReasons;
			rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_XPM_XPT_WATERMAKEUPTEMPTOOLOW;

			// The water make-up temperature is below the minimum admissible temperature for this product (%1 < %2).
			CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, m_pclInputUser->GetWaterMakeUpWaterTemp(), true );
			CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclTecBoxPleno->GetTmin(), true );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_XPM_XPT_WATERMAKEUPTEMPTOOLOW, strTemp1, strTemp2 );

			vecRejectedReasons.push_back( rRejectedReasons );
		}
		else if( m_pclInputUser->GetWaterMakeUpWaterTemp() > pclTecBoxPleno->GetTmaxWaterMakeUp() )
		{
			CSelectedPMBase::RejectedReasons rRejectedReasons;
			rRejectedReasons.m_iIDError = IDS_SELECTPMPSE_XTP_XPM_XPT_WATERMAKEUPTEMPTOOHIGH;

			// The water make-up temperature is above the maximum admissible temperature for this product (%1 > %2).
			CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, m_pclInputUser->GetWaterMakeUpWaterTemp(), true );
			CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclTecBoxPleno->GetTmaxWaterMakeUp(), true );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMPSE_XTP_XPM_XPT_WATERMAKEUPTEMPTOOHIGH, strTemp1, strTemp2 );

			vecRejectedReasons.push_back( rRejectedReasons );
		}
	}

	return true;
}

void CSelectPMList::_CheckSimplyCompressoIntegratedVessel( CDB_TecBox *pTecBox, std::vector<CSelectedPMBase::RejectedReasons> *pvecRejectReasons )
{
	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	if( NULL == pTecBox || NULL == pTecBox->GetIntegratedVesselIDPtr().MP || NULL == pvecRejectReasons )
	{
		return;
	}

	CDB_Vessel *pclIntegratedVessel = dynamic_cast<CDB_Vessel*>( pTecBox->GetIntegratedVesselIDPtr().MP );

	if( NULL == pclIntegratedVessel )
	{
		return;
	}

	CSelectedPMBase::RejectedReasons rRejectedReasons;

	// Don't use vessel that has its nominal volume multiply by PSV bigger than the value in technical parameter.
	// HYS-1402 : Use IsVolumeRespectsPressureVolumeLimit() function to consider GetSafetyValveResponsePressure > 2 bar
	if( CSelectPMList::eRejectVessel == 
		IsVolumeRespectsPressureVolumeLimit( pclIntegratedVessel, m_pclInputUser->GetSafetyValveResponsePressure() ) )
	{
		// Set the text for the 'pressure.volume' unit.
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		CString strUnit = pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str();
		strUnit += CString( _T(".") ) + pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str();

		// Value in 'localdb.txt' is set in SI units (i.e. Pascal.m3). To display value with the current unit, we must convert value SI to
		// CU in 2 phases. First one for the pressure, and second one for the volume.
		double dPressVolLimitSI = pclIntegratedVessel->GetNominalVolume() * m_pclInputUser->GetSafetyValveResponsePressure();
		double dPressVolLimitCUP = CDimValue::SItoCU( _U_PRESSURE, dPressVolLimitSI );
		double dPressVolLimitCUPV = CDimValue::SItoCU( _U_VOLUME, dPressVolLimitCUP );
		CString strPresVolVessel = WriteCUDouble( _U_NODIM, dPressVolLimitCUPV );
		strPresVolVessel += _T(" ") + strUnit;
				
		CString strPresVolLimit = m_pTADS->GetpTechParams()->GetPressureVolumeLimitCU().c_str();
				
		FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMINTVSE_PRESSVOL_XPM_XVT_PRESSVOLLIMITREACHED, strPresVolVessel, strPresVolLimit );
		rRejectedReasons.m_iIDError = IDS_SELECTPMINTVSE_PRESSVOL_XPM_XVT_PRESSVOLLIMITREACHED;
		pvecRejectReasons->push_back( rRejectedReasons );
	}

	// Test vessel nominal volume.
	double dRequestedNominalVolume = m_pclInputUser->GetVesselNominalVolume( true );
	dRequestedNominalVolume -= ( dRequestedNominalVolume / 100.0 * m_pTADS->GetpTechParams()->GetVesselSelectionRangePercentage() );

	if( dRequestedNominalVolume > pclIntegratedVessel->GetNominalVolume() * 2 )
	{
		double dMaxVolume = CDimValue::SItoCU( _U_VOLUME, pclIntegratedVessel->GetNominalVolume() * 2 );
		CString strMaxVol = WriteCUDouble( _U_VOLUME, dMaxVolume, true );
		FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVSE_XTP_XPM_XVT_NOMVOLTOOBIGFORTECBOX, strMaxVol );
		rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_XTP_XPM_XVT_NOMVOLTOOBIGFORTECBOX;
		pvecRejectReasons->push_back( rRejectedReasons );
	}

}

void CSelectPMList::_SortVsslListAndInsertInVector( ProductType eProductType, bool bVesselAlone, bool bUnlockPressureVolumeLimit, 
		vecSelectePMBase *pvecSelectedList, vecSelectePMBase *pvecRejectedList )
{
	if( NULL == m_pTADB || NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}
	
	vecSelectePMBase *pvecToSort = &m_vSelectedPMlist;
	
	if( 0 == m_vSelectedPMlist.size() )
	{
		return;
	}

	bool bForTecBox = false;

	if( eProductType == PT_CompressoVessel || eProductType == PT_TransferoVessel )
	{
		std::sort( m_vSelectedPMlist.begin(), m_vSelectedPMlist.end(), CSelectPMList::_CompareTecBoxVessel );
		bForTecBox = true;
	}
	else
	{
		std::sort( m_vSelectedPMlist.begin(), m_vSelectedPMlist.end(), CSelectPMList::_CompareVssl );
	}

	if( NULL == pvecSelectedList )
	{
		pvecSelectedList = &m_arvecSelectedProducts[eProductType];
	}

	if( NULL == pvecRejectedList )
	{
		pvecRejectedList = &m_arvecRejectedProducts[eProductType];
	}

	CDB_StringID *pBestStrType = NULL;
	CDB_StringID *pBestStrFam = NULL;
	CDB_Vessel *pBestVessel = NULL;
	CDB_Vessel *pPrevVssl = NULL;

	// Determine if the number of the biggest vessel is greater than max number allowed for additional vessels in parallel.
	// It allows us to display at least one big vessel even if the number of vessels in parallel exceeds the max. authorized 
	// in the technical parameters.
	int iMaxNumberOfAdditionalVesselInParallel = max( ( (CSelectedVssl *)m_vSelectedPMlist.at( m_vSelectedPMlist.size() - 1 ) )->GetNbreOfVsslNeeded() - 1,
			m_pTADS->GetpTechParams()->GetMaxNumberOfAdditionalVesselsInParallel() );

	int iBestNbreVssl = INT_MAX;
	int iBestVsslIndex = -1;
	double dBestDelta = DBL_MAX;

	// Array is ordered by type, family, number of vessels, VNDelta and Pmaxmax, so the first vessel of each type/family is the best.
	for( unsigned i = 0; i < m_vSelectedPMlist.size(); i++ )
	{
		// identify Best Vessel
		// For each type, minimum number of Vessel, VNDelta minimum
		CDB_Vessel *pcdbVssl = dynamic_cast<CDB_Vessel *>( m_vSelectedPMlist.at( i )->GetProductIDPtr().MP );
		CSelectedVssl *pclSelVssl = dynamic_cast <CSelectedVssl *>( m_vSelectedPMlist.at( i ) );

		if( NULL == pcdbVssl )
		{
			ASSERT( 0 );
			continue;
		}

		if( pclSelVssl->GetRejectReasonCount() > 0 )
		{
			// Break. Already a reason to be rejected, we don't take this one.
			continue;
		}

		CDB_StringID *pStrType = (CDB_StringID *)pcdbVssl->GetTypeIDPtr().MP;
		CDB_StringID *pStrFam = (CDB_StringID *)pcdbVssl->GetFamilyIDPtr().MP;

		if( pStrType != pBestStrType )
		{
			// New type.
			pBestStrType = pStrType;
			pBestStrFam = pStrFam;
			pBestVessel = NULL;
			iBestNbreVssl = INT_MAX;
			iBestVsslIndex = -1;
		}

		bool bEligibleToBest = false;

		if( NULL != pBestVessel 
				&& pBestVessel->GetTypeIDPtr().MP == pStrType 
				&& pBestVessel->GetNominalVolume() == pcdbVssl->GetNominalVolume() )
		{
			if( pBestVessel->GetFamilyIDPtr().MP == pStrFam  )
			{
				// Allow to differentiate 2 vessels with same type, family but pmaxmax different.
				// Ex: CU 300.6 and CU 300.10

				if( pcdbVssl->GetPmaxmax() < pBestVessel->GetPmaxmax() )
				{
					bEligibleToBest = true;
				}
			}
			else if( pcdbVssl->GetPmaxmax() == pBestVessel->GetPmaxmax() )
			{
				// Allow to differentiate 2 vessels with same type, pmaxmax but family different.
				// Ex: CU 300.6 and CG 300.6

				CDB_StringID *pStrBestFam = (CDB_StringID *)pBestVessel->GetFamilyIDPtr().MP;
				int iStrFam;
				pStrFam->GetIDstrAsInt( 0, iStrFam );
				int iStrBestFam;
				pStrBestFam->GetIDstrAsInt( 0, iStrBestFam );

				if( iStrFam < iStrBestFam )
				{
					bEligibleToBest = true;
				}
			}
		}

		// Test use of pressure volume limit.
		bool bPressureVolumeLimitCompliant = true;

		// HYS-1003: We can force to derogate to this rule. It can happen that there is no result with big installation for example. In this case
		// we will show results with a warning.
		if( false == bUnlockPressureVolumeLimit )
		{
			// HYS-1402 : Use IsVolumeRespectsPressureVolumeLimit() function to consider GetSafetyValveResponsePressure > 2 bar
			if( CSelectPMList::eRejectVessel == 
				IsVolumeRespectsPressureVolumeLimit( pcdbVssl, m_pclInputUser->GetSafetyValveResponsePressure() ) )
			{
				bPressureVolumeLimitCompliant = false;
			}
		}
		
		double dRequestedNominalVolume = m_pclInputUser->GetVesselNominalVolume( bForTecBox );
		dRequestedNominalVolume -= ( dRequestedNominalVolume / 100.0 * m_pTADS->GetpTechParams()->GetVesselSelectionRangePercentage() );

		if( true == bPressureVolumeLimitCompliant )
		{
			if( NULL == pBestVessel )
			{
				// We accept the first one as the best.
				bEligibleToBest = true;
			}
			else
			{
				if( pclSelVssl->GetNbreOfVsslNeeded() < iBestNbreVssl )
				{
					// As soon as we need less vessel, it becomes the best.
					bEligibleToBest = true;
				}
				else if( pclSelVssl->GetNbreOfVsslNeeded() * pcdbVssl->GetNominalVolume() - dRequestedNominalVolume < dBestDelta )
				{
					bEligibleToBest = true;
				}
			}
		}
		else
		{
			bEligibleToBest = false;

			if( true == m_pclInputUser->UsePressureVolumeLimit() )
			{
				pclSelVssl->SetFlag( CSelectedPMBase::eVslFlags3000lbar, true );
			}
		}
			   
		// Test maximum limit of number of vessel.
		pclSelVssl->SetFlag( CSelectedPMBase::eVslFlagsNbreOfVssl,
				( ( pclSelVssl->GetNbreOfVsslNeeded() - 1 ) > m_pTADS->GetpTechParams()->GetMaxNumberOfAdditionalVesselsInParallel() ) );

		if( true == bEligibleToBest )
		{
			// Reset previous best flag if exist
			if( NULL != pBestVessel )
			{
				if( iBestVsslIndex >= 0 )
				{
					m_vSelectedPMlist.at( iBestVsslIndex )->SetFlag( CSelectedPMBase::eBest, false );
					m_vSelectedPMlist.at( iBestVsslIndex )->SetFlag( CSelectedPMBase::eNotPriority, true );
				}
				else
				{
					ASSERT( 0 );
				}
			}

			// Tag it as the best.
			pclSelVssl->SetFlag( CSelectedPMBase::eBest, true );
			pclSelVssl->SetFlag( CSelectedPMBase::eNotPriority, false );
			pBestVessel = pcdbVssl;
			iBestNbreVssl = pclSelVssl->GetNbreOfVsslNeeded();
			iBestVsslIndex = i;

			if( ( pclSelVssl->GetNbreOfVsslNeeded() * pcdbVssl->GetNominalVolume() ) - dRequestedNominalVolume < dBestDelta )
			{
				dBestDelta = ( pclSelVssl->GetNbreOfVsslNeeded() * pcdbVssl->GetNominalVolume() ) - dRequestedNominalVolume;
			}
		}
		else
		{
			pclSelVssl->SetFlag( CSelectedPMBase::eBest, false );
			pclSelVssl->SetFlag( CSelectedPMBase::eNotPriority, true );
		}

		pPrevVssl = pcdbVssl;
	}

	std::vector< CSelectedPMBase::RejectedReasons > vecRejectReasons;

	// Transfer list.
	int iCounter = 0;
	double dLowestPrice = DBL_MAX;
	CSelectedPMBase* pclPMLowestPrice = NULL;
	vecSelectePMBaseIter iter = m_vSelectedPMlist.begin();

	// Keep bigger vessel to allow to take at least the two ones just above the best one.
	std::multimap<double, CSelectedVssl *> mmapBiggerVessels;

	while( iter != m_vSelectedPMlist.end() )
	{
		CSelectedPMBase::RejectedReasons rRejectedReasons;
		vecRejectReasons.clear();
		CDB_Vessel *pcdbVssl = dynamic_cast<CDB_Vessel *>( (*iter)->GetProductIDPtr().MP );
		CSelectedVssl *pclSelVssl = dynamic_cast <CSelectedVssl *>( *iter );

		if( NULL == pcdbVssl )
		{
			ASSERT( 0 );
			continue;
		}

		if( pclSelVssl->GetNbreOfVsslNeeded() - 1 > iMaxNumberOfAdditionalVesselInParallel )
		{
			CString strNbrVessel;
			strNbrVessel.Format( _T("%i"), pclSelVssl->GetNbreOfVsslNeeded() - 1 );
			CString strMaxNbrVessel;
			strMaxNbrVessel.Format( _T("%i"), iMaxNumberOfAdditionalVesselInParallel );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVSE_XTP_XPM_XVT_NBRVESSELABOVETP, strNbrVessel, strMaxNbrVessel );
			rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_XTP_XPM_XVT_NBRVESSELABOVETP;
			vecRejectReasons.push_back( rRejectedReasons );
		}
		
		if( NULL != pBestVessel && pcdbVssl->GetNominalVolume() > pBestVessel->GetNominalVolume() )
		{
			CString strNomVol = WriteCUDouble( _U_VOLUME, pcdbVssl->GetNominalVolume(), true );
			CString strBestNomVol = WriteCUDouble( _U_VOLUME, pBestVessel->GetNominalVolume(), true );
			FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVSE_XTP_XPM_XVT_NOMVOLBIGGERBEST, strNomVol, strBestNomVol );
			rRejectedReasons.m_iIDError = IDS_SELECTPMVSE_XTP_XPM_XVT_NOMVOLBIGGERBEST;
			vecRejectReasons.push_back( rRejectedReasons );

			// Add also in the bigger vessels list to check after.
			mmapBiggerVessels.insert(std::make_pair( pcdbVssl->GetNominalVolume(), pclSelVssl ) );
		}

		// If there is already an error or a new error has been detected just above...
		if( pclSelVssl->GetRejectReasonCount() > 0 || 0 != (int)vecRejectReasons.size() )
		{
			if( 0 != (int)vecRejectReasons.size() )
			{
				pclSelVssl->AddRejectedReasons( vecRejectReasons );
			}

			// Add to the rejected list.
			pclSelVssl->SetRejected();
			m_vRejectedPMlist.push_back( pclSelVssl );

			// Remove from the selected list.
			m_vSelectedPMlist.erase( iter );
			iter = m_vSelectedPMlist.begin() + iCounter;

			pvecRejectedList->push_back( pclSelVssl );

			continue;
		}

		if( true == bVesselAlone )
		{
			// When vessel is selected alone (Statico, Membrane or Squeeze for example), we have to multiply price of one vessel by the needed
			// number of vessels.
			double dTotalPrice = m_pTADB->GetPrice( pcdbVssl->GetArtNum( true ) );

			if( 0.0 != dTotalPrice && ( dTotalPrice * pclSelVssl->GetNbreOfVsslNeeded() < dLowestPrice ) )
			{
				dLowestPrice = dTotalPrice * pclSelVssl->GetNbreOfVsslNeeded();
				pclPMLowestPrice = pclSelVssl;
			}
		}
		else
		{
			// When vessel is not selected alone (With Compresso/Transfero for example), we can have the primary vessel (integrated or not) and 
			// the secondary vessel.
			double dTotalPrice = 0.0;

			if( false == pclSelVssl->IsPrimaryIntegrated() )
			{
				dTotalPrice += m_pTADB->GetPrice( pcdbVssl->GetArtNum( true ) );
			}

			if( pclSelVssl->GetNbreOfVsslNeeded() > 1 )
			{
				CDB_Vessel *pclSecondaryVessel = dynamic_cast<CDB_Vessel*>( pcdbVssl->GetSecVsslIDPtr().MP );

				if( NULL != pclSecondaryVessel )
				{
					double dSecVesselPrice = m_pTADB->GetPrice( pclSecondaryVessel->GetArtNum( true ) );
					dTotalPrice += dSecVesselPrice * ( pclSelVssl->GetNbreOfVsslNeeded() - 1 );
				}
			}

			if( 0.0 != dTotalPrice && dTotalPrice < dLowestPrice )
			{
				dLowestPrice = dTotalPrice;
				pclPMLowestPrice = pclSelVssl;
			}
		}

		pvecSelectedList->push_back( pclSelVssl );

		++iter;
		++iCounter;
	}

	m_arsPriceData[eProductType].m_dLowestPrice = dLowestPrice;
	m_arsPriceData[eProductType].m_pclSelectedPMBase = pclPMLowestPrice;

	// If there are bigger vessels than the best one...
	if( mmapBiggerVessels.size() > 0 )
	{
		// Check first if the first bigger vessel are pressure.volume compliant.
		if( false == ( *mmapBiggerVessels.begin() ).second->GetFlag( CSelectedPMBase::eVslFlags3000lbar ) )
		{
			double dVNVessel = ( *mmapBiggerVessels.begin() ).first;
			bool bSecond = false;

			for( auto &iter : mmapBiggerVessels )
			{
				if( iter.first != dVNVessel )
				{
					if( false == bSecond )
					{
						dVNVessel = iter.first;
						bSecond = true;
					}
					else
					{
						// Just take two sizes of vessel bigger than the best.
						break;
					}
				}

				// We need to check if there is only one error.
				if( iter.second->GetRejectReasonCount() > 1 )
				{
					continue;
				}

				int iIDError;
				iter.second->GetFirstRejectedReason( &iIDError );

				// And if this error is the 'IDS_SELECTPMVSE_XTP_XPM_XVT_NOMVOLBIGGERBEST'.
				if( IDS_SELECTPMVSE_XTP_XPM_XVT_NOMVOLBIGGERBEST != iIDError )
				{
					continue;
				}

				// Clear error message and add the vessel in the selected list.
				iter.second->ClearRejectedReasons();
				pvecSelectedList->push_back( iter.second );

				// Remove from the rejected list.
				auto it = std::find( pvecRejectedList->begin(), pvecRejectedList->end(), iter.second );

				if( it != pvecRejectedList->end() )
				{
					pvecRejectedList->erase( it );
				}
			}
		}
	}

	// Now we can clear all error flags for the vessels in the rejected list. We do that here because in the lines just above
	// we parse the 'mmapBiggerVessels' map to take the two bigger vessels above the best one. And we need the error flags for
	// these vessels. We clear than all the rest now.
	if( pvecRejectedList->size() > 0 )
	{
		for( auto &iter : *pvecRejectedList )
		{
			iter->ClearAllFlags();
		}
	}

	if( eProductType != PT_CompressoVessel && eProductType != PT_TransferoVessel )
	{
		std::sort( pvecSelectedList->begin(), pvecSelectedList->end(), CSelectPMList::_CompareVsslAfter );
		std::sort( pvecRejectedList->begin(), pvecRejectedList->end(), CSelectPMList::_CompareVsslAfter );
	}
}

void CSelectPMList::_SortCompressoAndInsertInVector( WhichList eWhatToSort )
{
	if( NULL == m_pTADB )
	{
		ASSERT_RETURN;
	}

	vecSelectePMBase *pvecToSort = ( WhichList::WL_Selected == eWhatToSort ) ? &m_vSelectedPMlist : &m_vRejectedPMlist;

	if( 0 == pvecToSort->size() )
	{
		return;
	}
	
	vecSelectePMBase *pvecWhereToInsert = ( WhichList::WL_Selected == eWhatToSort ) ? &m_arvecSelectedProducts[ProductType::PT_Compresso] : 
			&m_arvecRejectedProducts[ProductType::PT_Compresso];

	// HYS-981: clear all flags before sorting.
	if( WhichList::WL_Rejected == eWhatToSort )
	{
		for( auto iter : *pvecToSort )
		{
			iter->ClearAllFlags();
		}
	}

	std::sort( pvecToSort->begin(), pvecToSort->end(), CSelectPMList::_CompareCompressoTransfero );

	// Transfer list.
	double dLowestPrice = DBL_MAX;
	CSelectedPMBase *pclPMLowestPrice = NULL;

	CString strCompressoCurveIDRef = _T("");

	for( unsigned i = 0; i < pvecToSort->size(); i++ )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pvecToSort->at( i )->GetProductIDPtr().MP );

		if( NULL == pclTecBox )
		{
			ASSERT_RETURN;
		}

		if( WhichList::WL_Selected == eWhatToSort )
		{
			bool bNotPriority = true;
			CDB_TBCurve *pclTBCurveCurrent = dynamic_cast<CDB_TBCurve*>( pclTecBox->GetPumpComprCurveIDPtr().MP );

			if( NULL != pclTBCurveCurrent )
			{
				if( 0 == i )
				{
					// We keep the first item as a priority.
					strCompressoCurveIDRef = CString( pclTBCurveCurrent->GetIDPtr().ID );
					bNotPriority = false;
				}
				else if( strCompressoCurveIDRef == CString( pclTBCurveCurrent->GetIDPtr().ID ) 
						&& CPMInputUser::MT_All != m_pclInputUser->GetPressureMaintenanceType() )
				{
					// Other elements are not prioritized if the Compressor has the same curve ID and if we are not
					// in all pressurisation maintenance type.
					bNotPriority = false;
				}
			}

			pvecToSort->at( i )->SetFlag( CSelectedPMBase::eNotPriority, bNotPriority );
			pvecToSort->at( i )->SetFlag( CSelectedPMBase::eBest, 0 == i );

			double dPrice = m_pTADB->GetPrice( pclTecBox->GetArtNum( true ) );

			if( 0 != _tcscmp( pclTecBox->GetFamilyID(), _T("TBXFAM_COMPR_CX") ) && 0.0 != dPrice && dPrice < dLowestPrice )
			{
				dLowestPrice = dPrice;
				pclPMLowestPrice = pvecToSort->at( i );
			}
		}

		// Remark: 'mmapKeyPMBase' is ordered in an inverse order (see definition).
		pvecWhereToInsert->push_back( pvecToSort->at( i ) );
	}

	if( WhichList::WL_Selected == eWhatToSort )
	{
		m_arsPriceData[ProductType::PT_Compresso].m_dLowestPrice = dLowestPrice;
		m_arsPriceData[ProductType::PT_Compresso].m_pclSelectedPMBase = pclPMLowestPrice;
	}
}

void CSelectPMList::_SortTransferoAndInsertInVector( WhichList eWhatToSort )
{
	if( NULL == m_pTADB )
	{
		ASSERT_RETURN;
	}

	vecSelectePMBase *pvecToSort = ( WhichList::WL_Selected == eWhatToSort ) ? &m_vSelectedPMlist : &m_vRejectedPMlist;

	if( 0 == pvecToSort->size() )
	{
		return;
	}
	
	vecSelectePMBase *pvecWhereToInsert = ( WhichList::WL_Selected == eWhatToSort ) ? &m_arvecSelectedProducts[ProductType::PT_Transfero] : 
			&m_arvecRejectedProducts[ProductType::PT_Transfero];

	// HYS-981: clear all flags before sorting.
	if( WhichList::WL_Rejected == eWhatToSort )
	{
		for( auto iter : *pvecToSort )
		{
			iter->ClearAllFlags();
		}
	}

	std::sort( pvecToSort->begin(), pvecToSort->end(), CSelectPMList::_CompareCompressoTransfero );

	// Transfer list.
	double dLowestPrice = DBL_MAX;
	CSelectedPMBase *pclPMLowestPrice = NULL;

	for( unsigned i = 0; i < pvecToSort->size(); i++ )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pvecToSort->at( i )->GetProductIDPtr().MP );

		if( NULL == pclTecBox )
		{
			continue;
		}

		if( WhichList::WL_Selected == eWhatToSort )
		{
			pvecToSort->at( i )->SetFlag( CSelectedPMBase::eBest, 0 == i );
			pvecToSort->at( i )->SetFlag( CSelectedPMBase::eNotPriority, 0 != i );

			double dPrice = m_pTADB->GetPrice( pclTecBox->GetArtNum( true ) );

			if( 0 != _tcscmp( pclTecBox->GetFamilyID(), _T("TBXFAM_COMPR_CX") ) && 0.0 != dPrice && dPrice < dLowestPrice )
			{
				dLowestPrice = dPrice;
				pclPMLowestPrice = pvecToSort->at( i );
			}
		}

		// Remark: 'mmapKeyPMBase' is ordered in an inverse order (see definition).
		pvecWhereToInsert->push_back( pvecToSort->at( i ) );
	}

	if( WhichList::WL_Selected == eWhatToSort )
	{
		m_arsPriceData[ProductType::PT_Transfero].m_dLowestPrice = dLowestPrice;
		m_arsPriceData[ProductType::PT_Transfero].m_pclSelectedPMBase = pclPMLowestPrice;
	}
}

void CSelectPMList::_SortPlenoAndInsertInVector( WhichList eWhatToSort, ProductType ePType )
{
	if( NULL == m_pTADB )
	{
		ASSERT_RETURN;
	}

	vecSelectePMBase *pvecToSort = ( WhichList::WL_Selected == eWhatToSort ) ? &m_vSelectedPMlist : &m_vRejectedPMlist;

	if( 0 == (int)pvecToSort->size() )
	{
		return;
	}

	vecSelectePMBase *pvecWhereToInsert = ( WhichList::WL_Selected == eWhatToSort ) ? &m_arvecSelectedProducts[ePType] :
			&m_arvecRejectedProducts[ePType];

	bool bWorkOnSelected = ( WL_Selected == eWhatToSort ) ? true : false;

	// HYS-981: clear all flags before sorting.
	if( WhichList::WL_Rejected == eWhatToSort )
	{
		for( auto iter : *pvecToSort )
		{
			iter->ClearAllFlags();
		}
	}

	std::sort( pvecToSort->begin(), pvecToSort->end(), CSelectPMList::_ComparePlenoVento );

	// Transfer list.
	vecSelectePMBaseIter vecBestIndexIter = pvecToSort->end();
	double dLowestPrice = DBL_MAX;
	CSelectedPMBase *pclPMLowestPrice = NULL;

	for( vecSelectePMBaseIter iter = pvecToSort->begin(); iter != pvecToSort->end(); ++iter )
	{
		CDB_TBPlenoVento* pclPleno = dynamic_cast<CDB_TBPlenoVento*>( (*iter)->GetProductIDPtr().MP );
		
		if( NULL == pclPleno )
		{
			continue;
		}

		if( pvecToSort->end() == vecBestIndexIter && false == (*iter)->GetFlag( CSelectedPMBase::efAll ) )
		{
			vecBestIndexIter = iter;
		}

		if( WhichList::WL_Selected == eWhatToSort )
		{
			(*iter)->SetFlag( CSelectedPMBase::eNotPriority, true );

			double dPrice = m_pTADB->GetPrice( pclPleno->GetArtNum( true ) );
			
			if( 0.0 != dPrice && dPrice < dLowestPrice )
			{
				dLowestPrice = dPrice;
				pclPMLowestPrice = (*iter);
			}
		}
		
		// Remark: 'mmapKeyPMBase' is ordered in an inverse order (see definition).
		// Remark: Verify first if item is not already in the vector. It can happen when user selects for example a vessel with 
		//         degassing and water make-up. If user selects a vessel, we show compatible Pleno device. And if user selects a
		//         degassing system we add also Pleno that are compatible with the Vento. In this case, we don't clear 'm_arvecSelectedProducts'
		//         but we have to pay attention to not add twice the same object.
		if( pvecWhereToInsert->end() == std::find( pvecWhereToInsert->begin(), pvecWhereToInsert->end(), *iter) )
		{
			pvecWhereToInsert->push_back( (*iter) );
		}
	}

	if( pvecToSort->end() == vecBestIndexIter )
	{
		vecBestIndexIter = pvecToSort->begin();
	}

	if( pvecWhereToInsert->size() > 0 && WhichList::WL_Selected == eWhatToSort )
	{
		(*vecBestIndexIter)->SetFlag( CSelectedPMBase::eBest, true );
		(*vecBestIndexIter)->SetFlag( CSelectedPMBase::eNotPriority, false );

		m_arsPriceData[ePType].m_dLowestPrice = dLowestPrice;
		m_arsPriceData[ePType].m_pclSelectedPMBase = pclPMLowestPrice;
	}
}

void CSelectPMList::_SortPlenoRefillAndInsertInVector( WhichList eWhatToSort )
{
	vecSelectePMBase *pvecToSort = ( WhichList::WL_Selected == eWhatToSort ) ? &m_vSelectedPMlist : &m_vRejectedPMlist;

	if( 0 == (int)pvecToSort->size() )
	{
		return;
	}

	vecSelectePMBase *pvecWhereToInsert = ( WhichList::WL_Selected == eWhatToSort ) ? &m_arvecSelectedProducts[ProductType::PT_PlenoRefill] : 
		&m_arvecRejectedProducts[ProductType::PT_PlenoRefill];

	// HYS-981: clear all flags before sorting.
	if( WhichList::WL_Rejected == eWhatToSort )
	{
		for( auto iter : *pvecToSort )
		{
			iter->ClearAllFlags();
		}
	}

	bool fWorkOnSelected = ( WL_Selected == eWhatToSort ) ? true : false;
	std::sort( pvecToSort->begin(), pvecToSort->end(), CSelectPMList::_ComparePlenoRefill );

	for( vecSelectePMBaseIter iter = pvecToSort->begin(); iter != pvecToSort->end(); ++iter )
	{
		CDB_PlenoRefill* pclPlenoRefill = dynamic_cast<CDB_PlenoRefill*>( (*iter)->GetProductIDPtr().MP );
		
		if( NULL == pclPlenoRefill )
		{
			continue;
		}

		if( WhichList::WL_Selected == eWhatToSort )
		{
			(*iter)->SetFlag( CSelectedPMBase::eNotPriority, false );
		}
		
		// Remark: 'mmapKeyPMBase' is ordered in an inverse order (see definition).
		pvecWhereToInsert->push_back( (*iter) );
	}
}

void CSelectPMList::_SortVentoAndInsertInVector( WhichList eWhatToSort, bool bAllAreBiggerThanMaxNumberOfVentoInParallel, 
		int iLowerMaxNumberOfVentoInParallel )
{
	if( NULL == m_pTADB || NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}
	
	vecSelectePMBase *pvecToSort = ( WhichList::WL_Selected == eWhatToSort ) ? &m_vSelectedPMlist : &m_vRejectedPMlist;

	if( 0 == pvecToSort->size() )
	{
		return;
	}

	vecSelectePMBase *pvecWhereToInsert = ( WhichList::WL_Selected == eWhatToSort ) ? &m_arvecSelectedProducts[ProductType::PT_Vento] : 
			&m_arvecRejectedProducts[ProductType::PT_Vento];

	// HYS-981: clear all flags before sorting.
	if( WhichList::WL_Rejected == eWhatToSort )
	{
		for( auto iter : *pvecToSort )
		{
			iter->ClearAllFlags();
		}
	}

	std::sort( pvecToSort->begin(), pvecToSort->end(), CSelectPMList::_ComparePlenoVento );

	// Transfer list.
	int iBestIndex = -1;
	
	std::vector<CSelectedPMBase::RejectedReasons> vecRejectReasons;

	int iCounter = 0;
	vecSelectePMBaseIter iter = pvecToSort->begin();
	double dLowestPrice = DBL_MAX;
	CSelectedPMBase *pclPMLowestPrice = NULL;

	while( iter != pvecToSort->end() )
	{
		CSelectedPMBase::RejectedReasons rRejectedReasons;
		vecRejectReasons.clear();
		CSelectedVento *pclSelectedVento = dynamic_cast<CSelectedVento *>( *iter );

		if( NULL == pclSelectedVento )
		{
			++iter;
			++iCounter;
			continue;
		}

		CDB_TBPlenoVento* pclVento = dynamic_cast<CDB_TBPlenoVento*>( pclSelectedVento->GetProductIDPtr().MP );
		
		if( NULL == pclVento )
		{
			++iter;
			++iCounter;
			continue;
		}

		if( false == bAllAreBiggerThanMaxNumberOfVentoInParallel )
		{
			// If some solutions have a number of Vento lower than the max number of Vento set in technical parameters, we keep only those that
			// have a number of Vento below this technical parameter.
			if( pclSelectedVento->GetNumberOfVento() > m_pTADS->GetpTechParams()->GetMaxNumberOfVentoInParallel() )
			{
				CString strNbrVento;
				strNbrVento.Format( _T("%i"), pclSelectedVento->GetNumberOfVento() );
				CString strMaxNbrVento;
				strMaxNbrVento.Format( _T("%i"), m_pTADS->GetpTechParams()->GetMaxNumberOfVentoInParallel() );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_NBRVENTOABOVETP, strNbrVento, strMaxNbrVento );
				rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_NBRVENTOABOVETP;
				vecRejectReasons.push_back( rRejectedReasons );
			}
		}
		else
		{
			// If all solutions have a number of Vento bigger than the max number of Vento set in technical parameters, we keep only those that have
			// the lower number of Vento in parallel.
			if( pclSelectedVento->GetNumberOfVento() != iLowerMaxNumberOfVentoInParallel )
			{
				CString strNbrVento;
				strNbrVento.Format( _T("%i"), pclSelectedVento->GetNumberOfVento() );
				CString strLowerMaxNbrVento;
				strLowerMaxNbrVento.Format( _T("%i"), iLowerMaxNumberOfVentoInParallel );
				FormatString( rRejectedReasons.m_strErrorMsg, IDS_SELECTPMVTSE_XTP_XPM_XP_NBRVENTOABOVELOWERSOL, strNbrVento, strLowerMaxNbrVento );
				rRejectedReasons.m_iIDError = IDS_SELECTPMVTSE_XTP_XPM_XP_NBRVENTOABOVELOWERSOL;
				vecRejectReasons.push_back( rRejectedReasons );
			}
		}

		if( true == bAllAreBiggerThanMaxNumberOfVentoInParallel )
		{
			pclSelectedVento->SetFlag( CSelectedPMBase::eVentoMaxParallel, true );
		}

		if( 0 != (int)vecRejectReasons.size() && WhichList::WL_Selected == eWhatToSort )
		{
			pclSelectedVento->AddRejectedReasons( vecRejectReasons );

			// Add to the rejected list.
			pclSelectedVento->SetRejected();
			m_vRejectedPMlist.push_back( pclSelectedVento );
				
			// Remove from the selected list.
			m_vSelectedPMlist.erase( iter );
			iter = m_vSelectedPMlist.begin() + iCounter;

			continue;
		}

		if( -1 == iBestIndex && false == pclSelectedVento->GetFlag( CSelectedPMBase::efAll ) )
		{
			iBestIndex = iCounter;
		}
		
		if( WhichList::WL_Selected == eWhatToSort )
		{
			pclSelectedVento->SetFlag( CSelectedPMBase::eNotPriority, true );

			double dPrice = m_pTADB->GetPrice( pclVento->GetArtNum( true ) );

			if( 0.0 != dPrice && dPrice * pclSelectedVento->GetNumberOfVento() < dLowestPrice )
			{
				dLowestPrice = dPrice * pclSelectedVento->GetNumberOfVento();
				pclPMLowestPrice = pclSelectedVento;
			}
		}

		// Remark: 'mmapKeyPMBase' is ordered in an inverse order (see definition).
		pvecWhereToInsert->push_back( pclSelectedVento );

		++iter;
		++iCounter;
	}

	if( -1 == iBestIndex )
	{
		iBestIndex = 0;
	}

	if( pvecWhereToInsert->size() > 0 && WhichList::WL_Selected == eWhatToSort )
	{
		pvecWhereToInsert->at( iBestIndex)->SetFlag( CSelectedPMBase::eBest, true );
		pvecWhereToInsert->at( iBestIndex)->SetFlag( CSelectedPMBase::eNotPriority, false );

		m_arsPriceData[ProductType::PT_Vento].m_dLowestPrice = dLowestPrice;
		m_arsPriceData[ProductType::PT_Vento].m_pclSelectedPMBase = pclPMLowestPrice;
	}
}

void CSelectPMList::_SetBestCompressoTecBox()
{
	// HYS-982
	// Compresso are sorted with the best in first position.
	// Compresso vessels (when calling '_SortVsslListAndInsertInVector') are sorted in reverse order with two keys:
	//   1 - biggest number of needed vessel in first position
	//   2 - lowest vessel volume in first position -> Ex: 3 * CU 200.6; 2 * CU 300.10; 2 * CU 300.6; 2 * CU 400.6; 2 * CU 500.6; 1 * CU 600.6
	// For each Compresso, we retrieve the vessel list and we parse it in reverse order. We take the lowest number of needed vessel and the lowest
	// volume. In this way, we are compliant with Christos Bozatzidis.

	bool bBestSet = false;

	int iLowestVesselNbr = INT_MAX;
	double dLowestVessetVolume = DBL_MAX;
	CSelectedPMBase *pclBestCompresso = NULL;
	CDB_Vessel *pclBestVessel = NULL;

	for( CSelectedPMBase *pclSelectedProduct = GetFirstProduct( CSelectPMList::PT_Compresso ); NULL != pclSelectedProduct; 
			pclSelectedProduct = GetNextProduct( CSelectPMList::PT_Compresso ) )
	{
		CDB_TecBox *pclCompresso = dynamic_cast<CDB_TecBox *>( pclSelectedProduct->GetProductIDPtr().MP );

		if( NULL == pclCompresso )
		{
			ASSERT( 0 );
			continue;
		}

		// By default set that the current is not the best.
		pclSelectedProduct->SetFlag( CSelectedPMBase::eBest, false );

		_CleanVecKeyPMBase( ProductType::PT_CompressoVessel );
		IDPTR idptr = m_pTADB->Get( _T("VSSLTYPE_PRIM") );

		double dTecBoxHeight = 0.0;

		if( CDB_TecBox::eilVesselTop == ( pclCompresso->GetInstallationLayout() & CDB_TecBox::eilVesselTop ) )
		{
			dTecBoxHeight = pclCompresso->GetHeight();
		}

		// HYS-1040: For compresso with integrated vessel don't check primary vessel
		if( true == pclCompresso->IsVariantIntegratedPrimaryVessel() )
		{
			// Case of Simply Compresso for example.
			_VerifyIntegratedVessel( pclCompresso );
			pclBestCompresso = pclSelectedProduct;
			_SortVsslListAndInsertInVector( ProductType::PT_CompressoVessel, false );
			break;
		}
		else
		{	
			_SelectVessel( CDB_Vessel::ePMComp_Compresso, (CDB_StringID *)idptr.MP, false, NULL, dTecBoxHeight, pclCompresso );
		}

		_SortVsslListAndInsertInVector( ProductType::PT_CompressoVessel, false );

		for( std::vector<CSelectedPMBase*>::reverse_iterator riter = m_arvecSelectedProducts[ProductType::PT_CompressoVessel].rbegin();
				riter != m_arvecSelectedProducts[ProductType::PT_CompressoVessel].rend(); riter++ )
		{
			CSelectedVssl *pclSelectedPrimaryVessel = dynamic_cast<CSelectedVssl*>( *riter );
			CDB_Vessel *pclPrimaryVessel = dynamic_cast<CDB_Vessel*>( pclSelectedPrimaryVessel->GetProductIDPtr().MP );

			if( NULL == pclPrimaryVessel )
			{
				ASSERT( 0 );
				continue;
			}

			if( pclSelectedPrimaryVessel->GetNbreOfVsslNeeded() < iLowestVesselNbr )
			{
				iLowestVesselNbr = pclSelectedPrimaryVessel->GetNbreOfVsslNeeded();
				dLowestVessetVolume = pclPrimaryVessel->GetNominalVolume();
				// pclBestCompresso = pclSelectedProduct;
				pclBestVessel = pclPrimaryVessel;
			}
			else if( pclSelectedPrimaryVessel->GetNbreOfVsslNeeded() == iLowestVesselNbr 
					&& pclPrimaryVessel->GetNominalVolume() < dLowestVessetVolume )
			{
				dLowestVessetVolume = pclPrimaryVessel->GetNominalVolume();
				// pclBestCompresso = pclSelectedProduct;
				pclBestVessel = pclPrimaryVessel;
			}
		}
	}

	if( NULL != pclBestVessel )
	{
		for( CSelectedPMBase *pclSelectedProduct = GetFirstProduct( CSelectPMList::PT_Compresso ); NULL != pclSelectedProduct; 
				pclSelectedProduct = GetNextProduct( CSelectPMList::PT_Compresso ) )
		{
			CDB_TecBox *pclCompresso = dynamic_cast<CDB_TecBox *>( pclSelectedProduct->GetProductIDPtr().MP );

			if( NULL == pclCompresso )
			{
				ASSERT( 0 );
				continue;
			}

			if( pclCompresso->GetPmaxmax() == pclBestVessel->GetPSch() )
			{
				pclBestCompresso = pclSelectedProduct;
				break;
			}
		}
	}

	if( NULL == pclBestCompresso )
	{
		pclBestCompresso = GetFirstProduct( CSelectPMList::PT_Compresso );
	}

	pclBestCompresso->SetFlag( CSelectedPMBase::Flags::eBest, true );
}

void CSelectPMList::_CleanVecKeyPMBase( ProductType eProductType, vecSelectePMBase *pvecSelectedToClean, vecSelectePMBase *pvecRejectedToClean )
{
	if( NULL == pvecSelectedToClean && NULL == pvecRejectedToClean )
	{
		for( int i = WhichList::WL_Selected; i <= WhichList::WL_Rejected; i++ )
		{
			vecSelectePMBase *pvecToClear = ( WhichList::WL_Selected == i ) ? &m_arvecSelectedProducts[eProductType] : 
					&m_arvecRejectedProducts[eProductType];

			if( 0 != ( int )pvecToClear->size() )
			{
				for( auto &iter : *pvecToClear )
				{
					if( NULL != iter )
					{
						delete iter;
					}
				}

				pvecToClear->clear();
			}
		}
	}
	else
	{
		if( NULL != pvecSelectedToClean && 0 != ( int )pvecSelectedToClean->size() )
		{
			for( auto &iter : *pvecSelectedToClean )
			{
				if( NULL != iter )
				{
					delete iter;
				}
			}

			pvecSelectedToClean->clear();
		}

		if( NULL != pvecRejectedToClean && 0 != ( int )pvecRejectedToClean->size() )
		{
			for( auto &iter : *pvecRejectedToClean )
			{
				if( NULL != iter )
				{
					delete iter;
				}
			}

			pvecRejectedToClean->clear();
		}
	}
}

void CSelectPMList::_ComputePriceIndex( bool bFirstPass )
{
	if( NULL == m_pTADB )
	{
		ASSERT_RETURN;
	}

	// 2017-10-06: I reset this value to 'false'. For the moment, there is no reason to compute best price index with the products
	// that are excluded. Perhaps that will be interesting when the option to select product in the excluded list will be available.
	bool bIncludeRejected = false;

	CSelectedPMBase *pclBestPriceIndexProduct = NULL;

	if( true == m_pclInputUser->IsPressurisationSystemExist() )
	{
		double dSolutionLowestPrice = DBL_MAX;

		if( false == bFirstPass )
		{
			if( m_arsBestPriceIndex[BestPriceIndexSolution::BPIS_ExpansionVessel].m_dLowestPrice < dSolutionLowestPrice )
			{
				dSolutionLowestPrice = m_arsBestPriceIndex[BestPriceIndexSolution::BPIS_ExpansionVessel].m_dLowestPrice;
			}

			if( m_arsBestPriceIndex[BestPriceIndexSolution::BPIS_ExpansionVesselMembrane].m_dLowestPrice < dSolutionLowestPrice )
			{
				dSolutionLowestPrice = m_arsBestPriceIndex[BestPriceIndexSolution::BPIS_ExpansionVesselMembrane].m_dLowestPrice;
			}
			
			if( m_arsBestPriceIndex[BestPriceIndexSolution::BPIS_Compresso].m_dLowestPrice < dSolutionLowestPrice )
			{
				dSolutionLowestPrice = m_arsBestPriceIndex[BestPriceIndexSolution::BPIS_Compresso].m_dLowestPrice;
			}
			
			if( m_arsBestPriceIndex[BestPriceIndexSolution::BPIS_Transfero].m_dLowestPrice < dSolutionLowestPrice )
			{
				dSolutionLowestPrice = m_arsBestPriceIndex[BestPriceIndexSolution::BPIS_Transfero].m_dLowestPrice;
			}
		}

		if( CPMInputUser::MT_All == m_pclInputUser->GetPressureMaintenanceType()
				|| CPMInputUser::MT_ExpansionVessel == m_pclInputUser->GetPressureMaintenanceType()
				|| CPMInputUser::MT_ExpansionVesselWithMembrane == m_pclInputUser->GetPressureMaintenanceType() )
		{
			// Check expansion vessel and expansion vessel with membrane.
			for( int i = 0; i < 2; i++ )
			{
				if( 0 == i && CPMInputUser::MT_ExpansionVesselWithMembrane == m_pclInputUser->GetPressureMaintenanceType() )
				{
					continue;
				}

				if( 1 == i && CPMInputUser::MT_ExpansionVessel == m_pclInputUser->GetPressureMaintenanceType() )
				{
					continue;
				}

				double dBestPriceIndex = DBL_MAX;

				ProductType eProductType = ( 0 == i ) ? PT_ExpansionVessel : PT_ExpansionVesselMembrane;
				BestPriceIndexSolution eBestPriceIndexSolution = ( 0 == i ) ? BPIS_ExpansionVessel : BPIS_ExpansionVesselMembrane;

				// Check vessel (Statico).
				if( true == IsProductExist( eProductType ) )
				{
					if( true == bFirstPass )
					{
						// Lowest price has been already set when sorting vessel selection.
						m_arsBestPriceIndex[eBestPriceIndexSolution].m_dLowestPrice = m_arsPriceData[eProductType].m_dLowestPrice;
					}
					else
					{
						double dBestPriceIndex = DBL_MAX;
						WhichList eWhichList = WL_Selected;
						bool fCanStop = false;

						while( false == fCanStop )
						{
							CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl*>( GetFirstProduct( eProductType, eWhichList ) );

							while( NULL != pclSelectedVessel )
							{
								CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel*>( pclSelectedVessel->GetProductIDPtr().MP );

								if( NULL == pclVessel )
								{
									pclSelectedVessel = dynamic_cast<CSelectedVssl*>( GetNextProduct( eProductType, eWhichList ) );
									continue;
								}

								double dPrice = m_pTADB->GetPrice( pclVessel->GetArtNum( true ) );

								if( dPrice > 0.0 )
								{
									double dVesselPrice = dPrice * pclSelectedVessel->GetNbreOfVsslNeeded();
									double dPriceIndex = dVesselPrice / dSolutionLowestPrice * 100.0;
									pclSelectedVessel->SetPriceIndex( dPriceIndex );

									// Best price index is used only for selected objects and not the rejected one.
									if( WL_Selected == eWhichList && dPriceIndex < dBestPriceIndex )
									{
										dBestPriceIndex = dPriceIndex;
										pclBestPriceIndexProduct = pclSelectedVessel;
									}
								}

								pclSelectedVessel = dynamic_cast<CSelectedVssl*>( GetNextProduct( eProductType, eWhichList ) );
							}

							if( true == bIncludeRejected && WL_Selected == eWhichList )
							{
								eWhichList = WL_Rejected;
							}
							else
							{
								fCanStop = true;
							}
						}

						m_arsBestPriceIndex[eBestPriceIndexSolution].m_pclSelectedPMBase = pclBestPriceIndexProduct;
						m_arsBestPriceIndex[eBestPriceIndexSolution].m_dBestPriceIndex = dBestPriceIndex;
					}
				}
			}
		}
		
		if( CPMInputUser::MT_All == m_pclInputUser->GetPressureMaintenanceType()
				|| CPMInputUser::MT_WithCompressor == m_pclInputUser->GetPressureMaintenanceType()
				|| CPMInputUser::MT_WithPump == m_pclInputUser->GetPressureMaintenanceType() )
		{
			// Check TecBox (Compresso and Transfero).
			for( int i = 0; i < 2; i++ )
			{
				if( 0 == i && CPMInputUser::MT_WithPump == m_pclInputUser->GetPressureMaintenanceType() )
				{
					continue;
				}

				if( 1 == i && CPMInputUser::MT_WithCompressor == m_pclInputUser->GetPressureMaintenanceType() )
				{
					continue;
				}

				double dBestPriceIndex = DBL_MAX;
				pclBestPriceIndexProduct = NULL;
				ProductType eTecBoxType = ( 0 == i ) ? PT_Compresso : PT_Transfero;
				ProductType eVesselType = ( 0 == i ) ? PT_CompressoVessel : PT_TransferoVessel;
				BestPriceIndexSolution eBPISolution = ( 0 == i ) ? BPIS_Compresso : BPIS_Transfero;
				WhichList eWhichList = WL_Selected;
				double dLowestPrice = DBL_MAX;
				bool fCanStop = false;

				while( false == fCanStop )
				{
					CSelectedPMBase *pclSelectedTecBox = GetFirstProduct( eTecBoxType, eWhichList );

					while( NULL != pclSelectedTecBox )
					{
						CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox*>( pclSelectedTecBox->GetProductIDPtr().MP );

						if( 0 == IDcmp( pclTecBox->GetFamilyID(), _T("TBXFAM_COMPR_CX") ) )
						{
							// Don't take this products for the best index computing.
							pclSelectedTecBox = GetNextProduct( eTecBoxType, eWhichList );
							continue;
						}

						double dTecBoxPrice = 0.0;

						if( NULL != pclTecBox && m_pTADB->GetPrice( pclTecBox->GetArtNum( true ) ) > 0.0 )
						{
							dTecBoxPrice = m_pTADB->GetPrice( pclTecBox->GetArtNum( true ) );
						}

						CSelectedVssl *pclSelectedPrimaryVessel = NULL;
						CDB_Vessel *pclPrimaryVessel = NULL;
						bool fIntegratedPrimaryVessel = pclTecBox->IsVariantIntegratedPrimaryVessel();
						bool fPrimaryVesselExist = false;
						bool fSecondaryVesselExist = false;
						double dPrimaryVesselPrice = 0.0;
						double dSecondaryVesselPrice = 0.0;

						int iNbrOfVessel = ( 0 == i ) ? SelectCompressoVessel( pclSelectedTecBox ) : SelectTransferoVessel( pclSelectedTecBox );

						if( false == fIntegratedPrimaryVessel )
						{
							if( 0 == iNbrOfVessel || NULL == m_arsPriceData[eVesselType].m_pclSelectedPMBase )
							{
								pclSelectedTecBox = GetNextProduct( eTecBoxType, eWhichList );
								continue;
							}

							// Take the primary vessel that has the lowest price.
							pclSelectedPrimaryVessel = dynamic_cast<CSelectedVssl*>( m_arsPriceData[eVesselType].m_pclSelectedPMBase );

							if( NULL == pclSelectedPrimaryVessel )
							{
								pclSelectedTecBox = GetNextProduct( eTecBoxType, eWhichList );
								continue;
							}

							pclPrimaryVessel = dynamic_cast<CDB_Vessel*>( pclSelectedPrimaryVessel->GetProductIDPtr().MP );

							if( NULL == pclPrimaryVessel )
							{
								pclSelectedTecBox = GetNextProduct( eTecBoxType, eWhichList );
								continue;
							}

							fPrimaryVesselExist = true;
							dPrimaryVesselPrice = m_pTADB->GetPrice( pclPrimaryVessel->GetArtNum( true ) );
						}
						else
						{
							// In this case we have only one solution.
							pclSelectedPrimaryVessel = dynamic_cast<CSelectedVssl*>( GetFirstProduct( ProductType::PT_CompressoVessel, WhichList::WL_Selected ) );

							if( NULL == pclSelectedPrimaryVessel )
							{
								pclSelectedTecBox = GetNextProduct( eTecBoxType, eWhichList );
								continue;
							}

							pclPrimaryVessel = dynamic_cast<CDB_Vessel*>( pclTecBox->GetIntegratedVesselIDPtr().MP );

							if( NULL == pclPrimaryVessel )
							{
								pclSelectedTecBox = GetNextProduct( eTecBoxType, eWhichList );
								continue;
							}

							// The primary vessel is integrated thus it exists.
							fPrimaryVesselExist = true;
						}

						if( pclSelectedPrimaryVessel->GetNbreOfVsslNeeded() > 1 )
						{
							CDB_Vessel *pclSecVessel = pclPrimaryVessel->GetAssociatedSecondaryVessel();

							if( NULL != pclSecVessel )
							{
								fSecondaryVesselExist = true;
								dSecondaryVesselPrice = m_pTADB->GetPrice( pclSecVessel->GetArtNum( true ) ) * ( pclSelectedPrimaryVessel->GetNbreOfVsslNeeded() - 1 );
							}
						}

						if( 0.0 == dTecBoxPrice || ( false == fIntegratedPrimaryVessel && true == fPrimaryVesselExist && 0.0 == dPrimaryVesselPrice ) ||
							( true == fSecondaryVesselExist && 0.0 == dSecondaryVesselPrice ) )
						{
							pclSelectedTecBox = GetNextProduct( eTecBoxType, eWhichList );
							continue;
						}

						double dTotalPrice = dTecBoxPrice + dPrimaryVesselPrice + dSecondaryVesselPrice;

						if( true == bFirstPass )
						{
							if( dTotalPrice < dLowestPrice )
							{
								dLowestPrice = dTotalPrice;
							}
						}
						else
						{
							double dPriceIndex = dTotalPrice / dSolutionLowestPrice * 100.0;
							pclSelectedTecBox->SetPriceIndex( dPriceIndex );

							// Best price index is used only for selected objects and not the rejected one.
							if( WL_Selected == eWhichList && dPriceIndex < dBestPriceIndex )
							{
								dBestPriceIndex = dPriceIndex;
								pclBestPriceIndexProduct = pclSelectedTecBox;
							}
						}

						pclSelectedTecBox = GetNextProduct( eTecBoxType, eWhichList );
					}

					if( true == bIncludeRejected && WL_Selected == eWhichList )
					{
						eWhichList = WL_Rejected;
					}
					else
					{
						fCanStop = true;
					}
				}

				if( true == bFirstPass )
				{
					m_arsBestPriceIndex[eBPISolution].m_dLowestPrice = dLowestPrice;
				}
				else
				{
					m_arsBestPriceIndex[eBPISolution].m_pclSelectedPMBase = pclBestPriceIndexProduct;
					m_arsBestPriceIndex[eBPISolution].m_dBestPriceIndex = dBestPriceIndex;
				}
			}
		}
	}
	else
	{
		// Pleno or/and Vento selected alone.

		if( true == bFirstPass )
		{
			return;
		}

		for( int i = 0; i < 3; i++ )
		{
			ProductType eProductType = ( 0 == i ) ? ProductType::PT_Pleno : ( 1 == i) ? ProductType::PT_Vento : ProductType::PT_Pleno_Protec;
			BestPriceIndexSolution eBPISolution = ( 0 == i || 2 == i ) ? BestPriceIndexSolution::BPIS_Pleno : BestPriceIndexSolution::BPIS_Vento;

			if( m_arsPriceData[eProductType].m_dLowestPrice <= 0.0 || NULL == m_arsPriceData[eProductType].m_pclSelectedPMBase )
			{
				continue;
			}
			
			double dBestPriceIndex = DBL_MAX;
			WhichList eWhichList = WL_Selected;		
			bool fCanStop = false;

			while( false == fCanStop )
			{
				CSelectedPMBase *pclSelectedTBPlenoVento = GetFirstProduct( eProductType, eWhichList );

				while( NULL != pclSelectedTBPlenoVento )
				{
					CDB_TBPlenoVento *pclTBPlenoVento = dynamic_cast<CDB_TBPlenoVento*>( pclSelectedTBPlenoVento->GetProductIDPtr().MP );
					double dPlenoPrice = 0.0;

					if( NULL == pclTBPlenoVento || m_pTADB->GetPrice( pclTBPlenoVento->GetArtNum( true ) ) <= 0.0 )
					{
						pclSelectedTBPlenoVento = GetNextProduct( eProductType, eWhichList );
						continue;
					}

					double dPriceIndex = m_pTADB->GetPrice( pclTBPlenoVento->GetArtNum( true ) ) / m_arsPriceData[eProductType].m_dLowestPrice * 100.0;
					pclSelectedTBPlenoVento->SetPriceIndex( dPriceIndex );

					// Best price index is used only for selected objects and not the rejected one.
					if( WL_Selected == eWhichList && dPriceIndex < dBestPriceIndex )
					{
						dBestPriceIndex = dPriceIndex;
						pclBestPriceIndexProduct = pclSelectedTBPlenoVento;
					}

					pclSelectedTBPlenoVento = GetNextProduct( eProductType, eWhichList );
				}

				if( true == bIncludeRejected && WL_Selected == eWhichList )
				{
					eWhichList = WL_Rejected;
				}
				else
				{
					fCanStop = true;
				}
			}

			m_arsBestPriceIndex[eBPISolution].m_pclSelectedPMBase = pclBestPriceIndexProduct;
			m_arsBestPriceIndex[eBPISolution].m_dLowestPrice = m_arsPriceData[eProductType].m_dLowestPrice;
			m_arsBestPriceIndex[eBPISolution].m_dBestPriceIndex = dBestPriceIndex;
		}
	}

	return;
}

int CSelectPMList::_GetTransferoTecBoxInSameGroup( CRankEx *pList, CDB_TecBox *pclTransferoReference )
{
	if( NULL == m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pList || NULL == pclTransferoReference )
	{
		return 0;
	}

	if( NULL == dynamic_cast<CDB_TBCurve*>( pclTransferoReference->GetPumpComprCurveIDPtr().MP ) )
	{
		return 0;
	}

	CDB_TBCurve* pclTBCurveReference = dynamic_cast<CDB_TBCurve*>( pclTransferoReference->GetPumpComprCurveIDPtr().MP );

	pList->PurgeAll();
	CTable *pTab = ( CTable * )( m_pTADB->Get( _T("TECBOX_TAB") ).MP );
	
	if( NULL == pTab )
	{
		return 0;
	}

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_TecBox *pCurrentTransfero = dynamic_cast<CDB_TecBox *>( idptr.MP );

		if( NULL == pCurrentTransfero )
		{
			continue;
		}

		if( 0 != CString( pCurrentTransfero->GetFamilyID() ).Compare( pclTransferoReference->GetFamilyID() ) )
		{
			continue;
		}

		if( pclTransferoReference->GetNbrPumpCompressor() != pCurrentTransfero->GetNbrPumpCompressor() )
		{
			continue;
		}

		if( pclTransferoReference->GetFunctions() != pCurrentTransfero->GetFunctions() )
		{
			continue;
		}

		if( pclTransferoReference->GetTecBoxVariant() != pCurrentTransfero->GetTecBoxVariant() )
		{
			continue;
		}

		// Special case for TI.
		if( CDB_TecBox::etbtTransferoTI == pclTransferoReference->GetTecBoxType() )
		{
			if( NULL == dynamic_cast<CDB_TBCurve*>( pCurrentTransfero->GetPumpComprCurveIDPtr().MP ) )
			{
				continue;
			}

			CDB_TBCurve* pclTBCurveCurrent = dynamic_cast<CDB_TBCurve*>( pCurrentTransfero->GetPumpComprCurveIDPtr().MP );
		
			// TI...0.2 as a VD range between 0-10000 l/h.
			// TI...1.2 as a VD range between 10000-20000 l/h
			// TI...2.2 as a VD range between 20000-30000 l/h
			// TI...3.2 as a VD range between 30000-..... l/h
			// We transform thus from m3/s in to l/h and divide by 10. This is why *360.0!
			int i1 = (int)( pclTBCurveReference->GetqNmax() * 360.0 );
			int i2 = (int)( pclTBCurveCurrent->GetqNmax() * 360.0 );
			
			if( (int)( pclTBCurveReference->GetqNmax() * 360.0 ) != (int)( pclTBCurveCurrent->GetqNmax() * 360.0 ) )
			{
				continue;
			}
		}

		pList->Add( pCurrentTransfero->GetName(), pCurrentTransfero->GetWeight(), ( LPARAM )( void * )pCurrentTransfero->GetIDPtr().MP );
	}

	return pList->GetCount();
}

int CSelectPMList::_GetCompressoCurveInSameGroup( CRankEx *pList, CDB_TecBox *pclCompressoReference )
{
	if( NULL == m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pList || NULL == pclCompressoReference )
	{
		return 0;
	}

	if( NULL == dynamic_cast<CDB_TBCurve *>( pclCompressoReference->GetPumpComprCurveIDPtr().MP ) )
	{
		return 0;
	}

	CDB_TBCurve *pclTBCurveReference = dynamic_cast<CDB_TBCurve *>( pclCompressoReference->GetPumpComprCurveIDPtr().MP );

	pList->PurgeAll();
	CTable *pTab = (CTable *)( m_pTADB->Get( _T("TECBOX_TAB") ).MP );
	
	if( NULL == pTab )
	{
		return 0;
	}

	CString strFamilyToCompare = pclCompressoReference->GetFamilyID();

	std::map<CString, CDB_TBCurve *> mapCompressoCurves;

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_TecBox *pCurrentCompresso = dynamic_cast<CDB_TecBox *>( idptr.MP );

		if( NULL == pCurrentCompresso || false == pCurrentCompresso->IsAvailable()
				|| CDB_TecBox::etbtCompresso != pCurrentCompresso->GetTecBoxType() )
		{
			continue;
		}

		CDB_TBCurve *pclTBCurveCurrent = dynamic_cast<CDB_TBCurve *>( pCurrentCompresso->GetPumpComprCurveIDPtr().MP );

		if( NULL == pclTBCurveCurrent )
		{
			continue;
		}

		if( strFamilyToCompare == CString( pCurrentCompresso->GetFamilyID() ) )
		{
			if( 0 == mapCompressoCurves.count( CString( pclTBCurveCurrent->GetIDPtr().ID ) ) )
			{
				mapCompressoCurves[CString( pclTBCurveCurrent->GetIDPtr().ID )] = pclTBCurveCurrent;
			}
		}
	}

	double dKey = 0.0;

	for( std::map<CString, CDB_TBCurve *>::iterator iter = mapCompressoCurves.begin(); iter != mapCompressoCurves.end(); ++iter )
	{
		pList->Add( iter->first.GetString(), dKey, (LPARAM)( iter->second ) );
		++dKey;
	}

	return pList->GetCount();
}
