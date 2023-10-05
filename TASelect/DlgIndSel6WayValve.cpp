#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Select.h"
#include "DlgNoDev6WValveFound.h"
#include "DlgConfSel.h"
#include "DlgTechParam.h"
#include "DlgLeftTabSelManager.h"

#include "DlgIndSelBase.h"
#include "DlgIndSelCtrlBase.h"
#include "ProductSelectionParameters.h"
#include "DlgIndSel6WayValve.h"
#include "DlgIndSel6WayValveCircuits.h"

#include "RViewSSelSS.h"
#include "RViewSSelCtrl.h"
#include "RViewSSel6WayValve.h"


CDlgIndSel6WayValve::CDlgIndSel6WayValve( CWnd *pParent )
	: CDlgIndSelCtrlBase( m_clIndSel6WayValveParams, CDlgIndSel6WayValve::IDD, pParent )
{
	// For 6Way.
	m_clIndSel6WayValveParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_6WayControlValve;
	m_clIndSel6WayValveParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	// For BV.
	m_clIndSel6WayValveParams.m_clIndSelBVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	m_clIndSel6WayValveParams.m_clIndSelBVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	// For PICV.
	m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_PressureIndepCtrlValve;
	m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_pclDlgIndSel6WayValve_6WayValveTab = NULL;
	m_pclDlgIndSel6WayValve_SecondTab = NULL;
}

CDlgIndSel6WayValve::~CDlgIndSel6WayValve()
{
	m_cl6WayValveCtrlTab.DestroyWindow();

	if( NULL != m_pclDlgIndSel6WayValve_6WayValveTab )
	{
		if( INVALID_HANDLE_VALUE != m_pclDlgIndSel6WayValve_6WayValveTab->GetSafeHwnd() )
		{
			m_pclDlgIndSel6WayValve_6WayValveTab->DestroyWindow();
		}
	
		delete m_pclDlgIndSel6WayValve_6WayValveTab;
		m_pclDlgIndSel6WayValve_6WayValveTab = NULL;
	}

	if( NULL != m_pclDlgIndSel6WayValve_SecondTab )
	{
		if( INVALID_HANDLE_VALUE != m_pclDlgIndSel6WayValve_SecondTab->GetSafeHwnd() )
		{
			m_pclDlgIndSel6WayValve_SecondTab->DestroyWindow();
		}
		
		delete m_pclDlgIndSel6WayValve_SecondTab;
		m_pclDlgIndSel6WayValve_SecondTab = NULL;
	}
}

void CDlgIndSel6WayValve::SetApplicationBackground( COLORREF cBackColor )
{
	CDlgIndSelCtrlBase::SetApplicationBackground( cBackColor );

	for( int i = 0; i < m_cl6WayValveCtrlTab.GetTabsNum(); i++ )
	{
		CDialogEx *pclDialogEx = dynamic_cast<CDialogEx *>( m_cl6WayValveCtrlTab.GetTabWnd( i ) );

		if( NULL == pclDialogEx )
		{
			ASSERT( 0 );
			continue;
		}

		pclDialogEx->SetBackgroundColor( cBackColor );
	}
}

void CDlgIndSel6WayValve::SaveSelectionParameters()
{
	CDlgIndSelCtrlBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSel6WayValveParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	if( NULL != m_pclDlgIndSel6WayValve_6WayValveTab )
	{
		m_pclDlgIndSel6WayValve_6WayValveTab->SaveSelectionParameters();
	}

	if( NULL != m_pclDlgIndSel6WayValve_SecondTab )
	{
		m_pclDlgIndSel6WayValve_SecondTab->SaveSelectionParameters();
	}

	pclIndSelParameter->Set6WValveSelectionMode( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode );
	pclIndSelParameter->Set6WValveCheckFastElectConnection( m_clIndSel6WayValveParams.m_bCheckFastConnection );
	// HYS-1877: Save set checkbox status.
	pclIndSelParameter->Set6WValveKitCheckbox( (int)m_clIndSel6WayValveParams.m_bOnlyForSet );

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSel6WayValve::UpdateWindow()
{
	CDlgIndSelCtrlBase::UpdateWindow();

	m_cl6WayValveCtrlTab.Invalidate(FALSE);
	m_cl6WayValveCtrlTab.UpdateWindow();

	if( NULL != m_pclDlgIndSel6WayValve_6WayValveTab )
	{
		m_pclDlgIndSel6WayValve_6WayValveTab->Invalidate( FALSE );
		m_pclDlgIndSel6WayValve_6WayValveTab->UpdateWindow();
	}
	
	if( NULL != m_pclDlgIndSel6WayValve_SecondTab )
	{
		m_pclDlgIndSel6WayValve_SecondTab->Invalidate( FALSE );
		m_pclDlgIndSel6WayValve_SecondTab->UpdateWindow();
	}
}

void CDlgIndSel6WayValve::UpdateTabTitle( int iTabID, e6WayValveSelectionMode eCircuit )
{
	if( e6Way_Alone == eCircuit )
	{
		m_cl6WayValveCtrlTab.ShowTab( iTabID, FALSE );
	}
	else if( e6Way_EQMControl == eCircuit || e6Way_OnOffControlWithPIBCV == eCircuit )
	{
		m_cl6WayValveCtrlTab.ShowTab( iTabID, TRUE );
		CString str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_PIBCV );
		m_cl6WayValveCtrlTab.SetTabLabel( iTabID, str );
		m_cl6WayValveCtrlTab.SetTabIcon( iTabID, CRCImageManager::ImgList6WayValveTabCtrl::IL6WV_PIBCv );
	}
	else if( e6Way_OnOffControlWithSTAD == eCircuit)
	{
		m_cl6WayValveCtrlTab.ShowTab( iTabID, TRUE );
		CString str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_BV );
		m_cl6WayValveCtrlTab.SetTabLabel( iTabID, str );
		m_cl6WayValveCtrlTab.SetTabIcon( iTabID, CRCImageManager::ImgList6WayValveTabCtrl::IL6WV_Bv );
	}
}

void CDlgIndSel6WayValve::OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgCOWaterChar *pclDlgWaterChar )
{
	CString strAdditiveFamilyID = _T( "" );

	if( NULL != pclDlgWaterChar )
	{
		strAdditiveFamilyID = pclDlgWaterChar->GetAdditiveFamilyID();
	}


	bEnable = true;
	strMsg = _T( "" );

	// HYS-1041: Disable suggest button when the additive is not a Glycol.
	// HYS-1111: 2019-06-05: Christian Thesing and Norbert Ramser allow alcohol for PAG products.
	if( 0 != StringCompare( strAdditiveFamilyID, _T("GLYCOL_ADDIT") )
			&& 0 != StringCompare( strAdditiveFamilyID, _T("ALCOHOL_ADDIT") )
			&& 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT") ) )
	{
		strMsg = TASApp.LoadLocalizedString( AFXMSG_BAD_ADDITIVE );
		bEnable = false;
	}
}

bool CDlgIndSel6WayValve::LeftTabKillFocus( bool bNext )
{
	// !!! Does not intentionally call the base class !!!

		bool bReturn = false;

	if( true == bNext )
	{
		// TAB -> must set the focus on the Dialog and set the focus on the first control.
		if( 0 == m_cl6WayValveCtrlTab.GetActiveTab() )
		{
			m_pclDlgIndSel6WayValve_6WayValveTab->SetFocus();
			m_pclDlgIndSel6WayValve_6WayValveTab->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::First );
		}
		else
		{
			m_pclDlgIndSel6WayValve_SecondTab->SetFocus();
			m_pclDlgIndSel6WayValve_SecondTab->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::First );
		}
		bReturn = true;
	}
	else
	{
		// SHIFT + TAB -> go to the top control only if right view is not empty.
		if( NULL != pRViewSSel6WayValve && TRUE == pRViewSSel6WayValve->IsWindowVisible() && false == pRViewSSel6WayValve->IsEmpty() )
		{
			// Set the focus on the right view.
			pRViewSSel6WayValve->SetFocus();
			// Set focus on the appropriate group in the right view.
			pRViewSSel6WayValve->SetFocusW( false );
			bReturn = true;
		}
		else
		{
			// Focus must be set on the last control of the Dialog.
			if( 0 == m_cl6WayValveCtrlTab.GetActiveTab() )
			{
				m_pclDlgIndSel6WayValve_6WayValveTab->SetFocus();
				m_pclDlgIndSel6WayValve_6WayValveTab->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::Last );
			}
			else
			{
				m_pclDlgIndSel6WayValve_SecondTab->SetFocus();
				m_pclDlgIndSel6WayValve_SecondTab->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::Last );
			}
			bReturn = true;
		}
	}

	return bReturn;
}

