#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "DlgHydroCalc.h"
#include "DlgHydroCalcBase.h"
#include "DlgHydroCalcRedDisk.h"
#include "DlgHydroCalcQKvDp.h"
#include "DlgHydroCalcQPDT.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC( CDlgHydroCalcQPDT, CDlgHydroCalcBase )

CDlgHydroCalcQPDT::CDlgHydroCalcQPDT( CDlgHydroCalc *pDlgHydroCalc )
	: CDlgHydroCalcBase( IDD_DLGHYDROCALCQPDT, pDlgHydroCalc )
{
}

BEGIN_MESSAGE_MAP( CDlgHydroCalcQPDT, CDlgHydroCalcBase )
	ON_CONTROL_RANGE( BN_CLICKED, IDC_HCQPDTRADIOFLOW, IDC_HCQPDTTRADIODT, &CDlgHydroCalcQPDT::OnBnClickedRadios )
END_MESSAGE_MAP()

void CDlgHydroCalcQPDT::OnChangeActiveTab( void )
{
	// Because each calling to 'SetWindowText' generates a 'ON_EN_CHANGE' message, we don't allow here the message
	// to be treated when it arrives in base class.
	m_bBlockEnHandlers = true;

	// First: set value with eventually some retrieved from other tabs.
	double dFlow = m_pDlgHydroCalc->GetFlowValue();
	m_EditFlow.SetWindowText( ( dFlow >= 0.0 ) ? WriteCUDouble( _U_FLOW, dFlow, false, 6, 6 ) : _T("") );

	double dPower = m_pDlgHydroCalc->GetPowerValue();
	m_EditPower.SetWindowText( ( dPower >= 0.0 ) ? WriteCUDouble( _U_TH_POWER, dPower, false, 6, 6 ) : _T("") );

	double dDT = m_pDlgHydroCalc->GetDTValue();
	m_EditDT.SetWindowText( ( dDT >= 0.0 ) ? WriteCUDouble( _U_DIFFTEMP, dDT, false, 6, 6 ) : _T("") );

	// Second: adapt value and fields in regards to previous tabs.
	int iPreviousTab = m_pDlgHydroCalc->GetPreviousTabID();
	int iPrevTabState = m_pDlgHydroCalc->GetRadioState( (CDlgHydroCalc::TabIDs)iPreviousTab );
	
	switch( iPreviousTab )
	{
		case CDlgHydroCalc::TabIDs::QKvDp:
		case CDlgHydroCalc::TabIDs::QRedDisk:

			if( CDlgHydroCalcQKvDp::RadioState::Kv == iPrevTabState || CDlgHydroCalcRedDisk::RadioState::Setting == iPrevTabState 
					|| ( ( CDlgHydroCalcQKvDp::RadioState::Flow == iPrevTabState || CDlgHydroCalcRedDisk::RadioState::Flow == iPrevTabState ) 
					&& m_pDlgHydroCalc->GetFlowValue() > 0.0 ) )
			{
				// Power reset.
				m_EditPower.SetWindowText( _T("") );
				m_pDlgHydroCalc->SetPowerValue( -1.0 );

				OnBnClickedRadios( IDC_HCQPDTRADIOPOWER );
			}
			else if( CDlgHydroCalcQKvDp::RadioState::Dp == iPrevTabState || CDlgHydroCalcRedDisk::RadioState::Dp == iPrevTabState )
			{
				// DT reset.
				m_EditDT.SetWindowText( _T("") );
				m_pDlgHydroCalc->SetDTValue( -1.0 );

				OnBnClickedRadios( IDC_HCQPDTRADIODT );
			}
			else
			{
				// Flow reset.
				m_EditFlow.SetWindowText( _T("") );
				m_pDlgHydroCalc->SetFlowValue( -1.0 );

				OnBnClickedRadios( IDC_HCQPDTRADIOFLOW );
			}
			break;

		case CDlgHydroCalc::TabIDs::Undefined:
		default:
			break;
	}

	Calculate();
	m_pDlgHydroCalc->SetPreviousTabID( CDlgHydroCalc::TabIDs::QPDT );
	m_bBlockEnHandlers = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgHydroCalcQPDT::DoDataExchange( CDataExchange *pDX )
{
	CDlgHydroCalcBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_EditPower );
	DDX_Control( pDX, IDC_EDITDT, m_EditDT );
	DDX_Control( pDX, IDC_HCQPDTRADIOFLOW, m_Radio1 );
	DDX_Control( pDX, IDC_HCQPDTRADIOPOWER, m_Radio2 );
	DDX_Control( pDX, IDC_HCQPDTRADIODT, m_Radio3 );
}

