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
#include "RViewSSelBCv.h"

#include "DlgBatchSelCtrlBase.h"
#include "DlgBatchSelBCv.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelBCv::CDlgBatchSelBCv( CWnd *pParent )
	: CDlgBatchSelCtrlBase( m_clBatchSelBCVParams, CDlgBatchSelBCv::IDD, pParent )
{
	m_clBatchSelBCVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_BalAndCtrlValve;
	m_clBatchSelBCVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;
}

CDlgBatchSelBCv::~CDlgBatchSelBCv()
{
}

void CDlgBatchSelBCv::SaveSelectionParameters()
{
	CDlgBatchSelCtrlBase::SaveSelectionParameters();

	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelBCVParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return;
	}

	pclBatchSelParameter->SetBCvFlowRadioState( (int)m_clBatchSelBCVParams.m_eFlowOrPowerDTMode );
	pclBatchSelParameter->SetBCvDpCheckboxState( (int)m_clBatchSelBCVParams.m_bIsDpGroupChecked );
	pclBatchSelParameter->SetBCvCtrlTypeStrictChecked( m_clBatchSelBCVParams.m_bIsCtrlTypeStrictChecked );
	pclBatchSelParameter->SetBCVCB2W3W( m_clBatchSelBCVParams.m_eCV2W3W );
	pclBatchSelParameter->SetBCvCBCtrlType( m_clBatchSelBCVParams.m_eCvCtrlType );
	
	pclBatchSelParameter->SetBCvTypeBelow65ID( m_clBatchSelBCVParams.m_strComboTypeBelow65ID );
	pclBatchSelParameter->SetBCvFamilyBelow65ID( m_clBatchSelBCVParams.m_strComboFamilyBelow65ID );
	pclBatchSelParameter->SetBCvMaterialBelow65ID( m_clBatchSelBCVParams.m_strComboMaterialBelow65ID );
	pclBatchSelParameter->SetBCvConnectBelow65ID( m_clBatchSelBCVParams.m_strComboConnectBelow65ID );
	pclBatchSelParameter->SetBCvVersionBelow65ID( m_clBatchSelBCVParams.m_strComboVersionBelow65ID );
	pclBatchSelParameter->SetBCvPNBelow65ID( m_clBatchSelBCVParams.m_strComboPNBelow65ID );
	
	pclBatchSelParameter->SetBCvTypeAbove50ID( m_clBatchSelBCVParams.m_strComboTypeAbove50ID );
	pclBatchSelParameter->SetBCvFamilyAbove50ID( m_clBatchSelBCVParams.m_strComboFamilyAbove50ID );
	pclBatchSelParameter->SetBCvMaterialAbove50ID( m_clBatchSelBCVParams.m_strComboMaterialAbove50ID );
	pclBatchSelParameter->SetBCvConnectAbove50ID( m_clBatchSelBCVParams.m_strComboConnectAbove50ID );
	pclBatchSelParameter->SetBCvVersionAbove50ID( m_clBatchSelBCVParams.m_strComboVersionAbove50ID );
	pclBatchSelParameter->SetBCvPNAbove50ID( m_clBatchSelBCVParams.m_strComboPNAbove50ID );
	
	pclBatchSelParameter->SetBCvActPowerSupplyID( m_clBatchSelBCVParams.m_strActuatorPowerSupplyID );
	pclBatchSelParameter->SetBCvActInputSignalID( m_clBatchSelBCVParams.m_strActuatorInputSignalID );
	pclBatchSelParameter->SetBCvFailSafeFct( m_clBatchSelBCVParams.m_iActuatorFailSafeFunction );
	pclBatchSelParameter->SetBCvDRPFct( m_clBatchSelBCVParams.m_eActuatorDRPFunction );

	BS_WriteAllColumnWidth( pclBatchSelParameter );
}

bool CDlgBatchSelBCv::ResetRightView()
{
	if( NULL == pRViewSSelBCv )
	{
		return false;
	}

	if( true == pRViewSSelBCv->IsEmpty() )
	{
		return true;
	}

	pRViewSSelBCv->Reset();
	return true;
}

bool CDlgBatchSelBCv::OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList )
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

	for( int iLoopColumn = BS_BCVCID_First; iLoopColumn < BS_BCVCID_Last; iLoopColumn++ )
	{
		CDlgBatchSelectionOutput::ColData rColData;

		if( BS_BCVCID_Name == iLoopColumn || BS_BCVCID_ActrName == iLoopColumn || BS_BCVCID_ActrAdapterName == iLoopColumn )
		{
			// Insert before the control valve title and a separator.
			rColData.m_lColumnID = -1;

			rColData.m_strName = TASApp.LoadLocalizedString( ( BS_BCVCID_Name == iLoopColumn ) ? IDS_DLGBATCHSELBCV_CTXTMENUVALVEOUTPUT :
					IDS_DLGBATCHSELBCV_CTXTMENUACTROUTPUT );

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

		if( BS_BCVColumnID::BS_BCVCID_ImgCharacteristic == iLoopColumn || BS_BCVColumnID::BS_BCVCID_ImgPushClose == iLoopColumn )
		{
			if( BS_BCVColumnID::BS_BCVCID_ImgCharacteristic == iLoopColumn )
			{
				rColData.m_strName = TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_CTXTMENUIMGCHAR );
			}
			else
			{
				rColData.m_strName = TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_CTXTMENUIMGPUSHCLOSE );
			}
		}
		else
		{
			rColData.m_strName = m_mapColumnList[iLoopColumn].m_strHeaderName;
		}

		rColData.m_bAlreadyDisplayed = m_mapColumnList[iLoopColumn].m_bDisplayed;

		// Show 'BS_BCVCID_DpFullOpening' and 'BS_BCVCID_DpHalfOpening' items in context menu ONLY if user has not input 'Dp' values.
		if( BS_BCVColumnID::BS_BCVCID_DpFullOpening == iLoopColumn || BS_BCVColumnID::BS_BCVCID_DpHalfOpening == iLoopColumn )
		{
			rColData.m_bEnable = !m_clBatchSelBCVParams.m_bIsDpGroupChecked;
		}

		rColData.m_bSeparator = false;
		mapColumnList[iInsert++] = rColData;
	}

	return true;
}

bool CDlgBatchSelBCv::OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct )
{
	if( false == CDlgBatchSelCtrlBase::OnBatchSelectionIsCatalogExist( iColumnID, lRowRelative, bCatExist, pclProduct ) )
	{
		return false;
	}

	if( iColumnID >= BS_BCVColumnID::BS_BCVCID_Name && iColumnID < BS_BCVColumnID::BS_BCVCID_PipeSize
			&& NULL != m_mapAllRowData[lRowRelative].m_pclSelectedProduct )
	{
		// Check valve.
		CDB_TAProduct *pclTAProduct = ( CDB_TAProduct * )( m_mapAllRowData[lRowRelative].m_pclSelectedProduct->GetProductIDPtr().MP );
		bCatExist = ( true == pclTAProduct->VerifyOneCatExist() );
		pclProduct = pclTAProduct;
	}
	else if( iColumnID >= BS_BCVColumnID::BS_BCVCID_ActrName && iColumnID < BS_BCVColumnID::BS_BCVCID_Last
			&& NULL != m_mapAllRowData[lRowRelative].m_pclSelectedActuator )
	{
		// Check actuator.
		CDB_Actuator *pclActuator = m_mapAllRowData[lRowRelative].m_pclSelectedActuator;
		bCatExist = ( true == pclActuator->VerifyOneCatExist() );
		pclProduct = pclActuator;
	}

	return true;
}

bool CDlgBatchSelBCv::OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelBCv )
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
	if( false == m_clInterface.IsRowValidForData( lRowRelative, (int)m_clBatchSelBCVParams.m_eProductSubCategory ) )
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
		return false;
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

	pRViewSSelBCv->Invalidate();
	pRViewSSelBCv->UpdateWindow();
	pRViewSSelBCv->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );

	m_clInterface.SelectRow( lRowRelative, true, (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	_BS_DisplaySolutions();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelBCv, CDlgBatchSelCtrlBase )
	ON_COMMAND( IDC_CHECKDP, OnBnClickedCheckDp )
	ON_COMMAND( IDC_CHECKSTRICT, OnBnClickedCheckStrict )
	ON_CBN_SELCHANGE( IDC_COMBO2W3W, OnCbnSelChange2w3w )
	ON_CBN_SELCHANGE( IDC_COMBOCTRLTYPE, OnCbnSelChangeCtrlType )
END_MESSAGE_MAP()

void CDlgBatchSelBCv::DoDataExchange( CDataExchange *pDX )
{
	CDlgBatchSelCtrlBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKDP, m_clCheckboxDp );
	DDX_Control( pDX, IDC_CHECKSTRICT, m_clCheckboxStrict );
	DDX_Control( pDX, IDC_COMBO2W3W, m_Combo2w3w );
	DDX_Control( pDX, IDC_COMBOCTRLTYPE, m_ComboCtrlType );
}

