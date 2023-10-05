#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "hydronic.h"
#include "ExtComboBox.h"
#include "DlgHydroCalc.h"
#include "DlgHydroCalcBase.h"
#include "DlgHydroCalcQKvDp.h"
#include "DlgHydroCalcQPDT.h"
#include "DlgHydroCalcRedDisk.h"


BEGIN_MESSAGE_MAP( CDlgHydroCalcBase::CMyRadioButton, CButton )
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()
void CDlgHydroCalcBase::CMyRadioButton::OnSetFocus( CWnd* pOldWnd ) {}

BEGIN_MESSAGE_MAP( CDlgHydroCalcBase::CMyEdit, CEdit )
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()
void CDlgHydroCalcBase::CMyEdit::OnLButtonDown( UINT nFlags, CPoint point )
{
	CEdit::OnLButtonDown( nFlags, point );
	SetSel( 0, -1 );
}

IMPLEMENT_DYNAMIC( CDlgHydroCalcBase, CDialogEx )

CDlgHydroCalcBase::CDlgHydroCalcBase( UINT nIDTemplate, CDlgHydroCalc *pDlgHydroCalc )
	: CDialogEx( nIDTemplate )
{
	m_pDlgHydroCalc = pDlgHydroCalc;
	m_bBlockEnHandlers = false;
}

void CDlgHydroCalcBase::Calculate( void )
{
	switch( (unsigned int)m_lpszTemplateName )
	{
		case IDD_DLGHYDROCALCQKVDP:
			CalculateQKvDp();
			break;

		case IDD_DLGHYDROCALCQPDT:
			CalculateQPowerDT();
			break;

		case IDD_DLGHYDROCALCREDDISK:
			CalculateRedDisk();
			break;

		default:
			break;
	}
}

BEGIN_MESSAGE_MAP( CDlgHydroCalcBase, CDialogEx )
	ON_EN_CHANGE( IDC_EDITFLOW, &OnEnChangeEditFlow )
	ON_EN_CHANGE( IDC_EDITKV, &OnEnChangeEditKv )
	ON_EN_CHANGE( IDC_EDITDP, &OnEnChangeEditDp )
	ON_EN_CHANGE( IDC_EDITPOWER, &OnEnChangeEditPower )
	ON_EN_CHANGE( IDC_EDITDT, &OnEnChangeEditDT )
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgHydroCalcBase::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

void CDlgHydroCalcBase::OnEnChangeEditFlow()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditFlow )
	{
		double dFlow = 0.0;
		ReadDoubleReturn_enum eReturn = ReadCUDouble( _U_FLOW, m_EditFlow, &dFlow );

		if( true == ValidateInput( eReturn, dFlow, m_EditFlow ) )
		{
			m_pDlgHydroCalc->SetFlowValue( dFlow );
			Calculate();
		}
	}
}

void CDlgHydroCalcBase::OnEnChangeEditKv()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditKv )
	{
		double dKv = 0.0;
		ReadDoubleReturn_enum eReturn = ReadCUDouble( _C_KVCVCOEFF, m_EditKv, &dKv );

		if( true == ValidateInput( eReturn, dKv, m_EditKv ) )
		{
			m_pDlgHydroCalc->SetKvValue( dKv );
			Calculate();
		}
	}
}

void CDlgHydroCalcBase::OnEnChangeEditDp()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditDp )
	{
		double dDp = 0.0;
		ReadDoubleReturn_enum eReturn = ReadCUDouble( _U_DIFFPRESS, m_EditDp, &dDp );

		if( true == ValidateInput( eReturn, dDp, m_EditDp ) )
		{
			m_pDlgHydroCalc->SetDpValue( dDp );
			Calculate();
		}
	}
}

void CDlgHydroCalcBase::OnEnChangeEditPower()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditPower )
	{
		double dPower = 0.0;
		ReadDoubleReturn_enum eReturn = ReadCUDouble( _U_TH_POWER, m_EditPower, &dPower );

		if( true == ValidateInput( eReturn, dPower, m_EditPower ) )
		{
			m_pDlgHydroCalc->SetPowerValue( dPower );
			Calculate();
		}
	}
}

void CDlgHydroCalcBase::OnEnChangeEditDT()
{
	if( false == m_bBlockEnHandlers && GetFocus() == &m_EditDT )
	{
		double dDT = 0.0;
		ReadDoubleReturn_enum eReturn = ReadCUDouble( _U_DIFFTEMP, m_EditDT, &dDT );

		if( true == ValidateInput( eReturn, dDT, m_EditDT ) )
		{
			m_pDlgHydroCalc->SetDTValue( dDT );
			Calculate();
		}
	}
}

void CDlgHydroCalcBase::OnOK()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ENTER] key).
}

void CDlgHydroCalcBase::OnCancel()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ESCAPE] key).
	m_pDlgHydroCalc->OnCancelFromMFCTab();
}

