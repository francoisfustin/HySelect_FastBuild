#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "Select.h"

#include "DlgLeftTabSelManager.h"

#include "ProductSelectionParameters.h"

#include "RViewSSelCtrl.h"
#include "RViewSSelPICv.h"

#include "DlgBatchSelCtrlBase.h"
#include "DlgBatchSelPICv.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelPICv::CDlgBatchSelPICv( CWnd *pParent )
	: CDlgBatchSelCtrlBase( m_clBatchSelPIBCVParams, CDlgBatchSelPICv::IDD, pParent )
{
	m_clBatchSelPIBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_PressureIndepCtrlValve;
	m_clBatchSelPIBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	m_pclSelComboHelperPIBCV = new CDlgSelComboHelperPIBCV( &m_clBatchSelPIBCVParams, CDlgSelComboHelperBase::SelType_Batch );

	if( NULL == m_pclSelComboHelperPIBCV )
	{
		ASSERT( 0 );
	}
}

CDlgBatchSelPICv::~CDlgBatchSelPICv()
{
	if( NULL != m_pclSelComboHelperPIBCV )
	{
		delete m_pclSelComboHelperPIBCV;
	}
}

void CDlgBatchSelPICv::SaveSelectionParameters()
{
	CDlgBatchSelCtrlBase::SaveSelectionParameters();

	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelPIBCVParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return;
	}

	pclBatchSelParameter->SetPICvFlowRadioState( (int)m_clBatchSelPIBCVParams.m_eFlowOrPowerDTMode );
	pclBatchSelParameter->SetPICvCtrlTypeStrictChecked( m_clBatchSelPIBCVParams.m_bIsCtrlTypeStrictChecked );
	pclBatchSelParameter->SetPICvCBCtrlType( m_clBatchSelPIBCVParams.m_eCvCtrlType );
	
	pclBatchSelParameter->SetPICvTypeBelow65ID( m_clBatchSelPIBCVParams.m_strComboTypeBelow65ID );
	pclBatchSelParameter->SetPICvFamilyBelow65ID( m_clBatchSelPIBCVParams.m_strComboFamilyBelow65ID );
	pclBatchSelParameter->SetPICvMaterialBelow65ID( m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID );
	pclBatchSelParameter->SetPICvConnectBelow65ID( m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID );
	pclBatchSelParameter->SetPICvVersionBelow65ID( m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID );
	pclBatchSelParameter->SetPICvPNBelow65ID( m_clBatchSelPIBCVParams.m_strComboPNBelow65ID );
	
	pclBatchSelParameter->SetPICvTypeAbove50ID( m_clBatchSelPIBCVParams.m_strComboTypeAbove50ID );
	pclBatchSelParameter->SetPICvFamilyAbove50ID( m_clBatchSelPIBCVParams.m_strComboFamilyAbove50ID );
	pclBatchSelParameter->SetPICvMaterialAbove50ID( m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID );
	pclBatchSelParameter->SetPICvConnectAbove50ID( m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID );
	pclBatchSelParameter->SetPICvVersionAbove50ID( m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID );
	pclBatchSelParameter->SetPICvPNAbove50ID( m_clBatchSelPIBCVParams.m_strComboPNAbove50ID );
	
	pclBatchSelParameter->SetPICvActPowerSupplyID( m_clBatchSelPIBCVParams.m_strActuatorPowerSupplyID );
	pclBatchSelParameter->SetPICvActInputSignalID( m_clBatchSelPIBCVParams.m_strActuatorInputSignalID );
	pclBatchSelParameter->SetPICvFailSafeFct( m_clBatchSelPIBCVParams.m_iActuatorFailSafeFunction );
	pclBatchSelParameter->SetPICvDRPFct( m_clBatchSelPIBCVParams.m_eActuatorDRPFunction );

	BS_WriteAllColumnWidth( pclBatchSelParameter );
}

bool CDlgBatchSelPICv::ResetRightView()
{
	if( NULL == pRViewSSelPICv )
	{
		return false;
	}

	if( true == pRViewSSelPICv->IsEmpty() )
	{
		return true;
	}

	pRViewSSelPICv->Reset();
	return true;
}

bool CDlgBatchSelPICv::OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return false;
	}

	mapColumnList.clear();

	if( 0 == m_mapColumnList.size() )
	{
		return true;
	}

	int iInsert = 0;

	for( int iLoopColumn = BS_PICVCID_First; iLoopColumn < BS_PICVCID_Last; iLoopColumn++ )
	{
		CDlgBatchSelectionOutput::ColData rColData;

		if( BS_PICVCID_Name == iLoopColumn || BS_PICVCID_ActrName == iLoopColumn || BS_PICVCID_ActrAdapterName == iLoopColumn )
		{
			// Insert before the control valve title and a separator.
			rColData.m_lColumnID = -1;

			rColData.m_strName = TASApp.LoadLocalizedString( ( BS_PICVCID_Name == iLoopColumn ) ? IDS_DLGBATCHSELPICV_CTXTMENUVALVEOUTPUT :
					IDS_DLGBATCHSELPICV_CTXTMENUACTROUTPUT );
			
			rColData.m_bAlreadyDisplayed = false;
			rColData.m_bEnable = false;
			rColData.m_bSeparator = false;
			mapColumnList[iInsert++] = rColData;

			rColData.m_lColumnID = -1;
			rColData.m_strName = _T( "" );
			rColData.m_bAlreadyDisplayed = false;
			rColData.m_bEnable = false;
			rColData.m_bSeparator = true;
			mapColumnList[iInsert++] = rColData;
		}

		if( true == m_mapColumnList[iLoopColumn].m_bCantRemove )
		{
			continue;
		}

		rColData.m_lColumnID = iLoopColumn;

		if( BS_PICVColumnID::BS_PICVCID_ImgCharacteristic == iLoopColumn || BS_PICVColumnID::BS_PICVCID_ImgPushClose == iLoopColumn )
		{
			if( BS_PICVColumnID::BS_PICVCID_ImgCharacteristic == iLoopColumn )
			{
				rColData.m_strName = TASApp.LoadLocalizedString( IDS_DLGBATCHSELPICV_CTXTMENUIMGCHAR );
			}
			else
			{
				rColData.m_strName = TASApp.LoadLocalizedString( IDS_DLGBATCHSELPICV_CTXTMENUIMGPUSHCLOSE );
			}
		}
		else
		{
			rColData.m_strName = m_mapColumnList[iLoopColumn].m_strHeaderName;
		}

		rColData.m_bAlreadyDisplayed = m_mapColumnList[iLoopColumn].m_bDisplayed;
		rColData.m_bEnable = true;
		rColData.m_bSeparator = false;
		mapColumnList[iInsert++] = rColData;
	}

	return true;
}

bool CDlgBatchSelPICv::OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct )
{
	if( false == CDlgBatchSelCtrlBase::OnBatchSelectionIsCatalogExist( iColumnID, lRowRelative, bCatExist, pclProduct ) )
	{
		return false;
	}

	if( iColumnID >= BS_PICVColumnID::BS_PICVCID_Name && iColumnID < BS_PICVColumnID::BS_PICVCID_PipeSize 
			&& NULL != m_mapAllRowData[lRowRelative].m_pclSelectedProduct )
	{
		// Check valve.
		CDB_TAProduct *pclTAProduct = ( CDB_TAProduct * )( m_mapAllRowData[lRowRelative].m_pclSelectedProduct->GetProductIDPtr().MP );
		bCatExist = ( true == pclTAProduct->VerifyOneCatExist() );
		pclProduct = pclTAProduct;
	}
	else if( iColumnID >= BS_PICVColumnID::BS_PICVCID_ActrName && iColumnID < BS_PICVColumnID::BS_PICVCID_Last &&
			 NULL != m_mapAllRowData[lRowRelative].m_pclSelectedActuator )
	{
		// Check actuator.
		CDB_Actuator *pclActuator = m_mapAllRowData[lRowRelative].m_pclSelectedActuator;
		bCatExist = ( true == pclActuator->VerifyOneCatExist() );
		pclProduct = pclActuator;
	}

	return true;
}

bool CDlgBatchSelPICv::OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelPICv )
	{
		ASSERTA_RETURN( false );
	}

	if( false == CDlgBatchSelCtrlBase::OnDlgOutputCellDblClicked( iOutputID, iColumnID, lRowRelative ) )
	{
		return false;
	}

	if( BS_ValidationStatus::BS_VS_Done == m_eValidationStatus )
	{
		return false;
	}

	// If no data available on this row (can be the case when user double click on the last blank line).
	if( false == m_clInterface.IsRowValidForData( lRowRelative, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory ) )
	{
		return false;
	}

	if( RowStatus::BS_RS_NotYetDone == m_mapAllRowData[lRowRelative].m_eStatus 
			|| RowStatus::BS_RS_FindNoSolution == m_mapAllRowData[lRowRelative].m_eStatus )
	{
		return false;
	}

	if( false == _CopyBatchSelParamsToIndSelParams( &m_mapAllRowData[lRowRelative] ) )
	{
	}

	m_pclCurrentRowParameters = &m_mapAllRowData[lRowRelative];

	// HYS-1168 : We active the autohide mode of the DockablePane in edition mode.
	CToolsDockablePane* pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	if( NULL != pclToolsDockablePane && TRUE == pclToolsDockablePane->IsWindowVisible() )
	{
		pclToolsDockablePane->SetAutoHideMode( TRUE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}
	// Change the tools dockable pane to no full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( false, true );

	pRViewSSelPICv->Invalidate();
	pRViewSSelPICv->UpdateWindow();
	pRViewSSelPICv->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );

	m_clInterface.SelectRow( lRowRelative, true, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	_BS_DisplaySolutions();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelPICv, CDlgBatchSelCtrlBase )
	ON_CBN_SELCHANGE( IDC_COMBOCTRLTYPE, OnCbnSelChangeCtrlType )
	ON_COMMAND( IDC_CHECKSTRICT, OnBnClickedCheckStrict )
