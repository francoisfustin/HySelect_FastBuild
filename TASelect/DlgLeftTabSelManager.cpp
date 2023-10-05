#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "DataStruct.h"
#include "SelectPM.h"
#include "DlgInfoSSelDpC.h"
#include "DlgSelectionComboHelper.h"
#include "RViewSSelSS.h"

#include "DlgSelectionBase.h"

#include "DlgIndSelBase.h"
#include "DlgIndSelBv.h"
#include "DlgIndSelDpC.h"
#include "DlgIndSelTrv.h"
#include "DlgIndSelCtrlBase.h"
#include "DlgIndSelCv.h"
#include "DlgIndSel6WayValve.h"
#include "DlgIndSelPICv.h"
#include "DlgIndSelHub.h"
#include "DlgIndSelBCV.h"
#include "DlgIndSelSeparator.h"
#include "DlgIndSelPM.h"
#include "DlgIndSelDpCBCV.h"
#include "DlgIndSelSv.h"
#include "DlgIndSelSafetyValve.h"
#include "DlgIndSel6WayValve.h"
#include "DlgIndSelSmartControlValve.h"
#include "DlgIndSelSmartDpC.h"

#include "DlgBatchSelBase.h"
#include "DlgBatchSelCtrlBase.h"
#include "DlgBatchSelBCv.h"
#include "DlgBatchSelBv.h"
#include "DlgBatchSelDpC.h"
#include "DlgBatchSelPICv.h"
#include "DlgBatchSelSeparator.h"
#include "DlgBatchSelSmartControlValve.h"
#include "DlgBatchSelSmartDpC.h"

#include "DlgWizardSelPM.h"

#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgLeftTabSelManager *pDlgLeftTabSelManager = NULL;

IMPLEMENT_DYNCREATE( CDlgLeftTabSelManager, CDlgLeftTabBase )

CDlgLeftTabSelManager::CDlgLeftTabSelManager( CWnd *pParent )
	: CDlgLeftTabBase( CMyMFCTabCtrl::TabIndex::etiSSel , CDlgLeftTabSelManager::IDD, pParent )
{
	pDlgLeftTabSelManager = this;
	m_bHScrollBarVisible = false;
	m_bVScrollBarVisible = false;
	m_bBypass = false;

	m_pclMainRbnCBSelPipeSizeIDSaved = _T("");
}

CDlgLeftTabSelManager::~CDlgLeftTabSelManager()
{
	pDlgLeftTabSelManager = NULL;
}

BEGIN_MESSAGE_MAP( CDlgLeftTabSelManager, CDlgLeftTabBase )
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CDlgLeftTabSelManager::DoDataExchange( CDataExchange* pDX )
{
	CDlgLeftTabBase::DoDataExchange( pDX );
}

BOOL CDlgLeftTabSelManager::OnInitDialog()
{
	// Do the Default Initialization
	CDlgLeftTabBase::OnInitDialog();

	// Initialize all left tab dialogs for the individual selection mode.
	if( true == TASApp.IsAirVentSepDisplayed() )
	{
		_AddPage( IDD_DLGINDSELSEPARATOR, ProductSubCategory::PSC_PM_Separator, ProductSelectionMode_Individual );
	}

	if( true == TASApp.IsPressureMaintenanceDisplayed() )
	{
		_AddPage( IDD_DLGINDSELPRESSUREMAINTENANCE, ProductSubCategory::PSC_PM_ExpansionVessel, ProductSelectionMode_Individual );
	}

	if( true == TASApp.IsSafetyValveDisplayed() )
	{
		_AddPage( IDD_DLGINDSELSAFETYVALVE, ProductSubCategory::PSC_PM_SafetyValve, ProductSelectionMode_Individual );
	}

	_AddPage( IDD_DLGINDSELBV, ProductSubCategory::PSC_BC_RegulatingValve, ProductSelectionMode_Individual );
	
	if( true == TASApp.IsDpCDisplayed() )
	{
		_AddPage( IDD_DLGINDSELDPC, ProductSubCategory::PSC_BC_DpController, ProductSelectionMode_Individual );
	}

	if( true == TASApp.IsBCvDisplayed() )
	{
		_AddPage( IDD_DLGINDSELBCV, ProductSubCategory::PSC_BC_BalAndCtrlValve, ProductSelectionMode_Individual );
	}

	if( true == TASApp.IsPICvDisplayed() )
	{
		_AddPage( IDD_DLGINDSELPICV, ProductSubCategory::PSC_BC_PressureIndepCtrlValve, ProductSelectionMode_Individual );
	}

	if( true == TASApp.IsDpCBCVDisplayed() )
	{
		_AddPage( IDD_DLGINDSELDPCBCV, ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve, ProductSelectionMode_Individual );
	}

	_AddPage( IDD_DLGINDSELCV, ProductSubCategory::PSC_BC_ControlValve, ProductSelectionMode_Individual );

	if( true == TASApp.IsSmartControlValveDisplayed() )
	{
		_AddPage( IDD_DLGINDSELSMARTCONTROLVALVE, ProductSubCategory::PSC_BC_SmartControlValve, ProductSelectionMode_Individual );
	}
	
	if( true == TASApp.IsSmartDpCDisplayed() )
	{
		_AddPage( IDD_DLGINDSELSMARTDPC, ProductSubCategory::PSC_BC_SmartDpC, ProductSelectionMode_Individual );
	}

	if( true == TASApp.Is6WayCVDisplayed() )
	{
		_AddPage( IDD_DLGINDSEL6WAYVALVE, ProductSubCategory::PSC_BC_6WayControlValve, ProductSelectionMode_Individual );
	}

	if( true == TASApp.IsShutOffValveDisplayed() )
	{
		_AddPage( IDD_DLGINDSELSV, ProductSubCategory::PSC_ESC_ShutoffValve, ProductSelectionMode_Individual );
	}

	if( true == TASApp.IsTrvDisplayed() )
	{
		_AddPage( IDD_DLGINDSELTRV, ProductSubCategory::PSC_TC_ThermostaticValve, ProductSelectionMode_Individual );
	}

	// Initialize all left tab dialogs for the batch selection mode.
	if( true == TASApp.IsAirVentSepDisplayed() )
	{
		_AddPage( IDD_DLGBATCHSELSEPARATOR, ProductSubCategory::PSC_PM_Separator, ProductSelectionMode_Batch );
	}

	_AddPage( IDD_DLGBATCHSELBV, ProductSubCategory::PSC_BC_RegulatingValve, ProductSelectionMode_Batch );
	
	if( true == TASApp.IsDpCDisplayed() )
	{
		_AddPage( IDD_DLGBATCHSELDPC, ProductSubCategory::PSC_BC_DpController, ProductSelectionMode_Batch );
	}

	if( true == TASApp.IsBCvDisplayed() )
	{
		_AddPage( IDD_DLGBATCHSELBCV, ProductSubCategory::PSC_BC_BalAndCtrlValve, ProductSelectionMode_Batch );
	}

	if( true == TASApp.IsPICvDisplayed() )
	{
		_AddPage( IDD_DLGBATCHSELPICV, ProductSubCategory::PSC_BC_PressureIndepCtrlValve, ProductSelectionMode_Batch );
	}

	if( true == TASApp.IsSmartControlValveDisplayed() )
	{
		_AddPage( IDD_DLGBATCHSELSMARTCONTROLVALVE, ProductSubCategory::PSC_BC_SmartControlValve, ProductSelectionMode_Batch );
	}

	if( true == TASApp.IsSmartDpCDisplayed() )
	{
		_AddPage( IDD_DLGBATCHSELSMARTDPC, ProductSubCategory::PSC_BC_SmartDpC, ProductSelectionMode_Batch );
	}

	// Initialize all left tab dialogs for the wizard selection mode.
	if( true == TASApp.IsPressureMaintenanceDisplayed() )
	{
		_AddPage( IDD_DLGWIZARDSELPM, ProductSubCategory::PSC_PM_ExpansionVessel, ProductSelectionMode_Wizard );
	}

	return TRUE;
}

