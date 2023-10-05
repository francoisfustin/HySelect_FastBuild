#pragma once


class CSelProdPageSv : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_ShutoffValve = 1,
	};

	CSelProdPageSv( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageSv( void ) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABSV; }

	// Let's these 4 methods public to be accessible from 'CSelProdPageDpCBCV'.
	long FillRowSvProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pclShutoffValve, int iQuantity );
	long FillRowSvInfo( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pclShutoffValve, double dQ, double dDp, double dRho, double dNu, CString strPowerDt );
	long FillRowSv( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM, CDS_HydroMod::CBase *pclBase, CDB_TAProduct **ppShutoffValve = NULL );
	void AddArticleInGroupListFromDpCBCV( CDS_SSelDpCBCV *pSSelDpCBCV );

// Protected variables.
protected:
	CData* m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _FillRowSv( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSv *pclSSelSv, CDB_TAProduct *pclShutoffValve );
	long _FillActuatorSelectedWithSv( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSv *pclSSelShutOff );

	// called when individual selection and when actuator doesn't belong to a set with the shut-off valve.
	void _AddArticleList( CDB_Actuator* pclActuator, CAccessoryList* pclAccessoryList, int iQuantity, bool fSelectionBySet );

	void _AddArticleList( CDB_TAProduct *pclTAP, int iQuantity );
	void _AddArticleInGroupList( CDS_SSelSv *pSSelSv );
};
