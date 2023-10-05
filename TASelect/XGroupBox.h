// XGroupBox.h  Version 1.0
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// License:
//     This software is released under the Code Project Open License (CPOL),
//     which may be found here:  http://www.codeproject.com/info/eula.aspx
//     You are free to use this software in any way you like, except that you 
//     may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>
class CXGroupBox : public CButton
{
	DECLARE_DYNAMIC( CXGroupBox )

public:
	class INotificationHandler
	{
	public:
		enum
		{
			NH_OnIconClicked = 0x01,
		};
		INotificationHandler() {}
		virtual ~INotificationHandler() {}
		virtual void OnGroupIconClicked( CXGroupBox* pXGroupBoxClicked ) = 0;
	};

	CXGroupBox();
	virtual ~CXGroupBox();

// Attributes
public:
	enum
	{ 
		DEFAULT_X_MARGIN = 8,
		DEFAULT_ICON_SPACING = 8,
		DEFAULT_ICON_SIZE = 16
	};

	enum CONTROL_STYLE
	{
		groupbox = 0, 
		header
	};

	enum BORDER_STYLE
	{ 
		flat = 0,
		etched
	};

	enum ALIGNMENT
	{ 
		left = 0, 
		center, 
		right
	};

	void SetAlignment( ALIGNMENT eAlign, bool bRedraw = true );
	void SetBold( bool fBold, bool bRedraw = true );
	void SetBckgndColor( COLORREF crBackground, bool bRedraw = true );
	void SetBorderColor( COLORREF crBorder, bool bRedraw = true );
	void SetBorderStyle( BORDER_STYLE eStyle, bool bRedraw = true );
	void SetControlStyle( CONTROL_STYLE eStyle, bool bRedraw = true );
	void SetDisabledStyle( bool bShowDisabledState, bool bRedraw = true );
	void SetForceShowDisable( bool bForceShowDisable, bool bRedraw = true );
	void SetFont( CFont *pFont, bool bRedraw = true );
	void SetFont( LOGFONT * pLogFont, bool bRedraw = true );
	void SetFont( LPCTSTR lpszFaceName, int iPointSize, bool bRedraw = true );
	void SetIcon( HICON hIcon, bool bForDisableMode = false, UINT uiIconSize = 16, bool bRedraw = true );
	void SetIcon( UINT uiIconId, bool bForDisableMode = false, UINT uiIconSize = 16, bool bRedraw = true );
	void SetIconAlignment( ALIGNMENT eAlign, bool bRedraw = true );
	void SetIconSpacing( int iIconSpacing, bool bRedraw = true );
	void SetImageList( CImageList *pclImageList, int iIconNo, bool bForDisableMode = false, bool bRedraw = true );
	void SetMargins( int iXMargin, int iYMargin, bool bRedraw = true );
	void SetTextColor( COLORREF cr, bool bRedraw = true );
	void SetWindowText( LPCTSTR lpszText, bool bRedraw = true );
	void SetExpandCollapseMode( bool bExpandCollapseMode ) { m_bExpandCollapseMode = bExpandCollapseMode; }
	void SetNotificationHandler( INotificationHandler* fNotificationHandler ) { m_pfNotificationHandler = fNotificationHandler; }

	ALIGNMENT GetAlignment() { return m_eControlAlignment; }
	bool GetBold();
	COLORREF GetBorderColor() { return m_crBorder; }
	BORDER_STYLE GetBorderStyle() { return m_eBorderStyle; }
	bool GetDisabledStyle() { return m_bShowDisabledState; }
	bool GetForceShowDisable() { return m_bForceShowDisable; }
	CFont *GetFont() { return &m_clFont; }
	bool GetFont( LOGFONT *pLF );
	CString GetFontFaceName();
	int GetFontPointSize();
	ALIGNMENT GetIconAlignment() { return m_eIconAlignment; }
	int GetIconSpacing() { return m_iIconSpacing; }
	
	void GetMargins( int &iXMargin, int &iYMargin )	{ iXMargin = m_iXMargin; iYMargin = m_iYMargin; }
	
	CONTROL_STYLE GetControlStyle()	{ return m_eStyle; }
	COLORREF GetTextColor() { return m_crText; }
	bool GetExpandCollapseMode( void ) { return m_bExpandCollapseMode; }
	int GetHeaderHeight( void );

	void EnableTheme( bool bEnable, bool bRedraw = true );
	bool EnableWindow( bool bEnable = true, bool bRecurseChildren = false );
	bool ShowWindow( bool bShow, bool bRecurseChildren = true );
	void MoveGroupBox( CRect rect );
	void SaveChildList();

