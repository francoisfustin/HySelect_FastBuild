#include "stdafx.h"

#include "MainFrm.h"
#include "HMInclude.h"
#include "utilities.h"

#include "ProductSelectionParameters.h"
#include "TabDlgSpecActDev.h"
#include "TabDlgSpecActUser.h"
#include "TabDlgSpecActTesting.h"
#include "TabDlgSpecActTesting2.h"
#include "TabDlgSpecActHMCalc.h"
#include "TUProductSelectionHelper.h"
#include "TabDlgSpecActProdSel.h"
#include "DlgSpecAct.h"


CDlgSpecAct::CDlgSpecAct( CWnd *pParent )
	: CDialogEx( CDlgSpecAct::IDD, pParent )
{
	m_pTabSpecActDev = NULL;
	m_pTabSpecActUser = NULL;
	m_pTabSpecActTesting = NULL;
	m_pTabSpecActTesting2 = NULL;
	m_pTabSpecActHMCalc = NULL;
	m_pTabSpecActTestProdSel = NULL;
}

CDlgSpecAct::~CDlgSpecAct()
{
	_Clean();
}

int CDlgSpecAct::Display()
{
	return DoModal();
}

int CDlgSpecAct::CheckFile()
{
	CFileTxt inf;
	CString InstDir = TASApp.GetStartDir();

	CString FileName = InstDir + CString( _T("developer.txt") );

	if( TRUE == inf.Open( ( LPCTSTR ) FileName, CFile::modeRead ) )
	{
		int ret;
		char buf[20];
		char key[] = {'t', 'a', 'h', 'c', 'o', 'l', 'l', 'e', 'g', 'e'};
		memset( buf, 0, sizeof( buf ) );
		inf.Read( buf, sizeof( key ) );
		ret = memcmp( buf, key, sizeof( key ) );
		return !ret;
	}

	return 0;
}

BEGIN_MESSAGE_MAP( CDlgSpecAct, CDialogEx )
	ON_NOTIFY( TCN_SELCHANGE, IDC_TABCTRL, OnSelchangeTabctrl )
END_MESSAGE_MAP()

void CDlgSpecAct::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TABCTRL, m_TabCtrl );
}

BOOL CDlgSpecAct::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText( _T("HySelect Special Actions") );
	m_pTabSpecActUser = new CTabDlgSpecActUser();

	if( NULL == m_pTabSpecActUser )
	{
		return FALSE;
	}

	m_pTabSpecActDev = new CTabDlgSpecActDev();

	if( NULL == m_pTabSpecActDev )
	{
		_Clean();
		return FALSE;
	}

	m_pTabSpecActTesting = new CTabDlgSpecActTesting();

	if( NULL == m_pTabSpecActTesting )
	{
		_Clean();
		return FALSE;
	}

	m_pTabSpecActTesting2 = new CTabDlgSpecActTesting2();

	if( NULL == m_pTabSpecActTesting2 )
	{
		_Clean();
		return FALSE;
	}

	m_pTabSpecActHMCalc = new CTabDlgSpecActHMCalc();

	if( NULL == m_pTabSpecActHMCalc )
	{
		_Clean();
		return FALSE;
	}

	m_pTabSpecActTestProdSel = new CTabDlgSpecActTestProdSel();

	if( NULL == m_pTabSpecActTestProdSel )
	{
		_Clean();
		return FALSE;
	}

	// Initialize the tab control.
	TC_ITEM tcinsert;
	tcinsert.mask = TCIF_TEXT ;
	m_TabCtrl.SetMinTabWidth( 20 );

	tcinsert.pszText = _T("Common");
	m_TabCtrl.InsertItem( 0, &tcinsert );

	if( FALSE == m_pTabSpecActUser->Create( IDD_TABSPECACT_USER, &m_TabCtrl ) )
	{
		delete m_pTabSpecActUser;
		m_pTabSpecActUser = NULL;
	}

	if( NULL != m_pTabSpecActUser )
	{
		m_pTabSpecActUser->ShowWindow( SW_HIDE );
	}

	if( 0 != CheckFile() )
	{
		if( FALSE == m_pTabSpecActDev->Create( IDD_TABSPECACT_DEV, &m_TabCtrl ) )
		{
			delete m_pTabSpecActDev;
			m_pTabSpecActDev = NULL;
		}
		else
		{
			m_pTabSpecActDev->ShowWindow( SW_HIDE );
			tcinsert.pszText = _T("Developer");
			m_TabCtrl.InsertItem( 1, &tcinsert );
		}

		if( FALSE == m_pTabSpecActTesting->Create( IDD_TABSPECACT_TESTING, &m_TabCtrl ) )
		{
			delete m_pTabSpecActTesting;
			m_pTabSpecActTesting = NULL;
		}
		else
		{
			m_pTabSpecActTesting->ShowWindow( SW_HIDE );
			tcinsert.pszText = _T("Testing");
			m_TabCtrl.InsertItem( 2, &tcinsert );
		}

		if( FALSE == m_pTabSpecActTesting2->Create( IDD_TABSPECACT_TESTING2, &m_TabCtrl ) )
		{
			delete m_pTabSpecActTesting2;
			m_pTabSpecActTesting2 = NULL;
		}
		else
		{
			m_pTabSpecActTesting2->ShowWindow( SW_HIDE );
			tcinsert.pszText = _T("Testing2");
			m_TabCtrl.InsertItem( 3, &tcinsert );
		}

		if( FALSE == m_pTabSpecActHMCalc->Create( IDD_TABSPECACT_HMCALC, &m_TabCtrl ) )
		{
			delete m_pTabSpecActHMCalc;
			m_pTabSpecActHMCalc = NULL;
		}
		else
		{
			m_pTabSpecActHMCalc->ShowWindow( SW_HIDE );
			tcinsert.pszText = _T("HMCalc");
			m_TabCtrl.InsertItem( 4, &tcinsert );
		}

		if( FALSE == m_pTabSpecActTestProdSel->Create( IDD_TABSPECACT_TESTPRODSEL, &m_TabCtrl ) )
		{
			delete m_pTabSpecActTestProdSel;
			m_pTabSpecActTestProdSel = NULL;
		}
		else
		{
			m_pTabSpecActTestProdSel->ShowWindow( SW_HIDE );
			tcinsert.pszText = _T("ProdSel");
			m_TabCtrl.InsertItem( 5, &tcinsert );
		}
	}

	m_pCurCDlg = m_pTabSpecActUser;

	// Select Tab 0 : Valve.
	m_TabCtrl.SetCurSel( 0 );
	OnSelchangeTabctrl( NULL, NULL );

	return TRUE;
}

