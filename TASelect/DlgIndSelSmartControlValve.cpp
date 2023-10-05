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
#include "RViewSSelSmartControlValve.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelSmartControlValve.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelSmartControlValve::CDlgIndSelSmartControlValve( CWnd *pParent )
	: CDlgIndSelBase( m_clIndSelSmartControlValveParams, CDlgIndSelSmartControlValve::IDD, pParent )
{
	m_clIndSelSmartControlValveParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_SmartControlValve;
	m_clIndSelSmartControlValveParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;
	m_strTypeID = _T( "SMARTCTRLVALVETYPE" );
}

void CDlgIndSelSmartControlValve::SaveSelectionParameters()
{
	CDlgIndSelBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSmartControlValveParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetSmartControlValveRadioFlowPowerDT( (int)m_clIndSelSmartControlValveParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetSmartControlValveBodyMaterialID( m_clIndSelSmartControlValveParams.m_strComboMaterialID );
	pclIndSelParameter->SetSmartControlValveConnectID( m_clIndSelSmartControlValveParams.m_strComboConnectID );
	pclIndSelParameter->SetSmartControlValvePNID( m_clIndSelSmartControlValveParams.m_strComboPNID );
	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	pclIndSelParameter->SetSmartControlValveDpMaxCheckBox( (int)m_clIndSelSmartControlValveParams.m_bIsDpMaxChecked );

	if( NULL != pRViewSSelSmartControlValve )
	{
		pRViewSSelSmartControlValve->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelSmartControlValve::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelSmartControlValve )
	{
		pRViewSSelSmartControlValve->Reset();
	}
}

void CDlgIndSelSmartControlValve::SetDpMax( double dDpMax )
{
	if( dDpMax > 0.0 )
	{
		m_clIndSelSmartControlValveParams.m_dDpMax = dDpMax;
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, dDpMax ) );
	}
	else
	{
		m_clIndSelSmartControlValveParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetWindowText( _T( "" ) );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelSmartControlValve, CDlgIndSelBase )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeBodyMat )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_EN_CHANGE( IDC_EDITDPMAX, OnEnChangeDpMax )
	ON_EN_KILLFOCUS( IDC_EDITDPMAX, OnEnKillFocusDpMax )
	ON_BN_CLICKED( IDC_GROUPDPMAX, OnBnClickedCheckGroupDpMax )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDPMAX, OnEditEnterChar )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDSMARTCONTROLVALVE, OnModifySelectedSmartControlValve )
END_MESSAGE_MAP()

void CDlgIndSelSmartControlValve::DoDataExchange( CDataExchange* pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboBdyMat );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPDPMAX, m_GroupDpMax );
	DDX_Control( pDX, IDC_EDITDPMAX, m_clExtEditDpMax );

	// 'CDlgIndSelBase' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelSmartControlValve::OnInitDialog() 
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
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str);
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICBODYMAT );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	
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

	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
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

	str = TASApp.LoadLocalizedString( IDS_LVSSELB_STATDPMAX );
	m_GroupDpMax.SetWindowText( str );

	m_clExtEditDpMax.SetPhysicalType( _U_DIFFPRESS );
	m_clExtEditDpMax.SetEditType( CNumString::eDouble, CNumString::ePositive );

	// Add a tooltip to max dp in the valve.
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
	 	CString TTstr = TASApp.LoadLocalizedString( IDS_MAXDPINVALVE );
 		m_ToolTip.AddToolWindow( &m_clExtEditDpMax, TTstr );
	}

	return TRUE;
}

void CDlgIndSelSmartControlValve::OnCbnSelChangeBodyMat() 
{
	m_clIndSelSmartControlValveParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
	_FillComboConnect();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeConnect();
}

void CDlgIndSelSmartControlValve::OnCbnSelChangeConnect() 
{
	m_clIndSelSmartControlValveParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	_FillComboPN();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangePN();
}

void CDlgIndSelSmartControlValve::OnCbnSelChangePN() 
{
	m_clIndSelSmartControlValveParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	ClearAll();
}