	void SetRibbonBckgnd( bool bRibbonColor ) { m_bUseRibbonColor = bRibbonColor; }
	void SetInOffice2007Mainframe( bool bInMainframe ) { m_bInOffice2007Mainframe = bInMainframe; m_bUseRibbonColor = bInMainframe; }
	void SetCheckBoxStyle( UINT uiStyle = BS_AUTOCHECKBOX );
	
	int	GetCheck() const;
	void SetCheck( int iCheck );

	void SetGroupID( UINT uiGroup );
	UINT GetGroupID() const;

	// Can be called by parent of control to specify if user has clicked on Group Box and not only on the checkbox in 
	// this control.
	void ClickedOnGroup( void );

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd( CDC *pDC );
	afx_msg void OnPaint();
	afx_msg LRESULT OnNcHitTest( CPoint point );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );

	// If current m_CheckBoxWnd is a radio button, search all controls in current parent
	// window, uncheck all other radio buttons if they have the same group id.
	afx_msg void OnClicked();
	
	virtual void PreSubclassWindow();

	HICON CreateGrayscaleIcon( HICON hIcon );
	HICON CreateGrayscaleIcon( HICON hIcon, COLORREF *pPalette );
	void DrawBorder( CDC *pDC, CRect &rect );
	void DrawEtchedLine( CDC *pDC, CRect &rect, COLORREF cr );
	void DrawEtchedRectangle( CDC *pDC, CRect &rect, COLORREF cr );
	void DrawHeader( CDC *pDC, CRect &rectText, CRect &rectIcon );
	int DrawIcon( CDC *pDC, CRect &rect );
	void DrawItem( CDC *pDC, CRect &rect );
	int DrawText( CDC *pDC, CRect &rect );
	int EnableChildWindows( HWND hWnd, bool fEnable, bool fEnableParent );
	int ShowChildWindows( HWND hWnd, bool fShow, bool fShowParent );
	void EraseBorder( CDC *pDC, CRect &rect, COLORREF crBackground );
	void EraseHeaderArea( CDC *pDC, CRect &rect, COLORREF crBackground );
	int GetFontHeight( int iPointSize );
	int GetFontPointSize( int iHeight );
	void GetHeaderRect( CRect &rect );
	CFont *GetSafeFont();
	void ComputeHeaderHeight();

// Protected variables.
protected:
	CFont m_clFont;									// Font used for text.
	COLORREF m_crText;								// Text color.
	CRect m_rectInitialClientSize;
	bool m_bInOffice2007Mainframe;					// GroupBox is in Office 2007 Mainframe: line color adapted for Win XP.
	bool m_bUseRibbonColor;							// Use background color to cope with ribbon.
	COLORREF m_crBackground;						// Background color.
	COLORREF m_crBorder;							// User-specified border color.
	int m_iXMargin, m_iYMargin;						// Space in pixels between border and icon/text.
	int m_iYOffset;									// Y offset of frame (text extends above frame).
	int m_iHeaderHeight;							// Return the group height when it is in header control style mode.
	int m_iIconSpacing;								// Horizontal space in pixels between icon and text.
	HICON m_hIcon;
	HICON m_hGrayIcon;								// Disabled icon handle.
	bool m_bDestroyIcon;
	int m_iIconSize;								// Size of icon, default to 16.
	CRect m_rectIcon;
	CRect m_rect;
	CRect m_CheckBoxRect;							// Space needed for check box.
	CSize m_sizeText;								// Size of text from GetTextExtent().
	bool m_bThemed;									// true = themes are enabled.
	bool m_bEnableTheme;							// true = theme will be used if enabled.
	bool m_bShowDisabledState;						// true = gray text and icon will be displayed if control is disabled.
	bool m_bForceShowDisable;						// true = gray text and icon will be displayed EVEN if control is not disabled.
	bool m_bGrayPaletteSet;							// Used for gray scale icon.

	static COLORREF	m_defaultGrayPalette[256];		// Used for gray scale icon.
	CONTROL_STYLE m_eStyle;							// Disabled groupbox or header.
	BORDER_STYLE m_eBorderStyle;					// Border line style - flat or etched; applies only if no border color and no theme.
	ALIGNMENT m_eIconAlignment;						// Left (of text) or right;  note that control's style bits control header alignment.
	ALIGNMENT m_eControlAlignment;					// Alignment for control.
	bool m_bExpandCollapseMode;						// true = if we can collapse and expand the group.

	CButton m_CheckBoxWnd;							// Could be check box or radio box.
	UINT m_uiGroupID;								// Radio button holds same group id.
	INotificationHandler *m_pfNotificationHandler;

	std::vector<HWND> m_vechWndChild;

};
