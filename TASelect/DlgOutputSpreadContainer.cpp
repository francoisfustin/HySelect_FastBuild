#include "stdafx.h"
#include <algorithm>
#include "TASelect.h"
#include "DlgOutputHelper.h"
#include "DlgOutputSpreadContainer.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC( CDlgOutputSpreadContainer, CDialogEx )

using namespace DlgOutputHelper;

CDlgOutputSpreadContainer::CDlgOutputSpreadContainer( int iOutputID, CWnd *pParent )
	: CDialogEx( CDlgOutputSpreadContainer::IDD, pParent ), CMessageManager( CMessageManager::ClassID::CID_CDlgOutputSpreadContainer )
{
	m_iOutputID = iOutputID;
	m_pclSheet = NULL;
	m_lSpreadMinWidth = 0;
	m_fAutoResize = false;
	m_fEditionEnabled = false;
	m_fVScrollPrevState = false;
	m_fIsRedrawBlocked = false;
	m_fIsOutputBlocked = false;
	m_fTextTipFetchEnabled = false;
	m_fBlankLineBeforeMainHeaderExist = false;
	m_dBlankLineBeforeMainHeaderHeight = -1;
	m_lColumnHeaderRows = 0;
	m_lColumnHeaderStart = 0;
	m_lColumnHeaderEnd = 0;
	m_fFreezeHeader = false;
	m_fAutomaticLastBlankLine = false;
	m_lIterRowData = 0;
	m_dDataRowHeight = -1;
	m_DefaultSelectionColor = _TAH_SELECTED_ORANGE;
	m_pclResizingColumnInfos = NULL;
	m_fResizingColumnActive = false;
}

CDlgOutputSpreadContainer::~CDlgOutputSpreadContainer()
{
	if( NULL != m_pclSheet )
	{
		delete m_pclSheet;
	}

	m_pclSheet = NULL;

	m_mapHeaderDefList.clear();

	if( m_mapHeaderCellBaseList.size() > 0 )
	{
		for( mapIntCellBaseIter iter = m_mapHeaderCellBaseList.begin(); iter != m_mapHeaderCellBaseList.end(); iter++ )
		{
			if( NULL != iter->second )
			{
				delete iter->second;
			}
		}

		m_mapHeaderCellBaseList.clear();
	}

	for( std::map<int, MainHeaderParam>::iterator iter = m_mapMainHeaders.begin(); iter != m_mapMainHeaders.end(); ++iter )
	{
		iter->second.m_mapMainHeaderDefList.clear();
		iter->second.m_mapMainHeaderText.clear();
	}

	m_mapColDefList.clear();

	if( m_mapCellBaseList.size() > 0 )
	{
		for( mapIntCellBaseIter iter = m_mapCellBaseList.begin(); iter != m_mapCellBaseList.end(); iter++ )
		{
			if( NULL != iter->second )
			{
				delete iter->second;
			}
		}

		m_mapCellBaseList.clear();
	}

	RemoveAllTextTipFetchTrigger();

	// Remark: we set NULL pointer in all 'CSheetDescription' to avoid that destructor attempt to kill 'CSSheet' again.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SD_Output );

	if( NULL != pclSheetDescription )
	{
		pclSheetDescription->SetSSheetPointer( NULL );
	}

	m_fResizingColumnActive = false;
}

void CDlgOutputSpreadContainer::Reset()
{
	m_mapHeaderDefList.clear();

	if( m_mapHeaderCellBaseList.size() > 0 )
	{
		for( mapIntCellBaseIter iter = m_mapHeaderCellBaseList.begin(); iter != m_mapHeaderCellBaseList.end(); iter++ )
		{
			if( NULL != iter->second )
			{
				delete iter->second;
			}
		}

		m_mapHeaderCellBaseList.clear();
	}

	for( std::map<int, MainHeaderParam>::iterator iter = m_mapMainHeaders.begin(); iter != m_mapMainHeaders.end(); ++iter )
	{
		iter->second.m_mapMainHeaderDefList.clear();
		iter->second.m_mapMainHeaderText.clear();
	}

	m_mapMainHeaders.clear();

	m_mapColDefList.clear();

	if( m_mapCellBaseList.size() > 0 )
	{
		for( mapIntCellBaseIter iter = m_mapCellBaseList.begin(); iter != m_mapCellBaseList.end(); iter++ )
		{
			if( NULL != iter->second )
			{
				delete iter->second;
			}
		}

		m_mapCellBaseList.clear();
	}

	m_mapColPosToID.clear();

	RemoveAllTextTipFetchTrigger();

	m_fAutoResize = false;
	m_fVScrollPrevState = false;
	m_fIsRedrawBlocked = false;
	m_fIsOutputBlocked = false;
	m_fTextTipFetchEnabled = false;
	m_fBlankLineBeforeMainHeaderExist = false;
	m_dBlankLineBeforeMainHeaderHeight = -1;
	m_lColumnHeaderRows = 0;
	m_lColumnHeaderStart = 0;
	m_lColumnHeaderEnd = 0;
	m_fFreezeHeader = false;
	m_fAutomaticLastBlankLine = false;
	m_lIterRowData = 0;
	m_dDataRowHeight = -1;
	m_DefaultSelectionColor = _TAH_SELECTED_ORANGE;
	m_pclSheet->MM_UnregisterAllNotificationHandlers( this );

	// Remark: we set NULL pointer in all 'CSheetDescription' to avoid that destructor attempt to kill 'CSSheet' again.
	m_ViewDescription.Reset();
	m_pclResizingColumnInfos = NULL;
	m_fResizingColumnActive = false;

	_InitializeSSheet();
}

void CDlgOutputSpreadContainer::RegisterNotificationHandler( INotificationHandler *pclHandler, short nNotificationHandlerFlags )
{
	m_mapNotificationHandlerList[nNotificationHandlerFlags] = pclHandler;

	for( short nLoop = INotificationHandler::NH_First; nLoop <= INotificationHandler::NH_Last; nLoop <<= 1 )
	{
		if( nLoop == ( nNotificationHandlerFlags & nLoop ) )
		{
			m_mapNotificationHandlerList[nLoop] = pclHandler;
		}
	}
}

void CDlgOutputSpreadContainer::UnregisterNotificationHandler( INotificationHandler *pclHandler )
{
	for( std::map<short, INotificationHandler *>::iterator mapIter = m_mapNotificationHandlerList.begin(); mapIter != m_mapNotificationHandlerList.end();
		 ++mapIter )
	{
		if( mapIter->second == pclHandler )
		{
			m_mapNotificationHandlerList.erase( mapIter );
			break;
		}
	}
}

void CDlgOutputSpreadContainer::BlockOutput( void )
{
	m_fIsOutputBlocked = true;
}

void CDlgOutputSpreadContainer::ReleaseOutput( void )
{
	if( false == m_fIsOutputBlocked )
	{
		return;
	}

	BeginWaitCursor();

	bool fIsCurrentlyBlocked = IsRedrawBlocked();

	if( false == fIsCurrentlyBlocked )
	{
		BlockRedraw();
	}

	long lRowLimit = ( true == m_fAutomaticLastBlankLine ) ? m_pclSheet->GetMaxRows() : m_pclSheet->GetMaxRows() + 1;

	for( int iLoopRow = m_lColumnHeaderEnd + 1; iLoopRow < lRowLimit; iLoopRow++ )
	{
		long lValue = m_pclSheet->GetCellParam( 1, iLoopRow );

		// Bit 'BitParam::BitBlock' is set to 1 if this row has been blocked, otherwise 0.
		if( BitParam::BitBlock != ( lValue & BitParam::BitBlock ) )
		{
			continue;
		}

		// Bit 'BitParam::BitPreviousBlock' -> state before the row has been blocked (0: shown; 1:hidden).
		m_pclSheet->ShowRow( iLoopRow, ( BitParam::BitPreviousBlock != ( lValue & BitParam::BitPreviousBlock ) ) ? TRUE : FALSE );
		lValue &= ~( BitParam::BitMaskBlocked );
		m_pclSheet->SetCellParam( 1, iLoopRow, ( LPARAM )lValue );
	}

	if( false == fIsCurrentlyBlocked )
	{
		ReleaseRedraw( true );
	}

	// Why use 'EmptyMessageQueue'? Because TSpread doesn't directly apply the change of showing rows. In fact, it flags some variables and the change
	// will be operate later (because TSpread post a SSM_UNHIDEACTIVEROW message). This is why we force TSpread to read its message queue before to
	// call 'ShowCell'.
	m_pclSheet->EmptyMessageQueue();

	int iFirstColID = m_mapColPosToID.begin()->second;
	lRowLimit = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine )
	{
		lRowLimit--;
	}

	m_pclSheet->ShowCell( m_mapColDefList[iFirstColID].GetInternalColPos(), lRowLimit, SS_SHOW_BOTTOMLEFT );

	m_fIsOutputBlocked = false;

	EndWaitCursor();
}

bool CDlgOutputSpreadContainer::EnableEdition( bool fEnable )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	m_pclSheet->EnableEdition( fEnable );
	return true;
}

bool CDlgOutputSpreadContainer::IsEditionEnabled( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	return m_pclSheet->IsEditionEnabled();
}

bool CDlgOutputSpreadContainer::IsCellEditable( int iColID, long lRowRelative, bool &fIsEditable )
{
	fIsEditable = false;

	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	if( lRowRelative < 1 || lRowRelative + m_lColumnHeaderEnd > m_pclSheet->GetMaxRows() )
	{
		return false;
	}

	if( 0 == m_mapColDefList.count( iColID ) )
	{
		return false;
	}

	fIsEditable = m_mapColDefList[iColID].IsEditAllowed();
	return true;
}

bool CDlgOutputSpreadContainer::EditCurrentCell( void )
{
	if( false == m_pclSheet->IsEditionEnabled() )
	{
		return false;
	}

	m_pclSheet->SetEditMode( TRUE );
	return true;
}

bool CDlgOutputSpreadContainer::ResetScrolling( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	m_pclSheet->ShowCell( 1, 1, SS_SHOW_TOPLEFT );
	return true;
}

void CDlgOutputSpreadContainer::AutomaticColumnResizing( bool fAutomaticColumnResize )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || m_fAutoResize == fAutomaticColumnResize )
	{
		return;
	}

	m_fAutoResize = fAutomaticColumnResize;
	m_pclSheet->ShowCol( m_pclSheet->GetMaxCols(), ( true == m_fAutoResize ) ? FALSE : TRUE );

	if( true == m_fAutoResize )
	{
		m_pclSheet->SetColWidthInPixels( m_pclSheet->GetMaxCols(), 0 );
	}

	_ResizeColumns( false );
}

void CDlgOutputSpreadContainer::AutomaticLastBlankLine( bool fAutomaticLastBlankLine )
{
	m_fAutomaticLastBlankLine = fAutomaticLastBlankLine;

	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return;
	}

	if( true == m_fAutomaticLastBlankLine )
	{
		_VerifyLastBlankLine();
	}
	else
	{
		// Verify if there is a blank line in last position and erase it.
		if( m_pclSheet->GetMaxRows() > 0 )
		{
			long lValue = m_pclSheet->GetCellParam( 1, m_pclSheet->GetMaxRows() );

			if( BitParam::BitBlankLine == ( lValue & BitParam::BitBlankLine ) )
			{
				m_pclSheet->SetMaxRows( m_pclSheet->GetMaxRows() - 1 );
			}
		}
	}
}

void CDlgOutputSpreadContainer::AddTextTipFetchTrigger( DlgOutputHelper::CCellBase *pclCellBase, long lRow, CString strText )
{
	pclCellBase->SetInternalHelper( strText );

	long lRowAbsolute = lRow + m_lColumnHeaderEnd;
	mapIntCellBase *pclmapIntCellBase = NULL;

	if( FALSE == m_mapTextTipFetchList.Lookup( lRowAbsolute, pclmapIntCellBase ) )
	{
		pclmapIntCellBase = new mapIntCellBase();
		m_mapTextTipFetchList[lRowAbsolute] = pclmapIntCellBase;
	}

	DlgOutputHelper::CCellBase *pclCellBaseCopy = new CCellBase( *pclCellBase );

	if( NULL != pclCellBaseCopy )
	{
		m_mapTextTipFetchList[lRowAbsolute]->SetAt( (WORD)pclCellBase->GetColumnID(), pclCellBaseCopy );
	}
}

void CDlgOutputSpreadContainer::RemoveAllTextTipFetchTrigger( void )
{
	if( FALSE == m_mapTextTipFetchList.IsEmpty() )
	{
		POSITION IterRow = m_mapTextTipFetchList.GetStartPosition();

		while( NULL != IterRow )
		{
			long lRow;
			mapIntCellBase *pclMapIntCellBase = NULL;
			m_mapTextTipFetchList.GetNextAssoc( IterRow, lRow, pclMapIntCellBase );

			if( NULL != pclMapIntCellBase )
			{
				POSITION IterColID = pclMapIntCellBase->GetStartPosition();

				while( NULL != IterColID )
				{
					int iColumnID;
					DlgOutputHelper::CCellBase *pclCellBase = NULL;
					pclMapIntCellBase->GetNextAssoc( IterColID, iColumnID, pclCellBase );

					if( NULL != pclCellBase )
					{
						delete pclCellBase;
					}
				}

				delete pclMapIntCellBase;
			}
		}

		m_mapTextTipFetchList.RemoveAll();
	}
}

bool CDlgOutputSpreadContainer::BlockRedraw( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || true == m_fIsRedrawBlocked )
	{
		return false;
	}

	m_fIsRedrawBlocked = true;
	m_pclSheet->SetBool( SSB_REDRAW, FALSE );
	return true;
}

bool CDlgOutputSpreadContainer::ReleaseRedraw( bool fResizeColumn )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_fIsRedrawBlocked )
	{
		return false;
	}

	m_fIsRedrawBlocked = false;
	m_pclSheet->SetBool( SSB_REDRAW, TRUE );

	if( true == fResizeColumn )
	{
		_ResizeColumns( false );
	}

	return true;
}

bool CDlgOutputSpreadContainer::IsRedrawBlocked( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_fIsRedrawBlocked )
	{
		return false;
	}

	return m_fIsRedrawBlocked;
}

bool CDlgOutputSpreadContainer::AddBlankLineBeforeMainHeader( double dHeight )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || true == m_fBlankLineBeforeMainHeaderExist )
	{
		return false;
	}

	m_fBlankLineBeforeMainHeaderExist = true;
	m_dBlankLineBeforeMainHeaderHeight = dHeight;
	m_lColumnHeaderStart++;
	m_lColumnHeaderEnd++;

	_VerifyMainHeaders();

	return true;
}

bool CDlgOutputSpreadContainer::AddMainHeaderDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CColDef &clColDef, CString strText )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iMainHeaderID < 0 || iMainHeaderSubID < 0 )
	{
		return false;
	}

	// If header definition is already exist...
	if( m_mapMainHeaders.size() > 0 && m_mapMainHeaders.count( iMainHeaderID ) > 0
		&& m_mapMainHeaders[iMainHeaderID].m_mapMainHeaderDefList.count( iMainHeaderSubID ) > 0 )
	{
		return false;
	}

	if( 0 == ( int )m_mapMainHeaders.size() || 0 == m_mapMainHeaders.count( iMainHeaderID ) )
	{
		MainHeaderParam rMainHeaderParam;
		m_mapMainHeaders[iMainHeaderID] = rMainHeaderParam;
		m_lColumnHeaderStart++;
		m_lColumnHeaderEnd++;
	}

	m_mapMainHeaders[iMainHeaderID].m_mapMainHeaderDefList[iMainHeaderSubID] = clColDef;
	m_mapMainHeaders[iMainHeaderID].m_mapMainHeaderText[iMainHeaderSubID] = strText;

	_VerifyMainHeaders();

	return true;
}

bool CDlgOutputSpreadContainer::AddMainHeaderFontDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CFontDef &clFontDef )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iMainHeaderID < 0 || iMainHeaderSubID < 0 )
	{
		return false;
	}

	// If main header definition is not exist...
	if( 0 == ( int )m_mapMainHeaders.size() || 0 == m_mapMainHeaders.count( iMainHeaderID )
		|| 0 == m_mapMainHeaders[iMainHeaderID].m_mapMainHeaderDefList.count( iMainHeaderSubID ) )
	{
		return false;
	}

	m_mapMainHeaders[iMainHeaderID].m_mapMainHeaderDefList[iMainHeaderSubID].SetFontDef( clFontDef );

	_VerifyMainHeaders();

	return true;
}

bool CDlgOutputSpreadContainer::SetMainHeaderRowHeight( int iMainHeaderID, double dHeight )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iMainHeaderID < 0 )
	{
		return false;
	}

	// If main header definition is not exist...
	if( 0 == ( int )m_mapMainHeaders.size() || 0 == m_mapMainHeaders.count( iMainHeaderID ) )
	{
		return false;
	}

	m_mapMainHeaders[iMainHeaderID].m_dMainHeaderRowHeight = dHeight;
	return true;
}

