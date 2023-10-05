#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPPageHC.h"


IMPLEMENT_DYNAMIC( CDlgTPPageHC, CDlgCtrlPropPage )

CDlgTPPageHC::CDlgTPPageHC( CWnd* pParent )
	: CDlgCtrlPropPage(pParent )
{
	m_eProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	m_pTADS = NULL;

	// Set the HeaderTitle not with IDS because it will be override into the tech parameters.
	m_csHeaderTitle = _T("Hydronic circuit calculation preferences");
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_PAGENAME );
	m_bShowPayAttention = false;

	// Initialize the remarks.
	m_bRemarkCV50 = false;
	m_bRemarkDN50 = false;
	m_bRemarkCV65 = false;
	m_bRemarkDN65 = false;
	m_strRemarkCV50 = _T("");
	m_strRemarkDN50 = _T("");
	m_strRemarkCV65 = _T("");
	m_strRemarkDN65 = _T("");

	m_strTypeBelow65 = _T("");
	m_strTypeAbove50 = _T("");
	m_strFamilyBelow65 = _T("");
	m_strFamilyAbove50 = _T("");
	m_strBodyMaterialBelow65 = _T("");
	m_strBodyMaterialAbove50 = _T("");
	m_strConnectBelow65 = _T("");
	m_strConnectAbove50 = _T("");
	m_strVersionBelow65 = _T("");
	m_strVersionAbove50 = _T("");
	m_strPNBelow65 = _T("");
	m_strPNAbove50 = _T("");

	m_bShowPayAttention = false;

	// Helper to fill combos.
	// Index must correspond to the 'ProductSubCategory' enum definition in the 'DataBObj.h' file.
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_Undefined
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_PM_AirVent
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_PM_Separator,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_PM_ExpansionVessel,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_PM_TecBox,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_PM_SofteningAndDesalination,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSC_PM_SafetyValve,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_BC_RegulatingValve,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_BC_DpController,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eForTechHCBCv );		// PSB_BC_BalAndCtrlValve,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eForPiCv );			// PSB_BC_PressureIndepCtrlValve,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eForDpCBCV );			// PSB_BC_CombinedDpcBalCtrlValve,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eForTechHCCv );		// PSB_BC_ControlValve,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eForSmartControlValve );		// PSB_BC_SmartControlValve,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );		        // PSB_BC_SmartDp, (HYS-1935: TA-Smart Dp - 02 - Add a new category for the ribbon)
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eFor6WayCV );			// PSC_BC_6WayControlValve, (HYS-1150)
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_BC_HUB,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_TC_ThermostaticValve,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_TC_TapWaterControl,
	m_arTargetTab.Add( CTADatabase::CvTargetTab::eLast );				// PSB_ESC_ShutoffValve,
}

void CDlgTPPageHC::Init( bool bResetToDefault )
{
	// Fill variables for combobox.
	if( true == bResetToDefault )
	{
		m_pTADS->GetpProjectParams()->GetpHmCalcParams()->ResetPrjParams( true );
	}
	
	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );
	
	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:
			m_strTypeBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BvBlw65TypeID );
			m_strTypeAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BvAbv50TypeID );
			m_strFamilyBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BvBlw65FamilyID );
			m_strFamilyAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BvAbv50FamilyID );
			m_strBodyMaterialBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BvBlw65BdyMatID );
			m_strBodyMaterialAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BvAbv50BdyMatID );
			m_strConnectBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BvBlw65ConnectID );
			m_strConnectAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BvAbv50ConnectID );
			m_strVersionBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BvBlw65VersionID );
			m_strVersionAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BvAbv50VersionID );
			m_strPNBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BvBlw65PNID );
			m_strPNAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BvAbv50PNID );
			break;

		case ProductSubCategory::PSC_BC_DpController:
			m_strTypeAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::DpCAbv50TypeID );
			m_strTypeBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBlw65TypeID );
			m_strFamilyAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::DpCAbv50FamilyID );
			m_strFamilyBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBlw65FamilyID );
			m_strBodyMaterialAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::DpCAbv50BdyMatID );
			m_strBodyMaterialBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBlw65BdyMatID );
			m_strConnectAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::DpCAbv50ConnectID );
			m_strConnectBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBlw65ConnectID );
			m_strVersionAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::DpCAbv50VersionID );
			m_strVersionBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBlw65VersionID );
			m_strPNAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::DpCAbv50PNID );
			m_strPNBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBlw65PNID );
			break;

		case ProductSubCategory::PSC_BC_ControlValve:
			m_strTypeBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::CvBlw65TypeID );
			m_strTypeAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::CvAbv50TypeID );
			m_strFamilyBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::CvBlw65FamilyID );
			m_strFamilyAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::CvAbv50FamilyID );
			m_strBodyMaterialBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::CvBlw65BdyMatID );
			m_strBodyMaterialAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::CvAbv50BdyMatID );
			m_strConnectBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::CvBlw65ConnectID );
			m_strConnectAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::CvAbv50ConnectID );
			m_strVersionBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::CvBlw65VersionID );
			m_strVersionAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::CvAbv50VersionID );
			m_strPNBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::CvBlw65PNID );
			m_strPNAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::CvAbv50PNID );
			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:
			m_strTypeBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveBlw65TypeID );
			m_strTypeAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveAbv50TypeID );
			m_strFamilyBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveBlw65FamilyID );
			m_strFamilyAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveAbv50FamilyID );
			m_strBodyMaterialBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveBlw65BdyMatID );
			m_strBodyMaterialAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveAbv50BdyMatID );
			m_strConnectBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveBlw65ConnectID );
			m_strConnectAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveAbv50ConnectID );
			m_strVersionBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveBlw65VersionID );
			m_strVersionAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveAbv50VersionID );
			m_strPNBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveBlw65PNID );
			m_strPNAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartControlValveAbv50PNID );
			break;

		case ProductSubCategory::PSC_BC_SmartDpC:
			m_strTypeBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCBlw65TypeID );
			m_strTypeAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCAbv50TypeID );
			m_strFamilyBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCBlw65FamilyID );
			m_strFamilyAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCAbv50FamilyID );
			m_strBodyMaterialBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCBlw65BdyMatID );
			m_strBodyMaterialAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCAbv50BdyMatID );
			m_strConnectBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCBlw65ConnectID );
			m_strConnectAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCAbv50ConnectID );
			m_strVersionBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCBlw65VersionID );
			m_strVersionAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCAbv50VersionID );
			m_strPNBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCBlw65PNID );
			m_strPNAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::SmartDpCAbv50PNID );
			break;
		
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
			m_strTypeBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BCvBlw65TypeID );
			m_strTypeAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BCvAbv50TypeID );
			m_strFamilyBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BCvBlw65FamilyID );
			m_strFamilyAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BCvAbv50FamilyID );
			m_strBodyMaterialBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BCvBlw65BdyMatID );
			m_strBodyMaterialAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BCvAbv50BdyMatID );
			m_strConnectBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BCvBlw65ConnectID );
			m_strConnectAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BCvAbv50ConnectID );
			m_strVersionBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BCvBlw65VersionID );
			m_strVersionAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BCvAbv50VersionID );
			m_strPNBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::BCvBlw65PNID );
			m_strPNAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::BCvAbv50PNID );
			break;

		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
			m_strTypeAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::PICvAbv50TypeID );
			m_strTypeBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::PICvBlw65TypeID );
			m_strFamilyAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::PICvAbv50FamilyID );
			m_strFamilyBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::PICvBlw65FamilyID );
			m_strBodyMaterialAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::PICvAbv50BdyMatID );
			m_strBodyMaterialBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::PICvBlw65BdyMatID );
			m_strConnectAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::PICvAbv50ConnectID );
			m_strConnectBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::PICvBlw65ConnectID );
			m_strVersionAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::PICvAbv50VersionID );
			m_strVersionBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::PICvBlw65VersionID );
			m_strPNAbove50 = pHMGenParam->GetPrjParamID( CPrjParams::PICvAbv50PNID );
			m_strPNBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::PICvBlw65PNID );
			break;

		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:
			m_strTypeBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBCValveBlw65TypeID );
			m_strFamilyBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBCValveBlw65FamilyID );
			m_strBodyMaterialBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBCValveBlw65BdyMatID );
			m_strConnectBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBCValveBlw65ConnectID );
			m_strVersionBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBCValveBlw65VersionID );
			m_strPNBelow65 = pHMGenParam->GetPrjParamID( CPrjParams::DpCBCValveBlw65PNID );

			// Not applicable for the moment because we have not yet combined Dp controller, control and balancing valve above DN50.
			GetDlgItem( IDC_STATICABV65 )->EnableWindow( FALSE );
			m_strTypeAbove50 = _T("");
			m_strFamilyAbove50 = _T("");
			m_strBodyMaterialAbove50 = _T("");
			m_strConnectAbove50 = _T("");
			m_strVersionAbove50 = _T("");
			m_strPNAbove50 = _T("");

			break;
	}
	
	// Fill the combo box.
	FillComboBoxType( Below65 );
	FillComboBoxFamily( Below65 );
	FillComboBoxBodyMaterial( Below65 );
	FillComboBoxConnect( Below65 );
	FillComboBoxVersion( Below65 );
	FillComboBoxPN( Below65 );

	FillComboBoxType( Above50 );
	FillComboBoxFamily( Above50 );
	FillComboBoxBodyMaterial( Above50 );
	FillComboBoxConnect( Above50 );
	FillComboBoxVersion( Above50 );
	FillComboBoxPN( Above50 );
}

