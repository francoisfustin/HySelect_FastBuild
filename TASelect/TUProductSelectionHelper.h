#pragma once


#ifdef DEBUG

#include "TUProductSelectionHelperError.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to help unit testing on the product selection in the individual, batch, wizard and direct mode.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macro defines.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TU_RETURNERROR( ErrorCode, OutputString, OutputBox, ... ) { _Clean(); _PrepareError( ErrorCode, OutputString, __VA_ARGS__ ); _OutputMessage( OutputString, OutputBox ); return ErrorCode; }
#define TU_CHECKERROR( ErrorCode, OutputString, OutputBox, ... )  { if( TU_PRODSELECT_ERROR_OK != ErrorCode ) TU_RETURNERROR( ErrorCode, OutputString, OutputBox, __VA_ARGS__ ) }


#define TU_FILEHELPER_READLINE( Helper, Line )									{ UINT uiErrorCode = Helper.ReadOneLine( Line ); if( TU_PRODSELECT_ERROR_OK != uiErrorCode ) { return uiErrorCode; } }
#define TU_FILEHELPER_READLINEKEYVALUE( Helper, Line, Key, Value, Token )		{ UINT uiErrorCode = Helper.ReadOneLineKeyValue( Line, Key, Value, Token ); if( TU_PRODSELECT_ERROR_OK != uiErrorCode ) { return uiErrorCode; } }
#define TU_FILEHELPER_WRITELINE( Helper, Line )									{ UINT uiErrorCode = Helper.WriteOneLine( Line ); if( TU_PRODSELECT_ERROR_OK != uiErrorCode ) { return uiErrorCode; } }
#define TU_FILEHELPER_WRITELINE_ARGTEXT( Helper, Line, ArgText )				{ UINT uiErrorCode = Helper.WriteOneLineArgText( Line, ArgText ); if( TU_PRODSELECT_ERROR_OK != uiErrorCode ) { return uiErrorCode; } }
#define TU_FILEHELPER_WRITELINE_ARGFLOAT( Helper, Line, ArgFloat )				{ UINT uiErrorCode = Helper.WriteOneLineArgFloat( Line, ArgFloat ); if( TU_PRODSELECT_ERROR_OK != uiErrorCode ) { return uiErrorCode; } }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to ease access to a file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTUProdSelFileHelper
{
public:
	CTUProdSelFileHelper();
	virtual ~CTUProdSelFileHelper();

	bool IsFileExist( CString strFileName );
	UINT OpenFile( CString strFileName, CString strMode );
	void CloseFile();

	UINT ReadOneLine( CString &strLine, bool bForProdSelTest = true );
	UINT ReadOneLineKeyValue( CString &strLine, CString &strKey, CString &strValue, CString strToken, bool bLowerKey = true, bool bLowerValue = true );
	UINT ReadOneLineMultiValues( CString &strLine, std::vector<CString> &vecStrings, CString strToken, bool bForProdSelTest = true );
	
	UINT WriteOneLine( CString strLine, bool bForProdSelTest = true );
	UINT WriteOneLineArgText( CString strLine, CString strArg, bool bForProdSelTest = true );
	UINT WriteOneLineArgFloat( CString strLine, double fValue, bool bForProdSelTest = true );

	UINT SplitOneLineMultiValues( CString &strLine, std::vector<CString> &vecStrings, CString strToken, bool bForProdSelTest = true );
	int GetCurrentLineNumber() { return m_iLineNumber; }

// Public variables.
public:
	int m_iLineNumber;
	FILE *m_pfFile;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on product selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTUProdSelHelper
{
public:
	CTUProdSelHelper( CProductSelelectionParameters &clProductSelectionParams );
	virtual ~CTUProdSelHelper() {}

	virtual UINT SetpTADB( CTADatabase *pTADB );
	virtual UINT SetpTADS( CTADatastruct *pTADS );
	virtual UINT SetpUserDB( CUserDatabase *pUserDB );
	virtual UINT SetpPipeDB( CPipeUserDatabase *pPipeDB );

	// Allow to read the inputs from the file and to save each 'Key' and 'Value' pair in a map.
	virtual UINT ReadInputs( CTUProdSelFileHelper &clTUFileHelper );

	// Once the 'ReadInputs' has finished, we ask to all inherited classes to decode input values.
	virtual UINT InterpreteInputs();

	// Individual selection: After that each inherited class can verify input consistencies.
	// Batch selection: In this case each result has its own input values like flow or Power/DT. Thus here we 
	//                  will only verify general parameters as combos and checkboxes.
	virtual UINT VerifyInputs();

	// Now we can read all the selections corresponding to the input values.
	virtual UINT ReadOutputs( CTUProdSelFileHelper &clTUFileHelper, bool bUpdate = false );

	// And finally, we can launch a selection and compare it to the input file.
	// If 'bNoVerify' set to 'true' only the selection is done without verification.
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false ) { return TU_PRODSELECT_ERROR_OK; }

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

	virtual UINT GetOutputTitlesNbr( std::vector<CString> *pvecString ) { return (int)m_vecOutputTitles.size(); }

	virtual UINT WriteVecResultInFile( CTUProdSelFileHelper &clTUFileHelper );

	CProductSelelectionParameters *GetProductSelectionParameters() { return m_pclProdSelParams; }

// Protected variables.
protected:
	// List of all 'Key = 'Value' pairs that are the input values needed to do a selection.
	std::map<CString, CString> m_mapInputs;
	std::map<CString, CString> m_mapInputsWithCase;
	
	// This vector must be initialized by inherited class.
	// Contains title for all fields in one output line.
	std::vector<CString> m_vecOutputTitles;

	// Contains value for all fields in one output line.
	std::vector< std::vector<CString> > m_vecOutputValues;

	std::vector<CString> m_vecResultData;

// Private variables.
private:
	CProductSelelectionParameters *m_pclProdSelParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on individual selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelHelper : public CTUProdSelHelper
{
public:
	CTUIndSelHelper( CIndividualSelectionParameters &clIndSelParams );
	virtual ~CTUIndSelHelper() {}

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private variables.
private:
	CIndividualSelectionParameters *m_pclIndSelParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for regulating valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelBVHelper : public CTUIndSelHelper
{
public:
	CTUIndSelBVHelper();
	virtual ~CTUIndSelBVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );
	
// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelBVParams *pclIndSelBVParams, CSelectedValve *pclSelectedValve, int iResultCount );

// Private variables.
private:
	CIndSelBVParams m_clIndSelBVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on individual selection for control valves, balancing and control valves, 
// pressure independent balancing and control valves and combined Dp controller, balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelCtrlBaseHelper : public CTUIndSelHelper
{
public:
	CTUIndSelCtrlBaseHelper( CIndSelCtrlParamsBase &clIndSelCtrlBaseParams );
	virtual ~CTUIndSelCtrlBaseHelper() {}

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private variables.
private:
	CIndSelCtrlParamsBase *m_pclIndSelCtrlBaseParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelBCVHelper : public CTUIndSelCtrlBaseHelper
{
public:
	CTUIndSelBCVHelper();
	virtual ~CTUIndSelBCVHelper();

	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelBCVParams *pclIndSelBCVParams, CSelectedValve *pclSelectedValve, int iResultCount );

// Private variables.
private:
	CIndSelBCVParams m_clIndSelBCVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for pure control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelCVHelper : public CTUIndSelCtrlBaseHelper
{
public:
	CTUIndSelCVHelper();
	virtual ~CTUIndSelCVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelCVParams *pclIndSelCVParams, CSelectedValve *pclSelectedValve, int iResultCount );

// Private variables.
private:
	CIndSelCVParams m_clIndSelCVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for Dp controller valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelDpCHelper : public CTUIndSelHelper
{
public:
	CTUIndSelDpCHelper();
	virtual ~CTUIndSelDpCHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelDpCParams *pclIndSelDpCParams, CSelectedValve *pclSelectedValve, int iResultCount );

// Private variables.
private:
	CIndSelDpCParams m_clIndSelDpCParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for combined Dp controller, balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelDpCBCVHelper : public CTUIndSelCtrlBaseHelper
{
public:
	CTUIndSelDpCBCVHelper();
	virtual ~CTUIndSelDpCBCVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelDpCBCVParams *pclIndSelDpCBCVParams, CSelectedValve *pclSelectedValve, int iResultCount );

// Private variables.
private:
	CIndSelDpCBCVParams m_clIndSelDpCBCVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for pressure independent balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelPIBCVHelper : public CTUIndSelCtrlBaseHelper
{
public:
	CTUIndSelPIBCVHelper();
	virtual ~CTUIndSelPIBCVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelPIBCVParams *pclIndSelPIBCVParams, CSelectedValve *pclSelectedValve, int iResultCount );
	
// Private variables.
private:
	CIndSelPIBCVParams m_clIndSelPIBCVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for pressure maintenance.
// Remark: same remark as for the 'CIndSelPMParams' class in the 'ProductSelectionParameters.h' file. This class
//         doesn't need the variables that are in the 'CTUIndSelHelper'. This is why it is directly inherited
//         from the 'CTUProdSelHelper' class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelPMHelper : public CTUProdSelHelper
{
public:
	CTUIndSelPMHelper();
	virtual ~CTUIndSelPMHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT SetpTADB( CTADatabase *pTADB );
	virtual UINT SetpTADS( CTADatastruct *pTADS );

	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

	virtual UINT GetOutputTitlesNbr( std::vector<CString> *pvecString );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareComputedDataResultData( CIndSelPMParams *pclIndSelPMParams, int iResultCount );
	void _PrepareExpansionVesselResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedVssl *pclSelectedVessel, int iResultCount );
	void _PrepareCompressoResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedCompresso *pclSelectedCompresso, int iResultCount );
	void _PrepareTransferoResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedTransfero *pclSelectedTransfero, int iResultCount );
	void _PrepareVentoResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedVento *pclSelectedVento, int iResultCount );
	void _PreparePlenoResultData( CIndSelPMParams *pclIndSelPMParams, CSelectedPMBase *pclSelectedPleno, int iResultCount );