void CDlgOutputSpreadContainer::SetColumnHeaderRows( long lRow )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return;
	}

	m_lColumnHeaderRows = lRow;
	m_lColumnHeaderStart = m_mapMainHeaders.size() + 1;
	m_lColumnHeaderEnd = lRow + ( long )m_mapMainHeaders.size();

	if( true == m_fBlankLineBeforeMainHeaderExist )
	{
		m_lColumnHeaderStart++;
		m_lColumnHeaderEnd++;
	}
}

bool CDlgOutputSpreadContainer::FreezeAllHeaderRows( bool fFreeze )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	if( 0 == m_lColumnHeaderRows )
	{
		return false;
	}

	long lFreeze = m_lColumnHeaderRows + ( long )m_mapMainHeaders.size();

	if( true == m_fBlankLineBeforeMainHeaderExist )
	{
		lFreeze++;
	}

	m_pclSheet->SetFreeze( 0, ( true == fFreeze ) ? lFreeze : 0 );
	m_fFreezeHeader = fFreeze;
	return true;
}

bool CDlgOutputSpreadContainer::AddColumnHeaderDef( int iColumnID, DlgOutputHelper::CColDef &clColDef )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return false;
	}

	// If header definition is already exist...
	if( m_mapHeaderDefList.count( iColumnID ) > 0 )
	{
		return false;
	}

	// Because we pass argument as reference, we don't want to change anything in this argument.
	CColDef clLocalColDef = clColDef;

	// Check what is the header column width defined.
	if( -1.0 == clLocalColDef.GetWidth() && -1 == clLocalColDef.GetWidthInPixels() )
	{
		return false;
	}
	else if( -1 == clLocalColDef.GetWidthInPixels() )
	{
		// Transform font unit in pixel.
		clLocalColDef.SetWidthInPixels( m_pclSheet->ColWidthToLogUnits( clLocalColDef.GetWidth() ) );
	}
	else if( -1.0 == clLocalColDef.GetWidth() )
	{
		// Transform pixel in font unit.
		clLocalColDef.SetWidth( m_pclSheet->LogUnitsToColWidthW( ( short )clLocalColDef.GetWidthInPixels() ) );
	}

	m_mapHeaderDefList[iColumnID] = clLocalColDef;

	long lColAbsolutePos;

	if( 0 == m_mapColDefList.count( iColumnID ) )
	{
		// If we want to add a header column and there is not yet data column with the same ID, we check where to insert.
		lColAbsolutePos = _CheckWhereToInsertColumn( iColumnID );

		if( 0 == lColAbsolutePos )
		{
			// Don't know where to insert, we insert at the last position.
			lColAbsolutePos = m_pclSheet->GetMaxCols();
		}

		// If column is not yet defined we insert the column.
		_InsertCol( iColumnID, lColAbsolutePos );
	}
	else
	{
		// If we want to add a header column ID and the data column ID is already defined, we take this position.
		lColAbsolutePos = m_mapColDefList[iColumnID].GetInternalColPos();
	}

	_ApplyCellFormat( lColAbsolutePos, m_lColumnHeaderStart, lColAbsolutePos, m_lColumnHeaderEnd, clLocalColDef );
	_ApplyCellProperties( lColAbsolutePos, m_lColumnHeaderStart, lColAbsolutePos, m_lColumnHeaderEnd, clLocalColDef );

	m_mapColPosToID[lColAbsolutePos] = iColumnID;
	m_mapHeaderDefList[iColumnID].SetInternalColPos( lColAbsolutePos );
	m_pclSheet->SetColWidth( lColAbsolutePos, m_mapHeaderDefList[iColumnID].GetWidth() );

	if( false == m_mapHeaderDefList[iColumnID].GetVisible() )
	{
		m_pclSheet->ShowCol( lColAbsolutePos, FALSE );
	}

	// Compute the current min width.
	CRect rectPixel = m_pclSheet->GetSheetSizeInPixels();
	m_lSpreadMinWidth = rectPixel.Width();

	// Prepare the map that will contain all cells for one row.
	CCellBase *pclCellBase = NULL;

	switch( m_mapHeaderDefList[iColumnID].GetContentType() )
	{
		case CColDef::Text:
			pclCellBase = new CCellText( iColumnID );
			break;

		case CColDef::Number:
			pclCellBase = new CCellNumber( iColumnID );
			break;

		case CColDef::Bitmap:
			pclCellBase = new CCellBitmap( iColumnID );
			break;

		case CColDef::Param:
			pclCellBase = new CCellParam( iColumnID );
			break;
	}

	if( NULL == pclCellBase )
	{
		return false;
	}

	m_mapHeaderCellBaseList[iColumnID] = pclCellBase;

	_VerifyMainHeaders();

	return true;
}

bool CDlgOutputSpreadContainer::AddColumnHeaderFontDef( int iColumnID, DlgOutputHelper::CFontDef &clFontDef )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return false;
	}

	// If header column definition not exist...
	if( 0 == m_mapHeaderDefList.count( iColumnID ) )
	{
		return false;
	}

	m_mapHeaderDefList[iColumnID].SetFontDef( clFontDef );
	return true;
}

bool CDlgOutputSpreadContainer::FillColumnHeader( int iColumnID, long lRowRelative, CString strData, double dRowHeight, bool fLineBelow )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return false;
	}

	// If header column definition not exist...
	if( 0 == m_mapHeaderDefList.count( iColumnID ) )
	{
		return false;
	}

	long lColAbsolute = m_mapHeaderDefList[iColumnID].GetInternalColPos();

	// Remark: first and last columns are reserved for internal use.
	if( lColAbsolute < 2 || lColAbsolute >= m_pclSheet->GetMaxCols() )
	{
		return false;
	}

	long lRowAbsolute = lRowRelative + m_lColumnHeaderStart - 1;

	if( lRowAbsolute < m_lColumnHeaderStart || lRowAbsolute > m_lColumnHeaderEnd )
	{
		return false;
	}

	_AddRows( BitParam::BitColumnHeader, lRowAbsolute, 1, dRowHeight );
	_SendOutputText( strData, lColAbsolute, lRowAbsolute, lRowAbsolute, &m_mapHeaderDefList[iColumnID].GetFontDef() );

	if( true == fLineBelow )
	{
		m_pclSheet->SetBorder( lColAbsolute, lRowAbsolute, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	}

	_VerifyMainHeaders();

	return true;
}

bool CDlgOutputSpreadContainer::AddColumnDefinition( int iColumnID, CColDef &clColDef )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return false;
	}

	// If column definition already exist...
	if( m_mapColDefList.count( iColumnID ) > 0 )
	{
		return false;
	}

	// Because we pass argument as reference, we don't want to change anything in this argument.
	CColDef clLocalColDef = clColDef;

	// If no header is defined, we can take the column width here.
	if( 0 == m_mapHeaderDefList.count( iColumnID ) )
	{
		if( -1.0 == clLocalColDef.GetWidth() && -1 == clLocalColDef.GetWidthInPixels() )
		{
			return false;
		}
		else if( -1 == clLocalColDef.GetWidthInPixels() )
		{
			// Transform font unit in pixel.
			clLocalColDef.SetWidthInPixels( m_pclSheet->ColWidthToLogUnits( clLocalColDef.GetWidth() ) );
		}
		else if( -1.0 == clLocalColDef.GetWidth() )
		{
			// Transform pixel in font unit.
			clLocalColDef.SetWidth( m_pclSheet->LogUnitsToColWidthW( ( short )clLocalColDef.GetWidthInPixels() ) );
		}
	}
	else
	{
		clLocalColDef.SetWidth( m_mapHeaderDefList[iColumnID].GetWidth() );
		clLocalColDef.SetWidthInPixels( m_mapHeaderDefList[iColumnID].GetWidthInPixels() );
		clLocalColDef.SetAutoResized( m_mapHeaderDefList[iColumnID].GetAutoResized() );
		clLocalColDef.SetVisible( m_mapHeaderDefList[iColumnID].GetVisible() );
	}

	m_mapColDefList[iColumnID] = clLocalColDef;

	long lMaxRows = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine )
	{
		lMaxRows--;
	}

	long lColAbsolutePos;

	if( 0 == m_mapHeaderDefList.count( iColumnID ) )
	{
		// If we want to add a data column and there is not yet header column with the same ID, we check where to insert.
		lColAbsolutePos = _CheckWhereToInsertColumn( iColumnID );

		if( 0 == lColAbsolutePos )
		{
			// Don't know where to insert, we insert at the last position.
			lColAbsolutePos = m_pclSheet->GetMaxCols();
		}

		// If column is not yet defined we insert the column.
		_InsertCol( iColumnID, lColAbsolutePos );
	}
	else
	{
		// If we want to add a data column ID and the header column ID is already defined, we take this position.
		lColAbsolutePos = m_mapHeaderDefList[iColumnID].GetInternalColPos();
	}

	if( lMaxRows >= m_lColumnHeaderEnd + 1 )
	{
		_ApplyCellFormat( lColAbsolutePos, m_lColumnHeaderEnd + 1, lColAbsolutePos, lMaxRows, clLocalColDef );
		_ApplyCellProperties( lColAbsolutePos, m_lColumnHeaderEnd + 1, lColAbsolutePos, lMaxRows, clLocalColDef );
	}

	m_mapColPosToID[lColAbsolutePos] = iColumnID;
	m_mapColDefList[iColumnID].SetInternalColPos( lColAbsolutePos );
	m_pclSheet->SetColWidth( lColAbsolutePos, m_mapColDefList[iColumnID].GetWidth() );

	if( false == m_mapColDefList[iColumnID].GetVisible() )
	{
		m_pclSheet->ShowCol( lColAbsolutePos, FALSE );
	}

	// Compute the current min width.
	CRect rectPixel = m_pclSheet->GetSheetSizeInPixels();
	m_lSpreadMinWidth = rectPixel.Width();

	// Prepare the map that will contain all cells for one row.
	CCellBase *pclCellBase = NULL;

	switch( m_mapColDefList[iColumnID].GetContentType() )
	{
		case CColDef::Text:
			pclCellBase = new CCellText( iColumnID );
			break;

		case CColDef::Number:
			pclCellBase = new CCellNumber( iColumnID, m_mapColDefList[iColumnID].GetPhysicalType(), m_mapColDefList[iColumnID].GetMaxDigit(),
										   m_mapColDefList[iColumnID].GetMinDecimal() );
			break;

		case CColDef::Bitmap:
			pclCellBase = new CCellBitmap( iColumnID );
			break;

		case CColDef::Param:
			pclCellBase = new CCellParam( iColumnID );
			break;
	}

	if( NULL == pclCellBase )
	{
		return false;
	}

	m_mapCellBaseList[iColumnID] = pclCellBase;

	_VerifyMainHeaders();

	if( lMaxRows >= m_lColumnHeaderEnd + 1 )
	{

		if( true == m_mapColDefList[iColumnID].GetRowSeparatorFlag() )
		{
			m_pclSheet->SetBorderRange( lColAbsolutePos, m_lColumnHeaderEnd + 1, lColAbsolutePos, lMaxRows, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
										m_mapColDefList[iColumnID].GetRowSeparatorColor() );
		}

		if( true == m_mapColDefList[iColumnID].GetColSeparatorFlag() )
		{
			m_pclSheet->SetBorderRange( lColAbsolutePos, m_lColumnHeaderEnd + 1, lColAbsolutePos, lMaxRows, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID,
										m_mapColDefList[iColumnID].GetColSeparatorColor() );
		}
	}

	return true;
}

bool CDlgOutputSpreadContainer::AddColumnFontDefinition( int iColumnID, DlgOutputHelper::CFontDef &clFontDef )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return false;
	}

	// If column definition not exist...
	if( 0 == m_mapColDefList.count( iColumnID ) )
	{
		return false;
	}

	m_mapColDefList[iColumnID].SetFontDef( clFontDef );
	return true;
}

bool CDlgOutputSpreadContainer::RemoveColumn( int iColumnID )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return false;
	}

	// If column definition not exist...
	if( 0 == m_mapColDefList.count( iColumnID ) )
	{
		return false;
	}

	long lColAbsolutePos = m_mapColDefList[iColumnID].GetInternalColPos();

	// Remark: first and last columns are reserved for internal use.
	if( lColAbsolutePos < 2 || lColAbsolutePos >= m_pclSheet->GetMaxCols() )
	{
		return false;
	}

	long lCurrentSelectedRowRelative = -1;
	GetSelectedDataRow( lCurrentSelectedRowRelative );

	long lBSStartCol = 0;
	long lBSStartRow = 0;
	long lBSEndCol = 0;
	long lBSEndRow = 0;
	bool fBlockSelectionExist = m_pclSheet->BS_IsSelectionExist();

	if( true == fBlockSelectionExist )
	{
		m_pclSheet->BS_GetSelection( lBSStartCol, lBSStartRow, lBSEndCol, lBSEndRow );
	}

	m_pclSheet->ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All );

	m_pclSheet->DelCol( lColAbsolutePos );
	// Because a new blank column appears as the far right column of the sheet.
	m_pclSheet->SetMaxCols( m_pclSheet->GetMaxCols() - 1 );

	m_mapColDefList.erase( iColumnID );

	if( 0 != m_mapCellBaseList.count( iColumnID ) )
	{
		if( NULL != m_mapCellBaseList[iColumnID] )
		{
			delete m_mapCellBaseList[iColumnID];
		}

		m_mapCellBaseList.erase( iColumnID );
	}

	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		m_mapHeaderDefList.erase( iColumnID );
	}

	if( 0 != m_mapHeaderCellBaseList.count( iColumnID ) )
	{
		if( NULL != m_mapHeaderCellBaseList[iColumnID] )
		{
			delete m_mapHeaderCellBaseList[iColumnID];
		}

		m_mapHeaderCellBaseList.erase( iColumnID );
	}

	for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
	{
		if( iter->second.GetInternalColPos() > lColAbsolutePos )
		{
			iter->second.SetInternalColPos( iter->second.GetInternalColPos() - 1 );
		}
	}

	m_mapColPosToID.clear();

	for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
	{
		m_mapColPosToID[iter->second.GetInternalColPos()] = iter->first;
	}

	for( mapIntColDefIter iter = m_mapHeaderDefList.begin(); iter != m_mapHeaderDefList.end(); iter++ )
	{
		if( iter->second.GetInternalColPos() > lColAbsolutePos )
		{
			iter->second.SetInternalColPos( iter->second.GetInternalColPos() - 1 );
		}
	}

	_VerifyMainHeaders();

	// Check the row selection.
	if( -1 != lCurrentSelectedRowRelative )
	{
		SelectDataRow( lCurrentSelectedRowRelative, false );
	}

	// Check the block selection.
	if( true == fBlockSelectionExist )
	{
		// Because we cancel the selection just before, we have only one cell.
		if( lBSStartCol > lColAbsolutePos )
		{
			m_pclSheet->BS_SetSelection( lBSStartCol - 1, lBSStartRow, lBSEndCol - 1, lBSEndRow );
		}
	}

	return true;
}

bool CDlgOutputSpreadContainer::SetColumnWidth( int iColumnID, double dWidth )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	// If column definition not exist...
	if( 0 == m_mapHeaderDefList.count( iColumnID ) && 0 == m_mapColDefList.count( iColumnID ) )
	{
		return false;
	}

	// Transform font unit in pixel.
	int iWidthInPixel = m_pclSheet->ColWidthToLogUnits( dWidth );

	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		m_mapHeaderDefList[iColumnID].SetWidth( dWidth );
		m_mapHeaderDefList[iColumnID].SetWidthInPixels( iWidthInPixel );
	}

	if( 0 != m_mapColDefList.count( iColumnID ) )
	{
		m_mapColDefList[iColumnID].SetWidth( dWidth );
		m_mapColDefList[iColumnID].SetWidthInPixels( iWidthInPixel );
	}

	m_pclSheet->SetColWidth( m_mapColDefList[iColumnID].GetInternalColPos(), dWidth );

	return true;
}

bool CDlgOutputSpreadContainer::SetColumnWidthInPixel( int iColumnID, long lWidth )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	// If column definition not exist...
	if( 0 == m_mapHeaderDefList.count( iColumnID ) && 0 == m_mapColDefList.count( iColumnID ) )
	{
		return false;
	}

	// Transform pixel in font unit.
	double dWidth = m_pclSheet->LogUnitsToColWidthW( ( short )lWidth );

	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		m_mapHeaderDefList[iColumnID].SetWidthInPixels( lWidth );
		m_mapHeaderDefList[iColumnID].SetWidth( dWidth );
	}

	if( 0 != m_mapColDefList.count( iColumnID ) )
	{
		m_mapColDefList[iColumnID].SetWidthInPixels( lWidth );
		m_mapColDefList[iColumnID].SetWidth( dWidth );
	}

	m_pclSheet->SetColWidth( m_mapColDefList[iColumnID].GetInternalColPos(), dWidth );

	return true;
}

bool CDlgOutputSpreadContainer::GetColumnWidth( int iColumnID, double &dWidth )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	// If column definition not exist...
	if( 0 == m_mapHeaderDefList.count( iColumnID ) && 0 == m_mapColDefList.count( iColumnID ) )
	{
		return false;
	}

	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		dWidth = m_mapHeaderDefList[iColumnID].GetWidth();
	}
	else
	{
		dWidth = m_mapColDefList[iColumnID].GetWidth();
	}

	return true;
}

