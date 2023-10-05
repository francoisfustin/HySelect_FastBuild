#pragma once



class CExtEdit : public CEdit
{
	friend class CNumString;
	DECLARE_DYNAMIC( CExtEdit )

public:
	class INotificationHandler
	{
	public:
		virtual void OnSpecialValidation( CWnd *pWnd, int iVirtualKey ) = 0;
	};

	CExtEdit();
	virtual ~CExtEdit();

	void SetTextColor( COLORREF rgb );

	// This works only for a simple line edit control for the moment!!
	void SetTextDisabledColor( COLORREF rgb );

	void SetBackColor( COLORREF rgb );
	COLORREF GetBackColor( void );
	
	// Set 'true' to block user to do selection on the 'CEdit' control.
	void SetBlockSelection( bool bBlockSelection ) { m_bBlockSelection = bBlockSelection; }

	// Set 'true' to avoid mouse cursor to be changed when passing over the 'CEdit' control.
	void SetBlockCursorChange( bool bBlockCursorChange ) { m_bBlockCursorChange = bBlockCursorChange; }

	// Set 'true' to force to draw a border around the control with the specified color (for example in case of input error).
	// This border is automatically erased as soon as user presses a key or the control receives the 'WM_USER_NEWDOCUMENT' message.
	void SetDrawBorder( bool bActivate, COLORREF crBorderColor );
	void ResetDrawBorder( void );
	bool IsDrawBorderActivated( void ) { return m_bDrawBorderActivated; }

	// Allow to activate or not a tooltip on the control.
	void SetToolTip( bool bActivate, CWnd *pParent, CString str = _T("") );

	// If 'true' the class will send a [TAB] to the parent when user presses the [ENTER] key or the down arrow key and will send a
	// [SHIFT] + [TAB] if user presses the up arrow key.
	void ActiveSpecialValidation( bool bActive, INotificationHandler* pclNotificationHandler = NULL ) { m_bSpecialValidationActive = bActive; m_pclNotificationHandler = pclNotificationHandler; }

	// Allow to set or reset handler for specific event.
	// Remark (FF): this allows me to avoid interference with my 'CDglTALink'.
	void SetOnCharHandler( bool bSet ) { m_bOnCharHandler = bSet; }
	void SetOnSetFocusHandler( bool bSet ) { m_bOnSetFocusHandler = bSet; }
	void SetOnKillFocusHandler( bool bSet ) { m_bOnKillFocusHandler = bSet; }
	void SetOnUnitChangeHandler( bool bSet ) { m_bOnUnitChangeHandler = bSet; }
	void SetOnNewDocumentHandler( bool bSet ) { m_bOnNewDocumentHandler = bSet; }
	void SetAllHandlers( bool bSet ) { m_bOnCharHandler = bSet; m_bOnSetFocusHandler = bSet; m_bOnKillFocusHandler = bSet; m_bOnUnitChangeHandler = bSet; m_bOnNewDocumentHandler = bSet; }

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg BOOL OnEnChange();
	virtual afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	virtual afx_msg void OnSetFocus( CWnd *pOldWnd );
	virtual afx_msg void OnKillFocus( CWnd *pNewWnd );
	virtual afx_msg HBRUSH CtlColor( CDC *pDC, UINT nCtlColor );
	virtual afx_msg void OnPaint();

	// For vertical alignment.
	virtual afx_msg void OnNcPaint();

	// To block selection.
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	virtual afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );

	// To avoid.
	virtual afx_msg BOOL OnSetCursor( CWnd *pWnd, UINT nHitTest, UINT message );

// Private variables.
private:
	// Text and text background colors.
	COLORREF m_crText;
	COLORREF m_crTextDisabled;
	COLORREF m_crBackGnd;

	eBool3 m_eFontBold;
	int m_iFontSize;

	// Background brush.
	CBrush m_brBackGnd;

	bool m_bBlockOnEnChangeNotification;
	bool m_bOnCharHandler;
	bool m_bOnSetFocusHandler;
	bool m_bOnKillFocusHandler;
	bool m_bOnUnitChangeHandler;
	bool m_bOnNewDocumentHandler;
	bool m_bSpecialValidationActive;

	// To block selection.
	bool m_bBlockSelection;

	// To avoid change of mouse cursor.
	bool m_bBlockCursorChange;

	// To allow to draw a border around the control.
	bool m_bDrawBorderActivated;
	COLORREF m_crBorderColor;

	// To display tooltip on control.
	CToolTipCtrl m_ToolTip;

	INotificationHandler *m_pclNotificationHandler;

// Private methods.
private:
	void _CancelDrawBorderMode();
};
