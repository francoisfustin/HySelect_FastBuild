#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "Select.h"

#include "DlgLeftTabSelManager.h"
#include "DlgInfoSSelDpC.h"

#include "ProductSelectionParameters.h"
#include "RViewSSelDpC.h"

#include "DlgBatchSelDpC.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelDpC::CDlgBatchSelDpC( CWnd *pParent )
	: CDlgBatchSelBase( m_clBatchSelDpCParams, CDlgBatchSelDpC::IDD, pParent )
{
	// For DpC.
	m_clBatchSelDpCParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_DpController;
	m_clBatchSelDpCParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;
	
	// For BV.
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Batch;

	m_pclDlgBatchSelDpC_DpCTab = NULL;
	m_pclDlgBatchSelDpC_BvTab = NULL;
}

CDlgBatchSelDpC::~CDlgBatchSelDpC()
{
	if( NULL != m_pclDlgBatchSelDpC_DpCTab )
	{
		if( INVALID_HANDLE_VALUE != m_pclDlgBatchSelDpC_DpCTab->GetSafeHwnd() )
		{
			m_pclDlgBatchSelDpC_DpCTab->DestroyWindow();
		}

		delete m_pclDlgBatchSelDpC_DpCTab;
		m_pclDlgBatchSelDpC_DpCTab = NULL;
	}

	if( NULL != m_pclDlgBatchSelDpC_BvTab )
	{
		if( INVALID_HANDLE_VALUE != m_pclDlgBatchSelDpC_BvTab->GetSafeHwnd() )
		{
			m_pclDlgBatchSelDpC_BvTab->DestroyWindow();
		}

		delete m_pclDlgBatchSelDpC_BvTab;
		m_pclDlgBatchSelDpC_BvTab = NULL;
	}
}

void CDlgBatchSelDpC::SetApplicationBackground( COLORREF cBackColor )
{
	CDlgBatchSelBase::SetApplicationBackground( cBackColor );

	for( int i = 0; i < m_clDpcBvCtrlTab.GetTabsNum(); i++ )
	{
		CDialogEx *pclDialogEx = dynamic_cast<CDialogEx *>( m_clDpcBvCtrlTab.GetTabWnd( i ) );

		if( NULL == pclDialogEx )
		{
			ASSERT( 0 );
			continue;
		}

		pclDialogEx->SetBackgroundColor( cBackColor );
	}
}

void CDlgBatchSelDpC::SaveSelectionParameters()
{
	CDlgBatchSelBase::SaveSelectionParameters();

	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelDpCParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return;
	}

	pclBatchSelParameter->SetDpCFlowRadioState( (int)m_clBatchSelDpCParams.m_eFlowOrPowerDTMode );
	pclBatchSelParameter->SetDpCDpBranchCheckboxState( (int)m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked );
	pclBatchSelParameter->SetDpCKvsCheckboxState( (int)m_clBatchSelDpCParams.m_bIsCheckboxKvsChecked );
	pclBatchSelParameter->SetDpCDpMaxCheckboxState( (int)m_clBatchSelDpCParams.m_bIsDpMaxChecked );
	pclBatchSelParameter->SetDpCDpStab( (int)m_clBatchSelDpCParams.m_eDpStab );
	pclBatchSelParameter->SetDpCDpCLocation( ( int )m_clBatchSelDpCParams.m_eDpCLoc );
	pclBatchSelParameter->SetDpCMvLocation( ( int )m_clBatchSelDpCParams.m_eMvLoc );

	if( NULL != m_pclDlgBatchSelDpC_DpCTab )
	{
		m_pclDlgBatchSelDpC_DpCTab->SaveSelectionParameters();
	}

	if( NULL != m_pclDlgBatchSelDpC_BvTab )
	{
		m_pclDlgBatchSelDpC_BvTab->SaveSelectionParameters();
	}
	
	BS_WriteAllColumnWidth( pclBatchSelParameter );
}

void CDlgBatchSelDpC::ActivateLeftTabDialog()
{
	CDlgBatchSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clBatchSelDpCParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clBatchSelDpCParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strPipeSeriesID = m_clBatchSelDpCParams.m_strPipeSeriesID;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strPipeID = m_clBatchSelDpCParams.m_strPipeID;
}

bool CDlgBatchSelDpC::ResetRightView()
{
	if( NULL == pRViewSSelDpC )
	{
		return false;
	}

	if( true == pRViewSSelDpC->IsEmpty() )
	{
		return true;
	}

	pRViewSSelDpC->Reset();
	return true;
}

bool CDlgBatchSelDpC::OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList )
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

	for( int iLoopColumn = BS_DpCCID_First; iLoopColumn < BS_DpCCID_Last; iLoopColumn++ )
	{
		CDlgBatchSelectionOutput::ColData rColData;

		if( BS_DpCCID_Name == iLoopColumn || BS_DpCCID_BvName == iLoopColumn )
		{
			// Insert before the Dp controller title and a separator.
			rColData.m_lColumnID = -1;

			rColData.m_strName = TASApp.LoadLocalizedString( ( BS_DpCCID_Name == iLoopColumn ) ? IDS_DLGBATCHSELDPC_CTXTMENUDPCOUTPUT :
					IDS_DLGBATCHSELDPC_CTXTMENUBVOUTPUT );
			
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
		rColData.m_strName = m_mapColumnList[iLoopColumn].m_strHeaderName;
		rColData.m_bAlreadyDisplayed = m_mapColumnList[iLoopColumn].m_bDisplayed;

		// Show 'BS_DpCCID_DpMax' item in context menu ONLY if user has checked the 'Dp max' check box.
		if( BS_DpCColumnID::BS_DpCCID_DpMax == iLoopColumn )
		{
			rColData.m_bEnable = m_mapColumnList[BS_DpCColumnID::BS_DpCCID_DpMax].m_bEnabled;
		}

		// Verify if we must display Dp at 50% or not.
		if( BS_DpCColumnID::BS_DpCCID_BvDpHalfOpening == iLoopColumn )
		{
			rColData.m_bEnable = !m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked;
		}

		rColData.m_bSeparator = false;
		mapColumnList[iInsert++] = rColData;
	}

	return true;
}

bool CDlgBatchSelDpC::OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct )
{
	if( false == CDlgBatchSelBase::OnBatchSelectionIsCatalogExist( iColumnID, lRowRelative, bCatExist, pclProduct ) )
	{
		return false;
	}

	if( iColumnID >= BS_DpCColumnID::BS_DpCCID_Name && iColumnID < BS_DpCColumnID::BS_DpCCID_PipeSize 
			&& NULL != m_mapAllRowData[lRowRelative].m_pclSelectedProduct )
	{
		// Check Dp controller.
		CDB_TAProduct *pclTAProduct = ( CDB_TAProduct * )( m_mapAllRowData[lRowRelative].m_pclSelectedProduct->GetProductIDPtr().MP );
		bCatExist = ( true == pclTAProduct->VerifyOneCatExist() );
		pclProduct = pclTAProduct;
	}
	else if( iColumnID >= BS_DpCColumnID::BS_DpCCID_BvName && iColumnID < BS_DpCColumnID::BS_DpCCID_Last 
		 && NULL != m_mapAllRowData[lRowRelative].m_pclSelectedSecondaryProduct )
	{
		// Check regulating valve.
		CDB_TAProduct *pclTAProduct = ( CDB_TAProduct * )( m_mapAllRowData[lRowRelative].m_pclSelectedSecondaryProduct->GetProductIDPtr().MP );
		bCatExist = ( true == pclTAProduct->VerifyOneCatExist() );
		pclProduct = pclTAProduct;
	}

	return true;
}

bool CDlgBatchSelDpC::OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelDpC )
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
	if( false == m_clInterface.IsRowValidForData( lRowRelative, (int)m_clBatchSelDpCParams.m_eProductSubCategory ) )
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

	pRViewSSelDpC->Invalidate();
	pRViewSSelDpC->UpdateWindow();
	pRViewSSelDpC->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );

	m_clInterface.SelectRow( lRowRelative, true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( true == _CopyBatchSelParamsToIndSelParams( m_pclCurrentRowParameters ) )
	{
		_BS_DisplaySolutions();
	}
	
	return true;
}

void CDlgBatchSelDpC::DpCBvCtrlTab_NotificationCbnChange()
{
	CheckIfWeNeedToClearResults();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelDpC, CDlgBatchSelBase )
	ON_COMMAND( IDC_CHECKDPKVS, OnBnClickedCheckDpBranchOrKvs )
	ON_COMMAND( IDC_CHECKDPMAX, OnBnClickedCheckDpMax )
	ON_EN_KILLFOCUS( IDC_EDITDPMAX, OnEnKillFocusDpMax )
	ON_CBN_SELCHANGE( IDC_COMBODPSTAB2, OnCbnSelChangeDpStab )
	ON_CBN_SELCHANGE( IDC_COMBODPCLOC2, OnCbnSelChangeDpCLoc )
	ON_CBN_SELCHANGE( IDC_COMBOMVLOC2, OnCbnSelChangeMvLoc )
END_MESSAGE_MAP()

void CDlgBatchSelDpC::DoDataExchange( CDataExchange *pDX )
{
	CDlgBatchSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKDPKVS, m_clCheckboxDpBranchOrKvs );
	DDX_Control( pDX, IDC_CHECKDPMAX, m_clCheckboxDpMax );
	DDX_Control( pDX, IDC_EDITDPMAX, m_clExtEditDpMax );
	DDX_Control( pDX, IDC_GROUPCONNSCHEME, m_clGroupDpCScheme );
	DDX_Control( pDX, IDC_COMBODPSTAB2, m_ComboDpStab );
	DDX_Control( pDX, IDC_COMBOMVLOC2, m_ComboMvLoc );
	DDX_Control( pDX, IDC_COMBODPCLOC2, m_ComboDpCLoc );
}