LRESULT CDlgIndSelSmartControlValve::OnModifySelectedSmartControlValve( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelSmartControlValve *pSelSmartControlValve = dynamic_cast<CDS_SSelSmartControlValve *>( (CData *)lParam );
	ASSERT( NULL != pSelSmartControlValve );

	if( NULL == pSelSmartControlValve || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelSmartControlValveParams.m_SelIDPtr = pSelSmartControlValve->GetIDPtr();
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
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_SmartControlValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelSmartControlValveParams.m_eApplicationType = pSelSmartControlValve->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelSmartControlValveParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelSmartControlValveParams.m_WC = *( pSelSmartControlValve->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelSmartControlValve->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Remark: The 'm_clIndSelSmartControlValveParams.m_strPipeSeriesID' and 'm_clIndSelSmartControlValveParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelSmartControlValve->GetPipeSeriesID(), pSelSmartControlValve->GetPipeID() );

	// Remark: Corresponding combo variables in 'm_clIndSelSmartControlValveParams' are updated in each of this following methods.
	_FillComboBodyMat( pSelSmartControlValve->GetMaterialID() );
	_FillComboConnect( pSelSmartControlValve->GetConnectID() );
	_FillComboPN( pSelSmartControlValve->GetPNID() );

	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	// Remark: 'm_clIndSelPIBCVParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_GroupDpMax.SetCheck( ( true == pSelSmartControlValve->GetCheckboxDpMax() ) ? BST_CHECKED : BST_UNCHECKED );
	_UpdateDpMaxFieldState();

	// Remark: 'm_clIndSelPIBCVParams.m_dDpMax' is updated in the following method.
	SetDpMax( pSelSmartControlValve->GetDpMax() );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelSmartControlValve->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelSmartControlValveParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelSmartControlValveParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelSmartControlValveParams.m_dFlow = pSelSmartControlValve->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelSmartControlValveParams.m_dPower = pSelSmartControlValve->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelSmartControlValveParams.m_dDT = pSelSmartControlValve->GetDT();
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

void CDlgIndSelSmartControlValve::OnBnClickedCheckGroupDpMax()
{
	_UpdateDpMaxFieldState();
	ClearAll();
}

void CDlgIndSelSmartControlValve::OnEnChangeDpMax()
{
	if( GetFocus() == &m_clExtEditDpMax )
	{
		ClearAll();
	}
}

void CDlgIndSelSmartControlValve::OnEnKillFocusDpMax()
{
	m_clIndSelSmartControlValveParams.m_dDpMax = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpMax, &m_clIndSelSmartControlValveParams.m_dDpMax )
		|| m_clIndSelSmartControlValveParams.m_dDpMax < 0.0 )
	{
		m_clIndSelSmartControlValveParams.m_dDpMax = 0.0;
	}
}

void CDlgIndSelSmartControlValve::OnBnClickedSuggest() 
{
	ClearAll();
	
 	if( true == _VerifySubmittedValues() )
 	{
		m_clIndSelSmartControlValveParams.m_SmartControlValveList.PurgeAll();

		// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
		m_clIndSelSmartControlValveParams.m_pTADB->GetSmartControlValveList(
				&m_clIndSelSmartControlValveParams.m_SmartControlValveList,
			    (LPCTSTR)m_strTypeID,
				_T(""), 
				(LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboMaterialID, 
				(LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboConnectID, 
				_T(""),
				(LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboPNID,
				m_clIndSelSmartControlValveParams.m_eFilterSelection );
		
		_SuggestIndividualSelection();
	}
}

void CDlgIndSelSmartControlValve::OnBnClickedSelect() 
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	_SelectIndividualSelection();
}

void CDlgIndSelSmartControlValve::OnEditEnterChar( NMHDR* pNMHDR, LRESULT* pResult )
{
	if( IDC_EDITDPMAX == pNMHDR->idFrom )
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

void CDlgIndSelSmartControlValve::ActivateLeftTabDialog()
{
	CDlgIndSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clIndSelSmartControlValveParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clIndSelSmartControlValveParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

void CDlgIndSelSmartControlValve::OnDestroy() 
{
	CDlgIndSelBase::OnDestroy();
	m_ImageListComboType.DeleteImageList();
	m_ImageListComboTypeGrayed.DeleteImageList();

	CDialogEx::OnDestroy();
}

LRESULT CDlgIndSelSmartControlValve::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSmartControlValveParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetSmartControlValveRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelSmartControlValveParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	// Remark: Corresponding combo variables in 'm_clIndSelSmartControlValveParams' are updated in each of this following methods.
	_FillComboBodyMat( pclIndSelParameter->GetSmartControlValveMaterialID() );
	_FillComboConnect( pclIndSelParameter->GetSmartControlValveConnectID() );
	_FillComboPN( pclIndSelParameter->GetSmartControlValvePNID() );

	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	// Remark: 'm_clIndSelSmartControlValveParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_GroupDpMax.SetCheck( (0 == pclIndSelParameter->GetSmartControlValveDpMaxCheckBox()) ? BST_UNCHECKED : BST_CHECKED );
	_UpdateDpMaxFieldState();

	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();
	m_bInitialised = true;

	if( NULL != pRViewSSelSmartControlValve )
	{
		pRViewSSelSmartControlValve->OnNewDocument( pclIndSelParameter );
	}
	
	return 0;
}

LRESULT CDlgIndSelSmartControlValve::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnPipeChange( wParam, lParam );
	
	if( NULL != pRViewSSelSmartControlValve )
	{
		pRViewSSelSmartControlValve->Reset();
	}
	
	return 0;
}

LRESULT CDlgIndSelSmartControlValve::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelSmartControlValve )
	{
		pRViewSSelSmartControlValve->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSmartControlValve::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnUnitChange( wParam, lParam );

	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	TCHAR name[_MAXCHARS];
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATICDPMAXUNIT, name );

	if( m_clIndSelSmartControlValveParams.m_dDpMax > 0.0 )
	{
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_clIndSelSmartControlValveParams.m_dDpMax ) );
	}

	if( NULL != pRViewSSelSmartControlValve )
	{
		pRViewSSelSmartControlValve->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSmartControlValve::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel )
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnWaterChange( wParam, lParam );

	if( NULL != pRViewSSelSmartControlValve )
	{
		pRViewSSelSmartControlValve->Reset();
	}

	return 0;
}