void CDlgSpecAct::OnOK()
{
	PREVENT_ENTER_KEY
	::PostMessage( pMainFrame->GetSafeHwnd(), WM_USER_DLGDEBUGCLOSE, 0, 0 );
}

void CDlgSpecAct::OnCancel()
{
	::PostMessage( pMainFrame->GetSafeHwnd(), WM_USER_DLGDEBUGCLOSE, 0, 0 );
}

void CDlgSpecAct::OnSelchangeTabctrl( NMHDR *pNMHDR, LRESULT *pResult )
{
	CString	str;

	switch( m_TabCtrl.GetCurSel() )
	{
		// User tab.
		case 0:
		default:
			_DisplayTabCDlg( (CDialogEx *)m_pTabSpecActUser );
			break;

		// Developer tab.
		case 1:
			_DisplayTabCDlg( (CDialogEx *)m_pTabSpecActDev );
			break;

		// Testing.
		case 2:
			_DisplayTabCDlg( (CDialogEx *)m_pTabSpecActTesting );
			break;

		// Testing 2.
		case 3:
			_DisplayTabCDlg( (CDialogEx *)m_pTabSpecActTesting2 );
			break;

		// HMCalc.
		case 4:
			_DisplayTabCDlg( (CDialogEx *)m_pTabSpecActHMCalc );
			break;

		// Product selection.
		case 5:
			_DisplayTabCDlg( (CDialogEx *)m_pTabSpecActTestProdSel );
			break;
	}

	if( NULL != pResult )
	{
		*pResult = 0;
	}
}

void CDlgSpecAct::_Clean( void )
{
	if( NULL != m_pTabSpecActDev )
	{
		delete m_pTabSpecActDev;
	}

	if( NULL != m_pTabSpecActUser )
	{
		delete m_pTabSpecActUser;
	}

	if( NULL != m_pTabSpecActTesting )
	{
		delete m_pTabSpecActTesting;
	}

	if( NULL != m_pTabSpecActTesting2 )
	{
		delete m_pTabSpecActTesting2;
	}

	if( NULL != m_pTabSpecActHMCalc )
	{
		delete m_pTabSpecActHMCalc;
	}

	if( NULL != m_pTabSpecActTestProdSel )
	{
		delete m_pTabSpecActTestProdSel;
	}

	m_pTabSpecActDev = NULL;
	m_pTabSpecActUser = NULL;
	m_pTabSpecActTesting = NULL;
	m_pTabSpecActTesting2 = NULL;
	m_pTabSpecActHMCalc = NULL;
	m_pTabSpecActTestProdSel = NULL;
}

void CDlgSpecAct::_DisplayTabCDlg( CDialogEx *pTabCDlg )
{
	// Calculate correct size and position for pTabCDlg.
	CRect rect;
	m_TabCtrl.GetClientRect( rect );
	m_TabCtrl.AdjustRect( FALSE, &rect );

	// Display pTabCDlg and hide previous child dlg.
	pTabCDlg->SetWindowPos( &wndTop, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW );

	if( m_pCurCDlg != pTabCDlg )
	{
		m_pCurCDlg->ShowWindow( SW_HIDE );
		m_pCurCDlg = pTabCDlg;
	}
}