BOOL CDlgBatchSelDpC::OnInitDialog()
{
	CDlgBatchSelBase::OnInitDialog();

	// Reset pointers on 'Database' and 'TADataStruct' to those ones by default also for the BV selection.
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pTADB = m_clBatchSelDpCParams.m_pTADB;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pTADS = m_clBatchSelDpCParams.m_pTADS;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_bEditModeRunning = m_clBatchSelDpCParams.m_bEditModeRunning;

	CString str;
	str = TASApp.LoadLocalizedString( IDS_BATCHSELDPC_DPBRANCH );
	m_clCheckboxDpBranchOrKvs.SetWindowText( str );
	m_clCheckboxDpBranchOrKvs.SetCheck( BST_UNCHECKED );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELDPC_DPMAX );
	m_clCheckboxDpMax.SetWindowText( str );
	m_clCheckboxDpMax.SetCheck( BST_UNCHECKED );

	m_clExtEditDpMax.SetPhysicalType( _U_DIFFPRESS );
	m_clExtEditDpMax.SetEditType( CNumString::eDouble, CNumString::ePositive );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELDPC_STATICDPSTAB );
	GetDlgItem( IDC_STATICDPSTAB )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_BATCHSELDPC_STATICDPCLOC );
	GetDlgItem( IDC_STATICDPCLOC )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_BATCHSELDPC_STATICMVLOC );
	GetDlgItem( IDC_STATICMVLOC )->SetWindowText( str );

	// Set proper style and add icons for the DpC scheme group.
	m_clGroupDpCScheme.SetInOffice2007Mainframe( true );
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupDpCScheme.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_DpC );
	}

	m_clGroupDpCScheme.SetInOffice2007Mainframe( true );
	m_clGroupDpCScheme.SaveChildList();

	// Create the DpC-Bv tab control with the good size.
	CRect rectCtrlTabContainer;
	GetDlgItem( IDC_DPCBVCTRLTAB )->GetClientRect( rectCtrlTabContainer );
	CRect rectCtrlTab = rectCtrlTabContainer;
	rectCtrlTab.DeflateRect( 1, 1 );

	if( FALSE == m_clDpcBvCtrlTab.Create( CMFCTabCtrl::STYLE_3D, rectCtrlTab, this, 1, CMFCTabCtrl::LOCATION_TOP, FALSE ) )
	{
		return FALSE;
	}

	// Position the ctrl tab.
	GetDlgItem( IDC_DPCBVCTRLTAB )->GetWindowRect( rectCtrlTabContainer );
	ScreenToClient( &rectCtrlTabContainer );
	m_clDpcBvCtrlTab.SetWindowPos( NULL, rectCtrlTabContainer.left + 1, rectCtrlTabContainer.top + 7, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Some properties.
	m_clDpcBvCtrlTab.SetDrawFrame( FALSE );
	m_clDpcBvCtrlTab.SetTabBorderSize( 0 );
	m_clDpcBvCtrlTab.SetTabBkColor( TabIndex::etiDpC, _RED );
	m_clDpcBvCtrlTab.SetActiveTabBoldFont( TRUE );

	// 'CDlgBatchSelcDpC_DpcTab' creation.
	m_pclDlgBatchSelDpC_DpCTab = new CDlgBatchSelDpC_DpCTab( m_clBatchSelDpCParams, this );

	if( NULL == m_pclDlgBatchSelDpC_DpCTab )
	{
		return FALSE;
	}

	if( FALSE == m_pclDlgBatchSelDpC_DpCTab->Create( CDlgBatchSelDpC_DpCTab::IDD, &m_clDpcBvCtrlTab ) )
	{
		delete m_pclDlgBatchSelDpC_DpCTab;
		m_pclDlgBatchSelDpC_DpCTab = NULL;
		return FALSE;
	}

	str = TASApp.LoadLocalizedString( IDS_BATCHSELDPC_DPCTABTITLE );
	m_clDpcBvCtrlTab.AddTab( m_pclDlgBatchSelDpC_DpCTab, str, -1, FALSE );

	// 'CDlgBatchSelDpC_BvTab' creation.
	m_pclDlgBatchSelDpC_BvTab = new CDlgBatchSelDpC_BvTab( m_clBatchSelDpCParams, this );

	if( NULL == m_pclDlgBatchSelDpC_BvTab )
	{
		m_pclDlgBatchSelDpC_DpCTab->DestroyWindow();
		delete m_pclDlgBatchSelDpC_DpCTab;
		m_pclDlgBatchSelDpC_DpCTab = NULL;
		return FALSE;
	}

	if( FALSE == m_pclDlgBatchSelDpC_BvTab->Create( CDlgBatchSelDpC_BvTab::IDD, &m_clDpcBvCtrlTab ) )
	{
		m_pclDlgBatchSelDpC_DpCTab->DestroyWindow();
		delete m_pclDlgBatchSelDpC_DpCTab;
		m_pclDlgBatchSelDpC_DpCTab = NULL;
		delete m_pclDlgBatchSelDpC_BvTab;
		m_pclDlgBatchSelDpC_BvTab = NULL;
		return FALSE;
	}

	str = TASApp.LoadLocalizedString( IDS_BATCHSELDPC_BVTABTITLE );
	m_clDpcBvCtrlTab.AddTab( m_pclDlgBatchSelDpC_BvTab, str, -1, FALSE );

	m_clDpcBvCtrlTab.SetActiveTab( etiDpC );

	// Add a tooltip to max dp in the valve.
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
	 	CString TTstr = TASApp.LoadLocalizedString( IDS_MAXDPINVALVE );
 		m_ToolTip.AddToolWindow( &m_clExtEditDpMax, TTstr );
	}

	return TRUE;
}

void CDlgBatchSelDpC::OnBnClickedCheckDpBranchOrKvs()
{
	if( eDpStab::DpStabOnBranch == m_clBatchSelDpCParams.m_eDpStab )
	{
		_UpdateDpBranchCheckboxState();
		_UpdateDpBranchColumn();
	}
	else
	{
		_UpdateKvsCheckboxState();
		_UpdateKvsColumn();
	}
}

void CDlgBatchSelDpC::OnBnClickedCheckDpMax()
{
	_UpdateDpMaxFieldState();
}

void CDlgBatchSelDpC::OnCbnSelChangeDpStab()
{
	if( m_clBatchSelDpCParams.m_eDpStab == (eDpStab)m_ComboDpStab.GetCurSel() )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// If user doesn't finally change the combo...
	if( false == BS_ComboChange() )
	{
		m_ComboDpStab.SetCurSel( ( int )m_clBatchSelDpCParams.m_eDpStab );
		return;
	}

	_UpdateDpStabilizedOn();

	// HYS-1188: We split 'm_iDpCCheckDpKvs' in two variables.
	if( eDpStab::DpStabOnBranch == m_clBatchSelDpCParams.m_eDpStab )
	{
		// We need to set the checkbox before to call the '_UpdateCheckDpBranch'.
		// Remark: there is only one check box for the two options (Dpl and Kvs).
		m_clCheckboxDpBranchOrKvs.SetCheck( ( true == m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked ) ? BST_CHECKED : BST_UNCHECKED );
		_UpdateDpBranchCheckboxState();
		_UpdateDpBranchColumn();
	}
	else
	{
		// We need to set the checkbox before to call the '_UpdateCheckKvs'.
		// Remark: there is only one check box for the two options (Dpl and Kvs).
		m_clCheckboxDpBranchOrKvs.SetCheck( ( true == m_clBatchSelDpCParams.m_bIsCheckboxKvsChecked ) ? BST_CHECKED : BST_UNCHECKED );
		_UpdateKvsCheckboxState();
		_UpdateKvsColumn();
	}

	if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
	{
		OnBatchSelectionButtonClearResults();
		OnBnClickedSuggest();
	}
	else
	{
		ClearAll();
	}
}

void CDlgBatchSelDpC::OnCbnSelChangeDpCLoc()
{
	if( m_clBatchSelDpCParams.m_eDpCLoc == (eDpCLoc)m_ComboDpCLoc.GetCurSel() )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// If user doesn't finally change the combo...
	if( false == BS_ComboChange() )
	{
		m_ComboDpCLoc.SetCurSel( ( int )m_clBatchSelDpCParams.m_eDpCLoc );
		return;
	}

	_UpdateDpCLocation();
	m_pclDlgBatchSelDpC_DpCTab->UpdateCombos();
	m_pclDlgBatchSelDpC_BvTab->UpdateCombos();
	
	if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
	{
		OnBatchSelectionButtonClearResults();
		OnBnClickedSuggest();
	}
	else
	{
		ClearAll();
	}
}

void CDlgBatchSelDpC::OnCbnSelChangeMvLoc()
{
	if( m_clBatchSelDpCParams.m_eMvLoc == (eMvLoc)m_ComboMvLoc.GetCurSel() )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// If user doesn't finally change the combo...
	if( false == BS_ComboChange() )
	{
		m_ComboMvLoc.SetCurSel( ( int )m_clBatchSelDpCParams.m_eMvLoc );
		return;
	}

	_UpdateMvLocation();
	
	if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
	{
		OnBatchSelectionButtonClearResults();
		OnBnClickedSuggest();
	}
	else
	{
		ClearAll();
	}
}

void CDlgBatchSelDpC::OnEnChangeDpMax()
{
	if( GetFocus() == &m_clExtEditDpMax )
	{
		ClearAll();
	}
}

void CDlgBatchSelDpC::OnEnKillFocusDpMax()
{
	m_clBatchSelDpCParams.m_dDpMax = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpMax, &m_clBatchSelDpCParams.m_dDpMax ) 
			|| m_clBatchSelDpCParams.m_dDpMax < 0.0 )
	{
		m_clBatchSelDpCParams.m_dDpMax = 0.0;
	}
}

void CDlgBatchSelDpC::OnBnClickedSuggest()
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

void CDlgBatchSelDpC::OnBnClickedValidate()
{
	CDlgBatchSelBase::OnBnClickedValidate();

	if( true == m_clBatchSelDpCParams.m_bEditModeRunning )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
		}

		_BS_ApplySelection();

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
		}
	}
}

void CDlgBatchSelDpC::OnBnClickedCancel()
{
	// User has edited a valid selection to change it but finally cancel it.
	OnRViewSSelKeyboardEvent( VK_ESCAPE );
	ResetRightView();
}

LRESULT CDlgBatchSelDpC::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelBase::OnNewDocument( wParam, lParam );

	// Reset pointers on 'Database' and 'TADataStruct' to those ones by default also for BV selection.
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pTADB = m_clBatchSelDpCParams.m_pTADB;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pTADS = m_clBatchSelDpCParams.m_pTADS;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pUserDB = m_clBatchSelDpCParams.m_pUserDB;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_pPipeDB = m_clBatchSelDpCParams.m_pPipeDB;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_WC = m_clBatchSelDpCParams.m_WC;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strPipeSeriesID = m_clBatchSelDpCParams.m_strPipeSeriesID;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strPipeID = m_clBatchSelDpCParams.m_strPipeID;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_bEditModeRunning = m_clBatchSelDpCParams.m_bEditModeRunning;

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_BatchSelParameter *pclBatchSelParameter = m_clBatchSelDpCParams.m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameter )
	{
		return -1;
	}

	// Set the 'Flow' radio state.
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclBatchSelParameter->GetDpCRadioFlowState() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clBatchSelDpCParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	UpdateFlowOrPowerDTState();

	// Fill the combo boxes.
	// Remark: Corresponding combo variables in 'm_clBatchSelDpCParams' are updated in each of this following methods.
	_FillComboDpStab( (eDpStab)pclBatchSelParameter->GetDpCDpStab() );
	_FillComboDpCLoc( (eDpCLoc)pclBatchSelParameter->GetDpCMvLocation() );
	_FillComboMvLoc( (eMvLoc)pclBatchSelParameter->GetDpCMvLocation() );

	m_mapColumnList[BS_DpCColumnID::BS_DpCCID_DpMax].m_bEnabled = m_clBatchSelDpCParams.m_bIsDpMaxChecked;

	// To check if we need to change 'm_clCheckBoxDpKvs' text ("Dp branch" or "Kvs control valve").
	_UpdateDpStabilizedOn();
	_UpdateDpCLocation();
	_UpdateMvLocation();

	m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked = ( 0 == pclBatchSelParameter->GetDpCDpBranchCheckboxState() ) ? BST_UNCHECKED : BST_CHECKED;
	m_clBatchSelDpCParams.m_bIsCheckboxKvsChecked = ( 0 == pclBatchSelParameter->GetDpCKvsCheckboxState() ) ? BST_UNCHECKED : BST_CHECKED;

	// HYS-1188: We split 'm_iDpCCheckDpKvs' in two variables.
	if( eDpStab::DpStabOnBranch == pclBatchSelParameter->GetDpCDpStab() )
	{
		// Set the Dp branch group check.
		m_clCheckboxDpBranchOrKvs.SetCheck( ( 0 == pclBatchSelParameter->GetDpCDpBranchCheckboxState() ) ? BST_UNCHECKED : BST_CHECKED );
		_UpdateDpBranchCheckboxState();
	}
	else if( eDpStab::DpStabOnCV == pclBatchSelParameter->GetDpCDpStab() )
	{
		// Set the Kvs group check.
		m_clCheckboxDpBranchOrKvs.SetCheck( ( 0 == pclBatchSelParameter->GetDpCKvsCheckboxState() ) ? BST_UNCHECKED : BST_CHECKED );
		_UpdateKvsCheckboxState();
	}

	// Set the Dp max check.
	// Remark: 'm_clBatchSelDpCParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_clCheckboxDpMax.SetCheck( ( 0 == pclBatchSelParameter->GetDpCDpMaxCheckboxState() ) ? BST_UNCHECKED : BST_CHECKED );
	_UpdateDpMaxFieldState();

	OnUnitChange();

	if( NULL != m_pclDlgBatchSelDpC_DpCTab )
	{
		m_pclDlgBatchSelDpC_DpCTab->OnNewDocument( wParam, lParam );
	}

	if( NULL != m_pclDlgBatchSelDpC_BvTab )
	{
		m_pclDlgBatchSelDpC_BvTab->OnNewDocument( wParam, lParam );
	}

	// This is needed only in the case of the dialog is currently active when user creates a new document (or opens a new one).
	OnAfterNewDocument();

	m_bInitialised = true;
	return 0;
}

