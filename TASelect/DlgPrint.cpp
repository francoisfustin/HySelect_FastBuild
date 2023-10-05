#include "stdafx.h"
#include "afxpriv.h"
#include <winspool.h>
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "Utilities.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "SheetHmCalc.h"
#include "DlgLeftTabProject.h"
#include "SSheetLDSumm.h"
#include "SSheetLogData.h"
#include "SSheetPanelCirc2.h"
#include "SSheetQuickMeas.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewProj.h"
#include "RViewChart.h"
#include "RViewLDSumm.h"
#include "RViewLogData.h"
#include "RViewSelProd.h"
#include "RviewGen.h"
#include "DlgHeader.h"
#include "DlgDispRef.h"

#include "DlgPrint.h"

IMPLEMENT_DYNAMIC( CDlgPrint, CDialogEx )

CDlgPrint::CDlgPrint( CWnd* pParent )
	: CDialogEx( CDlgPrint::IDD, pParent )
{
	m_pTADS = NULL;
	m_TreeItemHeight = 0;
	m_pRSV = NULL;
	m_bMainModule = false;
	m_bInitialized = false;
	m_pclTreeImageList = NULL;
}

CDlgPrint::~CDlgPrint()
{
	if( NULL != m_pclTreeImageList )
		delete m_pclTreeImageList;
}

void CDlgPrint::Print_PrintPrev( bool fPreview )
{
	int CurSel = m_CBList.GetItemData( m_CBList.GetCurSel() );
	m_pRSV = NULL;

	switch( CurSel )
	{
		case ecbSelProd:
		{
			CDB_PageSetup *pTADSPageSetup = m_pTADS->GetpPageSetup();
			ASSERT( NULL != pTADSPageSetup );

			pTADSPageSetup->ResetCheck();
			
			for( HTREEITEM hItem = m_Tree.GetRootItem(); hItem; hItem = m_Tree.GetNextSiblingItem( hItem ) )
			{
				int PageSetup = (int)m_Tree.GetItemData( hItem );
				pTADSPageSetup->SetCheck( (CDB_PageSetup::enCheck)PageSetup, m_Tree.GetCheck( hItem ) ? 1 : 0 );
			}

			if( NULL != pRViewProj )
			{
				pRViewProj->PrintSelectedProduct( false );
				m_pRSV = pRViewProj;
			
				if( true == fPreview )
				{
					PrintPreview();
				}
				else
				{
					pRViewProj->PrintSelectedProduct( true );
				}
			}

			break;
		}

		case ecbProject:
		{
			if( BST_CHECKED == m_RadProjectStructure.GetCheck() || BST_CHECKED == m_RadCondensedProjectStructure.GetCheck() )
			{
				m_CheckedList.PurgeAll();
				double Key = 0;
				
				// Initialize m_bMainModule so "Hydraulic Network" box will be the main distribution pipe.
				m_bMainModule = true;
				FillCheckedItemRankLst( m_Tree.GetRootItem(), Key );
				LPARAM lparam;
				CString str;
				m_CheckedList.GetFirst( str, lparam );

				// Change the test when the recursive function will be implemented.
				if( NULL == lparam )
				{
					m_CheckedList.GetNext( str, lparam );
				}

				// Init printing variables.
				if( NULL != pRViewProj )
				{
					m_pRSV = pRViewProj;

					if( true == fPreview ) 
					{
						pRViewProj->PrintTreeView( (CTable *)lparam, false, ( BST_CHECKED == m_RadCondensedProjectStructure.GetCheck() ) ? 1 : 0 );
						PrintPreview();
					}
					else
					{
						pRViewProj->PrintTreeView( (CTable *)lparam, true, ( BST_CHECKED == m_RadCondensedProjectStructure.GetCheck() ) ? 1 : 0 );
					}
				}
			}
			else if( BST_CHECKED == m_RadCircuitTables.GetCheck() )
			{
				m_CheckedList.PurgeAll();
				double Key = 0;
				FillCheckedItemRankLst( m_Tree.GetRootItem(), Key );

				if( NULL != pRViewProj )
				{
					pRViewProj->PrintRecursive( &m_CheckedList, false );
					m_pRSV = pRViewProj;

					if( true == fPreview ) 
					{
						PrintPreview();
					}
					else
					{
						pRViewProj->PrintRecursive( &m_CheckedList, true );
					}
				}
			}
			else if( BST_CHECKED == m_RadDetailedCircuitView.GetCheck() )
			{
				m_CheckedList.PurgeAll();
				double Key = 0;
				FillCheckedItemRankLst( m_Tree.GetRootItem(), Key );
				
				if( NULL != pRViewProj )
				{
					pRViewProj->PrintRecursive( &m_CheckedList, false );
					pRViewProj->m_fPrintCircuit = true;
					m_pRSV = pRViewProj;

					if( true == fPreview ) 
					{
						PrintPreview();
					}
					else
					{
						pRViewProj->PrintRecursive( &m_CheckedList, true );
					}
				}
			}
		}
		break;

		case ecbLogData:
		{
			m_CheckedList.PurgeAll();
			double Key = 0;
			FillCheckedItemRankLst( m_Tree.GetRootItem(), Key );
			LPARAM lparam;
			CString str;
			m_CheckedList.GetFirst( str, lparam );
			m_pRSV = NULL;
			sLogData sld = m_aLD.GetAt( lparam );
			switch( sld.type )
			{
				case eldSummary:

					if( NULL != pRViewProj )
					{
						pRViewProj->PrintLogDataSumm( false );
						m_pRSV = pRViewProj;

						if( true == fPreview ) 
						{
							PrintPreview();
						}
						else
						{
							pRViewProj->PrintLogDataSumm( true );
						}
					}

					break;

				case eldChart:

					if( NULL != pRViewChart )
					{
						m_pRSV = pRViewChart;
						pRViewChart->SetRedraw( (CLog *)sld.lparam );
					
						if( true == fPreview ) 
						{
							PrintPreview();
						}
						else
						{
							m_pRSV->Print();
						}
					}

					break;

				case eldData:

					if( NULL != pRViewProj )
					{
						pRViewProj->PrintLogData( false );
						m_pRSV = pRViewProj;
						pRViewProj->SetCurrentLog( (CLog *)sld.lparam );

						if( true == fPreview ) 
						{
							PrintPreview();
						}
						else
						{
							pRViewProj->PrintLogData( true );
						}
					}

					break;
			};
		}
		break;
		
		case ecbQuickMeas:
		{
			m_CheckedList.PurgeAll();
			double Key = 0;
			FillCheckedItemRankLst( m_Tree.GetRootItem(), Key );
			
			if( NULL != pRViewProj )
			{
				pRViewProj->PrintQuickMeas(&m_CheckedList, false );
				m_pRSV = pRViewProj;

				if( true == fPreview ) 
				{
					PrintPreview();
				}
				else
				{
					pRViewProj->PrintQuickMeas( &m_CheckedList, true );
				}
			}

			break;
		}
	}
}

