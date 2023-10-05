#include "stdafx.h"

#include "TASelect.h"
#include "DlgSelectionComboHelper.h"

#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "Select.h"

#include "DlgWaterChar.h"
#include "DlgNoDevCtrlFound.h"
#include "DlgConfSel.h"
#include "DlgTechParam.h"

#include "DlgLTtabctrl.h"
#include "DlgLeftTabSelManager.h"

#include "DlgInfoSSelDpCBCV.h"

#include "RViewSSelSS.h"
#include "RViewSSelCtrl.h"
#include "RViewSSelDpCBCV.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelCtrlBase.h"
#include "DlgIndSelDpCBCV.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelDpCBCV::CDlgIndSelDpCBCV( CWnd *pParent )
	: CDlgIndSelCtrlBase( m_clIndSelDpCBCVParams, CDlgIndSelDpCBCV::IDD, pParent )
{
	m_clIndSelDpCBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve;
	m_clIndSelDpCBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_pclSelComboHelperDpCBCV = new CDlgSelComboHelperDpCBCV( &m_clIndSelDpCBCVParams, CDlgSelComboHelperBase::SelType_Individual );
	
	if( NULL == m_pclSelComboHelperDpCBCV )
	{
		ASSERT( 0 );
	}

	m_pInfoDialogDpCBCV = NULL;
}

CDlgIndSelDpCBCV::~CDlgIndSelDpCBCV()
{
	if( NULL != m_pclSelComboHelperDpCBCV )
	{
		delete m_pclSelComboHelperDpCBCV;
	}

	if( NULL != m_pInfoDialogDpCBCV )
	{
		delete m_pInfoDialogDpCBCV;
	}
}

void CDlgIndSelDpCBCV::SetDpToStabilize( double dDpToStabilize )
{
	if( dDpToStabilize > 0.0 )
	{
		m_clIndSelDpCBCVParams.m_dDpToStabilize = dDpToStabilize;
		m_clExtEditDpToStabilize.SetWindowText( WriteCUDouble( _U_DIFFPRESS, dDpToStabilize ) );
	}
	else
	{
		m_clIndSelDpCBCVParams.m_dDpToStabilize = 0.0;
		m_clExtEditDpToStabilize.SetWindowText( _T( "" ) );
	}
}

void CDlgIndSelDpCBCV::SaveSelectionParameters()
{
	CDlgIndSelCtrlBase::SaveSelectionParameters();

	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelDpCBCVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetDpCBCVRadioFlowPowerDT( (int)m_clIndSelDpCBCVParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetDpCBCVTypeID( m_clIndSelDpCBCVParams.m_strComboTypeID );
	pclIndSelParameter->SetDpCBCVCBCtrlType( m_clIndSelDpCBCVParams.m_eCvCtrlType );
	pclIndSelParameter->SetDpCBCVFamilyID( m_clIndSelDpCBCVParams.m_strComboFamilyID );
	pclIndSelParameter->SetDpCBCVMaterialID( m_clIndSelDpCBCVParams.m_strComboMaterialID );
	pclIndSelParameter->SetDpCBCVConnectID( m_clIndSelDpCBCVParams.m_strComboConnectID );
	pclIndSelParameter->SetDpCBCVVersionID( m_clIndSelDpCBCVParams.m_strComboVersionID );
	pclIndSelParameter->SetDpCBCVPNID( m_clIndSelDpCBCVParams.m_strComboPNID );
	pclIndSelParameter->SetDpCBCVActPowerSupplyID( m_clIndSelDpCBCVParams.m_strActuatorPowerSupplyID );
	pclIndSelParameter->SetDpCBCVActInputSignalID( m_clIndSelDpCBCVParams.m_strActuatorInputSignalID );
	pclIndSelParameter->SetDpCBCVFailSafeFct( m_clIndSelDpCBCVParams.m_iActuatorFailSafeFunction );
	pclIndSelParameter->SetDpCBCVDRPFct( m_clIndSelDpCBCVParams.m_eActuatorDRPFunction );
	pclIndSelParameter->SetDpCBCVPackageChecked( (int)m_clIndSelDpCBCVParams.m_bOnlyForSet );
	pclIndSelParameter->SetDpCBCVDpToStabilizeCheckBox( (int)m_clIndSelDpCBCVParams.m_bIsGroupDpToStabilizeChecked );
	pclIndSelParameter->SetDpCBCVWithSTSCheckBox( (int)m_clIndSelDpCBCVParams.m_bIsWithSTSChecked );
	pclIndSelParameter->SetDpCBCVInfoWndChecked( (int)m_clIndSelDpCBCVParams.m_bIsDpCInfoWndChecked );

	if( NULL != pRViewSSelDpCBCV )
	{
		pRViewSSelDpCBCV->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelDpCBCV::LeaveLeftTabDialog()
{
	if( NULL != m_pInfoDialogDpCBCV->GetSafeHwnd() )
	{
		m_pInfoDialogDpCBCV->DestroyWindow();
	}

	CDlgIndSelCtrlBase::LeaveLeftTabDialog();
}

void CDlgIndSelDpCBCV::ActivateLeftTabDialog()
{
	OnBnClickedCheckShowInfoWnd();
	CDlgIndSelCtrlBase::ActivateLeftTabDialog();
}

void CDlgIndSelDpCBCV::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelCtrlBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelDpCBCV )
	{
		pRViewSSelDpCBCV->Reset();
	}
}

void CDlgIndSelDpCBCV::ClearAll( void )
{
	CDlgIndSelCtrlBase::ClearAll();

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelDpCBCV, CDlgIndSelCtrlBase )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOCTRLTYPE, OnCbnSelChangeCtrlType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeBodyMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_CBN_SELCHANGE( IDC_COMBOPOWERSUPPLY, OnCbnSelChangePowerSupply )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnCbnSelChangeInputSignal )
	ON_CBN_SELCHANGE( IDC_COMBODRP, OnCbnSelChangeDRPFct )

	ON_BN_CLICKED( IDC_CHECKSTS, OnBnClickedCheckboxWithSTS )
	ON_BN_CLICKED( IDC_CHECKSHOWINFOWND, OnBnClickedCheckShowInfoWnd )
	ON_BN_CLICKED( IDC_CHECKDPCBCVPACKAGE, OnBnClickedCheckboxSet )
	ON_BN_CLICKED( IDC_CHECKFAILSAFE, OnBnClickedCheckFailSafe )

	ON_EN_CHANGE( IDC_EDITDPL, OnEnChangeDpToStabilize )
	ON_EN_KILLFOCUS( IDC_EDITDPL, OnEnKillFocusDpToStabilize )

	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDPL, OnEditEnterChar )

	ON_MESSAGE( WM_USER_MODIFYSELECTEDDPCBCV, OnModifySelectedDpCBCV )
	ON_MESSAGE( WM_USER_DESTROYDIALOGINFOSSELDPCBCV, OnDestroyInfoDialogDpCBCV )

