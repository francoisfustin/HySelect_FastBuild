#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Select.h"

#include "DlgNoDevSafetyValveFound.h"
#include "DlgConfSel.h"
#include "DlgWaterChar.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelSafetyValve.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelSafetyValve.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelSafetyValve::CDlgIndSelSafetyValve( CWnd *pParent )
	: CDlgIndSelBase( m_clIndSelSafetyValveParams, CDlgIndSelSafetyValve::IDD, pParent )
{
	m_clIndSelSafetyValveParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_SafetyValve;
	m_clIndSelSafetyValveParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;
}

ProjectType CDlgIndSelSafetyValve::GetSytemType()
{
	ProjectType eProjectType = Heating;
	int iCur = m_ComboSystemType.GetCurSel();

	if( iCur >= 0 )
	{
        eProjectType = (ProjectType)m_ComboSystemType.GetItemData( iCur );
	}

	return eProjectType;
}

CString CDlgIndSelSafetyValve::GetNormID()
{
	CString strNormID = _T("PM_NONE");
	int iCur = m_ComboNorm.GetCurSel();

	if( iCur >= 0 )
	{
		CDB_StringID *pStrID = NULL;
		pStrID = dynamic_cast<CDB_StringID *>( (CData *)m_ComboNorm.GetItemData( iCur ) );

		if( NULL != pStrID )
		{
			strNormID = pStrID->GetIDPtr().ID;
		}
	}

	return strNormID;
}

double CDlgIndSelSafetyValve::GetSafetyValveSetPressure()
{
	double dSetPressure = 0.0;

	int iCur = m_ComboSafetyValveSetPressure.GetCurSel();

	if( iCur >= 0 )
	{
		dSetPressure = (double)m_ComboSafetyValveSetPressure.GetItemData( iCur );
	}

	return dSetPressure;
}

void CDlgIndSelSafetyValve::SaveSelectionParameters()
{
	CDlgIndSelBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSafetyValveParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetSafetyValveSystemType( m_clIndSelSafetyValveParams.m_eSystemApplicationType );
	pclIndSelParameter->SetSafetyValveSystemHeatGeneratorTypeID( m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID );
	pclIndSelParameter->SetSafetyValveNormID( m_clIndSelSafetyValveParams.m_strNormID );
	pclIndSelParameter->SetSafetyValveFamilyID( m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID );
	pclIndSelParameter->SetSafetyValveConnectionID( m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID );
	pclIndSelParameter->SetSafetyValveSetPressure( m_clIndSelSafetyValveParams.m_dUserSetPressureChoice );

	if( NULL != pRViewSSelSafetyValve )
	{
		pRViewSSelSafetyValve->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelSafetyValve::ActivateLeftTabDialog()
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::ActivateLeftTabDialog();

	ProductSubCategory eProductSubCategory;
	ProductSelectionMode eProductSelectionMode;

	if( true == pDlgLeftTabSelManager->GetCurrentProductSubCategoryAndSelectionMode( eProductSubCategory, eProductSelectionMode )
			&& ProductSubCategory::PSC_PM_SafetyValve == eProductSubCategory
			&& ProductSelectionMode::ProductSelectionMode_Individual == eProductSelectionMode )
	{
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_APPLICATIONTYPECHANGE, (WPARAM)m_clIndSelSafetyValveParams.m_eSystemApplicationType );
	}

	if( this == pDlgLeftTabSelManager->GetCurrentLeftTabDialog() )
	{
		_VerifyFluidCharacteristics( false );
	}
}

void CDlgIndSelSafetyValve::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelBase::ChangeApplicationType( eProductSelectionApplicationType );
	m_clIndSelSafetyValveParams.m_eSystemApplicationType = eProductSelectionApplicationType;
	_FillComboSystemType( m_clIndSelSafetyValveParams.m_eSystemApplicationType );
	_ChangeApplicationType();
}

void CDlgIndSelSafetyValve::OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg, pclDlgWaterChar );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelSafetyValve, CDlgIndSelBase )
	ON_CBN_SELCHANGE( IDC_COMBO_SYSTEMTYPE, OnCbnSelChangeSystemType )
	ON_CBN_SELCHANGE( IDC_COMBO_SYSTEMHEATGENERATORTYPE, OnCbnSelChangeSystemHeatGeneratorTypeID )
	ON_CBN_SELCHANGE( IDC_COMBO_NORM, OnCbnSelChangeNormID )
	ON_CBN_SELCHANGE( IDC_COMBO_FAMILY, OnCbnSelChangeSafetyValveFamilyID )
	ON_CBN_SELCHANGE( IDC_COMBO_CONNECT, OnCbnSelChangeSafetyValveConnectID )
	ON_CBN_SELCHANGE( IDC_COMBO_SETPRESSURE, OnCbnSelChangeSafetyValveSetPressure )

	ON_EN_SETFOCUS( IDC_EDIT_SYSTEMPOWER, OnEnSetFocusSystemPower )
	ON_EN_SETFOCUS( IDC_EDIT_SYSTEMCOLLECTOR, OnEnSetFocusSystemCollector )

	ON_EN_KILLFOCUS( IDC_EDIT_SYSTEMPOWER, OnKillFocusSystemPower )
	ON_EN_KILLFOCUS( IDC_EDIT_SYSTEMCOLLECTOR, OnKillFocusSystemCollector )

	ON_MESSAGE( WM_USER_MODIFYSELECTEDSAFETYVALVE, OnModifySelectedSafetyValve )

	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDIT_SYSTEMPOWER, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDIT_SYSTEMCOLLECTOR, OnEditEnterChar )
END_MESSAGE_MAP()

