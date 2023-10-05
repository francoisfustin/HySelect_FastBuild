#include "stdafx.h"


#include "TASelect.h"
#include "ExtListCtrl.h"

IMPLEMENT_DYNAMIC( CExtListCtrl, CListCtrl )

CExtListCtrl::CExtListCtrl()
{
	m_bClickDisabled = false;
}

CExtListCtrl::~CExtListCtrl()
{
	m_clFontBold.DeleteObject();
	m_mapFontDetailsList.clear();
	m_mapEnableItemStateList.clear();
}

void CExtListCtrl::SetItemTextColor( int iRow, int iColumn, COLORREF clUnselectedColor, COLORREF clSelectedColor )
{
	if( 0 == m_mapFontDetailsList.count( iRow ) || 0 == m_mapFontDetailsList[iRow].count( iColumn ) )
	{
		m_mapFontDetailsList[iRow][iColumn] = _FontDetails( clUnselectedColor, clSelectedColor );
	}
	else
	{
		m_mapFontDetailsList[iRow][iColumn].m_clUnselectedColor = clUnselectedColor;
		m_mapFontDetailsList[iRow][iColumn].m_clSelectedColor = clSelectedColor;
	}
}

void CExtListCtrl::SetItemTextBkColor( int iRow, int iColumn, COLORREF clBackgroundColor )
{
	if( 0 == m_mapFontDetailsList.count( iRow ) || 0 == m_mapFontDetailsList[iRow].count( iColumn ) )
	{
		m_mapFontDetailsList[iRow][iColumn] = _FontDetails( clBackgroundColor );
	}
	else
	{
		m_mapFontDetailsList[iRow][iColumn].m_clBackgroundColor = clBackgroundColor;
	}
}

void CExtListCtrl::SetItemTextBold( int iRow, int iColumn, bool bBoldItem )
{
	if( 0 == m_mapFontDetailsList.count( iRow ) || 0 == m_mapFontDetailsList[iRow].count( iColumn ) )
	{
		m_mapFontDetailsList[iRow][iColumn] = _FontDetails( bBoldItem );
	}
	else
	{
		m_mapFontDetailsList[iRow][iColumn].m_bBold = bBoldItem;
	}
}

void CExtListCtrl::SetItemEnableState( int iItem, bool bEnabled )
{
	if( m_mapEnableItemStateList.end() != m_mapEnableItemStateList.find( iItem ) )
	{
		m_mapEnableItemStateList.at( iItem ) = bEnabled;
	}
	else
	{
		m_mapEnableItemStateList.insert( std::pair<int, bool>( iItem, bEnabled ) );
	}
}

void CExtListCtrl::SetColumnAutoWidth( int iColumn )
{
	LVCOLUMN rColumn;

	// 2014-03-07 : Bug correction: LVCOLUMN structure MUST BE initialized before the call otherwise HySelect crashes in some system.
	ZeroMemory( &rColumn, sizeof( LVCOLUMN ) );
	rColumn.mask = LVCF_FMT;
	
	if( FALSE == GetColumn( iColumn, &rColumn ) )
	{
		return;
	}
	
	int iItems = GetItemCount();

	if( 0 == iItems )
	{
		return;
	}

	CFont *pclFontNormal = GetFont();
	LOGFONT rLogFont;
	
	pclFontNormal->GetLogFont( &rLogFont );
	rLogFont.lfWeight = FW_BOLD;
	
 	CFont clFontBold;
 	clFontBold.CreateFontIndirectW( &rLogFont );

	int iMaxWidth = 0;

	for( int i = 0; i < iItems; i++ )
	{
		CString strText = GetItemText( i, 0 );
		int iWidth;

		if( true == _CheckItemBold( i, iColumn ) )
		{
			SetFont( &clFontBold );
			iWidth = GetStringWidth( strText );
			SetFont( pclFontNormal );
		}
		else
		{
			iWidth = GetStringWidth( strText );
		}
		
		if( iWidth > iMaxWidth )
		{
			iMaxWidth = iWidth;
		}
	}
	
	clFontBold.DeleteObject();
	SetColumnWidth( iColumn, iMaxWidth + 4 );
}

