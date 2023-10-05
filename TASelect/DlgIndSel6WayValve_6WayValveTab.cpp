#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "Select.h"
#include "DlgLeftTabSelManager.h"
#include "DlgIndSelBase.h"
#include "DlgIndSel6WayValve_6WayValveTab.h"

#include "RViewSSelSS.h"
#include "RViewSSelCtrl.h"
#include "RViewSSel6WayValve.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSel6WayValve_6WayValveTab::CDlgIndSel6WayValve_6WayValveTab( CIndSel6WayValveParams &clIndSel6WayValveParams, CWnd *pParent )
	: CDlgIndSelCtrlBase( clIndSel6WayValveParams, CDlgIndSel6WayValve_6WayValveTab::IDD, pParent )
{
	m_pclIndSel6WayValveParams = &clIndSel6WayValveParams;
	m_pclIndSel6WayValveParams->m_eCV2W3W = CDB_ControlProperties::CV2W3W::CV6W;
	m_pNotificationHandler = NULL;
	m_bIsDTCoolingEdited = false;
	m_bIsDTHeatingEdited = false;
	m_dPreviousCoolingDT = 0.0;
	m_dPreviousHeatingDT = 0.0;
}

CDlgIndSel6WayValve_6WayValveTab::~CDlgIndSel6WayValve_6WayValveTab()
{
	m_pNotificationHandler = NULL;
	m_pclIndSel6WayValveParams = NULL;
}

void CDlgIndSel6WayValve_6WayValveTab::SaveSelectionParameters()
{
	CDlgIndSelCtrlBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_pclIndSel6WayValveParams->m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->Set6WValveRadioFlowPowerDT( (int)m_pclIndSel6WayValveParams->m_eFlowOrPowerDTMode );
	pclIndSelParameter->Set6WValveConnectID( m_pclIndSel6WayValveParams->m_strComboConnectID );
	pclIndSelParameter->Set6WValveVersionID( m_pclIndSel6WayValveParams->m_strComboVersionID );
}

void CDlgIndSel6WayValve_6WayValveTab::SetNotificationHandler( I6WayValveTab_NotificationHandler *pINotificationHandler )
{
	if( NULL == pINotificationHandler )
	{
		return;
	}

	m_pNotificationHandler = pINotificationHandler;
}

void CDlgIndSel6WayValve_6WayValveTab::ResetNotificationHandler( void )
{
	m_pNotificationHandler = NULL;
}

