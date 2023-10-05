#include "stdafx.h"
#include <errno.h>

#include "Taselect.h"
#include "MainFrm.h"
#include "Hydronic.h"

#include "RViewSSelSS.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"

CDlgIndSelBase::CDlgIndSelBase( CIndividualSelectionParameters &clIndSelParam, UINT nID, CWnd *pParent )
	: CDlgSelectionBase( clIndSelParam, nID, pParent )
{
	m_pclIndSelParams = &clIndSelParam;

	m_iRadioFlowPowerDT = Radio_Flow;
	m_fMustFocusDT = false;
	m_bAutoCheckboxSetManaging = false;
	m_bAtLeastOneNo = false;
	m_bAtLeastOneYes = false;
	m_bAtLeastOneYesBreakOk = false;
	m_bAtLeastOneYesBreakOnMsg = false;
	m_EditFieldHandlerBlocked = false;
	m_bFlowEditing = false;
	m_bPowerDTEditing = false;
}

CDlgIndSelBase::~CDlgIndSelBase()
{
	if( NULL != GetLinkedRightViewSSel() )
	{
		GetLinkedRightViewSSel()->UnregisterNotificationHandler( this );
	}
}

void CDlgIndSelBase::VerifyModificationMode()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	// Object still exist?
	m_pclIndSelParams->m_SelIDPtr = m_pclIndSelParams->m_pTADS->Get( m_pclIndSelParams->m_SelIDPtr.ID );

	if( NULL == m_pclIndSelParams->m_SelIDPtr.MP )
	{
		if( NULL != GetpBtnSelect() )
		{
			GetpBtnSelect()->SetWindowText( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_BUTTONSELECT ) );
			GetpBtnSelect()->EnableWindow( FALSE );
		}

		if( NULL != GetpBtnCancelModif() )
		{
			GetpBtnCancelModif()->ShowWindow( SW_HIDE );
		}

		m_pclIndSelParams->m_bEditModeRunning = false;
	}
	else
	{
		if( NULL != GetpBtnSelect() )
		{
			GetpBtnSelect()->SetWindowText( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_APPLYMODIFICATION ) );
		}

		if( NULL != GetpBtnCancelModif() )
		{
			GetpBtnCancelModif()->ShowWindow( SW_SHOW );
			GetpBtnCancelModif()->EnableWindow( TRUE );
			GetpBtnCancelModif()->SetWindowText( TASApp.LoadLocalizedString( IDS_CANCEL ) );
		}

		m_pclIndSelParams->m_bEditModeRunning = true;
	}
}

void CDlgIndSelBase::EnableSelectButton( bool fEnabled )
{
	if( NULL != GetpBtnSelect()->GetSafeHwnd() )
	{
		GetpBtnSelect()->EnableWindow( ( true == fEnabled ) ? TRUE : FALSE );
	}
}

void CDlgIndSelBase::UpdateDpFieldValue()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	// If there is no Dp edit field.
	if( NULL == GetpEditDp() )
	{
		return;
	}

	if( true == m_pclIndSelParams->m_bDpEnabled )
	{
		if( m_pclIndSelParams->m_dDp > 0.0 )
		{
			GetpEditDp()->SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_pclIndSelParams->m_dDp ) );
		}
		else
		{
			GetpEditDp()->SetWindowText( _T("") );
		}
	}
	else
	{
		GetpEditDp()->SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
	}
}

void CDlgIndSelBase::UpdateFlowFieldValue()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	// If there is no flow edit field.
	if( NULL == GetpEditFlow() )
	{
		return;
	}

	if( m_pclIndSelParams->m_dFlow > 0.0 )
	{
		GetpEditFlow()->SetWindowText( WriteCUDouble( _U_FLOW,  m_pclIndSelParams->m_dFlow ) );
	}
	else
	{
		GetpEditFlow()->SetWindowText( _T("") );
	}
}

