#pragma once


class CSelProdPageCtrl : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_ControlValve = 1,
	};

	CSelProdPageCtrl( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageCtrl(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABCV; }


	// Allow to retrieve a list of accessories that are common with accessories of control valve defined in first IDPTR and with accessories of actuators
	// defined in the second IDPTR.
	int GetFUSIONAccessories( CDB_ControlValve* pclControlValve, std::set< CDB_Product* >* pclAccessoryList );

// Protected variables.
protected:
	CData* m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription* pclSheetDescription );
	long _FillControlValve( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv );
	long _FillControlValve( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelCtrl* pclSSelCtrl );
	long _FillCvTAP( CSheetDescription* pclSheetDescription, long lRow, CDB_TAProduct* pTAP );
	long _FillAccessoryAlone( CSheetDescription* pclSheetDescription, long lRow, CDS_Accessory* pclSSelAccessory, bool fWithArticleNumber );
	long _FillAccessoriesCvOrActr( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelCtrl* pclSSelCtrl, bool fForCtrl );
	long _FillSetAccessoriesHM( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv );
	long _FillActuatorSelectedWithCv( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelCtrl* pclSSelCtrl );
	long _FillActuatorSelectedAlone( CSheetDescription* pclSheetDescription, long lRow, CDS_Actuator* pclSSelActuator );
	long _FillActuatorFromHM( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv );
	long _FillAdapterFromHM( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv );

	bool _InitDSActuator( CSheetDescription* pclSheetDescription, CDS_Actuator** paActr, int iCount, int iMax, bool fSort, int &iRemIndex );
	bool _InitDSAccessory( CSheetDescription* pclSheetDescription, CDS_Accessory** paAcc, int iCount, int iMax, bool fSort, int &iRemIndex );
		
	// called when individual selection and when actuator doesn't belong to a set with the control valve.
	void _AddArticleList( CDB_Actuator* pclActuator, CAccessoryList* pclAccessoryList, int iQuantity, bool fSelectionBySet );
		
	// Called for a hydromod when its actuator doesn't belong to a set with the control valve.
	void _AddArticleList( CDB_Actuator* pclActuator, std::vector<CDS_Actuator::AccessoryItem>* pvecAccessoryList, int iQuantity, bool fSelectionBySet );

	// Called when direct selection and when selected in hydronic calculator.
	void _AddArticleList( CDB_Actuator* pclActuator, CArray<IDPTR>* parAccessory, int iQuantity, bool fSelectionBySet );
		
	void _AddArticleList( CDS_SSelCtrl* pclSSelCtrl );
	void _AddArticleList( CDS_HydroMod::CCv* pclHMCV );
};