void CDlgIndSelSafetyValve::DoDataExchange( CDataExchange* pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBO_SYSTEMTYPE, m_ComboSystemType );
	DDX_Control( pDX, IDC_COMBO_SYSTEMHEATGENERATORTYPE, m_ComboSystemHeatGeneratorType );
	DDX_Control( pDX, IDC_COMBO_NORM, m_ComboNorm );
	DDX_Control( pDX, IDC_COMBO_FAMILY, m_ComboSafetyValveFamily );
	DDX_Control( pDX, IDC_COMBO_CONNECT, m_ComboSafetyValveConnect );
	DDX_Control( pDX, IDC_COMBO_SETPRESSURE, m_ComboSafetyValveSetPressure );

	DDX_Control( pDX, IDC_EDIT_SYSTEMPOWER, m_clExtEditInstalledPower );
	DDX_Control( pDX, IDC_EDIT_SYSTEMCOLLECTOR, m_clExtEditInstalledCollector );

	DDX_Control( pDX, IDC_GROUPSYSTEM, m_clGroupSystem );
	DDX_Control( pDX, IDC_GROUPNORM, m_clGroupNorm );
	DDX_Control( pDX, IDC_GROUPINSTALLEDPOWER, m_clGroupInstalledPower );
	DDX_Control( pDX, IDC_GROUPSAFETYVALVE, m_clGroupSafetyValve );
	DDX_Control( pDX, IDC_BUTTONSUGGEST, m_clButtonSuggest );
}

BOOL CDlgIndSelSafetyValve::OnInitDialog() 
{
	CDlgIndSelBase::OnInitDialog();

	// 'System' group.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_SYSTEM );
	m_clGroupSystem.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_TYPE );
	GetDlgItem( IDC_STATICSYSTEMTYPE )->SetWindowText( str );

	// 'Norm' group.
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_NORM );
	m_clGroupNorm.SetWindowText( str );

	// 'Installed power' group.
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_POWER );
	GetDlgItem( IDC_STATICINSTALLEDPOWER )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_COLLECTOR );
	GetDlgItem( IDC_STATICINSTALLEDCOLLECTOR )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );

	// 'Safety valve' group.
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_HEATGENERATORTYPE );
	GetDlgItem( IDC_STATICHEATGENERATORTYPE )->SetWindowText( str);

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_FAMILY );
	GetDlgItem( IDC_STATICSAFETYVALVEFAMILY )->SetWindowText( str);

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_CONNECT );
	GetDlgItem( IDC_STATICSAFETYVALVECONNECT )->SetWindowText( str);

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSAFETYVALVE_SETPRESSURE );
	GetDlgItem( IDC_STATICSAFETYVALVESETPRESSURE )->SetWindowText( str);

	str.Empty();

	// Set proper style and add icons for groups.
	m_clGroupNorm.SetInOffice2007Mainframe( true );
	m_clGroupSystem.SetInOffice2007Mainframe( true );

	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupInstalledPower.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT ( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_clGroupInstalledPower.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_Q, true );
	}

	m_clGroupInstalledPower.SetInOffice2007Mainframe( true );
	
	pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupSafetyValve.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_SafetyValve );
	}

	pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT ( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_clGroupSafetyValve.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_SafetyValve, true );
	}

	m_clGroupSafetyValve.SetInOffice2007Mainframe( true );

	m_clExtEditInstalledPower.SetPhysicalType( _U_TH_POWER );
	m_clExtEditInstalledPower.SetEditSign( CNumString::ePositive );
	m_clExtEditInstalledPower.SetMinDblValue( 0.0 );

	m_clExtEditInstalledCollector.SetPhysicalType( _U_AREA );
	m_clExtEditInstalledCollector.SetEditSign( CNumString::ePositive );
	m_clExtEditInstalledCollector.SetMinDblValue( 0.0 );

	return TRUE;
}

void CDlgIndSelSafetyValve::OnCbnSelChangeSystemType()
{
	m_clIndSelSafetyValveParams.m_eSystemApplicationType = GetSytemType();
	::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_APPLICATIONTYPECHANGE, (WPARAM)m_clIndSelSafetyValveParams.m_eSystemApplicationType );
	
	_ChangeApplicationType();
}

void CDlgIndSelSafetyValve::OnCbnSelChangeSystemHeatGeneratorTypeID()
{
	m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID = m_ComboSystemHeatGeneratorType.GetCBCurSelIDPtr().ID;

	_FillComboNormID();
	_FillComboSafetyValveFamily();
	_FillComboSafetyValveConnect();
	_FillComboSafetyValveSetPressure();
	_VerifyFluidCharacteristics( true );
	ClearAll();
}

void CDlgIndSelSafetyValve::OnCbnSelChangeNormID()
{
	m_clIndSelSafetyValveParams.m_strNormID = GetNormID();
	
	_FillComboSafetyValveFamily();
	_FillComboSafetyValveConnect();
	_FillComboSafetyValveSetPressure();
	_VerifyFluidCharacteristics( true );
	ClearAll();
}

void CDlgIndSelSafetyValve::OnCbnSelChangeSafetyValveFamilyID() 
{
	m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID = m_ComboSafetyValveFamily.GetCBCurSelIDPtr().ID;

	_FillComboSafetyValveConnect();
	_FillComboSafetyValveSetPressure();
	_VerifyFluidCharacteristics( true );
	ClearAll();
}

void CDlgIndSelSafetyValve::OnCbnSelChangeSafetyValveConnectID() 
{
	m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID = m_ComboSafetyValveConnect.GetCBCurSelIDPtr().ID;

	_FillComboSafetyValveSetPressure();
	_VerifyFluidCharacteristics( true );
	ClearAll();
}

void CDlgIndSelSafetyValve::OnCbnSelChangeSafetyValveSetPressure() 
{
	m_clIndSelSafetyValveParams.m_dUserSetPressureChoice = GetSafetyValveSetPressure();
	_VerifyFluidCharacteristics( true );
	ClearAll();
}

