#pragma once


class CSelProdPageTrv : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_Trv = 1,
	};

	CSelProdPageTrv( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageTrv(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABTRV; }

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription* pclSheetDescription );
	void _FillRowTrvTAP( CSheetDescription* pclSheetDescription, long lRow, CDB_TAProduct* pTAP, int iQuantity );
	long _FillRowSupplyValve( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pHMCv );
	long _FillRowSupplyValve( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelRadSet* pSelRadSet );
	long _FillRowSVInsert( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelRadSet* pSelRadSet );

	long _FillSVActuatorFromSSel( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelRadSet *pSelRadSet );
	long _FillSVActuatorFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv *pclHMCv );
	long _FillSVActuatorHelper( CSheetDescription *pclSheetDescription, long lRow, CDB_Actuator *pclActuator, int iQuantity, 
		CAccessoryList *pclAccessoryList, bool fIsSet );
	
	long _FillRowReturnValve( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelRadSet* pSelRadSet );
	
	void _AddArticleList( CDB_TAProduct* pTAP, int iQuantity );
		
	// Add article for supply or return valve and their accessories.
	void _AddArticleList( CDB_TAProduct* pTAP, int iQuantity, CAccessoryList* pclAccessoryList, bool fSelectionBySet );
		
	// Add article for actuator and its accessories.
	void _AddArticleList( CDB_Actuator* pclActuator, int iQuantity, CAccessoryList* pclAccessoryList, bool fSelectionBySet );
};