BOOL CDlgBatchSelBCv::OnInitDialog()
{
	CDlgBatchSelCtrlBase::OnInitDialog();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_BATCHSELBCV_DP );
	m_clCheckboxDp.SetWindowText( str );
	m_clCheckboxDp.SetCheck( BST_UNCHECKED );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBCV_CHECKSTRICT );
	m_clCheckboxStrict.SetWindowText( str );
	m_clCheckboxStrict.SetCheck( BST_UNCHECKED );

	str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_STATIC2W3W );
	GetDlgItem( IDC_STATIC2W3W )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_STATICCTRLTYPE );
	GetDlgItem( IDC_STATICCTRLTYPE )->SetWindowText( str );
	
	return TRUE;
}

void CDlgBatchSelBCv::OnBnClickedCheckDp()
{
	m_clBatchSelBCVParams.m_bIsDpGroupChecked = ( BST_CHECKED == m_clCheckboxDp.GetCheck() ) ? true : false;

	if( 0 == m_mapAllRowData.size()
			|| BS_InputsVerificationStatus::BS_IVS_Empty == m_eInputsVerificationStatus
			|| BS_InputsVerificationStatus::BS_IVS_NotYetDone == m_eInputsVerificationStatus )
	{
		// When there is not yet data pasted in the batch selection output, user can change input method (Flow or Power/DT and Dp or not).
		BS_UpdateInputColumnLayout( BS_UpdateInputColumn::BS_UIC_Dp );
	}
}

void CDlgBatchSelBCv::OnBnClickedCheckStrict()
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

	m_clBatchSelBCVParams.m_bIsCtrlTypeStrictChecked = ( BST_CHECKED == m_clCheckboxStrict.GetCheck() ) ? true : false;
}

void CDlgBatchSelBCv::OnCbnSelChange2w3w()
{
	if( m_clBatchSelBCVParams.m_eCV2W3W == (CDB_ControlProperties::CV2W3W)m_Combo2w3w.GetItemData( m_Combo2w3w.GetCurSel() ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_Combo2w3w.FindItemDataPtr( ( DWORD_PTR )m_clBatchSelBCVParams.m_eCV2W3W ), 0 );

		if( iPos != m_Combo2w3w.GetCurSel() )
		{
			m_Combo2w3w.SetCurSel( iPos );
		}

		return;
	}

	m_clBatchSelBCVParams.m_eCV2W3W = CDB_ControlProperties::CV2W3W::CV2W;		// By default in case of error
	int iCur = m_Combo2w3w.GetCurSel();

	if( iCur >= 0 )
	{
        m_clBatchSelBCVParams.m_eCV2W3W = (CDB_ControlProperties::CV2W3W)m_Combo2w3w.GetItemData( iCur );
	}

	_FillComboCtrlType();
	OnCbnSelChangeCtrlType();
}

void CDlgBatchSelBCv::OnCbnSelChangeCtrlType()
{
	if( m_clBatchSelBCVParams.m_eCvCtrlType == (CDB_ControlProperties::CvCtrlType)m_ComboCtrlType.GetItemData( m_ComboCtrlType.GetCurSel() ) )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboCtrlType.FindItemDataPtr( ( DWORD_PTR )m_clBatchSelBCVParams.m_eCvCtrlType ), 0 );

		if( iPos != m_ComboCtrlType.GetCurSel() )
		{
			m_ComboCtrlType.SetCurSel( iPos );
		}

		return;
	}

	m_clBatchSelBCVParams.m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvProportional;		// By default in case of error
	int iCur = m_ComboCtrlType.GetCurSel();

	if( iCur >= 0 )
	{
        m_clBatchSelBCVParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)m_ComboCtrlType.GetItemData( iCur );
	}

	// Change all combos for size below 65.
	FillComboBoxType( AboveOrBelow::Below65 );
	FillComboBoxFamily( AboveOrBelow::Below65 );
	FillComboBoxBodyMaterial( AboveOrBelow::Below65 );
	FillComboBoxConnect( AboveOrBelow::Below65 );
	FillComboBoxVersion( AboveOrBelow::Below65 );
	FillComboBoxPN( AboveOrBelow::Below65 );

	// Change all combos for size above 50.
	FillComboBoxType( AboveOrBelow::Above50 );
	FillComboBoxFamily( AboveOrBelow::Above50 );
	FillComboBoxBodyMaterial( AboveOrBelow::Above50 );
	FillComboBoxConnect( AboveOrBelow::Above50 );
	FillComboBoxVersion( AboveOrBelow::Above50 );
	FillComboBoxPN( AboveOrBelow::Above50 );

	// To allow the 'CDlgBatchSelCtrlBase' class to change combos for the actuator.
	EndOfComboChange();

	// Reset to 'false' to allow to clear and suggest new results.
	m_bMsgDoNotClearResult = false;
	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBCv::OnBnClickedSuggest()
{
	ClearAll();

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

void CDlgBatchSelBCv::OnBnClickedValidate()
{
	CDlgBatchSelCtrlBase::OnBnClickedValidate();

	if( true == m_clBatchSelBCVParams.m_bEditModeRunning )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
		}

		_BS_ApplySelection();

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
		}
	}
}

void CDlgBatchSelBCv::OnBnClickedCancel()
{
	// User has edited a valid selection to change it but finally cancel it.
	OnRViewSSelKeyboardEvent( VK_ESCAPE );
	ResetRightView();
}

LRESULT CDlgBatchSelBCv::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelCtrlBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelBCVParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return -1;
	}

	// Fill the combo box.
	// Remark: Corresponding combo variables in 'm_clBatchSelBCVParams' are updated in each of this following methods.
	_FillCombo2w3w( pclBatchSelParameter->GetBCvCB2W3W() );
	_FillComboCtrlType( pclBatchSelParameter->GetBCvCBCtrlType() );

	FillComboBoxType( AboveOrBelow::Below65, pclBatchSelParameter->GetBCvTypeBelow65ID() );
	FillComboBoxFamily( AboveOrBelow::Below65, pclBatchSelParameter->GetBCvFamilyBelow65ID() );
	FillComboBoxBodyMaterial( AboveOrBelow::Below65, pclBatchSelParameter->GetBCvMaterialBelow65ID() );
	FillComboBoxConnect( AboveOrBelow::Below65, pclBatchSelParameter->GetBCvConnectBelow65ID() );
	FillComboBoxVersion( AboveOrBelow::Below65, pclBatchSelParameter->GetBCvVersionBelow65ID() );
	FillComboBoxPN( AboveOrBelow::Below65, pclBatchSelParameter->GetBCvPNBelow65ID() );

	FillComboBoxType( AboveOrBelow::Above50, pclBatchSelParameter->GetBCvTypeAbove50ID() );
	FillComboBoxFamily( AboveOrBelow::Above50, pclBatchSelParameter->GetBCvFamilyAbove50ID() );
	FillComboBoxBodyMaterial( AboveOrBelow::Above50, pclBatchSelParameter->GetBCvMaterialAbove50ID() );
	FillComboBoxConnect( AboveOrBelow::Above50, pclBatchSelParameter->GetBCvConnectAbove50ID() );
	FillComboBoxVersion( AboveOrBelow::Above50, pclBatchSelParameter->GetBCvVersionAbove50ID() );
	FillComboBoxPN( AboveOrBelow::Above50, pclBatchSelParameter->GetBCvPNAbove50ID() );

	FillComboPowerSupply( pclBatchSelParameter->GetBCvActPowerSupplyID() );
	FillComboInputSignal( pclBatchSelParameter->GetBCvActInputSignalID() );
	int iCheck = -1;
	if( 1 == pclBatchSelParameter->GetBCvFailSafeFct() )
	{
		iCheck = BST_CHECKED;
	}
	else if( 0 == pclBatchSelParameter->GetBCvFailSafeFct() )
	{
		iCheck = BST_UNCHECKED;
	}
	ASSERT( iCheck != -1 );
	m_CheckFailSafeFct.SetCheck( iCheck );
	m_clBatchSelBCVParams.m_iActuatorFailSafeFunction = pclBatchSelParameter->GetBCvFailSafeFct();
	VerifyCheckboxFailsafeStatus();
	FillComboDRPFct( pclBatchSelParameter->GetBCvDRPFct() );

	// Set the 'Flow' radio state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclBatchSelParameter->GetBCvRadioFlowState() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clBatchSelBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgBatchSelCtrlBase::UpdateFlowOrPowerDTState();

	// Set the 'Dp' check box state.
	m_clBatchSelBCVParams.m_bIsDpGroupChecked = ( 0 == pclBatchSelParameter->GetBCvDpCheckboxState() ) ? false : true;
	m_clCheckboxDp.SetCheck( ( true == m_clBatchSelBCVParams.m_bIsDpGroupChecked ) ? BST_CHECKED : BST_UNCHECKED );

	// Set the 'Control type strict' check box state.
	m_clBatchSelBCVParams.m_bIsCtrlTypeStrictChecked = pclBatchSelParameter->GetBCvCtrlTypeStrictChecked();
	m_clCheckboxStrict.SetCheck( ( true == m_clBatchSelBCVParams.m_bIsCtrlTypeStrictChecked ) ? BST_CHECKED : BST_UNCHECKED );

	// This is needed only in the case of the dialog is currently active when user creates a new document (or opens a new one).
	OnAfterNewDocument();

	m_bInitialised = true;
	return 0;
}

