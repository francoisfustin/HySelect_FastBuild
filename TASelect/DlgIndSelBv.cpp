#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Select.h"

#include "DlgNoDevFound.h"
#include "DlgConfSel.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelBv.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelBv.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelBv::CDlgIndSelBv( CWnd *pParent )
	: CDlgIndSelBase( m_clIndSelBVParams, CDlgIndSelBv::IDD, pParent )
{
	m_clIndSelBVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	m_clIndSelBVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;
}

void CDlgIndSelBv::SaveSelectionParameters()
{
	CDlgIndSelBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelBVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetBvRadioFlowPowerDT( (int)m_clIndSelBVParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetBvDpCheckBox( (int)m_clIndSelBVParams.m_bDpEnabled );
	pclIndSelParameter->SetBvTypeID( m_clIndSelBVParams.m_strComboTypeID );
	pclIndSelParameter->SetBvFamilyID( m_clIndSelBVParams.m_strComboFamilyID );
	pclIndSelParameter->SetBvMaterialID( m_clIndSelBVParams.m_strComboMaterialID );
	pclIndSelParameter->SetBvConnectID( m_clIndSelBVParams.m_strComboConnectID );
	pclIndSelParameter->SetBvVersionID( m_clIndSelBVParams.m_strComboVersionID );
	pclIndSelParameter->SetBvPNID( m_clIndSelBVParams.m_strComboPNID );

	if( NULL != pRViewSSelBv )
	{
		pRViewSSelBv->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelBv::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelBv )
	{
		pRViewSSelBv->Reset();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelBv, CDlgIndSelBase )
	ON_CBN_SELCHANGE( IDC_COMBOEXTYPE, OnCbnSelChangeExType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeBodyMat )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_COMMAND( IDC_GROUPDP, OnBnClickedCheckDp )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDBV, OnModifySelectedBV )
END_MESSAGE_MAP()

void CDlgIndSelBv::DoDataExchange( CDataExchange* pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOEXTYPE, m_ComboExType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboBdyMat );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );

	// 'CDlgIndSelBase' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_EDITDP, m_clExtEditDp );
	DDX_Control( pDX, IDC_GROUPDP, m_clGroupDp );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelBv::OnInitDialog() 
{
	CDlgIndSelBase::OnInitDialog();
	
	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str);
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICBODYMAT );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );

	str.Empty();

	// Set proper style and add icons to Qdp and valve groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupValve.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT ( NULL != pclImgListGroupBoxGrayed );

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

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupDp.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_DP );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_clGroupDp.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_DP, true );
	}

	m_clGroupDp.SetCheckBoxStyle( BS_AUTOCHECKBOX );
	m_clGroupDp.SetInOffice2007Mainframe( true );

	return TRUE;
}

void CDlgIndSelBv::OnCbnSelChangeExType() 
{
	m_clIndSelBVParams.m_strComboTypeID = m_ComboExType.GetCBCurSelIDPtr().ID;

	// If the selected type is fixed orifice, the Dp edit box is disabled.
	bool bDpEnable = ( _tcscmp( m_clIndSelBVParams.m_strComboTypeID, _T("RVTYPE_FO") ) != 0 ) ? true : false;

	if( true == bDpEnable )
	{
		if( FALSE == m_clGroupDp.IsWindowEnabled() )
		{
			m_clGroupDp.EnableWindow( true, false );
		}
	}
	else
	{
		m_clGroupDp.SetCheck( BST_UNCHECKED );
		
		// No need to recurse children because 'UpdateDpEdit' already do it.
		m_clGroupDp.EnableWindow( false, false );
		
		// The 'CButton::SetCheck' method used to change checkbox state doesn't fire event.
		// We have thus to do it manually.
		OnBnClickedCheckDp();
	}

	_FillComboFamily();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeFamily();
}

void CDlgIndSelBv::OnCbnSelChangeFamily()
{
	m_clIndSelBVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	_FillComboBodyMat();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeBodyMat();
}

