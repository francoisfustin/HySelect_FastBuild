#pragma once


class CSelProdPageSmartDpC : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_SmartDpC = 1,
	};

	CSelProdPageSmartDpC( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageSmartDpC(void) {}
		
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CSelProdPageBase public virtual methods.
	virtual void Reset( void );
	virtual bool PreInit( HMvector &vecHMList );
	virtual bool Init( bool bResetOrder = false, bool bPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABSMARTDPC; }
	// End of overriding CSelProdPageBase public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CSelProdPageBase protected virtual methods.
	virtual void FillAccessoryHelper_GetNameDescription( CDB_Product *pclAccessory, CString &strName, CString &strDescription, LPARAM lpParam = NULL );
	virtual void FillAccessoryHelper_GetErrorMessage( CDB_Product *pclAccessory, int iCol, std::vector<CString> *pvecErrorMsgList, LPARAM lpParam = NULL );
	// End of overriding CSelProdPageBase protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );
	long _FillRowSmartDpC( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartDpC *pclSSelSmartDpC, CDB_SmartControlValve *pclSmartDpC );
	long _FillRowSmartDpC( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM );

	// This method is called by the both '_FillRowSmartDpC' methods above.
	// For the one with 'CDS_SSelSmartDpC', we need 'pclSmartDpCDpSensorSet' argument to have access to reference number to show in the article number 
	// column in case of Smart Dp - Dp sensor package (Don't take 'Article Number' of pTAP because it is the smart differential pressure controller one and not the package one).
	// For the method with 'CDS_HydroMod::CSmartDpC' we need only 'pclSmartDpC'.
	long _FillRowSmartDpCProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_Set *pclSmartDpDpSensorSet = NULL, CString strArtNumber = _T(""), 
			CDS_SSelSmartDpC *pclSSelSmartDpC = NULL, CDS_HydroMod *pclHM = NULL );
	
	typedef struct _SmartDpCParams
	{
		_SmartDpCParams()
		{
			dFlowMax = 0.0;
			dDp = -1.0;
			dDpMin = -1.0;
			strPowerDT = _T( "" );
			dPowerMax = 0.0;
			eLocalization = SmartValveLocalization::SmartValveLocNone;
			pclSSelSmartDpC = NULL;
			pclHM = NULL;
		};

		double dFlowMax;
		double dDp;
		double dDpMin;
		CString strPowerDT;
		double dPowerMax;
		SmartValveLocalization eLocalization;
		CDS_SSelSmartDpC *pclSSelSmartDpC;
		CDS_HydroMod *pclHM;
	}SmartDpCParams;

	long _FillRowSmartDpCInfo( CSheetDescription *pclSheetDescription, long lRow, SmartDpCParams &rSmartDpCParams );

	long _FillRowDpSensor( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartDpC *pclSSelSmartDpC );
	long _FillRowDpSensor( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM );

	long _FillRowDpSensorProduct( CSheetDescription *pclSheetDescription, long lRow, bool bIsSet, 
			CDS_SSelSmartDpC *pclSSelSmartDpC = NULL, CDS_HydroMod *pclHM = NULL );

	long _FillRowDpSensorInfo( CSheetDescription *pclSheetDescription, long lRow, bool bIsSet = false, CDS_SSelSmartDpC *pclSSelSmartDpC = NULL, CDS_HydroMod *pclHM = NULL  );

	long _FillDpSensorAccessoriesHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CSmartDpC *pclHMSmartDpC );

	long _FillRowSet( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartDpC *pclSSelSmartDpC );
	long _FillRowSet( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM );

	long _FillSetContent( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartDpC *pclSSelSmartDpC );
	long _FillSetContent( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CSmartDpC *pclHMSmartDpC );

	void _AddArticleList( CDS_SSelSmartDpC *pclSSelSmartDpC );
	void _AddArticleList( CDS_HydroMod::CSmartDpC *pclHMSmartDpC );
};