LRESULT CDlgBatchSelDpC::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelBase::OnPipeChange( wParam, lParam );

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strPipeSeriesID = m_clBatchSelDpCParams.m_strPipeSeriesID;
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_strPipeID = m_clBatchSelDpCParams.m_strPipeID;

	return 0;
}

LRESULT CDlgBatchSelDpC::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgBatchSelBase::OnUnitChange( wParam, lParam );

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATICDPMAXUNIT, name );

	if( m_clBatchSelDpCParams.m_dDpMax > 0.0 )
	{
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_clBatchSelDpCParams.m_dDpMax ) );
	}

	return 0;
}

LRESULT CDlgBatchSelDpC::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( -1 == CDlgBatchSelBase::OnWaterChange( wParam, lParam ) )
	{
		return 0;
	}

	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_WC = m_clBatchSelDpCParams.m_WC;

	return 0;
}

void CDlgBatchSelDpC::BS_PrepareColumnList( void )
{
	CDlgBatchSelBase::BS_PrepareColumnList();

	BSColParameters rColParameters;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Prepare header and data parameters for output.
	rColParameters.m_clHeaderColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_DPCMainHeaderID::BS_DPCMHID_DpCBv,
			BS_DPCMainHeaderSubID::BS_DPCMHSUBID_DpController, DlgOutputHelper::CColDef::AutoResizeDisabled, 
			DlgOutputHelper::CColDef::Visible, DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionDisabled,
			DlgOutputHelper::CColDef::BlockSelectionDisabled, DlgOutputHelper::CColDef::RowSelectionDisabled, 
			DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );

	rColParameters.m_clHeaderColDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );

	rColParameters.m_clHeaderFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Center, 
			DlgOutputHelper::CFontDef::VA_Center, DlgOutputHelper::CFontDef::BoldDisabled, DlgOutputHelper::CFontDef::ItalicDisabled, 
			DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_clDataColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_DPCMainHeaderID::BS_DPCMHID_DpCBv,
			BS_DPCMainHeaderSubID::BS_DPCMHSUBID_DpController, DlgOutputHelper::CColDef::AutoResizeDisabled, DlgOutputHelper::CColDef::Visible, 
			DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionEnabled, DlgOutputHelper::CColDef::BlockSelectionEnabled,
			DlgOutputHelper::CColDef::RowSelectionEnabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataEnabled );

	rColParameters.m_clDataColDef.SetRowSeparatorFlag( true, DLGBATCHSELBASE_COLOR_DATAROWSEPARATOR );

	rColParameters.m_clDataFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Left, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled, DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, 
			_WHITE, _BLACK );

	rColParameters.m_iColumnID = BS_DpCCID_Name;
	rColParameters.m_clHeaderColDef.SetWidth( 20.0 );
	rColParameters.m_clDataColDef.SetWidth( 20.0 );
	rColParameters.m_bCantRemove = true;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_bLineBelowHeader = true;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_Material;
	rColParameters.m_clHeaderColDef.SetWidth( 12.0 );
	rColParameters.m_clDataColDef.SetWidth( 12.0 );
	rColParameters.m_bCantRemove = false;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_Connection;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_Version;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_VERSION );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_PN;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_Size;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHVALVESIZE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_DpMin;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetMaxDigit( 3 );
	rColParameters.m_clDataColDef.SetMinDecimal( 0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SHEETHDR_DPMIN );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	rColParameters.m_strHeaderUnit = _T( "" );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_DpRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SHEETHDR_DPLRANGE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_DpMax;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetMaxDigit( 3 );
	rColParameters.m_clDataColDef.SetMinDecimal( 0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_RVIEWSSELDPC_DPMAX );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_TemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_PipeSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPESIZE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_PipeLinDp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_LINPRESSDROP );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPELINDP );
	rColParameters.m_iHeaderUnit = _U_LINPRESSDROP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_PipeV;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_VELOCITY );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPEV );
	rColParameters.m_iHeaderUnit = _U_VELOCITY;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	// Columns linked to the regulating valve.
	rColParameters.m_iColumnID = BS_DpCCID_BvName;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_clHeaderColDef.SetMainHeaderLinks( BS_DPCMainHeaderID::BS_DPCMHID_DpCBv, BS_DPCMainHeaderSubID::BS_DPCMHSUBID_RegulatingValve );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_NAME );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvIn;
	rColParameters.m_bCantRemove = true;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_BVIN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvMaterial;
	rColParameters.m_clHeaderColDef.SetWidth( 12.0 );
	rColParameters.m_clDataColDef.SetWidth( 12.0 );
	rColParameters.m_bCantRemove = false;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvConnection;
	rColParameters.m_clHeaderColDef.SetWidth( 10.0 );
	rColParameters.m_clDataColDef.SetWidth( 10.0 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvVersion;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_VERSION );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvPN;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PN );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHVALVESIZE );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvPreset;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetMaxDigit( 2 );
	rColParameters.m_clDataColDef.SetMinDecimal( 2 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PRESET );
	rColParameters.m_strHeaderUnit = TASApp.LoadLocalizedString( IDS_SHEETHDR_TURNSPOS );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvDpSignal;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_DIFFPRESS );
	rColParameters.m_clDataColDef.SetMaxDigit( -1 );
	rColParameters.m_clDataColDef.SetMinDecimal( -1 );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SHEETHDR_SIGNAL );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	rColParameters.m_strHeaderUnit = _T( "" );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvDp;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DP );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvDpFullOpening;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPFO );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvDpHalfOpening;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_DPHO );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvTemperatureRange;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_NODIM );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Text );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_COLTEMPRANGE );
	rColParameters.m_iHeaderUnit = _U_TEMPERATURE;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvPipeSize;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPESIZE );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvPipeLinDp;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::Number );
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_LINPRESSDROP );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPELINDP );
	rColParameters.m_iHeaderUnit = _U_LINPRESSDROP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_DpCCID_BvPipeV;
	rColParameters.m_clDataColDef.SetPhysicalType( ePHYSTYPE::_U_VELOCITY );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_CHPIPEV );
	rColParameters.m_iHeaderUnit = _U_VELOCITY;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;
}