void CDlgTPPageHC::Save( CString strSectionName )
{
	CPrjParams *pHMGenParam = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );
	
	if ( NULL != GetSafeHwnd() )
	{
		switch( GetProductSubCategory() )
		{
			case ProductSubCategory::PSC_BC_RegulatingValve:

				pHMGenParam->SetPrjParamID( CPrjParams::BvAbv50TypeID, m_strTypeAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvBlw65TypeID, m_strTypeBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvAbv50FamilyID, m_strFamilyAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvBlw65FamilyID, m_strFamilyBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvAbv50BdyMatID, m_strBodyMaterialAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvBlw65BdyMatID, m_strBodyMaterialBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvAbv50ConnectID, m_strConnectAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvBlw65ConnectID, m_strConnectBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvAbv50VersionID, m_strVersionAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvBlw65VersionID, m_strVersionBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvAbv50PNID, m_strPNAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BvBlw65PNID, m_strPNBelow65 );

				break;

			case ProductSubCategory::PSC_BC_ControlValve:

				pHMGenParam->SetPrjParamID( CPrjParams::CvAbv50TypeID, m_strTypeAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvBlw65TypeID, m_strTypeBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvAbv50FamilyID, m_strFamilyAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvBlw65FamilyID, m_strFamilyBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvAbv50BdyMatID, m_strBodyMaterialAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvBlw65BdyMatID, m_strBodyMaterialBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvAbv50ConnectID, m_strConnectAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvBlw65ConnectID, m_strConnectBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvAbv50VersionID, m_strVersionAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvBlw65VersionID, m_strVersionBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvAbv50PNID, m_strPNAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::CvBlw65PNID, m_strPNBelow65 );

				break;

			case ProductSubCategory::PSC_BC_SmartControlValve:

				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveAbv50TypeID, m_strTypeAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveBlw65TypeID, m_strTypeBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveAbv50FamilyID, m_strFamilyAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveBlw65FamilyID, m_strFamilyBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveAbv50BdyMatID, m_strBodyMaterialAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveBlw65BdyMatID, m_strBodyMaterialBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveAbv50ConnectID, m_strConnectAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveBlw65ConnectID, m_strConnectBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveAbv50VersionID, m_strVersionAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveBlw65VersionID, m_strVersionBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveAbv50PNID, m_strPNAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartControlValveBlw65PNID, m_strPNBelow65 );

				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveAbv50TypeID"), m_strTypeAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveBlw65TypeID"), m_strTypeBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveAbv50FamilyID"), m_strFamilyAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveBlw65FamilyID"), m_strFamilyBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveAbv50BdyMatID"), m_strBodyMaterialAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveBlw65BdyMatID"), m_strBodyMaterialBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveAbv50ConnectID"), m_strConnectAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveBlw65ConnectID"), m_strConnectBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveAbv50VersionID"), m_strVersionAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveBlw65VersionID"), m_strVersionBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveAbv50PNID"), m_strPNAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartControlValveBlw65PNID"), m_strPNBelow65 );

				break;

			case ProductSubCategory::PSC_BC_SmartDpC:

				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCAbv50TypeID, m_strTypeAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCBlw65TypeID, m_strTypeBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCAbv50FamilyID, m_strFamilyAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCBlw65FamilyID, m_strFamilyBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCAbv50BdyMatID, m_strBodyMaterialAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCBlw65BdyMatID, m_strBodyMaterialBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCAbv50ConnectID, m_strConnectAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCBlw65ConnectID, m_strConnectBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCAbv50VersionID, m_strVersionAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCBlw65VersionID, m_strVersionBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCAbv50PNID, m_strPNAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::SmartDpCBlw65PNID, m_strPNBelow65 );

				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCAbv50TypeID"), m_strTypeAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCBlw65TypeID"), m_strTypeBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCAbv50FamilyID"), m_strFamilyAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCBlw65FamilyID"), m_strFamilyBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCAbv50BdyMatID"), m_strBodyMaterialAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCBlw65BdyMatID"), m_strBodyMaterialBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCAbv50ConnectID"), m_strConnectAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCBlw65ConnectID"), m_strConnectBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCAbv50VersionID"), m_strVersionAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCBlw65VersionID"), m_strVersionBelow65 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCAbv50PNID"), m_strPNAbove50 );
				::AfxGetApp()->WriteProfileString( strSectionName, _T("SmartDpCBlw65PNID"), m_strPNBelow65 );

				break;

			case ProductSubCategory::PSC_BC_BalAndCtrlValve:

				pHMGenParam->SetPrjParamID( CPrjParams::BCvAbv50TypeID, m_strTypeAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvBlw65TypeID, m_strTypeBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvAbv50FamilyID, m_strFamilyAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvBlw65FamilyID, m_strFamilyBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvAbv50BdyMatID, m_strBodyMaterialAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvBlw65BdyMatID, m_strBodyMaterialBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvAbv50ConnectID, m_strConnectAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvBlw65ConnectID, m_strConnectBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvAbv50VersionID, m_strVersionAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvBlw65VersionID, m_strVersionBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvAbv50PNID, m_strPNAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::BCvBlw65PNID, m_strPNBelow65 );

				break;

			case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:

				pHMGenParam->SetPrjParamID( CPrjParams::PICvAbv50TypeID, m_strTypeAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvBlw65TypeID, m_strTypeBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvAbv50FamilyID, m_strFamilyAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvBlw65FamilyID, m_strFamilyBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvAbv50BdyMatID, m_strBodyMaterialAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvBlw65BdyMatID, m_strBodyMaterialBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvAbv50ConnectID, m_strConnectAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvBlw65ConnectID, m_strConnectBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvAbv50VersionID, m_strVersionAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvBlw65VersionID, m_strVersionBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvAbv50PNID, m_strPNAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::PICvBlw65PNID, m_strPNBelow65 );

				break;

			case ProductSubCategory::PSC_BC_DpController:

				pHMGenParam->SetPrjParamID( CPrjParams::DpCAbv50TypeID, m_strTypeAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBlw65TypeID, m_strTypeBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCAbv50FamilyID, m_strFamilyAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBlw65FamilyID, m_strFamilyBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCAbv50BdyMatID, m_strBodyMaterialAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBlw65BdyMatID, m_strBodyMaterialBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCAbv50ConnectID, m_strConnectAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBlw65ConnectID, m_strConnectBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCAbv50VersionID, m_strVersionAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBlw65VersionID, m_strVersionBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCAbv50PNID, m_strPNAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBlw65PNID, m_strPNBelow65 );

				break;

			case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:

				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveAbv50TypeID, m_strTypeAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveBlw65TypeID, m_strTypeBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveAbv50FamilyID, m_strFamilyAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveBlw65FamilyID, m_strFamilyBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveAbv50BdyMatID, m_strBodyMaterialAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveBlw65BdyMatID, m_strBodyMaterialBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveAbv50ConnectID, m_strConnectAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveBlw65ConnectID, m_strConnectBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveAbv50VersionID, m_strVersionAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveBlw65VersionID, m_strVersionBelow65 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveAbv50PNID, m_strPNAbove50 );
				pHMGenParam->SetPrjParamID( CPrjParams::DpCBCValveBlw65PNID, m_strPNBelow65 );

				break;
		}
	}
}

