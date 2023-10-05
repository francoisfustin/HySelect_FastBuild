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
#include "RViewSSelSmartDpC.h"
#include "DlgBatchSelSmartDpC.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelSmartDpC::CDlgBatchSelSmartDpC( CWnd *pParent )
	: CDlgBatchSelBase( m_clBatchSelSmartDpCParams, CDlgBatchSelSmartDpC::IDD, pParent )
{
	m_clBatchSelSmartDpCParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_SmartDpC;
	m_clBatchSelSmartDpCParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	// For the moment, the are only the possibility to select by set.
	m_clBatchSelSmartDpCParams.m_bIsSelectedAsSet = true;
}

CDlgBatchSelSmartDpC::~CDlgBatchSelSmartDpC()
{
}

void CDlgBatchSelSmartDpC::SaveSelectionParameters()
{
	CDlgBatchSelBase::SaveSelectionParameters();

	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelSmartDpCParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return;
	}

	pclBatchSelParameter->SetSmartDpCRadioFlowState( (int)m_clBatchSelSmartDpCParams.m_eFlowOrPowerDTMode );
	pclBatchSelParameter->SetSmartDpCDpBranchCheckboxState( (int)m_clBatchSelSmartDpCParams.m_bIsCheckboxDpBranchChecked );
	pclBatchSelParameter->SetSmartDpCDpMaxCheckboxState( (int)m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked );
	pclBatchSelParameter->SetSmartDpCMaterialBelow65ID( m_clBatchSelSmartDpCParams.m_strComboMaterialBelow65ID );
	pclBatchSelParameter->SetSmartDpCConnectBelow65ID( m_clBatchSelSmartDpCParams.m_strComboConnectBelow65ID );
	pclBatchSelParameter->SetSmartDpCPNBelow65ID( m_clBatchSelSmartDpCParams.m_strComboPNBelow65ID );
	pclBatchSelParameter->SetSmartDpCMaterialAbove50ID( m_clBatchSelSmartDpCParams.m_strComboMaterialAbove50ID );
	pclBatchSelParameter->SetSmartDpCConnectAbove50ID( m_clBatchSelSmartDpCParams.m_strComboConnectAbove50ID );
	pclBatchSelParameter->SetSmartDpCPNAbove50ID( m_clBatchSelSmartDpCParams.m_strComboPNAbove50ID );

	BS_WriteAllColumnWidth( pclBatchSelParameter );
}

bool CDlgBatchSelSmartDpC::ResetRightView()
{
	if( NULL == pRViewSSelSmartDpC )
	{
		return false;
	}

	if( true == pRViewSSelSmartDpC->IsEmpty() )
	{
		return true;
	}

	pRViewSSelSmartDpC->Reset();
	return true;
}

void CDlgBatchSelSmartDpC::ActivateLeftTabDialog()
{
	CDlgBatchSelBase::ActivateLeftTabDialog();

	CDS_TechnicalParameter *pTechP = m_clBatchSelSmartDpCParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clBatchSelSmartDpCParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

bool CDlgBatchSelSmartDpC::OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList )
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

	for( int iLoopColumn = BS_SMARTDPCCID_First; iLoopColumn < BS_SMARTDPCCID_Last; iLoopColumn++ )
	{
		CDlgBatchSelectionOutput::ColData rColData;

		if( BS_SMARTDPCCID_Name == iLoopColumn || BS_SMARTDPCCID_DpSensorName == iLoopColumn )
		{
			// Add the name and a separator.
			rColData.m_lColumnID = -1;

			rColData.m_strName = TASApp.LoadLocalizedString( ( BS_SMARTDPCCID_Name == iLoopColumn ) ? IDS_DLGBATCHSELSMARTDPC_CTXTMENUSMARTOUTPUT :
					IDS_DLGBATCHSELSMARTDPC_CTXTMENUDPSENSOROUTPUT );
			
			rColData.m_bAlreadyDisplayed = false;
			rColData.m_bEnable = false;
			rColData.m_bSeparator = false;
			mapColumnList[iInsert++] = rColData;

			rColData.m_lColumnID = -1;
			rColData.m_strName = _T("");
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
		rColData.m_strName = m_mapColumnList[iLoopColumn].m_strHeaderName;
		rColData.m_bAlreadyDisplayed = m_mapColumnList[iLoopColumn].m_bDisplayed;

		// Show 'BS_SMARTDPCCID_DpMax' item in context menu ONLY if user has checked the 'Dp max' check box.
		if( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax == iLoopColumn )
		{
			rColData.m_bEnable = m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax].m_bEnabled;
		}

		// Show 'BS_SMARTDPCCID_DpSensorDpl' item in context menu ONLY if user has checked the 'Dp branch' check box.
		if( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl == iLoopColumn )
		{
			rColData.m_bEnable = m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl].m_bEnabled;
		}

		rColData.m_bSeparator = false;
		mapColumnList[iInsert++] = rColData;
	}

	return true;
}

bool CDlgBatchSelSmartDpC::OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct )
{
	if( false == CDlgBatchSelBase::OnBatchSelectionIsCatalogExist( iColumnID, lRowRelative, bCatExist, pclProduct ) )
	{
		return false;
	}

	if( iColumnID >= BS_SmartDpCColumnID::BS_SMARTDPCCID_Name && iColumnID < BS_SmartDpCColumnID::BS_SMARTDPCCID_PipeSize
			&& NULL != m_mapAllRowData[lRowRelative].m_pclSelectedProduct )
	{
		// Check smart differential pressure controller.
		CDB_TAProduct *pclTAProduct = (CDB_TAProduct *)( m_mapAllRowData[lRowRelative].m_pclSelectedProduct->GetProductIDPtr().MP );
		bCatExist = ( true == pclTAProduct->VerifyOneCatExist() );
		pclProduct = pclTAProduct;
	}

	return true;
}

bool CDlgBatchSelSmartDpC::OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelSmartDpC )
	{
		ASSERTA_RETURN( false );
	}

	if( false == CDlgBatchSelBase::OnDlgOutputCellDblClicked( iOutputID, iColumnID, lRowRelative ) )
	{
		return false;
	}

	if( BS_ValidationStatus::BS_VS_Done == m_eValidationStatus )
	{
		return false;
	}

	// If no data available on this row (can be the case when user double click on the last blank line).
	if( false == m_clInterface.IsRowValidForData( lRowRelative, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory ) )
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
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL != pclToolsDockablePane && TRUE == pclToolsDockablePane->IsWindowVisible() )
	{
		pclToolsDockablePane->SetAutoHideMode( TRUE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}
	
	// Change the tools dockable pane to no full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( false, true );

	pRViewSSelSmartDpC->Invalidate();
	pRViewSSelSmartDpC->UpdateWindow();
	pRViewSSelSmartDpC->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );

	m_clInterface.SelectRow( lRowRelative, true, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	_BS_DisplaySolutions();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelSmartDpC, CDlgBatchSelBase )
	ON_COMMAND( IDC_CHECKDPBRANCH, OnBnClickedCheckDpBranch )
	ON_COMMAND( IDC_CHECKDPMAX, OnBnClickedCheckDpMax )
	ON_EN_KILLFOCUS( IDC_EDITDPMAX, OnEnKillFocusDpMax )