bool CDlgBatchSelDpC::BS_InitMainHeaders( void )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_InitMainHeaders() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
		}

		return false;
	}

	// Add main header for 'Dp controller'.
	DlgOutputHelper::CColDef clColMHInDef;
	clColMHInDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHInDef.SetMouseEventFlag( true );
	clColMHInDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );
	
	// To specify that the group 'Dp controller' is linked to the main header 'Output'.
	clColMHInDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output );
	m_clInterface.AddMainHeaderDef( BS_DPCMainHeaderID::BS_DPCMHID_DpCBv, BS_DPCMainHeaderSubID::BS_DPCMHSUBID_DpController,
			clColMHInDef, TASApp.LoadLocalizedString( IDS_DLGBATCHSELDPC_TITLEGROUPDPC ), (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHInDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center, 
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _IMI_GRAY_MED, _WHITE );

	m_clInterface.AddMainHeaderFontDef( BS_DPCMainHeaderID::BS_DPCMHID_DpCBv, BS_DPCMainHeaderSubID::BS_DPCMHSUBID_DpController,
			clFontMHInDef, (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	// Add main header for 'Regulating valve'.
	DlgOutputHelper::CColDef clColMHOutDef;
	clColMHOutDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHOutDef.SetMouseEventFlag( true );
	clColMHOutDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );
	
	// To specify that the group 'Regulating valve' is linked to the main header 'Output'.
	clColMHOutDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output );
	
	m_clInterface.AddMainHeaderDef( BS_DPCMainHeaderID::BS_DPCMHID_DpCBv, BS_DPCMainHeaderSubID::BS_DPCMHSUBID_RegulatingValve,
			clColMHOutDef, TASApp.LoadLocalizedString( IDS_DLGBATCHSELDPC_TITLEGROUPBV ), (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHOutDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center, 
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _IMI_GRAY_MED, _WHITE );
	
	m_clInterface.AddMainHeaderFontDef( BS_DPCMainHeaderID::BS_DPCMHID_DpCBv, BS_DPCMainHeaderSubID::BS_DPCMHSUBID_RegulatingValve,
			clFontMHOutDef, (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	m_clInterface.SetMainHeaderRowHeight( BS_DPCMainHeaderID::BS_DPCMHID_DpCBv, DLBBATCHSELDPC_ROWHEIGHT_MAINHEADER,
			(int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelDpC::BS_InitColumns()
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_InitColumns() )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
		}

		return false;
	}

	BS_AddOneColumn( BS_DpCColumnID::BS_DpCCID_Name );
	BS_AddOneColumn( BS_DpCColumnID::BS_DpCCID_BvName );
	BS_AddOneColumn( BS_DpCColumnID::BS_DpCCID_BvIn );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelDpC::BS_Validate()
{
	try
	{
		if( false == CDlgBatchSelBase::BS_Validate() )
		{
			return false;
		}

		CTable *pTab = dynamic_cast<CTable *>( m_clBatchSelDpCParams.m_pTADS->Get( _T("DPCONTR_TAB") ).MP );

		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'DPCONTR_TAB' table from the datastruct.") );
		}

		bool bAtLeastOneInserted = false;

		for( mapLongRowParamsIter iterRow = m_mapAllRowData.begin(); iterRow != m_mapAllRowData.end(); iterRow++ )
		{
			BSRowParameters *pclRowParameters = &iterRow->second;

			if( RowStatus::BS_RS_FindOneSolution != pclRowParameters->m_eStatus && RowStatus::BS_RS_FindOneSolutionAlter != pclRowParameters->m_eStatus )
			{
				continue;
			}

			CDS_SSelDpC *pSSelDpC = ( CDS_SSelDpC * )pclRowParameters->m_pclCDSSelSelected;

			if( NULL == pSSelDpC )
			{
				// Create selected object and initialize it.
				pSSelDpC = _BS_CreateSSelDpC( pclRowParameters );
			}

			if( NULL == pSSelDpC )
			{
				continue;
			}

			IDPTR IDPtr = pSSelDpC->GetIDPtr();
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
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelDpC::BS_Validate'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelDpC::BS_EnableRadios( bool bEnable )
{
	CDlgBatchSelBase::BS_EnableRadios( bEnable );

	m_clCheckboxDpBranchOrKvs.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
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

	// We manage the 'Dp stabilized on' combo here because we need to enable/disable it in exactly the same
	// way as the radios.
	m_ComboDpStab.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
}

void CDlgBatchSelDpC::BS_EnableCombos( bool bEnable )
{
	CDlgBatchSelBase::BS_EnableCombos( bEnable );

	m_clGroupDpCScheme.EnableWindow( bEnable, false );
	
	m_ComboDpCLoc.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	m_ComboMvLoc.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( NULL != m_pclDlgBatchSelDpC_DpCTab )
	{
		m_pclDlgBatchSelDpC_DpCTab->EnableCombos( bEnable );
	}

	if( NULL != m_pclDlgBatchSelDpC_BvTab )
	{
		m_pclDlgBatchSelDpC_BvTab->EnableCombos( bEnable );
	}
}

void CDlgBatchSelDpC::BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters )
	{
		return;
	}

	for( int iLoop = BS_DpCColumnID::BS_DpCCID_First; iLoop < BS_DpCColumnID::BS_DpCCID_Last; iLoop++ )
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

bool CDlgBatchSelDpC::BS_SuggestSelection()
{
	if( NULL == m_pclDlgBatchSelDpC_DpCTab || NULL == m_pclDlgBatchSelDpC_BvTab )
	{
		return false;
	}

	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL == pclBSMessage )
	{
		return false;
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_clBatchSelDpCParams.m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	// We block the redraw because it is too long otherwise.
	BeginWaitCursor();
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;

	// Variable uses in test unit to drop out results in a file.
	m_clBatchSelDpCParams.m_vecRowParameters.clear();

	for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
	{
		BSRowParameters *pclRowParameters = &iterRows->second;

		if( NULL == pclRowParameters )
		{
			continue;
		}

		// Put this 'pclRowParameters' in the 'CBatchSelectionParameters::m_vecRowParameters' to facilitate test unit.
		m_clBatchSelDpCParams.m_vecRowParameters.push_back( pclRowParameters );

		// 'CBatchSelectDpCList' is created and the pointer is saved in 'pclRowParameters->m_pclBatchResults' for each row. But the pointer is
		// also saved in the 'm_clBatchSelDpCParams.m_pclBatchDpCList' variable to help us calling 'SelectDpC' method. This is the 
		// 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods that will delete this pointer. 
		// Thus no need to do it in the 'CBatchSelDpCParams' destructor.
		pclRowParameters->m_pclBatchResults = new CBatchSelectDpCList();

		if( NULL == pclRowParameters->m_pclBatchResults )
		{
			continue;
		}

		m_clBatchSelDpCParams.m_pclBatchDpCList = (CBatchSelectDpCList*)pclRowParameters->m_pclBatchResults;
		long lCurrentRow = iterRows->first;

		pclRowParameters->m_pclSelectedProduct = NULL;
		pclRowParameters->m_pclSelectedSecondaryProduct = NULL;

		if( CDS_SelProd::efdFlow != m_clBatchSelDpCParams.m_eFlowOrPowerDTMode )
		{
			if( pclRowParameters->m_dPower > 0.0 && pclRowParameters->m_dDT > 0.0 )
			{
				CWaterChar clWaterCharReturn = m_clBatchSelDpCParams.m_WC;
				double dReturnTemperature = ( ProjectType::Cooling == m_clBatchSelDpCParams.m_eApplicationType ) ? m_clBatchSelDpCParams.m_WC.GetTemp() + pclRowParameters->m_dDT :
						m_clBatchSelDpCParams.m_WC.GetTemp() - pclRowParameters->m_dDT;

				clWaterCharReturn.UpdateFluidData( dReturnTemperature );

				pclRowParameters->m_dFlow = m_clBatchSelDpCParams.m_WC.GetTemp() * m_clBatchSelDpCParams.m_WC.GetDens() * m_clBatchSelDpCParams.m_WC.GetSpecifHeat();
				pclRowParameters->m_dFlow -= ( clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat() );
				pclRowParameters->m_dFlow = abs( pclRowParameters->m_dPower / pclRowParameters->m_dFlow );
			}
		}

		int iReturn = m_clBatchSelDpCParams.m_pclBatchDpCList->SelectDpC( &m_clBatchSelDpCParams, pclRowParameters->m_dFlow, 
				pclRowParameters->m_dDpBranch, pclRowParameters->m_dKvs );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturn & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturn & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturn & CBatchSelectBaseList::BR_NoPipeFound ) )
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturn & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			m_bAtLeastOneRowWithNoSolution = true;
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
			pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_Name, _T( "" ), true );
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_OUTPUTBOXERROR, true );
			delete pclRowParameters->m_pclBatchResults;
			pclRowParameters->m_pclBatchResults = NULL;

			if( NULL != pclCellBase )
			{
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( IDS_BATCHSELECTION_NOSOLUTION ),
						(int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturn & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetBestProduct();

			if( NULL != m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected() )
			{
				pclRowParameters->m_pclSelectedSecondaryProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected()->GetBestProduct();
			}

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
				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;
				
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
						(int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturn & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == m_clBatchSelDpCParams.m_pclBatchDpCList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetFirst<CSelectedBase>();

				if( NULL != m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected() )
				{
					pclRowParameters->m_pclSelectedSecondaryProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected()->GetBestProduct();
				}

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
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_ONERESULT : IDS_BATCHSELECTION_ONERESULTINFO;

					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelDpCParams.m_eProductSubCategory );
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

				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_Name, _T( "" ), true );
				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, iBitmapID, true );

				if( NULL != pclCellBase )
				{
					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					int iTextID = ( false == bAlternative ) ? IDS_BATCHSELECTION_SEVRESULTS : IDS_BATCHSELECTION_SEVRESULTSWITHINFO;
					
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, lCurrentRow, TASApp.LoadLocalizedString( iTextID ), 
							(int)m_clBatchSelDpCParams.m_eProductSubCategory );
				}
			}
		}

		// Send message.
		pclBSMessage->SendMessage( lCurrentRow, true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
		pclBSMessage->Clear();
	}

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	EndWaitCursor();

	return CDlgBatchSelBase::BS_SuggestSelection();
}

bool CDlgBatchSelDpC::BS_AddOneColumn( int iColumnID )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	if( false == CDlgBatchSelBase::BS_AddOneColumn( iColumnID ) )
	{
		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
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
				m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}

			return false;
		}

		for( mapLongRowParamsIter iterRows = m_mapAllRowData.begin(); iterRows != m_mapAllRowData.end(); iterRows++ )
		{
			long lCurrentRow = iterRows->first;
			pclBSMessage->Clear();

			_BS_FillCell( pclBSMessage, &iterRows->second, iColumnID );
			pclBSMessage->SendMessage( lCurrentRow, false, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	return true;
}

void CDlgBatchSelDpC::BS_UpdateInputColumnLayout( int iUpdateWhat )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	switch( iUpdateWhat )
	{
		case BS_DpCUpdateInputColumn::BS_DpCUIC_DpBranch:
			if( true == m_mapColumnList[BS_ColumnID::BS_CID_Input_DpBranch].m_bEnabled )
			{
				// Input column.
				BS_AddOneColumn( BS_ColumnID::BS_CID_Input_DpBranch );

				// Output column.
				if( true == m_mapColumnList[BS_DpCColumnID::BS_DpCCID_BvDpFullOpening].m_bDisplayed )
				{
					BS_RemoveOneColumn( BS_DpCColumnID::BS_DpCCID_BvDpFullOpening );
				}

				if( true == m_mapColumnList[BS_DpCColumnID::BS_DpCCID_BvDpHalfOpening].m_bDisplayed )
				{
					BS_RemoveOneColumn( BS_DpCColumnID::BS_DpCCID_BvDpHalfOpening );
				}
			}
			else
			{
				// Input column.
				BS_RemoveOneColumn( BS_ColumnID::BS_CID_Input_DpBranch );

				// Output column.
				if( eBool3::eb3True == m_mapColumnList[BS_DpCColumnID::BS_DpCCID_BvDpFullOpening].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_DpCColumnID::BS_DpCCID_BvDpFullOpening );
				}

				if( eBool3::eb3True == m_mapColumnList[BS_DpCColumnID::BS_DpCCID_BvDpHalfOpening].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_DpCColumnID::BS_DpCCID_BvDpHalfOpening );
				}
			}

			break;

		case BS_DpCUpdateInputColumn::BS_DpCUIC_Kvs:
			if( true == m_mapColumnList[BS_ColumnID::BS_CID_Input_Kvs].m_bEnabled )
			{
				BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Kvs );
			}
			else
			{
				BS_RemoveOneColumn( BS_ColumnID::BS_CID_Input_Kvs );
			}

			break;

		case BS_DpCUpdateInputColumn::BS_DpCUIC_DpMax:
			if( true == m_mapColumnList[BS_DpCColumnID::BS_DpCCID_DpMax].m_bEnabled )
			{
				// Output column.
				if( eBool3::eb3True == m_mapColumnList[BS_DpCColumnID::BS_DpCCID_DpMax].m_eb3UserChoice )
				{
					BS_AddOneColumn( BS_DpCColumnID::BS_DpCCID_DpMax );
				}
			}
			else
			{
				// Output column.
				if( true == m_mapColumnList[BS_DpCColumnID::BS_DpCCID_DpMax].m_bDisplayed )
				{
					BS_RemoveOneColumn( BS_DpCColumnID::BS_DpCCID_DpMax );
				}
			}

			break;
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	CDlgBatchSelBase::BS_UpdateIOColumnLayout( iUpdateWhat );
}

// Remark: 'CW' is for 'Column Width'.
// HYS-1995: Save version. Version 2.
#define CW_BATCHSELDPC_VERSION			2
void CDlgBatchSelDpC::BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}

	int iVersion = CW_BATCHSELDPC_VERSION;
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth(
				CW_WINDOWID_BATCHSELDPC, true );
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

				if( iter->first >= BS_DpCCID_First )
				{
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3True;
				}
			}
			else
			{
				m_clInterface.SetColumnWidthInPixel( iter->first, iter->second.m_lWidth, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	}
}

void CDlgBatchSelDpC::BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter )
{
	CDS_BatchSelParameter::CCWBatchSelWindow *pclCWWindow = ( CDS_BatchSelParameter::CCWBatchSelWindow * )pclBatchSelParameter->GetWindowColumnWidth(
				CW_WINDOWID_BATCHSELDPC, true );
	CDS_BatchSelParameter::CCWBatchSelSheet *pclCWSheet = ( CDS_BatchSelParameter::CCWBatchSelSheet * )pclCWWindow->GetCWSheet( 0, true );

	int iAfterColumnID = -1;
	CDS_BatchSelParameter::CCWBatchSelSheet::mapShortColumnInfo mapColumnWidth;

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); iter++ )
	{
		CDS_BatchSelParameter::CCWBatchSelSheet::ColumnInfo rColumnInfo;
		rColumnInfo.m_bIsVisible = iter->second.m_bDisplayed;

		if( true == iter->second.m_bDisplayed )
		{
			m_clInterface.GetColumnWidthInPixel( iter->first, rColumnInfo.m_lWidth, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
			rColumnInfo.m_iAfterColumnID = iAfterColumnID;
			iAfterColumnID = iter->first;
		}

		mapColumnWidth[iter->first] = rColumnInfo;
	}

	pclCWSheet->GetMap() = mapColumnWidth;
	// HYS-1995: Save version.
	pclCWSheet->SetVersion( CW_BATCHSELDPC_VERSION );
}