BOOL CDlgPrint::IFVPN_OnPreparePrinting( CPrintInfo* pInfo )
{
	return m_pRSV->OnPreparePrinting( pInfo );
}

void CDlgPrint::IFVPN_OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo )
{
	m_pRSV->OnBeginPrinting( pDC, pInfo );
}

void CDlgPrint::IFVPN_OnPrint( CDC* pDC, CPrintInfo* pInfo )
{
	m_pRSV->OnPrint( pDC, pInfo );
}

void CDlgPrint::IFVPN_OnEndPrinting( CDC* pDC, CPrintInfo* pInfo )
{
	m_pRSV->OnEndPrinting( pDC, pInfo );
}

void CDlgPrint::IFVPN_OnEndPrintPreview( CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView )
{
	// HYS-1090: Display title bar
	CFrameWnd *pFrame = dynamic_cast<CFrameWnd *>( AfxGetThread()->m_pMainWnd );
	HWND hwnd = pFrame->GetSafeHwnd();
	LONG lStyle = WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
	lStyle |= GetWindowLong( hwnd, GWL_STYLE );
	SetWindowLong( hwnd, GWL_STYLE, lStyle );
}

void CDlgPrint::IFVPN_OnFilePrint()
{
	OnBnClickedPrint();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgPrint, CDialogEx )
	ON_WM_MOVE()
	ON_BN_CLICKED( IDC_BUTEDITHEADERFOOTER, &CDlgPrint::OnBnClickedEditHeaderFooter )
	ON_BN_CLICKED( IDC_BUTEDITPRJINFOS, &CDlgPrint::OnBnClickedEditProjectInfos )
	ON_BN_CLICKED( IDC_BUTPRINTSETUP, OnBnClickedPrintSetup )
	ON_BN_CLICKED( IDC_CHECKPRINTPRJINFO, &CDlgPrint::OnCheckClickedPrintProjectInfo )
	ON_BN_CLICKED( IDC_RADSHORTPRJSTRUCT, OnRadioClickedCondensedProjectStructure )
	ON_BN_CLICKED( IDC_RADPRJSTRUCT, OnRadioClickedProjectStructure )
	ON_BN_CLICKED( IDC_RADCIRCTAB, OnRadioClickedCircuitTables )
	ON_BN_CLICKED( IDC_RADCIRC, OnRadioClickedDetailsCircuitView )
	ON_BN_CLICKED( IDC_BUTEXTEND, OnBnClickedExtend )
	ON_BN_CLICKED( IDC_BUTCOLLAPSE, OnBnClickedCollapse )
	ON_BN_CLICKED( IDC_BUTSELECT, OnBnClickedSelect )
	ON_BN_CLICKED( IDC_BUTUNSELECT, OnBnClickedUnselect )
	ON_CBN_SELCHANGE( IDC_COMBO1, OnCbnSelChangePrintingOutput )
	ON_NOTIFY( NM_CLICK, IDC_TREE, OnNMClickTree )
	ON_NOTIFY( NM_RCLICK, IDC_TREE, OnNMRclickTree )
	ON_BN_CLICKED( IDC_BUTPREV, OnBnClickedPreview )
	ON_BN_CLICKED( IDPRINT, OnBnClickedPrint )
	ON_BN_CLICKED( IDCANCEL, &CDlgPrint::OnBnClickedCancel )
	ON_MESSAGE( WM_USER_CHECKSTATECHANGE, OnCheckStateChange )
END_MESSAGE_MAP()

