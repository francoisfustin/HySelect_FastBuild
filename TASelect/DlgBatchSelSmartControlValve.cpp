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
#include "RViewSSelSmartControlValve.h"
#include "DlgBatchSelSmartControlValve.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelSmartControlValve::CDlgBatchSelSmartControlValve( CWnd *pParent )
	: CDlgBatchSelBase( m_clBatchSelSmartControlValveParams, CDlgBatchSelSmartControlValve::IDD, pParent )
{
	m_clBatchSelSmartControlValveParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_SmartControlValve;
	m_clBatchSelSmartControlValveParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;
}

CDlgBatchSelSmartControlValve::~CDlgBatchSelSmartControlValve()
{
}

void CDlgBatchSelSmartControlValve::SaveSelectionParameters()
{
	CDlgBatchSelBase::SaveSelectionParameters();

	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelSmartControlValveParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return;
	}

	pclBatchSelParameter->SetSmartControlValveRadioFlowState( (int)m_clBatchSelSmartControlValveParams.m_eFlowOrPowerDTMode );
	pclBatchSelParameter->SetSmartControlValveMaterialBelow65ID( m_clBatchSelSmartControlValveParams.m_strComboMaterialBelow65ID );
	pclBatchSelParameter->SetSmartControlValveConnectBelow65ID( m_clBatchSelSmartControlValveParams.m_strComboConnectBelow65ID );
	pclBatchSelParameter->SetSmartControlValvePNBelow65ID( m_clBatchSelSmartControlValveParams.m_strComboPNBelow65ID );
	pclBatchSelParameter->SetSmartControlValveMaterialAbove50ID( m_clBatchSelSmartControlValveParams.m_strComboMaterialAbove50ID );
	pclBatchSelParameter->SetSmartControlValveConnectAbove50ID( m_clBatchSelSmartControlValveParams.m_strComboConnectAbove50ID );
	pclBatchSelParameter->SetSmartControlValvePNAbove50ID( m_clBatchSelSmartControlValveParams.m_strComboPNAbove50ID );

	BS_WriteAllColumnWidth( pclBatchSelParameter );
}

bool CDlgBatchSelSmartControlValve::ResetRightView()
{
	if( NULL == pRViewSSelSmartControlValve )
	{
		return false;
	}

	if( true == pRViewSSelSmartControlValve->IsEmpty() )
	{
		return true;
	}

	pRViewSSelSmartControlValve->Reset();
	return true;
}

void CDlgBatchSelSmartControlValve::ActivateLeftTabDialog()
{
	CDlgBatchSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clBatchSelSmartControlValveParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clBatchSelSmartControlValveParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

bool CDlgBatchSelSmartControlValve::OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct )
{
	if( false == CDlgBatchSelBase::OnBatchSelectionIsCatalogExist( iColumnID, lRowRelative, bCatExist, pclProduct ) )
	{
		return false;
	}

	if( iColumnID >= BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Name && iColumnID < BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PipeSize
			&& NULL != m_mapAllRowData[lRowRelative].m_pclSelectedProduct )
	{
		// Check smart control valve.
		CDB_TAProduct *pclTAProduct = (CDB_TAProduct *)( m_mapAllRowData[lRowRelative].m_pclSelectedProduct->GetProductIDPtr().MP );
		bCatExist = ( true == pclTAProduct->VerifyOneCatExist() );
		pclProduct = pclTAProduct;
	}

	return true;
}

bool CDlgBatchSelSmartControlValve::OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelSmartControlValve )
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
	if( false == m_clInterface.IsRowValidForData( lRowRelative, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory ) )
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

	pRViewSSelSmartControlValve->Invalidate();
	pRViewSSelSmartControlValve->UpdateWindow();
	pRViewSSelSmartControlValve->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );

	m_clInterface.SelectRow( lRowRelative, true, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	_BS_DisplaySolutions();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CDlgBatchSelSmartControlValve::OnInitDialog()
{
	CDlgBatchSelBase::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgBatchSelSmartControlValve::OnBnClickedSuggest()
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

void CDlgBatchSelSmartControlValve::OnBnClickedValidate()
{
	CDlgBatchSelBase::OnBnClickedValidate();

	if( true == m_clBatchSelSmartControlValveParams.m_bEditModeRunning )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
		}

		_BS_ApplySelection();

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
		}
	}
}

