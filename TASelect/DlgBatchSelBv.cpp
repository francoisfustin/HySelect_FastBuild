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

#include "RViewSSelBv.h"

#include "DlgBatchSelBv.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelBv::CDlgBatchSelBv( CWnd *pParent )
	: CDlgBatchSelBase( m_clBatchSelBVParams, CDlgBatchSelBv::IDD, pParent )
{
	m_clBatchSelBVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	m_clBatchSelBVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;
	m_clIndSelBVParams.m_pclSelectBvList = NULL;
}

void CDlgBatchSelBv::SaveSelectionParameters()
{
	CDlgBatchSelBase::SaveSelectionParameters();

	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelBVParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return;
	}

	pclBatchSelParameter->SetBvFlowRadioState( (int)m_clBatchSelBVParams.m_eFlowOrPowerDTMode );
	pclBatchSelParameter->SetBvDpCheckboxState( (int)m_clBatchSelBVParams.m_bIsDpGroupChecked );
	pclBatchSelParameter->SetBvTypeBelow65ID( m_clBatchSelBVParams.m_strComboTypeBelow65ID );
	pclBatchSelParameter->SetBvFamilyBelow65ID( m_clBatchSelBVParams.m_strComboFamilyBelow65ID );
	pclBatchSelParameter->SetBvMaterialBelow65ID( m_clBatchSelBVParams.m_strComboMaterialBelow65ID );
	pclBatchSelParameter->SetBvConnectBelow65ID( m_clBatchSelBVParams.m_strComboConnectBelow65ID );
	pclBatchSelParameter->SetBvVersionBelow65ID( m_clBatchSelBVParams.m_strComboVersionBelow65ID );
	pclBatchSelParameter->SetBvPNBelow65ID( m_clBatchSelBVParams.m_strComboPNBelow65ID );
	pclBatchSelParameter->SetBvTypeAbove50ID( m_clBatchSelBVParams.m_strComboTypeAbove50ID );
	pclBatchSelParameter->SetBvFamilyAbove50ID( m_clBatchSelBVParams.m_strComboFamilyAbove50ID );
	pclBatchSelParameter->SetBvMaterialAbove50ID( m_clBatchSelBVParams.m_strComboMaterialAbove50ID );
	pclBatchSelParameter->SetBvConnectAbove50ID( m_clBatchSelBVParams.m_strComboConnectAbove50ID );
	pclBatchSelParameter->SetBvVersionAbove50ID( m_clBatchSelBVParams.m_strComboVersionAbove50ID );
	pclBatchSelParameter->SetBvPNAbove50ID( m_clBatchSelBVParams.m_strComboPNAbove50ID );

	BS_WriteAllColumnWidth( pclBatchSelParameter );
}

bool CDlgBatchSelBv::ResetRightView()
{
	if( NULL == pRViewSSelBv )
	{
		return false;
	}

	if( true == pRViewSSelBv->IsEmpty() )
	{
		return true;
	}

	pRViewSSelBv->Reset();
	return true;
}

bool CDlgBatchSelBv::OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList )
{
	if( false == CDlgBatchSelBase::OnBatchSelectionGetColumnList( mapColumnList ) )
	{
		return false;
	}

	// Verify if we must display Dp at 50% or not.
	if( 0 != mapColumnList.count( BS_BVCID_DpHalfOpening ) )
	{
		mapColumnList[BS_BVCID_DpHalfOpening].m_bEnable = !m_clBatchSelBVParams.m_bIsDpGroupChecked;
	}

	return true;
}

bool CDlgBatchSelBv::OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &fCatExist, CData* &pclProduct )
{
	if( false == CDlgBatchSelBase::OnBatchSelectionIsCatalogExist( iColumnID, lRowRelative, fCatExist, pclProduct ) )
	{
		return false;
	}

	if( iColumnID >= BS_BVColumnID::BS_BVCID_Name && iColumnID < BS_BVColumnID::BS_BVCID_PipeSize &&
		NULL != m_mapAllRowData[lRowRelative].m_pclSelectedProduct )
	{
		// Check valve.
		CDB_TAProduct *pclTAProduct = ( CDB_TAProduct * )( m_mapAllRowData[lRowRelative].m_pclSelectedProduct->GetProductIDPtr().MP );
		fCatExist = ( true == pclTAProduct->VerifyOneCatExist() );
		pclProduct = pclTAProduct;
	}

	return true;
}

bool CDlgBatchSelBv::OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelBv )
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
	if( false == m_clInterface.IsRowValidForData( lRowRelative, (int)m_clBatchSelBVParams.m_eProductSubCategory ) )
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

	pRViewSSelBv->Invalidate();
	pRViewSSelBv->UpdateWindow();
	pRViewSSelBv->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );

	m_clInterface.SelectRow( lRowRelative, true, (int)m_clBatchSelBVParams.m_eProductSubCategory );

	_BS_DisplaySolutions();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelBv, CDlgBatchSelBase )
	ON_COMMAND( IDC_CHECKDP, OnBnClickedCheckDp )
END_MESSAGE_MAP()

void CDlgBatchSelBv::DoDataExchange( CDataExchange *pDX )
{
	CDlgBatchSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKDP, m_clCheckboxDp );
}

BOOL CDlgBatchSelBv::OnInitDialog()
{
	CDlgBatchSelBase::OnInitDialog();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_DP );
	m_clCheckboxDp.SetWindowText( str );
	m_clCheckboxDp.SetCheck( BST_UNCHECKED );

	return TRUE;
}

void CDlgBatchSelBv::OnBnClickedCheckDp()
{
	m_clBatchSelBVParams.m_bIsDpGroupChecked = ( BST_CHECKED == m_clCheckboxDp.GetCheck() ) ? true : false;

	if( 0 == m_mapAllRowData.size()
		|| BS_InputsVerificationStatus::BS_IVS_Empty == m_eInputsVerificationStatus
		|| BS_InputsVerificationStatus::BS_IVS_NotYetDone == m_eInputsVerificationStatus )
	{
		// When there is not yet data pasted in the batch selection output, user can change input method (Flow or Power/DT and Dp or not).
		BS_UpdateIOColumnLayout( BS_UpdateInputColumn::BS_UIC_Dp );
	}
}