CRViewSSelSS *CDlgBatchSelBCv::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelBCv;
}

void CDlgBatchSelBCv::ClearAll( void )
{
	if( NULL == pRViewSSelBCv )
	{
		return;
	}

	// Disable the 'Validate' button.
	if( false == m_clBatchSelBCVParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	CDlgSelectionBase::ClearAll();
}

void CDlgBatchSelBCv::BS_PrepareColumnList( void )
{
	CDlgBatchSelCtrlBase::BS_PrepareColumnList();

	BSColParameters rColParameters;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Prepare header and data parameters.
	rColParameters.m_clHeaderColDef.SetAllParams( DlgOutputHelper::CColDef::ContentType::Text, 10.0, BS_BCVMainHeaderID::BS_BCVMHID_ValveActuator,
			BS_BCVMainHeaderSubID::BS_BCVMHSUBID_Valve, DlgOutputHelper::CColDef::AutoResizeDisabled,
			DlgOutputHelper::CColDef::Visible, DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionDisabled,
			DlgOutputHelper::CColDef::BlockSelectionDisabled,
			DlgOutputHelper::CColDef::RowSelectionDisabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );
	
	rColParameters.m_clHeaderColDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );

	rColParameters.m_clHeaderFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Center, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled,
			DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_clDataColDef.SetAllParams( DlgOutputHelper::CColDef::ContentType::Text, 10.0, BS_BCVMainHeaderID::BS_BCVMHID_ValveActuator,
			BS_BCVMainHeaderSubID::BS_BCVMHSUBID_Valve, DlgOutputHelper::CColDef::AutoResizeDisabled,
			DlgOutputHelper::CColDef::Visible, DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionEnabled,
			DlgOutputHelper::CColDef::BlockSelectionEnabled,
			DlgOutputHelper::CColDef::RowSelectionEnabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataEnabled );
	rColParameters.m_clDataColDef.SetRowSeparatorFlag( true, DLGBATCHSELBASE_COLOR_DATAROWSEPARATOR );

	rColParameters.m_clDataFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Left, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled,
			DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	// Columns linked to the valve.
	rColParameters.m_iColumnID = BS_BCVCID_Name;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_bLineBelowHeader = true;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_Material;
	rColParameters.m_bCantRemove = false;
	rColParameters.m_clHeaderColDef.SetWidth( 12.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_clDataColDef.SetWidth( 12.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_Connection;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_Version;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_VERSION );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_Size;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHVALVESIZE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_PN;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_Rangeability;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_RANGEABILITY );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_LeakageRate;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_LEAKAGERATE );
	rColParameters.m_strHeaderUnit = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_LEAKAGERATEUNIT );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_Stroke;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIAMETER );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_CVSTROKE );
	rColParameters.m_strHeaderUnit = _T( "" );	// Unit will be set in regards to the control valve just near the stroke value.
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ImgCharacteristic;
	rColParameters.m_clHeaderColDef.SetWidth( 2.0 );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Bitmap );
	rColParameters.m_clDataColDef.SetWidth( 2.0 );
	rColParameters.m_strHeaderName = _T( "" );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ImgPushClose;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_Preset;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Number );
	rColParameters.m_clDataColDef.SetMaxDigit( 2 );
	rColParameters.m_clDataColDef.SetMinDecimal( 2 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PRESET );
	rColParameters.m_strHeaderUnit = TASApp.LoadLocalizedString( IDS_SHEETHDR_TURNSPOS );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_Dp;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DP );
	rColParameters.m_strHeaderUnit = _T( "" );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_DpFullOpening;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPFO );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_DpHalfOpening;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPHO );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_TemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_PipeSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPESIZE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_PipeLinDp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_LINPRESSDROP );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPELINDP );
	rColParameters.m_iHeaderUnit = _U_LINPRESSDROP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_PipeV;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_VELOCITY );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPEV );
	rColParameters.m_iHeaderUnit = _U_VELOCITY;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	// Columns linked to the actuator.
	rColParameters.m_iColumnID = BS_BCVCID_ActrName;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clHeaderColDef.SetMainHeaderLinks( BS_BCVMainHeaderID::BS_BCVMHID_ValveActuator, BS_BCVMainHeaderSubID::BS_BCVMHSUBID_Actuator );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRNAME );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrCloseOffDp;
	rColParameters.m_bCantRemove = false;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRCLOSEOFFDP );
	rColParameters.m_strHeaderUnit = _T( "" );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrMaxInletPressure;
	rColParameters.m_bCantRemove = false;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_PRESSURE );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXINLETPRESS );
	rColParameters.m_strHeaderUnit = _T( "" );
	rColParameters.m_iHeaderUnit = _U_PRESSURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrActuatingTime;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTUATINGTIME );
	rColParameters.m_iHeaderUnit = _U_TIME;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrIP;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRIP );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrPowSupply;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRPOWERSUP );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrInputSig;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRINPUTSIG );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrOutputSig;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTROUTPUTSIG );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrFailSafe;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRFAILSAFE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BCVCID_ActrDRP;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	// HYS-956: Display adapter column
	rColParameters.m_iColumnID = BS_BCVCID_ActrAdapterName;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRADAPTNAME );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;
}

