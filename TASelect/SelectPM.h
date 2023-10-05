#pragma once


#include "tchart.h"
#include "DlgPMTChartHelper.h"
#include <gdiplus.h>

/*
P0 Minimum pressure GetMinimumPressure()
Pa initial pressure ComputeInitialPressure(...) (take into account water reserve)
Pe Final Pressure GetFinalPressure()
*/

class CPMWQPrefs
{
public:
	typedef enum PMWQPreference
	{
		PMWQP_First = 0,
		GeneralCombinedInOneDevice = PMWQP_First,
		GeneralRedundancyPumpComp,
		GeneralRedundancyTecBox,
		CompressoInternalCoating,
		CompressoExternalAir,
		PumpDegassingCoolingVersion,
		WaterMakeupBreakTank,
		WaterMakeupDutyStandBy,
		PMWQP_Last
	};

	CPMWQPrefs()
	{
		Reset();
	}
	virtual ~CPMWQPrefs() {}

	void Reset( void );

	bool CompareTo( CPMWQPrefs *pclPMWQSelectionPreferences );
	void CopyFrom( CPMWQPrefs *pclPMWQSelectionPreferences );

	void SetChecked( PMWQPreference ePMWQPreference, bool bSet );
	void SetDisabled( PMWQPreference ePMWQPreference, bool bSet );
	
	void SetRedundancy( wchar_t *pID );
	CString GetRedundancy( void );
	bool IsRedundancyIDMatch( wchar_t *pID );

	bool IsChecked( PMWQPreference ePMWQPreference );
	bool IsDisabled( PMWQPreference ePMWQPreference );

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );
	void WriteToRegistry( CString strSectionName );
	void ReadFromRegistry( CString strSectionName );

private:
	struct PMWQPrefState
	{
		bool m_bChecked;
		bool m_bDisabled;
	};

	PMWQPrefState m_arPMWQPrefState[PMWQP_Last];
};

class CPMWQPrefsValidation
{
public:
	typedef struct _PMWQPrefArray
	{
		bool m_arPMWQPrefs[CPMWQPrefs::PMWQP_Last];
		bool m_bAtLeastOneExist;
	}PMWQPrefArray;

	// 'IV' is for 'Index in vector'.
	typedef enum IndexVector
	{
		IV_Undefined = -1,
		IV_First = 0,
		IV_Compresso = IV_First,
		IV_Transfero,
		IV_TransferoTI,
		IV_Vento,
		IV_Pleno,
		IV_Statico,
		IV_StaticoMembrane,
		IV_Last = IV_StaticoMembrane
	};
	
	// 'DWFCT' is for 'Degassing and Water make-up function'.
	enum
	{
		DWFCT_None = 0,
		DWFCT_Degassing = 1,
		DWFCT_WaterMakeUpStd = 2,
		DWFCT_WaterMakeUpSoftDesal = 4
	};

	typedef std::map<int, PMWQPrefArray> mapCombinedPreferences;
	typedef std::vector<mapCombinedPreferences> vecValidateOptions;

	typedef std::pair<CDB_TecBox::TecBoxType, PMWQPrefArray> pairTBTypePrefArray;
//	typedef std::pair<CDB_TecBox::TecBoxType, mapCombinedPreferences> pairTBTypeFctPref;

	CPMWQPrefsValidation();
	virtual ~CPMWQPrefsValidation() {}

	bool IsInitialized() { return m_bInitialiazed; }
	void Init( void );

	void CopyFrom( CPMWQPrefsValidation *pclPMWQPrefsValidation );
	vecValidateOptions *GetVecValidateOptions( void ) { return &m_vecValidateOptions; }

	bool IsAtLeastOneProductExist( IndexVector eIndexVector, int iFunction );

	bool IsAtLeastOneTecBoxHasMultiFct( CDB_TecBox::TecBoxType eTecBox );
	
	// Set the option for one PM product (Vessel or TecBox) and for a specific combination of functions.
	void SetValidateOptionFlag( CDB_Product *pclPMProduct, int iFunction, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet );
	
	// Set the option for one PM product (Vessel or TecBox) and for all available combinations.
	void SetValidateOptionFlag( CDB_Product *pclPMProduct, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet );

	// Set the option directly for an object in the vector list (see 'IndexVector' definitions above) and for a specific combination of functions.
	void SetValidateOptionFlag( IndexVector eIndexVector, int iFunction, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet );

	// Set the option directly for an object in the vector list (see 'IndexVector' definitions above) and for all available combinations.
	void SetValidateOptionFlag( IndexVector eIndexVector, CPMWQPrefs::PMWQPreference ePMWQPref, bool bSet );

	// It's a special method to allow to populate one preference from the no combination to all others where the 
	// 'GeneralCombinedInOneDevice' is false.
	// When we check (with the 'CDlgIndSelPressureMaintenance::_CheckDB' method) all the PM products to determine what are the 
	// preferences available for different combination (with degassing, with water male-up soft, with nothing, ... ) we need
	// to verify if some preferences are not always available for other combinations.
	// Example: 'External air' preference for Compresso. When '_CheckDB' is finished, we have this preference available only for
	//          the combination where it is selected without degassing and water make-up. But if user wants for example degassing
	//          and there is no CV combined, user must be able to choose a CX and an external Vento.
	void PopulateValidateOptionFlag( IndexVector eIndexVector, CPMWQPrefs::PMWQPreference ePMWQPref );

	// Get the option state for one type of TecBox and for a specific combination of functions.
	bool GetValidateOptionFlag( CDB_Product *pclPMProduct, int iFunctions, CPMWQPrefs::PMWQPreference ePMWQPref );
	bool GetValidateOptionFlag( IndexVector eIndexVector, int iFunctions, CPMWQPrefs::PMWQPreference ePMWQPref );

private:
	IndexVector _GetIndexVector( CDB_Product *pclPMProduct );

private:
	// Match tecbox type with the correspondent entry in the 'm_vecValidateOptions' vector.
	std::map<CDB_TecBox::TecBoxType, IndexVector > m_mapTecBoxEntries;
	std::map<CDB_Vessel::VsslType, IndexVector > m_mapVesselEntries;

	std::vector<int> m_vecFunctions;

	// These variables are not saved. There are filled in the 'CDlgIndSelPressureMaintenance::_CheckDB' during
	// the runtime.
	vecValidateOptions m_vecValidateOptions;
	bool m_bInitialiazed;
};

class CHeatGeneratorList
{
public:
	CHeatGeneratorList()
	{
		Reset();
	}
	virtual ~CHeatGeneratorList() {}

	typedef struct _HeatGeneratorSave
	{
		CString m_strHeatGeneratorTypeID;
		double m_dHeatCapacity;
		double m_dContent;
		bool m_bManual;
	} HeatGeneratorSave;
	typedef std::vector<HeatGeneratorSave> vecHeatGeneratorList;
	typedef vecHeatGeneratorList::iterator vecHeatGeneratorIter;

	void Reset()
	{
		m_dTotalHeatCapacity = 0.0;
		m_dTotalContent = 0.0;
		m_bHeatGeneratorProtect = false;
		m_vecHeatGeneratorList.clear();
	}

	bool CompareTo( CHeatGeneratorList *pclHeatGeneratorInput );
	void CopyFrom( CHeatGeneratorList *pclHeatGeneratorInput );

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );

	// If 'dContent' is -1.0 than the content is computed with the generator type and the heat capacity. Otherwise we directly save the value
	// passed as argument.
	bool AddHeatGenerator( CString strHeatGeneratorTypeID, double dHeatCapacity, double dContent = -1.0 );

	void SetHeatGeneratorProtect( bool bProtect ) { m_bHeatGeneratorProtect = bProtect; }

	// 'iIndex' is the 0-based index as it appears in the dialog where user introduces the heat generators.
	// Remark: if 'IIndex' is set to -1 we erase all entries.
	bool RemoveHeatGenerator( int iIndex );

	bool UpdateHeatGenerator( int iIndex, CString strHeatGeneratorTypeID, double dHeatCapacity, double dContent = -1.0 );

	int GetHeatGeneratorCount( void ) { return ( int )m_vecHeatGeneratorList.size(); }
	vecHeatGeneratorList *GetHeatGeneratorList( void ) { return &m_vecHeatGeneratorList; }

	HeatGeneratorSave *GetHeatGenerator( int iIndex );
	HeatGeneratorSave *GetFirstHeatGenerator( void );
	HeatGeneratorSave *GetNextHeatGenerator( void );

	double GetTotalHeatCapacity( void ) { return m_dTotalHeatCapacity; }
	double GetTotalContent( void ) { return m_dTotalContent; }
	bool GetHeatHeneratorProtect( void ) { return m_bHeatGeneratorProtect; }

	void SetSupplyTemperature( double dSupplyTemperature ) {}
	void SetReturnTemperature( double dSupplyTemperature ) {}

	// Private methods.
