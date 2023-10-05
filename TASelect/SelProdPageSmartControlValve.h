#pragma once


class CSelProdPageSmartControlValve : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_SmartControlValve = 1,
	};

	CSelProdPageSmartControlValve( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageSmartControlValve(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector &vecHMList );
	virtual bool Init( bool bResetOrder = false, bool bPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABSMARTCONTROLVALVE; }

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _FillRowSmartControlValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartControlValve *pclSSelSmartControlValve, CDB_SmartControlValve *pclSmartControlValve );
	long _FillRowSmartControlValveProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_SmartControlValve *pclSmartControlValve, int iQuantity, CDS_HydroMod *pclHM = NULL );
	
	typedef struct _SmartControlValveParams
	{
		struct _SmartControlValveParams()
		{
			pclSmartControlValve = NULL;
			dFlowMax = 0.0;
			dDp = -1.0;
			dDpMin = -1.0;
			strPowerDT = _T("");
			eLocalization = SmartValveLocalization::SmartValveLocNone;
			eControlMode = SmartValveControlMode::SCVCM_Undefined;
			dPowerMax = 0.0;
			pclHM = NULL;
		}

		CDB_TAProduct *pclSmartControlValve;
		double dFlowMax;
		double dDp;
		double dDpMin;
		CString strPowerDT;
		SmartValveLocalization eLocalization;
		SmartValveControlMode eControlMode;
		double dPowerMax;
		CDS_HydroMod *pclHM;
	}SmartControlValveParams;

	long _FillRowSmartControlValveInfo( CSheetDescription *pclSheetDescription, long lRow, SmartControlValveParams &rSmartControlValveParams );
	
	long _FillRowSmartControlValve( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM, CDS_HydroMod::CBase *pclBase, CDB_TAProduct **ppSmartControlValve = NULL );
	void _AddArticleList( CDB_TAProduct *pclTAP, int iQuantity );
	void _AddArticleList( CDS_SSelSmartControlValve *pclSSelSmartControlValve );
};
