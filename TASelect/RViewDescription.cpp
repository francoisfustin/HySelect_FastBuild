#include "stdafx.h"


#include "TASelect.h"
#include "SSheet.h"
#include "RViewDescription.h"

//////////////////////////////////////////////////////////////////////
// Class CCellDescription
//////////////////////////////////////////////////////////////////////

// Remark: needed to be written here because forward declaration (class CSheetDescription) doesn't work
//         with code in .h file.
CSSheet *CCellDescription::GetpSSheet( void ) { return m_pclSheetDescription->GetSSheetPointer(); }

//////////////////////////////////////////////////////////////////////
// Class CResizingColumnInfo
//////////////////////////////////////////////////////////////////////

CResizingColumnInfo::CResizingColumnInfo()
{
	m_pclSheetDescription = NULL;
	Reset();
}

CResizingColumnInfo::~CResizingColumnInfo()
{
	Reset();
}

void CResizingColumnInfo::Reset( void )
{
	m_fAutoColSeparator = true;
	m_lRowTopPixel = 0;
	m_lRowBottomPixel = 0;
	m_lRowTopLogical = 0;
	m_lRowBottomLogical = 0;
	m_lDblClickFromRow = 0;
	m_lDblClickToRow = 0;
	m_fActivated = false;
	m_fDlbClkActivated = false;
	m_nDetectionRange = 3;
	m_nMovePrecision = 2;
	m_nMinimalWidth = 20;
	m_fColMaxWidthIncludeTitle = true;
	m_fCursorDisplayed = false;
	m_hDefaultCursor = (HCURSOR)INVALID_HANDLE_VALUE;
	m_pclColFocused = NULL;
	
	_ClearMap();
	m_vecColumnList.clear();

	if( m_hMemDC != INVALID_HANDLE_VALUE )
	{
		if( m_hOldBitmap != INVALID_HANDLE_VALUE )
		{
			SelectObject( m_hMemDC, m_hOldBitmap );
		}

		if( m_hMemBitmap != INVALID_HANDLE_VALUE )
		{
			DeleteObject( m_hMemBitmap );
		}

		if( m_hBrush != INVALID_HANDLE_VALUE )
		{
			DeleteObject( m_hBrush );
		}

		DeleteDC( m_hMemDC );
	}
	
	m_hMemDC = (HDC)INVALID_HANDLE_VALUE;
	m_hMemBitmap = (HBITMAP)INVALID_HANDLE_VALUE;
	m_hOldBitmap = (HBITMAP)INVALID_HANDLE_VALUE;
	m_hBrush = (HBRUSH)INVALID_HANDLE_VALUE;
	m_lSaveHeight = -1;

	if( NULL != m_pclSheetDescription && NULL != m_pclSheetDescription->GetSSheetPointer() )
	{
		m_pclSheetDescription->GetSSheetPointer()->MM_UnregisterAllNotificationHandlers( m_pclSheetDescription->GetSSheetPointer() );
	}
}

void CResizingColumnInfo::RegisterNotificationHandler( CResizingColumnInfo::INotificationHandler *pclHandler )
{
	if( m_vecNotificationHandlerList.end() == std::find( m_vecNotificationHandlerList.begin(), m_vecNotificationHandlerList.end(), pclHandler ) )
	{
		m_vecNotificationHandlerList.push_back( pclHandler );
	}
}
	
void CResizingColumnInfo::UnregisterNotificationHandler( CResizingColumnInfo::INotificationHandler *pclHandler )
{
	std::vector<INotificationHandler *>::iterator iter = std::find( m_vecNotificationHandlerList.begin(), m_vecNotificationHandlerList.end(), pclHandler );

	if( iter != m_vecNotificationHandlerList.end() )
	{
		m_vecNotificationHandlerList.erase( iter );
	}
}

void CResizingColumnInfo::SetSheetDescription( CSheetDescription *pclSheetDescription )
{ 
	m_pclSheetDescription = pclSheetDescription; 
}

bool CResizingColumnInfo::AddRangeColumn( long lFromColumn, long lToColumn, long lCursorFromRow, long lCursorToRow, bool fUpdateRowRange )
{
	if( NULL == m_pclSheetDescription || NULL == m_pclSheetDescription->GetSSheetPointer() )
		return false;

	if( lCursorToRow == -1 )
		lCursorToRow = lCursorFromRow;
	if( lFromColumn <= 0 || lToColumn <= 0 || lCursorFromRow <= 0 || lCursorToRow <= 0 )
		return false;

	if( lFromColumn > lToColumn )
	{
		long lTemp = lFromColumn;
		lFromColumn = lToColumn;
		lToColumn = lTemp;
	}

	if( lCursorFromRow > lCursorToRow )
	{
		long lTemp = lCursorFromRow;
		lCursorFromRow = lCursorToRow;
		lCursorToRow = lTemp;
	}

	// Check first if range is not out.
	if( lCursorToRow > m_pclSheetDescription->GetSSheetPointer()->GetMaxRows() || 
		( m_lRowTopLogical > 0 && m_lRowTopLogical != lCursorFromRow ) || ( m_lRowBottomLogical > 0 && m_lRowBottomLogical != lCursorToRow ) )
		return false;

	for( long lLoopColumn = lFromColumn; lLoopColumn <= lToColumn; lLoopColumn++ )
	{
		// First of all, if column is visible or not, we save it. In case of HMCalc (Measurement sheet for example) at initialization we show 
		// all columns in a collapse mode. That means some columns can be hidden. If user clicks on any expand button to show these hidden columns, we 
		// must be able to rescan all columns submitted to 'AddRangeColumn'.
		m_vecColumnList.push_back( lLoopColumn );

		// Check first if current column is visible or not.
		if( TRUE == m_pclSheetDescription->GetSSheetPointer()->IsColHidden( lLoopColumn ) )
			continue;

		// Retrieve cell coordinates in pixel.
		CRect rectPixel = m_pclSheetDescription->GetSSheetPointer()->GetCellCoordInPixel( lLoopColumn, lCursorFromRow );
		long lStartEntry = max( 0, rectPixel.right - m_nDetectionRange );
		long lEndEntry = rectPixel.right + m_nDetectionRange;

		ColInfo *pclColInfo = new ColInfo();
		for( long lLoopEntry = lStartEntry; lLoopEntry <= lEndEntry; lLoopEntry++ )
		{
			if( 0 == m_mapColInfos.count( lLoopEntry ) )
			{
				m_mapColInfos[lLoopEntry] = pclColInfo;
			}
			else
			{
				// A stop hereo ften means that you have a column defined, not hidden but with a width of 0 pixel !!
				ASSERT( 0 );
			}
		}

		// Add separator.
		if( true == m_fAutoColSeparator )
			m_pclSheetDescription->GetSSheetPointer()->SetCellBorder( lLoopColumn, lCursorFromRow, lLoopColumn, lCursorToRow, true, SS_BORDERTYPE_RIGHT, 1, RGB( 192, 192, 192 ) );

		pclColInfo->m_lID = lLoopColumn;
		pclColInfo->m_lLeftCoord = rectPixel.left;
	}

	if( true == fUpdateRowRange )
	{
		CRect rectPixel = m_pclSheetDescription->GetSSheetPointer()->GetCellCoordInPixel( lFromColumn, lCursorFromRow );
		if( lCursorToRow > lCursorFromRow )
		{
			CRect rectTemp = m_pclSheetDescription->GetSSheetPointer()->GetCellCoordInPixel( lFromColumn, lCursorToRow );
			rectPixel.bottom = rectTemp.bottom;
		}
		m_lRowTopPixel = rectPixel.top;
		m_lRowBottomPixel = rectPixel.bottom;

		m_lRowTopLogical = lCursorFromRow;
		m_lRowBottomLogical = lCursorToRow;
	}

	return true;
}

bool CResizingColumnInfo::RemoveAllColumn( void )
{
	m_lRowTopPixel = 0;
	m_lRowBottomPixel = 0;
	m_lRowTopLogical = 0;
	m_lRowBottomLogical = 0;
	m_fCursorDisplayed = false;
	m_pclColFocused = NULL;
	
	_ClearMap();
	m_vecColumnList.clear();
	m_lSaveHeight = -1;
	return true;
}

void CResizingColumnInfo::ActivateFeature( bool fActivate, bool fExcludeDblClick, long lDblClkFromRow, long lDblClkToRow, bool fAutoColSeparator )
{
	if( NULL == m_pclSheetDescription || NULL == m_pclSheetDescription->GetSSheetPointer() )
		return;

	m_fActivated = fActivate;
	m_fDlbClkActivated = !fExcludeDblClick;
	m_lDblClickFromRow = lDblClkFromRow;
	m_lDblClickToRow = lDblClkToRow;
	m_fAutoColSeparator = fAutoColSeparator;
}

void CResizingColumnInfo::OnMouseEvent( int iMouseEventType, long lColumn, long lRow, CPoint ptMouse )
{
	if( false == m_fActivated )
		return;

	switch( iMouseEventType )
	{
		case CMessageManager::SSheetMET::MET_MouseMove:
			_OnMouseMove( lColumn, lRow, ptMouse );
			break;

		case CMessageManager::SSheetMET::MET_LButtonDown:
			_OnLButtonDown();
			break;

		case CMessageManager::SSheetMET::MET_LButtonDblClk:
			if( true == m_fDlbClkActivated )
				_OnLButtonDblClk( lColumn, lRow, ptMouse );
			break;

		case CMessageManager::SSheetMET::MET_LButtonUp:
			_OnLButtonUp();
			break;

		case CMessageManager::SSheetMET::MET_MouseCaptured:
			if( m_pclColFocused != NULL && true == m_pclColFocused->m_fIsResizing )
				_StopResizing();
			break;
	}
}

bool CResizingColumnInfo::_IsPointerOverSeparator( CPoint ptPointer, long lXOffset )
{
	bool fReturn = false;
	if( ptPointer.y >= m_lRowTopPixel && ptPointer.y <= m_lRowBottomPixel )
	{
		if( m_mapColInfos.count( ptPointer.x + lXOffset ) > 0 )
			fReturn = true;
	}
	return fReturn;
}