private:
	double _ComputeWaterContent( CString strHeatGeneratorTypeID, double dHeatCapacity );

	// Private variables.
private:
	double m_dTotalHeatCapacity;
	double m_dTotalContent;
	bool m_bHeatGeneratorProtect;
	vecHeatGeneratorList m_vecHeatGeneratorList;
	vecHeatGeneratorIter m_vecHeatGeneratorListIter;
};

class CColdGeneratorList
{
public:
	CColdGeneratorList()
	{
		Reset();
	}
	virtual ~CColdGeneratorList() {}

	typedef struct _ColdGeneratorSave
	{
		CString m_strColdGeneratorTypeID;
		double m_dColdCapacity;
		double m_dContent;
		double m_dSupplyTemp;
		bool m_bManual;
		bool m_bManualTemp;
		long m_lQuantity;
	} ColdGeneratorSave;
	typedef std::vector<ColdGeneratorSave> vecColdGeneratorList;
	typedef vecColdGeneratorList::iterator vecColdGeneratorIter;

	void Reset()
	{
		m_dTotalColdCapacity = 0.0;
		m_dTotalContent = 0.0;
		m_vecColdGeneratorList.clear();
	}

	bool CompareTo( CColdGeneratorList *pclColdGeneratorInput );
	void CopyFrom( CColdGeneratorList *pclColdGeneratorInput );

	void Write(OUTSTREAM outf);
	bool Read(INPSTREAM inpf);

	// If 'dContent' is -1.0 than the content is computed with the generator type and the heat capacity. Otherwise we directly save the value
	// passed as argument.
	bool AddColdGenerator( CString strColdGeneratorTypeID, double dColdCapacity, double dContent = -1.0, double dSupplyTemp = -1.0 );

	// 'iIndex' is the 0-based index as it appears in the dialog where user introduces the cold generators.
	// Remark: if 'IIndex' is set to -1 we erase all entries.
	bool RemoveColdGenerator( int iIndex );

	bool UpdateColdGenerator( int iIndex, CString strColdGeneratorTypeID, double dColdCapacity, double dContent = -1.0, double dSupplyTemp = -1.0, 
			bool bManualTemp = false, long lQuantity = 1 );

	int GetColdGeneratorCount( void ) { return (int)m_vecColdGeneratorList.size();	}
	vecColdGeneratorList *GetColdGeneratorList( void ) { return &m_vecColdGeneratorList; }

	ColdGeneratorSave *GetColdGenerator( int iIndex );
	ColdGeneratorSave *GetFirstColdGenerator( void );
	ColdGeneratorSave *GetNextColdGenerator( void );

	double GetTotalColdCapacity( void ) { return m_dTotalColdCapacity; }
	double GetTotalContent( void ) { return m_dTotalContent; }
	double GetTempOutdoor( void ) { return m_dTempOutdoor; }

	void SetSupplyTemperature( double dSupplyTemperature );
	void SetReturnTemperature( double dSupplyTemperature ) {}
	void SetTempOutdoor( double dTempOutdoor );
	
// Private methods.
private:
	double _ComputeWaterContent( CString strColdGeneratorTypeID, double dColdCapacity, double dSupplyTemp );

	// Private variables.
private:
	double m_dTotalColdCapacity;
	double m_dTotalContent;
	double m_dTempOutdoor;			// Outdoor temperature is the same as the maximum temperature.
	vecColdGeneratorList m_vecColdGeneratorList;
	vecColdGeneratorIter m_vecColdGeneratorListIter;
};

class CHeatConsumersList
{
public:
	CHeatConsumersList()
	{
		Reset();
	}
	virtual ~CHeatConsumersList() {}

	typedef struct _ConsumersSave
	{
		CString m_strConsumersTypeID;
		double m_dHeatCapacity;
		double m_dContent;
		bool m_bManualContent;
		double m_dSupplyTemp;
		double m_dReturnTemp;
		bool m_bManualTemp;
	} ConsumersSave;
	typedef std::vector<ConsumersSave> vecConsumersList;
	typedef vecConsumersList::iterator vecConsumersListIter;

	void Reset()
	{
		m_dTotalHeatCapacity = 0.0;
		m_dBufferContent = 0.0;
		m_vecConsumersList.clear();
	}

	bool CompareTo( CHeatConsumersList *pclConsumersList );
	void CopyFrom( CHeatConsumersList *pclConsumersList );

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );

	// If 'dContent' is -1.0 than the content is computed with the consumers type and the heat capacity. Otherwise we directly save the value
	// passed as argument.
	bool AddConsumers( CString strConsumersTypeID, double dHeatCapacity, double dContent, double dSupplyTemperature, double dReturnTemperature, bool bManualTemp );

	void SetBufferContent( double dBufferContent ) { m_dBufferContent = dBufferContent; }

	// 'iIndex' is the 0-based index as it appears in the dialog where user introduces the consumers.
	// Remark: if 'IIndex' is set to -1 we erase all entries.
	bool RemoveConsumers( int iIndex );

	bool UpdateConsumers( int iIndex, CString strConsumersTypeID, double dHeatCapacity, double dContent, double dSupplyTemperature, double dReturnTemperature,
			bool bManualTemp );

	int GetConsumersCount( void ) { return ( int )m_vecConsumersList.size(); }
	vecConsumersList *GetConsumersList( void ) { return &m_vecConsumersList; }

	ConsumersSave *GetConsumers( int iIndex );
	ConsumersSave *GetFirstConsumers( void );
	ConsumersSave *GetNextConsumers( void );

	double GetTotalHeatCapacity( void ) { return m_dTotalHeatCapacity; }
	double GetTotalContent( double dSupplyTemperature, double dReturnTemperature );
	double GetBufferContent( void ) { return m_dBufferContent; }

	void SetSupplyTemperature( double dSupplyTemperature );
	void SetReturnTemperature( double dReturnTemperature );

	// HYS-940: Verify that temperatures have correct values
	void VerifyConsHeatingTemperatureValues( double dSupplyTemperature, double dReturnTemperature, bool *pbSupplyTempOK, bool *pbReturnTempOK );

	// Private methods.
private:
	double _ComputeWaterContent( CString strConsumersTypeID, double dHeatCapacity, double dSupplyTemperature, double dReturnTemperature );

	// Private variables.
private:
	double m_dTotalHeatCapacity;
	double m_dBufferContent;
	vecConsumersList m_vecConsumersList;
	vecConsumersListIter m_vecConsumersListIter;
};

class CColdConsumersList
{
public:
	CColdConsumersList()
	{ 
		m_pTADS = NULL; 
		Reset();
	}

	virtual ~CColdConsumersList() {}

	void SetpTADS( CTADatastruct *pTADS ) { m_pTADS = pTADS; }

	typedef struct _ConsumersSave
	{
		CString m_strConsumersTypeID;
		double m_dColdCapacity;
		double m_dContent;
		bool m_bManualContent;
		double m_dSupplyTemp;
		double m_dReturnTemp;
		bool m_bManualTemp;
		double m_dRoomTemp;
		long m_lQuantity;
		double m_dSurface;
	} ConsumersSave;
	typedef std::vector<ConsumersSave> vecConsumersList;
	typedef vecConsumersList::iterator vecConsumersListIter;

	void Reset()
	{
		m_dTotalColdConsCapacity = 0.0;
		m_dBufferContent = 0.0;
		m_dMaxAirInputTempForAHU = -1.0;
		m_vecConsumersList.clear();
	}

