#pragma once


class CSelProdPageSafetyValve : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_SafetyValve = 1,
	};

	CSelProdPageSafetyValve( CArticleGroupList* pclArticleGroupList );
	virtual ~CSelProdPageSafetyValve(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABSAFETYVALVE; }

// Protected variables.
protected:
	CData* m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _FillRowSafetyValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSafetyValve *pclSSelSafetyValve, CDB_SafetyValveBase *pclSafetyValve );
	long _FillRowSafetyValve( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM );
	long _FillRowSafetyValveProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_SafetyValveBase *pclSafetyValve, int iQuantity );
	long _FillRowSafetyValveInfo( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelSafetyValve *pclSSelSafetyValve, CDB_SafetyValveBase* pclSafetyValve );
	long _FillRowSafetyValveInfoFromHM( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod *pHM, CDB_SafetyValveBase* pclSafetyValve );
	long _FillWaterMedium( CSheetDescription *pclSheetDescription, long lRow, CString strMediumName, double dTmin, double dTmax );
	long _FillRowBlowTank( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSafetyValve *pclSSelSafetyValve );
	long _FillRowBlowTankProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_BlowTank *pclBlowTank, int iQuantity );
	long _FillRowBlowTankInfo( CSheetDescription* pclSheetDescription, long lRow, CDB_BlowTank *pclBlowTank );
	
	// HYS-1741
	long _FillAccessoriesServicesAlone( CSheetDescription* pclSheetDescription, long lRow, CDS_SelPWQAccServices* pclSSelAccessAndServices );
	void _AddArticleList( CDB_TAProduct* pclTAP, int iQuantity );
	void _AddSafetyValveArticleList( CDS_SSelSafetyValve* pclSSelSafetyValve );
	void _AddBlowTankArticleList( CDS_SSelSafetyValve* pclSSelSafetyValve );

	// HYS-1741
	bool InitDSAccessoriesAndService( CSheetDescription* pclSheetDescription, CDS_SelPWQAccServices** paSSelAccessAndServices, int iAccessAndServicesCount,
								 int iMaxIndexAccServ, bool bResetOrder, int& iRemIndex );
};
