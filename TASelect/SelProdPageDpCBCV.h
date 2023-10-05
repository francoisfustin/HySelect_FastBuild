#pragma once


class CSelProdPageDpCBCV : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_DpCBCValve = 1,
	};

	CSelProdPageDpCBCV( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageDpCBCV(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABDPCBCV; }

// Protected variables.
protected:
	CData* m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _FillDpCBCValve( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv );
	long _FillDpCBCValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpCBCV *pclSSelDpCBCV );
	long _FillDpCBCVTAP( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pTAP );
	long _FillAccessoriesDpCBCVOrActr( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpCBCV *pclSSelDpCBCV, bool fForValve );
	long _FillSetAccessoriesHM( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv );
	long _FillActuator( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpCBCV *pclSSelDpCBCV );
 	long _FillActuatorFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv );
 	long _FillAdapterFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv );

	// Called when individual selection and when actuator doesn't belong to a set with the valve.
	void _AddArticleList( CDB_Actuator *pclActuator, CAccessoryList *pclAccessoryList, int iQuantity, bool fSelectionBySet );
		
	// Called for a hydromod when its actuator doesn't belong to a set with the control valve.
	void _AddArticleList( CDB_Actuator *pclActuator, std::vector<CDS_Actuator::AccessoryItem> *pvecAccessoryList, int iQuantity, bool fSelectionBySet );

	// Called when selected in hydronic calculator.
	void _AddArticleList( CDB_Actuator* pclActuator, CArray<IDPTR>* parAccessory, int iQuantity, bool fSelectionBySet );

	// Called when direct selection.
	void _AddArticleList( CDS_SSelDpCBCV *pclSSelDpCBCV );
	void _AddArticleList( CDS_HydroMod::CCv* pclHMCV );
};