	bool CompareTo( CColdConsumersList *pclConsumersList );
	void CopyFrom( CColdConsumersList *pclConsumersList );

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );

	// If 'dContent' is -1.0 than the content is computed with the consumers type and the heat capacity. Otherwise we directly save the value
	// passed as argument.
	bool AddConsumers( CString strConsumersTypeID, double dColdCapacity, double dContent, double dSupplyTemperature, double dReturnTemperature, 
			bool bManualTemp, double dRoomTemperature, long lQuantity, double dSurface );

	void SetBufferContent( double dBufferContent ) { m_dBufferContent = dBufferContent; }

	// 'iIndex' is the 0-based index as it appears in the dialog where user introduces the consumers.
	// Remark: if 'IIndex' is set to -1 we erase all entries.
	bool RemoveConsumers( int iIndex );

	bool UpdateConsumers( int iIndex, CString strConsumersTypeID, double dColdCapacity,  double dContent,  double dSupplyTemperature, 
			double dReturnTemperature, bool bManualTemp, double dRoomTemperature, long lQuantity, double dSurface );

	int GetConsumersCount( void ) { return (int)m_vecConsumersList.size(); }
	vecConsumersList *GetConsumersList( void ) { return &m_vecConsumersList; }

	ConsumersSave *GetConsumer( int iIndex );
	ConsumersSave *GetFirstConsumers( void );
	ConsumersSave *GetNextConsumers( void );

	CTADatastruct *GetpTADS() { return m_pTADS; }
	double GetTotalColdCapacity( void ) { return m_dTotalColdConsCapacity; }
	double GetTotalContent( double dSupplyTemperature, double dReturnTemperature );
	double GetMaxAirInputTempForAHU( double ) { return m_dMaxAirInputTempForAHU; }
	double GetBufferContent( void )	{ return m_dBufferContent; }

	double GetMaxAirInputTempForAHU( void ) { return m_dMaxAirInputTempForAHU; }
	
	void SetSupplyTemperature( double dSupplyTemperature );
	void SetReturnTemperature( double dReturnTemperature );
	void SetMaxAirInputTempForAHU( double dMaxAirInputTempForAHU );

	// HYS-940: Verify that temperatures have correct values.
	void VerifyConsCoolingTemperatureValues( CString strConsumersTypeID, double dSupplyTemperature, double dReturnTemperature,
			double dRoomTemperature, bool *pbSupplyTempOK, bool *pbReturnTempOK, bool *pbRoomTempOK);

// Private methods.
private:
	double _ComputeWaterContent( CString strConsumersTypeID, double dColdCapacity,  double dSupplyTemperature, double dReturnTemperature, 
			double dRoomTemperature, long lQuantity, double dSurface );

// Private variables.
private:
	CTADatastruct *m_pTADS;

	double m_dTotalColdConsCapacity;
	double m_dBufferContent;

	// HYS-958: 'm_dTempOutdoor' becomes 'm_dMaxAirInputTempForAHU'.
	double m_dMaxAirInputTempForAHU;
	vecConsumersList m_vecConsumersList;
	vecConsumersListIter m_vecConsumersListIter;
};

class CPipeList
{
public:
	CPipeList()
	{
		Reset();
	}
	virtual ~CPipeList() {}

	typedef struct _PipeSave
	{
		CString m_strPipeSerieID;
		CString m_strPipeSizeID;
		double m_dLength;
		double m_dContent;
	} PipeSave;
	typedef std::vector<PipeSave> vecPipeList;
	typedef vecPipeList::iterator vecPipeListIter;

	void Reset()
	{
		m_dTotalContent = 0.0;
		m_vecPipeList.clear();
	}

	bool CompareTo( CPipeList *pclPipeList );
	void CopyFrom( CPipeList *pclPipeList );

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );

	bool AddPipe( CString strPipeSeriesID, CString strPipeSizeID, double dLength );

	// 'iIndex' is the 0-based index as it appears in the dialog where user introduces the pipes.
	// Remark: if 'IIndex' is set to -1 we erase all entries.
	bool RemovePipe( int iIndex );

	bool UpdatePipe( int iIndex, CString strPipeSerieID, CString strPipeSizeID, double dLength );

	int GetPipeCount( void ) { return ( int )m_vecPipeList.size(); }
	vecPipeList *GetPipeList( void ) { return &m_vecPipeList; }

	PipeSave *GetPipe( int iIndex );
	PipeSave *GetFirstPipe( void );
	PipeSave *GetNextPipe( void );

	double GetTotalContent( void ) { return m_dTotalContent; }

	// For now the pipe added by the user are not saved in the project
	bool PipeExistInDB();

// Private methods.
private:
	double _ComputeWaterContent( CString strPipeID, double dLength );

// Private variables.
private:
	double m_dTotalContent;
	vecPipeList m_vecPipeList;
	vecPipeListIter m_vecPipeListIter;
};

class CSelectedVssl;
class CPMInputUser
{
public:
	enum eRetCodePressMaint
	{
		ePM_InputEmpty,
		ePM_InvalidPSV,
		ePM_Last
	};
	
	enum eRetAddCodeInputEmpty
	{
		RACIE_First,
		RACIE_WaterContent,
		RACIE_SolarContent,
		RACIE_InstalledPower,
		RACIE_PumpHead,
		RACIE_WaterMakeUpPSN,					// Static pressure of water network.
		RACIE_Last = RACIE_WaterMakeUpPSN
	};

	enum MaintenanceType
	{
		MT_All,
		MT_ExpansionVessel,
		MT_ExpansionVesselWithMembrane,
		MT_WithCompressor,
		MT_WithPump,
		MT_None,
		MT_Unknown
	};

	CPMInputUser();
	virtual ~CPMInputUser() {}

	// Returns 'true' if the both 'CPMInputUser' are the same.
	bool CompareTo( CPMInputUser *pclInputUser );

