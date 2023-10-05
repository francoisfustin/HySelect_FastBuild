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
#include "RViewSSelSeparator.h"
#include "DlgBatchSelSeparator.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelSeparator::CDlgBatchSelSeparator( CWnd *pParent )
	: CDlgBatchSelBase( m_clBatchSelSeparatorParams, CDlgBatchSelSeparator::IDD, pParent )
{
	m_clBatchSelSeparatorParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_Separator;
	m_clBatchSelSeparatorParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;
}

CDlgBatchSelSeparator::~CDlgBatchSelSeparator()
{
}

void CDlgBatchSelSeparator::SaveSelectionParameters()
{
	CDlgBatchSelBase::SaveSelectionParameters();

	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelSeparatorParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return;
	}

	pclBatchSelParameter->SetSeparatorFlowRadioState( (int)m_clBatchSelSeparatorParams.m_eFlowOrPowerDTMode );
	pclBatchSelParameter->SetSeparatorTypeID( m_clBatchSelSeparatorParams.m_strComboTypeID );
	pclBatchSelParameter->SetSeparatorFamilyID( m_clBatchSelSeparatorParams.m_strComboFamilyID );
	pclBatchSelParameter->SetSeparatorConnectID( m_clBatchSelSeparatorParams.m_strComboConnectID );
	pclBatchSelParameter->SetSeparatorVersionID( m_clBatchSelSeparatorParams.m_strComboVersionID );

	BS_WriteAllColumnWidth( pclBatchSelParameter );
}

bool CDlgBatchSelSeparator::ResetRightView()
{
	if( NULL == pRViewSSelSeparator )
	{
		return false;
	}

	if( true == pRViewSSelSeparator->IsEmpty() )
	{
		return true;
	}

	pRViewSSelSeparator->Reset();
	return true;
}

void CDlgBatchSelSeparator::ActivateLeftTabDialog()
{
	CDlgBatchSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clBatchSelSeparatorParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clBatchSelSeparatorParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}

	// HYS-1041: This case is not allowed for separator selection
	_VerifyFluidCharacteristics( false );
}

void CDlgBatchSelSeparator::OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg, pclDlgWaterChar );
}

bool CDlgBatchSelSeparator::OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct )
{
	if( false == CDlgBatchSelBase::OnBatchSelectionIsCatalogExist( iColumnID, lRowRelative, bCatExist, pclProduct ) )
	{
		return false;
	}

	if( iColumnID >= BS_SeparatorColumnID::BS_SEPARATORCID_Name && iColumnID < BS_SeparatorColumnID::BS_SEPARATORCID_PipeSize &&
		NULL != m_mapAllRowData[lRowRelative].m_pclSelectedProduct )
	{
		// Check separator.
		CDB_TAProduct *pclTAProduct = ( CDB_TAProduct * )( m_mapAllRowData[lRowRelative].m_pclSelectedProduct->GetProductIDPtr().MP );
		bCatExist = ( true == pclTAProduct->VerifyOneCatExist() );
		pclProduct = pclTAProduct;
	}

	return true;
}

bool CDlgBatchSelSeparator::OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelSeparator )
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
	if( false == m_clInterface.IsRowValidForData( lRowRelative, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory ) )
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

	pRViewSSelSeparator->Invalidate();
	pRViewSSelSeparator->UpdateWindow();
	pRViewSSelSeparator->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );

	m_clInterface.SelectRow( lRowRelative, true, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	_BS_DisplaySolutions();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelSeparator, CDlgBatchSelBase )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnComboSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnComboSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnComboSelChangeConnection )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnComboSelChangeVersion )
END_MESSAGE_MAP()

void CDlgBatchSelSeparator::DoDataExchange( CDataExchange *pDX )
{
	CDlgBatchSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
}