bool CDlgOutputSpreadContainer::GetColumnWidthInPixel( int iColumnID, long &lWidth )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	// If column definition not exist...
	if( 0 == m_mapHeaderDefList.count( iColumnID ) && 0 == m_mapColDefList.count( iColumnID ) )
	{
		return false;
	}

	// Update to be sure.
	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		lWidth = m_pclSheet->GetColWidthInPixelsW( m_mapHeaderDefList[iColumnID].GetInternalColPos() );
	}
	else
	{
		lWidth = m_pclSheet->GetColWidthInPixelsW( m_mapColDefList[iColumnID].GetInternalColPos() );
	}

	// Update both is available.
	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		m_mapHeaderDefList[iColumnID].SetWidthInPixels( lWidth );
	}

	if( 0 != m_mapColDefList.count( iColumnID ) )
	{
		m_mapColDefList[iColumnID].SetWidthInPixels( lWidth );
	}

	return true;
}

bool CDlgOutputSpreadContainer::SendMessage( DlgOutputHelper::CMessageBase *pclMessage, DlgOutputHelper::CFilterList *pclFilterList, long lRowRelative,
		bool fScrollOnTheLastLine )
{
	if( NULL == pclMessage || NULL == pclMessage->GetCellList() || NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	if( lRowRelative < -1 || 0 == lRowRelative )
	{
		return false;
	}

	BeginWaitCursor();

	long lRowAbsolute;

	if( -1 == lRowRelative )
	{
		lRowAbsolute = ( true == m_fAutomaticLastBlankLine ) ? m_pclSheet->GetMaxRows() : m_pclSheet->GetMaxRows() + 1;
	}
	else
	{
		lRowAbsolute = lRowRelative + m_lColumnHeaderEnd;
	}

	_AddRows( BitParam::BitData, lRowAbsolute, 1, m_dDataRowHeight );

	// Check if we have pre job to do.
	if( true == pclMessage->HasPreJob() )
	{
		_ExecutePreJob( pclMessage, lRowAbsolute );
	}

	bool fAtLeastOneCellOutput = false;
	DlgOutputHelper::vecCCellBasePtr *pvecCellList = pclMessage->GetCellList();

	for( vecCCellBasePtrIter iter = pvecCellList->begin(); iter != pvecCellList->end(); iter++ )
	{
		CCellBase *pclBase = *iter;
		int iColumnID = pclBase->GetColumnID();

		// If column definition doesn't exist bypass.
		if( 0 == m_mapColDefList.count( iColumnID ) )
		{
			continue;
		}

		// If cell type is not the same as the column type defined bypass.
		if( pclBase->GetContentType() != m_mapColDefList[iColumnID].GetContentType() )
		{
			ASSERT_CONTINUE;
		}

		// Set properties.
		long lColAbsolute = m_mapColDefList[iColumnID].GetInternalColPos();

		// Remark: first and last columns are reserved for internal use.
		if( lColAbsolute < 2 || lColAbsolute >= m_pclSheet->GetMaxCols() )
		{
			continue;
		}

		bool fReturn = false;

		switch( pclBase->GetContentType() )
		{
			case CColDef::Text:
			{
				CCellText *pCellText = dynamic_cast<CCellText *>( pclBase );

				COLORREF textBackColor;
				COLORREF textForeColor;
				DlgOutputHelper::CFontDef clFontDef( m_mapColDefList[iColumnID].GetFontDef() );

				if( true == pCellText->GetTextBackgroundColor( textBackColor ) || true == pCellText->GetTextForegroundColor( textForeColor ) )
				{
					if( true == pCellText->GetTextBackgroundColor( textBackColor ) )
					{
						clFontDef.SetTextBackgroundColor( textBackColor );
					}

					if( true == pCellText->GetTextForegroundColor( textForeColor ) )
					{
						clFontDef.SetTextForegroundColor( textForeColor );
					}
				}

				fReturn = _SendOutputText( pCellText->GetText(), lColAbsolute, lRowAbsolute, lRowAbsolute, &clFontDef );
			}
			break;

			case CColDef::Number:
			{
				CCellNumber *pCellNumber = dynamic_cast<CCellNumber *>( pclBase );
				COLORREF textBackColor;
				COLORREF textForeColor;
				DlgOutputHelper::CFontDef clFontDef( m_mapColDefList[iColumnID].GetFontDef() );

				if( true == pCellNumber->GetTextBackgroundColor( textBackColor ) || true == pCellNumber->GetTextForegroundColor( textForeColor ) )
				{
					if( true == pCellNumber->GetTextBackgroundColor( textBackColor ) )
					{
						clFontDef.SetTextBackgroundColor( textBackColor );
					}

					if( true == pCellNumber->GetTextForegroundColor( textForeColor ) )
					{
						clFontDef.SetTextForegroundColor( textForeColor );
					}
				}

				fReturn = _SendOutputText( pCellNumber->GetNumberAsText(), lColAbsolute, lRowAbsolute, lRowAbsolute, &clFontDef );
			}
			break;

			case CColDef::Bitmap:
				fReturn = _SendOutputBitmap( dynamic_cast<CCellBitmap *>( pclBase ), lColAbsolute, lRowAbsolute );
				break;

			case CColDef::Param:
				fReturn = _SendOutputParam( dynamic_cast<CCellParam *>( pclBase ), lColAbsolute, lRowAbsolute );
				break;
		}

		_ApplyCellProperties( lColAbsolute, lRowAbsolute, lColAbsolute, lRowAbsolute, m_mapColDefList[iColumnID] );

		if( true == fReturn )
		{
			fAtLeastOneCellOutput = true;
		}
	}

	if( false == fAtLeastOneCellOutput )
	{
		return false;
	}

	_ApplyFilterRow( lRowAbsolute, pclFilterList );

	if( true == m_fIsOutputBlocked )
	{
		long lValue = m_pclSheet->GetCellParam( 1, lRowAbsolute );
		lValue &= ~BitParam::BitMaskBlocked;
		lValue |= BitParam::BitBlock;
		lValue |= ( TRUE == m_pclSheet->IsRowHidden( lRowAbsolute ) ) ? BitParam::BitPreviousBlock : 0;

		// Force the row to be hidden.
		m_pclSheet->ShowRow( lRowAbsolute, FALSE );
		m_pclSheet->SetCellParam( 1, lRowAbsolute, ( LPARAM )lValue );
	}

	if( false == m_fIsOutputBlocked && false == m_fIsRedrawBlocked )
	{
		_ResizeColumns( fScrollOnTheLastLine );
	}

	EndWaitCursor();

	return true;
}

int CDlgOutputSpreadContainer::PasteData( CString strData, int iColumnID, long lStartRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return PDE_NoDialog;
	}

	if( 0 == m_mapColDefList.count( iColumnID ) )
	{
		return PDE_BadColStart;
	}

	long lStartColAbsolute = m_mapColDefList[iColumnID].GetInternalColPos();

	BeginWaitCursor();

	// Snapshot of all current selections if exist.
	m_pclSheet->SaveSelections( CSSheet::SaveSelectionChoice::SSC_All );
	m_pclSheet->ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All );

	if( lStartRowRelative < 1 )
	{
		return PDE_BadRowStart;
	}

	// Remark: first and last columns are reserved for internal use.
	if( lStartColAbsolute < 2 || lStartColAbsolute >= m_pclSheet->GetMaxCols() )
	{
		return PDE_BadColStart;
	}

	// Check number of columns and rows to copy.
	int iCurRow = 0;
	long lRowCounter = 0;
	long lMaxCols = 0;
	CString strRow = strData.Tokenize( _T("\n"), iCurRow );

	while( _T( "" ) != strRow )
	{
		lRowCounter++;
		int iCurCol = 0;
		long lColCounter = 0;
		CString strCol = strRow.Tokenize( _T("\t"), iCurCol );

		while( _T( "" ) != strCol )
		{
			lColCounter++;
			strCol = strRow.Tokenize( _T("\t"), iCurCol );
		}

		if( lColCounter > lMaxCols )
		{
			lMaxCols = lColCounter;
		}

		strRow = strData.Tokenize( _T("\n"), iCurRow );
	}

	if( 0 == lMaxCols )
	{
		return PDE_NoDataToPaste;
	}

	long lStartRowAbsolute = lStartRowRelative + m_lColumnHeaderEnd;
	long lEndRowAbsolute = lStartRowAbsolute + lRowCounter - 1;
	long lEndColAbsolute = lStartColAbsolute + lMaxCols - 1;

	if( lEndColAbsolute >= m_pclSheet->GetMaxCols() )
	{
		lEndColAbsolute = m_pclSheet->GetMaxCols() - 1;
	}

	// Verify now if there is no column in which we can't paste data.
	for( long lLoopColumn = lStartColAbsolute; lLoopColumn <= lEndColAbsolute; lLoopColumn++ )
	{
		if( 0 == m_mapColPosToID.count( lLoopColumn ) )
		{
			continue;
		}

		if( false == m_mapColDefList[m_mapColPosToID[lLoopColumn]].IsPasteDataAllowed() )
		{
			lEndColAbsolute = lLoopColumn - 1;
			break;
		}
	}

	if( lEndColAbsolute == lStartColAbsolute - 1 )
	{
		return PDE_CanPasteData;
	}

	// Check if we must add rows.
	_AddRows( BitParam::BitData, lStartRowAbsolute, lEndRowAbsolute - lStartRowAbsolute + 1, m_dDataRowHeight );

	if( FALSE == m_pclSheet->ClipIn( lStartColAbsolute, lStartRowAbsolute, lEndColAbsolute, lEndRowAbsolute, ( LPCTSTR )strData, strData.GetLength() ) )
	{
		return PDE_ClipInError;
	}

	// Restore previous selection if exist.
	m_pclSheet->RestoreSelections();

	ReleaseRedraw();
	return PDE_Ok;
}

GLOBALHANDLE CDlgOutputSpreadContainer::CopyDataInGlobalBuffer( long lStartColAbsolute, long lStartRowAbsolute, long lEndColAbsolute, long lEndRowAbsolute )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return ( GLOBALHANDLE )NULL;
	}

	return m_pclSheet->ClipOut( lStartColAbsolute, lStartRowAbsolute, lEndColAbsolute, lEndRowAbsolute );
}

bool CDlgOutputSpreadContainer::CopyDataInCString( long lStartColAbsolute, long lStartRowAbsolute, long lEndColAbsolute, long lEndRowAbsolute,
		CString &strOutput )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	if( lStartColAbsolute < 1 || lStartRowAbsolute < 1 || lEndColAbsolute < 1 || lEndRowAbsolute < 1 )
	{
		return false;
	}

	long lMaxRows = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine )
	{
		lMaxRows--;
	}

	if( lStartColAbsolute > m_pclSheet->GetMaxCols() || lEndColAbsolute > m_pclSheet->GetMaxCols() || lStartRowAbsolute > lMaxRows || lEndRowAbsolute > lMaxRows )
	{
		return false;
	}

	if( lStartColAbsolute > lEndColAbsolute )
	{
		long lTemp = lStartColAbsolute;
		lStartColAbsolute = lEndColAbsolute;
		lEndColAbsolute = lTemp;
	}

	if( lStartRowAbsolute > lEndRowAbsolute )
	{
		long lTemp = lStartRowAbsolute;
		lStartRowAbsolute = lEndRowAbsolute;
		lEndRowAbsolute = lTemp;
	}

	strOutput = _T( "" );

	for( long lLoopRow = lStartRowAbsolute; lLoopRow <= lEndRowAbsolute; lLoopRow++ )
	{
		for( long lLoopCol = lStartColAbsolute; lLoopCol <= lEndColAbsolute; lLoopCol++ )
		{
			int iColumnID = m_mapColPosToID[lLoopCol];

			if( false == m_mapColDefList[iColumnID].IsCopyDataAllowed() )
			{
				continue;
			}

			CString strText( _T( "" ) );

			switch( m_mapColDefList[iColumnID].GetContentType() )
			{
				case DlgOutputHelper::CColDef::ContentType::Text:
				case DlgOutputHelper::CColDef::ContentType::Number:
					strText = m_pclSheet->GetCellText( lLoopCol, lLoopRow );
					break;

				case DlgOutputHelper::CColDef::ContentType::Bitmap:
				case DlgOutputHelper::CColDef::ContentType::Param:
					break;
			}

			strOutput += strText;

			if( lLoopCol < lEndColAbsolute )
			{
				strOutput += _T("\t");
			}
		}

		if( lLoopRow < lEndRowAbsolute )
		{
			strOutput += _T("\r\n");
		}
	}

	return true;
}

bool CDlgOutputSpreadContainer::CopyDataInXMLSpreadsheet( long lStartColAbsolute, long lStartRowAbsolute, long lEndColAbsolute, long lEndRowAbsolute,
		CString &strOutput )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	if( lStartColAbsolute < 1 || lStartRowAbsolute < 1 || lEndColAbsolute < 1 || lEndRowAbsolute < 1 )
	{
		return false;
	}

	long lMaxRows = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine )
	{
		lMaxRows--;
	}

	if( lStartColAbsolute > m_pclSheet->GetMaxCols() || lEndColAbsolute > m_pclSheet->GetMaxCols() || lStartRowAbsolute > lMaxRows || lEndRowAbsolute > lMaxRows )
	{
		return false;
	}

	if( lStartColAbsolute > lEndColAbsolute )
	{
		long lTemp = lStartColAbsolute;
		lStartColAbsolute = lEndColAbsolute;
		lEndColAbsolute = lTemp;
	}

	if( lStartRowAbsolute > lEndRowAbsolute )
	{
		long lTemp = lStartRowAbsolute;
		lStartRowAbsolute = lEndRowAbsolute;
		lEndRowAbsolute = lTemp;
	}

	strOutput = _T("<?xml version=\"1.0\"?>\r\n");
	strOutput += _T("<?mso-application progid=\"Excel.Sheet\"?>\r\n");
	strOutput += _T("<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\r\n");
	strOutput += _T(" xmlns:o=\"urn:schemas-microsoft-com:office:office\"\r\n");
	strOutput += _T(" xmlns:x=\"urn:schemas-microsoft-com:office:excel\"\r\n");
	strOutput += _T(" xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\r\n");
	strOutput += _T(" xmlns:html=\"http://www.w3.org/TR/REC-html40\">\r\n");
	strOutput += _T(" <Styles>\r\n");
	strOutput += _T("  <Style ss:ID=\"Default\" ss:Name=\"Normal\">\r\n");
	strOutput += _T("   <Alignment/>\r\n");
	strOutput += _T("   <Borders/>\r\n");
	strOutput += _T("   <Font/>\r\n");
	strOutput += _T("   <Interior/>\r\n");
	strOutput += _T("   <NumberFormat/>\r\n");
	strOutput += _T("   <Protection/>\r\n");
	strOutput += _T("  </Style>\r\n");
	strOutput += _T("  <Style ss:ID=\"s66\">\r\n");
	strOutput += _T("   <NumberFormat/>\r\n");
	strOutput += _T("  </Style>\r\n");
	strOutput += _T(" </Styles>\r\n");
	strOutput += _T(" <Worksheet ss:Name=\"a\">\r\n");

	CString strColNumber;
	strColNumber.Format( _T("%i"), lEndColAbsolute - lStartColAbsolute + 1 );
	CString strRowNumber;
	strRowNumber.Format( _T("%i"), lEndRowAbsolute - lStartRowAbsolute + 1 );
	strOutput += _T("  <Table ss:ExpandedColumnCount=\"" ) + strColNumber + _T("\" ss:ExpandedRowCount=\"") + strRowNumber + _T( "\">");

	for( long lLoopRow = lStartRowAbsolute; lLoopRow <= lEndRowAbsolute; lLoopRow++ )
	{
		strOutput += _T("   <Row>\r\n");

		for( long lLoopCol = lStartColAbsolute; lLoopCol <= lEndColAbsolute; lLoopCol++ )
		{
			strOutput += _T("    <Cell ss:StyleID=\"s66\"><Data ss:Type=\"String\">");

			int iColumnID = m_mapColPosToID[lLoopCol];

			if( false == m_mapColDefList[iColumnID].IsCopyDataAllowed() )
			{
				strOutput += _T("</Data></Cell>\r\n");
				continue;
			}

			CString strText( _T( "" ) );

			switch( m_mapColDefList[iColumnID].GetContentType() )
			{
				case DlgOutputHelper::CColDef::ContentType::Text:
				case DlgOutputHelper::CColDef::ContentType::Number:
					strText = m_pclSheet->GetCellText( lLoopCol, lLoopRow );
					strOutput += strText;
					break;

				case DlgOutputHelper::CColDef::ContentType::Bitmap:
				case DlgOutputHelper::CColDef::ContentType::Param:
					break;
			}

			strOutput += _T("</Data></Cell>\r\n");
		}

		strOutput += _T("   </Row>\r\n");
	}

	strOutput += _T("  </Table>\r\n");
	strOutput += _T(" </Worksheet>\r\n");
	strOutput += _T("</Workbook>\r\n");

	return true;
}

