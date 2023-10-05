#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "Select.h"
#include "DlgLeftTabSelManager.h"

#include "DlgIndSelBase.h"
#include "DlgIndSel6WayValve_SecondTab.h"

#include "RViewSSelSS.h"
#include "RViewSSelCtrl.h"
#include "RViewSSel6WayValve.h"


CDlgIndSel6WayValve_SecondTab::CDlgIndSel6WayValve_SecondTab( CIndSel6WayValveParams &clIndSel6WayValveParams, CWnd * pParent )
	: CDlgIndSelCtrlBase( clIndSel6WayValveParams.m_clIndSelPIBCVParams, CDlgIndSel6WayValve_SecondTab::IDD, pParent )
{
	m_pclIndSel6WayValveParams = &clIndSel6WayValveParams;
	m_pNotificationHandler = NULL;
}

CDlgIndSel6WayValve_SecondTab::~CDlgIndSel6WayValve_SecondTab()
{
	m_pNotificationHandler = NULL;
	m_pclIndSel6WayValveParams = NULL;
}

void CDlgIndSel6WayValve_SecondTab::SaveSelectionParameters()
{
	CDlgIndSelCtrlBase::SaveSelectionParameters();

	CDS_IndSelParameter *pclIndSelParameter = m_pclIndSel6WayValveParams->m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		// PIBCv case.
		pclIndSelParameter->Set6WValvePICvDpMaxChecked( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked );
		pclIndSelParameter->Set6WValvePICvTypeID( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID );
		pclIndSelParameter->Set6WValvePICvCBCtrlType( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType );
		pclIndSelParameter->Set6WValvePICvFamilyID( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID );
		pclIndSelParameter->Set6WValvePICvMaterialID( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID );
		pclIndSelParameter->Set6WValvePICvConnectID( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID );
		pclIndSelParameter->Set6WValvePICvVersionID( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID );
		pclIndSelParameter->Set6WValvePICvPNID( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboPNID );
		pclIndSelParameter->Set6WValvePICvActInputSignalID( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorInputSignalID );
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		// BV case.
		pclIndSelParameter->Set6WValveBvDpChecked( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled );
		pclIndSelParameter->Set6WValveBVTypeID( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID );
		pclIndSelParameter->Set6WValveBVFamilyID( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboFamilyID );
		pclIndSelParameter->Set6WValveBVMaterialID( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboMaterialID );
		pclIndSelParameter->Set6WValveBVConnectID( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboConnectID );
		pclIndSelParameter->Set6WValveBVVersionID( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboVersionID );
		pclIndSelParameter->Set6WValveBVPNID( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboPNID );
	}
}

int CDlgIndSel6WayValve_SecondTab::VerifyDpValue( void )
{
	int iReturn = VDPFlag_OK;

	if( true == m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled )
	{
		// If Dp value not valid...
		if( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp <= 0.0 )
		{
			// Ask to user if he wants continue selection without Dp.
			if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_NODP_DPCHECKED, MB_YESNO | MB_ICONQUESTION ) )
			{
				// Disable Dp field.
				// Remark: don't call 'OnBnClickedCheckdp()' because this one internally calls 'ClearAll' and it is too much!
				m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled = false;
				m_clGroupDp.SetCheck( BST_UNCHECKED );
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
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp = 0.0;
	}

	return iReturn;
}

