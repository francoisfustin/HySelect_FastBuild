#include "stdafx.h"


#include "mainfrm.h"
#include "taselectdoc.h"
#include "ProductSelectionParameters.h"
#include "DlgLeftTabSelManager.h"
#include "TUProductSelectionHelper.h"
#include "TabDlgSpecActProdSelUpdate.h"

CTabDlgSpecActTestProdSelUpdate::CTabDlgSpecActTestProdSelUpdate( CWnd *pParent )
	: CDialogExt( CTabDlgSpecActTestProdSelUpdate::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
}

BEGIN_MESSAGE_MAP( CTabDlgSpecActTestProdSelUpdate, CDialogExt )
	ON_BN_CLICKED( IDC_BUTTONOPENINPUTFILE, &OnBnClickedOpenInputFileName )
	ON_BN_CLICKED( IDC_BUTTONOPENOUTPUTFILE, &OnBnClickedOpenOutputFileName )
	ON_BN_CLICKED( IDC_BUTTONSTARTUPDATE, &OnBnClickedStartUpdate )
	ON_BN_CLICKED( IDC_BUTTONCLEAR, &OnBnClickedClear )
END_MESSAGE_MAP()

void CTabDlgSpecActTestProdSelUpdate::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITINPUTFILENAME, m_InputFileName );
	DDX_Control( pDX, IDC_EDITOUTPUTFILENAME, m_OutputFileName );
	DDX_Control( pDX, IDC_LIST, m_List );
}

BOOL CTabDlgSpecActTestProdSelUpdate::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CString str = TASApp.GetProfileString( _T("Testing"), _T("ProdSelUpdateInputFilename"), _T("") );
	m_InputFileName.SetWindowText( str );

	str = TASApp.GetProfileString( _T("Testing"), _T("ProdSelUpdateOutputFilename"), _T("") );
	m_OutputFileName.SetWindowText( str );

	m_List.ModifyStyle( LBS_SORT, 0 );

	return TRUE;
}

void CTabDlgSpecActTestProdSelUpdate::OnBnClickedOpenInputFileName()
{
	CString str;
	m_InputFileName.GetWindowText( str );
	CString strFilter = _T("Input product selection files (*.txt)|*.txt||");
	CFileDialog dlg( TRUE, _T("txt"), str, OFN_EXTENSIONDIFFERENT, (LPCTSTR)strFilter, NULL, 0, FALSE );

	if( IDOK == dlg.DoModal() )
	{
		m_InputFileName.SetWindowText( dlg.GetPathName() );
		TASApp.WriteProfileString( _T("Testing"), _T("ProdSelUpdateInputFilename"), (LPCTSTR)dlg.GetPathName() );
	}
}

void CTabDlgSpecActTestProdSelUpdate::OnBnClickedOpenOutputFileName()
{
	CString str;
	m_OutputFileName.GetWindowText( str );
	CString strFilter = _T("Output product seletion files (*.txt)|*.txt||");
	CFileDialog dlg( TRUE, _T("txt"), str, OFN_EXTENSIONDIFFERENT, (LPCTSTR)strFilter, NULL, 0, FALSE );

	if( IDOK == dlg.DoModal() )
	{
		m_OutputFileName.SetWindowText( dlg.GetPathName() );
		TASApp.WriteProfileString( _T("Testing"), _T("ProdSelUpdateOutputFilename"), (LPCTSTR)dlg.GetPathName() );
	}
}

void CTabDlgSpecActTestProdSelUpdate::OnBnClickedStartUpdate()
{
	m_clTUProdSelLauncher.SetTADB( TASApp.GetpTADB() );
	m_clTUProdSelLauncher.SetTADS( TASApp.GetpTADS() );
	m_clTUProdSelLauncher.SetUserDB( TASApp.GetpUserDB() );
	m_clTUProdSelLauncher.SetPipeDB( TASApp.GetpPipeDB() );

	CString strInputFileName;
	m_InputFileName.GetWindowText( strInputFileName );

	CString strOutputFileName;
	m_OutputFileName.GetWindowText( strOutputFileName );

	BeginWaitCursor();

	TASApp.SetUnitTest( true );

	// HYS-1192: force locale info to be in English - United States to have the same between all computers.
	LCID CurrentLocale = GetThreadLocale();

	// Prefer the 'SetThreadLocale' function instead of the '_tsetlocale' function. Because the first function will set the DEFAULT
	// regional settings corresponding to the LCID and not the one that the user can change in the Windows settings.
	SetThreadLocale( 0x0409 );

	CString strErrorMessage;
	m_clTUProdSelLauncher.LoadTestAndDropResults( strInputFileName, strOutputFileName, strErrorMessage, &m_List, true );

	// Restore regional settings.
	SetThreadLocale( CurrentLocale );

	TASApp.SetUnitTest( false );

	EndWaitCursor();
}

void CTabDlgSpecActTestProdSelUpdate::OnBnClickedClear()
{
	m_List.ResetContent();
}
