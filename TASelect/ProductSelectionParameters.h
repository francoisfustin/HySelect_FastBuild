#pragma once


#include "RViewSSelSS.h"
#include "DlgSelectionBase.h"
#include "DlgBatchSelBase.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for each product selection type.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class parameters for each product selection type.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Base class parameters to allow dialog to suggest selection.
class CProductSelelectionParameters
{
public:
	CProductSelelectionParameters();

	CProductSelelectionParameters( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB,
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CProductSelelectionParameters() {}

	virtual void CopyFrom( CProductSelelectionParameters *pclProductSelectionParameters );
	virtual void Clean();

// Public variables.
public:
	CTADatabase *m_pTADB;										// Database containing TA products
	CTADatastruct *m_pTADS;										// Data structure constituting the document
	CUserDatabase *m_pUserDB;									// Database that contains object belonging to the user.
	CPipeUserDatabase *m_pPipeDB;								// Database that contains all pipes (default and user).
	ProductSubCategory m_eProductSubCategory;
	ProductSelectionMode m_eProductSelectionMode;
	CWaterChar m_WC;
	ProjectType m_eApplicationType;
	CString m_strPipeSeriesID;
	CString m_strPipeID;
	bool m_bEditModeRunning;
	IDPTR m_SelIDPtr;											// Used to modify selected object
	CTADatabase::FilterSelection m_eFilterSelection;

// Private methods.
private:
	void _CleanInternal();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DIRECT SELECTION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDirectSelectionParameters : public CProductSelelectionParameters
{
public:
	CDirectSelectionParameters() {}
	virtual ~CDirectSelectionParameters() {}

	// Nothing for the moment, it's just to be help to integrate all selection types.
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INDIVIDUAL SELECTION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Base class parameters for individual selection.
class CIndividualSelectionParameters : public CProductSelelectionParameters
{
public:
	CIndividualSelectionParameters();

	CIndividualSelectionParameters( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndividualSelectionParameters() {}

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void CopyFrom( CProductSelelectionParameters *pclProductSelectionParameters );
	virtual void Clean();

	int GetComboTypeAllID( void ) { return IDS_COMBOTEXT_ALL_TYPES; }
	int GetComboFamilyAllID( void ) { return IDS_COMBOTEXT_ALL_FAMILIES; }
	int GetComboMaterialAllID( void ) { return IDS_COMBOTEXT_ALL_MATERIALS; }
	int GetComboConnectAllID( void ) { return IDS_COMBOTEXT_ALL_CONNECTIONS; }
	int GetComboVersionAllID( void ) { return IDS_COMBOTEXT_ALL_VERSIONS; }
	int GetComboPNAllID( void ) { return IDS_COMBOTEXT_ALL; }
	int GetComboActuatorPowerSupplyAllID( void ) { return IDS_COMBOTEXT_ALL_POWERSUPPLIES; }
	int GetComboActuatorInputSignalAllID( void ) { return IDS_COMBOTEXT_ALL_INPUTSIGNALS; }
	int GetComboActuatorFailSafeAllID( void ) { return IDS_COMBOTEXT_ALL_DRP; }

// Public variables.
public:
	CDS_SelProd::eFlowDef m_eFlowOrPowerDTMode;
	double m_dFlow;
	double m_dPower;
	double m_dDT;
	bool m_bDpEnabled;
	double m_dDp;
	CString m_strComboTypeID;
	CString m_strComboFamilyID;
	CString m_strComboMaterialID;
	CString m_strComboConnectID;
	CString m_strComboVersionID;
	CString m_strComboPNID;
	bool m_bOnlyForSet;
	bool m_bCheckboxSetEnabled;
 
// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for balancing valves.
class CSelectList;
class CIndSelBVParams : public CIndividualSelectionParameters
{
public:
	CIndSelBVParams();

	CIndSelBVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelBVParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CRankEx m_BvList;
	CSelectList *m_pclSelectBvList;

// Private methods.
private:
	void _CleanInternal();
};

// Base class parameters for individual selection for control valves, balancing and control valves, pressure independent
// balancing and control valves and combined Dp controller, balancing and control valves.
class CIndSelCtrlParamsBase : public CIndividualSelectionParameters
{
public:
	CIndSelCtrlParamsBase();

	CIndSelCtrlParamsBase( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelCtrlParamsBase() {}

	virtual CSelectCtrlList *GetSelectCtrlList() = 0;

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CRankEx m_CtrlList;
	CDB_ControlProperties::CV2W3W m_eCV2W3W;
	CDB_ControlProperties::CvCtrlType m_eCvCtrlType;
	CString m_strActuatorPowerSupplyID;
	CString m_strActuatorInputSignalID;
	int m_iActuatorFailSafeFunction;
	CDB_ControlValve::DRPFunction m_eActuatorDRPFunction;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for balancing and control valves.
class CIndSelBCVParams : public CIndSelCtrlParamsBase
{
public:
	CIndSelBCVParams();

	CIndSelBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelBCVParams();

	// Overrides 'CIndSelCtrlParamsBase' public pure virtual method.
	virtual CSelectCtrlList *GetSelectCtrlList() { return m_pclSelectBCVList; }

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CSelectBCVList *m_pclSelectBCVList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for pure control valves.
class CIndSelCVParams : public CIndSelCtrlParamsBase
{
public:
	CIndSelCVParams();

	CIndSelCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelCVParams();

	// Overrides 'CIndSelCtrlParamsBase' public pure virtual method.
	virtual CSelectCtrlList *GetSelectCtrlList() { return (CSelectCtrlList *)m_pclSelectCVList; }

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	double m_dKvs;
	bool m_bIsGroupKvsOrDpChecked;
	CDS_SSelCv::KvsOrDp m_eRadioKvsOrDp;
	CSelectCVList *m_pclSelectCVList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for Dp controller.
class CIndSelDpCParams : public CIndividualSelectionParameters
{
public:
	CIndSelDpCParams();

	CIndSelDpCParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelDpCParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	bool m_bIsGroupDpbranchOrKvChecked;
	bool m_bIsDpMaxChecked;
	double m_dDpBranch;
	double m_dDpMax;
	double m_dKv;
	bool m_bIsDpCInfoWndChecked;
	eDpStab m_eDpStab;
	eDpCLoc m_eDpCLoc;
	eMvLoc m_eMvLoc;
	CRankEx m_DpCList;
	CSelectDpCList *m_pclSelectDpCList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for combined Dp controller, balancing and control valves.
class CIndSelDpCBCVParams : public CIndSelCtrlParamsBase
{
public:
	CIndSelDpCBCVParams();

	CIndSelDpCBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelDpCBCVParams();

	// Overrides 'CIndSelCtrlParamsBase' public pure virtual method.
	virtual CSelectCtrlList *GetSelectCtrlList() { return (CSelectCtrlList *)m_pclSelectDpCBCVList; }

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	bool m_bIsGroupDpToStabilizeChecked;
	double m_dDpToStabilize;
	bool m_bIsWithSTSChecked;
	bool m_bIsDpCInfoWndChecked;
	CSelectDpCBCVList *m_pclSelectDpCBCVList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for HUB.
class CIndSelHUBParams : public CIndividualSelectionParameters
{
public:
	CIndSelHUBParams() : CIndividualSelectionParameters() {}
	
	CIndSelHUBParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") )
		: CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeD, pclWC, strPipeSeriesID, strPipeID ) {}
	
	virtual ~CIndSelHUBParams() {}

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean() {}
};

// Class parameters for pressure independent balancing and control valves.
class CIndSelPIBCVParams : public CIndSelCtrlParamsBase
{
public:
	CIndSelPIBCVParams();

	CIndSelPIBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelPIBCVParams();

	// Overrides 'CIndSelCtrlParamsBase' public pure virtual method.
	virtual CSelectCtrlList *GetSelectCtrlList() { return m_pclSelectPIBCVList; }

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	bool m_bIsDpMaxChecked;
	double m_dDpMax;
	CSelectPICVList *m_pclSelectPIBCVList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for 6-way control valves.
class CIndSel6WayValveParams : public CIndSelCtrlParamsBase
{
public:
	CIndSel6WayValveParams();

	CIndSel6WayValveParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSel6WayValveParams();

	// Overrides 'CIndSelCtrlParamsBase' public pure virtual method.
	virtual CSelectCtrlList *GetSelectCtrlList() { return m_pclSelect6WayValveList; }
	
	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

	double GetHeatingFlow() { return m_dHeatingFlow; }
	double GetHeatingPower() { return m_dHeatingPower; }
	double GetHeatingDT() { return m_dHeatingDT; }
	double GetHeatingSupplyTemp() { return m_dHeatingSupplyTemp; }
	CWaterChar &GetHeatingWaterChar() { return m_HeatingWC; }

	double GetCoolingFlow() { return m_dCoolingFlow; }
	double GetCoolingPower() { return m_dCoolingPower; }
	double GetCoolingDT() { return m_dCoolingDT; }
	double GetCoolingSupplyTemp() { return m_dCoolingSupplyTemp; }
	CWaterChar &GetCoolingWaterChar() { return m_CoolingWC; }

// Public variables.
public:
	e6WayValveSelectionMode m_e6WayValveSelectionMode;

	// For heating, it's the ones defined in the 'CIndividualSelectionParameters' base class.
	double m_dCoolingFlow;
	double m_dCoolingPower;
	double m_dCoolingDT;

	double m_dHeatingFlow;
    double m_dHeatingPower;
    double m_dHeatingDT;
	double m_dCoolingSupplyTemp;
    double m_dHeatingSupplyTemp;

	CWaterChar m_CoolingWC;
	CWaterChar m_HeatingWC;

	CRankEx m_e6WayValveList;
	CSelect6WayValveList *m_pclSelect6WayValveList;

	CIndSelPIBCVParams m_clIndSelPIBCVParams;
	CIndSelBVParams m_clIndSelBVParams;

	bool m_bCheckFastConnection;

	// HYS-1877: Add Set selection
	bool m_bOnlyForSet;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for smart control valves.
class CSelectList;
class CIndSelSmartControlValveParams : public CIndividualSelectionParameters
{
public:
	CIndSelSmartControlValveParams();

	CIndSelSmartControlValveParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelSmartControlValveParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CRankEx m_SmartControlValveList;
	CSelectSmartControlValveList *m_pclSelectSmartControlValveList;
	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	bool m_bIsDpMaxChecked;
	double m_dDpMax;
// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for smart differential controllers.
// HYS-1937: TA-Smart Dp - 04 - Individual selection: left tab.
class CSelectList;
class CIndSelSmartDpCParams : public CIndividualSelectionParameters
{
public:
	CIndSelSmartDpCParams();

	CIndSelSmartDpCParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD,
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelSmartDpCParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	bool m_bIsGroupDpbranchChecked;
	double m_dDpBranch;
	bool m_bIsDpMaxChecked;
	double m_dDpMax;
	CRankEx m_SmartDpCList;
	CSelectSmartDpCList *m_pclSelectSmartDpCList;
	bool m_bOnlyForSet;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for pressurisation maintenance system.
// Remark: this class doesn't need all the variables in the 'CIndividualSelectionParameters'. This is why it is directly inherited
//         from the 'CProductSelectionParameters' class.
class CSelectPMList;
class CIndSelPMParams : public CProductSelelectionParameters
{
public:
	CIndSelPMParams();

	CIndSelPMParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelPMParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	bool m_bIsVesselMembraneTypeExistInPMTypeCombo;
	CSelectPMList *m_pclSelectPMList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for air vents and separators.
class CIndSelSeparatorParams : public CIndividualSelectionParameters
{
public:
	CIndSelSeparatorParams();

	CIndSelSeparatorParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelSeparatorParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CRankEx m_SeparatorList;
	CSelectSeparatorList *m_pclSelectSeparatorList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for shutoff valves.
class CIndSelSVParams : public CIndividualSelectionParameters
{
public:
	CIndSelSVParams();

	CIndSelSVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelSVParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CRankEx m_SVList;
	CSelectShutoffList *m_pclSelectSVList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for thermostatic regulative valves.
class CIndSelTRVParams : public CIndividualSelectionParameters
{
public:
	CIndSelTRVParams();

	CIndSelTRVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CIndSelTRVParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	bool m_bIsTrvTypePreset;				// It is more a working variable (not needed to select TRV but to display them).
	RadiatorValveType m_eValveType;
	RadiatorInsertType m_eInsertType;
	RadiatorReturnValveMode m_eReturnValveMode;
	bool m_bIsFLCVInsert;					// HYS-1305 : Is thermostatic insert with automatic flow limiter or not
	bool m_bIsThermostaticHead;
	double m_dInsertKvValue;
	CString m_strSVTypeID;					// There is no combo for the type but we use type to extract valves from the database.
	CString m_strComboSVFamilyID;
	CString m_strComboSVConnectID;
	CString m_strComboSVVersionID;
	CString m_strComboRVFamilyID;
	CString m_strComboRVConnectID;
	CString m_strComboRVVersionID;
	CString m_strComboSVInsertName;
	CString m_strComboSVInsertFamilyID;
	CString m_strComboSVInsertTypeID;
	CString m_strComboRVInsertFamilyID;
	CString m_strComboActuatorTypeID;
	CString m_strComboActuatorFamilyID;
	CString m_strComboActuatorVersionID;
	CRankEx m_SupplyValveList;
	RadInfo_struct m_RadInfos;
	CSelectTrvList *m_pclSelectSupplyValveList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for individual selection for safety valves.
class CIndSelSafetyValveParams : public CIndividualSelectionParameters
{
public:
	CIndSelSafetyValveParams();

	CIndSelSafetyValveParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC );

	virtual ~CIndSelSafetyValveParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	ProjectType m_eSystemApplicationType;
	CString m_strSystemHeatGeneratorTypeID;					// Only for heating.
	CString m_strNormID;
	double m_dInstalledPower;
	double m_dInstalledCollector;							// Only for solar.
	CString m_strSafetyValveFamilyID;
	CString m_strSafetyValveConnectionID;
	double m_dUserSetPressureChoice;
	CRankEx m_SafetyValveList;
	CSelectSafetyValveList *m_pclSelectSafetyValveList;

// Private methods.
private:
	void _CleanInternal();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BATCH SELECTION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Base class parameters for batch selection.
class CDlgBatchSelBase;
class CBatchSelectionParameters : public CProductSelelectionParameters
{
public:
	CBatchSelectionParameters();

	CBatchSelectionParameters( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelectionParameters() {}

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void CopyFrom( CProductSelelectionParameters *pclProductSelectionParameters );
	virtual void Clean();

// Public variables.
public:
	CDS_SelProd::eFlowDef m_eFlowOrPowerDTMode;
	bool m_bIsDpGroupChecked;
	CString m_strComboTypeBelow65ID;
	CString m_strComboFamilyBelow65ID;
	CString m_strComboMaterialBelow65ID;
	CString m_strComboConnectBelow65ID;
	CString m_strComboVersionBelow65ID;
	CString m_strComboPNBelow65ID;
	CString m_strComboTypeAbove50ID;
	CString m_strComboFamilyAbove50ID;
	CString m_strComboMaterialAbove50ID;
	CString m_strComboConnectAbove50ID;
	CString m_strComboVersionAbove50ID;
	CString m_strComboPNAbove50ID;
	std::vector<CDlgBatchSelBase::BSRowParameters *> m_vecRowParameters;			// To help us to drop out a current batch selection.

// Private methods.
private:
	void _CleanInternal();
};

// Base class parameters for batch selection for balancing and control valves and pressure independent
// balancing and control valves.
class CBatchSelCtrlParamsBase : public CBatchSelectionParameters
{
public:
	CBatchSelCtrlParamsBase();

	CBatchSelCtrlParamsBase( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelCtrlParamsBase() {}

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	bool m_bIsCtrlTypeStrictChecked;
	CDB_ControlProperties::CV2W3W m_eCV2W3W;
	CDB_ControlProperties::CvCtrlType m_eCvCtrlType;
	bool m_bActuatorSelectedAsSet;
	CString m_strActuatorPowerSupplyID;
	CString m_strActuatorInputSignalID;
	int m_iActuatorFailSafeFunction;
	CDB_ControlValve::DRPFunction m_eActuatorDRPFunction;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for batch selection for balancing & control valves.
class CBatchSelBCVParams : public CBatchSelCtrlParamsBase
{
public:
	CBatchSelBCVParams();

	CBatchSelBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelBCVParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CBatchSelectBCVList *m_pclBatchBCVList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for batch selection for balancing valves.
class CBatchSelBVParams : public CBatchSelectionParameters
{
public:
	CBatchSelBVParams();

	CBatchSelBVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelBVParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CBatchSelectBvList *m_pclBatchBVList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for batch selection for Dp controller valves.
class CBatchSelDpCParams : public CBatchSelectionParameters
{
public:
	CBatchSelDpCParams();

	CBatchSelDpCParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelDpCParams() {}

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	// HYS-1188: we split 'm_bIsCheckboxDpbranchOrKvChecked' in 2 variables.
	bool m_bIsCheckboxDpBranchChecked;
	bool m_bIsCheckboxKvsChecked;
	bool m_bIsDpMaxChecked;
	double m_dDpMax;
	eDpStab m_eDpStab;
	eMvLoc m_eMvLoc;
	eDpCLoc m_eDpCLoc;
	CBatchSelectDpCList *m_pclBatchDpCList;
	CBatchSelBVParams m_clBatchSelBVParams;			// To save all data corresponding to the balancing valve.

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for batch selection for combined Dp controller, balancing and control valves.
class CBatchSelDpCBCVParams : public CBatchSelectionParameters
{
public:
	CBatchSelDpCBCVParams() : CBatchSelectionParameters() {}
	
	CBatchSelDpCBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") )
		:CBatchSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeD, pclWC, strPipeSeriesID, strPipeID ) {}
	
	virtual ~CBatchSelDpCBCVParams() {}

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for batch selection for pressure independent balancing and control valves.
class CBatchSelPIBCVParams : public CBatchSelCtrlParamsBase
{
public:
	CBatchSelPIBCVParams();

	CBatchSelPIBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelPIBCVParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CBatchSelectPICvList *m_pclBatchPIBCVList;
	
// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for batch selection for air vents and separators.
class CBatchSelSeparatorParams : public CBatchSelectionParameters
{
public:
	CBatchSelSeparatorParams();

	CBatchSelSeparatorParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelSeparatorParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CString m_strComboTypeID;
	CString m_strComboFamilyID;
	CString m_strComboConnectID;
	CString m_strComboVersionID;
	CBatchSelectSeparatorList *m_pclBatchSeparatorList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for batch selection for smart control valves.
class CBatchSelSmartControlValveParams : public CBatchSelectionParameters
{
public:
	CBatchSelSmartControlValveParams();

	CBatchSelSmartControlValveParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelSmartControlValveParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CBatchSelectSmartControlValveList *m_pclBatchSmartControlValveList;

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for batch selection for smart differential pressure controllers.
class CBatchSelSmartDpCParams : public CBatchSelectionParameters
{
public:
	CBatchSelSmartDpCParams();

	CBatchSelSmartDpCParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CBatchSelSmartDpCParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CBatchSelectSmartDpCList *m_pclBatchSmartDpCList;
	bool m_bIsSelectedAsSet;
	bool m_bIsCheckboxDpBranchChecked;
	bool m_bIsDpMaxChecked;
	double m_dDpMax;

// Private methods.
private:
	void _CleanInternal();
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WIZARD SELECTION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Base class parameters for wizard selection.
class CWizardSelectionParameters : public CProductSelelectionParameters
{
public:
	CWizardSelectionParameters();

	CWizardSelectionParameters( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CWizardSelectionParameters();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void CopyFrom( CProductSelelectionParameters *pclProductSelectionParameters );
	virtual void Clean();

// Private methods.
private:
	void _CleanInternal();
};

// Class parameters for wizard selection for pressurisation.
class CWizardSelPMParams : public CWizardSelectionParameters
{
public:
	CWizardSelPMParams();

	CWizardSelPMParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeD, 
			CWaterChar *pclWC, CString strPipeSeriesID = _T(""), CString strPipeID = _T("") );

	virtual ~CWizardSelPMParams();

	// Overrides 'CProductSelelectionParameters' public virtual method.
	virtual void Clean();

// Public variables.
public:
	CSelectPMList *m_pclSelectPMList;

// Private methods.
private:
	void _CleanInternal();
};
