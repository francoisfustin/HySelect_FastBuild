#include "stdafx.h"
#include "TASelect.h"
#include "ExtComboBox.h"


CExtComboBoxDBTools::CExtComboBoxDBTools( CComboBox *pCB )
{
	m_pCB = pCB;
}

const IDPTR CExtComboBoxDBTools::GetCBCurSelIDPtr()
{
	if( NULL == m_pCB )
	{
		ASSERTA_RETURN( _NULL_IDPTR );
	}

	IDPTR IDPtr = _NULL_IDPTR;
	int iCurrentSelection = m_pCB->GetCurSel();

	if( iCurrentSelection >= 0 )
	{
		CData *pData = (CData *)m_pCB->GetItemData( iCurrentSelection );

		if( NULL != pData )
		{
			IDPtr = pData->GetIDPtr();
		}
	}

	return IDPtr;
}

void CExtComboBoxDBTools::SetCurSelWithID( CString strID )
{
	if( NULL == m_pCB )
	{
		ASSERT_RETURN;
	}

	int iSel = FindCBIDPtr( strID );

	if( -1 != iSel )
	{
		m_pCB->SetCurSel( iSel );
	}
}

const int CExtComboBoxDBTools::FindCBIDPtr( CString strID )
{
	if( NULL == m_pCB )
	{
		ASSERTA_RETURN( -1 );
	}

	int iReturn = -1;
	IDPTR IDPtr = _NULL_IDPTR;

	for( int iPos = 0; iPos < m_pCB->GetCount() && -1 == iReturn; iPos++ )
	{
		CData *pData = (CData *)m_pCB->GetItemData( iPos );

		if( NULL != pData )
		{
			IDPtr = pData->GetIDPtr();

			if( 0 == strID.Compare( IDPtr.ID ) )
			{
				iReturn = iPos;
			}
		}
	}

	return iReturn;
}

const int CExtComboBoxDBTools::FindItemDataPtr( DWORD_PTR dwItemToFind )
{
	if( NULL == m_pCB )
	{
		ASSERTA_RETURN( -1 );
	}

	int iReturn = -1;
	
	for( int iPos = 0; iPos < m_pCB->GetCount() && -1 == iReturn; iPos++ )
	{
		if( dwItemToFind == m_pCB->GetItemData( iPos ) )
		{
			iReturn = iPos;
		}
	}

	return iReturn;
}

void CExtComboBoxDBTools::FillInCombo( CRankEx *pList, CString strID, int iAllIds, bool bIsCustomized )
{
	if( NULL == m_pCB || NULL == pList )
	{
		ASSERT_RETURN;
	}

	m_pCB->ResetContent();
	pList->Transfer( m_pCB, bIsCustomized );

	// I have removed 'ASSERT( m_pCB->GetCount() )' because it's possible to have no item or just 'AllIds'.
	// As in the case of actuator combos in 'CTabCDialogSSelTrv'.

	// Add "** All..." string
	if( 0 != iAllIds && m_pCB->GetCount() > 1 )
	{
		CString str = TASApp.LoadLocalizedString( iAllIds );
		m_pCB->InsertString( 0, str );
		m_pCB->SetItemData( 0, 0 );
	}
	
	int iSelPos = max( FindCBIDPtr( strID ), 0 );
	m_pCB->SetCurSel( iSelPos );

	if( m_pCB->GetCount() <= 1 )
	{
		m_pCB->EnableWindow( FALSE );
	}
	else
	{
		m_pCB->EnableWindow( TRUE );
	}
}

///////////////////////////////////////////////////////////
//
//						CExtNumEditComboBox
//
///////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CExtNumEditComboBox, CComboBox )

CExtNumEditComboBox::CExtNumEditComboBox() : CExtComboBoxDBTools( this )
{
	// By default, we set white for all edit text controls.
	m_crBackGnd = _WHITE;
	m_brBackGnd.CreateSolidBrush( m_crBackGnd );
}

CExtNumEditComboBox::~CExtNumEditComboBox()
{
	// Delete brush.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}
}

