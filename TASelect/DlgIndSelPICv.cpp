#include "stdafx.h"

#include "TASelect.h"
#include "DlgSelectionComboHelper.h"

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
#include "RViewSSelPICv.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelCtrlBase.h"
#include "DlgIndSelPICv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelPICv::CDlgIndSelPICv( CWnd *pParent )
	: CDlgIndSelCtrlBase( m_clIndSelPIBCVParams, CDlgIndSelPICv::IDD, pParent )
{
	m_clIndSelPIBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_PressureIndepCtrlValve;
	m_clIndSelPIBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_pclSelComboHelperPIBCV = new CDlgSelComboHelperPIBCV( &m_clIndSelPIBCVParams, CDlgSelComboHelperBase::SelType_Individual );
	
	if( NULL == m_pclSelComboHelperPIBCV )
	{
		ASSERT( 0 );
	}
}

CDlgIndSelPICv::~CDlgIndSelPICv()
{
	if( NULL != m_pclSelComboHelperPIBCV )
	{
		delete m_pclSelComboHelperPIBCV;
	}
}

void CDlgIndSelPICv::SetDpMax( double dDpMax )
{
	if( dDpMax > 0.0 )
	{
		m_clIndSelPIBCVParams.m_dDpMax = dDpMax;
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, dDpMax ) );
	}
	else
	{
		m_clIndSelPIBCVParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetWindowText( _T( "" ) );
	}

	m_clExtEditDpMax.Update();
}

void CDlgIndSelPICv::SaveSelectionParameters()
{
	CDlgIndSelCtrlBase::SaveSelectionParameters();

	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelPIBCVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetPICvRadioFlowPowerDT( (int)m_clIndSelPIBCVParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetPICvTypeID( m_clIndSelPIBCVParams.m_strComboTypeID );
	pclIndSelParameter->SetPICvCBCtrlType( m_clIndSelPIBCVParams.m_eCvCtrlType );
	pclIndSelParameter->SetPICvFamilyID( m_clIndSelPIBCVParams.m_strComboFamilyID );
	pclIndSelParameter->SetPICvMaterialID( m_clIndSelPIBCVParams.m_strComboMaterialID );
	pclIndSelParameter->SetPICvConnectID( m_clIndSelPIBCVParams.m_strComboConnectID );
	pclIndSelParameter->SetPICvVersionID( m_clIndSelPIBCVParams.m_strComboVersionID );
	pclIndSelParameter->SetPICvPNID( m_clIndSelPIBCVParams.m_strComboPNID );
	pclIndSelParameter->SetPICvActPowerSupplyID( m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID );
	pclIndSelParameter->SetPICvActInputSignalID( m_clIndSelPIBCVParams.m_strActuatorInputSignalID );
	pclIndSelParameter->SetPICvFailSafeFct( m_clIndSelPIBCVParams.m_iActuatorFailSafeFunction );
	pclIndSelParameter->SetPICvDRPFct( m_clIndSelPIBCVParams.m_eActuatorDRPFunction );
	pclIndSelParameter->SetPICvPackageChecked( (int)m_clIndSelPIBCVParams.m_bOnlyForSet );
	pclIndSelParameter->SetPICvDpMaxCheckBox( (int)m_clIndSelPIBCVParams.m_bIsDpMaxChecked );

	if( NULL != pRViewSSelPICv )
	{
		pRViewSSelPICv->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelPICv::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelCtrlBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelPICv )
	{
		pRViewSSelPICv->Reset();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelPICv::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelCtrlBase::DoDataExchange( pDX );
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
	DDX_Control( pDX, IDC_CHECKPICVPACKAGE, m_CheckboxSet );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPVALVETYPE, m_GroupValveType );
	DDX_Control( pDX, IDC_GROUPACTUATOR, m_GroupActuator );
	DDX_Control( pDX, IDC_GROUPDPMAX, m_GroupDpMax );
	DDX_Control( pDX, IDC_CHECKFAILSAFE, m_CheckboxFailSafe );

	// 'CDlgIndSelBaseCtrl' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_EDITDPMAX, m_clExtEditDpMax );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelPICv::OnInitDialog()
{
	// Initialize the default dialog
	CDlgIndSelCtrlBase::OnInitDialog();

	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICCTRLTYPE );
	GetDlgItem( IDC_STATICCTRLTYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICMATERIAL );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICPOWERSUPPLY );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICINPUTSIGNAL );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_STATICFAILSAFE );
	GetDlgItem( IDC_CHECKFAILSAFE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
	GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_CHECKPACKAGE );
	GetDlgItem( IDC_CHECKPICVPACKAGE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );

	// Set proper style and add icons to groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupQ.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	m_clGroupQ.SetInOffice2007Mainframe( true );

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

	m_GroupDpMax.SetCheckBoxStyle( BS_AUTOCHECKBOX );

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

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

	m_CheckStatus = CTADatabase::FailSafeCheckStatus::eFirst;

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

BEGIN_MESSAGE_MAP( CDlgIndSelPICv, CDlgIndSelCtrlBase )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOCTRLTYPE, OnCbnSelChangeCtrlType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeBodyMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDPICV, OnModifySelectedPICV )
	ON_CBN_SELCHANGE( IDC_COMBOPOWERSUPPLY, OnCbnSelChangePowerSupply )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnCbnSelChangeInputSignal )
	ON_CBN_SELCHANGE( IDC_COMBODRP, OnCbnSelChangeDRPFct )
	ON_BN_CLICKED( IDC_CHECKPICVPACKAGE, OnBnClickedCheckboxSet )
	ON_EN_CHANGE( IDC_EDITDPMAX, OnEnChangeDpMax )
	ON_EN_KILLFOCUS( IDC_EDITDPMAX, OnEnKillFocusDpMax )
	ON_BN_CLICKED( IDC_GROUPDPMAX, OnBnClickedCheckGroupDpMax )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDPMAX, OnEditEnterChar )
	ON_BN_CLICKED(IDC_CHECKFAILSAFE, OnBnClickedCheckFailSafe )