void CDlgIndSelBase::UpdatePowerFieldValue()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	// If there is no power edit field.
	if( NULL == GetpEditPower() )
	{
		return;
	}

	if( m_pclIndSelParams->m_dPower > 0.0 )
	{
		GetpEditPower()->SetWindowText( WriteCUDouble( _U_TH_POWER, m_pclIndSelParams->m_dPower ) );
	}
	else
	{
		GetpEditPower()->SetWindowText( _T("") );
	}
}

void CDlgIndSelBase::UpdateDTFieldValue()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	// If there is no DT edit field.
	if( NULL == GetpEditDT() )
	{
		return;
	}

	if( m_pclIndSelParams->m_dDT > 0.0 )
	{
		GetpEditDT()->SetWindowText( WriteCUDouble( _U_DIFFTEMP, m_pclIndSelParams->m_dDT ) );
	}
	else
	{
		GetpEditDT()->SetWindowText( _T("") );
	}
}

void CDlgIndSelBase::UpdateFlowOrPowerDTState()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	// 'TRUE' to fill 'm_iRadioFlowPowerDT' variable with the radio state in the dialog.
	UpdateData( TRUE );

	// To not be disturbed by all the 'OnEnChange' handlers.
	m_EditFieldHandlerBlocked = true;

	if( Radio_Flow == m_iRadioFlowPowerDT )
	{
		// Input by the flow.
		GetpEditPower()->SetWindowText( _T("") );
		GetpEditDT()->SetWindowText( _T("") );
		GetpEditPower()->EnableWindow( FALSE );
		GetpEditDT()->EnableWindow( FALSE );
		
		if( NULL != GetDlgItem( IDC_STATICPUNIT ) )
		{
			GetDlgItem( IDC_STATICPUNIT )->EnableWindow( FALSE );
		}

		if( NULL != GetDlgItem( IDC_STATICDTUNIT ) )
		{
			GetDlgItem( IDC_STATICDTUNIT )->EnableWindow( FALSE );
		}

		GetpEditFlow()->EnableWindow( TRUE );
		GetpEditFlow()->SetFocus();

		if( NULL != GetDlgItem( IDC_STATICQUNIT ) )
		{
			GetDlgItem( IDC_STATICQUNIT )->EnableWindow( TRUE );
		}

		m_pclIndSelParams->m_eFlowOrPowerDTMode = CDS_SelProd::efdFlow;
	}
	else
	{
		// Input by the power and DT.
		m_pclIndSelParams->m_dFlow = 0.0;
		UpdateFlowFieldValue();
		GetpEditFlow()->EnableWindow( FALSE );

		if( NULL != GetDlgItem( IDC_STATICQUNIT ) )
		{
			GetDlgItem( IDC_STATICQUNIT )->EnableWindow( FALSE );
		}

		GetpEditPower()->EnableWindow( TRUE );
		GetpEditDT()->EnableWindow( TRUE );

		if( false == m_fMustFocusDT )
		{
			GetpEditPower()->SetFocus();
		}
		else
		{
			GetpEditDT()->SetFocus();
		}

		if( NULL != GetDlgItem( IDC_STATICPUNIT ) )
		{
			GetDlgItem( IDC_STATICPUNIT )->EnableWindow( TRUE );
		}

		if( NULL != GetDlgItem( IDC_STATICDTUNIT ) )
		{
			GetDlgItem( IDC_STATICDTUNIT )->EnableWindow( TRUE );
		}

		m_fMustFocusDT = false;
		m_pclIndSelParams->m_eFlowOrPowerDTMode = CDS_SelProd::efdPower;
	}

	m_EditFieldHandlerBlocked = false;
}