CDlgSelectionBase *CDlgLeftTabSelManager::GetCurrentLeftTabDialog()
{
	return GetLeftTabDialog( pMainFrame->GetCurrentProductSubCategory() );
}

CDlgSelectionBase *CDlgLeftTabSelManager::GetLeftTabDialog( ProductSubCategory eProductSubCategory )
{
	return m_clAllLeftDialogs.GetLeftTabDialog( eProductSubCategory );
}

CDlgSelectionBase *CDlgLeftTabSelManager::GetLeftTabDialog( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	return m_clAllLeftDialogs.GetLeftTabDialog( eProductSubCategory, eProductSelectionMode );
}

void CDlgLeftTabSelManager::OnApplicationLook( COLORREF cBackColor )
{
	for( int iLoopProductSubCategory = ProductSubCategory::PSC_First; iLoopProductSubCategory <= ProductSubCategory::PSC_Last; iLoopProductSubCategory++ )
	{
		for( int iLoopSelectionMode = ProductSelectionMode_First; iLoopSelectionMode <= ProductSelectionMode_Last; iLoopSelectionMode++ )
		{
			if( NULL != m_clAllLeftDialogs.GetLeftTabDialog( (ProductSubCategory)iLoopProductSubCategory, (ProductSelectionMode)iLoopSelectionMode ) )
			{
				m_clAllLeftDialogs.GetLeftTabDialog( (ProductSubCategory)iLoopProductSubCategory, (ProductSelectionMode)iLoopSelectionMode )->SetApplicationBackground( cBackColor );
				m_clAllLeftDialogs.GetLeftTabDialog( (ProductSubCategory)iLoopProductSubCategory, (ProductSelectionMode)iLoopSelectionMode )->OnApplicationLook( cBackColor );
			}
		}
	}
}