END_MESSAGE_MAP()

void CDlgIndSelPICv::OnCbnSelChangeType()
{
	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboType );
	m_clIndSelPIBCVParams.m_strComboTypeID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboType );

	m_pclSelComboHelperPIBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeCtrlType();
}

void CDlgIndSelPICv::OnCbnSelChangeCtrlType()
{
	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboCtrlType );

	m_clIndSelPIBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_pclSelComboHelperPIBCV->GetIntFromCombo( &m_ComboCtrlType );

	if( iCur >= 0 )
	{
		m_clIndSelPIBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)iCur;
	}

	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamily, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeFamily();
}

void CDlgIndSelPICv::OnCbnSelChangeFamily()
{
	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboFamily );
	m_clIndSelPIBCVParams.m_strComboFamilyID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamily );

	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboMaterial, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeBodyMaterial();
}

void CDlgIndSelPICv::OnCbnSelChangeBodyMaterial()
{
	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboMaterial );
	m_clIndSelPIBCVParams.m_strComboMaterialID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboMaterial );

	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnect, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeConnect();
}

void CDlgIndSelPICv::OnCbnSelChangeConnect()
{
	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboConnect );
	m_clIndSelPIBCVParams.m_strComboConnectID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnect );

	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersion, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeVersion();
}

void CDlgIndSelPICv::OnCbnSelChangeVersion()
{
	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboVersion );
	m_clIndSelPIBCVParams.m_strComboVersionID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersion );

	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPN, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangePN();
}

void CDlgIndSelPICv::OnCbnSelChangePN()
{
	m_clIndSelPIBCVParams.m_strComboPNID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPN );

	_VerifyCheckboxSetStatus();
	_FillComboPowerSupply();
	OnCbnSelChangePowerSupply();
}

void CDlgIndSelPICv::OnCbnSelChangePowerSupply()
{
	m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
	_FillComboInputSignal();
	OnCbnSelChangeInputSignal();
}

void CDlgIndSelPICv::OnCbnSelChangeInputSignal()
{
	m_clIndSelPIBCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	_SetCheckFailSafeFct();
	OnBnClickedCheckFailSafe();
}

void CDlgIndSelPICv::OnCbnSelChangeDRPFct()
{
	m_clIndSelPIBCVParams.m_eActuatorDRPFunction = CDB_ControlValve::drpfUndefined;
	
	if( m_ComboDRPFct.GetCurSel() >= 0 )
	{
        m_clIndSelPIBCVParams.m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( m_ComboDRPFct.GetCurSel() );
	}

	ClearAll();
}

void CDlgIndSelPICv::OnBnClickedCheckboxSet()
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

	m_clIndSelPIBCVParams.m_bOnlyForSet = ( BST_CHECKED == m_CheckboxSet.GetCheck() ) ? true : false;

	if( true == fCanClear )
	{
		ClearAll();
	}
}

LRESULT CDlgIndSelPICv::OnModifySelectedPICV( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelPICv *pSelPICv = dynamic_cast<CDS_SSelPICv *>( ( CData * )lParam );
	ASSERT( NULL != pSelPICv );

	if( NULL == pSelPICv || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelPIBCVParams.m_SelIDPtr = pSelPICv->GetIDPtr();

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
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_PressureIndepCtrlValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelPIBCVParams.m_eApplicationType = pSelPICv->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelPIBCVParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelPIBCVParams.m_WC = *( pSelPICv->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelPICv->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	m_clIndSelPIBCVParams.m_bOnlyForSet = pSelPICv->IsSelectedAsAPackage();
	m_CheckboxSet.SetCheck( ( true == m_clIndSelPIBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );
	
	// By default we enable the 'Set' checkbox. The state of this one will be checked below by calling the '_VerifyCheckboxSetStatus'.
	m_clIndSelPIBCVParams.m_bCheckboxSetEnabled = true;
	m_CheckboxSet.EnableWindow( TRUE );

	m_pclSelComboHelperPIBCV->PrepareComboContents();
	
	// 'Type' combo.
	m_pclSelComboHelperPIBCV->FillComboType( &m_ComboType, CDlgSelComboHelperBase::CS_All, pSelPICv->GetTypeID() );
	m_clIndSelPIBCVParams.m_strComboTypeID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboType );

	// 'Control type' combo.
	m_pclSelComboHelperPIBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All, pSelPICv->GetCtrlType() );

	m_clIndSelPIBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_pclSelComboHelperPIBCV->GetIntFromCombo( &m_ComboCtrlType );

	if( iCur >= 0 )
	{
		m_clIndSelPIBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)iCur;
	}

	// 'Family' combo.
	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamily, CDlgSelComboHelperBase::CS_All, pSelPICv->GetFamilyID() );
	m_clIndSelPIBCVParams.m_strComboFamilyID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamily );

	// 'Material' combo.
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboMaterial, CDlgSelComboHelperBase::CS_All, pSelPICv->GetMaterialID() );
	m_clIndSelPIBCVParams.m_strComboMaterialID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboMaterial );

	// 'Connection' combo.
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnect, CDlgSelComboHelperBase::CS_All, pSelPICv->GetConnectID() );
	m_clIndSelPIBCVParams.m_strComboConnectID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnect );

	// 'Version' combo.
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersion, CDlgSelComboHelperBase::CS_All, pSelPICv->GetVersionID() );
	m_clIndSelPIBCVParams.m_strComboVersionID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersion );

	// 'PN' combo.
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPN, CDlgSelComboHelperBase::CS_All, pSelPICv->GetPNID() );
	m_clIndSelPIBCVParams.m_strComboPNID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPN );

	// We must now verify state of the checkbox before filling combo for actuator.
	_VerifyCheckboxSetStatus();

	// Remark: Corresponding combo variables in 'm_clIndSelPIBCVParams' are updated in each of this following methods.
	_FillComboPowerSupply( pSelPICv->GetActuatorPowerSupplyIDPtr() );
	_FillComboInputSignal( pSelPICv->GetActuatorInputSignalIDPtr() );

	int iCheck = -1;
	
	if( 1 == pSelPICv->GetFailSafeFunction() )
	{
		iCheck = BST_CHECKED;
	}
	else if( 0 == pSelPICv->GetFailSafeFunction() )
	{
		iCheck = BST_UNCHECKED;
	}
	
	ASSERT( iCheck != -1 );
	
	m_CheckboxFailSafe.SetCheck( iCheck );
	m_clIndSelPIBCVParams.m_iActuatorFailSafeFunction = pSelPICv->GetFailSafeFunction();
	_VerifyCheckboxFailsafeStatus();
	_FillComboDRPFct( pSelPICv->GetDRPFunction() );

	// Remark: The 'm_clIndSelPIBCVParams.m_strPipeSeriesID' and 'm_clIndSelPIBCVParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelPICv->GetPipeSeriesID(), pSelPICv->GetPipeID() );

	// Remark: 'm_clIndSelPIBCVParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_GroupDpMax.SetCheck( ( pSelPICv->GetDpMax() > 0.0 ) ? BST_CHECKED : BST_UNCHECKED );
	_UpdateDpMaxFieldState();

	// Remark: 'm_clIndSelPIBCVParams.m_dDpMax' is updated in the following method.
	SetDpMax( pSelPICv->GetDpMax() );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelPICv->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.	
	UpdateData( FALSE );

	// Remark: 'm_clIndSelPIBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelPIBCVParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelPIBCVParams.m_dFlow = pSelPICv->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelPIBCVParams.m_dPower = pSelPICv->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelPIBCVParams.m_dDT = pSelPICv->GetDT();
		UpdateDTFieldValue();
	}

	pSelPICv->ResetCVActrSetIDPtr();

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );

	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

