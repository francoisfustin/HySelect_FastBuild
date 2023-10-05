#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "HMTreeDroptarget.h"
#include "HMTreeListCtrl.h"
#include "DlgSearchReplace.h"
#include <algorithm>

IMPLEMENT_DYNAMIC( CDlgSearchReplace, CDlgCtrlPropPageMng )

CDlgSearchReplace *pDlgSearchReplace = NULL;

CDlgSearchReplace::CDlgSearchReplace( OpenFromWhere eOpenFromWhere, CWnd *pParent )
	: CDlgCtrlPropPageMng( CDlgSearchReplace::IDD, pParent )
{
	m_eOpenFromWhere = eOpenFromWhere;
	m_eCurrentPage = OpeningPage::OP_HMPipes;
	m_pHMHighlighted = NULL;
	m_pclImgListCheckBox = NULL;
	m_bIndSelTreeInitialized = false;
	m_bHMTreeInitialized = false;
	pDlgSearchReplace = this;
}

CDlgSearchReplace::~CDlgSearchReplace()
{
	if( NULL != m_pclImgListCheckBox )
	{
		delete m_pclImgListCheckBox;
	}

	pDlgSearchReplace = NULL;
}

HTREEITEM CDlgSearchReplace::GetHItem( CDS_HydroMod *pHMtoFind, HTREEITEM hTreeItem )
{
	if( NULL == hTreeItem )
	{
		hTreeItem = m_Tree.GetRootItem();
	}

	// Do a loop on the tree.
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hTreeItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		CDS_HydroMod *pHM = reinterpret_cast<CDS_HydroMod *>( m_Tree.GetItemData( hChildItem ) );

		if( pHM == pHMtoFind )
		{
			return 	hChildItem;
		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			HTREEITEM hit = GetHItem( pHMtoFind, hChildItem );

			if( NULL != hit )
			{
				return hit;
			}
		}
	}

	return NULL;
}

void CDlgSearchReplace::ResetItemsColor( COLORREF col, HTREEITEM hTreeItem )
{
	if( NULL == hTreeItem )
	{
		hTreeItem = m_Tree.GetRootItem();
	}

	m_Tree.SetItemTextColor( hTreeItem, 0, col );

	// Do a loop on the tree.
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hTreeItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		m_Tree.SetItemTextColor( hChildItem, 0, col );

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			ResetItemsColor( col, hChildItem );
		}
	}
}

int CDlgSearchReplace::GetSelectedModules( std::map<int, CDS_HydroMod *> *pMap, int &iPos, CDS_HydroMod::eHMObj fLockCircuits, BOOL fModules, BOOL fCircuits,
		HTREEITEM hTreeItem )
{
	if( NULL == pMap )
	{
		return false;
	}

	if( NULL == hTreeItem )
	{
		hTreeItem = m_Tree.GetRootItem();
	}

	// Do a loop on the tree.
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hTreeItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		if( true == _IsCheckboxChecked( hChildItem ) )
		{
			CDS_HydroMod *pHM = reinterpret_cast<CDS_HydroMod *>( m_Tree.GetItemData( hChildItem ) );

			if( CDS_HydroMod::eHMObj::eNone != fLockCircuits && true == pHM->IsLocked( fLockCircuits ) )
			{
				continue;
			}

			if( true == pHM->IsaModule() )
			{
				if( TRUE == fModules )
				{
					( *pMap )[++iPos] = pHM;
				}
			}
			else
			{
				if( TRUE == fCircuits )
				{
					( *pMap )[++iPos] = pHM;
				}
			}

		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			GetSelectedModules( pMap, iPos, fLockCircuits, fModules, fCircuits, hChildItem );
		}
	}

	return pMap->size();

}

void CDlgSearchReplace::UpdateTreeHMpointers( std::map<WPARAM, CDS_HydroMod *> *pMap, HTREEITEM hTreeItem )
{
	if( NULL == pMap )
	{
		return;
	}

	if( NULL == hTreeItem )
	{
		hTreeItem = m_Tree.GetRootItem();
	}

	// Do a loop on the tree.
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hTreeItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		if( true == _IsCheckboxChecked( hChildItem ) )
		{
			CDS_HydroMod *pHM = reinterpret_cast<CDS_HydroMod *>( m_Tree.GetItemData( hChildItem ) );

			// Try to find it into the map.
			std::map<WPARAM, CDS_HydroMod *>::iterator it = pMap->find( ( WPARAM )pHM );

			if( it != pMap->end() && NULL != it->second )
			{
				m_Tree.SetItemData( hChildItem, ( DWORD_PTR )it->second );
			}
		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			UpdateTreeHMpointers( pMap, hChildItem );
		}
	}
}

CRect CDlgSearchReplace::GetLeftTreeWidth()
{
	CRect rect;
	GetDlgItem( IDC_FRAMETREE )->GetWindowRect( &rect );
	return rect;
}

void CDlgSearchReplace::SRPageSet_OnRadioSplitClicked()
{
	if( OpeningPage::OP_SetGroupOrSplit != m_eCurrentPage )
	{
		return;
	}

	_ReInitializeLayout();
}

void CDlgSearchReplace::SRPageSet_OnRadioGroupClicked()
{
	if( OpeningPage::OP_SetGroupOrSplit != m_eCurrentPage )
	{
		return;
	}

	_ReInitializeLayout();
}

void CDlgSearchReplace::SRPageSet_OnReplacePopupHM( CDS_HydroMod *pHM )
{
	if( NULL != m_pHMHighlighted )
	{
		HTREEITEM hItem = GetHItem( m_pHMHighlighted );

		if( NULL != hItem )
		{
			m_Tree.SetItemTextColor( hItem, 0, _TAH_BLACK );
			m_Tree.EnsureVisible( hItem );
		}
	}

	m_pHMHighlighted = NULL;

	if( NULL != pHM )
	{
		HTREEITEM hItem = GetHItem( pHM );

		if( NULL != hItem )
		{
			m_Tree.SetItemTextColor( hItem, 0, _TAH_ORANGE );
			m_Tree.EnsureVisible( hItem );
			m_pHMHighlighted = pHM;
		}
	}
}

void CDlgSearchReplace::SRPageActuator_OnRadioRemoveClicked()
{
	if( OpeningPage::OP_ActrAddOrRemove != m_eCurrentPage )
	{
		return;
	}

	_ReInitializeLayout();
}

void CDlgSearchReplace::SRPageActuator_OnRadioAddClicked()
{
	if( OpeningPage::OP_ActrAddOrRemove != m_eCurrentPage )
	{
		return;
	}

	_ReInitializeLayout();
}

void CDlgSearchReplace::SRPageActuator_OnCheckRemoveForSet()
{
	if( OpeningPage::OP_ActrAddOrRemove != m_eCurrentPage )
	{
		return;
	}

	_ReInitializeLayout();
}