void CDlgIndSel6WayValve_6WayValveTab::FillComboConnection( CString strConnectID )
{
	CRankEx ConnList;

	// HYS-1877: Consider Set selection
	m_pclIndSel6WayValveParams->m_pTADB->Get6WayValveConnectionList( &ConnList, m_pclIndSel6WayValveParams->m_eFilterSelection, 0, INT_MAX, m_pclIndSel6WayValveParams->m_bOnlyForSet );

	m_ComboConnect.FillInCombo( &ConnList, strConnectID, m_pclIndSel6WayValveParams->GetComboConnectAllID() );
	m_pclIndSel6WayValveParams->m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSel6WayValve_6WayValveTab::FillComboVersion( CString strVersionID )
{
	CRankEx VerList;

	m_pclIndSel6WayValveParams->m_pTADB->Get6WayValveVersList( &VerList, (LPCTSTR)m_pclIndSel6WayValveParams->m_strComboConnectID, 
			m_pclIndSel6WayValveParams->m_eFilterSelection );

	m_ComboVersion.FillInCombo( &VerList, strVersionID, m_pclIndSel6WayValveParams->GetComboVersionAllID() );
	m_pclIndSel6WayValveParams->m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSel6WayValve_6WayValveTab::Set6WayValveTabParameters( CDS_SSel6WayValve *pclCurrent6WayValve )
{
	if( NULL == pclCurrent6WayValve )
	{
		return;
	}

	// Remark: Corresponding combo variables in 'm_clIndSel6WParams' are updated in each of this following methods.
	FillComboConnection( pclCurrent6WayValve->GetConnectID() );
	FillComboVersion( pclCurrent6WayValve->GetVersionID() );

	SelectCurrentComboPipes( pclCurrent6WayValve->GetPipeSeriesID(), pclCurrent6WayValve->GetPipeID() );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pclCurrent6WayValve->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.	
	UpdateData( FALSE );
	Update6WayFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_pclIndSel6WayValveParams->m_eFlowOrPowerDTMode )
	{
		m_pclIndSel6WayValveParams->m_dHeatingFlow = pclCurrent6WayValve->GetHeatingFlow();
		
		if( m_pclIndSel6WayValveParams->m_dHeatingFlow > 0 )
		{
			m_EditFlowHeating.SetWindowText( WriteCUDouble( _U_FLOW, m_pclIndSel6WayValveParams->m_dHeatingFlow ) );
		}
		else
		{
			m_EditFlowHeating.SetWindowText( _T("") );
		}

		m_pclIndSel6WayValveParams->m_dCoolingFlow = pclCurrent6WayValve->GetCoolingFlow();
		
		if( m_pclIndSel6WayValveParams->m_dCoolingFlow > 0 )
		{
			m_EditFlowCooling.SetWindowText( WriteCUDouble( _U_FLOW, m_pclIndSel6WayValveParams->m_dCoolingFlow ) );
		}
		else
		{
			m_EditFlowCooling.SetWindowText( _T("") );
		}
	}
	else
	{
		m_pclIndSel6WayValveParams->m_dHeatingPower = pclCurrent6WayValve->GetHeatingPower();
		
		if( m_pclIndSel6WayValveParams->m_dHeatingPower > 0 )
		{
			m_EditPowerHeating.SetWindowText( WriteCUDouble( _U_TH_POWER, m_pclIndSel6WayValveParams->m_dHeatingPower ) );
		}
		else
		{
			m_EditPowerHeating.SetWindowText( _T("") );
		}

		m_pclIndSel6WayValveParams->m_dHeatingDT = pclCurrent6WayValve->GetHeatingDT();
		
		if( m_pclIndSel6WayValveParams->m_dHeatingDT > 0 )
		{
			m_EditDTHeating.SetWindowText( WriteCUDouble( _U_DIFFTEMP, m_pclIndSel6WayValveParams->m_dHeatingDT ) );
		}
		else
		{
			m_EditDTHeating.SetWindowText( _T("") );
		}
		
		_UpdateFlowHeating();

		m_pclIndSel6WayValveParams->m_dCoolingPower = pclCurrent6WayValve->GetCoolingPower();
		
		if( m_pclIndSel6WayValveParams->m_dCoolingPower > 0 )
		{
			m_EditPowerCooling.SetWindowText( WriteCUDouble( _U_TH_POWER, m_pclIndSel6WayValveParams->m_dCoolingPower ) );
		}
		else
		{
			m_EditPowerCooling.SetWindowText( _T("") );
		}

		m_pclIndSel6WayValveParams->m_dCoolingDT = pclCurrent6WayValve->GetCoolingDT();
		
		if( m_pclIndSel6WayValveParams->m_dCoolingDT > 0 )
		{
			m_EditDTCooling.SetWindowText( WriteCUDouble( _U_DIFFTEMP, m_pclIndSel6WayValveParams->m_dCoolingDT ) );
		}
		else
		{
			m_EditDTCooling.SetWindowText( _T("") );
		}
		
		_UpdateFlowCooling();
	}
	
	m_bInitialised = true;
}

void CDlgIndSel6WayValve_6WayValveTab::Update6WayValveTabDTField()
{
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();

	if( false == m_bIsDTCoolingEdited )
	{
		m_EditDTCooling.SetCurrentValSI( pTechP->GetDefaultISChangeOverDT( CoolingSide ) );
		m_pclIndSel6WayValveParams->m_dCoolingDT = pTechP->GetDefaultISChangeOverDT( CoolingSide );
		m_EditDTCooling.Update();
	}
	
	if( false == m_bIsDTHeatingEdited )
	{
		m_EditDTHeating.SetCurrentValSI( pTechP->GetDefaultISChangeOverDT( HeatingSide ) );
		m_pclIndSel6WayValveParams->m_dHeatingDT = pTechP->GetDefaultISChangeOverDT( HeatingSide );
		m_EditDTHeating.Update();
	}
}

bool CDlgIndSel6WayValve_6WayValveTab::LeftTabKillFocus( bool fNext )
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

void CDlgIndSel6WayValve_6WayValveTab::OnRViewSSelLostFocusWithTabKey( bool bShiftPressed )
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
			SetFocus();
			// Set the focus on the last available edit control of the lef tab.
			SetFocusOnControl( SetFocusWhere::Last );
		}
	}
	else
	{
		CDlgIndSelCtrlBase::OnRViewSSelLostFocusWithTabKey( bShiftPressed );
	}
}

