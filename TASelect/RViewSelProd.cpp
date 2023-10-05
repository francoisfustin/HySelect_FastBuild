#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "HydroMod.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewSelProd.h"

CRViewSelProd *pRViewSelProd = NULL;

IMPLEMENT_DYNCREATE( CRViewSelProd, CFormViewEx )

CRViewSelProd::CRViewSelProd()
	: CFormViewEx( CRViewSelProd::IDD )
{
	memset( m_arwTemplate, 0, sizeof( m_arwTemplate ) );
	m_pclContainerForExport = NULL;
	m_pclContainerForPrint = NULL;
	m_pTADB = TASApp.GetpTADB();
	pRViewSelProd = this;
}

CRViewSelProd::~CRViewSelProd()
{
	if( NULL != m_pclContainerForExport )
	{
		delete m_pclContainerForExport;
	}

	if( NULL != m_pclContainerForPrint )
	{
		delete m_pclContainerForPrint;
	}

	pRViewSelProd = NULL;
}

void CRViewSelProd::OnEnterTab( bool bComeFromProductSelTab )
{
	if( NULL == m_SelProdDockPane.GetSafeHwnd() )
	{
		_InitSelProdDockPane();
	}

	if( NULL == m_SelProdDockPane.GetSafeHwnd() )
	{
		return;
	}

	m_SelProdDockPane.ShowPane( TRUE, FALSE, TRUE );
	RedrawRightView( false, false, false, bComeFromProductSelTab );
}

void CRViewSelProd::OnLeaveTab()
{
	if( NULL == m_SelProdDockPane.GetSafeHwnd() )
	{
		return;
	}
		
	m_SelProdDockPane.ShowPane( FALSE, FALSE, FALSE );
}

void CRViewSelProd::RedrawRightView( bool bResetOrder, bool bPrint, bool bExport, bool bComeFromProductSelTab )
{
	if( NULL == m_SelProdDockPane.GetSafeHwnd() )
	{
		return;
	}

	bool bIsActiveView = ( this == pMainFrame->GetActiveView() ) ? true : false;
	
	if( true == TASApp.GetpTADS()->IsRefreshResults() )
	{
		if( true == bIsActiveView )
		{
			AfxGetMainWnd()->BeginWaitCursor();
		}

		m_SelProdDockPane.RedrawAll( bResetOrder, bPrint, bExport, bComeFromProductSelTab );

		if( true == bIsActiveView )
		{
			AfxGetMainWnd()->EndWaitCursor();
		}
	}
	else if( true == bComeFromProductSelTab )
	{
		m_SelProdDockPane.SynchronizePageWithLeftTab();
	}
}

CContainerForPrint *CRViewSelProd::GetContainerToPrint()
{
	if( NULL == m_pclContainerForPrint )
	{
		m_pclContainerForPrint = new CContainerForPrint( this );
	}

	return m_pclContainerForPrint;
}

bool CRViewSelProd::GoToPage( CDB_PageSetup::enCheck ePage )
{
	if( NULL == m_SelProdDockPane.GetSafeHwnd() || false == m_SelProdDockPane.IsInitialized() )
	{
		return false;
	}

	return m_SelProdDockPane.GoToPage( ePage );
}

void CRViewSelProd::OnFileExportSelp( CString strFn )
{
	if( NULL == m_pclContainerForExport )
	{
		m_pclContainerForExport = new CContainerForExcelExport( this );
	}

	if( NULL == m_pclContainerForExport )
	{
		return;
	}

	if( false == m_pclContainerForExport->Init() )
	{
		return;
	}

	if ( true == strFn.IsEmpty() )
	{
		CString PrjDir = GetProjectDirectory();

		// Load file filter, compose file name and initialize 'CFileDialog'.
		CString str, strExt, strName;
		str = TASApp.LoadLocalizedString( IDS_EXPORTXLSXFILTER );
		strExt = _T("xlsx");
		strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

		CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, (LPCTSTR)str, NULL );
		dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;
		
		if( IDOK != dlg.DoModal() )
		{
			return;
		}
		else 
		{
			strFn = dlg.GetPathName();
		}
	}

	if( true != strFn.IsEmpty() )
	{
		BeginWaitCursor();
		m_pclContainerForExport->Export( strFn );
		EndWaitCursor();
	}
}

