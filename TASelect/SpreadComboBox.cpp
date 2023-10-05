#include "stdafx.h"
#include "TASelect.h"
#include "Hydronic.h"
#include "HydroMod.h"
#include "DlgSpreadCB.h"
#include "DlgComboBoxHM.h"
#include "SpreadComboBox.h"

CSpreadComboBox::CSpreadComboBox( void ) : CSSheet()
{
	m_pOwner = NULL;
	m_lCols = m_lShowCols = m_lRows = m_lShowRows = m_lSelRow = 0;
}

CSpreadComboBox::~CSpreadComboBox( void )
{
}

void CSpreadComboBox::Empty()
{
	Init();
	SetBool( SSB_ALLOWDRAGDROP, FALSE );
	m_lCols = m_lShowCols = m_lRows = m_lShowRows = m_lSelRow = 0;
	
	SetBool( SSB_HORZSCROLLBAR, FALSE );
	SetBool( SSB_VERTSCROLLBAR, TRUE );
}

bool CSpreadComboBox::SetnCols( long lCols, long lShowCols )
{
	m_lShowCols = lShowCols;
	long MaxCol = GetMaxCols();
	SetMaxCols( lCols );
	return true;
}

bool CSpreadComboBox::SetnRows( long lRows )
{
	m_lShowRows = lRows;
	return true;
}

long CSpreadComboBox::SetRowParameters( long lColumn, long lRow, CString str, LPARAM lparam, bool fAutoWidth )
{
	if( lRow > m_lRows )
	{
		SetMaxRows( m_lRows );
		lRow = m_lRows;
	}
	
	// Add a row.
	if( -1 == lRow )
	{
		m_lRows ++;
		SetMaxRows( m_lRows );
		lRow = m_lRows;
	}
	
	SetStaticText( lColumn, lRow, str );
	SetCellParam( lColumn, lRow, (long)lparam );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );

	if( true == fAutoWidth )
	{
		SetColWidth( lColumn, GetMaxTextColWidthW( lColumn ) * 1.1 );
	}

	return lRow;
}

bool CSpreadComboBox::GetRow( long lColumn, long lRow, CString &str, LPARAM &lparam )
{
	lparam = (LPARAM)GetCellParam( lColumn, lRow );
	str = GetCellText( lColumn, lRow );
	return true;
}

long CSpreadComboBox::GetSelectedRow()
{
	return m_lSelRow;
}

long CSpreadComboBox::SelectRow( long lRow )
{
	if( lRow <= GetMaxRows() )
	{
		SetSelBlockOptions( SS_SELBLOCK_ROWS );
		SetBool( SSB_HSCROLLBARTRACK, FALSE );
		SetBool( SSB_VSCROLLBARTRACK, TRUE );
		// Don't know why 'lRow-2'
//		SetActiveCell( 1, max( lRow - 2, 0 ) );
		SetActiveCell( 1, lRow );
		LeaveCell( 1, lRow, 1, lRow, NULL );
		return lRow;
	}
	return 0;
}

long CSpreadComboBox::FindAndSelectRow( long lColumn, CString str )
{
	for( long lLoopRow = 1; lLoopRow <= GetMaxRows(); lLoopRow++)
	{
		CString text = GetCellText( lColumn, lLoopRow );
		if( str == text )
		{
			return SelectRow( lLoopRow );
		}
	}
	return 0;
}

long CSpreadComboBox::FindAndSelectRow( long lColumn, LPARAM lparam )
{
	for( long lLoopRow = 1; lLoopRow <= GetMaxRows(); lLoopRow++ )
	{
		LPARAM lp = (LPARAM)GetCellParam( lColumn, lLoopRow );
		if( lp == lparam )
		{
			return SelectRow( lLoopRow );
		}
	}
	return 0;
}

void CSpreadComboBox::CellClicked( long lColumn, long lRow )
{
	if( lColumn != -1 && lRow  != -1 )
	{
		UnSelectMultipleRows( m_lSelRow, true );
		SelectOneRow( lRow );
		Invalidate();
		m_lSelRow = lRow;
		( (CDlgComboBoxHM *)m_pOwner )->Close();
	}
}

void CSpreadComboBox::LeaveCell( long lOldColumn, long lOldRow, long lNewColumn, long lNewRow, BOOL* pfCancel )
{
	if( NULL != pfCancel )
		*pfCancel = FALSE;

	if( lNewColumn != -1 && lNewRow != -1 )
	{
		if( lNewRow <= GetRowsFreeze() )
		{
			if( NULL != pfCancel )
				*pfCancel = TRUE;
			return;
		}

		// Restore unselected color
		SelectOneRow( lNewRow, 0, -1, _TAH_UNDER_SELECTION );
		UnSelectMultipleRows( m_lSelRow, true );
		m_lSelRow = lNewRow;
	}
}

void CSpreadComboBox::DblClick( long lColumn, long lRow )
{
	UnSelectMultipleRows( m_lSelRow, true );
	SelectOneRow( lRow, 0, -1, _TAH_UNDER_SELECTION );

	Invalidate();
	( (CDlgComboBoxHM *)m_pOwner )->Close();
}

BEGIN_MESSAGE_MAP( CSpreadComboBox, CSSheet )
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void CSpreadComboBox::OnKillFocus( CWnd* pNewWnd )
{
	// Warning fired when user press a key !!!!!!
	// To do intercept all user key except arrow and enter key OnKeyDown
	CSSheet::OnKillFocus( pNewWnd );
}