bool CDlgOutputSpreadContainer::GetFirstRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long &lCurrentRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || NULL == pmapCellData )
	{
		return false;
	}

	m_lIterRowData = m_lColumnHeaderEnd + 1;
	lCurrentRowRelative = m_lIterRowData - m_lColumnHeaderEnd;
	return GetRow( pmapCellData, lCurrentRowRelative );
}

bool CDlgOutputSpreadContainer::GetNextRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long &lCurrentRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || NULL == pmapCellData )
	{
		return false;
	}

	m_lIterRowData++;
	lCurrentRowRelative = m_lIterRowData - m_lColumnHeaderEnd;
	return GetRow( pmapCellData, lCurrentRowRelative );
}

bool CDlgOutputSpreadContainer::GetRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long lCurrentRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || NULL == pmapCellData )
	{
		return false;
	}

	long lRowAbsolute = lCurrentRowRelative + m_lColumnHeaderEnd;
	long lRowLimit = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine )
	{
		lRowLimit--;
	}

	if( lRowAbsolute > lRowLimit )
	{
		return false;
	}

	// Check if it's well a row with the bit 'BitParam::BitData' set to 1.
	long lValue = m_pclSheet->GetCellParam( 1, lRowAbsolute );

	if( BitParam::BitData != ( lValue & BitParam::BitData ) )
	{
		return false;
	}

	for( DlgOutputHelper::mapIntCellBaseIter iter = m_mapCellBaseList.begin(); iter != m_mapCellBaseList.end(); ++iter )
	{
		if( 0 == m_mapColDefList.count( iter->first ) )
		{
			continue;
		}

		long lColAbsolute = m_mapColDefList[iter->first].GetInternalColPos();

		switch( iter->second->GetContentType() )
		{
			case DlgOutputHelper::CColDef::Text:
			{
				CString strText = m_pclSheet->GetCellText( lColAbsolute, lRowAbsolute );
				DlgOutputHelper::CCellText *pclCellText = new DlgOutputHelper::CCellText( iter->first, strText );
				( *pmapCellData )[iter->first] = pclCellText;
			}
			break;

			case DlgOutputHelper::CColDef::Number:
			{
				CString strText = m_pclSheet->GetCellText( lColAbsolute, lRowAbsolute );
				DlgOutputHelper::CCellNumber *pclCellNumber = new DlgOutputHelper::CCellNumber( iter->first, strText, m_mapColDefList[iter->first].GetPhysicalType(),
						m_mapColDefList[iter->first].GetMaxDigit(), m_mapColDefList[iter->first].GetMinDecimal() );
				( *pmapCellData )[iter->first] = pclCellNumber;
			}
			break;

			case DlgOutputHelper::CColDef::Bitmap:
			{
				int iBitmapID = ( int )m_pclSheet->GetCellParam( lColAbsolute, lRowAbsolute );
				DlgOutputHelper::CCellBitmap *pclCellBitmap = new DlgOutputHelper::CCellBitmap( iter->first, iBitmapID );
				( *pmapCellData )[iter->first] = pclCellBitmap;
			}
			break;

			case DlgOutputHelper::CColDef::Param:
			{
				long lParam = m_pclSheet->GetCellParam( lColAbsolute, lRowAbsolute );
				DlgOutputHelper::CCellParam *pclCellParam = new DlgOutputHelper::CCellParam( iter->first, ( LPARAM )lParam );
				( *pmapCellData )[iter->first] = pclCellParam;
			}
			break;
		}
	}

	return true;
}

bool CDlgOutputSpreadContainer::IsRowValidForData( long lCurrentRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	long lRowAbsolute = lCurrentRowRelative + m_lColumnHeaderEnd;
	long lRowLimit = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine )
	{
		lRowLimit--;
	}

	if( lRowAbsolute > lRowLimit )
	{
		return false;
	}

	// Check if it's well a row with the bit 'BitParam::BitData' set to 1.
	long lValue = m_pclSheet->GetCellParam( 1, lRowAbsolute );

	if( BitParam::BitData != ( lValue & BitParam::BitData ) )
	{
		return false;
	}

	return true;
}

bool CDlgOutputSpreadContainer::EnableBlockSelection( bool fSet, WORD wBorderStyle, COLORREF backgroundColor )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	m_pclSheet->BS_Activate( fSet, wBorderStyle, backgroundColor );
	return true;
}

bool CDlgOutputSpreadContainer::IsBlockSelectionExist()
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	return m_pclSheet->BS_IsSelectionExist();
}

bool CDlgOutputSpreadContainer::IsDataExistInBlockSelection( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() || false == m_pclSheet->BS_IsSelectionExist() )
	{
		return false;
	}

	long lStartColAbsolute, lStartRowAbsolute, lEndColAbsolute, lEndRowAbsolute;
	m_pclSheet->BS_GetSelection( lStartColAbsolute, lStartRowAbsolute, lEndColAbsolute, lEndRowAbsolute );
	bool fFound = false;

	for( long lLoopRow = lStartRowAbsolute; lLoopRow <= lEndRowAbsolute && false == fFound; lLoopRow++ )
	{
		for( long lLoopCol = lStartColAbsolute; lLoopCol <= lEndColAbsolute && false == fFound; lLoopCol++ )
		{
			if( 0 == m_mapColPosToID.count( lLoopCol ) )
			{
				continue;
			}

			switch( m_mapColDefList[m_mapColPosToID[lLoopCol]].GetContentType() )
			{
				case DlgOutputHelper::CColDef::ContentType::Text:
				case DlgOutputHelper::CColDef::ContentType::Number:
				{
					CString strText = m_pclSheet->GetCellText( lLoopCol, lLoopRow );

					if( false == strText.IsEmpty() )
					{
						fFound = true;
					}
				}
				break;

				case DlgOutputHelper::CColDef::ContentType::Bitmap:
					// No need for the moment.
					break;
			}
		}
	}

	return fFound;
}

bool CDlgOutputSpreadContainer::GetBlockSelectionAbsolute( long &lStartColAbsolute, long &lStartRowAbsolute, long &lEndColAbsolute, long &lEndRowAbsolute )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	return m_pclSheet->BS_GetSelection( lStartColAbsolute, lStartRowAbsolute, lEndColAbsolute, lEndRowAbsolute );
}

bool CDlgOutputSpreadContainer::GetBlockSelectionRelative( int &iStartColID, long &lStartRowRelative, int &iEndColID, long &lEndRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	iStartColID = -1;
	iEndColID = -1;
	long lStartColAbsolute, lEndColAbsolute;
	long lStartRowAbsolute, lEndRowAbsolute;

	if( false == m_pclSheet->BS_GetSelection( lStartColAbsolute, lStartRowAbsolute, lEndColAbsolute, lEndRowAbsolute ) )
	{
		return false;
	}

	if( 0 == m_mapColPosToID.count( lStartColAbsolute ) || 0 == m_mapColPosToID.count( lEndColAbsolute ) )
	{
		return false;
	}

	iStartColID = m_mapColPosToID[lStartColAbsolute];
	iEndColID = m_mapColPosToID[lEndColAbsolute];
	lStartRowRelative = lStartRowAbsolute - m_lColumnHeaderEnd;
	lEndRowRelative = lEndRowAbsolute - m_lColumnHeaderEnd;

	return true;
}

bool CDlgOutputSpreadContainer::SetBlockSelectionRelative( int iStartColID, long lStartRowRelative, int iEndColID, long lEndRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	if( 0 == m_mapColDefList.count( iStartColID ) || 0 == m_mapColDefList.count( iEndColID ) )
	{
		return false;
	}

	long lStartColAbsolute = m_mapColDefList[iStartColID].GetInternalColPos();
	long lEndColAbsolute = m_mapColDefList[iEndColID].GetInternalColPos();
	long lStartRowAbsolute = lStartRowRelative + m_lColumnHeaderEnd;
	long lEndRowAbsolute = lEndRowRelative + m_lColumnHeaderEnd;

	// Coordinates verification is done in the 'CSSheet' class.
	return m_pclSheet->BS_SetSelection( lStartColAbsolute, lStartRowAbsolute, lEndColAbsolute, lEndRowAbsolute );
}

bool CDlgOutputSpreadContainer::SetBlockSelectionAll( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	if( 0 == m_mapColDefList.size() )
	{
		return false;
	}

	long lNumRows = m_pclSheet->GetMaxRows();

	if( ( false == m_fAutomaticLastBlankLine && lNumRows == m_lColumnHeaderEnd ) ||
		( true == m_fAutomaticLastBlankLine && lNumRows == m_lColumnHeaderEnd + 1 ) )
	{
		return false;
	}

	// Check first and last column ID with the 'BlockSelectionEnabled' property set to true.
	long lStartColAbsolute = -1;

	for( DlgOutputHelper::mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end() && -1 == lStartColAbsolute; ++iter )
	{
		if( true == iter->second.IsBlockSelectionAllowed() )
		{
			lStartColAbsolute = iter->second.GetInternalColPos();
		}
	}

	if( -1 == lStartColAbsolute )
	{
		return false;
	}

	long lEndColAbsolute = -1;

	for( DlgOutputHelper::mapInColDefRITer riter = m_mapColDefList.rbegin(); riter != m_mapColDefList.rend() && -1 == lEndColAbsolute; ++riter )
	{
		if( true == riter->second.IsBlockSelectionAllowed() )
		{
			lEndColAbsolute = riter->second.GetInternalColPos();
		}
	}

	if( -1 == lEndColAbsolute )
	{
		return false;
	}

	long lStartRowAbsolute = m_lColumnHeaderEnd + 1;
	long lEndRowAbsolute = lNumRows;

	if( true == m_fAutomaticLastBlankLine )
	{
		lEndRowAbsolute--;
	}

	return m_pclSheet->BS_SetSelection( lStartColAbsolute, lStartRowAbsolute, lEndColAbsolute, lEndRowAbsolute, true );
}

bool CDlgOutputSpreadContainer::ResetBlockSelectionToOrigin( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	m_pclSheet->BS_CancelSelection( true );
	return true;
}

bool CDlgOutputSpreadContainer::EraseBlockSelection( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	m_pclSheet->BS_CancelSelection( false );
	return true;
}

bool CDlgOutputSpreadContainer::EraseDataInBlockSelection( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	return m_pclSheet->BS_DeleteDataInSelection();
}

bool CDlgOutputSpreadContainer::IsMousePointerInBlockSelection( CPoint ptMouse )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	ClientToScreen( &ptMouse );
	return m_pclSheet->BS_IsMousePointerInSelection( ptMouse );
}

void CDlgOutputSpreadContainer::EnableLineSelection( bool fSet, int iColumnIDRef, COLORREF backgroundColor )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnIDRef < 0 || 0 == m_mapColDefList.count( iColumnIDRef ) )
	{
		return;
	}

	m_pclSheet->RS_Activate( fSet, backgroundColor );
	m_pclSheet->RS_SetColumnReference( m_mapColDefList[iColumnIDRef].GetInternalColPos() );
}

bool CDlgOutputSpreadContainer::IsRowSelectionExist()
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	return m_pclSheet->RS_IsSelectionExist();
}

bool CDlgOutputSpreadContainer::GetRowSelection( long &lStartRowRelative, long &lEndRowRelative )
{
	lStartRowRelative = -1;
	lEndRowRelative = -1;

	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return false;
	}

	long lStartRow;
	long lEndRow;

	if( false == m_pclSheet->RS_GetSelection( lStartRow, lEndRow ) )
	{
		return false;
	}

	lStartRowRelative = lStartRow;
	lEndRowRelative = lEndRow;

	GetRelativeRow( lStartRowRelative );
	GetRelativeRow( lEndRowRelative );

	return true;
}

void CDlgOutputSpreadContainer::DeleteRowSelection()
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_pclSheet->BS_IsActivated() )
	{
		return;
	}

	long lStartRow;
	long lEndRow;

	if( false == m_pclSheet->RS_GetSelection( lStartRow, lEndRow ) )
	{
		return;
	}

	m_pclSheet->ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All );

	long lMaxRows = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine && lEndRow == lMaxRows )
	{
		if( lEndRow > lStartRow )
		{
			lEndRow--;
		}
		else
		{
			// There is only one line selected and it is the blank line. Do nothing in this case. 
			return;
		}
	}

	m_pclSheet->DelRowRange( lStartRow, lEndRow );
	m_pclSheet->SetMaxRows( m_pclSheet->GetMaxRows() - ( lEndRow - lStartRow + 1 ) );
}

void CDlgOutputSpreadContainer::GetRelativeRow( long &lRow )
{
	if( lRow <= m_lColumnHeaderEnd )
	{
		lRow = -1;
	}
	else
	{
		lRow -= m_lColumnHeaderEnd;
	}
}

void CDlgOutputSpreadContainer::ApplyFilter( DlgOutputHelper::CFilterList *pclFilterList )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || NULL == pclFilterList )
	{
		return;
	}

	BeginWaitCursor();

	long lRowLimit = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine )
	{
		lRowLimit--;
	}

	for( long lLoopRow = 1 + m_lColumnHeaderEnd; lLoopRow <= lRowLimit; lLoopRow++ )
	{
		_ApplyFilterRow( lLoopRow, pclFilterList );

		if( false == m_fIsOutputBlocked )
		{
			_ResizeColumns( false );
		}
	}

	EndWaitCursor();
}

void CDlgOutputSpreadContainer::Clear( bool fAlsoHeader )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || NULL == m_pclSheet->GetMaxRows() )
	{
		return;
	}

	BeginWaitCursor();

	if( true == fAlsoHeader )
	{
		m_fBlankLineBeforeMainHeaderExist = false;
		m_dBlankLineBeforeMainHeaderHeight = -1;
		m_lColumnHeaderRows = 0;
		m_lColumnHeaderStart = 0;
		m_lColumnHeaderEnd = 0;
	}

	m_pclSheet->ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All );
	m_pclSheet->DelRowRange( m_lColumnHeaderEnd + 1, m_pclSheet->GetMaxRows() );
	m_pclSheet->SetMaxRows( m_lColumnHeaderEnd );

	if( true == m_fAutomaticLastBlankLine )
	{
		_VerifyLastBlankLine();
	}

	_ResizeColumns( false );

	ResetScrolling();

	EndWaitCursor();
}

long CDlgOutputSpreadContainer::GetRows( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return -1;
	}

	long lRows = m_pclSheet->GetMaxRows() - m_lColumnHeaderEnd;

	if( true == m_fAutomaticLastBlankLine )
	{
		lRows--;
	}

	return lRows;
}

long CDlgOutputSpreadContainer::GetTotalRows( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return -1;
	}

	long lTotalRows = m_pclSheet->GetMaxRows();

	if( true == m_fAutomaticLastBlankLine )
	{
		lTotalRows--;
	}

	return lTotalRows;
}

bool CDlgOutputSpreadContainer::SelectHeaderRow( long lRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	bool fReturn = false;

	if( lRowRelative >= m_lColumnHeaderStart && lRowRelative <= m_lColumnHeaderEnd )
	{
		fReturn = true;
		m_pclSheet->UnSelectMultipleRows();

		for( mapIntColDefIter iter = m_mapHeaderDefList.begin(); iter != m_mapHeaderDefList.end(); iter++ )
		{
			if( true == iter->second.IsSelectionAllowed() )
			{
				m_pclSheet->SelectOneRow( lRowRelative, iter->second.GetInternalColPos(), iter->second.GetInternalColPos(), m_DefaultSelectionColor );
			}
		}
	}

	return fReturn;
}

bool CDlgOutputSpreadContainer::SelectDataRow( long lRowRelative, bool fAutoscroll )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	// Check if it's well a row with the bit 'BitParam::BitData' set to 1.
	long lRowAbsolute = lRowRelative + m_lColumnHeaderEnd;
	long lValue = m_pclSheet->GetCellParam( 1, lRowAbsolute );

	if( BitParam::BitData != ( lValue & BitParam::BitData ) )
	{
		return false;
	}

	m_pclSheet->UnSelectMultipleRows();
	mapLongIntIter iter = m_mapColPosToID.begin();
	mapLongIntRIter riter = m_mapColPosToID.rbegin();

	if( m_mapColPosToID.end() != iter && m_mapColPosToID.rend() != riter )
	{

		if( 0 != m_mapColDefList.count( iter->second ) && 0 != m_mapColDefList.count( riter->second ) )
		{
			m_pclSheet->SelectOneRow( lRowAbsolute, m_mapColDefList[iter->second].GetInternalColPos(), m_mapColDefList[riter->second].GetInternalColPos(),
									  m_DefaultSelectionColor );
		}

	}

	if( true == fAutoscroll )
	{
		int iFirstColID = m_mapColPosToID.begin()->second;
		m_pclSheet->ShowCell( m_mapColDefList[iFirstColID].GetInternalColPos(), lRowAbsolute, SS_SHOW_TOPLEFT );
	}

	return true;
}

