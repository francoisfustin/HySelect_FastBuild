#pragma once


class CExtProgressCtrl : public CProgressCtrl
{
public:
	CExtProgressCtrl();
	virtual ~CExtProgressCtrl() {}

    void SetShowText( bool bShow );

	void SetForegroundColor( COLORREF ForegroundColor );
	void SetBackgroundColor( COLORREF BackgroundColor );
	void SetTextForegroundColor( COLORREF TextForegroundColor );
	void SetTextBackgroundColor( COLORREF TextBackgroundColor );
	
	COLORREF GetForegroundColor();
	COLORREF GetBackgorundColor();
	COLORREF GetTextForegroundColor();
	COLORREF GetTextBackgorundColor();

protected:
	DECLARE_MESSAGE_MAP()
	virtual afx_msg BOOL OnEraseBkgnd( CDC* pDC );
 	virtual afx_msg void OnPaint();
	virtual afx_msg LRESULT OnSetText( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnGetText( WPARAM wParam, LPARAM lParam );

	// Private variables.
private:
	bool m_bShowText;
	CString m_strText;
	COLORREF m_ForegroundColor;
	COLORREF m_BackgroundColor;
	COLORREF m_TextForegroundColor;
	COLORREF m_TextBackgroundColor;
};
