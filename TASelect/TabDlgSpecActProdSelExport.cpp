#include "stdafx.h"


#include "mainfrm.h"
#include "taselectdoc.h"
#include "ProductSelectionParameters.h"
#include "DlgLeftTabSelManager.h"
#include "TUProductSelectionHelper.h"
#include "TabDlgSpecActProdSelExport.h"


CTabDlgSpecActTestProdSelExport::CTabDlgSpecActTestProdSelExport( CWnd *pParent )
	: CDialogExt( CTabDlgSpecActTestProdSelExport::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
}

BEGIN_MESSAGE_MAP( CTabDlgSpecActTestProdSelExport, CDialogExt )
	ON_BN_CLICKED( IDC_BUTTONOPENFILE, &OnBnClickedOpenFile )
	ON_BN_CLICKED( IDC_BUTTONEXPORTSELECTION, &OnBnClickedExportSelection )
	ON_BN_CLICKED( IDC_BUTTONCLEAR, &OnBnClickedClear )
END_MESSAGE_MAP()

void CTabDlgSpecActTestProdSelExport::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFILENAME, m_EditFileName );
	DDX_Control( pDX, IDC_LIST, m_List );
}

BOOL CTabDlgSpecActTestProdSelExport::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CString str = TASApp.GetProfileString( _T("Testing"), _T("ProdSelExportFilename"), _T("") );
	m_EditFileName.SetWindowText( str );
	
	m_List.ModifyStyle( LBS_SORT, 0 );

	return TRUE;
}

void CTabDlgSpecActTestProdSelExport::OnBnClickedOpenFile()
{
	CString str;
	m_EditFileName.GetWindowText( str );
	CString strFilter = _T("Product seletion files (*.txt)|*.txt||");
	CFileDialog dlg( TRUE, _T("txt"), str, OFN_EXTENSIONDIFFERENT, (LPCTSTR)strFilter, NULL, 0, FALSE );

	if( IDOK == dlg.DoModal() )
	{
		m_EditFileName.SetWindowText( dlg.GetPathName() );
		TASApp.WriteProfileString( _T("Testing"), _T("ProdSelExportFilename"), (LPCTSTR)dlg.GetPathName() );
	}
}

void CTabDlgSpecActTestProdSelExport::OnBnClickedExportSelection()
{
	CDlgSelectionBase *pclDlgSelectionBase = pDlgLeftTabSelManager->GetCurrentLeftTabDialog();

	if( NULL == pclDlgSelectionBase )
	{
		return;
	}

	CProductSelelectionParameters *pclProdSelParams = pclDlgSelectionBase->GetProductSelectParameters();

	// Drop test into file.
	CString strFileName;
	m_EditFileName.GetWindowText( strFileName );

	BeginWaitCursor();

	TASApp.SetUnitTest( true );

	// HYS-1192: force locale info to be in English - United States to have the same between all computers.
	LCID CurrentLocale = GetThreadLocale();

	// Prefer the 'SetThreadLocale' function instead of the '_tsetlocale' function. Because the first function will set the DEFAULT
	// regional settings corresponding to the LCID and not the one that the user can change in the Windows settings.
	SetThreadLocale( 0x0409 );

	CString strErrorMessage;
	m_clTUProdSelLauncher.DropOutCurrentSelection( pclProdSelParams, strFileName, strErrorMessage, &m_List );

	// Restore regional settings.
	SetThreadLocale( CurrentLocale );

	TASApp.SetUnitTest( false );

	EndWaitCursor();
}

void CTabDlgSpecActTestProdSelExport::OnBnClickedClear()
{
	m_List.ResetContent();
}
