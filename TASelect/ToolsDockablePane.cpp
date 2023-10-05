#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "DlgOutput.h"
#include "DlgHMCompilationOutput.h"
#include "DlgBatchSelectionOutput.h"
#include "DlgPMGraphsOutput.h"
#include "ToolsDockablePane.h"

#include "HydroMod.h"
#include "HMPipes.h"
#include "TerminalUnit.h"
#include "SSheetPanelCirc2.h"
#include "SSheetLDSumm.h"
#include "SSheetLogData.h"
#include "SSheetQuickMeas.h"
#include "TASelectDoc.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewProj.h"

IMPLEMENT_DYNAMIC( CToolsDockablePane, CDockablePane )

CToolsDockablePane::CToolsDockablePane()
{
	m_pDlgHMCompilationOutput = NULL;
	m_pDlgBatchSelectionOutput = NULL;
	m_pDlgPMGraphsOutput = NULL;
	m_bInitialized = false;
	m_bContextMenuEnabled = true;
	m_bBlockSavingState = false;
	m_iCurrentContextID = -1;
}

CToolsDockablePane::~CToolsDockablePane()
{
	if( m_mapOutputList.size() > 0 )
	{
		for( iterOutputList iter = m_mapOutputList.begin(); iter != m_mapOutputList.end(); iter++ )
		{
			if( NULL != iter->second )
			{
				if( NULL != iter->second->GetSafeHwnd() )
				{
					iter->second->DestroyWindow();
				}

				delete iter->second;
			}
		}

		m_mapOutputList.clear();
	}

	if( m_arContextList.GetCount() > 0 )
	{
		for( int iLoop = 0; iLoop < m_arContextList.GetCount(); iLoop++ )
		{
			if( NULL != m_arContextList.GetAt( iLoop ) )
			{
				delete m_arContextList.GetAt( iLoop );
			}
		}

		m_arContextList.RemoveAll();
	}
}

CToolsDockablePane::ContextParameters *CToolsDockablePane::RegisterContext()
{
	ContextParameters *pContext = new ContextParameters;

	if( NULL == pContext )
	{
		ASSERT( 0 );
		return NULL;
	}

	pContext->m_nTimeOutBeforeAutoHide = 700;
	pContext->m_lHeight = 100;
	pContext->m_dwAlignement = CBRS_ALIGN_BOTTOM;
	pContext->m_iContextID = m_arContextList.Add( pContext );

	return pContext;
}