void CDlgIndSelBase::UpdateDpFieldState()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	if( true == m_pclIndSelParams->m_bDpEnabled )
	{
		if( NULL != GetpEditDp() )
		{
			( (CEdit *)GetpEditDp() )->SetReadOnly( FALSE );
			GetpEditDp()->EnableWindow( TRUE );
			GetpEditDp()->SetFocus();
			GetpEditDp()->SetWindowText( _T("") );
		}

		if( NULL != GetDlgItem( IDC_STATICDPUNIT ) )
		{
			GetDlgItem( IDC_STATICDPUNIT )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICDPUNIT )->EnableWindow( TRUE );
		}

		if( NULL != GetpGroupDp() )
		{
			GetpGroupDp()->SetForceShowDisable( false );
		}
	}
	else
	{
		if( NULL != GetpEditDp() )
		{
			( (CEdit *)GetpEditDp() )->SetReadOnly( TRUE );
			GetpEditDp()->EnableWindow( FALSE );
			GetpEditDp()->SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
		}

		if( NULL != GetDlgItem( IDC_STATICDPUNIT ) )
		{
			GetDlgItem( IDC_STATICDPUNIT )->EnableWindow( FALSE );
		}

		if( NULL != GetpGroupDp() )
		{
			GetpGroupDp()->SetForceShowDisable( true );
		}

		m_pclIndSelParams->m_dDp = 0.0;
	}
}

bool CDlgIndSelBase::IsButtonSelectEnabled( void )
{
	if( NULL == GetpBtnSelect() )
	{
		return false;
	}

	if( FALSE == GetpBtnSelect()->IsWindowEnabled() )
	{
		return false;
	}

	return true;
}

void CDlgIndSelBase::ClearCommonInputEditors()
{
	if( NULL != GetpEditFlow() )
	{
		GetpEditFlow()->SetWindowText( _T("") );
		OnEnKillFocusFlow();
	}

	if( NULL != GetpEditDp() )
	{
		GetpEditDp()->SetWindowText( _T("") );
		OnEnKillFocusDp();
	}

	if( NULL != GetpEditPower() )
	{
		GetpEditPower()->SetWindowText( _T("") );
		OnEnKillFocusPower();
	}

	if( NULL != GetpEditDT() )
	{
		GetpEditDT()->SetWindowText( _T("") );
		OnEnKillFocusDT();
	}
}

int CDlgIndSelBase::VerifyDpValue( void )
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERTA_RETURN( VDPFlag_DpError );
	}

	int iReturn = VDPFlag_OK;

	if( true == m_pclIndSelParams->m_bDpEnabled )
	{
		// If Dp value not valid...
		if( m_pclIndSelParams->m_dDp <= 0.0 )
		{
			// Ask to user if he wants continue selection without Dp.
			if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_NODP_DPCHECKED, MB_YESNO | MB_ICONQUESTION ) )
			{
				// Disable Dp field.
				// Remark: don't call 'OnBnClickedCheckdp()' because this one internally calls 'ClearAll' and it is too much!
				m_pclIndSelParams->m_bDpEnabled = false;

				// We set the flag 'VDPFlag_DpFieldToDisable' to tell to caller to disable the Dp field.
				iReturn = VDPFlag_DpFieldToDisable;
				UpdateDpFieldState();
			}
			else
			{
				iReturn = VDPFlag_DpError;
			}
		}
	}
	else
	{
		// HYS-1081: to be sure to not keep the previous value set in the 'Select.cpp'.
		m_pclIndSelParams->m_dDp = 0.0;
	}

	return iReturn;
}

void CDlgIndSelBase::SaveSelectionParameters( void )
{
	if( NULL == m_pclIndSelParams || NULL == m_pclIndSelParams->m_pTADS || NULL == m_pclIndSelParams->m_pTADS->GetpIndSelParameter() )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::SaveSelectionParameters();

	CDS_IndSelParameter *pclIndSelParameters = m_pclIndSelParams->m_pTADS->GetpIndSelParameter();
	pclIndSelParameters->SetPipeSeriesID( m_pclIndSelParams->m_strPipeSeriesID );
	pclIndSelParameters->SetPipeSizeID( m_pclIndSelParams->m_strPipeID );
}