void CDlgIndSelPICv::OnBnClickedCheckGroupDpMax()
{
	_UpdateDpMaxFieldState();
	ClearAll();
}

void CDlgIndSelPICv::OnEnChangeDpMax()
{
	if( GetFocus() == &m_clExtEditDpMax )
	{
		ClearAll();
	}
}

void CDlgIndSelPICv::OnEnKillFocusDpMax()
{
	m_clIndSelPIBCVParams.m_dDpMax = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpMax, &m_clIndSelPIBCVParams.m_dDpMax ) 
		|| m_clIndSelPIBCVParams.m_dDpMax < 0.0 )
	{
		m_clIndSelPIBCVParams.m_dDpMax = 0.0;
	}
}

void CDlgIndSelPICv::OnBnClickedCheckFailSafe()
{
	m_clIndSelPIBCVParams.m_iActuatorFailSafeFunction = ( BST_CHECKED == m_CheckboxFailSafe.GetCheck() ) ? 1 : 0;
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

void CDlgIndSelPICv::OnBnClickedSuggest()
{
	if( NULL == pRViewSSelPICv )
	{
		return;
	}

	if( m_clIndSelPIBCVParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetpEditFlow()->SetFocus();
		return;
	}

	if( m_clIndSelPIBCVParams.m_dDpMax <= 0.0 )
	{
		m_GroupDpMax.SetCheck( BST_UNCHECKED );
		_UpdateDpMaxFieldState();
	}

	// Clear the right view.
	ClearAll();

	// OnBnClickedButtonsuggest is called several time during initialisation;
	// return if not yet fully initialized
	if( false == m_bInitialised )
	{
		return;
	}

	// Create the Select PICVList to verify if at least one device exist.
	if( NULL != m_clIndSelPIBCVParams.m_pclSelectPIBCVList )
	{
		delete m_clIndSelPIBCVParams.m_pclSelectPIBCVList;
	}

	m_clIndSelPIBCVParams.m_pclSelectPIBCVList = new CSelectPICVList();

	if( NULL == m_clIndSelPIBCVParams.m_pclSelectPIBCVList || NULL == m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetSelectPipeList() )
	{
		ASSERT_RETURN;
	}

	m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetSelectPipeList()->SelectPipes( &m_clIndSelPIBCVParams, m_clIndSelPIBCVParams.m_dFlow );

	m_clIndSelPIBCVParams.m_CtrlList.PurgeAll();

	int iValveCount = m_clIndSelPIBCVParams.m_pTADB->GetTaCVList(
			&m_clIndSelPIBCVParams.m_CtrlList,							// List where to saved
			CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			m_clIndSelPIBCVParams.m_eCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboVersionID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboPNID,
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelPIBCVParams.m_eFilterSelection,
			0,															// DNMin
			INT_MAX,													// DNMax
			false,														// 'true' if it's for hub station.
			NULL,														// 'pProd'.
			m_clIndSelPIBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.
										 
	bool bSizeShiftProblem = false;
	bool bTryAlternative = false;

	if( 0 == iValveCount )
	{
		bTryAlternative = true;
	}
	else
	{
		int iDevFound = m_clIndSelPIBCVParams.m_pclSelectPIBCVList->SelectQ( &m_clIndSelPIBCVParams, &bSizeShiftProblem );

		if( iDevFound > 0 && false == bSizeShiftProblem )
		{
			VerifyModificationMode();
			pRViewSSelPICv->Suggest( &m_clIndSelPIBCVParams );
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

			if( pdlg->DoModal() != IDOK )
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

void CDlgIndSelPICv::OnBnClickedSelect()
{
	try
	{
		if( NULL == m_clIndSelPIBCVParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelPIBCVParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelPIBCVParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelPIBCVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelPICv )
		{
			HYSELECT_THROW( _T("Internal error: The pressure independent balancing & control valve right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The pressure independent balancing & control valve right view is not visible.") );
		}

		// Verify pointer validity; user can delete object after entering in Edition.
		if( _T('\0') != *m_clIndSelPIBCVParams.m_SelIDPtr.ID )
		{
			m_clIndSelPIBCVParams.m_SelIDPtr = m_clIndSelPIBCVParams.m_pTADS->Get( m_clIndSelPIBCVParams.m_SelIDPtr.ID );
		}

		if( m_clIndSelPIBCVParams.m_dFlow <= 0.0 )
		{
			HYSELECT_THROW( _T("Internal error: Flow '%f' can't be lower or equal to 0."), m_clIndSelPIBCVParams.m_dFlow );
		}

		// Create selected object and initialize it.
		CDS_SSelPICv *pSelPICv = dynamic_cast<CDS_SSelPICv *>( m_clIndSelPIBCVParams.m_SelIDPtr.MP );

		if( NULL == pSelPICv )
		{
			IDPTR IDPtr;
			m_clIndSelPIBCVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPICv ) );
			pSelPICv = ( CDS_SSelPICv * )IDPtr.MP;
		}

		pSelPICv->SetQ( m_clIndSelPIBCVParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelPIBCVParams.m_eFlowOrPowerDTMode )
		{
			pSelPICv->SetFlowDef( CDS_SelProd::efdFlow );
			pSelPICv->SetPower( 0.0 );
			pSelPICv->SetDT( 0.0 );
		}
		else
		{
			pSelPICv->SetFlowDef( CDS_SelProd::efdPower );
			pSelPICv->SetPower( m_clIndSelPIBCVParams.m_dPower );
			pSelPICv->SetDT( m_clIndSelPIBCVParams.m_dDT );
		}

		pSelPICv->SetDpMax( m_clIndSelPIBCVParams.m_dDpMax );

		pSelPICv->SetSelectedAsAPackage( m_clIndSelPIBCVParams.m_bOnlyForSet );
		pSelPICv->SetCheckPackageEnable( ( true == m_clIndSelPIBCVParams.m_bCheckboxSetEnabled ) ? TRUE : FALSE );

		pSelPICv->SetTypeID( m_clIndSelPIBCVParams.m_strComboTypeID );
		pSelPICv->SetCtrlType( m_clIndSelPIBCVParams.m_eCvCtrlType );
		pSelPICv->SetFamilyID( m_clIndSelPIBCVParams.m_strComboFamilyID );
		pSelPICv->SetMaterialID( m_clIndSelPIBCVParams.m_strComboMaterialID );
		pSelPICv->SetConnectID( m_clIndSelPIBCVParams.m_strComboConnectID );
		pSelPICv->SetVersionID( m_clIndSelPIBCVParams.m_strComboVersionID );
		pSelPICv->SetPNID( m_clIndSelPIBCVParams.m_strComboPNID );

		// Set the actuators info.
		pSelPICv->SetActuatorPowerSupplyID( m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID );
		pSelPICv->SetActuatorInputSignalID( m_clIndSelPIBCVParams.m_strActuatorInputSignalID );
		pSelPICv->SetFailSafeFunction( m_clIndSelPIBCVParams.m_iActuatorFailSafeFunction );
		pSelPICv->SetDRPFunction( m_clIndSelPIBCVParams.m_eActuatorDRPFunction );

		pSelPICv->SetPipeSeriesID( m_clIndSelPIBCVParams.m_strPipeSeriesID );
		pSelPICv->SetPipeID( m_clIndSelPIBCVParams.m_strPipeID );

		*pSelPICv->GetpSelectedInfos()->GetpWCData() = m_clIndSelPIBCVParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelPICv->GetpSelectedInfos()->SetDT( m_clIndSelPIBCVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelPICv->GetpSelectedInfos()->SetApplicationType( m_clIndSelPIBCVParams.m_eApplicationType );

		pRViewSSelPICv->FillInSelected( pSelPICv );

		// Show the confirm selection dialog box.
		CDlgConfSel dlg( &m_clIndSelPIBCVParams );
		IDPTR IDPtr = pSelPICv->GetIDPtr();

		dlg.Display( pSelPICv );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelPIBCVParams.m_pTADS->Get( _T("CTRLVALVE_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: 'CTRLVALVE_TAB' table from the datastruct.") );
			}

			if( _T('\0') == *m_clIndSelPIBCVParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelPIBCVParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelPICv ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelPIBCVParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelPIBCVParams.m_SelIDPtr.ID )
			{
				m_clIndSelPIBCVParams.m_pTADS->DeleteObject( IDPtr );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelPICv::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelPICv::OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult )
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
		CDlgIndSelCtrlBase::OnEditEnterChar( pNMHDR, pResult );
	}
}

LRESULT CDlgIndSelPICv::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnNewDocument( wParam, lParam );

	// Reinit parameters.
	ClearCommonInputEditors();

	// Get last selected parameters.
	// Remark: if user clicks create a new document, the 'SaveSelectionParameters' is automatically called before 'OnNewDocument'.
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelPIBCVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_clIndSelPIBCVParams.m_bOnlyForSet = ( 0 == pclIndSelParameter->GetPICvPackageChecked() ) ? false : true;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelPIBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	// By default we enable the 'Set' checkbox. The state of this one will be checked below by calling the '_VerifyCheckboxSetStatus'.
	m_clIndSelPIBCVParams.m_bCheckboxSetEnabled = true;
	m_CheckboxSet.EnableWindow( TRUE );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetPICvRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );
	
	// Remark: 'm_clIndSelPIBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	m_pclSelComboHelperPIBCV->PrepareComboContents();

	// 'Type' combo.
	m_pclSelComboHelperPIBCV->FillComboType( &m_ComboType, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetPICvTypeID() );
	m_clIndSelPIBCVParams.m_strComboTypeID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboType );

	// 'Control type' combo.
	m_pclSelComboHelperPIBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetPICvCBCtrlType() );

	m_clIndSelPIBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_pclSelComboHelperPIBCV->GetIntFromCombo( &m_ComboCtrlType );

	if( iCur >= 0 )
	{
		m_clIndSelPIBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)iCur;
	}

	// 'Family' combo.
	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamily, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetPICvFamilyID() );
	m_clIndSelPIBCVParams.m_strComboFamilyID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamily );

	// 'Material' combo.
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboMaterial, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetPICvMaterialID() );
	m_clIndSelPIBCVParams.m_strComboMaterialID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboMaterial );

	// 'Connection' combo.
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnect, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetPICvConnectID() );
	m_clIndSelPIBCVParams.m_strComboConnectID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnect );

	// 'Version' combo.
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersion, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetPICvVersionID() );
	m_clIndSelPIBCVParams.m_strComboVersionID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersion );

	// 'PN' combo.
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPN, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetPICvPNID() );
	m_clIndSelPIBCVParams.m_strComboPNID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPN );

	// We must now verify state of the checkbox before filling combo for actuator.
	_VerifyCheckboxSetStatus();

	// Remark: Corresponding combo variables in 'm_clIndSelPIBCVParams' are updated in each of this following methods.
	_FillComboPowerSupply( pclIndSelParameter->GetPICvActPowerSupplyID() );
	_FillComboInputSignal( pclIndSelParameter->GetPICvActInputSignalID() );
	
	int iCheck = -1;
	
	// HYS-1313: In old version, fail safe function was a combo with 4 values (fsfNone, fsfClosing, fsfOpening and fsfAll).
	// It can happen that we read here value of 2 or 3!
	if( 0 == pclIndSelParameter->GetPICvFailSafeFct() )
	{
		iCheck = BST_UNCHECKED;
	}
	else 
	{
		iCheck = BST_CHECKED;
	}
	
	m_CheckboxFailSafe.SetCheck( iCheck );
	m_clIndSelPIBCVParams.m_iActuatorFailSafeFunction = iCheck;
	_VerifyCheckboxFailsafeStatus();
	_FillComboDRPFct( pclIndSelParameter->GetPICvDRPFct() );

	OnCbnSelChangeDRPFct();

	// Remark: 'm_clIndSelPIBCVParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_GroupDpMax.SetCheck( ( 0 == pclIndSelParameter->GetPICvDpMaxCheckBox() ) ? BST_UNCHECKED : BST_CHECKED );
	_UpdateDpMaxFieldState();

	// Fill the flow and update water TCHAR. strings.
	OnUnitChange();

	m_bInitialised = true;

	if( NULL != pRViewSSelPICv )
	{
		pRViewSSelPICv->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}