void CDlgBatchSelBv::OnBnClickedSuggest()
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

void CDlgBatchSelBv::OnBnClickedValidate()
{
	CDlgBatchSelBase::OnBnClickedValidate();

	if( true == m_clBatchSelBVParams.m_bEditModeRunning )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBVParams.m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
		}

		_BS_ApplySelection();

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
		}
	}
}

void CDlgBatchSelBv::OnBnClickedCancel()
{
	// User has edited a valid selection to change it but finally cancel it.
	OnRViewSSelKeyboardEvent( VK_ESCAPE );
	ResetRightView();
}

void CDlgBatchSelBv::ActivateLeftTabDialog()
{
	CDlgBatchSelBase::ActivateLeftTabDialog();
	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clBatchSelBVParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clBatchSelBVParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

LRESULT CDlgBatchSelBv::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelBVParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return -1;
	}

	if( NULL != m_clIndSelBVParams.m_pclSelectBvList )
	{
		delete m_clIndSelBVParams.m_pclSelectBvList;
		m_clIndSelBVParams.m_pclSelectBvList = NULL;
	}

	// Fill the combo box.
	// Remark: Corresponding combo variables in 'm_clBatchSelBVParams' are updated in each of this following methods.
	FillComboBoxType( AboveOrBelow::Below65, pclBatchSelParameter->GetBvTypeBelow65ID() );
	FillComboBoxFamily( AboveOrBelow::Below65, pclBatchSelParameter->GetBvFamilyBelow65ID() );
	FillComboBoxBodyMaterial( AboveOrBelow::Below65, pclBatchSelParameter->GetBvMaterialBelow65ID() );
	FillComboBoxConnect( AboveOrBelow::Below65, pclBatchSelParameter->GetBvConnectBelow65ID() );
	FillComboBoxVersion( AboveOrBelow::Below65, pclBatchSelParameter->GetBvVersionBelow65ID() );
	FillComboBoxPN( AboveOrBelow::Below65, pclBatchSelParameter->GetBvPNBelow65ID() );

	FillComboBoxType( AboveOrBelow::Above50, pclBatchSelParameter->GetBvTypeAbove50ID() );
	FillComboBoxFamily( AboveOrBelow::Above50, pclBatchSelParameter->GetBvFamilyAbove50ID() );
	FillComboBoxBodyMaterial( AboveOrBelow::Above50, pclBatchSelParameter->GetBvMaterialAbove50ID() );
	FillComboBoxConnect( AboveOrBelow::Above50, pclBatchSelParameter->GetBvConnectAbove50ID() );
	FillComboBoxVersion( AboveOrBelow::Above50, pclBatchSelParameter->GetBvVersionAbove50ID() );
	FillComboBoxPN( AboveOrBelow::Above50, pclBatchSelParameter->GetBvPNAbove50ID() );

	// Set the 'Flow' radio state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclBatchSelParameter->GetBvRadioFlowState() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clBatchSelBVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgBatchSelBase::UpdateFlowOrPowerDTState();

	// Set the 'Dp' check box state.
	m_clBatchSelBVParams.m_bIsDpGroupChecked = ( 0 == pclBatchSelParameter->GetBvDpCheckboxState() ) ? false : true;
	m_clCheckboxDp.SetCheck( pclBatchSelParameter->GetBvDpCheckboxState() );

	// This is needed only in the case of the dialog is currently active when user creates a new document (or opens a new one).
	OnAfterNewDocument();

	m_bInitialised = true;
	return 0;
}

CRViewSSelSS *CDlgBatchSelBv::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelBv;
}

void CDlgBatchSelBv::ClearAll( void )
{
	// Disable the 'Validate' button.
	if( false == m_clBatchSelBVParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	CDlgSelectionBase::ClearAll();
}

void CDlgBatchSelBv::BS_PrepareColumnList( void )
{
	CDlgBatchSelBase::BS_PrepareColumnList();

	BSColParameters rColParameters;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Prepare header and data parameters.
	rColParameters.m_clHeaderColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output,
			DlgOutputHelper::CColDef::AutoResizeDisabled, DlgOutputHelper::CColDef::Visible, DlgOutputHelper::CColDef::MouseEventEnabled, 
			DlgOutputHelper::CColDef::SelectionDisabled, DlgOutputHelper::CColDef::BlockSelectionDisabled, DlgOutputHelper::CColDef::RowSelectionDisabled, 
			DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );
	
	rColParameters.m_clHeaderColDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );

	rColParameters.m_clHeaderFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Center, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled, DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_clDataColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output,
			DlgOutputHelper::CColDef::AutoResizeDisabled, DlgOutputHelper::CColDef::Visible, DlgOutputHelper::CColDef::MouseEventEnabled, 
			DlgOutputHelper::CColDef::SelectionEnabled, DlgOutputHelper::CColDef::BlockSelectionEnabled, DlgOutputHelper::CColDef::RowSelectionEnabled, 
			DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataEnabled );

	rColParameters.m_clDataColDef.SetRowSeparatorFlag( true, DLGBATCHSELBASE_COLOR_DATAROWSEPARATOR );

	rColParameters.m_clDataFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Left, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled, DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_iColumnID = BS_BVCID_Name;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_bLineBelowHeader = true;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_Material;
	rColParameters.m_clHeaderColDef.SetWidth( 12.0 );
	rColParameters.m_clDataColDef.SetWidth( 12.0 );
	rColParameters.m_bCantRemove = false;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_Connection;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_Version;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_VERSION );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_PN;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_Size;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHVALVESIZE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_Preset;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetMaxDigit( 2 );
	rColParameters.m_clDataColDef.SetMinDecimal( 2 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PRESET );
	rColParameters.m_strHeaderUnit = TASApp.LoadLocalizedString( IDS_SHEETHDR_TURNSPOS );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_DpSignal;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SHEETHDR_SIGNAL );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	rColParameters.m_strHeaderUnit = _T( "" );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_Dp;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DP );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_DpFullOpening;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPFO );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_DpHalfOpening;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPHO );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_TemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_PipeSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPESIZE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_PipeLinDp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_LINPRESSDROP );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPELINDP );
	rColParameters.m_iHeaderUnit = _U_LINPRESSDROP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_BVCID_PipeV;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_VELOCITY );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPEV );
	rColParameters.m_iHeaderUnit = _U_VELOCITY;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;
}

