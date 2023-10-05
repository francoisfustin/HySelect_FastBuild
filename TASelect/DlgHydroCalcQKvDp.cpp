#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "DlgHydroCalc.h"
#include "DlgHydroCalcBase.h"
#include "DlgHydroCalcQPDT.h"
#include "DlgHydroCalcRedDisk.h"
#include "DlgHydroCalcQKvDp.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC( CDlgHydroCalcQKvDp, CDlgHydroCalcBase )

CDlgHydroCalcQKvDp::CDlgHydroCalcQKvDp( CDlgHydroCalc *pDlgHydroCalc )
	: CDlgHydroCalcBase( IDD_DLGHYDROCALCQKVDP, pDlgHydroCalc )
{
}

BEGIN_MESSAGE_MAP( CDlgHydroCalcQKvDp, CDlgHydroCalcBase )
	ON_CONTROL_RANGE( BN_CLICKED, IDC_HCQKVDPRADIOFLOW, IDC_HCQKVDPRADIODP, &CDlgHydroCalcQKvDp::OnBnClickedRadios )
END_MESSAGE_MAP()

void CDlgHydroCalcQKvDp::OnChangeActiveTab( void )
{
	// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
	// to be treated when it arrives in base class.
	m_bBlockEnHandlers = true;

	// First: set value with eventually some retrieved from other tabs.
	double dFlow = m_pDlgHydroCalc->GetFlowValue();
	m_EditFlow.SetWindowText( ( dFlow >= 0.0 ) ? WriteCUDouble( _U_FLOW, dFlow, false, 6, 6 ) : _T("") );

	double dKv = m_pDlgHydroCalc->GetKvValue();
	m_EditKv.SetWindowText( ( dKv >= 0.0 ) ? WriteCUDouble( _C_KVCVCOEFF, dKv, false, 6, 6 ) : _T("") );

	double dDp = m_pDlgHydroCalc->GetDpValue();
	m_EditDp.SetWindowText( ( dDp >= 0.0 ) ? WriteCUDouble( _U_DIFFPRESS, dDp, false, 6, 6 ) : _T("") );

	// Second: adapt value and fields in regards to previous tabs.
	int iPreviousTab = m_pDlgHydroCalc->GetPreviousTabID();
	int iPrevRadioState = m_pDlgHydroCalc->GetRadioState( (CDlgHydroCalc::TabIDs)iPreviousTab );
	
	switch( iPreviousTab )
	{
		case CDlgHydroCalc::TabIDs::QPDT:
			if( CDlgHydroCalcQPDT::RadioState::Power == iPrevRadioState || 
				( CDlgHydroCalcQPDT::RadioState::Flow == iPrevRadioState && m_pDlgHydroCalc->GetFlowValue() > 0.0 ) )
			{
				// Kv reset; Q kept as is; Dp recuperated if available.
				m_EditKv.SetWindowText( _T("") );
				m_pDlgHydroCalc->SetKvValue( -1.0 );

				OnBnClickedRadios( IDC_HCQKVDPRADIOKV );
			}
			else if( CDlgHydroCalcQPDT::RadioState::DT == iPrevRadioState )
			{
				// Dp reset; Q kept as is; Kv recuperated if available.
				m_EditDp.SetWindowText( _T("") );
				m_pDlgHydroCalc->SetDpValue( -1.0 );

				OnBnClickedRadios( IDC_HCQKVDPRADIODP );
			}
			else
			{
				// Q kept as is; Kv and Dp recuperated if available.
				OnBnClickedRadios( IDC_HCQKVDPRADIOFLOW );
			}
			break;

		case CDlgHydroCalc::TabIDs::QRedDisk:
			if( CDlgHydroCalcRedDisk::RadioState::Setting == iPrevRadioState || 
				( CDlgHydroCalcRedDisk::RadioState::Flow == iPrevRadioState && m_pDlgHydroCalc->GetFlowValue() > 0.0 ) ||
				( CDlgHydroCalcRedDisk::RadioState::Dp == iPrevRadioState && m_pDlgHydroCalc->GetDpValue() > 0.0 ) )
			{
				// Q & Dp kept as is; Kv reset.
				m_EditKv.SetWindowText( _T("") );
				m_pDlgHydroCalc->SetKvValue( -1.0 );

				OnBnClickedRadios( IDC_HCQKVDPRADIOKV );
			}
			else if( CDlgHydroCalcRedDisk::RadioState::Dp == iPrevRadioState )
			{
				// Q and Dp kept as is; Kv recuperated if available.
				OnBnClickedRadios( IDC_HCQKVDPRADIODP );
			}
			else
			{
				// Q and Dp kept as is; Kv recuperated if available.
				OnBnClickedRadios( IDC_HCQKVDPRADIOFLOW );
			}
			break;

		case CDlgHydroCalc::TabIDs::Undefined:
			OnBnClickedRadios( IDC_HCQKVDPRADIOFLOW );
			break;

		default:
			break;
	}

	Calculate();
	m_pDlgHydroCalc->SetPreviousTabID( CDlgHydroCalc::TabIDs::QKvDp );
	m_bBlockEnHandlers = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgHydroCalcQKvDp::DoDataExchange( CDataExchange *pDX )
{
	CDlgHydroCalcBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow );
	DDX_Control( pDX, IDC_EDITKV, m_EditKv );
	DDX_Control( pDX, IDC_EDITDP, m_EditDp );
	DDX_Control( pDX, IDC_HCQKVDPRADIOFLOW, m_Radio1 );
	DDX_Control( pDX, IDC_HCQKVDPRADIOKV, m_Radio2 );
	DDX_Control( pDX, IDC_HCQKVDPRADIODP, m_Radio3 );
}