void CDlgHydroCalcBase::CalculateQKvDp( void )
{
	double dRho = m_pDlgHydroCalc->GetWaterChar().GetDens();

	switch( m_pDlgHydroCalc->GetRadioState( CDlgHydroCalc::TabIDs::QKvDp ) )
	{
		case CDlgHydroCalcQKvDp::RadioState::Flow:
		{
			double dKv = m_pDlgHydroCalc->GetKvValue();
			double dDp = m_pDlgHydroCalc->GetDpValue();
			double dFlow = -1.0;

			if( dKv >= 0.0 && dDp >= 0.0 )
			{
				dFlow = CalcqT( dKv, dDp, dRho );
			}

			m_EditFlow.SetWindowText( ( dFlow >= 0.0 ) ? WriteCUDouble( _U_FLOW, dFlow, false, 6, 6 ) : _T("") );
			m_pDlgHydroCalc->SetFlowValue( dFlow );
			break;
		}
	
		case CDlgHydroCalcQKvDp::RadioState::Kv:
		{
			double dFlow = m_pDlgHydroCalc->GetFlowValue();
			double dDp = m_pDlgHydroCalc->GetDpValue();
			double dKv = -1.0;

			if( dFlow > 0.0 && dDp > 0.0 )
			{
				dKv = CalcKv( dFlow, dDp, dRho );
			}

			m_EditKv.SetWindowText( ( dKv >= 0.0 ) ? WriteCUDouble( _C_KVCVCOEFF, dKv, false, 6, 6 ) : _T("") );
			m_pDlgHydroCalc->SetKvValue( dKv );
			break;
		}

		case CDlgHydroCalcQKvDp::RadioState::Dp:
		{
			double dFlow = m_pDlgHydroCalc->GetFlowValue();
			double dKv = m_pDlgHydroCalc->GetKvValue();
			double dDp = -1.0;

			if( dFlow >= 0.0 && dKv > 0.0 )
			{
				dDp = CalcDp( dFlow, dKv, dRho );
			}

			m_EditDp.SetWindowText( ( dDp >= 0.0 ) ? WriteCUDouble( _U_DIFFPRESS, dDp, false, 6, 6 ) : _T("") );
			m_pDlgHydroCalc->SetDpValue( dDp );
			break;
		}
	
		default:
			break;
	}
}

void CDlgHydroCalcBase::CalculateQPowerDT( void )
{
	double dRho = m_pDlgHydroCalc->GetWaterChar().GetDens();
	double dCp = m_pDlgHydroCalc->GetWaterChar().GetSpecifHeat();

	switch( m_pDlgHydroCalc->GetRadioState( CDlgHydroCalc::TabIDs::QPDT ) )
	{
		case CDlgHydroCalcQPDT::RadioState::Flow:
		{
			double dPower = m_pDlgHydroCalc->GetPowerValue();
			double dDT = m_pDlgHydroCalc->GetDTValue();
			double dFlow = -1.0;

			if( dPower >= 0.0 && dDT > 0.0 )
			{
				dFlow = CalcqFromPDT( dPower, dDT, dRho, dCp );
			}

			m_EditFlow.SetWindowText( ( dFlow >= 0.0 ) ? WriteCUDouble( _U_FLOW, dFlow, false, 6, 6 ) : _T("") );
			m_pDlgHydroCalc->SetFlowValue( dFlow );
			break;
		}
	
		case CDlgHydroCalcQPDT::RadioState::Power:
		{
			double dFlow = m_pDlgHydroCalc->GetFlowValue();
			double dDT = m_pDlgHydroCalc->GetDTValue();
			double dPower = -1.0;

			if( dFlow > 0.0 && dDT > 0.0 )
			{
				dPower = CalcP( dFlow, dDT, dRho, dCp );
			}

			m_EditPower.SetWindowText( ( dPower >= 0.0 ) ? WriteCUDouble( _U_TH_POWER, dPower, false, 6, 6 ) : _T("") );
			m_pDlgHydroCalc->SetPowerValue( dPower );
			break;
		}

		case CDlgHydroCalcQPDT::RadioState::DT:
		{
			double dFlow = m_pDlgHydroCalc->GetFlowValue();
			double dPower = m_pDlgHydroCalc->GetPowerValue();
			double dDT = -1.0;

			if( dFlow > 0.0 && dPower >= 0.0 )
			{
				dDT = CalcDT( dPower, dFlow, dRho, dCp );
			}

			m_EditDT.SetWindowText( ( dDT >= 0.0 ) ? WriteCUDouble( _U_DIFFTEMP, dDT, false, 6, 6 ) : _T("") );
			m_pDlgHydroCalc->SetDTValue( dDT );
			break;
		}
	
		default:
			break;
	}
}

bool CDlgHydroCalcBase::ValidateInput( ReadDoubleReturn_enum eReturn, double& dValue, CEdit& clEditField )
{
	bool bCorrect = false;

	switch( eReturn )
	{
		case ReadDoubleReturn_enum::RD_OK:

			if( dValue < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				clEditField.SetFocus();
			}
			else
			{
				bCorrect = true;
			}

			break;

		case ReadDoubleReturn_enum::RD_NOT_NUMBER:
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
			clEditField.SetFocus();
			break;

		case ReadDoubleReturn_enum::RD_EMPTY:
			dValue = -1.0;
			// Return 'true' to launch calculate to reset other fields.
			bCorrect = true;
			break;

		default:
			break;
	}

	return bCorrect;
}