END_MESSAGE_MAP()

void CDlgBatchSelPICv::RenameColHeader()
{
	// Update Default result position column
	if( m_clBatchSelPIBCVParams.m_iActuatorFailSafeFunction == 1 )
	{
		m_mapColumnList[BS_PICVColumnID::BS_PICVCID_ActrDRP].m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP );
	}
	else
	{
		m_mapColumnList[BS_PICVColumnID::BS_PICVCID_ActrDRP].m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
	}

	m_clInterface.FillColumnHeader( BS_PICVColumnID::BS_PICVCID_ActrDRP, BS_RowRelativePos::BS_RID_HeaderName,
		m_mapColumnList[BS_PICVColumnID::BS_PICVCID_ActrDRP].m_strHeaderName,
		DLBBATCHSELBASE_ROWHEIGHT_HEADERNAME, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
}

void CDlgBatchSelPICv::DoDataExchange( CDataExchange *pDX )
{
	CDlgBatchSelCtrlBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOCTRLTYPE, m_ComboCtrlType );
	DDX_Control( pDX, IDC_CHECKSTRICT, m_clCheckboxStrict );
}

BOOL CDlgBatchSelPICv::OnInitDialog()
{
	CDlgBatchSelCtrlBase::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELPICV_STATICCTRLTYPE );
	GetDlgItem( IDC_STATICCTRLTYPE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELPIBCV_CHECKSTRICT );
	m_clCheckboxStrict.SetWindowText( str );
	m_clCheckboxStrict.SetCheck( BST_UNCHECKED );

	str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELPICV_STATICPOWERSUPPLY );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELPICV_STATICINPUTSIGNAL );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELPICV_STATICFAILSAFE );
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

	return TRUE;
}

void CDlgBatchSelPICv::OnCbnSelChangeCtrlType()
{
	if( m_clBatchSelPIBCVParams.m_eCvCtrlType == (CDB_ControlProperties::CvCtrlType)m_pclSelComboHelperPIBCV->GetIntFromCombo( &m_ComboCtrlType ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataInt( &m_ComboCtrlType, ( DWORD_PTR )m_clBatchSelPIBCVParams.m_eCvCtrlType ), 0 );

		if( iPos != m_ComboCtrlType.GetCurSel() )
		{
			m_ComboCtrlType.SetCurSel( iPos );
		}

		return;
	}
	
	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboCtrlType );
	
	m_clBatchSelPIBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;		// By default in case of error
	int iCur = m_ComboCtrlType.GetCurSel();

	if( iCur >= 0 )
	{
		m_clBatchSelPIBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)m_pclSelComboHelperPIBCV->GetIntFromCombo( &m_ComboCtrlType, iCur );
	}

	// Change all combos for size below 65.
	m_pclSelComboHelperPIBCV->FillComboType( &m_ComboTypeBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamilyBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboBodyMaterialBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboVersion(  &m_ComboVersionBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNBelow65, CDlgSelComboHelperBase::CS_Below65 );

	m_clBatchSelPIBCVParams.m_strComboTypeBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboTypeBelow65 );
	m_clBatchSelPIBCVParams.m_strComboFamilyBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyBelow65 );
	m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialBelow65 );
	m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectBelow65 );
	m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionBelow65 );
	m_clBatchSelPIBCVParams.m_strComboPNBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 );

	// Change all combos for size above 50.
	m_pclSelComboHelperPIBCV->FillComboType( &m_ComboTypeAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamilyAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboBodyMaterialAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboVersion(  &m_ComboVersionAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNAbove50, CDlgSelComboHelperBase::CS_Above50 );

	m_clBatchSelPIBCVParams.m_strComboTypeAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboTypeAbove50 );
	m_clBatchSelPIBCVParams.m_strComboFamilyAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyAbove50 );
	m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialAbove50 );
	m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectAbove50 );
	m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionAbove50 );
	m_clBatchSelPIBCVParams.m_strComboPNAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	// Reset to 'false' to allow to clear and suggest new results.
	m_bMsgDoNotClearResult = false;
	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnBnClickedCheckStrict()
{
	if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
	{
		// Ask him if he wants to delete current results.
		CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_BSCLEARRESULTS );

		if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
		{
			OnBatchSelectionButtonClearResults();
			OnBnClickedSuggest();
		}
		else
		{
			m_clCheckboxStrict.SetCheck( ( BST_CHECKED == m_clCheckboxStrict.GetCheck() ) ? BST_UNCHECKED : BST_CHECKED );
		}
	}

	m_clBatchSelPIBCVParams.m_bIsCtrlTypeStrictChecked = ( BST_CHECKED == m_clCheckboxStrict.GetCheck() ) ? true : false;
}