CRViewSSelSS *CDlgBatchSelDpC::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelDpC;
}

void CDlgBatchSelDpC::UpdateFlowOrPowerDTState()
{
	CDlgBatchSelBase::UpdateFlowOrPowerDTState();

	// Update also for BV selection.
	m_clBatchSelDpCParams.m_clBatchSelBVParams.m_eFlowOrPowerDTMode = m_clBatchSelDpCParams.m_eFlowOrPowerDTMode;
}

void CDlgBatchSelDpC::ClearAll( void )
{
	// Disable the 'Validate' button.
	if( false == m_clBatchSelDpCParams.m_bEditModeRunning )
	{
		m_clButtonValidate.EnableWindow( FALSE );
	}

	// Clear the right sheet.
	// Remark: here the right sheet is the sheet displayed when user to change one result of the batch selection.
	CDlgSelectionBase::ClearAll();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgBatchSelDpC::_FillComboDpStab( eDpStab eDpStabilizationOn )
{
	if( NULL == m_clBatchSelDpCParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx DpStabList;
	m_clBatchSelDpCParams.m_pTADB->GetDpCDpStabList( &DpStabList );
	
	// Intentionally do not use 'CExtNumEditComboBox::FillInCombo' method!!!
	m_ComboDpStab.ResetContent();
	DpStabList.Transfer( &m_ComboDpStab );

	m_ComboDpStab.SetCurSel( (int)eDpStabilizationOn );
	m_clBatchSelDpCParams.m_eDpStab = (eDpStab)m_ComboDpStab.GetCurSel();
}

void CDlgBatchSelDpC::_FillComboDpCLoc( eDpCLoc eDpCLocation )
{
	if( NULL == m_clBatchSelDpCParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx DpCLocList;
	m_clBatchSelDpCParams.m_pTADB->GetDpCLocList( &DpCLocList, m_clBatchSelDpCParams.m_eFilterSelection );

	// Intentionally do not use 'CExtNumEditComboBox::FillInCombo' method!!!
	m_ComboDpCLoc.ResetContent();
	DpCLocList.Transfer( &m_ComboDpCLoc );
	
	m_ComboDpCLoc.SetCurSel( (int)eDpCLocation );
	m_clBatchSelDpCParams.m_eDpCLoc = (eDpCLoc)m_ComboDpCLoc.GetCurSel();
}

void CDlgBatchSelDpC::_FillComboMvLoc( eMvLoc eMvLocation )
{
	if( NULL == m_clBatchSelDpCParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx MvLocList;
	m_clBatchSelDpCParams.m_pTADB->GetDpCMvLocList( &MvLocList );

	// Intentionally do not use 'CExtNumEditComboBox::FillInCombo' method!!!
	m_ComboMvLoc.ResetContent();
	MvLocList.Transfer( &m_ComboMvLoc );
	
	m_ComboMvLoc.SetCurSel( (int)eMvLocation );
	m_clBatchSelDpCParams.m_eMvLoc = (eMvLoc)m_ComboMvLoc.GetCurSel();
}

bool CDlgBatchSelDpC::_BS_ApplySelection()
{
	if( NULL == pDlgLeftTabSelManager || NULL == pRViewSSelDpC )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == m_pclCurrentRowParameters || NULL == m_pclCurrentRowParameters->m_pclBatchResults )
	{
		return false;
	}

	CDB_TAProduct *pclTAProductDpC = pRViewSSelDpC->GetCurrentDpControllerSelected();

	if( NULL == pclTAProductDpC )
	{
		return false;
	}

	// Check Dp controller valve.
	bool bFound = false;
	CBatchSelectDpCList *pclBatchDpCList = dynamic_cast<CBatchSelectDpCList *>( m_pclCurrentRowParameters->m_pclBatchResults );

	if( NULL == pclBatchDpCList )
	{
		return false;
	}

	for( CSelectedBase *pclSelectedDpC = pclBatchDpCList->GetFirst<CSelectedBase>(); NULL != pclSelectedDpC && false == bFound;
			pclSelectedDpC = pclBatchDpCList->GetNext<CSelectedBase>() )
	{
		CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclSelectedDpC->GetpData() );

		if( pclDpController == pclTAProductDpC )
		{
			m_pclCurrentRowParameters->m_pclSelectedProduct = pclSelectedDpC;
			bFound = true;
		}
	}

	if( false == bFound )
	{
		return false;
	}

	// Check regulating valve.
	CBatchSelectBvList *pclBatchBvList = pclBatchDpCList->GetBvSelected();
	CDB_TAProduct *pclTAProductBv = pRViewSSelDpC->GetCurrentBalancingValveSelected();

	if( NULL != pclBatchBvList && NULL != pclTAProductBv )
	{
		for( CSelectedBase *pclSelectedBv = pclBatchBvList->GetFirst<CSelectedBase>(); NULL != pclSelectedBv;
			 pclSelectedBv = pclBatchBvList->GetNext<CSelectedBase>() )
		{
			CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( pclSelectedBv->GetpData() );

			if( pclRegulatingValve == pclTAProductBv )
			{
				m_pclCurrentRowParameters->m_pclSelectedSecondaryProduct = pclSelectedBv;
				bFound = true;
			}
		}
	}

	// When user comes back from the right view (by selecting a product, actuator, accessories, ...) we have two possibilities. If user edits a
	// product for which there is only one solution (marks with the green arrow icon in the 'Status' column), we create a 'CDS_SSelProd' in the
	// 'OnDlgOutputCellDblClicked' method. If user edits a product for which there is more than one solution (marks with yellow exclamation icon), we can't
	// create. In this case we have to do it here.
	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		m_pclCurrentRowParameters->m_pclCDSSelSelected = _BS_CreateSSelDpC( m_pclCurrentRowParameters );
	}

	if( NULL == m_pclCurrentRowParameters->m_pclCDSSelSelected )
	{
		return false;
	}

	pRViewSSelDpC->FillInSelected( m_pclCurrentRowParameters->m_pclCDSSelSelected );

	// Memorize current block selection if exist.
	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	// If previous status contained alternative, we reset this flag and keep only the user action.
	m_pclCurrentRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;

	CDlgBatchSelectionOutput::CBSMessage *pclBSMessage = m_clInterface.GetMessage();

	if( NULL != pclBSMessage )
	{
		pclBSMessage->SetCellBitmap( BS_ColumnID::BS_CID_Input_Status, IDI_BATCHSELOK, true );
	}

	BS_PrepareOutput( pclBSMessage, m_pclCurrentRowParameters );

	// Send message.
	pclBSMessage->SendMessage( m_pclCurrentRowParameters->m_lRow, true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	// Restore the previous selections.
	m_clInterface.RestoreSelections( (int)m_clBatchSelDpCParams.m_eProductSubCategory );

	// HYS-1168 : We hide the DockablePane in edition mode, we have remove the auto hide mode when doing Apply.
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}

	// Change the tools dockable pane to full screen mode.
	pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, true );

	pRViewSSelDpC->UnregisterNotificationHandler( this );

	m_pclCurrentRowParameters = NULL;
	ClearAll();

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	SetModificationMode( false );
	return true;
}

CDS_SSelDpC *CDlgBatchSelDpC::_BS_CreateSSelDpC( BSRowParameters *pclRowParameters )
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

		CDB_DpController *pclDpController = ( CDB_DpController * )( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

		if( NULL == pclDpController )
		{
			return NULL;
		}

		// Create selected object and initialize it.
		CDS_SSelDpC *pSelDpC = NULL;
		IDPTR IDPtr = _NULL_IDPTR;
		m_clBatchSelDpCParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelDpC ) );

		pSelDpC = ( CDS_SSelDpC * )( IDPtr.MP );
		pSelDpC->SetDpCIDPtr( pclDpController->GetIDPtr() );

		// Set DpStab (stabilization on branch or CV).
		pSelDpC->SetDpStab( m_clBatchSelDpCParams.m_eDpStab );

		// Set the DpC location (supply/return).
		pSelDpC->SetDpCLoc( m_clBatchSelDpCParams.m_eDpCLoc );

		// Set balancing valve location (primary or secondary).
		pSelDpC->SetMvLoc( m_clBatchSelDpCParams.m_eMvLoc );

		// Set DpL or Kv (depending on DpStab).
		if( eDpStab::DpStabOnBranch == m_clBatchSelDpCParams.m_eDpStab )
		{
			pSelDpC->SetDpL( pclRowParameters->m_dDpBranch );
		}
		else
		{
			pSelDpC->SetKv( pclRowParameters->m_dKvs );
		}

		pSelDpC->SetDpMax( m_clBatchSelDpCParams.m_dDpMax );
		pSelDpC->SetQ( pclRowParameters->m_dFlow );

		if( CDS_SelProd::efdFlow == m_clBatchSelDpCParams.m_eFlowOrPowerDTMode )
		{
			pSelDpC->SetFlowDef( CDS_SelProd::efdFlow );
			pSelDpC->SetPower( 0.0 );
			pSelDpC->SetDT( 0.0 );
		}
		else
		{
			pSelDpC->SetFlowDef( CDS_SelProd::efdPower );
			pSelDpC->SetPower( pclRowParameters->m_dPower );
			pSelDpC->SetDT( pclRowParameters->m_dDT );
		}

		pSelDpC->ResetDpCMvPackageIDPtr();

		pSelDpC->SetTypeID( pclDpController->GetTypeIDPtr().ID );
		pSelDpC->SetFamilyID( pclDpController->GetFamilyIDPtr().ID );
		pSelDpC->SetMaterialID( pclDpController->GetBodyMaterialIDPtr().ID );
		pSelDpC->SetConnectID( pclDpController->GetConnectIDPtr().ID );
		pSelDpC->SetVersionID( pclDpController->GetVersionIDPtr().ID );
		pSelDpC->SetPNID( pclDpController->GetPNIDPtr().ID );

		pSelDpC->SetPipeSeriesID( m_clBatchSelDpCParams.m_strPipeSeriesID );
		pSelDpC->SetPipeID( m_clBatchSelDpCParams.m_strPipeID );

		// Selected pipe informations.
		pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclDpController->GetSizeKey(), *pSelDpC->GetpSelPipe() );

		*pSelDpC->GetpSelectedInfos()->GetpWCData() = m_clBatchSelDpCParams.m_WC;
		pSelDpC->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef1, pclRowParameters->m_strReference1 );
		pSelDpC->GetpSelectedInfos()->SetReference( CSelectedInfos::eRef2, pclRowParameters->m_strReference2 );
		pSelDpC->GetpSelectedInfos()->SetQuantity( 1 );

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelDpC->GetpSelectedInfos()->SetDT( m_clBatchSelDpCParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelDpC->GetpSelectedInfos()->SetApplicationType( m_clBatchSelDpCParams.m_eApplicationType );

		// Add information for regulating valve.
		pSelDpC->SetProductIDPtr( ( NULL != pclRowParameters->m_pclSelectedSecondaryProduct ) ? pclRowParameters->m_pclSelectedSecondaryProduct->GetProductIDPtr() :
				_NULL_IDPTR );

		if( _NULL_IDPTR != pclRowParameters->m_pclSelectedSecondaryProduct->GetProductIDPtr() )
		{
			CDB_Product *pclSelectedBV = dynamic_cast<CDB_Product *>( pclRowParameters->m_pclSelectedSecondaryProduct->GetProductIDPtr().MP );
			
			if( NULL == pclSelectedBV )
			{
				HYSELECT_THROW( _T("Internal error: '%s' is not a 'CDB_Product' object."), pclRowParameters->m_pclSelectedSecondaryProduct->GetProductIDPtr().ID );
			}

			for( CSelectedValve *pclSelectedProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected()->GetFirst<CSelectedValve>(); NULL != pclSelectedProduct; 
					pclSelectedProduct = m_clBatchSelDpCParams.m_pclBatchDpCList->GetBvSelected()->GetNext<CSelectedValve>() )
			{
				CDB_Product *pTAP = dynamic_cast<CDB_Product*>( pclSelectedProduct->GetpData() );

				if( pTAP == pclSelectedBV )
				{
					pSelDpC->SetOpening( pclSelectedProduct->GetH() );
					break;
				}
			}
		}

		CTable *pTab = m_clBatchSelDpCParams.m_pTADS->GetpClipboardTable();
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CLIPBOARD_TABLE' table from the datastruct.") );
		}

		if( _T('\0') == *IDPtr.ID )
		{
			pTab->Insert( IDPtr );
		}
		
		return pSelDpC;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgBatchSelDpC::_BS_CreateSSelDpC'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgBatchSelDpC::_BS_DisplaySolutions()
{
	if( NULL != pRViewSSelDpC )
	{
		m_bRViewEmpty = false;
		SetModificationMode( true );

		// Remark: The 'SetModificationMode' method just above set the 'm_pclBatchSelParams->m_bEditModeRunning' to true.
		//         Thus we need to manually set it in 'm_clIndSelDpCParams' for the right view to pass in edition mode.
		m_clIndSelDpCParams.m_bEditModeRunning = true;

		pRViewSSelDpC->Suggest( &m_clIndSelDpCParams );
	}

	return;
}

