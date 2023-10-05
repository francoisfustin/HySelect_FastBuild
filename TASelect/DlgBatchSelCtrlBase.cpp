#include "stdafx.h"
#include <errno.h>

#include "Taselect.h"
#include "MainFrm.h"

#include "ProductSelectionParameters.h"
#include "DlgBatchSelCtrlBase.h"

CDlgBatchSelCtrlBase::CDlgBatchSelCtrlBase( CBatchSelCtrlParamsBase &clBatchSelCtrlParams, UINT nID, CWnd *pParent )
		: CDlgBatchSelBase( clBatchSelCtrlParams, nID, pParent )
{
	m_pclBatchSelCtrlParams = &clBatchSelCtrlParams;

	m_mapLeftTabID2CvTargetTab[ProductSubCategory::PSC_BC_RegulatingValve] = CTADatabase::CvTargetTab::eLast;
	m_mapLeftTabID2CvTargetTab[ProductSubCategory::PSC_BC_ControlValve] = CTADatabase::CvTargetTab::eForHMCv;
	m_mapLeftTabID2CvTargetTab[ProductSubCategory::PSC_BC_BalAndCtrlValve] = CTADatabase::CvTargetTab::eForBCv;
	m_mapLeftTabID2CvTargetTab[ProductSubCategory::PSC_BC_PressureIndepCtrlValve] = CTADatabase::CvTargetTab::eForPiCv;
	m_mapLeftTabID2CvTargetTab[ProductSubCategory::PSC_BC_DpController] = CTADatabase::CvTargetTab::eLast;
	m_mapLeftTabID2CvTargetTab[ProductSubCategory::PSC_TC_ThermostaticValve] = CTADatabase::CvTargetTab::eLast;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelCtrlBase, CDlgBatchSelBase )
	ON_CBN_SELCHANGE( IDC_COMBOPOWERSUPPLY, OnCbnSelChangePowerSupply )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnCbnSelChangeInputSignal )
	ON_CBN_SELCHANGE( IDC_COMBODRP, OnCbnSelChangeDRPFct )
	ON_BN_CLICKED( IDC_CHECKFAILSAFE, OnBnClickedFailSafe )
END_MESSAGE_MAP()

void CDlgBatchSelCtrlBase::DoDataExchange( CDataExchange *pDX )
{
	CDlgBatchSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOPOWERSUPPLY, m_ComboPowerSupply );
	DDX_Control( pDX, IDC_COMBOINPUTSIGNAL, m_ComboInputSignal );
	DDX_Control( pDX, IDC_COMBODRP, m_ComboDRPFct );
	DDX_Control( pDX, IDC_CHECKFAILSAFE, m_CheckFailSafeFct );
	DDX_Control( pDX, IDC_GROUPACTUATOR, m_GroupActuator );
}

BOOL CDlgBatchSelCtrlBase::OnInitDialog()
{
	CDlgBatchSelBase::OnInitDialog();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_STATICPOWERSUPPLY );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_STATICINPUTSIGNAL );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_STATICFAILSAFE );
	GetDlgItem( IDC_CHECKFAILSAFE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
	GetDlgItem( IDC_STATICDRP )->SetWindowText( str );

	// Set proper style and add icons to groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupActuator.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Actuator );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupActuator.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Actuator, true );
	}

	m_GroupActuator.SetInOffice2007Mainframe( true );

	m_CheckStatus = CTADatabase::FailSafeCheckStatus::eFirst;

	return TRUE;
}