void CDlgSearchReplace::SRPageActuator_OnReplacePopupHM( CDS_HydroMod *pHM )
{
	if( NULL != m_pHMHighlighted )
	{
		HTREEITEM hItem = GetHItem( m_pHMHighlighted );

		if( NULL != hItem )
		{
			m_Tree.SetItemTextColor( hItem, 0, _TAH_BLACK );
			m_Tree.EnsureVisible( hItem );
		}
	}

	m_pHMHighlighted = NULL;

	if( NULL != pHM )
	{
		HTREEITEM hItem = GetHItem( pHM );

		if( NULL != hItem )
		{
			m_Tree.SetItemTextColor( hItem, 0, _TAH_ORANGE );
			m_Tree.EnsureVisible( hItem );
			m_pHMHighlighted = pHM;
		}
	}
}

BEGIN_MESSAGE_MAP( CDlgSearchReplace, CDlgCtrlPropPageMng )
	ON_CONTROL_RANGE( BN_CLICKED, IDC_RADIOHMCALC, IDC_RADIOINDSEL, OnBnClickedRadios )
	ON_CBN_SELCHANGE( IDC_COMBO, OnCbnSelChange )
	ON_NOTIFY( NM_CLICK, IDC_CURRPROJTREE, OnNMClickTree )
	ON_BN_CLICKED( IDC_BTNCHECK, OnBnClickedCheckAll )
	ON_BN_CLICKED( IDC_BTNUNCHECK, OnBnClickedUncheckAll )
	ON_BN_CLICKED( IDC_BTNEXPAND, OnBnClickedExpandAll )
	ON_BN_CLICKED( IDC_BTNCOLLAPSE, OnBnClickedCollapseAll )
	ON_BN_CLICKED( IDC_START, OnBnClickedStart )
	ON_BN_CLICKED( IDCANCEL, OnBnClickedCancel )
END_MESSAGE_MAP()

void CDlgSearchReplace::DoDataExchange( CDataExchange *pDX )
{
	CDlgCtrlPropPageMng::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_RADIOHMCALC, m_RadioHMCalc );
	DDX_Control( pDX, IDC_RADIOINDSEL, m_RadioIndSel );
	DDX_Control( pDX, IDC_COMBO, m_ComboBox );
	DDX_Control( pDX, IDC_TREEHEADER, m_ExtStaticHeader );
	DDX_Control( pDX, IDC_BTNCHECK, m_BtnCheck );
	DDX_Control( pDX, IDC_BTNUNCHECK, m_BtnUnCheck );
	DDX_Control( pDX, IDC_BTNEXPAND, m_BtnExpand );
	DDX_Control( pDX, IDC_BTNCOLLAPSE, m_BtnCollapse );
	DDX_Control( pDX, IDC_START, m_BtnStart );
}

BOOL CDlgSearchReplace::OnInitDialog()
{
	BOOL Breturn = CDlgCtrlPropPageMng::OnInitDialog();

	// Init strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_CAPTION );
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_JOBCHOICE );
	GetDlgItem( IDC_STATICJOBCHOICE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_RADIOHMCALC );
	GetDlgItem( IDC_RADIOHMCALC )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_RADIOINDSEL );
	GetDlgItem( IDC_RADIOINDSEL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CLOSE );
	GetDlgItem( IDCANCEL )->SetWindowText( str );

	m_BtnStart.SetWindowText( TASApp.LoadLocalizedString( IDS_TABDLGSEARCHREPLACE_START ) );

	// Set bitmap into the buttons.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_BtnExpand.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_ExpandAllTree ) );
		m_BtnCollapse.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_CollapseAllTree ) );
		m_BtnCheck.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_SelectAll ) );
		m_BtnUnCheck.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_UnselectAll ) );
	}

	// Set tooltips into the buttons.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BTNEXPAND ), TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTEXTEND ) );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BTNCOLLAPSE ), TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTCOLLAPSE ) );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BTNCHECK ), TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTSELECT ) );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BTNUNCHECK ), TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTUNSELECT ) );

	// Define the position to place the current page.
	CRect rect;
	GetDlgItem( IDC_FRAMEPAGE )->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_PageRect.top		= rect.top;
	m_PageRect.left		= rect.left;
	m_PageRect.right	= rect.right;
	m_PageRect.bottom	= rect.bottom;

	// Create pages.
	m_PagePipes.Create( IDD_TABDLGSEARCHPIPES, this );
	m_PagePipes.SetPageNumber( OpeningPage::OP_HMPipes );
	m_PagePipes.ShowWindow( SW_HIDE );

	m_PageTemp.Create( IDD_TABDLGSEARCHTEMP, this );
	m_PageTemp.SetPageNumber(OpeningPage::OP_HMTemp);
	m_PageTemp.ShowWindow(SW_HIDE);

	m_PageCircuit.Create( IDD_TABDLGSEARCHCIRCUITS, this );
	m_PageCircuit.SetPageNumber( OpeningPage::OP_HMCircuit );
	m_PageCircuit.ShowWindow( SW_HIDE );

	m_PageSet.Create( IDD_TABDLGSEARCHSET, this );
	m_PageSet.SetNotificationHandler( this );
	m_PageSet.SetPageNumber( OpeningPage::OP_SetGroupOrSplit );
	m_PageSet.ShowWindow( SW_HIDE );

	m_PageActuator.SetHMCalcOrIndSel( ( int )m_eRadioState );
	m_PageActuator.Create( IDD_TABDLGSEARCHACTUATOR, this );
	m_PageActuator.SetNotificationHandler( this );
	m_PageActuator.SetPageNumber( OpeningPage::OP_ActrAddOrRemove );
	m_PageActuator.ShowWindow( SW_HIDE );

	// Check what is available (HM or/and individual selection).
	// Remark: if any of both is available, we are not able to call 'Search and replace' dialog.

	bool fHMExist = true;
	bool fIndSelExist = true;
	CTableHM *pTableHM = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
	ASSERT( NULL != pTableHM );

	if( NULL == pTableHM || 0 == pTableHM->GetItemCount() ||
		( pTableHM->GetItemCount() > 0 && true == pMainFrame->IsFreezeChecked() ) )
	{
		fHMExist = false;
		m_RadioHMCalc.EnableWindow( FALSE );
	}
	else if( false == TASApp.GetpTADS()->IsIndividualSelectionExist() )
	{
		fIndSelExist = false;
		m_RadioIndSel.EnableWindow( FALSE );
	}

	if( ( OpenFromWhere::OFW_RibbonProject == m_eOpenFromWhere && true == fHMExist ) ||
		( OpenFromWhere::OFW_RibbonProductSel == m_eOpenFromWhere && false == fIndSelExist ) )
	{
		m_eCurrentPage = OP_HMPipes;
		CheckRadioButton( IDC_RADIOHMCALC, IDC_RADIOINDSEL, IDC_RADIOHMCALC );
		m_eRadioState = RadioState::RS_HMCalc;
	}
	else
	{
		m_eCurrentPage = OP_SetGroupOrSplit;
		CheckRadioButton( IDC_RADIOHMCALC, IDC_RADIOINDSEL, IDC_RADIOINDSEL );
		m_eRadioState = RadioState::RS_IndSel;
	}

	_InitializeLayout( m_eCurrentPage );

	return Breturn;
}