void CDlgTPPageHC::UpdateComboWithDeleted()
{
	// Fill the combo box.
	FillComboBoxType( Below65 );
	FillComboBoxFamily( Below65 );
	FillComboBoxBodyMaterial( Below65 );
	FillComboBoxConnect( Below65 );
	FillComboBoxVersion( Below65 );
	FillComboBoxPN( Below65 );

	FillComboBoxType( Above50 );
	FillComboBoxFamily( Above50 );
	FillComboBoxBodyMaterial( Above50 );
	FillComboBoxConnect( Above50 );
	FillComboBoxVersion( Above50 );
	FillComboBoxPN( Above50 );
}

BEGIN_MESSAGE_MAP( CDlgTPPageHC, CDlgCtrlPropPage )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE50, OnCbnSelChangeTypeBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE65, OnCbnSelChangeTypeAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOFAM50, OnCbnSelChangeFamilyBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOFAM65, OnCbnSelChangeFamilyAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOBDYMAT50, OnCbnSelChangeBodyMaterialBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOBDYMAT65,OnCbnSelChangeBodyMaterialAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOCON50, OnCbnSelChangeConnectBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOCON65,OnCbnSelChangeConnectAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOVER50,OnCbnSelChangeVersionBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOVER65, OnCbnSelChangeVersionAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOPN50, OnCbnSelChangePNBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOPN65, OnCbnSelChangePNAbove50 )
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void CDlgTPPageHC::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOTYPE50, m_ComboTypeBelow65 );
	DDX_Control( pDX, IDC_COMBOTYPE65, m_ComboTypeAbove50 );
	DDX_Control( pDX, IDC_COMBOFAM50, m_ComboFamilyBelow65 );
	DDX_Control( pDX, IDC_COMBOFAM65, m_ComboFamilyAbove50 );
	DDX_Control( pDX, IDC_COMBOBDYMAT50, m_ComboBodyMaterialBelow65 );
	DDX_Control( pDX, IDC_COMBOBDYMAT65, m_ComboBodyMaterialAbove50 );
	DDX_Control( pDX, IDC_COMBOCON50, m_ComboConnextBelow65 );
	DDX_Control( pDX, IDC_COMBOCON65, m_ComboConnectAbove50 );
	DDX_Control( pDX, IDC_COMBOVER50, m_ComboVersionBelow65 );
	DDX_Control( pDX, IDC_COMBOVER65, m_ComboVersionAbove50 );
	DDX_Control( pDX, IDC_COMBOPN50, m_ComboPNBelow65 );
	DDX_Control( pDX, IDC_COMBOPN65, m_ComboPNAbove50 );
	
	DDX_Control( pDX, IDC_STATICINFO1, m_InfoType1 );
	DDX_Control( pDX, IDC_STATICINFONBR1, m_InfoTypeNbr1 );
	DDX_Control( pDX, IDC_STATICINFO2, m_InfoType2 );
	DDX_Control( pDX, IDC_STATICINFONBR2, m_InfoTypeNbr2 );
	DDX_Control( pDX, IDC_STATICINFOBOX1, m_Info1 );
	DDX_Control( pDX, IDC_STATICINFOBOXNBR1, m_InfoNbr1 );
	DDX_Control( pDX, IDC_STATICINFOBOX2, m_Info2 );
	DDX_Control( pDX, IDC_STATICINFOBOXNBR2, m_InfoNbr2 );
	DDX_Control( pDX, IDC_EDIT_TP_HC1, m_EditBoxText1 );
	DDX_Control( pDX, IDC_EDIT_TP_HC2, m_EditBoxText2 );
	DDX_Control( pDX, IDC_EDIT_TP_HC3, m_EditBoxText3 );
	DDX_Control( pDX, IDC_STATICBLW50, m_GroupBelow50 );
	DDX_Control( pDX, IDC_STATICABV65, m_GroupAbove65 );
}

BOOL CDlgTPPageHC::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE50 )->SetWindowText( str );
	GetDlgItem( IDC_STATICTYPE65 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_STATICFAM );
	GetDlgItem( IDC_STATICFAM50 )->SetWindowText( str );
	GetDlgItem( IDC_STATICFAM65 )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_STATICBDYMAT );
	GetDlgItem( IDC_STATICBDYMAT50 )->SetWindowText( str );
	GetDlgItem( IDC_STATICBDYMAT65 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_STATICCON );
	GetDlgItem( IDC_STATICCON50 )->SetWindowText( str );
	GetDlgItem( IDC_STATICCON65 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_STATICVER );
	GetDlgItem( IDC_STATICVERSION50 )->SetWindowText( str );
	GetDlgItem( IDC_STATICVERSION65 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_STATICPN );
	GetDlgItem( IDC_STATICPN50 )->SetWindowText( str );
	GetDlgItem( IDC_STATICPN65 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_STATICBLW50 );
	m_GroupBelow50.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_STATICABV65 );
	m_GroupAbove65.SetWindowText( str );

	// Define the background colors for the group boxes.
	m_GroupBelow50.SetBckgndColor( _WHITE_DLGBOX );
	m_GroupAbove65.SetBckgndColor( _WHITE_DLGBOX );
	
	// Format some strings.
	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:
			m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCBV_HEADERTITLE );
			m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCBV_PAGENAME );
			m_EditBoxText3.SetWindowText( TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKGENERALBV ) );
			break;

		case ProductSubCategory::PSC_BC_ControlValve:
			m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCCV_HEADERTITLE );
			m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCCV_PAGENAME );
			m_EditBoxText3.SetWindowText( TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKGENERALCV ) );
			break;

		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
			m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCBCV_HEADERTITLE );
			m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCBCV_PAGENAME );
			m_EditBoxText3.SetWindowText( TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKGENERALBCV ) );
			break;

		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
			m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPICV_HEADERTITLE );
			m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPICV_PAGENAME );
			m_EditBoxText3.SetWindowText( TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKGENERALCV ) );
			break;

		case ProductSubCategory::PSC_BC_DpController:
			m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCDPC_HEADERTITLE );
			m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCDPC_PAGENAME );
			m_EditBoxText3.SetWindowText( TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKGENERALDPC ) );
			break;

		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:
			m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCDPCBCV_HEADERTITLE );
			m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCDPCBCV_PAGENAME );
			m_EditBoxText3.SetWindowText( TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKGENERALDPCBCV ) );
			break;
	}

	m_ImageList.Create( IDB_CANNOTSENDSIGNS, 16, 1, _BLACK );
	m_ImageList.SetBkColor( CLR_NONE );
	
	// Get the icon "Pay attention".
	HICON hIcon = m_ImageList.ExtractIcon( 1 );
	
	// Set the icon where it's needed.
	m_InfoType1.SetIcon( hIcon );
	m_InfoType2.SetIcon(hIcon);
	m_Info1.SetIcon( hIcon );
	m_Info2.SetIcon( hIcon );
	
	// Remarks are disable by default.
	ShowInfos( RemarkDN::DN50, RemarkType::CV, false );
	ShowInfos( RemarkDN::DN65, RemarkType::CV, false );
	ShowInfos( RemarkDN::DN50, RemarkType::DN, false );
	ShowInfos( RemarkDN::DN65, RemarkType::DN, false );

	// Initialize 'm_pTADS'.
	m_pTADS = TASApp.GetpTADS();

	Init();

	return TRUE;
}