void CResizingColumnInfo::_StartMoving( long lPosX, long lXOffset )
{
	m_hDefaultCursor = m_pclSheetDescription->GetSSheetPointer()->GetCursor( SS_CURSORTYPE_DEFAULT );
	m_pclSheetDescription->GetSSheetPointer()->SetCursor( SS_CURSORTYPE_DEFAULT, m_pclSheetDescription->GetSSheetPointer()->GetCursor( SS_CURSORTYPE_COLRESIZE ) );
	m_fCursorDisplayed = true;
	m_pclColFocused = m_mapColInfos[lPosX + lXOffset];
	m_pclColFocused->m_lCurrentPos = lPosX + lXOffset;
	m_pclColFocused->m_lScrollX = lXOffset;
}

void CResizingColumnInfo::_StopMoving()
{
	m_pclSheetDescription->GetSSheetPointer()->SetCursor( SS_CURSORTYPE_DEFAULT, m_hDefaultCursor );
	m_hDefaultCursor = (HCURSOR)INVALID_HANDLE_VALUE;
	m_fCursorDisplayed = false;
	m_pclColFocused->m_lPrevPos = 0;
	m_pclColFocused->m_lCurrentPos = 0;
	m_pclColFocused = NULL;
}

void CResizingColumnInfo::_StartResizing()
{
	// 'SetCapture' allow to force mouse messages to be sent only to TSpread even if pointer goes out of client area.
	::SetCapture( m_pclSheetDescription->GetSSheetPointer()->GetSafeHwnd() );

	// Set variables.
	m_pclColFocused->m_fIsResizing = true;
	m_pclColFocused->m_lPrevPos = 0;
	_DrawLine();
}

void CResizingColumnInfo::_StopResizing()
{
	// Set variables.
	m_pclColFocused->m_fIsResizing = false;

	// Restore line.
	_RestoreLine();

	// Resize column.
	m_pclSheetDescription->GetSSheetPointer()->SetColWidthInPixels( m_pclColFocused->m_lID, m_pclColFocused->m_lCurrentPos - m_pclColFocused->m_lLeftCoord + 1 );

	// Clean all flags.
	// Remark: save column ID because '_StopMoving' clears 'm_pclColFocused'.
	long lColumnID = m_pclColFocused->m_lID;
	_StopMoving();

	// Recompute map in regards to the new sizes.
	_RecomputeMap();

	// Send message to prevent the change.
	UINT uiSheetDescriptionID = m_pclSheetDescription->GetSheetDescriptionID();
	AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_SSHEETCOLWIDTHCHANGE, (WPARAM)uiSheetDescriptionID, (LPARAM)lColumnID );

	for( auto &iter : m_vecNotificationHandlerList )
	{
		iter->ResizingColumnInfoNH_OnSSheetColumnWidthChanged( uiSheetDescriptionID, lColumnID );
	}
}

void CResizingColumnInfo::_CancelResizing( void )
{
	// Set variables.
	m_pclColFocused->m_fIsResizing = false;
	_RestoreLine();
	_StopMoving();
}

void CResizingColumnInfo::_ClearMap( void )
{
	if( m_mapColInfos.size() > 0 )
	{
		ColInfo* prPrevColInfo = NULL;
		for( mapLongColInfosIter iter = m_mapColInfos.begin(); iter != m_mapColInfos.end(); ++iter )
		{
			if( prPrevColInfo != NULL && iter->second != prPrevColInfo )
				delete prPrevColInfo;
			prPrevColInfo = iter->second;
		}
		if( prPrevColInfo != NULL )
			delete prPrevColInfo;
		m_mapColInfos.clear();
	}
}

void CResizingColumnInfo::_RecomputeMap()
{
	if( m_mapColInfos.size() > 0 )
	{
		// Clear all previous value.
		_ClearMap();

		// Recompute
		vecLong vecTemp = m_vecColumnList;
		m_vecColumnList.clear();
		for( long lLoop = 0; lLoop < (long)vecTemp.size(); lLoop++ )
			AddRangeColumn( vecTemp[lLoop], vecTemp[lLoop], m_lRowTopLogical, m_lRowBottomLogical, false );
	}
}

void CResizingColumnInfo::_DrawLine()
{
	CDC *pDC = m_pclSheetDescription->GetSSheetPointer()->GetDC();

	if( -1 == m_lSaveHeight )
	{
		CRect rectSpread = m_pclSheetDescription->GetSSheetPointer()->GetSheetSizeInPixels();
		m_lSaveHeight = rectSpread.bottom - m_lRowTopPixel;
	}

	// If memory context not yet created.
	if( INVALID_HANDLE_VALUE == m_hMemDC )
	{
		m_hMemDC = ::CreateCompatibleDC( ::GetDC( NULL ) );
		m_hMemBitmap = ::CreateCompatibleBitmap( pDC->GetSafeHdc(), 1, m_lSaveHeight );
		m_hOldBitmap = (HBITMAP)::SelectObject( m_hMemDC, m_hMemBitmap );
	}

	// If brush not yet created...
	if( INVALID_HANDLE_VALUE == m_hBrush )
		m_hBrush = ::CreateSolidBrush( RGB( 192, 192, 192 ) );

	HBRUSH hOldBrush = (HBRUSH)::SelectObject( pDC->GetSafeHdc(), m_hBrush );

	// Clear previous line if exist.
	_RestoreLine();

	// Save current line.
	CDC *pMemDC = CDC::FromHandle( m_hMemDC );
	pMemDC->BitBlt( 0, 0, 1, m_lSaveHeight, pDC, m_pclColFocused->m_lCurrentPos - m_pclColFocused->m_lScrollX, m_lRowTopPixel, SRCCOPY );

	// Draw new line.
	pDC->MoveTo( m_pclColFocused->m_lCurrentPos - m_pclColFocused->m_lScrollX, m_lRowTopPixel );
	pDC->LineTo( m_pclColFocused->m_lCurrentPos - m_pclColFocused->m_lScrollX, m_lRowTopPixel + m_lSaveHeight );

	::SelectObject( pDC->GetSafeHdc(), hOldBrush );
	m_pclSheetDescription->GetSSheetPointer()->ReleaseDC( pDC );

	m_pclColFocused->m_lPrevPos = m_pclColFocused->m_lCurrentPos;
}

void CResizingColumnInfo::_RestoreLine()
{
	CDC *pDC = m_pclSheetDescription->GetSSheetPointer()->GetDC();
	// If a previous line was drawn...
	if( m_pclColFocused->m_lPrevPos != 0 )
	{
		// Restore first this previous line.
		CDC *pMemDC = CDC::FromHandle( m_hMemDC );
		pDC->BitBlt( m_pclColFocused->m_lPrevPos - m_pclColFocused->m_lScrollX, m_lRowTopPixel, 1, m_lSaveHeight, pMemDC, 0, 0, SRCCOPY );
	}
	m_pclSheetDescription->GetSSheetPointer()->ReleaseDC( pDC );
}

void CResizingColumnInfo::_DumpMap( CString strFctName )
{
	CStdioFile myFile;
	myFile.Open( _T("c:\\temp\\logmap.txt"), CFile::modeCreate | CFile::modeNoTruncate | CFile::typeText | CFile::modeWrite );
	myFile.Seek( 0, CFile::end);
	myFile.WriteString( strFctName );
	myFile.WriteString( _T("\n") );
	for( mapLongColInfosIter iter = m_mapColInfos.begin(); iter != m_mapColInfos.end(); ++iter )
	{
		CString str;
		str.Format( _T("\ty: %d|ID: %d|Left: %d|IsResizing: %d|PrevPos: %d|CurPos: %d\n"), iter->first, iter->second->m_lID, iter->second->m_lLeftCoord, iter->second->m_fIsResizing, iter->second->m_lPrevPos, iter->second->m_lCurrentPos );
		myFile.WriteString( str );
	}
	myFile.WriteString( _T("\n") );
}

void CResizingColumnInfo::_OnMouseMove( long lMouseMoveCol, long lMouseMoveRow, CPoint ptMouse )
{
	// If resizing feature is activated...
	if( true == m_fActivated )
	{
		long lScrollX = 0;

		if( TRUE == m_pclSheetDescription->GetSSheetPointer()->IsHorizontalScrollBarVisible() )
		{
			SS_COORD lCol, lRow;
			m_pclSheetDescription->GetSSheetPointer()->GetTopLeftCell( &lCol, &lRow );
			CRect rect = m_pclSheetDescription->GetSSheetPointer()->GetCellCoordInPixel( lCol, lRow );

			SS_COORD lFreezeCol, lFreezeRow;
			m_pclSheetDescription->GetSSheetPointer()->GetFreeze( &lFreezeCol, &lFreezeRow );
			long lFreezeWidth = 0;

			for( long lLoopCol = 1; lLoopCol <= lFreezeCol; lLoopCol++ )
			{
				lFreezeWidth += m_pclSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( lLoopCol );
			}

			lScrollX = rect.left - lFreezeWidth;
		}

		// If resizing is currently running...
		if( m_pclColFocused != NULL && true == m_pclColFocused->m_fIsResizing )
		{
			// Do something only if there is a change...
			m_pclColFocused->m_lScrollX = lScrollX;

			if( ptMouse.x + lScrollX < max( 0, m_pclColFocused->m_lPrevPos - m_nMovePrecision ) || ptMouse.x + lScrollX > m_pclColFocused->m_lPrevPos + m_nMovePrecision )
			{
				// If pointer is well after the cell left coordinate...
				if( ptMouse.x + lScrollX >= m_pclColFocused->m_lLeftCoord + m_nMinimalWidth )
				{
					m_pclColFocused->m_lCurrentPos = ptMouse.x + lScrollX;
					// Draw the line.
					_DrawLine();
				}
			}
		}
		else
		{
			bool bOver = _IsPointerOverSeparator( ptMouse, lScrollX );
			
			// If pointer is over a separator...
			if( true == bOver )
			{
				// If cursor is not yet displayed...
				if( NULL == m_pclColFocused )
				{
					_StartMoving( ptMouse.x, lScrollX );
				}
				else
				{
					m_pclColFocused->m_lCurrentPos = ptMouse.x + lScrollX;
					m_pclColFocused->m_lScrollX = lScrollX;
				}
			}
			else
			{
				// If cursor was previously displayed...
				if( m_pclColFocused != NULL )
				{
					_StopMoving();
				}
			}
		}
	}
}

void CResizingColumnInfo::_OnLButtonDown( void )
{
	if( m_pclColFocused != NULL )
	{
		_StartResizing();
	}
}