void CDlgIndSel6WayValve_6WayValveTab::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelCtrlBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );

	DDX_Control( pDX, IDC_EDITFLOW_H, m_EditFlowHeating );
	DDX_Control( pDX, IDC_EDITFLOW_C, m_EditFlowCooling );
	DDX_Control( pDX, IDC_EDITPOWER_H, m_EditPowerHeating );
	DDX_Control( pDX, IDC_EDITPOWER_C, m_EditPowerCooling );
	DDX_Control( pDX, IDC_EDITDT_H, m_EditDTHeating );
	DDX_Control( pDX, IDC_EDITDT_C, m_EditDTCooling );
	DDX_Control( pDX, IDC_GPHEATING, m_clGroupHeating );
	DDX_Control( pDX, IDC_GPCOOLING, m_clGroupCooling );

	// 'CDlgIndSelBase' variables.
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSel6WayValve_6WayValveTab::OnInitDialog()
{
	CDlgIndSelCtrlBase::OnInitDialog();

	// Set proper style and add icons for groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupHeating.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Heating );
		m_clGroupCooling.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Cooling );
		( (CXGroupBox *)GetDlgItem( IDC_GROUPVALVE ) )->SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_clGroupHeating.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_Heating, true );
		m_clGroupCooling.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_Cooling, true );
		( (CXGroupBox *)GetDlgItem( IDC_GROUPVALVE ) )->SetImageList( pclImgListGroupBox, CRCImageManager::ILGBG_Valve );
	}

	// Set proper style and add icons for groups.
	m_clGroupHeating.SetInOffice2007Mainframe( true );
	m_clGroupCooling.SetInOffice2007Mainframe( true );

	CString str = CteEMPTY_STRING;
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_GPCOOLING );
	m_clGroupCooling.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_GPHEATING );
	m_clGroupHeating.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_GPVALVE );
	( (CXGroupBox*)GetDlgItem( IDC_GROUPVALVE ) )->SetWindowText( str );

	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICFLOW );
	GetDlgItem( IDC_STATICFLOW_H )->SetWindowText( str );
	GetDlgItem( IDC_STATICFLOW_C )->SetWindowText( str );
		
	if( NULL != GetpRadioFlow() )
	{
		GetpRadioFlow()->SetWindowText( str );
	}
		
	m_EditFlowHeating.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_EditFlowHeating.SetPhysicalType( _U_FLOW );
	m_EditFlowCooling.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_EditFlowCooling.SetPhysicalType( _U_FLOW );

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICPOWER );
	GetDlgItem( IDC_STATICPOWER_H )->SetWindowText( str );
	GetDlgItem( IDC_STATICPOWER_C )->SetWindowText( str );
		
	if( NULL != GetpRadioPower() )
	{
		GetpRadioPower()->SetWindowText( str );
	}

	m_EditPowerHeating.SetPhysicalType( _U_TH_POWER );
	m_EditPowerHeating.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_EditPowerCooling.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_EditPowerCooling.SetPhysicalType( _U_TH_POWER );

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICDT );
	GetDlgItem( IDC_STATICDT_H )->SetWindowText( str );
	GetDlgItem( IDC_STATICDT_C )->SetWindowText( str );

	m_EditDTHeating.SetPhysicalType( _U_DIFFTEMP );
	m_EditDTHeating.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_EditDTCooling.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_EditDTCooling.SetPhysicalType( _U_DIFFTEMP );

	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICCONNECT );
	GetDlgItem( IDC_STATIC_CONNECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_STATICVERSION );
	GetDlgItem( IDC_STATIC_VERSION )->SetWindowText( str );

	return TRUE;
}

