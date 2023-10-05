#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Select.h"

#include "DlgNoDevSmartControlValveFound.h"
#include "DlgConfSel.h"
#include "DlgTechParam.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelSmartDpC.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelSmartDpC.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelSmartDpC::CDlgIndSelSmartDpC( CWnd *pParent )
	: CDlgIndSelBase( m_clIndSelSmartDpCParams, CDlgIndSelSmartDpC::IDD, pParent )
{
	m_clIndSelSmartDpCParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_SmartDpC;
	m_clIndSelSmartDpCParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;
	m_strTypeID = _T("SMARTDPCTYPE");
}

void CDlgIndSelSmartDpC::SaveSelectionParameters()
{
	CDlgIndSelBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSmartDpCParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetSmartDpCRadioFlowPowerDT( (int)m_clIndSelSmartDpCParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetSmartDpCDpBranchCheckBox( ( true == m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked ) ? BST_CHECKED : BST_UNCHECKED );
	pclIndSelParameter->SetSmartDpCDpMaxCheckBox( ( true == m_clIndSelSmartDpCParams.m_bIsDpMaxChecked ) ? BST_CHECKED : BST_UNCHECKED );
	pclIndSelParameter->SetSmartDpCBodyMaterialID( m_clIndSelSmartDpCParams.m_strComboMaterialID );
	pclIndSelParameter->SetSmartDpCConnectID( m_clIndSelSmartDpCParams.m_strComboConnectID );
	pclIndSelParameter->SetSmartDpCPNID( m_clIndSelSmartDpCParams.m_strComboPNID );
	// pclIndSelParameter->SetSmartDpCSetCheckBox( m_clIndSelSmartDpCParams.m_bOnlyForSet );

	if( NULL != pRViewSSelSmartDpC )
	{
		pRViewSSelSmartDpC->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelSmartDpC::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelSmartDpC )
	{
		pRViewSSelSmartDpC->Reset();
	}
}

void CDlgIndSelSmartDpC::SetDpMax( double dDpMax )
{
	if( dDpMax > 0.0 )
	{
		m_clIndSelSmartDpCParams.m_dDpMax = dDpMax;
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, dDpMax ) );
	}
	else
	{
		m_clIndSelSmartDpCParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetWindowText( _T("") );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelSmartDpC, CDlgIndSelBase )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeBodyMat )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_EN_KILLFOCUS( IDC_EDITDPBRANCH, OnEnKillFocusDpBranch )
	ON_EN_CHANGE( IDC_EDITDPBRANCH, OnEnChangeDpBranch )
	ON_BN_CLICKED( IDC_GROUPDPBR, OnBnClickedCheckGroupDpBranch )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDPBRANCH, OnEditEnterChar )
	ON_EN_CHANGE( IDC_EDITDPMAX, OnEnChangeDpMax )
	ON_EN_KILLFOCUS( IDC_EDITDPMAX, OnEnKillFocusDpMax )
	ON_BN_CLICKED( IDC_GROUPDPMAX, OnBnClickedCheckGroupDpMax )
	//ON_BN_CLICKED( IDC_CHECKVALVEKIT, OnBnClickedOnlyForSet )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDPMAX, OnEditEnterChar )

	ON_MESSAGE( WM_USER_MODIFYSELECTEDSMARTDPC, OnModifySelectedSmartDpC )
END_MESSAGE_MAP()