BOOL CDlgBatchSelSeparator::OnInitDialog()
{
	CDlgBatchSelBase::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_BATCHSELSEP_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELSEP_STATICFAM );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELSEP_STATICCON );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELSEP_STATICVER );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgBatchSelSeparator::OnComboSelChangeType()
{
	if( m_clBatchSelSeparatorParams.m_strComboTypeID == m_ComboType.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboType.FindCBIDPtr( m_clBatchSelSeparatorParams.m_strComboTypeID ), 0 );

		if( iPos != m_ComboType.GetCurSel() )
		{
			m_ComboType.SetCurSel( iPos );
		}

		return;
	}

	m_clBatchSelSeparatorParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;

	_FillComboBoxFamily();
	_FillComboBoxConnect();
	_FillComboBoxVersion();
	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelSeparator::OnComboSelChangeFamily()
{
	if( m_clBatchSelSeparatorParams.m_strComboFamilyID == m_ComboFamily.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboFamily.FindCBIDPtr( m_clBatchSelSeparatorParams.m_strComboFamilyID ), 0 );

		if( iPos != m_ComboFamily.GetCurSel() )
		{
			m_ComboFamily.SetCurSel( iPos );
		}

		return;
	}

	m_clBatchSelSeparatorParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;

	_FillComboBoxConnect();
	_FillComboBoxVersion();
	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelSeparator::OnComboSelChangeConnection()
{
	if( m_clBatchSelSeparatorParams.m_strComboConnectID == m_ComboConnect.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboConnect.FindCBIDPtr( m_clBatchSelSeparatorParams.m_strComboConnectID ), 0 );

		if( iPos != m_ComboConnect.GetCurSel() )
		{
			m_ComboConnect.SetCurSel( iPos );
		}

		return;
	}

	m_clBatchSelSeparatorParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;

	_FillComboBoxVersion();
	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelSeparator::OnComboSelChangeVersion()
{
	if( m_clBatchSelSeparatorParams.m_strComboVersionID == m_ComboVersion.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboVersion.FindCBIDPtr( m_clBatchSelSeparatorParams.m_strComboVersionID ), 0 );

		if( iPos != m_ComboVersion.GetCurSel() )
		{
			m_ComboVersion.SetCurSel( iPos );
		}

		return;
	}

	m_clBatchSelSeparatorParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelSeparator::OnBnClickedSuggest()
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

void CDlgBatchSelSeparator::OnBnClickedValidate()
{
	CDlgBatchSelBase::OnBnClickedValidate();

	if( true == m_clBatchSelSeparatorParams.m_bEditModeRunning )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
		}

		_BS_ApplySelection();

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
		}
	}
}

void CDlgBatchSelSeparator::OnBnClickedCancel()
{
	// User has edited a valid selection to change it but finally cancel it.
	OnRViewSSelKeyboardEvent( VK_ESCAPE );
	ResetRightView();
}

LRESULT CDlgBatchSelSeparator::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelSeparatorParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return -1;
	}

	// Fill the combo box.
	// Remark: Corresponding combo variables in 'm_clBatchSelSeparatorParams' are updated in each of this following methods.
	_FillComboBoxType( pclBatchSelParameter->GetSeparatorTypeID() );
	_FillComboBoxFamily( pclBatchSelParameter->GetSeparatorFamilyID() );
	_FillComboBoxConnect( pclBatchSelParameter->GetSeparatorConnectID() );
	_FillComboBoxVersion( pclBatchSelParameter->GetSeparatorVersionID() );

	// Set the 'Flow' radio state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclBatchSelParameter->GetSeparatorRadioFlowState() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clBatchSelSeparatorParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgBatchSelBase::UpdateFlowOrPowerDTState();

	// This is needed only in the case of the dialog is currently active when user creates a new document (or opens a new one).
	OnAfterNewDocument();

	m_bInitialised = true;
	return 0;
}

LRESULT CDlgBatchSelSeparator::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( -1 == CDlgBatchSelBase::OnWaterChange( wParam, lParam ) )
	{
		return 0;
	}

	// HYS-1106: The restrictions of other addict than water or water + glycol is only for product selection ribbon
	// HYS-1041: This case is not allowed for separator selection
	bool bShowErrorMsg = ( ID_PRODUCTSEL_CATEGORY_BASE == pMainFrame->GetRibbonBar()->GetActiveCategory()->GetContextID() && this == pDlgLeftTabSelManager->GetCurrentLeftTabDialog() );
	_VerifyFluidCharacteristics( bShowErrorMsg );

	return 0;
}

CRViewSSelSS *CDlgBatchSelSeparator::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelSeparator;
}

