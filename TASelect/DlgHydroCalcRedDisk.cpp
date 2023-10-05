#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"
#include "WinSysUtil.h"
#include "ExtComboBox.h"
#include "DlgHydroCalc.h"
#include "DlgHydroCalcBase.h"
#include "DlgHydroCalcQKvDp.h"
#include "DlgHydroCalcQPDT.h"
#include "DlgHydroCalcRedDisk.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC( CDlgHydroCalcRedDisk, CDlgHydroCalcBase )

CDlgHydroCalcRedDisk::CDlgHydroCalcRedDisk( CDlgHydroCalc *pDlgHydroCalc )
	: CDlgHydroCalcBase( IDD_DLGHYDROCALCREDDISK, pDlgHydroCalc )
{
	m_dPreviousKvs = -1.0;
	m_iPreviousRadio = -1;
	m_bSavePreviousRadioState = false;
}

BEGIN_MESSAGE_MAP( CDlgHydroCalcRedDisk, CDlgHydroCalcBase )
	ON_CONTROL_RANGE( BN_CLICKED, IDC_HCREDDISKRADIOFLOW, IDC_HCREDDISKRADIODP, &OnBnClickedRadios )
	ON_EN_CHANGE( IDC_EDITSETTING, &OnEnChangeEditSetting )
	ON_EN_SETFOCUS( IDC_EDITSETTING, OnSetFocusEditSetting )
	ON_BN_CLICKED( IDC_CHECKDELETEDVALVES, OnBnClickedDeletedValves )
	ON_CBN_SELCHANGE( IDC_COMBOVALVETYPE, OnSelChangeComboValveType )
	ON_CBN_SELCHANGE( IDC_COMBOVALVEFAMILY, OnSelChangeComboValveFamily )
	ON_CBN_SELCHANGE( IDC_COMBOVALVE, OnSelChangeComboValve )
END_MESSAGE_MAP()