void CExtListCtrl::ResetAllFonts()
{
	m_mapFontDetailsList.clear();
}

void CExtListCtrl::SetClickListCtrlDisable( bool bFlag )
{
	m_bClickDisabled = bFlag;
}

int CExtListCtrl::AddItemText( CString strText, int iColumn )
{
	int iRow = InsertItem( LVIF_TEXT | LVIF_STATE, GetItemCount(), _T(""), 0, LVIS_SELECTED, 0, 0 );
	SetItemText( iRow, iColumn, strText );
	return iRow;
}

int CExtListCtrl::AddItemTextVerify( CString strText, int iRow, int iColumn )
{
	int iReturn = iRow;

	if( iRow >= GetItemCount() )
	{
		iReturn = AddItemText( strText, iColumn );
	}
	else
	{
		SetItemText( iRow, iColumn, strText );
	}

	return iReturn;
}

int CExtListCtrl::AddItemTextColorVerify( CString strText, int iRow, int iColumn, COLORREF clUnselectedColor, COLORREF clSelectedColor, COLORREF clBackgroundColor )
{
	SetItemTextColor( iRow, iColumn, clUnselectedColor, clSelectedColor );
	SetItemTextBkColor( iRow, iColumn, clBackgroundColor );
	return AddItemTextVerify( strText, iRow, iColumn );
}

int CExtListCtrl::GetRowFromPoint( CPoint &point, int *piCol ) const
{
	int iColumn = 0;
	int iRow = HitTest( point, NULL );
	int iReturnedRow = -1;

	if( NULL != piCol )
	{
		*piCol = 0;
	}

	// Make sure that the ListView is in LVS_REPORT .
	if( ( GetWindowLong( m_hWnd, GWL_STYLE ) & LVS_TYPEMASK ) != LVS_REPORT )
	{
		return iRow;
	}

	// Get the top and bottom row visible   
	iRow = GetTopIndex();
	int iBottom = iRow + GetCountPerPage();

	if( iBottom > GetItemCount() )
	{
		iBottom = GetItemCount();
	}

	// Get the number of columns.
	CHeaderCtrl *pHeader = (CHeaderCtrl *)GetDlgItem( 0 );
	int nColumnCount = pHeader->GetItemCount();

	// Loop through the visible rows.
	for( ; iRow <= iBottom; iRow++ )
	{
		// Get bounding rectangle of item and check whether point falls in it.
		CRect rect;
		GetItemRect( iRow, &rect, LVIR_BOUNDS );

		if( TRUE == rect.PtInRect( point ) )
		{
			// Find the column.
			for( iColumn = 0; iColumn < nColumnCount; iColumn++ )
			{
				int iColWidth = GetColumnWidth( iColumn );

				if( point.x >= rect.left && point.x <= ( rect.left + iColWidth ) )
				{
					if( NULL != piCol )
					{
						*piCol = iColumn;
					}

					iReturnedRow = iRow;
					break;
				}

				rect.left += iColWidth;
			}
		}
	}

	return iReturnedRow;
}