bool CDlgOutputSpreadContainer::GetSelectedDataRow( long &lCurrentSelectedRowRelative )
{
	lCurrentSelectedRowRelative = -1;

	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	CArray<long> arSelectedRows;
	m_pclSheet->GetSelectedRows( &arSelectedRows );

	// Remark: normally the second condition must not happen because we have one selected row at a time.
	if( 0 == arSelectedRows.GetCount() || arSelectedRows.GetCount() > 1 )
	{
		return false;
	}

	// Check if it's well a row with the bit 'BitParam::BitData' set to 1.
	long lValue = m_pclSheet->GetCellParam( 1, arSelectedRows.GetAt( 0 ) );

	if( BitParam::BitData != ( lValue & BitParam::BitData ) )
	{
		return false;
	}

	lCurrentSelectedRowRelative = arSelectedRows.GetAt( 0 ) - m_lColumnHeaderEnd;
	return true;
}

bool CDlgOutputSpreadContainer::SaveSelections( CSSheet::SaveSelectionChoice eWhichSelection )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	m_pclSheet->SaveSelections( eWhichSelection );
	return true;
}

bool CDlgOutputSpreadContainer::ClearCurrentSelections( CSSheet::SaveSelectionChoice eWhichSelection )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	m_pclSheet->ClearCurrentSelections( eWhichSelection );
	return true;
}

bool CDlgOutputSpreadContainer::RestoreSelections( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	m_pclSheet->RestoreSelections();
	return true;
}

bool CDlgOutputSpreadContainer::GetColID2Absolute( int iColumnID, long &lColAbsolute )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return false;
	}

	if( 0 == m_mapHeaderDefList.count( iColumnID ) && 0 == m_mapColDefList.count( iColumnID ) )
	{
		return false;
	}

	if( m_mapHeaderDefList.count( iColumnID ) > 0 )
	{
		lColAbsolute = m_mapHeaderDefList[iColumnID].GetInternalColPos();
	}
	else
	{
		lColAbsolute = m_mapColDefList[iColumnID].GetInternalColPos();
	}

	return true;
}

bool CDlgOutputSpreadContainer::ResetCellContent( int iColumnID, long lFromRowRelative, long lToRowRelative )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iColumnID < 0 )
	{
		return false;
	}

	if( 0 == m_mapColDefList.count( iColumnID ) )
	{
		return false;
	}

	long lColAbsolutePos = m_mapColDefList[iColumnID].GetInternalColPos();

	// Remark: first and last columns are reserved for internal use.
	if( lColAbsolutePos < 2 || lColAbsolutePos >= m_pclSheet->GetMaxCols() )
	{
		return false;
	}

	// Check if it's well a row with the bit 'BitParam::BitData' set to 1.
	long lFromRowAbsolute = lFromRowRelative + m_lColumnHeaderEnd;
	long lToRowAbsolute = ( -1 == lToRowRelative ) ? lFromRowAbsolute : lToRowRelative + m_lColumnHeaderEnd;

	for( long lLoopRow = lFromRowAbsolute; lLoopRow <= lToRowAbsolute; lLoopRow++ )
	{
		long lValue = m_pclSheet->GetCellParam( 1, lLoopRow );

		if( BitParam::BitData != ( lValue & BitParam::BitData ) )
		{
			return false;
		}
	}

	switch( m_mapColDefList[iColumnID].GetContentType() )
	{
		case CColDef::Text:
		case CColDef::Number:
			_SendOutputText( _T( "" ), lColAbsolutePos, lFromRowAbsolute, lToRowAbsolute, &m_mapColDefList[iColumnID].GetFontDef() );
			break;

		case CColDef::Bitmap:
			_SendOutputBitmap( &CCellBitmap( iColumnID ), lColAbsolutePos, lFromRowAbsolute );
			break;
	}

	return true;
}

bool CDlgOutputSpreadContainer::IsCellCanPasteData( int iColumnID )
{
	bool fReturn;

	if( iColumnID < 0 || 0 == m_mapColDefList.count( iColumnID ) )
	{
		fReturn = false;
	}
	else
	{
		fReturn = m_mapColDefList[iColumnID].IsPasteDataAllowed();
	}

	return fReturn;
}

bool CDlgOutputSpreadContainer::ChangeCellBackground( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative, COLORREF crColor,
		bool fForceColorWhenSelection )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || iStartColID < 0 || iEndColID < 0 )
	{
		return false;
	}

	if( 0 == m_mapColDefList.count( iStartColID ) || 0 == m_mapColDefList.count( iEndColID ) )
	{
		return false;
	}

	long lStartColAbsolutePos = m_mapColDefList[iStartColID].GetInternalColPos();

	// Remark: first and last columns are reserved for internal use.
	if( lStartColAbsolutePos < 2 || lStartColAbsolutePos >= m_pclSheet->GetMaxCols() )
	{
		return false;
	}

	long lEndColAbsolutePos = m_mapColDefList[iEndColID].GetInternalColPos();

	// Remark: first and last columns are reserved for internal use.
	if( lEndColAbsolutePos < 2 || lEndColAbsolutePos >= m_pclSheet->GetMaxCols() )
	{
		return false;
	}

	if( lStartColAbsolutePos > lEndColAbsolutePos )
	{
		long lTemp = lStartColAbsolutePos;
		lStartColAbsolutePos = lEndColAbsolutePos;
		lEndColAbsolutePos = lTemp;
	}

	// Check if it's well a row with the bit 'BitParam::BitData' set to 1.
	long lStartRowAbsolute = lStartRowRelative + m_lColumnHeaderEnd;
	long lEndRowAbsolute = lEndRowRelative + m_lColumnHeaderEnd;

	if( lStartRowAbsolute > lEndRowAbsolute )
	{
		long lTemp = lStartRowAbsolute;
		lStartRowAbsolute = lEndRowAbsolute;
		lEndRowAbsolute = lTemp;
	}

	for( long lLoopRow = lStartRowAbsolute; lLoopRow <= lEndRowAbsolute; lLoopRow++ )
	{
		long lValue = m_pclSheet->GetCellParam( 1, lLoopRow );

		if( BitParam::BitData != ( lValue & BitParam::BitData ) )
		{
			return false;
		}
	}

	m_pclSheet->SetBackColor( lStartColAbsolutePos, lStartRowAbsolute, lEndColAbsolutePos, lEndRowAbsolute, crColor, fForceColorWhenSelection );

	return true;
}