bool CDlgBatchSelBv::BS_InitColumns()
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_InitColumns() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
		}

		return false;
	}

	BS_AddOneColumn( BS_BVColumnID::BS_BVCID_Name );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelBv::BS_Validate()
{
	try
	{
		if( false == CDlgBatchSelBase::BS_Validate() )
		{
			return false;
		}

		CTable *pTab = dynamic_cast<CTable *>( m_clBatchSelBVParams.m_pTADS->Get( _T("REGVALV_TAB") ).MP );
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'REGVALV_TAB' table from the datastruct.") );
		}

		bool bAtLeastOneInserted = false;

		for( mapLongRowParamsIter iterRow = m_mapAllRowData.begin(); iterRow != m_mapAllRowData.end();
			 iterRow++ )
		{
			BSRowParameters *pclRowParameters = &iterRow->second;

			if( RowStatus::BS_RS_FindOneSolution != pclRowParameters->m_eStatus 
					&& RowStatus::BS_RS_FindOneSolutionAlter != pclRowParameters->m_eStatus )
			{
				continue;
			}

			CDS_SSelBv *pSSelBv = ( CDS_SSelBv * )pclRowParameters->m_pclCDSSelSelected;

			if( NULL == pSSelBv )
			{
				// Create selected object and initialize it.
				pSSelBv = _BS_CreateSSelBv( pclRowParameters );
			}

			if( NULL == pSSelBv )
			{
				continue;
			}

			IDPTR IDPtr = pSSelBv->GetIDPtr();
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
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelBv::BS_Validate'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelBv::BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults )
	{
		return;
	}

	for( int iLoop = BS_BVColumnID::BS_BVCID_First; iLoop < BS_BVColumnID::BS_BVCID_Last; iLoop++ )
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

bool CDlgBatchSelBv::BS_SuggestSelection()
{
	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL == pclBSMessage )
	{
		return false;
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_clBatchSelBVParams.m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_clBatchSelBVParams.m_eProductSubCategory );
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_clBatchSelBVParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelBVParams.m_eProductSubCategory );

	// We block the redraw because it is too long otherwise.
	BeginWaitCursor();

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;

	// Variable uses in test unit to drop out results in a file.
	m_clBatchSelBVParams.m_vecRowParameters.clear();

	for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
	{
		BSRowParameters *pclRowParameters = &iterRows->second;

		if( NULL == pclRowParameters )
		{
			continue;
		}

		// Put this 'pclRowParameters' in the 'CBatchSelectionParameters::m_vecRowParameters' to facilitate test unit.
		m_clBatchSelBVParams.m_vecRowParameters.push_back( pclRowParameters );

		// 'CBatchSelectBvList' is created and the pointer is saved in 'pclRowParameters->m_pclBatchResults' for each row. But the pointer is
		// also saved in the 'm_clBatchSelBVParams.m_pclBatchBVList' variable to help us calling 'SelectBv' method. This is the 
		// 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods that will delete this pointer. 
		// Thus no need to do it in the 'CBatchSelBVParams' destructor.
		pclRowParameters->m_pclBatchResults = new CBatchSelectBvList();

		if( NULL == pclRowParameters->m_pclBatchResults )
		{
			continue;
		}

		m_clBatchSelBVParams.m_pclBatchBVList = (CBatchSelectBvList*)pclRowParameters->m_pclBatchResults;
		long lCurrentRow = iterRows->first;

		pclBSMessage->Clear();

		pclRowParameters->m_pclSelectedProduct = NULL;

		if( CDS_SelProd::efdFlow != m_clBatchSelBVParams.m_eFlowOrPowerDTMode )
		{
			if( pclRowParameters->m_dPower > 0.0 && pclRowParameters->m_dDT > 0.0 )
			{
				CWaterChar clWaterCharReturn = m_clBatchSelBVParams.m_WC;
				double dReturnTemperature = ( ProjectType::Cooling == m_clBatchSelBVParams.m_eApplicationType ) ? m_clBatchSelBVParams.m_WC.GetTemp() + pclRowParameters->m_dDT :
						m_clBatchSelBVParams.m_WC.GetTemp() - pclRowParameters->m_dDT;

				clWaterCharReturn.UpdateFluidData( dReturnTemperature );

				pclRowParameters->m_dFlow = m_clBatchSelBVParams.m_WC.GetTemp() * m_clBatchSelBVParams.m_WC.GetDens() * m_clBatchSelBVParams.m_WC.GetSpecifHeat();
				pclRowParameters->m_dFlow -= ( clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat() );
				pclRowParameters->m_dFlow = abs( pclRowParameters->m_dPower / pclRowParameters->m_dFlow );
			}
		}

		int iReturn = m_clBatchSelBVParams.m_pclBatchBVList->SelectBv( &m_clBatchSelBVParams, pclRowParameters->m_dFlow, pclRowParameters->m_dDp );
		
		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturn & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturn & CBatchSelectBaseList::BR_BadArguments ) ) ||
			( CBatchSelectBaseList::BR_NoPipeFound == ( iReturn & CBatchSelectBaseList::BR_NoPipeFound ) ) ||
			( CBatchSelectBaseList::BR_NotFound == ( iReturn & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			m_bAtLeastOneRowWithNoSolution = true;
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
			pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_Name, _T( "" ), true );
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_OUTPUTBOXERROR, true );
			delete pclRowParameters->m_pclBatchResults;
			pclRowParameters->m_pclBatchResults = NULL;

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBVParams.m_eProductSubCategory );
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( IDS_BATCHSELECTION_NOSOLUTION ), 
						(int)m_clBatchSelBVParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturn & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelBVParams.m_pclBatchBVList->GetBestProduct();
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
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBVParams.m_eProductSubCategory );
				int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
						(int)m_clBatchSelBVParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturn & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelBVParams.m_pclBatchBVList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelBVParams.m_pclBatchBVList->GetFirst<CSelectedBase>();

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
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBVParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
					
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelBVParams.m_eProductSubCategory );
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

				pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_Name, _T( "" ), true );
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBVParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_SEVRESULTS : IDS_BATCHSELECTION_SEVRESULTSWITHINFO;
		 			
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelBVParams.m_eProductSubCategory );
				}
			}
		}

		// Send message.
		pclBSMessage->SendMessage( lCurrentRow, true, (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_clBatchSelBVParams.m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_clBatchSelBVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	EndWaitCursor();

	return CDlgBatchSelBase::BS_SuggestSelection();
}

bool CDlgBatchSelBv::BS_AddOneColumn( int iColumnID )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_AddOneColumn( iColumnID ) )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
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
				m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
			}

			return false;
		}

		for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
		{
			long lCurrentRow = iterRows->first;
			pclBSMessage->Clear();

			_BS_FillCell( pclBSMessage, &iterRows->second, iColumnID );
			pclBSMessage->SendMessage( lCurrentRow, false, (int)m_clBatchSelBVParams.m_eProductSubCategory );
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	return true;
}