void CDlgIndSelSmartDpC::DoDataExchange( CDataExchange* pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboBdyMat );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPDPBR, m_GroupDpBr );
	DDX_Control( pDX, IDC_STATICDPBRANCHUNIT, m_StaticDpBranchUnit );
	DDX_Control( pDX, IDC_EDITDPBRANCH, m_clExtEditDpBranch );
	DDX_Control( pDX, IDC_GROUPDPMAX, m_GroupDpMax );
	DDX_Control( pDX, IDC_EDITDPMAX, m_clExtEditDpMax );
	//DDX_Control( pDX, IDC_CHECKVALVEKIT, m_CheckOnlyForSet );

	// 'CDlgIndSelBase' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelSmartDpC::OnInitDialog()
{
	CDlgIndSelBase::OnInitDialog();

	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICBODYMAT );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );

	str.Empty();

	// Set proper style and add icons to Qdp, DpBranch and valve groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupValve.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupValve.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Valve, true );
	}

	m_GroupValve.SetInOffice2007Mainframe( true );

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupValve.SetExpandCollapseMode( true );
// 	m_GroupValve.SetNotificationHandler( this );

	if( NULL != pclImgListGroupBox )
	{
		GetpGroupQ()->SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		GetpGroupQ()->SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_Q, true );
	}

	GetpGroupQ()->SetInOffice2007Mainframe( true );

	m_GroupDpBr.SetCheckBoxStyle( BS_AUTOCHECKBOX );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupDpBr.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pen );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupDpBr.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Pen, true );
	}

	m_GroupDpBr.SetInOffice2007Mainframe( true );

	m_GroupDpMax.SetCheckBoxStyle( BS_AUTOCHECKBOX );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupDpMax.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pen );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupDpMax.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Pen, true );
	}

	m_GroupDpMax.SetInOffice2007Mainframe( true );

	// Update group text ("Dp branch").
	str = TASApp.LoadLocalizedString( IDS_LEFTTAB_SMARTDPBRANCH );
	m_GroupDpBr.SetWindowText( str );
	m_GroupDpBr.SetCheck( BST_UNCHECKED );
	_UpdateDpBranchFieldValue();

	// Group Dpmax
	str = TASApp.LoadLocalizedString( IDS_LVSSELB_STATDPMAX );
	m_GroupDpMax.SetWindowText( str );

	m_clExtEditDpMax.SetPhysicalType( _U_DIFFPRESS );
	m_clExtEditDpMax.SetEditType( CNumString::eDouble, CNumString::ePositive );

	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	//str = TASApp.LoadLocalizedString( IDS_DLGINDSEL_SMARTONLYFORSET );
	//GetDlgItem( IDC_CHECKVALVEKIT )->SetWindowText( str );

	// Add a tooltip to max dp in the valve.
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
	 	CString TTstr = TASApp.LoadLocalizedString( IDS_MAXDPINVALVE );
 		m_ToolTip.AddToolWindow( &m_clExtEditDpMax, TTstr );
	}

	return TRUE;
}

void CDlgIndSelSmartDpC::OnCbnSelChangeBodyMat()
{
	m_clIndSelSmartDpCParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
	_FillComboConnect();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeConnect();
}

void CDlgIndSelSmartDpC::OnCbnSelChangeConnect()
{
	m_clIndSelSmartDpCParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	_FillComboPN();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangePN();
}

void CDlgIndSelSmartDpC::OnCbnSelChangePN()
{
	m_clIndSelSmartDpCParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	ClearAll();
}

void CDlgIndSelSmartDpC::OnEditEnterChar( NMHDR* pNMHDR, LRESULT* pResult )
{
	if( IDC_EDITDPBRANCH == pNMHDR->idFrom )
	{
		GetpBtnSuggest()->SetFocus();
		m_clExtEditDpBranch.SetFocus();
		PostWMCommandToControl( GetpBtnSuggest() );
	}
	else if( IDC_EDITDPMAX == pNMHDR->idFrom )
	{
		GetpBtnSuggest()->SetFocus();
		// Reset Focus on the modified Edit.
		m_clExtEditDpMax.SetFocus();
		PostWMCommandToControl( GetpBtnSuggest() );
	}
	else
	{
		CDlgIndSelBase::OnEditEnterChar( pNMHDR, pResult );
	}
}
void CDlgIndSelSmartDpC::OnEnChangeDpBranch()
{
	if( GetFocus() == &m_clExtEditDpBranch )
	{
		ClearAll();
	}
}