void CDlgIndSelBv::OnCbnSelChangeBodyMat() 
{
	m_clIndSelBVParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
	_FillComboConnect();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeConnect();
}

void CDlgIndSelBv::OnCbnSelChangeConnect() 
{
	m_clIndSelBVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	_FillComboVersion();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeVersion();
}

void CDlgIndSelBv::OnCbnSelChangeVersion() 
{
	m_clIndSelBVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	_FillComboPN();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangePN();
}

void CDlgIndSelBv::OnCbnSelChangePN() 
{
	m_clIndSelBVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	ClearAll();
}

void CDlgIndSelBv::OnBnClickedCheckDp() 
{
	m_clIndSelBVParams.m_bDpEnabled = ( BST_CHECKED == m_clGroupDp.GetCheck() ) ? true : false;
	UpdateDpFieldState();
	ClearAll();
}

LRESULT CDlgIndSelBv::OnModifySelectedBV( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelBv *pSelBv = dynamic_cast<CDS_SSelBv *>( (CData *)lParam );
	ASSERT( NULL != pSelBv );

	if( NULL == pSelBv || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelBVParams.m_SelIDPtr = pSelBv->GetIDPtr();
	m_bInitialised = false;

	// Send message to notify the display of the product selection tab.
	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// HYS-1242: SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );
	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_RegulatingValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelBVParams.m_eApplicationType = pSelBv->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelBVParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelBVParams.m_WC = *( pSelBv->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelBv->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Remark: The 'm_clIndSelBVParams.m_strPipeSeriesID' and 'm_clIndSelBVParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelBv->GetPipeSeriesID(), pSelBv->GetPipeID() );

	// Remark: Corresponding combo variables in 'm_clIndSelBVParams' are updated in each of this following methods.
	_FillComboExType( pSelBv->GetTypeID() );
	_FillComboFamily( pSelBv->GetFamilyID() );
	_FillComboBodyMat( pSelBv->GetMaterialID() );
	_FillComboConnect( pSelBv->GetConnectID() );
	_FillComboVersion( pSelBv->GetVersionID() );
	_FillComboPN( pSelBv->GetPNID() );

	m_clIndSelBVParams.m_bDpEnabled = ( pSelBv->GetDp() > 0.0 ) ? true : false;
	m_clGroupDp.SetCheck( ( pSelBv->GetDp() > 0.0 ) ? BST_CHECKED : BST_UNCHECKED );

	m_clIndSelBVParams.m_dDp = pSelBv->GetDp();
	CDlgIndSelBase::UpdateDpFieldState();
	CDlgIndSelBase::UpdateDpFieldValue();

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelBv->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelBVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelBVParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelBVParams.m_dFlow = pSelBv->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelBVParams.m_dPower = pSelBv->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelBVParams.m_dDT = pSelBv->GetDT();
		UpdateDTFieldValue();
	}

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );
	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

void CDlgIndSelBv::OnBnClickedSuggest() 
{
	ClearAll();
	
 	if( true == _VerifySubmittedValues() )
 	{
		m_clIndSelBVParams.m_BvList.PurgeAll();

		m_clIndSelBVParams.m_pTADB->GetBVList( 
				&m_clIndSelBVParams.m_BvList,
				(LPCTSTR)m_clIndSelBVParams.m_strComboTypeID,
				(LPCTSTR)m_clIndSelBVParams.m_strComboFamilyID, 
				(LPCTSTR)m_clIndSelBVParams.m_strComboMaterialID, 
				(LPCTSTR)m_clIndSelBVParams.m_strComboConnectID, 
				(LPCTSTR)m_clIndSelBVParams.m_strComboVersionID,
				m_clIndSelBVParams.m_eFilterSelection );
		
		_SuggestIndividualSelection();
	}
}

void CDlgIndSelBv::OnBnClickedSelect() 
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	_SelectIndividualSelection();
}