BOOL CDlgPrint::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_pTADS = TASApp.GetpTADS();

	// Set the window position to the last stored position in registry.
	// If window position is not yet stored in the registry, the window is centered by default in the mainframe rect.
	CRect apprect, rect;
	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
	GetWindowRect( &rect );
	int x = ::AfxGetApp()->GetProfileInt( _T("DialogPrint"), _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( _T("DialogPrint"), _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// If the window is placed on a screen that is currently deactivated, the window is centered to the application.
	HMONITOR hMonitor = NULL;
	GetWindowRect( &rect );
	hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );
	if( NULL == hMonitor )
		CenterWindow();
	
	// Table initialization.
	sTab stab;
	stab.iIDS = IDS_HYDRAULIC_NETWORK;
	stab.iImages = 4;
	stab.idptr = TASApp.GetpTADS()->GetpHydroModTable()->GetIDPtr();
	m_aTab.Add( stab );
	
	stab.iIDS = IDS_TABCDIALOGPROJ_TAHUB;
	stab.iImages = 29;
	stab.idptr = TASApp.GetpTADS()->GetpHUBSelectionTable()->GetIDPtr();
	m_aTab.Add( stab );
	
	// String initialization.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CLOSE );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_STATICCB1 );
	GetDlgItem( IDC_STATICCB1 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTPRINT );
	m_ButPrint.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTPREVIEW );
	m_ButPreview.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_GROUPPRJ );
	m_GroupCircuit.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_RADCONDPRJSTRUCT );
	GetDlgItem( IDC_RADSHORTPRJSTRUCT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_RADPRJSTRUCT );
	GetDlgItem( IDC_RADPRJSTRUCT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_RADCIRCTAB );
	GetDlgItem( IDC_RADCIRCTAB )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_RADDETCIRCV );
	GetDlgItem( IDC_RADCIRC )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPRINT_CHECKPRINTPRJINFO );
	GetDlgItem( IDC_CHECKPRINTPRJINFO )->SetWindowText( str );
	
	m_pclTreeImageList = TASApp.GetpRCImageManager()->GetImageListCopy( CRCImageManager::ILN_ProjectTree );
	ASSERT ( NULL != m_pclTreeImageList );

	if( NULL == m_pclTreeImageList )
	{
		return FALSE;
	}

	m_pclTreeImageList->SetBkColor( CLR_NONE );

	CImageList *pclImgListPrint = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Print );
	ASSERT ( NULL != pclImgListPrint );

	if( NULL == pclImgListPrint )
	{
		return FALSE;
	}

	pclImgListPrint->SetBkColor( CLR_NONE );
	
	m_mapImageIndex.clear();
	m_mapImageIndex[CRCImageManager::ILP_AirVentSeparator] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_AirVentSeparator ) );
	m_mapImageIndex[CRCImageManager::ILP_PressureMaintenance] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_PressureMaintenance ) );
	m_mapImageIndex[CRCImageManager::ILP_BalancingValve] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_BalancingValve ) );
	m_mapImageIndex[CRCImageManager::ILP_DpController] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_DpController ) );
	m_mapImageIndex[CRCImageManager::ILP_ThermostaticValve] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_ThermostaticValve ) );
	m_mapImageIndex[CRCImageManager::ILP_HUB] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_HUB ) );
	m_mapImageIndex[CRCImageManager::ILP_Image4] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_Image4 ) );
	m_mapImageIndex[CRCImageManager::ILP_PartiallyDefined] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_PartiallyDefined ) );
	m_mapImageIndex[CRCImageManager::ILP_ArticleList] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_ArticleList ) );
	m_mapImageIndex[CRCImageManager::ILP_PipeList] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_PipeList ) );
	m_mapImageIndex[CRCImageManager::ILP_ControlValve] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_ControlValve ) );
	m_mapImageIndex[CRCImageManager::ILP_6WayValveValve] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_6WayValveValve ) );
	m_mapImageIndex[CRCImageManager::ILP_TenderText] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_TenderText ) );
	m_mapImageIndex[CRCImageManager::ILP_ShutoffValve] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_ShutoffValve ) );
	m_mapImageIndex[CRCImageManager::ILP_DpcBCValve] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_DpcBCValve ) );
	m_mapImageIndex[CRCImageManager::ILP_SafetyValve] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_SafetyValve ) );
	m_mapImageIndex[CRCImageManager::ILP_TapWaterControl] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_TapWaterControl ) );
	m_mapImageIndex[CRCImageManager::ILP_SmartControlValve] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_SmartControlValve ) );
	m_mapImageIndex[CRCImageManager::ILP_SmartDpC] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_SmartDpC ) );
	m_mapImageIndex[CRCImageManager::ILP_FloorHeatingControl] = m_pclTreeImageList->Add( pclImgListPrint->ExtractIcon( CRCImageManager::ILP_FloorHeatingControl ) );

	// Overlay printer image.
	m_Tree.SetImageList( m_pclTreeImageList, TVSIL_NORMAL );
	
	// ToolTips.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	
	CString TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTEXTEND );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTEXTEND ), TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTCOLLAPSE);
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTCOLLAPSE ), TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTSELECT);
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTSELECT ), TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTUNSELECT);
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTUNSELECT ), TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_TTEDITHEADERFOOTER);
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTEDITHEADERFOOTER ), TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_TTEDITPRJINFO);
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTEDITPRJINFOS ), TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_TTPRINTERSETUP);
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTPRINTSETUP ), TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_TTCHECKPRINTPRJINFO);
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_CHECKPRINTPRJINFO ), TTstr );

	// Load button images.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButEditHeaderFooter.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_EditHeaderFooter ) );
		m_ButEditPrjInfos.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_EditPrjInfo ) );
		m_ButPrintSetup.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_PrintSetup ) );
		m_ButExpand.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_ExpandAllTree ) );
		m_ButCollapse.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_CollapseAllTree ) );
		m_ButSelect.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_SelectAll ) );
		m_ButUnselect.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_UnselectAll ) );
	}

	// Print project informations check box.
	CDB_PageSetup *pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != pTADSPageSetup );

	m_CheckPrintPrjInfo.SetCheck( pTADSPageSetup->GetPrintPrjInfo() );

#pragma warning( disable : 4244)
	m_TreeItemHeight = m_Tree.GetItemHeight() * 1.2;
#pragma warning( default : 4244)
	InitCB();
	OnCbnSelChangePrintingOutput();
	m_RadCircuitTables.SetCheck( BST_CHECKED );
	DrawPicture();

	m_bInitialized = true;

	return TRUE;
}

void CDlgPrint::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBO1, m_CBList );
	DDX_Control( pDX, IDC_TREE, m_Tree );
	DDX_Control( pDX, IDC_BUTEXTEND, m_ButExpand );
	DDX_Control( pDX, IDC_BUTCOLLAPSE, m_ButCollapse );
	DDX_Control( pDX, IDC_BUTSELECT, m_ButSelect );
	DDX_Control( pDX, IDC_BUTUNSELECT, m_ButUnselect );
	DDX_Control( pDX, IDPRINT, m_ButPrint );
	DDX_Control( pDX, IDC_BUTPREV, m_ButPreview );
	DDX_Control( pDX, IDC_GROUPCIRCUIT, m_GroupCircuit );
	DDX_Control( pDX, IDC_RADPRJSTRUCT, m_RadProjectStructure );
	DDX_Control( pDX, IDC_RADCIRCTAB, m_RadCircuitTables );
	DDX_Control( pDX, IDC_RADCIRC, m_RadDetailedCircuitView );
	DDX_Control( pDX, IDC_BUTPRINTSETUP, m_ButPrintSetup );
	DDX_Control( pDX, IDC_RADSHORTPRJSTRUCT, m_RadCondensedProjectStructure );
	DDX_Control( pDX, IDC_PICTURECTRL, m_Picture );
	DDX_Control( pDX, IDC_CHECKPRINTPRJINFO, m_CheckPrintPrjInfo );
	DDX_Control( pDX, IDC_BUTEDITHEADERFOOTER, m_ButEditHeaderFooter );
	DDX_Control( pDX, IDC_BUTEDITPRJINFOS, m_ButEditPrjInfos );
}

void CDlgPrint::OnMove( int x, int y )
{
	CDialogEx::OnMove( x,  y );
	
	if( true == m_bInitialized )
	{
		CRect rect;
		GetWindowRect( &rect );
		::AfxGetApp()->WriteProfileInt( _T("DialogPrint"), _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( _T("DialogPrint"), _T("ScreenYPos"), y );
	}
}

void CDlgPrint::OnBnClickedEditHeaderFooter()
{
	CDB_PageSetup *pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != pTADSPageSetup );

	CDlgHeader dlg;
	dlg.Display( pTADSPageSetup );
}

void CDlgPrint::OnBnClickedEditProjectInfos()
{
	CDlgDispRef dlg;
	dlg.Display();
}

void CDlgPrint::OnBnClickedPrintSetup()
{
	TASApp.LaunchPrintSetupDlg();
	DrawPicture();
}