void CDlgBatchSelSmartControlValve::OnBnClickedCancel()
{
	// User has edited a valid selection to change it but finally cancel it.
	OnRViewSSelKeyboardEvent( VK_ESCAPE );
	ResetRightView();
}

LRESULT CDlgBatchSelSmartControlValve::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelSmartControlValveParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return -1;
	}

	// Fill the combo box.
	// Remark: Corresponding combo variables in 'm_clBatchSelSmartControlValveParams' are updated in each of this following methods.
	FillComboBoxBodyMaterial( AboveOrBelow::Below65, pclBatchSelParameter->GetSmartControlValveMaterialBelow65ID() );
	FillComboBoxConnect( AboveOrBelow::Below65, pclBatchSelParameter->GetSmartControlValveConnectBelow65ID() );
	FillComboBoxPN( AboveOrBelow::Below65, pclBatchSelParameter->GetSmartControlValvePNBelow65ID() );

	FillComboBoxBodyMaterial( AboveOrBelow::Above50, pclBatchSelParameter->GetSmartControlValveMaterialAbove50ID() );
	FillComboBoxConnect( AboveOrBelow::Above50, pclBatchSelParameter->GetSmartControlValveConnectAbove50ID() );
	FillComboBoxPN( AboveOrBelow::Above50, pclBatchSelParameter->GetSmartControlValvePNAbove50ID() );

	// Set the 'Flow' radio state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclBatchSelParameter->GetSmartControlValveRadioFlowState() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clBatchSelSmartControlValveParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgBatchSelBase::UpdateFlowOrPowerDTState();

	// This is needed only in the case of the dialog is currently active when user creates a new document (or opens a new one).
	OnAfterNewDocument();

	m_bInitialised = true;
	return 0;
}

CRViewSSelSS *CDlgBatchSelSmartControlValve::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelSmartControlValve;
}

void CDlgBatchSelSmartControlValve::ClearAll( void )
{
	if( NULL == pRViewSSelSmartControlValve )
	{
		return;
	}

	// Disable the 'Validate' button.
	if( false == m_clBatchSelSmartControlValveParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	// Clear the right sheet.
	ResetRightView();
}

void CDlgBatchSelSmartControlValve::EnableSuggestButton( bool bEnable )
{
	// Disable the 'Validate' button.
	if( false == m_clBatchSelSmartControlValveParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	CDlgSelectionBase::ClearAll();
}

void CDlgBatchSelSmartControlValve::BS_PrepareColumnList( void )
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

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_Name;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_bLineBelowHeader = true;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_Material;
	rColParameters.m_clHeaderColDef.SetWidth( 12.0 );
	rColParameters.m_clDataColDef.SetWidth( 12.0 );
	rColParameters.m_bCantRemove = false;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_Connection;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_PN;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_Size;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHVALVESIZE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_DpMin;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPMIN );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_TemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_PipeSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPESIZE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_PipeLinDp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_LINPRESSDROP );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPELINDP );
	rColParameters.m_iHeaderUnit = _U_LINPRESSDROP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_SMARTCONTROLVALVECID_PipeV;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_VELOCITY );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPEV );
	rColParameters.m_iHeaderUnit = _U_VELOCITY;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;
}