LRESULT CDlgIndSelSafetyValve::OnModifySelectedSafetyValve( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelSafetyValve *pclSelectedSafetyValve = dynamic_cast<CDS_SSelSafetyValve *>( (CData *)lParam );
	ASSERT( NULL != pclSelectedSafetyValve );

	if( NULL == pclSelectedSafetyValve || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelSafetyValveParams.m_SelIDPtr = pclSelectedSafetyValve->GetIDPtr();
	m_bInitialised = false;

	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// HYS-1242: SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );
	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_SafetyValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelSafetyValveParams.m_eSystemApplicationType = pclSelectedSafetyValve->GetpSelectedInfos()->GetApplicationType();
	// Update also the 'CProductSelectionParameters' base.
	m_clIndSelSafetyValveParams.m_eApplicationType = m_clIndSelSafetyValveParams.m_eSystemApplicationType;
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelSafetyValveParams.m_eSystemApplicationType );
	m_clIndSelSafetyValveParams.m_pTADS->GetpTechParams()->SetProductSelectionApplicationType( m_clIndSelSafetyValveParams.m_eSystemApplicationType );

	// Transfer water characteristics.
	m_clIndSelSafetyValveParams.m_WC = *( pclSelectedSafetyValve->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pclSelectedSafetyValve->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Remark: The 'm_clIndSelSafetyValveParams.m_strPipeSeriesID' and 'm_clIndSelSafetyValveParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pclSelectedSafetyValve->GetPipeSeriesID(), pclSelectedSafetyValve->GetPipeID() );

	// Remark: Corresponding combo variables in 'm_clIndSelSafetyValveParams' are updated in each of this following methods.
	_FillComboSystemType( m_clIndSelSafetyValveParams.m_eSystemApplicationType );
	_FillComboSystemHeatGeneratorType( pclSelectedSafetyValve->GetSystemHeatGeneratorTypeID() );
	_FillComboNormID( pclSelectedSafetyValve->GetNormID() );
	_FillComboSafetyValveFamily( pclSelectedSafetyValve->GetSafetyValveFamilyID() );
	_FillComboSafetyValveConnect( pclSelectedSafetyValve->GetSafetyValveConnectID() );
	_FillComboSafetyValveSetPressure( pclSelectedSafetyValve->GetSetPressure() );

	_UpdateLayout();

	// Installed power.
	m_clIndSelSafetyValveParams.m_dInstalledPower = pclSelectedSafetyValve->GetInstalledPower();
	m_clExtEditInstalledPower.SetCurrentValSI( m_clIndSelSafetyValveParams.m_dInstalledPower );
	m_clExtEditInstalledPower.Update();

	// Collector if we are in solar.
	m_clIndSelSafetyValveParams.m_dInstalledCollector = 0.0;

	if( Solar == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		m_clIndSelSafetyValveParams.m_dInstalledCollector = pclSelectedSafetyValve->GetInstalledCollector();
		m_clExtEditInstalledCollector.SetCurrentValSI( m_clIndSelSafetyValveParams.m_dInstalledCollector );
		m_clExtEditInstalledCollector.Update();
	}

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );
	
	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );
	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

void CDlgIndSelSafetyValve::OnEnSetFocusSystemPower()
{
	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	m_clExtEditInstalledPower.SetSel( 0, -1 );
	m_dInstalledPowerSaved = m_clExtEditInstalledPower.GetCurrentValSI();
}

void CDlgIndSelSafetyValve::OnEnSetFocusSystemCollector()
{
	if( true == m_EditFieldHandlerBlocked )
	{
		return;
	}

	m_clExtEditInstalledCollector.SetSel( 0, -1 );
	m_dInstalledCollectorSaved = m_clExtEditInstalledCollector.GetCurrentValSI();
}

void CDlgIndSelSafetyValve::OnKillFocusSystemPower()
{
	// Do nothing if no change.
	if( true == m_EditFieldHandlerBlocked || m_dInstalledPowerSaved == m_clExtEditInstalledPower.GetCurrentValSI() )
	{
		return;
	}

	m_clIndSelSafetyValveParams.m_dInstalledPower = m_clExtEditInstalledPower.GetCurrentValSI();
}

void CDlgIndSelSafetyValve::OnKillFocusSystemCollector()
{
	// Do nothing if no change.
	if( true == m_EditFieldHandlerBlocked || m_dInstalledCollectorSaved == m_clExtEditInstalledCollector.GetCurrentValSI() )
	{
		return;
	}

	m_clIndSelSafetyValveParams.m_dInstalledCollector = m_clExtEditInstalledCollector.GetCurrentValSI();
}

void CDlgIndSelSafetyValve::OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDIT_SYSTEMPOWER == pNMHDR->idFrom || IDC_EDIT_SYSTEMCOLLECTOR == pNMHDR->idFrom  )
	{
		// To force the current edit to lost focus (And then allow it to save value input by user).
		GetDlgItem( IDC_BUTTONSUGGEST )->SetFocus();

		// Reset Focus on the modified Edit.
		CWnd::FromHandle( pNMHDR->hwndFrom )->SetFocus();

		PostWMCommandToControl( GetDlgItem( IDC_BUTTONSUGGEST ) );
	}
}

void CDlgIndSelSafetyValve::OnBnClickedSuggest() 
{
	ClearAll();
	
 	if( true == _VerifySubmittedValues() )
 	{
		m_clIndSelSafetyValveParams.m_SafetyValveList.PurgeAll();

		m_clIndSelSafetyValveParams.m_pTADB->GetSafetyValveList( 
				&m_clIndSelSafetyValveParams.m_SafetyValveList, 
				m_clIndSelSafetyValveParams.m_eSystemApplicationType,
				(LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID,
				(LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID,
				m_clIndSelSafetyValveParams.m_dUserSetPressureChoice,
				m_clIndSelSafetyValveParams.m_eFilterSelection );
		
		_SuggestIndividualSelection();
	}
}

void CDlgIndSelSafetyValve::OnBnClickedSelect() 
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	_SelectIndividualSelection();
}

LRESULT CDlgIndSelSafetyValve::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );

	if( false == TASApp.IsSafetyValveDisplayed() )
	{
		return 0;
	}
	
	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSafetyValveParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	// Remark: Corresponding combo variables in 'm_clIndSelBVParams' are updated in each of this following methods.
	_FillComboSystemType( pclIndSelParameter->GetSafetyValveSystemType() );
	_FillComboSystemHeatGeneratorType( pclIndSelParameter->GetSafetyValveSystemHeatGeneratorTypeID() );
	_FillComboNormID( pclIndSelParameter->GetSafetyValveNormID() );
	_FillComboSafetyValveFamily( pclIndSelParameter->GetSafetyValveFamilyID() );
	_FillComboSafetyValveConnect( pclIndSelParameter->GetSafetyValveConnectionID() );
	_FillComboSafetyValveSetPressure( pclIndSelParameter->GetSafetyValveSetPressure() );

	OnUnitChange();
	_UpdateLayout();

	m_bInitialised = true;

	if( NULL != pRViewSSelSafetyValve )
	{
		pRViewSSelSafetyValve->OnNewDocument( pclIndSelParameter );
	}
	
	return 0;
}

