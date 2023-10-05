#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "ProductSelectionParameters.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for each product selection type.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class parameters for each product selection type.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CProductSelelectionParameters::CProductSelelectionParameters()
{
	Clean();
}

CProductSelelectionParameters::CProductSelelectionParameters( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID, CString strPipeID )
{
	m_pTADB = pclTADB;
	m_pTADS = pclTADS;
	m_pUserDB = pclUserDB;
	m_pPipeDB = pclPipeDB;
	m_WC = *pclWC;
	m_strPipeSeriesID = strPipeSeriesID;
	m_strPipeID = strPipeID;

	_CleanInternal();
}

void CProductSelelectionParameters::CopyFrom( CProductSelelectionParameters *pclProductSelectionParameters )
{
	if( NULL == pclProductSelectionParameters )
	{
		return;
	}

	m_pTADB = pclProductSelectionParameters->m_pTADB;
	m_pTADS = pclProductSelectionParameters->m_pTADS;
	m_pUserDB = pclProductSelectionParameters->m_pUserDB;
	m_pPipeDB = pclProductSelectionParameters->m_pPipeDB;
	m_eProductSubCategory = pclProductSelectionParameters->m_eProductSubCategory;
	m_eProductSelectionMode = pclProductSelectionParameters->m_eProductSelectionMode;
	m_WC = pclProductSelectionParameters->m_WC;
	m_eApplicationType = pclProductSelectionParameters->m_eApplicationType;
	m_strPipeSeriesID = pclProductSelectionParameters->m_strPipeSeriesID;
	m_strPipeID = pclProductSelectionParameters->m_strPipeID;
	m_bEditModeRunning = pclProductSelectionParameters->m_bEditModeRunning;
	m_SelIDPtr = pclProductSelectionParameters->m_SelIDPtr;
	m_eFilterSelection = pclProductSelectionParameters->m_eFilterSelection;
}

void CProductSelelectionParameters::Clean()
{
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUserDB = NULL;
	m_pPipeDB = NULL;
	m_strPipeSeriesID = _T("STEEL_GEN");
	m_strPipeID = _T("");

	_CleanInternal();
}