void CDlgBatchSelSeparator::ClearAll( void )
{
	if( NULL == pRViewSSelSeparator )
	{
		return;
	}

	// Disable the 'Validate' button.
	if( false == m_clBatchSelSeparatorParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	// Clear the right sheet.
	ResetRightView();
}

void CDlgBatchSelSeparator::EnableSuggestButton( bool bEnable )
{
	CDlgBatchSelBase::EnableSuggestButton( bEnable );

	if( true == bEnable )
	{
		_VerifyFluidCharacteristics( false );
	}
}

void CDlgBatchSelSeparator::BS_PrepareColumnList( void )
{
	CDlgBatchSelBase::BS_PrepareColumnList();

	BSColParameters rColParameters;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Prepare header and data parameters.
	rColParameters.m_clHeaderColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_MainHeaderID::BS_MHID_InputOutput, 
			BS_MainHeaderSubID::BS_MHSUBID_Output, DlgOutputHelper::CColDef::AutoResizeDisabled, DlgOutputHelper::CColDef::Visible, 
			DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionDisabled, DlgOutputHelper::CColDef::BlockSelectionDisabled,
			DlgOutputHelper::CColDef::RowSelectionDisabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );

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

	// Columns linked to the valve.
	rColParameters.m_iColumnID = BS_SEPARATORCID_Name;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetWidth( 15.0 );
	rColParameters.m_clDataColDef.SetWidth( 15.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_bLineBelowHeader = true;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_Type;
	rColParameters.m_bCantRemove = false;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_TYPE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_Material;
	rColParameters.m_clHeaderColDef.SetWidth( 12.0 );
	rColParameters.m_clDataColDef.SetWidth( 12.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_Connection;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_Version;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_VERSION );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_PN;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_Size;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHVALVESIZE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_Dp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DP );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	rColParameters.m_strHeaderUnit = _T( "" );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_TemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_PipeSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPESIZE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_PipeLinDp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_LINPRESSDROP );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPELINDP );
	rColParameters.m_iHeaderUnit = _U_LINPRESSDROP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SEPARATORCID_PipeV;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_VELOCITY );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPEV );
	rColParameters.m_iHeaderUnit = _U_VELOCITY;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;
}

bool CDlgBatchSelSeparator::BS_InitColumns()
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_InitColumns() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
		}

		return false;
	}

	BS_AddOneColumn( BS_SeparatorColumnID::BS_SEPARATORCID_Name );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelSeparator::BS_Validate()
{
	try
	{
		if( false == CDlgBatchSelBase::BS_Validate() )
		{
			return false;
		}

		CTable *pTab = dynamic_cast<CTable *>( m_clBatchSelSeparatorParams.m_pTADS->Get( _T("AIRVENTSEP_TAB") ).MP );

		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'AIRVENTSEP_TAB' table from the datastruct.") );
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

			CDS_SSelAirVentSeparator *pSSelSeparator = (CDS_SSelAirVentSeparator *)pclRowParameters->m_pclCDSSelSelected;

			if( NULL == pSSelSeparator )
			{
				// Create selected object and initialize it.
				pSSelSeparator = _BS_CreateSSelSeparator( pclRowParameters );
			}

			if( NULL == pSSelSeparator )
			{
				continue;
			}

			IDPTR IDPtr = pSSelSeparator->GetIDPtr();
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
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelSeparator::BS_Validate'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelSeparator::BS_EnableCombos( bool bEnable )
{
	m_ComboType.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_ComboType.GetCount() <= 1 )
	{
		m_ComboType.EnableWindow( FALSE );
	}

	GetDlgItem( IDC_STATICTYPE )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	m_ComboFamily.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_ComboFamily.GetCount() <= 1 )
	{
		m_ComboFamily.EnableWindow( FALSE );
	}

	GetDlgItem( IDC_STATICFAMILY )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	m_ComboConnect.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_ComboConnect.GetCount() <= 1 )
	{
		m_ComboConnect.EnableWindow( FALSE );
	}

	GetDlgItem( IDC_STATICCONNECT )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	m_ComboVersion.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( TRUE == bEnable && m_ComboVersion.GetCount() <= 1 )
	{
		m_ComboVersion.EnableWindow( FALSE );
	}

	GetDlgItem( IDC_STATICVERSION )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
}

void CDlgBatchSelSeparator::BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters )
	{
		return;
	}

	for( int iLoop = BS_SeparatorColumnID::BS_SEPARATORCID_First; iLoop < BS_SeparatorColumnID::BS_SEPARATORCID_Last; iLoop++ )
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