	void CopyFrom( CPMInputUser *pclInputUser );
	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );

	// HYS-1022: 'pa' becomes 'pa,min' = p0 + 0.3.
	double GetMinimumInitialPressure();
	double ComputeInitialPressure( double dTotalVesselVolume, double dWaterReserve );

	// This method returns the water reserve optimized considering the current total volume available with vessel.
	double GetWaterReserve( double dTotalVesselVolume );

	// This method returns the water reverse pushed in vessel at a given pressure.
	double GetWaterReservePushed( double dTotalVesselVolume, double dPressure );

	// Allow to compute the pressure at a temperature.
	double GetIntermediatePressure( double dIntermediateTemperature, double dWaterReserve, double dTotalVesselVolume );
	
	// Allow to retrieve what is the lower pressure that a system will reach in the cooling mode.
	double GetLowestPressure( double dWaterReserve, double dTotalVesselVolume );

	// Only for cooling system. Return 0.0 in other cases.
	double ComputeContractionVolume( void );

	double GetContractionCoefficient( void );

	bool UsePressureVolumeLimit();
	bool CheckInputData( std::vector<std::pair<int, int>> *pveciRetCode );

	//////////////////////////////////////////////////////
	// Getter
	CTADatabase *GetpTADB() { return m_pTADB; }
	CTADatastruct *GetpTADS() { return m_pTADS; }

	CWaterChar *GetpWC() { return &m_WC; }
	ProjectType GetApplicationType() const { return m_eApplicationType; }

	MaintenanceType GetPressureMaintenanceType();
	CString GetPressureMaintenanceTypeID() const { return m_strPressureMaintenanceTypeID; }
	bool IsPressurisationSystemExist() { return ( MaintenanceType::MT_None == GetPressureMaintenanceType() ) ? false : true;	}
	
	CString GetWaterMakeUpTypeID() const { return m_strWaterMakeUpTypeID; }
	bool IsWaterMakeUpExist() { return ( 0 == m_strWaterMakeUpTypeID.Compare( _T("WMUP_TYPE_NONE") ) ) ? false : true;	}
	
	int GetDegassingChecked() const { return m_iDegassingChecked; }
	double GetDegassingWaterReserve() const;
	
	CString GetNormID() const { return m_strNormID; }
	PressurisationNorm GetNorm() const;
	bool IsNorm( ProjectType eProjectType, PressurisationNorm ePMNorm );

	double GetStaticHeight() const { return m_dStaticHeight; }
	int GetPzChecked() const { return m_iPzChecked; }
	double GetPz() const { return m_dPz; }
	double GetSystemVolume();
	double GetSolarCollectorVolume() const { return m_dSolarCollectorVolume; }
	int GetSolarCollectorMultiplierFactorChecked() const { return m_iSolarCollectorMultiplierFactorChecked; }
	double GetSolarCollectorMultiplierFactor();
	double GetStorageTankVolume() const { return m_dStorageTankVolume; }				// HYS-1534: Common to heating, cooling and solar.
	double GetStorageTankMaxTemp() const { return m_dStorageTankMaxTemp; }				// HYS-1534: Common to heating, cooling and solar.
	double GetInstalledPower();
	double GetSafetyValveResponsePressure() const { return m_dSafetyPressValve; }

	// HYS-1083.
	int GetSafetyValveLocationChecked() const { return m_iSafetyValveLocationChecked; }
	double GetSafetyValveLocation() const { return m_dSafetyValveLocation; }
	
	double GetSafetyTempLimiter() const { return m_dSafetyTempLimiter; }
	double GetMaxTemperature() const { return m_dMaxTemperature; }
	double GetSupplyTemperature() const { return m_dSupplyTemperature; }
	double GetReturnTemperature() const { return m_dReturnTemperature; }
	double GetMinTemperature() const { return m_dMinTemperature; }
	bool GetCheckMinTemperature() const { return m_fCheckMinTemperature; }
	double GetFillTemperature() const { return m_dFillTemperature; }
	PressurON GetPressOn() const { return m_ePressOn; }
	double GetPumpHead() const { return m_dPumpHead; }
	double GetMaxWidth() const { return m_dMaxWidth; }
	double GetMaxHeight() const { return m_dMaxHeight; }
	double GetDegassingMaxTempConnectPoint() const { return m_dDegassingMaxTempConnectPoint; }
	double GetDegassingPressureConnectPoint() const { return m_dDegassingPressureConnectPoint; }
	double GetWaterMakeUpNetworkPN() const { return m_dWaterMakeUpNetworkPN; }
	double GetWaterMakeUpWaterTemp() const { return m_dWaterMakeUpWaterTemp; }
	double GetWaterMakeUpWaterHardness() const { return m_dWaterMakeUpWaterHardness; }
	double GetXFactorSWKI();
	double GetSystemExpansionCoefficient();
	double GetSystemExpansionVolume();

	// HYS-1534: add a method to get the solar collector security volume.
	double GetSolarCollectorSecurityVolume();

	// HYS-1534: SWKI HE301-01.
	double GetStorageTankExpansionCoefficient();
	double GetStorageTankExpansionVolume();

	// For all norms except SWKI HE301-01, this will return the same as the 'GetSystemExpansionVolume' method.
	// For SWKI HE301-01 norm, we add the expansion of the storage tank content if exist.
	double GetTotalExpansionVolume();

	// HYS-1565: Vessel net volume is the total expansion volume + VDK (if solar) + Vv (if degassing) + Vwr,min (If not SWKI HE301-01 norm).
	double GetVesselNetVolume();

	// HYS-1022: 'VWr' becomes 'Vwr,min'.
	double GetMinimumWaterReserve();

	// This method returns the vessel volume needed for the expansion and the minimum water reserve.
	// (+ eventually the contraction volume and the reserve for the degassing device).
	double GetVesselNominalVolume( bool bForTecBox = false );

	// Allow to retrieve for a specific vessel volume what is the required nominal volume with
	// the pressure = p0 + 0.3.
	double GetNominalVolumeForPaEN12828( double dTotalVesselVolume );

	double GetMinimumPressure( bool bVerifyPz = true );
	double GetVaporPressure();
	double GetFinalPressure();
	double GetPressureFactor( bool bForTecBox = false );
	double GetNominalVolumeInterVssl( double dVesselTempMin, double dVesselTempMax );

	CPMWQPrefs *GetPMWQSelectionPreferences( void ) { return &m_clPMWQSelectionPreferences; }
	
	CHeatGeneratorList *GetHeatGeneratorList( void ) { return &m_clHeatGeneratorList; }
	CColdGeneratorList *GetColdGeneratorList( void ) { return &m_clColdGeneratorList; }
	CHeatConsumersList *GetHeatConsumersList( void ) { return &m_clHeatConsumersList; }
	CColdConsumersList *GetColdConsumersList( void ) { return &m_clColdConsumersList; }

	// This method will return 'm_clHeatingPipeList' or 'm_clCoolingPipeList' in regards to the current application
	// type set in this class (m_eApplicationType).
	CPipeList *GetPipeList( void );

	// It's just an help for the 'CopyFrom' method.
	CPipeList *GetHeatingPipeList( void ) { return &m_clHeatingPipeList; }
	CPipeList *GetCoolingPipeList( void ) { return &m_clCoolingPipeList; }

	double GetTotalHeatWaterContent( void );
	double GetTotalHeatInstalledPower( void );
	double GetTotalColdWaterContent( void );
	double GetTotalColdInstalledPower( void );

	// Check PSV (from 'Pflichtenheft_offline_Algorithmus_10620.xlsx' documentation).
	bool CheckPSV( double *pdPSVLimit, double dCurrentPSV = -1.0 );

	// For TecBox
	// Based on equation from planning and calculation p9 and xls sheet from ESC
	// Remark: this method returns the value that is rounded in regards to the current unit.
	double GetMinimumRequiredPSVRounded( MaintenanceType eMaintenanceType );

	// Same as above but not rounded.
	double GetMinimumRequiredPSVRaw( MaintenanceType eMaintenanceType );

	double GetEqualizingVolumetricFlow();
	double GetVD();
	double GetTargetPressureForTecBox( CDB_TecBox::TecBoxType TbT );
	double GetFinalPressure( CDB_TecBox::TecBoxType TbT );

	// Allow to get max weight (weight of vessel filled up with fluid at minimum temperature).
	double GetMaxWeight( double dVesselWeight, double dVesselVolume, double dMinTemperature );

	// Allow to get the maximum admissible hardness of the installation (VDI 2035 norm).
	// ( See the 'CDB_TotalHardnessCharacteristic' class in the 'DataObj.h' file for more explanation).
	double GetTotalHardnessOfSystem( void );

	//////////////////////////////////////////////////////
	// Setter
	void SetpTADB( CTADatabase *pTADB ) { m_pTADB = pTADB; }
	void SetpTADS( CTADatastruct *pTADS );
	
	void SetWC( CWaterChar val ) { m_WC = val; }
	void SetApplicationType( ProjectType val ) { m_eApplicationType = val; }
	void SetPressureMaintenanceTypeID( CString val ) { m_strPressureMaintenanceTypeID = val; }
	void SetWaterMakeUpTypeID( CString val ) { m_strWaterMakeUpTypeID = val; }
	void SetDegassingChecked( int val ) { m_iDegassingChecked = val; }
	void SetNormID( CString val ) { m_strNormID = val; }
	void SetPzChecked( int val ) { m_iPzChecked = val; }
	void SetPz( double val ) { m_dPz = val; }
	void SetStaticHeight( double val ) { m_dStaticHeight = val; }
	void SetSystemVolume( double val ) { m_dSystemVolume = val; }
	void SetSolarCollectorVolume( double val ) { m_dSolarCollectorVolume = val; }
	void SetSolarCollectorMultiplierFactorChecked( int iChecked ) { m_iSolarCollectorMultiplierFactorChecked = iChecked; }
	void SetSolarCollectorMultiplierFactor( double val ) { m_dSolarCollectorMultiplierFactor = val; }
	void SetStorageTankVolume( double val ) { m_dStorageTankVolume = val; }			// HYS-1534: Common to heating, cooling and solar in SWKI HE301-01 norm.
	void SetStorageTankMaxTemp( double val ) { m_dStorageTankMaxTemp = val; }		// HYS-1534: Common to heating, cooling and solar in SWKI HE301-01 norm.
	void SetInstalledPower( double val ) { m_dInstalledPower = val; }
	void SetSafetyValveResponsePressure( double val ) { m_dSafetyPressValve = val; }
	
	// HYS-1083.
	void SetSafetyValveLocationChecked( int val ) { m_iSafetyValveLocationChecked = val; }
	void SetSafetyValveLocation( double val ) { m_dSafetyValveLocation = val;  }

	void SetSafetyTempLimiter( double val ) { m_dSafetyTempLimiter = val; }
	void SetSupplyTemperature( double dSupplyTemperature );
	void SetReturnTemperature( double dReturnTemperature );
	void SetMaxTemperature( double dMaxTemperature );
	void SetMinTemperature( double val ) { m_dMinTemperature = val; }
	void SetCheckMinTemperature( bool fCheckMinTemperature ) { m_fCheckMinTemperature = fCheckMinTemperature; }
	void SetFillTemperature( double val ) { m_dFillTemperature = val; }
	void SetPressOn( PressurON val ) { m_ePressOn = val; }
	void SetPumpHead( double val ) { m_dPumpHead = val; }
	void SetMaxWidth( double val ) { m_dMaxWidth = val; }
	void SetMaxHeight( double val ) { m_dMaxHeight = val; }
	void SetDegassingMaxTempConnectPoint( double dValue ) { m_dDegassingMaxTempConnectPoint = dValue; }
	void SetDegassingPressureConnectPoint( double dValue ) { m_dDegassingPressureConnectPoint = dValue; }
	void SetWaterMakeUpNetworkPN( double dValue ) { m_dWaterMakeUpNetworkPN = dValue; }
	void SetWaterMakeUpWaterTemp( double dValue ) { m_dWaterMakeUpWaterTemp = dValue; }
	void SetWaterMakeUpWaterHardness( double dValue ) { m_dWaterMakeUpWaterHardness = dValue; }

	void SetPMWQSelectionPreferences( CPMWQPrefs *pclPMWQSelectionPreferences );

	// Return true if user defined system volume by entering Production/Pipe/Consumer volume
	bool IfSysVolExtDefExist();
	
	// Return true if user defined Power by entering Production/Pipe/Consumer power
	bool IfInstPowExtDefExist();

	// HYS-1083: Centralize the expansion pipe here.
	IDPTR GetExpansionPipeSizeIDPtr( CDB_Product *pclProduct, double dPipelength );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// These methods are to help us to switch between normal and advanced mode for the heat and cold generators/consumers.
	// All the variables are not saved in .tsp file.
	double GetSystemVolumeBackup( void ) { return m_dSystemVolumeBackup; }
	double GetInstalledPowerBackup( void ) { return m_dInstalledPowerBackup; }
	void SetSystemVolumeBackup( double dSystemVolume ) { m_dSystemVolumeBackup = dSystemVolume; }
	void SetInstalledPowerBackup( double dInstalledPower ) { m_dInstalledPowerBackup = dInstalledPower; }

	CHeatGeneratorList *GetpHeatGeneratorListBackup( void ) { return &m_clHeatGeneratorListBackup; }
	CHeatConsumersList *GetpHeatConsumersListBackup( void ) { return &m_clHeatConsumersListBackup; }
	CColdGeneratorList *GetpColdGeneratorListBackup( void ) { return &m_clColdGeneratorListBackup; }
	CColdConsumersList *GetpColdConsumersListBackup( void ) { return &m_clColdConsumersListBackup; }
	CPipeList *GetpPipeListBackup( void ) { return &m_clPipeListBackup; }
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Working variables.
	bool GetSolarContentMultiplierFactorWarningDisplayed( void ) { return m_bSolarContentMultiplierFactorWarningDisplayed; }
	void SetSolarContentMultiplierFactorWarningDisplayed( bool bValue ) { m_bSolarContentMultiplierFactorWarningDisplayed = bValue; }