void CRViewSelProd::OnFileExportSelInOneSheet( CString strFn )
{
	if( NULL == m_pclContainerForExport )
	{
		m_pclContainerForExport = new CContainerForExcelExport( this );
	}

	if( NULL == m_pclContainerForExport )
	{
		return;
	}

	if( false == m_pclContainerForExport->Init() )
	{
		return;
	}

	if ( true == strFn.IsEmpty() )
	{
		CString PrjDir = GetProjectDirectory();

		// Load file filter, compose file name and initialize 'CFileDialog'.
		CString str, strExt, strName;
		str = TASApp.LoadLocalizedString( IDS_EXPORTXLSXFILTER );
		strExt = _T("xlsx");
		strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

		CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, (LPCTSTR)str, NULL );
		dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;
		
		if( IDOK != dlg.DoModal() )
		{
			return;
		}
		else 
		{
			strFn = dlg.GetPathName();
		}
	}

	if( true != strFn.IsEmpty() )
	{
		BeginWaitCursor();
		m_pclContainerForExport->ExportInOneSheet( strFn );
		EndWaitCursor();
	}
}

void CRViewSelProd::OnFileExportTADiagnostic( CString strFn )
{
	if( NULL == m_pclContainerForExport )
	{
		m_pclContainerForExport = new CContainerForExcelExport( this );
	}

	if( NULL == m_pclContainerForExport )
	{
		return;
	}

	if( false == m_pclContainerForExport->Init() )
	{
		return;
	}

	if( true == strFn.IsEmpty() )
	{
		CString PrjDir = GetProjectDirectory();

		// Load file filter, compose file name and initialize 'CFileDialog'.
		CString str, strExt, strName;
		str = TASApp.LoadLocalizedString( IDS_EXPORTXLSXFILTER );
		strExt = _T( "xlsx" );
		strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

		CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, (LPCTSTR)str, NULL );
		dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;

		if( IDOK != dlg.DoModal() )
		{
			return;
		}
		else
		{
			strFn = dlg.GetPathName();
		}
	}

	if( true != strFn.IsEmpty() )
	{
		BeginWaitCursor();
		m_pclContainerForExport->ExportTADiagnostic( strFn );
		EndWaitCursor();
	}
}

