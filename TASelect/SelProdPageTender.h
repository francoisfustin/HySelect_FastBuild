#pragma once


class CSelProdPageTender : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_TenderText = 1,
	};

	CSelProdPageTender( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageTender( void ) {}

	virtual void Reset( void );
	virtual bool PreInit( HMvector &vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void )
	{
		return HasSomethingToDisplay();
	}
	virtual UINT GetTabTitleID( void );

	// Protected variables.
protected:
	// Remark: header column must be set to the first position (index 1).
	//         pointer column is the last (=m_iColNum).
	//         footer column must be set just before the pointer column (m_iColNum-1).
	enum ColumnDescriptionPTender
	{
		Header				= 1,
		ArticleNumber		= 2,
		TenderText			= 3,
		LocalArticleNumber	= 4,
		Description			= 5,
		Quantity			= 6,
		UnitPrice			= 7,
		TotalPrice			= 8,
		Footer				= 9,
		Pointer				= 10
	};
	CData *m_pSelected;

	// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _FillRow( CSheetDescription *pclSheetDescription, long lRow, CArticleItem *pclArticleGroup );

	// Private variables.
private:
	bool m_fTAArtNumberShown;
	bool m_fLocalArtNumberShown;
};