void CDlgIndSelBase::ActivateLeftTabDialog( void )
{
	if( NULL == m_pclIndSelParams || NULL == m_pclIndSelParams->m_pTADS || NULL == m_pclIndSelParams->m_pTADS->GetpIndSelParameter() )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::ActivateLeftTabDialog();

	// Reinitialize pipes accordingly to the current context.
	CDS_IndSelParameter *pclCDSIndSelParameters = m_pclIndSelParams->m_pTADS->GetpIndSelParameter();
	m_pclIndSelParams->m_strPipeSeriesID = pclCDSIndSelParameters->GetPipeSeriesID( m_pclIndSelParams );
	m_pclIndSelParams->m_strPipeID = pclCDSIndSelParameters->GetPipeSizeID();
}

void CDlgIndSelBase::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgSelectionBase::ChangeApplicationType( eProductSelectionApplicationType );

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	_UpdateFlowEdit();
}

void CDlgIndSelBase::OnRViewSSelSelectProduct( void )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	OnBnClickedSelect();
}

void CDlgIndSelBase::OnRViewSSelSelectionChanged( bool bSelectionActive )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	EnableSelectButton( bSelectionActive );
}

void CDlgIndSelBase::OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	switch( iKeyboardVirtualKey )
	{
		case VK_RETURN:
			if( true == IsButtonSelectEnabled() )
			{
				PostWMCommandToControl( GetpBtnSelect() );
			}
			else
			{
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			break;

		case VK_ESCAPE:
			// Set the focus on the first available edit control.
			SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
			break;
	}
}

void CDlgIndSelBase::ClearAll( void )
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::ClearAll();

	// Disable the selection button.
	if( false == m_pclIndSelParams->m_bEditModeRunning )
	{
		EnableSelectButton( false );
	}
}

bool CDlgIndSelBase::ResetRightView()
{
	bool bReturn = false;
	CRViewSSelSS *pCurrentRightView = GetLinkedRightViewSSel();

	if( NULL != pCurrentRightView )
	{

		if( false == pCurrentRightView->IsEmpty() )
		{
			pCurrentRightView->Reset();
		}

		pCurrentRightView->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );
		bReturn = true;
	}

	return bReturn;
}

CMainFrame::RightViewList CDlgIndSelBase::GetRViewID()
{
	CMainFrame::RightViewList eRViewID = CMainFrame::eUndefined;
	CRViewSSelSS *pCurrentRightView = GetLinkedRightViewSSel();

	if( NULL != pCurrentRightView )
	{
		eRViewID = pCurrentRightView->GetRViewID();
	}

	return eRViewID;
}

