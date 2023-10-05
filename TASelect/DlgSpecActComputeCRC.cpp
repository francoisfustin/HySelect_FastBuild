#include "stdafx.h"


#include "mainfrm.h"
#include "taselectdoc.h"
#include "Utilities.h"
#include "DlgLeftTabSelManager.h"
#include "DlgSpecActComputeCRC.h"
#include "CRC32.h"

CDlgSpecActComputeCRC::CDlgSpecActComputeCRC( CWnd *pParent )
	: CDialogExt( CDlgSpecActComputeCRC::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
}

BEGIN_MESSAGE_MAP( CDlgSpecActComputeCRC, CDialogExt )
	ON_BN_CLICKED( IDC_BUTTONFILETOCOMPUTE, &OnBnClickedOpenFileToCompute )
	ON_BN_CLICKED( IDC_BUTTONCOMPUTECRC, &OnBnClickedComputeCRC )
	ON_BN_CLICKED( IDC_BUTTONCLEAR, &OnBnClickedClear )
END_MESSAGE_MAP()

void CDlgSpecActComputeCRC::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFILETOCOMPUTE, m_EditFileToCompute );
	DDX_Control( pDX, IDC_LIST, m_List );
}

BOOL CDlgSpecActComputeCRC::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CString str = TASApp.GetProfileString( _T("Testing"), _T("ComputeCRC"), _T("") );
	m_EditFileToCompute.SetWindowText( str );

	m_List.ModifyStyle( LBS_SORT, 0 );

	return TRUE;
}

void CDlgSpecActComputeCRC::OnBnClickedOpenFileToCompute()
{
	HRESULT hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );

	if( FAILED( hr ) )
	{
		return;

	}

	CString str;
	m_EditFileToCompute.GetWindowText( str );

	IFileOpenDialog *pFileDialog;
	hr = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>( &pFileDialog ) );

	if( FAILED( hr ) )
	{
		return;
	}
	
	pFileDialog->SetOptions( FOS_FILEMUSTEXIST );

	hr = pFileDialog->Show( NULL );

	if( FAILED( hr ) )
	{
		return;
	}

	IShellItem *pItem;
	hr = pFileDialog->GetResult( &pItem );

	if( FAILED( hr ) )
	{
		return;
	}

	PWSTR pszFilePath;
	hr = pItem->GetDisplayName( SIGDN_FILESYSPATH, &pszFilePath );

	if( FAILED( hr ) )
	{
		return;
	}

	m_EditFileToCompute.SetWindowText( pszFilePath );
	TASApp.WriteProfileString( _T("Testing"), _T("ComputeCRC"), pszFilePath );

	CoTaskMemFree( pszFilePath );
	pItem->Release();
	pFileDialog->Release();
	CoUninitialize();
}

void CDlgSpecActComputeCRC::OnBnClickedComputeCRC()
{
	CString strFileToCompute;
	m_EditFileToCompute.GetWindowText( strFileToCompute );

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Start...") ) );

	CCRC32 clCRC32;
	unsigned int uiCRC = clCRC32.Get_CRC( strFileToCompute );

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Finished.") ) );

	CString strCRC;
	strCRC.Format( _T("CRC computed is: 0x%08X"), uiCRC );
	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), strCRC ) );
}

void CDlgSpecActComputeCRC::OnBnClickedClear()
{
	m_List.ResetContent();
}