bool CDlgBatchSelSeparator::BS_SuggestSelection()
{
	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL == pclBSMessage )
	{
		return false;
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_clBatchSelSeparatorParams.m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	// We block the redraw because it is too long otherwise.
	BeginWaitCursor();
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;

	// Variable uses in test unit to drop out results in a file.
	m_clBatchSelSeparatorParams.m_vecRowParameters.clear();

	for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
	{
		BSRowParameters *pclRowParameters = &iterRows->second;

		if( NULL == pclRowParameters )
		{
			continue;
		}

		// Put this 'pclRowParameters' in the 'CBatchSelectionParameters::m_vecRowParameters' to facilitate test unit.
		m_clBatchSelSeparatorParams.m_vecRowParameters.push_back( pclRowParameters );

		// 'CBatchSelectSeparatorList' is created and the pointer is saved in 'pclRowParameters->m_pclBatchResults' for each row. But the pointer is
		// also saved in the 'm_clBatchSelSeparatorParams.m_pclBatchSeparatorList' variable to help us calling 'SelectSeparator' method. This is the 
		// 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods that will delete this pointer. 
		// Thus no need to do it in the 'CBatchSelSeparatorParams' destructor.
		pclRowParameters->m_pclBatchResults = new CBatchSelectSeparatorList();

		if( NULL == pclRowParameters->m_pclBatchResults )
		{
			continue;
		}

		m_clBatchSelSeparatorParams.m_pclBatchSeparatorList = (CBatchSelectSeparatorList*)pclRowParameters->m_pclBatchResults;
		long lCurrentRow = iterRows->first;
		
		pclBSMessage->Clear();

		pclRowParameters->m_pclSelectedProduct = NULL;

		if( CDS_SelProd::efdFlow != m_clBatchSelSeparatorParams.m_eFlowOrPowerDTMode )
		{
			if( pclRowParameters->m_dPower > 0.0 && pclRowParameters->m_dDT > 0.0 )
			{
				CWaterChar clWaterCharReturn = m_clBatchSelSeparatorParams.m_WC;
				double dReturnTemperature = ( ProjectType::Cooling == m_clBatchSelSeparatorParams.m_eApplicationType ) ? m_clBatchSelSeparatorParams.m_WC.GetTemp() + pclRowParameters->m_dDT :
						m_clBatchSelSeparatorParams.m_WC.GetTemp() - pclRowParameters->m_dDT;

				clWaterCharReturn.UpdateFluidData( dReturnTemperature );

				pclRowParameters->m_dFlow = m_clBatchSelSeparatorParams.m_WC.GetTemp() * m_clBatchSelSeparatorParams.m_WC.GetDens() * m_clBatchSelSeparatorParams.m_WC.GetSpecifHeat();
				pclRowParameters->m_dFlow -= ( clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat() );
				pclRowParameters->m_dFlow = abs( pclRowParameters->m_dPower / pclRowParameters->m_dFlow );
			}
		}

		int iReturn = m_clBatchSelSeparatorParams.m_pclBatchSeparatorList->SelectSeparator( &m_clBatchSelSeparatorParams, pclRowParameters->m_dFlow );
		
		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturn & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturn & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturn & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturn & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			m_bAtLeastOneRowWithNoSolution = true;
			iterRows->second.m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_Name, _T( "" ), true );
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_OUTPUTBOXERROR, true );
			delete pclRowParameters->m_pclBatchResults;
			pclRowParameters->m_pclBatchResults = NULL;

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( IDS_BATCHSELECTION_NOSOLUTION ), 
						(int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturn & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelSeparatorParams.m_pclBatchSeparatorList->GetBestProduct();
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
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
				int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
						(int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturn & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelSeparatorParams.m_pclBatchSeparatorList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelSeparatorParams.m_pclBatchSeparatorList->GetFirst<CSelectedBase>();
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
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
					
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
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

				pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_Name, _T( "" ), true );
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_SEVRESULTS : IDS_BATCHSELECTION_SEVRESULTSWITHINFO;
		 			
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
				}
			}
		}

		// Send message.
		pclBSMessage->SendMessage( lCurrentRow, true, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	EndWaitCursor();

	return CDlgBatchSelBase::BS_SuggestSelection();
}

bool CDlgBatchSelSeparator::BS_AddOneColumn( int iColumnID )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_AddOneColumn( iColumnID ) )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
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
				m_clInterface.ReleaseRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
			}

			return false;
		}

		for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
		{
			long lCurrentRow = iterRows->first;
			pclBSMessage->Clear();

			_BS_FillCell( pclBSMessage, &iterRows->second, iColumnID );
			pclBSMessage->SendMessage( lCurrentRow, false, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	return true;
}

// Remark: 'CW' is for 'Column Width'.
// HYS-1995: Save version. Version 2.
#define CW_BATCHSELSEPARATOR_VERSION			2
void CDlgBatchSelSeparator::BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}

	int iVersion = CW_BATCHSELSEPARATOR_VERSION;
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth(
				CW_WINDOWID_BATCHSELSEPARATOR, true );
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

				if( iter->first >= BS_SEPARATORCID_First )
				{
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3True;
				}
			}
			else
			{
				m_clInterface.SetColumnWidthInPixel( iter->first, iter->second.m_lWidth, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
			}
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	}
}