END_MESSAGE_MAP()
void CDlgIndSelDpCBCV::DoDataExchange( CDataExchange *pDX )
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
	DDX_Control( pDX, IDC_CHECKSTS, m_CheckboxWithSTS );
	DDX_Control( pDX, IDC_CHECKSHOWINFOWND, m_CheckInfoWnd );
	DDX_Control( pDX, IDC_CHECKDPCBCVPACKAGE, m_CheckboxSet );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPVALVETYPE, m_GroupValveType );
	DDX_Control( pDX, IDC_GROUPACTUATOR, m_GroupActuator );
	DDX_Control( pDX, IDC_GROUPDPL, m_GroupDpToStabilize );
	DDX_Control( pDX, IDC_CHECKFAILSAFE, m_CheckboxFailSafe );

	// 'CDlgIndSelBaseCtrl' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_EDITDPL, m_clExtEditDpToStabilize );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelDpCBCV::OnInitDialog()
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
	GetDlgItem( IDC_CHECKDPCBCVPACKAGE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPCBCV_CHECKWITHSTS );
	GetDlgItem( IDC_CHECKSTS )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELPICV_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPCBCV_CHECKINFOWND );
	GetDlgItem( IDC_CHECKSHOWINFOWND )->SetWindowText( str );

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

	if( NULL != pclImgListGroupBox )
	{
		m_GroupDpToStabilize.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pen );
	}

	str = TASApp.LoadLocalizedString( IDS_DLGINDSELDPCBCV_DPL );
	m_GroupDpToStabilize.SetWindowText( str );
	m_GroupDpToStabilize.SetInOffice2007Mainframe( true );

	if( NULL == m_clIndSelDpCBCVParams.m_pTADS || NULL == m_clIndSelDpCBCVParams.m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( FALSE );
	}

	m_clIndSelDpCBCVParams.m_dDpToStabilize = m_clIndSelDpCBCVParams.m_pTADS->GetpTechParams()->GetDpCBCVDplmin();
	m_clExtEditDpToStabilize.SetPhysicalType( _U_DIFFPRESS );
	m_clExtEditDpToStabilize.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_clExtEditDpToStabilize.SetMinDblValue( m_clIndSelDpCBCVParams.m_dDpToStabilize );
	m_clExtEditDpToStabilize.SetCurrentValSI( m_clIndSelDpCBCVParams.m_dDpToStabilize );
	m_clExtEditDpToStabilize.Update();
	
	m_CheckStatus = CTADatabase::FailSafeCheckStatus::eFirst;

	// Create Flying DpCBCV info.
	if( NULL == m_pInfoDialogDpCBCV )
	{
		m_pInfoDialogDpCBCV = new CDlgInfoSSelDpCBCV( &m_clIndSelDpCBCVParams, this );

		if( NULL == m_pInfoDialogDpCBCV )
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CDlgIndSelDpCBCV::OnCbnSelChangeType()
{
	m_pclSelComboHelperDpCBCV->OnCbnSelChange( &m_ComboType );
	m_clIndSelDpCBCVParams.m_strComboTypeID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboType );

	m_pclSelComboHelperDpCBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeCtrlType();
}

void CDlgIndSelDpCBCV::OnCbnSelChangeCtrlType()
{
	m_pclSelComboHelperDpCBCV->OnCbnSelChange( &m_ComboCtrlType );

	m_clIndSelDpCBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_pclSelComboHelperDpCBCV->GetIntFromCombo( &m_ComboCtrlType );

	if( iCur >= 0 )
	{
		m_clIndSelDpCBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)iCur;
	}

	m_pclSelComboHelperDpCBCV->FillComboFamily( &m_ComboFamily, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeFamily();
}

void CDlgIndSelDpCBCV::OnCbnSelChangeFamily()
{
	m_pclSelComboHelperDpCBCV->OnCbnSelChange( &m_ComboFamily );
	m_clIndSelDpCBCVParams.m_strComboFamilyID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboFamily );

	m_pclSelComboHelperDpCBCV->FillComboBodyMaterial( &m_ComboMaterial, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeBodyMaterial();
}

void CDlgIndSelDpCBCV::OnCbnSelChangeBodyMaterial()
{
	m_pclSelComboHelperDpCBCV->OnCbnSelChange( &m_ComboMaterial );
	m_clIndSelDpCBCVParams.m_strComboMaterialID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboMaterial );

	m_pclSelComboHelperDpCBCV->FillComboConnect( &m_ComboConnect, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeConnect();
}

void CDlgIndSelDpCBCV::OnCbnSelChangeConnect()
{
	m_pclSelComboHelperDpCBCV->OnCbnSelChange( &m_ComboConnect );
	m_clIndSelDpCBCVParams.m_strComboConnectID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboConnect );

	m_pclSelComboHelperDpCBCV->FillComboVersion( &m_ComboVersion, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangeVersion();
}

void CDlgIndSelDpCBCV::OnCbnSelChangeVersion()
{
	m_pclSelComboHelperDpCBCV->OnCbnSelChange( &m_ComboVersion );
	m_clIndSelDpCBCVParams.m_strComboVersionID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboVersion );

	m_pclSelComboHelperDpCBCV->FillComboPN( &m_ComboPN, CDlgSelComboHelperBase::CS_All );
	OnCbnSelChangePN();
}

void CDlgIndSelDpCBCV::OnCbnSelChangePN()
{
	m_clIndSelDpCBCVParams.m_strComboPNID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboPN );

	_VerifyCheckboxSetStatus();
	_FillComboPowerSupply();
	OnCbnSelChangePowerSupply();
}

void CDlgIndSelDpCBCV::OnCbnSelChangePowerSupply()
{
	m_clIndSelDpCBCVParams.m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
	_FillComboInputSignal();
	OnCbnSelChangeInputSignal();
}

void CDlgIndSelDpCBCV::OnCbnSelChangeInputSignal()
{
	m_clIndSelDpCBCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	_SetCheckFailSafeFct();
	OnBnClickedCheckFailSafe();
}

void CDlgIndSelDpCBCV::OnCbnSelChangeDRPFct()
{
	m_clIndSelDpCBCVParams.m_eActuatorDRPFunction = CDB_ControlValve::drpfUndefined;
	
	if( m_ComboDRPFct.GetCurSel() >= 0 )
	{
        m_clIndSelDpCBCVParams.m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( m_ComboDRPFct.GetCurSel() );
	}

	ClearAll();
}

void CDlgIndSelDpCBCV::OnBnClickedCheckboxWithSTS()
{
	m_clIndSelDpCBCVParams.m_bIsWithSTSChecked = ( BST_CHECKED == m_CheckboxWithSTS.GetCheck() ) ? true : false;
	ClearAll();
}

void CDlgIndSelDpCBCV::OnBnClickedCheckboxSet()
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

	m_clIndSelDpCBCVParams.m_bOnlyForSet = ( BST_CHECKED == m_CheckboxSet.GetCheck() ) ? true : false;

	if( true == fCanClear )
	{
		ClearAll();
	}

	// Force a refresh of all ComboBox
	OnCbnSelChangeType();
}