bool CDlgIndSelBase::LeftTabKillFocus( bool fNext )
{
	bool fReturn = false;

	// TAB -> go to the top control only if right view is empty.
	if(  NULL != GetLinkedRightViewSSel() && TRUE == GetLinkedRightViewSSel()->IsWindowVisible() && false == GetLinkedRightViewSSel()->IsEmpty() )
	{
		// Set the focus on the right view.
		GetLinkedRightViewSSel()->SetFocus();
		
		// Set focus on the appropriate group in the right view.
		GetLinkedRightViewSSel()->SetFocusW( fNext );
		
		fReturn = true;
	}

	return fReturn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelBase, CDlgSelectionBase )
	ON_BN_CLICKED( IDC_BUTTONSUGGEST, OnBnClickedSuggest )
	ON_BN_CLICKED( IDC_BUTTONSELECT, OnBnClickedSelect )
	ON_BN_CLICKED( IDC_BUTTONCANCEL, OnBnClickedCancel )

	ON_BN_CLICKED( IDC_RADIOFLOW, OnBnClickedRadioFlowOrPowerDT )
	ON_BN_CLICKED( IDC_RADIOPOWER, OnBnClickedRadioFlowOrPowerDT )

	ON_EN_CHANGE( IDC_EDITFLOW, OnEnChangeFlow )
	ON_EN_CHANGE( IDC_EDITPOWER, OnEnChangePower )
	ON_EN_CHANGE( IDC_EDITDT, OnEnChangeDT )
	ON_EN_CHANGE( IDC_EDITDP, OnEnChangeDp )

	ON_EN_KILLFOCUS( IDC_EDITFLOW, OnEnKillFocusFlow )
	ON_EN_KILLFOCUS( IDC_EDITPOWER, OnEnKillFocusPower )
	ON_EN_KILLFOCUS( IDC_EDITDT, OnEnKillFocusDT )
	ON_EN_KILLFOCUS( IDC_EDITDP, OnEnKillFocusDp )

	ON_EN_SETFOCUS( IDC_EDITFLOW, OnEnSetFocusFlow )
	ON_EN_SETFOCUS( IDC_EDITPOWER, OnEnSetFocusPower )
	ON_EN_SETFOCUS( IDC_EDITDT, OnEnSetFocusDT )

	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITFLOW, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITPOWER, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDT, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDP, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_COMBOKVS, OnEditEnterChar )
END_MESSAGE_MAP()

BOOL CDlgIndSelBase::OnInitDialog()
{
	CDlgSelectionBase::OnInitDialog();

	CString str;

	if( NULL != GetpEditFlow() )
	{
		if( NULL != GetpRadioFlow() )
		{
			str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_FLOW );
			GetpRadioFlow()->SetWindowText( str );
		}

		GetpEditFlow()->SetEditType( CNumString::eDouble, CNumString::ePositive );
		GetpEditFlow()->SetPhysicalType( _U_FLOW );
	}

	if( NULL != GetpEditDp() )
	{
		GetpEditDp()->SetPhysicalType( _U_DIFFPRESS );
		GetpEditDp()->SetEditType( CNumString::eDouble, CNumString::ePositive );
	}

	if( NULL != GetpEditPower() )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_POWER );

		if( NULL != GetpRadioPower() )
		{
			GetpRadioPower()->SetWindowText( str );
		}

		GetpEditPower()->SetPhysicalType( _U_TH_POWER );
		GetpEditPower()->SetEditType( CNumString::eDouble, CNumString::ePositive );
	}

	if( NULL != GetpEditDT() )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_DT );

		if( NULL != GetpStaticDT() )
		{
			GetpStaticDT()->SetWindowText( str );
		}

		GetpEditDT()->SetPhysicalType( _U_DIFFTEMP );
		GetpEditDT()->SetEditType( CNumString::eDouble, CNumString::ePositive );
	}

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
	// 	m_clGroupQ.SetExpandCollapseMode( true );
	// 	m_clGroupQ.SetNotificationHandler( this );
	// 	m_clGroupDp.SetExpandCollapseMode( true );
	// 	m_clGroupDp.SetNotificationHandler( this );

	return TRUE;
}

LRESULT CDlgIndSelBase::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	CDlgSelectionBase::OnNewDocument( wParam, lParam );

	m_pclIndSelParams->m_SelIDPtr = _NULL_IDPTR;
	return 0;
}

void CDlgIndSelBase::OnBnClickedCancel()
{
	// User cancel edition of an individual selection.
	SetModificationMode( false );
	PostWMCommandToControl( GetpBtnSuggest() );
}

void CDlgIndSelBase::OnBnClickedRadioFlowOrPowerDT()
{
	UpdateFlowOrPowerDTState();
	ClearAll();
}

