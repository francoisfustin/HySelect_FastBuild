#include "stdafx.h"


#include "TASelect.h"
#include "Utilities.h"
#include "DlgExceptionMessage.h"

IMPLEMENT_DYNAMIC( CDlgExceptionMessage, CDialogEx )

CDlgExceptionMessage::CDlgExceptionMessage( bool bCurrentProjectSaved, bool bMiniDumpCreated, CString strTimestamp, CWnd *pParent )
	: CDialogEx( CDlgExceptionMessage::IDD, pParent )
{
	m_bCurrentProjectSaved = bCurrentProjectSaved;
	m_bMiniDumpCreated = bMiniDumpCreated;
	m_strTimestamp = strTimestamp;
	m_hProgramIcon = NULL;
}

CDlgExceptionMessage::~CDlgExceptionMessage()
{
	if( INVALID_HANDLE_VALUE != m_hProgramIcon )
	{
		DeleteObject( m_hProgramIcon );
	}
}

BEGIN_MESSAGE_MAP( CDlgExceptionMessage, CDialogEx )
END_MESSAGE_MAP()

void CDlgExceptionMessage::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICPROGRAMICON, m_StaticProgramIcon );
	DDX_Control( pDX, IDC_STATICMAINTITLEBACKGROUND, m_ExtStaticMainTitleBackground );
	DDX_Control( pDX, IDC_STATICMAINTITLETEXT, m_ExtStaticMainTitleText );
	DDX_Control( pDX, IDC_STATICSUBTITLEBACKGROUND, m_ExtStaticSubTitleBackground );
	DDX_Control( pDX, IDC_STATICSUBTITLETEXT, m_ExtStaticSubTitleText );
	DDX_Control( pDX, IDC_STATICMSGPROJECTSAVESTATUS, m_ExtStaticMsgProjectSaveStatus );
	DDX_Control( pDX, IDC_STATICMSGSAVEPROJECTFILE, m_ExtStaticMsgProjectSaveFile );
	DDX_Control( pDX, IDC_STATICMSGCRASHDUMPSTATUS, m_ExtStaticMsgCrashDumpStatus );
	DDX_Control( pDX, IDC_STATICMSGCRASHDUMPFILE, m_ExtStaticMsgCrashDumpFile );
	DDX_Control( pDX, IDC_STATICMSGCRASHDUMPSENDREPORT, m_ExtStaticMsgCrashDumpSendReport );
}