void CDlgPrint::OnCheckClickedPrintProjectInfo()
{
	CDB_PageSetup *pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != pTADSPageSetup );

	pTADSPageSetup->SetPrintPrjInfo( ( BST_CHECKED == m_CheckPrintPrjInfo.GetCheck() ) ? true : false );
}

void CDlgPrint::OnRadioClickedCondensedProjectStructure()
{
	// Reset all selection done before to be sure to have a maximum of one selected product.
	m_ButPrint.EnableWindow( FALSE );
	m_ButPreview.EnableWindow( FALSE );
	TreeSelect( m_Tree.GetRootItem(), FALSE, true, true );
	
	m_Tree.DeleteAllItems();
	FillHMTreeView( NULL, NULL, true );
	m_ButSelect.ShowWindow( SW_HIDE );
	m_ButUnselect.ShowWindow( SW_HIDE );
	m_ButExpand.ShowWindow( SW_SHOW );
	m_ButCollapse.ShowWindow( SW_SHOW );
	DrawPicture();
}

void CDlgPrint::OnRadioClickedProjectStructure()
{
	// Reset all selection done before to be sure to have a maximum of one selected product.
	m_ButPrint.EnableWindow( FALSE );
	m_ButPreview.EnableWindow( FALSE );
	TreeSelect( m_Tree.GetRootItem(), FALSE, true, true );
	
	m_Tree.DeleteAllItems();
	FillHMTreeView( NULL, NULL, true );
	m_ButSelect.ShowWindow( SW_HIDE );
	m_ButUnselect.ShowWindow( SW_HIDE );
	m_ButExpand.ShowWindow( SW_SHOW );
	m_ButCollapse.ShowWindow( SW_SHOW );
	DrawPicture();
}

void CDlgPrint::OnRadioClickedCircuitTables()
{
	m_Tree.DeleteAllItems();
	for( int i = 0; i < m_aTab.GetCount(); i++ )
	{
		sTab stab = m_aTab.GetAt( i );
		CTable *pTab = (CTable *)( stab.idptr.MP );
		CDS_HydroMod *pHM = (CDS_HydroMod*)( pTab->GetFirst().MP );
		// Skip if empty.
		if( NULL == pHM )
			continue;
		
		CString str = TASApp.LoadLocalizedString( stab.iIDS );
		HTREEITEM hParentItem = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, stab.iImages, stab.iImages + 1, 0, 0, NULL, TVI_ROOT, TVI_ROOT );
		m_Tree.Expand( hParentItem, TVE_EXPAND );
		FillHMTreeView( pTab, hParentItem, true );
	}
	m_ButSelect.ShowWindow( SW_SHOW );
	m_ButUnselect.ShowWindow( SW_SHOW );
	m_ButExpand.ShowWindow( SW_SHOW );
	m_ButCollapse.ShowWindow( SW_SHOW );
	DrawPicture();
}

void CDlgPrint::OnRadioClickedDetailsCircuitView()
{
	m_Tree.DeleteAllItems();
	FillHMTreeView( NULL, NULL, false );
	m_ButSelect.ShowWindow( SW_SHOW );
	m_ButUnselect.ShowWindow( SW_SHOW );
	m_ButExpand.ShowWindow( SW_SHOW );
	m_ButCollapse.ShowWindow( SW_SHOW );
	DrawPicture();
}

void CDlgPrint::OnBnClickedExtend()
{
	HTREEITEM hRI = m_Tree.GetRootItem();
	TreeExpand( hRI, TVE_EXPAND );
}

void CDlgPrint::OnBnClickedCollapse()
{
	HTREEITEM hRI = m_Tree.GetRootItem();
	TreeExpand( hRI, TVE_COLLAPSE );
}

void CDlgPrint::OnBnClickedSelect()
{
	HTREEITEM hRI = m_Tree.GetRootItem();
	TreeSelect( hRI, TRUE, true, true );
	CheckPrintButtonState();
}

void CDlgPrint::OnBnClickedUnselect()
{
	HTREEITEM hRI = m_Tree.GetRootItem();
	TreeSelect( hRI, FALSE, true, true );
	CheckPrintButtonState();
}

void CDlgPrint::OnCbnSelChangePrintingOutput()
{
	InitTreeView();
	DrawPicture();
}

void CDlgPrint::OnNMClickTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	// On a TreeView Control with the CheckBoxes, there is no notification that the check state
	// of the item has been changed, you can just determine that the user has clicked the state Icon.
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is 
	// post a user defined message as a notification that the check state has changed.
	TVHITTESTINFO ht = {0};
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( (int)(short)LOWORD( dwpos ) );
	ht.pt.y = ( (int)(short)HIWORD( dwpos ) );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( ( TVHT_ONITEMSTATEICON & ht.flags) || ( TVHT_ONITEM & ht.flags ) )
	{
		// Save current state of item in WPARAM.
		// Remark: in case of 'Project' choice in 'Printing output' combo AND in case of 'Condensed project structure' or 'Project structure'
		//         radio button in 'Project' group, we use the right click mode for check box selection in the tree.
		//         Right click mode allows to use exclusive check box mode. That means there is only one checked item in the tree at a time.
		WPARAM wpState = m_Tree.GetCheck( ht.hItem );
		if( TRUE == m_GroupCircuit.IsWindowEnabled() && ( BST_CHECKED == m_RadCondensedProjectStructure.GetCheck() || BST_CHECKED == m_RadProjectStructure.GetCheck() ) )
			wpState |= DLGPRINT_TREEEXCLUSIVEMODE;
		::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, wpState, (LPARAM)ht.hItem );
	}
	*pResult = 0;
}

void CDlgPrint::OnNMRclickTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	TVHITTESTINFO ht = {0};
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( (int)(short)LOWORD( dwpos ) );
	ht.pt.y = ( (int)(short)HIWORD( dwpos ) );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( ( TVHT_ONITEMSTATEICON & ht.flags ) || ( TVHT_ONITEM & ht.flags ) )
	{
		WPARAM wpState = m_Tree.GetCheck( ht.hItem ) | DLGPRINT_RIGHTCLICK;
		::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, wpState, (LPARAM)ht.hItem );
	}
	*pResult = 0;
}

void CDlgPrint::OnBnClickedPreview()
{
	Print_PrintPrev( true );
}

void CDlgPrint::OnBnClickedPrint()
{
	Print_PrintPrev( false );
}

void CDlgPrint::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
	::PostMessage( pMainFrame->GetSafeHwnd(), WM_USER_DLGPRINTTOCLOSE, 0, 0 );
}