void CDlgBatchSelDpC::_BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters || NULL == pclRowParameters->m_pclSelectedProduct
			|| _NULL_IDPTR == pclRowParameters->m_pclSelectedProduct->GetProductIDPtr() || iColumnID < BS_DpCColumnID::BS_DpCCID_First
			|| iColumnID >= BS_DpCColumnID::BS_DpCCID_Last || 0 == ( int )m_mapColumnList.count( iColumnID ) )
	{
		return;
	}

	BSColParameters *pclColParameters = &m_mapColumnList[iColumnID];

	if( false == pclColParameters->m_bEnabled || false == pclColParameters->m_bDisplayed )
	{
		return;
	}

	CSelectedValve *pclSelectedDpC = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct );

	if( NULL == pclSelectedDpC )
	{
		return;
	}

	CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );

	if( NULL == pclDpController )
	{
		return;
	}

	CDS_TechnicalParameter *pclTechnicalParameter = m_clBatchSelDpCParams.m_pTADS->GetpTechParams();

	if( NULL == pclTechnicalParameter )
	{
		return;
	}

	double dRho = m_clBatchSelDpCParams.m_WC.GetDens();
	double dKinVisc = m_clBatchSelDpCParams.m_WC.GetKinVisc();

	// If needed.
	int iPhysicalType = pclColParameters->m_clDataColDef.GetPhysicalType();
	int iMaxDigit = pclColParameters->m_clDataColDef.GetMaxDigit();
	int iMinDecimal = pclColParameters->m_clDataColDef.GetMinDecimal();

	CSelectedValve *pclSelectedBv = dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedSecondaryProduct );
	CDB_TAProduct *pclBvTAProduct = NULL;
	CDB_ValveCharacteristic *pclBvValveCharacteristic = NULL;

	if( NULL != pclSelectedBv )
	{
		pclBvTAProduct = ( CDB_TAProduct * )( pclSelectedBv->GetProductIDPtr().MP );
		pclBvValveCharacteristic = ( CDB_ValveCharacteristic * )pclBvTAProduct->GetValveCharDataPointer();
	}

	CString str;

	switch( iColumnID )
	{
		case BS_DpCColumnID::BS_DpCCID_Name:
			pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_Name, pclDpController->GetName(), true );
			break;

		case BS_DpCColumnID::BS_DpCCID_Material:
			pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_Material, pclDpController->GetBodyMaterial(), true );
			break;

		case BS_DpCColumnID::BS_DpCCID_Connection:
			pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_Connection, pclDpController->GetConnect(), true );
			break;

		case BS_DpCColumnID::BS_DpCCID_Version:
			pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_Version, pclDpController->GetVersion(), true );
			break;

		case BS_DpCColumnID::BS_DpCCID_PN:
			pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_PN, pclDpController->GetPN().c_str(), true );
			break;

		case BS_DpCColumnID::BS_DpCCID_Size:
			pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_Size, pclDpController->GetSize(), true );
			break;

		case BS_DpCColumnID::BS_DpCCID_DpMin:
		{
			bool bOrange = false;

			if( pclSelectedDpC->GetDpMin() < pclTechnicalParameter->GetDpCMinDp() )
			{
				bOrange = true;
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_DpMin, pclSelectedDpC->GetDpMin(), iPhysicalType, 
					iMaxDigit, iMinDecimal, true );

			if( NULL != pclCellBase && true == bOrange )
			{
				pclCellBase->SetTextForegroundColor( _ORANGE );

				CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedDpC->GetDpMin() );
				str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechnicalParameter->GetDpCMinDp(), true );
				FormatString( str, IDS_SSHEETSSELDPC_DPMINERROR, str2 );

				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}
		}
		break;

		case BS_DpCColumnID::BS_DpCCID_DpRange:
		{
			bool bRed = false;

			if( ( true == pclSelectedDpC->IsFlagSet( CSelectedBase::eValveDpToLarge ) || true == pclSelectedDpC->IsFlagSet( CSelectedBase::eValveDpToSmall ) ) )
			{
				bRed = true;
			}

			str = pclDpController->GetFormatedDplRange( false ).c_str();

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_DpRange, str, true );

			if( NULL != pclCellBase && true == bRed )
			{
				pclCellBase->SetTextForegroundColor( _RED );

				// Remark: in case of balancing valve in secondary, DpC must stabilized both Dp branch + Dp through BV (Dpl). In this case,
				//         we size and preset the balancing valve to be sure to have the needed Dpl corresponding at least to the minimum of
				//         the DpC Dpl range. Thus we will never have an error on the Dpl range when balancing valve is on the secondary.
				//         This is why in case of error, it's only when balancing valve is set on the primary. And in this case we can take
				//         the 'Dp branch' value input by the user.
				CString str2 = WriteCUDouble( _U_DIFFPRESS, pclRowParameters->m_dDpBranch, false, 3, 0 );

				if( true == pclSelectedDpC->IsFlagSet( CSelectedBase::eValveDpToLarge ) && -1.0 != pclDpController->GetDplmax() )
				{
					str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclDpController->GetDplmax(), true, 3, 0 );
					FormatString( str, IDS_SSHEETSSELDPC_DPLRANGEERRORH, str2 );
				}
				else if( true == pclSelectedDpC->IsFlagSet( CSelectedBase::eValveDpToSmall ) && -1.0 != pclDpController->GetDplmin() )
				{
					str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclDpController->GetDplmin(), true, 3, 0 );
					FormatString( str, IDS_SSHEETSSELDPC_DPLRANGEERRORL, str2 );
				}

				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}
		}

		break;

		case BS_DpCColumnID::BS_DpCCID_DpMax:
		{
			bool bRed = false;

			if( pclDpController->GetDpmax() < m_clBatchSelDpCParams.m_dDpMax )
			{
				bRed = true;
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_DpMax, pclDpController->GetDpmax(), 
					iPhysicalType, iMaxDigit, iMinDecimal, true );

			if( NULL != pclCellBase && true == bRed )
			{
				pclCellBase->SetTextForegroundColor( _RED );

				CString str2 = WriteCUDouble( _U_DIFFPRESS, m_clBatchSelDpCParams.m_dDpMax, true, 3, 0 );
				CString str3 = WriteCUDouble( _U_DIFFPRESS, pclDpController->GetDpmax(), true, 3, 0 );
				FormatString( str, IDS_PRODUCTSELECTION_ERROR_DPMAX, str2, str3 );

				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}
		}

		break;

		case BS_DpCColumnID::BS_DpCCID_TemperatureRange:
		{
			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_TemperatureRange, pclDpController->GetTempRange(), true );

			if( NULL != pclCellBase )
			{
				if( m_clBatchSelDpCParams.m_WC.GetTemp() < pclDpController->GetTmin() || m_clBatchSelDpCParams.m_WC.GetTemp() > pclDpController->GetTmax() )
				{
					pclCellBase->SetTextForegroundColor( _RED );

					FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclDpController->GetTempRange() );

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				}
			}
		}
		break;

		case BS_DpCColumnID::BS_DpCCID_PipeSize:
		{
			CString str = _T( "" );
			CSelectPipe selPipe( &m_clBatchSelDpCParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclDpController->GetSizeKey(), selPipe );
			}

			pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_PipeSize, selPipe.GetpPipe()->GetName(), true );
		}
		break;

		case BS_DpCColumnID::BS_DpCCID_PipeLinDp:
		{
			bool bOrange = false;
			CSelectPipe selPipe( &m_clBatchSelDpCParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclDpController->GetSizeKey(), selPipe );
			}

			if( selPipe.GetLinDp() > pclTechnicalParameter->GetPipeMaxDp() || selPipe.GetLinDp() < pclTechnicalParameter->GetPipeMinDp() )
			{
				bOrange = true;
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_PipeLinDp, selPipe.GetLinDp(), iPhysicalType, iMaxDigit,
					iMinDecimal, true );

			if( NULL != pclCellBase && true == bOrange )
			{
				pclCellBase->SetTextForegroundColor( _ORANGE );

				CString str2 = CString( _T("[") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechnicalParameter->GetPipeMinDp() );
				str2 += CString( _T(" - ") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechnicalParameter->GetPipeMaxDp() );
				str2 += CString( _T("]") );
				FormatString( str, IDS_SSHEETSSEL_PIPELINDPERROR, str2 );

				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}
		}
		break;

		case BS_DpCColumnID::BS_DpCCID_PipeV:
		{
			bool bOrange = false;
			CSelectPipe selPipe( &m_clBatchSelDpCParams );

			if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
			{
				pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclDpController->GetSizeKey(), selPipe );
			}

			if( selPipe.GetU() > pclTechnicalParameter->GetPipeMaxVel() || selPipe.GetU() < pclTechnicalParameter->GetPipeMinVel() )
			{
				bOrange = true;
			}

			DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_PipeV, selPipe.GetU(), iPhysicalType, 
					iMaxDigit, iMinDecimal, true );

			if( NULL != pclCellBase && true == bOrange )
			{
				pclCellBase->SetTextForegroundColor( _ORANGE );

				CString str2 = CString( _T("[") ) + WriteCUDouble( _U_VELOCITY, pclTechnicalParameter->GetPipeMinVel() );
				str2 += CString( _T(" - ") ) + WriteCUDouble( _U_VELOCITY, pclTechnicalParameter->GetPipeMaxVel() );
				str2 += CString( _T("]") );
				FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );

				m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
			}
		}

		case BS_DpCColumnID::BS_DpCCID_BvName:
			if( NULL != pclBvTAProduct )
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvName, pclBvTAProduct->GetName(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvName, _T("-"), true );
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvIn:
			if( NULL != pclBvTAProduct )
			{
				CString strMvIn = TASApp.LoadLocalizedString( ( eMvLoc::MvLocPrimary == m_clBatchSelDpCParams.m_eMvLoc ) ? IDS_BATCHSELDPC_BVPRIMARY : IDS_BATCHSELDPC_BVSECONDARY );
				bool bOrange = false;

				if( true == pclSelectedDpC->GetFlag( CSelectedBase::eValveBatchDpCForceMvInSec ) )
				{
					bOrange = true;
					strMvIn = TASApp.LoadLocalizedString( IDS_BATCHSELDPC_BVSECONDARY );
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvIn, strMvIn, true );

				if( NULL != pclCellBase && true == bOrange )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );

					// Initialize dialog strings
					CString str2 = WriteCUDouble( _U_DIFFPRESS, pclRowParameters->m_dDpBranch, true, 2, 0 );
					FormatString( str, IDS_BATCHSELDPC_BVFORCEDINSEC, str2 );

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				}
			}
			else
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvIn, _T("-"), true );
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvMaterial:
			if( NULL != pclBvTAProduct )
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvMaterial, pclBvTAProduct->GetBodyMaterial(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvMaterial, _T("-"), true );
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvConnection:
			if( NULL != pclBvTAProduct )
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvConnection, pclBvTAProduct->GetConnect(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvConnection, _T("-"), true );
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvVersion:
			if( NULL != pclBvTAProduct )
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvVersion, pclBvTAProduct->GetVersion(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvVersion, _T("-"), true );
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvPN:
			if( NULL != pclBvTAProduct )
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvPN, pclBvTAProduct->GetPN().c_str(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvPN, _T("-"), true );
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvSize:
			if( NULL != pclBvTAProduct )
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvSize, pclBvTAProduct->GetSize(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvSize, _T("-"), true );
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvPreset:
			if( NULL != pclBvTAProduct )
			{
				bool bOrange = false;

				if( false == pclSelectedBv->IsFlagSet( CSelectedBase::eValveMaxSetting ) &&
					true == pclSelectedBv->IsFlagSet( CSelectedBase::eValveSetting ) )
				{
					bOrange = true;
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_BvPreset, pclSelectedBv->GetH(), iPhysicalType, iMaxDigit,
						iMinDecimal, true );

				if( NULL != pclCellBase && true == bOrange )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );

					CString str2 = pclBvValveCharacteristic->GetSettingString( pclSelectedBv->GetH() );
					double dMinRecommendedSetting = pclBvTAProduct->GetValveCharacteristic()->GetMinRecSetting();
					str2 += _T(" < ") + pclBvValveCharacteristic->GetSettingString( dMinRecommendedSetting );
					FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str2 );

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				}
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvDpSignal:
			if( NULL != pclBvTAProduct )
			{
				if( true == pclBvTAProduct->IsKvSignalEquipped() )
				{
					pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_BvDpSignal, pclSelectedBv->GetDpSignal(), iPhysicalType,
							iMaxDigit, iMinDecimal, true );
				}
				else
				{
					pclBSMessage->SetCellNumberAsText( BS_DpCColumnID::BS_DpCCID_BvDpSignal, _T("-"), _U_NODIM, 0, 0, true );
				}
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvDp:
			if( NULL != pclBvTAProduct )
			{
				bool bOrange = false;

				if( true == m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked && pclSelectedBv->GetDp() > 0.0 )
				{
					if( pclSelectedBv->GetDp() < pclTechnicalParameter->GetValvMinDp( pclBvTAProduct->GetTypeIDPtr().ID ) 
							|| pclSelectedBv->GetDp() > pclTechnicalParameter->VerifyValvMaxDp( pclBvTAProduct ) )
					{
						bOrange = true;
					}
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_BvDp, pclSelectedBv->GetDp(), 
						iPhysicalType, iMaxDigit, iMinDecimal, true );

				if( NULL != pclCellBase && true == bOrange )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );

					CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDp() );

					if( pclSelectedBv->GetDp() < pclTechnicalParameter->GetValvMinDp( pclBvTAProduct->GetTypeIDPtr().ID ) )
					{
						str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechnicalParameter->GetValvMinDp( pclBvTAProduct->GetTypeIDPtr().ID ), true );

						// "Dp is below the minimum value defined in technical parameters ( %1 )"
						FormatString( str, IDS_SSHEETSSEL_DPERRORL, str2 );
					}
					else if( pclSelectedBv->GetDp() > pclTechnicalParameter->VerifyValvMaxDp( pclBvTAProduct ) )
					{
						str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechnicalParameter->VerifyValvMaxDp( pclBvTAProduct ), true );

						// "Dp is above the maximum value defined in technical parameters ( %1 )"
						FormatString( str, IDS_SSHEETSSEL_DPERRORH, str2 );
					}

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				}
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvDpFullOpening:
			if( NULL != pclBvTAProduct )
			{
				if( false == m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked || pclSelectedBv->GetDp() <= 0.0 )
				{
					bool bOrange = false;

					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eValveFullODp ) )
					{
						bOrange = true;
					}

					DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_BvDpFullOpening, pclSelectedBv->GetDpFullOpen(), 
							iPhysicalType, iMaxDigit, iMinDecimal, true );

					if( NULL != pclCellBase && true == bOrange )
					{
						pclCellBase->SetTextForegroundColor( _ORANGE );

						CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpFullOpen() );
						str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechnicalParameter->GetValvMinDp(), true );

						// "Pressure drop on valve fully open is below the minimum value defined in technical parameters ( %1 )"
						FormatString( str, IDS_SSHEETSSEL_DPFOERROR, str2 );

						m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
						m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					}
				}
				else
				{
					// To match the individual selection, when it's a fixed orifice valve we don't show the 'Dp 100%' column.
					pclBSMessage->SetCellNumberAsText( BS_DpCColumnID::BS_DpCCID_BvDpFullOpening, _T("-"), _U_NODIM, 0, 0, true );
				}
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvDpHalfOpening:
			if( NULL != pclBvTAProduct )
			{
				if( false == m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked || pclSelectedBv->GetDp() <= 0.0 )
				{
					bool bOrange = false;

					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eValveHalfODp ) )
					{
						bOrange = true;
					}

					DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_BvDpHalfOpening, 
							pclSelectedBv->GetDpHalfOpen(), iPhysicalType, iMaxDigit, iMinDecimal, true );

					if( NULL != pclCellBase && true == bOrange )
					{
						pclCellBase->SetTextForegroundColor( _ORANGE );

						CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpHalfOpen() );

						if( pclSelectedBv->GetDpHalfOpen() < pclTechnicalParameter->GetValvMinDp() )
						{
							str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechnicalParameter->GetValvMinDp(), true );
							FormatString( str, IDS_SSHEETSSEL_DPHOERRORL, str2 );
						}
						else if( pclSelectedBv->GetDpHalfOpen() > pclTechnicalParameter->VerifyValvMaxDp( pclBvTAProduct ) )
						{
							str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechnicalParameter->VerifyValvMaxDp( pclBvTAProduct ), true );
							FormatString( str, IDS_SSHEETSSEL_DPHOERRORH, str2 );
						}

						m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
						m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					}
				}
				else
				{
					// To match the individual selection, when it's a fixed orifice valve we don't show the 'Dp 100%' column.
					pclBSMessage->SetCellNumberAsText( BS_DpCColumnID::BS_DpCCID_BvDpFullOpening, _T("-"), _U_NODIM, 0, 0, true );
				}
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvTemperatureRange:
			if( NULL != pclBvTAProduct )
			{
				bool bRed = false;

				if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eTemperature ) )
				{
					bRed = true;
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvTemperatureRange, 
						pclBvTAProduct->GetTempRange(), true );

				if( NULL != pclCellBase && true == bRed )
				{
					pclCellBase->SetTextForegroundColor( _RED );

					FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclBvTAProduct->GetTempRange() );

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				}
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvPipeSize:
			
			if( NULL != pclBvTAProduct )
			{
				CString str = _T( "" );
				CSelectPipe selPipe( &m_clBatchSelDpCParams );

				if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
				{
					pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBvTAProduct->GetSizeKey(), selPipe );
				}

				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvPipeSize, selPipe.GetpPipe()->GetName(), true );
			}
			else
			{
				pclBSMessage->SetCellText( BS_DpCColumnID::BS_DpCCID_BvPipeSize, _T("-"), true );
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvPipeLinDp:
			
			if( NULL != pclBvTAProduct )
			{
				bool bOrange = false;
				CSelectPipe selPipe( &m_clBatchSelDpCParams );

				if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
				{
					pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBvTAProduct->GetSizeKey(), selPipe );
				}

				if( selPipe.GetLinDp() > pclTechnicalParameter->GetPipeMaxDp() || selPipe.GetLinDp() < pclTechnicalParameter->GetPipeMinDp() )
				{
					bOrange = true;
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_BvPipeLinDp, selPipe.GetLinDp(), 
						iPhysicalType, iMaxDigit, iMinDecimal, true );

				if( NULL != pclCellBase && true == bOrange )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );

					CString str2 = CString( _T("[") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechnicalParameter->GetPipeMinDp() );
					str2 += CString( _T(" - ") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechnicalParameter->GetPipeMaxDp() );
					str2 += CString( _T("]") );
					FormatString( str, IDS_SSHEETSSEL_PIPELINDPERROR, str2 );

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				}
			}

			break;

		case BS_DpCColumnID::BS_DpCCID_BvPipeV:
			
			if( NULL != pclBvTAProduct )
			{
				bool bOrange = false;
				CSelectPipe selPipe( &m_clBatchSelDpCParams );
				
				if( NULL != pclRowParameters->m_pclBatchResults && NULL != pclRowParameters->m_pclBatchResults->GetSelectPipeList() )
				{
					pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetMatchingPipe( pclBvTAProduct->GetSizeKey(), selPipe );
				}

				if( selPipe.GetU() > pclTechnicalParameter->GetPipeMaxVel() || selPipe.GetU() < pclTechnicalParameter->GetPipeMinVel() )
				{
					bOrange = true;
				}

				DlgOutputHelper::CCellBase *pclCellBase = pclBSMessage->SetCellNumber( BS_DpCColumnID::BS_DpCCID_BvPipeV, selPipe.GetU(), 
						iPhysicalType, iMaxDigit, iMinDecimal, true );

				if( NULL != pclCellBase && true == bOrange )
				{
					pclCellBase->SetTextForegroundColor( _ORANGE );

					CString str2 = CString( _T("[") ) + WriteCUDouble( _U_VELOCITY, pclTechnicalParameter->GetPipeMinVel() );
					str2 += CString( _T(" - ") ) + WriteCUDouble( _U_VELOCITY, pclTechnicalParameter->GetPipeMaxVel() );
					str2 += CString( _T("]") );
					FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );

					m_clInterface.EnableTextTipFetch( true, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
					m_clInterface.AddTextTipFetchTrigger( pclCellBase, pclRowParameters->m_lRow, str, (int)m_clBatchSelDpCParams.m_eProductSubCategory );
				}
			}

			break;
	}
}

