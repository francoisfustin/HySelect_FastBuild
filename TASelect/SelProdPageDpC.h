#pragma once


class CSelProdPageDpC : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_DpC = 1,
	};

	CSelProdPageDpC( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageDpC( void ) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABDPC; }

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	
	// Fill row with Dp controller info from a 'CDS_SSelDpC'.
	long _FillRowDpController( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpC *pclSSelDpController );

	// Fill row with Dp controller info from a 'CDS_HydroMod::CDpC'.
	long _FillRowDpController( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CDpC *pclHMDpController );

	// This method is called by the both '_FillRowDpController' methods above.
	// For the one with 'CDS_SSelDpC', we need 'pclSet' argument to have access to reference number to show in the article number 
	// column in case of DpC-Mv package (Don't take 'Article Number' of pTAP because it is the DpC one and not the package one).
	// For the method with 'CDS_HydroMod::CDpC' we need only 'pTAP'.
	void _FillRowDpControllerProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pTAP, int iQuantity, CDB_Set *pclSet = NULL, CString strArtNumber = _T("") );

	long _FillDpControllerSCheme( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpC *pclSSelDpController );

	void _AddDpControllerArticleList( CDS_SSelDpC *pclSSelDpController, CDB_TAProduct *pTAP = NULL, int iQuantity = 1 );

	// Fill row with Dp relief valve info from a 'CDS_SSelDpReliefValve'.
	long _FillRowDpReliefValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpReliefValve *pclSSelReliefValve );
	void _FillRowDpReliefValveProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_DpReliefValve *pclDpReliefValve, int iQuantity, CString strArtNumber = _T("") );
	void _AddDpReliefValveArticleList( CDB_TAProduct *pTAP, int iQuantity = 1 );
	// HYS-2007: Add Dp sensor selected alone.
	long _FillRowDpSensor( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelDpSensor* pclSSelDpSensor );
	void _FillRowDpSensorProduct( CSheetDescription* pclSheetDescription, long lRow, CDB_DpSensor* pclDpReliefValve, int iQuantity, CString strArtNumber );
	void _AddDpSensorArticleList( CDB_Product* pclProduct, CDS_SSelDpSensor* pSSel, int iQuantity = 1);
};
