#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Select.h"

#include "DlgNoDevCtrlFound.h"
#include "DlgConfSel.h"
#include "DlgTechParam.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelCtrl.h"
#include "RViewSSelBCV.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelCtrlBase.h"
#include "DlgIndSelBCV.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelBCV::CDlgIndSelBCV( CWnd *pParent )
	: CDlgIndSelCtrlBase( m_clIndSelBCVParams, CDlgIndSelBCV::IDD, pParent )
{
	m_clIndSelBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_BalAndCtrlValve;
	m_clIndSelBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;
}

void CDlgIndSelBCV::SaveSelectionParameters()
{
	CDlgIndSelCtrlBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelBCVParams.m_pTADS->GetpIndSelParameter();
	
	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetBCVRadioFlowPowerDT( (int)m_clIndSelBCVParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetBCVDpCheckBox( (int)m_clIndSelBCVParams.m_bDpEnabled );
	pclIndSelParameter->SetBCVTypeID( m_clIndSelBCVParams.m_strComboTypeID );
	pclIndSelParameter->SetBCVCB2W3W( m_clIndSelBCVParams.m_eCV2W3W );
	pclIndSelParameter->SetBCVCBCtrlType( m_clIndSelBCVParams.m_eCvCtrlType );
	pclIndSelParameter->SetBCVFamilyID( m_clIndSelBCVParams.m_strComboFamilyID );
	pclIndSelParameter->SetBCVMaterialID( m_clIndSelBCVParams.m_strComboMaterialID );
	pclIndSelParameter->SetBCVConnectID( m_clIndSelBCVParams.m_strComboConnectID );
	pclIndSelParameter->SetBCVVersionID( m_clIndSelBCVParams.m_strComboVersionID );
	pclIndSelParameter->SetBCVPNID( m_clIndSelBCVParams.m_strComboPNID );
	pclIndSelParameter->SetBCVActPowerSupplyID( m_clIndSelBCVParams.m_strActuatorPowerSupplyID );
	pclIndSelParameter->SetBCVActInputSignalID( m_clIndSelBCVParams.m_strActuatorInputSignalID );
	pclIndSelParameter->SetBCVFailSafeFct( m_clIndSelBCVParams.m_iActuatorFailSafeFunction );
	pclIndSelParameter->SetBCVDRPFct( m_clIndSelBCVParams.m_eActuatorDRPFunction );
	pclIndSelParameter->SetBCVPackageChecked( (int)m_clIndSelBCVParams.m_bOnlyForSet );

	if( NULL != pRViewSSelBCv )
	{
		pRViewSSelBCv->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelBCV::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelCtrlBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelBCv )
	{
		pRViewSSelBCv->Reset();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelBCV::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelCtrlBase::DoDataExchange( pDX );

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
	DDX_Control( pDX, IDC_CHECKBCVPACKAGE, m_CheckboxSet );
	DDX_Control( pDX, IDC_GROUPVALVETYPE, m_GroupValveType );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPACTUATOR, m_GroupActuator );
	DDX_Control( pDX, IDC_CHECKFAILSAFE, m_CheckboxFailSafe );

	// 'CDlgIndSelBaseCtrl' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_EDITDP, m_clExtEditDp );
	DDX_Control( pDX, IDC_GROUPDP, m_clGroupDp );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelBCV::OnInitDialog()
{
	CDlgIndSelCtrlBase::OnInitDialog();
	
	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATIC2W3W );
	GetDlgItem( IDC_STATIC2W3W )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICCTRLTYPE );
	GetDlgItem( IDC_STATICCTRLTYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICMATERIAL );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICPOWERSUPPLY );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICINPUTSIGNAL );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_STATICFAILSAFE );
	GetDlgItem( IDC_CHECKFAILSAFE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
	GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_CHECKPACKAGE );
	GetDlgItem( IDC_CHECKBCVPACKAGE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBCV_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );

	m_clGroupDp.SetCheckBoxStyle( BS_AUTOCHECKBOX );
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupDp.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_DP );
	}

	m_clGroupDp.SetInOffice2007Mainframe( true );

	// Set proper style and add icons to groups.
	if( NULL != pclImgListGroupBox )
	{
		m_clGroupQ.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	m_clGroupQ.SetInOffice2007Mainframe( true );
	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
//	m_clGroupQ.SetExpandCollapseMode( true );	

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
	m_CheckStatus = CTADatabase::FailSafeCheckStatus::eFirst;

	return TRUE;  
}

BEGIN_MESSAGE_MAP( CDlgIndSelBCV, CDlgIndSelCtrlBase )
	ON_CBN_SELCHANGE( IDC_COMBO2W3W, OnCbnSelChange2w3w )
	ON_CBN_SELCHANGE( IDC_COMBOCTRLTYPE, OnCbnSelChangeCtrlType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeBodyMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_CBN_SELCHANGE( IDC_COMBOPOWERSUPPLY, OnCbnSelChangePowerSupply )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnCbnSelChangeInputSignal )
	ON_CBN_SELCHANGE( IDC_COMBODRP, OnCbnSelChangeDRPFct )
	ON_BN_CLICKED( IDC_CHECKBCVPACKAGE, OnBnClickedCheckboxSet )
	ON_BN_CLICKED( IDC_CHECKFAILSAFE, OnBnClickedCheckFailSafe )
	ON_COMMAND( IDC_GROUPDP, OnBnClickedCheckDp )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDBCV, OnModifySelectedBCV )
END_MESSAGE_MAP()

void CDlgIndSelBCV::OnCbnSelChange2w3w()
{
	m_clIndSelBCVParams.m_eCV2W3W = CDB_ControlProperties::CV2W3W::CV2W;		// By default in case of error
	int iCur = m_Combo2w3w.GetCurSel();
	
	if( iCur >= 0 )
	{
        m_clIndSelBCVParams.m_eCV2W3W = (CDB_ControlProperties::CV2W3W)m_Combo2w3w.GetItemData( iCur );
	}
	
	_FillComboType();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	OnCbnSelChangeType();
}

