#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "DialogExt.h"
#include "HydroMod.h"
#include "Hydronic.h"
#include "DlgConfSel.h"
#include "RViewSSelSS.h"
#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelTrv.h"
#include "DlgLeftTabSelManager.h"
#include "RViewDescription.h"
#include "MultiSpreadInDialog.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPage6WayValve.h"
#include "SelProdPageAirVentSep.h"
#include "SelProdPageArtList.h"
#include "SelProdPageBv.h"
#include "SelProdPageCtrl.h"
#include "SelProdPageDiversityFactor.h"
#include "SelProdPageDpC.h"
#include "SelProdPageDpCBCV.h"
#include "SelProdPageSv.h"
#include "SelProdPagePDef.h"
#include "SelProdPageHub.h"
#include "SelProdPagePipeList.h"
#include "SelProdPagePressMaint.h"
#include "SelProdPageTrv.h"
#include "SelProdPageTCFloorHeatingControl.h"
#include "SelProdPageTCTapWaterControl.h"
#include "SelProdPageSafetyValve.h"
#include "SelProdPageSmartControlValve.h"
#include "SelProdPageSmartDpC.h"
#include "SelProdDockPane.h"
#include "Excel_Workbook.h"
#include "DlgTender.h"
#include "SelProdPageTender.h"
#include "HMPipes.h"


IMPLEMENT_DYNAMIC( CSelProdDockPane, CDockablePane )

CSelProdDockPane::CSelProdDockPane()
{
	m_bInitialized = false;
	m_pTADS = NULL;
	m_eTabIndex = CDB_PageSetup::enCheck::ARTLIST;
	m_arHMmap.SetSize( CDB_PageSetup::enCheck::LASTFIELD );
}

CSelProdDockPane::~CSelProdDockPane()
{
	_Reset();
}

void CSelProdDockPane::OnApplicationLook()
{
	COLORREF clrDark;
	COLORREF clrBlack;
	COLORREF clrHighlight;
	COLORREF clrFace;
	COLORREF clrDarkShadow;
	COLORREF clrLight;
	CBrush *pbrFace = NULL;
	CBrush *pbrBlack = NULL;

	// Interface color retrieve.
	CMFCVisualManager::GetInstance()->GetTabFrameColors( &m_wndTabs, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack );

	for( int i = 0; i < m_wndTabs.GetTabsNum(); i++ )
	{
		// CDialogEx own his method to fix background color.
		static_cast<CDialogExt *>( m_wndTabs.GetTabWnd( i ) )->SetBackgroundColor( clrDarkShadow );
		static_cast<CDialogExt *>( m_wndTabs.GetTabWnd( i ) )->OnApplicationLook( clrDarkShadow );
	}
}

void CSelProdDockPane::SetActiveTab( CDB_PageSetup::enCheck eTabIndex )
{
	if( 0 == m_mapPageList.count( eTabIndex ) || NULL == m_mapPageList[eTabIndex] || NULL == m_wndTabs.GetSafeHwnd() )
	{
		return;
	}

	int iTabToSet = m_wndTabs.GetTabFromHwnd( m_mapPageList[eTabIndex]->GetSafeHwnd() );

	if( -1 != iTabToSet )
	{
		m_wndTabs.SetActiveTab( iTabToSet );
		m_eTabIndex = eTabIndex;
	}
}

void CSelProdDockPane::RedrawAll( bool fResetOrder, bool fPrint, bool fExport, bool fComeFromProductSelTab )
{
	if( NULL == this->GetSafeHwnd() )
	{
		return;
	}

	LockWindowUpdate();

	// By default we show the previous tab that was shown.
	CDB_PageSetup::enCheck ePreviousTab = m_eTabIndex;

	// If we came from the individual, batch or wizard selection tab, we set the corresponding page.
	if( true == fComeFromProductSelTab )
	{
		ePreviousTab = _GetPageLinkedToLeftTab();
	}

	std::map<CDB_PageSetup::enCheck, LPARAM> mapBackupSelPos;

	if( false == fPrint || false == fExport )
	{
		for( int iLoopPage = CDB_PageSetup::enCheck::FIRSTFIELD; iLoopPage < CDB_PageSetup::enCheck::LASTFIELD; iLoopPage++ )
		{
			CDB_PageSetup::enCheck eLoopPage = ( CDB_PageSetup::enCheck )iLoopPage;

			if( 0 != m_mapPageList.count( eLoopPage ) && NULL != m_mapPageList[eLoopPage] && -1 != m_wndTabs.GetTabFromHwnd( m_mapPageList[eLoopPage]->GetSafeHwnd() ) )
			{
				mapBackupSelPos[eLoopPage] = m_mapPageList[eLoopPage]->BackupSelectedProduct();
			}
		}
	}

	_Reset();
	_Init();

	CDB_PageSetup *pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != pTADSPageSetup );

	CRect rectClient;
	m_wndTabs.GetWndArea( rectClient );
	CDB_PageSetup::enCheck ePageToHide = CDB_PageSetup::enCheck::LASTFIELD;
	CDB_PageSetup::enCheck eFirstVisibleTab = CDB_PageSetup::enCheck::LASTFIELD;

	for( int iLoopPage = CDB_PageSetup::enCheck::FIRSTFIELD; iLoopPage < CDB_PageSetup::enCheck::LASTFIELD; iLoopPage++ )
	{
		CDB_PageSetup::enCheck eLoopPage = ( CDB_PageSetup::enCheck )iLoopPage;

		// We display at least the article list even if there is nothing in it.
		if( CDB_PageSetup::enCheck::ARTLIST == eLoopPage || true == m_mapPageList[eLoopPage]->PreInit( m_arHMmap[eLoopPage] ) )
		{
			// Create this page.
			if( FALSE == m_mapPageList[eLoopPage]->Create( CMultiSpreadInDialog::IDD, &m_wndTabs ) )
			{
				continue;
			}

			m_mapPageList[eLoopPage]->ShowWindow( SW_SHOW );
			m_mapPageList[eLoopPage]->SetWindowPos( NULL, -1, -1, rectClient.Width(), rectClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );

			if( false == m_mapPageList[eLoopPage]->Init( fResetOrder, fPrint ) )
			{
				m_mapPageList[eLoopPage]->DestroyWindow();
				continue;
			}

			CString str = TASApp.LoadLocalizedString( m_mapPageList[eLoopPage]->GetTabTitleID() );
			m_wndTabs.AddTab( m_mapPageList[eLoopPage], str, 1 );

			if( CDB_PageSetup::enCheck::LASTFIELD == eFirstVisibleTab )
			{
				eFirstVisibleTab = eLoopPage;
			}
		}
		else if( eLoopPage == ePreviousTab )
		{
			ePageToHide = eLoopPage;
		}
	}

	if( false == fPrint && false == fExport )
	{
		// Restore the previous row position for each spread sheet.
		for( int iLoopPage = CDB_PageSetup::enCheck::FIRSTFIELD; iLoopPage < CDB_PageSetup::enCheck::LASTFIELD; iLoopPage++ )
		{
			CDB_PageSetup::enCheck eLoopPage = ( CDB_PageSetup::enCheck )iLoopPage;

			// If page is not created or if page is not visible...
			if( 0 == m_mapPageList.count( eLoopPage ) || NULL == m_mapPageList[eLoopPage] || -1 == m_wndTabs.GetTabFromHwnd( m_mapPageList[eLoopPage]->GetSafeHwnd() ) )
			{
				continue;
			}

			// Restore backup if exist...
			bool fRestored = false;

			if( 0 != mapBackupSelPos.count( eLoopPage ) && NULL != mapBackupSelPos[eLoopPage] )
			{
				fRestored = m_mapPageList[eLoopPage]->RestoreSelectedProduct( mapBackupSelPos[eLoopPage] );
			}

			if( false == fRestored )
			{
				m_mapPageList[eLoopPage]->SelectFirstProduct();
			}
		}

		// Clean map of backups.
		for( std::map<CDB_PageSetup::enCheck, LPARAM>::iterator iter = mapBackupSelPos.begin(); iter != mapBackupSelPos.end();
			 iter++ )
		{
			CSelProdPageBase::FreeSelectedProduct( iter->second );
		}

		TASApp.GetpTADS()->RefreshResults( false );
	}

	// Reset the tab on the first position.
	m_wndTabs.SetActiveTab( 0 );

	// Now we can set the good tab.
	if( false == fPrint || false == fExport )
	{
		// Find the good tab to show.
		CDB_PageSetup::enCheck eTabToShow;

		if( ePreviousTab < CDB_PageSetup::enCheck::FIRSTFIELD || ePreviousTab >= CDB_PageSetup::enCheck::LASTFIELD )
		{
			// Previous tab was not correct.
			// Set on the first visible tab if exist. Otherwise article list tab is always shown.
			eTabToShow = ( CDB_PageSetup::enCheck::LASTFIELD != eFirstVisibleTab ) ? eFirstVisibleTab : CDB_PageSetup::enCheck::ARTLIST;
		}
		else
		{
			// If user was previously on a tab that is no more exist...
			if( CDB_PageSetup::enCheck::LASTFIELD != ePageToHide )
			{
				if( CDB_PageSetup::enCheck::LASTFIELD != eFirstVisibleTab )
				{
					// Set on the first visible sheet.
					eTabToShow = eFirstVisibleTab;
				}
				else
				{
					// By default article list page is always shown.
					eTabToShow = CDB_PageSetup::enCheck::ARTLIST;
				}
			}
			else
			{
				eTabToShow = ePreviousTab;
			}
		}

		SetActiveTab( eTabToShow );
	}

	UnlockWindowUpdate();
}

CSelProdPageBase *CSelProdDockPane::GetpPage( CDB_PageSetup::enCheck ePage )
{
	if( 0 == m_mapPageList.count( ePage ) )
	{
		return NULL;
	}

	return m_mapPageList[ePage];
}

void CSelProdDockPane::SynchronizePageWithLeftTab( void )
{
	CDB_PageSetup::enCheck ePage = _GetPageLinkedToLeftTab();
	SetActiveTab( ePage );
}

bool CSelProdDockPane::GoToPage( CDB_PageSetup::enCheck ePage )
{
	if( 0 == m_mapPageList.count( ePage ) || -1 == m_wndTabs.GetTabFromHwnd( m_mapPageList[ePage]->GetSafeHwnd() ) )
	{
		return false;
	}

	bool fReturn = false;
	int iActiveTab = m_wndTabs.GetActiveTab();

	if( ePage != ( CDB_PageSetup::enCheck )iActiveTab )
	{
		SetActiveTab( ePage );
		fReturn = true;
	}

	return fReturn;
}

BEGIN_MESSAGE_MAP( CSelProdDockPane, CDockablePane )
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()

	ON_COMMAND( ID_SSELPFLT_COPY, OnEditCopy )
	ON_COMMAND( ID_SSELPFLT_CUT, OnEditCut )
	ON_COMMAND( ID_SSELPFLT_PASTE, OnEditPaste )

	ON_COMMAND( ID_SSELPFLT_DELETE, OnSSelDelete )
	ON_COMMAND( ID_SSELPFLT_EDIT, OnSSelEdit )
	ON_COMMAND( ID_SSELPFLT_QUICKEDIT, OnSSelQuickEdit )
	ON_COMMAND( ID_SSELPFLT_GETTENDER, OnSSelGetTender )

	ON_COMMAND( ID_SSELPFLT_GETFULLINFO, OnSSelGetFullInfo )

	ON_UPDATE_COMMAND_UI( ID_EDIT_COPY, OnUpdateEditCopy )
	ON_UPDATE_COMMAND_UI( ID_EDIT_CUT, OnUpdateEditCut )
	ON_UPDATE_COMMAND_UI( ID_EDIT_PASTE, OnUpdateEditPaste )
	ON_UPDATE_COMMAND_UI( ID_SSELPFLT_GETTENDER, OnUpdateGetTender )
	ON_UPDATE_COMMAND_UI_RANGE( ID_SSELPFLT_CUT, ID_SSELPFLT_QUICKEDIT, OnUpdateMenuText )

	ON_REGISTERED_MESSAGE( AFX_WM_CHANGE_ACTIVE_TAB, OnChangeActiveTab )

	ON_MESSAGE( WM_USER_USERSELECTIONCHANGE, OnUserSelectionChange )
	ON_MESSAGE( WM_USER_SELPRODEDITPRODUCT, OnSelProdEditProduct )
END_MESSAGE_MAP()

void CSelProdDockPane::OnDestroy()
{
	_Reset();
	CDockablePane::OnDestroy();
}

int CSelProdDockPane::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( -1 == CDockablePane::OnCreate( lpCreateStruct ) )
	{
		return -1;
	}

	// Create tabs window:
	if( FALSE == m_wndTabs.Create( CMFCTabCtrl::STYLE_3D, CRect( 0, 0, 0, 0 ), this, 1 ) )
	{
		TRACE0( "Failed to create output tab window\n" );
		return -1;      // fail to create
	}

	m_wndTabs.SetFlatFrame( FALSE, FALSE );
	m_wndTabs.AutoDestroyWindow( FALSE );

	OnApplicationLook();

	return 0;
}

void CSelProdDockPane::OnContextMenu( CWnd *pWnd, CPoint point )
{
	if( NULL == m_wndTabs.GetSafeHwnd() || CDB_PageSetup::enCheck::LASTFIELD == m_eTabIndex || 0 == m_mapPageList.count( m_eTabIndex )
			|| NULL == m_mapPageList[m_eTabIndex] )
	{
		return;
	}

	// Enable all menu resource ID by default.
	m_rCtxtMenuData.Clear();
	pMainFrame->EnableMenuResID( NULL );

	bool bShowPopupMenu = false;
	CMenu menu, popup, popup2;
	menu.LoadMenu( IDR_FLTMENU_SSELP );
	CMenu *pContextMenu = menu.GetSubMenu( 0 );

	// Load correct strings.
	CString str;
	str = pMainFrame->GetSplitStringFromResID( ID_SSELPFLT_EDIT );
	pContextMenu->ModifyMenu( ID_SSELPFLT_EDIT, MF_BYCOMMAND, ID_SSELPFLT_EDIT, str );
	str = pMainFrame->GetSplitStringFromResID( ID_SSELPFLT_QUICKEDIT );
	pContextMenu->ModifyMenu( ID_SSELPFLT_QUICKEDIT, MF_BYCOMMAND, ID_SSELPFLT_QUICKEDIT, str );
	str = pMainFrame->GetSplitStringFromResID( ID_SSELPFLT_GETTENDER );
	pContextMenu->ModifyMenu( ID_SSELPFLT_GETTENDER, MF_BYCOMMAND, ID_SSELPFLT_GETTENDER, str );
	str = pMainFrame->GetSplitStringFromResID( ID_SSELPFLT_DELETE );
	pContextMenu->ModifyMenu( ID_SSELPFLT_DELETE, MF_BYCOMMAND, ID_SSELPFLT_DELETE, str );
	str = pMainFrame->GetSplitStringFromResID( ID_SSELPFLT_CUT );
	pContextMenu->ModifyMenu( ID_SSELPFLT_CUT, MF_BYCOMMAND, ID_SSELPFLT_CUT, str );
	str = pMainFrame->GetSplitStringFromResID( ID_SSELPFLT_COPY );
	pContextMenu->ModifyMenu( ID_SSELPFLT_COPY, MF_BYCOMMAND, ID_SSELPFLT_COPY, str );
	str = pMainFrame->GetSplitStringFromResID( ID_SSELPFLT_PASTE );
	pContextMenu->ModifyMenu( ID_SSELPFLT_PASTE, MF_BYCOMMAND, ID_SSELPFLT_PASTE, str );
	str = pMainFrame->GetSplitStringFromResID( ID_SSELPFLT_GETFULLINFO );
	pContextMenu->ModifyMenu( ID_SSELPFLT_GETFULLINFO, MF_BYCOMMAND, ID_SSELPFLT_GETFULLINFO, str );

	pMainFrame->DisableMenuResID( ID_SSELPFLT_EDIT );
	pMainFrame->DisableMenuResID( ID_SSELPFLT_QUICKEDIT );
	pMainFrame->DisableMenuResID( ID_SSELPFLT_GETTENDER );
	pMainFrame->DisableMenuResID( ID_SSELPFLT_DELETE );
	pMainFrame->DisableMenuResID( ID_SSELPFLT_CUT );
	pMainFrame->DisableMenuResID( ID_SSELPFLT_COPY );
	pMainFrame->DisableMenuResID( ID_SSELPFLT_PASTE );
	pMainFrame->DisableMenuResID( ID_SSELPFLT_GETFULLINFO );

	// Get the clipBoard table.
	CTable *pclClipBoardTable = m_pTADS->GetpClipboardTable();
	ASSERT( NULL != pclClipBoardTable );

	CData *pclSSelObject = NULL;
	CData *pclObject = NULL;
	bool bSSelUnderMouse = m_mapPageList[m_eTabIndex]->GetSSelUnderMouse( point, &pclSSelObject, &pclObject );

	// Case where the user click on a product.
	if( true == bSSelUnderMouse )
	{
		// Simulate a click to eventually move selection under the right click!
		// Remark: only if there is no selection or only one. If more that one selection, let these rows in selected mode to allow
		//         user to do 'delete' operation.
		int iNbrSSelSelected = m_mapPageList[m_eTabIndex]->GetSSelSelectedNumbers();

		if( 0 == iNbrSSelSelected || 1 == iNbrSSelSelected )
		{
			m_mapPageList[m_eTabIndex]->ClickOnSSel( ( LPARAM )pclSSelObject );
		}

		if( 0 != iNbrSSelSelected )
		{
			bool bEnableCopyItem = true;
			bool bEnableCutItem = true;
			bool bEnableDeleteItem = true;

			// Fill 'm_rCtxtMenuData' for the selection.
			bool bFromDirSel = false;

			if( NULL != dynamic_cast<CDS_SSel *>( pclSSelObject ) )
			{
				CDS_SSel *pclSSelectedValve = (CDS_SSel *)pclSSelObject;
				m_rCtxtMenuData.m_pclSSelectedValve = pclSSelectedValve;
				bFromDirSel = pclSSelectedValve->IsFromDirSel();
			}
			else if( NULL != dynamic_cast<CDS_SSelPMaint *>( pclSSelObject ) )
			{
				// Remark: 'CDS_SSelPMain' is inherited from a 'CData'.
				CDS_SSelPMaint *pclSSelectedPM = (CDS_SSelPMaint *)pclSSelObject;
				m_rCtxtMenuData.m_pclSSelectedPM = pclSSelectedPM;
				bFromDirSel = pclSSelectedPM->IsFromDirSel();
			}
			else if( NULL != dynamic_cast<CDS_Actuator *>( pclSSelObject ) )
			{
				CDS_Actuator *pclSelectedActuator = (CDS_Actuator *)pclSSelObject;
				m_rCtxtMenuData.m_pclSSelectedActuator = pclSelectedActuator;
				bFromDirSel = true;
			}
			else if( NULL != dynamic_cast<CDS_Accessory *>( pclSSelObject ) )
			{
				CDS_Accessory *pclSelectedAccessory = (CDS_Accessory *)pclSSelObject;
				m_rCtxtMenuData.m_pclSSelectedAccessory = pclSelectedAccessory;
				bFromDirSel = true;
			}
			else if( NULL != dynamic_cast<CDS_HmHub *>( pclSSelObject ) )
			{
				CDS_HmHub *pclSelectedHub = (CDS_HmHub *)pclSSelObject;
				m_rCtxtMenuData.m_pclSSelectedHUB = pclSelectedHub;
				bEnableCopyItem = false;
				bEnableCutItem = false;
			}

			// Fill 'm_rCtxtMenuData' for the product under the mouse pointer.
			bool fFullCatExist = false;

			if( NULL != dynamic_cast<CDB_Product *>( pclObject ) )
			{
				CDB_Product *pclProduct = ( CDB_Product * )pclObject;
				m_rCtxtMenuData.m_pclProduct = pclProduct;
				fFullCatExist = pclProduct->VerifyOneCatExist();
			}
			else if( NULL != dynamic_cast<CDB_Actuator *>( pclObject ) )
			{
				CDB_Actuator *pclActuator = (CDB_Actuator *)pclObject;
				m_rCtxtMenuData.m_pclActuator = pclActuator;
				fFullCatExist = pclActuator->VerifyOneCatExist();
			}


			if( 1 == iNbrSSelSelected )
			{
				// Full catalog sheet item.
				if( true == fFullCatExist )
				{
					pMainFrame->EnableMenuResID( ID_SSELPFLT_GETFULLINFO );
				}

				// Quick edit and edit items.
				if( true == TASApp.GetpTADB()->CanEditSelection( pclSSelObject ) )
				{
					pMainFrame->EnableMenuResID( ID_SSELPFLT_QUICKEDIT );

					if( false == bFromDirSel )
					{
						pMainFrame->EnableMenuResID( ID_SSELPFLT_EDIT );
					}
				}

				pMainFrame->EnableMenuResID( ID_SSELPFLT_GETTENDER );
			}

			// Enable 'Copy', 'Cut' and 'Delete' items.
			if( true == bEnableCopyItem )
			{
				pMainFrame->EnableMenuResID( ID_SSELPFLT_COPY );
			}

			if( true == bEnableCutItem )
			{
				pMainFrame->EnableMenuResID( ID_SSELPFLT_CUT );
			}

			if( true == bEnableDeleteItem )
			{
				pMainFrame->EnableMenuResID( ID_SSELPFLT_DELETE );
			}

			bShowPopupMenu = true;
		}
	}

	// Remark: 'Article list' and 'Diversity factor' pages have no table and return NULL. It is effectively impossible to 'Paste' in these pages.
	if( NULL != _GetpTableLinkedToPage( m_eTabIndex ) && 0 != pclClipBoardTable->GetItemCount() )
	{
		bool bEnablePaste = false;

		if( CDB_PageSetup::enCheck::AIRVENTSEPARATOR == m_eTabIndex && NULL != dynamic_cast<CDS_SSelAirVentSeparator *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::PRESSMAINT == m_eTabIndex && NULL != dynamic_cast<CDS_SSelPMaint *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::SAFETYVALVE == m_eTabIndex && NULL != dynamic_cast<CDS_SSelSafetyValve *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::BV == m_eTabIndex && NULL != dynamic_cast<CDS_SSelBv *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::DPC == m_eTabIndex && NULL != dynamic_cast<CDS_SSelDpC *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::DPC == m_eTabIndex && NULL != dynamic_cast<CDS_SSelDpReliefValve *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::DPCBCV == m_eTabIndex && NULL != dynamic_cast<CDS_SSelDpCBCV *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::SV == m_eTabIndex && NULL != dynamic_cast<CDS_SSelSv *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::CV == m_eTabIndex && NULL != dynamic_cast<CDS_SSelCtrl *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::CV == m_eTabIndex && NULL != dynamic_cast<CDS_Actuator *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::SMARTCONTROLVALVE == m_eTabIndex && NULL != dynamic_cast<CDS_SSelSmartControlValve *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::SMARTDPC == m_eTabIndex && NULL != dynamic_cast<CDS_SSelSmartDpC *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::SIXWAYVALVE == m_eTabIndex && NULL != dynamic_cast<CDS_SSel6WayValve *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::TRV == m_eTabIndex && NULL != dynamic_cast<CDS_SSelRadSet *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::TAPWATERCONTROL == m_eTabIndex && NULL != dynamic_cast<CDS_SSelTapWaterControl *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::FLOORHEATINGCONTROL == m_eTabIndex && NULL != dynamic_cast<CDS_SSelFloorHeatingManifold *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::FLOORHEATINGCONTROL == m_eTabIndex && NULL != dynamic_cast<CDS_SSelFloorHeatingValve *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}
		else if( CDB_PageSetup::enCheck::FLOORHEATINGCONTROL == m_eTabIndex && NULL != dynamic_cast<CDS_SSelFloorHeatingController *>( pclClipBoardTable->GetFirst().MP ) )
		{
			bEnablePaste = true;
		}

		if( true == bEnablePaste )
		{
			pMainFrame->EnableMenuResID( ID_SSELPFLT_PASTE );
			bShowPopupMenu = true;
		}
	}

	if (false == TASApp.GetTender().IsTenderNeeded())
	{
		pContextMenu->RemoveMenu(ID_SSELPFLT_GETTENDER, MF_BYCOMMAND);
	}

	// Show the popup menu.
	if( true == bShowPopupMenu )
	{
		TASApp.GetContextMenuManager()->ShowPopupMenu( HMENU( *pContextMenu ), point.x, point.y, this, TRUE );
	}
}

void CSelProdDockPane::OnSize( UINT nType, int cx, int cy )
{
	CDockablePane::OnSize( nType, cx, cy );

	LockWindowUpdate();

	// Tab control should cover the whole client area.
	m_wndTabs.SetWindowPos( NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
	CRect rectClient;
	m_wndTabs.GetWndArea( rectClient );

	for( mapSelProdPageIter iter = m_mapPageList.begin(); iter != m_mapPageList.end(); iter++ )
	{
		if( NULL != iter->second && NULL != iter->second->GetSafeHwnd() )
		{
			iter->second->SetWindowPos( NULL, -1, -1, rectClient.Width(), rectClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
			iter->second->OptimizePageSize( rectClient.Width(), rectClient.Height() );
		}
	}

	UnlockWindowUpdate();
}

void CSelProdDockPane::OnAfterDock( CBasePane *pBar, LPCRECT lpRect, AFX_DOCK_METHOD dockMethod )
{
	CDockablePane::OnAfterDock( pBar, lpRect, dockMethod );
	CRect rectClient;
	m_wndTabs.GetWndArea( rectClient );

	for( mapSelProdPageIter iter = m_mapPageList.begin(); iter != m_mapPageList.end(); iter++ )
	{
		if( NULL != iter->second && NULL != iter->second->GetSafeHwnd() )
		{
			iter->second->SetWindowPos( NULL, -1, -1, rectClient.Width(), rectClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
			iter->second->OptimizePageSize( rectClient.Width(), rectClient.Height() );
		}
	}
}

BOOL CSelProdDockPane::PreTranslateMessage( MSG *pMsg )
{
	// By default message should be dispatched.
	BOOL fReturn = FALSE;

	if( pMsg->message == WM_KEYDOWN )
	{
		if( GetKeyState( VK_LCONTROL ) < 0 && GetKeyState( VK_TAB ) < 0 )
		{
			_ChangeTab( ( GetKeyState( VK_SHIFT ) < 0 ) ? false : true );
			fReturn = TRUE;
		}
	}

	if( FALSE == fReturn )
	{
		fReturn = CDockablePane::PreTranslateMessage( pMsg );
	}

	return fReturn;
}

void CSelProdDockPane::OnEditCopy()
{
	try
	{
		if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
		{
			return;
		}

		BYTE bModified = m_pTADS->IsModified();
		m_pTADS->CleanClipboard();

		CTable *pclClipboardTable = m_pTADS->GetpClipboardTable();
		
		if( NULL == pclClipboardTable )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CLIPBOARD_TABLE' table from the datastruct."), __LINE__, __FILE__ );
		}

		CString strClassName;
		CArray<LPARAM> arlpObj;

		if( 0 == m_mapPageList[m_eTabIndex]->GetAllSSelSelected( strClassName, arlpObj ) )
		{
			return;
		}

		for( int i = 0; i < arlpObj.GetCount(); i++ )
		{
			// Remark: We can have different classes selected at once. For example in the 'Control products' we can have 'CDS_SSel' object and 'CDS_Actuator'.
			//         This is why we can't no more take into account 'strClassName' to create copy but the real class name of the current object.

			LPARAM lpObj = arlpObj.GetAt( i );

			IDPTR IDPtr = _NULL_IDPTR;
			CDS_SSel *pSel = dynamic_cast<CDS_SSel *>( (CData *)lpObj );

			if( NULL != pSel )
			{
				// Case of 'CDS_SSelCV', 'CDS_SSelPICV', 'CDS_SSelBV', 'CDS_SSelDpC', 'CDS_SSelDpCBCV', 'CDS_SSelSv' and 'CDS_SSelRadSet' objects.
				m_pTADS->CreateObject( IDPtr, pSel->GetClassName() );
				pclClipboardTable->Insert( IDPtr );
				pSel->Copy( (CData *)IDPtr.MP );
			}
			else if( NULL != dynamic_cast<CDS_SSelPMaint *>( (CData *)lpObj ) )
			{
				CDS_SSelPMaint *pclSSelPMaint = ( CDS_SSelPMaint * )lpObj;
			
				m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPMaint ) );
				pclClipboardTable->Insert( IDPtr );
				pclSSelPMaint->Copy( ( CDS_SSelPMaint * )IDPtr.MP );
			}
			else if( NULL != dynamic_cast<CDS_Actuator *>( (CData *)lpObj ) )
			{
				CDS_Actuator *pSel = ( CDS_Actuator * )lpObj;
			
				m_pTADS->CreateObject( IDPtr, CLASS( CDS_Actuator ) );
				pclClipboardTable->Insert( IDPtr );
				pSel->Copy( ( CDS_Actuator * )IDPtr.MP );
			}
			else if( NULL != dynamic_cast<CDS_Accessory *>( (CData *)lpObj ) )
			{
				CDS_Accessory *pSel = ( CDS_Accessory * )lpObj;

				m_pTADS->CreateObject( IDPtr, CLASS( CDS_Accessory ) );
				pclClipboardTable->Insert( IDPtr );
				pSel->Copy( ( CDS_Accessory * )IDPtr.MP );
			}
		}

		m_pTADS->Modified( bModified );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CSelProdDockPane::OnEditCopy'."), __LINE__, __FILE__ );
		throw;
	}
}

void CSelProdDockPane::OnEditCut()
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	CString strClassName;
	LPARAM lpObj = ( LPARAM )0;

	if( false == m_mapPageList[m_eTabIndex]->GetFirstSSelSelected( strClassName, lpObj ) )
	{
		return;
	}

	OnEditCopy();							// Put a copy of selected item into the clipboard
	_DeleteTADSObj();
	RedrawAll();
}

void CSelProdDockPane::OnEditPaste()
{
	try
	{
		if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
		{
			return;
		}

		CTable *pCBTab = m_pTADS->GetpClipboardTable();

		if( NULL == pCBTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CLIPBOARD_TABLE' table from the datastruct."), __LINE__, __FILE__ );
		}

		// Retrieve the first selected element in the sheet.
		CString strFirstElementClassName;
		LPARAM lpFirstObj;
		CTable *pTab = _GetpTableLinkedToPage( m_eTabIndex );

		if( NULL == pTab )
		{
			return;
		}

		BeginWaitCursor();
		bool bOneElementSelected = m_mapPageList[m_eTabIndex]->GetFirstSSelSelected( strFirstElementClassName, lpFirstObj );

		for( IDPTR IDPtrData = pCBTab->GetFirst(); _T('\0') != *IDPtrData.ID; IDPtrData = pCBTab->GetNext() )
		{
			IDPTR IDPtr;

			CData *pData = (CData *)( IDPtrData.MP );

			if( NULL == pData )
			{
				continue;
			}

			CString strClassName = pData->GetClassName();
			CDS_SSel *pSelObjIntoClipBoard = dynamic_cast<CDS_SSel *>( pData );

			if( NULL != pSelObjIntoClipBoard )
			{
				m_pTADS->CreateObject( IDPtr, strClassName );
				pTab->Insert( IDPtr );
				
				// Copy values from clipboard.
				pSelObjIntoClipBoard->Copy( (CDS_SSel *) IDPtr.MP );

				// Update Row Index.
				// If no 'fOneElementSelected' insert the new element at the end of the table.
				// Otherwise insert the new product before the selected product.
				if( true == bOneElementSelected )
				{
					// Update Row Index
					// If Row == 0 insert the new element at the end  table
					// otherwise insert the new object before the selected Bv
					CDS_SSel *pSelObj = dynamic_cast<CDS_SSel *>( (CData *)lpFirstObj );

					if( NULL != pSelObj )
					{
						int iRow = pSelObj->GetpSelectedInfos()->GetRowIndex();

						for( IDPTR idptr = pTab->GetFirst(); _T('\0') != *idptr.ID; idptr = pTab->GetNext() )
						{
							CDS_SSel *pObj = dynamic_cast<CDS_SSel *>( idptr.MP );

							if( NULL != pObj )
							{
								// Create a hole in the row indexation to insert new object.
								if( pObj->GetpSelectedInfos()->GetRowIndex() >= iRow )
								{
									pObj->GetpSelectedInfos()->SetRowIndex( pObj->GetpSelectedInfos()->GetRowIndex() + 1 );
								}
							}
						}

						// Set the new index .
						( ( CDS_SSel * )( IDPtr.MP ) )->GetpSelectedInfos()->SetRowIndex( iRow );
					}
				}
				else
				{
					( ( CDS_SSel * )( IDPtr.MP ) )->GetpSelectedInfos()->SetRowIndex( pTab->GetItemCount() );
				}
			}
			else if( strClassName == CLASS( CDS_SSelPMaint ) )
			{
				CDS_SSelPMaint *pSelObjIntoClipBoard = dynamic_cast<CDS_SSelPMaint *>( pData );
			
				m_pTADS->CreateObject( IDPtr, strClassName );
				pTab->Insert( IDPtr );

				// Copy values from clipboard.
				pSelObjIntoClipBoard->Copy( ( CDS_SSelPMaint * )IDPtr.MP );

				// Update row index.
				// If no 'fOneElementSelected' insert the new element at the end of the table.
				// Otherwise insert the new product before the selected product.
				if( true == bOneElementSelected )
				{
					// Update row index.
					// If Row == 0 insert the new element at the end  table.
					// Otherwise insert the new object before the selected Bv.
					CDS_SSelPMaint *pSelObj = dynamic_cast<CDS_SSelPMaint *>( (CData *)lpFirstObj );

					if( NULL != pSelObj )
					{
						int iRow = pSelObj->GetpSelectedInfos()->GetRowIndex();

						for( IDPTR idptr = pTab->GetFirst(); _T('\0') != *idptr.ID; idptr = pTab->GetNext() )
						{
							CDS_SSelPMaint *pObj = dynamic_cast<CDS_SSelPMaint *>( idptr.MP );

							if( NULL != pObj )
							{
								// Create a hole in the row indexation to insert new object.
								if( pObj->GetpSelectedInfos()->GetRowIndex() >= iRow )
								{
									pObj->GetpSelectedInfos()->SetRowIndex( pObj->GetpSelectedInfos()->GetRowIndex() + 1 );
								}
							}
						}

						// Set the new index .
						( ( CDS_SSelPMaint * )( IDPtr.MP ) )->GetpSelectedInfos()->SetRowIndex( iRow );
					}
				}
				else
				{
					( ( CDS_SSelPMaint * )( IDPtr.MP ) )->GetpSelectedInfos()->SetRowIndex( pTab->GetItemCount() );
				}
			}
			else if( strClassName == CLASS( CDS_Actuator ) )
			{
				m_pTADS->CreateObject( IDPtr, CLASS( CDS_Actuator ) );
				pTab->Insert( IDPtr );

				// Copy values from clipboard.
				CDS_Actuator *pSel = dynamic_cast<CDS_Actuator *>( IDPtr.MP );
				( (CDS_Actuator *)pData )->Copy( pSel );
			}
			else if( strClassName == CLASS( CDS_Accessory ) )				// Cv Exist Paste it
			{
				m_pTADS->CreateObject( IDPtr, CLASS( CDS_Accessory ) );
				pTab->Insert( IDPtr );

				// Copy values from clipboard.
				CDS_Accessory *pSel = dynamic_cast<CDS_Accessory *>( IDPtr.MP );
				( (CDS_Accessory *)pData )->Copy( pSel );
			}
			else
			{
				HYSELECT_THROW( _T("Internal error: '%s' is not managed."), __LINE__, __FILE__, strClassName );
			}
		}

		RedrawAll();
		EndWaitCursor();
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CSelProdDockPane::OnEditPaste'."), __LINE__, __FILE__ );
		throw;
	}
}

void CSelProdDockPane::OnSSelDelete()
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	_DeleteTADSObj( true );
	RedrawAll();
}

void CSelProdDockPane::OnSSelEdit()
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	CDlgConfSel Dlg;
	UINT uiMessage;
	bool bSendMessage = false;
	bool bCallDoModal = true;

	LPARAM lpObj = NULL;

	if( NULL != m_rCtxtMenuData.m_pclSSelectedHUB )
	{
		lpObj = ( LPARAM )m_rCtxtMenuData.m_pclSSelectedHUB;
		uiMessage = WM_USER_MODIFYSELECTEDHUB;
		bSendMessage = true;
		bCallDoModal = false;
	}
	else if( NULL != m_rCtxtMenuData.m_pclSSelectedValve )
	{
		if( true == m_rCtxtMenuData.m_pclSSelectedValve->IsFromDirSel() )
		{
			Dlg.Display( m_rCtxtMenuData.m_pclSSelectedValve );
		}
		else
		{
			// Edition is done with DlgIndSelXXX and RViewSSelXXX.
			bCallDoModal = false;
			bSendMessage = true;
			lpObj = ( LPARAM )m_rCtxtMenuData.m_pclSSelectedValve;

			// Try to dynamic_cast on all the different CDS_SSelXXX to determine what is the class.
			if( NULL != dynamic_cast<CDS_SSelPICv *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDPICV;
			}
			else if( NULL != dynamic_cast<CDS_SSelBCv *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDBCV;
			}
			else if( NULL != dynamic_cast<CDS_SSelCv *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDCV;
			}
			else if( NULL != dynamic_cast<CDS_SSelDpC *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDDPC;
			}
			else if( NULL != dynamic_cast<CDS_SSelDpCBCV *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDDPCBCV;
			}
			else if( NULL != dynamic_cast<CDS_SSelSv *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDSV;
			}
			else if( NULL != dynamic_cast<CDS_SSelBv *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDBV;
			}
			else if( NULL != dynamic_cast<CDS_SSelRadSet *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDTRV;
			}
			else if( NULL != dynamic_cast<CDS_SSelAirVentSeparator *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDSEPARATOR;
			}
			else if( NULL != dynamic_cast<CDS_SSelSafetyValve *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDSAFETYVALVE;
			}
			else if( NULL != dynamic_cast<CDS_SSel6WayValve *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTED6WAYVALVE;
			}
			else if( NULL != dynamic_cast<CDS_SSelSmartControlValve *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDSMARTCONTROLVALVE;
			}
			else if( NULL != dynamic_cast<CDS_SSelSmartDpC *>( m_rCtxtMenuData.m_pclSSelectedValve ) )
			{
				uiMessage = WM_USER_MODIFYSELECTEDSMARTDPC;
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}
	else if( NULL != m_rCtxtMenuData.m_pclSSelectedPM )
	{
		lpObj = ( LPARAM )m_rCtxtMenuData.m_pclSSelectedPM;

		if( true == m_rCtxtMenuData.m_pclSSelectedPM->IsFromDirSel() )
		{
			Dlg.Display( m_rCtxtMenuData.m_pclSSelectedPM );
		}
		else if( ProductSelectionMode_Individual == m_rCtxtMenuData.m_pclSSelectedPM->GetSelectionMode() )
		{
			// Edition is done with DlgIndSelXXX and RViewSSelXXX.
			bCallDoModal = false;
			bSendMessage = true;
			uiMessage = WM_USER_MODIFYSELECTEDPM;
		}
		else if( ProductSelectionMode_Wizard == m_rCtxtMenuData.m_pclSSelectedPM->GetSelectionMode() )
		{
			// Edition is done with DlgWizardSelPM and RViewWizardSelPM.
			bCallDoModal = false;
			bSendMessage = true;
			uiMessage = WM_USER_MODIFYSELECTEDWIZPM;
		}
	}
	else if( NULL != m_rCtxtMenuData.m_pclSSelectedActuator )
	{
		lpObj = ( LPARAM )m_rCtxtMenuData.m_pclSSelectedActuator;
		Dlg.Display( m_rCtxtMenuData.m_pclSSelectedActuator );
	}
	else if( NULL != m_rCtxtMenuData.m_pclSSelectedAccessory )
	{
		lpObj = ( LPARAM )m_rCtxtMenuData.m_pclSSelectedAccessory;
		Dlg.Display( m_rCtxtMenuData.m_pclSSelectedAccessory );
	}
	else
	{
		ASSERT( 0 );
		bCallDoModal = false;
	}

	// If we can't show confirm selection dialog...
	if( true == bCallDoModal )
	{
		if( IDOK == Dlg.DoModal() )
		{
			RedrawAll();
		}
	}
	else if( true == bSendMessage )
	{
		pMainFrame->PostMessageToDescendants( uiMessage, 0, lpObj );
	}
}

void CSelProdDockPane::OnSSelQuickEdit()
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	CDlgConfSel Dlg;
	bool bCallDoModal = true;

	if( NULL != m_rCtxtMenuData.m_pclSSelectedValve )
	{
		Dlg.Display( m_rCtxtMenuData.m_pclSSelectedValve );
	}
	else if( NULL != m_rCtxtMenuData.m_pclSSelectedPM )
	{
		Dlg.Display( m_rCtxtMenuData.m_pclSSelectedPM );
	}
	else if( NULL != m_rCtxtMenuData.m_pclSSelectedActuator )
	{
		Dlg.Display( m_rCtxtMenuData.m_pclSSelectedActuator );
	}
	else if( NULL != m_rCtxtMenuData.m_pclSSelectedAccessory )
	{
		Dlg.Display( m_rCtxtMenuData.m_pclSSelectedAccessory );
	}
	else
	{
		ASSERT( 0 );
		bCallDoModal = false;
	}

	// If we can't show confirm selection dialog...
	if( true == bCallDoModal )
	{
		if( IDOK == Dlg.DoModal() )
		{
			RedrawAll();
		}
	}
}

void CSelProdDockPane::OnSSelGetTender()
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	CTenderDlg Dlg;
	CDB_Product *product = m_rCtxtMenuData.m_pclProduct;

	if( NULL != product )
	{
		const TCHAR *artNum = product->GetBodyArtNum();

		_string tenderText;

		TASApp.GetTender().GetTenderTxt( _string( artNum ), tenderText );
		Dlg.SetTender(product->GetName(), _string( artNum ) , tenderText );

		INT_PTR result = Dlg.DoModal();

		if( result == -1 )
		{
			DWORD err = GetLastError();
		}
	}
}

void CSelProdDockPane::OnSSelGetFullInfo()
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	_DisplayRequestedDocSheet( CatalogType::CT_FullCat );
}

void CSelProdDockPane::OnUpdateEditCopy( CCmdUI *pCmdUI )
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	CString strClassName;
	CArray<LPARAM> arlpObj;

	if( 0 != m_mapPageList[m_eTabIndex]->GetAllSSelSelected( strClassName, arlpObj ) )
	{
		if( 0 != arlpObj.GetCount() )
		{
			pCmdUI->Enable( TRUE );
		}
		else
		{
			pCmdUI->Enable( FALSE );
		}
	}
}

void CSelProdDockPane::OnUpdateEditCut( CCmdUI *pCmdUI )
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	CString strClassName;
	CArray<LPARAM> arlpObj;

	if( 0 != m_mapPageList[m_eTabIndex]->GetAllSSelSelected( strClassName, arlpObj ) )
	{
		if( 0 != arlpObj.GetCount() )
		{
			pCmdUI->Enable( TRUE );
		}
		else
		{
			pCmdUI->Enable( FALSE );
		}
	}
}

void CSelProdDockPane::OnUpdateEditPaste( CCmdUI *pCmdUI )
{
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	CTable *pCurTab = _GetpTableLinkedToPage( m_eTabIndex );
	CTable *pCBTab = m_pTADS->GetpClipboardTable();
	ASSERT( NULL != pCBTab );

	if( NULL == pCurTab || NULL == pCBTab )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	bool bSthToPaste = false;

	// Retrieve first element in clipboard.
	IDPTR IDPtrData = pCBTab->GetFirst();
	CString strClassName = IDPtrData.MP->GetClassName();

	if( IDPtrData.GetAs<CDS_SSelBv>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("REGVALV_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelCv>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("CTRLVALVE_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelDpC>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("DPCONTR_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelDpReliefValve>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("DPCONTR_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelDpCBCV>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("DPCBALCTRLVALV_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelSv>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("SHUTOFF_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelRadSet>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("RADSET_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelAirVentSeparator>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("AIRVENTSEP_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelPMaint>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("PRESSMAINT_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelSafetyValve>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("SAFETYVALVE_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSel6WayValve>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("6WAYCTRLVALV_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelTapWaterControl>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("TAPWATERCTRL_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelSmartControlValve>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("SMARTCONTROLVALVE_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelFloorHeatingManifold>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("FLOORHCTRL_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelFloorHeatingValve>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("FLOORHCTRL_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelFloorHeatingController>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("FLOORHCTRL_TAB") ) )
	{
		bSthToPaste = true;
	}
	else if( IDPtrData.GetAs<CDS_SSelSmartDpC>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("SMARTDPC_TAB") ) )
	{
		bSthToPaste = true;
	}
	// HYS-2007
	else if( IDPtrData.GetAs<CDS_SSelDpSensor>() && 0 == _tcscmp( pCurTab->GetIDPtr().ID, _T("DPCONTR_TAB") ) )
	{
		bSthToPaste = true;
	}

	if( true == bSthToPaste )
	{
		pCmdUI->Enable( TRUE );
	}
}

void CSelProdDockPane::OnUpdateGetTender(CCmdUI *pCmdUI)
 {
	if( false == m_bInitialized || 0 == m_mapPageList.count( m_eTabIndex ) )
	{
		return;
	}

	CDB_Product *product = m_rCtxtMenuData.m_pclProduct;

	if( product )
	{
		const TCHAR *artNum = product->GetBodyArtNum();

		int tenderID = TASApp.GetTender().GetTenderID( _string( artNum ) );

		if( tenderID >= 0 )
		{
			pCmdUI->Enable( TRUE );
		}
		else
		{
			pCmdUI->Enable( FALSE );
		}
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CSelProdDockPane::OnUpdateMenuText( CCmdUI *pCmdUI )
{
	// Update the status bar
	pMainFrame->UpdateMenuToolTip( pCmdUI );
}

LRESULT CSelProdDockPane::OnChangeActiveTab( WPARAM wp, LPARAM lp )
{
	if( wp >= 0 && wp < ( WPARAM )m_wndTabs.GetTabsNum() )
	{
		CSelProdPageBase *pclPage = dynamic_cast<CSelProdPageBase *>( m_wndTabs.GetTabWnd( ( int )wp ) );
		ASSERT( NULL != pclPage );
		m_eTabIndex = pclPage->GetPageID();
	}

	return 0;
}

LRESULT CSelProdDockPane::OnUserSelectionChange( WPARAM wParam, LPARAM lParam )
{
	if( TRUE == IsWindowVisible() )
	{
		RedrawAll();
	}

	return 0;
}

LRESULT CSelProdDockPane::OnSelProdEditProduct( WPARAM wParam, LPARAM lParam )
{
	m_rCtxtMenuData.Clear();

	if( NULL != dynamic_cast<CDS_SSel *>( (CData *)wParam ) )
	{
		m_rCtxtMenuData.m_pclSSelectedValve = ( CDS_SSel * )wParam;
	}
	else if( NULL != dynamic_cast<CDS_SSelPMaint *>( (CData *)wParam ) )
	{
		m_rCtxtMenuData.m_pclSSelectedPM = ( CDS_SSelPMaint * )wParam;
	}
	else if( NULL != dynamic_cast<CDS_Actuator *>( (CData *)wParam ) )
	{
		m_rCtxtMenuData.m_pclSSelectedActuator = ( CDS_Actuator * )wParam;
	}
	else if( NULL != dynamic_cast<CDS_Accessory *>( (CData *)wParam ) )
	{
		m_rCtxtMenuData.m_pclSSelectedAccessory = ( CDS_Accessory * )wParam;
	}
	else if( NULL != dynamic_cast<CDS_HmHub *>( (CData *)wParam ) )
	{
		m_rCtxtMenuData.m_pclSSelectedHUB = ( CDS_HmHub * )wParam;
	}
	else
	{
		return 0;
	}

	OnSSelEdit();
	return 0;
}

void CSelProdDockPane::_Reset()
{
	// Reset 'm_bInitialized' here because 'TSpread::Reset' will fire event that we wan't manage now.
	m_bInitialized = false;

	if( NULL != m_wndTabs.GetSafeHwnd() )
	{
		m_wndTabs.RemoveAllTabs();
	}

	for( mapSelProdPageIter iter = m_mapPageList.begin(); iter != m_mapPageList.end(); iter++ )
	{
		if( NULL != iter->second )
		{
			delete iter->second;
		}
	}

	m_mapPageList.clear();
	m_clArticleGroupList.Clean();

	// Clear HM tables.
	for( int iLoopPage = CDB_PageSetup::enCheck::FIRSTFIELD; iLoopPage < CDB_PageSetup::enCheck::LASTFIELD; iLoopPage++ )
	{
		m_arHMmap[iLoopPage].clear();
	}

	m_rCtxtMenuData.Clear();
}

void CSelProdDockPane::_Init()
{
	// By default we instantiate the class without create the dialog. It's to have access to some methods to check if we must display the
	// dialog or not.
	m_mapPageList[CDB_PageSetup::enCheck::AIRVENTSEPARATOR] = new CSelProdPageAirVentSep( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::PRESSMAINT] = new CSelProdPagePressMaint( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::SAFETYVALVE] = new CSelProdPageSafetyValve( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::BV] = new CSelProdPageBv( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::DPC] = new CSelProdPageDpC( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::DPCBCV] = new CSelProdPageDpCBCV( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::CV] = new CSelProdPageCtrl( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::SMARTCONTROLVALVE] = new CSelProdPageSmartControlValve( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::SMARTDPC] = new CSelProdPageSmartDpC( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::SIXWAYVALVE] = new CSelProdPage6WayValve( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::SV] = new CSelProdPageSv( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::TRV] = new CSelProdPageTrv( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::FLOORHEATINGCONTROL] = new CSelProdPageTCFloorHeatingControl( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::TAPWATERCONTROL] = new CSelProdPageTCTapWaterControl( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::HUB] = new CSelProdPageHub( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::PARTDEF] = new CSelProdPagePDef( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::PIPELIST] = new CSelProdPagePipeList( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::DIVERSITYFACTOR] = new CSelProdPageDiversityFactor( &m_clArticleGroupList );
	m_mapPageList[CDB_PageSetup::enCheck::ARTLIST] = new CSelProdPageArtList(&m_clArticleGroupList);
	m_mapPageList[CDB_PageSetup::enCheck::TENDERTEXT] = new CSelProdPageTender(&m_clArticleGroupList);

	CTable *pPipingTab = TASApp.GetpTADS()->GetpHydroModTable();
	ASSERT( NULL != pPipingTab );

	if( NULL != pPipingTab )
	{
		_ExtractHMInfos( pPipingTab );
	}

	CTable *pHubTab = TASApp.GetpTADS()->GetpHUBSelectionTable();
	ASSERT( NULL != pHubTab );

	if( NULL != pHubTab )
	{
		_ExtractHMInfos( pHubTab );
	}

	m_pTADS = TASApp.GetpTADS();
	ASSERT( NULL != m_pTADS );
	m_rCtxtMenuData.Clear();
	m_bInitialized = true;
}

void CSelProdDockPane::_ChangeTab( bool fShiftRight )
{
	int iActiveTab = m_wndTabs.GetActiveTab();

	if( true == fShiftRight )
	{
		if( iActiveTab == m_wndTabs.GetTabsNum() )
		{
			iActiveTab = 0;
		}
		else
		{
			iActiveTab++;
		}
	}
	else
	{
		if( 0 == iActiveTab )
		{
			iActiveTab = m_wndTabs.GetTabsNum();
		}
		else
		{
			iActiveTab--;
		}
	}

	m_wndTabs.SetActiveTab( iActiveTab );
	CSelProdPageBase *pclPage = dynamic_cast<CSelProdPageBase *>( m_wndTabs.GetTabWnd( iActiveTab ) );
	ASSERT( NULL != pclPage );
	m_eTabIndex = pclPage->GetPageID();
}

void CSelProdDockPane::_ExtractHMInfos( CTable *pTable )
{
	if( NULL == pTable )
	{
		return;
	}

	// TO COMPLETE: add separators & pressure maintenance products.

	for( IDPTR idptr = pTable->GetFirst(); NULL != idptr.MP; idptr = pTable->GetNext( idptr.MP ) )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );

		if( NULL == pHM )
		{
			continue;
		}

		bool bDpCAdded = false;

		// Test for CBI circuits, valve created in CBI mode.
		bool b = ( ( NULL != pHM->GetpBv() ) && ( _T('\0') != *pHM->GetpBv()->GetIDPtr().ID ) );
		b |= ( ( NULL != pHM->GetpDpC() ) && ( _T('\0') != *pHM->GetpDpC()->GetIDPtr().ID ) );
		b |= ( ( NULL != pHM->GetpCV() ) && ( _T('\0') != *pHM->GetpCV()->GetCvIDPtr().ID ) );
		b |= ( ( NULL != pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply ) ) && ( _T('\0') != *pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply )->GetIDPtr().ID ) );
		b |= ( ( NULL != pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn ) ) && ( _T('\0') != *pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn )->GetIDPtr().ID ) );
		b |= ( ( NULL != pHM->GetpSmartControlValve() ) && ( _T('\0') != *pHM->GetpSmartControlValve()->GetIDPtr().ID ) );
		b |= ( ( NULL != pHM->GetpSmartDpC() ) && ( _T('\0') != *pHM->GetpSmartDpC()->GetIDPtr().ID ) );

		if( false == b )
		{
			if( NULL != pHM->GetCBIValveIDPtr().MP )
			{
				m_arHMmap[CDB_PageSetup::enCheck::BV].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptHM, ( void * )pHM ) );
			}
			else
			{
				if( ( _T('\0') != *pHM->GetCBIType() ) && ( _T('\0') != *pHM->GetCBISize() ) )
				{
					m_arHMmap[CDB_PageSetup::enCheck::PARTDEF].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptHM, ( void * )pHM ) );
				}
			}
		}

		// Test for balancing valve and DpC, HUB valves must be skipped.
		if( NULL != pHM->GetpBv() && _T('\0') != *( pHM->GetpBv()->GetIDPtr() ).ID )				// Balancing valve exist
		{
			// Test if balancing valve is a measuring valve for DpC.
			if( NULL != pHM->GetpDpC() && _T('\0') != *( pHM->GetpDpC()->GetIDPtr() ).ID )
			{
				// Skip TAProduct used into HUB.
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pHM->GetpDpC()->GetIDPtr().MP );

				if( false == pTAP->IsForHub() && false == pTAP->IsForHubStation() )
				{
					m_arHMmap[CDB_PageSetup::enCheck::DPC].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCDpC, ( void * )pHM->GetpDpC() ) );
					bDpCAdded	= true;
				}
			}
			else
			{
				// Skip TAProduct used into HUB.
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pHM->GetpBv()->GetIDPtr().MP );

				if( false == pTAP->IsForHub() && false == pTAP->IsForHubStation() )
				{
					m_arHMmap[CDB_PageSetup::enCheck::BV].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCBV, ( void * )pHM->GetpBv() ) );
				}
			}
		}

		if( NULL != pHM->GetpBypBv() && _T('\0') != *( pHM->GetpBypBv()->GetIDPtr() ).ID )					// Bypass valve exist
		{
			m_arHMmap[CDB_PageSetup::enCheck::BV].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCBV, ( void * )pHM->GetpBypBv() ) );
		}

		if( NULL != pHM->GetpSecBv() && _T('\0') != *( pHM->GetpSecBv()->GetIDPtr() ).ID )					// Secondary Bv
		{
			m_arHMmap[CDB_PageSetup::enCheck::BV].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCBV, ( void * )pHM->GetpSecBv() ) );
		}

		// Test for control valve.
		if( NULL != pHM->GetpCV() && _T('\0') != *( pHM->GetpCV()->GetCvIDPtr().ID ) )
		{
			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pHM->GetpCV()->GetCvIDPtr().MP );

			if( NULL != pCV )
			{
				switch( pCV->GetCVParentTable() )
				{
					case CDB_ControlValve::ControlValveTable::CV:
					case CDB_ControlValve::ControlValveTable::BCV:
					case CDB_ControlValve::ControlValveTable::PICV:
						m_arHMmap[CDB_PageSetup::enCheck::CV].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCCV, ( void * )pHM->GetpCV() ) );
						break;

					case CDB_ControlValve::ControlValveTable::TRV:
						m_arHMmap[CDB_PageSetup::enCheck::TRV].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCCV, ( void * )pHM->GetpCV() ) );
						break;

					case CDB_ControlValve::ControlValveTable::DPCBCV:
						m_arHMmap[CDB_PageSetup::enCheck::DPCBCV].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCCV, ( void * )pHM->GetpCV() ) );
						break;
				}

				// Test is there is a Dp controller with the control valve.
				if( false == bDpCAdded && NULL != pHM->GetpDpC() && _T('\0') != *( pHM->GetpDpC()->GetIDPtr() ).ID )
				{
					// Skip TAProduct used into HUB.
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pHM->GetpDpC()->GetIDPtr().MP );

					if( false == pTAP->IsForHub() && false == pTAP->IsForHubStation() )
					{
						m_arHMmap[CDB_PageSetup::enCheck::DPC].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCDpC, ( void * )pHM->GetpDpC() ) );
						bDpCAdded = true;
					}
				}
			}
		}

		// Smart control valve.
		if( NULL != pHM->GetpSmartControlValve() && _T('\0') != *( pHM->GetpSmartControlValve()->GetIDPtr() ).ID )
		{
			m_arHMmap[CDB_PageSetup::enCheck::SMARTCONTROLVALVE].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCSmartControlValve, ( void * )pHM->GetpSmartControlValve() ) );
		}

		// Smart differential pressure controller
		if( NULL != pHM->GetpSmartDpC() && _T('\0') != *( pHM->GetpSmartDpC()->GetIDPtr() ).ID )
		{
			m_arHMmap[CDB_PageSetup::enCheck::SMARTDPC].push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCSmartDpC, ( void * )pHM->GetpSmartDpC() ) );
		}

		_ExtractHMInfos( pHM );
	}
}

CDB_PageSetup::enCheck CSelProdDockPane::_GetPageLinkedToLeftTab( void )
{
	CDB_PageSetup::enCheck ePage;

	switch( pMainFrame->GetCurrentProductSubCategory() )
	{
		// "ProductSubCategory::PSC_PM_AirVent" is included in "ProductSubCategory::PSC_PM_Separator".
		case ProductSubCategory::PSC_PM_Separator:
			ePage = CDB_PageSetup::enCheck::AIRVENTSEPARATOR;
			break;

		// "ProductSubCategory::PSC_PM_TecBox" and "ProductSubCategory::PSC_PM_SofteningAndDesalination" are included in "ProductSubCategory::PSC_PM_ExpansionVessel".
		case ProductSubCategory::PSC_PM_ExpansionVessel:
			ePage = CDB_PageSetup::enCheck::PRESSMAINT;
			break;

		case ProductSubCategory::PSC_PM_SafetyValve:
			ePage = CDB_PageSetup::enCheck::SAFETYVALVE;
			break;

		case ProductSubCategory::PSC_BC_RegulatingValve:
			ePage = CDB_PageSetup::enCheck::BV;
			break;

		case ProductSubCategory::PSC_BC_DpController:
			ePage = CDB_PageSetup::enCheck::DPC;
			break;

		case ProductSubCategory::PSC_BC_BalAndCtrlValve:
		case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
			ePage = CDB_PageSetup::enCheck::CV;
			break;

		case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:
			ePage = CDB_PageSetup::enCheck::DPCBCV;
			break;

		case ProductSubCategory::PSC_BC_ControlValve:
			ePage = CDB_PageSetup::enCheck::CV;
			break;

		case ProductSubCategory::PSC_BC_SmartControlValve:
			ePage = CDB_PageSetup::enCheck::SMARTCONTROLVALVE;
			break;

		case ProductSubCategory::PSC_BC_SmartDpC:
			ePage = CDB_PageSetup::enCheck::SMARTDPC;
			break;

		case ProductSubCategory::PSC_BC_6WayControlValve:
			ePage = CDB_PageSetup::enCheck::SIXWAYVALVE;
			break;

		case ProductSubCategory::PSC_BC_HUB:
			ePage = CDB_PageSetup::enCheck::HUB;
			break;

		case ProductSubCategory::PSC_TC_ThermostaticValve:
			ePage = CDB_PageSetup::enCheck::TRV;
			break;

		// "ProductSubCategory::PSC_TC_FloorHeatingControl" is only for direct selection.
		// "ProductSubCategory::PSC_TC_TapWaterControl" is only for direct selection.

		case ProductSubCategory::PSC_ESC_ShutoffValve:
			ePage = CDB_PageSetup::enCheck::SV;
			break;

		default:
			ePage = CDB_PageSetup::enCheck::BV;
			break;
	}

	return ePage;
}

CTable *CSelProdDockPane::_GetpTableLinkedToPage( CDB_PageSetup::enCheck ePage )
{
	if( 0 == m_mapPageList.count( ePage ) )
	{
		return NULL;
	}

	return m_mapPageList[ePage]->GetpTableLinkedToPage();
}

void CSelProdDockPane::_DeleteTADSObj( bool fConfirm )
{
	CString strClassName;
	CArray<LPARAM> arlpObj;
	m_mapPageList[m_eTabIndex]->GetAllSSelSelected( strClassName, arlpObj );

	if( true == fConfirm && arlpObj.GetCount() > 1 )
	{
		// If more than one product, we don't ask confirmation to each object but only once.
		if( IDNO == AfxMessageBox( TASApp.LoadLocalizedString( AFXMSG_DELETEALLENTRIES ), MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
		{
			return;
		}

		fConfirm = false;
	}

	for( int i = 0; i < arlpObj.GetCount(); i++ )
	{
		LPARAM lpObj = arlpObj.GetAt( i );

		CDS_SSel *pSSel = dynamic_cast<CDS_SSel *>( (CData *)lpObj );

		if( NULL != pSSel )
		{
			// If user confirmation needed...
			if( true == fConfirm )
			{
				CString str;

				// Check name to display in the message box.
				switch( pSSel->GetProductSubCategory() )
				{
					case ProductSubCategory::PSC_BC_RegulatingValve:
					{
						// Show balancing valve name if exist.
						CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( pclRegulatingValve != NULL )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclRegulatingValve->GetName() );
						}
					}
					break;

					case ProductSubCategory::PSC_BC_BalAndCtrlValve:
					case ProductSubCategory::PSC_BC_ControlValve:
					case ProductSubCategory::PSC_BC_PressureIndepCtrlValve:
					case ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve:
					{
						CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( pclRegulatingValve != NULL )
						{
							// Show control valve name.
							CString strTemp = pclRegulatingValve->GetName();
							CDS_SSelCtrl *pSSelCtrl = dynamic_cast<CDS_SSelCtrl *>( pSSel );

							if( pSSelCtrl != NULL && pSSelCtrl->GetActrIDPtr().MP != NULL )
							{
								// Add the actuator name if exist.
								CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)pSSelCtrl->GetActrIDPtr().MP );
								CString strName = pclActuator->GetName();


								if( pclActuator != NULL && false == strName.IsEmpty() )
								{
									CString strTemp2 = strTemp;
									strTemp2.Format( _T("%s / %s"), strTemp, strName );
									strTemp = strTemp2;
								}
							}

							FormatString( str, AFXMSG_DELETEENTRY, strTemp );
						}
					}
					break;

					case ProductSubCategory::PSC_BC_6WayControlValve:
					{
						CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( pcl6WayValve != NULL )
						{
							// Show 6-way valve name.
							CString strTemp = pcl6WayValve->GetName();
							CDS_SSel6WayValve *pclSSel6WayValve = dynamic_cast<CDS_SSel6WayValve *>( pSSel );

							if( pclSSel6WayValve != NULL && pclSSel6WayValve->GetActrIDPtr().MP != NULL )
							{
								// Add the actuator name if exist.
								CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)pclSSel6WayValve->GetActrIDPtr().MP );
								CString strName = pclActuator->GetName();

								if( pclActuator != NULL && false == strName.IsEmpty() )
								{
									CString strTemp2 = strTemp;
									strTemp2.Format( _T("%s / %s"), strTemp, strName );
									strTemp = strTemp2;
								}
							}

							FormatString( str, AFXMSG_DELETEENTRY, strTemp );
						}
					}
					break;

					case ProductSubCategory::PSC_BC_DpController:
					{
						CDS_SSelDpC *pSSelDpController = dynamic_cast<CDS_SSelDpC *>( pSSel );
						CDS_SSelDpReliefValve *pSSelDpReliefValve = dynamic_cast<CDS_SSelDpReliefValve *>( pSSel );

						if( pSSelDpController != NULL && pSSelDpController->GetDpCIDPtr().MP != NULL )
						{
							// Show Dp controller valve name.
							CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( (CData *)pSSelDpController->GetDpCIDPtr().MP );

							if( pclDpController != NULL )
							{
								CString strTemp = pclDpController->GetName();

								if( pSSelDpController->GetProductIDPtr().MP != NULL )
								{
									// Add the balancing valve name if exist.
									CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( (CData *)pSSel->GetProductIDPtr().MP );

									if( pclRegulatingValve != NULL )
									{
										CString strTemp2 = strTemp;
										strTemp2.Format( _T("%s / %s"), strTemp, pclRegulatingValve->GetName() );
										strTemp = strTemp2;
									}
								}

								FormatString( str, AFXMSG_DELETEENTRY, strTemp );
							}
						}
						else if( NULL != pSSelDpReliefValve && NULL != pSSelDpReliefValve->GetProductAs<CDB_DpReliefValve>() )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pSSelDpReliefValve->GetProductAs<CDB_DpReliefValve>()->GetName() );
						}
					}
					break;

					case ProductSubCategory::PSC_ESC_ShutoffValve:
					{
						// Show shut-off valve name if exist.
						CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( pclShutoffValve != NULL )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclShutoffValve->GetName() );
						}
					}
					break;

					case ProductSubCategory::PSC_TC_ThermostaticValve:
					{
						CDS_SSelRadSet *pSSelTrv = dynamic_cast<CDS_SSelRadSet *>( pSSel );

						if( pSSelTrv != NULL )
						{
							CString strTemp( _T("") );

							// 3 cases: supply valve is a TA product (Normal of flow limited) or supply valve is an insert and it has been selected by its Kv value.
							if( RVT_Inserts == (RadiatorValveType)pSSelTrv->GetRadiatorValveType() &&
									true == pSSelTrv->IsInsertInKv() )
							{
								strTemp = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_OTHERINSERT );
							}
							else if( NULL != dynamic_cast<CDB_ThermostaticValve *>( (CData *)pSSelTrv->GetSupplyValveIDPtr().MP ) )
							{
								strTemp = dynamic_cast<CDB_ThermostaticValve *>( (CData *)pSSelTrv->GetSupplyValveIDPtr().MP )->GetName();
							}
							else if( NULL != dynamic_cast<CDB_FlowLimitedControlValve *>( (CData *)pSSelTrv->GetSupplyValveIDPtr().MP ) )
							{
								strTemp = dynamic_cast<CDB_FlowLimitedControlValve *>( (CData *)pSSelTrv->GetSupplyValveIDPtr().MP )->GetName();
							}

							// Is return valve exist?
							if( pSSelTrv->GetReturnValveIDPtr().MP != NULL )
							{
								CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( (CData *)
										pSSelTrv->GetReturnValveIDPtr().MP );

								if( pclRegulatingValve != NULL )
								{
									CString strTemp2 = strTemp;
									strTemp2.Format( _T("%s / %s"), strTemp, pclRegulatingValve->GetName() );
									strTemp = strTemp2;
								}
							}

							FormatString( str, AFXMSG_DELETEENTRY, strTemp );
						}

					}
					break;

					// HYS-1571.
					case ProductSubCategory::PSC_TC_FloorHeatingControl:
					{
						// Show floor heating control name if exist.
						CDB_FloorHeatingManifold *pclFloorHeatingManifold = dynamic_cast<CDB_FloorHeatingManifold *>( (CData *)pSSel->GetProductIDPtr().MP );
						CDB_FloorHeatingValve *pclFloorHeatingValve = dynamic_cast<CDB_FloorHeatingValve *>( (CData *)pSSel->GetProductIDPtr().MP );
						CDB_FloorHeatingController *pclFloorHeatingController = dynamic_cast<CDB_FloorHeatingController *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( NULL != pclFloorHeatingManifold )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclFloorHeatingManifold->GetName() );
						}
						else if( NULL != pclFloorHeatingValve )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclFloorHeatingValve->GetName() );
						}
						else if( NULL != pclFloorHeatingController )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclFloorHeatingController->GetName() );
						}
					}
					break;

					// HYS-1258.
					case ProductSubCategory::PSC_TC_TapWaterControl:
					{
						// Show tap water control name if exist.
						CDB_TapWaterControl *pclTapWaterControl = dynamic_cast<CDB_TapWaterControl *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( NULL != pclTapWaterControl )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclTapWaterControl->GetName() );
						}

					}
					break;

					case ProductSubCategory::PSC_PM_Separator:
					{
						// Show air vents or separator name if exist.
						CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( NULL != pclSeparator )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclSeparator->GetName() );
						}
						else
						{
							CDB_AirVent *pclAirVent = dynamic_cast<CDB_AirVent *>( (CData *)pSSel->GetProductIDPtr().MP );

							if( NULL != pclAirVent )
							{
								FormatString( str, AFXMSG_DELETEENTRY, pclAirVent->GetName() );
							}
						}
					}
					break;
					
					// HYS-1088
					case ProductSubCategory::PSC_PM_SafetyValve:
					{
						// Show safety valve name if exist.
						CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( (CData *)pSSel->GetProductIDPtr().MP );
						// HYS-1741
						CDB_PWQPressureReducer *pclPressureReducer = dynamic_cast<CDB_PWQPressureReducer*>( (CData *)pSSel->GetProductIDPtr().MP );

						if( NULL != pclSafetyValve )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclSafetyValve->GetName() );
						}
						else if( NULL != pclPressureReducer )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclPressureReducer->GetName() );
						}

					}
					break;

					// Smart control valve.
					/*
					case ProductSubCategory::PSC_ESC_ShutoffValve:
					{
						// Show shut-off valve name if exist.
						CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( pclShutoffValve != NULL )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclShutoffValve->GetName() );
						}
					}
					break;
					*/

					default:
					{
						CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSSel->GetProductIDPtr().MP );

						if( NULL != pclProduct )
						{
							FormatString( str, AFXMSG_DELETEENTRY, pclProduct->GetName() );
						}
					}
				}

				if( IDNO == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
				{
					continue;
				}
			}

			m_pTADS->DeleteSelectionFromTADS( pSSel->GetIDPtr() );
			continue;
		}

		if( strClassName == CLASS( CDS_SSelPMaint ) )
		{
			// Delete a pressure maintenance from TADS...
			CDS_SSelPMaint *pclSSelPMaint = dynamic_cast<CDS_SSelPMaint *>( (CData *)lpObj );

			if( NULL != pclSSelPMaint )
			{
				// If user confirmation needed...
				if( true == fConfirm )
				{
					CString str, str1;
					CDB_Product *pclProduct = pclSSelPMaint->GetSelectedProduct();

					if( NULL != pclProduct )
					{
						if( CDS_SSelPMaint::NoPressurization != pclSSelPMaint->GetSelectionType() )
						{
							// Complete pressurisation selection.
							str = TASApp.LoadLocalizedString( AFXMSG_DELETEENTRYPM );
						}
						else
						{
							str1 = pclProduct->GetName();
							FormatString( str, AFXMSG_DELETEENTRY, str1 );
						}
					}

					if( IDNO == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
					{
						continue;
					}
				}

				m_pTADS->DeleteSelectionFromTADS( pclSSelPMaint->GetIDPtr() );
			}
		}

		if( strClassName == CLASS( CDS_HmHub ) )
		{
			CDS_HmHub *pHub = ( CDS_HmHub * )lpObj;

			if( NULL != pHub )
			{
				if( true == fConfirm )
				{
					CString str;
					FormatString( str, AFXMSG_DELETEENTRY, pHub->GetHMName() );

					if( IDNO == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
					{
						continue;
					}
				}

				m_pTADS->DeleteHM( pHub );
			}

			continue;
		}

		if( strClassName == CLASS( CDS_Actuator ) )
		{
			// Delete actuator from TADS...
			CDS_Actuator *pActr = dynamic_cast<CDS_Actuator *>( (CData *)lpObj );

			if( NULL != pActr )
			{
				// If user confirmation needed...
				if( true == fConfirm )
				{
					CString str, str1;
					str1 = ( ( CDB_Actuator * )( pActr->GetActuatorIDPtr().MP ) )->GetName();
					FormatString( str, AFXMSG_DELETEENTRY, str1 );

					if( IDNO == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
					{
						continue;
					}
				}

				m_pTADS->DeleteSelectionFromTADS( pActr->GetIDPtr() );
			}
		}

		if( strClassName == CLASS( CDS_Accessory ) )
		{
			// Delete accessory from TADS...
			CDS_Accessory *pAcc = dynamic_cast<CDS_Accessory *>( (CData *)lpObj );

			if( NULL != pAcc )
			{
				// If user confirmation needed...
				if( true == fConfirm )
				{
					CString str, str1;
					str1 = ( (CDB_Product * )( pAcc->GetAccessoryIDPtr().MP ) )->GetName();
					FormatString( str, AFXMSG_DELETEENTRY, str1 );

					if( IDNO == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
					{
						continue;
					}
				}

				m_pTADS->DeleteSelectionFromTADS( pAcc->GetIDPtr() );
			}
		}
	}
}

void CSelProdDockPane::_DisplayRequestedDocSheet( CatalogType eCatalogType )
{
	if( NULL != m_rCtxtMenuData.m_pclProduct )
	{
		pMainFrame->PostMessageToDescendants( WM_USER_DISPLAYDOCUMENTATION, 0, ( LPARAM )m_rCtxtMenuData.m_pclProduct->GetIDPtr().ID );
	}
	else if( NULL != m_rCtxtMenuData.m_pclActuator )
	{
		pMainFrame->PostMessageToDescendants( WM_USER_DISPLAYDOCUMENTATION, 0, ( LPARAM )m_rCtxtMenuData.m_pclProduct->GetIDPtr().ID );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CContainerBase - base class helper for exporting to excel and printing.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
CContainerBase::CContainerBase( CWnd *pclParent )
	: CSelProdDockPane()
{
	m_pclParent = pclParent;
}

CContainerBase::~CContainerBase()
{
	_ClearMap();
}

bool CContainerBase::Init()
{
	if( NULL == m_pclParent )
	{
		return false;
	}

	if( NULL == GetSafeHwnd() )
	{
		if( FALSE == Create( _T(""), m_pclParent, CRect( 0, 0, 400, 400 ), TRUE, 50000, WS_CHILD, AFX_CBRS_REGULAR_TABS ) )
		{
			return false;
		}
	}

	// By default, we don't show the pane.
	ShowPane( FALSE, FALSE, FALSE );

	// We just reset, init and pre-init each page without drawing it just to be able to know if there are some pages to print or export.
	ResetInit();

	for( int iLoopPage = CDB_PageSetup::enCheck::FIRSTFIELD; iLoopPage < CDB_PageSetup::enCheck::LASTFIELD; iLoopPage++ )
	{
		CDB_PageSetup::enCheck eLoopPage = ( CDB_PageSetup::enCheck )iLoopPage;
		CSelProdPageBase *pPage = GetpPage( eLoopPage );

		if( NULL != pPage )
		{
			pPage->PreInit( m_arHMmap[eLoopPage] );
		}
	}

	return true;
}

BEGIN_MESSAGE_MAP( CContainerBase, CSelProdDockPane )
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CContainerBase::OnDestroy()
{
	_ClearMap();
	CSelProdDockPane::OnDestroy();
}

bool CContainerBase::PrepareMultiSheetInOne( bool fForPrint, bool fForExport )
{
	if( NULL == GetSafeHwnd() )
	{
		return false;
	}

	RedrawAll( false, fForPrint, fForExport );
	_ClearMap();

	for( int iLoopPage = CDB_PageSetup::enCheck::FIRSTFIELD; iLoopPage < CDB_PageSetup::enCheck::LASTFIELD; iLoopPage++ )
	{
		CDB_PageSetup::enCheck eLoopPage = ( CDB_PageSetup::enCheck )iLoopPage;
		CSelProdPageBase *pPage = GetpPage( eLoopPage );
		CTender clTender;

		if( NULL == pPage 
			|| false == pPage->HasSomethingToPrint()
			|| true == pPage->UseOnlyOneSpread() 
			|| ( eLoopPage == CDB_PageSetup::TENDERTEXT && false == clTender.IsTenderNeeded() ) )
		{
			continue;
		}

		CSSheet *pclSheet = new CSSheet();
		
		if( NULL == pclSheet )
		{
			continue;
		}

		if( FALSE == pclSheet->Create( ( GetStyle() | WS_CHILD ) & ~WS_BORDER, CRect( 0, 0, 400, 400 ), this, IDC_FPSPREAD ) )
		{
			continue;
		}

		pclSheet->SetSheetVisible( 1, TRUE );
		pclSheet->SetSheetName( 1, TASApp.LoadLocalizedString( pPage->GetTabTitleID() ) );
		pclSheet->SetSheet( 1 );
		pclSheet->Init();
		pclSheet->SetOperationMode( SS_OPMODE_NORMAL );

		pPage->ClearAllProductSelected();
		pPage->CopyMultiSheetInSheet( pclSheet );

		m_mapAllInOneSheetList[eLoopPage] = pclSheet;
	}

	return true;
}

void CContainerBase::_ClearMap( void )
{
	if( 0 != (int)m_mapAllInOneSheetList.size() )
	{
		for( mapPageSetupCSSheetIter iter = m_mapAllInOneSheetList.begin(); iter != m_mapAllInOneSheetList.end(); ++iter )
		{
			if( NULL != iter->second )
			{
				delete iter->second;
			}
		}
		
		m_mapAllInOneSheetList.clear();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CContainerForExcelExport - class helper for exporting to excel.
/////////////////////////////////////////////////////////////////////////////////////////////////////////

CContainerForExcelExport::CContainerForExcelExport( CWnd *pclParent ) : CContainerBase( pclParent )
{
	m_selectionMode = _T("");

	// For pressurisation.
	m_bAtLeastOneCooling = false;
	m_bAtLeastOneSolar = false;
	m_bAtLeastOneSWKINorm = false;
	m_bAtLeastOneNotSWKINorm = false;
	m_bAtLeastOneSWKINormWithStorageTank = false;
}

bool CContainerForExcelExport::Export( CString strFileName )
{
	if( NULL == GetSafeHwnd() )
	{
		return false;
	}

	if( false == PrepareMultiSheetInOne( false, true ) )
	{
		return false;
	}

	Excel_Workbook out;

	for( int iLoopPage = (int)CDB_PageSetup::enCheck::FIRSTFIELD; iLoopPage < (int)CDB_PageSetup::enCheck::LASTFIELD; ++iLoopPage )
	{
		CDB_PageSetup::enCheck eLoopPage = (CDB_PageSetup::enCheck)iLoopPage;
		CSelProdPageBase *pPage = GetpPage( eLoopPage );

		if( NULL == pPage || NULL == pPage->GetSheetPointer() )
		{
			continue;
		}

		if( false == pPage->UseOnlyOneSpread() )
		{
			if( 0 == m_mapAllInOneSheetList.count( eLoopPage ) || NULL == m_mapAllInOneSheetList[eLoopPage] )
			{
				continue;
			}

			out.AddSheet( m_mapAllInOneSheetList[eLoopPage] );
		}
		else
		{
			CSSheet *pclSheet = pPage->GetSheetPointer();
			CString strTabName = TASApp.LoadLocalizedString( pPage->GetTabTitleID() );
			int iResult = strTabName.Find( _T("&&"), 0 );
			
			if( iResult >= 0 )
			{
				strTabName.Delete( iResult, 1 );
			}

			pclSheet->SetSheetName( 1, strTabName );
			out.AddSheet( pclSheet );
		}
	}

	out.Write( strFileName );

	return true;
}

bool CContainerForExcelExport::ExportInOneSheet( CString strFileName )
{
	if( NULL == GetSafeHwnd() )
	{
		return false;
	}

	Excel_Workbook out;
	bool result = false;
	CSSheet *pclSheet = new CSSheet(); 
	result = pclSheet->Create( ( GetStyle() | WS_CHILD ) & ~WS_BORDER, CRect( 0, 0, 400, 400 ), this, IDC_FPSPREAD );
	pclSheet->ModifyStyle( WS_VISIBLE, 0 );
	pclSheet->SetSheetName( 1, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMEVALVE ) );
	result = InitProjectResultExport( pclSheet);
	
	if( result )
	{
		out.AddSheet( pclSheet );
	}

	// HYS-1071: Pressurisation and water quality.
	CSSheet *pclSheetPMWQ = new CSSheet();
	result = pclSheetPMWQ->Create( ( GetStyle() | WS_CHILD ) & ~WS_BORDER, CRect( 0, 0, 400, 400 ), this, IDC_FPSPREAD );
	pclSheetPMWQ->ModifyStyle( WS_VISIBLE, 0 );
	pclSheetPMWQ->SetSheetName( 1, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMEPNEUMATEX ) );
	result = InitProjectPMWQExport( pclSheetPMWQ );
	
	if( result )
	{
		out.AddSheet( pclSheetPMWQ );
	}
	
	// HYS-1070: Add article list.
	if( true == PrepareMultiSheetInOne( false, true ) )
	{
		CSelProdPageBase *pPage = GetpPage( CDB_PageSetup::enCheck::ARTLIST );

		if( NULL != pPage && NULL != pPage->GetSheetPointer() )
		{
			CSSheet *pclSheetArticles = pPage->GetSheetPointer();
			double dwidth = 0.0;
			
			// HYS-1095: 5 for col  SelProdPageArtList::ColumnDescriptionPArtList::Description.
			pclSheetArticles->GetMaxTextColWidth( 5, &dwidth );
			pclSheetArticles->SetColWidth( 5, dwidth );
			CString strTabName = TASApp.LoadLocalizedString( pPage->GetTabTitleID() );
			int iResult = strTabName.Find( _T("&&"), 0 );

			if( iResult >= 0 )
			{
				strTabName.Delete( iResult, 1 );
			}

			pclSheetArticles->SetSheetName( 1, strTabName );
			out.AddSheet( pclSheetArticles );
		}
	}
	
	out.Write( strFileName, &m_vectColumnShown, true );
	delete pclSheetPMWQ;
	delete pclSheet;
	
	return true;
}

bool CContainerForExcelExport::ExportTADiagnostic( CString strFileName )
{
	if( NULL == GetSafeHwnd() )
	{
		return false;
	}

	Excel_Workbook out;
	bool result = false;
	// Sheet measurements
	CSSheet* pclSheet = new CSSheet();
	result = pclSheet->Create( ( GetStyle() | WS_CHILD ) & ~WS_BORDER, CRect( 0, 0, 400, 400 ), this, IDC_FPSPREAD );
	pclSheet->ModifyStyle( WS_VISIBLE, 0 );
	pclSheet->SetSheetName( 1, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMEMEASUREMENTS ) );
	result = InitExportMeasurements( pclSheet );

	if( result )
	{
		out.AddSheet( pclSheet );
	}

	// TA-Diagnostic.
	CSSheet* pclSheetDiag = new CSSheet();
	result = pclSheetDiag->Create( ( GetStyle() | WS_CHILD ) & ~WS_BORDER, CRect( 0, 0, 400, 400 ), this, IDC_FPSPREAD );
	pclSheetDiag->ModifyStyle( WS_VISIBLE, 0 );
	pclSheetDiag->SetSheetName( 1, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMETADIAGNOSTIC ) );
	result = InitExportTADiagnostic( pclSheetDiag );

	if( result )
	{
		out.AddSheet( pclSheetDiag );
	}

	// Add article list.
	if( true == PrepareMultiSheetInOne( false, true ) )
	{
		CSelProdPageBase* pPage = GetpPage( CDB_PageSetup::enCheck::ARTLIST );

		if( NULL != pPage && NULL != pPage->GetSheetPointer() )
		{
			CSSheet* pclSheetArticles = pPage->GetSheetPointer();
			double dwidth = 0.0;

			// HYS-1095: 5 for col  SelProdPageArtList::ColumnDescriptionPArtList::Description.
			pclSheetArticles->GetMaxTextColWidth( 5, &dwidth );
			pclSheetArticles->SetColWidth( 5, dwidth );
			CString strTabName = TASApp.LoadLocalizedString( pPage->GetTabTitleID() );
			int iResult = strTabName.Find( _T( "&&" ), 0 );

			if( iResult >= 0 )
			{
				strTabName.Delete( iResult, 1 );
			}

			pclSheetArticles->SetSheetName( 1, strTabName );
			out.AddSheet( pclSheetArticles );
		}
	}

	out.Write( strFileName, &m_vectColumnShown, true );
	delete pclSheetDiag;
	delete pclSheet;

	return true;
}

void CContainerForExcelExport::InitMeasurementSheet( CSSheet* pclSheet )
{
	if( NULL == pclSheet )
	{
		return;
	}


	pclSheet->SetMaxRows( RD_Header_Meas_FirstCirc - 1 );
	pclSheet->SetMaxCols( CD_Measurement_Pointer - 1 );


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Information' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Measurement_DateTime, RD_Header_Meas_GroupName, CD_Measurement_Description - CD_Measurement_DateTime + 1, 1 );
	pclSheet->SetStaticText( CD_Measurement_DateTime, RD_Header_Meas_GroupName, IDS_SSHEET_MEAS_INFO );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	// Date and time.
	// pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetStaticText( CD_Measurement_DateTime, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DATETIME );

	// Reference.
	pclSheet->SetStaticText( CD_Measurement_Reference, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_REFERENCE );

	// Description.
	pclSheet->SetStaticText( CD_Measurement_Description, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DESC );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Circuit' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Measurement_WaterChar, RD_Header_Meas_GroupName, CD_Measurement_KvSignal - CD_Measurement_WaterChar + 1, 1 );
	pclSheet->SetStaticText( CD_Measurement_WaterChar, RD_Header_Meas_GroupName, IDS_SSHEET_MEAS_CIRCUIT );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	// WaterChar.
	pclSheet->SetStaticText( CD_Measurement_WaterChar, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_WC );

	// Valve.
	pclSheet->SetStaticText( CD_Measurement_Valve, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_VALVE );

	// Presetting.
	pclSheet->SetStaticText( CD_Measurement_Presetting, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_PRESET );

	// Setting.
	pclSheet->SetStaticText( CD_Measurement_Setting, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_SETTING );

	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	ASSERT( pUnitDB );
	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		// Kv
		pclSheet->SetStaticText( CD_Measurement_Kv, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_KV );
		// Kv signal.
		pclSheet->SetStaticText( CD_Measurement_KvSignal, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_KVSIG );
	}
	else
	{
		// Kv
		pclSheet->SetStaticText( CD_Measurement_Kv, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_CV );
		// Kv signal.
		pclSheet->SetStaticText( CD_Measurement_KvSignal, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_CVSIG );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Dp' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Measurement_Dp, RD_Header_Meas_GroupName, CD_Measurement_Dpl - CD_Measurement_Dp + 1, 1 );
	pclSheet->SetStaticText( CD_Measurement_Dp, RD_Header_Meas_GroupName, IDS_SSHEET_MEAS_DIFFPRESS );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	// Dp
	pclSheet->SetStaticText( CD_Measurement_Dp, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DP );

	// DpL.
	pclSheet->SetStaticText( CD_Measurement_Dpl, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DPL );


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Flow' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Measurement_MeasureDp, RD_Header_Meas_GroupName, CD_Measurement_FlowDeviation - CD_Measurement_MeasureDp + 1, 1 );
	pclSheet->SetStaticText( CD_Measurement_MeasureDp, RD_Header_Meas_GroupName, IDS_SSHEET_MEAS_FLOW );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	// Measured Dp.
	pclSheet->SetStaticText( CD_Measurement_MeasureDp, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DP );

	// Design Dp.
	pclSheet->SetStaticText( CD_Measurement_DesignDp, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DESIGNDP );

	// Measured Flow.
	pclSheet->SetStaticText( CD_Measurement_MeasureFlow, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_MEASFLOW );

	// Design flow.
	pclSheet->SetStaticText( CD_Measurement_DesignFlow, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DESIGNFLOW );

	// Fow deviation.
	pclSheet->SetStaticText( CD_Measurement_FlowDeviation, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_FLOWDEV );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Temperature' group
	////////////////////////////////////////////////////////////////////////////////////////////////////	

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Measurement_TempHH, RD_Header_Meas_GroupName, CD_Measurement_DiffTempDeviation - CD_Measurement_TempHH + 1, 1 );
	pclSheet->SetStaticText( CD_Measurement_TempHH, RD_Header_Meas_GroupName, IDS_SSHEET_MEAS_TEMPERATURE );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	// Temp. on HH.
	pclSheet->SetStaticText( CD_Measurement_TempHH, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_TEMPHH );

	// Temp. 1 on DPS.
	pclSheet->SetStaticText( CD_Measurement_Temp1DPS, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_TEMP1DPS );

	// Temp. 2 on DPS
	pclSheet->SetStaticText( CD_Measurement_Temp2DPS, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_TEMP2DPS );

	// Temp. Ref.
	pclSheet->SetStaticText( CD_Measurement_TempRef, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_TEMPREF );

	// Diff temp.
	pclSheet->SetStaticText( CD_Measurement_DiffTemp, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DIFFTEMP );

	// Design diff. temp.
	pclSheet->SetStaticText( CD_Measurement_DesignDiffTemp, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DESIGNDT );

	// Diff. Temp. Deviation.
	pclSheet->SetStaticText( CD_Measurement_DiffTempDeviation, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DTDEV );


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Power' group
	////////////////////////////////////////////////////////////////////////////////////////////////////	

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Measurement_Power, RD_Header_Meas_GroupName, CD_Measurement_PowerDeviation - CD_Measurement_Power + 1, 1 );
	pclSheet->SetStaticText( CD_Measurement_Power, RD_Header_Meas_GroupName, IDS_SSHEET_MEAS_POWER );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	// Power.
	pclSheet->SetStaticText( CD_Measurement_Power, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_MEASPOWER );


	// Design power.
	pclSheet->SetStaticText( CD_Measurement_DesignPower, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_DESIGNPOWER );

	// Power deviation.
	pclSheet->SetStaticText( CD_Measurement_PowerDeviation, RD_Header_Meas_ColName, IDS_SSHEET_MEAS_POWERDEV );


	// Border.
	pclSheet->SetCellBorder( CD_Measurement_DateTime, RD_Header_Meas_GroupName - 1, CD_Measurement_PowerDeviation - 1, RD_Header_Meas_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Measurement_DateTime, RD_Header_Meas_GroupName - 1, CD_Measurement_PowerDeviation - 1, RD_Header_Meas_FirstCirc - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Measurement_DateTime, RD_Header_Meas_GroupName - 1, CD_Measurement_PowerDeviation, RD_Header_Meas_FirstCirc - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Measurement_DateTime, RD_Header_Meas_GroupName - 1, CD_Measurement_PowerDeviation - 1, RD_Header_Meas_FirstCirc - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );
}

bool CContainerForExcelExport::InitExportMeasurements( CSSheet* pclSheet )
{
	if( NULL == pclSheet )
	{
		return false;
	}

	pclSheet->SetBool( SSB_REDRAW, FALSE );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSheet->FormatStaticText( -1, -1, -1, -1, CteEMPTY_STRING );

	// Increase ROW height.
	double RowHeight = 12.75;
	pclSheet->SetRowHeight( RD_Header_Meas_GroupName, RowHeight * 1.5 );
	pclSheet->SetRowHeight( RD_Header_Meas_ColName, RowHeight * 10 );

	// Resize column.
	SetAllColumnWidth( pclSheet, CD_Measurement_DateTime, CD_Measurement_PowerDeviation, RD_Header_Meas_ColName, 30 );

	// Initialize sheet.
	InitMeasurementSheet( pclSheet );

	long lRow = RD_Header_Meas_FirstCirc;
	// Get HM table.
	CTable* pHMTab = TASApp.GetpTADS()->GetpHydroModTable();
	for( IDPTR IDPtr = pHMTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pHMTab->GetNext( IDPtr.MP ) )
	{
		CTable* pRootTable = (CTable *) IDPtr.MP;
		CDS_HydroMod* pHMRoot = (CDS_HydroMod *)IDPtr.MP;
		// Measurements.
		if( NULL != pHMRoot && true == pHMRoot->IsMeasurementExistsInProject( pHMRoot ) )
		{
			// Measurements exists
			FillHMMeasurementRow( pHMRoot, pclSheet, &lRow );
		}
	}

	// Resize all column width.
	SetAllColumnWidth( pclSheet, CD_Measurement_DateTime, CD_Measurement_PowerDeviation, RD_Header_Meas_ColName, 30 );
	HideEmptyColumns( pclSheet, RD_Header_Meas_FirstCirc, CD_Measurement_Sub, CD_Measurement_Pointer );

	pclSheet->SetBool( SSB_REDRAW, TRUE );

	if( lRow <= RD_Header_Meas_FirstCirc )
	{
		return false;
	}

	return true;
}

void CContainerForExcelExport::InitTADiagnosticSheet( CSSheet* pclSheet )
{
	if( NULL == pclSheet )
	{
		return;
	}

	pclSheet->SetMaxRows( RD_Header_Diag_FirstCirc - 1 );
	pclSheet->SetMaxCols( CD_TADiagnostic_Pointer - 1 );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Information' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_TADiagnostic_DateTime, RD_Header_Diag_GroupName, CD_TADiagnostic_Description - CD_TADiagnostic_DateTime + 1, 1 );
	pclSheet->SetStaticText( CD_TADiagnostic_DateTime, RD_Header_Diag_GroupName, IDS_SSHEET_MEAS_INFO );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	// Date and time.
	pclSheet->SetStaticText( CD_TADiagnostic_DateTime, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_DATETIME );

	// Reference.
	pclSheet->SetStaticText( CD_TADiagnostic_Reference, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_REFERENCE );

	// Description.
	pclSheet->SetStaticText( CD_TADiagnostic_Description, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_DESC );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Circuit' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_TADiagnostic_WaterChar, RD_Header_Diag_GroupName, CD_TADiagnostic_DesignFlow - CD_TADiagnostic_WaterChar + 1, 1 );
	pclSheet->SetStaticText( CD_TADiagnostic_WaterChar, RD_Header_Diag_GroupName, IDS_SSHEET_MEAS_CIRCUIT );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	// WaterChar.
	pclSheet->SetStaticText( CD_TADiagnostic_WaterChar, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_WC );

	// Valve.
	pclSheet->SetStaticText( CD_TADiagnostic_Valve, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_VALVE );

	// Presetting.
	pclSheet->SetStaticText( CD_TADiagnostic_Presetting, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_PRESET );

	// Computed setting.
	pclSheet->SetStaticText( CD_TADiagnostic_ComputedSetting, RD_Header_Diag_ColName, IDS_SSHEET_DIAG_CALCSETTING );

	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	ASSERT( pUnitDB );
	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		// Kv
		pclSheet->SetStaticText( CD_TADiagnostic_Kv, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_KV );
		// Kv signal.
		pclSheet->SetStaticText( CD_TADiagnostic_KvSignal, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_KVSIG );
	}
	else
	{
		// Kv
		pclSheet->SetStaticText( CD_TADiagnostic_Kv, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_CV );
		// Kv signal.
		pclSheet->SetStaticText( CD_TADiagnostic_KvSignal, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_CVSIG );
	}

	// Design flow.
	pclSheet->SetStaticText( CD_TADiagnostic_DesignFlow, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_DESIGNFLOW );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'TA-Diagnostic measuring data' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_TADiagnostic_Dp1, RD_Header_Diag_GroupName, CD_TADiagnostic_FlowDeviation - CD_TADiagnostic_Dp1 + 1, 1 );
	pclSheet->SetStaticText( CD_TADiagnostic_Dp1, RD_Header_Diag_GroupName, IDS_SSHEET_DIAG_MEASDATA );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	// Dp1
	pclSheet->SetStaticText( CD_TADiagnostic_Dp1, RD_Header_Diag_ColName, IDS_SSHEET_DIAG_DP1 );

	// Setting1
	pclSheet->SetStaticText( CD_TADiagnostic_Setting1, RD_Header_Diag_ColName, IDS_SSHEET_DIAG_SETTING1 );

	// Dp2
	pclSheet->SetStaticText( CD_TADiagnostic_Dp2, RD_Header_Diag_ColName, IDS_SSHEET_DIAG_DP2 );

	// Measured flow
	pclSheet->SetStaticText( CD_TADiagnostic_MeasFlow, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_MEASFLOW );

	// Fow deviation.
	pclSheet->SetStaticText( CD_TADiagnostic_FlowDeviation, RD_Header_Diag_ColName, IDS_SSHEET_MEAS_FLOWDEV );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'TA-Diagnostic Dp trouble shooting' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_TADiagnostic_CircuitDpMeas, RD_Header_Diag_GroupName, CD_TADiagnostic_DistribDpDesign - CD_TADiagnostic_CircuitDpMeas + 1, 1 );
	pclSheet->SetStaticText( CD_TADiagnostic_CircuitDpMeas, RD_Header_Diag_GroupName, IDS_SSHEET_DIAG_DPSHOOT );

	// Column name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	// Circuit Measured Dp. merged with Circuit Design Dp.

	pclSheet->AddCellSpanW( CD_TADiagnostic_CircuitDpMeas, RD_Header_Diag_ColName, CD_TADiagnostic_CircuitDpDesign - CD_TADiagnostic_CircuitDpMeas + 1, 1 );
	pclSheet->SetStaticText( CD_TADiagnostic_CircuitDpMeas, RD_Header_Diag_ColName, IDS_SHEET_CIRCUITDPMEASDESIGN );

	// Distrib. Measured Dp. merged Distrib. Design Dp.

	pclSheet->AddCellSpanW( CD_TADiagnostic_DistribDpMeas, RD_Header_Diag_ColName, CD_TADiagnostic_DistribDpDesign - CD_TADiagnostic_DistribDpMeas + 1, 1 );
	pclSheet->SetStaticText( CD_TADiagnostic_DistribDpMeas, RD_Header_Diag_ColName, IDS_SHEET_DISTRIBDPMEASDESIGN );

	// Border.
	pclSheet->SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Diag_GroupName - 1, CD_TADiagnostic_Pointer - 1, RD_Header_Diag_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Diag_GroupName - 1, CD_TADiagnostic_Pointer - 1, RD_Header_Diag_FirstCirc - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Diag_GroupName - 1, CD_TADiagnostic_Pointer, RD_Header_Diag_FirstCirc - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Diag_GroupName - 1, CD_TADiagnostic_Pointer - 1, RD_Header_Diag_FirstCirc - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );
}

bool CContainerForExcelExport::InitExportTADiagnostic( CSSheet *pclSheet )
{
	if( NULL == pclSheet )
	{
		return false;
	}

	pclSheet->SetBool( SSB_REDRAW, FALSE );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSheet->FormatStaticText( -1, -1, -1, -1, CteEMPTY_STRING );

	// Increase ROW height.
	double RowHeight = 12.75;
	pclSheet->SetRowHeight( RD_Header_Diag_GroupName, RowHeight * 1.5 );
	pclSheet->SetRowHeight( RD_Header_Diag_ColName, RowHeight * 10.0 );

	// Resize column.
	SetAllColumnWidth( pclSheet, CD_TADiagnostic_DateTime, CD_TADiagnostic_DistribDpDesign, RD_Header_Diag_ColName, 30.0 );

	// Initialize sheet.
	InitTADiagnosticSheet( pclSheet );

	long lRow = RD_Header_Diag_FirstCirc;
	
	// Get HM table.
	CTable *pHMTab = TASApp.GetpTADS()->GetpHydroModTable();

	for( IDPTR IDPtr = pHMTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHMTab->GetNext( IDPtr.MP ) )
	{
		CTable *pRootTable = (CTable *)IDPtr.MP;
		CDS_HydroMod *pHMRoot = (CDS_HydroMod *)IDPtr.MP;
		
		// Measurements.
		bool bTADiagnosticExists = false;

		if( NULL != pHMRoot && true == pHMRoot->IsMeasurementExistsInProject( pHMRoot ) )
		{
			// Remarks: For balancing, all modules must have measurements. Thus, if first module is balanced, it's ok.
			CDS_HydroMod::CMeasData* pMeasData = pHMRoot->GetpMeasData( 0 );

			if( NULL != pMeasData )
			{
				bTADiagnosticExists = ( pMeasData->GetFlagBalanced() == CDS_HydroModX::eTABalanceMethod::etabmTABalPlus );
			}
			else		//No PV! test first child
			{
				CDS_HydroMod *pChildHM = dynamic_cast<CDS_HydroMod *>( pHMRoot->GetFirst().MP );

				if( NULL != pChildHM )
				{
					pMeasData = pChildHM->GetpMeasData( 0 );

					if( NULL != pMeasData )
					{
						bTADiagnosticExists = ( pMeasData->GetFlagBalanced() == CDS_HydroModX::eTABalanceMethod::etabmTABalPlus );
					}
				}
			}
			
			if( true == bTADiagnosticExists )
			{
				FillHMTADiagnosticRow( pHMRoot, pclSheet, &lRow );
			}
		}
	}

	// Resize all column width.
	SetAllColumnWidth( pclSheet, CD_TADiagnostic_DateTime, CD_TADiagnostic_DistribDpDesign, RD_Header_Diag_ColName, 30.0 );
	
	// Not for the last columns that are merged column.
	HideEmptyColumns( pclSheet, RD_Header_Diag_FirstCirc, CD_TADiagnostic_Sub, CD_TADiagnostic_FlowDeviationSep );

	pclSheet->SetBool( SSB_REDRAW, TRUE );

	if( lRow <= RD_Header_Diag_FirstCirc )
	{
		return false;
	}

	return true;
}

void CContainerForExcelExport::FillHMMeasurementRow( CDS_HydroMod *pHM, CSSheet *pclSheet, long *plRow )
{
	if( NULL == pHM || NULL == pclSheet )
	{
		return;
	}

	long lRow = *plRow;

	// Sort HM in function of position.
	CRank HMList;
	CString str;
	long lHMRows = lRow;

	for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext() )
	{
		CDS_HydroMod* pChildHM = (CDS_HydroMod *)IDPtr.MP;
		HMList.Add( str, pChildHM->GetPos(), (LPARAM)IDPtr.MP );
		lHMRows += 1;	// 1 row by circuit
	}

	// If we are on the root module we have to continue.
	if( ( 0 != pHM->GetLevel() ) && ( lHMRows == lRow ) )
	{
		return;
	}
	
	// Parent module at the first position.
	if( NULL == pHM->GetParent() )
	{
		HMList.Add( str, 0, (LPARAM)pHM->GetIDPtr().MP );
		lHMRows += 1;	// 1 row for parent module.
	}

	CDS_HydroMod *pChildHM = NULL;
	LPARAM lparam = (LPARAM)0;

	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		pChildHM = (CDS_HydroMod *)lparam;
		CDB_TAProduct *pclValve = NULL;

		if( NULL != (CDB_TAProduct *)( pChildHM->GetTADBValveIDPtr().MP ) )
		{
			pclValve = (CDB_TAProduct *)( pChildHM->GetTADBValveIDPtr().MP );
		}
		
		if( NULL == pclValve && edt_KvCv != pChildHM->GetVDescrType() )
		{
			// We look under HM children even if they don't have any valve
			if( pChildHM != pHM )
			{
				FillHMMeasurementRow( pChildHM, pclSheet, &lRow );
			}
			continue;
		}
		else
		{
			// We add new line for child line
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );

			FillOneHMMeasurementRow( pChildHM, pclValve, pclSheet, &lRow );
			
			// Add another line for secondary
			if( NULL != pChildHM->GetpSchcat() && true == pChildHM->GetpSchcat()->IsSecondarySideExist() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
				FillOneHMMeasurementRow( pChildHM, pclValve, pclSheet, &lRow, true );
			}
		}

		if( pChildHM != pHM )
		{
			FillHMMeasurementRow( pChildHM, pclSheet, &lRow );
		}
	}

	// Border.
	pclSheet->SetCellBorder( CD_Measurement_DateTime, RD_Header_Meas_FirstCirc, CD_Measurement_PowerDeviation, lRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Measurement_DateTime, RD_Header_Meas_FirstCirc + 1, CD_Measurement_PowerDeviation, lRow - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Measurement_DateTime, RD_Header_Meas_FirstCirc, CD_Measurement_PowerDeviation, lRow - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Measurement_DateTime, RD_Header_Meas_FirstCirc, CD_Measurement_PowerDeviation, lRow - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	*plRow = lRow;
}

void CContainerForExcelExport::FillOneHMMeasurementRow( CDS_HydroMod *pHM, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow, bool bIsSecondary )
{
	long lRow = *plRow;

	if( NULL == pHM || NULL == pclSheet )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Write the reference.
	pclSheet->SetStaticText( CD_Measurement_Reference, lRow, pHM->GetHMName() );

	// Write the description.
	pclSheet->SetStaticText( CD_Measurement_Description, lRow, pHM->GetDescription() );

	CDB_TAProduct *pclValve = pclProduct;

	if( false == bIsSecondary )
	{
		if( NULL == pHM->GetpBv() && NULL != pHM->GetpCV() )
		{
			if( NULL != (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP ) )
			{
				pclValve = (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP );
			}
		}
		else if( NULL != pHM->GetpBv() )
		{
			if( NULL != (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP ) )
			{
				pclValve = (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP );
			}
		}
	}
	else
	{
		// Secondary side.

		// Do not display CBI valve in case of secondary exist. Because if secondary size exists but there is no
		// balancing valve, we don't want to display the primary valve here.
		pclValve = NULL;

		if( NULL != pHM->GetpSecBv() )
		{
			if( NULL != (CDB_TAProduct *)( pHM->GetpSecBv()->GetIDPtr().MP ) )
			{
				pclValve = (CDB_TAProduct *)( pHM->GetpSecBv()->GetIDPtr().MP );
			}
		}
	}
	
	CString str = CteEMPTY_STRING;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	ASSERT( NULL != pUnitDB );


	bool bKvSignalExist = false;
	bool bPresettingExist = false;		// Settings computed by TASelect.
	bool bSettingExist = false;			// Settings set by user.

	// Write the valve.
	if( edt_TADBValve == pHM->GetVDescrType() )
	{
		str = _T("");

		if( NULL != pclValve )
		{
			str = pclValve->GetName();
		}
	}
	else if( edt_KvCv == pHM->GetVDescrType() && NULL != pUnitDB )
	{
		if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KV );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CV );
		}

		str += (CString)_T( " = " ) + WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, pHM->GetKvCv() ), 3, 0 );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GRAY );

	}
	else
	{
		str = pHM->GetCBIType() + (CString)_T(" ") + pHM->GetCBISize();
		str.TrimLeft();

		if( !str.IsEmpty() )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GRAY );
		}
	}

	pclSheet->SetStaticText( CD_Measurement_Valve, lRow, str );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	
	// Write the presetting (if a presetting exist in the CMeasData it will be overwritten).
	if( false == bIsSecondary )
	{
		if( pHM->GetPresetting() > 0.0 )
		{
			if( NULL != pclValve && NULL != pclValve->GetValveCharacteristic() )
			{
				str = pclValve->GetValveCharacteristic()->GetSettingString( pHM->GetPresetting(), true );
				pclSheet->SetStaticText( CD_Measurement_Presetting, lRow, str );
			}
		}

		if( NULL != pHM->GetpBv() && _T("") != pHM->GetpBv()->GetSettingStr() )
		{
			pclSheet->SetStaticText( CD_Measurement_Presetting, lRow, pHM->GetpBv()->GetSettingStr( true ) );
		}
		else if( NULL != pHM->GetpCV() && _T("") != pHM->GetpCV()->GetSettingStr() )
		{
			pclSheet->SetStaticText( CD_Measurement_Presetting, lRow, pHM->GetpCV()->GetSettingStr( true ) );
		}
	}
	else if( NULL != pHM->GetpSecBv() && _T("") != pHM->GetpSecBv()->GetSettingStr() )
	{
		pclSheet->SetStaticText( CD_Measurement_Presetting, lRow, pHM->GetpSecBv()->GetSettingStr( true ) );
	}

	// Write the Kv Signal (if a meas exist, re-enter the Kv signal value).
	if( NULL != pclValve && true == pclValve->IsKvSignalEquipped() && -1.0 != pclValve->GetKvSignal() )
	{
		pclSheet->SetStaticText( CD_Measurement_KvSignal, lRow, WriteCUDouble( _C_KVCVCOEFF, pclValve->GetKvSignal(), true ) );
		bKvSignalExist = true;
	}
	else if( NULL != pclValve )
	{
		bSettingExist = true;
	}

	// Write the design flow.
	CString strFlow = GetDashDotDash();

	if( false == bIsSecondary )
	{
		if( pHM->GetQDesign() > 0.0 )
		{
			strFlow = WriteCUDouble( _U_FLOW, pHM->GetQDesign(), true );
		}
	}
	else
	{
		if( NULL != pHM->GetpSecBv() && pHM->GetpSecBv()->GetQ() > 0.0 )
		{
			strFlow = WriteCUDouble( _U_FLOW, pHM->GetpSecBv()->GetQ(), true );
		}
		else if( ( NULL != pHM->GetpCircuitSecondaryPipe() ) && ( pHM->GetpCircuitSecondaryPipe()->GetTotalQ() > 0.0 ) )
		{
			strFlow = WriteCUDouble( _U_FLOW, pHM->GetpCircuitSecondaryPipe()->GetTotalQ(), true );
		}
	}

	pclSheet->SetStaticText( CD_Measurement_DesignFlow, lRow, strFlow );

	// Define the CMeasData to show (this will be the last that fit the boundaries).
	bool bAtLeastOneMeasFit = false;
	unsigned int iMeasDataToShow = 0;
	COleDateTime timLast;
	unsigned int uiNbrMeasData = pHM->GetMeasDataSize();

	for( unsigned int i = 0; i < uiNbrMeasData; i++ )
	{
		// Get the MeasData.
		CDS_HydroMod::CMeasData* pMeasData = pHM->GetpMeasData( i );

		if( NULL == pMeasData )
		{
			continue;
		}

		// HYS-1734: CDS_HydroModX::CMeasData::GetLocate renamed to GetPipeLocation.
		if( true == bIsSecondary && CDS_HydroModX::eLocate::InSecondary != pMeasData->GetPipeLocation() )
		{
			continue;
		}

		if( CDS_HydroModX::eLocate::undef == pMeasData->GetPipeLocation() )
		{
			pMeasData->SetLocate( CDS_HydroModX::InPrimary );
		}

		if( false == bIsSecondary
			&& ( CDS_HydroModX::eLocate::InPrimary != pMeasData->GetPipeLocation() /*&& CDS_HydroModX::eLocate::undef != pMeasData->GetPipeLocation() */ ) )
		{
			continue;
		}

		// In case we have measurements from the CBI, no date exist, it will always be shown.
		if( pMeasData->GetInstrument() == CDS_HydroMod/*::eInstrumentUsed*/::TACBI )
		{
			iMeasDataToShow = i;
			bAtLeastOneMeasFit = true;
			continue;
		}

		// Write DateTime.
		__time32_t time32 = pMeasData->GetMeasDateTime();
		tm dt;

		if( 0 != _gmtime32_s( &dt, &time32 ) )
		{
			ZeroMemory( &dt, sizeof( dt ) );
			dt.tm_year = 70;
			dt.tm_mday = 1;
		}

		COleDateTime tim( dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec );

		// In case the measure is outside the limits.
		if( tim < pMainFrame->GetLowerDateTime() || tim > pMainFrame->GetUpperDateTime() )
		{
			continue;
		}

		bAtLeastOneMeasFit = true;

		if( tim > timLast )
		{
			timLast = tim;
			iMeasDataToShow = i;
		}
	}

	// Show the last CMeasData.
	if( uiNbrMeasData > 0 && true == bAtLeastOneMeasFit )
	{
		// Get the MeasData.
		CDS_HydroMod::CMeasData* pMeasData = pHM->GetpMeasData( iMeasDataToShow );

		// Can show measurement information only if Date time is set OR if date time is NULL (it's the case for CBI) we must
		// be sure that data don't come from TAScope.
		if( NULL != pMeasData && 0 != pMeasData->GetMeasDateTime() || ( 0 == pMeasData->GetMeasDateTime()
			&& pMeasData->GetInstrument() != CDS_HydroMod::eInstrumentUsed::TASCOPE ) )
		{
			// Write DateTime.
			if( pMeasData->GetMeasDateTime() != 0 )
			{
				__time32_t t = pMeasData->GetMeasDateTime();
				str = _T("");

				if( t >= 0 )
				{
					CTimeUnic dtu( (__time64_t)t );
					str = dtu.FormatGmt( IDS_DATE_FORMAT ) + CString( _T(" ") ) + dtu.FormatGmt( IDS_TIME_FORMAT );
				}

				pclSheet->SetStaticText( CD_Measurement_DateTime, lRow, str );
			}

			// Write Water Char.
			if( CDS_HydroModX::eQMtype::QMTemp == pMeasData->GetQMType() || CDS_HydroModX::eQMtype::QMundef == pMeasData->GetQMType() )
			{
				pclSheet->SetStaticText( CD_Measurement_WaterChar, lRow, _T("") );
			}
			else
			{
				CWaterChar* pWC = pMeasData->GetpWC();
				ASSERT( NULL != pWC );

				if( NULL != pWC )
				{
					pclSheet->SetStaticText( CD_Measurement_WaterChar, lRow, WriteCUDouble( _U_TEMPERATURE, pWC->GetTemp(), true ) );
				}
			}

			// Write Valve.
			CDB_TAProduct* pTAP = pMeasData->GetpTAP();

			if( NULL != pTAP )
			{
				pclSheet->SetStaticText( CD_Measurement_Valve, lRow, pTAP->GetName() );
			}

			// Write Presetting.
			if( CDS_HydroMod::eInstrumentUsed::TACBI == pMeasData->GetInstrument() ) // If it is from a CBI
			{
				if( pHM->GetPresetting() > 0.0 )
				{
					if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
					{
						str = pTAP->GetValveCharacteristic()->GetSettingString( pHM->GetPresetting(), true );
						pclSheet->SetStaticText( CD_Measurement_Presetting, lRow, str );
					}
				}
			}
			else if( pMeasData->GetDesignOpening() > 0 )
			{
				if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
				{
					str = pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetDesignOpening(), true );
					pclSheet->SetStaticText( CD_Measurement_Presetting, lRow, str );
				}
			}

			// Write Setting.
			if( pMeasData->GetCurOpening() > 0.0 )
			{
				if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
				{
					str = pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetCurOpening(), true );
					pclSheet->SetStaticText( CD_Measurement_Setting, lRow, str );
				}
			}

			// Write Kv.
			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = _T( "Kv = " );
			}
			else
			{
				str = _T( "Cv = " );
			}
			pclSheet->SetStaticText( CD_Measurement_Kv, lRow, str + (CString)pMeasData->GetstrKv().c_str() );

			// Write Kv Signal.
			CDB_TAProduct* pTAProd = pMeasData->GetpTAP();

			if( NULL != pTAProd && pTAProd->IsKvSignalEquipped() && -1.0 != pTAProd->GetKvSignal() )
			{
				pclSheet->SetStaticText( CD_Measurement_KvSignal, lRow, WriteCUDouble( _C_KVCVCOEFF, pTAProd->GetKvSignal(), true ) );
			}

			// Write information depending on the QMtype.
			if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMPower ) )
			{
				// Write Meas. power.
				pclSheet->SetStaticText( CD_Measurement_Power, lRow, WriteCUDouble( _U_TH_POWER, pMeasData->GetPower(), true ) );
			}

			if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMPower ) ||
				true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMFlow ) )
			{
				// Write Meas. Dp.
				pclSheet->SetStaticText( CD_Measurement_MeasureDp, lRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetMeasDp(), true ) );

				// Write Design Dp.
				if( CDS_HydroMod::eInstrumentUsed::TACBI == pMeasData->GetInstrument() ) // If it is from a CBI
				{
					if( 0.0 != pHM->GetDpDesign() )
					{
						str = WriteCUDouble( _U_DIFFPRESS, pHM->GetDpDesign(), true );
						pclSheet->SetStaticText( CD_Measurement_DesignDp, lRow, str );
					}
				}
				else
				{
					double dDp = pMeasData->GetDesignDp();

					if( dDp > 0.0 )
					{
						pclSheet->SetStaticText( CD_Measurement_DesignDp, lRow, WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
					}
				}

				// Write Meas. Flow.
				if( pMeasData->GetMeasFlow() != 0.0 )
				{
					pclSheet->SetStaticText( CD_Measurement_MeasureFlow, lRow, WriteCUDouble( _U_FLOW, pMeasData->GetMeasFlow(), true ) );
				}

				// Write Flow Deviation.
				if( pMeasData->GetMeasFlow() != 0.0 && pMeasData->GetDesignQ() != 0.0 )
				{
					str = WriteDouble( pMeasData->GetFlowRatio() - 100, 3, 0 );

					// Special treatment for "0.0".
					if( 0 == IDcmp( str, L"-0.0" ) || 0 == IDcmp( str, L"0.0" ) )
					{
						str = L"0";
					}
					str += _T( " %" );
					pclSheet->SetStaticText( CD_Measurement_FlowDeviation, lRow, _T( "FD = " ) + str );
				}
			}

			for( int j = CDS_HydroModX::DTSFIRST; j < CDS_HydroModX::LastDTS; j++ )
			{
				double dT = pMeasData->GetT( (CDS_HydroModX::eDTS)j );

				if( dT > -273.15 )
				{
					str = WriteCUDouble( _U_TEMPERATURE, dT, true );
				}
				else
				{
					str = _T("");
				}

				long lColumnID = -1;

				switch( j )
				{
					case CDS_HydroModX::DTS2onDPS:
						lColumnID = CD_Measurement_Temp2DPS;
						break;

					case CDS_HydroModX::DTS1onDPS:
						lColumnID = CD_Measurement_Temp1DPS;
						break;

					case CDS_HydroModX::DTSonHH:
						lColumnID = CD_Measurement_TempHH;
						break;

					case CDS_HydroModX::DTSRef:
						lColumnID = CD_Measurement_TempRef;
						break;

					default:
						lColumnID = -1;
						break;
				}

				if( -1 != lColumnID )
				{
					pclSheet->SetStaticText( lColumnID, lRow, str );
				}
			}

			bool bDisplayDT = ( pMeasData->GetUsedDTSensor( 0 ) > CDS_HydroModX::DTSUndef && pMeasData->GetT( pMeasData->GetUsedDTSensor( 0 ) ) > -273.15 &&
				pMeasData->GetUsedDTSensor( 1 ) > CDS_HydroModX::DTSUndef && pMeasData->GetT( pMeasData->GetUsedDTSensor( 1 ) ) > -273.15 );

			if( true == bDisplayDT )
			{
				// Write Meas. DT.
				double dDT = abs( pMeasData->GetT( pMeasData->GetUsedDTSensor( 0 ) ) - pMeasData->GetT( pMeasData->GetUsedDTSensor( 1 ) ) );
				pclSheet->SetStaticText( CD_Measurement_DiffTemp, lRow, WriteCUDouble( _U_DIFFTEMP, dDT, true ) );
			}

			if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMDp ) )
			{
				// Write Meas. Dp.
				pclSheet->SetStaticText( CD_Measurement_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetMeasDp(), true ) );
			}

			if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMDpl ) )
			{
				// Write Meas. Dpl.
				pclSheet->SetStaticText( CD_Measurement_Dpl, lRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetMeasDpL(), true ) );
			}

			// Write Design Flow.
			pclSheet->SetStaticText( CD_Measurement_DesignFlow, lRow, WriteCUDouble( _U_FLOW, pMeasData->GetDesignQ(), true ) );
		}
	}

	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillHMTADiagnosticRow( CDS_HydroMod *pHM, CSSheet *pclSheet, long *plRow )
{
	if( NULL == pHM || NULL == pclSheet )
	{
		return;
	}

	long lRow = *plRow;

	// Sort HM in function of position.
	CRank HMList;
	CString str;
	long lHMRows = lRow;
	int iNbChildren = 0;

	for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext() )
	{
		CDS_HydroMod *pChildHM = (CDS_HydroMod *)IDPtr.MP;
		HMList.Add( str, pChildHM->GetPos(), (LPARAM)IDPtr.MP );
		lHMRows += 1;	// 1 row by circuit
		iNbChildren++;
	}

	// If we are on the root module we have to continue.
	if( 0 != pHM->GetLevel() && lHMRows == lRow )
	{
		return;
	}

	// Parent module at the first position.
	if( NULL == pHM->GetParent() )
	{
		HMList.Add( str, 0, (LPARAM)pHM->GetIDPtr().MP );
		lHMRows += 1;	// 1 row for parent module.
	}

	CDS_HydroMod *pChildHM = NULL;
	LPARAM lparam = (LPARAM)0;

	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		pChildHM = (CDS_HydroMod *)lparam;
		CDB_TAProduct *pclValve = NULL;

		if( NULL != (CDB_TAProduct *)( pChildHM->GetTADBValveIDPtr().MP ) )
		{
			pclValve = (CDB_TAProduct *)( pHM->GetTADBValveIDPtr().MP );
		}

		if( NULL == pclValve )
		{
			// We look under HM children even if they don't have any valve.
			if( pChildHM != pHM )
			{
				FillHMTADiagnosticRow( pChildHM, pclSheet, &lRow );
			}
			continue;
		}
		else
		{
			// We add new line for child line.
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );

			FillOneHMTADiagnosticRow( pChildHM, pclValve, pclSheet, iNbChildren, &lRow );
		}

		if( pChildHM != pHM )
		{
			FillHMTADiagnosticRow( pChildHM, pclSheet, &lRow );
		}
	}

	// Border.
	pclSheet->SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Diag_FirstCirc, CD_TADiagnostic_DistribDpDesign, lRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Diag_FirstCirc + 1, CD_TADiagnostic_DistribDpDesign, lRow - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Diag_FirstCirc, CD_TADiagnostic_DistribDpDesign, lRow - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Diag_FirstCirc, CD_TADiagnostic_DistribDpDesign, lRow - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	*plRow = lRow;
}

void CContainerForExcelExport::FillOneHMTADiagnosticRow( CDS_HydroMod *pHM, CDB_TAProduct *pclProduct, CSSheet *pclSheet, int iCountHM, long *plRow, 
		bool bIsSecondary )
{
	long lRow = *plRow;

	if( NULL == pHM || NULL == pclSheet || NULL == pclProduct )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Write the reference.
	pclSheet->SetStaticText( CD_TADiagnostic_Reference, lRow, pHM->GetHMName() );

	// Write the description.
	pclSheet->SetStaticText( CD_TADiagnostic_Description, lRow, pHM->GetDescription() );

	CWaterChar *pclWaterChar = NULL;
	CDB_TAProduct *pclValve = pclProduct;

	// Diagnostic can be done only on balancing device. This can be either a regulating valve or 
	// a control valve that can be preset and measured: BCV, PIBCV & DPCBCV.
	if( false == bIsSecondary )
	{
		// We are at the primary side. 
		if( NULL == pHM->GetpBv() && NULL != pHM->GetpCV() )
		{
			if( NULL != (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP ) )
			{
				pclValve = (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP );
				pclWaterChar = pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
			}
		}
		else if( NULL != pHM->GetpBv() )
		{
			if( NULL != (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP ) )
			{
				pclValve = (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP );
				pclWaterChar = pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eBVprim );
			}
		}
		else
		{
			// Smart control valve must be added when available in the TA-Scope.
			ASSERT_RETURN;
		}
	}
	else
	{
		// Secondary side.

		// Do not display CBI valve in case of secondary exist. Because if secondary size exists but there is no
		// balancing valve, we don't want to display the primary valve here.
		pclValve = NULL;

		if( NULL != pHM->GetpSecBv() )
		{
			if( NULL != (CDB_TAProduct *)( pHM->GetpSecBv()->GetIDPtr().MP ) )
			{
				pclValve = (CDB_TAProduct *)( pHM->GetpSecBv()->GetIDPtr().MP );
				pclWaterChar = pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eBVsec );
			}
		}
	}

	// Write water characteristics.
	ASSERT( NULL != pclWaterChar );

	if( NULL != pclWaterChar )
	{
		pclSheet->SetStaticText( CD_TADiagnostic_WaterChar, lRow, WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp(), true ) );
	}

	CString str = CteEMPTY_STRING;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	ASSERT( NULL != pUnitDB );

	// Write the valve.
	if( edt_TADBValve == pHM->GetVDescrType() )
	{
		str = _T("");

		if( NULL != pclValve )
		{
			str = pclValve->GetName();
		}
	}
	else if( edt_KvCv == pHM->GetVDescrType() && NULL != pUnitDB )
	{
		if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KV );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CV );
		}

		str += (CString)_T(" = ") + WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, pHM->GetKvCv() ), 3, 0 );

		if( false == str.IsEmpty() )
		{
			str += (CString)_T("*");
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GRAY );
		}
	}
	else
	{
		str = pHM->GetCBIType() + (CString)_T(" ") + pHM->GetCBISize();
		str.TrimLeft();

		if( !str.IsEmpty() )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GRAY );
		}
	}

	pclSheet->SetStaticText( CD_TADiagnostic_Valve, lRow, str );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Write the presetting (if a presetting exist in the CMeasData it will be overwritten).
	if( false == bIsSecondary )
	{
		if( pHM->GetPresetting() > 0.0 )
		{
			if( NULL != pclValve && NULL != pclValve->GetValveCharacteristic() )
			{
				str = pclValve->GetValveCharacteristic()->GetSettingString( pHM->GetPresetting(), true );
				pclSheet->SetStaticText( CD_TADiagnostic_Presetting, lRow, str );
			}
		}

		if( NULL != pHM->GetpBv() && _T("") != pHM->GetpBv()->GetSettingStr() )
		{
			pclSheet->SetStaticText( CD_TADiagnostic_Presetting, lRow, pHM->GetpBv()->GetSettingStr( true ) );
		}
		else if( NULL != pHM->GetpCV() && _T("") != pHM->GetpCV()->GetSettingStr() )
		{
			pclSheet->SetStaticText( CD_TADiagnostic_Presetting, lRow, pHM->GetpCV()->GetSettingStr( true ) );
		}
	}
	else if( NULL != pHM->GetpSecBv() && _T("") != pHM->GetpSecBv()->GetSettingStr() )
	{
		pclSheet->SetStaticText( CD_TADiagnostic_Presetting, lRow, pHM->GetpSecBv()->GetSettingStr( true ) );
	}

	// Write Kv (if a meas exist, re-enter the Kv value).
	CString strKv = CteEMPTY_STRING;
	
	if( NULL != pclValve && NULL != pclValve->GetValveCharacteristic() && 0.0 != pHM->GetPresetting() && -1.0 != pHM->GetPresetting() )
	{
		double dKv = pclValve->GetValveCharacteristic()->GetKv( pHM->GetPresetting() );
		strKv = _T("Kv = ");
		
		if( -1.0 != dKv )
		{
			if( 0 != pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				strKv = _T("Cv = ");
			}
			
			str = strKv + WriteCUDouble( _C_KVCVCOEFF, dKv );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_ERROR );
		}
		pclSheet->SetStaticText( CD_TADiagnostic_Kv, lRow, str );
	}

	// Write Kv Signal (if a meas exist, re-enter the Kv signal value).
	if( NULL != pclValve && true == pclValve->IsKvSignalEquipped() && -1.0 != pclValve->GetKvSignal() )
	{
		str = WriteCUDouble( _C_KVCVCOEFF, pclValve->GetKvSignal(), true );
		pclSheet->SetStaticText( CD_TADiagnostic_KvSignal, lRow, str );
	}

	// Define the CMeasData to show (this will be the last that fit the boundaries).
	bool bAtLeastOneMeasFit = false;
	unsigned int iMeasDataToShow = 0;
	COleDateTime timLast;
	unsigned int uiNbrMeasData = pHM->GetMeasDataSize();

	for( unsigned int i = 0; i < uiNbrMeasData; i++ )
	{
		// Get the MeasData.
		CDS_HydroMod::CMeasData *pMeasData = pHM->GetpMeasData( i );

		if( NULL == pMeasData )
		{
			continue;
		}

		// HYS-1734: CDS_HydroModX::CMeasData::GetLocate renamed to GetPipeLocation.
		if( true == bIsSecondary && CDS_HydroModX::eLocate::InSecondary != pMeasData->GetPipeLocation() )
		{
			continue;
		}

		if( false == bIsSecondary && ( CDS_HydroModX::eLocate::InPrimary != pMeasData->GetPipeLocation() ) )
		{
			continue;
		}

		// In case we have measurements from the CBI, no date exist, it will always be shown.
		if( pMeasData->GetInstrument() == CDS_HydroMod/*::eInstrumentUsed*/::TACBI )
		{
			iMeasDataToShow = i;
			bAtLeastOneMeasFit = true;
			continue;
		}

		// Write DateTime.
		__time32_t time32 = pMeasData->GetTABalDateTime();

		if( 0 >= time32 )
		{
			time32 = pMeasData->GetMeasDateTime();    // For backward compatibility
		}

		if( 0 >= time32 && true == pMeasData->GetFlagTaBalMeasCompleted() )
		{
			iMeasDataToShow = i;
			bAtLeastOneMeasFit = true;
			continue;
		}

		if( 0 >= time32 )
		{
			continue;
		}

		tm dt;
		_gmtime32_s( &dt, &time32 );
		COleDateTime tim( dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec );

		// In case the measure is outside the limits.
		if( tim < pMainFrame->GetLowerDateTime() || tim > pMainFrame->GetUpperDateTime() )
		{
			continue;
		}

		bAtLeastOneMeasFit = true;

		if( tim > timLast )
		{
			timLast = tim;
			iMeasDataToShow = i;
		}
	}

	// Show the last CMeasData.
	if( uiNbrMeasData > 0 && true == bAtLeastOneMeasFit )
	{
		// Get the MeasData.
		CDS_HydroMod::CMeasData* pMeasData = pHM->GetpMeasData( iMeasDataToShow );

		if( NULL != pMeasData )
		{
			__time32_t t = pMeasData->GetTABalDateTime();

			if( 0 == t )
			{
				t = pMeasData->GetMeasDateTime();    // For backward compatibility
			}

			// Write DateTime.
			if( t != 0 )
			{
				CTimeUnic dtu( (__time64_t)t );
				str = dtu.FormatGmt( IDS_DATE_FORMAT ) + CString( _T(" ") ) + dtu.FormatGmt( IDS_TIME_FORMAT );
				pclSheet->SetStaticText( CD_TADiagnostic_DateTime, lRow, str );
			}

			// Write Water Char.
			CWaterChar *pWC = pMeasData->GetpWC();
			ASSERT( NULL != pWC );

			if( NULL != pWC )
			{
				pclSheet->SetStaticText( CD_TADiagnostic_WaterChar, lRow, WriteCUDouble( _U_TEMPERATURE, pWC->GetTemp(), true ) );
			}

			if( edt_TADBValve == pHM->GetVDescrType() )
			{
				// Write valve.
				CDB_TAProduct *pTAP = pMeasData->GetpTAP();

				if( NULL != pTAP )
				{
					pclSheet->SetStaticText( CD_TADiagnostic_Valve, lRow, pTAP->GetName() );
				}

				// Write Presetting.
				if( CDS_HydroMod::eInstrumentUsed::TACBI == pMeasData->GetInstrument() ) // If it is from a CBI
				{
					if( pHM->GetPresetting() != 0.0 && pHM->GetPresetting() != -1.0 )
					{
						if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
						{
							str = pTAP->GetValveCharacteristic()->GetSettingString( pHM->GetPresetting(), true );
							pclSheet->SetStaticText( CD_TADiagnostic_Presetting, lRow, str );
						}
					}
				}
				else
				{
					if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
					{
						str = pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetDesignOpening(), true );
						pclSheet->SetStaticText( CD_TADiagnostic_Presetting, lRow, str );
					}
				}

				// Write computed setting.
				if( pMeasData->GetTaBalOpeningResult() > 0.0 )
				{
					if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
					{
						str = pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetTaBalOpeningResult(), true );
						pclSheet->SetStaticText( CD_TADiagnostic_ComputedSetting, lRow, str );
					}
				}
			}

			// Write Kv.
			pclSheet->SetStaticText( CD_TADiagnostic_Kv, lRow, strKv + pMeasData->GetstrKv().c_str() );

			// Write Kv Signal.
			CDB_TAProduct *pTAProd = pMeasData->GetpTAP();

			if( NULL != pTAProd && true == pTAProd->IsKvSignalEquipped() && -1.0 != pTAProd->GetKvSignal() )
			{
				pclSheet->SetStaticText( CD_TADiagnostic_KvSignal, lRow, WriteCUDouble( _C_KVCVCOEFF, pTAProd->GetKvSignal(), true ) );
			}

			// Write Design Flow.
			pclSheet->SetStaticText( CD_TADiagnostic_DesignFlow, lRow, WriteCUDouble( _U_FLOW, pMeasData->GetDesignQ(), true ) );

			if( pHM->GetLevel() != 0 )
			{
				// Write Dp1.
				pclSheet->SetStaticText( CD_TADiagnostic_Dp1, lRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetTaBalMeasDp_1(), true ) );

				// Write Setting 1.
				if( NULL != pTAProd && NULL != pTAProd->GetValveCharacteristic() )
				{
					str = pTAProd->GetValveCharacteristic()->GetSettingString( pMeasData->GetTaBalOpening_1(), true );
					pclSheet->SetStaticText( CD_TADiagnostic_Setting1, lRow, str );
				}
			}
			else
			{
				// Write Dp1.
				pclSheet->SetStaticText( CD_TADiagnostic_Dp1, lRow, GetDashDotDash() );

				// Write Setting 1.
				pclSheet->SetStaticText( CD_TADiagnostic_Setting1, lRow, GetDashDotDash() );
			}

			// Write Dp2.
			pclSheet->SetStaticText( CD_TADiagnostic_Dp2, lRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetTaBalMeasDp_2(), true ) );

			// Write Measuring flow.
			if( pMeasData->GetTaBalOpening_1() != 0 && pMeasData->GetTaBalMeasDp_1() != 0 )
			{
				double dFlow = pMeasData->GetFlow( pMeasData->GetTaBalOpening_1(), pMeasData->GetTaBalMeasDp_1() );

				pclSheet->SetStaticText( CD_TADiagnostic_MeasFlow, lRow, WriteCUDouble( _U_FLOW, dFlow, true ) );

				// Write flow deviation.
				if( dFlow != 0 && pMeasData->GetDesignQ() != 0.0 )
				{
					double dFlowRatio = ( dFlow / pMeasData->GetDesignQ() * 100 );
					str = WriteDouble( dFlowRatio - 100, 3, 0 );

					// Special treatment for "0.0".
					if( NULL == IDcmp( str, _T("-0.0") ) || NULL == IDcmp( str, _T("0.0") ) )
					{
						str = _T("0");
					}
					
					str += _T(" %");
					pclSheet->SetStaticText( CD_TADiagnostic_FlowDeviation, lRow, _T("FD = ") + str );
				}
			}

			// Write Circuit DP & Measured.
			// Remarks: do nothing with partner valve.

			if( pHM->GetLevel() != 0 )
			{
				// Measuring.
				CString cstrMeas = ( pMeasData->GetCircDpComputed() > 0 ) ? WriteCUDouble( _U_DIFFPRESS, pMeasData->GetCircDpComputed(), true ) : _T( "0.0" );
				cstrMeas += _T( " / " );
				
				// Design.
				CString cstrDesign;
				CHMXPipe *pPipeCirc = pMeasData->GetpCircuitPipe();
				CHMXTU *pTermUnit = pMeasData->GetpTerminalUnit();

				if( NULL != pPipeCirc && pPipeCirc->GetFlow() > 0 )
				{
					double dTotalDp = 0.0;

					if( NULL != pPipeCirc )
					{
						dTotalDp = pPipeCirc->GetTotalDp();
					}

					if( NULL != pTermUnit )
					{
						dTotalDp += pTermUnit->GetDp();
					}

					cstrDesign = WriteCUDouble( _U_DIFFPRESS, dTotalDp, true );
				}
				else
				{
					cstrDesign = GetDashDotDash();
				}

				pclSheet->SetStaticText( CD_TADiagnostic_CircuitDpMeas, lRow, cstrMeas + cstrDesign );
			}

			// Write distribution pressure drop computed & measured.
			if( pHM->GetLevel() != 0 )
			{
				if( 1 == pHM->GetPos() || iCountHM == pHM->GetPos() )
				{
					// Measuring.
					str = GetDashDotDash() + _T(" / ");

					// Design.
					CString cstrDesign;
					CHMXPipe *pPipeDistr = pMeasData->GetpDistributionPipe();

					if( NULL != pPipeDistr && pPipeDistr->GetFlow() != 0 )
					{
						cstrDesign = WriteCUDouble( _U_DIFFPRESS, pPipeDistr->GetTotalDp() );
					}
					else
					{
						cstrDesign = GetDashDotDash();
					}

					pclSheet->SetStaticText( CD_TADiagnostic_DistribDpMeas, lRow, str + cstrDesign );
				}
				else
				{
					CString cstrMeas = ( pMeasData->GetDistribDpComputed() > 0 ) ? WriteCUDouble( _U_DIFFPRESS, pMeasData->GetDistribDpComputed(), true ) : _T("0.0");

					// Design.
					CString cstrDesign;
					CHMXPipe *pPipeDistr = pMeasData->GetpDistributionPipe();

					if( NULL != pPipeDistr && pPipeDistr->GetFlow() != 0 )
					{
						cstrDesign = WriteCUDouble( _U_DIFFPRESS, pPipeDistr->GetTotalDp(), true );
					}
					else
					{
						cstrDesign = GetDashDotDash();
					}
					pclSheet->SetStaticText( CD_TADiagnostic_DistribDpMeas, lRow, cstrMeas + _T(" / ") + cstrDesign );
				}
			}
		}
	}

	pclSheet->AddCellSpanW( CD_TADiagnostic_CircuitDpMeas, lRow, CD_TADiagnostic_CircuitDpDesign - CD_TADiagnostic_CircuitDpMeas + 1, 1 );
	pclSheet->AddCellSpanW( CD_TADiagnostic_DistribDpMeas, lRow, CD_TADiagnostic_DistribDpDesign - CD_TADiagnostic_DistribDpMeas + 1, 1 );
	
	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::SetOrResizeColumnWidth( CSSheet *pclSheet, long lCol, long lRow, double dMaxWidth, double *pdWidth )
{
	if( ( NULL == pclSheet ) || ( lCol <= 0 ) || ( NULL == pdWidth ) )
	{
		return;
	}
	
	// HYS-987: The Flow column is too large because of the title in a merged cell.
	TCHAR tSheetName[30] = { 0 };
	pclSheet->GetSheetName( 1, tSheetName, 30 );
	
	if( ( 0 == wcscmp( tSheetName, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMEVALVE ) ) )
			&& ( ( CD_Prim_TechInfo_Flow == lCol ) || ( CD_Second_TechInfo_Flow == lCol ) ) )
	{
		long lcolLast = ( CD_Prim_TechInfo_Flow == lCol ) ? ( CD_Prim_TechInfo_DpCAuth - lCol + 1 ) : ( CD_Second_TechInfo_Setting - lCol + 1 );
		pclSheet->SetStaticText( lCol, RD_Header_HMSubGroupName, CteEMPTY_STRING );
		pclSheet->GetMaxTextColWidth( lCol, pdWidth );
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		pclSheet->AddCellSpanW( lCol, RD_Header_HMSubGroupName, lcolLast, 1 );
		pclSheet->SetStaticText( lCol, RD_Header_HMSubGroupName, IDS_SSHEET_TECHNICAL_INFORMATIONS );
	}
	// HYS-1605: To avoid that the first cell of merged cells takes the width of the merged column title
	else if( ( 0 == wcscmp( tSheetName, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMETADIAGNOSTIC ) ) )
		&& ( ( CD_TADiagnostic_Dp1 == lCol ) || ( CD_TADiagnostic_CircuitDpMeas == lCol ) ) )
	{
		long lcolLast = -1;
		if( CD_TADiagnostic_Dp1 == lCol )
		{
			lcolLast = CD_TADiagnostic_FlowDeviation - lCol + 1;
			pclSheet->SetStaticText( lCol, RD_Header_Diag_GroupName, CteEMPTY_STRING );
			pclSheet->GetMaxTextColWidth( lCol, pdWidth );
			pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
			pclSheet->AddCellSpanW( lCol, RD_Header_Diag_GroupName, lcolLast, 1 );
			pclSheet->SetStaticText( lCol, RD_Header_Diag_GroupName, IDS_SSHEET_DIAG_MEASDATA );
		}
		else
		{
			lcolLast = CD_TADiagnostic_DistribDpDesign - lCol + 1;
			pclSheet->SetStaticText( lCol, RD_Header_Diag_GroupName, CteEMPTY_STRING );
			pclSheet->GetMaxTextColWidth( lCol, pdWidth );
			pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
			pclSheet->AddCellSpanW( lCol, RD_Header_Diag_GroupName, lcolLast, 1 );
			pclSheet->SetStaticText( lCol, RD_Header_Diag_GroupName, IDS_SSHEET_DIAG_DPSHOOT );
		}
	}
	else
	{
		pclSheet->GetMaxTextColWidth( lCol, pdWidth );
	}
	
	if( *pdWidth > dMaxWidth )
	{
		pclSheet->SetColWidth( lCol, dMaxWidth );
	}
	else
	{
		pclSheet->SetColWidth( lCol, *pdWidth );
	}

	// Check now if text is not in two lines for lRow(As 'Cable length to remove temp. sensor').
	CString strText = pclSheet->GetCellText( lCol, lRow );
	
	if( false == strText.IsEmpty() )
	{
		// Retrieve the current cell height.
		double dCurrentHeight;
		pclSheet->GetRowHeight( lRow, &dCurrentHeight );
		double dCurrentWidth;
		pclSheet->GetColWidth( lRow, &dCurrentWidth );
		double dCurrentWidthUnits = pclSheet->ColWidthToLogUnits( dCurrentWidth );
		
		// Save the pattern text used for this cell.
		pclSheet->SaveTextPattern( lCol, lRow );
		
		// Compute the height needed to write all the text.
		double dWidth, dHeight;
		pclSheet->GetMaxTextCellSize( lCol, lRow, &dWidth, &dHeight );

		// If height needed if bigger than the current height.
		if( dHeight > dCurrentHeight )
		{
			// We increase the height.
			pclSheet->SetRowHeight( lRow, dHeight );
		}

		// Restore the text pattern for the cell.
		pclSheet->RestoreTextPattern( lCol, lRow );
	}
}

void CContainerForExcelExport::SetAllColumnWidth(CSSheet *pclSheet, long lColStart,  long lColEnd, long lRow, double dMaxWidth )
{
	if( NULL == pclSheet )
	{
		return;
	}

	//Resize Column Width
	// Run all columns
	for( long lLoopCol = lColStart; lLoopCol <= lColEnd; lLoopCol++ )
	{
		double dWidth = 0.0;
		SetOrResizeColumnWidth( pclSheet, lLoopCol, lRow, dMaxWidth, &dWidth );
	}
}

void CContainerForExcelExport::HideEmptyColumns( CSSheet *pclSheet, long lFirstRow, long lColStart, long  lColEnd )
{
	long lRowStart;
	
	if( NULL == pclSheet )
	{
		return;
	}

	lRowStart = lFirstRow;
	
	// HYS-987: to handle last visible col border.
	long lLastVisibleCol = -1;
	
	// Run all columns.
	for( long lLoopCol = lColStart; lLoopCol <= lColEnd; lLoopCol++ )
	{
		bool bIsColEmpty = true;

		// If current column is not already hidden...
		if( FALSE == pclSheet->IsColHidden( lLoopCol ) )
		{
			// Run all rows.
			for( long lLoopRow = lRowStart; lLoopRow <= pclSheet->GetMaxRows() && bIsColEmpty; lLoopRow++ )
			{
				bIsColEmpty = pclSheet->GetCellText( lLoopCol, lLoopRow ).IsEmpty();

				if( false == bIsColEmpty )
				{
					lLastVisibleCol = lLoopCol;
					// A element exist, no need to run all rows
					break;
				}
			}
			
			// Hide or not the current column.
			pclSheet->ShowCol( lLoopCol, ( true == bIsColEmpty ) ? FALSE : TRUE );
			
			TCHAR tSheetName[30] = { 0 };
			pclSheet->GetSheetName( 1, tSheetName, 30 );

			if( ( 0 == wcscmp( tSheetName, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMEPNEUMATEX ) ) )
				&& CD_PMWQ_LAST + 7 == lLoopCol )
			{
				pclSheet->SetCellBorder( lLastVisibleCol, 0, lLastVisibleCol, pclSheet->GetMaxRows(), true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );
			}
		}
	}
}

void CContainerForExcelExport::InitProjectSheet( CSSheet *pclSheet )
{
	if( NULL == pclSheet )
	{
		return;
	}

	m_selectionMode = CteEMPTY_STRING;

	pclSheet->SetMaxRows( RD_Header_FirstCirc - 1 );
	pclSheet->SetMaxCols( CD_Pointer - 1 );


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'References' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main title.
	// References.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Prim_Ref1, RD_Header_HMGroupName, 2, 3 );
	pclSheet->SetStaticText( CD_Prim_Ref1, RD_Header_HMGroupName, _T("#1") );

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Prim_Ref2, RD_Header_HMGroupName, 1, 3 );
	pclSheet->SetStaticText( CD_Prim_Ref2, RD_Header_HMGroupName, _T("#2") );

	// Main title.
	// Selection mode.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Mode, RD_Header_HMGroupName, 1, 3 );
	pclSheet->SetStaticText( CD_Mode, RD_Header_HMGroupName, IDS_SSHEET_EXPORT_MODE );
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Primary product' group.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Prim_Product_Name, RD_Header_HMGroupName, CD_Prim_Qty - CD_Prim_Product_Name + 1, 1 );
	pclSheet->SetStaticText( CD_Prim_Product_Name, RD_Header_HMGroupName, IDS_SSHEET_PRIMARY_PRODUCT );


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Product informations 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSheet->AddCellSpanW( CD_Prim_Product_Name, RD_Header_HMSubGroupName, CD_Prim_Product_TASmartOutputSignal - CD_Prim_Product_Name + 1, 1 );
	pclSheet->SetStaticText( CD_Prim_Product_Name, RD_Header_HMSubGroupName, IDS_SELP_PRODUCT );

	// Column name.
	// Name.
	//pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetStaticText( CD_Prim_Product_Name, RD_Header_ColName, IDS_SELP_NAME );

	// Body material CV.
	pclSheet->SetStaticText( CD_Prim_Product_BodyMaterial, RD_Header_ColName, IDS_SSHEETSSEL_MATERIAL );

	// Size for BV ShutoffValve DpC.
	pclSheet->SetStaticText( CD_Prim_Product_Size, RD_Header_ColName, IDS_SELP_SIZE );

	// Connection for CV BV ShutoffValve DpC.
	pclSheet->SetStaticText( CD_Prim_Product_Connection, RD_Header_ColName, IDS_SELP_CONNECTION );

	// Version for CV BV ShutoffValve DpC.
	pclSheet->SetStaticText( CD_Prim_Product_Version, RD_Header_ColName, IDS_SELP_VERSION );

	// DpL range for DpC.
	pclSheet->SetStaticText( CD_Prim_Product_DpCDpLr, RD_Header_ColName, IDS_DPLRANGE );

	// PN-TminTmax for CV BV SV DpC.
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, RD_Header_ColName, IDS_SELP_PNTMINTMAX );

	// HYS-1680
	// Location for smart control valve
	pclSheet->SetStaticText( CD_Prim_Product_Location, RD_Header_ColName, IDS_SELP_LOCATION );

	// Kvs for smart control valve.
	pclSheet->SetStaticText( CD_Prim_Product_Kvs, RD_Header_ColName, IDS_SELP_KVS );

	// Nominal flow for smart control valve.
	pclSheet->SetStaticText( CD_Prim_Product_Qnom, RD_Header_ColName, IDS_SELP_QNOM );

	// Cable length to remote temp. sensor for smart control valve.
	// We force the width to this specific value (Will be split in two lines when calling the 'SetAllColumnWidth' method).
	pclSheet->SetColWidth( CD_Prim_Product_CableLengthToRemoteTempSensor, 15.0 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSheet->SetStaticText( CD_Prim_Product_CableLengthToRemoteTempSensor, RD_Header_ColName, IDS_SELP_CABLELENGTHTOREMOTETEMPSENSOR );
	
	// Power supply
	pclSheet->SetStaticText( CD_Prim_Product_TASmartPowerSupply, RD_Header_ColName, IDS_SSHEETSELPROD_POWSUPPL );
	// Input signal
	pclSheet->SetStaticText( CD_Prim_Product_TASmartInputSignal, RD_Header_ColName, IDS_SSHEETSELPROD_INPUT );
	// Output signal
	pclSheet->SetStaticText( CD_Prim_Product_TASmartOutputSignal, RD_Header_ColName, IDS_SSHEETSELPROD_OUTPUTSIG );
	// PN-TminTmax for CV BV SV DpC.

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Technical informations 
	////////////////////////////////////////////////////////////////////////////////////////////////////	

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSheet->AddCellSpanW( CD_Prim_TechInfo_Flow, RD_Header_HMSubGroupName, CD_Prim_TechInfo_DpCAuth - CD_Prim_TechInfo_Flow + 1, 1 );
	pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, RD_Header_HMSubGroupName, IDS_SSHEET_TECHNICAL_INFORMATIONS );

	// Column name.

	// Flow for CV BV ShutoffValve DpC.
	pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, RD_Header_ColName, IDS_SELP_FLOWRATE );

	// Power/DT for CV BV ShutoffValve DpC.
	CString strPowerDTName = TASApp.LoadLocalizedString( IDS_SELP_POWER );
	strPowerDTName += _T("/") + TASApp.LoadLocalizedString( IDS_DT );
	pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, RD_Header_ColName, strPowerDTName );

	// Dp for CV BV ShutoffValve.
	pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, RD_Header_ColName, IDS_SSHEETSSEL_DP );

	// Control mode for smart control valve.
	pclSheet->SetStaticText( CD_Prim_TechInfo_ControlMode, RD_Header_ColName, IDS_SSHEETSSEL_CONTROLMODE );

	// DpL for DpC.
	pclSheet->SetStaticText( CD_Prim_TechInfo_DpCDpL, RD_Header_ColName, IDS_DPL );

	// Dpmin for DpC or smart control valve.
	pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, RD_Header_ColName, IDS_SSHEETSSEL_DPMIN );

	// Dpmax for smart control valve and smart differential pressure controller (Can be also added for some others).
	pclSheet->SetStaticText( CD_Prim_TechInfo_DpMax, RD_Header_ColName, IDS_SELPRODDOCKPANE_DPMAX );

	// Setting for CV BV DpC.
	pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, RD_Header_ColName, IDS_SHEETHDR_SETTING );

	// Hmin for DpC.
	pclSheet->SetStaticText( CD_Prim_TechInfo_DpCHmin, RD_Header_ColName, IDS_SELP_HMIN );

	// Authority for DpC.
	pclSheet->SetStaticText( CD_Prim_TechInfo_DpCAuth, RD_Header_ColName, IDS_AUTHOR );

	// Article number for product.
	pclSheet->AddCellSpanW( CD_Prim_ArticleNumber, RD_Header_HMSubGroupName, 1, 2 );
	pclSheet->SetStaticText( CD_Prim_ArticleNumber, RD_Header_HMSubGroupName, IDS_SELPHDR_ART );

	// Local article number for product.
	pclSheet->AddCellSpanW( CD_Prim_LocalArticleNumber, RD_Header_HMSubGroupName, 1, 2 );
	CString str = CteEMPTY_STRING;

	if( false == TASApp.GetLocalArtNumberName().IsEmpty() )
	{
		str = TASApp.GetLocalArtNumberName();
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_SELPHDR_LOCART );
	}
	
	pclSheet->SetStaticText( CD_Prim_LocalArticleNumber, RD_Header_HMSubGroupName, str );

	// Quantity.
	pclSheet->AddCellSpanW( CD_Prim_Qty, RD_Header_HMSubGroupName, 1, 2 );
	pclSheet->SetStaticText( CD_Prim_Qty, RD_Header_HMSubGroupName, IDS_SELPHDR_QTY );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Secondary' group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_Second_Product_Name, RD_Header_HMGroupName, CD_Second_Qty - CD_Second_Product_Name + 1, 1 );
	pclSheet->SetStaticText( CD_Second_Product_Name, RD_Header_HMGroupName, IDS_SSHEET_SECONDARY_PRODUCT );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Product informations 
	////////////////////////////////////////////////////////////////////////////////////////////////////

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSheet->AddCellSpanW( CD_Second_Product_Name, RD_Header_HMSubGroupName, CD_Second_Product_PNTminTmax - CD_Second_Product_Name + 1, 1 );
	pclSheet->SetStaticText( CD_Second_Product_Name, RD_Header_HMSubGroupName, IDS_SELP_PRODUCT );

	// Column name.
	// Name for secondary product.
	pclSheet->SetStaticText( CD_Second_Product_Name, RD_Header_ColName, IDS_SELP_NAME );

	pclSheet->AddCellSpanW( CD_Second_Product_ActCloseOffDp, RD_Header_ColName, CD_Second_product_ActMaxTemp - CD_Second_Product_ActCloseOffDp + 1, 1 );
	pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, RD_Header_ColName, IDS_SSHEET_ACTINFORMATIONS );

	// Size for BV.
	pclSheet->SetStaticText( CD_Second_Product_Size, RD_Header_ColName, IDS_SELP_SIZE );

	// Connection for BV.
	pclSheet->SetStaticText( CD_Second_Product_Connection, RD_Header_ColName, IDS_SELP_CONNECTION );

	// Version for BV.
	pclSheet->SetStaticText( CD_Second_Product_Version, RD_Header_ColName, IDS_SELP_VERSION );

	// PN-TminTmax for BV.
	pclSheet->SetStaticText( CD_Second_Product_PNTminTmax, RD_Header_ColName, IDS_SELP_PNTMINTMAX );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Technical informations 
	////////////////////////////////////////////////////////////////////////////////////////////////////	

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSheet->AddCellSpanW( CD_Second_TechInfo_Flow, RD_Header_HMSubGroupName, CD_Second_TechInfo_Setting - CD_Second_TechInfo_Flow + 1, 1 );
	pclSheet->SetStaticText( CD_Second_TechInfo_Flow, RD_Header_HMSubGroupName, IDS_SSHEET_TECHNICAL_INFORMATIONS );

	// Column name.

	// Flow for BV.
	pclSheet->SetStaticText( CD_Second_TechInfo_Flow, RD_Header_ColName, IDS_SELP_FLOWRATE );

	// Power/DT for BV.
	strPowerDTName = TASApp.LoadLocalizedString( IDS_SELP_POWER );
	strPowerDTName += _T("/") + TASApp.LoadLocalizedString( IDS_DT );
	pclSheet->SetStaticText( CD_Second_TechInfo_PowerDt, RD_Header_ColName, strPowerDTName );

	// Dp for BV.
	pclSheet->SetStaticText( CD_Second_TechInfo_Dp, RD_Header_ColName, IDS_SSHEETSSEL_DP );

	// Setting for BV.
	pclSheet->SetStaticText( CD_Second_TechInfo_Setting, RD_Header_ColName, IDS_SHEETHDR_SETTING );

	// Article number for secondary.
	pclSheet->AddCellSpanW( CD_Second_ArticleNumber, RD_Header_HMSubGroupName, 1, 2 );
	pclSheet->SetStaticText( CD_Second_ArticleNumber, RD_Header_HMSubGroupName, IDS_SELPHDR_ART );

	// Local article number for product.
	pclSheet->AddCellSpanW( CD_Second_LocalArticleNumber, RD_Header_HMSubGroupName, 1, 2 );

	str = CteEMPTY_STRING;

	if( false == TASApp.GetLocalArtNumberName().IsEmpty() )
	{
		str = TASApp.GetLocalArtNumberName();
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_SELPHDR_LOCART );
	}
	
	pclSheet->SetStaticText( CD_Second_LocalArticleNumber, RD_Header_HMSubGroupName, str );

	// Quantity.
	pclSheet->AddCellSpanW( CD_Second_Qty, RD_Header_HMSubGroupName, 1, 2 );
	pclSheet->SetStaticText( CD_Second_Qty, RD_Header_HMSubGroupName, IDS_SELPHDR_QTY );

	// Border.
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_HMGroupName - 1, CD_Mode-1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_HMGroupName - 1, CD_Mode-1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_HMGroupName - 1, CD_Mode, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_HMGroupName - 1, CD_Mode - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );
}

bool CContainerForExcelExport::InitProjectResultExport( CSSheet *pclSheet )
{
	if( NULL == pclSheet )
	{
		return false;
	}
	
	pclSheet->SetBool( SSB_REDRAW, FALSE );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSheet->FormatStaticText( -1, -1, -1, -1, CteEMPTY_STRING );

	// Increase ROW height.
	double RowHeight = 12.75;
	pclSheet->SetRowHeight( RD_Header_HMGroupName, RowHeight * 1.5 );
	pclSheet->SetRowHeight( RD_Header_HMSubGroupName, RowHeight * 1.2 );
	pclSheet->SetRowHeight( RD_Header_ColName, RowHeight * 10 );

	// Resize column.
	SetAllColumnWidth( pclSheet, CD_Prim_Ref1, CD_Mode, RD_Header_ColName, 30 );

	// Initialize sheet.
	InitProjectSheet( pclSheet );

	// Get HM table.
	CTable *pHMTab = TASApp.GetpTADS()->GetpHydroModTable();

	// Something for individual or direct selection ?
	CTable *pclCtrlTable = (CTable *)( TASApp.GetpTADS()->Get( _T("CTRLVALVE_TAB") ).MP );
	CTable *pclDpCTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DPCONTR_TAB") ).MP );
	CTable *pclBvTable = (CTable *)( TASApp.GetpTADS()->Get( _T("REGVALV_TAB") ).MP );
	CTable *pclSvTable = (CTable *)( TASApp.GetpTADS()->Get( _T("SHUTOFF_TAB") ).MP );
	CTable *pclDpCBCvTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DPCBCVALVE_TAB") ).MP );
	CTable *pclTrvTable = (CTable *)( TASApp.GetpTADS()->Get( _T("RADSET_TAB") ).MP );
	CTable *pcl6WayValveTable = (CTable *)( TASApp.GetpTADS()->Get( _T("6WAYCTRLVALV_TAB") ).MP );
	CTable *pclSmartControlValveTable = (CTable *)( TASApp.GetpTADS()->Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
	CTable *pclSmartDpCTable = (CTable *)( TASApp.GetpTADS()->Get( _T("SMARTDPC_TAB") ).MP );

	// HYS-1070: This code is optimized and the correct number of rows is also used instead of more of this.
	//Direct selection
	///////////////////

	// CV.
	CTable *pclTable = ( _T('\0') != *pclCtrlTable->GetFirst().ID ) ? pclCtrlTable : NULL;
	bool bCvExist = ( NULL != pclTable );
	int iCount = 0;
	long lRow = RD_Header_FirstCirc;

	if( true == bCvExist )
	{
		iCount = pclTable->GetItemCount();
		bool bFound = false;

		for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDS_SSel *pclSSel = dynamic_cast<CDS_SSel *>( IDPtr.MP );

			if( NULL == pclSSel )
			{
				CDS_Actuator *pclSSel = dynamic_cast<CDS_Actuator *>( IDPtr.MP );
				
				if( NULL != pclSSel )
				{
					if( false == bFound )
					{
						pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
						bFound = true;
					}
					
					FillSelectedActrRow( pclSSel, pclSheet, &lRow );
				}
			}
			else
			{
				if( true == pclSSel->IsFromDirSel() )
				{
					if( false == bFound )
					{
						pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
						bFound = true;
					}
					
					FillSelectionRow( pclSSel, pclSheet, ValveType::CV, &lRow );
				}
				else if( true == bFound )
				{
					// HYS-987: if we are not in DirSel do not the row in 'count' elements
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() - 1 );
				}
			}

			if( false == bFound )
			{
				iCount--;
			}
		}
	}

	// DpCBCv.
	FillProductSelection( pclDpCBCvTable, pclSheet, true, ValveType::DpCBCV, &lRow );

	// DpC.
	FillProductSelection( pclDpCTable, pclSheet, true, ValveType::DpC, &lRow );
	
	// Bv.
	FillProductSelection( pclBvTable, pclSheet, true, ValveType::BV, &lRow );
	
	// Sv.
	FillProductSelection( pclSvTable, pclSheet, true, ValveType::SV, &lRow );

	// Trv.
	FillProductSelection( pclTrvTable, pclSheet, true, ValveType::Trv, &lRow );
	
	// 6-way valve.
	FillProductSelection( pcl6WayValveTable, pclSheet, true, ValveType::TA6WayV, &lRow );

	// Smart control valve.
	FillProductSelection( pclSmartControlValveTable, pclSheet, true, ValveType::SmartControlValve, &lRow );

	// Smart differential pressure controller.
	FillProductSelection( pclSmartDpCTable, pclSheet, true, ValveType::SmartDpC, &lRow );

	//Individual selection
	///////////////////////

	// Cv.
	FillProductSelection( pclCtrlTable, pclSheet, false, ValveType::CV, &lRow );

	// DpCBcv.
	FillProductSelection( pclDpCBCvTable, pclSheet, false, ValveType::DpCBCV, &lRow );

	// DpC.
	FillProductSelection( pclDpCTable, pclSheet, false, ValveType::DpC, &lRow );

	// Bv.
	FillProductSelection( pclBvTable, pclSheet, false, ValveType::BV, &lRow );

	// Sv.
	FillProductSelection( pclSvTable, pclSheet, false, ValveType::SV, &lRow );

	// Trv.
	FillProductSelection( pclTrvTable, pclSheet, false, ValveType::Trv, &lRow );

	// 6-way valve.
	FillProductSelection( pcl6WayValveTable, pclSheet, false, ValveType::TA6WayV, &lRow );

	// Smart control valve.
	FillProductSelection( pclSmartControlValveTable, pclSheet, false, ValveType::SmartControlValve, &lRow );

	// Smart differential pressure controller.
	FillProductSelection( pclSmartDpCTable, pclSheet, false, ValveType::SmartDpC, &lRow );

	// HYS-2007: Dp Sensor selected alone.
	FillProductSelection( pclDpCTable, pclSheet, false, ValveType::DpC, &lRow );

	//HM Calc
	//////////////

	for( IDPTR IDPtr = pHMTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHMTab->GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );

		if( NULL != pHM )
		{
			FillHMRow( pHM, pclSheet, &lRow );
		}
	}

	// Resize all column width.
	SetAllColumnWidth( pclSheet, CD_Prim_Ref1, CD_Mode, RD_Header_ColName, 30 );
	HideEmptyColumns( pclSheet, RD_Header_FirstCirc );

	pclSheet->SetBool( SSB_REDRAW, TRUE );

	if( lRow <= RD_Header_FirstCirc )
	{
		return false;
	}
	
	return true;
}

void CContainerForExcelExport::FillProductSelection( CTable *pclProdTable, CSSheet *pclSheet, bool isFromDirSel, ValveType etype, long *plRow )
{
	if( NULL == pclProdTable )
	{
		return;
	}

	long lRow = *plRow;
	CTable	*pclTable = ( _T('\0') != *pclProdTable->GetFirst().ID ) ? pclProdTable : NULL;
	bool bProdExist = ( NULL != pclTable );
	
	if( true == bProdExist )
	{
		int iCount = pclTable->GetItemCount();
		bool bFound = false;

		for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDS_SSel *pclSSel = dynamic_cast<CDS_SSel *>( IDPtr.MP );

			if( NULL != pclSSel && isFromDirSel == pclSSel->IsFromDirSel() )
			{
				if( false == bFound )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					bFound = true;
				}
				
				FillSelectionRow( pclSSel, pclSheet, etype, &lRow );
			}
			else if( false == bFound )
			{
				// HYS-987: if we are not in DirSel do not the row in 'count' elements
				iCount--;
			}
			else
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() - 1 );
			}
		}
	}
	
	*plRow = lRow;
}

void CContainerForExcelExport::FillOneHMRowForBv( CDS_HydroMod *pHM, CDS_HydroMod::CBV *pclHMBase, CSSheet *pclSheet, CDB_RegulatingValve *pclBv, long lRow )
{
	if( ( NULL == pHM ) || ( NULL == pclHMBase ) || ( NULL == pclSheet ) || ( NULL == pclBv ) )
	{
		return;
	}

	CWaterChar *pclWaterChar = pHM->GetpWaterChar( pclHMBase->GetHMObjectType() );

	if( NULL == pclWaterChar )
	{
		ASSERT_RETURN;
	}
	
	// Fill Bv row.
	pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclBv->GetName() );
	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclBv->GetSize() );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclBv->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclBv->GetVersion() );
	
	CString str = pclBv->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclBv->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

	pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pclHMBase->GetQ(), true ) );
	double dDp = pclHMBase->GetDp();
	
	if( true == pclBv->IsKvSignalEquipped() )
	{
		CDB_FixOCharacteristic *pChar = dynamic_cast<CDB_FixOCharacteristic *>( pclBv->GetValveCharDataPointer() );
		
		if( NULL != pChar && -1.0 != pChar->GetKvSignal() )
		{
			double dDpSignal = CalcDp( pclHMBase->GetQ(), pChar->GetKvSignal(), pclWaterChar->GetDens() );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSELP_DPSIGNAL );
			str += _T("=") + CString( WriteCUDouble( _U_DIFFPRESS, dDpSignal, true ) );
			dDp = CalcDp( pclHMBase->GetQ(), pChar->GetKv(), pclWaterChar->GetDens() );
		}
	}

	if( dDp > 0.0 )
	{
		str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
	}
	else	//dDp == 0.0
	{
		// Compute Dp based on opening and flow.
		if( NULL != pclBv->GetValveCharacteristic() )
		{
			if( true == pclBv->GetValveCharacteristic()->GetValveDp( pclHMBase->GetQ(), &dDp, pclHMBase->GetSetting(), pclWaterChar->GetDens(), pclWaterChar->GetKinVisc() ) )
			{
				str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
			}
		}
	}

	pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, str );
	
	double dPresset = pclHMBase->GetSetting();

	if( dPresset > 0.0 && NULL != pclBv->GetValveCharacteristic() )
	{
		pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, pclBv->GetValveCharacteristic()->GetSettingString( dPresset, true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, _T("-") );
	}

	FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclBv, pclBv->GetArtNum() );
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, _T("1") );
}

void CContainerForExcelExport::FillOneHMRow( CDS_HydroMod *pHM, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow, CString strBvPosition )
{
	long lRow = *plRow;
	CDB_RegulatingValve *pclBv = NULL;
	CDB_ControlValve *pclCv = NULL;
	CDB_DpController *pclDpc = NULL;
	CDB_ShutoffValve *pclShutoffValve = NULL;
	CDB_SmartControlValve *pclSmartControlValve = NULL;
	CDB_SmartControlValve *pclSmartDpC = NULL;

	if( ( NULL == pHM ) || ( NULL == pclProduct ) || ( NULL == pclSheet ) )
	{
		return;
	}

	if( ( NULL != pHM->GetpCV() ) && ( NULL != dynamic_cast<CDB_ControlValve*>( pclProduct ) ) )
	{
		pclCv = dynamic_cast<CDB_ControlValve*>( pclProduct );
	}
	else if( ( NULL != pHM->GetpDpC() ) && ( NULL != dynamic_cast<CDB_DpController*>( pclProduct ) ) )
	{
		pclDpc = dynamic_cast<CDB_DpController*>( pclProduct );
		
		// DpC wtih Bv in the same HM
		if ( NULL != pHM->GetpBv() )
		{
			pclBv = dynamic_cast<CDB_RegulatingValve*>( pHM->GetpBv()->GetIDPtr().MP );
		}
	}
	else if( NULL != dynamic_cast<CDB_SmartControlValve*>( pclProduct ) && NULL != pHM->GetpSmartControlValve() )
	{
		// HYS-1680
		pclSmartControlValve = dynamic_cast<CDB_SmartControlValve*>( pclProduct );
	}
	else if( NULL != dynamic_cast<CDB_SmartControlValve*>( pclProduct ) && NULL != pHM->GetpSmartDpC() )
	{
		// HYS-2007
		pclSmartDpC = dynamic_cast<CDB_SmartControlValve*>( pclProduct );
	}
	else if( ( NULL != pHM->GetpBv() || NULL != pHM->GetpBypBv() || NULL != pHM->GetpSecBv() ) && ( NULL != dynamic_cast<CDB_RegulatingValve*>( pclProduct ) ) )
	{
		pclBv = dynamic_cast<CDB_RegulatingValve*>( pclProduct );
	}
	else if( NULL != dynamic_cast<CDB_ShutoffValve*>( pclProduct ) ) 
	{
		pclShutoffValve = dynamic_cast<CDB_ShutoffValve*>( pclProduct );
	}

	if( ( NULL == pclCv ) && ( NULL == pclDpc ) && ( NULL == pclBv ) && ( NULL == pclShutoffValve )  && ( NULL == pclSmartControlValve )  && ( NULL == pclSmartDpC) )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Selection mode.
	CString str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{	
			// Insert a line separator.
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}

		m_selectionMode = str;
	}
	
	pclSheet->SetStaticText( CD_Mode, lRow, str );

	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pHM->GetHMName() );
	pclSheet->SetStaticText( CD_Prim_Infos, lRow, strBvPosition );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pHM->GetDescription() );
	str = CteEMPTY_STRING;

	// Fill CV product row.
	if( NULL != pclCv )
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclCv->GetName() );
		CDB_DpCBCValve *pclDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( pHM->GetpCV()->GetCvIDPtr().MP );

		if( NULL != pclDpCBCValve )
		{
			pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
		}
		else
		{
			pclSheet->SetStaticText( CD_Prim_Product_BodyMaterial, lRow, pclCv->GetBodyMaterial() );

			if( NULL == pclCv->GetValveCharDataPointer() && -1.0 != pclCv->GetKvs() )
			{
				// BVC or pure CV.
				CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

				if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				{
					str = TASApp.LoadLocalizedString( IDS_KVS );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_CV );
				}

				str += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pclCv->GetKvs() );
			}

			pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, str );
		}

		pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclCv->GetConnect() );
		pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclCv->GetVersion() );
		
		CString str = pclCv->GetPN().c_str();
		str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
		str = str + pclCv->GetTempRange( true );
		pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pHM->GetpCV()->GetQ(), true ) );
		str = CteEMPTY_STRING;
		double dDp = pHM->GetpCV()->GetDp();

		if( 0.0 != dDp )
		{
			if( true == pclCv->IsaPICV() )
			{
				// HYS-1380: Show Dp min. in Dp min. column
				pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
			}
			else
			{
				pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
			}
		}
		else
		{
			pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, str );
		}

		str = _T("-");
		
		// HYS-1871: Display setting for DpCBCV.
		if( true == pclCv->IsaBCV() || true == pclCv->IsaPICV() || true == pclCv->IsaDPCBCV() )
		{
			str = pHM->GetpCV()->GetSettingStr( true );

			if( true == str.IsEmpty() )
			{
				str = _T("-");
			}
		}
		else if( pclCv->IsaTrv() )
		{
			double dSet = pHM->GetpCV()->GetSetting();

			if( dSet != 0.0 )
			{
				CDB_ValveCharacteristic* pChar = ( (CDB_ThermostaticValve *)pclProduct )->GetThermoCharacteristic();
				CString str2 = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );
				str = str2 + (CString)_T(" = ");

				if( ( (CString)pclProduct->GetFamilyID() ) == _T("FAM_TRV2") && dSet < 1.0 )
				{
					str += _T("<1**");
				}
				else if( NULL != pChar )
				{
					str += pChar->GetSettingString( dSet );
				}
				else
				{
					str += _T("-");
				}
			}
		}
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str );
		
		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclCv, pclCv->GetArtNum() );
		pclSheet->SetStaticText( CD_Prim_Qty, lRow, _T("1") );
		
		// Secondary.
		CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( pHM->GetpCV()->GetActrIDPtr().MP );

		if( NULL != pclActuator )
		{
			pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pclActuator->GetName() );
			CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( ( (CDB_ControlValve *)pclProduct )->GetCloseOffCharIDPtr().MP );

			if( NULL != pCloseOffChar )
			{
				str = CteEMPTY_STRING;

				if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CLOSEOFFDP );
					double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );

					if( dCloseOffDp > 0.0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
					}
				}
				else if( CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXINLETPRESSURE );
					double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclActuator->GetMaxForceTorque() );

					if( dMaxInletPressure > 0.0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
					}
				}

				pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, lRow, str );
			}

			// Compute Actuating time in sec.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclActuator->GetActuatingTimesStr( ( (CDB_ControlValve *)pclProduct )->GetStroke(), true );
			pclSheet->SetStaticText( CD_Second_Product_ActActTime, lRow, str );

			// Write IP.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
			str += CString( _T(" : ") ) + pclActuator->GetIPxxAuto();

			if( false == pclActuator->GetIPxxManual().IsEmpty() )
			{
				str += _T("(" ) + pclActuator->GetIPxxManual() + _T( ")");
			}

			pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );
			pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, pclActuator->GetPowerSupplyStr() );
			pclSheet->SetStaticText( CD_Second_Product_ActInputSignal, lRow, pclActuator->GetInOutSignalsStr( true ) );
			pclSheet->SetStaticText( CD_Second_Product_ActOutputSignal, lRow, pclActuator->GetInOutSignalsStr( false ) );
			pclSheet->SetStaticText( CD_Second_Product_ActRelayType, lRow, pclActuator->GetRelayStr() );
			str.Empty();
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

			if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
			{
				str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
			}
			else if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
			{
				str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
			}
			else
			{
				str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_NO );
			}
			
			pclSheet->SetStaticText( CD_Second_Product_ActFailSafe, lRow, str );
			
			CString strPositionTitle = ( (int)pclActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
			str = strPositionTitle + CString( _T(" : ") ) + pclActuator->GetDefaultReturnPosStr( pclActuator->GetDefaultReturnPos() ).c_str();
			pclSheet->SetStaticText( CD_Second_Product_ActDRP, lRow, str );
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

			if( DBL_MAX == pclActuator->GetTmax() )
			{
				str += CString( _T(" :    -") );
			}
			else
			{
				str += CString( _T(" : ") ) + WriteDouble( pclActuator->GetTmax(), 3 );
				str += TASApp.LoadLocalizedString( IDS_UNITDEGREECELSIUS );
			}

			pclSheet->SetStaticText( CD_Second_product_ActMaxTemp, lRow, str );
			FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclActuator, pclActuator->GetArtNum() );
			pclSheet->SetStaticText( CD_Second_Qty, lRow, _T("1") );
		}
	}
	else if( NULL != pclDpc )
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclDpc->GetName() );
		pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclDpc->GetSize() );
		pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclDpc->GetConnect() );
		pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclDpc->GetVersion() );
		pclSheet->SetStaticText( CD_Prim_Product_DpCDpLr, lRow, pclDpc->GetFormatedDplRange( true ).c_str() );
		str = pclDpc->GetPN().c_str();
		str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
		str = str + pclDpc->GetTempRange( true );
		pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pHM->GetQDesign(), true ) );
		str = CteEMPTY_STRING;

		double dDpL = pHM->GetpDpC()->GetDpL();

		// Show Dpl only if different from -1.
		CString str2 = TASApp.LoadLocalizedString( IDS_DPL );
		CString str1 = CteEMPTY_STRING;

		if( dDpL <= 0 )
		{
			CString str3;
			str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
			str3.MakeLower();
			str1 = str2 + (CString)_T(" ") + str3;
		}
		else
		{
			str1 = str2 + (CString)_T(" = ");
			str1 += WriteCUDouble( _U_DIFFPRESS, pHM->GetpDpC()->GetDpToStab(), true );
		}

		pclSheet->SetStaticText( CD_Prim_TechInfo_DpCDpL, lRow, str1 );
		pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, WriteCUDouble( _U_DIFFPRESS, pHM->GetpDpC()->GetDpmin(), true ) );
		pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, pHM->GetpDpC()->GetSettingStr( true ) );

		// Show HMin only if different from 0.
		double dHMin = pHM->GetpDpC()->GetpParentHM()->GetDp( true );
		str1 = CteEMPTY_STRING;

		if( dHMin > 0.0 )
		{
			str1 = WriteCUDouble( _U_DIFFPRESS, dHMin, true );
		}

		if( NULL != pHM->GetpDpC()->GetpParentHM()->GetpSch() && eDpStab::DpStabOnCV == pHM->GetpDpC()->GetpParentHM()->GetpSch()->GetDpStab()
				&& true == pHM->GetpDpC()->GetpParentHM()->IsCvExist( true ) )
		{
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str2 = TASApp.LoadLocalizedString( IDS_KVSERIES_KV );
			}
			else
			{
				str2 = TASApp.LoadLocalizedString( IDS_KVSERIES_CV );
			}
			
			if( NULL != pHM->GetpDpC()->GetpParentHM()->GetpCV() )
			{
				double dKvCv = pHM->GetpDpC()->GetpParentHM()->GetpCV()->GetKvs();
				str1 += _T(" (" ) + str2 + _T( "=" ) + (CString)WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, dKvCv ), 2, 0, true ) + _T( ")");
			}
		}

		pclSheet->SetStaticText( CD_Prim_TechInfo_DpCHmin, lRow, str1 );

		if( NULL != pHM->GetpDpC()->GetpParentHM()->GetpCV() && pHM->GetpDpC()->GetpParentHM()->GetpCV()->GetAuth() > 0 )
		{
			if( NULL != pHM->GetpDpC()->GetpParentHM()->GetpSch() && eDpStab::DpStabOnCV == pHM->GetpDpC()->GetpParentHM()->GetpSch()->GetDpStab() )
			{
				str1 = WriteDouble( pHM->GetpDpC()->GetpParentHM()->GetpCV()->GetAuth(), 2, 0 );
				pclSheet->SetStaticText( CD_Prim_TechInfo_DpCAuth, lRow, str1 );
			}
		}
		
		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclDpc, pclDpc->GetArtNum() );
		pclSheet->SetStaticText( CD_Prim_Qty, lRow, _T("1") );
		
		// Secondary.
		if( NULL != pclBv )
		{
			pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pclBv->GetName() );
			pclSheet->SetStaticText( CD_Second_Product_Size, lRow, pclBv->GetSize() );
			pclSheet->SetStaticText( CD_Second_Product_Connection, lRow, pclBv->GetConnect() );
			pclSheet->SetStaticText( CD_Second_Product_Version, lRow, pclBv->GetVersion() );
			str = pclBv->GetPN().c_str();
			str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
			str = str + pclBv->GetTempRange( true );
			pclSheet->SetStaticText( CD_Second_Product_PNTminTmax, lRow, str );
			
			pclSheet->SetStaticText( CD_Second_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pHM->GetQ(), true ) );

			CWaterChar *pclWaterChar = pHM->GetpWaterChar( pHM->GetpBv()->GetHMObjectType() );
			ASSERT( NULL != pclWaterChar );
			
			// Dp.
			double dDp = pHM->GetpBv()->GetDp();
			str = CteEMPTY_STRING;
			
			if( true == pclBv->IsKvSignalEquipped() )
			{
				CDB_FixOCharacteristic *pChar = dynamic_cast<CDB_FixOCharacteristic *>( pclBv->GetValveCharDataPointer() );
				
				if( NULL != pChar && -1.0 != pChar->GetKvSignal() )
				{
					double dDpSignal = CalcDp( pHM->GetpBv()->GetQ(), pChar->GetKvSignal(), pclWaterChar->GetDens() );

					str = TASApp.LoadLocalizedString( IDS_SSHEETSELP_DPSIGNAL );
					str += _T("=") + CString( WriteCUDouble( _U_DIFFPRESS, dDpSignal, true ) );
					dDp = CalcDp( pHM->GetpBv()->GetQ(), pChar->GetKv(), pclWaterChar->GetDens() );
				}
			}

			if( dDp > 0.0 )
			{
				str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
			}
			else	//dDp == 0.0
			{
				// Compute Dp based on opening and flow.
				if( NULL != pclBv->GetValveCharacteristic() )
				{
					if( true == pclBv->GetValveCharacteristic()->GetValveDp( pHM->GetpBv()->GetQ(), &dDp, pHM->GetpBv()->GetSetting(), pclWaterChar->GetDens(), pclWaterChar->GetKinVisc() ) )
					{
						str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
					}
				}
			}
			
			pclSheet->SetStaticText( CD_Second_TechInfo_Dp, lRow, str );
			double dPresset = pHM->GetpBv()->GetSetting();
			
			if( dPresset > 0.0 && NULL != pclBv->GetValveCharacteristic() )
			{
				pclSheet->SetStaticText( CD_Second_TechInfo_Setting, lRow, pclBv->GetValveCharacteristic()->GetSettingString( dPresset, true ) );
			}
			else
			{
				pclSheet->SetStaticText( CD_Second_TechInfo_Setting, lRow, _T("-") );
			}
			
			FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclBv, pclBv->GetArtNum() );
			pclSheet->SetStaticText( CD_Second_Qty, lRow, _T("1") );
		}
	}
	else if( NULL != pclBv )
	{
		if( CteEMPTY_STRING == strBvPosition )
		{
			FillOneHMRowForBv( pHM, pHM->GetpBv(), pclSheet, pclBv, lRow );
		}
		else if( TASApp.LoadLocalizedString( IDS_SECONDARY ) == strBvPosition )
		{
			FillOneHMRowForBv( pHM, pHM->GetpSecBv(), pclSheet, pclBv, lRow );
		}
		else if( TASApp.LoadLocalizedString(IDS_BYPASS) == strBvPosition )
		{
			FillOneHMRowForBv( pHM, pHM->GetpBypBv(), pclSheet, pclBv, lRow );
		}
			
	}
	else if( NULL != pclShutoffValve )
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclShutoffValve->GetName() );
		pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclShutoffValve->GetSize() );
		pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclShutoffValve->GetConnect() );
		pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclShutoffValve->GetVersion() );
		str = pclShutoffValve->GetPN().c_str();
		str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
		str = str + pclShutoffValve->GetTempRange( true );
		pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

		CDS_HydroMod::eHMObj eShutoffValveLocated = CDS_HydroMod::eNone;

		if( true == pHM->IsShutoffValveExist( CDS_HydroMod::eShutoffValveSupply, true ) )
		{
			eShutoffValveLocated = CDS_HydroMod::eShutoffValveSupply;
		}
		else if( true == pHM->IsShutoffValveExist( CDS_HydroMod::eShutoffValveReturn, true ) )
		{
			eShutoffValveLocated = CDS_HydroMod::eShutoffValveReturn;
		}
		else
		{
			// Error: no shut-off valve.
			return ;
		}

		double dQ = pHM->GetpShutoffValve( eShutoffValveLocated )->GetQ();
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
		double dDp = pHM->GetpShutoffValve( eShutoffValveLocated )->GetDp();
		
		CWaterChar *pclWaterChar = pHM->GetpWaterChar( eShutoffValveLocated );
		ASSERT( NULL != pclWaterChar );

		CString str;
		
		if( dDp > 0.0 )
		{
			str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
		}
		else	//dDp == 0.0
		{
			// Compute Dp based on opening and flow.
			if( NULL != pclShutoffValve->GetValveCharacteristic() )
			{
				double dDp = pclShutoffValve->GetValveCharacteristic()->GetDpFullOpening( dQ, pclWaterChar->GetDens(), pclWaterChar->GetKinVisc() );

				if( -1.0 != dDp )
				{
					str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
				}
			}
		}
			
		pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, str );
		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclShutoffValve, pclShutoffValve->GetArtNum() );
		pclSheet->SetStaticText( CD_Prim_Qty, lRow, _T("1") );
	}	
	else if( NULL != pclSmartControlValve )
	{
		// HYS-1680.
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclSmartControlValve->GetName() );
		pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
		pclSheet->SetStaticText( CD_Prim_Product_BodyMaterial, lRow, pclSmartControlValve->GetBodyMaterial() );
		pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclSmartControlValve->GetConnect() );
		pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclSmartControlValve->GetVersion() );

		CString str = pclSmartControlValve->GetPN().c_str();
		str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
		str = str + pclSmartControlValve->GetTempRange( true );
		pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

		pclSheet->SetStaticText( CD_Prim_Product_Location, lRow, pHM->GetpSmartControlValve()->GetStrLocalization() );
		pclSheet->SetStaticText( CD_Prim_Product_Kvs, lRow, WriteCUDouble( _C_KVCVCOEFF, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs() ) );
		pclSheet->SetStaticText( CD_Prim_Product_Qnom, lRow, WriteCUDouble( _U_FLOW, pclSmartControlValve->GetSmartValveCharacteristic()->GetQnom(), true ) );
		pclSheet->SetStaticText( CD_Prim_Product_CableLengthToRemoteTempSensor, lRow, WriteCUDouble( _U_LENGTH, pclSmartControlValve->GetRemoteTempSensorCableLength(), true ) );

		// Write power supply.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) + CString( _T(" : ") ) + pclSmartControlValve->GetPowerSupplyStr();
		pclSheet->SetStaticText( CD_Prim_Product_TASmartPowerSupply, lRow, str );

		// Write input signal.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_INPUT ) + CString( _T(" : ") ) + pclSmartControlValve->GetInOutSignalsStr( true );
		pclSheet->SetStaticText( CD_Prim_Product_TASmartInputSignal, lRow, str );

		// Write output signal.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_OUTPUTSIG );
		if( CString( _T("") ) == pclSmartControlValve->GetInOutSignalsStr( false ) )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pclSmartControlValve->GetInOutSignalsStr( false );
		}

		pclSheet->SetStaticText( CD_Prim_Product_TASmartOutputSignal, lRow, str );

		// Flow (That is equal to the flow max.).
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pHM->GetpSmartControlValve()->GetQ(), true ) );

		// Power/DT.
		if( SmartValveControlMode::SCVCM_Power == pHM->GetpSmartControlValve()->GetControlMode() )
		{
			str = CString( WriteCUDouble( _U_TH_POWER, pHM->GetpSmartControlValve()->GetPowerMax(), true ) );
			str += _T(" / ");
			str += WriteCUDouble( _U_DIFFTEMP, pHM->GetpSmartControlValve()->GetDT(), true );
			pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, str );
		}

		// HYS-1914: It's here the Dp min.
		double dDpMin = pHM->GetpSmartControlValve()->GetDpMin();
		str = WriteCUDouble( _U_DIFFPRESS, dDpMin, true );
		pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, str );

		// Control mode.
		pclSheet->SetStaticText( CD_Prim_TechInfo_ControlMode, lRow, pclSmartControlValve->GetControlModeString( pHM->GetpSmartControlValve()->GetControlMode() ) );

		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclSmartControlValve, pclSmartControlValve->GetArtNum() );
		pclSheet->SetStaticText( CD_Prim_Qty, lRow, _T("1") );
	}
	else if( NULL != pclSmartDpC )
	{
		// HYS-2007.
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclSmartDpC->GetName() );
		pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
		pclSheet->SetStaticText( CD_Prim_Product_BodyMaterial, lRow, pclSmartDpC->GetBodyMaterial() );
		pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclSmartDpC->GetConnect() );
		pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclSmartDpC->GetVersion() );

		CString str = pclSmartDpC->GetPN().c_str();
		str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
		str = str + pclSmartDpC->GetTempRange( true );
		pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

		pclSheet->SetStaticText( CD_Prim_Product_Location, lRow, pHM->GetpSmartDpC()->GetStrLocalization() );
		pclSheet->SetStaticText( CD_Prim_Product_Kvs, lRow, WriteCUDouble( _C_KVCVCOEFF, pclSmartDpC->GetSmartValveCharacteristic()->GetKvs() ) );
		pclSheet->SetStaticText( CD_Prim_Product_Qnom, lRow, WriteCUDouble( _U_FLOW, pclSmartDpC->GetSmartValveCharacteristic()->GetQnom(), true ) );
		pclSheet->SetStaticText( CD_Prim_Product_CableLengthToRemoteTempSensor, lRow, WriteCUDouble( _U_LENGTH, pclSmartDpC->GetRemoteTempSensorCableLength(), true ) );


		// Flow (That is equal to the flow max.).
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pHM->GetpSmartDpC()->GetQ(), true ) );

		if( 0 < pHM->GetpSmartDpC()->GetDp() )
		{
			pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pHM->GetpSmartDpC()->GetDp() ) );
		}

		// HYS-2007: DpL for Smart Dp
		if( pHM->GetpSmartDpC()->GetDpToStabilize() > 0.0 )
		{
			pclSheet->SetStaticText( CD_Prim_TechInfo_DpCDpL, lRow, WriteCUDouble( _U_DIFFPRESS, pHM->GetpSmartDpC()->GetDpToStabilize(), true ) );
		}
		// It's here the Dp min.
		double dDpMin = pHM->GetpSmartDpC()->GetDpMin();
		str = WriteCUDouble( _U_DIFFPRESS, dDpMin, true );
		pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, str );

		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclSmartDpC, pclSmartDpC->GetArtNum() );
		pclSheet->SetStaticText( CD_Prim_Qty, lRow, _T("1") );

		if( NULL != pHM->GetpSmartDpC()->GetpProductSet() )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			CDB_Product* pclSet = dynamic_cast<CDB_Product*>(pHM->GetpSmartDpC()->GetpProductSet() );
			FillSelectedDpSensorRow( NULL, pclSet, pclSheet, &lRow );

			CDB_DpSensor* pclDpSensorSet = dynamic_cast<CDB_DpSensor*>(pHM->GetpSmartDpC()->GetpProductSet() );
			if( NULL == pclDpSensorSet )
			{
				// for connection set
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_CONNECTIONSETCONTENT );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_DPSENSORSETCONTENT );
			}

			CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>(pclSet->GetAccessoriesGroupIDPtr().MP);

			if( NULL != pclRuledTable )
			{
				for( int iLoopSmartDpCDpSensorAccSet = 0; iLoopSmartDpCDpSensorAccSet < pHM->GetpSmartDpC()->GetSetContentCount(); iLoopSmartDpCDpSensorAccSet++ )
				{
					CDB_Product* pclAccessory = (CDB_Product*)(pHM->GetpSmartDpC()->GetSetContentIDPtr( iLoopSmartDpCDpSensorAccSet ).MP);

					if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
					{
						continue;
					}

					bool bByPair = pclRuledTable->IsByPair( pclAccessory->GetIDPtr().ID );
					lRow = FillAccessoryDpSensorSet( pclSheet, lRow, pclAccessory, false, (false == bByPair) ? 1 : 2, str, (LPARAM)pHM );

					if( iLoopSmartDpCDpSensorAccSet + 1 == pHM->GetpSmartDpC()->GetSetContentCount() )
					{
						lRow--;
					}
				}
			}
		}
	}

	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillHMRow( CDS_HydroMod *m_pHM, CSSheet *pclSheet, long *plRow )
{
	if( ( NULL == m_pHM ) || ( NULL == pclSheet ) )
	{
		return;
	}

	long lRow = *plRow;
	
	// Sort HM in function of position.
	CRank HMList;
	CString str;
	long lHMRows = lRow;
	int iNbChildren = 0;

	for( IDPTR IDPtr = m_pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pHM->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)IDPtr.MP;
		HMList.Add( str, pHM->GetPos(), (LPARAM)IDPtr.MP );
		lHMRows += 1;	// 1 row by circuit
		iNbChildren++;
	}

	//  HYS-1095: If we are on the root module we have to continue.
	if( (0 != m_pHM->GetLevel() ) && ( lHMRows == lRow ) )
	{
		return;
	}
	// Parent module at the first position.
	if( NULL == m_pHM->GetParent() )
	{
		HMList.Add( str, 0, (LPARAM)m_pHM->GetIDPtr().MP );
		lHMRows += 1;	// 1 row for parent module.
	}

	CDS_HydroMod *pHM = NULL;
	LPARAM lparam = (LPARAM)0;

	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		pHM = (CDS_HydroMod *)lparam;
		CString strBvPosition = CteEMPTY_STRING;
		CDB_RegulatingValve *pclBv = NULL;
		CDB_TAProduct *pclBvBp = NULL;
		CDB_TAProduct *pclSecBv = NULL;
		CDB_ControlValve *pclCv = NULL;
		CDB_DpController *pclDpc = NULL;
		CDB_TAProduct *pclSv = NULL;
		CDB_SmartControlValve *pclSmCv = NULL;
		CDB_SmartControlValve *pclSmDpC = NULL;

		if( NULL != pHM->GetpBv() )
		{
			pclBv = dynamic_cast<CDB_RegulatingValve*>( pHM->GetpBv()->GetIDPtr().MP );
		}
		if( NULL != pHM->GetpBypBv() )
		{
			pclBvBp = dynamic_cast<CDB_TAProduct*>( pHM->GetpBypBv()->GetIDPtr().MP );
		}
		if( NULL != pHM->GetpSecBv() )
		{
			pclSecBv = dynamic_cast<CDB_TAProduct*>( pHM->GetpSecBv()->GetIDPtr().MP );
		}
		if( NULL != pHM->GetpCV() )
		{
			pclCv = dynamic_cast<CDB_ControlValve*>( pHM->GetpCV()->GetCvIDPtr().MP );
		}
		if( NULL != pHM->GetpDpC() )
		{
			pclDpc = dynamic_cast<CDB_DpController*>( pHM->GetpDpC()->GetIDPtr().MP );
		}
		if( NULL != pHM->GetpShutoffValve( CDS_HydroMod::eALL ) )
		{
			pclSv = dynamic_cast<CDB_TAProduct*>( pHM->GetpShutoffValve( CDS_HydroMod::eALL )->GetIDPtr().MP );
		}
		else if( NULL != pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn ) )
		{
			pclSv = dynamic_cast<CDB_TAProduct*>( pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn )->GetIDPtr().MP );
		}		
		else if( NULL != pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply ) )
		{
			pclSv = dynamic_cast<CDB_TAProduct*>( pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn )->GetIDPtr().MP );
		}

		// HYS-1680
		if( NULL != pHM->GetpSmartControlValve() )
		{
			pclSmCv = dynamic_cast<CDB_SmartControlValve*>( pHM->GetpSmartControlValve()->GetIDPtr().MP );
		}

		// HYS-2007
		if( NULL != pHM->GetpSmartDpC() )
		{
			pclSmDpC = dynamic_cast<CDB_SmartControlValve*>( pHM->GetpSmartDpC()->GetIDPtr().MP );
		}

		if( ( NULL == pclCv ) && ( NULL == pclBv ) && ( NULL == pclBvBp ) && ( NULL == pclSecBv ) && ( NULL == pclDpc ) && ( NULL == pclSv ) 
			&& ( NULL == pclSmCv ) && ( NULL == pclSmDpC ) )
		{
			// HYS-1055: We look under HM children even if they don't have any valve
			if( pHM != m_pHM )
			{
				FillHMRow( pHM, pclSheet, &lRow );
			}
			continue;
		}
		else
		{
			// HYS-1095: We add new line for child line
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}

		if( NULL != pclCv )
		{
			if( lRow > pclSheet->GetMaxRows() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}
			
			FillOneHMRow( pHM, pclCv, pclSheet, &lRow, strBvPosition );
		}
		
		if( NULL != pclDpc )
		{
			if( lRow > pclSheet->GetMaxRows() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}
			
			FillOneHMRow( pHM, pclDpc, pclSheet, &lRow, strBvPosition );
			pclBv = NULL;
		}

		// HYS-1680
		if( NULL != pclSmCv )
		{
			if( lRow > pclSheet->GetMaxRows() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}

			FillOneHMRow( pHM, pclSmCv, pclSheet, &lRow, strBvPosition );
		}

		// HYS-2007
		if( NULL != pclSmDpC )
		{
			if( lRow > pclSheet->GetMaxRows() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}

			FillOneHMRow( pHM, pclSmDpC, pclSheet, &lRow, strBvPosition );
		}

		if( NULL != pclBv )
		{
			if( lRow > pclSheet->GetMaxRows() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}
			
			FillOneHMRow( pHM, pclBv, pclSheet, &lRow, strBvPosition );
		}		
		
		if( NULL != pclSecBv )
		{
			if( lRow > pclSheet->GetMaxRows() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}
			
			strBvPosition = TASApp.LoadLocalizedString( IDS_SECONDARY );
			FillOneHMRow( pHM, pclSecBv, pclSheet, &lRow, strBvPosition );
		}		
		
		if( NULL != pclBvBp )
		{
			if( lRow > pclSheet->GetMaxRows() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}
			
			strBvPosition = TASApp.LoadLocalizedString( IDS_BYPASS );
			FillOneHMRow( pHM, pclBvBp, pclSheet, &lRow, strBvPosition );
		}
		
		if( NULL != pclSv )
		{
			if( lRow > pclSheet->GetMaxRows() )
			{
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}
			
			FillOneHMRow( pHM, pclSv, pclSheet, &lRow, strBvPosition );
		}
		
		if( pHM != m_pHM )
		{
			FillHMRow( pHM, pclSheet, &lRow );
		}
	}
	
	// Border.
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc + 1, CD_Mode, lRow - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectionRow( CDS_SSel *pclSSel, CSSheet *pclSheet, ValveType etype, long *plRow )
{
	long lRow = *plRow;
	CString str;

	if( ( NULL == pclSSel ) || ( NULL == pclSheet ) )
	{
		return;
	}

	CDB_ControlValve *pclCv = pclSSel->GetProductAs<CDB_ControlValve>();
	CDB_DpCBCValve *pclDpCBCv = pclSSel->GetProductAs<CDB_DpCBCValve>();
	CDS_SSelDpC *pclSSelDpC = dynamic_cast<CDS_SSelDpC *>( pclSSel );
	CDS_SSelBv *pclSSelBv = dynamic_cast<CDS_SSelBv *>( pclSSel );
	CDB_ShutoffValve *pclSv = pclSSel->GetProductAs<CDB_ShutoffValve>();
	CDB_ThermostaticValve *pclTrv = pclSSel->GetProductAs<CDB_ThermostaticValve>();
	CDS_SSelRadSet *pclInsert = NULL;
	CDB_FlowLimitedControlValve *pFlowLimitedControlValve = pclSSel->GetProductAs<CDB_FlowLimitedControlValve>();
	CDB_SmartControlValve *pclSmartControlValve = pclSSel->GetProductAs<CDB_SmartControlValve>();
	CDS_SSelSmartControlValve *pclSSelSmartControlValve = dynamic_cast<CDS_SSelSmartControlValve *>( pclSSel );
	CDS_SSelSmartDpC *pclSSelSmartDpC = dynamic_cast<CDS_SSelSmartDpC *>( pclSSel );
	CDB_SmartControlValve *pclSmartDpC = pclSSel->GetProductAs<CDB_SmartControlValve>();
	CDB_DpSensor *pclDpSensor = pclSSel->GetProductAs<CDB_DpSensor>();

	if( NULL == pclCv && NULL == pclSSelDpC && NULL == pclSSelBv && NULL == pclSv && NULL == pclTrv && NULL == pFlowLimitedControlValve && NULL == pclSSelSmartControlValve 
			&& NULL == pclSSelSmartDpC && NULL == pclDpSensor )
	{
		// for Trv with insert.
		pclInsert = dynamic_cast<CDS_SSelRadSet *>( pclSSel );

		if( NULL == pclInsert )
		{
			return;
		}
	}

	if( NULL != pclCv && etype == ValveType::CV )
	{
		FillSelectedCVRow( pclSSel, pclCv, pclSheet, &lRow );
		
		if( true == ( (CDS_SSelCtrl *)pclSSel )->IsAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_CVACCESSORY );
			FillAccessoriesCtrlSet( pclSheet, &lRow,  (CDS_SSelCtrl *)pclSSel, str, true  );
		}

		if( true == ( (CDS_SSelCtrl *)pclSSel )->IsActuatorAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_CVACTUATORACCESSORY );
			FillAccessoriesCtrlSet( pclSheet, &lRow, (CDS_SSelCtrl *)pclSSel, str, false );
		}
	}	

	if( NULL != pclDpCBCv && etype == ValveType::DpCBCV )
	{
		FillSelectedDpCBCvRow( pclSSel, pclDpCBCv, pclSheet, &lRow );

		if( true == ( (CDS_SSelDpCBCV *)pclSSel )->IsAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_DPCBCVACCESSORY );
			FillAccessoriesDpCBCV( pclSheet, &lRow, (CDS_SSelDpCBCV *)pclSSel, str, true );
		}		
		if( true == ( (CDS_SSelDpCBCV *)pclSSel )->IsActuatorAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_DPCBCVACTUATORACCESSORY );
			FillAccessoriesDpCBCV( pclSheet, &lRow, (CDS_SSelDpCBCV *)pclSSel, str, false );
		}

		CDB_ShutoffValve *pclSVselected = dynamic_cast<CDB_ShutoffValve*>( ( (CDS_SSelDpCBCV *)pclSSel )->GetSVIDPtr().MP);
		
		if( NULL != pclSVselected )
		{
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			FillSelectedSVRow( pclSSel, pclSVselected, pclSheet, &lRow );
			
			if( true == ( (CDS_SSelDpCBCV *)pclSSel )->IsSVAccessoryExist() )
			{
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_DPCBCVSHUTOFFACCESSORY );
				FillAccessories( pclSheet, &lRow, ( (CDS_SSelDpCBCV *)pclSSel )->GetSVAccessoryList(), ( (CDS_SSelDpCBCV *)pclSSel )->GetpSelectedInfos()->GetQuantity(), str, pclSSel );
			}
		}
	}

	if( NULL != pclSSelDpC && etype == ValveType::DpC )
	{
		FillSelectedDpCRow( pclSSelDpC, pclSheet, &lRow );
		
		if( true == pclSSelDpC->IsDpCAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_DPCACCESSORY );
			FillAccessories( pclSheet, &lRow, pclSSelDpC->GetDpCAccessoryList(), pclSSelDpC->GetpSelectedInfos()->GetQuantity(), str, pclSSelDpC );
		}
		
		if( true == pclSSelDpC->IsAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_BVACCESSORY );
			FillAccessories( pclSheet, &lRow, pclSSelDpC->GetAccessoryList(), pclSSelDpC->GetpSelectedInfos()->GetQuantity(), str, pclSSelDpC );
		}
		
		pclSSelBv = NULL;
	}

	if( NULL != pclSSelBv && etype == ValveType::BV )
	{
		FillSelectedBVRow( pclSSelBv, pclSheet, &lRow );
		
		if( true == pclSSelBv->IsAccessoryExist() )
		{	
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_BVACCESSORY );
			FillAccessories( pclSheet, &lRow, pclSSelBv->GetAccessoryList(), pclSSelBv->GetpSelectedInfos()->GetQuantity(), str, pclSSelBv );
		}
	}

	if( NULL != pclSv && etype == ValveType::SV )
	{
		FillSelectedSVRow( pclSSel, pclSv, pclSheet, &lRow );
		
		if( true == ( (CDS_SSelSv *)pclSSel )->IsAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_SVACCESSORY );
			FillAccessories( pclSheet, &lRow, ( (CDS_SSelSv *)pclSSel )->GetAccessoryList(), ( (CDS_SSelSv *)pclSSel )->GetpSelectedInfos()->GetQuantity(), str, pclSSel );
		}
		
		if( true == ( (CDS_SSelSv *)pclSSel )->IsActuatorAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_SVACTUATORACCESSORY );
			FillAccessories( pclSheet, &lRow, ( (CDS_SSelSv *)pclSSel )->GetActuatorAccessoryList(), ( (CDS_SSelSv *)pclSSel )->GetpSelectedInfos()->GetQuantity(), str, pclSSel );
		}
	}

	if( ( NULL != pclTrv || NULL != pFlowLimitedControlValve ) && etype == ValveType::Trv )
	{
		CDB_TAProduct *pclProduct = pclSSel->GetProductAs<CDB_TAProduct>();
		FillSelectedTrvRow( pclSSel, pclProduct, pclSheet, &lRow );
		
		if( true == ( (CDS_SSelRadSet *)pclSSel )->IsSupplyValveAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_SUPPLYVALVEACCESSORY );
			FillAccessories( pclSheet, &lRow, ( (CDS_SSelRadSet *)pclSSel )->GetSupplyValveAccessoryList(), ( (CDS_SSelRadSet *)pclSSel )->GetpSelectedInfos()->GetQuantity(), str, pclSSel );
		}
		
		if( true == ( (CDS_SSelRadSet *)pclSSel )->IsSVActuatorAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_SUPPLYVALVEACTUATORACCESSORY );
			FillAccessories( pclSheet, &lRow, ( (CDS_SSelRadSet *)pclSSel )->GetSVActuatorAccessoryList(), ( (CDS_SSelRadSet *)pclSSel )->GetpSelectedInfos()->GetQuantity(), str, pclSSel );
		}
		
		if( NULL != dynamic_cast<CDS_SSelRadSet *>( pclSSel ) && true == ( dynamic_cast<CDS_SSelRadSet *>( pclSSel ) )->IsReturnValveExist() )
		{
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			CDB_TAProduct *pclProductReturn = dynamic_cast<CDB_TAProduct *>( dynamic_cast<CDS_SSelRadSet *>( pclSSel )->GetReturnValveIDPtr().MP );
			FillSelectedReturnValveRow( pclSSel, pclProductReturn, pclSheet, &lRow );
		
			if( true == ( (CDS_SSelRadSet *)pclSSel )->IsReturnValveAccessoryExist() )
			{
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_RETURNVALVEACTUATORACCESSORY );
				FillAccessories( pclSheet, &lRow, ( (CDS_SSelRadSet *)pclSSel )->GetReturnValveAccessoryList(), ( (CDS_SSelRadSet *)pclSSel )->GetpSelectedInfos()->GetQuantity(), str, pclSSel );
			}
		}
	}	

	if( NULL != pclInsert )
	{
		FillSelectedTrvRow( pclInsert, NULL, pclSheet, &lRow );
		
		if( true == pclInsert->IsReturnValveExist() )
		{
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			CDB_TAProduct *pclProduct = dynamic_cast<CDB_TAProduct*>( pclInsert->GetReturnValveIDPtr().MP );
			FillSelectedReturnValveRow( pclSSel, pclProduct, pclSheet, &lRow );
		}
	}

	if( NULL != pclCv && etype == ValveType::TA6WayV )
	{
		FillSelected6WayValveRow( pclSSel, pclCv, pclSheet, &lRow );

		if( true == ( (CDS_SSelCtrl *)pclSSel )->IsAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_CVACCESSORY );
			FillAccessoriesCtrlSet( pclSheet, &lRow, (CDS_SSelCtrl *)pclSSel, str, true );
		}

		if( true == ( (CDS_SSelCtrl *)pclSSel )->IsActuatorAccessoryExist() )
		{
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_CVACTUATORACCESSORY );
			FillAccessoriesCtrlSet( pclSheet, &lRow, (CDS_SSelCtrl *)pclSSel, str, false );
		}

		if( e6WayValveSelectionMode::e6Way_EQMControl == ( (CDS_SSel6WayValve *)pclSSel )->GetSelectionMode()
				&& NULL != ( (CDS_SSel6WayValve *)pclSSel )->GetCDSSSelPICv( SideDefinition::BothSide ) )
		{
			CDS_SSelPICv *pclPIBCv = ( (CDS_SSel6WayValve *)pclSSel )->GetCDSSSelPICv( SideDefinition::BothSide );
			long fistRowPibcv = lRow;
			CDB_ControlValve* pclPIBCvProduct = pclPIBCv->GetProductAs< CDB_ControlValve>();

			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			// HYS-1380: we call directly FillSelectedCVRow with CDS_SSel6WayValve to have change-over information
			FillSelectedCVRow( pclSSel, pclPIBCvProduct, pclSheet, &lRow );

			if( true == ((CDS_SSelCtrl*)pclPIBCv)->IsAccessoryExist() )
			{
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_CVACCESSORY );
				FillAccessoriesCtrlSet( pclSheet, &lRow, (CDS_SSelCtrl*)pclPIBCv, str, true );
			}

			if( true == ((CDS_SSelCtrl*)pclPIBCv)->IsActuatorAccessoryExist() )
			{
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_CVACTUATORACCESSORY );
				FillAccessoriesCtrlSet( pclSheet, &lRow, (CDS_SSelCtrl*)pclPIBCv, str, false );
			}
			pclSheet->SetStaticText( CD_Prim_Infos, fistRowPibcv, TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYEQM ) );
			pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, fistRowPibcv, _T( "-" ) );
		}
		else if( e6WayValveSelectionMode::e6Way_OnOffControlWithPIBCV == ( (CDS_SSel6WayValve *)pclSSel )->GetSelectionMode()
				&& NULL != ( (CDS_SSel6WayValve *)pclSSel )->GetCDSSSelPICv( SideDefinition::CoolingSide ) )
		{
			if( NULL != ( (CDS_SSel6WayValve *)pclSSel )->GetCDSSSelPICv( SideDefinition::HeatingSide ) )
			{
				// Two products are added.
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 2 );

				for( int i = 0; i < 2; i++ )
				{
					CString infoStr = ( i == 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_6WAYVALVECOOLINGSIDE ) : TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_6WAYVALVEHEATINGSIDE );
					SideDefinition eSide = ( i == 0 ) ? SideDefinition::CoolingSide : SideDefinition::HeatingSide;
					CDS_SSelPICv *pclPIBCv = ( (CDS_SSel6WayValve *)pclSSel )->GetCDSSSelPICv( eSide );
					long fistRowPibcv = lRow;
					pclPIBCv->SetQ( ( (CDS_SSel6WayValve *)pclSSel )->GetFlow( eSide ) );
					pclPIBCv->SetPower( ( (CDS_SSel6WayValve *)pclSSel )->GetPower( eSide ) );
					pclPIBCv->SetDT( ( (CDS_SSel6WayValve *)pclSSel )->GetDT( eSide ) );
					pclPIBCv->SetFlowDef( ( (CDS_SSel6WayValve *)pclSSel )->GetFlowDef() );

					pclPIBCv->GetpSelectedInfos()->SetQuantity( ( (CDS_SSel6WayValve *)pclSSel )->GetpSelectedInfos()->GetQuantity() );

					FillSelectionRow( pclPIBCv, pclSheet, ValveType::CV, &lRow );
					pclSheet->SetStaticText( CD_Prim_Infos, fistRowPibcv, infoStr );
				}
			}
		}
		else if( e6WayValveSelectionMode::e6Way_OnOffControlWithSTAD == ( (CDS_SSel6WayValve *)pclSSel )->GetSelectionMode()
				&& NULL != ( (CDS_SSel6WayValve *)pclSSel )->GetCDSSSelBv( SideDefinition::CoolingSide ) )
		{
			if( NULL != ( (CDS_SSel6WayValve *)pclSSel )->GetCDSSSelBv( SideDefinition::HeatingSide ) )
			{
				// Two products are added.
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 2 );

				for( int i = 0; i < 2; i++ )
				{
					CString infoStr = ( i == 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_6WAYVALVECOOLINGSIDE ) : TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_6WAYVALVEHEATINGSIDE );
					SideDefinition eSide = ( i == 0 ) ? SideDefinition::CoolingSide : SideDefinition::HeatingSide;
					CDS_SSelBv *pclBv = ( (CDS_SSel6WayValve *)pclSSel )->GetCDSSSelBv( eSide );
					long fistRowBv = lRow;
					pclBv->SetQ( ( (CDS_SSel6WayValve *)pclSSel )->GetFlow( eSide ) );
					// HYS-1380: Set PDT information to display it if it is available
					pclBv->SetPower( ( (CDS_SSel6WayValve *)pclSSel )->GetPower( eSide ) );
					pclBv->SetDT( ( (CDS_SSel6WayValve *)pclSSel )->GetDT( eSide ) );
					pclBv->SetFlowDef( ( (CDS_SSel6WayValve *)pclSSel )->GetFlowDef() );
					pclBv->GetpSelectedInfos()->SetQuantity( ( (CDS_SSel6WayValve *)pclSSel )->GetpSelectedInfos()->GetQuantity() );

					FillSelectionRow( pclBv, pclSheet, ValveType::BV, &lRow );
					pclSheet->SetStaticText( CD_Prim_Infos, fistRowBv, infoStr );
				}
			}
		}
	}

	if( NULL != pclSSelSmartControlValve && ValveType::SmartControlValve == etype )
	{
		FillSelectedSmartControlValveRow( pclSSelSmartControlValve, pclSmartControlValve, pclSheet, &lRow );
		
		if( true == pclSSelSmartControlValve->IsAccessoryExist() )
		{	
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_SMARTVALVEACCESSORY );
			FillAccessories( pclSheet, &lRow, pclSSelSmartControlValve->GetAccessoryList(), pclSSelSmartControlValve->GetpSelectedInfos()->GetQuantity(), str, pclSSelSmartControlValve );
		}
	}

	if( NULL != pclSSelSmartDpC && ValveType::SmartDpC == etype )
	{
		FillSelectedSmartDpCRow( pclSSelSmartDpC, pclSmartDpC, pclSheet, &lRow );
		
		if( true == pclSSelSmartDpC->IsAccessoryExist() )
		{	
			str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_SMARTVALVEACCESSORY );
			FillAccessories( pclSheet, &lRow, pclSSelSmartDpC->GetAccessoryList(), pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity(), str, pclSSelSmartDpC );
		}

		// HYS-2007
		if( NULL != dynamic_cast<CDB_Product*>(pclSSelSmartDpC->GetSetIDPtr().MP) )
		{
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			CDB_Product* pclDpSensorSet = dynamic_cast<CDB_Product*>(pclSSelSmartDpC->GetSetIDPtr().MP);
			FillSelectedDpSensorRow( pclSSelSmartDpC, pclDpSensorSet, pclSheet, &lRow );
		}

		if( true == pclSSelSmartDpC->IsSetContentAccessoryExist() )
		{
			CDB_DpSensor* pclDpSensorSet = dynamic_cast<CDB_DpSensor*>(pclSSelSmartDpC->GetSetIDPtr().MP);
			if( NULL == pclDpSensorSet )
			{
				// for connection set
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_CONNECTIONSETCONTENT );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_DPSENSORSETCONTENT );
			}

			FillAccessories( pclSheet, &lRow, pclSSelSmartDpC->GetSetContentAccessoryList(), pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity(), str, pclSSelSmartDpC, true );
		}
	}
	// HYS-2007: Dp sensor alone via dirsel.
	if( NULL != pclDpSensor && ValveType::DpC == etype )
	{
		FillSelectedDpSensorRow( pclSSel, pclDpSensor, pclSheet, &lRow );

		if( true == pclSSel->IsAccessoryExist() )
		{	
			if( true == pclDpSensor->IsPartOfaSet() )
			{
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_DPSENSORSETCONTENT );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_EXPORTINONESHEET_DPSENSORACCESSORY );
			}
			FillAccessories( pclSheet, &lRow, pclSSel->GetAccessoryList(), pclSSel->GetpSelectedInfos()->GetQuantity(), str, pclSSel, pclDpSensor->IsPartOfaSet() );
		}
	}

	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc + 1, CD_Mode, lRow - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedBVRow( CDS_SSelBv *pclSSel, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CString str;

	if( NULL == pclSSel || NULL == pclSheet )
	{
		return;
	}

	CDB_TAProduct *pclProduct = dynamic_cast<CDB_TAProduct *>( pclSSel->GetProductIDPtr().MP );

	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}
		
		m_selectionMode = str;
	}
	
	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclProduct->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclProduct->GetVersion() );
	
	str = pclProduct->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclProduct->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
	
	if( false == pclSSel->IsFromDirSel() )
	{
		double dQ = pclSSel->GetQ();
		double dRho = pclSSel->GetpSelectedInfos()->GetpWCData()->GetDens();
		double dNu = pclSSel->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
		double dPresset = pclSSel->GetOpening();
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );

		CString strPowerDt = CteEMPTY_STRING;

		if( CDS_SelProd::efdPower == pclSSel->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSel->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSel->GetDT(), true );
		}
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
		double dDp = 0.0;
		str = CteEMPTY_STRING;
		
		if( true == pclProduct->IsKvSignalEquipped() )
		{
			CDB_FixOCharacteristic *pChar = dynamic_cast<CDB_FixOCharacteristic *>( ( (CDB_RegulatingValve *)(pclProduct) )->GetValveCharDataPointer() );
			
			if( NULL != pChar && -1.0 != pChar->GetKvSignal() )
			{
				double dDpSignal = CalcDp( dQ, pChar->GetKvSignal(), dRho );
				CString str;
				str = TASApp.LoadLocalizedString( IDS_SSHEETSELP_DPSIGNAL );
				str += _T("=") + CString( WriteCUDouble( _U_DIFFPRESS, dDpSignal, true ) );
				dDp = CalcDp( dQ, pChar->GetKv(), dRho );
			}
		}

		if( 0.0 == dDp )
		{
			// Compute Dp based on opening and flow.
			if( NULL != ( (CDB_RegulatingValve*)( pclProduct ) )->GetValveCharacteristic() )
			{
				if( true == pclProduct->GetValveCharacteristic()->GetValveDp( dQ, &dDp, dPresset, dRho, dNu ) )
				{
					str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
				}
			}
		}
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, str );

		// Not for a fixed orifice valve.
		if( 0 != _tcscmp( pclProduct->GetTypeID(), _T("RVTYPE_FO") ) )
		{
			CString str1;
			str1 = _T("-");
			
			if( dPresset > 0.0 && NULL != ( (CDB_RegulatingValve *)( pclProduct ) )->GetValveCharacteristic() )
			{
				str1 = ( (CDB_RegulatingValve *)( pclProduct ) )->GetValveCharacteristic()->GetSettingString( dPresset, true );
			}
			
			pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str1 );
		}
	}
	
	FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, pclProduct->GetArtNum(true) );
	
	double dQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedCVRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CDS_SSelCtrl *pclCv = NULL;
	// HYS-1380: To show change-over information if we are with change-over selection
	CDS_SSel6WayValve *pclTA6WayValve = NULL;
	CString str;

	if( NULL == pclSSel || NULL == pclSheet || NULL == pclProduct )
	{
		return;
	}

	// HYS-1380: Check if we are is PIBCV case for TA-6way-valve
	if( NULL != dynamic_cast<CDS_SSel6WayValve*>( pclSSel ) )
	{
		// Set information about change-over PIBCV in EQM control mode
		pclCv = ( (CDS_SSel6WayValve*)pclSSel )->GetCDSSSelPICv( SideDefinition::BothSide );
		pclTA6WayValve = dynamic_cast<CDS_SSel6WayValve*>( pclSSel );
		pclCv->SetQ( ( (CDS_SSel6WayValve*)pclSSel )->GetBiggestFlow() );
		pclCv->SetPower( ( (CDS_SSel6WayValve*)pclSSel )->GetBiggestPower() );
		pclCv->SetDT( ( (CDS_SSel6WayValve*)pclSSel )->GetBiggestDT() );
		pclCv->GetpSelectedInfos()->SetQuantity( ( (CDS_SSel6WayValve*)pclSSel )->GetpSelectedInfos()->GetQuantity() );
		pclCv->SetFlowDef( ( (CDS_SSel6WayValve*)pclSSel )->GetFlowDef() );
	}
	else if( NULL != dynamic_cast<CDB_ControlValve*>( pclProduct ) )
	{
		pclCv = dynamic_cast<CDS_SSelCtrl*>( pclSSel );
	}

	if( NULL == pclCv )
	{
		return;
	}

	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}
		
		m_selectionMode = str;
	}

	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	// HYS-1877: For Set product we display set name.
	if( true == pclCv->IsSelectedAsAPackage( true ) && ( CDB_ControlValve::ControlValveTable::PICV != ((CDB_ControlValve*)pclProduct)->GetCVParentTable()
														 || NULL == pclTA6WayValve ) )
	{
		CDB_Set* pCvActSet = dynamic_cast<CDB_Set*>(pclCv->GetCvActrSetIDPtr().MP);
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pCvActSet->GetName() );
	}
	else
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	}
	pclSheet->SetStaticText( CD_Prim_Product_BodyMaterial, lRow, pclProduct->GetBodyMaterial() );

	CDB_PIControlValve *pTAPICV = dynamic_cast<CDB_PIControlValve *>( pclProduct );
	str = CteEMPTY_STRING;

	CDB_ControlValve *pTACV = dynamic_cast<CDB_ControlValve *>( pclProduct );

	if( NULL != pTACV )
	{
		if( NULL == pTACV->GetValveCharDataPointer() && -1.0 != pTACV->GetKvs() )
		{
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_KVS );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_CV );
			}

			str += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, ( (CDB_ControlValve *)( pclProduct ) )->GetKvs() );
		}
	}

	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclProduct->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclProduct->GetVersion() );

	str = pclProduct->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclProduct->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
	
	if( false == pclCv->IsFromDirSel() )
	{
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pclCv->GetQ(), true ) );

		CString strPowerDt = CteEMPTY_STRING;
		
		if( CDS_SelProd::efdPower == pclCv->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclCv->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclCv->GetDT(), true );
			pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
		}
		
		str = CteEMPTY_STRING;
		double dQ = pclCv->GetQ();
		double dRho = pclCv->GetpSelectedInfos()->GetpWCData()->GetDens();
		double dKinVisc = pclCv->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
		CDB_ControlValve::ControlValveTable eControlValveTable = ( (CDB_ControlValve *)( pclProduct ) )->GetCVParentTable();
		
		switch( eControlValveTable )
		{
			case CDB_ControlValve::ControlValveTable::CV:
				{
					double dDp = CalcDp( dQ, ( (CDB_ControlValve *)( pclProduct ) )->GetKvs(), dRho );
					pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
				}
				break;

			case CDB_ControlValve::ControlValveTable::BCV:
				{
					CDS_SSelBCv *pSelBCv = dynamic_cast<CDS_SSelBCv *>( pclCv );
					double dH = pSelBCv->GetOpening();

					if( dH > 0.0 )
					{
						// Compute Dp for current opening and current flow.
						CDB_ValveCharacteristic *pValvChar = (CDB_ValveCharacteristic *)( (CDB_ControlValve *)( pclProduct ) )->GetValveCharDataPointer();

						if( NULL != pValvChar )
						{
							double dDp = 0.0;

							if( true == pValvChar->GetValveDp( dQ, &dDp, dH, dRho, dKinVisc ) )
							{
								str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
							}
						}
					}
					pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, str );
				}
				break;

			case CDB_ControlValve::ControlValveTable::PICV:
				{
					double dDpMin = 0.0;
					CDB_PIControlValve* pPICV = pclCv->GetProductAs<CDB_PIControlValve>();
					// HYS-1380: For PIBCV in EQM control mode show both Dp min. cooling and heating
					// Check if this is for TA-6Way-valve
					if( NULL != pclTA6WayValve )
					{
						double dCoolingFlow = pclTA6WayValve->GetFlow( CoolingSide );
						double dHeatingFlow = pclTA6WayValve->GetFlow( HeatingSide );
						double dRhoCooling = pclTA6WayValve->GetWC( CoolingSide ).GetDens();
						double dRhoHeating = pclTA6WayValve->GetWC( HeatingSide ).GetDens();
						double dDpMinCooling = pPICV->GetDpmin( dCoolingFlow, dRhoCooling );
						double dDpMinHeating = pPICV->GetDpmin( dHeatingFlow, dRhoHeating );

						if( -1.0 != dDpMinCooling )
						{
							str = WriteCUDouble( _U_DIFFPRESS, dDpMinCooling, true );
						}
						else
						{
							str = GetDashDotDash();
						}

						if( -1.0 != dDpMinHeating )
						{
							str += (CString)_T( " / " );
							str += WriteCUDouble( _U_DIFFPRESS, dDpMinHeating, true );
						}
						else
						{
							str += (CString)_T( " / " ) + GetDashDotDash();
						}
					}
					else
					{
						dDpMin = pPICV->GetDpmin( dQ, dRho );
						if( -1.0 != dDpMin )
						{
							str = WriteCUDouble( _U_DIFFPRESS, dDpMin, true );
						}
						else
						{
							str = GetDashDotDash();
						}
					}
					
					// HYS-1380: Show Dp min. in colum Dp min.
					pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, str );
				}
				break;
		}

		str = _T("-");

		switch( eControlValveTable )
		{
			case CDB_ControlValve::ControlValveTable::CV:
				break;

			case CDB_ControlValve::ControlValveTable::BCV:
				{
					CDS_SSelBCv *pSelBCv = dynamic_cast<CDS_SSelBCv *>( pclCv );
					double dH = pSelBCv->GetOpening();

					if( 0.0 != dH && NULL != ( (CDB_ControlValve *)( pclProduct ) )->GetValveCharDataPointer() )
					{
						CDB_ValveCharacteristic *pValvChar = (CDB_ValveCharacteristic *)( (CDB_ControlValve *)( pclProduct ) )->GetValveCharDataPointer();
						str = pValvChar->GetSettingString( dH, true );
					}
				}

				break;

			case CDB_ControlValve::ControlValveTable::PICV:
				{
					CDB_PIControlValve *pPICV = pclCv->GetProductAs<CDB_PIControlValve>();
					CDB_ValveCharacteristic *pValvChar = pPICV->GetValveCharacteristic();

					if( NULL != pValvChar && 0.0 != dQ )
					{
						double dPresetting = pPICV->GetPresetting( dQ, dRho, dKinVisc );

						if( -1.0 != dPresetting )
						{
							str = pValvChar->GetSettingString( dPresetting, true );
						}
					}
				}

				break;
		}

		pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str );
	}
	
	str = _T("-");
	
	if( true == pclCv->IsSelectedAsAPackage( true ) )
	{
		CDB_Set* pCvActSet = dynamic_cast<CDB_Set *>( pclCv->GetCvActrSetIDPtr().MP );
		// HYS-1877: For Set product we display set article number.
		str = pCvActSet->GetReference();
		FillSetArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pCvActSet, str );
	}
	else
	{
		// Remark: 'true' to exclude any connection reference in the article number.
		str = pclProduct->GetArtNum( true );
		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, str );
	}
	
	double dQuantity = pclCv->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );

	// Secondary.
	CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( pclCv->GetpActuator() );
	
	if( NULL != pclActuator )
	{
		pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pclActuator->GetName() );
		CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( ( (CDB_ControlValve *)pclProduct )->GetCloseOffCharIDPtr().MP );

		if( NULL != pCloseOffChar )
		{
			str = CteEMPTY_STRING;

			if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CLOSEOFFDP );
				double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );

				if( dCloseOffDp > 0.0 )
				{
					str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
				}
			}
			else if( CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXINLETPRESSURE );
				double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclActuator->GetMaxForceTorque() );

				if( dMaxInletPressure > 0.0 )
				{
					str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
				}
			}

			pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, lRow, str );
		}

		// Compute Actuating time in sec.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclActuator->GetActuatingTimesStr( ( (CDB_ControlValve *)pclProduct )->GetStroke(), true );
		pclSheet->SetStaticText( CD_Second_Product_ActActTime, lRow, str );

		// Write IP.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
		str += CString( _T(" : ") ) + pclActuator->GetIPxxAuto();

		if( false == pclActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T("(" ) + pclActuator->GetIPxxManual() + _T( ")");
		}
		
		pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );
		pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, pclActuator->GetPowerSupplyStr() );
		pclSheet->SetStaticText( CD_Second_Product_ActInputSignal, lRow, pclActuator->GetInOutSignalsStr( true ) );
		pclSheet->SetStaticText( CD_Second_Product_ActOutputSignal, lRow, pclActuator->GetInOutSignalsStr( false ) );
		pclSheet->SetStaticText( CD_Second_Product_ActRelayType, lRow, pclActuator->GetRelayStr() );
		str.Empty();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

		if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_NO );
		}

		pclSheet->SetStaticText( CD_Second_Product_ActFailSafe, lRow, str );
		CString strPositionTitle = ( (int)pclActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
		str = strPositionTitle + CString( _T(" : ") ) + pclActuator->GetDefaultReturnPosStr( pclActuator->GetDefaultReturnPos() ).c_str();
		pclSheet->SetStaticText( CD_Second_Product_ActDRP, lRow, str );
		
		if( true == pclCv->IsSelectedAsAPackage( true ) )
		{
			pclSheet->SetStaticText( CD_Second_ArticleNumber, lRow, _T("-") );
		}
		else
		{
			FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclActuator, pclActuator->GetArtNum() );
		}
		
		double dQuantity = pclCv->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_Second_Qty, lRow, WriteDouble( dQuantity, 1) );
	}

	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedActrRow( CDS_Actuator *pclSSel, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CDB_Actuator *pclActuator = NULL;
	if( NULL == pclSSel || NULL == pclSheet )
	{
		return;
	}

	pclActuator = dynamic_cast<CDB_Actuator *>(pclSSel->GetActuatorIDPtr().MP);

	if( NULL == pclActuator )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}
	
		m_selectionMode = str;
	}

	pclSheet->SetStaticText( CD_Mode, lRow, str );		
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pclActuator->GetName() );

	FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclActuator, pclActuator->GetArtNum() );

	double dQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Second_Qty, lRow, WriteDouble( dQuantity, 1 ) );

	CDB_ElectroActuator *pclElecActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );
	CDB_ThermostaticActuator *pclThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator *>( pclActuator );

	if( NULL != pclElecActuator )
	{
		if( CDB_CloseOffChar::Linear == pclElecActuator->GetOpeningType() )
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXFORCE );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXTORQUE );
		}

		str += CString( _T(" : ") ) + pclElecActuator->GetMaxForceTorqueStr( true );
		pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, lRow, str );

		// Compute Actuating time in sec/mm or sec/deg.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclElecActuator->GetActuatingTimesStr( 0, true );
		pclSheet->SetStaticText( CD_Second_Product_ActActTime, lRow, str );

		// Write IP.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
		str += CString( _T(" : ") ) + pclElecActuator->GetIPxxAuto();

		if( false == pclElecActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T("(" ) + pclElecActuator->GetIPxxManual() + _T( ")");
		}
		
		pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );
		pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, pclElecActuator->GetPowerSupplyStr() );
		pclSheet->SetStaticText( CD_Second_Product_ActInputSignal, lRow, pclElecActuator->GetInOutSignalsStr( true ) );

		// Write output signal.
		if( CString( CteEMPTY_STRING ) == pclElecActuator->GetInOutSignalsStr( false ) )
		{
			str = CString( _T(" :    -") );
		}
		else
		{
			str = CString( _T(" : ") ) + pclElecActuator->GetInOutSignalsStr( false );
		}

		pclSheet->SetStaticText( CD_Second_Product_ActOutputSignal, lRow, str );
		pclSheet->SetStaticText( CD_Second_Product_ActRelayType, lRow, pclElecActuator->GetRelayStr() );

		str.Empty();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

		if( pclElecActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pclElecActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_NO );
		}
		
		pclSheet->SetStaticText( CD_Second_Product_ActFailSafe, lRow, str );
		
		CString strPositionTitle = ( (int)pclElecActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
		str = strPositionTitle + CString( _T(" : ") ) + pclElecActuator->GetDefaultReturnPosStr( pclElecActuator->GetDefaultReturnPos() ).c_str();
		pclSheet->SetStaticText( CD_Second_Product_ActDRP, lRow, str );
	}
	else if ( NULL != pclThermostaticActuator )
	{
		int iMinSetting = pclThermostaticActuator->GetMinSetting();
		int iMaxSetting = pclThermostaticActuator->GetMaxSetting();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SETTINGRANGE );

		if( iMinSetting >= 0 && iMaxSetting > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iMinSetting, true );
			str += CString( _T(" - ") ) + WriteCUDouble( _U_TEMPERATURE, iMaxSetting, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}
		
		pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, lRow, str );

		// Write capillary length.
		int iCapillaryLength = pclThermostaticActuator->GetCapillaryLength();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CAPILLARYLEN );

		if( iCapillaryLength != -1 && iCapillaryLength > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_LENGTH, iCapillaryLength, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}
		
		pclSheet->SetStaticText( CD_Second_Product_ActActTime, lRow, str );

		// Write hysteresis.
		double dHysteresis = pclThermostaticActuator->GetHysteresis();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_HYSTERESIS );

		if( dHysteresis != -1.0 && dHysteresis > 0.0 )
		{
			str += CString( _T(" : " ) ) + WriteDouble( dHysteresis, 3, 1, 1 ) + CString( _T( " K") );
		}
		else
		{
			str += CString( _T(" : - ") );
		}
		
		pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );

		// Write frost protection.
		int iFrostProtection = pclThermostaticActuator->GetFrostProtection();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FROSTPROTECTION );

		if( iFrostProtection != -1 && iFrostProtection > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iFrostProtection, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}
		
		pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, str );
	}
	
	// HYS-987: Fill Accessory for actuator alone
	if( pclSSel->GetpActuatorAccessoryList()->size() > 0 )
	{
		lRow++;
		CRank rkList;

		for( int i = 0; i < (int)pclSSel->GetpActuatorAccessoryList()->size(); i++ )
		{
			CDS_Actuator::AccessoryItem *pclAccessoryItem = new CDS_Actuator::AccessoryItem();
			*pclAccessoryItem = pclSSel->GetpActuatorAccessoryList()->at( i );
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( pclAccessoryItem->IDPtr.MP );
			VERIFY( NULL != pclAccessory );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				break;
			}

			rkList.AddStrSort( pclAccessory->GetName(), 0, (LPARAM)pclAccessoryItem, false, false );
		}

		CString str;
		LPARAM lParam;
		
		for( BOOL bContinue = rkList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rkList.GetNext( str, lParam ) )
		{
			CDS_Actuator::AccessoryItem *pclAccessoryItem = ( CDS_Actuator::AccessoryItem * )lParam;
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
			bool bByPair = pclAccessoryItem->fByPair;
			
			// HYS-987: Manage actuator accessories.
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int ActAccQty = pclSSel->GetpSelectedInfos()->GetQuantity();

			if( -1 != pclAccessoryItem->lEditedQty )
			{
				ActAccQty = pclAccessoryItem->lEditedQty;
			}
			else if( true == bByPair )
			{
				ActAccQty *= 2;
			}
			
			delete pclAccessoryItem;
			
			pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			pclSheet->SetStaticText( CD_Prim_Infos, lRow, _T("Act accessory") );
			CString strAccessoryInfos = (CString)pclAccessory->GetName() + _T(" ; ") + pclAccessory->GetComment();
			pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, strAccessoryInfos );

			if( ( pclAccessory->GetPmaxmax() < DBL_MAX  && pclAccessory->GetPmaxmax() >= 0 ) && ( pclAccessory->GetTmin() > -273.15 || pclAccessory->GetTmax() < DBL_MAX ) )
			{
				CString str = pclAccessory->GetPN().c_str();
				str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
				str = str + pclAccessory->GetTempRange( true );
				pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
			}
			
			FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclAccessory, pclAccessory->GetArtNum() );
			pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( ActAccQty, 1 ) );
			lRow++;
		}
	}
	else
	{
		lRow++;
	}
	
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc + 1, CD_Mode, lRow - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_Prim_Ref1, RD_Header_FirstCirc, CD_Mode, lRow - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedDpCBCvRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CDS_SSelDpCBCV *pclDpCBCv = NULL;

	if( NULL == pclSSel || NULL == pclProduct || NULL == pclSheet )
	{
		return;
	}

	if( NULL != dynamic_cast<CDB_DpCBCValve*>( pclProduct ) )
	{
		pclDpCBCv = dynamic_cast<CDS_SSelDpCBCV*>( pclSSel );
	}

	if( NULL == pclDpCBCv )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	CString str;

	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}

		m_selectionMode = str;
	}

	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	// HYS-1877: For Set product we display set name.
	if( true == pclDpCBCv->IsSelectedAsAPackage( true ) )
	{
		CDB_Set* pCvActSet = dynamic_cast<CDB_Set*>(pclDpCBCv->GetCvActrSetIDPtr().MP);
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pCvActSet->GetName() );
	}
	else
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	}
	pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclProduct->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclProduct->GetVersion() );
	
	str = pclProduct->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclProduct->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

	if( false == pclDpCBCv->IsFromDirSel() )
	{
		double dQ = pclDpCBCv->GetQ();
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
		CString strPowerDt = CteEMPTY_STRING;

		if( CDS_SelProd::efdPower == pclDpCBCv->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclDpCBCv->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclDpCBCv->GetDT(), true );
		}

		pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );

		double dRho = pclDpCBCv->GetpSelectedInfos()->GetpWCData()->GetDens();
		double dKinVisc = pclDpCBCv->GetpSelectedInfos()->GetpWCData()->GetKinVisc();

		// Dp min.
		// Get pressure drop over the pressure part of the valve.
		double dDpp = ( (CDB_DpCBCValve*)pclProduct )->GetDppmin( dQ, dRho );

		// Get pressure drop over the control part of the valve.
		double dDpc = ( (CDB_DpCBCValve*)pclProduct )->GetDpc( dQ, pclDpCBCv->GetDpToStalibize() );

		pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDpp + dDpc, true ) );

		// Setting.
		double dH = pclDpCBCv->GetOpening();

		str = _T("-");

		if( 0.0 != dH && NULL != ( (CDB_DpCBCValve*)pclProduct )->GetValveCharDataPointer() )
		{
			CDB_ValveCharacteristic *pValvChar = (CDB_ValveCharacteristic *)( (CDB_DpCBCValve*)pclProduct )->GetValveCharDataPointer();
			str = pValvChar->GetSettingString( dH, true );
		}
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str );

		// H min.
		str = _T("-");
		double dHMin = pclDpCBCv->GetHMin();

		if( 0.0 != dHMin )
		{
			str = WriteCUDouble( _U_DIFFPRESS, dHMin, true );
		}
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_DpCHmin, lRow, str );
		
		if( true == pclDpCBCv->IsSelectedAsAPackage( true ) )
		{
			CDB_Set *pDpCBCVActSet = dynamic_cast<CDB_Set *>( pclDpCBCv->GetCvActrSetIDPtr().MP );

			if( NULL == pDpCBCVActSet )
			{
				ASSERT( 0 );
				return;
			}
			
			FillSetArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pDpCBCVActSet, pDpCBCVActSet->GetReference() );
		}
		else
		{
			FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, pclProduct->GetArtNum(true) );
		}
	}

	double dQuantity = pclDpCBCv->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	
	// Secondary.
	CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( pclDpCBCv->GetpActuator() );

	if( NULL != pclActuator )
	{
		pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pclActuator->GetName() );
		CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( ( (CDB_DpCBCValve *)pclProduct )->GetCloseOffCharIDPtr().MP );

		if( NULL != pCloseOffChar )
		{
			str = CteEMPTY_STRING;

			if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CLOSEOFFDP );
				double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );

				if( dCloseOffDp > 0.0 )
				{
					str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
				}
			}
			else if( CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXINLETPRESSURE );
				double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclActuator->GetMaxForceTorque() );

				if( dMaxInletPressure > 0.0 )
				{
					str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
				}
			}
			
			pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, lRow, str );
		}

		// Compute Actuating time in sec.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclActuator->GetActuatingTimesStr( ( (CDB_DpCBCValve *)pclProduct )->GetStroke(), true );
		pclSheet->SetStaticText( CD_Second_Product_ActActTime, lRow, str );

		// Write IP.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
		str += CString( _T(" : ") ) + pclActuator->GetIPxxAuto();

		if( false == pclActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T("(" ) + pclActuator->GetIPxxManual() + _T( ")");
		}
		
		pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );
		pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, pclActuator->GetPowerSupplyStr() );
		pclSheet->SetStaticText( CD_Second_Product_ActInputSignal, lRow, pclActuator->GetInOutSignalsStr( true ) );
		pclSheet->SetStaticText( CD_Second_Product_ActOutputSignal, lRow, pclActuator->GetInOutSignalsStr( false ) );
		pclSheet->SetStaticText( CD_Second_Product_ActRelayType, lRow, pclActuator->GetRelayStr() );
		str.Empty();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

		if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_NO );
		}

		
		pclSheet->SetStaticText( CD_Second_Product_ActFailSafe, lRow, str );
		
		CString strPositionTitle = ( (int)pclActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
		str = strPositionTitle + CString( _T(" : ") ) + pclActuator->GetDefaultReturnPosStr( pclActuator->GetDefaultReturnPos() ).c_str() ;
		pclSheet->SetStaticText( CD_Second_Product_ActDRP, lRow, str);
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

		if( DBL_MAX == pclActuator->GetTmax() )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + WriteDouble( pclActuator->GetTmax(), 3 );
			str += TASApp.LoadLocalizedString( IDS_UNITDEGREECELSIUS );
		}
		
		pclSheet->SetStaticText( CD_Second_product_ActMaxTemp, lRow, str );
		FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclActuator, pclActuator->GetArtNum() );
		double dQuantity = pclDpCBCv->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_Second_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	}
	
	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedDpCRow( CDS_SSelDpC *pclSSel, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;

	if( NULL == pclSSel || NULL == pclSheet )
	{
		return;
	}

	CDB_TAProduct *pclProduct = dynamic_cast<CDB_TAProduct *>( pclSSel->GetDpCIDPtr().MP );
	
	if( NULL == pclProduct )
	{
		return;
	}
	
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	
	CString str;
	
	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}
	
		m_selectionMode = str;
	}

	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );

	// HYS-1877: For Set product we display set name.
	if( true == pclSSel->IsSelectedAsAPackage( true ) )
	{
		CDB_Set* pSet = dynamic_cast<CDB_Set*>(pclSSel->GetDpCMvPackageIDPtr().MP);
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pSet->GetName() );
	}
	else
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	}

	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclProduct->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclProduct->GetVersion() );
	pclSheet->SetStaticText( CD_Prim_Product_DpCDpLr, lRow, ( (CDB_DpController*)pclProduct )->GetFormatedDplRange( true ).c_str() );

	str = pclProduct->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclProduct->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

	if( false == pclSSel->IsFromDirSel() )
	{
		double Q = pclSSel->GetQ();
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, Q, true ) );
		CString strPowerDt = CteEMPTY_STRING;

		if( CDS_SelProd::efdPower == pclSSel->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSel->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSel->GetDT(), true );
		}

		pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
		str = CteEMPTY_STRING;
		double dDpL = pclSSel->GetDpL();

		// Show Dpl only if different from -1.
		CString str2 = TASApp.LoadLocalizedString( IDS_DPL );
		CString str1;
		
		if( DpStabOnCV == pclSSel->GetDpStab() )
		{
			str1 = str2 + (CString)_T(" = ");

			if( pclSSel->GetKv() > 0.0 )
			{
				str1 += WriteCUDouble( _U_DIFFPRESS, pclSSel->GetDpToStab(), true );
			}
			else
			{
				str1 += ( (CDB_DpController *)( pclProduct ) )->GetFormatedDplRange( true ).c_str();
			}
		}
		else
		{
			if( dDpL <= 0.0 )
			{
				CString str3;
				str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
				str3.MakeLower();
				str1 = str2 + (CString)_T(" ") + str3;
			}
			else
			{
				str1 = str2 + (CString)_T(" = ");
				str1 += WriteCUDouble( _U_DIFFPRESS, pclSSel->GetDpToStab(), true );
			}

		}
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_DpCDpL, lRow, str1 );
		
		// Show DPMin only if different from 0.
		if( pclSSel->GetQ() > 0.0 )
		{
			double dDpmin = pclSSel->GetDpMin();

			if( dDpmin > 0.0 )
			{
				pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDpmin, true ) );
			}
		}

		if( pclSSel->GetDpCSetting() > 0.0 )
		{
			CDB_DpCCharacteristic *pDpCCharacteristic = ( (CDB_DpController *)pclProduct )->GetDpCCharacteristic();

			if( NULL != pDpCCharacteristic )
			{
				str1 = pDpCCharacteristic->GetSettingString( pclSSel->GetDpCSetting() );
			}

			pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str1 );
		}

		// Show HMin only if different from 0.
		double dHMin = pclSSel->GetHMin();
		str1 = CteEMPTY_STRING;

		if( dHMin > 0 )
		{
			if( ( pclSSel->GetDpStab() == DpStabOnBranch && pclSSel->GetDpL() > 0.0 )
					|| pclSSel->GetDpStab() == DpStabOnCV )
			{
				pclSheet->SetStaticText( CD_Prim_TechInfo_DpCHmin, lRow, WriteCUDouble( _U_DIFFPRESS, dHMin, true ) );
			}
		}

		if( eDpStab::DpStabOnCV == pclSSel->GetDpStab() && pclSSel->GetKv() > 0.0 )
		{
			double dBeta = pclSSel->GetAuthority();
			str2 = TASApp.LoadLocalizedString( IDS_AUTHORITY );
			str1 = str2 + _T(">") + (CString)WriteDouble( dBeta, 2, 0 );
			pclSheet->SetStaticText( CD_Prim_TechInfo_DpCAuth, lRow, str1 );
		}
	}

	str = CteEMPTY_STRING;
	CDB_Set *pSet = NULL;

	if( true == pclSSel->IsSelectedAsAPackage( true ) )
	{
		pSet = dynamic_cast<CDB_Set *>( pclSSel->GetDpCMvPackageIDPtr().MP );

		if( NULL != pSet )
		{
			str = pSet->GetReference();
		}
	}

	if( true == str.IsEmpty() )
	{
		str = pclProduct->GetBodyArtNum();
		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, str );
	}
	else
	{
		FillSetArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pSet, str );
	}
	
	double dQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	CDB_RegulatingValve *pMV = pclSSel->GetProductAs<CDB_RegulatingValve>();
	
	// Secondary.
	if( NULL != pMV )
	{
		pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pMV->GetName() );
		pclSheet->SetStaticText( CD_Second_Product_Size, lRow, pMV->GetSize() );
		pclSheet->SetStaticText( CD_Second_Product_Connection, lRow, pMV->GetConnect() );
		pclSheet->SetStaticText( CD_Second_Product_Version, lRow, pMV->GetVersion() );
		CString str = pMV->GetPN().c_str();
		str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
		str = str + pMV->GetTempRange( true );
		pclSheet->SetStaticText( CD_Second_Product_PNTminTmax, lRow, str );
		
		if( false == pclSSel->IsFromDirSel() )
		{
			double dQ = pclSSel->GetQ();
			double dRho = pclSSel->GetpSelectedInfos()->GetpWCData()->GetDens();
			double dNu = pclSSel->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
			double dPresset = pclSSel->GetOpening();
			pclSheet->SetStaticText( CD_Second_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
			CString strPowerDt = CteEMPTY_STRING;
			
			if( CDS_SelProd::efdPower == pclSSel->GetFlowDef() )
			{
				strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSel->GetPower(), true );
				strPowerDt += _T(" / ");
				strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSel->GetDT(), true );
			}
			
			pclSheet->SetStaticText( CD_Second_TechInfo_PowerDt, lRow, strPowerDt );
			double dDp = 0.0;
			str = CteEMPTY_STRING;
			
			if( true == pMV->IsKvSignalEquipped() )
			{
				CDB_FixOCharacteristic *pChar = dynamic_cast<CDB_FixOCharacteristic *>( pMV->GetValveCharDataPointer() );
				
				if( NULL != pChar && -1.0 != pChar->GetKvSignal() )
				{
					double dDpSignal = CalcDp( dQ, pChar->GetKvSignal(), dRho );
					CString str;
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELP_DPSIGNAL );
					str += _T("=") + CString( WriteCUDouble( _U_DIFFPRESS, dDpSignal, true ) );
					dDp = CalcDp( dQ, pChar->GetKv(), dRho );
				}
			}

			if( 0.0 == dDp )
			{
				// Compute Dp based on opening and flow.
				if( NULL != pMV->GetValveCharacteristic() )
				{
					if( true == pMV->GetValveCharacteristic()->GetValveDp( dQ, &dDp, dPresset, dRho, dNu ) )
					{
						str == WriteCUDouble( _U_DIFFPRESS, dDp, true );
					}
				}
			}
			
			pclSheet->SetStaticText( CD_Second_TechInfo_Dp, lRow, str );
			
			// Not for a fixed orifice valve.
			if( 0 != _tcscmp( pMV->GetTypeID(), _T("RVTYPE_FO") ) )
			{
				CString str1;
				str1 = _T("-");

				if( dPresset > 0.0 && NULL != pMV->GetValveCharacteristic() )
				{
					str1 = pMV->GetValveCharacteristic()->GetSettingString( dPresset, true );
				}
				
				pclSheet->SetStaticText( CD_Second_TechInfo_Setting, lRow, str1 );
			}
		}
		
		FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pMV, pMV->GetArtNum( true ) );
		
		double dQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_Second_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	}

	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedSVRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CDS_SSelSv *pclSv = NULL;
	CDS_SSelDpCBCV *pclDpCBCv = NULL;

	if( NULL == pclSheet || NULL == pclSSel )
	{
		return;
	}

	if( NULL != dynamic_cast<CDB_ShutoffValve *>( pclProduct ) )
	{
		pclSv = dynamic_cast<CDS_SSelSv *>( pclSSel );

		if( NULL == pclSv )
		{
			pclDpCBCv = dynamic_cast<CDS_SSelDpCBCV *>( pclSSel );
		}
	}

	if( ( NULL == pclSv ) && ( NULL == pclDpCBCv) )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	CString str;

	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}
	
	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}
		
		m_selectionMode = str;
	}
	
	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclProduct->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclProduct->GetVersion() );

	str = pclProduct->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclProduct->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

	if( NULL != pclSv )
	{
		if( false == pclSv->IsFromDirSel() )
		{
			double dQ = pclSv->GetQ();
			double dRho = pclSv->GetpSelectedInfos()->GetpWCData()->GetDens();
			double dNu = pclSv->GetpSelectedInfos()->GetpWCData()->GetKinVisc();

			pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
			CString strPowerDt = CteEMPTY_STRING;

			if( CDS_SelProd::efdPower == pclSv->GetFlowDef() )
			{
				strPowerDt = WriteCUDouble( _U_TH_POWER, pclSv->GetPower(), true );
				strPowerDt += _T(" / ");
				strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSv->GetDT(), true );
				pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
			}
			
			// Compute Dp based on opening and flow.
			if( NULL != pclProduct->GetValveCharacteristic() )
			{
				double dDp = pclProduct->GetValveCharacteristic()->GetDpFullOpening( dQ, dRho, dNu );

				if( -1.0 != dDp )
				{
					pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
				}
			}
		}
		
		// Shutoff valve actuator.
		CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( pclSv->GetActrIDPtr().MP );

		if( NULL != pclActuator )
		{
			pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pclActuator->GetName() );
			
			// Write IP.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
			str += CString( _T(" : ") ) + pclActuator->GetIPxxAuto();

			if( false == pclActuator->GetIPxxManual().IsEmpty() )
			{
				str += _T("(" ) + pclActuator->GetIPxxManual() + _T( ")");
			}
			
			pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );
			pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, pclActuator->GetPowerSupplyStr() );
			pclSheet->SetStaticText( CD_Second_Product_ActInputSignal, lRow, pclActuator->GetInOutSignalsStr( true ) );
			pclSheet->SetStaticText( CD_Second_Product_ActOutputSignal, lRow, pclActuator->GetInOutSignalsStr( false ) );
			pclSheet->SetStaticText( CD_Second_Product_ActRelayType, lRow, pclActuator->GetRelayStr() );
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

			if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
			{
				str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
			}
			else if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
			{
				str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
			}
			else
			{
				str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_NO );
			}

			pclSheet->SetStaticText( CD_Second_Product_ActFailSafe, lRow, str );
			
			CString strPositionTitle = ( (int)pclActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
			str = strPositionTitle + CString( _T(" : ") ) + pclActuator->GetDefaultReturnPosStr( pclActuator->GetDefaultReturnPos() ).c_str();
			pclSheet->SetStaticText( CD_Second_Product_ActDRP, lRow, str );
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

			if( DBL_MAX == pclActuator->GetTmax() )
			{
				str += CString( _T(" :    -") );
			}
			else
			{
				str += CString( _T(" : ") ) + WriteDouble( pclActuator->GetTmax(), 3 );
				str += TASApp.LoadLocalizedString( IDS_UNITDEGREECELSIUS );
			}
			
			pclSheet->SetStaticText( CD_Second_product_ActMaxTemp, lRow, str );
			FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclActuator, pclActuator->GetArtNum() );
			
			double dQuantity = pclSv->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_Second_Qty, lRow, WriteDouble( dQuantity, 1 ) );
		}
	}
	else if( NULL != pclDpCBCv )
	{
		pclSheet->SetStaticText( CD_Prim_Infos, lRow, _T("With STS") );

		if( false == pclDpCBCv->IsFromDirSel() )
		{
			double dQ = pclDpCBCv->GetQ();
			double dRho = pclDpCBCv->GetpSelectedInfos()->GetpWCData()->GetDens();
			double dNu = pclDpCBCv->GetpSelectedInfos()->GetpWCData()->GetKinVisc();

			pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
			CString strPowerDt = CteEMPTY_STRING;
			
			if( CDS_SelProd::efdPower == pclDpCBCv->GetFlowDef() )
			{
				strPowerDt = WriteCUDouble( _U_TH_POWER, pclDpCBCv->GetPower(), true );
				strPowerDt += _T(" / ");
				strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclDpCBCv->GetDT(), true );
				pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
			}
			
			// Compute Dp based on opening and flow.
			if( NULL != pclProduct->GetValveCharacteristic() )
			{
				double dDp = pclProduct->GetValveCharacteristic()->GetDpFullOpening( dQ, dRho, dNu );

				if( -1.0 != dDp )
				{
					pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
				}
			}
		}
	}
	
	FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, pclProduct->GetArtNum() );
	
	double dQuantity = (( NULL != pclDpCBCv ) ? (pclDpCBCv->GetpSelectedInfos()->GetQuantity() ) : ( pclSv->GetpSelectedInfos()->GetQuantity() ));
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );

	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedTrvRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CDS_SSelRadSet *pclTrv = NULL;
	CDS_SSelRadSet *pclFlCV = NULL;

	if( NULL != pclProduct )
	{
		if( NULL != dynamic_cast<CDB_ThermostaticValve *>( pclProduct ) )
		{
			pclTrv = dynamic_cast<CDS_SSelRadSet *>( pclSSel );
		}
		else if( NULL != dynamic_cast<CDB_FlowLimitedControlValve *>( pclProduct ) )
		{
			pclFlCV = dynamic_cast<CDS_SSelRadSet *>( pclSSel );
		}

		if( ( NULL == pclTrv ) && ( NULL == pclFlCV ) )
		{
			return;
		}
	}
	else
	{
		pclTrv = dynamic_cast<CDS_SSelRadSet *>( pclSSel );

		if( NULL == pclTrv )
		{
			return;
		}
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	CString str;

	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}
	
	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}
		
		m_selectionMode = str;
	}
	
	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );

	if( true == ( dynamic_cast<CDS_SSelRadSet *>( pclSSel ) )->IsInsertInKv() )
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_OTHERINSERT ) );

		if( false == pclTrv->IsFromDirSel() )
		{
			double dQ = pclTrv->GetQ();
			double dRho = pclTrv->GetpSelectedInfos()->GetpWCData()->GetDens();
			double dNu = pclTrv->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
			pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
			
			CString str1;
			CString str2;
			CString strPowerDt = CteEMPTY_STRING;

			if( CDS_SelProd::efdPower == pclTrv->GetFlowDef() )
			{
				strPowerDt = WriteCUDouble( _U_TH_POWER, pclTrv->GetPower(), true );
				strPowerDt += _T(" / ");
				strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclTrv->GetDT(), true );
				pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
			}
			
			// Add Kvs (or CV depending of current unit).
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
			
			if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str1 = TASApp.LoadLocalizedString( IDS_KVS );
			}
			else
			{
				str1 = TASApp.LoadLocalizedString( IDS_CV );
			}

			str1 += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pclTrv->GetInsertKvValue(), false, 3, 2 );
			pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, str1 );

			// Show Dp only if different from -1.
			if( pclTrv->GetSupplyValveDp() != -1 )
			{
				if( pclTrv->GetSupplyValveDp() <= 0 )
				{
					str1 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
					str1.MakeLower();
				}
				else
				{
					str1 = WriteCUDouble( _U_DIFFPRESS, pclTrv->GetSupplyValveDp(), true );
				}
				
				pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, str1 );
			}

			// Show total Dp only if different from -1.
			if( pclTrv->GetDp() != -1 )
			{
				str1 = CteEMPTY_STRING;
				str2 = TASApp.LoadLocalizedString( IDS_DPTOT );
				
				if( 0 == pclTrv->GetDp() )
				{
					CString str3;
					str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
					str3.MakeLower();
					str1 = str2 + (CString)_T(" ") + str3;
				}
				else
				{
					str1 = str2 + (CString)_T(" = ");
					str1 += WriteCUDouble( _U_DIFFPRESS, pclTrv->GetDp(), true );
				}
				
				pclSheet->SetStaticText( CD_Prim_TechInfo_DpCHmin, lRow, str1 );
			}
		}
		
		pclSheet->SetStaticText( CD_Prim_ArticleNumber, lRow, _T("-") );
		
		double dQuantity = pclTrv->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
		pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
		pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclProduct->GetConnect() );
		pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclProduct->GetVersion() );
		
		str = pclProduct->GetPN().c_str();
		str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
		str = str + pclProduct->GetTempRange( true );
		pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
		
		if( NULL != pclTrv )
		{
			if( false == pclTrv->IsFromDirSel() )
			{
				double dQ = pclTrv->GetQ();
				double dRho = pclTrv->GetpSelectedInfos()->GetpWCData()->GetDens();
				double dNu = pclTrv->GetpSelectedInfos()->GetpWCData()->GetKinVisc();

				pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
				CString strPowerDt = CteEMPTY_STRING;
				
				if( CDS_SelProd::efdPower == pclTrv->GetFlowDef() )
				{
					strPowerDt = WriteCUDouble( _U_TH_POWER, pclTrv->GetPower(), true );
					strPowerDt += _T(" / ");
					strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclTrv->GetDT(), true );
					pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
				}
				
				// Show Dp only if different from -1.
				CString str1;
				CString str2;

				if( pclTrv->GetSupplyValveDp() != -1 )
				{
					if( pclTrv->GetSupplyValveDp() <= 0 )
					{
						CString str3;
						str1 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
						str1.MakeLower();
					}
					else
					{
						str1 = WriteCUDouble( _U_DIFFPRESS, pclTrv->GetSupplyValveDp(), true );
					}
					
					pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, str1 );
				}
				
				// Setting .
				// HYS-1305 : Display setting for presettable insert
				if( ( 0 == _tcscmp( ( (CDB_ThermostaticValve*)pclProduct )->GetTypeID(), _T( "TRVTYPE_PRESET" ) ) )
						|| ( 0 == _tcscmp( ( (CDB_ThermostaticValve*)pclProduct )->GetTypeID(), _T( "TRVTYPE_INSERT_PRESET" ) ) ) )
				{
					CDB_ValveCharacteristic *pChar = ( (CDB_ThermostaticValve *)pclProduct )->GetThermoCharacteristic();
					str2 = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );
					str1 = str2 + (CString)_T(" = ");

					if( NULL != pChar )
					{
						str1 += pChar->GetSettingString( pclTrv->GetSupplyValveOpening() );
					}
					else
					{
						str1 = _T("-");
					}
					
					pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str1 );
				}

				// Show total Dp only if different from -1.
				if( pclTrv->GetDp() != -1 )
				{
					str1 = CteEMPTY_STRING;
					str2 = TASApp.LoadLocalizedString( IDS_DPTOT );
					
					if( 0 == pclTrv->GetDp() )
					{
						CString str3;
						str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
						str3.MakeLower();
						str1 = str2 + (CString)_T(" ") + str3;
					}
					else
					{
						str1 = str2 + (CString)_T(" = ");
						str1 += WriteCUDouble( _U_DIFFPRESS, pclTrv->GetDp(), true );
					}
					
					pclSheet->SetStaticText( CD_Prim_TechInfo_DpCHmin, lRow, str1 );
				}
			}
			
			FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, pclProduct->GetArtNum() );
			
			double dQuantity = pclTrv->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );
		}
		else if( NULL != pclFlCV )
		{
			if( false == pclFlCV->IsFromDirSel() )
			{
				CString str1;
				CString str2;
				double dQ = pclFlCV->GetQ();
				double dRho = pclFlCV->GetpSelectedInfos()->GetpWCData()->GetDens();
				double dNu = pclFlCV->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
				str1 = TASApp.LoadLocalizedString( IDS_SSHEETSELP_FLOWRANGE );
				
				double dQmin = -1.0;
				double dQmax = -1.0;
				CDB_FLCVCharacteristic *pclFLCVCharacteristic = ( (CDB_FlowLimitedControlValve *)pclProduct )->GetFLCVCharacteristic();
				
				if( NULL != pclFLCVCharacteristic )
				{
					dQmin = pclFLCVCharacteristic->GetQLFmin();
					dQmax = pclFLCVCharacteristic->GetQNFmax();
				}
				
				CString strQmin = ( dQmin != -1.0 ) ? WriteCUDouble( _U_FLOW, dQmin ) : GetDashDotDash();
				CString strQmax = ( dQmax != -1.0 ) ? WriteCUDouble( _U_FLOW, dQmax ) : GetDashDotDash();
				str1 += _T("[" ) + strQmin + _T( ";" ) + strQmax + _T( "]");

				TCHAR unitname[_MAXCHARS];
				CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
				ASSERT( NULL != pUnitDB );

				GetNameOf( pUnitDB->GetUnit( _U_FLOW, pUnitDB->GetDefaultUnitIndex( _U_FLOW ) ), unitname );
				str1 += _T(" ") + CString( unitname );
				str1 += _T(" flow = ") + (CString)WriteCUDouble( _U_FLOW, dQ, true );
				pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, str1 );
				
				CString strPowerDt = CteEMPTY_STRING;
				
				if( CDS_SelProd::efdPower == pclFlCV->GetFlowDef() )
				{
					strPowerDt = WriteCUDouble( _U_TH_POWER, pclFlCV->GetPower(), true );
					strPowerDt += _T(" / ");
					strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclFlCV->GetDT(), true );
					pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
				}

				// Setting.
				if( ( 0 == _tcscmp( ( (CDB_FlowLimitedControlValve*)pclProduct )->GetTypeID(), _T("TRVTYPE_FLOWLIMITED") ) )
					|| ( 0 == _tcscmp( ( (CDB_FlowLimitedControlValve*)pclProduct )->GetTypeID(), _T("TRVTYPE_INSERT_FL") ) ) )
				{
					/*CDB_FLCVCharacteristic* pChar = pFlowLimitedControlValve->GetFLCVCharacteristic();
					CDB_ValveCharacteristic* pChar = ( (CDB_FlowLimitedControlValve*)pclProduct )->GetThermoCharacteristic();*/
					str2 = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );
					str1 = str2 + (CString)_T(" = ");

					if( NULL != pclFLCVCharacteristic )
					{
						str1 += pclFLCVCharacteristic->GetSettingString( pclFlCV->GetSupplyValveOpening() );
					}
					else
					{
						str1 += _T("-");
					}

					pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str1 );
				}

				// Show Dp min only if different from -1.
				double dDpmin = -1.0;

				if( NULL != pclFLCVCharacteristic )
				{
					dDpmin = pclFLCVCharacteristic->GetDpmin( pclFlCV->GetSupplyValveOpening() );
				}

				str1 = ( dDpmin != -1.0 ) ? WriteCUDouble( _U_DIFFPRESS, dDpmin, true ) : GetDashDotDash();
				pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, str1 );

			}

			FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, pclProduct->GetArtNum() );
			
			double dQuantity = pclFlCV->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );

		}
		
		// Fill actuator.
		pclTrv = dynamic_cast<CDS_SSelRadSet *>( pclSSel );
		
		if( true == pclTrv->IsSupplyValveActuatorExist() )
		{
			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclTrv->GetSupplyValveActuatorIDPtr().MP );

			// Fill actuator.
			if( NULL != pclActuator )
			{
				pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pclActuator->GetName() );

				if( NULL != dynamic_cast<CDB_ElectroActuator *>( pclActuator ) )
				{
					CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );

					// Write max force (or torque).
					if( CDB_CloseOffChar::Linear == pclElectroActuator->GetOpeningType() )
					{
						str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXFORCE );
					}
					else
					{
						str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXTORQUE );
					}

					str += CString( _T(" : ") ) + pclElectroActuator->GetMaxForceTorqueStr( true );
					pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, lRow, str );

					// Compute Actuating time in sec.
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclElectroActuator->GetActuatingTimesStr( 0, true );
					pclSheet->SetStaticText( CD_Second_Product_ActActTime, lRow, str );

					// Write IP.
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
					str += CString( _T(" : ") ) + pclElectroActuator->GetIPxxAuto();

					if( false == pclElectroActuator->GetIPxxManual().IsEmpty() )
					{
						str += _T("(" ) + pclElectroActuator->GetIPxxManual() + _T( ")");
					}
					
					pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );
					pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, pclElectroActuator->GetPowerSupplyStr() );
					pclSheet->SetStaticText( CD_Second_Product_ActInputSignal, lRow, pclElectroActuator->GetInOutSignalsStr( true ) );
					pclSheet->SetStaticText( CD_Second_Product_ActOutputSignal, lRow, pclElectroActuator->GetInOutSignalsStr( false ) );
					pclSheet->SetStaticText( CD_Second_Product_ActRelayType, lRow, pclElectroActuator->GetRelayStr() );
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

					if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
					{
						str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
					}
					else if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
					{
						str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
					}
					else
					{
						str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_NO );
					}
					
					pclSheet->SetStaticText( CD_Second_Product_ActFailSafe, lRow, str );
			
					CString strPositionTitle = ( (int)pclElectroActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
					str = strPositionTitle + CString( _T(" : ") ) + pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str();
					pclSheet->SetStaticText( CD_Second_Product_ActDRP, lRow, str );
					
					str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

					if( DBL_MAX == pclElectroActuator->GetTmax() )
					{
						str += CString( _T(" :    -") );
					}
					else
					{
						str += CString( _T(" : ") ) + WriteDouble( pclElectroActuator->GetTmax(), 3 );
						str += TASApp.LoadLocalizedString( IDS_UNITDEGREECELSIUS );
					}

					pclSheet->SetStaticText( CD_Second_product_ActMaxTemp, lRow, str );
					FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclActuator, pclActuator->GetArtNum() );

					double dQuantity = pclTrv->GetpSelectedInfos()->GetQuantity();
					pclSheet->SetStaticText( CD_Second_Qty, lRow, WriteDouble( dQuantity, 1 ) );
				}
				else if( NULL != dynamic_cast<CDB_ThermostaticActuator *>( pclActuator ) )
				{
					CDB_ThermostaticActuator *pclThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator *>( pclActuator );
					pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, lRow, pclActuator->GetComment() );
					
					// Write setting range.
					int iMinSetting = pclThermostaticActuator->GetMinSetting();
					int iMaxSetting = pclThermostaticActuator->GetMaxSetting();
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SETTINGRANGE );

					// HYS-951: Min setting can be 0.
					if( iMinSetting >= 0 && iMaxSetting > 0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iMinSetting, true );
						str += CString( _T(" - ") ) + WriteCUDouble( _U_TEMPERATURE, iMaxSetting, true );
					}
					else
					{
						str += CString( _T(" : - ") );
					}

					pclSheet->SetStaticText( CD_Second_Product_ActActTime, lRow, str );

					// Write capillary length.
					int iCapillaryLength = pclThermostaticActuator->GetCapillaryLength();
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CAPILLARYLEN );

					if( iCapillaryLength > 0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_LENGTH, iCapillaryLength, true );
					}
					else
					{
						str += CString( _T(" : - ") );
					}

					pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );

					// Write hysteresis.
					double dHysteresis = pclThermostaticActuator->GetHysteresis();
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_HYSTERESIS );

					if( dHysteresis != -1.0 && dHysteresis > 0.0 )
					{
						str += CString( _T(" : " ) ) + WriteDouble( dHysteresis, 3, 1, 1 ) + CString( _T( " K") );
					}
					else
					{
						str += CString( _T(" : - ") );
					}

					pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, str );

					// Write frost protection.
					int iFrostProtection = pclThermostaticActuator->GetFrostProtection();
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FROSTPROTECTION );

					if( iFrostProtection > 0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iFrostProtection, true );
					}
					else
					{
						str += CString( _T(" : - ") );
					}
					
					pclSheet->SetStaticText( CD_Second_Product_ActInputSignal, lRow, str );
					str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

					if( DBL_MAX == pclThermostaticActuator->GetTmax() )
					{
						str += CString( _T(" :    -") );
					}
					else
					{
						str += CString( _T(" : ") ) + WriteDouble( pclThermostaticActuator->GetTmax(), 3 );
						str += TASApp.LoadLocalizedString( IDS_UNITDEGREECELSIUS );
					}

					pclSheet->SetStaticText( CD_Second_product_ActMaxTemp, lRow, str );
					FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclActuator, pclActuator->GetArtNum() );

					double dQuantity = pclTrv->GetpSelectedInfos()->GetQuantity();
					pclSheet->SetStaticText( CD_Second_Qty, lRow, WriteDouble( dQuantity, 1 ) );
				}
			}
		}
	}

	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedSmartDpCRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CString str;

	if( NULL == pclSSel || NULL == dynamic_cast<CDS_SSelSmartDpC *>( pclSSel ) || NULL == pclProduct || NULL == pclSheet 
			|| NULL == dynamic_cast<CDB_SmartControlValve *>( pclProduct ) )
	{
		ASSERT_RETURN;
	}

	CDB_SmartControlValve *pclSmartDpC = (CDB_SmartControlValve *)( pclProduct );
	CDS_SSelSmartDpC *pclSelectedSmartDpC = (CDS_SSelSmartDpC *)( pclSSel );

	if( NULL == pclSmartDpC->GetSmartValveCharacteristic() )
	{
		ASSERT_RETURN;
	}

	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}
		
		m_selectionMode = str;
	}
	
	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );

	pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclSmartDpC->GetName() );

	pclSheet->SetStaticText( CD_Prim_Product_BodyMaterial, lRow, pclSmartDpC->GetBodyMaterial() );
	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclSmartDpC->GetSize() );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclSmartDpC->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclSmartDpC->GetVersion() );
	
	str = pclSmartDpC->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclSmartDpC->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

	pclSheet->SetStaticText( CD_Prim_Product_Kvs, lRow, WriteCUDouble( _C_KVCVCOEFF, pclSmartDpC->GetSmartValveCharacteristic()->GetKvs() ) );
	pclSheet->SetStaticText( CD_Prim_Product_Qnom, lRow, WriteCUDouble( _U_FLOW, pclSmartDpC->GetSmartValveCharacteristic()->GetQnom(), true ) );
	
	if( false == pclSSel->IsFromDirSel() )
	{
		double dQ = pclSSel->GetQ();
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );

		CString strPowerDt = CteEMPTY_STRING;

		if( CDS_SelProd::efdPower == pclSSel->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSel->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSel->GetDT(), true );
		}
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );

		if( 0 < pclSelectedSmartDpC->GetDp() )
		{
			pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedSmartDpC->GetDp() ) );
		}

		// Dpl if exists.
		if( true == pclSelectedSmartDpC->IsCheckboxDpBranchChecked() && pclSelectedSmartDpC->GetDpBranchValue() > 0.0 )
		{
			pclSheet->SetStaticText( CD_Prim_TechInfo_DpCDpL, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedSmartDpC->GetDpBranchValue(), true ) );
		}
		
		// HYS-1914: It's here the Dp min (Kvs is the Kv at full opening).
		double dDpMin = CalcDp( pclSSel->GetQ(), pclSmartDpC->GetSmartValveCharacteristic()->GetKvs(), pclSSel->GetpSelectedInfos()->GetpWCData()->GetDens() );
		pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDpMin, true ) );

		if( true == pclSelectedSmartDpC->IsCheckboxDpMaxChecked() )
		{
			pclSheet->SetStaticText( CD_Prim_TechInfo_DpMax, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedSmartDpC->GetDpMaxValue(), true ) );
		}
	}

	// HYS-2007: there is no more TA-Smart Dp set.
	// Remark: 'true' to exclude any connection reference in the article number.
	str = pclProduct->GetArtNum( true );
	FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclSmartDpC, str );

	double dQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedDpSensorRow( CDS_SSel *pclSSel, CDB_Product *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CString str;

	if( NULL == pclProduct || NULL == pclSheet )
	{
		ASSERT_RETURN;
	}

	bool bIsforSmartDpc = (NULL == dynamic_cast<CDS_SSelDpSensor*>(pclSSel)) ? true : false;

	CDB_DpSensor* pclDpSensor = dynamic_cast<CDB_DpSensor*>( pclProduct );
	CDS_SSelDpSensor *pclSelectedDpSensor = dynamic_cast<CDS_SSelDpSensor*>(pclSSel);
	
	// NULL == pclSSel means that we are in HMCalc.
	if( NULL != pclSSel )
	{
		if( true == pclSSel->IsFromDirSel() )
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
		}
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );


	if( NULL != pclSelectedDpSensor && false == bIsforSmartDpc )
	{	
		if( m_selectionMode != str )
		{
			if( m_selectionMode != CteEMPTY_STRING )
			{
				lRow++;
				pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			}
		
			m_selectionMode = str;
		}
		pclSheet->SetStaticText( CD_Mode, lRow, str );
		pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSelectedDpSensor->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
		pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSelectedDpSensor->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	}

	if( NULL != pclDpSensor )
	{
		// HYS-2059
		if( 0 != pclDpSensor->GetBurstPressure() )
		{
			// Burst pressure %1
			CString strBurstPressure = CteEMPTY_STRING;
			str = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true );
			FormatString( strBurstPressure, IDS_TALINK_BURSTPRESSURE, str );

			str = pclDpSensor->GetFullName() + _T( " ; " ) + strBurstPressure;
		}
		else
		{
			str = pclDpSensor->GetFullName();
		}

		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, str );

		if( false == pclDpSensor->IsPartOfaSet() )
		{
			// Dpl if exists.
			pclSheet->SetStaticText( CD_Prim_Product_DpCDpLr, lRow, pclDpSensor->GetFormatedDplRange().c_str() );
		}
	}
	else
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	}

	// Remark: 'true' to exclude any connection reference in the article number.
	str = pclProduct->GetArtNum( true );
	FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, str );

	double dQuantity = 1;

	if( NULL != pclSSel )
	{
		dQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
	}

	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedReturnValveRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CDB_RegulatingValve *pReturnValve = NULL;
	CDS_SSelRadSet *pclRadset = NULL;

	if( NULL == pclSSel || NULL == pclProduct || NULL == pclSheet )
	{
		return;
	}

	if( NULL != dynamic_cast<CDB_RegulatingValve *>( pclProduct ) )
	{
		pReturnValve = (CDB_RegulatingValve *)pclProduct;
		pclRadset = dynamic_cast<CDS_SSelRadSet *>( pclSSel );
	}

	if( NULL == pReturnValve || NULL == pclRadset )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclRadset->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclRadset->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetStaticText( CD_Prim_Infos, lRow, IDS_SSHEETSSELTRV_RVGROUP );
	pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclProduct->GetSize() );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclProduct->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclProduct->GetVersion() );

	CString str = pclProduct->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclProduct->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

	if( false == pclRadset->IsFromDirSel() )
	{
		double dQ = pclRadset->GetQ();
		double dRho = pclRadset->GetpSelectedInfos()->GetpWCData()->GetDens();
		double dNu = pclRadset->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
	
		CString str2;
		CString str1;
		CString strPowerDt = CteEMPTY_STRING;

		if( CDS_SelProd::efdPower == pclRadset->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclRadset->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclRadset->GetDT(), true );
			pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
		}
		
		// Compute Dp.
		pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pclRadset->GetReturnValveDp(), true ) );
		
		if( RadiatorReturnValveMode::RRVM_IMI == pclRadset->GetReturnValveMode() )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );
			str1 = str2 + (CString)_T(" = ");

			CDB_ValveCharacteristic *pclValveCharacteristic = pReturnValve->GetValveCharacteristic();

			if( NULL != pclValveCharacteristic )
			{
				str1 += pclValveCharacteristic->GetSettingString( pclRadset->GetReturnValveOpening() );
			}
			else
			{
				str1 += _T("-");
			}

			pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str1 );
		}
	}
	
	FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, pclProduct->GetArtNum() );
	
	double dQuantity = pclRadset->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );

	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillAccessories( CSSheet *pclSheet, long *plRow, CAccessoryList *pclAccessoryList, int iGroupQuantity, CString strInfos, 
		CDS_SSel *pclSSel, bool bForSet, int iDistributedQty )
{
	if( NULL == pclSheet || NULL == pclAccessoryList || 0 == pclAccessoryList->GetCount() || NULL == pclSSel )
	{
		return;
	}

	CRank rkList;
	CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();
	long lRow = *plRow;

	while( rAccessoryItem.IDPtr.MP != NULL )
	{
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
		VERIFY( NULL != pclAccessory );

		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			break;
		}

		CAccessoryList::AccessoryItem *pclAccessoryItem = new CAccessoryList::AccessoryItem();

		if( NULL == pclAccessoryItem )
		{
			break;
		}

		*pclAccessoryItem = rAccessoryItem;
		int iSortType;
		double dKey;
		FillAccessoriesHelper_GetSortTypeAndKey( pclAccessory, pclSSel, iSortType, dKey );

		if( SortType_Name == iSortType )
		{
			rkList.AddStrSort( pclAccessory->GetName(), dKey, (LPARAM)pclAccessoryItem, false, false );
		}
		else if( SortType_Key == iSortType )
		{
			rkList.Add( pclAccessory->GetName(), dKey, (LPARAM)pclAccessoryItem, false, false );
		}
		else
		{
			ASSERT( 0 );
		}

		rAccessoryItem = pclAccessoryList->GetNext();
	}

	CString str;
	LPARAM lParam;

	for( BOOL bContinue = rkList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rkList.GetNext( str, lParam ) )
	{
		CAccessoryList::AccessoryItem *pclAccessoryItem = ( CAccessoryList::AccessoryItem * )lParam;
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
		ASSERT( NULL != pclAccessory );

		bool bByPair = pclAccessoryItem->fByPair;
		bool bDistributed = pclAccessoryItem->fDistributed;

		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			continue;
		}

		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );

		int iTotalQuantity = 1;
		
		// HYS-987: To handle editable accessory quantity
		if( -1 != pclAccessoryItem->lEditedQty )
		{
			iTotalQuantity = pclAccessoryItem->lEditedQty;
		}
		else if( true == bByPair )
		{
			iTotalQuantity = 2;
			iTotalQuantity *= iGroupQuantity;
		}
		else if( iDistributedQty > 1 && true == bDistributed )
		{
			iTotalQuantity = iDistributedQty;
			iTotalQuantity *= iGroupQuantity;
		}
		else
		{
			iTotalQuantity *= iGroupQuantity;
		}
		
		delete pclAccessoryItem;
		
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		pclSheet->SetStaticText( CD_Prim_Infos, lRow, strInfos ); 
		
		CString strName;
		CString strDescription;
		FillAccessoriesHelper_GetName( pclAccessory, pclSSel, strName, strDescription );

		CString strAccessoryInfos = strName + _T("; ") + strDescription;
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, strAccessoryInfos );

		if( ( pclAccessory->GetPmaxmax() < DBL_MAX  && pclAccessory->GetPmaxmax() >= 0 ) && ( pclAccessory->GetTmin() > -273.15 || pclAccessory->GetTmax() < DBL_MAX ) )
		{
			CString str = pclAccessory->GetPN().c_str();
			str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
			str = str + pclAccessory->GetTempRange( true );
			pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
		}
		
		// HYS-2007.
		if( NULL != dynamic_cast<CDB_DpSensor*>(pclAccessory) )
		{
			CDB_DpSensor* pclDpSensor = dynamic_cast<CDB_DpSensor*>(pclAccessory);
			pclSheet->SetStaticText( CD_Prim_Product_DpCDpLr, lRow, pclDpSensor->GetFormatedDplRange( true ).c_str() );
		}

		CString strArtNum = _T("");

		if( false == bForSet )
		{
			strArtNum = pclAccessory->GetArtNum();
		}

		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclAccessory, strArtNum );
		
		pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( iTotalQuantity, 1 ) );
		lRow++;
	}
	
	*plRow = lRow;
}

long CContainerForExcelExport::FillAccessoryDpSensorSet( CSSheet* pclSheet, long lRow, CDB_Product* pAccessory, bool fWithArticleNumber, int iQuantity, CString strInfos, 
											  LPARAM lpParam )
{
	if( NULL == pclSheet || NULL == pAccessory || false == pAccessory->IsAnAccessory() )
	{
		return lRow;
	}


	pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );

	int iTotalQuantity = 1;
		
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSheet->SetStaticText( CD_Prim_Infos, lRow, strInfos ); 
		
	CString strName;
	CString strDescription;
	strName = pAccessory->GetName();
	strDescription = pAccessory->GetComment();

	if( NULL != dynamic_cast<CDB_DpSensor*>(pAccessory) )
	{
		// Name with Dp range.
		CDB_DpSensor* pclDpSensor = (CDB_DpSensor*)(pAccessory);
		strName = pclDpSensor->GetFullName();

		// Add burst pressure: %1.
		FormatString( strDescription, IDS_TALINK_BURSTPRESSURE, WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true ) );
		
		// DpL range
		pclSheet->SetStaticText( CD_Prim_Product_DpCDpLr, lRow, pclDpSensor->GetFormatedDplRange(true).c_str() );
	}

	CString strAccessoryInfos = strName + _T("; ") + strDescription;
	pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, strAccessoryInfos );

	if( ( pAccessory->GetPmaxmax() < DBL_MAX  && pAccessory->GetPmaxmax() >= 0 ) && ( pAccessory->GetTmin() > -273.15 || pAccessory->GetTmax() < DBL_MAX ) )
	{
		CString str = pAccessory->GetPN().c_str();
		str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
		str = str + pAccessory->GetTempRange( true );
		pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
	}
		
	CString strArtNum = pAccessory->GetArtNum();

	if( false == fWithArticleNumber )
	{
		strArtNum = _T("-");
	}

	FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pAccessory, strArtNum );
		
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( iTotalQuantity, 1 ) );
	lRow++;
	return lRow;
}

double CContainerForExcelExport::FillAccessoriesHelper_GetSortTypeAndKey( CDB_Product *pclProduct, CDS_SSel *pclSSel, int &iSortType, double &dKey )
{
	if( NULL == pclProduct || NULL == pclSSel )
	{
		return 0.0;
	}

	iSortType = SortType_Name;
	dKey = 0.0;

	if( NULL != dynamic_cast<CDS_SSelSmartDpC *>( pclSSel ) )
	{
		iSortType = SortType_Key;
		dKey = (double)pclProduct->GetSortingKey();

		if( NULL != dynamic_cast<CDB_DpSensor *>( pclProduct ) )
		{
			CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProduct );

			ASSERT( ( pclDpSensor->GetMinMeasurableDp() / 1000.0 ) < 100.0 );
			dKey = dKey * 100.0 + ( pclDpSensor->GetMinMeasurableDp() / 1000.0 );
			dKey += 1e9;
		}
	}

	return dKey;
}

void CContainerForExcelExport::FillAccessoriesHelper_GetName( CDB_Product *pclProduct, CDS_SSel *pclSSel, CString &strName, CString &strDescription )
{
	strName = _T("");
	strDescription = _T("");

	if( NULL == pclProduct || NULL == pclSSel )
	{
		return;
	}

	strName = pclProduct->GetName();
	strDescription = pclProduct->GetComment();

	if( ( NULL != dynamic_cast<CDS_SSelSmartDpC *>( pclSSel ) || NULL != dynamic_cast<CDS_SSelDpSensor *>( pclSSel ) ) 
		&& NULL != dynamic_cast<CDB_DpSensor *>( pclProduct ) )
	{
		// Name with Dp range.
		CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProduct );
		strName = pclDpSensor->GetFullName();

		// Add burst pressure: %1.
		FormatString( strDescription, IDS_TALINK_BURSTPRESSURE, WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true ) );
	}
}

void CContainerForExcelExport::FillAccessoriesCtrlSet( CSSheet *pclSheet, long* plRow, CDS_SSelCtrl* pclSSelCtrl, CString strInfos, bool fForCtrl )
{
	long lRow = *plRow;

	if( NULL == pclSheet || NULL == pclSSelCtrl )
	{
		return;
	}

	CAccessoryList *pclAccessoryList = ( true == fForCtrl ) ? pclSSelCtrl->GetCvAccessoryList() : pclSSelCtrl->GetActuatorAccessoryList();

	int iCount = pclAccessoryList->GetCount();

	if( iCount > 0 )
	{
		CRank rkList;
		CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();
		
		while( rAccessoryItem.IDPtr.MP != NULL )
		{
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
			VERIFY( pclAccessory != NULL );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				break;
			}

			CAccessoryList::AccessoryItem *pclAccessoryItem = new CAccessoryList::AccessoryItem();

			if( NULL == pclAccessoryItem )
			{
				break;
			}

			*pclAccessoryItem = rAccessoryItem;
			rkList.AddStrSort( pclAccessory->GetName(), 0, (LPARAM)pclAccessoryItem, false, false );
			rAccessoryItem = pclAccessoryList->GetNext();
		}

		CString str;
		LPARAM lParam;

		CDB_Set *pCvActSet = dynamic_cast<CDB_Set *>( pclSSelCtrl->GetCvActrSetIDPtr().MP );
		CDB_RuledTable *pclAccessoryGroupInSet = NULL;
		std::set<CDB_Product *> setCVAccessorySetList;

		if( NULL != pCvActSet )
		{
			pclAccessoryGroupInSet = dynamic_cast<CDB_RuledTable *>( pCvActSet->GetAccGroupIDPtr().MP );

			if( NULL == pclAccessoryGroupInSet )
			{
				// Particular case for FUSION-C and FUSION-P in package selection mode. When we have no accessory group IDPTR defined in 'CDB_Set', 
				// we take an union between accessories belonging to valve and actuators (see for example code in 'CRViewSSelBCv::GetAdapterList').
				CDB_ControlValve *pTApCV = pclSSelCtrl->GetProductAs<CDB_ControlValve>();
				if( pTApCV != NULL && true == pclSSelCtrl->IsSelectedAsAPackage() &&
					( 0 == CString( pTApCV->GetFamilyID() ).Compare( _T("FAM_FUSION_C") ) ||
					0 == CString( pTApCV->GetFamilyID() ).Compare( _T("FAM_FUSION_P") ) ) )
				{
					CSelProdPageCtrl *pclPageCtrl = new CSelProdPageCtrl( NULL );
					pclPageCtrl->GetFUSIONAccessories( pTApCV, &setCVAccessorySetList );
					delete pclPageCtrl;
					pclPageCtrl = NULL;
				}
			}
		}

		for( BOOL fContinue = rkList.GetFirst( str, lParam ); TRUE == fContinue; fContinue = rkList.GetNext( str, lParam ) )
		{
			CAccessoryList::AccessoryItem *pclAccessoryItem = (CAccessoryList::AccessoryItem *)lParam;
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
			bool bByPair = pclAccessoryItem->fByPair;

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				continue;
			}

			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );

			// If accessory is included into CvActSet, mask his article number.
			bool bWithArticleNumber = true;

			if( true == fForCtrl )
			{
				// Don't show adapter article number if it belongs to a set.
				if( ( NULL != pclAccessoryGroupInSet && _T('\0') != *pclAccessoryGroupInSet->Get( pclAccessory->GetIDPtr().ID ).ID )
						|| ( setCVAccessorySetList.size() > 0 && setCVAccessorySetList.count( pclAccessory ) > 0 ) )
				{
					bWithArticleNumber = false;
				}
			}

			int iQuantity = pclSSelCtrl->GetpSelectedInfos()->GetQuantity();
			
			// HYS-987: Handle editable accessory quantity
			if( -1 != pclAccessoryItem->lEditedQty )
			{
				iQuantity = pclAccessoryItem->lEditedQty;
			}
			else if( true == bByPair )
			{
				iQuantity *= 2;
			}

			delete pclAccessoryItem;
			
			pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			pclSheet->SetStaticText( CD_Prim_Infos, lRow, strInfos );
			
			CString strAccessoryInfos = (CString)pclAccessory->GetName() + _T(" ; ") + pclAccessory->GetComment();
			pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, strAccessoryInfos );

			if( ( pclAccessory->GetPmaxmax() < DBL_MAX  && pclAccessory->GetPmaxmax() >= 0 ) && ( pclAccessory->GetTmin() > -273.15 || pclAccessory->GetTmax() < DBL_MAX ) )
			{
				CString str = pclAccessory->GetPN().c_str();
				str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
				str = str + pclAccessory->GetTempRange( true );
				pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
			}
			
			if( false == bWithArticleNumber )
			{
				CString strArticleNumber = _T("-");

				if( true == pclAccessory->IsDeleted() )
				{
					strArticleNumber += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_TAPDELETED );
				}
				else if( false == pclAccessory->IsAvailable() )
				{
					strArticleNumber += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
				}

				pclSheet->SetStaticText( CD_Prim_ArticleNumber, lRow, strArticleNumber );
			}
			else
			{
				FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclAccessory, pclAccessory->GetArtNum() );
			}
			
			pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( iQuantity, 1 ) );
			lRow++;
		}
	}
	
	*plRow = lRow;
}

void CContainerForExcelExport::FillAccessoriesDpCBCV( CSSheet *pclSheet, long *plRow, CDS_SSelDpCBCV *pclSSelDpCBCV, CString strInfos, bool fForValve )
{
	if( NULL == pclSheet ||  NULL == pclSSelDpCBCV )
	{
		return;
	}

	long lRow = *plRow;
	CAccessoryList *pclAccessoryList = ( true == fForValve ) ? pclSSelDpCBCV->GetCvAccessoryList() : pclSSelDpCBCV->GetActuatorAccessoryList();
	int iCount = pclAccessoryList->GetCount();

	if( iCount > 0 )
	{
		CRank rkList;
		CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

		while( rAccessoryItem.IDPtr.MP != NULL )
		{
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
			VERIFY( pclAccessory != NULL );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				break;
			}

			CAccessoryList::AccessoryItem *pclAccessoryItem = new CAccessoryList::AccessoryItem();

			if( NULL == pclAccessoryItem )
			{
				break;
			}

			*pclAccessoryItem = rAccessoryItem;
			rkList.AddStrSort( pclAccessory->GetName(), 0, (LPARAM)pclAccessoryItem, false, false );
			rAccessoryItem = pclAccessoryList->GetNext();
		}

		CString str;
		LPARAM lParam;

		for( BOOL fContinue = rkList.GetFirst( str, lParam ); TRUE == fContinue; fContinue = rkList.GetNext( str, lParam ) )
		{
			CAccessoryList::AccessoryItem *pclAccessoryItem = (CAccessoryList::AccessoryItem *)lParam;
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
			bool bByPair = pclAccessoryItem->fByPair;

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				continue;
			}

			// If accessory is included into DpCBCVSet, mask his article number.
			bool bWithArticleNumber = true;

			if( true == fForValve )
			{
				// TODO
			}

			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );

			int iQuantity = pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity();
			
			// HYS-987: Handle Editable accessory quantity
			if( pclAccessoryItem->lEditedQty != -1 )
			{
				iQuantity = pclAccessoryItem->lEditedQty;
			}
			else if( true == bByPair )
			{
				iQuantity *= 2;
			}

			delete pclAccessoryItem;
			
			pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			pclSheet->SetStaticText( CD_Prim_Infos, lRow, strInfos );
			CString strAccessoryInfos = (CString)pclAccessory->GetName() + _T(" ; ") + pclAccessory->GetComment();
			pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, strAccessoryInfos );

			if( ( pclAccessory->GetPmaxmax() < DBL_MAX  && pclAccessory->GetPmaxmax() >= 0 ) && ( pclAccessory->GetTmin() > -273.15 || pclAccessory->GetTmax() < DBL_MAX ) )
			{
				CString str = pclAccessory->GetPN().c_str();
				str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
				str = str + pclAccessory->GetTempRange( true );
				pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
			}
			
			if( false == bWithArticleNumber )
			{
				CString strArticleNumber = _T("-");

				if( true == pclAccessory->IsDeleted() )
				{
					strArticleNumber += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_TAPDELETED );
				}
				else if( false == pclAccessory->IsAvailable() )
				{
					strArticleNumber += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
				}

				pclSheet->SetStaticText( CD_Prim_ArticleNumber, lRow, strArticleNumber );
			}
			else
			{
				FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclAccessory, pclAccessory->GetArtNum() );
			}
			
			pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( iQuantity, 1 ) );
			lRow++;
		}
	}

	*plRow = lRow;
}

void CContainerForExcelExport::FillSelected6WayValveRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CDS_SSel6WayValve *pcl6wCv = NULL;
	CString str;

	if( NULL == pclSSel || NULL == pclSheet || NULL == pclProduct )
	{
		return;
	}

	if( NULL != dynamic_cast<CDB_6WayValve*>( pclProduct ) )
	{
		pcl6wCv = dynamic_cast<CDS_SSel6WayValve*>( pclSSel );
	}

	if( NULL == pcl6wCv )
	{
		return;
	}

	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}

		m_selectionMode = str;
	}

	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	// HYS-1877: For Set product we display set name.
	if( true == pcl6wCv->IsSelectedAsAPackage( true ) )
	{
		CDB_Set* pclSet = dynamic_cast<CDB_Set*>(pcl6wCv->GetCvActrSetIDPtr().MP);
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclSet->GetName() );
	}
	else
	{
		pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclProduct->GetName() );
	}
	pclSheet->SetStaticText( CD_Prim_Product_BodyMaterial, lRow, pclProduct->GetBodyMaterial() );

	str = CteEMPTY_STRING;

	if( NULL == pclProduct->GetValveCharDataPointer() && -1.0 != ( (CDB_ControlValve *)( pclProduct ) )->GetKvs() )
	{
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

		if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KVS );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CV );
		}

		str += CString( _T( " = " ) ) + WriteCUDouble( _C_KVCVCOEFF, ( (CDB_ControlValve *)( pclProduct ) )->GetKvs() );
	}

	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, str );
	// Connection 
	CString str1, str2;
	// HYS-1884: Do not use any more GetConnect2(). We have now only one colum. 
	str = pclProduct->GetConnect();
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclProduct->GetVersion() );

	str = pclProduct->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclProduct->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );
	str = CteEMPTY_STRING;
	str1 = CteEMPTY_STRING;
	str2 = CteEMPTY_STRING;
	if( false == pcl6wCv->IsFromDirSel() )
	{
		str1 = WriteCUDouble( _U_FLOW, pcl6wCv->GetCoolingFlow(), true );
		str2 = WriteCUDouble( _U_FLOW, pcl6wCv->GetHeatingFlow(), true );
		str = str1 + _T( " / " ) + str2;
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, str );

		CString strPowerDt = CteEMPTY_STRING;

		if( CDS_SelProd::efdPower == pcl6wCv->GetFlowDef() )
		{
			CString strPowerDt1 = WriteCUDouble( _U_TH_POWER, pcl6wCv->GetCoolingPower(), true );
			strPowerDt += strPowerDt1;
			strPowerDt += _T( " / " );
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pcl6wCv->GetCoolingDT(), true );
			CString strPowerDt2 = WriteCUDouble( _U_TH_POWER, pcl6wCv->GetHeatingPower(), true );
			strPowerDt += _T( " - " ) + strPowerDt2;
			strPowerDt += _T( " / " );
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pcl6wCv->GetHeatingDT(), true );
			pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );
		}

		str = CteEMPTY_STRING;
		str1 = CteEMPTY_STRING;
		str2 = CteEMPTY_STRING;
		// Dp in cooling mode.
		str1 = TASApp.LoadLocalizedString( IDS_SSHETSELPROD_6WAYVALVE_6WAYVALVEDPCOOLING ) + _T(" ");
		double dDp = CalcDp( pcl6wCv->GetFlow( CoolingSide ), ( ( CDB_6WayValve* )pclProduct)->GetKvs(), pcl6wCv->GetWC( CoolingSide ).GetDens() );

		if( -1.0 != dDp )
		{
			str1 += WriteCUDouble( _U_DIFFPRESS, dDp, true );
		}
		else
		{
			str1 += _T( "-" );
		}


		// Dp in heating mode.
		str2 = TASApp.LoadLocalizedString( IDS_SSHETSELPROD_6WAYVALVE_6WAYVALVEDPHEATING ) + _T(" ");
		dDp = CalcDp( pcl6wCv->GetFlow( HeatingSide ), ( (CDB_6WayValve*)pclProduct )->GetKvs(), pcl6wCv->GetWC( HeatingSide ).GetDens() );

		if( -1.0 != dDp )
		{
			str2 += WriteCUDouble( _U_DIFFPRESS, dDp, true );
		}
		else
		{
			str2 += _T( "-" );
		}
		str = str1 + _T( " / " ) + str2;


		pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, str );
		str = _T( "-" );

		pclSheet->SetStaticText( CD_Prim_TechInfo_Setting, lRow, str );
	}

	str = _T( "-" );

	// HYS-1877: For Set product we display set article number.
	if( true == pcl6wCv->IsSelectedAsAPackage( true ) )
	{
		CDB_Set* pclSet = dynamic_cast<CDB_Set*>(pcl6wCv->GetCvActrSetIDPtr().MP);
		FillSetArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclSet, pclSet->GetReference() );
	}
	else
	{
		// Remark: 'true' to exclude any connection reference in the article number.
		str = pclProduct->GetArtNum( true );

		FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclProduct, str );
	}
	double dQuantity = pcl6wCv->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );

	// Secondary.
	CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( pcl6wCv->GetpActuator() );

	if( NULL != pclActuator )
	{
		pclSheet->SetStaticText( CD_Second_Product_Name, lRow, pclActuator->GetName() );
		CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( ( (CDB_ControlValve *)pclProduct )->GetCloseOffCharIDPtr().MP );

		if( NULL != pCloseOffChar )
		{
			str = CteEMPTY_STRING;

			if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CLOSEOFFDP );
				double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );

				if( dCloseOffDp > 0.0 )
				{
					str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
				}
			}
			else if( CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXINLETPRESSURE );
				double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclActuator->GetMaxForceTorque() );

				if( dMaxInletPressure > 0.0 )
				{
					str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
				}
			}

			pclSheet->SetStaticText( CD_Second_Product_ActCloseOffDp, lRow, str );
		}

		// Compute Actuating time in sec.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclActuator->GetActuatingTimesStr( ( (CDB_ControlValve *)pclProduct )->GetStroke(), true );
		pclSheet->SetStaticText( CD_Second_Product_ActActTime, lRow, str );

		// Write IP.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
		str += CString( _T(" : ") ) + pclActuator->GetIPxxAuto();

		if( false == pclActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T( "(" ) + pclActuator->GetIPxxManual() + _T( ")" );
		}

		pclSheet->SetStaticText( CD_Second_Product_ActIP, lRow, str );
		pclSheet->SetStaticText( CD_Second_Product_ActPowerSupply, lRow, pclActuator->GetPowerSupplyStr() );
		pclSheet->SetStaticText( CD_Second_Product_ActInputSignal, lRow, pclActuator->GetInOutSignalsStr( true ) );
		pclSheet->SetStaticText( CD_Second_Product_ActOutputSignal, lRow, pclActuator->GetInOutSignalsStr( false ) );
		pclSheet->SetStaticText( CD_Second_Product_ActRelayType, lRow, pclActuator->GetRelayStr() );
		str.Empty();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

		if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn)
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_NO );
		}

		pclSheet->SetStaticText( CD_Second_Product_ActFailSafe, lRow, str );
		CString strPositionTitle = ( (int)pclActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
		str = strPositionTitle + CString( _T(" : ") ) + pclActuator->GetDefaultReturnPosStr( pclActuator->GetDefaultReturnPos() ).c_str();
		pclSheet->SetStaticText( CD_Second_Product_ActDRP, lRow, str );

		if( true == pcl6wCv->IsSelectedAsAPackage( true ) )
		{
			pclSheet->SetStaticText( CD_Second_ArticleNumber, lRow, _T( "-" ) );
		}
		else
		{
			FillArtNumberCol( pclSheet, CD_Second_ArticleNumber, lRow, pclActuator, pclActuator->GetArtNum() );
		}

		double dQuantity = pcl6wCv->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_Second_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	}

	lRow++;
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedSmartControlValveRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CString str;

	if( NULL == pclSSel || NULL == dynamic_cast<CDS_SSelSmartControlValve *>( pclSSel ) || NULL == pclProduct || NULL == pclSheet 
			|| NULL == dynamic_cast<CDB_SmartControlValve *>( pclProduct ) )
	{
		ASSERT_RETURN;
	}

	CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( pclProduct );
	CDS_SSelSmartControlValve *pclSelectedSmartControlValve = (CDS_SSelSmartControlValve *)( pclSSel );

	if( NULL == pclSmartControlValve->GetSmartValveCharacteristic() )
	{
		ASSERT_RETURN;
	}

	if( true == pclSSel->IsFromDirSel() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TABDIRSEL );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( m_selectionMode != str )
	{
		if( m_selectionMode != CteEMPTY_STRING )
		{
			lRow++;
			pclSheet->SetBackColor( CD_Prim_Ref1, lRow - 1, CD_Pointer, lRow - 1, _VLIGHTYELLOW );
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		}
		
		m_selectionMode = str;
	}
	
	pclSheet->SetStaticText( CD_Mode, lRow, str );
	pclSheet->SetStaticText( CD_Prim_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_Prim_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetStaticText( CD_Prim_Product_Name, lRow, pclSmartControlValve->GetName() );
	pclSheet->SetStaticText( CD_Prim_Product_BodyMaterial, lRow, pclSmartControlValve->GetBodyMaterial() );
	pclSheet->SetStaticText( CD_Prim_Product_Size, lRow, pclSmartControlValve->GetSize() );
	pclSheet->SetStaticText( CD_Prim_Product_Connection, lRow, pclSmartControlValve->GetConnect() );
	pclSheet->SetStaticText( CD_Prim_Product_Version, lRow, pclSmartControlValve->GetVersion() );
	
	str = pclSmartControlValve->GetPN().c_str();
	str = TASApp.LoadLocalizedString( IDS_PN ) + str + _T("; ");
	str = str + pclSmartControlValve->GetTempRange( true );
	pclSheet->SetStaticText( CD_Prim_Product_PnTminTmax, lRow, str );

	pclSheet->SetStaticText( CD_Prim_Product_Kvs, lRow, WriteCUDouble( _C_KVCVCOEFF, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs() ) );
	pclSheet->SetStaticText( CD_Prim_Product_Qnom, lRow, WriteCUDouble( _U_FLOW, pclSmartControlValve->GetSmartValveCharacteristic()->GetQnom(), true ) );
	pclSheet->SetStaticText( CD_Prim_Product_CableLengthToRemoteTempSensor, lRow, WriteCUDouble( _U_LENGTH, pclSmartControlValve->GetRemoteTempSensorCableLength(), true ) );
	
	// HYS-1660
	// Write power supply.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) + CString( _T(" : ") ) + pclSmartControlValve->GetPowerSupplyStr();
	pclSheet->SetStaticText( CD_Prim_Product_TASmartPowerSupply, lRow, str );

	// Write input signal.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_INPUT ) + CString( _T(" : ") ) + pclSmartControlValve->GetInOutSignalsStr( true );
	pclSheet->SetStaticText( CD_Prim_Product_TASmartInputSignal, lRow, str );

	// Write output signal.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_OUTPUTSIG );
	if( CString( _T("") ) == pclSmartControlValve->GetInOutSignalsStr( false ) )
	{
		str += CString( _T(" :    -") );
	}
	else
	{
		str += CString( _T(" : ") ) + pclSmartControlValve->GetInOutSignalsStr( false );
	}

	pclSheet->SetStaticText( CD_Prim_Product_TASmartOutputSignal, lRow, str );

	if( false == pclSSel->IsFromDirSel() )
	{
		double dQ = pclSSel->GetQ();
		pclSheet->SetStaticText( CD_Prim_TechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );

		CString strPowerDt = CteEMPTY_STRING;

		if( CDS_SelProd::efdPower == pclSSel->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSel->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSel->GetDT(), true );
		}
		
		pclSheet->SetStaticText( CD_Prim_TechInfo_PowerDt, lRow, strPowerDt );

		pclSheet->SetStaticText( CD_Prim_TechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedSmartControlValve->GetDp() ) );
		
		// HYS-1914: It's here the Dp min (Kvs is the Kv at full opening).
		double dDpMin = CalcDp( pclSSel->GetQ(), pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs(), pclSSel->GetpSelectedInfos()->GetpWCData()->GetDens() );
		pclSheet->SetStaticText( CD_Prim_TechInfo_DpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDpMin, true ) );

		if( true == pclSelectedSmartControlValve->IsCheckboxDpMaxChecked() && pclSelectedSmartControlValve->GetDpMax() > 0.0 )
		{
			pclSheet->SetStaticText( CD_Prim_TechInfo_DpMax, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedSmartControlValve->GetDpMax(), true ) );
		}
	}

	FillArtNumberCol( pclSheet, CD_Prim_ArticleNumber, lRow, pclSmartControlValve, pclProduct->GetArtNum( true ) );
	
	double dQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_Prim_Qty, lRow, WriteDouble( dQuantity, 1 ) );
	lRow++;
	*plRow = lRow;
}

bool CContainerForExcelExport::InitProjectPMWQExport( CSSheet *pclSheet )
{
	if( NULL == pclSheet )
	{
		return false;
	}
	
	pclSheet->SetBool( SSB_REDRAW, FALSE );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSheet->FormatStaticText( -1, -1, -1, -1, CteEMPTY_STRING );

	// Increase ROW height.
	double RowHeight = 12.75;
	pclSheet->SetRowHeight( RD_Header_PMWQ_GroupName, RowHeight * 1.5 );
	pclSheet->SetRowHeight( RD_Header_PMWQ_ColName, RowHeight * 3 );

	// Resize column.
	SetAllColumnWidth( pclSheet, CD_PMWQ_Ref1, CD_PMWQ_LAST, RD_Header_PMWQ_ColName, 20 );

	// Initialize sheet.
	InitPMWQProjectSheet( pclSheet );

	// Something for individual or direct selection ?
	CTable *pclPMTable = (CTable *)( TASApp.GetpTADS()->Get( _T("PRESSMAINT_TAB") ).MP );
	CTable *pclAirVentSepTable = (CTable *)( TASApp.GetpTADS()->Get( _T("AIRVENTSEP_TAB") ).MP );
	CTable *pclSafetyValveTable = (CTable *)( TASApp.GetpTADS()->Get( _T("SAFETYVALVE_TAB") ).MP );

	// PM
	CTable *pclTable = ( _T('\0') != *pclPMTable->GetFirst().ID ) ? pclPMTable : NULL;
	bool bPMExists = ( NULL != pclTable );
	int iCount = 0;
	long lRow = RD_Header_PMWQ_FirstCirc;

	if( true == bPMExists )
	{
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
		iCount = pclTable->GetItemCount();
		bool bFound = false;

		// The fist row of group is written or not.
		bool bInitRow = false;

		//Individual selection
		///////////////////////
		for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDS_SSelPMaint *pclSSel = dynamic_cast<CDS_SSelPMaint *>( IDPtr.MP );
			if( NULL == pclSSel )
			{
				continue;
			}

			if( NULL == pclSSel->GetIDPtr().MP )
			{
				continue;
			}

			if( false == pclSSel->IsFromDirSel() )
			{
				if( false == bFound )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, Pressurisation, &lRow );
					bInitRow = true;
					AddLineSeparator( pclSheet, Individual_Sel, &lRow );
					bFound = true;
				}
				
				if( CDS_SSelPMaint::NoPressurization != pclSSel->GetSelectionType() )
				{
					FillSelectionPMRow( pclSSel, pclSheet, &lRow );
				}
				else
				{
					FillSelectedPlenoVentoRow( pclSSel, pclSheet, &lRow );
				}
			}
		}
		
		//Direct selection
		///////////////////////
		bFound = false;
		
		for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDS_SSelPMaint *pclSSel = dynamic_cast<CDS_SSelPMaint *>( IDPtr.MP );

			if( NULL == pclSSel )
			{
				continue;
			}

			if( NULL == pclSSel->GetIDPtr().MP )
			{
				continue;
			}

			if( true == pclSSel->IsFromDirSel() )
			{
				if( false == bInitRow )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, Pressurisation, &lRow );
					bInitRow = true;
				}
				
				if( false == bFound )
				{
					// line seperator between individual and direct selection
					bFound = true;
					AddLineSeparator( pclSheet, Direct_Sel, &lRow );
				}
				
				FillSelectionPMRow( pclSSel, pclSheet, &lRow );
			}
		}
	}

	// Air vent separator.
	pclTable = ( _T('\0') != *pclAirVentSepTable->GetFirst().ID ) ? pclAirVentSepTable : NULL;
	bool bSepExists = ( NULL != pclTable );
	iCount = 0;
	
	if( true == bSepExists )
	{
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
		iCount = pclTable->GetItemCount();
		bool bFound = false;
		
		// The fist row of group is written or not.
		bool bInitRow = false;

		// Individual selection.
		for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDS_SSelAirVentSeparator *pclSSel = dynamic_cast<CDS_SSelAirVentSeparator *>( IDPtr.MP );
			
			if( NULL == pclSSel )
			{
				continue;
			}

			if( NULL == pclSSel->GetIDPtr().MP )
			{
				continue;
			}

			if( false == pclSSel->IsFromDirSel() )
			{
				if( false == bFound )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, SeparatorAirVent, &lRow );
					bInitRow = true;
					AddLineSeparator( pclSheet, Individual_Sel, &lRow );
					bFound = true;
				}
				
				FillSelectedSeparatorRow( pclSSel, pclSheet, &lRow );
			}
		}

		// Direct selection.
		bFound = false;

		for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDS_SSelAirVentSeparator *pclSSel = dynamic_cast<CDS_SSelAirVentSeparator *>( IDPtr.MP );
			
			if( NULL == pclSSel )
			{
				continue;
			}

			if( NULL == pclSSel->GetIDPtr().MP )
			{
				continue;
			}

			if( true == pclSSel->IsFromDirSel() )
			{
				if( false == bInitRow )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, SeparatorAirVent, &lRow );
					bInitRow = true;
				}
				
				if( false == bFound )
				{
					bFound = true;
					AddLineSeparator( pclSheet, Direct_Sel, &lRow );
				}
				
				FillSelectedSeparatorRow( pclSSel, pclSheet, &lRow );
			}
		}
	}

	// Safety valves.
	pclTable = ( _T('\0') != *pclSafetyValveTable->GetFirst().ID ) ? pclSafetyValveTable : NULL;
	bool bSafeValvExists = ( NULL != pclTable );
	iCount = 0;
	
	if( true == bSafeValvExists )
	{
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
		iCount = pclTable->GetItemCount();
		bool bFound = false;
		
		// The fist row of group is written or not.
		bool bInitRow = false;

		// Individual selection.
		for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDS_SSelSafetyValve *pclSSel = dynamic_cast<CDS_SSelSafetyValve *>( IDPtr.MP );

			if( NULL == pclSSel )
			{
				continue;
			}

			if( NULL == pclSSel->GetIDPtr().MP )
			{
				continue;
			}

			if( false == pclSSel->IsFromDirSel() )
			{
				if( false == bFound )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, SafetyValve, &lRow );
					bInitRow = true;
					AddLineSeparator( pclSheet, Individual_Sel, &lRow );
					bFound = true;
				}
				
				FillSelectedSafetyValveRow( pclSSel, pclSheet, &lRow );
			}
		}

		// Direct selection.
		bFound = false;

		// HYS-1741: Add accessories and services product rows
		for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDS_SSelSafetyValve *pclSSelSaftyValve = dynamic_cast<CDS_SSelSafetyValve *>( IDPtr.MP );
			CDS_SelPWQAccServices *pclSSelAccServ = dynamic_cast<CDS_SelPWQAccServices*>( IDPtr.MP );

			if( NULL != pclSSelSaftyValve && NULL != pclSSelSaftyValve->GetIDPtr().MP && true == pclSSelSaftyValve->IsFromDirSel() )
			{
				if( false == bInitRow )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, SafetyValve, &lRow );
					bInitRow = true;
				}

				if( false == bFound )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, Direct_Sel, &lRow );
					bFound = true;
				}

				FillSelectedSafetyValveRow( pclSSelSaftyValve, pclSheet, &lRow );
			}
			else if( NULL != pclSSelAccServ && NULL != pclSSelAccServ->GetIDPtr().MP && true == pclSSelAccServ->IsFromDirSel() )
			{
				if( false == bInitRow )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, SafetyValve, &lRow );
					bInitRow = true;
				}

				if( false == bFound )
				{
					pclSheet->SetMaxRows( pclSheet->GetMaxRows() + iCount );
					AddLineSeparator( pclSheet, Direct_Sel, &lRow );
					bFound = true;
				}

				FillSelectedAccessServicesRow( pclSSelAccServ, pclSheet, &lRow );
			}
		}
	}

	if( lRow <= RD_Header_PMWQ_FirstCirc )
	{
		return false;
	}

	pclSheet->SetMaxRows( lRow - 1 );
	pclSheet->SetCellBorder( CD_PMWQ_Ref1, pclSheet->GetMaxRows(), CD_PMWQ_LAST + 7, pclSheet->GetMaxRows(), true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSheet->SetRowHeight( RD_Header_PMWQ_ColName, RowHeight * 3 );
	
	// Resize all column width.
	SetAllColumnWidth( pclSheet, CD_PMWQ_Sub, CD_PMWQ_LocalArticleNumber, RD_Header_PMWQ_ColName, 20 );
	SetAllColumnWidth( pclSheet, CD_PMWQ_IO_Sep, CD_PMWQ_LAST + 7, RD_Header_PMWQ_ColName, 10 );
	HideEmptyColumns( pclSheet, RD_Header_PMWQ_FirstCirc, 1, CD_PMWQ_LAST + 7 );

	pclSheet->SetBool( SSB_REDRAW, TRUE );
	return true;
}

void CContainerForExcelExport::FillArtNumberCol( CSSheet *pclSheet, long lColumn, long lRow, CDB_Product *pclProduct, CString strArtNumberStr )
{
	if( NULL == pclSheet || NULL == pclProduct )
	{
		return;
	}

	CString str1 = CteEMPTY_STRING;

	if( true == pclProduct->IsDeleted() || false == pclProduct->IsAvailable() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	// Check first if we have a local article number available.
	CString strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclProduct->GetArtNum() );

	if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
	{
		if( true == pclProduct->IsDeleted() )
		{
			str1 = strArtNumberStr + TASApp.LoadLocalizedString( IDS_TAPDELETED );
		}
		else if( false == pclProduct->IsAvailable() )
		{
			str1 = strArtNumberStr + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
		}
		else
		{
			str1 = strArtNumberStr;
		}

		pclSheet->SetStaticText( lColumn, lRow, str1 );
	}

	// Set local article number if allowed and exist.
	if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
	{
		if( true == pclProduct->IsDeleted() )
		{
			str1 = strLocalArticleNumber + TASApp.LoadLocalizedString( IDS_TAPDELETED );
		}
		else if( false == pclProduct->IsAvailable() )
		{
			str1 = strLocalArticleNumber + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
		}
		else
		{
			str1 = strLocalArticleNumber;
		}

		pclSheet->SetStaticText( lColumn + 1, lRow, str1 );
	}
	
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
}

void CContainerForExcelExport::FillSetArtNumberCol( CSSheet *pclSheet, long lColumn, long lRow, CDB_Set *pclSet, CString strArtNumberStr )
{
	if( NULL == pclSheet || NULL == pclSet )
	{
		return;
	}

	CString str1 = CteEMPTY_STRING;

	if( true == pclSet->IsDeleted() || false == pclSet->IsAvailable() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	// Check first if we have a local article number available.
	CString strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclSet->GetReference() );

	if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
	{
		if( true == pclSet->IsDeleted() )
		{
			str1 = strArtNumberStr + TASApp.LoadLocalizedString( IDS_TAPDELETED );
		}
		else if( false == pclSet->IsAvailable() )
		{
			str1 = strArtNumberStr + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
		}
		else
		{
			str1 = strArtNumberStr;
		}

		pclSheet->SetStaticText( lColumn, lRow, str1 );
	}

	// Set local article number if allowed and exist.
	if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
	{
		if( true == pclSet->IsDeleted() )
		{
			str1 = strLocalArticleNumber + TASApp.LoadLocalizedString( IDS_TAPDELETED );
		}
		else if( false == pclSet->IsAvailable() )
		{
			str1 = strLocalArticleNumber + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
		}
		else
		{
			str1 = strLocalArticleNumber;
		}

		pclSheet->SetStaticText( lColumn + 1, lRow, str1 );
	}
	
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
}

void CContainerForExcelExport::InitPMWQProjectSheet( CSSheet *pclSheet )
{
	if( NULL == pclSheet )
	{
		return;
	}

	m_selectionMode = CteEMPTY_STRING;

	pclSheet->SetMaxRows( RD_Header_PMWQ_FirstCirc - 1 );
	pclSheet->SetMaxCols( CD_PMWQ_LAST + 7 );

	CString str = CteEMPTY_STRING;
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Pressurisation and Water Quality selection' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Title.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->AddCellSpanW( CD_PMWQ_Sub, RD_Header_PMWQ_GroupName, CD_PMWQ_Pointer - CD_PMWQ_Sub, 1 );
	pclSheet->SetStaticText( CD_PMWQ_Sub, RD_Header_PMWQ_GroupName, IDS_SSHEET_EXPORT_PMWQ );

	// Columns.
	// Reference 1.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSheet->AddCellSpanW( CD_PMWQ_Ref1, RD_Header_PMWQ_ColName, 1, 2 );
	pclSheet->SetStaticText( CD_PMWQ_Ref1, RD_Header_PMWQ_ColName, _T("#1") );

	// Reference 2.
	pclSheet->AddCellSpanW( CD_PMWQ_Ref2, RD_Header_PMWQ_ColName, 1, 2 );
	pclSheet->SetStaticText( CD_PMWQ_Ref2, RD_Header_PMWQ_ColName, _T("#2") );

	// Quantity.
	pclSheet->AddCellSpanW( CD_PMWQ_Qty, RD_Header_PMWQ_ColName, 1, 2 );
	pclSheet->SetStaticText( CD_PMWQ_Qty, RD_Header_PMWQ_ColName, IDS_SELPHDR_QTY );

	// Product name.
	pclSheet->AddCellSpanW( CD_PMWQ_ProductName, RD_Header_PMWQ_ColName, 1, 2 );
	pclSheet->SetStaticText( CD_PMWQ_ProductName, RD_Header_PMWQ_ColName, IDS_SSHEET_EXPORT_PRODNAME );

	// Product function.
	pclSheet->AddCellSpanW( CD_PMWQ_ProductFunction, RD_Header_PMWQ_ColName, 1, 2 );
	pclSheet->SetStaticText( CD_PMWQ_ProductFunction, RD_Header_PMWQ_ColName, IDS_SSHEET_EXPORT_PRODFUNCT );

	// Article number.
	pclSheet->AddCellSpanW( CD_PMWQ_ArticleNumber, RD_Header_PMWQ_ColName, 1, 2 );
	pclSheet->SetStaticText( CD_PMWQ_ArticleNumber, RD_Header_PMWQ_ColName, IDS_SELPHDR_ART );

	// Local article number for product.
	pclSheet->AddCellSpanW( CD_PMWQ_LocalArticleNumber, RD_Header_PMWQ_ColName, 1, 2 );
	str = CteEMPTY_STRING;

	if( false == TASApp.GetLocalArtNumberName().IsEmpty() )
	{
		str = TASApp.GetLocalArtNumberName();
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_SELPHDR_LOCART );
	}
	
	pclSheet->SetStaticText( CD_PMWQ_LocalArticleNumber, RD_Header_PMWQ_ColName, str );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Pressurisation input data' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Do a first loop on selected products to detect which columns to display.
	// For the moment we have only for the SWKI HE301-01 norm.
	CTable *pclPMTable = (CTable *)( TASApp.GetpTADS()->Get( _T("PRESSMAINT_TAB") ).MP );
	ASSERT( NULL != pclPMTable );

	m_bAtLeastOneCooling = false;
	m_bAtLeastOneSolar = false;
	m_bAtLeastOneSWKINorm = false;
	m_bAtLeastOneNotSWKINorm = false;
	m_bAtLeastOneSWKINormWithStorageTank = false;

	for( IDPTR IDPtr = pclPMTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclPMTable->GetNext( IDPtr.MP ) )
	{
		CDS_SSelPMaint *pclSSel = dynamic_cast<CDS_SSelPMaint *>( IDPtr.MP );
		if( NULL == pclSSel )
		{
			continue;
		}

		if( NULL == pclSSel->GetIDPtr().MP )
		{
			continue;
		}

		if( true == pclSSel->IsFromDirSel() )
		{
			continue;
		}
	
		if( CDS_SSelPMaint::NoPressurization == pclSSel->GetSelectionType() )
		{
			continue;
		}

		if( pclSSel->GetpInputUser()->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
		{
			m_bAtLeastOneSWKINorm = true;

			if( pclSSel->GetpInputUser()->GetStorageTankVolume() > 0.0 )
			{
				m_bAtLeastOneSWKINormWithStorageTank = true;
			}
		}
		else
		{
			m_bAtLeastOneNotSWKINorm = true;
		}

		if( ProjectType::Cooling == pclSSel->GetpInputUser()->GetApplicationType() )
		{
			m_bAtLeastOneCooling = true;
		}
		else if( ProjectType::Solar == pclSSel->GetpInputUser()->GetApplicationType() )
		{
			m_bAtLeastOneSolar = true;
		}
	}

	// Title.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSheet->AddCellSpanW( CD_PMWQ_IO_Sep, RD_Header_PMWQ_GroupName, 1 + CD_PMWQ_IO_MaxHeight - CD_PMWQ_IO_Sep, 1 );
	pclSheet->SetStaticText( CD_PMWQ_IO_Sep, RD_Header_PMWQ_GroupName, IDS_SSHEET_EXPORT_PMINPUTDATA );
	pclSheet->SetBackColor( CD_PMWQ_IO_Sep, RD_Header_PMWQ_GroupName, CD_PMWQ_IO_MaxHeight, RD_Header_PMWQ_GroupName, _WHITE );

	// Columns name.

	// Application type.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_APPTYPE );
	pclSheet->SetStaticText( CD_PMWQ_IO_AppType, RD_Header_PMWQ_ColName, str );

	// Fluid type.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_WATERTYPE );
	pclSheet->SetStaticText( CD_PMWQ_IO_FluidType, RD_Header_PMWQ_ColName, str );

	// Norm.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_NORM );
	pclSheet->SetStaticText( CD_PMWQ_IO_Norm, RD_Header_PMWQ_ColName, str );

	// System volume.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SYSTEMVOLUME );
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemVolume, RD_Header_PMWQ_ColName, str );

	// Solar colletor volume [Vkoll].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SOLARCOLLECTORVOLUME );
	pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorVolume, RD_Header_PMWQ_ColName, str );

	// Solar colletor multiplier factor.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SOLARCOLLECTORMULTIPLIERFACTOR );
	pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorMultiplierFactor, RD_Header_PMWQ_ColName, str );

	// Storage tank volume [Vsto].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_STORAGETANKVOLUME );
	pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankVolume, RD_Header_PMWQ_ColName, str );

	// Storage tank max. temperature [Tmax,sto].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_STORAGETANKMAXTEMP );
	pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankMaxTemp, RD_Header_PMWQ_ColName, str );

	// Installed power.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_INSTPOWER );
	pclSheet->SetStaticText( CD_PMWQ_IO_InstPower, RD_Header_PMWQ_ColName, str );

	// Static height.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_STATHEIGHT );
	pclSheet->SetStaticText( CD_PMWQ_IO_StatHeight, RD_Header_PMWQ_ColName, str );

	// Min. required equipment pressure.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_PZ );
	pclSheet->SetStaticText( CD_PMWQ_IO_Pz, RD_Header_PMWQ_ColName, str );

	// Safety valve response pressure.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SAFVALRESPRESS );
	pclSheet->SetStaticText( CD_PMWQ_IO_SafValvPress, RD_Header_PMWQ_ColName, str );

	// Safety temp limiter.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SAFTEMPLIM );
	pclSheet->SetStaticText( CD_PMWQ_IO_SafTempLimit, RD_Header_PMWQ_ColName, str );

	// Max. temperature.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_MAXTEMP );
	pclSheet->SetStaticText( CD_PMWQ_IO_MaxTemp, RD_Header_PMWQ_ColName, str );

	// Supply temp.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SUPPLYTEMP );
	pclSheet->SetStaticText( CD_PMWQ_IO_SupplyTemp, RD_Header_PMWQ_ColName, str );

	// Return temp.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_RETURNTEMP );
	pclSheet->SetStaticText( CD_PMWQ_IO_ReturnTemp, RD_Header_PMWQ_ColName, str );

	// Min. temp.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_MINTEMP );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinTemp, RD_Header_PMWQ_ColName, str );

	// Fill temp.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_FILLTEMP );
	pclSheet->SetStaticText( CD_PMWQ_IO_FillTemp, RD_Header_PMWQ_ColName, str );

	// Pressurisation on.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_PRESSON );
	pclSheet->SetStaticText( CD_PMWQ_IO_PressOn, RD_Header_PMWQ_ColName, str );

	// Pump head.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_PUMPHEAD );
	pclSheet->SetStaticText( CD_PMWQ_IO_PumpHead, RD_Header_PMWQ_ColName, str );

	// Max width.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_MAXWIDTH );
	pclSheet->SetStaticText( CD_PMWQ_IO_MaxWidth, RD_Header_PMWQ_ColName, str );

	// Max height.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_MAXHEIGHT );
	pclSheet->SetStaticText( CD_PMWQ_IO_MaxHeight, RD_Header_PMWQ_ColName, str );

	// Column of units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSheet->SetStaticText( CD_PMWQ_IO_AppType, RD_Header_PMWQ_ColUnit, _T("-") );
	pclSheet->SetStaticText( CD_PMWQ_IO_FluidType, RD_Header_PMWQ_ColUnit, _T("-") );
	pclSheet->SetStaticText( CD_PMWQ_IO_Norm, RD_Header_PMWQ_ColUnit, _T("-") );
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemVolume, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorVolume, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorMultiplierFactor, RD_Header_PMWQ_ColUnit, _T("-") );
	pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankVolume, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankMaxTemp, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_InstPower, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_StatHeight, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_Pz, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_SafValvPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_SafTempLimit, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_MaxTemp, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_SupplyTemp, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_ReturnTemp, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinTemp, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_FillTemp, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_PressOn, RD_Header_PMWQ_ColUnit, _T("-") );
	pclSheet->SetStaticText( CD_PMWQ_IO_PumpHead, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_MaxWidth, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_MaxHeight, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Pressurisation calculated data' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Title.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSheet->AddCellSpanW( CD_PMWQ_IO_SystemExpCoeff, RD_Header_PMWQ_GroupName, CD_PMWQ_IO_Pointer - CD_PMWQ_IO_SystemExpCoeff, 1 );
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpCoeff, RD_Header_PMWQ_GroupName, IDS_SSHEET_EXPORT_PMCALCDATA );
	pclSheet->SetBackColor( CD_PMWQ_IO_SystemExpCoeff, RD_Header_PMWQ_GroupName, CD_PMWQ_IO_ExpPipe30, RD_Header_PMWQ_GroupName, _WHITE );

	// Columns name.

	// System expansion coefficient.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SYSTEMEXPCOEFF );
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpCoeff, RD_Header_PMWQ_ColName, str );

	// System expansion volume [Ve].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SYSTEMEXPVOL );
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpVol, RD_Header_PMWQ_ColName, str );

	// Factor [X].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_XFACTOR );
	pclSheet->SetStaticText( CD_PMWQ_IO_FactorX, RD_Header_PMWQ_ColName, str );

	// System expansin volume [Vs.e.X].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SYSTEMEXPVOLSWKI );
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpVolSWKI, RD_Header_PMWQ_ColName, str );

	// Storage tank expansion [esto].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_STORAGETANKEXPCOEFF );
	pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankExpCoeff, RD_Header_PMWQ_ColName, str );

	// Storage tank expansion volume [Vsto.esto].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_STORAGETANKEXPVOL );
	pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankExpVolume, RD_Header_PMWQ_ColName, str );

	// Total expansion volume [Ve,tot].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_TOTALEXPVOL );
	pclSheet->SetStaticText( CD_PMWQ_IO_TotalExpVolume, RD_Header_PMWQ_ColName, str );

	// Solar collector security volume [VDK].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SOLARCOLLECTORSECURITYVOLUME );
	pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorSecurityVolume, RD_Header_PMWQ_ColName, str );

	// Min. water reserve (To add for expansion) -> Except for SWKI HE301-01 norm.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_MINWATRESTOADDFOREXPANSION );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinWResInExpansion, RD_Header_PMWQ_ColName, str );

	// Degassing water reserve [Vv].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_DEGASSINGWATERESERVE );
	pclSheet->SetStaticText( CD_PMWQ_IO_DegassingWaterReserve, RD_Header_PMWQ_ColName, str );

	// Vessel net volume [Vs].
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_VESSELNETVOLUME );
	pclSheet->SetStaticText( CD_PMWQ_IO_VesselNetVolume, RD_Header_PMWQ_ColName, str );

	// Min. water reserve (Included in expansion) -> For SWKI HE301-01 norm.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_MINWATRESINCLUDEDINEXPANSION );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinWResNotInExpansion, RD_Header_PMWQ_ColName, str );

	// Water reserve.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_WATRES );
	pclSheet->SetStaticText( CD_PMWQ_IO_WaterRes, RD_Header_PMWQ_ColName, str );

	// Contraction volume.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_CONTRVOL );
	pclSheet->SetStaticText( CD_PMWQ_IO_CtrVol, RD_Header_PMWQ_ColName, str );

	// Vapor pressure.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_VAPPRESS );
	pclSheet->SetStaticText( CD_PMWQ_IO_VapPress, RD_Header_PMWQ_ColName, str );

	// Min. pressure p0.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_MINPRESS );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinPress, RD_Header_PMWQ_ColName, str );

	// Min. initial pressure pa min.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_MININITPRESS );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinInitPress, RD_Header_PMWQ_ColName, str );

	// Initial pressure pa.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_INITPRESS );
	pclSheet->SetStaticText( CD_PMWQ_IO_InitPress, RD_Header_PMWQ_ColName, str );

	// Filling pressure.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_FILLINGPRESS );
	pclSheet->SetStaticText( CD_PMWQ_IO_FillPress, RD_Header_PMWQ_ColName, str );

	// Target pressure.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_TARGPRESS );
	pclSheet->SetStaticText( CD_PMWQ_IO_TargetPress, RD_Header_PMWQ_ColName, str );

	// Final pressure.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_FINALPRESS );
	pclSheet->SetStaticText( CD_PMWQ_IO_FinalPress, RD_Header_PMWQ_ColName, str );

	// Specific flow rate of equalization volume.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SPECQRATEEQVOL );
	pclSheet->SetStaticText( CD_PMWQ_IO_SpeFlEqVol, RD_Header_PMWQ_ColName, str );

	// Necessary flow rate of equalization volume.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_NEEDEDQRATEEQVOL );
	pclSheet->SetStaticText( CD_PMWQ_IO_NeFlEqVol, RD_Header_PMWQ_ColName, str );

	// Pressure factor.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_PRESSFACTOR );
	pclSheet->SetStaticText( CD_PMWQ_IO_PressFact, RD_Header_PMWQ_ColName, str );

	// Nominal volume.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_NOMVOL );
	pclSheet->SetStaticText( CD_PMWQ_IO_NomVol, RD_Header_PMWQ_ColName, str );

	// Expansion pipe up to 10.
	CString strLength = WriteCUDouble( _U_LENGTH, 10, true, 0 );
	FormatString( str, IDS_SSHEETSELPROD_CPM_CDDESCR_EXPPIPE, strLength );
	pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe10, RD_Header_PMWQ_ColName, str );

	// Expansion pipe up to 30.
	strLength = WriteCUDouble( _U_LENGTH, 30, true, 0 );
	FormatString( str, IDS_SSHEETSELPROD_CPM_CDDESCR_EXPPIPE, strLength );
	pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe30, RD_Header_PMWQ_ColName, str );

	// Cof units.
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpCoeff, RD_Header_PMWQ_ColUnit, _T("-") );
	
	// System expansion volume [Ve].
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpVol, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	// Factor [X] -> For SWKI HE301-01.
	pclSheet->SetStaticText( CD_PMWQ_IO_FactorX, RD_Header_PMWQ_ColUnit, _T("-") );
	
	// System expansion volume [Vs.e.X] -> For SWKI HE301-01.
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpVolSWKI, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	// Storage tank expansion coefficient [esto].
	pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankExpCoeff, RD_Header_PMWQ_ColUnit, _T("-") );

	// Storage tank expansion volume [Vsto.esto].
	pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankExpVolume, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	// Total expansion volume [Ve,tot].
	pclSheet->SetStaticText( CD_PMWQ_IO_TotalExpVolume, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	// Solar collector security volume [VDK].
	pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorSecurityVolume, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	// Min. water reserve (In expansion) -> Except for SWKI HE301-01 norm.
	pclSheet->SetStaticText( CD_PMWQ_IO_MinWResInExpansion, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	// Degassing water reserve [Vv].
	pclSheet->SetStaticText( CD_PMWQ_IO_DegassingWaterReserve, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	// Vessel net volume [Vn].
	pclSheet->SetStaticText( CD_PMWQ_IO_VesselNetVolume, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	// Min. water reserve (Not in expansion) -> For SWKI HE301-01 norm.
	pclSheet->SetStaticText( CD_PMWQ_IO_MinWResNotInExpansion, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	pclSheet->SetStaticText( CD_PMWQ_IO_WaterRes, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_CtrVol, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_VapPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinInitPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_InitPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_FillPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_TargetPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_FinalPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	
	CString strUnit = CString( pUnitDB->GetNameOfDefaultUnit( _U_FLOW ).c_str() ) + CString( _T("/") ) + CString( pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_SpeFlEqVol, RD_Header_PMWQ_ColUnit, strUnit );
	
	pclSheet->SetStaticText( CD_PMWQ_IO_NeFlEqVol, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_PressFact, RD_Header_PMWQ_ColUnit, _T("-") );
	pclSheet->SetStaticText( CD_PMWQ_IO_NomVol, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe10, RD_Header_PMWQ_ColUnit, _T("-") );
	pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe30, RD_Header_PMWQ_ColUnit, _T("-") );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Separators Technical informations 
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Title.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSheet->AddCellSpanW( CD_PMWQ_SepTechInfo_Sep, RD_Header_PMWQ_GroupName, 1 + CD_PMWQ_SepTechInfo_Dp - CD_PMWQ_SepTechInfo_Sep, 1 );
	pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_Sep, RD_Header_PMWQ_GroupName, IDS_SSHEET_EXPORT_SEPTECINFO );
	pclSheet->SetBackColor( CD_PMWQ_SepTechInfo_Sep, RD_Header_PMWQ_GroupName, CD_PMWQ_SepTechInfo_Dp, RD_Header_PMWQ_GroupName, _YELLOW );

	// Columns name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	
	// Flow.
	pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_Flow, RD_Header_PMWQ_ColName, IDS_SELP_FLOWRATE );

	// Power/DT .
	CString strPowerDTName = TASApp.LoadLocalizedString( IDS_SELP_POWER );
	strPowerDTName += _T("/") + TASApp.LoadLocalizedString( IDS_DT );
	pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_PowerDt, RD_Header_PMWQ_ColName, strPowerDTName );

	// Dp.
	pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_Dp, RD_Header_PMWQ_ColName, IDS_SSHEETSSEL_DP );

	// Column of units.
	pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_Flow, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
	
	strUnit = CString( pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() ) + CString( _T("/") )
		+ CString( pUnitDB->GetNameOfDefaultUnit( _U_DIFFTEMP ).c_str() );
	
	pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_PowerDt, RD_Header_PMWQ_ColUnit, strUnit );
	
	pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_Dp, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Safety valves Technical informations 
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Title.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSheet->AddCellSpanW( CD_PMWQ_SafValTechInfo_Sep, RD_Header_PMWQ_GroupName, CD_PMWQ_LAST - CD_PMWQ_SafValTechInfo_Sep, 1 );
	pclSheet->SetStaticText( CD_PMWQ_SafValTechInfo_Sep, RD_Header_PMWQ_GroupName, IDS_SSHEET_EXPORT_SAFVALTECINFO );
	pclSheet->SetBackColor( CD_PMWQ_SafValTechInfo_Sep, RD_Header_PMWQ_GroupName, CD_PMWQ_SafValechInfo_Collec, RD_Header_PMWQ_GroupName, _YELLOW );

	// Columns name.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	
	// Set Pressure.
	pclSheet->SetStaticText( CD_PMWQ_SafValTechInfo_SetPress, RD_Header_PMWQ_ColName, IDS_SSHEETSSELSAFETYVALVE_SETPRESSURE );

	// Heat generator.
	pclSheet->SetStaticText( CD_PMWQ_SafValTechInfo_HeatGenType, RD_Header_PMWQ_ColName, IDS_SELP_HEATGENERATORTYPE );

	// Max power.
	pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_MaxPower, RD_Header_PMWQ_ColName, IDS_SSHEETSSELSAFETYVALVE_POWER100PC );

	// Power.
	pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_Power, RD_Header_PMWQ_ColName, IDS_SSHEETSSELSAFETYVALVE_POWER );

	// Max collector.
	pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_MaxCollec, RD_Header_PMWQ_ColName, IDS_SSHEETSELPROD_SAFETYVALVE_COLLECTORMAX );

	// Collector.
	pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_Collec, RD_Header_PMWQ_ColName, IDS_SELP_COLLECTOR );

	// Column of units.
	pclSheet->SetStaticText( CD_PMWQ_SafValTechInfo_SetPress, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_SafValTechInfo_HeatGenType, RD_Header_PMWQ_ColUnit, _T("-") );
	pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_MaxPower, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_Power, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_MaxCollec, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_AREA ) ).c_str() );
	pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_Collec, RD_Header_PMWQ_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_AREA ) ).c_str() );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Product parameters
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Title.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSheet->AddCellSpanW( CD_PMWQ_LAST + 1, RD_Header_PMWQ_GroupName, 7, 1 );
	pclSheet->SetStaticText( CD_PMWQ_LAST + 1, RD_Header_PMWQ_GroupName, IDS_SSHEET_EXPORT_PRODPARAM );
	pclSheet->SetBackColor( CD_PMWQ_LAST + 1, RD_Header_PMWQ_GroupName, CD_PMWQ_LAST + 7, RD_Header_PMWQ_GroupName, _WHITE );
	pclSheet->SetCellBorder( CD_PMWQ_LAST + 1, RD_Header_PMWQ_GroupName, CD_PMWQ_LAST + 7, RD_Header_PMWQ_GroupName, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_PMWQ_LAST + 7, RD_Header_PMWQ_GroupName, CD_PMWQ_LAST + 7, RD_Header_PMWQ_FirstCirc - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );

	// Columns name.
	//pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	// Border.
	pclSheet->SetCellBorder( CD_PMWQ_Ref1, RD_Header_PMWQ_GroupName - 1, CD_PMWQ_LAST, RD_Header_PMWQ_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_PMWQ_Ref1, RD_Header_PMWQ_GroupName - 1, CD_PMWQ_LAST, RD_Header_PMWQ_FirstCirc - 1, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_PMWQ_Ref1, RD_Header_PMWQ_GroupName - 1, CD_PMWQ_LAST, RD_Header_PMWQ_FirstCirc - 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( CD_PMWQ_Ref1, RD_Header_PMWQ_GroupName - 1, CD_PMWQ_LAST, RD_Header_PMWQ_FirstCirc - 1, true, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );
}

void CContainerForExcelExport::FillSelectionPMRow( CDS_SSelPMaint *pclSSelPressMaint, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;

	if( NULL == pclSheet || NULL == pclSSelPressMaint )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	CString str;

	pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, pclSSelPressMaint->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );

	pclSheet->SetStaticText( CD_PMWQ_Ref2, lRow, pclSSelPressMaint->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );

	// Fill input data columns.
	FillPMInputDataRow( pclSSelPressMaint, pclSheet, lRow );

	// Fill calculated data columns.
	FillPMCalculatedDataRow( pclSSelPressMaint, pclSheet, lRow );

	pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST, lRow, _VLIGHTYELLOW );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( true == pclSSelPressMaint->IsFromDirSel() )
	{
		FillDirSelRow( pclSSelPressMaint, pclSheet, &lRow );
		*plRow = lRow;
		return;
	}
	
	// Fill Statico.
	if( CDS_SSelPMaint::SelectionType::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		// Fill Statico.
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );

		int iQuantity = pclSSelPressMaint->GetVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );

		CDB_Vessel *pclVessel = (CDB_Vessel *)( pclSSelPressMaint->GetVesselIDPtr().MP );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclVessel->GetName() );
		
		pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclVessel->GetVesselTypeStr( false ) );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclVessel, pclVessel->GetArtNum() );
		FillProductParametersRow( pclVessel, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
		
		// Add accessories.
		if( NULL != pclSSelPressMaint->GetVesselAccessoryList() && pclSSelPressMaint->GetVesselAccessoryList()->GetCount() > 0 )
		{
			lRow++;
			int iDistributedQty = pclSSelPressMaint->GetVesselNumber() + pclSSelPressMaint->GetSecondaryVesselNumber();
			FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
		}

		// Add intermediate vessel if exist.
		IDPTR IntermVesselIDPtr = pclSSelPressMaint->GetIntermediateVesselIDPtr();

		if( _NULL_IDPTR != IntermVesselIDPtr && NULL != dynamic_cast<CDB_Vessel *>( IntermVesselIDPtr.MP ) )
		{
			// Intermediate vessel is in fact a Statico.
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetIntermediateVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

			CDB_Vessel *pclIntermediateVessel = (CDB_Vessel *)( pclSSelPressMaint->GetIntermediateVesselIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclIntermediateVessel->GetName() );

			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclIntermediateVessel->GetVesselTypeStr() );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclIntermediateVessel, pclIntermediateVessel->GetArtNum() );
			FillProductParametersRow( pclIntermediateVessel, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

			// Add accessory if exist.
			if( NULL != pclSSelPressMaint->GetIntermediateVesselAccessoryList() && pclSSelPressMaint->GetIntermediateVesselAccessoryList()->GetCount() > 0 )
			{
				lRow++; 
				int iDistributedQty = pclSSelPressMaint->GetIntermediateVesselNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetIntermediateVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}

		// Fill Vento if exist.
		if( NULL != (CDB_TBPlenoVento *)pclSSelPressMaint->GetTecBoxVentoIDPtr().MP )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetTecBoxVentoNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );
			
			CDB_TBPlenoVento *pclTechBoxVento = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBoxVento->GetName() );
			
			CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTVENTO );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTechBoxVento->GetFunctionsStr( str ) );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBoxVento, pclTechBoxVento->GetArtNum() );
			FillProductParametersRow( pclTechBoxVento, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
			
			// Add accessories.
			if( NULL != pclSSelPressMaint->GetTecBoxVentoAccessoryList() && pclSSelPressMaint->GetTecBoxVentoAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetTecBoxVentoNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxVentoAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}

		// Fill Pleno if exist.
		if( ( 0 < pclSSelPressMaint->GetTecBoxPlenoNumber() ) && ( NULL != pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP || NULL != pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP ) )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetTecBoxPlenoNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

			CDB_TBPlenoVento *pclTechBoxPleno = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP );
			
			if( NULL == pclTechBoxPleno )
			{
				CDB_Set *pWTCombination = dynamic_cast<CDB_Set *>( (CData*)pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP );
				pclTechBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( (CData*)( pWTCombination->GetFirstIDPtr().MP ) );
			}
			
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBoxPleno->GetName() );
			
			CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENO );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTechBoxPleno->GetFunctionsStr( str ) );
			
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBoxPleno, pclTechBoxPleno->GetArtNum() );
			FillProductParametersRow( pclTechBoxPleno, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
			
			// Add accessories.
			if( NULL != pclSSelPressMaint->GetTecBoxPlenoAccessoryList() && pclSSelPressMaint->GetTecBoxPlenoAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetTecBoxPlenoNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxPlenoAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}

			if( NULL != pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP )
			{
				lRow++;
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
				pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

				CDB_TBPlenoVento *pclTBPlenoProtection = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP );
				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
				pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTBPlenoProtection->GetName() );

				CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENO );
				pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTechBoxPleno->GetFunctionsStr( str ) );
				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTBPlenoProtection, pclTBPlenoProtection->GetArtNum() );
				FillProductParametersRow( pclTBPlenoProtection, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

				// Add accessories.
				if( NULL != pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList() && pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList()->GetCount() > 0 )
				{
					lRow++;
					int iDistributedQty = pclSSelPressMaint->GetTecBoxPlenoNumber();
					FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
				}
			}
		}

		// Fill Pleno refill if exist.
		if( NULL != pclSSelPressMaint->GetPlenoRefillIDPtr().MP )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetPlenoRefillNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

			CDB_PlenoRefill *pclPlenoRefill = (CDB_PlenoRefill *)( pclSSelPressMaint->GetPlenoRefillIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclPlenoRefill->GetName() );
			
			CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENOREFILL ) + _T(" ");
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, str + pclPlenoRefill->GetFunctionsStr() );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclPlenoRefill, pclPlenoRefill->GetArtNum() );
			FillProductParametersRow( pclPlenoRefill, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
			
			// Add accessories.
			if( NULL != pclSSelPressMaint->GetPlenoRefillAccessoryList() && pclSSelPressMaint->GetPlenoRefillAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetPlenoRefillNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetPlenoRefillAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}
	}
	else
	{
		// Fill Compresso or Transfero.
		CDB_TecBox *pclTechBox = (CDB_TecBox *)( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP );
		CString strFunction = CteEMPTY_STRING;

		if( ( NULL != pclTechBox ) && ( CDB_TecBox::etbtCompresso == pclTechBox->GetTecBoxType() ) )
		{
			strFunction = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTCPRSSO );
		}
		else if( NULL != pclTechBox )
		{
			strFunction = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTTRSFRO );
		}
		
		if( NULL != pclTechBox )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetTecBoxCompTransfNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );
		
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBox->GetName() );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTechBox->GetFunctionsStr( strFunction ) );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBox, pclTechBox->GetArtNum() );
			FillProductParametersRow( pclTechBox, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
		}
		
		bool bPrimaryVesselIntegrated = false;
		
		// Add accessories.
		if( NULL != pclSSelPressMaint->GetTecBoxCompTransfAccessoryList() && pclSSelPressMaint->GetTecBoxCompTransfAccessoryList()->GetCount() > 0 )
		{
			lRow++;
			int iDistributedQty = pclSSelPressMaint->GetTecBoxCompTransfNumber();
			FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxCompTransfAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
		}

		// Fill integrated vessel if needed.
		if( _NULL_IDPTR != pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr() )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetTecBoxCompTransfNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

			CDB_Vessel *pclIntegratedVessel = (CDB_Vessel *)( pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclIntegratedVessel->GetName() );
			
			bool bIsBuffer = ( true == pclIntegratedVessel->IsInstallationLayoutIntegrated() ) ? false : true;
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclIntegratedVessel->GetVesselTypeStr( bIsBuffer ) );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclIntegratedVessel, pclIntegratedVessel->GetArtNum() );
			FillProductParametersRow( pclIntegratedVessel, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

			bPrimaryVesselIntegrated = pclIntegratedVessel->IsPrimaryVesselIntegrated();

			//  Add accessories.
			if( NULL != pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList() && pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetTecBoxCompTransfNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}

		// Fill primary vessel.
		int iQuantity = pclSSelPressMaint->GetVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		CDB_Vessel *pclVessel = (CDB_Vessel *)( pclSSelPressMaint->GetVesselIDPtr().MP );

		// Prepare three accessory lists for primary and secondary vessel in case of an accessory in the primary vessel list is set as 'Distributed'.
		bool bSecondaryVesselExist = ( NULL != dynamic_cast<CDB_Vessel *>( (CData *)pclSSelPressMaint->GetSecondaryVesselIDPtr().MP ) ) ? true : false;
		CAccessoryList clPrimOnlyVesselAccessoryList;
		CAccessoryList clSecOnlyVesselAccessoryList;
		CAccessoryList clPrimAndSecVesselAccessoryList;

		if( NULL != pclSSelPressMaint->GetVesselAccessoryList() )
		{
			CAccessoryList::AccessoryItem rAccessoryItem = pclSSelPressMaint->GetVesselAccessoryList()->GetFirst();

			while( _NULL_IDPTR != rAccessoryItem.IDPtr )
			{
				if( false == rAccessoryItem.fDistributed )
				{
					if( true == bSecondaryVesselExist )
					{
						clPrimAndSecVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
					}
					else
					{
						clPrimOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
					}
				}
				else
				{
					clPrimOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );

					if( true == bSecondaryVesselExist )
					{
						clSecOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
					}
				}

				rAccessoryItem = pclSSelPressMaint->GetVesselAccessoryList()->GetNext();
			}
		}

		// We have two solutions about integrated vessel. Either it's a buffer vessel as it's the case for Transfero TV.
		// Or it's a real integrated vessel that is playing the role of the primary vessel (like the one in the Simply Compresso).
		// When it's a buffer vessel, we show primary and secondary just after the buffer vessel.
		// When it's a integrated vessel, we must show after only the secondary vessel.

		if( false == bPrimaryVesselIntegrated && NULL != pclVessel )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );
			
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclVessel->GetName() );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclVessel->GetVesselTypeStr() );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclVessel, pclVessel->GetArtNum() );
			FillProductParametersRow( pclVessel, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

			// If primary vessel accessories exist...
			if( clPrimOnlyVesselAccessoryList.GetCount() > 0 )
			{
				lRow++;
				FillAccessoriesPM( pclSheet, &lRow, &clPrimOnlyVesselAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1 );
			}
		}

		// Fill secondary vessel if exist.
		if( _NULL_IDPTR != pclSSelPressMaint->GetSecondaryVesselIDPtr() )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			iQuantity = pclSSelPressMaint->GetSecondaryVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );

			pclVessel = (CDB_Vessel *)( pclSSelPressMaint->GetSecondaryVesselIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclVessel->GetName() );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclVessel->GetVesselTypeStr() );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclVessel, pclVessel->GetArtNum() );
			FillProductParametersRow( pclVessel, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

			// If secondary vessel accessories exist...
			if( clSecOnlyVesselAccessoryList.GetCount() > 0 )
			{
				lRow++;
				FillAccessoriesPM( pclSheet, &lRow, &clSecOnlyVesselAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1 );
			}

			// Add primary & secondary vessel accessories if exist.
			if( clPrimAndSecVesselAccessoryList.GetCount() > 0 )
			{
				lRow++;
				FillAccessoriesPM( pclSheet, &lRow, &clPrimAndSecVesselAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1 );
			}
		}

		// Add intermediate vessel if exist.
		IDPTR IntermVesselIDPtr = pclSSelPressMaint->GetIntermediateVesselIDPtr();

		if( _NULL_IDPTR != IntermVesselIDPtr && NULL != dynamic_cast<CDB_Vessel *>( IntermVesselIDPtr.MP ) )
		{
			// Intermediate vessel is in fact a Statico.
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQuantity = pclSSelPressMaint->GetIntermediateVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );
			
			CDB_Vessel *pclInermediateVessel = dynamic_cast<CDB_Vessel *>( IntermVesselIDPtr.MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclInermediateVessel->GetName() );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclInermediateVessel->GetVesselTypeStr() );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclInermediateVessel, pclInermediateVessel->GetArtNum() );
			FillProductParametersRow( pclInermediateVessel, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

			// Add accessory if exist.
			if( NULL != pclSSelPressMaint->GetIntermediateVesselAccessoryList() && pclSSelPressMaint->GetIntermediateVesselAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetIntermediateVesselNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetIntermediateVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}

		// Fill Vento if exist.
		if( NULL != pclSSelPressMaint->GetTecBoxVentoIDPtr().MP )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetTecBoxVentoNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

			CDB_TBPlenoVento *pclTechBoxVento = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBoxVento->GetName() );
			
			CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTVENTO );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTechBoxVento->GetFunctionsStr( str ) );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBoxVento, pclTechBoxVento->GetArtNum() );
			FillProductParametersRow( pclTechBoxVento, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
			
			if( NULL != pclSSelPressMaint->GetTecBoxVentoAccessoryList() && pclSSelPressMaint->GetTecBoxVentoAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetTecBoxVentoNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxVentoAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}

		// Fill Pleno if exist.
		if( (0 < pclSSelPressMaint->GetTecBoxPlenoNumber() ) && ( NULL != pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP || NULL != pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP ) )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetTecBoxPlenoNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

			CDB_TBPlenoVento *pclTechBoxPleno = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP );
			
			if( NULL == pclTechBoxPleno )
			{
				CDB_Set *pWTCombination = dynamic_cast<CDB_Set *>( (CData*)pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP );
				pclTechBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( (CData*)( pWTCombination->GetFirstIDPtr().MP ));
			}

			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBoxPleno->GetName() );

			CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENO );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTechBoxPleno->GetFunctionsStr( str ) );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBoxPleno, pclTechBoxPleno->GetArtNum() );
			FillProductParametersRow( pclTechBoxPleno, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

			// Add accessories.
			if( NULL != pclSSelPressMaint->GetTecBoxPlenoAccessoryList() && pclSSelPressMaint->GetTecBoxPlenoAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetTecBoxPlenoNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxPlenoAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}

			if( NULL != pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP )
			{
				lRow++;
				pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
				pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

				CDB_TBPlenoVento *pclTBPlenoProtection = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP );
				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
				pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTBPlenoProtection->GetName() );
				
				CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENO );
				pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTBPlenoProtection->GetFunctionsStr( str ) );
				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
				
				FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTBPlenoProtection, pclTBPlenoProtection->GetArtNum() );
				FillProductParametersRow( pclTBPlenoProtection, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
				
				// Add accessories.
				if( NULL != pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList() && pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList()->GetCount() > 0 )
				{
					lRow++;
					int iDistributedQty = pclSSelPressMaint->GetTecBoxPlenoNumber();
					FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
				}
			}
		}

		// Fill Pleno refill if exist.
		if( NULL != pclSSelPressMaint->GetPlenoRefillIDPtr().MP )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iQty = pclSSelPressMaint->GetPlenoRefillNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1) );

			CDB_PlenoRefill *pclPlenoRefill = (CDB_PlenoRefill *)( pclSSelPressMaint->GetPlenoRefillIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclPlenoRefill->GetName() );
			
			CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENOREFILL ) + _T(" ");
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, str + pclPlenoRefill->GetFunctionsStr() );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclPlenoRefill, pclPlenoRefill->GetArtNum() );
			FillProductParametersRow( pclPlenoRefill, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
			
			// Add accessories.
			if( NULL != pclSSelPressMaint->GetPlenoRefillAccessoryList() && pclSSelPressMaint->GetPlenoRefillAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetPlenoRefillNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetPlenoRefillAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}
	}
	
	lRow++;

	pclSheet->SetCellBorder( CD_PMWQ_Qty, *plRow+1, CD_PMWQ_LAST + 7, pclSheet->GetMaxRows(), true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );
	*plRow = lRow;
}

void CContainerForExcelExport::FillDirSelRow( CDS_SSelPMaint *pclSSelPressMaint, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;

	if( NULL == pclSheet || NULL == pclSSelPressMaint )
	{
		return;
	}

	if( NULL != dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetVesselIDPtr().MP )
			|| NULL != dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetIntermediateVesselIDPtr().MP )
			|| NULL != dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetSecondaryVesselIDPtr().MP )  )
	{
		CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetVesselIDPtr().MP );
		int iQuantity = pclSSelPressMaint->GetVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		CAccessoryList *pclaccessories = pclSSelPressMaint->GetVesselAccessoryList();
		
		if( NULL == pclVessel )
		{
			pclVessel = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetIntermediateVesselIDPtr().MP );
			iQuantity = pclSSelPressMaint->GetIntermediateVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclaccessories = pclSSelPressMaint->GetIntermediateVesselAccessoryList();
			
			if( NULL == pclVessel )
			{
				pclVessel = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetSecondaryVesselIDPtr().MP );
				iQuantity = pclSSelPressMaint->GetSecondaryVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			}
		}
		
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );
		
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclVessel->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclVessel, pclVessel->GetArtNum() );
		FillProductParametersRow( pclVessel, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
		
		if( NULL != pclaccessories && pclaccessories->GetCount() > 0 )
		{
			lRow++;
			int iDistributedQty = pclSSelPressMaint->GetVesselNumber() + pclSSelPressMaint->GetSecondaryVesselNumber();
			FillAccessoriesPM( pclSheet, &lRow, pclaccessories, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
		}
	}
	else if( NULL != dynamic_cast<CDB_TecBox *>( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP ) )
	{
		CDB_TecBox *pclTechBox = (CDB_TecBox *)( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP );
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQty = pclSSelPressMaint->GetTecBoxCompTransfNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );
		
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBox->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBox, pclTechBox->GetArtNum() );
		FillProductParametersRow( pclTechBox, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
		
		// Accessories.
		if( NULL != pclSSelPressMaint->GetTecBoxCompTransfAccessoryList() && pclSSelPressMaint->GetTecBoxCompTransfAccessoryList()->GetCount() > 0 )
		{
			lRow++;
			int iDistributedQty = pclSSelPressMaint->GetTecBoxCompTransfNumber();
			FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxCompTransfAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
		}
		
		// Fill integrated vessel if needed.
		if( _NULL_IDPTR != pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr() )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			CDB_Vessel *pclIntegratedVessel = (CDB_Vessel *)( pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr().MP );
			
			int iQty = pclSSelPressMaint->GetTecBoxCompTransfNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );
			
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclIntegratedVessel->GetName() );

			bool bIsBuffer = ( true == pclIntegratedVessel->IsInstallationLayoutIntegrated() ) ? false : true;
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclIntegratedVessel->GetVesselTypeStr( bIsBuffer ) );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclIntegratedVessel, pclIntegratedVessel->GetArtNum() );
			FillProductParametersRow( pclIntegratedVessel, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
			
			//  Add accessories.
			if( NULL != pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList() && pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetTecBoxCompTransfNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}
	}
	else if( NULL != pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP || NULL != pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP )
	{
		CDB_TBPlenoVento *pclTechBoxPleno = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP );
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQty = pclSSelPressMaint->GetTecBoxPlenoNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );
		
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBoxPleno->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBoxPleno, pclTechBoxPleno->GetArtNum() );
		FillProductParametersRow( pclTechBoxPleno, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
		
		// Add accessories.
		if( NULL != pclSSelPressMaint->GetTecBoxPlenoAccessoryList() && pclSSelPressMaint->GetTecBoxPlenoAccessoryList()->GetCount() > 0 )
		{
			lRow++;
			int iDistributedQty = pclSSelPressMaint->GetTecBoxPlenoNumber();
			FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxPlenoAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
		}

		if( NULL != pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

			CDB_TBPlenoVento *pclTBPlenoProtection = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTBPlenoProtection->GetName() );
			
			CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENO );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTBPlenoProtection->GetFunctionsStr( str ) );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTBPlenoProtection, pclTBPlenoProtection->GetArtNum() );
			FillProductParametersRow( pclTBPlenoProtection, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
			
			// Add accessories.
			if( NULL != pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList() && pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetTecBoxPlenoNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}
	}
	else if( NULL != pclSSelPressMaint->GetTecBoxVentoIDPtr().MP )
	{
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQty = pclSSelPressMaint->GetTecBoxVentoNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

		CDB_TBPlenoVento *pclTechBoxVento = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBoxVento->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBoxVento, pclTechBoxVento->GetArtNum() );
		FillProductParametersRow( pclTechBoxVento, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
		
		if( NULL != pclSSelPressMaint->GetTecBoxVentoAccessoryList() && pclSSelPressMaint->GetTecBoxVentoAccessoryList()->GetCount() > 0 )
		{
			lRow++;
			int iDistributedQty = pclSSelPressMaint->GetTecBoxVentoNumber();
			FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxVentoAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
		}
	}
	else if( NULL != pclSSelPressMaint->GetPlenoRefillIDPtr().MP )
	{
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQty = pclSSelPressMaint->GetPlenoRefillNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

		CDB_PlenoRefill *pclPlenoRefill = (CDB_PlenoRefill *)( pclSSelPressMaint->GetPlenoRefillIDPtr().MP );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclPlenoRefill->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclPlenoRefill, pclPlenoRefill->GetArtNum() );
		FillProductParametersRow( pclPlenoRefill, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
		
		// Add accessories.
		if( NULL != pclSSelPressMaint->GetPlenoRefillAccessoryList() && pclSSelPressMaint->GetPlenoRefillAccessoryList()->GetCount() > 0 )
		{
			lRow++;
			int iDistributedQty = pclSSelPressMaint->GetPlenoRefillNumber();
			FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetPlenoRefillAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
		}
	}

	lRow++;

	pclSheet->SetCellBorder( CD_PMWQ_Qty, *plRow + 1, CD_PMWQ_LAST + 7, pclSheet->GetMaxRows(), true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedPlenoVentoRow( CDS_SSelPMaint *pclSSelPressMaint, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;

	if( NULL == pclSheet || NULL == pclSSelPressMaint )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	CString str;

	pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, pclSSelPressMaint->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_PMWQ_Ref2, lRow, pclSSelPressMaint->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST, lRow, _VLIGHTYELLOW );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Fill Vento if exist.
	if( NULL != dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP ) )
	{
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQty = pclSSelPressMaint->GetTecBoxVentoNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

		CDB_TBPlenoVento *pclTechBoxVento = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBoxVento->GetName() );
		
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTVENTO );
		pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTechBoxVento->GetFunctionsStr( str ) );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBoxVento, pclTechBoxVento->GetArtNum() );
		FillProductParametersRow( pclTechBoxVento, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

		// Add Vento accessories if exist.
		CAccessoryList *pclAccessoryList = pclSSelPressMaint->GetTecBoxVentoAccessoryList();

		if( NULL != pclAccessoryList && pclAccessoryList->GetCount() > 0 )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			FillAccessoriesPM( pclSheet, &lRow, pclAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1 );
		}
	}

	// Fill Pleno if exist.
	if( ( 0 < pclSSelPressMaint->GetTecBoxPlenoNumber() ) && 
		( NULL != dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP ) ||
		NULL != dynamic_cast<CDB_Set*>( pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP ) ) )
	{
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQty = pclSSelPressMaint->GetTecBoxPlenoNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

		CDB_TBPlenoVento *pclTechBoxPleno = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP );
		if( NULL == pclTechBoxPleno )
		{
			CDB_Set *pWTCombination = dynamic_cast<CDB_Set *>( (CData*)pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP );
			pclTechBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( (CData*)( pWTCombination->GetFirstIDPtr().MP ) );
		}

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTechBoxPleno->GetName() );
		
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENO );
		pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTechBoxPleno->GetFunctionsStr( str ) );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTechBoxPleno, pclTechBoxPleno->GetArtNum() );
		FillProductParametersRow( pclTechBoxPleno, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

		// Add Pleno accessories if exist.
		CAccessoryList *pclAccessoryList = pclSSelPressMaint->GetTecBoxPlenoAccessoryList();

		if( NULL != pclAccessoryList && pclAccessoryList->GetCount() > 0 )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			FillAccessoriesPM( pclSheet, &lRow, pclAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1 );
		}

		if( NULL != pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

			CDB_TBPlenoVento *pclTBPlenoProtection = (CDB_TBPlenoVento *)( pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclTBPlenoProtection->GetName() );
			
			CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENO );
			pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclTBPlenoProtection->GetFunctionsStr( str ) );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclTBPlenoProtection, pclTBPlenoProtection->GetArtNum() );
			FillProductParametersRow( pclTBPlenoProtection, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );
			
			// Add accessories.
			if( NULL != pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList() && pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList()->GetCount() > 0 )
			{
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetTecBoxPlenoNumber();
				FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty );
			}
		}
	}

	// Fill Pleno refill if exist.
	if( NULL != dynamic_cast<CDB_PlenoRefill *>( pclSSelPressMaint->GetPlenoRefillIDPtr().MP ) )
	{
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQty = pclSSelPressMaint->GetPlenoRefillNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQty, 1 ) );

		CDB_PlenoRefill *pclPlenoRefill = (CDB_PlenoRefill *)( pclSSelPressMaint->GetPlenoRefillIDPtr().MP );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclPlenoRefill->GetName() );

		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENOREFILL ) + _T(" ");
		pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, str + pclPlenoRefill->GetFunctionsStr() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclPlenoRefill, pclPlenoRefill->GetArtNum() );
		FillProductParametersRow( pclPlenoRefill, pclSSelPressMaint, NULL, NULL, pclSheet, lRow );

		// Add accessories.
		if( NULL != pclSSelPressMaint->GetPlenoRefillAccessoryList() && pclSSelPressMaint->GetPlenoRefillAccessoryList()->GetCount() > 0 )
		{
			lRow++;
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			int iDistributedQty = pclSSelPressMaint->GetPlenoRefillNumber();
			FillAccessoriesPM( pclSheet, &lRow, pclSSelPressMaint->GetPlenoRefillAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(),
				iDistributedQty );
		}
	}

	lRow++;

	pclSheet->SetCellBorder( CD_PMWQ_Qty, *plRow + 1, CD_PMWQ_LAST + 7, pclSheet->GetMaxRows(), true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );
	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedSeparatorRow( CDS_SSelAirVentSeparator *pclSSel, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;

	if( NULL == pclSheet || NULL == pclSSel )
	{
		return;
	}

	CDB_Separator *pclSeparator = pclSSel->GetProductAs<CDB_Separator>();
	CDB_AirVent *pclAirVent = pclSSel->GetProductAs<CDB_AirVent>();

	if( NULL == pclSeparator && NULL == pclAirVent )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	CString str;

	pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_PMWQ_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );

	// Fill technical informations
	// Water informations.
	if( false == pclSSel->IsFromDirSel() )
	{
		CString str1, str2;
		pclSSel->GetpSelectedInfos()->GetpWCData()->BuildWaterStrings( str1, str2 );

		long lPrev = pclSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
		CString strFinal = str1 + _T("\n") + str2;
		pclSheet->SetStaticText( CD_PMWQ_IO_FluidType, lRow, strFinal );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev );

		// Separator information.
		CString strPowerDt = _T("");

		if( CDS_SelProd::efdPower == pclSSel->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSel->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSel->GetDT(), true );
		}

		if( pclSSel->GetQ() > 0.0 )
		{
			// HYS-38: Show power dt info when their radio button is checked
			if( strPowerDt != _T("") )
			{
				CString strQ = _T("(");
				strQ += WriteCUDouble( _U_FLOW, pclSSel->GetQ(), true );
				strQ += _T(")");
				pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_PowerDt, lRow, strPowerDt );
				pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pclSSel->GetQ(), true ) );
			}
			else
			{
				// Flow
				pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_Flow, lRow, WriteCUDouble( _U_FLOW, pclSSel->GetQ(), true ) );
			}
		}

		// Dp.
		if( pclSSel->GetDp() > 0 )
		{
			pclSheet->SetStaticText( CD_PMWQ_SepTechInfo_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pclSSel->GetDp(), true ) );
		}
	}
	
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST, lRow, _VLIGHTYELLOW );

	// Product selection information.
	if( NULL != pclSeparator )
	{
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );
		
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclSeparator->GetName() );
		pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclSeparator->GetType() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclSeparator, pclSeparator->GetArtNum() );
		FillProductParametersRow( pclSeparator, NULL, pclSSel, NULL, pclSheet, lRow );
		
		// Add accessories.
		if( ( true == pclSSel->IsAccessoryExist() ) && ( NULL != pclSSel->GetAccessoryList() ) )
		{
			lRow++;
			FillAccessoriesPM( pclSheet, &lRow, pclSSel->GetAccessoryList(), pclSSel->GetpSelectedInfos()->GetQuantity(), 1 );
		}
	}
	else if( NULL != pclAirVent )
	{
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );
		
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclAirVent->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclAirVent, pclAirVent->GetArtNum() );
		FillProductParametersRow( pclAirVent, NULL, pclSSel, NULL, pclSheet, lRow );
		
		// Add accessories.
		if( ( true == pclSSel->IsAccessoryExist() ) && ( NULL != pclSSel->GetAccessoryList() ) )
		{
			lRow++;
			FillAccessoriesPM( pclSheet, &lRow, pclSSel->GetAccessoryList(), pclSSel->GetpSelectedInfos()->GetQuantity(), 1 );
		}
	}
	
	lRow++;
	pclSheet->SetCellBorder( CD_PMWQ_Qty, *plRow + 1, CD_PMWQ_LAST + 7, pclSheet->GetMaxRows(), true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );

	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedSafetyValveRow( CDS_SSelSafetyValve *pclSSel, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;

	if( NULL == pclSheet || NULL == pclSSel )
	{
		return;
	}

	CDB_SafetyValveBase *pclSafetyValve = pclSSel->GetProductAs<CDB_SafetyValveBase>();

	if( ( NULL == pclSafetyValve ) && ( NULL == pclSSel->GetBlowTankIDPtr().MP ) )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	CString str;

	if( NULL != pclSafetyValve )
	{
		pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
		pclSheet->SetStaticText( CD_PMWQ_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );

		// Fill technical information.
		// Water information.
		CString strMediumName = pclSafetyValve->GetMediumName();
		CString strTMin = WriteCUDouble( _U_TEMPERATURE, pclSafetyValve->GetTmin() );
		CString strTMax = WriteCUDouble( _U_TEMPERATURE, pclSafetyValve->GetTmax(), true );
		CString strFull = strMediumName + _T("(" ) + strTMin + _T( " / " ) + strTMax + _T( ")");
		long lPrev = pclSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_IO_FluidType, lRow, strFull );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev );

		if( false == pclSSel->IsFromDirSel() )
		{
			// Safety valve information.
			pclSheet->SetStaticText( CD_PMWQ_SafValTechInfo_SetPress, lRow, WriteCUDouble( _U_PRESSURE, pclSSel->GetSetPressure(), true ) );

			if( ProjectType::Heating == pclSSel->GetpSelectedInfos()->GetApplicationType() )
			{
				CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
				str = TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Heating ) );
				str += _T(", ");

				CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( TASApp.GetpTADB()->Get( pclSSel->GetSystemHeatGeneratorTypeID() ).MP );

				if( NULL != pStrID )
				{
					str += pStrID->GetString();
				}
				
				pclSheet->SetStaticText( CD_PMWQ_SafValTechInfo_HeatGenType, lRow, str );
			}

			double dPowerLimit = pclSafetyValve->GetPowerLimit( pclSSel->GetSystemHeatGeneratorTypeID(), pclSSel->GetNormID() );
			pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_MaxPower, lRow, WriteCUDouble( _U_TH_POWER, dPowerLimit, true ) );
			pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_Power, lRow, WriteCUDouble( _U_TH_POWER, pclSSel->GetInstalledPower(), true ) );

			if( ProjectType::Solar == pclSSel->GetpSelectedInfos()->GetApplicationType() )
			{
				CDB_SafetyValveSolar *pclSafetyValveSolar = (CDB_SafetyValveSolar *)pclSafetyValve;
				pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_MaxCollec, lRow, WriteCUDouble( _U_AREA, pclSafetyValveSolar->GetCollectorLimit(), true ) );

				pclSheet->SetStaticText( CD_PMWQ_SafValechInfo_Collec, lRow, WriteCUDouble( _U_AREA, pclSSel->GetInstalledCollector(), true ) );
			}
		}

		pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
		pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST, lRow, _VLIGHTYELLOW );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Product selection information.
		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		int iQuantity = pclSSel->GetpSelectedInfos()->GetQuantity() * pclSSel->GetQuantityNeeded();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclSafetyValve->GetName() );
		pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, IDS_SSHEETSSELSAFETYVALVE_SAFETYVALVE );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclSafetyValve, pclSafetyValve->GetArtNum() );
		FillProductParametersRow( pclSafetyValve, NULL, NULL, pclSSel, pclSheet, lRow );

		// Add accessories.
		if( ( true == pclSSel->IsAccessoryExist() ) && ( NULL != pclSSel->GetAccessoryList() ) )
		{
			lRow++;
			FillAccessoriesPM( pclSheet, &lRow, pclSSel->GetAccessoryList(), pclSSel->GetpSelectedInfos()->GetQuantity() * pclSSel->GetQuantityNeeded(), 1 );
		}

		FillSelectedBlowTankRow( pclSSel, pclSheet, &lRow );
	}
	else
	{
		// Add blow tank.
		FillSelectedBlowTankRow( pclSSel, pclSheet, &lRow );
	}

	lRow++;
	pclSheet->SetCellBorder( CD_PMWQ_Qty, *plRow + 1, CD_PMWQ_LAST + 7, pclSheet->GetMaxRows(), true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );

	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedAccessServicesRow( CDS_SelPWQAccServices *pclSSel, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;

	if( NULL == pclSheet || NULL == pclSSel )
	{
		return;
	}

	CDB_PWQPressureReducer *pclPressureReduc = pclSSel->GetProductAs<CDB_PWQPressureReducer>();

	if( ( NULL == pclPressureReduc) )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	CString str;

	pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
	pclSheet->SetStaticText( CD_PMWQ_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );

	// Fill information

	pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
	pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST, lRow, _VLIGHTYELLOW );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Product selection information.
	lRow++;
	pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
	int iQuantity = pclSSel->GetpSelectedInfos()->GetQuantity();
	pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclPressureReduc->GetName() );
	pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, IDS_SSHEETSSEL_PRESSREDUC );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclPressureReduc, pclPressureReduc->GetArtNum() );

	long lCurrentCol = CD_PMWQ_LAST + 1;

	//// Product parameter
	//////////////////////////

	str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_MATERIAL );
	str += CString( _T( " = " ) ) + pclPressureReduc->GetBodyMaterial();
	pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

	str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
	str += CString( _T( " = " ) ) + pclPressureReduc->GetConnect();
	pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

	str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_VERSION );
	str += CString( _T( " = " ) ) + pclPressureReduc->GetVersion();
	pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

	CString str2 = TASApp.LoadLocalizedString( IDS_PN );
	str = str2 + _T( " " );
	str2 = pclPressureReduc->GetPN().c_str();
	str += str2;
	str += CString( "; " );

	if( pclPressureReduc->GetTmin() > -273.15 )
	{
		str += WriteCUDouble( _U_TEMPERATURE, pclPressureReduc->GetTmin(), false == (pclPressureReduc->GetTmax() < DBL_MAX) );
	}
	else
	{
		str += CString( "- " );
	}

	str += CString( "/" );
	if( pclPressureReduc->GetTmax() < DBL_MAX )
	{
		str += WriteCUDouble( _U_TEMPERATURE, pclPressureReduc->GetTmax(), true );
	}
	else
	{
		str += CString( " -" );
	}

	pclSheet->SetStaticText( ++lCurrentCol, lRow, str );


	str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_INLETPRESS );
	str += CString( _T( ": " ) ) + WriteCUDouble( _U_PRESSURE, pclPressureReduc->GetMaxInletPressure(), true );
	pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

	str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_OUTLETPRESS );
	str += CString( _T( ": " ) ) + WriteCUDouble( _U_PRESSURE, pclPressureReduc->GetMaxOutletPressure(), true );
	pclSheet->SetStaticText( ++lCurrentCol, lRow, str );


	lRow++;
	pclSheet->SetCellBorder( CD_PMWQ_Qty, *plRow + 1, CD_PMWQ_LAST + 7, pclSheet->GetMaxRows(), true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, _BLACK );

	*plRow = lRow;
}

void CContainerForExcelExport::FillSelectedBlowTankRow( CDS_SSelSafetyValve *pclSSel, CSSheet *pclSheet, long *plRow )
{
	long lRow = *plRow;
	CString str = CteEMPTY_STRING;

	if( NULL == pclSSel )
	{
		return;
	}
	
	// Fill blow tank rows.
	CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( pclSSel->GetBlowTankIDPtr().MP );

	if( NULL != pclBlowTank )
	{
		if( true == pclSSel->IsFromDirSel() )
		{
			pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );
			pclSheet->SetStaticText( CD_PMWQ_Ref2, lRow, pclSSel->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	
			// Fill technical information.
			// Water information.
			CString strMediumName = pclBlowTank->GetMediumName();
			CString strTMin = WriteCUDouble( _U_TEMPERATURE, pclBlowTank->GetTmin() );
			CString strTMax = WriteCUDouble( _U_TEMPERATURE, pclBlowTank->GetTmax(), true );
			CString strFull = strMediumName + _T("(" ) + strTMin + _T( " / " ) + strTMax + _T( ")");
			long lPrev = pclSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_IO_FluidType, lRow, strFull );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev );

			pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
			pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST, lRow, _VLIGHTYELLOW );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		}

		lRow++;
		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );

		int iQuantity = pclSSel->GetpSelectedInfos()->GetQuantity() * pclSSel->GetQuantityNeeded();
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iQuantity, 1 ) );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclBlowTank->GetName() );
		pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, IDS_SSHEETSSELSAFETYVALVE_BLOWTANK );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclBlowTank, pclBlowTank->GetArtNum() );
		
		// Blow tank product param.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_SIZE );
		str += CString( _T(" = ") ) + pclBlowTank->GetAllSizesString();
		pclSheet->SetStaticText( CD_PMWQ_LAST + 2, lRow, str );

		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_BLOWTANK_PS );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_PRESSURE, pclBlowTank->GetPmaxmax(), true );
		pclSheet->SetStaticText( CD_PMWQ_LAST + 2 + 1, lRow, str );
		
		// Add accessories.
		if( ( true == pclSSel->IsBlowTankAccessoryExist() ) && ( NULL != pclSSel->GetBlowTankAccessoryList() ) )
		{
			lRow++;
			FillAccessoriesPM( pclSheet, &lRow, pclSSel->GetBlowTankAccessoryList(), pclSSel->GetpSelectedInfos()->GetQuantity() * pclSSel->GetQuantityNeeded(), 1 );
		}
	}
	
	*plRow = lRow;
}

void CContainerForExcelExport::FillPMInputDataRow( CDS_SSelPMaint *pclSSel, CSSheet *pclSheet, long lRow )
{
	if( NULL == pclSheet || NULL == pclSSel )
	{
		return;
	}

	if( true == pclSSel->IsFromDirSel() )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );

	CDS_TechnicalParameter *pclTechParam = TASApp.GetpTADS()->GetpTechParams();
	CPMInputUser *pclPMInputUser = pclSSel->GetpInputUser();

	CString str = TASApp.LoadLocalizedString( pclTechParam->GetProductSelectionApplicationTypeIDS( pclPMInputUser->GetApplicationType() ) );
	pclSheet->SetStaticText( CD_PMWQ_IO_AppType, lRow, str );

	CString str1 = CteEMPTY_STRING;
	pclSSel->GetpSelectedInfos()->GetpWCData()->BuildWaterStrings( str, str1 );
	pclSheet->SetStaticText( CD_PMWQ_IO_FluidType, lRow, str );

	IDPTR NormIDPtr = TASApp.GetpTADB()->Get( (LPCTSTR)pclPMInputUser->GetNormID() );
	CDB_StringID *pStrNormID = dynamic_cast<CDB_StringID *>( NormIDPtr.MP );
		
	if( NULL != pStrNormID )
	{
		str = pStrNormID->GetString();
		pclSheet->SetStaticText( CD_PMWQ_IO_Norm, lRow, str );
	}

	pclSheet->SetStaticText( CD_PMWQ_IO_SystemVolume, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSystemVolume(), true ) );

	if( ProjectType::Solar == pclPMInputUser->GetApplicationType() )
	{
		// Solar colletor volume [Vkoll].
		pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorVolume, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSolarCollectorVolume(), true ) );

		// Solar colletor multiplier factor.
		pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorMultiplierFactor, lRow, WriteDouble( pclPMInputUser->GetSolarCollectorMultiplierFactor(), 2, 1 ) );
	}
	//else if( true == m_bAtLeastOneSolar )
	//{
		pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorVolume, lRow, _T("-") );
		pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorMultiplierFactor, lRow, _T("-") );
	//}

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 )
			&& pclPMInputUser->GetStorageTankVolume() > 0.0 )
	{
		// Storage tank volume [Vsto].
		pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankVolume, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetStorageTankVolume(), true ) );

		// Storage tank max. temperature [Tmax,sto].
		pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankMaxTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetStorageTankMaxTemp(), true ) );
	}
	//else if( true == m_bAtLeastOneSWKINormWithStorageTank )
	//{
		pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankVolume, lRow, _T("-") );
		pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankMaxTemp, lRow, _T("-") );
	//}

	// Installed power.
	pclSheet->SetStaticText( CD_PMWQ_IO_InstPower, lRow, WriteCUDouble( _U_TH_POWER, pclPMInputUser->GetInstalledPower(), true ) );

	// Static height.
	pclSheet->SetStaticText( CD_PMWQ_IO_StatHeight, lRow, WriteCUDouble( _U_LENGTH, pclPMInputUser->GetStaticHeight(), true ) );

	// Min. required equipment pressure.
	if( BST_CHECKED == pclPMInputUser->GetPzChecked() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_Pz, lRow, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetPz(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_Pz, lRow, _T("-") );
	}

	// Safety valve response pressure.
	pclSheet->SetStaticText( CD_PMWQ_IO_SafValvPress, lRow, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetSafetyValveResponsePressure(), true ) );

	if( ProjectType::Heating == pclPMInputUser->GetApplicationType() || ProjectType::Solar == pclPMInputUser->GetApplicationType() )
	{
		// Safety temp limiter.
		pclSheet->SetStaticText( CD_PMWQ_IO_SafTempLimit, lRow, WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetSafetyTempLimiter(), true ) );
		
		// Max. temperature.
		pclSheet->SetStaticText( CD_PMWQ_IO_MaxTemp, lRow, _T("-") );
	}
	else
	{
		// Safety temp limiter.
		pclSheet->SetStaticText( CD_PMWQ_IO_SafTempLimit, lRow, _T("-") );

		// Max. temperature.
		pclSheet->SetStaticText( CD_PMWQ_IO_MaxTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetMaxTemperature(), true ) );
	}

	// Supply temp.
	pclSheet->SetStaticText( CD_PMWQ_IO_SupplyTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetSupplyTemperature(), true ) );

	// Return temp.
	pclSheet->SetStaticText( CD_PMWQ_IO_ReturnTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetReturnTemperature(), true ) );

	// Min. temp.
	pclSheet->SetStaticText( CD_PMWQ_IO_MinTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetMinTemperature(), true ) );

	// HYS-1343 : We show the fill temperature only for expansion vessel.
	if( CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_FillTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetFillTemperature(), true ) );
	}

	str = TASApp.LoadLocalizedString( pclTechParam->GetPressurONIDS( pclPMInputUser->GetPressOn() ) );
	pclSheet->SetStaticText( CD_PMWQ_IO_PressOn, lRow, str );

	if( PressurON::poPumpSuction == pclPMInputUser->GetPressOn() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_PumpHead, lRow, _T("-") );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_PumpHead, lRow, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetPumpHead(), true ) );
	}

	pclSheet->SetStaticText( CD_PMWQ_IO_MaxWidth, lRow, WriteCUDouble( _U_LENGTH, pclPMInputUser->GetMaxWidth(), true ) );
	pclSheet->SetStaticText( CD_PMWQ_IO_MaxHeight, lRow, WriteCUDouble( _U_LENGTH, pclPMInputUser->GetMaxHeight(), true ) );
}

void CContainerForExcelExport::FillPMCalculatedDataRow( CDS_SSelPMaint *pclSSel, CSSheet *pclSheet, long lRow )
{
	if( NULL == pclSheet || NULL == pclSSel )
	{
		return;
	}

	if( true == pclSSel->IsFromDirSel() )
	{
		return;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, false );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );

	CPMInputUser *pclPMInputUser = pclSSel->GetpInputUser();

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetpTADS() || NULL == pclPMInputUser->GetpTADS()->GetpTechParams() )
	{
		return;
	}

	CDS_TechnicalParameter *pclTechnicalParameters = pclPMInputUser->GetpTADS()->GetpTechParams();
	
	// System expansion coefficient.
	pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpCoeff, lRow, WriteDouble( pclPMInputUser->GetSystemExpansionCoefficient(), 4, 3 ) );

	// System expansion volume [Ve].
	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpVol, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSystemExpansionVolume(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpVol, lRow, _T("-") );
	}

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// Factor [X].
		pclSheet->SetStaticText( CD_PMWQ_IO_FactorX, lRow, WriteDouble( pclPMInputUser->GetXFactorSWKI(), 3, 1 ) );

		// System expansion volume [Vs.e.X].
		pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpVolSWKI, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSystemExpansionVolume(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_FactorX, lRow, _T("-") );
		pclSheet->SetStaticText( CD_PMWQ_IO_SystemExpVolSWKI, lRow, _T("-") );
	}

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 )
			&& pclPMInputUser->GetStorageTankVolume() > 0.0 )
	{
		// Storage tank expansion coefficient [esto].
		pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankExpCoeff, lRow, WriteDouble( pclPMInputUser->GetStorageTankExpansionCoefficient(), 4, 3 ) );

		// Storage tank expansion volume [Vsto.esto].
		pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankExpVolume, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetStorageTankExpansionVolume(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankExpCoeff, lRow, _T("-") );
		pclSheet->SetStaticText( CD_PMWQ_IO_StorageTankExpVolume, lRow, _T("-") );
	}

	// Total expansion volume [Ve,tot].
	pclSheet->SetStaticText( CD_PMWQ_IO_TotalExpVolume, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetTotalExpansionVolume(), true ) );

	if( ProjectType::Solar == pclPMInputUser->GetApplicationType() )
	{
		// Solar collector security volume [VDK].
		pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorSecurityVolume, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSolarCollectorSecurityVolume(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_SolarCollectorSecurityVolume, lRow, _T("-") );
	}

	// Min. water reserve (In expansion) -> Except for SWKI 93-1 norm.
	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_MinWResInExpansion, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetMinimumWaterReserve(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_MinWResInExpansion, lRow, _T("-") );
	}

	// Degassing water reserve [Vv].
	if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_DegassingWaterReserve, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetDegassingWaterReserve(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_DegassingWaterReserve, lRow, _T("-") );
	}

	// Vessel net volume [Vn].
	pclSheet->SetStaticText( CD_PMWQ_IO_VesselNetVolume, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNetVolume(), true ) );

	// Min. water reserve (Not in expansion) -> For SWKI HE301-01 norm.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_MinWResNotInExpansion, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetMinimumWaterReserve(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_MinWResNotInExpansion, lRow, _T("-") );
	}

	if( CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_WaterRes, lRow, WriteCUDouble( _U_VOLUME, pclSSel->GetVesselWaterReserve(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_WaterRes, lRow, _T("-") );
	}

	// HYS-1343 : We show contraction volume only for expansion vessel in cooling.
	if( ProjectType::Cooling == pclSSel->GetpInputUser()->GetApplicationType()
			&& CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_CtrVol, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->ComputeContractionVolume(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_CtrVol, lRow, _T("-") );
	}

	pclSheet->SetStaticText( CD_PMWQ_IO_VapPress, lRow, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetVaporPressure(), true ) );
	pclSheet->SetStaticText( CD_PMWQ_IO_MinPress, lRow, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumPressure(), true ) );

	// HYS-1116: We show this value only if we are in EN12828 norm AND with a Statico.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_EN12828 )
			&& CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() )
	{
		CString str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumInitialPressure(), true );
		pclSheet->SetStaticText( CD_PMWQ_IO_MinInitPress, lRow, str );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_MinInitPress, lRow, _T("-") );
	}

	// Initial pressure for Statico.
	if( CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_InitPress, lRow, WriteCUDouble( _U_PRESSURE, pclSSel->GetVesselInitialPressure(), true ) );
	}
	else
	{
		// For Compresso/Transfero it is always the minimum initial pressure.
		CString str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumInitialPressure(), true );
		pclSheet->SetStaticText( CD_PMWQ_IO_InitPress, lRow, str );
	}

	// Filling pressure
	CString str = CteEMPTY_STRING;

	// HYS-1343 : We show the filling pressure only for expansion vessel.
	if( ( CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() ) 
			&& ( NULL != dynamic_cast<CDB_Vessel *>( pclSSel->GetVesselIDPtr().MP ) ) )
	{
		double dTotalVesselVolume = pclSSel->GetVesselNumber() * ( dynamic_cast<CDB_Vessel *>( pclSSel->GetVesselIDPtr().MP )->GetNominalVolume() );
		double dFillingPressure = pclPMInputUser->GetIntermediatePressure( pclPMInputUser->GetFillTemperature(), pclSSel->GetVesselWaterReserve(), dTotalVesselVolume );
		str = WriteCUDouble( _U_PRESSURE, dFillingPressure, true );
		pclSheet->SetStaticText( CD_PMWQ_IO_FillPress, lRow, str );
	}

	// Target pressure.
	str = _T("-");

	if( CDS_SSelPMaint::Statico != pclSSel->GetSelectionType() )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pclSSel->GetTecBoxCompTransfIDPtr().MP );

		if( NULL != pclTecBox )
		{
			str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetTargetPressureForTecBox( pclTecBox->GetTecBoxType() ), true );
		}
	}
	
	pclSheet->SetStaticText( CD_PMWQ_IO_TargetPress, lRow, str );

	// Final pressure.
	str = _T("-");

	if( CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() )
	{
		str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetFinalPressure(), true );
	}
	else
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pclSSel->GetTecBoxCompTransfIDPtr().MP );
		
		if( NULL != pclTecBox )
		{
			str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetFinalPressure( pclTecBox->GetTecBoxType() ), true );
		}
	}
	
	pclSheet->SetStaticText( CD_PMWQ_IO_FinalPress, lRow, str );

	// Specific flow rate of equalization volume.
	str = _T("-");

	if( CDS_SSelPMaint::Statico != pclSSel->GetSelectionType() )
	{
		double dEqualizingVolumetricFlow = pclPMInputUser->GetEqualizingVolumetricFlow();
		double d1 = CDimValue::SItoCU( _U_FLOW, dEqualizingVolumetricFlow );
		double d2 = CDimValue::SItoCU( _U_TH_POWER, 1 / d1 );

		if( 0 != d2 )
		{
			d2 = 1 / d2;
			str = WriteDouble( d2, 4, 0 );
			CString strUnit = CString( CDimValue::AccessUDB()->GetNameOfDefaultUnit( _U_FLOW ).c_str() ) + CString( _T("/") )
				+ CString( CDimValue::AccessUDB()->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
			str = str + _T(" ") + strUnit;
		}
	}
	
	pclSheet->SetStaticText( CD_PMWQ_IO_SpeFlEqVol, lRow, str );

	// Needed flow rate of equalization volume.
	str = _T("-");

	if( CDS_SSelPMaint::Statico != pclSSel->GetSelectionType() )
	{
		str = WriteCUDouble( _U_FLOW, pclPMInputUser->GetVD(), true );
	}
	
	pclSheet->SetStaticText( CD_PMWQ_IO_NeFlEqVol, lRow, str );

	// Pressure factor at nominal volume.
	if( CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_PressFact, lRow,WriteDouble( pclPMInputUser->GetPressureFactor(), 3, 2 ) );
		pclSheet->SetStaticText( CD_PMWQ_IO_NomVol, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNominalVolume(), true ) );
	}
	else
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_PressFact, lRow, WriteDouble( pclPMInputUser->GetPressureFactor( true ), 3, 2 ) );
		pclSheet->SetStaticText( CD_PMWQ_IO_NomVol, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNominalVolume( true ), true ) );
	}

	// Expansion pipe.
	if( CDS_SSelPMaint::Statico == pclSSel->GetSelectionType() )
	{
		pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe10, lRow, _T("-") );
		pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe30, lRow, _T("-") );
	}
	else
	{
		CDB_TecBox *pclTecBox = (CDB_TecBox *)( pclSSel->GetTecBoxCompTransfIDPtr().MP );
		IDPTR PipeDNIDPTR = pclPMInputUser->GetExpansionPipeSizeIDPtr( pclTecBox, 10.0 );
		CDB_StringID *pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

		if( NULL != pPipeDN )
		{
			pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe10, lRow, pPipeDN->GetString() );
		}
		else
		{
			pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe10, lRow, _T("-") );
		}

		PipeDNIDPTR = pclPMInputUser->GetExpansionPipeSizeIDPtr( pclTecBox, 30.0 );
		pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

		if( NULL != pPipeDN )
		{
			pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe30, lRow, pPipeDN->GetString() );
		}
		else
		{
			pclSheet->SetStaticText( CD_PMWQ_IO_ExpPipe30, lRow, _T("-") );
		}
	}

}

void CContainerForExcelExport::FillProductParametersRow( CData *pclProduct, CDS_SSelPMaint *pclSSelPM, CDS_SSelAirVentSeparator *pclSSelSep, CDS_SSelSafetyValve *pclSSelSafVal, CSSheet *pclSheet, long lRow )
{
	if( NULL == pclProduct || NULL == pclSheet )
	{
		return;
	}

	CString str = CteEMPTY_STRING;
	long lCurrentCol = CD_PMWQ_LAST + 1;

	if( NULL != pclSSelPM )
	{
		//CDS_SSelPMaint* pclSSelPressMaint = dynamic_cast<CDS_SSelPMaint *>( pclSSel );
		// vessel
		////////////////////
		if( NULL != dynamic_cast<CDB_Vessel *>( pclProduct ) )
		{
			CDB_Vessel *pclVessel = (CDB_Vessel *)( pclProduct );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELCON );
			str += CString( _T(" = ") ) + ( (CDB_StringID *)( pclVessel->GetConnectIDPtr().MP ) )->GetString();
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELNOMVOL );
			str += CString( _T(" = ") ) + WriteCUDouble( _U_VOLUME, pclVessel->GetNominalVolume(), true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			bool bCHVersion = ( 0 == TASApp.GetTADBKey().CompareNoCase( _T("CH") ) ) ? true : false;
			int iID = ( true == bCHVersion ) ? IDS_SSHEETSSELPROD_CPM_VESSELMAXPRESSCH : IDS_SSHEETSSELPROD_CPM_VESSELMAXPRESS;
			str = TASApp.LoadLocalizedString( iID );
			double dPS = ( true == bCHVersion ) ? pclVessel->GetPSch() : pclVessel->GetPmaxmax();
			str += CString( _T(" = ") ) + WriteCUDouble( _U_PRESSURE, dPS, true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELTEMPRANGE );
			str += CString( _T(" = " ) ) + pclVessel->GetTempRange() + _T( " ") + GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str();
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			// Weight.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELWEIGHT );
			str += CString( _T(" = ") ) + WriteCUDouble( _U_MASS, pclVessel->GetWeight(), true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			// Max weight.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELMAXWEIGHT );
			double dVesselMaxWeight = pclSSelPM->GetpInputUser()->GetMaxWeight( pclVessel->GetWeight(), pclVessel->GetNominalVolume(),
				pclSSelPM->GetpInputUser()->GetMinTemperature() );
			str += CString( _T(" = ") ) + WriteCUDouble( _U_MASS, dVesselMaxWeight, true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );
		}
		else if( ( NULL != dynamic_cast<CDB_TecBox *>( pclProduct ) ) || ( NULL != dynamic_cast<CDB_TBPlenoVento *>( pclProduct ) ) )
		{
			CDB_TecBox *pclTechBox = (CDB_TecBox *)( pclProduct );

			///Vento // Pleno // Compresso // Transfero///
			//////////////////////////////////////////////

			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPS );
			str += CString( _T(" = ") ) + WriteCUDouble( _U_PRESSURE, pclTechBox->GetPmaxmax(), true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPOWER );
			str += CString( _T(" = ") ) + WriteCUDouble( _U_ELEC_POWER, pclTechBox->GetPower(), true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXSV );
			str += CString( _T(" = ") ) + pclTechBox->GetPowerSupplyStr();
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );
		}
		else if( NULL != (CDB_PlenoRefill *)( pclProduct ) )
		{
			/// Pleno refill
			//////////////////
			CDB_PlenoRefill* pclPlenoRefill = (CDB_PlenoRefill *)( pclProduct );
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_PRCAPACITY );
			str += CString( _T(" = ") ) + pclPlenoRefill->GetCapacityStr( true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CPHEIGHT );
			str += CString( _T(" = ") ) + WriteCUDouble( _U_LENGTH, pclPlenoRefill->GetHeight(), true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );

			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CPMASS );
			str += CString( _T(" = ") ) + WriteCUDouble( _U_MASS, pclPlenoRefill->GetWeight(), true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str );
		}
	}
	else if( NULL != pclSSelSep )
	{
		//air vent
		if( NULL != dynamic_cast<CDB_AirVent *>(pclProduct) )
		{
			CDB_AirVent *pclAirVent = (CDB_AirVent *)pclProduct;
			CString str1 = TASApp.LoadLocalizedString( IDS_SELP_DPP );
			str1 += CString( _T(" = ") ) + WriteCUDouble( _U_DIFFPRESS, pclAirVent->GetDpp(), true );
			pclSheet->SetStaticText( ++lCurrentCol, lRow, str1 );
		}

		//// Separator & air vent
		/////////////////////////
		CDB_TAProduct *pclAirVentSeparator = (CDB_TAProduct *)pclProduct;

		str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_CONNECT );
		str += CString( _T(" = ") ) + ( (CDB_StringID *)( pclAirVentSeparator->GetConnectIDPtr().MP ) )->GetString();
		pclSheet->SetStaticText( ++lCurrentCol, lRow, str );
		
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_VERSION );
		str += CString( _T(" = ") ) + ( (CDB_StringID *)( pclAirVentSeparator->GetVersionIDPtr().MP ) )->GetString();
		pclSheet->SetStaticText( ++lCurrentCol, lRow, str );
		
		CString str2 = TASApp.LoadLocalizedString( IDS_PN );
		CString str1 = str2 + _T(" ");
		str2 = pclAirVentSeparator->GetPN().c_str();
		str1 += str2;
		str1 += CString( "; " );

		if( pclAirVentSeparator->GetTmin() > -273.15 )
		{
			str1 += WriteCUDouble( _U_TEMPERATURE, pclAirVentSeparator->GetTmin(), false == ( pclAirVentSeparator->GetTmax() < DBL_MAX ) );
		}
		else
		{
			str1 += CString( "- " );
		}

		str1 += CString( "/" );

		if( pclAirVentSeparator->GetTmax() < DBL_MAX )
		{
			str1 += WriteCUDouble( _U_TEMPERATURE, pclAirVentSeparator->GetTmax(), true );
		}
		else
		{
			str1 += CString( " -" );
		}

		pclSheet->SetStaticText( ++lCurrentCol, lRow, str1 );
	}
	else if( NULL != pclSSelSafVal )
	{
		//// Safety valve
		////////////////////
		CDB_SafetyValveBase *pclSafetyValve = (CDB_SafetyValveBase *)pclProduct;
		
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_SIZE );
		str += CString( _T(" = ") ) + pclSafetyValve->GetSize();
		pclSheet->SetStaticText( ++lCurrentCol, lRow, str );
		
		str = TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_CONN_IN_OUT );
		str += CString( _T(" = ") ) + pclSafetyValve->GetInOutletConnectionSizeString();
		pclSheet->SetStaticText( ++lCurrentCol, lRow, str );
	}
}

void CContainerForExcelExport::AddLineSeparator( CSSheet *pclSheet, RowDescription_LineSeparator eLine, long *plRow )
{
	if( NULL == pclSheet )
	{
		return;
	}

	long lRow = *plRow;

	CString str = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
	CString str1 = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMDIRECTSEL );
	
	switch( eLine )
	{
		case CContainerForExcelExport::Individual_Sel:
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
			pclSheet->AddCellSpan( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, 1 );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, str );
			pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, _LIGHTYELLOW );
			lRow++;
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
			break;

		case CContainerForExcelExport::Direct_Sel:
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
			pclSheet->AddCellSpan( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, 1 );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, str1 );
			pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, _LIGHTYELLOW );
			lRow++;
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
			break;

		case CContainerForExcelExport::HMCalc_Sel:
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
			pclSheet->AddCellSpan( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, 1 );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, IDS_HYDRAULIC_NETWORK );
			pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, _LIGHTYELLOW );
			lRow++;
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
			break;

		case CContainerForExcelExport::Pressurisation:
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
			pclSheet->AddCellSpan( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, 1 );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, IDS_LEFTTABTITLE_PRESSURISATION );
			pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, _YELLOW );
			lRow++;
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
			break;

		case CContainerForExcelExport::SeparatorAirVent:
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
			pclSheet->AddCellSpan( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, 1 );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, IDS_TABCDIALOGSELP_AIRVENTSEPARATOR );
			pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, _YELLOW );
			lRow++;
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
			break;

		case CContainerForExcelExport::SafetyValve:
			pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
			pclSheet->SetCellBorder( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_FINE_SOLID, _BLACK );
			pclSheet->AddCellSpan( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, 1 );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			pclSheet->SetStaticText( CD_PMWQ_Ref1, lRow, IDS_TABCDIALOGSELP_SAFETYVALVE );
			pclSheet->SetBackColor( CD_PMWQ_Ref1, lRow, CD_PMWQ_LAST + 7, lRow, _YELLOW );
			lRow++;
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
			break;

		default:
			break;
	}
	
	*plRow = lRow;
}

void CContainerForExcelExport::FillAccessoriesPM( CSSheet *pclSheet, long *plRow, CAccessoryList *pclAccessoryList, int iGroupQuantity, int iDistributedQty )
{
	if( NULL == pclSheet || NULL == pclAccessoryList || 0 == pclAccessoryList->GetCount() )
	{
		return;
	}

	long lRow = *plRow;

	CRank rkList;
	CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

	while( rAccessoryItem.IDPtr.MP != NULL )
	{
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
		VERIFY( NULL != pclAccessory );

		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			break;
		}

		CAccessoryList::AccessoryItem *pclAccessoryItem = new CAccessoryList::AccessoryItem();

		if( NULL == pclAccessoryItem )
		{
			break;
		}

		*pclAccessoryItem = rAccessoryItem;
		rkList.AddStrSort( pclAccessory->GetName(), 0, (LPARAM)pclAccessoryItem, false, false );
		rAccessoryItem = pclAccessoryList->GetNext();
	}

	CString str;
	LPARAM lParam;
	int iNbrAccessories = 0;

	for( BOOL bContinue = rkList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rkList.GetNext( str, lParam ) )
	{
		CAccessoryList::AccessoryItem *pclAccessoryItem = ( CAccessoryList::AccessoryItem * )lParam;
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
		ASSERT( NULL != pclAccessory );

		bool bByPair = pclAccessoryItem->fByPair;
		bool bDistributed = pclAccessoryItem->fDistributed;

		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			continue;
		}

		int iTotalQuantity = 1;

		// HYS-987: Handle editable accessory quantity
		if( -1 != pclAccessoryItem->lEditedQty )
		{
			iTotalQuantity = pclAccessoryItem->lEditedQty;
		}
		else if( true == bByPair )
		{
			iTotalQuantity = 2;
			iTotalQuantity *= iGroupQuantity;
		}
		else if( iDistributedQty > 1 && true == bDistributed )
		{
			iTotalQuantity = iDistributedQty;
			iTotalQuantity *= iGroupQuantity;
		}
		else
		{
			iTotalQuantity *= iGroupQuantity;
		}

		delete pclAccessoryItem;

		pclSheet->SetMaxRows( pclSheet->GetMaxRows() + 1 );
		pclSheet->SetStaticText( CD_PMWQ_Qty, lRow, (CString)WriteDouble( iTotalQuantity, 1 ) );

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		pclSheet->SetStaticText( CD_PMWQ_ProductName, lRow, pclAccessory->GetName() );
		pclSheet->SetStaticText( CD_PMWQ_ProductFunction, lRow, pclAccessory->GetComment() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		FillArtNumberCol( pclSheet, CD_PMWQ_ArticleNumber, lRow, pclAccessory, pclAccessory->GetArtNum() );
		lRow++;
	}
	
	*plRow = lRow-1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CContainerForPrint - class helper for printing.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CContainerForPrint::HasSomethingToPrint( CDB_PageSetup::enCheck ePage )
{
	if( NULL == GetSafeHwnd() || NULL == GetpPage( ePage ) )
	{
		return false;
	}

	bool bHasSomethingToPrint = false;

	if( CDB_PageSetup::enCheck::ARTLIST != ePage && CDB_PageSetup::enCheck::TENDERTEXT != ePage)
	{
		bHasSomethingToPrint = GetpPage( ePage )->HasSomethingToPrint();
	}
	else
	{
		// HYS-1830: Add SMARTCONTROLVALVE, TAPWATERCONTROL, FLOORHEATINGCONTROL
		CDB_PageSetup::enCheck arrayEncheck[] = { CDB_PageSetup::AIRVENTSEPARATOR,
												  CDB_PageSetup::PRESSMAINT,
												  CDB_PageSetup::SAFETYVALVE,
												  CDB_PageSetup::BV,
												  CDB_PageSetup::DPC,
												  CDB_PageSetup::DPCBCV,
												  CDB_PageSetup::CV,
												  CDB_PageSetup::SIXWAYVALVE,
												  CDB_PageSetup::SV,
												  CDB_PageSetup::TRV,
												  CDB_PageSetup::HUB,
												  CDB_PageSetup::PARTDEF,
												  CDB_PageSetup::PIPELIST,
												  CDB_PageSetup::TENDERTEXT,
												  CDB_PageSetup::SMARTCONTROLVALVE,
												  CDB_PageSetup::TAPWATERCONTROL,
												  CDB_PageSetup::FLOORHEATINGCONTROL,
												  CDB_PageSetup::SMARTDPC,
												};

		for( int i = 0; i < sizeof( arrayEncheck ) / sizeof( arrayEncheck[0] ); ++i )
		{
			if( NULL != GetpPage( arrayEncheck[i] ) )
			{
				bHasSomethingToPrint = GetpPage( arrayEncheck[i] )->HasSomethingToPrint();
			}

			if( true == bHasSomethingToPrint )
			{
				break;
			}
		}
	}

	return bHasSomethingToPrint;
}

bool CContainerForPrint::BeginPrinting( void )
{
	return PrepareMultiSheetInOne( true, false );
}

CRect CContainerForPrint::GetSheetSizeInPixels( CDB_PageSetup::enCheck ePage )
{
	if( NULL == GetSafeHwnd() || NULL == GetpPage( ePage ) )
	{
		return CRect( 0 );
	}

	CRect rectSize = CRect( 0 );

	if( false == GetpPage( ePage )->UseOnlyOneSpread() )
	{
		if( 0 == m_mapAllInOneSheetList.count( ePage ) || NULL == m_mapAllInOneSheetList[ePage] )
		{
			return CRect( 0 );
		}

		rectSize = m_mapAllInOneSheetList[ePage]->GetSheetSizeInPixels();
	}
	else
	{
		if( NULL != GetpPage( ePage )->GetSheetPointer() )
		{
			rectSize = GetpPage( ePage )->GetSheetPointer()->GetSheetSizeInPixels();
		}
	}

	return rectSize;
}

void CContainerForPrint::SetPageBreak( CDB_PageSetup::enCheck ePage, CDC *pDC, CRect rect, double dScaleRatio )
{
	if( NULL == GetSafeHwnd() || NULL == GetpPage( ePage ) )
	{
		return;
	}

	if( false == GetpPage( ePage )->UseOnlyOneSpread() )
	{
		if( 0 == m_mapAllInOneSheetList.count( ePage ) || NULL == m_mapAllInOneSheetList[ePage] )
		{
			return;
		}

		// Optimize columns size.
		double dHeaderHeight = 0.0;
		CRect r;
		long lRow;

		// Once all sheet has been merged into one in 'm_pclAllInOneSheet', all columns have the same size.
		double dColWidth = m_mapAllInOneSheetList[ePage]->GetColWidthW( 1 );

		for( long lLoopColumn = 2; lLoopColumn < m_mapAllInOneSheetList[ePage]->GetMaxCols() - 1; lLoopColumn++ )
		{
			m_mapAllInOneSheetList[ePage]->SetColWidth( lLoopColumn, dColWidth * dScaleRatio );
		}

		double dRowsHeight = 0;
		long lFirstRow = 0;

		for( lRow = 1; lRow <= m_mapAllInOneSheetList[ePage]->GetMaxRows(); )
		{
			r = m_mapAllInOneSheetList[ePage]->GetSelectionInPixels( 1, lRow, 1, lRow );
			dRowsHeight += r.Height();

			if( dRowsHeight > rect.Height() )
			{
				// Next page set a page break 1St row before
				m_mapAllInOneSheetList[ePage]->SetRowPageBreak( lFirstRow, true );
				dRowsHeight = 0;
				lRow = lFirstRow;
			}
			else
			{
				bool bCurrentBreakable = m_mapAllInOneSheetList[ePage]->IsFlagRowNoBreakable( lRow );

				// If no change...
				if( false == bCurrentBreakable )
				{
					lFirstRow = lRow;
				}

				lRow++;
			}
		}
	}
	else
	{
		CSSheet *pclSheet = GetpPage( ePage )->GetSheetPointer();

		if( NULL == pclSheet )
		{
			return;
		}

		// Here we go from column after header (1)  to the column before the footer (maxCols - 1).
		double dColWidth;

		for( long lLoopColumn = 2; lLoopColumn < pclSheet->GetMaxCols() - 1; lLoopColumn++ )
		{
			pclSheet->GetColWidth( lLoopColumn, &dColWidth );
			pclSheet->SetColWidth( lLoopColumn, dColWidth * dScaleRatio );
		}

		// Header is frozen. We must take into account this part.
		double dHeaderHeight = 0.0;

		for( long lRow = 1; lRow <= pclSheet->GetRowsFreeze(); lRow++ )
		{
			CRect rectSize = pclSheet->GetSelectionInPixels( 1, lRow, 1, lRow );
			dHeaderHeight += rectSize.Height();
		}

		double dRowsHeight = dHeaderHeight;
		long lFirstRow = pclSheet->GetRowsFreeze() + 1;

		for( long lRow = lFirstRow; lRow <= pclSheet->GetMaxRows(); )
		{
			CRect rectSize = pclSheet->GetSelectionInPixels( 1, lRow, 1, lRow );
			dRowsHeight += rectSize.Height();

			if( dRowsHeight > rect.Height() )
			{
				// HYS-1303 : print crash for tender text
				if( lRow - lFirstRow <= 1 )// if one row won't fit on a page
				{
					lFirstRow++;
				}
				
				// Next page set a page break just before 'lFirstRow'.
				pclSheet->SetRowPageBreak( lFirstRow, true );
				dRowsHeight = dHeaderHeight;
				lRow = lFirstRow;
			}
			else
			{
				bool bCurrentBreakable = pclSheet->IsFlagRowNoBreakable( lRow );

				// If no change...
				if( false == bCurrentBreakable )
				{
					lFirstRow = lRow;
				}

				lRow++;
			}
		}
	}
}

void CContainerForPrint::PrintPageCount( CDB_PageSetup::enCheck ePage, CDC *pDC, CRect &PrintRect, long &lPageCount )
{
	lPageCount = 0;

	if( NULL == GetSafeHwnd() || NULL == GetpPage( ePage ) )
	{
		return;
	}

	if( false == GetpPage( ePage )->UseOnlyOneSpread() )
	{
		if( 0 == m_mapAllInOneSheetList.count( ePage ) || NULL == m_mapAllInOneSheetList[ePage] )
		{
			return;
		}

		m_mapAllInOneSheetList[ePage]->OwnerPrintPageCount( pDC, PrintRect, lPageCount );
	}
	else
	{
		CSSheet *pclSheet = GetpPage( ePage )->GetSheetPointer();

		if( NULL == pclSheet )
		{
			return;
		}

		pclSheet->OwnerPrintPageCount( pDC, PrintRect, lPageCount );
	}
}

CSSheet *CContainerForPrint::GetSheetToPrint( CDB_PageSetup::enCheck ePage )
{
	if( NULL == GetpPage( ePage ) )
	{
		return NULL;
	}

	if( false == GetpPage( ePage )->UseOnlyOneSpread() )
	{
		if( 0 == m_mapAllInOneSheetList.count( ePage ) || NULL == m_mapAllInOneSheetList[ePage] )
		{
			return NULL;
		}

		return m_mapAllInOneSheetList[ePage];
	}
	else
	{
		return GetpPage( ePage )->GetSheetPointer();
	}
}