void CRViewSelProd::OnFileExportSelpD82( bool bIsD82 )
{
	if( NULL == m_pclContainerForExport )
	{
		m_pclContainerForExport = new CContainerForExcelExport( this );
	}

	if( NULL == m_pclContainerForExport )
	{
		return;
	}

	if( false == m_pclContainerForExport->Init() )
	{
		return;
	}

	CString strFn;
	CString strPrjDir = GetProjectDirectory();

	// Load file filter, compose file name and initialize 'CFileDialog'.
	CString str, strExt, strName;
	int iExportFilter = bIsD82 ? IDS_EXPORTD82FILTER : IDS_EXPORTD81FILTER;
	str = TASApp.LoadLocalizedString( iExportFilter );
	strExt = bIsD82 ? _T("d82" ) : _T( "d81");
	strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

	CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, ( LPCTSTR )str, NULL );
	dlg.m_ofn.lpstrInitialDir = ( LPCTSTR )strPrjDir;

	if( IDOK != dlg.DoModal() )
	{
		return;
	}
	else
	{
		strFn = dlg.GetPathName();
	}

	std::vector<CArticleItem*> vArtList;

	if( true != strFn.IsEmpty() )
	{
		BeginWaitCursor();

		// Do the stuff

		long lArticleCount = m_SelProdDockPane.GetArticleList()->GetArticleGroupCount();

		for( int i = 0; i < lArticleCount; i++ )
		{

			CArticleGroup *group = m_SelProdDockPane.GetArticleList()->GetArticleGroupAt( i );


			for( int j = 0; j < group->GetArticleContainerCount(); j++ )
			{
				CArticleContainer *pArticle = NULL;

				if( j )
				{
					pArticle = group->GetNextArticleContainer();
				}
				else
				{
					pArticle = group->GetFirstArticleContainer();
				}
				
				vArtList.push_back(pArticle->GetArticleItem());

				for (int k = 0; k < pArticle->GetAccessoryCount(); k++)
				{
					if (k)
					{
						vArtList.push_back(pArticle->GetNextAccessory());
					}
					else
					{
						vArtList.push_back(pArticle->GetFirstAccessory());
					}
				}
			}
		}

		FILE *fpExportXml;
		errno_t err = _wfopen_s(&fpExportXml, strFn, _T("wb"));

		if (0 != err)
		{
			return;
		}

		int totalPrice = 0;
		int linecount = 4;
		int countDigit = 0;
		time_t t = time(0);   // get time now
		struct tm * now = localtime(&t);
		int day = now->tm_mday;
		int month = now->tm_mon + 1;
		int year = now->tm_year % 100;
		while (int(vArtList.size() / pow(10, countDigit))){ countDigit++; }
		char strPDigit[8];
		WriteGaebPCounter(countDigit, strPDigit);

		fprintf( fpExportXml, "00        %2dL                                                 %4s0000090 000001\r\n", bIsD82 ? 82 : 81 , strPDigit );
		fprintf( fpExportXml, "01Leistungsverzeichnis                    %02d.%02d.%02d                     %c  000002\r\n",day, month, year, bIsD82 ? 'X' : ' ' );
		fprintf( fpExportXml, "02Projekt                                                                 000003\r\n" );
		fprintf( fpExportXml, "08EUR   Euro                                                              000004\r\n" );

		for (unsigned int i = 0; i < vArtList.size();++i)
		{
			CArticleItem* artItem = vArtList[i];
			char counter[8];
			WriteGaebCounter(i + 1, countDigit, counter);

			WriteGaebLine21(fpExportXml, counter, artItem->GetQuantity(), &linecount);

			if (bIsD82)
			{
				totalPrice += WriteGaebLine22(fpExportXml, counter, artItem->GetPrice(), artItem->GetQuantity(), &linecount);
			}

			WriteGaebLine25(fpExportXml, CT2A(artItem->GetDescription(), CP_UTF8).m_psz, &linecount);

			_string strTender;

			if (TASApp.IsTenderText())
			{
				TASApp.GetTender().GetTenderTxt(_string(artItem->GetArticle()), strTender);
			}

			if (strTender.length())
			{
				WriteGaebLine26(fpExportXml, strTender.c_str(), &linecount);
			}
			else
			{
				WriteGaebLine26(fpExportXml, artItem->GetDescription(), &linecount);
			}
		}

		WriteGaebLine99( fpExportXml, totalPrice, vArtList.size() , &linecount );
		fclose( fpExportXml );

		EndWaitCursor();
	}
}

void CRViewSelProd::WriteGaebPCounter( int iCountDigit, char *pcCounter )
{
	int iDigit = ( iCountDigit < 2 ) ? 2 : iCountDigit;
	int iMaxCount = 4 - iDigit;

	pcCounter[0] = '\0';

	for( int i = 0; i < iDigit; ++i )
	{
		strcat( pcCounter, "P" );
	}

	while( iMaxCount-- > 0 )
	{
		strcat( pcCounter, "0" );
	}
}

void CRViewSelProd::WriteGaebCounter( int iCount, int iCountDigit, char *pcCounter )
{
	int iDigit = ( iCountDigit < 2 ) ? 2 : iCountDigit;
	int iMaxCount = 4 - iDigit;
	char strCount[32] = { 0 };
	sprintf_s( strCount, "%%%dd", iDigit );

	while( iMaxCount-- > 0 )
	{
		strcat( strCount, " " );
	}

	sprintf_s( pcCounter, 8, strCount, iCount );
}