void CDlgBatchSelCtrlBase::OnCbnSelChangePowerSupply()
{
	if( m_pclBatchSelCtrlParams->m_strActuatorPowerSupplyID == m_ComboPowerSupply.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboPowerSupply.FindCBIDPtr( m_pclBatchSelCtrlParams->m_strActuatorPowerSupplyID ), 0 );

		if( iPos != m_ComboPowerSupply.GetCurSel() )
		{
			m_ComboPowerSupply.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelCtrlParams->m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;

	FillComboInputSignal();
	SetCheckFailSafeFct();
	FillComboDRPFct();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelCtrlBase::OnCbnSelChangeInputSignal()
{
	if( m_pclBatchSelCtrlParams->m_strActuatorInputSignalID == m_ComboInputSignal.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboInputSignal.FindCBIDPtr( m_pclBatchSelCtrlParams->m_strActuatorInputSignalID ), 0 );

		if( iPos != m_ComboInputSignal.GetCurSel() )
		{
			m_ComboInputSignal.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelCtrlParams->m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	SetCheckFailSafeFct();
	FillComboDRPFct();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelCtrlBase::OnCbnSelChangeDRPFct()
{
	if( m_pclBatchSelCtrlParams->m_eActuatorDRPFunction == (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( m_ComboDRPFct.GetCurSel() ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange( true ) )
	{
		int iPos = max( m_ComboDRPFct.FindItemDataPtr( m_pclBatchSelCtrlParams->m_eActuatorDRPFunction ), 0 );

		if( iPos != m_ComboDRPFct.GetCurSel() )
		{
			m_ComboDRPFct.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelCtrlParams->m_eActuatorDRPFunction = CDB_ControlValve::drpfUndefined;
	int iCur = m_ComboDRPFct.GetCurSel();

	if( iCur >= 0 )
	{
		m_pclBatchSelCtrlParams->m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( iCur );
	}

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelCtrlBase::OnBnClickedFailSafe()
{
	if( BST_CHECKED == m_CheckFailSafeFct.GetCheck() )
	{
		m_pclBatchSelCtrlParams->m_iActuatorFailSafeFunction = 1;
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_FAILSAFEPOSITION );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else if ( BST_UNCHECKED == m_CheckFailSafeFct.GetCheck() )
	{
		m_pclBatchSelCtrlParams->m_iActuatorFailSafeFunction = 0;
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	// HYS-1458 : Rename column
	RenameColHeader();
	SetCheckStatus( CTADatabase::FailSafeCheckStatus::eBothFailSafe );

	FillComboDRPFct();

	if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
	{
		// Ask him if he wants to delete current results.
		CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_BSCLEARRESULTS );

		if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
		{
			CheckIfWeNeedToClearResults();
		}
	}
}

void CDlgBatchSelCtrlBase::GetTypeList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == m_pclBatchSelCtrlParams || NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
			
			if( Above50 == eAboveOrBelow )
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVTypeList(	
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						CDB_ControlProperties::eCVFUNC::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel,
						m_iDN65, INT_MAX );
			}
			else
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVTypeList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						CDB_ControlProperties::eCVFUNC::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel,
						0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelCtrlBase::GetFamilyList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == m_pclBatchSelCtrlParams || NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
			
			if( Above50 == eAboveOrBelow )
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVFamList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
						CDB_ControlProperties::eCVFUNC::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						m_pclBatchSelCtrlParams->m_eFilterSelection, m_iDN65, INT_MAX );
			}
			else
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVFamList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
						CDB_ControlProperties::eCVFUNC::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						m_pclBatchSelCtrlParams->m_eFilterSelection, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelCtrlBase::GetMaterialList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == m_pclBatchSelCtrlParams || NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:

			if( Above50 == eAboveOrBelow )
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVBdyMatList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
						CDB_ControlProperties::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVBdyMatList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
						CDB_ControlProperties::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelCtrlBase::GetConnectList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == m_pclBatchSelCtrlParams || NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
			
			if( Above50 == eAboveOrBelow )
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVConnList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialAbove50ID,
						CDB_ControlProperties::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVConnList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialBelow65ID,
						CDB_ControlProperties::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelCtrlBase::GetVersionList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == m_pclBatchSelCtrlParams || NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
			
			if( Above50 == eAboveOrBelow )
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVVersList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectAbove50ID,
						CDB_ControlProperties::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVVersList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectBelow65ID,
						CDB_ControlProperties::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelCtrlBase::GetPNList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == m_pclBatchSelCtrlParams || NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_ControlValve:
		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
			
			if( Above50 == eAboveOrBelow )
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVPNList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectAbove50ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionAbove50ID,
						CDB_ControlProperties::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_pclBatchSelCtrlParams->m_pTADB->GetTaCVPNList(
						pclRankEx,
						m_mapLeftTabID2CvTargetTab[m_pclBatchSelCtrlParams->m_eProductSubCategory],
						m_pclBatchSelCtrlParams->m_eCV2W3W,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectBelow65ID,
						(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionBelow65ID,
						CDB_ControlProperties::LastCVFUNC,
						m_pclBatchSelCtrlParams->m_eCvCtrlType,
						CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelCtrlBase::EndOfComboChange()
{
	CDlgBatchSelBase::EndOfComboChange();

	// Change all combos concerning actuator.
	FillComboPowerSupply();
	FillComboInputSignal();
	SetCheckFailSafeFct();
	FillComboDRPFct();
}

void CDlgBatchSelCtrlBase::ActivateLeftTabDialog()
{
	CDlgBatchSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_pclBatchSelCtrlParams->m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_pclBatchSelCtrlParams->m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

void CDlgBatchSelCtrlBase::FillComboPowerSupply( CString strPowerSupplyID )
{
	CRankEx ControlValveList;
	CTADatabase::CvTargetTab eCvTarget = CTADatabase::CvTargetTab::eLast;

	if( ProductSubCategory::PSC_BC_BalAndCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForTechHCBCv;
	}
	else if( ProductSubCategory::PSC_BC_PressureIndepCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForPiCv;
	}
	else if( ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForDpCBCV;
	}
	else if( ProductSubCategory::PSC_BC_ControlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForSSelCv;
	}

	if( CTADatabase::CvTargetTab::eLast == eCvTarget )
	{
		return;
	}

	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(	
		&ControlValveList,													// List where to saved
		eCvTarget,															// Control valve target
		false, 																// 'true' returns as soon a result is found
		CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNBelow65ID,
		CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
		m_pclBatchSelCtrlParams->m_eFilterSelection,
		0,																	// DNMin
		INT_MAX,															// DNMax
		false,																// 'true' if it's for hub station.
		NULL,																// 'pProd'.
		false );															// 'true' if it's only for a set.

	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(	
		&ControlValveList,													// List where to saved
		eCvTarget,															// Control valve target
		false, 																// 'true' returns as soon a result is found
		CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNAbove50ID,
		CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
		m_pclBatchSelCtrlParams->m_eFilterSelection,
		0,																	// DNMin
		INT_MAX,															// DNMax
		false,																// 'true' if it's for hub station.
		NULL,																// 'pProd'.
		false );															// 'true' if it's only for a set.

	CRankEx PowerSupplyList;
	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVActuatorPowerSupplyList( &PowerSupplyList, &ControlValveList, m_pclBatchSelCtrlParams->m_eCvCtrlType, false, m_pclBatchSelCtrlParams->m_eFilterSelection );
	m_ComboPowerSupply.FillInCombo( &PowerSupplyList, strPowerSupplyID );
	m_pclBatchSelCtrlParams->m_strActuatorPowerSupplyID = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
}

void CDlgBatchSelCtrlBase::FillComboInputSignal( CString strInputSignalID )
{
	CRankEx ControlValveList;

	CTADatabase::CvTargetTab eCvTarget = CTADatabase::CvTargetTab::eLast;

	if( ProductSubCategory::PSC_BC_BalAndCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForTechHCBCv;
	}
	else if( ProductSubCategory::PSC_BC_PressureIndepCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForPiCv;
	}
	else if( ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForDpCBCV;
	}
	else if( ProductSubCategory::PSC_BC_ControlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForSSelCv;
	}

	if( CTADatabase::CvTargetTab::eLast == eCvTarget )
	{
		return;
	}

	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(
		&ControlValveList,													// List where to saved
		eCvTarget,															// Control valve target
		false, 																// 'true' returns as soon a result is found
		CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNBelow65ID,
		CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
		m_pclBatchSelCtrlParams->m_eFilterSelection,
		0,																	// DNMin
		INT_MAX,															// DNMax
		false,																// 'true' if it's for hub station.
		NULL,																// 'pProd'.
		false );															// 'true' if it's only for a set.

	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(	
		&ControlValveList,													// List where to saved
		eCvTarget,															// Control valve target
		false, 																// 'true' returns as soon a result is found
		CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNAbove50ID,
		CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
		m_pclBatchSelCtrlParams->m_eFilterSelection,
		0,																	// DNMin
		INT_MAX,															// DNMax
		false,																// 'true' if it's for hub station.
		NULL,																// 'pProd'.
		false );															// 'true' if it's only for a set.

	CRankEx InputSignalList;

	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVActuatorInputSignalList( &InputSignalList, &ControlValveList, m_pclBatchSelCtrlParams->m_eCvCtrlType, false, 
			m_ComboPowerSupply.GetCBCurSelIDPtr().ID, m_pclBatchSelCtrlParams->m_eFilterSelection );

	m_ComboInputSignal.FillInCombo( &InputSignalList, strInputSignalID );

	m_pclBatchSelCtrlParams->m_strActuatorInputSignalID = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
}

void CDlgBatchSelCtrlBase::SetCheckFailSafeFct()
{
	CTADatabase::CvTargetTab eCvTarget = CTADatabase::CvTargetTab::eLast;

	if( ProductSubCategory::PSC_BC_BalAndCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForTechHCBCv;
	}
	else if( ProductSubCategory::PSC_BC_PressureIndepCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForPiCv;
	}
	else if( ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForDpCBCV;
	}
	else if( ProductSubCategory::PSC_BC_ControlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForSSelCv;
	}

	if( CTADatabase::CvTargetTab::eLast == eCvTarget )
	{
		return;
	}
	if( 0 == m_ComboInputSignal.GetCount() )
	{
		// Checkbox unchecked and disabled
		m_CheckFailSafeFct.SetCheck( BST_UNCHECKED );
		m_CheckFailSafeFct.EnableWindow( false );
	}
	else
	{
		CRankEx ControlValveList;

		m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(
			&ControlValveList,													// List where to saved
			eCvTarget,															// Control valve target
			false, 																// 'true' returns as soon a result is found
			CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialBelow65ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectBelow65ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionBelow65ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNBelow65ID,
			CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
			m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
			m_pclBatchSelCtrlParams->m_eFilterSelection,
			0,																	// DNMin
			INT_MAX,															// DNMax
			false,																// 'true' if it's for hub station.
			NULL,																// 'pProd'.
			false );															// 'true' if it's only for a set.

		m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(
			&ControlValveList,													// List where to saved
			eCvTarget,															// Control valve target
			false, 																// 'true' returns as soon a result is found
			CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialAbove50ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectAbove50ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionAbove50ID,
			(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNAbove50ID,
			CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
			m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
			m_pclBatchSelCtrlParams->m_eFilterSelection,
			0,																	// DNMin
			INT_MAX,															// DNMax
			false,																// 'true' if it's for hub station.
			NULL,																// 'pProd'.
			false );															// 'true' if it's only for a set.

		m_CheckStatus = m_pclBatchSelCtrlParams->m_pTADB->GetTaCVActuatorFailSafeValues( &ControlValveList, m_pclBatchSelCtrlParams->m_eCvCtrlType, false, m_ComboPowerSupply.GetCBCurSelIDPtr().ID,
			m_ComboInputSignal.GetCBCurSelIDPtr().ID, m_pclBatchSelCtrlParams->m_eFilterSelection );
		if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus )
		{
			// Checkbox unchecked and disabled
			m_CheckFailSafeFct.SetCheck( BST_UNCHECKED );
			m_CheckFailSafeFct.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithoutFailSafe == m_CheckStatus )
		{
			// Checkbox unchecked and enable
			m_CheckFailSafeFct.SetCheck( BST_UNCHECKED );
			m_CheckFailSafeFct.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithFailSafe == m_CheckStatus )
		{
			// Checkbox checked and enable
			m_CheckFailSafeFct.SetCheck( BST_CHECKED );
			m_CheckFailSafeFct.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eBothFailSafe == m_CheckStatus )
		{
			// Checkbox checked and enable
			m_CheckFailSafeFct.SetCheck( BST_UNCHECKED );
			m_CheckFailSafeFct.EnableWindow( true );
		}
		// HYS-1079 : Update failsafe value if a combo above is changed
		m_pclBatchSelCtrlParams->m_iActuatorFailSafeFunction = ( BST_CHECKED == m_CheckFailSafeFct.GetCheck() ) ? 1 : 0;
	}
	// HYS-1458 : update static
	if( BST_CHECKED == m_CheckFailSafeFct.GetCheck() )
	{
		m_pclBatchSelCtrlParams->m_iActuatorFailSafeFunction = 1;
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_FAILSAFEPOSITION );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else if( BST_UNCHECKED == m_CheckFailSafeFct.GetCheck() )
	{
		m_pclBatchSelCtrlParams->m_iActuatorFailSafeFunction = 0;
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	RenameColHeader();
}

void CDlgBatchSelCtrlBase::FillComboDRPFct( CDB_ControlValve::DRPFunction eDRPFct )
{
	CRankEx ControlValveList;

	CTADatabase::CvTargetTab eCvTarget = CTADatabase::CvTargetTab::eLast;

	if( ProductSubCategory::PSC_BC_BalAndCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForTechHCBCv;
	}
	else if( ProductSubCategory::PSC_BC_PressureIndepCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForPiCv;
	}
	else if( ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForDpCBCV;
	}
	else if( ProductSubCategory::PSC_BC_ControlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForSSelCv;
	}

	if( CTADatabase::CvTargetTab::eLast == eCvTarget )
	{
		return;
	}

	if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus && false == m_CheckFailSafeFct.IsWindowEnabled() )
	{
		m_ComboDRPFct.ResetContent();
		m_ComboDRPFct.EnableWindow( FALSE );
		m_pclBatchSelCtrlParams->m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfUndefined;
		return;
	}

	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(	
		&ControlValveList,													// List where to saved
		eCvTarget,															// Control valve target
		false, 																// 'true' returns as soon a result is found
		CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNBelow65ID,
		CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
		m_pclBatchSelCtrlParams->m_eFilterSelection,
		0,																	// DNMin
		INT_MAX,															// DNMax
		false,																// 'true' if it's for hub station.
		NULL,																// 'pProd'.
		false );															// 'true' if it's only for a set.

	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(	
		&ControlValveList,													// List where to saved
		eCvTarget,															// Control valve target
		false, 																// 'true' returns as soon a result is found
		CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNAbove50ID,
		CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
		m_pclBatchSelCtrlParams->m_eFilterSelection,
		0,																	// DNMin
		INT_MAX,															// DNMax
		false,																// 'true' if it's for hub station.
		NULL,																// 'pProd'.
		false );															// 'true' if it's only for a set.

	CRankEx DRPFctList;
	ASSERT( m_CheckFailSafeFct.GetCheck() != -1 );
	int iFailSafe =  m_CheckFailSafeFct.GetCheck();
	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVDRPFunctionList( &DRPFctList, m_pclBatchSelCtrlParams->m_eCvCtrlType, &ControlValveList, false, m_ComboPowerSupply.GetCBCurSelIDPtr().ID,
		m_ComboInputSignal.GetCBCurSelIDPtr().ID, iFailSafe, m_pclBatchSelCtrlParams->m_eFilterSelection );

	m_ComboDRPFct.ResetContent();
	int iSelPos = 0;

	if( DRPFctList.GetCount() > 0 )
	{
		DRPFctList.Transfer( &m_ComboDRPFct );

		if( m_ComboDRPFct.GetCount() > 1 && eDRPFct >= CDB_ControlValve::DRPFunction::drpfNone
			&& eDRPFct < CDB_ControlValve::DRPFunction::drpfLast )
		{
			for( int i = 0; i < m_ComboDRPFct.GetCount(); i++ )
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
	m_pclBatchSelCtrlParams->m_eActuatorDRPFunction = (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( iSelPos );

	m_ComboDRPFct.EnableWindow( ( m_ComboDRPFct.GetCount() <= 1 ) ? FALSE : TRUE );
}

void CDlgBatchSelCtrlBase::VerifyCheckboxFailsafeStatus()
{
	CRankEx ControlValveList;
	CTADatabase::CvTargetTab eCvTarget = CTADatabase::CvTargetTab::eLast;

	if( ProductSubCategory::PSC_BC_BalAndCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForTechHCBCv;
	}
	else if( ProductSubCategory::PSC_BC_PressureIndepCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForPiCv;
	}
	else if( ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForDpCBCV;
	}
	else if( ProductSubCategory::PSC_BC_ControlValve == m_pclBatchSelCtrlParams->m_eProductSubCategory )
	{
		eCvTarget = CTADatabase::CvTargetTab::eForSSelCv;
	}

	if( CTADatabase::CvTargetTab::eLast == eCvTarget )
	{
		return;
	}
	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(
		&ControlValveList,													// List where to saved
		eCvTarget,															// Control valve target
		false, 																// 'true' returns as soon a result is found
		CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionBelow65ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNBelow65ID,
		CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
		m_pclBatchSelCtrlParams->m_eFilterSelection,
		0,																	// DNMin
		INT_MAX,															// DNMax
		false,																// 'true' if it's for hub station.
		NULL,																// 'pProd'.
		false );															// 'true' if it's only for a set.

	m_pclBatchSelCtrlParams->m_pTADB->GetTaCVList(
		&ControlValveList,													// List where to saved
		eCvTarget,															// Control valve target
		false, 																// 'true' returns as soon a result is found
		CDB_ControlProperties::LastCV2W3W, 									// Set way number of valve
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboTypeAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboFamilyAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboMaterialAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboConnectAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboVersionAbove50ID,
		(LPCTSTR)m_pclBatchSelCtrlParams->m_strComboPNAbove50ID,
		CDB_ControlProperties::eCVFUNC::LastCVFUNC, 						// Set the control function (control only, presettable, ...)
		m_pclBatchSelCtrlParams->m_eCvCtrlType,								// Set the control type (on/off, proportional, ...)
		m_pclBatchSelCtrlParams->m_eFilterSelection,
		0,																	// DNMin
		INT_MAX,															// DNMax
		false,																// 'true' if it's for hub station.
		NULL,																// 'pProd'.
		false );

	m_CheckStatus = m_pclBatchSelCtrlParams->m_pTADB->GetTaCVActuatorFailSafeValues( &ControlValveList, m_pclBatchSelCtrlParams->m_eCvCtrlType, false, m_ComboPowerSupply.GetCBCurSelIDPtr().ID,
		m_ComboInputSignal.GetCBCurSelIDPtr().ID, m_pclBatchSelCtrlParams->m_eFilterSelection );
	if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckStatus ||
		CTADatabase::FailSafeCheckStatus::eOnlyWithoutFailSafe == m_CheckStatus ||
		CTADatabase::FailSafeCheckStatus::eOnlyWithFailSafe == m_CheckStatus )
	{
		m_CheckFailSafeFct.EnableWindow( false );
	}
	else if( CTADatabase::FailSafeCheckStatus::eBothFailSafe == m_CheckStatus )
	{
		m_CheckFailSafeFct.EnableWindow( true );
	}

	if( BST_CHECKED == m_CheckFailSafeFct.GetCheck() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_FAILSAFEPOSITION );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	// HYS-1458 : Rename column
	RenameColHeader();
}