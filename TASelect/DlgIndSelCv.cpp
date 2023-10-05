#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "Select.h"

#include "DlgNoDevCtrlFound.h"
#include "DlgConfSel.h"
#include "DlgTechParam.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelCtrl.h"
#include "RViewSSelCv.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelCtrlBase.h"
#include "DlgIndSelCv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelCv::CDlgIndSelCv( CWnd *pParent )
	: CDlgIndSelCtrlBase( m_clIndSelCVParams, CDlgIndSelCv::IDD, pParent )
{
	m_clIndSelCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_ControlValve;
	m_clIndSelCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_iRadioKvsOrDp = Radio_Kvs;
}

void CDlgIndSelCv::SaveSelectionParameters()
{
	CDlgIndSelCtrlBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelCVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetCvRadioFlowPowerDT( (int)m_clIndSelCVParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetCvRadioKvs( (int)m_clIndSelCVParams.m_eRadioKvsOrDp );
	pclIndSelParameter->SetCvCheckDpKvs( (int)m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked );
	pclIndSelParameter->SetCvCB2W3W( m_clIndSelCVParams.m_eCV2W3W );
	pclIndSelParameter->SetCvTypeID( m_clIndSelCVParams.m_strComboTypeID );
	pclIndSelParameter->SetCvCBCtrlType( m_clIndSelCVParams.m_eCvCtrlType );
	pclIndSelParameter->SetCvFamilyID( m_clIndSelCVParams.m_strComboFamilyID );
	pclIndSelParameter->SetCvMaterialID( m_clIndSelCVParams.m_strComboMaterialID );
	pclIndSelParameter->SetCvConnectID( m_clIndSelCVParams.m_strComboConnectID );
	pclIndSelParameter->SetCvVersionID( m_clIndSelCVParams.m_strComboVersionID );
	pclIndSelParameter->SetCvPNID( m_clIndSelCVParams.m_strComboPNID );
	pclIndSelParameter->SetCvActPowerSupplyID( m_clIndSelCVParams.m_strActuatorPowerSupplyID );
	pclIndSelParameter->SetCvActInputSignalID( m_clIndSelCVParams.m_strActuatorInputSignalID );
	pclIndSelParameter->SetCvFailSafeFct( m_clIndSelCVParams.m_iActuatorFailSafeFunction );
	pclIndSelParameter->SetCvDRPFct( m_clIndSelCVParams.m_eActuatorDRPFunction );
	pclIndSelParameter->SetCvPackageChecked( (int)m_clIndSelCVParams.m_bOnlyForSet );

	if( NULL != pRViewSSelCv )
	{
		pRViewSSelCv->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelCv::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelCtrlBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelCv )
	{
		pRViewSSelCv->Reset();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelCv, CDlgIndSelCtrlBase )
	ON_CBN_SELCHANGE( IDC_COMBO2W3W, OnCbnSelChange2w3w )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOCTRLTYPE, OnCbnSelChangeCtrlType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_CBN_SELCHANGE( IDC_COMBOKVS,OnCbnSelChangeKvs )
	ON_CBN_EDITCHANGE( IDC_COMBOKVS, OnCbnEditChangeKvs )
	ON_BN_CLICKED( IDC_GROUPKVDP, OnBnClickedCheckGroupKvsOrDp )
	ON_BN_CLICKED( IDC_RADIOKVS, OnBnClickedRadioKvsOrDp )
	ON_BN_CLICKED( IDC_RADIODP, OnBnClickedRadioKvsOrDp )
	ON_CBN_SELCHANGE( IDC_COMBOPOWERSUPPLY, OnCbnSelChangePowerSupply )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnCbnSelChangeInputSignal )
	ON_CBN_SELCHANGE( IDC_COMBODRP, OnCbnSelChangeDRPFct )
	ON_BN_CLICKED( IDC_CHECKCVPACKAGE, OnBnClickedCheckboxSet )
	ON_BN_CLICKED( IDC_CHECKFAILSAFE, OnBnClickedCheckFailSafe )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDCV, OnModifySelectedCV )
END_MESSAGE_MAP()

void CDlgIndSelCv::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelCtrlBase::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_COMBOKVS, m_ComboKvs );
	DDX_Control( pDX, IDC_COMBO2W3W, m_Combo2w3w );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOCTRLTYPE, m_ComboCtrlType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboMaterial );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
	DDX_Control( pDX, IDC_COMBOPOWERSUPPLY, m_ComboPowerSupply );
	DDX_Control( pDX, IDC_COMBOINPUTSIGNAL, m_ComboInputSignal );
	DDX_Control( pDX, IDC_COMBODRP, m_ComboDRPFct );
	DDX_Control( pDX, IDC_CHECKCVPACKAGE, m_CheckboxSet );
	DDX_Control( pDX, IDC_GROUPKVDP, m_GroupKvDp );
	DDX_Control( pDX, IDC_GROUPVALVETYPE, m_GroupValveType );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPACTUATOR, m_GroupActuator );
	DDX_Radio( pDX, IDC_RADIOKVS, m_iRadioKvsOrDp );
	DDX_Control( pDX, IDC_CHECKFAILSAFE, m_CheckboxFailSafe );

	// 'CDlgIndSelBase' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_EDITDP, m_clExtEditDp );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelCv::OnInitDialog()
{
	CDlgIndSelCtrlBase::OnInitDialog();
	
	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );

	_SetKvsCvString( );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_RADIODP );
	GetDlgItem( IDC_RADIODP )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATIC2W3W );
	GetDlgItem( IDC_STATIC2W3W )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICCTRLTYPE );
	GetDlgItem( IDC_STATICCTRLTYPE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICMATERIAL );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICPOWERSUPPLY );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICINPUTSIGNAL );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_STATICFAILSAFE );
	GetDlgItem( IDC_CHECKFAILSAFE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
	GetDlgItem( IDC_STATICDRP )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_CHECKPACKAGE );
	GetDlgItem( IDC_CHECKCVPACKAGE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELCV_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );

	// Set proper style and add icons to groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupQ.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	m_clGroupQ.SetInOffice2007Mainframe( true );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupKvDp.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pen );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupKvDp.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Pen, true );
	}

	m_GroupKvDp.SetInOffice2007Mainframe( true );
	m_GroupKvDp.SetCheckBoxStyle( BS_AUTOCHECKBOX );
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

	m_CheckStatus = CTADatabase::FailSafeCheckStatus::eFirst;

	m_GroupActuator.SetInOffice2007Mainframe( true );

	return TRUE;  
}

void CDlgIndSelCv::OnCbnSelChange2w3w()
{
	m_clIndSelCVParams.m_eCV2W3W = CDB_ControlProperties::CV2W3W::CV2W;		// By default in case of error
	int iCur = m_Combo2w3w.GetCurSel();
	
	if( iCur >= 0 )
	{
        m_clIndSelCVParams.m_eCV2W3W = (CDB_ControlProperties::CV2W3W)m_Combo2w3w.GetItemData( iCur );
	}

	_FillComboType();
	OnCbnSelChangeType();
}

void CDlgIndSelCv::OnCbnSelChangeType()
{
	m_clIndSelCVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
	FillComboCtrlType();

	OnCbnSelChangeCtrlType();
}

void CDlgIndSelCv::OnCbnSelChangeCtrlType()
{
	m_clIndSelCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_ComboCtrlType.GetCurSel();
		
	if( iCur >= 0 )
	{
		m_clIndSelCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)m_ComboCtrlType.GetItemData( iCur );
	}

	_FillComboFamily();
	OnCbnSelChangeFamily();
}