void CDlgBatchSelPICv::OnCbnSelChangeTypeBelow65()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboTypeBelow65ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboTypeBelow65 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboTypeBelow65, m_clBatchSelPIBCVParams.m_strComboTypeBelow65ID ), 0 );

		if( iPos != m_ComboTypeBelow65.GetCurSel() )
		{
			m_ComboTypeBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboTypeBelow65 );
	m_clBatchSelPIBCVParams.m_strComboTypeBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboTypeBelow65 );

	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamilyBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboBodyMaterialBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNBelow65, CDlgSelComboHelperBase::CS_Below65 );

	m_clBatchSelPIBCVParams.m_strComboFamilyBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyBelow65 );
	m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialBelow65 );
	m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectBelow65 );
	m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionBelow65 );
	m_clBatchSelPIBCVParams.m_strComboPNBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeFamilyBelow65()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboFamilyBelow65ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyBelow65 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboFamilyBelow65, m_clBatchSelPIBCVParams.m_strComboFamilyBelow65ID ), 0 );

		if( iPos != m_ComboFamilyBelow65.GetCurSel() )
		{
			m_ComboFamilyBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboFamilyBelow65 );

	m_clBatchSelPIBCVParams.m_strComboFamilyBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyBelow65 );

	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboBodyMaterialBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNBelow65, CDlgSelComboHelperBase::CS_Below65 );

	m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialBelow65 );
	m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectBelow65 );
	m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionBelow65 );
	m_clBatchSelPIBCVParams.m_strComboPNBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeBdyMatBelow65()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialBelow65 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboBodyMaterialBelow65, m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID ), 0 );

		if( iPos != m_ComboBodyMaterialBelow65.GetCurSel() )
		{
			m_ComboBodyMaterialBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboBodyMaterialBelow65 );

	m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialBelow65 );

	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNBelow65, CDlgSelComboHelperBase::CS_Below65 );

	m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectBelow65 );
	m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionBelow65 );
	m_clBatchSelPIBCVParams.m_strComboPNBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeConnectBelow65()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectBelow65 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboConnectBelow65, m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID ), 0 );

		if( iPos != m_ComboConnectBelow65.GetCurSel() )
		{
			m_ComboConnectBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboConnectBelow65 );

	m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectBelow65 );

	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNBelow65, CDlgSelComboHelperBase::CS_Below65 );

	m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionBelow65 );
	m_clBatchSelPIBCVParams.m_strComboPNBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeVersionBelow65()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionBelow65 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboVersionBelow65, m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID ), 0 );

		if( iPos != m_ComboVersionBelow65.GetCurSel() )
		{
			m_ComboVersionBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboVersionBelow65 );

	m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionBelow65 );

	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNBelow65, CDlgSelComboHelperBase::CS_Below65 );
	m_clBatchSelPIBCVParams.m_strComboPNBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangePNBelow65()
{
	if( m_clBatchSelPIBCVParams.m_strComboPNBelow65ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// Remark: 'true' to signal that it is the last combo in the chain to be changed.
	//         It allows us to reset the 'm_prBatchSelectionVariables->m_fMsgComboChangeDone' variable to false.
	if( false == BS_ComboChange( true ) )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboPNBelow65, m_clBatchSelPIBCVParams.m_strComboPNBelow65ID ), 0 );

		if( iPos != m_ComboPNBelow65.GetCurSel() )
		{
			m_ComboPNBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_clBatchSelPIBCVParams.m_strComboPNBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeTypeAbove50()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboTypeAbove50ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboTypeAbove50 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboTypeAbove50, m_clBatchSelPIBCVParams.m_strComboTypeAbove50ID ), 0 );

		if( iPos != m_ComboTypeAbove50.GetCurSel() )
		{
			m_ComboTypeAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboTypeAbove50 );

	m_clBatchSelPIBCVParams.m_strComboTypeAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboTypeAbove50 );

	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamilyAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboBodyMaterialAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNAbove50, CDlgSelComboHelperBase::CS_Above50 );

	m_clBatchSelPIBCVParams.m_strComboFamilyAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyAbove50 );
	m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialAbove50 );
	m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectAbove50 );
	m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionAbove50 );
	m_clBatchSelPIBCVParams.m_strComboPNAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeFamilyAbove50()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboFamilyAbove50ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyAbove50 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboFamilyAbove50, m_clBatchSelPIBCVParams.m_strComboFamilyAbove50ID ), 0 );

		if( iPos != m_ComboFamilyAbove50.GetCurSel() )
		{
			m_ComboFamilyAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboFamilyAbove50 );

	m_clBatchSelPIBCVParams.m_strComboFamilyAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyAbove50 );

	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboBodyMaterialAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNAbove50, CDlgSelComboHelperBase::CS_Above50 );

	m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialAbove50 );
	m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectAbove50 );
	m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionAbove50 );
	m_clBatchSelPIBCVParams.m_strComboPNAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeBdyMatAbove50()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialAbove50 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboBodyMaterialAbove50, m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID ), 0 );

		if( iPos != m_ComboBodyMaterialAbove50.GetCurSel() )
		{
			m_ComboBodyMaterialAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboBodyMaterialAbove50 );

	m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialAbove50 );

	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNAbove50, CDlgSelComboHelperBase::CS_Above50 );

	m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectAbove50 );
	m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionAbove50 );
	m_clBatchSelPIBCVParams.m_strComboPNAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeConnectAbove50()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectAbove50 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboConnectAbove50, m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID ), 0 );

		if( iPos != m_ComboConnectAbove50.GetCurSel() )
		{
			m_ComboConnectAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboConnectAbove50 );

	m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectAbove50 );

	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNAbove50, CDlgSelComboHelperBase::CS_Above50 );

	m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionAbove50 );
	m_clBatchSelPIBCVParams.m_strComboPNAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangeVersionAbove50()
{
	// Don't call base class!! Combo management for PIBCV has been changed and we don't need anymore the base class for this.

	if( m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionAbove50 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboVersionAbove50, m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID ), 0 );

		if( iPos != m_ComboVersionAbove50.GetCurSel() )
		{
			m_ComboVersionAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclSelComboHelperPIBCV->OnCbnSelChange( &m_ComboVersionAbove50 );

	m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionAbove50 );

	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNAbove50, CDlgSelComboHelperBase::CS_Above50 );
	m_clBatchSelPIBCVParams.m_strComboPNAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnCbnSelChangePNAbove50()
{
	if( m_clBatchSelPIBCVParams.m_strComboPNAbove50ID == m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// Remark: 'true' to signal that it is the last combo in the chain to be changed.
	//         It allows us to reset the 'm_prBatchSelectionVariables->m_fMsgComboChangeDone' variable to false.
	if( false == BS_ComboChange( true ) )
	{
		int iPos = max( m_pclSelComboHelperPIBCV->FindItemDataID( &m_ComboPNAbove50, m_clBatchSelPIBCVParams.m_strComboPNAbove50ID ), 0 );

		if( iPos != m_ComboPNAbove50.GetCurSel() )
		{
			m_ComboPNAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_clBatchSelPIBCVParams.m_strComboPNAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelPICv::OnBnClickedSuggest()
{
	ClearAll();

	// Validate data passed from the clipboard to the sheet.
	//	BS_VerifyDataInput();

	if( BS_InputsVerificationStatus::BS_IVS_Error == m_eInputsVerificationStatus )
	{
		// Show message only if the window is visible.
		if( TRUE == IsWindowVisible() )
		{
			MessageBox( TASApp.LoadLocalizedString( IDS_BATCHSELECTION_DATAINVALID ) );
		}

		m_clButtonSuggest.EnableWindow( FALSE );
	}
	else if( BS_InputsVerificationStatus::BS_IVS_OK == m_eInputsVerificationStatus )
	{
		BS_SuggestSelection();
	}
}

void CDlgBatchSelPICv::OnBnClickedValidate()
{
	CDlgBatchSelCtrlBase::OnBnClickedValidate();

	if( true == m_clBatchSelPIBCVParams.m_bEditModeRunning )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
		}

		_BS_ApplySelection();

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
		}
	}
}

void CDlgBatchSelPICv::OnBnClickedCancel()
{
	// User has edited a valid selection to change it but finally cancel it.
	OnRViewSSelKeyboardEvent( VK_ESCAPE );
	ResetRightView();
}

LRESULT CDlgBatchSelPICv::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelCtrlBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelPIBCVParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return -1;
	}

	m_clBatchSelPIBCVParams.m_eCvCtrlType = pclBatchSelParameter->GetPICvCBCtrlType();

	// Fill the combo box.
	m_pclSelComboHelperPIBCV->PrepareComboContents();

	m_pclSelComboHelperPIBCV->FillComboCtrlType( &m_ComboCtrlType, CDlgSelComboHelperBase::CS_All, pclBatchSelParameter->GetPICvCBCtrlType() );
	
	m_pclSelComboHelperPIBCV->FillComboType( &m_ComboTypeBelow65, CDlgSelComboHelperBase::CS_Below65, pclBatchSelParameter->GetPICvTypeBelow65ID() );
	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamilyBelow65, CDlgSelComboHelperBase::CS_Below65, pclBatchSelParameter->GetPICvFamilyBelow65ID() );
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboBodyMaterialBelow65, CDlgSelComboHelperBase::CS_Below65, pclBatchSelParameter->GetPICvMaterialBelow65ID() );
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectBelow65, CDlgSelComboHelperBase::CS_Below65, pclBatchSelParameter->GetPICvConnectBelow65ID() );
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionBelow65, CDlgSelComboHelperBase::CS_Below65, pclBatchSelParameter->GetPICvVersionBelow65ID() );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNBelow65, CDlgSelComboHelperBase::CS_Below65, pclBatchSelParameter->GetPICvPNBelow65ID() );

	m_pclSelComboHelperPIBCV->FillComboType( &m_ComboTypeAbove50, CDlgSelComboHelperBase::CS_Above50, pclBatchSelParameter->GetPICvTypeAbove50ID() );
	m_pclSelComboHelperPIBCV->FillComboFamily( &m_ComboFamilyAbove50, CDlgSelComboHelperBase::CS_Above50, pclBatchSelParameter->GetPICvFamilyAbove50ID() );
	m_pclSelComboHelperPIBCV->FillComboBodyMaterial( &m_ComboBodyMaterialAbove50, CDlgSelComboHelperBase::CS_Above50, pclBatchSelParameter->GetPICvMaterialAbove50ID() );
	m_pclSelComboHelperPIBCV->FillComboConnect( &m_ComboConnectAbove50, CDlgSelComboHelperBase::CS_Above50, pclBatchSelParameter->GetPICvConnectAbove50ID() );
	m_pclSelComboHelperPIBCV->FillComboVersion( &m_ComboVersionAbove50, CDlgSelComboHelperBase::CS_Above50, pclBatchSelParameter->GetPICvVersionAbove50ID() );
	m_pclSelComboHelperPIBCV->FillComboPN( &m_ComboPNAbove50, CDlgSelComboHelperBase::CS_Above50, pclBatchSelParameter->GetPICvPNAbove50ID() );

	// Update now 'm_clBatchSelPIBCVParams' with the current value selected in the combos.
	m_clBatchSelPIBCVParams.m_strComboTypeBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboTypeBelow65 );
	m_clBatchSelPIBCVParams.m_strComboFamilyBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyBelow65 );
	m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialBelow65 );
	m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectBelow65 );
	m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionBelow65 );
	m_clBatchSelPIBCVParams.m_strComboPNBelow65ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNBelow65 );

	m_clBatchSelPIBCVParams.m_strComboTypeAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboTypeAbove50 );
	m_clBatchSelPIBCVParams.m_strComboFamilyAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboFamilyAbove50 );
	m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboBodyMaterialAbove50 );
	m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboConnectAbove50 );
	m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboVersionAbove50 );
	m_clBatchSelPIBCVParams.m_strComboPNAbove50ID = m_pclSelComboHelperPIBCV->GetIDFromCombo( &m_ComboPNAbove50 );

	// Remark: Corresponding combo variables in 'm_clBatchSelPIBCVParams' are updated in each of this following methods.
	FillComboPowerSupply( pclBatchSelParameter->GetPICvActPowerSupplyID() );
	FillComboInputSignal( pclBatchSelParameter->GetPICvActInputSignalID() );
	int iCheck = -1;
	if( 1 == pclBatchSelParameter->GetPICvFailSafeFct() )
	{
		iCheck = BST_CHECKED;
	}
	else if( 0 == pclBatchSelParameter->GetPICvFailSafeFct() )
	{
		iCheck = BST_UNCHECKED;
	}
	ASSERT( iCheck != -1 );
	m_CheckFailSafeFct.SetCheck( iCheck );
	m_clBatchSelPIBCVParams.m_iActuatorFailSafeFunction = pclBatchSelParameter->GetPICvFailSafeFct();
	VerifyCheckboxFailsafeStatus();
	FillComboDRPFct( pclBatchSelParameter->GetPICvDRPFct() );

	// Set the 'Flow' radio state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclBatchSelParameter->GetPICvRadioFlowState() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clBatchSelPIBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgBatchSelCtrlBase::UpdateFlowOrPowerDTState();

	// Set the 'Control type strict' check box state.
	m_clBatchSelPIBCVParams.m_bIsCtrlTypeStrictChecked = pclBatchSelParameter->GetPICvCtrlTypeStrictChecked();
	m_clCheckboxStrict.SetCheck( ( true == m_clBatchSelPIBCVParams.m_bIsCtrlTypeStrictChecked ) ? BST_CHECKED : BST_UNCHECKED );

	// This is needed only in the case of the dialog is currently active when user creates a new document (or opens a new one).
	OnAfterNewDocument();

	m_bInitialised = true;
	return 0;
}

CRViewSSelSS *CDlgBatchSelPICv::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelPICv;
}