void CDlgLeftTabSelManager::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( SB_HORZ, &rScrollInfo );

	// Retrieve current position
	int nCurrentPos = GetScrollPos( SBS_HORZ );
	int nPreviousPos = nCurrentPos;

	switch( nSBCode )
	{
		// User has clicked on arrow left.
		case SB_LINELEFT:
			nCurrentPos = max( nCurrentPos - 10, 0 );
			break;

		// User has clicked on arrow right.
		case SB_LINERIGHT:
			nCurrentPos = min( nCurrentPos + 10, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// User has clicked on bar between arrow left and scroll box
		case SB_PAGELEFT:
			nCurrentPos = max( nCurrentPos - (int)rScrollInfo.nPage, 0 );
			break;

		// User has clicked on bar between arrow right and scroll box
		case SB_PAGERIGHT:
			nCurrentPos = min( nCurrentPos + (int)rScrollInfo.nPage, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// First one is when user drag the scroll box.
		// Second one is when user release the scroll box.
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			// Pay attention: 'nPos' is not really a UINT but a 16 bits value! To get position on 32 bits, we must use 'nTrackPos'.
			nCurrentPos = rScrollInfo.nTrackPos;
			break;
	}

	SetScrollPos( SB_HORZ, nCurrentPos );
	ScrollWindow( nPreviousPos - nCurrentPos, 0 );

	CDlgLeftTabBase::OnHScroll( nSBCode, nPos, pScrollBar );
}

void CDlgLeftTabSelManager::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( SB_VERT, &rScrollInfo );

	// Retrieve current position
	int nCurrentPos = GetScrollPos( SBS_VERT );
	int nPreviousPos = nCurrentPos;

	switch( nSBCode )
	{
		// User has clicked on arrow up.
		case SB_LINEUP:
			nCurrentPos = max( nCurrentPos - 10, 0 );
			break;

		// User has clicked on arrow down.
		case SB_LINEDOWN:
			nCurrentPos = min( nCurrentPos + 10, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// User has clicked on bar between arrow up and scroll box
		case SB_PAGEUP:
			nCurrentPos = max( nCurrentPos - (int)rScrollInfo.nPage, 0 );
			break;

		// User has clicked on bar between arrow down and scroll box
		case SB_PAGEDOWN:
			nCurrentPos = min( nCurrentPos + (int)rScrollInfo.nPage, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// First one is when user drag the scroll box.
		// Second one is when user release the scroll box.
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			// Pay attention: 'nPos' is not really a UINT but a 16 bits value! To get position on 32 bits, we must use 'nTrackPos'.
			nCurrentPos = rScrollInfo.nTrackPos;
			break;
	}

	SetScrollPos( SB_VERT, nCurrentPos );
	ScrollWindow( 0, nPreviousPos - nCurrentPos );

	CDlgLeftTabBase::OnVScroll( nSBCode, nPos, pScrollBar );
}

void CDlgLeftTabSelManager::OnSize( UINT nType, int cx, int cy )
{
	CDlgLeftTabBase::OnSize( nType, cx, cy );

	if( ProductSubCategory::PSC_Undefined == pMainFrame->GetCurrentProductSubCategory() )
	{
		return;
	}

	if( NULL != m_clAllLeftDialogs.GetLeftTabDialog() )
	{
		CRect rectLeftTabDialog;
		m_clAllLeftDialogs.GetLeftTabDialog()->GetClientRect( &rectLeftTabDialog );

		CRect rectClient;
		GetClientRect( &rectClient );

		m_clAllLeftDialogs.GetLeftTabDialog()->SetWindowPos( NULL, -1, -1, rectClient.Width(), rectLeftTabDialog.Height(), SWP_NOMOVE | SWP_NOZORDER );
	}

	// Verify if we need or not the scrollbars.
	_CheckScrollBar();
}

void CDlgLeftTabSelManager::ChangeStaticGroupSize( CRect rectNewSize )
{
	CWnd *pWnd = GetDlgItem( IDC_STATICGROUP );
	
	if( NULL != pWnd && NULL != pWnd->GetSafeHwnd() )
	{
		CRect rect;
		pWnd->GetWindowRect( &rect );
		ScreenToClient( &rect );
		pWnd->SetWindowPos( NULL, -1, -1, rectNewSize.Width(), rectNewSize.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE );

		_CheckScrollBar();
	}
}

void CDlgLeftTabSelManager::ChangeApplicationType( ProjectType eNewProjectType )
{
	if( NULL == m_clAllLeftDialogs.GetLeftTabDialog() )
	{
		return;
	}

	m_clAllLeftDialogs.GetLeftTabDialog()->ChangeApplicationType( eNewProjectType );
}

CMainFrame::RightViewList CDlgLeftTabSelManager::UpdateLeftTabDialog( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	CDlgSelectionBase *pclDlgSelectionBase = m_clAllLeftDialogs.GetLeftTabDialog( eProductSubCategory, eProductSelectionMode );
	
	if( NULL == pclDlgSelectionBase )
	{
		// If doesn't exist.
		return CMainFrame::eUndefined;
	}

	// Do not show the pipes when you work on a Hub.
	bool bEnableComboPipes = true;

	if( ProductSubCategory::PSC_BC_HUB == eProductSubCategory )
	{
		bEnableComboPipes = false;
	}
	
	pMainFrame->EnableRibbonComboSelPipeSeries( bEnableComboPipes );
	pMainFrame->EnableRibbonComboSelPipeSize( bEnableComboPipes );

	// Prepare linked right view.
	CMainFrame::RightViewList eSelectedRightView = pclDlgSelectionBase->GetRViewID();

	if( CMainFrame::eRVSSelHub == eSelectedRightView && NULL != pDlgIndSelHub )
	{
		// Force the Hub to refresh his values because the
		// Units (for example) can be modify outside that view and
		// will not be refreshed otherwise. The problem doesn't exist
		// for other views because they are Reinitialize --> white sheet
		BeginWaitCursor();
		pDlgIndSelHub->UpdateValues( pDlgIndSelHub->IsNewDoc() );
		pDlgIndSelHub->SetNewDoc( false );
		EndWaitCursor();
	}

	// Verify the batch selection output dialog state.
	bool bMustSwitchBatch = false;
	FullScreenMode eFullScreenMode = FullScreenMode::FSM_Undefined;
		
	if( ProductSelectionMode_Batch == eProductSelectionMode )
	{
		bMustSwitchBatch = true;
		eFullScreenMode = ( true == m_clAllLeftDialogs.GetLeftTabDialog( eProductSubCategory )->IsRightViewInEdition() ) ? FullScreenMode::FSM_False : FullScreenMode::FSM_True;
	}

	_UpdateBatchSelectionOutputDialog( eProductSubCategory, eProductSelectionMode, bMustSwitchBatch, eFullScreenMode );

	_SelectPage( eProductSubCategory, eProductSelectionMode );

	CRect rectClient;
	GetClientRect( &rectClient );
	
	// Call ourself directly OnSize to change scrollbars.
	// Remark: 'SetWindowPos' doesn't send 'OnSize' message and 'MoveWindow' doesn't send good client coordinate
	//         to 'OnSize' (because 'GetClientRect' sends back client area without scrollbar).
	OnSize( 0, rectClient.Width(), rectClient.Height() );

	// Force a paint.
	Invalidate();
	UpdateWindow();

	// To update all contains of the dialog (tab control, tab window, dlg6way_tab and dlgpicv_tab).
	if ( NULL != dynamic_cast<CDlgIndSel6WayValve*>( pclDlgSelectionBase ) )
	{
		dynamic_cast<CDlgIndSel6WayValve*>( pclDlgSelectionBase )->Invalidate();
		dynamic_cast<CDlgIndSel6WayValve*>( pclDlgSelectionBase )->UpdateWindow();
	}

	return eSelectedRightView;
}

void CDlgLeftTabSelManager::OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex )
{
	// Here, we intentionally don't call the base class.

	if( CMyMFCTabCtrl::TabIndex::etiSSel != eTabIndex )
	{
		return;
	}

	CDlgSelectionBase *pclDlgSelectionBase = m_clAllLeftDialogs.GetLeftTabDialog( pMainFrame->GetCurrentProductSubCategory(), pMainFrame->GetCurrentProductSelectionMode() );

	if( NULL == pclDlgSelectionBase )
	{
		ASSERT_RETURN;
	}

	CMainFrame *pFrame = static_cast<CMainFrame*>( AfxGetMainWnd() );
	pFrame->ActiveFormView( pclDlgSelectionBase->GetRViewID() );

	UpdateLeftTabDialog( pMainFrame->GetCurrentProductSubCategory(), pMainFrame->GetCurrentProductSelectionMode() );
}

void CDlgLeftTabSelManager::OnLTTabCtrlLeaveTab( CMyMFCTabCtrl::TabIndex eTabIndex )
{
	// Base class.
	CDlgLeftTabBase::OnLTTabCtrlLeaveTab( eTabIndex );

	if( CMyMFCTabCtrl::TabIndex::etiSSel != eTabIndex )
	{
		return;
	}

	// Warn attached tab.
	if( NULL != m_clAllLeftDialogs.GetLeftTabDialog() )
	{
		m_clAllLeftDialogs.GetLeftTabDialog()->LeaveLeftTabDialog();
	}
}

void CDlgLeftTabSelManager::InitializeToolsDockablePaneContextID( void )
{
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	
	if( NULL == pclToolsDockablePane )
	{
		return;
	}

	// Because this method will be called after 'OnInitDialog' we already have here all the left tab dialogs defined.
	for( int iLoopProductSubCat = ProductSubCategory::PSC_First; iLoopProductSubCat <= ProductSubCategory::PSC_Last; iLoopProductSubCat++ )
	{
		for( int iLoopSelectionMode = ProductSelectionMode::ProductSelectionMode_First; iLoopSelectionMode <= ProductSelectionMode::ProductSelectionMode_Last; iLoopSelectionMode++ )
		{
			if( false == m_clAllLeftDialogs.IsAtLeastOneProductSelectionModeExist( (ProductSubCategory)iLoopProductSubCat, (ProductSelectionMode)iLoopSelectionMode ) 
					|| NULL == m_clAllLeftDialogs.GetLeftTabDialog( (ProductSubCategory)iLoopProductSubCat, (ProductSelectionMode)iLoopSelectionMode ) )
			{
				continue;
			}

			CToolsDockablePane::ContextParameters *pclContext = pclToolsDockablePane->RegisterContext();

			if( NULL == pclContext )
			{
				continue;
			}

			CDlgSelectionBase *pclDlgSelectionBase = m_clAllLeftDialogs.GetLeftTabDialog( (ProductSubCategory)iLoopProductSubCat, (ProductSelectionMode)iLoopSelectionMode );
			pclContext->m_iLinkedRightView = pclDlgSelectionBase->GetRViewID();

			if( ProductSubCategory::PSC_PM_ExpansionVessel == iLoopProductSubCat )
			{
				pclContext->m_bIsPaneVisible = true;
				pclContext->m_bIsPaneAutoHide = TRUE;
				pclContext->m_bIsGripperEnabled = TRUE;
				pclContext->m_bIsPaneFullScreen = false;
				pclContext->m_bIsContextMenuEnabled = true;
				pclContext->m_nTimeOutBeforeAutoHide = 1400;
				pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
				pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
				pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = true;
			}
			else
			{
				pclContext->m_bIsPaneVisible = false;
				pclContext->m_bIsPaneAutoHide = FALSE;
				pclContext->m_bIsGripperEnabled = TRUE;
				pclContext->m_bIsPaneFullScreen = false;
				pclContext->m_bIsContextMenuEnabled = true;
				pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
				pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
				pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
			}

			m_clAllLeftDialogs.SetToolsDockablePaneContextID( pclContext->m_iContextID, (ProductSubCategory)iLoopProductSubCat, (ProductSelectionMode)iLoopSelectionMode );
		}
	}

	m_bToolsDockablePaneContextInitialized = true;
}

bool CDlgLeftTabSelManager::GetToolsDockablePaneContextID( int &iContextID  )
{
	// We are forced to call first the base class. Because there must be at first an initialization of all the context ID.
	if( false == CDlgLeftTabBase::GetToolsDockablePaneContextID( iContextID ) )
	{
		return false;
	}

	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	
	if( NULL == pclToolsDockablePane )
	{
		return false;
	}

	iContextID = m_clAllLeftDialogs.GetToolsDockablePaneContextID();
	
	return ( -1 != iContextID ) ? true : false;
}

bool CDlgLeftTabSelManager::GetToolsDockablePaneContextID( ProductSubCategory eProductSubCatagory, int &iContextID )
{
	// We are forced to call first the base class. Because there must be at first an initialization of all the context ID.
	if( false == CDlgLeftTabBase::GetToolsDockablePaneContextID( iContextID ) )
	{
		return false;
	}

	iContextID = m_clAllLeftDialogs.GetToolsDockablePaneContextID( eProductSubCatagory );
	
	return ( -1 != iContextID ) ? true : false;
}

bool CDlgLeftTabSelManager::GetToolsDockablePaneContextID( ProductSubCategory eProductSubCatagory, ProductSelectionMode eProductSelectionMode, int &iContextID )
{
	// We are forced to call first the base class. Because there must be at first an initialization of all the context ID.
	if( false == CDlgLeftTabBase::GetToolsDockablePaneContextID( iContextID ) )
	{
		return false;
	}

	iContextID = m_clAllLeftDialogs.GetToolsDockablePaneContextID( eProductSubCatagory, eProductSelectionMode );

	return ( -1 != iContextID ) ? true : false;
}

void CDlgLeftTabSelManager::SetToolsDockablePaneFullScreenMode( bool bFullscreen, bool bApply )
{
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL == pclToolsDockablePane || NULL == pclToolsDockablePane->GetSafeHwnd() )
	{
		return;
	}

	int iContextID = -1;

	if( true == GetToolsDockablePaneContextID( iContextID ) )
	{
		CToolsDockablePane::ContextParameters *pclContextParameters = pclToolsDockablePane->GetContextByID( iContextID );
		pclContextParameters->m_bIsPaneFullScreen = bFullscreen;

		if( false == bFullscreen )
		{
			// HYS-1168 : It was 200 and that works for Separator and Bv but not for DpC, BCV and PIBCV because there is
			// one more line for title output.
			pclContextParameters->m_lHeight = 220;
		}

		if( true == bApply )
		{
			// Remark: 'NULL' for the second argument because it is the same right view.
			pclToolsDockablePane->RestoreContext( iContextID, true );
		}
	}
}