void CDlgIndSelCv::OnCbnSelChangeFamily()
{
	m_clIndSelCVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	_FillComboBodyMat();
	OnCbnSelChangeMaterial();
}

void CDlgIndSelCv::OnCbnSelChangeMaterial()
{
	m_clIndSelCVParams.m_strComboMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
	_FillComboConnect();
	OnCbnSelChangeConnect();
}

void CDlgIndSelCv::OnCbnSelChangeConnect()
{
	m_clIndSelCVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgIndSelCv::OnCbnSelChangeVersion()
{
	m_clIndSelCVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	_FillComboPN();
	OnCbnSelChangePN();
}

void CDlgIndSelCv::OnCbnSelChangePN()
{
	m_clIndSelCVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;

	// Do it only if activated.
	if( true == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked && CDS_SSelCv::KvsOrDp::Kvs == m_clIndSelCVParams.m_eRadioKvsOrDp )
	{
		_FillComboKvs( m_clIndSelCVParams.m_dKvs );
		OnCbnSelChangeKvs();
	}

	_VerifyCheckPackageStatus();
	_FillComboPowerSupply();
	OnCbnSelChangePowerSupply();
}

void CDlgIndSelCv::OnCbnSelChangeKvs()
{
	ReadCUDouble( _C_KVCVCOEFF, m_ComboKvs, &m_clIndSelCVParams.m_dKvs );
}

void CDlgIndSelCv::OnCbnEditChangeKvs()
{
	OnCbnSelChangeKvs();
}

void CDlgIndSelCv::OnBnClickedCheckGroupKvsOrDp()
{
	_UpdateGroupKvsOrDp();
	_UpdateRadioKvsOrDp();
	CDlgIndSelBase::UpdateDpFieldState();

	// Clear the right sheet.
	ClearAll();
}

void CDlgIndSelCv::OnBnClickedRadioKvsOrDp()
{
	_UpdateRadioKvsOrDp();
	CDlgIndSelBase::UpdateDpFieldState();

	// Clear the right sheet.
	ClearAll();
}

void CDlgIndSelCv::OnCbnSelChangePowerSupply()
{
	m_clIndSelCVParams.m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
	_FillComboInputSignal();
	OnCbnSelChangeInputSignal();
}

void CDlgIndSelCv::OnCbnSelChangeInputSignal()
{
	m_clIndSelCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	_SetCheckFailSafeFct();
	OnBnClickedCheckFailSafe();
}

void CDlgIndSelCv::OnCbnSelChangeDRPFct()
{
	m_clIndSelCVParams.m_eActuatorDRPFunction = CDB_ControlValve::drpfUndefined;
	
	if( m_ComboDRPFct.GetCurSel() >= 0 )
	{
        m_clIndSelCVParams.m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( m_ComboDRPFct.GetCurSel() );
	}

	ClearAll();
	EnableSelectButton( false );
}

void CDlgIndSelCv::OnBnClickedCheckboxSet()
{
	m_clIndSelCVParams.m_bOnlyForSet = ( BST_CHECKED == m_CheckboxSet.GetCheck() ) ? true : false;
	ClearAll();
}

void CDlgIndSelCv::OnBnClickedCheckFailSafe()
{
	m_clIndSelCVParams.m_iActuatorFailSafeFunction = ( BST_CHECKED == m_CheckboxFailSafe.GetCheck() ) ? 1 : 0;
	SetCheckStatus( CTADatabase::FailSafeCheckStatus::eBothFailSafe );

	if( BST_CHECKED == m_CheckboxFailSafe.GetCheck() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_FAILSAFEPOSITION );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}

	_FillComboDRPFct();
	OnCbnSelChangeDRPFct();
}

