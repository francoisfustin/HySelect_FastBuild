#pragma once

class CExtListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC( CExtListCtrl )

public:
	CExtListCtrl();
	virtual ~CExtListCtrl();

	// If you want to change the item text color just tell the reference for the selected and unselected item.
	void SetItemTextColor( int iRow, int iColumn, COLORREF clUnselectedColor, COLORREF clSelectedColor );

	// If you want to change the item text bk color "FOR A SELECTED ITEM" just tell the item : "iItem" his BkColour.
	void SetItemTextBkColor( int iRow, int iColumn, COLORREF clBackgroundColor );

	// If you want to change the item text bold just tell the item iItem is bold -> set to true.
	void SetItemTextBold( int iRow, int iColumn, bool fBoldItem );

	// Allow to save a flag to know if current item is enabled or disabled.
	// Notes: there is no way in the 'CListCtlr' class to disable an item and to avoid to click on it.
	void SetItemEnableState( int iItem, bool bEnabled );

	// Created to change the 'CListCtrl::SetColumnWidth' method. Because this one when called with the 'LVSCW_AUTOSIZE' arguments doesn't
	// take into account the fact that some row can be in 'bold' format. Thus width is not correct.
	void SetColumnAutoWidth( int iColumn );
	
	void ResetAllFonts( void );

	// Set the CListCtrl unable to click on it.
	void SetClickListCtrlDisable( bool fFlag = false );

	// Allow to add a new line after the last one and to set the test.
	// Param: 'strText' -> text to insert.
	// Param: 'iColumn' -> set the column index where to insert text.
	// Return: index of the new line.
	int AddItemText( CString strText, int iColumn = 0 );

	// Same as above EXCEPT that if iRow is below or equal to the number of items already in the list,
	// the method will not insert a new line at the end but just fill.
	int AddItemTextVerify( CString strText, int iRow, int iColumn = 0 );

	// Same as above but with colors.
	int AddItemTextColorVerify( CString strText, int iRow, int iColumn = 0, COLORREF clUnselectedColor = _BLACK, COLORREF clSelectedColor = _BLACK, COLORREF clBackgroundColor = _WHITE );

	// HYS-987: Manage editable accessory quantity in confirm sel dialog
	/////////////////////////////////////////////////////////////////////

	// This function return the row from the point "point" and set the right column "col"
	int GetRowFromPoint( CPoint &point, int *piCol ) const;

	// This function create an editable rectangle (CEdit) at the item / col placement.
	CEdit *EditSubLabel( int nItem, int nCol );

	// This function allow to find the initial position of the item before scrolling. Now this position allow to know if the item / col
	// can be editable or not
	void GetVertScrollPosition( int *piDeltaPos );

	// Action to do ath the end of edit label
	void OnEndLabelEdit( NMHDR *pNMHDR, LRESULT *pResult );

	// Manage left mouse click. forEditable param is true when we want can edit label or cell 
	void SendLButtonDown( UINT nFlags, CPoint point, bool bForEditable = false );

	void AddToolTipText( int iRow, int iCol, CString strText );
	void RemoveAllToolTip(); 

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	virtual afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	virtual afx_msg BOOL OnToolNeedText( UINT id, NMHDR *pNMHDR, LRESULT *pResult );
	
	virtual INT_PTR OnToolHitTest( CPoint point, TOOLINFO *pTI) const;
	virtual void PreSubclassWindow();
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

	void CellHitTest( const CPoint &pt, int &nRow, int &nCol ) const;
	bool ShowToolTip( const CPoint &pt) const;
	TCHAR *GetToolTipText( int nRow, int nCol );

// Private members.
private:
	// Verify to the function OnDraw if the item color must be changed or not.
	COLORREF _CheckItemColor( int iRow, int iColumn, bool bSelected );

	// Verify to the function OnDraw if the item bold must be change or not.
	bool _CheckItemBold( int iRow, int iColumn );

	// Verify to the function OnDraw if the item BkColour must be change or not.
	COLORREF _CheckItemTextBkColor( int iRow, int iColumn );

// Protected variables.
protected:
	struct _FontDetails
	{
		COLORREF m_clUnselectedColor;
		COLORREF m_clSelectedColor;
		COLORREF m_clBackgroundColor;
		bool m_bBold;

		struct _FontDetails()
		{
			m_clUnselectedColor = RGB( 0, 0, 0 ); 
			m_clSelectedColor = RGB( 0, 0, 255 );
			m_clBackgroundColor = RGB( 255, 255, 255 );
			m_bBold = false;
		}
		
		struct _FontDetails( COLORREF clUnselectedColor, COLORREF clSelectedColor )
		{ 
			m_clUnselectedColor = clUnselectedColor; 
			m_clSelectedColor = clSelectedColor;
			m_clBackgroundColor = RGB( 255, 255, 255 );
			m_bBold = false;
		}
		
		struct _FontDetails( COLORREF clBackgroundColor )
		{ 
			m_clUnselectedColor = RGB( 0, 0, 0 );
			m_clSelectedColor = RGB( 0, 0, 255 );
			m_clBackgroundColor = clBackgroundColor; 
			m_bBold = false;
		}
		
		struct _FontDetails( bool fBold )
		{ 
			m_clUnselectedColor = RGB( 0, 0, 0 );
			m_clSelectedColor = RGB( 0, 0, 255 );
			m_clBackgroundColor = RGB( 255, 255, 255 );
			m_bBold = fBold;
		}
	};

	// Map column with font details.
	typedef std::map< int, _FontDetails > mapColFontDetails;

	// Map row with map of column and font details.
	typedef std::map< int, mapColFontDetails > mapRowColFontDetails;

	// Map containing enable/disable status of each item in the list.
	std::map<int, bool> m_mapEnableItemStateList;

	mapRowColFontDetails m_mapFontDetailsList;
	bool m_bClickDisabled;
	CFont m_clFontBold;

	std::map<int, std::map<int, CString>> m_mapToolTipText;
};

class CPlaceEdit : public CEdit
{

public:
	CPlaceEdit( int iItem, int iSubItem, CString sInitText );

	// ClassWizard generated virtual function overrides 
public: virtual BOOL PreTranslateMessage( MSG* pMsg );

public: virtual ~CPlaceEdit();

		// Generated message map functions
protected:  
	afx_msg void OnKillFocus( CWnd* pNewWnd );
	afx_msg void OnNcDestroy();
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );

	DECLARE_MESSAGE_MAP()

private:
	int m_iItem;
	int m_iSubItem;
	CString m_sInitText;
	BOOL m_bESC;
};