#include "stdafx.h"

#include "TASelect.h"
#include "DlgLicense.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC( CDlgLicense, CDialogEx )

CDlgLicense::CDlgLicense( CWnd *pParent )
	: CDialogEx( IDD_DLGLICENSE, pParent )
{
}

void CDlgLicense::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITLICENSE, m_EditLicense );
}

BOOL CDlgLicense::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGLICENSE_TITLE );
	SetWindowText(str);

	m_EditLicense.SetBlockSelection( true );
	m_EditLicense.SetBlockCursorChange( true );
	_ShowLicense();

	return TRUE;
}

void CDlgLicense::_ShowLicense()
{
	CString strDocumentFolder = TASApp.GetDocumentsFolderForHelp();
	CString strFilename = strDocumentFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _HELP_DIRECTORY );

	// Retrieve the current documentation language.
	CString strDocLanguage = TASApp.GetDocLanguage();
	strFilename += TASApp.GetDocDb().GetLangDoc( _string( strDocLanguage ) ).c_str() + CString( _T("\\") ) + CString( _T("License.txt") );

	CFileFind finder;
	BOOL bFound = finder.FindFile( strFilename );

	if( FALSE == bFound )
	{
		strFilename += CString( _T("en\\") ) + CString( _T("License.txt") );
		bFound = finder.FindFile( strFilename );
	}

	CString strLicense( _T("") );

	if( TRUE == bFound )
	{
		// Try to read it.
		CFile file;
		file.Open( strFilename, CFile::modeRead | CFile::typeUnicode );
		file.Read( strLicense.GetBufferSetLength( ( (int)file.GetLength() >> 1 ) + 1 ), (int)file.GetLength() );
		strLicense.SetAt( (int)file.GetLength() >> 1, _T('\0') ); 
		file.Close();
	}
	else
	{
		// The "license.txt" file has not been found.\r\nPlease be sure to have well updated HySelect.
		strLicense = TASApp.LoadLocalizedString( IDS_DLGLICENSE_NOTFOUND );	
	}

	m_EditLicense.SetWindowTextW( strLicense );
}
