#include "stdafx.h"


#include "MainFrm.h"
#include "TASelectDoc.h"
#include "HMInclude.h"
#include "HiPerfTimer.h"
#include "TabDlgSpecActTesting2.h"

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

CTabDlgSpecActTesting2::CTabDlgSpecActTesting2( CWnd *pParent )
	: CDialogExt( CTabDlgSpecActTesting2::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
	m_fIsRecording = false;
	m_fIsMacroLoading = false;
	m_dInputMouseFactorX = 0;
	m_dInputMouseFactorY = 0;
	m_pThread = NULL;
	m_pclStopEvent = NULL;
	m_pclThreadStopped = NULL;
	m_fIsRunning = false;
	m_iPrevMacroIndex = 0;
}

CTabDlgSpecActTesting2::~CTabDlgSpecActTesting2()
{
	if( NULL != m_pclStopEvent )
	{
		delete m_pclStopEvent;
	}

	if( NULL != m_pclThreadStopped )
	{
		delete m_pclThreadStopped;
	}
}

void CTabDlgSpecActTesting2::EnableMacroRunning( bool fEnable )
{
	m_fIsRecording = fEnable;
}

bool CTabDlgSpecActTesting2::IsMacroRunning( void )
{
	return m_fIsRecording;
}

void CTabDlgSpecActTesting2::WriteMacro( MSG *pMsg )
{
	if( NULL == pMsg || false == m_fIsRecording )
	{
		return;
	}

	// Filter.
	HWND hWnd = pMsg->hwnd;

	while( ( HWND )0 != hWnd )
	{
		if( hWnd == GetSafeHwnd() )
		{
			return;
		}

		hWnd = ::GetParent( hWnd );
	}

	_FillInfo( pMsg );
	CMacro *pclMacro = m_vecMacroSaved.back();

	DWORD dwTimeElapsed;

	if( 0 == m_List.GetCount() )
	{
		m_iPrevMacroIndex = 0;
		dwTimeElapsed = 0;
	}
	else
	{
		dwTimeElapsed = pclMacro->m_dwTickCount - m_vecMacroSaved[m_iPrevMacroIndex]->m_dwTickCount;
		m_iPrevMacroIndex++;
	}

	CString str = _T("Windows message not managed!");

	CString strPrefix;
	strPrefix.Format( _T("(%08i ms) hWnd: %08x; ID: "), dwTimeElapsed, pclMacro->m_msg.hwnd );

	CWnd *pWnd = CWnd::FromHandle( pclMacro->m_msg.hwnd );

	if( NULL != pWnd )
	{
		strPrefix.AppendFormat( _T("%08x (%i)"), pWnd->GetDlgCtrlID(), pWnd->GetDlgCtrlID() );
	}
	else
	{
		strPrefix.AppendFormat( _T("unknown!") );
	}

	switch( pclMacro->m_msg.message )
	{
		case WM_LBUTTONDOWN:
			str.Format( _T("%s; Msg: WM_LBUTTONDOWN; x: %4i; y: %4i"), strPrefix, GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
			break;

		case WM_LBUTTONDBLCLK:
			str.Format( _T("%s; Msg: WM_LBUTTONDBLCLK; x: %4i; y: %4i"), strPrefix, GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
			break;

		case WM_LBUTTONUP:
			str.Format( _T("%s; Msg: WM_LBUTTONUP; x: %4i; y: %4i"), strPrefix, GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
			break;

		case WM_RBUTTONDOWN:
			str.Format( _T("%s; Msg: WM_RBUTTONDOWN; x: %4i; y: %4i"), strPrefix, GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
			break;

		case WM_RBUTTONUP:
			str.Format( _T("%s; Msg: WM_RBUTTONUP; x: %4i; y: %4i"), strPrefix, GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
			break;

		case WM_KEYDOWN:
		{
			int iRepeatCount = pclMacro->m_msg.lParam & 0x0000FFFF;
			int iScanCode = ( pclMacro->m_msg.lParam & 0x00FF0000 ) >> 16;
			CString strIsExtendedKey = ( pclMacro->m_msg.lParam & 0x01000000 ) ? _T("true" ) : _T( "false");
			str.Format( _T("%s; Msg: WM_KEYDOWN; KC: %03x; RC: %5i; SC: %2x; ExtendedKey: %s"), strPrefix, pclMacro->m_msg.wParam, iRepeatCount, iScanCode,
						strIsExtendedKey );
		}
		break;

		case WM_KEYUP:
		{
			int iRepeatCount = pclMacro->m_msg.lParam & 0x0000FFFF;
			int iScanCode = ( pclMacro->m_msg.lParam & 0x00FF0000 ) >> 16;
			CString strIsExtendedKey = ( pclMacro->m_msg.lParam & 0x01000000 ) ? _T("true" ) : _T( "false");
			str.Format( _T("%s; Msg: WM_KEYUP; KC: %03x; RC: %5i; SC: %2x; ExtendedKey: %s"), strPrefix, pclMacro->m_msg.wParam, iRepeatCount, iScanCode, strIsExtendedKey );
		}
		break;
	}

	int iIndex = m_List.AddString( str );

	if( LB_ERR != iIndex && LB_ERRSPACE != iIndex )
	{
		m_List.SetCurSel( iIndex );
	}
}

UINT CTabDlgSpecActTesting2::ThreadRunMacro( LPVOID pParam )
{
	CTabDlgSpecActTesting2 *pclTabDlgSpecActTesting2 = ( CTabDlgSpecActTesting2 * )pParam;

	if( NULL == pclTabDlgSpecActTesting2 )
	{
		return -1;
	}

	pclTabDlgSpecActTesting2->GetDlgItem( IDC_STATIC_RECORDSTATE )->SetWindowText( _T( "" ) );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONSTART )->EnableWindow( FALSE );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONSTOP )->EnableWindow( FALSE );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONCLEAR )->EnableWindow( FALSE );

	pclTabDlgSpecActTesting2->GetDlgItem( IDC_STATIC_RUNSTATE )->SetWindowText( _T("Macro is running") );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONSAVE )->EnableWindow( FALSE );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONLOAD )->EnableWindow( FALSE );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONRUN )->SetWindowText( _T("Stop") );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONRUN )->EnableWindow( TRUE );

	pclTabDlgSpecActTesting2->m_fIsRunning = true;

	bool fMustStop = false;

	for( long lLoop = 0; lLoop < ( long )pclTabDlgSpecActTesting2->m_vecMacroSaved.size() && false == fMustStop; lLoop++ )
	{
		if( lLoop > 0 )
		{
			// Test if the user want to cancel the exportation or suspend it.
			DWORD dwReturn = WaitForSingleObject( pclTabDlgSpecActTesting2->m_pclStopEvent->m_hObject,
												  pclTabDlgSpecActTesting2->m_vecMacroSaved[lLoop]->m_dwTickCount - pclTabDlgSpecActTesting2->m_vecMacroSaved[lLoop - 1]->m_dwTickCount );
			DWORD dwError = GetLastError();

			switch( dwReturn )
			{
				// Stop.
				case WAIT_OBJECT_0:
					fMustStop = true;
					break;

				// Timeout.
				case WAIT_TIMEOUT:
					break;
			}
		}

		pclTabDlgSpecActTesting2->m_List.SetCurSel( lLoop );

		CMacro *pclMacro = pclTabDlgSpecActTesting2->m_vecMacroSaved[lLoop];

		if( WM_LBUTTONDOWN == pclMacro->m_msg.message || WM_LBUTTONDBLCLK == pclMacro->m_msg.message || WM_LBUTTONUP == pclMacro->m_msg.message ||
			WM_RBUTTONDOWN == pclMacro->m_msg.message || WM_RBUTTONUP == pclMacro->m_msg.message )
		{
			CPoint ptMainFrame( GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
			CPoint ptAbsolute = ptMainFrame;
			pMainFrame->ClientToScreen( &ptAbsolute );

			INPUT rInput;
			ZeroMemory( &rInput, sizeof( INPUT ) );
			rInput.type = INPUT_MOUSE;
			rInput.mi.dx = ( LONG )( ( double )ptAbsolute.x * pclTabDlgSpecActTesting2->m_dInputMouseFactorX );
			rInput.mi.dy = ( LONG )( ( double )ptAbsolute.y * pclTabDlgSpecActTesting2->m_dInputMouseFactorY );
			rInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
			// If we work with multiple monitor, we must use this line (and change in 'OnInitDialog').
			// rInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE;
			SendInput( 1, &rInput, sizeof( INPUT ) );

			rInput.mi.dx = 0;
			rInput.mi.dy = 0;

			switch( pclMacro->m_msg.message )
			{
				case WM_LBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
					rInput.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
					break;

				case WM_LBUTTONUP:
					rInput.mi.dwFlags = MOUSEEVENTF_LEFTUP;
					break;

				case WM_RBUTTONDOWN:
					rInput.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
					break;

				case WM_RBUTTONUP:
					rInput.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
					break;
			}

			SendInput( 1, &rInput, sizeof( INPUT ) );

			if( WM_LBUTTONDBLCLK == pclMacro->m_msg.message )
			{
				SendInput( 1, &rInput, sizeof( INPUT ) );
			}


			/*
			HWND hWnd = ::WindowFromPoint( ptAbsolute );
			if( (HWND)0 == hWnd )
				continue;
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			if( NULL == pWnd )
				continue;

			// Search child window.
			CPoint ptRelative;
			CWnd* pWndChild = pWnd;
			do
			{
				pWnd = pWndChild;
				ptRelative = ptAbsolute;
				pWnd->ScreenToClient( &ptRelative );
				pWndChild = CWnd::FromHandle( ::ChildWindowFromPoint( pWnd->GetSafeHwnd(), ptRelative ) );
			}while( NULL != pWndChild && pWnd != pWndChild );

			pclMacro->m_msg.lParam = (DWORD)(short)(int)( ptRelative.y ) << 16;
			pclMacro->m_msg.lParam += (DWORD)(short)(int)( ptRelative.x );
			::SendMessage( pWnd->GetSafeHwnd(), pclMacro->m_msg.message, pclMacro->m_msg.wParam, pclMacro->m_msg.lParam );
			*/
		}
		else if( WM_KEYDOWN == pclMacro->m_msg.message || WM_KEYUP == pclMacro->m_msg.message )
		{
			INPUT rInput;
			ZeroMemory( &rInput, sizeof( INPUT ) );
			rInput.type = INPUT_KEYBOARD;
			rInput.ki.wVk = pclMacro->m_msg.wParam;
			//			rInput.ki.wScan = MapVirtualKey( pclMacro->m_msg.wParam, MAPVK_VK_TO_VSC );
			rInput.ki.dwFlags = ( WM_KEYDOWN == pclMacro->m_msg.message ) ? 0 : KEYEVENTF_KEYUP;
			SendInput( 1, &rInput, sizeof( INPUT ) );
		}
	}

	pclTabDlgSpecActTesting2->m_List.SetCurSel( 0 );

	pclTabDlgSpecActTesting2->GetDlgItem( IDC_STATIC_RECORDSTATE )->SetWindowText( _T( "" ) );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONSTART )->EnableWindow( TRUE );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONSTOP )->EnableWindow( FALSE );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONCLEAR )->EnableWindow( TRUE );

	pclTabDlgSpecActTesting2->GetDlgItem( IDC_STATIC_RUNSTATE )->SetWindowText( _T("Macro is finished") );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONSAVE )->EnableWindow( TRUE );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONLOAD )->EnableWindow( TRUE );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONRUN )->SetWindowText( _T("Run") );
	pclTabDlgSpecActTesting2->GetDlgItem( IDC_BUTTONRUN )->EnableWindow( TRUE );

	pclTabDlgSpecActTesting2->m_fIsRunning = false;

	// Signal that thread is stopped.
	pclTabDlgSpecActTesting2->m_pclThreadStopped->SetEvent();

	return 0;
}