void CDlgBatchSelPICv::ClearAll( void )
{
	if( NULL == pRViewSSelPICv )
	{
		return;
	}

	// Disable the 'Validate' button.
	if( false == m_clBatchSelPIBCVParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	// Clear the right sheet.
	ResetRightView();
}

void CDlgBatchSelPICv::BS_PrepareColumnList( void )
{
	CDlgBatchSelCtrlBase::BS_PrepareColumnList();

	BSColParameters rColParameters;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Prepare header and data parameters.
	rColParameters.m_clHeaderColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_PICVMainHeaderID::BS_PICVMHID_ValveActuator,
			BS_PICVMainHeaderSubID::BS_PICVMHSUBID_Valve, DlgOutputHelper::CColDef::AutoResizeDisabled, DlgOutputHelper::CColDef::Visible, 
			DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionDisabled, DlgOutputHelper::CColDef::BlockSelectionDisabled,
			DlgOutputHelper::CColDef::RowSelectionDisabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );

	rColParameters.m_clHeaderColDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );

	rColParameters.m_clHeaderFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Center, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled, DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_clDataColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_PICVMainHeaderID::BS_PICVMHID_ValveActuator,
			BS_PICVMainHeaderSubID::BS_PICVMHSUBID_Valve, DlgOutputHelper::CColDef::AutoResizeDisabled, DlgOutputHelper::CColDef::Visible, 
			DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionEnabled, DlgOutputHelper::CColDef::BlockSelectionEnabled,
			DlgOutputHelper::CColDef::RowSelectionEnabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataEnabled );
	
	rColParameters.m_clDataColDef.SetRowSeparatorFlag( true, DLGBATCHSELBASE_COLOR_DATAROWSEPARATOR );

	rColParameters.m_clDataFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Left, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled, DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	// Columns linked to the valve.
	rColParameters.m_iColumnID = BS_PICVCID_Name;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_bLineBelowHeader = true;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_FC;
	rColParameters.m_bCantRemove = false;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_FC );
	rColParameters.m_strHeaderUnit = _T( "" );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_Material;
	rColParameters.m_clHeaderColDef.SetWidth( 12.0 );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_clDataColDef.SetWidth( 12.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_Connection;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_Version;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_VERSION );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_Size;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHVALVESIZE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_PN;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_Rangeability;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_RANGEABILITY );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_LeakageRate;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_LEAKAGERATE );
	rColParameters.m_strHeaderUnit = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_LEAKAGERATEUNIT );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_Stroke;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIAMETER );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_CVSTROKE );
	rColParameters.m_strHeaderUnit = _T( "" );	// Unit will be set in regards to the control valve just near the stroke value.
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ImgCharacteristic;
	rColParameters.m_clHeaderColDef.SetWidth( 2.0 );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Bitmap );
	rColParameters.m_clDataColDef.SetWidth( 2.0 );
	rColParameters.m_strHeaderName = _T( "" );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ImgPushClose;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_Preset;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetMaxDigit( 2 );
	rColParameters.m_clDataColDef.SetMinDecimal( 2 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PRESET );
	rColParameters.m_strHeaderUnit = TASApp.LoadLocalizedString( IDS_SHEETHDR_TURNSPOS );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_DpMin;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_DPMIN );
	rColParameters.m_strHeaderUnit = _T( "" );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_TemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_PipeSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPESIZE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_PipeLinDp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_LINPRESSDROP );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPELINDP );
	rColParameters.m_iHeaderUnit = _U_LINPRESSDROP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_PipeV;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_VELOCITY );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPEV );
	rColParameters.m_iHeaderUnit = _U_VELOCITY;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	// Columns linked to the actuator.
	rColParameters.m_iColumnID = BS_PICVCID_ActrName;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clHeaderColDef.SetMainHeaderLinks( BS_PICVMainHeaderID::BS_PICVMHID_ValveActuator, BS_PICVMainHeaderSubID::BS_PICVMHSUBID_Actuator );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRNAME );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrCloseOffDp;
	rColParameters.m_bCantRemove = false;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRCLOSEOFFDP );
	rColParameters.m_strHeaderUnit = _T( "" );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrMaxInletPressure;
	rColParameters.m_bCantRemove = false;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_PRESSURE );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXINLETPRESS );
	rColParameters.m_strHeaderUnit = _T( "" );
	rColParameters.m_iHeaderUnit = _U_PRESSURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrActuatingTime;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTUATINGTIME );
	rColParameters.m_iHeaderUnit = _U_TIME;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrIP;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRIP );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrPowSupply;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRPOWERSUP );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrInputSig;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRINPUTSIG );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrOutputSig;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTROUTPUTSIG );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrFailSafe;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRFAILSAFE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_PICVCID_ActrDRP;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	// HYS-956: Display adapter column
	rColParameters.m_iColumnID = BS_PICVCID_ActrAdapterName;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRADAPTNAME );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

}

bool CDlgBatchSelPICv::BS_InitMainHeaders( void )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelCtrlBase::BS_InitMainHeaders() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
		}

		return false;
	}

	// Add main header for 'Valve'.
	DlgOutputHelper::CColDef clColMHInDef;
	clColMHInDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHInDef.SetMouseEventFlag( true );
	clColMHInDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );
	
	// To specify that the group valve is linked to the main header 'Output'.
	clColMHInDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output );
	
	m_clInterface.AddMainHeaderDef( BS_PICVMainHeaderID::BS_PICVMHID_ValveActuator, BS_PICVMainHeaderSubID::BS_PICVMHSUBID_Valve,
			clColMHInDef, TASApp.LoadLocalizedString( IDS_DLGBATCHSELPICV_TITLEGROUPVALVE ), (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHInDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center, 
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _IMI_GRAY_MED, _WHITE );
	
	m_clInterface.AddMainHeaderFontDef( BS_PICVMainHeaderID::BS_PICVMHID_ValveActuator, BS_PICVMainHeaderSubID::BS_PICVMHSUBID_Valve,
			clFontMHInDef, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	// Add main header for 'Actuator'.
	DlgOutputHelper::CColDef clColMHOutDef;
	clColMHOutDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHOutDef.SetMouseEventFlag( true );
	clColMHOutDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );
	
	// To specify that the group actuator is linked to the main header 'Output'.
	clColMHOutDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output );
	
	m_clInterface.AddMainHeaderDef( BS_PICVMainHeaderID::BS_PICVMHID_ValveActuator, BS_PICVMainHeaderSubID::BS_PICVMHSUBID_Actuator,
			clColMHOutDef, TASApp.LoadLocalizedString( IDS_DLGBATCHSELPICV_TITLEGROUPACTR ), (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHOutDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center, 
		DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _IMI_GRAY_MED, _WHITE );

	m_clInterface.AddMainHeaderFontDef( BS_PICVMainHeaderID::BS_PICVMHID_ValveActuator,
			BS_PICVMainHeaderSubID::BS_PICVMHSUBID_Actuator, clFontMHOutDef, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	m_clInterface.SetMainHeaderRowHeight( BS_PICVMainHeaderID::BS_PICVMHID_ValveActuator, DLBBATCHSELPICV_ROWHEIGHT_MAINHEADER,
			(int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelPICv::BS_InitColumns()
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelCtrlBase::BS_InitColumns() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
		}

		return false;
	}

	BS_AddOneColumn( BS_PICVColumnID::BS_PICVCID_Name );
	BS_AddOneColumn( BS_PICVColumnID::BS_PICVCID_ActrName );
	BS_AddOneColumn( BS_PICVColumnID::BS_PICVCID_ActrAdapterName );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelPICv::BS_Validate()
{
	try
	{
		if( false == CDlgBatchSelCtrlBase::BS_Validate() )
		{
			return false;
		}

		CTable *pTab = dynamic_cast<CTable *>( m_clBatchSelPIBCVParams.m_pTADS->Get( _T("CTRLVALVE_TAB") ).MP );

		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CTRLVALVE_TAB' table from the datastruct.") );
		}

		bool bAtLeastOneInserted = false;

		for( mapLongRowParamsIter iterRow = m_mapAllRowData.begin(); iterRow != m_mapAllRowData.end();
			 iterRow++ )
		{
			BSRowParameters *pclRowParameters = &iterRow->second;

			if( RowStatus::BS_RS_FindOneSolution != pclRowParameters->m_eStatus && RowStatus::BS_RS_FindOneSolutionAlter != pclRowParameters->m_eStatus )
			{
				continue;
			}

			CDS_SSelPICv *pSSelPICv = ( CDS_SSelPICv * )pclRowParameters->m_pclCDSSelSelected;

			if( NULL == pSSelPICv )
			{
				// Create selected object and initialize it.
				pSSelPICv = _BS_CreateSSelPICv( pclRowParameters );
			}

			if( NULL == pSSelPICv )
			{
				continue;
			}

			IDPTR IDPtr = pSSelPICv->GetIDPtr();
			pTab->Insert( IDPtr );

			bAtLeastOneInserted = true;

			// Remark: It's up to the database to clean now the object if no more needed. To avoid that the 'CDlgBatchSelCtrlBase::BS_ClearAllData' method
			//         try to clear, we set the pointer to NULL.
			pclRowParameters->m_pclCDSSelSelected = NULL;
		}

		return bAtLeastOneInserted;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelPICv::BS_Validate'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelPICv::BS_EnableCombos( bool bEnable )
{
	CDlgBatchSelCtrlBase::BS_EnableCombos( bEnable );

	m_ComboCtrlType.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_ComboCtrlType.GetCount() <= 1 )
	{
		m_ComboCtrlType.EnableWindow( FALSE );
	}

	GetDlgItem( IDC_STATICCTRLTYPE )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	m_clCheckboxStrict.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	m_GroupActuator.EnableWindow( bEnable, false );

	m_ComboPowerSupply.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_ComboPowerSupply.GetCount() <= 1 )
	{
		m_ComboPowerSupply.EnableWindow( FALSE );
	}

	GetDlgItem( IDC_STATICPOWERSUPPLY )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	m_ComboInputSignal.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_ComboInputSignal.GetCount() <= 1 )
	{
		m_ComboInputSignal.EnableWindow( FALSE );
	}

	GetDlgItem( IDC_STATICINPUTSIGNAL )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	m_CheckFailSafeFct.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	// HYS-1079 : When all field are setted to true we have to check for fail-safe field the m_CheckStatus value before enable it
	if( CTADatabase::FailSafeCheckStatus::eBothFailSafe != m_CheckStatus && TRUE == bEnable )
	{
		m_CheckFailSafeFct.EnableWindow( FALSE );
	}

	m_ComboDRPFct.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_ComboDRPFct.GetCount() <= 1 )
	{
		m_ComboDRPFct.EnableWindow( FALSE );
	}

	GetDlgItem( IDC_STATICDRP )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
}