END_MESSAGE_MAP()

void CDlgBatchSelSmartDpC::DoDataExchange( CDataExchange *pDX )
{
	CDlgBatchSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKDPBRANCH, m_clCheckboxDpBranch );
	DDX_Control( pDX, IDC_CHECKDPMAX, m_clCheckboxDpMax );
	DDX_Control( pDX, IDC_EDITDPMAX, m_clExtEditDpMax );
}

BOOL CDlgBatchSelSmartDpC::OnInitDialog()
{
	CDlgBatchSelBase::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_BATCHSELSMARTDPC_DPBRANCH );
	m_clCheckboxDpBranch.SetWindowText( str );
	m_clCheckboxDpBranch.SetCheck( BST_UNCHECKED );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELSMARTDPC_DPMAX );
	m_clCheckboxDpMax.SetWindowText( str );
	m_clCheckboxDpMax.SetCheck( BST_UNCHECKED );

	m_clExtEditDpMax.SetPhysicalType( _U_DIFFPRESS );
	m_clExtEditDpMax.SetEditType( CNumString::eDouble, CNumString::ePositive );

	return TRUE;
}

void CDlgBatchSelSmartDpC::OnBnClickedCheckDpBranch()
{
	_UpdateDpBranchCheckboxState();
	_UpdateDpBranchColumn();
}

void CDlgBatchSelSmartDpC::OnBnClickedCheckDpMax()
{
	_UpdateDpMaxFieldState();
	_UpdateDpMaxColumn();
}

void CDlgBatchSelSmartDpC::OnEnChangeDpMax()
{
	if( GetFocus() == &m_clExtEditDpMax )
	{
		ClearAll();
	}
}

void CDlgBatchSelSmartDpC::OnEnKillFocusDpMax()
{
	m_clBatchSelSmartDpCParams.m_dDpMax = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpMax, &m_clBatchSelSmartDpCParams.m_dDpMax ) 
			|| m_clBatchSelSmartDpCParams.m_dDpMax < 0.0 )
	{
		m_clBatchSelSmartDpCParams.m_dDpMax = 0.0;
	}
}

void CDlgBatchSelSmartDpC::OnBnClickedSuggest()
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

void CDlgBatchSelSmartDpC::OnBnClickedValidate()
{
	CDlgBatchSelBase::OnBnClickedValidate();

	if( true == m_clBatchSelSmartDpCParams.m_bEditModeRunning )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
		}

		_BS_ApplySelection();

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
		}
	}
}

void CDlgBatchSelSmartDpC::OnBnClickedCancel()
{
	// User has edited a valid selection to change it but finally cancel it.
	OnRViewSSelKeyboardEvent( VK_ESCAPE );
	ResetRightView();
}

LRESULT CDlgBatchSelSmartDpC::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelSmartDpCParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return -1;
	}

	// Fill the combo box.
	// Remark: Corresponding combo variables in 'm_clBatchSelSmartDpCParams' are updated in each of this following methods.
	FillComboBoxBodyMaterial( AboveOrBelow::Below65, pclBatchSelParameter->GetSmartDpCMaterialBelow65ID() );
	FillComboBoxConnect( AboveOrBelow::Below65, pclBatchSelParameter->GetSmartDpCConnectBelow65ID() );
	FillComboBoxPN( AboveOrBelow::Below65, pclBatchSelParameter->GetSmartDpCPNBelow65ID() );

	FillComboBoxBodyMaterial( AboveOrBelow::Above50, pclBatchSelParameter->GetSmartDpCMaterialAbove50ID() );
	FillComboBoxConnect( AboveOrBelow::Above50, pclBatchSelParameter->GetSmartDpCConnectAbove50ID() );
	FillComboBoxPN( AboveOrBelow::Above50, pclBatchSelParameter->GetSmartDpCPNAbove50ID() );

	// Set the 'Flow' radio state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclBatchSelParameter->GetSmartDpCRadioFlowState() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clBatchSelSmartDpCParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgBatchSelBase::UpdateFlowOrPowerDTState();

	m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax].m_bEnabled = m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked;
	m_clBatchSelSmartDpCParams.m_bIsCheckboxDpBranchChecked = ( 0 == pclBatchSelParameter->GetDpCDpBranchCheckboxState() ) ? BST_UNCHECKED : BST_CHECKED;
	m_clCheckboxDpBranch.SetCheck( ( 0 == pclBatchSelParameter->GetDpCDpBranchCheckboxState() ) ? BST_UNCHECKED : BST_CHECKED );
	_UpdateDpBranchCheckboxState();

	// Set the Dp max check.
	// Remark: 'm_clBatchSelSmartDpCParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_clCheckboxDpMax.SetCheck( ( 0 == pclBatchSelParameter->GetDpCDpMaxCheckboxState() ) ? BST_UNCHECKED : BST_CHECKED );
	_UpdateDpMaxFieldState();

	OnUnitChange();

	// This is needed only in the case of the dialog is currently active when user creates a new document (or opens a new one).
	OnAfterNewDocument();

	m_bInitialised = true;
	return 0;
}

LRESULT CDlgBatchSelSmartDpC::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelBase::OnUnitChange( wParam, lParam );

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATICDPMAXUNIT, name );

	if( m_clBatchSelSmartDpCParams.m_dDpMax > 0.0 )
	{
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_clBatchSelSmartDpCParams.m_dDpMax ) );
	}

	return 0;
}

