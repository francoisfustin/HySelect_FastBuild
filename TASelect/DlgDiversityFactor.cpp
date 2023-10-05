#include "StdAfx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "HMTreeDroptarget.h"
#include "HMTreeListCtrl.h"
#include "TCImageList.h"
#include "HydroMod.h"
#include "DlgLTtabctrl.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewSelProd.h"
#include "DlgDiversityFactor.h"

BEGIN_MESSAGE_MAP( CDlgTreeContainer, CDialogEx )
	ON_WM_HSCROLL()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

CDlgTreeContainer::CDlgTreeContainer() : CDialogEx()
{
	m_pclEvent = NULL;
	m_pHM = NULL;
	m_uiPrevSliderPos = 0;
	m_dDiversityFactor = -1.0;
	m_iSliderLeftPos = 0;
	m_bInitialized = false;
}

void CDlgTreeContainer::SetHydroMod( CDS_HydroMod* pHM )
{
	if( NULL == pHM )
		return;

	// To avoid to take into account the 'TVN_ITEMEXPANDED' notify message while tree is being built.
	m_bInitialized = false;

	m_pHM = pHM;
	_FillTree();

	m_bInitialized = true;
}

void CDlgTreeContainer::ChangeColorMode( bool fSet )
{
	TASetVAlternateColorParam rParam;
	rParam.fVAlternateMode = true;
	rParam.fDrawAllLine = fSet;
	rParam.uUnCheckedBkgColor = _TAH_ORANGE_LIGHT;
	rParam.uCheckedBkgColor = _IMI_GRAY_XLIGHT;
	m_Tree.TASetVAlternateColor( &rParam );
	m_Tree.Invalidate();
	m_Tree.UpdateWindow();
}