void CDlgIndSelBv::OnEnChangeDp() 
{
	if( GetFocus() == GetpEditDp() )
	{
		ClearAll();	
	}
}

void CDlgIndSelBv::ActivateLeftTabDialog()
{
	CDlgIndSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clIndSelBVParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clIndSelBVParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

void CDlgIndSelBv::OnDestroy() 
{
	CDlgIndSelBase::OnDestroy();
	m_ImageListComboType.DeleteImageList();
	m_ImageListComboTypeGrayed.DeleteImageList();

	CDialogEx::OnDestroy();
}

LRESULT CDlgIndSelBv::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelBVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetBvRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelBVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	// Set the Dp check box state.
	m_clIndSelBVParams.m_bDpEnabled = ( 0 == pclIndSelParameter->GetBvDpCheckBox() ) ? false : true;
	m_clGroupDp.SetCheck( ( true == m_clIndSelBVParams.m_bDpEnabled ) ? BST_CHECKED : BST_UNCHECKED );
	CDlgIndSelBase::UpdateDpFieldState();

	// Remark: Corresponding combo variables in 'm_clIndSelBVParams' are updated in each of this following methods.
	_FillComboExType( pclIndSelParameter->GetBvTypeID() );
	_FillComboFamily( pclIndSelParameter->GetBvFamilyID() );
	_FillComboBodyMat( pclIndSelParameter->GetBvMaterialID() );
	_FillComboConnect( pclIndSelParameter->GetBvConnectID() );
	_FillComboVersion( pclIndSelParameter->GetBvVersionID() );
	_FillComboPN( pclIndSelParameter->GetBvPNID() );

	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();
	m_bInitialised = true;

	if( NULL != pRViewSSelBv )
	{
		pRViewSSelBv->OnNewDocument( pclIndSelParameter );
	}
	
	return 0;
}

LRESULT CDlgIndSelBv::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnPipeChange( wParam, lParam );
	
	if( NULL != pRViewSSelBv )
	{
		pRViewSSelBv->Reset();
	}
	
	return 0;
}

LRESULT CDlgIndSelBv::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelBv )
	{
		pRViewSSelBv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelBv::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnUnitChange( wParam, lParam );

	if( NULL != pRViewSSelBv )
	{
		pRViewSSelBv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelBv::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel )
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnWaterChange( wParam, lParam );

	if( NULL != pRViewSSelBv )
	{
		pRViewSSelBv->Reset();
	}

	return 0;
}