private:
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;

	// Input Data
	CWaterChar m_WC;
	ProjectType m_eApplicationType;								// Common to heating, cooling and solar.
	CString m_strPressureMaintenanceTypeID;						// Common to heating, cooling and solar.
	CString m_strWaterMakeUpTypeID;								// Common to heating, cooling and solar.
	int m_iDegassingChecked;									// Common to heating, cooling and solar.
	CString m_strNormID;										// Common to heating, cooling and solar.
	int m_iPzChecked;											// Common to heating, cooling and solar.
	double m_dPz;												// Common to heating, cooling and solar.
	double m_dStaticHeight;										// Common to heating, cooling and solar.
	double m_dSystemVolume;										// Common to heating, cooling and solar.
	double m_dSolarCollectorVolume;								// For solar (Vk).
	int m_iSolarCollectorMultiplierFactorChecked;				// For solar and SWKI HE301-01 norm.
	double m_dSolarCollectorMultiplierFactor;					// For solar and SWKI HE301-01 norm.
	double m_dStorageTankVolume;								// HYS-1534: Common to heating, cooling and solar.
	double m_dStorageTankMaxTemp;								// HYS-1534: Common to heating, cooling and solar.
	double m_dInstalledPower;									// Common to heating, cooling and solar.
	double m_dSafetyPressValve;									// Common to heating, cooling and solar.
	int m_iSafetyValveLocationChecked;									// HYS-1083. Common to heating, cooling and solar.
	double m_dSafetyValveLocation;										// HYS-1083. Common to heating, cooling and solar.
	double m_dSafetyTempLimiter;								// Common to heating and solar.
	double m_dMaxTemperature;									// For cooling.
	double m_dSupplyTemperature;								// For Heating/Solar.
	double m_dReturnTemperature;								// For Heating/Solar.
	double m_dMinTemperature;									//
	bool m_fCheckMinTemperature;								// For Cooling (true if user entered a min temperature, false if it's the same as supply).
	double m_dFillTemperature;									// Only for cooling.
	PressurON m_ePressOn;										// Common to heating, cooling and solar.
	double m_dPumpHead;											// Common to heating, cooling and solar.
	double m_dMaxWidth;											// Common to heating, cooling and solar.
	double m_dMaxHeight;										// Common to heating, cooling and solar.
	double m_dDegassingMaxTempConnectPoint;
	double m_dDegassingPressureConnectPoint;
	double m_dWaterMakeUpNetworkPN;
	double m_dWaterMakeUpWaterTemp;
	double m_dWaterMakeUpWaterHardness;

	// Allow to keep a link between pressure maintenance type ID and pressure maintenance enum.
	std::map<CString, MaintenanceType> m_mapPMTypeMatchIDEnum;

	// Class for pressurization and water quality selection preferences.
	CPMWQPrefs m_clPMWQSelectionPreferences;

	// Variables for system volume definitions.
	CHeatGeneratorList m_clHeatGeneratorList;
	CColdGeneratorList m_clColdGeneratorList;
	CHeatConsumersList m_clHeatConsumersList;
	CColdConsumersList m_clColdConsumersList;
	CPipeList m_clHeatingPipeList;
	CPipeList m_clCoolingPipeList;

	// Backup variables (not saved in tsp file).
	double m_dSystemVolumeBackup;
	double m_dInstalledPowerBackup;
	CHeatGeneratorList m_clHeatGeneratorListBackup;
	CHeatConsumersList m_clHeatConsumersListBackup;
	CColdGeneratorList m_clColdGeneratorListBackup;
	CColdConsumersList m_clColdConsumersListBackup;
	CPipeList m_clPipeListBackup;

	// Variables not saved, it's just for working in HySelect!
	bool m_bSolarContentMultiplierFactorWarningDisplayed;
};

class CSelectedPMBase
{
public:
	typedef enum SelectedType
	{
		ST_Undefined,
		ST_Vessel,
		ST_Transfero,
		ST_Compresso,
		ST_Pleno,
		ST_PlenoRefill,
		ST_Vento,
		ST_WaterTreatmentCombination,
	};

	CSelectedPMBase( SelectedType eSelectedType );
	virtual ~CSelectedPMBase() {}

	typedef enum Flags
	{
		eCommonFlagFirst =			0x00000001,
		eBest =						eCommonFlagFirst,				// Best technical choice.
		eTemperature =				( eBest << 1 ),					// Temperature validity flag.
		ePN =						( eTemperature << 1 ),			// PN (Static pressure) validity flag.
		eNotPriority =				( ePN << 1 ),					// This selection is not a priority.
		eCommonFlagLast =			( eNotPriority ),

		eVslFlagsFirst =			( eCommonFlagLast << 1 ),
		eVslFlagsNbreOfVssl =		( eVslFlagsFirst << 1 ),
		eVslFlags3000lbar =			( eVslFlagsNbreOfVssl << 1 ),
		eVslFlagsLast =				( eVslFlags3000lbar ),

		eCprssoFlagsFirst =			( eVslFlagsLast << 1 ),
		eCprssoFlagsLast =			( eCprssoFlagsFirst ),

		eTrfroFlagsFirst =			( eCprssoFlagsLast << 1 ),
		eTrfroFlagsLast =			( eTrfroFlagsFirst ),

		ePlenoFlagsFirst =			( eTrfroFlagsLast << 1 ),
		ePlenoWMUpMaxTemp =			( ePlenoFlagsFirst << 1 ),
		ePlenoWMUpMinTemp =			( ePlenoWMUpMaxTemp << 1 ),
		ePlenoFlagsLast =			( ePlenoWMUpMinTemp ),

		ePRefillFlagsFirst =		( ePlenoFlagsLast << 1 ),
		ePRefillWMUpMaxTemp =		( ePRefillFlagsFirst << 1 ),
		ePRefillWMUpMinTemp =		( ePRefillWMUpMaxTemp << 1 ),
		ePRefillFlagsLast =			( ePRefillWMUpMinTemp ),

		eVentoFlagsFirst =			( ePRefillFlagsLast << 1 ),
		eVentoMaxTempConnectPoint =	( eVentoFlagsFirst << 1 ),
		eVentoMinTemp =				( eVentoMaxTempConnectPoint << 1 ),
		eVentoMaxParallel =			( eVentoMinTemp << 1 ),
		eVentoFlagsLast =			( eVentoMaxParallel ),

		efAll =					0xFFFFFFFF
	};