void CDlgIndSel6WayValve::OnRViewSSelLostFocusWithTabKey( bool bShiftPressed )
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
			SetFocus();
			// Set the focus on the first available edit control or last control.
			SetFocusOnControl( SetFocusWhere::First );
		}
		else
		{
			// Focus must be set on the last control of the Dialog 6-way valve left tab.
			if( 0 == m_cl6WayValveCtrlTab.GetActiveTab() )
			{
				m_pclDlgIndSel6WayValve_6WayValveTab->SetFocus();
				m_pclDlgIndSel6WayValve_6WayValveTab->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::Last );
			}
			else
			{
				m_pclDlgIndSel6WayValve_SecondTab->SetFocus();
				m_pclDlgIndSel6WayValve_SecondTab->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::Last );
			}
		}
	}
	else
	{
		CDlgIndSelCtrlBase::OnRViewSSelLostFocusWithTabKey( bShiftPressed );
	}
}

void CDlgIndSel6WayValve::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelCtrlBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTONSELECTION, m_BnSelectionMode );
	DDX_Control( pDX, IDC_STATICSELECTIONMODE, m_StaticSelectionMode );
	DDX_Control( pDX, IDC_GP_SELECTION, m_Group6WayValveSelection );
	DDX_Control( pDX, IDC_CHECKCHANGEOVER, m_CheckFastConnection );
	DDX_Control( pDX, IDC_CHECK6WVALVEKIT, m_CheckOnlyForSet );
	DDX_Control( pDX, IDC_STATICFLUID, m_StaticWater );
	DDX_Control( pDX, IDC_STATICTEMP_C, m_StaticCooling );
	DDX_Control( pDX, IDC_STATICTEMP_H, m_StaticHeating );
	DDX_Control( pDX, IDC_GROUPWATER, m_GroupWater );
	DDX_Control( pDX, IDC_BUTTONMODWATER, m_ButtonModWater );
}

BOOL CDlgIndSel6WayValve::OnInitDialog()
{
	CDlgIndSelCtrlBase::OnInitDialog();

	CString str;

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_SUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_SELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_CANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_SELECTION );
	m_BnSelectionMode.SetWindowText( str );

	m_StaticSelectionMode.SetTextAndToolTip( Get6WayValveSelectionModeStr( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode ) );

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL_6WCHANGEOVER );
	GetDlgItem( IDC_CHECKCHANGEOVER )->SetWindowText( str );

	// Set proper style and add icons for the 6-way scheme group.
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_GBCIRCUIT );
 	m_Group6WayValveSelection.SetWindowText( str );
 	m_Group6WayValveSelection.SetInOffice2007Mainframe( true );
	
	// Build new water characteristic strings.
	CString str1, str2;
	m_clIndSel6WayValveParams.m_CoolingWC.BuildWaterStrings( str1, str2 );
	
	// Set the text for the first static.
	SetDlgItemText( IDC_STATICFLUID, str1 );

	// Create the 6Way_second control with the good size.
	CRect rectCtrlTabContainer;
	GetDlgItem( IDC_GROUPPICVTAB )->GetClientRect( rectCtrlTabContainer );
	// Save the current secondTab size
	m_RCGroupPIBCV = rectCtrlTabContainer;
	CRect rectCtrlTab = rectCtrlTabContainer;
	rectCtrlTab.DeflateRect( 1, 1 );

	if( FALSE == m_cl6WayValveCtrlTab.Create( CMFCTabCtrl::STYLE_3D, rectCtrlTab, this, 1, CMFCTabCtrl::LOCATION_TOP, FALSE ) )
	{
		return FALSE;
	}

	// Position the ctrl tab.
	GetDlgItem( IDC_GROUPPICVTAB )->GetWindowRect( rectCtrlTabContainer );
	ScreenToClient( &rectCtrlTabContainer );
	m_cl6WayValveCtrlTab.SetWindowPos( NULL, rectCtrlTabContainer.left + 1, rectCtrlTabContainer.top + 7, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Some properties.
	m_cl6WayValveCtrlTab.SetDrawFrame( FALSE );
	m_cl6WayValveCtrlTab.SetTabBorderSize( 0 );

	// Add icons to tab ctrl.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_6WayValveTabCltr );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_cl6WayValveCtrlTab.SetImageList( pclImgListGroupBox->GetSafeHandle() );
	}

	// '6Way tab' creation.
	m_pclDlgIndSel6WayValve_6WayValveTab = new CDlgIndSel6WayValve_6WayValveTab( m_clIndSel6WayValveParams, this );

	if( NULL == m_pclDlgIndSel6WayValve_6WayValveTab )
	{
		return FALSE;
	}

	if( FALSE == m_pclDlgIndSel6WayValve_6WayValveTab->Create( CDlgIndSel6WayValve_6WayValveTab::IDD, &m_cl6WayValveCtrlTab ) )
	{
		delete m_pclDlgIndSel6WayValve_6WayValveTab;
		m_pclDlgIndSel6WayValve_6WayValveTab = NULL;
		return FALSE;
	}

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAY );
	m_cl6WayValveCtrlTab.AddTab( (CWnd *)m_pclDlgIndSel6WayValve_6WayValveTab, str, CRCImageManager::ImgList6WayValveTabCtrl::IL6WV_6WayValve, FALSE );
	m_pclDlgIndSel6WayValve_6WayValveTab->SetNotificationHandler( this );
	
	// 'CDlgIndSel6Way_PicvTab' creation.
	m_pclDlgIndSel6WayValve_SecondTab = new CDlgIndSel6WayValve_SecondTab( m_clIndSel6WayValveParams, this );

	if( NULL == m_pclDlgIndSel6WayValve_SecondTab )
	{
		m_pclDlgIndSel6WayValve_6WayValveTab->DestroyWindow();
		delete m_pclDlgIndSel6WayValve_6WayValveTab;
		m_pclDlgIndSel6WayValve_6WayValveTab = NULL;
		return FALSE;
	}

	if( FALSE == m_pclDlgIndSel6WayValve_SecondTab->Create( CDlgIndSel6WayValve_SecondTab::IDD, &m_cl6WayValveCtrlTab ) )
	{
		m_pclDlgIndSel6WayValve_6WayValveTab->DestroyWindow();
		delete m_pclDlgIndSel6WayValve_6WayValveTab;
		m_pclDlgIndSel6WayValve_6WayValveTab = NULL;
		delete m_pclDlgIndSel6WayValve_SecondTab;
		m_pclDlgIndSel6WayValve_SecondTab = NULL;
		return FALSE;
	}

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_CIRCUITWITTH );
	m_cl6WayValveCtrlTab.AddTab( (CWnd*)m_pclDlgIndSel6WayValve_SecondTab, str, CRCImageManager::ImgList6WayValveTabCtrl::IL6WV_PIBCv, FALSE );
	m_pclDlgIndSel6WayValve_SecondTab->SetNotificationHandler( this );
	
	m_cl6WayValveCtrlTab.SetActiveTab( eti6Way );
	UpdateTabTitle( 1, m_clIndSel6WayValveParams.m_e6WayValveSelectionMode );

	// Set proper style and add icons for groups.
	pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	// Add bitmap to "Water" button.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonModWater.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_ModifyWater ) );
	}

	if( NULL != pclImgListGroupBox )
	{
		// Add icon to water group.
		m_GroupWater.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Water );
	}
	
	m_GroupWater.SetInOffice2007Mainframe( true );

	// HYS-1877: Init checkbox static text.
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL_6WONLYFORSET );
	GetDlgItem( IDC_CHECK6WVALVEKIT )->SetWindowText( str );

	return TRUE;
}

BEGIN_MESSAGE_MAP( CDlgIndSel6WayValve, CDlgIndSelCtrlBase )
	ON_BN_CLICKED( IDC_BUTTONSELECTION, OnBnClickedSelectionMode )
	ON_BN_CLICKED( IDC_CHECKCHANGEOVER, OnBnClickedFastConnection )
	ON_BN_CLICKED( IDC_CHECK6WVALVEKIT, OnBnClickedOnlyForSet )
	ON_BN_CLICKED( IDC_BUTTONMODWATER, OnButtonModWater )
	ON_MESSAGE( WM_USER_MODIFYSELECTED6WAYVALVE, OnModifySelected6Way )
END_MESSAGE_MAP()

void CDlgIndSel6WayValve::OnBnClickedSelectionMode()
{
	bool bModified = false;
	CDlgIndSel6WayValveCircuits dDlg( &bModified, m_clIndSel6WayValveParams.m_e6WayValveSelectionMode );
	if( ( IDOK == dDlg.DoModal() ) && ( true == bModified ) )
	{	
		// HYS-1280: Only if the selection mode is changed
		ClearAll();
		m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = dDlg.GetSelectionMode6wayValve();
		m_StaticSelectionMode.SetTextAndToolTip( Get6WayValveSelectionModeStr( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode ) );
		
		// To disable checkbox if doesn't needed.
		_UpdateCheckFastElectConnection();
		// HYS-1877: Verify if checkbox for set can always be used.
		_UpdateCheckOnlyForSet();
		
		UpdateTabTitle( 1, m_clIndSel6WayValveParams.m_e6WayValveSelectionMode );
		
		m_pclDlgIndSel6WayValve_SecondTab->Update6WayTabCombo( CDlgIndSel6WayValve_SecondTab::UF_SelectionModeChanged );
		Invalidate();
		UpdateWindow();
	}
}