LRESULT CDlgPrint::OnCheckStateChange( WPARAM wParam, LPARAM lParam )
{
	HTREEITEM hItem = (HTREEITEM)lParam;

	// Remark: bit 0: contains check state before user has clicked.
	//         bit 7: define if it's a left (0) or right (1) click.
	//         If right click, we use exclusive mode. That means, if click happened on one check box, all other ones will be unchecked.
	BOOL bState = wParam & 0x01;

	// In case of LogData  selection allow only one item checked...
	if( ecbLogData == m_CBList.GetItemData( m_CBList.GetCurSel() ) )
	{
		// UnCheck all.
		TreeSelect( m_Tree.GetRootItem(), FALSE, true, true );
		TreeSelect( hItem, !bState );
		DrawPicture();
	}
	else
	{
		// Right click (or exclusive mode).
		if( wParam & DLGPRINT_RIGHTCLICK )
		{
			TreeSelect( m_Tree.GetRootItem(), FALSE, true, true );
			TreeSelect( hItem, !bState, false, false );
		}
		else
			TreeSelect( hItem, !bState, false, false );
	}
	CheckPrintButtonState();
	return 0;
}

int CDlgPrint::InitCB( int iSelItemID )
{
	m_CBList.ResetContent();
	CString str;

	// Project.
	CTable *pTab = NULL;
	CDS_HydroMod *pHM = NULL;
	for( int i = 0; i < m_aTab.GetCount() && NULL == pHM; i++ )
	{
		pTab = (CTable *)m_aTab.GetAt( i ).idptr.MP;
		pHM = (CDS_HydroMod*)pTab->GetFirst().MP;
	}
	
	if( pHM != NULL )
	{
		// HydroMod exist.
		CString str = TASApp.LoadLocalizedString( IDS_DLGPRINT_PROJECT );
		m_CBList.SetItemData( m_CBList.AddString( str ), ecbProject );
	}

	bool fProductExist = false;
	if( NULL != pRViewSelProd )
	{
		CContainerForPrint* pclContainerForPrint = pRViewSelProd->GetContainerToPrint();				ASSERT( NULL != pclContainerForPrint );
		if( NULL != pclContainerForPrint && true == pclContainerForPrint->Init() )
		{
			for( int i = CDB_PageSetup::enCheck::FIRSTFIELD; i < CDB_PageSetup::enCheck::LASTFIELD && false == fProductExist; ++i )
			{
				fProductExist = pclContainerForPrint->HasSomethingToPrint( ( CDB_PageSetup::enCheck )i );
			}
		}
	}
	
	if( true == fProductExist )
	{
		str = TASApp.LoadLocalizedString( ecbSelProd );
		m_CBList.SetItemData( m_CBList.AddString( str ), ecbSelProd );
	}

	pTab = m_pTADS->GetpLogDataTable();
	ASSERT( NULL != pTab );

	if( _T('\0') != *pTab->GetFirst( CLASS( CDS_LoggedData ) ).ID )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGPRINT_LOGDATA );
		m_CBList.SetItemData( m_CBList.AddString( str ), ecbLogData );
	}
	else if( _T('\0') != *pTab->GetFirst( CLASS( CDS_LogDataX ) ).ID )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGPRINT_LOGDATA );
		m_CBList.SetItemData( m_CBList.AddString( str ), ecbLogData );
	}
	
	pTab = (CTable *)m_pTADS->GetpQuickMeasureTable();
	ASSERT( NULL != pTab );
	
	if( _T('\0') != *pTab->GetFirst( CLASS( CDS_HydroModX ) ).ID )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGPRINT_QUICKMEAS );
		m_CBList.SetItemData( m_CBList.AddString( str ), ecbQuickMeas );
	}
	
	if( m_CBList.GetCount() > 0 )
	{
		m_CBList.SetCurSel( 0 );
	}
	
	// Select Item if requested.
	if( iSelItemID > 0 )
	{
		for( int i = 0; i < m_CBList.GetCount(); i++ )
		{
			if( m_CBList.GetItemData( i ) == iSelItemID )
			{
				m_CBList.SetCurSel( iSelItemID );
				break;
			}
		}
	}
	if( m_CBList.GetCurSel() > 0 )
		return m_CBList.GetItemData( m_CBList.GetCurSel() );
	return 0;
}

void CDlgPrint::InitTreeView( void )
{
	if( m_Tree.GetCount() != -1 )
		m_Tree.DeleteAllItems();
	
	if( m_CBList.GetCurSel() < 0 )
		return;
	
	int CurSel = m_CBList.GetItemData( m_CBList.GetCurSel() );
	EnableTreeView( false );
	EnableGroupPrj( false );
	m_ButSelect.ShowWindow( SW_HIDE );
	m_ButUnselect.ShowWindow( SW_HIDE );
	m_ButExpand.ShowWindow( SW_HIDE );
	m_ButCollapse.ShowWindow( SW_HIDE );
	
	// Initialize parameters for insertion of items.
	switch( CurSel )
	{
		case ecbSelProd:
			EnableTreeView( true );
			FillSPTreeView();
			break;
		
		case ecbProject:
			EnableTreeView( true );
			EnableGroupPrj( true );
			OnRadioClickedCircuitTables();
			break;
		
		case ecbLogData:
			FillLDTreeView();
			EnableTreeView( true );
			m_ButExpand.ShowWindow( SW_SHOW );
			m_ButCollapse.ShowWindow( SW_SHOW );
			break;
		
		case ecbQuickMeas:
			EnableTreeView( true );
			FillQMTreeView();
			break;
	}
	CheckPrintButtonState();
}

void CDlgPrint::EnableTreeView( bool fEnable )
{
	m_Tree.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	m_ButExpand.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	m_ButCollapse.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	m_ButSelect.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	m_ButUnselect.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
}

void CDlgPrint::EnableGroupPrj( bool fEnable )
{
	m_GroupCircuit.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	m_RadCondensedProjectStructure.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	m_RadCircuitTables.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	if( true == pMainFrame->IsHMCalcChecked() )
	{
		m_RadProjectStructure.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
		m_RadDetailedCircuitView.EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}
	else
	{
		m_RadProjectStructure.EnableWindow( FALSE );
		m_RadDetailedCircuitView.EnableWindow( FALSE );
	}
	GetDlgItem( IDC_PICTURECTRL )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
}