void CDlgBatchSelSeparator::BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth(
				CW_WINDOWID_BATCHSELSEPARATOR, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = ( CDS_BatchSelParameter::CCWBatchSelSheet * )pclCWWindow->GetCWSheet( 0, true );

	int iAfterColumnID = -1;
	CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfo mapColumnWidth;

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); iter++ )
	{
		CDS_BatchSelParameter::CCWBatchSelSheet::ColumnInfo rColumnInfo;
		rColumnInfo.m_bIsVisible = iter->second.m_bDisplayed;

		if( true == iter->second.m_bDisplayed )
		{
			m_clInterface.GetColumnWidthInPixel( iter->first, rColumnInfo.m_lWidth, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
			rColumnInfo.m_iAfterColumnID = iAfterColumnID;
			iAfterColumnID = iter->first;
		}

		mapColumnWidth[iter->first] = rColumnInfo;
	}

	pclCWSheet->GetMap() = mapColumnWidth;
	// HYS-1995: Save version.
	pclCWSheet->SetVersion( CW_BATCHSELSEPARATOR_VERSION );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDlgBatchSelSeparator::_FillComboBoxType( CString strTypeID )
{
	if( NULL == m_clBatchSelSeparatorParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx TypeList;

	m_clBatchSelSeparatorParams.m_pTADB->GetSeparatorTypeList( &TypeList, CTADatabase::FilterSelection::ForIndAndBatchSel );
	
	m_ComboType.FillInCombo( &TypeList, strTypeID );
	m_clBatchSelSeparatorParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
}

void CDlgBatchSelSeparator::_FillComboBoxFamily( CString strFamilyID )
{
	if( NULL == m_clBatchSelSeparatorParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx FamilyList;

	m_clBatchSelSeparatorParams.m_pTADB->GetSeparatorFamilyList( &FamilyList, (LPCTSTR)m_clBatchSelSeparatorParams.m_strComboTypeID,
			CTADatabase::FilterSelection::ForIndAndBatchSel );

	m_ComboFamily.FillInCombo( &FamilyList, strFamilyID );
	m_clBatchSelSeparatorParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgBatchSelSeparator::_FillComboBoxConnect( CString strConnectID )
{
	if( NULL == m_clBatchSelSeparatorParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx ConnectionList;

	m_clBatchSelSeparatorParams.m_pTADB->GetSeparatorConnList( &ConnectionList, (LPCTSTR)m_clBatchSelSeparatorParams.m_strComboTypeID,
			(LPCTSTR)m_clBatchSelSeparatorParams.m_strComboFamilyID, CTADatabase::FilterSelection::ForIndAndBatchSel );

	m_ComboConnect.FillInCombo( &ConnectionList, strConnectID );
	m_clBatchSelSeparatorParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgBatchSelSeparator::_FillComboBoxVersion( CString strVersionID )
{
	if( NULL == m_clBatchSelSeparatorParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx VersionList;

	m_clBatchSelSeparatorParams.m_pTADB->GetSeparatorVersList( &VersionList, (LPCTSTR)m_clBatchSelSeparatorParams.m_strComboTypeID,
			(LPCTSTR)m_clBatchSelSeparatorParams.m_strComboFamilyID, (LPCTSTR)m_clBatchSelSeparatorParams.m_strComboConnectID,
			CTADatabase::FilterSelection::ForIndAndBatchSel );

	m_ComboVersion.FillInCombo( &VersionList, strVersionID );
	m_clBatchSelSeparatorParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

bool CDlgBatchSelSeparator::_BS_ApplySelection()
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelSeparator )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == m_pclCurrentRowParameters || NULL == m_pclCurrentRowParameters->m_pclBatchResults )
	{
		return false;
	}

	CDB_TAProduct *pclTAProduct = pRViewSSelSeparator->GetCurrentSeparatorSelected();

	if( NULL == pclTAProduct )
	{
		return false;
	}

	bool bFound = false;
	CBatchSelectBaseList *pclBatchList = m_pclCurrentRowParameters->m_pclBatchResults;

	for( CSelectedBase *pclSelectedSeparator = pclBatchList->GetFirst<CSelectedBase>(); NULL != pclSelectedSeparator && false == bFound;
			pclSelectedSeparator = pclBatchList->GetNext<CSelectedBase>() )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( pclSelectedSeparator->GetpData() );

		if( pclSeparator == pclTAProduct )
		{
			m_pclCurrentRowParameters->m_pclSelectedProduct = pclSelectedSeparator;
			bFound = true;
		}
	}

	if( false == bFound )
	{
		return false;
	}

	// When user comes back from the right view (by selecting a product, actuator, accessories, ...) we have two possibilities. If user edits a
	// product for which there is only one solution (marks with the green arrow icon in the 'Status' column), we create a 'CDS_SSelAirVentSeparator' in the
	// 'OnDlgOutputCellDblClicked' method. If user edits a product for which there is more than one solution (marks with yellow exclamation icon), we can't
	// create. In this case we have to do it here.
	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		m_pclCurrentRowParameters->m_pclCDSSelSelected = _BS_CreateSSelSeparator( m_pclCurrentRowParameters );
	}

	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		return false;
	}

	pRViewSSelSeparator->FillInSelected( m_pclCurrentRowParameters->m_pclCDSSelSelected );

	// Memorize current block selection if exist.
	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	// If previous status contained alternative, we reset this flag and keep only the user action.
	m_pclCurrentRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;

	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL != pclBSMessage )
	{
		pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_BATCHSELOK, true );
	}

	BS_PrepareOutput( pclBSMessage, m_pclCurrentRowParameters );

	// Send message.
	pclBSMessage->SendMessage( m_pclCurrentRowParameters->m_lRow, true, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	// Restore the previous selections.
	m_clInterface.RestoreSelections( (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );

	// HYS-1168 : We hide the DockablePane in edition mode, we have remove the auto hide mode when doing Apply
	CToolsDockablePane* pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}

	// Change the tools dockable pane to full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, true );

	pRViewSSelSeparator->UnregisterNotificationHandler( this );

	m_pclCurrentRowParameters = NULL;
	ClearAll();

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	SetModificationMode( false );
	return true;
}

CDS_SSelAirVentSeparator *CDlgBatchSelSeparator::_BS_CreateSSelSeparator( BSRowParameters *pclRowParameters )
{
	try
	{
		if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct || _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() 
				|| NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
		{
			return NULL;
		}

		CDB_Separator *pclSeparator = (CDB_Separator *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

		if( NULL == pclSeparator )
		{
			return NULL;
		}

		// Create selected object and initialize it.
		CDS_SSelAirVentSeparator *pclSelSeparator = NULL;
		IDPTR IDPtr = _NULL_IDPTR;
		m_clBatchSelSeparatorParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelAirVentSeparator ) );

		pclSelSeparator = (CDS_SSelAirVentSeparator *)( IDPtr.MP );
		pclSelSeparator->SetProductIDPtr( pclSeparator->GetIDPtr() );
		pclSelSeparator->SetQ( pclRowParameters->m_dFlow );

		if( CDS_SelProd::efdFlow == m_clBatchSelSeparatorParams.m_eFlowOrPowerDTMode )
		{
			pclSelSeparator->SetFlowDef( CDS_SelProd::efdFlow );
			pclSelSeparator->SetPower( 0.0 );
			pclSelSeparator->SetDT( 0.0 );
		}
		else
		{
			pclSelSeparator->SetFlowDef( CDS_SelProd::efdPower );
			pclSelSeparator->SetPower( pclRowParameters->m_dPower );
			pclSelSeparator->SetDT( pclRowParameters->m_dDT );
		}

		pclSelSeparator->SetDp( pclRowParameters->m_pclSelectedProduct->GetDp() );

		pclSelSeparator->SetTypeID( pclSeparator->GetTypeIDPtr().ID );
		pclSelSeparator->SetFamilyID( pclSeparator->GetFamilyIDPtr().ID );
		pclSelSeparator->SetConnectID( pclSeparator->GetConnectIDPtr().ID );
		pclSelSeparator->SetVersionID( pclSeparator->GetVersionIDPtr().ID );

		pclSelSeparator->SetPipeSeriesID( m_clBatchSelSeparatorParams.m_strPipeSeriesID );
		pclSelSeparator->SetPipeID( m_clBatchSelSeparatorParams.m_strPipeID );

		// Selected pipe informations.
		pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSeparator->GetSizeKey(), *pclSelSeparator->GetpSelPipe() );

		*pclSelSeparator->GetpSelectedInfos()->GetpWCData() = m_clBatchSelSeparatorParams.m_WC;
		pclSelSeparator->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef1, pclRowParameters->m_strReference1 );
		pclSelSeparator->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef2, pclRowParameters->m_strReference2 );
		pclSelSeparator->GetpSelectedInfos()->SetQuantity( 1 );

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pclSelSeparator->GetpSelectedInfos()->SetDT( m_clBatchSelSeparatorParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pclSelSeparator->GetpSelectedInfos()->SetApplicationType( m_clBatchSelSeparatorParams.m_eApplicationType );

		CTable *pTab = m_clBatchSelSeparatorParams.m_pTADS->GetpClipboardTable();
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CLIPBOARD_TABLE' table from the datastruct.") );
		}

		if( _T('\0') == *IDPtr.ID )
		{
			pTab->Insert( IDPtr );
		}

		return pclSelSeparator;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelSeparator::_BS_CreateSSelSeparator'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelSeparator::_BS_DisplaySolutions()
{
	if( NULL != pRViewSSelSeparator )
	{
		m_bRViewEmpty = false;
		SetModificationMode( true );

		// Remark: The 'SetModificationMode' method just above set the 'm_pclBatchSelParams->m_bEditModeRunning' to true.
		//         Thus we need to manually set it in 'm_clIndSelSeparatorParams' for the right view to pass in edition mode.
		m_clIndSelSeparatorParams.m_bEditModeRunning = true;

		pRViewSSelSeparator->Suggest( &m_clIndSelSeparatorParams );
	}

	return;
}