LRESULT CDlgIndSel6WayValve_6WayValveTab::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
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
	
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();

	// Set the radio Flow/PowerDT state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == ( CDS_SelProd::eFlowDef )pclIndSelParameter->Get6WValveRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_pclIndSel6WayParams->m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	Update6WayFlowOrPowerDTState();

	// Remark: Corresponding combo variables in 'm_clIndSel6WParams' are updated in each of this following methods.
	FillComboConnection( pclIndSelParameter->Get6WValveConnectID() );
	FillComboVersion( pclIndSelParameter->Get6WValveVersionID() );

	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();

	EnableSelectButtonNotification( false );
	m_bInitialised = true;

	return 0;
}

BEGIN_MESSAGE_MAP( CDlgIndSel6WayValve_6WayValveTab, CDlgIndSelCtrlBase )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnComboSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnComboSelChangeVersion )
	ON_BN_CLICKED( IDC_RADIOPOWER, OnBnClickedRadioFlowOrPowerDT )
	ON_BN_CLICKED( IDC_RADIOFLOW, OnBnClickedRadioFlowOrPowerDT )
	ON_EN_KILLFOCUS( IDC_EDITFLOW_C, OnEnKillFocusFlowCooling )
	ON_EN_KILLFOCUS( IDC_EDITPOWER_C, OnEnKillFocusPowerCooling )
	ON_EN_KILLFOCUS( IDC_EDITDT_C, OnEnKillFocusDTCooling )
	ON_EN_KILLFOCUS( IDC_EDITFLOW_H, OnEnKillFocusFlowHeating )
	ON_EN_KILLFOCUS( IDC_EDITPOWER_H, OnEnKillFocusPowerHeating )
	ON_EN_KILLFOCUS( IDC_EDITDT_H, OnEnKillFocusDTHeating )
	ON_EN_CHANGE( IDC_EDITFLOW_C, OnEnChangeFlowCooling )
	ON_EN_CHANGE( IDC_EDITPOWER_C, OnEnChangePowerCooling )
	ON_EN_CHANGE( IDC_EDITDT_C, OnEnChangeDTCooling )
	ON_EN_CHANGE( IDC_EDITFLOW_H, OnEnChangeFlowHeating )
	ON_EN_CHANGE( IDC_EDITPOWER_H, OnEnChangePowerHeating )
	ON_EN_CHANGE( IDC_EDITDT_H, OnEnChangeDTHeating )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITFLOW_C, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITPOWER_C, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDT_C, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITFLOW_H, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITPOWER_H, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDT_H, OnEditEnterChar )
END_MESSAGE_MAP()

void CDlgIndSel6WayValve_6WayValveTab::OnBnClickedRadioFlowOrPowerDT()
{
	Update6WayFlowOrPowerDTState();

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}
}

void CDlgIndSel6WayValve_6WayValveTab::OnComboSelChangeConnect()
{
	m_pclIndSel6WayValveParams->m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	FillComboVersion();
	OnComboSelChangeVersion();
}

void CDlgIndSel6WayValve_6WayValveTab::OnComboSelChangeVersion()
{
	m_pclIndSel6WayValveParams->m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	ClearAllNotification();
}