void CDlgHydroCalcRedDisk::OnChangeActiveTab( void )
{
	// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
	// to be treated when it arrives in base class.
	m_bBlockEnHandlers = true;

	// First: set value with eventually some retrieved from other tabs.
	double dFlow = m_pDlgHydroCalc->GetFlowValue();
	m_EditFlow.SetWindowText( ( dFlow >= 0.0 ) ? WriteCUDouble( _U_FLOW, dFlow, false, 6, 6 ) : _T("") );

	double dSetting = m_pDlgHydroCalc->GetSettingValue();

	if( dSetting < 0.0 )
	{
		m_EditSetting.SetWindowText( _T("") );
	}
	else
	{
		CDB_TAProduct *pTAP = m_pDlgHydroCalc->GetValve();

		if( NULL != pTAP )
		{
			CDB_ValveCharacteristic *pValveChar = (CDB_ValveCharacteristic *)pTAP->GetValveCharDataPointer();

			if( NULL == pValveChar )
			{
				ASSERT_RETURN;
			}

			if( false == pTAP->IsKvSignalEquipped() )
			{
				m_EditSetting.SetWindowText( pValveChar->GetSettingString( dSetting ) );
			}
			else
			{
				// Valves with Kv signal equipped are for example the commissioning set, fixed orifice or venturi valve.
				m_EditSetting.SetWindowText( WriteDouble( dSetting, 6, 6 ) );
			}
		}
	}

	double dDp = m_pDlgHydroCalc->GetDpValue();
	m_EditDp.SetWindowText( ( dDp >= 0.0 ) ? WriteCUDouble( _U_DIFFPRESS, dDp, false, 6, 6 ) : _T("") );

	// Second: adapt value and fields in regards to previous tabs.
	int iPreviousTab = m_pDlgHydroCalc->GetPreviousTabID();
	int iPrevTabState = m_pDlgHydroCalc->GetRadioState( (CDlgHydroCalc::TabIDs)iPreviousTab );
	CDB_TAProduct *pValve = m_pDlgHydroCalc->GetValve();
	
	switch( iPreviousTab )
	{
		case CDlgHydroCalc::TabIDs::QPDT:

			if( false == pValve->IsKvSignalEquipped() 
					&& ( CDlgHydroCalcQPDT::RadioState::Power == iPrevTabState || 
						( CDlgHydroCalcQPDT::RadioState::Flow == iPrevTabState && m_pDlgHydroCalc->GetFlowValue() > 0.0 ) ) )
			{
				// Setting reset; Q kept as is; Dp recuperated if available.
				m_EditSetting.SetWindowText( _T("") );
				m_pDlgHydroCalc->SetSettingValue( 0.0 );

				OnBnClickedRadios( IDC_HCREDDISKRADIOSETTING );
			}
			else if( CDlgHydroCalcQPDT::RadioState::DT == iPrevTabState )
			{
				// Dp reset; Q kept as is; setting recuperated if available.
				if( false == pValve->IsKvSignalEquipped() )
				{
					m_EditDp.SetWindowText( _T("") );
					m_pDlgHydroCalc->SetDpValue( 0.0 );
				}

				OnBnClickedRadios( IDC_HCREDDISKRADIOSETTING );
			}
			else
			{
				// Q kept as is; Setting and Dp recuperated if available.
				OnBnClickedRadios( IDC_HCREDDISKRADIOFLOW );
			}
			break;

		case CDlgHydroCalc::TabIDs::QKvDp:
			if( false == pValve->IsKvSignalEquipped() &&
				(   CDlgHydroCalcQKvDp::RadioState::Kv == iPrevTabState || 
				  ( CDlgHydroCalcQKvDp::RadioState::Flow == iPrevTabState && m_pDlgHydroCalc->GetFlowValue() > 0.0 ) ||
				  ( CDlgHydroCalcQKvDp::RadioState::Dp == iPrevTabState && m_pDlgHydroCalc->GetDpValue() > 0.0 ) ) )

			{
				// Q & Dp kept as is; setting reset.
				m_EditSetting.SetWindowText( _T("") );
				m_pDlgHydroCalc->SetSettingValue( 0.0 );

				OnBnClickedRadios( IDC_HCREDDISKRADIOSETTING );
			}
			else if( CDlgHydroCalcQKvDp::RadioState::Dp == iPrevTabState )
			{
				// Q and Dp kept as is; setting recuperated if available.
				OnBnClickedRadios( IDC_HCREDDISKRADIODP );
			}
			else
			{
				// Q and Dp kept as is; setting recuperated if available.
				OnBnClickedRadios( IDC_HCREDDISKRADIOFLOW );
			}
			break;

		case CDlgHydroCalc::TabIDs::Undefined:
		default:
			break;
	}

	Calculate();
	m_pDlgHydroCalc->SetPreviousTabID( CDlgHydroCalc::TabIDs::QRedDisk );
	m_bBlockEnHandlers = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgHydroCalcRedDisk::DoDataExchange( CDataExchange *pDX )
{
	CDlgHydroCalcBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow );
	DDX_Control( pDX, IDC_EDITSETTING, m_EditSetting );
	DDX_Control( pDX, IDC_EDITDP, m_EditDp );
	DDX_Control( pDX, IDC_COMBOVALVETYPE, m_ComboValveType );
	DDX_Control( pDX, IDC_COMBOVALVEFAMILY, m_ComboValveFamily );
	DDX_Control( pDX, IDC_COMBOVALVE, m_ComboValve );
	DDX_Control( pDX, IDC_HCREDDISKRADIOFLOW, m_Radio1 );
	DDX_Control( pDX, IDC_HCREDDISKRADIOSETTING, m_Radio2 );
	DDX_Control( pDX, IDC_HCREDDISKRADIODP, m_Radio3 );
	DDX_Control( pDX, IDC_CHECKDELETEDVALVES, m_CBDeletedValves );
}