CEdit *CExtListCtrl::EditSubLabel( int nItem, int nCol )
{
	// The returned pointer should not be saved, make sure item visible.
	if( FALSE == EnsureVisible( nItem, TRUE ) )
	{
		return NULL;
	}

	// Make sure that column number is valid.   
	CHeaderCtrl *pHeader = (CHeaderCtrl *)GetDlgItem( 0 );
	int nColumnCount = pHeader->GetItemCount();
	
	if( nCol >= nColumnCount || GetColumnWidth( nCol ) < 5 )
	{
		return NULL;
	}

	// Get the column offset.
	int iOffset = 0;

	for( int i = 0; i < nCol; i++ )
	{
		iOffset += GetColumnWidth( i );
	}

	CRect rect;
	GetItemRect( nItem, &rect, LVIR_BOUNDS );

	// Scroll horizontally if we need to expose the column.
	CRect rcClient;
	GetClientRect( &rcClient );

	if( iOffset + rect.left < 0 || iOffset + rect.left > rcClient.right )
	{
		CSize size;
		size.cx = iOffset + rect.left;
		size.cy = 0;
		Scroll( size );
		rect.left -= size.cx;
	}

	// Get column alignment.
	LV_COLUMN lvcol;
	lvcol.mask = LVCF_FMT;
	GetColumn( nCol, &lvcol );
	DWORD dwStyle;

	if( LVCFMT_LEFT == ( lvcol.fmt & LVCFMT_JUSTIFYMASK ) )
	{
		dwStyle = ES_LEFT;
	}
	else if( LVCFMT_RIGHT == ( lvcol.fmt & LVCFMT_JUSTIFYMASK ) )
	{
		dwStyle = ES_RIGHT;
	}
	else
	{
		dwStyle = ES_CENTER;
	}

	rect.left += iOffset + 4;
	rect.right = rect.left + GetColumnWidth( nCol ) - 3;

	if( rect.right > rcClient.right )
	{
		rect.right = rcClient.right;
	}

	dwStyle |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;

	CEdit *pEdit = new CPlaceEdit( nItem, nCol, GetItemText( nItem, nCol ) );
	pEdit->Create( dwStyle, rect, this, IDC_LIST1 );

	return pEdit;
}

void CExtListCtrl::DoDataExchange( CDataExchange *pDX )
{
	CListCtrl::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CExtListCtrl, CListCtrl )
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_EX( TTN_NEEDTEXTA, 0, OnToolNeedText )
	ON_NOTIFY_EX( TTN_NEEDTEXTW, 0, OnToolNeedText )
END_MESSAGE_MAP()

void CExtListCtrl::GetVertScrollPosition( int *piDeltaPos )
{
	if( NULL == piDeltaPos )
	{
		return;
	}
	
	*piDeltaPos = 0;
	*piDeltaPos = GetScrollPos( SB_VERT );
}

void CExtListCtrl::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
{
	if( GetFocus() != this )
	{
		SetFocus();
	}

	CListCtrl::OnVScroll( nSBCode, nPos, pScrollBar );
}

BOOL CExtListCtrl::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	if( GetFocus() != this )
	{
		SetFocus();
	}
	
	return CListCtrl::OnMouseWheel( nFlags, zDelta, pt );
}

void CExtListCtrl::OnLButtonDown( UINT nFlags, CPoint point )
{
	// Check if we click on a item
	for( int i = 0; i < GetItemCount(); i++ )
	{
		CRect rect;
		GetItemRect( i, &rect, LVIR_BOUNDS );

		if( TRUE == rect.PtInRect( point ) )
		{
			// Check first if we can find item in the map and verify if disabled.
			if( m_mapEnableItemStateList.end() != m_mapEnableItemStateList.find( i ) 
					&& false == m_mapEnableItemStateList.at( i ) )
			{
				// Do nothing
				return;
			}
		}
	}

	CListCtrl::OnLButtonDown( nFlags, point );
}

BOOL CExtListCtrl::OnToolNeedText( UINT id, NMHDR *pNMHDR, LRESULT *pResult )
{
	CPoint pt( GetMessagePos() );
	ScreenToClient( &pt );

	int nRow, nCol;
	CellHitTest( pt, nRow, nCol );

	TCHAR *ptcToolTip = GetToolTipText( nRow, nCol );

	if( NULL == ptcToolTip )
	{
		return FALSE;
	}

	// Non-unicode applications can receive requests for strToolTip-text in unicode
	TOOLTIPTEXTW *pTTTW = (TOOLTIPTEXTW *)pNMHDR;
	pTTTW->lpszText = ptcToolTip;

	CToolTipCtrl *pToolTip = AfxGetModuleThreadState()->m_pToolTip;

	if( NULL != pToolTip )
	{
		pToolTip->SetMaxTipWidth( SHRT_MAX );
	}

	return TRUE;
}

