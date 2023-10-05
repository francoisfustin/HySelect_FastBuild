#pragma once


class CSelProdPageArtList : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_ArticleList = 1,
	};

	CSelProdPageArtList( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageArtList( void ) {}
		
	virtual void Reset( void );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABARTLIST; }

	BOOL PreTranslateMessage(MSG* pMsg);

// Protected variables.
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnSSClick(WPARAM wParam, LPARAM lParam);
	// Remark: header column must be set to the first position (index 1).
	//         pointer column is the last (=m_iColNum).
	//         footer column must be set just before the pointer column (m_iColNum-1).
	enum ColumnDescriptionPArtList
	{
		Header				= 1,
		ArticleNumber		= 2,
		AccArtNumber		= 3,
		LocalArticleNumber	= 4,
		Description			= 5,
		Quantity			= 6,
		UnitPrice			= 7,
		TotalPrice			= 8,
		Footer				= 9,
		Pointer				= 10
	};
	CData* m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _FillRow( CSheetDescription *pclSheetDescription, long lRow, CArticleGroup *pclArticleGroup );

// Private variables.
private:
	bool m_bTAArtNumberShown;
	bool m_bLocalArtNumberShown;
	// HYS-1358 : Show or not TA article number column when there is local article number
	bool m_bTAArticleNumberIsEmpty;
};
