
//
// SpreadComboBox.h: interface for the CSpreadComboBox class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SPREADCOMBOBOX_H__INCLUDED_
#define _SPREADCOMBOBOX_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "SSheet.h"
class CSpreadComboBox :	public CSSheet
{
public:
	CSpreadComboBox( void );
	virtual ~CSpreadComboBox( void );
	
	void SetOwner( CDialogEx *pOwner ) { m_pOwner = pOwner; }

	void Empty();

	// Set number of columns and number of visible columns (wide of dialog)
	bool SetnCols( long lCols, long lShowCols );

	// Set number of visible rows
	bool SetnRows( long lRows );

	long GetVisibleRows() { return m_lShowRows; }

	//  Set row parameters set row to -1 to add a row at the end of list
	// return used row
	long SetRowParameters( long lColumn, long lRow, CString str, LPARAM lparam, bool fAutoWidth = true );

	// return row content
	bool GetRow( long lColumn, long lRow, CString &str, LPARAM &lparam );

	// return row index of selected row.
	long GetSelectedRow();

	// SelectRow
	long SelectRow( long lRow );
	long FindAndSelectRow( long lColumn, CString str );
	long FindAndSelectRow( long lColumn, LPARAM lparam );

	long GetLastVisibleRow() { return m_lShowRows; }
	long GetLastVisibleCol() { return m_lShowCols; }

	void CellClicked( long lColumn, long lRow );
	void LeaveCell( long lOldColumn, long lOldRow, long lNewColumn, long lNewRow, BOOL* pfCancel );
	void DblClick( long lColumn, long lRow );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKillFocus(CWnd* pNewWnd);

// Protected variables.
protected:
	CDialogEx* m_pOwner;
	long m_lCols, m_lShowCols, m_lRows, m_lShowRows, m_lSelRow;
};

#endif //_SPREADCOMBOBOX_H__INCLUDED_