void CDlgLeftTabSelManager::SaveAllSSelParamsToTADS()
{
	for( int iLoopProductSubCategory = ProductSubCategory::PSC_First; iLoopProductSubCategory <= ProductSubCategory::PSC_Last; iLoopProductSubCategory++ )
	{
		for( int iLoopSelectionMode = ProductSelectionMode_First; iLoopSelectionMode <= ProductSelectionMode_Last; iLoopSelectionMode++ )
		{
			if( NULL != m_clAllLeftDialogs.GetLeftTabDialog( (ProductSubCategory)iLoopProductSubCategory, (ProductSelectionMode)iLoopSelectionMode ) )
			{
				m_clAllLeftDialogs.GetLeftTabDialog( (ProductSubCategory)iLoopProductSubCategory, (ProductSelectionMode)iLoopSelectionMode )->SaveSelectionParameters();
			}
		}
	}
}

bool CDlgLeftTabSelManager::GetCurrentProductSubCategoryAndSelectionMode( ProductSubCategory &eProductSubCategory, ProductSelectionMode &eProductSelectionMode )
{
	if( CMyMFCTabCtrl::TabIndex::etiSSel != pDlgLTtabctrl->GetActiveTab() )
	{
		return false;
	}

	eProductSubCategory = m_clAllLeftDialogs.GetProductSubCategory();
	eProductSelectionMode = m_clAllLeftDialogs.GetProductSelectionMode();

	return true;
}