void CDlgBatchSelBv::BS_UpdateIOColumnLayout( int iUpdateWhat )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	switch( iUpdateWhat )
	{
		case BS_UpdateInputColumn::BS_UIC_Dp:

			// Update output columns in regards to the current change.
			if( true == m_clBatchSelBVParams.m_bIsDpGroupChecked )
			{
				// First hide column 'BS_BVCID_DpHalfOpening' if exist.
				m_mapColumnList[BS_BVColumnID::BS_BVCID_DpHalfOpening].m_bEnabled = false;

				if( true == m_mapColumnList[BS_BVColumnID::BS_BVCID_DpHalfOpening].m_bDisplayed )
				{
					BS_RemoveOneColumn( BS_BVColumnID::BS_BVCID_DpHalfOpening );
				}

				// Add 'BS_BVCID_Dp' and 'BS_BVCID_DpFullOpening' columns if user had previously intentionally added.
				m_mapColumnList[BS_BVColumnID::BS_BVCID_Dp].m_bEnabled = true;
				m_mapColumnList[BS_BVColumnID::BS_BVCID_DpFullOpening].m_bEnabled = true;

				if( eBool3::eb3True == m_mapColumnList[BS_BVColumnID::BS_BVCID_Dp].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BVColumnID::BS_BVCID_Dp );
				}

				if( eBool3::eb3True == m_mapColumnList[BS_BVColumnID::BS_BVCID_DpFullOpening].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BVColumnID::BS_BVCID_DpFullOpening );
				}
			}
			else
			{
				// Add columns 'BS_BVCID_Dp', 'BS_BVCID_DpFullOpening' and 'BS_BVCID_DpHalfOpening' if user had previously intentionally added.
				m_mapColumnList[BS_BVColumnID::BS_BVCID_DpHalfOpening].m_bEnabled = true;
				m_mapColumnList[BS_BVColumnID::BS_BVCID_Dp].m_bEnabled = true;
				m_mapColumnList[BS_BVColumnID::BS_BVCID_DpFullOpening].m_bEnabled = true;

				if( eBool3::eb3True == m_mapColumnList[BS_BVColumnID::BS_BVCID_Dp].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BVColumnID::BS_BVCID_Dp );
				}

				if( eBool3::eb3True == m_mapColumnList[BS_BVColumnID::BS_BVCID_DpFullOpening].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BVColumnID::BS_BVCID_DpFullOpening );
				}

				if( eBool3::eb3True == m_mapColumnList[BS_BVColumnID::BS_BVCID_DpHalfOpening].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_BVColumnID::BS_BVCID_DpHalfOpening );
				}
			}

			break;
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	CDlgBatchSelBase::BS_UpdateIOColumnLayout( iUpdateWhat );
}

// Remark: 'CW' is for 'Column Width'.
// HYS-1995: Save version. Version 2.
#define CW_BATCHSELBV_VERSION			2
void CDlgBatchSelBv::BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelBVParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}

	int iVersion = CW_BATCHSELBV_VERSION;
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = (CDS_BatchSelParameter::CCWBatchSelWindow *)pclBatchSelParameter->GetWindowColumnWidth(
			CW_WINDOWID_BATCHSELBV, true );

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

				if( iter->first >= BS_BVCID_First )
				{
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3True;
				}
			}
			else
			{
				m_clInterface.SetColumnWidthInPixel( iter->first, iter->second.m_lWidth, (int)m_clBatchSelBVParams.m_eProductSubCategory );
			}
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelBVParams.m_eProductSubCategory );
	}
}