LRESULT CDlgIndSelCv::OnModifySelectedCV( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelCv *pSelCv = dynamic_cast<CDS_SSelCv *>( (CData *)lParam );
	ASSERT( NULL != pSelCv );
	
	if( NULL == pSelCv || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelCVParams.m_SelIDPtr = pSelCv->GetIDPtr();

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
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_ControlValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelCVParams.m_eApplicationType = pSelCv->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelCVParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelCVParams.m_WC = *( pSelCv->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelCv->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	m_clIndSelCVParams.m_bOnlyForSet = pSelCv->IsSelectedAsAPackage();
	m_CheckboxSet.SetCheck( ( true == m_clIndSelCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );
	
	// By default we enable the 'Set' checkbox. The state of this one will be checked below by calling the '_VerifyCheckboxSetStatus'.
	m_clIndSelCVParams.m_bCheckboxSetEnabled = true;
	m_CheckboxSet.EnableWindow( TRUE );

	m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked = ( BST_CHECKED == pSelCv->GetCheckBoxKvsDp() ) ? true : false;
	m_GroupKvDp.SetCheck( pSelCv->GetCheckBoxKvsDp() );
	
	m_clIndSelCVParams.m_eRadioKvsOrDp = pSelCv->GetRadioKvsOrDp();
	m_iRadioKvsOrDp = ( CDS_SSelCv::KvsOrDp::Kvs == m_clIndSelCVParams.m_eRadioKvsOrDp ) ? Radio_Kvs : Radio_Dp;


	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioKvsOrDp' variable.	
	UpdateData( FALSE );

	_UpdateGroupKvsOrDp();
	_UpdateRadioKvsOrDp();
	CDlgIndSelBase::UpdateDpFieldState();

	// Remark: Corresponding combo variables in 'm_clIndSelCVParams' are updated in each of this following methods.
	FillCombo2w3w( pSelCv->GetCtrlProp() );
	_FillComboType( pSelCv->GetTypeID() );
	FillComboCtrlType( pSelCv->GetCtrlType() );
	_FillComboFamily( pSelCv->GetFamilyID() );
	_FillComboBodyMat( pSelCv->GetMaterialID() );
	_FillComboConnect( pSelCv->GetConnectID() );
	_FillComboVersion( pSelCv->GetVersionID() );
	_FillComboPN( pSelCv->GetPNID() );

	// We must now verify state of the checkbox before filling combo for actuator.
	_VerifyCheckPackageStatus();

	_FillComboPowerSupply( pSelCv->GetActuatorPowerSupplyIDPtr() );
	_FillComboInputSignal( pSelCv->GetActuatorInputSignalIDPtr() );
	int iCheck = -1;
	if( 1 == pSelCv->GetFailSafeFunction() )
	{
		iCheck = BST_CHECKED;
	}
	else if( 0 == pSelCv->GetFailSafeFunction() )
	{
		iCheck = BST_UNCHECKED;
	}
	ASSERT( iCheck != -1 );
	m_CheckboxFailSafe.SetCheck( iCheck );
	_VerifyCheckboxFailsafeStatus();
	m_clIndSelCVParams.m_iActuatorFailSafeFunction = pSelCv->GetFailSafeFunction();
	_FillComboDRPFct( pSelCv->GetDRPFunction() );

	// Remark: The 'm_clIndSelCVParams.m_strPipeSeriesID' and 'm_clIndSelCVParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelCv->GetPipeSeriesID(), pSelCv->GetPipeID() );
	
	if( true == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked )
	{
		if( CDS_SSelCv::KvsOrDp::Kvs == m_clIndSelCVParams.m_eRadioKvsOrDp )
		{
			m_clIndSelCVParams.m_dKvs = pSelCv->GetKvs();
			_FillComboKvs( m_clIndSelCVParams.m_dKvs );
		}
		else
		{
			m_clIndSelCVParams.m_dDp = pSelCv->GetDp();
			UpdateDpFieldValue();
		}
	}

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelCv->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.	
	UpdateData( FALSE );

	// Remark: 'm_clIndSelCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();
	
	if( CDS_SelProd::efdFlow == m_clIndSelCVParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelCVParams.m_dFlow = pSelCv->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelCVParams.m_dPower = pSelCv->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelCVParams.m_dDT = pSelCv->GetDT();
		UpdateDTFieldValue();
	}

	pSelCv->ResetCVActrSetIDPtr();

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );

	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

void CDlgIndSelCv::OnBnClickedSuggest()
{
	if( NULL == pRViewSSelCv )
	{
		return;
	}

	if( false == m_bInitialised )
	{
		return;
	}
		
	if( m_clIndSelCVParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetpEditFlow()->SetFocus();
		return;
	}

	// Dp-Kvs group checked.
	if( true == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked )
	{
		if( CDS_SSelCv::KvsOrDp::Kvs == m_clIndSelCVParams.m_eRadioKvsOrDp )
		{
			if( m_clIndSelCVParams.m_dKvs <= 0.0 )
			{
				CString	str;
				CString tmp;
				CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

				if( 0 != pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				{
					tmp = TASApp.LoadLocalizedString( IDS_KVSERIES_CV );
				}
				else
				{
					tmp = TASApp.LoadLocalizedString( IDS_KVS );
				}

				FormatString( str, AFXMSG_NOKV_KVCHECKED, tmp );

				if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
				{
					m_GroupKvDp.SetCheck( BST_UNCHECKED );

					// We update first the state of the Kvs or Dp group.
					_UpdateGroupKvsOrDp();

					// Next we update the state of the 'Kvs' radio and the 'Dp' edit field.
					_UpdateRadioKvsOrDp();
					CDlgIndSelBase::UpdateDpFieldState();
				}
				else
				{
					return;
				}
			}
		}
		else
		{
			int iDpField = VerifyDpValue();

			if( CDlgIndSelBase::VDPFlag_DpError == iDpField )
			{
				return;
			}
			else if( CDlgIndSelBase::VDPFlag_DpFieldToDisable == iDpField )
			{
				// We need also to uncheck GroupKvDp button.
				m_GroupKvDp.SetCheck( BST_UNCHECKED );

				// We update first the state of the Kvs or Dp group.
				_UpdateGroupKvsOrDp();

				// Next we update the state of the 'Kvs' radio and the 'Dp' edit field.
				_UpdateRadioKvsOrDp();
				CDlgIndSelBase::UpdateDpFieldState();
			}
		}
	}

	// Clear the right sheet.
	ResetRightView();

	// Disable the 'Select' button.
	if( false == m_clIndSelCVParams.m_bEditModeRunning )
	{
		EnableSelectButton(false);
	}

	// Create the Select CVList to verify if at least one device exist.
	if( m_clIndSelCVParams.m_pclSelectCVList != NULL )
	{
		delete m_clIndSelCVParams.m_pclSelectCVList;
	}

	m_clIndSelCVParams.m_pclSelectCVList = new CSelectCVList();

	if( NULL == m_clIndSelCVParams.m_pclSelectCVList || NULL == m_clIndSelCVParams.m_pclSelectCVList->GetSelectPipeList() )
	{
		return;
	}

	m_clIndSelCVParams.m_pclSelectCVList->GetSelectPipeList()->SelectPipes( &m_clIndSelCVParams, m_clIndSelCVParams.m_dFlow );
	
	m_clIndSelCVParams.m_CtrlList.PurgeAll();

	int iValveCount = m_clIndSelCVParams.m_pTADB->GetTaCVList(	
			&m_clIndSelCVParams.m_CtrlList,						// List where to saved
			CTADatabase::eForSSelCv,							// Control valve target (cv, hmcv, picv or bcv)
			false,												// 'true' returns as soon a result is found
			m_clIndSelCVParams.m_eCV2W3W,						// Set way number of valve
			(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,		// Type ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID,		// Family ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, 	// Body material ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,	// Connection ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID, 	// Version ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboPNID,			// PN ID
			CDB_ControlProperties::LastCVFUNC,					// Set the control function (control only, presettable, ...)
			m_clIndSelCVParams.m_eCvCtrlType,					// Set the control type (on/off, proportional, ...)
			m_clIndSelCVParams.m_eFilterSelection,
			0,													// DNMin
			INT_MAX,											// DNMax
			false,												// 'true' if it's for hub station.
			NULL,												// 'pProd'.
			m_clIndSelCVParams.m_bOnlyForSet );					// 'true' if it's only for a set.
	
	bool bSizeShiftProblem = false;
	bool bTryAlternative = false;

	if( 0 == iValveCount )
	{
		bTryAlternative = true;
	}
	else
	{
		int iDevFound = 0;
	
		if( m_clIndSelCVParams.m_dKvs > 0.0 )
		{
			iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectCvKvs( &m_clIndSelCVParams, &bSizeShiftProblem );
		}
		else if( m_clIndSelCVParams.m_dDp > 0.0 )
		{
			iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectCvDp( &m_clIndSelCVParams, &bSizeShiftProblem );
		}
		else
		{
			iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectQ( &m_clIndSelCVParams, &bSizeShiftProblem );
		}
	
		if( iDevFound > 0 && false == bSizeShiftProblem )
		{
			VerifyModificationMode();
			pRViewSSelCv->Suggest( &m_clIndSelCVParams );
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

void CDlgIndSelCv::OnBnClickedSelect()
{
	try
	{
		if( NULL == m_clIndSelCVParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelCVParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelCVParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelCVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelCv )
		{
			HYSELECT_THROW( _T("Internal error: The control valve right view is not created.") );
		}
		if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The control valve right view is not visible.") );
		}

		// Verify pointer validity; user can delete object after entering in Edition.
		if( _T('\0') != *m_clIndSelCVParams.m_SelIDPtr.ID )
		{
			m_clIndSelCVParams.m_SelIDPtr = m_clIndSelCVParams.m_pTADS->Get( m_clIndSelCVParams.m_SelIDPtr.ID );
		}
	
		// Create selected object and initialize it.
		CDS_SSelCv *pSelCv = dynamic_cast<CDS_SSelCv *>( m_clIndSelCVParams.m_SelIDPtr.MP );
	
		if( NULL == pSelCv )
		{
			IDPTR IDPtr;
			m_clIndSelCVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelCv ) );
			pSelCv = (CDS_SSelCv *)IDPtr.MP;
		}
	
		if( m_clIndSelCVParams.m_dFlow <= 0.0 )
		{
			HYSELECT_THROW( _T("Internal error: Flow '%f' can't be lower or equal to 0."), m_clIndSelCVParams.m_dFlow );
		}

		pSelCv->SetQ( m_clIndSelCVParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelCVParams.m_eFlowOrPowerDTMode )
		{
			pSelCv->SetFlowDef( CDS_SelProd::efdFlow );
			pSelCv->SetPower( 0.0 );
			pSelCv->SetDT( 0.0 );
		}
		else
		{
			pSelCv->SetFlowDef( CDS_SelProd::efdPower );
			pSelCv->SetPower( m_clIndSelCVParams.m_dPower );
			pSelCv->SetDT( m_clIndSelCVParams.m_dDT );
		}

		pSelCv->SetSelectedAsAPackage( m_clIndSelCVParams.m_bOnlyForSet );
		pSelCv->SetCheckPackageEnable( ( true == m_clIndSelCVParams.m_bCheckboxSetEnabled ) ? TRUE : FALSE );
	
		pSelCv->SetCheckBoxKvsDp( m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked );	
		pSelCv->SetRadioKvsOrDp( m_clIndSelCVParams.m_eRadioKvsOrDp );
		pSelCv->SetKvs( m_clIndSelCVParams.m_dKvs );
		pSelCv->SetDp( m_clIndSelCVParams.m_dDp );
		pSelCv->SetCtrlProp( m_clIndSelCVParams.m_eCV2W3W );
		pSelCv->SetTypeID( m_clIndSelCVParams.m_strComboTypeID );
		pSelCv->SetCtrlType( m_clIndSelCVParams.m_eCvCtrlType );
		pSelCv->SetFamilyID( m_clIndSelCVParams.m_strComboFamilyID );
		pSelCv->SetMaterialID( m_clIndSelCVParams.m_strComboMaterialID );
		pSelCv->SetConnectID( m_clIndSelCVParams.m_strComboConnectID );
		pSelCv->SetVersionID( m_clIndSelCVParams.m_strComboVersionID );
		pSelCv->SetPNID( m_clIndSelCVParams.m_strComboPNID );
	
			// Set the actuators info.
		pSelCv->SetActuatorPowerSupplyID( m_clIndSelCVParams.m_strActuatorPowerSupplyID );
		pSelCv->SetActuatorInputSignalID( m_clIndSelCVParams.m_strActuatorInputSignalID );
		pSelCv->SetFailSafeFunction( m_clIndSelCVParams.m_iActuatorFailSafeFunction );
		pSelCv->SetDRPFunction( m_clIndSelCVParams.m_eActuatorDRPFunction );

		pSelCv->SetPipeSeriesID( m_clIndSelCVParams.m_strPipeSeriesID );
		pSelCv->SetPipeID( m_clIndSelCVParams.m_strPipeID );

		*pSelCv->GetpSelectedInfos()->GetpWCData() = m_clIndSelCVParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelCv->GetpSelectedInfos()->SetDT( m_clIndSelCVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelCv->GetpSelectedInfos()->SetApplicationType( m_clIndSelCVParams.m_eApplicationType );
	
		pRViewSSelCv->FillInSelected( pSelCv );

		CDlgConfSel dlg( &m_clIndSelCVParams );
		IDPTR IDPtr = pSelCv->GetIDPtr();
	
		dlg.Display( pSelCv );
	
		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelCVParams.m_pTADS->Get( _T("CTRLVALVE_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTRLVALVE_TAB' table from the datastruct.") );
			}

			if( _T('\0') == *m_clIndSelCVParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelCVParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelCv ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelCVParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable Select Button
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelCVParams.m_SelIDPtr.ID )
			{
				m_clIndSelCVParams.m_pTADS->DeleteObject( IDPtr );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelCv::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelCv::OnEnChangeDp() 
{
	if( GetFocus() == GetpEditDp() )
	{
		ClearAll();
	}
}

void CDlgIndSelCv::OnEnKillFocusFlow()
{
	CDlgIndSelCtrlBase::OnEnKillFocusFlow();
	m_bFlowEditing = false;
}

LRESULT CDlgIndSelCv::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnNewDocument( wParam, lParam );
	ClearCommonInputEditors();

	if( false == TASApp.IsCvDisplayed() )
	{
		return 0;
	}

	// Get last selected parameters.
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelCVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_clIndSelCVParams.m_bOnlyForSet = ( 0 == pclIndSelParameter->GetCvPackageChecked() ) ? false : true;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	// By default we enable the 'Set' checkbox. The state of this one will be checked below by calling the '_VerifyCheckboxSetStatus'.
	m_clIndSelCVParams.m_bCheckboxSetEnabled = true;
	m_CheckboxSet.EnableWindow( TRUE );

	// Set the radio Flow/PowerDT state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetCvRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	// Set the Kvs or Dp group check.
	m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked = ( 0 == pclIndSelParameter->GetCvCheckDpKvs() ) ? false : true;
	m_GroupKvDp.SetCheck( pclIndSelParameter->GetCvCheckDpKvs() );

	m_clIndSelCVParams.m_eRadioKvsOrDp = (CDS_SSelCv::KvsOrDp)pclIndSelParameter->GetCvRadioKvs(); 
	m_iRadioKvsOrDp = ( CDS_SSelCv::KvsOrDp::Kvs == m_clIndSelCVParams.m_eRadioKvsOrDp ) ? Radio_Kvs : Radio_Dp;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioKvsOrDp' variable.	
	UpdateData( FALSE );
	OnBnClickedCheckGroupKvsOrDp();

	// Remark: Corresponding combo variables in 'm_clIndSelCVParams' are updated in each of this following methods.
	FillCombo2w3w( pclIndSelParameter->GetCvCB2W3W() );
	_FillComboType( pclIndSelParameter->GetCvTypeID() );
	FillComboCtrlType( pclIndSelParameter->GetCvCBCtrlType() );
	_FillComboFamily( pclIndSelParameter->GetCvFamilyID() );
	_FillComboBodyMat( pclIndSelParameter->GetCvMaterialID() );
	_FillComboConnect( pclIndSelParameter->GetCvConnectID() );
	_FillComboVersion( pclIndSelParameter->GetCvVersionID() );
	_FillComboPN( pclIndSelParameter->GetCvPNID() );
	
	// We must now verify state of the checkbox before filling combo for actuator.
	_VerifyCheckPackageStatus();

	_FillComboPowerSupply( pclIndSelParameter->GetCvActPowerSupplyID() );
	_FillComboInputSignal( pclIndSelParameter->GetCvActInputSignalID() );
	
	int iCheck = -1;

	// HYS-1313: In old version, fail safe function was a combo with 4 values (fsfNone, fsfClosing, fsfOpening and fsfAll).
	// It can happen that we read here value of 2 or 3!
	if( 0 == pclIndSelParameter->GetCvFailSafeFct() )
	{
		iCheck = BST_UNCHECKED;
	}
	else
	{
		iCheck = BST_CHECKED;
	}

	m_CheckboxFailSafe.SetCheck( iCheck );
	m_clIndSelCVParams.m_iActuatorFailSafeFunction = iCheck;
	_VerifyCheckboxFailsafeStatus();
	_FillComboDRPFct( pclIndSelParameter->GetCvDRPFct() );

	OnCbnSelChangeDRPFct();
	
	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();

	EnableSelectButton( false );
	m_bInitialised = true;

	if( NULL != pRViewSSelCv )
	{
		pRViewSSelCv->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}

LRESULT CDlgIndSelCv::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnPipeChange( wParam, lParam );
	
	if( NULL != pRViewSSelCv )
	{
		pRViewSSelCv->Reset();
	}

	return 0;
}


LRESULT CDlgIndSelCv::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelCv )
	{
		pRViewSSelCv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelCv::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnUnitChange( wParam, lParam );

	if( NULL != pRViewSSelCv )
	{
		pRViewSSelCv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelCv::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) 
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnWaterChange( wParam, lParam );

	if( NULL != pRViewSSelCv )
	{
		pRViewSSelCv->Reset();
	}

	return 0;
}

void CDlgIndSelCv::FillCombo2w3w( CDB_ControlProperties::CV2W3W eCv2w3w )
{
	CDlgIndSelCtrlBase::FillCombo2w3w( _T("CTRLVALV_TAB"), eCv2w3w );
}

void CDlgIndSelCv::FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	CDlgIndSelCtrlBase::FillComboCtrlType( CTADatabase::eForSSelCv, eCvCtrlType );
}

CRViewSSelSS *CDlgIndSelCv::GetLinkedRightViewSSel( void )
{ 
	return pRViewSSelCv;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelCv::_FillComboType( CString strTypeID )
{
	CRankEx TypeList;

	m_clIndSelCVParams.m_pTADB->GetTaCVTypeList( &TypeList, CTADatabase::eForSSelCv, m_clIndSelCVParams.m_eCV2W3W, CDB_ControlProperties::LastCVFUNC, 
			CDB_ControlProperties::eCvNU, m_clIndSelCVParams.m_eFilterSelection );

	m_ComboType.FillInCombo( &TypeList, strTypeID );
	m_clIndSelCVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelCv::_FillComboFamily( CString strFamilyID )
{
	CRankEx FamList;

	m_clIndSelCVParams.m_pTADB->GetTaCVFamList(	&FamList, CTADatabase::eForSSelCv, m_clIndSelCVParams.m_eCV2W3W, (LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,
			CDB_ControlProperties::LastCVFUNC, m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_eFilterSelection );	
	
	m_ComboFamily.FillInCombo( &FamList, strFamilyID, m_clIndSelCVParams.GetComboFamilyAllID() );
	m_clIndSelCVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelCv::_FillComboBodyMat( CString strBodyMaterialID )
{
	CRankEx BdyMatList;

	m_clIndSelCVParams.m_pTADB->GetTaCVBdyMatList( &BdyMatList, CTADatabase::eForSSelCv, m_clIndSelCVParams.m_eCV2W3W, (LPCTSTR)m_clIndSelCVParams.m_strComboTypeID, 
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID, CDB_ControlProperties::LastCVFUNC, m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_eFilterSelection );
	
	m_ComboMaterial.FillInCombo( &BdyMatList, strBodyMaterialID, m_clIndSelCVParams.GetComboMaterialAllID() );
	m_clIndSelCVParams.m_strComboMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelCv::_FillComboConnect( CString strConnectID )
{
	CRankEx ConnList;

	m_clIndSelCVParams.m_pTADB->GetTaCVConnList( &ConnList, CTADatabase::eForSSelCv, m_clIndSelCVParams.m_eCV2W3W, (LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, CDB_ControlProperties::LastCVFUNC, m_clIndSelCVParams.m_eCvCtrlType,
			m_clIndSelCVParams.m_eFilterSelection );

	m_ComboConnect.FillInCombo( &ConnList, strConnectID, m_clIndSelCVParams.GetComboConnectAllID() );
	m_clIndSelCVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelCv::_FillComboVersion( CString strVersionID )
{
	CRankEx VerList;

	m_clIndSelCVParams.m_pTADB->GetTaCVVersList( &VerList, CTADatabase::eForSSelCv, m_clIndSelCVParams.m_eCV2W3W, (LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,
			CDB_ControlProperties::LastCVFUNC, m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_eFilterSelection );

	m_ComboVersion.FillInCombo( &VerList, strVersionID, m_clIndSelCVParams.GetComboVersionAllID() );
	m_clIndSelCVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelCv::_FillComboPN( CString strPNID )
{
	CRankEx PNList;

	m_clIndSelCVParams.m_pTADB->GetTaCVPNList( &PNList, CTADatabase::eForSSelCv, m_clIndSelCVParams.m_eCV2W3W, (LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID, CDB_ControlProperties::LastCVFUNC, m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_eFilterSelection );
	
	m_ComboPN.FillInCombo( &PNList, strPNID, m_clIndSelCVParams.GetComboPNAllID() );
	m_clIndSelCVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

int CDlgIndSelCv::_FillComboKvs( double dKvs )
{
	CRankEx CvList;
	
	m_clIndSelCVParams.m_pTADB->GetTaCVList( &CvList, CTADatabase::eForSSelCv, false, m_clIndSelCVParams.m_eCV2W3W, (LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID, (LPCTSTR)m_clIndSelCVParams.m_strComboPNID, CDB_ControlProperties::LastCVFUNC, m_clIndSelCVParams.m_eCvCtrlType, 
			m_clIndSelCVParams.m_eFilterSelection );
	
	CRank KvsList;
	_string str;
	LPARAM pItemdata;
	int iSelItem = 0;
	double dKvs1 = DBL_MAX;
	double dKvs2 = DBL_MAX;
	CDS_TechnicalParameter *pTechP = m_clIndSelCVParams.m_pTADS->GetpTechParams();
	
	if( m_clIndSelCVParams.m_dFlow < 0.0 )
	{
		m_clIndSelCVParams.m_dFlow = 0.0;
	}

	for( BOOL bContinue = CvList.GetFirst( str, pItemdata ); TRUE == bContinue; bContinue = CvList.GetNext( str, pItemdata ) )
	{
		CDB_ControlValve *pCV = (CDB_ControlValve *)pItemdata;
		ASSERT( NULL != pCV );

		double dKey = pCV->GetKvs();
		
		// Check if already exist.
		if( TRUE == KvsList.IfExist( dKey ) )
		{
			continue;
		}
		
		// Keep Kvs value that return a Dp below that pTechP->GetCVMaxDispDp();
		if( m_clIndSelCVParams.m_dFlow > 0.0 )
		{
			double dDp = 0.0;
			dDp = CalcDp( m_clIndSelCVParams.m_dFlow, dKey, m_clIndSelCVParams.m_WC.GetDens() );

			if( dDp > pTechP->GetCVMaxDispDp() )
			{
				continue;
			}
			
			if( ( CDB_ControlProperties::eCvOnOff == m_clIndSelCVParams.m_eCvCtrlType && dDp < pTechP->GetCVMinDpOnoff() ) || 
				( CDB_ControlProperties::eCvProportional == m_clIndSelCVParams.m_eCvCtrlType && dDp < pTechP->GetCVMinDpProp() ) )
			{
				if( dKvs1 > dKey )
				{
					dKvs2 = dKvs1;
					dKvs1 = dKey;
				}
				else if( dKvs1 != dKey && dKvs2 > dKey )
				{
					dKvs2 = dKey;
				}

				continue;
			}
		}
		
		// dKvs1 and dKvs2 will always be at minimum equal to the biggest Kvs.
		if( dKvs1 < dKey )
		{
			dKvs1 = dKey;
		}

		if( dKvs2 < dKey )
		{
			dKvs2 = dKey;
		}

		// Add Kvs.
		CString KvsStr = WriteCUDouble( _C_KVCVCOEFF, dKey );
		LPARAM KvsIdata = (LPARAM)( pCV->GetKvs() * 1000.0 );
		KvsList.Add( KvsStr, dKey, KvsIdata );
	}
	
	// Add 2 more Kvs value to go lower than the minimum Dp.
	if( dKvs1 != DBL_MAX && FALSE == KvsList.IfExist( dKvs1 ) )
	{
		// Add Kvs.
		CString KvsStr = WriteCUDouble( _C_KVCVCOEFF, dKvs1 );
		LPARAM KvsIdata = (LPARAM)( dKvs1 * 1000.0 );
		KvsList.Add( KvsStr, dKvs1, KvsIdata );
	}
	
	if( dKvs2 != DBL_MAX && FALSE == KvsList.IfExist( dKvs2 ) )
	{
		// Add Kvs.
		CString KvsStr = WriteCUDouble( _C_KVCVCOEFF, dKvs2 );
		LPARAM KvsIdata = (LPARAM)( dKvs2 * 1000.0 );
		KvsList.Add( KvsStr, dKvs2, KvsIdata );
	}

	KvsList.Transfer( &m_ComboKvs );
	
	if( dKvs > 0.0 )
	{
		m_ComboKvs.SetWindowText( WriteCUDouble( _C_KVCVCOEFF, dKvs ) );
	}
	else
	{
		m_ComboKvs.SetCurSel( 0 );
	}
	
	// If 'Kvs' radio or 'KvsOrDp' group is unchecked disable the combo 'Kvs'.
	if( false == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked || CDS_SSelCv::KvsOrDp::Kvs != m_clIndSelCVParams.m_eRadioKvsOrDp )
	{
		m_ComboKvs.EnableWindow( FALSE );
	}

	ReadCUDouble( _C_KVCVCOEFF, m_ComboKvs, &m_clIndSelCVParams.m_dKvs );
	return m_ComboKvs.GetCount();
}

void CDlgIndSelCv::_FillComboPowerSupply( CString strPowerSupplyID )
{
	// HYS-1448 : Cv type have to be considered
	// HYS-1381 : Cv Set value have to be considered
	CRankEx CVList;
	m_clIndSelCVParams.m_pTADB->GetTaCVList(
			&CVList,												// List where to saved
			CTADatabase::eForSSelCv,								// Control valve target (cv, hmcv, picv or bcv)
			false, 													// 'true' returns as soon a result is found
			m_clIndSelCVParams.m_eCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,    		// Type ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID,			// Family ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, 		// Body material ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,		// Connection ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID, 		// Version ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboPNID,				// PN ID
			CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
			m_clIndSelCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelCVParams.m_eFilterSelection,
			0,													// DNMin
			INT_MAX,											// DNMax
			false,												// 'true' if it's for hub station.
			NULL,												// 'pProd'.
			m_clIndSelCVParams.m_bOnlyForSet );					// 'true' if it's only for a set.);

	CRankEx PowerSupplyList;

	m_clIndSelCVParams.m_pTADB->GetTaCVActuatorPowerSupplyList( &PowerSupplyList, &CVList, m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_bOnlyForSet, m_clIndSelCVParams.m_eFilterSelection );
	m_ComboPowerSupply.FillInCombo( &PowerSupplyList, strPowerSupplyID, m_clIndSelCVParams.GetComboActuatorPowerSupplyAllID() );
	m_clIndSelCVParams.m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelCv::_FillComboInputSignal( CString strInputSignalID )
{
	if( 0 == m_ComboPowerSupply.GetCount() )
	{
		m_ComboInputSignal.ResetContent();
		m_ComboInputSignal.EnableWindow( FALSE );
		m_clIndSelCVParams.m_strActuatorInputSignalID = _T("");
	}
	else
	{
		// HYS-1448 : Cv type have to be considered
		// HYS-1381 : Cv Set value have to be considered
		CRankEx CVList;
		m_clIndSelCVParams.m_pTADB->GetTaCVList(
				&CVList,												// List where to saved
				CTADatabase::eForSSelCv,								// Control valve target (cv, hmcv, picv or bcv)
				false, 													// 'true' returns as soon a result is found
				m_clIndSelCVParams.m_eCV2W3W, 							// Set way number of valve
				(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,			// Type ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID,			// Family ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, 		// Body material ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,		// Connection ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID, 		// Version ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboPNID,				// PN ID
				CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
				m_clIndSelCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSelCVParams.m_eFilterSelection,
				0,													// DNMin
				INT_MAX,											// DNMax
				false,												// 'true' if it's for hub station.
				NULL,												// 'pProd'.
				m_clIndSelCVParams.m_bOnlyForSet );

		CRankEx InputSignalList;
		m_clIndSelCVParams.m_pTADB->GetTaCVActuatorInputSignalList( &InputSignalList, &CVList, m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_bOnlyForSet, 
				(LPCTSTR)m_clIndSelCVParams.m_strActuatorPowerSupplyID, m_clIndSelCVParams.m_eFilterSelection );

		m_ComboInputSignal.FillInCombo( &InputSignalList, strInputSignalID, m_clIndSelCVParams.GetComboActuatorInputSignalAllID() );
		m_clIndSelCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSelCv::_SetCheckFailSafeFct()
{
	if( 0 == m_ComboInputSignal.GetCount() )
	{
		// Checkbox unchecked and disabled
		m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
		m_CheckboxFailSafe.EnableWindow( false );
	}
	else
	{
		// HYS-1448 : Cv type have to be considered
		// HYS-1381 : Cv Set value have to be considered
		CRankEx CVList;
		m_clIndSelCVParams.m_pTADB->GetTaCVList(
			&CVList,												// List where to saved
			CTADatabase::eForSSelCv,								// Control valve target (cv, hmcv, picv or bcv)
			false, 													// 'true' returns as soon a result is found
			m_clIndSelCVParams.m_eCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,			// Type ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID,			// Family ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID, 		// Body material ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,		// Connection ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID, 		// Version ID
			(LPCTSTR)m_clIndSelCVParams.m_strComboPNID,				// PN ID
			CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
			m_clIndSelCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelCVParams.m_eFilterSelection,
			0,													// DNMin
			INT_MAX,											// DNMax
			false,												// 'true' if it's for hub station.
			NULL,												// 'pProd'.
			m_clIndSelCVParams.m_bOnlyForSet );

		m_CheckStatus = m_clIndSelCVParams.m_pTADB->GetTaCVActuatorFailSafeValues( &CVList, m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_bOnlyForSet,
			(LPCTSTR)m_clIndSelCVParams.m_strActuatorPowerSupplyID, (LPCTSTR)m_clIndSelCVParams.m_strActuatorInputSignalID, m_clIndSelCVParams.m_eFilterSelection );
		if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus )
		{
			// Checkbox unchecked and disabled
			m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			m_CheckboxFailSafe.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithoutFailSafe == m_CheckStatus )
		{
			// Checkbox unchecked and enable
			m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			m_CheckboxFailSafe.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithFailSafe == m_CheckStatus )
		{
			// Checkbox checked and enable
			m_CheckboxFailSafe.SetCheck( BST_CHECKED );
			m_CheckboxFailSafe.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eBothFailSafe == m_CheckStatus )
		{
			// Checkbox checked and enable
			m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			m_CheckboxFailSafe.EnableWindow( true );
		}
	}
}

void CDlgIndSelCv::_FillComboDRPFct( CDB_ControlValve::DRPFunction eDRPFct )
{
	if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus && false == m_CheckboxFailSafe.IsWindowEnabled() )
	{
		m_ComboDRPFct.ResetContent();
		m_ComboDRPFct.EnableWindow( FALSE );
		m_clIndSelCVParams.m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfUndefined;
	}
	else
	{
		// HYS-1448 : Cv type have to be considered
		// HYS-1381 : Cv Set value have to be considered
		CRankEx CVList;
		m_clIndSelCVParams.m_pTADB->GetTaCVList(
				&CVList,												// List where to saved
				CTADatabase::eForSSelCv,								// Control valve target (cv, hmcv, picv or bcv)
				false, 													// 'true' returns as soon a result is found
				m_clIndSelCVParams.m_eCV2W3W, 							// Set way number of valve
				(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,			// Type ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID,			// Family ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID,		// Body material ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,		// Connection ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID, 		// Version ID
				(LPCTSTR)m_clIndSelCVParams.m_strComboPNID,				// PN ID
				CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
				m_clIndSelCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSelCVParams.m_eFilterSelection,
				0,													// DNMin
				INT_MAX,											// DNMax
				false,												// 'true' if it's for hub station.
				NULL,												// 'pProd'.
				m_clIndSelCVParams.m_bOnlyForSet );

		CRankEx DRPFctList;
		m_clIndSelCVParams.m_pTADB->GetTaCVDRPFunctionList( &DRPFctList, m_clIndSelCVParams.m_eCvCtrlType, &CVList, m_clIndSelCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelCVParams.m_strActuatorPowerSupplyID,
				(LPCTSTR)m_clIndSelCVParams.m_strActuatorInputSignalID, m_clIndSelCVParams.m_iActuatorFailSafeFunction, m_clIndSelCVParams.m_eFilterSelection );

		m_ComboDRPFct.ResetContent();
		int iSelPos = 0;

		if( DRPFctList.GetCount() > 0 )
		{
			DRPFctList.Transfer( &m_ComboDRPFct );

			// If we have more than one item in the combo, we can insert "** All fail safe **" string.
			if( m_ComboDRPFct.GetCount() > 1 )
			{
				CString str = TASApp.LoadLocalizedString( m_clIndSelCVParams.GetComboActuatorFailSafeAllID() );
				m_ComboDRPFct.InsertString( 0, str );
                m_ComboDRPFct.SetItemData( 0, CDB_ControlValve::DRPFunction::drpfAll );
			}

			if( m_ComboDRPFct.GetCount() > 1 && eDRPFct >= CDB_ControlValve::DRPFunction::drpfNone && eDRPFct < CDB_ControlValve::DRPFunction::drpfLast )
			{
				// If we have only one 1 item, this one is set at the position 0.
				// If we have more than one 1 item, "** All fail safe **" has been inserted at the position 0. Thus first valid item begins at 1.
				int i = ( 1 == m_ComboDRPFct.GetCount() ) ? 0 : 1;
				
				for( ; i < m_ComboDRPFct.GetCount(); i++ )
				{
                    if( eDRPFct == (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( i ) )
					{
						iSelPos = i;
						break;
					}
				}
			}
		}

		m_ComboDRPFct.SetCurSel( iSelPos );
		m_clIndSelCVParams.m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( iSelPos );

		m_ComboDRPFct.EnableWindow( ( m_ComboDRPFct.GetCount() <= 1 ) ? FALSE : TRUE );
	}
}

void CDlgIndSelCv::_LaunchSuggestion( CDB_ControlProperties::CvCtrlType eCvCtrlType, CString strFamilyID, CString strMaterialID, CString strConnectID, 
			CString strVersionID, CString strPNID, bool bSelectionBySet )
{
	FillComboCtrlType( eCvCtrlType );
	_FillComboFamily( strFamilyID );
	_FillComboBodyMat( strMaterialID );
	_FillComboConnect( strConnectID );
	_FillComboVersion( strVersionID );
	_FillComboPN( strPNID );

	m_clIndSelCVParams.m_bOnlyForSet = bSelectionBySet;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	OnCbnSelChangePN();

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelCv::_TryAlternativeSuggestion( bool bSizeShiftProblem )
{
	// Variables.
	int iDevFound = 0;
	bool bReturn = false;

	// If no device found try to change the combo to get at least one valid product.
	// If it's the case, show the 'No device found' dialog. Otherwise, show AFXMSG_...
	if( 0 == iDevFound && false == bSizeShiftProblem )
	{
		// Combos | Set |
		// -------+-----+---------------------------------------
		//    0   |  0  | Strict
		//    0   |  1  | If set is 'true' -> try without set
		//    1   |  0  | Relax combos
		//    1   |  1  | Relax combos and set
		enum bitCriterion
		{
			bitSet = 1,
			bitCombos = 2
		};

		bool bStop = false;

		// Remark: start to 1 because the strict case has been already done.
		for( int iLoopAlternative = 1; iLoopAlternative < 4 && false == bStop; iLoopAlternative++ )
		{
			m_clIndSelCVParams.m_CtrlList.PurgeAll();
			int iValveCount = 0;
			bool bSetFlag = ( bitSet == ( iLoopAlternative & bitSet ) ) ? true : false;
			bool bRelaxCombo = ( bitCombos == ( iLoopAlternative & bitCombos ) ) ? true : false;

			// We do these cases only if user has selected set.
			if( true == bSetFlag && false == m_clIndSelCVParams.m_bOnlyForSet )
			{
				continue;
			}

			bool bOnlySet = ( false == bSetFlag ) ? m_clIndSelCVParams.m_bOnlyForSet : false;

			CString strFamilyID( _T( "" ) );
			CString strBodyMaterialID( _T( "" ) );
			CString strConnectID( _T( "" ) );
			CString strVersionID( _T( "" ) );
			CString strPNID( _T( "" ) );

			if( false == bRelaxCombo )
			{
				strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
				strBodyMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
				strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
				strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
				strPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
			}

			// Launch the no device found dialog box if something is found when all the combos are open.
			iValveCount = m_clIndSelCVParams.m_pTADB->GetTaCVList(	
					&m_clIndSelCVParams.m_CtrlList,						// List where to saved
					CTADatabase::eForSSelCv,							// Control valve target (cv, hmcv, picv or bcv)
					false, 												// 'true' returns as soon a result is found
					m_clIndSelCVParams.m_eCV2W3W,						// Set way number of valve
					(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID, 		// Type ID
					(LPCTSTR)strFamilyID,								// Family ID
					(LPCTSTR)strBodyMaterialID, 						// Body material ID
					(LPCTSTR)strConnectID,								// Connection ID
					(LPCTSTR)strVersionID, 								// Version ID
					(LPCTSTR)strPNID,									// PN ID
					CDB_ControlProperties::LastCVFUNC, 					// Set the control function (control only, presettable, ...)
					m_clIndSelCVParams.m_eCvCtrlType, 					// Set the control type (on/off, proportional, ...)
					m_clIndSelCVParams.m_eFilterSelection,
					0,													// DNMin
					INT_MAX,											// DNMax
					false,												// 'true' if it's for hub station.
					NULL,												// 'pProd'.
					bOnlySet );											// 'true' if it's only for a set.
					
			if( 0 == iValveCount )
			{
				continue;
			}

			if( 0.0 != m_clIndSelCVParams.m_dKvs )
			{
				iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectCvKvs( &m_clIndSelCVParams, &bSizeShiftProblem );
			}
			else if( 0.0 != m_clIndSelCVParams.m_dDp )
			{
				iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectCvDp( &m_clIndSelCVParams, &bSizeShiftProblem );
			}
			else
			{
				iDevFound = m_clIndSelCVParams.m_pclSelectCVList->SelectQ( &m_clIndSelCVParams, &bSizeShiftProblem );
			}
		
			if( iDevFound > 0 && false == bSizeShiftProblem )
			{
				if( true == bRelaxCombo )
				{
					// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevCtrlFound' dialog.
					m_clIndSelCVParams.m_CtrlList.PurgeAll();
					double dKey = 0.0;

					for( CSelectedValve *pclSelectedValve = m_clIndSelCVParams.m_pclSelectCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = m_clIndSelCVParams.m_pclSelectCVList->GetNext<CSelectedValve>() )
					{
						CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

						if( NULL == pTAP )
						{
							continue;
						}

						m_clIndSelCVParams.m_CtrlList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
					}

					// Launch the 'No device found' dialog box.
					// Remark: Combos in this dialog will be filled thanks to the new alternative control valves found and saved in the 'CvList' variable.
					CDlgNoDevCtrlFoundParams clNoDevCtrlParams = CDlgNoDevCtrlFoundParams( &m_clIndSelCVParams.m_CtrlList, m_clIndSelCVParams.m_strComboTypeID, 
							m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_strComboFamilyID, m_clIndSelCVParams.m_strComboMaterialID, 
							m_clIndSelCVParams.m_strComboConnectID, m_clIndSelCVParams.m_strComboVersionID, m_clIndSelCVParams.m_strComboPNID );

					CDlgNoDevCtrlFound dlg( &m_clIndSelCVParams, &clNoDevCtrlParams );
					
					if( IDOK == dlg.DoModal() )
					{
						clNoDevCtrlParams = *dlg.GetNoDevCtrlFoundParams();

						_LaunchSuggestion( clNoDevCtrlParams.m_eCvCtrlType, clNoDevCtrlParams.m_strFamilyID, clNoDevCtrlParams.m_strMaterialID, clNoDevCtrlParams.m_strConnectID,
								clNoDevCtrlParams.m_strVersionID, clNoDevCtrlParams.m_strPNID, bOnlySet );
					}

					bStop = true;
				}
				else
				{
					if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_INDSEL_CTRLVALVENOTFOUND_NOSET, MB_YESNO | MB_ICONQUESTION, 0 ) )
					{
						_LaunchSuggestion( m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_strComboFamilyID, m_clIndSelCVParams.m_strComboMaterialID, 
								m_clIndSelCVParams.m_strComboConnectID, m_clIndSelCVParams.m_strComboVersionID, m_clIndSelCVParams.m_strComboPNID, bOnlySet );
					}

					bStop = true;
				}
			}
		}

		if( false == bStop )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NO_DEV_FOUND );
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

void CDlgIndSelCv::_VerifyCheckPackageStatus()
{
	CRankEx CVList;
	m_clIndSelCVParams.m_pTADB->GetTaCVList(
			&CVList, 
			CTADatabase::eForSSelCv, 
			false, 
			m_clIndSelCVParams.m_eCV2W3W, 
			(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID,
			(LPCTSTR)m_clIndSelCVParams.m_strComboPNID,
			CDB_ControlProperties::LastCVFUNC, 
			m_clIndSelCVParams.m_eCvCtrlType, 
			m_clIndSelCVParams.m_eFilterSelection	);

	// Remark: the 'm_clIndSelCVParams.m_bOnlyForSet' variable is updated in this method.
	VerifyCheckboxSetStatus( &CVList );

	m_CheckboxSet.SetCheck( ( true == m_clIndSelCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );
	m_CheckboxSet.EnableWindow( ( true == m_clIndSelCVParams.m_bCheckboxSetEnabled ) ? BST_CHECKED : BST_UNCHECKED );
}

void CDlgIndSelCv::_SetKvsCvString()
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	CString str;

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_KVS );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_CV );
	}

	GetDlgItem( IDC_RADIOKVS )->SetWindowText( str );
}

void CDlgIndSelCv::_UpdateGroupKvsOrDp()
{
	m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked = ( BST_CHECKED == m_GroupKvDp.GetCheck() ) ? true : false;

	GetDlgItem( IDC_RADIOKVS )->EnableWindow( ( true == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked ) ? TRUE : FALSE );
	GetDlgItem( IDC_RADIODP )->EnableWindow( ( true == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked ) ? TRUE : FALSE );
	m_GroupKvDp.SetForceShowDisable( !m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked );

	if( false == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked )
	{
		m_clIndSelCVParams.m_dKvs = 0.0;
		m_clIndSelCVParams.m_dDp = 0.0;
	}
}

void CDlgIndSelCv::_UpdateRadioKvsOrDp()
{
	// 'TRUE' to fill 'm_iRadioKvsOrDp' variable with the radio state in the dialog.
	UpdateData( TRUE );

	if( true == m_clIndSelCVParams.m_bIsGroupKvsOrDpChecked )
	{
		if( Radio_Kvs == m_iRadioKvsOrDp )
		{
			// Kvs combo to enable.
			m_ComboKvs.EnableWindow( TRUE );

			m_clIndSelCVParams.m_eRadioKvsOrDp = CDS_SSelCv::KvsOrDp::Kvs;

			ReadCUDouble( _C_KVCVCOEFF, m_ComboKvs, &m_clIndSelCVParams.m_dKvs );
			_FillComboKvs( m_clIndSelCVParams.m_dKvs );

			m_clIndSelCVParams.m_dDp = 0.0;

			// Even if the 'm_clIndSelCVParams.m_eRadioKvsOrDp' is enough to know if it's the 'Kvs' combo or the 'Dp' edit
			// that is enabled, we need the 'm_clIndSelCVParams.m_bDpEnabled' to call the 'CDlgIndSelBase::UpdateDpFieldState' method.
			m_clIndSelCVParams.m_bDpEnabled = false;
		}
		else
		{
			// Kvs combo to disable.
			m_ComboKvs.EnableWindow( FALSE );
			m_ComboKvs.ResetContent();

			m_clIndSelCVParams.m_eRadioKvsOrDp = CDS_SSelCv::KvsOrDp::Dp;
			m_clIndSelCVParams.m_dKvs = 0.0;

			// Same remark as above.
			m_clIndSelCVParams.m_bDpEnabled = true;
		}
	}
	else
	{
		// Kvs combo to disable.
		m_ComboKvs.EnableWindow( FALSE );
		m_clIndSelCVParams.m_dKvs = 0.0;
		
		m_clIndSelCVParams.m_dDp = 0.0;

		// Same remark as above.
		m_clIndSelCVParams.m_bDpEnabled = false;
	}
}

void CDlgIndSelCv::_VerifyCheckboxFailsafeStatus()
{
	CRankEx CVList;
	m_clIndSelCVParams.m_pTADB->GetTaCVList(
		&CVList,
		CTADatabase::eForSSelCv,
		false,
		m_clIndSelCVParams.m_eCV2W3W,
		(LPCTSTR)m_clIndSelCVParams.m_strComboTypeID,
		(LPCTSTR)m_clIndSelCVParams.m_strComboFamilyID,
		(LPCTSTR)m_clIndSelCVParams.m_strComboMaterialID,
		(LPCTSTR)m_clIndSelCVParams.m_strComboConnectID,
		(LPCTSTR)m_clIndSelCVParams.m_strComboVersionID,
		(LPCTSTR)m_clIndSelCVParams.m_strComboPNID,
		CDB_ControlProperties::LastCVFUNC,
		m_clIndSelCVParams.m_eCvCtrlType,
		m_clIndSelCVParams.m_eFilterSelection );

	m_CheckStatus = m_clIndSelCVParams.m_pTADB->GetTaCVActuatorFailSafeValues( &CVList, m_clIndSelCVParams.m_eCvCtrlType, m_clIndSelCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelCVParams.m_strActuatorPowerSupplyID,
		(LPCTSTR)m_clIndSelCVParams.m_strActuatorInputSignalID, m_clIndSelCVParams.m_eFilterSelection );
	if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus ||
		CTADatabase::FailSafeCheckStatus::eOnlyWithoutFailSafe == m_CheckStatus ||
		CTADatabase::FailSafeCheckStatus::eOnlyWithFailSafe == m_CheckStatus )
	{
		m_CheckboxFailSafe.EnableWindow( false );
	}
	else if( CTADatabase::FailSafeCheckStatus::eBothFailSafe == m_CheckStatus )
	{
		m_CheckboxFailSafe.EnableWindow( true );
	}

	if( BST_CHECKED == m_CheckboxFailSafe.GetCheck() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_FAILSAFEPOSITION );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
}