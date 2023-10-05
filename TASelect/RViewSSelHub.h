#pragma once


#include "afxwin.h"
#include "EnBitmapRow.h"
#include "FormViewEx.h"
#include "DrawSet.h" 


class CSSheetTAHub;
class CRViewSSelHub : public CFormViewEx, protected CDrawSet
{
	DECLARE_DYNCREATE( CRViewSSelHub )

public:
	CRViewSSelHub();           // protected constructor used by dynamic creation
	virtual ~CRViewSSelHub();
	
	enum { IDD = IDV_RVIEW };

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext &dc) const;
#endif

	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext = NULL );
	void SetRedraw( CDS_HmHub *pHM = NULL );
	void ResizeSheet();
	void UpdatePicture( bool fUseOldPos );
	void RefreshSheets();
	void RefreshScrollBars();

	void SetHMSheetsPos();
	void DrawScrollBars();
	bool IsLocked();
	void CheckSelectButton( CDS_HmHub *pHmHub = NULL );
	// Used as a bridge to TabCDialogSSelh to fill ComboBox measuring valve location
	void RefreshMvLoc();
	void Paint(){OnPaint();};

// Public variables
public:
	typedef std::vector< short > vecReturnCommands;
	typedef vecReturnCommands::iterator vecReturnCommandsIter;

// Protected members
protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	
	afx_msg void OnPaint();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnContextMenu( CWnd *pWnd, CPoint point );
	afx_msg void OnFltRvSSelHubDelete();
	afx_msg void OnFltRvSSelHubCopy();
	afx_msg void OnFltRvSSelHubPaste();
	afx_msg void OnFltRvSSelHubEditaccessories();
	afx_msg void OnFltRvSSelHubCopyAccessories();
	afx_msg void OnFltRvSSelHubPasteAccessories();
	afx_msg void OnGetFullInfoHub();
	afx_msg void OnFltRvSSelHubLock();
	afx_msg void OnFltRvSSelHubUnLock();
	afx_msg void OnFltRvSSelHubLockAll();
	afx_msg void OnFltRvSSelHubUnlockAll();
	afx_msg void OnFltRvSSelHubLockcolumn();
	afx_msg void OnFltRvSSelHubUnlockcolumn();
	afx_msg void OnUpdateMenuText( CCmdUI *pCmdUI );
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );

	// Spread DLL message handlers
 	afx_msg LRESULT ClickFpspread( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT ComboDropDown( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT DblClickFpspread( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT DragDropBlock( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT EditChange( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT KeyDown( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT KeyPress( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT LeaveCell( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT RightClick( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT SheetChanged( WPARAM wParam, LPARAM lParam );
 	afx_msg LRESULT TextTipFetch( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()

 	void ClickFpspreadHub( long lColumn, long lRow );
	void LeaveCellHub( long lColumn, long lRow, long lNewColumn, long lNewRow, BOOL *pfCancel );
	void RightClickHub( short nClickType, long lColumn, long lRow, long lMouseX, long lMouseY );
 	void ClickFpspreadStation( long lColumn, long lRow );
 	void LeaveCellStation( long lColumn,long lRow, long lNewColumn, long lNewRow, BOOL *pfCancel );
 	void RightClickStation( short nClickType, long lColumn, long lRow, long lMouseX, long lMouseY );
 	void DragDropBlockStation( long lOldFromColumn, long lOldFromRow, long lOldToColumn, long lOldToRow, long lNewFromColumn, long lNewFromRow, long lNewToColumn, long lNewToRow, BOOL *pfCancel );

	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );

// Protected variables
protected:
	CSSheetTAHub *m_pSheetTAHub;
	CSSheetTAHub *m_pSheetTAStation;
	CSSheetTAHub *m_pCurSheet;
	CEnBitmap m_EnBmpHub;
	CEnBitmap m_EnBmpBox;

	long m_lRow;
	long m_lColumn;
	int m_iXpos;
	int m_iYpos;
	bool m_fMustbeSized;

// Private variables
private:

};

extern CRViewSSelHub *pRViewSSelHub;