bool CDlgBatchSelSmartControlValve::BS_InitColumns()
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_InitColumns() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
		}

		return false;
	}

	BS_AddOneColumn( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Name );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelSmartControlValve::BS_Validate()
{
	try
	{
		if( false == CDlgBatchSelBase::BS_Validate() )
		{
			return false;
		}

		CTable *pTab = dynamic_cast<CTable *>( m_clBatchSelSmartControlValveParams.m_pTADS->Get( _T("SMARTCONTROLVALVE_TAB") ).MP );

		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'SMARTCONTROLVALVE_TAB' table from the datastruct.") );
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

			CDS_SSelSmartControlValve *pSSelSmartControlValve = (CDS_SSelSmartControlValve *)pclRowParameters->m_pclCDSSelSelected;

			if( NULL == pSSelSmartControlValve )
			{
				// Create selected object and initialize it.
				pSSelSmartControlValve = _BS_CreateSmartControlValve( pclRowParameters );
			}

			if( NULL == pSSelSmartControlValve )
			{
				continue;
			}

			IDPTR IDPtr = pSSelSmartControlValve->GetIDPtr();
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
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelSmartControlValve::BS_Validate'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelSmartControlValve::BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters )
	{
		return;
	}

	for( int iLoop = BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_First; iLoop < BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Last; iLoop++ )
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

bool CDlgBatchSelSmartControlValve::BS_SuggestSelection()
{
	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL == pclBSMessage )
	{
		return false;
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	// We block the redraw because it is too long otherwise.
	BeginWaitCursor();
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;

	// Variable uses in test unit to drop out results in a file.
	m_clBatchSelSmartControlValveParams.m_vecRowParameters.clear();

	for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
	{
		BSRowParameters *pclRowParameters = &iterRows->second;

		if( NULL == pclRowParameters )
		{
			continue;
		}

		// Put this 'pclRowParameters' in the 'CBatchSelectionParameters::m_vecRowParameters' to facilitate test unit.
		m_clBatchSelSmartControlValveParams.m_vecRowParameters.push_back( pclRowParameters );

		// 'CBatchSelectSmartControlValveList' is created and the pointer is saved in 'pclRowParameters->m_pclBatchResults' for each row. But the pointer is
		// also saved in the 'm_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList' variable to help us calling 'SelectSmartControlValve' method. This is the 
		// 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods that will delete this pointer. 
		// Thus no need to do it in the 'CBatchSelSmartControlValveParams' destructor.
		pclRowParameters->m_pclBatchResults = new CBatchSelectSmartControlValveList();

		if( NULL == pclRowParameters->m_pclBatchResults )
		{
			continue;
		}

		m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList = (CBatchSelectSmartControlValveList*)pclRowParameters->m_pclBatchResults;
		long lCurrentRow = iterRows->first;
		
		pclBSMessage->Clear();

		pclRowParameters->m_pclSelectedProduct = NULL;

		if( CDS_SelProd::efdFlow != m_clBatchSelSmartControlValveParams.m_eFlowOrPowerDTMode )
		{
			if( pclRowParameters->m_dPower > 0.0 && pclRowParameters->m_dDT > 0.0 )
			{
				CWaterChar clWaterCharReturn = m_clBatchSelSmartControlValveParams.m_WC;
				double dReturnTemperature = ( ProjectType::Cooling == m_clBatchSelSmartControlValveParams.m_eApplicationType ) ? m_clBatchSelSmartControlValveParams.m_WC.GetTemp() + pclRowParameters->m_dDT :
						m_clBatchSelSmartControlValveParams.m_WC.GetTemp() - pclRowParameters->m_dDT;

				clWaterCharReturn.UpdateFluidData( dReturnTemperature );

				pclRowParameters->m_dFlow = m_clBatchSelSmartControlValveParams.m_WC.GetTemp() * m_clBatchSelSmartControlValveParams.m_WC.GetDens() * m_clBatchSelSmartControlValveParams.m_WC.GetSpecifHeat();
				pclRowParameters->m_dFlow -= ( clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat() );
				pclRowParameters->m_dFlow = abs( pclRowParameters->m_dPower / pclRowParameters->m_dFlow );
			}
		}

		int iReturn = m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList->SelectSmartControlValve( &m_clBatchSelSmartControlValveParams, pclRowParameters->m_dFlow );
		
		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturn & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturn & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturn & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturn & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			m_bAtLeastOneRowWithNoSolution = true;
			iterRows->second.m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
			pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Name, _T( "" ), true );
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_OUTPUTBOXERROR, true );
			delete pclRowParameters->m_pclBatchResults;
			pclRowParameters->m_pclBatchResults = NULL;

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( IDS_BATCHSELECTION_NOSOLUTION ), 
						(int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturn & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList->GetBestProduct();
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
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
				int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
						(int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturn & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelSmartControlValveParams.m_pclBatchSmartControlValveList->GetFirst<CSelectedBase>();
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
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
					
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
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

				pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Name, _T( "" ), true );
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_SEVRESULTS : IDS_BATCHSELECTION_SEVRESULTSWITHINFO;
		 			
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
				}
			}
		}

		// Send message.
		pclBSMessage->SendMessage( lCurrentRow, true, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	EndWaitCursor();

	return CDlgBatchSelBase::BS_SuggestSelection();
}