BEGIN_MESSAGE_MAP( CTabDlgSpecActTesting2, CDialogExt )
	ON_WM_SIZE()
	ON_BN_CLICKED( IDC_BUTTONSTART, OnBnClickedStart )
	ON_BN_CLICKED( IDC_BUTTONSTOP, OnBnClickedStop )
	ON_BN_CLICKED( IDC_BUTTONCLEAR, OnBnClickedClear )
	ON_BN_CLICKED( IDC_BUTTONSAVE, OnBnClickedSave )
	ON_BN_CLICKED( IDC_BUTTONLOAD, OnBnClickedLoad )
	ON_BN_CLICKED( IDC_BUTTONRUN, OnBnClickedRun )
END_MESSAGE_MAP()

void CTabDlgSpecActTesting2::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST, m_List );
}

BOOL CTabDlgSpecActTesting2::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	// Create an event to stop the thread when needed.
	m_pclStopEvent = new CEvent( FALSE, TRUE );

	if( NULL == m_pclStopEvent )
	{
		return FALSE;
	}

	m_pclStopEvent->ResetEvent();

	// Create an event to allow thread to specify when it has finished.
	m_pclThreadStopped = new CEvent( FALSE, TRUE );

	if( NULL == m_pclThreadStopped )
	{
		return FALSE;
	}

	m_pclThreadStopped->ResetEvent();

	// If we want work with multiple monitor, we have to use 2 lines below. In this case in 'MOUSEINPUT' structure, we must
	// use 'MOUSEEVENTF_VIRTUALDESK' and 'MOUSEEVENTF_ABSOLUTE' for the 'dwFlags' variable.
	m_dInputMouseFactorX = ( 65535.0f / double( GetSystemMetrics( SM_CXSCREEN ) - 1 ) );
	m_dInputMouseFactorY = ( 65535.0f / double( GetSystemMetrics( SM_CYSCREEN ) - 1 ) );
	//	m_dInputMouseFactorX = ( 65535.0f / double( GetSystemMetrics( SM_CXVIRTUALSCREEN ) - 1 ) );
	//	m_dInputMouseFactorY = ( 65535.0f / double( GetSystemMetrics( SM_CYVIRTUALSCREEN ) - 1 ) );

	GetDlgItem( IDC_STATIC_RECORDSTATE )->SetWindowTextW( _T( "" ) );
	GetDlgItem( IDC_STATIC_RUNSTATE )->SetWindowTextW( _T( "" ) );
	GetDlgItem( IDC_BUTTONSTOP )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONCLEAR )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONSAVE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONRUN )->EnableWindow( FALSE );
	return TRUE;
}