bool CDlgBatchSelBCv::BS_InitMainHeaders( void )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelCtrlBase::BS_InitMainHeaders() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
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
	
	m_clInterface.AddMainHeaderDef( BS_BCVMainHeaderID::BS_BCVMHID_ValveActuator, BS_BCVMainHeaderSubID::BS_BCVMHSUBID_Valve,
			clColMHInDef, TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_TITLEGROUPVALVE ), (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHInDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center, 
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _IMI_GRAY_MED, _WHITE );

	m_clInterface.AddMainHeaderFontDef( BS_BCVMainHeaderID::BS_BCVMHID_ValveActuator, BS_BCVMainHeaderSubID::BS_BCVMHSUBID_Valve,
			clFontMHInDef, (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	// Add main header for 'Actuator'.
	DlgOutputHelper::CColDef clColMHOutDef;
	clColMHOutDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHOutDef.SetMouseEventFlag( true );
	clColMHOutDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );
	
	// To specify that the group actuator is linked to the main header 'Output'.
	clColMHOutDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output );
	
	m_clInterface.AddMainHeaderDef( BS_BCVMainHeaderID::BS_BCVMHID_ValveActuator, BS_BCVMainHeaderSubID::BS_BCVMHSUBID_Actuator,
			clColMHOutDef, TASApp.LoadLocalizedString( IDS_DLGBATCHSELBCV_TITLEGROUPACTR ), (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHOutDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center,
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _IMI_GRAY_MED, _WHITE );

	m_clInterface.AddMainHeaderFontDef( BS_BCVMainHeaderID::BS_BCVMHID_ValveActuator, BS_BCVMainHeaderSubID::BS_BCVMHSUBID_Actuator,
			clFontMHOutDef, (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	m_clInterface.SetMainHeaderRowHeight( BS_BCVMainHeaderID::BS_BCVMHID_ValveActuator, DLBBATCHSELBCV_ROWHEIGHT_MAINHEADER,
			(int)m_clBatchSelBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelBCv::BS_InitColumns()
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelCtrlBase::BS_InitColumns() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
		}

		return false;
	}

	BS_AddOneColumn( BS_BCVColumnID::BS_BCVCID_Name );
	BS_AddOneColumn( BS_BCVColumnID::BS_BCVCID_ActrName );
	BS_AddOneColumn( BS_BCVColumnID::BS_BCVCID_ActrAdapterName );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelBCv::BS_Validate()
{
	try
	{
		if( false == CDlgBatchSelCtrlBase::BS_Validate() )
		{
			return false;
		}

		CTable *pTab = dynamic_cast<CTable *>( m_clBatchSelBCVParams.m_pTADS->Get( _T("CTRLVALVE_TAB") ).MP );

		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CTRLVALVE_TAB' table from the datastruct.") );
		}

		bool bAtLeastOneInserted = false;

		for( mapLongRowParamsIter iterRow = m_mapAllRowData.begin(); iterRow != m_mapAllRowData.end(); iterRow++ )
		{
			BSRowParameters *pclRowParameters = &iterRow->second;

			if( RowStatus::BS_RS_FindOneSolution != pclRowParameters->m_eStatus 
					&& RowStatus::BS_RS_FindOneSolutionAlter != pclRowParameters->m_eStatus )
			{
				continue;
			}

			CDS_SSelBCv *pSSelBCv = (CDS_SSelBCv *)pclRowParameters->m_pclCDSSelSelected;

			if( NULL == pSSelBCv )
			{
				// Create selected object and initialize it.
				pSSelBCv = _BS_CreateSSelBCv( pclRowParameters );
			}

			if( NULL == pSSelBCv )
			{
				continue;
			}

			IDPTR IDPtr = pSSelBCv->GetIDPtr();
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
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelBCv::BS_Validate'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelBCv::BS_EnableCombos( bool bEnable )
{
	CDlgBatchSelCtrlBase::BS_EnableCombos( bEnable );

	m_Combo2w3w.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_Combo2w3w.GetCount() <= 1 )
	{
		m_Combo2w3w.EnableWindow( FALSE );
	}
	
	GetDlgItem( IDC_STATIC2W3W )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

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

	// HYS-1079 : When all field are setted to true we have to check for fail-safe field the m_CheckStatus value before enable it
	m_CheckFailSafeFct.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	if( ( CTADatabase::FailSafeCheckStatus::eBothFailSafe != m_CheckStatus ) && TRUE == bEnable )
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

void CDlgBatchSelBCv::BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters )
	{
		return;
	}

	for( int iLoop = BS_BCVColumnID::BS_BCVCID_First; iLoop < BS_BCVColumnID::BS_BCVCID_Last; iLoop++ )
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

bool CDlgBatchSelBCv::BS_SuggestSelection()
{
	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL == pclBSMessage )
	{
		return false;
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_clBatchSelBCVParams.m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	// We block the redraw because it is too long otherwise.
	BeginWaitCursor();
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;

	// Variable uses in test unit to drop out results in a file.
	m_clBatchSelBCVParams.m_vecRowParameters.clear();

	for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
	{
		BSRowParameters *pclRowParameters = &iterRows->second;

		if( NULL == pclRowParameters )
		{
			continue;
		}

		// Put this 'pclRowParameters' in the 'CBatchSelectionParameters::m_vecRowParameters' to facilitate test unit.
		m_clBatchSelBCVParams.m_vecRowParameters.push_back( pclRowParameters );

		// 'CBatchSelectBCVList' is created and the pointer is saved in 'pclRowParameters->m_pclBatchResults' for each row. But the pointer is
		// also saved in the 'm_clBatchSelBCVParams.m_pclBatchBCVList' variable to help us calling 'SelectBCV' method. This is the 
		// 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods that will delete this pointer. 
		// Thus no need to do it in the 'CBatchSelBCVParams' destructor.
		pclRowParameters->m_pclBatchResults = new CBatchSelectBCVList();

		if( NULL == pclRowParameters->m_pclBatchResults )
		{
			continue;
		}

		m_clBatchSelBCVParams.m_pclBatchBCVList = (CBatchSelectBCVList*)pclRowParameters->m_pclBatchResults;
		long lCurrentRow = iterRows->first;
		
		pclBSMessage->Clear();

		pclRowParameters->m_pclSelectedProduct = NULL;

		if( CDS_SelProd::efdFlow != m_clBatchSelBCVParams.m_eFlowOrPowerDTMode )
		{
			if( pclRowParameters->m_dPower > 0.0 && pclRowParameters->m_dDT > 0.0 )
			{
				CWaterChar clWaterCharReturn = m_clBatchSelBCVParams.m_WC;
				double dReturnTemperature = ( ProjectType::Cooling == m_clBatchSelBCVParams.m_eApplicationType ) ? m_clBatchSelBCVParams.m_WC.GetTemp() + pclRowParameters->m_dDT :
						m_clBatchSelBCVParams.m_WC.GetTemp() - pclRowParameters->m_dDT;

				clWaterCharReturn.UpdateFluidData( dReturnTemperature );

				pclRowParameters->m_dFlow = m_clBatchSelBCVParams.m_WC.GetTemp() * m_clBatchSelBCVParams.m_WC.GetDens() * m_clBatchSelBCVParams.m_WC.GetSpecifHeat();
				pclRowParameters->m_dFlow -= ( clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat() );
				pclRowParameters->m_dFlow = abs( pclRowParameters->m_dPower / pclRowParameters->m_dFlow );
			}
		}

		int iReturn = m_clBatchSelBCVParams.m_pclBatchBCVList->SelectBCV( &m_clBatchSelBCVParams, pclRowParameters->m_dFlow, pclRowParameters->m_dDp );
		
		bool bAlternative = CBatchSelectBaseList::BR_FoundAlernative == ( iReturn & CBatchSelectBaseList::BR_FoundAlernative );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturn & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturn & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturn & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			m_bAtLeastOneRowWithNoSolution = true;
			pclRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindNoSolution;
			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_Name, _T( "" ), true );
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_OUTPUTBOXERROR, true );
			delete pclRowParameters->m_pclBatchResults;
			pclRowParameters->m_pclBatchResults = NULL;

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( IDS_BATCHSELECTION_NOSOLUTION ), 
						(int)m_clBatchSelBCVParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturn & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			int iActMatch = -1;
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelBCVParams.m_pclBatchBCVList->GetBestProduct();

			pclRowParameters->m_pclSelectedActuator = m_clBatchSelBCVParams.m_pclBatchBCVList->SelectActuator( &m_clBatchSelBCVParams,

					(CDB_ControlValve *)(pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ), pclRowParameters->m_dDp,  &iActMatch );
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
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
				int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;

				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
						(int)m_clBatchSelBCVParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturn & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			int iActMatch = -1;
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelBCVParams.m_pclBatchBCVList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelBCVParams.m_pclBatchBCVList->GetFirst<CSelectedBase>();
				
				pclRowParameters->m_pclSelectedActuator = m_clBatchSelBCVParams.m_pclBatchBCVList->SelectActuator( &m_clBatchSelBCVParams,

						(CDB_ControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ), pclRowParameters->m_dDp, &iActMatch );
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
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
					
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelBCVParams.m_eProductSubCategory );
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

				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_Name, _T( "" ), true );
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_SEVRESULTS : IDS_BATCHSELECTION_SEVRESULTSWITHINFO;
		 			
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelBCVParams.m_eProductSubCategory );
				}
			}
		}

		// Send message.
		pclBSMessage->SendMessage( lCurrentRow, true, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	EndWaitCursor();

	return CDlgBatchSelCtrlBase::BS_SuggestSelection();
}

bool CDlgBatchSelBCv::BS_AddOneColumn( int iColumnID )
{
	// Check first if we can show this column.
	if( true == m_clBatchSelBCVParams.m_bIsDpGroupChecked && ( BS_BCVColumnID::BS_BCVCID_DpFullOpening == iColumnID
			|| BS_BCVColumnID::BS_BCVCID_DpHalfOpening == iColumnID ) )
	{
		return false;
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelCtrlBase::BS_AddOneColumn( iColumnID ) )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
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
				m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
			}

			return false;
		}

		for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
		{
			long lCurrentRow = iterRows->first;
			pclBSMessage->Clear();

			_BS_FillCell( pclBSMessage, &iterRows->second, iColumnID );
			pclBSMessage->SendMessage( lCurrentRow, false, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	return true;
}

void CDlgBatchSelBCv::BS_UpdateInputColumnLayout( int iUpdateWhat )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	switch( iUpdateWhat )
	{
		case BS_UpdateInputColumn::BS_UIC_Dp:
			if( true == m_clBatchSelBCVParams.m_bIsDpGroupChecked )
			{
				// Add 'BS_BCVCID_Dp'.
				m_mapColumnList[BS_BCVColumnID::BS_BCVCID_Dp].m_bEnabled = true;

				if( eBool3::eb3True == m_mapColumnList[BS_BCVColumnID::BS_BCVCID_Dp].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BCVColumnID::BS_BCVCID_Dp );
				}

				// And remove 'BS_BCVCID_DpFullOpening' and 'BS_BCVCID_DpHalfOpening' columns if user had previously intentionally added.
				m_mapColumnList[BS_BCVColumnID::BS_BCVCID_DpFullOpening].m_bEnabled = false;
				m_mapColumnList[BS_BCVColumnID::BS_BCVCID_DpHalfOpening].m_bEnabled = false;

				if( true == m_mapColumnList[BS_BCVColumnID::BS_BCVCID_DpFullOpening].m_bDisplayed )
				{
					BS_RemoveOneColumn( BS_BCVColumnID::BS_BCVCID_DpFullOpening );
				}

				if( true == m_mapColumnList[BS_BCVColumnID::BS_BCVCID_DpHalfOpening].m_bDisplayed )
				{
					BS_RemoveOneColumn( BS_BCVColumnID::BS_BCVCID_DpHalfOpening );
				}
			}
			else
			{
				// When 'Dp' check box is unchecked, we must show 'Dp', 'Dp100%' and 'Dp50%' ONLY if user has asked to show them.
				m_mapColumnList[BS_BCVColumnID::BS_BCVCID_Dp].m_bEnabled = true;
				m_mapColumnList[BS_BCVColumnID::BS_BCVCID_DpFullOpening].m_bEnabled = true;
				m_mapColumnList[BS_BCVColumnID::BS_BCVCID_DpHalfOpening].m_bEnabled = true;

				if( eBool3::eb3True == m_mapColumnList[BS_BCVColumnID::BS_BCVCID_Dp].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BCVColumnID::BS_BCVCID_Dp );
				}
			
				if( eBool3::eb3True == m_mapColumnList[BS_BCVColumnID::BS_BCVCID_DpFullOpening].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BCVColumnID::BS_BCVCID_DpFullOpening );
				}

				if( eBool3::eb3True == m_mapColumnList[BS_BCVColumnID::BS_BCVCID_DpHalfOpening].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BCVColumnID::BS_BCVCID_DpHalfOpening );
				}
			}

			break;
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	CDlgBatchSelCtrlBase::BS_UpdateIOColumnLayout( iUpdateWhat );
}