void CDlgSearchReplace::OnBnClickedRadios( UINT nID )
{
	switch( nID )
	{
		case IDC_RADIOHMCALC:
			if( BST_CHECKED != m_RadioHMCalc.GetCheck() )
			{
				CheckRadioButton( IDC_RADIOHMCALC, IDC_RADIOINDSEL, IDC_RADIOHMCALC );
				m_eRadioState = RadioState::RS_HMCalc;

				// Disable possibility to get tab goes to focus on radio button.
				m_RadioHMCalc.ModifyStyle( WS_TABSTOP, 0 );

				_InitializeLayout( OpeningPage::OP_HMPipes );
			}

			break;

		case IDC_RADIOINDSEL:
			if( BST_CHECKED != m_RadioIndSel.GetCheck() )
			{
				CheckRadioButton( IDC_RADIOHMCALC, IDC_RADIOINDSEL, IDC_RADIOINDSEL );
				m_eRadioState = RadioState::RS_IndSel;

				// Disable possibility to get tab goes to focus on radio button.
				m_RadioIndSel.ModifyStyle( WS_TABSTOP, 0 );

				_InitializeLayout( OpeningPage::OP_SetGroupOrSplit );
			}

			break;
	}
}

void CDlgSearchReplace::OnCbnSelChange()
{
	int iItem = m_ComboBox.GetCurSel();
	CDlgCtrlPropPage *pPage = dynamic_cast<CDlgCtrlPropPage *>( ( CDialogEx * )m_ComboBox.GetItemData( iItem ) );
	ASSERT( NULL != pPage );

	if( NULL == pPage )
	{
		return;
	}

	m_eCurrentPage = ( OpeningPage )pPage->GetPageNumber();
	_ReInitializeLayout();

	_DisplayLeftPane( pPage );
}

void CDlgSearchReplace::OnNMClickTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	// On a TreeView Control with the CheckBoxes, there is no notification that the check state of the item has been changed,
	// you can just determine that the user has clicked the state Icon.
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is to post a user defined message
	// as a notification that the check state has changed.

	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( ( int )( short )LOWORD( dwpos ) );				// GET_X_LPARAM(dwpos);
	ht.pt.y = ( ( int )( short )HIWORD( dwpos ) );				// GET_Y_LPARAM(dwpos);

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	// User click on an item or select it with the arrows.
	if( NULL != ht.hItem && ( ( TVHT_ONITEMICON & ht.flags ) || ( TVHT_ONITEM & ht.flags ) ) )
	{
		switch( m_eRadioState )
		{
			case RadioState::RS_HMCalc:
			{
				// Verify first if checkbox is not disabled.
				if( false == _IsCheckboxEnabled( ht.hItem ) )
				{
					return;
				}

				// Change state.
				bool fCurrentCheck = _IsCheckboxChecked( ht.hItem );
				_SetCheckboxCheck( ht.hItem, !fCurrentCheck );

				// Check/Uncheck children according to the parent.
				_CheckAllHMChildren( ht.hItem, !fCurrentCheck );

				if( OpeningPage::OP_HMPipes == m_eCurrentPage
					|| OpeningPage::OP_HMCircuit == m_eCurrentPage
					|| OpeningPage::OP_HMTemp == m_eCurrentPage )
				{
					// Send information to the current page that the selected hItems has been changed.
					CDlgCtrlPropPage *pPage = dynamic_cast<CDlgCtrlPropPage *>( ( CDialogEx * )m_ComboBox.GetItemData( m_ComboBox.GetCurSel() ) );
					ASSERT( NULL != pPage );

					if( NULL == pPage )
					{
						return;
					}

					pPage->SendMessage( WM_USER_HMTREEITEMCHECKCHANGED, 0, ( LPARAM )&m_Tree );
				}
				else
				{
					_VerifyButtonStart();
				}
			}

			_VerifyRootItem();
			break;

			case RadioState::RS_IndSel:
				_ClickOnControlValveType( ht.hItem );
				break;
		}
	}

	*pResult = 0;
}

void CDlgSearchReplace::OnBnClickedCheckAll()
{
	if( RadioState::RS_HMCalc == m_eRadioState )
	{
		// Check the parent.
		if( true == _IsCheckboxEnabled( m_Tree.GetRootItem() ) && false == _IsCheckboxChecked( m_Tree.GetRootItem() ) )
		{
			_SetCheckboxCheck( m_Tree.GetRootItem(), true );
		}

		_CheckAllHMChildren( m_Tree.GetRootItem(), true );

		// Send information to the current page that the selected hItems has been changed.
		CDlgCtrlPropPage *pPage = dynamic_cast<CDlgCtrlPropPage *>( ( CDialogEx * )m_ComboBox.GetItemData( m_ComboBox.GetCurSel() ) );
		ASSERT( NULL != pPage );

		if( NULL == pPage )
		{
			return;
		}

		pPage->SendMessage( WM_USER_HMTREEITEMCHECKCHANGED, 0, ( LPARAM )&m_Tree );
	}
	else
	{
		_CheckAllControlValveType( true );
		m_BtnStart.EnableWindow( TRUE );
	}
}

void CDlgSearchReplace::OnBnClickedUncheckAll()
{
	if( RadioState::RS_HMCalc == m_eRadioState )
	{
		// Uncheck the parent.
		if( true == _IsCheckboxEnabled( m_Tree.GetRootItem() ) && true == _IsCheckboxChecked( m_Tree.GetRootItem() ) )
		{
			_SetCheckboxCheck( m_Tree.GetRootItem(), false );
		}

		_CheckAllHMChildren( m_Tree.GetRootItem(), false );

		// Send information to the current page that the selected hItems has been changed.
		CDlgCtrlPropPage *pPage = dynamic_cast<CDlgCtrlPropPage *>( ( CDialogEx * )m_ComboBox.GetItemData( m_ComboBox.GetCurSel() ) );
		ASSERT( NULL != pPage );

		if( NULL == pPage )
		{
			return;
		}

		pPage->SendMessage( WM_USER_HMTREEITEMCHECKCHANGED, 0, ( LPARAM )&m_Tree );
	}
	else
	{
		_CheckAllControlValveType( false );
		m_BtnStart.EnableWindow( FALSE );
	}
}

void CDlgSearchReplace::OnBnClickedExpandAll()
{
	m_Tree.ExpandRecursive( m_Tree.GetRootItem(), TVE_EXPAND );
}

void CDlgSearchReplace::OnBnClickedCollapseAll()
{
	m_Tree.ExpandRecursive( m_Tree.GetRootItem(), TVE_COLLAPSE );
	m_Tree.Expand( m_Tree.GetRootItem(), TVE_EXPAND );
}