void CDlgLeftTabSelManager::WriteAllSSelParamsToRegistry()
{
	SaveAllSSelParamsToTADS();

	CDS_IndSelParameter *pclIndSelParameter = TASApp.GetpTADS()->GetpIndSelParameter();

	if( NULL != pclIndSelParameter )
	{
		pclIndSelParameter->WriteToRegistry();
	}

	CDS_BatchSelParameter *pclBatchSelParameter = TASApp.GetpTADS()->GetpBatchSelParameter();
	
	if( NULL != pclBatchSelParameter )
	{
		pclBatchSelParameter->WriteToRegistry();
	}

	CDS_WizardSelParameter *pclWizardSelParameter = TASApp.GetpTADS()->GetpWizardSelParameter();
	
	if( NULL != pclWizardSelParameter )
	{
		pclWizardSelParameter->WriteToRegistry();
	}
}

bool CDlgLeftTabSelManager::_AddPage( UINT nDialogID, ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	CDlgSelectionBase *pDlg = _CreatePage( nDialogID );
	
	if( NULL != pDlg )
	{
		m_clAllLeftDialogs.AddLeftTabDialog( pDlg, eProductSubCategory, eProductSelectionMode );
	}
	
	return ( pDlg != NULL );
}

CDlgSelectionBase *CDlgLeftTabSelManager::_CreatePage( UINT nDialogID )
{
	CDlgSelectionBase *pDlg = NULL;

	switch( nDialogID )
	{
		case IDD_DLGINDSELSEPARATOR:
			pDlg = new CDlgIndSelSeparator();
			break;

		case IDD_DLGINDSELPRESSUREMAINTENANCE:
			pDlg = new CDlgIndSelPressureMaintenance();
			break;

		case IDD_DLGINDSELSAFETYVALVE:
			pDlg = new CDlgIndSelSafetyValve();
			break;

		case IDD_DLGINDSELBV:
			pDlg = new CDlgIndSelBv();
			break;

		case IDD_DLGINDSELDPC:
			pDlg = new CDlgIndSelDpC();
			break;

		case IDD_DLGINDSELBCV:
			pDlg = new CDlgIndSelBCV();
			break;

		case IDD_DLGINDSELPICV:
			pDlg = new CDlgIndSelPICv();
			break;

		case IDD_DLGINDSELDPCBCV:
			pDlg = new CDlgIndSelDpCBCV();
			break;

		case IDD_DLGINDSELCV:
			pDlg = new CDlgIndSelCv();
			break;

		case IDD_DLGINDSELSMARTCONTROLVALVE:
			pDlg = new CDlgIndSelSmartControlValve();
			break;

		case IDD_DLGINDSELSMARTDPC:
			// HYS-1937: TA-Smart Dp - 04 - Individual selection: left tab.
			pDlg = new CDlgIndSelSmartDpC();
			break;
			
		case IDD_DLGINDSEL6WAYVALVE:
			pDlg = new CDlgIndSel6WayValve();
			break;

		case IDD_DLGINDSELSV:
			pDlg = new CDlgIndSelSv();
			break;

		case IDD_DLGINDSELTRV:
			pDlg = new CDlgIndSelTrv();
			break;

		case IDD_DLGBATCHSELSEPARATOR:
			pDlg = new CDlgBatchSelSeparator();
			break;

		case IDD_DLGBATCHSELBV:
			pDlg = new CDlgBatchSelBv();
			break;

		case IDD_DLGBATCHSELDPC:
			pDlg = new CDlgBatchSelDpC();
			break;

		case IDD_DLGBATCHSELBCV:
			pDlg = new CDlgBatchSelBCv();
			break;

		case IDD_DLGBATCHSELPICV:
			pDlg = new CDlgBatchSelPICv();
			break;

		case IDD_DLGBATCHSELSMARTCONTROLVALVE:
			pDlg = new CDlgBatchSelSmartControlValve();
			break;

		case IDD_DLGBATCHSELSMARTDPC:
			pDlg = new CDlgBatchSelSmartDpC();
			break;

		case IDD_DLGWIZARDSELPM:
			pDlg = new CDlgWizardSelPM();
			break;

	}

	if( NULL == pDlg )
	{
		ASSERT( 0 );
		return NULL;
	}
		
	pDlg->Create( nDialogID, this );

	ASSERT( IsWindow( pDlg->m_hWnd ) );

	pDlg->SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );

	return pDlg;
}

CDlgSelectionBase *CDlgLeftTabSelManager::_SelectPage( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	if( NULL == m_clAllLeftDialogs.GetLeftTabDialog( eProductSubCategory, eProductSelectionMode ) )
	{
		return NULL;
	}

	// If it's same selection as the current one...
	if( eProductSubCategory == m_clAllLeftDialogs.GetProductSubCategory() 
			&& eProductSelectionMode == m_clAllLeftDialogs.GetProductSelectionMode() )
	{
		if( NULL != m_clAllLeftDialogs.GetLeftTabDialog() )
		{
			// We just activate the correct dialog.
			m_clAllLeftDialogs.GetLeftTabDialog()->ActivateLeftTabDialog();
			return m_clAllLeftDialogs.GetLeftTabDialog();
		}
		else
		{
			// Should never arrive here!
			ASSERT( 0 );
			return NULL;
		}
	}

	if( ProductSubCategory::PSC_Undefined != m_clAllLeftDialogs.GetProductSubCategory()
			&& NULL != m_clAllLeftDialogs.GetLeftTabDialog() )
	{
		m_clAllLeftDialogs.GetLeftTabDialog()->LeaveLeftTabDialog();
		m_clAllLeftDialogs.GetLeftTabDialog()->ShowWindow( SW_HIDE );
		m_clAllLeftDialogs.GetLeftTabDialog()->EnableWindow( FALSE );
	}
	
	m_clAllLeftDialogs.SetProductSubCategory( eProductSubCategory );
	m_clAllLeftDialogs.SetProductSelectionMode( eProductSelectionMode );

	m_clAllLeftDialogs.GetLeftTabDialog()->ShowWindow( SW_SHOW );
	m_clAllLeftDialogs.GetLeftTabDialog()->EnableWindow( TRUE );
	m_clAllLeftDialogs.GetLeftTabDialog()->ActivateLeftTabDialog();

	// Resize the dialog to the correct position.
	CRect rect;
	m_clAllLeftDialogs.GetLeftTabDialog()->GetClientRect( &rect );
	
	// Allow the 'DlgLeftTabBase' to draw the corporate logo at the bottom of the dialog.
	ChangeStaticGroupSize( rect );

	return m_clAllLeftDialogs.GetLeftTabDialog();
}