void CResizingColumnInfo::_OnLButtonDblClk( long lLButtonDlbClkCol, long lLButtonDlbClkRow, CPoint ptMouse )
{
	if( NULL == m_pclSheetDescription || NULL == m_pclSheetDescription->GetSSheetPointer() )
		return;

	if( 0 == m_lDblClickFromRow || 0 == m_lDblClickToRow )
		return;
	
	long lColumnID = -1;
	long lScrollX = 0;
	if( TRUE == m_pclSheetDescription->GetSSheetPointer()->IsHorizontalScrollBarVisible() )
	{
		SS_COORD lCol, lRow;
		m_pclSheetDescription->GetSSheetPointer()->GetTopLeftCell( &lCol, &lRow );
		CRect rect = m_pclSheetDescription->GetSSheetPointer()->GetCellCoordInPixel( lCol, lRow );

		SS_COORD lFreezeCol, lFreezeRow;
		m_pclSheetDescription->GetSSheetPointer()->GetFreeze( &lFreezeCol, &lFreezeRow );
		long lFreezeWidth = 0;
		for( long lLoopCol = 1; lLoopCol <= lFreezeCol; lLoopCol++ )
			lFreezeWidth += m_pclSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( lLoopCol );

		lScrollX = rect.left - lFreezeWidth;
	}

	// Remark: 'm_pclColFocused' different from NULL means that mouse pointer is moving over a separator.
	if( m_pclColFocused != NULL && false == m_pclColFocused->m_fIsResizing )
	{
		lColumnID = m_pclColFocused->m_lID;
		_StopMoving();
	}
	else if( NULL == m_pclColFocused )
	{
		long lColumn, lRow;
		m_pclSheetDescription->GetSSheetPointer()->GetCellFromPixel( &lColumn, &lRow, ptMouse.x + lScrollX, ptMouse.y );
		if( lRow >= m_lRowTopLogical && lRow <= m_lRowBottomLogical  )
		{
			// Retrieve cell coordinates in pixel.
			CRect rectPixel = m_pclSheetDescription->GetSSheetPointer()->GetCellCoordInPixel( lColumn, lRow );
			if( m_mapColInfos.count( rectPixel.right ) > 0 )
				lColumnID = lColumn;
		}
	}

	if( lColumnID != - 1 )
	{
		long lLarger = m_nMinimalWidth;

		// Check all rows to see what is the larger column.
		long lLoopRow = m_lDblClickFromRow;
		while( lLoopRow <= m_lDblClickToRow )
		{
			CString strText = m_pclSheetDescription->GetSSheetPointer()->GetCellText( lColumnID, lLoopRow );
			if( false == strText.IsEmpty() )
			{
				double dWidth, dHeight;
				m_pclSheetDescription->GetSSheetPointer()->GetMaxTextCellSize( lColumnID, lLoopRow, &dWidth, &dHeight );
				lLarger = max( lLarger, (long)m_pclSheetDescription->GetSSheetPointer()->ColWidthToLogUnits( dWidth ) );
			}
			lLoopRow++;
		}

		// If we must check also row where are separators...
		if( true == m_fColMaxWidthIncludeTitle )
		{
			for( lLoopRow = m_lRowTopLogical; lLoopRow <= m_lRowBottomLogical; lLoopRow++ )
			{
				CString strText = m_pclSheetDescription->GetSSheetPointer()->GetCellText( lColumnID, lLoopRow );
				if( false == strText.IsEmpty() )
				{
					double dWidth, dHeight;
					m_pclSheetDescription->GetSSheetPointer()->GetMaxTextCellSize( lColumnID, lLoopRow, &dWidth, &dHeight );
					lLarger = max( lLarger, (long)m_pclSheetDescription->GetSSheetPointer()->ColWidthToLogUnits( dWidth ) );
				}
			}
		}

		// Resize column.
		m_pclSheetDescription->GetSSheetPointer()->SetColWidthInPixels( lColumnID, lLarger );

		// Recompute map in regards to the new sizes.
		_RecomputeMap();

		// Send message to prevent the change.
		int iCtrlID = m_pclSheetDescription->GetSSheetPointer()->GetDlgCtrlID();
		m_pclSheetDescription->GetSSheetPointer()->PostMessage( WM_USER_SSHEETCOLWIDTHCHANGE, (WPARAM)iCtrlID, (LPARAM)lColumnID );

		for( auto &iter : m_vecNotificationHandlerList )
		{
			iter->ResizingColumnInfoNH_OnSSheetColumnWidthChanged( iCtrlID, lColumnID );
		}
	}
}

void CResizingColumnInfo::_OnLButtonUp( void )
{
	// Do anything here except 'ReleaseCapture'!
	if( m_pclColFocused != NULL )
		::ReleaseCapture();
}

//////////////////////////////////////////////////////////////////////
// Class CSheetDescription
//////////////////////////////////////////////////////////////////////

CSheetDescription::CSheetDescription( UINT uiSheedDescriptionID, CSSheet *pclSSheet, CPoint ptOffset, CViewDescription* pclViewDescription ) 
{ 
	_Clean();

	m_uiSheetDescriptionID = uiSheedDescriptionID;
	m_pclSSheet = pclSSheet;
	m_ptOffset = ptOffset;
	m_pclViewDescription = pclViewDescription;
	if( NULL != pclSSheet )
		m_uiSSheetID = pclSSheet->GetDlgCtrlID();
	m_clResizingColumnsInfo.SetSheetDescription( this );
}

CSheetDescription::~CSheetDescription()
{
	if( NULL != m_pclSSheet )
		m_pclSSheet->PostPoneDelete();
	m_pclSSheet = NULL;
	_Clean();
}

void CSheetDescription::Init( long lRowStart, long lRowEnd, short nRowsByItem, short nParameterMode )
{
	m_lRowStart = lRowStart;
	m_lRowEnd = lRowEnd;
	m_nRowsByItem = nRowsByItem;
	m_nParameterMode = nParameterMode;
}

void CSheetDescription::SetActiveColumn( long lColumn )
{
	m_lActiveColumn = lColumn;
}

void CSheetDescription::AddColumn( long lColumn, double dWidth )
{
	if( lColumn > m_pclSSheet->GetMaxCols() )
		m_pclSSheet->SetMaxCols( lColumn );

	m_pclSSheet->SetColWidth( lColumn, dWidth );
}

void CSheetDescription::AddColumnInPixels( long lColumn, long lWidth )
{
	if( lColumn > m_pclSSheet->GetMaxCols() )
		m_pclSSheet->SetMaxCols( lColumn );

	m_pclSSheet->SetColWidthInPixels( lColumn, lWidth );
}

void CSheetDescription::SetUserVariable( USHORT usVarID, LPARAM lVarValue ) { m_mapUserVariable[usVarID] = lVarValue; }

bool CSheetDescription::GetUserVariable( USHORT usVarID, LPARAM &lVarValue )
{ 
	lVarValue = (LPARAM)0;
	if( 0 == m_mapUserVariable.count( usVarID ) )
		return false;
	lVarValue = m_mapUserVariable[usVarID];
	return true;
}

void CSheetDescription::SetFocusColumnRange( long lFromColumn, long lToColumn )
{
	m_lSelectionFromColumn = lFromColumn;
	m_lSelectionToColumn = lToColumn;
}

void CSheetDescription::GetFocusColumnRange( long &lFromColumn, long &lToColumn )
{ 
	lFromColumn = m_lSelectionFromColumn; 
	lToColumn = m_lSelectionToColumn;
}

long CSheetDescription::GetSelectionFrom( void ) { return m_lSelectionFromColumn; }
long CSheetDescription::GetSelectionTo( void ) { return m_lSelectionToColumn; }

long CSheetDescription::GetFirstFocusedColumn( void )
{
	long lColumn = -1;
	if( SD_ParameterMode_Mono == m_nParameterMode )
	{
		// There is only one column on which we can set the focus
		lColumn = m_lSelectionFromColumn;
	}
	else
	{
		// In case of accessories sheet for example we can have more than one column on which we
		// can set the focus.
		lColumn = m_vecParameterList[0];
	}

	return lColumn;
}

long CSheetDescription::GetActiveColumn( void ) { return m_lActiveColumn; }

void CSheetDescription::AddRows( long lRow, bool fSelectable )
{
	if( lRow <= 0 )
		return;

	m_pclSSheet->SetMaxRows( m_pclSSheet->GetMaxRows() + lRow );

	// Add rows in the selectable list.
	if( true == fSelectable )
	{
		long lLoop = m_lRowEnd + 1;
		while( lLoop <= m_lRowEnd + lRow )
		{
			if( true == fSelectable )
				_InsertRowInSelectableList( lLoop );
		
			lLoop += m_nRowsByItem;
		}
	}
		
	m_lRowEnd += lRow;
}

bool CSheetDescription::SetSelectableRangeRow( long lFirstSelectableRow, long lLastSelectableRow )
{
	bool fResult = false;

	// Swap if needed.
	if( lFirstSelectableRow > lLastSelectableRow )
	{
		long lTemp = lFirstSelectableRow;
		lFirstSelectableRow = lLastSelectableRow;
		lLastSelectableRow = lTemp;
	}
	
	// Do only if range is correct.
	if( lFirstSelectableRow >= m_lRowStart && lLastSelectableRow <= m_lRowEnd )
	{
		long lLoop = lFirstSelectableRow;
		while( lLoop <= lLastSelectableRow )
		{
			_InsertRowInSelectableList( lLoop );
			lLoop += m_nRowsByItem;
		}
		fResult = true;
	}
	return fResult;
}

bool CSheetDescription::RemoveSelectableRow( long lRow )
{
	bool fRemoved = false;
	if( m_vecSelectableRowsList.size() > 0 )
	{
		for( vecLongIter vecIter = m_vecSelectableRowsList.begin(); vecIter != m_vecSelectableRowsList.end(); ++vecIter )
		{
			if( lRow == *vecIter )
			{
				m_vecSelectableRowsList.erase( vecIter );
				fRemoved = true;
				break;
			}
		}
	}
	return fRemoved;
}