void CDlgBatchSelPICv::BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults )
	{
		return;
	}

	for( int iLoop = BS_PICVColumnID::BS_PICVCID_First; iLoop < BS_PICVColumnID::BS_PICVCID_Last; iLoop++ )
	{
		if( 0 == m_mapColumnList.count( iLoop ) )
		{
			continue;
		}

		if( false == m_mapColumnList[iLoop].m_bDisplayed )
		{
			continue;
		}

		_BS_FillCell( pclBSMessage, pclRowParameters, iLoop );
	}
}

bool CDlgBatchSelPICv::BS_SuggestSelection()
{
	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL == pclBSMessage )
	{
		return false;
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_clBatchSelPIBCVParams.m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	// We block the redraw because it is too long otherwise.
	BeginWaitCursor();
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;

	// Variable uses in test unit to drop out results in a file.
	m_clBatchSelPIBCVParams.m_vecRowParameters.clear();

	for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
	{
		BSRowParameters *pclRowParameters = &iterRows->second;

		if( NULL == pclRowParameters )
		{
			continue;
		}

		// Put this 'pclRowParameters' in the 'CBatchSelectionParameters::m_vecRowParameters' to facilitate test unit.
		m_clBatchSelPIBCVParams.m_vecRowParameters.push_back( pclRowParameters );

		// 'CBatchSelectPICvList' is created and the pointer is saved in 'pclRowParameters->m_pclBatchResults' for each row. But the pointer is
		// also saved in the 'm_clBatchSelPIBCVParams.m_pclBatchPIBCVList' variable to help us calling 'SelectPICv' method. This is the 
		// 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods that will delete this pointer. 
		// Thus no need to do it in the 'CBatchSelPIBCVParams' destructor.
		pclRowParameters->m_pclBatchResults = new CBatchSelectPICvList();

		if( NULL == pclRowParameters->m_pclBatchResults )
		{
			continue;
		}

		m_clBatchSelPIBCVParams.m_pclBatchPIBCVList = (CBatchSelectPICvList*)pclRowParameters->m_pclBatchResults;
		long lCurrentRow = iterRows->first;
		
		pclBSMessage->Clear();

		pclRowParameters->m_pclSelectedProduct = NULL;

		if( CDS_SelProd::efdFlow != m_clBatchSelPIBCVParams.m_eFlowOrPowerDTMode )
		{
			if( pclRowParameters->m_dPower > 0.0 && pclRowParameters->m_dDT > 0.0 )
			{
				CWaterChar clWaterCharReturn = m_clBatchSelPIBCVParams.m_WC;
				double dReturnTemperature = ( ProjectType::Cooling == m_clBatchSelPIBCVParams.m_eApplicationType ) ? m_clBatchSelPIBCVParams.m_WC.GetTemp() + pclRowParameters->m_dDT :
						m_clBatchSelPIBCVParams.m_WC.GetTemp() - pclRowParameters->m_dDT;

				clWaterCharReturn.UpdateFluidData( dReturnTemperature );

				pclRowParameters->m_dFlow = m_clBatchSelPIBCVParams.m_WC.GetTemp() * m_clBatchSelPIBCVParams.m_WC.GetDens() * m_clBatchSelPIBCVParams.m_WC.GetSpecifHeat();
				pclRowParameters->m_dFlow -= ( clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat() );
				pclRowParameters->m_dFlow = abs( pclRowParameters->m_dPower / pclRowParameters->m_dFlow );
			}
		}

		int iReturn = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->SelectPICv( &m_clBatchSelPIBCVParams, pclRowParameters->m_dFlow );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturn & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturn & CBatchSelectBaseList::BR_BadArguments ) )
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturn & CBatchSelectBaseList::BR_NoPipeFound ) )
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturn & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			m_bAtLeastOneRowWithNoSolution = true;
			iterRows->second.m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindNoSolution;
			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_Name, _T( "" ), true );
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_OUTPUTBOXERROR, true );
			delete pclRowParameters->m_pclBatchResults;
			pclRowParameters->m_pclBatchResults = NULL;

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( IDS_BATCHSELECTION_NOSOLUTION ),
						(int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturn & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			int iActMatch = -1;
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->GetBestProduct();
			
			pclRowParameters->m_pclSelectedActuator = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->SelectActuator( &m_clBatchSelPIBCVParams,

					(CDB_ControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ), pclRowParameters->m_dDp,  &iActMatch );
			// HYS-956: Display adapter			
			pclRowParameters->m_pclSelectedAdapActuator = 
				((CDB_ControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ))->GetMatchingAdapter( pclRowParameters->m_pclSelectedActuator);

			m_bAtLeastOneRowWithBestSolution = true;
			int iBitmapID;

			if( false == bAlternative && iActMatch == CBatchSelectBaseList::BatchReturn::BR_FoundOneBest )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindOneSolution;
				iBitmapID = IDI_BATCHSELOK;
			}
			else
			{
				m_bAtLeastOneRowWithSolutionButAlternative = true;
				bAlternative = true;
				pclRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindOneSolutionAlter;
				iBitmapID = IDI_OUTPUTBOXOKINFO;
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );
			BS_PrepareOutput( pclBSMessage, pclRowParameters );

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
				int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
						(int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturn & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			int iActMatch = -1;
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == pclRowParameters->m_pclBatchResults->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->GetFirst<CSelectedBase>();
				
				pclRowParameters->m_pclSelectedActuator = m_clBatchSelPIBCVParams.m_pclBatchPIBCVList->SelectActuator( &m_clBatchSelPIBCVParams,

						(CDB_ControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ), pclRowParameters->m_dDp,  &iActMatch );
				// HYS-956: Display adapter
				pclRowParameters->m_pclSelectedAdapActuator = 
				( (CDB_ControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ) )->GetMatchingAdapter( pclRowParameters->m_pclSelectedActuator );

				m_bAtLeastOneRowWithBestSolution = true;
				int iBitmapID;

				if( false == bAlternative && iActMatch == CBatchSelectBaseList::BatchReturn::BR_FoundOneBest )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindOneSolution;
					iBitmapID = IDI_BATCHSELOK;
				}
				else
				{
					m_bAtLeastOneRowWithSolutionButAlternative = true;
					bAlternative = true;
					pclRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindOneSolutionAlter;
					iBitmapID = IDI_OUTPUTBOXOKINFO;
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );
				BS_PrepareOutput( pclBSMessage, pclRowParameters );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
					
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
				}
			}
			else
			{
				m_bAtLeastOneRowWithSolutionButNotBest = true;
				int iBitmapID;

				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindSolutions;
					iBitmapID = IDI_OUTPUTBOXWARNING;
				}
				else
				{
					m_bAtLeastOneRowWithSolutionButAlternative = true;
					pclRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindSolutionsAlter;
					iBitmapID = IDI_OUTPUTBOXWARNINFO;
				}

				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_Name, _T( "" ), true );
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_SEVRESULTS : IDS_BATCHSELECTION_SEVRESULTSWITHINFO;
					
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
				}
			}
		}

		// Send message.
		pclBSMessage->SendMessage( lCurrentRow, true, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	EndWaitCursor();

	return CDlgBatchSelCtrlBase::BS_SuggestSelection();
}