void CDlgIndSelSmartDpC::OnEnKillFocusDpBranch()
{
	m_clIndSelSmartDpCParams.m_dDpBranch = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpBranch, &m_clIndSelSmartDpCParams.m_dDpBranch )
			|| m_clIndSelSmartDpCParams.m_dDpBranch < 0.0 )
	{
		m_clIndSelSmartDpCParams.m_dDpBranch = 0.0;
	}
}

void CDlgIndSelSmartDpC::OnBnClickedCheckGroupDpBranch()
{
	m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked = ( BST_CHECKED == m_GroupDpBr.GetCheck() ) ? true : false;
	_UpdateDpBranchFieldState();
	ClearAll();
}

LRESULT CDlgIndSelSmartDpC::OnModifySelectedSmartDpC( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelSmartDpC *pSelSmartDpC = dynamic_cast<CDS_SSelSmartDpC *>( (CData *)lParam );
	ASSERT( NULL != pSelSmartDpC );

	if( NULL == pSelSmartDpC || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelSmartDpCParams.m_SelIDPtr = pSelSmartDpC->GetIDPtr();
	m_bInitialised = false;

	// Send message to notify the display of the product selection tab.
	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );

	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_SmartDpC );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelSmartDpCParams.m_eApplicationType = pSelSmartDpC->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelSmartDpCParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelSmartDpCParams.m_WC = *(pSelSmartDpC->GetpSelectedInfos()->GetpWCData());

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelSmartDpC->GetpSelectedInfos() );

	// To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Remark: The 'm_clIndSelSmartDpCParams.m_strPipeSeriesID' and 'm_clIndSelSmartDpCParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelSmartDpC->GetPipeSeriesID(), pSelSmartDpC->GetPipeID() );

	// Remark: Corresponding combo variables in 'm_clIndSelSmartDpCParams' are updated in each of this following methods.
	_FillComboBodyMat( pSelSmartDpC->GetMaterialID() );
	_FillComboConnect( pSelSmartDpC->GetConnectID() );
	_FillComboPN( pSelSmartDpC->GetPNID() );

	m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked = ( BST_CHECKED == pSelSmartDpC->GetCheckboxDpBranch() ) ? true : false;
	m_clIndSelSmartDpCParams.m_dDpBranch = pSelSmartDpC->GetDpBranchValue();
	m_GroupDpBr.SetCheck( pSelSmartDpC->GetCheckboxDpBranch() );
	_UpdateDpBranchFieldState();
	_UpdateDpBranchFieldValue();
 
	// Remark: 'm_clIndSelSmartDpCParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_GroupDpMax.SetCheck( pSelSmartDpC->GetCheckboxDpMax() );
	_UpdateDpMaxFieldState();

	// Remark: 'm_clIndSelSmartDpCParams.m_dDpMax' is updated in the following method.
	SetDpMax( pSelSmartDpC->GetDpMaxValue() );

	// Only for set check box.
	//m_clIndSelSmartDpCParams.m_bOnlyForSet =  ( BST_CHECKED == pSelSmartDpC->GetCheckboxOnlyForSet() ) ? true : false;
	//m_CheckOnlyForSet.SetCheck( pSelSmartDpC->GetCheckboxOnlyForSet() );

	m_iRadioFlowPowerDT = (CDS_SelProd::efdFlow == pSelSmartDpC->GetFlowDef()) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelSmartDpCParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelSmartDpCParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelSmartDpCParams.m_dFlow = pSelSmartDpC->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelSmartDpCParams.m_dPower = pSelSmartDpC->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelSmartDpCParams.m_dDT = pSelSmartDpC->GetDT();
		UpdateDTFieldValue();
	}

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );
	PostWMCommandToControl( GetpBtnSuggest() );
 
	// _UpdateCheckOnlyForSet();

	return 0;
}

void CDlgIndSelSmartDpC::OnBnClickedCheckGroupDpMax()
{
	_UpdateDpMaxFieldState();
	ClearAll();
}

void CDlgIndSelSmartDpC::OnEnChangeDpMax()
{
	if( GetFocus() == &m_clExtEditDpMax )
	{
		ClearAll();
	}
}