bool CSheetDescription::IsRowSelectable( long lRow )
{
	bool fSelectable = false;
	if( m_vecSelectableRowsList.size() > 0 )
	{
		for( vecLongIter vecIter = m_vecSelectableRowsList.begin(); vecIter != m_vecSelectableRowsList.end(); ++vecIter )
		{
			// Check in vector list if lRow exists
			if( lRow == *vecIter )
			{
				fSelectable = true;
				break;
			}
			else if( lRow < *vecIter )
			{
				// No need to go further because vector is explicitly sorted 
				break;
			}
		}
	}
	return fSelectable;
}

long CSheetDescription::GetFirstSelectableRow( bool fExcludeHidden )
{
	long lFirstSelectableRow = -1;
	if( m_vecSelectableRowsList.size() > 0 )
	{
		// Remark: vector is explicitly sorted!
		for( vecLongIter vecIter = m_vecSelectableRowsList.begin(); vecIter != m_vecSelectableRowsList.end() && -1 == lFirstSelectableRow; ++vecIter )
		{
			// Can take this row only if it is not hidden.
			if( false == fExcludeHidden || ( true == fExcludeHidden && FALSE == m_pclSSheet->IsRowHidden( *vecIter ) ) )
				lFirstSelectableRow = *vecIter;
		}
	}
	return lFirstSelectableRow;
}

long CSheetDescription::GetNextSelectableRow( long lColumn, long lRow, bool fExcludeHidden )
{
	long lNextSelectableRow = -1;
	// If there is selectable rows and 'lRow' is in the range for this group...
	if( m_vecSelectableRowsList.size() > 0 && lRow >= m_lRowStart && lRow < m_lRowEnd )
	{
		// Try first to find the row in selectable list.
		bool fFound = false;
		long lLoopSelectableRow = 0;
		for( ; lLoopSelectableRow < (long)m_vecSelectableRowsList.size() && false == fFound; lLoopSelectableRow++ )
		{
			if( lRow == m_vecSelectableRowsList[lLoopSelectableRow] )
				fFound = true;
		}
		
		if( ( true == fFound ) && ( lLoopSelectableRow < (long)m_vecSelectableRowsList.size() ) )
		{
			// Find the next selectable row.
			fFound = false;
			for( long lLoopRow = lLoopSelectableRow; lLoopRow < (long)m_vecSelectableRowsList.size() && false == fFound; lLoopRow++ )
			{
				// If we are in multi parameter mode...
				if( SD_ParameterMode_Multi == m_nParameterMode )
				{
					// Check if next row is valid (has at least one parameter).
					LPARAM lCurrentParam = m_pclSSheet->GetCellParam( lColumn, m_vecSelectableRowsList[lLoopSelectableRow] );
					if( NULL == lCurrentParam )
						continue;
				}

				// Now check if this row is not hidden.
				if( false == fExcludeHidden || ( true == fExcludeHidden && FALSE == m_pclSSheet->IsRowHidden( m_vecSelectableRowsList[lLoopRow] ) ) )
				{
					lNextSelectableRow = m_vecSelectableRowsList[lLoopRow];
					fFound = true;
				}
			}
		}
	}
	else if( lRow < m_lRowStart )
	{
		// Search in this case the first selectable row in this group that is not hidden
		bool fFound = false;
		for( long lLoopRow = 0; lLoopRow < (long)m_vecSelectableRowsList.size() && false == fFound ; lLoopRow++ )
		{
			if( false == fExcludeHidden || ( true == fExcludeHidden && FALSE == m_pclSSheet->IsRowHidden( m_vecSelectableRowsList[lLoopRow] ) ) )
			{
				lNextSelectableRow = m_vecSelectableRowsList[lLoopRow];
				fFound = true;
			}
		}
	}

	return lNextSelectableRow;
}

long CSheetDescription::GetPrevSelectableRow( long lColumn, long lRow, bool fExcludeHidden )
{
	long lPrevSelectableRow = -1;
	// If there is selectable rows and 'lRow' is in the range for this group...
	if( m_vecSelectableRowsList.size() > 0 && lRow > m_lRowStart && lRow <= m_lRowEnd )
	{
		// Try first to find the row in selectable list.
		bool fFound = false;
		long lLoopSelectableRow = (long)m_vecSelectableRowsList.size() - 1;
		for( ; lLoopSelectableRow >= 0 && false == fFound; lLoopSelectableRow-- )
		{
			if( lRow == m_vecSelectableRowsList[lLoopSelectableRow] )
				fFound = true;
		}

		if( true == fFound && lLoopSelectableRow >= 0 )
		{
			// Find the previous selectable row.
			fFound = false;
			for( long lLoopRow = lLoopSelectableRow; lLoopRow >= 0 && false == fFound; lLoopRow-- )
			{
				// If we are in multi mode...
				if( SD_ParameterMode_Multi == m_nParameterMode )
				{
					// Check if previous row is valid (has at least one parameter).
					LPARAM lCurrentParam = m_pclSSheet->GetCellParam( lColumn, m_vecSelectableRowsList[lLoopSelectableRow] );
					if( NULL == lCurrentParam )
						continue;
				}

				// Now check if this row is not hidden.
				if( false == fExcludeHidden || ( true == fExcludeHidden && FALSE == m_pclSSheet->IsRowHidden( m_vecSelectableRowsList[lLoopRow] ) ) )
				{
					lPrevSelectableRow = m_vecSelectableRowsList[lLoopRow];
					fFound = true;
				}
			}
		}
	}
	else if( lRow > m_lRowEnd )
	{
		// Search in this case the first selectable row in this group that is not hidden
		bool fFound = false;
		for( long lLoopRow = m_vecSelectableRowsList.size() - 1; lLoopRow >= 0 && false == fFound ; lLoopRow-- )
		{
			if( false == fExcludeHidden || ( true == fExcludeHidden && FALSE == m_pclSSheet->IsRowHidden( m_vecSelectableRowsList[lLoopRow] ) ) )
			{
				lPrevSelectableRow = m_vecSelectableRowsList[lLoopRow];
				fFound = true;
			}
		}
	}

	return lPrevSelectableRow;
}

long CSheetDescription::GetLastSelectableRow( bool fExcludeHidden )
{
	long lLastSelectableRow = -1;
	if( m_vecSelectableRowsList.size() > 0 )
	{
		// Remark: vector is explicitly sorted!
		for( vecLongReverseIter vecIter = m_vecSelectableRowsList.rbegin(); vecIter != m_vecSelectableRowsList.rend() && -1 == lLastSelectableRow; ++vecIter )
		{
			// Can take this row only if it is not hidden.
			if( false == fExcludeHidden || ( true == fExcludeHidden && FALSE == m_pclSSheet->IsRowHidden( *vecIter ) ) )
				lLastSelectableRow = *vecIter;
		}
	}
	return lLastSelectableRow;
}

long CSheetDescription::GetFirstRow( void ) { return m_lRowStart; }
long CSheetDescription::GetLastRow( void ) { return m_lRowEnd; }

long CSheetDescription::GetFirstRowOfItem( long lReferenceRow )
{
	long lFirstRow = -1;
	if( lReferenceRow >= m_lRowStart && lReferenceRow <= m_lRowEnd && m_vecSelectableRowsList.size() > 0 )
	{
		for( vecLongIter vecIter = m_vecSelectableRowsList.begin(); vecIter != m_vecSelectableRowsList.end(); ++vecIter )
		{
			if( lReferenceRow >= *vecIter && lReferenceRow < *vecIter + m_nRowsByItem )
			{
				lFirstRow = *vecIter;
				break;
			}
		}
	}
	return lFirstRow;
}

short CSheetDescription::GetRowsByItem( void ) { return m_nRowsByItem; }

long CSheetDescription::GetFirstColumn( bool fExcludeHidden )
{
	// Sanity check.
	long lMaxCols = m_pclSSheet->GetMaxCols();
	if( lMaxCols <= 0 )
		return -1;

	// By default the first column has index 1.
	long lFirstColumn = 1;

	// If we must exclude hidden columns...
	if( true == fExcludeHidden )
	{
		// Run all columns to find first non hidden one.
		while( lFirstColumn <= lMaxCols )
		{
			if( false == m_pclSSheet->IsColHidden( lFirstColumn ) )
				break;
			lFirstColumn++;
		}
	
		// If not found...
		if( lFirstColumn > lMaxCols )
			lFirstColumn = -1;
	}
	
	return lFirstColumn;
}

long CSheetDescription::GetNextColumn( long lFromColumn, bool fExcludeHidden )
{
	// Sanity check
	long lMaxCols = m_pclSSheet->GetMaxCols();
	if( lMaxCols <= 0 || lFromColumn <= 0 || lFromColumn >= lMaxCols )
		return -1;

	// Set the next column.
	long lNextColumn = lFromColumn + 1;

	// If we must exclude hidden columns...
	if( true == fExcludeHidden )
	{
		// Run all columns to find first non hidden one.
		while( lNextColumn <= lMaxCols )
		{
			if( false == m_pclSSheet->IsColHidden( lNextColumn ) )
				break;
			lNextColumn++;
		}
	
		// If not found...
		if( lNextColumn > lMaxCols )
			lNextColumn = -1;
	}
	
	return lNextColumn;
}

long CSheetDescription::GetPreviousColumn( long lFromColumn, bool fExcludeHidden )
{
	// Sanity check
	long lMaxCols = m_pclSSheet->GetMaxCols();
	if( lMaxCols <= 0 || lFromColumn <= 1 || lFromColumn >= lMaxCols )
		return -1;

	// Set the previous column.
	long lPrevColumn = lFromColumn - 1;
	
	// If we must exclude hidden columns...
	if( true == fExcludeHidden )
	{
		// Run all columns to find first non hidden one.
		while( lPrevColumn >= 1 )
		{
			if( false == m_pclSSheet->IsColHidden( lPrevColumn ) )
				break;
			lPrevColumn--;
		}
	
		// If not found...
		if( 0 == lPrevColumn )
			lPrevColumn = -1;
	}
	
	return lPrevColumn;
}

long CSheetDescription::GetLastColumn( bool fExcludeHidden )
{
	// Sanity check.
	long lLastColumn = m_pclSSheet->GetMaxCols();
	if( lLastColumn <= 0 )
		return -1;

	// If we must exclude hidden columns...
	if( true == fExcludeHidden )
	{
		// Run all columns to find first non hidden one.
		while( lLastColumn >= 1 )
		{
			if( false == m_pclSSheet->IsColHidden( lLastColumn ) )
				break;
			lLastColumn--;
		}
	
		// If not found...
		if( 0 == lLastColumn )
			lLastColumn = -1;
	}
	
	return lLastColumn;
}