HBRUSH CDlgTPPageHC::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	HBRUSH hbr;

	if( pWnd == &m_InfoTypeNbr1 || pWnd == &m_InfoNbr1 || pWnd == &m_InfoTypeNbr2 || pWnd == &m_InfoNbr2 )
	{
		pDC->SetBkMode( TRANSPARENT );
		hbr = (HBRUSH)GetStockObject( NULL_BRUSH );

	}
	else
	{
		hbr = CDialogExt::OnCtlColor( pDC, pWnd, nCtlColor );
	
		if( NULL != m_brBkgr.GetSafeHandle() || NULL != m_hBkgrBitmap )
		{
			#define AFX_MAX_CLASS_NAME 255
			#define AFX_EDIT_CLASS _T("Edit")
			TCHAR lpszClassName[AFX_MAX_CLASS_NAME + 1];
			::GetClassName( pWnd->GetSafeHwnd(), lpszClassName, AFX_MAX_CLASS_NAME );
			CString strClass = lpszClassName;
		
			if( AFX_EDIT_CLASS == strClass )
			{	
				pDC->SetBkMode( TRANSPARENT );
				return (HBRUSH)m_brBkgr;
			}
		}
	}

	return hbr;
}

void CDlgTPPageHC::OnCbnSelChangeTypeBelow65()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboTypeBelow65.GetItemData( m_ComboTypeBelow65.GetCurSel() ) );

	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strTypeBelow65 = pStrIDPointer->GetIDPtr().ID;

	// Verify if ControlValve type is only valid for 2W,3W or 3w-4W circuit
	// Display warning message if needed.
	WarningMsgCV( RemarkDN::DN50 );
	
	// Refresh combobox.
	FillComboBoxFamily( Below65 );
	FillComboBoxBodyMaterial( Below65 );
	FillComboBoxConnect( Below65 );
	FillComboBoxVersion( Below65 );
	FillComboBoxPN( Below65 );
}

void CDlgTPPageHC::OnCbnSelChangeTypeAbove50()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboTypeAbove50.GetItemData( m_ComboTypeAbove50.GetCurSel() ) );

	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strTypeAbove50 = pStrIDPointer->GetIDPtr().ID;

	// Verify if ControlValve type is only valid for 2W,3W or 3w-4W circuit
	// Display warning message if needed
	WarningMsgCV( RemarkDN::DN65 );

	// Refresh combobox.
	FillComboBoxFamily( Above50 );
	FillComboBoxBodyMaterial( Above50 );
	FillComboBoxConnect( Above50 );
	FillComboBoxVersion( Above50 );
	FillComboBoxPN( Above50 );
}

void CDlgTPPageHC::OnCbnSelChangeFamilyBelow65()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboFamilyBelow65.GetItemData( m_ComboFamilyBelow65.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strFamilyBelow65 = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxBodyMaterial( Below65 );
	FillComboBoxConnect( Below65 );
	FillComboBoxVersion( Below65 );
	FillComboBoxPN( Below65 );
}

void CDlgTPPageHC::OnCbnSelChangeFamilyAbove50()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboFamilyAbove50.GetItemData( m_ComboFamilyAbove50.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strFamilyAbove50 = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxBodyMaterial( Above50 );
	FillComboBoxConnect( Above50 );
	FillComboBoxVersion( Above50 );
	FillComboBoxPN( Above50 );
}

void CDlgTPPageHC::OnCbnSelChangeBodyMaterialBelow65()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboBodyMaterialBelow65.GetItemData( m_ComboBodyMaterialBelow65.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strBodyMaterialBelow65 = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxConnect( Below65 );
	FillComboBoxVersion( Below65 );
	FillComboBoxPN( Below65 );
}

void CDlgTPPageHC::OnCbnSelChangeBodyMaterialAbove50()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboBodyMaterialAbove50.GetItemData( m_ComboBodyMaterialAbove50.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strBodyMaterialAbove50 = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxConnect( Above50 );
	FillComboBoxVersion( Above50 );
	FillComboBoxPN( Above50 );
}

void CDlgTPPageHC::OnCbnSelChangeConnectBelow65()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboConnextBelow65.GetItemData( m_ComboConnextBelow65.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strConnectBelow65 = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxVersion( Below65 );
	FillComboBoxPN( Below65 );
}

void CDlgTPPageHC::OnCbnSelChangeConnectAbove50()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboConnectAbove50.GetItemData( m_ComboConnectAbove50.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strConnectAbove50 = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxVersion( Above50 );
	FillComboBoxPN( Above50 );
}

void CDlgTPPageHC::OnCbnSelChangeVersionBelow65()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboVersionBelow65.GetItemData( m_ComboVersionBelow65.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strVersionBelow65 = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxPN( Below65 );
}

void CDlgTPPageHC::OnCbnSelChangeVersionAbove50()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboVersionAbove50.GetItemData( m_ComboVersionAbove50.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strVersionAbove50 = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxPN( Above50 );
}

void CDlgTPPageHC::OnCbnSelChangePNBelow65()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboPNBelow65.GetItemData( m_ComboPNBelow65.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strPNBelow65 = pStrIDPointer->GetIDPtr().ID;
}

void CDlgTPPageHC::OnCbnSelChangePNAbove50()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboPNAbove50.GetItemData( m_ComboPNAbove50.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strPNAbove50 = pStrIDPointer->GetIDPtr().ID;
}

