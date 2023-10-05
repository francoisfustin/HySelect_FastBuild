#pragma once


class CExtMenu : public CMenu
{
	DECLARE_DYNAMIC( CExtMenu )

public:
	CExtMenu();
	virtual ~CExtMenu() {};

	BOOL AppendMenuEx( UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, COLORREF textColor, COLORREF backColor );

protected:
	virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

private:
	typedef struct _DrawItemParam
	{
		COLORREF m_textColor;
		COLORREF m_backColor;
		CString m_strText;
	}DrawItemParam;
	typedef std::map<UINT_PTR, DrawItemParam> mapItemParam;
	typedef mapItemParam::iterator mapItemParamIter;
	mapItemParam m_mapOwnerDrawnParams;
};

class CMFCToolBarMenuButtonEx : public CMFCToolBarMenuButton
{
	DECLARE_DYNCREATE( CMFCToolBarMenuButtonEx )
	
	CMFCToolBarMenuButtonEx();
	CMFCToolBarMenuButtonEx( UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText = NULL, BOOL bUserButton = FALSE );
	
	typedef struct _DrawItemParam
	{
		bool m_fCustomized;
		COLORREF m_textColor;
		COLORREF m_backColor;
	}DrawItemParam;

	void SetItemTextColor( COLORREF clTextColor );
	void SetItemBackColor( COLORREF clBackColor );
	void SetSubItemTextColor( UINT uiID, COLORREF clTextColor );
	void SetSubItemBackColor( UINT uiID, COLORREF clBackColor );
	void CopyParamTo( DrawItemParam* pclDrawItemParam );

protected:
	virtual void OnDraw(
		CDC* pDC,
		const CRect& rect,
		CMFCToolBarImages* pImages,
		BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE,
		BOOL bHighlight = FALSE,
		BOOL bDrawBorder = TRUE,
		BOOL bGrayDisabledButtons = TRUE );

	virtual void CopyFrom( const CMFCToolBarButton& src );

// Private methods.
private:
	void _DrawMenuItem( CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bCustomizeMode, BOOL bHighlight, BOOL bGrayDisabledButtons, BOOL bContentOnly = FALSE );

// Private variables.
private:
	DrawItemParam m_rDrawItemParam;
};