void CDlgLeftTabSelManager::_CheckScrollBar( void )
{
	// When calling 'SetScrollInfo' below, we ask to refresh (third argument set to TRUE) and Windows internally calls the 'OnSize'
	// and this method call '_CheckScrollBar'. If we set 'FALSE', the scrollbars are not refreshed.
	if( true == m_bBypass )
	{
		return;
	}

	if( NULL == m_clAllLeftDialogs.GetLeftTabDialog() )
	{
		return;
	}

	// Reset first scroll if exist.
	CRect rectWin;
	m_clAllLeftDialogs.GetLeftTabDialog()->GetWindowRect( &rectWin );
	ScreenToClient( &rectWin );

	ScrollWindow( -min( 0, rectWin.left ), -min( 0, rectWin.top ) );

	// Size of top part of child window.
	CRect rectTopChild;
	m_clAllLeftDialogs.GetLeftTabDialog()->GetClientRect( &rectTopChild );

	// Child window will contains also the corporate logo in the bottom (see CDlgLeftTabBase::OnPaint for details).
	CRect rectBitmap = ( NULL != m_Bmp.GetSafeHandle() ) ? m_Bmp.GetSizeImage() : CRect( 0, 0, 0, 0 );
	CRect rectChild( rectTopChild.left, rectTopChild.top, rectTopChild.right, rectTopChild.bottom + _DLGLEFTTABBASE_LOGOYSHIFT + rectBitmap.Height() );

	// Remark: - iRealcx and iRealcy are width and height of available area included scrollbar if exists.
	//         - cy and cy are width and height of available area without scrollbar if exists.

	// Take in count height/width of scroll bar.
	CRect rectClient;
	GetClientRect( &rectClient );
	int iRealcx = ( true == m_bVScrollBarVisible ) ? ( rectClient.Width() + GetSystemMetrics( SM_CXVSCROLL ) ) : rectClient.Width();
	int iRealcy = ( true == m_bHScrollBarVisible ) ? ( rectClient.Height() + GetSystemMetrics( SM_CXHSCROLL ) ) : rectClient.Height();

	// Check if we need scroll bar.
	bool fNeedHScrollBar = ( rectChild.Width() > iRealcx );
	bool fNeedVScrollBar = ( rectChild.Height() > iRealcy );

	// Check if when added horizontal bar, we don't need vertical bar.
	if( ( true == fNeedHScrollBar ) && ( false == fNeedVScrollBar ) && rectChild.Height() > iRealcy - GetSystemMetrics( SM_CXHSCROLL ) )
	{
		fNeedVScrollBar = true;
	}

	// Check if when added vertical bar, we don't need horizontal bar.
	if( (true == fNeedVScrollBar ) && ( false == fNeedHScrollBar ) && rectChild.Width() > iRealcx - GetSystemMetrics( SM_CXVSCROLL ) )
	{
		fNeedHScrollBar = true;
	}

	// If we need horizontal scroll and before there is no bar, we adjust cy to have correct height available
	if( true == fNeedHScrollBar && false == m_bHScrollBarVisible )
	{
		rectClient.bottom -= GetSystemMetrics(SM_CXVSCROLL);
	}

	// If we need vertical scroll and before there is no bar, we adjust cx to have correct width available
	if( true == fNeedVScrollBar && false == m_bVScrollBarVisible )
	{
		rectClient.right -= GetSystemMetrics(SM_CXHSCROLL);
	}

	m_bHScrollBarVisible = fNeedHScrollBar;
	m_bVScrollBarVisible = fNeedVScrollBar;

	// Remark: for 'm_fByPass', see remark above.
	m_bBypass = true;
	if( true == fNeedHScrollBar )
	{
		ShowScrollBar( SB_HORZ, TRUE  );

		SCROLLINFO rScrollInfo;
		ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
		rScrollInfo.cbSize = sizeof(SCROLLINFO);
		rScrollInfo.fMask = SIF_ALL;
		rScrollInfo.nMin = 0;
		rScrollInfo.nMax = rectChild.Width();
		rScrollInfo.nPage = rectClient.Width();
		rScrollInfo.nPos = 0;
		SetScrollInfo( SB_HORZ, &rScrollInfo, TRUE );
	}
	else
	{
		ShowScrollBar( SB_HORZ, FALSE );
	}

	if( true == fNeedVScrollBar )
	{
		ShowScrollBar( SB_VERT, TRUE  );

		SCROLLINFO rScrollInfo;
		ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
		rScrollInfo.cbSize = sizeof(SCROLLINFO);
		rScrollInfo.fMask = SIF_ALL;
		rScrollInfo.nMin = 0;
		rScrollInfo.nMax = rectChild.Height();
		rScrollInfo.nPage = rectClient.Height();
		rScrollInfo.nPos = 0;
		SetScrollInfo( SB_VERT, &rScrollInfo, TRUE );
	}
	else
	{
		ShowScrollBar( SB_VERT, FALSE );
	}

	m_bBypass = false;
}