BOOL CDlgTreeContainer::OnInitDialog()
{
	// Do the default initialization.
	CDialogEx::OnInitDialog();

	// Retrieve 'Rectangle' client area.
	CRect rectDialog;
	GetClientRect( &rectDialog );

	// Create slider control.
	CRect rectSlider( rectDialog );
	rectSlider.bottom = rectSlider.top + _TREECONTAINER_SLIDERHEIGHT;
	m_SliderCtrl.Create( TBS_AUTOTICKS | TBS_HORZ | TBS_BOTTOM | WS_TABSTOP | WS_VISIBLE, rectSlider, this, IDC_DFSLIDER );
	m_SliderCtrl.SetWindowPos( NULL, rectDialog.left, rectDialog.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

	// We have finished we can prevent 'CDlgDiversityFactor'.
	if( NULL != m_pclEvent )
		m_pclEvent->SetEvent();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgTreeContainer::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	HWND hWnd = pScrollBar->GetSafeHwnd();
	if( hWnd == m_SliderCtrl.GetSafeHwnd() )
	{
		// EVENT COMES FROM THE SLIDER CONTROL.

		if( m_SliderCtrl.GetPos() != m_uiPrevSliderPos )
		{
			if( m_SliderCtrl.GetPos() > (int)m_uiPrevSliderPos )
				_SliderCtrlRightMove();
			else
				_SliderCtrlLeftMove();

			// Now we can do the check.
			m_uiPrevSliderPos = m_SliderCtrl.GetPos();
			if( m_uiPrevSliderPos > 0 )
				m_Tree.SetCheckByLevel( m_uiPrevSliderPos - 1, true, true );
			else
				m_Tree.SetCheckByLevel( 0, false, true );
			m_Tree.Invalidate();
			m_Tree.UpdateWindow();
		}
	}
	else if( hWnd == m_Tree.GetSafeHwnd() )
	{
		// EVENT COMES FROM THE HORIZONTAL SCROLLBAR OF THE TREE.
		// Remark: We have two possibilities:
		//  1 - Call comes from 'TreeListProc' (see 'TreeListWnd.cpp'). In that case, that simply means user has used horizontal scrollbar of the tree.
		//      Thus we have to move also the slider control with the same quantity of pixels.
		//  2 - Call comes from 'TAEnsureVisible' (see 'TreeListWnd.cpp'). In that case, user moved slider control. But to be able to completely show items
		//      at the same level, we have left/right scrolled the tree. Thus we want also scroll the slider control and modify current mouse position.

		// Retrieve current position of the horizontal scroll bar.
		SCROLLINFO rScrollInfo;
		ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
		rScrollInfo.cbSize = sizeof( SCROLLINFO );
		rScrollInfo.fMask = SIF_ALL;
		m_Tree.GetScrollInfo( SB_HORZ, &rScrollInfo );
		
		// Retrieve current position of the slider control.
		CRect rectSlider;
		m_SliderCtrl.GetWindowRect( &rectSlider );
		ScreenToClient( &rectSlider );

		rectSlider.left = m_iSliderLeftPos - rScrollInfo.nPos;
		m_SliderCtrl.SetWindowPos( NULL, rectSlider.left, rectSlider.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
		m_SliderCtrl.Invalidate();
		m_SliderCtrl.UpdateWindow();
		
		if( (UINT)-1 == nPos )
		{
			// Get current position
			UINT uiSliderPos = m_SliderCtrl.GetPos();
			CRect rectThumb;
			m_SliderCtrl.GetThumbRect( &rectThumb );
			m_SliderCtrl.SetPos( m_uiPrevSliderPos );
			CRect rectThumb2;
			m_SliderCtrl.GetThumbRect( &rectThumb2 );
			m_SliderCtrl.SetPos( uiSliderPos );

			// Move mouse pointer.
			POINT ptMouse;
			GetCursorPos( &ptMouse );
			ptMouse.x -= nPos;
			ptMouse.x += abs( rectThumb.left - rectThumb2.left ) / 2;
			SetCursorPos( ptMouse.x, ptMouse.y );
		}
	}
	else
		CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDlgTreeContainer::OnSysCommand( UINT nID, LPARAM lParam )
{
	if( SC_HSCROLL == nID)
		int i = 0;

	CDialogEx::OnSysCommand(nID, lParam);
}

BOOL CDlgTreeContainer::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	if( true == m_bInitialized )
	{
		NMHDR* pNMHdr = (NMHDR*)lParam;
		if( TVN_ITEMEXPANDED == pNMHdr->code )
		{
			NMTREEVIEW* pNotify = (NMTREEVIEW*)lParam;
			if( NULL != pNotify && NULL != pNotify->itemNew.hItem )
			{
				_RefreshSliderControl();
				TreeListData* pTreeListData = m_Tree.TAGetTreeListData();
				int i = 0;
// 				SCROLLINFO rScrollInfo;
// 				m_Tree.GetScrollInfo( SB_HORZ, &rScrollInfo );
// 				SendMessage( m_Tree.GetSafeHwnd(), WM_HSCROLL, MAKEWPARAM( SB_THUMBPOSITION, rScrollInfo.nPos ), 0 );
			}
		}
		else if( TVN_CBSTATECHANGED == pNMHdr->code )
		{
			NMTREEVIEW* pNotify = (NMTREEVIEW*)lParam;
			if( NULL != pNotify && NULL != pNotify->itemNew.hItem )
			{
				// Remark: 0x1000 hard coded in 'TreeListWnd'.
				if( 0x1000 == ( pNotify->itemNew.state & TVIS_STATEIMAGEMASK ) )
					m_Tree.SetCheckSpecial( pNotify->itemNew.hItem, true, false, true, false );
				else
					m_Tree.SetCheckSpecial( pNotify->itemNew.hItem, false, true, false, false );
				m_Tree.Invalidate();
				m_Tree.UpdateWindow();
			}
		}
	}

	return CDialogEx::OnNotify( wParam, lParam, pResult );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgTreeContainer::_FillTree( void )
{
	if( NULL == m_pHM )
		return;

	// Because there is no internal reset in 'TreeListWnd' we need to destroy tree and create it again.
	
	// Verify first if tree already exists.
	if( m_Tree.GetSafeHwnd() != NULL )
		m_Tree.DestroyWindow();
	if( m_TreeImageList.GetSafeHandle() != NULL )
		m_TreeImageList.DeleteImageList();
	if( m_TreeCheckBox.GetSafeHandle() != NULL )
		m_TreeCheckBox.DeleteImageList();

	// Prepare image list for 'm_Tree'.
	m_TreeImageList.CreateTC( IDB_IMGLST_HMTREE, 16, 16, _BLACK );
	m_TreeImageList.SetBkColor( CLR_NONE );
	m_TreeCheckBox.Create( IDB_IMGLST_CHECKBOX1616, 16, 1, _BLACK );
	m_TreeCheckBox.SetBkColor( CLR_NONE );

	// Retrieve client area of 'm_TreeContainer'.
	CRect rectDialog;
	GetClientRect( &rectDialog );

	// Create tree.
	CRect rectTree( rectDialog );
	rectTree.top += ( 2 + _TREECONTAINER_SLIDERHEIGHT );
	// Remark: remove two pixels to not overlay border.
	m_Tree.Create( TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_CHECKBOXES | TVS_DISABLEDRAGDROP | WS_TABSTOP | WS_VISIBLE, rectTree, this, IDC_CURRPROJTREE );
	m_Tree.SetExtendedStyle( TVS_EX_BITCHECKBOX | TVS_EX_HIDEHEADERS );
	m_Tree.SetImageList( &m_TreeImageList, TVSIL_NORMAL );

	// Check box image list.
	m_Tree.SetImageList( &m_TreeCheckBox, TVSIL_STATE );
	m_Tree.SetImageList( &m_TreeCheckBox, TVSIL_CHECK );

	// User Data.
	m_Tree.SetUserDataSize( sizeof( CHMTreeListCtrl::m_UserData ) );

	// Fill node with current module.
	m_Tree.FillHMRoot( m_pHM, m_pHM->GetHMName(), true );

	// Expand all.
	m_Tree.Expand( m_Tree.GetRootItem(), TVE_EXPAND );
	
	// Run all the module to check if there is already diversity factor applied.
	std::vector<HTREEITEM> vecTreeList;
	HTREEITEM hRoot = m_Tree.GetRootItem( );
	int iCurrentDeep = 0;
	HTREEITEM hNextItem = hRoot;
	bool fCanStop = false;
	m_dDiversityFactor = -1.0;
	do 
	{
		if( NULL != hNextItem )
		{
			CDS_HydroMod* pHM = dynamic_cast<CDS_HydroMod*>( (CData*)( m_Tree.GetItemData( hNextItem ) ) );
			if( NULL != pHM && pHM->GetDiversityFactor() != -1.0 )
			{
				// Must be optimized later!
				m_Tree.SetItemState( hNextItem, 0, INDEXTOSTATEIMAGEMASK( 1 ), TVIS_STATEIMAGEMASK );
				m_Tree.SetCheckSpecial( hNextItem, true, false, true, false );
				m_dDiversityFactor = pHM->GetDiversityFactor();
			}
		}

		if( NULL == hNextItem )
		{
			if( vecTreeList.size() > 0 )
			{
				iCurrentDeep--;
				hNextItem = vecTreeList.back();
				hNextItem = m_Tree.GetNextSiblingItem( hNextItem );
				vecTreeList.pop_back();
			}
			else
				fCanStop = true;
		}
		else if( NULL != m_Tree.GetChildItem( hNextItem ) )
		{
			iCurrentDeep++;
			vecTreeList.push_back( hNextItem );
			hNextItem = m_Tree.GetChildItem( hNextItem );
		}
		else
			hNextItem = m_Tree.GetNextSiblingItem( hNextItem );
	}while( false == fCanStop );

	// Refresh slider control.
	_RefreshSliderControl();

	// Move slider control.
	_MoveSliderControl();

	// Must be placed after tree initialization because this method will compute what is the largest item.
	ChangeColorMode( true );

	m_bInitialized = true;
}

void CDlgTreeContainer::_RefreshSliderControl( void )
{
	HTREEITEM hRoot = m_Tree.GetRootItem();
	if( NULL == hRoot )
		return;

	short nTreeDeep = m_Tree.GetTreeDeep( true );
	if( -1 == nTreeDeep )
		return;

	m_SliderCtrl.SetRange( 0, ++nTreeDeep );

	// Retrieve shift and indent from tree.
	// Remark: - shift is the number of pixels between from the beginning of one row to the vertical line.
	//         - indent is the number of pixels between each vertical rows.
	TreeListData* pTreeListData = m_Tree.TAGetTreeListData();
	int iShift = pTreeListData->iShift;
	int iIndent = m_Tree.GetIndent();

	// Retrieve current position of the slider control.
	CRect rectSlider;
	m_SliderCtrl.GetWindowRect( &rectSlider );
	ScreenToClient( &rectSlider );

	// Retrieve thumb coordinates at the first position.
	int iOldPos = m_SliderCtrl.GetPos();
	m_SliderCtrl.SetPos( 0 );
	CRect rectThumb;
	m_SliderCtrl.GetThumbRect( &rectThumb );
	m_SliderCtrl.SetPos( iOldPos );

	// Remark: there is 1 pixel border set internally in TreeListWnd.cpp (see TreeListDraw function).
	int iPosToCentrate = 1 + iShift;
	m_iSliderLeftPos = iPosToCentrate - ( ( rectThumb.right - rectThumb.left ) / 2 + rectThumb.left );
		
	rectSlider.left = m_iSliderLeftPos;

	// If there is a current horizontal scrolling, add it to the slider.
	CRect rectItem;
	m_Tree.GetItemRect( hRoot, &rectItem, (BOOL)FALSE );
	rectSlider.left += rectItem.left;

	// Remark: 13 pixels are added to the left and the right of respectively the first and last tic.
	rectSlider.right = rectSlider.left + 2 * 13 + nTreeDeep * iIndent;
	m_SliderCtrl.SetWindowPos( NULL, rectSlider.left, rectSlider.top, rectSlider.Width(), rectSlider.Height(), SWP_NOZORDER );
}

void CDlgTreeContainer::_MoveSliderControl( void )
{
	std::vector<HTREEITEM> vecTreeList;
	HTREEITEM hRoot = m_Tree.GetRootItem( );
	if( NULL == hRoot )
		return;

	// Run all tree to detect for each level if modules are ALL selected or not.
	int iCurrentDeep = 0;
	HTREEITEM hNextItem = hRoot;
	HTREEITEM hWiderItem = NULL;
	std::vector<int> vecLevelState;
	bool fCanStop = false;
	do
	{
		if( NULL != hNextItem )
		{
			// If there is no yet item verified at this level...
			if( iCurrentDeep + 1 > (int)vecLevelState.size() )
				vecLevelState.push_back( ( m_Tree.GetItemCheckBox( hNextItem ) > 0 ) ? 1 : 0 );
			else
			{
				if( 1 == m_Tree.GetItemCheckBox( hNextItem ) )
				{
					// In case of value in vector already exist:
					//  1 - that means previous item at the same level is selected. We must not change value in vector.
					//  0 - that means at least one item at the same level is not selected. In this case, we let this value at 0.
				}
				else
				{
					// Force to 0.
					vecLevelState[iCurrentDeep] = 0;
				}
			}
		}

		if( NULL == hNextItem )
		{
			if( vecTreeList.size() > 0 )
			{
				iCurrentDeep--;
				hNextItem = vecTreeList.back();
				hNextItem = m_Tree.GetNextSiblingItem( hNextItem );
				vecTreeList.pop_back();
			}
			else
				fCanStop = true;
		}
		else if( NULL != m_Tree.GetChildItem( hNextItem ) )
		{
			iCurrentDeep++;
			vecTreeList.push_back( hNextItem );
			hNextItem = m_Tree.GetChildItem( hNextItem );
		}
		else
			hNextItem = m_Tree.GetNextSiblingItem( hNextItem );
	}while( false == fCanStop );

	// Take the deeper level where ALL modules are selected.
	int iPos = (int)vecLevelState.size();
	if( iPos > 0 )
	{
		for( int iLoop = (int)vecLevelState.size() - 1; iLoop >= 0; iLoop-- )
		{
			if( 1 == vecLevelState[iLoop] )
				break;
			iPos--;
		}
	}

	m_SliderCtrl.SetPos( iPos );
	m_uiPrevSliderPos = iPos;
}

void CDlgTreeContainer::_SliderCtrlRightMove( void )
{
	std::vector<HTREEITEM> vecTreeList;
	HTREEITEM hRoot = m_Tree.GetRootItem( );
	if( NULL == hRoot )
		return;

	// Run all tree to detect what is the wider item in the current level.
	int iLevel = m_SliderCtrl.GetPos() - 1;
	int iCurrentDeep = 0;
	HTREEITEM hNextItem = hRoot;
	HTREEITEM hWiderItem = NULL;
	long lWiderItem = 0;
	bool fCanStop = false;
	do
	{
		if( NULL != hNextItem )
		{
			if( iCurrentDeep == iLevel )
			{
				TAGetItemRectParam rParam;
				rParam.uItem = (unsigned)hNextItem;
				rParam.fTextOnly = true;
				m_Tree.TAGetItemRect( &rParam );

				if( rParam.Rect.Width() > lWiderItem )
				{
					lWiderItem = rParam.Rect.Width();
					hWiderItem = hNextItem;
				}
			}
		}

		if( NULL == hNextItem )
		{
			if( vecTreeList.size() > 0 )
			{
				iCurrentDeep--;
				hNextItem = vecTreeList.back();
				hNextItem = m_Tree.GetNextSiblingItem( hNextItem );
				vecTreeList.pop_back();
			}
			else
				fCanStop = true;
		}
		else if( NULL != m_Tree.GetChildItem( hNextItem ) )
		{
			iCurrentDeep++;
			vecTreeList.push_back( hNextItem );
			hNextItem = m_Tree.GetChildItem( hNextItem );
		}
		else
			hNextItem = m_Tree.GetNextSiblingItem( hNextItem );
	}while( false == fCanStop );

	if( NULL != hWiderItem && lWiderItem > 0 )
	{
		TAEnsureVisibleParam rParam;
		rParam.uItem = (unsigned)hWiderItem;
		rParam.fMoveLeft = false;
		m_Tree.TAEnsureVisible( &rParam );
	}
}

void CDlgTreeContainer::_SliderCtrlLeftMove( void )
{
	std::vector<HTREEITEM> vecTreeList;
	HTREEITEM hRoot = m_Tree.GetRootItem( );
	if( NULL == hRoot )
		return;

	// Find the first item at the corresponding level.
	int iLevel = max( 0, m_SliderCtrl.GetPos() - 1 );
	int iCurrentDeep = 0;
	HTREEITEM hNextItem = hRoot;
	bool fCanStop = false;
	do
	{
		if( NULL != hNextItem )
		{
			LPCTSTR pText = m_Tree.GetItemText( hNextItem );
			if( iCurrentDeep == iLevel )
			{
				TAEnsureVisibleParam rParam;
				rParam.uItem = (unsigned)hNextItem;
				rParam.fMoveLeft = true;
				m_Tree.TAEnsureVisible( &rParam );
				fCanStop = true;
			}
		}

		if( NULL == hNextItem )
		{
			if( vecTreeList.size() > 0 )
			{
				iCurrentDeep--;
				hNextItem = vecTreeList.back();
				hNextItem = m_Tree.GetNextSiblingItem( hNextItem );
				vecTreeList.pop_back();
			}
			else
				fCanStop = true;
		}
		else if( NULL != m_Tree.GetChildItem( hNextItem ) )
		{
			iCurrentDeep++;
			vecTreeList.push_back( hNextItem );
			hNextItem = m_Tree.GetChildItem( hNextItem );
		}
		else
			hNextItem = m_Tree.GetNextSiblingItem( hNextItem );
	}while( false == fCanStop );
}

void CDlgTreeContainer::ResetTree( void )
{
	_FillTree( );
}


BEGIN_MESSAGE_MAP( CDlgDiversityFactor::CMyRichEditCtrl, CRichEditCtrl )
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()
void CDlgDiversityFactor::CMyRichEditCtrl::OnLButtonDown( UINT nFlags, CPoint point )
{
	// INTENTIONALLY DO NOTHING (TO AVOID USER SELECTION).
}

void CDlgDiversityFactor::CMyRichEditCtrl::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	// INTENTIONALLY DO NOTHING (TO AVOID USER SELECTION).
}

void CDlgDiversityFactor::CMyRichEditCtrl::OnRButtonDown( UINT nFlags, CPoint point )
{
	// INTENTIONALLY DO NOTHING (TO AVOID USER SELECTION).
}

void CDlgDiversityFactor::CMyRichEditCtrl::OnRButtonDblClk( UINT nFlags, CPoint point )
{
	// INTENTIONALLY DO NOTHING (TO AVOID USER SELECTION).
}

void CDlgDiversityFactor::CMyRichEditCtrl::OnMButtonDown( UINT nFlags, CPoint point )
{
	// INTENTIONALLY DO NOTHING (TO AVOID USER SELECTION).
}

void CDlgDiversityFactor::CMyRichEditCtrl::OnMButtonDblClk( UINT nFlags, CPoint point )
{
	// INTENTIONALLY DO NOTHING (TO AVOID USER SELECTION).
}

BOOL CDlgDiversityFactor::CMyRichEditCtrl::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
	return TRUE;
}

void CDlgDiversityFactor::CMyStatic::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	
	CBrush newBrush;
	if( IDC_LEGENDNODF_SQUARE == lpDrawItemStruct->CtlID )
		newBrush.CreateSolidBrush( _TAH_ORANGE );
	else if( IDC_LEGENDDF_SQUARE == lpDrawItemStruct->CtlID )
		newBrush.CreateSolidBrush( _IMI_GRAY_MED );
	else
		return;

	HGDIOBJ hOldBrush = pDC->SelectObject( (HGDIOBJ)newBrush );
	CRect rectClient;
	GetClientRect( &rectClient );
	pDC->Rectangle( &rectClient );

	pDC->SelectObject( hOldBrush );
}