void CDlgBatchSelSeparator::_BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct
			|| _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() || iColumnID < BS_SeparatorColumnID::BS_SEPARATORCID_First
			|| iColumnID >= BS_SeparatorColumnID::BS_SEPARATORCID_Last )
	{
		return;
	}

	CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

	if( NULL == pclSeparator )
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
	double dRho = m_clBatchSelSeparatorParams.m_WC.GetDens();
	double dKinVisc = m_clBatchSelSeparatorParams.m_WC.GetKinVisc();

	CString str;

	switch( iColumnID )
	{
		case BS_SeparatorColumnID::BS_SEPARATORCID_Name:
			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_Name, pclSeparator->GetName(), true );
			break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_Type:
			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_Type, pclSeparator->GetType(), true );
			break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_Material:
			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_Material, pclSeparator->GetBodyMaterial(), true );
			break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_Connection:
			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_Connection, pclSeparator->GetConnect(), true );
			break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_Version:
			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_Version, pclSeparator->GetVersion(), true );
			break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_PN:
			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_PN, pclSeparator->GetPN().c_str(), true );
			break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_Size:
			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_Size, pclSeparator->GetSize(), true );
			break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_Dp:
			pclBSMessage->SetCellNumber( BS_SeparatorColumnID::BS_SEPARATORCID_Dp, pclRowParameters->m_pclSelectedProduct->GetDp(), iPhysicalType, 
					iMaxDigit, iMinDecimal, true );
			break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_TemperatureRange:
		{
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_TemperatureRange, 
					pclSeparator->GetTempRange(), true );

			if( NULL != pclCellBase )
			{
				if( m_clBatchSelSeparatorParams.m_WC.GetTemp() < pclSeparator->GetTmin() || m_clBatchSelSeparatorParams.m_WC.GetTemp() > pclSeparator->GetTmax() )
				{
					pclCellBase->SetTextForegroundColor( _RED );
				}
			}
		}
		break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_PipeSize:
		{
			CString str = _T( "" );
			CSelectPipe selPipe( &m_clBatchSelSeparatorParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSeparator->GetSizeKey(), selPipe );
			}

			pclBSMessage->SetCellText( BS_SeparatorColumnID::BS_SEPARATORCID_PipeSize, selPipe.GetpPipe()->GetName(), true );
		}
		break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_PipeLinDp:
		{
			CSelectPipe selPipe( &m_clBatchSelSeparatorParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSeparator->GetSizeKey(), selPipe );
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SeparatorColumnID::BS_SEPARATORCID_PipeLinDp, selPipe.GetLinDp(), 
					iPhysicalType, iMaxDigit, iMinDecimal, true );

			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelSeparatorParams.m_pTADS->GetpTechParams();

			if( NULL != pclCellBase && NULL != pclTechnicalParameter )
			{
				if( selPipe.GetLinDp() > pclTechnicalParameter->GetPipeMaxDp() || selPipe.GetLinDp() < pclTechnicalParameter->GetPipeMinDp() )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );
				}
			}
		}
		break;

		case BS_SeparatorColumnID::BS_SEPARATORCID_PipeV:
		{
			CSelectPipe selPipe( &m_clBatchSelSeparatorParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSeparator->GetSizeKey(), selPipe );
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SeparatorColumnID::BS_SEPARATORCID_PipeV, selPipe.GetU(),
					iPhysicalType, iMaxDigit, iMinDecimal, true );

			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelSeparatorParams.m_pTADS->GetpTechParams();

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

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelSeparatorParams.m_eProductSubCategory );
				}
			}
		}
		break;
	}
}