void CDlgIndSelBCV::OnCbnSelChangeType()
{
	m_clIndSelBCVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
	FillComboCtrlType();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	OnCbnSelChangeCtrlType();
}

void CDlgIndSelBCV::OnCbnSelChangeCtrlType()
{
	m_clIndSelBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_ComboCtrlType.GetCurSel();
		
	if( iCur >= 0 )
	{
		m_clIndSelBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)m_ComboCtrlType.GetItemData( iCur );
	}

	_FillComboFamily();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	OnCbnSelChangeFamily();
}

void CDlgIndSelBCV::OnCbnSelChangeFamily()
{
	m_clIndSelBCVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	_FillComboBodyMat();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	OnCbnSelChangeBodyMaterial();
}

void CDlgIndSelBCV::OnCbnSelChangeBodyMaterial()
{
	m_clIndSelBCVParams.m_strComboMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
	_FillComboConnect();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'
	// We have thus to do it manually.
	OnCbnSelChangeConnect();
}

void CDlgIndSelBCV::OnCbnSelChangeConnect()
{
	m_clIndSelBCVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	_FillComboVersion();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	OnCbnSelChangeVersion();
}

void CDlgIndSelBCV::OnCbnSelChangeVersion()
{
	m_clIndSelBCVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	_FillComboPN();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	OnCbnSelChangePN();
}

void CDlgIndSelBCV::OnCbnSelChangePN()
{
	m_clIndSelBCVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	_VerifyCheckboxSetStatus();
	_FillComboPowerSupply();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	OnCbnSelChangePowerSupply();
}

void CDlgIndSelBCV::OnCbnSelChangePowerSupply()
{
	m_clIndSelBCVParams.m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
	_FillComboInputSignal();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	OnCbnSelChangeInputSignal();
}

void CDlgIndSelBCV::OnCbnSelChangeInputSignal()
{
	m_clIndSelBCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE' event.
	// We have thus to do it manually.
	_SetCheckFailSafeFct();
	OnBnClickedCheckFailSafe();
}

void CDlgIndSelBCV::OnCbnSelChangeDRPFct()
{
	m_clIndSelBCVParams.m_eActuatorDRPFunction = CDB_ControlValve::drpfUndefined;
	
	if( m_ComboDRPFct.GetCurSel() >= 0 )
	{
        m_clIndSelBCVParams.m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( m_ComboDRPFct.GetCurSel() );
	}
	
	ClearAll();
	EnableSelectButton( false );
}