void CDlgIndSel6WayValve::OnBnClickedFastConnection()
{
	m_clIndSel6WayValveParams.m_bCheckFastConnection = ( BST_CHECKED == m_CheckFastConnection.GetCheck() ) ? true : false;
	m_pclDlgIndSel6WayValve_SecondTab->Update6WayTabCombo( CDlgIndSel6WayValve_SecondTab::UF_FastEelectricalConnectionChanged );

	// HYS-1877: Verify if checkbox for set can always be used.
	_UpdateCheckOnlyForSet();
	
	ClearAll();
}

void CDlgIndSel6WayValve::OnBnClickedOnlyForSet()
{
	m_clIndSel6WayValveParams.m_bOnlyForSet = ( BST_CHECKED == m_CheckOnlyForSet.GetCheck() ) ? true : false;
	
	// Update combo boxes accordingly to the new value of the checkbox
	m_pclDlgIndSel6WayValve_6WayValveTab->FillComboConnection();
	m_pclDlgIndSel6WayValve_6WayValveTab->FillComboVersion();
	m_pclDlgIndSel6WayValve_SecondTab->Update6WayTabCombo( CDlgIndSel6WayValve_SecondTab::UF_Other );
	ClearAll();
}

void CDlgIndSel6WayValve::OnButtonModWater()
{
	// Display the dialog.
	CDlgCOWaterChar dlg;
	dlg.Display( NULL, WMUserWaterCharWParam::WM_UWC_WP_ForProductSel );
}

void CDlgIndSel6WayValve::I6WayNotification_EnableSelectButton( bool bEnable )
{
	EnableSelectButton( bEnable );
}

void CDlgIndSel6WayValve::I6WayNotification_OnEditEnterChar( NMHDR * pNMHDR )
{
	if( NULL == GetpBtnSuggest() || ( WS_DISABLED == ( WS_DISABLED & GetpBtnSuggest()->GetStyle() ) ) )
	{
		return;
	}

	// To force the current edit to lost focus (And then allow it to save value input by user).
	GetpBtnSuggest()->SetFocus();

	// Reset Focus on the modified Edit.
	CWnd::FromHandle( pNMHDR->hwndFrom )->SetFocus();

	PostWMCommandToControl( GetpBtnSuggest() );
}

void CDlgIndSel6WayValve::I6WayNotification_ClearAll()
{
	ClearAll();
}

void CDlgIndSel6WayValve::I6WayNotification_SetFocus()
{
	SetFocus();
}

void CDlgIndSel6WayValve::I6WayNotification_SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere ePosition )
{
	SetFocusOnControl( ePosition );
}

void CDlgIndSel6WayValve::IPIBCvBvNotification_EnableSelectButton( bool bEnable )
{
	EnableSelectButton( bEnable );
}

void CDlgIndSel6WayValve::IPIBCvBvNotification_OnEditEnterChar( NMHDR * pNMHDR )
{
	if( NULL == GetpBtnSuggest() || ( WS_DISABLED == ( WS_DISABLED & GetpBtnSuggest()->GetStyle() ) ) )
	{
		return;
	}

	// To force the current edit to lost focus (And then allow it to save value input by user).
	GetpBtnSuggest()->SetFocus();

	// Reset Focus on the modified Edit.
	CWnd::FromHandle( pNMHDR->hwndFrom )->SetFocus();

	PostWMCommandToControl( GetpBtnSuggest() );

}

void CDlgIndSel6WayValve::IPIBCvBvNotification_ClearAll()
{
	ClearAll();
}

void CDlgIndSel6WayValve::IPIBCvBvNotification_ResizeSecondTab( long &lOffset, bool bReduce )
{
	CRect CurrentClientRect;

	if( false == bReduce )
	{
		lOffset = -lOffset;
	}

	GetDlgItem( IDC_GROUPPICVTAB )->GetClientRect( &CurrentClientRect );
	CurrentClientRect.bottom -= lOffset;
	GetDlgItem( IDC_GROUPPICVTAB )->SetWindowPos( NULL, -1, -1, CurrentClientRect.Width(), CurrentClientRect.Height(), SWP_NOMOVE | SWP_NOACTIVATE );
	GetDlgItem( IDC_GROUPPICVTAB )->Invalidate();
	GetDlgItem( IDC_GROUPPICVTAB )->UpdateWindow();

	if( false == bReduce )
	{
		lOffset = -lOffset;
	}
}

void CDlgIndSel6WayValve::IPIBCvBvNotification_SetFocus()
{
	SetFocus();
}

void CDlgIndSel6WayValve::IPIBCvBvNotification_SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere ePosition )
{
	SetFocusOnControl( ePosition );
}

bool CDlgIndSel6WayValve::IPIBCvBvNotification_IsPIBCVGroupResized()
{
	CRect RectCurrentSize;
	GetDlgItem( IDC_GROUPPICVTAB )->GetClientRect( RectCurrentSize );

	return ( RectCurrentSize != m_RCGroupPIBCV );
}

LRESULT CDlgIndSel6WayValve::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnNewDocument( wParam, lParam );
	ClearCommonInputEditors();

	if( false == TASApp.Is6WayCVDisplayed() )
	{
		return 0;
	}

	// Get last selected parameters.
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSel6WayValveParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	// HYS-1429 : Check availability of selection mode
	bool bIsEQMEnabled = false;
	bool bIsOnOffPibcvEnabled = false;
	bool bIsOnOffbvEnabled = false;
	bool bIs6WayValveEnabled = false;
	bool bIsPackageEnabled = false;
	// HYS-1877: Add Set checkbox availability.
	_CheckProductAvailability( bIsEQMEnabled, bIsOnOffPibcvEnabled, bIsOnOffbvEnabled, bIs6WayValveEnabled, bIsPackageEnabled );
	if( ( true == bIsEQMEnabled && true == bIsOnOffPibcvEnabled && true == bIsOnOffbvEnabled && true == bIs6WayValveEnabled )
		|| ( ( true == bIsEQMEnabled && e6Way_EQMControl == pclIndSelParameter->Get6WValveSelectionMode() ) ||
		( true == bIsOnOffPibcvEnabled && e6Way_OnOffControlWithPIBCV == pclIndSelParameter->Get6WValveSelectionMode() ) ||
		( true == bIsOnOffbvEnabled && e6Way_OnOffControlWithSTAD == pclIndSelParameter->Get6WValveSelectionMode() ) ||
		( true == bIs6WayValveEnabled && e6Way_Alone == pclIndSelParameter->Get6WValveSelectionMode() ) ) )
	{
		m_StaticSelectionMode.SetTextAndToolTip( Get6WayValveSelectionModeStr( pclIndSelParameter->Get6WValveSelectionMode() ) );
		m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = pclIndSelParameter->Get6WValveSelectionMode();

		if( true == bIsEQMEnabled && false == bIsOnOffPibcvEnabled && false == pclIndSelParameter->Get6WValveCheckFastConnection() )
		{
			m_clIndSel6WayValveParams.m_bCheckFastConnection = true;
		}
		else
		{
			m_clIndSel6WayValveParams.m_bCheckFastConnection = pclIndSelParameter->Get6WValveCheckFastConnection();
		}
	}
	else
	{
		// reset
		if( true == bIsEQMEnabled )
		{
			m_StaticSelectionMode.SetTextAndToolTip( Get6WayValveSelectionModeStr( e6Way_EQMControl ) );
			m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = e6Way_EQMControl;
			m_clIndSel6WayValveParams.m_bCheckFastConnection = true;
		}
		else if( true == bIs6WayValveEnabled )
		{
			m_StaticSelectionMode.SetTextAndToolTip( Get6WayValveSelectionModeStr( e6Way_Alone ) );
			m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = e6Way_Alone;
			m_clIndSel6WayValveParams.m_bCheckFastConnection = false;
		}

	}

	int iCheck = -1;
	if( true == m_clIndSel6WayValveParams.m_bCheckFastConnection )
	{
		iCheck = BST_CHECKED;
	}
	else
	{
		iCheck = BST_UNCHECKED;
	}

	m_CheckFastConnection.SetCheck( iCheck );

	_UpdateCheckFastElectConnection();

	// HYS-1877: Set check box status.
	if( true == bIsPackageEnabled && e6Way_Alone != m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
	{
		m_CheckOnlyForSet.EnableWindow( TRUE );
		m_CheckOnlyForSet.SetCheck( pclIndSelParameter->Get6WValveKitCheckbox() );
	}
	else
	{
		m_CheckOnlyForSet.EnableWindow( FALSE );
	}
	_UpdateCheckOnlyForSet();

	// Water
	// Set values for water char group
	_Update6WayValveWaterChar();
	_SetStaticFluidTemp();

	UpdateTabTitle( 1, m_clIndSel6WayValveParams.m_e6WayValveSelectionMode );
	
	// This is needed only in the case of the dialog is currently active when user creates a new document (or opens a new one).
	EnableSelectButton( false );
	m_bInitialised = true;

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}