LRESULT CDlgIndSel6WayValve_6WayValveTab::OnPipeChange( WPARAM wParam, LPARAM lParam )
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

LRESULT CDlgIndSel6WayValve_6WayValveTab::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}
	
	return 0;
}

LRESULT CDlgIndSel6WayValve_6WayValveTab::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelCtrlBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATICUNITFLOW_H )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_FLOW ).c_str() );
	InitNumericalEdit( &m_EditFlowHeating, _U_FLOW );
	GetDlgItem( IDC_STATICUNITFLOW_C )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_FLOW ).c_str() );
	InitNumericalEdit( &m_EditFlowCooling, _U_FLOW );

	GetDlgItem( IDC_STATICUNITPOWER_H )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
	InitNumericalEdit( &m_EditPowerHeating, _U_TH_POWER );
	GetDlgItem( IDC_STATICUNITPOWER_C )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
	InitNumericalEdit( &m_EditPowerCooling, _U_TH_POWER );

	GetDlgItem( IDC_STATICUNITDT_H )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_DIFFTEMP ).c_str() );
	InitNumericalEdit( &m_EditDTHeating, _U_DIFFTEMP );
	GetDlgItem( IDC_STATICUNITDT_C )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_DIFFTEMP ).c_str() );
	InitNumericalEdit( &m_EditDTCooling, _U_DIFFTEMP );

	if( NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	return 0;
}

CRViewSSelSS * CDlgIndSel6WayValve_6WayValveTab::GetLinkedRightViewSSel( void )
{
	return pRViewSSel6WayValve;
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnKillFocusFlowCooling()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// Variable
	double dFlow = 0.0;

	if( RD_NOT_NUMBER != ReadCUDouble( _U_FLOW, m_EditFlowCooling, &dFlow ) )
	{
		m_pclIndSel6WayValveParams->m_dCoolingFlow = dFlow;
	}
	else
	{
		m_pclIndSel6WayValveParams->m_dCoolingFlow = 0.0;
	}
}


void CDlgIndSel6WayValve_6WayValveTab::OnEnKillFocusPowerCooling()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// Variable.
	double dPower = 0.0;

	if( RD_NOT_NUMBER != ReadCUDouble( _U_TH_POWER, m_EditPowerCooling, &dPower ) )
	{
		m_pclIndSel6WayValveParams->m_dCoolingPower = dPower;
	}
	else
	{
		m_pclIndSel6WayValveParams->m_dCoolingPower = 0.0;
	}
}