BOOL CDlgExceptionMessage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGEXCEPTIONMSG_DIALOGTITLE );
	SetWindowText( str );

	m_StaticProgramIcon.SetBackColor( _TAH_ORANGE );
	m_StaticProgramIcon.SetIcon( (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_HYSELECT ), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR ) );

	m_ExtStaticMainTitleBackground.SetBackColor( _TAH_ORANGE );

	CString strProgramName = TASApp.LoadLocalizedString( AFX_IDS_APP_TITLE );
	m_ExtStaticMainTitleText.SetWindowText( strProgramName );
	m_ExtStaticMainTitleText.SetTextColor( _WHITE );
	m_ExtStaticMainTitleText.SetFontBold( true );
	m_ExtStaticMainTitleText.SetFontSize( 22 );

	m_ExtStaticSubTitleBackground.SetBackColor( _TAH_ORANGE_LIGHT );

	// Subtitle.
	m_ExtStaticSubTitleText.SetFontBold( true );
	m_ExtStaticSubTitleText.SetFontSize( 8 );
	FormatString( str, IDS_DLGEXCEPTIONMSG_SUBTITLE, strProgramName );
	m_ExtStaticSubTitleText.SetWindowText( str );

	// Project save status.
	CRect rectClient;
	m_ExtStaticMsgProjectSaveStatus.GetClientRect( &rectClient );

	// Retrieve Y coordinate in regards to the dialog.
	CRect rectStatic = rectClient;
	m_ExtStaticMsgProjectSaveStatus.ClientToScreen( &rectStatic );
	ScreenToClient( &rectStatic );
	int iY = rectStatic.top;

	CDC *pDC = m_ExtStaticMsgProjectSaveStatus.GetDC();
	CFont *pOldFont = pDC->SelectObject( m_ExtStaticMsgProjectSaveStatus.GetFont() );

	if( true == m_bCurrentProjectSaved )
	{
		FormatString( str, IDS_DLGEXCEPTIONMSG_SAVEPROJECTSUCCESS, strProgramName );
	}
	else
	{
		FormatString( str, IDS_DLGEXCEPTIONMSG_SAVEPROJECTFAILED, strProgramName );
	}

	pDC->DrawText( str, rectClient, DT_CALCRECT | DT_LEFT | DT_WORDBREAK );
	m_ExtStaticMsgProjectSaveStatus.SetWindowPos( NULL, -1, -1, rectClient.Width(), rectClient.Height(), SWP_NOMOVE | SWP_NOZORDER );
	m_ExtStaticMsgProjectSaveStatus.SetWindowText( str );
	iY += rectClient.Height();
	m_ExtStaticMsgProjectSaveStatus.ReleaseDC( pDC );

	// Path of the project saved or the last backup done by autosave.
	if( true == m_bCurrentProjectSaved )
	{
		str.Format( _T("%s%s-project.tsp"), TASApp.GetLogsPath(), m_strTimestamp );
	}
	else
	{
		str = TASApp.GetAutoSavePath();
	}

	m_ExtStaticMsgProjectSaveFile.GetClientRect( &rectClient );
	rectStatic = rectClient;
	m_ExtStaticMsgProjectSaveFile.ClientToScreen( &rectStatic );
	ScreenToClient( &rectStatic );

	pDC = m_ExtStaticMsgProjectSaveFile.GetDC();
	pOldFont = pDC->SelectObject( m_ExtStaticMsgProjectSaveStatus.GetFont() );
	pDC->DrawText( str, rectClient, DT_CALCRECT | DT_LEFT | DT_WORDBREAK );
	iY += 15;
	m_ExtStaticMsgProjectSaveFile.SetWindowPos( NULL, rectStatic.left, iY, rectClient.Width(), rectClient.Height(), SWP_NOZORDER );

	m_ExtStaticMsgProjectSaveFile.SetWindowText( str );
	iY += rectClient.Height();
	m_ExtStaticMsgProjectSaveFile.ReleaseDC( pDC );

	// Crash dump status.
	if( true == m_bMiniDumpCreated )
	{
		FormatString( str, IDS_DLGEXCEPTIONMSG_CRASHDUMPSUCCESS, strProgramName );
	}
	else
	{
		FormatString( str, IDS_DLGEXCEPTIONMSG_CRASHDUMPFAILED, strProgramName );
	}

	m_ExtStaticMsgCrashDumpStatus.GetClientRect( &rectClient );
	rectStatic = rectClient;
	m_ExtStaticMsgCrashDumpStatus.ClientToScreen( &rectStatic );
	ScreenToClient( &rectStatic );

	pDC = m_ExtStaticMsgCrashDumpStatus.GetDC();
	pOldFont = pDC->SelectObject( m_ExtStaticMsgProjectSaveStatus.GetFont() );
	pDC->DrawText( str, rectClient, DT_CALCRECT | DT_LEFT | DT_WORDBREAK );
	iY += 15;
	m_ExtStaticMsgCrashDumpStatus.SetWindowPos( NULL, rectStatic.left, iY, rectClient.Width(), rectClient.Height(), SWP_NOZORDER );

	m_ExtStaticMsgCrashDumpStatus.SetWindowText( str );
	iY += rectClient.Height();
	m_ExtStaticMsgCrashDumpStatus.ReleaseDC( pDC );
	
	// Logs path.
	if( true == m_bMiniDumpCreated )
	{
		str.Format( _T("%s%s-crashdump.dmp"), TASApp.GetLogsPath(), m_strTimestamp );

		m_ExtStaticMsgCrashDumpFile.GetClientRect( &rectClient );
		rectStatic = rectClient;
		m_ExtStaticMsgCrashDumpFile.ClientToScreen( &rectStatic );
		ScreenToClient( &rectStatic );

		pDC = m_ExtStaticMsgCrashDumpFile.GetDC();
		pOldFont = pDC->SelectObject( m_ExtStaticMsgProjectSaveStatus.GetFont() );
		pDC->DrawText( str, rectClient, DT_CALCRECT | DT_LEFT | DT_WORDBREAK );
		iY += 15;
		m_ExtStaticMsgCrashDumpFile.SetWindowPos( NULL, rectStatic.left, iY, rectClient.Width(), rectClient.Height(), SWP_NOZORDER );

		m_ExtStaticMsgCrashDumpFile.SetWindowText( str );
		iY += rectClient.Height();
		m_ExtStaticMsgCrashDumpFile.ReleaseDC( pDC );
	}
	else
	{
		m_ExtStaticMsgCrashDumpFile.EnableWindow( FALSE );
		m_ExtStaticMsgCrashDumpFile.ShowWindow( SW_HIDE );
	}

	// Send crash dump file or contact team if no crash dump.
	if( true == m_bMiniDumpCreated )
	{
		FormatString( str, IDS_DLGEXCEPTIONMSG_SENDREPORT, strProgramName );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGEXCEPTIONMSG_CONTACTTEAM );
	}

	m_ExtStaticMsgCrashDumpSendReport.GetClientRect( &rectClient );
	rectStatic = rectClient;
	m_ExtStaticMsgCrashDumpSendReport.ClientToScreen( &rectStatic );
	ScreenToClient( &rectStatic );

	pDC = m_ExtStaticMsgCrashDumpSendReport.GetDC();
	pOldFont = pDC->SelectObject( m_ExtStaticMsgProjectSaveStatus.GetFont() );
	pDC->DrawText( str, rectClient, DT_CALCRECT | DT_LEFT | DT_WORDBREAK );
	iY += 15;
	m_ExtStaticMsgCrashDumpSendReport.SetWindowPos( NULL, rectStatic.left, iY, rectClient.Width(), rectClient.Height(), SWP_NOZORDER );

	m_ExtStaticMsgCrashDumpSendReport.SetWindowText( str );
	m_ExtStaticMsgCrashDumpSendReport.ReleaseDC( pDC );

	return TRUE;
}