void CExtNumEditComboBox::SetBackColor( COLORREF rgb )
{
	// Set background color ref (used for text's background).
	m_crBackGnd = rgb;
	
	// Free brush.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}
	
	// Set brush to new color.
	m_brBackGnd.CreateSolidBrush( rgb );
	
	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

COLORREF CExtNumEditComboBox::GetBackColor( void )
{
	COLORREF clReturn = -1;

	if( NULL !=  m_brBackGnd.GetSafeHandle() )
	{
		clReturn = m_crBackGnd;
	}

	return clReturn;
}

BEGIN_MESSAGE_MAP( CExtNumEditComboBox, CComboBox )
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT( CBN_DROPDOWN, OnCbnDropdown )
END_MESSAGE_MAP()

// CExtNumEditComboBox message handlers
BOOL CExtNumEditComboBox::PreTranslateMessage( MSG *pMsg )
{
	// Notify Enter key to the parent window
	if( WM_KEYDOWN == pMsg->message && VK_RETURN == pMsg->wParam )
	{
		NMHDR hdr;
		hdr.hwndFrom = GetSafeHwnd();
		hdr.idFrom = GetDlgCtrlID();
		hdr.code = WM_USER_ENTERKEYPRESSED;
		GetParent()->SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&hdr );
	}

	return CComboBox::PreTranslateMessage( pMsg );
}

HBRUSH CExtNumEditComboBox::CtlColor( CDC *pDC, UINT nCtlColor )
{
	// The combo box control can contain either a 'CStatic' or 'CEdit' control in regards to the style.
	// If 'Simple' or 'Drop-down', it's a 'CEdit' control, if 'Drop List' it's a 'CStatic' control.
	if( CBS_DROPDOWN == ( CBS_DROPDOWN & GetStyle() ) || CBS_SIMPLE == ( CBS_SIMPLE & GetStyle() ) )
	{
		// Depending of the current state of the control when Windows wants to paint it, 'nCtlColor' can be
		// either 'CTLCOLOR_EDIT' in the case of the combobox is in an enabled state or can be 'CTLCOLOR_STATIC'
		// in the case of the disabled state.
		if( ( CTLCOLOR_EDIT == nCtlColor || CTLCOLOR_STATIC == nCtlColor )
				&& NULL == m_clExtNumEdit.GetSafeHwnd() )
		{
			// If we have not yet sub classed the 'CEdit' control we do it now. So we are now able to
			// catch the 'CtlColor' message with our 'CExtStatic' class.
			m_clExtNumEdit.SubclassWindow( GetDlgItem( 1001 )->GetSafeHwnd() );
		}
	}

	// Set the static background color only if has been defined by calling 'SetBackColor'.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		pDC->SetBkColor( m_crBackGnd );
	}
	else
	{
		pDC->SetBkMode( TRANSPARENT );
	}

	// Return the brush used for background if set otherwise NULL.
	return ( NULL != m_brBackGnd.GetSafeHandle() ) ? m_brBackGnd : (HBRUSH)GetStockObject( NULL_BRUSH );
}

void CExtNumEditComboBox::OnCbnDropdown()
{
    // Find the longest string in the combo box.
	CString str;
	CSize sz;
	int dx = 0;
	TEXTMETRIC tm;
	CDC *pDC = GetDC();
	CFont *pFont = GetFont();

	// Select the listbox font, save the old font.
	CFont *pOldFont = pDC->SelectObject( pFont );

	// Get the text metrics for avg char width.
	pDC->GetTextMetrics( &tm );

	for( int i = 0; i < GetCount(); i++ )
	{
	   GetLBText( i, str );
	   sz = pDC->GetTextExtent( str );

	   // Add the avg width to prevent clipping
	   sz.cx += tm.tmAveCharWidth;

	   if( sz.cx > dx )
	   {
		  dx = sz.cx;
	   }
	}
	
	// Select the old font back into the DC.
	pDC->SelectObject( pOldFont );
	ReleaseDC( pDC );

	// Adjust the width for the vertical scroll bar and the left and right border.
	dx += ::GetSystemMetrics( SM_CXVSCROLL ) + 2 * ::GetSystemMetrics( SM_CXEDGE );

	// Set the width of the list box so that every item is completely visible.
	SetDroppedWidth( dx );
}