void CDlgIndSel6WayValve_SecondTab::Update6WayTabCombo( UpdateFrom eUpdateFrom )
{
	bool bGroupActuatorEnabled = false;
	int iTypeBottomMarginFit = 3; // When we delete ctrl type we have to remove the space to get the exact bottom margin
	int iPibcvGpBottomMarginFit = 5; // When we delete actuator group we have to remove the space to get the exact bottom margin

	if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode || e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		bGroupActuatorEnabled = true;
	}

	if( UF_NewDocument == eUpdateFrom || UF_SelectionModeChanged == eUpdateFrom || UF_ProductEdited == eUpdateFrom )
	{
		_FillComboType();
	
		// Hide CtrlType combo and actuator group when the circuit type is with STAD
		CRect CurrentClientRect;
		CRect RectCtrlType;
		CRect RectCBCtrlType;
	
		// To calculate the hidden height in group valve type 'offset'.
		GetDlgItem( IDC_STATICCTRLTYPE )->GetWindowRect( &RectCtrlType );
		ScreenToClient( RectCtrlType );
	
		m_ComboCtrlType.GetWindowRect( &RectCBCtrlType );
		ScreenToClient( RectCBCtrlType );
	
		m_GroupValveType.GetClientRect( &CurrentClientRect );
		// for bottom margin add iTypeBottomMarginFit
		long lOffset = RectCtrlType.Height() + RectCBCtrlType.Height() + iTypeBottomMarginFit;

		if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
		{
			m_GroupDpMax.ShowWindow( false );
			m_clExtEditDpMax.ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICDPMAXUNIT )->ShowWindow( SW_HIDE );

			m_clGroupDp.ShowWindow( true );
			m_clExtEditDp.ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICDPUNIT )->ShowWindow( SW_SHOW );

			UpdateDpFieldState();
		
			m_ComboCtrlType.ResetContent();
			m_ComboCtrlType.EnableWindow( FALSE );
		
			// Hide CtrlType.
			m_ComboCtrlType.ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICCTRLTYPE )->ShowWindow( SW_HIDE );
		
			// Resize the group valve type only if the previous state has the initial size
			if( m_RCGroupValveType == CurrentClientRect )
			{
				CurrentClientRect.bottom -= lOffset;
				m_GroupValveType.SetWindowPos( NULL, -1, -1, CurrentClientRect.Width(), CurrentClientRect.Height(), SWP_NOMOVE | SWP_NOACTIVATE );
				m_GroupValveType.Invalidate();
				m_GroupValveType.UpdateWindow();
			
				// Move up all objects below .
				_VerticalMoveForBvDisplay( lOffset );
				if( false == IsPIBCVGroupResized() )
				{
					// if the previous state is EQM control
					// Hide actuator group.
					CRect ActGroupRect;
					m_GroupActuator.GetWindowRect( &ActGroupRect );
					ScreenToClient( ActGroupRect );
					long lGpOffset = lOffset + ActGroupRect.Height() + iPibcvGpBottomMarginFit;

					// Resize Bv group by removing lGpOffset.
					ResizeSecondTabNotification( lGpOffset );
				}
				else
				{
					// If the previous state is On off control with compact-p the actuator group is already removed
					// we have to move up the bottom limit
					// Resize Bv group by removing lOffset.
					ResizeSecondTabNotification( lOffset );
				}
			}

			m_ComboInputSignal.ResetContent();
			m_ComboInputSignal.EnableWindow( FALSE );
			m_GroupActuator.ShowWindow( SW_HIDE );
		}
		else if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) 
			|| ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
		{
			m_GroupDpMax.ShowWindow( true );
			m_clExtEditDpMax.ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICDPMAXUNIT )->ShowWindow( SW_SHOW );

			UpdateDpMaxFieldState();
		
			m_clGroupDp.ShowWindow( SW_HIDE );
			m_clExtEditDp.ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICDPUNIT )->ShowWindow( SW_HIDE );
		
			m_ComboCtrlType.EnableWindow( TRUE );
		
			// Show ctrlType infos.
			m_ComboCtrlType.ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICCTRLTYPE )->ShowWindow( SW_SHOW );
			FillComboCtrlType( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType );
		
			// Show actuator infos.
			m_GroupActuator.ShowWindow( true );

			// Resize valve type group only if the previous state has not the initial size.
			m_GroupValveType.GetClientRect( &CurrentClientRect );

			if( m_RCGroupValveType != CurrentClientRect )
			{
				// control type infos must be added
				CurrentClientRect.bottom += lOffset;
			
				// Resize the group valve type.
				m_GroupValveType.SetWindowPos( NULL, -1, -1, CurrentClientRect.Width(), CurrentClientRect.Height(), SWP_NOMOVE | SWP_NOACTIVATE );
				m_GroupValveType.Invalidate();
				m_GroupValveType.UpdateWindow();
			
				// Move down all objects below.
				_VerticalMoveForBvDisplay( lOffset, false );
				if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
				{
					// Resize Bv group by adding actuator group size.
					CRect ActGroupRect;
					m_GroupActuator.GetWindowRect( &ActGroupRect );
					ScreenToClient( ActGroupRect );
					long lGpOffset = lOffset + ActGroupRect.Height() + iPibcvGpBottomMarginFit;
					ResizeSecondTabNotification( lGpOffset, false );
				}
				else
				{
					// The previous state is not EQM control, actuator group has already removed
					ResizeSecondTabNotification( lOffset, false );
				}
			}
			else if( true == IsPIBCVGroupResized() )
			{
				// control type infos must not be added but actuator group was missing in the previous state
				CRect ActGroupRect;
				m_GroupActuator.GetWindowRect( &ActGroupRect );
				ScreenToClient( ActGroupRect );
				long lGpOffset = ActGroupRect.Height() + iPibcvGpBottomMarginFit;
				ResizeSecondTabNotification( lGpOffset, false );
			}
			else if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				// control type infos must not be added, actuator group is added in the previous state
				CRect ActGroupRect;
				m_GroupActuator.GetWindowRect( &ActGroupRect );
				ScreenToClient( ActGroupRect );
				long lGpOffset = ActGroupRect.Height() + iPibcvGpBottomMarginFit;
				ResizeSecondTabNotification( lGpOffset );
			}
		}
	}

	// HYS-1877: Update combo when set checkbox status changes.
	_FillComboFamily();
	_FillComboBodyMat();
	_FillComboConnect();
	_FillComboVersion();
	_FillComboPN();

	if( true == bGroupActuatorEnabled )
	{
		if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
		{
			m_GroupActuator.EnableWindow( FALSE );
			m_GroupActuator.ShowWindow( false );
		}
		else
		{
			_FillComboInputSignal();
		}
	}
}

void CDlgIndSel6WayValve_SecondTab::UpdateDpMaxFieldState()
{
	m_GroupDpMax.SetCheck( ( false == m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked ) ? BST_UNCHECKED : BST_CHECKED );
	
	if( true == m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked )
	{
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( TRUE );
		m_clExtEditDpMax.SetFocus();
		m_clExtEditDpMax.SetCurrentValSI( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax );
		m_clExtEditDpMax.Update();
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( TRUE );
	}
	else
	{
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( FALSE );
		m_clExtEditDpMax.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_UNKNOWN ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( FALSE );
	}
}

void CDlgIndSel6WayValve_SecondTab::SetNotificationHandler( ISecondTab_NotificationHandler *pINotificationHandler )
{
	if( NULL == pINotificationHandler )
	{
		return;
	}

	m_pNotificationHandler = pINotificationHandler;
}

void CDlgIndSel6WayValve_SecondTab::ResetNotificationHandler( void )
{
	m_pNotificationHandler = NULL;
}