// Remark: 'CW' is for 'Column Width'.
// HYS-1995: Save version. Version 2.
#define CW_BATCHSELBCV_VERSION			2
void CDlgBatchSelBCv::BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}

	int iVersion = CW_BATCHSELBCV_VERSION;
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = (CDS_BatchSelParameter::CCWBatchSelWindow *)pclBatchSelParameter->GetWindowColumnWidth(
				CW_WINDOWID_BATCHSELBCV, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = (CDS_BatchSelParameter::CCWBatchSelSheet *)pclCWWindow->GetCWSheet( 0 );

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

				if( iter->first >= BS_BCVCID_First )
				{
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3True;
				}
			}
			else
			{
				m_clInterface.SetColumnWidthInPixel( iter->first, iter->second.m_lWidth, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
			}
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	}
}

void CDlgBatchSelBCv::BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = (CDS_BatchSelParameter::CCWBatchSelWindow *)pclBatchSelParameter->GetWindowColumnWidth( CW_WINDOWID_BATCHSELBCV, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = (CDS_BatchSelParameter::CCWBatchSelSheet *)pclCWWindow->GetCWSheet( 0, true );

	int iAfterColumnID = -1;
	CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfo mapColumnWidth;

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); iter++ )
	{
		CDS_BatchSelParameter::CCWBatchSelSheet::ColumnInfo rColumnInfo;
		rColumnInfo.m_bIsVisible = iter->second.m_bDisplayed;

		if( true == iter->second.m_bDisplayed )
		{
			m_clInterface.GetColumnWidthInPixel( iter->first, rColumnInfo.m_lWidth, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
			rColumnInfo.m_iAfterColumnID = iAfterColumnID;
			iAfterColumnID = iter->first;
		}

		mapColumnWidth[iter->first] = rColumnInfo;
	}

	pclCWSheet->GetMap() = mapColumnWidth;
	// HYS-1995: Save version.
	pclCWSheet->SetVersion( CW_BATCHSELBCV_VERSION );
}

void CDlgBatchSelBCv::RenameColHeader()
{
	// Update Default result position column
	if( m_clBatchSelBCVParams.m_iActuatorFailSafeFunction == 1 )
	{
		m_mapColumnList[BS_BCVColumnID::BS_BCVCID_ActrDRP].m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP );
	}
	else
	{
		m_mapColumnList[BS_BCVColumnID::BS_BCVCID_ActrDRP].m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
	}

	m_clInterface.FillColumnHeader( BS_BCVColumnID::BS_BCVCID_ActrDRP, BS_RowRelativePos::BS_RID_HeaderName,
		m_mapColumnList[BS_BCVColumnID::BS_BCVCID_ActrDRP].m_strHeaderName,
		DLBBATCHSELBASE_ROWHEIGHT_HEADERNAME, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDlgBatchSelBCv::_FillCombo2w3w( CDB_ControlProperties::CV2W3W eCv2w3w )
{
	CTable *pTab = (CTable *)( m_clBatchSelBCVParams.m_pTADB->Get( _T("BALCTRLVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		ASSERT_RETURN;
	}

	m_Combo2w3w.ResetContent();
	
	// Fill an array with all possibilities.
	bool ar[CDB_ControlProperties::CV2W3W::LastCV2W3W];
	memset( ar, 0, sizeof( ar ) );

	for( IDPTR idptr = pTab->GetFirst(); NULL != *idptr.ID; idptr = pTab->GetNext() )
	{
		CDB_ControlValve *pCV =dynamic_cast<CDB_ControlValve *>( idptr.MP );

		if( NULL == pCV )
		{
			ASSERT( 0 );
			continue;
		}
		
		if( false == pCV->IsSelectable( true ) )
		{
			continue;
		}
		
		CDB_ControlProperties::CV2W3W CtrlProp2w3w = ( (CDB_ControlProperties *)pCV->GetCtrlPropIDPtr().MP )->GetCv2w3w();
		ASSERT( CtrlProp2w3w<CDB_ControlProperties::CV2W3W::LastCV2W3W );
		ar[CtrlProp2w3w] = true;
		bool bAllChecked = true;	

		for( int i = 0; i < CDB_ControlProperties::CV2W3W::LastCV2W3W && true == bAllChecked; i++ )
		{
			if( false == ar[i] )
			{
				bAllChecked = false;
			}
		}
		
		if( true == bAllChecked )
		{
			break;
		}
	}
	
	int iSelPos = 0;
	CDB_ControlProperties::CV2W3W eSelectedCV2W3W = CDB_ControlProperties::CV2W3W::LastCV2W3W;

	for( int i = 0; i < CDB_ControlProperties::CV2W3W::LastCV2W3W; i++ )
	{
		if( true == ar[i] )
		{
			CString str = CDB_ControlProperties::GetCv2W3WStr( (CDB_ControlProperties::CV2W3W)i ).c_str();
			int pos = m_Combo2w3w.AddString( (LPCTSTR)str );
            m_Combo2w3w.SetItemData( pos, (DWORD_PTR)i );

			if( i == eCv2w3w )
			{
				eSelectedCV2W3W = eCv2w3w;
				iSelPos = pos;
			}
		}
	}
	
	m_Combo2w3w.SetCurSel( iSelPos );
	m_clBatchSelBCVParams.m_eCV2W3W = (CDB_ControlProperties::CV2W3W)m_Combo2w3w.GetItemData( iSelPos );

	if( m_Combo2w3w.GetCount() <= 1 )
	{
		m_Combo2w3w.EnableWindow( false );
	}
	else
	{
		m_Combo2w3w.EnableWindow( true );
	}
}

void CDlgBatchSelBCv::_FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	CRankEx CtrlTypeList;

	m_clBatchSelBCVParams.m_pTADB->GetTACVCtrlTypeList( &CtrlTypeList, CTADatabase::eForTechHCBCv, true, m_clBatchSelBCVParams.m_eCV2W3W, L"", CDB_ControlProperties::LastCVFUNC,
			m_clBatchSelBCVParams.m_eFilterSelection );

	m_ComboCtrlType.ResetContent();
	CtrlTypeList.Transfer( &m_ComboCtrlType );
	ASSERT( m_ComboCtrlType.GetCount() );

	int iSelPos = 0;

	for( int i = 0; i < m_ComboCtrlType.GetCount(); i++ )
	{
        if( eCvCtrlType == ( CDB_ControlProperties::CvCtrlType )m_ComboCtrlType.GetItemData( i ) )
		{
			iSelPos = i;
			break;
		}
	}

	m_ComboCtrlType.SetCurSel( iSelPos );
    m_clBatchSelBCVParams.m_eCvCtrlType = ( CDB_ControlProperties::CvCtrlType )m_ComboCtrlType.GetItemData( iSelPos );

	if( m_ComboCtrlType.GetCount() <= 1 )
	{
		m_ComboCtrlType.EnableWindow( false );
	}
	else
	{
		m_ComboCtrlType.EnableWindow( true );
	}
}

bool CDlgBatchSelBCv::_BS_ApplySelection()
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelBCv )
	{
		ASSERTA_RETURN( false );
	}
	
	if( NULL == m_pclCurrentRowParameters || NULL == m_pclCurrentRowParameters->m_pclBatchResults )
	{
		return false;
	}

	CDB_TAProduct *pclTAProduct = pRViewSSelBCv->GetCurrentControlValveSelected();

	if( NULL == pclTAProduct )
	{
		return false;
	}

	bool bFound = false;
	CBatchSelectBaseList *pclBatchList = m_pclCurrentRowParameters->m_pclBatchResults;

	for( CSelectedBase *pclSelectedValve = pclBatchList->GetFirst<CSelectedBase>(); NULL != pclSelectedValve && false == bFound;
			pclSelectedValve = pclBatchList->GetNext<CSelectedBase>() )
	{
		CDB_ControlValve *pBalancingControlValve = dynamic_cast<CDB_ControlValve *>( pclSelectedValve->GetpData() );

		if( pBalancingControlValve == pclTAProduct )
		{
			m_pclCurrentRowParameters->m_pclSelectedProduct = pclSelectedValve;
			m_pclCurrentRowParameters->m_pclSelectedActuator = pRViewSSelBCv->GetCurrentActuatorSelected();
			// HYS-956: Display adapter
			m_pclCurrentRowParameters->m_pclSelectedAdapActuator = pRViewSSelBCv->GetFirstAdapterSelected();
			bFound = true;
		}
	}

	if( false == bFound )
	{
		return false;
	}

	// When user comes back from the right view (by selecting a product, actuator, accessories, ...) we have two possibilities. If user edits a
	// product for which there is only one solution (marks with the green arrow icon in the 'Status' column), we create a 'CDS_SSelProduct' in the
	// 'OnDlgOutputCellDblClicked' method. If user edits a product for which there is more than one solution (marks with yellow exclamation icon), we can't
	// create. In this case we have to do it here.
	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		m_pclCurrentRowParameters->m_pclCDSSelSelected = _BS_CreateSSelBCv( m_pclCurrentRowParameters );
	}

	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		return false;
	}

	pRViewSSelBCv->FillInSelected( m_pclCurrentRowParameters->m_pclCDSSelSelected );

	// Memorize current block selection if exist.
	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	// If previous status contained alternative, we reset this flag and keep only the user action.
	m_pclCurrentRowParameters->m_eStatus = CDlgBatchSelCtrlBase::BS_RS_FindOneSolution;

	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL != pclBSMessage )
	{
		pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_BATCHSELOK, true );
	}

	BS_PrepareOutput( pclBSMessage, m_pclCurrentRowParameters );

	// Send message.
	pclBSMessage->SendMessage( m_pclCurrentRowParameters->m_lRow, true, (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	// Restore the previous selections.
	m_clInterface.RestoreSelections( (int)m_clBatchSelBCVParams.m_eProductSubCategory );

	// HYS-1168 : We hide the DockablePane in edition mode, we have remove the auto hide mode when doing Apply
	CToolsDockablePane* pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}

	// Change the tools dockable pane to full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, true );

	pRViewSSelBCv->UnregisterNotificationHandler( this );

	m_pclCurrentRowParameters = NULL;
	ClearAll();

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	SetModificationMode( false );
	return true;
}