void CDlgIndSelBase::OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( NULL == GetpBtnSuggest() || ( WS_DISABLED == ( WS_DISABLED & GetpBtnSuggest()->GetStyle() ) ) )
	{
		return;
	}

	if( IDC_EDITFLOW == pNMHDR->idFrom || IDC_EDITDP == pNMHDR->idFrom || IDC_EDITPOWER == pNMHDR->idFrom || IDC_EDITDT == pNMHDR->idFrom 
			|| IDC_COMBOKVS == pNMHDR->idFrom )
	{
		// To force the current edit to lost focus (And then allow it to save value input by user).
		GetpBtnSuggest()->SetFocus();

		// Reset Focus on the modified Edit.
		CWnd::FromHandle( pNMHDR->hwndFrom )->SetFocus();

		PostWMCommandToControl( GetpBtnSuggest() );
	}
}

LRESULT CDlgIndSelBase::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	CDlgSelectionBase::OnPipeChange( wParam, lParam );

	if( WMUserPipeChange::WM_UPC_ForProductSelection != ( WMUserPipeChange )wParam )
	{
		return 0;
	}

	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return 0;
	}

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	return 0;
}

LRESULT CDlgIndSelBase::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	return 0;
}

LRESULT CDlgIndSelBase::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Set the text for statics
	// Use SetDlgItemText in place of GetDlgItem()->SetWindowText because it's based on window message and don't care if the control doesn't exist
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), name );
	SetDlgItemText( IDC_STATICQUNIT, name );

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATICDPUNIT, name );

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), name );
	SetDlgItemText( IDC_STATICDTUNIT, name );

	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), name );
	SetDlgItemText( IDC_STATICPUNIT, name );

	// Reset values in the edit fields.
	// Remarks: block all handlers (ON_EN_CHANGE, ON_EN_KILLFOCUS and ON_EN_SETFOCUS) to avoid interaction with operation.
	m_EditFieldHandlerBlocked = true;
	UpdateDpFieldValue();
	UpdateFlowFieldValue();
	UpdateDTFieldValue();
	UpdatePowerFieldValue();
	m_EditFieldHandlerBlocked = false;

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	return 0;
}

LRESULT CDlgIndSelBase::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( ( WMUserWaterCharWParam )wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) )
	{
		return 0;
	}

	CDlgSelectionBase::OnWaterChange( wParam, lParam );

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	_UpdateFlowEdit();

	return 0;
}

void CDlgIndSelBase::OnEnChangeFlow()
{
	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	// Allow to clear the right view only on the first user change in the flow field.
	if( false == m_bFlowEditing )
	{
		m_bFlowEditing = true;
		ResetRightView();
	}

	EnableSelectButton( false );
}

void CDlgIndSelBase::OnEnChangePower()
{
	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	// Allow to clear the right view only on first user action in power field.
	if( false == m_bPowerDTEditing )
	{
		m_bPowerDTEditing = true;
		ResetRightView();
	}

	_UpdateFlowEdit();
}

void CDlgIndSelBase::OnEnChangeDT()
{
	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	// Allow to clear the right view only on first user action in power field.
	if( false == m_bPowerDTEditing )
	{
		m_bPowerDTEditing = true;
		ResetRightView();
	}

	_UpdateFlowEdit();
}

void CDlgIndSelBase::OnEnKillFocusFlow()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	// Variable
	double dFlow;

	if( true == _ReadFlow( dFlow ) )
	{
		m_pclIndSelParams->m_dFlow = dFlow;
	}
	else
	{
		m_pclIndSelParams->m_dFlow = 0.0;
	}

	m_bFlowEditing = false;
}

void CDlgIndSelBase::OnEnKillFocusPower()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	double dPower;

	if( true == _ReadPower( dPower ) )
	{
		m_pclIndSelParams->m_dPower = dPower;
	}
	else
	{
		m_pclIndSelParams->m_dPower = 0.0;
	}

	m_bPowerDTEditing = false;
}

void CDlgIndSelBase::OnEnKillFocusDT()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	double dDT;

	if( true == _ReadDT( dDT ) )
	{
		m_pclIndSelParams->m_dDT = dDT;
	}
	else
	{
		m_pclIndSelParams->m_dDT = 0.0;
	}

	m_bPowerDTEditing = false;
}

