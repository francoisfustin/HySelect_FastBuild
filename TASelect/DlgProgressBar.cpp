#include "stdafx.h"
#include "TASelect.h"
#include "DlgProgressBar.h"


IMPLEMENT_DYNAMIC( CDlgProgressBar, CDialogEx )

CDlgProgressBar::CDlgProgressBar( CWnd* pParent, CString Title )
	: CDialogEx( CDlgProgressBar::IDD, pParent )
{
	// Save the title name.
	m_strTitle = Title;
	m_bModeless = false;
	m_nLowerLimitProgBar = 0;
	m_nUpperLimitProgBar = 100;
}

CDlgProgressBar::CDlgProgressBar( CWinApp* pWnd, CString Title)	// modeless constructor
{
	// Save the title name
	m_strTitle = Title;
	m_bModeless = false;
	m_nLowerLimitProgBar = 0;
	m_nUpperLimitProgBar = 100;
}

int CDlgProgressBar::Display( bool fModeless )
{
	m_bModeless = fModeless;
	int iReturn;
	if( true == fModeless )
	{
		iReturn = CDialogEx::Create( CDlgProgressBar::IDD );
		ShowWindow( SW_SHOW );
	}
	else
		iReturn = DoModal();

	return iReturn;
}

void CDlgProgressBar::UpdateProgress( int iValue, CString strItemName )
{
	// Refresh window if Cancel button was clicked and the user wanted to continue the copy.
	if( true == m_bModeless )
		ShowWindow( SW_SHOW );

	m_ProgressBar.SetPos( iValue );

	// Change dialog strings.
	GetDlgItem( IDC_STATIC_PROGRESSBAR_NAME )->SetWindowText( strItemName );
	
	CString str;
	str.Format( _T("%i/%i"), iValue, m_nUpperLimitProgBar );
	GetDlgItem( IDC_STATIC_PROGRESSBAR_COUNT )->SetWindowText( str );
}

void CDlgProgressBar::SetRange( short nLower, short nUpper )
{
	m_nUpperLimitProgBar = nUpper;
	m_nLowerLimitProgBar = nLower;
}

BEGIN_MESSAGE_MAP( CDlgProgressBar, CDialogEx )
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CDlgProgressBar::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_PROGRESSBAR, m_ProgressBar );
}

BOOL CDlgProgressBar::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	// Center the dialog to the program.
	CenterWindow();

	// Initialize dialog title.
	SetWindowText( m_strTitle );
	
	CString str;
	str.Format( _T("0/%i"), m_nUpperLimitProgBar );
	GetDlgItem( IDC_STATIC_PROGRESSBAR_COUNT )->SetWindowText( str );

	// Set the range for the progress bar.
	m_ProgressBar.SetRange( m_nLowerLimitProgBar, m_nUpperLimitProgBar );
		
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgProgressBar::OnClose()
{
	CDialogEx::OnClose();
}