void CDlgBatchSelSmartDpC::EnableSuggestButton( bool bEnable )
{
	// Disable the 'Validate' button.
	if( false == m_clBatchSelSmartDpCParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	CDlgSelectionBase::ClearAll();
}

void CDlgBatchSelSmartDpC::BS_PrepareColumnList( void )
{
	CDlgBatchSelBase::BS_PrepareColumnList();

	BSColParameters rColParameters;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Prepare header and data parameters.
	rColParameters.m_clHeaderColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_SmartDpCMainHeaderID::BS_SMARTDPCMHID_SmartDpCDpSensor, 
			BS_SmartDpCMainHeaderSubID::BS_SMARTDPCMHSUBID_SmartDpC, DlgOutputHelper::CColDef::AutoResizeDisabled, 
			DlgOutputHelper::CColDef::Visible, DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionDisabled, 
			DlgOutputHelper::CColDef::BlockSelectionDisabled, DlgOutputHelper::CColDef::RowSelectionDisabled, 
			DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );
	
	rColParameters.m_clHeaderColDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );

	rColParameters.m_clHeaderFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Center, 
			DlgOutputHelper::CFontDef::VA_Center, DlgOutputHelper::CFontDef::BoldDisabled, DlgOutputHelper::CFontDef::ItalicDisabled, 
			DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_clDataColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_SmartDpCMainHeaderID::BS_SMARTDPCMHID_SmartDpCDpSensor, 
			BS_SmartDpCMainHeaderSubID::BS_SMARTDPCMHSUBID_SmartDpC, DlgOutputHelper::CColDef::AutoResizeDisabled, DlgOutputHelper::CColDef::Visible, 
			DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionEnabled, DlgOutputHelper::CColDef::BlockSelectionEnabled, 
			DlgOutputHelper::CColDef::RowSelectionEnabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataEnabled );

	rColParameters.m_clDataColDef.SetRowSeparatorFlag( true, DLGBATCHSELBASE_COLOR_DATAROWSEPARATOR );

	rColParameters.m_clDataFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Left, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled, DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_iColumnID = BS_SMARTDPCCID_Name;
	rColParameters.m_clHeaderColDef.SetWidth( 20.0 );
	rColParameters.m_clDataColDef.SetWidth( 20.0 );
	rColParameters.m_bCantRemove = true;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_bLineBelowHeader = true;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_Material;
	rColParameters.m_clHeaderColDef.SetWidth( 12.0 );
	rColParameters.m_clDataColDef.SetWidth( 12.0 );
	rColParameters.m_bCantRemove = false;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_Connection;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_PN;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_Size;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHVALVESIZE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_Kvs;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_C_KVCVCOEFF );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_KVS );
	rColParameters.m_iHeaderUnit = _C_KVCVCOEFF;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_Qnom;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_FLOW );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_QNOM );
	rColParameters.m_iHeaderUnit = _U_FLOW;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_DpMin;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPMIN );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_DpMax;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELSMARTDPC_DPMAX );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_TemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_PipeSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPESIZE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_PipeLinDp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_LINPRESSDROP );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPELINDP );
	rColParameters.m_iHeaderUnit = _U_LINPRESSDROP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_PipeV;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_VELOCITY );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPEV );
	rColParameters.m_iHeaderUnit = _U_VELOCITY;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	// Columns linked to the Dp sensor.
	rColParameters.m_iColumnID = BS_SMARTDPCCID_DpSensorName;
	rColParameters.m_clHeaderColDef.SetWidth( 20.0 );
	rColParameters.m_clDataColDef.SetWidth( 20.0 );
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetMainHeaderLinks( BS_SmartDpCMainHeaderID::BS_SMARTDPCMHID_SmartDpCDpSensor, BS_SmartDpCMainHeaderSubID::BS_SMARTDPCMHSUBID_DpSensor );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_DpSensorDpl;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_bCantRemove = false;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_DPL );
	rColParameters.m_strHeaderUnit = _T( "" );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_DpSensorBurstPressure;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BURSTPRESSURE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTDPCCID_DpSensorTemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;
}

