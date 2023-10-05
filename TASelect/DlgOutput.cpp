#include "stdafx.h"
#include <algorithm>
#include "TASelect.h"
#include "DlgOutput.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC( CDlgOutput, CDialogEx )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CButtonNormal class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CDlgOutput::CButtonNormal, CMFCButton )
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void CDlgOutput::CButtonNormal::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( m_pDlgOutput != NULL )
		m_pDlgOutput->OnHeaderButtonClicked( GetDlgCtrlID(), State::Unpressed );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CButtonSpecial class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgOutput::CButtonSpecial::CButtonSpecial( CVButton::State eInitialState, CDlgOutput *pDlgOutput )
		: CDialogEx( (UINT)0 ), CVButton( pDlgOutput ) 
{ 
	m_strText = _T("");
	m_hIcon = (HICON)0;
	ZeroMemory( &m_rIconInfos, sizeof( m_rIconInfos ) );
	m_bHighlighted = false;
	m_eButtonState = eInitialState;
}

CDlgOutput::CButtonSpecial::~CButtonSpecial()
{
	if( m_hIcon != (HICON)0 )
	{
		DeleteObject( m_hIcon );
	}
}

void CDlgOutput::CButtonSpecial::SetIcon( int iIconID )
{
	if( m_hIcon != (HICON)0 )
	{
		DeleteObject( m_hIcon );
		m_hIcon = (HICON)0;
	}

	// Remark: it's a better way to use 'LoadImage' for API32 that the one from MFC 'CWinApp::LoadIcon'.
	// 2 sizes exist in the icon: 16x16 and 32x32. The first size is for the button and second one is for the spread.
	HICON hIcon = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( iIconID ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	
	if( hIcon != NULL )
	{
		// Retrieve the info about this icon.
		ICONINFO rIconInfo;
		
		if( FALSE == GetIconInfo( hIcon, &rIconInfo ) )
		{
			return;
		}

		CDC dc;
		dc.CreateCompatibleDC( NULL );

		// Load info of the bitmap from the mask or color bitmap.
		ZeroMemory( &m_rIconInfos, sizeof( m_rIconInfos ) );
		int iResult;
		bool fColor = false;
		
		if( rIconInfo.hbmColor != NULL )
		{
			// Icon is in color.
			iResult = GetObject( rIconInfo.hbmColor, sizeof( m_rIconInfos ), (LPVOID)&m_rIconInfos );
			fColor = true;
			
			// 'hbmMask' is created only for the color icon.
			DeleteObject( rIconInfo.hbmColor );
		}
		else
		{
			// Icon is in monochrome.
			iResult = GetObject( rIconInfo.hbmMask, sizeof( m_rIconInfos ), (LPVOID)&m_rIconInfos );
	
		}
		
		// 'hbmMask' is created in both case (color or monochrome icon).
		DeleteObject( rIconInfo.hbmMask );
		
		if( 0 == iResult )
		{
			DeleteObject( hIcon );
			return;
		}

		m_hIcon = hIcon;
	}
}

BEGIN_MESSAGE_MAP( CDlgOutput::CButtonSpecial, CDialogEx )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CDlgOutput::CButtonSpecial::OnLButtonDown( UINT nFlags, CPoint point )
{
	m_eButtonState = ( State::Unpressed == m_eButtonState ) ? State::Pressed : State::Unpressed;

	if( m_pDlgOutput != NULL )
	{
		m_pDlgOutput->OnHeaderButtonClicked( GetDlgCtrlID(), m_eButtonState );
	}

	Invalidate();
}

void CDlgOutput::CButtonSpecial::OnMouseMove( UINT nFlags, CPoint point )
{
	CRect rectClient;
	GetClientRect( &rectClient );
	
	if( false == m_bHighlighted )
	{
		if( TRUE == rectClient.PtInRect( point ) )
		{
			SetCapture();
			m_bHighlighted = true;
			Invalidate();
		}
	}
	else
	{
		if( FALSE == rectClient.PtInRect( point ) )
		{
			ReleaseCapture();
			m_bHighlighted = false;
			Invalidate();
		}
	}
}

void CDlgOutput::CButtonSpecial::OnPaint()
{
	CPaintDC dc( this ); // device context for painting

	CRect rectClient;
	GetClientRect( &rectClient );

	// Draw border.
	if( State::Pressed == m_eButtonState || true == m_bHighlighted )
	{
		// Create border pen.
		CPen clPen;
		COLORREF refPenColor;
		refPenColor = BS_COLOR_HIGHLIGHTED_BORDER_SOLID;
		clPen.CreatePen( PS_SOLID, 1, refPenColor );
		CPen* pOldPen = dc.SelectObject( &clPen );

		dc.Rectangle( &rectClient );
		dc.SelectObject( pOldPen );
	}
	
	// Draw background.
	CRect rectBackground( rectClient );
	rectBackground.DeflateRect( 1, 1 );
	CBrush clBrush;
	CBrush* pOldBrush = NULL;
	
	// The highlighting effect is priority.
	if( true == m_bHighlighted )
	{
		CDrawingManager dcm( dc );
		
		// First part.
		CRect rectGradient( rectBackground );
		rectGradient.bottom >>= 1;
		dcm.FillGradient2( rectGradient, BS_COLOR_HIGHLIGHTED_TOP_START, BS_COLOR_HIGHLIGHTED_TOP_FINISH, 270 );

		// Second part.
		CRect rectFill( rectBackground );
		rectFill.top = rectGradient.bottom + 1;
		clBrush.CreateSolidBrush( BS_COLOR_HIGHLIGHTED_BOTTOM_SOLID );
		pOldBrush = dc.SelectObject( &clBrush );
		dc.FillRect( rectFill, & clBrush );
	}
	else if( State::Pressed == m_eButtonState )
	{
		clBrush.CreateSolidBrush( BS_COLOR_PRESSED_SOLID );
		pOldBrush = dc.SelectObject( &clBrush );
		dc.FillRect( rectBackground, &clBrush );
	}
	
	if( pOldBrush != NULL )
	{
		dc.SelectObject( pOldBrush );
	}

	// Draw image.
	CRect rectText( rectClient );
	CRect rectImage( rectClient );
	BITMAP bmp;
	
	if( (HBITMAP)m_clBitmap != (HBITMAP)0 && m_clBitmap.GetBitmap( &bmp ) != 0 )
	{
		rectText.left += bmp.bmWidth + ( 2 * BS_PIXELS_EDGE_IMAGE );
		rectImage.left += BS_PIXELS_EDGE_IMAGE;
		rectImage.right = rectImage.left + bmp.bmWidth;
		// Center image.
		int iDeflateX = max( 0, ( rectImage.Width() - bmp.bmWidth ) >> 1 );
		int iDeflateY = max( 0, ( rectImage.Height() - bmp.bmHeight ) >> 1 );
		rectImage.DeflateRect( iDeflateX, iDeflateY );
		dc.DrawState( CPoint( rectImage.left, rectImage.top ), CSize( rectImage.Width(), rectImage.Height() ), &m_clBitmap, DST_BITMAP );
	}
	else if( m_hIcon != (HICON)0 )
	{
		rectText.left +=  m_rIconInfos.bmWidth + ( 2 * BS_PIXELS_EDGE_IMAGE );
		rectImage.left += BS_PIXELS_EDGE_IMAGE;
		rectImage.right = rectImage.left + m_rIconInfos.bmWidth;
		// Center image.
		int iDeflateX = max( 0, ( rectImage.Width() - m_rIconInfos.bmWidth ) >> 1 );
		int iDeflateY = max( 0, ( rectImage.Height() - m_rIconInfos.bmHeight ) >> 1 );
		rectImage.DeflateRect( iDeflateX, iDeflateY );
		dc.DrawState( CPoint( rectImage.left, rectImage.top ), CSize( rectImage.Width(), rectImage.Height() ), m_hIcon, DST_ICON | DSS_NORMAL, HBRUSH( 0 ) );
	}
	else
	{
		rectText.left = BS_PIXELS_LEFTEDGE_TEXT;
	}

	// Draw text.
	CFont *pOldFont = (CFont*)dc.SelectStockObject( DEFAULT_GUI_FONT );
	dc.SetBkMode( TRANSPARENT );
	UINT uiDTFlags = DT_END_ELLIPSIS | DT_LEFT;

	// Center text.
	CSize sizeText = dc.GetTextExtent( m_strText );
	rectText.DeflateRect( 0, max( 0, ( rectClient.Height() - sizeText.cy ) >> 1 ) );
	dc.DrawText( m_strText, rectText, uiDTFlags );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 'CBaseInterface' class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgOutput::CBaseInterface::CBaseInterface()
{
	m_pDlgOutput = NULL;
}

bool CDlgOutput::CBaseInterface::Reset( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->Reset( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SetFocus( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SetFocus( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ClearOutput( bool fAlsoHeader, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ClearOutput( fAlsoHeader, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::BlockOutput( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->BlockOutput( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ReleaseOutput( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ReleaseOutput( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::BlockRedraw( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->BlockRedraw( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ReleaseRedraw( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ReleaseRedraw( iOutputID );
	}
	return bReturn;
}

bool CDlgOutput::CBaseInterface::IsRedrawBlocked( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->IsRedrawBlocked( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::EnableEdition( bool fEnable, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->EnableEdition( fEnable, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::IsEditionEnabled( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->IsEditionEnabled( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::IsCellEditable( int iColID, long lRowRelative, bool& fIsEditable, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->IsCellEditable( iColID, lRowRelative, fIsEditable, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::EditCurrentCell( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->EditCurrentCell( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ResetScrolling( int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ResetScrolling( iOutputID );
	}
	return bReturn;
}

bool CDlgOutput::CBaseInterface::AutomaticColumnResize( bool fAutomaticColumnResize, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AutomaticColumnResizing( fAutomaticColumnResize, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AutomaticLastBlankLine( bool fEnable, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AutomaticLastBlankLine( fEnable, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SetSelectionColor( COLORREF backgroundColor, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SetSelectionColor( backgroundColor, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::EnableBlockSelection( bool fEnable, WORD wBorderStyle, COLORREF backgroundColor, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->EnableBlockSelection( fEnable, wBorderStyle, backgroundColor, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::IsBlockSelectionExist( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->IsBlockSelectionExist( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetBlockSelection( int& iStartColID, long& lStartRow, int& iEndColID, long& lEndRow, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetBlockSelection( iStartColID, lStartRow, iEndColID, lEndRow, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SetBlockSelection( int iStartColID, long lStartRow, int iEndColID, long lEndRow, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SetBlockSelection( iStartColID, lStartRow, iEndColID, lEndRow, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ResetBlockSelectionToOrigin( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ResetBlockSelectionToOrigin( iOutputID );
	}

	return bReturn;
}		

bool CDlgOutput::CBaseInterface::EraseBlockSelection( int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->EraseBlockSelection( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::EnableRowSelection( bool fEnable, int iColumnIDRef, COLORREF backgroundColor, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->EnableRowSelection( fEnable, iColumnIDRef, backgroundColor, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SetColumnWidth( int iColumnID, double dWidth, int iOutputID )
{
	bool bReturn = false;

	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SetColumnWidth( iColumnID, dWidth, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SetColumnWidthInPixel( int iColumnID, long lWidth, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SetColumnWidthInPixel( iColumnID, lWidth, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetColumnWidth( int iColumnID, double& dWidth, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetColumnWidth( iColumnID, dWidth, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetColumnWidthInPixel( int iColumnID, long& lWidth, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{

		bReturn = m_pDlgOutput->GetColumnWidthInPixel( iColumnID, lWidth, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SetDataRowHeight( double dRowHeight, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SetDataRowHeight( dRowHeight, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetColumnHeaderRowsAbsolute( long& lColumnAbsoluteStart, long& lColumnAbsoluteEnd, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetColumnHeaderRowsAbsolute( lColumnAbsoluteStart, lColumnAbsoluteEnd, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::EnableTopPanelButton( int iButtonID, bool fEnable, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->EnableTopPanelButton( iButtonID, fEnable, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::EnableTextTipFetch( bool fSet, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->EnableTextTipFetch( fSet, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AddTextTipFetchTrigger( DlgOutputHelper::CCellBase* pclCellBase, long lRow, CString strText, int iOutputID )
{
	if( NULL == pclCellBase )
	{
		return false;
	}

	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AddTextTipFetchTrigger( pclCellBase, lRow, strText, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::RemoveAllTextTipFetchTrigger( int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->RemoveAllTextTipFetchTrigger( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AddBlankLineBeforeMainHeader( double dHeight, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AddBlankLineBeforeMainHeader( dHeight, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AddMainHeaderDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CColDef& clColDef, CString strText, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AddMainHeaderDef( iMainHeaderID, iMainHeaderSubID, clColDef, strText, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AddMainHeaderFontDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CFontDef& clFontDef, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AddMainHeaderFontDef( iMainHeaderID, iMainHeaderSubID, clFontDef, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SetMainHeaderRowHeight( int iMainHeaderID, double dHeight, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SetMainHeaderRowHeight( iMainHeaderID, dHeight, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SetColumnHeaderRows( long lRow, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SetColumnHeaderRows( lRow, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::FreezeAllHeaderRows( bool fFreeze, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->FreezeAllHeaderRows( fFreeze, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AddColumnHeaderDef( int iColumnID, DlgOutputHelper::CColDef& clColDef, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AddColumnHeaderDef( iColumnID, clColDef, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AddColumnHeaderFontDef( int iColumnID, DlgOutputHelper::CFontDef& clFontDef, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AddColumnHeaderFontDef( iColumnID, clFontDef, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::FillColumnHeader( int iColumnID, long lRow, CString strData, double dHeight, bool fLineBelow, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->FillColumnHeader( iColumnID, lRow, strData, dHeight, fLineBelow, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AddColumnDefinition( int iColumnID, DlgOutputHelper::CColDef& clColDef, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AddColumnDefinition( iColumnID, clColDef, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::RemoveColumn( int iColumnID, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->RemoveColumn( iColumnID, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::AddColumnFontDefiniton( int iColumnID, DlgOutputHelper::CFontDef& clFontDef, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->AddColumnFontDefiniton( iColumnID, clFontDef, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetRows( long& lRows, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetRows( lRows, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetTotalRows( long& lTotalRows, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetTotalRows( lTotalRows, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetFirstRow( DlgOutputHelper::mapIntCellBase* pmapCellData, long& lCurrentRowRelative, int iOutputID )
{
	if( NULL == pmapCellData )
	{
		return false;
	}
	
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetFirstRow( pmapCellData, lCurrentRowRelative, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetNextRow( DlgOutputHelper::mapIntCellBase* pmapCellData, long& lCurrentRowRelative, int iOutputID )
{
	if( NULL == pmapCellData )
	{
		return false ;
	}
	
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetNextRow( pmapCellData, lCurrentRowRelative, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetRow( DlgOutputHelper::mapIntCellBase* pmapCellData, long lRowRelative, int iOutputID )
{
	if( NULL == pmapCellData )
	{
		return false ;
	}

	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetRow( pmapCellData, lRowRelative, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::IsRowValidForData( long lRowRelative, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->IsRowValidForData( lRowRelative, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SelectRow( long lRowRelative, bool fAutoScroll, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SelectRow( lRowRelative, fAutoScroll, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetSelectedRow( long& lCurrentSelectedRowRelative, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetSelectedRow( lCurrentSelectedRowRelative, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::SaveSelections( CSSheet::SaveSelectionChoice eWhichSelection, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->SaveSelections( eWhichSelection, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ClearCurrentSelections( CSSheet::SaveSelectionChoice eWhichSelection, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ClearCurrentSelections( eWhichSelection, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::RestoreSelections( int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->RestoreSelections( iOutputID );
	}
	
	return bReturn;
}

bool CDlgOutput::CBaseInterface::GetColID2Absolute( int iColumnID, long& lColAbsolute, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->GetColID2Absolute( iColumnID, lColAbsolute, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ResetCellContent( int iColumnID, long lFromRowRelative, long lToRowRelative, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ResetCellContent( iColumnID, lFromRowRelative, lToRowRelative, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ChangeCellBackground( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative, COLORREF crColor, bool fForceColorWhenSelection, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ChangeCellBackground( iStartColID, iEndColID, lStartRowRelative, lEndRowRelative, crColor, fForceColorWhenSelection, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ResizingColumnActivate( bool fActivate, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ResizingColumnActivate( fActivate, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ResizingColumnAddRange( long lColStartAbsolutePos, long lColEndAbsolutePos, long lRowStart, long lRowEnd, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ResizingColumnAddRange( lColStartAbsolutePos, lColEndAbsolutePos, lRowStart, lRowEnd, iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::ResizingColumnRemoveAll( int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		bReturn = m_pDlgOutput->ResizingColumnRemoveAll( iOutputID );
	}

	return bReturn;
}

bool CDlgOutput::CBaseInterface::RegisterNotificationHandler( CDlgOutput::IBaseNotificationHandler* pclHandler, int iNotificationHandlerFlags, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		m_pDlgOutput->RegisterNotificationHandler( pclHandler, iNotificationHandlerFlags, iOutputID );
		bReturn = true;
	}
	
	return bReturn;
}

bool CDlgOutput::CBaseInterface::UnregisterNotificationHandler( CDlgOutput::IBaseNotificationHandler* pclHandler, int iOutputID )
{
	bool bReturn = false;
	
	if( true == Init() )
	{
		m_pDlgOutput->UnregisterNotificationHandler( pclHandler, iOutputID );
		bReturn = true;
	}
	
	return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgOutput class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgOutput::CDlgOutput( bool bCreateOutput, int iOutputID, bool bUseSpreadContainer )
{
	memset( m_arwTemplate, 0, sizeof( m_arwTemplate ) );
	m_bCreateOutput = bCreateOutput;
	m_iOutputID = iOutputID;
	m_bUseSpreadContainer = bUseSpreadContainer;
	m_pclCurrentDlgOutputSpreadContainer = NULL;
}

CDlgOutput::~CDlgOutput()
{
	for( iterOutputContext iter = m_mapOutputContextParams.begin(); iter != m_mapOutputContextParams.end(); iter++ )
	{
		if( NULL != iter->second.m_pclDlgOutputSpreadContainer )
		{
			delete iter->second.m_pclDlgOutputSpreadContainer;
		}
	}
	m_mapOutputContextParams.clear();
	m_pclCurrentDlgOutputSpreadContainer = NULL;
	_ClearAllTopPanelButtons();
}

bool CDlgOutput::ActivateOutput( int iOutputID, bool bShow )
{
	bool bSame = false;

	if( 0 == m_mapOutputContextParams.count( iOutputID ) )
	{
		if( false == AddOutput( iOutputID ) )
		{
			return false;
		}
	}
	else if( m_pclCurrentDlgOutputSpreadContainer == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		bSame = true;
	}

	if( false == bSame && NULL != m_pclCurrentDlgOutputSpreadContainer )
	{
		if( true == bShow )
		{
			m_pclCurrentDlgOutputSpreadContainer->ShowWindow( SW_HIDE );
		}
	}
	
	m_pclCurrentDlgOutputSpreadContainer = m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer;

	if( true == bShow && NULL != m_pclCurrentDlgOutputSpreadContainer )
	{
		m_pclCurrentDlgOutputSpreadContainer->ShowWindow( SW_SHOW );
	}

	OnDlgOutputActivated( iOutputID );

	// Activate or not top panel button.
	for( std::map<CVButton*, bool>::iterator iter = m_mapOutputContextParams[iOutputID].m_mapTopPanelButtonState.begin(); 
			iter != m_mapOutputContextParams[iOutputID].m_mapTopPanelButtonState.end(); ++iter )
	{
		iter->first->EnableButton( iter->second );
	}

	return true;
}

bool CDlgOutput::IsOutputExist( int iOutputID )
{
	return ( 0 != m_mapOutputContextParams.count( iOutputID ) ) ? true : false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods declared in the 'CBaseInterface' class.

bool CDlgOutput::Reset( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}
	
	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->Reset();
	return true;
}

bool CDlgOutput::SetFocus( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}
	
	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SetFocus();
	return true;
}

bool CDlgOutput::ClearOutput( bool bAlsoHeader, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->Clear( bAlsoHeader );
	return true;
}

bool CDlgOutput::BlockOutput( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->BlockOutput();
	return true;
}

bool CDlgOutput::ReleaseOutput( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}
	
	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ReleaseOutput();
	return true;
}

bool CDlgOutput::BlockRedraw( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->BlockRedraw();
	return true;
}

bool CDlgOutput::ReleaseRedraw( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ReleaseRedraw();
	return true;
}

bool CDlgOutput::IsRedrawBlocked( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->IsRedrawBlocked();
}

bool CDlgOutput::EnableEdition( bool bEnable, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->EnableEdition( bEnable );
}

bool CDlgOutput::IsEditionEnabled( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->IsEditionEnabled();
}

bool CDlgOutput::IsCellEditable( int iColID, long lRowRelative, bool &bIsEditable, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->IsCellEditable( iColID, lRowRelative, bIsEditable );
	return true;
}

bool CDlgOutput::EditCurrentCell( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->EditCurrentCell();
	return true;
}

bool CDlgOutput::ResetScrolling( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ResetScrolling();
	return true;
}

bool CDlgOutput::AutomaticColumnResizing( bool bAutomaticColumnResize, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AutomaticColumnResizing( bAutomaticColumnResize );
	return true;
}

bool CDlgOutput::AutomaticLastBlankLine( bool bEnable, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AutomaticLastBlankLine( bEnable );
	return true;
}

bool CDlgOutput::SetSelectionColor( COLORREF backgroundColor, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SetSelectionColor( backgroundColor );
	return true;
}

bool CDlgOutput::EnableBlockSelection( bool bEnable, WORD wBorderStyle, COLORREF backgroundColor, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->EnableBlockSelection( bEnable, wBorderStyle, backgroundColor );
	return true;
}

bool CDlgOutput::IsBlockSelectionExist( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->IsBlockSelectionExist();
}

bool CDlgOutput::GetBlockSelection( int &iStartColID, long&lStartRow, int &iEndColID, long &lEndRow, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetBlockSelectionRelative( iStartColID, lStartRow, iEndColID, lEndRow );
}

bool CDlgOutput::SetBlockSelection( int iStartColID, long lStartRow, int iEndColID, long lEndRow, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SetBlockSelectionRelative( iStartColID, lStartRow, iEndColID, lEndRow );
}

bool CDlgOutput::ResetBlockSelectionToOrigin( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ResetBlockSelectionToOrigin();
}		

bool CDlgOutput::EraseBlockSelection( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->EraseBlockSelection();
}

bool CDlgOutput::EnableRowSelection( bool bEnable, int iColumnIDRef, COLORREF backgroundColor, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->EnableLineSelection( bEnable, iColumnIDRef, backgroundColor );
	return true;
}

bool CDlgOutput::SetColumnWidth( int iColumnID, double dWidth, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SetColumnWidth( iColumnID, dWidth );
	return true;
}

bool CDlgOutput::SetColumnWidthInPixel( int iColumnID, long lWidth, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SetColumnWidthInPixel( iColumnID, lWidth );
	return true;
}

bool CDlgOutput::GetColumnWidth( int iColumnID, double& dWidth, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetColumnWidth( iColumnID, dWidth );
	return true;
}

bool CDlgOutput::GetColumnWidthInPixel( int iColumnID, long& lWidth, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetColumnWidthInPixel( iColumnID, lWidth );
	return true;
}

bool CDlgOutput::SetDataRowHeight( double dHeight, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SetDataRowHeight( dHeight );
	return true;
}

bool CDlgOutput::GetColumnHeaderRowsAbsolute( long &lColumnAbsoluteStart, long &lColumnAbsoluteEnd, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetColumnHeaderRowsAbsolute( lColumnAbsoluteStart, lColumnAbsoluteEnd );
	return true;
}

bool CDlgOutput::EnableTopPanelButton( int iButtonID, bool bEnable, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	CVButton *pButton = _GetTopPanelButton( iButtonID );
	
	if( NULL == pButton )
	{
		return false;
	}

	pButton->EnableButton( bEnable );

	// Save state.
	m_mapOutputContextParams[iOutputID].m_mapTopPanelButtonState[pButton] = bEnable;
	return true;
}

bool CDlgOutput::EnableTextTipFetch( bool bSet, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->EnableTextTipFetch( bSet );
	return true;
}

bool CDlgOutput::AddTextTipFetchTrigger( DlgOutputHelper::CCellBase *pclCellBase, long lRow, CString strText, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer || NULL == pclCellBase )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AddTextTipFetchTrigger( pclCellBase, lRow, strText );
	return true;
}

bool CDlgOutput::RemoveAllTextTipFetchTrigger( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->RemoveAllTextTipFetchTrigger();
	return true;
}

bool CDlgOutput::AddBlankLineBeforeMainHeader( double dHeight, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AddBlankLineBeforeMainHeader( dHeight );
}

bool CDlgOutput::AddMainHeaderDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CColDef &clColDef, CString strText, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AddMainHeaderDef( iMainHeaderID, iMainHeaderSubID, clColDef, strText );
}

bool CDlgOutput::AddMainHeaderFontDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AddMainHeaderFontDef( iMainHeaderID, iMainHeaderSubID, clFontDef );
}

bool CDlgOutput::SetMainHeaderRowHeight( int iMainHeaderID, double dHeight, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SetMainHeaderRowHeight( iMainHeaderID, dHeight );
}

bool CDlgOutput::SetColumnHeaderRows( long lRow, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SetColumnHeaderRows( lRow );
	return true;
}

bool CDlgOutput::FreezeAllHeaderRows( bool bFreeze, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->FreezeAllHeaderRows( bFreeze );
	return true;
}

bool CDlgOutput::AddColumnHeaderDef( int iColumnID, DlgOutputHelper::CColDef &clColDef, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AddColumnHeaderDef( iColumnID, clColDef );
}

bool CDlgOutput::AddColumnHeaderFontDef( int iColumnID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AddColumnHeaderFontDef( iColumnID, clFontDef );
}

bool CDlgOutput::FillColumnHeader( int iColumnID, long lRow, CString strData, double dHeight, bool bLineBelow, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->FillColumnHeader( iColumnID, lRow, strData, dHeight, bLineBelow );
}

bool CDlgOutput::AddColumnDefinition( int iColumnID, DlgOutputHelper::CColDef &clColDef, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AddColumnDefinition( iColumnID, clColDef );
}

bool CDlgOutput::RemoveColumn( int iColumnID, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->RemoveColumn( iColumnID );
}

bool CDlgOutput::AddColumnFontDefiniton( int iColumnID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->AddColumnFontDefinition( iColumnID, clFontDef );
}

bool CDlgOutput::GetRows( long &lRows, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	lRows = m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetRows();
	return true;
}

bool CDlgOutput::GetTotalRows( long &lTotalRows, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	lTotalRows = m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetTotalRows();
	return true;
}

bool CDlgOutput::GetFirstRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long& lCurrentRowRelative, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer || NULL == pmapCellData )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetFirstRow( pmapCellData, lCurrentRowRelative );
}

bool CDlgOutput::GetNextRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long& lCurrentRowRelative, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer || NULL == pmapCellData )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetNextRow( pmapCellData, lCurrentRowRelative );
}

bool CDlgOutput::GetRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long lRowRelative, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer || NULL == pmapCellData )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetRow( pmapCellData, lRowRelative );
}

bool CDlgOutput::IsRowValidForData( long lRowRelative, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->IsRowValidForData( lRowRelative );
}

bool CDlgOutput::SelectRow( long lRowRelative, bool bAutoScroll, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SelectDataRow( lRowRelative, bAutoScroll );
}

bool CDlgOutput::GetSelectedRow( long &lCurrentSelectedRowRelative, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetSelectedDataRow( lCurrentSelectedRowRelative );
}

bool CDlgOutput::SaveSelections( CSSheet::SaveSelectionChoice eWhichSelection, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SaveSelections( eWhichSelection );
}

bool CDlgOutput::ClearCurrentSelections( CSSheet::SaveSelectionChoice eWhichSelection, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ClearCurrentSelections( eWhichSelection );
}

bool CDlgOutput::RestoreSelections( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->RestoreSelections();
}

bool CDlgOutput::GetColID2Absolute( int iColumnID, long &lColAbsolute, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->GetColID2Absolute( iColumnID, lColAbsolute );
}

bool CDlgOutput::ResetCellContent( int iColumnID, long lFromRowRelative, long lToRowRelative, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ResetCellContent( iColumnID, lFromRowRelative, lToRowRelative );
}

bool CDlgOutput::ChangeCellBackground( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative, COLORREF crColor, 
		bool bForceColorWhenSelection, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ChangeCellBackground( iStartColID, iEndColID, lStartRowRelative, lEndRowRelative, crColor, bForceColorWhenSelection );
}

bool CDlgOutput::ResizingColumnActivate( bool bActivate, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ResizingColumnActivate( bActivate );
}

bool CDlgOutput::ResizingColumnAddRange( long lColStartAbsolutePos, long lColEndAbsolutePos, long lRowStart, long lRowEnd, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}
	
	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ResizingColumnAddRange( lColStartAbsolutePos, lColEndAbsolutePos, lRowStart, lRowEnd );
}

bool CDlgOutput::ResizingColumnRemoveAll( int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ResizingColumnRemoveAll();
}

void CDlgOutput::RegisterNotificationHandler( IBaseNotificationHandler *pclHandler, int iNotificationHandlerFlags, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	// Check first if output ID exist.
	if( 0 == m_mapOutputContextParams.count( iOutputID ) )
	{
		return;
	}

	for( int iLoop = IBaseNotificationHandler::NH_First; iLoop <= IBaseNotificationHandler::NH_Last; iLoop <<= 1 )
	{
		if( iLoop == ( iNotificationHandlerFlags & iLoop ) )
		{
			m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList[iLoop] = pclHandler;
		}
	}
}

void CDlgOutput::UnregisterNotificationHandler( IBaseNotificationHandler *pclHandler, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	// Check first if output ID exist.
	if( 0 == m_mapOutputContextParams.count( iOutputID ) )
	{
		return;
	}

	for( mapIntNotifIter iter = m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList.begin(); iter != m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList.end(); ++iter )
	{
		if( iter->second == pclHandler )
		{
			m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList.erase( iter );
			break;
		}
	}
}

// End of methods declared in the 'CBaseInterface' class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDlgOutput::AddTopPanelButton( int iButtonID, int iStrID, int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton )
{
	CString strCaption = TASApp.LoadLocalizedString( iStrID );
	return _AddTopPanelButton( CVButton::Type::Normal, iButtonID, CVButton::State::Unpressed, strCaption, iImageID, iImageType, bLeftAlign, sizeButton );
}

bool CDlgOutput::AddTopPanelButton( int iButtonID, CString strText, int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton )
{
	return _AddTopPanelButton( CVButton::Type::Normal, iButtonID, CVButton::State::Unpressed, strText, iImageID, iImageType, bLeftAlign, sizeButton );
}

bool CDlgOutput::AddTopPanelPushButton( int iButtonID, CVButton::State eInitialState, int iStrID, int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton )
{
	CString strCaption = TASApp.LoadLocalizedString( iStrID );
	return _AddTopPanelButton( CVButton::Type::Special, iButtonID, eInitialState, strCaption, iImageID, iImageType, bLeftAlign, sizeButton );
}

bool CDlgOutput::AddTopPanelPushButton( int iButtonID, CVButton::State eInitialState, CString strCaption, int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton )
{
	return _AddTopPanelButton( CVButton::Type::Special, iButtonID, eInitialState, strCaption, iImageID, iImageType, bLeftAlign, sizeButton );
}

void CDlgOutput::UpdateTopPanelButtonCaption( int iButtonID, CString strCaption )
{
	CVButton* pButton = _GetTopPanelButton( iButtonID );
	
	if( NULL != pButton )
	{
		pButton->SetCaption( strCaption );
		pButton->GetpWnd()->Invalidate();
	}
}

CDlgOutput::CVButton::State CDlgOutput::GetTopPanelButtonState( int iButtonID )
{
	CVButton::State eButtonState = CVButton::State::NotExist;

	CVButton *pButton = _GetTopPanelButton( iButtonID );

	if( NULL != pButton )
	{
		eButtonState = pButton->GetState();
	}

	return eButtonState;
}

void CDlgOutput::OnOutputSpreadContainerCellClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam *pVecParamList )
{
	if( 0 != m_mapOutputContextParams.count( iOutputID ) 
			&&  0 != m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList.count( IBaseNotificationHandler::NH_OnCellClicked ) )
	{
		long lRelRow = lRow;
		m_pclCurrentDlgOutputSpreadContainer->GetRelativeRow( lRelRow );
		m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList[IBaseNotificationHandler::NH_OnCellClicked]->OnDlgOutputCellClicked( iOutputID, iColumnID, lRelRow );
	}
}

void CDlgOutput::OnOutputSpreadContainerCellDblClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam *pVecParamList )
{
	if( 0 != m_mapOutputContextParams.count( iOutputID ) 
			&& 0 != m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList.count( IBaseNotificationHandler::NH_OnCellDblClicked ) )
	{
		long lRelRow = lRow;
		m_pclCurrentDlgOutputSpreadContainer->GetRelativeRow( lRelRow );
		m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList[IBaseNotificationHandler::NH_OnCellDblClicked]->OnDlgOutputCellDblClicked( iOutputID, iColumnID, lRelRow );
	}
}

void CDlgOutput::OnOutputSpreadContainerCellEditModeOff( int iOutputID, int iColumnID, long lRow, bool bChangeMade )
{
	if( 0 != m_mapOutputContextParams.count( iOutputID ) 
			&& 0 != m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList.count( IBaseNotificationHandler::NH_OnCellEditModeOff ) )
	{
		long lRelRow = lRow;
		m_pclCurrentDlgOutputSpreadContainer->GetRelativeRow( lRelRow );
		m_mapOutputContextParams[iOutputID].m_mapBaseNotificationHandlerList[IBaseNotificationHandler::NH_OnCellEditModeOff]->OnDlgOutputCellEditModeOff( iOutputID, iColumnID, lRelRow, bChangeMade );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgOutput, CDialogEx )
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CDlgOutput::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

BOOL CDlgOutput::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetDlgCtrlID( m_iOutputID );

	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_DLGOUTPUT_CAPTION );
	SetWindowText( str );

	// Create tooltips for top buttons.
	m_ToolTip.Create( this, TTS_NOPREFIX );

	m_pclCurrentDlgOutputSpreadContainer = NULL;
	if( true == m_bCreateOutput )
	{
		if( false == AddOutput( m_iOutputID ) )
		{
			return FALSE;
		}

		m_pclCurrentDlgOutputSpreadContainer = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer;
	}

	// Retrieve size of the header.
	CRect rectClient;
	GetClientRect( &rectClient );
	CRect rectSpread;
	GetDlgItem( IDC_SPREADCONTAINER )->GetClientRect( &rectSpread );
	GetDlgItem( IDC_SPREADCONTAINER )->ClientToScreen( &rectSpread );
	ScreenToClient( &rectSpread );
	m_rectHeader.SubtractRect( &rectClient, &rectSpread );

	return TRUE;
}

void CDlgOutput::OnSize( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize( nType, cx, cy );

	if( 0 != m_mapOutputContextParams.size() )
	{
		for( iterOutputContext iter = m_mapOutputContextParams.begin(); iter != m_mapOutputContextParams.end(); ++iter )
		{
			if( NULL == iter->second.m_pclDlgOutputSpreadContainer )
			{
				continue;
			}

			CRect rectSpredContainer;
			iter->second.m_pclDlgOutputSpreadContainer->GetWindowRect( &rectSpredContainer );
			ScreenToClient( &rectSpredContainer );
			iter->second.m_pclDlgOutputSpreadContainer->SetWindowPos( NULL, -1, -1, cx, max( 0, cy - rectSpredContainer.top ), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
			m_rectHeader.right = m_rectHeader.left + cx;
		}

		// Move header buttons that are right aligned.
		if( cx != 0 && m_vecHeaderButtonRightList.size() > 0 )
		{
			CRect rectCurrentButton;
			CRect rectPrevButton;
			
			for( int iLoop = 0; iLoop < (int)m_vecHeaderButtonRightList.size(); iLoop++ )
			{
				CWnd* pWnd = m_vecHeaderButtonRightList[iLoop]->GetpWnd();
				pWnd->GetClientRect( &rectCurrentButton );
				pWnd->ClientToScreen( &rectCurrentButton );
				ScreenToClient( &rectCurrentButton );
				int iPosX;

				if( 0 == iLoop )
				{
					iPosX = m_rectHeader.right - rectCurrentButton.Width() - BS_PIXELS_BUTTONS;
				}
				else
				{
					iPosX = rectPrevButton.right - rectCurrentButton.Width() - BS_PIXELS_BUTTONS;
				}
					
				pWnd->SetWindowPos( NULL, iPosX, rectCurrentButton.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
				rectPrevButton = rectCurrentButton;
			}
		}
	}
}

void CDlgOutput::OnPaint()
{
	CPaintDC dc( this ); // device context for painting

	CRect rectClient;
	GetClientRect( &rectClient );

	// Create separator pen.
	CPen clPen;
	COLORREF refPenColor;
	refPenColor = DO_COLOR_BUTTON_SEPARATOR;
	clPen.CreatePen( PS_SOLID, 1, refPenColor );
	CPen* pOldPen = dc.SelectObject( &clPen );

	// If there is more than one left button...
	if( m_vecHeaderButtonLeftList.size() > 1 )
	{
		for( int iLoop = 0; iLoop < (int)m_vecHeaderButtonLeftList.size() - 1; iLoop++ )
		{
			if( NULL == m_vecHeaderButtonLeftList[iLoop] || NULL == m_vecHeaderButtonLeftList[iLoop]->GetpWnd() )
			{
				continue;
			}

			CWnd* pButton = m_vecHeaderButtonLeftList[iLoop]->GetpWnd();
			CRect rectButton;
			pButton->GetWindowRect( &rectButton );
			ScreenToClient( &rectButton );
			dc.MoveTo( rectButton.right + 3, rectButton.top + 2 );
			dc.LineTo( rectButton.right + 3, rectButton.bottom - 2 );
		}
	}

	// If there is more than one right button...
	if( m_vecHeaderButtonRightList.size() > 1 )
	{
		for( int iLoop = 0; iLoop < (int)m_vecHeaderButtonRightList.size() - 1; iLoop++ )
		{
			if( NULL == m_vecHeaderButtonRightList[iLoop] || NULL == m_vecHeaderButtonRightList[iLoop]->GetpWnd() )
			{
				continue;
			}

			CWnd* pButton = m_vecHeaderButtonRightList[iLoop]->GetpWnd();
			CRect rectButton;
			pButton->GetWindowRect( &rectButton );
			ScreenToClient( &rectButton );
			dc.MoveTo( rectButton.left - 3, rectButton.top + 2 );
			dc.LineTo( rectButton.left - 3, rectButton.bottom - 2 );
		}
	}

	dc.SelectObject( pOldPen );
}

void CDlgOutput::OnCancel()
{
	// MSDN:
	// If you implement the Cancel button in a modeless dialog box, you must override the OnCancel member function and 
	// call DestroyWindow from within it. Don't call the base-class member function, because it calls EndDialog, which 
	// will make the dialog box invisible but not destroy it.

	// http://www.flounder.com/kb103788.htm
	// PS: and an <esc> key can generate a WM_COMMAND:IDCANCEL message (which will invoke the OnCancel virtual method)
}

void CDlgOutput::OnOK()
{
	// MSDN:
	// If you implement the Cancel button in a modeless dialog box, you must override the OnCancel member function and 
	// call DestroyWindow from within it. Don't call the base-class member function, because it calls EndDialog, which 
	// will make the dialog box invisible but not destroy it.

	// http://www.flounder.com/kb103788.htm
	// PS: an <enter> key can generate a WM_COMMAND:IDOK message (which will invoke the OnOK virtual method).
}

/*
void CDlgOutput::OnBnClickedCopy()
{
	if( FALSE == ::OpenClipboard( this->GetSafeHwnd() ) )
	{
		// TODO: message
		return;
	}
	EmptyClipboard();

	CString strAllItems, strOneItem;
	strAllItems = _T("");
	for( int iLoopItem = 0; iLoopItem < m_OutputList.GetCount(); iLoopItem++ )
	{
		int iTextLength = m_OutputList.GetTextLen( iLoopItem );
		m_OutputList.GetText( iLoopItem, strOneItem.GetBuffer( iTextLength ) );
		strOneItem.ReleaseBuffer();
		strAllItems += strOneItem + _T("\n");
	}
	
	HGLOBAL hCopy = GlobalAlloc( GMEM_MOVEABLE, ( strAllItems.GetLength() + 1 ) * sizeof( TCHAR ) ); 
	if( NULL == hCopy )
	{ 
		CloseClipboard(); 
		// TODO: message
		return; 
	} 

	// Lock the handle and copy the text to the buffer. 
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock( hCopy ); 
	memcpy( lptstrCopy, strAllItems, strAllItems.GetLength() * sizeof( TCHAR ) );
	lptstrCopy[strAllItems.GetLength()] = (TCHAR)0;
	GlobalUnlock( hCopy ); 
 
	// Place the handle on the clipboard. 
	SetClipboardData( CF_UNICODETEXT, hCopy ); 

	CloseClipboard();
}
*/

bool CDlgOutput::AddOutput( int iOutputID )
{
	// If already exist, do nothing.
	if( 0 != m_mapOutputContextParams.count( iOutputID) )
	{
		return false;
	}

	CDlgOutputSpreadContainer *pclCurrentDlgOutputSpreadContainer = NULL;

	if( true == m_bUseSpreadContainer )
	{
		// Create instance of a dialog spread container.
		pclCurrentDlgOutputSpreadContainer = new CDlgOutputSpreadContainer( iOutputID );
		
		if( NULL == pclCurrentDlgOutputSpreadContainer )
		{
			return false;
		}

		// Create dialog.
		if( FALSE == pclCurrentDlgOutputSpreadContainer->Create( CDlgOutputSpreadContainer::IDD, this ) )
		{
			return false;
		}
	
		// Size this dialog to group-box control IDC_SPREADCONTAINER.
		CRect rectGroup;
		GetDlgItem( IDC_SPREADCONTAINER )->GetWindowRect( &rectGroup );
		ScreenToClient( &rectGroup );
		pclCurrentDlgOutputSpreadContainer->SetWindowPos( NULL, rectGroup.left, rectGroup.top, rectGroup.Width(), rectGroup.Height(), SWP_NOACTIVATE | SWP_NOZORDER );

		pclCurrentDlgOutputSpreadContainer->RegisterNotificationHandler( this, CDlgOutputSpreadContainer::INotificationHandler::NH_All );

		m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer = pclCurrentDlgOutputSpreadContainer;
	}

	// Add button state.
	for( std::vector<CVButton*>::iterator iter = m_vecHeaderButtonLeftList.begin(); iter != m_vecHeaderButtonLeftList.end(); ++iter )
	{
		m_mapOutputContextParams[iOutputID].m_mapTopPanelButtonState[*iter] = (*iter)->IsEnable();
	}

	OnDlgOutputAdded( iOutputID, pclCurrentDlgOutputSpreadContainer );

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private members
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDlgOutput::_AddTopPanelButton( CVButton::Type eButtonType, int iButtonID, CVButton::State eInitialState, CString strCaption, 
		int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton )
{
	if( true == strCaption.IsEmpty() && -1 == iImageID )
	{
		return false;
	}

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER;

	// Retrieve size.
	CRect rectLastLeftButton( 0, 0, 0, 0 );
	
	if( m_vecHeaderButtonLeftList.size() > 0 )
	{
		CWnd* pWnd = m_vecHeaderButtonLeftList.back()->GetpWnd();
		pWnd->GetClientRect( &rectLastLeftButton );
		pWnd->ClientToScreen( &rectLastLeftButton );
		ScreenToClient( &rectLastLeftButton );
	}

	CRect rectLastRightButton( 0, 0, 0, 0 );
	
	if( m_vecHeaderButtonRightList.size() > 0 )
	{
		CWnd* pWnd = m_vecHeaderButtonRightList.back()->GetpWnd();
		pWnd->GetClientRect( &rectLastRightButton );
		pWnd->ClientToScreen( &rectLastRightButton );
		ScreenToClient( &rectLastRightButton );
	}

	// Retrieve space available for this new button.
	int iAvailWidth = m_rectHeader.right - ( BS_PIXELS_EDGEHEADER_BUTTON * 2 );
	
	if( FALSE == rectLastLeftButton.IsRectNull() )
	{
		iAvailWidth -= ( rectLastLeftButton.right + BS_PIXELS_BUTTONS );
	}
	
	if( FALSE == rectLastRightButton.IsRectNull() )
	{
		iAvailWidth -= ( rectLastRightButton.left + BS_PIXELS_BUTTONS );
	}

	int iAvailHeight = m_rectHeader.Height() - ( BS_PIXELS_EDGEHEADER_BUTTON * 2 );

	int iButtonWidth = 0;
	int iButtonHeight = 0;
	
	if( sizeButton != CSize( 0 ) )
	{
		if( sizeButton.cx > iAvailWidth )
		{
			return false;
		}

		if( sizeButton.cy > iAvailHeight )
		{
			return false;
		}

		iButtonWidth = sizeButton.cx;
		iButtonHeight = sizeButton.cy;
	}

	// Create the button.
	CVButton *pclButton = NULL;
	
	switch( eButtonType )
	{
		case CVButton::Type::Normal:
			pclButton = new CButtonNormal( this );
			break;
		
		case CVButton::Type::Special:
			pclButton = new CButtonSpecial( eInitialState, this );
			break;
	}
	
	if( NULL == pclButton )
	{
		return false;
	}

	int iTextWidth = 0;
	int iTextHeight = 0;
	
	if( false == strCaption.IsEmpty() )
	{
		// If size of the button is not defined...
		if( 0 == iButtonWidth && 0 == iButtonHeight )
		{
			if( false == strCaption.IsEmpty() )
			{
				// Text is defined, we compute the space needed for this text.

				// To have the good font, we temporary create it and retrieve its device context.
				CSize sizeText( 0, 0 );
				
				if(	CVButton::Type::Normal == eButtonType )
				{
					CMFCButton clTempButton;
					
					if( TRUE == clTempButton.Create( _T(""), dwStyle | BS_TEXT, CRect( 0, 0, 0, 0 ), this, 0 ) )
					{
						CDC *pDC = clTempButton.GetDC();
						
						if( pDC != NULL )
						{
							sizeText = pDC->GetTextExtent( strCaption );
						}

						clTempButton.DestroyWindow();
					}
				}
				else if( CVButton::Type::Special == eButtonType )
				{
					CDC *pDC = GetDC();
					CFont clFont;
					clFont.CreateFont( -11, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, _T("MS Shell Dlg") );
					CFont *pOldFont = pDC->SelectObject( &clFont );
					sizeText = pDC->GetTextExtent( strCaption );
					pDC->SelectObject( pOldFont );
					ReleaseDC( pDC );
				}
				
				// If text is too long or to high.
				if( sizeText.cx > iAvailWidth - BS_PIXELS_LEFTEDGE_TEXT - BS_PIXELS_RIGHTEDGE_TEXT )
				{
					return false;
				}

				if( sizeText.cy > iAvailHeight - BS_PIXELS_TOPBOTTOMEDGE_TEXT )
				{
					return false;
				}

				iTextWidth = sizeText.cx;
				iTextHeight = sizeText.cy;
				dwStyle |= BS_TEXT;
			}
			else
			{
				// Text is empty, we take default values.
				iTextWidth = 70;
				iTextHeight = 20;
			}
		}
	}
	
	int iImageWidth = 0;
	int iImageHeight = 0;
	CBitmap clBitmap;
	HANDLE hIcon = NULL;
	
	if( iImageID != -1 && 0 == iButtonWidth && 0 == iButtonHeight )
	{
		CSize sizeImage( 0, 0 );
		
		if( BS_BITMAP == iImageType )
		{
			// If bitmap is not empty...
			if( clBitmap.LoadBitmap( iImageID ) != 0 )
			{
				BITMAP rBitmap;
				clBitmap.GetBitmap( &rBitmap );
				sizeImage = CSize( rBitmap.bmWidth, rBitmap.bmHeight );
				clBitmap.SetBitmapDimension( sizeImage.cx, sizeImage.cy );
			}
		}
		else if( BS_ICON == iImageType )
		{
			HICON hIcon = TASApp.LoadIcon( iImageID );
			
			if( hIcon != NULL )
			{
				sizeImage = CSize( 16, 16 );
			}
		}

		if( sizeImage != CSize( 0, 0 ) )
		{
			// If the image width is bigger than the available space...
			int iAvailImageWidth = iAvailWidth - 2 * BS_PIXELS_EDGE_IMAGE - iTextWidth - BS_PIXELS_RIGHTEDGE_TEXT;
			
			if( sizeImage.cx > iAvailImageWidth )
			{
				// If the bitmap height is bigger than the available height. And if we can proportionally reduce the bitmap size to match its height
				// with the available height and its width becomes lower than the available space...
				if( BS_BITMAP == iImageType && sizeImage.cy > iAvailHeight && ( sizeImage.cx / sizeImage.cy * iAvailHeight <= iAvailImageWidth ) )
				{
					double dRatio = (double)sizeImage.cy / (double)iAvailHeight;
					iImageWidth = (int)( (double)sizeImage.cx / dRatio );
					clBitmap.SetBitmapDimension( iImageWidth, iAvailHeight );
					dwStyle |= BS_BITMAP;
				}
			}
			else
			{
				iImageWidth = sizeImage.cx;
				dwStyle |= iImageType;
			}

			// If the bitmap height is bigger than the available space...
			if( sizeImage.cy > iAvailHeight )
			{
				// If the bitmap width is bigger than the available width. And if we can proportionally reduce the bitmap size to match its width
				// with the available width and its height becomes lower than the available space...
				if( BS_BITMAP == iImageType && sizeImage.cx > iAvailWidth && ( sizeImage.cy / sizeImage.cx * iAvailWidth <= iAvailHeight ) )
				{
					double dRatio = (double)sizeImage.cx / (double)iAvailHeight;
					iImageHeight = (int)( (double)sizeImage.cy / dRatio );
					clBitmap.SetBitmapDimension( iImageWidth, iImageHeight );
					dwStyle |= BS_BITMAP;
				}
			}
			else
			{
				iImageHeight = sizeImage.cy;
				dwStyle |= iImageType;
			}
		}
	}

	int iFinalWidth;

	if( iButtonWidth != 0 )
	{
		iFinalWidth = iButtonWidth;
	}
	else
	{
		iFinalWidth = BS_PIXELS_EDGE_IMAGE + iImageWidth + BS_PIXELS_EDGE_IMAGE + iTextWidth + BS_PIXELS_RIGHTEDGE_TEXT;
	}
	
	int iFinalHeight;

	if( iButtonHeight != 0 ) 
	{
		iFinalHeight = iButtonHeight;
	}
	else
	{
		int iImageHeightNeeded = iImageHeight + ( 2 * BS_PIXELS_EDGE_IMAGE );
		int iTextHeightNeeded = iTextHeight + ( 2 * BS_PIXELS_TOPBOTTOMEDGE_TEXT );
		iFinalHeight = max( iImageHeightNeeded, iTextHeightNeeded );
	}
	
	BOOL iReturn;
	
	switch( eButtonType )
	{
		case CVButton::Type::Normal:
			iReturn = ( (CButtonNormal*)pclButton )->Create( strCaption, dwStyle, CRect( 0, 0, iFinalWidth, iFinalHeight ), this, iButtonID );
			
			if( BS_BITMAP == ( dwStyle & BS_BITMAP ) )
			{
				( (CButtonNormal*)pclButton )->SetImage( (HBITMAP)clBitmap );
			}

			break;

		case CVButton::Type::Special:
			{
				LONG lBaseUnits = ::GetDialogBaseUnits();
				WORD* pTemplate = &m_arwTemplate[0];
				DLGTEMPLATE& rDlgTemplate = *((DLGTEMPLATE*)pTemplate);
				rDlgTemplate.style = ( DS_SETFONT | DS_FIXEDSYS | WS_CHILD ) & ~WS_BORDER;
				rDlgTemplate.dwExtendedStyle = 0;
				rDlgTemplate.x = 0;
				rDlgTemplate.y = 0;
				rDlgTemplate.cx = MulDiv( iFinalWidth, 4, LOWORD( lBaseUnits ) );
				rDlgTemplate.cy = MulDiv( iFinalHeight, 8, HIWORD( lBaseUnits ) );
				rDlgTemplate.cdit = 0;
				pTemplate += ( sizeof(DLGTEMPLATE) >> 1 );
				*pTemplate++ = 0x0000; // menu (none)
				*pTemplate++ = 0x0000; // dialog class (use standard)
				*pTemplate++ = 0x0000; // no caption
				iReturn = ( (CButtonSpecial*)pclButton )->CreateIndirect( &rDlgTemplate, this );
				( (CButtonSpecial*)pclButton )->SetDlgCtrlID( iButtonID );
				( (CButtonSpecial*)pclButton )->SetText( strCaption );
				
				if( BS_BITMAP == ( dwStyle & BS_BITMAP ) )
				{
					( (CButtonSpecial*)pclButton )->SetBitmap( iImageID );
				}
				else if( BS_ICON == ( dwStyle & BS_ICON ) )
				{
					( (CButtonSpecial*)pclButton )->SetIcon( iImageID );
				}
			}
			break;
	}

	if( FALSE == iReturn )
	{
		delete pclButton;
		return false;
	}

	// Set the position.
	int iPosY = ( m_rectHeader.Height() - iFinalHeight ) / 2;
	int iPosX;
	
	if( true == bLeftAlign )
	{
		iPosX = ( FALSE == rectLastLeftButton.IsRectNull() ) ? rectLastLeftButton.right + BS_PIXELS_BUTTONS : BS_PIXELS_EDGEHEADER_BUTTON;
	}
	else
	{
		if( FALSE == rectLastLeftButton.IsRectNull() )
		{
			iPosX = rectLastRightButton.right - iFinalWidth - BS_PIXELS_BUTTONS;
		}
		else
		{
			iPosX = m_rectHeader.right - iFinalWidth - BS_PIXELS_EDGEHEADER_BUTTON;
		}
	}

	pclButton->GetpWnd()->SetWindowPos( NULL, iPosX, iPosY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	pclButton->GetpWnd()->ShowWindow( SW_SHOW );
	
	if( true == bLeftAlign )
	{
		m_vecHeaderButtonLeftList.push_back( pclButton );
	}
	else
	{
		m_vecHeaderButtonRightList.push_back( pclButton );
	}

	// Add this button.
	for( iterOutputContext iter = m_mapOutputContextParams.begin(); iter != m_mapOutputContextParams.end(); ++iter )
	{
		iter->second.m_mapTopPanelButtonState[pclButton] = true;
	}

	return true;
}

void CDlgOutput::_ClearAllTopPanelButtons( void )
{
	if( m_vecHeaderButtonLeftList.size() > 0 )
	{
		for( int iLoop = 0; iLoop < (int)m_vecHeaderButtonLeftList.size(); iLoop++ )
		{
			if( m_vecHeaderButtonLeftList[iLoop] != NULL )
			{
				if( m_vecHeaderButtonLeftList[iLoop]->GetpWnd()->GetSafeHwnd() != NULL )
				{
					m_vecHeaderButtonLeftList[iLoop]->GetpWnd()->DestroyWindow();
				}

				delete m_vecHeaderButtonLeftList[iLoop];
			}
		}
		
		m_vecHeaderButtonLeftList.clear();

		for( int iLoop = 0; iLoop < (int)m_vecHeaderButtonRightList.size(); iLoop++ )
		{
			if( m_vecHeaderButtonRightList[iLoop] != NULL )
			{
				if( m_vecHeaderButtonRightList[iLoop]->GetpWnd()->GetSafeHwnd() != NULL )
				{
					m_vecHeaderButtonRightList[iLoop]->GetpWnd()->DestroyWindow();
				}

				delete m_vecHeaderButtonRightList[iLoop];
			}
		}
		
		m_vecHeaderButtonRightList.clear();
	}

	// Remove all buttons.
	for( iterOutputContext iter = m_mapOutputContextParams.begin(); iter != m_mapOutputContextParams.end(); ++iter )
	{
		iter->second.m_mapTopPanelButtonState.clear();
	}
}

CDlgOutput::CVButton *CDlgOutput::_GetTopPanelButton( int iButtonID )
{
	CVButton *pButton = NULL;
	
	for( int iLoop = 0; iLoop < (int)m_vecHeaderButtonLeftList.size(); iLoop++ )
	{
		if( iButtonID == m_vecHeaderButtonLeftList[iLoop]->GetID() )
		{
			pButton = m_vecHeaderButtonLeftList[iLoop];
			break;
		}
	}
	
	if( NULL != pButton )
	{
		return pButton;
	}
	
	for( int iLoop = 0; iLoop < (int)m_vecHeaderButtonRightList.size(); iLoop++ )
	{
		if( iButtonID == m_vecHeaderButtonRightList[iLoop]->GetID() )
		{
			pButton = m_vecHeaderButtonRightList[iLoop];
			break;
		}
	}

	return pButton;
}