	enum TempErrorCode
	{
		TEC_FirstError						= 1,
		// Return temperature of the system is greater than the max temperature of the TecBox (Heating/Solar).
		TEC_SystemReturnTempBiggerTBMaxTemp = TEC_FirstError,
		// Min. temperature that can be reached when the system is out of operation\r\nis lower or equal to the min. temperature (%1).
		TEC_SystemMinTempLowerTBMinTemp		= ( TEC_SystemReturnTempBiggerTBMaxTemp << 1 ),
		TEC_LastError = TEC_SystemMinTempLowerTBMinTemp
	};

	enum RejectedReasonsFlag
	{
		RRF_AlwaysDisplay,			// This error must be always displayed.
		RRF_DisplayOnlyWhenAlone	// This error must be displayed only if there is only this one.
	};

	struct RejectedReasons
	{
		RejectedReasons()
		{
			m_iIDError = -1;
			m_eFlag = RRF_AlwaysDisplay;
			m_strErrorMsg = _T("");
		}

		bool operator==( const RejectedReasons &RejectedReasonsRight )
		{
			if( m_iIDError == RejectedReasonsRight.m_iIDError && m_eFlag == m_eFlag && 0 == m_strErrorMsg.Compare( RejectedReasonsRight.m_strErrorMsg ) )
			{
				return true;
			}

			return false;
		}

		int m_iIDError;
		RejectedReasonsFlag m_eFlag;
		CString m_strErrorMsg;
	};

	SelectedType GetSelectedType( void ) { return m_eSelectedType; }

	IDPTR GetProductIDPtr() { return m_IDPtr; }
	
	CData *GetpData() { return ( CData * )m_IDPtr.MP; }

	void SetProductIDPtr( IDPTR idptr )	{ m_IDPtr = idptr; }
	
	CDB_Vessel::ePMCompFlags GetCompatibilityFlag() const {	return m_CompatibilityFlag;	}

	void SetPriceIndex( double dPriceIndex ) { m_dPriceIndex = dPriceIndex; }
	double GetPriceIndex( void ) { return m_dPriceIndex; }

	// Set if the product has been rejected.
	void SetRejected( void ) { m_bRejected = true; }
	bool GetRejected( void ) { return m_bRejected; }

	// 'bVerifyDuplication' true if the method must exclude duplicate rejected reasons.
	void AddRejectedReasons( std::vector<RejectedReasons> &vecRejectedReasons, bool bVerifyDuplication = false );
	void ClearRejectedReasons();

	int GetRejectReasonCount( void );

	// Return -1 if nothing.
	CString GetFirstRejectedReason( int *piIDIerror = NULL );
	CString GetNextRejectedReason( int *piIDIerror = NULL );

	// Flags methods.
	bool IsFlagSet( Flags eFlag ) {	return ( GetFlag( eFlag ) ? true : false );	}

	// Allow to set a flag to 'true' or 'false'.
	// Param: 'iErrorCode': if different of -1, it contains an error code linked with this flag.
	// Remark: 'iErrorCode' is to facilitate management with 'OnTextTipFetch' in each RViewSSelXXX class.
	void SetFlag( Flags eFlag, bool bSet, int iErrorCode = -1 );

	// Param: 'iErrorCode': if defined and if an error code exists for this flag then method returns error code with this pointer.
	bool GetFlag( Flags eFlag, int *piErrorCode = NULL );
	int	GetFlagError( Flags eFlag );
	virtual void ClearAllFlags( void ) { m_ulFlags = 0;	}
	
	// For Compresso & Transfero.
	double GetDelta() const { return m_dDelta; }
	void SetDelta( double val ) { m_dDelta = val; }
	double GetqNmax() const { return m_dqNmax; }
	void SetqNax( double val ) { m_dqNmax = val; }

protected:
	SelectedType m_eSelectedType;
	IDPTR m_IDPtr;
	unsigned long m_ulFlags;
	CDB_Vessel::ePMCompFlags m_CompatibilityFlag;
	double m_dPriceIndex;

	// Set to 'true' if the current product has been rejected.
	bool m_bRejected;

	std::vector<RejectedReasons> m_vecRejectedReasons;
	std::vector<RejectedReasons>::iterator m_vecRejectedReasonsIter;

	// Allow to link a flag with a error code (to facilitate management with 'OnTextTipFetch' in each RViewSSelXXX class).
	typedef std::map< Flags, int > mapFlagsInt;
	typedef mapFlagsInt::iterator	mapFlagsIntIter;
	mapFlagsInt m_mapFlagsErrorCode;
	double m_dDelta;
	double m_dqNmax;
};

class CSelectedVssl : public CSelectedPMBase
{
public:
	CSelectedVssl();
	virtual ~CSelectedVssl() {}

	void SetNbreOfVsslNeeded( int val ) { m_iNbreOfVsslNeeded = val; }

	// HYS-1161: Contains what was the initial vessel number that didn't satisfy the rule pa >= P0 + 0.3.
	void SetInitialNbreOfVsslNeeded( int val ) { m_iInitialNbreOfVsslNeeded = val; }

	// HYS-1022: 'pa' becomes 'pa,min'.
	void SetMinimumInitialPressure( double val ) { m_dMinimumInitialPressure = val; }

	// HYS-1022: 'pa,opt' becomes 'pa'.
	void SetInitialPressure( double val ) { m_dInitialPressure = val;	}

	// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
	void SetWaterReserve( double val ) { m_dWaterReserve = val; }

	void SetCompatibilityFlag( CDB_Vessel::ePMCompFlags val ) {	m_CompatibilityFlag = val; }
	void SetSecVesselPriceIndex( double dPriceIndex ) { m_dSecVesselPriceIndex2 = dPriceIndex; }
	void SetPrimaryIntegrated( bool bIntegrated ) { m_bPrimaryIntegrated = bIntegrated; }

	void SetFillingPressure( double dFillingPressure ) { m_dFillingPressure = dFillingPressure; }

	CDB_Vessel *GetVessel() { return dynamic_cast<CDB_Vessel*>( GetpData() ); }
	int GetNbreOfVsslNeeded() const	{ return m_iNbreOfVsslNeeded; }

	// HYS-1161: Contains what was the initial vessel number that didn't satisfy the rule pa >= P0 + 0.3.
	int GetInitialNbreOfVsslNeeded() const	{ return m_iInitialNbreOfVsslNeeded; }

	// HYS-1022: 'pa' becomes 'pa,min'.
	double GetMinimumInitialPressure() const { return m_dMinimumInitialPressure; }

	// HYS-1022: 'pa,opt' becomes 'pa'.
	double GetInitialPressure() const { return m_dInitialPressure; }

	// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
	double GetWaterReserve() const { return m_dWaterReserve; }

	double GetSecVesselPriceIndex() const { return m_dSecVesselPriceIndex2; }
	bool IsPrimaryIntegrated() const { return m_bPrimaryIntegrated; }

	double GetFillingPressure() const { return m_dFillingPressure; }

private:
	int	m_iNbreOfVsslNeeded;
	int m_iInitialNbreOfVsslNeeded;			// HYS-1161: Contains what was the initial vessel number that didn't satisfy the rule pa >= P0 + 0.3.
	double m_dMinimumInitialPressure;		// HYS-1022: 'pa' becomes 'pa,min'.
	double m_dInitialPressure;				// HYS-1022: 'pa,opt' becomes 'pa'.
	double m_dWaterReserve;					// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
	double m_dFillingPressure;				// HYS-1022: we add now the filling pressure.
	bool m_bPrimaryIntegrated;