void CExtListCtrl::SendLButtonDown( UINT nFlags, CPoint point, bool bForEditable )
{
	int iIndex;
	CListCtrl::OnLButtonDown( nFlags, point );

	if( true == bForEditable )
	{
		ModifyStyle( 0, LVS_EDITLABELS );
		int iColNum;

		if( ( iIndex = GetRowFromPoint( point, &iColNum ) ) != -1 )
		{
			UINT uiFlag = LVIS_FOCUSED;

			if( ( GetItemState( iIndex, uiFlag ) & uiFlag ) == uiFlag && iColNum == 1 )
			{
				// Add check for LVS_EDITLABELS.
				if( GetWindowLong( m_hWnd, GWL_STYLE ) & LVS_EDITLABELS )
				{
					EditSubLabel( iIndex, iColNum );
				}
			}
			else
			{
				SetItemState( iIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
			}
		}
	}
	else
	{
		ModifyStyle( LVS_EDITLABELS, 0 );
	}
}

void CExtListCtrl::AddToolTipText( int iRow, int iCol, CString strText )
{
	if( 0 == m_mapToolTipText.count( iRow ) )
	{
		std::map<int, CString> mapTemp = { { iCol, strText } };
		m_mapToolTipText.insert( std::pair<int, std::map<int, CString>>( iRow, mapTemp ) );
	}
	else if( 0 == m_mapToolTipText.at( iRow ).count( iCol ) )
	{
		m_mapToolTipText.at( iRow ).insert( std::pair<int, CString>( iCol, strText ) );
	}
	else
	{
		m_mapToolTipText.at( iRow ).at( iCol ) = strText;
	}
}

void CExtListCtrl::RemoveAllToolTip()
{
	m_mapToolTipText.clear();
}

// Handle the end of the label edit
void CExtListCtrl::OnEndLabelEdit( NMHDR *pNMHDR, LRESULT *pResult )
{
	LV_DISPINFO *plvDispInfo = (LV_DISPINFO *)pNMHDR;

	LV_ITEM	*plvItem = &plvDispInfo->item;

	if( plvItem->pszText != NULL )
	{
		SetItemText( plvItem->iItem, plvItem->iSubItem, plvItem->pszText );
	}

	*pResult = FALSE;
}

void CExtListCtrl::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	// Disable the CToolTipCtrl of CListCtrl so it won't disturb the CWnd tooltip.
	GetToolTips()->Activate( FALSE );

	// Activates the standard CWnd tooltip functionality.
	VERIFY( EnableToolTips(TRUE) );
}

 INT_PTR CExtListCtrl::OnToolHitTest( CPoint point, TOOLINFO *pTI ) const
 {
	CPoint pt( GetMessagePos() );
	ScreenToClient( &pt );

	if( false == ShowToolTip( pt ) )
	{
		return -1;
	}

	int nRow, nCol;
	CellHitTest( pt, nRow, nCol );

	//Get the client (area occupied by this control).
	RECT rcClient;
	GetClientRect( &rcClient );

	// Fill in the TOOLINFO structure.
	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT)( nRow * 1000 + nCol );
	pTI->lpszText = LPSTR_TEXTCALLBACK;	// Send TTN_NEEDTEXT when tooltip should be shown
	pTI->rect = rcClient;

	return pTI->uId; // Must return a unique value for each cell (Marks a new tooltip)
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Watch out !! To use this you have to Enable your "owner draw fixed" in the property ressources//
///////////////////////////////////////////////////////////////////////////////////////////////////
void CExtListCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if( NULL == lpDrawItemStruct )
	{
		return; 
	}
	
	CDC *pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	int nSavedDC = pDC->SaveDC();
    
	RECT r;
	CRect rItem( lpDrawItemStruct->rcItem );
	CBrush *pBrush;
   
	r.left = lpDrawItemStruct->rcItem.left;
	r.right = lpDrawItemStruct->rcItem.right;
	r.top = lpDrawItemStruct->rcItem.top;
	r.bottom = lpDrawItemStruct->rcItem.bottom;
	CRect rect(r);
    
	int iItem = lpDrawItemStruct->itemID;

	// If the click function is disabled...
	if( true == m_bClickDisabled )
	{
		// Unselect selected item.
		if( ( lpDrawItemStruct->itemState & ODS_SELECTED ) || ( lpDrawItemStruct->itemState & ODS_FOCUS ) )
		{
			lpDrawItemStruct->itemState &= !ODS_SELECTED;
		}
	}
	else
	{
		// Change the background color to the desired one.
		BOOL bSelected = ( ( lpDrawItemStruct->itemState & ODS_FOCUS ) || ( lpDrawItemStruct->itemState & ODS_SELECTED ) );

		if( TRUE == bSelected )
		{
			COLORREF color = _CheckItemTextBkColor( iItem, 0 );
			CBrush brush( color );
			pBrush = pDC->SelectObject( &brush );
			pDC->FillRect( &rect, &brush );
			pDC->SetTextColor( RGB( 0, 255, 0 ) );
			pDC->SetBkColor( color );
			pDC->SelectObject( pBrush );
		}
	}
	
	CString s;
	int iNoColumn;
	int iWidthColumn;
	char *p = 0;
	int nNbColumns = GetHeaderCtrl()->GetItemCount();
	int ncx = 0;
	LV_COLUMN lvc;
	TCHAR szItem[255];
	LVITEM  LvItem;
	CImageList *pImgList = GetImageList( LVSIL_NORMAL );

	if( NULL == pImgList )
	{
		pImgList = GetImageList( LVSIL_SMALL );
	}
    
	bool bImageExist = ( NULL != pImgList && pImgList->m_hImageList != NULL );
	IMAGEINFO info;
	CRect SizeImg( 0, 0, 0, 0 );

	if( true == bImageExist )
	{
		pImgList->GetImageInfo( 0, &info );
		SizeImg = CRect( info.rcImage );
	}
	
	for( iNoColumn = 0; iNoColumn < nNbColumns ; iNoColumn++ )
	{        
		iWidthColumn = GetColumnWidth( iNoColumn );
		lvc.mask = LVCF_FMT;

		if( 0 == GetColumn( iNoColumn, &lvc ) )
		{
			continue;
		}
        
		LvItem.mask = LVIF_TEXT | LVIF_IMAGE;
		LvItem.cchTextMax = sizeof( szItem );
		LvItem.pszText = szItem;
		LvItem.iItem = iItem;
		LvItem.iSubItem = iNoColumn;
		LvItem.iImage = 0;
        
		GetItem( &LvItem );
        
		s = LvItem.pszText;
		CRect rZone = r;

		if( true == bImageExist && LvItem.iImage != -1 ) 
		{
			rZone.left += SizeImg.Width();
			pImgList->Draw( pDC, LvItem.iImage, CPoint( r.left, r.top ), ILD_NORMAL );
		}
		
		COLORREF color = _CheckItemColor( LvItem.iItem, iNoColumn, lpDrawItemStruct->itemState & ODS_SELECTED );
		pDC->SetTextColor( color );
		bool bTestBold = _CheckItemBold( LvItem.iItem, iNoColumn );
		
		// Create the font.
		LOGFONT lf;
		memset( &lf, 0, sizeof( LOGFONT ) );
		this->GetFont()->GetLogFont( &lf );

		if( true == bTestBold )
		{
			lf.lfWeight = FW_BOLD;
		}
		
		m_clFontBold.CreateFontIndirect( &lf );
		
		// Keep the Font.
		CFont *pOldFont = pDC->SelectObject( &m_clFontBold );

		if( lvc.fmt & LVCFMT_RIGHT )
		{
			pDC->SetTextAlign( TA_RIGHT );
			s += ' ';
			pDC->ExtTextOut( r.left + iWidthColumn , r.top, ETO_OPAQUE | ETO_CLIPPED, &rZone, s, NULL );
		}
		else if( lvc.fmt & LVCFMT_CENTER )
		{
			// HYS-987
			pDC->SetTextAlign( TA_CENTER );
			s += ' ';
			pDC->ExtTextOut( r.left + (iWidthColumn/2) , r.top, ETO_OPAQUE | ETO_CLIPPED, &rZone, s, NULL );
		}
		else
		{
			pDC->SetTextAlign( TA_LEFT );
			s = ' ' + s;
			pDC->ExtTextOut( rZone.left ,r.top, ETO_OPAQUE | ETO_CLIPPED, &rZone, s, NULL );
		}
		
		r.left += iWidthColumn;

		// Release the previous font after deleted the object.
		m_clFontBold.DeleteObject();
		pDC->SelectObject( pOldFont );
	}
	
	pDC->RestoreDC( nSavedDC );

	if( lpDrawItemStruct->itemState & ODS_SELECTED )
	{
		pDC->DrawFocusRect( &lpDrawItemStruct->rcItem );
	}
}