LRESULT CDlgIndSel6WayValve::OnModifySelected6Way( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSel6WayValve *pSel6Way = dynamic_cast<CDS_SSel6WayValve *>( (CData *)lParam );
	ASSERT( NULL != pSel6Way );

	if( NULL == pSel6Way || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSel6WayValveParams.m_SelIDPtr = pSel6Way->GetIDPtr();

	m_bInitialised = false;

	// Send message to notify the display of the SSel tab.
	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}
	
	// HYS-1242: SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );
	
	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_6WayControlValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSel6WayValveParams.m_eApplicationType = pSel6Way->GetpSelectedInfos()->GetApplicationType();

	// Transfer water characteristics.
	m_clIndSel6WayValveParams.m_WC = *( pSel6Way->GetpSelectedInfos()->GetpWCData() );

	m_clIndSel6WayValveParams.m_CoolingWC = m_clIndSel6WayValveParams.m_WC;
	m_clIndSel6WayValveParams.m_HeatingWC = *( m_clIndSel6WayValveParams.m_CoolingWC.GetSecondWaterChar() );

	CWaterChar *pTechDataWC = TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData();
	*pTechDataWC = m_clIndSel6WayValveParams.m_WC;

	CDS_TechnicalParameter *pclTechParams = TASApp.GetpTADS()->GetpTechParams();
	pclTechParams->SetDefaultISChangeOverTps( m_clIndSel6WayValveParams.m_HeatingWC.GetTemp(), HeatingSide );
	pclTechParams->SetDefaultISChangeOverDT( pSel6Way->GetpSelectedInfos()->GetHeatingDT(), HeatingSide );

	pclTechParams->SetDefaultISChangeOverTps( m_clIndSel6WayValveParams.m_CoolingWC.GetTemp(), CoolingSide );
	pclTechParams->SetDefaultISChangeOverDT( pSel6Way->GetpSelectedInfos()->GetCoolingDT(), CoolingSide );
	pclTechParams->SetDefaultISChangeOverAdditID( m_clIndSel6WayValveParams.m_WC.GetAdditID(), CoolingSide );
	pclTechParams->SetDefaultISChangeOverPcWeight( m_clIndSel6WayValveParams.m_WC.GetPcWeight(), CoolingSide );


	// Set fluid string
	CString str1, str2;
	m_clIndSel6WayValveParams.m_CoolingWC.BuildWaterStrings( str1, str2 );
	
	// Set the text for the first static.
	SetDlgItemText( IDC_STATICFLUID, str1 );
	_SetStaticFluidTemp();
	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );

	// Update ribbon.
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Selection mode.
	m_clIndSel6WayValveParams.m_e6WayValveSelectionMode = (e6WayValveSelectionMode)pSel6Way->GetSelectionMode();
	m_StaticSelectionMode.SetTextAndToolTip( Get6WayValveSelectionModeStr( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode ) );

	m_pclDlgIndSel6WayValve_SecondTab->Update6WayTabCombo( CDlgIndSel6WayValve_SecondTab::UF_ProductEdited );

	// Fast electrical connection.
	int iCheck = -1;
	iCheck = pSel6Way->GetCheckboxFastConnection();

	if( 1 == iCheck )
	{
		iCheck = BST_CHECKED;
		m_clIndSel6WayValveParams.m_bCheckFastConnection = true;
	}
	else
	{
		iCheck = BST_UNCHECKED;
		m_clIndSel6WayValveParams.m_bCheckFastConnection = false;
	}

	m_CheckFastConnection.SetCheck( iCheck );
	
	// HYS-1877: Only for set check box
	iCheck = -1;
	iCheck = pSel6Way->GetCheckboxOnlyForSet();

	if( 1 == iCheck )
	{
		iCheck = BST_CHECKED;
		m_clIndSel6WayValveParams.m_bOnlyForSet = true;
	}
	else
	{
		iCheck = BST_UNCHECKED;
		m_clIndSel6WayValveParams.m_bOnlyForSet = false;
	}

	m_CheckOnlyForSet.SetCheck( iCheck );

	if( NULL != m_pclDlgIndSel6WayValve_6WayValveTab )
	{
		m_pclDlgIndSel6WayValve_6WayValveTab->Set6WayValveTabParameters( pSel6Way );
	}

	if( NULL != m_pclDlgIndSel6WayValve_SecondTab )
	{
		m_pclDlgIndSel6WayValve_SecondTab->SetSecondTabParameters( pSel6Way );
	}

	UpdateTabTitle( 1, m_clIndSel6WayValveParams.m_e6WayValveSelectionMode );

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );

	// This function use m_bEditModeRunning
	_UpdateCheckFastElectConnection();
	// HYS-1877
	_UpdateCheckOnlyForSet();

	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

LRESULT CDlgIndSel6WayValve::OnPipeChange( WPARAM wParam, LPARAM lParam )
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

LRESULT CDlgIndSel6WayValve::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	return 0;
}

LRESULT CDlgIndSel6WayValve::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnUnitChange( wParam, lParam );

	_SetStaticFluidTemp();

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	return 0;
}

LRESULT CDlgIndSel6WayValve::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: For change over to consider changes we should have lParam = WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) 
		|| lParam == WMUserWaterCharLParam::WM_UWC_LWP_NoChange || lParam == WMUserWaterCharLParam::WM_UWC_LWP_Change
		|| lParam == WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnWaterChange( wParam, lParam );

	// Set values for water char group.
	if( this == pDlgLeftTabSelManager->GetCurrentLeftTabDialog() )
	{
		CWaterChar clWC = *( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() );
		m_clIndSel6WayValveParams.m_CoolingWC = clWC;
		m_clIndSel6WayValveParams.m_HeatingWC = *( clWC.GetSecondWaterChar() );
		
		// Set fluid string.
		CString str1, str2;
		m_clIndSel6WayValveParams.m_CoolingWC.BuildWaterStrings( str1, str2 );
		
		// Set the text for the first static.
		SetDlgItemText( IDC_STATICFLUID, str1 );
		_SetStaticFluidTemp();
		
		// If (PDT) DT field is not modified by the user update it with DT of water char .
		m_pclDlgIndSel6WayValve_6WayValveTab->Update6WayValveTabDTField();

		if( true == m_clIndSel6WayValveParams.m_bEditModeRunning )
		{
			EnableSelectButton( true );
		}

		ClearAll();
	}

	return 0;
}