void CDlgIndSelSmartDpC::OnEnKillFocusDpMax()
{
	m_clIndSelSmartDpCParams.m_dDpMax = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpMax, &m_clIndSelSmartDpCParams.m_dDpMax )
			|| m_clIndSelSmartDpCParams.m_dDpMax < 0.0 )
	{
		m_clIndSelSmartDpCParams.m_dDpMax = 0.0;
	}
}

void CDlgIndSelSmartDpC::OnBnClickedSuggest()
{
	ClearAll();

	if( true == _VerifySubmittedValues() )
	{
		m_clIndSelSmartDpCParams.m_SmartDpCList.PurgeAll();

		m_clIndSelSmartDpCParams.m_pTADB->GetSmartDpCList(
			&m_clIndSelSmartDpCParams.m_SmartDpCList,
			(LPCTSTR)m_strTypeID,
			_T(""),
			(LPCTSTR)m_clIndSelSmartDpCParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelSmartDpCParams.m_strComboConnectID,
			_T(""),
			(LPCTSTR)m_clIndSelSmartDpCParams.m_strComboPNID,
			m_clIndSelSmartDpCParams.m_eFilterSelection, 0, INT_MAX, NULL, false, 
			m_clIndSelSmartDpCParams.m_bOnlyForSet ); // HYS-1938: Consider set selection.

		_SuggestIndividualSelection();
	}
}

void CDlgIndSelSmartDpC::OnBnClickedSelect()
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	_SelectIndividualSelection();
}