IMPLEMENT_DYNAMIC( CDlgDiversityFactor, CDialogEx )

CDlgDiversityFactor::CDlgDiversityFactor( CWnd* pParent )
	: CDialogEx( CDlgDiversityFactor::IDD, pParent )
{
	// To allow to use RichEdit control 2.
	AfxInitRichEdit2();
	m_dDF = -1.0;
	m_bIsChanges = false;
	m_pclTADatastruct = NULL;
	m_pCurrentModule = NULL;
}

CDlgDiversityFactor::~CDlgDiversityFactor( void )
{
	m_TreeContainer.DestroyWindow();
	if( NULL != m_pclTADatastruct )
		delete m_pclTADatastruct;
}

void CDlgDiversityFactor::CheckInternalState( void )
{
	// Retrieve the current project state.
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	bool fActive = ( true == pMainFrame->IsHMCalcChecked() && false == pPrjParam->IsFreezed() );

	// Enable/Disable all the tree.
	m_TreeContainer.EnableWindow( ( true == fActive ) ? TRUE : FALSE );
	DWORD dwCurrentStyle = m_TreeContainer.GetpTree()->GetStyle();
	m_TreeContainer.GetpTree()->SetStyle( ( true == fActive ) ? dwCurrentStyle & ~WS_DISABLED : dwCurrentStyle | WS_DISABLED );
	DWORD dwCurrentStyleEx = m_TreeContainer.GetpTree()->GetExtendedStyle();
	m_TreeContainer.GetpTree()->SetExtendedStyle( ( true == fActive ) ? dwCurrentStyleEx & ~TVS_EX_GRAYEDDISABLE : dwCurrentStyleEx | TVS_EX_GRAYEDDISABLE );
	
	// Retrieve the static text to paint.
	CString strStaticText = TASApp.LoadLocalizedString( ( true == fActive ) ? IDS_DLGDIVFACTOR_STATICTEXT : IDS_DLGDIVFACTOR_STATICTEXTNODF );
	m_cStaticText.SetWindowText( strStaticText );

	// Apply justify alignment.
	PARAFORMAT rFormat;
	rFormat.cbSize = sizeof( PARAFORMAT );
	rFormat.dwMask = PFM_ALIGNMENT;
	rFormat.wAlignment = PFA_JUSTIFY;
	m_cStaticText.SetSel( 0, -1 );
	m_cStaticText.SetParaFormat( rFormat );

	// Adjust position of all controls below static text.
	CRect rectStaticText;
	m_cStaticText.GetClientRect( &rectStaticText );
	
	// Compute the true height with current text.
	CDC *pDC = GetDC();
	int iSavedDC = pDC->SaveDC();
	CFont* pOldFont = pDC->SelectObject( &afxGlobalData.fontRegular );
	pDC->DrawText( strStaticText, rectStaticText, DT_WORDBREAK | DT_LEFT | DT_EXPANDTABS | DT_CALCRECT );
	m_cStaticText.ClientToScreen( &rectStaticText );
	ScreenToClient( &rectStaticText );
	
	// Apply new height.
	m_cStaticText.SetWindowPos( NULL, -1, -1, rectStaticText.Width(), rectStaticText.Height(), SWP_NOMOVE | SWP_NOZORDER );

	// Compute the new height difference.
	CRect rectControl;
	GetDlgItem( IDC_STATICEDITDF )->GetWindowRect( &rectControl );
	ScreenToClient( &rectControl );
	long lDiffHeight = rectControl.top - rectStaticText.bottom - 7;
	
	// Apply on static text above the input field.
	GetDlgItem( IDC_STATICEDITDF )->SetWindowPos( NULL, rectControl.left, rectControl.top - lDiffHeight, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	
	// Apply on the input field.
	m_cEditDF.GetWindowRect( &rectControl );
	ScreenToClient( &rectControl );
	m_cEditDF.SetWindowPos( NULL, rectControl.left, rectControl.top - lDiffHeight, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	
	GetDlgItem( IDC_BUTTONRST )->GetWindowRect( &rectControl );
	ScreenToClient( &rectControl );
	GetDlgItem( IDC_BUTTONRST )->SetWindowPos( NULL, rectControl.left, rectControl.top - lDiffHeight, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	
	// Enable/Disable input.
	m_cEditDF.EnableWindow( ( true == fActive ) ? TRUE : FALSE );

	// Enable/Disable the "Apply" button.
	m_clApplyButton.EnableWindow( ( true == fActive ) ? TRUE : FALSE );

	// By default "Cancel" button is disabled. It will be enabled as soon as user will has clicked on "Apply".
	// If user has already clicked on "Apply" in another module available, we must let the button cancel enabled.
	GetDlgItem( IDCANCEL )->EnableWindow( ( true == m_bIsChanges) ? TRUE : FALSE );
	
	// Force a repaint.
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDlgDiversityFactor, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED( IDCANCEL, &CDlgDiversityFactor::OnBnClickedCancel )
	ON_BN_CLICKED( IDCLOSE, &CDlgDiversityFactor::OnBnClickedClose )
	ON_BN_CLICKED( IDAPPLY, &CDlgDiversityFactor::OnBnClickedApply )
	ON_EN_CHANGE( IDC_EDITDF, &CDlgDiversityFactor::OnEnChangeEditDF )
	ON_CBN_SELENDOK( IDC_COMBOCHOOSEROOT, OnCbnSelChangeModule )
	ON_BN_CLICKED(IDC_BUTTONRST, &CDlgDiversityFactor::OnBnClickedButtonrst)
END_MESSAGE_MAP()

void CDlgDiversityFactor::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITDF, m_cEditDF );
	DDX_Control( pDX, IDC_STATICTEXT, m_cStaticText );
	DDX_Control( pDX, IDAPPLY, m_clApplyButton );
	DDX_Control( pDX, IDC_COMBOCHOOSEROOT, m_clComboChooseRoot );
	DDX_Control( pDX, IDC_LEGENDNODF_SQUARE, m_LegendNoDFSquare );
	DDX_Control( pDX, IDC_LEGENDDF_SQUARE, m_LegendDFSquare );
}

BOOL CDlgDiversityFactor::OnInitDialog()
{
	// Do the default initialization.
	CDialogEx::OnInitDialog();

	// Take a snapshot of the CTableHM
	_TakeSnapshot();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGDIVFACTOR_CAPTION );
	SetWindowText( str );

	// Text for 'IDC_STATICEDITDF' will be set in the 'CDlgDiversityFactor::CMyStatic::DrawItem' method above.
	
	// To allow to user justify alignment.
	::SendMessage( m_cStaticText.m_hWnd, EM_SETTYPOGRAPHYOPTIONS, TO_ADVANCEDTYPOGRAPHY, TO_ADVANCEDTYPOGRAPHY );

	// For RichEdit control we need to disable this option to allow text be word wrapped.
	m_cStaticText.ModifyStyle( WS_HSCROLL, 0 );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDIVFACTOR_LEGENDNODF );
	GetDlgItem( IDC_LEGENDNODF_TEXT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDIVFACTOR_LEGENDDF );
	GetDlgItem( IDC_LEGENDDF_TEXT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDIVFACTOR_APPLY );
	m_clApplyButton.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDIVFACTOR_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CLOSE );
	GetDlgItem( IDCLOSE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDIVFACTOR_RESET );
	GetDlgItem( IDC_BUTTONRST )->SetWindowText( str );

	_FillComboChooseRoot();

	// Change style of 'm_cStaticText'.
	m_cStaticText.ModifyStyle( 0, SS_OWNERDRAW );

	// Retrieve 'Rectangle' client area.
	CRect rect;
	GetDlgItem( IDC_FRAMETREE )->GetWindowRect( &rect );
	ScreenToClient( &rect );

	// Create indirect dialog tree container.
	LONG lBaseUnits = ::GetDialogBaseUnits();
	
	WORD* pTemplate = &m_arwTemplate[0];
	DLGTEMPLATE& rDlgTemplate = *((DLGTEMPLATE*)pTemplate);
	rDlgTemplate.style = DS_SETFONT | DS_FIXEDSYS | WS_CHILD | WS_VISIBLE | WS_BORDER;
	rDlgTemplate.dwExtendedStyle = 0;
	rDlgTemplate.x = 0;
	rDlgTemplate.y = 0;
	rDlgTemplate.cx = MulDiv( rect.Width(), 4, LOWORD( lBaseUnits ) );
	rDlgTemplate.cy = MulDiv( rect.Height(), 8, HIWORD( lBaseUnits ) );
	rDlgTemplate.cdit = 0;
	pTemplate += ( sizeof( DLGTEMPLATE ) >> 1 );
	*pTemplate++ = 0x0000; // menu (none)
	*pTemplate++ = 0x0000; // dialog class (use standard)
	*pTemplate++ = 0x0000; // no caption

	// When calling 'CreateIndirect', 'OnInitDialog' in 'm_TreeContainer' is called asynchronously. We are not sure that internal
	// variables are well set when calling 'OnCbnSelChangeModule' below. This is why we create an event and wait for 'OnInitDialog'
	// set it to specify that it is ready.
	CEvent *pclEvent = new CEvent( FALSE, TRUE );
	pclEvent->ResetEvent();
	m_TreeContainer.GiveEvent( pclEvent );
	
	// Create.
	BOOL fReturn = m_TreeContainer.CreateIndirect( &rDlgTemplate, this );
	m_TreeContainer.SetWindowPos( NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

	// Wait.
	WaitForSingleObject( (HANDLE)pclEvent, INFINITE );
	m_TreeContainer.GiveEvent( NULL );
	delete pclEvent;

	// Fill tree.
	OnCbnSelChangeModule();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgDiversityFactor::OnSysCommand( UINT nID, LPARAM lParam )
{
	CDialogEx::OnSysCommand( nID, lParam );
	if( SC_CLOSE == ( nID & 0xFFF0 ) )
		OnBnClickedClose();
}

void CDlgDiversityFactor::OnShowWindow( BOOL bShow, UINT nStatus )
{
	CheckInternalState();
}

void CDlgDiversityFactor::OnBnClickedCancel()
{
	if( true == m_bIsChanges )
	{
		BeginWaitCursor();

		_RestoreSnapshot();

		TASApp.GetpTADS()->ComputeAllInstallation();

		// To force a redraw with restores state.
		
		if( NULL != pRViewSelProd )
		{
			pRViewSelProd->RedrawRightView();
			pRViewSelProd->SetRedraw( TRUE );
		}

		EndWaitCursor();
	}
	::PostMessage( pMainFrame->GetSafeHwnd(), WM_USER_DIVERSITYFACTORTOCLOSE, 0, 0 );
}

void CDlgDiversityFactor::OnBnClickedClose()
{
	::PostMessage( pMainFrame->GetSafeHwnd(), WM_USER_DIVERSITYFACTORTOCLOSE, 0, 0 );
}

void CDlgDiversityFactor::OnBnClickedApply()
{
	// Check first if there is a least one checkbox checked in the tree.
	CHMTreeListCtrl* pTree = m_TreeContainer.GetpTree();
	if( NULL == pTree )
		return;

	if( false == pTree->IsAtLeastOneChecked() )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_DFNOMODULESELECTED );
		m_cEditDF.SetFocus();
	}
	else if( -1.0 == m_dDF )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_DFINPUTINVALID );
		m_cEditDF.SetFocus();
	}
	else
	{
		// All is OK, we can apply diversity factor.
		if( true == _ApplyDiversityFactor() )
		{
			GetDlgItem( IDCANCEL )->EnableWindow( TRUE );
			m_bIsChanges = true;
		}
	}
}

