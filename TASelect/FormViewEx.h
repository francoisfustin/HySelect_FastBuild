#pragma once


#define AFX_MAX_CLASS_NAME 255
#define AFX_STATIC_CLASS _T("Static")
#define AFX_BUTTON_CLASS _T("Button")
#define	AFX_SLIDER_CLASS _T("msctls_trackbar32")

class IFormViewPrintNotification
{
public:
	virtual BOOL IFVPN_OnPreparePrinting( CPrintInfo* pInfo ) = 0;
	virtual void IFVPN_OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo ) = 0;
	virtual void IFVPN_OnPrint( CDC* pDC, CPrintInfo* pInfo ) = 0;
	virtual void IFVPN_OnEndPrinting( CDC* pDC, CPrintInfo* pInfo ) = 0;
	virtual void IFVPN_OnEndPrintPreview( CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView ) = 0;
	virtual void IFVPN_OnFilePrint() = 0;
};

class CFormViewEx : public CFormView
{
	DECLARE_DYNCREATE( CFormViewEx )

protected:
	CFormViewEx();
	CFormViewEx( UINT nIDTemplate );
	virtual ~CFormViewEx() {}

// Public methods.
public:
	enum BackgroundLocation
	{
		BACKGR_UNDEFINED = -1,
		BACKGR_TILE = 0,
		BACKGR_TOPLEFT,
		BACKGR_TOPRIGHT,
		BACKGR_BOTTOMLEFT,
		BACKGR_BOTTOMRIGHT,
	};

	void SetBackgroundColor( COLORREF color, BOOL FRepaint = TRUE );
	void SetBackgroundImage( HBITMAP hBitmap, BackgroundLocation eLocation = BackgroundLocation::BACKGR_TILE, BOOL fAutoDestroy = TRUE, BOOL fRepaint = TRUE );
	BOOL SetBackgroundImage( UINT uiBmpResId, BackgroundLocation eLocation = BackgroundLocation::BACKGR_TILE, BOOL fRepaint = TRUE);
	virtual void OnApplicationLook( UINT uiID );

	void SetNotificationHandler( IFormViewPrintNotification* pINotificationHandler );
	void ResetNotificationHandler( void );

	afx_msg void OnFilePrintPreview();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnInitialUpdate();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	virtual BOOL OnPreparePrinting( CPrintInfo* pInfo );
	virtual void OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnPrint( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnEndPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnEndPrintPreview( CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView );
	virtual void OnFilePrint();

// Protected variables.
protected:
	HBITMAP				m_hBkgrBitmap;
	CSize				m_sizeBkgrBitmap;
	CBrush				m_brBkgr;
	COLORREF			m_cBkgr;
	BackgroundLocation	m_BkgrLocation;	
	BOOL				m_fAutoDestroyBmp;
	IFormViewPrintNotification *m_pNotificationHandler;
};

/////////////////////////////////////////////////////////////////////////////
// CMyPreviewViewEx
// Derived the CPreviewViewEx class because we need to tread One Page/ Two Page button
// with the OnUpdateCommandUI() function.
class CMyPreviewViewEx : public CPreviewViewEx
{
public:
	CMyPreviewViewEx();
	DECLARE_DYNCREATE( CMyPreviewViewEx )

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnUpdatePreviewNumPage( CCmdUI *pCmdUI );

	void SetToolTipRibbonElement( CMFCRibbonBaseElement *pRbnBaseElement, UINT ID );

// Private variables.
private:
	CMFCRibbonApplicationButton m_MainButton;
};