bool CDlgLeftTabSelManager::_UpdateBatchSelectionOutputDialog( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode, 
		bool bEnable, FullScreenMode eFullScreenMode )
{
	if( ProductSubCategory::PSC_Undefined == eProductSubCategory )
	{
		return false;
	}
	
	int iContextID = -1;

	if( false == GetToolsDockablePaneContextID( eProductSubCategory, eProductSelectionMode, iContextID ) || -1 == iContextID )
	{
		return false;
	}

	CDlgSelectionBase *pclDlgSelectionBase = m_clAllLeftDialogs.GetLeftTabDialog( eProductSubCategory, eProductSelectionMode );

	if( NULL == pclDlgSelectionBase )
	{
		return false;
	}
	
	bool bSwitchOccurs = false;
	CToolsDockablePane *pToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	
	if( NULL != pToolsDockablePane && NULL != pToolsDockablePane->GetSafeHwnd() )
	{
		CToolsDockablePane::ContextParameters *pclContextParameters = pToolsDockablePane->GetContextByID( iContextID );

		if( NULL == pclContextParameters )
		{
			return false;
		}
		
		if( true == bEnable )
		{
			// We enter in the batch selection mode.
			pclContextParameters->m_bIsPaneVisible = true;
			pclContextParameters->m_bIsPaneAutoHide = FALSE;
			pclContextParameters->m_bIsGripperEnabled = FALSE;
			pclContextParameters->m_bIsPaneFullScreen = ( FullScreenMode::FSM_False == eFullScreenMode ) ? false : true;
			pclContextParameters->m_bIsContextMenuEnabled = false;
			pclContextParameters->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
			pclContextParameters->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = true;

			// Activate the correct output in regards to the current page.
			if( NULL != pToolsDockablePane->GetpDlgBatchSelectionOutput() )
			{
				pToolsDockablePane->GetpDlgBatchSelectionOutput()->ActivateOutput( (int)eProductSubCategory );
			}

			// Apply this new context.
			pToolsDockablePane->RestoreContext( iContextID );

			m_pclMainRbnCBSelPipeSizeIDSaved = pMainFrame->GetRibbonComboSelPipeSizeID();
			pMainFrame->SetRibbonComboSelPipeSize( _T("") );
			pMainFrame->EnableRibbonComboSelPipeSize( false );
		}
		else if( false == bEnable )
		{
			if( ProductSubCategory::PSC_PM_ExpansionVessel == eProductSubCategory
					&& ProductSelectionMode_Individual == eProductSelectionMode )
			{
				pclContextParameters->m_bIsPaneVisible = true;
				pclContextParameters->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = true;
			}
			else
			{
				pclContextParameters->m_bIsPaneVisible = false;
				pclContextParameters->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
			}

			pclContextParameters->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;

			// Apply this new context.
			pToolsDockablePane->RestoreContext( iContextID );

			pMainFrame->SetRibbonComboSelPipeSize( m_pclMainRbnCBSelPipeSizeIDSaved );
			pMainFrame->EnableRibbonComboSelPipeSize( true );
		}

		bSwitchOccurs = true;
	}

	return bSwitchOccurs;
}

CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::CLeftTabDialogsByProductSubCategory()
{
	for( int iLoopProductSelectionMode = ProductSelectionMode_First; iLoopProductSelectionMode <= ProductSelectionMode_Last; iLoopProductSelectionMode++ )
	{
		m_arpLeftTabs.Add( NULL );
	}

	m_eCurrentProductSelectionMode = ProductSelectionMode_Individual;
	m_iContextID = -1;
}

CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::~CLeftTabDialogsByProductSubCategory()
{
	CleanAll();
}

void CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::CleanAll()
{
	if( ProductSelectionMode_Last + 1 == m_arpLeftTabs.GetCount() )
	{
		for( int iLoopProductSelectionMode = ProductSelectionMode_First; iLoopProductSelectionMode <= ProductSelectionMode_Last; iLoopProductSelectionMode++ )
		{
			if( NULL != m_arpLeftTabs[iLoopProductSelectionMode] )
			{
				delete m_arpLeftTabs[iLoopProductSelectionMode];
			}
		}
	
		m_arpLeftTabs.RemoveAll();
	}
}

void CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::AddLeftTabDialog( CDlgSelectionBase *pDlgSelectionBase, ProductSelectionMode eProductSelectionMode )
{
	m_arpLeftTabs[eProductSelectionMode] = pDlgSelectionBase;
}

CDlgSelectionBase *CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::GetLeftTabDialog( ProductSelectionMode eProductSelectionMode )
{
	return m_arpLeftTabs[eProductSelectionMode];
}

CDlgSelectionBase *CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::GetLeftTabDialog()
{
	return m_arpLeftTabs[m_eCurrentProductSelectionMode];
}

ProductSelectionMode CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::GetProductSelectionMode()
{
	return m_eCurrentProductSelectionMode;
}

void CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::SetProductSelectionMode( ProductSelectionMode eProductSelectionMode )
{
	m_eCurrentProductSelectionMode = eProductSelectionMode;
}

void CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::SetToolsDockablePaneContextID( int iContextID )
{
	if( NULL != m_arpLeftTabs.GetAt( m_eCurrentProductSelectionMode ) )
	{
		m_arpLeftTabs.GetAt( m_eCurrentProductSelectionMode )->SetToolsDockablePaneContextID( iContextID );
	}
}

void CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::SetToolsDockablePaneContextID( int iContextID, ProductSelectionMode eProductSelectionMode )
{ 
	if( NULL != m_arpLeftTabs.GetAt( eProductSelectionMode ) )
	{
		m_arpLeftTabs.GetAt( eProductSelectionMode )->SetToolsDockablePaneContextID( iContextID );
	}
}

int CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::GetToolsDockablePaneContextID()
{ 
	if( NULL != m_arpLeftTabs.GetAt( m_eCurrentProductSelectionMode ) )
	{
		return m_arpLeftTabs.GetAt( m_eCurrentProductSelectionMode )->GetToolsDockablePaneContextID();
	}
	else
	{
		return -1;
	}
}

int CDlgLeftTabSelManager::CLeftTabDialogsByProductSubCategory::GetToolsDockablePaneContextID( ProductSelectionMode eProductSelectionMode )
{ 
	if( NULL != m_arpLeftTabs.GetAt( eProductSelectionMode ) )
	{
		return m_arpLeftTabs.GetAt( eProductSelectionMode )->GetToolsDockablePaneContextID();
	}
	else
	{
		return -1;
	}
}
	
CDlgLeftTabSelManager::CAllLeftTabDialogs::CAllLeftTabDialogs()
{
	for( int iLoopSubCat = ProductSubCategory::PSC_First; iLoopSubCat <= ProductSubCategory::PSC_Last; iLoopSubCat++ )
	{
		m_arLeftDialogList.Add( new CLeftTabDialogsByProductSubCategory );
	}

	m_eCurrentProductSubCategory = PSC_Undefined;
}