void CDlgIndSelDpCBCV::OnBnClickedCheckShowInfoWnd()
{
	if( FALSE == IsWindowVisible() || false == m_bInitialised )
	{
		return;
	}

	m_clIndSelDpCBCVParams.m_bIsDpCInfoWndChecked = ( BST_CHECKED == m_CheckInfoWnd.GetCheck() ) ? true : false;
	
	if( true == m_clIndSelDpCBCVParams.m_bIsDpCInfoWndChecked )
	{
		// Creates the Info dialog if not already created.
		if( NULL == m_pInfoDialogDpCBCV->GetSafeHwnd() )
		{
			if( FALSE == m_pInfoDialogDpCBCV->Create() )
			{
				return;
			}
		}

		// Update picture.
		m_pInfoDialogDpCBCV->SetSchemePict( m_clIndSelDpCBCVParams.m_dDpToStabilize, m_clIndSelDpCBCVParams.m_bIsWithSTSChecked );

		if( NULL != pRViewSSelDpCBCV )
		{
			pRViewSSelDpCBCV->UpdateSVInfos();
		}
	}
	else
	{
		if( NULL != m_pInfoDialogDpCBCV->GetSafeHwnd() )
		{
			m_pInfoDialogDpCBCV->DestroyWindow();
		}
	}
}

void CDlgIndSelDpCBCV::OnBnClickedCheckFailSafe()
{
	m_clIndSelDpCBCVParams.m_iActuatorFailSafeFunction = ( BST_CHECKED == m_CheckboxFailSafe.GetCheck() ) ? 1 : 0;
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

LRESULT CDlgIndSelDpCBCV::OnModifySelectedDpCBCV( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelDpCBCV *pSelDpCBCV = dynamic_cast<CDS_SSelDpCBCV *>( ( CData * )lParam );
	ASSERT( NULL != pSelDpCBCV );

	if( NULL == pSelDpCBCV || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelDpCBCVParams.m_SelIDPtr = pSelDpCBCV->GetIDPtr();

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
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelDpCBCVParams.m_eApplicationType = pSelDpCBCV->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelDpCBCVParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelDpCBCVParams.m_WC = *( pSelDpCBCV->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelDpCBCV->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	m_clIndSelDpCBCVParams.m_bIsWithSTSChecked = ( BST_CHECKED == pSelDpCBCV->GetCheckboxWithSTS() ) ? true : false;
	m_CheckboxWithSTS.SetCheck( pSelDpCBCV->GetCheckboxWithSTS() );

	m_clIndSelDpCBCVParams.m_bOnlyForSet = pSelDpCBCV->IsSelectedAsAPackage();
	m_CheckboxSet.SetCheck( ( true == m_clIndSelDpCBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	// By default we enable the 'Set' checkbox. The state of this one will be checked below by calling the '_VerifyCheckboxSetStatus'.
	m_clIndSelDpCBCVParams.m_bCheckboxSetEnabled = true;
	m_CheckboxSet.EnableWindow( TRUE );

	m_pclSelComboHelperDpCBCV->PrepareComboContents();

	// 'Type' combo.
	m_pclSelComboHelperDpCBCV->FillComboType( &m_ComboType, CDlgSelComboHelperBase::CS_All, pSelDpCBCV->GetTypeID() );
	m_clIndSelDpCBCVParams.m_strComboTypeID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboType );

	// 'Control type' combo.
	m_pclSelComboHelperDpCBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All, pSelDpCBCV->GetCtrlType() );
	
	m_clIndSelDpCBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_pclSelComboHelperDpCBCV->GetIntFromCombo( &m_ComboCtrlType );

	if( iCur >= 0 )
	{
		m_clIndSelDpCBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)iCur;
	}

	// 'Family' combo.
	m_pclSelComboHelperDpCBCV->FillComboFamily( &m_ComboFamily, CDlgSelComboHelperBase::CS_All, pSelDpCBCV->GetFamilyID() );
	m_clIndSelDpCBCVParams.m_strComboFamilyID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboFamily );

	// 'Material' combo.
	m_pclSelComboHelperDpCBCV->FillComboBodyMaterial( &m_ComboMaterial, CDlgSelComboHelperBase::CS_All, pSelDpCBCV->GetMaterialID() );
	m_clIndSelDpCBCVParams.m_strComboMaterialID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboMaterial );

	// 'Connection' combo.
	m_pclSelComboHelperDpCBCV->FillComboConnect( &m_ComboConnect, CDlgSelComboHelperBase::CS_All, pSelDpCBCV->GetConnectID() );
	m_clIndSelDpCBCVParams.m_strComboConnectID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboConnect );

	// 'Version' combo.
	m_pclSelComboHelperDpCBCV->FillComboVersion( &m_ComboVersion, CDlgSelComboHelperBase::CS_All, pSelDpCBCV->GetVersionID() );
	m_clIndSelDpCBCVParams.m_strComboVersionID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboVersion );

	// 'PN' combo.
	m_pclSelComboHelperDpCBCV->FillComboPN( &m_ComboPN, CDlgSelComboHelperBase::CS_All, pSelDpCBCV->GetPNID() );
	m_clIndSelDpCBCVParams.m_strComboPNID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboPN );

	// We must now verify state of the checkbox before filling combo for actuator.
	_VerifyCheckboxSetStatus();

	// Remark: Corresponding combo variables in 'm_clIndSelDpCBCVParams' are updated in each of this following methods.
	_FillComboPowerSupply( pSelDpCBCV->GetActuatorPowerSupplyIDPtr() );
	_FillComboInputSignal( pSelDpCBCV->GetActuatorInputSignalIDPtr() );

	int iCheck = -1;
	if( 1 == pSelDpCBCV->GetFailSafeFunction() )
	{
		iCheck = BST_CHECKED;
	}
	else if( 0 == pSelDpCBCV->GetFailSafeFunction() )
	{
		iCheck = BST_UNCHECKED;
	}
	ASSERT( iCheck != -1 );
	m_CheckboxFailSafe.SetCheck( iCheck );
	m_clIndSelDpCBCVParams.m_iActuatorFailSafeFunction = pSelDpCBCV->GetFailSafeFunction();
	_VerifyCheckboxFailsafeStatus();
	_FillComboDRPFct( pSelDpCBCV->GetDRPFunction() );

	// Remark: The 'm_clIndSelDpCBCVParams.m_strPipeSeriesID' and 'm_clIndSelDpCBCVParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelDpCBCV->GetPipeSeriesID(), pSelDpCBCV->GetPipeID() );

	// Remark: The 'm_clIndSelDpCBCVParams.m_dDpToStabilize' is updated by calling the 'SetDpToStabilize' method.
	SetDpToStabilize( pSelDpCBCV->GetDpToStalibize() );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelDpCBCV->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.	
	UpdateData( FALSE );
	
	// Remark: 'm_clIndSelDpCBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelDpCBCVParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelDpCBCVParams.m_dFlow = pSelDpCBCV->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelDpCBCVParams.m_dPower = pSelDpCBCV->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelDpCBCVParams.m_dDT = pSelDpCBCV->GetDT();
		UpdateDTFieldValue();
	}

	pSelDpCBCV->ResetCVActrSetIDPtr();

	m_bInitialised = true;

	OnBnClickedCheckShowInfoWnd();

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );

	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