BOOL CDlgHydroCalcRedDisk::OnInitDialog()
{
	// Do the default initialization.
	CDlgHydroCalcBase::OnInitDialog();

	// Initialization of static text.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_FLOW );
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_SETTING );
	GetDlgItem( IDC_STATICSETTING )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_DP );
	GetDlgItem( IDC_STATICDP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_GROUPVALVE );
	GetDlgItem( IDC_GROUPVALVE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_VALVETYPE );
	GetDlgItem( IDC_STATICVALVETYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_VALVEFAMILY );
	GetDlgItem( IDC_STATICVALVEFAMILY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_VALVE );
	GetDlgItem( IDC_STATICVALVE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHGEN_DELETEDVALVES );
	GetDlgItem( IDC_CHECKDELETEDVALVES )->SetWindowText( str );
	// HYS-1398 : We retrieve the check box value, saved previously.
	int iCheck = AfxGetApp()->GetProfileInt( _T( "DlgHydroCalcRedDisk" ), _T( "ShowOldValves" ), BST_UNCHECKED );
	m_CBDeletedValves.SetCheck( iCheck );
	CDS_PersistData* pPD = TASApp.GetpTADS()->GetpPersistData();
	ASSERT( NULL != pPD );

	if( pPD != NULL )
	{
		// Save that deleted valves can be selected.
		if( BST_CHECKED == m_CBDeletedValves.GetCheck() )
		{
			pPD->SetOldValveUsed( true );
		}
		else
		{
			pPD->SetOldValveUsed( false );
		}
	}

	// Set the text for the static controls displaying units.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Set the text for the flow unit.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	SetDlgItemText( IDC_STATICFLOWUNIT, tcName );

	// Set the text for the Dp unit.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetDlgItemText( IDC_STATICDPUNIT, tcName );

	_FillComboValveType();
	OnSelChangeComboValveType();

	return TRUE;
}

void CDlgHydroCalcRedDisk::OnBnClickedRadios( UINT nID )
{
	switch( nID )
	{
		case IDC_HCREDDISKRADIOFLOW:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICSETTINGEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICDPEQUAL )->ShowWindow( SW_HIDE );

			// Edit fields.
			m_EditFlow.EnableWindow( FALSE );
			m_EditSetting.EnableWindow( TRUE );
			m_EditDp.EnableWindow( TRUE );

			// Set focus on setting field.
			m_EditSetting.SetFocus();

			CheckRadioButton( IDC_HCREDDISKRADIOFLOW, IDC_HCREDDISKRADIODP, IDC_HCREDDISKRADIOFLOW );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio1.ModifyStyle( WS_TABSTOP, 0 );

			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QRedDisk, RadioState::Flow );
			break;

		case IDC_HCREDDISKRADIOSETTING:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICSETTINGEQUAL )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICDPEQUAL )->ShowWindow( SW_HIDE );

			// Edit fields.
			m_EditFlow.EnableWindow( TRUE );
			m_EditSetting.EnableWindow( FALSE );
			m_EditDp.EnableWindow( TRUE );

			// Set focus on flow field.
			m_EditFlow.SetFocus();

			CheckRadioButton( IDC_HCREDDISKRADIOFLOW, IDC_HCREDDISKRADIODP, IDC_HCREDDISKRADIOSETTING );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio2.ModifyStyle( WS_TABSTOP, 0 );
			
			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QRedDisk, RadioState::Setting );
			break;

		case IDC_HCREDDISKRADIODP:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICSETTINGEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICDPEQUAL )->ShowWindow( SW_SHOW );

			// Edit fields.
			m_EditFlow.EnableWindow( TRUE );
			m_EditSetting.EnableWindow( TRUE );
			m_EditDp.EnableWindow( FALSE );

			// Set focus on flow field.
			m_EditFlow.SetFocus();

			CheckRadioButton( IDC_HCREDDISKRADIOFLOW, IDC_HCREDDISKRADIODP, IDC_HCREDDISKRADIODP );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio3.ModifyStyle( WS_TABSTOP, 0 );

			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QRedDisk, RadioState::Dp );
			break;
	}
}