void CDlgDiversityFactor::OnEnChangeEditDF()
{
	m_dDF = -1.0;
	if( GetFocus() == &m_cEditDF )
	{
		double dDF;
		ReadDoubleReturn_enum eReturn = ReadDouble( m_cEditDF, &dDF );
		switch( eReturn )
		{
			case ReadDoubleReturn_enum::RD_OK:
				if( dDF < 0.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
					m_cEditDF.SetFocus();
				}
				else if( dDF > 1.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_DFOUTOFRANGE );
					m_cEditDF.SetFocus();
				}
				else
					m_dDF = dDF;
				break;

			case ReadDoubleReturn_enum::RD_NOT_NUMBER:
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
				m_cEditDF.SetFocus();
				break;

			case ReadDoubleReturn_enum::RD_EMPTY:
			default:
				break;
		}
	}
	(m_dDF > 0) ? m_clApplyButton.EnableWindow( TRUE ) : m_clApplyButton.EnableWindow( FALSE );
}

void CDlgDiversityFactor::OnCbnSelChangeModule() 
{
	m_pCurrentModule = dynamic_cast<CDS_HydroMod*>( (CData*)( m_clComboChooseRoot.GetCBCurSelIDPtr().MP ) );
	
	if( NULL != m_pCurrentModule )
	{
		m_TreeContainer.SetHydroMod( m_pCurrentModule );
	}

	CString str( _T("") );
	
	if( m_TreeContainer.GetDiversityFactor() != -1.0 )
	{
		str.Format( _T("%.2f"), m_TreeContainer.GetDiversityFactor() );
		m_dDF = m_TreeContainer.GetDiversityFactor();
	}
	
	m_cEditDF.SetWindowText( str );

	CheckInternalState();
}

