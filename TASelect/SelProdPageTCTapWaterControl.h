#pragma once


class CSelProdPageTCTapWaterControl : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_TapWaterControl = 1,
	};

	CSelProdPageTCTapWaterControl( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageTCTapWaterControl(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector &vecHMList );
	virtual bool Init( bool bResetOrder = false, bool bPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABTAPWATERCONTROL; }

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _FillRowTapWaterControl( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelTapWaterControl *pclSSelTapWaterControl, CDB_TapWaterControl *pclTapWaterControl );
	long _FillRowTapWaterControlProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_TapWaterControl *pclTapWaterControl, int iQuantity );
	void _AddArticleList( CDB_TAProduct *pclTAP, int iQuantity );
	void _AddArticleList( CDS_SSelTapWaterControl *pclSSelTapWaterControl );
};