void CDlgIndSelBase::OnEnKillFocusDp()
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	double dDp;

	if( true == _ReadDp( dDp ) )
	{
		m_pclIndSelParams->m_dDp = dDp;
	}
	else
	{
		m_pclIndSelParams->m_dDp = 0.0;
	}
}

void CDlgIndSelBase::OnEnSetFocusFlow()
{
	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	GetpEditFlow()->SetSel( 0, -1 );
}

void CDlgIndSelBase::OnEnSetFocusPower()
{
	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	GetpEditPower()->SetSel( 0, -1 );
}

void CDlgIndSelBase::OnEnSetFocusDT()
{
	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	GetpEditDT()->SetSel( 0, -1 );
}

void CDlgIndSelBase::SetModificationMode( bool bFlag )
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	if( false == bFlag )
	{
		m_pclIndSelParams->m_SelIDPtr = _NULL_IDPTR;
	}

	VerifyModificationMode();
}

void CDlgIndSelBase::VerifyCheckboxSetStatus( CRankEx *pList )
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	m_bAtLeastOneNo = false;
	m_bAtLeastOneYes = false;
	m_bAtLeastOneYesBreakOk = false;
	m_bAtLeastOneYesBreakOnMsg = false;

	if( NULL == pList )
	{
		return;
	}

	wstring str;
	LPARAM lParam;
	double dKey;

	for( BOOL bContinue = pList->GetFirst( str, lParam, &dKey ); TRUE == bContinue; bContinue = pList->GetNext( str, lParam, &dKey ) )
	{
		CDB_TAProduct *pTAProduct = dynamic_cast<CDB_TAProduct *>( ( CData * )lParam );

		if( NULL == pTAProduct )
		{
			continue;
		}

		switch( pTAProduct->GetPartOfaSet() )
		{
			case ePartOfaSet::ePartOfaSetNo:
				m_bAtLeastOneNo = true;
				break;

			case ePartOfaSet::ePartOfaSetYes:
				m_bAtLeastOneYes = true;
				break;

			case ePartOfaSet::ePartOfaSetYesBreakOk:
				m_bAtLeastOneYesBreakOk = true;
				break;

			case ePartOfaSet::ePartOfaSetYesBreakOnMsg:
				m_bAtLeastOneYesBreakOnMsg = true;
				break;
		}
	}

	//                      +____________________________+_____________________+______________________+
	//                      | m_fAutoCheckboxSetManaging | m_iCheckboxSetState | m_fCheckboxSetEnable |
	//                      +____________________________+_____________________+______________________+
	//                      |                            |                     |                      |
	// PartOfaSetNo         | 0                          | BST_UNCHECKED       | FALSE                |
	//                      | 1                          | BST_UNCHECKED       | FALSE                |
	// PartOfaSetYes        | 0                          | BST_CHECKED         | FALSE                |
	//                      | 1                          | BST_CHECKED         | FALSE                |
	// PartOfaSetBreakOK    | 0                          | Previous state      | TRUE                 |
	//                      | 1                          | BST_CHECKED         | TRUE                 |
	// PartOfaSetBreakOnMsg | 0                          | Previous state      | TRUE                 |
	//                      | 1                          | BST_CHECKED         | TRUE                 |
	//                      +---------------------------+---------------------+---------------------+

	if( true == m_bAtLeastOneYesBreakOk || true == m_bAtLeastOneYesBreakOnMsg )
	{
		// HYS-1068: Keep the previous value
		//m_pclIndSelParams->m_bOnlyForSet = m_bAutoCheckboxSetManaging;
		m_pclIndSelParams->m_bCheckboxSetEnabled = true;
	}
	else if( true == m_bAtLeastOneNo )
	{
		m_pclIndSelParams->m_bOnlyForSet = false;
		m_pclIndSelParams->m_bCheckboxSetEnabled = false;
	}
	else if( true == m_bAtLeastOneYes )
	{
		m_pclIndSelParams->m_bOnlyForSet = true;
		m_pclIndSelParams->m_bCheckboxSetEnabled = false;
	}
}

