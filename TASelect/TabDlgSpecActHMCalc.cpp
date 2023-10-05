#include "stdafx.h"


#include "TASelect.h"
#include "Database.h"
#include "DataBObj.h"
#include "DataStruct.h"
#include "utilities.h"
#include "HydroMod.h"
#include "TabDlgSpecActHMCalc.h"

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

CTabDlgSpecActHMCalc::CTabDlgSpecActHMCalc( CWnd* pParent )
	: CDialogExt( CTabDlgSpecActHMCalc::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
	m_hFolderIcon = (HICON)INVALID_HANDLE_VALUE;
	m_hFolderBitmap = (HBITMAP)INVALID_HANDLE_VALUE;
	m_strHMDropFileName = _T("");
}

CTabDlgSpecActHMCalc::~CTabDlgSpecActHMCalc()
{
	if( INVALID_HANDLE_VALUE != m_hFolderIcon )
	{
		DeleteObject( m_hFolderIcon );
	}

	if( INVALID_HANDLE_VALUE != m_hFolderBitmap )
	{
		DeleteObject( m_hFolderBitmap );
	}
}

BEGIN_MESSAGE_MAP(CTabDlgSpecActHMCalc, CDialogExt)
	ON_BN_CLICKED( IDC_CHECKDROPHMCALC, &OnBnClickedDropHMCalc )
	ON_BN_CLICKED( IDC_BUTTONDROPFILE, &OnBnClickedDropFile )
	ON_EN_KILLFOCUS( IDC_EDITDROPFILENAME, &OnEnKillFocusDropFileName )
END_MESSAGE_MAP()

void CTabDlgSpecActHMCalc::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKDROPHMCALC, m_CheckDropHMCalc );
	DDX_Control( pDX, IDC_BUTTONDROPFILE, m_ButtonDropFile );
	DDX_Control( pDX, IDC_EDITDROPFILENAME, m_EditDropFileName );
}

BOOL CTabDlgSpecActHMCalc::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	int iActivated = TASApp.GetProfileInt( _T("Testing"), _T("HMCalc - Drop activated"), BST_UNCHECKED );
	m_CheckDropHMCalc.SetCheck( iActivated );
	_SetState();

	m_strHMDropFileName = TASApp.GetProfileString( _T("Testing"), _T("HMCalc - Drop file name"), _T("c:\\temp\\logfile1.txt") );
	m_EditDropFileName.SetWindowText( m_strHMDropFileName );

	m_hFolderIcon = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_BROWSE ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	
	if( INVALID_HANDLE_VALUE == m_hFolderIcon )
	{
		return FALSE;
	}

	m_ButtonDropFile.SetIcon( m_hFolderIcon );

	return TRUE;
}

void CTabDlgSpecActHMCalc::OnBnClickedDropHMCalc()
{
	m_clHydroModDebugLog.ComputeAllLog_Activate( ( BST_CHECKED == m_CheckDropHMCalc.GetCheck() ) ? true : false );
	TASApp.WriteProfileInt( _T("Testing"), _T("HMCalc - Drop activated"), m_CheckDropHMCalc.GetCheck() );
	m_clHydroModDebugLog.ComputeAllLog_SetLogFileName( m_strHMDropFileName );
	_SetState();
}

void CTabDlgSpecActHMCalc::OnBnClickedDropFile()
{
	CFileDialog cFileDlg( TRUE, _T("txt"), _T("*.txt"), OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, _T("Text Files (*.txt)|*.txt"), this );

	if( IDOK == cFileDlg.DoModal () )
	{
		m_strHMDropFileName = cFileDlg.GetPathName();
		m_EditDropFileName.SetWindowText( m_strHMDropFileName );
		m_EditDropFileName.SetSel( m_strHMDropFileName.GetLength(), m_strHMDropFileName.GetLength() );
		TASApp.WriteProfileString( _T("Testing"), _T("HMCalc - Drop file name"), (LPCTSTR)m_strHMDropFileName );
	}
}

void CTabDlgSpecActHMCalc::OnEnKillFocusDropFileName()
{
	CString strText;
	int iLineLength = m_EditDropFileName.LineLength( m_EditDropFileName.LineIndex( 0 ) );
	m_EditDropFileName.GetLine( 0, strText.GetBuffer( iLineLength ), iLineLength );
	m_strHMDropFileName = strText;
	strText.ReleaseBuffer();
	TASApp.WriteProfileString( _T("Testing"), _T("HMCalc - Drop file name"), (LPCTSTR)m_strHMDropFileName );

	m_clHydroModDebugLog.ComputeAllLog_SetLogFileName( m_strHMDropFileName );
}

void CTabDlgSpecActHMCalc::_SetState( void )
{
	if( BST_CHECKED == m_CheckDropHMCalc.GetCheck() )
	{
		GetDlgItem( IDC_STATICDROPFILE )->EnableWindow( TRUE );
		m_EditDropFileName.EnableWindow( TRUE );
		m_ButtonDropFile.EnableWindow( TRUE );
	}
	else 
	{
		GetDlgItem( IDC_STATICDROPFILE )->EnableWindow( FALSE );
		m_EditDropFileName.EnableWindow( FALSE );
		m_ButtonDropFile.EnableWindow( FALSE );
	}
}