CDS_SSelBCv *CDlgBatchSelBCv::_BS_CreateSSelBCv( BSRowParameters *pclRowParameters )
{
	try
	{
		if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct || _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() 
				|| NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
		{
			return NULL;
		}

		CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );

		if( NULL == pclSelectedValve )
		{
			return NULL;
		}

		CDB_ControlValve *pclBalancingControlValve = ( CDB_ControlValve * )( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

		if( NULL == pclBalancingControlValve )
		{
			return NULL;
		}

		// Create selected object and initialize it.
		CDS_SSelBCv *pSelBCv = NULL;
		IDPTR IDPtr = _NULL_IDPTR;
		m_clBatchSelBCVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelBCv ) );

		pSelBCv = ( CDS_SSelBCv * )( IDPtr.MP );
		pSelBCv->SetProductIDPtr( pclBalancingControlValve->GetIDPtr() );
		pSelBCv->SetQ( pclRowParameters->m_dFlow );
		pSelBCv->SetOpening( pclSelectedValve->GetH() );

		if( CDS_SelProd::efdFlow == m_clBatchSelBCVParams.m_eFlowOrPowerDTMode )
		{
			pSelBCv->SetFlowDef( CDS_SelProd::efdFlow );
			pSelBCv->SetPower( 0.0 );
			pSelBCv->SetDT( 0.0 );
		}
		else
		{
			pSelBCv->SetFlowDef( CDS_SelProd::efdPower );
			pSelBCv->SetPower( pclRowParameters->m_dPower );
			pSelBCv->SetDT( pclRowParameters->m_dDT );
		}

		pSelBCv->SetSelectedAsAPackage(	m_clBatchSelBCVParams.m_bActuatorSelectedAsSet );

		pSelBCv->SetCtrlProp( m_clBatchSelBCVParams.m_eCV2W3W );
		pSelBCv->SetCtrlType( m_clBatchSelBCVParams.m_eCvCtrlType );
		pSelBCv->SetDp( pclSelectedValve->GetDp() );

		pSelBCv->SetTypeID( pclBalancingControlValve->GetTypeIDPtr().ID );
		pSelBCv->SetFamilyID( pclBalancingControlValve->GetFamilyIDPtr().ID );
		pSelBCv->SetMaterialID( pclBalancingControlValve->GetBodyMaterialIDPtr().ID );
		pSelBCv->SetConnectID( pclBalancingControlValve->GetConnectIDPtr().ID );
		pSelBCv->SetVersionID( pclBalancingControlValve->GetVersionIDPtr().ID );
		pSelBCv->SetPNID( pclBalancingControlValve->GetPNIDPtr().ID );

		pSelBCv->SetActrIDPtr( ( NULL != pclRowParameters->m_pclSelectedActuator ) ? pclRowParameters->m_pclSelectedActuator->GetIDPtr() : _NULL_IDPTR );
		
		// HYS-956: Display adapter.
		if( ( NULL != pSelBCv->GetCvAccessoryList() ) && ( NULL != pclRowParameters->m_pclSelectedAdapActuator ) )
		{
			pSelBCv->GetCvAccessoryList()->Add( pclRowParameters->m_pclSelectedAdapActuator->GetIDPtr(), CAccessoryList::_AT_Adapter );
		}
		
		// Set the actuators info.
		pSelBCv->SetActuatorPowerSupplyID( m_clBatchSelBCVParams.m_strActuatorPowerSupplyID );
		pSelBCv->SetActuatorInputSignalID( m_clBatchSelBCVParams.m_strActuatorInputSignalID );
		pSelBCv->SetFailSafeFunction( m_clBatchSelBCVParams.m_iActuatorFailSafeFunction );
		pSelBCv->SetDRPFunction( m_clBatchSelBCVParams.m_eActuatorDRPFunction );

		pSelBCv->SetPipeSeriesID( m_clBatchSelBCVParams.m_strPipeSeriesID );
		pSelBCv->SetPipeID( m_clBatchSelBCVParams.m_strPipeID );

		// Selected pipe informations.
		pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBalancingControlValve->GetSizeKey(), *pSelBCv->GetpSelPipe() );

		*pSelBCv->GetpSelectedInfos()->GetpWCData() = m_clBatchSelBCVParams.m_WC;
		pSelBCv->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef1, pclRowParameters->m_strReference1 );
		pSelBCv->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef2, pclRowParameters->m_strReference2 );
		pSelBCv->GetpSelectedInfos()->SetQuantity( 1 );

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelBCv->GetpSelectedInfos()->SetDT( m_clBatchSelBCVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelBCv->GetpSelectedInfos()->SetApplicationType( m_clBatchSelBCVParams.m_eApplicationType );

		CTable *pTab = m_clBatchSelBCVParams.m_pTADS->GetpClipboardTable();
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CLIPBOARD_TABLE' from the datastruct.") );
		}

		if( _T('\0') == *IDPtr.ID )
		{
			pTab->Insert( IDPtr );
		}

		return pSelBCv;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelBCv::_BS_CreateSSelBCv'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelBCv::_BS_DisplaySolutions()
{
	if( NULL != pRViewSSelBCv )
	{
		m_bRViewEmpty = false;
		SetModificationMode( true );

		// Remark: The 'SetModificationMode' method just above set the 'm_pclBatchSelParams->m_bEditModeRunning' to true.
		//         Thus we need to manually set it in 'm_clIndSelBCVParams' for the right view to pass in edition mode.
		m_clIndSelBCVParams.m_bEditModeRunning = true;

		pRViewSSelBCv->Suggest( &m_clIndSelBCVParams );
	}

	return;
}