void CDlgSearchReplace::OnBnClickedStart()
{
	// Send information to the current page that the selected hItems has been changed.
	CDlgCtrlPropPage *pPage = dynamic_cast<CDlgCtrlPropPage *>( ( CDialogEx * )m_ComboBox.GetItemData( m_ComboBox.GetCurSel() ) );
	ASSERT( NULL != pPage );

	if( NULL == pPage )
	{
		return;
	}

	// Prepare parameters.
	_PrepareParameters();
	pPage->OnBnClickedBtnStart( &m_vecParams );

	_ReInitializeLayout();
}

void CDlgSearchReplace::OnBnClickedCancel()
{
	// Reset the tree.
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_RESETHMTREE, ( WPARAM )false, ( LPARAM )0 );
	CDialogEx::OnCancel();
}

void CDlgSearchReplace::_InitializeLayout( OpeningPage eOpeningPage )
{
	m_eCurrentPage = eOpeningPage;
	m_ComboBox.ResetContent();

	switch( m_eCurrentPage )
	{
	    case OpeningPage::OP_HMTemp:
		case OpeningPage::OP_HMPipes:
		case OpeningPage::OP_HMCircuit:
			_InitLayoutHMCalc();
			_AddPageToComboBox( &m_PageTemp );
			_AddPageToComboBox( &m_PagePipes );
			_AddPageToComboBox( &m_PageCircuit );
			_AddPageToComboBox( &m_PageSet );
			_AddPageToComboBox( &m_PageActuator );
			break;

		case OpeningPage::OP_SetGroupOrSplit:
		case OpeningPage::OP_ActrAddOrRemove:
			_InitLayoutIndSel();
			_AddPageToComboBox( &m_PageSet );
			_AddPageToComboBox( &m_PageActuator );
			break;
	}

	m_PageActuator.SetHMCalcOrIndSel( ( int )m_eRadioState );

	// Show the desire page.
	CDlgCtrlPropPage *pPage = NULL;
	int iItem = 0;
	bool fFound = false;

	for( ; iItem < m_ComboBox.GetCount() && false == fFound; )
	{
		pPage = dynamic_cast<CDlgCtrlPropPage *>( ( CDialogEx * )m_ComboBox.GetItemData( iItem ) );
		ASSERT( NULL != pPage );

		if( NULL != pPage && m_eCurrentPage == ( OpeningPage )pPage->GetPageNumber() )
		{
			fFound = true;
		}
		else
		{
			iItem++;
		}
	}

	if( true == fFound && NULL != pPage )
	{
		m_ComboBox.SetCurSel( iItem );
		_DisplayLeftPane( pPage );
	}
}

void CDlgSearchReplace::_ReInitializeLayout( void )
{
	switch( m_eRadioState )
	{
		case RadioState::RS_HMCalc:
			_InitLayoutHMCalc();
			break;

		case RadioState::RS_IndSel:
			_InitLayoutIndSel();
			break;
	}

	_VerifyButtonStart();
}

void CDlgSearchReplace::_CreateTree( void )
{
	if( NULL != m_Tree.GetSafeHwnd() )
	{
		m_Tree.SetImageList( ( CImageList * )NULL, TVSIL_NORMAL );
		m_Tree.SetImageList( ( CImageList * )NULL, TVSIL_STATE );
		m_Tree.DestroyWindow();

		if( NULL != m_pclImgListCheckBox )
		{
			delete m_pclImgListCheckBox;
			m_pclImgListCheckBox = NULL;
		}
	}

	CRect rect;
	GetDlgItem( IDC_FRAMETREE )->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Tree.Create( WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP, rect, this, IDC_CURRPROJTREE );
	m_Tree.SetExtendedStyle( TVS_EX_HIDEHEADERS );

	// Attach an image list to 'm_Tree'.
	if( RadioState::RS_HMCalc == m_eRadioState )
	{
		CImageList *pclImgListHMTree = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_HMTree );
		ASSERT( NULL != pclImgListHMTree );

		if( NULL != pclImgListHMTree )
		{
			m_Tree.SetImageList( pclImgListHMTree, TVSIL_NORMAL );
		}
	}

	// Attach an image list to 'm_Tree'.
	m_pclImgListCheckBox = TASApp.GetpRCImageManager()->GetImageListCopy( CRCImageManager::ILN_CheckBox, true );
	ASSERT( NULL != m_pclImgListCheckBox );

	if( NULL != m_pclImgListCheckBox )
	{
		m_Tree.SetImageList( m_pclImgListCheckBox, TVSIL_STATE );
	}

	// User data.
	m_Tree.SetUserDataSize( sizeof( CHMTreeListCtrl::m_UserData ) );
	m_Tree.InsertColumn( 0, L"" );
}

void CDlgSearchReplace::_SetItemStateToDefault( HTREEITEM hItem )
{
	_SetCheckboxEnable( hItem, true );

	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		// Do the recursive function.
		_SetItemStateToDefault( hChildItem );
	}
}

void CDlgSearchReplace::_DisplayLeftPane( CDlgCtrlPropPage *pPage )
{
	pPage->SetTree( ( LPARAM )( &m_Tree ) );
	DisplayPage( pPage );
	pPage->Init();
}

bool CDlgSearchReplace::_AddPageToComboBox( CDlgCtrlPropPage *pPage )
{
	int iItem = m_ComboBox.AddString( pPage->GetPageName() );

	if( CB_ERR == iItem )
	{
		return false;
	}

	iItem = m_ComboBox.SetItemData( iItem, ( DWORD_PTR )pPage );

	if( CB_ERR == iItem )
	{
		return false;
	}

	return true;
}

void CDlgSearchReplace::_PrepareParameters( void )
{
	m_vecParams.clear();

	switch( m_eCurrentPage )
	{
		case OpeningPage::OP_HMPipes:
			break;

		case OpeningPage::OP_HMCircuit:
			break;

		case OpeningPage::OP_SetGroupOrSplit:
		case OpeningPage::OP_ActrAddOrRemove:
			switch( m_eRadioState )
			{
				case RadioState::RS_HMCalc:
				{
					int iPos = 0;
					std::map<int, CDS_HydroMod *> mapSelectedCircuits;
					GetSelectedModules( &mapSelectedCircuits, iPos, CDS_HydroMod::eHMObj::eNone, true, true );
					m_vecSelectedCicuits.clear();

					for( std::map<int, CDS_HydroMod *>::iterator iter = mapSelectedCircuits.begin(); iter != mapSelectedCircuits.end(); iter++ )
					{
						m_vecSelectedCicuits.push_back( iter->second );
					}

					m_vecParams.push_back( ( LPARAM )( m_eRadioState ) );
					m_vecParams.push_back( ( LPARAM )( &m_vecSelectedCicuits ) );
				}
				break;

				case RadioState::RS_IndSel:
					for( int iLoopSSelCtrl = 0; iLoopSSelCtrl < ( int )m_vecSSelCtrl.size(); )
					{
						bool fRemoved = false;

						for( int iLoopCVType = ControlValveType::CVT_CV; iLoopCVType < ControlValveType::CVT_Last && false == fRemoved; iLoopCVType++ )
						{
							if( 0 == m_mapControlValveList.count( ( ControlValveType )iLoopCVType ) )
							{
								continue;
							}

							std::vector<CDS_SSelCtrl *>::iterator iter = std::find(	m_mapControlValveList[( ControlValveType )iLoopCVType].begin(),
									m_mapControlValveList[( ControlValveType )iLoopCVType].end(),	m_vecSSelCtrl[iLoopSSelCtrl] );

							if( iter != m_mapControlValveList[( ControlValveType )iLoopCVType].end() && false == m_mapCVCheckboxCheckState[( ControlValveType )iLoopCVType] )
							{
								m_vecSSelCtrl.erase( m_vecSSelCtrl.begin() + iLoopSSelCtrl );
								fRemoved = true;
							}
						}

						if( false == fRemoved )
						{
							iLoopSSelCtrl++;
						}
					}

					m_vecParams.push_back( ( LPARAM )( m_eRadioState ) );
					m_vecParams.push_back( ( LPARAM )( &m_vecSSelCtrl ) );
					break;
			}

			break;
	}
}