void CDlgIndSel6WayValve_SecondTab::SetSecondTabParameters( CDS_SSel6WayValve *pclCurrent6WayValve )
{
	if( NULL == pclCurrent6WayValve )
	{
		return;
	}

	// Remark: Corresponding combo variables in 'm_clIndSelCVParams' are updated in each of this following methods.
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		// Remark: 'm_clIndSelPIBCVParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
		m_GroupDpMax.ShowWindow( true );
		m_clExtEditDpMax.ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICDPMAXUNIT )->ShowWindow( SW_SHOW );

		m_clGroupDp.ShowWindow( SW_HIDE );
		m_clExtEditDp.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICDPUNIT )->ShowWindow( SW_HIDE );

		// Remark: 'm_clIndSelPIBCVParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax = pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetDpMax();
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked = ( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax > 0.0 ) ? true : false;
		m_GroupDpMax.SetCheck( ( true == m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked ) ? BST_CHECKED : BST_UNCHECKED );
		
		UpdateDpMaxFieldState();

		// Remark: 'm_clIndSelPIBCVParams.m_dDpMax' is updated in the following method.
		// _SetDpMax( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetDpMax() );

		_FillComboType( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetTypeID() );
		FillComboCtrlType( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetCtrlType() );
		_FillComboFamily( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetFamilyID() );
		_FillComboBodyMat( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetMaterialID() );
		_FillComboConnect( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetConnectID() );
		_FillComboVersion( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetVersionID() );
		_FillComboPN( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetPNID() );

		if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
		{
			//GetDlgItem( IDC_STATICINPUTSIGNAL )->EnableWindow( FALSE );
			m_GroupActuator.EnableWindow( FALSE );
			m_ComboInputSignal.ResetContent();
			m_ComboInputSignal.EnableWindow( FALSE );
		}
		else
		{
			_FillComboInputSignal( pclCurrent6WayValve->GetCDSSSelPICv( BothSide )->GetActuatorInputSignalIDPtr() );
		}
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_GroupDpMax.ShowWindow( SW_HIDE );
		m_clExtEditDpMax.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICDPMAXUNIT )->ShowWindow( SW_HIDE );

		m_clGroupDp.ShowWindow( true );
		m_clExtEditDp.ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICDPUNIT )->ShowWindow( SW_SHOW );

		// Set the Dp check box state.
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled = ( pclCurrent6WayValve->GetCDSSSelBv( HeatingSide )->GetDp() > 0.0 ) ? true : false;
		m_clGroupDp.SetCheck( ( true == m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled ) ? BST_CHECKED : BST_UNCHECKED );
		
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp = pclCurrent6WayValve->GetCDSSSelBv( HeatingSide )->GetDp();
		UpdateDpFieldState();
		GetpEditDp()->SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp ) );

		_FillComboType( pclCurrent6WayValve->GetCDSSSelBv( HeatingSide )->GetTypeID() );
		_FillComboFamily( pclCurrent6WayValve->GetCDSSSelBv( HeatingSide )->GetFamilyID() );
		_FillComboBodyMat( pclCurrent6WayValve->GetCDSSSelBv( HeatingSide )->GetMaterialID() );
		_FillComboConnect( pclCurrent6WayValve->GetCDSSSelBv( HeatingSide )->GetConnectID() );
		_FillComboVersion( pclCurrent6WayValve->GetCDSSSelBv( HeatingSide )->GetVersionID() );
		_FillComboPN( pclCurrent6WayValve->GetCDSSSelBv( HeatingSide )->GetPNID() );
		
		m_ComboCtrlType.ResetContent();
		m_ComboCtrlType.EnableWindow( FALSE );

		m_ComboInputSignal.ResetContent();
		m_ComboInputSignal.EnableWindow( FALSE );
	}
}

bool CDlgIndSel6WayValve_SecondTab::LeftTabKillFocus( bool fNext )
{
	// !!! Does not intentionally call the base class !!!

	bool fReturn = false;

	if( true == fNext )
	{
		// TAB -> must set the focus on the 'CDlgInSelPMPanels' and set the focus on the first control.

		if( NULL != pRViewSSel6WayValve && TRUE == pRViewSSel6WayValve->IsWindowVisible() && false == pRViewSSel6WayValve->IsEmpty() )
		{
			// Set the focus on the right view.
			pRViewSSel6WayValve->SetFocus();
			// Set focus on the appropriate group in the right view.
			pRViewSSel6WayValve->SetFocusW( true );
			fReturn = true;
		}
		else
		{
			// Focus must be set on the first control of the parent 'CDlgIndSelPressureMaintenance'.
			SetFocusNotification();
			SetFocusOnControlNotification( CDlgSelectionTabHelper::SetFocusWhere::First );
			fReturn = true;
		}
	}
	else
	{
		// SHIFT + TAB -> go to the top control only if right view is not empty.
		SetFocusNotification();
		SetFocusOnControlNotification( CDlgSelectionTabHelper::SetFocusWhere::Last );
		fReturn = true;
	}

	return fReturn;
}

void CDlgIndSel6WayValve_SecondTab::OnRViewSSelLostFocusWithTabKey( bool bShiftPressed )
{
	if( NULL != pRViewSSel6WayValve && TRUE == pRViewSSel6WayValve->IsWindowVisible() && false == pRViewSSel6WayValve->IsEmpty() )
	{
		// Verify the dialog is active.
		if( FALSE == this->IsWindowVisible() )
		{
			return;
		}

		// Reset the focus on the left tab.
		if( NULL != pDlgLeftTabSelManager )
		{
			pDlgLeftTabSelManager->SetFocus();
		}
		if( false == bShiftPressed )
		{
			SetFocusNotification();
			// Set the focus on the first available edit control of the lef tab.
			SetFocusOnControlNotification( SetFocusWhere::First );
		}
		else
		{
			// Set the focus on the last available edit control of the lef tab.
			SetFocus();
			SetFocusOnControl( SetFocusWhere::Last );
		}
	}
	else
	{
		CDlgIndSelCtrlBase::OnRViewSSelLostFocusWithTabKey( bShiftPressed );
	}
}