void CDlgDiversityFactor::OnBnClickedButtonrst()
{
	m_cEditDF.SetWindowText( _T("") );
	_CleanAllDiversityFactor( );
	CHMTreeListCtrl* pTree = m_TreeContainer.GetpTree();
	HTREEITEM hRoot = pTree->GetRootItem();
	if( NULL == hRoot )
		return;
	BeginWaitCursor();
	CDS_HydroMod* pRoot = (CDS_HydroMod *)pTree->GetItemData( hRoot );
	pRoot->ComputeAll();
	EndWaitCursor();

	m_TreeContainer.ResetTree( );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgDiversityFactor::_TakeSnapshot( void )
{
	CTableHM *pTableHM = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
	ASSERT( NULL != pTableHM );

	if( NULL == pTableHM || 0 == pTableHM->GetItemCount() )
	{
		return;
	}

	m_mapDFSnapshot.clear();

	std::vector<CDS_HydroMod*> vecTreeList;
	CTable *pTableParent = (CTable*)pTableHM;
	CDS_HydroMod *pHMNext = dynamic_cast<CDS_HydroMod*>( (CData*)( pTableParent->GetFirst().MP ) );
	bool fCanStop = false;
	
	do 
	{
		if( NULL != pHMNext )
		{
			m_mapDFSnapshot[pHMNext] = pHMNext->GetDiversityFactor();
		}

		if( NULL == pHMNext )
		{
			if( vecTreeList.size() > 0 )
			{
				pTableParent = vecTreeList.back();
				pHMNext = NULL;

				if( NULL != pTableParent ) 
				{
					pHMNext = dynamic_cast<CDS_HydroMod*>( (CData*)( pTableParent->GetNext().MP ) );
				}

				vecTreeList.pop_back();
			}
			else
			{
				fCanStop = true;
			}
		}
		else if( NULL != pHMNext->GetFirst().MP )
		{
			vecTreeList.push_back( pHMNext );
			pTableParent = (CTable*)pHMNext;
			pHMNext = dynamic_cast<CDS_HydroMod*>( (CData*)( pTableParent->GetFirst().MP ) );
		}
		else
		{
			pHMNext = dynamic_cast<CDS_HydroMod*>( (CData*)( pTableParent->GetNext().MP ) );
		}

	}while( false == fCanStop );
}

void CDlgDiversityFactor::_RestoreSnapshot( void )
{
	if( 0 == m_mapDFSnapshot.size() )
		return;

	for( mapHMDoubleIter mapIter = m_mapDFSnapshot.begin(); mapIter != m_mapDFSnapshot.end(); mapIter++ )
		mapIter->first->SetDiversityFactor( mapIter->second );
}

bool CDlgDiversityFactor::_ApplyDiversityFactor( void )
{
	// Hard cleaning of all diversity, should be removed when we accept several diversity coefficients
	_CleanAllDiversityFactor( );

	// We must check the tree and set hydromod that are ready for diversity factor.
	// Typically, we set a flag only for the last selected child by branch.

	CHMTreeListCtrl* pTree = m_TreeContainer.GetpTree();
	if( NULL == pTree )
		return false;

	HTREEITEM hRoot = pTree->GetRootItem();
	if( NULL == hRoot )
		return false;

	std::vector<HTREEITEM> vecTreeList;
	std::vector<CDS_HydroMod*> vecHydroModDF;
	int iCurrentDeep = 0;
	HTREEITEM hNextItem = hRoot;
	HTREEITEM hModuleToSet = NULL;
	bool fCanStop = false;
	bool fGoToChildren;
	do 
	{
		fGoToChildren = true;
		if( NULL != hNextItem )
		{
			CDS_HydroMod *pHM = (CDS_HydroMod *)pTree->GetItemData( hNextItem );

			// Check current checkbox state.
			if( 1 == pTree->GetItemCheckBox( hNextItem ) )
				hModuleToSet = hNextItem;
			else
				fGoToChildren = false;
		}

		if( NULL == hNextItem )
		{
			if( vecTreeList.size() > 0 )
			{
				// If we must set the diversity factor flag...
				if( NULL != hModuleToSet )
				{
					CDS_HydroMod *pHM = (CDS_HydroMod *)pTree->GetItemData( hModuleToSet );
					if( NULL != pHM )
					{
						pHM->SetDiversityFactor( m_dDF );
						vecHydroModDF.push_back( pHM );
						hModuleToSet = NULL;
					}
				}

				iCurrentDeep--;
				hNextItem = vecTreeList.back();
				hNextItem = pTree->GetNextSiblingItem( hNextItem );
				vecTreeList.pop_back();
			}
			else
				fCanStop = true;
		}
		else if( true == fGoToChildren && NULL != pTree->GetChildItem( hNextItem ) )
		{
			iCurrentDeep++;
			vecTreeList.push_back( hNextItem );
			hNextItem = pTree->GetChildItem( hNextItem );
		}
		else
		{
			// If a module has no children and it is checked...
			if( NULL != hModuleToSet && hModuleToSet == hNextItem )
			{
				CDS_HydroMod *pHM = (CDS_HydroMod *)pTree->GetItemData( hModuleToSet );
				if( NULL != pHM )
				{
					pHM->SetDiversityFactor( m_dDF );
					vecHydroModDF.push_back( pHM );
					hModuleToSet = NULL;
				}
			}
			hNextItem = pTree->GetNextSiblingItem( hNextItem );
		}
	}while( false == fCanStop );

	// Now we can set the diversity flag active

	BeginWaitCursor();
	CDS_HydroMod* pRoot = (CDS_HydroMod *)pTree->GetItemData( hRoot );
	pRoot->ComputeAll();
	EndWaitCursor();

	// Send message to notify the display of the result tab.
	if( NULL != pDlgLTtabctrl )
	{
		int iActiveTab = pDlgLTtabctrl->GetActiveTab();

		if( iActiveTab != CMyMFCTabCtrl::TabIndex::etiSelP )
		{
			// If we are not in the selected product tab, when changing tab 'SSheetSelProd' will be completely redrawn.
			pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSelP );
		}
	}

	// If we are already in the good tab, there are no call to 'SSheetSelProd::SetRedraw' method. We have to
	// do it implicitly.
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->RedrawRightView();
	}
	
	// Force now to go on the good SSheetSelProd tab.
	bool bReturn = false;

	if( NULL != pRViewSelProd )
	{
		bReturn = pRViewSelProd->GoToPage( CDB_PageSetup::DIVERSITYFACTOR );
	}

	return bReturn;
}