void CDlgIndSelBase::_UpdateFlowEdit( void )
{
	if( NULL == m_pclIndSelParams )
	{
		ASSERT_RETURN;
	}

	if( CDS_SelProd::efdFlow == m_pclIndSelParams->m_eFlowOrPowerDTMode )
	{
		return;
	}

	_ReadPower( m_pclIndSelParams->m_dPower );
	_ReadDT( m_pclIndSelParams->m_dDT );

	m_pclIndSelParams->m_dFlow = 0.0;

	if( m_pclIndSelParams->m_dPower > 0.0 && m_pclIndSelParams->m_dDT > 0.0 )
	{
		CWaterChar clWaterCharReturn = m_pclIndSelParams->m_WC;
		double dReturnTemperature = ( ProjectType::Cooling == m_pclIndSelParams->m_eApplicationType ) ? m_pclIndSelParams->m_WC.GetTemp() + m_pclIndSelParams->m_dDT :
				m_pclIndSelParams->m_WC.GetTemp() - m_pclIndSelParams->m_dDT;

		clWaterCharReturn.UpdateFluidData( dReturnTemperature );

		double denominator = m_pclIndSelParams->m_WC.GetTemp() * m_pclIndSelParams->m_WC.GetDens() * m_pclIndSelParams->m_WC.GetSpecifHeat();
		denominator -= ( clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat() );
		m_pclIndSelParams->m_dFlow = abs( m_pclIndSelParams->m_dPower / denominator );
	}

	if( NULL == GetpEditFlow() )
	{
		return;
	}

	if( m_pclIndSelParams->m_dFlow > 0.0 )
	{
		GetpEditFlow()->SetCurrentValSI( m_pclIndSelParams->m_dFlow );
		GetpEditFlow()->Update();
	}
	else
	{
		GetpEditFlow()->SetWindowText( _T("") );
	}
}

bool CDlgIndSelBase::_ReadFlow( double &dFlow )
{
	if( NULL == GetpEditFlow() )
	{
		return false;
	}

	dFlow = 0.0;

	if( RD_NOT_NUMBER == ReadCUDouble( _U_FLOW, *( (CEdit *)GetpEditFlow() ), &dFlow ) )
	{
		return false;
	}

	if( dFlow <= 0.0 )
	{
		return false;
	}

	return true;
}

bool CDlgIndSelBase::_ReadDp( double &dDp )
{
	if( NULL == GetpEditDp() )
	{
		return false;
	}

	dDp = 0.0;

	if( RD_NOT_NUMBER == ReadCUDouble( _U_DIFFPRESS, *( (CEdit *)GetpEditDp() ), &dDp ) )
	{
		return false;
	}

	if( dDp <= 0.0 )
	{
		return false;
	}

	return true;
}

bool CDlgIndSelBase::_ReadPower( double &dPower )
{
	if( NULL == GetpEditPower() )
	{
		return false;
	}

	dPower = 0.0;

	if( RD_NOT_NUMBER == ReadCUDouble( _U_TH_POWER, *( (CEdit *)GetpEditPower() ), &dPower ) )
	{
		return false;
	}

	if( dPower <= 0.0 )
	{
		return false;
	}

	return true;
}

bool CDlgIndSelBase::_ReadDT( double &dDT )
{
	if( NULL == GetpEditDT() )
	{
		return false;
	}

	dDT = 0.0;

	if( RD_NOT_NUMBER == ReadCUDouble( _U_DIFFTEMP, *( (CEdit *)GetpEditDT() ), &dDT ) )
	{
		return false;
	}

	if( dDT <= 0.0 )
	{
		return false;
	}

	return true;
}