bool CDlgOutputSpreadContainer::ResizingColumnActivate( bool fActivate )
{
	if( NULL == m_pclResizingColumnInfos || NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	m_pclResizingColumnInfos->ActivateFeature( fActivate, true );
	m_fResizingColumnActive = fActivate;

	if( true == fActivate )
	{
		m_pclSheet->MM_RegisterNotificationHandler( this, CMessageManager::SSheetNHFlags::SSheetNHF_ColWidthChanged );
	}
	else
	{
		m_pclSheet->MM_UnregisterNotificationHandler( this, CMessageManager::SSheetNHFlags::SSheetNHF_ColWidthChanged );
	}

	return true;
}

bool CDlgOutputSpreadContainer::ResizingColumnAddRange( long lColStartAbsolutePos, long lColEndAbsolutePos, long lRowStartAbsolute, long lRowEndAbsolute )
{
	if( NULL == m_pclResizingColumnInfos || NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	if( 0 == m_mapColPosToID.count( lColStartAbsolutePos ) || 0 == m_mapColPosToID.count( lColEndAbsolutePos ) )
	{
		return false;
	}

	// Remark: first and last columns are reserved for internal use.
	if( lColStartAbsolutePos < 2 || lColEndAbsolutePos < 2 || lColStartAbsolutePos >= m_pclSheet->GetMaxCols() || lColEndAbsolutePos >= m_pclSheet->GetMaxCols() )
	{
		return false;
	}

	if( lRowStartAbsolute < 1 || lRowEndAbsolute < 1 || lRowStartAbsolute > m_pclSheet->GetMaxRows() || lRowEndAbsolute > m_pclSheet->GetMaxRows() )
	{
		return false;
	}

	if( lColStartAbsolutePos > lColEndAbsolutePos )
	{
		long lTemp = lColStartAbsolutePos;
		lColStartAbsolutePos = lColEndAbsolutePos;
		lColEndAbsolutePos = lTemp;
	}

	if( lRowStartAbsolute > lRowEndAbsolute )
	{
		long lTemp = lRowStartAbsolute;
		lRowStartAbsolute = lRowEndAbsolute;
		lRowEndAbsolute = lTemp;
	}

	return m_pclResizingColumnInfos->AddRangeColumn( lColStartAbsolutePos, lColEndAbsolutePos, lRowStartAbsolute, lRowEndAbsolute );
}

bool CDlgOutputSpreadContainer::ResizingColumnRemoveAll( void )
{
	if( NULL == m_pclResizingColumnInfos || NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	return m_pclResizingColumnInfos->RemoveAllColumn();
}

bool CDlgOutputSpreadContainer::GetCellFromPixel( CPoint point, int &iColumnID, long &lRowRelative, long &lRowAbsolute )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return false;
	}

	iColumnID = -1;
	lRowRelative = -1;
	lRowAbsolute = -1;

	// Convert mouse position relative to sheet.
	m_pclSheet->ScreenToClient( &point );

	// Determine col and row.
	long lClickedCol, lClickedRow;
	m_pclSheet->GetCellFromPixel( &lClickedCol, &lClickedRow, point.x, point.y );

	if( lClickedCol < 1 || lClickedRow < 1 )
	{
		return false;
	}

	if( 0 == m_mapColPosToID.count( lClickedCol ) )
	{
		return false;
	}

	iColumnID = m_mapColPosToID[lClickedCol];
	lRowAbsolute = lClickedRow;

	if( lRowAbsolute > m_lColumnHeaderEnd )
	{
		lRowRelative = lRowAbsolute - m_lColumnHeaderEnd;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgOutputSpreadContainer, CDialogEx )
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	// Spread DLL messages
	ON_MESSAGE( SSM_TEXTTIPFETCH, OnTextTipFetch )
	ON_MESSAGE( SSM_EDITMODEOFF, OnEditModeOff )
	ON_MESSAGE( WM_USER_MESSAGEDATA, MM_OnMessageReceive )
END_MESSAGE_MAP()

void CDlgOutputSpreadContainer::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

BOOL CDlgOutputSpreadContainer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Create an instance of CSSheet.
	BOOL fReturn = FALSE;
	m_pclSheet = new CSSheet( );

	if( NULL != m_pclSheet )
	{
		// Create CSSheet.
		fReturn = m_pclSheet->Create( ( GetStyle() | WS_CHILD | WS_VISIBLE & ~WS_BORDER ), CRect( 0, 0, 0, 0 ), this, IDC_FPSPREAD );

		if( TRUE == fReturn )
		{
			CRect rectClient;
			GetClientRect( &rectClient );
			m_pclSheet->SetWindowPos( NULL, 0, 0, rectClient.right, rectClient.bottom, SWP_NOACTIVATE | SWP_NOZORDER );

			_InitializeSSheet();
		}
	}

	return fReturn;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgOutputSpreadContainer::OnDestroy()
{
	m_mapColDefList.clear();
}

void CDlgOutputSpreadContainer::OnPaint()
{
	SetBackgroundColor( _WHITE, 0 );
	CDialogEx::OnPaint();
}

void CDlgOutputSpreadContainer::OnSize( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize( nType, cx, cy );

	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return;
	}

	m_pclSheet->SetWindowPos( NULL, -1, -1, cx, cy, SWP_NOZORDER | SWP_NOMOVE );
	m_pclSheet->EmptyMessageQueue();

	_ResizeColumns( false );
}

void CDlgOutputSpreadContainer::OnCancel()
{
	// MSDN:
	// If you implement the Cancel button in a modeless dialog box, you must override the OnCancel member function and
	// call DestroyWindow from within it. Don't call the base-class member function, because it calls EndDialog, which
	// will make the dialog box invisible but not destroy it.

	// http://www.flounder.com/kb103788.htm
	// PS: and an <esc> key can generate a WM_COMMAND:IDCANCEL message (which will invoke the OnCancel virtual method)
}

void CDlgOutputSpreadContainer::OnOK()
{
	// MSDN:
	// If you implement the Cancel button in a modeless dialog box, you must override the OnCancel member function and
	// call DestroyWindow from within it. Don't call the base-class member function, because it calls EndDialog, which
	// will make the dialog box invisible but not destroy it.

	// http://www.flounder.com/kb103788.htm
	// PS: an <enter> key can generate a WM_COMMAND:IDOK message (which will invoke the OnOK virtual method).
}

LRESULT CDlgOutputSpreadContainer::MM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
{
	CMessageManager::CMessage *pclMessage = CMessageManager::MM_ReadMessage( wParam );

	if( NULL == pclMessage )
	{
		return 0;
	}

	if( CMessageManager::ClassID::CID_CSSheet != ( CMessageManager::ClassID )pclMessage->GetClassID() )
	{
		delete pclMessage;
		return 0;
	}

	switch( ( CMessageManager::SSheetMessageType )pclMessage->GetMessageType() )
	{
		case CMessageManager::SSheetMessageType::SSheetMST_MouseEvent:
		{
			CMessageManager::CSSheetMEMsg *pclSShetMEMsg = dynamic_cast<CMessageManager::CSSheetMEMsg *>( pclMessage );

			if( NULL == pclSShetMEMsg )
			{
				delete pclMessage;
				return 0;
			}

			SSheetMEParams *prParams = pclSShetMEMsg->GetParams();

			if( NULL == prParams->m_hSheet || prParams->m_hSheet != m_pclSheet->GetSafeHwnd() )
			{
				delete pclMessage;
				return 0;
			}

			switch( prParams->m_eMouseEventType )
			{
				case CMessageManager::SSheetMET::MET_LButtonDown:
					_OnLButtonDown( prParams->m_lColumn, prParams->m_lRow, prParams->m_ptMouse );
					break;

				case CMessageManager::SSheetMET::MET_LButtonDblClk:
					_OnLButtonDblClk( prParams->m_lColumn, prParams->m_lRow, prParams->m_ptMouse );
					break;

				case CMessageManager::SSheetMET::MET_RButtonDown:
					_OnRButtonDown( prParams->m_lColumn, prParams->m_lRow, prParams->m_ptMouse );
					break;
			}

			if( true == m_fResizingColumnActive )
			{
				// Send manually to 'CViewDescription' for the class 'CResizingColumnInfo'. Because at now, 'MM_OnMessageReceive' messages are only sent to a 'CWnd'
				// and 'CResizingColumnInfo' is not.
				m_ViewDescription.OnMouseEvent( m_pclSheet, prParams->m_eMouseEventType, prParams->m_lColumn, prParams->m_lRow, prParams->m_ptMouse );
			}
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

			SSheetKEParams *prParams = pclSShetKEMsg->GetParams();

			if( NULL == prParams->m_hSheet || prParams->m_hSheet != m_pclSheet->GetSafeHwnd() )
			{
				delete pclMessage;
				return 0;
			}

			switch( prParams->m_eKeyboardEventType )
			{
				case CMessageManager::SSheetKET::KET_Shortcut:
					if( m_mapNotificationHandlerList.count( INotificationHandler::NH_OnKeyboardShortcut ) > 0 )
					{
						m_mapNotificationHandlerList[INotificationHandler::NH_OnKeyboardShortcut]->OnOutputSpreadContainerKeyboardShortcut( m_iOutputID, prParams->m_iKeyCode );
					}

					break;

				case CMessageManager::SSheetKET::KET_VirtualKeyDown:
					if( m_mapNotificationHandlerList.count( INotificationHandler::NH_OnKeyboardVirtualKey ) > 0 )
					{
						m_mapNotificationHandlerList[INotificationHandler::NH_OnKeyboardShortcut]->OnOutputSpreadContainerKeyboardVirtualKey( m_iOutputID, prParams->m_iKeyCode );
					}

					break;
			}
		}
		break;

		case CMessageManager::SSheetMessageType::SSheetMST_NavigateOccurs:
		{
			CMessageManager::CSSheetNOMsg *pclSShetNOMsg = dynamic_cast<CMessageManager::CSSheetNOMsg *>( pclMessage );

			if( NULL == pclSShetNOMsg )
			{
				delete pclMessage;
				return 0;
			}

			SSheetNOParams *prParams = pclSShetNOMsg->GetParams();

			if( NULL == prParams->m_hSheet || prParams->m_hSheet != m_pclSheet->GetSafeHwnd() )
			{
				delete pclMessage;
				return 0;
			}

			if( prParams->m_lNewRow != prParams->m_lOldRow )
			{
				if( prParams->m_lNewRow >= m_lColumnHeaderStart && prParams->m_lNewRow <= m_lColumnHeaderEnd )
				{
					SelectHeaderRow( prParams->m_lNewRow );
				}
				else if( prParams->m_lNewRow > m_lColumnHeaderEnd )
				{
					SelectDataRow( prParams->m_lNewRow - m_lColumnHeaderEnd, false );
				}
			}
		}
		break;
	}

	delete pclMessage;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Spread DLL message handlers
/////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CDlgOutputSpreadContainer::OnTextTipFetch( WPARAM wParam, LPARAM lParam )
{
	if( false == m_fTextTipFetchEnabled || TRUE == m_mapTextTipFetchList.IsEmpty() )
	{
		return 0;
	}

	SS_TEXTTIPFETCH *pTextTipFetch = ( SS_TEXTTIPFETCH * )lParam;

	mapLongMapIntCellBase::CPair *pclPairRowMap = m_mapTextTipFetchList.PLookup( (long)pTextTipFetch->Row );

	if( NULL == pclPairRowMap )
	{
		return 0;
	}

	int iColumnID = -1;
	for( DlgOutputHelper::mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
	{
		if( pTextTipFetch->Col == iter->second.GetInternalColPos() )
		{
			iColumnID = iter->first;
			break;
		}
	}

	if( -1 == iColumnID )
	{
		return 0;
	}

	DlgOutputHelper::CCellBase *pclCellBase = NULL;

	if( FALSE == pclPairRowMap->value->Lookup( iColumnID, pclCellBase ) || NULL == pclCellBase )
	{
		return 0;
	}

	// If hText is not empty, spread takes hText in place of szText!
	pTextTipFetch->hText = NULL;
	pTextTipFetch->fShow = true;
	pTextTipFetch->wMultiLine = SS_TT_MULTILINE_AUTO;
	CString str = pclCellBase->GetInternalHelper();
	pTextTipFetch->nWidth = ( SHORT )m_pclSheet->GetTipTextWidth( str );
	wcsncpy_s( pTextTipFetch->szText, SS_TT_TEXTMAX, ( LPCTSTR )str, SS_TT_TEXTMAX );

	return 0;
}

LRESULT CDlgOutputSpreadContainer::OnEditModeOff( WPARAM wParam, LPARAM lParam )
{
	SS_EDITMODEOFF *pclEditModeOff = ( SS_EDITMODEOFF * )lParam;

	// Verify if the row where user has edited a value is the last. To check if we must add a automatic blank line.
	if( true == m_fAutomaticLastBlankLine && TRUE == pclEditModeOff->fChangeMade && pclEditModeOff->Row == m_pclSheet->GetMaxRows() )
	{
		long lValue = m_pclSheet->GetCellParam( 1, pclEditModeOff->Row );

		if( BitParam::BitBlankLine == ( lValue & BitParam::BitBlankLine ) )
		{
			bool fIsCurrentlyBlocked = IsRedrawBlocked();

			if( false == fIsCurrentlyBlocked )
			{
				BlockRedraw();
			}

			m_pclSheet->SaveSelections( CSSheet::SaveSelectionChoice::SSC_All );
			m_pclSheet->ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All );

			// We retrieve the value of this cell just edited.
			CString strData = m_pclSheet->GetCellText( pclEditModeOff->Col, pclEditModeOff->Row );
			m_pclSheet->SetCellText( pclEditModeOff->Col, pclEditModeOff->Row, _T( "" ) );

			// Insert this new data row.
			_AddRows( BitParam::BitData, pclEditModeOff->Row, 1, m_dDataRowHeight, true );

			// Restore text.
			m_pclSheet->SetCellText( pclEditModeOff->Col, pclEditModeOff->Row, strData );

			m_pclSheet->RestoreSelections();

			if( false == fIsCurrentlyBlocked )
			{
				ReleaseRedraw();
			}
		}
	}

	if( m_mapNotificationHandlerList.count( INotificationHandler::NH_OnCellEditModeOff ) > 0 )
	{
		int iColumnID = ( 0 != m_mapColPosToID.count( pclEditModeOff->Col ) ) ? m_mapColPosToID[pclEditModeOff->Col] : -1;
		m_mapNotificationHandlerList[INotificationHandler::NH_OnCellEditModeOff]->OnOutputSpreadContainerCellEditModeOff( m_iOutputID, iColumnID, pclEditModeOff->Row,
				( TRUE == pclEditModeOff->fChangeMade ) ? true : false );
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgOutputSpreadContainer::_InitializeSSheet( void )
{
	m_pclSheet->Init();
	m_pclSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	m_pclSheet->SetBool( SSB_HORZSCROLLBAR, TRUE );
	m_pclSheet->SetBool( SSB_VERTSCROLLBAR, TRUE );

	m_pclSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	m_pclSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );
	m_pclSheet->SetBool( SSB_GRIDLINES, FALSE );

	// Set max cols.
	// Remark: the first column is reserved for internal use. The last column is used for resizing.
	m_pclSheet->SetMaxCols( 2 );
	m_pclSheet->ShowCol( 1, FALSE );

	// By default set the last column to 0 pixels. Because if we use auto resize column feature, we don't need this last column.
	m_pclSheet->SetColWidthInPixels( 2, 0 );
	m_pclSheet->ShowCol( 2, FALSE );

	// Set max rows.
	m_pclSheet->SetMaxRows( 0 );

	// All cells are static by default and filled with _T("").
	m_pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	m_pclSheet->MM_RegisterNotificationHandler( this,	CMessageManager::SSheetNHFlags::SSheetNHF_MouseMove |
			CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDown |
			CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDblClk |
			CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonUp |
			CMessageManager::SSheetNHFlags::SSheetNHF_MouseRButtonDown |
			CMessageManager::SSheetNHFlags::SSheetNHF_MouseCaptureChanged |
			CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardShortcut |
			CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyDown |
			CMessageManager::SSheetNHFlags::SSheetNHF_NavigateOccurs );

	if( true == m_fAutomaticLastBlankLine )
	{
		_VerifyLastBlankLine();
	}

	CSheetDescription *pclSheetDescription = m_ViewDescription.AddSheetDescription( SheetDescription::SD_Output, -1, m_pclSheet, CPoint( 0, 0 ) );

	if( NULL == pclSheetDescription )
	{
		return;
	}

	m_pclResizingColumnInfos = pclSheetDescription->GetResizingColumnInfo();
}

void CDlgOutputSpreadContainer::_ResizeColumns( bool fScrollOnTheLastLine )
{
	CRect rectClient;
	GetClientRect( &rectClient );

	// Resize spread taking in count the true height and width available.
	long lVScrollBarWidth = ( TRUE == m_pclSheet->IsVerticalScrollBarVisible() ) ? GetSystemMetrics( SM_CXVSCROLL ) : 0;

	// If columns are not yet resized and if there is at least one column defined...
	// Remark: '>2' because the first and the last one are for internal use.
	if( m_pclSheet->GetMaxCols() > 2 )
	{
		if( false == m_fAutoResize )
		{
			// Resize the last column to have separator taking all the width.
			long lParamWidth = max( 0, rectClient.Width() - m_lSpreadMinWidth - lVScrollBarWidth );
			m_pclSheet->SetColWidthInPixels( m_pclSheet->GetMaxCols(), lParamWidth );
		}
		else
		{
			if( rectClient.Width() >= m_lSpreadMinWidth + lVScrollBarWidth )
			{
				// Check what is the initial size of all the columns that can be resized.
				int iInitialResizedWidth = 0;
				int iInitialFixedWidth = 0;

				for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
				{
					if( true == iter->second.GetAutoResized() )
					{
						iInitialResizedWidth += iter->second.GetWidthInPixels();
					}
					else
					{
						iInitialFixedWidth += iter->second.GetWidthInPixels();
					}
				}

				int iAvailableWidth = rectClient.Width() - iInitialFixedWidth - lVScrollBarWidth;

				for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
				{
					if( true == iter->second.GetAutoResized() )
					{
						double dRatio = ( double )iter->second.GetWidthInPixels() / ( double )iInitialResizedWidth;
						m_pclSheet->SetColWidthInPixels( iter->second.GetInternalColPos(), ( int )( ( double )iAvailableWidth * dRatio ) );
					}
				}
			}
			else
			{
				// Otherwise we apply the initial width.
				for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
				{
					if( true == iter->second.GetAutoResized() )
					{
						m_pclSheet->SetColWidthInPixels( iter->second.GetInternalColPos(), iter->second.GetWidthInPixels() );
					}
				}
			}
		}
	}

	// If we have just added a message, we try to scroll down at the last line.
	if( true == fScrollOnTheLastLine )
	{
		int iFirstColID = m_mapColPosToID.begin()->second;
		long lRowLimit = m_pclSheet->GetMaxRows();

		if( true == m_fAutomaticLastBlankLine )
		{
			lRowLimit--;
		}

		m_pclSheet->ShowCell( m_mapColDefList[iFirstColID].GetInternalColPos(), lRowLimit, SS_SHOW_BOTTOMLEFT );
	}
}

void CDlgOutputSpreadContainer::_ExecutePreJob( DlgOutputHelper::CMessageBase *pclMessage, long lRowAbsolute )
{
	if( NULL == pclMessage )
	{
		return;
	}

	DlgOutputHelper::CPreJobBase *pclPreJob = pclMessage->GetFirstPreJob();

	while( pclPreJob != NULL )
	{
		switch( pclPreJob->GetPreJobType() )
		{
			case DlgOutputHelper::PreJobType::Spaning:
			{
				DlgOutputHelper::CPreJobSpan *pclPreJobSpan = dynamic_cast<DlgOutputHelper::CPreJobSpan *>( pclPreJob );

				if( NULL == pclPreJobSpan )
				{
					break;
				}

				long lFromColumnID, lToColumnID;
				pclPreJobSpan->GetRange( lFromColumnID, lToColumnID );

				if( 0 == m_mapColDefList.count( lFromColumnID ) || 0 == m_mapColDefList.count( lToColumnID ) )
				{
					break;
				}

				long lFromColAbsolutePos = m_mapColDefList[lFromColumnID].GetInternalColPos();
				long lToColAbsolutePos = m_mapColDefList[lToColumnID].GetInternalColPos();

				if( lFromColAbsolutePos < 2 || lFromColAbsolutePos >= m_pclSheet->GetMaxCols() || lToColAbsolutePos < 2 || lToColAbsolutePos >= m_pclSheet->GetMaxCols() )
				{
					break;
				}

				long lNumCol = abs( lToColAbsolutePos - lFromColAbsolutePos ) + 1;
				m_pclSheet->AddCellSpanW( lFromColAbsolutePos, lRowAbsolute, lNumCol, 1 );
				break;
			}
		}

		pclPreJob = pclMessage->GetNextPreJob();
	}
}

bool CDlgOutputSpreadContainer::_SendOutputText( CString strText, long lColAbsolute, long lFromRowAbsolute, long lToRowAbsolute,
		DlgOutputHelper::CFontDef *pclFontDef )
{
	if( NULL == pclFontDef )
	{
		return false;
	}

	// Set the font value.
	m_pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontName, ( LPARAM )&pclFontDef->GetFontName() );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )pclFontDef->GetSize() );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )pclFontDef->GetInternalHorzAlign() );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleVerticalAlign, ( LPARAM )pclFontDef->GetInternalVertAlign() );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )pclFontDef->GetBold() );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontItalic, ( LPARAM )pclFontDef->GetItalic() );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontUnderline, ( LPARAM )pclFontDef->GetUnderline() );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )pclFontDef->GetTextForegroundColor() );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )pclFontDef->GetTextBackgroundColor() );

	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, ( LPARAM )TRUE );

	// Set text.
	if( false == m_pclSheet->ApplyTextPattern( lColAbsolute, lFromRowAbsolute, lColAbsolute, lToRowAbsolute ) )
	{
		return false;
	}

	if( false == m_pclSheet->SetDataRange( lColAbsolute, lFromRowAbsolute, lColAbsolute, lToRowAbsolute, strText ) )
	{
		return false;
	}

	return true;
}

bool CDlgOutputSpreadContainer::_SendOutputBitmap( CCellBitmap *pclCellBitmap, long lColAbsolute, long lRowAbsolute )
{
	// Set bitmap.
	bool fReturn = false;

	if( NULL != pclCellBitmap && -1 != pclCellBitmap->GetBitmapID() )
	{
		if( true == m_pclSheet->SetPictureCellWithID( pclCellBitmap->GetBitmapID(), lColAbsolute, lRowAbsolute, CSSheet::PictureCellType::Icon ) )
		{
			m_pclSheet->SetCellParam( lColAbsolute, lRowAbsolute, pclCellBitmap->GetBitmapID() );
			fReturn = true;
		}
	}
	else if( NULL == pclCellBitmap || -1 == pclCellBitmap->GetBitmapID() )
	{
		m_pclSheet->SetCellParam( lColAbsolute, lRowAbsolute, -1 );

		// We must call 'ClearData' otherwise 'TSpread' doesn't clear the handle on the previous bitmap.
		m_pclSheet->Clear( lColAbsolute, lRowAbsolute );

		// Restore border.
		if( 0 != (int)m_mapColDefList.count( pclCellBitmap->GetColumnID() ) )
		{
			if( true == m_mapColDefList[pclCellBitmap->GetColumnID()].GetRowSeparatorFlag() )
			{
				m_pclSheet->SetBorder( lColAbsolute, lRowAbsolute, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
						m_mapColDefList[pclCellBitmap->GetColumnID()].GetRowSeparatorColor() );
			}
		}
	}

	return fReturn;
}

bool CDlgOutputSpreadContainer::_SendOutputParam( CCellParam *pclCellParam, long lColAbsolute, long lRowAbsolute )
{
	if( NULL == pclCellParam )
	{
		return false;
	}

	// Set param.
	m_pclSheet->SetCellParam( lColAbsolute, lRowAbsolute, ( long )pclCellParam->GetParam() );
	return true;
}

bool CDlgOutputSpreadContainer::_ReadCell( DlgOutputHelper::CCellMaster *pclCellMaster, long lColAbsolute, long lRowAbsolute )
{
	DlgOutputHelper::CCellBase *pclCellBase = pclCellMaster->GetCellBase();

	if( NULL == pclCellBase )
	{
		return false;
	}

	bool fReturn = true;

	switch( pclCellBase->GetContentType() )
	{
		case DlgOutputHelper::CColDef::ContentType::Text:
		case DlgOutputHelper::CColDef::ContentType::Number:
			( ( DlgOutputHelper::CCellText * )pclCellBase )->SetText( m_pclSheet->GetCellText( lColAbsolute, lRowAbsolute ) );
			break;

		case DlgOutputHelper::CColDef::ContentType::Bitmap:
			( ( DlgOutputHelper::CCellBitmap * )pclCellBase )->SetBitmapID( m_pclSheet->GetCellParam( lColAbsolute, lRowAbsolute ) );
			break;

		case DlgOutputHelper::CColDef::ContentType::Param:
			( ( DlgOutputHelper::CCellParam * )pclCellBase )->SetParam( m_pclSheet->GetCellParam( lColAbsolute, lRowAbsolute ) );
			break;

		default:
			fReturn = false;
	}

	return fReturn;
}

void CDlgOutputSpreadContainer::_ApplyFilterRow( long lRowAbsolute, DlgOutputHelper::CFilterList *pclFilterList )
{
	if( NULL == pclFilterList )
	{
		return;
	}

	// Check if it's well a row with the bit 'BitParam::BitData' set to 1.
	long lValue = m_pclSheet->GetCellParam( 1, lRowAbsolute );

	if( BitParam::BitData != ( lValue & BitParam::BitData ) )
	{
		return;
	}

	// Run all filters.
	DlgOutputHelper::CCellFilterList *pclCellFilterList = pclFilterList->GetFirstCellFilter();

	while( pclCellFilterList != NULL )
	{
		// Run all conditions for one filter.
		BOOL fShow = TRUE;
		DlgOutputHelper::CCellFilter *pclCellFilter = pclCellFilterList->GetFirstFilter();

		while( pclCellFilter != NULL )
		{
			// Verify if condition is ok.
			if( m_mapColDefList.count( pclCellFilter->GetColumnID() ) > 0 &&
				m_mapColDefList[pclCellFilter->GetColumnID()].GetContentType() == pclCellFilter->GetContentType() )
			{
				CCellMaster clCellMaster( pclCellFilter->GetColumnID(), pclCellFilter->GetContentType() );
				long lColAbsolutePos = m_mapColDefList[pclCellFilter->GetColumnID()].GetInternalColPos();

				if( true == _ReadCell( &clCellMaster, lColAbsolutePos, lRowAbsolute ) )
				{
					// If filter is active and the current condition is OK...
					if( true == pclCellFilterList->GetFlagActive() && true == pclCellFilter->CanFilter( clCellMaster.GetCellBase() ) )
					{
						fShow = FALSE;
					}
					else
					{
						fShow = TRUE;
					}
				}
			}

			pclCellFilter = pclCellFilterList->GetNextFilter();
		}

		m_pclSheet->ShowRow( lRowAbsolute, fShow );

		// As soon as a row has been hidden, no more need to run all filters.
		if( FALSE == fShow )
		{
			break;
		}

		pclCellFilterList = pclFilterList->GetNextCellFilter();
	}
}

