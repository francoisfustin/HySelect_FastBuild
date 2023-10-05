#include "stdafx.h"
#include <algorithm>
#include "TASelect.h"
#include "MainFrm.h"
#include "RViewSSelSS.h"
#include "DlgLTtabctrl.h"
#include "DlgInfoSSelDpC.h"
#include "DlgCreateModule.h"
#include "DlgIndSelBv.h"
#include "DlgLeftTabInfo.h"
#include "DlgLeftTabProject.h"
#include "DlgLeftTabSelP.h"
#include "DlgLeftTabSelManager.h"
#include "ToolsDockablePane.h"

#include "DlgLTtabctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgLTtabctrl *pDlgLTtabctrl = NULL;

IMPLEMENT_DYNAMIC( CDlgLTtabctrl, CDockablePane )

CDlgLTtabctrl::CDlgLTtabctrl()
{
	pDlgLTtabctrl = this;
	m_eTabIndex = CMyMFCTabCtrl::TabIndex::etiProj;
	m_ePrevTabIndex = CMyMFCTabCtrl::TabIndex::etiProj;
}

CDlgLTtabctrl::~CDlgLTtabctrl()
{
	pDlgLTtabctrl = NULL;
}

void CDlgLTtabctrl::SetActiveTab( CMyMFCTabCtrl::TabIndex eTabIndex )
{
	if( NULL != m_wndTabs.GetSafeHwnd() )
	{
		m_wndTabs.SetActiveTab( (int)eTabIndex );
		m_eTabIndex = eTabIndex;
	}
}

void CDlgLTtabctrl::RegisterNotificationHandler( CMyMFCTabCtrl::TabIndex eTabIndex, INotificationHandler* pclHandler, short nNotificationHandlerFlags )
{
	for( short nLoop = INotificationHandler::NH_OnLeaveTab; nLoop <= INotificationHandler::NH_OnEnterTab; nLoop <<= 1 )
	{
		if( nLoop == ( nNotificationHandlerFlags & nLoop ) )
		{
			m_mapNotificationHandlerList[eTabIndex][nLoop] = pclHandler;
		}
	}
}

void CDlgLTtabctrl::UnregisterNotificationHandler( CMyMFCTabCtrl::TabIndex eTabIndex, INotificationHandler* pclHandler )
{
	if( 0 == m_mapNotificationHandlerList.count( eTabIndex ) )
		return;

	for( mapShortNotifIter iter = m_mapNotificationHandlerList[eTabIndex].begin(); iter != m_mapNotificationHandlerList[eTabIndex].end(); iter++ )
	{
		if( iter->second == pclHandler )
		{
			m_mapNotificationHandlerList[eTabIndex].erase( iter );
			break;
		}
	}
}

void CDlgLTtabctrl::OnApplicationLook()
{
	COLORREF clrDark;
	COLORREF clrBlack;
	COLORREF clrHighlight;
	COLORREF clrFace;
	COLORREF clrDarkShadow;
	COLORREF clrLight;
	CBrush* pbrFace = NULL;
	CBrush* pbrBlack = NULL;

	// Interface color retrieve.
	CMFCVisualManager::GetInstance()->GetTabFrameColors( &m_wndTabs, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack );

	for( int i = 0; i < m_wndTabs.GetTabsNum(); i++ )
	{
		// CDialogEx own his method to fix background color.
		static_cast<CDialogExt *>( m_wndTabs.GetTabWnd( i ) )->SetBackgroundColor( clrDarkShadow );
		static_cast<CDialogExt *>( m_wndTabs.GetTabWnd( i ) )->OnApplicationLook( clrDarkShadow );
	}
}

BEGIN_MESSAGE_MAP( CDlgLTtabctrl, CDockablePane )
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_REGISTERED_MESSAGE( AFX_WM_CHANGE_ACTIVE_TAB, OnChangeActiveTab )
	ON_REGISTERED_MESSAGE( AFX_WM_CHANGING_ACTIVE_TAB, OnChangingActiveTab )
END_MESSAGE_MAP()

void CDlgLTtabctrl::OnDestroy()
{
	_ClearTabs();
	CDockablePane::OnDestroy();
}

void CDlgLTtabctrl::OnSize( UINT nType, int cx, int cy )
{
	CDockablePane::OnSize( nType, cx, cy );

	CRect rectClient, rectCombo;
	GetClientRect( rectClient );

	int cyTlb = 0;
	
	// Set the new position to the WndTabs.
	// Force the Wnd Tabs to be redrawn completely to avoid logo view problem when hidding the Ribbon bar by
	// double clicking on the Ribbon tabs.(Home, Project,...)
	m_wndTabs.SetRedraw( FALSE );
	m_wndTabs.SetWindowPos( NULL, 0, 0, 10, 10, SWP_SHOWWINDOW );
	m_wndTabs.SetWindowPos( NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER );
	m_wndTabs.SetRedraw( TRUE );
}