void CDlgIndSel6WayValve_6WayValveTab::OnEnKillFocusDTCooling()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// Variable.
	double dDT = 0.0;

	if( RD_NOT_NUMBER != ReadCUDouble( _U_DIFFTEMP, m_EditDTCooling, &dDT ) )
	{
		m_pclIndSel6WayValveParams->m_dCoolingDT = dDT;
	}
	else
	{
		m_pclIndSel6WayValveParams->m_dCoolingDT = 0.0;
	}

	if( false == m_bIsDTCoolingEdited && m_dPreviousCoolingDT != m_pclIndSel6WayValveParams->m_dCoolingDT )
	{
		m_bIsDTCoolingEdited = true;
	}
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnKillFocusFlowHeating()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// Variable.
	double dFlow = 0.0;

	if( RD_NOT_NUMBER != ReadCUDouble( _U_FLOW, m_EditFlowHeating, &dFlow ) )
	{
		m_pclIndSel6WayValveParams->m_dHeatingFlow = dFlow;
	}
	else
	{
		m_pclIndSel6WayValveParams->m_dHeatingFlow = 0.0;
	}
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnKillFocusPowerHeating()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// Variable.
	double dPower = 0.0;

	if( RD_NOT_NUMBER != ReadCUDouble( _U_TH_POWER, m_EditPowerHeating, &dPower ) )
	{
		m_pclIndSel6WayValveParams->m_dHeatingPower = dPower;
	}
	else
	{
		m_pclIndSel6WayValveParams->m_dHeatingPower = 0.0;
	}
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnKillFocusDTHeating()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// Variable.
	double dDT = 0.0;

	if( RD_NOT_NUMBER != ReadCUDouble( _U_DIFFTEMP, m_EditDTHeating, &dDT ) )
	{
		m_pclIndSel6WayValveParams->m_dHeatingDT = dDT;
	}
	else
	{
		m_pclIndSel6WayValveParams->m_dHeatingDT = 0.0;
	}

	if( false == m_bIsDTHeatingEdited && m_dPreviousHeatingDT != m_pclIndSel6WayValveParams->m_dHeatingDT )
	{
		m_bIsDTHeatingEdited = true;
	}
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnChangeFlowCooling()
{
	// Allow to clear the right view only on the first user change in the flow cooling field.
	if( Radio_Flow == m_iRadioFlowPowerDT  && NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	EnableSelectButtonNotification( false );
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnChangePowerCooling()
{
	// Allow to clear the right view only on the first user change in the Power cooling field.
	if( Radio_PowerDT == m_iRadioFlowPowerDT && NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	EnableSelectButtonNotification( false );
	_UpdateFlowCooling();
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnChangeDTCooling()
{
	// Allow to clear the right view only on the first user change in the Dt cooling field.
	if( Radio_PowerDT == m_iRadioFlowPowerDT && NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}
	
	m_dPreviousCoolingDT = m_pclIndSel6WayValveParams->m_dCoolingDT;
	EnableSelectButtonNotification( false );
	_UpdateFlowCooling();
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnChangeFlowHeating()
{
	// Allow to clear the right view only on the first user change in the flow heating field.
	if( Radio_Flow == m_iRadioFlowPowerDT && NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	EnableSelectButtonNotification( false );
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnChangePowerHeating()
{
	// Allow to clear the right view only on the first user change in the power heating field.
	if( Radio_PowerDT == m_iRadioFlowPowerDT && NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	EnableSelectButtonNotification( false );
	_UpdateFlowHeating();
}

void CDlgIndSel6WayValve_6WayValveTab::OnEnChangeDTHeating()
{
	// Allow to clear the right view only on the first user change in the DT heating field.
	if( Radio_PowerDT == m_iRadioFlowPowerDT && NULL != pRViewSSel6WayValve )
	{
		pRViewSSel6WayValve->Reset();
	}

	m_dPreviousHeatingDT = m_pclIndSel6WayValveParams->m_dHeatingDT;
	EnableSelectButtonNotification( false );
	_UpdateFlowHeating();
}

void CDlgIndSel6WayValve_6WayValveTab::OnEditEnterChar( NMHDR * pNMHDR, LRESULT * pResult )
{
	if( IDC_EDITFLOW_C == pNMHDR->idFrom || IDC_EDITFLOW_H == pNMHDR->idFrom || IDC_EDITPOWER_C == pNMHDR->idFrom 
		|| IDC_EDITPOWER_H == pNMHDR->idFrom || IDC_EDITDT_C == pNMHDR->idFrom || IDC_EDITDT_H == pNMHDR->idFrom )
	{
		OnEditEnterCharNotification( pNMHDR );
	}
}

void CDlgIndSel6WayValve_6WayValveTab::EnableSelectButtonNotification( bool bEnable )
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->I6WayNotification_EnableSelectButton( bEnable );
	}
}

void CDlgIndSel6WayValve_6WayValveTab::OnEditEnterCharNotification( NMHDR * pNMHDR )
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->I6WayNotification_OnEditEnterChar( pNMHDR );
	}
}

void CDlgIndSel6WayValve_6WayValveTab::ClearAllNotification()
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->I6WayNotification_ClearAll();
	}
}

void CDlgIndSel6WayValve_6WayValveTab::SetFocusNotification()
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->I6WayNotification_SetFocus();
	}
}