LRESULT CDlgIndSelSafetyValve::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnPipeChange( wParam, lParam );
	
	if( NULL != pRViewSSelSafetyValve )
	{
		pRViewSSelSafetyValve->Reset();
	}
	
	return 0;
}

LRESULT CDlgIndSelSafetyValve::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelSafetyValve )
	{
		pRViewSSelSafetyValve->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSafetyValve::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnUnitChange( wParam, lParam );

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), name );
	SetDlgItemText( IDC_STATICINSTALLEDPOWERUNIT, name );

	GetNameOf( pUnitDB->GetDefaultUnit( _U_AREA ), name );
	SetDlgItemText( IDC_STATICINSTALLEDCOLLECTORUNIT, name );

	// HYS-1085: Update combo if unit of pressur change
	if( CB_ERR != m_ComboSafetyValveSetPressure.GetCurSel() )
	{
		_FillComboSafetyValveSetPressure();
	}
	// Reset values in the edit fields.
	// Remarks: block all handlers (ON_EN_CHANGE, ON_EN_KILLFOCUS and ON_EN_SETFOCUS) to avoid interaction with operation.
	m_EditFieldHandlerBlocked = true;
	m_clExtEditInstalledPower.Update();
	m_clExtEditInstalledCollector.Update();
	m_EditFieldHandlerBlocked = false;

	if( NULL != pRViewSSelSafetyValve )
	{
		pRViewSSelSafetyValve->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSafetyValve::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) 
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnWaterChange( wParam, lParam );

	if( NULL != pRViewSSelSafetyValve )
	{
		pRViewSSelSafetyValve->Reset();
	}

	_VerifyFluidCharacteristics( false );

	return 0;
}

CRViewSSelSS *CDlgIndSelSafetyValve::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelSafetyValve;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelSafetyValve::_ChangeApplicationType()
{
	_UpdateLayout();

	if( Heating == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		_FillComboSystemHeatGeneratorType();
	}

	_FillComboNormID();
	_FillComboSafetyValveFamily();
	_FillComboSafetyValveConnect();
	_FillComboSafetyValveSetPressure();
	_VerifyFluidCharacteristics( true );
	ClearAll();
}

void CDlgIndSelSafetyValve::_UpdateLayout()
{
	if( ProjectType::Heating == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		GetDlgItem( IDC_STATICHEATGENERATORTYPE )->EnableWindow( TRUE );
		m_ComboSystemHeatGeneratorType.EnableWindow( TRUE );

		GetDlgItem( IDC_COMBO_NORM )->EnableWindow( FALSE );

		GetDlgItem( IDC_STATICINSTALLEDCOLLECTOR )->EnableWindow( FALSE );
		m_clExtEditInstalledCollector.EnableWindow( FALSE );
		GetDlgItem( IDC_STATICINSTALLEDCOLLECTORUNIT )->EnableWindow( FALSE );
	}
	else if( ProjectType::Cooling == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		GetDlgItem( IDC_STATICHEATGENERATORTYPE )->EnableWindow( FALSE );
		m_ComboSystemHeatGeneratorType.EnableWindow( FALSE );

		GetDlgItem( IDC_COMBO_NORM )->EnableWindow( TRUE );

		GetDlgItem( IDC_STATICINSTALLEDCOLLECTOR )->EnableWindow( FALSE );
		m_clExtEditInstalledCollector.EnableWindow( FALSE );
		GetDlgItem( IDC_STATICINSTALLEDCOLLECTORUNIT )->EnableWindow( FALSE );
	}
	else if( ProjectType::Solar == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		GetDlgItem( IDC_STATICHEATGENERATORTYPE )->EnableWindow( FALSE );
		m_ComboSystemHeatGeneratorType.EnableWindow( FALSE );

		GetDlgItem( IDC_COMBO_NORM )->EnableWindow( FALSE );

		GetDlgItem( IDC_STATICINSTALLEDCOLLECTOR )->EnableWindow( TRUE );
		m_clExtEditInstalledCollector.EnableWindow( TRUE );
		GetDlgItem( IDC_STATICINSTALLEDCOLLECTORUNIT )->EnableWindow( TRUE );
	}
}