int CDlgLTtabctrl::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( -1 == CDockablePane::OnCreate( lpCreateStruct ) )
	{
		return -1;
	}

	// Create tabs window:
	if( FALSE == m_wndTabs.Create( CMFCTabCtrl::STYLE_3D, CRect( 0, 0, 0, 0 ), this, 1 ) )
	{
		TRACE0( "Failed to create output tab window\n" );
		return -1;      // fail to create
	}

	m_wndTabs.SetFlatFrame( FALSE, FALSE );
	m_wndTabs.AutoDestroyWindow( FALSE );

	// Build and attach the image list.
	m_TabCtrlImageList.Create( 16, 16, ILC_COLOR32, CMyMFCTabCtrl::etiLast, 1 );
	CBitmap bm;
	bm.LoadBitmap( IDB_LVTABCTRL );
	m_TabCtrlImageList.Add( &bm, CLR_NONE );

	m_wndTabs.SetImageList( m_TabCtrlImageList.GetSafeHandle() );


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'CDlgLeftTabProject' creation.
	CDlgLeftTabProject *pclDlgLeftTabProject = new CDlgLeftTabProject;

	if( NULL == pclDlgLeftTabProject )
	{
		return FALSE;
	}

	if( FALSE == pclDlgLeftTabProject->Create( CDlgLeftTabProject::IDD, &m_wndTabs ) )
	{
		delete pclDlgLeftTabProject;
		return FALSE;
	}

	pclDlgLeftTabProject->ShowWindow( SW_SHOW );
	CString str = TASApp.LoadLocalizedString( IDS_TABCTRLPROJ );
	m_wndTabs.AddTab( pclDlgLeftTabProject, str, 0 );
	m_mapLeftTabList[CMyMFCTabCtrl::TabIndex::etiProj] = pclDlgLeftTabProject;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'CDlgLeftTabSelManager' creation.
	CDlgLeftTabSelManager *pclDlgLeftTabSelManager = new CDlgLeftTabSelManager;

	if( NULL == pclDlgLeftTabSelManager )
	{
		_ClearTabs();
		return FALSE;
	}

	if( FALSE == pclDlgLeftTabSelManager->Create( CDlgLeftTabSelManager::IDD, &m_wndTabs ) )
	{
		delete pclDlgLeftTabSelManager;
		_ClearTabs();
		return FALSE;
	}

	pclDlgLeftTabSelManager->ShowWindow( SW_SHOW );
	str = TASApp.LoadLocalizedString( IDS_TABCTRLSELECT );
	m_wndTabs.AddTab( pclDlgLeftTabSelManager, str, 1 );
	m_mapLeftTabList[CMyMFCTabCtrl::TabIndex::etiSSel] = pclDlgLeftTabSelManager;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'CDlgLeftTabSelP' creation.
	CDlgLeftTabSelP *pclDlgLeftTabResults = new CDlgLeftTabSelP;

	if( NULL == pclDlgLeftTabResults )
	{
		_ClearTabs();
		return FALSE;
	}

	if( FALSE == pclDlgLeftTabResults->Create( CDlgLeftTabSelP::IDD, &m_wndTabs ) )
	{
		delete pclDlgLeftTabResults;
		_ClearTabs();
		return FALSE;
	}

	pclDlgLeftTabResults->ShowWindow( SW_SHOW );
	str = TASApp.LoadLocalizedString( IDS_TABCTRLRESULT );
	m_wndTabs.AddTab( pclDlgLeftTabResults, str, 2 );
	m_mapLeftTabList[CMyMFCTabCtrl::TabIndex::etiSelP] = pclDlgLeftTabResults;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'CDlgLeftTabInfo' creation.
	CDlgLeftTabInfo *pclDlgLeftTabInfo = new CDlgLeftTabInfo;

	if( NULL == pclDlgLeftTabInfo )
	{
		_ClearTabs();
		return FALSE;
	}

	if( FALSE == pclDlgLeftTabInfo->Create( CDlgLeftTabInfo::IDD, &m_wndTabs ) )
	{
		delete pclDlgLeftTabInfo;
		_ClearTabs();
		return FALSE;
	}

	pclDlgLeftTabInfo->ShowWindow( SW_SHOW );
	str = TASApp.LoadLocalizedString( IDS_TABCTRLINFO );
	m_wndTabs.AddTab( pclDlgLeftTabInfo, str, 3 );
	m_mapLeftTabList[CMyMFCTabCtrl::TabIndex::etiInfo] = pclDlgLeftTabInfo;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	OnApplicationLook();

	return 0;
}

LRESULT CDlgLTtabctrl::OnChangingActiveTab( WPARAM wp, LPARAM lp )
{
	m_ePrevTabIndex = m_eTabIndex;

	if( 0 != m_mapNotificationHandlerList.count( m_eTabIndex ) )
	{
		m_mapNotificationHandlerList[m_eTabIndex][INotificationHandler::NH_OnLeaveTab]->OnLTTabCtrlLeaveTab( (CMyMFCTabCtrl::TabIndex)m_wndTabs.GetActiveTab() );
	}

	return 0;
}

LRESULT CDlgLTtabctrl::OnChangeActiveTab( WPARAM wp, LPARAM lp )
{
	if( wp >= 0 && wp < (WPARAM)m_wndTabs.GetTabsNum() )
	{
		m_eTabIndex = (CMyMFCTabCtrl::TabIndex)wp;

		if( 0 != m_mapNotificationHandlerList.count( m_eTabIndex ) )
		{
			m_mapNotificationHandlerList[m_eTabIndex][INotificationHandler::NH_OnEnterTab]->OnLTTabCtrlEnterTab( m_eTabIndex, m_ePrevTabIndex );
		}
	}

	return 0;
}

void CDlgLTtabctrl::_ClearTabs()
{
	// Window freeing CTabCtrl.
	for( int i = 0; i < m_wndTabs.GetTabsNum (); i++ )
	{
		CWnd *pWnd = m_wndTabs.GetTabWnd( i );

		if( NULL != pWnd )
		{
			if( NULL != pWnd->GetSafeHwnd() )
			{
				pWnd->DestroyWindow();
			}

			delete pWnd;
		}
	}

	m_wndTabs.RemoveAllTabs();
}