CDlgLeftTabSelManager::CAllLeftTabDialogs::~CAllLeftTabDialogs()
{
	// Delete all dialogs.
	for( int iLoopType = ProductSubCategory::PSC_First; iLoopType <= ProductSubCategory::PSC_Last; iLoopType++ )
	{
		if( NULL != m_arLeftDialogList[iLoopType] )
		{
			m_arLeftDialogList[iLoopType]->CleanAll();
			delete m_arLeftDialogList[iLoopType];
		}
	}

	m_arLeftDialogList.RemoveAll();
}

void CDlgLeftTabSelManager::CAllLeftTabDialogs::AddLeftTabDialog( CDlgSelectionBase *pDlgSelectionBase, ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	if( NULL != m_arLeftDialogList[eProductSubCategory] )
	{
		m_arLeftDialogList[eProductSubCategory]->AddLeftTabDialog( pDlgSelectionBase, eProductSelectionMode );
	}
}

ProductSubCategory CDlgLeftTabSelManager::CAllLeftTabDialogs::GetProductSubCategory()
{
	return m_eCurrentProductSubCategory;
}

CDlgSelectionBase *CDlgLeftTabSelManager::CAllLeftTabDialogs::GetLeftTabDialog( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	if( NULL != m_arLeftDialogList[eProductSubCategory] )
	{
		return m_arLeftDialogList[eProductSubCategory]->GetLeftTabDialog( eProductSelectionMode );
	}
	else
	{
		return NULL;
	}
}

CDlgSelectionBase *CDlgLeftTabSelManager::CAllLeftTabDialogs::GetLeftTabDialog( ProductSubCategory eProductSubCategory )
{
	if( NULL != m_arLeftDialogList[eProductSubCategory] )
	{
		return m_arLeftDialogList[eProductSubCategory]->GetLeftTabDialog();
	}
	else
	{
		return NULL;
	}
}

CDlgSelectionBase *CDlgLeftTabSelManager::CAllLeftTabDialogs::GetLeftTabDialog()
{
	if( NULL != m_arLeftDialogList[m_eCurrentProductSubCategory] )
	{
		return m_arLeftDialogList[m_eCurrentProductSubCategory]->GetLeftTabDialog();
	}
	else
	{
		return NULL;
	}
}

bool CDlgLeftTabSelManager::CAllLeftTabDialogs::IsAtLeastOneProductSelectionModeExist( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	bool bExist = false;

	if( NULL != GetLeftTabDialog( eProductSubCategory, eProductSelectionMode ) )
	{
		bExist = true;
	}

	return bExist;
}

ProductSelectionMode CDlgLeftTabSelManager::CAllLeftTabDialogs::GetProductSelectionMode( ProductSubCategory eProductSubCategory )
{
	if( NULL != m_arLeftDialogList[eProductSubCategory] )
	{
		return m_arLeftDialogList[eProductSubCategory]->GetProductSelectionMode();
	}
	else
	{
		return ProductSelectionMode::ProductSelectionMode_Last;
	}
}

ProductSelectionMode CDlgLeftTabSelManager::CAllLeftTabDialogs::GetProductSelectionMode()
{
	if( NULL != m_arLeftDialogList[m_eCurrentProductSubCategory] )
	{
		return m_arLeftDialogList[m_eCurrentProductSubCategory]->GetProductSelectionMode();
	}
	else
	{
		return ProductSelectionMode::ProductSelectionMode_Last;
	}
}

void CDlgLeftTabSelManager::CAllLeftTabDialogs::SetProductSubCategory( ProductSubCategory eProductSubCategory )
{
	m_eCurrentProductSubCategory = eProductSubCategory;
}

void CDlgLeftTabSelManager::CAllLeftTabDialogs::SetProductSelectionMode( ProductSelectionMode eProductSelectionMode )
{
	if( NULL != m_arLeftDialogList[m_eCurrentProductSubCategory] )
	{
		m_arLeftDialogList[m_eCurrentProductSubCategory]->SetProductSelectionMode( eProductSelectionMode );
	}
}

void CDlgLeftTabSelManager::CAllLeftTabDialogs::SetToolsDockablePaneContextID( int iContextID )
{
	if( NULL != m_arLeftDialogList[m_eCurrentProductSubCategory] )
	{
		m_arLeftDialogList[m_eCurrentProductSubCategory]->SetToolsDockablePaneContextID( iContextID );
	}
}

void CDlgLeftTabSelManager::CAllLeftTabDialogs::SetToolsDockablePaneContextID( int iContextID, ProductSubCategory eProductSubCategory )
{
	if( NULL != m_arLeftDialogList[eProductSubCategory] )
	{
		m_arLeftDialogList[eProductSubCategory]->SetToolsDockablePaneContextID( iContextID );
	}
}

void CDlgLeftTabSelManager::CAllLeftTabDialogs::SetToolsDockablePaneContextID( int iContextID, ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	if( NULL != m_arLeftDialogList[eProductSubCategory] )
	{
		m_arLeftDialogList[eProductSubCategory]->SetToolsDockablePaneContextID( iContextID, eProductSelectionMode );
	}
}

int CDlgLeftTabSelManager::CAllLeftTabDialogs::GetToolsDockablePaneContextID()
{
	if( NULL != m_arLeftDialogList[m_eCurrentProductSubCategory] )
	{
		return m_arLeftDialogList[m_eCurrentProductSubCategory]->GetToolsDockablePaneContextID();
	}
	else
	{
		return -1;
	}
}

int CDlgLeftTabSelManager::CAllLeftTabDialogs::GetToolsDockablePaneContextID( ProductSubCategory eProductSubCategory )
{
	if( NULL != m_arLeftDialogList[eProductSubCategory] )
	{
		return m_arLeftDialogList[eProductSubCategory]->GetToolsDockablePaneContextID();
	}
	else
	{
		return -1;
	}
}

int CDlgLeftTabSelManager::CAllLeftTabDialogs::GetToolsDockablePaneContextID( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode )
{
	if( NULL != m_arLeftDialogList[eProductSubCategory] )
	{
		return m_arLeftDialogList[eProductSubCategory]->GetToolsDockablePaneContextID( eProductSelectionMode );
	}
	else
	{
		return -1;
	}
}