void CExtListCtrl::CellHitTest(const CPoint &pt, int &nRow, int &nCol) const
{
	nRow = -1;
	nCol = -1;

	LVHITTESTINFO lvhti = {0};
	lvhti.pt = pt;
	nRow = ListView_SubItemHitTest( m_hWnd, &lvhti );	// SubItemHitTest is non-const
	nCol = lvhti.iSubItem;
	
	if( 0 == ( lvhti.flags & LVHT_ONITEMLABEL ) )
	{
		nRow = -1;
	}
}

bool CExtListCtrl::ShowToolTip( const CPoint &pt ) const
{
	// Lookup up the cell.
	int nRow, nCol;
	CellHitTest( pt, nRow, nCol );

	if( nRow != -1 && nCol != -1 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

TCHAR *CExtListCtrl::GetToolTipText( int nRow, int nCol )
{
	TCHAR *ptcText = NULL;

	if( nRow != -1 && nCol != -1 )
	{
		if( 0 != m_mapToolTipText.count( nRow ) )
		{
			if( 0 != m_mapToolTipText.at( nRow ).count( nCol ) )
			{
				ptcText = (LPTSTR)(LPCTSTR)( m_mapToolTipText.at( nRow ).at( nCol ).GetBuffer() );
			}
		}
	}

	return ptcText;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

COLORREF CExtListCtrl::_CheckItemColor( int iRow, int iColumn, bool bSelected )
{
	COLORREF clColor;

	if( 0 == m_mapFontDetailsList.count( iRow ) || 0 == m_mapFontDetailsList[iRow].count( iColumn ) )
	{
		clColor = ( true == bSelected ) ? RGB( 255, 255, 255 ) : RGB( 0, 0, 0 );
	}
	else
	{
		clColor = ( true == bSelected ) ? m_mapFontDetailsList[iRow][iColumn].m_clSelectedColor : m_mapFontDetailsList[iRow][iColumn].m_clUnselectedColor;
	}

	return clColor;
}

bool CExtListCtrl::_CheckItemBold( int iRow, int iColumn )
{
	bool bBold;

	if( 0 == m_mapFontDetailsList.count( iRow ) || 0 == m_mapFontDetailsList[iRow].count( iColumn ) )
	{
		bBold = false;
	}
	else
	{
		bBold = m_mapFontDetailsList[iRow][iColumn].m_bBold;
	}

	return bBold;
}

COLORREF CExtListCtrl::_CheckItemTextBkColor( int iRow, int iColumn )
{
	COLORREF clColor;

	if( 0 == m_mapFontDetailsList.count( iRow ) || 0 == m_mapFontDetailsList[iRow].count( iColumn ) )
	{
		clColor = RGB( 0, 0, 255 );
	}
	else
	{
		clColor = m_mapFontDetailsList[iRow][iColumn].m_clBackgroundColor;
	}

	return clColor;
}

CPlaceEdit::CPlaceEdit( int iItem, int iSubItem, CString sInitText )
	: m_sInitText( sInitText )
{
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_bESC = FALSE;
}

CPlaceEdit::~CPlaceEdit() {}

BEGIN_MESSAGE_MAP( CPlaceEdit, CEdit )
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions.
BOOL CPlaceEdit::PreTranslateMessage( MSG *pMsg )
{
	if( WM_KEYDOWN == pMsg->message )
	{
		if( VK_RETURN == pMsg->wParam || VK_DELETE == pMsg->wParam || VK_ESCAPE == pMsg->wParam || GetKeyState( VK_CONTROL ) )
		{
			::TranslateMessage( pMsg );
			::DispatchMessage( pMsg );
			return TRUE;
			// DO NOT process further       
		}
	}

	return CEdit::PreTranslateMessage( pMsg );
}

// Called immediately before losing the input focus
void CPlaceEdit::OnKillFocus( CWnd *pNewWnd )
{
	CEdit::OnKillFocus( pNewWnd );
	CString str;
	GetWindowText( str );

	DestroyWindow();
}

// Called when nonclient area is being destroyed
void CPlaceEdit::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;
}

// Called for nonsystem character keystrokes
void CPlaceEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if( VK_ESCAPE == nChar || VK_RETURN == nChar )
	{
		if( VK_ESCAPE == nChar )
		{
			m_bESC = TRUE;
		}

		GetParent()->SetFocus();
		return;
	}

	CEdit::OnChar( nChar, nRepCnt, nFlags );

	// Resize edit control if needed.
	CString str;
	GetWindowText( str );
	CWindowDC dc( this );
	CFont *pFont = GetParent()->GetFont();
	CFont *pFontDC = dc.SelectObject( pFont );
	CSize size = dc.GetTextExtent( str );
	dc.SelectObject( pFontDC );
	size.cx += 5;

	// Get the client rectangle.
	CRect rect, parentrect;
	GetClientRect( &rect );
	GetParent()->GetClientRect( &parentrect );

	// Transform rectangle to parent coordinates.
	ClientToScreen( &rect );
	GetParent()->ScreenToClient( &rect );

	// Check whether control needs resizing and if sufficient space to grow.
	if( size.cx > rect.Width() )
	{
		if( size.cx + rect.left < parentrect.right )
		{
			rect.right = rect.left + size.cx;
		}
		else
		{
			rect.right = parentrect.right;
		}

		MoveWindow( &rect );
	}

	// Construct list control item data.
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;
	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	dispinfo.item.pszText = m_bESC ? NULL : LPTSTR( (LPCTSTR)str );
	dispinfo.item.cchTextMax = str.GetLength();

	// Send this Notification to parent of ListView ctrl.
	CWnd *pWndViewAttachmentsDlg = GetParent()->GetParent();

	if( NULL != pWndViewAttachmentsDlg )
	{
		pWndViewAttachmentsDlg->SendMessage( WM_APP + 1, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo );
	}
}

// Called when application requests the Windows window be created by calling the Create/CreateEx member function.
int CPlaceEdit::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( -1 == CEdit::OnCreate( lpCreateStruct ) )
	{
		return -1;
	}

	// Set the proper font.
	CFont *pclFont = GetParent()->GetFont();
	SetFont( pclFont );
	SetWindowText( m_sInitText );
	SetFocus();
	SetSel( 0, -1 );
	return 0;
}