void CExtNumEditComboBox::OnDestroy()
{
	if( NULL != m_clExtNumEdit.GetSafeHwnd() )
	{
		m_clExtNumEdit.UnsubclassWindow();
	}

	if( NULL != m_clExtStatic.GetSafeHwnd() )
	{
		m_clExtStatic.UnsubclassWindow();
	}
 
	CComboBox::OnDestroy();
}

///////////////////////////////////////////////////////////
//
//						CExtEditComboBox
//
///////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CExtEditComboBox, CComboBox )

CExtEditComboBox::CExtEditComboBox() : CExtComboBoxDBTools( this )
{
	// By default, we set white for all edit text controls.
	m_crBackGnd = _WHITE;
	m_brBackGnd.CreateSolidBrush( m_crBackGnd );
}

CExtEditComboBox::~CExtEditComboBox()
{
	// Delete brush.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}
}

void CExtEditComboBox::SetBackColor( COLORREF rgb )
{
	// Set background color ref (used for text's background).
	m_crBackGnd = rgb;
	
	// Free brush.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}
	
	// Set brush to new color.
	m_brBackGnd.CreateSolidBrush( rgb );
	
	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

COLORREF CExtEditComboBox::GetBackColor( void )
{
	COLORREF clReturn = -1;

	if( NULL !=  m_brBackGnd.GetSafeHandle() )
	{
		clReturn = m_crBackGnd;
	}

	return clReturn;
}

BEGIN_MESSAGE_MAP( CExtEditComboBox, CComboBox )
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT( CBN_DROPDOWN, OnCbnDropdown )
END_MESSAGE_MAP()

// CExtNumEditComboBox message handlers
BOOL CExtEditComboBox::PreTranslateMessage( MSG *pMsg )
{
	// Notify Enter key to the parent window
	if( WM_KEYDOWN == pMsg->message && VK_RETURN == pMsg->wParam )
	{
		NMHDR hdr;
		hdr.hwndFrom = GetSafeHwnd();
		hdr.idFrom = GetDlgCtrlID();
		hdr.code = WM_USER_ENTERKEYPRESSED;
		GetParent()->SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&hdr );
	}

	return CComboBox::PreTranslateMessage( pMsg );
}

HBRUSH CExtEditComboBox::CtlColor( CDC *pDC, UINT nCtlColor )
{
	// The combo box control can contain either a 'CStatic' or 'CEdit' control in regards to the style.
	// If 'Simple' or 'Drop-down', it's a 'CEdit' control, if 'Drop List' it's a 'CStatic' control.
	if( CBS_DROPDOWN == ( CBS_DROPDOWN & GetStyle() ) || CBS_SIMPLE == ( CBS_SIMPLE & GetStyle() ) )
	{
		// Depending of the current state of the control when Windows wants to paint it, 'nCtlColor' can be
		// either 'CTLCOLOR_EDIT' in the case of the combobox is in an enabled state or can be 'CTLCOLOR_STATIC'
		// in the case of the disabled state.
		if( ( CTLCOLOR_EDIT == nCtlColor || CTLCOLOR_STATIC == nCtlColor )
				&& NULL == m_clExtEdit.GetSafeHwnd() )
		{
			// If we have not yet sub classed the 'CEdit' control we do it now. So we are now able to
			// catch the 'CtlColor' message with our 'CExtStatic' class.
			m_clExtEdit.SubclassWindow( GetDlgItem( 1001 )->GetSafeHwnd() );
		}
	}

	// Set the static background color only if has been defined by calling 'SetBackColor'.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		pDC->SetBkColor( m_crBackGnd );
	}
	else
	{
		pDC->SetBkMode( TRANSPARENT );
	}

	// Return the brush used for background if set otherwise NULL.
	return ( NULL != m_brBackGnd.GetSafeHandle() ) ? m_brBackGnd : (HBRUSH)GetStockObject( NULL_BRUSH );
}