void CDlgBatchSelDpC::_UpdateDpBranchCheckboxState()
{
	m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked = ( BST_CHECKED == m_clCheckboxDpBranchOrKvs.GetCheck() ) ? true : false;
	m_mapColumnList[BS_ColumnID::BS_CID_Input_DpBranch].m_bEnabled = m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked;

	// By default, we don't enable and display 'Kvs' input column when user choice to stabilize on a branch.
	m_mapColumnList[BS_ColumnID::BS_CID_Input_Kvs].m_bEnabled = false;
	m_mapColumnList[BS_ColumnID::BS_CID_Input_Kvs].m_bDisplayed = false;
}

void CDlgBatchSelDpC::_UpdateDpBranchColumn()
{
	if( 0 == m_mapAllRowData.size()
			|| BS_InputsVerificationStatus::BS_IVS_Empty == m_eInputsVerificationStatus
			|| BS_InputsVerificationStatus::BS_IVS_NotYetDone == m_eInputsVerificationStatus )
	{
		// When there is not yet data pasted in the batch selection output, user can change input method (Flow or Power/DT and Dp or not).
		BS_UpdateInputColumnLayout( BS_DpCUpdateInputColumn::BS_DpCUIC_DpBranch );

		// HYS-1188: We also remove the 'Kvs' column if exist.
		BS_UpdateInputColumnLayout( BS_DpCUpdateInputColumn::BS_DpCUIC_Kvs );
	}
}