void CDlgOutputSpreadContainer::_AddRows( BitParam eWhatToInsert, long lWhereToInsertAbsolute, long lNumberOfRow, double dRowHeight, bool fInsert )
{
	m_pclSheet->ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All );
	long lCurrentMaxRows = m_pclSheet->GetMaxRows();

	bool fRowAdded = false;
	bool fAddBlankLine = false;

	if( false == fInsert )
	{
		long lLimit = lCurrentMaxRows;

		if( true == m_fAutomaticLastBlankLine )
		{
			lLimit--;
		}

		long lRowEnd = lWhereToInsertAbsolute + lNumberOfRow - 1;

		if( lRowEnd > lLimit )
		{
			// We delete the range of row to be sure to clean bits in the first column and all data.
			m_pclSheet->DelRowRange( lWhereToInsertAbsolute, lRowEnd );
			m_pclSheet->SetMaxRows( lRowEnd );
			fRowAdded = true;

			if( true == m_fAutomaticLastBlankLine )
			{
				fAddBlankLine = true;
			}
		}
	}
	else
	{
		if( lWhereToInsertAbsolute <= lCurrentMaxRows )
		{
			// Pay attention: in regards to the Farpoint documentation, inserting row does not increase the total number of rows in the sheet.
			// We must thus first increase with 'SetMaxRows'.
			m_pclSheet->SetMaxRows( lCurrentMaxRows + lNumberOfRow );

			if( 1 == lNumberOfRow )
			{
				m_pclSheet->InsRow( lWhereToInsertAbsolute );
			}
			else
			{
				m_pclSheet->InsRowRange( lWhereToInsertAbsolute, lWhereToInsertAbsolute + lNumberOfRow - 1 );
			}

			fRowAdded = true;
		}
	}

	if( true == fRowAdded )
	{
		// Change height if defined.
		if( -1.0 != dRowHeight )
		{
			for( long lLoopRow = lWhereToInsertAbsolute; lLoopRow < lWhereToInsertAbsolute + lNumberOfRow; lLoopRow++ )
			{
				m_pclSheet->SetRowHeight( lLoopRow, dRowHeight );
			}
		}

		// We need now to set the good properties for these new cells.
		long lEndRow = lWhereToInsertAbsolute + lNumberOfRow - 1;
		_PrepareCellsInRows( eWhatToInsert, lWhereToInsertAbsolute, lEndRow );

		// Remark: We must add the blank line at the end. Because in the beginning of this method, we test the 'BitParam' flag to check if blank line is already
		// existing. This flag is only reset by calling 'SetCellParam' just above.
		if( true == fAddBlankLine )
		{
			_VerifyLastBlankLine();
		}
	}
}

void CDlgOutputSpreadContainer::_PrepareCellsInRows( BitParam eWhatToPrepare, long lFromRow, long lToRow, long lColumn )
{
	if( lFromRow < 1 || lToRow < 1 || lFromRow > m_pclSheet->GetMaxRows() || lToRow > m_pclSheet->GetMaxRows() )
	{
		return;
	}

	if( lFromRow > lToRow )
	{
		long lTemp = lFromRow;
		lFromRow = lToRow;
		lToRow = lTemp;
	}

	if( -1 != lColumn && ( lColumn < 2 || lColumn > m_pclSheet->GetMaxCols() - 1 ) )
	{
		return;
	}

	long lColumnStart = ( -1 == lColumn ) ? 2 : lColumn;
	long lColumnEnd = ( -1 == lColumn ) ? m_pclSheet->GetMaxCols() - 1 : lColumn;

	// TSpread documentation - Cell type settings remembered:
	// When you are working with cell type settings in the ActiveX controls, you must be aware that the controls remember cell type settings.
	// For example, if you create a date cell and set the TypeDateSeparator property for that cell, subsequent date cells you create will have
	// the same TypeDateSeparator setting.
	// This is why when calling '_AddRows' method (the method that called '_PrepareCellInRows'), we don't know what is the last cell type used.
	// And to be sure, we reset all new cells created to be static text.
	SS_CELLTYPE rCellType;
	m_pclSheet->SetTypeStaticText( &rCellType, 0 );
	m_pclSheet->SetCellTypeRange( lColumnStart, lFromRow, lColumnEnd, lToRow, &rCellType );

	switch( eWhatToPrepare )
	{
		case BitParam::BitBlankLineBeforeMH:
		{
			long lProperties = CSSheet::_SSCellProperty::CellNoMouseEvent | CSSheet::_SSCellProperty::CellCantSelect;
			lProperties |= ( CSSheet::_SSCellProperty::CellCantBlockSelect | CSSheet::_SSCellProperty::CellCantRowSelect );
			// Remark: Blank line before main header is always on the first row.
			m_pclSheet->SetCellProperty( 1, 1, m_pclSheet->GetMaxCols(), 1, lProperties, true, true );

			long lValue = m_pclSheet->GetCellParam( 1, 1 );
			lValue &= ~BitParam::BitMaskDataType;
			lValue |= BitParam::BitBlankLineBeforeMH;
			m_pclSheet->SetCellParam( 1, 1, lValue );
		}
		break;

		case BitParam::BitMainHeader:
		{
			// No need to change cell properties here. It is done in the '_VerifyMainHeaders' method.

			long lValue = m_pclSheet->GetCellParam( 1, lFromRow );
			lValue &= ~BitParam::BitMaskDataType;
			lValue |= BitParam::BitMainHeader;
			m_pclSheet->SetCellParam( 1, lFromRow, lValue );
		}
		break;

		case BitParam::BitColumnHeader:
		case BitParam::BitData:
		{
			DlgOutputHelper::mapIntColDef *pclColDef = NULL;

			if( lFromRow >= m_lColumnHeaderStart && lToRow <= m_lColumnHeaderEnd )
			{
				pclColDef = &m_mapHeaderDefList;
			}
			else if( lFromRow > m_lColumnHeaderEnd )
			{
				pclColDef = &m_mapColDefList;
			}

			if( NULL == pclColDef )
			{
				return;
			}

			for( mapIntColDefIter iter = pclColDef->begin(); iter != pclColDef->end(); iter++ )
			{
				long lColAbsolutePos = iter->second.GetInternalColPos();

				if( true == iter->second.GetRowSeparatorFlag() )
				{
					m_pclSheet->SetBorderRange( lColAbsolutePos, lFromRow, lColAbsolutePos, lToRow, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
												iter->second.GetRowSeparatorColor() );
				}

				if( true == iter->second.GetColSeparatorFlag() )
				{
					m_pclSheet->SetBorderRange( lColAbsolutePos, lFromRow, lColAbsolutePos, lToRow, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID,
												iter->second.GetColSeparatorColor() );
				}

				switch( iter->second.GetContentType() )
				{
					case DlgOutputHelper::CColDef::ContentType::Text:
						if( false == iter->second.IsEditAllowed() )
						{
							m_pclSheet->FormatStaticText( lColAbsolutePos, lFromRow, lColAbsolutePos, lToRow );
						}
						else
						{
							m_pclSheet->FormatEditTextRowRange( lColAbsolutePos, lFromRow, lToRow, true );
						}

						break;

					case DlgOutputHelper::CColDef::ContentType::Number:
						if( false == iter->second.IsEditAllowed() )
						{
							m_pclSheet->FormatStaticText( lColAbsolutePos, lFromRow, lColAbsolutePos, lToRow );
						}
						else
						{
							m_pclSheet->FormatEditDoubleRowRange( lColAbsolutePos, lFromRow, lToRow, true );
						}

						break;

					case DlgOutputHelper::CColDef::ContentType::Bitmap:
					case DlgOutputHelper::CColDef::ContentType::Param:
						// Because we don't have yet bitmap handle, we can't format cells.
						m_pclSheet->FormatStaticText( lColAbsolutePos, lFromRow, lColAbsolutePos, lToRow );
						break;
				}

				// Reset all cells to well format to default text pattern.
				_SendOutputText( _T( "" ), lColAbsolutePos, lFromRow, lToRow, &iter->second.GetFontDef() );

				// Do this at the end because 'FormatStaticText' and 'FormatEditText' methods in TSpread change properties.
				_ApplyCellProperties( lColAbsolutePos, lFromRow, lColAbsolutePos, lToRow, iter->second );

				m_pclSheet->SetCellParam( 1, lFromRow, 1, lToRow, eWhatToPrepare );
			}
		}
		break;

		case BitParam::BitBlankLine:
		{
			for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
			{
				long lColAbsolutePos = iter->second.GetInternalColPos();

				if( true == iter->second.GetRowSeparatorFlag() )
				{
					// Intentionally force to _BLACK color.
					m_pclSheet->SetBorder( lColAbsolutePos, lFromRow, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
				}

				if( true == iter->second.GetColSeparatorFlag() )
				{
					m_pclSheet->SetBorder( lColAbsolutePos, lFromRow, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, iter->second.GetColSeparatorColor() );
				}

				switch( iter->second.GetContentType() )
				{
					case DlgOutputHelper::CColDef::ContentType::Text:
						if( false == iter->second.IsEditAllowed() )
						{
							m_pclSheet->FormatStaticText( lColAbsolutePos, lFromRow, lColAbsolutePos, lFromRow );
						}
						else
						{
							m_pclSheet->FormatEditText( lColAbsolutePos, lFromRow );
						}

						break;

					case DlgOutputHelper::CColDef::ContentType::Number:
						if( false == iter->second.IsEditAllowed() )
						{
							m_pclSheet->FormatStaticText( lColAbsolutePos, lFromRow, lColAbsolutePos, lFromRow );
						}
						else
						{
							m_pclSheet->FormatEditText( lColAbsolutePos, lFromRow );
						}

						break;

					case DlgOutputHelper::CColDef::ContentType::Bitmap:
					case DlgOutputHelper::CColDef::ContentType::Param:
						// Because we don't have yet bitmap handle, we can't format cells.
						m_pclSheet->FormatStaticText( lColAbsolutePos, lFromRow, lColAbsolutePos, lFromRow );
						break;
				}

				// Reset all cells to well format to default text pattern.
				_SendOutputText( _T( "" ), lColAbsolutePos, lFromRow, lFromRow, &iter->second.GetFontDef() );

				// Do this at the end because 'FormatStaticText' and 'FormatEditText' methods in TSpread change properties.
				_ApplyCellProperties( lColAbsolutePos, lFromRow, lColAbsolutePos, lFromRow, iter->second );
			}

			long lValue = m_pclSheet->GetCellParam( 1, lFromRow );
			lValue &= ~BitParam::BitMaskDataType;
			lValue |= ( BitParam::BitBlankLine | BitParam::BitData );
			m_pclSheet->SetCellParam( 1, lFromRow, lValue );
		}
		break;
	}

	// Set property for the first and last column (internal use).
	m_pclSheet->SetCellProperty( 1, lFromRow, 1, lToRow, CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );
	m_pclSheet->SetCellProperty( m_pclSheet->GetMaxCols(), lFromRow, m_pclSheet->GetMaxCols(), lToRow, CSSheet::_SSCellProperty::CellNoSelection, true );
}

long CDlgOutputSpreadContainer::_CheckWhereToInsertColumn( int iColumnID )
{
	long lInternalColPos = 0;

	int iMainHeaderID = -1;
	int iMainHeaderSubID = -1;

	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		m_mapHeaderDefList[iColumnID].GetMainHeaderLinks( iMainHeaderID, iMainHeaderSubID );
	}
	else if( 0 != m_mapColDefList.count( iColumnID ) )
	{
		m_mapColDefList[iColumnID].GetMainHeaderLinks( iMainHeaderID, iMainHeaderSubID );
	}

	if( -1 == iMainHeaderID || -1 == iMainHeaderSubID )
	{
		return 0;
	}

	// Check first in the column header definitions.
	int iBestDeltaID = INT_MAX;

	for( mapIntColDefIter iter = m_mapHeaderDefList.begin(); iter != m_mapHeaderDefList.end(); iter++ )
	{
		int iMainHeaderIDCheck, iMainHeaderSubIDCheck;
		iter->second.GetMainHeaderLinks( iMainHeaderIDCheck, iMainHeaderSubIDCheck );

		if( iMainHeaderID == iMainHeaderIDCheck && iMainHeaderSubID == iMainHeaderSubIDCheck )
		{
			if( iColumnID > iter->first && iColumnID - iter->first < iBestDeltaID )
			{
				iBestDeltaID = iColumnID - iter->first;
				lInternalColPos = iter->second.GetInternalColPos() + 1;
			}
		}
	}

	// Check next in the column data definitions.
	for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
	{
		int iMainHeaderIDCheck, iMainHeaderSubIDCheck;
		iter->second.GetMainHeaderLinks( iMainHeaderIDCheck, iMainHeaderSubIDCheck );

		if( iMainHeaderID == iMainHeaderIDCheck && iMainHeaderSubID == iMainHeaderSubIDCheck )
		{
			if( iColumnID > iter->first && iColumnID - iter->first < iBestDeltaID )
			{
				iBestDeltaID = iColumnID - iter->first;
				lInternalColPos = iter->second.GetInternalColPos() + 1;
			}
		}
	}

	return lInternalColPos;
}

void CDlgOutputSpreadContainer::_InsertCol( int iColumnID, long lWhereToInsertAbsolute )
{
	// PAY ATTENTION: in regards to the Farpoint documentation, inserting column does not increase the total number of columns in the sheet.
	// We must thus first increase with 'SetMaxCols'.
	m_pclSheet->SetMaxCols( m_pclSheet->GetMaxCols() + 1 );
	m_pclSheet->InsCol( lWhereToInsertAbsolute );

	// Shift column position in all column header definitions.
	for( mapIntColDefIter iter = m_mapHeaderDefList.begin(); iter != m_mapHeaderDefList.end(); iter++ )
	{

		if( iter->second.GetInternalColPos() >= lWhereToInsertAbsolute )
		{
			iter->second.SetInternalColPos( iter->second.GetInternalColPos() + 1 );
		}

	}

	// Shift column position in all column definitions.
	for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
	{

		if( iter->second.GetInternalColPos() >= lWhereToInsertAbsolute )
		{
			iter->second.SetInternalColPos( iter->second.GetInternalColPos() + 1 );
		}

	}

	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		m_mapHeaderDefList[iColumnID].SetInternalColPos( lWhereToInsertAbsolute );
	}

	if( 0 != m_mapColDefList.count( iColumnID ) )
	{
		m_mapColDefList[iColumnID].SetInternalColPos( lWhereToInsertAbsolute );
	}

	m_mapColPosToID.clear();

	for( mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); iter++ )
	{
		m_mapColPosToID[iter->second.GetInternalColPos()] = iter->first;
	}

	// TSpread documentation - Cell type settings remembered:
	// When you are working with cell type settings in the ActiveX controls, you must be aware that the controls remember cell type settings.
	// For example, if you create a date cell and set the TypeDateSeparator property for that cell, subsequent date cells you create will have
	// the same TypeDateSeparator setting.
	// This is why when calling '_InserCol' method, we don't know what is the last cell type used. And to be sure, we reset all new cells
	// created to be static text.
	SS_CELLTYPE rCellType;
	m_pclSheet->SetTypeStaticText( &rCellType, 0 );
	m_pclSheet->SetCellTypeRange( lWhereToInsertAbsolute, m_lColumnHeaderStart, lWhereToInsertAbsolute, m_pclSheet->GetMaxRows(), &rCellType );

	// We need now to set the good properties for these new cells.
	if( 0 != m_mapHeaderDefList.count( iColumnID ) )
	{
		// Set col & row border if needed.
		if( true == m_mapHeaderDefList[iColumnID].GetRowSeparatorFlag() )
		{
			m_pclSheet->SetBorderRange( lWhereToInsertAbsolute, m_lColumnHeaderStart, lWhereToInsertAbsolute, m_lColumnHeaderEnd, SS_BORDERTYPE_BOTTOM,
										SS_BORDERSTYLE_SOLID, m_mapHeaderDefList[iColumnID].GetRowSeparatorColor() );
		}

		if( true == m_mapHeaderDefList[iColumnID].GetColSeparatorFlag() )
		{
			m_pclSheet->SetBorderRange( lWhereToInsertAbsolute, m_lColumnHeaderStart, lWhereToInsertAbsolute, m_lColumnHeaderEnd, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID,
										m_mapHeaderDefList[iColumnID].GetColSeparatorColor() );
		}
	}

	if( 0 != m_mapColDefList.count( iColumnID ) )
	{
		// Set col & row border if needed.
		if( true == m_mapColDefList[iColumnID].GetRowSeparatorFlag() )
		{
			m_pclSheet->SetBorderRange( lWhereToInsertAbsolute, m_lColumnHeaderEnd + 1, lWhereToInsertAbsolute, m_pclSheet->GetMaxRows(), SS_BORDERTYPE_BOTTOM,
										SS_BORDERSTYLE_SOLID, m_mapColDefList[iColumnID].GetRowSeparatorColor() );
		}

		if( true == m_mapColDefList[iColumnID].GetColSeparatorFlag() )
		{
			m_pclSheet->SetBorderRange( lWhereToInsertAbsolute, m_lColumnHeaderEnd + 1, lWhereToInsertAbsolute, m_pclSheet->GetMaxRows(), SS_BORDERTYPE_RIGHT,
										SS_BORDERSTYLE_SOLID, m_mapColDefList[iColumnID].GetColSeparatorColor() );
		}
	}

	// Reset the blank line properties before the main header if exist to be sure.
	if( true == m_fBlankLineBeforeMainHeaderExist )
	{
		m_pclSheet->SetCellProperty( 1, 1, m_pclSheet->GetMaxCols(), 1, CSSheet::_SSCellProperty::CellNoSelection, true, true );
	}

	// Set the last column to be sure (internal use).
	m_pclSheet->SetCellProperty( m_pclSheet->GetMaxCols(), 1, m_pclSheet->GetMaxCols(), m_pclSheet->GetMaxRows(), CSSheet::_SSCellProperty::CellNoSelection, true );
}

