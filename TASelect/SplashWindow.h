#pragma once


class CSplashWindow : public CWnd
{
// Construction
protected:
	CSplashWindow();

// Attributes:
public:
	CBitmap m_bitmap;

// Operations
public:
	static void ShowSplashScreen(CWnd* pParentWnd = NULL, int millisecondsToDisplay=0 );
	static void HideSplashScreen();
	
	~CSplashWindow();
	virtual void PostNcDestroy();

	static int m_millisecondsToDisplay;

protected:
	BOOL Create(CWnd* pParentWnd = NULL);

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

// Protected variables.
protected:
	static CSplashWindow* m_pSplashWindow;
	UINT_PTR m_nTimer;
};