void CDlgPrint::FillHMTreeView( CTable *pTab, HTREEITEM hParentItem, bool fOnlyModule )
{
	CString str;

	if( NULL == pTab )
	{
		pTab = m_pTADS->GetpHydroModTable();
		ASSERT( NULL != pTab );

		str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
		hParentItem = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, CRCImageManager::ILPT_HydronicNetwork, 
			CRCImageManager::ILPT_HydronicNetworkSelected, 0, 0, NULL, TVI_ROOT, TVI_ROOT );
		m_Tree.Expand( hParentItem, TVE_EXPAND );
	}
	
	CRank SortList;
	for( IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext() )
		SortList.Add( _T(""), ( (CDS_HydroMod *)IDPtr.MP )->GetPos(), (long)IDPtr.MP );
	
	LPARAM lparam;
	for( BOOL fContinue = SortList.GetFirst( str, lparam ); TRUE == fContinue; fContinue = SortList.GetNext( str, lparam ) )
	{
		CDS_HydroMod* pHM = (CDS_HydroMod*)lparam;
		HTREEITEM hChild = NULL;
		if( true == pHM->IsaModule() )
		{
			hChild = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHM->GetHMName(), CRCImageManager::ILPT_HydronicCircuit, 
				CRCImageManager::ILPT_HydronicCircuitSelected, 0, 0, (long)pHM, hParentItem, TVI_LAST );
			m_Tree.Expand( hChild, TVE_EXPAND );
			m_Tree.SetCheck( hChild, pHM->IsChecked() );
			FillHMTreeView( pHM, hChild, fOnlyModule );
		}
		else if( false == fOnlyModule )
		{
			hChild = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHM->GetHMName(), CRCImageManager::ILPT_TerminalUnit, 
				CRCImageManager::ILPT_TerminalUnitSelected, 0, 0, (long)pHM, hParentItem, TVI_LAST );
			m_Tree.SetCheck( hChild, pHM->IsChecked() );
		}
	}
	TreeExpand( hParentItem, TVE_EXPAND );
}

int CDlgPrint::TreeExpand( HTREEITEM hItem, UINT uiCode )
{
	if( NULL == hItem )
		return 0;
	
	m_Tree.Expand( hItem, uiCode );
	if( TRUE == m_Tree.ItemHasChildren( hItem ) )
	{
		HTREEITEM hChildItem = m_Tree.GetChildItem( hItem );
		TreeExpand( hChildItem, uiCode );
	}
	HTREEITEM hNextItem = m_Tree.GetNextSiblingItem( hItem );
	TreeExpand( hNextItem, uiCode );
	return 0;
}

int CDlgPrint::TreeSelect( HTREEITEM hItem, BOOL bCheck, bool fSibling, bool fChildren )
{
	if( NULL == hItem )
		return 0;
	
	m_Tree.SetCheck( hItem, bCheck );
	if( ecbProject == m_CBList.GetItemData( m_CBList.GetCurSel() ) )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)m_Tree.GetItemData( hItem );
		if( NULL != pHM )
			pHM->SetChecked( bCheck ? 1 : 0 );
	}
	else if( ecbSelProd == m_CBList.GetItemData( m_CBList.GetCurSel() ) )
	{
		CDB_PageSetup::enCheck ePS = (CDB_PageSetup::enCheck)m_Tree.GetItemData( hItem );
		
		CDB_PageSetup *pTADSPageSetup = m_pTADS->GetpPageSetup();
		ASSERT( NULL != pTADSPageSetup );
		
		pTADSPageSetup->SetCheck( ePS, ( TRUE == bCheck ) ? true : false );
		pTADSPageSetup->GetCheck( ePS );
	}

	if( m_Tree.ItemHasChildren( hItem ) && true == fChildren )
	{
		HTREEITEM hChildItem = m_Tree.GetChildItem( hItem );
		TreeSelect( hChildItem, bCheck, true, true );
	}
	if( true == fSibling )
	{
		HTREEITEM hNextItem = m_Tree.GetNextSiblingItem( hItem );
		TreeSelect( hNextItem, bCheck, true, fChildren );
	}
	return 0;
}

void CDlgPrint::FillLDTreeView( void )
{
	// Clear array with Log data pointer.
	m_aLD.RemoveAll();
	sLogData ld;
	ld.lparam = NULL;
	ld.type = eLogData::eldSummary;
	int k = m_aLD.Add( ld );
	CString str = TASApp.LoadLocalizedString( IDS_LOGGEDDATAS );

	HTREEITEM hLoggedDataBranch = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, CRCImageManager::ILPT_LogBranch, 
		CRCImageManager::ILPT_LogBranchSelected, 0, 0, NULL, TVI_ROOT, TVI_ROOT );
	
	CTable *pTab = m_pTADS->GetpLogDataTable();
	ASSERT( NULL != pTab );
	
	for( IDPTR LDIDPtr = pTab->GetFirst(); _T('\0') != *LDIDPtr.ID; LDIDPtr = pTab->GetNext() )
	{
		// The table can have CDS_LoggedData, CDS_LogDataX and TMPLOGDATA_TAB.
		// Only the two first are CLog.
		if( true == LDIDPtr.MP->IsaTable() )
		{
			continue;
		}

		CLog *pLD = (CLog *)LDIDPtr.MP;
		ASSERT( NULL != pLD );

		// Add Chart at the first level.
		ld.lparam = (LPARAM)pLD;
		ld.type = eLogData::eldChart;
		int index = m_aLD.Add( ld );
		
		HTREEITEM hLoggedData = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pLD->GetName(), CRCImageManager::ILPT_LogChart, 
			CRCImageManager::ILPT_LogChartSelected, 0, 0, index, hLoggedDataBranch, TVI_LAST );
		
		str = TASApp.LoadLocalizedString( IDS_DATALIST );
		
		ld.lparam = (LPARAM)pLD;
		ld.type = eLogData::eldData;
		index = m_aLD.Add( ld );
		
		m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, CRCImageManager::ILPT_LogData, 
			CRCImageManager::ILPT_LogDataSelected, 0, 0, index, hLoggedData, TVI_LAST );
	}

	TreeExpand( hLoggedDataBranch, TVE_EXPAND );
	m_Tree.SetCheck( hLoggedDataBranch );
}

