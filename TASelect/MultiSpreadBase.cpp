#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "SSheet.h"
#include "RViewDescription.h"
#include "MultiSpreadBase.h"

CMultiSpreadBase::CMultiSpreadBase( bool bUseOnlyOneSpread )
	: CMessageManager( CMessageManager::ClassID::CID_CMultiSpreadBase )
{
	m_bInitialised = false;
	m_rectVirtualSize.SetRectEmpty();
	InitializeCriticalSection( &m_CriticalSection );
	m_bScrollbarRunning = false;
	m_iFreezeTopHeightPixels = 0;
	m_bUseOnlyOneSpread = bUseOnlyOneSpread;
}

CMultiSpreadBase::~CMultiSpreadBase()
{
	if( m_ViewDescription.GetSheetNumber() > 0 )
	{
		CViewDescription::vecSheetDescription vecSheetDescriptionList;
		m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );
	
		for( CViewDescription::vecSheetDescriptionIter iter = vecSheetDescriptionList.begin(); iter != vecSheetDescriptionList.end(); iter++ )
		{
			CSSheet *pclSSheet = (*iter)->GetSSheetPointer();

			if( NULL != pclSSheet )
			{
				delete pclSSheet;
				(*iter)->SetSSheetPointer( NULL );
			}
		}

		m_ViewDescription.RemoveAllSheetDescription( true );
	}
}

void CMultiSpreadBase::Reset( void )
{
	m_bInitialised = false;

	if( NULL == GetpWnd() )
	{
		return;
	}

	// Reset scrolling.
	// Don't use 'ScrollWindow'! I don't know why but when calling 'ScrollWindow' scroll infos are not refreshed.
	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	GetpWnd()->SetScrollInfo( SB_HORZ, &rScrollInfo );
	GetpWnd()->SetScrollInfo( SB_VERT, &rScrollInfo );
	
	if( m_ViewDescription.GetSheetNumber() > 0 )
	{
		CViewDescription::vecSheetDescription vecSheetDescriptionList;
		m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );

		for( CViewDescription::vecSheetDescriptionIter iter = vecSheetDescriptionList.begin(); iter != vecSheetDescriptionList.end(); iter++ )
		{
			CSSheet *pclSSheet = (*iter)->GetSSheetPointer();

			if( NULL != pclSSheet )
			{
				delete pclSSheet;
				(*iter)->SetSSheetPointer( NULL );
			}
		}

		m_ViewDescription.RemoveAllSheetDescription( true );
	}

	m_rectVirtualSize.SetRectEmpty();
	m_iFreezeTopHeightPixels = 0;
}

CSSheet *CMultiSpreadBase::GetSheetPointer( void )
{
	CSheetDescription*pclSheetDescription = m_ViewDescription.GetTopSheetDescription();

	if( NULL == pclSheetDescription )
	{
		return NULL;
	}

	return pclSheetDescription->GetSSheetPointer();
}

void CMultiSpreadBase::GetScrollingPos( int &iVertPos, int &iHorzPos )
{
	iHorzPos = 0;
	iVertPos = 0;

	if( NULL == GetpWnd() )
	{
		return;
	}

	if( true == IsScrollingExist( ScrollType::ST_Horz ) )
	{
		iHorzPos = GetpWnd()->GetScrollPos( SB_HORZ );
	}

	if( true == IsScrollingExist( ScrollType::ST_Vert ) )
	{
		iVertPos = GetpWnd()->GetScrollPos( SB_VERT );
	}
}

bool CMultiSpreadBase::IsScrollingExist( ScrollType eScrollType )
{
	SCROLLBARINFO rScrollBarInfo;
	ZeroMemory( &rScrollBarInfo, sizeof( rScrollBarInfo ) );
	rScrollBarInfo.cbSize = sizeof( rScrollBarInfo );
	GetpWnd()->GetScrollBarInfo( ( ScrollType::ST_Horz == eScrollType ) ? OBJID_HSCROLL : OBJID_VSCROLL, &rScrollBarInfo );
	return ( 0 == rScrollBarInfo.rgstate[0] ) ? true : false;
}

void CMultiSpreadBase::GetInfoUnderMouse( CPoint ptMousePosScreen, CSheetDescription *&pclSheetDescription, long &lColumn, long &lRow )
{
	pclSheetDescription = NULL;
	lColumn = 0;
	lRow = 0;

	// If at least one sheet is defined...
	if( m_ViewDescription.GetSheetNumber() > 0 )
	{
		// Run all sheet descriptions.
		CViewDescription::vecSheetDescription vecSheetDescriptionList;
		m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );
		bool bStop = false;

		for( int iLoopSheet = 0; iLoopSheet < (int)vecSheetDescriptionList.size() && false == bStop; iLoopSheet++ )
		{
			pclSheetDescription = vecSheetDescriptionList[iLoopSheet];
			CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

			if( NULL == pclSSheet )
			{
				continue;
			}

			if( false == pclSheetDescription->GetShowFlag() )
			{
				continue;
			}

			// Retrieve coordinates from current sheet relative to the current window.
			CRect rectSSheetSizeScreen;
			pclSSheet->GetClientRect( &rectSSheetSizeScreen );
			pclSSheet->ClientToScreen( &rectSSheetSizeScreen );

			// If click occurs in this sheet...
			if( TRUE == rectSSheetSizeScreen.PtInRect( ptMousePosScreen ) )
			{
				// Convert mouse position relative to sheet.
				CPoint ptMousePosSSheet = ptMousePosScreen;
				pclSSheet->ScreenToClient( &ptMousePosSSheet );

				// Determine col and row.
				pclSSheet->GetCellFromPixel( &lColumn, &lRow, ptMousePosSSheet.x, ptMousePosSSheet.y );
				bStop = true;
			}
		}

		if( false == bStop )
		{
			pclSheetDescription = NULL;
		}
	}
}