void CDlgHydroCalcRedDisk::OnEnChangeEditSetting()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditSetting )
	{
		CDB_ValveCharacteristic *pValveChar = (CDB_ValveCharacteristic *)m_pDlgHydroCalc->GetValve()->GetValveCharDataPointer();

		if( NULL == pValveChar || false == pValveChar->HasKvCurve() )
		{
			ASSERT_RETURN;
		}

		double dSetting = 0.0;
		double dOpeningMax = pValveChar->GetOpeningMax();

		switch( ReadDouble( m_EditSetting, &dSetting ) )
		{
			case ReadDoubleReturn_enum::RD_OK:

				if( dOpeningMax > 0.0 && dSetting > dOpeningMax )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_SETTINGTOHIGH );
					m_pDlgHydroCalc->SetSettingValue( dOpeningMax );
					m_EditSetting.SetSel( 0, -1 );
					m_EditSetting.ReplaceSel( pValveChar->GetSettingString( dOpeningMax ) );
					m_EditSetting.SetFocus();
				}
				else if( dSetting < 0.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
					m_pDlgHydroCalc->SetSettingValue( 0.0 );
					m_EditSetting.SetSel( 0, -1 );
					m_EditSetting.ReplaceSel( pValveChar->GetSettingString( 0.0 ) );
					m_EditSetting.SetFocus();
				}
				else
				{
					m_pDlgHydroCalc->SetSettingValue( dSetting );
				}

				break;

			case ReadDoubleReturn_enum::RD_NOT_NUMBER:
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
				m_pDlgHydroCalc->SetSettingValue( 0.0 );
				m_EditSetting.SetSel( 0, -1 );
				m_EditSetting.ReplaceSel( pValveChar->GetSettingString( 0.0 ) );
				m_EditSetting.SetFocus();
				break;

			case ReadDoubleReturn_enum::RD_EMPTY:
				m_pDlgHydroCalc->SetSettingValue( -1.0 );
				break;
		}
		
		Calculate();
	}
}

void CDlgHydroCalcRedDisk::OnSetFocusEditSetting()
{
	m_EditSetting.SetSel( 0, -1 );
}

void CDlgHydroCalcRedDisk::OnSelChangeComboValveType()
{
	_FillComboValveFamily();
	OnSelChangeComboValveFamily();
}

void CDlgHydroCalcRedDisk::OnSelChangeComboValveFamily()
{
	// Save previous Kvs of the current valve.
	m_dPreviousKvs = m_pDlgHydroCalc->GetKvValue();

	_FillComboValve();

	// Set this variable to 'true' to force save previous radio button state.
	m_bSavePreviousRadioState = true;
	OnSelChangeComboValve();

	// When we change type or family, we try to find valve that has Kvs nearest above the previous one.
	m_bSavePreviousRadioState = false;
	_FindBestValve();
}