CRViewSSelSS *CDlgIndSelBv::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelBv;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDlgIndSelBv::_SuggestIndividualSelection()
{
	if( NULL == pRViewSSelBv )
	{
		return false;
	}

	if( NULL != m_clIndSelBVParams.m_pclSelectBvList )
	{
		delete m_clIndSelBVParams.m_pclSelectBvList;
	}

	m_clIndSelBVParams.m_pclSelectBvList = new CSelectList();

	if( NULL == m_clIndSelBVParams.m_pclSelectBvList || NULL == m_clIndSelBVParams.m_pclSelectBvList->GetSelectPipeList() )
	{
		return false;
	}

	m_clIndSelBVParams.m_pclSelectBvList->GetSelectPipeList()->SelectPipes( &m_clIndSelBVParams, m_clIndSelBVParams.m_dFlow );

	int iDevFound = m_clIndSelBVParams.m_pclSelectBvList->GetManBvList( &m_clIndSelBVParams );
	
	if( 0 != iDevFound )
	{
		VerifyModificationMode();
		pRViewSSelBv->Suggest( &m_clIndSelBVParams );
	}
	else
	{
		// HYS-697: relax criterion one by one from the lowest to the highest important.
		_string strSelTypeID = m_clIndSelBVParams.m_strComboTypeID;
		_string strSelFamilyID = m_clIndSelBVParams.m_strComboFamilyID;
		_string strSelBdyMatID = m_clIndSelBVParams.m_strComboMaterialID;
		_string strSelConnectID = m_clIndSelBVParams.m_strComboConnectID;
		_string strSelVersionID = m_clIndSelBVParams.m_strComboVersionID;
		bool bShowNoDeviceFoundMsg = true;

		for( int i = 0; i < 4; i++ )
		{
			m_clIndSelBVParams.m_BvList.PurgeAll();

			if( 0 == i )
			{
				strSelVersionID = _T("");
			}
			else if( 1 == i )
			{
				strSelConnectID = _T( "" );
			}
			else if( 2 == i )
			{
				strSelBdyMatID = _T( "" );
			}
			else
			{
				strSelFamilyID = _T( "" );
			}

			m_clIndSelBVParams.m_pTADB->GetBVList( &m_clIndSelBVParams.m_BvList, strSelTypeID, strSelFamilyID, strSelBdyMatID, strSelConnectID,
				strSelVersionID, m_clIndSelBVParams.m_eFilterSelection );

			iDevFound = m_clIndSelBVParams.m_pclSelectBvList->GetManBvList( &m_clIndSelBVParams );

			if( iDevFound > 0 )
			{
				// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevFound' dialog.
				m_clIndSelBVParams.m_BvList.PurgeAll();
				double dKey = 0.0;

				for( CSelectedValve *pclSelectedValve = m_clIndSelBVParams.m_pclSelectBvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
					pclSelectedValve = m_clIndSelBVParams.m_pclSelectBvList->GetNext<CSelectedValve>() )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

					if( NULL == pTAP )
					{
						continue;
					}

					m_clIndSelBVParams.m_BvList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
				}

				// Launch the no device found dialog box if something is found.
				// Remark: Combos in this dialog will be filled thanks to the new alternative shut-off valves found and saved in the 'm_SvList' variable.
				CDlgNoDevFoundParams clNoDevParams = CDlgNoDevFoundParams( &m_clIndSelBVParams.m_BvList, strSelTypeID.c_str(), strSelFamilyID.c_str(),
					strSelBdyMatID.c_str(), strSelConnectID.c_str(), strSelVersionID.c_str(), m_clIndSelBVParams.m_strComboPNID );

				CDlgNoDevFound dlg( &m_clIndSelBVParams, &clNoDevParams );

				if( IDOK == dlg.DoModal() )
				{
					clNoDevParams = *dlg.GetNoDevFoundParams();

					_LaunchSuggestion( clNoDevParams.m_strFamilyID, clNoDevParams.m_strMaterialID, clNoDevParams.m_strConnectID, clNoDevParams.m_strVersionID, clNoDevParams.m_strPNID );
				}
				else
				{
					// User has cancel, we return without error message.
					bShowNoDeviceFoundMsg = false;
				}

				break;
			}
		}

		if( 0 == iDevFound && true == bShowNoDeviceFoundMsg )
		{
			// No device found message.
			TASApp.AfxLocalizeMessageBox( AFXMSG_NO_DEV_FOUND );
		}
	}

	return true;
}