void CDlgTPPageHC::ShowInfos( RemarkDN eDN, RemarkType eType, bool bShow, CString Text )
{
	CStatic *pInfoType = NULL;
	CStatic *pInfoTypeNbr = NULL;
	CStatic *pInfo = NULL;
	CStatic *pInfoNbr = NULL;
	CEdit *pEditBoxText = NULL;
	CString strRemarkCV;
	CString strRemarkDN;
	bool bShowRemarkCV = false;
	bool bShowRemarkDN = false;

	// Initialize the remarks type.
	if( RemarkType::DN == eType && RemarkDN::DN50 == eDN )
	{
		m_bRemarkDN50 = bShow;
		m_strRemarkDN50 = Text;
	}
	else if( RemarkType::DN == eType && RemarkDN::DN65 == eDN )
	{
		m_bRemarkDN65 = bShow;
		m_strRemarkDN65 = Text;
	}
	else if( RemarkType::CV == eType && RemarkDN::DN50 == eDN )
	{
		m_bRemarkCV50 = bShow;
		m_strRemarkCV50 = Text;
	}
	else if( RemarkType::CV == eType && RemarkDN::DN65 == eDN )
	{
		m_bRemarkCV65 = bShow;
		m_strRemarkCV65 = Text;
	}

	// Initialize the remarks DNs
	// ( < DN65 or > DN50)
	if( RemarkDN::DN50 == eDN )
	{
		pInfoType = &m_InfoType1;
		pInfoTypeNbr = &m_InfoTypeNbr1;
		pInfo = &m_Info1;
		pInfoNbr = &m_InfoNbr1;
		pEditBoxText = &m_EditBoxText1;
		strRemarkCV = m_strRemarkCV50;
		strRemarkDN = m_strRemarkDN50;
		bShowRemarkCV = m_bRemarkCV50;
		bShowRemarkDN = m_bRemarkDN50;
	}
	else if( RemarkDN::DN65 == eDN )
	{
		pInfoType = &m_InfoType2;
		pInfoTypeNbr = &m_InfoTypeNbr2;
		pInfo = &m_Info2;
		pInfoNbr = &m_InfoNbr2;
		pEditBoxText = &m_EditBoxText2;
		strRemarkCV = m_strRemarkCV65;
		strRemarkDN = m_strRemarkDN65;
		bShowRemarkCV = m_bRemarkCV65;
		bShowRemarkDN = m_bRemarkDN65;
	}	
	
	if( true == bShowRemarkDN && false == bShowRemarkCV )
	{
		// Show the edit box.
		pInfoType->ShowWindow( SW_SHOW );
		pInfoTypeNbr->ShowWindow( SW_SHOW );
		pInfo->ShowWindow( SW_SHOW );
		pInfoNbr->ShowWindow( SW_SHOW );
		pEditBoxText->ShowWindow( SW_SHOW );

		pEditBoxText->SetWindowText( strRemarkDN );
	}
	else if( false == bShowRemarkDN && true == bShowRemarkCV )
	{
		// Show the edit box.
		pInfoType->ShowWindow( SW_SHOW );
		pInfoTypeNbr->ShowWindow( SW_SHOW );
		pInfo->ShowWindow( SW_SHOW );
		pInfoNbr->ShowWindow( SW_SHOW );
		pEditBoxText->ShowWindow( SW_SHOW );

		pEditBoxText->SetWindowText( strRemarkCV );
	}
	else if( true == bShowRemarkDN && true == bShowRemarkCV )
	{
		// Show the edit box.
		pInfoType->ShowWindow( SW_SHOW );
		pInfoTypeNbr->ShowWindow( SW_SHOW );
		pInfo->ShowWindow( SW_SHOW );
		pInfoNbr->ShowWindow( SW_SHOW );
		pEditBoxText->ShowWindow( SW_SHOW );

		CString str;
		str = strRemarkDN;
		str += _T("\r\n") + strRemarkCV;
		pEditBoxText->SetWindowText( str );
	}
	else if( false == bShowRemarkDN && false == bShowRemarkCV )
	{
		// Hide the edit box.
		pInfoType->ShowWindow( SW_HIDE );
		pInfoTypeNbr->ShowWindow( SW_HIDE );
		pInfo->ShowWindow( SW_HIDE );
		pInfoNbr->ShowWindow( SW_HIDE );
		pEditBoxText->ShowWindow( SW_HIDE );

		pEditBoxText->SetWindowText( _T("") );
	}

	if( true == m_bRemarkDN50 || true == m_bRemarkCV50 || true == m_bRemarkDN65 || true == m_bRemarkCV65 )
	{
		m_EditBoxText3.ShowWindow( SW_SHOW );
	}
	else
	{
		m_EditBoxText3.ShowWindow( SW_HIDE );
	}
}

void CDlgTPPageHC::FillComboBoxType( AboveOrBelow eAboveOrBelow )
{
	int iDN;
	int iCount;
	CExtNumEditComboBox *pCCombo;
	CString* pCstr;
	CRankEx ListEx;
	RemarkDN eDN;

	CTADatabase *pTADB = TASApp.GetpTADB();
	ASSERT( NULL != pTADB );

	CTableDN *pclTableDN = (CTableDN *)( pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	if( Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboTypeAbove50;
		pCstr = &m_strTypeAbove50;
		eDN = RemarkDN::DN65;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboTypeBelow65;
		pCstr = &m_strTypeBelow65;
		eDN = RemarkDN::DN50;
	}

	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetBVTypeList( &ListEx, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX, true );
			}
			else
			{
				iCount = pTADB->GetBVTypeList( &ListEx, CTADatabase::FilterSelection::ForHMCalc, 0, iDN, true );
			}

			break;

		case ProductSubCategory::PSC_BC_ControlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetTaCVTypeList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::CV2W3W::LastCV2W3W,
													CDB_ControlProperties::eCVFUNC::ControlOnly,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc,
													iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetTaCVTypeList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::CV2W3W::LastCV2W3W,
													CDB_ControlProperties::eCVFUNC::ControlOnly,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc,
													0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetSmartControlValveTypeList( &ListEx, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetSmartControlValveTypeList( &ListEx, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;
			
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetTaCVTypeList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::CV2W3W::LastCV2W3W,
													CDB_ControlProperties::eCVFUNC::Presettable,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc,
													iDN, INT_MAX );
													
				iCount += pTADB->GetTaCVTypeList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::CV2W3W::LastCV2W3W,
													CDB_ControlProperties::eCVFUNC::PresetPT,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc,
													iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetTaCVTypeList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::CV2W3W::LastCV2W3W,
													CDB_ControlProperties::eCVFUNC::Presettable,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc,
													0, iDN );
													
				iCount = pTADB->GetTaCVTypeList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::CV2W3W::LastCV2W3W,
													CDB_ControlProperties::eCVFUNC::PresetPT,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc,
													0, iDN );
			}

			break;
	
		case ProductSubCategory::PSC_BC_DpController:
			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetDpCTypeList( &ListEx, CTADatabase::FilterSelection::ForHMCalc, eDpCLoc::DpCLocNone, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetDpCTypeList( &ListEx, CTADatabase::FilterSelection::ForHMCalc, eDpCLoc::DpCLocNone, 0, iDN );
			}
			break;
	}
	
	FillComboBox( &ListEx, pCCombo, iCount, pCstr );

	// Verify if ControlValve type is only valid for 2W,3W or 3w-4W circuit.
	// Display warning message if needed.
	WarningMsgCV( eDN );
}