bool CDlgBatchSelSmartControlValve::BS_AddOneColumn( int iColumnID )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_AddOneColumn( iColumnID ) )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
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
				m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
			}

			return false;
		}

		for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
		{
			long lCurrentRow = iterRows->first;
			pclBSMessage->Clear();

			_BS_FillCell( pclBSMessage, &iterRows->second, iColumnID );
			pclBSMessage->SendMessage( lCurrentRow, false, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	return true;
}

// Remark: 'CW' is for 'Column Width'.
// HYS-1995: Save version. Version 2.
#define CW_BATCHSELSMARTCONTROLVALVE_VERSION			2
void CDlgBatchSelSmartControlValve::BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}

	int iVersion = CW_BATCHSELSMARTCONTROLVALVE_VERSION;
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth( CW_WINDOWID_BATCHSMARTCONTROLVALVE, true );
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

				if( iter->first >= BS_SMARTCONTROLVALVECID_First )
				{
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3True;
				}
			}
			else
			{
				m_clInterface.SetColumnWidthInPixel( iter->first, iter->second.m_lWidth, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
			}
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	}
}

void CDlgBatchSelSmartControlValve::BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth( CW_WINDOWID_BATCHSMARTCONTROLVALVE, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = ( CDS_BatchSelParameter::CCWBatchSelSheet * )pclCWWindow->GetCWSheet( 0, true );

	int iAfterColumnID = -1;
	CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfo mapColumnWidth;

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); iter++ )
	{
		CDS_BatchSelParameter::CCWBatchSelSheet::ColumnInfo rColumnInfo;
		rColumnInfo.m_bIsVisible = iter->second.m_bDisplayed;

		if( true == iter->second.m_bDisplayed )
		{
			m_clInterface.GetColumnWidthInPixel( iter->first, rColumnInfo.m_lWidth, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
			rColumnInfo.m_iAfterColumnID = iAfterColumnID;
			iAfterColumnID = iter->first;
		}

		mapColumnWidth[iter->first] = rColumnInfo;
	}

	pclCWSheet->GetMap() = mapColumnWidth;
	// HYS-1995: Save version.
	pclCWSheet->SetVersion( CW_BATCHSELSMARTCONTROLVALVE_VERSION );
}