bool CDlgIndSelBv::_SelectIndividualSelection()
{
	try
	{
		if( NULL == m_clIndSelBVParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelBVParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelBVParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelBVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelBv )
		{
			HYSELECT_THROW( _T("Internal error: The balancing valve right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The balancing valve right view is not visible.") );
		}

		// Verify pointer validity; user can delete object after entering in edition.
		if( _T('\0') != *m_clIndSelBVParams.m_SelIDPtr.ID )
		{
			m_clIndSelBVParams.m_SelIDPtr = m_clIndSelBVParams.m_pTADS->Get( m_clIndSelBVParams.m_SelIDPtr.ID );
		}
	
		// Create selected object and initialize it.
		CDS_SSelBv *pSelBv = dynamic_cast<CDS_SSelBv *>( m_clIndSelBVParams.m_SelIDPtr.MP );

		if( NULL == pSelBv )
		{
			IDPTR IDPtr;
			m_clIndSelBVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelBv ) );
			pSelBv = (CDS_SSelBv *)( IDPtr.MP );
		}
	
		pSelBv->SetQ( m_clIndSelBVParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelBVParams.m_eFlowOrPowerDTMode )
		{
			pSelBv->SetFlowDef( CDS_SelProd::efdFlow );
			pSelBv->SetPower( 0.0 );
			pSelBv->SetDT( 0.0 );
		}
		else
		{
			pSelBv->SetFlowDef( CDS_SelProd::efdPower );
			pSelBv->SetPower( m_clIndSelBVParams.m_dPower );
			pSelBv->SetDT( m_clIndSelBVParams.m_dDT );
		}
	
		if( m_clIndSelBVParams.m_dDp > 0.0 )
		{
			pSelBv->SetDp( m_clIndSelBVParams.m_dDp );
		}
		else
		{
			pSelBv->SetDp( 0.0 );
		}

		pSelBv->SetSelectedAsAPackage( m_clIndSelBVParams.m_bOnlyForSet );
		pSelBv->SetTypeID( m_clIndSelBVParams.m_strComboTypeID );
		pSelBv->SetFamilyID( m_clIndSelBVParams.m_strComboFamilyID );
		pSelBv->SetMaterialID( m_clIndSelBVParams.m_strComboMaterialID );
		pSelBv->SetConnectID( m_clIndSelBVParams.m_strComboConnectID );
		pSelBv->SetVersionID( m_clIndSelBVParams.m_strComboVersionID );
		pSelBv->SetPNID( m_clIndSelBVParams.m_strComboPNID );
		pSelBv->SetPipeSeriesID( m_clIndSelBVParams.m_strPipeSeriesID );
		pSelBv->SetPipeID( m_clIndSelBVParams.m_strPipeID );

		*pSelBv->GetpSelectedInfos()->GetpWCData() = m_clIndSelBVParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelBv->GetpSelectedInfos()->SetDT( m_clIndSelBVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelBv->GetpSelectedInfos()->SetApplicationType( m_clIndSelBVParams.m_eApplicationType );
	
		pRViewSSelBv->FillInSelected( pSelBv );

		CDlgConfSel dlg( &m_clIndSelBVParams );
		IDPTR IDPtr = pSelBv->GetIDPtr();
		dlg.Display( pSelBv );
	
		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelBVParams.m_pTADS->Get( _T("REGVALV_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'REGVALV_TAB' table from the database.") );
			}

			if( _T('\0') == *m_clIndSelBVParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelBVParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelBv ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelBVParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelBVParams.m_SelIDPtr.ID )
			{
				m_clIndSelBVParams.m_pTADS->DeleteObject( IDPtr );
			}
		}

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelBv::_SelectIndividualSelection'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelBv::_FillComboExType( CString strTypeID )
{
	// Reset the combo if needed.
	if( m_ComboExType.GetCount() > 0 )
	{
		m_ComboExType.ResetContent();
	}

	// Create if needed and attach an image list to 'm_ComboexType'.
	if( NULL == m_ImageListComboType.GetSafeHandle() )
	{
		m_ImageListComboType.Create( IDB_COMBOBOXEXSSELA, 16, 1, _BLACK );
		m_ImageListComboType.SetBkColor( CLR_NONE );
	}

	m_ComboExType.SetImageListW( &m_ImageListComboType, true );
	
	if( NULL == m_ImageListComboTypeGrayed.GetSafeHandle() )
	{
		m_ImageListComboTypeGrayed.Create( IDB_COMBOBOXEXSSELAGRAYED, 16, 1, _BLACK );
		m_ImageListComboTypeGrayed.SetBkColor( CLR_NONE );
	}

	m_ComboExType.SetImageListW( &m_ImageListComboTypeGrayed, false );
	
	// Find the regulating valve type table.
	CTable *pTab = (CTable*)( m_clIndSelBVParams.m_pTADB->Get( _T("RVTYPE_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Item insertion !!! Must be done starting at position 0 and then by increment of 1.
	int iTotalType = 0;
	IDPTR IDPtr = _NULL_IDPTR;
	
	for( IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext(), iTotalType++ );
	ASSERT( 0 != iTotalType );

	// Prepare vector of 'ComboBoxEx' items.
	COMBOBOXEXITEM *pComboBoxExItem = new COMBOBOXEXITEM[iTotalType];

	if( NULL == pComboBoxExItem )
	{
		ASSERT( 0 );
		return;
	}

	for( int iLoopType = 0; iLoopType < iTotalType; iLoopType++ )
	{
		pComboBoxExItem[iLoopType].iItem = -1;
		pComboBoxExItem[iLoopType].mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM;
		pComboBoxExItem[iLoopType].cchTextMax = 31;
	}

	// Loop on all entries of the table and insert data in vector cbi.
	IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) );
	ASSERT( _T('\0') != *IDPtr.ID );
	
	// Create an array to store pointers of strings that will be deleted just after have filled the CExtComboBoxEx.
	CArray <CString *> arStr;
	int iNumType = 0;
	
	while( _T('\0') != *IDPtr.ID )
	{
		if( 0 != _tcscmp( IDPtr.ID, _T("RVTYPE_MV") ) )
		{
			CDB_StringID *pstrI = (CDB_StringID*)( IDPtr.MP );
			int iTypeOrder = _ttoi( pstrI->GetIDstr() ) - 1;
			pComboBoxExItem[iNumType].iItem = iNumType;
			CString *pStr = new CString( pstrI->GetString() );

			if( NULL == pStr )
			{
				ASSERT( 0 );
				continue;
			}

			arStr.Add( pStr );
			pComboBoxExItem[iNumType].pszText = (TCHAR*)(LPCTSTR)( *pStr );
			pComboBoxExItem[iNumType].iImage = iTypeOrder;
			pComboBoxExItem[iNumType].iSelectedImage = iTypeOrder;
			pComboBoxExItem[iNumType].lParam = (LPARAM)(void *)IDPtr.MP;
			iNumType++;
		}
		IDPtr = pTab->GetNext();
	};

	// Insert items from vector cbi in the combo.
	for( int iLoopType = 0; iLoopType < iNumType; iLoopType++ )
	{
		VERIFY( iLoopType == m_ComboExType.InsertItem( &pComboBoxExItem[iLoopType] ) );
	}
	
	for( int iLoopType = 0; iLoopType < arStr.GetCount() ; iLoopType++ )
	{
		delete arStr.GetAt( iLoopType );
	}

	m_ComboExType.SetCurSel( 0 );

	delete [] pComboBoxExItem;

	// Try to find at least one regulating valve available for this type.
	pTab = (CTable*)( m_clIndSelBVParams.m_pTADB->Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( int iLoopType = 0; iLoopType < m_ComboExType.GetCount(); )
	{
        CDB_StringID *pStrID = (CDB_StringID*)m_ComboExType.GetItemDataPtr( iLoopType );

		if( NULL == pStrID )
		{
			continue;
		}

		CString strType = pStrID->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Scan all regulating valves with the same type.
			if( _tcscmp( ( (CDB_TAProduct*)IDPtr.MP )->GetTypeIDPtr().ID, (LPCTSTR)strType ) != 0 )
			{
				continue;
			}

			if( false == ( (CDB_Thing*)IDPtr.MP )->IsSelectable( true ) )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted one BV of this type is enough to display the type.
			break;					
		}
		
		if( false == bSelectable )
		{
			m_ComboExType.DeleteItem( iLoopType );
		}
		else 
		{
			iLoopType++;
		}
	}
	
	int iPos = m_ComboExType.FindCBIDPtr( strTypeID );

	if( iPos >= 0 )
	{
		m_ComboExType.SetCurSel( iPos );
	}
	else
	{
		m_ComboExType.SetCurSel( 0 );
	}

	if( 1 == m_ComboExType.GetCount() )
	{
		m_ComboExType.EnableWindow( FALSE );
	}

	m_clIndSelBVParams.m_strComboTypeID = m_ComboExType.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBv::_FillComboFamily( CString strFamilyID )
{
	if( NULL == m_clIndSelBVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;
	m_clIndSelBVParams.m_pTADB->GetBVFamilyList( &List, (LPCTSTR)m_clIndSelBVParams.m_strComboTypeID, m_clIndSelBVParams.m_eFilterSelection );
	m_ComboFamily.FillInCombo( &List, strFamilyID, m_clIndSelBVParams.GetComboFamilyAllID() );
	m_clIndSelBVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBv::_FillComboBodyMat( CString strBodyMaterialID )
{
	if( NULL == m_clIndSelBVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	m_clIndSelBVParams.m_pTADB->GetBVBdyMatList( &List, (LPCTSTR)m_clIndSelBVParams.m_strComboTypeID, (LPCTSTR)m_clIndSelBVParams.m_strComboFamilyID, 
			m_clIndSelBVParams.m_eFilterSelection );

	m_ComboBdyMat.FillInCombo( &List, strBodyMaterialID, m_clIndSelBVParams.GetComboMaterialAllID() );
	m_clIndSelBVParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBv::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_clIndSelBVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	m_clIndSelBVParams.m_pTADB->GetBVConnList( &List, (LPCTSTR)m_clIndSelBVParams.m_strComboTypeID, (LPCTSTR)m_clIndSelBVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelBVParams.m_strComboMaterialID, m_clIndSelBVParams.m_eFilterSelection );

	m_ComboConnect.FillInCombo( &List, strConnectID, m_clIndSelBVParams.GetComboConnectAllID() );
	m_clIndSelBVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBv::_FillComboVersion( CString strVersionID )
{
	if( NULL == m_clIndSelBVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	m_clIndSelBVParams.m_pTADB->GetBVVersList( &List, (LPCTSTR)m_clIndSelBVParams.m_strComboTypeID, (LPCTSTR)m_clIndSelBVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelBVParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelBVParams.m_strComboConnectID, m_clIndSelBVParams.m_eFilterSelection );

	m_ComboVersion.FillInCombo( &List, strVersionID, m_clIndSelBVParams.GetComboVersionAllID() );
	m_clIndSelBVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBv::_FillComboPN( CString strPNID )
{
	if( NULL == m_clIndSelBVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	m_clIndSelBVParams.m_pTADB->GetBVPNList( &List, (LPCTSTR)m_clIndSelBVParams.m_strComboTypeID, (LPCTSTR)m_clIndSelBVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelBVParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelBVParams.m_strComboConnectID, (LPCTSTR)m_clIndSelBVParams.m_strComboVersionID, 
			m_clIndSelBVParams.m_eFilterSelection );

	m_ComboPN.FillInCombo( &List, strPNID, m_clIndSelBVParams.GetComboPNAllID() );
	m_clIndSelBVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBv::_LaunchSuggestion( CString strFamilyID, CString strMaterialID, CString strConnectID, CString strVersionID, CString strPNID )
{
	_FillComboFamily( strFamilyID );
	_FillComboBodyMat( strMaterialID );
	_FillComboConnect( strConnectID );
	_FillComboVersion( strVersionID );
	_FillComboPN( strPNID );

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelBv::_VerifySubmittedValues()
{
	bool bValid = false;

	if( m_clIndSelBVParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetDlgItem( IDC_EDITFLOW )->SetFocus();
	}
	else
	{
		// Check if Dp is valid or not (or check box is checked but there is no value).
		if( CDlgIndSelBase::VDPFlag_DpError != VerifyDpValue() )
		{
			bValid = true;
		}
	}

	return bValid;
}