void CDlgIndSelBCV::OnBnClickedCheckboxSet()
{
	bool fCanClear = true;
	
	if( BST_UNCHECKED == m_CheckboxSet.GetCheck() )
	{
		if( true == m_bAtLeastOneYesBreakOnMsg )
		{
			CString str = TASApp.LoadLocalizedString( AFXMSG_PRODUCTSETBREAK );
			
			if( IDNO == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
			{
				m_CheckboxSet.SetCheck( BST_CHECKED );
				fCanClear = false;
			}
			else
			{
				// User has intentionally break the set
				m_bAutoCheckboxSetManaging = false;
			}
		}
	}

	m_clIndSelBCVParams.m_bOnlyForSet = ( BST_CHECKED == m_CheckboxSet.GetCheck() ) ? true : false;
	
	if( true == fCanClear )
	{
		ClearAll();
	}
}

void CDlgIndSelBCV::OnBnClickedCheckFailSafe()
{
	m_clIndSelBCVParams.m_iActuatorFailSafeFunction = ( BST_CHECKED == m_CheckboxFailSafe.GetCheck() ) ? 1 : 0;
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

void CDlgIndSelBCV::OnBnClickedCheckDp()
{
	m_clIndSelBCVParams.m_bDpEnabled = ( BST_CHECKED == m_clGroupDp.GetCheck() ) ? true : false;
	UpdateDpFieldState();
	ClearAll();
}

LRESULT CDlgIndSelBCV::OnModifySelectedBCV( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelBCv *pSelBCv = dynamic_cast<CDS_SSelBCv *>( (CData *)lParam );
	ASSERT( NULL != pSelBCv );

	if( NULL == pSelBCv || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelBCVParams.m_SelIDPtr = pSelBCv->GetIDPtr();

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
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_BalAndCtrlValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelBCVParams.m_eApplicationType = pSelBCv->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelBCVParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelBCVParams.m_WC = *pSelBCv->GetpSelectedInfos()->GetpWCData();

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelBCv->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	m_clIndSelBCVParams.m_bDpEnabled = ( pSelBCv->GetDp() > 0.0 ) ? true : false;
	m_clGroupDp.SetCheck( ( pSelBCv->GetDp() > 0.0 ) ? BST_CHECKED : BST_UNCHECKED );

	m_clIndSelBCVParams.m_dDp = pSelBCv->GetDp();
	UpdateDpFieldState();
	UpdateDpFieldValue();
	
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelBCv->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.	
	UpdateData( FALSE );
	
	// Remark: 'm_clIndSelBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelBCVParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelBCVParams.m_dFlow = pSelBCv->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelBCVParams.m_dPower = pSelBCv->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelBCVParams.m_dDT = pSelBCv->GetDT();
		UpdateDTFieldValue();
	}
	
	m_clIndSelBCVParams.m_bOnlyForSet = pSelBCv->IsSelectedAsAPackage();
	m_CheckboxSet.SetCheck( ( true == m_clIndSelBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	// By default we enable the 'Set' checkbox. The state of this one will be checked below by calling the '_VerifyCheckboxSetStatus'.
	m_clIndSelBCVParams.m_bCheckboxSetEnabled = true;
	m_CheckboxSet.EnableWindow( TRUE );

	// Remark: Corresponding combo variables in 'm_clIndSelBCVParams' are updated in each of this following methods.
	FillCombo2w3w( pSelBCv->GetCtrlProp() );
	_FillComboType( pSelBCv->GetTypeID() );
	FillComboCtrlType( pSelBCv->GetCtrlType() );
	_FillComboFamily( pSelBCv->GetFamilyID() );
	_FillComboBodyMat( pSelBCv->GetMaterialID() );
	_FillComboConnect( pSelBCv->GetConnectID() );
	_FillComboVersion( pSelBCv->GetVersionID() );
	_FillComboPN( pSelBCv->GetPNID() );

	// We must now verify state of the checkbox before filling combo for actuator.
	_VerifyCheckboxSetStatus();

	_FillComboPowerSupply( pSelBCv->GetActuatorPowerSupplyIDPtr() );
	_FillComboInputSignal( pSelBCv->GetActuatorInputSignalIDPtr() );
	int iCheck = -1;
	if( 1 == pSelBCv->GetFailSafeFunction() )
	{
		iCheck = BST_CHECKED;
	} 
	else if( 0 == pSelBCv->GetFailSafeFunction() )
	{
		iCheck = BST_UNCHECKED;
	}
	ASSERT( iCheck != -1 );
	m_CheckboxFailSafe.SetCheck( iCheck );
	_VerifyCheckboxFailsafeStatus();
	m_clIndSelBCVParams.m_iActuatorFailSafeFunction = pSelBCv->GetFailSafeFunction();
	_FillComboDRPFct( pSelBCv->GetDRPFunction() );

	// Remark: The 'm_clIndSelBCVParams.m_strPipeSeriesID' and 'm_clIndSelBCVParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelBCv->GetPipeSeriesID(), pSelBCv->GetPipeID() );

	pSelBCv->ResetCVActrSetIDPtr();

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );
	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

void CDlgIndSelBCV::OnBnClickedSuggest()
{
	try
	{
		if( NULL == pRViewSSelBCv )
		{
			return;
		}

		if( m_clIndSelBCVParams.m_dFlow <= 0.0 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
			GetpEditFlow()->SetFocus();
			return;
		}

		// Check if Dp value is valid.
		if( CDlgIndSelBase::VDPFlag_DpError == VerifyDpValue() )
		{
			return;
		}

		// Clear the right sheet.
		ResetRightView();

		// Disable select button if we are not in edition mode.
		if( false == m_clIndSelBCVParams.m_bEditModeRunning )
		{
			EnableSelectButton( false );
		}

		// 'OnBnClickedSuggest' is called several time during initialisation; 
		// return if not yet fully initialized
		if( false == m_bInitialised )
		{
			return;
		}

		if( NULL != m_clIndSelBCVParams.m_pclSelectBCVList )
		{
			delete m_clIndSelBCVParams.m_pclSelectBCVList;
		}

		m_clIndSelBCVParams.m_pclSelectBCVList = new CSelectBCVList();

		if( NULL == m_clIndSelBCVParams.m_pclSelectBCVList || NULL == m_clIndSelBCVParams.m_pclSelectBCVList->GetSelectPipeList() )
		{
			return;
		}

		m_clIndSelBCVParams.m_pclSelectBCVList->GetSelectPipeList()->SelectPipes( &m_clIndSelBCVParams, m_clIndSelBCVParams.m_dFlow );

		m_clIndSelBCVParams.m_CtrlList.PurgeAll();

		int iValveCount = m_clIndSelBCVParams.m_pTADB->GetTaCVList(	
				&m_clIndSelBCVParams.m_CtrlList,					// List where to saved
				CTADatabase::eForBCv,								// Control valve target (cv, hmcv, picv or bcv)
				false, 												// 'true' returns as soon a result is found
				m_clIndSelBCVParams.m_eCV2W3W, 						// Set way number of valve
				_T(""),							 					// Type ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID,	// Family ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, 	// Body material ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID,	// Connection ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID, 	// Version ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboPNID,		// PN ID
				CDB_ControlProperties::LastCVFUNC, 					// Set the control function (control only, presettable, ...)
				m_clIndSelBCVParams.m_eCvCtrlType,					// Set the control type (on/off, proportional, ...)
				m_clIndSelBCVParams.m_eFilterSelection,
				0,													// DNMin
				INT_MAX,											// DNMax
				false,												// 'true' if it's for hub station.
				NULL,												// 'pProd'.
				m_clIndSelBCVParams.m_bOnlyForSet );				// 'true' if it's only for a set.

		bool bSizeShiftProblem = false;
		bool bTryAlternative = false;

		if( 0 == iValveCount )
		{
			bTryAlternative = true;
		}
		else
		{
			int iDevFound = m_clIndSelBCVParams.m_pclSelectBCVList->SelectQ( &m_clIndSelBCVParams, &bSizeShiftProblem );

			if( 0 != iDevFound && false == bSizeShiftProblem )
			{
				VerifyModificationMode();
				pRViewSSelBCv->Suggest( &m_clIndSelBCVParams );
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
				CDlgTechParam *pDlgTechParams = new CDlgTechParam( CDlgTechParam::PageGeneralDeviceSizes );

				if( NULL == pDlgTechParams )
				{
					HYSELECT_THROW( _T("Internal error: 'pDlgTechParams' instantiation error.") );
				}
			
				if( IDOK != pDlgTechParams->DoModal() )
				{
					// Set the focus on the first available edit control.
					SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
				}
			
				delete pDlgTechParams;
			}
			else
			{
				// Set the focus on the first available edit control.
				SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelBCV::OnBnClickedSuggest'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelBCV::OnBnClickedSelect()
{
	try
	{
		if( NULL == m_clIndSelBCVParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelBCVParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelBCVParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelBCVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelBCv || FALSE == this->IsWindowVisible() )
		{
			return;
		}

		// Verify pointer validity; user can delete object after entering in Edition.
		if( _T('\0') != *m_clIndSelBCVParams.m_SelIDPtr.ID )
		{
			m_clIndSelBCVParams.m_SelIDPtr = m_clIndSelBCVParams.m_pTADS->Get( m_clIndSelBCVParams.m_SelIDPtr.ID );
		}
	
		// If doesn't exist yet (re-edition), create selected object and initialize it.
		CDS_SSelBCv *pSelBCv = dynamic_cast<CDS_SSelBCv *>( m_clIndSelBCVParams.m_SelIDPtr.MP );

		if( NULL == pSelBCv )
		{
			IDPTR IDPtr;
			m_clIndSelBCVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelBCv ) );
			pSelBCv = (CDS_SSelBCv *)( IDPtr.MP );
		}
	
		pSelBCv->SetQ( m_clIndSelBCVParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelBCVParams.m_eFlowOrPowerDTMode )
		{
			pSelBCv->SetFlowDef( CDS_SelProd::efdFlow );
			pSelBCv->SetPower( 0.0 );
			pSelBCv->SetDT( 0.0 );
		}
		else
		{
			pSelBCv->SetFlowDef( CDS_SelProd::efdPower );
			pSelBCv->SetPower( m_clIndSelBCVParams.m_dPower );
			pSelBCv->SetDT( m_clIndSelBCVParams.m_dDT );
		}
	
		if( m_clIndSelBCVParams.m_dDp > 0.0 )
		{
			pSelBCv->SetDp( m_clIndSelBCVParams.m_dDp );
		}
		else
		{
			pSelBCv->SetDp( 0.0 );
		}

		pSelBCv->SetSelectedAsAPackage(	m_clIndSelBCVParams.m_bOnlyForSet );
		pSelBCv->SetCheckPackageEnable( ( true == m_clIndSelBCVParams.m_bCheckboxSetEnabled ) ? TRUE : FALSE );
		pSelBCv->SetCtrlProp( m_clIndSelBCVParams.m_eCV2W3W );
		pSelBCv->SetTypeID( m_clIndSelBCVParams.m_strComboTypeID );
		pSelBCv->SetCtrlType( m_clIndSelBCVParams.m_eCvCtrlType );
		pSelBCv->SetFamilyID( m_clIndSelBCVParams.m_strComboFamilyID );
		pSelBCv->SetMaterialID( m_clIndSelBCVParams.m_strComboMaterialID );
		pSelBCv->SetConnectID( m_clIndSelBCVParams.m_strComboConnectID );
		pSelBCv->SetVersionID( m_clIndSelBCVParams.m_strComboVersionID );
		pSelBCv->SetPNID( m_clIndSelBCVParams.m_strComboPNID );

		// Set the actuators info.
		pSelBCv->SetActuatorPowerSupplyID( m_clIndSelBCVParams.m_strActuatorPowerSupplyID );
		pSelBCv->SetActuatorInputSignalID( m_clIndSelBCVParams.m_strActuatorInputSignalID );
		pSelBCv->SetFailSafeFunction( m_clIndSelBCVParams.m_iActuatorFailSafeFunction );
		pSelBCv->SetDRPFunction( m_clIndSelBCVParams.m_eActuatorDRPFunction );

		pSelBCv->SetPipeSeriesID( m_clIndSelBCVParams.m_strPipeSeriesID );
		pSelBCv->SetPipeID( m_clIndSelBCVParams.m_strPipeID );

		*pSelBCv->GetpSelectedInfos()->GetpWCData() = m_clIndSelBCVParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelBCv->GetpSelectedInfos()->SetDT( m_clIndSelBCVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelBCv->GetpSelectedInfos()->SetApplicationType( m_clIndSelBCVParams.m_eApplicationType );

		pRViewSSelBCv->FillInSelected( pSelBCv );

		CDlgConfSel dlg( &m_clIndSelBCVParams );
		IDPTR IDPtr = pSelBCv->GetIDPtr();

		dlg.Display( pSelBCv );
	
		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelBCVParams.m_pTADS->Get( _T("CTRLVALVE_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: can't retrieve the 'CTRLVALVE_TAB' table from the datastruct.") );
			}

			if( _T('\0') == *m_clIndSelBCVParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelBCVParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelBCv ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelBCVParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable Select Button
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelBCVParams.m_SelIDPtr.ID )
			{
				m_clIndSelBCVParams.m_pTADS->DeleteObject( IDPtr );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelBCV::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelBCV::OnEnChangeDp() 
{
	if( GetFocus() == GetpEditDp() )
	{
		ClearAll();	
	}
}

LRESULT CDlgIndSelBCV::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnNewDocument( wParam, lParam );

	// Re-init parameters.
	ClearCommonInputEditors();
	
	// Get last selected parameters.
	// Remark: if user clicks create a new document, the 'SaveSelectionParameters' is automatically called before 'OnNewDocument'.
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelBCVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	// Set the radio Flow/PowerDT state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetBCVRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	// Set the Dp check box state.
	m_clIndSelBCVParams.m_bDpEnabled = ( 0 == pclIndSelParameter->GetBCVDpCheckBox() ) ? false : true;
	m_clGroupDp.SetCheck( ( true == m_clIndSelBCVParams.m_bDpEnabled ) ? BST_CHECKED : BST_UNCHECKED );
	CDlgIndSelBase::UpdateDpFieldState();

	m_clIndSelBCVParams.m_bOnlyForSet = ( 0 == pclIndSelParameter->GetBCVPackageChecked() ) ? false : true;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	// By default we enable the 'Set' checkbox. The state of this one will be checked below by calling the '_VerifyCheckboxSetStatus'.
	m_clIndSelBCVParams.m_bCheckboxSetEnabled = true;
	m_CheckboxSet.EnableWindow( ( true == m_clIndSelBCVParams.m_bCheckboxSetEnabled ) ? TRUE : FALSE );

	// Remark: Corresponding combo variables in 'm_clIndSelBCVParams' are updated in each of this following methods.
	FillCombo2w3w( pclIndSelParameter->GetBCVCB2W3W() );
	_FillComboType( pclIndSelParameter->GetBCVTypeID() );
	FillComboCtrlType( pclIndSelParameter->GetBCVCBCtrlType() );
	_FillComboFamily( pclIndSelParameter->GetBCVFamilyID() );
	_FillComboBodyMat( pclIndSelParameter->GetBCVMaterialID() );
	_FillComboConnect( pclIndSelParameter->GetBCVConnectID() );
	_FillComboVersion( pclIndSelParameter->GetBCVVersionID() );
	_FillComboPN( pclIndSelParameter->GetBCVPNID() );

	// We must now verify state of the checkbox before filling combo for actuator.
	_VerifyCheckboxSetStatus();
	
	_FillComboPowerSupply( pclIndSelParameter->GetBCVActPowerSupplyID() );
	_FillComboInputSignal( pclIndSelParameter->GetBCVActInputSignalID() );
	
	int iCheck = -1;
	
	// HYS-1313: In old version, fail safe function was a combo with 4 values (fsfNone, fsfClosing, fsfOpening and fsfAll).
	// It can happen that we read here value of 2 or 3!
	if( 0 == pclIndSelParameter->GetBCVFailSafeFct() )
	{ 
		iCheck = BST_UNCHECKED; 
	}
	else
	{
		iCheck = BST_CHECKED;
	}

	m_CheckboxFailSafe.SetCheck( iCheck );
	m_clIndSelBCVParams.m_iActuatorFailSafeFunction = iCheck;
	_VerifyCheckboxFailsafeStatus();
	_FillComboDRPFct( pclIndSelParameter->GetBCVDRPFct() );

	OnCbnSelChangeDRPFct();

	// Fill the flow and update water characteristic strings.
	OnUnitChange();
	
	EnableSelectButton( false );
	m_bInitialised = true;

	if( NULL != pRViewSSelBCv )
	{
		pRViewSSelBCv->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}

LRESULT CDlgIndSelBCV::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnPipeChange( wParam, lParam );

	if( NULL != pRViewSSelBCv )
	{
		pRViewSSelBCv->Reset();
	}
	
	return 0;
}

LRESULT CDlgIndSelBCV::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	// Clear the right sheet.
	CDlgIndSelCtrlBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelBCv )
	{
		pRViewSSelBCv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelBCV::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	// Call base class to reset units of common controls.
	CDlgIndSelCtrlBase::OnUnitChange( wParam, lParam );

	if( NULL != pRViewSSelBCv )
	{
		pRViewSSelBCv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelBCV::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel )
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnWaterChange(wParam, lParam);
	
	if( NULL != pRViewSSelBCv )
	{
		pRViewSSelBCv->Reset();
	}

	return 0;
}

void CDlgIndSelBCV::FillCombo2w3w( CDB_ControlProperties::CV2W3W eCv2w3w )
{
	// Remark: the 'm_clIndSelBCVParams->m_eCV2WW3' variable is updated in this base method.
	CDlgIndSelCtrlBase::FillCombo2w3w( _T("BALCTRLVALV_TAB"), eCv2w3w );
}

void CDlgIndSelBCV::FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	// Remark: the 'm_clIndSelBCVParams->m_eCvCtrlType' variable is updated in this base method.
	CDlgIndSelCtrlBase::FillComboCtrlType( CTADatabase::eForBCv, eCvCtrlType );
}

CRViewSSelSS *CDlgIndSelBCV::GetLinkedRightViewSSel( void )
{ 
	return pRViewSSelBCv;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelBCV::_FillComboType( CString strTypeID )
{
	CRankEx TypeList;

	m_clIndSelBCVParams.m_pTADB->GetTaCVTypeList( &TypeList, CTADatabase::eForBCv, m_clIndSelBCVParams.m_eCV2W3W, CDB_ControlProperties::LastCVFUNC, 
			CDB_ControlProperties::eCvNU, m_clIndSelBCVParams.m_eFilterSelection );

	m_ComboType.FillInCombo( &TypeList, strTypeID );
	m_clIndSelBCVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBCV::_FillComboFamily( CString strFamilyID )
{
	CRankEx FamList;

	m_clIndSelBCVParams.m_pTADB->GetTaCVFamList( &FamList, CTADatabase::eForBCv, m_clIndSelBCVParams.m_eCV2W3W,_T(""), CDB_ControlProperties::LastCVFUNC, 
			m_clIndSelBCVParams.m_eCvCtrlType, m_clIndSelBCVParams.m_eFilterSelection );

	m_ComboFamily.FillInCombo( &FamList, strFamilyID, m_clIndSelBCVParams.GetComboFamilyAllID() );
	m_clIndSelBCVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBCV::_FillComboBodyMat( CString strBodyMaterialID )
{
	CRankEx BdyMatList;

	m_clIndSelBCVParams.m_pTADB->GetTaCVBdyMatList( &BdyMatList, CTADatabase::eForBCv, m_clIndSelBCVParams.m_eCV2W3W, _T(""), 
			(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID, CDB_ControlProperties::LastCVFUNC, m_clIndSelBCVParams.m_eCvCtrlType, 
			m_clIndSelBCVParams.m_eFilterSelection );

	m_ComboMaterial.FillInCombo( &BdyMatList, strBodyMaterialID,m_clIndSelBCVParams.GetComboMaterialAllID() );
	m_clIndSelBCVParams.m_strComboMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBCV::_FillComboConnect( CString strConnectID )
{
	CRankEx ConnList;

	m_clIndSelBCVParams.m_pTADB->GetTaCVConnList( &ConnList, CTADatabase::eForBCv, m_clIndSelBCVParams.m_eCV2W3W, _T(""), 
			(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, CDB_ControlProperties::LastCVFUNC, 
			m_clIndSelBCVParams.m_eCvCtrlType, m_clIndSelBCVParams.m_eFilterSelection );

	m_ComboConnect.FillInCombo( &ConnList, strConnectID, m_clIndSelBCVParams.GetComboConnectAllID() );
	m_clIndSelBCVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBCV::_FillComboVersion( CString strVersionID )
{
	CRankEx VerList;

	m_clIndSelBCVParams.m_pTADB->GetTaCVVersList( &VerList, CTADatabase::eForBCv, m_clIndSelBCVParams.m_eCV2W3W, _T(""), 
			(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID,(LPCTSTR) 
			m_clIndSelBCVParams.m_strComboConnectID, CDB_ControlProperties::LastCVFUNC, m_clIndSelBCVParams.m_eCvCtrlType, 
			m_clIndSelBCVParams.m_eFilterSelection );

	m_ComboVersion.FillInCombo( &VerList, strVersionID, m_clIndSelBCVParams.GetComboVersionAllID() );
	m_clIndSelBCVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBCV::_FillComboPN( CString strPNID )
{
	CRankEx PNList;

	m_clIndSelBCVParams.m_pTADB->GetTaCVPNList( &PNList, CTADatabase::eForBCv, m_clIndSelBCVParams.m_eCV2W3W, _T(""), 
			(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, 
			(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID, (LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID,
			CDB_ControlProperties::LastCVFUNC, m_clIndSelBCVParams.m_eCvCtrlType, m_clIndSelBCVParams.m_eFilterSelection );

	m_ComboPN.FillInCombo( &PNList, strPNID, m_clIndSelBCVParams.GetComboPNAllID() );
	m_clIndSelBCVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBCV::_FillComboPowerSupply( CString strPowerSupplyID )
{
	CRankEx BCVList;

	m_clIndSelBCVParams.m_pTADB->GetTaCVList(	
			&BCVList,												// List where to saved
			CTADatabase::eForBCv,									// Control valve target (cv, hmcv, picv or bcv)
			false, 													// 'true' returns as soon a result is found
			m_clIndSelBCVParams.m_eCV2W3W, 							// Set way number of valve
			_T(""),							 						// Type ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID,		// Family ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, 		// Body material ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID,		// Connection ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID, 		// Version ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboPNID,			// PN ID
			CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
			m_clIndSelBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelBCVParams.m_eFilterSelection,
			0,														// DNMin
			INT_MAX,												// DNMax
			false,													// 'true' if it's for hub station.
			NULL,													// 'pProd'.
			m_clIndSelBCVParams.m_bOnlyForSet );					// 'true' if it's only for a set.

	CRankEx PowerSupplyList;
	m_clIndSelBCVParams.m_pTADB->GetTaCVActuatorPowerSupplyList( &PowerSupplyList, &BCVList, m_clIndSelBCVParams.m_eCvCtrlType, m_clIndSelBCVParams.m_bOnlyForSet, m_clIndSelBCVParams.m_eFilterSelection );
	m_ComboPowerSupply.FillInCombo( &PowerSupplyList, strPowerSupplyID, m_clIndSelBCVParams.GetComboActuatorPowerSupplyAllID() );
	m_clIndSelBCVParams.m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelBCV::_FillComboInputSignal( CString strInputSignalID )
{
	if( 0 == m_ComboPowerSupply.GetCount() )
	{
		m_ComboInputSignal.ResetContent();
		m_ComboInputSignal.EnableWindow( FALSE );
		m_clIndSelBCVParams.m_strActuatorInputSignalID = _T("");
	}
	else
	{
		CRankEx BCVList;
		m_clIndSelBCVParams.m_pTADB->GetTaCVList(	
				&BCVList,												// List where to saved
				CTADatabase::eForBCv,									// Control valve target (cv, hmcv, picv or bcv)
				false, 													// 'true' returns as soon a result is found
				m_clIndSelBCVParams.m_eCV2W3W, 							// Set way number of valve
				_T(""),							 						// Type ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID,		// Family ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, 		// Body material ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID,		// Connection ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID, 		// Version ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboPNID,			// PN ID
				CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
				m_clIndSelBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSelBCVParams.m_eFilterSelection,
				0,														// DNMin
				INT_MAX,												// DNMax
				false,													// 'true' if it's for hub station.
				NULL,													// 'pProd'.
				m_clIndSelBCVParams.m_bOnlyForSet );					// 'true' if it's only for a set.

		CRankEx InputSignalList;

		m_clIndSelBCVParams.m_pTADB->GetTaCVActuatorInputSignalList( &InputSignalList, &BCVList, m_clIndSelBCVParams.m_eCvCtrlType, 
				m_clIndSelBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelBCVParams.m_strActuatorPowerSupplyID, m_clIndSelBCVParams.m_eFilterSelection );

		m_ComboInputSignal.FillInCombo( &InputSignalList, strInputSignalID, m_clIndSelBCVParams.GetComboActuatorInputSignalAllID() );
		m_clIndSelBCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSelBCV::_SetCheckFailSafeFct()
{
	if( 0 == m_ComboInputSignal.GetCount() )
	{
		// Checkbox unchecked and disabled
		m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
		m_CheckboxFailSafe.EnableWindow( false );
	}
	else
	{
		CRankEx BCVList;
		m_clIndSelBCVParams.m_pTADB->GetTaCVList(
			&BCVList,												// List where to saved
			CTADatabase::eForBCv,									// Control valve target (cv, hmcv, picv or bcv)
			false, 													// 'true' returns as soon a result is found
			m_clIndSelBCVParams.m_eCV2W3W, 							// Set way number of valve
			_T( "" ),							 						// Type ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID,		// Family ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, 		// Body material ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID,		// Connection ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID, 		// Version ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboPNID,			// PN ID
			CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
			m_clIndSelBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelBCVParams.m_eFilterSelection,
			0,														// DNMin
			INT_MAX,												// DNMax
			false,													// 'true' if it's for hub station.
			NULL,													// 'pProd'.
			m_clIndSelBCVParams.m_bOnlyForSet );					// 'true' if it's only for a set.

		m_CheckStatus = m_clIndSelBCVParams.m_pTADB->GetTaCVActuatorFailSafeValues( &BCVList, m_clIndSelBCVParams.m_eCvCtrlType, m_clIndSelBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelBCVParams.m_strActuatorPowerSupplyID,
			(LPCTSTR)m_clIndSelBCVParams.m_strActuatorInputSignalID, m_clIndSelBCVParams.m_eFilterSelection );
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


void CDlgIndSelBCV::_FillComboDRPFct( CDB_ControlValve::DRPFunction eDRPFct )
{
	if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus && false == m_CheckboxFailSafe.IsWindowEnabled() )
	{
		m_ComboDRPFct.ResetContent();
		m_ComboDRPFct.EnableWindow( FALSE );
		m_clIndSelBCVParams.m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfUndefined;
	}
	else
	{
		CRankEx BCVList;
		m_clIndSelBCVParams.m_pTADB->GetTaCVList(	
				&BCVList,												// List where to saved
				CTADatabase::eForBCv,									// Control valve target (cv, hmcv, picv or bcv)
				false, 													// 'true' returns as soon a result is found
				m_clIndSelBCVParams.m_eCV2W3W, 							// Set way number of valve
				_T(""),							 						// Type ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID,		// Family ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID,		// Body material ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID,		// Connection ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID, 		// Version ID
				(LPCTSTR)m_clIndSelBCVParams.m_strComboPNID,			// PN ID
				CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
				m_clIndSelBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSelBCVParams.m_eFilterSelection,
				0,														// DNMin
				INT_MAX,												// DNMax
				false,													// 'true' if it's for hub station.
				NULL,													// 'pProd'.
				m_clIndSelBCVParams.m_bOnlyForSet );					// 'true' if it's only for a set.

		CRankEx DRPFctList;

		m_clIndSelBCVParams.m_pTADB->GetTaCVDRPFunctionList( &DRPFctList, m_clIndSelBCVParams.m_eCvCtrlType, &BCVList, m_clIndSelBCVParams.m_bOnlyForSet,
				(LPCTSTR)m_clIndSelBCVParams.m_strActuatorPowerSupplyID, (LPCTSTR)m_clIndSelBCVParams.m_strActuatorInputSignalID, 
			m_clIndSelBCVParams.m_iActuatorFailSafeFunction, m_clIndSelBCVParams.m_eFilterSelection );

		m_ComboDRPFct.ResetContent();
		int iSelPos = 0;
		
		if( DRPFctList.GetCount() > 0 )
		{
			DRPFctList.Transfer( &m_ComboDRPFct );

			// If we have more than one item in the combo, we can insert "** All fail safe **" string.
			if( m_ComboDRPFct.GetCount() > 1 )
			{
				CString str = TASApp.LoadLocalizedString( m_clIndSelBCVParams.GetComboActuatorFailSafeAllID() );
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
		m_clIndSelBCVParams.m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( iSelPos );

		m_ComboDRPFct.EnableWindow( ( m_ComboDRPFct.GetCount() <= 1 ) ? FALSE : TRUE );
		
	}
}

void CDlgIndSelBCV::_LaunchSuggestion( CDB_ControlProperties::CvCtrlType eCvCtrlType, CString strFamilyID, CString strMaterialID, CString strConnectID, 
			CString strVersionID, CString strPNID, bool bSelectionBySet )
{
	FillComboCtrlType( eCvCtrlType );
	_FillComboFamily( strFamilyID );
	_FillComboBodyMat( strMaterialID );
	_FillComboConnect( strConnectID );
	_FillComboVersion( strVersionID );
	_FillComboPN( strPNID );
	
	m_clIndSelBCVParams.m_bOnlyForSet = bSelectionBySet;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	OnCbnSelChangePN();

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelBCV::_TryAlternativeSuggestion( bool bSizeShiftProblem )
{
	// Variables.
	int iDevFound = 0;
	bool bReturn = false;

	// If no device found try to change the combo to get at least one valid product.
	// If it's the case, show the 'No device found' dialog. Otherwise, show AFXMSG_...
	if( false == bSizeShiftProblem )
	{
		bool bUserPressureDropValid = ( m_clIndSelBCVParams.m_dDp > 0.0 ) ? true : false;

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

		// Remark: start to 1 because the strict case has been already done in the 'OnBnClickedSuggest' method.
		for( int iLoopAlternative = 1; iLoopAlternative < 4 && false == bStop; iLoopAlternative++ )
		{
			m_clIndSelBCVParams.m_CtrlList.PurgeAll();
			int iValveCount = 0;
			bool bSetFlag = ( bitSet == ( iLoopAlternative & bitSet ) ) ? true : false;
			bool bCombosFlag = ( bitCombos == ( iLoopAlternative & bitCombos ) ) ? true : false;

			// We do these cases only if user has selected set.
			if( true == bSetFlag && false == m_clIndSelBCVParams.m_bOnlyForSet )
			{
				continue;
			}

			bool bOnlySet = ( false == bSetFlag ) ? m_clIndSelBCVParams.m_bOnlyForSet : false;

			CString strFamilyID( _T("") );
			CString strBodyMaterialID( _T("") );
			CString strConnectID( _T("") );
			CString strVersionID( _T("") );
			CString strPNID( _T("") );

			if( false == bCombosFlag )
			{
				strFamilyID = m_clIndSelBCVParams.m_strComboFamilyID;
				strBodyMaterialID = m_clIndSelBCVParams.m_strComboMaterialID;
				strConnectID = m_clIndSelBCVParams.m_strComboConnectID;
				strVersionID = m_clIndSelBCVParams.m_strComboVersionID;
				strPNID = m_clIndSelBCVParams.m_strComboPNID;
			}

			iValveCount = m_clIndSelBCVParams.m_pTADB->GetTaCVList(	
					&m_clIndSelBCVParams.m_CtrlList,					// List where to saved
					CTADatabase::eForBCv,								// Control valve target (cv, hmcv, picv or bcv)
					false, 												// 'false' to not return as soon one result is found.
					m_clIndSelBCVParams.m_eCV2W3W, 						// Set way number of valve
					_T( "" ),							 				// Type ID
					(LPCTSTR)strFamilyID,								// Family ID
					(LPCTSTR)strBodyMaterialID, 						// Body material ID
					(LPCTSTR)strConnectID,								// Connection ID
					(LPCTSTR)strVersionID, 								// Version ID
					(LPCTSTR)strPNID,									// PN ID
					CDB_ControlProperties::LastCVFUNC, 					// Set the control function (control only, presettable, ...)
					m_clIndSelBCVParams.m_eCvCtrlType,					// Set the control type (on/off, proportional, ...)
					m_clIndSelBCVParams.m_eFilterSelection,
					0,													// DNMin
					INT_MAX,											// DNMax
					false,												// 'false' because it's not for hub station.
					NULL,												// 'pProd'.
					bOnlySet );											// 'false' if we don't want set.

			if( 0 == iValveCount )
			{
				continue;
			}

			iDevFound = m_clIndSelBCVParams.m_pclSelectBCVList->SelectQ( &m_clIndSelBCVParams, &bSizeShiftProblem );
			
			if( iDevFound > 0 && false == bSizeShiftProblem )
			{
				if( true == bCombosFlag )
				{
					// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevCtrlFound' dialog.
					m_clIndSelBCVParams.m_CtrlList.PurgeAll();
					double dKey = 0.0;

					for( CSelectedValve *pclSelectedValve = m_clIndSelBCVParams.m_pclSelectBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = m_clIndSelBCVParams.m_pclSelectBCVList->GetNext<CSelectedValve>() )
					{
						CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

						if( NULL == pTAP )
						{
							continue;
						}

						m_clIndSelBCVParams.m_CtrlList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
					}

					// Launch the 'No device found' dialog box.
					// Remark: Combos in this dialog will be filled thanks to the new alternative balancing and control valves found and saved in the 
					// 'm_clIndSelBCVParams.m_CtrlList' variable.
					CDlgNoDevCtrlFoundParams clNoDevCtrlParams = CDlgNoDevCtrlFoundParams( &m_clIndSelBCVParams.m_CtrlList, m_clIndSelBCVParams.m_strComboTypeID, 
							m_clIndSelBCVParams.m_eCvCtrlType, m_clIndSelBCVParams.m_strComboFamilyID, m_clIndSelBCVParams.m_strComboMaterialID, 
							m_clIndSelBCVParams.m_strComboConnectID, m_clIndSelBCVParams.m_strComboVersionID, m_clIndSelBCVParams.m_strComboPNID );

					CDlgNoDevCtrlFound dlg( &m_clIndSelBCVParams, &clNoDevCtrlParams );

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
						_LaunchSuggestion( m_clIndSelBCVParams.m_eCvCtrlType, m_clIndSelBCVParams.m_strComboFamilyID, m_clIndSelBCVParams.m_strComboMaterialID, 
								m_clIndSelBCVParams.m_strComboConnectID, m_clIndSelBCVParams.m_strComboVersionID, m_clIndSelBCVParams.m_strComboPNID, bOnlySet );
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

void CDlgIndSelBCV::_VerifyCheckboxSetStatus()
{
	CRankEx BCVList;
	
	m_clIndSelBCVParams.m_pTADB->GetTaCVList(	
			&BCVList,												// List where to saved
			CTADatabase::eForBCv,									// Control valve target (cv, hmcv, picv or bcv)
			false, 													// 'true' returns as soon a result is found
			m_clIndSelBCVParams.m_eCV2W3W, 							// Set way number of valve
			_T(""),							 						// Type ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID,		// Family ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, 		// Body material ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID,		// Connection ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID, 		// Version ID
			(LPCTSTR)m_clIndSelBCVParams.m_strComboPNID,			// PN ID
			CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
			m_clIndSelBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelBCVParams.m_eFilterSelection );

	// Remark: the 'm_clIndSelBCVParams.m_bOnlyForSet' variable is updated in this method.
	VerifyCheckboxSetStatus( &BCVList );

	m_CheckboxSet.SetCheck( ( true == m_clIndSelBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );
	m_CheckboxSet.EnableWindow( ( true == m_clIndSelBCVParams.m_bCheckboxSetEnabled ) ? BST_CHECKED : BST_UNCHECKED );
}

void CDlgIndSelBCV::_VerifyCheckboxFailsafeStatus()
{
	CRankEx BCVList;
	m_clIndSelBCVParams.m_pTADB->GetTaCVList(
		&BCVList,												// List where to saved
		CTADatabase::eForBCv,									// Control valve target (cv, hmcv, picv or bcv)
		false, 													// 'true' returns as soon a result is found
		m_clIndSelBCVParams.m_eCV2W3W, 							// Set way number of valve
		_T( "" ),							 						// Type ID
		(LPCTSTR)m_clIndSelBCVParams.m_strComboFamilyID,		// Family ID
		(LPCTSTR)m_clIndSelBCVParams.m_strComboMaterialID, 		// Body material ID
		(LPCTSTR)m_clIndSelBCVParams.m_strComboConnectID,		// Connection ID
		(LPCTSTR)m_clIndSelBCVParams.m_strComboVersionID, 		// Version ID
		(LPCTSTR)m_clIndSelBCVParams.m_strComboPNID,			// PN ID
		CDB_ControlProperties::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_clIndSelBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
		m_clIndSelBCVParams.m_eFilterSelection,
		0,														// DNMin
		INT_MAX,												// DNMax
		false,													// 'true' if it's for hub station.
		NULL,													// 'pProd'.
		m_clIndSelBCVParams.m_bOnlyForSet );					// 'true' if it's only for a set.

	m_CheckStatus = m_clIndSelBCVParams.m_pTADB->GetTaCVActuatorFailSafeValues( &BCVList, m_clIndSelBCVParams.m_eCvCtrlType, m_clIndSelBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelBCVParams.m_strActuatorPowerSupplyID,
		(LPCTSTR)m_clIndSelBCVParams.m_strActuatorInputSignalID, m_clIndSelBCVParams.m_eFilterSelection );
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