	// When secondary vessel is available, we have not a 'CSelectedVessel'. It's only a link directly set in 
	// the vessel object itself (CDB_Vessel). Thus we have no way to save the price index for it.
	double m_dSecVesselPriceIndex2;
};

class CSelectedCompresso : public CSelectedPMBase
{
public:
	CSelectedCompresso() : CSelectedPMBase( SelectedType::ST_Compresso ) { m_iCompressoCXNumber = 0; }
	virtual ~CSelectedCompresso() {}
	void SetCompressoCXNumber( int iCompressoCXNumber ) { m_iCompressoCXNumber = iCompressoCXNumber; }
	int GetCompressoCXNumber( void ) { return m_iCompressoCXNumber; }

private:
	int m_iCompressoCXNumber;
};

class CSelectedTransfero : public CSelectedPMBase
{
public:
	typedef struct _BufferVesselData
	{
		struct _BufferVesselData() { m_bOnRequest = false; m_pclVessel = NULL; }
		bool m_bOnRequest;
		CDB_Vessel *m_pclVessel;
	}BufferVesselData;

	CSelectedTransfero() : CSelectedPMBase( SelectedType::ST_Transfero ) { m_mapBufferVesselList.clear(); }

	virtual ~CSelectedTransfero() {}
	
	void SetBufferVesselList( std::map<int, BufferVesselData> &mapBufferVesselList ) { m_mapBufferVesselList = mapBufferVesselList; }
	std::map<int, BufferVesselData> *GetBufferVesselList() { return &m_mapBufferVesselList; }
	IDPTR GetBufferVesselIDPtr( CString strVesselName );
	bool IsBufferVesselExist() { return ( m_mapBufferVesselList.size() > 0 ) ? true : false; }

private:
	std::map<int, BufferVesselData> m_mapBufferVesselList;
};

class CSelectedVento : public CSelectedPMBase
{
public:
	CSelectedVento() : CSelectedPMBase( SelectedType::ST_Vento )
	{
		m_iNbrOfVento = 0;
	}

	virtual ~CSelectedVento() {}

	void SetNumberOfVento( int iNbrOfVento ) { m_iNbrOfVento = iNbrOfVento;	}
	int GetNumberOfVento() const { return m_iNbrOfVento; }

private:
	int m_iNbrOfVento;
};

class CSelectedWaterTreatmentCombination : public CSelectedPMBase
{
public:
	CSelectedWaterTreatmentCombination();
	virtual ~CSelectedWaterTreatmentCombination();

	CSelectedPMBase *GetSelectedFirst() { return m_pclSelectedFirst; }
	CSelectedPMBase *GetSelectedSecond() { return m_pclSelectedSecond; }
	CDB_Set *GetWTCombination() { return m_pclWTCombination; }
	
	void SetWTCombination( CDB_Set *pclWTCombination ) { m_pclWTCombination = pclWTCombination; }

	// Overrides 'CSelectedPMBase' public virtual methods.
	virtual void ClearAllFlags( void );

private:
	CSelectedPMBase *m_pclSelectedFirst;
	CSelectedPMBase *m_pclSelectedSecond;
	CDB_Set *m_pclWTCombination;
};

class CSelectPMList
{
public:
	enum ePMListRetCode
	{
		epmlrcNone = 0,
		epmlrcOK = 1,
		epmlrcTooHighPressure = 2,
		epmlrcTooHighTemp = 4,
	};

	typedef enum ProductType
	{
		PT_First,
		PT_ExpansionVessel = PT_First,
		PT_ExpansionVesselMembrane,
		PT_IntermediateVessel,
		PT_Compresso,
		PT_CompressoVessel,
		PT_Transfero,
		PT_TransferoVessel,
		PT_Pleno,
		PT_Pleno_Protec,
		PT_PlenoRefill,
		PT_Vento,
		PT_Last
	};

	typedef enum WhichList
	{
		WL_Selected,
		WL_Rejected
	};

	typedef enum BestPriceIndexSolution
	{
		BPIS_First,
		BPIS_ExpansionVessel = BPIS_First,
		BPIS_ExpansionVesselMembrane,
		BPIS_Compresso,
		BPIS_Transfero,
		BPIS_Pleno,
		BPIS_Vento,
		BPIS_Last
	};

	typedef enum CheckPressureVolume
	{
		eAcceptVessel,
		eRejectVessel,
		eNotApplicable
	};

	CSelectPMList();
	virtual ~CSelectPMList();

	void SetpTADB( CTADatabase *pTADB );
	void SetpTADS( CTADatastruct *pTADS );

	void Clean( void );
	int Select( void );

	CPMInputUser *GetpclPMInputUser() { return m_pclInputUser; }

	// Test the Tmin & Tmax of the product in regards to the application type and input user temperatures.
	bool IsErrorTemperature( CDB_Product *pclProduct, int *piTempErrorCode = NULL );

	bool IsIntermVesselNeeded( CDB_Product *pclProduct )
	{
		return IsErrorTemperature( pclProduct );
	}

	int GetIntermVsslList( CSelectedVssl *pSelectedVssl );
	int SelectCompressoVessel( CSelectedPMBase *pSelectedCompresso );
	int SelectTransferoVessel( CSelectedPMBase *pSelectedTransfero );
	
	// 'bOnlyPleno': the method must search only Pleno.
	void SelectVentoPleno( CSelectedPMBase *pSelectedPM = NULL, bool bOnlyPleno = false );
	int SelectPlenoProtec( CSelectedPMBase * pSelectedPM = NULL );
	int SelectPlenoRefill( CDB_TBPlenoVento *pclSelectedPleno, bool bAtLeastOneDeviceSelectedHasVacuumDegassingFunction, CDB_Set *pclWTCombination = NULL );

	bool IsProductExist( ProductType eProductType, WhichList eWhichList = WL_Selected );

	// Check if there is at least one selected and/or rejected product.
	bool IsProductExistInBothList( ProductType eProductType );

	CSelectedPMBase *GetBestProduct( ProductType eProductType );
	CSelectedPMBase *GetFirstProduct( ProductType eProductType, WhichList eWhichList = WL_Selected );
	CSelectedPMBase *GetNextProduct( ProductType eProductType, WhichList eWhichList = WL_Selected );
	CSelectedPMBase *GetPrevProduct( ProductType eProductType, WhichList eWhichList = WL_Selected );
	CSelectedPMBase *GetLastProduct( ProductType eProductType, WhichList eWhichList = WL_Selected );

	// Allow to prepare price indexes based on the current result.
	void ComputePriceIndex( void );
	
	// To get the best price index for a solution (Statico or Compresso + Primary vessel + Secondary vessel for example).
	double GetBestPriceIndex( BestPriceIndexSolution eWhichSolution );
	CSelectedPMBase *GetBestPriceIndexProduct( BestPriceIndexSolution eWhichSolution );
	
	// To get the price of the solution that is the cheapest one.
	double GetBPILowestPrice( BestPriceIndexSolution eWhichSolution ) { return m_arsBestPriceIndex[eWhichSolution].m_dLowestPrice; }
	
	// To get lowest price for a type of object.
	double GetLowestPrice( ProductType eProductType ) { return m_arsPriceData[eProductType].m_dLowestPrice; }
	CSelectedPMBase *GetLowestPriceObject( ProductType eProductType ) { return m_arsPriceData[eProductType].m_pclSelectedPMBase; }
	
	// Allow to retrieve curves of all Transfero belonging to the same family of the 'pclTecBoxReference'.
	// Param: 'dPmanSI' is the target pressure.
	// Param: 'dqNSI' is the needed flow rate of equalization volume.
	CDlgPMTChartHelper *GetTransferoCurves( CDB_TecBox *pclTransferoReference, double dPmanSI, double dqNSI, CWnd *pWnd );

	// HYS-599: 'iNbrOfDevices' has been added in the case of Compresso CX.
	CDlgPMTChartHelper *GetCompressoCurves( CDB_TecBox *pclCompressoReference, double dPmanSI, double dqNSI, CWnd *pWn, int iNbrOfDevices );