void CDlgPrint::FillSPTreeView( void )
{
	CDB_PageSetup *pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != pTADSPageSetup );
	
	if( NULL == pTADSPageSetup || NULL == pRViewSelProd )
	{
		return;
	}

	CContainerForPrint *pclContainerForPrint = pRViewSelProd->GetContainerToPrint();
	ASSERT( NULL != pclContainerForPrint );
	
	if( NULL == pclContainerForPrint )
	{
		return;
	}

	m_Tree.SetItemHeight( m_TreeItemHeight );

	// Verify for each category is something must could be printed.

	CString str; 
	HTREEITEM hItem = NULL;

	if( NULL == pRViewSelProd )
	{
		return;
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::AIRVENTSEPARATOR ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_AIRVENTSEPARATOR );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_AirVentSeparator];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::AIRVENTSEPARATOR, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::AIRVENTSEPARATOR ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::PRESSMAINT ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_PRESSMAINT );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_PressureMaintenance];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::PRESSMAINT, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::PRESSMAINT ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::SAFETYVALVE ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_SAFETYVALVE );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_SafetyValve];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::SAFETYVALVE, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::SAFETYVALVE ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::BV ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_STATICBV );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_BalancingValve];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::BV, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::BV ) );
	}
	
	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::DPC ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_STATICDPC );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_DpController];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::DPC, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::DPC ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::DPCBCV ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_STATICDPCBCV );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_DpcBCValve];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::DPCBCV, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::DPCBCV ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::TRV ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_TRV );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_ThermostaticValve];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::TRV, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::TRV ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::FLOORHEATINGCONTROL ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_FLOORHEATINGCONTROL );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_FloorHeatingControl];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::FLOORHEATINGCONTROL, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::FLOORHEATINGCONTROL ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::TAPWATERCONTROL ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_TAPWATERCONTROL );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_TapWaterControl];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::TAPWATERCONTROL, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::TAPWATERCONTROL ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::CV ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_CV );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_ControlValve];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::CV, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::CV ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::SMARTCONTROLVALVE ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_SMARTCONTROLVALVE );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_SmartControlValve];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::SMARTCONTROLVALVE, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::SMARTCONTROLVALVE ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::SMARTDPC ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_SMARTDPC );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_SmartDpC];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::SMARTDPC, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::SMARTDPC ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::SIXWAYVALVE ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_6WAYVALVE );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_6WayValveValve];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::SIXWAYVALVE, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::SIXWAYVALVE ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::SV ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_SV );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_ShutoffValve];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::SV, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::SV ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::HUB ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_HUB );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_HUB];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::HUB, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::HUB ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::PARTDEF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_STATICPARTDEF );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_PartiallyDefined];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::PARTDEF, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::PARTDEF ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::PIPELIST ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABPIPELIST );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_PipeList];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::PIPELIST, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::PIPELIST ) );
	}

	if( true == pclContainerForPrint->HasSomethingToPrint( CDB_PageSetup::enCheck::ARTLIST ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_STATICARTLIST );
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_ArticleList];
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::ARTLIST, TVI_ROOT, TVI_LAST );
		m_Tree.SetCheck( hItem, pTADSPageSetup->GetCheck( CDB_PageSetup::enCheck::ARTLIST ) );
	}

	CTender clTender;
	if (true == pclContainerForPrint->HasSomethingToPrint(CDB_PageSetup::enCheck::TENDERTEXT) && true == clTender.IsTenderNeeded())
	{
		str = TASApp.LoadLocalizedString(IDS_DLGDIRSEL_TENDERTEXT);
		int iIndex = m_mapImageIndex[CRCImageManager::ILP_TenderText];
		hItem = m_Tree.InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, iIndex, iIndex, 0, TVIS_SELECTED, CDB_PageSetup::enCheck::TENDERTEXT, TVI_ROOT, TVI_LAST);
		m_Tree.SetCheck(hItem, pTADSPageSetup->GetCheck(CDB_PageSetup::enCheck::TENDERTEXT));
	}
}

void CDlgPrint::FillQMTreeView( void )
{
	// Get the QUICKMEAS_TAB.
	CTable *pTab = TASApp.GetpTADS()->GetpQuickMeasureTable();

	if( NULL == pTab )
	{
		return;
	}

	// Verify the number of QuickMeasurements.
	if( 0 == pTab->GetItemCount() )
	{
		return;
	}

	// Variables.
	CString str; 
	HTREEITEM hItem = NULL;

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDS_HydroModX *pHMX = NULL;
		pHMX = (CDS_HydroModX *)IDPtr.MP;
		
		if( NULL != pHMX )
		{
			// Verify the Date with the limits.
			__time32_t time32 = pHMX->GetMeasDateTime();
			tm dt;
			_gmtime32_s( &dt, &time32 );
			COleDateTime tim( dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec );
			
			// Do not show measurements outside the limits.
			if( tim < pMainFrame->GetLowerDateTime() || tim > pMainFrame->GetUpperDateTime() )
			{
				continue;
			}

			str = pHMX->GetHMName()->c_str();
			hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, (LPCTSTR)str, CRCImageManager::ILPT_QuickMeasurementBranch, 
				CRCImageManager::ILPT_QuickMeasurementBranchSelected, 0, 0, (LPARAM)pHMX, TVI_ROOT, TVI_LAST );
			m_Tree.SetCheck( hItem, TRUE );
		}
	}
}

void CDlgPrint::FillCheckedItemRankLst( HTREEITEM hItem, double& dKey )
{
	if( NULL == hItem )
		return;
	
	// The first item must be the hydraulic network in all cases.
	// Create a loop that first will implement the main hydraulic.
	// network as the main part of the pipes.
	if( m_Tree.GetRootItem() && true == m_bMainModule )
	{
		m_CheckedList.Add( m_Tree.GetItemText( hItem ), dKey, m_Tree.GetItemData( hItem ) );
		dKey++;
		m_bMainModule = false;
	}

	if( m_Tree.GetCheck( hItem ) )
	{
		m_CheckedList.Add( m_Tree.GetItemText( hItem ), dKey, m_Tree.GetItemData( hItem ) );
		dKey++;
	}
	
	if( m_Tree.ItemHasChildren( hItem ) )
	{
		HTREEITEM hChildItem = m_Tree.GetChildItem( hItem );
		FillCheckedItemRankLst( hChildItem, dKey );
	}
	HTREEITEM hNextItem = m_Tree.GetNextSiblingItem( hItem );
	FillCheckedItemRankLst( hNextItem, dKey );
}