void CDlgSearchReplace::_InitLayoutHMCalc()
{
	m_BtnExpand.EnableWindow( TRUE );
	m_BtnCollapse.EnableWindow( TRUE );
	m_pHMHighlighted = NULL;

	// Change the tree header title.
	CString str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_TREEHEADER );
	m_ExtStaticHeader.SetWindowText( str );

	if( false == m_bHMTreeInitialized )
	{
		// Create the tree.
		_CreateTree();

		// Fill the tree.
		CTable *pTab = TASApp.GetpTADS()->GetpHydroModTable();
		ASSERT( NULL != pTab );
		
		CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
		ASSERT( NULL != pPrjRef );

		if( NULL != pTab && NULL != pPrjRef )
		{
			m_Tree.FillHMRoot( pTab, pPrjRef->GetString( CDS_ProjectRef::Name ) );
			m_Tree.Expand( m_Tree.GetRootItem(), TVE_EXPAND );
		}

		m_bHMTreeInitialized = true;
	}

	// To force all node states to be well set.
	//_SetCheckboxCheck( m_Tree.GetRootItem(), false );
	_SetItemStateToDefault( m_Tree.GetRootItem() );
	ResetItemsColor(_TAH_BLACK);

	if( OpeningPage::OP_SetGroupOrSplit == m_eCurrentPage )
	{
		// We are in the page where user can group control valve and actuator to form a set or where he can separate set
		// into separated components.
		_PrepareForSetHM( m_Tree.GetRootItem() );
	}
	else if( OpeningPage::OP_ActrAddOrRemove == m_eCurrentPage )
	{
		// We are in the page where user can add or remove actuator on control valve.
		_PrepareForActuatorHM( m_Tree.GetRootItem() );
	}
}

void CDlgSearchReplace::_PrepareForSetHM( HTREEITEM hItem )
{
	if( true == m_PageSet.IsRadioSplitChecked() )
	{
		// We are in the page where user can split control valve and actuator set into separated components.

		for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
		{
			CDS_HydroMod *pclHM = ( CDS_HydroMod * )m_Tree.GetItemData( hChildItem );

			if( NULL == pclHM )
			{
				continue;
			}

			bool fEnable = false;

			if( true == pclHM->IsCvExist( true ) )
			{
				// If actuator exist...
				if( _NULL_IDPTR != pclHM->GetpCV()->GetActrIDPtr() )
				{
					// If selection is a set...
					if( eBool3::eb3True == pclHM->GetpCV()->GetActrSelectedAsaPackage() )
					{
						// We must verify if control valve and actuator are well belonging to a set.
						CTableSet *pCVActSetTab = pclHM->GetpCV()->GetpCV()->GetTableSet();

						if( NULL == pCVActSetTab )
						{
							continue;
						}

						CDB_Actuator *pclActuator = ( CDB_Actuator * )( pclHM->GetpCV()->GetActrIDPtr().MP );

						if( NULL == pclActuator )
						{
							continue;
						}

						CDB_Set *pCVActSet = pCVActSetTab->FindCompatibleSet( pclHM->GetpCV()->GetpCV()->GetIDPtr().ID, pclActuator->GetIDPtr().ID );

						if( NULL != pCVActSet )
						{
							fEnable = true;
						}
					}
				}
			}

			_SetCheckboxEnable( hChildItem, fEnable );

			if( NULL != m_Tree.GetChildItem( hChildItem ) )
			{
				_PrepareForSetHM( hChildItem );
			}
		}
	}
	else
	{
		// We are in the page where user can group control valve and actuator into set.

		for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
		{
			CDS_HydroMod *pclHM = ( CDS_HydroMod * )m_Tree.GetItemData( hChildItem );

			if( NULL == pclHM )
			{
				continue;
			}

			bool fEnable = false;

			if( true == pclHM->IsCvExist( true ) )
			{
				// If actuator exist...
				if( _NULL_IDPTR != pclHM->GetpCV()->GetActrIDPtr() )
				{
					CTableSet *pCVActSetTab = pclHM->GetpCV()->GetpCV()->GetTableSet();

					if( NULL == pCVActSetTab )
					{
						continue;
					}

					CDB_Set *pCVActSet = pCVActSetTab->FindCompatibleSet( pclHM->GetpCV()->GetpCV()->GetIDPtr().ID, pclHM->GetpCV()->GetActrIDPtr().ID );

					if( NULL != pCVActSet && eBool3::eb3True != pclHM->GetpCV()->GetActrSelectedAsaPackage() )
					{
						fEnable = true;
					}
				}
			}

			_SetCheckboxEnable( hChildItem, fEnable );

			if( NULL != m_Tree.GetChildItem( hChildItem ) )
			{
				_PrepareForSetHM( hChildItem );
			}
		}
	}

	_VerifyRootItem();
}