void CDlgIndSel6WayValve_SecondTab::DoDataExchange( CDataExchange * pDX )
{
	CDlgIndSelCtrlBase::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_EDITDP, m_clExtEditDp );
	DDX_Control( pDX, IDC_GROUPDP, m_clGroupDp );
	DDX_Control( pDX, IDC_GROUPDPMAX, m_GroupDpMax );
	DDX_Control( pDX, IDC_EDITDPMAX, m_clExtEditDpMax );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOCTRLTYPE, m_ComboCtrlType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboMaterial );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
	DDX_Control( pDX, IDC_COMBOINPUTSIGNAL, m_ComboInputSignal );
	DDX_Control( pDX, IDC_GROUPVALVETYPE, m_GroupValveType );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPACTUATOR, m_GroupActuator );
}

BOOL CDlgIndSel6WayValve_SecondTab::OnInitDialog()
{
	CDlgIndSelCtrlBase::OnInitDialog();

	// Reset pointers on 'Database' and 'TADataStruct' to those ones by default also for the BV selection.
	m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_pTADB = m_pclIndSel6WayValveParams->m_pTADB;
	m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_pTADS = m_pclIndSel6WayValveParams->m_pTADS;
	m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bEditModeRunning = m_pclIndSel6WayValveParams->m_bEditModeRunning;

	m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_pTADB = m_pclIndSel6WayValveParams->m_pTADB;
	m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_pTADS = m_pclIndSel6WayValveParams->m_pTADS;
	m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bEditModeRunning = m_pclIndSel6WayValveParams->m_bEditModeRunning;

	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICCTRLTYPE );
	GetDlgItem( IDC_STATICCTRLTYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICMATERIAL );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_SECTABCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_SECTABVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICINPUTSIGNAL );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );

	// Set proper style and add icons to groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupKvDp.SetExpandCollapseMode( true );
// 	m_GroupKvDp.SetNotificationHandler( this );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupValveType.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
	}

	m_GroupValveType.SetInOffice2007Mainframe( true );

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupValveType.SetExpandCollapseMode( true );
// 	m_GroupValveType.SetNotificationHandler( this );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupValve.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
	}

	m_GroupValve.SetInOffice2007Mainframe( true );

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupValve.SetExpandCollapseMode( true );
// 	m_GroupValve.SetNotificationHandler( this );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupActuator.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Actuator );
	}
	m_GroupActuator.SetInOffice2007Mainframe( true );
	m_GroupActuator.SaveChildList();


	if( NULL != pclImgListGroupBox )
	{
		m_clGroupDp.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_DP );
	}

	m_clGroupDp.SetCheckBoxStyle( BS_AUTOCHECKBOX );
	m_clGroupDp.SetInOffice2007Mainframe( true );

	m_GroupDpMax.SetCheckBoxStyle( BS_AUTOCHECKBOX );
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICDPMAX );
	m_GroupDpMax.SetWindowText( str );
	m_GroupDpMax.SetInOffice2007Mainframe( true );
	m_clExtEditDpMax.SetPhysicalType( _U_DIFFPRESS );
	m_clExtEditDpMax.SetEditType( CNumString::eDouble, CNumString::ePositive );
	
	// To save the init size of the group for Bv view. Because if we switch in Bv mode, we hide ctrlType combo
	m_GroupValveType.GetClientRect( &m_RCGroupValveType );
	return TRUE;
}

LRESULT CDlgIndSel6WayValve_SecondTab::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if ( NULL == m_pclIndSel6WayValveParams->m_pTADS )
	{
		return -1;
	}

	CDlgIndSelCtrlBase::OnNewDocument( wParam, lParam );
	ClearCommonInputEditors();

	if( false == TASApp.Is6WayCVDisplayed() )
	{
		return 0;
	}

	// Get last selected parameters.
	CDS_IndSelParameter *pclIndSelParameter = m_pclIndSel6WayValveParams->m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}
	
	// Update the view regarding to the circuit.
	Update6WayTabCombo( UF_NewDocument );
	
	// Remark: Corresponding combo variables in 'm_clIndSelCVParams' are updated in each of this following methods.
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		// Remark: 'm_clIndSelPIBCVParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
		m_GroupDpMax.ShowWindow( true );
		m_clExtEditDpMax.ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICDPMAXUNIT )->ShowWindow( SW_SHOW );

		m_clGroupDp.ShowWindow( SW_HIDE );
		m_clExtEditDp.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICDPUNIT )->ShowWindow( SW_HIDE );
		
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked = pclIndSelParameter->Get6WValvePICvDpMaxChecked();
		m_GroupDpMax.SetCheck( ( false == pclIndSelParameter->Get6WValvePICvDpMaxChecked() ) ? BST_UNCHECKED : BST_CHECKED );
		
		UpdateDpMaxFieldState();
		
		_FillComboType( pclIndSelParameter->Get6WValvePICvTypeID() );
		FillComboCtrlType( pclIndSelParameter->Get6WValvePICvCBCtrlType() );
		_FillComboFamily( pclIndSelParameter->Get6WValvePICvFamilyID() );
		_FillComboBodyMat( pclIndSelParameter->Get6WValvePICvMaterialID() );
		_FillComboConnect( pclIndSelParameter->Get6WValvePICvConnectID() );
		_FillComboVersion( pclIndSelParameter->Get6WValvePICvVersionID() );
		_FillComboPN( pclIndSelParameter->Get6WValvePICvPNID() );
		if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
		{
			//GetDlgItem( IDC_STATICINPUTSIGNAL )->EnableWindow( FALSE );
			m_GroupActuator.EnableWindow( FALSE );
			m_ComboInputSignal.ResetContent();
			m_ComboInputSignal.EnableWindow( FALSE );
		}
		else
		{
			_FillComboInputSignal( pclIndSelParameter->Get6WValvePICvActInputSignalID() );
		}
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_GroupDpMax.ShowWindow( SW_HIDE );
		m_clExtEditDpMax.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICDPMAXUNIT )->ShowWindow( SW_HIDE );

		m_clGroupDp.ShowWindow( true );
		m_clExtEditDp.ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICDPUNIT )->ShowWindow( SW_SHOW );

		// Set the Dp check box state.
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled = pclIndSelParameter->Get6WValveBvDpChecked();
		m_clGroupDp.SetCheck( ( true == m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled ) ? BST_CHECKED : BST_UNCHECKED );
		
		UpdateDpFieldState();
		
		_FillComboType( pclIndSelParameter->Get6WValveBVTypeID() );
		_FillComboFamily( pclIndSelParameter->Get6WValveBVFamilyID() );
		_FillComboBodyMat( pclIndSelParameter->Get6WValveBVMaterialID() );
		_FillComboConnect( pclIndSelParameter->Get6WValveBVConnectID() );
		_FillComboVersion( pclIndSelParameter->Get6WValveBVVersionID() );
		_FillComboPN( pclIndSelParameter->Get6WValveBVPNID() );
		
		m_ComboCtrlType.ResetContent();
		m_ComboCtrlType.EnableWindow( FALSE );

		m_ComboInputSignal.ResetContent();
		m_ComboInputSignal.EnableWindow( FALSE );
	}

	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();

	EnableSelectButtonNotification( false );
	m_bInitialised = true;

	return 0;
}

