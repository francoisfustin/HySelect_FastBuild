#pragma once


class CExtButton : public CButton
{
public:
	class INotificationHandler
	{
	public:
		virtual void OnSpecialValidation( CWnd *pWnd, int iVirtualKey ) = 0;
	};

	CExtButton();
	virtual ~CExtButton();

	void SetTextColor( COLORREF rgb );
	void SetTextDisabledColor( COLORREF rgb );
	void SetBackColor( COLORREF rgb );

	// Set 'true' to force to draw a border around the control with the specified color (for example in case of input error).
	// This border is automatically erased as soon as user presses a key or the control receives the 'WM_USER_NEWDOCUMENT' message.
	void SetDrawBorder( bool bActivate, COLORREF crBorderColor );
	void ResetDrawBorder( void );

	void EnableLeftClick( bool bEnable ) { m_bLeftClickEnabled = bEnable; }

protected:
	DECLARE_MESSAGE_MAP()
	virtual afx_msg HBRUSH CtlColor( CDC *pDC, UINT nCtlColor );
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg void OnPaint();
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	virtual afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	   // Private methods.
private:
	void _CancelDrawBorderMode();

// Private variables.
private:
	// Text and text background colors.
	COLORREF m_crText;
	COLORREF m_crTextDisabled;
	COLORREF m_crBackGnd;
	// Background brush.
	CBrush m_brBackGnd;

	LOGFONT m_lf;
	CFont m_clFont;

	// To allow to draw a border around the control if error.
	bool m_bDrawBorderActivated;
	COLORREF m_crBorderColor;

	bool m_bLeftClickEnabled;

	INotificationHandler *m_pclNotificationHandler;
};