bool CDlgBatchSelPICv::BS_AddOneColumn( int iColumnID )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelCtrlBase::BS_AddOneColumn( iColumnID ) )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
		}

		return false;
	}

	if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
	{
		CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

		if( NULL == pclBSMessage )
		{
			if( false == bIsRedrawBlocked )
			{
				m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
			}

			return false;
		}

		for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
		{
			long lCurrentRow = iterRows->first;
			pclBSMessage->Clear();

			_BS_FillCell( pclBSMessage, &iterRows->second, iColumnID );
			pclBSMessage->SendMessage( lCurrentRow, false, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	return true;
}

// Remark: 'CW' is for 'Column Width'.
// HYS-1995: Save version. Version 2.
#define CW_BATCHSELPICV_VERSION			2
void CDlgBatchSelPICv::BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}

	int iVersion = CW_BATCHSELPICV_VERSION;
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth(
				CW_WINDOWID_BATCHSELPICV, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = ( CDS_BatchSelParameter::CCWBatchSelSheet * )pclCWWindow->GetCWSheet( 0 );

	if( NULL != pclCWSheet && pclCWSheet->GetVersion() == iVersion )
	{
		CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfo mapColumnInfo = pclCWSheet->GetMap();
		CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfoIter iter;

		for( iter = mapColumnInfo.begin(); iter != mapColumnInfo.end(); ++iter )
		{
			m_mapColumnList[iter->first].m_clHeaderColDef.SetWidthInPixels( iter->second.m_lWidth );
			m_mapColumnList[iter->first].m_clDataColDef.SetWidthInPixels( iter->second.m_lWidth );

			if( false == m_mapColumnList[iter->first].m_bEnabled || false == iter->second.m_bIsVisible )
			{
				continue;
			}

			if( false == m_mapColumnList[iter->first].m_bDisplayed )
			{
				BS_AddOneColumn( iter->first );

				if( iter->first >= BS_PICVCID_First )
				{
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3True;
				}
			}
			else
			{
				m_clInterface.SetColumnWidthInPixel( iter->first, iter->second.m_lWidth, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
			}
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	}
}

void CDlgBatchSelPICv::BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth(
				CW_WINDOWID_BATCHSELPICV, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = ( CDS_BatchSelParameter::CCWBatchSelSheet * )pclCWWindow->GetCWSheet( 0, true );

	int iAfterColumnID = -1;
	CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfo mapColumnWidth;

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); iter++ )
	{
		CDS_BatchSelParameter::CCWBatchSelSheet::ColumnInfo rColumnInfo;
		rColumnInfo.m_bIsVisible = iter->second.m_bDisplayed;

		if( true == iter->second.m_bDisplayed )
		{
			m_clInterface.GetColumnWidthInPixel( iter->first, rColumnInfo.m_lWidth, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
			rColumnInfo.m_iAfterColumnID = iAfterColumnID;
			iAfterColumnID = iter->first;
		}

		mapColumnWidth[iter->first] = rColumnInfo;
	}

	pclCWSheet->GetMap() = mapColumnWidth;
	// HYS-1995: Save version.
	pclCWSheet->SetVersion( CW_BATCHSELPICV_VERSION );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDlgBatchSelPICv::_BS_ApplySelection()
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelPICv )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == m_pclCurrentRowParameters || NULL == m_pclCurrentRowParameters->m_pclBatchResults )
	{
		return false;
	}

	CDB_TAProduct *pclTAProduct = pRViewSSelPICv->GetCurrentControlValveSelected();

	if( NULL == pclTAProduct )
	{
		return false;
	}

	bool bFound = false;
	CBatchSelectBaseList *pclBatchList = m_pclCurrentRowParameters->m_pclBatchResults;

	for( CSelectedBase *pclSelectedValve = pclBatchList->GetFirst<CSelectedBase>(); NULL != pclSelectedValve && false == bFound;
			pclSelectedValve = pclBatchList->GetNext<CSelectedBase>() )
	{
		CDB_PIControlValve *pclPIControlValve = dynamic_cast<CDB_PIControlValve *>( pclSelectedValve->GetpData() );

		if( pclPIControlValve == pclTAProduct )
		{
			m_pclCurrentRowParameters->m_pclSelectedProduct = pclSelectedValve;
			m_pclCurrentRowParameters->m_pclSelectedActuator = pRViewSSelPICv->GetCurrentActuatorSelected();
			// HYS-956: Display adapter
			m_pclCurrentRowParameters->m_pclSelectedAdapActuator = pRViewSSelPICv->GetFirstAdapterSelected();
			bFound = true;
		}
	}

	if( false == bFound )
	{
		return false;
	}

	// When user comes back from the right view (by selecting a product, actuator, accessories, ...) we have two possibilities. If user edits a
	// product for which there is only one solution (marks with the green arrow icon in the 'Status' column), we create a 'CDS_SSelProd' in the
	// 'OnDlgOutputCellDblClicked' method. If user edits a product for which there is more than one solution (marks with yellow exclamation icon), we can't
	// create. In this case we have to do it here.
	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		m_pclCurrentRowParameters->m_pclCDSSelSelected = _BS_CreateSSelPICv( m_pclCurrentRowParameters );
	}

	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		return false;
	}

	pRViewSSelPICv->FillInSelected( m_pclCurrentRowParameters->m_pclCDSSelSelected );

	// Memorize current block selection if exist.
	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	// If previous status contained alternative, we reset this flag and keep only the user action.
	m_pclCurrentRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindOneSolution;

	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL != pclBSMessage )
	{
		pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_BATCHSELOK, true );
	}

	BS_PrepareOutput( pclBSMessage, m_pclCurrentRowParameters );

	// Send message.
	pclBSMessage->SendMessage( m_pclCurrentRowParameters->m_lRow, true, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	// Restore the previous selections.
	m_clInterface.RestoreSelections( (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );

	// HYS-1168 : We hide the DockablePane in edition mode, we have remove the auto hide mode when doing Apply
	CToolsDockablePane* pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}

	// Change the tools dockable pane to full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, true );

	pRViewSSelPICv->UnregisterNotificationHandler( this );

	m_pclCurrentRowParameters = NULL;
	ClearAll();

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	SetModificationMode( false );
	return true;
}

CDS_SSelPICv *CDlgBatchSelPICv::_BS_CreateSSelPICv( BSRowParameters *pclRowParameters )
{
	try
	{
		if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct || _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr()
				|| NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
		{
			return NULL;
		}

		CDB_PIControlValve *pclPIControlValve = ( CDB_PIControlValve * )( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

		if( NULL == pclPIControlValve )
		{
			return NULL;
		}

		// Create selected object and initialize it.
		CDS_SSelPICv *pSelPICv = NULL;
		IDPTR IDPtr = _NULL_IDPTR;
		m_clBatchSelPIBCVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPICv ) );

		pSelPICv = ( CDS_SSelPICv * )( IDPtr.MP );
		pSelPICv->SetProductIDPtr( pclPIControlValve->GetIDPtr() );

		double dRho = m_clBatchSelPIBCVParams.m_WC.GetDens();
		double dKinVisc = m_clBatchSelPIBCVParams.m_WC.GetKinVisc();
		double dPresetting = pclPIControlValve->GetPresetting( pclRowParameters->m_dFlow, dRho, dKinVisc );
		pSelPICv->SetOpening( dPresetting );

		pSelPICv->SetQ( pclRowParameters->m_dFlow );

		if( CDS_SelProd::efdFlow == m_clBatchSelPIBCVParams.m_eFlowOrPowerDTMode )
		{
			pSelPICv->SetFlowDef( CDS_SelProd::efdFlow );
			pSelPICv->SetPower( 0.0 );
			pSelPICv->SetDT( 0.0 );
		}
		else
		{
			pSelPICv->SetFlowDef( CDS_SelProd::efdPower );
			pSelPICv->SetPower( pclRowParameters->m_dPower );
			pSelPICv->SetDT( pclRowParameters->m_dDT );
		}

		pSelPICv->SetSelectedAsAPackage( m_clBatchSelPIBCVParams.m_bActuatorSelectedAsSet );

		pSelPICv->SetCtrlType( m_clBatchSelPIBCVParams.m_eCvCtrlType );
		pSelPICv->SetTypeID( pclPIControlValve->GetTypeIDPtr().ID );
		pSelPICv->SetFamilyID( pclPIControlValve->GetFamilyIDPtr().ID );
		pSelPICv->SetMaterialID( pclPIControlValve->GetBodyMaterialIDPtr().ID );
		pSelPICv->SetConnectID( pclPIControlValve->GetConnectIDPtr().ID );
		pSelPICv->SetVersionID( pclPIControlValve->GetVersionIDPtr().ID );
		pSelPICv->SetPNID( pclPIControlValve->GetPNIDPtr().ID );

		pSelPICv->SetActrIDPtr( ( NULL != pclRowParameters->m_pclSelectedActuator ) ? pclRowParameters->m_pclSelectedActuator->GetIDPtr() : _NULL_IDPTR );
		// HYS-956: Display adapter
		if(( NULL != pSelPICv->GetCvAccessoryList()) && (NULL != pclRowParameters->m_pclSelectedAdapActuator ))
		{
			pSelPICv->GetCvAccessoryList()->Add( pclRowParameters->m_pclSelectedAdapActuator->GetIDPtr(), CAccessoryList::_AT_Adapter );
		}

		// Set the actuators info.
		pSelPICv->SetActuatorPowerSupplyID( m_clBatchSelPIBCVParams.m_strActuatorPowerSupplyID );
		pSelPICv->SetActuatorInputSignalID( m_clBatchSelPIBCVParams.m_strActuatorInputSignalID );
		pSelPICv->SetFailSafeFunction( m_clBatchSelPIBCVParams.m_iActuatorFailSafeFunction );
		pSelPICv->SetDRPFunction( m_clBatchSelPIBCVParams.m_eActuatorDRPFunction );

		pSelPICv->SetPipeSeriesID( m_clBatchSelPIBCVParams.m_strPipeSeriesID );
		pSelPICv->SetPipeID( m_clBatchSelPIBCVParams.m_strPipeID );

		// Selected pipe informations.
		pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclPIControlValve->GetSizeKey(), *pSelPICv->GetpSelPipe() );

		*pSelPICv->GetpSelectedInfos()->GetpWCData() = m_clBatchSelPIBCVParams.m_WC;
		pSelPICv->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef1, pclRowParameters->m_strReference1 );
		pSelPICv->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef2, pclRowParameters->m_strReference2 );
		pSelPICv->GetpSelectedInfos()->SetQuantity( 1 );

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelPICv->GetpSelectedInfos()->SetDT( m_clBatchSelPIBCVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelPICv->GetpSelectedInfos()->SetApplicationType( m_clBatchSelPIBCVParams.m_eApplicationType );

		CTable *pTab = m_clBatchSelPIBCVParams.m_pTADS->GetpClipboardTable();
	
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CLIPBOARD_TABLE' table from the datastruct.") );
		}

		if( _T('\0') == *IDPtr.ID )
		{
			pTab->Insert( IDPtr );
		}

		return pSelPICv;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelPICv::_BS_CreateSSelPICv'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelPICv::_BS_DisplaySolutions()
{
	if( NULL != pRViewSSelPICv )
	{
		m_bRViewEmpty = false;
		SetModificationMode( true );

		// Remark: The 'SetModificationMode' method just above set the 'm_pclBatchSelParams->m_bEditModeRunning' to true.
		//         Thus we need to manually set it in 'm_clIndSelPIBCVParams' for the right view to pass in edition mode.
		m_clIndSelPIBCVParams.m_bEditModeRunning = true;

		pRViewSSelPICv->Suggest( &m_clIndSelPIBCVParams );
	}

	return;
}