void CDlgIndSel6WayValve_6WayValveTab::SetFocusOnControlNotification( CDlgSelectionTabHelper::SetFocusWhere ePosition )
{
	if( NULL != m_pNotificationHandler )
	{
		m_pNotificationHandler->I6WayNotification_SetFocusOnControl( ePosition );
	}
}

void CDlgIndSel6WayValve_6WayValveTab::Update6WayFlowOrPowerDTState()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// 'TRUE' to fill 'm_iRadioFlow' variable with the radio state in the dialog.
	UpdateData( TRUE );

	if( Radio_Flow == m_iRadioFlowPowerDT )
	{
		// Input by the flow.
		m_EditPowerHeating.SetWindowText( _T("") );
		m_EditPowerCooling.SetWindowText( _T("") );
		m_EditDTHeating.SetWindowText( _T("") );
		m_EditDTCooling.SetWindowText( _T("") );
		m_EditPowerHeating.EnableWindow( FALSE );
		m_EditPowerCooling.EnableWindow( FALSE );
		m_EditDTHeating.EnableWindow( FALSE );
		m_EditDTCooling.EnableWindow( FALSE );
		m_EditFlowHeating.EnableWindow( TRUE );
		m_EditFlowCooling.EnableWindow( TRUE );
		m_EditFlowCooling.SetFocus();
		m_pclIndSelParams->m_eFlowOrPowerDTMode = CDS_SelProd::efdFlow;
		//Invalidate( false );
		//UpdateWindow();
	}
	else
	{
		// Input by the power and DT.
		m_pclIndSel6WayValveParams->m_dCoolingFlow = 0.0;
		m_pclIndSel6WayValveParams->m_dHeatingFlow = 0.0;
		m_EditFlowCooling.SetWindowText( _T("") );
		m_EditFlowHeating.SetWindowText( _T("") );
		m_EditFlowHeating.EnableWindow( FALSE );
		m_EditFlowCooling.EnableWindow( FALSE );
		m_EditPowerHeating.EnableWindow( TRUE );
		m_EditPowerCooling.EnableWindow( TRUE );
		m_EditDTHeating.EnableWindow( TRUE );
		m_EditDTCooling.EnableWindow( TRUE );

		CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
		
		if( false == m_bIsDTCoolingEdited )
		{
			m_EditDTCooling.SetCurrentValSI( pTechP->GetDefaultISChangeOverDT( CoolingSide ) );
			m_pclIndSel6WayValveParams->m_dCoolingDT = pTechP->GetDefaultISChangeOverDT( CoolingSide );
			m_EditDTCooling.Update();
		}
		
		if( false == m_bIsDTHeatingEdited )
		{
			m_EditDTHeating.SetCurrentValSI( pTechP->GetDefaultISChangeOverDT( HeatingSide ) );
			m_pclIndSel6WayValveParams->m_dHeatingDT = pTechP->GetDefaultISChangeOverDT( HeatingSide );
			m_EditDTHeating.Update();
		}

		m_EditPowerCooling.SetFocus();
		m_pclIndSelParams->m_eFlowOrPowerDTMode = CDS_SelProd::efdPower;
		//Invalidate( false );
		//UpdateWindow();
	}
}

void CDlgIndSel6WayValve_6WayValveTab::InitNumericalEdit( CExtNumEdit *pEdit, ePHYSTYPE phystype )
{
	if( NULL != pEdit )
	{
		pEdit->SetPhysicalType( phystype );
		pEdit->SetEditSign( CNumString::ePositive );
		pEdit->SetMinDblValue( 0.0 );

		if( _U_TEMPERATURE == phystype )
		{
			pEdit->SetEditSign( CNumString::eBoth );
			pEdit->SetMinDblValue( -273.15 );
		}

		pEdit->Update();
	}
}