	CDlgPMTChartHelper *GetExpansionVesselCurves( CSelectedVssl *pclSelectedVessel, CWnd *pWnd );

	HBITMAP GetPressureGauge( double dp0, double dpa, double dpe, double dPSV, CWnd *pWnd );

	// Just use for 'sort' routine that needs a static function.
	// Remark: this method must return 'true' if 'i' must be placed before 'j'. Otherwise returns 'false' and it is 'j' that is placed before 'i'.
	static bool _CompareVssl( CSelectedPMBase *i, CSelectedPMBase *j );
	static bool _CompareVsslAfter( CSelectedPMBase *i, CSelectedPMBase *j );
	static bool _CompareTecBoxVessel( CSelectedPMBase *i, CSelectedPMBase *j );
	static bool _CompareCompressoTransfero( CSelectedPMBase *pclFirstTecBox, CSelectedPMBase *pclSecondTecBox );
	static bool _ComparePlenoVento( CSelectedPMBase *pclFirstPlenoVento, CSelectedPMBase *pclSecondPlenoVento );
	static bool _ComparePlenoRefill( CSelectedPMBase *pclFirstPlenoRefill, CSelectedPMBase *pclSecondPlenoRefill );
	static bool _CompareBufferVessel( CSelectedTransfero::BufferVesselData rFirstBufferVesselData, CSelectedTransfero::BufferVesselData rSecondBufferVesselData );
	
	void GetTransferoBufferVesselList( CDB_TecBox *pTB, std::map<int, CSelectedTransfero::BufferVesselData> *pmapBufVssl = NULL );

	// HYS-1402 : Is the vessel nominal volume and presure respects the pressure volume limit ?
	// This function return eNotApplicable if false == m_pclInputUser->UsePressureVolumeLimit(), eAcceptVessel if the 
	// vessel is accepted and eRejectVessel if the vessel is rejected.
	// HYS-1593: Add bForBufferVessel
	CheckPressureVolume IsVolumeRespectsPressureVolumeLimit( CDB_Vessel *pclVessel, double dPSVS, bool bForBufferVessel = false );

// Private methods.
private:
	typedef std::vector< CSelectedPMBase * > vecSelectePMBase;
	typedef vecSelectePMBase::iterator vecSelectePMBaseIter;

	// Param: 'PMCompatibilityFlag' can be a combination of 'ePMComp_Statico', 'ePMComp_Compresso', 'ePMComp_Transfero', 'ePMComp_TransferoTI' and 'ePMComp_VsslIntCoating'.
	// Param: 'pTypeID' contains the vessel type ID that we want (VSSLTYPE_STAT, VSSLTYPE_STATMEMBR, VSSLTYPE_PRIM, VSSLTYPE_SEC, VSSLTYPE_INTERM or VSSLTYPE_AQUA).
	// Param: 'bUnlockPressureVolumeLimit': HYS-1003: if we are working with SWKI HE301-01 norm and there is no solution, we can propose a list of vessel that derogate to this rule.
	// Param: 'pclBaseVesselForIntermediate' if defined it's the vessel for which we want the intermediate one.
	int _SelectVessel( int iPMCompatibilityFlag, CDB_StringID *pTypeID, bool bUnlockPressureVolumeLimit = false, CSelectedVssl *pclBaseVesselForIntermediate = NULL, 
			double dTecBoxHeight = 0.0, CDB_TecBox *pCurrentTBSelected = NULL );

	double GetSmallestMinVesselVolumeForDegassing( void );
	
	int _VerifyIntegratedVessel( CDB_TecBox *pclCompresso );
	int _SelectTecBox( CStringArray *parTypeID, CDB_TecBox::TecBoxType Tbt );

	// HYS-981: special case for Transfero TV. We need to check if we need 'H' module.
	void _CheckHModuleForTransferoTV( void );
	
	int _SelectPleno( CSelectedPMBase *pSelectedPM = NULL );

	int _SelectVento( CSelectedPMBase *pSelectedPM = NULL );
	bool _IsSelectedVP( void );
	bool _SelectPlenoHelper( CDB_TBPlenoVento *pTecBoxPleno, CSelectedPMBase *pSelectedPM, std::vector<CSelectedPMBase::RejectedReasons> &vecRejectedReasons );
	void _CheckSimplyCompressoIntegratedVessel( CDB_TecBox *pTecBox, std::vector<CSelectedPMBase::RejectedReasons> *pvecRejectReasons );

	// Param: 'bVesselAlone': set if vessel is selected alone or with a Compresso/Transfero
	// Param: 'bUnlockPressureVolumeLimit': HYS-1003: if we are working with SWKI HE301-01 norm and there is no solution, we can propose a list of vessel that derogate to this rule.
	// Param: if 'pvecSelectedList' and 'pvecRejectedList' we will work with these vectors instead of 'm_arvecSelectedProducts' and 'm_arvecSelectedProducts'.
	//        BE SURE to clear yourself 'pvecSelectedList' and 'pvecRejectedList' when no more needed!
	// Remark: In the 'SelectVessel' method we run all vessel and we insert all in the 'm_vecSelectedPMList'. If there is an error, the reason
	//         is saved but the vessel is not yet in the 'm_vecRejectedList'. Here we will run all the results and dispatch in 'm_arvecSelectedProducts'
	//         or 'm_arvecSelectedProducts' in regards to if there is no error or if error is already set (or a new error is detected).
	void _SortVsslListAndInsertInVector( ProductType eProductType, bool bVesselAlone, bool bUnlockPressureVolumeLimit = false, vecSelectePMBase *pvecSelectedList = NULL, vecSelectePMBase *pvecRejectedList = NULL );

	void _SortCompressoAndInsertInVector( WhichList eWhatToSort );
	void _SortTransferoAndInsertInVector( WhichList eWhatToSort );

	void _SortPlenoAndInsertInVector( WhichList eWhatToSort, ProductType ePType = ProductType::PT_Pleno );
	void _SortPlenoRefillAndInsertInVector( WhichList eWhatToSort );
	void _SortVentoAndInsertInVector( WhichList eWhatToSort, bool bAllAreBiggerThanMaxNumberOfVentoInParallel, int iLowerMaxNumberOfVentoInParallel );

	void _SetBestCompressoTecBox( void );
	
	// Clear 'm_arvecSelectedProducts' and 'm_arvecSelectedProducts' for the specify product type.
	// Param: if 'pvecSelectedToClean' and 'pvecRejectedToClean' are defined, we will clean this vectors instead of the internally ones.
	void _CleanVecKeyPMBase( ProductType eProductType, vecSelectePMBase *pvecSelectedToClean = NULL, vecSelectePMBase *pvecRejectedToClean = NULL );

	void _ComputePriceIndex( bool bFirstPass );

	int _GetTransferoTecBoxInSameGroup( CRankEx *pList, CDB_TecBox *pclTransferoReference );
	int _GetCompressoCurveInSameGroup( CRankEx *pList, CDB_TecBox *pclCompressoReference );

private:
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;

	vecSelectePMBase m_vSelectedPMlist;
	vecSelectePMBase m_vRejectedPMlist;

	CPMInputUser *m_pclInputUser;

	vecSelectePMBase m_arvecSelectedProducts[PT_Last];
	int m_ariSelectedProductsIndex[PT_Last];				// Just for iteration.

	vecSelectePMBase m_arvecRejectedProducts[PT_Last];
	int m_ariRejectedProductsIndex[PT_Last];				// Just for iteration.

	typedef struct _PriceData
	{
		CSelectedPMBase *m_pclSelectedPMBase;
		double m_dLowestPrice;
		struct _PriceData()
		{
			m_pclSelectedPMBase = NULL;
			m_dLowestPrice = 0.0;
		}
	}PriceData;
	PriceData m_arsPriceData[PT_Last];

	typedef struct _BestPriceIndex
	{
		CSelectedPMBase *m_pclSelectedPMBase;
		double m_dLowestPrice;
		double m_dBestPriceIndex;
		struct _BestPriceIndex()
		{
			m_pclSelectedPMBase = NULL;
			m_dLowestPrice = 0.0;
			m_dBestPriceIndex = 0.0;
		}
	}BestPriceIndex;
	BestPriceIndex m_arsBestPriceIndex[BPIS_Last];

	// TO TEST
	HBITMAP m_hIconButtonDone;
};