LRESULT CDlgIndSelDpCBCV::OnDestroyInfoDialogDpCBCV( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pInfoDialogDpCBCV && NULL != m_pInfoDialogDpCBCV->GetSafeHwnd() )
	{
		m_pInfoDialogDpCBCV->DestroyWindow();
	}

	m_clIndSelDpCBCVParams.m_bIsDpCInfoWndChecked = false;
	m_CheckInfoWnd.SetCheck( BST_UNCHECKED );

	return 0L;
}

void CDlgIndSelDpCBCV::OnEnChangeDpToStabilize()
{
	if( GetFocus() == &m_clExtEditDpToStabilize )
	{
		ClearAll();
		OnBnClickedCheckShowInfoWnd();
	}
}

void CDlgIndSelDpCBCV::OnEnKillFocusDpToStabilize()
{
	m_clIndSelDpCBCVParams.m_dDpToStabilize = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpToStabilize, &m_clIndSelDpCBCVParams.m_dDpToStabilize ) 
		|| m_clIndSelDpCBCVParams.m_dDpToStabilize < 0.0 )
	{
		m_clIndSelDpCBCVParams.m_dDpToStabilize = 0.0;
	}
}

void CDlgIndSelDpCBCV::OnBnClickedSuggest()
{
	if( NULL == pRViewSSelDpCBCV )
	{
		return;
	}

	if( m_clIndSelDpCBCVParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetpEditFlow()->SetFocus();
		return;
	}

	// Clear the right view.
	ClearAll();

	// OnBnClickedButtonsuggest is called several time during initialisation;
	// return if not yet fully initialized
	if( false == m_bInitialised )
	{
		return;
	}

	// Create the Select DpCBCVList to verify if at least one device exist.
	if( NULL != m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList )
	{
		delete m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList;
	}

	m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList = new CSelectDpCBCVList();

	if( NULL == m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList || NULL == m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->GetSelectPipeList() )
	{
		ASSERT_RETURN;
	}

	m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->GetSelectPipeList()->SelectPipes( &m_clIndSelDpCBCVParams, m_clIndSelDpCBCVParams.m_dFlow );

	m_clIndSelDpCBCVParams.m_CtrlList.PurgeAll();

	int iValveCount = m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(
			&m_clIndSelDpCBCVParams.m_CtrlList,
			CTADatabase::eForDpCBCV,
			false,
			m_clIndSelDpCBCVParams.m_eCV2W3W,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboVersionID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboPNID,
			CDB_ControlProperties::LastCVFUNC,
			m_clIndSelDpCBCVParams.m_eCvCtrlType,
			m_clIndSelDpCBCVParams.m_eFilterSelection,
			0,
			INT_MAX,
			false,
			NULL,
			m_clIndSelDpCBCVParams.m_bOnlyForSet );

	bool bSizeShiftProblem = false;
	bool bTryAlternative = false;

	if( 0 == iValveCount )
	{
		bTryAlternative = true;
	}
	else
	{
		bool bValidFound = false;
		int iDevFound = m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->SelectDpCBCV( &m_clIndSelDpCBCVParams, &bValidFound, &bSizeShiftProblem );

		if( iDevFound > 0 && false == bSizeShiftProblem )
		{
			VerifyModificationMode();
			pRViewSSelDpCBCV->Suggest( &m_clIndSelDpCBCVParams, (LPARAM)m_pInfoDialogDpCBCV );
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

void CDlgIndSelDpCBCV::OnBnClickedSelect()
{
	try
	{
		if( NULL == m_clIndSelDpCBCVParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelDpCBCVParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelDpCBCVParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelDpCBCVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelDpCBCV )
		{
			HYSELECT_THROW( _T("Internal error: The combined Dp controller balancing & control valve right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The combined Dp controller balancing & control valve right view is not visible.") );
		}

		// Verify pointer validity; user can delete object after entering in Edition.
		if( _T('\0') != *m_clIndSelDpCBCVParams.m_SelIDPtr.ID )
		{
			m_clIndSelDpCBCVParams.m_SelIDPtr = m_clIndSelDpCBCVParams.m_pTADS->Get( m_clIndSelDpCBCVParams.m_SelIDPtr.ID );
		}

		if( m_clIndSelDpCBCVParams.m_dFlow <= 0.0 )
		{
			HYSELECT_THROW( _T("Internal error: Flow '%f' can't be lower or equal to 0."), m_clIndSelDpCBCVParams.m_dFlow );
		}

		// Create selected object and initialize it.
		CDS_SSelDpCBCV *pSelDpCBCV = dynamic_cast<CDS_SSelDpCBCV *>( m_clIndSelDpCBCVParams.m_SelIDPtr.MP );

		if( NULL == pSelDpCBCV )
		{
			IDPTR IDPtr;
			m_clIndSelDpCBCVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelDpCBCV ) );
			pSelDpCBCV = ( CDS_SSelDpCBCV * )IDPtr.MP;
		}

		pSelDpCBCV->SetQ( m_clIndSelDpCBCVParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelDpCBCVParams.m_eFlowOrPowerDTMode )
		{
			pSelDpCBCV->SetFlowDef( CDS_SelProd::efdFlow );
			pSelDpCBCV->SetPower( 0.0 );
			pSelDpCBCV->SetDT( 0.0 );
		}
		else
		{
			pSelDpCBCV->SetFlowDef( CDS_SelProd::efdPower );
			pSelDpCBCV->SetPower( m_clIndSelDpCBCVParams.m_dPower );
			pSelDpCBCV->SetDT( m_clIndSelDpCBCVParams.m_dDT );
		}

		pSelDpCBCV->SetDpToStalibize( m_clIndSelDpCBCVParams.m_dDpToStabilize );

		pSelDpCBCV->SetCheckboxWithSTS( m_clIndSelDpCBCVParams.m_bIsWithSTSChecked );
		pSelDpCBCV->SetSelectedAsAPackage( m_clIndSelDpCBCVParams.m_bOnlyForSet );
		pSelDpCBCV->SetCheckPackageEnable( ( true == m_clIndSelDpCBCVParams.m_bCheckboxSetEnabled ) ? TRUE : FALSE );
		pSelDpCBCV->SetTypeID( m_clIndSelDpCBCVParams.m_strComboTypeID );
		pSelDpCBCV->SetCtrlType( m_clIndSelDpCBCVParams.m_eCvCtrlType );
		pSelDpCBCV->SetFamilyID( m_clIndSelDpCBCVParams.m_strComboFamilyID );
		pSelDpCBCV->SetMaterialID( m_clIndSelDpCBCVParams.m_strComboMaterialID );
		pSelDpCBCV->SetConnectID( m_clIndSelDpCBCVParams.m_strComboConnectID );
		pSelDpCBCV->SetVersionID( m_clIndSelDpCBCVParams.m_strComboVersionID );
		pSelDpCBCV->SetPNID( m_clIndSelDpCBCVParams.m_strComboPNID );

		// Set the actuators info.
		pSelDpCBCV->SetActuatorPowerSupplyID( m_clIndSelDpCBCVParams.m_strActuatorPowerSupplyID );
		pSelDpCBCV->SetActuatorInputSignalID( m_clIndSelDpCBCVParams.m_strActuatorInputSignalID );
		pSelDpCBCV->SetFailSafeFunction( m_clIndSelDpCBCVParams.m_iActuatorFailSafeFunction );
		pSelDpCBCV->SetDRPFunction( m_clIndSelDpCBCVParams.m_eActuatorDRPFunction );

		pSelDpCBCV->SetPipeSeriesID( m_clIndSelDpCBCVParams.m_strPipeSeriesID );
		pSelDpCBCV->SetPipeID( m_clIndSelDpCBCVParams.m_strPipeID );

		*pSelDpCBCV->GetpSelectedInfos()->GetpWCData() = m_clIndSelDpCBCVParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelDpCBCV->GetpSelectedInfos()->SetDT( m_clIndSelDpCBCVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelDpCBCV->GetpSelectedInfos()->SetApplicationType( m_clIndSelDpCBCVParams.m_eApplicationType );

		pRViewSSelDpCBCV->FillInSelected( pSelDpCBCV );

		// Show the confirm selection dialog box.
		CDlgConfSel dlg( &m_clIndSelDpCBCVParams );
		IDPTR IDPtr = pSelDpCBCV->GetIDPtr();

		dlg.Display( pSelDpCBCV );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelDpCBCVParams.m_pTADS->Get( _T("DPCBCVALVE_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'DPCBCVALVE_TAB' table from the datastruct.") );
			}

			if( _T('\0') == *m_clIndSelDpCBCVParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelDpCBCVParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelDpCBCV ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelDpCBCVParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelDpCBCVParams.m_SelIDPtr.ID )
			{
				m_clIndSelDpCBCVParams.m_pTADS->DeleteObject( IDPtr );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelDpCBCV::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelDpCBCV::OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITDPL == pNMHDR->idFrom )
	{
		GetpBtnSuggest()->SetFocus();

		// Reset Focus on the modified Edit.
		m_clExtEditDpToStabilize.SetFocus();

		PostWMCommandToControl( GetpBtnSuggest() );
	}
	else
	{
		CDlgIndSelCtrlBase::OnEditEnterChar( pNMHDR, pResult );
	}
}

LRESULT CDlgIndSelDpCBCV::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnNewDocument( wParam, lParam );

	// Reinit parameters.
	ClearCommonInputEditors();

	if( false == TASApp.IsDpCBCVDisplayed() )
	{
		return 0;
	}

	// Get last selected parameters.
	// Remark: if user clicks create a new document, the 'SaveSelectionParameters' is automatically called before 'OnNewDocument'.
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelDpCBCVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_clIndSelDpCBCVParams.m_bOnlyForSet = ( 0 == pclIndSelParameter->GetDpCBCVPackageChecked() ) ? false : true;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelDpCBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );
	
	// By default we enable the 'Set' checkbox. The state of this one will be checked below by calling the '_VerifyCheckboxSetStatus'.
	m_clIndSelDpCBCVParams.m_bCheckboxSetEnabled = true;
	m_CheckboxSet.EnableWindow( TRUE );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetDpCBCVRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelDpCBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	m_clIndSelDpCBCVParams.m_bIsDpCInfoWndChecked = ( ( 0 == pclIndSelParameter->GetDpCBCVInfoWnd() ) ? false : true );
	m_CheckInfoWnd.SetCheck( ( true == m_clIndSelDpCBCVParams.m_bIsDpCInfoWndChecked ) ? BST_CHECKED : BST_UNCHECKED );

	m_pclSelComboHelperDpCBCV->PrepareComboContents();

	// 'Type' combo.
	m_pclSelComboHelperDpCBCV->FillComboType( &m_ComboType, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetDpCBCVTypeID() );
	m_clIndSelDpCBCVParams.m_strComboTypeID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboType );

	// 'Control type' combo.
	m_pclSelComboHelperDpCBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetDpCBCVCBCtrlType() );

	m_clIndSelDpCBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_pclSelComboHelperDpCBCV->GetIntFromCombo( &m_ComboCtrlType );

	if( iCur >= 0 )
	{
		m_clIndSelDpCBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)iCur;
	}

	// 'Family' combo.
	m_pclSelComboHelperDpCBCV->FillComboFamily( &m_ComboFamily, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetDpCBCVFamilyID() );
	m_clIndSelDpCBCVParams.m_strComboFamilyID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboFamily );

	// 'Material' combo.
	m_pclSelComboHelperDpCBCV->FillComboBodyMaterial( &m_ComboMaterial, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetDpCBCVMaterialID() );
	m_clIndSelDpCBCVParams.m_strComboMaterialID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboMaterial );

	// 'Connection' combo.
	m_pclSelComboHelperDpCBCV->FillComboConnect( &m_ComboConnect, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetDpCBCVConnectID() );
	m_clIndSelDpCBCVParams.m_strComboConnectID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboConnect );

	// 'Version' combo.
	m_pclSelComboHelperDpCBCV->FillComboVersion( &m_ComboVersion, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetDpCBCVVersionID() );
	m_clIndSelDpCBCVParams.m_strComboVersionID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboVersion );

	// 'PN' combo.
	m_pclSelComboHelperDpCBCV->FillComboPN( &m_ComboPN, CDlgSelComboHelperBase::CS_All, pclIndSelParameter->GetDpCBCVPNID() );
	m_clIndSelDpCBCVParams.m_strComboPNID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboPN );

	// We must now verify state of the checkbox before filling combo for actuator.
	_VerifyCheckboxSetStatus();

	// Remark: Corresponding combo variables in 'm_clIndSelDpCBCVParams' are updated in each of this following methods.
	_FillComboPowerSupply( pclIndSelParameter->GetDpCBCVActPowerSupplyID() );
	_FillComboInputSignal( pclIndSelParameter->GetDpCBCVActInputSignalID() );
	
	int iCheck = -1;

	// HYS-1313: In old version, fail safe function was a combo with 4 values (fsfNone, fsfClosing, fsfOpening and fsfAll).
	// It can happen that we read here value of 2 or 3!
	if( 0 == pclIndSelParameter->GetDpCBCVFailSafeFct() )
	{
		iCheck = BST_UNCHECKED;
	}
	else
	{
		iCheck = BST_CHECKED;
	}
	
	m_CheckboxFailSafe.SetCheck( iCheck );
	m_clIndSelDpCBCVParams.m_iActuatorFailSafeFunction = iCheck;
	_VerifyCheckboxFailsafeStatus();
	_FillComboDRPFct( pclIndSelParameter->GetDpCBCVDRPFct() );

	OnCbnSelChangeDRPFct();

	m_clIndSelDpCBCVParams.m_bIsWithSTSChecked = ( 0 == pclIndSelParameter->GetDpCBCVWithSTSCheckBox() ) ? false : true;
	m_CheckboxWithSTS.SetCheck( ( true == m_clIndSelDpCBCVParams.m_bIsWithSTSChecked ) ? BST_CHECKED : BST_UNCHECKED );

	// Fill the flow and update water TCHAR. strings.
	OnUnitChange();
	
	m_bInitialised = true;

	EnableSelectButton( false );
	OnBnClickedCheckShowInfoWnd();

	if( NULL != pRViewSSelDpCBCV )
	{
		pRViewSSelDpCBCV->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}