CRViewSSelSS *CDlgIndSelSmartControlValve::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelSmartControlValve;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDlgIndSelSmartControlValve::_SuggestIndividualSelection()
{
	if( NULL == pRViewSSelSmartControlValve )
	{
		return false;
	}

	if( NULL != m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList )
	{
		delete m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList;
	}

	m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList = new CSelectSmartControlValveList();

	if( NULL == m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList || NULL == m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->GetSelectPipeList() )
	{
		return false;
	}

	m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->GetSelectPipeList()->SelectPipes( &m_clIndSelSmartControlValveParams, m_clIndSelSmartControlValveParams.m_dFlow );

	bool bSizeShiftProblem = false;
	bool bBestFound = false;
	int iDevFound = m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->SelectSmartControlValve( &m_clIndSelSmartControlValveParams, &bSizeShiftProblem, bBestFound );
	
	if( iDevFound != 0 && false == bSizeShiftProblem )
	{
		VerifyModificationMode();
		pRViewSSelSmartControlValve->Suggest( &m_clIndSelSmartControlValveParams );
	}
	else
	{
		if( false == bSizeShiftProblem )
		{
			m_clIndSelSmartControlValveParams.m_SmartControlValveList.PurgeAll();

			// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
			m_clIndSelSmartControlValveParams.m_pTADB->GetSmartControlValveList( &m_clIndSelSmartControlValveParams.m_SmartControlValveList, (LPCTSTR)m_strTypeID, _T("" ), _T("" ), _T( ""), _T( ""), _T(""),
					m_clIndSelSmartControlValveParams.m_eFilterSelection );
			
			iDevFound = m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->SelectSmartControlValve( &m_clIndSelSmartControlValveParams, &bSizeShiftProblem, bBestFound );

			if( iDevFound > 0 )
			{
				// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevFound' dialog.
				m_clIndSelSmartControlValveParams.m_SmartControlValveList.PurgeAll();
				double dKey = 0.0;

				for( CSelectedValve *pclSelectedSmartControlValve = m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedSmartControlValve;
						pclSelectedSmartControlValve = m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList->GetNext<CSelectedValve>() )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedSmartControlValve->GetpData() );

					if( NULL == pTAP )
					{
						continue;
					}

					m_clIndSelSmartControlValveParams.m_SmartControlValveList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
				}

				// Launch the no device found dialog box if something is found.
				// Remark: Combos in this dialog will be filled thanks to the new alternative shut-off valves found and saved in the 'm_SvList' variable.
				CDlgNoDevSmartControlValveFoundParams clNoDevSmartControlValveParams = CDlgNoDevSmartControlValveFoundParams( &m_clIndSelSmartControlValveParams.m_SmartControlValveList, m_clIndSelSmartControlValveParams.m_strComboMaterialID, 
						m_clIndSelSmartControlValveParams.m_strComboConnectID, m_clIndSelSmartControlValveParams.m_strComboPNID );
				
				CDlgNoDevSmartControlValveFound dlg( &m_clIndSelSmartControlValveParams, &clNoDevSmartControlValveParams );

				if( IDOK == dlg.DoModal() )
				{
					clNoDevSmartControlValveParams = *dlg.GetNoDevFoundSmartControlValveParams();

					_LaunchSuggestion( clNoDevSmartControlValveParams.m_strBodyMaterialID, clNoDevSmartControlValveParams.m_strConnectID, clNoDevSmartControlValveParams.m_strPNID );
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
				CDlgTechParam *pdlg = new CDlgTechParam( CDlgTechParam::PageGeneralDeviceSizes );

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

bool CDlgIndSelSmartControlValve::_SelectIndividualSelection()
{
	try
	{
		if( NULL == m_clIndSelSmartControlValveParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelSmartControlValveParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelSmartControlValveParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelSmartControlValveParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelSmartControlValve )
		{
			HYSELECT_THROW( _T("Internal error: The smart control valve right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The smart control valve right view is not visible.") );
		}

		// Verify pointer validity; user can delete object after entering in edition.
		if( _T('\0') != *m_clIndSelSmartControlValveParams.m_SelIDPtr.ID )
		{
			m_clIndSelSmartControlValveParams.m_SelIDPtr = m_clIndSelSmartControlValveParams.m_pTADS->Get( m_clIndSelSmartControlValveParams.m_SelIDPtr.ID );
		}
	
		// Create selected object and initialize it.
		CDS_SSelSmartControlValve *pSelSmartControlValve = dynamic_cast<CDS_SSelSmartControlValve *>( m_clIndSelSmartControlValveParams.m_SelIDPtr.MP );

		if( NULL == pSelSmartControlValve )
		{
			IDPTR IDPtr;
			m_clIndSelSmartControlValveParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSmartControlValve ) );
			pSelSmartControlValve = (CDS_SSelSmartControlValve *)( IDPtr.MP );
		}
	
		pSelSmartControlValve->SetQ( m_clIndSelSmartControlValveParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelSmartControlValveParams.m_eFlowOrPowerDTMode )
		{
			pSelSmartControlValve->SetFlowDef( CDS_SelProd::efdFlow );
			pSelSmartControlValve->SetPower( 0.0 );
			pSelSmartControlValve->SetDT( 0.0 );
		}
		else
		{
			pSelSmartControlValve->SetFlowDef( CDS_SelProd::efdPower );
			pSelSmartControlValve->SetPower( m_clIndSelSmartControlValveParams.m_dPower );
			pSelSmartControlValve->SetDT( m_clIndSelSmartControlValveParams.m_dDT );
		}

		// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
		pSelSmartControlValve->SetCheckboxDpMax( m_clIndSelSmartControlValveParams.m_bIsDpMaxChecked );
		pSelSmartControlValve->SetDpMax( m_clIndSelSmartControlValveParams.m_dDpMax );

		pSelSmartControlValve->SetSelectedAsAPackage( m_clIndSelSmartControlValveParams.m_bOnlyForSet );
		pSelSmartControlValve->SetMaterialID( m_clIndSelSmartControlValveParams.m_strComboMaterialID );
		pSelSmartControlValve->SetConnectID( m_clIndSelSmartControlValveParams.m_strComboConnectID );
		pSelSmartControlValve->SetPNID( m_clIndSelSmartControlValveParams.m_strComboPNID );
		pSelSmartControlValve->SetPipeSeriesID( m_clIndSelSmartControlValveParams.m_strPipeSeriesID );
		pSelSmartControlValve->SetPipeID( m_clIndSelSmartControlValveParams.m_strPipeID );

		*pSelSmartControlValve->GetpSelectedInfos()->GetpWCData() = m_clIndSelSmartControlValveParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelSmartControlValve->GetpSelectedInfos()->SetDT( m_clIndSelSmartControlValveParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelSmartControlValve->GetpSelectedInfos()->SetApplicationType( m_clIndSelSmartControlValveParams.m_eApplicationType );
	
		pRViewSSelSmartControlValve->FillInSelected( pSelSmartControlValve );

		CDlgConfSel dlg( &m_clIndSelSmartControlValveParams );
		IDPTR IDPtr = pSelSmartControlValve->GetIDPtr();
		dlg.Display( pSelSmartControlValve );
	
		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelSmartControlValveParams.m_pTADS->Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'SMARTCONTROLVALVE_TAB' table from the database.") );
			}

			if( _T('\0') == *m_clIndSelSmartControlValveParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelSmartControlValveParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelSmartControlValve ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelSmartControlValveParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelSmartControlValveParams.m_SelIDPtr.ID )
			{
				m_clIndSelSmartControlValveParams.m_pTADS->DeleteObject( IDPtr );
			}
		}

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelSmartControlValve::_SelectIndividualSelection'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelSmartControlValve::_FillComboBodyMat( CString strBodyMaterialID )
{
	if( NULL == m_clIndSelSmartControlValveParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	m_clIndSelSmartControlValveParams.m_pTADB->GetSmartControlValveBdyMatList( &List, (LPCTSTR)m_strTypeID, (LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboFamilyID,
			m_clIndSelSmartControlValveParams.m_eFilterSelection );

	m_ComboBdyMat.FillInCombo( &List, strBodyMaterialID, m_clIndSelSmartControlValveParams.GetComboMaterialAllID() );
	m_clIndSelSmartControlValveParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSmartControlValve::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_clIndSelSmartControlValveParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	m_clIndSelSmartControlValveParams.m_pTADB->GetSmartControlValveConnList( &List, (LPCTSTR)m_strTypeID, (LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboFamilyID,
			(LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboMaterialID, m_clIndSelSmartControlValveParams.m_eFilterSelection );

	m_ComboConnect.FillInCombo( &List, strConnectID, m_clIndSelSmartControlValveParams.GetComboConnectAllID() );
	m_clIndSelSmartControlValveParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSmartControlValve::_FillComboPN( CString strPNID )
{
	if( NULL == m_clIndSelSmartControlValveParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx List;

	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	m_clIndSelSmartControlValveParams.m_pTADB->GetSmartControlValvePNList( &List, (LPCTSTR)m_strTypeID, (LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboFamilyID,
			(LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboConnectID, (LPCTSTR)m_clIndSelSmartControlValveParams.m_strComboVersionID, 
			m_clIndSelSmartControlValveParams.m_eFilterSelection );

	m_ComboPN.FillInCombo( &List, strPNID, m_clIndSelSmartControlValveParams.GetComboPNAllID() );
	m_clIndSelSmartControlValveParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSmartControlValve::_LaunchSuggestion( CString strMaterialID, CString strConnectID, CString strPNID )
{
	_FillComboBodyMat( strMaterialID );
	_FillComboConnect( strConnectID );
	_FillComboPN( strPNID );

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelSmartControlValve::_VerifySubmittedValues()
{
	bool bValid = false;

	if( m_clIndSelSmartControlValveParams.m_dFlow <= 0.0 )
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
		// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
		if( m_clIndSelSmartControlValveParams.m_dDpMax <= 0.0 )
		{
			m_GroupDpMax.SetCheck( BST_UNCHECKED );
			_UpdateDpMaxFieldState();
		}

	}

	return bValid;
}

void CDlgIndSelSmartControlValve::_UpdateDpMaxFieldState()
{
	m_clIndSelSmartControlValveParams.m_bIsDpMaxChecked = (BST_CHECKED == m_GroupDpMax.GetCheck()) ? true : false;

	if( true == m_clIndSelSmartControlValveParams.m_bIsDpMaxChecked )
	{
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( TRUE );
		m_clExtEditDpMax.SetFocus();
		m_clExtEditDpMax.SetWindowText( _T( "" ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( TRUE );
		m_GroupDpMax.SetForceShowDisable( false );
	}
	else
	{
		m_clIndSelSmartControlValveParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( FALSE );
		m_clExtEditDpMax.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( FALSE );
		m_GroupDpMax.SetForceShowDisable( true );
	}
}