// Private variables.
private:
	CPMInputUser m_clPMInputUser;
	CIndSelPMParams m_clIndSelPMParams;
	std::map<int, CString > m_mapLayoutName;
	std::vector<CString> m_vecOutputComputedDataTitles;
	std::vector<CString> m_vecOutputExpansionVesselTitles;
	std::vector<CString> m_vecOutputCompressoTitles;
	std::vector<CString> m_vecOutputTransferoTitles;
	std::vector<CString> m_vecOutputVentoTitles;
	std::vector<CString> m_vecOutputPlenoTitles;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for separators and air vents.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelSeparatorHelper : public CTUIndSelHelper
{
public:
	CTUIndSelSeparatorHelper();
	virtual ~CTUIndSelSeparatorHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelSeparatorParams *pclIndSelSeparatorParams, CSelectedSeparator *pclSelectedSeparator, int iResultCount );

// Private variables.
private:
	CIndSelSeparatorParams m_clIndSelSeparatorParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for shutoff valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelSVHelper : public CTUIndSelHelper
{
public:
	CTUIndSelSVHelper();
	virtual ~CTUIndSelSVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelSVParams *pclIndSelSVParams, CSelectedValve *pclSelectedValve, int iResultCount );

// Private variables.
private:
	CIndSelSVParams m_clIndSelSVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for thermostatic regulative valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelTRVHelper : public CTUIndSelHelper
{
public:
	CTUIndSelTRVHelper();
	virtual ~CTUIndSelTRVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.	
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

	virtual UINT GetOutputTitlesNbr( std::vector<CString> *pvecString );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelTRVParams *pclIndSelTRVParams, CSelectedValve *pclSelectedValve, int iResultCount );
	void _PreprareSupplyValveResultData( CIndSelTRVParams *pclIndSelTRVParams, CSelectedValve *pclSelectedValve, int iResultCount );
	void _PreprareSupplyFlowLimitedResultData( CIndSelTRVParams *pclIndSelTRVParams, CSelectedValve *pclSelectedValve, int iResultCount );
	void _PreprareOtherInsertResultData( CIndSelTRVParams *pclIndSelTRVParams, int iResultCount );

