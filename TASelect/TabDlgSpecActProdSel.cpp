#include "stdafx.h"


#include "mainfrm.h"
#include "taselectdoc.h"
#include "ProductSelectionParameters.h"
#include "DlgLeftTabSelManager.h"
#include "TUProductSelectionHelper.h"
#include "TabDlgSpecActProdSel.h"

CTabDlgSpecActTestProdSel::CTabDlgSpecActTestProdSel( CWnd *pParent )
	: CDialogExt( CTabDlgSpecActTestProdSel::IDD, pParent )
{
	m_pCurCDlg = NULL;
	m_pclTabDlgSpecActTestProdSelExport = NULL;
	m_pclTabDlgSpecActTestProdSelTest = NULL;
	m_pclTabDlgSpecActTestProdSelUpdate = NULL;
}

CTabDlgSpecActTestProdSel::~CTabDlgSpecActTestProdSel()
{
	_Clean();
}

BEGIN_MESSAGE_MAP( CTabDlgSpecActTestProdSel, CDialogEx )
	ON_NOTIFY( TCN_SELCHANGE, IDC_TABPRODSEL, OnSelChangeTabCtrl )
END_MESSAGE_MAP()

void CTabDlgSpecActTestProdSel::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TABPRODSEL, m_WndTabCtrl );
}

BOOL CTabDlgSpecActTestProdSel::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	// Initialize the tab control.
	TC_ITEM tcinsert;
	tcinsert.mask = TCIF_TEXT ;
	m_WndTabCtrl.SetMinTabWidth( 20 );

	// 'CTabDlgSpecActTestProdSelExport' creation.
	m_pclTabDlgSpecActTestProdSelExport = new CTabDlgSpecActTestProdSelExport;

	if( NULL == m_pclTabDlgSpecActTestProdSelExport )
	{
		_Clean();
		return FALSE;
	}

	if( FALSE == m_pclTabDlgSpecActTestProdSelExport->Create( CTabDlgSpecActTestProdSelExport::IDD, &m_WndTabCtrl ) )
	{
		_Clean();
		return FALSE;
	}

	m_pclTabDlgSpecActTestProdSelExport->ShowWindow( SW_HIDE );
	tcinsert.pszText = _T("Export");
	m_WndTabCtrl.InsertItem( 0, &tcinsert );
	
	// 'CTabDlgSpecActTestProdSelTest' creation.
	m_pclTabDlgSpecActTestProdSelTest = new CTabDlgSpecActTestProdSelTest;

	if( NULL == m_pclTabDlgSpecActTestProdSelTest )
	{
		_Clean();
		return FALSE;
	}

	if( FALSE == m_pclTabDlgSpecActTestProdSelTest->Create( CTabDlgSpecActTestProdSelTest::IDD, &m_WndTabCtrl ) )
	{
		_Clean();
		return FALSE;
	}

	m_pclTabDlgSpecActTestProdSelTest->ShowWindow( SW_HIDE );
	tcinsert.pszText = _T("Test");
	m_WndTabCtrl.InsertItem( 1, &tcinsert );

	// 'CTabDlgSpecActTestProdSelUpdate' creation.
	m_pclTabDlgSpecActTestProdSelUpdate = new CTabDlgSpecActTestProdSelUpdate;

	if( NULL == m_pclTabDlgSpecActTestProdSelUpdate )
	{
		_Clean();
		return FALSE;
	}

	if( FALSE == m_pclTabDlgSpecActTestProdSelUpdate->Create( CTabDlgSpecActTestProdSelUpdate::IDD, &m_WndTabCtrl ) )
	{
		_Clean();
		return FALSE;
	}

	m_pclTabDlgSpecActTestProdSelUpdate->ShowWindow( SW_HIDE );
	tcinsert.pszText = _T("Update");
	m_WndTabCtrl.InsertItem( 2, &tcinsert );

	m_pCurCDlg = m_pclTabDlgSpecActTestProdSelExport;

	// Select tab 0.
	m_WndTabCtrl.SetCurSel( 0 );
	OnSelChangeTabCtrl( NULL, NULL );

	return TRUE;
}

void CTabDlgSpecActTestProdSel::OnSelChangeTabCtrl( NMHDR *pNMHDR, LRESULT *pResult )
{
	CString	str;

	switch( m_WndTabCtrl.GetCurSel() )
	{
		// 'Export' tab.
		case 0:
		default:
			_DisplayTabCDlg( (CDialogEx *)m_pclTabDlgSpecActTestProdSelExport );
			break;

		// 'Test' tab.
		case 1:
			_DisplayTabCDlg( (CDialogEx *)m_pclTabDlgSpecActTestProdSelTest );
			break;

		// 'Correct' tab.
		case 2:
			_DisplayTabCDlg( (CDialogEx *)m_pclTabDlgSpecActTestProdSelUpdate );
			break;
	}

	if( NULL != pResult )
	{
		*pResult = 0;
	}
}

void CTabDlgSpecActTestProdSel::_Clean( void )
{
	if( NULL != m_pclTabDlgSpecActTestProdSelExport )
	{
		delete m_pclTabDlgSpecActTestProdSelExport;
		m_pclTabDlgSpecActTestProdSelExport = NULL;
	}

	if( NULL != m_pclTabDlgSpecActTestProdSelTest )
	{
		delete m_pclTabDlgSpecActTestProdSelTest;
		m_pclTabDlgSpecActTestProdSelTest = NULL;
	}

	if( NULL != m_pclTabDlgSpecActTestProdSelUpdate )
	{
		delete m_pclTabDlgSpecActTestProdSelUpdate;
		m_pclTabDlgSpecActTestProdSelUpdate = NULL;
	}
}

void CTabDlgSpecActTestProdSel::_DisplayTabCDlg( CDialogEx *pTabCDlg )
{
	// Calculate correct size and position for pTabCDlg.
	CRect rect;
	m_WndTabCtrl.GetClientRect( rect );
	m_WndTabCtrl.AdjustRect( FALSE, &rect );

	// Display pTabCDlg and hide previous child dlg.
	pTabCDlg->SetWindowPos( &wndTop, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW );

	if( m_pCurCDlg != pTabCDlg )
	{
		m_pCurCDlg->ShowWindow( SW_HIDE );
		m_pCurCDlg = pTabCDlg;
	}
}