bool CDlgBatchSelSmartDpC::BS_InitMainHeaders( void )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_InitMainHeaders() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
		}

		return false;
	}

	// Add main header for 'Smart diff. pressure controller'.
	DlgOutputHelper::CColDef clColMHInDef;
	clColMHInDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHInDef.SetMouseEventFlag( true );
	clColMHInDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );
	
	// To specify that the group 'Smart diff. pressure controller' is linked to the main header 'Output'.
	clColMHInDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output );
	m_clInterface.AddMainHeaderDef( BS_SmartDpCMainHeaderID::BS_SMARTDPCMHID_SmartDpCDpSensor, BS_SmartDpCMainHeaderSubID::BS_SMARTDPCMHSUBID_SmartDpC,
			clColMHInDef, TASApp.LoadLocalizedString( IDS_DLGBATCHSELSMARTDPC_TITLEGROUPSMARTDPC ), (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHInDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center, 
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _IMI_GRAY_MED, _WHITE );

	m_clInterface.AddMainHeaderFontDef( BS_SmartDpCMainHeaderID::BS_SMARTDPCMHID_SmartDpCDpSensor, BS_SmartDpCMainHeaderSubID::BS_SMARTDPCMHSUBID_SmartDpC,
			clFontMHInDef, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	// Add main header for 'Dp sensor sets'.
	DlgOutputHelper::CColDef clColMHOutDef;
	clColMHOutDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHOutDef.SetMouseEventFlag( true );
	clColMHOutDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );
	
	// To specify that the group 'Dp sensor sets' is linked to the main header 'Output'.
	clColMHOutDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output );
	
	m_clInterface.AddMainHeaderDef( BS_SmartDpCMainHeaderID::BS_SMARTDPCMHID_SmartDpCDpSensor, BS_SmartDpCMainHeaderSubID::BS_SMARTDPCMHSUBID_DpSensor,
			clColMHOutDef, TASApp.LoadLocalizedString( IDS_DLGBATCHSELSMARTDPC_TITLEGROUPDPSENSOR ), (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHOutDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center, 
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _IMI_GRAY_MED, _WHITE );
	
	m_clInterface.AddMainHeaderFontDef( BS_SmartDpCMainHeaderID::BS_SMARTDPCMHID_SmartDpCDpSensor, BS_SmartDpCMainHeaderSubID::BS_SMARTDPCMHSUBID_DpSensor,
			clFontMHOutDef, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	m_clInterface.SetMainHeaderRowHeight( BS_SmartDpCMainHeaderID::BS_SMARTDPCMHID_SmartDpCDpSensor, DLBBATCHSELSMARTDPC_ROWHEIGHT_MAINHEADER,
			(int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelSmartDpC::BS_InitColumns()
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_InitColumns() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
		}

		return false;
	}

	BS_AddOneColumn( BS_SmartDpCColumnID::BS_SMARTDPCCID_Name );
	BS_AddOneColumn( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorName );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelSmartDpC::BS_Validate()
{
	try
	{
		if( false == CDlgBatchSelBase::BS_Validate() )
		{
			return false;
		}

		CTable *pTab = dynamic_cast<CTable *>( m_clBatchSelSmartDpCParams.m_pTADS->Get( _T("SMARTDPC_TAB") ).MP );

		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'SMARTDIFFPRESS_TAB' table from the datastruct.") );
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

			CDS_SSelSmartDpC *pSSelSmartDpC = (CDS_SSelSmartDpC *)pclRowParameters->m_pclCDSSelSelected;

			if( NULL == pSSelSmartDpC )
			{
				// Create selected object and initialize it.
				pSSelSmartDpC = _BS_CreateSmartDpC( pclRowParameters );
			}

			if( NULL == pSSelSmartDpC )
			{
				continue;
			}

			IDPTR IDPtr = pSSelSmartDpC->GetIDPtr();
			pTab->Insert( IDPtr );

			bAtLeastOneInserted = true;

			// Remark: It's up to the database to clean now the object if no more needed. To avoid that the 'CDlgBatchSelBase::BS_ClearAllData' method
			//         try to clear, we set the pointer to NULL.
			pclRowParameters->m_pclCDSSelSelected = NULL;
		}

		return bAtLeastOneInserted;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelSmartDpC::BS_Validate'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelSmartDpC::BS_EnableRadios( bool bEnable )
{
	CDlgBatchSelBase::BS_EnableRadios( bEnable );

	m_clCheckboxDpBranch.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	m_clCheckboxDpMax.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable )
	{
		// In the base class, if 'bEnable' is 'true' we don't enable by default all controls that are in the
		// group. This is why we need to do it here for this control.
		if( BST_CHECKED == m_clCheckboxDpMax.GetCheck() )
		{
			m_clExtEditDpMax.EnableWindow( TRUE );
		}
	}

	GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
}

void CDlgBatchSelSmartDpC::BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters )
	{
		return;
	}

	for( int iLoop = BS_SmartDpCColumnID::BS_SMARTDPCCID_First; iLoop < BS_SmartDpCColumnID::BS_SMARTDPCCID_Last; iLoop++ )
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

bool CDlgBatchSelSmartDpC::BS_SuggestSelection()
{
	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL == pclBSMessage )
	{
		return false;
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	// We block the redraw because it is too long otherwise.
	BeginWaitCursor();
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;

	// Variable uses in test unit to drop out results in a file.
	m_clBatchSelSmartDpCParams.m_vecRowParameters.clear();

	for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
	{
		BSRowParameters *pclRowParameters = &iterRows->second;

		if( NULL == pclRowParameters )
		{
			continue;
		}

		// Put this 'pclRowParameters' in the 'CBatchSelectionParameters::m_vecRowParameters' to facilitate test unit.
		m_clBatchSelSmartDpCParams.m_vecRowParameters.push_back( pclRowParameters );

		// 'CBatchSelectSmartDpCList' is created and the pointer is saved in 'pclRowParameters->m_pclBatchResults' for each row. But the pointer is
		// also saved in the 'm_clBatchSelSmartDpCParams.m_pclBatchSmartDpCList' variable to help us calling 'SelectSmartDpC' method. This is the 
		// 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods that will delete this pointer. 
		// Thus no need to do it in the 'CBatchSelSmartDpCParams' destructor.
		pclRowParameters->m_pclBatchResults = new CBatchSelectSmartDpCList();

		if( NULL == pclRowParameters->m_pclBatchResults )
		{
			continue;
		}

		m_clBatchSelSmartDpCParams.m_pclBatchSmartDpCList = (CBatchSelectSmartDpCList *)pclRowParameters->m_pclBatchResults;
		long lCurrentRow = iterRows->first;
		
		pclBSMessage->Clear();

		pclRowParameters->m_pclSelectedProduct = NULL;

		if( CDS_SelProd::efdFlow != m_clBatchSelSmartDpCParams.m_eFlowOrPowerDTMode )
		{
			if( pclRowParameters->m_dPower > 0.0 && pclRowParameters->m_dDT > 0.0 )
			{
				CWaterChar clWaterCharReturn = m_clBatchSelSmartDpCParams.m_WC;
				double dReturnTemperature = ( ProjectType::Cooling == m_clBatchSelSmartDpCParams.m_eApplicationType ) ? m_clBatchSelSmartDpCParams.m_WC.GetTemp() + pclRowParameters->m_dDT :
						m_clBatchSelSmartDpCParams.m_WC.GetTemp() - pclRowParameters->m_dDT;

				clWaterCharReturn.UpdateFluidData( dReturnTemperature );

				pclRowParameters->m_dFlow = m_clBatchSelSmartDpCParams.m_WC.GetTemp() * m_clBatchSelSmartDpCParams.m_WC.GetDens() * m_clBatchSelSmartDpCParams.m_WC.GetSpecifHeat();
				pclRowParameters->m_dFlow -= ( clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat() );
				pclRowParameters->m_dFlow = abs( pclRowParameters->m_dPower / pclRowParameters->m_dFlow );
			}
		}

		int iReturn = m_clBatchSelSmartDpCParams.m_pclBatchSmartDpCList->SelectSmartDpC( &m_clBatchSelSmartDpCParams, pclRowParameters->m_dFlow );
		
		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturn & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturn & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturn & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturn & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			m_bAtLeastOneRowWithNoSolution = true;
			iterRows->second.m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
			pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_Name, _T( "" ), true );
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_OUTPUTBOXERROR, true );
			delete pclRowParameters->m_pclBatchResults;
			pclRowParameters->m_pclBatchResults = NULL;

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( IDS_BATCHSELECTION_NOSOLUTION ), 
						(int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturn & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelSmartDpCParams.m_pclBatchSmartDpCList->GetBestProduct();
			m_bAtLeastOneRowWithBestSolution = true;
			int iBitmapID;

			if( false == bAlternative )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
				iBitmapID = IDI_BATCHSELOK;
			}
			else
			{
				m_bAtLeastOneRowWithSolutionButAlternative = true;
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
				iBitmapID = IDI_OUTPUTBOXOKINFO;
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );
			BS_PrepareOutput( pclBSMessage, pclRowParameters );

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
				int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
						(int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturn & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelSmartDpCParams.m_pclBatchSmartDpCList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelSmartDpCParams.m_pclBatchSmartDpCList->GetFirst<CSelectedBase>();
				m_bAtLeastOneRowWithBestSolution = true;
				int iBitmapID;

				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
					iBitmapID = IDI_BATCHSELOK;
				}
				else
				{
					m_bAtLeastOneRowWithSolutionButAlternative = true;
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
					iBitmapID = IDI_OUTPUTBOXOKINFO;
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );
				BS_PrepareOutput( pclBSMessage, pclRowParameters );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
					
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
				}
			}
			else
			{
				m_bAtLeastOneRowWithSolutionButNotBest = true;
				int iBitmapID;

				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutions;
					iBitmapID = IDI_OUTPUTBOXWARNING;
				}
				else
				{
					m_bAtLeastOneRowWithSolutionButAlternative = true;
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutionsAlter;
					iBitmapID = IDI_OUTPUTBOXWARNINFO;
				}

				pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_Name, _T( "" ), true );
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_SEVRESULTS : IDS_BATCHSELECTION_SEVRESULTSWITHINFO;
		 			
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
				}
			}
		}

		// Send message.
		pclBSMessage->SendMessage( lCurrentRow, true, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	EndWaitCursor();

	return CDlgBatchSelBase::BS_SuggestSelection();
}

bool CDlgBatchSelSmartDpC::BS_AddOneColumn( int iColumnID )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_AddOneColumn( iColumnID ) )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
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
				m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
			}

			return false;
		}

		for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
		{
			long lCurrentRow = iterRows->first;
			pclBSMessage->Clear();

			_BS_FillCell( pclBSMessage, &iterRows->second, iColumnID );
			pclBSMessage->SendMessage( lCurrentRow, false, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	return true;
}

void CDlgBatchSelSmartDpC::BS_UpdateInputColumnLayout( int iUpdateWhat )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	switch( iUpdateWhat )
	{
		case BS_SmartDpCUpdateInputColumn::BS_SmartDpCUIC_DpBranch:

			if( true == m_mapColumnList[BS_ColumnID::BS_CID_Input_DpBranch].m_bEnabled )
			{
				// Input column.
				BS_AddOneColumn( BS_ColumnID::BS_CID_Input_DpBranch );

				// Add output column only if user has intentionally asked before.
				if( eBool3::eb3True == m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl );
				}
			}
			else
			{
				// Input column.
				BS_RemoveOneColumn( BS_ColumnID::BS_CID_Input_DpBranch );

				// Output column.
				if( true == m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl].m_bDisplayed )
				{
					BS_RemoveOneColumn( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl );
				}
			}

			break;

		case BS_SmartDpCUpdateInputColumn::BS_SmartDpCUIC_DpMax:

			if( true == m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax].m_bEnabled )
			{
				// Add output column only if user has intentionally asked before.
				if( eBool3::eb3True == m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax );
				}
			}
			else
			{
				// Output column.
				if( true == m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax].m_bDisplayed )
				{
					BS_RemoveOneColumn( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax);
				}
			}

			break;
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	CDlgBatchSelBase::BS_UpdateIOColumnLayout( iUpdateWhat );
}