void CDlgIndSel6WayValve::OnBnClickedSuggest()
{
	if( NULL == pRViewSSel6WayValve )
	{
		return;
	}
	
	if( NULL == m_pclDlgIndSel6WayValve_6WayValveTab || NULL == m_pclDlgIndSel6WayValve_SecondTab )
	{
		return;
	}

	if( false == m_bInitialised )
	{
		return;
	}

	if( m_clIndSel6WayValveParams.m_dHeatingFlow <= 0.0 || m_clIndSel6WayValveParams.m_dCoolingFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		return;
	}
	
	// Check if Dp is valid or not (or check box is checked but there is no value).
	if( e6Way_OnOffControlWithSTAD == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode 
			&& CDlgIndSelBase::VDPFlag_DpError == m_pclDlgIndSel6WayValve_SecondTab->VerifyDpValue() )
	{
		return;
	}

	if( ( e6Way_OnOffControlWithPIBCV == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode || e6Way_EQMControl == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
			&& m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_dDpMax <= 0.0 )
	{
		if( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_bIsDpMaxChecked = true )
		{
			m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_bIsDpMaxChecked = false;
			m_pclDlgIndSel6WayValve_SecondTab->UpdateDpMaxFieldState();
		}
	}

	ClearAll();
	
	// Create the Select PICVList to verify if at least one device exist.
	if( NULL != m_clIndSel6WayValveParams.m_pclSelect6WayValveList )
	{
		delete m_clIndSel6WayValveParams.m_pclSelect6WayValveList;
	}

	m_clIndSel6WayValveParams.m_pclSelect6WayValveList = new CSelect6WayValveList();

	if( NULL == m_clIndSel6WayValveParams.m_pclSelect6WayValveList || NULL == m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetSelectPipeList() )
	{
		return;
	}

	CWaterChar *pclWaterChar = NULL;
	double dBiggestFlow = 0.0;

	// To size pipe we need the biggest flow but also the corresponding fluid characteristic.
	if( m_clIndSel6WayValveParams.GetCoolingFlow() >= m_clIndSel6WayValveParams.GetHeatingFlow() )
	{
		dBiggestFlow = m_clIndSel6WayValveParams.GetCoolingFlow();
		pclWaterChar = &m_clIndSel6WayValveParams.GetCoolingWaterChar();
	}
	else
	{
		// Normally this case will never appear.
		dBiggestFlow = m_clIndSel6WayValveParams.GetHeatingFlow();
		pclWaterChar = &m_clIndSel6WayValveParams.GetHeatingWaterChar();
	}

	m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetSelectPipeList()->SelectPipes( &m_clIndSel6WayValveParams,
			max( m_clIndSel6WayValveParams.GetCoolingFlow(), m_clIndSel6WayValveParams.GetHeatingFlow() ),  true, true, 0, 0, pclWaterChar );

	m_clIndSel6WayValveParams.m_e6WayValveList.PurgeAll();

	int iValveCount = m_clIndSel6WayValveParams.m_pTADB->GetTaCVList(
			&m_clIndSel6WayValveParams.m_e6WayValveList,												// List where to saved
			CTADatabase::eFor6WayCV,								// Control valve target (cv, hmcv, picv or bcv)
			false, 													// 'true' returns as soon a result is found
			m_clIndSel6WayValveParams.m_eCV2W3W, 							// Set way number of valve
			_T( "" ),							 						// Type ID
			_T( "" ),			// Family ID
			_T( "" ),		// Body material ID
			(LPCTSTR)m_clIndSel6WayValveParams.m_strComboConnectID,		// Connection ID HYS-1252 : Now we use connectID instead of familyID
			(LPCTSTR)m_clIndSel6WayValveParams.m_strComboVersionID, 		// Version ID
			_T( "" ),				// PN ID
			CDB_ControlProperties::ControlOnly, 						// Set the control function (control only, presettable, ...)
			m_clIndSel6WayValveParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSel6WayValveParams.m_eFilterSelection, 0, INT_MAX,
		    false, NULL, m_clIndSel6WayValveParams.m_bOnlyForSet ); // HYS-1877: Consider the fact that we looking for a set or not.

	bool bSizeShiftProblem = false;
	bool bTryAlternative = false;

	if( 0 == iValveCount )
	{
		bTryAlternative = true;
	}
	else
	{
		int iDevFound = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->Select6WayValve( &m_clIndSel6WayValveParams, &bSizeShiftProblem );

		if( iDevFound > 0 && false == bSizeShiftProblem )
		{
			VerifyModificationMode();
			pRViewSSel6WayValve->Suggest( &m_clIndSel6WayValveParams );
		}
		else
		{
			bTryAlternative = true;
		}
	}

	if( true == bTryAlternative )
	{
		if( true == _TryAlternativeSuggestion( bSizeShiftProblem ) )
		{
			CDlgTechParam *pdlg = new CDlgTechParam( CDlgTechParam::PageGeneralDeviceSizes );

			if( NULL == pdlg )
			{
				return;
			}

			if( IDOK != pdlg->DoModal() )
			{
				// Set the focus on the first available edit control.
				SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
			}

			delete pdlg;
		}
		else
		{
			// Set the focus on the first available edit control.
			SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
		}
	}
}

void CDlgIndSel6WayValve::OnBnClickedSelect()
{
	try
	{
		if( NULL == m_clIndSel6WayValveParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSel6WayValveParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSel6WayValveParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSel6WayValveParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSel6WayValve || FALSE == this->IsWindowVisible() )
		{
			return;
		}

		// Verify pointer validity; user can delete object after entering in Edition.
		if( _T( '\0' ) != *m_clIndSel6WayValveParams.m_SelIDPtr.ID )
		{
			m_clIndSel6WayValveParams.m_SelIDPtr = m_clIndSel6WayValveParams.m_pTADS->Get( m_clIndSel6WayValveParams.m_SelIDPtr.ID );
		}

		// Create selected object and initialize it.
		CDS_SSel6WayValve *pSel6WayValve = dynamic_cast<CDS_SSel6WayValve *>( m_clIndSel6WayValveParams.m_SelIDPtr.MP );

		if( NULL == pSel6WayValve )
		{
			IDPTR IDPtr;
			m_clIndSel6WayValveParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSel6WayValve ) );
			pSel6WayValve = (CDS_SSel6WayValve *)IDPtr.MP;
		}

		if( NULL == pSel6WayValve )
		{
			return;
		}

		if( m_clIndSel6WayValveParams.m_dHeatingFlow <= 0.0 )
		{
			HYSELECT_THROW( _T("Internal error: Heating flow '%f' can't be lower or equal to 0."), m_clIndSel6WayValveParams.m_dHeatingFlow );
		}
		else if( m_clIndSel6WayValveParams.m_dCoolingFlow <= 0.0 )
		{
			HYSELECT_THROW( _T("Internal error: Cooling flow '%f' can't be lower or equal to 0."), m_clIndSel6WayValveParams.m_dCoolingFlow );
		}

		pSel6WayValve->SetCheckboxFastConnection( m_clIndSel6WayValveParams.m_bCheckFastConnection );
		// HYS-1877
		pSel6WayValve->SetCheckboxOnlyForSet( m_clIndSel6WayValveParams.m_bOnlyForSet );

		pSel6WayValve->SetCoolingFlow( m_clIndSel6WayValveParams.m_dCoolingFlow );
		pSel6WayValve->SetHeatingFlow( m_clIndSel6WayValveParams.m_dHeatingFlow );

		if( CDS_SelProd::efdFlow == m_clIndSel6WayValveParams.m_eFlowOrPowerDTMode )
		{
			pSel6WayValve->SetFlowDef( CDS_SelProd::efdFlow );
			pSel6WayValve->SetHeatingPower( 0.0 );
			pSel6WayValve->SetCoolingPower( 0.0 );
			pSel6WayValve->SetHeatingDT( 0.0 );
			pSel6WayValve->SetCoolingDT( 0.0 );
		}
		else
		{
			pSel6WayValve->SetFlowDef( CDS_SelProd::efdPower );
			pSel6WayValve->SetHeatingPower( m_clIndSel6WayValveParams.m_dHeatingPower );
			pSel6WayValve->SetCoolingPower( m_clIndSel6WayValveParams.m_dCoolingPower );
			pSel6WayValve->SetHeatingDT( m_clIndSel6WayValveParams.m_dHeatingDT );
			pSel6WayValve->SetCoolingDT( m_clIndSel6WayValveParams.m_dCoolingDT );
		}
	
		pSel6WayValve->SetSelectionMode( m_clIndSel6WayValveParams.m_e6WayValveSelectionMode );
		// HYS-1877
		pSel6WayValve->SetSelectedAsAPackage( m_clIndSel6WayValveParams.m_bOnlyForSet );
		pSel6WayValve->SetCtrlProp( m_clIndSel6WayValveParams.m_eCV2W3W );
		pSel6WayValve->SetTypeID( m_clIndSel6WayValveParams.m_strComboTypeID );
		pSel6WayValve->SetCtrlType( m_clIndSel6WayValveParams.m_eCvCtrlType );
		pSel6WayValve->SetFamilyID( m_clIndSel6WayValveParams.m_strComboFamilyID );
		pSel6WayValve->SetMaterialID( m_clIndSel6WayValveParams.m_strComboMaterialID );
		pSel6WayValve->SetConnectID( m_clIndSel6WayValveParams.m_strComboConnectID );
		pSel6WayValve->SetVersionID( m_clIndSel6WayValveParams.m_strComboVersionID );
		pSel6WayValve->SetPNID( m_clIndSel6WayValveParams.m_strComboPNID );

		// Set the actuators info.
		pSel6WayValve->SetActuatorInputSignalID( m_clIndSel6WayValveParams.m_strActuatorInputSignalID );

		pSel6WayValve->SetPipeSeriesID( m_clIndSel6WayValveParams.m_strPipeSeriesID );
		pSel6WayValve->SetPipeID( m_clIndSel6WayValveParams.m_strPipeID );

		pSel6WayValve->SetCoolingWC( m_clIndSel6WayValveParams.GetCoolingWaterChar() );
		*pSel6WayValve->GetpSelectedInfos()->GetpWCData() = m_clIndSel6WayValveParams.m_WC;
		pSel6WayValve->SetHeatingWC( m_clIndSel6WayValveParams.GetHeatingWaterChar() );
	
		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSel6WayValve->GetpSelectedInfos()->SetDT( m_clIndSel6WayValveParams.m_pTADS->GetpTechParams()->GetDefaultISChangeOverDT( CoolingSide ) );
		pSel6WayValve->GetpSelectedInfos()->SetCoolingDT( m_clIndSel6WayValveParams.m_pTADS->GetpTechParams()->GetDefaultISChangeOverDT( CoolingSide ) );
		pSel6WayValve->GetpSelectedInfos()->SetHeatingDT( m_clIndSel6WayValveParams.m_pTADS->GetpTechParams()->GetDefaultISChangeOverDT( HeatingSide ) );
	
		// For the same reason, we also save the application type.
		pSel6WayValve->GetpSelectedInfos()->SetApplicationType( m_clIndSel6WayValveParams.m_eApplicationType );

		if( e6Way_OnOffControlWithSTAD == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
		{
			// Save params for BV.
			if( NULL == m_clIndSel6WayValveParams.m_clIndSelBVParams.m_pTADS )
			{
				HYSELECT_THROW( _T("Internal error: 'm_clIndSel6WayValveParams.m_clIndSelBVParams.m_pTADS' argument can't be NULL.") );
			}
			else if( NULL == m_clIndSel6WayValveParams.m_clIndSelBVParams.m_pTADS->GetpTechParams() )
			{
				HYSELECT_THROW( _T("Internal error: 'm_clIndSel6WayValveParams.m_clIndSelBVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
			}
		
			CDS_SSelBv *pSelBvH = pSel6WayValve->GetCDSSSelBv( SideDefinition::HeatingSide );
			CDS_SSelBv *pSelBvC = pSel6WayValve->GetCDSSSelBv( SideDefinition::CoolingSide );

			if( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_dDp > 0.0 )
			{
				pSelBvH->SetDp( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_dDp );
				pSelBvC->SetDp( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_dDp );
			}
			else
			{
				pSelBvH->SetDp( 0.0 );
				pSelBvC->SetDp( 0.0 );
			}

			// HYS-1877
			pSelBvH->SetSelectedAsAPackage( m_clIndSel6WayValveParams.m_bOnlyForSet );
			pSelBvH->SetTypeID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboTypeID );
			pSelBvH->SetFamilyID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboFamilyID );
			pSelBvH->SetMaterialID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboMaterialID );
			pSelBvH->SetConnectID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboConnectID );
			pSelBvH->SetVersionID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboVersionID );
			pSelBvH->SetPNID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboPNID );
			pSelBvH->SetPipeSeriesID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strPipeSeriesID );
			pSelBvH->SetPipeID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strPipeID );

			// HYS-1877
			pSelBvC->SetSelectedAsAPackage( m_clIndSel6WayValveParams.m_bOnlyForSet );
			pSelBvC->SetTypeID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboTypeID );
			pSelBvC->SetFamilyID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboFamilyID );
			pSelBvC->SetMaterialID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboMaterialID );
			pSelBvC->SetConnectID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboConnectID );
			pSelBvC->SetVersionID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboVersionID );
			pSelBvC->SetPNID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strComboPNID );
			pSelBvC->SetPipeSeriesID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strPipeSeriesID );
			pSelBvC->SetPipeID( m_clIndSel6WayValveParams.m_clIndSelBVParams.m_strPipeID );
		}
		else if( e6Way_OnOffControlWithPIBCV == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
		{
			// Save params for PIBCv.
			if( NULL == m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_pTADS )
			{
				HYSELECT_THROW( _T("Internal error: 'm_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_pTADS' argument can't be NULL.") );
			}
			else if( NULL == m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_pTADS->GetpTechParams() )
			{
				HYSELECT_THROW( _T("Internal error: 'm_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
			}

			CDS_SSelPICv *pSelPICvH = pSel6WayValve->GetCDSSSelPICv( SideDefinition::HeatingSide );
			CDS_SSelPICv *pSelPICvC = pSel6WayValve->GetCDSSSelPICv( SideDefinition::CoolingSide );

			pSelPICvC->SetDpMax( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_dDpMax );

			// HYS-1877
			pSelPICvC->SetSelectedAsAPackage( m_clIndSel6WayValveParams.m_bOnlyForSet );

			pSelPICvC->SetTypeID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboTypeID );
			pSelPICvC->SetCtrlType( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_eCvCtrlType );
			pSelPICvC->SetFamilyID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboFamilyID );
			pSelPICvC->SetMaterialID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboMaterialID );
			pSelPICvC->SetConnectID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboConnectID );
			pSelPICvC->SetVersionID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboVersionID );
			pSelPICvC->SetPNID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboPNID );

			// Set the actuators info.
			pSelPICvC->SetActuatorPowerSupplyID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID );
			pSelPICvC->SetActuatorInputSignalID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strActuatorInputSignalID );

			pSelPICvC->SetPipeSeriesID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strPipeSeriesID );
			pSelPICvC->SetPipeID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strPipeID );

			pSelPICvH->SetDpMax( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_dDpMax );

			// HYS-1877
			pSelPICvH->SetSelectedAsAPackage( m_clIndSel6WayValveParams.m_bOnlyForSet );

			pSelPICvH->SetTypeID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboTypeID );
			pSelPICvH->SetCtrlType( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_eCvCtrlType );
			pSelPICvH->SetFamilyID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboFamilyID );
			pSelPICvH->SetMaterialID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboMaterialID );
			pSelPICvH->SetConnectID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboConnectID );
			pSelPICvH->SetVersionID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboVersionID );
			pSelPICvH->SetPNID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboPNID );

			// Set the actuators info.
			pSelPICvH->SetActuatorPowerSupplyID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID );
			pSelPICvH->SetActuatorInputSignalID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strActuatorInputSignalID );

			pSelPICvH->SetPipeSeriesID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strPipeSeriesID );
			pSelPICvH->SetPipeID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strPipeID );
		}
		else if( e6Way_EQMControl == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
		{
			// Save params for PIBCv.
			if( NULL == m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_pTADS )
			{
				HYSELECT_THROW( _T("Internal error: 'm_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_pTADS' argument can't be NULL.") );
			}
			else if( NULL == m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_pTADS->GetpTechParams() )
			{
				HYSELECT_THROW( _T("Internal error: 'm_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
			}

			CDS_SSelPICv *pSelPICv = pSel6WayValve->GetCDSSSelPICv( SideDefinition::BothSide );

			pSelPICv->SetDpMax( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_dDpMax );

			// HYS-1877
			pSelPICv->SetSelectedAsAPackage( m_clIndSel6WayValveParams.m_bOnlyForSet );

			pSelPICv->SetTypeID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboTypeID );
			pSelPICv->SetCtrlType( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_eCvCtrlType );
			pSelPICv->SetFamilyID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboFamilyID );
			pSelPICv->SetMaterialID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboMaterialID );
			pSelPICv->SetConnectID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboConnectID );
			pSelPICv->SetVersionID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboVersionID );
			pSelPICv->SetPNID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strComboPNID );

			// Set the actuators info.
			pSelPICv->SetActuatorPowerSupplyID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID );
			pSelPICv->SetActuatorInputSignalID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strActuatorInputSignalID );

			pSelPICv->SetPipeSeriesID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strPipeSeriesID );
			pSelPICv->SetPipeID( m_clIndSel6WayValveParams.m_clIndSelPIBCVParams.m_strPipeID );
		}
	
		pRViewSSel6WayValve->FillInSelected( pSel6WayValve );

		CDlgConfSel dlg( &m_clIndSel6WayValveParams );
		IDPTR IDPtr = pSel6WayValve->GetIDPtr();

		dlg.Display( pSel6WayValve );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSel6WayValveParams.m_pTADS->Get( _T("6WAYCTRLVALV_TAB") ).MP );
			ASSERT( NULL != pTab );

			if( NULL != pTab && _T('\0') == *m_clIndSel6WayValveParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSel6WayValveParams.m_pTADS->SetNewIndex( CLASS( CDS_SSel6WayValve ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSel6WayValveParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable Select Button
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSel6WayValveParams.m_SelIDPtr.ID )
			{
				m_clIndSel6WayValveParams.m_pTADS->DeleteObject( IDPtr );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSel6WayValve::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

CRViewSSelSS *CDlgIndSel6WayValve::GetLinkedRightViewSSel( void )
{
	return pRViewSSel6WayValve;
}

CString CDlgIndSel6WayValve::Get6WayValveSelectionModeStr( e6WayValveSelectionMode eCircuitType )
{
	
	CString str = CteEMPTY_STRING;

	switch( eCircuitType )
	{
		case e6WayValveSelectionMode::e6Way_Alone:

			str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYALONE );
			break;

		case  e6WayValveSelectionMode::e6Way_EQMControl:

			str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYEQM );
			break;

		case e6WayValveSelectionMode::e6Way_OnOffControlWithPIBCV:

			str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYONOFFPICV );
			break;

		case e6WayValveSelectionMode::e6Way_OnOffControlWithSTAD:

			str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYONOFFSTAD );
			break;

		default:
			break;
	}

	return str;
}