void CDlgIndSelSmartDpC::ActivateLeftTabDialog()
{
	CDlgIndSelBase::ActivateLeftTabDialog();

	CDS_TechnicalParameter *pTechP = m_clIndSelSmartDpCParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clIndSelSmartDpCParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

void CDlgIndSelSmartDpC::OnDestroy()
{
	CDlgIndSelBase::OnDestroy();

	CDialogEx::OnDestroy();
}

LRESULT CDlgIndSelSmartDpC::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSmartDpCParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_iRadioFlowPowerDT = (CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetSmartDpRadioFlowPowerDT()) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelSmartDpCParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked = ( BST_CHECKED == pclIndSelParameter->GetSmartDpCDpBranchCheckBox() ) ? true : false;
	m_GroupDpBr.SetCheck( pclIndSelParameter->GetSmartDpCDpBranchCheckBox() );
	_UpdateDpBranchFieldState();
	_UpdateDpBranchFieldValue();

	// Remark: 'm_clIndSelSmartDpCParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_GroupDpMax.SetCheck( pclIndSelParameter->GetSmartDpCDpMaxCheckBox() );
	_UpdateDpMaxFieldState();

	// Remark: Corresponding combo variables in 'm_clIndSelSmartDpCParams' are updated in each of this following methods.
	_FillComboBodyMat( pclIndSelParameter->GetSmartDpCMaterialID() );
	_FillComboConnect( pclIndSelParameter->GetSmartDpCConnectID() );
	_FillComboPN( pclIndSelParameter->GetSmartDpCPNID() );
	
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	//m_CheckOnlyForSet.SetCheck( (0 == pclIndSelParameter->GetSmartDpCSetCheckBox()) ? BST_UNCHECKED : BST_CHECKED );
	//_UpdateCheckOnlyForSet();
	
	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();
	m_bInitialised = true;

	if( NULL != pRViewSSelSmartDpC )
	{
		pRViewSSelSmartDpC->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}


LRESULT CDlgIndSelSmartDpC::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnPipeChange( wParam, lParam );

	if( NULL != pRViewSSelSmartDpC )
	{
		pRViewSSelSmartDpC->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSmartDpC::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelSmartDpC )
	{
		pRViewSSelSmartDpC->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSmartDpC::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnUnitChange( wParam, lParam );


	TCHAR name[_MAXCHARS];
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATICDPBRANCHUNIT, name );

	SetDlgItemText( IDC_STATICDPMAXUNIT, name );
	if( m_clIndSelSmartDpCParams.m_dDpMax > 0.0 )
	{
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_clIndSelSmartDpCParams.m_dDpMax ) );
	}

	if( NULL != pRViewSSelSmartDpC )
	{
		pRViewSSelSmartDpC->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSmartDpC::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ((WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel)
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnWaterChange( wParam, lParam );

	if( NULL != pRViewSSelSmartDpC )
	{
		pRViewSSelSmartDpC->Reset();
	}

	return 0;
}

CRViewSSelSS* CDlgIndSelSmartDpC::GetLinkedRightViewSSel( void )
{
	return pRViewSSelSmartDpC;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDlgIndSelSmartDpC::_SuggestIndividualSelection()
{
	if( NULL == pRViewSSelSmartDpC )
	{
		return false;
	}

	if( NULL != m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList )
	{
		delete m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList;
	}

	m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList = new CSelectSmartDpCList();

	if( NULL == m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList || NULL == m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList->GetSelectPipeList() )
	{
		return false;
	}

	m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList->GetSelectPipeList()->SelectPipes( &m_clIndSelSmartDpCParams, m_clIndSelSmartDpCParams.m_dFlow );

	bool bSizeShiftProblem = false;
	bool bBestFound = false;
	int iDevFound = m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList->SelectSmartDpC( &m_clIndSelSmartDpCParams, &bSizeShiftProblem, bBestFound );

	if( iDevFound != 0 && false == bSizeShiftProblem )
	{
		VerifyModificationMode();
		pRViewSSelSmartDpC->Suggest( &m_clIndSelSmartDpCParams );
	}
	else
	{
		if( false == bSizeShiftProblem )
		{
			m_clIndSelSmartDpCParams.m_SmartDpCList.PurgeAll();

			// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
			// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view - Add Set information.
			m_clIndSelSmartDpCParams.m_pTADB->GetSmartDpCList( &m_clIndSelSmartDpCParams.m_SmartDpCList, (LPCTSTR)m_strTypeID, _T(""), _T(""), _T(""), _T(""), _T(""),
					m_clIndSelSmartDpCParams.m_eFilterSelection, 0, INT_MAX, NULL, false, m_clIndSelSmartDpCParams.m_bOnlyForSet );

			iDevFound = m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList->SelectSmartDpC( &m_clIndSelSmartDpCParams, &bSizeShiftProblem, bBestFound );

			if( iDevFound > 0 )
			{
				// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevFound' dialog.
				m_clIndSelSmartDpCParams.m_SmartDpCList.PurgeAll();
				double dKey = 0.0;

				for( CSelectedValve* pclSelectedSmartControlValve = m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList->GetFirst<CSelectedValve>(); NULL != pclSelectedSmartControlValve;
						pclSelectedSmartControlValve = m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList->GetNext<CSelectedValve>() )
				{
					CDB_TAProduct* pTAP = dynamic_cast<CDB_TAProduct*>(pclSelectedSmartControlValve->GetpData());

					if( NULL == pTAP )
					{
						continue;
					}

					m_clIndSelSmartDpCParams.m_SmartDpCList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
				}

				// Launch the no device found dialog box if something is found.
				// Remark: Combos in this dialog will be filled thanks to the new alternative shut-off valves found and saved in the 'm_SvList' variable.
				CDlgNoDevSmartControlValveFoundParams clNoDevSmartDpCParams = CDlgNoDevSmartControlValveFoundParams( &m_clIndSelSmartDpCParams.m_SmartDpCList, m_clIndSelSmartDpCParams.m_strComboMaterialID,
																															  m_clIndSelSmartDpCParams.m_strComboConnectID, m_clIndSelSmartDpCParams.m_strComboPNID );

				CDlgNoDevSmartControlValveFound dlg( &m_clIndSelSmartDpCParams, &clNoDevSmartDpCParams );

				if( IDOK == dlg.DoModal() )
				{
					clNoDevSmartDpCParams = *dlg.GetNoDevFoundSmartControlValveParams();

					_LaunchSuggestion( clNoDevSmartDpCParams.m_strBodyMaterialID, clNoDevSmartDpCParams.m_strConnectID, clNoDevSmartDpCParams.m_strPNID );
				}
			}
			else
			{
				// No device found message.
				TASApp.AfxLocalizeMessageBox( AFXMSG_NO_DEV_FOUND );
			}
		}
		else
		{
			// In that case valves exist but not with the parameters specified.
			// Problem of SizeShift.
			if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_SIZE_SHIFT_PROB, MB_YESNO | MB_ICONQUESTION, 0 ) )
			{
				CDlgTechParam* pdlg = new CDlgTechParam( CDlgTechParam::PageGeneralDeviceSizes );

				if( NULL != pdlg && IDOK != pdlg->DoModal() )
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

	return true;
}

bool CDlgIndSelSmartDpC::_SelectIndividualSelection()
{
	try
	{
		if( NULL == m_clIndSelSmartDpCParams.m_pTADS )
		{
			HYSELECT_THROW( _T( "Internal error: 'm_clIndSelSmartDpCParams.m_pTADS' argument can't be NULL." ) );
		}
		else if( NULL == m_clIndSelSmartDpCParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T( "Internal error: 'm_clIndSelSmartDpCParams.m_pTADS->GetpTechParams()' argument can't be NULL." ) );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelSmartDpC )
		{
			HYSELECT_THROW( _T( "Internal error: The smart DpC right view is not created." ) );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T( "Internal error: The smart DpC right view is not visible." ) );
		}

		// Verify pointer validity; user can delete object after entering in edition.
		if( _T( '\0' ) != *m_clIndSelSmartDpCParams.m_SelIDPtr.ID )
		{
			m_clIndSelSmartDpCParams.m_SelIDPtr = m_clIndSelSmartDpCParams.m_pTADS->Get( m_clIndSelSmartDpCParams.m_SelIDPtr.ID );
		}

		// Create selected object and initialize it.
		CDS_SSelSmartDpC *pSelSmartDpC = dynamic_cast<CDS_SSelSmartDpC *>(m_clIndSelSmartDpCParams.m_SelIDPtr.MP);

		if( NULL == pSelSmartDpC )
		{
			IDPTR IDPtr;
			m_clIndSelSmartDpCParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSmartDpC ) );
			pSelSmartDpC = (CDS_SSelSmartDpC*)(IDPtr.MP);
		}

		pSelSmartDpC->SetQ( m_clIndSelSmartDpCParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelSmartDpCParams.m_eFlowOrPowerDTMode )
		{
			pSelSmartDpC->SetFlowDef( CDS_SelProd::efdFlow );
			pSelSmartDpC->SetPower( 0.0 );
			pSelSmartDpC->SetDT( 0.0 );
		}
		else
		{
			pSelSmartDpC->SetFlowDef( CDS_SelProd::efdPower );
			pSelSmartDpC->SetPower( m_clIndSelSmartDpCParams.m_dPower );
			pSelSmartDpC->SetDT( m_clIndSelSmartDpCParams.m_dDT );
		}
		
		pSelSmartDpC->SetCheckboxDpBranch( (true == m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked ) ? BST_CHECKED : BST_UNCHECKED );
		pSelSmartDpC->SetDpBranchValue( m_clIndSelSmartDpCParams.m_dDpBranch );
		
		pSelSmartDpC->SetCheckboxDpMax( (true == m_clIndSelSmartDpCParams.m_bIsDpMaxChecked ) ? BST_CHECKED : BST_UNCHECKED );
		pSelSmartDpC->SetDpMaxValue( m_clIndSelSmartDpCParams.m_dDpMax );

		//pSelSmartDpC->SetCheckboxOnlyForSet( (int)m_clIndSelSmartDpCParams.m_bOnlyForSet );
		//pSelSmartDpC->SetSelectedAsAPackage( m_clIndSelSmartDpCParams.m_bOnlyForSet );
		pSelSmartDpC->SetMaterialID( m_clIndSelSmartDpCParams.m_strComboMaterialID );
		pSelSmartDpC->SetConnectID( m_clIndSelSmartDpCParams.m_strComboConnectID );
		pSelSmartDpC->SetPNID( m_clIndSelSmartDpCParams.m_strComboPNID );
		pSelSmartDpC->SetPipeSeriesID( m_clIndSelSmartDpCParams.m_strPipeSeriesID );
		pSelSmartDpC->SetPipeID( m_clIndSelSmartDpCParams.m_strPipeID );

		*pSelSmartDpC->GetpSelectedInfos()->GetpWCData() = m_clIndSelSmartDpCParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelSmartDpC->GetpSelectedInfos()->SetDT( m_clIndSelSmartDpCParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelSmartDpC->GetpSelectedInfos()->SetApplicationType( m_clIndSelSmartDpCParams.m_eApplicationType );

		pRViewSSelSmartDpC->FillInSelected( pSelSmartDpC );

		CDlgConfSel dlg( &m_clIndSelSmartDpCParams );
		IDPTR IDPtr = pSelSmartDpC->GetIDPtr();
		dlg.Display( pSelSmartDpC );

		if( IDOK == dlg.DoModal() )
		{
			CTable* pTab = dynamic_cast<CTable*>(m_clIndSelSmartDpCParams.m_pTADS->Get( _T( "SMARTDPC_TAB" ) ).MP);

			if( NULL == pTab )
			{
				HYSELECT_THROW( _T( "Internal error: Can't retrieve the 'SMARTDPC_TAB' table from the database." ) );
			}

			if( _T( '\0' ) == *m_clIndSelSmartDpCParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelSmartDpCParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelSmartDpC ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelSmartDpCParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T( '\0' ) == *m_clIndSelSmartDpCParams.m_SelIDPtr.ID )
			{
				m_clIndSelSmartDpCParams.m_pTADS->DeleteObject( IDPtr );
			}
		}

		return true;
	}
	catch( CHySelectException& clHySelectException )
	{
		clHySelectException.AddMessage( _T( "Error in 'CDlgIndSelSmartDpC::_SelectIndividualSelection'." ), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelSmartDpC::_FillComboBodyMat( CString strBodyMaterialID )
{
	if( NULL == m_clIndSelSmartDpCParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	m_clIndSelSmartDpCParams.m_pTADB->GetSmartDpCBdyMatList( &List, (LPCTSTR)m_strTypeID, (LPCTSTR)m_clIndSelSmartDpCParams.m_strComboFamilyID,
			m_clIndSelSmartDpCParams.m_eFilterSelection, 0, INT_MAX, m_clIndSelSmartDpCParams.m_bOnlyForSet );

	m_ComboBdyMat.FillInCombo( &List, strBodyMaterialID, m_clIndSelSmartDpCParams.GetComboMaterialAllID() );
	m_clIndSelSmartDpCParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSmartDpC::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_clIndSelSmartDpCParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	m_clIndSelSmartDpCParams.m_pTADB->GetSmartDpCConnList( &List, (LPCTSTR)m_strTypeID, (LPCTSTR)m_clIndSelSmartDpCParams.m_strComboFamilyID,
			(LPCTSTR)m_clIndSelSmartDpCParams.m_strComboMaterialID, m_clIndSelSmartDpCParams.m_eFilterSelection, 0, INT_MAX, m_clIndSelSmartDpCParams.m_bOnlyForSet );

	m_ComboConnect.FillInCombo( &List, strConnectID, m_clIndSelSmartDpCParams.GetComboConnectAllID() );
	m_clIndSelSmartDpCParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSmartDpC::_FillComboPN( CString strPNID )
{
	if( NULL == m_clIndSelSmartDpCParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	m_clIndSelSmartDpCParams.m_pTADB->GetSmartDpCPNList( &List, (LPCTSTR)m_strTypeID, (LPCTSTR)m_clIndSelSmartDpCParams.m_strComboFamilyID,
			(LPCTSTR)m_clIndSelSmartDpCParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelSmartDpCParams.m_strComboConnectID, (LPCTSTR)m_clIndSelSmartDpCParams.m_strComboVersionID, 
			m_clIndSelSmartDpCParams.m_eFilterSelection, 0, INT_MAX, m_clIndSelSmartDpCParams.m_bOnlyForSet );

	m_ComboPN.FillInCombo( &List, strPNID, m_clIndSelSmartDpCParams.GetComboPNAllID() );
	m_clIndSelSmartDpCParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSmartDpC::_LaunchSuggestion( CString strMaterialID, CString strConnectID, CString strPNID )
{
	_FillComboBodyMat( strMaterialID );
	_FillComboConnect( strConnectID );
	_FillComboPN( strPNID );

	PostWMCommandToControl( GetpBtnSuggest() );
}

void CDlgIndSelSmartDpC::_UpdateDpBranchFieldValue()
{
	if( true == m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked )
	{
		if( m_clIndSelSmartDpCParams.m_dDpBranch > 0.0 )
		{
			m_clExtEditDpBranch.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_clIndSelSmartDpCParams.m_dDpBranch ) );
		}
		else
		{
			m_clExtEditDpBranch.SetWindowText( _T("") );
		}
	}
	else
	{
		m_clExtEditDpBranch.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
	}
}

void CDlgIndSelSmartDpC::_UpdateDpBranchFieldState()
{
	if( true == m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked )
	{
		m_clExtEditDpBranch.SetReadOnly( FALSE );
		m_clExtEditDpBranch.EnableWindow( TRUE );
		m_clExtEditDpBranch.SetFocus();
		m_clExtEditDpBranch.SetWindowText( _T("") );

		m_StaticDpBranchUnit.ShowWindow( SW_SHOW );
		m_StaticDpBranchUnit.EnableWindow( TRUE );

		m_GroupDpBr.SetForceShowDisable( false );
	}
	else
	{
		m_clExtEditDpBranch.SetReadOnly( TRUE );
		m_clExtEditDpBranch.EnableWindow( FALSE );
		m_clExtEditDpBranch.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );

		m_StaticDpBranchUnit.EnableWindow( FALSE );

		m_GroupDpBr.SetForceShowDisable( true );

		m_clIndSelSmartDpCParams.m_dDpBranch = 0.0;
	}
}

bool CDlgIndSelSmartDpC::_VerifySubmittedValues()
{
	bool bValid = false;

	if( m_clIndSelSmartDpCParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetDlgItem( IDC_EDITFLOW )->SetFocus();
	}
	if( m_clIndSelSmartDpCParams.m_dDpMax <= 0.0 )
	{
		m_GroupDpMax.SetCheck( BST_UNCHECKED );
		_UpdateDpMaxFieldState();
		bValid = true;
	}
	else
	{
		// Check if Dp is valid or not (or check box is checked but there is no value).
		if( CDlgIndSelBase::VDPFlag_DpError != VerifyDpValue() )
		{
			bValid = true;
		}
	}

	double dDpBranchValue = 0.0;

	if( BST_CHECKED == m_GroupDpBr.GetCheck() 
			&& ( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpBranch, &dDpBranchValue ) || dDpBranchValue < 0.0 ) )
	{
		// Uncheck the button.
		m_GroupDpBr.SetCheck( BST_UNCHECKED );
		m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked = false;
		_UpdateDpBranchFieldState();
		bValid = true;
	} 

	return bValid;
}

void CDlgIndSelSmartDpC::_UpdateDpMaxFieldState()
{
	m_clIndSelSmartDpCParams.m_bIsDpMaxChecked = ( BST_CHECKED == m_GroupDpMax.GetCheck() ) ? true : false;

	if( true == m_clIndSelSmartDpCParams.m_bIsDpMaxChecked )
	{
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( TRUE );
		m_clExtEditDpMax.SetFocus();
		m_clExtEditDpMax.SetWindowText( _T("") );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( TRUE );
		m_GroupDpMax.SetForceShowDisable( false );
	}
	else
	{
		m_clIndSelSmartDpCParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( FALSE );
		m_clExtEditDpMax.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( FALSE );
		m_GroupDpMax.SetForceShowDisable( true );
	}
}