void CDlgBatchSelSmartControlValve::GetMaterialList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_SmartControlValve:
			if( Above50 == eAboveOrBelow )
			{
				m_clBatchSelSmartControlValveParams.m_pTADB->GetSmartControlValveBdyMatList( pclRankEx, _T(""), _T(""), CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_clBatchSelSmartControlValveParams.m_pTADB->GetSmartControlValveBdyMatList( pclRankEx, _T(""), _T(""), CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelSmartControlValve::GetConnectList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_SmartControlValve:
			if( Above50 == eAboveOrBelow )
			{
				m_clBatchSelSmartControlValveParams.m_pTADB->GetSmartControlValveConnList( pclRankEx, _T(""), _T(""), (LPCTSTR)m_clBatchSelSmartControlValveParams.m_strComboMaterialAbove50ID,
						CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_clBatchSelSmartControlValveParams.m_pTADB->GetSmartControlValveConnList( pclRankEx, _T(""), _T(""), (LPCTSTR)m_clBatchSelSmartControlValveParams.m_strComboMaterialBelow65ID,
						CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

void CDlgBatchSelSmartControlValve::GetPNList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	// Intentionally don't call the base class.

	if( NULL == pclRankEx )
	{
		ASSERT_RETURN;
	}

	switch( (ProductSubCategory)(int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory )
	{
		case ProductSubCategory::PSC_BC_SmartControlValve:
			if( Above50 == eAboveOrBelow )
			{
				m_clBatchSelSmartControlValveParams.m_pTADB->GetSmartControlValvePNList( pclRankEx, _T(""), _T(""), (LPCTSTR)m_clBatchSelSmartControlValveParams.m_strComboMaterialAbove50ID,
						(LPCTSTR)m_clBatchSelSmartControlValveParams.m_strComboConnectAbove50ID, _T(""), CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX );
			}
			else
			{
				m_clBatchSelSmartControlValveParams.m_pTADB->GetSmartControlValvePNList( pclRankEx, _T(""), _T(""), (LPCTSTR)m_clBatchSelSmartControlValveParams.m_strComboMaterialBelow65ID,
						(LPCTSTR)m_clBatchSelSmartControlValveParams.m_strComboConnectBelow65ID, _T(""), CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50 );
			}

			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDlgBatchSelSmartControlValve::_BS_ApplySelection()
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelSmartControlValve )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == m_pclCurrentRowParameters || NULL == m_pclCurrentRowParameters->m_pclBatchResults )
	{
		return false;
	}

	CDB_TAProduct *pclTAProduct = pRViewSSelSmartControlValve->GetCurrentSmartControlValveSelected();

	if( NULL == pclTAProduct )
	{
		return false;
	}

	bool bFound = false;
	CBatchSelectBaseList *pclBatchList = m_pclCurrentRowParameters->m_pclBatchResults;

	for( CSelectedBase *pclSelectedSeparator = pclBatchList->GetFirst<CSelectedBase>(); NULL != pclSelectedSeparator && false == bFound;
			pclSelectedSeparator = pclBatchList->GetNext<CSelectedBase>() )
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pclSelectedSeparator->GetpData() );

		if( pclSmartControlValve == pclTAProduct )
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
	// product for which there is only one solution (marks with the green arrow icon in the 'Status' column), we create a 'CDS_SSelSmartControlValve' in the
	// 'OnDlgOutputCellDblClicked' method. If user edits a product for which there is more than one solution (marks with yellow exclamation icon), we can't
	// create. In this case we have to do it here.
	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		m_pclCurrentRowParameters->m_pclCDSSelSelected = _BS_CreateSmartControlValve( m_pclCurrentRowParameters );
	}

	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		return false;
	}

	pRViewSSelSmartControlValve->FillInSelected( m_pclCurrentRowParameters->m_pclCDSSelSelected );

	// Memorize current block selection if exist.
	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	// If previous status contained alternative, we reset this flag and keep only the user action.
	m_pclCurrentRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;

	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL != pclBSMessage )
	{
		pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_BATCHSELOK, true );
	}

	BS_PrepareOutput( pclBSMessage, m_pclCurrentRowParameters );

	// Send message.
	pclBSMessage->SendMessage( m_pclCurrentRowParameters->m_lRow, true, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	// Restore the previous selections.
	m_clInterface.RestoreSelections( (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );

	// HYS-1168 : We hide the DockablePane in edition mode, we have remove the auto hide mode when doing Apply
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}

	// Change the tools dockable pane to full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, true );

	pRViewSSelSmartControlValve->UnregisterNotificationHandler( this );

	m_pclCurrentRowParameters = NULL;
	ClearAll();

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	SetModificationMode( false );
	return true;
}

CDS_SSelSmartControlValve *CDlgBatchSelSmartControlValve::_BS_CreateSmartControlValve( BSRowParameters *pclRowParameters )
{
	try
	{
		if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct || _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() 
				|| NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
		{
			return NULL;
		}

		CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

		if( NULL == pclSmartControlValve )
		{
			return NULL;
		}

		// Create selected object and initialize it.
		CDS_SSelSmartControlValve *pclSelSmartControlValve = NULL;
		IDPTR IDPtr = _NULL_IDPTR;
		m_clBatchSelSmartControlValveParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSmartControlValve ) );

		pclSelSmartControlValve = (CDS_SSelSmartControlValve *)( IDPtr.MP );
		pclSelSmartControlValve->SetProductIDPtr( pclSmartControlValve->GetIDPtr() );

		pclSelSmartControlValve->SetQ( pclRowParameters->m_dFlow );

		if( CDS_SelProd::efdFlow == m_clBatchSelSmartControlValveParams.m_eFlowOrPowerDTMode )
		{
			pclSelSmartControlValve->SetFlowDef( CDS_SelProd::efdFlow );
			pclSelSmartControlValve->SetPower( 0.0 );
			pclSelSmartControlValve->SetDT( 0.0 );
		}
		else
		{
			pclSelSmartControlValve->SetFlowDef( CDS_SelProd::efdPower );
			pclSelSmartControlValve->SetPower( pclRowParameters->m_dPower );
			pclSelSmartControlValve->SetDT( pclRowParameters->m_dDT );
		}

		pclSelSmartControlValve->SetDp( pclRowParameters->m_pclSelectedProduct->GetDp() );

		pclSelSmartControlValve->SetMaterialID( pclSmartControlValve->GetBodyMaterialIDPtr().ID );
		pclSelSmartControlValve->SetConnectID( pclSmartControlValve->GetConnectIDPtr().ID );
		pclSelSmartControlValve->SetPNID( pclSmartControlValve->GetPNIDPtr().ID );

		pclSelSmartControlValve->SetPipeSeriesID( m_clBatchSelSmartControlValveParams.m_strPipeSeriesID );
		pclSelSmartControlValve->SetPipeID( m_clBatchSelSmartControlValveParams.m_strPipeID );

		// Selected pipe informations.
		pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSmartControlValve->GetSizeKey(), *pclSelSmartControlValve->GetpSelPipe() );

		*pclSelSmartControlValve->GetpSelectedInfos()->GetpWCData() = m_clBatchSelSmartControlValveParams.m_WC;
		pclSelSmartControlValve->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef1, pclRowParameters->m_strReference1 );
		pclSelSmartControlValve->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef2, pclRowParameters->m_strReference2 );
		pclSelSmartControlValve->GetpSelectedInfos()->SetQuantity( 1 );

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pclSelSmartControlValve->GetpSelectedInfos()->SetDT( m_clBatchSelSmartControlValveParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pclSelSmartControlValve->GetpSelectedInfos()->SetApplicationType( m_clBatchSelSmartControlValveParams.m_eApplicationType );

		CTable *pTab = m_clBatchSelSmartControlValveParams.m_pTADS->GetpClipboardTable();
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CLIPBOARD_TABLE' table from the datastruct.") );
		}

		if( _T('\0') == *IDPtr.ID )
		{
			pTab->Insert( IDPtr );
		}

		return pclSelSmartControlValve;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelSmartControlValve::_BS_CreateSmartControlValve'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelSmartControlValve::_BS_DisplaySolutions()
{
	if( NULL != pRViewSSelSmartControlValve )
	{
		m_bRViewEmpty = false;
		SetModificationMode( true );

		// Remark: The 'SetModificationMode' method just above set the 'm_pclBatchSelParams->m_bEditModeRunning' to true.
		//         Thus we need to manually set it in 'm_clIndSelSmartControlValveParams' for the right view to pass in edition mode.
		m_clIndSelSmartControlValveParams.m_bEditModeRunning = true;

		pRViewSSelSmartControlValve->Suggest( &m_clIndSelSmartControlValveParams );
	}

	return;
}