void CDlgIndSel6WayValve::_Update6WayValveWaterChar()
{
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	CWaterChar clWC = *( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() );

	IDPTR AdditIDPtr = TASApp.GetpTADB()->Get( pTechP->GetDefaultISChangeOverAdditID( CoolingSide ) );

	clWC.SetAdditFamID( AdditIDPtr.PP->GetIDPtr().ID );
	clWC.SetAdditID( AdditIDPtr.ID );

	double dPcWeight = pTechP->GetDefaultISChangeOverPcWeight( CoolingSide );
	clWC.SetPcWeight( dPcWeight );

	double dCoolingTps = pTechP->GetDefaultISChangeOverTps( CoolingSide );

	// To get a correct freeze temperature, we must at least one time call 'UpdateFluidData'.
	clWC.SetTemp( dCoolingTps );
	clWC.UpdateFluidData( dCoolingTps );
	double dTFreeze = clWC.GetTfreez();

	double dDT = pTechP->GetDefaultISChangeOverDT( CoolingSide );

	// HYS-1092: can do this test only if we have a water characteristic for the current media.
	if( NULL != clWC.GetpAdditChar() )
	{
		double dTMax = clWC.GetpAdditChar()->GetMaxT( clWC.GetPcWeight() );

		// Verify that saved temperatures match TFreeze / TMax of current fluid
		dCoolingTps = min( dCoolingTps, ( dTMax - dDT ) );
		dCoolingTps = max( dCoolingTps, dTFreeze );
	}

	clWC.SetTemp( dCoolingTps );
	clWC.UpdateFluidData( dCoolingTps );

	m_clIndSel6WayValveParams.m_CoolingWC = clWC;
	m_clIndSel6WayValveParams.m_dCoolingDT = dDT;

	if( NULL == m_clIndSel6WayValveParams.m_CoolingWC.GetSecondWaterChar() )
	{
		m_clIndSel6WayValveParams.m_CoolingWC.SetIsForChangeOver( true );
	}

	m_clIndSel6WayValveParams.m_HeatingWC = *( m_clIndSel6WayValveParams.m_CoolingWC.GetSecondWaterChar() );

	double dHeatingTps = pTechP->GetDefaultISChangeOverTps( HeatingSide);

	// To get a correct freeze temperature, we must at least one time call 'UpdateFluidData'.
	m_clIndSel6WayValveParams.m_HeatingWC.SetTemp( dHeatingTps );
	m_clIndSel6WayValveParams.m_HeatingWC.UpdateFluidData( dHeatingTps );

	double dHeatingDT = pTechP->GetDefaultISChangeOverDT( HeatingSide );

	// HYS-1092: can do this test only if we have a water characteristic for the current media.
	if( NULL != clWC.GetpAdditChar() )
	{
		double dTMax = clWC.GetpAdditChar()->GetMaxT( clWC.GetPcWeight() );

		// Verify that saved temperatures match TFreeze / TMax of current fluid

		dHeatingTps = min( dHeatingTps, dTMax );
		dHeatingTps = max( dHeatingTps, ( dTFreeze + dHeatingDT ) );
	}

	m_clIndSel6WayValveParams.m_HeatingWC.SetTemp( dHeatingTps );
	m_clIndSel6WayValveParams.m_HeatingWC.UpdateFluidData( dHeatingTps );
	*( m_clIndSel6WayValveParams.m_CoolingWC.GetSecondWaterChar() ) = m_clIndSel6WayValveParams.m_HeatingWC;
	m_clIndSel6WayValveParams.m_dHeatingDT = dHeatingDT;

	// Set fluid string.
	CString str1, str2;
	m_clIndSel6WayValveParams.m_CoolingWC.BuildWaterStrings( str1, str2 );
	
	// Set the text for the first static.
	SetDlgItemText( IDC_STATICFLUID, str1 );
}