void CDlgSearchReplace::_PrepareForActuatorHM( HTREEITEM hItem )
{
	if( true == m_PageActuator.IsRadioRemoveChecked() )
	{
		// We are in the page where user can remove actuators from control valves.

		bool fRemoveForSet = m_PageActuator.IsCheckRemoveForSet();

		for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
		{
			CDS_HydroMod *pclHM = ( CDS_HydroMod * )m_Tree.GetItemData( hChildItem );

			if( NULL == pclHM )
			{
				continue;
			}

			bool fEnable = false;

			if( true == pclHM->IsCvExist( true ) )
			{
				// If actuator exist...
				if( _NULL_IDPTR != pclHM->GetpCV()->GetActrIDPtr() )
				{
					fEnable = true;

					// If user enabled selection by set...
					if( eBool3::eb3True == pclHM->GetpCV()->GetActrSelectedAsaPackage() && false == fRemoveForSet )
					{
						// We must verify if control valve and actuator are belonging to a set.
						CTableSet *pCVActSetTab = pclHM->GetpCV()->GetpCV()->GetTableSet();
						ASSERT( NULL != pCVActSetTab );

						if( NULL == pCVActSetTab )
						{
							continue;
						}

						CDB_Actuator *pclActuator = ( CDB_Actuator * )( pclHM->GetpCV()->GetActrIDPtr().MP );

						if( NULL == pclActuator )
						{
							continue;
						}

						CDB_Set *pCVActSet = pCVActSetTab->FindCompatibleSet( pclHM->GetpCV()->GetpCV()->GetIDPtr().ID, pclActuator->GetIDPtr().ID );

						if( NULL != pCVActSet )
						{
							fEnable = false;
						}
					}
				}
			}

			_SetCheckboxEnable( hChildItem, fEnable );

			if( NULL != m_Tree.GetChildItem( hChildItem ) )
			{
				_PrepareForActuatorHM( hChildItem );
			}
		}
	}
	else
	{
		// We are in the page where user can add actuators on control valves.
		for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
		{
			CDS_HydroMod *pclHM = ( CDS_HydroMod * )m_Tree.GetItemData( hChildItem );

			if( NULL == pclHM )
			{
				continue;
			}

			bool fEnable = false;

			if( true == pclHM->IsCvExist( true ) && true == pclHM->GetpCV()->IsTaCV() )
			{
				// If actuator doesn't exist...
				if( _NULL_IDPTR == pclHM->GetpCV()->GetActrIDPtr() )
				{
					fEnable = true;
				}
			}

			_SetCheckboxEnable( hChildItem, fEnable );

			if( NULL != m_Tree.GetChildItem( hChildItem ) )
			{
				_PrepareForActuatorHM( hChildItem );
			}
		}
	}

	_VerifyRootItem();
}

void CDlgSearchReplace::_CheckAllHMChildren( HTREEITEM hItem, bool fCheck )
{
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		// Check/Uncheck the box.
		CDS_HydroMod *pclHM = ( CDS_HydroMod * )m_Tree.GetItemData( hChildItem );

		if( true == _IsCheckboxEnabled( hChildItem ) && fCheck != _IsCheckboxChecked( hChildItem ) )
		{
			_SetCheckboxCheck( hChildItem, fCheck );
		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			_CheckAllHMChildren( hChildItem, fCheck );
		}
	}
}

void CDlgSearchReplace::_InitLayoutIndSel()
{
	m_BtnExpand.EnableWindow( FALSE );
	m_BtnCollapse.EnableWindow( FALSE );
	m_BtnStart.EnableWindow( FALSE );
	m_pHMHighlighted = NULL;

	// Change the tree header title.
	CString str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_TREEHEADERINDSEL );
	m_ExtStaticHeader.SetWindowText( str );

	// Fill the tree.
	_CreateTree();
	m_Tree.DeleteAllItems();
	CTADatastruct *pTADS = TASApp.GetpTADS();
	ASSERT( NULL != pTADS );

	if( NULL == pTADS )
	{
		return;
	}

	m_bHMTreeInitialized = false;

	if( false == m_bIndSelTreeInitialized )
	{
		m_mapControlValveList.clear();
		CTable *pTable = ( CTable * )( pTADS->Get( _T("CTRLVALVE_TAB") ).MP );
		ASSERT( NULL != pTable );

		if( NULL != pTable && '\0' != pTable->GetFirst().ID )
		{
			// Run all the table to check what are the control valve types we have.
			for( IDPTR idptr = pTable->GetFirst(); '\0' != *idptr.ID; idptr = pTable->GetNext() )
			{
				CDS_SSelCtrl *pclSSelCtrl = dynamic_cast<CDS_SSelCtrl *>( idptr.MP );

				if( NULL == pclSSelCtrl )
				{
					continue;
				}

				if( true == pclSSelCtrl->IsFromDirSel() )
				{
					continue;
				}

				// All control products are based on 'CDB_RegulatingValve'.
				CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( pclSSelCtrl->GetProductIDPtr().MP );

				if( NULL == pclRegulatingValve )
				{
					continue;
				}

				ControlValveType eControlValveType;

				if( true == pclRegulatingValve->IsaCV() )
				{
					eControlValveType = ControlValveType::CVT_CV;
				}
				else if( true == pclRegulatingValve->IsaBCV() )
				{
					eControlValveType = ControlValveType::CVT_BCV;
				}
				else if( true == pclRegulatingValve->IsaPICV() )
				{
					eControlValveType = ControlValveType::CVT_PICV;
				}

				m_mapControlValveList[eControlValveType].push_back( pclSSelCtrl );
			}
		}

		m_bIndSelTreeInitialized = true;
	}

	if( OpeningPage::OP_SetGroupOrSplit == m_eCurrentPage )
	{
		// We are in the page where user can group control valve and actuator to form a set or where he can separate set
		// into separated components.
		_PrepareForSetIndSel();
	}
	else if( OpeningPage::OP_ActrAddOrRemove == m_eCurrentPage )
	{
		// We are in the page where user can add or remove actuator on control valve.
		_PrepareForActuatorIndSel();
	}
}