void CDlgBatchSelBCv::_BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct
			|| _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() || iColumnID < BS_BCVColumnID::BS_BCVCID_First
			|| iColumnID >= BS_BCVColumnID::BS_BCVCID_Last )
	{
		return;
	}

	CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );

	if( NULL == pclSelectedValve )
	{
		return;
	}

	CDB_ControlValve *pclBalancingControlValve = dynamic_cast<CDB_ControlValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

	if( NULL == pclBalancingControlValve )
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
	double dRho = m_clBatchSelBCVParams.m_WC.GetDens();
	double dKinVisc = m_clBatchSelBCVParams.m_WC.GetKinVisc();

	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclRowParameters->m_pclSelectedActuator );
	CDB_ValveCharacteristic *pclValveCharacteristic = ( CDB_ValveCharacteristic * )pclBalancingControlValve->GetValveCharDataPointer();
	CDB_CloseOffChar *pclCloseOffChar = ( CDB_CloseOffChar * )( pclBalancingControlValve->GetCloseOffCharIDPtr().MP );
	CDB_Product *pclAdaptActuator = dynamic_cast<CDB_Product *>( pclRowParameters->m_pclSelectedAdapActuator );

	CString str;

	switch( iColumnID )
	{
		case BS_BCVColumnID::BS_BCVCID_Name:
			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_Name, pclBalancingControlValve->GetName(), true );
			break;

		case BS_BCVColumnID::BS_BCVCID_Material:
			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_Material, pclBalancingControlValve->GetBodyMaterial(), true );
			break;

		case BS_BCVColumnID::BS_BCVCID_Connection:
			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_Connection, pclBalancingControlValve->GetConnect(), true );
			break;

		case BS_BCVColumnID::BS_BCVCID_Version:
			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_Version, pclBalancingControlValve->GetVersion(), true );
			break;

		case BS_BCVColumnID::BS_BCVCID_PN:
			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_PN, pclBalancingControlValve->GetPN().c_str(), true );
			break;
		
		case BS_BCVColumnID::BS_BCVCID_Size:
			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_Size, pclBalancingControlValve->GetSize(), true );
			break;

		case BS_BCVColumnID::BS_BCVCID_Rangeability:
			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_Rangeability, pclBalancingControlValve->GetStrRangeability().c_str(), true );
			break;

		case BS_BCVColumnID::BS_BCVCID_LeakageRate:
			pclBSMessage->SetCellNumber( BS_BCVColumnID::BS_BCVCID_LeakageRate, pclBalancingControlValve->GetLeakageRate() * 100, iPhysicalType, iMaxDigit, iMinDecimal,
										 true );
			break;

		case BS_BCVColumnID::BS_BCVCID_Stroke:
		{
			if( NULL != pclCloseOffChar )
			{
				if( CDB_CloseOffChar::eOpenType::Linear == pclCloseOffChar->GetOpenType() )
				{
					str = WriteCUDouble( iPhysicalType, pclBalancingControlValve->GetStroke(), false, iMaxDigit, iMinDecimal );
					str += CString( _T( "" ) ) + GetNameOf( pUnitDB->GetDefaultUnit( iPhysicalType ) ).c_str();
					pclBSMessage->SetCellNumberAsText( BS_BCVColumnID::BS_BCVCID_Stroke, str, iPhysicalType, iMaxDigit, iMinDecimal, true );
				}
				else
				{
					str = WriteDouble( pclBalancingControlValve->GetStroke(), 0, 0 );
					str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_ANGULARDEGRE );
					pclBSMessage->SetCellNumberAsText( BS_BCVColumnID::BS_BCVCID_Stroke, str, _U_NODIM, 0, 0, true );
				}
			}
			else
			{
				str = WriteCUDouble( iPhysicalType, pclBalancingControlValve->GetStroke(), false, iMaxDigit, iMinDecimal );
				str += CString( _T( "" ) ) + GetNameOf( pUnitDB->GetDefaultUnit( iPhysicalType ) ).c_str();
				pclBSMessage->SetCellNumberAsText( BS_BCVColumnID::BS_BCVCID_Stroke, str, iPhysicalType, iMaxDigit, iMinDecimal, true );
			}
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_ImgCharacteristic:
			if( NULL != pclBalancingControlValve->GetCtrlProp() )
			{
				CDB_ControlProperties::eCTRLCHAR CtrlChar = pclBalancingControlValve->GetCtrlProp()->GetCvCtrlChar();

				if( CDB_ControlProperties::Linear == CtrlChar )
				{
					pclBSMessage->SetCellBitmap( BS_BCVColumnID::BS_BCVCID_ImgCharacteristic, IDI_CHARACTLIN, true );
				}
				else if( CDB_ControlProperties::EqualPc == CtrlChar )
				{
					pclBSMessage->SetCellBitmap( BS_BCVColumnID::BS_BCVCID_ImgCharacteristic, IDI_CHARACTEQM, true );
				}
				else
				{
					pclBSMessage->SetCellBitmap( BS_BCVColumnID::BS_BCVCID_ImgCharacteristic, IDI_CHARACTNONE, true );
				}
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ImgPushClose:
			if( NULL != pclBalancingControlValve->GetCtrlProp() )
			{
				CDB_ControlProperties::ePushOrPullToClose PushClose = pclBalancingControlValve->GetCtrlProp()->GetCvPushClose();

				if( CDB_ControlProperties::PushToClose == PushClose )
				{
					pclBSMessage->SetCellBitmap( BS_BCVColumnID::BS_BCVCID_ImgPushClose, IDI_PUSHCLOSE, true );
				}
				else if( CDB_ControlProperties::PullToClose == PushClose )
				{
					pclBSMessage->SetCellBitmap( BS_BCVColumnID::BS_BCVCID_ImgPushClose, IDI_PUSHOPEN, true );
				}
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_Preset:
		{
			double dPresetting = pclSelectedValve->GetH();
			str = _T("-");

			if( NULL != pclValveCharacteristic && -1.0 != dPresetting )
			{
				str = pclValveCharacteristic->GetSettingString( dPresetting );
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

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumberAsText( BS_BCVColumnID::BS_BCVCID_Preset, str, iPhysicalType, iMaxDigit, iMinDecimal,
					true );

			if( NULL != pclCellBase && true == bOrange )
			{
				pclCellBase->SetTextForegroundColor( _ORANGE );
			}
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_Dp:
		{
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BCVColumnID::BS_BCVCID_Dp, pclSelectedValve->GetDp(), iPhysicalType, iMaxDigit,
					iMinDecimal, true );
			
			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBCVParams.m_pTADS->GetpTechParams();

			if( NULL != pclCellBase && NULL != pclTechnicalParameter )
			{
				double dValveMinDp = ( CDB_ControlProperties::eCvOnOff == m_clBatchSelBCVParams.m_eCvCtrlType ) ? pclTechnicalParameter->GetCVMinDpOnoff() : pclTechnicalParameter->GetCVMinDpProp();

				if( pclSelectedValve->GetDp() < dValveMinDp || pclSelectedValve->GetDp() > pclTechnicalParameter->VerifyValvMaxDp( pclBalancingControlValve ) )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );
				}
			}
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_DpFullOpening:
		{
			if( NULL != pclValveCharacteristic )
			{
				double dHMax = pclValveCharacteristic->GetOpeningMax();

				if( dHMax > 0.0 )
				{
					double dDpFullyOpen;

					if( true == pclValveCharacteristic->GetValveDp( pclRowParameters->m_dFlow, &dDpFullyOpen, dHMax, dRho, dKinVisc ) )
					{
						DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BCVColumnID::BS_BCVCID_DpFullOpening, dDpFullyOpen, iPhysicalType, iMaxDigit,
								iMinDecimal, true );

						CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBCVParams.m_pTADS->GetpTechParams();

						if( NULL != pclCellBase && NULL != pclTechnicalParameter )
						{
							double dValveMinDp = ( CDB_ControlProperties::eCvOnOff == m_clBatchSelBCVParams.m_eCvCtrlType ) ? pclTechnicalParameter->GetCVMinDpOnoff() : pclTechnicalParameter->GetCVMinDpProp();

							if( dDpFullyOpen < dValveMinDp )
							{
								pclCellBase->SetTextForegroundColor( _ORANGE );
							}
						}
					}
				}
			}
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_DpHalfOpening:
		{
			if( NULL != pclValveCharacteristic )
			{
				double dHMax = pclValveCharacteristic->GetOpeningMax();

				if( dHMax > 0.0 )
				{
					double dDpHalpOpen;

					if( true == pclValveCharacteristic->GetValveDp( pclRowParameters->m_dFlow, &dDpHalpOpen, dHMax / 2, dRho, dKinVisc ) )
					{
						DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BCVColumnID::BS_BCVCID_DpHalfOpening, dDpHalpOpen, iPhysicalType, iMaxDigit,
								iMinDecimal, true );
						CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBCVParams.m_pTADS->GetpTechParams();

						if( NULL != pclCellBase && NULL != pclTechnicalParameter )
						{
							double dValveMinDp = ( CDB_ControlProperties::eCvOnOff == m_clBatchSelBCVParams.m_eCvCtrlType ) ? pclTechnicalParameter->GetCVMinDpOnoff() : pclTechnicalParameter->GetCVMinDpProp();

							if( ( dDpHalpOpen > pclTechnicalParameter->VerifyValvMaxDp( pclBalancingControlValve ) || dDpHalpOpen < dValveMinDp ) )
							{
								pclCellBase->SetTextForegroundColor( _ORANGE );
							}
						}
					}
				}
			}
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_TemperatureRange:
		{
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_TemperatureRange, pclBalancingControlValve->GetTempRange(),
					true );

			if( NULL != pclCellBase )
			{
				if( m_clBatchSelBCVParams.m_WC.GetTemp() < pclBalancingControlValve->GetTmin() || m_clBatchSelBCVParams.m_WC.GetTemp() > pclBalancingControlValve->GetTmax() )
				{
					pclCellBase->SetTextForegroundColor( _RED );
				}
			}
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_PipeSize:
		{
			CString str = _T( "" );
			CSelectPipe selPipe( &m_clBatchSelBCVParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBalancingControlValve->GetSizeKey(), selPipe );
			}

			pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_PipeSize, selPipe.GetpPipe()->GetName(), true );
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_PipeLinDp:
		{
			CSelectPipe selPipe( &m_clBatchSelBCVParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBalancingControlValve->GetSizeKey(), selPipe );
			}
			
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BCVColumnID::BS_BCVCID_PipeLinDp, selPipe.GetLinDp(), iPhysicalType, iMaxDigit,
					iMinDecimal, true );
			
			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBCVParams.m_pTADS->GetpTechParams();

			if( NULL != pclCellBase && NULL != pclTechnicalParameter )
			{
				if( selPipe.GetLinDp() > pclTechnicalParameter->GetPipeMaxDp() || selPipe.GetLinDp() < pclTechnicalParameter->GetPipeMinDp() )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );
				}
			}
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_PipeV:
		{
			CSelectPipe selPipe( &m_clBatchSelBCVParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBalancingControlValve->GetSizeKey(), selPipe );
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BCVColumnID::BS_BCVCID_PipeV, selPipe.GetU(), iPhysicalType, iMaxDigit, iMinDecimal,
					true );
			
			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBCVParams.m_pTADS->GetpTechParams();

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

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelBCVParams.m_eProductSubCategory );
				}
			}
		}
		break;

		case BS_BCVColumnID::BS_BCVCID_ActrName:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrName, pclElectroActuator->GetName(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrName, _T("-"), true );
			}
			break;

		case BS_BCVColumnID::BS_BCVCID_ActrCloseOffDp:
			if( NULL != pclElectroActuator )
			{
				double dCloseOffDp = -1;

				if( NULL != pclCloseOffChar && CDB_CloseOffChar::CloseOffDp == pclCloseOffChar->GetLimitType() )
				{
					dCloseOffDp = pclCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );
				}

				bool fRed = false;

				if( -1.0 == dCloseOffDp )
				{
					pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrCloseOffDp, _T("-"), true );
				}
				else
				{
					DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BCVColumnID::BS_BCVCID_ActrCloseOffDp, dCloseOffDp, iPhysicalType, iMaxDigit,
							iMinDecimal, true );

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
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrCloseOffDp, _T("-"), true );
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ActrMaxInletPressure:
			if( NULL != pclElectroActuator )
			{
				double dMaxInletPressure = -1.0;

				if( NULL != pclCloseOffChar && CDB_CloseOffChar::InletPressure == pclCloseOffChar->GetLimitType() )
				{
					dMaxInletPressure = pclCloseOffChar->GetMaxInletPressure( pclElectroActuator->GetMaxForceTorque() );
				}

				if( -1.0 == dMaxInletPressure )
				{
					pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrMaxInletPressure, _T("-"), true );
				}
				else
				{
					pclBSMessage->SetCellNumber( BS_BCVColumnID::BS_BCVCID_ActrMaxInletPressure, dMaxInletPressure, iPhysicalType, iMaxDigit,
							iMinDecimal, true );
				}
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrMaxInletPressure, _T("-"), true );
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ActrActuatingTime:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrActuatingTime, pclElectroActuator->GetActuatingTimesStr( pclBalancingControlValve->GetStroke() ),
						true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrActuatingTime, _T("-"), true );
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ActrIP:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrIP, pclElectroActuator->GetIPxxFull(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrIP, _T("-"), true );
			}
			break;

		case BS_BCVColumnID::BS_BCVCID_ActrPowSupply:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrPowSupply, pclElectroActuator->GetPowerSupplyStr(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrPowSupply, _T("-"), true );
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ActrInputSig:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrInputSig, pclElectroActuator->GetInOutSignalsStr( true ), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrInputSig, _T("-"), true );
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ActrOutputSig:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrOutputSig, pclElectroActuator->GetInOutSignalsStr( false ), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrOutputSig, _T("-"), true );
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ActrFailSafe:
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
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrFailSafe, str, true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrFailSafe, _T("-"), true );
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ActrDRP:
			if( NULL != pclElectroActuator )
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrDRP, pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrDRP, _T("-"), true );
			}

			break;

		case BS_BCVColumnID::BS_BCVCID_ActrAdapterName:
			if( ( NULL != pclAdaptActuator ) && ( pclAdaptActuator->IsAnAccessory() ) )
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrAdapterName, pclAdaptActuator->GetName(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_BCVColumnID::BS_BCVCID_ActrAdapterName, _T("-"), true );
			}

			break;
	}
}