void CDlgBatchSelBv::BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = (CDS_BatchSelParameter::CCWBatchSelWindow *)pclBatchSelParameter->GetWindowColumnWidth( CW_WINDOWID_BATCHSELBV, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = (CDS_BatchSelParameter::CCWBatchSelSheet *)pclCWWindow->GetCWSheet( 0, true );

	int iAfterColumnID = -1;
	CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfo mapColumnWidth;

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); iter++ )
	{
		CDS_BatchSelParameter::CCWBatchSelSheet::ColumnInfo rColumnInfo;
		rColumnInfo.m_bIsVisible = iter->second.m_bDisplayed;

		if( true == iter->second.m_bDisplayed )
		{
			m_clInterface.GetColumnWidthInPixel( iter->first, rColumnInfo.m_lWidth, (int)m_clBatchSelBVParams.m_eProductSubCategory );
			rColumnInfo.m_iAfterColumnID = iAfterColumnID;
			iAfterColumnID = iter->first;
		}

		mapColumnWidth[iter->first] = rColumnInfo;
	}

	pclCWSheet->GetMap() = mapColumnWidth;
	// HYS-1995: Save version.
	pclCWSheet->SetVersion( CW_BATCHSELBV_VERSION );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDlgBatchSelBv::_BS_ApplySelection()
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelBv )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == m_pclCurrentRowParameters || NULL == m_pclCurrentRowParameters->m_pclBatchResults )
	{
		return false;
	}

	CDB_TAProduct *pclTAProduct = pRViewSSelBv->GetCurrentBalancingValveSelected();

	if( NULL == pclTAProduct )
	{
		return false;
	}

	bool bFound = false;
	CBatchSelectBaseList *pclBatchList = m_pclCurrentRowParameters->m_pclBatchResults;

	for( CSelectedBase *pclSelectedValve = pclBatchList->GetFirst<CSelectedBase>(); NULL != pclSelectedValve && false == bFound;
		 pclSelectedValve = pclBatchList->GetNext<CSelectedBase>() )
	{
		CDB_RegulatingValve *pBalancingValve = dynamic_cast<CDB_RegulatingValve *>( pclSelectedValve->GetpData() );

		if( pBalancingValve == pclTAProduct )
		{
			m_pclCurrentRowParameters->m_pclSelectedProduct = pclSelectedValve;
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
		m_pclCurrentRowParameters->m_pclCDSSelSelected = _BS_CreateSSelBv( m_pclCurrentRowParameters );
	}

	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		return false;
	}

	pRViewSSelBv->FillInSelected( m_pclCurrentRowParameters->m_pclCDSSelSelected );

	// Memorize current block selection if exist.
	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelBVParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelBVParams.m_eProductSubCategory );

	// If previous status contained alternative, we reset this flag and keep only the user action.
	m_pclCurrentRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;

	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL != pclBSMessage )
	{
		pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_BATCHSELOK, true );
	}

	BS_PrepareOutput( pclBSMessage, m_pclCurrentRowParameters );

	// Send message.
	pclBSMessage->SendMessage( m_pclCurrentRowParameters->m_lRow, true, (int)m_clBatchSelBVParams.m_eProductSubCategory );

	// Restore the previous selections.
	m_clInterface.RestoreSelections( (int)m_clBatchSelBVParams.m_eProductSubCategory );

	// HYS-1168 : We hide the DockablePane in edition mode, we have remove the auto hide mode when doing Apply
	CToolsDockablePane* pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}

	// Change the tools dockable pane to full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, true );

	pRViewSSelBv->UnregisterNotificationHandler( this );

	m_pclCurrentRowParameters = NULL;
	ClearAll();

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	SetModificationMode( false );
	return true;
}

CDS_SSelBv *CDlgBatchSelBv::_BS_CreateSSelBv( BSRowParameters *pclRowParameters )
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

		CDB_RegulatingValve *pclBalancingValve = ( CDB_RegulatingValve * )( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

		if( NULL == pclBalancingValve )
		{
			return NULL;
		}

		// Create selected object and initialize it.
		CDS_SSelBv *pSelBv = NULL;
		IDPTR IDPtr = _NULL_IDPTR;
		m_clBatchSelBVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelBv ) );

		pSelBv = ( CDS_SSelBv * )( IDPtr.MP );
		pSelBv->SetProductIDPtr( pclBalancingValve->GetIDPtr() );
		pSelBv->SetOpening( pclSelectedValve->GetH() );
		pSelBv->SetQ( pclRowParameters->m_dFlow );

		if( CDS_SelProd::efdFlow == m_clBatchSelBVParams.m_eFlowOrPowerDTMode )
		{
			pSelBv->SetFlowDef( CDS_SelProd::efdFlow );
			pSelBv->SetPower( 0.0 );
			pSelBv->SetDT( 0.0 );
		}
		else
		{
			pSelBv->SetFlowDef( CDS_SelProd::efdPower );
			pSelBv->SetPower( pclRowParameters->m_dPower );
			pSelBv->SetDT( pclRowParameters->m_dDT );
		}

		pSelBv->SetDp( pclRowParameters->m_dDp );

		pSelBv->SetTypeID( pclBalancingValve->GetTypeIDPtr().ID );
		pSelBv->SetFamilyID( pclBalancingValve->GetFamilyIDPtr().ID );
		pSelBv->SetMaterialID( pclBalancingValve->GetBodyMaterialIDPtr().ID );
		pSelBv->SetConnectID( pclBalancingValve->GetConnectIDPtr().ID );
		pSelBv->SetVersionID( pclBalancingValve->GetVersionIDPtr().ID );
		pSelBv->SetPNID( pclBalancingValve->GetPNIDPtr().ID );

		pSelBv->SetPipeSeriesID( m_clBatchSelBVParams.m_strPipeSeriesID );
		pSelBv->SetPipeID( m_clBatchSelBVParams.m_strPipeID );

		// Selected pipe informations.
		pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBalancingValve->GetSizeKey(), *pSelBv->GetpSelPipe() );

		*pSelBv->GetpSelectedInfos()->GetpWCData() = m_clBatchSelBVParams.m_WC;
		pSelBv->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef1, pclRowParameters->m_strReference1 );
		pSelBv->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef2, pclRowParameters->m_strReference2 );
		pSelBv->GetpSelectedInfos()->SetQuantity( 1 );

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelBv->GetpSelectedInfos()->SetDT( m_clBatchSelBVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelBv->GetpSelectedInfos()->SetApplicationType( m_clBatchSelBVParams.m_eApplicationType );

		CTable *pTab = m_clBatchSelBVParams.m_pTADS->GetpClipboardTable();
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CLIPBOARD_TABLE' table from the datastruct.") );
		}

		if( _T('\0') == *IDPtr.ID )
		{
			pTab->Insert( IDPtr );
		}

		return pSelBv;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelBv::_BS_CreateSSelBv'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelBv::_BS_DisplaySolutions()
{
	if( NULL != pRViewSSelBv )
	{
		m_bRViewEmpty = false;
		SetModificationMode( true );

		// Remark: The 'SetModificationMode' method just above set the 'm_pclBatchSelParams->m_bEditModeRunning' to true.
		//         Thus we need to manually set it in 'm_clIndSelBVParams' for the right view to pass in edition mode.
		m_clIndSelBVParams.m_bEditModeRunning = true;

		pRViewSSelBv->Suggest( &m_clIndSelBVParams );
	}
}