// Remark: 'CW' is for 'Column Width'.
// HYS-1995: Save version. Version 2.
#define CW_BATCHSELSMARTDPC_VERSION			2
void CDlgBatchSelSmartDpC::BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}

	int iVersion = CW_BATCHSELSMARTDPC_VERSION;

	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth( CW_WINDOWID_BATCHSMARTDPC, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = ( CDS_BatchSelParameter::CCWBatchSelSheet * )pclCWWindow->GetCWSheet( 0 );

	if( pclCWSheet != NULL && pclCWSheet->GetVersion() == iVersion )
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

				if( iter->first >= BS_SMARTDPCCID_First )
				{
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3True;
				}
			}
			else
			{
				m_clInterface.SetColumnWidthInPixel( iter->first, iter->second.m_lWidth, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
			}
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	}
}

void CDlgBatchSelSmartDpC::BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth( CW_WINDOWID_BATCHSMARTDPC, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = ( CDS_BatchSelParameter::CCWBatchSelSheet * )pclCWWindow->GetCWSheet( 0, true );

	int iAfterColumnID = -1;
	CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfo mapColumnWidth;

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); iter++ )
	{
		CDS_BatchSelParameter::CCWBatchSelSheet::ColumnInfo rColumnInfo;
		rColumnInfo.m_bIsVisible = iter->second.m_bDisplayed;

		if( true == iter->second.m_bDisplayed )
		{
			m_clInterface.GetColumnWidthInPixel( iter->first, rColumnInfo.m_lWidth, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
			rColumnInfo.m_iAfterColumnID = iAfterColumnID;
			iAfterColumnID = iter->first;
		}

		mapColumnWidth[iter->first] = rColumnInfo;
	}

	pclCWSheet->GetMap() = mapColumnWidth;
	// HYS-1995: Save version.
	pclCWSheet->SetVersion( CW_BATCHSELSMARTDPC_VERSION );
}