bool CDlgBatchSelSeparator::_CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters )
{
	// Here, we can have either a best result that is saved in the 'pclRowParameters->m_pclSelectedProduct' variable or we can have more that one result.
	// If we have a best result, we save it in a 'CDS_SSelAirVentSeparator' object and we call the 'RViewSSelSeparator::Suggest' method in edit mode. 
	// Otherwise, we call the 'RViewSSelSeparator::Suggest' method in normal mode to display all results and let the user to choose one.

	if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()
			|| NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe() )
	{
		ASSERTA_RETURN( false );
	}

	// This method will just copy the variables that are in the 'CProductSelectionParameters' base class.
	m_clIndSelSeparatorParams.CopyFrom( &m_clBatchSelSeparatorParams );
	
	// We need to do it manually for all variables of 'm_clBatchSelSeparatorParams' that are needed by the 'RViewSSelSeparator'.

	m_clIndSelSeparatorParams.m_eFilterSelection = m_clBatchSelSeparatorParams.m_eFilterSelection;

	// Create now the 'CSelectSeparatorList' that 'RViewSSelSeparator' needs.
	if( NULL != m_clIndSelSeparatorParams.m_pclSelectSeparatorList )
	{
		delete m_clIndSelSeparatorParams.m_pclSelectSeparatorList;
		m_clIndSelSeparatorParams.m_pclSelectSeparatorList = NULL;
	}

	m_clIndSelSeparatorParams.m_pclSelectSeparatorList = new CSelectSeparatorList();

	if( NULL == m_clIndSelSeparatorParams.m_pclSelectSeparatorList )
	{
		ASSERTA_RETURN( false );
	}

	// We now transfer results from the batch selection list to the individual selection list.
	pclRowParameters->m_pclBatchResults->TransferResultsToIndividualSelectList( m_clIndSelSeparatorParams.m_pclSelectSeparatorList );

	// Create an 'CDS_SSelAirVentSeparator' to allow 'RViewSSelSeparator' to be in edit mode with this object selected.
	// Remark: Only if we have only one result.
	if( NULL != pclRowParameters->m_pclSelectedProduct )
	{
		CDS_SSelAirVentSeparator *pclSSelSeparator = NULL;

		if( NULL == pclRowParameters->m_pclCDSSelSelected )
		{
			pclSSelSeparator = _BS_CreateSSelSeparator( pclRowParameters );
		}
		else
		{
			pclSSelSeparator = dynamic_cast<CDS_SSelAirVentSeparator *>( pclRowParameters->m_pclCDSSelSelected );
		}

		if( NULL == pclSSelSeparator )
		{
			return false;
		}

		pclRowParameters->m_pclCDSSelSelected = pclSSelSeparator;
		m_clIndSelSeparatorParams.m_SelIDPtr = pclSSelSeparator->GetIDPtr();
	}

	return true;
}