LRESULT CDlgIndSelPICv::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != ( WMUserPipeChange )wParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnPipeChange( wParam, lParam );

	// Clear the right view.
	if( NULL != pRViewSSelPICv )
	{
		pRViewSSelPICv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelPICv::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnTechParChange( wParam, lParam );

	// Clear the right view.
	if( NULL != pRViewSSelPICv )
	{
		pRViewSSelPICv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelPICv::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnUnitChange( wParam, lParam );

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATICDPMAXUNIT, name );

	if( m_clIndSelPIBCVParams.m_dDpMax > 0.0 )
	{
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_clIndSelPIBCVParams.m_dDpMax ) );
	}

	// Clear the right view.
	if( NULL != pRViewSSelPICv )
	{
		pRViewSSelPICv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelPICv::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( ( WMUserWaterCharWParam )wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel )
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnWaterChange( wParam, lParam );

	// Clear the right view.
	if( NULL != pRViewSSelPICv )
	{
		pRViewSSelPICv->Reset();
	}

	return 0;
}

CRViewSSelSS *CDlgIndSelPICv::GetLinkedRightViewSSel( void )
{
	return pRViewSSelPICv;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelPICv::_FillComboPowerSupply( CString strPowerSupplyID )
{
	CRankEx PICVList;
	// HYS-1448 : PIBCV type have to be considered
	m_clIndSelPIBCVParams.m_pTADB->GetTaCVList(	
			&PICVList,													// List where to saved
			CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			CDB_ControlProperties::LastCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboTypeID,			// Type ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboFamilyID,			// Family ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboMaterialID,		// Body material ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboConnectID,			// Connection ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboVersionID,			// Version ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboPNID,				// PN ID
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelPIBCVParams.m_eFilterSelection,
			0,															// DNMin
			INT_MAX,													// DNMax
			false,														// 'true' if it's for hub station.
			NULL,														// 'pProd'.
			m_clIndSelPIBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.
						
	CRankEx PowerSupplyList;
	m_clIndSelPIBCVParams.m_pTADB->GetTaCVActuatorPowerSupplyList( &PowerSupplyList, &PICVList, m_clIndSelPIBCVParams.m_eCvCtrlType, m_clIndSelPIBCVParams.m_bOnlyForSet, m_clIndSelPIBCVParams.m_eFilterSelection );
	m_ComboPowerSupply.FillInCombo( &PowerSupplyList, strPowerSupplyID, m_clIndSelPIBCVParams.GetComboActuatorPowerSupplyAllID() );
	m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelPICv::_FillComboInputSignal( CString strInputSignalID )
{
	if( 0 == m_ComboPowerSupply.GetCount() )
	{
		m_ComboInputSignal.ResetContent();
		m_ComboInputSignal.EnableWindow( FALSE );
		m_clIndSelPIBCVParams.m_strActuatorInputSignalID = _T("");
	}
	else
	{
		CRankEx PICVList;
		// HYS-1448 : PIBCV type have to be considered
		m_clIndSelPIBCVParams.m_pTADB->GetTaCVList(	
				&PICVList,													// List where to saved
				CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
				false, 														// 'true' returns as soon a result is found
				CDB_ControlProperties::LastCV2W3W, 							// Set way number of valve
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboTypeID,			// Type ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboFamilyID,			// Family ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboMaterialID,		// Body material ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboConnectID,			// Connection ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboVersionID,			// Version ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboPNID,				// PN ID
				CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
				m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSelPIBCVParams.m_eFilterSelection,
				0,															// DNMin
				INT_MAX,													// DNMax
				false,														// 'true' if it's for hub station.
				NULL,														// 'pProd'.
				m_clIndSelPIBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.

		CRankEx InputSignalList;

		m_clIndSelPIBCVParams.m_pTADB->GetTaCVActuatorInputSignalList( &InputSignalList, &PICVList, m_clIndSelPIBCVParams.m_eCvCtrlType, 
				m_clIndSelPIBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID, m_clIndSelPIBCVParams.m_eFilterSelection );
		m_ComboInputSignal.FillInCombo( &InputSignalList, strInputSignalID, m_clIndSelPIBCVParams.GetComboActuatorInputSignalAllID() );
		m_clIndSelPIBCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSelPICv::_SetCheckFailSafeFct()
{
	if( 0 == m_ComboInputSignal.GetCount() )
	{
		// Checkbox unchecked and disabled
		m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
		m_CheckboxFailSafe.EnableWindow( false );
	}
	else
	{
		CRankEx PICVList;
		// HYS-1448 : PIBCV type have to be considered
		m_clIndSelPIBCVParams.m_pTADB->GetTaCVList(
			&PICVList,													// List where to saved
			CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			CDB_ControlProperties::LastCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboTypeID,			// Type ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboFamilyID,			// Family ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboMaterialID,		// Body material ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboConnectID,			// Connection ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboVersionID,			// Version ID
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboPNID,				// PN ID
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelPIBCVParams.m_eFilterSelection,
			0,															// DNMin
			INT_MAX,													// DNMax
			false,														// 'true' if it's for hub station.
			NULL,														// 'pProd'.
			m_clIndSelPIBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.

		m_CheckStatus = m_clIndSelPIBCVParams.m_pTADB->GetTaCVActuatorFailSafeValues( &PICVList, m_clIndSelPIBCVParams.m_eCvCtrlType, m_clIndSelPIBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strActuatorInputSignalID, m_clIndSelPIBCVParams.m_eFilterSelection );
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

void CDlgIndSelPICv::_FillComboDRPFct( CDB_ControlValve::DRPFunction eDRPFct )
{
	if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus && false == m_CheckboxFailSafe.IsWindowEnabled() )
	{
		m_ComboDRPFct.ResetContent();
		m_ComboDRPFct.EnableWindow( FALSE );
		m_clIndSelPIBCVParams.m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfUndefined;
	}
	else
	{
		CRankEx PICVList;
		// HYS-1448 : PIBCV type have to be considered
		m_clIndSelPIBCVParams.m_pTADB->GetTaCVList(
				&PICVList,													// List where to saved
				CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
				false, 														// 'true' returns as soon a result is found
				CDB_ControlProperties::LastCV2W3W, 							// Set way number of valve
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboTypeID,			// Type ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboFamilyID,			// Family ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboMaterialID,		// Body material ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboConnectID,			// Connection ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboVersionID,			// Version ID
				(LPCTSTR)m_clIndSelPIBCVParams.m_strComboPNID,				// PN ID
				CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
				m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSelPIBCVParams.m_eFilterSelection,
				0,															// DNMin
				INT_MAX,													// DNMax
				false,														// 'true' if it's for hub station.
				NULL,														// 'pProd'.
				m_clIndSelPIBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.
							
		CRankEx DRPFctList;

		m_clIndSelPIBCVParams.m_pTADB->GetTaCVDRPFunctionList( &DRPFctList, m_clIndSelPIBCVParams.m_eCvCtrlType, &PICVList, m_clIndSelPIBCVParams.m_bOnlyForSet,
				(LPCTSTR)m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID, (LPCTSTR)m_clIndSelPIBCVParams.m_strActuatorInputSignalID, 
			m_clIndSelPIBCVParams.m_iActuatorFailSafeFunction, m_clIndSelPIBCVParams.m_eFilterSelection );

		m_ComboDRPFct.ResetContent();
		int iSelPos = 0;

		if( DRPFctList.GetCount() > 0 )
		{
			DRPFctList.Transfer( &m_ComboDRPFct );

			// If we have more than one item in the combo, we insert "** All fail safe **" string.
			if( m_ComboDRPFct.GetCount() > 1 )
			{
				CString str = TASApp.LoadLocalizedString( m_clIndSelPIBCVParams.GetComboActuatorFailSafeAllID() );
				m_ComboDRPFct.InsertString( 0, str );
                m_ComboDRPFct.SetItemData( 0, CDB_ControlValve::DRPFunction::drpfAll );
			}

			if( m_ComboDRPFct.GetCount() > 1 && eDRPFct >= CDB_ControlValve::DRPFunction::drpfNone
					&& eDRPFct < CDB_ControlValve::DRPFunction::drpfLast )
			{
				// If we have only one 1 item, this one is set at the position 0.
				// If we have more than one 1 item, "** All fail safe **" has been inserted at the position 0. Thus first valid item begins at 1.
				int i = ( 1 == m_ComboDRPFct.GetCount() ) ? 0 : 1;

				for( ; i < m_ComboDRPFct.GetCount(); i++ )
				{
                    if( eDRPFct == ( CDB_ControlValve::DRPFunction )m_ComboDRPFct.GetItemData( i ) )
					{
						iSelPos = i;
						break;
					}
				}
			}
		}

		m_ComboDRPFct.SetCurSel( iSelPos );
		m_clIndSelPIBCVParams.m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( iSelPos );

		m_ComboDRPFct.EnableWindow( ( m_ComboDRPFct.GetCount() <= 1 ) ? FALSE : TRUE );
	}
}

void CDlgIndSelPICv::_LaunchSuggestion( CString strTypeID, CDB_ControlProperties::CvCtrlType eCvCtrlType, CString strFamilyID, CString strMaterialID, 
		CString strConnectID, CString strVersionID, CString strPNID, bool bSelectionBySet )
{
	// 'Type' combo.
	m_pclSelComboHelperPIBCV->FillComboType( &m_ComboType, CDlgSelComboHelperBase::CS_All, strTypeID );
	m_clIndSelPIBCVParams.m_strComboTypeID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboType );

	// 'Control type' combo.
	m_pclSelComboHelperPIBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All, eCvCtrlType );
	
	m_clIndSelPIBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_pclSelComboHelperPIBCV->GetIntFromCombo( &m_ComboCtrlType );

	if( iCur >= 0 )
	{
		m_clIndSelPIBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)iCur;
	}

	// 'Family' combo.
	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamily, CDlgSelComboHelperBase::CS_All, strFamilyID );
	m_clIndSelPIBCVParams.m_strComboFamilyID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamily );

	// 'Material' combo.	
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboMaterial, CDlgSelComboHelperBase::CS_All, strMaterialID );
	m_clIndSelPIBCVParams.m_strComboMaterialID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboMaterial );

	// 'Connection' combo.	
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnect, CDlgSelComboHelperBase::CS_All, strConnectID );
	m_clIndSelPIBCVParams.m_strComboConnectID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnect );

	// 'Version' combo.	
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersion, CDlgSelComboHelperBase::CS_All, strVersionID );
	m_clIndSelPIBCVParams.m_strComboVersionID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersion );

	// 'PN' combo.	
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPN, CDlgSelComboHelperBase::CS_All, strPNID );
	m_clIndSelPIBCVParams.m_strComboPNID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPN );

	m_clIndSelPIBCVParams.m_bOnlyForSet = bSelectionBySet;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelPIBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	OnCbnSelChangePN();

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelPICv::_TryAlternativeSuggestion( bool bSizeShiftProblem )
{
	// Variables.
	int iDevFound = 0;
	bool bReturn = false;

	// If no device found try to change the combo to get at least one valid product.
	// If it's the case, show the 'No device found' dialog. Otherwise, show AFXMSG_...
	if( false == bSizeShiftProblem )
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
			m_clIndSelPIBCVParams.m_CtrlList.PurgeAll();
			int iValveCount = 0;
			bool bSetFlag = ( bitSet == ( iLoopAlternative & bitSet ) ) ? true : false;
			bool bRelaxCombo = ( bitCombos == ( iLoopAlternative & bitCombos ) ) ? true : false;

			// We do these cases only if user has selected set.
			if( true == bSetFlag && false == m_clIndSelPIBCVParams.m_bOnlyForSet )
			{
				continue;
			}

			bool bOnlySet = ( false == bSetFlag ) ? m_clIndSelPIBCVParams.m_bOnlyForSet : false;

			CString strTypeID( _T( "" ) );
			CString strFamilyID( _T( "" ) );
			CString strBodyMaterialID( _T( "" ) );
			CString strConnectID( _T( "" ) );
			CString strVersionID( _T( "" ) );
			CString strPNID( _T( "" ) );

			if( false == bRelaxCombo )
			{
				strTypeID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboType );
				strFamilyID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamily );
				strBodyMaterialID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboMaterial );
				strConnectID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnect );
				strVersionID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersion );
				strPNID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPN );
			}

			iValveCount = m_clIndSelPIBCVParams.m_pTADB->GetTaCVList( 
					&m_clIndSelPIBCVParams.m_CtrlList,			// List where to saved
					CTADatabase::eForPiCv,						// Control valve target (cv, hmcv, picv or bcv)
					false, 										// 'true' returns as soon a result is found
					m_clIndSelPIBCVParams.m_eCV2W3W, 			// Set way number of valve
					(LPCTSTR)strTypeID,							// Type ID
					(LPCTSTR)strFamilyID,						// Family ID
					(LPCTSTR)strBodyMaterialID,					// Body material ID
					(LPCTSTR)strConnectID,						// Connection ID
					(LPCTSTR)strVersionID, 						// Version ID
					(LPCTSTR)strPNID,							// PN ID
					CDB_ControlProperties::LastCVFUNC, 			// Set the control function (control only, presettable, ...)
					m_clIndSelPIBCVParams.m_eCvCtrlType,		// Set the control type (on/off, proportional, ...)
					CTADatabase::ForIndAndBatchSel,
					0,											// DNMin
					INT_MAX,									// DNMax
					false,										// 'true' if it's for hub station.
					NULL,										// 'pProd'.
					bOnlySet );									// 'true' if it's only for a set.

			if( 0 == iValveCount )
			{
				continue;
			}

			iDevFound = m_clIndSelPIBCVParams.m_pclSelectPIBCVList->SelectQ( &m_clIndSelPIBCVParams, &bSizeShiftProblem );

			if( iDevFound > 0 && false == bSizeShiftProblem )
			{
				if( true == bRelaxCombo )
				{
					// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevCtrlFound' dialog.
					m_clIndSelPIBCVParams.m_CtrlList.PurgeAll();
					double dKey = 0.0;

					for( CSelectedValve *pclSelectedValve = m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = m_clIndSelPIBCVParams.m_pclSelectPIBCVList->GetNext<CSelectedValve>() )
					{
						CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

						if( NULL == pTAP )
						{
							continue;
						}

						m_clIndSelPIBCVParams.m_CtrlList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
					}

					// Launch the 'No device found' dialog box.
					// Remark: Combos in this dialog will be filled thanks to the new alternative pressure independent balancing and control valves found and 
					//         saved in the 'PICVList' variable.
					CDlgNoDevCtrlFoundParams clNoDevCtrlParams = CDlgNoDevCtrlFoundParams( &m_clIndSelPIBCVParams.m_CtrlList, m_clIndSelPIBCVParams.m_strComboTypeID, 
							m_clIndSelPIBCVParams.m_eCvCtrlType, m_clIndSelPIBCVParams.m_strComboFamilyID, m_clIndSelPIBCVParams.m_strComboMaterialID, 
							m_clIndSelPIBCVParams.m_strComboConnectID, m_clIndSelPIBCVParams.m_strComboVersionID, m_clIndSelPIBCVParams.m_strComboPNID );

					CDlgNoDevCtrlFound dlg( &m_clIndSelPIBCVParams, &clNoDevCtrlParams );

					if( IDOK == dlg.DoModal() )
					{
						clNoDevCtrlParams = *dlg.GetNoDevCtrlFoundParams();

						_LaunchSuggestion( clNoDevCtrlParams.m_strTypeID, clNoDevCtrlParams.m_eCvCtrlType, clNoDevCtrlParams.m_strFamilyID, clNoDevCtrlParams.m_strMaterialID, 
								clNoDevCtrlParams.m_strConnectID, clNoDevCtrlParams.m_strVersionID, clNoDevCtrlParams.m_strPNID, bOnlySet );
					}

					bStop = true;
				}
				else
				{
					if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_INDSEL_CTRLVALVENOTFOUND_NOSET, MB_YESNO | MB_ICONQUESTION, 0 ) )
					{
						_LaunchSuggestion( m_clIndSelPIBCVParams.m_strComboTypeID, m_clIndSelPIBCVParams.m_eCvCtrlType, m_clIndSelPIBCVParams.m_strComboFamilyID, 
								m_clIndSelPIBCVParams.m_strComboMaterialID, m_clIndSelPIBCVParams.m_strComboConnectID, m_clIndSelPIBCVParams.m_strComboVersionID, 
								m_clIndSelPIBCVParams.m_strComboPNID, bOnlySet );
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

void CDlgIndSelPICv::_VerifyCheckboxSetStatus()
{
	CRankEx PICVList;

	m_clIndSelPIBCVParams.m_pTADB->GetTaCVList(
			&PICVList,													// List where to saved
			CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			m_clIndSelPIBCVParams.m_eCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboFamilyID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboVersionID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strComboPNID,
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelPIBCVParams.m_eFilterSelection );

	// HYS-1877: We will not consider PIBCV that belong to a 6-way valve set
	_FilterPIBCVSetFor6WayControlValve( &PICVList );

	// Remark: the 'm_clIndSelPIBCVParams.m_bOnlyForSet' variable is updated in this method.
	VerifyCheckboxSetStatus( &PICVList );

	m_CheckboxSet.SetCheck( ( true == m_clIndSelPIBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );
	m_CheckboxSet.EnableWindow( ( true == m_clIndSelPIBCVParams.m_bCheckboxSetEnabled ) ? BST_CHECKED : BST_UNCHECKED );
}

void CDlgIndSelPICv::_UpdateDpMaxFieldState()
{
	m_clIndSelPIBCVParams.m_bIsDpMaxChecked = ( BST_CHECKED == m_GroupDpMax.GetCheck() ) ? true : false;

	if( true == m_clIndSelPIBCVParams.m_bIsDpMaxChecked )
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
		m_clIndSelPIBCVParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( FALSE );
		m_clExtEditDpMax.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( FALSE );
		m_GroupDpMax.SetForceShowDisable( true );
	}
}

void CDlgIndSelPICv::_FilterPIBCVSetFor6WayControlValve( CRankEx* pclRankList )
{
	wstring str;
	LPARAM lParam;
	double dKey;

	for( BOOL bContinue = pclRankList->GetFirst( str, lParam, &dKey ); TRUE == bContinue; bContinue = pclRankList->GetNext( str, lParam, &dKey ) )
	{
		CDB_TAProduct* pTAProduct = dynamic_cast<CDB_TAProduct*>((CData*)lParam);

		if( NULL == pTAProduct )
		{
			continue;
		}

		if( NULL != pTAProduct->GetTableSet() && 0 == IDcmp( pTAProduct->GetTableSet()->GetIDPtr().ID, _T( "6WAYCTRLVALVSET_TAB" ) ) )
		{
			pclRankList->Delete( dKey );
			continue;
		}

	}
}

void CDlgIndSelPICv::_VerifyCheckboxFailsafeStatus()
{
	CRankEx PICVList;

	m_clIndSelPIBCVParams.m_pTADB->GetTaCVList(
		&PICVList,													// List where to saved
		CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
		false, 														// 'true' returns as soon a result is found
		m_clIndSelPIBCVParams.m_eCV2W3W, 							// Set way number of valve
		(LPCTSTR)m_clIndSelPIBCVParams.m_strComboTypeID,
		(LPCTSTR)m_clIndSelPIBCVParams.m_strComboFamilyID,
		(LPCTSTR)m_clIndSelPIBCVParams.m_strComboMaterialID,
		(LPCTSTR)m_clIndSelPIBCVParams.m_strComboConnectID,
		(LPCTSTR)m_clIndSelPIBCVParams.m_strComboVersionID,
		(LPCTSTR)m_clIndSelPIBCVParams.m_strComboPNID,
		CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
		m_clIndSelPIBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
		m_clIndSelPIBCVParams.m_eFilterSelection );

	m_CheckStatus = m_clIndSelPIBCVParams.m_pTADB->GetTaCVActuatorFailSafeValues( &PICVList, m_clIndSelPIBCVParams.m_eCvCtrlType, 
			m_clIndSelPIBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID,
			(LPCTSTR)m_clIndSelPIBCVParams.m_strActuatorInputSignalID, m_clIndSelPIBCVParams.m_eFilterSelection );
	
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