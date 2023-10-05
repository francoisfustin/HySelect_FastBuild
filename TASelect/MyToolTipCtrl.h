#pragma once


class CMyToolTipCtrl : public CToolTipCtrl
{
public:
	CMyToolTipCtrl();
	virtual ~CMyToolTipCtrl();

	BOOL Create( CWnd *pParentWnd, DWORD dwStyle = 0 );
	BOOL AddToolWindow( CWnd *pWnd, LPCTSTR pszText );
	BOOL AddToolRect( CWnd *pWnd, LPCTSTR pszText, LPCRECT lpRect, UINT nIDTool );
	void RelayEvent( LPMSG lpMsg ); 

protected:
	DECLARE_MESSAGE_MAP()

// Private variable.
private:
	CList<CWnd *, CWnd *> m_ToolList; 
	CWnd *m_pParent;
};