LRESULT CDlgIndSelDpCBCV::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != ( WMUserPipeChange )wParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnPipeChange( wParam, lParam );

	// Clear the right view.
	if( NULL != pRViewSSelDpCBCV )
	{
		pRViewSSelDpCBCV->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelDpCBCV::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnTechParChange( wParam, lParam );

	// Clear the right view.
	if( NULL != pRViewSSelDpCBCV )
	{
		pRViewSSelDpCBCV->Reset();
	}

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();

	return 0;
}

LRESULT CDlgIndSelDpCBCV::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnUnitChange( wParam, lParam );

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATICDPLUNIT, name );

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();

	// Clear the right view.
	if( NULL != pRViewSSelDpCBCV )
	{
		pRViewSSelDpCBCV->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelDpCBCV::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( ( WMUserWaterCharWParam )wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel )
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelCtrlBase::OnWaterChange( wParam, lParam );

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();

	// Clear the right view.
	if( NULL != pRViewSSelDpCBCV )
	{
		pRViewSSelDpCBCV->Reset();
	}

	return 0;
}

CRViewSSelSS *CDlgIndSelDpCBCV::GetLinkedRightViewSSel( void )
{
	return pRViewSSelDpCBCV;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelDpCBCV::_FillComboPowerSupply( CString strPowerSupplyID )
{
	CRankEx DpCBCVList;
	// HYS-1448 : DpCBCv type have to be considered
	m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(
			&DpCBCVList,												// List where to saved
			CTADatabase::eForDpCBCV,									// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			CDB_ControlProperties::LastCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboTypeID,			// Type ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboFamilyID,			// Family ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboMaterialID,		// Body material ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboConnectID,		// Connection ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboVersionID,		// Version ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboPNID,				// PN ID
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			m_clIndSelDpCBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelDpCBCVParams.m_eFilterSelection,
			0,															// DNMin
			INT_MAX,													// DNMax
			false,														// 'true' if it's for hub station.
			NULL,														// 'pProd'.
			m_clIndSelDpCBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.

	CRankEx PowerSupplyList;
	m_clIndSelDpCBCVParams.m_pTADB->GetTaCVActuatorPowerSupplyList( &PowerSupplyList, &DpCBCVList, m_clIndSelDpCBCVParams.m_eCvCtrlType, IsSelectionBySet(), m_clIndSelDpCBCVParams.m_eFilterSelection );
	m_ComboPowerSupply.FillInCombo( &PowerSupplyList, strPowerSupplyID, m_clIndSelDpCBCVParams.GetComboActuatorPowerSupplyAllID() );
	m_clIndSelDpCBCVParams.m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelDpCBCV::_FillComboInputSignal( CString strInputSignalID )
{
	if( 0 == m_ComboPowerSupply.GetCount() )
	{
		m_ComboInputSignal.ResetContent();
		m_ComboInputSignal.EnableWindow( FALSE );
		m_clIndSelDpCBCVParams.m_strActuatorInputSignalID = _T("");
	}
	else
	{
		CRankEx DpCBCVList;
		// HYS-1448 : DpCBCv type have to be considered
		m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(	
				&DpCBCVList,												// List where to saved
				CTADatabase::eForDpCBCV,									// Control valve target (cv, hmcv, picv or bcv)
				false, 														// 'true' returns as soon a result is found
				CDB_ControlProperties::LastCV2W3W, 							// Set way number of valve
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboTypeID,			// Type ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboFamilyID,			// Family ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboMaterialID,		// Body material ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboConnectID,		// Connection ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboVersionID,		// Version ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboPNID,				// PN ID
				CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
				m_clIndSelDpCBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSelDpCBCVParams.m_eFilterSelection,
				0,															// DNMin
				INT_MAX,													// DNMax
				false,														// 'true' if it's for hub station.
				NULL,														// 'pProd'.
				m_clIndSelDpCBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.

		CRankEx InputSignalList;

		m_clIndSelDpCBCVParams.m_pTADB->GetTaCVActuatorInputSignalList( &InputSignalList, &DpCBCVList, m_clIndSelDpCBCVParams.m_eCvCtrlType, 
				m_clIndSelDpCBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelDpCBCVParams.m_strActuatorPowerSupplyID, m_clIndSelDpCBCVParams.m_eFilterSelection );

		m_ComboInputSignal.FillInCombo( &InputSignalList, strInputSignalID, m_clIndSelDpCBCVParams.GetComboActuatorInputSignalAllID() );
		m_clIndSelDpCBCVParams.m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	}
}

void CDlgIndSelDpCBCV::_SetCheckFailSafeFct()
{
	if( 0 == m_ComboInputSignal.GetCount() )
	{
		// Checkbox unchecked and disabled
		m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
		m_CheckboxFailSafe.EnableWindow( false );
	}
	else
	{
		// HYS-1448 : DpCBCv type have to be considered
		CRankEx DpCBCVList;
		m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(
			&DpCBCVList,												// List where to saved
			CTADatabase::eForDpCBCV,									// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			CDB_ControlProperties::LastCV2W3W, 							// Set way number of valve
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboTypeID,			// Type ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboFamilyID,			// Family ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboMaterialID,		// Body material ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboConnectID,		// Connection ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboVersionID,		// Version ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboPNID,				// PN ID
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			m_clIndSelDpCBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelDpCBCVParams.m_eFilterSelection,
			0,															// DNMin
			INT_MAX,													// DNMax
			false,														// 'true' if it's for hub station.
			NULL,														// 'pProd'.
			m_clIndSelDpCBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.

		m_CheckStatus = m_clIndSelDpCBCVParams.m_pTADB->GetTaCVActuatorFailSafeValues( &DpCBCVList, m_clIndSelDpCBCVParams.m_eCvCtrlType, m_clIndSelDpCBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelDpCBCVParams.m_strActuatorPowerSupplyID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strActuatorInputSignalID, m_clIndSelDpCBCVParams.m_eFilterSelection );
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

void CDlgIndSelDpCBCV::_FillComboDRPFct( CDB_ControlValve::DRPFunction eDRPFct )
{
	if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus && false == m_CheckboxFailSafe.IsWindowEnabled() )
	{
		m_ComboDRPFct.ResetContent();
		m_ComboDRPFct.EnableWindow( FALSE );
		m_clIndSelDpCBCVParams.m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfUndefined;
	}
	else
	{
		// HYS-1448 : DpCBCv type have to be considered
		CRankEx DpCBCVList;
		m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(
				&DpCBCVList,												// List where to saved
				CTADatabase::eForDpCBCV,									// Control valve target (cv, hmcv, picv or bcv)
				false, 														// 'true' returns as soon a result is found
				CDB_ControlProperties::LastCV2W3W, 							// Set way number of valve
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboTypeID,				// Type ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboFamilyID,			// Family ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboMaterialID,		// Body material ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboConnectID,		// Connection ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboVersionID,		// Version ID
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboPNID,				// PN ID
				CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
				m_clIndSelDpCBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
				m_clIndSelDpCBCVParams.m_eFilterSelection,
				0,															// DNMin
				INT_MAX,													// DNMax
				false,														// 'true' if it's for hub station.
				NULL,														// 'pProd'.
				m_clIndSelDpCBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.
							

		CRankEx DRPFctList;

		m_clIndSelDpCBCVParams.m_pTADB->GetTaCVDRPFunctionList( &DRPFctList, m_clIndSelDpCBCVParams.m_eCvCtrlType, &DpCBCVList, m_clIndSelDpCBCVParams.m_bOnlyForSet,
				(LPCTSTR)m_clIndSelDpCBCVParams.m_strActuatorPowerSupplyID, (LPCTSTR)m_clIndSelDpCBCVParams.m_strActuatorInputSignalID, 
			m_clIndSelDpCBCVParams.m_iActuatorFailSafeFunction, m_clIndSelDpCBCVParams.m_eFilterSelection );

		m_ComboDRPFct.ResetContent();
		int iSelPos = 0;

		if( DRPFctList.GetCount() > 0 )
		{
			DRPFctList.Transfer( &m_ComboDRPFct );

			// If we have more than one item in the combo, we insert "** All fail safe **" string.
			if( m_ComboDRPFct.GetCount() > 1 )
			{
				CString str = TASApp.LoadLocalizedString( m_clIndSelDpCBCVParams.GetComboActuatorFailSafeAllID() );
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
		m_ComboDRPFct.EnableWindow( ( m_ComboDRPFct.GetCount() <= 1 ) ? FALSE : TRUE );
		m_clIndSelDpCBCVParams.m_eActuatorDRPFunction = ( CDB_ControlValve::DRPFunction )m_ComboDRPFct.GetItemData( iSelPos );
	}
}

void CDlgIndSelDpCBCV::_LaunchSuggestion( CString strTypeID, CDB_ControlProperties::CvCtrlType eCvCtrlType, CString strFamilyID, CString strMaterialID, 
		CString strConnectID, CString strVersionID, CString strPNID, bool bSelectionBySet )
{
	// 'Type' combo.
	m_pclSelComboHelperDpCBCV->FillComboType( &m_ComboType, CDlgSelComboHelperBase::CS_All, strTypeID );
	m_clIndSelDpCBCVParams.m_strComboTypeID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboType );

	// 'Control type' combo.
	m_pclSelComboHelperDpCBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All, eCvCtrlType );
	
	m_clIndSelDpCBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	int iCur = m_pclSelComboHelperDpCBCV->GetIntFromCombo( &m_ComboCtrlType );

	if( iCur >= 0 )
	{
		m_clIndSelDpCBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)iCur;
	}

	// 'Family' combo.
	m_pclSelComboHelperDpCBCV->FillComboFamily( &m_ComboFamily, CDlgSelComboHelperBase::CS_All, strFamilyID );
	m_clIndSelDpCBCVParams.m_strComboFamilyID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboFamily );

	// 'Material' combo.	
	m_pclSelComboHelperDpCBCV->FillComboBodyMaterial( &m_ComboMaterial, CDlgSelComboHelperBase::CS_All, strMaterialID );
	m_clIndSelDpCBCVParams.m_strComboMaterialID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboMaterial );

	// 'Connection' combo.	
	m_pclSelComboHelperDpCBCV->FillComboConnect( &m_ComboConnect, CDlgSelComboHelperBase::CS_All, strConnectID );
	m_clIndSelDpCBCVParams.m_strComboConnectID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboConnect );

	// 'Version' combo.	
	m_pclSelComboHelperDpCBCV->FillComboVersion( &m_ComboVersion, CDlgSelComboHelperBase::CS_All, strVersionID );
	m_clIndSelDpCBCVParams.m_strComboVersionID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboVersion );

	// 'PN' combo.	
	m_pclSelComboHelperDpCBCV->FillComboPN( &m_ComboPN, CDlgSelComboHelperBase::CS_All, strPNID );
	m_clIndSelDpCBCVParams.m_strComboPNID = m_pclSelComboHelperDpCBCV->GetIDFromCombo( &m_ComboPN );

	m_clIndSelDpCBCVParams.m_bOnlyForSet = bSelectionBySet;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelDpCBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	OnCbnSelChangePN();

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelDpCBCV::_TryAlternativeSuggestion( bool bSizeShiftProblem )
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

		// Remark: start to 1 because the strict case has been already done in the 'OnBnClickedSuggest' method.
		for( int iLoopAlternative = 1; iLoopAlternative < 4 && false == bStop; iLoopAlternative++ )
		{
			m_clIndSelDpCBCVParams.m_CtrlList.PurgeAll();
			int iValveCount = 0;
			bool bSetFlag = ( bitSet == ( iLoopAlternative & bitSet ) ) ? true : false;
			bool bRelaxCombo = ( bitCombos == ( iLoopAlternative & bitCombos ) ) ? true : false;

			// We do these cases only if user has selected set.
			if( true == bSetFlag && false == m_clIndSelDpCBCVParams.m_bOnlyForSet )
			{
				continue;
			}

			bool bOnlySet = ( false == bSetFlag ) ? m_clIndSelDpCBCVParams.m_bOnlyForSet : false;

			CString strTypeID( _T( "" ) );
			CString strFamilyID( _T( "" ) );
			CString strBodyMaterialID( _T( "" ) );
			CString strConnectID( _T( "" ) );
			CString strVersionID( _T( "" ) );
			CString strPNID( _T( "" ) );

			if( false == bRelaxCombo )
			{
				strTypeID = m_pclSelComboHelperDpCBCV->GetIDFromCombo(&m_ComboType);
				strFamilyID = m_pclSelComboHelperDpCBCV->GetIDFromCombo(&m_ComboFamily);
				strBodyMaterialID = m_pclSelComboHelperDpCBCV->GetIDFromCombo(&m_ComboMaterial);
				strConnectID = m_pclSelComboHelperDpCBCV->GetIDFromCombo(&m_ComboConnect);
				strVersionID = m_pclSelComboHelperDpCBCV->GetIDFromCombo(&m_ComboVersion);
				strPNID = m_pclSelComboHelperDpCBCV->GetIDFromCombo(&m_ComboPN);
			}

			iValveCount = m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(
					&m_clIndSelDpCBCVParams.m_CtrlList,			// List where to saved
					CTADatabase::eForDpCBCV,					// Control valve target (cv, hmcv, picv or bcv)
					false, 										// 'true' returns as soon a result is found
					m_clIndSelDpCBCVParams.m_eCV2W3W, 			// Set way number of valve
					(LPCTSTR)strTypeID,							// Type ID
					(LPCTSTR)strFamilyID,						// Family ID
					(LPCTSTR)strBodyMaterialID,					// Body material ID
					(LPCTSTR)strConnectID,						// Connection ID
					(LPCTSTR)strVersionID, 						// Version ID
					(LPCTSTR)strPNID,							// PN ID
					CDB_ControlProperties::LastCVFUNC, 			// Set the control function (control only, presettable, ...)
					m_clIndSelDpCBCVParams.m_eCvCtrlType,		// Set the control type (on/off, proportional, ...)
					m_clIndSelDpCBCVParams.m_eFilterSelection,
					0,											// DNMin
					INT_MAX,									// DNMax
					false,										// 'true' if it's for hub station.
					NULL,										// 'pProd'.
					bOnlySet );									// 'true' if it's only for a set.
			
			if( 0 == iValveCount )
			{
				continue;
			}

			bool bValidFound = false;

			iDevFound = m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->SelectDpCBCV( &m_clIndSelDpCBCVParams, &bValidFound, &bSizeShiftProblem );

			if( iDevFound > 0 && false == bSizeShiftProblem )
			{
				if( true == bRelaxCombo )
				{
					// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevCtrlFound' dialog.
					m_clIndSelDpCBCVParams.m_CtrlList.PurgeAll();
					double dKey = 0.0;

					for( CSelectedValve *pclSelectedValve = m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
						pclSelectedValve = m_clIndSelDpCBCVParams.m_pclSelectDpCBCVList->GetNext<CSelectedValve>() )
					{
						CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

						if( NULL == pTAP )
						{
							continue;
						}

						m_clIndSelDpCBCVParams.m_CtrlList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
					}

					// Launch the 'No device found' dialog box.
					// Remark: Combos in this dialog will be filled thanks to the new alternative combined Dp controller, balancing and control valves found 
					//         and saved in the 'DpCBCVList' variable.
					CDlgNoDevCtrlFoundParams clNoDevCtrlParams = CDlgNoDevCtrlFoundParams( &m_clIndSelDpCBCVParams.m_CtrlList, m_clIndSelDpCBCVParams.m_strComboTypeID, m_clIndSelDpCBCVParams.m_eCvCtrlType, 
						m_clIndSelDpCBCVParams.m_strComboFamilyID, m_clIndSelDpCBCVParams.m_strComboMaterialID, m_clIndSelDpCBCVParams.m_strComboConnectID, 
						m_clIndSelDpCBCVParams.m_strComboVersionID, m_clIndSelDpCBCVParams.m_strComboPNID );

					CDlgNoDevCtrlFound dlg( &m_clIndSelDpCBCVParams, &clNoDevCtrlParams );

					if( IDOK == dlg.DoModal() )
					{
						clNoDevCtrlParams = *dlg.GetNoDevCtrlFoundParams();

						_LaunchSuggestion( clNoDevCtrlParams.m_strTypeID, clNoDevCtrlParams.m_eCvCtrlType, clNoDevCtrlParams.m_strFamilyID, clNoDevCtrlParams.m_strMaterialID, clNoDevCtrlParams.m_strConnectID,
								clNoDevCtrlParams.m_strVersionID, clNoDevCtrlParams.m_strPNID, bOnlySet );
					}

					bStop = true;
				}
				else
				{
					if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_INDSEL_CTRLVALVENOTFOUND_NOSET, MB_YESNO | MB_ICONQUESTION, 0 ) )
					{
						_LaunchSuggestion( m_clIndSelDpCBCVParams.m_strComboTypeID, m_clIndSelDpCBCVParams.m_eCvCtrlType, m_clIndSelDpCBCVParams.m_strComboFamilyID, 
								m_clIndSelDpCBCVParams.m_strComboMaterialID, m_clIndSelDpCBCVParams.m_strComboConnectID, m_clIndSelDpCBCVParams.m_strComboVersionID, 
								m_clIndSelDpCBCVParams.m_strComboPNID, bOnlySet );
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

void CDlgIndSelDpCBCV::_VerifyCheckboxSetStatus()
{
	CRankEx DpCBCVList;

	m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(
			&DpCBCVList,												// List where to saved
			CTADatabase::eForDpCBCV,									// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			m_clIndSelDpCBCVParams.m_eCV2W3W, 							// Set way number of valve
			_T( "" ),							 						// Type ID
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboFamilyID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboMaterialID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboVersionID,
			(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboPNID,
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			m_clIndSelDpCBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
			m_clIndSelDpCBCVParams.m_eFilterSelection );

	// Remark: the 'm_clIndSelDpCBCVParams.m_bOnlyForSet' variable is updated in this method.
	VerifyCheckboxSetStatus( &DpCBCVList );

	m_CheckboxSet.SetCheck( ( true == m_clIndSelDpCBCVParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );
	m_CheckboxSet.EnableWindow( ( true == m_clIndSelDpCBCVParams.m_bCheckboxSetEnabled ) ? BST_CHECKED : BST_UNCHECKED );
}

void CDlgIndSelDpCBCV::_VerifyCheckboxFailsafeStatus()
{
	CRankEx DpCBCVList;

	m_clIndSelDpCBCVParams.m_pTADB->GetTaCVList(
		&DpCBCVList,												// List where to saved
		CTADatabase::eForDpCBCV,									// Control valve target (cv, hmcv, picv or bcv)
		false, 														// 'true' returns as soon a result is found
		m_clIndSelDpCBCVParams.m_eCV2W3W, 							// Set way number of valve
		_T( "" ),							 						// Type ID
		(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboFamilyID,
		(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboMaterialID,
		(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboConnectID,
		(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboVersionID,
		(LPCTSTR)m_clIndSelDpCBCVParams.m_strComboPNID,
		CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
		m_clIndSelDpCBCVParams.m_eCvCtrlType,						// Set the control type (on/off, proportional, ...)
		m_clIndSelDpCBCVParams.m_eFilterSelection );


	m_CheckStatus = m_clIndSelDpCBCVParams.m_pTADB->GetTaCVActuatorFailSafeValues( &DpCBCVList, m_clIndSelDpCBCVParams.m_eCvCtrlType, m_clIndSelDpCBCVParams.m_bOnlyForSet, (LPCTSTR)m_clIndSelDpCBCVParams.m_strActuatorPowerSupplyID,
		(LPCTSTR)m_clIndSelDpCBCVParams.m_strActuatorInputSignalID, m_clIndSelDpCBCVParams.m_eFilterSelection );
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