void CRViewSelProd::WriteGaebLine21( FILE *fp, const char *pcCount, int iQuantity, int *piLineNum )
{
	fprintf( fp, "21%4s     NNN         %011dStck                                    %06d\r\n", pcCount, iQuantity * 1000, ++( *piLineNum ) );
}

int CRViewSelProd::WriteGaebLine22( FILE *fp, const char *pcCount, double dPrice, int iQuantity, int *piLineNum )
{
	int iUnitPrice = int( dPrice * 100 );
	int iTotalPrice = iUnitPrice * iQuantity;
	fprintf( fp, "22%4s      %010d  %012d                                      %06d\r\n", pcCount, iUnitPrice, iTotalPrice, ++( *piLineNum ) );
	return iTotalPrice;
}

void CRViewSelProd::WriteGaebLine25( FILE *fp, char *pstrUtf8, int *piLineNum )
{
	char *pcTemp = _strdup( pstrUtf8 );

	if( strlen( pcTemp ) > 70 )
	{
		pcTemp[70] = 0;
	}

	char tmp2[128];

	for( unsigned int i = 0; i < 70 - strlen( pcTemp ); ++i )
	{
		tmp2[i] = ' ';
		tmp2[i + 1] = '\0';
	}


	fprintf( fp, "25%s%s  %06d\r\n", pcTemp, tmp2, ++( *piLineNum ) );
	free( pcTemp );
}

void CRViewSelProd::WriteGaebLine26( FILE *fp, const wchar_t *pstrUtf8, int *piLineNum )
{
	char *pcTemp = UnicodeToCodePage( 852, pstrUtf8 );

	size_t strDescLen = strlen( pcTemp );
	size_t printCount = INT_MAX;
	size_t readPos = 0;

	while( strDescLen > readPos )
	{
		char tmp2[64];
		tmp2[55] = 0;
		printCount = 0;

		while( strDescLen > readPos && printCount < 55 )
		{
			if( pcTemp[readPos] == '\r' )
			{
				readPos++;
				tmp2[printCount++] = ' ';
				continue;
			}

			if( pcTemp[readPos] == '\n' )
			{
				readPos++;
				break;
			}

			tmp2[printCount++] = ( char )pcTemp[readPos++];
		}

		while( printCount < 55 )
		{
			tmp2[printCount++] = ' ';
		}

		tmp2[printCount++] = '\0';

		fprintf( fp, "26   %55s              %06d\r\n", tmp2, ++( *piLineNum ) );
	}

	delete[] pcTemp;
}

void CRViewSelProd::WriteGaebLine99( FILE *fp, int totalPrice, int iCount, int *piLineNum )
{
	fprintf(fp, "99%012d                                                       %05d%06d\r\n", totalPrice, iCount, ++(*piLineNum));
}

//////////////////////////////////////////////////////////////////////
// PROTECTED MEMEBERS
//////////////////////////////////////////////////////////////////////
// CRViewSelProd message handlers
BEGIN_MESSAGE_MAP( CRViewSelProd, CFormViewEx )
	ON_WM_SIZE()
	ON_MESSAGE( WM_USER_MAINFRAMERESIZE, OnMainFrameResize )
END_MESSAGE_MAP()

void CRViewSelProd::DoDataExchange( CDataExchange *pDX )
{
	CFormViewEx::DoDataExchange( pDX );
}

void CRViewSelProd::OnSize( UINT nType, int cx, int cy )
{
	// Do not call base class to not interfere with the 'CSelProdDockPane'.
}