void CDlgHydroCalcRedDisk::OnSelChangeComboValve()
{
	IDPTR ValveIDPtr = m_ComboValve.GetCBCurSelIDPtr();

	if( NULL == ValveIDPtr.MP )
	{
		return;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( ValveIDPtr.MP );

	if( NULL == pTAP )
	{
		return;
	}

	// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
	// to be treated when it arrives in base class.
	m_bBlockEnHandlers = true;
	m_pDlgHydroCalc->SetValve( pTAP );

	CDB_ValveCharacteristic *pValveChar = (CDB_ValveCharacteristic *)pTAP->GetValveCharDataPointer();

	if( NULL == pValveChar )
	{
		ASSERT_RETURN;
	}

	if( false == pTAP->IsKvSignalEquipped() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_SETTING );
		
		double dOpeningMax = pValveChar->GetOpeningMax();

		if( -1.0 != dOpeningMax )
		{
			str += CString( _T(" (") ) + TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_MAX );
			str += CString( _T(" ") ) + pValveChar->GetSettingString( dOpeningMax ) + _T(")");
		}

		GetDlgItem( IDC_STATICSETTING )->SetWindowText( str );
		GetDlgItem( IDC_STATICSETTING )->ShowWindow( SW_SHOW );

		double dCurrentOpening = m_pDlgHydroCalc->GetSettingValue();

		if( dCurrentOpening > dOpeningMax )
		{
			dCurrentOpening = dOpeningMax;
		}

		str = pValveChar->GetSettingString( dCurrentOpening );
		
		m_EditSetting.SetWindowText( str );
		m_EditSetting.ShowWindow( TRUE );
		m_pDlgHydroCalc->SetSettingValue( dCurrentOpening );
				
		// Reset to be sure.
		GetDlgItem( IDC_HCREDDISKRADIOSETTING )->ShowWindow( SW_SHOW );

		// Hide Kv signal
		GetDlgItem( IDC_STATICKVSIGNAL )->ShowWindow( SW_HIDE );

		if( m_iPreviousRadio != -1 )
		{
			OnBnClickedRadios( m_iPreviousRadio );
			m_iPreviousRadio = -1;
		}
	}
	else
	{
		CString str;
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

		if( false == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_KVSIGNAL );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_CVSIGNAL );
		}

		str += CString( _T(" ") ) + WriteCUDouble( _C_KVCVCOEFF, pTAP->GetKvSignal(), false, 6, 6 );

		// Show Kv signal static text.
		GetDlgItem( IDC_STATICKVSIGNAL )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICKVSIGNAL )->SetWindowText( str );
		
		// Clear text in edit field to be sure.
		m_EditSetting.EnableWindow( FALSE );
		m_EditSetting.ShowWindow( FALSE );

		// Reset static setting text to be sure and hide it.
		str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_SETTING );
		GetDlgItem( IDC_STATICSETTING )->SetWindowText( str );
		GetDlgItem( IDC_STATICSETTING )->ShowWindow( SW_HIDE );
		
		// There is no more reason to show setting radio button and edit.
		GetDlgItem( IDC_HCREDDISKRADIOSETTING )->ShowWindow( SW_HIDE );

		// Memorize what was the current radio. Because when we coming back to a family valve that is not equipped with Kv signal,
		// we need to reset back the right radio button!
		// But not when call comes from '_FindBestValve' (see comments in the header file).
		if( true == m_bSavePreviousRadioState )
			m_iPreviousRadio = GetCheckedRadioButton( IDC_HCREDDISKRADIOFLOW, IDC_HCREDDISKRADIODP );

		// If previous radio button was 'Dp', we must enable same radio button.
		// If previous one was 'Setting', we are not able anymore to edit opening because now it's a fixed KvSignal. Thus we enable flow radio button.
		// And finally, if previous radio button was flow, we enable flow.
		OnBnClickedRadios( ( IDC_HCREDDISKRADIODP == m_iPreviousRadio ) ? IDC_HCREDDISKRADIODP : IDC_HCREDDISKRADIOFLOW );
	}

	Calculate();
	m_bBlockEnHandlers = false;
}

void CDlgHydroCalcRedDisk::OnBnClickedDeletedValves()
{
	CDS_PersistData* pPD = TASApp.GetpTADS()->GetpPersistData();
	ASSERT( NULL != pPD );

	if( pPD != NULL )
	{
		// Save that deleted valves can be selected.
		if( BST_CHECKED == m_CBDeletedValves.GetCheck() )
		{
			pPD->SetOldValveUsed( true );
		}
		else
		{
			pPD->SetOldValveUsed( false );
		}
	}
	// Save check box value
	AfxGetApp()->WriteProfileInt( _T( "DlgHydroCalcRedDisk" ), _T( "ShowOldValves" ), m_CBDeletedValves.GetCheck() );
	
	// Fill combo to add or remove deleted products
	_FillComboValveType();
	OnSelChangeComboValveType();
}

