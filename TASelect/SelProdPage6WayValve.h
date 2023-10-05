#pragma once


class CSelProdPage6WayValve : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_6WayValve = 1,
	};

	CSelProdPage6WayValve( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPage6WayValve(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool bResetOrder = false, bool bPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_6WAYVALVE; }

// Protected members.
protected:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CSelProdPageBase protected virtual methods.
	virtual void OnSelectCell( CProductParam &clProductParam );
	virtual void OnUnselectCell( CProductParam &clProductParam );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _Fill6WayValve( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::C6WayValve *pclHM6WayValve );
	long _Fill6WayValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel6WayValve *pclSSel6WayValve );
	long _Fill6WayValveTAP( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pTAP );
	long _Fill6WayValveRowGen( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel6WayValve *pclSSel6WayValve );

	// HYS-1877: Manage and consider Set accessories. 
	long _FillAccessories( CSheetDescription *pclSheetDescription, long lRow, CAccessoryList *pclAccessoryList, int iGlobalQuantity, bool bIsSelectedAspackage = false );
	
	// HYS-1877: CDS_SSelCtrl intead of CDB_ControlValve because we need selection information from CDS_SSelCtrl.
	long _FillActuator( CSheetDescription *pclSheetDescription, long lRow, CDB_Actuator *pclActuator, CDS_SSelCtrl *pclControlValve, int iGlobalQuantity,
			bool bShowFailSafeDRP = true );

	long _FillAdapterFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::C6WayValve *pclHM6WayValve );

	long _FillPIBCValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel6WayValve *pclSSel6WayValve, SideDefinition eSideDefinition );
	long _FillPIBCValveTAP( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pTAP );

	long _FillBalancingValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel6WayValve *pclSSel6WayValve, SideDefinition eSideDefinition );

	void _AddArticleList( CDS_SSel6WayValve *pclSSel6WayValve );
	void _AddArticleList( CDS_HydroMod::C6WayValve *pclHM6WayValve );

	// Called by '_AddArticleList' for 'CDS_SSel6WayValve' object.
	void _AddArticleList( CDB_Actuator *pclActuator, CAccessoryList *pclAccessoryList, int iQuantity );

	// Called by '_AddArticleList' for 'CDS_HydroMod::C6WayValve' object.
	void _AddArticleList( CDB_Actuator *pclActuator, CArray<IDPTR> *parAccessory, int iQuantity );

	void _AddArticleList( CDB_Actuator *pclActuator, std::vector<CDS_Actuator::AccessoryItem> *pvecAccessoryList, int iQuantity );

	void _AddArticleListPIBCvHelper( CDS_SSelPICv *pclSSelPIBCValve, int iGlobalQuantity );
	void _AddArticleListBvHelper( CDS_SSelBv *pclSSelBalancingValve, int iGlobalQuantity );

	// 'bIsMain' is to differentiate between '6-way valve' and 'Pressure indep. balancing & control valve' titles (For the EQMControl case).
	void _GetGroupColorAndSizes( bool bIsMain, SideDefinition eSideDefinition, COLORREF &clTextColor, COLORREF &clBackgroundColor, double &dRowSize, int &iFontSize );
};