void CProductSelelectionParameters::_CleanInternal()
{
	m_eProductSubCategory = ProductSubCategory::PSC_First;
	m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_First;
	m_eApplicationType = ProjectType::Heating;
	m_bEditModeRunning = false;
	m_SelIDPtr = _NULL_IDPTR;
	m_eFilterSelection = CTADatabase::ForIndAndBatchSel;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INDIVIDUAL SELECTION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class parameters for individual selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndividualSelectionParameters::CIndividualSelectionParameters()
	:CProductSelelectionParameters()
{
	_CleanInternal();
}

CIndividualSelectionParameters::CIndividualSelectionParameters( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CProductSelelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

void CIndividualSelectionParameters::CopyFrom( CProductSelelectionParameters *pclProductSelectionParameters )
{
	if( NULL == pclProductSelectionParameters )
	{
		return;
	}

	CProductSelelectionParameters::CopyFrom( pclProductSelectionParameters );

	if( NULL == dynamic_cast<CIndividualSelectionParameters *>( pclProductSelectionParameters ) )
	{
		return;
	}

	CIndividualSelectionParameters *pclIndSelParamsSource = dynamic_cast<CIndividualSelectionParameters *>( pclProductSelectionParameters );
	m_eFlowOrPowerDTMode = pclIndSelParamsSource->m_eFlowOrPowerDTMode;
	m_dFlow = pclIndSelParamsSource->m_dFlow;
	m_dPower = pclIndSelParamsSource->m_dPower;
	m_dDT = pclIndSelParamsSource->m_dDT;
	m_bDpEnabled = pclIndSelParamsSource->m_bDpEnabled;
	m_dDp = pclIndSelParamsSource->m_dDp;
	m_strComboTypeID = pclIndSelParamsSource->m_strComboTypeID;
	m_strComboFamilyID = pclIndSelParamsSource->m_strComboFamilyID;
	m_strComboMaterialID = pclIndSelParamsSource->m_strComboMaterialID;
	m_strComboConnectID = pclIndSelParamsSource->m_strComboConnectID;
	m_strComboVersionID = pclIndSelParamsSource->m_strComboVersionID;
	m_strComboPNID = pclIndSelParamsSource->m_strComboPNID;
	m_bOnlyForSet = pclIndSelParamsSource->m_bOnlyForSet;
	m_bCheckboxSetEnabled = pclIndSelParamsSource->m_bCheckboxSetEnabled;
}

void CIndividualSelectionParameters::Clean()
{
	CProductSelelectionParameters::Clean();
	_CleanInternal();
}

void CIndividualSelectionParameters::_CleanInternal()
{
	m_eFlowOrPowerDTMode = CDS_SelProd::efdFlow;
	m_dFlow = 0.0;
	m_dPower = 0.0;
	m_dDT = 0.0;
	m_bDpEnabled = false;
	m_dDp = 0.0;
	m_strComboTypeID = _T("");
	m_strComboFamilyID = _T("");
	m_strComboMaterialID = _T("");
	m_strComboConnectID = _T("");
	m_strComboVersionID = _T("");
	m_strComboPNID = _T("");
	m_bOnlyForSet = false;
	m_bCheckboxSetEnabled = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for balancing valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelBVParams::CIndSelBVParams() 
	: CIndividualSelectionParameters()
{
	m_pclSelectBvList = NULL;
	_CleanInternal();
}

CIndSelBVParams::CIndSelBVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectBvList = NULL;
	_CleanInternal();
}

CIndSelBVParams::~CIndSelBVParams()
{
	_CleanInternal();
}

void CIndSelBVParams::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelBVParams::_CleanInternal()
{
	m_BvList.PurgeAll();
	
	if( NULL != m_pclSelectBvList )
	{
		delete m_pclSelectBvList;
	}

	m_pclSelectBvList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class parameters for individual selection for control valves, balancing and control valves, pressure 
// independent balancing and control valves and combined Dp controller, balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelCtrlParamsBase::CIndSelCtrlParamsBase() 
	: CIndividualSelectionParameters()
{
	_CleanInternal();
}

CIndSelCtrlParamsBase::CIndSelCtrlParamsBase( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

void CIndSelCtrlParamsBase::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelCtrlParamsBase::_CleanInternal()
{
	m_CtrlList.PurgeAll();
	m_eCV2W3W = CDB_ControlProperties::LastCV2W3W;
	m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvNU;
	m_strActuatorPowerSupplyID = _T("");
	m_strActuatorInputSignalID = _T("");
	m_iActuatorFailSafeFunction = 0;
	m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfUndefined;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelBCVParams::CIndSelBCVParams() 
	: CIndSelCtrlParamsBase()
{
	m_pclSelectBCVList = NULL;
	_CleanInternal();
}

CIndSelBCVParams::CIndSelBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndSelCtrlParamsBase( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectBCVList = NULL;
	_CleanInternal();
}

CIndSelBCVParams::~CIndSelBCVParams()
{
	_CleanInternal();
}

void CIndSelBCVParams::Clean()
{
	CIndSelCtrlParamsBase::Clean();
	_CleanInternal();
}

void CIndSelBCVParams::_CleanInternal()
{
	if( NULL != m_pclSelectBCVList )
	{
		delete m_pclSelectBCVList;
	}

	m_pclSelectBCVList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for pure control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelCVParams::CIndSelCVParams() 
	: CIndSelCtrlParamsBase()
{
	m_pclSelectCVList = NULL;
	_CleanInternal();
}

CIndSelCVParams::CIndSelCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndSelCtrlParamsBase( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectCVList = NULL;
	_CleanInternal();
}

CIndSelCVParams::~CIndSelCVParams()
{
	_CleanInternal();
}

void CIndSelCVParams::Clean()
{
	CIndSelCtrlParamsBase::Clean();
	_CleanInternal();
}

void CIndSelCVParams::_CleanInternal()
{
	m_dKvs = 0.0;
	m_bIsGroupKvsOrDpChecked = false;
	m_eRadioKvsOrDp = CDS_SSelCv::KvsOrDp::Kvs;

	if( NULL != m_pclSelectCVList )
	{
		delete m_pclSelectCVList;
	}

	m_pclSelectCVList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for Dp controller.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelDpCParams::CIndSelDpCParams() 
	: CIndividualSelectionParameters()
{
	m_pclSelectDpCList = NULL;
	_CleanInternal();
}

CIndSelDpCParams::CIndSelDpCParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectDpCList = NULL;
	_CleanInternal();
}

CIndSelDpCParams::~CIndSelDpCParams()
{
	_CleanInternal();
}

void CIndSelDpCParams::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelDpCParams::_CleanInternal()
{
	m_bIsGroupDpbranchOrKvChecked = false;
	m_bIsDpMaxChecked = false;
	m_dDpBranch = 0.0;
	m_dDpMax = 0.0;
	m_dKv = 0.0;
	m_bIsDpCInfoWndChecked = false;
	m_eDpStab = eDpStab::DpStabNone;
	m_eMvLoc = eMvLoc::MvLocNone;
	m_eDpCLoc = eDpCLoc::DpCLocNone;

	if( NULL != m_pclSelectDpCList )
	{
		delete m_pclSelectDpCList;
	}

	m_pclSelectDpCList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for combined Dp controller, balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelDpCBCVParams::CIndSelDpCBCVParams() 
	: CIndSelCtrlParamsBase()
{
	m_pclSelectDpCBCVList = NULL;
	_CleanInternal();
}

CIndSelDpCBCVParams::CIndSelDpCBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndSelCtrlParamsBase( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectDpCBCVList = NULL;
	_CleanInternal();
}

CIndSelDpCBCVParams::~CIndSelDpCBCVParams()
{
	_CleanInternal();
}

void CIndSelDpCBCVParams::Clean()
{
	CIndSelCtrlParamsBase::Clean();
	_CleanInternal();
}

void CIndSelDpCBCVParams::_CleanInternal()
{
	m_bIsGroupDpToStabilizeChecked = false;
	m_dDpToStabilize = -1.0;
	m_bIsWithSTSChecked = false;
	m_bIsDpCInfoWndChecked = false;
	
	if( NULL != m_pclSelectDpCBCVList )
	{
		delete m_pclSelectDpCBCVList;
	}

	m_pclSelectDpCBCVList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for pressure independent balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelPIBCVParams::CIndSelPIBCVParams() 
	: CIndSelCtrlParamsBase()
{
	m_pclSelectPIBCVList = NULL;
	_CleanInternal();
}

CIndSelPIBCVParams::CIndSelPIBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndSelCtrlParamsBase( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectPIBCVList = NULL;
	_CleanInternal();
}

CIndSelPIBCVParams::~CIndSelPIBCVParams()
{
	_CleanInternal();
}

void CIndSelPIBCVParams::Clean()
{
	CIndSelCtrlParamsBase::Clean();
	_CleanInternal();
}

void CIndSelPIBCVParams::_CleanInternal()
{
	m_bIsDpMaxChecked = false;
	m_dDpMax = 0.0;
	
	if( NULL != m_pclSelectPIBCVList )
	{
		delete m_pclSelectPIBCVList;
	}

	m_pclSelectPIBCVList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for 6-way control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSel6WayValveParams::CIndSel6WayValveParams() 
	: CIndSelCtrlParamsBase()
{
	m_pclSelect6WayValveList = NULL;
	_CleanInternal();
}

CIndSel6WayValveParams::CIndSel6WayValveParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndSelCtrlParamsBase( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelect6WayValveList = NULL;
	_CleanInternal();
}

CIndSel6WayValveParams::~CIndSel6WayValveParams()
{
	_CleanInternal();
}

void CIndSel6WayValveParams::Clean()
{
	CIndSelCtrlParamsBase::Clean();
	_CleanInternal();
}

void CIndSel6WayValveParams::_CleanInternal()
{
	if( NULL != m_pclSelect6WayValveList )
	{
		delete m_pclSelect6WayValveList;
	}

	m_dCoolingFlow = 0.0;
	m_dCoolingPower = 0.0;
	m_dCoolingDT = 0.0;
	m_dHeatingFlow = 0.0;
	m_dHeatingPower = 0.0;
	m_dHeatingDT = 0.0;
	m_dCoolingSupplyTemp = 0.0;
	m_dHeatingSupplyTemp = 0.0;
	m_e6WayValveSelectionMode = e6Way_Alone;
	m_pclSelect6WayValveList = NULL;
	m_e6WayValveList.PurgeAll();
	m_clIndSelPIBCVParams.Clean();
	m_clIndSelBVParams.Clean();
	m_bCheckFastConnection = false;
	// HYS-1877: Add Set selection
	m_bOnlyForSet = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for smart control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelSmartControlValveParams::CIndSelSmartControlValveParams() 
	: CIndividualSelectionParameters()
{
	m_pclSelectSmartControlValveList = NULL;
	_CleanInternal();
}

CIndSelSmartControlValveParams::CIndSelSmartControlValveParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectSmartControlValveList = NULL;
	_CleanInternal();
}

CIndSelSmartControlValveParams::~CIndSelSmartControlValveParams()
{
	_CleanInternal();
}

void CIndSelSmartControlValveParams::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelSmartControlValveParams::_CleanInternal()
{
	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	m_bIsDpMaxChecked = false;
	m_dDpMax = 0.0;
	m_SmartControlValveList.PurgeAll();
	
	if( NULL != m_pclSelectSmartControlValveList )
	{
		delete m_pclSelectSmartControlValveList;
	}

	m_pclSelectSmartControlValveList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for smart DpC.
// HYS-1937: TA-Smart Dp - 04 - Individual selection: left tab.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelSmartDpCParams::CIndSelSmartDpCParams() 
	: CIndividualSelectionParameters()
{
	m_pclSelectSmartDpCList = NULL;
	_CleanInternal();
}

CIndSelSmartDpCParams::CIndSelSmartDpCParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	:CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectSmartDpCList = NULL;
	_CleanInternal();
}

CIndSelSmartDpCParams::~CIndSelSmartDpCParams()
{
	_CleanInternal();
}

void CIndSelSmartDpCParams::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelSmartDpCParams::_CleanInternal()
{
	m_bIsGroupDpbranchChecked = false;
	m_dDpBranch = 0.0;
	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	m_bIsDpMaxChecked = false;
	m_dDpMax = 0.0;
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	m_bOnlyForSet = false;
	m_SmartDpCList.PurgeAll();
	
	if( NULL != m_pclSelectSmartDpCList )
	{
		delete m_pclSelectSmartDpCList;
	}

	m_pclSelectSmartDpCList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for pressurisation maintenance system.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelPMParams::CIndSelPMParams() 
	: CProductSelelectionParameters()
{
	m_pclSelectPMList = NULL;
	_CleanInternal();
}

CIndSelPMParams::CIndSelPMParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CProductSelelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectPMList = NULL;
	_CleanInternal();
}

CIndSelPMParams::~CIndSelPMParams()
{
	_CleanInternal();
}

void CIndSelPMParams::Clean()
{
	CProductSelelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelPMParams::_CleanInternal()
{
	m_bIsVesselMembraneTypeExistInPMTypeCombo = false;

	// !!!! PAY ATTENTION: This variable is not created when user clicks on 'Suggest' button like the other cases. This variable
	// points on either heating, cooling or solar that are created in the 'CPMSelectionHelper' class.
	m_pclSelectPMList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for air vents and separators.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelSeparatorParams::CIndSelSeparatorParams() 
	: CIndividualSelectionParameters()
{
	m_pclSelectSeparatorList = NULL;
	_CleanInternal();
}

CIndSelSeparatorParams::CIndSelSeparatorParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectSeparatorList = NULL;
	_CleanInternal();
}

CIndSelSeparatorParams::~CIndSelSeparatorParams()
{
	_CleanInternal();
}

void CIndSelSeparatorParams::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelSeparatorParams::_CleanInternal()
{
	if( NULL != m_pclSelectSeparatorList )
	{
		delete m_pclSelectSeparatorList;
	}

	m_pclSelectSeparatorList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for shutoff valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelSVParams::CIndSelSVParams() 
	: CIndividualSelectionParameters()
{
	m_pclSelectSVList = NULL;
	_CleanInternal();
}

CIndSelSVParams::CIndSelSVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectSVList = NULL;
	_CleanInternal();
}

CIndSelSVParams::~CIndSelSVParams()
{
	_CleanInternal();
}

void CIndSelSVParams::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelSVParams::_CleanInternal()
{
	if( NULL != m_pclSelectSVList )
	{
		delete m_pclSelectSVList;
	}

	m_pclSelectSVList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for thermostatic regulative valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelTRVParams::CIndSelTRVParams() 
	: CIndividualSelectionParameters()
{
	m_pclSelectSupplyValveList = NULL;
	_CleanInternal();
}

CIndSelTRVParams::CIndSelTRVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, CPipeUserDatabase *pclPipeDB, 
		CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectSupplyValveList = NULL;
	_CleanInternal();
}

CIndSelTRVParams::~CIndSelTRVParams()
{
	_CleanInternal();
}

void CIndSelTRVParams::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelTRVParams::_CleanInternal()
{
	m_bIsTrvTypePreset = false;
	m_eValveType = RadiatorValveType::RVT_Undefined;
	m_eInsertType = RadiatorInsertType::RIT_Undefined;
	m_eReturnValveMode = RadiatorReturnValveMode::RRVM_Nothing;
	m_bIsFLCVInsert = false;
	m_bIsThermostaticHead = true;
	m_dInsertKvValue = 0.0;
	m_strSVTypeID = _T("");
	m_strComboSVFamilyID = _T("");
	m_strComboSVConnectID = _T("");
	m_strComboSVVersionID = _T("");
	m_strComboRVFamilyID = _T("");
	m_strComboRVConnectID = _T("");
	m_strComboRVVersionID = _T("");
	m_strComboSVInsertName = _T("");
	m_strComboSVInsertFamilyID = _T("");
	m_strComboSVInsertTypeID = _T("");
	m_strComboRVInsertFamilyID = _T("");
	m_strComboActuatorTypeID = _T("");
	m_strComboActuatorFamilyID = _T("");
	m_strComboActuatorVersionID = _T("");
	memset( &m_RadInfos, 0, sizeof( m_RadInfos ) );

	if( NULL != m_pclSelectSupplyValveList )
	{
		delete m_pclSelectSupplyValveList;
	}

	m_pclSelectSupplyValveList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for individual selection for safety valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIndSelSafetyValveParams::CIndSelSafetyValveParams() 
	: CIndividualSelectionParameters()
{
	m_pclSelectSafetyValveList = NULL;
	_CleanInternal();
}

CIndSelSafetyValveParams::CIndSelSafetyValveParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC )
	: CIndividualSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC )
{
	m_pclSelectSafetyValveList = NULL;
	_CleanInternal();
}

CIndSelSafetyValveParams::~CIndSelSafetyValveParams()
{
	_CleanInternal();
}

void CIndSelSafetyValveParams::Clean()
{
	CIndividualSelectionParameters::Clean();
	_CleanInternal();
}

void CIndSelSafetyValveParams::_CleanInternal()
{
	m_eSystemApplicationType = ProjectType::Heating;
	m_strSystemHeatGeneratorTypeID = _T( "" );
	m_dInstalledPower = 0.0;
	m_dInstalledCollector = 0.0;
	m_strNormID = _T( "" );
	m_strSafetyValveFamilyID = _T("");
	m_strSafetyValveConnectionID = _T("");
	m_dUserSetPressureChoice = 0.0;

	if( NULL != m_pclSelectSafetyValveList )
	{
		delete m_pclSelectSafetyValveList;
	}

	m_pclSelectSafetyValveList = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BATCH SELECTION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class parameters for batch selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CBatchSelectionParameters::CBatchSelectionParameters()
	:CProductSelelectionParameters()
{
	_CleanInternal();
}

CBatchSelectionParameters::CBatchSelectionParameters( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CProductSelelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

void CBatchSelectionParameters::CopyFrom( CProductSelelectionParameters *pclProductSelectionParameters )
{
	if( NULL == pclProductSelectionParameters )
	{
		return;
	}

	CProductSelelectionParameters::CopyFrom( pclProductSelectionParameters );

	if( NULL == dynamic_cast<CBatchSelectionParameters *>( pclProductSelectionParameters ) )
	{
		return;
	}

	CBatchSelectionParameters *pclBatchSelParamsSource = dynamic_cast<CBatchSelectionParameters *>( pclProductSelectionParameters );
	m_eFlowOrPowerDTMode = pclBatchSelParamsSource->m_eFlowOrPowerDTMode;
	m_strComboTypeBelow65ID = pclBatchSelParamsSource->m_strComboTypeBelow65ID;
	m_strComboFamilyBelow65ID = pclBatchSelParamsSource->m_strComboFamilyBelow65ID;
	m_strComboMaterialBelow65ID = pclBatchSelParamsSource->m_strComboMaterialBelow65ID;
	m_strComboConnectBelow65ID = pclBatchSelParamsSource->m_strComboConnectBelow65ID;
	m_strComboVersionBelow65ID = pclBatchSelParamsSource->m_strComboVersionBelow65ID;
	m_strComboPNBelow65ID = pclBatchSelParamsSource->m_strComboPNBelow65ID;
	m_strComboTypeAbove50ID = pclBatchSelParamsSource->m_strComboTypeAbove50ID;
	m_strComboFamilyAbove50ID = pclBatchSelParamsSource->m_strComboFamilyAbove50ID;
	m_strComboMaterialAbove50ID = pclBatchSelParamsSource->m_strComboMaterialAbove50ID;
	m_strComboConnectAbove50ID = pclBatchSelParamsSource->m_strComboConnectAbove50ID;
	m_strComboVersionAbove50ID = pclBatchSelParamsSource->m_strComboVersionAbove50ID;
	m_strComboPNAbove50ID = pclBatchSelParamsSource->m_strComboPNAbove50ID;
}

void CBatchSelectionParameters::Clean()
{
	CProductSelelectionParameters::Clean();
	_CleanInternal();
}

void CBatchSelectionParameters::_CleanInternal()
{
	m_eFlowOrPowerDTMode = CDS_SelProd::efdFlow;
	m_bIsDpGroupChecked = false;
	m_strComboTypeBelow65ID = _T("");
	m_strComboFamilyBelow65ID = _T("");
	m_strComboMaterialBelow65ID = _T("");
	m_strComboConnectBelow65ID = _T("");
	m_strComboVersionBelow65ID = _T("");
	m_strComboPNBelow65ID = _T("");
	m_strComboTypeAbove50ID = _T("");
	m_strComboFamilyAbove50ID = _T("");
	m_strComboMaterialAbove50ID = _T("");
	m_strComboConnectAbove50ID = _T("");
	m_strComboVersionAbove50ID = _T("");
	m_strComboPNAbove50ID = _T("");
	m_vecRowParameters.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class parameters for batch selection for balancing and control valves and pressure independent
// balancing and control valves.
CBatchSelCtrlParamsBase::CBatchSelCtrlParamsBase()
	: CBatchSelectionParameters()
{
	_CleanInternal();
}

CBatchSelCtrlParamsBase::CBatchSelCtrlParamsBase( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CBatchSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

void CBatchSelCtrlParamsBase::Clean()
{
	CBatchSelectionParameters::Clean();
	_CleanInternal();
}

void CBatchSelCtrlParamsBase::_CleanInternal()
{
	m_bIsCtrlTypeStrictChecked = false;
	m_eCV2W3W = CDB_ControlProperties::LastCV2W3W;
	m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvNU;
	m_bActuatorSelectedAsSet = false;
	m_strActuatorPowerSupplyID = _T("");
	m_strActuatorInputSignalID = _T("");
	m_iActuatorFailSafeFunction = 0;
	m_eActuatorDRPFunction = CDB_ControlValve::DRPFunction::drpfUndefined;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for batch selection for balancing & control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBatchSelBCVParams::CBatchSelBCVParams() 
	: CBatchSelCtrlParamsBase()
{
	_CleanInternal();
}

CBatchSelBCVParams::CBatchSelBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CBatchSelCtrlParamsBase( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

CBatchSelBCVParams::~CBatchSelBCVParams()
{
	_CleanInternal();
}

void CBatchSelBCVParams::Clean()
{
	CBatchSelectionParameters::Clean();
	_CleanInternal();
}

void CBatchSelBCVParams::_CleanInternal()
{
	// The 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods delete themself this pointer. 
	// Thus no need to do it here.
	m_pclBatchBCVList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for batch selection for balancing valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBatchSelBVParams::CBatchSelBVParams() 
	: CBatchSelectionParameters()
{
	m_pclBatchBVList = NULL;
	_CleanInternal();
}

CBatchSelBVParams::CBatchSelBVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CBatchSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

CBatchSelBVParams::~CBatchSelBVParams()
{
	_CleanInternal();
}

void CBatchSelBVParams::Clean()
{
	CBatchSelectionParameters::Clean();
	_CleanInternal();
}

void CBatchSelBVParams::_CleanInternal()
{
	// The 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods delete themself this pointer. 
	// Thus no need to do it here.
	m_pclBatchBVList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for batch selection for Dp controller valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBatchSelDpCParams::CBatchSelDpCParams() 
	: CBatchSelectionParameters()
{
	m_pclBatchDpCList = NULL;
	_CleanInternal();
}

CBatchSelDpCParams::CBatchSelDpCParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CBatchSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_clBatchSelBVParams.m_pTADB = pclTADB;
	m_clBatchSelBVParams.m_pTADS = pclTADS;
	m_clBatchSelBVParams.m_pUserDB = pclUserDB;
	m_clBatchSelBVParams.m_pPipeDB = pclPipeDB;
	m_clBatchSelBVParams.m_WC = *pclWC;
	m_clBatchSelBVParams.m_strPipeSeriesID = strPipeSeriesID;
	m_clBatchSelBVParams.m_strPipeID = strPipeID;

	m_pclBatchDpCList = NULL;

	_CleanInternal();
}

void CBatchSelDpCParams::Clean()
{
	CBatchSelectionParameters::Clean();
	_CleanInternal();
}

void CBatchSelDpCParams::_CleanInternal()
{
	m_bIsCheckboxDpBranchChecked = false;
	m_bIsCheckboxKvsChecked = false;
	m_bIsDpMaxChecked = false;
	m_dDpMax = 0.0;
	m_eDpStab = eDpStab::DpStabNone;
	m_eMvLoc = eMvLoc::MvLocNone;
	m_eDpCLoc = eDpCLoc::DpCLocNone;

	// The 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods delete themself this pointer. 
	// Thus no need to do it here.
	m_pclBatchDpCList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for batch selection for pressure independent balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBatchSelPIBCVParams::CBatchSelPIBCVParams() 
	: CBatchSelCtrlParamsBase()
{
	_CleanInternal();
}

CBatchSelPIBCVParams::CBatchSelPIBCVParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CBatchSelCtrlParamsBase( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

CBatchSelPIBCVParams::~CBatchSelPIBCVParams()
{
	_CleanInternal();
}

void CBatchSelPIBCVParams::Clean()
{
	CBatchSelCtrlParamsBase::Clean();
	_CleanInternal();
}
void CBatchSelPIBCVParams::_CleanInternal()
{
	// The 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods delete themself this pointer. 
	// Thus no need to do it here.
	m_pclBatchPIBCVList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for batch selection for air vents and separators.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBatchSelSeparatorParams::CBatchSelSeparatorParams() 
	: CBatchSelectionParameters()
{
	_CleanInternal();
}

CBatchSelSeparatorParams::CBatchSelSeparatorParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CBatchSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

CBatchSelSeparatorParams::~CBatchSelSeparatorParams()
{
	_CleanInternal();
}

void CBatchSelSeparatorParams::Clean()
{
	CBatchSelectionParameters::Clean();
	_CleanInternal();
}
void CBatchSelSeparatorParams::_CleanInternal()
{
	m_strComboTypeID = _T("");
	m_strComboFamilyID = _T("");
	m_strComboConnectID = _T("");
	m_strComboVersionID = _T("");

	// The 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods delete themself this pointer. 
	// Thus no need to do it here.
	m_pclBatchSeparatorList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for batch selection for smart control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBatchSelSmartControlValveParams::CBatchSelSmartControlValveParams() 
	: CBatchSelectionParameters()
{
	_CleanInternal();
}

CBatchSelSmartControlValveParams::CBatchSelSmartControlValveParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CBatchSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

CBatchSelSmartControlValveParams::~CBatchSelSmartControlValveParams()
{
	_CleanInternal();
}

void CBatchSelSmartControlValveParams::Clean()
{
	CBatchSelectionParameters::Clean();
	_CleanInternal();
}
void CBatchSelSmartControlValveParams::_CleanInternal()
{
	// The 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods delete themself this pointer. 
	// Thus no need to do it here.
	m_pclBatchSmartControlValveList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for batch selection for smart differential pressure controllers.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBatchSelSmartDpCParams::CBatchSelSmartDpCParams() 
	: CBatchSelectionParameters()
{
	_CleanInternal();
}

CBatchSelSmartDpCParams::CBatchSelSmartDpCParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CBatchSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

CBatchSelSmartDpCParams::~CBatchSelSmartDpCParams()
{
	_CleanInternal();
}

void CBatchSelSmartDpCParams::Clean()
{
	CBatchSelectionParameters::Clean();
	_CleanInternal();
}

void CBatchSelSmartDpCParams::_CleanInternal()
{
	m_bIsSelectedAsSet = false;
	m_bIsCheckboxDpBranchChecked = false;
	m_bIsDpMaxChecked = false;
	m_dDpMax = 0.0;

	// The 'CDlgBatchSelBase::OnBatchSelectionButtonClearResults' and the 'CDlgBatchSelBase::BS_ClearAllData' methods delete themself this pointer. 
	// Thus no need to do it here.
	m_pclBatchSmartDpCList = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WIZARD SELECTION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class parameters for wizard selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CWizardSelectionParameters::CWizardSelectionParameters() 
	: CProductSelelectionParameters()
{
	_CleanInternal();
}

CWizardSelectionParameters::CWizardSelectionParameters( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CProductSelelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	_CleanInternal();
}

CWizardSelectionParameters::~CWizardSelectionParameters()
{
	_CleanInternal();
}

void CWizardSelectionParameters::CopyFrom( CProductSelelectionParameters *pclProductSelectionParameters )
{
	if( NULL == dynamic_cast<CIndividualSelectionParameters *>( pclProductSelectionParameters ) )
	{
		return;
	}

	CWizardSelectionParameters *pclWizardSelParamsSource = dynamic_cast<CWizardSelectionParameters *>( pclProductSelectionParameters );
}

void CWizardSelectionParameters::Clean()
{
	CProductSelelectionParameters::Clean();
	_CleanInternal();
}

void CWizardSelectionParameters::_CleanInternal()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class parameters for wizard selection for pressurisation.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CWizardSelPMParams::CWizardSelPMParams() 
	: CWizardSelectionParameters()
{
	m_pclSelectPMList = NULL;
	_CleanInternal();
}

CWizardSelPMParams::CWizardSelPMParams( CTADatabase *pclTADB, CTADatastruct *pclTADS, CUserDatabase *pclUserDB, 
		CPipeUserDatabase *pclPipeDB, CWaterChar *pclWC, CString strPipeSeriesID , CString strPipeID )
	: CWizardSelectionParameters( pclTADB, pclTADS, pclUserDB, pclPipeDB, pclWC, strPipeSeriesID, strPipeID )
{
	m_pclSelectPMList = NULL;
	_CleanInternal();
}

CWizardSelPMParams::~CWizardSelPMParams()
{
	_CleanInternal();
}

void CWizardSelPMParams::Clean()
{
	CWizardSelectionParameters::Clean();
	_CleanInternal();
}

void CWizardSelPMParams::_CleanInternal()
{
	// This variable is a pointer on the global 'CSelectPMList' that is in the 'CPMSelectionHelper' class. No need to delete it!
	m_pclSelectPMList = NULL;
}