void CTabDlgSpecActTesting2::OnSize( UINT nType, int cx, int cy )
{
	CDialogExt::OnSize( nType, cx, cy );
	CRect rect;

	if( NULL != m_List.GetSafeHwnd() )
	{
		m_List.GetWindowRect( &rect );
		ScreenToClient( rect );
		m_List.SetWindowPos( NULL, rect.left, rect.top, cx - rect.left, cy - rect.top, SWP_NOZORDER );
	}
}

void CTabDlgSpecActTesting2::OnBnClickedStart()
{
	if( 0 != m_vecMacroSaved.size() )
	{
		if( false == m_fIsMacroLoading )
		{
			if( IDNO == MessageBox( _T("Data are already recorded. Do you want to overwrite them?" ), _T( "Overwrite data?"), MB_YESNO | MB_ICONQUESTION ) )
			{
				return;
			}
		}
		else
		{
			if( IDNO == MessageBox( _T("Data has been loaded. Do you want to overwrite them?" ), _T( "Overwrite data?"), MB_YESNO | MB_ICONQUESTION ) )
			{
				return;
			}

			m_fIsMacroLoading = false;
		}

		OnBnClickedClear();
	}

	m_fIsRecording = true;
	GetDlgItem( IDC_STATIC_RECORDSTATE )->SetWindowText( _T("Macro is recording...") );
	GetDlgItem( IDC_BUTTONSTART )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONSTOP )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONCLEAR )->EnableWindow( FALSE );

	GetDlgItem( IDC_STATIC_RUNSTATE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_BUTTONSAVE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONLOAD )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONRUN )->EnableWindow( FALSE );
}