void CDlgSearchReplace::_PrepareForSetIndSel( void )
{
	m_vecSSelCtrl.clear();

	if( true == m_PageSet.IsRadioSplitChecked() )
	{
		// We are in the page where user can split control valve and actuator set into separated components.

		// We must check if there are some set in list.
		m_mapCVCheckboxEnableState.clear();

		for( int iLoopCVType = ControlValveType::CVT_CV; iLoopCVType < ControlValveType::CVT_LastSet; iLoopCVType++ )
		{
			m_mapCVCheckboxCheckState[( ControlValveType )iLoopCVType] = false;
			m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] = false;

			if( 0 == m_mapControlValveList.count( ( ControlValveType )iLoopCVType ) )
			{
				continue;
			}

			for( int iLoopIndSel = 0; iLoopIndSel < ( int )m_mapControlValveList[( ControlValveType )iLoopCVType].size(); iLoopIndSel++ )
			{
				CDS_SSelCtrl *pclSSelCtrl = m_mapControlValveList[( ControlValveType )iLoopCVType][iLoopIndSel];

				if( NULL == pclSSelCtrl )
				{
					continue;
				}

				if( true == pclSSelCtrl->IsSelectedAsAPackage() )
				{
					m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] = true;
					m_vecSSelCtrl.push_back( pclSSelCtrl );
				}
			}
		}
	}
	else
	{
		// We are in the page where user can group control valve and actuator into set.

		// We must check for each list if at least one control valve and actuator can be group into a set.
		m_mapCVCheckboxEnableState.clear();

		for( int iLoopCVType = ControlValveType::CVT_CV; iLoopCVType < ControlValveType::CVT_LastSet; iLoopCVType++ )
		{
			m_mapCVCheckboxCheckState[( ControlValveType )iLoopCVType] = false;
			m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] = false;

			if( 0 == m_mapControlValveList.count( ( ControlValveType )iLoopCVType ) )
			{
				continue;
			}

			for( int iLoopIndSel = 0; iLoopIndSel < ( int )m_mapControlValveList[( ControlValveType )iLoopCVType].size(); iLoopIndSel++ )
			{
				CDS_SSelCtrl *pclSSelCtrl = m_mapControlValveList[( ControlValveType )iLoopCVType][iLoopIndSel];

				if( NULL == pclSSelCtrl )
				{
					continue;
				}

				// If no actuator has been selected with this control valve, we can't group!
				if( _NULL_IDPTR == pclSSelCtrl->GetActrIDPtr() )
				{
					continue;
				}

				if( false == pclSSelCtrl->IsSelectedAsAPackage() && _T('\0') == pclSSelCtrl->GetCvActrSetIDPtr().ID )
				{
					m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] = true;
					m_vecSSelCtrl.push_back( m_mapControlValveList[( ControlValveType )iLoopCVType][iLoopIndSel] );
				}
			}
		}
	}

	// Show checkbox.
	for( int iLoopCVType = ControlValveType::CVT_CV; iLoopCVType < ControlValveType::CVT_LastSet; iLoopCVType++ )
	{
		CString str;

		switch( iLoopCVType )
		{
			case ControlValveType::CVT_CV:
				str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_CV );
				break;

			case ControlValveType::CVT_BCV:
				str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_BCV );
				break;

			case ControlValveType::CVT_PICV:
				str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_PICV );
				break;
		}

		HTREEITEM hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, str, 0, 0, 0, 0, ( LPARAM )iLoopCVType, TVI_ROOT, TVI_LAST );
		_SetCheckboxEnable( hItem, m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] );
	}
}

void CDlgSearchReplace::_PrepareForActuatorIndSel( void )
{
	if( true == m_PageActuator.IsRadioRemoveChecked() )
	{
		// We are in the page where user can remove actuators from control valves.

		// We must check if there are some control valve with actuator in the list.
		bool fRemoveForSet = m_PageActuator.IsCheckRemoveForSet();
		m_vecSSelCtrl.clear();
		m_mapCVCheckboxEnableState.clear();

		for( int iLoopCVType = ControlValveType::CVT_CV; iLoopCVType < ControlValveType::CVT_LastSet; iLoopCVType++ )
		{
			m_mapCVCheckboxCheckState[( ControlValveType )iLoopCVType] = false;
			m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] = false;

			if( 0 == m_mapControlValveList.count( ( ControlValveType )iLoopCVType ) )
			{
				continue;
			}

			for( int iLoopIndSel = 0; iLoopIndSel < ( int )m_mapControlValveList[( ControlValveType )iLoopCVType].size(); iLoopIndSel++ )
			{
				CDS_SSelCtrl *pclSSelCtrl = m_mapControlValveList[( ControlValveType )iLoopCVType][iLoopIndSel];

				if( NULL == pclSSelCtrl )
				{
					continue;
				}

				// If actuator exist...
				if( _NULL_IDPTR != pclSSelCtrl->GetActrIDPtr() )
				{
					// If selection by set...
					if( true == pclSSelCtrl->IsSelectedAsAPackage() && false == fRemoveForSet )
					{
						continue;
					}

					m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] = true;
					m_vecSSelCtrl.push_back( pclSSelCtrl );
				}
			}
		}
	}
	else
	{
		// We are in the page where user can add actuators on control valves.

		// We must check for each list if there is a least one control valve without actuator.
		m_vecSSelCtrl.clear();
		m_mapCVCheckboxEnableState.clear();

		for( int iLoopCVType = ControlValveType::CVT_CV; iLoopCVType < ControlValveType::CVT_LastSet; iLoopCVType++ )
		{
			m_mapCVCheckboxCheckState[( ControlValveType )iLoopCVType] = false;
			m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] = false;

			if( 0 == m_mapControlValveList.count( ( ControlValveType )iLoopCVType ) )
			{
				continue;
			}

			for( int iLoopIndSel = 0; iLoopIndSel < ( int )m_mapControlValveList[( ControlValveType )iLoopCVType].size(); iLoopIndSel++ )
			{
				CDS_SSelCtrl *pclSSelCtrl = m_mapControlValveList[( ControlValveType )iLoopCVType][iLoopIndSel];

				if( NULL == pclSSelCtrl )
				{
					continue;
				}

				// If actuator doesn't exist...
				if( _NULL_IDPTR == pclSSelCtrl->GetActrIDPtr() )
				{
					m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] = true;
					m_vecSSelCtrl.push_back( pclSSelCtrl );
				}
			}
		}
	}

	// Show checkbox.
	for( int iLoopCVType = ControlValveType::CVT_CV; iLoopCVType < ControlValveType::CVT_LastSet; iLoopCVType++ )
	{
		CString str;

		switch( iLoopCVType )
		{
			case ControlValveType::CVT_CV:
				str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_CV );
				break;

			case ControlValveType::CVT_BCV:
				str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_BCV );
				break;

			case ControlValveType::CVT_PICV:
				str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_PICV );
				break;

				// 			case ControlValveType::CVT_TRV:
				// 				str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_TRV );
				// 				break;
				//
				// 			case ControlValveType::CVT_FLCV:
				// 				str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_FLCV );
				// 				break;
		}

		HTREEITEM hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, str, 0, 0, 0, 0, ( LPARAM )iLoopCVType, TVI_ROOT, TVI_LAST );
		_SetCheckboxEnable( hItem, m_mapCVCheckboxEnableState[( ControlValveType )iLoopCVType] );
	}
}

void CDlgSearchReplace::_ClickOnControlValveType( HTREEITEM hItem )
{
	// Verify first if checkbox is not disabled.
	if( false == _IsCheckboxEnabled( hItem ) )
	{
		return;
	}

	ControlValveType eControlValveType = ( ControlValveType )m_Tree.GetItemData( hItem );
	bool fCheck = _IsCheckboxChecked( hItem );
	_SetCheckboxCheck( hItem, !fCheck );
	m_mapCVCheckboxCheckState[eControlValveType] = !fCheck;
	_VerifyButtonStart();
}

void CDlgSearchReplace::_CheckAllControlValveType( bool fCheck )
{
	HTREEITEM hItem = m_Tree.GetRootItem();

	while( NULL != hItem )
	{
		ControlValveType eControlValveType = ( ControlValveType )m_Tree.GetItemData( hItem );

		if( true == _IsCheckboxEnabled( hItem ) && fCheck != _IsCheckboxChecked( hItem ) )
		{
			_SetCheckboxCheck( hItem, fCheck );
			m_mapCVCheckboxCheckState[eControlValveType] = fCheck;
		}

		hItem = m_Tree.GetNextItem( hItem, TVGN_NEXT );
	}
}

