#pragma once


class CSelProdPageBv : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_BalancingValve = 1,
	};

	CSelProdPageBv( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageBv( void ) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABBV; }

	// Let's these 4 methods public to be accessible from 'CSelProdPageDpC'.
	long FillRowBvProduct( CSheetDescription* pclSheetDescription, long lRow, CDB_TAProduct* pclBv, int iQuantity, bool fForDpCSet = false );
	long FillRowBvInfo( CSheetDescription* pclSheetDescription, long lRow, CDB_TAProduct* pclBv, double dQ, double dDp, double dPresset,
		double dRho, double dNu, CString strPowerDt );
	long FillRowBv( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod* pclHM, CDS_HydroMod::CBase* pclBase, CDB_TAProduct** ppMv = NULL );
	void AddArticleInGroupList( CDS_SSel* pclSSel );

// Protected variables.
protected:
	CData* m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription* pclSheetDescription );
	long _FillRowBv( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelBv* pclSSelBv, CDB_TAProduct* pclBv );
	void _AddArticleList( CDB_TAProduct* pclTAP, int iQuantity );
};