void CTabDlgSpecActTesting2::OnBnClickedStop()
{
	m_fIsRecording = false;
	GetDlgItem( IDC_STATIC_RECORDSTATE )->SetWindowText( _T("Macro recording finished") );
	GetDlgItem( IDC_BUTTONSTART )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONSTOP )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONCLEAR )->EnableWindow( TRUE );

	GetDlgItem( IDC_STATIC_RUNSTATE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_BUTTONSAVE )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONLOAD )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONRUN )->EnableWindow( TRUE );
}

void CTabDlgSpecActTesting2::OnBnClickedClear()
{
	m_List.ResetContent();
	m_vecMacroSaved.clear();
	GetDlgItem( IDC_STATIC_RECORDSTATE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_BUTTONSTART )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONSTOP )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONCLEAR )->EnableWindow( FALSE );

	GetDlgItem( IDC_STATIC_RUNSTATE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_BUTTONSAVE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONLOAD )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONRUN )->EnableWindow( FALSE );
}

void CTabDlgSpecActTesting2::OnBnClickedSave()
{
	if( true == m_fIsRecording )
	{
		if( IDNO == MessageBox( _T("Do you want to abort the current recording?" ), _T( "Interrupt process?"), MB_YESNO | MB_ICONQUESTION ) )
		{
			return;
		}

		OnBnClickedStop();
	}

	// Load file filter, compose file name and initialize 'CFileDialog'.
	CString strExt = _T("tsm");
	CString strFilter = _T("HySelect macro files (*.tsm)|*.tsm");
	CString strPrjDir = GetProjectDirectory();

	CFileDialog dlg( FALSE, strExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, ( LPCTSTR )strFilter, NULL );
	dlg.m_ofn.lpstrInitialDir = ( LPCTSTR )strPrjDir;

	if( IDOK != dlg.DoModal() )
	{
		return;
	}

	CFile clFile;

	if( 0 == clFile.Open( dlg.GetPathName(), CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone | CFile::modeCreate ) )
	{
		CString str;
		str.Format( _T("Can't open file: %s"), dlg.GetPathName() );
		MessageBox( str, _T("File error"), MB_OK | MB_ICONERROR );
		return;
	}

	int iNbrMacro = (int)m_vecMacroSaved.size();
	clFile.Write( &iNbrMacro, sizeof( iNbrMacro ) );

	if( iNbrMacro > 0 )
	{
		for( vecMacroIter iter = m_vecMacroSaved.begin(); iter != m_vecMacroSaved.end(); iter++ )
		{
			if( false == ( *iter )->Write( &clFile ) )
			{
				return;
			}
		}
	}

	clFile.Close();

	GetDlgItem( IDC_STATIC_RECORDSTATE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_BUTTONSTART )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONSTOP )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONCLEAR )->EnableWindow( TRUE );

	GetDlgItem( IDC_STATIC_RUNSTATE )->SetWindowText( _T("Macro is saved") );
	GetDlgItem( IDC_BUTTONSAVE )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONLOAD )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONRUN )->EnableWindow( TRUE );
}