void CMultiSpreadBase::CopyMultiSheetInSheet( CSSheet *pclSheetDst )
{
	if( true == IsEmpty() || NULL == pclSheetDst )
	{
		return;
	}

	pclSheetDst->SetMaxCols( 0 );
	pclSheetDst->SetMaxRows( 0 );
	long lMaxWidth = 0;
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetTopSheetDescription();

	while( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
	{
		CSSheet *pclSheetSrc = pclSheetDescription->GetSSheetPointer();

		// Loop to memorize column width.
		double dTotalWidth = 0.0;
		double *parColWidth = (double*)malloc( pclSheetSrc->GetMaxCols() * sizeof( double ) );

		if( NULL == parColWidth )
		{
			continue;
		}

		for( int iLoopCol = 1; iLoopCol <= pclSheetSrc->GetMaxCols(); iLoopCol++ )
		{
			double dWidth;
			pclSheetSrc->GetColWidth( iLoopCol, &dWidth );
			parColWidth[iLoopCol - 1] = dWidth;
			dTotalWidth += dWidth;
		}

		// Set the max cols.
		long lTotalWidth = (long)ceil( dTotalWidth );
		
		if( lTotalWidth > lMaxWidth )
		{
			long lPrevCols = pclSheetDst->GetMaxCols();
			pclSheetDst->SetMaxCols( lTotalWidth );

			// Change column destination width.
			for( long lLoopCol = lPrevCols + 1; lLoopCol <= lTotalWidth; lLoopCol++ )
			{
				pclSheetDst->SetColWidth( lLoopCol, 1.5 );
			}

			lMaxWidth = lTotalWidth;
		}

		// Set the max rows.
		long lStartDstRow = pclSheetDst->GetMaxRows() + 1;
		pclSheetDst->SetMaxRows( pclSheetDst->GetMaxRows() + pclSheetSrc->GetMaxRows() );

		// Prepare a map for optimization.
		// std::map<long, std::map<long, long>> mapColWidth;
		long *parOptimisation = (long*)malloc( pclSheetSrc->GetMaxCols() * pclSheetSrc->GetMaxCols() * sizeof( long ) );

		if( NULL == parOptimisation )
		{
			free( parColWidth );
			continue;
		}
		
		long lMemNumCol = 0;
		double dTotalDecimalPart = 0.0;
		int lNumCol = 1;
		long lWidth = 0;
		lTotalWidth = 0;

		for( long lLoopCol = 1; lLoopCol <= pclSheetSrc->GetMaxCols(); lLoopCol++ )
		{
			double dWidth = parColWidth[lLoopCol - 1];
			dTotalDecimalPart += ( dWidth - floor( dWidth ) );

			if( dTotalDecimalPart > 1.0 )
			{
				for( long lLast = lMemNumCol; lLast < lNumCol; lLast++ )
				{
					parOptimisation[lLast - 1] += 1;
				}

				lTotalWidth += 1;
				dTotalDecimalPart -= 1.0;
			}

			lTotalWidth += (long)dWidth;
			parOptimisation[lNumCol - 1] = lTotalWidth;
			
			if( 0.0 != dWidth )
			{
				lMemNumCol = lNumCol;
			}

			lNumCol++;
		}

		lNumCol--;

		if( lTotalWidth < lMaxWidth || 0.0 != dTotalDecimalPart )
		{
			if( 0 == lMemNumCol )
			{
				parOptimisation[lNumCol - 1] = lMaxWidth;
			}
			else
			{
				for( long lLast = lMemNumCol; lLast <= lNumCol; lLast++ )
				{
					parOptimisation[lLast - 1] = lMaxWidth;
				}
			}
		}

		// Fill rest of map.
		if( pclSheetSrc->GetMaxCols() > 1 )
		{
			for( long lLoopCol = 2; lLoopCol <= pclSheetSrc->GetMaxCols(); lLoopCol++ )
			{
				long lShift = ( lLoopCol - 1 ) * pclSheetSrc->GetMaxCols();
				lNumCol = 1;
				long lPrevWidth = 0;
				
				for( long lLoopNumCol = lLoopCol; lLoopNumCol <= pclSheetSrc->GetMaxCols(); lLoopNumCol++ )
				{
					parOptimisation[lShift + lNumCol - 1] = parOptimisation[lLoopNumCol - 1] - parOptimisation[lLoopNumCol - 2] + lPrevWidth;
					lPrevWidth = parOptimisation[lShift + lNumCol - 1];
					lNumCol++;
				}
			}
		}

		long lLoopSrcRow = 1;
		long lLoopDestRow = lStartDstRow;

		while( lLoopSrcRow <= pclSheetSrc->GetMaxRows() )
		{
			long lCurrentColDst = 1;
			long lLoopColSrc = 1;
			
			while( lLoopColSrc <= pclSheetSrc->GetMaxCols() )
			{
				long lShift = (lLoopColSrc - 1) * pclSheetSrc->GetMaxCols();
				bool bByPass = false;

				if( TRUE == pclSheetSrc->IsColHidden( lLoopColSrc ) || 0.0 == pclSheetSrc->GetColWidthW( lLoopColSrc ) )
				{
					bByPass = true;
				}

				long lNumCols = 1;
				long lNumRows = 1;

				if( false == bByPass )
				{
					// Spans.
					SS_COORD lColAnchor, lRowAnchor;
					WORD wReturn = pclSheetSrc->GetCellSpan( lLoopColSrc, lLoopSrcRow, &lColAnchor, &lRowAnchor, &lNumCols, &lNumRows );

					if( SS_SPAN_NO != wReturn && lLoopSrcRow > lRowAnchor )
					{
						bByPass = true;
					}
				}

				int iSpanWidth = parOptimisation[lShift + lNumCols - 1];
				
				if( false == bByPass )
				{
					pclSheetDst->AddCellSpan( lCurrentColDst, lLoopDestRow, iSpanWidth, lNumRows );

					// Colors.
					COLORREF backColor, foreColor;
					pclSheetSrc->GetColor( lLoopColSrc, lLoopSrcRow, &backColor, &foreColor );
					pclSheetDst->SetColor( lCurrentColDst, lLoopDestRow, backColor, foreColor );

					// Fonts.
					HFONT hFont = pclSheetSrc->GetFont( lLoopColSrc, lLoopSrcRow );
					pclSheetDst->SetFont( lCurrentColDst, lLoopDestRow, hFont, FALSE );

					// Types and styles.
					SS_CELLTYPE rCellType;
					pclSheetSrc->GetCellType( lLoopColSrc, lLoopSrcRow, &rCellType );
					pclSheetDst->SetCellType( lCurrentColDst, lLoopDestRow, &rCellType );

					// Data.
					int iDataLen = pclSheetSrc->GetDataLen( lLoopColSrc, lLoopSrcRow );

					if( iDataLen > 0 )
					{
						TCHAR* ptBuffer = new TCHAR[iDataLen + 1];
						pclSheetSrc->GetData( lLoopColSrc, lLoopSrcRow, ptBuffer );
						pclSheetDst->SetData( lCurrentColDst, lLoopDestRow, ptBuffer );
						delete[] ptBuffer;
					}

					pclSheetDst->AddCellSpan( lCurrentColDst, lLoopDestRow, iSpanWidth, lNumRows );
				}

				// Borders.
				WORD wBorderType, wBorderStyle;
				COLORREF borderColor;
				pclSheetSrc->GetBorder( lLoopColSrc, lLoopSrcRow, &wBorderType, &wBorderStyle, &borderColor );
				
				for( int iLoopColBorder = lCurrentColDst; iLoopColBorder < lCurrentColDst + iSpanWidth; iLoopColBorder++ )
				{
					pclSheetDst->SetBorder( iLoopColBorder, lLoopDestRow, wBorderType, wBorderStyle, borderColor );
				}

				lLoopColSrc += lNumCols;
				lCurrentColDst += iSpanWidth;
			}

			// Row height.
			double dRowHeight;
			pclSheetSrc->GetRowHeight( lLoopSrcRow, &dRowHeight );
			pclSheetDst->SetRowHeight( lLoopDestRow, dRowHeight );

			// Row user data.
			// Remark: where is saved for example the flag 'no breakable'.
			long lUserData;
			pclSheetSrc->GetRowUserData( lLoopSrcRow, (LONG_PTR*)&lUserData );
			pclSheetDst->SetRowUserData( lLoopDestRow, lUserData );

			lLoopSrcRow++;
			lLoopDestRow++;
		}

		free( parColWidth );
		free( parOptimisation );

		pclSheetDescription = m_ViewDescription.GetNextSheetDescription( pclSheetDescription );
	}
}

bool CMultiSpreadBase::IsEmpty( void )
{
	return ( 0 == m_ViewDescription.GetSheetNumber() ? true : false );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected members
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMultiSpreadBase::BasePreTranslateMessage( MSG *pMsg )
{
	if( WM_LBUTTONDOWN == pMsg->message )
	{
	}
}

void CMultiSpreadBase::BaseOnSize( UINT nType, int cx, int cy )
{
	if( true == m_bUseOnlyOneSpread )
		return;

	_CheckScrollbars();
}

void CMultiSpreadBase::BaseOnHScroll( CWnd *pWnd, UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
{
	if( NULL == pWnd || true == m_bUseOnlyOneSpread )
	{
		return;
	}

	// By security!! To be sure to not execute ourself this method is there is no scrollbar.
	if( false == IsScrollingExist( ScrollType::ST_Horz ) )
	{
		return;
	}

	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	pWnd->GetScrollInfo( SB_HORZ, &rScrollInfo );

	// Retrieve current position
	int nCurrentPos = pWnd->GetScrollPos( SBS_HORZ );
	int nPreviousPos = nCurrentPos;
	
	switch(nSBCode)
	{
		// User has clicked on arrow left.
		case SB_LINELEFT:
			nCurrentPos = max( nCurrentPos - 10, 0);
			break;

		// User has clicked on arrow right.
		case SB_LINERIGHT:
			{
				// TO REMOVE
				CRect rectClient;
				pWnd->GetClientRect( &rectClient );
				CSheetDescription *pclLastSheetDescription = m_ViewDescription.GetBottomSheetDescription();

				if( NULL != pclLastSheetDescription && NULL != pclLastSheetDescription->GetSSheetPointer() )
				{
					CRect rectSheet;
					pclLastSheetDescription->GetSSheetPointer()->GetClientRect( &rectSheet );
					int i = 0;
				}
			}
			nCurrentPos = min( nCurrentPos + 10, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// User has clicked on bar between arrow left and scroll box
		case SB_PAGELEFT:
			nCurrentPos = max( nCurrentPos - (int)rScrollInfo.nPage, 0);
			break;

		// User has clicked on bar between arrow right and scroll box
		case SB_PAGERIGHT:
			nCurrentPos = min( nCurrentPos + (int)rScrollInfo.nPage, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// First one is when user drag the scroll box.
		// Second one is when user release the scroll box.
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			// Pay attention: 'nPos' is not really a UINT but a 16 bits value! To get position on 32 bits, we must use 'nTrackPos'.
			nCurrentPos = rScrollInfo.nTrackPos;
			break;
	}

	pWnd->SetScrollPos( SB_HORZ, nCurrentPos );
	pWnd->ScrollWindow( nPreviousPos - nCurrentPos, 0 );
}

void CMultiSpreadBase::BaseOnVScroll( CWnd *pWnd, UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
{
	if( NULL == pWnd || true == m_bUseOnlyOneSpread )
	{
		return;
	}

	// By security!! To be sure to not execute ourself this method is there is no scrollbar.
	if( false == IsScrollingExist( ScrollType::ST_Vert ) )
	{
		return;
	}

	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	pWnd->GetScrollInfo( SB_VERT, &rScrollInfo );

	// Retrieve current position
	int nCurrentPos = pWnd->GetScrollPos( SBS_VERT );

	int nPreviousPos = nCurrentPos;
	
	switch( nSBCode )
	{
		// User has clicked on arrow up.
		case SB_LINEUP:
			nCurrentPos = max( nCurrentPos - 20, 0);
			break;

		// User has clicked on arrow down.
		case SB_LINEDOWN:
			nCurrentPos = min( nCurrentPos + 20, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// User has clicked on bar between arrow up and scroll box
		case SB_PAGEUP:
			nCurrentPos = max( nCurrentPos - (int)rScrollInfo.nPage, 0);
			break;

		// User has clicked on bar between arrow down and scroll box
		case SB_PAGEDOWN:
			nCurrentPos = min( nCurrentPos + (int)rScrollInfo.nPage, rScrollInfo.nMax - (int)rScrollInfo.nPage );
			break;

		// First one is when user drag the scroll box.
		// Second one is when user release the scroll box.
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			// Pay attention: 'nPos' is not really a UINT but a 16 bits value! To get position on 32 bits, we must use 'nTrackPos'.
			nCurrentPos = rScrollInfo.nTrackPos;
			break;

		case SB_ENDSCROLL:
			break;
	}

	if( nCurrentPos != nPreviousPos )
	{
		pWnd->SetScrollPos( SB_VERT, nCurrentPos );

// 		CRect rectToScroll;
// 		pWnd->GetClientRect( &rectToScroll );
// 		rectToScroll.top += m_iFreezeTopHeightPixels;
// 		pWnd->ScrollWindow( 0, nPreviousPos - nCurrentPos, NULL, &rectToScroll );
		pWnd->ScrollWindow( 0, nPreviousPos - nCurrentPos );
	}
}

LRESULT CMultiSpreadBase::BaseMM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
{
	if( NULL == GetpWnd() )
	{
		return 0;
	}

	CMessageManager::CMessage *pclMessage = CMessageManager::MM_ReadMessage( wParam );

	if( NULL == pclMessage )
	{
		return 0;
	}
	
	if( CMessageManager::ClassID::CID_CSSheet != (CMessageManager::ClassID)pclMessage->GetClassID() )
	{
		delete pclMessage;
		return 0;
	}

	switch( (CMessageManager::SSheetMessageType)pclMessage->GetMessageType() )
	{
		case CMessageManager::SSheetMessageType::SSheetMST_MouseEvent:
			{
				CMessageManager::CSSheetMEMsg *pclSShetMEMsg = dynamic_cast<CMessageManager::CSSheetMEMsg *>( pclMessage );

				if( NULL == pclSShetMEMsg )
				{
					delete pclMessage;
					return 0;
				}

				CMessageManager::SSheetMEParams *prParams = pclSShetMEMsg->GetParams();
				CSheetDescription *pclSheetDescription = NULL;

				if( NULL == prParams->m_hSheet || false == _IsSheetExist( prParams->m_hSheet, pclSheetDescription ) || NULL == pclSheetDescription )
				{
					delete pclMessage;
					return 0;
				}

				switch( prParams->m_eMouseEventType )
				{
					case CMessageManager::SSheetMET::MET_LButtonDown:
						OnCellClicked( pclSheetDescription, prParams->m_lColumn, prParams->m_lRow );
						break;

					case CMessageManager::SSheetMET::MET_LButtonDblClk:
						OnCellDblClicked( pclSheetDescription, prParams->m_lColumn, prParams->m_lRow );
						break;

					case CMessageManager::SSheetMET::MET_RButtonDown:
						OnRightClick( pclSheetDescription, prParams->m_lColumn, prParams->m_lRow, prParams->m_ptMouse.x, prParams->m_ptMouse.y );
						break;

					case CMessageManager::SSheetMET::MET_MouseWheel:
						if( false == m_bUseOnlyOneSpread && true == IsScrollingExist( ScrollType::ST_Vert ) )
						{
							SCROLLINFO rScrollInfo;
							ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
							rScrollInfo.cbSize = sizeof( SCROLLINFO );
							rScrollInfo.fMask = SIF_ALL;

							if( TRUE == GetpWnd()->GetScrollInfo( SB_VERT, &rScrollInfo ) )
							{
								int iCurrentPos = GetpWnd()->GetScrollPos( SBS_VERT );
								int iPreviousPos = iCurrentPos;

								if( prParams->m_iDelta < 0 )
								{
									iCurrentPos = min( iCurrentPos - prParams->m_iDelta, rScrollInfo.nMax - (int)rScrollInfo.nPage );
								}
								else
								{
									iCurrentPos = max( iCurrentPos - prParams->m_iDelta, 0 );
								}

								GetpWnd()->SetScrollPos( SB_VERT, iCurrentPos );
								GetpWnd()->ScrollWindow( 0, iPreviousPos - iCurrentPos );
							}
						}
						break;
				}

				// Send manually to 'CViewDescription' for the class 'CResizingColumnInfo'. Because at now, 'MM_OnMessageReceive' messages are only sent to a 'CWnd'
				// and 'CResizingColumnInfo' is not.
				m_ViewDescription.OnMouseEvent( pclSheetDescription->GetSSheetPointer(), prParams->m_eMouseEventType, prParams->m_lColumn, prParams->m_lRow, prParams->m_ptMouse );
			}
			break;

		case CMessageManager::SSheetMessageType::SSheetMST_KeyboardEvent:
			{
				CMessageManager::CSSheetKEMsg *pclSShetKEMsg = dynamic_cast<CMessageManager::CSSheetKEMsg *>( pclMessage );

				if( NULL == pclSShetKEMsg )
				{
					delete pclMessage;
					return 0;
				}

				CMessageManager::SSheetKEParams *prParams = pclSShetKEMsg->GetParams();
				CSheetDescription *pclSheetDescription = NULL;

				if( NULL == prParams->m_hSheet || false == _IsSheetExist( prParams->m_hSheet, pclSheetDescription ) )
				{
					delete pclMessage;
					return 0;
				}

				switch( prParams->m_eKeyboardEventType )
				{
					case CMessageManager::SSheetKET::KET_VirtualKeyDown:

						switch( prParams->m_iKeyCode )
						{
							// Page up/down.
							case VK_PRIOR:
							case VK_NEXT:

								if( false == m_bUseOnlyOneSpread )
								{
									SetScrollPosition( ( VK_PRIOR == prParams->m_iKeyCode ) ? ScrollPosition::ScrollUpOnePage : ScrollPosition::ScrollDnOnePage );
								}

								break;

							case VK_HOME:
							case VK_END:

								if( false == m_bUseOnlyOneSpread )
								{
									SetScrollPosition( ( VK_HOME == prParams->m_iKeyCode ) ? ScrollPosition::ScrollToTop : ScrollPosition::ScrollToBottom );
								}

								break;

							default:
								break;
						}

						OnKeyDown( pclSheetDescription, (WPARAM)prParams->m_iKeyCode );
						break;

					case CMessageManager::SSheetKET::KET_VirtualKeyUp:
						OnKeyUp( pclSheetDescription, (WPARAM)prParams->m_iKeyCode );
						break;
				}
			}
			break;

		case CMessageManager::SSheetMessageType::SSheetMST_ColumnWidthChanged:
			{
				CMessageManager::CSSheetCWCMsg *pclSShetCWCMsg = dynamic_cast<CMessageManager::CSSheetCWCMsg *>( pclMessage );

				if( NULL == pclSShetCWCMsg )
				{
					delete pclMessage;
					return 0;
				}

				CMessageManager::SSheetCWCParams *prParams = pclSShetCWCMsg->GetParams();
				CSheetDescription *pclSheetDescription = NULL;

				if( NULL == prParams->m_hSheet || false == _IsSheetExist( prParams->m_hSheet, pclSheetDescription ) || NULL == pclSheetDescription )
				{
					delete pclMessage;
					return 0;
				}

				UINT uiSheetDescriptionID = pclSheetDescription->GetSheetDescriptionID();

				if( 0 == m_mapSSheetColumnWidth.count( uiSheetDescriptionID ) )
				{
					delete pclMessage;
					return 0;
				}

				for( mapShortLongIter iter = m_mapSSheetColumnWidth[uiSheetDescriptionID].begin(); iter != m_mapSSheetColumnWidth[uiSheetDescriptionID].end(); iter++ )
				{
					CRect rectPixel = pclSheetDescription->GetSSheetPointer()->GetCellCoordInPixel( iter->first, pclSheetDescription->GetFirstSelectableRow() );
					iter->second = rectPixel.Width();
				}

				SetSheetSize();
			}
			break;

		case CMessageManager::SSheetMessageType::SSheetMST_SizeChanged:
			// SetSheetSize();
			break;
	}

	delete pclMessage;
	return 0;
}

LRESULT CMultiSpreadBase::BaseTextTipFetch( WPARAM wParam, LPARAM lParam )
{
	SS_TEXTTIPFETCH *pTextTipFetch = (SS_TEXTTIPFETCH *)lParam;

	// Identify sheet description by the CSSheet window ID.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSSheetID( (UINT)wParam );

	if( NULL != pclSheetDescription )
	{
		// If hText is not empty, spread takes hText in place of szText!
		pTextTipFetch->hText = NULL;
		OnTextTipFetch( pclSheetDescription, pTextTipFetch->Col, pTextTipFetch->Row, &pTextTipFetch->wMultiLine, &pTextTipFetch->nWidth, pTextTipFetch->szText, &pTextTipFetch->fShow );
	}

	return 0;
}

LRESULT CMultiSpreadBase::BaseLeaveCell( WPARAM wParam, LPARAM lParam )
{
	// When initializing SSheet, we set normal operation mode (see SSheet.cpp).
	// To avoid interference from Spread, we catch 'LeaveCell' event from spread and return always 'TRUE' to force
	// spread to ignore any internal navigation.
	SS_LEAVECELL *pLeaveCell = (SS_LEAVECELL *)lParam;

	// Identify sheet description by the CSSheet window ID.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSSheetID( (UINT)wParam );

	if (NULL != pclSheetDescription)
	{
		return OnLeaveCell( pclSheetDescription, pLeaveCell->ColCurrent, pLeaveCell->RowCurrent, pLeaveCell->ColNew, pLeaveCell->RowNew );
	}

	return TRUE;
}

LRESULT CMultiSpreadBase::BaseRowHeightChange( WPARAM wParam, LPARAM lParam )
{
	SetSheetSize();
	return TRUE;
}

void CMultiSpreadBase::BaseOnUpdateMenuText( CCmdUI *pCmdUI )
{
	// Update the status bar.
	pMainFrame->UpdateMenuToolTip( pCmdUI );
}

bool CMultiSpreadBase::OnLeaveCell( CSheetDescription *pclSheetDescription, long lColumn, long lRow, long lNewColumn, long lNewRow )
{
	if( NULL != pclSheetDescription )
	{
		if( SS_TYPE_COMBOBOX == pclSheetDescription->GetSSheetPointer()->GetCellTypeW( lNewColumn, lNewRow ) )
		{
			return FALSE;
		}
	}

	return TRUE;
}

CSheetDescription*CMultiSpreadBase::CreateSSheet( UINT uiSheetDescriptionID, UINT uiAfterSheetDescriptionID )
{
	if( NULL == GetpWnd() || ( true == m_bUseOnlyOneSpread && 1 == m_ViewDescription.GetSheetNumber() ) )
	{
		return NULL;
	}

	CSheetDescription *pclSheetDescription = NULL;

	// If 'uiSheetDescriptionID' is not already existing yet...
	if( false == m_ViewDescription.IsSheetDescriptionExist( uiSheetDescriptionID ) )
	{
		_UpdateVirtualSizeAndYOffset();
		int iYOffset = 0;

		// Check where to insert this new sheet.
		bool bOffsetFollowing = false;
		CSheetDescription *pclAfterThisSheetDescription = NULL;
		CSheetDescription *pclNextSheetDescription = NULL;

		if( 0xFFFFFFFF != uiAfterSheetDescriptionID && true == m_ViewDescription.IsSheetDescriptionExist( uiAfterSheetDescriptionID ) )
		{
			pclAfterThisSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( uiAfterSheetDescriptionID );

			if( NULL != pclAfterThisSheetDescription )
			{
				pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclAfterThisSheetDescription );
			}

			bOffsetFollowing = true;
		}
		else
		{
			pclAfterThisSheetDescription = m_ViewDescription.GetBottomSheetDescription();

			if( NULL != pclAfterThisSheetDescription )
			{
				uiAfterSheetDescriptionID = pclAfterThisSheetDescription->GetSheetDescriptionID();
			}
		}

		if( NULL != pclAfterThisSheetDescription && NULL != pclAfterThisSheetDescription->GetSSheetPointer() )
		{
			CSSheet *pclAfterThisSSheet = pclAfterThisSheetDescription->GetSSheetPointer();
			CRect SSheetRect = pclAfterThisSSheet->GetSheetSizeInPixels();
			iYOffset = pclAfterThisSheetDescription->GetSSheetOffset().y + SSheetRect.Height();
		}

		// Create a new instance of CSSheet.
		CSSheet *pclSSheet = new CSSheet();

		if( NULL != pclSSheet )
		{
			// Create CSSheet (at now client area size is unknown).
			UINT uiWindowID = IDC_FPSPREAD + m_ViewDescription.GetSheetNumber();
			BOOL bReturn = pclSSheet->Create( ( GetpWnd()->GetStyle() | WS_CHILD | WS_VISIBLE & ~WS_BORDER ), CRect( 0, iYOffset, 0, iYOffset ), GetpWnd(), uiWindowID );

			if( TRUE == bReturn )
			{
				// Add this new CSSheet in 'm_ViewDescription'.
				pclSheetDescription = m_ViewDescription.AddSheetDescription( uiSheetDescriptionID, uiAfterSheetDescriptionID, pclSSheet, CPoint( 0, iYOffset ) );

				// Register notification to receive messages from CSSheet.
				pclSSheet->MM_RegisterNotificationHandler( GetpWnd(),	CMessageManager::SSheetNHFlags::SSheetNHF_MouseMove |
																		CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDown |
																		CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDblClk |
																		CMessageManager::SSheetNHFlags::SSheetNHF_MouseRButtonDown |
																		CMessageManager::SSheetNHFlags::SSheetNHF_MouseWheel |
																		CMessageManager::SSheetNHFlags::SSheetNHF_MouseCaptureChanged |
																		CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyDown |
																		CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyUp |
																		CMessageManager::SSheetNHFlags::SSheetNHF_ColWidthChanged |
																		CMessageManager::SSheetNHFlags::SSheetNHF_SizeChanged );

				pclSSheet->RegisterNotificationHandler( this );
			}
		}

		if( true == bOffsetFollowing && NULL != pclSheetDescription && NULL != pclNextSheetDescription )
		{
			while( NULL != pclNextSheetDescription )
			{
				if( true == pclNextSheetDescription->GetShowFlag() )
				{
					CPoint point = pclNextSheetDescription->GetSSheetOffset();
					point.y += 1;
					pclNextSheetDescription->SetSSheetOffset( point );
				}

				pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclNextSheetDescription );
			}
		}
	}
	
	return pclSheetDescription;
}

bool CMultiSpreadBase::HideSSheet( UINT uiSheetDescriptionID )
{
	if( false == m_ViewDescription.IsSheetDescriptionExist( uiSheetDescriptionID ) )
	{
		return false;
	}

	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( uiSheetDescriptionID );

	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return false;
	}

	if( true == pclSheetDescription->GetShowFlag() )
	{
		pclSheetDescription->SetShowFlag( false );
		pclSheetDescription->GetSSheetPointer()->ShowWindow( SW_HIDE );
	}

	SetSheetSize();
	return true;
}

bool CMultiSpreadBase::ShowSSheet( UINT uiSheetDescriptionID )
{
	if( false == m_ViewDescription.IsSheetDescriptionExist( uiSheetDescriptionID ) )
	{
		return false;
	}

	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( uiSheetDescriptionID );

	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return false;
	}

	if( false == pclSheetDescription->GetShowFlag() )
	{
		pclSheetDescription->SetShowFlag( true );
		pclSheetDescription->GetSSheetPointer()->ShowWindow( SW_SHOW );
	}

	SetSheetSize();
	return true;
}

void CMultiSpreadBase::EnableSSheetDrawing( BOOL bEnable, CSSheet *pclSSheet )
{
	// Retrieve sheet description list (ordered in terms of Y coordinate from top to bottom).
	CViewDescription::vecSheetDescription vecSheetDescriptionList;
	m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );

	for( int iLoopSheet = 0; iLoopSheet < (int)vecSheetDescriptionList.size(); iLoopSheet++ )
	{
		CSheetDescription *pclSheetDescription = vecSheetDescriptionList[iLoopSheet];

		if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		{
			continue;
		}
		
		CSSheet *pclLoopSSheet = pclSheetDescription->GetSSheetPointer();

		if( NULL == pclLoopSSheet || ( NULL != pclSSheet && pclLoopSSheet != pclSSheet ) )
		{
			continue;
		}

		pclLoopSSheet->SetBool( SSB_REDRAW, bEnable );
	}
}

void CMultiSpreadBase::SetSheetSize( bool bRedraw )
{
	if( NULL == GetpWnd() || 0 == m_ViewDescription.GetSheetNumber() )
	{
		return;
	}

	// Get the right view client area.
	CRect rectClient;
	GetpWnd()->GetClientRect( (LPRECT)rectClient );

	if( rectClient == CRect( 0, 0, 0, 0 ) )
	{
		return;
	}

	// Process child spread sheets.
	CRect rectChild( rectClient.left, rectClient.top, rectClient.left, rectClient.top );

	// Retrieve sheet description list (ordered in terms of Y coordinate from top to bottom).
	CViewDescription::vecSheetDescription vecSheetDescriptionList;
	m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Check first the total virtual size.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::vector<CRect> vecSheetSize;
	_UpdateVirtualSizeAndYOffset( &vecSheetSize );

	if( false == m_bUseOnlyOneSpread && vecSheetSize.size() > 0 )
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// With the total virtual size, we can now update scroll bar info.
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		_CheckScrollbars();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Now it is possible to move each sheet in regards to correct 'iHScrollOffset' and 'iYScrollOffset'.
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		int iHScrollOffset = GetpWnd()->GetScrollPos( SB_HORZ );
		int iYScrollOffset = GetpWnd()->GetScrollPos( SB_VERT );
		int iIndex = 0;

		for( int iLoopSheet = 0; iLoopSheet < (int)vecSheetDescriptionList.size(); iLoopSheet++ )
		{
			CSheetDescription *pclSheetDescription = vecSheetDescriptionList[iLoopSheet];

			if( NULL == pclSheetDescription )
			{
				continue;
			}

			CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

			if( NULL == pclSSheet )
			{
				continue;
			}

			if( false == pclSheetDescription->GetShowFlag() )
			{
				continue;
			}

			// Retrieve Y offset of current sheet.
			CPoint ptOffset = pclSheetDescription->GetSSheetOffset();
			CRect SSheetRect = vecSheetSize[iIndex++];

			// Move sheet.
			pclSSheet->SetBool( SSB_REDRAW, FALSE );
			pclSSheet->MoveWindow( rectClient.left - iHScrollOffset + ptOffset.x, rectClient.top - iYScrollOffset + ptOffset.y, SSheetRect.Width(), SSheetRect.Height(), TRUE );
			pclSSheet->SetBool( SSB_REDRAW, ( true == bRedraw ) ? TRUE : FALSE );
		}
	}
	else if( 1 == (int)vecSheetSize.size() )
	{
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetTopSheetDescription();

		if( NULL == pclSheetDescription )
		{
			return;
		}

		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

		if( NULL == pclSSheet )
		{
			return;
		}

		pclSSheet->MoveWindow( 0, 0, rectClient.Width(), rectClient.Height(), TRUE );
	}
}

void CMultiSpreadBase::SetScrollPosition( ScrollPosition eScrollPosition, CSheetDescription *pclSheetDescription, CRect rectSheetFocusInPixel, bool bTopPreference, long lRowReference )
{
	if( NULL == GetpWnd() )
	{
		return;
	}

	// Sanity check.
	if( ScrollPosition::NoScroll == eScrollPosition )
	{
		return;
	}

	if( ScrollPosition::ScrollToCompute == eScrollPosition && TRUE == rectSheetFocusInPixel.IsRectNull() )
	{
		return;
	}
	
	if( false == m_bUseOnlyOneSpread )
	{
		// Retrieve current vertical scrolling informations.
		SCROLLINFO rScrollInfo;
		ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
		rScrollInfo.cbSize = sizeof( SCROLLINFO );
		rScrollInfo.fMask = SIF_ALL;
		
		if( FALSE == GetpWnd()->GetScrollInfo( SB_VERT, &rScrollInfo ) )
		{
			return;
		}

		int iCurrentPos = GetpWnd()->GetScrollPos( SBS_VERT );
		int iNewPos = 0;
		bool bScroll = true;

		switch( eScrollPosition )
		{
			case ScrollPosition::ScrollToCompute:
			{
				CRect rectPosRView;

				if( NULL != pclSheetDescription )
				{
					CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

					// Convert coordinates in absolute coordinate.
					CRect rectPosScreen = rectSheetFocusInPixel;
					pclSSheet->ClientToScreen( &rectPosScreen );

					// Convert coordinates in regards to right view client area.
					rectPosRView = rectPosScreen;
					GetpWnd()->ScreenToClient( &rectPosRView );
				}
				else
				{
					rectPosRView = rectSheetFocusInPixel;
				}

				// Retrieve size of right view client area.
				CRect rectRView;
				GetpWnd()->GetClientRect( &rectRView );
				rectRView.top += m_iFreezeTopHeightPixels;

				// Check if we must do a vertical scrolling.
				if( rectPosRView.top < rectRView.top || rectPosRView.bottom > rectRView.bottom )
				{
					iNewPos = iCurrentPos;

					if( true == bTopPreference )
					{
						iNewPos = min( iCurrentPos + rectPosRView.top - m_iFreezeTopHeightPixels, rScrollInfo.nMax - (int)rScrollInfo.nPage );
					}
					else
					{
						iNewPos = min( iCurrentPos + ( rectPosRView.bottom - rectRView.Height() - m_iFreezeTopHeightPixels ), rScrollInfo.nMax - (int)rScrollInfo.nPage );
					}
				}
				else
				{
					// Do not scroll.
					bScroll = false;
				}
			}
				break;

			case ScrollPosition::ScrollUpOnePage:

				if( 0 != rScrollInfo.nPage )
				{
					iNewPos = max( iCurrentPos - (int)rScrollInfo.nPage, 0 );
				}
				else
				{
					bScroll = false;
				}

				break;

			case ScrollPosition::ScrollDnOnePage:

				if( 0 != rScrollInfo.nPage )
				{
					iNewPos = min( iCurrentPos + (int)rScrollInfo.nPage, rScrollInfo.nMax - (int)rScrollInfo.nPage );
				}
				else
				{
					bScroll = false;
				}

				break;

			case ScrollPosition::ScrollToTop:

				if( 0 != rScrollInfo.nPage )
				{
					iNewPos = 0;
				}
				else
				{
					bScroll = false;
				}

				break;

			case ScrollPosition::ScrollToBottom:

				if( 0 != rScrollInfo.nPage )
				{
					iNewPos = rScrollInfo.nMax - (int)rScrollInfo.nPage;
				}
				else
				{
					bScroll = false;
				}

				break;
		}

		if( true == bScroll )
		{
			GetpWnd()->SetScrollPos( SB_VERT, iNewPos );

			CRect rectToScroll;
			GetpWnd()->GetClientRect( &rectToScroll );
			rectToScroll.top += m_iFreezeTopHeightPixels;
			GetpWnd()->ScrollWindow( 0, iCurrentPos - iNewPos, NULL, &rectToScroll );
		}
	}
	else if( NULL != m_ViewDescription.GetTopSheetDescription() && NULL != m_ViewDescription.GetTopSheetDescription()->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = m_ViewDescription.GetTopSheetDescription()->GetSSheetPointer();

		switch( eScrollPosition )
		{
			case ScrollPosition::ScrollToCompute:

				if( true == bTopPreference )
				{
					if( FALSE == pclSSheet->GetVisible( pclSSheet->GetActiveCol(), lRowReference, SS_VISIBLE_ALL ) )
					{
						pclSSheet->ShowCell( pclSSheet->GetActiveCol(), lRowReference, SS_SHOW_TOPLEFT );
					}
				}
				else
				{
					if( FALSE == pclSSheet->GetVisible( pclSSheet->GetActiveCol(), lRowReference, SS_VISIBLE_ALL ) )
					{
						pclSSheet->ShowCell( pclSSheet->GetActiveCol(), lRowReference, SS_SHOW_BOTTOMLEFT );
					}
				}

				break;

			case ScrollPosition::ScrollUpOnePage:
				// TODO.
				break;

			case ScrollPosition::ScrollDnOnePage:
				// TODO.
				break;

			case ScrollPosition::ScrollToTop:
				pclSSheet->ShowCell( pclSSheet->GetActiveCol(), 1, SS_SHOW_TOPLEFT );
				break;

			case ScrollPosition::ScrollToBottom:
				pclSSheet->ShowCell( pclSSheet->GetActiveCol(), pclSSheet->GetMaxRows(), SS_SHOW_BOTTOMLEFT );
				break;
		}
	}
}

bool CMultiSpreadBase::_CheckScrollbars( void )
{
	if( NULL == GetpWnd() )
	{
		return false;
	}
	
	// Get the client area.
	CRect rectClient;
	GetpWnd()->GetClientRect( (LPRECT)rectClient );

	if( rectClient.IsRectEmpty() || m_rectVirtualSize.IsRectEmpty() )
	{
		return false;
	}

	EnterCriticalSection( &m_CriticalSection );
	m_bScrollbarRunning = true;
	LeaveCriticalSection( &m_CriticalSection );

	bool bHorzScrollbarAlreadyExist = false;
	int iCurrentHorzPos = 0;

	if( true == IsScrollingExist( ScrollType::ST_Horz ) )
	{
		SCROLLINFO rHScrollInfo;
		ZeroMemory( &rHScrollInfo, sizeof( rHScrollInfo ) );
		rHScrollInfo.cbSize = sizeof( SCROLLINFO );
		rHScrollInfo.fMask = SIF_ALL;
		GetpWnd()->GetScrollInfo( SB_HORZ, &rHScrollInfo );
		iCurrentHorzPos = rHScrollInfo.nPos;
		bHorzScrollbarAlreadyExist = true;
	}

	bool bVertScrollbarAlreadyExist = false;
	int iCurrentVertPos = 0;

	if( true == IsScrollingExist( ScrollType::ST_Vert ) )
	{
		SCROLLINFO rVScrollInfo;
		ZeroMemory( &rVScrollInfo, sizeof( rVScrollInfo ) );
		rVScrollInfo.cbSize = sizeof( SCROLLINFO );
		rVScrollInfo.fMask = SIF_ALL;
		GetpWnd()->GetScrollInfo( SB_VERT, &rVScrollInfo );
		iCurrentVertPos = rVScrollInfo.nPos;
		bVertScrollbarAlreadyExist = true;
	}
	
	// Check the horizontal scroll bar.
	SCROLLINFO rScrollInfo;
	rScrollInfo.cbSize = sizeof(SCROLLINFO);
	rScrollInfo.fMask = SIF_ALL;
	rScrollInfo.nMin = 0;
	rScrollInfo.nMax = 0;
	rScrollInfo.nPage = 0;
	int iNewHorzPos = 0;
	int iNewVertPos = 0;

	if( m_rectVirtualSize.Width() > rectClient.Width() )
	{
		rScrollInfo.nMax = m_rectVirtualSize.Width();
		rScrollInfo.nPage = rectClient.Width();

		// If a vertical scroll bar is needed and doesn't yet exit, the page is reduce with the width of the scrollbar.
		// Why the second condition? Because if vertical is already existing, 'rectClient' already excludes the width
		// of the vertical scrollbar.
		if( m_rectVirtualSize.Height() > rectClient.Height() && false == bVertScrollbarAlreadyExist )
		{
			rScrollInfo.nPage -= GetSystemMetrics( SM_CXVSCROLL );
		}
	}

	iNewHorzPos = min( iCurrentHorzPos, ( rScrollInfo.nMax - (int)rScrollInfo.nPage ) );
	rScrollInfo.nPos = iNewHorzPos;
	rScrollInfo.nTrackPos = rScrollInfo.nPos;
	GetpWnd()->SetScrollInfo( SB_HORZ, &rScrollInfo, TRUE );
	
	// Check the vertical scroll bar.
	rScrollInfo.nMax = 0;
	rScrollInfo.nPage = 0;

	if( m_rectVirtualSize.Height() > rectClient.Height() )
	{
		rScrollInfo.nMax = m_rectVirtualSize.Height();
		rScrollInfo.nPage = rectClient.Height();

		// If a horizontal scrollbar is needed and doesn't yet exit, the page is reduce with the height of the scrollbar.
		// Why the second condition? Because if horizontal is already existing, 'rectClient' already excludes the height
		// of the horizontal scrollbar.
		if( m_rectVirtualSize.Width() > rectClient.Width() && false == bHorzScrollbarAlreadyExist )
		{
			rScrollInfo.nPage -= GetSystemMetrics( SM_CXHSCROLL );
		}
	}

	iNewVertPos = min( iCurrentVertPos, ( rScrollInfo.nMax - (int)rScrollInfo.nPage ) );
	rScrollInfo.nPos = iNewVertPos;
	rScrollInfo.nTrackPos = rScrollInfo.nPos;
	GetpWnd()->SetScrollInfo( SB_VERT, &rScrollInfo, TRUE );

	// Scroll if needed.
	GetpWnd()->ScrollWindow( iCurrentHorzPos - iNewHorzPos, iCurrentVertPos - iNewVertPos );

	EnterCriticalSection( &m_CriticalSection );
	m_bScrollbarRunning = false;
	LeaveCriticalSection( &m_CriticalSection );

	return true;
}

bool CMultiSpreadBase::_IsSheetExist( HWND hSSheet, CSheetDescription *&pclSSheetDescription )
{
	// Retrieve sheet description list.
	CViewDescription::vecSheetDescription vecSheetDescriptionList;
	m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );

	// Loop all sheets.
	bool bExist = false;

	for( int iLoopSheet = 0; iLoopSheet < (int)vecSheetDescriptionList.size() && false == bExist; iLoopSheet++ )
	{
		CSheetDescription *pclSheetDescription = vecSheetDescriptionList[iLoopSheet];
		CSSheet *pclSSheet = vecSheetDescriptionList[iLoopSheet]->GetSSheetPointer();

		if( NULL == pclSSheet )
		{
			continue;
		}

		if( pclSSheet->GetSafeHwnd() == hSSheet )
		{
			bExist = true;
			pclSSheetDescription = vecSheetDescriptionList[iLoopSheet];
		}
	}

	return bExist;
}