void CDlgBatchSelBv::_BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct
			|| _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() || iColumnID < BS_BVColumnID::BS_BVCID_First
			|| iColumnID >= BS_BVColumnID::BS_BVCID_Last )
	{
		return;
	}

	CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );

	if( NULL == pclSelectedValve )
	{
		return;
	}

	CDB_TAProduct *pclRegulatingValve = dynamic_cast<CDB_TAProduct *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

	if( NULL == pclRegulatingValve )
	{
		return;
	}

	double dRho = m_clBatchSelBVParams.m_WC.GetDens();
	double dKinVisc = m_clBatchSelBVParams.m_WC.GetKinVisc();

	// If needed.
	int iPhysicalType = m_mapColumnList[iColumnID].m_clDataColDef.GetPhysicalType();
	int iMaxDigit = m_mapColumnList[iColumnID].m_clDataColDef.GetMaxDigit();
	int iMinDecimal = m_mapColumnList[iColumnID].m_clDataColDef.GetMinDecimal();

	CDB_ValveCharacteristic *pclValveCharacteristic = ( CDB_ValveCharacteristic * )pclRegulatingValve->GetValveCharDataPointer();

	CString str;

	switch( iColumnID )
	{
		case BS_BVColumnID::BS_BVCID_Name:
			pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_Name, pclRegulatingValve->GetName(), true );
			break;

		case BS_BVColumnID::BS_BVCID_Material:
			pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_Material, pclRegulatingValve->GetBodyMaterial(), true );
			break;

		case BS_BVColumnID::BS_BVCID_Connection:
			pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_Connection, pclRegulatingValve->GetConnect(), true );
			break;

		case BS_BVColumnID::BS_BVCID_Version:
			pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_Version, pclRegulatingValve->GetVersion(), true );
			break;

		case BS_BVColumnID::BS_BVCID_PN:
			pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_PN, pclRegulatingValve->GetPN().c_str(), true );
			break;

		case BS_BVColumnID::BS_BVCID_Size:
			pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_Size, pclRegulatingValve->GetSize(), true );
			break;

		case BS_BVColumnID::BS_BVCID_Preset:
		{
			bool bOrange = false;

			if( NULL != pclValveCharacteristic && -1.0 != pclSelectedValve->GetH() && pclSelectedValve->GetH() < pclValveCharacteristic->GetMinRecSetting() )
			{
				bOrange = true;
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BVColumnID::BS_BVCID_Preset, pclSelectedValve->GetH(), iPhysicalType, iMaxDigit,
					iMinDecimal, true );

			if( NULL != pclCellBase && true == bOrange )
			{
				pclCellBase->SetTextForegroundColor( _ORANGE );
			}
		}
		break;

		case BS_BVColumnID::BS_BVCID_DpSignal:

			if( true == pclRegulatingValve->IsKvSignalEquipped() )
			{
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BVColumnID::BS_BVCID_DpSignal, pclSelectedValve->GetDpSignal(), iPhysicalType,
						iMaxDigit, iMinDecimal, true );
				
				CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBVParams.m_pTADS->GetpTechParams();

				if( NULL != pclCellBase && NULL != pclTechnicalParameter )
				{
					if( ( pclSelectedValve->GetDpSignal() < pclTechnicalParameter->GetValvMinDp( pclRegulatingValve->GetTypeID() ) ) ||
						( pclSelectedValve->GetDpSignal() > pclTechnicalParameter->VerifyValvMaxDp( pclRegulatingValve ) ) )
					{
						pclCellBase->SetTextForegroundColor( _ORANGE );
					}
				}
			}
			else
			{
				pclBSMessage->SetCellNumberAsText( BS_BVColumnID::BS_BVCID_DpSignal, _T("-"), _U_NODIM, 0, 0, true );
			}

			break;

		case BS_BVColumnID::BS_BVCID_Dp:

			if( NULL == dynamic_cast<CDB_VenturiValve *>( pclRegulatingValve ) || true == m_clBatchSelBVParams.m_bIsDpGroupChecked )
			{
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BVColumnID::BS_BVCID_Dp, pclSelectedValve->GetDp(), iPhysicalType, iMaxDigit,
						iMinDecimal, true );
				
				CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBVParams.m_pTADS->GetpTechParams();

				if( NULL != pclCellBase && NULL != pclTechnicalParameter )
				{
					// !!! Refer to 'Select.cpp' to know the condition for the orange color.

					if( NULL != dynamic_cast<CDB_RegulatingValve *>( pclRegulatingValve ) && NULL != pclValveCharacteristic )
					{
						double dHMax = pclValveCharacteristic->GetOpeningMax();

						if( -1.0 == dHMax )
						{
							break;
						}

						double dHQuarter = 0.25 * dHMax;
						double dDpQuarterOpen;

						if( false == pclValveCharacteristic->GetValveDp( pclRowParameters->m_dFlow, &dDpQuarterOpen, dHQuarter, dRho, dKinVisc ) )
						{
							break;
						}

						double dDpFullyOpen = pclSelectedValve->GetDpFullOpen();

						if( ( true == m_clBatchSelBVParams.m_bIsDpGroupChecked && pclRowParameters->m_dDp >= dDpFullyOpen ) ||
							( false == m_clBatchSelBVParams.m_bIsDpGroupChecked && dDpQuarterOpen >= pclTechnicalParameter->GetValvMinDp()
							  && dDpFullyOpen < pclTechnicalParameter->GetValvMinDp() ) )
						{
							if( ( pclSelectedValve->GetDp() < pclTechnicalParameter->GetValvMinDp() ) ||
								( pclSelectedValve->GetDp() > pclTechnicalParameter->VerifyValvMaxDp( pclRegulatingValve ) ) )
							{
								pclCellBase->SetTextForegroundColor( _ORANGE );
							}
						}
						else
						{
							if( true == m_clBatchSelBVParams.m_bIsDpGroupChecked && pclRowParameters->m_dFlow < dDpFullyOpen )
							{
								pclCellBase->SetTextForegroundColor( _ORANGE );
							}
							else if( dDpQuarterOpen < pclTechnicalParameter->GetValvMinDp() || dDpFullyOpen >= pclTechnicalParameter->GetValvMinDp() )
							{
								pclCellBase->SetTextForegroundColor( _ORANGE );
							}
						}
					}
					else if( NULL != dynamic_cast<CDB_CommissioningSet *>( pclRegulatingValve ) )
					{
						// No more exist at now (2014/04/04).
					}
					else if( NULL != dynamic_cast<CDB_FixedOrifice *>( pclRegulatingValve ) ||
							 NULL != dynamic_cast<CDB_VenturiValve *>( pclRegulatingValve ) )
					{
						// No used in the batch selection.
						// 						if( pclSelectedValve->GetDp() < pclTechnicalParameter->GetValvMinDp() || pclSelectedValve->GetDp() > pclTechnicalParameter->GetValvMaxDp() )
						// 						{
						// 							pclCellBase->SetTextForegroundColor( _ORANGE );
						// 						}
					}
				}
			}
			else
			{
				// To match the individual selection, when it's a venturi valve and user has not input Dp, we show only 'Dp signal' and 'Dp 100%'.
				pclBSMessage->SetCellNumberAsText( BS_BVColumnID::BS_BVCID_Dp, _T("-"), _U_NODIM, 0, 0, true );
			}

			break;

		case BS_BVColumnID::BS_BVCID_DpFullOpening:

			if( NULL == dynamic_cast<CDB_FixedOrifice *>( pclRegulatingValve ) )
			{
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BVColumnID::BS_BVCID_DpFullOpening, pclSelectedValve->GetDpFullOpen(), iPhysicalType,
						iMaxDigit, iMinDecimal, true );

				CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBVParams.m_pTADS->GetpTechParams();

				if( NULL != pclCellBase && NULL != pclTechnicalParameter )
				{
					if( false == m_clBatchSelBVParams.m_bIsDpGroupChecked && pclSelectedValve->GetDpFullOpen() < pclTechnicalParameter->GetValvMinDp() )
					{
						pclCellBase->SetTextForegroundColor( _ORANGE );
					}
				}
			}
			else
			{
				// To match the individual selection, when it's a fixed orifice valve we don't show the 'Dp 100%' column.
				pclBSMessage->SetCellNumberAsText( BS_BVColumnID::BS_BVCID_DpFullOpening, _T("-"), _U_NODIM, 0, 0, true );
			}

			break;

		case BS_BVColumnID::BS_BVCID_DpHalfOpening:

			if( false == pclRegulatingValve->IsKvSignalEquipped() )
			{
				if( false == m_clBatchSelBVParams.m_bIsDpGroupChecked )
				{
					double dDpHalfOpen = pclSelectedValve->GetDpHalfOpen();
					
					DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BVColumnID::BS_BVCID_DpHalfOpening, dDpHalfOpen, iPhysicalType, iMaxDigit,
							iMinDecimal, true );
					
					CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBVParams.m_pTADS->GetpTechParams();

					if( NULL != pclCellBase && NULL != pclTechnicalParameter )
					{
						if( false == m_clBatchSelBVParams.m_bIsDpGroupChecked &&
							( dDpHalfOpen > pclTechnicalParameter->VerifyValvMaxDp( pclRegulatingValve ) || dDpHalfOpen < pclTechnicalParameter->GetValvMinDp() ) )
						{
							pclCellBase->SetTextForegroundColor( _ORANGE );
						}
					}
				}
				else
				{
					// To match the individual selection, when it's a balancing valve, we don't show the 'Dp 50%' column if user has input 'Dp'.
					pclBSMessage->SetCellNumberAsText( BS_BVColumnID::BS_BVCID_DpHalfOpening, _T("-"), _U_NODIM, 0, 0, true );
				}
			}
			else
			{
				// To match the individual selection, when it's a fixed orifice or venturi valve we don't show the 'Dp 50%' column.
				pclBSMessage->SetCellNumberAsText( BS_BVColumnID::BS_BVCID_DpHalfOpening, _T("-"), _U_NODIM, 0, 0, true );
			}

			break;

		case BS_BVColumnID::BS_BVCID_TemperatureRange:
		{
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_TemperatureRange, pclRegulatingValve->GetTempRange(), true );

			if( NULL != pclCellBase )
			{
				if( m_clBatchSelBVParams.m_WC.GetTemp() < pclRegulatingValve->GetTmin() || m_clBatchSelBVParams.m_WC.GetTemp() > pclRegulatingValve->GetTmax() )
				{
					pclCellBase->SetTextForegroundColor( _RED );
				}
			}
		}
		break;

		case BS_BVColumnID::BS_BVCID_PipeSize:
		{
			CString str = _T( "" );
			CSelectPipe selPipe( &m_clBatchSelBVParams );
			
			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclRegulatingValve->GetSizeKey(), selPipe );
			}

			pclBSMessage->SetCellText( BS_BVColumnID::BS_BVCID_PipeSize, selPipe.GetpPipe()->GetName(), true );
		}
		break;

		case BS_BVColumnID::BS_BVCID_PipeLinDp:
		{
			CSelectPipe selPipe( &m_clBatchSelBVParams );
			
			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclRegulatingValve->GetSizeKey(), selPipe );
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BVColumnID::BS_BVCID_PipeLinDp, selPipe.GetLinDp(), iPhysicalType, iMaxDigit,
					iMinDecimal, true );
			
			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBVParams.m_pTADS->GetpTechParams();

			if( NULL != pclCellBase && NULL != pclTechnicalParameter )
			{
				if( selPipe.GetLinDp() > pclTechnicalParameter->GetPipeMaxDp() || selPipe.GetLinDp() < pclTechnicalParameter->GetPipeMinDp() )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );
				}
			}
		}
		break;

		case BS_BVColumnID::BS_BVCID_PipeV:
		{
			CSelectPipe selPipe( &m_clBatchSelBVParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclRegulatingValve->GetSizeKey(), selPipe );
			}
			
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_BVColumnID::BS_BVCID_PipeV, selPipe.GetU(), iPhysicalType, iMaxDigit, iMinDecimal,
					true );
			
			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelBVParams.m_pTADS->GetpTechParams();

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

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelBVParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelBVParams.m_eProductSubCategory );
				}
			}
		}
		break;
	}
}