void CSheetDescription::AddParameterColumn( long lColumn )
{ 
	// If one parameter by row allowed...
	if( SD_ParameterMode_Mono == m_nParameterMode )
	{
		// Only one parameter is allowed.
		if( 0 == m_vecParameterList.size() )
			m_vecParameterList.push_back( lColumn );
		else
			m_vecParameterList[0] = lColumn;
	}
	else
	{
		// Check if column is not already in the list
		if( m_vecParameterList.size() )
		{
			for( unsigned int i = 0; i < m_vecParameterList.size(); i++ )
			{
				if( m_vecParameterList[i] == lColumn )
					return;
			}
		}

		m_vecParameterList.push_back( lColumn );
	}
}

long CSheetDescription::GetFirstParameterColumn( void )
{
	if( 0 == m_vecParameterList.size() )
		return -1;
	else
		return m_vecParameterList[0];
}

long CSheetDescription::GetNextParameterColumn( long lColumn )
{
	if( 0 == m_vecParameterList.size() || lColumn < 1 )
		return -1;

	// If we are in parameter mode mono, it means that we have only one possible column for the parameter.
	// If we are in parameter mode multi and there is only one parameter saved in list, we directly return the first one.
	if( SD_ParameterMode_Mono == m_nParameterMode || ( SD_ParameterMode_Multi == m_nParameterMode && 1 == m_vecParameterList.size() ) )
		return m_vecParameterList[0];

	// Search the next one.
	long lSmallerDifference = LONG_MAX;
	long lNextParameterColumn = -1;
	for( unsigned short nParameterLoop = 0; nParameterLoop < m_vecParameterList.size(); nParameterLoop++ )
	{
		if( ( m_vecParameterList[nParameterLoop] > lColumn ) && ( ( m_vecParameterList[nParameterLoop] - lColumn ) < lSmallerDifference ) )
		{
			lSmallerDifference = m_vecParameterList[nParameterLoop] - lColumn;
			lNextParameterColumn = m_vecParameterList[nParameterLoop];
		}
	}
	
	return lNextParameterColumn;
}

long CSheetDescription::GetPrevParameterColumn( long lColumn )
{
	if( 0 == m_vecParameterList.size() || lColumn < 1 )
		return -1;

	// If we are in parameter mode mono, it means that we have only one possible column for the parameter.
	// If we are in parameter mode multi and there is only one parameter saved in list, we directly return the first one.
	if( SD_ParameterMode_Mono == m_nParameterMode || ( SD_ParameterMode_Multi == m_nParameterMode && 1 == m_vecParameterList.size() ) )
		return m_vecParameterList[0];

	// Search the previous one.
	long lSmallerDifference = LONG_MAX;
	long lPrevParameterColumn = -1;
	for( unsigned short nParameterLoop = 0; nParameterLoop < m_vecParameterList.size(); nParameterLoop++ )
	{
		if( ( m_vecParameterList[nParameterLoop] < lColumn ) && ( ( lColumn - m_vecParameterList[nParameterLoop] ) < lSmallerDifference ) )
		{
			lSmallerDifference = lColumn - m_vecParameterList[nParameterLoop];
			lPrevParameterColumn = m_vecParameterList[nParameterLoop];
		}
	}
	
	return lPrevParameterColumn;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage 'm_mapCellDescriptionList'.
//

ULONG CSheetDescription::AddCellDescription( CCellDescription* pclCellDescription )
{
	ULONG ulCellDescriptionID = 0;

	// Sanity check.
	if( NULL != pclCellDescription )
	{
		// Save ID in cell param.
		m_pclSSheet->SetCellParam( pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y, (LONG)m_ulCellDescriptionID );
		
		// Add cell description in map.
		m_mapCellDescriptionList[m_ulCellDescriptionID] = pclCellDescription;
		ulCellDescriptionID = m_ulCellDescriptionID++;
	}
	
	return ulCellDescriptionID;
}

CCellDescription* CSheetDescription::GetCellDescription( long lColumn, long lRow )
{
	CCellDescription* pclCellDescription = NULL;
	
	// Retrieve ID.
	ULONG ulID = (ULONG)m_pclSSheet->GetCellParam( lColumn, lRow );
	
	// If no ID...
	if( 0 == ulID )
	{
		if( SD_ParameterMode_Mono == m_nParameterMode )
		{
			// if sheet is in mono parameter mode, it means that perhaps user has clicked on a row with a product.
			long lColumnParam = GetFirstParameterColumn();
			if( lColumnParam != - 1 )
				ulID = (ULONG)m_pclSSheet->GetCellParam( lColumnParam, lRow );
		}
		else if( SD_ParameterMode_Multi == m_nParameterMode )
		{
			// if sheet is in multi parameter mode, it means that perhaps user has clicked on a item with more than one line.
			long lFirstRow = -1;
			if( m_nRowsByItem > 1 )
				lFirstRow = GetFirstRowOfItem( lRow );
			if( lFirstRow != -1 )
				ulID = (ULONG)m_pclSSheet->GetCellParam( lColumn, lFirstRow );
		}
	}

	if( ulID > 0 )
	{
		if( m_mapCellDescriptionList.count( ulID ) > 0 && NULL != m_mapCellDescriptionList[ulID] )
			pclCellDescription = m_mapCellDescriptionList[ulID];
	}

	return pclCellDescription;
}

CCellDescription* CSheetDescription::GetCellDescription( ULONG ulCellDescriptionID )
{
	CCellDescription *pclCellDescription = NULL;
	if( m_mapCellDescriptionList.count( ulCellDescriptionID ) > 0 )
		pclCellDescription = m_mapCellDescriptionList[ulCellDescriptionID];
	return pclCellDescription;
}

void CSheetDescription::GetCellDescriptionList( vecCellDescription& vecCellDescriptionList, int iCellType )
{
	for( mapULongCellDescriptionIter mapIter = m_mapCellDescriptionList.begin(); mapIter != m_mapCellDescriptionList.end(); ++mapIter )
	{
		// If cell description is well defined and we must retrieve all cell description or it's the same type as the one passed as argument...
		if( NULL != mapIter->second && ( -1 == iCellType || ( iCellType > -1 && iCellType == mapIter->second->GetCellType() ) ) )
			vecCellDescriptionList.push_back( mapIter->second );
	}
}

bool CSheetDescription::RemoveCellDescription( CCellDescription* pclCellDescription )
{
	bool fReturn = false;
	if( NULL != pclCellDescription )
	{
		for( mapULongCellDescriptionIter mapIter = m_mapCellDescriptionList.begin(); mapIter != m_mapCellDescriptionList.end(); ++mapIter )
		{
			if( mapIter->second == pclCellDescription )
			{
				// Delete cell description.
				delete mapIter->second;
				// Remove entry from the map.
				m_mapCellDescriptionList.erase( mapIter );
				fReturn = true;
				break;
			}
		}
	}
	return fReturn;
}

void CSheetDescription::ClearCellDescription( int iCellType )
{
	mapULongCellDescriptionIter mapIter = m_mapCellDescriptionList.begin();
	while( mapIter != m_mapCellDescriptionList.end() )
	{
		// If cell description is well defined and we must delete all cell description or it's the same type as the one passed as argument...
		if( NULL != mapIter->second && ( -1 == iCellType || iCellType == mapIter->second->GetCellType() ) )
		{
			mapULongCellDescriptionIter mapIterNext = mapIter;
			++mapIterNext;
			delete mapIter->second;
			m_mapCellDescriptionList.erase( mapIter );
			mapIter = mapIterNext;
		}
		else
			++mapIter;
	}

	// If we have deleted all cell descriptions...
	if( -1 == iCellType )
		m_ulCellDescriptionID = 1;
}

CCellDescription* CSheetDescription::GetFirstCellDescription( int iCellType )
{
	CCellDescription* pclFirstCellDescription = NULL;
	if( m_mapCellDescriptionList.size() > 0 )
	{
		mapULongCellDescriptionIter mapIter = m_mapCellDescriptionList.begin();
		while( mapIter != m_mapCellDescriptionList.end() )
		{
			if( NULL != mapIter->second )
			{
				if( -1 == iCellType || iCellType == mapIter->second->GetCellType() )
				{
					pclFirstCellDescription = mapIter->second;
					break;
				}
			}
			++mapIter;
		}
	}
	return pclFirstCellDescription;
}

CCellDescription* CSheetDescription::GetNextCellDescription( CCellDescription *pclCurrentCellDescription, int iCellType )
{
	// Sanity check
	if( NULL == pclCurrentCellDescription )
		return NULL;

	CCellDescription* pclNextCellDescription = NULL;
	if( m_mapCellDescriptionList.size() > 0 )
	{
		mapULongCellDescriptionIter mapIter = m_mapCellDescriptionList.begin();
		bool fReferenceFound = false;
		while( mapIter != m_mapCellDescriptionList.end() )
		{
			if( NULL != mapIter->second )
			{
				if( false == fReferenceFound ) 
				{
					if( pclCurrentCellDescription == mapIter->second )
						fReferenceFound = true;
				}
				else if( -1 == iCellType || iCellType == mapIter->second->GetCellType() )
				{
					pclNextCellDescription = mapIter->second;
					break;
				}
			}
			++mapIter;
		}
	}
	return pclNextCellDescription;
}

// End of methods to manage 'm_mapCellDescriptionList'.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage remarks.

void CSheetDescription::RestartRemarkGenerator( void )
{
	m_vecRemarkList.clear();
}

void CSheetDescription::WriteTextWithFlags( CString strText, long lColumn, long lRow, RemarkFlags eRemarkFlag, CString strMessage, COLORREF cTextColor, COLORREF cBackColor )
{
	if( NULL == m_pclSSheet )
	{
		return;
	}

	// NB: if it's the first time we write a text with remark, we create a container to save data linked to it.
	//     For the next callings, we add only text coordinates of new text in the container.

	// Check if remark flag already exists in vector or add it.
	CString strIndex;
	short nIndex;

	for( nIndex = 0; nIndex < (short)m_vecRemarkList.size(); nIndex++ )
	{
		if( eRemarkFlag == m_vecRemarkList[nIndex].m_eRemarkflag )
		{
			if( eRemarkFlag != RemarkFlags::ErrorMessage && eRemarkFlag != RemarkFlags::WarningMessage && eRemarkFlag != RemarkFlags::InfoMessage )
			{
				break;
			}
			else
			{
				if( 0 == strMessage.CompareNoCase( m_vecRemarkList[nIndex].m_strMessage ) )
				{
					break;
				}
			}
		}
	}

	// If remark doesn't exist yet...
	if( nIndex == m_vecRemarkList.size() )
	{
		m_vecRemarkList.push_back( CSaveRemark( eRemarkFlag ) );
		m_vecRemarkList.back().m_strMessage = strMessage;
	}
	
	// If position already exists, don't add it...
	bool bNewCoordinate = true;

	if( nIndex < (short)m_vecRemarkList.size() && m_vecRemarkList[nIndex].m_vecTextList.size() > 0 )
	{
		for( int iLoopCoord = 0; iLoopCoord < (short)m_vecRemarkList[nIndex].m_vecTextList.size() && true == bNewCoordinate; iLoopCoord++ )
		{
			if( m_vecRemarkList[nIndex].m_vecTextList[iLoopCoord] == CPoint( lColumn, lRow ) )
			{
				bNewCoordinate = false;
			}
		}
	}
	
	// Save position of the text.
	if( true == bNewCoordinate )
	{
		m_vecRemarkList[nIndex].m_vecTextList.push_back( CPoint( lColumn, lRow ) );
	}
	
	if( false == strText.IsEmpty() )
	{
		strIndex.Format( _T(" (%u)"), nIndex + 1 );
	}
	else
	{
		strIndex.Format( _T("(%u)"), nIndex + 1 );
	}

	COLORREF cPrevForeColor = (COLORREF)m_pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor );
	COLORREF cPrevBackColor = (COLORREF)m_pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor );

	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)cTextColor );
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)cBackColor );

	strText += strIndex;
	m_pclSSheet->SetStaticText( lColumn, lRow, strText );

	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)cPrevForeColor );
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)cPrevBackColor );
}