void CDlgBatchSelSmartDpC::GetMaterialList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_clBatchSelSmartDpCParams.m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_SmartDpC:

			if( Above50 == eAboveOrBelow )
			{
				m_clBatchSelSmartDpCParams.m_pTADB->GetSmartDpCBdyMatList( pclRankEx, _T(""), _T(""), CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_clBatchSelSmartDpCParams.m_pTADB->GetSmartDpCBdyMatList( pclRankEx, _T(""), _T(""), CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelSmartDpC::GetConnectList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_clBatchSelSmartDpCParams.m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_SmartDpC:

			if( Above50 == eAboveOrBelow )
			{
				m_clBatchSelSmartDpCParams.m_pTADB->GetSmartDpCConnList( pclRankEx, _T(""), _T(""), (LPCTSTR)m_clBatchSelSmartDpCParams.m_strComboMaterialAbove50ID,
						CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_clBatchSelSmartDpCParams.m_pTADB->GetSmartDpCConnList( pclRankEx, _T(""), _T(""), (LPCTSTR)m_clBatchSelSmartDpCParams.m_strComboMaterialBelow65ID,
						CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelSmartDpC::GetPNList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_clBatchSelSmartDpCParams.m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_SmartDpC:

			if( Above50 == eAboveOrBelow )
			{
				m_clBatchSelSmartDpCParams.m_pTADB->GetSmartDpCPNList( pclRankEx, _T(""), _T(""), (LPCTSTR)m_clBatchSelSmartDpCParams.m_strComboMaterialAbove50ID,
						(LPCTSTR)m_clBatchSelSmartDpCParams.m_strComboConnectAbove50ID, _T(""), CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_clBatchSelSmartDpCParams.m_pTADB->GetSmartDpCPNList( pclRankEx, _T(""), _T(""), (LPCTSTR)m_clBatchSelSmartDpCParams.m_strComboMaterialBelow65ID,
						(LPCTSTR)m_clBatchSelSmartDpCParams.m_strComboConnectBelow65ID, _T(""), CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

CRViewSSelSS *CDlgBatchSelSmartDpC::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelSmartDpC;
}

void CDlgBatchSelSmartDpC::ClearAll( void )
{
	if( NULL == pRViewSSelSmartDpC )
	{
		return;
	}

	// Disable the 'Validate' button.
	if( false == m_clBatchSelSmartDpCParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	// Clear the right sheet.
	ResetRightView();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDlgBatchSelSmartDpC::_BS_ApplySelection()
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelSmartDpC )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == m_pclCurrentRowParameters || NULL == m_pclCurrentRowParameters->m_pclBatchResults )
	{
		return false;
	}

	CDB_TAProduct *pclTAProduct = pRViewSSelSmartDpC->GetCurrentSmartDpCSelected();

	if( NULL == pclTAProduct )
	{
		return false;
	}

	bool bFound = false;
	CBatchSelectBaseList *pclBatchList = m_pclCurrentRowParameters->m_pclBatchResults;

	for( CSelectedBase *pclSelectedSmartDpC = pclBatchList->GetFirst<CSelectedBase>(); NULL != pclSelectedSmartDpC && false == bFound;
			pclSelectedSmartDpC = pclBatchList->GetNext<CSelectedBase>() )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pclSelectedSmartDpC->GetpData() );

		if( pclSmartDpC == pclTAProduct )
		{
			m_pclCurrentRowParameters->m_pclSelectedProduct = pclSelectedSmartDpC;
			bFound = true;
		}
	}

	if( false == bFound )
	{
		return false;
	}

	// When user comes back from the right view (by selecting a product, actuator, accessories, ...) we have two possibilities. If user edits a
	// product for which there is only one solution (marks with the green arrow icon in the 'Status' column), we create a 'CDS_SSelSmartDpC' in the
	// 'OnDlgOutputCellDblClicked' method. If user edits a product for which there is more than one solution (marks with yellow exclamation icon), we can't
	// create. In this case we have to do it here.
	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		m_pclCurrentRowParameters->m_pclCDSSelSelected = _BS_CreateSmartDpC( m_pclCurrentRowParameters );
	}

	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		return false;
	}

	pRViewSSelSmartDpC->FillInSelected( m_pclCurrentRowParameters->m_pclCDSSelSelected );

	// Memorize current block selection if exist.
	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	// If previous status contained alternative, we reset this flag and keep only the user action.
	m_pclCurrentRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;

	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL != pclBSMessage )
	{
		pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_BATCHSELOK, true );
	}

	BS_PrepareOutput( pclBSMessage, m_pclCurrentRowParameters );

	// Send message.
	pclBSMessage->SendMessage( m_pclCurrentRowParameters->m_lRow, true, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	// Restore the previous selections.
	m_clInterface.RestoreSelections( (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );

	// HYS-1168 : We hide the DockablePane in edition mode, we have remove the auto hide mode when doing Apply.
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}

	// Change the tools dockable pane to full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, true );

	pRViewSSelSmartDpC->UnregisterNotificationHandler( this );

	m_pclCurrentRowParameters = NULL;
	ClearAll();

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	SetModificationMode( false );
	return true;
}

CDS_SSelSmartDpC *CDlgBatchSelSmartDpC::_BS_CreateSmartDpC( BSRowParameters *pclRowParameters )
{
	try
	{
		if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct || _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() 
				|| NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList() 
				|| NULL == m_clBatchSelSmartDpCParams.m_pclBatchSmartDpCList )
		{
			return NULL;
		}

		CDB_SmartControlValve *pclSmartDpC = (CDB_SmartControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

		if( NULL == pclSmartDpC )
		{
			return NULL;
		}

		// Create selected object and initialize it.
		CDS_SSelSmartDpC *pclSelectedSmartDpC = NULL;
		IDPTR IDPtr = _NULL_IDPTR;
		m_clBatchSelSmartDpCParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSmartDpC ) );

		pclSelectedSmartDpC = (CDS_SSelSmartDpC *)( IDPtr.MP );
		pclSelectedSmartDpC->SetSmartDpCIDPtr( pclSmartDpC->GetIDPtr() );

		pclSelectedSmartDpC->SetCheckboxDpBranch( ( true == m_clBatchSelSmartDpCParams.m_bIsCheckboxDpBranchChecked ) ? BST_CHECKED : BST_UNCHECKED );
		pclSelectedSmartDpC->SetDpBranchValue( pclRowParameters->m_dDpBranch );

		pclSelectedSmartDpC->SetCheckboxDpMax( ( true == m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked ) ? BST_CHECKED : BST_UNCHECKED );
		pclSelectedSmartDpC->SetDpMaxValue( m_clBatchSelSmartDpCParams.m_dDpMax );

		// HYS-1992: We get the fictif set (DpSensor or Connection set).
		CDB_DpSensor *pclSensorSet = (CDB_DpSensor *)m_clBatchSelSmartDpCParams.m_pclBatchSmartDpCList->GetBestDpSensorSet( &m_clBatchSelSmartDpCParams, pclRowParameters->m_dDpBranch );

		if( NULL != pclSensorSet )
		{
			pclSelectedSmartDpC->SetSetIDPtr( pclSensorSet->GetIDPtr() );

			CAccessoryList *pclSmartDpCAccessoryListToFill = pclSelectedSmartDpC->GetSetContentAccessoryList();
			CDB_RuledTable *pRuledTable = (CDB_RuledTable *)(pclSensorSet->GetAccessoriesGroupIDPtr().MP );

			if( NULL != pclSmartDpCAccessoryListToFill && NULL != pRuledTable )
			{
				CRank rList;
				int iCount = m_clBatchSelSmartDpCParams.m_pTADB->GetAccessories( &rList, pRuledTable, m_clBatchSelSmartDpCParams.m_eFilterSelection );

				if( iCount > 0 )
				{
					CString str;
					LPARAM lparam;
					BOOL bContinue = rList.GetFirst( str, lparam );

					while( TRUE == bContinue )
					{
						CDB_Product *pclAccessory = (CDB_Product *)lparam;
						VERIFY( NULL != pclAccessory );

						if( false == pclAccessory->IsAnAccessory() )
						{
							bContinue = rList.GetNext( str, lparam );
							continue;
						}

						pclSmartDpCAccessoryListToFill->Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_SetAccessory, pRuledTable );
						bContinue = rList.GetNext( str, lparam );
					}
				}
			}
		}

		pclSelectedSmartDpC->SetQ( pclRowParameters->m_dFlow );

		if( CDS_SelProd::efdFlow == m_clBatchSelSmartDpCParams.m_eFlowOrPowerDTMode )
		{
			pclSelectedSmartDpC->SetFlowDef( CDS_SelProd::efdFlow );
			pclSelectedSmartDpC->SetPower( 0.0 );
			pclSelectedSmartDpC->SetDT( 0.0 );
		}
		else
		{
			pclSelectedSmartDpC->SetFlowDef( CDS_SelProd::efdPower );
			pclSelectedSmartDpC->SetPower( pclRowParameters->m_dPower );
			pclSelectedSmartDpC->SetDT( pclRowParameters->m_dDT );
		}

		pclSelectedSmartDpC->SetDp( pclRowParameters->m_pclSelectedProduct->GetDp() );

		pclSelectedSmartDpC->SetMaterialID( pclSmartDpC->GetBodyMaterialIDPtr().ID );
		pclSelectedSmartDpC->SetConnectID( pclSmartDpC->GetConnectIDPtr().ID );
		pclSelectedSmartDpC->SetPNID( pclSmartDpC->GetPNIDPtr().ID );

		pclSelectedSmartDpC->SetPipeSeriesID( m_clBatchSelSmartDpCParams.m_strPipeSeriesID );
		pclSelectedSmartDpC->SetPipeID( m_clBatchSelSmartDpCParams.m_strPipeID );

		// Selected pipe informations.
		pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSmartDpC->GetSizeKey(), *pclSelectedSmartDpC->GetpSelPipe() );

		*pclSelectedSmartDpC->GetpSelectedInfos()->GetpWCData() = m_clBatchSelSmartDpCParams.m_WC;
		pclSelectedSmartDpC->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef1, pclRowParameters->m_strReference1 );
		pclSelectedSmartDpC->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef2, pclRowParameters->m_strReference2 );
		pclSelectedSmartDpC->GetpSelectedInfos()->SetQuantity( 1 );

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pclSelectedSmartDpC->GetpSelectedInfos()->SetDT( m_clBatchSelSmartDpCParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pclSelectedSmartDpC->GetpSelectedInfos()->SetApplicationType( m_clBatchSelSmartDpCParams.m_eApplicationType );

		CTable *pTab = m_clBatchSelSmartDpCParams.m_pTADS->GetpClipboardTable();
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CLIPBOARD_TABLE' table from the datastruct.") );
		}

		if( _T('\0') == *IDPtr.ID )
		{
			pTab->Insert( IDPtr );
		}

		return pclSelectedSmartDpC;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelSmartDpC::_BS_CreateSmartDpC'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelSmartDpC::_BS_DisplaySolutions()
{
	if( NULL != pRViewSSelSmartDpC )
	{
		m_bRViewEmpty = false;
		SetModificationMode( true );

		// Remark: The 'SetModificationMode' method just above set the 'm_pclBatchSelParams->m_bEditModeRunning' to true.
		//         Thus we need to manually set it in 'm_clIndSelSmartDpCParams' for the right view to pass in edition mode.
		m_clIndSelSmartDpCParams.m_bEditModeRunning = true;

		pRViewSSelSmartDpC->Suggest( &m_clIndSelSmartDpCParams );
	}

	return;
}