// Private variables.
private:
	CIndSelTRVParams m_clIndSelTRVParams;
	std::vector<CString> m_vecOutputFlowLimitedTitles;
	std::vector<CString> m_vecOutputOtherInsertTitles;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for safety valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelSafetyValveHelper : public CTUProdSelHelper
{
public:
	CTUIndSelSafetyValveHelper();
	virtual ~CTUIndSelSafetyValveHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, CSelectedSafetyValve *pclSelectedSafetyValve, int iResultCount );

// Private variables.
private:
	CIndSelSafetyValveParams m_clIndSelSafetyValveParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HYS-1375 : Add unit test for TA-6-way valve
// Class to do test units on individual selection for TA-6-way control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSel6WayValveHelper : public CTUIndSelCtrlBaseHelper
{
public:
	CTUIndSel6WayValveHelper();
	virtual ~CTUIndSel6WayValveHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString& strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters* pclProdSelParams, CTUProdSelFileHelper& clTUFileHelper );
	virtual UINT ReadOutputs( CTUProdSelFileHelper& clTUFileHelper, bool bUpdate = false );

	// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSel6WayValveParams* pclIndSel6WayValveParams, CSelectedValve* pclSelectedValve, int iResultCount );
	UINT InterpretePIBCVInputs( CIndSelPIBCVParams* pclIndSelPIBCVParams );
	UINT VerifyPIBCVInputs( CIndSel6WayValveParams* pclIndSel6WayValveParams );
	void _PrepareResultDataPIBCV( CIndSel6WayValveParams* pclIndSel6wayValveParams, CSelectedValve* pclSelectedValve, CSelectedValve* pclSelected6WayValve, 
		int iResultCount, SideDefinition eSideDefinition );
	UINT DropOutSelectionPIBCV( CProductSelelectionParameters* pclProdSelParams, CTUProdSelFileHelper& clTUFileHelper );
	UINT InterpreteBvInputs( CIndSelBVParams* pclIndSelBVParams );
	UINT VerifyBVInputs( CIndSel6WayValveParams* pclIndSel6WayValveParams );
	void _PrepareResultDataBV( CIndSel6WayValveParams* pclIndSel6wayValveParams, CSelectedValve* pclSelectedValve, CSelectedValve* pclSelected6WayValve, 
		int iResultCount, SideDefinition eSideDefinition );
	UINT DropOutSelectionBV( CProductSelelectionParameters* pclProdSelParams, CTUProdSelFileHelper& clTUFileHelper );
	bool _VerifyFlows( CIndSel6WayValveParams* pclIndSel6wayValveParams, CSelectedValve* pSelected6WayValve );

	// Private variables.
