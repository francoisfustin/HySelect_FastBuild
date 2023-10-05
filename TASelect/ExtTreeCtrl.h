#pragma once


class CExtTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CExtTreeCtrl)

public:
	CExtTreeCtrl() {}
	virtual ~CExtTreeCtrl() {}

	void ResetAllColors();
	void SetItemColor( HTREEITEM hItem, COLORREF color );

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual afx_msg void OnPaint();

private:
	std::map<HTREEITEM, COLORREF> m_mapItemColors;
};