void CDlgBatchSelSmartDpC::_BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct
			|| _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() 
			|| NULL == dynamic_cast<CBatchSelectSmartDpCList *>( pclRowParameters->m_pclBatchResults )
			|| iColumnID < BS_SmartDpCColumnID::BS_SMARTDPCCID_First || iColumnID >= BS_SmartDpCColumnID::BS_SMARTDPCCID_Last )
	{
		return;
	}

	CBatchSelectSmartDpCList *pclBatchSelectSmartDpCList = (CBatchSelectSmartDpCList *)( pclRowParameters->m_pclBatchResults );
	CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

	if( NULL == pclSmartDpC )
	{
		return;
	}

	CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelSmartDpCParams.m_pTADS->GetpTechParams();

	if( NULL == pclTechnicalParameter )
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
	double dRho = m_clBatchSelSmartDpCParams.m_WC.GetDens();
	double dKinVisc = m_clBatchSelSmartDpCParams.m_WC.GetKinVisc();

	CString str;

	switch( iColumnID )
	{
		case BS_SmartDpCColumnID::BS_SMARTDPCCID_Name:
			pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_Name, pclSmartDpC->GetName(), true );
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_Material:
			pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_Material, pclSmartDpC->GetBodyMaterial(), true );
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_Connection:
			pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_Connection, pclSmartDpC->GetConnect(), true );
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_PN:
			pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_PN, pclSmartDpC->GetPN().c_str(), true );
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_Size:
			pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_Size, pclSmartDpC->GetSize(), true );
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_Kvs:
			pclBSMessage->SetCellNumber( BS_SmartDpCColumnID::BS_SMARTDPCCID_Kvs, pclSmartDpC->GetKvs(), iPhysicalType, iMaxDigit, iMinDecimal, true );
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_Qnom:
			pclBSMessage->SetCellNumber( BS_SmartDpCColumnID::BS_SMARTDPCCID_Kvs, pclSmartDpC->GetQNom(), iPhysicalType, iMaxDigit, iMinDecimal, true );
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMin:
			pclBSMessage->SetCellNumber( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMin, pclRowParameters->m_pclSelectedProduct->GetDp(), iPhysicalType, iMaxDigit, iMinDecimal, true );
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax:
			{
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax, m_clBatchSelSmartDpCParams.m_dDpMax, 
						iPhysicalType, iMaxDigit, iMinDecimal, true );

				if( NULL != pclCellBase )
				{
					if( true == m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked && m_clBatchSelSmartDpCParams.m_dDpMax > 0.0
							&& m_clBatchSelSmartDpCParams.m_dDpMax > pclSmartDpC->GetDpmax() )
					{
						pclCellBase->SetTextForegroundColor( _RED );
					}
				}
			}
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_TemperatureRange:
			{
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_TemperatureRange, 
						pclSmartDpC->GetTempRange(), true );

				if( NULL != pclCellBase )
				{
					if( m_clBatchSelSmartDpCParams.m_WC.GetTemp() < pclSmartDpC->GetTmin() || m_clBatchSelSmartDpCParams.m_WC.GetTemp() > pclSmartDpC->GetTmax() )
					{
						pclCellBase->SetTextForegroundColor( _RED );
					}
				}
			}
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_PipeSize:
			{
				CString str = _T( "" );
				CSelectPipe selPipe( &m_clBatchSelSmartDpCParams );

				if( NULL != pclBatchSelectSmartDpCList->GetSelectPipeList() )
				{
					pclBatchSelectSmartDpCList->GetSelectPipeList()->GetMatchingPipe( pclSmartDpC->GetSizeKey(), selPipe );
				}

				pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_PipeSize, selPipe.GetpPipe()->GetName(), true );
			}
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_PipeLinDp:
			{
				CSelectPipe selPipe( &m_clBatchSelSmartDpCParams );

				if( NULL != pclBatchSelectSmartDpCList->GetSelectPipeList() )
				{
					pclBatchSelectSmartDpCList->GetSelectPipeList()->GetMatchingPipe( pclSmartDpC->GetSizeKey(), selPipe );
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SmartDpCColumnID::BS_SMARTDPCCID_PipeLinDp, selPipe.GetLinDp(), 
						iPhysicalType, iMaxDigit, iMinDecimal, true );

				CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelSmartDpCParams.m_pTADS->GetpTechParams();

				if( NULL != pclCellBase && NULL != pclTechnicalParameter )
				{
					if( selPipe.GetLinDp() > pclTechnicalParameter->GetPipeMaxDp() || selPipe.GetLinDp() < pclTechnicalParameter->GetPipeMinDp() )
					{
						pclCellBase->SetTextForegroundColor( _ORANGE );
					}
				}
			}
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_PipeV:
			{
				CSelectPipe selPipe( &m_clBatchSelSmartDpCParams );

				if( NULL != pclBatchSelectSmartDpCList->GetSelectPipeList() )
				{
					pclBatchSelectSmartDpCList->GetSelectPipeList()->GetMatchingPipe( pclSmartDpC->GetSizeKey(), selPipe );
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SmartDpCColumnID::BS_SMARTDPCCID_PipeV, selPipe.GetU(),
						iPhysicalType, iMaxDigit, iMinDecimal, true );

				CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelSmartDpCParams.m_pTADS->GetpTechParams();

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

						m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
						m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelSmartDpCParams.m_eProductSubCategory );
					}
				}
			}
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorName:
			{
			    // HYS-1992: Do not set the DpSensorSet name when user changes this. 
			    CDB_Product* pclBestDpSensor = NULL; 

				if( NULL == pclRowParameters->m_pclCDSSelSelected )
				{
					pclBestDpSensor = pclBatchSelectSmartDpCList->GetBestDpSensorSet( &m_clBatchSelSmartDpCParams, pclRowParameters->m_dDpBranch );
				}
				else
				{
					pclBestDpSensor = (CDB_Product*)((CDS_SSelSmartDpC*)pclRowParameters->m_pclCDSSelSelected)->GetSetIDPtr().MP;
				}

				CString strName = pclBestDpSensor->GetName();

				if( NULL != dynamic_cast<CDB_DpSensor*>(pclBestDpSensor) )
				{
					strName = ((CDB_DpSensor*)pclBestDpSensor)->GetFullName();
				}

				if( NULL != pclBestDpSensor )
				{
					pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorName, strName, true );
				}
			}
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl:
			{
				CDB_DpSensor *pclBestDpSensor = (CDB_DpSensor*)pclBatchSelectSmartDpCList->GetBestDpSensorSet( &m_clBatchSelSmartDpCParams, pclRowParameters->m_dDpBranch );

				if( NULL != pclBestDpSensor )
				{
					DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl, pclRowParameters->m_dDpBranch, iPhysicalType, 
							iMaxDigit, iMinDecimal, true );

					if( NULL != pclCellBase )
					{
						if( pclRowParameters->m_dDpBranch < pclBestDpSensor->GetMinMeasurableDp() || pclRowParameters->m_dDpBranch > pclBestDpSensor->GetMaxMeasurableDp() )
						{
							pclCellBase->SetTextForegroundColor( _RED );
						}
					}
				}
			}
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorBurstPressure:
			{
				CDB_Product *pclBestDpSensor = pclBatchSelectSmartDpCList->GetBestDpSensorSet( &m_clBatchSelSmartDpCParams, pclRowParameters->m_dDpBranch );

				if( NULL != pclBestDpSensor )
				{
					DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorBurstPressure, 
							pclBestDpSensor->GetPmaxmax(), iPhysicalType, iMaxDigit, iMinDecimal, true );

					if( NULL != pclCellBase )
					{
						if( true == m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked && m_clBatchSelSmartDpCParams.m_dDpMax > 0.0
								&& m_clBatchSelSmartDpCParams.m_dDpMax > pclBestDpSensor->GetPmaxmax() )
						{
							pclCellBase->SetTextForegroundColor( _RED );
						}
					}
				}
			}
			break;

		case BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorTemperatureRange:
			{
				CDB_Product *pclBestDpSensor = pclBatchSelectSmartDpCList->GetBestDpSensorSet( &m_clBatchSelSmartDpCParams, pclRowParameters->m_dDpBranch );

				if( NULL != pclBestDpSensor )
				{
					DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorTemperatureRange, 
							pclBestDpSensor->GetTempRange(), true );

					if( NULL != pclCellBase )
					{
						if( m_clBatchSelSmartDpCParams.m_WC.GetTemp() < pclBestDpSensor->GetTmin() || m_clBatchSelSmartDpCParams.m_WC.GetTemp() > pclBestDpSensor->GetTmax() )
						{
							pclCellBase->SetTextForegroundColor( _RED );
						}
					}
				}
			}
			break;
	}
}