void CExtEditComboBox::OnCbnDropdown()
{
    // Find the longest string in the combo box.
	CString str;
	CSize sz;
	int dx = 0;
	TEXTMETRIC tm;
	CDC *pDC = GetDC();
	CFont *pFont = GetFont();

	// Select the listbox font, save the old font.
	CFont *pOldFont = pDC->SelectObject( pFont );

	// Get the text metrics for avg char width.
	pDC->GetTextMetrics( &tm );

	for( int i = 0; i < GetCount(); i++ )
	{
	   GetLBText( i, str );
	   sz = pDC->GetTextExtent( str );

	   // Add the avg width to prevent clipping
	   sz.cx += tm.tmAveCharWidth;

	   if( sz.cx > dx )
	   {
		  dx = sz.cx;
	   }
	}
	
	// Select the old font back into the DC.
	pDC->SelectObject( pOldFont );
	ReleaseDC( pDC );

	// Adjust the width for the vertical scroll bar and the left and right border.
	dx += ::GetSystemMetrics( SM_CXVSCROLL ) + 2 * ::GetSystemMetrics( SM_CXEDGE );

	// Set the width of the list box so that every item is completely visible.
	SetDroppedWidth( dx );
}

void CExtEditComboBox::OnDestroy()
{
	if( NULL != m_clExtEdit.GetSafeHwnd() )
	{
		m_clExtEdit.UnsubclassWindow();
	}

	if( NULL != m_clExtStatic.GetSafeHwnd() )
	{
		m_clExtStatic.UnsubclassWindow();
	}
 
	CComboBox::OnDestroy();
}

///////////////////////////////////////////////////////////
//
//						CExtComboBoxEx
//
///////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CExtComboBoxEx, CComboBoxEx )

CExtComboBoxEx::CExtComboBoxEx() : CExtComboBoxDBTools( this )
{
	m_pclImageListNormal = NULL;
	m_pclImageListGrayed = NULL;
	m_fIsEnabled = true;
}

bool CExtComboBoxEx::EnableWindowW( bool bEnable, bool bRecurseChildren )
{
	bool bReturn;

	if( true == bEnable )
	{
		bReturn = ( TRUE == EnableWindow( TRUE ) ) ? true : false;

		if( NULL != m_pclImageListNormal )
		{
			SetImageList( m_pclImageListNormal );
		}
	}
	else
	{
		bReturn = ( TRUE == EnableWindow( FALSE ) ) ? true : false;

		if( NULL != m_pclImageListGrayed )
		{
			SetImageList( m_pclImageListGrayed );
		}
	}
	
	m_fIsEnabled = bEnable;
	return bReturn;
}

CImageList *CExtComboBoxEx::SetImageListW( CImageList *pclImageList, bool bEnabledMode )
{
	CImageList *pclReturn = pclImageList;
	
	if( true == bEnabledMode )
	{
		m_pclImageListNormal = pclReturn;
	}
	else
	{
		m_pclImageListGrayed = pclReturn;
	}
	
	if( true == bEnabledMode && true == m_fIsEnabled && NULL != m_pclImageListNormal )
	{
		pclReturn = SetImageList( m_pclImageListNormal );
	}
	else if( false == bEnabledMode && false == m_fIsEnabled && NULL != m_pclImageListGrayed )
	{
		pclReturn = SetImageList( m_pclImageListGrayed );
	}
	
	return pclReturn;
}

BEGIN_MESSAGE_MAP( CExtComboBoxEx, CComboBoxEx )
	ON_CONTROL_REFLECT( CBN_DROPDOWN, OnCbnDropdown )
END_MESSAGE_MAP()