void CDlgBatchSelPICv::_BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct
			|| _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() || iColumnID < BS_PICVColumnID::BS_PICVCID_First
			|| iColumnID >= BS_PICVColumnID::BS_PICVCID_Last )
	{
		return;
	}

	CDB_PIControlValve *pclPIControlValve = dynamic_cast<CDB_PIControlValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

	if( NULL == pclPIControlValve )
	{
		return;
	}

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( NULL == pUnitDB )
	{
		return;
	}

	// If needed.
	int iPhysicalType = m_mapColumnList[iColumnID].m_clDataColDef.GetPhysicalType();
	int iMaxDigit = m_mapColumnList[iColumnID].m_clDataColDef.GetMaxDigit();
	int iMinDecimal = m_mapColumnList[iColumnID].m_clDataColDef.GetMinDecimal();
	double dRho = m_clBatchSelPIBCVParams.m_WC.GetDens();
	double dKinVisc = m_clBatchSelPIBCVParams.m_WC.GetKinVisc();
	
	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclRowParameters->m_pclSelectedActuator );
	CDB_Product *pclAdaptActuator = dynamic_cast<CDB_Product *>( pclRowParameters->m_pclSelectedAdapActuator );

	CString str;

	switch( iColumnID )
	{
		case BS_PICVColumnID::BS_PICVCID_Name:
			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_Name, pclPIControlValve->GetName(), true );
			break;

		case BS_PICVColumnID::BS_PICVCID_FC:
			if( -1.0 != pclPIControlValve->GetFc() )
			{
				pclBSMessage->SetCellNumber( BS_PICVColumnID::BS_PICVCID_FC, pclPIControlValve->GetFc(), iPhysicalType, iMaxDigit, iMinDecimal, true );
			}
			break;

		case BS_PICVColumnID::BS_PICVCID_Material:
			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_Material, pclPIControlValve->GetBodyMaterial(), true );
			break;

		case BS_PICVColumnID::BS_PICVCID_Connection:
			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_Connection, pclPIControlValve->GetConnect(), true );
			break;

		case BS_PICVColumnID::BS_PICVCID_Version:
			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_Version, pclPIControlValve->GetVersion(), true );
			break;

		case BS_PICVColumnID::BS_PICVCID_PN:
			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_PN, pclPIControlValve->GetPN().c_str(), true );
			break;

		case BS_PICVColumnID::BS_PICVCID_Size:
			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_Size, pclPIControlValve->GetSize(), true );
			break;

		case BS_PICVColumnID::BS_PICVCID_Rangeability:
			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_Rangeability, pclPIControlValve->GetStrRangeability().c_str(), true );
			break;

		case BS_PICVColumnID::BS_PICVCID_LeakageRate:
			pclBSMessage->SetCellNumber( BS_PICVColumnID::BS_PICVCID_LeakageRate, pclPIControlValve->GetLeakageRate() * 100, iPhysicalType, 
					iMaxDigit, iMinDecimal, true );
			break;

		case BS_PICVColumnID::BS_PICVCID_Stroke:
		{
			CDB_CloseOffChar *pclCloseOffChar = ( CDB_CloseOffChar * )( pclPIControlValve->GetCloseOffCharIDPtr().MP );

			if( NULL != pclCloseOffChar )
			{
				if( CDB_CloseOffChar::eOpenType::Linear == pclCloseOffChar->GetOpenType() )
				{
					str = WriteCUDouble( iPhysicalType, pclPIControlValve->GetStroke(), false, iMaxDigit, iMinDecimal );
					str += CString( _T( "" ) ) + GetNameOf( pUnitDB->GetDefaultUnit( iPhysicalType ) ).c_str();
					pclBSMessage->SetCellNumberAsText( BS_PICVColumnID::BS_PICVCID_Stroke, str, iPhysicalType, iMaxDigit, iMinDecimal, true );
				}
				else
				{
					str = WriteDouble( pclPIControlValve->GetStroke(), 0, 0 );
					str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_ANGULARDEGRE );
					pclBSMessage->SetCellNumberAsText( BS_PICVColumnID::BS_PICVCID_Stroke, str, _U_NODIM, 0, 0, true );
				}
			}
			else
			{
				str = WriteCUDouble( iPhysicalType, pclPIControlValve->GetStroke(), false, iMaxDigit, iMinDecimal );
				str += CString( _T( "" ) ) + GetNameOf( pUnitDB->GetDefaultUnit( iPhysicalType ) ).c_str();
				pclBSMessage->SetCellNumberAsText( BS_PICVColumnID::BS_PICVCID_Stroke, str, iPhysicalType, iMaxDigit, iMinDecimal, true );
			}
		}
		break;

		case BS_PICVColumnID::BS_PICVCID_ImgCharacteristic:
			if( NULL != pclPIControlValve->GetCtrlProp() )
			{
				CDB_ControlProperties::eCTRLCHAR CtrlChar = pclPIControlValve->GetCtrlProp()->GetCvCtrlChar();

				if( CDB_ControlProperties::Linear == CtrlChar )
				{
					pclBSMessage->SetCellBitmap( BS_PICVColumnID::BS_PICVCID_ImgCharacteristic, IDI_CHARACTLIN, true );
				}
				else if( CDB_ControlProperties::EqualPc == CtrlChar )
				{
					pclBSMessage->SetCellBitmap( BS_PICVColumnID::BS_PICVCID_ImgCharacteristic, IDI_CHARACTEQM, true );
				}
				else
				{
					pclBSMessage->SetCellBitmap( BS_PICVColumnID::BS_PICVCID_ImgCharacteristic, IDI_CHARACTNONE, true );
				}
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ImgPushClose:
			if( NULL != pclPIControlValve->GetCtrlProp() )
			{
				CDB_ControlProperties::ePushOrPullToClose PushClose = pclPIControlValve->GetCtrlProp()->GetCvPushClose();

				if( CDB_ControlProperties::PushToClose == PushClose )
				{
					pclBSMessage->SetCellBitmap( BS_PICVColumnID::BS_PICVCID_ImgPushClose, IDI_PUSHCLOSE, true );
				}
				else if( CDB_ControlProperties::PullToClose == PushClose )
				{
					pclBSMessage->SetCellBitmap( BS_PICVColumnID::BS_PICVCID_ImgPushClose, IDI_PUSHOPEN, true );
				}
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_Preset:
		{
			double dPresetting = pclPIControlValve->GetPresetting( pclRowParameters->m_dFlow, dRho, dKinVisc );
			CDB_ValveCharacteristic *pclValveCharacteristic = ( CDB_ValveCharacteristic * )pclPIControlValve->GetValveCharDataPointer();

			if( NULL != pclValveCharacteristic && -1.0 != dPresetting )
			{
				str = pclValveCharacteristic->GetSettingString( dPresetting );
			}
			else
			{
				str = _T("-");
			}

			// If full opening indicator is set after the opening...
			bool bOrange = false;

			if( str.Find( _T('*') ) != -1 )
			{
				str.Replace( _T('*'), _T(' ') );
			}
			else if( NULL != pclValveCharacteristic && -1.0 != dPresetting && dPresetting < pclValveCharacteristic->GetMinRecSetting() )
			{
				bOrange = true;
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumberAsText( BS_PICVColumnID::BS_PICVCID_Preset, str, iPhysicalType, iMaxDigit, iMinDecimal,
					true );

			if( NULL != pclCellBase && true == bOrange )
			{
				pclCellBase->SetTextForegroundColor( _ORANGE );
			}
		}
		break;

		case BS_PICVColumnID::BS_PICVCID_DpMin:
		{
			// Get the Dp min value.
			double dDpMin =  pclPIControlValve->GetDpmin( pclRowParameters->m_dFlow, dRho );

			if( -1.0 != dDpMin )
			{
				pclBSMessage->SetCellNumber( BS_PICVColumnID::BS_PICVCID_DpMin, dDpMin, iPhysicalType, iMaxDigit, iMinDecimal, true );
			}
		}
		break;

		case BS_PICVColumnID::BS_PICVCID_TemperatureRange:
		{
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_TemperatureRange, 
					pclPIControlValve->GetTempRange(), true );

			if( NULL != pclCellBase )
			{
				if( m_clBatchSelPIBCVParams.m_WC.GetTemp() < pclPIControlValve->GetTmin() || m_clBatchSelPIBCVParams.m_WC.GetTemp() > pclPIControlValve->GetTmax() )
				{
					pclCellBase->SetTextForegroundColor( _RED );
				}
			}
		}
		break;

		case BS_PICVColumnID::BS_PICVCID_PipeSize:
		{
			CString str = _T( "" );
			CSelectPipe selPipe( &m_clBatchSelPIBCVParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclPIControlValve->GetSizeKey(), selPipe );
			}

			pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_PipeSize, selPipe.GetpPipe()->GetName(), true );
		}
		break;

		case BS_PICVColumnID::BS_PICVCID_PipeLinDp:
		{
			CSelectPipe selPipe( &m_clBatchSelPIBCVParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclPIControlValve->GetSizeKey(), selPipe );
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_PICVColumnID::BS_PICVCID_PipeLinDp, selPipe.GetLinDp(), 
					iPhysicalType, iMaxDigit, iMinDecimal, true );

			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelPIBCVParams.m_pTADS->GetpTechParams();

			if( NULL != pclCellBase && NULL != pclTechnicalParameter )
			{
				if( selPipe.GetLinDp() > pclTechnicalParameter->GetPipeMaxDp() || selPipe.GetLinDp() < pclTechnicalParameter->GetPipeMinDp() )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );
				}
			}
		}
		break;

		case BS_PICVColumnID::BS_PICVCID_PipeV:
		{
			CSelectPipe selPipe( &m_clBatchSelPIBCVParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclPIControlValve->GetSizeKey(), selPipe );
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_PICVColumnID::BS_PICVCID_PipeV, selPipe.GetU(), 
					iPhysicalType, iMaxDigit, iMinDecimal, true );
			
			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelPIBCVParams.m_pTADS->GetpTechParams();

			if( NULL != pclCellBase && NULL != pclTechnicalParameter )
			{
				if( selPipe.GetU() > pclTechnicalParameter->GetPipeMaxVel() || selPipe.GetU() < pclTechnicalParameter->GetPipeMinVel() )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );

					// HYS-1878: 1- Add tooltip for pipe velocity.
					CString str2 = CString( _T( "[" ) ) + WriteCUDouble( _U_VELOCITY, pclTechnicalParameter->GetPipeMinVel() );
					str2 += CString( _T( " - " ) ) + WriteCUDouble( _U_VELOCITY, pclTechnicalParameter->GetPipeMaxVel() );
					str2 += CString( _T( "]" ) );
					FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelPIBCVParams.m_eProductSubCategory );
				}
			}
		}
		break;

		case BS_PICVColumnID::BS_PICVCID_ActrName:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrName, pclElectroActuator->GetName(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrName, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrCloseOffDp:
			if( NULL != pclElectroActuator )
			{
				double dCloseOffDp = -1;
				CDB_CloseOffChar *pCloseOffChar = ( CDB_CloseOffChar * )( pclPIControlValve->GetCloseOffCharIDPtr().MP );

				if( NULL != pCloseOffChar && pCloseOffChar->GetLimitType() == CDB_CloseOffChar::CloseOffDp )
				{
					dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );
				}

				bool bRed = false;

				if( -1.0 == dCloseOffDp )
				{
					pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrCloseOffDp, _T("-"), true );
				}
				else
				{
					DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_PICVColumnID::BS_PICVCID_ActrCloseOffDp, 
							dCloseOffDp, iPhysicalType, iMaxDigit, iMinDecimal, true );

					if( NULL != pclCellBase )
					{
						// If there is a Dp on the control valve BUT Dp is above the close-off Dp of the actuator...
						if( -1.0 != pclRowParameters->m_dDp && pclRowParameters->m_dDp > dCloseOffDp )
						{
							pclCellBase->SetTextForegroundColor( _RED );
						}
					}
				}
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrCloseOffDp, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrMaxInletPressure:
			if( NULL != pclElectroActuator )
			{
				double dMaxInletPressure = -1.0;
				CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pclPIControlValve->GetCloseOffCharIDPtr().MP );

				if( NULL != pCloseOffChar && CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
				{
					dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclElectroActuator->GetMaxForceTorque() );
				}

				if( -1.0 != dMaxInletPressure )
				{
					pclBSMessage->SetCellNumber(BS_PICVColumnID::BS_PICVCID_ActrMaxInletPressure, dMaxInletPressure, iPhysicalType, iMaxDigit,
						iMinDecimal, true);
				}
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrMaxInletPressure, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrActuatingTime:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrActuatingTime, pclElectroActuator->GetActuatingTimesStr( pclPIControlValve->GetStroke() ), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrActuatingTime, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrIP:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrIP, pclElectroActuator->GetIPxxFull(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrIP, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrPowSupply:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrPowSupply, pclElectroActuator->GetPowerSupplyStr(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrPowSupply, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrInputSig:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrInputSig, pclElectroActuator->GetInOutSignalsStr( true ), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrInputSig, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrOutputSig:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrOutputSig, pclElectroActuator->GetInOutSignalsStr( false ), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrOutputSig, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrFailSafe:
			if( NULL != pclElectroActuator )
			{
				if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
				{
					str = TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
				}
				else if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
				{
					str = TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_NO );
				}

				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrFailSafe, str, true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrFailSafe, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrDRP:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrDRP, pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrDRP, _T("-"), true );
			}

			break;

		case BS_PICVColumnID::BS_PICVCID_ActrAdapterName:
			if( ( NULL != pclAdaptActuator ) && ( pclAdaptActuator->IsAnAccessory() ) )
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrAdapterName, pclAdaptActuator->GetName(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_PICVColumnID::BS_PICVCID_ActrAdapterName, _T("-"), true );
			}

			break;
	}
}