void CSheetDescription::WriteTextWithFlags( CString strText, long lColumn, long lRow, RemarkFlags eRemarkFlag, std::vector<CString> vecErrorMessages, COLORREF cTextColor, COLORREF cBackColor )
{
	if( NULL == m_pclSSheet )
	{
		return;
	}

	// NB: if it's the first time we write a text with remark, we create a container to save data linked to it.
	//     For the next callings, we add only text coordinates of new text in the container.

	// Check if remark flag already exists in vector or add it.
	std::vector<CString> vecnMessages;
	std::vector<short> vecnIndex;

	for( int i = 0; i < (short)m_vecRemarkList.size(); i++ )
	{
		if( eRemarkFlag == m_vecRemarkList.at(i).m_eRemarkflag )
		{
			// Found one remark with the same flag.

			if( eRemarkFlag != RemarkFlags::ErrorMessage && eRemarkFlag != RemarkFlags::WarningMessage && eRemarkFlag != RemarkFlags::InfoMessage )
			{
				// We can put the index in the vector and stop. Because for other remark that the error, warning and information messages, it can be
				// only on remark.
				vecnIndex.push_back( i );
				break;
			}
			else
			{
				// Special treatment for the error, warning and information messages.
				// We can have several messages for the same coordinate.
				// We save thus the indexes found with the same message.
				for( auto &iter : vecErrorMessages )
				{
					if( 0 == iter.CompareNoCase( m_vecRemarkList.at( i ).m_strMessage ) )
					{
						vecnIndex.push_back( i );
						vecnMessages.push_back( m_vecRemarkList.at( i ).m_strMessage );
					}
				}
			}
		}
	}

	// Special treatment for the error, warning and information messages.
	if( eRemarkFlag == RemarkFlags::ErrorMessage || eRemarkFlag == RemarkFlags::WarningMessage || eRemarkFlag == RemarkFlags::InfoMessage )
	{
		// Check what are the new messages.
		for( auto &iter : vecErrorMessages )
		{
			std::vector<CString>::iterator finditer = std::find( vecnMessages.begin(), vecnMessages.end(), iter );

			if( vecnMessages.end() == finditer )
			{
				m_vecRemarkList.push_back( CSaveRemark( eRemarkFlag ) );
				m_vecRemarkList.back().m_strMessage = iter;
				vecnIndex.push_back( (short)m_vecRemarkList.size() - 1 );
			}
		}
	}
	else if( 0 == (int)vecnIndex.size() )
	{
		// If remark doesn't exist yet...
		m_vecRemarkList.push_back( CSaveRemark( eRemarkFlag ) );
	}
	
	// If position already exists, don't add it...
	CString strIndex;

	for( auto &iterIndex : vecnIndex )
	{
		bool bNewCoordinate = true;

		if( iterIndex < (short)m_vecRemarkList.size() && m_vecRemarkList.at( iterIndex ).m_vecTextList.size() > 0 )
		{
			for( int iLoopCoord = 0; iLoopCoord < (short)m_vecRemarkList.at( iterIndex ).m_vecTextList.size() && true == bNewCoordinate; iLoopCoord++ )
			{
				if( m_vecRemarkList.at( iterIndex ).m_vecTextList[iLoopCoord] == CPoint( lColumn, lRow ) )
				{
					bNewCoordinate = false;
				}
			}
		}
	
		// Save position of the text.
		if( true == bNewCoordinate )
		{
			m_vecRemarkList.at( iterIndex ).m_vecTextList.push_back( CPoint( lColumn, lRow ) );
		}
	
		if( true == strIndex.IsEmpty() )
		{
			if( false == strText.IsEmpty() )
			{
				strIndex.Format( _T(" (%u)"), iterIndex + 1 );
			}
			else
			{
				strIndex.Format( _T("(%u)"), iterIndex + 1 );
			}
		}
		else
		{
			strIndex.Format( _T("%s(%u)"), strIndex, iterIndex + 1 );
		}
	}

	COLORREF cPrevForeColor = (COLORREF)m_pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor );
	COLORREF cPrevBackColor = (COLORREF)m_pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor );

	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)cTextColor );
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)cBackColor );

	strText += strIndex;
	m_pclSSheet->SetStaticText( lColumn, lRow, strText );

	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)cPrevForeColor );
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)cPrevBackColor );
}

long CSheetDescription::WriteRemarks( long lRow, long lMergeColFrom, long lMergeColTo )
{
	// Sanity check.
	if( NULL == m_pclSSheet || 0 == m_vecRemarkList.size() )
	{
		return lRow;
	}

	for( int iLoopIndex = 0; iLoopIndex < (short)m_vecRemarkList.size(); iLoopIndex++ )
	{
		_WriteOneRemark( m_vecRemarkList[iLoopIndex].m_eRemarkflag, iLoopIndex + 1, lRow, lMergeColFrom, lMergeColTo, m_vecRemarkList[iLoopIndex].m_strMessage );
		
		// Save coordinates.
		m_vecRemarkList[iLoopIndex].m_lColumnFrom = lMergeColFrom;
		m_vecRemarkList[iLoopIndex].m_lColumnTo = lMergeColTo;
		m_vecRemarkList[iLoopIndex].m_lRow = lRow;

		lRow++;
	}
	
	return lRow;
}

void CSheetDescription::ClearRemarks( void )
{
	// If no remarks added with 'WriteRemarks' then exit...
	if( 0 == m_vecRemarkList.size() )
		return;

	for( vecCSaveRemarkRIter iter = m_vecRemarkList.rbegin(); iter != m_vecRemarkList.rend(); ++iter )
	{
		if( iter->m_lRow <= m_lRowEnd )
		{
			m_pclSSheet->DelRow( iter->m_lRow );
			
			// Remark: 'DelRow' will erase the row BUT TSpread will automatically add a blank row at the end of the sheet.
			//         TSpread do that to respect the maximum number of rows specified by 'SetMaxRows'.
			m_pclSSheet->SetMaxRows( m_pclSSheet->GetMaxRows() - 1 );
			m_lRowEnd--;
		}
	}
}

bool CSheetDescription::RemoveOneRemark( RemarkFlags eRemarkFlag )
{
	// If not yet remarks added with 'WriteRemarks' then exit...
	if( 0 == m_vecRemarkList.size() )
		return false;

	// Check first if remark exists.
	CString strIndex;
	short nIndex;
	for( nIndex = 0; nIndex < (short)m_vecRemarkList.size() && eRemarkFlag != m_vecRemarkList[nIndex].m_eRemarkflag; nIndex++ )
	{}

	// If remark doesn't exist...
	if( nIndex == m_vecRemarkList.size() )
		return false;

	// Remove line.
	m_pclSSheet->DelRow( m_vecRemarkList[nIndex].m_lRow );
	
	// Remark: 'DelRow' will erase the row BUT TSpread will automatically add a blank row at the end of the sheet.
	//         TSpread do that to respect the maximum number of rows specified by 'SetMaxRows'.
	m_pclSSheet->SetMaxRows( m_pclSSheet->GetMaxRows() - 1 );
	m_lRowEnd--;

	// Delete reference for this index.
	CString strText, strToFind, strToReplace;
	for( int iLoopText = 0; iLoopText < (short)m_vecRemarkList[nIndex].m_vecTextList.size(); iLoopText++ )
	{
		CPoint ptCoord = m_vecRemarkList[nIndex].m_vecTextList[iLoopText];
		strText = m_pclSSheet->GetCellText( ptCoord.x, ptCoord.y );
		strToFind.Format( _T("(%u)"), nIndex + 1 );
		strText.Replace( strToFind, _T("") );
		m_pclSSheet->SetCellText( ptCoord.x, ptCoord.y, strText );
	}

	// Update next remarks (if exist).
	for( short nNextIndex = nIndex + 1; nNextIndex < (short)m_vecRemarkList.size(); nNextIndex++ )
	{
		// Update row coordinate in CSaveRemark.
		m_vecRemarkList[nNextIndex].m_lRow--;

		// Update index of this remark.
		strText = m_pclSSheet->GetCellText( m_vecRemarkList[nNextIndex].m_lColumnFrom, m_vecRemarkList[nNextIndex].m_lRow );
		strToFind.Format( _T("(%u)"), nNextIndex + 1 );
		strToReplace.Format( _T("(%u)"), nNextIndex );
		strText.Replace( strToFind, strToReplace );
		m_pclSSheet->SetCellText( m_vecRemarkList[nNextIndex].m_lColumnFrom, m_vecRemarkList[nNextIndex].m_lRow, strText );

		// Update all text corresponding to this remark.
		for( int iLoopText = 0; iLoopText < (short)m_vecRemarkList[nNextIndex].m_vecTextList.size(); iLoopText++ )
		{
			CPoint ptCoord = m_vecRemarkList[nNextIndex].m_vecTextList[iLoopText];
			strText = m_pclSSheet->GetCellText( ptCoord.x, ptCoord.y );
			strToFind.Format( _T("(%u)"), nNextIndex + 1 );
			strToReplace.Format( _T("(%u)"), nNextIndex );
			strText.Replace( strToFind, strToReplace );
			m_pclSSheet->SetCellText( ptCoord.x, ptCoord.y, strText );
		}
	}

	// Delete old entry.
	m_vecRemarkList.erase( m_vecRemarkList.begin() + nIndex );

	return true;
}