void CDlgTPPageHC::FillComboBoxFamily( AboveOrBelow eAboveOrBelow )
{
	int iDN;
	CExtNumEditComboBox *pCCombo;
	CString *pCstr;
	CRankEx ListEx;
	int iCount;

	CTADatabase *pTADB = TASApp.GetpTADB();
	ASSERT( NULL != pTADB );

	CTableDN *pclTableDN = (CTableDN *)( pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	if( Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboFamilyAbove50;
		pCstr = &m_strFamilyAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboFamilyBelow65;
		pCstr = &m_strFamilyBelow65;
	}

	CTADatabase::CvTargetTab arTargetTab[] = { CTADatabase::CvTargetTab::eLast, CTADatabase::CvTargetTab::eForTechHCCv, 
												CTADatabase::CvTargetTab::eForTechHCBCv, CTADatabase::CvTargetTab::eForPiCv };
	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetBVFamilyList( &ListEx, (LPCTSTR)m_strTypeAbove50, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetBVFamilyList( &ListEx, (LPCTSTR)m_strTypeBelow65, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetTaCVFamList(	&ListEx,
												m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
												CDB_ControlProperties::LastCV2W3W,
												(LPCTSTR)m_strTypeAbove50,
												CDB_ControlProperties::LastCVFUNC,
												CDB_ControlProperties::CvCtrlType::eCvNU,
												CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetTaCVFamList(	&ListEx,
												m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
												CDB_ControlProperties::LastCV2W3W,
												(LPCTSTR)m_strTypeBelow65,
												CDB_ControlProperties::LastCVFUNC,
												CDB_ControlProperties::CvCtrlType::eCvNU,
												CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetSmartControlValveFamilyList( &ListEx, (LPCTSTR)m_strTypeAbove50, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetSmartControlValveFamilyList( &ListEx, (LPCTSTR)m_strTypeBelow65, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_DpController:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetDpCFamilyList( &ListEx, eDpCLoc::DpCLocDownStream, (LPCTSTR)m_strTypeAbove50, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetDpCFamilyList( &ListEx, eDpCLoc::DpCLocDownStream, (LPCTSTR)m_strTypeBelow65, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;
	}	

	FillComboBox( &ListEx, pCCombo, iCount, pCstr );
}

void CDlgTPPageHC::FillComboBoxBodyMaterial( AboveOrBelow eAboveOrBelow )
{
	int iDN;
	CExtNumEditComboBox *pCCombo;
	CString *pCstr;
	CRankEx ListEx;
	int iCount;

	CTADatabase *pTADB = TASApp.GetpTADB();
	ASSERT( NULL != pTADB );

	CTableDN *pclTableDN = (CTableDN *)( pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	if( Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboBodyMaterialAbove50;
		pCstr = &m_strBodyMaterialAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboBodyMaterialBelow65;
		pCstr = &m_strBodyMaterialBelow65;
	}

	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetBVBdyMatList( &ListEx, (LPCTSTR)m_strTypeAbove50, (LPCTSTR)m_strFamilyAbove50, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetBVBdyMatList( &ListEx, (LPCTSTR)m_strTypeBelow65, (LPCTSTR)m_strFamilyBelow65, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetTaCVBdyMatList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::LastCV2W3W,
													(LPCTSTR)m_strTypeAbove50,
													(LPCTSTR)m_strFamilyAbove50,
													CDB_ControlProperties::LastCVFUNC,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetTaCVBdyMatList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::LastCV2W3W,
													(LPCTSTR)m_strTypeBelow65,
													(LPCTSTR)m_strFamilyBelow65,
													CDB_ControlProperties::LastCVFUNC,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetSmartControlValveBdyMatList( &ListEx, (LPCTSTR)m_strTypeAbove50, (LPCTSTR)m_strFamilyAbove50, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetSmartControlValveBdyMatList( &ListEx, (LPCTSTR)m_strTypeBelow65, (LPCTSTR)m_strFamilyBelow65, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_DpController:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetDpCBdyMatList( &ListEx, eDpCLoc::DpCLocDownStream, (LPCTSTR)m_strTypeAbove50, (LPCTSTR)m_strFamilyAbove50, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetDpCBdyMatList( &ListEx, eDpCLoc::DpCLocDownStream, (LPCTSTR)m_strTypeBelow65, (LPCTSTR)m_strFamilyBelow65, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;
	}
	
	FillComboBox( &ListEx, pCCombo, iCount, pCstr );
}

void CDlgTPPageHC::FillComboBoxConnect( AboveOrBelow eAboveOrBelow )
{
	int iDN;
	CExtNumEditComboBox *pCCombo;
	CString *pCstr;
	CRankEx ListEx;
	int iCount;

	CTADatabase *pTADB = TASApp.GetpTADB();
	ASSERT( NULL != pTADB );

	CTableDN *pclTableDN = (CTableDN *)( pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	if( Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboConnectAbove50;
		pCstr = &m_strConnectAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboConnextBelow65;
		pCstr = &m_strConnectBelow65;
	}

	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetBVConnList( &ListEx, (LPCTSTR)m_strTypeAbove50, (LPCTSTR)m_strFamilyAbove50, (LPCTSTR)m_strBodyMaterialAbove50, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetBVConnList( &ListEx, (LPCTSTR)m_strTypeBelow65, (LPCTSTR)m_strFamilyBelow65, (LPCTSTR)m_strBodyMaterialBelow65, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetTaCVConnList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::LastCV2W3W,
													(LPCTSTR)m_strTypeAbove50,
													(LPCTSTR)m_strFamilyAbove50,
													(LPCTSTR)m_strBodyMaterialAbove50,
													CDB_ControlProperties::LastCVFUNC,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetTaCVConnList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::LastCV2W3W,
													(LPCTSTR)m_strTypeBelow65,
													(LPCTSTR)m_strFamilyBelow65,
													(LPCTSTR)m_strBodyMaterialBelow65,
													CDB_ControlProperties::LastCVFUNC,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetSmartControlValveConnList( &ListEx, (LPCTSTR)m_strTypeAbove50, (LPCTSTR)m_strFamilyAbove50, (LPCTSTR)m_strBodyMaterialAbove50, CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetSmartControlValveConnList( &ListEx, (LPCTSTR)m_strTypeBelow65, (LPCTSTR)m_strFamilyBelow65, (LPCTSTR)m_strBodyMaterialBelow65, CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_DpController:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetDpCConnList(	&ListEx,
												eDpCLoc::DpCLocDownStream,
												(LPCTSTR)m_strTypeAbove50,
												(LPCTSTR)m_strFamilyAbove50,
												(LPCTSTR)m_strBodyMaterialAbove50,
												CTADatabase::FilterSelection::ForHMCalc,
												iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetDpCConnList(	&ListEx,
												eDpCLoc::DpCLocDownStream,
												(LPCTSTR)m_strTypeBelow65,
												(LPCTSTR)m_strFamilyBelow65,
												(LPCTSTR)m_strBodyMaterialBelow65,
												CTADatabase::FilterSelection::ForHMCalc,
												0, iDN );
			}

			break;
	}
	
	FillComboBox( &ListEx, pCCombo, iCount, pCstr );
}

void CDlgTPPageHC::FillComboBoxVersion( AboveOrBelow eAboveOrBelow )
{
	int iDN;
	CExtNumEditComboBox *pCCombo;
	CString *pCstr;
	CRankEx ListEx;
	int iCount;

	CTADatabase *pTADB = TASApp.GetpTADB();
	ASSERT( NULL != pTADB );

	CTableDN *pclTableDN = (CTableDN *)( pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	if( Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboVersionAbove50;
		pCstr = &m_strVersionAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboVersionBelow65;
		pCstr = &m_strVersionBelow65;
	}

	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetBVVersList( &ListEx, (LPCTSTR)m_strTypeAbove50, (LPCTSTR)m_strFamilyAbove50, (LPCTSTR)m_strBodyMaterialAbove50, (LPCTSTR)m_strConnectAbove50, CTADatabase::FilterSelection::ForHMCalc );
			}
			else
			{
				iCount = pTADB->GetBVVersList( &ListEx, (LPCTSTR)m_strTypeBelow65, (LPCTSTR)m_strFamilyBelow65, (LPCTSTR)m_strBodyMaterialBelow65, (LPCTSTR)m_strConnectBelow65, CTADatabase::FilterSelection::ForHMCalc );
			}

			break;

		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetTaCVVersList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::LastCV2W3W,
													(LPCTSTR)m_strTypeAbove50,
													(LPCTSTR)m_strFamilyAbove50,
													(LPCTSTR)m_strBodyMaterialAbove50,
													(LPCTSTR)m_strConnectAbove50,
													CDB_ControlProperties::LastCVFUNC,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetTaCVVersList(	&ListEx,
													m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
													CDB_ControlProperties::LastCV2W3W,
													(LPCTSTR)m_strTypeBelow65,
													(LPCTSTR)m_strFamilyBelow65,
													(LPCTSTR)m_strBodyMaterialBelow65,
													(LPCTSTR)m_strConnectBelow65,
													CDB_ControlProperties::LastCVFUNC,
													CDB_ControlProperties::CvCtrlType::eCvNU,
													CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetSmartControlValveVersList( &ListEx, (LPCTSTR)m_strTypeAbove50, (LPCTSTR)m_strFamilyAbove50, (LPCTSTR)m_strBodyMaterialAbove50, (LPCTSTR)m_strConnectAbove50, CTADatabase::FilterSelection::ForHMCalc );
			}
			else
			{
				iCount = pTADB->GetSmartControlValveVersList( &ListEx, (LPCTSTR)m_strTypeBelow65, (LPCTSTR)m_strFamilyBelow65, (LPCTSTR)m_strBodyMaterialBelow65, (LPCTSTR)m_strConnectBelow65, CTADatabase::FilterSelection::ForHMCalc );
			}

			break;
		
		case ProductSubCategory::PSC_BC_DpController:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetDpCVersList(	&ListEx,
												eDpCLoc::DpCLocDownStream,
												(LPCTSTR)m_strTypeAbove50,
												(LPCTSTR)m_strFamilyAbove50,
												(LPCTSTR)m_strBodyMaterialAbove50,
												(LPCTSTR)m_strConnectAbove50,
												CTADatabase::FilterSelection::ForHMCalc );
			}
			else
			{
				iCount = pTADB->GetDpCVersList(	&ListEx,
												eDpCLoc::DpCLocDownStream,
												(LPCTSTR)m_strTypeBelow65,
												(LPCTSTR)m_strFamilyBelow65,
												(LPCTSTR)m_strBodyMaterialBelow65,
												(LPCTSTR)m_strConnectBelow65,
												CTADatabase::FilterSelection::ForHMCalc );
			}

			break;
	}

	FillComboBox( &ListEx, pCCombo, iCount, pCstr );
}

void CDlgTPPageHC::FillComboBoxPN( AboveOrBelow eAboveOrBelow )
{
	int iDN;
	CExtNumEditComboBox *pCCombo;
	CString *pCstr;
	CRankEx ListEx;
	int iCount;
	RemarkDN eDN;

	CTADatabase *pTADB = TASApp.GetpTADB();
	ASSERT( NULL != pTADB );

	CTableDN *pclTableDN = (CTableDN *)( pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	if( Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboPNAbove50;
		pCstr = &m_strPNAbove50;
		eDN = RemarkDN::DN65;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboPNBelow65;
		pCstr = &m_strPNBelow65;
		eDN = RemarkDN::DN50;
	}

	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetBVPNList(	&ListEx,
												(LPCTSTR)m_strTypeAbove50,
												(LPCTSTR)m_strFamilyAbove50,
												(LPCTSTR)m_strBodyMaterialAbove50,
												(LPCTSTR)m_strConnectAbove50,
												(LPCTSTR)m_strVersionAbove50,
												CTADatabase::FilterSelection::ForHMCalc );
			}
			else
			{
				iCount = pTADB->GetBVPNList(	&ListEx,
												(LPCTSTR)m_strTypeBelow65,
												(LPCTSTR)m_strFamilyBelow65,
												(LPCTSTR)m_strBodyMaterialBelow65,
												(LPCTSTR)m_strConnectBelow65,
												(LPCTSTR)m_strVersionBelow65,
												CTADatabase::FilterSelection::ForHMCalc );	
			}

			break;

		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetTaCVPNList(	&ListEx,
												m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
												CDB_ControlProperties::LastCV2W3W,
												(LPCTSTR)m_strTypeAbove50,
												(LPCTSTR)m_strFamilyAbove50,
												(LPCTSTR)m_strBodyMaterialAbove50,
												(LPCTSTR)m_strConnectAbove50,
												(LPCTSTR)m_strVersionAbove50,
												CDB_ControlProperties::LastCVFUNC,
												CDB_ControlProperties::CvCtrlType::eCvNU,
												CTADatabase::FilterSelection::ForHMCalc, iDN, INT_MAX );
			}
			else
			{
				iCount = pTADB->GetTaCVPNList(	&ListEx,
												m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
												CDB_ControlProperties::LastCV2W3W,
												(LPCTSTR)m_strTypeBelow65,
												(LPCTSTR)m_strFamilyBelow65,
												(LPCTSTR)m_strBodyMaterialBelow65,
												(LPCTSTR)m_strConnectBelow65,
												(LPCTSTR)m_strVersionBelow65,
												CDB_ControlProperties::LastCVFUNC,
												CDB_ControlProperties::CvCtrlType::eCvNU,
												CTADatabase::FilterSelection::ForHMCalc, 0, iDN );
			}

			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetSmartControlValvePNList(	&ListEx,
														(LPCTSTR)m_strTypeAbove50,
														(LPCTSTR)m_strFamilyAbove50,
														(LPCTSTR)m_strBodyMaterialAbove50,
														(LPCTSTR)m_strConnectAbove50,
														(LPCTSTR)m_strVersionAbove50,
														CTADatabase::FilterSelection::ForHMCalc );
			}
			else
			{
				iCount = pTADB->GetSmartControlValvePNList(	&ListEx,
														(LPCTSTR)m_strTypeBelow65,
														(LPCTSTR)m_strFamilyBelow65,
														(LPCTSTR)m_strBodyMaterialBelow65,
														(LPCTSTR)m_strConnectBelow65,
														(LPCTSTR)m_strVersionBelow65,
														CTADatabase::FilterSelection::ForHMCalc );	
			}

			break;
		
		case ProductSubCategory::PSC_BC_DpController:

			if( Above50 == eAboveOrBelow )
			{
				iCount = pTADB->GetDpCPNList( &ListEx, eDpCLoc::DpCLocDownStream, (LPCTSTR)m_strTypeAbove50, (LPCTSTR)m_strFamilyAbove50,
						(LPCTSTR)m_strBodyMaterialAbove50, (LPCTSTR)m_strConnectAbove50, (LPCTSTR)m_strVersionAbove50, 
						CTADatabase::FilterSelection::ForHMCalc );
			}
			else
			{
				iCount = pTADB->GetDpCPNList( &ListEx, eDpCLoc::DpCLocDownStream, (LPCTSTR)m_strTypeBelow65, (LPCTSTR)m_strFamilyBelow65,
						(LPCTSTR)m_strBodyMaterialBelow65, (LPCTSTR)m_strConnectBelow65, (LPCTSTR)m_strVersionBelow65, 
						CTADatabase::FilterSelection::ForHMCalc );
			}

			break;
	}

	FillComboBox( &ListEx, pCCombo, iCount, pCstr );
	
	// Display warning message about the DN range if needed.
	WarningMsgDN( eDN );

}

void CDlgTPPageHC::FillComboBox( CRankEx *pListEx, CExtNumEditComboBox *pCCombo, int iCount, CString *pCstr )
{
	_string str1;
	LPARAM lpParam;

	if( iCount > 0 && true == pListEx->GetFirst( str1, lpParam ) )
	{
		ASSERT( lpParam );
		pListEx->Transfer( pCCombo );
		int iNbre = pCCombo->GetCount();
		int iPos = -1;

		if( pCstr != NULL )
		{
			for( int i = 0; i < iNbre && -1 == iPos; i++ )
			{
				CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)pCCombo->GetItemData( i ) );

				if( pStrIDPointer != NULL && pStrIDPointer->GetIDPtr().ID == *pCstr )
				{
					pCCombo->SetCurSel( i );
					iPos = i;
				}
			}
		}
		
		if( -1 == iPos && iNbre != 0 )
		{
			pCCombo->SetCurSel( 0 );

			if( pCstr != NULL )
			{
				CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)pCCombo->GetItemData( 0 ) );
				*pCstr = pStrIDPointer->GetIDPtr().ID;
			}

		}
	}
	
	if( iCount < 2 )
	{
		pCCombo->EnableWindow( FALSE );
	}
	else
	{
		pCCombo->EnableWindow( TRUE );
	}
	
	pListEx->PurgeAll();
}

void CDlgTPPageHC::WarningMsgCV( RemarkDN eDN )
{
	if( ProductSubCategory::PSC_BC_ControlValve == GetProductSubCategory() )
	{
		// Variables.
		int iDNMin = 0;
		int iDNMax = INT_MAX;
		CString strType;
		CString str;
		CRankEx ListEx;

		CTableDN *pclTableDN = (CTableDN *)( TASApp.GetpTADB()->Get( _T("DN_TAB") ).MP );
		ASSERT( NULL != pclTableDN );
		
		// Set DNMIn and DNMax and Type used.
		if( RemarkDN::DN50 == eDN )
		{
			iDNMin = 0;
			iDNMax = pclTableDN->GetSize( _T("DN_50") );
			strType = m_strTypeBelow65;
		}
		else if( RemarkDN::DN65 == eDN )
		{
			iDNMin = pclTableDN->GetSize( _T("DN_65") );
			iDNMax = INT_MAX;
			strType = m_strTypeAbove50;
		}
		else
		{
			ASSERT( 0 );
		}

		// Verify control valves 2-way.
		int iCount2W = TASApp.GetpTADB()->GetTaCVFamList(	&ListEx,
															CTADatabase::eForHMCv,
															CDB_ControlProperties::CV2W3W::CV2W,
															(LPCTSTR)strType,
															CDB_ControlProperties::eCVFUNC::LastCVFUNC,
															CDB_ControlProperties::CvCtrlType::eCvNU,
															CTADatabase::FilterSelection::ForHMCalc, iDNMin, iDNMax );
		ListEx.PurgeAll();

		// Verify control valves 3-way.
		int iCount3W = TASApp.GetpTADB()->GetTaCVFamList(	&ListEx,
															CTADatabase::eForHMCv,
															CDB_ControlProperties::CV2W3W::CV3W,
															(LPCTSTR)strType,
															CDB_ControlProperties::eCVFUNC::LastCVFUNC,
															CDB_ControlProperties::CvCtrlType::eCvNU,
															CTADatabase::FilterSelection::ForHMCalc, iDNMin, iDNMax );
		ListEx.PurgeAll();

		// Verify control valves 3-way and 4-way.
		int iCount4W = TASApp.GetpTADB()->GetTaCVFamList(	&ListEx,
															CTADatabase::eForHMCv,
															CDB_ControlProperties::CV2W3W::CV4W,
															(LPCTSTR)strType,
															CDB_ControlProperties::eCVFUNC::LastCVFUNC,
															CDB_ControlProperties::CvCtrlType::eCvNU,
															CTADatabase::FilterSelection::ForHMCalc, iDNMin, iDNMax );
		// TODO for 6-way

		// Show necessary Warning Box.
		if( iCount2W > 0 && 0 == iCount3W && 0 == iCount4W )
		{
			str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKCV2W );
			ShowInfos( eDN, RemarkType::CV, true, str );
		}
		else if( 0 == iCount2W && iCount3W != 0 && 0 == iCount4W )
		{
			str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKCV3W );
			ShowInfos( eDN, RemarkType::CV, true, str);
		}
		else if( 0 == iCount2W && iCount3W != 0 && iCount4W != 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKCV4W );
			ShowInfos( eDN, RemarkType::CV, true, str );
		}
		else if( iCount2W != 0 && iCount3W != 0 && 0 == iCount4W )
		{
			ShowInfos( eDN, RemarkType::CV, false );
		}
	}
	else
	{
		ShowInfos( eDN, RemarkType::CV, false );
	}

}

void CDlgTPPageHC::WarningMsgDN( RemarkDN eDN )
{
	// TODO CDlgTPPageHC::WarningMsgDN(eRmkDn eDN) review DN bound, it should be linked dynamically to the product size available
	// TODO CDlgTPPageHC::WarningMsgDN(eRmkDn eDN) pass *pList as a parameter to avoid double DB pass trough
	// Variables
	int iDNMin = 0;
	int iDNMax = INT_MAX;
	int iLowerDN = 0;
	int iHigherDN = INT_MAX;
	CRankEx ListEx;
	CString strType;
	CString strFamily;
	CString strBodyMaterial;
	CString strConnection;
	CString strVersion;
	CString strPN;
	CString str;

	CTADatabase *pTADB = TASApp.GetpTADB();
	ASSERT( NULL != pTADB );

	CTableDN *pclTableDN = (CTableDN *)( TASApp.GetpTADB()->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );
		
	// Set 'Type', 'Family', 'BodyMaterial', 'Connection', 'Version', 'PN', 'DNMIn' and 'DNMax' used.
	if( RemarkDN::DN50 == eDN ) 
	{
		iDNMin = 0;
		iDNMax = pclTableDN->GetSize( _T("DN_50") );
		iLowerDN = pclTableDN->GetSize( _T("DN_15") );
		iHigherDN = iDNMax;
		strType = m_strTypeBelow65;
		strFamily = m_strFamilyBelow65;
		strBodyMaterial = m_strBodyMaterialBelow65;
		strConnection = m_strConnectBelow65;
		strVersion = m_strVersionBelow65;
		strPN = m_strPNBelow65;
	}
	else if( RemarkDN::DN65 == eDN )
	{
		iDNMin = pclTableDN->GetSize( _T("DN_65") );
		iDNMax = INT_MAX;
		iLowerDN = iDNMin;
		iHigherDN = pclTableDN->GetSize( _T("DN_400") );
		strType = m_strTypeAbove50;
		strFamily = m_strFamilyAbove50;
		strBodyMaterial	= m_strBodyMaterialAbove50;
		strConnection = m_strConnectAbove50;
		strVersion = m_strVersionAbove50;
		strPN = m_strPNAbove50;
	}
	else
	{
		ASSERT( 0 );
	}
	
	// Create the matching list of product.
	switch( GetProductSubCategory() )
	{
		case ProductSubCategory::PSC_BC_RegulatingValve:
			pTADB->GetBVList(	&ListEx,
								(LPCTSTR)strType,
								(LPCTSTR)strFamily,
								(LPCTSTR)strBodyMaterial,
								(LPCTSTR)strConnection,
								(LPCTSTR)strVersion,
								CTADatabase::FilterSelection::ForHMCalc,
								iDNMin, iDNMax );
			break;

		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:
			pTADB->GetTaCVList(	&ListEx,
								m_arTargetTab.GetAt( (int)GetProductSubCategory() ),
								false,
								CDB_ControlProperties::CV2W3W::LastCV2W3W,
								(LPCTSTR)strType,
								(LPCTSTR)strFamily,
								(LPCTSTR)strBodyMaterial,
								(LPCTSTR)strConnection,
								(LPCTSTR)strVersion,
								(LPCTSTR)strPN,
								CDB_ControlProperties::eCVFUNC::LastCVFUNC,
								CDB_ControlProperties::CvCtrlType::eCvNU,
								CTADatabase::FilterSelection::ForHMCalc, iDNMin, iDNMax );
			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:
			pTADB->GetSmartControlValveList(	&ListEx,
										(LPCTSTR)strType,
										(LPCTSTR)strFamily,
										(LPCTSTR)strBodyMaterial,
										(LPCTSTR)strConnection,
										(LPCTSTR)strVersion,
										(LPCTSTR)strPN,
										CTADatabase::FilterSelection::ForHMCalc,
										iDNMin, iDNMax );
			break;
		
		case ProductSubCategory::PSC_BC_DpController:
			pTADB->GetDpCList(	&ListEx,
								eDpCLoc::DpCLocDownStream,
								(LPCTSTR)strType,
								(LPCTSTR)strFamily,
								(LPCTSTR)strBodyMaterial,
								(LPCTSTR)strConnection,
								(LPCTSTR)strVersion,
								(LPCTSTR)strPN,
								CTADatabase::FilterSelection::ForHMCalc,
								iDNMin, iDNMax );
			break;
	}

	if( ListEx.GetCount() > 0 )
	{
		CDB_StringID *psDNMin = NULL;
		CDB_StringID *psDNMax = NULL;
		pTADB->GetTAProdDNRange( &ListEx, &psDNMin, &psDNMax );

		int iDNMin;
		psDNMin->GetIDstrAs<int>( 0, iDNMin );

		int iDNMax;
		psDNMax->GetIDstrAs<int>( 0, iDNMax );

		// Test if the DN range is between DN 15 - DN 50 or between DN 65 - DN 400.
		if( iDNMin > iLowerDN || iDNMax < iHigherDN )
		{
			str.Format( TASApp.LoadLocalizedString( IDS_TABDLGTECHHC_RMKDN ), (CString)psDNMin->GetString(), (CString)psDNMax->GetString() );
			ShowInfos( eDN, RemarkType::DN, true, str );
		}
		else
		{
			ShowInfos( eDN, RemarkType::DN, false );
		}

	}
	else
	{
		ShowInfos( eDN, RemarkType::DN, false );
	}
}
