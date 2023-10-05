#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>

#include "TASelect.h"
#include "MainFrm.h"
#include "utilities.h"
#include "DlgDirSel.h"
#include "DlgConfSel.h"
#include "DlgTender.h"

IMPLEMENT_DYNAMIC( CDlgDirSel, CDialogEx )

CDlgDirSel::CDlgDirSel( CWnd *pParent )
	: CDialogEx( CDlgDirSel::IDD, pParent )
{
	m_bWorkingForDpCSet = false;
	m_bWorkingForControlSet = false;
	m_hParentItem = NULL;
}

CDlgDirSel::~CDlgDirSel()
{
	CleanVectorOfData( CTreeItemData::DataType::All );
}

// CDlgDirSel message handlers
BEGIN_MESSAGE_MAP( CDlgDirSel, CDialogEx )
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREECATEGORY, OnTvnSelChangedTreeCategory )
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREEPRODUCTS, OnTvnSelChangedTreeProducts )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_TREECATEGORY, OnNMEnterTreeCategory )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_TREEPRODUCTS, OnNMEnterTreeProducts )
	ON_NOTIFY( NM_CLICK, IDC_TREEPRODUCTS, OnNMClickTreeProducts )
//	ON_NOTIFY( NM_SETFOCUS, IDC_TREEPRODUCTS, OnNMSetFocusTreeProducts )
	ON_NOTIFY( NM_KILLFOCUS, IDC_TREEPRODUCTS, OnNMKillFocusTreeProducts )
	ON_BN_CLICKED( IDOK, OnBnClickedSelect )
	ON_BN_CLICKED( IDCANCEL, OnBnClickedCancel )
	ON_BN_CLICKED( IDC_BUTTONDIRSELEXP, OnBnClickedDirSelExport )
	ON_BN_CLICKED( IDC_DSEL_TENDER, OnBnClickedDirSelTender )
	ON_BN_CLICKED( IDC_CHECKDELETEDVALVES, OnBnClickedDeletedValves )
	ON_EN_CHANGE(IDC_EDITSEARCHPROD, &CDlgDirSel::OnEnChangeEditsearchprod)
	ON_CBN_SELCHANGE(IDC_COMBOSELECTION, &CDlgDirSel::OnCbnSelchangeComboselection)
	ON_BN_CLICKED(IDC_BUTTONSEARCH, &CDlgDirSel::OnBnClickedButtonsearch)
END_MESSAGE_MAP()

void CDlgDirSel::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TREECATEGORY, m_TreeProdCateg );
	DDX_Control( pDX, IDC_TREEPRODUCTS, m_TreeProducts );
	DDX_Control( pDX, IDC_STATICIMG, m_staticImg );
	DDX_Control( pDX, IDC_LIST1, m_List_ArticleDetails );
	DDX_Control( pDX, IDC_LISTFOCUS, m_ListBoxFocus );
	DDX_Control( pDX, IDC_CHECKDELETEDVALVES, m_CBDeletedValves );
	DDX_Control( pDX, IDC_EDITSEARCHPROD, m_EditSearchProduct );
	DDX_Control( pDX, IDC_COMBOSELECTION, m_ComboResults );
	DDX_Control( pDX, IDC_BUTTONSEARCH, m_btnSearch );
	DDX_Control( pDX, IDOK, m_btnOK );
}

BOOL CDlgDirSel::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Reset pointers on 'Database' and 'TADataStruct' to those ones by default.
	m_clDirectSelectionParams.m_pTADB = TASApp.GetpTADB();
	m_clDirectSelectionParams.m_pTADS = TASApp.GetpTADS();
	m_clDirectSelectionParams.m_pUserDB = TASApp.GetpUserDB();
	m_clDirectSelectionParams.m_pPipeDB = TASApp.GetpPipeDB();

	// Clean all selected object in the TreeProducts.
	m_ItemProductSelected.Clean();
	m_ItemAdapterSelected.Clean();
	m_ItemActuatorSelected.Clean();
    // HYS-945: Buffer vessel management
	m_ItemIntegratedSelected.Clean();
	m_bSkipTvnSelChangedProduct = false;

#ifdef DEBUG
	GetDlgItem( IDC_BUTTONDIRSELEXP )->ShowWindow( SW_SHOW );
#endif
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_CAPTION );
	SetWindowText(str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHGEN_DELETEDVALVES );
	GetDlgItem( IDC_CHECKDELETEDVALVES )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_STATICARTICLENUMBER );
	GetDlgItem( IDC_STATICARTICLENBR )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_STATICRESULTS );
	GetDlgItem( IDC_STATICRESULTS )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_BUTTONSEARCH );
	m_btnSearch.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_STATICCATEGORY );
	GetDlgItem( IDC_STATICCATEGORY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_STATICPRODUCTS );
	GetDlgItem( IDC_STATICPRODUCTS )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_STATICARTICLEDETAILS );
	GetDlgItem( IDC_STATICARTICLEDETAILS )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_BUTTONSELECT );
	GetDlgItem( IDOK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_BUTTONCLOSE );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_TENDERTEXT );
	GetDlgItem( IDC_DSEL_TENDER )->SetWindowText( str );

	if( false == TASApp.GetTender().IsTenderNeeded() )
	{
		GetDlgItem( IDC_DSEL_TENDER )->ShowWindow( SW_HIDE );
	}

	// Build and attach an image list to 'm_Tree'.
	m_TreeProdImgList.Create( IDB_ILDLGDIRSEL, 16, 1, _BLACK );
	m_TreeProdImgList.SetBkColor( CLR_NONE );

	// Refresh the list control.
	m_List_ArticleDetails.DeleteAllItems();
	m_List_ArticleDetails.DeleteColumn( 1 );
	m_List_ArticleDetails.DeleteColumn( 0 );

	// Insert the name of the first column.
	str.Format( TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ARTICLENUMB ), 0 );

	// Insert a column to the left with a width of 85 pixels.
	m_List_ArticleDetails.InsertColumn( 0, str, LVCFMT_LEFT, 93, 0 );

	// Insert the name of the second column.
	str.Format( TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ARTICLEDETAILS ), 1 );

	// Insert a column to the left with a width of 120 pixels.
	m_List_ArticleDetails.InsertColumn( 1, str, LVCFMT_LEFT, 236, 0 );

	// Set a good viewing style.
	m_List_ArticleDetails.ModifyStyle( 0, LVS_REPORT ); 
	// Get a full row selected if the click isn't disable.
	m_List_ArticleDetails.SetExtendedStyle( m_List_ArticleDetails.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	// Disable the possibility for someone to click on the articles details.
	m_List_ArticleDetails.SetClickListCtrlDisable( true );

	// HYS-1398 : We retrieve the check box value, saved previously.
	int iCheck = AfxGetApp()->GetProfileInt( _T( "DialogDirSel" ), _T( "ShowOldValves" ), BST_UNCHECKED );
	m_CBDeletedValves.SetCheck( iCheck );
	CDS_PersistData* pPD = TASApp.GetpTADS()->GetpPersistData();
	ASSERT( NULL != pPD );

	if( pPD != NULL )
	{
		// Save that deleted valves can be selected or not.
		if( BST_CHECKED == m_CBDeletedValves.GetCheck() )
		{
			pPD->SetOldValveUsed( true );
		}
		else
		{
			pPD->SetOldValveUsed( false );
		}
	}

	// Initialize the list focus
	// Do the same list control as the previous so we can use functions without changing anything.
	str.Format( TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ARTICLENUMB ), 0 );
	m_ListBoxFocus.InsertColumn( 0, str, LVCFMT_LEFT, 0, 0 );
	
	str.Format( TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ARTICLEDETAILS ), 1 );
	m_ListBoxFocus.InsertColumn( 1, str, LVCFMT_CENTER, 236, 0 );
	m_ListBoxFocus.ModifyStyle( 0, LVS_REPORT ); 
	m_ListBoxFocus.SetExtendedStyle( m_List_ArticleDetails.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_ListBoxFocus.SetClickListCtrlDisable( true );

	// Fill the tree with the CATEGORY_TAB data base.
	FillTreeProductCategories();

	//HYS-791: Tool tip to help for search
	CString strTT = _T("");
	strTT = TASApp.LoadLocalizedString(IDS_DLGDIRSEL_TTSEARCH);
	m_ToolTip.Create(this, TTS_NOPREFIX);
	m_ToolTip.AddToolWindow(GetDlgItem(IDC_EDITSEARCHPROD), strTT);
	m_ToolTip.AddToolWindow(GetDlgItem(IDC_COMBOSELECTION), strTT);
	m_ToolTip.AddToolWindow(GetDlgItem(IDC_BUTTONSEARCH), strTT);

	// By default the 'Search' button is disabled.
	m_btnSearch.EnableWindow( FALSE );
	
	// HYS-1331 : To manage bitmap replacement. If the background color is not defined 
	// OnCtlColor consider the object as transparent so the image stays visible.
	m_staticImg.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

	return TRUE;
}

void CDlgDirSel::OnTvnSelChangedTreeCategory( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>( pNMHDR );
	
	// Delete all items to be sure an empty family won't let previous products been visible.
	// Remark: block redrawing in all controls because at each deletion, an event is sent.
	m_TreeProducts.SetRedraw( FALSE );
	m_staticImg.SetRedraw( FALSE );
	m_ListBoxFocus.SetRedraw( FALSE );
	m_List_ArticleDetails.SetRedraw( FALSE );

	// HYS-945: To avoid notification of OnTvnSelChangedTreeProducts when deleting all items.
	m_bSkipTvnSelChangedProduct = true;
	m_TreeProducts.DeleteAllItems();
	m_bSkipTvnSelChangedProduct = false;

	m_TreeProducts.SetRedraw( TRUE );
	m_staticImg.SetRedraw( TRUE );
	m_ListBoxFocus.SetRedraw( TRUE );
	m_List_ArticleDetails.SetRedraw( TRUE );

	m_TreeProducts.ResetAllColors();
	m_List_ArticleDetails.DeleteAllItems();
	
	// Remove the image.
	m_staticImg.SetBitmap( (HBITMAP)NULL );

	// Clean all values.
	m_ItemProductSelected.Clean();
	m_ItemAdapterSelected.Clean();
	m_ItemActuatorSelected.Clean();

	// HYS-945: Buffer vessel management
	m_ItemIntegratedSelected.Clean();

	CleanVectorOfSelectedAccessories( &m_vecProductAccessorySelected );
	CleanVectorOfSelectedAccessories( &m_vecSubProductAccessorySelected );
	CleanVectorOfSelectedAccessories( &m_vecActuatorAccessorySelected );
	CleanVectorOfSelectedAccessories( &m_vecIntegratedAccessorySelected );
	CleanVectorOfData( CTreeItemData::DataType::AllMiddle );

	m_bWorkingForDpCSet = false;
	m_bWorkingForControlSet = false;

	CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( pNMTreeView->itemNew.hItem );

	if( NULL != pclTreeItemData && CTreeItemData::Family == pclTreeItemData->m_eDataType )
	{
		CTreeItemFamily *pclTreeItemFamily = (CTreeItemFamily *)pclTreeItemData;
		CTable *pTab = dynamic_cast<CTable *>( pclTreeItemFamily->m_pclFamilyID->GetIDPtr().PP );

		// Check if we are in the actuator category.
		if( 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ACTUATOR") ) )
		{
			FillTP_Actuators( pclTreeItemFamily );
		}
		else if( 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_MEASTOOLS") ) )
		{
			// Fill tree product measurig tools
			FillTP_MeasTools( pclTreeItemFamily );
		}
		else
		{
			// Identify if we are working for a set.
			if( 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_DPCONTRSET") ) )
			{
				m_bWorkingForDpCSet = true;
			}
			else if( 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_STDCVACTSET") ) )
			{
				m_bWorkingForControlSet = true;
			}
			else if( 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ADVCTRLVSET") ) )
			{
				m_bWorkingForControlSet = true;
			}

			// Fill the right tree ('Products').
			FillTP_Products( pclTreeItemFamily, m_bWorkingForDpCSet || m_bWorkingForControlSet );
		}
	}

	_UpdateSelectButton();
	
	// Reset the list focus.
	m_ListBoxFocus.DeleteAllItems();
	*pResult = 0;
}

void CDlgDirSel::OnNMEnterTreeCategory( NMHDR *pNMHDR, LRESULT *pResult )
{
	*pResult = 0;

	if( NULL != m_TreeProducts.GetRootItem() )
	{
		m_TreeProducts.SetFocus();
	}
}

void CDlgDirSel::OnNMEnterTreeProducts( NMHDR *pNMHDR, LRESULT *pResult )
{
	// On a TreeView Control with the CheckBoxes, there is no notification that the check state of the item has been changed, 
	// you can just determine that the user has clicked the state Icon.
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is post a user defined message
	// as a notification that the check state has changed.
	
	HTREEITEM hItem = m_TreeProducts.GetSelectedItem();		
	
	// User press enter on a selected item.
	if( NULL != hItem )
	{
		ClickTreeProducts( hItem );
	}

	_UpdateSelectButton();

	*pResult = 0;
}

void CDlgDirSel::OnNMClickTreeProducts( NMHDR *pNMHDR, LRESULT *pResult )
{
	// On a TreeView Control with the CheckBoxes, there is no notification that the check state of the item has been changed,
	// you can just determine that the user has clicked the state Icon.
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is to post a user defined message
	// as a notification that the check state has changed.

	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x= ( (int)(short)LOWORD( dwpos ) );				// GET_X_LPARAM(dwpos);
	ht.pt.y= ( (int)(short)HIWORD( dwpos ) );				// GET_Y_LPARAM(dwpos);

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );
	
	// User click on an item or select it with the arrows.
	if( NULL != ht.hItem && ( ( TVHT_ONITEMICON & ht.flags ) || ( TVHT_ONITEM & ht.flags ) ) )
	{
		ClickTreeProducts( ht.hItem );
	}

	_UpdateSelectButton();
	
	*pResult = 0;
}

void CDlgDirSel::OnTvnSelChangedTreeProducts( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if( NULL == pNMTreeView->itemNew.hItem )
	{
		return;
	}

	// HYS-945: To avoid notification of OnTvnSelChangedTreeProducts when deleting all items
	if( true == m_bSkipTvnSelChangedProduct )
	{
		return;
	}
	CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( pNMTreeView->itemNew.hItem );

	if( NULL == pclTreeItemData )
	{
		return;
	}

	switch( pclTreeItemData->m_eDataType )
	{
		case CTreeItemData::DataType::Product:
			{
				CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( pclTreeItemData );
				
				// HYS-1381: For example: We select a set product, the product image is displayed, we change the actuator
				// and click again on the product => Result : The product is unselected and the product image should not be displayed.
				HTREEITEM hCurrentItem = m_TreeProducts.GetSelectedItem();
				int iImage = -1;
				int iImageSelected = -1;
				m_TreeProducts.GetItemImage( hCurrentItem, iImage, iImageSelected );
				
				if( NULL != pclTreeItemProduct && NULL != pclTreeItemProduct->m_pclProduct && RadioNotSelected != iImage )
				{
					m_ListBoxFocus.DeleteAllItems();
					FillProductDetails( pclTreeItemProduct->m_pclProduct, &m_ListBoxFocus );
				}
			}
			break;

		case CTreeItemData::DataType::Accessory:
			{
				CTreeItemAccessory *pclTreeItemAccessory = dynamic_cast<CTreeItemAccessory *>( pclTreeItemData );

				if( NULL != pclTreeItemAccessory && NULL != pclTreeItemAccessory->m_pclAccessory && pclTreeItemAccessory->m_pclAccessory->IsAnAccessory() )
				{
					m_ListBoxFocus.DeleteAllItems();
					FillAccessoryDetails( pclTreeItemAccessory->m_pclAccessory, &m_ListBoxFocus );
				}
			}
			break;

		case CTreeItemData::DataType::Adapter:
			{
				CTreeItemAdapter *pclTreeItemAdapter = dynamic_cast<CTreeItemAdapter *>( pclTreeItemData );

				if( NULL != pclTreeItemAdapter && NULL != pclTreeItemAdapter->m_pclAdapter )
				{
					m_ListBoxFocus.DeleteAllItems();
					FillAdapterDetails( pclTreeItemAdapter->m_pclAdapter, &m_ListBoxFocus );
				}
			}
			break;

		case CTreeItemData::DataType::Actuator:
			{
				CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( pclTreeItemData );

				if( NULL != pclTreeItemActuator && NULL != pclTreeItemActuator->m_pclActuator )
				{
					m_ListBoxFocus.DeleteAllItems();
					FillActuatorDetails( pclTreeItemActuator->m_pclActuator, &m_ListBoxFocus );
				}
			}
			break;

		// HYS-945: Buffer vessel management
		case CTreeItemData::DataType::Integrated:
			{
				CTreeItemIntegrated *pclTreeItemIntegrated = dynamic_cast<CTreeItemIntegrated *>( pclTreeItemData );

				if( NULL != pclTreeItemIntegrated && NULL != pclTreeItemIntegrated->m_pclIntegrated )
				{
					m_ListBoxFocus.DeleteAllItems();
					FillProductDetails( pclTreeItemIntegrated->m_pclIntegrated, &m_ListBoxFocus );
				}
			}
			break;
	}

	_UpdateSelectButton();
}

void CDlgDirSel::OnNMKillFocusTreeProducts( NMHDR *pNMHDR, LRESULT *pResult )
{
	// Reset the list focus.
	m_ListBoxFocus.DeleteAllItems();
	*pResult = 0;
}

void CDlgDirSel::OnBnClickedSelect()
{
	try
	{
		if( NULL == m_clDirectSelectionParams.m_pTADB )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clDirectSelectionParams.m_pTADB' can't be NULL.") );
		}
		else if( NULL == m_clDirectSelectionParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clDirectSelectionParams.m_pTADS' can't be NULL.") );
		}

		PREVENT_ENTER_KEY

		// Process product.
		if( NULL != m_ItemProductSelected.m_hTreeItem )
		{
			CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItem ) );
			
			if( NULL == pclTreeItemProduct )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemProduct' from the tree.") );
			}
			else if( NULL == pclTreeItemProduct->m_pclProduct )
			{
				HYSELECT_THROW( _T("Internal error: Product in the 'CTreeItemProduct' object is not defined.") );
			}
			
			if( NULL != dynamic_cast<CDB_ThermostaticValve *>( pclTreeItemProduct->m_pclProduct )
					|| NULL != dynamic_cast<CDB_FlowLimitedControlValve *>( pclTreeItemProduct->m_pclProduct ) )
			{
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelRadSet ) );
				CDS_SSelRadSet *pSelRadSet = (CDS_SSelRadSet *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pSelRadSet->GetSupplyValveAccessoryList();
				CAccessoryList *pclActuatorAccessoryListToFill = NULL;
				pSelRadSet->SetFromDirSel( true );
				pSelRadSet->SetSupplyValveIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
				
				// Add actuator if exist.
				if( NULL != m_ItemActuatorSelected.m_hTreeItem )
				{
					CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem ) );
					
					if( NULL == pclTreeItemActuator )
					{
						HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemActuator' from the tree.") );
					}
					else if( NULL == pclTreeItemActuator->m_pclActuator )
					{
						HYSELECT_THROW( _T("Internal error: Actuator in the 'CTreeItemActuator' object is not defined.") );
					}

					pSelRadSet->SetSupplyValveActuatorIDPtr( pclTreeItemActuator->m_pclActuator->GetIDPtr() );
					// HYS-987: Display supply valve actuator accessories
					pclActuatorAccessoryListToFill = pSelRadSet->GetSVActuatorAccessoryList();
				}
				
				// Some accessories ?
				// Remark: pay attention that in this list we have also accessories that are excluded by the accessory chosen by user (see.h).
				for( int i = 0; i < (int)m_vecActuatorAccessorySelected.size(); i++ )
				{
					if( true == m_vecActuatorAccessorySelected[i].m_bCheck )
					{
						pclActuatorAccessoryListToFill->Add( m_vecActuatorAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecActuatorAccessorySelected[i].m_eAccessoryType, m_vecActuatorAccessorySelected[i].m_pclRuledTable );
					}
				}

				// Some accessories for RadSet.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}

				CString TabID = _T("RADSET_TAB");

				if( false == ConfirmSelection( pSelRadSet, TabID ) )
				{
					IDPTR idptr = pSelRadSet->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_ControlValve *>( pclTreeItemProduct->m_pclProduct ) )
			{
				IDPTR IDPtr;
				CDS_SSelCv *pSelCv = NULL;
				CAccessoryList *pclCvAccessoryListToFill = NULL;
				CAccessoryList *pclActuatorAccessoryListToFill = NULL;
				CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclTreeItemProduct->m_pclProduct );
				CString TabID = _T("CTRLVALVE_TAB");

				switch( pclControlValve->GetCVParentTable() )
				{
					case CDB_ControlValve::ControlValveTable::CV:
						m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelCv ) );
						break;
					
					case CDB_ControlValve::ControlValveTable::BCV:
						m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelBCv ) );
						break;
					
					case CDB_ControlValve::ControlValveTable::PICV:
						m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPICv ) );
						break;

					case CDB_ControlValve::ControlValveTable::DPCBCV:
						m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelDpCBCV ) );
						TabID = _T("DPCBCVALVE_TAB");
						break;
				}

				pSelCv = (CDS_SSelCv *)( IDPtr.MP );

				if( NULL == pSelCv )
				{
					// HYS-1241 : 6 way valve
					m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSel6WayValve ) );
					TabID = _T("6WAYCTRLVALV_TAB");
					( dynamic_cast<CDS_SSel6WayValve *>( IDPtr.MP ) )->SetSelectionMode( e6Way_Alone );
					pSelCv = (CDS_SSelCv *)( IDPtr.MP );
				}

				if( NULL == pSelCv )
				{
					return;
				}
				
				pclCvAccessoryListToFill = pSelCv->GetCvAccessoryList();
				pclActuatorAccessoryListToFill = pSelCv->GetActuatorAccessoryList();

				pSelCv->SetFromDirSel( true );
				pSelCv->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Add adapter if exist.
				if( NULL != m_ItemAdapterSelected.m_hTreeItem )
				{
					CTreeItemAdapter *pclTreeItemAdapter = dynamic_cast<CTreeItemAdapter *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemAdapterSelected.m_hTreeItem ) );

					if( NULL == pclTreeItemAdapter )
					{
						HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemAdapter' from the tree.") );
					}
					else if( NULL == pclTreeItemAdapter->m_pclAdapter )
					{
						HYSELECT_THROW( _T("Internal error: Adapter in the 'CTreeItemAdapter' object is not defined.") );
					}

					pclCvAccessoryListToFill->Add( pclTreeItemAdapter->m_pclAdapter->GetIDPtr(), CAccessoryList::AccessoryType::_AT_Adapter );
				}

				// Add actuator if exist.
				if( NULL != m_ItemActuatorSelected.m_hTreeItem )
				{
					CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem ) );

					if( NULL == pclTreeItemActuator )
					{
						HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemActuator' from the tree.") );
					}
					else if( NULL == pclTreeItemActuator->m_pclActuator )
					{
						HYSELECT_THROW( _T("Internal error: Actuator in the 'CTreeItemActuator' object is not defined.") );
					}
					
					pSelCv->SetActrIDPtr( pclTreeItemActuator->m_pclActuator->GetIDPtr() );

					if( true == m_bWorkingForControlSet )
					{
						// Try to find a CvActSet.
						CTableSet *pCVActSetTab = pclControlValve->GetTableSet();
						
						if( NULL == pCVActSetTab )
						{
							HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTableSet' object from the control valve '%s'."), pclControlValve->GetIDPtr().ID );
						}

						CDB_Set *pCVActSet = pCVActSetTab->FindCompatibleSet( pclTreeItemProduct->m_pclProduct->GetIDPtr().ID, pclTreeItemActuator->m_pclActuator->GetIDPtr().ID );

						if( pCVActSet != NULL )
						{
							pSelCv->SetSelectedAsAPackage( true );
						}
					}

					// Some accessories ?
					// Remark: pay attention that in this list we have also accessories that are excluded by the accessory chosen by user (see.h).
					for( int i = 0; i < (int)m_vecActuatorAccessorySelected.size(); i++ )
					{
						if( true == m_vecActuatorAccessorySelected[i].m_bCheck )
						{
							// Little exception compare to individual selection. For example, selection for butterfly valves are automatically consider as a selection by package. 
							// User chooses first the valve, accessories on valve and actuator are displayed below. It's only when user finishes selection that we display 
							// accessories on actuator and accessories that belongs to the package. In this case, accessories belonging to the set are added to the list 
							// linked to the valve and not to the actuator. In direct selection, we display accessory belonging to the package just under the accessories of 
							// the actuator. And these accessories are saved in 'm_vecActuatorAccessorySelected'. 
							if( m_vecActuatorAccessorySelected[i].m_eAccessoryType != CAccessoryList::AccessoryType::_AT_SetAccessory )
							{
								pclActuatorAccessoryListToFill->Add( m_vecActuatorAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecActuatorAccessorySelected[i].m_eAccessoryType, m_vecActuatorAccessorySelected[i].m_pclRuledTable );
							}
							else
							{
								pclCvAccessoryListToFill->Add( m_vecActuatorAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecActuatorAccessorySelected[i].m_eAccessoryType, m_vecActuatorAccessorySelected[i].m_pclRuledTable );
							}
						}
					}
				}
				// HYS-1877: For 6-way valve set, only one item is selected, the 6-way valve. So, m_ItemActuatorSelected, m_vecActuatorAccessorySelected etc... are not considered.
				else if( true == m_bWorkingForControlSet && 0 == StringCompare( TabID, _T( "6WAYCTRLVALV_TAB" ) ) )
				{
					// Find 6-way valve set
					CTableSet* pCVActSetTab = pclControlValve->GetTableSet();

					if( NULL == pCVActSetTab )
					{
						HYSELECT_THROW( _T( "Internal error: Can't retrieve the 'CTableSet' object from the control valve '%s'." ), pclControlValve->GetIDPtr().ID );
					}

					// We have just one actuator item linked to this 6-way valve so we put "" for actuator id.
					CDB_Set *pCVActSet = pCVActSetTab->FindCompatibleSet( pclTreeItemProduct->m_pclProduct->GetIDPtr().ID, _T("") );

					if( pCVActSet != NULL )
					{
						pSelCv->SetSelectedAsAPackage( true );

						// Set 6-way valve actuator.
						pSelCv->SetActrIDPtr( pCVActSet->GetSecondIDPtr() );

						// Set PIBCV, PIBCV accessories and PIBCV actuator.
						CDB_Set *pclPIBCVSet = dynamic_cast<CDB_Set *>( m_clDirectSelectionParams.m_pTADB->Get( pCVActSet->GetNextID() ).MP );
						
						// HYS-1968: static cast like above.
						if( NULL != pclPIBCVSet && NULL != (CDS_SSel6WayValve*)pSelCv )
						{
							CDS_SSel6WayValve *pSel6WayValve = (CDS_SSel6WayValve *)pSelCv;
							pSel6WayValve->SetSelectionMode( e6Way_EQMControl );
							CDB_PIControlValve *pclPIBCVFor6WayValveSet = dynamic_cast<CDB_PIControlValve *>( pclPIBCVSet->GetFirstIDPtr().MP );

							if( NULL != pclPIBCVFor6WayValveSet )
							{
								// Set PIBCV valve.
								CDS_SSelPICv *pSelPICv = pSel6WayValve->GetCDSSSelPICv( SideDefinition::BothSide );
								pSelPICv->SetProductIDPtr( pclPIBCVFor6WayValveSet->GetIDPtr() );
								pSelPICv->SetSelectedAsAPackage( true );
								
								// Accessories.
								pclCvAccessoryListToFill = pSelPICv->GetCvAccessoryList();
								CDB_RuledTable *pclPIBCVSetAccessories = dynamic_cast<CDB_RuledTable *>( pclPIBCVSet->GetAccGroupIDPtr().MP );
								
								if( NULL != pclPIBCVSetAccessories )
								{
									for( IDPTR IDPtr = pclPIBCVSetAccessories->GetFirst(); IDPtr.MP != NULL; IDPtr = pclPIBCVSetAccessories->GetNext() )
									{
										if( NULL != dynamic_cast<CDB_Product *>( IDPtr.MP ) )
										{
											pclCvAccessoryListToFill->Add( IDPtr, CAccessoryList::AccessoryType::_AT_Accessory, pclPIBCVSetAccessories );
										}
									}
								}

								// Set PIBCV actuator.
								pSelPICv->SetActrIDPtr( pclPIBCVSet->GetSecondIDPtr() );
							}
						}
					}
				}
				
				// Some accessories for CV.
				// Remark: pay attention that in this list we have also accessories that are excluded by the accessory chosen by user (see.h).
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pclCvAccessoryListToFill->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}

				if( false == ConfirmSelection( pSelCv, TabID ) )
				{
					IDPTR idptr = pSelCv->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_DpController *>( pclTreeItemProduct->m_pclProduct ) )
			{
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelDpC) );
				
				CDS_SSelDpC *pSel = (CDS_SSelDpC *)( IDPtr.MP );
				CAccessoryList *pMvAccessoryList = pSel->GetAccessoryList();
				CAccessoryList *pDpCAccessoryList = pSel->GetDpCAccessoryList();
				pSel->SetFromDirSel( true );
				pSel->SetDpCIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
				
				// Add measuring valve if exist.
				if( NULL != m_ItemProductSelected.m_hTreeItemSub )
				{
					CTreeItemProduct *pclTreeItemMv = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItemSub ) );
					
					if( NULL == pclTreeItemMv )
					{
						HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemProduct' from the tree.") );
					}
					else if( NULL == pclTreeItemMv->m_pclProduct )
					{
						HYSELECT_THROW( _T("Internal error: Product in the 'CTreeItemProduct' object is not defined.") );
					}
					
					pSel->SetProductIDPtr( pclTreeItemMv->m_pclProduct->GetIDPtr() );

					if( true == m_bWorkingForDpCSet )
					{
						// Try to find a set with a Dp controller and the measuring valve.
						CTableSet *pDpCMvSetTab = dynamic_cast<CTableSet *>( m_clDirectSelectionParams.m_pTADB->Get( _T("DPCMVSET_TAB") ).MP );
						ASSERT( NULL != pDpCMvSetTab );

						CDB_Set *pDpCMvSet = pDpCMvSetTab->FindCompatibleSet( pclTreeItemProduct->m_pclProduct->GetIDPtr().ID, pclTreeItemMv->m_pclProduct->GetIDPtr().ID );

						if( NULL != pDpCMvSet )
						{
							pSel->SetSelectedAsAPackage( true );
						}
					}
				}
				
				// Some accessories.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						IDPTR TypeIDPtr;
						if( CDB_Product::eProdUse::eBoth == m_vecProductAccessorySelected[i].m_pclAccessory->GetProductUseFlag() )
						{
							TypeIDPtr = m_vecProductAccessorySelected[i].m_pclAccessory->GetAccessTypeIDPtr();
						}
						else
						{
							TypeIDPtr = m_vecProductAccessorySelected[i].m_pclAccessory->GetTypeIDPtr();
						}
						
						if( NULL == TypeIDPtr.MP )
						{
							HYSELECT_THROW( _T("Internal error: Can't retrieve accessory type from the accessory '%s' at the position %i."), m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr().ID, i );
						}

						CDB_StringID *pTypeStringID = dynamic_cast<CDB_StringID *>( (CData *)TypeIDPtr.MP );

						if( NULL == pTypeStringID )
						{
							HYSELECT_THROW( _T("Internal error: Accessory type '%s' for accessory '%s' at the position %i is defined but it's not a 'CDB_StringID' object."), TypeIDPtr.ID, m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr().ID, i );
						}

						if( 0 == _tcscmp( TypeIDPtr.ID, _T("ACCTYP_DPC") ) )
						{
							pDpCAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
						}
						else if( 0 == _tcscmp( TypeIDPtr.ID, _T("ACCTYP_REGV") ) )
						{
							pMvAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
						}
						else
						{
							HYSELECT_THROW( _T("Internal error: Accessory type '%s' for accessory '%s' at the position %i is not recongnized."), TypeIDPtr.ID, m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr().ID, i );
						}
					}
				}
				
				if( false == ConfirmSelection( pSel, _T("DPCONTR_TAB") ) )
				{
					IDPTR idptr = pSel->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_DpReliefValve *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Differential pressure relief valve.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelDpReliefValve) );

				CDS_SSelDpReliefValve *pclSelDpReliefValve = (CDS_SSelDpReliefValve *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pclSelDpReliefValve->GetAccessoryList();
				pclSelDpReliefValve->SetFromDirSel( true );
				pclSelDpReliefValve->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelDpReliefValve, _T("DPCONTR_TAB") ) )
				{
					IDPTR idptr = pclSelDpReliefValve->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( ( NULL != dynamic_cast<CDB_Separator *>( pclTreeItemProduct->m_pclProduct ) ) 
					|| ( NULL != dynamic_cast<CDB_AirVent *>( pclTreeItemProduct->m_pclProduct ) )  )
			{
				// Separators.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelAirVentSeparator) );

				CDS_SSelAirVentSeparator *pclSelSeparator = (CDS_SSelAirVentSeparator *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pclSelSeparator->GetAccessoryList();
				pclSelSeparator->SetFromDirSel( true );
				pclSelSeparator->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelSeparator, _T("AIRVENTSEP_TAB") ) )
				{
					IDPTR idptr = pclSelSeparator->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_SafetyValveBase *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Safety valves.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSafetyValve ) );
				
				CDS_SSelSafetyValve *pclSelSafetyValve = (CDS_SSelSafetyValve *)( IDPtr.MP );
				pclSelSafetyValve->SetFromDirSel( true );
				pclSelSafetyValve->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
				pclSelSafetyValve->SetQuantityNeeded( 1 );

				// Some accessories.
				CAccessoryList *pAccessoryList = pclSelSafetyValve->GetAccessoryList();

				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelSafetyValve, _T("SAFETYVALVE_TAB") ) )
				{
					IDPTR idptr = pclSelSafetyValve->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_BlowTank *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// For a blow tank, we save it in a 'CDS_SSelSafetyValve' object.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSafetyValve ) );

				CDS_SSelSafetyValve *pclSelSafetyValve = (CDS_SSelSafetyValve *)( IDPtr.MP );
				pclSelSafetyValve->SetFromDirSel( true );
				pclSelSafetyValve->SetQuantityNeeded( 1 );
				pclSelSafetyValve->SetBlowTankIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories.
				CAccessoryList *pAccessoryList = pclSelSafetyValve->GetBlowTankAccessoryList();

				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelSafetyValve, _T("SAFETYVALVE_TAB") ) )
				{
					IDPTR idptr = pclSelSafetyValve->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_Vessel *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Pressure maintenance product.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPMaint ) );
				
				CDS_SSelPMaint *pclSelPMaint = (CDS_SSelPMaint *)( IDPtr.MP );
				pclSelPMaint->SetFromDirSel( true );

				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclTreeItemProduct->m_pclProduct );
				
				if( NULL == pclVessel )
				{
					return;
				}

				switch( pclVessel->GetVesselType() )
				{
					case CDB_Vessel::eVsslType_Statico:
					case CDB_Vessel::eVsslType_StatMbr:
					case CDB_Vessel::eVsslType_Prim:
					case CDB_Vessel::eVsslType_Aqua:
						pclSelPMaint->SetVesselIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
						pclSelPMaint->SetVesselNumber( 1 );
						break;

					case CDB_Vessel::eVsslType_Sec:
						pclSelPMaint->SetSecondaryVesselIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
						pclSelPMaint->SetSecondaryVesselNumber( 1 );
						break;

					case CDB_Vessel::eVsslType_Interm:
						pclSelPMaint->SetIntermediateVesselIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
						pclSelPMaint->SetIntermediateVesselNumber( 1 );
						break;
				}

				// Some accessories.
				CAccessoryList clAccessoryList;

				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						clAccessoryList.Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}

				if( 0 != (int)clAccessoryList.GetCount() )
				{
					if( NULL != dynamic_cast<CDB_Vessel *>( pclTreeItemProduct->m_pclProduct ) )
					{
						switch( pclVessel->GetVesselType() )
						{
							case CDB_Vessel::eVsslType_Statico:
							case CDB_Vessel::eVsslType_StatMbr:
							case CDB_Vessel::eVsslType_Prim:
							case CDB_Vessel::eVsslType_Aqua:
								pclSelPMaint->SetVesselAccessoryList( &clAccessoryList );
								break;

							case CDB_Vessel::eVsslType_Sec:
								// No accessory for secondary vessel.
								break;

							case CDB_Vessel::eVsslType_Interm:
								pclSelPMaint->SetIntermediateVesselAccessoryList( &clAccessoryList );
								break;
						}

						pclSelPMaint->SetVesselAccessoryList( &clAccessoryList );
					}
				}
				
				if( false == ConfirmSelection( pclSelPMaint, _T("PRESSMAINT_TAB") ) )
				{
					IDPTR idptr = pclSelPMaint->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_TecBox *>( pclTreeItemProduct->m_pclProduct ) )
			{
				CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pclTreeItemProduct->m_pclProduct );
				
				// Tecbox.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPMaint ) );
			
				CDS_SSelPMaint *pclSSelTecBox = (CDS_SSelPMaint *)( IDPtr.MP );
				pclSSelTecBox->SetFromDirSel( true );
				CAccessoryList *pclAccessoryList = NULL;
				
				// HYS-945: Buffer vessel management.
				CAccessoryList *pclIntegratedAccessoryList = NULL;

				switch( pclTecBox->GetTecBoxType() )
				{
					case CDB_TecBox::etbtCompresso:
					case CDB_TecBox::etbtTransfero:
					case CDB_TecBox::etbtTransferoTI:
						pclSSelTecBox->SetTecBoxCompTransfIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
						pclSSelTecBox->SetTecBoxCompTransfNumber( 1 );
						pclAccessoryList = pclSSelTecBox->GetTecBoxCompTransfAccessoryList();
						// HYS-945: Get the buffer vessel accessories
						pclIntegratedAccessoryList = pclSSelTecBox->GetTecBoxIntegratedVesselAccessoryList();

						break;

					case CDB_TecBox::etbtPleno:
						pclSSelTecBox->SetTecBoxPlenoIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
						pclSSelTecBox->SetTecBoxPlenoNumber( 1 );
						pclAccessoryList = pclSSelTecBox->GetTecBoxPlenoAccessoryList();
						break;

					case CDB_TecBox::etbtVento:
						pclSSelTecBox->SetTecBoxVentoIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
						pclSSelTecBox->SetTecBoxVentoNumber( 1 );
						pclAccessoryList = pclSSelTecBox->GetTecBoxVentoAccessoryList();
						break;
				}

				// Add integrated buffer if exist.
				if( NULL != pclTecBox->GetIntegratedVesselIDPtr().MP )
				{
					// For Compresso the 'GetIntegratedVesselIDPtr' method returns a 'CDB_Vessel*'.
					// For Transfero the 'GetIntegratedVesselIDPtr' method returns a 'CTable*'.
					if( NULL != dynamic_cast<CTable *>( pclTecBox->GetIntegratedVesselIDPtr().MP ) && NULL != m_ItemIntegratedSelected.m_hTreeItem )
					{
						CTreeItemIntegrated *pclTreeItemIntegrated = dynamic_cast<CTreeItemIntegrated *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemIntegratedSelected.m_hTreeItem ) );
						
						// HYS-945.
						if( NULL == pclTreeItemIntegrated )
						{
							HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemIntegrated' from the tree.") );
						}
						else if( NULL == pclTreeItemIntegrated->m_pclIntegrated )
						{
							HYSELECT_THROW( _T("Internal error: Integrated object in the 'CTreeItemIntegrated' object is not defined.") );
						}
						else if( false == _IsSelectable( pclTreeItemIntegrated->m_pclIntegrated ) )
						{
							HYSELECT_THROW( _T("Internal error: Object '%s' is not selectable."), pclTreeItemIntegrated->m_pclIntegrated->GetIDPtr().ID );
						}
						
						pclSSelTecBox->SetTecBoxIntegratedVesselIDPtr( pclTreeItemIntegrated->m_pclIntegrated->GetIDPtr() );
					}
					else if( NULL != dynamic_cast<CDB_Vessel *>( pclTecBox->GetIntegratedVesselIDPtr().MP ) )
					{
						pclSSelTecBox->SetTecBoxIntegratedVesselIDPtr( pclTecBox->GetIntegratedVesselIDPtr() );
					}
					// HYS-945
					pclSSelTecBox->SetVesselNumber( 1 );

					// Some accessories for buffer vessel
					// Remark: pay attention that in this list we have also accessories that are excluded by the accessory chosen by user (see.h).
					for( int i = 0; i < (int)m_vecIntegratedAccessorySelected.size(); i++ )
					{
						if( true == m_vecIntegratedAccessorySelected[i].m_bCheck )
						{
							pclIntegratedAccessoryList->Add( m_vecIntegratedAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecIntegratedAccessorySelected[i].m_eAccessoryType, m_vecIntegratedAccessorySelected[i].m_pclRuledTable );
						}
					}
				}

				// Some accessories.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pclAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSSelTecBox, _T("PRESSMAINT_TAB") ) )
				{
					IDPTR idptr = pclSSelTecBox->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_PlenoRefill *>( pclTreeItemProduct->m_pclProduct ) )
			{
				CDB_PlenoRefill *pclPlenoRefill = dynamic_cast<CDB_PlenoRefill *>( pclTreeItemProduct->m_pclProduct );

				// Create the object where to saved.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPMaint ) );
				
				CDS_SSelPMaint *pclSSelPlenoRefill = (CDS_SSelPMaint *)( IDPtr.MP );
				pclSSelPlenoRefill->SetFromDirSel( true );
				pclSSelPlenoRefill->SetPlenoRefillIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );
				pclSSelPlenoRefill->SetPlenoRefillNumber( 1 );
				CAccessoryList *pclAccessoryList = pclSSelPlenoRefill->GetPlenoRefillAccessoryList();

				// Some accessories.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pclAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}

				if( false == ConfirmSelection( pclSSelPlenoRefill, _T("PRESSMAINT_TAB") ) )
				{
					IDPTR idptr = pclSSelPlenoRefill->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if(  NULL != dynamic_cast<CDB_ShutoffValve *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Shut-off valves.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSv ) );
				
				CDS_SSelSv *pclSelShutoffValve = (CDS_SSelSv *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pclSelShutoffValve->GetAccessoryList();
				pclSelShutoffValve->SetFromDirSel( true );
				pclSelShutoffValve->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Add actuator if exist.
				if( NULL != m_ItemActuatorSelected.m_hTreeItem )
				{
					CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem ) );

					if( NULL == pclTreeItemActuator )
					{
						HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemActuator' from the tree.") );
					}
					else if( NULL == pclTreeItemActuator->m_pclActuator )
					{
						HYSELECT_THROW( _T("Internal error: Actuator in the 'CTreeItemActuator' object is not defined.") );
					}
					
					pclSelShutoffValve->SetActrIDPtr( pclTreeItemActuator->m_pclActuator->GetIDPtr() );
					CAccessoryList *pclShutOffValveAccessoryListToFill = pclSelShutoffValve->GetSvAccessoryList();
					CAccessoryList *pclActuatorAccessoryListToFill = pclSelShutoffValve->GetActuatorAccessoryList();

					// Some accessories ?
					// Remark: pay attention that in this list we have also accessories that are excluded by the accessory chosen by user (see.h).
					for( int i = 0; i < (int)m_vecActuatorAccessorySelected.size(); i++ )
					{
						if( true == m_vecActuatorAccessorySelected[i].m_bCheck )
						{
							// Little exception compared to individual selection. For example, selection for butterfly valves are automatically consider as a selection by package. 
							// User chooses first the valve, accessories on valve and actuator are displayed below. It's only when user finishes selection that we display 
							// accessories on actuator and accessories that belongs to the package. In this case, accessories belonging to the set are added to the list 
							// linked to the valve and not to the actuator. In direct selection, we display accessory belonging to the package just under the accessories of 
							// the actuator. And these accessories are saved in 'm_vecActuatorAccessorySelected'. 
							if( m_vecActuatorAccessorySelected[i].m_eAccessoryType != CAccessoryList::AccessoryType::_AT_SetAccessory )
							{
								pclActuatorAccessoryListToFill->Add( m_vecActuatorAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecActuatorAccessorySelected[i].m_eAccessoryType, m_vecActuatorAccessorySelected[i].m_pclRuledTable );
							}
							else
							{
								pclShutOffValveAccessoryListToFill->Add( m_vecActuatorAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecActuatorAccessorySelected[i].m_eAccessoryType, m_vecActuatorAccessorySelected[i].m_pclRuledTable );
							}
						}
					}
				}

				// Some accessories for shut-off valves.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelShutoffValve, _T("SHUTOFF_TAB") ) )
				{
					IDPTR idptr = pclSelShutoffValve->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if(  NULL != dynamic_cast<CDB_FloorHeatingManifold *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Floor heating manifold products.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelFloorHeatingManifold ) );
				
				CDS_SSelFloorHeatingManifold *pclSelFloorHeatingManifold = (CDS_SSelFloorHeatingManifold *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pclSelFloorHeatingManifold->GetAccessoryList();
				pclSelFloorHeatingManifold->SetFromDirSel( true );
				pclSelFloorHeatingManifold->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories for floor heating manifold products.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelFloorHeatingManifold, _T("FLOORHCTRL_TAB") ) )
				{
					IDPTR idptr = pclSelFloorHeatingManifold->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if(  NULL != dynamic_cast<CDB_FloorHeatingValve *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Floor heating valve products.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelFloorHeatingValve ) );
				
				CDS_SSelFloorHeatingValve *pclSelFloorHeatingValve = (CDS_SSelFloorHeatingValve *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pclSelFloorHeatingValve->GetAccessoryList();
				pclSelFloorHeatingValve->SetFromDirSel( true );
				pclSelFloorHeatingValve->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories for floor heating valve products.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelFloorHeatingValve, _T("FLOORHCTRL_TAB") ) )
				{
					IDPTR idptr = pclSelFloorHeatingValve->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if(  NULL != dynamic_cast<CDB_FloorHeatingController *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Floor heating controller products.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelFloorHeatingController ) );
				
				CDS_SSelFloorHeatingController *pclSelFloorHeatingController = (CDS_SSelFloorHeatingController *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pclSelFloorHeatingController->GetAccessoryList();
				pclSelFloorHeatingController->SetFromDirSel( true );
				pclSelFloorHeatingController->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories for floor heating controller products.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelFloorHeatingController, _T("FLOORHCTRL_TAB") ) )
				{
					IDPTR idptr = pclSelFloorHeatingController->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if(  NULL != dynamic_cast<CDB_TapWaterControl *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Tap water control products.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelTapWaterControl ) );
				
				CDS_SSelTapWaterControl *pclSelTapWaterControl = (CDS_SSelTapWaterControl *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pclSelTapWaterControl->GetAccessoryList();
				pclSelTapWaterControl->SetFromDirSel( true );
				pclSelTapWaterControl->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories for tap water control products.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pclSelTapWaterControl, _T("TAPWATERCTRL_TAB") ) )
				{
					IDPTR idptr = pclSelTapWaterControl->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if(  NULL != dynamic_cast<CDB_SmartControlValve *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// Smart control valve products.
				IDPTR IDPtr;

				if( 0 == StringCompare( _T("SMARTCTRLVALVETYPE"), pclTreeItemProduct->m_pclProduct->GetTypeID() ) )
				{
					m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSmartControlValve ) );

					CDS_SSelSmartControlValve *pclSelSmartControlValve = (CDS_SSelSmartControlValve *)( IDPtr.MP );
					CAccessoryList *pAccessoryList = pclSelSmartControlValve->GetAccessoryList();
					pclSelSmartControlValve->SetFromDirSel( true );
					pclSelSmartControlValve->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

					// HYS-2047: Add accessories for TA Smart.
					if ( NULL != pAccessoryList )
					{
						for ( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++)
						{
							if ( true == m_vecProductAccessorySelected[i].m_bCheck )
							{
								pAccessoryList->Add(m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType,
									m_vecProductAccessorySelected[i].m_pclRuledTable);
							}
						}
					}

					if( false == ConfirmSelection( pclSelSmartControlValve, _T("SMARTCONTROLVALVE_TAB") ) )
					{
						IDPTR idptr = pclSelSmartControlValve->GetIDPtr();
						m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
					}
				}
				else if( 0 == StringCompare( _T("SMARTDPCTYPE"), pclTreeItemProduct->m_pclProduct->GetTypeID() ) )
				{
					m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSmartDpC ) );

					CDS_SSelSmartDpC *pclSelSmartDpC = (CDS_SSelSmartDpC *)( IDPtr.MP );

					pclSelSmartDpC->SetFromDirSel( true );
					pclSelSmartDpC->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

					// Save smart differential pressure controller accessories if exist.
					CAccessoryList *pclSmartDpCAccessoryList = pclSelSmartDpC->GetAccessoryList();
					ASSERT( NULL != pclSmartDpCAccessoryList );

					if( NULL != pclSmartDpCAccessoryList )
					{
						for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
						{
							// HYS-1992: Fictif set is managed with hTreeItemSub.
							if( true == m_vecProductAccessorySelected[i].m_bCheck 
								&& false == IsAccessoryFictifSet( pclTreeItemProduct->m_pclProduct, m_vecProductAccessorySelected[i].m_pclAccessory ) )
							{
								pclSmartDpCAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, 
										m_vecProductAccessorySelected[i].m_pclRuledTable );
							}
						}
					}

					// Add Dp sensor or set if it exists...
					if( NULL != m_ItemProductSelected.m_hTreeItemSub )
					{
						CTreeItemAccessory *pclTreeItemDpSensor = dynamic_cast<CTreeItemAccessory*>( (CTreeItemAccessory *)m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItemSub ) );

						if( NULL == pclTreeItemDpSensor )
						{
							HYSELECT_THROW( _T( "Internal error: Can't retrieve the 'CTreeItemProduct' from the tree." ) );
						}
						else if( NULL == pclTreeItemDpSensor->m_pclAccessory )
						{
							HYSELECT_THROW( _T( "Internal error: Product in the 'CTreeItemProduct' object is not defined." ) );
						}

						// HYS-1992: Set fictif set accessories.
						if( true == pclTreeItemDpSensor->m_pclAccessory->IsPartOfaSet() )
						{
							pclSelSmartDpC->SetSetIDPtr( pclTreeItemDpSensor->m_pclAccessory->GetIDPtr() );
							// Save set content accessories.
							CAccessoryList* pclSetContentAccessoryList = pclSelSmartDpC->GetSetContentAccessoryList();
							ASSERT( NULL != pclSetContentAccessoryList );

							if( NULL != pclSetContentAccessoryList )
							{
								// Accessories are included into the fictif set
								// HYS-2007
								CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>(pclTreeItemDpSensor->m_pclAccessory->GetAccessoriesGroupIDPtr().MP);
								if( NULL != pclRuledTable )
								{
									// Sort the list with a string number parameter.
									CRank SortList;
									SortList.PurgeAll();

									TASApp.GetpTADB()->GetAccessories( &SortList, pclRuledTable, CTADatabase::ForDirSel );

									// Fill the tree with the sorted ID's.
									CString str;
									LPARAM lparam;

									for( BOOL bContinue = SortList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lparam ) )
									{
										CDB_Product* pclAccessory = (CDB_Product*)lparam;
										pclSetContentAccessoryList->Add( pclAccessory->GetIDPtr(), CAccessoryList::AccessoryType::_AT_SetAccessory,
																	   pclRuledTable );
									}
								}
							}
						}
						else
						{
							// Save the Dp sensor.
							pclSelSmartDpC->SetDpSensorIDPtr( pclTreeItemDpSensor->m_pclAccessory->GetIDPtr() );
							// Save Dp sensor accessories if exist.
							CAccessoryList* pclDpSensorAccessoryList = pclSelSmartDpC->GetDpSensorAccessoryList();
							ASSERT( NULL != pclDpSensorAccessoryList );

							if( NULL != pclDpSensorAccessoryList )
							{
								for( int i = 0; i < (int)m_vecSubProductAccessorySelected.size(); i++ )
								{
									if( true == m_vecSubProductAccessorySelected[i].m_bCheck )
									{
										pclDpSensorAccessoryList->Add( m_vecSubProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecSubProductAccessorySelected[i].m_eAccessoryType,
																		m_vecSubProductAccessorySelected[i].m_pclRuledTable );
									}
								}
							}
						}
					}

					if( false == ConfirmSelection( pclSelSmartDpC, _T( "SMARTDPC_TAB" ) ) )
					{
						IDPTR idptr = pclSelSmartDpC->GetIDPtr();
						m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
					}
				}
			}
			// HYS-1741: IPI March 2021 : Here we manage all accessories and services for Pressure maintenance and water quality
			else if( ( NULL != pclTreeItemProduct->m_pclProduct->GetFamilyIDPtr().PP)
			         && ( 0 == CString( pclTreeItemProduct->m_pclProduct->GetFamilyIDPtr().PP->GetIDPtr().ID ).Compare( _T( "ACCSERVICEFAM_TAB" ) ) ) )
			{
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SelPWQAccServices ) );
				
				CDS_SelPWQAccServices* pSel = (CDS_SelPWQAccServices*)(IDPtr.MP);
				
				pSel->SetFromDirSel( true );
				pSel->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				if( false == ConfirmSelection( pSel, _T( "SAFETYVALVE_TAB" ) ) )
				{
					IDPTR idptr = pSel->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != dynamic_cast<CDB_DpSensor*>(pclTreeItemProduct->m_pclProduct) )
			{
			    // HYS-2007
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelDpSensor ) );
				
				CDS_SSelDpSensor* pSel = (CDS_SSelDpSensor*)(IDPtr.MP);
				CAccessoryList* pAccessoryList = pSel->GetAccessoryList();

				pSel->SetFromDirSel( true );
				pSel->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, 
											 m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}

				if( false == ConfirmSelection( pSel, _T( "DPCONTR_TAB" ) ) )
				{
					IDPTR idptr = pSel->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else
			{
				// Regulating valves.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelBv) );
				
				CDS_SSelBv *pSel = (CDS_SSelBv *)( IDPtr.MP );
				CAccessoryList *pAccessoryList = pSel->GetAccessoryList();
				pSel->SetFromDirSel( true );
				pSel->SetProductIDPtr( pclTreeItemProduct->m_pclProduct->GetIDPtr() );

				// Some accessories.
				for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
				{
					if( true == m_vecProductAccessorySelected[i].m_bCheck )
					{
						pAccessoryList->Add( m_vecProductAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecProductAccessorySelected[i].m_eAccessoryType, m_vecProductAccessorySelected[i].m_pclRuledTable );
					}
				}
				
				if( false == ConfirmSelection( pSel, _T("REGVALV_TAB") ) )
				{
					IDPTR idptr = pSel->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}

		}
		
		if( NULL == m_ItemProductSelected.m_hTreeItem )
		{
			if( NULL != m_ItemActuatorSelected.m_hTreeItem && NULL == m_ItemAdapterSelected.m_hTreeItem )
			{
				// Case of actuator selected alone.

				CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem ) );

				if( NULL == pclTreeItemActuator )
				{
					HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemActuator' from the tree.") );
				}
				else if( NULL == pclTreeItemActuator->m_pclActuator )
				{
					HYSELECT_THROW( _T("Internal error: Actuator in the 'CTreeItemActuator' object is not defined.") );
				}

				// Create the 'CDS_Actuator' object.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_Actuator ) );

				CDS_Actuator *pSelAct = (CDS_Actuator *)( IDPtr.MP );
				pSelAct->SetActuatorIDPtr( pclTreeItemActuator->m_pclActuator->GetIDPtr() );

				// Add accessories.
				for( int i = 0; i < (int)m_vecActuatorAccessorySelected.size(); i++ )
				{
					if( true == m_vecActuatorAccessorySelected[i].m_bCheck )
					{
						pSelAct->AddActuatorAccessory( m_vecActuatorAccessorySelected[i].m_pclAccessory->GetIDPtr(), m_vecActuatorAccessorySelected[i].m_pclRuledTable );
					}
				}
			
				if( false == ConfirmSelection( pSelAct, _T("CTRLVALVE_TAB") ) )
				{
					IDPTR idptr = pSelAct->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL == m_ItemActuatorSelected.m_hTreeItem && NULL != m_ItemAdapterSelected.m_hTreeItem )
			{
				// Case of adapter selected alone.

				CTreeItemAdapter *pclTreeItemAdapter = dynamic_cast<CTreeItemAdapter *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemAdapterSelected.m_hTreeItem ) );

				if( NULL == pclTreeItemAdapter )
				{
					HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemAdapter' from the tree.") );
				}
				else if( NULL == pclTreeItemAdapter->m_pclAdapter )
				{
					HYSELECT_THROW( _T("Internal error: Adapter in the 'CTreeItemActuator' object is not defined.") );
				}

				// Create the 'CDS_Accessory' object.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_Accessory ) );
				
				CDS_Accessory *pSelectedAdapter = (CDS_Accessory *)( IDPtr.MP );
				pSelectedAdapter->SetAccessoryIDPtr( pclTreeItemAdapter->m_pclAdapter->GetIDPtr() );

				if( false == ConfirmSelection( pSelectedAdapter, _T("CTRLVALVE_TAB") ) )
				{
					IDPTR idptr = pSelectedAdapter->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
			else if( NULL != m_ItemActuatorSelected.m_hTreeItem && NULL != m_ItemAdapterSelected.m_hTreeItem )
			{
				// Case of actuator and adapter selected together.
				
				CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem ) );
				
				if( NULL == pclTreeItemActuator )
				{
					HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemActuator' from the tree.") );
				}
				else if( NULL == pclTreeItemActuator->m_pclActuator )
				{
					HYSELECT_THROW( _T("Internal error: Actuator in the 'CTreeItemActuator' object is not defined.") );
				}

				CTreeItemAdapter *pclTreeItemAdapter = dynamic_cast<CTreeItemAdapter *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemAdapterSelected.m_hTreeItem ) );

				if( NULL == pclTreeItemAdapter )
				{
					HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CTreeItemAdapter' from the tree.") );
				}
				else if( NULL == pclTreeItemAdapter->m_pclAdapter )
				{
					HYSELECT_THROW( _T("Internal error: Adapter in the 'CTreeItemAdapter' object is not defined.") );
				}

				// Create the 'CDS_SSelCv'.
				// Remark: In the 'confirm selection' dialog, the only way to display both actuator and adapter is to save them in a 'CDS_SSelCv'.
				IDPTR IDPtr;
				m_clDirectSelectionParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelCv ) );

				CDS_SSelCv *pSelectedControl = (CDS_SSelCv *)( IDPtr.MP );
				pSelectedControl->SetProductIDPtr( _NULL_IDPTR );
				pSelectedControl->SetActrIDPtr( pclTreeItemActuator->m_pclActuator->GetIDPtr() );
				
				CAccessoryList *pclAccessoryList = pSelectedControl->GetAccessoryList();

				if( NULL == pclAccessoryList )
				{
					HYSELECT_THROW( _T("Internal error: Accessory list from the control valve '%s' can't be empty."), pSelectedControl->GetIDPtr().ID );
				}

				pclAccessoryList->Add( pclTreeItemAdapter->m_pclAdapter->GetIDPtr(), CAccessoryList::_AT_Adapter );

				if( false == ConfirmSelection( pSelectedControl, _T("CTRLVALVE_TAB") ) )
				{
					IDPTR idptr = pSelectedControl->GetIDPtr();
					m_clDirectSelectionParams.m_pTADS->DeleteObject( idptr );
				}
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgDirSel::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgDirSel::OnBnClickedCancel()
{
	OnCancel();
}

void CDlgDirSel::OnBnClickedDeletedValves()
{
	CDS_PersistData* pPD = TASApp.GetpTADS()->GetpPersistData();
	ASSERT( NULL != pPD );

	if( pPD != NULL )
	{
		// Save that deleted valves can be selected.
		if( BST_CHECKED == m_CBDeletedValves.GetCheck() )
		{
			pPD->SetOldValveUsed( true );
		}
		else
		{
			pPD->SetOldValveUsed( false );
		}
	}

	m_TreeProdCateg.DeleteAllItems();
	// Fill the tree to add or remove deleted products.
	FillTreeProductCategories();
	// Save checkbox value
	AfxGetApp()->WriteProfileInt( _T( "DialogDirSel" ), _T( "ShowOldValves" ), m_CBDeletedValves.GetCheck() );
}

void CDlgDirSel::OnBnClickedDirSelExport()
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CDlgDirSelExport DlgAllOrAvailable;

	if( IDOK != DlgAllOrAvailable.DoModal() )
	{
		return;
	}

	bool bResultAll = DlgAllOrAvailable.IsExportAll();

	CFileDialog dlg( false, _T("txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST );
	
	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();
		
		//Filename Exist
		try
		{
			CString Filename = dlg.GetPathName();
			CPath path( Filename );
			CString File = path.SplitPath( (CPath::ePathFields)(CPath::ePathFields::epfDrive + CPath::ePathFields::epfDir + CPath::ePathFields::epfFName ) ) + _T(".txt");
			std::ofstream outf( (TCHAR*)(LPCTSTR)File, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );

			BYTE bHeader[2] = { 0xFF, 0xFE };
			outf.write( (char *)(LPCTSTR)bHeader, 2 );
			
			HTREEITEM hItem = m_TreeProdCateg.GetSelectedItem();
			
			if( NULL == hItem || NULL == ( (CTreeItemData *)m_TreeProdCateg.GetItemData( hItem ) )->GetStringCategoryID() )
			{
				outf.close();
				BeginWaitCursor();
				return;
			}

			CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProdCateg.GetItemData( hItem );
			IDPTR TabSubCatIDPtr = m_clDirectSelectionParams.m_pTADB->Get( pclTreeItemData->GetStringCategoryID()->GetIDstr2() );
			ASSERT( NULL != TabSubCatIDPtr.MP );

			if( NULL == TabSubCatIDPtr.MP )
			{
				outf.close();
				BeginWaitCursor();
				return;
			}
			
			CTable *pclSubCategoryTable = dynamic_cast<CTable *>( TabSubCatIDPtr.MP );
			ASSERT( NULL != pclSubCategoryTable );

			if( NULL == pclSubCategoryTable )
			{
				outf.close();
				BeginWaitCursor();
				return;
			}
			
			// Run all sub categories table in the selected category table.
			for( IDPTR SubCategoryIDPtr = pclSubCategoryTable->GetFirst(); NULL != SubCategoryIDPtr.MP; SubCategoryIDPtr = pclSubCategoryTable->GetNext() )
			{
				CDB_MultiString *pstrSubCategory = dynamic_cast<CDB_MultiString *>( SubCategoryIDPtr.MP );

				if( NULL == pstrSubCategory )
				{
					continue;
				}
				
				// If selection is on the sub category and it's not the same, we directly go the the next sub category.
				if( NULL != pclTreeItemData->GetMultiStringSubCategory() && pstrSubCategory != pclTreeItemData->GetMultiStringSubCategory() )
				{
					continue;
				}

				// Loop on all types.
				// Remark: for subcategory, we now use a 'CDB_MultiString' object to have the possibility to add more than one type
				//         under a sub category.
				for( int iLoopType = 2; iLoopType < pstrSubCategory->GetNumofLines(); iLoopType++ )
				{
					IDPTR TabTypeIDPtr = m_clDirectSelectionParams.m_pTADB->Get( pstrSubCategory->GetString( iLoopType ) );
					ASSERT( NULL != TabTypeIDPtr.MP );

					if( NULL == TabTypeIDPtr.MP )
					{
						continue;
					}
				
					CTable *pclTypeTable = dynamic_cast<CTable *>( TabTypeIDPtr.MP );
					ASSERT( NULL != pclTypeTable );

					if( NULL == pclTypeTable )
					{
						continue;
					}

					// Do not loop if type concerned does not include 'CDB_Product' or 'CDB_Actuator' objects.
					IDPTR TypeIDptr = pclTypeTable->GetFirst();
					CDB_StringID *pstrTypeID = dynamic_cast<CDB_StringID *>( TypeIDptr.MP );

					if( NULL == pstrTypeID )
					{
						continue;
					}
				
					// Retrieve the product table linked to this type.
					CTable *pclProductTab = dynamic_cast<CTable *>( (CData *)( m_clDirectSelectionParams.m_pTADB->Get( pstrTypeID->GetIDstr2() ).MP ) );

					if( NULL == pclProductTab )
					{
						continue;
					}
				
					// Retrieve the first product.
					IDPTR FirstIDPtr = pclProductTab->GetFirst();
				
					if( NULL == FirstIDPtr.MP )
					{
						continue;
					}
				
					if( NULL == dynamic_cast<CDB_Product *>( FirstIDPtr.MP ) && NULL == dynamic_cast<CDB_Actuator *>( FirstIDPtr.MP ) )
					{
						continue;
					}

					// Check also if the product belongs to a set or not.
					bool bForSetOnly = false;

					if( 0 == IDcmp( pstrSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_DPCONTRSET") ) )
					{
						bForSetOnly = true;
					}
					else if( 0 == IDcmp( pstrSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_STDCVACTSET") ) )
					{
						bForSetOnly = true;
					}
					else if( 0 == IDcmp( pstrSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ADVCTRLVSET") ) )
					{
						bForSetOnly = true;
					}

					// Run all types in the current sub category table.
					for( ; NULL != TypeIDptr.MP; TypeIDptr = pclTypeTable->GetNext() )
					{
						CDB_StringID *pstrTypeID = dynamic_cast<CDB_StringID *>( TypeIDptr.MP );

						if( NULL == pstrTypeID )
						{
							continue;
						}

						// If selection is on the type and it's not the same, we directly go the the next type.
						if( NULL != pclTreeItemData->GetStringTypeID() && pstrTypeID != pclTreeItemData->GetStringTypeID() )
						{
							continue;
						}

						IDPTR TabProductIDPtr = m_clDirectSelectionParams.m_pTADB->Get( pstrTypeID->GetIDstr2() );
						ASSERT( NULL != TabProductIDPtr.MP );

						if( NULL == TabProductIDPtr.MP )
						{
							continue;
						}

						CTable *pclProductTable = dynamic_cast<CTable *>( TabProductIDPtr.MP );
						ASSERT( NULL != pclProductTable );

						if( NULL == pclProductTable )
						{
							continue;
						}

						// Don't loop if products concerned are not a 'CDB_Product' or 'CDB_Actuator'.
						IDPTR ProductIDPtr = pclProductTable->GetFirst();

						if( NULL == dynamic_cast<CDB_Product *>( ProductIDPtr.MP ) && NULL == dynamic_cast<CDB_Actuator *>( ProductIDPtr.MP ) )
						{
							continue;
						}

						// Run all products in the product table.
						for( ; NULL != ProductIDPtr.MP; ProductIDPtr = pclProductTable->GetNext() )
						{
							CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( ProductIDPtr.MP );
							CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( ProductIDPtr.MP );

							if( NULL == pclProduct && NULL == pclActuator )
							{
								continue;
							}

							if( false == bResultAll )
							{
								if( NULL != pclProduct && false == _IsSelectable( pclProduct ) )
								{
									continue;
								}

								if( NULL != pclActuator && false == _IsSelectable( pclActuator ) )
								{
									continue;
								}
							}

							// If selection is on the family and it's not the same, we directly go to the next product.
							if( NULL != pclProduct && NULL != pclTreeItemData->GetStringFamilyID() 
									&& (CDB_StringID *)( pclProduct->GetFamilyIDPtr().MP ) != pclTreeItemData->GetStringFamilyID() )
							{
								continue;
							}

							if( NULL != pclActuator && NULL != pclTreeItemData->GetStringFamilyID() 
									&& (CDB_StringID *)( pclActuator->GetFamilyIDPtr().MP ) != pclTreeItemData->GetStringFamilyID() )
							{
								continue;
							}

							// If selection doesn't math the 'bForSetOnly' we continue.
							if( true == bForSetOnly && NULL != pclProduct && false == pclProduct->IsPartOfaSet() )
							{
								continue;
							}

							CString strTab;
							ProductIDPtr.MP->WriteTextAccess( outf, strTab );
						}
					}
				}
			}
			
			outf.close();

		}catch( ... ) {}

		EndWaitCursor();
	}
}

void CDlgDirSel::OnBnClickedDirSelTender()
{
	CTenderDlg Dlg;

	// Process product.
	if( NULL != m_ItemProductSelected.m_hTreeItem )
	{
		CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( ( CTreeItemData * )m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItem ) );

		if( NULL == pclTreeItemProduct || NULL == pclTreeItemProduct->m_pclProduct )
		{
			ASSERT_RETURN;
		}

		const TCHAR *artNum = pclTreeItemProduct->m_pclProduct->GetBodyArtNum();

		if( -1 != TASApp.GetTender().GetTenderID( artNum ) )
		{
				_string tenderText;

				TASApp.GetTender().GetTenderTxt( _string( artNum ), tenderText );
				Dlg.SetTender(pclTreeItemProduct->m_pclProduct->GetName(), _string(artNum), tenderText);

				INT_PTR result = Dlg.DoModal();
		}
	}
}

void CDlgDirSel::FillTreeProductCategories()
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	// CATEGORY                             -> Ex: "Balancing and Control"
	//   |_____SUB_CATEGORY                 -> Ex: "Regulating Valve"
	//	         |_________TYPE             -> Ex: "Balancing Valve"
	//	 		            |__FAMILY       -> Ex: "STAD"

	// Product and solution tab
	IDPTR idptr = m_clDirectSelectionParams.m_pTADB->Get( _T("CATEGORY_TAB") );
	ASSERT( _T('\0') != *idptr.ID );

	CTable *pTab = dynamic_cast<CTable *>( idptr.MP );
	ASSERT( NULL != pTab );

	std::map<int, CDB_StringID *> mapCategoryList;
	
	for( IDPTR idptrPS = pTab->GetFirst(); NULL != idptrPS.MP; idptrPS = pTab->GetNext( idptrPS.MP ) )
	{
		CDB_StringID *pStrCategoryID = dynamic_cast<CDB_StringID *>( idptrPS.MP );
		ASSERT( NULL != pStrCategoryID );

		if( NULL == pStrCategoryID )
		{
			continue;
		}

		mapCategoryList[_ttoi( pStrCategoryID->GetIDstr() )] = pStrCategoryID;
	}

	for( auto &iter : mapCategoryList )
	{
		HTREEITEM hItemCategory;
		CDB_StringID *pStrCategoryID = iter.second;
		ASSERT( NULL != pStrCategoryID );

		if( NULL == pStrCategoryID )
		{
			continue;
		}

		CTreeItemCategory *pclTreeItemCategory = new CTreeItemCategory( pStrCategoryID );
		ASSERT( NULL != pclTreeItemCategory );

		if(	NULL == pclTreeItemCategory )
		{
			continue;
		}

		m_vecItemDataInTreeList.push_back( pclTreeItemCategory );
		hItemCategory = m_TreeProdCateg.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pStrCategoryID->GetString(), 0, 0, 0, 0, (LPARAM)pclTreeItemCategory, TVI_ROOT, TVI_LAST );
		pclTreeItemCategory->m_hTreeItem = hItemCategory;

		IDPTR idptrCateg = m_clDirectSelectionParams.m_pTADB->Get( pStrCategoryID->GetIDstr2() );
		ASSERT( _T('\0') != *idptrCateg.ID );
		
		CTable *pTabCateg = dynamic_cast<CTable *>( idptrCateg.MP );
		ASSERT( NULL != pTabCateg );

		// Sort sub category list.
		CRank SortList;

		for( idptr = pTabCateg->GetFirst(); _T('\0') != *idptr.ID; idptr = pTabCateg->GetNext() )
		{
			CDB_MultiString *pclMultiStringSubCategory = dynamic_cast<CDB_MultiString *>( idptr.MP );
			ASSERT( NULL != pclMultiStringSubCategory );

			// Loop on all types to check if at least one can be display and exist.
			// Remark: for subcategory, we now use a 'CDB_MultiString' object to have the possibility to add more than one type
			//         under a sub category.
			bool bAtLeastOneSelectable = false;

			for( int iLoopType = 2; iLoopType < pclMultiStringSubCategory->GetNumofLines() && false == bAtLeastOneSelectable; iLoopType++ )
			{
				CString strTypeID = pclMultiStringSubCategory->GetString( iLoopType );

				// Product category "Accessory" is not added in the tree.
				// Accessories cannot be picked individually for now.
				if( 0 == strTypeID.Compare( _T("ACCTYPE_TAB") ) )
				{
					continue;
				}

				if( false == _CanDisplayType( strTypeID ) )
				{
					continue;
				}

				CTable *p = dynamic_cast<CTable *>( m_clDirectSelectionParams.m_pTADB->Get( strTypeID ).MP );
				ASSERT( NULL != p );
			
				if( NULL == p )
				{
					continue;
				}

				for( IDPTR IDptr = p->GetFirst(); NULL != IDptr.MP && false == bAtLeastOneSelectable; IDptr = p->GetNext() )
				{
					if( true == _IsSelectable( (CDB_Thing *)( IDptr.MP ) ) )
					{
						bAtLeastOneSelectable = true;
					}
				}
			}

			if( true == bAtLeastOneSelectable )
			{
				SortList.Add( _T(""), _tstof( pclMultiStringSubCategory->GetString( 1 ) ), (LPARAM)(pclMultiStringSubCategory) );
			}
		}

		// Fill the tree with the sorted sorted sub categories.
		CString str;
		LPARAM lparam;
		
		for( BOOL bContinue = SortList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lparam ) )
		{
			CDB_MultiString *pclMultiStringSubCategory = (CDB_MultiString *)lparam;
			ASSERT( NULL != pclMultiStringSubCategory );

			if( NULL == pclMultiStringSubCategory )
			{
				continue;
			}
			
			CTreeItemSubCategory *pclTreeItemSubCategory = new CTreeItemSubCategory( pclMultiStringSubCategory, pclTreeItemCategory );
			ASSERT( NULL != pclTreeItemSubCategory );

			if( NULL == pclTreeItemSubCategory )
			{
				continue;
			}

			m_vecItemDataInTreeList.push_back( pclTreeItemSubCategory );
			m_hParentItem = m_TreeProdCateg.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclMultiStringSubCategory->GetString( 0 ), 0, 0, 0, 0, (LPARAM)pclTreeItemSubCategory, hItemCategory, TVI_LAST );
			pclTreeItemSubCategory->m_hTreeItem = m_hParentItem;
		}

		// Third level is TYPE.
		// For each sub category, fill existing types.
		//  + Regulating Valve
		//    + Balancing Valve
		//    + Double Regulating Valve
		//    ...

		HTREEITEM hItemSubCategory = m_TreeProdCateg.GetChildItem( hItemCategory );

		while( NULL != hItemSubCategory )
		{
			CTreeItemSubCategory *pclTreeItemSubCategory = dynamic_cast<CTreeItemSubCategory *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemSubCategory ) );	
			ASSERT( NULL != pclTreeItemSubCategory );

			if( NULL == pclTreeItemSubCategory )
			{
				continue;
			}
			
			bool bForSetOnly = false;

			if( 0 == IDcmp( pclTreeItemSubCategory->m_pclSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_DPCONTRSET") ) )
			{
				bForSetOnly = true;
			}
			else if( 0 == IDcmp( pclTreeItemSubCategory->m_pclSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_STDCVACTSET") ) )
			{
				bForSetOnly = true;
			}
			else if( 0 == IDcmp( pclTreeItemSubCategory->m_pclSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ADVCTRLVSET") ) )
			{
				bForSetOnly = true;
			}

			// Sort TYPE list (for example: PICVTYPE_TAB -> PICVTYPE_STD, PICVTYPE_HIDP and PICVTYPE_CLASSICAL).
			SortList.PurgeAll();

			// Get Table ID of TYPE (for example: CVTYPE_TAB, BCVTYPE_TAB, ... ).
			// Loop on all types.
			// Remark: for subcategory, we now use a 'CDB_MultiString' object to have the possibility to add more than one type
			//         under a sub category.
			double dKeyFactor = 0.0;

			for( int iLoopType = 2; iLoopType < pclTreeItemSubCategory->m_pclSubCategory->GetNumofLines(); iLoopType++ )
			{
				CString strTypeID = pclTreeItemSubCategory->m_pclSubCategory->GetString( iLoopType );

				// Because now in a sub category we can have more than one type of product, it can happen that a sub category is visible
				// because at least one type can be displayed. But if one other type in the same sub category can't be displayed
				// we must not show this type.
				if( false == _CanDisplayType( strTypeID ) )
				{
					continue;
				}

				// Now, even if a type can be displayed, we check if there is a least one selectable product in it.
				bool bAtLeastOneSelectable = false;
				CTable *p = dynamic_cast<CTable *>( m_clDirectSelectionParams.m_pTADB->Get( strTypeID ).MP );
				ASSERT( NULL != p );
			
				if( NULL == p )
				{
					continue;
				}

				for( IDPTR IDptr = p->GetFirst(); NULL != IDptr.MP && false == bAtLeastOneSelectable; IDptr = p->GetNext() )
				{
					if( true == _IsSelectable( (CDB_Thing *)( IDptr.MP ) ) )
					{
						bAtLeastOneSelectable = true;
					}
				}

				if( false == bAtLeastOneSelectable )
				{
					continue;
				}

				IDPTR TypeTabIdptr = m_clDirectSelectionParams.m_pTADB->Get( strTypeID );
				ASSERT( NULL != TypeTabIdptr.MP);
			
				if( NULL == TypeTabIdptr.MP )
				{
					continue;
				}

				// Fill the sub tree.
				CTable *pTab = dynamic_cast<CTable *>( TypeTabIdptr.MP );
				ASSERT( NULL != pTab );
			
				for( idptr = pTab->GetFirst(); _T('\0') != *idptr.ID; idptr = pTab->GetNext() )
				{
					CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( idptr.MP );
					ASSERT( NULL != pStrID );

					if( NULL == pStrID )
					{
						continue;
					}

					SortList.Add( _T(""), _tstof( pStrID->GetIDstr() ) + dKeyFactor, (LPARAM)(pStrID) );
				}

				dKeyFactor += 1000.0;
			}

			// Fill the tree with the sorted ID's.
			CString str;
			LPARAM lparam;
			
			for( BOOL bContinue = SortList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lparam ) )
			{
				// Each CDB_StringID TYPE contains:
				//   GetString -> Type name.
				//   GetIDstr1 -> Order number.
				//   GetIDstr2 -> Table ID that contains products.
				CDB_StringID *pTypeIDstr = (CDB_StringID *)lparam;
				ASSERT( NULL != pTypeIDstr );

				if( NULL != pTypeIDstr )
				{
					// Insert type.
					HTREEITEM hChildItem = NULL;
					CTreeItemType *pclTreeItemType = new CTreeItemType( pTypeIDstr, pclTreeItemSubCategory );
					ASSERT( NULL != pclTreeItemType );

					if( NULL == pclTreeItemType )
					{
						continue;
					}

					m_vecItemDataInTreeList.push_back( pclTreeItemType );
					hChildItem = m_TreeProdCateg.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pTypeIDstr->GetString(), 0, 0, 0, 0, (LPARAM)pclTreeItemType, hItemSubCategory, TVI_LAST );
					pclTreeItemType->m_hTreeItem = hChildItem;

					// Insert family list for current category and type (For example: DpC-High Dp -> DA 516, DA 50, DAF 516 and DAF 50).
					int iCount;

					if( 0 == IDcmp( pclTreeItemSubCategory->m_pclSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ACTUATOR") ) )
					{
						iCount = FillTPC_ActuatorFamilies( pclTreeItemType, hChildItem );
					}
					// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
					else if( 0 == IDcmp( pclTreeItemSubCategory->m_pclSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_MEASTOOLS") ) )
					{
						iCount = FillTPC_MeasToolsFamilies( pclTreeItemType, hChildItem );
					}
					else
					{
						// Do an exception for the smart differential pressure controller that are not set but has the same behavior.
						// TODO: Find one other method instead of hard coding !!!!!!
						if( 0 == CString( pTypeIDstr->GetIDPtr().ID ).Compare( _T("SMARTDPCTYPE" ) ) )
						{
							// HYS-1992: Is a set if it is define in the database.
							CTable* p = dynamic_cast<CTable*>(m_clDirectSelectionParams.m_pTADB->Get( pTypeIDstr->GetIDstr2() ).MP);
							ASSERT( NULL != p );

							if( NULL == p )
							{
								continue;
							}

							for( IDPTR IDptr = p->GetFirst(); NULL != IDptr.MP && true != bForSetOnly; IDptr = p->GetNext() )
							{
								if( true == ( (CDB_Product*)(IDptr.MP) )->IsPartOfaSet() )
								{
									bForSetOnly = true;
								}
							}
						}

						iCount = FillTPC_ProductFamilies( pclTreeItemType, hChildItem, bForSetOnly );
					}

					// If there is no product or actuator for this category, we remove it.
					// Remark: it's the case for US version where Differential controller category is available but there is not set!
					if(0 == iCount)
					{
						m_TreeProdCateg.DeleteItem(hChildItem);
					}
				}
			}

			// Jump to the next category.
			hItemSubCategory = m_TreeProdCateg.GetNextItem( hItemSubCategory, TVGN_NEXT );
		}

		RemoveEmptyNode( &m_TreeProdCateg, 2 );
	}
}

int CDlgDirSel::FillTPC_ProductFamilies( CTreeItemType *pclTreeItemType, HTREEITEM hCurrentItem, bool bForSetOnly )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	int iCount = 0;

	// Get table with all the products.
	CTable *pTab = dynamic_cast<CTable *>( m_clDirectSelectionParams.m_pTADB->Get( pclTreeItemType->m_pclTypeID->GetIDstr2() ).MP );
	ASSERT( NULL != pTab );

	CRank SortList;

	// Check for each one the family and add it if it doesn't exist.
	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext() )
	{
		CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( idptr.MP );

		if( NULL == pclProduct )
		{
			continue;
		}
		
		if( false == _IsSelectable( pclProduct ) )
		{
			continue;
		}
		
		if( true == bForSetOnly && true != pclProduct->IsPartOfaSet() )
		{
			continue;
		}

		if( false == bForSetOnly && ePartOfaSet::ePartOfaSetYes == pclProduct->GetPartOfaSet() )
		{
			continue;
		}

		// HYS-1877: Do not consider product which has no set correspondance.
		// Due to a correction on virtual functions, here we call (for example) cdb_6wayValve::GetTableSet which is always different from 0.
		if( true == bForSetOnly && NULL != pclProduct->GetTableSet() && NULL == pclProduct->GetTableSet()->FindSet( pclProduct->GetIDPtr().ID, _T("") ) )
		{
			continue;
		}

		CDB_StringID *pFamIDstr = dynamic_cast<CDB_StringID *>( pclProduct->GetFamilyIDPtr().MP );

		if( NULL == pFamIDstr )
		{
			ASSERT( 0 );
			return iCount;
		}

		if( 0 == IDcmp( pFamIDstr->GetIDstr2(), pclTreeItemType->m_pclTypeID->GetIDPtr().ID ) )
		{		
			CString str;
			LPARAM lparam = (LPARAM)pFamIDstr;
			double dKey = 0;
			
			// Check if the family is already in the sorted list.
			if( TRUE == SortList.GetaCopy( str, dKey, lparam ) )
			{
				continue;	
			}

			SortList.Add( _T(""), _tstof( pFamIDstr->GetIDstr() ), (LPARAM)pFamIDstr );
		}
	}
	
	// Transfer sorted list into the tree.
	CString str;
	LPARAM lparam;

	for( BOOL bContinue = SortList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lparam ) )
	{
		CDB_StringID *pFamStrID = (CDB_StringID *)lparam;

		if( NULL == pFamStrID )
		{
			continue;
		}

		CTreeItemFamily *pclTreeItemFamily = new CTreeItemFamily( pFamStrID, pclTreeItemType );

		if( NULL == pclTreeItemFamily )
		{
			continue;
		}

		m_vecItemDataInTreeList.push_back( pclTreeItemFamily );
		m_hParentItem = m_TreeProdCateg.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pFamStrID->GetString(), 0, 0, 0, 0, (LPARAM)pclTreeItemFamily, hCurrentItem, TVI_LAST );
		pclTreeItemFamily->m_hTreeItem = m_hParentItem;
		iCount++;
	}

	return iCount;
}

int CDlgDirSel::FillTPC_ActuatorFamilies( CTreeItemType *pclTreeItemType, HTREEITEM hCurrentItem )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	int iCount = 0;
	CTable *pTab = dynamic_cast<CTable *>( m_clDirectSelectionParams.m_pTADB->Get( pclTreeItemType->m_pclTypeID->GetIDstr2() ).MP );
	ASSERT( NULL != pTab );

	CRank SortList;
	
	// Check for each one the family and add it if it doesn't exist.
	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext() )
	{
		CDB_Actuator *pAct = dynamic_cast<CDB_Actuator *>( idptr.MP );
		ASSERT( NULL != pAct );

		if( NULL == pAct ) 
		{
			continue;
		}
		
		if( false == _IsSelectable( pAct ) )
		{
			continue;
		}
		
		CDB_StringID *pFamIDstr = dynamic_cast<CDB_StringID *>( pAct->GetFamilyIDPtr().MP );

		if( NULL == pFamIDstr )
		{
			ASSERT( 0 );
			return iCount;
		}

		if( 0 == IDcmp( pclTreeItemType->m_pclTypeID->GetIDPtr().ID, pFamIDstr->GetIDstr2() ) )
		{
			CString str;
			LPARAM lparam = (LPARAM)pFamIDstr;
			double dKey = 0;
			
			// Check if the family is already in the sorted list.
			if( TRUE == SortList.GetaCopy( str, dKey, lparam ) )
			{
				continue;	
			}

			SortList.Add( _T(""), _tstof( pFamIDstr->GetIDstr() ), (LPARAM)pFamIDstr );
		}
	}
	
	CString str;
	LPARAM lpParam;

	for( BOOL bContinue = SortList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lpParam ) )
	{
		CDB_StringID *pFamStrID = (CDB_StringID *)lpParam;

		if( NULL == pFamStrID )
		{
			continue;
		}

		CTreeItemFamily *pclTreeItemFamily = new CTreeItemFamily( pFamStrID, pclTreeItemType );

		if( NULL == pclTreeItemFamily )
		{
			continue;
		}

		m_vecItemDataInTreeList.push_back( pclTreeItemFamily );
		m_hParentItem = m_TreeProdCateg.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pFamStrID->GetString(), 0, 0, 0, 0, (LPARAM)pclTreeItemFamily, hCurrentItem, TVI_LAST );
		pclTreeItemFamily->m_hTreeItem = m_hParentItem;
		iCount++;
	}

	return iCount;
}

int CDlgDirSel::FillTPC_MeasToolsFamilies( CTreeItemType *pclTreeItemType, HTREEITEM hCurrentItem )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	int iCount = 0;
	CTable *pTab = dynamic_cast<CTable *>( m_clDirectSelectionParams.m_pTADB->Get( pclTreeItemType->m_pclTypeID->GetIDstr2() ).MP );
	ASSERT( NULL != pTab );

	CRank SortList;
	
	// Check for each one the family and add it if it doesn't exist.
	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext() )
	{
		CDB_Product *pMeasuringTools = dynamic_cast<CDB_Product*>( idptr.MP );
		ASSERT( NULL != pMeasuringTools );

		if( NULL == pMeasuringTools ) 
		{
			continue;
		}
		
		if( false == _IsSelectable( pMeasuringTools ) )
		{
			continue;
		}
		
		CDB_StringID *pFamIDstr = dynamic_cast<CDB_StringID *>( pMeasuringTools->GetFamilyIDPtr().MP );

		if( NULL == pFamIDstr )
		{
			ASSERT( 0 );
			return iCount;
		}

		if( 0 == IDcmp( pclTreeItemType->m_pclTypeID->GetIDPtr().ID, pFamIDstr->GetIDstr2() ) )
		{
			CString str;
			LPARAM lparam = (LPARAM)pFamIDstr;
			double dKey = 0;
			
			// Check if the family is already in the sorted list.
			if( TRUE == SortList.GetaCopy( str, dKey, lparam ) )
			{
				continue;	
			}

			SortList.Add( _T(""), _tstof( pFamIDstr->GetIDstr() ), (LPARAM)pFamIDstr );
		}
	}
	
	CString str;
	LPARAM lpParam;

	for( BOOL bContinue = SortList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lpParam ) )
	{
		CDB_StringID *pFamStrID = (CDB_StringID *)lpParam;

		if( NULL == pFamStrID )
		{
			continue;
		}

		CTreeItemFamily *pclTreeItemFamily = new CTreeItemFamily( pFamStrID, pclTreeItemType );

		if( NULL == pclTreeItemFamily )
		{
			continue;
		}

		m_vecItemDataInTreeList.push_back( pclTreeItemFamily );
		m_hParentItem = m_TreeProdCateg.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pFamStrID->GetString(), 0, 0, 0, 0, (LPARAM)pclTreeItemFamily, hCurrentItem, TVI_LAST );
		pclTreeItemFamily->m_hTreeItem = m_hParentItem;
		iCount++;
	}

	return iCount;
}

void CDlgDirSel::FillTP_Products( CTreeItemFamily *pclTreeItemFamily, bool bSetAlone )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_TreeProducts.SetRedraw( FALSE );

	// Set image list.
	if( m_TreeProducts.GetImageList( TVSIL_NORMAL ) != &m_TreeProdImgList )
	{
		m_TreeProducts.SetImageList( &m_TreeProdImgList, TVSIL_NORMAL );
	}

	// Fill the product tree with the appropriate selected family product.
	// Get the ID of the family and the type to compare with all products.
	IDPTR FamIdptr = pclTreeItemFamily->m_pclFamilyID->GetIDPtr();

	// Get the table that contains all products.
	IDPTR idptr = m_clDirectSelectionParams.m_pTADB->Get( pclTreeItemFamily->m_pclParentTypeID->GetIDstr2() );
	CTable *pTab = dynamic_cast<CTable *>( idptr.MP );
	ASSERT( NULL != pTab );

	// Create a sorting list with all products.
	CRank SortList;
	SortList.PurgeAll();

	// By 'first level' we mean the accessories for the first level product.
	bool bIsFirstLevelAccessoryForSet = false;
	std::multimap<CString, CDB_Product *> multimapNameSorted;
	
	for( idptr = pTab->GetFirst(); _T('\0') != *idptr.ID; idptr = pTab->GetNext() )
	{
		CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( idptr.MP );
	
		if( false == _IsSelectable( pclProduct ) ) 
		{
			continue;
		}
		
		if( pclProduct->GetFamilyIDPtr().MP != pclTreeItemFamily->m_pclFamilyID )
		{
			continue;
		}

		if( true == bSetAlone && true != pclProduct->IsPartOfaSet() )
		{
			continue;
		}
		
		if( false == bSetAlone && ePartOfaSet::ePartOfaSetYes == pclProduct->GetPartOfaSet() )
		{
			continue;
		}

		double dKey = 0;
		
		// Specific treatment for CDB_Vessel
		CDB_Vessel *pVssl = dynamic_cast<CDB_Vessel *>( pclProduct );
		CDB_TecBox *pTecBox = dynamic_cast<CDB_TecBox *> ( pclProduct );
		CDB_SafetyValveBase *pclSafetyValveBase = dynamic_cast<CDB_SafetyValveBase *>( pclProduct );
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclProduct );

		// HYS-1009: specific treatment for CDB_PlenoRefill.
		CDB_PlenoRefill *pclPlenoRefill = dynamic_cast<CDB_PlenoRefill *>( pclProduct );

		// HYS-1241: specific treatment for CDB_6WayValve.
		CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pclProduct );

		// HYS-1571: specific treatment for CDB_FloorHeatingManifold, CDB_FloorHeatingController and CDB_DpReliefValve.
		CDB_FloorHeatingManifold *pclFloorHeatingManifold = dynamic_cast<CDB_FloorHeatingManifold *>( pclProduct );
		CDB_FloorHeatingController *pclFloorHeatingController = dynamic_cast<CDB_FloorHeatingController *>( pclProduct );
		CDB_DpReliefValve *pclDpReliefValve = dynamic_cast<CDB_DpReliefValve *>( pclProduct );
		
		if( NULL != pVssl )
		{
			CProductSortKey::CPSKMask mask(CProductSortKey::PSKM_Type + CProductSortKey::PSKM_PriorL + CProductSortKey::PSKM_Family);
			dKey = pclProduct->GetSortingKey(&mask);
			dKey = dKey * 100 + pVssl->GetPmaxmax() / 100000;
			dKey = dKey * 100 + pVssl->GetNominalVolume();
		}
		else if( NULL != pTecBox )
		{
			CProductSortKey::CPSKMask mask( CProductSortKey::PSKM_PriorL + CProductSortKey::PSKM_Type + CProductSortKey::PSKM_Family );
			dKey = pclProduct->GetSortingKey( &mask );
			
			// HYS-1004: Sort tecbox Transfero and Vento.
			if( ( CDB_TecBox::etbtVento == pTecBox->GetTecBoxType() ) || ( CDB_TecBox::etbtTransfero == pTecBox->GetTecBoxType() ) )
			{
				// Number of pump/compressor can be 0, 1 or 2. One digit is enough (this is why *10).
				dKey = dKey * 10 + pTecBox->GetNbrPumpCompressor();

				if( CDB_TecBox::etbtTransfero == pTecBox->GetTecBoxType() )
				{
					CDB_TBCurve *pclTBCurve = dynamic_cast<CDB_TBCurve *>( pTecBox->GetPumpComprCurveIDPtr().MP );
					dKey = dKey * 1000 + pclTBCurve->GetPmin() / 10000;
				}
				else
				{
					CDB_TBPlenoVento *pclTBVento = dynamic_cast<CDB_TBPlenoVento *>( pTecBox );
					dKey = dKey * 1000 + ( pclTBVento->GetWorkingPressRangeMin() / 10000);
				}

				double variant = 0.0;

				if( pTecBox->IsVariantEcoAuto() &&  pTecBox->IsVariantHighFlowTemperature() && pTecBox->IsVariantCooling() )
				{
					variant = 3;
				}
				else if ( pTecBox->IsVariantEcoAuto() && pTecBox->IsVariantCooling() )
				{
						variant = 2;
				}
				else if ( pTecBox->IsVariantEcoAuto()  && pTecBox->IsVariantHighFlowTemperature() )
				{
						variant = 1;
				}
				else if ( pTecBox->IsVariantEcoAuto() )
				{
						variant = 0;
				}

				dKey = dKey * 10 + variant;
			}
			else
			{
				// Number of pump/compressor can be 0, 1 or 2. One digit is enough (this is why *10).
				dKey = dKey * 10 + pTecBox->GetNbrPumpCompressor();

				// Pmaxmax can go from 1e+5 to 9.9e+6 Pa / 1e+5 -> 0-99. Two digits are enough (this is why *100)
				dKey = dKey * 100 + pTecBox->GetPmaxmax() / 100000;

				// Weight can be from 0 to at least 999 kg (TI 212.2 = 305 kg)
				dKey = dKey * 1000 + pTecBox->GetWeight();
			}
		}
		else if( NULL != pclSafetyValveBase )
		{
			CTAPSortKey sKey( (CDB_TAProduct *)pclSafetyValveBase );

			dKey = sKey.GetSortingKeyCustomAsDouble( &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_All ), CTAPSortKey::TAPKO_Priority | CTAPSortKey::TAPKO_Inverse,
				CTAPSortKey::TAPKO_Size, -1 );

			// Set pressure is between 2.5 and 10 bar. We multiply by 10 and than we consider we can go up to 999.
			dKey = ( dKey * 1000 ) + ( pclSafetyValveBase->GetSetPressure() * 10.0 );
		}
		else if( NULL != pclPlenoRefill )
		{
			// Sort first with the version (PRVERS_STD or PRVERS_FILTER).
			CDB_StringID *pclVersion = (CDB_StringID *)( pclPlenoRefill->GetVersionIDPtr().MP );
			pclVersion->GetIDstrAs<double>( 0, dKey );
			
			// Sort second with the function (ePRFunc_Soft, ePRFunc_Desalin or ePRFunc_None).
			dKey = ( dKey * 10.0 );

			if( CDB_PlenoRefill::ePRFunc_None == pclPlenoRefill->GetFunctions() )
			{
				dKey += 2;
			}
			else if( CDB_PlenoRefill::ePRFunc_Desalin == pclPlenoRefill->GetFunctions() )
			{
				dKey += 1;
			}

			// And finally add capacity.
			dKey = ( dKey * 10.0 ) + pclPlenoRefill->GetCapacity();
		}
		else if( NULL != pcl6WayValve )
		{
			// Priority level key.
			dKey = pcl6WayValve->GetPriorityLevel();

			// Size key.
			CDB_StringID *pclSize = (CDB_StringID *)( pcl6WayValve->GetSizeIDPtr().MP );
			ASSERT( NULL != pclSize );

			double dSize;
			pclSize->GetIDstrAs<double>( 0, dSize );
			ASSERT( dSize < 100.0 );

			dKey = ( dKey * 100.0 ) + dSize;

			// Kvs value.
			double dKvs = pcl6WayValve->GetKvs();
			ASSERT( dKvs < 10.0 );

			dKey = ( dKey * 10.0 ) + dKvs;
		}
		else if( NULL != pclFloorHeatingManifold )
		{
			dKey = (double)pclFloorHeatingManifold->GetHeatingCircuits();
		}
		else if( NULL != pclFloorHeatingController )
		{
			multimapNameSorted.insert( std::pair<CString, CDB_Product *>( pclFloorHeatingController->GetName(), pclFloorHeatingController ) );
		}
		else if( NULL != pclDpReliefValve )
		{
			if( 0 == CString( pclDpReliefValve->GetFamilyID() ).Compare( _T("FAM_PM512") ) )
			{
				// Priority level key.
				dKey = pclDpReliefValve->GetPriorityLevel();

				// Size key.
				CDB_StringID *pclSize = (CDB_StringID *)( pclDpReliefValve->GetSizeIDPtr().MP );
				ASSERT( NULL != pclSize );

				double dSize;
				pclSize->GetIDstrAs<double>( 0, dSize );
				ASSERT( dSize < 100.0 );

				dKey = ( dKey * 100.0 ) + dSize;
			}
			else if( 0 == CString( pclDpReliefValve->GetFamilyID() ).Compare( _T("FAM_HYDROLUX") ) )
			{
				// Priority level key.
				dKey = pclDpReliefValve->GetPriorityLevel();

				// Size key.
				CDB_StringID *pclSize = (CDB_StringID *)( pclDpReliefValve->GetSizeIDPtr().MP );
				ASSERT( NULL != pclSize );

				double dSize;
				pclSize->GetIDstrAs<double>( 0, dSize );
				ASSERT( dSize < 100.0 );

				dKey = ( dKey * 100.0 ) + dSize;

				// Minimum preset differential pressure.
				double dMinSettingDp = pclDpReliefValve->GetMinSettingDp() / 1000.0;
				ASSERT( dMinSettingDp < 1000.0 );

				dKey = ( dKey * 1000.0 ) + dMinSettingDp;

				// After that we don't have any key to place sleeve female thread (5501-03.000) before flat sealing with fitting (5503-03.000).
				// So we will sort on the 4 first digits of the article number.
				TCHAR *ptcEnd;
				double dArtNumber = _tcstod( CString( pclDpReliefValve->GetArtNum() ).Left( 4 ), &ptcEnd );
				dKey = ( dKey * 1000.0 ) + dArtNumber;
			}
			else
			{
				// Priority level key.
				dKey = pclDpReliefValve->GetPriorityLevel();

				// Pmaxmin/Pmaxmax key.
				double dPmaxmin = pclDpReliefValve->GetPmaxmax() / 100000.0;
				ASSERT( dPmaxmin < 100.0 );

				dKey = ( dKey * 100.0 ) + dPmaxmin;

				// Size key.
				CDB_StringID *pclSize = (CDB_StringID *)( pclDpReliefValve->GetSizeIDPtr().MP );
				ASSERT( NULL != pclSize );

				double dSize;
				pclSize->GetIDstrAs<double>( 0, dSize );
				ASSERT( dSize < 100.0 );

				dKey = ( dKey * 100.0 ) + dSize;

				// Connection key.
				CDB_StringID *pclConnect = (CDB_StringID *)( pclDpReliefValve->GetConnectIDPtr().MP );
				ASSERT( NULL != pclConnect );

				double dConnection;
				pclConnect->GetIDstrAs<double>( 0, dConnection );
				ASSERT( dConnection < 100.0 );

				dKey = ( dKey * 100.0 ) + dConnection;

				// Minimum preset differential pressure.
				double dMinSettingDp = pclDpReliefValve->GetMinSettingDp() / 1000.0;
				ASSERT( dMinSettingDp < 1000.0 );

				dKey = ( dKey * 1000.0 ) + dMinSettingDp;
			}
		}
		else
		{
			// HYS-1010: special treatment for KTCM 512, KTM 512 and KTM 50.
			if( 0 == CString( pclProduct->GetFamilyID() ).Compare( _T("FAM_KTM512") ) )
			{
				CDB_PIControlValve *pclPIBControlValve = dynamic_cast<CDB_PIControlValve *>( pclProduct );
				ASSERT( NULL != pclPIBControlValve );

				// Priority level key.
				dKey = pclProduct->GetPriorityLevel();

				// Size key.
				CDB_StringID *pclSize = (CDB_StringID *)( pclPIBControlValve->GetSizeIDPtr().MP );
				ASSERT( NULL != pclSize );

				double dSize;
				pclSize->GetIDstrAs<double>( 0, dSize );
				ASSERT( dSize < 100.0 );

				dKey = ( dKey * 100.0 ) + dSize;

				// Connect key.
				CDB_StringID *pclConnect = (CDB_StringID *)( pclPIBControlValve->GetConnectIDPtr().MP );
				ASSERT( NULL != pclConnect );

				double dConnect;
				pclConnect->GetIDstrAs<double>( 0, dConnect );
				ASSERT( dConnect < 100.0 );

				dKey = ( dKey * 100.0 ) + dConnect;

				// Version key.
				CDB_StringID *pclVersion = (CDB_StringID *)( pclPIBControlValve->GetVersionIDPtr().MP );
				ASSERT( NULL != pclVersion );

				double dVersion;
				pclVersion->GetIDstrAs<double>( 0, dVersion );
				ASSERT( dVersion < 100.0 );

				dKey = ( dKey * 100.0 ) + dVersion;

				// Min flow (KTCM512 15LF) = 5.8333333333e-5 (m3/s) / Max flow (KTM512 125) = 1.524375000e-2
				// 5.8333333333e-5 * 1e5 = 5.8333333333
				// 1.524375000e-2 * 1e5  = 152000.0
				CDB_PICVCharacteristic *pclPIBCVCharacteristic = dynamic_cast<CDB_PICVCharacteristic *>( pclPIBControlValve->GetPICVCharacteristic() );
				ASSERT( NULL != pclPIBCVCharacteristic );

				double dQmax = pclPIBCVCharacteristic->GetQmax();
				ASSERT( dQmax < 1.0e-1 );

				dKey = ( dKey * 1.0e6 ) + ( dQmax * 1.0e5 );
			}
			else if( 0 == CString( pclProduct->GetFamilyID() ).Compare( _T("FAM_PILOT_R") ) )
			{
				// HYS-1011: special treatment for TA-PILOT-R.
				CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclProduct );
				ASSERT( NULL != pclDpController );

				// Priority level key.
				dKey = pclProduct->GetPriorityLevel();

				// Size key.
				CDB_StringID *pclSize = (CDB_StringID *)( pclDpController->GetSizeIDPtr().MP );
				ASSERT( NULL != pclSize );

				double dSize;
				pclSize->GetIDstrAs<double>( 0, dSize );
				ASSERT( dSize < 100.0 );

				dKey = ( dKey * 100.0 ) + dSize;

				// Version key.
				CDB_StringID *pclVersion = (CDB_StringID *)( pclDpController->GetVersionIDPtr().MP );
				ASSERT( NULL != pclVersion );

				double dVersion;
				pclVersion->GetIDstrAs<double>( 0, dVersion );
				ASSERT( dVersion < 100.0 );

				dKey = ( dKey * 100.0 ) + dVersion;

				// PN key: 1.6e+6 or 2.5e+6 -> / 1e5 -> 16 or 25.
				double dPN = pclDpController->GetPmaxmax() / 1e5;
				ASSERT( dPN < 100.0 );

				dKey = ( dKey * 100.0 ) + dPN;

				// Dplmin: min = 10000 Pa ; max = 80000 Pa / 10000 -> 1; 8
				CDB_DpCCharacteristic *pclDpCCharacteristic = dynamic_cast<CDB_DpCCharacteristic *>( pclDpController->GetDpCCharacteristic() );
				ASSERT( NULL != pclDpCCharacteristic );

				double dDplmin = pclDpCCharacteristic->GetDplmin() / 10000.0;
				ASSERT( dDplmin < 10.0 );

				dKey = ( dKey * 10.0 ) + dDplmin;
			}
			else if( 0 == CString( pclProduct->GetFamilyID() ).Compare( _T("FAM_DA516") ) 
					|| 0 == CString( pclProduct->GetFamilyID() ).Compare( _T("FAM_DAF516") )
					|| 0 == CString( pclProduct->GetFamilyID() ).Compare( _T("FAM_DAL516") ) )
			{
				// HYS-1011: special treatment for DA 516 and DAF 516.
				CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclProduct );
				ASSERT( NULL != pclDpController );

				// Priority level key.
				dKey = pclProduct->GetPriorityLevel();

				// Size key.
				CDB_StringID *pclSize = (CDB_StringID *)( pclDpController->GetSizeIDPtr().MP );
				ASSERT( NULL != pclSize );

				double dSize;
				pclSize->GetIDstrAs<double>( 0, dSize );
				ASSERT( dSize < 100.0 );

				dKey = ( dKey * 100.0 ) + dSize;

				// Connect key.
				CDB_StringID *pclConnect = (CDB_StringID *)( pclDpController->GetConnectIDPtr().MP );
				ASSERT( NULL != pclConnect );

				double dConnect;
				pclConnect->GetIDstrAs<double>( 0, dConnect );
				ASSERT( dConnect < 100.0 );

				dKey = ( dKey * 100.0 ) + dConnect;

				// Dplmin: min = 10000 Pa ; max = 130000 Pa / 10000 -> 1; 13
				CDB_DpCCharacteristic *pclDpCCharacteristic = dynamic_cast<CDB_DpCCharacteristic *>( pclDpController->GetDpCCharacteristic() );
				ASSERT( NULL != pclDpCCharacteristic );

				double dDplmin = pclDpCCharacteristic->GetDplmin() / 10000.0;
				ASSERT( dDplmin < 100.0 );

				dKey = ( dKey * 100.0 ) + dDplmin;

				// Dplmax: min = 30000 Pa ; max = 150000 Pa / 10000 -> 3; 15
				double dDplmax = pclDpCCharacteristic->GetDplmax() / 10000.0;
				ASSERT( dDplmax < 100.0 );

				dKey = ( dKey * 100.0 ) + dDplmax;
			}
			else if( 0 == CString( pclProduct->GetFamilyID() ).Compare( _T("FAM_DAF50") ) )
			{
				// HYS-1011: special treatment for DA 50 and DAF 50.
				CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclProduct );
				ASSERT( NULL != pclDpController );

				// Priority level key.
				dKey = pclProduct->GetPriorityLevel();

				// Size key.
				CDB_StringID *pclSize = (CDB_StringID *)( pclDpController->GetSizeIDPtr().MP );
				ASSERT( NULL != pclSize );

				double dSize;
				pclSize->GetIDstrAs<double>( 0, dSize );
				ASSERT( dSize < 100.0 );

				dKey = ( dKey * 100.0 ) + dSize;

				// Dplmin: min = 50000 Pa ; max = 130000 Pa / 1000 -> 50; 130
				CDB_DpCCharacteristic *pclDpCCharacteristic = dynamic_cast<CDB_DpCCharacteristic *>( pclDpController->GetDpCCharacteristic() );
				ASSERT( NULL != pclDpCCharacteristic );

				double dDplmin = pclDpCCharacteristic->GetDplmin() / 1000.0;
				ASSERT( dDplmin < 1000.0 );

				dKey = ( dKey * 1000.0 ) + dDplmin;
			}
			else if( 0 == CString( pclProduct->GetFamilyID() ).Compare( _T("FAM_VHV") ) || 0 == CString( pclProduct->GetFamilyID() ).Compare( _T("FAM_VHF") ) )
			{
				// HYS-266: special treatment for Insert
				CDB_ControlValve *pclThermoInsert = dynamic_cast<CDB_ControlValve *>( pclProduct );
				ASSERT( NULL != pclThermoInsert );

				// Priority level key.
				dKey = pclProduct->GetPriorityLevel();
				double dKvs = pclThermoInsert->GetKvs();
				ASSERT( dKvs < 100.0 );
				dKey = ( dKey * 100.0 ) + dKvs;
			}
			// HYS-1741: IPI March 2021
			else if( 0 == CString( pclProduct->GetFamilyID() ).Compare( _T( "ACCSFAM_PRV_G06F" ) ) )
			{
				CDB_TAProduct* pclTAProduct = dynamic_cast<CDB_TAProduct*>(pclProduct);
				// Size key.
				if( NULL != pclTAProduct )
				{
					dKey = pclProduct->GetPriorityLevel();
					CDB_StringID* pclSize = (CDB_StringID*)(pclTAProduct->GetSizeIDPtr().MP);
					if( NULL != pclSize )
					{
						double dSize;
						pclSize->GetIDstrAs<double>( 0, dSize );
						if( dSize < 100.0 )
						{
							dKey = (dKey * 100.0) + dSize;
						}
					}
				}
            }
			else if(NULL != dynamic_cast<CDB_DpSensor *>(pclProduct) )
			{
				// NOT GOOD -> To review !!!
				/*
			    CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclProduct );
				dKey = pclDpSensor->GetSortingKey();
				dKey = dKey *100 + pclDpSensor->GetMinMeasurableDp();
				*/
			}
			else
			{
				dKey = pclProduct->GetSortingKey();
			}
		}

		SortList.Add( _T(""), dKey, (LPARAM)pclProduct );
	}

	// If list must be sorted by name.
	if( multimapNameSorted.size() > 0 )
	{
		SortList.PurgeAll();
		double dKey = 0;

		for( auto &iter : multimapNameSorted )
		{
			SortList.Add( _T(""), dKey++, (LPARAM)iter.second );
		}
	}

	// HYS-945: To avoid notification of OnTvnSelChangedTreeProducts when deleting all items
	m_bSkipTvnSelChangedProduct = true;
	m_TreeProducts.DeleteAllItems();
	m_bSkipTvnSelChangedProduct = false;

	m_TreeProducts.ResetAllColors();

	// Fill the product tree with the sorted products (level 1).
	CString str;
	LPARAM lparam;
	
	for( BOOL bContinue = SortList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lparam ) )
	{
		CDB_Product *pclProduct = (CDB_Product*)lparam;
		
		if( NULL == pclProduct )
		{
			continue;
		}

		CString str;
		_string strstd;
		pclProduct->GetNameEx( &strstd, false );
		str = strstd.c_str();

		// If it's a control valve, we add Kvs (or Cv) after the product name...
		if( true == pclProduct->IsClass( CLASS( CDB_ControlValve ) ) )
		{
			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pclProduct );
				
			if( NULL != pCV )
			{
				str += CString( _T(" - ") );
				CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
					
				if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				{
					str += TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_KVS );
				}
				else
				{
					str += TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CV );
				}
					
				str += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pCV->GetKvs() );
			}
		}

		CTreeItemProduct *pclTreeItemProduct = new CTreeItemProduct( pclProduct, pclTreeItemFamily );

		if( NULL == pclTreeItemProduct )
		{
			continue;
		}

		m_vecItemDataInTreeList.push_back( pclTreeItemProduct );
		m_hParentItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 1, 1, 0, 0, (LPARAM)pclTreeItemProduct, TVI_ROOT, TVI_LAST );
		pclTreeItemProduct->m_hTreeItem = m_hParentItem;

		if( false == pclProduct->IsSelectable( true ) )
		{
			m_TreeProducts.SetItemColor( m_hParentItem, _RED );
		}
	}
	
	// Add compatible accessories (level 2).
	// ATTENTION: we don't have for the moment accessory sets for the level 2. If it's the case, we need to verify above
	// before to call thist method.
	FillTP_CompatibleAccessories( pclTreeItemFamily, NULL, bIsFirstLevelAccessoryForSet );

	// HYS-945: Buffer vessels management
	if( 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGPMWQ_TECBOX") ) )
	{
		FillTP_CompatibleBufferVessels( pclTreeItemFamily);
	}

	// Add compatible actuators for specific products (level 2).
	if(	0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_STDCTRLVALV") )
			|| 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ADVCTRLVALV") )
			|| 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_DPCBCV") )
			|| 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGTC_TRVALV") )
			|| 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_STDCVACTSET") )
			|| 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ADVCTRLVSET") )
			|| 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGESC_SHUTOFF" ) ) )
	{
		FillTP_CompatibleAdapters( pclTreeItemFamily );
		FillTP_CompatibleActuators( pclTreeItemFamily );
	}
	
	if( 0 == IDcmp( pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T( "SUBCATEGBC_DPCONTR" ) ) )
	{
		// For smart differential pressure controller.
		FillTP_CompatibleSmartDpCSets( pclTreeItemFamily );
	}
	// Add compatible measuring valves or Dp sensor for differential pressure controller set.
	if( true == m_bWorkingForDpCSet )
	{
		// For standard Dp controller.
		FillTP_CompatibleMeasuringValves( pclTreeItemFamily );
	}

	RemoveEmptyNode( &m_TreeProducts );

	m_TreeProducts.SetRedraw( TRUE );
}

void CDlgDirSel::FillTP_Actuators( CTreeItemFamily *pclTreeItemFamily )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_TreeProducts.SetRedraw( FALSE );

	// Set image list.
	if( m_TreeProducts.GetImageList( TVSIL_NORMAL ) != &m_TreeProdImgList )
	{
		m_TreeProducts.SetImageList( &m_TreeProdImgList, TVSIL_NORMAL );
	}

	IDPTR FamIdptr = pclTreeItemFamily->m_pclFamilyID->GetIDPtr();
	
	// Get the table that contains all actuators.
	IDPTR IDPtr = m_clDirectSelectionParams.m_pTADB->Get( pclTreeItemFamily->m_pclParentTypeID->GetIDstr2() );
	CTable *pTab = dynamic_cast<CTable *>( IDPtr.MP );
	ASSERT( NULL != pTab );
	
	// Create a sorting list with all actuators.
	CRank SortList;
	SortList.PurgeAll();

	for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( IDPtr.MP );

		if( false == pclActuator )
		{
			continue;
		}
		
		if( false == _IsSelectable( pclActuator ) )
		{
			continue;
		}
		
		if( pclActuator->GetFamilyIDPtr().MP != pclTreeItemFamily->m_pclFamilyID )
		{
			continue;
		}
		
		CString str = pclActuator->GetName();
		double dKey = 0.0;

		if( 0 == CString( pclActuator->GetFamilyID() ).Compare( _T("ACTFAM_MC") ) )
		{
			// HYS-1012: special treatment for TA-MC actuators.
			CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );
			ASSERT( NULL != pclElectroActuator );

			// Priority level key.
			dKey = pclElectroActuator->GetPriorityLevel();

			// Max force: Min force = 150N (TA-MC 15); Max force = 15000N (TA-MC 1500).
			double dMaxForce = pclElectroActuator->GetMaxForceTorque() / 100.0;
			ASSERT( dMaxForce < 1000.0 );

			dKey = ( dKey * 1000.0 ) + dMaxForce;

			// Sort key.
			double dSortKey = (double)pclElectroActuator->GetSortInt();
			ASSERT( dSortKey < 10.0 );

			dKey = ( dKey * 10.0 ) + dSortKey;

			// Power.
			CDB_StringID *pclPower = (CDB_StringID *)( pclElectroActuator->GetPowerSupplyIDPtr( 0 ).MP );
			ASSERT( NULL != pclPower );

			double dPower;
			pclPower->GetIDstrAs<double>( 0, dPower );
			ASSERT( dPower < 10.0 );

			dKey = ( dKey * 10.0 ) + dPower;

			SortList.Add( str, dKey, (LPARAM)pclActuator, true );
		}
		else
		{
			dKey = pclActuator->GetOrderKey();
			SortList.Add( str, dKey, (LPARAM)pclActuator, false );
		}
	}
	
	// Fill the product tree with the sorted actuators.
	CString str;
	LPARAM lpParam;

	for( BOOL bContinue = SortList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lpParam ) )
	{
		CDB_Actuator *pclActuator = (CDB_Actuator *)lpParam;
		CString str = pclActuator->GetName() + CString(_T(" [") ) + pclActuator->GetArtNum() + CString( _T("]") );

		CTreeItemActuator *pclTreeItemActuator = new CTreeItemActuator( pclActuator, NULL, pclTreeItemFamily );

		if( NULL == pclTreeItemActuator )
		{
			continue;
		}

		m_vecItemDataInTreeList.push_back( pclTreeItemActuator );
		m_hParentItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 1, 1, 0, 0, (LPARAM)pclTreeItemActuator, TVI_ROOT, TVI_LAST );
		pclTreeItemActuator->m_hTreeItem = m_hParentItem;

		if( false == pclActuator->IsSelectable( true ) )
		{
			m_TreeProducts.SetItemColor( m_hParentItem, _RED );
		}

		// If actuator is deleted, we don't know its reference (article number) and thus we CAN NOT FIND AND SHOW its accessories.
		if( true == pclActuator->IsDeleted() )
		{
			continue;
		}

		// Fill sub tree accessories for an actuator.
		IDPTR ActAccIDPtr = pclActuator->GetAccessoriesGroupIDPtr();
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( ActAccIDPtr.MP );

		if( NULL != pclRuledTable )
		{
			CRank SortListActAcces;
			SortListActAcces.PurgeAll();

			// HYS-1617/HYS_1618: We call this method to disable the crossing table option in the 'CDB_RuledTable' table.
			TASApp.GetpTADB()->GetAccessories( &SortListActAcces, pclRuledTable, CTADatabase::ForDirSel, false );

			bool bTitleAccAdded = false;
			HTREEITEM hItemCompAccTitle = NULL;

			// Fill the accessory tree with the sorted ID's.
			for( BOOL bContinue2 = SortListActAcces.GetFirst( str, lpParam ); TRUE == bContinue2; bContinue2 = SortListActAcces.GetNext( str, lpParam ) )
			{
				if( false == bTitleAccAdded )
				{
					CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleAccessories );

					if( NULL == pclTreeItemTitle )
					{
						continue;
					}

					m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
					hItemCompAccTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLEACC ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, m_hParentItem, TVI_LAST );
					pclTreeItemTitle->m_hTreeItem = hItemCompAccTitle;
					bTitleAccAdded = true;
				}

				HTREEITEM hChildChildItem = NULL;
				
				CTreeItemAccessory *pclTreeItemAccessory = new CTreeItemAccessory( (CDB_Product *)lpParam, CAccessoryList::AccessoryType::_AT_Accessory, pclRuledTable, pclTreeItemFamily, pclActuator );

				if( NULL == pclTreeItemAccessory || NULL == pclTreeItemAccessory->m_pclAccessory || false == pclTreeItemAccessory->m_pclAccessory->IsAnAccessory() )
				{
					continue;
				}
				
				m_vecItemDataInTreeList.push_back( pclTreeItemAccessory );

				hChildChildItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclTreeItemAccessory->m_pclAccessory->GetName(), CheckboxNotSelected, CheckboxNotSelected, 0, 0, (LPARAM)pclTreeItemAccessory, hItemCompAccTitle, TVI_LAST );
				pclTreeItemAccessory->m_hTreeItem = hChildChildItem;
			}
		}
	}

	m_TreeProducts.SetRedraw(TRUE);
}

void CDlgDirSel::FillTP_MeasTools( CTreeItemFamily *pclTreeItemFamily )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_TreeProducts.SetRedraw( FALSE );

	// Set image list.
	if( m_TreeProducts.GetImageList( TVSIL_NORMAL ) != &m_TreeProdImgList )
	{
		m_TreeProducts.SetImageList( &m_TreeProdImgList, TVSIL_NORMAL );
	}

	IDPTR FamIdptr = pclTreeItemFamily->m_pclFamilyID->GetIDPtr();
	
	// Get the table that contains all measuring tools.
	IDPTR IDPtr = m_clDirectSelectionParams.m_pTADB->Get( pclTreeItemFamily->m_pclParentTypeID->GetIDstr2() );
	CTable *pTab = dynamic_cast<CTable *>( IDPtr.MP );
	ASSERT( NULL != pTab );
	
	// Create a sorting list with all measuring tools.
	CRank SortList;
	SortList.PurgeAll();

	for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Product *pclMeasuringTool = dynamic_cast<CDB_Product*>( IDPtr.MP );

		if( false == pclMeasuringTool )
		{
			continue;
		}
		
		if( false == _IsSelectable( pclMeasuringTool ) )
		{
			continue;
		}
		
		if( pclMeasuringTool->GetFamilyIDPtr().MP != pclTreeItemFamily->m_pclFamilyID )
		{
			continue;
		}
		
		CString str = pclMeasuringTool->GetName();
		double dKey = 0.0;

		if( 0 == CString( pclMeasuringTool->GetFamilyID() ).Compare( _T("FAM_TALINK") ) )
		{
			// Special treatment for .
			CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor*>( pclMeasuringTool );
			ASSERT( NULL != pclDpSensor );

			// Don't show the product if it is has not article number
			if( 0 == StringCompare( pclDpSensor->GetArtNum(), _T( "-" ) ) )
			{
				continue;
			}
			// Priority level key.
			dKey = pclDpSensor->GetSortingKey();
			dKey = dKey* 100 + pclDpSensor->GetMinMeasurableDp();
			SortList.Add( str, dKey, (LPARAM)pclMeasuringTool, true );
		}
		else
		{
			dKey = pclMeasuringTool->GetSortingKey();
			SortList.Add( str, dKey, (LPARAM)pclMeasuringTool, false );
		}
	}
	
	// Fill the product tree with the sorted measuring tools.
	CString str;
	LPARAM lpParam;

	for( BOOL bContinue = SortList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lpParam ) )
	{
		CDB_Product *pclMeasTools = (CDB_Product*)lpParam;
		CString str = CteEMPTY_STRING;
		if( 0 == CString( pclMeasTools->GetFamilyID() ).Compare( _T( "FAM_TALINK" ) ) )
		{
			str = ((CDB_DpSensor*)pclMeasTools)->GetFullName();
		}
		else
		{
			str = pclMeasTools->GetName();
		}

		CTreeItemProduct *pclTreeItemProduct = new CTreeItemProduct(pclMeasTools, pclTreeItemFamily );

		if( NULL == pclTreeItemProduct )
		{
			continue;
		}

		m_vecItemDataInTreeList.push_back( pclTreeItemProduct );
		m_hParentItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 1, 1, 0, 0, (LPARAM)pclTreeItemProduct, TVI_ROOT, TVI_LAST );
		pclTreeItemProduct->m_hTreeItem = m_hParentItem;

		if( false == pclMeasTools->IsSelectable( true ) )
		{
			m_TreeProducts.SetItemColor( m_hParentItem, _RED );
		}

		// If measuring tool is deleted, we don't know its reference (article number) and thus we CAN NOT FIND AND SHOW its accessories.
		if( true == pclMeasTools->IsDeleted() )
		{
			continue;
		}

		// Fill sub tree accessories for a mesuring tool.
		IDPTR MeasToolIDPtr = pclMeasTools->GetAccessoriesGroupIDPtr();
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( MeasToolIDPtr.MP );

		if( NULL != pclRuledTable )
		{
			CRank SortListAccess;
			SortListAccess.PurgeAll();

			// We call this method to disable the crossing table option in the 'CDB_RuledTable' table.
			TASApp.GetpTADB()->GetAccessories( &SortListAccess, pclRuledTable, CTADatabase::ForDirSel, false );

			bool bTitleAccAdded = false;
			HTREEITEM hItemCompAccTitle = NULL;

			// Fill the accessory tree with the sorted ID's.
			for( BOOL bContinue2 = SortListAccess.GetFirst( str, lpParam ); TRUE == bContinue2; bContinue2 = SortListAccess.GetNext( str, lpParam ) )
			{
				if( false == bTitleAccAdded )
				{
					CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleAccessories );

					if( NULL == pclTreeItemTitle )
					{
						continue;
					}

					m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
					hItemCompAccTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLEACC ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, m_hParentItem, TVI_LAST );
					pclTreeItemTitle->m_hTreeItem = hItemCompAccTitle;
					bTitleAccAdded = true;
				}

				HTREEITEM hChildChildItem = NULL;
				
				CTreeItemAccessory *pclTreeItemAccessory = new CTreeItemAccessory( (CDB_Product *)lpParam, CAccessoryList::AccessoryType::_AT_Accessory, pclRuledTable, pclTreeItemFamily, pclMeasTools );

				if( NULL == pclTreeItemAccessory || NULL == pclTreeItemAccessory->m_pclAccessory || false == pclTreeItemAccessory->m_pclAccessory->IsAnAccessory() )
				{
					continue;
				}
				
				m_vecItemDataInTreeList.push_back( pclTreeItemAccessory );

				hChildChildItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclTreeItemAccessory->m_pclAccessory->GetName(), CheckboxNotSelected, CheckboxNotSelected, 0, 0, (LPARAM)pclTreeItemAccessory, hItemCompAccTitle, TVI_LAST );
				pclTreeItemAccessory->m_hTreeItem = hChildChildItem;
			}
		}
	}

	m_TreeProducts.SetRedraw(TRUE);
}

void CDlgDirSel::FillTP_CompatibleActuators( CTreeItemFamily *pclTreeItemFamily )
{
	HTREEITEM hCurrentItem = m_TreeProducts.GetRootItem();

	while( NULL != hCurrentItem )
	{
		HTREEITEM hItemActuatorTitle = NULL;
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hCurrentItem );

		if( CTreeItemData::DataType::Product != pclTreeItemData->m_eDataType )
		{
			// Jump to the next root member.
			hCurrentItem = m_TreeProducts.GetNextItem( hCurrentItem, TVGN_NEXT );
		}

		bool bTitleActuatorAdded = false;

		CTreeItemProduct *pclTreeItemTAProduct = (CTreeItemProduct *)pclTreeItemData;
		ASSERT( NULL != pclTreeItemTAProduct );

		CRank clActuatorList;
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclTreeItemTAProduct->m_pclProduct );
		CDB_ShutoffValve *pclShutOffValve = dynamic_cast<CDB_ShutoffValve *>( pclTreeItemTAProduct->m_pclProduct );
		CDB_Product *pclProduct = NULL;

		// HYS-1877: For 6-way valve kit we only display one product in this column. We display the 6-way valve the
		// 6-way valve actuator is displayed at the next column.
		bool bIsFor6WayValveSet = false;

		// HYS-1381 : For Set we have to show compatible actuators even if it's deleted.
		if( NULL != pclControlValve && ( false == pclControlValve->IsDeleted() || true == pclControlValve->IsPartOfaSet() ) )
		{
			// Get all actuators.
			// Remark: actuators in 'clActuatorList' are already sorted with the good key.
			_GetActuatorListRelatedToControlValve( pclControlValve, &clActuatorList, m_bWorkingForDpCSet || m_bWorkingForControlSet );
			pclProduct = pclControlValve;
			// HYS-1877: Do we look for 6-way valve actuator ? 
			bIsFor6WayValveSet = (NULL != dynamic_cast<CDB_6WayValve*>(pclProduct)) ? true : false;
		}
		else if( NULL != pclShutOffValve && false == pclShutOffValve->IsDeleted() )
		{
			// Get all actuators.
			// Remark: actuators in 'clActuatorList' are already sorted with the good key.
			_GetActuatorListRelatedToShutOffValve( pclShutOffValve, &clActuatorList );
			pclProduct = pclShutOffValve;
		}

		if( NULL != pclProduct && ( ( false == bIsFor6WayValveSet ) || ( true == bIsFor6WayValveSet && 1 < clActuatorList.GetCount() ) ) )
		{
			FillTP_CompatibleActuatorsHelper( pclTreeItemFamily, hCurrentItem, clActuatorList, pclProduct );
		}

		// Jump to the next root member.
		hCurrentItem = m_TreeProducts.GetNextItem( hCurrentItem, TVGN_NEXT );
	}
}

void CDlgDirSel::FillTP_CompatibleActuatorsHelper( CTreeItemFamily *pclTreeItemFamily, HTREEITEM hCurrentItem, CRank &clActuatorList, CDB_Product *pclProduct )
{
	// Run all actuators (level 2).
	HTREEITEM hItemActuatorTitle = NULL;
	bool bTitleActuatorAdded = false;
	CString str;
	LPARAM lparam;

	for( BOOL bContinue = clActuatorList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = clActuatorList.GetNext( str, lparam ) )
	{
		CDB_Actuator *pclActuator = (CDB_Actuator *)lparam;

		if( false == bTitleActuatorAdded )
		{
			CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleActuators );

			if( NULL == pclTreeItemTitle )
			{
				continue;
			}

			m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
			hItemActuatorTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLEACT ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hCurrentItem, TVI_LAST );
			pclTreeItemTitle->m_hTreeItem = hItemActuatorTitle;
			bTitleActuatorAdded = true;
		}

		CTreeItemActuator *pclTreeItemActuator = new CTreeItemActuator( pclActuator, pclProduct, pclTreeItemFamily );

		if( NULL == pclTreeItemActuator )
		{
			continue;
		}

		m_vecItemDataInTreeList.push_back( pclTreeItemActuator );

		HTREEITEM hItemActuator = NULL;
		CString str = pclActuator->GetName() + CString(_T(" [") ) + pclActuator->GetArtNum() + CString(_T("]") );
		hItemActuator = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 1, 1, 0, 0, (LPARAM)pclTreeItemActuator, hItemActuatorTitle, TVI_LAST );
		pclTreeItemActuator->m_hTreeItem = hItemActuator;

		if( false == pclActuator->IsSelectable( true ) )
		{
			m_TreeProducts.SetItemColor( hItemActuator, _RED );
		}
						
		// If actuator is deleted, we don't know its reference (article number) and thus we CAN NOT FIND AND SHOW its accessories.
		if( true == pclActuator->IsDeleted() )
		{
			continue;
		}

		bool bTitleAccessoryAdded = false;
		HTREEITEM hItemCompAccTitle = hItemActuator;

		// Fill sub tree accessories for an actuator.
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclActuator->GetAccessoriesGroupIDPtr().MP );

		if( NULL != pclRuledTable )
		{
			// Sort the list.
			CRank SortListActAcces;

			// HYS-1617/HYS_1618: We call this method to activate the crossing table option in the 'CDB_RuledTable' table.
			TASApp.GetpTADB()->GetAccessories( &SortListActAcces, pclRuledTable, CTADatabase::ForDirSel, true );
						
			// Fill the accessory tree with the sorted ID's.
			CString str;
			LPARAM lparam;

			for( BOOL bContinue2 = SortListActAcces.GetFirst( str, lparam ); TRUE == bContinue2; bContinue2 = SortListActAcces.GetNext( str, lparam ) )
			{
				CDB_Product *pActuatorAccessory = (CDB_Product *)lparam;

				if( NULL == pActuatorAccessory || false == pActuatorAccessory->IsAnAccessory() )
				{
					continue;
				}

				if( false == bTitleAccessoryAdded )
				{
					CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleAccessories );

					if( NULL == pclTreeItemTitle )
					{
						continue;
					}

					m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
					hItemCompAccTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLEACC ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hItemActuator, TVI_LAST );
					pclTreeItemTitle->m_hTreeItem = hItemCompAccTitle;
					bTitleAccessoryAdded = true;
				}

				CTreeItemAccessory *pclTreeItemAccessory = new CTreeItemAccessory( pActuatorAccessory, CAccessoryList::AccessoryType::_AT_Accessory, pclRuledTable, pclTreeItemFamily, pclActuator );

				if( NULL == pclTreeItemAccessory )
				{
					continue;
				}

				m_vecItemDataInTreeList.push_back( pclTreeItemAccessory );
				int iImage = ( true == pActuatorAccessory->IsAttached() && true == m_bWorkingForControlSet ) ? CheckboxNotSelectedDisable : CheckboxNotSelected;
				pclTreeItemAccessory->m_hTreeItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pActuatorAccessory->GetName(), iImage, iImage, 0, 0, (LPARAM)pclTreeItemAccessory, hItemCompAccTitle, TVI_LAST );
			}
		}

		// Fill sub tree accessories for a set.
		CTableSet *pTabSet = pclProduct->GetTableSet();
			
		if( NULL != pTabSet )
		{
			std::set<CDB_Set *> pSetTable;
			// HYS-1381 : For Set we have to show compatible actuators even if it's deleted.
			if( pTabSet->FindCompatibleSet( &pSetTable, pclProduct->GetIDPtr().ID, pclActuator->GetIDPtr().ID, CTADatabase::FilterSelection::ForDirSel ) )
			{
				std::set<CDB_Set *>::const_iterator IterpSetTable;

				for( IterpSetTable = pSetTable.begin(); IterpSetTable != pSetTable.end(); IterpSetTable++ )
				{
					if( 0 == IDcmp( (*IterpSetTable)->GetAccGroupIDPtr().ID, _NO_ID ) )
					{
						continue;
					}

					CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( (*IterpSetTable)->GetAccGroupIDPtr().MP );
					ASSERT( NULL != pclRuledTable );

					if( NULL == pclRuledTable )
					{
						continue;
					}

					// Sort the list.
					CRank SortListActAcces;

					for( IDPTR idptr = pclRuledTable->GetFirst(); _T('\0') != *idptr.ID; idptr = pclRuledTable->GetNext() )
					{
						CDB_Product *pActuatorAccesory = dynamic_cast<CDB_Product *>( idptr.MP );
						ASSERT( NULL != pActuatorAccesory );

						if( false == pActuatorAccesory->IsAnAccessory() )
						{
							continue;
						}

						if( false == _IsSelectable( pActuatorAccesory ) )
						{
							continue;
						}

						SortListActAcces.AddStrSort( pActuatorAccesory->GetName(), 0, (LPARAM)(pActuatorAccesory), false );
					}							
									
					// Fill the accessory tree with the sorted ID's.
					CString str;
					LPARAM lparam;

					for( BOOL bContinue2 = SortListActAcces.GetFirst( str, lparam ); TRUE == bContinue2; bContinue2 = SortListActAcces.GetNext( str, lparam ) )
					{
						CDB_Product *pclActuatorAccesory = (CDB_Product *)lparam;

						if( NULL == pclActuatorAccesory || false == pclActuatorAccesory->IsAnAccessory() )
						{
							continue;
						}

						if( false == bTitleAccessoryAdded )
						{
							CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleAccessories );

							if( NULL == pclTreeItemTitle )
							{
								continue;
							}

							m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
							hItemCompAccTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLEACC ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hItemActuator, TVI_LAST );
							pclTreeItemTitle->m_hTreeItem = hItemCompAccTitle;
							bTitleAccessoryAdded = true;
						}

						CTreeItemAccessory *pclTreeItemAccessory = new CTreeItemAccessory( pclActuatorAccesory, CAccessoryList::AccessoryType::_AT_SetAccessory, pclRuledTable, pclTreeItemFamily, pclActuator );

						if( NULL == pclTreeItemAccessory )
						{
							continue;
						}

						int iImage = ( true == pclActuatorAccesory->IsAttached() && true == m_bWorkingForControlSet ) ? CheckboxNotSelectedDisable : CheckboxNotSelected;
						m_vecItemDataInTreeList.push_back( pclTreeItemAccessory );
						pclTreeItemAccessory->m_hTreeItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclActuatorAccesory->GetName(), iImage, iImage, 0, 0, (LPARAM)pclTreeItemAccessory, hItemCompAccTitle, TVI_LAST );
					}
				}
			}
		}
	}
}

void CDlgDirSel::FillTP_CompatibleBufferVessels( CTreeItemFamily *pclTreeItemFamily )
{
	HTREEITEM hCurrentItem = m_TreeProducts.GetRootItem();

	while( NULL != hCurrentItem )
	{
		HTREEITEM hItemBufferVesselTitle = NULL;
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hCurrentItem );

		if( CTreeItemData::DataType::Product != pclTreeItemData->m_eDataType )
		{
			// Jump to the next root member.
			hCurrentItem = m_TreeProducts.GetNextItem( hCurrentItem, TVGN_NEXT );
		}

		bool bTitleBufferVesselAdded = false;

		CTreeItemProduct *pclTreeItemTAProduct = (CTreeItemProduct *)pclTreeItemData;
		ASSERT( NULL != pclTreeItemTAProduct );

		if( NULL != pclTreeItemTAProduct->m_pclProduct && false == _IsSelectable( pclTreeItemTAProduct->m_pclProduct ) )
		{
			// Jump to the next root member.
			hCurrentItem = m_TreeProducts.GetNextItem( hCurrentItem, TVGN_NEXT );
		}

		CTable *pclTabBufferVesselList;
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pclTreeItemTAProduct->m_pclProduct );
		CDB_Product *pclProduct = NULL;

		if( NULL != pclTecBox )
		{
			// Get all buffer vessel.
			pclTabBufferVesselList = dynamic_cast<CTable *>( pclTecBox->GetIntegratedVesselIDPtr().MP );
		}

		if( NULL != pclTabBufferVesselList )
		{
			for( IDPTR IDPtr = pclTabBufferVesselList->GetFirst(); NULL != IDPtr.MP; IDPtr = pclTabBufferVesselList->GetNext() )
			{
				CDB_Vessel *pclBufVessel = dynamic_cast<CDB_Vessel *>(IDPtr.MP);

				if( false == bTitleBufferVesselAdded )
				{
					CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleBufferVessels );

					if( NULL == pclTreeItemTitle )
					{
						continue;
					}

					m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
					hItemBufferVesselTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, _T("Compatible buffer vessels"), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hCurrentItem, TVI_LAST );
					pclTreeItemTitle->m_hTreeItem = hItemBufferVesselTitle;
					bTitleBufferVesselAdded = true;
				}

				CTreeItemIntegrated *pclTreeItemBuffVessel = new CTreeItemIntegrated( pclBufVessel, pclTreeItemFamily );

				if( NULL == pclTreeItemBuffVessel )
				{
					continue;
				}

				int iImage = RadioNotSelected;
				m_vecItemDataInTreeList.push_back( pclTreeItemBuffVessel );
				HTREEITEM hItemIntegrated;
				hItemIntegrated = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclBufVessel->GetName(), iImage, iImage, 0, 0, (LPARAM)pclTreeItemBuffVessel, hItemBufferVesselTitle, TVI_LAST );
				pclTreeItemBuffVessel->m_hTreeItem = hItemIntegrated;

				if( false == pclBufVessel->IsSelectable( true ) )
				{
					m_TreeProducts.SetItemColor( hItemIntegrated, _RED );
				}

				FillTP_IntegratedAccessories( pclTreeItemFamily, hItemIntegrated );
			}
		}
		else if ( NULL != dynamic_cast<CDB_Vessel *>( pclTecBox->GetIntegratedVesselIDPtr().MP ) )
		{
			CDB_Vessel *pclBufVessel = dynamic_cast<CDB_Vessel *>( pclTecBox->GetIntegratedVesselIDPtr().MP );

			if( false == bTitleBufferVesselAdded )
			{
				CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleBufferVessels );

				if( NULL == pclTreeItemTitle )
				{
					continue;
				}

				m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
				hItemBufferVesselTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, _T("Compatible Buffer vessels"), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hCurrentItem, TVI_LAST );
				pclTreeItemTitle->m_hTreeItem = hItemBufferVesselTitle;
				bTitleBufferVesselAdded = true;
			}

			CTreeItemIntegrated *pclTreeItemBuffVessel = new CTreeItemIntegrated( pclBufVessel, pclTreeItemFamily );

			if( NULL == pclTreeItemBuffVessel )
			{
				continue;
			}

			int iImage = RadioNotSelected;
			m_vecItemDataInTreeList.push_back( pclTreeItemBuffVessel );
			HTREEITEM hItemIntegrated;
			hItemIntegrated = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclBufVessel->GetName(), iImage, iImage, 0, 0, (LPARAM)pclTreeItemBuffVessel, hItemBufferVesselTitle, TVI_LAST );
			pclTreeItemBuffVessel->m_hTreeItem = hItemIntegrated;

			if( false == pclBufVessel->IsSelectable( true ) )
			{
				m_TreeProducts.SetItemColor( hItemIntegrated, _RED );
			}

			FillTP_IntegratedAccessories( pclTreeItemFamily, hItemIntegrated );
		}

		// Jump to the next root member.
		hCurrentItem = m_TreeProducts.GetNextItem( hCurrentItem, TVGN_NEXT );
	}
}

void CDlgDirSel::FillTP_IntegratedAccessories( CTreeItemFamily *pclTreeItemFamily, HTREEITEM hItemParent )
{
	if( NULL == hItemParent )
	{
		return;
	}

	while( NULL != hItemParent )
	{
		HTREEITEM hItemCompAccTitle = NULL;
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hItemParent );

		if( CTreeItemData::DataType::Integrated == pclTreeItemData->m_eDataType )
		{
			bool bTitleAccessoryAdded = false;
			CTreeItemIntegrated *pclTreeItemIntegrated = (CTreeItemIntegrated *)pclTreeItemData;
			ASSERT( NULL != pclTreeItemIntegrated );

			if( NULL != pclTreeItemIntegrated->m_pclIntegrated && true == _IsSelectable( pclTreeItemIntegrated->m_pclIntegrated ) )
			{
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTreeItemIntegrated->m_pclIntegrated->GetAccessoriesGroupIDPtr().MP );

				if( NULL != pclRuledTable )
				{
					// Sort the list with a string number parameter.
					CRank SortList;
					SortList.PurgeAll();

					for( IDPTR idptr = pclRuledTable->GetFirst(); _T( '\0' ) != *idptr.ID; idptr = pclRuledTable->GetNext() )
					{
						CDB_Product *pAccessory = dynamic_cast<CDB_Product *>( idptr.MP );
						ASSERT( NULL != pAccessory );

						if( false == pAccessory->IsAnAccessory() )
						{
							continue;
						}

						if( false == _IsSelectable( pAccessory ) )
						{
							continue;
						}

						SortList.AddStrSort( pAccessory->GetName(), 0, (LPARAM)( pAccessory ), false );
					}

					// Fill the tree with the sorted ID's.
					CString str;
					LPARAM lparam;

					for( BOOL bContinue = SortList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lparam ) )
					{
						CDB_Product *pclAccessory = (CDB_Product *)lparam;

						if( NULL != pclAccessory && pclAccessory->IsAnAccessory() )
						{
							if( false == bTitleAccessoryAdded )
							{
								CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleAccessories );

								if( NULL == pclTreeItemTitle )
								{
									continue;
								}

								m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
								hItemCompAccTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLEACC ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hItemParent, TVI_LAST );
								pclTreeItemTitle->m_hTreeItem = hItemCompAccTitle;
								bTitleAccessoryAdded = true;
							}

							HTREEITEM hChildItem = NULL;

							CTreeItemAccessory *pclTreeItemAccessory = new CTreeItemAccessory( pclAccessory, CAccessoryList::AccessoryType::_AT_Accessory, pclRuledTable, pclTreeItemFamily, pclTreeItemIntegrated->m_pclIntegrated );

							if( NULL == pclTreeItemAccessory )
							{
								continue;
							}

							int iImage = CheckboxNotSelected;
							m_vecItemDataInTreeList.push_back( pclTreeItemAccessory );
							hChildItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclAccessory->GetName(), iImage, iImage, 0, 0, (LPARAM)pclTreeItemAccessory, hItemCompAccTitle, TVI_LAST );
							pclTreeItemAccessory->m_hTreeItem = hChildItem;

							if( false == pclAccessory->IsSelectable( true ) )
							{
								m_TreeProducts.SetItemColor( hChildItem, _RED );
							}
						}
					}
				}
			}
		}
		
		// Jump to the next root member.
		hItemParent = m_TreeProducts.GetNextItem( hItemParent, TVGN_NEXT );
	}
}

void CDlgDirSel::FillTP_CompatibleAdapters( CTreeItemFamily *pclTreeItemFamily )
{
	if( 0 == CString( pclTreeItemFamily->GetStringTypeID()->GetIDstr2() ).Compare( _T("SHUTOFF_TAB") ) )
	{
		// No adapter for shut-off valve for the moment.
		return;
	}

	int iImageNotSelected = ( true == m_bWorkingForControlSet ) ? CheckboxNotSelectedDisable : CheckboxNotSelected;
	HTREEITEM hCurrentItem = m_TreeProducts.GetRootItem();

	while( NULL != hCurrentItem )
	{
		HTREEITEM hItemAdapterTitle = NULL;
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hCurrentItem );

		if( CTreeItemData::DataType::Product == pclTreeItemData->m_eDataType )
		{
			CTreeItemProduct *pclTreeItemProduct = (CTreeItemProduct *)pclTreeItemData;
			ASSERT( NULL != pclTreeItemProduct );

			CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclTreeItemProduct->m_pclProduct );
			
			// Pay attention that now in a sub category we can have more than one product type.
			// "Advanced control valve" for example can contain "Adjustable control valve", "PIBCV-Std HVAC",
			// "PIBCV-High Dp" and "Smart control valve". But TA-Smart is not a "CDB_ControlValve" object and
			// don't have adapter.
			if( NULL != pclControlValve )
			{
				// Remark: if control valve is deleted, we don't know its reference (article number) and thus we CAN NOT FIND AND SHOW its actuators.
				if( NULL != pclControlValve && false == pclControlValve->IsDeleted() )
				{
					CDB_RuledTable *pclRuledTable = (CDB_RuledTable *)( pclControlValve->GetAdapterGroupIDPtr().MP );

					if( NULL != pclRuledTable )
					{
						CRank clAdapterList;
						TASApp.GetpTADB()->GetAccessories( &clAdapterList, pclRuledTable, CTADatabase::FilterSelection::ForDirSel );

						bool bTitleAdapterAdded = false;
						CString str;
						LPARAM lpParam;
						double dKey;

						for( BOOL bContinue = clAdapterList.GetFirst( str, lpParam, &dKey ); TRUE == bContinue; bContinue = clAdapterList.GetNext( str, lpParam, &dKey ) )
						{
							if( NULL == lpParam )
							{
								continue;
							}
						
							CDB_Product *pclAdapter = (CDB_Product *)lpParam;
							if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
							{
								continue;
							}

							if( false == bTitleAdapterAdded )
							{
								CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleAdapters );

								if( NULL == pclTreeItemTitle )
								{
									continue;
								}

								m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
								hItemAdapterTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLEADAPTERS ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hCurrentItem, TVI_LAST );
								pclTreeItemTitle->m_hTreeItem = hItemAdapterTitle;
								bTitleAdapterAdded = true;
							}

							CTreeItemAdapter *pclTreeItemAdapter = new CTreeItemAdapter( pclAdapter, pclTreeItemFamily );

							if( NULL == pclTreeItemAdapter )
							{
								continue;
							}

							m_vecItemDataInTreeList.push_back( pclTreeItemAdapter );
							HTREEITEM hChildItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclAdapter->GetName(), iImageNotSelected, iImageNotSelected, 0, 0, (LPARAM)pclTreeItemAdapter, hItemAdapterTitle, TVI_LAST );
							pclTreeItemAdapter->m_hTreeItem = hChildItem;

							if( false == pclAdapter->IsSelectable( true ) )
							{
								m_TreeProducts.SetItemColor( hChildItem, _RED );
							}
						}
					}
				}
			}
		}

		// Jump to the next root member.
		hCurrentItem = m_TreeProducts.GetNextItem( hCurrentItem, TVGN_NEXT );
	}
}

void CDlgDirSel::FillTP_CompatibleMeasuringValves( CTreeItemFamily *pclTreeItemFamily )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	HTREEITEM hCurrentItem = m_TreeProducts.GetRootItem();

	while( NULL != hCurrentItem )
	{
		HTREEITEM hItemMeasValveTitle = NULL;
		
		CString str;
		LPARAM lpParam;
		CRank List;
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hCurrentItem );

		if( CTreeItemData::DataType::Product == pclTreeItemData->m_eDataType )
		{
			bool bTitleMVAdded = false;

			CTreeItemProduct *pclTreeItemTAProduct = (CTreeItemProduct *)pclTreeItemData;
			ASSERT( NULL != pclTreeItemTAProduct );

			CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( pclTreeItemTAProduct->m_pclProduct );
			if( NULL == pDpC )
			{
				return;
			}
			
			// Remark: if differential pressure controller is deleted, we don't know its reference (article number) and thus we CAN NOT FIND AND SHOW its measuring valve.
			if( NULL != pDpC && false == pDpC->IsDeleted() )
			{
				m_clDirectSelectionParams.m_pTADB->GetSetDpCMeasValve( &List, pDpC );

				for( BOOL bContinue = List.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = List.GetNext( str, lpParam ) )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)lpParam );

					if( NULL != pTAP && true == _IsSelectable( pTAP ) )
					{
						if( false == bTitleMVAdded )
						{
							CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleMeasValve );

							if( NULL == pclTreeItemTitle )
							{
								continue;
							}

							m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
							hItemMeasValveTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLMEASVALV ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hCurrentItem, TVI_LAST );
							pclTreeItemTitle->m_hTreeItem = hItemMeasValveTitle;
							bTitleMVAdded = true;
						}

						CTreeItemProduct *pclTreeItemMv = new CTreeItemProduct( pTAP, pclTreeItemFamily );

						if( NULL == pclTreeItemMv )
						{
							continue;
						}

						m_vecItemDataInTreeList.push_back( pclTreeItemMv );

						HTREEITEM hItemMeasValv = NULL;
						hItemMeasValv = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pTAP->GetName(), 1, 1, 0, 0, (LPARAM)pclTreeItemMv, hItemMeasValveTitle, TVI_LAST );
						pclTreeItemMv->m_hTreeItem = hItemMeasValv;

						if( false == pTAP->IsSelectable( true ) )
						{
							m_TreeProducts.SetItemColor( hItemMeasValv, _RED );
						}

						// The 'FillTP_CompatibleMeasuringValves' method is called only in the case of a set selection of Dp controller and balancing valve.
						// So compatible accessories are here only for the set.
						FillTP_CompatibleAccessories( pclTreeItemFamily, hItemMeasValv, true );
					}
				}
			}
		}
		
		// Jump to the next root member.
		hCurrentItem = m_TreeProducts.GetNextItem( hCurrentItem, TVGN_NEXT );
	}
}

void CDlgDirSel::FillTP_CompatibleSmartDpCSets( CTreeItemFamily *pclTreeItemFamily )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	HTREEITEM hCurrentItem = m_TreeProducts.GetRootItem();

	while( NULL != hCurrentItem )
	{
		HTREEITEM hItemDpSensorTitle = NULL;

		CString str;
		LPARAM lpParam;
		CRank List;
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hCurrentItem );

		if( CTreeItemData::DataType::Product == pclTreeItemData->m_eDataType )
		{
			bool bTitleSetsAdded = false;

			CTreeItemProduct *pclTreeItemTAProduct = (CTreeItemProduct *)pclTreeItemData;
			ASSERT( NULL != pclTreeItemTAProduct );

			CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pclTreeItemTAProduct->m_pclProduct );

			if( NULL != pclSmartDpC && ( 0 != StringCompare( _T("SMARTDPCTYPE"), pclTreeItemTAProduct->m_pclProduct->GetTypeID() ) ) )
			{
				return;
			}
			
			// Remark: if smart differential pressure controller is deleted, we don't know its reference (article number) 
			// and thus we CAN NOT FIND AND SHOW its Dp sensor.
			if( NULL != pclSmartDpC && false == pclSmartDpC->IsDeleted() )
			{
				// If a set we have to add the sets in the list.
				if( true == pclSmartDpC->IsPartOfaSet() )
				{
					CTableSet *pTableSet = pclSmartDpC->GetTableSet();
					std::set<CDB_Set *> SmartDpCDpSensorSet;

					int iSetCount = 0;

					if( NULL != pTableSet )
					{
						iSetCount = pTableSet->FindCompatibleSet( &SmartDpCDpSensorSet, pclSmartDpC->GetIDPtr().ID, _T("") );
					}

					// Add sets linked to the current smart differential pressure controller.
					if( iSetCount > 0 )
					{
						for( std::set<CDB_Set *>::iterator iter = SmartDpCDpSensorSet.begin(); iter != SmartDpCDpSensorSet.end(); ++iter )
						{
							CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>( (*iter)->GetSecondIDPtr().MP );

							double dKey = (double)pclProductSet->GetSortingKey();

							if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
							{
								CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );

								ASSERT( ( pclDpSensor->GetMinMeasurableDp() / 1000.0 ) < 100.0 );
								dKey = dKey * 100.0 + ( pclDpSensor->GetMinMeasurableDp() / 1000.0 );
							}
							else
							{
								dKey += 1e9;
							}

							List.Add( pclProductSet->GetName(), dKey, (LPARAM)( (*iter)->GetSecondIDPtr().MP) );
						}
					}
				}
				else
				{
					// HYS-1992: for fictif set.
					CTable *pTableSet = (CTable*)pclSmartDpC->GetDpSensorGroupIDPtr().MP;

					int iSetCount = pTableSet->GetItemCount();

					// Add sets linked to the current smart differential pressure controller.
					if( iSetCount > 0 )
					{
						for( IDPTR idptr = pTableSet->GetFirst(); NULL != idptr.MP; idptr = pTableSet->GetNext() )
						{
							CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>(idptr.MP );

							double dKey = (double)pclProductSet->GetSortingKey();

							if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
							{
								CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );

								ASSERT( ( pclDpSensor->GetMinMeasurableDp() / 1000.0 ) < 100.0 );
								dKey = dKey * 100.0 + ( pclDpSensor->GetMinMeasurableDp() / 1000.0 );
							}
							else
							{
								dKey += 1e9;
							}

							List.Add( pclProductSet->GetName(), dKey, (LPARAM)(idptr.MP) );
						}
					}
				}
				CDB_RuledTable *pclRuledTable = (CDB_RuledTable*)pclSmartDpC->GetDpSensorGroupIDPtr().MP;
				for( BOOL bContinue = List.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = List.GetNext( str, lpParam ) )
				{
					CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>( (CData *)lpParam );

					if( NULL != pclProductSet && true == _IsSelectable( pclProductSet ) )
					{
						if( false == bTitleSetsAdded )
						{
							CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleSmartDpCSets );

							if( NULL == pclTreeItemTitle )
							{
								continue;
							}

							m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
							hItemDpSensorTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLESMARTDPCSET ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hCurrentItem, TVI_LAST );
							pclTreeItemTitle->m_hTreeItem = hItemDpSensorTitle;
							bTitleSetsAdded = true;
						}

						// HYS-1992: ItemAccessory because it is an accessory. The product is managed differently.
						CTreeItemAccessory* pclTreeItemSmartDpCSet = new CTreeItemAccessory(pclProductSet, CAccessoryList::AccessoryType::_AT_Accessory, 
																							pclRuledTable, pclTreeItemFamily, pclSmartDpC );

						if( NULL == pclTreeItemSmartDpCSet )
						{
							continue;
						}

						m_vecItemDataInTreeList.push_back( pclTreeItemSmartDpCSet );

						CString strName = pclProductSet->GetName();

						if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
						{
							CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );
							strName = pclDpSensor->GetFullName();
						}

						HTREEITEM hItemSmartDpCSet = NULL;
						hItemSmartDpCSet = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)strName, 1, 1, 0, 0, (LPARAM)pclTreeItemSmartDpCSet, hItemDpSensorTitle, TVI_LAST );
						pclTreeItemSmartDpCSet->m_hTreeItem = hItemSmartDpCSet;

						if( false == pclProductSet->IsSelectable( true ) )
						{
							m_TreeProducts.SetItemColor( hItemSmartDpCSet, _RED );
						}

					}
				}
			}
		}

		// Jump to the next root member.
		hCurrentItem = m_TreeProducts.GetNextItem( hCurrentItem, TVGN_NEXT );
	}
}


void CDlgDirSel::FillTP_CompatibleAccessories( CTreeItemFamily *pclTreeItemFamily, HTREEITEM hMainItem, bool bIsForSet )
{
	// Fill sub tree Accessories.
	if( NULL == hMainItem )
	{
		hMainItem = m_TreeProducts.GetRootItem();
	}
	
	while( NULL != hMainItem )
	{
		HTREEITEM hItemCompAccTitle = NULL;
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hMainItem );

		if( CTreeItemData::DataType::Product == pclTreeItemData->m_eDataType )
		{
			bool bTitleAccAdded = false;

			CTreeItemProduct *pclTreeItemTAProduct = (CTreeItemProduct *)pclTreeItemData;
			ASSERT( NULL != pclTreeItemTAProduct );

			// Remark: if product is deleted, we don't know its reference (article number) and thus we CAN NOT FIND AND SHOW its accessories.
			if( NULL != pclTreeItemTAProduct->m_pclProduct && false == pclTreeItemTAProduct->m_pclProduct->IsDeleted() )
			{
				CDB_RuledTable *pclRuledTable = NULL;

				if( NULL != dynamic_cast<CDB_SmartControlValve *>( pclTreeItemTAProduct->m_pclProduct ) )
				{
					// For smart differential pressure controler, the accessories in the set are displayed under the Dp sensor set.
					// In this case, 'm_WorkingForDpCSet' is 'true' but we want to show accessories belonging to the smart differential
					// pressure controller that is not in a set.
					pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTreeItemTAProduct->m_pclProduct->GetAccessoriesGroupIDPtr().MP );
				}
				else if( false == m_bWorkingForControlSet && false == m_bWorkingForDpCSet )
				{
					pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTreeItemTAProduct->m_pclProduct->GetAccessoriesGroupIDPtr().MP );
				}
				else
				{
					// Take accesories set.
					CTableSet *pclTableSet = pclTreeItemTAProduct->m_pclProduct->GetTableSet();

					if( NULL != pclTableSet )
					{
						CDB_Set *pclProductSet = pclTableSet->FindCompatibleSet( pclTreeItemTAProduct->m_pclProduct->GetIDPtr().ID, _T("") );

						if( NULL != pclProductSet )
						{
							pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclProductSet->GetAccGroupIDPtr().MP );
						}
					}
				}

				if( NULL != pclRuledTable )
				{
					// Sort the list with a string number parameter.
					CRank SortList;
					SortList.PurgeAll();

					// HYS-1617/HYS_1618: We call this method to activate the crossing table option in the 'CDB_RuledTable' table.
					TASApp.GetpTADB()->GetAccessories( &SortList, pclRuledTable, CTADatabase::ForDirSel );

					// Fill the tree with the sorted ID's.
					CString str;
					LPARAM lparam;

					for( BOOL bContinue = SortList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lparam ) )
					{
						CDB_Product *pclAccessory = (CDB_Product *)lparam;

						if( NULL != pclAccessory && pclAccessory->IsAnAccessory() )
						{
							if( false == bTitleAccAdded )
							{
								CTreeItemTitle *pclTreeItemTitle = new CTreeItemTitle( CTreeItemTitle::TitleType::CompatibleAccessories );

								if( NULL == pclTreeItemTitle )
								{
									continue;
								}

								m_vecItemDataInTreeList.push_back( pclTreeItemTitle );
								hItemCompAccTitle = m_TreeProducts.InsertItem( TVIF_TEXT | TVIF_PARAM, TASApp.LoadLocalizedString( IDS_DLGDIRSEL_COMPATIBLEACC ), 0, 0, 0, 0, (LPARAM)pclTreeItemTitle, hMainItem, TVI_LAST );
								pclTreeItemTitle->m_hTreeItem = hItemCompAccTitle;
								bTitleAccAdded = true;
							}

							HTREEITEM hChildItem = NULL;

							CAccessoryList::AccessoryType eAccessoryType = CAccessoryList::AccessoryType::_AT_Accessory;

							if( true == bIsForSet )
							{
								eAccessoryType = CAccessoryList::AccessoryType::_AT_SetAccessory;
							}

							CTreeItemAccessory *pclTreeItemAccessory = new CTreeItemAccessory( pclAccessory, eAccessoryType, pclRuledTable, pclTreeItemFamily, pclTreeItemTAProduct->m_pclProduct );

							if( NULL == pclTreeItemAccessory )
							{
								continue;
							}
							
							int iImage = ( true == pclAccessory->IsAttached() && ( true == m_bWorkingForDpCSet || true == m_bWorkingForControlSet ) ) ? CheckboxNotSelectedDisable : CheckboxNotSelected;

							if( CheckboxNotSelected == iImage  )
							{
								// We also verify accessory that are built-in.
								if( true == pclRuledTable->IsBuiltIn( pclAccessory ) )
								{
									iImage = CheckboxNotSelectedDisable;
								}
							}

							m_vecItemDataInTreeList.push_back( pclTreeItemAccessory );
							hChildItem = m_TreeProducts.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pclAccessory->GetName(), iImage, iImage, 0, 0, (LPARAM)pclTreeItemAccessory, hItemCompAccTitle, TVI_LAST );
							pclTreeItemAccessory->m_hTreeItem = hChildItem;

							if( false == pclAccessory->IsSelectable( true ) )
							{
								m_TreeProducts.SetItemColor( hChildItem, _RED );
							}
						}
					}
				}
			}
		}

		// Jump to the next root member.
		hMainItem = m_TreeProducts.GetNextItem( hMainItem, TVGN_NEXT );
	}
}

void CDlgDirSel::FillProductDetails( CDB_Product *pclProduct, CExtListCtrl *pExtListCtrl )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclProduct || NULL == pExtListCtrl )
	{
		return;
	}
	
	// Variables.
	CString str, strName, test; 
	int iNbrRow = pExtListCtrl->GetItemCount();
	int iLeftPos = 0;
	int iRightPos = 0;
	bool bDisplayName = true;
	
	// Don't show some info for the focus list.
	if( pExtListCtrl != &m_ListBoxFocus )
	{
		// If it's a set, add its set article number on the head of the CListCtrl...
		if( true == m_bWorkingForControlSet )
		{
			CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclProduct );
			CDB_6WayValve *pcl6WayControlValve = dynamic_cast<CDB_6WayValve *>( pclProduct );
			CTableSet *pCVActSetTab = pclControlValve->GetTableSet();

			// HYS-1877: We don't have m_ItemActuatorSelected for 6-way valve set because only the 6-way valve product
			// is selected in product column.
			if( NULL != m_ItemActuatorSelected.m_hTreeItem || NULL != pCVActSetTab )
			{
				DWORD_PTR dwptrAct;
				CTreeItemActuator *pclTreeItemActuator = NULL;

				if( NULL != m_ItemActuatorSelected.m_hTreeItem )
				{
					dwptrAct = m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem );
					pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)dwptrAct );
				}

				if( NULL == pclTreeItemActuator && NULL != pCVActSetTab && ( 0 != StringCompare( pCVActSetTab->GetIDPtr().ID, _T("6WAYCTRLVALVSET_TAB") ) ) )
				{
					ASSERT_RETURN;
				}

				CString strActID;

				if( NULL != pclTreeItemActuator )
				{
					strActID = pclTreeItemActuator->m_pclActuator->GetIDPtr().ID;
				}

				if( NULL != pCVActSetTab )
				{
					// HYS-1877: Display Set information
					CDB_PIControlValve *pclPIBCVFor6WayValveSet = NULL;
					if( 0 == StringCompare( pCVActSetTab->GetIDPtr().ID, _T("6WAYCTRLVALVSET_TAB") ) )
					{
						strActID = _T("");
						// Exclude PIBCV set for 6-way control valve. In this case we don't display set information
						pclPIBCVFor6WayValveSet = dynamic_cast<CDB_PIControlValve*>(pclProduct);
					}

					CDB_Set *pCVActSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, strActID );

					if( NULL != pCVActSet && NULL == pclPIBCVFor6WayValveSet )
					{
						// Check if we must display reference in red.
						bool bNotAvailOrDeleted = ( false == pCVActSet->IsAvailable() || true == pCVActSet->IsDeleted( ) );
						COLORREF cTextColor = ( false == bNotAvailOrDeleted ) ? _BLACK : _RED;

						// Check first if we have a local article number available.
						CString strLocalArticleNumber = _T("");
						
						if( true == TASApp.IsLocalArtNumberUsed() )
						{
							strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pCVActSet->GetReference() );
							strLocalArticleNumber.Trim();
						}

						// Set the reference.
						bool bArticleNumberShown = false;
						
						// If we can show article number OR we can't show but there is no local article number...
						if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
						{
							CString str = pCVActSet->GetReference();
							
							if( false == str.IsEmpty() )
							{
								pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
								pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
								pExtListCtrl->AddItemTextVerify( str, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
								bArticleNumberShown = true;
							}
						}

						// Set local article number if allowed and exist.
						if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
						{
							strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pCVActSet->GetReference() );
							strLocalArticleNumber.Trim();
							pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, ( true == bArticleNumberShown ) ? false : true );
							pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
							pExtListCtrl->AddItemTextVerify( strLocalArticleNumber, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
						}

						if( true == bNotAvailOrDeleted )
						{
							// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
							CString strFlag = TASApp.LoadLocalizedString( ( true == pCVActSet->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
							pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
							pExtListCtrl->AddItemTextVerify( strFlag, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
						}

						// Set the name.
						str = pCVActSet->GetName();
						pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
						pExtListCtrl->AddItemTextVerify( TASApp.LoadLocalizedString( IDS_DLGDIRSET_SET ), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
					}
				}
			}
		}
		
		if( true == m_bWorkingForDpCSet )
		{
			CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( pclProduct );
			CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( pclProduct );

			if( NULL != pDpC && NULL != m_ItemProductSelected.m_hTreeItemSub )
			{
				DWORD_PTR dwptrMv = m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItemSub );
				CTreeItemProduct *pclTreeItemMv = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)dwptrMv );
				
				if( NULL == pclTreeItemMv )
				{
					ASSERT_RETURN;
				}
				
				// Establish a list of available set for this differential pressure controller.
				CTableSet *pSetTab = (CTableSet *)( m_clDirectSelectionParams.m_pTADB->Get( _T("DPCMVSET_TAB") ).MP );
				
				if( NULL == pSetTab )
				{
					ASSERT_RETURN;
				}

				CDB_Set *pDpCSet =  pSetTab->FindCompatibleSet( pDpC->GetIDPtr().ID, pclTreeItemMv->m_pclProduct->GetIDPtr().ID ); 
				
				if( NULL != pDpCSet )
				{
					// Check if we must display reference in red.
					bool bNotAvailOrDeleted = ( false == pDpCSet->IsAvailable() || true == pDpCSet->IsDeleted( ) );
					COLORREF cTextColor =  ( false == bNotAvailOrDeleted ) ? _BLACK : _RED;

					// Check first if we have a local article number available.
					CString strLocalArticleNumber = _T("");
					
					if( true == TASApp.IsLocalArtNumberUsed() )
					{
						strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pDpCSet->GetReference() );
						strLocalArticleNumber.Trim();
					}

					// Set the reference.
					bool bArticleNumberShown = false;
					
					if( false == TASApp.IsTAArtNumberHidden()  || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
					{
						CString str = pDpCSet->GetReference();
						
						if( false == str.IsEmpty() )
						{
							pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
							pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
							pExtListCtrl->AddItemTextVerify( str, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
							bArticleNumberShown = true;
						}
					}

					// Set local article number if allowed and exist.
					if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
					{
						strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pDpCSet->GetReference() );
						strLocalArticleNumber.Trim();
						pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, ( true == bArticleNumberShown ) ? false : true );
						pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
						pExtListCtrl->AddItemTextVerify( strLocalArticleNumber, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
					}
					
					if( true == bNotAvailOrDeleted )
					{
						// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
						CString strFlag = TASApp.LoadLocalizedString( ( true == pDpCSet->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
						pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
						pExtListCtrl->AddItemTextVerify( strFlag, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
					}

					// Set the name.
					str = pDpCSet->GetName();
					pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
					pExtListCtrl->AddItemTextVerify( TASApp.LoadLocalizedString( IDS_DLGDIRSET_SET ), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
				}
			}
			else if( NULL == pclRegulatingValve )
			{
				bDisplayName = false;
			}
		}
	}

	DLGDIRSEL_AJUSTCOORDS( iLeftPos, iRightPos );

	// Insert a blank line if we have DpC-Mv or Cv-Actuator set.
	if( iLeftPos > 0 )
	{
		pExtListCtrl->InsertItem( LVIF_TEXT | LVIF_STATE, iNbrRow + iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
		iLeftPos = ++iRightPos;
	}

	// Check if we must display reference in red.
	bool bNotAvailOrDeleted = ( false == pclProduct->IsAvailable() || true == pclProduct->IsDeleted( ) );
	COLORREF cTextColor =  ( false == bNotAvailOrDeleted ) ? _BLACK : _RED;
	CDB_Vessel *pclIntegratedVessel = NULL;

	if( true == bDisplayName )
	{
		if( false == m_bWorkingForDpCSet && false == m_bWorkingForControlSet )
		{
			// Check first if we have a local article number available.
			CString strLocalArticleNumber = _T("");
			CString strConnectionAN = CteEMPTY_STRING;
		
			if( true == TASApp.IsLocalArtNumberUsed() )
			{
				// Why calling 'CDB_Product::GetBodyArtNum' instead of 'CDB_TAProduct::GetArtNum'? Because 'GetLocalArticleNumber' needs only body article
				// number without any connection (inlet/outlet).
				strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclProduct->GetBodyArtNum() );
				strLocalArticleNumber.Trim();
			}

			CString strArticleNumber = _T("");
		
			// If we can show article number OR we can't show but there is no local article number...
			if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
			{
				strArticleNumber = pclProduct->GetArtNum( bNotAvailOrDeleted );
			}

			// Check for the local article number, and if it exist, change the name.
			if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
			{
				strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclProduct->GetBodyArtNum() );
				strLocalArticleNumber.Trim();

				if( false == strArticleNumber.IsEmpty() )
				{
					strArticleNumber += (CString)_T("&") + strLocalArticleNumber;
				}
				else
				{
					strArticleNumber = strLocalArticleNumber;
					strConnectionAN = pclProduct->GetArtNum( bNotAvailOrDeleted );
					// Look if we have connection
					int index = 0;
					index = strConnectionAN.Find( _T("&") );
				
					if( index > 0 )
					{
						strConnectionAN.Delete( 0, index + 1 );
						if( false == strConnectionAN.IsEmpty() )
						{
							strArticleNumber += _T("&") + strConnectionAN;
						}
					}
				}
			}

			bool bOnePrint = false;
			int iPos = 0;

			do
			{
				iPos = strArticleNumber.Find( _T("&") );
				CString str2 = strArticleNumber;

				if( iPos >= 0 )
				{
					str2 = strArticleNumber.Left( iPos );
					strArticleNumber.Delete( 0, iPos + 1 );
				}

				pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, ( false == bOnePrint ) ? true : false );
				pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
				pExtListCtrl->AddItemTextVerify( str2, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
				bOnePrint = true;

			}while( iPos >= 0 );

			// If something has been printed in left column and product is not available or deleted...
			if( true == bOnePrint && true == bNotAvailOrDeleted )
			{
				// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
				CString strFlag = TASApp.LoadLocalizedString( ( true == pclProduct->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
				pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
				pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
				pExtListCtrl->AddItemTextVerify( strFlag, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
			}
		}
		else
		{
			pExtListCtrl->AddItemTextVerify( _T("-"), iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
		}

		// HYS-2059: Do not show simple name for DpSensor add DpL range with the name by using "GetFullName".
		if( NULL != dynamic_cast<CDB_DpSensor*>(pclProduct) )
		{
			str = ((CDB_DpSensor*)pclProduct)->GetFullName();
		}
		else
		{
			str = pclProduct->GetName();
		}

		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	}

	// Try to see if it's a thermostatic valve. If it's the case, add description if defined.
	CDB_ThermostaticValve *pclThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( pclProduct );

	if( NULL != pclThermostaticValve )
	{
		str = pclThermostaticValve->GetDescription();

		if( false == str.IsEmpty() )
		{
			// HYS-266: Treatment to display very long description.
			int nField = 0;
			int index = 0;
			CStringArray p_outArray;
			ParseString( str, _T("\n"), &nField, &p_outArray );
		
			while( index != nField )
			{
				pExtListCtrl->AddItemTextVerify( p_outArray.GetAt( index ), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
				iNbrRow++;
				index++;
			}
		}
	}

	// HYS-1038: Try to see if it's a FlowLimitedControlValve. If it's the case, add description if defined.
	CDB_FlowLimitedControlValve *pclFLCValve = dynamic_cast<CDB_FlowLimitedControlValve *>( pclProduct );

	if( NULL != pclFLCValve )
	{
		str = pclFLCValve->GetDescription();

		if( false == str.IsEmpty() )
		{
			// HYS-266: Treatment to display very long description.
			int nField = 0;
			int index = 0;
			CStringArray p_outArray;
			ParseString( str, _T("\n"), &nField, &p_outArray );
			
			while( index != nField )
			{
				pExtListCtrl->AddItemTextVerify( p_outArray.GetAt( index ), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
				iNbrRow++;
				index++;
			}
		}
	}

	if( CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclProduct ) )
	{
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

		if( true == pclProduct->IsClass( CLASS( CDB_ControlValve ) ) )
		{
			if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_KVS );
				str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CV );
				str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
			}

			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}

		// Set the rangeability.
		double dRangeability = pclControlValve->GetRangeability();

		if( dRangeability > 0.0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_RANGEABILITY );
			str += CString( _T(" : ") ) + pclControlValve->GetStrRangeability().c_str();
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}

		// Set the leakage rate.
		if( pclControlValve->GetLeakageRate() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_CV_LEAKAGE );
			str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetLeakageRate() * 100 );
			str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_DLGDIRSEL_CV_LEAKAGEUNIT );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
	}

	if( NULL != dynamic_cast<CDB_AirVent *>( pclProduct ) || NULL != dynamic_cast<CDB_Separator *>( pclProduct ) )
	{
		if( NULL != dynamic_cast<CDB_AirVent *>( pclProduct ) )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_DPP );
			str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dynamic_cast<CDB_AirVent *>( pclProduct )->GetDpp(), true );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_FAMILY );
		str += CString( _T(" : ") ) + pclProduct->GetFamily();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	}

	bool bIntegratedVessel = false;
	bool bShowTemperatureRange = true;

	if( NULL != dynamic_cast<CDB_SafetyValveBase *>( pclProduct ) )
	{
		CDB_SafetyValveBase *pclSafetyValve = (CDB_SafetyValveBase *)pclProduct;

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_BDYMAT );
		str += CString( _T(" : ") ) + pclSafetyValve->GetBodyMaterial();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CONNECT );
		str += CString( _T(" : ") ) + pclSafetyValve->GetConnect();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_VERSION );
		str += CString( _T(" : ") ) + pclSafetyValve->GetVersion();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_SAFETYVALVE_LIFTINGMECHANISM );
		str += CString( _T(" : ") ) + pclSafetyValve->GetLiftingType();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_SAFETYVALVE_SETPRESSURE );
		str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclSafetyValve->GetSetPressure(), true );
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	}
	else if( NULL != dynamic_cast<CDB_BlowTank *>( pclProduct ) )
	{
		CDB_BlowTank *pclBlowTank = (CDB_BlowTank *)pclProduct;

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_PS );
		str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, pclBlowTank->GetPmaxmax() );
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_BDYMAT );
		str += CString( _T(" : ") ) + pclBlowTank->GetBodyMaterial();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_SIZE );
		str += CString( _T(" : ") ) + pclBlowTank->GetAllSizesString();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	}
	else if( NULL != dynamic_cast<CDB_TapWaterControl *>( pclProduct ) )
	{
		CDB_TapWaterControl *pclTapWaterControl = (CDB_TapWaterControl *)pclProduct;

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_SETTINGRANGE );
		str += CString( _T(" : ") ) + pclTapWaterControl->GetTempAdjustableRangeStr( true );
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_BDYMAT );
		str += CString( _T(" : ") ) + pclTapWaterControl->GetBodyMaterial();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CONNECT );
		str += CString( _T(" : ") ) + pclTapWaterControl->GetConnect();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		if( false == CString( pclTapWaterControl->GetVersion() ).IsEmpty() )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_VERSION );
			str += CString( _T(" : ") ) + pclTapWaterControl->GetVersion();
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}

		bShowTemperatureRange = false;
	}
	else if( NULL != dynamic_cast<CDB_FloorHeatingManifold *>( pclProduct ) )
	{
		CDB_FloorHeatingManifold *pclFloorHeatingManifold = (CDB_FloorHeatingManifold *)pclProduct;

		str.Format( _T("%s : %i"), TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_HEATINGCIRCUITS ), pclFloorHeatingManifold->GetHeatingCircuits() );
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	}
	else if( NULL != dynamic_cast<CDB_FloorHeatingValve *>( pclProduct ) )
	{
		CDB_FloorHeatingValve *pclFloorHeatingValve = (CDB_FloorHeatingValve *)pclProduct;

		// Comment if exist.
		if( 0 != _tcslen( pclFloorHeatingValve->GetComment() ) )
		{
			pExtListCtrl->AddItemTextVerify( pclFloorHeatingValve->GetComment(), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}

		// Connection inlet.
		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CONNECTIN );
		str += CString( _T(" : ") ) + pclFloorHeatingValve->GetConnectInlet();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		// Connection outlet.
		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CONNECTOUT );
		str += CString( _T(" : ") ) + pclFloorHeatingValve->GetConnectOutlet();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		// With or without handheld (Only for supply pipe control valve).
		if( 0 == pclFloorHeatingValve->GetSupplyOrLockshield() )
		{
			pExtListCtrl->AddItemTextVerify( pclFloorHeatingValve->GetHasHandwheelString(), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
	}
	else if( NULL != dynamic_cast<CDB_FloorHeatingController *>( pclProduct ) )
	{
		CDB_FloorHeatingController *pclFloorHeatingController = (CDB_FloorHeatingController *)pclProduct;

		if( 0 != _tcslen( pclFloorHeatingController->GetComment() ) )
		{
			pExtListCtrl->AddItemTextVerify( pclFloorHeatingController->GetComment(), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
	}
	else if( NULL != dynamic_cast<CDB_SmartControlValve *>( pclProduct ) )
	{
		CDB_SmartControlValve *pclSmartValve = (CDB_SmartControlValve *)pclProduct;
		ASSERT( NULL != pclSmartValve->GetSmartValveCharacteristic() );

		// Set the body material.
		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_BDYMAT );
		str += CString( _T(" : ") ) + pclSmartValve->GetBodyMaterial();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		// Set the size.
		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CONNECT );
		str += CString( _T(" : ") ) + pclSmartValve->GetConnect();

		if( 0 == IDcmp( pclSmartValve->GetConnTabID(), _T("COMPONENT_TAB") ) )
		{
			str += CString( _T(" -> ") ) + pclSmartValve->GetSize();
		}

		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		// Set the PN.
		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_PN );
		str += CString( _T(" ") ) + pclSmartValve->GetPN().c_str();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		// Set the Kvs value.
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

		if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_KVS ) + CString( _T(" : ") );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CV ) + CString( _T(" : ") );
		}

		if( NULL != pclSmartValve->GetSmartValveCharacteristic() )
		{
			if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str += WriteCUDouble( _C_KVCVCOEFF, pclSmartValve->GetSmartValveCharacteristic()->GetKvs() );
			}
			else
			{
				str += WriteCUDouble( _C_KVCVCOEFF, pclSmartValve->GetSmartValveCharacteristic()->GetKvs() );
			}
		}
		else
		{
			str += GetDashDotDash();
		}
		
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		// Set the nominal flow.
		str = _T("Qnom : ");

		if( NULL != pclSmartValve->GetSmartValveCharacteristic() )
		{
			str += WriteCUDouble( _U_FLOW, pclSmartValve->GetSmartValveCharacteristic()->GetQnom(), true );
		}
		else
		{
			str += GetDashDotDash();
		}

		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		// HYS-1660: These information are not shown for listBoxFocus
		if( pExtListCtrl != &m_ListBoxFocus )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_POWSUP );
			str += CString( _T(" : ") ) + pclSmartValve->GetPowerSupplyStr();
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			if( SmartValveType::SmartValveTypeControl == pclSmartValve->GetSmartValveType() )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_INPUTSIG );
				str += CString( _T(" : ") ) + pclSmartValve->GetInOutSignalsStr( true );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			}

			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_OUTPUTSIG );

			if( CString( _T("") ) == pclSmartValve->GetInOutSignalsStr( false ) )
			{
				str += CString( _T(" :    -") );
			}
			else
			{
				str += CString( _T(" : ") ) + pclSmartValve->GetInOutSignalsStr( false );
			}
			
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
	}
	else if( NULL != dynamic_cast<CDB_DpSensor *>( pclProduct ) )
	{
	    // DpSensor alone
		CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)pclProduct;

		if( false == pclDpSensor->IsPartOfaSet() )
		{
			if( 0 != _tcslen( pclProduct->GetComment() ) )
			{
				str = _T("(") + CString( pclProduct->GetComment() ) + _T(")");
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			}

			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_DPC_DPLRANGE );
			str += CString( _T(" : ") ) + pclDpSensor->GetFormatedDplRange( true ).c_str();
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			// HYS-2007
			CString strBurstPressure = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true );
			FormatString( str, IDS_TALINK_BURSTPRESSURE, strBurstPressure );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_OUTPUTSIG );
			if( CString( _T( "" ) ) == pclDpSensor->GetOutputSignalsStr() )
			{
				str += CString( _T( " :    -" ) );
			}
			else
			{
				str += CString( _T( " : " ) ) + pclDpSensor->GetOutputSignalsStr();
			}

			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
		else
		{
			bShowTemperatureRange = false;
		}
	}
	else if( NULL != dynamic_cast<CDB_TAProduct *>( pclProduct ) )
	{
		if( 0 != _tcslen( pclProduct->GetComment() ) )
		{
			str = _T("(") + CString( pclProduct->GetComment() ) + _T(")");
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
		
		CDB_TAProduct *pclTAProduct = ( CDB_TAProduct *)pclProduct;
		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_BDYMAT );
		str += CString( _T(" : ") ) + pclTAProduct->GetBodyMaterial();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_CONNECT );
		str += CString( _T(" : ") ) + pclTAProduct->GetConnect();

		if( 0 == IDcmp( pclTAProduct->GetConnTabID(), _T("COMPONENT_TAB") ) )
		{
			str += CString( _T(" -> ") ) + pclTAProduct->GetSize();
		}

		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_VERSION );
		str += CString( _T(" : ") ) + pclTAProduct->GetVersion();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_PN );
		str += CString( _T(" ") ) + pclProduct->GetPN().c_str();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		if( CDB_DpController *pDpController = dynamic_cast<CDB_DpController *>( pclProduct ) )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_DPC_DPLRANGE );
			str += CString( _T(" : ") ) + pDpController->GetFormatedDplRange( true ).c_str();
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
		else if( CDB_DpReliefValve *pDpReliefValve = dynamic_cast<CDB_DpReliefValve *>( pclProduct ) )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_PRODUCT_SETTINGRANGE );
			str += CString( _T(" : ") ) + pDpReliefValve->GetAdjustableRange( true );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
	}
	else
	{
		CDB_Vessel *pVssl = dynamic_cast<CDB_Vessel *>( pclProduct );
		CDB_TecBox *pTecBox = dynamic_cast<CDB_TecBox *>( pclProduct );

		if( NULL != pVssl )
		{
			CString strPmax = WriteCUDouble( _U_PRESSURE, pclProduct->GetPmaxmax(), true );
			FormatString( str, IDS_DLGDIRSEL_MAXPRESS, strPmax );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			CString strVol = WriteCUDouble( _U_VOLUME, pVssl->GetNominalVolume(), true );
			FormatString( str, IDS_DLGDIRSEL_NOMINALVOLUME, strVol );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			if( pVssl->GetFactoryP0() > 0 )
			{
				CString strFactoryP0 = WriteCUDouble( _U_PRESSURE, pVssl->GetFactoryP0(), true );
				FormatString( str, IDS_DLGDIRSEL_FACTORYP0, strFactoryP0 );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			}

			CString strHeight = WriteCUDouble( _U_LENGTH, pVssl->GetHeight(), true );
			FormatString( str, IDS_DLGDIRSEL_HEIGHT, strHeight );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			CString strDiameter = WriteCUDouble( _U_LENGTH, pVssl->GetDiameter(), true );
			FormatString( str, IDS_DLGDIRSEL_DIAMETER, strDiameter );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			CString strWeight = WriteCUDouble( _U_MASS, pVssl->GetWeight(), true );
			FormatString( str, IDS_DLGDIRSEL_WEIGHT, strWeight );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
		else if( NULL != pTecBox )
		{
			CString strTmp = WriteCUDouble( _U_PRESSURE, pTecBox->GetPmaxmax(), true );
			FormatString( str, IDS_DLGDIRSEL_TECHBOXPS, strTmp );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			strTmp = pTecBox->GetPowerSupplyStr();
			FormatString( str, IDS_DLGDIRSEL_TECHBOXSUPPLYVOLTAGE, strTmp );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			strTmp = WriteCUDouble( _U_ELEC_POWER, pTecBox->GetPower(), true );
			FormatString( str, IDS_DLGDIRSEL_TECHBOXPOWER, strTmp );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			FormatString( str, IDS_DLGDIRSEL_TBIPXX, pTecBox->GetIPxx() );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			if( pTecBox->GetSoundPressureLevel() > 0 )
			{
				strTmp = WriteDouble( pTecBox->GetSoundPressureLevel(), 1 );
				FormatString( str, IDS_DLGDIRSEL_TBSOUNDPRESSLEVEL, strTmp );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			}

			FormatString( str, IDS_DLGDIRSEL_TBDIMENSION, (CString)WriteCUDouble( _U_LENGTH, pTecBox->GetHeight() ), (CString)WriteCUDouble( _U_LENGTH, pTecBox->GetWidth()), (CString)WriteCUDouble( _U_LENGTH, pTecBox->GetDepth(), true ) );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			strTmp = WriteCUDouble( _U_MASS, pTecBox->GetWeight(), true );
			FormatString( str, IDS_DLGDIRSEL_WEIGHT, strTmp );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

			if( NULL != pTecBox->GetIntegratedVesselIDPtr().MP )
			{
				bIntegratedVessel = true;
				// HYS-945: Get the buffer vessel
				if( NULL != m_ItemIntegratedSelected.m_hTreeItem )
				{
					DWORD_PTR dwptrInteg = m_TreeProducts.GetItemData( m_ItemIntegratedSelected.m_hTreeItem );
					CTreeItemIntegrated *pclTreeItemIntegrated = dynamic_cast<CTreeItemIntegrated *>( (CTreeItemData *)dwptrInteg );

					if( NULL == pclTreeItemIntegrated )
					{
						ASSERT_RETURN;
					}

					pclIntegratedVessel = pclTreeItemIntegrated->m_pclIntegrated;
				}
			}
		}
		else if( NULL != dynamic_cast<CDB_Product *>( pclProduct ) )
		{
			if( true == m_bWorkingForDpCSet && true == pclProduct->IsPartOfaSet() )
			{
				bShowTemperatureRange = false;
			}
		}
	}

	if( true == bShowTemperatureRange )
	{
		FormatString( str, IDS_DLGDIRSEL_TEMPRANGE, pclProduct->GetTempRange( true ) );
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	}

	// Insert a final blank line.
	DLGDIRSEL_AJUSTCOORDS( iLeftPos, iRightPos );

	if( iLeftPos > 0 )
	{
		pExtListCtrl->InsertItem( LVIF_TEXT | LVIF_STATE, iNbrRow + iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
	}

	if( true == bIntegratedVessel && NULL != dynamic_cast<CDB_TecBox *>( pclProduct ) )
	{
		iRightPos++;
		iLeftPos++;

		// Particular case for integrated vessel.
		CDB_TecBox *pTecBox = dynamic_cast<CDB_TecBox *>( pclProduct );
		
		// HYS-945: Buffer vessel display management.
		if( NULL != m_ItemIntegratedSelected.m_hTreeItem )
		{
			if( ( NULL != pclIntegratedVessel ) && ( true == _IsSelectable( pclIntegratedVessel ) ) )
			{
				CString strTitle;

				if( CDB_Product::eilIntegrated == pclIntegratedVessel->GetInstallationLayout() )
				{
					strTitle = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_INTEGRATEDVESSEL );
				}
				else
				{
					strTitle = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_BUFFERVESSEL );
				}


				// Check first if we have a local article number available.
				CString strLocalArticleNumber = _T("");
				if( true == TASApp.IsLocalArtNumberUsed() )
				{
					strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclIntegratedVessel->GetArtNum() );
					strLocalArticleNumber.Trim();
				}

				// Fill article number.
				bool bArticleNumberShown = false;

				// If we can show article number OR we can't show but there is no local article number...
				if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
				{
					CString str = pclIntegratedVessel->GetArtNum();

					if( false == str.IsEmpty() )
					{
						pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
						pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
						pExtListCtrl->AddItemTextVerify( str, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
						bArticleNumberShown = true;
					}
				}

				// Set local article number if allowed and exist.
				if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
				{
					strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclIntegratedVessel->GetArtNum() );
					strLocalArticleNumber.Trim();
					pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, ( true == bArticleNumberShown ) ? false : true );
					pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
					pExtListCtrl->AddItemTextVerify( strLocalArticleNumber, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
				}

				// Title.
				pExtListCtrl->AddItemTextVerify( strTitle, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

				// Vessel name.
				pExtListCtrl->SetItemTextBold( iNbrRow + iRightPos, DLGDIRSEL_RIGHTCOLUMN, false );
				pExtListCtrl->AddItemTextVerify( pclIntegratedVessel->GetName(), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

				CString strPmax = WriteCUDouble( _U_PRESSURE, pclIntegratedVessel->GetPmaxmax(), true );
				FormatString( str, IDS_DLGDIRSEL_MAXPRESS, strPmax );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

				CString strVol = WriteCUDouble( _U_VOLUME, pclIntegratedVessel->GetNominalVolume(), true );
				FormatString( str, IDS_DLGDIRSEL_NOMINALVOLUME, strVol );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

				CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
				str = TASApp.LoadLocalizedString( IDS_DIRSEL_VESSELTEMPRANGE );
				str += CString( _T(": " ) ) + pclIntegratedVessel->GetTempRange() + _T( " ") + GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str();
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

				CString strWeight = WriteCUDouble( _U_MASS, pclIntegratedVessel->GetWeight(), true );
				FormatString( str, IDS_DLGDIRSEL_WEIGHT, strWeight );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

				// Insert a final blank line.
				DLGDIRSEL_AJUSTCOORDS( iLeftPos, iRightPos );
				pExtListCtrl->InsertItem( LVIF_TEXT | LVIF_STATE, iNbrRow + iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
			}
		}
	}

	pExtListCtrl->SetColumnWidth( DLGDIRSEL_RIGHTCOLUMN, LVSCW_AUTOSIZE_USEHEADER );
}

void CDlgDirSel::FillDpSAccessoryDetails( CDB_Product *pAccessory, CExtListCtrl *pExtListCtrl, bool bHideArticleNumber, bool bByPair )
{
	// Variables.
	CString str; 
	int iNbrRow = pExtListCtrl->GetItemCount();
	int iLeftPos = 0;
	int iRightPos = 0;

	if( NULL == pAccessory || false == pAccessory->IsAnAccessory() )
	{
		return;
	}

	// Check if we must display reference in red.
	bool bNotAvailOrDeleted = ( false == pAccessory->IsAvailable() || true == pAccessory->IsDeleted( ) );
	COLORREF cTextColor = ( false == bNotAvailOrDeleted ) ? _BLACK : _RED;

	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	if( ( true == pAccessory->IsAttached() && ( true == m_bWorkingForDpCSet || true == m_bWorkingForControlSet ) )
			|| ( true == bHideArticleNumber ) )
	{
		// No article or local article number for set.
		pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
		pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
		pExtListCtrl->AddItemTextVerify( _T("-"), iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");
		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		bool bArticleNumberShown = false;

		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
		{
			CString str = pAccessory->GetArtNum();

			if( false == str.IsEmpty() )
			{
				pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
				pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
				bArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
			pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, ( true == bArticleNumberShown ) ? false : true );
			pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
			pExtListCtrl->AddItemTextVerify( strLocalArticleNumber, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == bNotAvailOrDeleted )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pAccessory->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
		pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
		pExtListCtrl->AddItemTextVerify( strFlag, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
	}
	
	str = _T("");

	if( true == bByPair )
	{
		str = _T("2x ");
	}

	CString strName;
	CString strDescription;
	FillAccessoryDetailsHelper_GetNameDescription( pAccessory, strName, strDescription );

	str += strName;
	pExtListCtrl->SetItemTextBold( iNbrRow + iRightPos, DLGDIRSEL_RIGHTCOLUMN, false );
	pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

	str = strDescription;

	if( false == str.IsEmpty() )
	{
		// Treatment to display a very long description.
		int nField = 0;
		int index = 0;
		CStringArray p_outArray;
		ParseString( str, _T( "\n" ), &nField, &p_outArray );
		while( index != nField )
		{
			pExtListCtrl->AddItemTextVerify( p_outArray.GetAt( index ), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			iNbrRow++;
			index++;
		}
	}

	// Add set accessories
	CDB_RuledTable* pclRuledTable = NULL;

	pclRuledTable = dynamic_cast<CDB_RuledTable*>(pAccessory->GetAccessoriesGroupIDPtr().MP);
	DLGDIRSEL_AJUSTCOORDS( iLeftPos, iRightPos );
    pExtListCtrl->InsertItem( LVIF_TEXT | LVIF_STATE, iNbrRow + iLeftPos, _T( "" ), 0, LVIS_SELECTED, 0, 0 );
	if( NULL != pclRuledTable )
	{
		// Sort the list with a string number parameter.
		CRank SortList;
		SortList.PurgeAll();

		// HYS-1617/HYS_1618: We call this method to activate the crossing table option in the 'CDB_RuledTable' table.
		TASApp.GetpTADB()->GetAccessories( &SortList, pclRuledTable, CTADatabase::ForDirSel );

		// Fill the tree with the sorted ID's.
		CString str;
		LPARAM lparam;

		for( BOOL bContinue = SortList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortList.GetNext( str, lparam ) )
		{
			CDB_Product* pclAccessory = (CDB_Product*)lparam;

			if( NULL != pclAccessory && pclAccessory->IsAnAccessory() )
			{
				FillAccessoryDetails( pclAccessory, pExtListCtrl, true, bByPair );
			}
		}
	}
}

void CDlgDirSel::FillAccessoryDetails( CDB_Product *pAccessory, CExtListCtrl *pExtListCtrl, bool bHideArticleNumber, bool bByPair )
{
	// Variables.
	CString str; 
	int iNbrRow = pExtListCtrl->GetItemCount();
	int iLeftPos = 0;
	int iRightPos = 0;

	if( NULL == pAccessory || false == pAccessory->IsAnAccessory() )
	{
		return;
	}

	// Check if we must display reference in red.
	bool bNotAvailOrDeleted = ( false == pAccessory->IsAvailable() || true == pAccessory->IsDeleted( ) );
	COLORREF cTextColor = ( false == bNotAvailOrDeleted ) ? _BLACK : _RED;

	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	if( ( true == pAccessory->IsAttached() && ( true == m_bWorkingForDpCSet || true == m_bWorkingForControlSet ) )
			|| ( true == bHideArticleNumber ) )
	{
		// No article or local article number for set.
		pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
		pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
		pExtListCtrl->AddItemTextVerify( _T("-"), iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");
		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		bool bArticleNumberShown = false;

		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
		{
			CString str = pAccessory->GetArtNum();

			if( false == str.IsEmpty() )
			{
				pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
				pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
				bArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
			pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, ( true == bArticleNumberShown ) ? false : true );
			pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
			pExtListCtrl->AddItemTextVerify( strLocalArticleNumber, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == bNotAvailOrDeleted )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pAccessory->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
		pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
		pExtListCtrl->AddItemTextVerify( strFlag, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
	}
	
	str = _T("");

	if( true == bByPair )
	{
		str = _T("2x ");
	}

	CString strName;
	CString strDescription;
	FillAccessoryDetailsHelper_GetNameDescription( pAccessory, strName, strDescription );

	str += strName;
	pExtListCtrl->SetItemTextBold( iNbrRow + iRightPos, DLGDIRSEL_RIGHTCOLUMN, false );
	pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

	str = strDescription;

	if( false == str.IsEmpty() )
	{
		// Treatment to display a very long description.
		int nField = 0;
		int index = 0;
		CStringArray p_outArray;
		ParseString( str, _T( "\n" ), &nField, &p_outArray );
		while( index != nField )
		{
			pExtListCtrl->AddItemTextVerify( p_outArray.GetAt( index ), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			iNbrRow++;
			index++;
		}
	}

	// Insert a final blank line.
	DLGDIRSEL_AJUSTCOORDS( iLeftPos, iRightPos );
	pExtListCtrl->InsertItem( LVIF_TEXT | LVIF_STATE, iNbrRow + iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgDirSel::FillAccessoryDetailsHelper_GetNameDescription( CDB_Product *pAccessory, CString &strName, CString &strDescription )
{
	strName = pAccessory->GetName();
	strDescription = pAccessory->GetComment();

	if( NULL != dynamic_cast<CDB_DpSensor *>( pAccessory ) )
	{
		// Name with Dp range.
		CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pAccessory );
		strName = pclDpSensor->GetFullName();

		// Add description.
		// HYS-2007: Display burst pressure if it's above 0 value.
		if( 0 < pclDpSensor->GetBurstPressure() )
		{
			// Burst pressure: %1.
			CString strBurstPressure = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true );
			FormatString( strDescription, IDS_TALINK_BURSTPRESSURE, strBurstPressure );
		}
	}
}

void CDlgDirSel::FillAdapterDetails( CDB_Product *pclAdapter, CExtListCtrl *pExtListCtrl )
{
	if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() || NULL == pExtListCtrl )
	{
		return;
	}

	// Variables.
	CString str; 
	int iNbrRow = pExtListCtrl->GetItemCount();
	int iLeftPos = 0;
	int iRightPos = 0;

	// Check if we must display reference in red.
	bool bNotAvailOrDeleted = ( false == pclAdapter->IsAvailable() || true == pclAdapter->IsDeleted( ) );
	COLORREF cTextColor = ( false == bNotAvailOrDeleted ) ? _BLACK : _RED;
				
	if( true == m_bWorkingForDpCSet || true == m_bWorkingForControlSet )
	{
		// No article or local article number for set.
		pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
		pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
		pExtListCtrl->AddItemTextVerify( _T("-"), iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");

		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclAdapter->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		bool bArticleNumberShown = false;

		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
		{
			CString str = pclAdapter->GetArtNum();

			if( false == str.IsEmpty() )
			{
				pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
				pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
				bArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclAdapter->GetArtNum() );
			strLocalArticleNumber.Trim();
			pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, ( true == bArticleNumberShown ) ? false : true );
			pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
			pExtListCtrl->AddItemTextVerify( strLocalArticleNumber, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
		}
	}

	str = pclAdapter->GetName();
	pExtListCtrl->SetItemTextBold( iNbrRow + iRightPos, DLGDIRSEL_RIGHTCOLUMN, false );
	pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

	str = pclAdapter->GetComment();
	pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

	// Insert a blank line.
	DLGDIRSEL_AJUSTCOORDS( iLeftPos, iRightPos );
	pExtListCtrl->InsertItem( LVIF_TEXT | LVIF_STATE, iNbrRow + iLeftPos++, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgDirSel::FillActuatorDetails( CDB_Actuator *pActuator, CExtListCtrl *pExtListCtrl )
{
	// Sanity test.
	if( NULL == pActuator || NULL == pExtListCtrl )
	{
		return;
	}

	CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pActuator );

	// Variables.
	CString str; 
	int iNbrRow = pExtListCtrl->GetItemCount();
	int iLeftPos = 0;
	int iRightPos = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill the left part.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Check if we must display reference in red.
	bool bNotAvailOrDeleted = ( false == pActuator->IsAvailable() || true == pActuator->IsDeleted( ) );
	COLORREF cTextColor =  ( false == bNotAvailOrDeleted ) ? _BLACK : _RED;

	// Insert reference.
	if( true == m_bWorkingForDpCSet || true == m_bWorkingForControlSet )
	{
		// No article or local article number for set.
		pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
		pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
		pExtListCtrl->AddItemTextVerify( _T("-"), iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");

		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pActuator->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		bool bArticleNumberShown = false;

		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )   )
		{
			CString str = pActuator->GetArtNum();
			if( false == str.IsEmpty() )
			{
				pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
				pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
				bArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pActuator->GetArtNum() );
			strLocalArticleNumber.Trim();
			pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, ( true == bArticleNumberShown ) ? false : true );
			pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
			pExtListCtrl->AddItemTextVerify( strLocalArticleNumber, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == bNotAvailOrDeleted )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pActuator->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pExtListCtrl->SetItemTextBold( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, true );
		pExtListCtrl->SetItemTextColor( iNbrRow + iLeftPos, DLGDIRSEL_LEFTCOLUMN, cTextColor, cTextColor );
		pExtListCtrl->AddItemTextVerify( strFlag, iNbrRow + iLeftPos++, DLGDIRSEL_LEFTCOLUMN );
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill the right part.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Insert name.
	str = pActuator->GetName();
	pExtListCtrl->SetItemTextBold( iNbrRow + iRightPos, DLGDIRSEL_RIGHTCOLUMN, false );
	pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	
	CString strDescription = pActuator->GetComment();

	if( false == strDescription.IsEmpty() )
	{
		pExtListCtrl->AddItemTextVerify( pActuator->GetComment(), iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	}

	if( m_ItemProductSelected.m_hTreeItem )
	{
		DWORD_PTR dwptr = m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItem );
		CTreeItemProduct *pclTreeItemTAProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)dwptr );

		if( NULL == pclTreeItemTAProduct )
		{
			ASSERT_RETURN;
		}

		CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pclTreeItemTAProduct->m_pclProduct );

		if( NULL != pCV )
		{
			// Set the Close off Dp if exist.
			CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pCV->GetCloseOffCharIDPtr().MP );

			if( NULL != pCloseOffChar && CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_CLOSEOFFDP );
				double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pActuator->GetMaxForceTorque() );

				if( dCloseOffDp > 0.0 )
				{
					str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
				}
				else
				{
					str += CString( _T(" : -") );
				}

				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			}

			// Set the maximum inlet pressure if exist.
			if( NULL != pCloseOffChar && CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_MAXINLETPRESSURE );
				double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pActuator->GetMaxForceTorque() );

				if( dMaxInletPressure > 0.0 )
				{
					str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
				}
				else
				{
					str += CString( _T(" : -") );
				}

				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			}
			
			// Compute actuating time in sec.
			if( NULL != pElectroActuator )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACTUATING_TIME ) + CString( _T(" : ") ) + pElectroActuator->GetActuatingTimesStr( pCV->GetStroke(), true );
				pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
			}
		}
	}
	else
	{
		if( pActuator->GetMaxForceTorque() > 0 )
		{
			if( CDB_CloseOffChar::Linear == pActuator->GetOpeningType() )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_MAXFORCE );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_MAXTORQUE );
			}

			str += CString( _T(" : ") ) + pActuator->GetMaxForceTorqueStr( true );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}

		// Compute Actuating time in s/mm or s/deg.
		if( NULL != pElectroActuator )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACTUATING_TIME ) + CString( _T(" : ") )	+ pElectroActuator->GetActuatingTimesStr( 0, true );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
	}
	
	if(NULL != pElectroActuator)
	{
		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_IPXXAUTO );
		str += CString( _T(" : ") ) + pElectroActuator->GetIPxxAuto();

		if( false == pElectroActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T("(") + pElectroActuator->GetIPxxManual() + _T(")");
		}

		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_POWSUP );
		str += CString( _T(" : ") ) + pElectroActuator->GetPowerSupplyStr();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_INPUTSIG );
		str += CString( _T(" : ") ) + pElectroActuator->GetInOutSignalsStr( true );
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_OUTPUTSIG );

		if( CString( _T("") ) == pElectroActuator->GetInOutSignalsStr( false ) )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pElectroActuator->GetInOutSignalsStr( false );
		}

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_RELAYTYPE ) + _T(" : ");
		str += pElectroActuator->GetRelayStr();
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_FAILSAFE );

		if( CDB_ElectroActuator::FailSafeType::eFSTypeElectronic == pElectroActuator->GetFailSafe() )
		{
			str += CString( _T(" : ") ) +  TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn == pElectroActuator->GetFailSafe() )
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str += CString( _T(" : ") ) + TASApp.LoadLocalizedString( IDS_NO );
		}
		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );

		if( pElectroActuator->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic
			&& pElectroActuator->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FSP ) + _T( " " );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP ) + _T( " " );
		}

		if( CString( _T("") ) == pElectroActuator->GetDefaultReturnPosStr( pElectroActuator->GetDefaultReturnPos() ).c_str() )
		{
			str += CString( _T("   -") );
		}
		else
		{
			str += pElectroActuator->GetDefaultReturnPosStr( pElectroActuator->GetDefaultReturnPos() ).c_str();
		}

		pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
	}
	
	CDB_ThermostaticActuator *pThermoAct = dynamic_cast<CDB_ThermostaticActuator *>( pActuator );

	if( NULL != pThermoAct )
	{
		// HYS-951: Min setting can be 0.
		if( pThermoAct->GetMinSetting() >= 0 && pThermoAct->GetMaxSetting() > 0 )
		{
			CString str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_SETTINGRANGE );
			CString strMin = WriteCUDouble( _U_TEMPERATURE, pThermoAct->GetMinSetting(), false );
			CString strMax =  WriteCUDouble( _U_TEMPERATURE, pThermoAct->GetMaxSetting(), true );
			str.Format( _T("%s: [%s-%s]"), str1, strMin, strMax );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
		
		if( pThermoAct->GetCapillaryLength() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_CAPILLENGTH );
			str += CString( _T(": ") ) + WriteCUDouble( _U_LENGTH, pThermoAct->GetCapillaryLength(), true );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
		
		if( pThermoAct->GetHysteresis() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_HYSTERESIS );
			str += CString( _T(": ") ) + WriteCUDouble( _U_DIFFTEMP, pThermoAct->GetHysteresis(), true );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}

		if( pThermoAct->GetFrostProtection() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_FROSTPROTECT );
			str += CString( _T(": ") ) + WriteCUDouble( _U_TEMPERATURE, pThermoAct->GetFrostProtection(), true );
			pExtListCtrl->AddItemTextVerify( str, iNbrRow + iRightPos++, DLGDIRSEL_RIGHTCOLUMN );
		}
	}

	// Insert a final blank line.
	DLGDIRSEL_AJUSTCOORDS( iLeftPos, iRightPos );
	pExtListCtrl->InsertItem( LVIF_TEXT | LVIF_STATE, iNbrRow + iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgDirSel::FillImage( CDB_Product *pclProduct )
{
	if( NULL != m_Bmp.GetSafeHandle() )
	{
		m_Bmp.DeleteObject();
	}

	CProdPic *pclProductPicture = TASApp.GetpProdPic( pclProduct->GetProdPicID(), &m_Bmp );

	// Verify the image exist.
	if( NULL != pclProductPicture && CProdPic::Pic == pclProductPicture->GetProdPicType() )
	{
		COLORREF cBck = m_Bmp.GetPixel( 0, 0 );
		m_Bmp.ReplaceColor( cBck, GetSysColor( COLOR_BTNFACE ) );

		if( epVertical != pclProductPicture->GetProdPosition())
		{
			m_Bmp.RotateImage( -90, GetSysColor( COLOR_BTNFACE ) );
		}

		if( NULL != m_Bmp.m_hObject )
		{
			// Verify the "Gif" image has no higher dimension than the rectangle
			// If it's the case, stretch the image to fit in the rectangle.
			CRect rect;
			m_staticImg.GetClientRect( &rect );
			rect.DeflateRect( 3, 3, 3, 3 );
			CRect rectbmp = m_Bmp.GetSizeImage();
			
			if( rectbmp.Height() > rect.Height() || rectbmp.Width() > rect.Width() )
			{
				double dHeightDiff = 0;
				double dWidthDiff = 0;

				if( rectbmp.Height() > rect.Height() )
				{
					dHeightDiff = rectbmp.Height() - rect.Height();
				}
					
				if( rectbmp.Width() > rect.Width() )
				{
					dWidthDiff = rectbmp.Width() - rect.Width();
				}
					
				if( dHeightDiff > dWidthDiff )
				{
					m_Bmp.ResizeImage( 1 - ( dHeightDiff / (double)rectbmp.Height() ) );
				}
				else
				{
					m_Bmp.ResizeImage( 1 - ( dWidthDiff / (double)rectbmp.Width() ) );
				}
			}

			m_staticImg.SetBitmap( (HBITMAP)m_Bmp );
		}
	}
}

void CDlgDirSel::ClickTreeProducts( HTREEITEM hItem )
{
	if( NULL == hItem )
	{
		return;
	}

	// Remove the image.
	m_staticImg.SetBitmap( (HBITMAP)NULL );
	HTREEITEM hTreeItemParent = _GetParentProduct( hItem, true );

	CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hItem );

	switch( pclTreeItemData->m_eDataType )
	{
		case CTreeItemData::DataType::Product:
			ClickProduct( hItem, true );
			break;

		case CTreeItemData::DataType::Adapter:
			ClickAdapter( hItem );
			break;

		case CTreeItemData::DataType::Actuator:
			ClickActuator( hItem );
			break;

		case CTreeItemData::DataType::Accessory:
			ClickAccessory( hItem );
			break;
		//HYS-945
		case CTreeItemData::DataType::Integrated:
			ClickIntegrated( hItem );
			break;
	}

	m_List_ArticleDetails.DeleteAllItems();

	// HYS-1877: We click on one product, the 6-way valve set and display all set contain in the last column.
	CDB_Actuator *pclActFor6WayValveSet = NULL;
	CDB_Product *pclPIBCVFor6WayValveSet = NULL;
	CDB_Actuator *pclPIBCVActFor6WayValveSet = NULL;
	CDB_RuledTable *pclAccessoriesForSet = NULL;

	if( NULL != m_ItemProductSelected.m_hTreeItem )
	{
		DWORD_PTR dwptrTAProduct = m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItem );
		CTreeItemProduct *pclTreeItemTAProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)dwptrTAProduct );
		
		if( NULL != pclTreeItemTAProduct && NULL != pclTreeItemTAProduct->m_pclProduct )
		{
			FillProductDetails( pclTreeItemTAProduct->m_pclProduct, &m_List_ArticleDetails );
		}

		// HYS-1877: Display 6-way valve set contains.
		if( true == m_bWorkingForControlSet && NULL != dynamic_cast<CDB_6WayValve *>(pclTreeItemTAProduct->m_pclProduct) )
		{
			CDB_6WayValve *pcl6WayValveForSet = dynamic_cast<CDB_6WayValve *>( pclTreeItemTAProduct->m_pclProduct );
			CTableSet *pCVActSetTab = pcl6WayValveForSet->GetTableSet();

			if( NULL != pCVActSetTab )
			{
				CDB_Set *pcl6WayValveSet = pCVActSetTab->FindCompatibleSet( pcl6WayValveForSet->GetIDPtr().ID, _T("") );
				
				if( NULL != pcl6WayValveSet )
				{
					// Get 6-way valve actuator for set.
					pclActFor6WayValveSet = dynamic_cast<CDB_Actuator *>( pcl6WayValveSet->GetSecondIDPtr().MP );
					
					if( NULL != pclActFor6WayValveSet )
					{
						FillActuatorDetails( pclActFor6WayValveSet, &m_List_ArticleDetails );
					}

					// Find PIBCV set for 6-way valve set.
					CDB_Set *pclPIBCVSet = dynamic_cast<CDB_Set *>( m_clDirectSelectionParams.m_pTADB->Get( pcl6WayValveSet->GetNextID() ).MP );
					
					if( NULL != pclPIBCVSet )
					{
						pclPIBCVFor6WayValveSet = dynamic_cast<CDB_Product* >( pclPIBCVSet->GetFirstIDPtr().MP );
						pclPIBCVActFor6WayValveSet = dynamic_cast<CDB_Actuator *>( pclPIBCVSet->GetSecondIDPtr().MP );
						
						// Find accessories.
						pclAccessoriesForSet = (CDB_RuledTable *)( pclPIBCVSet->GetAccGroupIDPtr().MP );
					}
				}
			}
		}
	}

	// Manage smart differential pressure controller.
	if( true == m_bWorkingForDpCSet && NULL != m_ItemProductSelected.m_hTreeItemSub && NULL != m_ItemProductSelected.m_hTreeItem )
	{
		DWORD_PTR dwptrProductParentDpC = m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItem );
		CTreeItemProduct *pclTreeItemProductParentDpC = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)dwptrProductParentDpC );

		DWORD_PTR dwptrTAProduct = m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItemSub );
		CTreeItemProduct *pclTreeItemTAProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)dwptrTAProduct );

		if( NULL != pclTreeItemTAProduct && NULL != pclTreeItemTAProduct->m_pclProduct )
		{
			FillProductDetails( pclTreeItemTAProduct->m_pclProduct, &m_List_ArticleDetails );
		}
	}

	if( m_vecProductAccessorySelected.size() > 0 )
	{
		CRank SortListAccessories;
		SortListAccessories.PurgeAll();
	
		// Sort the list.
		for( int i = 0; i < (int)m_vecProductAccessorySelected.size(); i++ )
		{
			if( true == m_vecProductAccessorySelected[i].m_bCheck )
			{
				SortListAccessories.AddStrSort( m_vecProductAccessorySelected[i].m_pclAccessory->GetName(), 0, (LPARAM)( &m_vecProductAccessorySelected[i] ), false );
			}
		}
		
		CString str;
		LPARAM lparam;

		for( BOOL bContinue = SortListAccessories.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortListAccessories.GetNext( str, lparam ) )
		{
			CAccessorySelected *pclAccessorySelected = (CAccessorySelected *)lparam;
			bool bByPair = false;
			
			if( NULL != pclAccessorySelected->m_pclRuledTable 
					&& true == pclAccessorySelected->m_pclRuledTable->IsByPair( pclAccessorySelected->m_pclAccessory->GetIDPtr().ID ) )
			{
				bByPair = true;
			}

			// HYS-1992: Show accessory details for Smart DpC.
			DWORD_PTR dwptrParent = m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItem );
			CTreeItemProduct* pclTreeItemParent = dynamic_cast<CTreeItemProduct*>((CTreeItemData*)dwptrParent);

			// Select accessory under the fictif set
			if( (NULL != dynamic_cast<CDB_SmartControlValve*>(pclTreeItemParent->m_pclProduct))
				&& (0 == StringCompare( _T( "SMARTDPCTYPE" ), pclTreeItemParent->m_pclProduct->GetTypeID() )) )
			{
				FillDpSAccessoryDetails( pclAccessorySelected->m_pclAccessory, &m_List_ArticleDetails, false, bByPair );
			}
			else
			{
				// HYS-2007: Do not show dp sensor set accessories article number.
				bool bHideArticleNumber = false;
				if( (NULL != dynamic_cast<CDB_DpSensor*>(pclTreeItemParent->m_pclProduct))
					&& (true == pclTreeItemParent->m_pclProduct->IsPartOfaSet()) )
				{
					// fictif set don't show accessorie's article number 
					bHideArticleNumber = true;
				}
				FillAccessoryDetails( pclAccessorySelected->m_pclAccessory, &m_List_ArticleDetails, bHideArticleNumber, bByPair );
			}
		}
	}

	if( NULL != m_ItemAdapterSelected.m_hTreeItem )
	{
		DWORD_PTR dwptrAdapter = m_TreeProducts.GetItemData( m_ItemAdapterSelected.m_hTreeItem );
		CTreeItemAdapter *pclTreeItemAdapter = dynamic_cast<CTreeItemAdapter *>( (CTreeItemData *)dwptrAdapter );

		if( NULL != pclTreeItemAdapter && NULL != pclTreeItemAdapter->m_pclAdapter )
		{
			FillAdapterDetails( pclTreeItemAdapter->m_pclAdapter, &m_List_ArticleDetails );
		}
	}

	if( NULL != m_ItemActuatorSelected.m_hTreeItem )
	{
		DWORD_PTR dwptrActuator = m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem );
		CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)dwptrActuator );

		if( NULL != pclTreeItemActuator && NULL != pclTreeItemActuator->m_pclActuator )
		{
			FillActuatorDetails( pclTreeItemActuator->m_pclActuator, &m_List_ArticleDetails );
		}
	}
	
	if( m_vecActuatorAccessorySelected.size() > 0 )
	{
		CRank SortListActAccessories;
		SortListActAccessories.PurgeAll();
		
		// Sort the list.
		for( int i = 0; i < (int)m_vecActuatorAccessorySelected.size(); i++ )
		{
			if( true == m_vecActuatorAccessorySelected[i].m_bCheck )
			{
				SortListActAccessories.AddStrSort( m_vecActuatorAccessorySelected[i].m_pclAccessory->GetName(), 0, (LPARAM)( &m_vecActuatorAccessorySelected[i] ), false );
			}
		}
		
		CString str;
		LPARAM lparam;
		
		for( BOOL bContinue = SortListActAccessories.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortListActAccessories.GetNext( str, lparam ) )
		{
			CAccessorySelected *pclAccessorySelected = (CAccessorySelected *)lparam;
			bool bByPair = false;

			if( NULL != pclAccessorySelected->m_pclRuledTable 
					&& true == pclAccessorySelected->m_pclRuledTable->IsByPair( pclAccessorySelected->m_pclAccessory->GetIDPtr().ID ) )
			{
				bByPair = true;
			}

			FillAccessoryDetails( pclAccessorySelected->m_pclAccessory, &m_List_ArticleDetails, false, bByPair );
		}
	}

	// HYS-1877: Manage 6-way valve kit
	if( NULL != pclPIBCVFor6WayValveSet )
	{
		// Do not use PIBCV as a set.
		FillProductDetails( pclPIBCVFor6WayValveSet, &m_List_ArticleDetails );

		// Fill accessories.
		if( NULL != pclAccessoriesForSet )
		{
			CRank rList;
			m_clDirectSelectionParams.m_pTADB->GetAccessories( &rList, pclAccessoriesForSet, CTADatabase::FilterSelection::ForDirSel );
			CString str;
			LPARAM lparam;

			for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rList.GetNext( str, lparam ) )
			{
				CDB_Product *pclAccessory = (CDB_Product *)lparam;
				bool bByPair = false;

				if( true == pclAccessoriesForSet->IsByPair( pclAccessory->GetIDPtr().ID ) )
				{
					bByPair = true;
				}

				FillAccessoryDetails( pclAccessory, &m_List_ArticleDetails, true, bByPair );
			}
		}

		// Fill Actuators
		if( NULL != pclPIBCVActFor6WayValveSet )
		{
			FillActuatorDetails( pclPIBCVActFor6WayValveSet, &m_List_ArticleDetails );
		}
	}

	// HYS-945: Buffer vessel accessories management
	if( m_vecIntegratedAccessorySelected.size() > 0 )
	{
		CRank SortListIntegratedAccessories;
		SortListIntegratedAccessories.PurgeAll();

		// Sort the list.
		for( int i = 0; i < (int)m_vecIntegratedAccessorySelected.size(); i++ )
		{
			if( true == m_vecIntegratedAccessorySelected[i].m_bCheck )
			{
				SortListIntegratedAccessories.AddStrSort( m_vecIntegratedAccessorySelected[i].m_pclAccessory->GetName(), 0, (LPARAM)( &m_vecIntegratedAccessorySelected[i] ), false );
			}
		}

		CString str;
		LPARAM lparam;

		for( BOOL bContinue = SortListIntegratedAccessories.GetFirst( str, lparam ); TRUE == bContinue; bContinue = SortListIntegratedAccessories.GetNext( str, lparam ) )
		{
			CAccessorySelected *pclIntegratedAccessorySelected = (CAccessorySelected *)lparam;
			bool bByPair = false;

			if( NULL != pclIntegratedAccessorySelected->m_pclRuledTable
				&& true == pclIntegratedAccessorySelected->m_pclRuledTable->IsByPair( pclIntegratedAccessorySelected->m_pclAccessory->GetIDPtr().ID ) )
			{
				bByPair = true;
			}

			FillAccessoryDetails( pclIntegratedAccessorySelected->m_pclAccessory, &m_List_ArticleDetails, false, bByPair );
		}
	}

	if( NULL != hTreeItemParent )
	{
		CTreeItemProduct *pclTreeItemParent = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( hTreeItemParent ) );

		if( NULL != pclTreeItemParent && NULL != pclTreeItemParent->m_pclProduct )
		{
			FillImage( pclTreeItemParent->m_pclProduct );
		}
	}
}

void CDlgDirSel::ClickProduct( HTREEITEM hItemSelected, bool bUserClick )
{
	CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemSelected ) );

	if( NULL == pclTreeItemProduct )
	{
		return;
	}

	bool bSameItemAsBefore = ( m_ItemProductSelected.m_hTreeItem == hItemSelected || 
			( true == m_bWorkingForDpCSet && m_ItemProductSelected.m_hTreeItemSub == hItemSelected ) );
	
	// New selected product.
	if( false == bSameItemAsBefore )
	{
		// Particular case: if no product is selected but at least one of its adapter or actuator is already selected, we do not
		// clean the selection.
		bool bClean = true;

		if( NULL == m_ItemProductSelected.m_hTreeItem )
		{
			if( ( NULL != m_ItemAdapterSelected.m_hTreeItem && hItemSelected == _GetParentProduct( m_ItemAdapterSelected.m_hTreeItem ) ) 
					|| ( NULL != m_ItemActuatorSelected.m_hTreeItem && hItemSelected == _GetParentProduct( m_ItemActuatorSelected.m_hTreeItem ) ) )
			{
				bClean = false;
			}
		}

		// Un-select previous ones.
		if( true == bClean )
		{
			// HYS-1992: Check builtin accessory to know if the iten stay disabled.
			if( NULL != m_ItemProductSelected.m_hTreeItem )
			{
				bool bBuiltinExists = false;
				bool bBuiltinExistsSub = false;

				CTreeItemProduct* pclItemProductSelected = (CTreeItemProduct*)m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItem );
				CTreeItemProduct* pclItemProductSubSelected = NULL;

				if( NULL != m_ItemProductSelected.m_hTreeItemSub )
				{
					pclItemProductSubSelected = (CTreeItemProduct*)m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItemSub );
				}

				CDB_Product* pclSubProduct = NULL;

				if( NULL == pclItemProductSubSelected && NULL != m_ItemProductSelected.m_hTreeItemSub
					&& NULL != (CTreeItemAccessory*)m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItemSub ) )
				{
					pclSubProduct = ((CTreeItemAccessory*)m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItemSub ))->m_pclAccessory;
				}
				else if( NULL != pclItemProductSubSelected )
				{
					pclSubProduct = pclItemProductSubSelected->m_pclProduct;
				}

				HasBuiltinAccessory( pclItemProductSelected->m_pclProduct, pclSubProduct, bBuiltinExists, bBuiltinExistsSub );
				_CleanCurrentProduct( true, bBuiltinExists, bBuiltinExistsSub );
			}
			else
			{
				_CleanCurrentProduct( true );
			}
		}
		
		// Select the new one.
		m_TreeProducts.SetItemImage( hItemSelected, RadioSelected, RadioSelected );

		if( true == m_bWorkingForDpCSet )
		{
			if( NULL != dynamic_cast<CDB_RegulatingValve *>( pclTreeItemProduct->m_pclProduct ) )
			{
				// User clicks on a balancing valve.
				m_ItemProductSelected.m_hTreeItemSub = hItemSelected;

				// We automatically select the corresponding Dp controller.
				HTREEITEM htDpC = m_TreeProducts.GetParentItem( m_TreeProducts.GetParentItem( hItemSelected ) );
				m_ItemProductSelected.m_hTreeItem = htDpC;
				m_TreeProducts.SetItemImage( htDpC, RadioSelected, RadioSelected );
			}
			else
			{
				// User clicks on a Dp controller or a smart differential pressure controller.
				m_ItemProductSelected.m_hTreeItem = hItemSelected;
				// We automatically select the first corresponding balancing valve.
				HTREEITEM hMeasValveTitleItem = _GetMeasValveTitle( hItemSelected );

				if( NULL != hMeasValveTitleItem && NULL != m_TreeProducts.GetChildItem( hMeasValveTitleItem ) )
				{
					m_ItemProductSelected.m_hTreeItemSub = m_TreeProducts.GetChildItem( hMeasValveTitleItem );
					m_TreeProducts.SetItemImage( m_ItemProductSelected.m_hTreeItemSub, RadioSelected, RadioSelected );
				}
			}
		}
		else
		{
			m_ItemProductSelected.m_hTreeItem = hItemSelected;
		}
		
		// Do the built-in solution.
		GetBuiltInAccessories( hItemSelected );

		if( true == bUserClick && true == m_bWorkingForControlSet )
		{
			// We select first actuator by default.
			HTREEITEM hActuatorTitleItem = _GetActuatorTitle( hItemSelected );

			if( NULL != hActuatorTitleItem && NULL != m_TreeProducts.GetChildItem( hActuatorTitleItem ) )
			{
				ClickActuator( m_TreeProducts.GetChildItem( hActuatorTitleItem ) );
			}
		}

		// Reset the list focus and fill the list box just below the tree with short infos.
		m_ListBoxFocus.DeleteAllItems();
		FillProductDetails( pclTreeItemProduct->m_pclProduct, &m_ListBoxFocus );

		FillImage( pclTreeItemProduct->m_pclProduct );
	}
	else
	{
		// Same selected product than before then erase selection.

		m_TreeProducts.SetItemImage( m_ItemProductSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
		
		// Case of Dp controller and balancing valve set or smart differential pressure controller set.
		if( true == m_bWorkingForDpCSet && NULL != m_ItemProductSelected.m_hTreeItemSub )
		{
			m_TreeProducts.SetItemImage( m_ItemProductSelected.m_hTreeItemSub, RadioNotSelected, RadioNotSelected );
		}

		m_ItemProductSelected.Clean();

		// Check if we are with the primary product (parent NULL) or the secondary.

		if( NULL == m_TreeProducts.GetParentItem( hItemSelected ) )
		{
			// Primary product -> clean both product and sub product accessories.
			CleanVectorOfSelectedAccessories( &m_vecProductAccessorySelected );
			CleanVectorOfSelectedAccessories( &m_vecSubProductAccessorySelected );
		}
		else
		{
			// Secondary product -> clean only secondary accessories
			CleanVectorOfSelectedAccessories( &m_vecSubProductAccessorySelected );
		}

		// Clean adapter if exist. It's a nonsense to select adapter without control valve.
		if( NULL != m_ItemAdapterSelected.m_hTreeItem )
		{
			_UnselectAllAdapters( m_TreeProducts.GetParentItem( m_ItemAdapterSelected.m_hTreeItem ) );
			m_ItemAdapterSelected.Clean();
		}

		// Clean actuator if exist. It's a nonsense to select actuator without control valve.
		// If user wants to select actuator only, he must select the actuator category in the left pane.
		if( NULL != m_ItemActuatorSelected.m_hTreeItem )
		{
			m_TreeProducts.SetItemImage( m_ItemActuatorSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
			m_ItemActuatorSelected.Clean();
			CleanVectorOfSelectedAccessories( &m_vecActuatorAccessorySelected );
		}
		// HYS-945
		if( NULL != m_ItemIntegratedSelected.m_hTreeItem )
		{
			m_TreeProducts.SetItemImage( m_ItemIntegratedSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
			m_ItemIntegratedSelected.Clean();
			CleanVectorOfSelectedAccessories( &m_vecIntegratedAccessorySelected );
		}

		// Reset the list focus and fill the list box just below the tree with short infos.
		m_ListBoxFocus.DeleteAllItems();
	}
}

void CDlgDirSel::ClickAccessory( HTREEITEM hItem )
{
	// Verify first if checkbox is not disabled.
	int iImage;
	int iImageSelected;
	bool bButtonImageCheckBox = true;

	if( FALSE == m_TreeProducts.GetItemImage( hItem, iImage, iImageSelected ) )
	{
		return;
	}

	// HYS-1992: Now we have accessories with radio button.
	if( RadioSelected == iImage || RadioNotSelected == iImage || RadioSelectedDisable == iImage || RadioNotSelectedDisable == iImage )
	{
		bButtonImageCheckBox = false;
	}
	
	if( ( true == bButtonImageCheckBox ) && ( CheckboxSelectedDisable == iImage || CheckboxNotSelectedDisable == iImage ) )
	{
		return;
	}
		
	if( ( false == bButtonImageCheckBox ) && ( RadioSelectedDisable == iImage || RadioNotSelectedDisable == iImage ) )
	{
		return;
	}
	
	CTreeItemAccessory *pclTreeItemAccessoryClicked = dynamic_cast<CTreeItemAccessory *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItem ) );

	if( NULL == pclTreeItemAccessoryClicked || NULL == pclTreeItemAccessoryClicked->m_pclAccessory || false == pclTreeItemAccessoryClicked->m_pclAccessory->IsAnAccessory() )
	{
		ASSERT_RETURN;
	}
	
	CAccessorySelected clItemAccessoryToSave;
	clItemAccessoryToSave.m_pclAccessory = pclTreeItemAccessoryClicked->m_pclAccessory;
	clItemAccessoryToSave.m_eAccessoryType = pclTreeItemAccessoryClicked->m_eAccessoryType;
	clItemAccessoryToSave.m_pclRuledTable = pclTreeItemAccessoryClicked->m_pclRuledTable;
	clItemAccessoryToSave.m_hTreeItem = hItem;

	// Identify the parent.
	// Remark: PAY ATTENTION 'm_ItemProductSelected' is not necessarily the product linked to the accessory. User can click on an accessory on an other
	//         product. This is the reason why we need to check below the correct parent of the accessory clicked.
	HTREEITEM hParentItem = _GetParentProduct( hItem );

	if( NULL == hParentItem )
	{
		ASSERT_RETURN;
	}
	
	// Identify the parent type, it could be an actuator or a product.
	CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hParentItem );

	if( NULL == pclTreeItemData )
	{
		ASSERT_RETURN;
	}

	bool bCleaningNeeded = false;
	bool bWorkForProduct = ( CTreeItemData::DataType::Product == pclTreeItemData->m_eDataType ) ? true : false;
	// HYS-945: Buffer vessel management
	bool bWorkForIntegrated = ( CTreeItemData::DataType::Integrated == pclTreeItemData->m_eDataType ) ? true : false;
	
	CDB_Product *pclProduct = NULL;
	CDB_Actuator *pclActuator = NULL;
	
	if( true == bWorkForProduct )
	{
		CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( pclTreeItemData );

		if( pclTreeItemProduct != NULL )
		{
			pclProduct = pclTreeItemProduct->m_pclProduct;
		}
	}
	else if( true == bWorkForIntegrated )
	{
		CTreeItemIntegrated *pclTreeItemIntegrated = dynamic_cast<CTreeItemIntegrated *>( pclTreeItemData );

		if( pclTreeItemIntegrated != NULL )
		{
			pclProduct = pclTreeItemIntegrated->m_pclIntegrated;
		}
	}
	else
	{
		CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( pclTreeItemData );

		if( pclTreeItemActuator != NULL )
		{
			pclActuator = pclTreeItemActuator->m_pclActuator;
		}
	}

	// Sanity.
	if( NULL == pclProduct && NULL == pclActuator )
	{
		ASSERT_RETURN;
	}

	if( true == bWorkForProduct )
	{
		// Selected accessory is below a product.

		if( true == m_bWorkingForDpCSet )
		{
			if( hParentItem != m_ItemProductSelected.m_hTreeItem && hParentItem != m_ItemProductSelected.m_hTreeItemSub )
			{
				bCleaningNeeded = true;
			}
		}
		else if( hParentItem != m_ItemProductSelected.m_hTreeItem )
		{
			bCleaningNeeded = true;
		}
	}
	else
	{
		if( true == bWorkForIntegrated )
		{ 
			if( hParentItem != m_ItemIntegratedSelected.m_hTreeItem )
			{
				bCleaningNeeded = true;
			}
		}
		// Selected accessory is below actuator but it's not the same.
		else if( hParentItem != m_ItemActuatorSelected.m_hTreeItem )
		{
			bCleaningNeeded = true;
		}
	}
	
	// HYS-945: Buffer vessel management.
	std::vector<CAccessorySelected> *pvecAccessorySelected = NULL;
	
	if( true == bWorkForProduct )
	{
		// To check if 'hParentItem' is the main product or a sub product.
		// Remark: it's typically for smart differential pressur controller + sets.
		HTREEITEM hGrandParentItem = _GetParentProduct( hParentItem );

		if( NULL == hGrandParentItem )
		{
			pvecAccessorySelected = &m_vecProductAccessorySelected;
		}
		else
		{
			pvecAccessorySelected = &m_vecSubProductAccessorySelected;
		}
	}
	else if( true == bWorkForIntegrated )
	{
		pvecAccessorySelected = &m_vecIntegratedAccessorySelected;
	}
	else
	{
		pvecAccessorySelected = &m_vecActuatorAccessorySelected;
	}

	bool bSelectedItem = false;
	bool bUnselectableItem = false;
	int iIndex = 0;
	
	// Verify if this accessory is already existing into selected array.
	// Remark: verify ONLY if we are under the same parent.
	if( false == bCleaningNeeded )
	{
		for( int i = 0; i < (int)pvecAccessorySelected->size(); i++ )
		{
			CTreeItemAccessory *pclTreeItemAccessory = (CTreeItemAccessory *)( m_TreeProducts.GetItemData( (*pvecAccessorySelected)[i].m_hTreeItem ) );

			// Try to find accessory.
			if( NULL != pclTreeItemAccessory && pclTreeItemAccessoryClicked->m_pclAccessory == pclTreeItemAccessory->m_pclAccessory )
			{
				// To say the item exist.
				bSelectedItem = true;

				// A pointer that point to the correct selection.
				iIndex = i;

				if( false == (*pvecAccessorySelected)[i].m_bEnable )
				{
					bUnselectableItem = true;			// To say the item isn't un-selectable
				}
			}
		}
	}

	// HYS-2031: Get all excluded products what ever their parent in the tree.
	std::vector<CData *> setAccGroup;
	CDB_RuledTable *pclRuledTable = NULL;

	pclRuledTable = dynamic_cast<CDB_RuledTable *>(pclProduct->GetAccessoriesGroupIDPtr().MP);
	if( NULL != pclRuledTable )
	{
		// Accessories group exist.
		pclRuledTable->GetVector( CDB_RuledTable::Excluded, pclTreeItemAccessoryClicked->m_pclAccessory, &setAccGroup );
	}

	// If the item wasn't selected, select it...
	if( false == bSelectedItem )
	{
		clItemAccessoryToSave.m_bCheck = true;
		clItemAccessoryToSave.m_bEnable = true;
		int iImageStatus = CheckboxSelected;
		if( false == bButtonImageCheckBox )
		{
			iImageStatus = RadioSelected;
			// HYS-1992: Clean only accessories with same item type.
			CleanVectorOfSelectedAccessories( pvecAccessorySelected, true, RadioSelected );
		}

		m_TreeProducts.SetItemImage( hItem, iImageStatus, iImageStatus );

		if( true == bCleaningNeeded )
		{
			// Previous selected accessory was not below the same parent then clean the list.
			CleanVectorOfSelectedAccessories( pvecAccessorySelected );

			// Unselect previous adapter, actuator and actuator accessories if exist.
			if( NULL != m_ItemAdapterSelected.m_hTreeItem )
			{
				_UnselectAllAdapters( m_TreeProducts.GetParentItem( m_ItemAdapterSelected.m_hTreeItem ) );
				m_ItemAdapterSelected.Clean();
			}

			if( NULL != m_ItemActuatorSelected.m_hTreeItem )
			{
				m_TreeProducts.SetItemImage( m_ItemActuatorSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
				m_ItemActuatorSelected.Clean();
				CleanVectorOfSelectedAccessories( &m_vecActuatorAccessorySelected );
			}
					
			// Select parent item.
			if( true == bWorkForProduct )
			{
				ClickProduct( hParentItem );
			}
			else if( true == bWorkForIntegrated )
			{
				ClickIntegrated( hParentItem );
			}
			else
			{
				ClickActuator( hParentItem );
			}
	
			// Check for excluded and implicated items.
			GetLinkedAccessories( hItem, hParentItem, bWorkForProduct, CDB_RuledTable::Excluded );
			GetLinkedAccessories( hItem, hParentItem, bWorkForProduct, CDB_RuledTable::Implicated );

			// HYS-2031: Manage excluded products for the current accessory when they are in different items.
			// Set excluded products button status.
			VerifyOtherExcludedProduct( hItem, hParentItem, setAccGroup );

			bool bAccExist = false;

			for( int i = 0; i < (int)pvecAccessorySelected->size(); i++ )
			{
				if( (*pvecAccessorySelected)[i].m_hTreeItem == hItem )
				{
					bAccExist = true;
				}
			}
			
			if( false == bAccExist )
			{
				pvecAccessorySelected->push_back( clItemAccessoryToSave );
			}
		}
		else
		{
			pvecAccessorySelected->push_back( clItemAccessoryToSave );
			
			// Check for excluded and implicated items.
			GetLinkedAccessories( hItem, hParentItem, bWorkForProduct, CDB_RuledTable::Excluded  );
			GetLinkedAccessories( hItem, hParentItem, bWorkForProduct, CDB_RuledTable::Implicated );

			// HYS-2031: Manage excluded products for the current accessory when they are in different items.
			// Set excluded products button status.
			VerifyOtherExcludedProduct( hItem, hParentItem, setAccGroup );
		}
	}
	else
	{
		// If the item was selected , un-select it (if possible).

		int iImageStatus = CheckboxNotSelected;
		if( false == bButtonImageCheckBox )
		{
			iImageStatus = RadioNotSelected;
		}
		// If the selected item is selectable...
		if( false == bUnselectableItem )
		{
			pvecAccessorySelected->erase( pvecAccessorySelected->begin() + iIndex );
			m_TreeProducts.SetItemImage( hItem, iImageStatus, iImageStatus );
			RemoveLinkedAccessories( hItem, bWorkForProduct );

			// HYS-2031: Manage excluded products for the current accessory when they are in different items.
			// Set excluded products button status.
			VerifyOtherExcludedProduct( hItem, hParentItem, setAccGroup );
		}
	}

	// HYS-1992: We look we have item under the set accessory
	bool bIsForSmartDpC = false;
	if( (NULL != dynamic_cast<CDB_SmartControlValve*>(pclProduct))
		&& (0 == StringCompare( _T( "SMARTDPCTYPE" ), pclProduct->GetTypeID() )) )
	{
		bIsForSmartDpC = true;
	}

	if( NULL != pclTreeItemAccessoryClicked->m_pclAccessory && false == pclTreeItemAccessoryClicked->m_pclAccessory->IsDeleted()
		&& true == bIsForSmartDpC )
	{
		CDB_RuledTable* pclRuledTable = NULL;
		pclRuledTable = dynamic_cast<CDB_RuledTable*>(pclTreeItemAccessoryClicked->m_pclAccessory->GetAccessoriesGroupIDPtr().MP);

		if( NULL != pclRuledTable )
		{
			m_ItemProductSelected.m_hTreeItemSub = hItem;
			CleanVectorOfSelectedAccessories( &m_vecSubProductAccessorySelected, true );
			// Fill builin accessories
			GetBuiltInAccessoriesForAccessory( pclTreeItemAccessoryClicked->m_hTreeItem );
		}
	}

	// Reset the list focus and fill the list box just below the tree with short infos.
	m_ListBoxFocus.DeleteAllItems();
	if( true == bIsForSmartDpC )
	{
		bool bIsByPair = pclTreeItemAccessoryClicked->m_pclRuledTable->IsByPair( pclTreeItemAccessoryClicked->m_pclAccessory->GetIDPtr().ID );
		FillDpSAccessoryDetails( pclTreeItemAccessoryClicked->m_pclAccessory, &m_ListBoxFocus, true, bIsByPair );
	}
	else
	{
		FillAccessoryDetails( pclTreeItemAccessoryClicked->m_pclAccessory, &m_ListBoxFocus );
	}
}

void CDlgDirSel::ClickAdapter( HTREEITEM hItemSelected )
{
	// Identify the parent.
	// Remark: PAY ATTENTION 'm_ItemProductSelected' is not necessarily the product linked to the accessory. User can click on an accessory on an other
	//         product. This is the reason why we need to check below the correct parent of the accessory clicked.
	HTREEITEM hItemCV = _GetParentProduct( hItemSelected );

	if( NULL != m_ItemAdapterSelected.m_hTreeItem && hItemSelected == m_ItemAdapterSelected.m_hTreeItem )
	{
		// User clicks on a adapter to unselect it.

		if( NULL != m_ItemActuatorSelected.m_hTreeItem )
		{
			// If actuator is selected, we unselect only the adapter and don't change status of any other adapter.
			m_TreeProducts.SetItemImage( m_ItemAdapterSelected.m_hTreeItem, CheckboxNotSelected, CheckboxNotSelected );
		}
		else
		{
			// If there is no actuator selected, we can enable all adapters.
			_UnselectAllAdapters( m_TreeProducts.GetParentItem( hItemSelected ) );
		}

		m_ItemAdapterSelected.Clean();
	}
	else
	{
		// User clicks on a adapter to select it.

		// Verify first if checkbox is not disabled.
		int iImage;
		int iImageSelected;

		if( FALSE == m_TreeProducts.GetItemImage( hItemSelected, iImage, iImageSelected ) )
		{
			return;
		}

		if( CheckboxSelectedDisable == iImage || CheckboxNotSelectedDisable == iImage )
		{
			return;
		}

		// +----------------+                                         +-----------------------+
		// | Parent exist ? |--------------------------------------> | Is actuator selected ? | -----+
		// +----------------+                 no                      +-----------------------+  no  |
		//      |                                                                 |                  |
		//      | yes                                                             | yes              |
		//      |                                                                 |                  |
		// +------------------+       +------------------------+       +------------------+          |
		// | Is same parent ? | ----> | _CleanCurentProduct()  | <---- | Is same parent ?  |         |
		// +------------------+  no   +------------------------+   no  +------------------+          |
		//      |                                 |                               | yes              |
		//      | yes                             |                               |                  |
		//      |                                 |                               |                  |
		// +------------------+                   |                               |                  |
		// | _SelectAdapter() | <-----------------+<------------------------------+<-----------------+
		// +------------------+

		// If parent exist ?
		if( NULL != m_ItemProductSelected.m_hTreeItem )
		{
			// Is same parent ?
			if( hItemCV != m_ItemProductSelected.m_hTreeItem )
			{
				// Clean all the previous selection.
				_CleanCurrentProduct();
				// HYS-1007: Select the new Parent
				ClickProduct( hItemCV );
			}
		}
		else
		{
			// Is actuator selected ?
			if( NULL != m_ItemActuatorSelected.m_hTreeItem )
			{
				// Is same parent as adapter ?
				if( hItemCV !=  _GetParentProduct( m_ItemActuatorSelected.m_hTreeItem ) )
				{
					// Clean all the previous selection.
					_CleanCurrentProduct();
				}
			}
			// HYS-1007: If adapter is alone select the parent product item
			else
			{
				ClickProduct( hItemCV );
			}
		}

		_SelectAdapter( hItemSelected );
	}

	CTreeItemAdapter *pclTreeItemAdapterClicked = dynamic_cast<CTreeItemAdapter *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemSelected ) );

	if( NULL != pclTreeItemAdapterClicked )
	{
		// Reset the list focus and fill the list box just below the tree with short infos.
		m_ListBoxFocus.DeleteAllItems();
		FillAdapterDetails( pclTreeItemAdapterClicked->m_pclAdapter, &m_ListBoxFocus );
	}
}

void CDlgDirSel::ClickActuator( HTREEITEM hItemSelected )
{
	// Identify the parent.
	// Remark: PAY ATTENTION 'm_ItemProductSelected' is not necessarily the product linked to the accessory. User can click on an accessory on an other
	//         product. This is the reason why we need to check below the correct parent of the accessory clicked.
	HTREEITEM hItemCV = _GetParentProduct( hItemSelected );
	
	if( m_ItemActuatorSelected.m_hTreeItem != hItemSelected )
	{
		// User clicks on a not yet selected actuator to select it.

		// Unselect previous adapter if exist.
		if( NULL != m_ItemAdapterSelected.m_hTreeItem )
		{
			_UnselectAllAdapters( m_TreeProducts.GetParentItem( m_ItemAdapterSelected.m_hTreeItem ) );
			m_ItemAdapterSelected.Clean();
		}

		// Unselect previous actuator and its accessories if exist.
		if( NULL != m_ItemActuatorSelected.m_hTreeItem )
		{
			m_TreeProducts.SetItemImage( m_ItemActuatorSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
			m_ItemActuatorSelected.Clean();
			CleanVectorOfSelectedAccessories( &m_vecActuatorAccessorySelected );
		}

		// Select the new actuator.
		m_TreeProducts.SetItemImage( hItemSelected, RadioSelected, RadioSelected );
		m_ItemActuatorSelected.m_hTreeItem = hItemSelected;

		// Do the built-in solution.
		if( NULL != hItemCV )
		{
			GetBuiltInAccessories( hItemSelected, hItemCV );
		}

		// If it's not a set selection...
		if( false == m_bWorkingForControlSet )
		{
			// If it's not the same parent...
			if( NULL != hItemCV && hItemCV != m_ItemProductSelected.m_hTreeItem )
			{
				if( NULL != m_ItemProductSelected.m_hTreeItem )
				{
					// If use clicks on an actuator that are not under the same valve, we need to reset previous valve and its accessories.
					m_TreeProducts.SetItemImage( m_ItemProductSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
					m_ItemProductSelected.Clean();
					CleanVectorOfSelectedAccessories( &m_vecProductAccessorySelected );
					CleanVectorOfSelectedAccessories( &m_vecSubProductAccessorySelected );
				}
				
				// HYS-1007: If actuator is alone and has a parent select the parent product item
				ClickProduct( hItemCV );
			}
		}
		else
		{
			if( m_ItemProductSelected.m_hTreeItem != hItemCV )
			{
				// Remark: 'false' to specify to not clean and do actuator selection because it is already done!
				ClickProduct( hItemCV, false );
			}
			
			// HYS-1381 : If we click on actuator for a set, the product image should be displayed.
			if( (HBITMAP)NULL == m_staticImg.GetBitmap() )
			{
				CTreeItemProduct *pclTreeItemProductParent = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemCV ) );
				FillImage( pclTreeItemProductParent->m_pclProduct );
			}
		}

		if( NULL != hItemCV )
		{
			_SelectAdapter();
			_CheckCrossingAccessoriesCVAndActuator( hItemCV, m_ItemActuatorSelected.m_hTreeItem, true );
		}
	}
	else
	{
		// User clicks on a selected actuator to unselect it.

		// If we are working on a set, reset the product.
		if( true == m_bWorkingForControlSet )
		{
			// Remark: 'false' to specify to not clean and do actuator selection because it is already done!
			ClickProduct( hItemCV, false );
		}
		else
		{
			// Remove adapter.
			if( NULL != m_ItemAdapterSelected.m_hTreeItem )
			{
				m_ItemAdapterSelected.Clean();
			}

			// In all cases, we reenable all checbox for adapter.
			_UnselectAllAdapters( _GetAdapterTitle( hItemCV ) );

			// Remove actuator and its accessories if exist.
			if( NULL != m_ItemActuatorSelected.m_hTreeItem )
			{
				m_TreeProducts.SetItemImage( m_ItemActuatorSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
				m_ItemActuatorSelected.Clean();
				CleanVectorOfSelectedAccessories( &m_vecActuatorAccessorySelected );
			}
		}

		if( NULL != hItemCV )
		{
			_CheckCrossingAccessoriesCVAndActuator( hItemCV, m_ItemActuatorSelected.m_hTreeItem, false );
		}
	}

	CTreeItemActuator *pclTreeItemActuatorClicked = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemSelected ) );
	
	if( NULL != pclTreeItemActuatorClicked )
	{
		// Reset the list focus and fill the list box just below the tree with short infos.
		m_ListBoxFocus.DeleteAllItems();
		FillActuatorDetails( pclTreeItemActuatorClicked->m_pclActuator, &m_ListBoxFocus );
	}
}

void CDlgDirSel::ClickIntegrated( HTREEITEM hItem )
{
	// Verify first if checkbox is not disabled.
	int iImage;
	int iImageSelected;

	if( FALSE == m_TreeProducts.GetItemImage( hItem, iImage, iImageSelected ) )
	{
		return;
	}

	if( CheckboxSelectedDisable == iImage || CheckboxNotSelectedDisable == iImage )
	{
		return;
	}

	CTreeItemIntegrated *pclTreeItemIntegratedClicked = dynamic_cast<CTreeItemIntegrated *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItem ) );

	if( NULL == pclTreeItemIntegratedClicked || NULL == pclTreeItemIntegratedClicked->m_pclIntegrated )
	{
		ASSERT_RETURN;
	}

	HTREEITEM hParentItem = _GetParentProduct( hItem );
	if( NULL == hParentItem )
	{
		ASSERT_RETURN;
	}
	bool bCleaningNeeded = false;
	CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hParentItem );
	CDB_Product *pclProduct = NULL;
	CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( pclTreeItemData );

	if( pclTreeItemProduct != NULL )
	{
		pclProduct = pclTreeItemProduct->m_pclProduct;
	}

	if( NULL == pclTreeItemData )
	{
		ASSERT_RETURN;
	}

	if( hParentItem != m_ItemProductSelected.m_hTreeItem )
	{
		bCleaningNeeded = true;
	}

	if( m_ItemIntegratedSelected.m_hTreeItem != hItem )
	{
		if( NULL != m_ItemIntegratedSelected.m_hTreeItem )
		{
			m_TreeProducts.SetItemImage( m_ItemIntegratedSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
			m_ItemIntegratedSelected.Clean();
			CleanVectorOfSelectedAccessories( &m_vecIntegratedAccessorySelected );
		}

		if( true == bCleaningNeeded )
		{
			ClickProduct( hParentItem );
			m_TreeProducts.SetItemImage( hItem, RadioSelected, RadioSelected );
			m_ItemIntegratedSelected.m_hTreeItem = hItem;

			// If it's not the same parent...
			if( NULL != hParentItem && hParentItem != m_ItemProductSelected.m_hTreeItem )
			{
				if( NULL != m_ItemProductSelected.m_hTreeItem )
				{
					m_TreeProducts.SetItemImage( m_ItemProductSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
					m_ItemProductSelected.Clean();
				}
			}
		}
		else
		{
			m_TreeProducts.SetItemImage( hItem, RadioSelected, RadioSelected );
			m_ItemIntegratedSelected.m_hTreeItem = hItem;
		}
	}
	else
	{

		// User clicks on a selected buffer vessel
		if( NULL != m_ItemIntegratedSelected.m_hTreeItem )
		{
			m_TreeProducts.SetItemImage( m_ItemIntegratedSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
			m_ItemIntegratedSelected.Clean();
			CleanVectorOfSelectedAccessories( &m_vecIntegratedAccessorySelected );
		}
	}

	if( NULL != pclTreeItemIntegratedClicked )
	{
		// Reset the list focus and fill the list box just below the tree with short infos.
		m_ListBoxFocus.DeleteAllItems();
		FillProductDetails( pclTreeItemIntegratedClicked->m_pclIntegrated, &m_ListBoxFocus );
		FillImage( pclTreeItemIntegratedClicked->m_pclIntegrated );
	}
}

void CDlgDirSel::CleanVectorOfData( int iDataType )
{
	if( iDataType < CTreeItemData::DataType::Category || iDataType > CTreeItemData::DataType::All )
	{
		return;
	}

	for( int iLoop = 0; iLoop < (int)m_vecItemDataInTreeList.size(); )
	{
		if( NULL != m_vecItemDataInTreeList[iLoop] && 
			( CTreeItemData::DataType::All == (CTreeItemData::DataType)iDataType || 
			( CTreeItemData::DataType::AllMiddle == (CTreeItemData::DataType)iDataType && 0 != ( m_vecItemDataInTreeList[iLoop]->m_eDataType & CTreeItemData::DataType::AllMiddle ) ) ||
			  m_vecItemDataInTreeList[iLoop]->m_eDataType == (CTreeItemData::DataType)iDataType ) )
		{
			delete m_vecItemDataInTreeList[iLoop];
			m_vecItemDataInTreeList.erase( m_vecItemDataInTreeList.begin() + iLoop );
		}
		else
		{
			iLoop++;
		}
	}
}

void CDlgDirSel::CleanVectorOfSelectedAccessories( std::vector<CAccessorySelected> *pvecAccessorySelected, bool bKeepDisabledState, int iButtonType )
{
	// HYS-1992: Consider radio and checkbox. If button type = 0 we clean all. 
	// Else we clean only same item type
	bool bButtonImageCheckBox = true;

	if( RadioSelected == iButtonType || RadioNotSelected == iButtonType || RadioSelectedDisable == iButtonType || RadioNotSelectedDisable == iButtonType )
	{
		bButtonImageCheckBox = false;
	}

	for( int i = 0; i < (int)pvecAccessorySelected->size(); i++ )
	{
		int iImageToSet = CheckboxNotSelected;
		int iImage;
		int iSelectedImage;
		bool bImageCanBeSet = false;
		m_TreeProducts.GetItemImage( (*pvecAccessorySelected)[i].m_hTreeItem, iImage, iSelectedImage );

		if( true == bKeepDisabledState )
		{
			if( CheckboxSelectedDisable == iImage )
			{
				iImageToSet = CheckboxNotSelectedDisable;
			}
			else if( RadioSelectedDisable == iImage )
			{
				iImageToSet = RadioNotSelectedDisable;
			}
		}

		if( RadioSelected == iImage )
		{
			iImageToSet = RadioNotSelected;
		}

		if( 0 == iButtonType )
		{
			m_TreeProducts.SetItemImage( (*pvecAccessorySelected)[i].m_hTreeItem, iImageToSet, iImageToSet );
			
			//	HYS-2031: Check excluded product to be also clear.
			std::vector<CData *> setAccGroup;
			CDB_RuledTable *pclRuledTable = NULL;
			HTREEITEM hParentProductItem = _GetParentProduct( (*pvecAccessorySelected)[i].m_hTreeItem );
			CTreeItemProduct *pclParentItemProduct = (CTreeItemProduct*)(m_TreeProducts.GetItemData( hParentProductItem ));
			if( NULL != hParentProductItem && NULL != pclParentItemProduct )
			{
				pclRuledTable = dynamic_cast<CDB_RuledTable *>(pclParentItemProduct->m_pclProduct->GetAccessoriesGroupIDPtr().MP);
				if( NULL != pclRuledTable )
				{
					// Accessories group exist.
					pclRuledTable->GetVector( CDB_RuledTable::Excluded, (*pvecAccessorySelected)[i].m_pclAccessory, &setAccGroup );
				}

				// Set excluded products button status.
				VerifyOtherExcludedProduct( (*pvecAccessorySelected)[i].m_hTreeItem, hParentProductItem, setAccGroup );
			}
		}
		else
		{
			if( true == bButtonImageCheckBox && ((CheckboxSelected == iImage || CheckboxNotSelected == iImage
												   || CheckboxSelectedDisable == iImage || CheckboxNotSelectedDisable == iImage)) )
			{
				bImageCanBeSet = true;
			}
			else if( false == bButtonImageCheckBox && ((RadioSelected == iImage || RadioNotSelected == iImage
														 || RadioSelectedDisable == iImage || RadioNotSelectedDisable == iImage)) )
			{
				bImageCanBeSet = true;
			}

			if( true == bImageCanBeSet )
			{
				std::vector<CAccessorySelected>::iterator iterator = pvecAccessorySelected->begin() + i;
				m_TreeProducts.SetItemImage( (*pvecAccessorySelected)[i].m_hTreeItem, iImageToSet, iImageToSet );
				pvecAccessorySelected->erase( iterator );
			}
		}
	}

	if( 0 == iButtonType )
	{
		pvecAccessorySelected->clear();
	}
}

void CDlgDirSel::CleanOneLevel( CExtTreeCtrl *pTreeCtrl, HTREEITEM hItem )
{
	// Clean one level of the products.
	while( NULL != hItem )
	{
		HTREEITEM Temp = pTreeCtrl->GetNextItem( hItem, TVGN_NEXT );

		if( NULL == pTreeCtrl->GetChildItem( hItem ) )
		{
			pTreeCtrl->DeleteItem( hItem );
		}

		// Jump to the next item member.
		hItem = Temp;
	}
}

void CDlgDirSel::RemoveEmptyNode( CExtTreeCtrl *pTreeCtrl, int iToplevel, HTREEITEM Hitem, int iCurLevel )
{
	HTREEITEM hCurrentItem = NULL;

	if( NULL == Hitem )
	{
		hCurrentItem = pTreeCtrl->GetRootItem();
		iCurLevel = 0;
	}
	else
	{
		hCurrentItem = Hitem;
		iCurLevel++;
	}
	
	while( NULL != hCurrentItem )
	{
		HTREEITEM HitemChild = pTreeCtrl->GetChildItem( hCurrentItem );
	
		if( NULL != HitemChild )
		{
			// Children exist clean it first.
			RemoveEmptyNode( pTreeCtrl, iToplevel, HitemChild, iCurLevel );
		}
		else
		{
			if( 0 == iToplevel || iCurLevel < iToplevel )
			{  
				// Current level could be removed.
				HitemChild = pTreeCtrl->GetChildItem( hCurrentItem );
				CString str = pTreeCtrl->GetItemText( hCurrentItem );
				
				if( NULL == HitemChild )
				{
					// No child anymore!
					if( NULL == pTreeCtrl->GetItemData( hCurrentItem ) || pTreeCtrl == &m_TreeProdCateg )
					{
						// No Data
						// Remove it
						//Next Sibling
						HTREEITEM HitemToDelete = hCurrentItem;
						CString str = pTreeCtrl->GetItemText( hCurrentItem );
						hCurrentItem = pTreeCtrl->GetNextItem( hCurrentItem, TVGN_NEXT );
						pTreeCtrl->DeleteItem( HitemToDelete );
						continue;
					}
				}
			}
		}

		hCurrentItem = pTreeCtrl->GetNextItem( hCurrentItem, TVGN_NEXT );
	}
}

void CDlgDirSel::ResetFocus()
{
	// Delete all items first.
	m_ListBoxFocus.DeleteAllItems();
}

void CDlgDirSel::GetBuiltInAccessories( HTREEITEM hItemProd, HTREEITEM hItemProduct )
{
	CDB_Product *pclProduct = NULL;
	CDB_Actuator *pclActuator = NULL;
	CDB_RuledTable *pclRuledTable = NULL;

	bool bForProduct = ( NULL == hItemProduct ) ? true : false;
	CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hItemProd );

	if( NULL == pclTreeItemData )
	{
		ASSERT_RETURN;
	}

	for( int iLoop = 0; iLoop < 2; iLoop++ )
	{
		HTREEITEM hItemAcc = NULL;

		if( true == bForProduct )
		{
			if( iLoop > 0 )
			{
				return;
			}
			
			CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( pclTreeItemData );
			
			if( NULL == pclTreeItemProduct || NULL == pclTreeItemProduct->m_pclProduct )
			{
				continue;
			}
			
			pclProduct = pclTreeItemProduct->m_pclProduct;

			if( NULL == pclProduct->GetAccessoriesGroupIDPtr().MP )
			{
				continue;
			}

			pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclProduct->GetAccessoriesGroupIDPtr().MP );
			
			// Found tree branch where are located accessories for the product!
			// Look inside the product branch.
			HTREEITEM hItemAccTitle = _GetAccessoryTitle( hItemProd );

			if( NULL == hItemAccTitle )
			{
				continue;
			}
			
			// Get the first accessory under the title.
			hItemAcc = m_TreeProducts.GetChildItem( hItemAccTitle );
		}
		else
		{
			// Built-in accessories for actuators.

			// Found associated product that contains this actuator.
			DWORD_PTR dwPtr = m_TreeProducts.GetItemData( hItemProduct );
			CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemProduct ) );
			
			if( NULL == pclTreeItemProduct || NULL == pclTreeItemProduct->m_pclProduct )
			{
				continue;
			}
			
			pclProduct = pclTreeItemProduct->m_pclProduct;

			if( 0 == iLoop )
			{
				// First loop: we directly take accessories for group defined in 'CDB_Actuator'.
				CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( pclTreeItemData );

				if( NULL == pclTreeItemActuator || NULL == pclTreeItemActuator->m_pclActuator )
				{
					ASSERT_RETURN;
				}
				
				if( NULL == pclTreeItemActuator->m_pclActuator->GetAccessoriesGroupIDPtr().MP )
				{
					continue;
				}
				
				pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTreeItemActuator->m_pclActuator->GetAccessoriesGroupIDPtr().MP );

				if( NULL == pclRuledTable )
				{
					continue;
				}
			}
			else
			{
				// Second loop: we take accessories that belongs to a set.

				// If we are not working for a set, stop here!
				if( false == m_bWorkingForControlSet )
				{
					return;
				}
				
				// If there is accessories from set check for them too.
				CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclProduct );

				if( NULL == pclControlValve )
				{
					continue;
				}
				
				CTableSet *pTabSet = pclControlValve->GetTableSet();

				if( NULL == pTabSet )
				{
					continue;
				}
				
				std::set<CDB_Set *> pSetTable;
				std::set<CDB_Set *>::const_iterator IterpSetTable;

				if( pTabSet->FindCompatibleSet( &pSetTable, pclProduct->GetIDPtr().ID, pclActuator->GetIDPtr().ID, CTADatabase::FilterSelection::ForDirSel ) )
				{
					IterpSetTable = pSetTable.begin();
					pclRuledTable = dynamic_cast<CDB_RuledTable *>( (*IterpSetTable)->GetAccGroupIDPtr().MP );
				}
			}
			
			// Title 'Compatible Accessories'.
			HTREEITEM hItemAccTitle = m_TreeProducts.GetChildItem( hItemProd );

			// Accessory Item.
			hItemAcc = m_TreeProducts.GetChildItem( hItemAccTitle );		// Fist child
		}
		
		// No accessories.
		if( NULL == hItemAcc )
		{
			continue;
		}
		
		// Sanity.
		if( NULL == pclProduct )
		{
			ASSERT_RETURN;
		}
		
		if( NULL != pclRuledTable )
		{
			std::vector<CData *> vecBuiltInAccessories;
			int nCount = pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			HTREEITEM hStartItem = hItemAcc;

			// Run all built-in accessories.
			for( int i = 0; i < nCount; i++ )
			{
				CDB_Product *pclBuiltInAccessory = dynamic_cast<CDB_Product *>( vecBuiltInAccessories.at( i ) );

				// Run all accessories in the tree.
				for( hItemAcc = hStartItem; hItemAcc != 0; hItemAcc = m_TreeProducts.GetNextItem( hItemAcc, 1 ) )
				{
					CTreeItemAccessory *pclTreeItemAccessory = dynamic_cast<CTreeItemAccessory *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemAcc ) );

					if( NULL == pclTreeItemAccessory || NULL == pclTreeItemAccessory->m_pclAccessory || false == pclTreeItemAccessory->m_pclAccessory->IsAnAccessory() )
					{
						continue;
					}

					if( pclTreeItemAccessory->m_pclAccessory == pclBuiltInAccessory )
					{
						CAccessorySelected clItemAccessoryToSave;
						clItemAccessoryToSave.m_pclAccessory = pclTreeItemAccessory->m_pclAccessory;
						clItemAccessoryToSave.m_eAccessoryType = pclTreeItemAccessory->m_eAccessoryType;
						clItemAccessoryToSave.m_pclRuledTable = pclTreeItemAccessory->m_pclRuledTable;
						clItemAccessoryToSave.m_hTreeItem = hItemAcc;
						clItemAccessoryToSave.m_bCheck = true;
						clItemAccessoryToSave.m_bEnable = false;
						m_TreeProducts.SetItemImage( hItemAcc, CheckboxSelectedDisable, CheckboxSelectedDisable );

						// HYS-1483 : We should look bForProduct variable not pclProduct
						if( true == bForProduct )
						{
							// To check if 'hItemProd' is the main product or a sub product.
							// Remark: it's typically for smart differential pressur controller + sets.
							HTREEITEM hParentItem = _GetParentProduct( hItemProd );

							if( NULL == hParentItem )
							{
								m_vecProductAccessorySelected.push_back( clItemAccessoryToSave );
							}
							else
							{
								m_vecSubProductAccessorySelected.push_back( clItemAccessoryToSave );
							}
						}
						else
						{
							m_vecActuatorAccessorySelected.push_back( clItemAccessoryToSave );
						}

						// HYS-2031: Get all excluded products for the current accessory what ever their parent in the tree.
						std::vector<CData *> setAccGroup;

						if( NULL != pclRuledTable )
						{
							pclRuledTable->GetVector( CDB_RuledTable::Excluded, pclTreeItemAccessory->m_pclAccessory, &setAccGroup );
						}

						GetLinkedAccessories( hItemAcc, hItemProd, bForProduct, CDB_RuledTable::Excluded );
						GetLinkedAccessories( hItemAcc, hItemProd, bForProduct, CDB_RuledTable::Implicated );
						
						VerifyOtherExcludedProduct( hItemAcc, hItemProd, setAccGroup );
					}
				}
			}
		}
	}
}

void CDlgDirSel::GetBuiltInAccessoriesForAccessory( HTREEITEM hItemProd )
{
	CDB_Product *pclProduct = NULL;
	CDB_RuledTable *pclRuledTable = NULL;

	CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hItemProd );

	if( NULL == pclTreeItemData )
	{
		ASSERT_RETURN;
	}

	for( int iLoop = 0; iLoop < 2; iLoop++ )
	{
		HTREEITEM hItemAcc = NULL;

		if( iLoop > 0 )
		{
			return;
		}
			
		CTreeItemAccessory *pclTreeItemProduct = dynamic_cast<CTreeItemAccessory *>( pclTreeItemData );
			
		if( NULL == pclTreeItemProduct || NULL == pclTreeItemProduct->m_pclAccessory )
		{
			continue;
		}
			
		pclProduct = pclTreeItemProduct->m_pclAccessory;

		if( NULL == pclProduct->GetAccessoriesGroupIDPtr().MP )
		{
			continue;
		}

		pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclProduct->GetAccessoriesGroupIDPtr().MP );
			
		// Found tree branch where are located accessories for the product!
		// Look inside the product branch.
		HTREEITEM hItemAccTitle = _GetAccessoryTitle( hItemProd );

		if( NULL == hItemAccTitle )
		{
			continue;
		}
			
		// Get the first accessory under the title.
		hItemAcc = m_TreeProducts.GetChildItem( hItemAccTitle );
		
		// No accessories.
		if( NULL == hItemAcc )
		{
			continue;
		}
		
		// Sanity.
		if( NULL == pclProduct )
		{
			ASSERT_RETURN;
		}
		
		if( NULL != pclRuledTable )
		{
			std::vector<CData *> vecBuiltInAccessories;
			int nCount = pclRuledTable->GetBuiltIn( &vecBuiltInAccessories );
			HTREEITEM hStartItem = hItemAcc;

			// Run all built-in accessories.
			for( int i = 0; i < nCount; i++ )
			{
				CDB_Product *pclBuiltInAccessory = dynamic_cast<CDB_Product *>( vecBuiltInAccessories.at( i ) );

				// Run all accessories in the tree.
				for( hItemAcc = hStartItem; hItemAcc != 0; hItemAcc = m_TreeProducts.GetNextItem( hItemAcc, 1 ) )
				{
					CTreeItemAccessory *pclTreeItemAccessory = dynamic_cast<CTreeItemAccessory *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemAcc ) );

					if( NULL == pclTreeItemAccessory || NULL == pclTreeItemAccessory->m_pclAccessory || false == pclTreeItemAccessory->m_pclAccessory->IsAnAccessory() )
					{
						continue;
					}

					if( pclTreeItemAccessory->m_pclAccessory == pclBuiltInAccessory )
					{
						CAccessorySelected clItemAccessoryToSave;
						clItemAccessoryToSave.m_pclAccessory = pclTreeItemAccessory->m_pclAccessory;
						clItemAccessoryToSave.m_eAccessoryType = pclTreeItemAccessory->m_eAccessoryType;
						clItemAccessoryToSave.m_pclRuledTable = pclTreeItemAccessory->m_pclRuledTable;
						clItemAccessoryToSave.m_hTreeItem = hItemAcc;
						clItemAccessoryToSave.m_bCheck = true;
						clItemAccessoryToSave.m_bEnable = false;
						m_TreeProducts.SetItemImage( hItemAcc, CheckboxSelectedDisable, CheckboxSelectedDisable );

						// We should look bForProduct variable not pclProduct

						// To check if 'hItemProd' is the main product or a sub product.
						// Remark: it's typically for smart differential pressur controller + sets.
						HTREEITEM hParentItem = _GetParentProduct( hItemProd );

						if( NULL == hParentItem )
						{
							m_vecProductAccessorySelected.push_back( clItemAccessoryToSave );
						}
						else
						{
							m_vecSubProductAccessorySelected.push_back( clItemAccessoryToSave );
						}

						// HYS-2031: Get all excluded products for the current accessory what ever their parent in the tree.
						std::vector<CData *> setAccGroup;

						if( NULL != pclRuledTable )
						{
							pclRuledTable->GetVector( CDB_RuledTable::Excluded, pclTreeItemAccessory->m_pclAccessory, &setAccGroup );
						}

						GetLinkedAccessories( hItemAcc, hItemProd, true, CDB_RuledTable::Excluded );
						GetLinkedAccessories( hItemAcc, hItemProd, true, CDB_RuledTable::Implicated );

						VerifyOtherExcludedProduct( hItemAcc, hItemProd, setAccGroup );
					}
				}
			}
		}
	}
}

void CDlgDirSel::GetLinkedAccessories( HTREEITEM hItemSelected, HTREEITEM hParentItem, bool bForProduct, CDB_RuledTable::MMapName rule )
{
	CTreeItemAccessory *pclTreeItemMainAccessory = dynamic_cast<CTreeItemAccessory *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItemSelected ) );

	if( NULL == pclTreeItemMainAccessory || NULL == pclTreeItemMainAccessory->m_pclAccessory || false == pclTreeItemMainAccessory->m_pclAccessory->IsAnAccessory() )
	{
		ASSERT_RETURN;
	}

	int nCount = 0;
	std::vector<CData *> setAccSeries;
	std::vector<CData *> setAccGroup;
	CData *pItemDataSelected = pclTreeItemMainAccessory->m_pclAccessory;
	
	// Verify accessory series rule.
	CDB_RuledTable *pFamRtab = (CDB_RuledTable *)( TASApp.GetpTADB()->Get( _T("ACCSERIESRULES") ).MP );
	VERIFY( NULL != pFamRtab );

	if( NULL != pFamRtab && NULL != pclTreeItemMainAccessory->m_pclAccessory->GetSeriesIDPtr().MP )
	{
		pFamRtab->GetExcluded( pclTreeItemMainAccessory->m_pclAccessory->GetSeriesIDPtr().MP, &setAccSeries );
	}
	
	// Verify accessories group.
	if( NULL != hParentItem )
	{
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hParentItem );
		CDB_RuledTable *pclRuledTable = NULL;

		if( CTreeItemData::DataType::Product == pclTreeItemData->m_eDataType )
		{
			// Parent is a product.
			CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( pclTreeItemData );

			if( NULL != pclTreeItemProduct && NULL != pclTreeItemProduct->m_pclProduct  )
			{
				pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTreeItemProduct->m_pclProduct->GetAccessoriesGroupIDPtr().MP );
			}
		}
		else if( CTreeItemData::DataType::Integrated == pclTreeItemData->m_eDataType )
		{
			CTreeItemIntegrated *pclTreeItemIntegrated = dynamic_cast<CTreeItemIntegrated *>( pclTreeItemData );

			if( NULL != pclTreeItemIntegrated && NULL != pclTreeItemIntegrated->m_pclIntegrated )
			{
				pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTreeItemIntegrated->m_pclIntegrated->GetAccessoriesGroupIDPtr().MP );
			}
		}
		else
		{
			// Parent is an actuator.
			CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( pclTreeItemData );

			if( NULL != pclTreeItemActuator && NULL != pclTreeItemActuator->m_pclActuator )
			{
				pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclTreeItemActuator->m_pclActuator->GetAccessoriesGroupIDPtr().MP );
			}
		}
		
		if( NULL != pclRuledTable )
		{
			// Accessories group exist.
			pclRuledTable->GetVector( rule, pclTreeItemMainAccessory->m_pclAccessory, &setAccGroup );
		}
	}
	
	// Check 'Series' and 'Group' rules exclusions/implications.
	std::vector<CData *> *pSet = NULL;

	for( int j = 0; j < 2; j++ )
	{
		// For the first pass, we check series, for the second one we check group.
		pSet = ( 0 == j ) ? &setAccSeries : &setAccGroup;
		
		for( unsigned int i = 0; i < pSet->size(); i++ )
		{
			// For each item into the tree.
			HTREEITEM hChildChildItem = m_TreeProducts.GetChildItem( m_TreeProducts.GetParentItem( hItemSelected ) );	

			for( hChildChildItem; hChildChildItem != 0; hChildChildItem = m_TreeProducts.GetNextItem( hChildChildItem, 1 ) )
			{
				CTreeItemAccessory *pclTreeItemCurrentAccessory = dynamic_cast<CTreeItemAccessory *>( (CTreeItemData *)m_TreeProducts.GetItemData( hChildChildItem ) );

				if( NULL == pclTreeItemCurrentAccessory || NULL == pclTreeItemCurrentAccessory->m_pclAccessory || false == pclTreeItemCurrentAccessory->m_pclAccessory->IsAnAccessory() )
				{
					continue;
				}
				
				CData *pData = NULL;

				if( 0 == j )
				{
					// 'pData' points on a series.
					pData = (CData *)( pclTreeItemCurrentAccessory->m_pclAccessory->GetSeriesIDPtr().MP );
				}
				else
				{
					// 'pData' points on an accessory.
					pData = pclTreeItemCurrentAccessory->m_pclAccessory;
				}

				if( pData == pSet->at( i ) )
				{
					CAccessorySelected clItemAccessoryToSave;
					clItemAccessoryToSave.m_pclAccessory = pclTreeItemCurrentAccessory->m_pclAccessory;
					// Save pointer on the accessory on which current one is linked.
					clItemAccessoryToSave.m_pclLinkToAccessory = pclTreeItemMainAccessory->m_pclAccessory;
					clItemAccessoryToSave.m_eAccessoryType = pclTreeItemCurrentAccessory->m_eAccessoryType;
					clItemAccessoryToSave.m_pclRuledTable = pclTreeItemCurrentAccessory->m_pclRuledTable;
					clItemAccessoryToSave.m_hTreeItem = hChildChildItem;
					
					if( 0 == j )
					{
						// If we are working on series, we disabled all accessories that belongs to a series that is excluded from the 
						// current accessory selected.
						clItemAccessoryToSave.m_bCheck = false;
						clItemAccessoryToSave.m_bEnable = false;
						m_TreeProducts.SetItemImage( hChildChildItem, CheckboxNotSelectedDisable, CheckboxNotSelectedDisable );
					}
					else
					{
						// If we are working on group, we enabled/disabled depending on 'rule' argument.
						clItemAccessoryToSave.m_bCheck = ( CDB_RuledTable::MMapName::Implicated == rule ) ? true : false;
						clItemAccessoryToSave.m_bEnable = false;
						int iImage = ( true == clItemAccessoryToSave.m_bCheck ) ? CheckboxSelectedDisable : CheckboxNotSelectedDisable;
						m_TreeProducts.SetItemImage( hChildChildItem, iImage, iImage );
					}
					
					if( true == bForProduct )
					{
						// To check if 'hParentItem' is the main product or a sub product.
						// Remark: it's typically for smart differential pressur controller + sets.
						HTREEITEM hGrandParentItem = _GetParentProduct( hParentItem );

						if( NULL == hGrandParentItem )
						{
							m_vecProductAccessorySelected.push_back( clItemAccessoryToSave );
						}
						else
						{
							m_vecSubProductAccessorySelected.push_back( clItemAccessoryToSave );
						}
					}
					else if( NULL != dynamic_cast<CTreeItemIntegrated *>( (CTreeItemData *)m_TreeProducts.GetItemData( hParentItem ) ) )
					{
						m_vecIntegratedAccessorySelected.push_back( clItemAccessoryToSave );
					}
					else
					{
						m_vecActuatorAccessorySelected.push_back( clItemAccessoryToSave );
					}
				}
			}	
		}
	}

	m_TreeProducts.Invalidate();
}

void CDlgDirSel::RemoveLinkedAccessories( HTREEITEM hItem, bool bForProduct )
{
	CTreeItemAccessory *pclTreeItemMainAccessory = dynamic_cast<CTreeItemAccessory *>( (CTreeItemData *)m_TreeProducts.GetItemData( hItem ) );

	if( NULL == pclTreeItemMainAccessory || NULL == pclTreeItemMainAccessory->m_pclAccessory || false == pclTreeItemMainAccessory->m_pclAccessory )
	{
		ASSERT_RETURN;
	}
	
	std::vector<CAccessorySelected> *pvecAccessorySelected = NULL;
	pvecAccessorySelected = ( true == bForProduct ) ? &m_vecProductAccessorySelected : &m_vecActuatorAccessorySelected;
	
	// Scan all implicated/excluded accessories.
	for( int i = (int)pvecAccessorySelected->size() - 1; i >= 0; i-- )
	{
		if( NULL == (*pvecAccessorySelected)[i].m_pclLinkToAccessory )
		{
			continue;
		}

		// Found correct disabled accessory.
		if( (*pvecAccessorySelected)[i].m_pclLinkToAccessory != pclTreeItemMainAccessory->m_pclAccessory )
		{
			continue;
		}
		
		// Found an accessory 'i' locked by 'pclTreeItemMainAccessory'.
		// Verify if this accessory is not locked by another accessory.
		bool bAnotherLockerExist = false;
		
		for( int j = (int)pvecAccessorySelected->size() - 1; j >= 0 && false == bAnotherLockerExist; j-- )
		{
			if( j == i )
			{
				continue;
			}
			
			if( (*pvecAccessorySelected)[j].m_hTreeItem == (*pvecAccessorySelected)[i].m_hTreeItem )	// Accessory exist with another locker
			{
				bAnotherLockerExist = true;
			}
		}
		
		if( false == (*pvecAccessorySelected)[i].m_bEnable )
		{
			if( false == bAnotherLockerExist )
			{
				m_TreeProducts.SetItemImage( (*pvecAccessorySelected)[i].m_hTreeItem, CheckboxNotSelected, CheckboxNotSelected );
			}

			pvecAccessorySelected->erase( pvecAccessorySelected->begin() + i );
		}
	}
}

bool CDlgDirSel::ConfirmSelection( CData *pSel, CString strTabID )
{
	try
	{
		if( NULL == m_clDirectSelectionParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clDirectSelectionParams.m_pTADS' can't be NULL.") );
		}

		CDlgConfSel dlg( &m_clDirectSelectionParams );
		dlg.Display( pSel );

		bool bReturn = false;

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clDirectSelectionParams.m_pTADS->Get( strTabID ).MP );
			VERIFY( NULL != pTab );

			if( NULL != pTab )
			{
				IDPTR idptr = pSel->GetIDPtr();
				pTab->Insert( idptr );

				m_clDirectSelectionParams.m_pTADS->SetNewIndex( strTabID, CTADatastruct::SNI_TableID );
			
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_USERSELECTIONCHANGE );	// Re-Draw RightView Selp
				bReturn = true;
			}
		}

		return bReturn;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgDirSel::ConfirmSelection'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgDirSel::VerifyOtherExcludedProduct( HTREEITEM hItemSelected, HTREEITEM hParentProductItem, std::vector<CData*>& vectOtherExcluded )
{
	// Accessory selected
	CTreeItemAccessory *pclTreeItemMainAccessory = dynamic_cast<CTreeItemAccessory *>((CTreeItemData *)m_TreeProducts.GetItemData( hItemSelected ));

	if( NULL == pclTreeItemMainAccessory || NULL == pclTreeItemMainAccessory->m_pclAccessory || false == pclTreeItemMainAccessory->m_pclAccessory->IsAnAccessory() )
	{
		ASSERT_RETURN;
	}

	// List of products to exclude
	if( 0 >= vectOtherExcluded.size() )
	{
		return;
	}

	int iImage = -1;
	int iImageSelected = -1;

	// Information if the item image is selected or not.
	m_TreeProducts.GetItemImage( hItemSelected, iImage, iImageSelected );

	// Get parent that is a product.
	HTREEITEM hParentItem = m_TreeProducts.GetParentItem( hItemSelected );

	// Loop on excluded product list to find it in the tree.
	for( unsigned int i = 0; i < vectOtherExcluded.size(); i++ )
	{
		// Run all items to find the ones that have the same parent (product) 
		for( auto &iter : m_vecItemDataInTreeList )
		{
			CTreeItemData *pclTreeItemData = dynamic_cast<CTreeItemData *>(iter);

			// Excluded products that have the same parent item have been already managed.
			if( NULL == pclTreeItemData || hParentItem == m_TreeProducts.GetParentItem( pclTreeItemData->m_hTreeItem )
				|| (NULL == dynamic_cast<CTreeItemAccessory *>(iter) && NULL == dynamic_cast<CTreeItemAdapter *>(iter)) )
			{
				continue;
			}

			if( (NULL != dynamic_cast<CTreeItemAccessory *>(iter) && NULL == dynamic_cast<CTreeItemAccessory *>(iter)->m_pclAccessory)
				|| (NULL != dynamic_cast<CTreeItemAdapter *>(iter) && NULL == dynamic_cast<CTreeItemAdapter *>(iter)->m_pclAdapter) )
			{
				continue;
			}

			// We consider only item which have the same parent product.
			if( hParentProductItem != _GetParentProduct( pclTreeItemData->m_hTreeItem ) )
			{
				continue;
			}

			CData *pData = NULL;

			// The product to exclude is an accessory.
			if( NULL != dynamic_cast<CTreeItemAccessory *>(iter) && NULL != dynamic_cast<CTreeItemAccessory *>(iter)->m_pclAccessory )
			{
				pData = dynamic_cast<CTreeItemAccessory *>(iter)->m_pclAccessory;

				if( pData == vectOtherExcluded.at( i ) )
				{
					// Set excluded item.
					int iImageExcl = (CheckboxNotSelected == iImageSelected) ? CheckboxNotSelected : CheckboxNotSelectedDisable;
					m_TreeProducts.SetItemImage( pclTreeItemData->m_hTreeItem, iImageExcl, iImageExcl );
					break;
				}
			}
			else if( NULL != dynamic_cast<CTreeItemAdapter *>(iter) && NULL != dynamic_cast<CTreeItemAdapter *>(iter)->m_pclAdapter )
			{
				// The product to exclude is an adapter
				pData = dynamic_cast<CTreeItemAdapter *>(iter)->m_pclAdapter;

				if( pData == vectOtherExcluded.at( i ) )
				{
					if( m_ItemAdapterSelected.m_hTreeItem == pclTreeItemData->m_hTreeItem )
					{
						// If the adapter is selected, call ClickAdapter to unselect it and enable other adapters.
						ClickAdapter( iter->m_hTreeItem );
					}

					// Set excluded item.
					int iImageExcl = (CheckboxNotSelected == iImageSelected) ? CheckboxNotSelected : CheckboxNotSelectedDisable;
					m_TreeProducts.SetItemImage( pclTreeItemData->m_hTreeItem, iImageExcl, iImageExcl );

					// If the accessory is unchecked and the apdapter included unchecked, verify if there was an actuator selected with the adapter.
					// Select the adapter according to the actuator.
					if( CheckboxNotSelected == iImageExcl && NULL != m_ItemActuatorSelected.m_hTreeItem )
					{
						_SelectAdapter( pclTreeItemData->m_hTreeItem );
					}

					break;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDlgDirSel::_IsSelectable( CDB_Thing *pclThing )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB || NULL == pclThing )
	{
		ASSERTA_RETURN( false );
	}

	if( true == m_clDirectSelectionParams.m_pTADB->MustExclude( pclThing, CTADatabase::FilterSelection::ForDirSel ) )
	{
		return false;
	}

	// HYS-1034: Don't set 'true' for the parameter because we want to show deleted product in the direct selection.
	return pclThing->IsSelectable();
}

CDB_Product *CDlgDirSel::_GetAdapterCommonToValveAndActuator( CDB_Product *pclProduct, CDB_Actuator *pclActuator )
{
	if( NULL == pclProduct || NULL == pclActuator )
	{
		return NULL;
	}

	CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclProduct );

	if( NULL == pclControlValve )
	{
		// For the moment, we have only adapters between control valves and their corresponding actuators.
		return NULL;
	}

	CDB_Product *pclAdapter = NULL;

	bool bIsFusionC = ( CString( pclControlValve->GetFamilyID() ).Compare( _T("FAM_FUSION_C") ) != 0 ) ? false : true;
	bool bIsFusionP = ( CString( pclControlValve->GetFamilyID() ).Compare( _T("FAM_FUSION_P") ) != 0 ) ? false : true;
	
	// If we are not in a selection by package, or the control valve is not a FUSION_C or a FUSION_P...
	if( false == m_bWorkingForControlSet || false == bIsFusionC || false == bIsFusionP )
	{
		CTable *pclCvAdapterTable = (CTable *)( pclControlValve->GetAdapterGroupIDPtr().MP );
		CTable *pclActuatorAdapterTable = (CTable *)( pclActuator->GetActAdapterGroupIDPtr().MP );

		if( NULL != pclCvAdapterTable && NULL != pclActuatorAdapterTable )
		{
			// Find the first adapter that fits the control valve and its actuator.
			for( IDPTR CVIDPtr = pclCvAdapterTable->GetFirst(); _T('\0') != *CVIDPtr.ID && NULL == pclAdapter; CVIDPtr = pclCvAdapterTable->GetNext( CVIDPtr.MP ) )
			{
				CDB_Product *pclCvAdapter = (CDB_Product *)( CVIDPtr.MP );

				for( IDPTR ACTIDPtr = pclActuatorAdapterTable->GetFirst(); _T('\0') != *ACTIDPtr.ID && NULL == pclAdapter; ACTIDPtr = pclActuatorAdapterTable->GetNext( ACTIDPtr.MP ) )
				{
					CDB_Product *pclActuatorAdapter = (CDB_Product *)( ACTIDPtr.MP );

					if( ( NULL != pclActuatorAdapter ) && ( pclActuatorAdapter->IsAnAccessory() ) && pclActuatorAdapter == pclCvAdapter )
					{
						pclAdapter = pclCvAdapter;
					}
				}
			}
		}
	}
	else
	{
		// Find set that contain the control valve and the actuator.
		CTableSet *pCVActTableSet = pclControlValve->GetTableSet();

		if( NULL == pCVActTableSet )
		{
			return NULL;
		}

		std::set<CDB_Set *> CVActSetArray;
		int iCDBSetCount = pCVActTableSet->FindCompatibleSet( &CVActSetArray, pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID, CTADatabase::FilterSelection::ForDirSel );
		
		if( 0 == iCDBSetCount )
		{
			return NULL;
		}

		// Normally here we MUST have only 1 set!
		ASSERT( 1 == iCDBSetCount );

		// Take the first one.
		CDB_Set *pCVActSet = *CVActSetArray.begin();
		IDPTR AdapterIDPtr = pCVActSet->GetAccGroupIDPtr();

		if( NULL != AdapterIDPtr.MP )
		{
			// Adapter is defined in the set.
			CDB_Product *pclAdapterInSet = dynamic_cast<CDB_Product *>( AdapterIDPtr.MP );

			if( NULL != pclAdapterInSet && pclAdapterInSet->IsAnAccessory() && true == _IsSelectable( pclAdapterInSet ) )
			{
				pclAdapter = pclAdapterInSet;
			}
		}
		else
		{
			CDB_RuledTable *pclCVAdapterGroup = (CDB_RuledTable *)( pclControlValve->GetAdapterGroupIDPtr().MP );

			if( NULL == pclCVAdapterGroup )
			{
				return NULL;
			}

			CDB_RuledTable *pclActuatorAdapterTable = (CDB_RuledTable *)( pclActuator->GetActAdapterGroupIDPtr().MP );

			if( NULL == pclActuatorAdapterTable )
			{
				return NULL;
			}

			// Run all adapters linked to the control valve.
			for( IDPTR CVAdapterIDPtr = pclCVAdapterGroup->GetFirst(); NULL != CVAdapterIDPtr.MP && NULL == pclAdapter; CVAdapterIDPtr = pclCVAdapterGroup->GetNext( CVAdapterIDPtr.MP ) )
			{
				// Run all adapters link on current actuator.
				for( IDPTR ActuatorAdapterIDPtr = pclActuatorAdapterTable->GetFirst(); NULL != ActuatorAdapterIDPtr.MP && NULL == pclAdapter; ActuatorAdapterIDPtr = pclActuatorAdapterTable->GetNext( ActuatorAdapterIDPtr.MP ) )
				{
					if( 0 == IDcmp( CVAdapterIDPtr.ID, ActuatorAdapterIDPtr.ID ) )
					{
						CDB_Product *pclAdapterInGroup = dynamic_cast<CDB_Product *>( CVAdapterIDPtr.MP );

						if( NULL == pclAdapterInGroup || false == pclAdapterInGroup->IsAnAccessory() )
						{
							continue;
						}

						if( false == _IsSelectable( pclAdapterInGroup ) )
						{
							continue;
						}

						pclAdapter = pclAdapterInGroup;
					}
				}
			}
		}
	}
	
	return pclAdapter;
}

void CDlgDirSel::_GetActuatorListRelatedToControlValve( CDB_ControlValve *pclControlValve, CRank *pclActuatorList, bool bWorkingForASet )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclControlValve || NULL == pclActuatorList )
	{
		return;
	}

	// Get all actuators.
	// Remark: actuators in 'ListAct' are already sorted with the good key.
	CRank ListAct;
	m_clDirectSelectionParams.m_pTADB->GetActuator( &ListAct, pclControlValve, bWorkingForASet, CTADatabase::FilterSelection::ForDirSel );
				
	// Run all actuators.
	CString str;
	LPARAM lpParam;

	for( BOOL bContinue = ListAct.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = ListAct.GetNext( str, lpParam ) )
	{
		CDB_Actuator *pclActuator = (CDB_Actuator *)lpParam;

		if( NULL != pclActuator && true == _IsSelectable( pclActuator ) )
		{
			pclActuatorList->Add( pclActuator->GetName(), pclActuator->GetOrderKey(), (LPARAM)pclActuator, false, false );
		}
	}
}

void CDlgDirSel::_GetActuatorListRelatedToShutOffValve( CDB_ShutoffValve *pclShutOffValve, CRank *pclActuatorList )
{
	if( NULL == m_clDirectSelectionParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclShutOffValve || NULL == pclActuatorList )
	{
		return;
	}

	// Get all actuators.
	// Remark: actuators in 'ListAct' are already sorted with the good key.
	CRank ListAct;
	m_clDirectSelectionParams.m_pTADB->GetSvActuatorList( &ListAct, pclShutOffValve );
				
	// Run all actuators.
	CString str;
	LPARAM lpParam;

	for( BOOL bContinue = ListAct.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = ListAct.GetNext( str, lpParam ) )
	{
		CDB_Actuator *pclActuator = (CDB_Actuator *)lpParam;

		if( NULL != pclActuator && true == _IsSelectable( pclActuator ) )
		{
			pclActuatorList->Add( pclActuator->GetName(), pclActuator->GetOrderKey(), (LPARAM)pclActuator, false, false );
		}
	}
}

void CDlgDirSel::_CleanCurrentProduct( bool bAlsoChildren, bool bProductAccessoryKeepDisabledState, bool bSubProductAccessoryKeepDisabledState )
{
	if( NULL != m_ItemProductSelected.m_hTreeItem )
	{
		m_TreeProducts.SetItemImage( m_ItemProductSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
	}

	// Case of a dp controller and balancing valve set or case of a smart differential pressure controller set.
	if( true == m_bWorkingForDpCSet && NULL != m_ItemProductSelected.m_hTreeItemSub )
	{
		m_TreeProducts.SetItemImage( m_ItemProductSelected.m_hTreeItemSub, RadioNotSelected, RadioNotSelected );
	}

	m_ItemProductSelected.Clean();

	if( true == bAlsoChildren )
	{
		CleanVectorOfSelectedAccessories( &m_vecProductAccessorySelected, bProductAccessoryKeepDisabledState );
		CleanVectorOfSelectedAccessories( &m_vecSubProductAccessorySelected, bSubProductAccessoryKeepDisabledState );

		if( NULL != m_ItemAdapterSelected.m_hTreeItem )
		{
			_UnselectAllAdapters( m_TreeProducts.GetParentItem( m_ItemAdapterSelected.m_hTreeItem ) );
			m_ItemAdapterSelected.Clean();
		}

		if( NULL != m_ItemActuatorSelected.m_hTreeItem )
		{
			m_TreeProducts.SetItemImage( m_ItemActuatorSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
			m_ItemActuatorSelected.Clean();
			CleanVectorOfSelectedAccessories( &m_vecActuatorAccessorySelected );
		}

		if( NULL != m_ItemIntegratedSelected.m_hTreeItem )
		{
			m_TreeProducts.SetItemImage( m_ItemIntegratedSelected.m_hTreeItem, RadioNotSelected, RadioNotSelected );
			m_ItemIntegratedSelected.Clean();
			CleanVectorOfSelectedAccessories( &m_vecIntegratedAccessorySelected );
		}
	}	
}

HTREEITEM CDlgDirSel::_GetParentProduct( HTREEITEM hChildItem, bool bRootParent )
{
	// Identify parent hItem; first parent with a 'ItemData' bigger than 0 ( > TitleCode::FirstTitleCode it could be for instance 
	// the 'Compatible actuator' title).
	HTREEITEM hItemParent = m_TreeProducts.GetParentItem( hChildItem );
	HTREEITEM hItem = hItemParent;

	while( NULL != hItem )
	{
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hItem );

		if( NULL != pclTreeItemData && CTreeItemData::DataType::Title != pclTreeItemData->m_eDataType )
		{
			hItemParent = hItem;

			if( false == bRootParent )
			{
				break;
			}
		}
			
		hItem = m_TreeProducts.GetParentItem( hItem );
	}

	return hItemParent;
}

void CDlgDirSel::_GetChildrenProduct( HTREEITEM hParentItem, std::vector<HTREEITEM> &vecChildren )
{
	HTREEITEM hChildItem = m_TreeProducts.GetChildItem( hParentItem );

	while( NULL != hChildItem )
	{
		vecChildren.push_back( hChildItem );
		hChildItem = m_TreeProducts.GetNextItem( hChildItem, TVGN_NEXT );
	}
}

HTREEITEM CDlgDirSel::_GetAccessoryTitle( HTREEITEM hItemProduct )
{
	// Find accessory root.
	// Remark: in fact, it is the root with the title.
	HTREEITEM hChildItem = m_TreeProducts.GetChildItem( hItemProduct );

	while( NULL != hChildItem )
	{
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hChildItem );

		if( NULL != pclTreeItemData && CTreeItemData::DataType::Title == pclTreeItemData->m_eDataType 
				&& CTreeItemTitle::TitleType::CompatibleAccessories == ( (CTreeItemTitle*)pclTreeItemData )->m_eTitleType )
		{
			break;
		}

		hChildItem = m_TreeProducts.GetNextItem( hChildItem, TVGN_NEXT );
	}

	return hChildItem;
}

HTREEITEM CDlgDirSel::_GetAdapterTitle( HTREEITEM hItemProduct )
{
	// Find adapter root.
	// Remark: in fact, it is the root with the title.
	HTREEITEM hChildItem = m_TreeProducts.GetChildItem( hItemProduct );

	while( NULL != hChildItem )
	{
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hChildItem );

		if( NULL != pclTreeItemData && CTreeItemData::DataType::Title == pclTreeItemData->m_eDataType 
				&& CTreeItemTitle::TitleType::CompatibleAdapters == ( (CTreeItemTitle*)pclTreeItemData )->m_eTitleType )
		{
			break;
		}

		hChildItem = m_TreeProducts.GetNextItem( hChildItem, TVGN_NEXT );
	}

	return hChildItem;
}

HTREEITEM CDlgDirSel::_GetActuatorTitle( HTREEITEM hItemProduct )
{
	// Find actuator root.
	// Remark: in fact, it is the root with the title.
	HTREEITEM hChildItem = m_TreeProducts.GetChildItem( hItemProduct );

	while( NULL != hChildItem )
	{
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hChildItem );

		if( NULL != pclTreeItemData && CTreeItemData::DataType::Title == pclTreeItemData->m_eDataType 
				&& CTreeItemTitle::TitleType::CompatibleActuators == ( (CTreeItemTitle*)pclTreeItemData )->m_eTitleType )
		{
			break;
		}

		hChildItem = m_TreeProducts.GetNextItem( hChildItem, TVGN_NEXT );
	}

	return hChildItem;
}

HTREEITEM CDlgDirSel::_GetMeasValveTitle( HTREEITEM hItemProduct )
{
	// Find balancing valve root.
	// Remark: in fact, it is the root with the title.
	HTREEITEM hChildItem = m_TreeProducts.GetChildItem( hItemProduct );

	while( NULL != hChildItem )
	{
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hChildItem );

		if( NULL != pclTreeItemData && CTreeItemData::DataType::Title == pclTreeItemData->m_eDataType 
				&& CTreeItemTitle::TitleType::CompatibleMeasValve == ( (CTreeItemTitle *)pclTreeItemData )->m_eTitleType )
		{
			break;
		}

		hChildItem = m_TreeProducts.GetNextItem( hChildItem, TVGN_NEXT );
	}

	return hChildItem;
}

HTREEITEM CDlgDirSel::_GetSmartDpCSetTitle( HTREEITEM hItemProduct )
{
	// Find smart differential pressure controller root.
	// Remark: in fact, it is the root with the title.
	HTREEITEM hChildItem = m_TreeProducts.GetChildItem( hItemProduct );

	while( NULL != hChildItem )
	{
		CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProducts.GetItemData( hChildItem );

		if( NULL != pclTreeItemData && CTreeItemData::DataType::Title == pclTreeItemData->m_eDataType 
				&& CTreeItemTitle::TitleType::CompatibleSmartDpCSets == ( (CTreeItemTitle *)pclTreeItemData )->m_eTitleType )
		{
			break;
		}

		hChildItem = m_TreeProducts.GetNextItem( hChildItem, TVGN_NEXT );
	}

	return hChildItem;
}

void CDlgDirSel::_SelectAdapter( HTREEITEM hAdapterItem )
{
	if( NULL == hAdapterItem && NULL == m_ItemActuatorSelected.m_hTreeItem )
	{
		return;
	}

	HTREEITEM hAdapterTitleItem = NULL;
	CDB_Product *pclAdapter = NULL;

	if( NULL == hAdapterItem )
	{
		// This case is called by 'ClickActuator'.

		// Retrieve the current control valve.
		HTREEITEM hParentItem = _GetParentProduct( m_ItemActuatorSelected.m_hTreeItem );
		CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( hParentItem ) );

		if( NULL == pclTreeItemProduct )
		{
			return;
		}

		CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( pclTreeItemProduct->m_pclProduct );
		ASSERT( NULL != pclProduct );

		if( NULL == pclProduct )
		{
			return;
		}

		hAdapterTitleItem = _GetAdapterTitle( hParentItem );

		if( NULL == hAdapterTitleItem )
		{
			return;
		}

		// Retrieve the current actuator.
		CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem ) );

		if( NULL == pclTreeItemActuator )
		{
			return;
		}

		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclTreeItemActuator->m_pclActuator );
		ASSERT( NULL != pclActuator );

		if( NULL == pclActuator )
		{
			return;
		}

		pclAdapter = _GetAdapterCommonToValveAndActuator( pclProduct, pclActuator );

		if( NULL == pclAdapter )
		{
			_UnselectAllAdapters( hAdapterTitleItem, true );
			return;
		}
	}
	else
	{
		// This case is called by 'ClickAdapter'.
		CTreeItemAdapter *pclTreeItemAdapter = dynamic_cast<CTreeItemAdapter *>( (CTreeItemData *)m_TreeProducts.GetItemData( hAdapterItem ) );

		if( NULL == pclTreeItemAdapter )
		{
			return;
		}

		pclAdapter = dynamic_cast<CDB_Product *>( pclTreeItemAdapter->m_pclAdapter );

		if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
		{
			return;
		}

		hAdapterTitleItem = m_TreeProducts.GetParentItem( hAdapterItem );

		if( NULL == hAdapterTitleItem )
		{
			return;
		}
	}

	int iImageSelected = ( true == m_bWorkingForControlSet ) ? CheckboxSelectedDisable : CheckboxSelected;
	HTREEITEM hAdapterItemInTree = m_TreeProducts.GetChildItem( hAdapterTitleItem );
	
	while( NULL != hAdapterItemInTree )
	{
		CTreeItemAdapter *pclTreeItemAdapter = dynamic_cast<CTreeItemAdapter *>( (CTreeItemData *)m_TreeProducts.GetItemData( hAdapterItemInTree ) );

		if( NULL == pclTreeItemAdapter )
		{
			continue;
		}

		CDB_Product *pclAdapterInTree = dynamic_cast<CDB_Product *>( pclTreeItemAdapter->m_pclAdapter );

		if( NULL == pclAdapterInTree || false == pclAdapterInTree->IsAnAccessory() )
		{
			continue;
		}

		if( pclAdapter == pclAdapterInTree )
		{
			// HYS-2031: Verify if the adapter is not already included in a selected accessory in the tree.
			CTreeItemActuator *pclTreeItemActuator = NULL;
			pclTreeItemActuator = (NULL != m_ItemActuatorSelected.m_hTreeItem) ? dynamic_cast<CTreeItemActuator *>((CTreeItemData *)m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem )) : NULL;
			HTREEITEM hItemActuatorForAdapter = (NULL != pclTreeItemActuator) ? pclTreeItemActuator->m_hTreeItem : NULL;
			
			if( false == _IsAdapterIncludedInASelectedItem( pclAdapterInTree, hItemActuatorForAdapter ) )
			{
				m_TreeProducts.SetItemImage( hAdapterItemInTree, iImageSelected, iImageSelected );
				m_ItemAdapterSelected.m_hTreeItem = hAdapterItemInTree;
			}
		}
		else
		{
			m_TreeProducts.SetItemImage( hAdapterItemInTree, CheckboxNotSelectedDisable, CheckboxNotSelectedDisable );
		}

		hAdapterItemInTree = m_TreeProducts.GetNextItem( hAdapterItemInTree, TVGN_NEXT );
	}
}

void CDlgDirSel::_UnselectAllAdapters( HTREEITEM hAdapterTitleItem, bool fForceDisable )
{
	if( NULL == hAdapterTitleItem )
	{
		return;
	}

	HTREEITEM hChildItem = m_TreeProducts.GetChildItem( hAdapterTitleItem );
	
	while( NULL != hChildItem )
	{
		CTreeItemAdapter *pclItemAdapterInTree = dynamic_cast<CTreeItemAdapter*>((CTreeItemData *)m_TreeProducts.GetItemData( hChildItem ) );

		if( NULL == pclItemAdapterInTree || NULL == pclItemAdapterInTree->m_pclAdapter )
		{
			continue;
		}

		int iImage = ( true == m_bWorkingForControlSet || true == fForceDisable ) ? CheckboxNotSelectedDisable : CheckboxNotSelected;
		
		// HYS-2031: Verify if the adapter is not already included in a selected accessory in the tree.

		if( false == _IsAdapterIncludedInASelectedItem( pclItemAdapterInTree->m_pclAdapter, m_ItemActuatorSelected.m_hTreeItem ) )
		{
			m_TreeProducts.SetItemImage( hChildItem, iImage, iImage );
		}
		else
		{
			m_TreeProducts.SetItemImage( hChildItem, CheckboxNotSelectedDisable, CheckboxNotSelectedDisable );
		}

		hChildItem = m_TreeProducts.GetNextItem( hChildItem, TVGN_NEXT );
	}
}

IMPLEMENT_DYNAMIC( CDlgDirSelExport, CDialogEx )

CDlgDirSelExport::CDlgDirSelExport( CWnd* pParent )
	: CDialogEx( CDlgDirSelExport::IDD, pParent )
{
	m_fExportAll = false;
}

BEGIN_MESSAGE_MAP( CDlgDirSelExport, CDialogEx )
	ON_BN_CLICKED( IDC_BUTTONALL, OnBnClickedAll )
	ON_BN_CLICKED( IDC_BUTTONAVAILABLE, OnBnClickedAvailable )
END_MESSAGE_MAP()

void CDlgDirSelExport::OnBnClickedAll()
{
	m_fExportAll = true;
	EndDialog( IDOK );
}

void CDlgDirSelExport::OnBnClickedAvailable()
{
	m_fExportAll = false;
	EndDialog( IDOK );
}

void CDlgDirSel::OnEnChangeEditsearchprod()
{
	m_ComboResults.Clear();
	m_ComboResults.ResetContent();
	m_ComboResults.SetWindowTextW(_T(""));
	m_btnSearch.EnableWindow( FALSE );

	CString strArticleNumber = _T("");
	CArray <CString> Arrstr;

	int index = 0;
	m_EditSearchProduct.GetWindowText(strArticleNumber);
	int iLength = strArticleNumber.GetLength();

	if (0 == iLength)
	{
		return;
	}

	// Get the list of ID corresponding to the article number.
	m_clDirectSelectionParams.m_pTADB->SearchByArticleNumber(strArticleNumber, &Arrstr);

	// Fill the comboResult with the Name of each element.
	for (index = 0; index < Arrstr.GetSize(); index++)
	{
		IDPTR ObjectIDPtr = TASApp.GetpTADB()->Get(Arrstr.GetAt(index));
		int iPos = -1;
		//Cast to get the GetName() method
		if (NULL != dynamic_cast<CDB_Product *>(ObjectIDPtr.MP))
		{
			iPos = m_ComboResults.AddString((LPCTSTR) dynamic_cast<CDB_Product *>(ObjectIDPtr.MP)->GetName());
			//To have ID in combobox
			m_ComboResults.SetItemDataPtr(iPos, (void *)(ObjectIDPtr.MP));
		}
		else if (NULL != dynamic_cast<CDB_Set *>(ObjectIDPtr.MP))
		{
			iPos = m_ComboResults.AddString((LPCTSTR) dynamic_cast<CDB_Set *>(ObjectIDPtr.MP)->GetName());
			m_ComboResults.SetItemDataPtr(iPos, (void *)(ObjectIDPtr.MP));
		}
		else if (NULL != dynamic_cast<CDB_Actuator *>(ObjectIDPtr.MP))
		{
			iPos = m_ComboResults.AddString((LPCTSTR) dynamic_cast<CDB_Actuator *>(ObjectIDPtr.MP)->GetName());
			m_ComboResults.SetItemDataPtr(iPos, (void *)(ObjectIDPtr.MP));
		}
		//The accessories are not be able to be selected in the combobox
	}

	if( m_ComboResults.GetCount() > 0 )
	{
		m_ComboResults.SetCurSel(0);
		m_btnSearch.EnableWindow( TRUE );
	}
}

void CDlgDirSel::OnCbnSelchangeComboselection()
{
	m_ComboResults.SetCurSel(m_ComboResults.GetCurSel());
}

void CDlgDirSel::OnBnClickedButtonsearch()
{
	CString strArticleNumber = _T("");
	IDPTR ObjectIDPtr;
	IDPTR ObjectIDPtrSet;
	CData *pDataCombo = (CData *)m_ComboResults.GetItemDataPtr(m_ComboResults.GetCurSel());
	
	if (NULL == pDataCombo)
	{
		return;
	}
	ObjectIDPtr = pDataCombo->GetIDPtr();
	strArticleNumber = ObjectIDPtr.MP->GetArtNum();

	if (NULL != dynamic_cast<CDB_Product *>(ObjectIDPtr.MP))
	{
		CDB_Product *pcdbProduct = dynamic_cast<CDB_Product *>(ObjectIDPtr.MP);
		HTREEITEM hItem = m_TreeProdCateg.GetRootItem();
		bool found = false;
		m_TreeProdCateg.SetFocus();
		FindFamilyInCategory(pcdbProduct->GetFamilyIDPtr().MP, hItem, &found, false);
		if ( found )
		{
			found = false;
			hItem = m_TreeProducts.GetRootItem();
			m_TreeProducts.SetFocus();
			SelectAndDisplayProduct(ObjectIDPtr, hItem, &found, false);
		}
	}
	else if (NULL != dynamic_cast<CDB_Set *>(ObjectIDPtr.MP))
	{
		CDB_Set *pcdbSet = dynamic_cast<CDB_Set *>(ObjectIDPtr.MP);
		ObjectIDPtrSet = TASApp.GetpTADB()->Get(pcdbSet->GetFirstIDPtr().ID);
		CDB_Product *pcdbProduct = dynamic_cast<CDB_Product *>(ObjectIDPtrSet.MP);
		HTREEITEM hItem = m_TreeProdCateg.GetRootItem();
		bool found = false;
		m_TreeProdCateg.SetFocus();
		FindFamilyInCategory(pcdbProduct->GetFamilyIDPtr().MP, hItem, &found, true);
		if (found)
		{
			found = false;
			hItem = m_TreeProducts.GetRootItem();
			m_TreeProducts.SetFocus();
			SelectAndDisplayProduct(ObjectIDPtr, hItem, &found, true);
		}
	}
	else if (NULL != dynamic_cast<CDB_Actuator *>(ObjectIDPtr.MP))
	{
		CDB_Actuator *pcdbActuator = dynamic_cast<CDB_Actuator *>(ObjectIDPtr.MP);
		HTREEITEM hItem = m_TreeProdCateg.GetRootItem();
		bool found = false;
		m_TreeProdCateg.SetFocus();
		FindFamilyInCategory(pcdbActuator->GetFamilyIDPtr().MP, hItem, &found, false);
		found = false;
		hItem = m_TreeProducts.GetRootItem();
		m_TreeProducts.SetFocus();
		SelectAndDisplayProduct(ObjectIDPtr, hItem, &found, false);
	}
}

void CDlgDirSel::FindFamilyInCategory(CData* pfam, HTREEITEM hItem, bool* pfound, bool bIsAset)
{
	if (NULL == pfam)
	{
		return;
	}
	while ((NULL != hItem) && (*pfound == false))
	{
		m_TreeProdCateg.Expand(hItem, TVE_EXPAND);
		m_TreeProdCateg.EnsureVisible(hItem);
		HTREEITEM hItemSubCategory = m_TreeProdCateg.GetChildItem(hItem);
		if ( NULL != hItemSubCategory )
		{
			FindFamilyInCategory(pfam, hItemSubCategory, pfound, bIsAset);
			if (!(*pfound))
			{
				m_TreeProdCateg.Expand(m_TreeProdCateg.GetParentItem(hItemSubCategory), TVE_COLLAPSE);
			}
		}
		else
		{
			CTreeItemData *pclTreeItemData = (CTreeItemData *)m_TreeProdCateg.GetItemData(hItem);
			CTreeItemFamily *pclTreeItemFamily = (CTreeItemFamily *)pclTreeItemData;
			// Identify if we are working for a set.
			// If not working on a Set we do not look under the Item identifed as a SET
			// and If working on a Set we look only the item identified like a SET  
			if ((!bIsAset) 
				&& (0 != IDcmp(pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_DPCONTRSET") ) )
				&& (0 != IDcmp(pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_STDCVACTSET") ) )
				&& (0 != IDcmp(pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ADVCTRLVSET") ) ) )
			{
				if ((NULL != pclTreeItemData->GetStringFamilyID()) &&
					(0 == IDcmp(pfam->GetIDPtr().ID, pclTreeItemData->GetStringFamilyID()->GetIDPtr().ID)))
				{
					m_TreeProdCateg.Expand(m_TreeProdCateg.GetParentItem(hItem), TVE_EXPAND);
					m_TreeProdCateg.EnsureVisible(hItem);
					m_TreeProdCateg.SelectItem(hItem);
					*pfound = true;
					break;
				}
			}
			else if ((bIsAset)
				&& ((0 == IDcmp(pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_DPCONTRSET") ) )
					|| (0 == IDcmp(pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_STDCVACTSET") ) )
					|| (0 == IDcmp(pclTreeItemFamily->m_pclParentSubCategory->GetIDPtr().ID, _T("SUBCATEGBC_ADVCTRLVSET") ) ) ) )
			{
				if ((NULL != pclTreeItemData->GetStringFamilyID()) &&
					(0 == IDcmp(pfam->GetIDPtr().ID, pclTreeItemData->GetStringFamilyID()->GetIDPtr().ID)))
				{
					m_TreeProdCateg.Expand(m_TreeProdCateg.GetParentItem(hItem), TVE_EXPAND);
					m_TreeProdCateg.EnsureVisible(hItem);
					m_TreeProdCateg.SelectItem(hItem);
					*pfound = true;
					break;
				}
			}
			m_TreeProdCateg.Expand(hItem, TVE_COLLAPSE);
		}
		hItem = m_TreeProdCateg.GetNextItem(hItem, TVGN_NEXT);			
	}
}

void CDlgDirSel::SelectAndDisplayProduct(IDPTR IdptrProductID, HTREEITEM hItem, bool* bPfound, bool bIsAset)
{
	while ((NULL != hItem) && (!(*bPfound)))
	{
		m_TreeProducts.EnsureVisible(hItem);

		CTreeItemProduct *pclTreeItemProduct = (CTreeItemProduct *)m_TreeProducts.GetItemData(hItem);
		if (bIsAset)
		{
			if (0 == IDcmp((dynamic_cast<CDB_Set *>(IdptrProductID.MP))->GetFirstIDPtr().ID, pclTreeItemProduct->m_pclProduct->GetIDPtr().ID))
			{
				//Subdirectory here
				HTREEITEM hChildItem = m_TreeProducts.GetChildItem(hItem);
				while (hChildItem != NULL)
				{
					//Content of subdirectory: accessories are not managed
					if (NULL == m_TreeProducts.GetChildItem(hChildItem))
					{
						hChildItem = m_TreeProducts.GetNextItem(hChildItem, TVGN_NEXT);
						continue;
					}
					CTreeItemProduct *pclChildItemProduct = (CTreeItemProduct *)m_TreeProducts.GetItemData(m_TreeProducts.GetChildItem(hChildItem));
					if ((NULL != dynamic_cast<CDB_Set *>(pclChildItemProduct->m_pclProduct->GetIDPtr().MP))
						|| (NULL != dynamic_cast<CDB_Product *>(pclChildItemProduct->m_pclProduct->GetIDPtr().MP))
						|| (NULL != dynamic_cast<CDB_Actuator *>(pclChildItemProduct->m_pclProduct->GetIDPtr().MP)))
					{
						if (NULL != dynamic_cast<CDB_Set *>(pclChildItemProduct->m_pclProduct->GetIDPtr().MP))
						{
							SelectAndDisplayProduct((dynamic_cast<CDB_Set *>(IdptrProductID.MP))->GetSecondIDPtr(), m_TreeProducts.GetChildItem(hChildItem), bPfound, true);
						}
						else
						{
							SelectAndDisplayProduct((dynamic_cast<CDB_Set *>(IdptrProductID.MP))->GetSecondIDPtr(), m_TreeProducts.GetChildItem(hChildItem), bPfound, false);
						}

						if (*bPfound)
						{
							break;
						}
					}
					hChildItem = m_TreeProducts.GetNextItem(hChildItem, TVGN_NEXT);
				}

			}
		}
		else
		{
			if ((NULL != pclTreeItemProduct->m_pclProduct->GetIDPtr().ID) &&
				(0 == IDcmp(IdptrProductID.ID, pclTreeItemProduct->m_pclProduct->GetIDPtr().ID)))
			{
				m_TreeProducts.Expand(hItem, TVE_EXPAND);
				if (hItem != m_TreeProducts.GetSelectedItem())
				{
					m_TreeProducts.SelectItem(hItem);
					m_TreeProducts.SetItemImage(hItem, RadioSelected, RadioSelected);
					ClickTreeProducts(hItem);
				}
				m_TreeProducts.SetFocus();
				m_TreeProducts.EnsureVisible(hItem);
				*bPfound = true;
				break;
			}
		}
		hItem = m_TreeProducts.GetNextItem(hItem, TVGN_NEXT);
	}
}

bool CDlgDirSel::IsAccessoryFictifSet( CDB_Product* pclProductParent, CDB_Product* pclFictifSet )
{
	if( NULL == pclProductParent || NULL == pclFictifSet )
	{
		return false;
	}

	bool bReturn = false;
	// Get Set accessories for product parent
	if( (NULL != dynamic_cast<CDB_SmartControlValve*>(pclProductParent))
		&& (0 == StringCompare( _T( "SMARTDPCTYPE" ), pclProductParent->GetTypeID() )) )
	{
		CDB_SmartControlValve* pclSmartDpC = dynamic_cast<CDB_SmartControlValve*>(pclProductParent);
		CTable *pTableSet = (CTable*)pclSmartDpC->GetDpSensorGroupIDPtr().MP;

		if( NULL != pTableSet )
		{
			for( IDPTR idptr = pTableSet->GetFirst(); NULL != idptr.MP; idptr = pTableSet->GetNext() )
			{
				CDB_Product* pclProductSet = dynamic_cast<CDB_Product*>(idptr.MP);

				if( 0 == StringCompare( pclProductSet->GetIDPtr().ID, pclFictifSet->GetIDPtr().ID ) )
				{
					bReturn = true;
					break;
				}
			}
		}
	}

	return bReturn;
}

void CDlgDirSel::HasBuiltinAccessory( CDB_Product* pclProduct, CDB_Product* pclProductSub, bool& bBuiltinExists, bool& bBuiltinExistsSub )
{
	if( NULL == pclProduct || NULL == pclProductSub )
	{
		return;
	}

	bBuiltinExists = false;
	bBuiltinExistsSub = false;
	CDB_RuledTable* pclRuledTable = NULL;
	CDB_RuledTable* pclRuledTableSub = NULL;

	if( NULL != pclProduct )
	{
		pclRuledTable = (CDB_RuledTable*)pclProduct->GetAccessoriesGroupIDPtr().MP;

		if( NULL != pclRuledTable )
		{
			CRank rList;

			CString str;
			LPARAM lparam;

			for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rList.GetNext( str, lparam ) )
			{
				CDB_Product* pAccessory = (CDB_Product*)lparam;
				if( NULL == pAccessory || false == pAccessory->IsSelectable( true ) )
				{
					continue;
				}

				if( true == pclRuledTable->IsBuiltIn( pAccessory ) )
				{
					bBuiltinExists = true;
					break;
				}
			}
		}
	}

	if( NULL != pclProductSub )
	{
		pclRuledTableSub = (CDB_RuledTable*)pclProductSub->GetAccessoriesGroupIDPtr().MP;
		if( NULL != pclRuledTableSub )
		{
			CRank rList;
			m_clDirectSelectionParams.m_pTADB->GetAccessories( &rList, pclRuledTableSub, m_clDirectSelectionParams.m_eFilterSelection );

			CString str;
			LPARAM lparam;

			for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rList.GetNext( str, lparam ) )
			{
				CDB_Product* pAccessory = (CDB_Product*)lparam;
				if( NULL == pAccessory || false == pAccessory->IsSelectable( true ) )
				{
					continue;
				}

				if( true == pclRuledTableSub->IsBuiltIn( pAccessory ) )
				{
					bBuiltinExistsSub = true;
					break;
				}
			}
		}
	}
}

void CDlgDirSel::_UpdateSelectButton()
{
	 if( NULL == m_ItemProductSelected.m_hTreeItem && NULL == m_ItemAdapterSelected.m_hTreeItem 
			&& NULL == m_ItemActuatorSelected.m_hTreeItem && NULL == m_ItemIntegratedSelected.m_hTreeItem )
	 {
		 m_btnOK.EnableWindow( TRUE );
	 }
	 else
	 {
		 if( NULL != m_ItemProductSelected.m_hTreeItem )
		 {
			 CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemProductSelected.m_hTreeItem ) );

			 if( NULL != pclTreeItemProduct && NULL != pclTreeItemProduct->m_pclProduct )
			 {
				 m_btnOK.EnableWindow( ( true == pclTreeItemProduct->m_pclProduct->IsSelectable( true ) ) ? TRUE : FALSE );
			 }
		 }
		 else if( NULL != m_ItemAdapterSelected.m_hTreeItem )
		 {
			 CTreeItemAdapter *pclTreeItemAdapter = dynamic_cast<CTreeItemAdapter *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemAdapterSelected.m_hTreeItem ) );

			 if( NULL != pclTreeItemAdapter && NULL != pclTreeItemAdapter->m_pclAdapter )
			 {
				 m_btnOK.EnableWindow( ( true == pclTreeItemAdapter->m_pclAdapter->IsSelectable( true ) ) ? TRUE : FALSE );
			 }
		 }
		 else if( NULL != m_ItemActuatorSelected.m_hTreeItem )
		 {
			 CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemActuatorSelected.m_hTreeItem ) );

			 if( NULL != pclTreeItemActuator && NULL != pclTreeItemActuator->m_pclActuator )
			 {
				 m_btnOK.EnableWindow( ( true == pclTreeItemActuator->m_pclActuator->IsSelectable( true ) ) ? TRUE : FALSE );
			 }
		 }
		 else if( NULL != m_ItemIntegratedSelected.m_hTreeItem )
		 {
			 CTreeItemIntegrated *pclTreeItemIntegrated = dynamic_cast<CTreeItemIntegrated *>( (CTreeItemData *)m_TreeProducts.GetItemData( m_ItemIntegratedSelected.m_hTreeItem ) );

			 if( NULL != pclTreeItemIntegrated && NULL != pclTreeItemIntegrated->m_pclIntegrated )
			 {
				 m_btnOK.EnableWindow( ( true == pclTreeItemIntegrated->m_pclIntegrated->IsSelectable( true ) ) ? TRUE : FALSE );
			 }
		 }
	 }
}

void CDlgDirSel::_CheckCrossingAccessoriesCVAndActuator( HTREEITEM hControlValve, HTREEITEM hActuator, bool bCheck )
{
	// HYS-1617: It's exactly the same code as in the 'CRViewSSelCtrl::GrayUncompatibleCrossingAccessories' method in the 'RViewSSelCtrl.cpp' file.
	// Any modification applied here must be done also in 'CRViewSSelCtrl::GrayUncompatibleCrossingAccessories' !!

	// First, try to retrieve the ruled table for the control valve.
	CTreeItemProduct *pclTreeItemCV = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( hControlValve ) );

	if( NULL == pclTreeItemCV || NULL == pclTreeItemCV->m_pclProduct || NULL == dynamic_cast<CDB_ControlValve *>( pclTreeItemCV->m_pclProduct ) )
	{
		return;
	}

	CDB_ControlValve *pclCurrentControlValveSelected = (CDB_ControlValve *)( pclTreeItemCV->m_pclProduct );

	if( NULL == dynamic_cast<CDB_RuledTable *>( pclCurrentControlValveSelected->GetAccessoriesGroupIDPtr().MP ) )
	{
		return;
	}

	CDB_RuledTable *pclRuledTableCtrlValve = (CDB_RuledTable *)( pclCurrentControlValveSelected->GetAccessoriesGroupIDPtr().MP );

	// Now, try to retrieve the ruled table for actuator.
	CDB_RuledTable *pclRuledTableActuator = NULL;

	if( NULL != hActuator )
	{
		CTreeItemActuator *pclTreeItemActuator = dynamic_cast<CTreeItemActuator *>( (CTreeItemData *)m_TreeProducts.GetItemData( hActuator ) );

		if( NULL != pclTreeItemActuator && NULL != pclTreeItemActuator->m_pclProduct && NULL != dynamic_cast<CDB_Actuator *>( pclTreeItemActuator->m_pclActuator ) )
		{
			CDB_Actuator *pclActuator = (CDB_Actuator *)( pclTreeItemActuator->m_pclActuator );
			pclRuledTableActuator = (CDB_RuledTable *)( pclActuator->GetAccessoriesGroupIDPtr().MP );
		}
	}
	
	vector<CDB_Product *> vecCrossingAccessories;

	for( IDPTR IDPtr = pclRuledTableCtrlValve->GetFirst(); NULL != IDPtr.MP; IDPtr = pclRuledTableCtrlValve->GetNext() )
	{
		if( true == pclRuledTableCtrlValve->IsACrossingAccDisplayed( IDPtr.MP ) )
		{
			vecCrossingAccessories.push_back( dynamic_cast<CDB_Product *>( IDPtr.MP ) );
		}
	}

	std::vector<CTreeItemAccessory *> vecControlValveAccessories;
	_GetAccessoryList( vecControlValveAccessories, hControlValve );

	if( 0 == vecControlValveAccessories.size() )
	{
		return;
	}

	// First step: If we select an actuator (bCheck = true), check if we need to activate the accessory in common if exist.
	if( NULL != pclRuledTableActuator && true == bCheck )
	{
		// Find shared accessories between the two cross tables.
		for( IDPTR IDPtr = pclRuledTableActuator->GetFirst(); NULL != IDPtr.MP; IDPtr = pclRuledTableActuator->GetNext() )
		{
			// Search if the accessory in the actuator accessory table exists in the accessory table of the valve that is cross-referenced.
			vector<CDB_Product *>::iterator it = find( vecCrossingAccessories.begin(), vecCrossingAccessories.end(), dynamic_cast<CDB_Product *>( IDPtr.MP ) );

			// If accessory is common (We have in this case the '+' for the accessory in the actuator accessory group which signals us that it's an accessory
			// common with the product but that we can't show -> The 'IsACrossingAcc' method returns in this case 'true').
			if( ( true == pclRuledTableActuator->IsACrossingAcc( IDPtr.MP ) ) && ( vecCrossingAccessories.end() != it ) )
			{
				vecCrossingAccessories.erase( it );

				// Search in the available accessories for the current control valve if this common accessory exist.
				for( auto &iter : vecControlValveAccessories )
				{
					if( NULL != iter )
					{
						// If the current control valve accessory displayed is the same as the common accessory...
						if( iter->m_pclAccessory->GetIDPtr().MP == IDPtr.MP )
						{
							int iImage = -1;
							int iImageSelected = -1;
							m_TreeProducts.GetItemImage( iter->m_hTreeItem, iImage, iImageSelected );

							// Change state if the previous one was disabled.
							if( CheckboxNotSelectedDisable == iImage )
							{
								m_TreeProducts.SetItemImage( iter->m_hTreeItem, CheckboxNotSelected, CheckboxNotSelected );
							}
						}
					}
				}
			}
		}
	}

	// bCheck = true  -> Accessories which are not compatible with actuator (Those that were common with the actuator were removed in the first step just above).
	// bCheck = false -> Accessories that belong to a cross-reference table and are displayed.
	if( vecCrossingAccessories.size() > 0 )
	{
		// Run all the control valve accessories.
		for( auto &iter : vecControlValveAccessories )
		{
			if( NULL != iter )
			{
				// Verify if the current accessory in the right view belongs to a cross-reference table.
				vector<CDB_Product *>::iterator it = find( vecCrossingAccessories.begin(), vecCrossingAccessories.end(), iter->m_pclAccessory );

				// If it's a common accessory between valve and actuator...
				if( it != vecCrossingAccessories.end() )
				{
					if( true == bCheck )
					{
						// If 'bCheck = true' and there are yet accessories in 'vecCrossingAccessories', it means that these accessories are not common with the
						// the current selected actuator (Because if common the 'vecCrossingAccessories.erase( it )' remove this accessory from the vector).
				
						// Set it as disable. Because it's a common accessory but not available with the current selected actuator.
						m_TreeProducts.SetItemImage( iter->m_hTreeItem, CheckboxNotSelectedDisable, CheckboxNotSelectedDisable );
					}
					else
					{
						// If 'bCheck = false', there is not yet actuator selected. So, it's not possible for the moment to set the status of the common accessories.
						// This is why we enable all the common accessories for the moment.
						int iImage = -1;
						int iImageSelected = -1;
						m_TreeProducts.GetItemImage( iter->m_hTreeItem, iImage, iImageSelected );

						if( CheckboxNotSelectedDisable == iImage )
						{
							m_TreeProducts.SetItemImage( iter->m_hTreeItem, CheckboxNotSelected, CheckboxNotSelected );
						}
					}
				}
			}
		}
	}
}

void CDlgDirSel::_GetAccessoryList( std::vector<CTreeItemAccessory *> &vecAccessoryList, HTREEITEM hParent )
{
	vecAccessoryList.clear();

	CTreeItemProduct *pclTreeItemProduct = dynamic_cast<CTreeItemProduct *>( (CTreeItemData *)m_TreeProducts.GetItemData( hParent ) );

	if( NULL == pclTreeItemProduct || NULL == pclTreeItemProduct->m_pclProduct )
	{
		return;
	}

	// Run all products in the tree.
	for( auto &iter : m_vecItemDataInTreeList )
	{
		// Check only the accessories.
		if( NULL != iter && (CTreeItemData::DataType::Accessory == iter->m_eDataType ) 
				&& NULL != dynamic_cast<CTreeItemAccessory *>( iter ) )
		{
			CTreeItemAccessory *pclTreeItemAccessory = dynamic_cast<CTreeItemAccessory *>( iter );

			// If object on which is linked the accessory is the same of 'hParent', we can add it.
			if( pclTreeItemAccessory->m_pclParent == pclTreeItemProduct->m_pclProduct )
			{
				vecAccessoryList.push_back( pclTreeItemAccessory );
			}
		}
	}
}

bool CDlgDirSel::_CanDisplayType( CString strTypeID )
{
	if( false == TASApp.IsDpCDisplayed() && 0 == strTypeID.Compare( _T("DPCTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsTrvDisplayed() && 0 == strTypeID.Compare( _T("TRVTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsCvDisplayed() && 0 == strTypeID.Compare( _T("CVTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsBCvDisplayed() && 0 == strTypeID.Compare( _T("BCVTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsPICvDisplayed() && 0 == strTypeID.Compare( _T("PICVTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsAirVentSepDisplayed() && 0 == strTypeID.Compare( _T("AIRVTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsAirVentSepDisplayed() && 0 == strTypeID.Compare( _T("SEPTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsDpCBCVDisplayed() && 0 == strTypeID.Compare( _T("DPCBALCTRLVALV_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsShutOffValveDisplayed() && 0 == strTypeID.Compare( _T("SHUTOFF_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsPressureMaintenanceDisplayed() && 0 == strTypeID.Compare( _T("VSSLTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsSafetyValveDisplayed() && 0 == strTypeID.Compare( _T("SFTYVTYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.IsSmartControlValveDisplayed() && 0 == strTypeID.Compare( _T("SMARTCTRLVALVETYPE_TAB") ) )
	{
		return false;
	}

	if( false == TASApp.Is6WayCVDisplayed() && 0 == strTypeID.Compare( _T("6WAYTYPE_TAB") ) )
	{
		return false;
	}

	return true;
}

bool CDlgDirSel::_IsAdapterIncludedInASelectedItem( CDB_Product *pclProductIncluded, HTREEITEM hItemActuator )
{
	if( NULL == pclProductIncluded || NULL == hItemActuator )
	{
		return false;
	}

	HTREEITEM hItemParent = _GetParentProduct( hItemActuator );

	// Get actuator adater group.
	std::vector<CData *> setAccGroup;
	CDB_RuledTable *pclRuledTable = NULL;

	CTreeItemActuator *pclItemActuator = (CTreeItemActuator *)(m_TreeProducts.GetItemData( hItemActuator ));
	if( NULL != pclItemActuator )
	{
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>(pclItemActuator->m_pclActuator);

		if( NULL != pclActuator )
		{
			pclRuledTable = dynamic_cast<CDB_RuledTable *>(pclActuator->GetActAdapterGroupIDPtr().MP);
		}
	}
	
	// Get excluded products
	if( NULL != pclRuledTable )
	{
		// Accessories group exist.
		pclRuledTable->GetVector( CDB_RuledTable::Excluded, pclProductIncluded, &setAccGroup );
	}

	if( 0 >= setAccGroup.size() )
	{
		return false;
	}

	bool bAlreadyInSelection = false;

	// Loop on all excluded products
	for( int i = 0; i < (int)setAccGroup.size(); i++ )
	{
		// Lopp on selected accessory to check if the current product is included.
		for( auto &itVect : m_vecProductAccessorySelected )
		{
			CTreeItemAccessory *pclItemData = (CTreeItemAccessory *)(m_TreeProducts.GetItemData( itVect.m_hTreeItem ));

			if( NULL != pclItemData && NULL != pclItemData->m_pclAccessory && hItemParent == _GetParentProduct( pclItemData->m_hTreeItem ) )
			{
				int iImage = -1;
				int iImageSelected = -1;

				// Information if the item image is selected or not.
				m_TreeProducts.GetItemImage( pclItemData->m_hTreeItem, iImage, iImageSelected );
				// Insure that the item is selected.
				if( ( RadioSelected == iImageSelected || CheckboxSelected == iImageSelected || RadioSelectedDisable == iImageSelected || CheckboxSelectedDisable == iImageSelected )
					&& ( 0 == IDcmp( pclItemData->m_pclAccessory->GetIDPtr().ID, setAccGroup.at(i)->GetIDPtr().ID ) ) )
				{
					bAlreadyInSelection = true;
					break;
				}
			}
		}
		if( true == bAlreadyInSelection )
		{
			break;
		}
	}

	return bAlreadyInSelection;
}