void CDlgHydroCalcRedDisk::CalculateRedDisk( void )
{
	CDB_TAProduct *pTAP = m_pDlgHydroCalc->GetValve();

	if( NULL == pTAP )
	{
		return;
	}
	
	CDB_ValveCharacteristic *pValveChar = (CDB_ValveCharacteristic *)pTAP->GetValveCharDataPointer();

	if( NULL == pValveChar || false == pValveChar->HasKvCurve() )
	{
		ASSERT_RETURN;
	}

	double dRho = m_pDlgHydroCalc->GetWaterChar().GetDens();
	double dNu = m_pDlgHydroCalc->GetWaterChar().GetKinVisc();
	
	switch( GetCheckedRadioButton( IDC_HCREDDISKRADIOFLOW, IDC_HCREDDISKRADIODP ) )
	{
		case IDC_HCREDDISKRADIOFLOW:
		{
			double dFlow = -1.0;
			double dOpening = m_pDlgHydroCalc->GetSettingValue();
			double dDp = m_pDlgHydroCalc->GetDpValue();
			
			if( false == pTAP->IsKvSignalEquipped() && dOpening > 0.0 && dDp >= 0.0 )
			{
				dOpening = min( pValveChar->GetOpeningMax(), dOpening );
				dOpening = max( 0 , dOpening );
				pValveChar->GetValveQ( &dFlow, dDp, dOpening, dRho, dNu );
			}
			else if( dDp >= 0.0 )
			{
				double dKvs = pTAP->GetKvSignal();

				if( dKvs >= 0.0 )
				{
					dFlow = CalcqT( dKvs, dDp, dRho );
				}
			}

			m_EditFlow.SetWindowText( ( dFlow >= 0.0 ) ? WriteCUDouble( _U_FLOW, dFlow, false, 6, 6 ) : _T("") );
			m_pDlgHydroCalc->SetFlowValue( dFlow );
			break;
		}
	
		case IDC_HCREDDISKRADIOSETTING:
		{
			double dFlow = m_pDlgHydroCalc->GetFlowValue();
			double dDp = m_pDlgHydroCalc->GetDpValue();
			double dOpening =-1.0;

			if( dDp > 0.0 && dFlow >= 0.0 )
			{
				bool bReturnValue = pValveChar->GetValveOpening( dFlow, dDp, &dOpening, dRho, dNu, ( eBool3::eb3True == pValveChar->IsMultiTurn() ) ? 0 : 1 );
			
				// If error...
				if( false == bReturnValue )
				{
					// Change Dp static text to display max dp at full opening.
					CString str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_DP );

					double dDpFullOpening = pValveChar->GetDpFullOpening( dFlow, dRho, dNu );

					if( -1.0 != dDpFullOpening )
					{
						str += CString( _T(" (") ) + TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_DPMAX );
						str += CString( _T(" ") ) + WriteCUDouble( _U_DIFFPRESS, dDpFullOpening, true ) + _T(")");
					}

					GetDlgItem( IDC_STATICDP )->SetWindowText( str );

					m_EditSetting.SetWindowText( GetDashDotDash() );
				}
				else
				{
					// Reset Dp static text to be sure.
					CString str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCREDDISK_DP );
					GetDlgItem( IDC_STATICDP )->SetWindowText( str );
					str = pValveChar->GetSettingString( dOpening );
					m_EditSetting.SetWindowText( str );
				}
			}
			else
			{
				m_EditSetting.SetWindowText( _T("") );
			}

			m_pDlgHydroCalc->SetSettingValue( dOpening );
			break;
		}

		case IDC_HCREDDISKRADIODP:
		{
			double dFlow = m_pDlgHydroCalc->GetFlowValue();
			double dOpening = m_pDlgHydroCalc->GetSettingValue();
			double dDp = -1.0;

			if( false == pTAP->IsKvSignalEquipped() )
			{
				pValveChar->GetValveDp( dFlow, &dDp, dOpening, dRho, dNu );
			}
			else
			{
				double dKv = pTAP->GetKvSignal();

				if( dKv > 0.0 )
				{
					dDp = CalcDp( dFlow, dKv, dRho );
				}
			}

			m_EditDp.SetWindowText( ( dDp >= 0.0 ) ? WriteCUDouble( _U_DIFFPRESS, dDp, false, 6, 6 ) : _T("") );
			m_pDlgHydroCalc->SetDpValue( dDp );
			break;
		}
	
		default:
			break;
	}

	// Save Kvs.
	double dOpening = m_pDlgHydroCalc->GetSettingValue();
	double dKvs = ( false == pTAP->IsKvSignalEquipped() && -1.0 != dOpening ) ? pValveChar->GetKv( dOpening ) : pTAP->GetKvSignal();

	if( -1.0 != dKvs  )
	{
		m_pDlgHydroCalc->SetKvValue( dKvs );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgHydroCalcRedDisk::_FillComboValveType( CString strValveTypeID )
{
	CRankEx ValveTypeList;
	TASApp.GetpTADB()->GetBVTypeList( &ValveTypeList );
	// HYS-1467 : Add presettable trv, presettable trv insert and BCV
	TASApp.GetpTADB()->GetTaCVTypeList( &ValveTypeList, CTADatabase::eForTrv, CDB_ControlProperties::LastCV2W3W, CDB_ControlProperties::Presettable );
	TASApp.GetpTADB()->GetTrvInsertTypeList( &ValveTypeList, _T(""), CDB_ControlProperties::Presettable );
	TASApp.GetpTADB()->GetTaCVTypeList( &ValveTypeList, CTADatabase::eForBCv );
	m_ComboValveType.FillInCombo( &ValveTypeList, strValveTypeID, 0 );
}

void CDlgHydroCalcRedDisk::_FillComboValveFamily( CString strValveFamilyID )
{
	CRankEx ValveFamilyList;
	// HYS-1467
	if( 0 == _tcscmp( m_ComboValveType.GetCBCurSelIDPtr().ID, _T( "TRVTYPE_PRESET" ) ) )
	{
		TASApp.GetpTADB()->GetTrvFamilyList( &ValveFamilyList, m_ComboValveType.GetCBCurSelIDPtr().ID, CTADatabase::NoFiltering );
	}
	else if( 0 == _tcscmp( m_ComboValveType.GetCBCurSelIDPtr().ID, _T( "TRVTYPE_INSERT_PRESET" ) ) )
	{
	    // Add filter selecion to filter old_installation family
		TASApp.GetpTADB()->GetTrvInsertFamilyList( &ValveFamilyList, CTADatabase::FilterSelection::ForIndAndBatchSel );
	}
	else if( 0 == _tcscmp( m_ComboValveType.GetCBCurSelIDPtr().ID, _T( "RVTYPE_BVC" ) ) )
	{
		TASApp.GetpTADB()->GetTaCVFamList( &ValveFamilyList, CTADatabase::eForBCv, CDB_ControlProperties::LastCV2W3W, m_ComboValveType.GetCBCurSelIDPtr().ID );
	}
	else if( true == m_ComboValveType.GetCBCurSelIDPtr().MP->FindOwner(_T("RVTYPE_TAB" )))
	{
		TASApp.GetpTADB()->GetBVFamilyList( &ValveFamilyList,m_ComboValveType.GetCBCurSelIDPtr().ID, CTADatabase::NoFiltering, 0, INT_MAX, false, true );
	}
	m_ComboValveFamily.FillInCombo( &ValveFamilyList, strValveFamilyID, 0, true );
}

void CDlgHydroCalcRedDisk::_FillComboValve( CString strValveID )
{
	CRankEx TempValveList;
	// HYS-1467
	if( 0 == _tcscmp( m_ComboValveType.GetCBCurSelIDPtr().ID, _T( "TRVTYPE_PRESET" ) ) )
	{
		TASApp.GetpTADB()->GetTrvList( &TempValveList, m_ComboValveType.GetCBCurSelIDPtr().ID, m_ComboValveFamily.GetCBCurSelIDPtr().ID,
			_T( "" ), _T( "" ), _T( "" ) );
	}
	else if( 0 == _tcscmp( m_ComboValveType.GetCBCurSelIDPtr().ID, _T( "TRVTYPE_INSERT_PRESET" ) ) )
	{
		TASApp.GetpTADB()->GetTrvInsertList( &TempValveList, m_ComboValveType.GetCBCurSelIDPtr().ID, m_ComboValveFamily.GetCBCurSelIDPtr().ID,
			_T( "" ), _T( "" ), _T( "" ) );
	}
	else if( 0 == _tcscmp( m_ComboValveType.GetCBCurSelIDPtr().ID, _T( "RVTYPE_BVC" ) ) )
	{
		TASApp.GetpTADB()->GetTaCVList( &TempValveList, CTADatabase::eForBCv, false, CDB_ControlProperties::LastCV2W3W, m_ComboValveType.GetCBCurSelIDPtr().ID,
			m_ComboValveFamily.GetCBCurSelIDPtr().ID, _T( "" ), _T( "" ), _T( "" ) );
	}
	else if( true == m_ComboValveType.GetCBCurSelIDPtr().MP->FindOwner( _T( "RVTYPE_TAB" ) ) )
	{
		TASApp.GetpTADB()->GetBVList( &TempValveList, m_ComboValveType.GetCBCurSelIDPtr().ID, m_ComboValveFamily.GetCBCurSelIDPtr().ID,
			_T( "" ), _T( "" ), _T( "" ), CTADatabase::NoFiltering, 0, INT_MAX, NULL, false, true );
	}
	
	// Loop on list to take only valves that have different characteristic.
	_string str;
	LPARAM itemdata;
	CRankEx ValveList;
	CDB_TAProduct *pMatchTAP = NULL;

	for( BOOL bContinue = TempValveList.GetFirst( str, itemdata ); TRUE == bContinue; bContinue = TempValveList.GetNext( str, itemdata ) )
	{
		CDB_TAProduct *pTAP = (CDB_TAProduct *)itemdata;
		ASSERT( NULL != pTAP );

		if( true == TASApp.GetpTADB()->CheckIfCharactAlreadyExist( &ValveList, pTAP, pMatchTAP ) )
		{
			CData *pValveChar = pTAP->GetValveCharDataPointer();

			if( NULL != pValveChar )
			{
				ValveList.Add( pTAP->GetName(), (double)pTAP->GetSortingKey(), (LPARAM)( pTAP->GetIDPtr().MP ) );
				pMatchTAP = pTAP;
			}
		}
	}
	
	TempValveList.PurgeAll();
	m_ComboValve.FillInCombo( &ValveList, strValveID, 0, true );
}

void CDlgHydroCalcRedDisk::_FindBestValve( void )
{
	if( m_dPreviousKvs != -1.0 )
	{
		int iValveCount = m_ComboValve.GetCount();

		if( iValveCount > 0 )
		{
			bool bFound = false;
			double dKvsBestAbove = -1.0;
			double dKvsBestBelow = -1.0;
			int iBestAboveIndex = -1;
			int iBestBelowIndex = -1;

			for( int iLoopValve = 0; iLoopValve < iValveCount; iLoopValve++ )
			{
				CDB_TAProduct *pTAProduct = dynamic_cast<CDB_TAProduct *>( (CData *)m_ComboValve.GetItemData( iLoopValve ) );

				if( NULL == pTAProduct )
				{
					continue;
				}

				double dKvs = -1.0;
				bool bGetKvs = true;

				if( false == pTAProduct->IsKvSignalEquipped() )
				{
					double dOpening = m_pDlgHydroCalc->GetSettingValue();
					CDB_ValveCharacteristic *pValveCharacteristic = pTAProduct->GetValveCharacteristic();

					if( NULL != pValveCharacteristic && dOpening != -1.0 )
					{
						dKvs = pValveCharacteristic->GetKv( dOpening );
						bGetKvs = false;
					}
				}

				if( true == bGetKvs )
				{
					dKvs = pTAProduct->GetKvSignal();
				}

				if( -1.0 == dKvs )
				{
					continue;
				}

				if(  dKvs > m_dPreviousKvs && ( -1 == dKvsBestAbove || dKvs < dKvsBestAbove ) )
				{
					dKvsBestAbove = dKvs;
					iBestAboveIndex = iLoopValve;
				}
				else if( dKvs < m_dPreviousKvs && ( -1 == dKvsBestBelow || dKvs > dKvsBestBelow ) )
				{
					dKvsBestBelow = dKvs;
					iBestBelowIndex = iLoopValve;
				}
			}

			double dBestKv =-1.0;
			int iBestIndex = -1;

			if( dKvsBestAbove != -1 )
			{
				dBestKv = dKvsBestAbove;
				iBestIndex = iBestAboveIndex;
			}
			else if( dKvsBestBelow != -1 )
			{
				dBestKv = dKvsBestBelow;
				iBestIndex = iBestBelowIndex;
			}

			// If found...
			if( iBestIndex != -1.0 )
			{
				m_ComboValve.SetCurSel( iBestIndex );
				OnSelChangeComboValve();
			}
		}
	}
}