bool CDlgBatchSelPICv::_CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters )
{
	// Here, we can have either a best result that is saved in the 'pclRowParameters->m_pclSelectedProduct' variable or we can have more that one result.
	// If we have a best result, we save it in a 'CDS_SSelPICv' object and we call the 'RViewSSelPICV::Suggest' method in edit mode. 
	// Otherwise, we call the 'RViewSSelPICV::Suggest' method in normal mode to display all results and let the user to choose one.

	if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()
			|| NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe() )
	{
		ASSERTA_RETURN( false );
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( m_clBatchSelPIBCVParams.m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( false );
	}

	int iDN65 = pclTableDN->GetSize( _T("DN_65") );
	
	// This method will just copy the variables that are in the 'CProductSelectionParameters' base class.
	m_clIndSelPIBCVParams.CopyFrom( &m_clBatchSelPIBCVParams );

	// We need to do it manually for all variables of 'm_clBatchSelPIBCVParams' that are needed by the 'RViewSSelPICV'.

	// All variables in for 'CIndividualSelectionParameters'.
	m_clIndSelPIBCVParams.m_dFlow = pclRowParameters->m_dFlow;
	m_clIndSelPIBCVParams.m_dDp = pclRowParameters->m_dDp;

	CDB_Pipe *pclBestPipe =  pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe();
	int iSize = pclBestPipe->GetSizeKey( m_clBatchSelPIBCVParams.m_pTADB );

	if( iSize < iDN65 )
	{
		m_clIndSelPIBCVParams.m_strComboTypeID = m_clBatchSelPIBCVParams.m_strComboTypeBelow65ID;
		m_clIndSelPIBCVParams.m_strComboFamilyID = m_clBatchSelPIBCVParams.m_strComboFamilyBelow65ID;
		m_clIndSelPIBCVParams.m_strComboMaterialID = m_clBatchSelPIBCVParams.m_strComboMaterialBelow65ID;
		m_clIndSelPIBCVParams.m_strComboConnectID = m_clBatchSelPIBCVParams.m_strComboConnectBelow65ID;
		m_clIndSelPIBCVParams.m_strComboVersionID = m_clBatchSelPIBCVParams.m_strComboVersionBelow65ID;
		m_clIndSelPIBCVParams.m_strComboPNID = m_clBatchSelPIBCVParams.m_strComboPNBelow65ID;
	}
	else
	{
		m_clIndSelPIBCVParams.m_strComboTypeID = m_clBatchSelPIBCVParams.m_strComboTypeAbove50ID;
		m_clIndSelPIBCVParams.m_strComboFamilyID = m_clBatchSelPIBCVParams.m_strComboFamilyAbove50ID;
		m_clIndSelPIBCVParams.m_strComboMaterialID = m_clBatchSelPIBCVParams.m_strComboMaterialAbove50ID;
		m_clIndSelPIBCVParams.m_strComboConnectID = m_clBatchSelPIBCVParams.m_strComboConnectAbove50ID;
		m_clIndSelPIBCVParams.m_strComboVersionID = m_clBatchSelPIBCVParams.m_strComboVersionAbove50ID;
		m_clIndSelPIBCVParams.m_strComboPNID = m_clBatchSelPIBCVParams.m_strComboPNAbove50ID;
	}

	m_clIndSelPIBCVParams.m_eFilterSelection = m_clBatchSelPIBCVParams.m_eFilterSelection;
	
	// All variables in for 'CIndSelCtrlParamsBase'.
	m_clIndSelPIBCVParams.m_eCV2W3W = m_clBatchSelPIBCVParams.m_eCV2W3W;
	m_clIndSelPIBCVParams.m_eCvCtrlType = m_clBatchSelPIBCVParams.m_eCvCtrlType;
	m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID = m_clBatchSelPIBCVParams.m_strActuatorPowerSupplyID;
	m_clIndSelPIBCVParams.m_strActuatorInputSignalID = m_clBatchSelPIBCVParams.m_strActuatorInputSignalID;
	m_clIndSelPIBCVParams.m_iActuatorFailSafeFunction = m_clBatchSelPIBCVParams.m_iActuatorFailSafeFunction;
	m_clIndSelPIBCVParams.m_eActuatorDRPFunction = m_clBatchSelPIBCVParams.m_eActuatorDRPFunction;

	// Create now the 'CSelectPICVList' that 'RViewSSelPICV' needs.
	if( NULL != m_clIndSelPIBCVParams.m_pclSelectPIBCVList )
	{
		delete m_clIndSelPIBCVParams.m_pclSelectPIBCVList;
		m_clIndSelPIBCVParams.m_pclSelectPIBCVList = NULL;
	}

	m_clIndSelPIBCVParams.m_pclSelectPIBCVList = new CSelectPICVList();

	if( NULL == m_clIndSelPIBCVParams.m_pclSelectPIBCVList )
	{
		ASSERTA_RETURN( false );
	}
	
	// We now transfer results from the batch selection list to the individual selection list.
	pclRowParameters->m_pclBatchResults->TransferResultsToIndividualSelectList( m_clIndSelPIBCVParams.m_pclSelectPIBCVList );

	// Create an 'CDS_SSelPICv' to allow 'RViewSSelPICV' to be in edit mode with this object selected.
	// Remark: Only if we have only one result.
	if( NULL != pclRowParameters->m_pclSelectedProduct )
	{
		CDS_SSelPICv *pclSSelPICv = NULL;

		if( NULL == pclRowParameters->m_pclCDSSelSelected )
		{
			pclSSelPICv = _BS_CreateSSelPICv( pclRowParameters );
		}
		else
		{
			pclSSelPICv = dynamic_cast<CDS_SSelPICv *>( pclRowParameters->m_pclCDSSelSelected );
		}

		if( NULL == pclSSelPICv )
		{
			return false;
		}

		pclRowParameters->m_pclCDSSelSelected = pclSSelPICv;
		m_clIndSelPIBCVParams.m_SelIDPtr = pclSSelPICv->GetIDPtr();
	}

	return true;
}