void CTabDlgSpecActTesting2::OnBnClickedLoad()
{
	if( true == m_fIsRecording )
	{
		if( IDNO == MessageBox( _T("Do you want to abort the current recording?" ), _T( "Interrupt process?"), MB_YESNO | MB_ICONQUESTION ) )
		{
			return;
		}

		OnBnClickedStop();
	}

	if( 0 != m_vecMacroSaved.size() )
	{
		if( IDNO == MessageBox( _T("Records exist. Do you want to overwrite them?" ), _T( "Overwrite data?"), MB_YESNO | MB_ICONQUESTION ) )
		{
			return;
		}

		OnBnClickedClear();
	}

	// Load file filter, compose file name and initialize 'CFileDialog'.
	CString strExt = _T("tsm");
	CString strFilter = _T("HySelect macro files (*.tsm)|*.tsm");
	CString strPrjDir = GetProjectDirectory();

	CFileDialog dlg( TRUE, strExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, ( LPCTSTR )strFilter, NULL );
	dlg.m_ofn.lpstrInitialDir = ( LPCTSTR )strPrjDir;

	if( IDOK != dlg.DoModal() )
	{
		return;
	}

	CFile clFile;

	if( 0 == clFile.Open( dlg.GetPathName(), CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone ) )
	{
		CString str;
		str.Format( _T("Can't open file: %s"), dlg.GetPathName() );
		MessageBox( str, _T("File error"), MB_OK | MB_ICONERROR );
		return;
	}

	int iNbrMacro = 0;
	clFile.Read( &iNbrMacro, sizeof( iNbrMacro ) );

	if( iNbrMacro > 0 )
	{
		for( int iLoop = 0; iLoop < iNbrMacro; iLoop++ )
		{
			CMacro *pclMacro = new CMacro();

			if( NULL == pclMacro )
			{
				return;
			}

			if( false == pclMacro->Read( &clFile ) )
			{
				delete pclMacro;
				return;
			}

			m_vecMacroSaved.push_back( pclMacro );

			CString str;

			switch( pclMacro->m_msg.message )
			{
				case WM_LBUTTONDOWN:
					str.Format( _T("Msg: WM_LBUTTONDOWN; x: %4i; y: %4i"), GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
					break;

				case WM_LBUTTONDBLCLK:
					str.Format( _T("Msg: WM_LBUTTONDBLCLK; x: %4i; y: %4i"), GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
					break;

				case WM_LBUTTONUP:
					str.Format( _T("Msg: WM_LBUTTONUP; x: %4i; y: %4i"), GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
					break;

				case WM_RBUTTONDOWN:
					str.Format( _T("Msg: WM_RBUTTONDOWN; x: %4i; y: %4i"), GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
					break;

				case WM_RBUTTONUP:
					str.Format( _T("Msg: WM_RBUTTONUP; x: %4i; y: %4i"), GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
					break;

				case WM_KEYDOWN:
				case WM_KEYUP:
				{
					int iRepeatCount = pclMacro->m_msg.lParam & 0x0000FFFF;
					int iScanCode = ( pclMacro->m_msg.lParam & 0x00FF0000 ) >> 16;
					CString strIsExtendedKey = ( pclMacro->m_msg.lParam & 0x01000000 ) ? _T("true" ) : _T( "false");
					str.Format( _T("Msg: WM_KEYDOWN; KeyCode: %03x; Repeatcount: %5i; ScanCode: %2x; IsExtendedKey: %s"), pclMacro->m_msg.wParam, iRepeatCount, iScanCode,
								strIsExtendedKey );
				}
				break;
			}

			m_List.AddString( str );
		}
	}

	clFile.Close();

	GetDlgItem( IDC_STATIC_RECORDSTATE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_BUTTONSTART )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONSTOP )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTONCLEAR )->EnableWindow( TRUE );

	GetDlgItem( IDC_STATIC_RUNSTATE )->SetWindowText( _T("Macro is loaded") );
	GetDlgItem( IDC_BUTTONSAVE )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONLOAD )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTONRUN )->EnableWindow( TRUE );
	m_fIsMacroLoading = true;
}

void CTabDlgSpecActTesting2::OnBnClickedRun()
{
	if( true == m_fIsRecording )
	{
		if( IDNO == MessageBox( _T("Do you want to abort the current recording?" ), _T( "Interrupt process?"), MB_YESNO | MB_ICONQUESTION ) )
		{
			return;
		}

		OnBnClickedStop();
	}

	if( false == m_fIsRunning )
	{
		// Create the thread and start it.
		m_pclStopEvent->ResetEvent();
		m_pclThreadStopped->ResetEvent();
		m_pThread = AfxBeginThread( &ThreadRunMacro, ( LPVOID )this );
	}
	else
	{
		// If thread is not yet stopped...
		DWORD dwReturn = WaitForSingleObject( m_pclThreadStopped->m_hObject, 0 );

		if( dwReturn != WAIT_OBJECT_0 )
		{
			m_pclStopEvent->SetEvent();

			// Wait end of the thread.
			while( WAIT_TIMEOUT == WaitForSingleObject( m_pclThreadStopped->m_hObject, 10 ) )
			{
				TASApp.PumpMessages();
			}
		}
	}
}

bool CTabDlgSpecActTesting2::_FillInfo( MSG *pMsg )
{
	CMacro *pclMacro = new CMacro();

	if( NULL == pclMacro )
	{
		return false;
	}

	pclMacro->m_dwTickCount = ::GetTickCount();
	pclMacro->m_msg = *pMsg;

	CWnd *pFirstWnd = CWnd::FromHandle( pclMacro->m_msg.hwnd );
	pclMacro->m_pt = CPoint( GET_X_LPARAM( pclMacro->m_msg.lParam ), GET_Y_LPARAM( pclMacro->m_msg.lParam ) );
	m_vecMacroSaved.push_back( pclMacro );

	m_List.AddString( _T("Class hierarchy") );
	CWnd *pNextWnd = pFirstWnd;

	while( NULL != pNextWnd )
	{
		// Save ID.
		CMacroWndInfo *pclWndInfo = new CMacroWndInfo();

		if( NULL == pclWndInfo )
		{
			return false;
		}

		pclWndInfo->m_iID = pFirstWnd->GetDlgCtrlID();

		// Save class name.
		CRuntimeClass *pRuntimeClass = pNextWnd->GetRuntimeClass();
		CA2W strClassName( pRuntimeClass->m_lpszClassName );
		pclWndInfo->m_strClassName = strClassName;

		CString str;
		str.Format( _T("  ID: 0x%x (%i); name: %s"), pclWndInfo->m_iID, pclWndInfo->m_iID, pclWndInfo->m_strClassName );
		m_List.AddString( str );


		if( 0 == pclWndInfo->m_strClassName.Compare( _T("CMFCTabCtrl" ) ) )
		{
			_FillMFCTabCtrlData( pclWndInfo, pFirstWnd, pNextWnd, pclMacro );
		}
		else if( 0 == pclWndInfo->m_strClassName.Compare( _T("CTreeCtrl" ) ) )
		{
			_FillTreeCtrlData( pclWndInfo, pFirstWnd, pNextWnd, pclMacro );
		}
		else if( 0 == pclWndInfo->m_strClassName.Compare( _T("CMFCRibbonBar" ) ) )
		{
			_FillMFCRibbonBarData( pclWndInfo, pFirstWnd, pNextWnd, pclMacro );
		}

		m_vecMacroSaved.back()->m_vecWndHierarchy.push_back( pclWndInfo );

		pNextWnd = pNextWnd->GetParent();
	}

	return true;
}

bool CTabDlgSpecActTesting2::_FillMFCTabCtrlData( CMacroWndInfo *pclWndInfo, CWnd *pFirstWnd, CWnd *pCurrentWnd, CMacro *pclMacro )
{
	if( NULL == pclWndInfo || NULL == pFirstWnd || NULL == pCurrentWnd || NULL == pclMacro )
	{
		return false;
	}

	CMFCTabCtrl *pclMFCTabCtrl = dynamic_cast<CMFCTabCtrl*>( pCurrentWnd );

	if( NULL == pclMFCTabCtrl )
	{
		return false;
	}

	CMacroMFCTabCtrlData *pData = new CMacroMFCTabCtrlData();

	if( NULL == pData )
	{
		return false;
	}

	CPoint pt = pclMacro->m_pt;

	if( pFirstWnd->GetSafeHwnd() != pCurrentWnd->GetSafeHwnd() )
	{
		pFirstWnd->ClientToScreen( &pt );
		pCurrentWnd->ScreenToClient( &pt );
	}

	pData->m_iTabID = pclMFCTabCtrl->GetTabFromPoint( pt );

	CString strTabID;
	strTabID.Format( _T("    Tab ID: %x (%i)"), pData->m_iTabID, pData->m_iTabID );
	m_List.AddString( strTabID );

	pclWndInfo->m_iAdDataID = AdData_CMFCTabCtrl;
	pclWndInfo->m_pclAdData = pData;

	return true;
}

bool CTabDlgSpecActTesting2::_FillTreeCtrlData( CMacroWndInfo *pclWndInfo, CWnd *pFirstWnd, CWnd *pCurrentWnd, CMacro *pclMacro )
{
	if( NULL == pclWndInfo || NULL == pFirstWnd || NULL == pCurrentWnd || NULL == pclMacro )
	{
		return false;
	}

	CTreeCtrl *pclTreeCtrl = dynamic_cast<CTreeCtrl*>( pCurrentWnd );

	if( NULL == pclTreeCtrl )
	{
		return false;
	}

	CMacroTreeCtrlData *pData = new CMacroTreeCtrlData();

	if( NULL == pData )
	{
		return false;
	}

	CPoint pt = pclMacro->m_pt;

	if( pFirstWnd->GetSafeHwnd() != pCurrentWnd->GetSafeHwnd() )
	{
		pFirstWnd->ClientToScreen( &pt );
		pCurrentWnd->ScreenToClient( &pt );
	}

	UINT uiFlags;
	HTREEITEM hItem = pclTreeCtrl->HitTest( pt, &uiFlags );

	if( NULL != hItem )
	{
		if( TVHT_NOWHERE == ( TVHT_NOWHERE & uiFlags ) )
		{
			m_List.AddString( _T("    TVHT_NOWHERE") );
		}

		if( TVHT_ONITEMICON == ( TVHT_ONITEMICON & uiFlags ) )
		{
			m_List.AddString( _T("    TVHT_ONITEMICON") );
		}

		if( TVHT_ONITEMLABEL == ( TVHT_ONITEMLABEL & uiFlags ) )
		{
			m_List.AddString( _T("    TVHT_ONITEMLABEL") );
		}

		if( TVHT_ONITEM == ( TVHT_ONITEM & uiFlags ) )
		{
			m_List.AddString( _T("    TVHT_ONITEM") );
		}

		if( TVHT_ONITEMINDENT == ( TVHT_ONITEMINDENT & uiFlags ) )
		{
			m_List.AddString( _T("    TVHT_ONITEMINDENT") );
		}

		if( TVHT_ONITEMBUTTON == ( TVHT_ONITEMBUTTON & uiFlags ) )
		{
			m_List.AddString( _T("    TVHT_ONITEMBUTTON") );
		}

		if( TVHT_ONITEMRIGHT == ( TVHT_ONITEMRIGHT & uiFlags ) )
		{
			m_List.AddString( _T("    TVHT_ONITEMRIGHT") );
		}

		if( TVHT_ONITEMSTATEICON == ( TVHT_ONITEMSTATEICON & uiFlags ) )
		{
			m_List.AddString( _T("    TVHT_ONITEMSTATEICON") );
		}

		pData->m_dwParam = pclTreeCtrl->GetItemData( hItem );

		CString str;
		str.Format( _T("    Data: 0x%x (%i)"), pData->m_dwParam, pData->m_dwParam );
		m_List.AddString( str );
	}

	pclWndInfo->m_iAdDataID = AdData_CTreeCtrl;
	pclWndInfo->m_pclAdData = pData;

	return true;
}

bool CTabDlgSpecActTesting2::_FillMFCRibbonBarData( CMacroWndInfo *pclWndInfo, CWnd *pFirstWnd, CWnd *pCurrentWnd, CMacro *pclMacro )
{
	if( NULL == pclWndInfo || NULL == pFirstWnd || NULL == pCurrentWnd || NULL == pclMacro )
	{
		return false;
	}

	CMFCRibbonBar *pclMFCRibbonBar = dynamic_cast<CMFCRibbonBar*>( pCurrentWnd );

	if( NULL == pclMFCRibbonBar )
	{
		return false;
	}

	CMacroMFCRibbonBarData *pData = new CMacroMFCRibbonBarData();

	if( NULL == pData )
	{
		return false;
	}

	CPoint pt = pclMacro->m_pt;

	if( pFirstWnd->GetSafeHwnd() != pCurrentWnd->GetSafeHwnd() )
	{
		pFirstWnd->ClientToScreen( &pt );
		pCurrentWnd->ScreenToClient( &pt );
	}

	CMFCRibbonCategory *pclMFCRibbonCategory = pclMFCRibbonBar->GetActiveCategory();

	if( NULL == pclMFCRibbonCategory )
	{
		delete pData;
		return false;
	}

	CString str = _T("    ");
	str += pclMFCRibbonCategory->GetName();
	m_List.AddString( str );

	pclWndInfo->m_iAdDataID = AdData_CMFCRibbonBar;
	pclWndInfo->m_pclAdData = pData;

	return true;
}

void CTabDlgSpecActTesting2::_ClearAllMacro()
{
	for( vecMacroIter macroIter = m_vecMacroSaved.begin(); macroIter != m_vecMacroSaved.end(); macroIter++ )
	{
		if( NULL != *macroIter )
		{
			for( vecWndInfoIter wndInfoIter = (*macroIter)->m_vecWndHierarchy.begin(); wndInfoIter != (*macroIter)->m_vecWndHierarchy.end(); wndInfoIter++ )
			{
				if( NULL != *wndInfoIter )
				{
					delete *wndInfoIter;
				}
			}
			
			delete *macroIter;
		}
	}

	m_vecMacroSaved.clear();
}