void CDlgBatchSelSmartDpC::_UpdateDpBranchCheckboxState()
{
	m_clBatchSelSmartDpCParams.m_bIsCheckboxDpBranchChecked = ( BST_CHECKED == m_clCheckboxDpBranch.GetCheck() ) ? true : false;
	m_mapColumnList[BS_ColumnID::BS_CID_Input_DpBranch].m_bEnabled = m_clBatchSelSmartDpCParams.m_bIsCheckboxDpBranchChecked;
	m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpSensorDpl].m_bEnabled = m_clBatchSelSmartDpCParams.m_bIsCheckboxDpBranchChecked;
}

void CDlgBatchSelSmartDpC::_UpdateDpBranchColumn()
{
	BS_UpdateInputColumnLayout( BS_SmartDpCUpdateInputColumn::BS_SmartDpCUIC_DpBranch );
}

void CDlgBatchSelSmartDpC::_UpdateDpMaxFieldState()
{
	m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked = ( BST_CHECKED == m_clCheckboxDpMax.GetCheck() ) ? true : false;

	if( true == m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked )
	{
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( TRUE );
		m_clExtEditDpMax.SetFocus();
		m_clExtEditDpMax.SetWindowText( _T( "" ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( TRUE );
	}
	else
	{
		m_clBatchSelSmartDpCParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( FALSE );
		m_clExtEditDpMax.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( FALSE );
	}

	m_mapColumnList[BS_SmartDpCColumnID::BS_SMARTDPCCID_DpMax].m_bEnabled = m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked;
}

void CDlgBatchSelSmartDpC::_UpdateDpMaxColumn()
{
	BS_UpdateInputColumnLayout( BS_SmartDpCUpdateInputColumn::BS_SmartDpCUIC_DpMax );
}

bool CDlgBatchSelSmartDpC::_CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters )
{
	// Here, we can have either a best result that is saved in the 'pclRowParameters->m_pclSelectedProduct' variable or we can have more that one result.
	// If we have a best result, we save it in a 'CDS_SSelSmartDpC' object and we call the 'RViewSSelSmartDpC::Suggest' method in edit mode. 
	// Otherwise, we call the 'RViewSSelSmartDpC::Suggest' method in normal mode to display all results and let the user to choose one.

	if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()
			|| NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe() )
	{
		ASSERTA_RETURN( false );
	}

	// This method will just copy the variables that are in the 'CProductSelectionParameters' base class.
	m_clIndSelSmartDpCParams.CopyFrom( &m_clBatchSelSmartDpCParams );
	
	// We need to do it manually for all variables of 'm_clBatchSelSmartDpCParams' that are needed by the 'RViewSSelSmartDpC'.

	m_clIndSelSmartDpCParams.m_eFilterSelection = m_clBatchSelSmartDpCParams.m_eFilterSelection;

	m_clIndSelSmartDpCParams.m_bIsGroupDpbranchChecked = m_clBatchSelSmartDpCParams.m_bIsCheckboxDpBranchChecked;
	m_clIndSelSmartDpCParams.m_dDpBranch = pclRowParameters->m_dDpBranch;
	
	m_clIndSelSmartDpCParams.m_bIsDpMaxChecked = m_clBatchSelSmartDpCParams.m_bIsDpMaxChecked;
	m_clIndSelSmartDpCParams.m_dDpMax = m_clBatchSelSmartDpCParams.m_dDpMax;

	// Create now the 'CSelectSmartDpCList' that 'RViewSSelSmartDpC' needs.
	if( NULL != m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList )
	{
		delete m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList;
		m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList = NULL;
	}

	m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList = new CSelectSmartDpCList();

	if( NULL == m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList )
	{
		ASSERTA_RETURN( false );
	}

	// We now transfer results from the batch selection list to the individual selection list.
	pclRowParameters->m_pclBatchResults->TransferResultsToIndividualSelectList( m_clIndSelSmartDpCParams.m_pclSelectSmartDpCList );

	// Create an 'CDS_SSelSmartDpC' to allow 'RViewSSelSmartDpC' to be in edit mode with this object selected.
	// Remark: Only if we have only one result.
	if( NULL != pclRowParameters->m_pclSelectedProduct )
	{
		CDS_SSelSmartDpC *pclSSelSmartDpC = NULL;

		if( NULL == pclRowParameters->m_pclCDSSelSelected )
		{
			pclSSelSmartDpC = _BS_CreateSmartDpC( pclRowParameters );
		}
		else
		{
			pclSSelSmartDpC = dynamic_cast<CDS_SSelSmartDpC *>( pclRowParameters->m_pclCDSSelSelected );
		}

		if( NULL == pclSSelSmartDpC )
		{
			return false;
		}

		pclRowParameters->m_pclCDSSelSelected = pclSSelSmartDpC;
		m_clIndSelSmartDpCParams.m_SelIDPtr = pclSSelSmartDpC->GetIDPtr();
	}

	return true;
}