void CDlgBatchSelDpC::_UpdateKvsCheckboxState()
{
	m_clBatchSelDpCParams.m_bIsCheckboxKvsChecked = ( BST_CHECKED == m_clCheckboxDpBranchOrKvs.GetCheck() ) ? true : false;
	m_mapColumnList[BS_ColumnID::BS_CID_Input_Kvs].m_bEnabled = m_clBatchSelDpCParams.m_bIsCheckboxKvsChecked;

	// HYS-1188: By default, we don't enable and display 'Dpl' input column when user choice to stabilize on a control valve.
	m_mapColumnList[BS_ColumnID::BS_CID_Input_DpBranch].m_bEnabled = false;
	m_mapColumnList[BS_ColumnID::BS_CID_Input_DpBranch].m_bDisplayed = false;
}

void CDlgBatchSelDpC::_UpdateKvsColumn()
{
	if( 0 == m_mapAllRowData.size()
			|| BS_InputsVerificationStatus::BS_IVS_Empty == m_eInputsVerificationStatus
			|| BS_InputsVerificationStatus::BS_IVS_NotYetDone == m_eInputsVerificationStatus )
	{
		// When there is not yet data pasted in the batch selection output, user can change input method (Flow or Power/DT and Dp or not).
		BS_UpdateInputColumnLayout( BS_DpCUpdateInputColumn::BS_DpCUIC_Kvs );

		// HYS-1188: We also remove the 'Dpl' column if exist.
		BS_UpdateInputColumnLayout( BS_DpCUpdateInputColumn::BS_DpCUIC_DpBranch );
	}
}

void CDlgBatchSelDpC::_UpdateDpMaxFieldState()
{
	m_clBatchSelDpCParams.m_bIsDpMaxChecked = ( BST_CHECKED == m_clCheckboxDpMax.GetCheck() ) ? true : false;

	if( true == m_clBatchSelDpCParams.m_bIsDpMaxChecked )
	{
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( TRUE );
		m_clExtEditDpMax.SetFocus();
		m_clExtEditDpMax.SetWindowText( _T( "" ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( TRUE );
	}
	else
	{
		m_clBatchSelDpCParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( FALSE );
		m_clExtEditDpMax.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( FALSE );
	}

	m_mapColumnList[BS_DpCColumnID::BS_DpCCID_DpMax].m_bEnabled = m_clBatchSelDpCParams.m_bIsDpMaxChecked;
}

void CDlgBatchSelDpC::_UpdateDpMaxColumn()
{
	if( 0 == m_mapAllRowData.size()
			|| BS_InputsVerificationStatus::BS_IVS_Empty == m_eInputsVerificationStatus
			|| BS_InputsVerificationStatus::BS_IVS_NotYetDone == m_eInputsVerificationStatus )
	{
		// When there is not yet data pasted in the batch selection output, user can change input method (Flow or Power/DT and Dp or not).
		BS_UpdateInputColumnLayout( BS_DpCUpdateInputColumn::BS_DpCUIC_DpMax );
	}
}

void CDlgBatchSelDpC::_UpdateDpStabilizedOn()
{
	m_clBatchSelDpCParams.m_eDpStab = (eDpStab)m_ComboDpStab.GetCurSel();

	if( eDpStab::DpStabOnBranch == m_clBatchSelDpCParams.m_eDpStab )
	{
		m_clCheckboxDpBranchOrKvs.SetWindowText( TASApp.LoadLocalizedString( IDS_BATCHSELDPC_DPBRANCH ) );
	}
	else
	{
		m_clCheckboxDpBranchOrKvs.SetWindowText( TASApp.LoadLocalizedString( IDS_BATCHSELDPC_KVSCTRLVALVE ) );
	}
}

void CDlgBatchSelDpC::_UpdateDpCLocation()
{
	m_clBatchSelDpCParams.m_eDpCLoc = (eDpCLoc)m_ComboDpCLoc.GetCurSel();
}

void CDlgBatchSelDpC::_UpdateMvLocation()
{
	m_clBatchSelDpCParams.m_eMvLoc = (eMvLoc)m_ComboMvLoc.GetCurSel();
}

bool CDlgBatchSelDpC::_CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters )
{
	// Here, we can have either a best result that is saved in the 'pclRowParameters->m_pclSelectedProduct' variable or we can have more that one result.
	// If we have a best result, we save it in a 'CDS_SSelDpC' object and we call the 'RViewSSelDpC::Suggest' method in edit mode. 
	// Otherwise, we call the 'RViewSSelDpC::Suggest' method in normal mode to display all results and let the user to choose one.

	if( NULL == pclRowParameters || NULL == pclRowParameters->m_pclBatchResults || NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()
			|| NULL == pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe() )
	{
		ASSERTA_RETURN( false );
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( m_clBatchSelDpCParams.m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( false );
	}

	int iDN65 = pclTableDN->GetSize( _T("DN_65") );
	
	// This method will just copy the variables that are in the 'CProductSelectionParameters' base class.
	m_clIndSelDpCParams.CopyFrom( &m_clBatchSelDpCParams );

	// We need to do it manually for all variables of 'm_clBatchSelDpCParam' that are needed by the 'RViewSSelDpC'.

	// All variables for 'CIndividualSelectionParameters'.
	m_clIndSelDpCParams.m_dFlow = pclRowParameters->m_dFlow;

	CDB_Pipe *pclBestPipe =  pclRowParameters->m_pclBatchResults->GetSelectPipeList()->GetBestPipe();
	int iSize = pclBestPipe->GetSizeKey( m_clBatchSelDpCParams.m_pTADB );

	if( iSize < iDN65 )
	{
		m_clIndSelDpCParams.m_strComboTypeID = m_clBatchSelDpCParams.m_strComboTypeBelow65ID;
		m_clIndSelDpCParams.m_strComboFamilyID = m_clBatchSelDpCParams.m_strComboFamilyBelow65ID;
		m_clIndSelDpCParams.m_strComboMaterialID = m_clBatchSelDpCParams.m_strComboMaterialBelow65ID;
		m_clIndSelDpCParams.m_strComboConnectID = m_clBatchSelDpCParams.m_strComboConnectBelow65ID;
		m_clIndSelDpCParams.m_strComboVersionID = m_clBatchSelDpCParams.m_strComboVersionBelow65ID;
		m_clIndSelDpCParams.m_strComboPNID = m_clBatchSelDpCParams.m_strComboPNBelow65ID;
	}
	else
	{
		m_clIndSelDpCParams.m_strComboTypeID = m_clBatchSelDpCParams.m_strComboTypeAbove50ID;
		m_clIndSelDpCParams.m_strComboFamilyID = m_clBatchSelDpCParams.m_strComboFamilyAbove50ID;
		m_clIndSelDpCParams.m_strComboMaterialID = m_clBatchSelDpCParams.m_strComboMaterialAbove50ID;
		m_clIndSelDpCParams.m_strComboConnectID = m_clBatchSelDpCParams.m_strComboConnectAbove50ID;
		m_clIndSelDpCParams.m_strComboVersionID = m_clBatchSelDpCParams.m_strComboVersionAbove50ID;
		m_clIndSelDpCParams.m_strComboPNID = m_clBatchSelDpCParams.m_strComboPNAbove50ID;
	}

	m_clIndSelDpCParams.m_eFilterSelection = m_clBatchSelDpCParams.m_eFilterSelection;
	
	// All variables in for 'CIndSelDpCParams'.
	// HYS-1188: We split 'm_iDpCCheckDpKvs' in two variables.
	if( eDpStab::DpStabOnBranch == m_clBatchSelDpCParams.m_eDpStab )
	{
		m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked = m_clBatchSelDpCParams.m_bIsCheckboxDpBranchChecked;
	}
	else
	{
		m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked = m_clBatchSelDpCParams.m_bIsCheckboxKvsChecked;
	}

	m_clIndSelDpCParams.m_bIsDpMaxChecked = m_clBatchSelDpCParams.m_bIsDpMaxChecked;
	m_clIndSelDpCParams.m_dDpBranch = pclRowParameters->m_dDpBranch;
	m_clIndSelDpCParams.m_dDpMax = m_clBatchSelDpCParams.m_dDpMax;
	m_clIndSelDpCParams.m_dKv = pclRowParameters->m_dKvs;
	m_clIndSelDpCParams.m_eDpStab = m_clBatchSelDpCParams.m_eDpStab;
	m_clIndSelDpCParams.m_eDpCLoc = m_clBatchSelDpCParams.m_eDpCLoc;
	m_clIndSelDpCParams.m_eMvLoc = m_clBatchSelDpCParams.m_eMvLoc;

	// Create now the 'CSelectedDpCList' that 'RViewSSelDpC' needs.
	if( NULL != m_clIndSelDpCParams.m_pclSelectDpCList )
	{
		delete m_clIndSelDpCParams.m_pclSelectDpCList;
		m_clIndSelDpCParams.m_pclSelectDpCList = NULL;
	}

	m_clIndSelDpCParams.m_pclSelectDpCList = new CSelectDpCList();

	if( NULL == m_clIndSelDpCParams.m_pclSelectDpCList )
	{
		ASSERTA_RETURN( false );
	}
	
	// We now transfer results from the batch selection list to the individual selection list.
	pclRowParameters->m_pclBatchResults->TransferResultsToIndividualSelectList( m_clIndSelDpCParams.m_pclSelectDpCList );

	// Create an 'CDS_SelDpC' to allow 'RViewSSelDpC' to be in edit mode with this object selected.
	// Remark: Only if we have only one result.
	if( NULL != pclRowParameters->m_pclSelectedProduct )
	{
		CDS_SSelDpC *pclSSelDpC = NULL;

		if( NULL == pclRowParameters->m_pclCDSSelSelected )
		{
			pclSSelDpC = _BS_CreateSSelDpC( pclRowParameters );
		}
		else
		{
			pclSSelDpC = dynamic_cast<CDS_SSelDpC *>( pclRowParameters->m_pclCDSSelSelected );
		}

		if( NULL == pclSSelDpC )
		{
			return false;
		}

		pclRowParameters->m_pclCDSSelSelected = pclSSelDpC;
		m_clIndSelDpCParams.m_SelIDPtr = pclSSelDpC->GetIDPtr();
	}

	return true;
}