bool CDlgPrint::IsSomethingSelected( HTREEITEM hItem )
{
	if( NULL == hItem)
		return 0;
		
	if( TRUE == m_Tree.GetCheck( hItem ) )
		return true;
	
	if( TRUE == m_Tree.ItemHasChildren( hItem ) )
	{
		HTREEITEM hChildItem = m_Tree.GetChildItem( hItem );
		if( true == IsSomethingSelected( hChildItem ) )
			return true;
	}
	
	HTREEITEM hNextItem = m_Tree.GetNextSiblingItem( hItem );
	if( true == IsSomethingSelected( hNextItem ) )
		return true;
	return false;
}

void CDlgPrint::CheckPrintButtonState( HTREEITEM hItemRoot )
{
	int CurSel = m_CBList.GetItemData( m_CBList.GetCurSel() );
	if( NULL == hItemRoot )
		hItemRoot = m_Tree.GetRootItem();
	
	HTREEITEM hItem = NULL;
	switch( CurSel )
	{
		case ecbSelProd:
			hItem = hItemRoot;
			break;	
		
		case ecbProject:
			// for project take first HM
			hItem = hItemRoot;
			break;
	
		case ecbLogData:
			// for project take first LogData
			hItem = hItemRoot;
			break;
		
		case ecbQuickMeas:
			hItem = hItemRoot;
			break;
	}
	
	if( true == IsSomethingSelected( hItem ) )
	{
		m_ButPrint.EnableWindow( true );
		m_ButPreview.EnableWindow( true );
	}
	else
	{
		m_ButPrint.EnableWindow( false );
		m_ButPreview.EnableWindow( false );
		hItemRoot = m_Tree.GetNextSiblingItem( hItemRoot );
		if( hItemRoot != NULL )
			CheckPrintButtonState( hItemRoot );
	}
}

void CDlgPrint::DrawPicture()
{
	bool fLand = false;
	PRINTDLG pd;
	pd.lStructSize = (DWORD)sizeof(PRINTDLG);
	BOOL bRet = TASApp.GetPrinterDeviceDefaults( &pd );
	
	if( TRUE == bRet )
	{
		// Protect memory handle with ::GlobalLock and ::GlobalUnlock.
		DEVMODE FAR *pDevMode = (DEVMODE FAR *)::GlobalLock( TASApp.GetHDevMode() );
		fLand = ( pDevMode->dmOrientation == DMORIENT_LANDSCAPE ) ? true : false;
		::GlobalUnlock( TASApp.GetHDevMode() );
	}
	
	USHORT usGifID = GIF_SELP_PORT;
	int CurSel = m_CBList.GetItemData( m_CBList.GetCurSel() );
	switch( CurSel )
	{
		case ecbSelProd:
			usGifID = ( true == fLand ) ? GIF_SELP_LAND : GIF_SELP_PORT;
			break;

		case ecbProject:

			if( true == pMainFrame->IsHMCalcChecked() )
			{
				if( BST_CHECKED == m_RadProjectStructure.GetCheck() )
				{
					usGifID = ( true == fLand ) ? GIF_PROJSTRUCT_LAND : GIF_PROJSTRUCT_PORT;
				}
				else if( BST_CHECKED == m_RadCondensedProjectStructure.GetCheck() )
				{
					usGifID = ( true == fLand ) ? GIF_CONDPROJSTRUCT_LAND : GIF_CONDPROJSTRUCT_PORT;
				}
				else if( BST_CHECKED == m_RadCircuitTables.GetCheck() )
				{
					usGifID = ( true == fLand ) ? GIF_CIRCTAB_LAND : GIF_CIRCTAB_PORT;
				}
				else if ( BST_CHECKED == m_RadDetailedCircuitView.GetCheck())
				{
					usGifID = ( true == fLand ) ? GIF_CIRCDET_LAND : GIF_CIRCDET_PORT;
				}
			}
			else
			{
				if( BST_CHECKED == m_RadCondensedProjectStructure.GetCheck() )
				{
					usGifID = ( true == fLand ) ? GIF_CONDPROJSTRUCT_LAND : GIF_CONDPROJSTRUCT_PORT;
				}
				else if( BST_CHECKED == m_RadCircuitTables.GetCheck() )
				{
					usGifID = ( true == fLand ) ? GIF_CIRCTAB_LAND : GIF_CIRCTAB_PORT;
				}
			}
			
			break;

		case ecbLogData:
			{
				m_CheckedList.PurgeAll();
				double Key = 0;
				FillCheckedItemRankLst( m_Tree.GetRootItem(), Key );
				
				LPARAM lparam;
				CString str;

				if( !m_CheckedList.GetFirst( str, lparam ) )
				{
					return;
				}
				
				sLogData sld = m_aLD.GetAt( lparam );
				
				switch( sld.type )
				{
					case eldSummary:
						usGifID = ( true == fLand ) ? GIF_LOGDATA_LAND : GIF_LOGDATA_PORT;
						break;

					case eldChart:
						usGifID = ( true == fLand ) ? GIF_LOGCHART_LAND : GIF_LOGCHART_PORT;
						break;

					case eldData:
						usGifID = ( true == fLand ) ? GIF_LOGDATA_LAND : GIF_LOGDATA_PORT;
						break;
				}
			}
			
			break;
	}
	
	m_Bmp.DeleteObject();
	m_Bmp.LoadImage( usGifID, _T("GIF") );
	m_Picture.SetBitmap( (HBITMAP)m_Bmp );
}

bool CDlgPrint::PrintPreview()
{
	bool bReturn = false;
	CFrameWnd *pFrame = dynamic_cast<CFrameWnd *>( AfxGetThread()->m_pMainWnd );

	if( NULL != pFrame )
	{
		CFormViewEx *pCurrentView = dynamic_cast<CFormViewEx *>( pFrame->GetActiveView() );

		if( NULL == pCurrentView ) // To avoid non CFormViewEx view (e.g.: Log chart)
		{
			CMainFrame *pMainFrame = static_cast<CMainFrame *>( AfxGetMainWnd() );
			pMainFrame->ActiveFormView( CMainFrame::eRVGeneral );
			pCurrentView = dynamic_cast<CFormViewEx *>( pFrame->GetActiveView() );

			if( NULL != pRViewGen )
			{
				pRViewGen->SetRedraw();
			}
		}

		if( NULL != pCurrentView )
		{
			HWND hwnd = pFrame->GetSafeHwnd();
			// HYS-1090: remove title bar  
			LONG lStyle = GetWindowLong( hwnd, GWL_STYLE );
			lStyle &= ~( WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU );
			SetWindowLong( hwnd, GWL_STYLE, lStyle );
			pCurrentView->SetNotificationHandler( this );
			pCurrentView->OnFilePrintPreview();
			bReturn = true;
		}
	}

	return bReturn;
}