void CDlgIndSel6WayValve::_SetStaticFluidTemp()
{
	CString str1, str2;
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	
	if( NULL == pTechP )
	{
		return;
	}
	
	str1 = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_COOLING );
	str1 += _T(" : ");
	str2 = WriteCUDouble( _U_TEMPERATURE, m_clIndSel6WayValveParams.m_CoolingWC.GetTemp(), true, 3 );
	str2 += _T(" / ");
	str2 += WriteCUDouble( _U_TEMPERATURE, m_clIndSel6WayValveParams.m_CoolingWC.GetTemp() + pTechP->GetDefaultISChangeOverDT( CoolingSide ), true, 3 );
	str1 += str2;
	SetDlgItemText( IDC_STATICTEMP_C, str1 );

	str1 = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_HEATING );
	str1 += _T(" : ");
	str2 = WriteCUDouble( _U_TEMPERATURE, m_clIndSel6WayValveParams.m_HeatingWC.GetTemp(), true, 3 );
	str2 += _T(" / ");
	str2 += WriteCUDouble( _U_TEMPERATURE, m_clIndSel6WayValveParams.m_HeatingWC.GetTemp() - pTechP->GetDefaultISChangeOverDT( HeatingSide ), true, 3 );
	str1 += str2;
	SetDlgItemText( IDC_STATICTEMP_H, str1 );
}

void CDlgIndSel6WayValve::_UpdateCheckFastElectConnection()
{
	if( e6Way_EQMControl != m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
	{
		// Don't need this technology.
		m_CheckFastConnection.EnableWindow( FALSE );
		m_CheckFastConnection.SetCheck( BST_UNCHECKED );
	}
	else
	{
		// We check by default in EQM control
		// HYS-1429 : If only CO actuator is available don't allow to change the checkbox status
		bool bIsEQMEnabled = false;
		bool bIsOnOffPibcvEnabled = false;
		bool bIsOnOffbvEnabled = false;
		bool bIs6WayValveEnabled = false;
		bool bIsPackageEnabled = false;
		// HYS-1877: Update Set check box
		_CheckProductAvailability( bIsEQMEnabled, bIsOnOffPibcvEnabled, bIsOnOffbvEnabled, bIs6WayValveEnabled, bIsPackageEnabled );
		if( bIsEQMEnabled != bIsOnOffPibcvEnabled )
		{
			m_CheckFastConnection.EnableWindow( FALSE );
		}
		else
		{
			m_CheckFastConnection.EnableWindow( TRUE );
		}
		
		if( false == m_clIndSel6WayValveParams.m_bEditModeRunning )
		{
			m_CheckFastConnection.SetCheck( BST_CHECKED );
		}
	}

	// To update variables
	m_clIndSel6WayValveParams.m_bCheckFastConnection = ( BST_CHECKED == m_CheckFastConnection.GetCheck() ) ? true : false;
}

void CDlgIndSel6WayValve::_UpdateCheckOnlyForSet()
{
	bool bIsEQMEnabled = false;
	bool bIsOnOffPibcvEnabled = false;
	bool bIsOnOffbvEnabled = false;
	bool bIs6WayValveEnabled = false;
	bool bIsPackageEnabled = false;
	_CheckProductAvailability( bIsEQMEnabled, bIsOnOffPibcvEnabled, bIsOnOffbvEnabled, bIs6WayValveEnabled, bIsPackageEnabled );
	if( false == bIsPackageEnabled || e6Way_Alone == m_clIndSel6WayValveParams.m_e6WayValveSelectionMode )
	{
		// Don't need this technology.
		m_CheckOnlyForSet.EnableWindow( FALSE );
		m_CheckOnlyForSet.SetCheck( BST_UNCHECKED );
	}
	else
	{
		m_CheckOnlyForSet.EnableWindow( TRUE );
	}

	// To update variables
	m_clIndSel6WayValveParams.m_bOnlyForSet = ( BST_CHECKED == m_CheckOnlyForSet.GetCheck() ) ? true : false;

	// Update combo to fit with set 6-way valve version and connection
	m_pclDlgIndSel6WayValve_6WayValveTab->FillComboConnection();
	m_pclDlgIndSel6WayValve_6WayValveTab->FillComboVersion();
}

void CDlgIndSel6WayValve::_LaunchSuggestion( CString strConnectID, CString strVersionID )
{
	m_pclDlgIndSel6WayValve_6WayValveTab->FillComboConnection( strConnectID );
	m_pclDlgIndSel6WayValve_6WayValveTab->FillComboVersion( strVersionID );

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSel6WayValve::_TryAlternativeSuggestion( bool bSizeShiftProblem )
{
	// Variables.
	int iDevFound = 0;
	bool bReturn = false;

	// If no device found try to change the combo to get at least one valid product.
	// If it's the case, show the 'No device found' dialog. Otherwise, show AFXMSG_...
	if( false == bSizeShiftProblem )
	{
		bool bStop = false;

		m_clIndSel6WayValveParams.m_e6WayValveList.PurgeAll();
		int iValveCount = 0;
		bool bRelaxCombo = true;

		CString strConnectID( _T( "" ) );
		CString strVersionID( _T( "" ) );

		// Launch the no device found dialog box if something is found when all the combos are open.
		iValveCount = m_clIndSel6WayValveParams.m_pTADB->GetTaCVList(
				&m_clIndSel6WayValveParams.m_e6WayValveList,												// List where to saved
				CTADatabase::eFor6WayCV,								// Control valve target (cv, hmcv, picv or bcv)
				false, 													// 'true' returns as soon a result is found
				m_clIndSel6WayValveParams.m_eCV2W3W, 							// Set way number of valve
				_T( "" ),							 						// Type ID
				_T( "" ),			// Family ID
				_T( "" ),		// Body material ID
				(LPCTSTR)strConnectID,		// Connection ID
				(LPCTSTR)strVersionID, 		// Version ID
				_T( "" ),				// PN ID
				CDB_ControlProperties::ControlOnly, 						// Set the control function (control only, presettable, ...)
				m_clIndSel6WayValveParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSel6WayValveParams.m_eFilterSelection, 
			    0, INT_MAX, false, NULL, 
			    m_clIndSel6WayValveParams.m_bOnlyForSet );					// HYS-1877: 'true' if it's only for a set.

		if( 0 < iValveCount )
		{
			iDevFound = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->Select6WayValve( &m_clIndSel6WayValveParams, &bSizeShiftProblem );

			if( iDevFound > 0 && false == bSizeShiftProblem )
			{
				if( true == bRelaxCombo )
				{
					// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevCtrlFound' dialog.
					m_clIndSel6WayValveParams.m_e6WayValveList.PurgeAll();
					double dKey = 0.0;

					for( CSelectedValve *pclSelectedValve = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
							pclSelectedValve = m_clIndSel6WayValveParams.m_pclSelect6WayValveList->GetNext<CSelectedValve>() )
					{
						CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

						if( NULL == pTAP )
						{
							continue;
						}

						m_clIndSel6WayValveParams.m_e6WayValveList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
					}

					// Launch the 'No device found' dialog box.
					// Remark: Combos in this dialog will be filled thanks to the new alternative control valves found and saved in the 'CvList' variable.
					CDlgNoDev6WValveFoundParams clNoDevCtrlParams = CDlgNoDev6WValveFoundParams( &m_clIndSel6WayValveParams.m_e6WayValveList,
						m_clIndSel6WayValveParams.m_strComboConnectID, m_clIndSel6WayValveParams.m_strComboVersionID );

					CDlgNoDev6WValveFound dlg( &m_clIndSel6WayValveParams, &clNoDevCtrlParams );

					if( IDOK == dlg.DoModal() )
					{
						clNoDevCtrlParams = *dlg.GetNoDev6WValveFoundParams();

						_LaunchSuggestion( clNoDevCtrlParams.m_strConnectID, clNoDevCtrlParams.m_strVersionID );
					}

					bStop = true;
				}
			}
		}
		
		if( false == bStop )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_6WVALVE_NO_DEV_FOUND );
		}
	}

	if( 0 == iDevFound && true == bSizeShiftProblem )
	{
		// In that case valves exist but not with the parameters specified.
		// Problem of SizeShift.
		if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_SIZE_SHIFT_PROB, MB_YESNO | MB_ICONQUESTION, 0 ) )
		{
			bReturn = true;
		}
	}

	return bReturn;
}