BEGIN_MESSAGE_MAP( CDlgIndSel6WayValve_SecondTab, CDlgIndSelCtrlBase )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOCTRLTYPE, OnCbnSelChangeCtrlType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeBodyMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnCbnSelChangeInputSignal )
	ON_COMMAND( IDC_GROUPDP, OnBnClickedCheckDp )
	ON_EN_CHANGE( IDC_EDITDPMAX, OnEnChangeDpMax )
	ON_EN_KILLFOCUS( IDC_EDITDPMAX, OnEnKillFocusDpMax )
	ON_BN_CLICKED( IDC_GROUPDPMAX, OnBnClickedCheckGroupDpMax )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDPMAX, OnEditEnterChar )
END_MESSAGE_MAP()

void CDlgIndSel6WayValve_SecondTab::OnBnClickedCheckGroupDpMax()
{
	m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked = ( BST_CHECKED == m_GroupDpMax.GetCheck() ) ? true : false;
	UpdateDpMaxFieldState();

	EnableSelectButtonNotification( false );
	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}
}

void CDlgIndSel6WayValve_SecondTab::OnEnChangeDpMax()
{
	if( GetFocus() == &m_clExtEditDpMax )
	{
		EnableSelectButtonNotification( false );

		if( NULL != pRViewSSel6WayValve )
		{
			pRViewSSel6WayValve->Reset();
		}
	}
}

void CDlgIndSel6WayValve_SecondTab::OnEnKillFocusDpMax()
{
	m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpMax, &(m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax) )
			|| m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax < 0.0 )
	{
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax = 0.0;
	}
}

void CDlgIndSel6WayValve_SecondTab::OnBnClickedCheckDp()
{
	m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled = ( BST_CHECKED == m_clGroupDp.GetCheck() ) ? true : false;
	UpdateDpFieldState();
	EnableSelectButtonNotification( false );

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}
}

void CDlgIndSel6WayValve_SecondTab::OnCbnSelChangeType()
{
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{ 
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
		FillComboCtrlType();
		OnCbnSelChangeCtrlType();
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
		m_ComboCtrlType.ResetContent();
		m_ComboCtrlType.EnableWindow( FALSE );
		_FillComboFamily();
		OnCbnSelChangeFamily();
	}
}

void CDlgIndSel6WayValve_SecondTab::OnCbnSelChangeCtrlType()
{
	m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_ComboCtrlType.GetCurSel();

	if( iCur >= 0 )
	{
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType = ( CDB_ControlProperties::CvCtrlType )m_ComboCtrlType.GetItemData( iCur );
	}

	_FillComboFamily();
	OnCbnSelChangeFamily();
}

void CDlgIndSel6WayValve_SecondTab::OnCbnSelChangeFamily()
{
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	}
	
	_FillComboBodyMat();
	OnCbnSelChangeBodyMaterial();
}

void CDlgIndSel6WayValve_SecondTab::OnCbnSelChangeBodyMaterial()
{
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
	}

	_FillComboConnect();
	OnCbnSelChangeConnect();
}

void CDlgIndSel6WayValve_SecondTab::OnCbnSelChangeConnect()
{
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	}

	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgIndSel6WayValve_SecondTab::OnCbnSelChangeVersion()
{
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	}

	_FillComboPN();
	OnCbnSelChangePN();
}

void CDlgIndSel6WayValve_SecondTab::OnCbnSelChangePN()
{
	m_pclIndSel6WayValveParams->m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	
	if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		_FillComboInputSignal();
		OnCbnSelChangeInputSignal();
	}
	else
	{
		ClearAllNotification();
	}
}

void CDlgIndSel6WayValve_SecondTab::OnCbnSelChangeInputSignal()
{
	m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	ClearAllNotification();
}

void CDlgIndSel6WayValve_SecondTab::FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	CDlgIndSelCtrlBase::FillComboCtrlType( CTADatabase::eForPiCv, eCvCtrlType );
	
	// For PIBCV with 6 way valve we just need proportional control type.
	if( ( (CExtNumEditComboBox *)GetDlgItem( IDC_COMBOCTRLTYPE ) )->GetCount() > 0 )
	{
		( (CExtNumEditComboBox *)GetDlgItem( IDC_COMBOCTRLTYPE ) )->SetCurSel( 0 );
		( (CExtNumEditComboBox *)GetDlgItem( IDC_COMBOCTRLTYPE ) )->EnableWindow( FALSE );
	}
}