bool CDlgSearchReplace::_IsCheckboxEnabled( HTREEITEM hItem )
{
	// Verify first if checkbox is not disabled.
	int iCurrentState = ( m_Tree.GetItemState( hItem, TVIS_STATEIMAGEMASK ) >> 12 ) - 1;
	bool fEnable = ( CRCImageManager::ILCB_NotSelected == iCurrentState || CRCImageManager::ILCB_Selected == iCurrentState ) ? true : false;
	return fEnable;
}

void CDlgSearchReplace::_SetCheckboxEnable( HTREEITEM hItem, bool fEnable )
{
	int iImage = ( true == fEnable ) ? CRCImageManager::ILCB_NotSelected + 1 : CRCImageManager::ILCB_NotSelectedDisable + 1;

	if( _IsCheckboxChecked( hItem ) )
	{
		iImage = ( true == fEnable ) ? CRCImageManager::ILCB_Selected + 1 : CRCImageManager::ILCB_NotSelectedDisable + 1;
	}

	m_Tree.SetItemState( hItem, INDEXTOSTATEIMAGEMASK( iImage ), TVIS_STATEIMAGEMASK );
}

bool CDlgSearchReplace::_IsCheckboxChecked( HTREEITEM hItem )
{
	int iCurrentState = ( m_Tree.GetItemState( hItem, TVIS_STATEIMAGEMASK ) >> 12 ) - 1;
	bool fCurrentCheck = ( CRCImageManager::ILCB_Selected == iCurrentState || CRCImageManager::ILCB_SelectedDisable == iCurrentState ) ? true : false;
	return fCurrentCheck;
}

void CDlgSearchReplace::_SetCheckboxCheck( HTREEITEM hItem, bool fCheck )
{
	int iImage = ( true == fCheck ) ? CRCImageManager::ILCB_SelectedDisable + 1 : CRCImageManager::ILCB_NotSelectedDisable + 1;

	if( _IsCheckboxEnabled( hItem ) )
	{
		iImage = ( true == fCheck ) ? CRCImageManager::ILCB_Selected + 1 : CRCImageManager::ILCB_NotSelected + 1;
	}

	m_Tree.SetItemState( hItem, INDEXTOSTATEIMAGEMASK( iImage ), TVIS_STATEIMAGEMASK );
}

bool CDlgSearchReplace::_IsAtLeastOneCheckedEnabled( HTREEITEM hItem )
{
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		int iCurrentState = ( m_Tree.GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) >> 12 ) - 1;
		bool fCurrentCheck = ( CRCImageManager::ILCB_Selected == iCurrentState ) ? true : false;

		if( true == fCurrentCheck )
		{
			return true;
		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			if( true == _IsAtLeastOneCheckedEnabled( hChildItem ) )
			{
				return true;
			}
		}
	}

	return false;
}

bool CDlgSearchReplace::_IsAllChecked( HTREEITEM hItem )
{
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		int iCurrentState = ( m_Tree.GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) >> 12 ) - 1;
		bool fCurrentCheck = ( CRCImageManager::ILCB_Selected == iCurrentState ) ? true : false;

		if( false == fCurrentCheck )
		{
			return false;
		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			if( false == _IsAllChecked( hChildItem ) )
			{
				return false;
			}
		}
	}

	return true;
}

bool CDlgSearchReplace::_IsAllUnchecked( HTREEITEM hItem )
{
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		int iCurrentState = ( m_Tree.GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) >> 12 ) - 1;
		bool fCurrentCheck = ( CRCImageManager::ILCB_NotSelected == iCurrentState ) ? true : false;

		if( false == fCurrentCheck )
		{
			return false;
		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			if( false == _IsAllUnchecked( hChildItem ) )
			{
				return false;
			}
		}
	}

	return true;
}

bool CDlgSearchReplace::_IsAllEnabled( HTREEITEM hItem )
{
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		int iCurrentState = ( m_Tree.GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) >> 12 ) - 1;
		bool fCurrentEnable = ( CRCImageManager::ILCB_Selected == iCurrentState || CRCImageManager::ILCB_NotSelected == iCurrentState ) ? true : false;

		if( false == fCurrentEnable )
		{
			return false;
		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			if( false == _IsAllEnabled( hChildItem ) )
			{
				return false;
			}
		}
	}

	return true;
}

bool CDlgSearchReplace::_IsAllDisabled( HTREEITEM hItem )
{
	for( HTREEITEM hChildItem = m_Tree.GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
	{
		int iCurrentState = ( m_Tree.GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) >> 12 ) - 1;
		bool fCurrentDisabled = ( CRCImageManager::ILCB_SelectedDisable == iCurrentState || CRCImageManager::ILCB_NotSelectedDisable == iCurrentState ) ? true : false;

		if( false == fCurrentDisabled )
		{
			return false;
		}

		// Do the recursive function.
		if( NULL != m_Tree.GetChildItem( hChildItem ) )
		{
			if( false == _IsAllDisabled( hChildItem ) )
			{
				return false;
			}
		}
	}

	return true;
}

void CDlgSearchReplace::_VerifyRootItem( void )
{
	bool fEnable = true;

	if( true == _IsAllEnabled( m_Tree.GetRootItem() ) )
	{
		_SetCheckboxEnable( m_Tree.GetRootItem(), true );
	}
	else if( true == _IsAllDisabled( m_Tree.GetRootItem() ) )
	{
		_SetCheckboxEnable( m_Tree.GetRootItem(), false );
		fEnable = false;
	}

	if( true == fEnable )
	{
		if( true == _IsAllChecked( m_Tree.GetRootItem() ) )
		{
			_SetCheckboxCheck( m_Tree.GetRootItem(),  true );
		}
		else if( true == _IsAllUnchecked( m_Tree.GetRootItem() ) )
		{
			_SetCheckboxCheck( m_Tree.GetRootItem(), false );
		}
	}
}

void CDlgSearchReplace::_VerifyButtonStart( void )
{
	BOOL fEnable = FALSE;

	switch( m_eRadioState )
	{
		case RadioState::RS_HMCalc:
			if( true == _IsAtLeastOneCheckedEnabled( m_Tree.GetRootItem() ) )
			{
				fEnable = TRUE;
			}

			break;

		case RadioState::RS_IndSel:
			for( int iLoop = ( int )ControlValveType::CVT_CV; iLoop < ( int )ControlValveType::CVT_Last && FALSE == fEnable; iLoop++ )
			{
				if( 0 == m_mapCVCheckboxCheckState.count( ( ControlValveType )iLoop ) )
				{
					continue;
				}

				if( true == m_mapCVCheckboxCheckState[( ControlValveType )iLoop] )
				{
					fEnable = TRUE;
				}
			}

			break;
	}

	m_BtnStart.EnableWindow( fEnable );
}

BEGIN_MESSAGE_MAP( CDlgSearchReplace::CMyRadioButton, CButton )
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()
void CDlgSearchReplace::CMyRadioButton::OnSetFocus( CWnd *pOldWnd ) {}