LRESULT CRViewSelProd::OnMainFrameResize( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_SelProdDockPane.GetSafeHwnd() )
	{
		return 0;
	}

	// Because 'm_SelProdDockPane' is docked to the bottom of the main frame, when resizing the main frame the 'm_SelProdDockPane' width
	// is automatically computed. But not the width, this is why we need to do it each time the main frame size is changed.
	CDockingManager *pclDockingManager = pMainFrame->GetDockingManager();

	if( NULL == pclDockingManager )
	{
		return 0;
	}

	CRect rectMainFrame;
	pMainFrame->GetWindowRect( &rectMainFrame );

	// Heights.
	CMFCRibbonBar *pclRibbonBar = pMainFrame->GetRibbonBar();

	if( NULL == pclRibbonBar )
	{
		return 0;
	}

	CRect rectRibbonBar;
	pclRibbonBar->GetWindowRect( &rectRibbonBar );

	CMFCStatusBar *pclStatusBar = pMainFrame->GetStatusBar();

	if( NULL == pclStatusBar )
	{
		return 0;
	}

	CRect rectStatusBar;
	pclStatusBar->GetWindowRect( &rectStatusBar );

	// Normally available height is a little bit lower. Because there is a border around the 'm_SelProdDockPane' and also there is the slider at the
	// top of the dockable plane even if this one is blocked (the height of this slider is 4 pixels). But it's not important because after calling
	// 'SetWindowPos' we call 'AdjustDockingLayout' to let mainframe correctly adjust the dimensions.
	int iAvailableHeight = rectMainFrame.Height() - rectRibbonBar.Height() - rectStatusBar.Height();

	// To get width.
	CRect rectClientAreaBounds = pclDockingManager->GetClientAreaBounds();

	// 'left' and 'top' can sometimes not be correct. This is why we use ribbon and left tab to correctly position the 'm_SelProdDockPane'.
	CRect rectDlgLeftTab = CRect( 0, 0, 0, 0 );

	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->GetWindowRect( &rectDlgLeftTab );
	}

	pMainFrame->ScreenToClient( &rectDlgLeftTab );
	pMainFrame->ScreenToClient( &rectRibbonBar );
	
	// Lock to no have blink each time we resize the mainframe. Because 'm_SelProdDockPane' is not correctly positioned, 'AdjustDockingLayout' below
	// will move again to the correct position.
	pMainFrame->LockWindowUpdate();
 	m_SelProdDockPane.SetWindowPos( NULL, rectDlgLeftTab.right, rectRibbonBar.bottom, rectClientAreaBounds.Width(), iAvailableHeight, SWP_NOACTIVATE );
	pMainFrame->UnlockWindowUpdate();
	
	pMainFrame->AdjustDockingLayout();

	return 0;
}

void CRViewSelProd::_InitSelProdDockPane()
{
	// CString str = TASApp.LoadLocalizedString( IDS_TOOLSDOCKABLEPANEOUTPUT_CAPTION );
	if( FALSE == m_SelProdDockPane.Create( _T("TOCHANGE"), this, CRect( 0, 0, 50, 50 ), FALSE, CSelProdDockPane::IDD, 
			WS_CHILD | WS_VISIBLE | CBRS_ALIGN_BOTTOM, AFX_CBRS_REGULAR_TABS ) )
	{
		TRACE0( "Failed to create selection product dockable pane window\n" );
		return;
	}

	// By default, we don't show the pane.
	m_SelProdDockPane.EnableGripper( FALSE );
	m_SelProdDockPane.ShowPane( FALSE, FALSE, FALSE );

	pMainFrame->DockPane( &m_SelProdDockPane );

	CPaneDivider *pclPaneDivider = m_SelProdDockPane.GetDefaultPaneDivider();

	if( NULL != pclPaneDivider )
	{
		// To avoid the possibility to resize the dockable pane when it is in full screen mode.
		pclPaneDivider->EnableWindow( FALSE );
		pclPaneDivider->ShowWindow( SW_HIDE );
	}

	// To avoid to be able to undock (floating) the dialog when in full screen.
	m_SelProdDockPane.SetControlBarStyle( 0 );

	// Resize to take all height.
	CDockingManager* pclDockingManager = pMainFrame->GetDockingManager();
 	CRect rectClientAreaBound = pclDockingManager->GetClientAreaBounds();
 	m_SelProdDockPane.CalcWindowRect( &rectClientAreaBound, CWnd::adjustBorder );
 	m_SelProdDockPane.SetWindowPos( NULL, rectClientAreaBound.left, rectClientAreaBound.top, rectClientAreaBound.Width(), rectClientAreaBound.Height(), SWP_NOACTIVATE );
	pMainFrame->AdjustDockingLayout();
}