void CDlgIndSel6WayValve_SecondTab::EnableSelectButtonNotification( bool bEnable )
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->IPIBCvBvNotification_EnableSelectButton( bEnable );
	}
}

void CDlgIndSel6WayValve_SecondTab::ClearAllNotification()
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->IPIBCvBvNotification_ClearAll();
	}
}

void CDlgIndSel6WayValve_SecondTab::ResizeSecondTabNotification( long &lOffset, bool bReduce )
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->IPIBCvBvNotification_ResizeSecondTab( lOffset, bReduce );
	}
}

void CDlgIndSel6WayValve_SecondTab::SetFocusNotification()
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->IPIBCvBvNotification_SetFocus();
	}
}

void CDlgIndSel6WayValve_SecondTab::SetFocusOnControlNotification( CDlgSelectionTabHelper::SetFocusWhere ePosition )
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->IPIBCvBvNotification_SetFocusOnControl( ePosition );
	}
}

bool CDlgIndSel6WayValve_SecondTab::IsPIBCVGroupResized()
{
	if( NULL != m_pNotificationHandler )
	{
		return m_pNotificationHandler->IPIBCvBvNotification_IsPIBCVGroupResized();
	}
	return false;
}

void CDlgIndSel6WayValve_SecondTab::OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITDPMAX == pNMHDR->idFrom )
	{
		if( NULL != m_pNotificationHandler )
		{
			m_pNotificationHandler->IPIBCvBvNotification_OnEditEnterChar( pNMHDR );
		}
	}
	else
	{
		CDlgIndSelCtrlBase::OnEditEnterChar( pNMHDR, pResult );
	}
}

void CDlgIndSel6WayValve_SecondTab::OnEnChangeDp()
{
	if( GetFocus() == GetpEditDp() )
	{
		EnableSelectButtonNotification( false );
		if( NULL != pRViewSSel6WayValve )
		{
			pRViewSSel6WayValve->Reset();
		}
	}
}

void CDlgIndSel6WayValve_SecondTab::OnEnKillFocusDp()
{
	if( NULL == GetpEditDp() )
	{
		return;
	}
	
	m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, *( (CEdit *)GetpEditDp() ), &( m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp ) )
			|| m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp < 0.0 )
	{
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp = 0.0;
	}
}

CRViewSSelSS * CDlgIndSel6WayValve_SecondTab::GetLinkedRightViewSSel( void )
{
	return pRViewSSel6WayValve;
}

LRESULT CDlgIndSel6WayValve_SecondTab::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	return 0;
}

LRESULT CDlgIndSel6WayValve_SecondTab::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnPipeChange( wParam, lParam );

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}
	
	return 0;
}

LRESULT CDlgIndSel6WayValve_SecondTab::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	TCHAR tcName[_MAXCHARS];

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetDlgItemText( IDC_STATICDPMAXUNIT, tcName );

	if( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax > 0.0 )
	{
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax ) );
	}

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}
	
	return 0;
}

void CDlgIndSel6WayValve_SecondTab::UpdateDpFieldState()
{
	m_clGroupDp.SetCheck( ( true == m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled ) ? BST_CHECKED : BST_UNCHECKED );
	
	if( true == m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_bDpEnabled )
	{
		if( NULL != GetpEditDp() )
		{
			( (CEdit *)GetpEditDp() )->SetReadOnly( FALSE );
			GetpEditDp()->EnableWindow( TRUE );
			GetpEditDp()->SetFocus();
			GetpEditDp()->SetWindowText( _T( "" ) );
		}

		if( NULL != GetDlgItem( IDC_STATICDPUNIT ) )
		{
			GetDlgItem( IDC_STATICDPUNIT )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_STATICDPUNIT )->EnableWindow( TRUE );
		}
	}
	else
	{
		if( NULL != GetpEditDp() )
		{
			( (CEdit *)GetpEditDp() )->SetReadOnly( TRUE );
			GetpEditDp()->EnableWindow( FALSE );
		}

		if( NULL != GetDlgItem( IDC_STATICDPUNIT ) )
		{
			GetDlgItem( IDC_STATICDPUNIT )->EnableWindow( FALSE );
		}

		GetpEditDp()->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_UNKNOWN ) );

		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_dDp = 0.0;
	}
}