bool CSheetDescription::IsRemarkExist( RemarkFlags eRemarkFlag )
{
	short nIndex;
	for( nIndex = 0; nIndex < (short)m_vecRemarkList.size() && eRemarkFlag != m_vecRemarkList[nIndex].m_eRemarkflag; nIndex++ )
	{}

	// If remark doesn't exist...
	return ( nIndex < (short)m_vecRemarkList.size() );
}

// End of methods to manage remarks.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSheetDescription::DumpCellDescriptionList( void )
{
	// Dump accessory
	int i = 0;
	CString str;
	CCellDescription *pclCellDescription;
	for( mapULongCellDescriptionIter mapIter = m_mapCellDescriptionList.begin(); mapIter != m_mapCellDescriptionList.end(); ++mapIter )
	{
		str.Format( _T("Cell description %02d\n"), i++ );
		OutputDebugString( str );
		
		str.Format( _T("===================\n") );
		OutputDebugString( str );
		
		pclCellDescription = mapIter->second;
		str.Format( _T(" ID: %d\n"), mapIter->first );
		OutputDebugString( str );

		str.Format( _T(" x: %02d - y: %02d\n"), pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y );
		OutputDebugString( str );
		
		str.Format( _T(" Type: %d\n"), pclCellDescription->GetCellType() );
		OutputDebugString( str );
		
		str.Format( _T(" Param: %d\n\n"), m_pclSSheet->GetCellParam( pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y ) );
		OutputDebugString( str );
	}
}

void CSheetDescription::OnMouseEvent( int iMouseEventType, long lColumn, long lRow, CPoint ptMouse )
{
	m_clResizingColumnsInfo.OnMouseEvent( iMouseEventType, lColumn, lRow, ptMouse );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSheetDescription::_InsertRowInSelectableList( long lRow )
{
	vecLongIter vecIter = m_vecSelectableRowsList.begin();
	long lPrevRow = -1;
	bool fByPass = false;
	for( vecIter; vecIter != m_vecSelectableRowsList.end(); ++vecIter )
	{
		// If column to insert is below current value in vector...
		if( lRow < *vecIter )
		{
			// Do not insert if previous value is the same...
			if( ( lPrevRow != -1 && lRow != lPrevRow ) )
				m_vecSelectableRowsList.insert( vecIter, lRow );

			// Specify to not execute last condition after this loop for.
			fByPass = true;
			break;
		}
	}

	if( false == fByPass && vecIter == m_vecSelectableRowsList.end() )
		m_vecSelectableRowsList.push_back( lRow );
}

void CSheetDescription::_Clean( void )
{
	// Clear all user variables.
	m_mapUserVariable.clear();

	// Clear all parameter column positions.
	m_vecParameterList.clear();

	// Clear all selectable rows.
	m_vecSelectableRowsList.clear();

	// Clear all cell type.
	ClearCellDescription();

	m_uiSheetDescriptionID = 0;
	m_uiSSheetID = 0;
	m_pclSSheet = NULL;
	m_ptOffset = CPoint( 0, 0 );

	m_lRowStart = 0;
	m_lRowEnd = 0;
	m_nRowsByItem = 1;
	m_nParameterMode = SD_ParameterMode_Mono;
	m_lActiveColumn = -1;
	m_lSelectionFromColumn = -1;
	m_lSelectionToColumn = -1;

	m_bShow = true;
	m_uiGroupID = -1;
	m_uiParentID = -1;
}

void CSheetDescription::_WriteOneRemark( RemarkFlags eRemarkFlag, int iIndex, long lRow, long lMergeColFrom, long lMergeColTo, CString strMessage )
{
	// 'false' to specify that this row can't be selected.
	AddRows( 1, false );

	// Prepare remark.
	COLORREF cTextColor;
	CString strRemark;
	strRemark.Format( _T("(%u) "), iIndex );

	switch( eRemarkFlag )
	{
		case RemarkFlags::NotAvailable:
			cTextColor = _RED;
			strRemark += TASApp.LoadLocalizedString( IDS_REMARKVALVENOTAVAILABLE );
			break;

		case RemarkFlags::Deleted:
			cTextColor = _RED;
			strRemark += TASApp.LoadLocalizedString( IDS_REMARKPRODUCTDELETED );
			break;

		case RemarkFlags::FullOpening:
			cTextColor = _BLACK;
			strRemark += TASApp.LoadLocalizedString( IDS_REMARKFULLOPENING );
			break;

		case RemarkFlags::KTHProportional:
			cTextColor = _BLACK;
			strRemark += TASApp.LoadLocalizedString( IDS_INFOKTHPROP );
			break;

		case RemarkFlags::KTH3Points:
			cTextColor = _BLACK;
			strRemark += TASApp.LoadLocalizedString( IDS_INFOKTH3P );
			break;

		case RemarkFlags::ErrorMessage:
			cTextColor = _RED;
			strRemark += strMessage;
			break;

		case RemarkFlags::WarningMessage:
			cTextColor = _ORANGE;
			strRemark += strMessage;
			break;

		case RemarkFlags::InfoMessage:
			cTextColor = _BLACK;
			strRemark += strMessage;
			break;
	}

	// Preserve alignment style and text color.
	long lPreviousStyle = m_pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign );
	long lPreviousTextColor = m_pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor );

	// Span the cell and set the static text.
	m_pclSSheet->AddCellSpanW( lMergeColFrom, lRow, lMergeColTo - lMergeColFrom, 1 );

	// Set style, color and text.
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)cTextColor );
	m_pclSSheet->SetStaticText( lMergeColFrom, lRow, strRemark );

	// Reset style and color.
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)lPreviousStyle );
	m_pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)lPreviousTextColor );
}

CViewDescription::~CViewDescription()
{
	Reset();
}

void CViewDescription::Reset( void )
{
	if( m_SheetsDescriptionList.size() > 0 )
	{
		for( mapSheetDescriptionIter mapIter = m_SheetsDescriptionList.begin(); mapIter != m_SheetsDescriptionList.end(); ++mapIter )
		{
			if( NULL != mapIter->second )
				delete mapIter->second;
		}
	}
	m_SheetsDescriptionList.clear();
	m_vecSheetOrder.clear();
}

CSheetDescription* CViewDescription::AddSheetDescription( UINT uiSheetDescriptionID, UINT uiAfterSheetDescriptionID, CSSheet *pclSheet, CPoint m_ptOffset )
{
	CSheetDescription* pclSheetDescription = NULL;
	
	// If ID doesn't yet exist...
	if( false == IsSheetDescriptionExist( uiSheetDescriptionID ) )
	{
		pclSheetDescription = new CSheetDescription( uiSheetDescriptionID, pclSheet, m_ptOffset, this );
		if( NULL != pclSheetDescription )
		{
			m_SheetsDescriptionList[uiSheetDescriptionID] = pclSheetDescription;

			if( -1 != uiAfterSheetDescriptionID && true == IsSheetDescriptionExist( uiAfterSheetDescriptionID ) )
			{
				vecSheetOrderIter vecIter = std::find( m_vecSheetOrder.begin(), m_vecSheetOrder.end(), uiAfterSheetDescriptionID );
				if( vecIter != m_vecSheetOrder.end() && ( ( vecIter + 1 ) != m_vecSheetOrder.end() ) )
					m_vecSheetOrder.insert( vecIter + 1, uiSheetDescriptionID );
				else
					m_vecSheetOrder.push_back( uiSheetDescriptionID );
			}
			else
			{
				m_vecSheetOrder.push_back( uiSheetDescriptionID );
			}
		}
	}
	
	return pclSheetDescription;
}

bool CViewDescription::IsSheetDescriptionExist( UINT uiSheetDescriptionID )
{
	return ( ( 0 == m_SheetsDescriptionList.count( uiSheetDescriptionID ) ) ? false : true );
}

bool CViewDescription::RemoveOneSheetDescription( UINT uiSheetDescriptionID )
{
	bool fReturn = false;
	mapSheetDescriptionIter mapIter = m_SheetsDescriptionList.find( uiSheetDescriptionID );
	if( mapIter != m_SheetsDescriptionList.end() )
	{
		if( NULL != mapIter->second )
			delete mapIter->second;
		m_SheetsDescriptionList.erase( mapIter );
		
		vecSheetOrderIter vecIter = std::find( m_vecSheetOrder.begin(), m_vecSheetOrder.end(), uiSheetDescriptionID );
		if( vecIter != m_vecSheetOrder.end() )
			m_vecSheetOrder.erase( vecIter );
		
		fReturn = true;
	}
	return fReturn;
}

void CViewDescription::RemoveAllSheetDescription( bool fAlsoFirstOne )
{
	if( 0 == m_SheetsDescriptionList.size() )
		return;

	mapSheetDescriptionIter mapIter, mapNextIter;
	UINT uiFirstSheet = -1;
	if( false == fAlsoFirstOne )
	{
		// We must before determined which is the first sheet in terms of y position (the upper one).
		long lUpperYOffset = LONG_MAX;
		for( mapIter = m_SheetsDescriptionList.begin(); mapIter != m_SheetsDescriptionList.end(); ++mapIter )
		{
			long lYOffset = mapIter->second->GetSSheetOffset().y;
			if( -1 != lYOffset && lYOffset < lUpperYOffset )
			{
				lUpperYOffset = lYOffset;
				uiFirstSheet = mapIter->first;
			}
		}
	}
	
	// Must run all defined sheets.
	mapIter = m_SheetsDescriptionList.begin();
	while( mapIter != m_SheetsDescriptionList.end() )
	{
		// If first one 
		if( uiFirstSheet != -1 && mapIter->first == uiFirstSheet )
			++mapIter;
		else
		{
			mapNextIter = mapIter;
			++mapNextIter;

			UINT uiSheetDescriptionID = -1;
			if( NULL != mapIter->second )
			{
				uiSheetDescriptionID = mapIter->second->GetSheetDescriptionID();
				delete mapIter->second;
			}
			m_SheetsDescriptionList.erase( mapIter );
			
			vecSheetOrderIter vecIter = std::find( m_vecSheetOrder.begin(), m_vecSheetOrder.end(), uiSheetDescriptionID );
			if( vecIter != m_vecSheetOrder.end() )
				m_vecSheetOrder.erase( vecIter );

			mapIter = mapNextIter;
		}
	}
}