void CDlgIndSel6WayValve_6WayValveTab::_UpdateFlowCooling()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	if( CDS_SelProd::efdFlow == m_pclIndSel6WayValveParams->m_eFlowOrPowerDTMode )
	{
		return;
	}

	if( ( RD_OK != ReadCUDouble( _U_TH_POWER, m_EditPowerCooling, &( m_pclIndSel6WayValveParams->m_dCoolingPower ) ) )
			|| ( 0 >= m_pclIndSel6WayValveParams->m_dCoolingPower ) )
	{
		m_pclIndSel6WayValveParams->m_dCoolingPower = 0.0;
	}

	if( ( RD_OK != ReadCUDouble( _U_DIFFTEMP, m_EditDTCooling, &( m_pclIndSel6WayValveParams->m_dCoolingDT ) ) )
			|| ( 0 >= m_pclIndSel6WayValveParams->m_dCoolingDT ) )
	{
		m_pclIndSel6WayValveParams->m_dCoolingDT = 0.0;
	}

	m_pclIndSel6WayValveParams->m_dCoolingFlow = 0.0;

	if( m_pclIndSel6WayValveParams->m_dCoolingPower > 0.0 && m_pclIndSel6WayValveParams->m_dCoolingDT > 0.0 )
	{
		m_pclIndSel6WayValveParams->m_dCoolingFlow = CalcqFromPDT( m_pclIndSel6WayValveParams->m_dCoolingPower, m_pclIndSel6WayValveParams->m_dCoolingDT,
				m_pclIndSel6WayValveParams->m_CoolingWC.GetDens(), m_pclIndSel6WayValveParams->m_CoolingWC.GetSpecifHeat() );
	}

	if( m_pclIndSel6WayValveParams->m_dCoolingFlow > 0.0 )
	{
		m_EditFlowCooling.SetWindowText( WriteCUDouble( _U_FLOW, m_pclIndSel6WayValveParams->m_dCoolingFlow ) );
	}
	else
	{
		m_EditFlowCooling.SetWindowText( _T("") );
	}
}

void CDlgIndSel6WayValve_6WayValveTab::_UpdateFlowHeating()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	if( CDS_SelProd::efdFlow == m_pclIndSel6WayValveParams->m_eFlowOrPowerDTMode )
	{
		return;
	}

	if( ( RD_OK != ReadCUDouble( _U_TH_POWER, m_EditPowerHeating, &( m_pclIndSel6WayValveParams->m_dHeatingPower ) ) )
			|| ( 0 >= m_pclIndSel6WayValveParams->m_dHeatingPower ) )
	{
		m_pclIndSel6WayValveParams->m_dHeatingPower = 0.0;
	}

	if( ( RD_OK != ReadCUDouble( _U_DIFFTEMP, m_EditDTHeating, &( m_pclIndSel6WayValveParams->m_dHeatingDT ) ) )
			|| ( 0 >= m_pclIndSel6WayValveParams->m_dHeatingDT ) )
	{
		m_pclIndSel6WayValveParams->m_dHeatingDT = 0.0;
	}

	m_pclIndSel6WayValveParams->m_dHeatingFlow = 0.0;

	if( m_pclIndSel6WayValveParams->m_dHeatingPower > 0.0 && m_pclIndSel6WayValveParams->m_dHeatingDT > 0.0 )
	{
		m_pclIndSel6WayValveParams->m_dHeatingFlow = CalcqFromPDT( m_pclIndSel6WayValveParams->m_dHeatingPower, m_pclIndSel6WayValveParams->m_dHeatingDT,
			m_pclIndSel6WayValveParams->m_HeatingWC.GetDens(), m_pclIndSel6WayValveParams->m_HeatingWC.GetSpecifHeat() );
	}

	if( m_pclIndSel6WayValveParams->m_dHeatingFlow > 0.0 )
	{
		m_EditFlowHeating.SetWindowText( WriteCUDouble( _U_FLOW, m_pclIndSel6WayValveParams->m_dHeatingFlow ) );
	}
	else
	{
		m_EditFlowHeating.SetWindowText( _T("") );
	}
}