void CDlgBatchSelSeparator::_VerifyFluidCharacteristics( bool bShowErrorMsg )
{
	bool bEnable = true;
	CString strMsg = _T("");
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg );

	if( true == bEnable )
	{
		// Do not enable suggest button if input data is empty
		long lMaxRows;
		m_clInterface.GetRows( lMaxRows, (int)m_pclBatchSelParams->m_eProductSubCategory );
			
		m_clButtonSuggest.EnableWindow( TRUE );
		m_clButtonSuggest.ModifyStyle( BS_OWNERDRAW, 0 );
		m_clButtonSuggest.SetTextColor( _BLACK );
		m_clButtonSuggest.ResetDrawBorder();
		
		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.DelTool( &m_clButtonSuggest );
		}

		if( 0 == lMaxRows )
		{
			m_clButtonSuggest.EnableWindow( FALSE );
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

void CDlgBatchSelSeparator::_VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	CString strAdditiveFamilyID = _T("");
	
	if( NULL != pclDlgWaterChar )
	{
		strAdditiveFamilyID = pclDlgWaterChar->GetAdditiveFamilyID();
	}
	else
	{
		strAdditiveFamilyID = m_clBatchSelSeparatorParams.m_WC.GetAdditFamID();
	}

	bEnable = true;
	strMsg = _T("");

	// HYS-1041: Disable suggest button when the additive is not a Glycol.
	if( 0 != StringCompare( strAdditiveFamilyID, _T("GLYCOL_ADDIT" ) ) 
			&& 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT" ) )
			&& 0 != StringCompare( strAdditiveFamilyID, _T("OTHER_ADDIT" ) ) )
	{
		strMsg = TASApp.LoadLocalizedString( AFXMSG_BAD_ADDITIVE );
		bEnable = false;
	}
}