BOOL CDlgHydroCalcQPDT::OnInitDialog()
{
	// Do the default initialization.
	CDlgHydroCalcBase::OnInitDialog();

	// Initialization of static text.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCQPDT_FLOW );
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCQPDT_POWER );
	GetDlgItem( IDC_STATICPOWER )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALCQPDT_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );

	// Set the text for the static controls displaying units.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Set the text for the flow unit.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	SetDlgItemText( IDC_STATICFLOWUNIT, tcName );

	// Set the text for the power unit.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), tcName );
	SetDlgItemText( IDC_STATICPOWERUNIT, tcName );

	// Set the text for the DT unit.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), tcName );
	SetDlgItemText( IDC_STATICDTUNIT, tcName );

	return TRUE;
}

void CDlgHydroCalcQPDT::OnBnClickedRadios( UINT nID )
{
	switch( nID )
	{
		case IDC_HCQPDTRADIOFLOW:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICPOWEREQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICDTEQUAL )->ShowWindow( SW_HIDE );
			
			// Edit fields.
			m_EditFlow.EnableWindow( FALSE );
			m_EditPower.EnableWindow( TRUE );
			m_EditDT.EnableWindow( TRUE );

			// Set focus on power field.
			m_EditPower.SetFocus();

			CheckRadioButton( IDC_HCQPDTRADIOFLOW, IDC_HCQPDTRADIODT, IDC_HCQPDTRADIOFLOW );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio1.ModifyStyle( WS_TABSTOP, 0 );

			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QPDT, RadioState::Flow );
			break;

		case IDC_HCQPDTRADIOPOWER:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICPOWEREQUAL )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICDTEQUAL )->ShowWindow( SW_HIDE );

			// Edit fields.
			m_EditFlow.EnableWindow( TRUE );
			m_EditPower.EnableWindow( FALSE );
			m_EditDT.EnableWindow( TRUE );

			// Set focus on DT field.
			m_EditDT.SetFocus();

			CheckRadioButton( IDC_HCQPDTRADIOFLOW, IDC_HCQPDTRADIODT, IDC_HCQPDTRADIOPOWER );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio2.ModifyStyle( WS_TABSTOP, 0 );

			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QPDT, RadioState::Power );
			break;

		case IDC_HCQPDTRADIODT:
			// Equal signs.
			GetDlgItem( IDC_STATICFLOWEQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICPOWEREQUAL )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICDTEQUAL )->ShowWindow( SW_SHOW );

			// Edit fields.
			m_EditFlow.EnableWindow( TRUE );
			m_EditPower.EnableWindow( TRUE );
			m_EditDT.EnableWindow( FALSE );
			
			// Set focus on flow field.
			m_EditFlow.SetFocus();
			
			CheckRadioButton( IDC_HCQPDTRADIOFLOW, IDC_HCQPDTRADIODT, IDC_HCQPDTRADIODT );
			// Disable possibility to get tab goes to focus on radio button.
			m_Radio3.ModifyStyle( WS_TABSTOP, 0 );

			m_pDlgHydroCalc->SetRadioState( CDlgHydroCalc::TabIDs::QPDT, RadioState::DT );
			break;
	}
}