BOOL CDlgHydroCalcQKvDp::OnInitDialog()
{
	// Do the default initialization.
	CDialogEx::OnInitDialog();

	// Initialization of static text.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCQKVDP_FLOW );
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCQKVDP_KV );
	GetDlgItem( IDC_STATICKV )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCQKVDP_DP );
	GetDlgItem( IDC_STATICDP )->SetWindowText( str );

	// Set the text for the static controls displaying units.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Set the text for the flow unit.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	SetDlgItemText( IDC_STATICFLOWUNIT, tcName );

	// Set the text for the Kv unit.
	GetNameOf( pUnitDB->GetDefaultUnit( _C_KVCVCOEFF ), tcName );
	SetDlgItemText( IDC_STATICKVUNIT, tcName );

	// Set the text for the Dp unit.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetDlgItemText( IDC_STATICDPUNIT, tcName );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgHydroCalcQKvDp::OnBnClickedRadios( UINT nID )
{
	switch( nID )
	{
		case IDC_HCQKVDPRADIOFLOW:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICKVEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICDPEQUAL )->ShowWindow( SW_HIDE );

			// Edit fields.
			m_EditFlow.EnableWindow( FALSE );
			m_EditKv.EnableWindow( TRUE );
			m_EditDp.EnableWindow( TRUE );

			// Set focus on Kv field.
			m_EditKv.SetFocus();

			CheckRadioButton( IDC_HCQKVDPRADIOFLOW, IDC_HCQKVDPRADIODP, IDC_HCQKVDPRADIOFLOW );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio1.ModifyStyle( WS_TABSTOP, 0 );

			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QKvDp, RadioState::Flow );
			break;

		case IDC_HCQKVDPRADIOKV:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICKVEQUAL )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICDPEQUAL )->ShowWindow( SW_HIDE );

			// Edit fields.
			m_EditFlow.EnableWindow( TRUE );
			m_EditKv.EnableWindow( FALSE );
			m_EditDp.EnableWindow( TRUE );
			
			// Set focus on flow field.
			m_EditFlow.SetFocus();

			CheckRadioButton( IDC_HCQKVDPRADIOFLOW, IDC_HCQKVDPRADIODP, IDC_HCQKVDPRADIOKV );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio2.ModifyStyle( WS_TABSTOP, 0 );

			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QKvDp, RadioState::Kv );
			break;

		case IDC_HCQKVDPRADIODP:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICKVEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICDPEQUAL )->ShowWindow( SW_SHOW );

			// Edit fields.
			m_EditFlow.EnableWindow( TRUE );
			m_EditKv.EnableWindow( TRUE );
			m_EditDp.EnableWindow( FALSE );
			
			// Set focus on flow field.
			m_EditFlow.SetFocus();

			CheckRadioButton( IDC_HCQKVDPRADIOFLOW, IDC_HCQKVDPRADIODP, IDC_HCQKVDPRADIODP );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio3.ModifyStyle( WS_TABSTOP, 0 );

			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QKvDp, RadioState::Dp );
			break;
	}
}