void CDlgDiversityFactor::_FillComboChooseRoot( void )
{
	if( m_clComboChooseRoot.GetCount() > 0 )
	{
		m_clComboChooseRoot.ResetContent();
	}

	CTable *pTab = dynamic_cast<CTable *>( TASApp.GetpTADS()->GetpHydroModTable() );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return;
	}

	std::map< int, CDS_HydroMod* > mapModuleName;
	
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod*>( (CData*)( IDPtr.MP ) );

		if( NULL != pHM && _T('\0') != *pHM->GetHMName() )
		{
			mapModuleName[pHM->GetPos()] = pHM;
		}
	}

	if( 0 == mapModuleName.size() )
	{
		return;
	}

	CRankEx RootList;
	
	for( std::map< int, CDS_HydroMod* >::iterator iter = mapModuleName.begin(); iter != mapModuleName.end(); iter++ )
	{
		std::wstring wtemp = iter->second->GetHMName();
		RootList.Add( wtemp, iter->first, (LPARAM)(void *)iter->second );
	}

	m_clComboChooseRoot.FillInCombo( &RootList, mapModuleName.begin()->second->GetHMName() );
}

void CDlgDiversityFactor::_CleanAllDiversityFactor( CDS_HydroMod *pHM )
{
	if ( NULL == pHM )
	{
		CHMTreeListCtrl* pTree = m_TreeContainer.GetpTree();
		if( NULL == pTree )
			return ;

		HTREEITEM hRoot = pTree->GetRootItem();
		if( NULL == hRoot )
			return ;

		HTREEITEM hNextItem = hRoot;
		if( NULL == hRoot )
			return ;

		pHM = (CDS_HydroMod *)pTree->GetItemData( hNextItem );

		if( NULL == pHM )
			return;
	}
	pHM->CleanAllDiversityInfos( pHM );
}
