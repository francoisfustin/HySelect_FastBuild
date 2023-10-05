#pragma once


#include "Global.h"
#include "MyToolTipCtrl.h"

class CExtStatic : public CStatic
{
	DECLARE_DYNAMIC( CExtStatic )

public:
	CExtStatic();
	virtual ~CExtStatic();

	void SetTextColor( COLORREF rgb );
	void SetTextDisabledColor( COLORREF rgb );
	void SetBackColor( COLORREF rgb );
	void SetThinBorder( bool fThinBorder, COLORREF crThinBorderColor );
	void SetFontBold( bool fBold );
	void SetFontSize( int iSize );

	// Allow to activate or not a tooltip on the control.
	void SetToolTip( CString strToolTip );
	void SetTextAndToolTip( CString strText, CString strToolTip );
	void SetTextAndToolTip( CString strTextAndToolTip );
	// HYS-869: reset toolTip if needed
	void ResetToolTip();

	COLORREF GetTextColor( void ) { return m_crText; }
	COLORREF GetTextDisabledColor( void ) { return m_crTextDisabled; }
	COLORREF GetBackColor( void );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH CtlColor( CDC *pDC, UINT nCtlColor );
	afx_msg void OnPaint( );
	
	virtual BOOL PreTranslateMessage( MSG *pMsg );

// Private methods.
private:
	void _VerifyTooltip( void );

// Private variables.
protected:
	// Text and text background colors.
	COLORREF m_crText;
	COLORREF m_crTextDisabled;
	COLORREF m_crBackGnd;
	COLORREF m_crThinBorderColor;

	eBool3 m_eFontBold;
	int m_iFontSize;
	bool m_bFontPropertyChanged;
	
	// Background brush.
	CBrush m_brBackGnd;
	bool m_fThinBorder;

	// The fictive one allows us to retrieve the current font in the device context (See the 'CtlColor' method).
	CFont m_clFictiveFont;
	CFont m_clFont;

	// To display tooltip on control.
	CMyToolTipCtrl m_ToolTip;
};