void CDlgIndSel6WayValve_SecondTab::_FillComboType( CString strTypeID )
{
	CRankEx TypeList;

	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_pTADB->Get6WayValvePICVTypeList( &TypeList, m_pclIndSel6WayValveParams->m_eFilterSelection );
		m_ComboType.FillInCombo( &TypeList, strTypeID );
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_pTADB->Get6WayValveSecondaryTypeList( &TypeList, m_pclIndSel6WayValveParams->m_eFilterSelection );
		m_ComboType.FillInCombo( &TypeList, strTypeID );
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSel6WayValve_SecondTab::_FillComboFamily( CString strFamilyID )
{
	CRankEx FamList;

	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		// HYS-1877: Consider Set selection
		m_pclIndSel6WayValveParams->m_pTADB->Get6WayValvePIBCvFamilyList( &FamList, m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode, m_pclIndSel6WayValveParams->m_eFilterSelection,
																		  0, INT_MAX, m_pclIndSel6WayValveParams->m_bOnlyForSet );
		m_ComboFamily.FillInCombo( &FamList, strFamilyID, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.GetComboFamilyAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_pTADB->Get6WayValveBvFamilyList( &FamList, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID, m_pclIndSel6WayValveParams->m_eFilterSelection );
		m_ComboFamily.FillInCombo( &FamList, strFamilyID, m_pclIndSel6WayValveParams->m_clIndSelBVParams.GetComboFamilyAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSel6WayValve_SecondTab::_FillComboBodyMat( CString strBodyMaterialID )
{
	CRankEx BdyMatList;

	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_pTADB->GetTaCVBdyMatList( &BdyMatList, CTADatabase::eForPiCv, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCV2W3W, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID,
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID, 
				CDB_ControlProperties::LastCVFUNC, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType, m_pclIndSel6WayValveParams->m_eFilterSelection );

		m_ComboMaterial.FillInCombo( &BdyMatList, strBodyMaterialID, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.GetComboMaterialAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_pTADB->GetBVBdyMatList( &BdyMatList, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboFamilyID,
				m_pclIndSel6WayValveParams->m_eFilterSelection );

		m_ComboMaterial.FillInCombo( &BdyMatList, strBodyMaterialID, m_pclIndSel6WayValveParams->m_clIndSelBVParams.GetComboMaterialAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
	}

}

void CDlgIndSel6WayValve_SecondTab::_FillComboConnect( CString strConnectID )
{
	CRankEx ConnList;
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_pTADB->GetTaCVConnList( &ConnList, CTADatabase::eForPiCv, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCV2W3W, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID,
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID, CDB_ControlProperties::LastCVFUNC,
				m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType, m_pclIndSel6WayValveParams->m_eFilterSelection );

		m_ComboConnect.FillInCombo( &ConnList, strConnectID, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.GetComboConnectAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_pTADB->GetBVConnList( &ConnList, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboFamilyID,
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboMaterialID, m_pclIndSel6WayValveParams->m_eFilterSelection );

		m_ComboConnect.FillInCombo( &ConnList, strConnectID, m_pclIndSel6WayValveParams->m_clIndSelBVParams.GetComboConnectAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSel6WayValve_SecondTab::_FillComboVersion( CString strVersionID )
{
	CRankEx VerList;
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_pTADB->GetTaCVVersList( &VerList, CTADatabase::eForPiCv, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCV2W3W, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID,
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID,
				CDB_ControlProperties::LastCVFUNC, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType, m_pclIndSel6WayValveParams->m_eFilterSelection );

		m_ComboVersion.FillInCombo( &VerList, strVersionID, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.GetComboVersionAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_pTADB->GetBVVersList( &VerList, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboFamilyID,
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboMaterialID, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboConnectID, 
				m_pclIndSel6WayValveParams->m_eFilterSelection );

		m_ComboVersion.FillInCombo( &VerList, strVersionID, m_pclIndSel6WayValveParams->m_clIndSelBVParams.GetComboVersionAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSel6WayValve_SecondTab::_FillComboPN( CString strPNID )
{
	CRankEx PNList;
	if( ( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) || ( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode ) )
	{
		m_pclIndSel6WayValveParams->m_pTADB->GetTaCVPNList( &PNList, CTADatabase::eForPiCv, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCV2W3W, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID,
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID, CDB_ControlProperties::LastCVFUNC, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType,
				m_pclIndSel6WayValveParams->m_eFilterSelection );

		m_ComboPN.FillInCombo( &PNList, strPNID, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.GetComboPNAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_pTADB->GetBVPNList( &PNList, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboFamilyID,
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboMaterialID, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboConnectID, 
				(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboVersionID,
				m_pclIndSel6WayValveParams->m_eFilterSelection );

		m_ComboPN.FillInCombo( &PNList, strPNID, m_pclIndSel6WayValveParams->m_clIndSelBVParams.GetComboPNAllID() );
		m_pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSel6WayValve_SecondTab::_FillComboInputSignal( CString strInputSignalID )
{
	CRankEx PICV6WayList;

	m_pclIndSel6WayValveParams->m_pTADB->GetTaCVList(
			&PICV6WayList,												// List where to saved
			CTADatabase::eForPiCv,								// Control valve target (cv, hmcv, picv or bcv)
			false, 													// 'true' returns as soon a result is found
			m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID,							 						// Type ID
			(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID,			// Family ID
			(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID,		// Body material ID
			(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID,		// Connection ID
			(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID, 		// Version ID
			(LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboPNID,				// PN ID
			CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
			m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_pclIndSel6WayValveParams->m_eFilterSelection,0, INT_MAX, false, NULL,
		    m_pclIndSel6WayValveParams->m_bOnlyForSet ); // HYS-1877: Consider Set selection

	CRankEx InputSignalList;
	
	// HYS-1877: Consider Set selection
	m_pclIndSel6WayValveParams->m_pTADB->Get6WayValveActrInputSignalList( &InputSignalList, &PICV6WayList, 
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType, (LPCTSTR)m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID,
		m_pclIndSel6WayValveParams->m_bCheckFastConnection, m_pclIndSel6WayValveParams->m_eFilterSelection, m_pclIndSel6WayValveParams->m_bOnlyForSet );

	m_ComboInputSignal.FillInCombo( &InputSignalList, strInputSignalID, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.GetComboActuatorInputSignalAllID() );
	m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
}

void CDlgIndSel6WayValve_SecondTab::_VerticalMoveForBvDisplay( long &lOffset, bool bUp )
{
	if( false == bUp )
	{
		lOffset = -lOffset;
	}

	// Resize the group Dp .
	CRect RectDp;
	m_clGroupDp.GetWindowRect( &RectDp );
	ScreenToClient( RectDp );
	RectDp.top -= lOffset;
	m_clGroupDp.SetWindowPos( NULL, RectDp.left, RectDp.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	m_clGroupDp.Invalidate();
	m_clGroupDp.UpdateWindow();

	CRect RectEditDp;
	m_clExtEditDp.GetWindowRect( &RectEditDp );
	ScreenToClient( RectEditDp );
	RectEditDp.top -= lOffset;
	m_clExtEditDp.SetWindowPos( NULL, RectEditDp.left, RectEditDp.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	m_clExtEditDp.Invalidate();
	m_clExtEditDp.UpdateWindow();

	CRect RectUnitDp;
	GetDlgItem( IDC_STATICDPUNIT )->GetWindowRect( &RectUnitDp );
	ScreenToClient( RectUnitDp );
	RectUnitDp.top -= lOffset;
	GetDlgItem( IDC_STATICDPUNIT )->SetWindowPos( NULL, RectUnitDp.left, RectUnitDp.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	GetDlgItem( IDC_STATICDPUNIT )->Invalidate();
	GetDlgItem( IDC_STATICDPUNIT )->UpdateWindow();
	
	// Resize valve group.
	CRect RectGroupValve;
	m_GroupValve.GetWindowRect( &RectGroupValve );
	ScreenToClient( RectGroupValve );
	RectGroupValve.top -= lOffset;
	m_GroupValve.SetWindowPos( NULL, RectGroupValve.left, RectGroupValve.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	m_GroupValve.Invalidate();
	m_GroupValve.UpdateWindow();
	
	// Family.
	CRect RectDpFamily;
	m_ComboFamily.GetWindowRect( &RectDpFamily );
	ScreenToClient( RectDpFamily );
	RectDpFamily.top -= lOffset;
	m_ComboFamily.SetWindowPos( NULL, RectDpFamily.left, RectDpFamily.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	m_ComboFamily.Invalidate();
	m_ComboFamily.UpdateWindow();
	CRect RectFamilyStatic;
	GetDlgItem( IDC_STATICFAMILY )->GetWindowRect( &RectFamilyStatic );
	ScreenToClient( RectFamilyStatic );
	RectFamilyStatic.top -= lOffset;
	GetDlgItem( IDC_STATICFAMILY )->SetWindowPos( NULL, RectFamilyStatic.left, RectFamilyStatic.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	GetDlgItem( IDC_STATICFAMILY )->Invalidate();
	GetDlgItem( IDC_STATICFAMILY )->UpdateWindow();
	
	// Material.
	CRect RectMaterial;
	m_ComboMaterial.GetWindowRect( &RectMaterial );
	ScreenToClient( RectMaterial );
	RectMaterial.top -= lOffset;
	m_ComboMaterial.SetWindowPos( NULL, RectMaterial.left, RectMaterial.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	m_ComboMaterial.Invalidate();
	m_ComboMaterial.UpdateWindow();
	CRect RectMaterialStatic;
	GetDlgItem( IDC_STATICMATERIAL )->GetWindowRect( &RectMaterialStatic );
	ScreenToClient( RectMaterialStatic );
	RectMaterialStatic.top -= lOffset;
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowPos( NULL, RectMaterialStatic.left, RectMaterialStatic.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	GetDlgItem( IDC_STATICMATERIAL )->Invalidate();
	GetDlgItem( IDC_STATICMATERIAL )->UpdateWindow();
	
	// Connect.
	CRect RectConnect;
	m_ComboConnect.GetWindowRect( &RectConnect );
	ScreenToClient( RectConnect );
	RectConnect.top -= lOffset;
	m_ComboConnect.SetWindowPos( NULL, RectConnect.left, RectConnect.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	m_ComboConnect.Invalidate();
	m_ComboConnect.UpdateWindow();
	CRect RectConnectStatic;
	GetDlgItem( IDC_STATICCONNECT )->GetWindowRect( &RectConnectStatic );
	ScreenToClient( RectConnectStatic );
	RectConnectStatic.top -= lOffset;
	GetDlgItem( IDC_STATICCONNECT )->SetWindowPos( NULL, RectConnectStatic.left, RectConnectStatic.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	GetDlgItem( IDC_STATICCONNECT )->Invalidate();
	GetDlgItem( IDC_STATICCONNECT )->UpdateWindow();
	
	// Version.
	CRect RectVersion;
	m_ComboVersion.GetWindowRect( &RectVersion );
	ScreenToClient( RectVersion );
	RectVersion.top -= lOffset;
	m_ComboVersion.SetWindowPos( NULL, RectVersion.left, RectVersion.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	m_ComboVersion.Invalidate();
	m_ComboVersion.UpdateWindow();
	CRect RectVersionStatic;
	GetDlgItem( IDC_STATICVERSION )->GetWindowRect( &RectVersionStatic );
	ScreenToClient( RectVersionStatic );
	RectVersionStatic.top -= lOffset;
	GetDlgItem( IDC_STATICVERSION )->SetWindowPos( NULL, RectVersionStatic.left, RectVersionStatic.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	GetDlgItem( IDC_STATICVERSION )->Invalidate();
	GetDlgItem( IDC_STATICVERSION )->UpdateWindow();
	
	// PN.
	CRect RectPN;
	m_ComboPN.GetWindowRect( &RectPN );
	ScreenToClient( RectPN );
	RectPN.top -= lOffset;
	m_ComboPN.SetWindowPos( NULL, RectPN.left, RectPN.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	m_ComboPN.Invalidate();
	m_ComboPN.UpdateWindow();
	CRect RectPNstatic;
	GetDlgItem( IDC_STATICPN )->GetWindowRect( &RectPNstatic );
	ScreenToClient( RectPNstatic );
	RectPNstatic.top -= lOffset;
	GetDlgItem( IDC_STATICPN )->SetWindowPos( NULL, RectPNstatic.left, RectPNstatic.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	GetDlgItem( IDC_STATICPN )->Invalidate();
	GetDlgItem( IDC_STATICPN )->UpdateWindow();
	
	if( false == bUp )
	{
		lOffset = -lOffset;
	}
}

void CDlgIndSel6WayValve_SecondTab::_SetDpMax( double dDpMax )
{
	if( dDpMax > 0.0 )
	{
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax = dDpMax;
		m_clExtEditDpMax.SetCurrentValSI( dDpMax );
	}
	else
	{
		m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetWindowText( _T("") );
	}

	m_clExtEditDpMax.Update();
}