void CMultiSpreadBase::_UpdateVirtualSizeAndYOffset( std::vector<CRect> *pvecSheetSize )
{
	// Retrieve sheet description list (ordered in terms of Y coordinate from top to bottom).
	CViewDescription::vecSheetDescription vecSheetDescriptionList;
	m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );

	m_rectVirtualSize.SetRectEmpty();
	int iYOffset = 0;

	for( int iLoopSheet = 0; iLoopSheet < (int)vecSheetDescriptionList.size(); iLoopSheet++ )
	{
		CSheetDescription *pclSheetDescription = vecSheetDescriptionList[iLoopSheet];
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

		if( NULL == pclSSheet )
		{
			continue;
		}

		// Retrieve Y offset of current sheet and update it with new position.
		CPoint ptOffset = pclSheetDescription->GetSSheetOffset();
		ptOffset.y = iYOffset;
		pclSheetDescription->SetSSheetOffset( ptOffset );

		if( false == pclSheetDescription->GetShowFlag() )
		{
			continue;
		}

		// Retrieve sheet size in pixels.
		CRect SSheetRect = pclSSheet->GetSheetSizeInPixels();
		
		// Save it to avoid to compute again below.
		if( NULL != pvecSheetSize )
		{
			pvecSheetSize->push_back( SSheetRect );
		}

		m_rectVirtualSize.right	= max( m_rectVirtualSize.right, SSheetRect.Width() );
		m_rectVirtualSize.bottom += SSheetRect.Height();

		// Update y offset for next sheet.
		iYOffset += SSheetRect.Height();
	}
}