void CViewDescription::RemoveAllSheetAfter( UINT uiSheetDescriptionID, bool fSheetIncluded )
{
	vecSheetOrderIter vecIter = std::find( m_vecSheetOrder.begin(), m_vecSheetOrder.end(), uiSheetDescriptionID );
	if( vecIter == m_vecSheetOrder.end() )
		return;

	CSheetDescription* pclCurrentSheetDescription = GetFromSheetDescriptionID( uiSheetDescriptionID );
	if( NULL == pclCurrentSheetDescription )
		return;

	if( false == fSheetIncluded )
		++vecIter;
	
	vecSheetOrderIter vecFirstIter = vecIter;
	while( vecIter != m_vecSheetOrder.end() )
	{
		mapSheetDescriptionIter mapIter = m_SheetsDescriptionList.find( *vecIter );
		if( mapIter != m_SheetsDescriptionList.end() )
		{
			if( NULL != mapIter->second )
				delete mapIter->second;
			m_SheetsDescriptionList.erase( mapIter );
		}
		++vecIter;
	}
	if( vecFirstIter != m_vecSheetOrder.end() )
	{
		m_vecSheetOrder.erase( vecFirstIter, m_vecSheetOrder.begin() + m_vecSheetOrder.size() );
	}
}

int CViewDescription::GetSheetNumber( void )
{
	return m_SheetsDescriptionList.size();
}

void CViewDescription::GetNextSelectableRow( CSheetDescription *pclSheetDescription, long lColumn, long lRow, bool fExcludeHidden, long& lSelectableRow, CSheetDescription*& pclNewSheetDescription )
{
	// Sanity check
	if( NULL == pclSheetDescription )
		return;

	// By default row is found in the same sheet.
	pclNewSheetDescription = pclSheetDescription;

	// Check first in current sheet if there is at least one next row.
	lSelectableRow = pclSheetDescription->GetNextSelectableRow( lColumn, lRow, fExcludeHidden );
	
	// If no more row...
	if( -1 == lSelectableRow )
	{
		// Try to run all sheets below.
		pclNewSheetDescription = GetNextSheetDescription( pclSheetDescription );
		while( NULL != pclNewSheetDescription && -1 == lSelectableRow )
		{
			lSelectableRow = pclNewSheetDescription->GetFirstSelectableRow( fExcludeHidden );
			if( lSelectableRow != - 1 )
				break;
			else
				pclNewSheetDescription = GetNextSheetDescription( pclNewSheetDescription );
		}

		// To be clean!
		if( -1 == lSelectableRow )
			pclNewSheetDescription = NULL;
	}
}

void CViewDescription::GetPrevSelectableRow( CSheetDescription *pclSheetDescription, long lColumn, long lRow, bool fExcludeHidden, long& lSelectableRow, CSheetDescription*& pclNewSheetDescription )
{
	// Sanity check
	if( NULL == pclSheetDescription )
		return;

	// By default row is found in the same sheet.
	pclNewSheetDescription = pclSheetDescription;

	// Check first in current sheet if there is at least one previous row.
	lSelectableRow = pclSheetDescription->GetPrevSelectableRow( lColumn, lRow, fExcludeHidden );
	
	// If no more row...
	if( -1 == lSelectableRow )
	{
		// Try to run all sheets above.
		pclNewSheetDescription = GetPrevSheetDescription( pclSheetDescription );
		while( NULL != pclNewSheetDescription && -1 == lSelectableRow )
		{
			lSelectableRow = pclNewSheetDescription->GetLastSelectableRow( fExcludeHidden );
			if( lSelectableRow != - 1 )
				break;
			else
				pclNewSheetDescription = GetPrevSheetDescription( pclNewSheetDescription );
		}

		// To be clean!
		if( -1 == lSelectableRow )
			pclNewSheetDescription = NULL;
	}
}

void CViewDescription::GetSheetDescriptionList( vecSheetDescription& vecSheetDescriptionList )
{
	vecSheetDescriptionList.clear();
	for( vecSheetOrderIter vecIter = m_vecSheetOrder.begin(); vecIter != m_vecSheetOrder.end(); ++vecIter )
	{
		if( 0 != m_SheetsDescriptionList.count( *vecIter ) )
			vecSheetDescriptionList.push_back( m_SheetsDescriptionList[*vecIter] );
	}
}

CSheetDescription* CViewDescription::GetTopSheetDescription( void )
{
	CSheetDescription* pclTopSheetDescription = NULL;
	if( m_SheetsDescriptionList.size() > 0 && 0 != m_SheetsDescriptionList.count( *( m_vecSheetOrder.begin() ) ) )
	{
		pclTopSheetDescription = m_SheetsDescriptionList[*( m_vecSheetOrder.begin() )];
	}
	return pclTopSheetDescription;
}

CSheetDescription* CViewDescription::GetNextSheetDescription( CSheetDescription* pclCurrentSheetDescription, UINT uiParentID, UINT uiGroupID )
{
	CSheetDescription* pclNextSheetDescription = NULL;
	
	if( NULL != pclCurrentSheetDescription )
	{
		vecSheetOrderIter vecIter = std::find( m_vecSheetOrder.begin(), m_vecSheetOrder.end(), pclCurrentSheetDescription->GetSheetDescriptionID() );
		
		if( vecIter != m_vecSheetOrder.end() && ( ( vecIter + 1 ) != m_vecSheetOrder.end() ) )
		{
			if( 0 != m_SheetsDescriptionList.count( *( vecIter + 1 ) ) )
			{
				pclNextSheetDescription = m_SheetsDescriptionList[*( vecIter + 1 )];
			}
		}
	}

	// Check now with parent and group ID if defined.
	if( NULL != pclNextSheetDescription && ( -1 != uiParentID || -1 != uiGroupID ) )
	{
		if( -1 != uiParentID && uiParentID != pclNextSheetDescription->GetParentSheetDescriptionID() )
		{
			// Not the same parent.
			pclNextSheetDescription = NULL;
		}
		else if( -1 != uiGroupID && uiGroupID != pclNextSheetDescription->GetGroupSheetDescriptionID() )
		{
			// Not the same group.
			pclNextSheetDescription = NULL;
		}
	}

	return pclNextSheetDescription;
}

CSheetDescription* CViewDescription::GetPrevSheetDescription( CSheetDescription* pclCurrentSheetDescription )
{
	CSheetDescription* pclPrevSheetDescription = NULL;
	if( NULL != pclCurrentSheetDescription )
	{
		vecSheetOrderIter vecIter = std::find( m_vecSheetOrder.begin(), m_vecSheetOrder.end(), pclCurrentSheetDescription->GetSheetDescriptionID() );
		if( vecIter != m_vecSheetOrder.end() && vecIter != m_vecSheetOrder.begin() )
		{
			if( 0 != m_SheetsDescriptionList.count( *( vecIter - 1 ) ) )
				pclPrevSheetDescription = m_SheetsDescriptionList[*( vecIter - 1 )];
		}
	}
	return pclPrevSheetDescription;
}

CSheetDescription* CViewDescription::GetBottomSheetDescription( void )
{
	CSheetDescription* pclBottomSheetDescription = NULL;
	if( m_SheetsDescriptionList.size() > 0 && 0 != m_SheetsDescriptionList.count( m_vecSheetOrder.back() ) )
		pclBottomSheetDescription = m_SheetsDescriptionList[m_vecSheetOrder.back()];
	return pclBottomSheetDescription;
}

int CViewDescription::CompareTwoSDPosition( CSheetDescription* pclSDToCompare, CSheetDescription* pclSDComparedTo )
{
	if( NULL == pclSDToCompare || NULL == pclSDComparedTo )
		return CompareSD_Error;

	if( pclSDToCompare == pclSDComparedTo || pclSDToCompare->GetSSheetOffset().y == pclSDComparedTo->GetSSheetOffset().y )
		return CompareSD_FirstEqualSecond;
	
	return ( pclSDToCompare->GetSSheetOffset().y < pclSDComparedTo->GetSSheetOffset().y ) ? CompareSD_FirstAboveSecond : CompareSD_FirstBelowSecond;
}

CSheetDescription* CViewDescription::GetFromSheetDescriptionID( UINT uiSheetDescriptionID )
{
	if( true == IsSheetDescriptionExist( uiSheetDescriptionID ) )
		return m_SheetsDescriptionList[ uiSheetDescriptionID ];
	else
		return NULL;
}

CSheetDescription* CViewDescription::GetFromSSheetID( UINT uiSSheetID )
{
	CSheetDescription* pclSheetDescription = NULL;
	if( m_SheetsDescriptionList.size() > 0 )
	{
		pclSheetDescription = GetTopSheetDescription();
		while( NULL != pclSheetDescription )
		{
			if( uiSSheetID == pclSheetDescription->GetSSheetID() )
				break;
			else
				pclSheetDescription = GetNextSheetDescription( pclSheetDescription );
		}
	}
	return pclSheetDescription;
}

void CViewDescription::OnMouseEvent( CSSheet* pclSSheet, int iMouseEventType, long lColumn, long lRow, CPoint ptMouse )
{
	bool fStop = false;
	CSheetDescription* pclSheetDescription = GetTopSheetDescription();
	while( NULL != pclSheetDescription && false == fStop )
	{
		if( pclSSheet == pclSheetDescription->GetSSheetPointer() )
		{
			pclSheetDescription->OnMouseEvent( iMouseEventType, lColumn, lRow, ptMouse );
			fStop = true;
		}
		else
			pclSheetDescription = GetNextSheetDescription( pclSheetDescription );
	}
}