bool CDlgBatchSelBCv::_CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters )
{
	// Here, we can have either a best result that is saved in the 'pclRowParameters->m_pclSelectedProduct' variable or we can have more that one result.
	// If we have a best result, we save it in a 'CDS_SSelBCv' object and we call the 'RViewSSelBCV::Suggest' method in edit mode. 
	// Otherwise, we call the 'RViewSSelBCV::Suggest' method in normal mode to display all results and let the user to choose one.

	if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()
			|| NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe() )
	{
		ASSERTA_RETURN( false );
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( m_clBatchSelBCVParams.m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( false );
	}

	int iDN65 = pclTableDN->GetSize( _T("DN_65") );
	
	// This method will just copy the variables that are in the 'CProductSelectionParameters' base class.
	m_clIndSelBCVParams.CopyFrom( &m_clBatchSelBCVParams );

	// We need to do it manually for all variables of 'm_clBatchSelBCVParams' that are needed by the 'RViewSSelBCV'.

	// All variables in for 'CIndividualSelectionParameters'.
	m_clIndSelBCVParams.m_dFlow = pclRowParameters->m_dFlow;
	m_clIndSelBCVParams.m_dDp = pclRowParameters->m_dDp;

	CDB_Pipe *pclBestPipe =  pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe();
	int iSize = pclBestPipe->GetSizeKey( m_clBatchSelBCVParams.m_pTADB );

	if( iSize < iDN65 )
	{
		m_clIndSelBCVParams.m_strComboTypeID = m_clBatchSelBCVParams.m_strComboTypeBelow65ID;
		m_clIndSelBCVParams.m_strComboFamilyID = m_clBatchSelBCVParams.m_strComboFamilyBelow65ID;
		m_clIndSelBCVParams.m_strComboMaterialID = m_clBatchSelBCVParams.m_strComboMaterialBelow65ID;
		m_clIndSelBCVParams.m_strComboConnectID = m_clBatchSelBCVParams.m_strComboConnectBelow65ID;
		m_clIndSelBCVParams.m_strComboVersionID = m_clBatchSelBCVParams.m_strComboVersionBelow65ID;
		m_clIndSelBCVParams.m_strComboPNID = m_clBatchSelBCVParams.m_strComboPNBelow65ID;
	}
	else
	{
		m_clIndSelBCVParams.m_strComboTypeID = m_clBatchSelBCVParams.m_strComboTypeAbove50ID;
		m_clIndSelBCVParams.m_strComboFamilyID = m_clBatchSelBCVParams.m_strComboFamilyAbove50ID;
		m_clIndSelBCVParams.m_strComboMaterialID = m_clBatchSelBCVParams.m_strComboMaterialAbove50ID;
		m_clIndSelBCVParams.m_strComboConnectID = m_clBatchSelBCVParams.m_strComboConnectAbove50ID;
		m_clIndSelBCVParams.m_strComboVersionID = m_clBatchSelBCVParams.m_strComboVersionAbove50ID;
		m_clIndSelBCVParams.m_strComboPNID = m_clBatchSelBCVParams.m_strComboPNAbove50ID;
	}

	m_clIndSelBCVParams.m_eFilterSelection = m_clBatchSelBCVParams.m_eFilterSelection;
	
	// All variables in for 'CIndSelCtrlParamsBase'.
	m_clIndSelBCVParams.m_eCV2W3W = m_clBatchSelBCVParams.m_eCV2W3W;
	m_clIndSelBCVParams.m_eCvCtrlType = m_clBatchSelBCVParams.m_eCvCtrlType;
	m_clIndSelBCVParams.m_strActuatorPowerSupplyID = m_clBatchSelBCVParams.m_strActuatorPowerSupplyID;
	m_clIndSelBCVParams.m_strActuatorInputSignalID = m_clBatchSelBCVParams.m_strActuatorInputSignalID;
	m_clIndSelBCVParams.m_iActuatorFailSafeFunction = m_clBatchSelBCVParams.m_iActuatorFailSafeFunction;
	m_clIndSelBCVParams.m_eActuatorDRPFunction = m_clBatchSelBCVParams.m_eActuatorDRPFunction;

	// Create now the 'CSelectBCVList' that 'RViewSSelBCV' needs.
	if( NULL != m_clIndSelBCVParams.m_pclSelectBCVList )
	{
		delete m_clIndSelBCVParams.m_pclSelectBCVList;
		m_clIndSelBCVParams.m_pclSelectBCVList = NULL;
	}

	m_clIndSelBCVParams.m_pclSelectBCVList = new CSelectBCVList();

	if( NULL == m_clIndSelBCVParams.m_pclSelectBCVList )
	{
		ASSERTA_RETURN( false );
	}
	
	// We now transfer results from the batch selection list to the individual selection list.
	pclRowParameters->m_pclBatchResults->TransferResultsToIndividualSelectList( m_clIndSelBCVParams.m_pclSelectBCVList );

	// Create an 'CDS_SelDpC' to allow 'RViewSSelDpC' to be in edit mode with this object selected.
	// Remark: Only if we have only one result.
	if( NULL != pclRowParameters->m_pclSelectedProduct )
	{
		CDS_SSelBCv *pclSSelBCv = NULL;

		if( NULL == pclRowParameters->m_pclCDSSelSelected )
		{
			pclSSelBCv = _BS_CreateSSelBCv( pclRowParameters );
		}
		else
		{
			pclSSelBCv = dynamic_cast<CDS_SSelBCv *>( pclRowParameters->m_pclCDSSelSelected );
		}

		if( NULL == pclSSelBCv )
		{
			return false;
		}

		pclRowParameters->m_pclCDSSelSelected = pclSSelBCv;
		m_clIndSelBCVParams.m_SelIDPtr = pclSSelBCv->GetIDPtr();
	}

	return true;
}