bool CDlgIndSelSafetyValve::_SuggestIndividualSelection()
{
	if( NULL == pRViewSSelSafetyValve )
	{
		return false;
	}

	if( NULL != m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList )
	{
		delete m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList;
	}

	m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList = new CSelectSafetyValveList();
	
	if( NULL == m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList )
	{
		return false;
	}

	bool bBestFound = false;
	int iDevFound = m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList->SelectSafetyValve( &m_clIndSelSafetyValveParams, bBestFound );

	if( 0 != iDevFound )
	{
		VerifyModificationMode();
		pRViewSSelSafetyValve->Suggest( &m_clIndSelSafetyValveParams );
	}
	else
	{
		m_clIndSelSafetyValveParams.m_SafetyValveList.PurgeAll();

		// Keep a copy of the user choice.
		CParamsSaved clParamSaved( m_clIndSelSafetyValveParams.m_eSystemApplicationType, m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID, m_clIndSelSafetyValveParams.m_strNormID,
				m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID, m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID, m_clIndSelSafetyValveParams.m_dUserSetPressureChoice );

		// Release at least the connection and the set pressure.
		m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID = _T( "" );
		m_clIndSelSafetyValveParams.m_dUserSetPressureChoice = 0.0;

		m_clIndSelSafetyValveParams.m_pTADB->GetSafetyValveList( &m_clIndSelSafetyValveParams.m_SafetyValveList, m_clIndSelSafetyValveParams.m_eSystemApplicationType,
				(LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID, (LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID, m_clIndSelSafetyValveParams.m_dUserSetPressureChoice, 
				m_clIndSelSafetyValveParams.m_eFilterSelection );

		bool bMustRestoreUserChoice = true;
		iDevFound = m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList->SelectSafetyValve( &m_clIndSelSafetyValveParams, bBestFound );
		
		if( iDevFound > 0 )
		{
			// Transfer from 'CSelectedProductList' to a 'CRankEx' object.
			CRankEx rList;
			m_clIndSelSafetyValveParams.m_pclSelectSafetyValveList->FillResultsInRankEx( &rList );

			// Launch the no device found dialog box if something is found.
			// Remark: Combos in this dialog will be filled thanks to the new alternative safety valves found and saved in the 'm_clIndSelSafetyValveParams.m_SafetyValveList' variable.
			CDlgNoDevSafetyValveFoundParams clNoDevSafetyValveParams = CDlgNoDevSafetyValveFoundParams( &rList, m_clIndSelSafetyValveParams.m_eSystemApplicationType,
					m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID, m_clIndSelSafetyValveParams.m_strNormID, m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID,
					m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID, m_clIndSelSafetyValveParams.m_dUserSetPressureChoice );

			CDlgNoDevSafetyValveFound dlg( &m_clIndSelSafetyValveParams, &clNoDevSafetyValveParams );

			if( IDOK == dlg.DoModal() )
			{
				clNoDevSafetyValveParams = *dlg.GetNoDevSafetyValveFoundParams();
				bMustRestoreUserChoice = false;

				_LaunchSuggestion( clNoDevSafetyValveParams.m_eSystemType, clNoDevSafetyValveParams.m_strSystemHeatGeneratorTypeID, clNoDevSafetyValveParams.m_strSafetyValveFamilyID, 
						clNoDevSafetyValveParams.m_strSafetyValveConnectID, clNoDevSafetyValveParams.m_dSafetyValveSetPressure );
			}
		}
		else
		{
			// No device found message.
			TASApp.AfxLocalizeMessageBox( IDS_DLGINDSELSAFETYVALVE_NODEVFOUND );
		}

		if( true == bMustRestoreUserChoice )
		{
			m_clIndSelSafetyValveParams.m_eSystemApplicationType = clParamSaved.m_eSystemType;
			m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID = clParamSaved.m_strSystemHeatGeneratorTypeID;
			m_clIndSelSafetyValveParams.m_strNormID = clParamSaved.m_strSystemHeatGeneratorTypeID;
			m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID = clParamSaved.m_strSafetyValveFamilyID;
			m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID = clParamSaved.m_strSafetyValveConnectID;
			m_clIndSelSafetyValveParams.m_dUserSetPressureChoice = clParamSaved.m_dUserSetPressureChoice;
		}
	}

	return true;
}

bool CDlgIndSelSafetyValve::_SelectIndividualSelection()
{
	try
	{
		if( NULL == m_clIndSelSafetyValveParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelSafetyValveParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelSafetyValveParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelSafetyValveParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelSafetyValve )
		{
			HYSELECT_THROW( _T("Internal error: The safety valve right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The safety valve right view is not visible.") );
		}

		// Verify pointer validity; user can delete object after entering in edition.
		if( _T('\0') != *m_clIndSelSafetyValveParams.m_SelIDPtr.ID )
		{
			m_clIndSelSafetyValveParams.m_SelIDPtr = m_clIndSelSafetyValveParams.m_pTADS->Get( m_clIndSelSafetyValveParams.m_SelIDPtr.ID );
		}
	
		// Create selected object and initialize it.
		CDS_SSelSafetyValve *pclSelSafetyValve = dynamic_cast<CDS_SSelSafetyValve *>( m_clIndSelSafetyValveParams.m_SelIDPtr.MP );

		if( NULL == pclSelSafetyValve )
		{
			IDPTR IDPtr;
			m_clIndSelSafetyValveParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSafetyValve ) );
			pclSelSafetyValve = (CDS_SSelSafetyValve *)( IDPtr.MP );
		}
	
		if( m_clIndSelSafetyValveParams.m_dInstalledPower <= 0.0 ) 
		{
			HYSELECT_THROW( _T("Internal error: Flow '%f' can't be lower or equal to 0."), m_clIndSelSafetyValveParams.m_dFlow );
		}
	
		pclSelSafetyValve->SetSystemHeatGeneratorTypeID( m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID );
		pclSelSafetyValve->SetNormID( m_clIndSelSafetyValveParams.m_strNormID );
		pclSelSafetyValve->SetInstalledPower( m_clIndSelSafetyValveParams.m_dInstalledPower );
		pclSelSafetyValve->SetInstalledCollector( ( Solar == m_clIndSelSafetyValveParams.m_eSystemApplicationType ) ? m_clIndSelSafetyValveParams.m_dInstalledCollector : 0.0 );
		pclSelSafetyValve->SetSafetyValveFamilyID( m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID );
		pclSelSafetyValve->SetSafetyValveConnectID( m_clIndSelSafetyValveParams.m_strComboConnectID );
		pclSelSafetyValve->SetSetPressure( GetSafetyValveSetPressure() );
	
		*pclSelSafetyValve->GetpSelectedInfos()->GetpWCData() = m_clIndSelSafetyValveParams.m_WC;
	
		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pclSelSafetyValve->GetpSelectedInfos()->SetDT( m_clIndSelSafetyValveParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pclSelSafetyValve->GetpSelectedInfos()->SetApplicationType( m_clIndSelSafetyValveParams.m_eSystemApplicationType );

		pRViewSSelSafetyValve->FillInSelected( pclSelSafetyValve );

		CDlgConfSel dlg( &m_clIndSelSafetyValveParams );
		IDPTR IDPtr = pclSelSafetyValve->GetIDPtr();
		dlg.Display( pclSelSafetyValve );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelSafetyValveParams.m_pTADS->Get( _T("SAFETYVALVE_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'SAFETYVALVE_TAB' table from the datastruct.") );
			}
		
			if( _T('\0') == *m_clIndSelSafetyValveParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelSafetyValveParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelSafetyValve ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelSafetyValveParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelSafetyValveParams.m_SelIDPtr.ID )
			{
				m_clIndSelSafetyValveParams.m_pTADS->DeleteObject( IDPtr );
			}
		}

		if( NULL != dlg.GetSafeHwnd() )
		{
			dlg.DestroyWindow();
		}

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelSafetyValve::_SelectIndividualSelection'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelSafetyValve::_FillComboNormID( CString strNormID )
{
	if( NULL == m_clIndSelSafetyValveParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx rkList;
	DWORD_PTR dwSelItem = 0;
	CTable *pTab = (CTable *)( m_clIndSelSafetyValveParams.m_pTADB->Get( _T("PRESSMAINTNORM_TAB") ).MP );

	struct _StringIDData
	{
		int m_i;
		_string m_str;
		CDB_StringID *m_pStrID;
	};
	std::map<CString, _StringIDData> mapNorms;

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		_StringIDData rStringIDData;
		rStringIDData.m_pStrID = dynamic_cast<CDB_StringID *>( idptr.MP );

		if( NULL == rStringIDData.m_pStrID )
		{
			ASSERT_CONTINUE;
		}

		rStringIDData.m_i = 0;

		if( false == rStringIDData.m_pStrID->GetIDstrAs<int>( 0, rStringIDData.m_i ) )
		{
			ASSERT_CONTINUE;
		}
		
		rStringIDData.m_str = rStringIDData.m_pStrID->GetString();
		mapNorms[rStringIDData.m_pStrID->GetIDPtr().ID] = rStringIDData;
	}

	if( Heating == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		if( 0 != mapNorms.count( _T("PM_NORM_EN12828") ) )
		{
			rkList.Add( mapNorms[_T("PM_NORM_EN12828")].m_str, mapNorms[_T("PM_NORM_EN12828")].m_i, (LPARAM)mapNorms[_T("PM_NORM_EN12828")].m_pStrID->GetIDPtr().MP );
		}
	}
	else if( Cooling == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		if( 0 != mapNorms.count( _T("PM_NORM_EN12828") ) )
		{
			rkList.Add( mapNorms[_T("PM_NORM_EN12828")].m_str, mapNorms[_T("PM_NORM_EN12828")].m_i, (LPARAM)mapNorms[_T("PM_NORM_EN12828")].m_pStrID->GetIDPtr().MP );
		}

		if( 0 != mapNorms.count( _T("PM_NORM_NONE") ) )
		{
			rkList.Add( mapNorms[_T("PM_NORM_NONE")].m_str, mapNorms[_T("PM_NORM_NONE")].m_i, (LPARAM)mapNorms[_T("PM_NORM_NONE")].m_pStrID->GetIDPtr().MP );
		}
	}
	else if( Solar == m_clIndSelSafetyValveParams.m_eSystemApplicationType )
	{
		if( 0 != mapNorms.count( _T("PM_NORM_EN12828") ) && 0 != mapNorms.count( _T("PM_NORM_DM11275") ) )
		{
			_string str = mapNorms[_T("PM_NORM_EN12828")].m_str + _T(" & ") + mapNorms[_T("PM_NORM_DM11275")].m_str;
			rkList.Add( str, mapNorms[_T("PM_NORM_EN12828")].m_i, (LPARAM)mapNorms[_T("PM_NORM_EN12828")].m_pStrID->GetIDPtr().MP );
		}
	}

	rkList.Transfer( &m_ComboNorm );
	int iSel = m_ComboNorm.FindItemDataPtr( dwSelItem );

	if( -1 == iSel )
	{
		iSel = 0;
	}

	m_ComboNorm.SetCurSel( iSel );

	if( m_ComboNorm.GetCount() <= 1 )
	{
		m_ComboNorm.EnableWindow( FALSE );
	}
	else
	{
		m_ComboNorm.EnableWindow( TRUE );
	}

	m_clIndSelSafetyValveParams.m_strNormID = m_ComboNorm.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSafetyValve::_FillComboSystemType( ProjectType eApplicationType )
{
	m_ComboSystemType.ResetContent();

	int iSel = 0;
	m_ComboSystemType.InsertString( 0, TASApp.LoadLocalizedString( IDS_RBN_APPLITYPE_HEATING ) );
	m_ComboSystemType.SetItemData( 0, ProjectType::Heating );

	m_ComboSystemType.InsertString( 1, TASApp.LoadLocalizedString( IDS_RBN_APPLITYPE_COOLING ) );
	m_ComboSystemType.SetItemData( 1, ProjectType::Cooling );

	if( ProjectType::Cooling == eApplicationType )
	{
		iSel = 1;
	}

	m_ComboSystemType.InsertString( 2, TASApp.LoadLocalizedString( IDS_RBN_APPLITYPE_SOLAR ) );
	m_ComboSystemType.SetItemData( 2, ProjectType::Solar );

	if( ProjectType::Solar == eApplicationType )
	{
		iSel = 2;
	}

	m_ComboSystemType.SetCurSel( iSel );
	m_clIndSelSafetyValveParams.m_eSystemApplicationType = eApplicationType;
}

void CDlgIndSelSafetyValve::_FillComboSystemHeatGeneratorType( CString strSystemHeatGeneratorTypeID )
{
	if( NULL == m_clIndSelSafetyValveParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx rkList;
	_string str;
	DWORD_PTR dwSelItem = 0;
	CTable *pTab = (CTable *)( m_clIndSelSafetyValveParams.m_pTADB->Get( _T("SVHEATGENTYPE_TAB") ).MP );

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( idptr.MP );

		if( NULL == pStrID )
		{
			ASSERT_CONTINUE;
		}

		str = pStrID->GetString();
		int i;

		if( false == pStrID->GetIDstrAs<int>( 0, i ) )
		{
			ASSERT_CONTINUE;
		}

		if( strSystemHeatGeneratorTypeID == idptr.ID )
		{
			dwSelItem = (LPARAM)idptr.MP;
		}

		rkList.Add( str, i, (LPARAM)idptr.MP );
	}

	rkList.Transfer( &m_ComboSystemHeatGeneratorType );
	int iSel = m_ComboSystemHeatGeneratorType.FindItemDataPtr( dwSelItem );

	if( -1 == iSel )
	{
		iSel = 0;
	}

	m_ComboSystemHeatGeneratorType.SetCurSel( iSel );
	m_clIndSelSafetyValveParams.m_strSystemHeatGeneratorTypeID = m_ComboSystemHeatGeneratorType.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSafetyValve::_FillComboSafetyValveFamily( CString strSafetyValveFamilyID )
{
	if( NULL == m_clIndSelSafetyValveParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx SafetyValveTypeList;
	m_clIndSelSafetyValveParams.m_pTADB->GetSafetyValveFamilyList( &SafetyValveTypeList, m_clIndSelSafetyValveParams.m_eSystemApplicationType );

	m_ComboSafetyValveFamily.FillInCombo( &SafetyValveTypeList, strSafetyValveFamilyID, IDS_COMBOTEXT_ALL_FAMILIES );
	m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID = m_ComboSafetyValveFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSafetyValve::_FillComboSafetyValveConnect( CString strSafetyValveConnectID )
{
	if( NULL == m_clIndSelSafetyValveParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx SafetyValveConnectList;

	m_clIndSelSafetyValveParams.m_pTADB->GetSafetyValveConnList( &SafetyValveConnectList, m_clIndSelSafetyValveParams.m_eSystemApplicationType,
			(LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID );

	m_ComboSafetyValveConnect.FillInCombo( &SafetyValveConnectList, strSafetyValveConnectID, IDS_COMBOTEXT_ALL_CONNECTIONS );
	m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID = m_ComboSafetyValveConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSafetyValve::_FillComboSafetyValveSetPressure( double dSetPressure )
{
	if( NULL == m_clIndSelSafetyValveParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx SafetyValveSetPressureList;

	m_clIndSelSafetyValveParams.m_pTADB->GetSafetyValveSetPressureList( &SafetyValveSetPressureList, m_clIndSelSafetyValveParams.m_eSystemApplicationType,
			(LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID, (LPCTSTR)m_clIndSelSafetyValveParams.m_strSafetyValveConnectionID );

	m_ComboSafetyValveSetPressure.ResetContent();

	std::wstring str;
	LPARAM lpParam = 0;
	int iSelPos = 0;
	int iCount = 0;

	for( bool bContinue = SafetyValveSetPressureList.GetFirst( str, lpParam ); true == bContinue; bContinue = SafetyValveSetPressureList.GetNext( str, lpParam ) )
	{
		int iItem = m_ComboSafetyValveSetPressure.AddString( str.c_str() );
		m_ComboSafetyValveSetPressure.SetItemData( iItem, lpParam );

		if( dSetPressure == (double)lpParam )
		{
			iSelPos = iCount;
		}

		iCount++;
	}

	m_ComboSafetyValveSetPressure.SetCurSel( iSelPos );

	if( m_ComboSafetyValveSetPressure.GetCount() <= 1 )
	{
		m_ComboSafetyValveSetPressure.EnableWindow( FALSE );
	}
	else
	{
		m_ComboSafetyValveSetPressure.EnableWindow( TRUE );
	}

	m_clIndSelSafetyValveParams.m_dUserSetPressureChoice = GetSafetyValveSetPressure();
}

void CDlgIndSelSafetyValve::_LaunchSuggestion( ProjectType eSystemType, CString strSystemHeatGeneratorTypeID, CString strSafetyValveTypeID, 
		CString strSafetyValveConnectID, double dSafetyValveSetPressure )
{
	_FillComboNormID( m_clIndSelSafetyValveParams.m_strNormID );
	_FillComboSystemType( eSystemType );
	_FillComboSystemHeatGeneratorType( strSystemHeatGeneratorTypeID );
	_FillComboSafetyValveFamily( strSafetyValveTypeID );
	_FillComboSafetyValveConnect( strSafetyValveConnectID );
	_FillComboSafetyValveSetPressure( dSafetyValveSetPressure );

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelSafetyValve::_VerifySubmittedValues()
{
	bool bValid = true;

	if( m_clIndSelSafetyValveParams.m_dInstalledPower <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_POWER );
		m_clExtEditInstalledPower.SetFocus();
		bValid = false;
	}

	return bValid;
}

void CDlgIndSelSafetyValve::_VerifyFluidCharacteristics( bool bShowErrorMsg )
{
	bool bEnable = true;
	CString strMsg = _T("");
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg );

	if( true == bEnable )
	{
		m_clButtonSuggest.EnableWindow( TRUE );
		m_clButtonSuggest.ModifyStyle( BS_OWNERDRAW, 0 );
		m_clButtonSuggest.SetTextColor( _BLACK );
		m_clButtonSuggest.ResetDrawBorder();

		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.DelTool( &m_clButtonSuggest );
		}
	}
	else
	{
		m_clButtonSuggest.EnableWindow( FALSE );
		m_clButtonSuggest.ModifyStyle( 0, BS_OWNERDRAW );
		m_clButtonSuggest.SetTextColor( _RED );
		m_clButtonSuggest.SetBackColor( RGB( 204, 204, 204 ) );
		m_clButtonSuggest.SetDrawBorder( true, _RED );

		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.AddToolWindow( &m_clButtonSuggest, strMsg );
		}

		if( true == bShowErrorMsg )
		{
			AfxMessageBox( strMsg );
		}
	}
}

void CDlgIndSelSafetyValve::_VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	CString strAdditiveFamilyID = _T("");
	double dTemp = 0.0;
	double dPCWeight = 0.0;
	
	if( NULL != pclDlgWaterChar )
	{
		strAdditiveFamilyID = pclDlgWaterChar->GetAdditiveFamilyID();
		dTemp = pclDlgWaterChar->GetTemp();
		dPCWeight = pclDlgWaterChar->GetPCWeight();
	}
	else
	{
		strAdditiveFamilyID = m_clIndSelSafetyValveParams.m_WC.GetAdditFamID();
		dTemp = m_clIndSelSafetyValveParams.m_WC.GetTemp();
		dPCWeight = m_clIndSelSafetyValveParams.m_WC.GetPcWeight();
	}

	if( Heating == m_clIndSelSafetyValveParams.m_eApplicationType )
	{
		if( 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT") ) 
				&& 0 != StringCompare( strAdditiveFamilyID, _T("GLYCOL_ADDIT") ) 
				&& 0 != StringCompare( strAdditiveFamilyID, _T("ALCOHOL_ADDIT") ) )
		{
			// For heating we accept only water without additive.
			strMsg = TASApp.LoadLocalizedString( IDS_DLGINDSELSAFETYVALVE_HEATING_ONLYWATERORGLYCOLALCOHOL );
			bEnable = false;
		}
		else if( dTemp > 120.0 )
		{
			bEnable = false;
			double dTempCU = CDimValue::SItoCU( _U_TEMPERATURE, 120.0 );
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTempCU, true );
			FormatString( strMsg, IDS_DLGINDSELSAFETYVALVE_HEATING_MAXTEMP, strTemp );
		}
		else if( dTemp < -10.0 )
		{
			bEnable = false;
			double dTempCU = CDimValue::SItoCU( _U_TEMPERATURE, -10.0 );
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTempCU, true );
			FormatString( strMsg, IDS_DLGINDSELSAFETYVALVE_HEATING_MINTEMP, strTemp );
		}
		else
		{
			if( ( _T("") == m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID 
					|| _T("SFTYVFAM_H") == m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID )
				&& dPCWeight> 30.0 )
			{
				CTable *pclSaftyValveFamilyTable =(CTable *)( m_clIndSelSafetyValveParams.m_pTADB->Get( _T("SFTYVFAMILY_TAB") ).MP );

				if( NULL == pclSaftyValveFamilyTable )
				{
					ASSERT_RETURN;
				}
				
				CDB_StringID *pclFamilyNameStrID = (CDB_StringID *)( pclSaftyValveFamilyTable->Get( _T("SFTYVFAM_H") ).MP );

				if( NULL == pclFamilyNameStrID )
				{
					ASSERT_RETURN;
				}

				bEnable = false;

				// For %1 family the addition of antifreeze is limited to %2.
				CString strFamilyName = pclFamilyNameStrID->GetString();
				CString strLimit = WriteCUDouble( _U_PERCENT, 30.0, true );
				FormatString( strMsg, IDS_DLGINDSELSAFETYVALVE_HEATING_ANTIFREEZELIMIT, strFamilyName, strLimit );
			}
			else if( _T("SFTYVFAM_DGH") == m_clIndSelSafetyValveParams.m_strSafetyValveFamilyID
					&& dPCWeight > 50.0 )
			{
				CTable *pclSaftyValveFamilyTable =(CTable *)( m_clIndSelSafetyValveParams.m_pTADB->Get( _T("SFTYVFAMILY_TAB") ).MP );

				if( NULL == pclSaftyValveFamilyTable )
				{
					ASSERT_RETURN;
				}
				
				CDB_StringID *pclFamilyNameStrID = (CDB_StringID *)( pclSaftyValveFamilyTable->Get( _T("SFTYVFAM_DGH") ).MP );

				if( NULL == pclFamilyNameStrID )
				{
					ASSERT_RETURN;
				}

				bEnable = false;

				// For %1 family the addition of antifreeze is limited to %2.
				CString strFamilyName = pclFamilyNameStrID->GetString();
				CString strLimit = WriteCUDouble( _U_PERCENT, 50.0, true );
				FormatString( strMsg, IDS_DLGINDSELSAFETYVALVE_HEATING_ANTIFREEZELIMIT, strFamilyName, strLimit );
			}
		}
	}
	else if( Cooling == m_clIndSelSafetyValveParams.m_eApplicationType )
	{
		if( 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT") )
 				&& 0 != StringCompare( strAdditiveFamilyID, _T("GLYCOL_ADDIT") ) 
				&& 0 != StringCompare( strAdditiveFamilyID, _T("ALCOHOL_ADDIT") ) )
		{
			// For cooling we accept only water or water + glycol.
			// HYS-1111: 2019-06-05: Christian Thesing and Norbert Ramser allow alcohol for PAG products.
			bEnable = false;
			strMsg = TASApp.LoadLocalizedString( IDS_DLGINDSELSAFETYVALVE_COOLING_ONLYWATERORGLYCOLALCOHOL );
		}
		else if( dTemp < -50.0 )
		{
			bEnable = false;
			double dTempCU = CDimValue::SItoCU( _U_TEMPERATURE, -50.0 );
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTempCU, true );
			FormatString( strMsg, IDS_DLGINDSELSAFETYVALVE_COOLING_LIMITTEMP, strTemp );
		}
	}
	else if( Solar == m_clIndSelSafetyValveParams.m_eApplicationType )
	{
		if( 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT") )
 				&& 0 != StringCompare( strAdditiveFamilyID, _T("GLYCOL_ADDIT") )
				&& 0 != StringCompare( strAdditiveFamilyID, _T("ALCOHOL_ADDIT") ) )
		{
			// For solar we accept only water or water + glycol.
			// HYS-1111: 2019-06-05: Christian Thesing and Norbert Ramser allow alcohol for PAG products.
			bEnable = false;
			strMsg = TASApp.LoadLocalizedString( IDS_DLGINDSELSAFETYVALVE_SOLAR_ONLYWATERORGLYCOLALCOHOL );
		}
		else if( dTemp > 160.0 )
		{
			bEnable = false;
			double dTempCU = CDimValue::SItoCU( _U_TEMPERATURE, 160.0 );
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTempCU, true );
			FormatString( strMsg, IDS_DLGINDSELSAFETYVALVE_SOLAR_MAXTEMP, strTemp );
		}
		else if( dTemp < -10.0 )
		{
			bEnable = false;
			double dTempCU = CDimValue::SItoCU( _U_TEMPERATURE, -10.0 );
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTempCU, true );
			FormatString( strMsg, IDS_DLGINDSELSAFETYVALVE_SOLAR_MINTEMP, strTemp );
		}
	}
}