void CDlgBatchSelSmartControlValve::_BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct
			|| _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() || iColumnID < BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_First
			|| iColumnID >= BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Last )
	{
		return;
	}

	CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

	if( NULL == pclSmartControlValve )
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
	double dRho = m_clBatchSelSmartControlValveParams.m_WC.GetDens();
	double dKinVisc = m_clBatchSelSmartControlValveParams.m_WC.GetKinVisc();

	CString str;

	switch( iColumnID )
	{
		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Name:
			pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Name, pclSmartControlValve->GetName(), true );
			break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Material:
			pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Material, pclSmartControlValve->GetBodyMaterial(), true );
			break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Connection:
			pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Connection, pclSmartControlValve->GetConnect(), true );
			break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PN:
			pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PN, pclSmartControlValve->GetPN().c_str(), true );
			break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Size:
			pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_Size, pclSmartControlValve->GetSize(), true );
			break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_DpMin:

			pclBSMessage->SetCellNumber( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_DpMin, pclRowParameters->m_pclSelectedProduct->GetDp(), iPhysicalType, iMaxDigit,
						iMinDecimal, true );
			break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_TemperatureRange:
		{
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_TemperatureRange, 
					pclSmartControlValve->GetTempRange(), true );

			if( NULL != pclCellBase )
			{
				if( m_clBatchSelSmartControlValveParams.m_WC.GetTemp() < pclSmartControlValve->GetTmin() || m_clBatchSelSmartControlValveParams.m_WC.GetTemp() > pclSmartControlValve->GetTmax() )
				{
					pclCellBase->SetTextForegroundColor( _RED );
				}
			}
		}
		break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PipeSize:
		{
			CString str = _T( "" );
			CSelectPipe selPipe( &m_clBatchSelSmartControlValveParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSmartControlValve->GetSizeKey(), selPipe );
			}

			pclBSMessage->SetCellText( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PipeSize, selPipe.GetpPipe()->GetName(), true );
		}
		break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PipeLinDp:
		{
			CSelectPipe selPipe( &m_clBatchSelSmartControlValveParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSmartControlValve->GetSizeKey(), selPipe );
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PipeLinDp, selPipe.GetLinDp(), 
					iPhysicalType, iMaxDigit, iMinDecimal, true );

			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelSmartControlValveParams.m_pTADS->GetpTechParams();

			if( NULL != pclCellBase && NULL != pclTechnicalParameter )
			{
				if( selPipe.GetLinDp() > pclTechnicalParameter->GetPipeMaxDp() || selPipe.GetLinDp() < pclTechnicalParameter->GetPipeMinDp() )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );
				}
			}
		}
		break;

		case BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PipeV:
		{
			CSelectPipe selPipe( &m_clBatchSelSmartControlValveParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclSmartControlValve->GetSizeKey(), selPipe );
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_SmartControlValveColumnID::BS_SMARTCONTROLVALVECID_PipeV, selPipe.GetU(),
					iPhysicalType, iMaxDigit, iMinDecimal, true );

			CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelSmartControlValveParams.m_pTADS->GetpTechParams();

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

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelSmartControlValveParams.m_eProductSubCategory );
				}
			}
		}
		break;
	}
}