void CDlgOutputSpreadContainer::_ApplyCellProperties( long lColStart, long lRowStart, long lColEnd, long lRowEnd, DlgOutputHelper::CColDef &clColDef )
{
	long lProperties = ( false == clColDef.IsMouseEventAllowed() ) ? CSSheet::_SSCellProperty::CellNoMouseEvent : 0;
	lProperties |= ( false == clColDef.IsSelectionAllowed() ) ? CSSheet::_SSCellProperty::CellCantSelect : 0;
	lProperties |= ( false == clColDef.IsBlockSelectionAllowed() ) ? CSSheet::_SSCellProperty::CellCantBlockSelect : 0;
	lProperties |= ( false == clColDef.IsRowSelectionAllowed() ) ? CSSheet::_SSCellProperty::CellCantRowSelect : 0;
	m_pclSheet->SetCellProperty( lColStart, lRowStart, lColEnd, lRowEnd, lProperties, true, true );
}

void CDlgOutputSpreadContainer::_ApplyCellFormat( long lColStart, long lRowStart, long lColEnd, long lRowEnd, DlgOutputHelper::CColDef &clColDef )
{
	if( lColStart < 2 || lColEnd < 2 || lColStart > m_pclSheet->GetMaxCols() - 1 || lColEnd > m_pclSheet->GetMaxCols() - 1 )
	{
		return;
	}

	if( lColEnd < lColStart )
	{
		long lTemp = lColEnd;
		lColEnd = lColStart;
		lColStart = lTemp;
	}

	if( lRowStart < 1 || lRowEnd < 1 || lRowStart > m_pclSheet->GetMaxRows() || lRowEnd > m_pclSheet->GetMaxRows() )
	{
		return;
	}

	if( lRowEnd < lRowStart )
	{
		long lTemp = lRowEnd;
		lRowEnd = lRowStart;
		lRowStart = lTemp;
	}

	switch( clColDef.GetContentType() )
	{
		case DlgOutputHelper::CColDef::ContentType::Text:

			if( false == clColDef.IsEditAllowed() )
			{
				m_pclSheet->FormatStaticText( lColStart, lRowStart, lColEnd, lRowEnd );
			}
			else
			{
				m_pclSheet->FormatEditTextRowRange( lColStart, lRowStart, lRowEnd, true );
			}

			break;

		case DlgOutputHelper::CColDef::ContentType::Number:

			if( false == clColDef.IsEditAllowed() )
			{
				m_pclSheet->FormatStaticText( lColStart, lRowStart, lColEnd, lRowEnd );
			}
			else
			{
				m_pclSheet->FormatEditDoubleRowRange( lColStart, lRowStart, lRowEnd, true );
			}

			break;

		case DlgOutputHelper::CColDef::ContentType::Bitmap:
		case DlgOutputHelper::CColDef::ContentType::Param:
			// Because we don't have yet bitmap handle, we can't format cells.
			m_pclSheet->FormatStaticText( lColStart, lRowStart, lColEnd, lRowEnd );
			break;
	}

	// Reset all cells to well format to default text pattern.
	_SendOutputText( _T( "" ), lColStart, lRowStart, lRowEnd, &clColDef.GetFontDef() );
}

void CDlgOutputSpreadContainer::_VerifyMainHeaders( void )
{
	if( 0 == m_mapMainHeaders.size() )
	{
		return;
	}

	// First, delete all previous blank line and main headers if their exist.
	if( m_pclSheet->GetMaxRows() > 0 )
	{
		bool fStop = false;
		long lRow = 1;

		do
		{
			long lValue = m_pclSheet->GetCellParam( 1, lRow );

			if( BitParam::BitBlankLineBeforeMH != ( lValue & BitParam::BitBlankLineBeforeMH ) &&
				BitParam::BitMainHeader != ( lValue & BitParam::BitMainHeader ) )
			{
				fStop = true;
			}
			else
			{
				lRow++;

				if( lRow > m_pclSheet->GetMaxRows() )
				{
					fStop = true;
				}
			}
		}
		while( false == fStop );

		if( lRow > 1 )
		{
			m_pclSheet->DelRowRange( 1, lRow - 1 );
			m_pclSheet->SetMaxRows( m_pclSheet->GetMaxRows() - lRow + 1 );
		}
	}

	// Insert now the blank line if needed.
	long lFirstMainHeaderRowPos = 1;

	if( true == m_fBlankLineBeforeMainHeaderExist )
	{
		// Add blank line before the first main header line.
		_AddRows( BitParam::BitBlankLineBeforeMH, 1, 1, m_dBlankLineBeforeMainHeaderHeight, true );
		lFirstMainHeaderRowPos++;
	}

	// Insert now the main header lines.
	long lLastMainHeaderPos = lFirstMainHeaderRowPos;

	for( std::map<int, MainHeaderParam>::iterator iter = m_mapMainHeaders.begin(); iter != m_mapMainHeaders.end(); ++iter )
	{
		// Add line.
		_AddRows( BitParam::BitMainHeader, lLastMainHeaderPos, 1, iter->second.m_dMainHeaderRowHeight , true );
		lLastMainHeaderPos++;
	}

	// If not yet column defined...
	if( m_pclSheet->GetMaxCols() <= 2 )
	{
		return;
	}

	// No selection and mouse event by default.
	lLastMainHeaderPos--;
	m_pclSheet->SetCellProperty( 1, lFirstMainHeaderRowPos, m_pclSheet->GetMaxCols(), lLastMainHeaderPos, CSSheet::_SSCellProperty::CellNoSelection, true );

	// Run all main headers.
	long lMainHeaderRowToSpan = lLastMainHeaderPos;

	// 1st loop: we check main header by starting with the last (the bottom position) and finishing with the first one (the top position).
	for( riterMHParam riterMHToSpan = m_mapMainHeaders.rbegin(); riterMHToSpan != m_mapMainHeaders.rend(); ++riterMHToSpan )
	{
		int iMainHeaderIDRef = riterMHToSpan->first;

		// 2nd loop: for each main header that we check spanning, we do a loop with all main and column header below.
		// Remark: it's to cover special case as batch selection for PICV:
		//
		//   |           Input             |  |       Output          |			-> Main header ID = 0; SubID Output = 0; SubID Input = 1
		//                                    | Valve     | Actuator  |			-> Main header ID = 1; subID Valve = 0; SubID Actuator = 1
		//   | Status | Ref1 | Ref2 | Flow |  | Name | Fc | Name | IP |
		//
		// If we do a simple loop to check the main header ID 1 with column headers and the main header ID 0 with the main header ID 1, we can't
		// do a span of the group 'Input'. Because there is no sub main header belonging to the group 'Input'.
		// If we do now a comparison between MHID1 with column headers and after that a comparison between MHID0-column headers and MHID0-MHID1,
		// group 'Input' will be spanned in regards to column headers.
		for( mapIntColDefIter iterMHSubToSpan = riterMHToSpan->second.m_mapMainHeaderDefList.begin();
			 iterMHSubToSpan != riterMHToSpan->second.m_mapMainHeaderDefList.end(); iterMHSubToSpan++ )
		{
			int iMainHeaderSubIDRef = iterMHSubToSpan->first;

			long lMainHeaderRowToCompare = lLastMainHeaderPos + 1;
			riterMHParam riterMHToCompare;
			bool fStop = false;

			do
			{
				mapIntColDef *pmapIntColDef;

				if( lMainHeaderRowToCompare == lLastMainHeaderPos + 1 )
				{
					// First, link column header with the last main header.
					pmapIntColDef = &m_mapHeaderDefList;
				}
				else
				{
					// Others, link next main header with the current one.
					pmapIntColDef = &riterMHToCompare->second.m_mapMainHeaderDefList;
				}

				long lStart = LONG_MAX;
				long lEnd = 0;
				int iMainHeaderID, iMainHeaderSubID;

				for( mapIntColDefIter iterCH = pmapIntColDef->begin(); iterCH != pmapIntColDef->end(); iterCH++ )
				{
					if( 0 == iterCH->second.GetInternalColPos() )
					{
						continue;
					}

					iterCH->second.GetMainHeaderLinks( iMainHeaderID, iMainHeaderSubID );

					if( iMainHeaderID == iMainHeaderIDRef && iMainHeaderSubID == iMainHeaderSubIDRef )
					{
						if( iterCH->second.GetInternalColPos() < lStart )
						{
							lStart = iterCH->second.GetInternalColPos();
						}

						if( iterCH->second.GetInternalColPos() > lEnd )
						{
							lEnd = iterCH->second.GetInternalColPos();
						}
					}
				}

				// If main header found.
				if( LONG_MAX != lStart )
				{
					long lColAnchor, lRowAnchor, lNumCols, lNumRows;

					if( 0 != m_pclSheet->GetCellSpan( lEnd, lMainHeaderRowToCompare, &lColAnchor, &lRowAnchor, &lNumCols, &lNumRows ) )
					{
						lEnd += ( lNumCols - 1 );
					}

					m_pclSheet->AddCellSpan( lStart, lMainHeaderRowToSpan, lEnd - lStart + 1, 1 );

					if( true == riterMHToSpan->second.m_mapMainHeaderDefList[iterMHSubToSpan->first].GetRowSeparatorFlag() )
					{
						m_pclSheet->SetBorderRange( lStart, lMainHeaderRowToSpan, lEnd, lMainHeaderRowToSpan, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
													riterMHToSpan->second.m_mapMainHeaderDefList[iterMHSubToSpan->first].GetRowSeparatorColor() );
					}

					if( true == riterMHToSpan->second.m_mapMainHeaderDefList[iterMHSubToSpan->first].GetColSeparatorFlag() )
					{
						m_pclSheet->SetBorderRange( lStart, lMainHeaderRowToSpan, lEnd, lMainHeaderRowToSpan, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID,
													riterMHToSpan->second.m_mapMainHeaderDefList[iterMHSubToSpan->first].GetColSeparatorColor() );
					}

					// Update header column internal position.
					iterMHSubToSpan->second.SetInternalColPos( lStart );

					// 1st arg: we take the current main header structure (riterMH->second). Go to the text of the current sub main header (iterSubMH->first).
					_SendOutputText( riterMHToSpan->second.m_mapMainHeaderText[iterMHSubToSpan->first], lStart, lMainHeaderRowToSpan, lMainHeaderRowToSpan,
									 &iterMHSubToSpan->second.GetFontDef() );
					_ApplyCellProperties( lStart, lMainHeaderRowToSpan, lEnd, lMainHeaderRowToSpan, iterMHSubToSpan->second );
				}

				lMainHeaderRowToCompare--;

				if( lMainHeaderRowToCompare == lLastMainHeaderPos )
				{
					riterMHToCompare = m_mapMainHeaders.rbegin();
				}
				else if( riterMHToCompare != riterMHToSpan )
				{
					++riterMHToCompare;
				}
				else
				{
					fStop = true;
				}

			}
			while( false == fStop );
		}

		lMainHeaderRowToSpan--;
	}

	// Verify about freeze.
	if( true == m_fFreezeHeader )
	{
		long lCurrentFreeze = m_pclSheet->GetRowsFreeze();

		if( lCurrentFreeze < m_lColumnHeaderEnd )
		{
			m_pclSheet->SetFreeze( 0, m_lColumnHeaderEnd );
		}
	}

	_VerifyLastBlankLine();
}

void CDlgOutputSpreadContainer::_OnLButtonDown( long lLButtonDownCol, long lLButtonDownRow, CPoint ptMouse )
{
	// Select row.
	// Column 1 is reserved for internal use and also the last column.
	if( lLButtonDownCol > 1 && lLButtonDownCol < m_pclSheet->GetMaxCols() )
	{
		if( lLButtonDownRow >= m_lColumnHeaderStart && lLButtonDownRow <= m_lColumnHeaderEnd )
		{
			SelectHeaderRow( lLButtonDownRow );
		}
		else if( lLButtonDownRow > m_lColumnHeaderEnd )
		{
			SelectDataRow( lLButtonDownRow - m_lColumnHeaderEnd, false );
		}
	}

	if( m_mapNotificationHandlerList.count( INotificationHandler::NH_OnCellSelected ) > 0 )
	{
		// Prepare list of param.
		DlgOutputHelper::vecCCellParam vecParamList;

		for( DlgOutputHelper::mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); ++iter )
		{
			if( DlgOutputHelper::CColDef::ContentType::Param == iter->second.GetContentType() )
			{
				vecParamList.push_back( CCellParam( iter->first, m_pclSheet->GetCellParam( iter->second.GetInternalColPos(), lLButtonDownRow ) ) );
			}
		}

		int iColumnID = ( 0 != m_mapColPosToID.count( lLButtonDownCol ) ) ? m_mapColPosToID[lLButtonDownCol] : -1;
		m_mapNotificationHandlerList[INotificationHandler::NH_OnCellSelected]->OnOutputSpreadContainerCellClicked( m_iOutputID, iColumnID, lLButtonDownRow, ptMouse,
				&vecParamList );
	}
}

void CDlgOutputSpreadContainer::_OnLButtonDblClk( long lLButtonDblClkCol, long lLButtonDblClkRow, CPoint ptMouse )
{
	// Select row.
	// Column 1 is reserved for internal use and also the last column.
	if( lLButtonDblClkCol > 1 && lLButtonDblClkCol < m_pclSheet->GetMaxCols() )
	{
		if( lLButtonDblClkRow >= m_lColumnHeaderStart && lLButtonDblClkRow <= m_lColumnHeaderEnd )
		{
			SelectHeaderRow( lLButtonDblClkRow );
		}
		else if( lLButtonDblClkRow > m_lColumnHeaderEnd )
		{
			SelectDataRow( lLButtonDblClkRow - m_lColumnHeaderEnd, false );
		}
	}

	if( m_mapNotificationHandlerList.count( INotificationHandler::NH_OnCellClicked ) > 0 )
	{
		// Prepare list of param.
		DlgOutputHelper::vecCCellParam vecParamList;

		for( DlgOutputHelper::mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); ++iter )
		{
			if( DlgOutputHelper::CColDef::ContentType::Param == iter->second.GetContentType() )
			{
				vecParamList.push_back( CCellParam( iter->first, m_pclSheet->GetCellParam( iter->second.GetInternalColPos(), lLButtonDblClkRow ) ) );
			}
		}

		int iColumnID = ( 0 != m_mapColPosToID.count( lLButtonDblClkCol ) ) ? m_mapColPosToID[lLButtonDblClkCol] : -1;
		m_mapNotificationHandlerList[INotificationHandler::NH_OnCellClicked]->OnOutputSpreadContainerCellDblClicked( m_iOutputID, iColumnID, lLButtonDblClkRow, ptMouse,
				&vecParamList );
	}
}

void CDlgOutputSpreadContainer::_OnRButtonDown( long lRButtonDownCol, long lRButtonDownRow, CPoint ptMouse )
{
	if( m_mapNotificationHandlerList.count( INotificationHandler::NH_OnCellRClicked ) > 0 )
	{
		// Prepare list of param.
		DlgOutputHelper::vecCCellParam vecParamList;

		for( DlgOutputHelper::mapIntColDefIter iter = m_mapColDefList.begin(); iter != m_mapColDefList.end(); ++iter )
		{
			if( DlgOutputHelper::CColDef::ContentType::Param == iter->second.GetContentType() )
			{
				vecParamList.push_back( CCellParam( iter->first, m_pclSheet->GetCellParam( iter->second.GetInternalColPos(), lRButtonDownRow ) ) );
			}
		}

		int iColumnID = ( 0 != m_mapColPosToID.count( lRButtonDownCol ) ) ? m_mapColPosToID[lRButtonDownCol] : -1;
		m_mapNotificationHandlerList[INotificationHandler::NH_OnCellRClicked]->OnOutputSpreadContainerCellRClicked( m_iOutputID, iColumnID, lRButtonDownRow, ptMouse,
				&vecParamList );
	}
}

void CDlgOutputSpreadContainer::_VerifyLastBlankLine( void )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() || false == m_fAutomaticLastBlankLine )
	{
		return;
	}

	// Verify if last line is not already a blank line.
	long lValue = m_pclSheet->GetCellParam( 1, m_pclSheet->GetMaxRows() );

	if( BitParam::BitBlankLine != ( lValue & BitParam::BitBlankLine ) )
	{
		m_pclSheet->SetMaxRows( m_pclSheet->GetMaxRows() + 1 );
	}

	// Force a verify to be sure that all cells and borders are well set in regards to the current sheet initialization (column added for example).
	_PrepareCellsInRows( BitParam::BitBlankLine, m_pclSheet->GetMaxRows(), m_pclSheet->GetMaxRows() );
}