bool CToolsDockablePane::RestoreContext( int iContextID, bool bForce, bool bMainFrameResized )
{
	if( false == m_bInitialized || iContextID < 0 || iContextID >= m_arContextList.GetCount() 
		|| NULL == m_arContextList.GetAt( iContextID ) || NULL == pMainFrame->GetSpecificView( (CMainFrame::RightViewList)m_arContextList.GetAt( iContextID )->m_iLinkedRightView )  )
	{
		return false;
	}

	if( false == bForce && iContextID == m_iCurrentContextID )
	{
		return true;
	}

	ContextParameters *pPrevContext = NULL;

	if( -1 != m_iCurrentContextID )
	{
		pPrevContext = m_arContextList.GetAt( m_iCurrentContextID );
	}
	
	ContextParameters *pNewContext = m_arContextList.GetAt( iContextID );
	m_iCurrentContextID = iContextID;

	// If the new state is hidden.
	if( false == pNewContext->m_bIsPaneVisible )
	{
		// We can simply hide the pane and exit.
		ShowPane( FALSE, FALSE, FALSE );
		return true;
	}

	// First step.
	if( NULL != pPrevContext )
	{
		m_bBlockSavingState = true;

		if( false == pPrevContext->m_bIsPaneVisible )
		{
			// If previous state was hidden, we show the pane.
			ShowPane( TRUE, FALSE, TRUE );

			// HYS-1048: Before to get in the non visible state, the pane can be in auto-hide mode. We force the pane
			// to be in normal mode to be sure.
			SetAutoHideMode(FALSE, pNewContext->m_dwAlignement);
		}
		else if( TRUE == pPrevContext->m_bIsPaneAutoHide && FALSE == pNewContext->m_bIsPaneAutoHide )
		{
			// If previous state was auto-hide, we unset this mode.
			SetAutoHideMode( FALSE, pNewContext->m_dwAlignement );
		}
		else if( true == pPrevContext->m_bIsPaneFullScreen && false == pNewContext->m_bIsPaneFullScreen )
		{
			// If previous state was full screen, we reactivate the slider.
			CPaneDivider *pclPaneDivider = GetDefaultPaneDivider();
			pclPaneDivider->EnableWindow( TRUE );
			pclPaneDivider->ShowWindow( SW_SHOW );

			// To allow to go in the auto-hide mode.
			SetControlBarStyle( AFX_CBRS_RESIZE | AFX_CBRS_AUTOHIDE );
		}

		m_bBlockSavingState = false;
	}

	CPaneDivider *pclPanelDivider = GetDefaultPaneDivider();

	if( NULL == pclPanelDivider )
	{
		return false;
	}

	// Modify height.
	if( false == pNewContext->m_bIsPaneFullScreen )
	{
		// In a normal mode, we will move the slider to adapt to the new height.
		CRect rectPaneDivider;
		pclPanelDivider->GetWindowRect( &rectPaneDivider );

		CSize clMinSize;
		GetMinSize( clMinSize );

		if( pNewContext->m_lHeight < clMinSize.cy )
		{
			pNewContext->m_lHeight = clMinSize.cy;
		}

		CRect rectWinPane;
		GetWindowRect( &rectWinPane );

		if( NULL != pPrevContext && TRUE == pPrevContext->m_bIsPaneAutoHide && TRUE == pNewContext->m_bIsPaneAutoHide )
		{
			// HYS-1059: If previous context was in auto-hide mode and also the new one, we don't play with the panel divider to avoid to show the
			// dockable pane and have flicking effect.
			if( CBRS_ALIGN_BOTTOM == GetCurrentAlignment() )
			{
				::SetWindowPos( GetSafeHwnd(), NULL, -1, -1, rectWinPane.Width(), pNewContext->m_lHeight, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
			}
		}
		else
		{
			// HYS-1046
			if( 0 == rectWinPane.Height() - pNewContext->m_lHeight )
			{
				forceResize();
			}
			else if( CBRS_ALIGN_BOTTOM == GetCurrentAlignment() )
			{
				pclPanelDivider->Move( CPoint( 0, rectWinPane.Height() - pNewContext->m_lHeight ), TRUE );
			}
			else
			{
				// When docked at the top, obviously the slider is at the bottom!
				pclPanelDivider->Move( CPoint( 0, pNewContext->m_lHeight - rectWinPane.Height() ), TRUE );
			}
		}
	}
	else
	{
		// In full screen, we disable the slider and we take all the height of the right view.
		CRect rectMainFrame;
		pMainFrame->GetWindowRect( &rectMainFrame );

		CMFCRibbonBar *pclRibbonBar = pMainFrame->GetRibbonBar();

		if( NULL == pclRibbonBar )
		{
			ASSERTA_RETURN( false );
		}

		CRect rectRibbonBar;
		pclRibbonBar->GetWindowRect( &rectRibbonBar );

		CMFCStatusBar *pclStatusBar = pMainFrame->GetStatusBar();

		if( NULL == pclStatusBar )
		{
			ASSERTA_RETURN( false );
		}

		CRect rectStatusBar;
		pclStatusBar->GetWindowRect( &rectStatusBar );

		CPaneDivider *pclPaneDivider = GetDefaultPaneDivider();

		if( NULL == pclPaneDivider )
		{
			ASSERTA_RETURN( false );
		}

		CRect rectPaneDivider;
		pclPaneDivider->GetWindowRect( &rectPaneDivider );

		// To avoid the possibility to resize the dockable pane when it is in full screen mode.
		pclPaneDivider->EnableWindow( FALSE );
		pclPaneDivider->ShowWindow( SW_HIDE );

		int iAvailableHeight = rectMainFrame.Height() - rectRibbonBar.Height() - rectStatusBar.Height() - rectPaneDivider.Height();
		pNewContext->m_lHeight = iAvailableHeight;

		// HYS-1087: We have to make a difference between a state change and a resize of the mainframe.
		if( false == bMainFrameResized )
		{
			CRect rectWinPane;
			GetWindowRect( &rectWinPane );

			// HYS-1046
			if( 0 == rectWinPane.Height() - iAvailableHeight )
			{
				forceResize();
			}
			else if( CBRS_ALIGN_BOTTOM == GetCurrentAlignment() )
			{
				pclPaneDivider->Move( CPoint( 0, rectWinPane.Height() - iAvailableHeight ), TRUE );
			}
			else
			{
				// When docked at the top, obviously the slider is at the bottom!
				pclPaneDivider->Move( CPoint( 0, pNewContext->m_lHeight - rectWinPane.Height() ), TRUE );
			}

			if( NULL != pPrevContext && true == pPrevContext->m_bIsPaneFullScreen )
			{
				// If previous state was already in full screen, we will not have 'OnSize' called because height will not change.

				// Tab control should cover the whole client area.
				CRect rectDockClient;
				GetClientRect( &rectDockClient );

				m_wndTabs.SetWindowPos( NULL, -1, -1, rectDockClient.Width(), rectDockClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
			
				CRect rectTabClient;
				m_wndTabs.GetWndArea( rectTabClient );

				for( iterOutputList iter = m_mapOutputList.begin(); iter != m_mapOutputList.end(); ++iter )
				{
					// For the dialog to resize itself by directly send the 'WM_SIZE' message.
					iter->second->SendMessage( WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM( rectTabClient.Width(), rectTabClient.Height() ) );
				}
			}
		}
		else
		{
			CRect rectLeftTab;
			pDlgLTtabctrl->GetWindowRect( &rectLeftTab );
			int iAvailableWidth = rectMainFrame.Width() - rectLeftTab.Width() - rectPaneDivider.Height();

			// Tab control should cover the whole client area.
			CRect rectDockClient;
			GetClientRect( &rectDockClient );

			CRect rectDockWin;
			GetWindowRect( &rectDockWin );
			pMainFrame->ScreenToClient( &rectDockWin );

			SetWindowPos( NULL, rectDockWin.left, rectDockWin.top, iAvailableWidth, iAvailableHeight, SWP_NOACTIVATE | SWP_NOZORDER );

			CRect rectTabClient;
			m_wndTabs.GetWndArea( rectTabClient );

			for( iterOutputList iter = m_mapOutputList.begin(); iter != m_mapOutputList.end(); ++iter )
			{
				// For the dialog to resize itself by directly send the 'WM_SIZE' message.
				iter->second->SendMessage( WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM( rectTabClient.Width(), rectTabClient.Height() ) );
			}
		}

		// To avoid to be able to undock (floating) the dialog when in full screen.
		SetControlBarStyle( 0 );
	}

	for( int iLoopTab = TabID::OutputID_First; iLoopTab < TabID::OutputID_Last; ++iLoopTab )
	{
		_ShowTab( (TabID)iLoopTab, pNewContext->m_arIsTabVisible[iLoopTab], false );
	}

	// Check now if we need to set the pane in auto-hide mode
	if( TRUE == pNewContext->m_bIsPaneAutoHide )
	{
		SetAutoHideMode( TRUE, GetCurrentAlignment(), NULL, FALSE );
	}

	m_bContextMenuEnabled = pNewContext->m_bIsContextMenuEnabled;

	return true;
}

CToolsDockablePane::ContextParameters *CToolsDockablePane::GetContextByID( int iContextID )
{
	if( false == m_bInitialized || iContextID < 0 || iContextID >= m_arContextList.GetCount() 
			|| NULL == m_arContextList.GetAt( iContextID ) )
	{
		return NULL;
	}

	return m_arContextList.GetAt( iContextID );
}

bool CToolsDockablePane::ChangeDlgOutputHeight( int iContextID, long lNewHeight )
{
	if( false == m_bInitialized || iContextID < 0 || iContextID >= m_arContextList.GetCount() 
		|| NULL == m_arContextList.GetAt( iContextID ) )
	{
		return false;
	}

	// Add the pane divider (Slider) height if exist.
	CPaneDivider *pclPaneDivider = GetDefaultPaneDivider();

	if( NULL != pclPaneDivider )
	{
		CRect rectPaneDivider;
		pclPaneDivider->GetWindowRect( &rectPaneDivider );
		lNewHeight += rectPaneDivider.Height();
	}

	// Add the caption height if exist.
	lNewHeight += GetCaptionHeight();

	// Add the tabs height if exist.
	lNewHeight += m_wndTabs.GetTabsHeight();

	m_arContextList.GetAt( m_iCurrentContextID )->m_lHeight = lNewHeight;
	return RestoreContext( iContextID, true );
}

void CToolsDockablePane::BlockSavingDockSize( bool bBlock )
{
	if( NULL == pDlgLTtabctrl )
	{
		return;
	}

	int iCurrentTab = (int)pDlgLTtabctrl->GetActiveTab();
	m_mapFreezeHeightSaving[iCurrentTab] = bBlock;
}

void CToolsDockablePane::OnMainFrameSizing( UINT bwSide, LPRECT pRect )
{
	if( NULL == pDlgLTtabctrl || true == m_bBlockSavingState || -1 == m_iCurrentContextID 
			|| m_iCurrentContextID >= m_arContextList.GetCount() || NULL == m_arContextList.GetAt( m_iCurrentContextID ) )
	{
		return;
	}

	ContextParameters *pclContext = m_arContextList.GetAt( m_iCurrentContextID );

	if( false == pclContext->m_bIsPaneVisible )
	{
		// Do nothing if the pane is not visible.
		return;
	}

	/*
	CRect rectSizing = *pRect;
	CRect rectMainFrame;
	pMainFrame->GetWindowRect( &rectMainFrame );

	bool bIncrease = ( rectSizing.Height() > rectMainFrame.Height() ) ? true : false;
	
	if( true == bIncrease )
	{
		CMFCRibbonBar *pclRibbonBar = pMainFrame->GetRibbonBar();
		CRect rectRibbonBar;
		pclRibbonBar->GetWindowRect( &rectRibbonBar );

		CMFCStatusBar *pclStatusBar = pMainFrame->GetStatusBar();
		CRect rectStatusBar;
		pclStatusBar->GetWindowRect( &rectStatusBar );

		long lAvailableHeightAfter = rectSizing.Height() - rectRibbonBar.Height() - rectStatusBar.Height();

		if( false == pclContext->m_bIsPaneFullScreen )
		{
			// Dockable pane is not in a full screen mode.
			// Remark: this allow us to not increase the dockable height above the initial height.
			pclContext->m_lHeight = min( lAvailableHeightAfter, pclContext->m_lHeight );
		}
		else
		{
			// Dockable pane is in a full screen mode.
			// Remark: this allow us to increase the dockable height to perfectly match the available height.
			pclContext->m_lHeight = lAvailableHeightAfter;
		}

		RestoreContext( m_iCurrentContextID, true );

		// What happens when decreasing? If available height becomes lower than the dockable pane height, the pane is automatically
		// resized by Windows and the method 'OnSize' is called. If we want to keep original size, we must not save in 'm_mapDockSizeByView'.
		// This is why we have the 'BlockSavingDockSize' method.
	}
	*/

	RestoreContext( m_iCurrentContextID, true, true );
}

void CToolsDockablePane::OnMainFrameSize( UINT nType, int cx, int cy )
{
	if( NULL == pDlgLTtabctrl || true == m_bBlockSavingState || -1 == m_iCurrentContextID 
			|| m_iCurrentContextID >= m_arContextList.GetCount() || NULL == m_arContextList.GetAt( m_iCurrentContextID ) )
	{
		return;
	}

	ContextParameters *pclContext = m_arContextList.GetAt( m_iCurrentContextID );

	if( false == pclContext->m_bIsPaneVisible )
	{
		// Do nothing if the pane is not visible.
		return;
	}

	RestoreContext( m_iCurrentContextID, true, true );
}

CMFCAutoHideBar *CToolsDockablePane::SetAutoHideMode( BOOL bMode, DWORD dwAlignment, CMFCAutoHideBar *pCurrAutoHideBar, BOOL bUseTimer )
{
	// Because 'SetAutoHideMode' can be dynamically called by the program (when 'RestoreContext' is called for example) or when user clicks on 
	// the pin button, we have to save the state of the pane in the case user choose to pin/unpin the pane.
	if( false == m_bBlockSavingState && -1 != m_iCurrentContextID && NULL != GetContextByID( m_iCurrentContextID ) )
	{
		GetContextByID( m_iCurrentContextID )->m_bIsPaneAutoHide = bMode;
	}
	
	// HYS-1059: When leaving auto hide mode, we want that the dockable pane take the right height.
	if( FALSE == bMode )
	{
		int iNewBottom = m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.top + m_arContextList.GetAt( m_iCurrentContextID )->m_lHeight;
		m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.bottom = iNewBottom;
	}

	return CDockablePane::SetAutoHideMode( bMode, dwAlignment, pCurrAutoHideBar, bUseTimer );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CToolsDockablePane, CDockablePane )
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CToolsDockablePane::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( -1 == CDockablePane::OnCreate( lpCreateStruct ) )
	{
		return -1;
	}

	SetControlBarStyle( AFX_CBRS_FLOAT | AFX_CBRS_AUTOHIDE | AFX_CBRS_RESIZE );

	// Create tabs window.
	if( FALSE == m_wndTabs.Create( CMFCTabCtrl::STYLE_3D, CRect( 0, 0, 0, 0 ), this, 1 ) )
	{
		TRACE0( "Failed to create output tab window\n" );
		return -1;
	}

	m_mapOutputCaption[OutputID_HMCompilation] = IDS_TOOLSDOCKABLEPANEOUTPUT_CAPTION;
	m_mapOutputCaption[OutputID_BatchSelection] = IDS_TOOLSDOCKABLEPANEBSOUTPUT_CAPTION;
	m_mapOutputCaption[OutputID_PMGraphs] = IDS_TOOLSDOCKABLEPANEPMOUTPUT_CAPTION;

	m_pDlgHMCompilationOutput = new CDlgHMCompilationOutput();

	if( NULL == m_pDlgHMCompilationOutput )
	{
		return -1;
	}

	// Create dialog HM Compilation output.
	if( FALSE == m_pDlgHMCompilationOutput->Create( CDlgHMCompilationOutput::IDD, &m_wndTabs ) )
	{
		TRACE0( "Failed to create hydraulic module compilation output tab window\n" );
		delete m_pDlgHMCompilationOutput;
		m_pDlgHMCompilationOutput = NULL;
		return -1;
	}

	m_mapOutputList[OutputID_HMCompilation] = m_pDlgHMCompilationOutput;
	
	// Attach list windows to tab.
	m_wndTabs.AddTab( m_mapOutputList[OutputID_HMCompilation],TASApp.LoadLocalizedString( m_mapOutputCaption[OutputID_HMCompilation] ), (UINT)0 );

	m_pDlgBatchSelectionOutput = new CDlgBatchSelectionOutput();

	if( NULL == m_pDlgBatchSelectionOutput )
	{
		return -1;
	}

	if( FALSE == m_pDlgBatchSelectionOutput->Create( CDlgBatchSelectionOutput::IDD, &m_wndTabs ) )
	{
		TRACE0( "Failed to create batch selection output tab window\n" );
		delete m_pDlgBatchSelectionOutput;
		m_pDlgBatchSelectionOutput = NULL;
		return -1;
	}
	
	m_mapOutputList[OutputID_BatchSelection] = m_pDlgBatchSelectionOutput;
	
	// Attach list windows to tab.
	m_wndTabs.AddTab( m_mapOutputList[OutputID_BatchSelection],TASApp.LoadLocalizedString( m_mapOutputCaption[OutputID_BatchSelection] ), (UINT)0 );
	m_wndTabs.ShowTab( m_wndTabs.GetTabFromHwnd( m_mapOutputList[OutputID_BatchSelection]->GetSafeHwnd() ), FALSE, FALSE, FALSE );

	m_pDlgPMGraphsOutput = new CDlgPMGraphsOutput();

	if( NULL == m_pDlgPMGraphsOutput )
	{
		return -1;
	}

	if( FALSE == m_pDlgPMGraphsOutput->Create( CDlgPMGraphsOutput::IDD, &m_wndTabs ) )
	{
		TRACE0( "Failed to create pressurisation mainteance graphs output tab window\n" );
		delete m_pDlgPMGraphsOutput;
		m_pDlgPMGraphsOutput = NULL;
		return -1;
	}
	
	m_mapOutputList[OutputID_PMGraphs] = m_pDlgPMGraphsOutput;
	
	// Attach list windows to tab.
	m_wndTabs.AddTab( m_mapOutputList[OutputID_PMGraphs],TASApp.LoadLocalizedString( m_mapOutputCaption[OutputID_PMGraphs] ), (UINT)0 );
	m_wndTabs.ShowTab( m_wndTabs.GetTabFromHwnd( m_mapOutputList[OutputID_PMGraphs]->GetSafeHwnd() ), FALSE, FALSE, FALSE );
	
	// Resize dialog to dockable pane.
	CRect rectClient;
	m_wndTabs.GetWndArea( rectClient );
	
	for( iterOutputList iter = m_mapOutputList.begin(); iter != m_mapOutputList.end(); ++iter )
	{
		iter->second->SetWindowPos( NULL, -1, -1, rectClient.Width(), rectClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
	}

	return 0;
}

void CToolsDockablePane::OnSize( UINT nType, int cx, int cy )
{
	CDockablePane::OnSize( nType, cx, cy );

	if( true == m_bBlockSavingState )
	{
		return;
	}

	// Tab control should cover the whole client area.
	m_wndTabs.SetWindowPos( NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
	CRect rectClient;
	m_wndTabs.GetWndArea( rectClient );

	for( iterOutputList iter = m_mapOutputList.begin(); iter != m_mapOutputList.end(); ++iter )
	{
		iter->second->SetWindowPos( NULL, -1, -1, rectClient.Width(), rectClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
	}

	if( true == m_bInitialized && NULL != pDlgLTtabctrl )
	{
		// Remark: 'm_mapFreezeHeightSaving' is set to 'true' when 'OnSize' is called by the system because the user decreases the 
		//         main frame size. In that case we must not update 'm_mapDockSizeByView' to allow us to keep the original height. 
		//         In case of user increases the mainframe size, this allows us to block dockable pane height to its original size.
		int iCurrentTab = (int)pDlgLTtabctrl->GetActiveTab();

		if( m_iCurrentContextID >= 0 && m_iCurrentContextID < m_arContextList.GetCount() 
				&& NULL != m_arContextList.GetAt( m_iCurrentContextID ) && false == m_arContextList.GetAt( m_iCurrentContextID )->m_bIsPaneFullScreen 
				&& false == m_mapFreezeHeightSaving[iCurrentTab] )
		{
			CRect rect;
			GetWindowRect( &rect );
			m_arContextList.GetAt( m_iCurrentContextID )->m_lHeight = rect.Height();
		}
	}
}

void CToolsDockablePane::OnAfterDock( CBasePane *pBar, LPCRECT lpRect, AFX_DOCK_METHOD dockMethod )
{
	CDockablePane::OnAfterDock( pBar, lpRect, dockMethod );

	if( true == m_bBlockSavingState )
	{
		return;
	}

	CRect rect;
	GetWindowRect( &rect );

	if( false == m_bInitialized )
	{
		for( int i = (int)CMyMFCTabCtrl::TabIndex::etiProj; i < (int)CMyMFCTabCtrl::TabIndex::etiLast; i++ )
		{
			m_mapFreezeHeightSaving[i] = false;
			m_mapHeightToRestore[i] = rect.Height();
		}

		m_bInitialized = true;
		SetMinSize( CSize( 0, DLGTDP_MINHEIGHT ) );
	}
	else if( -1 != m_iCurrentContextID && m_iCurrentContextID < m_arContextList.GetCount() 
			&& NULL != m_arContextList.GetAt( m_iCurrentContextID ) )
	{
		RestoreContext( m_iCurrentContextID, true );
	}

	CRect rectClient;
	m_wndTabs.GetWndArea( rectClient );
	
	for( iterOutputList iter = m_mapOutputList.begin(); iter != m_mapOutputList.end(); ++iter )
	{
		iter->second->SetWindowPos( NULL, -1, -1, rectClient.Width(), rectClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
	}
}

BOOL CToolsDockablePane::OnShowControlBarMenu( CPoint point )
{
	// HYS-1348 : If we right click on the graph we don't show the context menu.
	if( NULL != m_pDlgPMGraphsOutput && NULL != m_pDlgPMGraphsOutput->GetTChartDialog() )
	{
		CRect rectGraphOutPut;
		m_pDlgPMGraphsOutput->GetTChartDialog()->GetClientRect( &rectGraphOutPut );
		m_pDlgPMGraphsOutput->GetTChartDialog()->ClientToScreen( &rectGraphOutPut );
		if( TRUE == rectGraphOutPut.PtInRect( point ) )
		{
			return FALSE;
		}
	}
	if( false == m_bContextMenuEnabled )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		ClientToScreen( &rectClient );

		if( TRUE == rectClient.PtInRect( point ) )
		{
			return TRUE;
		}

		return CDockablePane::OnShowControlBarMenu( point );
	}
	else
	{
		return CDockablePane::OnShowControlBarMenu( point );
	}
}

void CToolsDockablePane::OnSlide( BOOL bSlideOut )
{
	if( m_iCurrentContextID < 0 || m_iCurrentContextID >= m_arContextList.GetCount() ||
			NULL == m_arContextList.GetAt( m_iCurrentContextID ) )
	{
		return;
	}

	if( false == m_bBlockSavingState )
	{
		m_arContextList.GetAt( m_iCurrentContextID )->m_bIsPaneAutoHide = IsAutoHideMode();	
	}

	CDockablePane::OnSlide( bSlideOut );
}

void CToolsDockablePane::_ShowTab( TabID eTabID, bool bShow, bool bHideShowOther )
{
	int iTabIndex = m_wndTabs.GetTabFromHwnd( m_mapOutputList[eTabID]->GetSafeHwnd() );

	if( true == bShow )
	{
		m_wndTabs.ShowTab( iTabIndex, TRUE );
		m_wndTabs.SetActiveTab( iTabIndex );
	}
	else
	{
		if( -1 != iTabIndex )
		{
			m_wndTabs.ShowTab( iTabIndex, FALSE );
		}
	}

	if( true == bHideShowOther )
	{
		for( int iLoop = 0; iLoop < m_wndTabs.GetTabsNum(); iLoop++ )
		{
			if( iLoop != iTabIndex )
			{
				m_wndTabs.ShowTab( iLoop, ( true == bHideShowOther ) ? FALSE : TRUE );
			}
		}
	}
		
	CPaneDivider *pclPaneDivider = GetDefaultPaneDivider();

	if( NULL != pclPaneDivider )
	{
		pclPaneDivider->EnableWindow( TRUE );
	}

	if( true == bShow )
	{
		SetWindowText( TASApp.LoadLocalizedString( m_mapOutputCaption[eTabID] ) );
	}
}

void  CToolsDockablePane::forceResize()
{
	// Tab control should cover the whole client area.
	CRect rectDockClient;
	GetClientRect( &rectDockClient );

	m_wndTabs.SetWindowPos( NULL, -1, -1, rectDockClient.Width(), rectDockClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );

	CRect rectTabClient;
	m_wndTabs.GetWndArea( rectTabClient );

	for( iterOutputList iter = m_mapOutputList.begin(); iter != m_mapOutputList.end(); ++iter )
	{
		// For the dialog to resize itself by directly send the 'WM_SIZE' message.
		iter->second->SendMessage( WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM( rectTabClient.Width(), rectTabClient.Height() ) );
	}
}