void CExtComboBoxEx::OnCbnDropdown()
{
    // Find the longest string in the combo box.
	CString str;
	CSize sz;
	int dx = 0;
	TEXTMETRIC tm;
	CDC *pDC = GetDC();
	CFont *pFont = GetFont();

	// Select the listbox font, save the old font.
	CFont *pOldFont = pDC->SelectObject( pFont );
	
	// Get the text metrics for avg char width.
	pDC->GetTextMetrics( &tm );

	for( int i = 0; i < GetCount(); i++ )
	{
	   GetLBText( i, str );
	   sz = pDC->GetTextExtent( str );

	   // Add the avg width to prevent clipping.
	   sz.cx += tm.tmAveCharWidth;

	   if( sz.cx > dx )
	   {
		  dx = sz.cx;
	   }
	}
	
	// Select the old font back into the DC.
	pDC->SelectObject( pOldFont );
	ReleaseDC( pDC );

	// Adjust the width for the vertical scroll bar and the left and right border.
	dx += ::GetSystemMetrics( SM_CXVSCROLL ) + 2 * ::GetSystemMetrics( SM_CXEDGE );

	// Set the width of the list box so that every item is completely visible.
	SetDroppedWidth( dx );
}

void CExtComboBoxEx::PreSubclassWindow()
{
	if( NULL != m_hWnd )
	{
		ModifyStyle( 0, CBS_OWNERDRAWFIXED );
	}

	CComboBoxEx::PreSubclassWindow();
}

BOOL CExtComboBoxEx::PreTranslateMessage( MSG *pMsg )
{
	if( NULL == m_hWnd || WM_DRAWITEM != pMsg->message )
	{
		return 0;
	}

	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)pMsg->lParam;
	_DrawItem( lpDrawItemStruct );
	return 1;
}

void CExtComboBoxEx::_DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if( -1 == lpDrawItemStruct->itemID )
	{
		return;
	}

	// Get the icon from the item data.
	HBITMAP hbmIcon = (HBITMAP)lpDrawItemStruct->itemData;

	// The colors depend on whether the item is selected.
	COLORREF clrForeground = SetTextColor( lpDrawItemStruct->hDC, GetSysColor( lpDrawItemStruct->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT ) );
	COLORREF clrBackground = SetBkColor(lpDrawItemStruct->hDC, GetSysColor( lpDrawItemStruct->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW ) );

	// Calculate the vertical and horizontal position.
	TEXTMETRIC tm;
	GetTextMetrics( lpDrawItemStruct->hDC, &tm );
	int y = ( lpDrawItemStruct->rcItem.bottom + lpDrawItemStruct->rcItem.top - tm.tmHeight ) / 2;
	int x = LOWORD( GetDialogBaseUnits() ) / 4;

	// Get and display the text for the list item.
	TCHAR achTemp[256];
	::SendMessage( lpDrawItemStruct->hwndItem, CB_GETLBTEXT, lpDrawItemStruct->itemID, (LPARAM)achTemp );

	size_t cch = wcslen( achTemp );
	ExtTextOut( lpDrawItemStruct->hDC, CX_BITMAP + 2 * x, y, ETO_CLIPPED | ETO_OPAQUE, &lpDrawItemStruct->rcItem, achTemp, (UINT)cch, NULL );

	// Restore the previous colors.
	SetTextColor( lpDrawItemStruct->hDC, clrForeground );
	SetBkColor( lpDrawItemStruct->hDC, clrBackground );
    
	// Draw the food icon for the item. 
	HDC hdc = CreateCompatibleDC( lpDrawItemStruct->hDC ); 

	if( NULL == hdc )
	{
		return; 
	}
 
	SelectObject( hdc, hbmIcon );
	BitBlt( lpDrawItemStruct->hDC, x, lpDrawItemStruct->rcItem.top + 1, CX_BITMAP, CY_BITMAP, hdc, 0, 0, SRCPAINT );
 
	DeleteDC( hdc );
  
	// If the item has the focus, draw the focus rectangle.
	if( ODS_FOCUS == ( lpDrawItemStruct->itemState & ODS_FOCUS ) )
	{
		DrawFocusRect( lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem );
	}
}