void CDlgIndSel6WayValve::_CheckProductAvailability( bool& bCanEQMEnabled, bool& bCanOnoffPibcvEnabled, bool& bCanOnoffBvEnabled, 
	bool& bCan6WayValveEnabled, bool &bCanPackageEnable )
{
	CRankEx List;
	_string strLoop = CteEMPTY_STRING;
	CDB_6WayValve* pclProduct = NULL;
	// HYS-1429 : We get all available TA-6way-valve and look if all mode can have solution. For example is US version
	// only actuator M106-CO is available taht's why only EQM will have a solution.
	
	// Get the list of TA-6way-valve
	GetpTADB()->Get6WayValveList( &List );
	bCanEQMEnabled = false;
	bCanOnoffPibcvEnabled = false;
	bCanOnoffBvEnabled = false;
	bCan6WayValveEnabled = false;
	// HYS-1877: Package is a set of: 6-way valve + Actuator + PIBCV + Actuator. So the fours should be exist
	bCanPackageEnable = false;
	bool bIsPackage6WayValveEnable = false;
	bool bIsPackage6WayValveActEnable = false;
	bool bIsPackagePIBCVEnable = false;
	for( BOOL bContinue = List.GetFirstT<CDB_6WayValve*>( strLoop, pclProduct ); TRUE == bContinue;
		bContinue = List.GetNextT<CDB_6WayValve*>( strLoop, pclProduct ) )
	{
		if( false == bCanPackageEnable )
		{
			// HYS-1877: Continue while bCanPackageEnable = false. Init boolean.
			bIsPackage6WayValveEnable = false;
		}

		if( NULL == pclProduct )
		{
			continue;
		}

		if( false == pclProduct->IsSelectable( true ) )
		{
			continue;
		}
		// At least one TA-6way valve could be selected
		bCan6WayValveEnabled = true;

		// HYS-1877: Look if set package are available for this product
		if( ( false == bCanPackageEnable ) && ( 0 != StringCompare( pclProduct->GetTableSetID(), _T( "NO_ID" ) ) ) )
		{
			bIsPackage6WayValveEnable = true;
		}

		// Has the 6-way valve a PIBCV table or BV table ?
		bool bIsAtLeastOnePIBCV = false;
		bool bIsAtLeastOneBV = false;
		CTable* pclPIBCVGroup = (CTable*)( pclProduct->GetPicvGroupTableIDPtr().MP );
		if( NULL != pclPIBCVGroup )
		{
			for( IDPTR idptr = pclPIBCVGroup->GetFirst(); NULL != idptr.MP; idptr = pclPIBCVGroup->GetNext( idptr.MP ) )
			{
				CDB_PIControlValve* pclPIControlValve = dynamic_cast<CDB_PIControlValve*>( idptr.MP );
				if( NULL == pclPIControlValve )
				{
					continue;
				}
				if( false == pclPIControlValve->IsSelectable( true ) )
				{
					continue;
				}
				// HYS-1877: Look if set package are available for this product
				if( false == bCanPackageEnable )
				{
					if( 0 == StringCompare( pclPIControlValve->GetTableSetID(), _T( "6WAYCTRLVALVSET_TAB" ) ) )
					{
						bIsPackagePIBCVEnable = true;
					}
					else if( true == bIsPackage6WayValveEnable )
					{
						continue;
					}
				}

				if( NULL != pclPIControlValve )
				{
					bIsAtLeastOnePIBCV = true;
					break;
				}
			}
		}

		CTable* pclBVGroup = (CTable*)( pclProduct->GetBvFamiliesGroupTableIDPtr().MP );
		if( NULL != pclBVGroup )
		{
			for( IDPTR idptr = pclBVGroup->GetFirst(); NULL != idptr.MP; idptr = pclBVGroup->GetNext( idptr.MP ) )
			{
				CRankEx ValveList;
				// Get all bv with this family ID
				TASApp.GetpTADB()->GetBVList( &ValveList, _T( "" ), idptr.ID, _T( "" ), _T( "" ), _T( "" ) );

				if( 0 < ValveList.GetCount() )
				{
					bIsAtLeastOneBV = true;
					break;
				}
			}
		}

		// Retrieve the actuator group on the selected control valve.
		CTable* pclActuatorGroup = (CTable*)( pclProduct->GetActuatorGroupIDPtr().MP );

		if( NULL == pclActuatorGroup )
		{
			continue;
		}

		// Retrieve list of all actuators in this group.
		CRank ActList( false );

		for( IDPTR idptr = pclActuatorGroup->GetFirst(); NULL != idptr.MP; idptr = pclActuatorGroup->GetNext( idptr.MP ) )
		{
			ActList.Add( idptr.ID, 0, (LPARAM)idptr.MP );
		}

		if( 0 >= ActList.GetCount() )
		{
			continue;
		}
		
		// Look actuator availability	
		CDB_ElectroActuator* pclActuator = NULL;
		CString strLoopAct = CteEMPTY_STRING;
		for( BOOL bLoop = ActList.GetFirstT<CDB_ElectroActuator*>( strLoopAct, pclActuator ); TRUE == bLoop;
			bLoop = ActList.GetNextT<CDB_ElectroActuator*>( strLoopAct, pclActuator ) )
		{
			if( NULL == pclActuator )
			{
				continue;
			}

			if( false == pclActuator->IsSelectable( true ) )
			{
				continue;
			}

			if( false == bCanPackageEnable )
			{
				// HYS-1877: Look if set package are available for this product
				if( 0 == StringCompare( pclActuator->GetTableSetID(), _T( "6WAYCTRLVALVSET_TAB" ) ) )
				{
					bIsPackage6WayValveActEnable = true;
				}
				else if( true == bIsPackage6WayValveEnable )
				{
					continue;
				}
			}

			if( ( bIsAtLeastOnePIBCV == true ) && ( 0 == CString( pclActuator->GetVersionID() ).CompareNoCase( _T( "VERS_CO" ) ) ) )
			{
				bCanEQMEnabled = true;
			}
			else if( bIsAtLeastOnePIBCV == true )
			{
				bCanOnoffPibcvEnabled = true;
			}
			if( ( bIsAtLeastOneBV == true ) && ( 0 != CString( pclActuator->GetVersionID() ).CompareNoCase( _T( "VERS_CO" ) ) ) )
			{
				bCanOnoffBvEnabled = true;
			}

			if( true == bCanOnoffBvEnabled && true == bCanOnoffPibcvEnabled && true == bCanEQMEnabled )
			{
				break;
			}
		}

		// HYS-1877: Checkbox package can be enabled
		bCanPackageEnable = bIsPackage6WayValveEnable & bIsPackage6WayValveActEnable & bIsPackagePIBCVEnable;

		if( true == bCanOnoffBvEnabled && true == bCanOnoffPibcvEnabled && true == bCanEQMEnabled )
		{
			break;
		}
	}
}

BOOL CDlgIndSel6WayValve_Tabs::SetActiveTab( int iTab )
{
	BOOL bResult = CMFCTabCtrl::SetActiveTab( iTab );
	CRect rectCtrlTabContainer;
	CDlgIndSel6WayValve *pclDlg6WayValve = dynamic_cast<CDlgIndSel6WayValve *>( GetParent() );

	if( ( NULL != GetParent() ) && ( NULL != pclDlg6WayValve ) )
	{
		if( 0 == GetActiveTab() )
		{
			pclDlg6WayValve->GetDlgItem( IDC_GROUPPICVTAB )->ShowWindow( SW_HIDE );
			pclDlg6WayValve->GetDlgItem( IDC_GROUP6WAYTAB )->ShowWindow( SW_SHOW );
		}
		else if( 1 == GetActiveTab() )
		{
			pclDlg6WayValve->GetDlgItem( IDC_GROUPPICVTAB )->ShowWindow( SW_SHOW );
			pclDlg6WayValve->GetDlgItem( IDC_GROUP6WAYTAB )->ShowWindow( SW_HIDE );
		}
		
		pclDlg6WayValve->Invalidate(FALSE);
		pclDlg6WayValve->UpdateWindow();
	}
	
	return bResult;
}