bool CDlgBatchSelBv::_CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters )
{
	// Here, we can have either a best result that is saved in the 'pclRowParameters->m_pclSelectedProduct' variable or we can have more that one result.
	// If we have a best result, we save it in a 'CDS_SSelBv' object and we call the 'RViewSSelBv::Suggest' method in edit mode. 
	// Otherwise, we call the 'RViewSSelBv::Suggest' method in normal mode to display all results and let the user to choose one.

	if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()
			|| NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe() )
	{
		ASSERTA_RETURN( false );
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( m_clBatchSelBVParams.m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( false );
	}

	int iDN65 = pclTableDN->GetSize( _T("DN_65") );
	
	// This method will just copy the variables that are in the 'CProductSelectionParameters' base class.
	m_clIndSelBVParams.CopyFrom( &m_clBatchSelBVParams );

	// We need to do it manually for all variables of 'm_clBatchSelBVParams' that are needed by the 'RViewSSelBv'.

	// All variables in for 'CIndividualSelectionParameters'.
	m_clIndSelBVParams.m_dFlow = pclRowParameters->m_dFlow;

	CDB_Pipe *pclBestPipe =  pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe();
	int iSize = pclBestPipe->GetSizeKey( m_clBatchSelBVParams.m_pTADB );

	if( iSize < iDN65 )
	{
		m_clIndSelBVParams.m_strComboTypeID = m_clBatchSelBVParams.m_strComboTypeBelow65ID;
		m_clIndSelBVParams.m_strComboFamilyID = m_clBatchSelBVParams.m_strComboFamilyBelow65ID;
		m_clIndSelBVParams.m_strComboMaterialID = m_clBatchSelBVParams.m_strComboMaterialBelow65ID;
		m_clIndSelBVParams.m_strComboConnectID = m_clBatchSelBVParams.m_strComboConnectBelow65ID;
		m_clIndSelBVParams.m_strComboVersionID = m_clBatchSelBVParams.m_strComboVersionBelow65ID;
		m_clIndSelBVParams.m_strComboPNID = m_clBatchSelBVParams.m_strComboPNBelow65ID;
	}
	else
	{
		m_clIndSelBVParams.m_strComboTypeID = m_clBatchSelBVParams.m_strComboTypeAbove50ID;
		m_clIndSelBVParams.m_strComboFamilyID = m_clBatchSelBVParams.m_strComboFamilyAbove50ID;
		m_clIndSelBVParams.m_strComboMaterialID = m_clBatchSelBVParams.m_strComboMaterialAbove50ID;
		m_clIndSelBVParams.m_strComboConnectID = m_clBatchSelBVParams.m_strComboConnectAbove50ID;
		m_clIndSelBVParams.m_strComboVersionID = m_clBatchSelBVParams.m_strComboVersionAbove50ID;
		m_clIndSelBVParams.m_strComboPNID = m_clBatchSelBVParams.m_strComboPNAbove50ID;
	}

	m_clIndSelBVParams.m_eFilterSelection = m_clBatchSelBVParams.m_eFilterSelection;
	
	// Create now the 'CSelectList' that 'RViewSSelBv' needs.
	if( NULL != m_clIndSelBVParams.m_pclSelectBvList )
	{
		delete m_clIndSelBVParams.m_pclSelectBvList;
		m_clIndSelBVParams.m_pclSelectBvList = NULL;
	}

	m_clIndSelBVParams.m_pclSelectBvList = new CSelectList();

	if( NULL == m_clIndSelBVParams.m_pclSelectBvList )
	{
		ASSERTA_RETURN( false );
	}
	
	// We now transfer results from the batch selection list to the individual selection list.
	pclRowParameters->m_pclBatchResults->TransferResultsToIndividualSelectList( m_clIndSelBVParams.m_pclSelectBvList );

	// Create an 'CDS_SSelBv' to allow 'RViewSSelBv' to be in edit mode with this object selected.
	// Remark: Only if we have only one result.
	if( NULL != pclRowParameters->m_pclSelectedProduct )
	{
		CDS_SSelBv *pclSSelBv = NULL;

		if( NULL == pclRowParameters->m_pclCDSSelSelected )
		{
			pclSSelBv = _BS_CreateSSelBv( pclRowParameters );
		}
		else
		{
			pclSSelBv = dynamic_cast<CDS_SSelBv *>( pclRowParameters->m_pclCDSSelSelected );
		}

		if( NULL == pclSSelBv )
		{
			return false;
		}

		pclRowParameters->m_pclCDSSelSelected = pclSSelBv;
		m_clIndSelBVParams.m_SelIDPtr = pclSSelBv->GetIDPtr();
	}

	return true;
}