bool CDlgBatchSelSmartControlValve::_CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters )
{
	// Here, we can have either a best result that is saved in the 'pclRowParameters->m_pclSelectedProduct' variable or we can have more that one result.
	// If we have a best result, we save it in a 'CDS_SSelSmartControlValve' object and we call the 'RViewSSelSmartControlValve::Suggest' method in edit mode. 
	// Otherwise, we call the 'RViewSSelSmartControlValve::Suggest' method in normal mode to display all results and let the user to choose one.

	if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()
			|| NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe() )
	{
		ASSERTA_RETURN( false );
	}

	// This method will just copy the variables that are in the 'CProductSelectionParameters' base class.
	m_clIndSelSmartControlValveParams.CopyFrom( &m_clBatchSelSmartControlValveParams );
	
	// We need to do it manually for all variables of 'm_clBatchSelSmartControlValveParams' that are needed by the 'RViewSSelSmartControlValve'.

	m_clIndSelSmartControlValveParams.m_eFilterSelection = m_clBatchSelSmartControlValveParams.m_eFilterSelection;

	// Create now the 'CSelectSeparatorList' that 'RViewSSelSmartControlValve' needs.
	if( NULL != m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList )
	{
		delete m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList;
		m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList = NULL;
	}

	m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList = new CSelectSmartControlValveList();

	if( NULL == m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList )
	{
		ASSERTA_RETURN( false );
	}

	// We now transfer results from the batch selection list to the individual selection list.
	pclRowParameters->m_pclBatchResults->TransferResultsToIndividualSelectList( m_clIndSelSmartControlValveParams.m_pclSelectSmartControlValveList );

	// Create an 'CDS_SSelSmartControlValve' to allow 'RViewSSelSmartControlValve' to be in edit mode with this object selected.
	// Remark: Only if we have only one result.
	if( NULL != pclRowParameters->m_pclSelectedProduct )
	{
		CDS_SSelSmartControlValve *pclSSelSmartControlValve = NULL;

		if( NULL == pclRowParameters->m_pclCDSSelSelected )
		{
			pclSSelSmartControlValve = _BS_CreateSmartControlValve( pclRowParameters );
		}
		else
		{
			pclSSelSmartControlValve = dynamic_cast<CDS_SSelSmartControlValve *>( pclRowParameters->m_pclCDSSelSelected );
		}

		if( NULL == pclSSelSmartControlValve )
		{
			return false;
		}

		pclRowParameters->m_pclCDSSelSelected = pclSSelSmartControlValve;
		m_clIndSelSmartControlValveParams.m_SelIDPtr = pclSSelSmartControlValve->GetIDPtr();
	}

	return true;
}