private:
	CIndSel6WayValveParams m_clIndSel6WayValveParams;
	std::vector<CString> m_vecOutputSecondEQMTitles;
	std::vector<CString> m_vecOutputSecondOnoffPIBCVTitles;
	std::vector<CString> m_vecOutpuSecondOnoffBvTitles;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on individual selection for smart control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUIndSelSmartControlValveHelper : public CTUIndSelHelper
{
public:
	CTUIndSelSmartControlValveHelper();
	virtual ~CTUIndSelSmartControlValveHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );
	
// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CIndSelSmartControlValveParams *pclIndSelSmartControlValveParams, CSelectedValve *pclSelectedSmartControlValve, int iResultCount );

// Private variables.
private:
	CIndSelSmartControlValveParams m_clIndSelSmartControlValveParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on batch selection.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUBatchSelHelper : public CTUProdSelHelper
{
public:
	CTUBatchSelHelper( CBatchSelectionParameters &clBatchSelParams );
	virtual ~CTUBatchSelHelper() {}

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
    virtual UINT VerifyInputs();

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Protected variable.
protected:
	bool m_bVerifyAlsoCombos;

	// Indexes for variables that are at the beginning of the 'm_vecOutputValues'.
	int m_ivecOutputFlowIndex;
	int m_ivecOutputPowerIndex;
	int m_ivecOutputDTIndex;
	int m_ivecOutputDpIndex;
	int m_ivecOutputDpbranchIndex;
	int m_ivecOutputKvsIndex;

// Private variables.
private:
	CBatchSelectionParameters *m_pclBatchSelParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for regulating valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUBatchSelBVHelper : public CTUBatchSelHelper
{
public:
	CTUBatchSelBVHelper();
	virtual ~CTUBatchSelBVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );
	
// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CBatchSelBVParams *pclBatchSelBVParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters, int iResultCount );

// Private variables.
private:
	CBatchSelBVParams m_clBatchSelBVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to do test units on batch selection for balancing and control valves and pressure independent 
// balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUBatchSelCtrlBaseHelper : public CTUBatchSelHelper
{
public:
	CTUBatchSelCtrlBaseHelper( CBatchSelCtrlParamsBase &clBatchSelCtrlBaseParams );
	virtual ~CTUBatchSelCtrlBaseHelper() {}

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private variables.
private:
	CBatchSelCtrlParamsBase *m_pclBatchSelCtrlBaseParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUBatchSelBCVHelper : public CTUBatchSelCtrlBaseHelper
{
public:
	CTUBatchSelBCVHelper();
	virtual ~CTUBatchSelBCVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CBatchSelBCVParams *pclBatchSelBCVParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters, int iResultCount );

// Private variables.
private:
	CBatchSelBCVParams m_clBatchSelBCVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for Dp controllers.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUBatchSelDpCHelper : public CTUBatchSelHelper
{
public:
	CTUBatchSelDpCHelper();
	virtual ~CTUBatchSelDpCHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CBatchSelDpCParams *pclBatchSelDpCParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters, int iResultCount );

// Private variables.
private:
	CBatchSelDpCParams m_clBatchSelDpCParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for pressure independent balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUBatchSelPIBCVHelper : public CTUBatchSelCtrlBaseHelper
{
public:
	CTUBatchSelPIBCVHelper();
	virtual ~CTUBatchSelPIBCVHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CBatchSelPIBCVParams *pclBatchSelPIBCVParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters, int iResultCount );

// Private variables.
private:
	CBatchSelPIBCVParams m_clBatchSelPIBCVParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for separators and air vents.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUBatchSelSeparatorHelper : public CTUBatchSelHelper
{
public:
	CTUBatchSelSeparatorHelper();
	virtual ~CTUBatchSelSeparatorHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT InterpreteInputs();
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );

// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CBatchSelSeparatorParams *pclBatchSelSeparatorParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters, int iResultCount );

// Private variables.
private:
	CBatchSelSeparatorParams m_clBatchSelSeparatorParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to do test units on batch selection for smart control valves.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUBatchSelSmartControlValveHelper : public CTUBatchSelHelper
{
public:
	CTUBatchSelSmartControlValveHelper();
	virtual ~CTUBatchSelSmartControlValveHelper();

	// Overrides 'CTUProdSelHelper' public virtual methods.
	virtual UINT VerifyInputs();
	virtual UINT LaunchTest( CString &strErrorDetails, bool bNoVerify = false );

	virtual UINT DropOutSelection( CProductSelelectionParameters *pclProdSelParams, CTUProdSelFileHelper &clTUFileHelper );
	
// Private methods.
private:
	// Can be used by the 'LanchTest' and 'DropOutSelection' methods.
	void _PrepareResultData( CBatchSelSmartControlValveParams *pclBatchSelBVParams, CDlgBatchSelBase::BSRowParameters *pclRowParameters, int iResultCount );

// Private variables.
private:
	CBatchSelSmartControlValveParams m_clBatchSelSmartControlValveParams;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main class to launch all unit tests in regards in what's in the input file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTUProdSelLauncher
{
public:
	CTUProdSelLauncher();
	virtual ~CTUProdSelLauncher();

	void SetTADB( CTADatabase *pTADB ) { m_pTADB = pTADB; }
	void SetTADS( CTADatastruct *pTADS ) { m_pTADS = pTADS; }
	void SetUserDB( CUserDatabase *pUserDB ) { m_pUserDB = pUserDB; }
	void SetPipeDB( CPipeUserDatabase *pPipeDB ) { m_pPipeDB = pPipeDB; }

	UINT ExecuteTest( CString strInputFileName, CString &strOutputString, CListBox *pclOutputBox = NULL );
	UINT DropOutCurrentSelection( CProductSelelectionParameters *pclProdSelParams, CString strOutputFileName, CString &strOutputString, CListBox *pclOutputBox = NULL );
	UINT LoadTestAndDropResults( CString strInputFileName, CString strOutputFileName, CString &strOutputString, CListBox *pclOutputBox = NULL, bool bUpdate = false );

// private methods.
private:
	void _Clean( void );
	UINT _DropOutTechnicalParameters( CDS_TechnicalParameter *pclTechParameters );
	UINT _ReadTechnicalParameters();
	void _PrepareError( UINT uiErrorCode, CString &strErrorMessage, CString strErrorAddtionnalInfo = _T("") );
	void _OutputMessage( CString strMessage, CListBox *pclOutput );

// Private variables.
private:
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CUserDatabase *m_pUserDB;									// Database that contains object belonging to the user.
	CPipeUserDatabase *m_pPipeDB;								// Database that contains all pipes (default and user).
	CTUProdSelFileHelper m_clTUExecuteFileHelper;
	CTUProdSelFileHelper m_clTUDropOutFileHelper;
	CTUProdSelHelper *m_pclProdSelHelper;
	CTUProductSelectionHelperError m_clErrors;
	int m_iDropNumbers;
	CDS_TechnicalParameter *m_pclTechParamBackup;
	CWaterChar m_clWaterCharBackup;
	bool m_bWaterCharSaved;
};

#endif
