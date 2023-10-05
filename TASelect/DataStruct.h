//////////////////////////////////////////////////////////////////////////
//  DataStruct.h : header file
//	Version: 1.0							Date: 10/04/08
//////////////////////////////////////////////////////////////////////////

#ifndef DATASTRUCT_H__INCLUDED_
	#define DATASTRUCT_H__INCLUDED_
	#include <list>
	#include "global.h"
	#ifndef PRODUCTION
		#include "database.h"
		#include "databobj.h"
		#include "tadatabase.h"
		#include "datastruct.hpp"
	#endif
	#ifdef TACBX
		#ifndef PRODUCTION
			#ifndef BOOTLOADER
				#include "version.h"
				#include "SensorDefinitions.h"
			#endif
		#endif
	#else
		#define uint8_t unsigned char 
		#define uint16_t unsigned short
	#endif
// #endif for 'DATASTRUCT_H__INCLUDED_' is at the end of this file.

///////////////////////////////////////////////////////////////////////////////////
//
//								CEquation
//
///////////////////////////////////////////////////////////////////////////////////
#define MAXPOLYORDER	10
class CEquation
{
public:
	enum eEquType
	{
		EqT_None,
		EqT_Poly,
		EqT_Linear,
		EqT_Ntc,
		EqT_Linear_Unsigned,
		EqT_LastEquType
	};

	enum eSensorType		// For TA-SCOPE see enum definition in SensorDefinition.h TempSensorType_t
	{
		est_None,
		est_TempDP = 1,		//TEMP_SENSOR_DP = 1,
		est_DTS,			//TEMP_SENSOR_DTS = 2
		est_DPTN,			//TEMP_SENSOR_DPTN
		est_Dp,
		est_Last
	};

	CEquation();
	CEquation(eEquType EType, unsigned char ucOrder);
	~CEquation(){};
protected:
	__time32_t	m_CalibDate;			// Last calibration date/time
	eSensorType m_SensorType;			// Type of connected device
	uint8_t		m_SensorVersion;
	uint8_t		m_SensorMac;
	eEquType	m_EquType;				// Equation type
	unsigned char m_nOrder;				// Equation order, in case of polynomial ones'
	std::vector<double> m_dCoefs;		// Equation coefficients, 
										//		Polynomial : y = K + Ax + Bx^2 + Cx^3 + ....
										//		Linear 	   : Y value (double)
										//		Ntc : 	R = (Rref * x)/(Maxval - x)
										//				y = -273,15 + 1/A + Bln(R) + C(ln(R))3
	std::vector<unsigned short> m_usCoefs;	// Equation coefficients,
											// 		Polynomial : not used
											//		Linear : 	 X value (unsigned short, output from ADC)
public:
	void SetSensorType( eSensorType SensorType) { m_SensorType = SensorType; }
	void SetSensorVersion ( uint8_t SensorVersion) {m_SensorVersion = SensorVersion; }
	void SetSensorMac ( uint8_t SensorMac) { m_SensorMac = SensorMac; }
	void SetCalibrationDate(__time32_t CalibDate){ m_CalibDate = CalibDate; }
	void SetEquationType(eEquType EquType);
	// Polynomial order < MAXPOLYORDER
	// or number of linear segments
	bool SetEquationSize(char Order);
	//  Equation coeficients, y = K + Ax + Bx^2 + Cx^3 + .... 
	//  In: K = Coef(0), A = Coef(1), B = Coef(2), ...
	//  In: Rref= Coef(0), Maxval = Coef(1), A = Coef(2), ...
	//  Out: true if Coef <= Order
	bool SetEquationCoefs(char Coef, double dVal);
	bool SetEquationCoefs(char Coef, unsigned short uiXval, double dYval);

#ifdef TACBX
	// Set equation size and Patch Equation type according to Calibration image type
	// Error in Calibration image of original DTS (XML file + EEPROM): the equation type value is programmed to "2 - Linear" type in place of "1 - Poly"
	// It cannot be used to set the equation type without correction
	// The DPTN/SPTP value is OK: "3 - Ntc"
	void PatchEquation (TempSensorCalibrationImage_t CalibrationImage);
#endif
	
	__time32_t GetCalibrationDate() { return m_CalibDate; }
	eSensorType GetSensorType()	{ return m_SensorType; }
	uint8_t GetSensorVersion () { return m_SensorVersion; }
	uint8_t GetSensorMac() { return m_SensorMac; }
	eEquType GetEquationType() {return m_EquType; }
	char	GetEquationSize() { return m_nOrder; }
	double  GetEquationCoefs(char Coef);
	unsigned short  GetEquationYCoefs(char nCoef);
	double  Solve(double dVal);
	
	CEquation& operator =(CEquation& Equation);
	void Copy( CEquation *pEquation );

#ifndef PRODUCTION
	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );
#endif
};



#ifndef PRODUCTION

class CWaterChar;
class CDS_WaterCharacteristic;
class CDS_HydroModX;
class CDS_PlantX;
class CProductSelelectionParameters;

#ifndef TACBX
class CPMWQPrefs;
#endif

#define TASCOPE_BASE_FORMAT_VERSION _T("400") // Never change this value, but modify value below

///////////////////////////////////////////////////////
//			CTADatastruct Version
// !!!!! DON'T FORGET TO UPDATE CTADATASTRUCTX_FORMAT_VERSION WHEN IT'S NEEDED
///////////////////////////////////////////////////////

// Structure of the CTADatastruct:
//
//  ROOT
//   |
//   +---> CTable ("PARAM_TABLE")
//   |       |
//   |       +---> CDS_TechnicalParameter ("PARAM_TECH")
//   |       +---> CDS_WaterCharacteristic ("PARAM_WC")
//   |       +---> CDS_IndSelParameter ("PARAM_INDSEL")
//   |       +---> CDS_BatchSelParameter ("PARAM_BATCHSEL")
//   |       +---> CDS_WizardSelParameter ("PARAM_WIZARDSEL")
//   |       +---> CDB_PageSetup ("PARAM_PAGESETUP")
//   |       +---> CDS_ProjectRef ("PROJECT_REF")
//   |       +---> CDS_UserRef ("USER_REF")
//   |    
//   +---> CTable ("PROJ_TABLE")
//   |       |
//   |       +---> CDS_PersistData ("PERSIST_DATA")
//   |       +---> CDS_PersistData ("PROJ_WC")
//   |       +---> CDS_ProjectParams ("GENERAL_PARAMS")
//   |       +---> CTable ("TMPSAVEDATA_TAB")
//   |       +---> CTableHM ("PIPING_TAB")						-> contains 'CDS_HydroMod'
//   |       +---> CTableHM ("TMPHUB_TAB")
//   |       +---> CTableHM ("SELHUB_TAB")						-> contains 'CDS_HmHub', 'CDS_HmHubStation'
//   |       +---> CTable ("HUBCVTEMP_TAB")
//   |       +---> CTable ("WC_TAB")							-> contains 'CDS_WaterCharacteristic'
//   |       +---> CDS_QMTable ("QUICKMEAS_TAB")				-> contains 'CDS_HydroModX'
//   |       +---> CDS_LogTable ("LOGDATA_TAB")					-> contains 'CDS_LoggedData'
//   |                 |
//   |                 +---> CDS_LogTable ("TMPLOGDATA_TAB")
//   |        
//   +---> CTable ("SSEL_TABLE")
//   |       |
//   |       +---> CTable ("REGVALV_TAB")			-> contains 'CDS_SSelBv'
//   |       +---> CTable ("DPCONTR_TAB")			-> contains 'CDS_SSelDpC', 'CDS_SelDpReliefValve' and 'CDS_SSelDpSensor'.
//   |       +---> CTable ("DPCBCVALVE_TAB")		-> contains 'CDS_SSelDpCBCV'
//   |       +---> CTable ("CTRLVALVE_TAB")			-> contains 'CDS_SelManBv' (old), 'CDS_SSelCv', 'CDS_SSelPICv', 'CDS_SSelBCv', 'CDS_Actuator', 'CDS_Accessory'
//   |       +---> CTable ("SHUTOFF_TAB")    		-> contains 'CDS_SSelSv'
//   |       +---> CTable ("PARTDEF_TAB")    		-> contains 'CDS_DirectSel'
//   |       +---> CTable ("RADSET_TAB")			-> contains 'CDS_SelRadSet' (old), 'CDS_SSelRadSet'
//   |       +---> CTable ("AIRVENTSEP_TAB")		-> contains 'CDS_SSelAirVentSeparator'
//   |       +---> CTable ("PRESSMAINT_TAB") 		-> contains 'CDS_SSelPMaint'
//   |       +---> CTable ("SAFETYVALVE_TAB")		-> contains 'CDS_SSelSafetyValve'
//   |       +---> CTable ("6WAYCTRLVALV_TAB")		-> contains 'CDS_SSel6WayValve'
//   |       +---> CTable ("TAPWATERCTRL_TAB")		-> contains 'CDS_SSelTapWaterControl'
//	 |		 +---> CTable ("SMARTCONTROLVALVE_TAB")	-> contains 'CDS_SSelSmartControlValve'
//	 |		 +---> CTable ("SMARTDPC_TAB")	        -> contains 'CDS_SSelSmartDpC'
// 	 |		 +---> CTable ("FLOORHCTRL_TAB")		-> contains 'CDS_SSelFloorHeatingManifold', 'CDS_SSelFloorHeatingValve' and 'CDS_SSelFloorHeatingController'
//   +---> CTable ("CLIPBOARD_TABLE")
//   |        
//   +---> CTable ("DIRSEL_TAB") -> contains 'CDS_DirectSel' (old)

// Remark: When to change the version ?
// The datastruct version (CTADATASTRUCT_FORMAT_VERSION) allows to an old HySelect to not open a 'tsp' file created
// with an earlier HySelect.
// The versions defined for all objects inserted in the CTADataStruct table allow to new HySelect to open 'tsp' file
// created with an old HySelect.
// Thus, as soon as there is a change in any of the objects that are saved in the CTADataStruct, we need to change the version
// in the concerned object AND for 'CTADATASTRUCT_FORMAT_VERSION'.

// 4007  TABal+
// 4008  Modification of CDS_SelProd
// 4009  DpC location
// 4010  CDS_SSelRadSet added
// 4011  CEquation modified; CDS_IndSelParameter
// 4012  Convert old CDS_DirectSel to CDS_SSelXXX, CMeasData variables DesignDp and Design power removed.
// 4014  (2016-10-06) Pay attention: CDS_SSelParameter was changed into CDS_IndSepParameter in CTADataStruct version 4.0.1.4
// 4015  Convert DA50
// 			DA50_???J   -->   DA50_???I
// 			DA50_???L   -->   DA50_???K
// 			DA50_???N   -->   DA50_???M
// 			DA50_???P   -->   DA50_???O
// 4016  Add the CDS_SSelAirVentSeparator object.
// 4017 
// 4018  TA-COMPACT-P & TA-PILOT_R added
// 4019 Add the SelectP object.
// 4020 Add max. number of Vento in parallel in the technical parameters.
// 4021 Version 9 -> 11 for CDB_PageField (CDB_PageField is saved with project)
// 4022 CDB_PageField Version 11 -> 12 (CDB_PageField is saved with project).
// 4023 DpMax added for SelDpC/PIBCV
// 4024 Add consumers and generators for Cooling System Volume
// 4025 Surface cooling for Cooling System Volume and new variable 'Default water hardness' in the technical parameters.
//      CDB_PageField Version 12 -> 13 (CDB_PageField is saved with project).
// 4026 2015/11/10 - Add the 'm_eBCv2W3W' variable for the batch selection.
//                 - Variable for DpC are ready (batch selection not yet activated in this version)
// 4027 2015/11/17 - Combined Dp controller and balancing & control valve added.
// 4028 2016/10/05 - Some changes for combined Dp controller, balancing and control valve (TA-COMPACT-DP).
// 4029 2016/12/13 - 'm_rTechParamData.m_dVesselSelectionRangePercentage' added.
// 4030 2017/03/07 - Additives are saved.
// OUPS!!! There was 2 major changes and we forgot to increment the version. !!!!!!!!!!!!!!!!!!!!!
// I write here the fictive version (They have never existed!!)
// 4031 2017/07/20 - 'CDS_WizardSelParameters' class added.
// 4032 2017/11/24 - 'm_bPICvCtrlTypeStrictChecked' and 'm_bBCvCtrlTypeStrictChecked' variables added in the 'CDS_BachSelParameters'
//                   class. And 'm_eDpCLoc' variable added in 'CDS_SSelDpC'.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 4033 2018/05/30 - Correction of the 4030!
// 4034 2018/07/10 - Change in the 'CPMInputUser' class that it is saved with the 'CDS_SSelPMaint' object.
// 4035 2018/08/16 - 'm_rTechParamData.m_dDegassingWaterReserveForVessel' added.
// 4036 2018/09/11 - 'm_rTechParamData.m_dDefaultISSolarTps', 'm_rTechParamData.m_dDefaultISSolarDT', 'm_rTechParamData.m_strDefaultISSolarAdditID'
//                   and 'm_rTechParamData.m_dDefaultISSolarPcWeight' added.
// 4037 2018/11/20 - Add the 'CDS_SSelSafetyValve' object.
// 4038 2019/02/18 - HYS-1022: Add the 'm_dInitialPressureMargin' variable.
// 4039 2019/04/25 - HYS-1058: We need to save pressurisation maintenance and water quality preferences.
// 4040 2019/06/03 - HYS-1104: Add safety temp limiter and supply/return temperature limit for the EN 12953 norm.
// 4041 2019/10/03 - HYS-1079: Add variables for NC/NO, HYS-1126: add a technical parameter for pst calculated in a simplified way.
// 4042 2019/10/15 - HYS-1150: Add the 'CDS_SSel6WayValve' object.
// 4043 2020/02/06 - HYS-1258: Add the 'CDS_SSelTapWaterControl' object.
// 4044 2020/11/02 - HYS-1511: Add the 'CDS_SSelSmartControlValve' object.
// 4045 2021/04/21 - HYS-1571: Add the 'CDS_SSelFloorHeatingManifold', 'CDS_SSelFloorHeatingValve', 'CDS_SSelFloorHeatingController' and 'CDS_SSelDpReliefValve' objects.
// 4046 2021/05/26 - HYS-1678: Correction in the 'CPrjParams::ResetPrjParams' for smart control valve above size DN 50.
// 4047 2021/11/04 - HYS-1763: 'm_rTechParamData.m_iDefaultSmartCVLoc' added.
// 4048 2022/01/22 - HYS-1857: Correction of bad circuit schemes for 3-way circuits.
// 4049 2022/07/08 - HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object. Add CDS_SSelSmartDpC. Add Dpmax for smart valve
// 4050 2022/12/05 - HYS-1992: Remove Checkox set for CDS_INDSELPARAMETER_VERSION
// 4051 2023/01/12 - HYS-1930: New hydronic circuit: auto-adapting variable flow decoupling circuit: Add default secondary pump Hmin.
//      2023/03/31 - HYS-2007: Add CDS_SSelDpSensor object for TA Link selected alone.
#define CTADATASTRUCT_FORMAT_VERSION	_T("4051")	// !!! Just an integer:
													// Used to prevent opening new .tsp files with 
													// old TAS versions
													// Update this value when read or/and write function of any 
													// object involved in the .tsp file is modified

///////////////////////////////////////////////////////
//			CTADatastructX Version		"*.tsc files"
///////////////////////////////////////////////////////
// 4031 2018-11-12 - Change in the 'CDS_LogDataX::CLogChan::Read' and 'CDS_LogDataX::CLogChan::Write' methods.
#define CTADATASTRUCTX_FORMAT_VERSION	_T("4031")	 	// !!! Just an integer:

///////////////////////////////////////////////////////
//			CTAMetaData Version			"*.tmd files"
//
// See the 'CTAMetaData::CTAMetaData' constructor in the 'DataStruc.cpp' file to have information about this class.
//
// This class is inherited from the 'CDataBase' class. It contains only 'CDS_PlantX' classes. If these last classes
// are changed, no need to increment the version here (It's in the 'CDS_PlantX' class that we need to change).
// But if we add other informations around this objects saved (A change in the file identificator or in the header for example)
// we need to increment.
///////////////////////////////////////////////////////
#define CTAMETADATA_FORMAT_VERSION		_T("4002")		 // !!! Just an integer:

///////////////////////////////////////////////////////
//			CTAPersistData Version		"*.tpd files"
///////////////////////////////////////////////////////
// 4006 2021/06/21 - TASCOPE-89: yet a new unit added in HySelect and not managed in TA-Scope !!
// 4005 2020/10/19 - TASCOPE-85: when resetting (TASCOPE-84), we must add a key in the 'CDB_MultiString' object.
// 4004 2020/10/15 - HYS-1521 (TASCOPE-84) : Units changed in HYS-1367 and then HYS-1518 causes a crash in some circumstances.
#define CTAPERSISTDATA_FORMAT_VERSION	_T("4006")		 // !!! Just an integer:


///////////////////////////////////////////////////////////////////////////////////
//
//								CTADataStructX
// TA-CBX storage data class
			//====================================
			// CDataStructX contains 4 tables
			//====================================
			// ID = PARAM_TAB
			// Parameter table
			// Misc. table used to store user preferences as Unit, Plant Information

			//====================================
			// ID = WC_TAB
			// Water characteristic table
			// Table that contains all Water characteristic used during measurement (Piping table, Quick measurement table)
			//====================================

			//====================================
			// ID = PIPING_TAB
			// Piping table
			// Table used for storing hydraulic network
			//====================================

			//====================================
			// ID = QUICKMEAS_TAB
			// Quick measurement table
			// Table used to store saved quick measurement
			//====================================

			//====================================
			// Logged data table
			// Table used to store saved logging (CDS_LogDataX)
			//====================================

///////////////////////////////////////////////////////////////////////////////////
class CTADataStructX : public CDatastruct
{
public:
	CTADataStructX();
	virtual ~CTADataStructX() {}
	
	// TASCOPE-59.
	typedef enum
	{
		DSXType_TADSX,					// If the concerned database is 'tadsx.tsc'.
		DSXType_HMProject,				// If the concerned database is other '.tsc' file (network).
	}DataStructXType;
	void SetDSType( DataStructXType eDataStructXType ) { m_eDataStructXType = eDataStructXType; }
	DataStructXType GetDSType() { return m_eDataStructXType; }
	void SetTAScopeVersion( int iVersion ) { m_iTAScopeVersion = iVersion; }
	int GetTAScopeVersion() { return m_iTAScopeVersion; }
	
	// Initialize tables.
	void Init();

	static void ReadShortInfosList( INPSTREAM  inpf, CDS_PlantX *pPlant );

	// CDS_WaterCharacteristic* of matching water characteristic
	// NULL if it could not find it.
	CDS_WaterCharacteristic *FindWC( CWaterChar *pWC );

	CDS_WaterCharacteristic *AddIfNotExist( CWaterChar *pWC );
	
	// Recursive function that reset all name for a module.
	void RenameAllCircuits( CTable *pTab );

	// Compose module/Circuit name.
	_string ComposeHMName( int Pos, TCHAR *pHMName, bool bTop, _string strPrefix, bool bNumber );
	_string ComposeHMName( CDS_HydroModX *pHM, TCHAR cStar = _T('*'), bool bAuto = true, _string strPrefix = _T(""), bool bNumber = true );

	// Update design flow for all parents modules up to the main table.
	// Reset TABalance flags for all ancestor.
	void UpdateParentDesignQResetBal( CDS_HydroModX *pHMX, bool bResetBalancing = true );

	// Delete HydroModX and all children.
	void DeleteHMX(CDS_HydroModX *pHMX);

	// Recursive function delete all HMX children.
	void DeleteHMXBranch( CDS_HydroModX *pHMX );
	
	// Re-index the modules of pTab if the deleted index generates a hole in the indexing.
	void RemoveIndexHole( CTable *pTab, int iDeletedIndex );
	
	// Used to insert modules.
	void CreatePosGap(CTable *pTab, int iFromPos, int iGap );

	// Move HM into a new position into the current module.
	void MoveCircuitInModule(CDS_HydroModX *pHMX, UINT uiNewPos );

	// Clean Balancing and Index flags for a line of HMX.
	void CleanBalancingFlags( CDS_HydroModX *pHMX );

	CDS_HydroModX *FindHydroMod( _string str, CTable *pTab, CDS_HydroModX *pHMToSkip = NULL, CDatastruct::FindMode eFindMode = CDatastruct::Both );

	virtual void Write( char *fn ) { CDataBase::Write( fn ); }
	virtual void Write( OUTSTREAM outf );

#ifndef TACBX
	virtual void Read( TCHAR *fn ) { CDataBase::Read( fn ); }
#else
	virtual void Read( char *fn ) { CDataBase::Read( fn ); }
#endif
	virtual void Read( INPSTREAM inpf );

	static bool ReadHeader( INPSTREAM  inpf, short Version, _string *pStrUID = NULL, CTADataStructX *pDS = NULL );

protected:
	virtual void WriteHeader( OUTSTREAM outf );

	// Fill short info list by getting information from CDS_Objects
	// Return Number of elements stored
	void WriteShortInfoList( OUTSTREAM outf );
	
private:
	// TASCOPE-59: This variable will contains the TA-Scope version that have written the "tadsx.tsc" file. 
	int m_iTAScopeVersion;
	DataStructXType m_eDataStructXType;
};

///////////////////////////////////////////////////////////////////////////////////
/*
 * Class used to store collected data information from other DataStructX 
 * 
 */
class CTAMetaData : public CDataBase
{
public:
	CTAMetaData();
	virtual ~CTAMetaData() {}
	
	// Initialize tables.
	void Init();

	virtual void Write( char *fn ) { CDataBase::Write( fn ); }
	virtual void Write( OUTSTREAM outf );

#ifndef TACBX
	virtual void Read( TCHAR *fn ) { CDataBase::Read( fn ); }
#else
	virtual void Read( char *fn ) { CDataBase::Read( fn ); }
#endif

	virtual void Read( INPSTREAM inpf );

protected:
	virtual void WriteHeader( OUTSTREAM outf );
	void ReadHeader( INPSTREAM  inpf, short Version, std::list<_string> *pList = NULL );
};

///////////////////////////////////////////////////////////////////////////////////


#ifdef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CTAPersistData
//
///////////////////////////////////////////////////////////////////////////////////
class CTAPersistData : public CDataBase
{
public:
	enum eUserPersistParam
	{
		name,
		Backlight,
		TimeToDim,
		TimeToSleep,
		TimeToOff,
		KeyBeep,
		ErrorBeep,
		dateformat,
		timeformat,
		dotdash,
		ShowOldValve,
		ShowAllValve,
		DoNotShowInfoMsg,
		RadioEnable,
		LanguageId,
		LogRefNum,
		PipeSerie,
		UseTwoDpS,
		PanChannel,
		UseDpFilter,
		DpFilterOrangeLevel,
		DpFilterRedLevel,
		last
	};

	CTAPersistData();
	virtual ~CTAPersistData() {}
	virtual void Write( char *fn ) { CDataBase::Write( fn ); }
	virtual void Write( OUTSTREAM outf );
	virtual void Read( char *fn ) { return CDataBase::Read( fn ); }
	virtual void Read( INPSTREAM inpf ) { return CDataBase::Read( inpf ); }
	
	// Initialize tables.
	bool FillMissingFields();
	void FillMissingUnits();

};
#endif

///////////////////////////////////////////////////////////////////////////////////
//
//								CTADatastruct
//
///////////////////////////////////////////////////////////////////////////////////
#ifndef TACBX
class CDS_HydroMod;
class CTableHM;
#endif
// CGenParam
// used to store General parameters, valve preference, pipe preference, ... 
// General parameters are stored in CDS_ProjectParams and when it's necessary in CDS_Hydromod

class CDS_TechnicalParameter;
class CDS_ProjectParams;
class CDS_IndSelParameter;
class CDS_BatchSelParameter;
class CDS_WizardSelParameter;
class CDS_ProjectRef;
class CDS_UserRef;
class CDS_PersistData;
class CTADatastruct : public CDatastruct
{
public:
	CTADatastruct();

	virtual void Write( char *&fn ) { CDataBase::Write(fn); }
	virtual void Write( OUTSTREAM outf );

#ifndef TACBX
	virtual void Read( TCHAR *fn ) { CDataBase::Read( fn ); }
#else
	virtual void Read( char *fn ) { CDataBase::Read( fn ); }
#endif
	virtual void Read( INPSTREAM inpf );

	// Initialize the data structure
	// Create the default main tables and their skeleton
	void Init();

#ifndef TACBX
public:
	// Create TSP Project table: PROJ_TABLE, PARTDEF_TAB, SSEL_TABLE, etc.
	void CreateTSPProjTable();
	
	const IDPTR CreateObjectInParentTable( IDPTR &ParentIDPtr, LPCTSTR strID, LPCTSTR className, LPCTSTR objName = NULL );

	// Recursive function that scan all TADS to verify existence of ID.
	void CheckTables( CTable *pTab = NULL );
	
	// Initialize the content of Tech and SSel parameters.
	void ReInitTechAndSSelParam();
	
	// HYS-1590: We add only user pipes that are really used in the project.
	void AddUsedUserPipesBeforeSavingProject();

	// HYS-1590: Verify old pipes IDs when opening a project and prepare a list of pipe series that are not yet in HySelect
	// or that are deleted now in HySelect.
	void VerifyPipeSeriesWhenOpeningProject( std::vector<CString> *pvecPipeSeriesInProjectNotInHySelect, std::vector<CString> *pvecPipeSeriesInProjectButDeletednInHySelect );
	
	// Scan recursively a Hydromod table to verify and to correct if needed position in the module.
	// Return "false" if something as been corrected.
	bool VerifyAndCorrectPosition( CTable *pTab = NULL );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the verify and clean empty s selected object. </summary>
	///
	/// <remarks>	Alen, 4/10/2010. </remarks>
	///
	/// <returns>	Number of objects deleted </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int VerifyAndCleanEmptySSelObj();

	// FF: 2014-03-27: A bug has been introduced in previous version before 4.2.3.4 concerning saving of the 'CDS_BatchSelParameter' in
	//                 a '.tsp' file. We need to verify if data in 'CDS_BatchSelParameter' are relevant.
	void VerifyBatchSelParameter();

	// FF: 2015-02-26: Sometimes the measurements done in hydronic network and imported from the TA Scope were bad (negative values). 
	// Before the correction (reset by a default value) it was possible to import and directly save to a TSP file. In this case, 
	// we need to correct the bad values.
	void VerifyMeasTaBalDateTime();

	// FF: 2015-02-26: Sometimes the quick measurements imported from the TA Scope were bad (negative values). Before the correction (reset
	// by a default value) it was possible to import and directly save to a TSP file. In this case, we need to correct the bad values.
	void VerifyQuickMeasurements();

	// FF: 2018-05-31: before datastruct v28 (before HySelect v4.4.4.9), we have not a perfect match between temperatures set in the 
	// 'CDS_TechnicalParameters' and the temperatures set in the 'PROJ_WC' and 'PARAM_WC' object. It happened for example when you work
	// on a hydraulic network and change the supply temperature in the ribbon. Temperature was saved in the 'PROJ_WC object but not in the
	// 'CDS_TechnicalParameters'. If you open the project with the actual version, it's the data from the 'CDS_TechnicalParameters' that 
	// overwrite the 'PROJ_WC' and 'PARAM_WC'. And because that we lost the real temperature that user had set in the ribbon for his project.
	// To summarize: if datastruct version <= v27, copy 'PROJC_WC'/'PARAM_WC' to 'CDS_TechnicalParameter' (It concerns the supply temperature).
	// HYS-1380: restore the previous application type if openning change-over project and old application type is not change-over.
	void VerifyWaterCharAndTechnicalParameters( ProjectType eOldApplicationType );

	// HYS-1716: When opening a project we run it to update all the water characteristics and temperatures for all pipes.
	// HYS-1882: Add booleen bDeepestToHighest to run also from the top to the deepest.
	void VerifyProjectAllWaterCharacteristics( bool bDeepestToHighest = true );

	// HYS-1857: From HySelect v4.5.0.1 until v4.5.0.6 (Included) an error has been introduced concerning BV at secondary side for 3-way mixing circuits.
	// It was set in the "tadb.txt" file that the BV was optional but it's no true. So, we need to correct now all projects where user
	// has intentionally removed the BV.
	void VerifyAllBVSecForAll3WayCircuits();

	bool GetCheckAllModules() const { return m_bCheckAllModules; }
	void SetCheckAllModules( bool val ) { m_bCheckAllModules = val; }

	void CleanClipboard();
	
	//	Count recursively the number of HM which are descendant of pHM 
	//  (pHM is excluded from the count)
	//  If bValvExcluded is true, all HM that are terminal HM are not included in
	//  the count
	int GetHMCount( CTable *pHM, bool bValvExcluded = true );
	
	// Return true if all Hydromod are in HMCalcMode
	// If bAtLeastOne = true --> Return true if at least one HydroMod is in HMCalcMode
	eBool3 IsHMCalcMode( CTable *pTab = NULL, bool bAtLeastOne = false );

	// Allow to set when user has intentionally force the hydronic calculation mode (when he has pressed the HM calc button in the ribbon).
	// We do that to have a distinction with projects created in CBI mode and sent to TAScope. These projects are automatically converted 
	// to an hydronic mode.
	void SetUserForcedHMCalcModeFlag( bool fUserForcedHMCalcMode );
	bool GetUserForcedHMCalcModeFlag();
	bool IsOldProject( void );
	
	// Do a loop on all module and define the module to be compatible in HydroCalc Mode
	void CompleteProjForHydroCalcMode( CTable *pTab = NULL, bool fLock = true );
	
	// Do a loop on all modules and lock all valves in it
	void SetHMValvesLock( bool bLock, CTable *pTab = NULL );
	
	// Do a loop on all module and verify if one Bv is partially define
	bool IsOneValveNotDefine( CTable *pTab = NULL );
	
	// Delete HM and all children.
	void DeleteHM( CDS_HydroMod *pHM );
	
	//	Delete contents of one TADS Branch (recursive function).
	void DeleteTADSBranch( CTable *pHM );
	
	// Duplicate a valve in selected valve and update ID in the hydraulic module.
	IDPTR DuplicateHydroMod( const IDPTR &idptrParent, const IDPTR &idptrSrc, bool fSetUID = true );
	
	// Return the last used index for this branch.
	int GetLastBranchPos( CTable *pHM );
	
	// Verify if the new index is "free". If the position is not free shift up 
	// all HM indices >= NewIndex 
	// !!! Must be called before the index of the inserted HM is set 
	// !!! (otherwise its index is also shifted up!)
	void VerifyNewPos( CTable *pTab, int NewIndex );
	
	// Change module position in his parent.
	void ChangePosition( CDS_HydroMod *pHM, int NewPos );

	// Reindex the modules of pTab if the deleted index generates a hole in the indexing.
	void RemoveIndexHole( CTable *pTab, int DeletedIndex );

	// If 'fArgIsClassName', the first argument must be a table ID.
	typedef enum
	{
		SNI_ClassName,
		SNI_TableID
	}SNI_ArgumentType;
	void SetNewIndex( CString strName, SNI_ArgumentType eArgType = SNI_ClassName );

	
	// Copy HydroMod to/from the clipboard, if HydroMod is a module copy children too
	// pHMSrc is the source HydroMod that will be copied into the clipboard 
	// pTab is used internally for recursivity.
	IDPTR CopyHMToClipBoard( CDS_HydroMod *pHMSrc, CTable *pTab = NULL );

	// If 'fNoChildren' is set to 'true' we copy only the module without any its children.
	// Remark: 'fNoChildren' set to 'true' is used for example in 'RViewHMCalc' when copy a module and specific children.
	IDPTR CreateCopyHM( IDPTR IDPtrSrc, CTable *pTabTrg, bool fNoChildren = false );

protected:
	// pTabTarget is the table where the clipboard content must be copied
	// pTab is used internally for recursivity
	IDPTR CopyHMFromClipBoard( CTable *pTabTarget, CTable *pTab = NULL );
	void CleanUnNeededSelectedProduct( CTable *pTab );
	void CleanOtherUnNeededSelectedProduct();
	void UpgradeTBVCSelection();
	void UpgradeManBvSelection();
	void UpgradeDpCSelection();
	void UpgradeDA50IDPTR();
	void UpgradeRadSetSelection();
	void UpgradeDirectSelSelection();
	IDPTR FoundPipe( CString strPipeSeriesName, CString strPipeSizeName );
	bool IsAnchorPtExist( CArray<CAnchorPt::eFunc> *paFunc, CAnchorPt::eFunc func );

	// If we work with a temporary pipe table, we need to review all the IDPTR.MP for pipes that are no
	// more valid.
	void UpdatePipeIDPtrs();

public:
	// Fast access to the technical parameters.
	CDS_TechnicalParameter *GetpTechParams();

	// Fast access to the water characteristics for product selection.
	CDS_WaterCharacteristic *GetpWCForProductSelection();

	// Fast access to the water characteristics for HM calc.
	CDS_WaterCharacteristic *GetpWCForProject();
	
	// Fast access to the project parameters.
	CDS_ProjectParams *GetpProjectParams();

	// Fast access to the individual selection parameters.
	CDS_IndSelParameter *GetpIndSelParameter();

	// Fast access to the batch selection parameters.
	CDS_BatchSelParameter *GetpBatchSelParameter();

	// Fast access to the wizard selection parameters.
	CDS_WizardSelParameter *GetpWizardSelParameter();

	// Fast access to the persisted data.
	CDS_PersistData *GetpPersistData();

	// Fast access to the page setup.
	CDB_PageSetup *GetpPageSetup();

	// Fast access to the project references.
	CDS_ProjectRef *GetpProjectRef();

	// Fast access to the user references.
	CDS_UserRef *GetpUserRef();
	
	// Fast access to the hydromod table.
	CTable *GetpHydroModTable();

	// Fast access to the clipboard table.
	CTable *GetpClipboardTable();

	// Fast access to table where are saved HUB selections.
	CTable *GetpHUBSelectionTable();

	// Fast access to table where are saved the logging data.
	CTable *GetpLogDataTable();

	// Fast access to table where are saved the quick measurements.
	CTable *GetpQuickMeasureTable();

	// Copy first HM from the clipboard.
	IDPTR CopyFirstHMFromClipBoard( CTable *pTabTarget );

	// Copy Next HM from the clipboard, used with CopyFirstHMFromClipBoard.
	IDPTR CopyNextHMFromClipBoard( CTable *pTabTarget, bool bFirst = false );

	// Retrieve number of HM in the clipboard.
	int GetCountHMClipboard( void );

	// Return first HM from the clipboard.
	IDPTR GetFirstHMFromClipBoard();

	// Return Next HM from the clipboard, used with GetFirstHMFromClipBoard.
	IDPTR GetNextHMFromClipBoard( bool bFirst = false );

	// Verify in clipboard if there are HM and if yes return true if HM are only module.
	// Return: false if there are at least one circuit, return undef if clipboard doesn't contains HM.
	eBool3 IsClipBoardContainsOnlyHMModule();

	bool IsClipBoardEmpty( CDS_HydroMod **pHM = NULL );

	// Create a copy of a branch.
	void CopyTADSBranch( CTable *pHMFrom, CTable *pHMTo, bool fSetUID = true );

	// Compute Flow for this branch.
	void ComputeFlow( CTable *pTab, bool fFromTop = false );
	
	// Create a valve's name in function of his parent, if pTab=NULL differentiation is done on PIPING_TAB.
	void ComposeValveName( const IDPTR &IDPtr, CTable *pTab = NULL, CString strPrefix = _T("") );

	// If the automatic rename character is present, create a Root Module Name in function of his index.
	// If pHM == NULL, create a name with index value.
	void ComposeRMName( CString &str, CDS_HydroMod *pHM, int Index = -1 );
	
	bool DifferentiateHMName( std::multimap<double, CDS_HydroMod *> *pmap, CDS_HydroMod *pHM, CString *pStr, int &longNameIndex );
	void RenameBranch( CTableHM *pTab );
	void CollectHMList( CTable *pTab, std::multimap<double, CDS_HydroMod *> *pmap );

	// Compose a HM Name based on the parent Name or on a prefix.
	// Index characters will be composed with letters or figure.
	CString ComposeHUBHMName( CDS_HydroMod *pHM, TCHAR cStar = _T('*'), bool bAuto = true, CString strPrefix = _T(""), bool bNumber = true );
	
	// Find a Module or a valve with name matching str, skip object 'pHMToSkip' if exist.
	CDS_HydroMod *FindHydroMod( CString str, CTable *pTab, CDS_HydroMod *pHMToSkip = NULL, CDatastruct::FindMode eFindMode = CDatastruct::FindMode::Both );
	
	// Find a Module or a valve with the correct uid.
	// Pay attention: here it's the unique identifier in the 'CDS_HydroMod' class and not 
	CDS_HydroMod *FindHydroMod( unsigned short uid, CTable *pTab );

	// Replace HydroMod uid with a new one.
	void SetHMUid( CTable *pTab );

	// Reset HydroMod level in function of its real position.
	void SetHMLevel( CTable *pTab, int level = -1 );
	
	// Check in PIPING_TAB, TMPHUB_TAB, SELHUBTAB_TAB if a name is already used.
	// Return: table where the name has been found
	CTable *VerifyIfNameAlreadyExist( CString sName, CDS_HydroMod *pCurrentHM );

	////////////////////////////////////////////////////////////////////////////////
	// str will be modified by making the last three characters to be
	// "%AAA" where AAA is incremented until a non-existing name is found
	// Limited to 3 capital alphabetical TCHAR 26 * 26 * 26-> 17576 variants !!!
	// return the new composed string; if a new name can't be found return false and str =""
	bool DifferentiateHMName( CTable *pTab, CString &str );
	
	// Set Row Index for each object in Table TabID used in SelP.
	void SetIndex( CString TabID );

	// Delete a selection from the TADS.
	void DeleteSelectionFromTADS( IDPTR SelectionIDPtr );
	
	// ????
	void DeleteValveFromTADS( CDS_HydroMod *pHM );
	
	// Move a hydromod to another position in the installation connect it to a PV if necessary.
	// Return: false if the user clicked on cancel
	unsigned short MoveHydroMod( CDS_HydroMod *pHMFrom, CTable *pHMTo, bool fKeepUid = false );

	// Compute Each root module.
	bool ComputeAllInstallation();
	
	// Verify content of TADS to see if there is something to print.
	bool IsSomethingToPrint();

	// Check if there is at least one individual selection done.
	bool IsIndividualSelectionExist( void );
	
	bool SaveToTADataStructX( CTADataStructX *pTADSX );

	// HYS-1590: Start to centralyze the writing in the registry in one place.
	void WriteAllDataInRegistry( void );

	//************************************
	// Method:    CreateNewHM
	// FullName:  CTADatastruct::CreateNewHM
	// Access:    public 
	// Returns:   CDS_HydroMod *
	// Qualifier: Create a new HM and insert it into "InsertIn" table
	// Parameter: CTable * pInsertIn, table where the new created HM will be inserted
	// Parameter: CDB_CircuitScheme * pSch, Type of circuit attached to this HM
	// Parameter: CDB_ControlProperties::eCvCtrlType CtrlType, On/Off or proportional
	// Parameter: bool fTaCV, use a CV from TA
	// Parameter: bool fTU, use with terminal unit
	// Parameter; CDS_HydroMod::ReturnType, use to tell if we are working in direct or reverse return mode.
	//************************************
	CDS_HydroMod *CreateNewHM( CTable *pInsertIn, CDB_CircuitScheme *pSch, CDB_ControlProperties::CvCtrlType CtrlType, bool fTaCV, bool fTU, int iReturnType = 0 /*, CArray<CAnchorPt::eFunc> *paFunc = NULL */);

	//************************************
	// Method:    CopyCommonHMData
	// FullName:  CTADatastruct::CopyCommonHMData
	// Access:    public 
	// Returns:   void
	// Qualifier: Will copy all common data from src HM to the Trg; usefull when the user change HM type PC2 or search and replace
	// Parameter: CDS_HydroMod * pHMSrc
	// Parameter: CDS_HydroMod * pHMTo
	//************************************
	void CopyCommonHMData( CDS_HydroMod *pHMSrc, CDS_HydroMod *pHMTo );
	
	void VerifyAllRootModules();
	void VerifyInstallation( CTable* pTab, bool fAddCurrentHM = true );
	void _SendValidityHMErrMessage( CDS_HydroMod *pHM, CDB_TAProduct *pTAP, int ValidityFlag );
	
	// HYS-1207: All updateWC() called in Read methods are deleted and moved here. Because we want to be sure
	// that the project is completly readed.
	void RefreshAllSecondaryWCForHMCalc( CTable *pclTable = NULL );

// Private members.
private:
	// HYS-1590: Allow to run all pipes reference in a project (In a CTADatastruct).
	typedef void (CTADatastruct::*pfRunAllPipesFeedBack)( IDPTR &, LPARAM );
	void _RunAllPipesInTheDatastruct( pfRunAllPipesFeedBack fOnePipeFeedBack, LPARAM lpFeedBackParam );

	// HYS-1590: Feedback method for the "AddUsedUserPipesBeforeSavingProject" method.
	void _CheckIfUserPipeSeriesToAdd( IDPTR &IDPtr, LPARAM lpFeedBackParam );

	// HYS-1590: Feedback method for the "VerifyPipeSeriesWhenOpeningProject" method.
	// No need of 'lpParam' for this feedback method.
	void _UpdateOneOldPipeID( IDPTR &IDPtr, LPARAM lpParam );

protected:
	bool m_bCheckAllModules;
	std::multimap<double, CDS_HydroMod *> m_mapHMList;
#endif
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_WaterCharacteristic
//
///////////////////////////////////////////////////////////////////////////////////

class CWaterChar
{
public:
	enum eFluidRetCode
	{
		efrcFluidUnknown,
		efrcFluidOk,
		efrcTemperatureTooLow,
		efrcTempTooHigh,
		efrcAdditiveTooHigh,
		efrcLast
	};

public:
	CWaterChar();
	CWaterChar( const CWaterChar &WaterCharCopy );
	~CWaterChar();

	LPCTSTR GetAdditFamID() { return m_AdditFamID; }
	LPCTSTR GetAdditID() { return m_AdditID; }
	CDB_AdditCharacteristic *GetpAdditChar();
	double GetTemp() { return m_dTemp; }
	double GetPcWeight() { return m_dPcWeight; }
	
	// Get density, kinematic viscosity and specific heat at the current temperature.
	double GetDens() { return m_dDens; }
	double GetKinVisc() { return m_dKinVisc; }
	double GetSpecifHeat() { return m_dSpecifHeat; }

	// Get density, kinematic viscosity and specific heat for a defined temperature.
	double GetDens( double dTemperature );
	double GetKinVisc( double dTemperature );
	double GetSpecifHeat( double dTemperature );
	
	double GetTfreez() { return m_dTfreez; }
	bool GetCheckSpecifHeat() { return m_bSpecifHeat; }
	
	// Get expansion factor between two temperatures 
	double GetExpansionCoeff( double dT1, double dT2 );
	
	// Vapor pressure TODO.
	double GetVaporPressure( double dT1 );
	bool GetIsForChangeOver() { return m_bIsForChangeOver; }
	CWaterChar *GetSecondWaterChar() { return m_pclSecondWaterChar; }

	bool SetAdditFamID( LPCTSTR ID );
	bool SetAdditID( LPCTSTR ID );
	bool SetTemp( double dTemperature );
	bool SetPcWeight( double dPCWeight );
	bool SetDens( double dDensity );
	bool SetKinVisc( double dKinematicViscosity );
	bool SetSpecifHeat( double dSpecificHeat );
	bool SetTfreez( double dFrozenTemperature );
	bool SetCheckSpecifHeat( bool bSpecificHeat );
	void SetIsForChangeOver( bool bForChangeOver );
	void BuildWaterStrings( _string &str1, _string &str2, const TCHAR *ptcWaterChar, const TCHAR *ptcFluid, const TCHAR *ptcFluidWeight, const TCHAR *ptcTemperature, bool bfull = true );
	void BuildWaterNameString( _string &str1, const TCHAR *ptcWaterChar, const TCHAR *ptcFluid, const TCHAR *ptcFluidWeight, bool bfull = true );

#ifndef TACBX 
	void BuildWaterStrings( CString &str1, CString &str2 );
	void BuildWaterStringsRibbonBar( CString &str1, CString &str2, CString &str3 );
	void BuildTempChangeOverStringsRibbonBar( CString &str1, CString &str2 );
	void BuildWaterNameString( CString &str1, bool bFull = true );
#endif

	bool IsDensityDefined();
	bool IsKinematicViscosityDefined();
	bool IsSpecificHeatDefined();
	bool IsVaporPressureDefined();

	CWaterChar &operator=( const CWaterChar &wc );
	bool operator!=( CWaterChar &wc );
	bool operator==( CWaterChar &wc );
	CWaterChar::eFluidRetCode CheckFluidData( double &dTemp, double *pdPCWeight = NULL );

	// Make a comparison, if bWithTemp = false don't compare fluid temperature.
	bool Compare( CWaterChar& wc, bool bWithTemp/*=true*/ );
	
	// Disk IO.
	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf, bool bOldFile = false);	// flag bOldFle used to keep compatibility with previous CDS_WaterCharacteristic
	
	// CAUTION : UpdateFluidData do not change m_AdditFamID ot m_AdditID
	bool UpdateFluidData( CDB_AdditCharacteristic *pAdditChar, double dTemp, double dPCWeight = -1.0 );
	bool UpdateFluidData( double dTemp, double dPCWeight = -1.0 );
	void InitChangeOverData();

// Protected variables.
protected:
	TCHAR m_AdditFamID[ _ID_LENGTH + 1 ];	// Additive family ID
	TCHAR m_AdditID[ _ID_LENGTH + 1 ];	// Additive ID
	double m_dTemp;					// Temperature
	double m_dPcWeight;				// % in weight of the additive in the mixture
	double m_dDens;					// Density
	double m_dKinVisc;				// Kinematic viscosity
	double m_dSpecifHeat;			// Specific heat
	double m_dTfreez;				// Freezing temperature
	bool m_bSpecifHeat;				// Specific heat specified or not for "Any Other Fluid"
	bool m_bIsForChangeOver;		// Need more data for change-over circuit
	CWaterChar *m_pclSecondWaterChar;	// To save more data for change-over circuit
};


class CDS_WaterCharacteristic : public CData
{
	DEFINE_DATA(CDS_WaterCharacteristic)

// Attribute
public:
	void SetAdditFamID( LPCTSTR ID ){ if( m_WC.SetAdditFamID( ID ) ) { Modified(); } }
	void SetAdditID( LPCTSTR ID ) { if( m_WC.SetAdditID( ID ) ) { Modified(); } }
	void SetTemp( double dTemp ) { if( m_WC.SetTemp( dTemp) ) { Modified(); } }
	void SetPcWeight( double dPCWeight ) { if( m_WC.SetPcWeight( dPCWeight ) ) { Modified(); } }
	void SetDens( double dDensity ) { if( m_WC.SetDens( dDensity ) ) { Modified(); } }
	void SetKinVisc( double dKinVisc ) { if( m_WC.SetKinVisc( dKinVisc ) ) { Modified(); } }
	void SetSpecifHeat( double dSpecifHeat ) { if( m_WC.SetSpecifHeat( dSpecifHeat ) ) { Modified(); } }
	void SetTfreez( double dTempFreeze ) { if( m_WC.SetTfreez( dTempFreeze ) ) { Modified(); } }
	void SetCheckSpecifHeat( bool bSpecifHeat ) { if( m_WC.SetCheckSpecifHeat( bSpecifHeat ) ) { Modified(); } }

	LPCTSTR GetAdditFamID() { return m_WC.GetAdditFamID(); }
	LPCTSTR GetAdditID() { return m_WC.GetAdditID(); } 		
	CDB_AdditCharacteristic *GetpAdditChar();
	double GetTemp() { return m_WC.GetTemp(); } 		
	double GetPcWeight() { return m_WC.GetPcWeight(); } 	
	double GetDens() { return m_WC.GetDens(); } 		
	double GetKinVisc() { return m_WC.GetKinVisc(); } 		
	double GetSpecifHeat() { return m_WC.GetSpecifHeat(); } 	
	double GetTfreez() { return m_WC.GetTfreez(); } 		
	bool GetCheckSpecifHeat(){ return m_WC.GetCheckSpecifHeat(); }

	// After a change of temperature or Pc weight,
	// this function re-compute the density, the viscosity, etc.
	// Remark : This function is not applicable for "OTHER_ADDIT"
	bool UpdateFluidData(double dTemp, double dPcWeight = -1.0);

#ifndef TACBX 
	// Build water characteristic strings for use in the SSel and Proj tabs
	void BuildWaterStrings( CString &str1, CString &str2 ) { m_WC.BuildWaterStrings( str1, str2 ); }
	
	// Build water characteristic strings for Ribbon used in the Project Selection and Project tabs
	void BuildWaterStringsRibbonBar( CString &str1, CString &str2, CString &str3){ m_WC.BuildWaterStringsRibbonBar( str1, str2, str3 ); }
	void BuildTempChangeOverStringsRibbonBar( CString& str1, CString& str2 ) { m_WC.BuildTempChangeOverStringsRibbonBar( str1, str2 ); }
#endif	

// Constructor and destructor.
protected:
	CDS_WaterCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_WaterCharacteristic() {}

// Public methods.
public:
	CWaterChar *GetpWCData() { return &m_WC; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CWaterChar m_WC;
};	

	
///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_TechnicalParameter
//
///////////////////////////////////////////////////////////////////////////////////
enum PressurON
{
	poPumpSuction,
	poPumpDischarge,
	poLast
};

struct TechParam_struct
{
	double m_dValveMinDp;							// Max. min. DP in valves
	double m_dValveMaxDp;							// Max. DP in valves
	double m_dValveMin2Dp;							// Max. min. DP in valves
	double m_dFixedOrificeMinDp;					// Min. DP in FO
	double m_dFixedOrificeMaxDp;					// Max. DP in FO
	double m_dVenturiMinDp;							// Min. DP in Venturi
	double m_dVenturiMaxDp;							// Max. DP in Venturi
	int m_iSizeShiftBelow;							// Shift between pipe and device sizes
	int m_iSizeShiftAbove;							// Shift between pipe and device sizes
	int m_iMaxSizeShift;							// Maximum size shift above and below the pipe size
	double m_dPipeMinDp;							// Min. DP in pipes
	double m_dPipeMaxDp;							// Max. DP in pipes
	double m_dPipeMinVelocity;						// Min. velocity in pipes
	double m_dPipeMaxVelocity;						// Max. velocity in pipes
	double m_dPipeTargetDp;							// Target DP in pipes
	double m_dPipeTargetVelocity;					// Target velocity in pipes
	int m_iPipeTargetVelosityUsed;					// 0/1: 1 if TargVel is used to determine the best pipe
	double m_dTrvMinDpRatio;						// Min. DP ratio to be taken in Trv
	double m_dTrvDefDpTotal;						// Default available diff. press. on rad. circuits
	double m_dTrvDefRoomTemperature;				// Default room temperature for radiator sizing
	double m_dTrvDefSupplyTemperature;				// Default supply temperature for radiator sizing
	double m_dTrvMaxDp;								// Max. DP in Trv's
	double m_dCVMinAuthor;							// Min. authority for control valves
	double m_dCVMinAuthCstFlow;						// Min. authority for control valves in cst flow systems
	double m_dCVMinDpOnoff;							// Min. DP in on-off control valves
	double m_dCVMinDpProp;							// Min. DP in proportional control valves
	int m_iCVDefKvReynard;							// 0/1: 1 if Kvs values of control valves are by default selected from Reynard series
	int m_iCVUseDpCPBand;							// 0/1: 1 if use PB of DpC when computing CV min authority
	double m_dPICvMinDp;							// Min. Dp in the Dp control part
	int m_iPICvHide;								// 0/1: 1 to hide PICv that do not satisfy the Min. Dp limit
	double m_dDpCMinDp;								// Min. Dp in the DpC
	int m_iDpCHide;									// 0/1: 1 to hide DpC that do not satisfy the Min. Dp limit
	int m_iDpCMvLocation;							// Default measuring valve location 0/1: MV in primary/MV in secondary
	int m_iDpCMvWithSameSizeOnly;					// Suggest measuring valves with the same size as Dp controller only
	int m_iDpCHminFullyOpenMv;						// 0/1: 1 if Hmin must be computed with Dp of fully open Mv
	double m_dFlowDeviationPlus;					// 0 to 1: 
	double m_dFlowDeviationMinus;					// 0 to 1:
	_string m_strCurrencyID;						// ID of the selected currency
	_string m_strDefaultPipeSerieID;				// ID of the default pipe series
	double m_dCVMaxDispDp;							// Max Dp for control valve; used for displaying warning
	double m_dCVMaxDpProp;							// Max Dp for proportional control valve
	double m_dDpCBCVDpcmin;							// Minimum Dp that a control part of a DpC BCV valve must have to correctly work.
	double m_dDpCBCVDplmin;							// Minimum Dp that a DpC BCV valve can stabilize.
	double m_dVesselSelectionRangePercentage;		// We accept the selection of a smaller vessel if it's in a percentage range
	
	double m_dMinPressureMargin;					// HYS-1022: Change the name of variable and methods to be synchronized with HyTools.
													// 0.2 bar margin over static height as per EN 12828-2014

	double m_dSafetyTempLimiterLimitEN12953;		// HYS-1104: safety temperature limiter limit from which we need to pass to the EN 12953 norm.
	double m_dFluidTempLimitEN12953;				// HYS-1104: supply/return temperature limit from which we need to pass to the EN 12953 norm.

	int m_iMaxNumberOfAdditionalVesselsInParallel;
	bool m_fUseVesselPressureVolumeLimit;			// 0/1: 1 if we must use a pressure.volume limit (as defined in SWKI HE301-01 for Switzerland)
													//		for the vessel selection (for example in SWKI HE301-01 -> 3000bars.liter).
													//      In Switzerland, we have no choice, we must apply this rule!
	double m_dPressureVolumeLimit;					// Get the value of the pressure volume limit (as defined in 'localdb.txt').
	bool m_bCanChangePressureVolumeLimit;			// 0/1: 0 in some country (Switzerland) the value is fixed. Some other countries can change.
	int m_iMaxNumberOfVentoInParallel;
	double m_dPSetupCompresso;
	double m_dPSetupTransfero;
	double m_dDefaultFillTemperature;
	bool m_fUseDefaultFreezingPointAsMinTemperature;
	double m_dDefaultWaterMakeUpTemp;
	double m_dDefaultMaxTempInCooling;
	double m_dDefaultMinTempInHeating;
	double m_dDefaultMinTempInSolar;
	double m_dDefaultSafetyPressureResponseValve;
	double m_dDefaultTAZ;
	_string m_sDefaultCalculationNorm;
	bool m_bBreakTankRequiredForWaterMakeUp;
	double m_dDefaultStaticPressureOfWaterNetwork;
	int m_iAvailablePMPanels;						// Heating 1 / Cooling 2 / Solar 4
	double m_dDefaultWaterHardness;
	double m_dDegassingWaterReserveForVessel;		// Additional water volume to add in the vessel when a degassing device is added in the system.

	ProjectType m_eProjectType;						// Heating or Cooling
	double m_dDefaultPrjCoolingTps;					// Default Tps used for the project
	double m_dDefaultPrjHeatingTps;					// Default Tps used for the project
	double m_dDefaultPrjCoolingDT;					// Default DT used for the project
	double m_dDefaultPrjHeatingDT;					// Default DT used for the project
	_string m_strDefaultPrjCoolingAdditID;			// Default Additive used for the project
	_string m_strDefaultPrjHeatingAdditID;			// Default Additive used for the project
	double m_dDefaultPrjCoolingPcWeight;			// Default Pc Weight used for the project
	double m_dDefaultPrjHeatingPcWeight;			// Default Pc Weight used for the project

	ProjectType m_eProductSelectionApplicationType;	// Heating, cooling or solar.

	// Version 36: HYS-1230: Default product selection water characteristic by project type.
	std::map<ProjectType, CWaterChar> m_mapPSDefaultWaterCharByProject;

	// Version 36: HYS-1230: Default product selection DT by project type.
	std::multimap<ProjectType, double> m_mmapPSDefaultDTByProject;

	int m_iMaxSafetyValveInParallel;				// Version 31: Max number of safety valve in parallel.

	double m_dInitialPressureMargin;				// HYS-1022: 0.3 bar margin for initial pressure (EN 12828-2014).

	// Working variable, not saved.
	double m_dPICVExtraThresholdDp;					// Help	to select smaller PICV that have the smallest Dp
	BYTE m_bVersion;

	bool m_bUseSimplyPst;							// HYS-1126 : 0/1 : 1 if we use the simplified calculation of Pst ( Hst / 10 ).
	double m_dDpRefForBest6Way;						// HYS-1151: The reference value used to determine the best 6-way valve at a flow given.
	int m_iDefaultSmartCVLoc;						// HYS-1763: Smart control valve default location in hydronic circuit calculation: 1 Supply side / 0 Return side.

	double m_dSmartValveDpMaxForBestSuggestion;		// Dp max for "best" suggestion.
	double m_dDefaultSecondaryPumpHMin;				// HYS-1930: New hydronic circuit: auto-adapting variable flow decoupling circuit: add default secondary pump Hmin.
};

class CDS_TechnicalParameter : public CData
{
	DEFINE_DATA(CDS_TechnicalParameter)

protected:
	CDS_TechnicalParameter( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_TechnicalParameter() {}

public:
	// Init() this function is public only for test purpose, hardcoded value for tech params are overwritten when the application start by user value stored in the registry
	void Init();
	double GetValvMinDp( _string strValveTypeID );
	double GetValvMinDp() { return m_rTechParamData.m_dValveMinDp; }
	// HYS-2024: ValvMin2Dp = ValvMinDp + offset (3000 kPa)
	double GetValvMin2Dp() { return m_rTechParamData.m_dValveMin2Dp; }

	double GetValvMaxDp() { return m_rTechParamData.m_dValveMaxDp; }

	double GetFoMinDp() { return m_rTechParamData.m_dFixedOrificeMinDp; }
	double GetFoMaxDp() { return m_rTechParamData.m_dFixedOrificeMaxDp; }
	double GetVtriMinDp() { return m_rTechParamData.m_dVenturiMinDp; }
	double GetVtriMaxDp() { return m_rTechParamData.m_dVenturiMaxDp; }

	double VerifyValvMaxDp( CDB_TAProduct *pTap );

	int GetSizeShiftAbove() { return m_rTechParamData.m_iSizeShiftAbove; }
	int GetSizeShiftBelow() { return m_rTechParamData.m_iSizeShiftBelow; }
	int GetMaxSizeShift() { return m_rTechParamData.m_iMaxSizeShift; }
	double GetPipeMinDp() { return m_rTechParamData.m_dPipeMinDp; }
	double GetPipeMaxDp() { return m_rTechParamData.m_dPipeMaxDp; }
	double GetPipeMinVel() { return m_rTechParamData.m_dPipeMinVelocity; }
	double GetPipeMaxVel() { return m_rTechParamData.m_dPipeMaxVelocity; }
	double GetPipeTargDp() { return m_rTechParamData.m_dPipeTargetDp; }
	double GetPipeTargVel() { return m_rTechParamData.m_dPipeTargetVelocity; }
	int GetPipeVtargUsed() { return m_rTechParamData.m_iPipeTargetVelosityUsed; }

	double GetTrvMinDpRatio() { return m_rTechParamData.m_dTrvMinDpRatio; }
	double GetTrvDefDpTot() { return m_rTechParamData.m_dTrvDefDpTotal; }
	double GetTrvDefRoomT() { return m_rTechParamData.m_dTrvDefRoomTemperature; }
	double GetTrvDefSupplyT() { return m_rTechParamData.m_dTrvDefSupplyTemperature; }
	double GetTrvMaxDp() { return m_rTechParamData.m_dTrvMaxDp; }

	double GetCVMinAuthor() { return m_rTechParamData.m_dCVMinAuthor; }
	double GetCVMinAuthCstFlow() { return m_rTechParamData.m_dCVMinAuthCstFlow; }
	double GetCVMinDpOnoff() { return m_rTechParamData.m_dCVMinDpOnoff; }
	double GetCVMinDpProp() { return m_rTechParamData.m_dCVMinDpProp; }
	double GetCVMaxDpProp() { return m_rTechParamData.m_dCVMaxDpProp; }
	double GetCVMaxDispDp() { return m_rTechParamData.m_dCVMaxDispDp; }
	int GetCVDefKvReynard() { return m_rTechParamData.m_iCVDefKvReynard; }
	int GetCVUseDpCBand() { return m_rTechParamData.m_iCVUseDpCPBand; }

	double GetPICvMinDp() { return m_rTechParamData.m_dPICvMinDp; }
	int GetPICvHide() { return m_rTechParamData.m_iPICvHide; }

	double GetDpCMinDp() { return m_rTechParamData.m_dDpCMinDp; }
	int GetDpCHide() { return m_rTechParamData.m_iDpCHide; }
	int GetDpCMvWithSameSizeOnly() { return m_rTechParamData.m_iDpCMvWithSameSizeOnly; }
	int GetDpCMvLoc() { return m_rTechParamData.m_iDpCMvLocation; }

	double GetDpCBCVDpcmin() { return m_rTechParamData.m_dDpCBCVDpcmin; }
	double GetDpCBCVDplmin() { return m_rTechParamData.m_dDpCBCVDplmin; }

	double GetVesselSelectionRangePercentage() { return m_rTechParamData.m_dVesselSelectionRangePercentage; }

	double GetFlowDeviationMinus() { return m_rTechParamData.m_dFlowDeviationMinus; }
	double GetFlowDeviationPlus() { return m_rTechParamData.m_dFlowDeviationPlus; }
	int GetDpCHminFullyOpenMv() { return m_rTechParamData.m_iDpCHminFullyOpenMv; }
	IDPTR GetDefaultPipeSerieIDPtr();

	// Working variables.
	double GetPICVExtraThresholdDp() { return m_rTechParamData.m_dPICVExtraThresholdDp; }
	BYTE GetVersion() { return m_rTechParamData.m_bVersion; }

#ifndef TACBX
	CDB_Currency *GetCurrentCurrencyDB( void );
	_string GetCurrentCurrencyISO( void );
	LPCTSTR GetCurrencyID() { return m_rTechParamData.m_strCurrencyID.c_str(); }
	LPCTSTR GetDefaultPipeSerieID() { return m_rTechParamData.m_strDefaultPipeSerieID.c_str(); }
#endif	
	
	ProjectType GetProjectApplicationType() const { return m_rTechParamData.m_eProjectType; }
	
	double GetDefaultPrjDT() { if( Cooling == m_rTechParamData.m_eProjectType ) return m_rTechParamData.m_dDefaultPrjCoolingDT; else return m_rTechParamData.m_dDefaultPrjHeatingDT; }
	double GetDefaultPrjTps() { if( Cooling == m_rTechParamData.m_eProjectType ) return m_rTechParamData.m_dDefaultPrjCoolingTps; else return m_rTechParamData.m_dDefaultPrjHeatingTps; }
	double GetDefaultPrjTpr();
	LPCTSTR GetDefaultPrjAdditID() { if(Cooling == m_rTechParamData.m_eProjectType) return m_rTechParamData.m_strDefaultPrjCoolingAdditID.c_str(); else return m_rTechParamData.m_strDefaultPrjHeatingAdditID.c_str(); }
	double GetDefaultPrjPcWeight() { if(Cooling == m_rTechParamData.m_eProjectType) return m_rTechParamData.m_dDefaultPrjCoolingPcWeight; else return m_rTechParamData.m_dDefaultPrjHeatingPcWeight; }

	double GetDefaultPrjHeatingTps() { return m_rTechParamData.m_dDefaultPrjHeatingTps; }
	double GetDefaultPrjHeatingTpr();
	double GetDefaultPrjHeatingDT() { return m_rTechParamData.m_dDefaultPrjHeatingDT; }
	LPCTSTR GetDefaultPrjHeatingAdditID() { return m_rTechParamData.m_strDefaultPrjHeatingAdditID.c_str(); }
	double GetDefaultPrjHeatingPcWeight() { return m_rTechParamData.m_dDefaultPrjHeatingPcWeight; }

	double GetDefaultPrjCoolingTps() { return m_rTechParamData.m_dDefaultPrjCoolingTps; }
	double GetDefaultPrjCoolingTpr();
	double GetDefaultPrjCoolingDT() { return m_rTechParamData.m_dDefaultPrjCoolingDT; }
	LPCTSTR GetDefaultPrjCoolingAdditID() { return m_rTechParamData.m_strDefaultPrjCoolingAdditID.c_str(); }
	double GetDefaultPrjCoolingPcWeight() { return m_rTechParamData.m_dDefaultPrjCoolingPcWeight; }

#ifndef TACBX
	int GetProductSelectionApplicationTypeIDS( ProjectType eApplicationType );
#endif

	ProjectType GetProductSelectionApplicationType() const { return m_rTechParamData.m_eProductSelectionApplicationType; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1230: Complete review of the getter/setter for water characteristic for product selection.
	// Default supply temperature for product selection.

	// In regards to the current application type defined in the technical parameters, this method will return the default supply temperature.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	double GetDefaultISTps( SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	double GetDefaultISTps( ProjectType eApplicationType, SideDefinition eSideDefinition = CoolingSide );

	// Returns the default supply temperature for heating mode.
	double GetDefaultISHeatingTps();

	// Returns the default supply temperature for cooling mode.
	double GetDefaultISCoolingTps();

	// Returns the default supply temperature for solar mode.
	double GetDefaultISSolarTps();

	// Returns the default supply temperature for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	double GetDefaultISChangeOverTps( SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Default DT for product selection.

	// In regards to the current application type defined in the technical parameters, this method will return the default DT.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	double GetDefaultISDT( SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	double GetDefaultISDT( ProjectType eApplicationType, SideDefinition eSideDefinition = CoolingSide );

	// Returns the default delta T for the heating and solar mode.
	double GetDefaultISHeatingDT();
	
	// Returns the default delta T for the cooling mode.
	double GetDefaultISCoolingDT();

	// Returns the default delta T for the solar mode.
	double GetDefaultISSolarDT();

	// Returns the default delta T for the change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	double GetDefaultISChangeOverDT( SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Default return temperature for product selection.

	// In regards to the current application type defined in the technical parameters, this method will return the default return temperature.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	double GetDefaultISTpr( SideDefinition eSideDefinition = Undefined );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	double GetDefaultISTpr( ProjectType eApplicationType, SideDefinition eSideDefinition = Undefined );

	// Returns the default return temperature for heating mode.
	double GetDefaultISHeatingTpr();

	// Returns the default return temperature for cooling mode.
	double GetDefaultISCoolingTpr();

	// Returns the default return temperature for solar mode.
	double GetDefaultISSolarTpr();

	// Returns the default return temperature for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	double GetDefaultISChangeOverTpr( SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Default return temperature for product selection.

	// In regards to the current application type defined in the technical parameters, this method will return the default additive ID.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	LPCTSTR GetDefaultISAdditID( SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	LPCTSTR GetDefaultISAdditID( ProjectType eApplicationType, SideDefinition eSideDefinition = CoolingSide );

	// Returns the default return temperature for heating mode.
	LPCTSTR GetDefaultISHeatingAdditID();

	// Returns the default return temperature for cooling mode.
	LPCTSTR GetDefaultISCoolingAdditID();

	// Returns the default return temperature for solar mode.
	LPCTSTR GetDefaultISSolarAdditID();

	// Returns the default return temperature for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	LPCTSTR GetDefaultISChangeOverAdditID( SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Default weight of the additive for product selection.

	// In regards to the current application type defined in the technical parameters, this method will return the default weight of the additive.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	double GetDefaultISPcWeight( SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	double GetDefaultISPcWeight( ProjectType eApplicationType, SideDefinition eSideDefinition = CoolingSide );
	
	// Returns the default weight of the additive for heating mode.
	double GetDefaultISHeatingPcWeight();

	// Returns the default weight of the additive for cooling mode.
	double GetDefaultISCoolingPcWeight();

	// Returns the default weight of the additive for solar mode.
	double GetDefaultISSolarPcWeight();

	// Returns the default weight for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	double GetDefaultISChangeOverPcWeight( SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Default water characteristic for product selection.

	// In regards to the current application type defined in the technical parameters, this method will return the default water characteristic.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	CWaterChar *GetDefaultISWC( SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	CWaterChar *GetDefaultISWC( ProjectType eApplicationType, SideDefinition eSideDefinition = Undefined );
	
	// Returns the default water characteristic for heating mode.
	CWaterChar *GetDefaultISHeatingWC();

	// Returns the default water characteristic for cooling mode.
	CWaterChar *GetDefaultISCoolingWC();

	// Returns the default water characteristic for solar mode.
	CWaterChar *GetDefaultISSolarWC();

	// Returns the default water characteristic for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	CWaterChar *GetDefaultISChangeOverWC( SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TACBX
	int GetPressurONIDS( PressurON ePressOn );
#endif

	// For pressure maintenance.
	// HYS-1022: Change the name of variable and methods to be synchronized with HyTools.
	// 0.2 bar margin over static height as per EN 12828-2014.
	double GetMinPressureMargin() const { return m_rTechParamData.m_dMinPressureMargin; }

	int GetMaxNumberOfAdditionalVesselsInParallel() const { return m_rTechParamData.m_iMaxNumberOfAdditionalVesselsInParallel; }
	bool GetUseVesselPressureVolumeLimit() const { return m_rTechParamData.m_fUseVesselPressureVolumeLimit; }
	double GetPressureVolumeLimit() const { return m_rTechParamData.m_dPressureVolumeLimit; }
	_string GetPressureVolumeLimitCU() const;

	// HYS-1126 : Simplify Pst
	bool GetUseSimplyPst() const { return m_rTechParamData.m_bUseSimplyPst; }

	bool GetCanChangePressureVolumeLimit() const { return m_rTechParamData.m_bCanChangePressureVolumeLimit; }
	double GetPSetupCompresso() const { return m_rTechParamData.m_dPSetupCompresso; }
	double GetPSetupTransfero()  const { return m_rTechParamData.m_dPSetupTransfero; }
	double GetDefaultFillTemperature() const { return m_rTechParamData.m_dDefaultFillTemperature; }
	bool GetUseDefaultFreezingPointAsMinTemperature() { return m_rTechParamData.m_fUseDefaultFreezingPointAsMinTemperature; }
	int GetMaxNumberOfVentoInParallel() const { return m_rTechParamData.m_iMaxNumberOfVentoInParallel; }

	double GetDefaultWaterMakeUpTemp() const{ return m_rTechParamData.m_dDefaultWaterMakeUpTemp; }
	double GetDefaultMaxTempInCooling() const{ return m_rTechParamData.m_dDefaultMaxTempInCooling; }
	double GetDefaultMinTempInHeating() const{ return m_rTechParamData.m_dDefaultMinTempInHeating; }
	double GetDefaultMinTempInSolar() const{ return m_rTechParamData.m_dDefaultMinTempInSolar; }
	double GetDefaultSafetyPressureResponseValve() const{ return m_rTechParamData.m_dDefaultSafetyPressureResponseValve; }
	double GetDefaultTAZ() const{ return m_rTechParamData.m_dDefaultTAZ; }
	_string GetDefaultCalculationNorm() const{ return m_rTechParamData.m_sDefaultCalculationNorm; }
	bool GetBreakTankRequiredForWaterMakeUp() const{ return m_rTechParamData.m_bBreakTankRequiredForWaterMakeUp; }
	double GetDefaultStaticPressureOfWaterNetwork() const{ return m_rTechParamData.m_dDefaultStaticPressureOfWaterNetwork; }
	int GetAvailablePMPanels() const{ return m_rTechParamData.m_iAvailablePMPanels; }
	double GetDefaultWaterHardness() const{ return m_rTechParamData.m_dDefaultWaterHardness; }
	double GetDegassingWaterRerserveForVessel()	const{ return 0.002000000000000000042; } //m_rTechParamData.m_dDegassingWaterReserveForVessel; }

	int GetMaxSafetyValveInParallel() const { return m_rTechParamData.m_iMaxSafetyValveInParallel; }

	// HYS-1022: 0.3 bar margin for initial pressure (EN 12828-2014).
	double GetInitialPressureMargin() const { return m_rTechParamData.m_dInitialPressureMargin; }

	// HYS-1104: safety temperature limiter limit from which we need to pass to the EN 12953 norm.
	double GetSafetyTempLimiterLimitEN12953() const { return m_rTechParamData.m_dSafetyTempLimiterLimitEN12953; }
	
	// HYS-1104: supply/return temperature limit from which we need to pass to the EN 12953 norm.
	double GetFluidTempLimitEN12953() const { return m_rTechParamData.m_dFluidTempLimitEN12953; }

	// HYS-1151: The reference value used to determine the best 6-way valve at a flow given.
	double GetDpRefForBest6Way() const { return m_rTechParamData.m_dDpRefForBest6Way; }
	
	// HYS-1763: Get default smart control valve location
	int GetDefaultSmartCVLoc() const { return m_rTechParamData.m_iDefaultSmartCVLoc; }

	double GetSmartValveDpMaxForBestSuggestion() const { return m_rTechParamData.m_dSmartValveDpMaxForBestSuggestion; }

	// HYS-1930: New hydronic circuit: auto-adapting variable flow decoupling circuit: add default secondary pump Hmin.
	double GetDefaultSecondaryPumpHMin() const { return m_rTechParamData.m_dDefaultSecondaryPumpHMin; }

	TechParam_struct *GetTechParamStructure( void ) { return &m_rTechParamData; }

	void SetValvMinDp( double dValveMinDp ) { m_rTechParamData.m_dValveMinDp = dValveMinDp; }
	void SetValvMin2Dp( double dValveMin2Dp ) { m_rTechParamData.m_dValveMin2Dp = dValveMin2Dp; }
	void SetValvMaxDp( double dValveMaxDp ) { m_rTechParamData.m_dValveMaxDp = dValveMaxDp; }

	void SetFoMinDp( double dFixedOrificeMinDp ) { m_rTechParamData.m_dFixedOrificeMinDp = dFixedOrificeMinDp; }
	void SetFoMaxDp( double dFixedOrificeMaxD ) { m_rTechParamData.m_dFixedOrificeMaxDp = dFixedOrificeMaxD; }

	void SetVtriMinDp( double dVenturiMinDp ) { m_rTechParamData.m_dVenturiMinDp = dVenturiMinDp; }
	void SetVtriMaxDp( double dVenturiMaxDp ) { m_rTechParamData.m_dVenturiMaxDp = dVenturiMaxDp; }

	void SetSizeShiftAbove( int iSizeShiftAbove ) { m_rTechParamData.m_iSizeShiftAbove = iSizeShiftAbove; }
	void SetSizeShiftBelow( int iSizeShiftBelow ) { m_rTechParamData.m_iSizeShiftBelow = iSizeShiftBelow; }
	void SetMaxSizeShift( int iMaxSizeShift ) { m_rTechParamData.m_iMaxSizeShift = iMaxSizeShift; }
	void SetPipeMinDp( double dPipeMinDp ) { m_rTechParamData.m_dPipeMinDp = dPipeMinDp; }
	void SetPipeMaxDp( double dPipeMaxDp ) { m_rTechParamData.m_dPipeMaxDp = dPipeMaxDp; }
	void SetPipeMinVel( double dPipeMinVelocity ) { m_rTechParamData.m_dPipeMinVelocity = dPipeMinVelocity; }
	void SetPipeMaxVel( double dPipeMaxVelocity ) { m_rTechParamData.m_dPipeMaxVelocity = dPipeMaxVelocity; }
	void SetPipeTargDp( double dPipeTargetDp ) { m_rTechParamData.m_dPipeTargetDp = dPipeTargetDp; }
	void SetPipeTargVel( double dPipeTargetVelocity ) { m_rTechParamData.m_dPipeTargetVelocity = dPipeTargetVelocity; }
	void SetPipeVtargUsed( int iPipeTargetVelosityUsed ) { m_rTechParamData.m_iPipeTargetVelosityUsed = iPipeTargetVelosityUsed; }

	void SetTrvMinDpRatio( double dTrvMinDpRatio ) { m_rTechParamData.m_dTrvMinDpRatio = dTrvMinDpRatio; }
	void SetTrvDefDpTot( double dTrvDefDpTotal ) { m_rTechParamData.m_dTrvDefDpTotal = dTrvDefDpTotal; }
	void SetTrvDefRoomT( double dTrvDefRoomTemperature ) { m_rTechParamData.m_dTrvDefRoomTemperature = dTrvDefRoomTemperature; }
	void SetTrvDefSupplyT( double dTrvDefSupplyTemperature ) { m_rTechParamData.m_dTrvDefSupplyTemperature = dTrvDefSupplyTemperature; }
	void SetTrvMaxDp( double dTrvMaxDp ) { m_rTechParamData.m_dTrvMaxDp = dTrvMaxDp; }

	void SetCVMinAuthor( double dCVMinAuthor ) { m_rTechParamData.m_dCVMinAuthor = dCVMinAuthor; }
	void SetCVMinAuthCstFlow( double dCVMinAuthCstFlow ) { m_rTechParamData.m_dCVMinAuthCstFlow = dCVMinAuthCstFlow; }
	void SetCVMinDpOnoff( double dCVMinDpOnoff ) { m_rTechParamData.m_dCVMinDpOnoff = dCVMinDpOnoff; }
	void SetCVMinDpProp( double dCVMinDpProp ) { m_rTechParamData.m_dCVMinDpProp = dCVMinDpProp; }
	void SetCVMaxDpProp( double dCVMaxDpProp ) { m_rTechParamData.m_dCVMaxDpProp = dCVMaxDpProp; }
	void SetCVMaxDispDp( double dCVMaxDispDp ) { m_rTechParamData.m_dCVMaxDispDp = dCVMaxDispDp; }
	void SetCVDefKvReynard( int iCVDefKvReynard ) { m_rTechParamData.m_iCVDefKvReynard = iCVDefKvReynard; }
	void SetCVUseDpCBand( int iCVUseDpCPBand ) { m_rTechParamData.m_iCVUseDpCPBand = iCVUseDpCPBand; }

	void SetPICvMinDp( double dPICvMinDp ) { m_rTechParamData.m_dPICvMinDp = dPICvMinDp; }
	void SetPICvHide( int iPICvHide ) { m_rTechParamData.m_iPICvHide = iPICvHide; }

	void SetDpCMinDp( double dDpCMinDp ) { m_rTechParamData.m_dDpCMinDp = dDpCMinDp; }
	void SetDpCHide( int iDpCHide ) { m_rTechParamData.m_iDpCHide = iDpCHide; }
	void SetDpCfDpCMvWithSameSizeOnly( int iDpCMvWithSameSizeOnly ) { m_rTechParamData.m_iDpCMvWithSameSizeOnly = iDpCMvWithSameSizeOnly; }
	void SetDpCMvLoc( int iDpCMvLocation ) { m_rTechParamData.m_iDpCMvLocation = iDpCMvLocation; }
	void SetDpCHminFullyOpenMv( int iDpCHminFullyOpenMv ) { m_rTechParamData.m_iDpCHminFullyOpenMv = iDpCHminFullyOpenMv; }

	void SetDpCBCVDpcmin( double dDpCBCVDpcmin ) { m_rTechParamData.m_dDpCBCVDpcmin = dDpCBCVDpcmin; }
	void SetDpCBCVDplmin( double dDpCBCVDplmin ) { m_rTechParamData.m_dDpCBCVDplmin = dDpCBCVDplmin; }

	void SetVesselSelectionRangePercentage( double dVesselSelectionRangePercentage ) { m_rTechParamData.m_dVesselSelectionRangePercentage = dVesselSelectionRangePercentage; }

	void SetFlowDeviationMinus( double dFlowDeviationMinus ) { m_rTechParamData.m_dFlowDeviationMinus = dFlowDeviationMinus; }
	void SetFlowDeviationPlus( double dFlowDeviationPlus ) { m_rTechParamData.m_dFlowDeviationPlus = dFlowDeviationPlus; }

	void SetCurrencyID( _string strCurrencyID ) { m_rTechParamData.m_strCurrencyID = strCurrencyID; }
	void SetDefaultPipeSerieID( _string strDefaultPipeSerieID ) { m_rTechParamData.m_strDefaultPipeSerieID = strDefaultPipeSerieID; }
	
	void SetProjectApplicationType( ProjectType eProjectType ) { m_rTechParamData.m_eProjectType = eProjectType; }
	
	void SetDefaultPrjTps( double dDefaultTps) { ( Cooling == m_rTechParamData.m_eProjectType ) ? m_rTechParamData.m_dDefaultPrjCoolingTps = dDefaultTps : m_rTechParamData.m_dDefaultPrjHeatingTps = dDefaultTps; }
	void SetDefaultPrjDT( double dDefaultDT ) { ( Cooling == m_rTechParamData.m_eProjectType ) ? m_rTechParamData.m_dDefaultPrjCoolingDT = dDefaultDT : m_rTechParamData.m_dDefaultPrjHeatingDT = dDefaultDT; }
	void SetDefaultPrjAddit(_string strDefaultAdditId) { (Cooling == m_rTechParamData.m_eProjectType) ? m_rTechParamData.m_strDefaultPrjCoolingAdditID = strDefaultAdditId : m_rTechParamData.m_strDefaultPrjHeatingAdditID = strDefaultAdditId; }
	void SetDefaultPrjPcWeight(double dDefaultPcWeight) { (Cooling == m_rTechParamData.m_eProjectType) ? m_rTechParamData.m_dDefaultPrjCoolingPcWeight = dDefaultPcWeight : m_rTechParamData.m_dDefaultPrjHeatingPcWeight = dDefaultPcWeight; }

	void SetDefaultPrjHeatingTps( double dDefaultTps ) { m_rTechParamData.m_dDefaultPrjHeatingTps = dDefaultTps; }
	void SetDefaultPrjHeatingDT( double dDefaultDT ) { m_rTechParamData.m_dDefaultPrjHeatingDT = dDefaultDT; }
	void SetDefaultPrjHeatingAddit(_string strDefaultAdditId) { m_rTechParamData.m_strDefaultPrjHeatingAdditID = strDefaultAdditId; }
	void SetDefaultPrjHeatingPcWeight(double dDefaultPcWeight) { m_rTechParamData.m_dDefaultPrjHeatingPcWeight = dDefaultPcWeight; }

	void SetDefaultPrjCoolingTps( double dDefaultTps ) { m_rTechParamData.m_dDefaultPrjCoolingTps = dDefaultTps; }
	void SetDefaultPrjCoolingDT( double dDefaultDT ) { m_rTechParamData.m_dDefaultPrjCoolingDT = dDefaultDT; }
	void SetDefaultPrjCoolingAddit(_string strDefaultAdditId) { m_rTechParamData.m_strDefaultPrjCoolingAdditID = strDefaultAdditId; }
	void SetDefaultPrjCoolingPcWeight(double dDefaultPcWeight) { m_rTechParamData.m_dDefaultPrjCoolingPcWeight = dDefaultPcWeight; }

	// Can't put "#ifdef TACBX" for these following lines as it's the case for the 'Get' methods.
	// Because we need these following methods for the 'Copy' method.
	void SetProductSelectionApplicationType( ProjectType eProductSelectionApplicationType ) { m_rTechParamData.m_eProductSelectionApplicationType = eProductSelectionApplicationType; }
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1230: Complete review of the getter/setter for water characteristic for product selection.
	// Set default supply temperature for product selection.
	
	// In regards to the current application type defined in the technical parameters, this method will set the default supply temperature.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISTps( double dDefaultISTps, SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	void SetDefaultISTps( double dDefaultISTps, ProjectType eApplicationType, SideDefinition eSideDefinition = Undefined );

	// Set the default supply temperature for heating mode.
	void SetDefaultISHeatingTps( double dDefaultISHeatingTps );

	// Set the default supply temperature for cooling mode.
	void SetDefaultISCoolingTps( double dDefaultISCoolingTps );
	
	// Set the default supply temperature for solar mode.
	void SetDefaultISSolarTps( double dDefaultISSolarTps );
	
	// Set the default supply temperature for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISChangeOverTps( double dDefaultISChangeOverTps, SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set default DT for product selection.
	
	// In regards to the current application type defined in the technical parameters, this method will set the default DT.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISDT( double dDefaultISDT, SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	void SetDefaultISDT( double dDefaultISDT, ProjectType eApplicationType, SideDefinition eSideDefinition = Undefined );

	// Set the default DT for heating mode.
	void SetDefaultISHeatingDT( double dDefaultISHeatingDT );

	// Set the default DT for cooling mode.
	void SetDefaultISCoolingDT( double dDefaultISCoolingDT );

	// Set the default DT for solar mode.
	void SetDefaultISSolarDT( double dDefaultISSolarDT );

	// Set the default DT for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISChangeOverDT( double dDefaultISChangeOverDT, SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set default additive ID for product selection.
	
	// In regards to the current application type defined in the technical parameters, this method will set the default additive ID.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISAdditID( _string strDefaultISAdditID, SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	void SetDefaultISAdditID( _string strDefaultISAdditID, ProjectType eApplicationType, SideDefinition eSideDefinition = Undefined );
	
	// Set the default additive ID for heating mode.
	void SetDefaultISHeatingAdditID( _string strDefaultISHeatingAdditID );
	
	// Set the default additive ID for cooling mode.
	void SetDefaultISCoolingAdditID( _string strDefaultISCoolingAdditID );
	
	// Set the default additive ID for solar mode.
	void SetDefaultISSolarAdditID( _string strDefaultISSolarAdditID );

	// Set the default additive ID for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISChangeOverAdditID( _string strDefaultISChangeOverAdditID, SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set default weight of the additive for product selection.
	
	// In regards to the current application type defined in the technical parameters, this method will set the default weight of the additive.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISPcWeight( double dDefaultISPcWeight, SideDefinition eSideDefinition = CoolingSide );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	void SetDefaultISPcWeight( double dDefaultISPcWeight, ProjectType eApplicationType, SideDefinition eSideDefinition = Undefined );

	// Set the default weight of the additive for heating mode.
	void SetDefaultISHeatingPcWeight( double dDefaultISHeatingPcWeight );

	// Set the default weight of the additive for cooling mode.
	void SetDefaultISCoolingPcWeight( double dDefaultISCoolingPcWeight );

	// Set the default weight of the additive for solar mode.
	void SetDefaultISSolarPcWeight( double dDefaultISSolarPcWeight );

	// Set the default weight of the additive for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISChangeOverPcWeight( double dDefaultISChangeOverPcWeight, SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set default water characteristic for product selection.
	
	// In regards to the current application type defined in the technical parameters, this method will set the default water characteristic.
	// If current application type for product selection is 'ChangeOver', we need the 'eSideDefinition' to be defined to either
	// 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISWC( CWaterChar &clWaterChar, SideDefinition eSideDefinition = Undefined );

	// Same as above but the application type is passed by the 'eApplicationType' argument.
	void SetDefaultISWC( CWaterChar &clWaterChar, ProjectType eApplicationType, SideDefinition eSideDefinition = Undefined );

	// Set the default water characteristic for heating mode.
	void SetDefaultISHeatingWC( CWaterChar &clWaterChar );

	// Set the default water characteristic for cooling mode.
	void SetDefaultISCoolingWC( CWaterChar &clWaterChar );

	// Set the default water characteristic for solar mode.
	void SetDefaultISSolarWC( CWaterChar &clWaterChar );

	// Set the default water characteristic for change-over mode.
	// Remark: 'eSideDefinition' must be set either on 'CoolingSide' or 'HeatingSide'.
	void SetDefaultISChangeOverWC( CWaterChar &clWaterChar, SideDefinition eSideDefinition );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// For pressure maintenance.
	// HYS-1022: Change the name of variable and methods to be synchronized with HyTools.
	// 0.2 bar margin over static height as per EN 12828-2014
	void SetMinPressureMargin( double dMinPressureMargin ) { m_rTechParamData.m_dMinPressureMargin = dMinPressureMargin; }

	// HYS-1104: safety temperature limiter limit from which we need to pass to the EN 12953 norm.
	void SetSafetyTempLimiterLimitEN12953( double dSafetyTempLimiterLimitEN12953 ) { m_rTechParamData.m_dSafetyTempLimiterLimitEN12953 = dSafetyTempLimiterLimitEN12953; }
	
	// HYS-1104: supply/return temperature limit from which we need to pass to the EN 12953 norm.
	void SetFluidTempLimitEN12953( double dFluidTempLimitEN12953 ) { m_rTechParamData.m_dFluidTempLimitEN12953 = dFluidTempLimitEN12953; }

	void SetMaxNumberOfAdditionalVesselsInParallel( int iMaxNumberOfAdditionalVesselsInParallel ) { m_rTechParamData.m_iMaxNumberOfAdditionalVesselsInParallel = iMaxNumberOfAdditionalVesselsInParallel; }
	void SetUseVesselPressureVolumeLimit( bool fUseVesselPressureVolumeLimit ) { m_rTechParamData.m_fUseVesselPressureVolumeLimit = fUseVesselPressureVolumeLimit; }
	void SetPressureVolumeLimit( double dPressureVolumeLimit ) { m_rTechParamData.m_dPressureVolumeLimit = dPressureVolumeLimit; }
	// HYS-1126 : Simplify Pst
	void SetUseSimplyPst( bool fUseSimplyPst ) { m_rTechParamData.m_bUseSimplyPst = fUseSimplyPst; }
	void SetCanChangePressureVolumeLimit( bool fCanChangePressureVolumeLimit ) { m_rTechParamData.m_bCanChangePressureVolumeLimit = fCanChangePressureVolumeLimit; }
	void SetPSetupCompresso( double dPSetupCompresso ) { m_rTechParamData.m_dPSetupCompresso = dPSetupCompresso; }
	void SetPSetupTransfero( double dPSetupTransfero ) { m_rTechParamData.m_dPSetupTransfero = dPSetupTransfero; }
	void SetDefaultFillTemperature( double dDefaultFillTemperature ) { m_rTechParamData.m_dDefaultFillTemperature = dDefaultFillTemperature; }
	void SetUseDefaultFreezingPointAsMinTemperature( bool fUseDefaultFreezingPointAsMinTemperature ) { m_rTechParamData.m_fUseDefaultFreezingPointAsMinTemperature = fUseDefaultFreezingPointAsMinTemperature; }
	void SetMaxNumberOfVentoInParallel( int iMaxNumberOfVentoInParallel) { m_rTechParamData.m_iMaxNumberOfVentoInParallel = iMaxNumberOfVentoInParallel; }

	void SetDefaultWaterMakeUpTemp( double dDefaultWaterMakeUpTemp ) { m_rTechParamData.m_dDefaultWaterMakeUpTemp = dDefaultWaterMakeUpTemp; }
	void SetDefaultMaxTempInCooling( double dDefaultMaxTempInCooling ) { m_rTechParamData.m_dDefaultMaxTempInCooling = dDefaultMaxTempInCooling; }
	void SetDefaultMinTempInHeating( double dDefaultMinTempInHeating ) { m_rTechParamData.m_dDefaultMinTempInHeating = dDefaultMinTempInHeating; }
	void SetDefaultMinTempInSolar( double dDefaultMinTempInSolar ) { m_rTechParamData.m_dDefaultMinTempInSolar = dDefaultMinTempInSolar; }
	void SetDefaultSafetyPressureResponseValve( double dDefaultSafetyPressureResponseValve ) { m_rTechParamData.m_dDefaultSafetyPressureResponseValve = dDefaultSafetyPressureResponseValve; }
	void SetDefaultTAZ( double dDefaultTAZ ) { m_rTechParamData.m_dDefaultTAZ = dDefaultTAZ; }
	void SetDefaultStaticPressureOfWaterNetwork( double dDefaultStaticPressureOfWaterNetwork ) { m_rTechParamData.m_dDefaultStaticPressureOfWaterNetwork = dDefaultStaticPressureOfWaterNetwork; }
	void SetDefaultCalculationNorm( _string sDefaultCalculationNorm ) { m_rTechParamData.m_sDefaultCalculationNorm = sDefaultCalculationNorm; }             
	void SetBreakTankRequiredForWaterMakeUp( bool fBreakTankRequiredForWaterMakeUp ) { m_rTechParamData.m_bBreakTankRequiredForWaterMakeUp = fBreakTankRequiredForWaterMakeUp; }
	void SetAvailablePMPanels( int iAvailablePMPanels ) { m_rTechParamData.m_iAvailablePMPanels = (iAvailablePMPanels & (Heating | Cooling | Solar) ); }
	void SetDefaultWaterHardness( double dDefaultWaterHardness ) { m_rTechParamData.m_dDefaultWaterHardness = dDefaultWaterHardness; }
	void SetDegassingWaterReserveForVessel( double dDegassingWaterReserveForVessel ) {  m_rTechParamData.m_dDegassingWaterReserveForVessel = dDegassingWaterReserveForVessel; }

	void SetPICVExtraThresholdDp( double dPICVExtraThresholdDp ) { m_rTechParamData.m_dPICVExtraThresholdDp = dPICVExtraThresholdDp; }

	void SetMaxSafetyValveInParallel( int iMaxSafetyValveInParallel ) { m_rTechParamData.m_iMaxSafetyValveInParallel = iMaxSafetyValveInParallel; }

	// HYS-1022: 0.3 bar margin for initial pressure (EN 12828-2014).
	void SetInitialPressureMargin( double dInitialPressureMargin ) { m_rTechParamData.m_dInitialPressureMargin = dInitialPressureMargin; }

	// HYS-1151: The reference value used to determine the best 6-way valve at a flow given.
	void SetDpRefForBest6Way( double dDpRefForBest6Way ) { m_rTechParamData.m_dDpRefForBest6Way = dDpRefForBest6Way; }

	// HYS-1763: Set the default smart control valve location.
	void SetDefaultSmartCVLoc( int iDefaultLocation ) { m_rTechParamData.m_iDefaultSmartCVLoc = iDefaultLocation; }

	void SetSmartValveDpMaxForBestSuggestion( double dSmartValveDpMaxForBestSuggestion ) { m_rTechParamData.m_dSmartValveDpMaxForBestSuggestion = dSmartValveDpMaxForBestSuggestion; }

	// HYS-1930: New hydronic circuit: auto-adapting variable flow decoupling circuit: add default secondary pump Hmin.
	void SetDefaultSecondaryPumpHMin( double dDefaultSecondaryPumpHMin ) { m_rTechParamData.m_dDefaultSecondaryPumpHMin = dDefaultSecondaryPumpHMin; }

	virtual int Compare( CData *pDataToCompare );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

#ifndef TACBX
	void ExportTechParams( std::map<CString, CString> *pmapExportedData);
	void ImportTechParams( std::map<CString, CString> *pmapExportedData);
#endif

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX	
	virtual void ReadText(INPSTREAM  inpf, unsigned short* LineCount);
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	double _GetValvMaxDp( _string strValveTypeID );

#ifndef TACBX
	void _ExportTechParamsHelper( std::map<CString, CString> *pmapExportedData, CWaterChar &clWaterChar, ProjectType eApplicationType, 
			SideDefinition eSideDefinition = SideDefinition::Undefined );

	void _ImportTechParamsHelper( std::map<CString, CString> *pmapExportedData, CWaterChar *pclWaterChar, ProjectType eApplicationType, 
			SideDefinition eSideDefinition = SideDefinition::Undefined );
#endif

// Private variables.
private:
	TechParam_struct m_rTechParamData;
};		


#ifndef TACBX	
/*
	CDS_PersistData object added to persist specific HySelect informations for the opened project.
	based on CDB_Multistring object
*/
class CDS_PersistData : public CDB_MultiString
{
	DEFINE_DATA(CDS_PersistData)

// Public enum defines.
public:
	typedef	enum ePersistDataDfields
	{
		epdOldValvesUsed = 0
	};

// Constructor and destructor.
protected:
	CDS_PersistData( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_PersistData() {}

// Public methods.
public:
	bool IsOldValveAreUsed() { return m_fOldValveUsed; }
	void SetOldValveUsed( bool bFlag );

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	bool m_fOldValveUsed;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SelParameterBase
//
///////////////////////////////////////////////////////////////////////////////////
// Allow to manage save of column width.
typedef std::map< short, long >					mapShortLong;
typedef mapShortLong::iterator					mapShortLongIter;
typedef std::map< short, mapShortLong >			mapMapShortLong;
typedef mapMapShortLong::iterator				mapMapShortLongIter;

class CDS_SelParameterBase : public CData
{
public:
	typedef enum ParameterType
	{
		IndividualSelection,
		BatchSelection,
		WizardSelection
	};

protected:
	CDS_SelParameterBase( ParameterType eParameterType, CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SelParameterBase();

// Public methods.
public:
	virtual void WriteToRegistry();
	virtual void ReadFromRegistry();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Class helper to manage column width for all windows.
	class CCWContainer;
	class CCWWindow;
	class CCWSheet
	{
	public:
		CCWSheet() { _Init(); }
		CCWSheet( short nSheetID ) { _Init(); m_nSheetID = nSheetID; }
		virtual ~CCWSheet() {}

		// Allow to manage saving of column width.
		typedef std::map< short, long >	mapShortLong;
		typedef mapShortLong::iterator	mapShortLongIter;

		void SetVersion( short nVersionID ) { m_nVersionID = nVersionID; }
		short GetVersion( void ) { return m_nVersionID; }
		mapShortLong& GetMap( void ) { return m_mapAllColumn; }
		virtual void ReadRegistry( CMemFile& clCWSheet );
		virtual void ReadRegistryOldVersion( CMemFile& clCWBurst );
		virtual void WriteRegistry( CMemFile& clCWSheet );
	private:
		void _Init( void );
	private:
		short m_nSheetID;
		short m_nVersionID;
		mapShortLong m_mapAllColumn;
	};
	
	typedef std::map<short, CCWSheet*> mapShortCCWSheet;
	typedef mapShortCCWSheet::iterator mapShortCCWSheetIter;
	class CCWWindow
	{
	public:
		CCWWindow() { _Init(); }
		CCWWindow( short nWindowID ) { _Init(); m_nWindowID = nWindowID; }
		virtual ~CCWWindow();

		CCWSheet* GetCWSheet( short nSheetID, bool fCreateIfNotExist = false );
		virtual void ReadRegistry( CString strSectionName );
		virtual void ReadRegistryOldVersion( CMemFile& clCWBurst, int iSheetNumber );
		virtual void WriteRegistry( CString strSectionName );
	protected:
		virtual CCWSheet* CreateNewCWSSheet( short nSheetID );
	private:
		void _Init( void );
	private:
		short m_nWindowID;
		mapShortCCWSheet m_mapAllSheet;
	};

	typedef std::map<short, CCWWindow*> mapShortCCWWindow;
	typedef mapShortCCWWindow::iterator mapShortCCWWindowIter;
	class CCWContainer
	{
	public:
		CCWContainer() { _Clear(); }
		virtual ~CCWContainer();

		CCWWindow* GetCCWindow( short nWindowID, bool fCreateIfNotExist = false );
		virtual void ReadRegistry( CString strSectionName );
		virtual void WriteRegistry( CString strSectionName );
	protected:
		virtual CCWWindow* CreateNewCWSWindow( short nWindowID );
	private:
		void _Clear( void );
	private:
		mapShortCCWWindow m_mapAllWindow;
		bool m_bInitialized;
	};
	CCWContainer *m_pclColumnWidthContainer;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CCWWindow *GetWindowColumnWidth( short nWindowID, bool fCreateIfNotExist = false );

// Protected variables.
protected:
	CString m_strSectionName;

// Private variables.
private:
	ParameterType m_eParameterType;
	int m_iColumnWidthVersion;
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_IndSelParameter
//
///////////////////////////////////////////////////////////////////////////////////
struct IndividualSelectionParameters
{
	// Parameters common to all selections.
	TCHAR m_strPipeSeriesID[ _ID_LENGTH + 1 ];					// Last selected pipe series ID.
	TCHAR m_strPipeSizeID[ _ID_LENGTH + 1 ];					// Last selected pipe size ID.

	// Additional parameters for balancing valves.
	int m_iBvRadioFlowPowerDT;									// Version 17: Last radio state.
	int m_iBvDpChecked;											// Last Dp check box state.
	TCHAR m_strBvTypeID[ _ID_LENGTH + 1 ];						// Last selected balancing valve type ID.
	TCHAR m_strBvFamilyID[ _ID_LENGTH + 1 ];					// Last selected balancing valve family ID.
	TCHAR m_strBvMaterialID[ _ID_LENGTH + 1 ];					// Last selected balancing valve material ID.
	TCHAR m_strBvConnectID[ _ID_LENGTH + 1 ];					// Last selected balancing valve connection ID.
	TCHAR m_strBvVersionID[ _ID_LENGTH + 1 ];					// Last selected balancing valve version ID.
	TCHAR m_strBvPNID[ _ID_LENGTH + 1 ];						// Last selected balancing valve PN ID.
	
	// Additional parameters for Dp controller.
	int m_iDpCRadioFlowPowerDT;									// Version 17: Last radio state.
	int m_iDpCDpStab;											// Last Dp Stab option (branch/control valve).
	int m_iDpCMvLoc;											// Last measuring valve location (primary/secondary).
	int m_iDpCLocation;											// Last DpC location.
	int m_iDpCDpChecked;										// Last KvDp check box state.
	int m_iDpCDpMaxChecked;										// Version 19: Last Dp Max check box state.
	TCHAR m_strDpCTypeID[ _ID_LENGTH + 1 ];						// Last selected Dp controller type ID.
	TCHAR m_strDpCFamilyID[ _ID_LENGTH + 1 ];					// Last selected Dp controller family ID.
	TCHAR m_strDpCMaterialID[ _ID_LENGTH + 1 ];					// Last selected Dp controller material ID.
	TCHAR m_strDpCConnectID[ _ID_LENGTH + 1 ];					// Last selected Dp controller connection ID.
	TCHAR m_strDpCVersionID[ _ID_LENGTH + 1 ];					// Last selected Dp controller version ID.
	TCHAR m_strDpCPNID[ _ID_LENGTH + 1 ];						// Last selected Dp controller PN ID.
	int m_iDpCPackageChecked;									// last selected DpC package checked.
	int m_iDpCInfoWnd;											// Show/Hide DpC info Wnd.

	// Additional parameters for thermostatic and return valves.
	int m_iTrvRadioFlowPowerDT;									// Version 28: Last radio state.
	int m_iTrvDpChecked;										// Last Dp check box state.
	int m_iTrvActuatorType;										// Last choice of actuator type.
	int m_iTrvRadiatorValveType;								// Last choice for thermostatic radiator valve type combo.
	TCHAR m_strTrvSVFamilyID[ _ID_LENGTH + 1 ];					// Last selected supply valve family ID.
	TCHAR m_strTrvSVVersionID[ _ID_LENGTH + 1 ];				// Last selected supply valve version ID.
	TCHAR m_strTrvSVConnectID[ _ID_LENGTH + 1 ];				// Last selected supply valve connection ID.
	TCHAR m_strTrvRVFamilyID[ _ID_LENGTH + 1 ];					// Last selected return valve family ID.
	TCHAR m_strTrvRVVersionID[ _ID_LENGTH + 1 ];				// Last selected return valve version ID.
	TCHAR m_strTrvRVConnectID[ _ID_LENGTH + 1 ];				// Last selected return valve connection ID.
	TCHAR m_strTrvSVInsertName[ _COMBO_ITEMNAME_MAX + 1 ];		// Last selected supply valve insert name ("Heimeier Insert" or "Other Insert").
	TCHAR m_strTrvSVInsertFamilyID[ _ID_LENGTH + 1 ];			// Last selected supply valve insert family ID.
	TCHAR m_strTrvSVInsertTypeID[ _ID_LENGTH + 1 ];				// Last selected supply valve insert type ID.
	bool m_bTrvSVInsertInKv;									// Flag to 'true' to tell if we work with Kv for supply valve insert.
	double m_dTrvSVInsertKvValue;
	TCHAR m_strTrvRVInsertFamilyID[ _ID_LENGTH + 1 ];			// Last selected return valve insert family ID.
	TCHAR m_strTrvActuatorTypeID[ _ID_LENGTH + 1 ];				// Last selected actuator head type ID.
	TCHAR m_strTrvActuatorFamilyID[ _ID_LENGTH + 1 ];			// Last selected actuator family ID.
	TCHAR m_strTrvActuatorVersionID[ _ID_LENGTH + 1 ];			// Last selected actuator version ID.
	RadiatorReturnValveMode m_eTrvRVMode;						// Last selected return valve mode.
	RadInfo_struct m_rTrvRadInfos;								// Last parameters in CDialogSizeRad.

	// Additional parameters for control valves.
	int m_iCvRadioFlowPowerDT;									// Version 17: Last radio state.
	CDB_ControlProperties::CV2W3W m_eCvCB2W3W;					// Last selected into combo 2W3W.
	CDB_ControlProperties::CvCtrlType m_eCvCBCtrlType;			// Last selected into combo ctrl type.
	int m_iCvCheckDpKvs;										// Last Dp-Kvs checkbox.
	int m_iCvRadioKvs;											// Last Radio Kvs state.
	TCHAR m_strCvTypeID[ _ID_LENGTH + 1 ];						// Last selected control valve type ID.
	TCHAR m_strCvFamilyID[ _ID_LENGTH + 1 ];					// Last selected control valve family ID.
	TCHAR m_strCvMaterialID[ _ID_LENGTH + 1 ];					// Last selected control valve material ID.
	TCHAR m_strCvConnectID[ _ID_LENGTH + 1 ];					// Last selected control valve connection ID.
	TCHAR m_strCvVersionID[ _ID_LENGTH + 1 ];					// Last selected control valve version ID.
	TCHAR m_strCvPNID[ _ID_LENGTH + 1 ];						// Last selected control valve PN ID.
	TCHAR m_strCvActPowerSupplyID[ _ID_LENGTH + 1 ];			// Last selected control valve power supply actuator ID.
	TCHAR m_strCvActInputSignalID[ _ID_LENGTH + 1 ];			// Last selected control valve input signal actuator ID.
	CDB_ControlValve::DRPFunction m_eCvDRPFct;		// Last selected control valve default return position function.
	int m_iCvFailSafeFct;
	int m_iCvPackageChecked;									// last selected control package checked.

	// Additional parameters for pressure independent control valves.
	int m_iPICvRadioFlowPowerDT;								// Version 17: Last radio state.
	CDB_ControlProperties::CvCtrlType m_ePICvCBCtrlType;		// Last selected into combo ctrl type.
	TCHAR m_strPICvTypeID[ _ID_LENGTH + 1 ];					// Last selected pressure independent control valve type ID.
	TCHAR m_strPICvFamilyID[ _ID_LENGTH + 1 ];					// Last selected pressure independent control valve family ID.
	TCHAR m_strPICvMaterialID[ _ID_LENGTH + 1 ];				// Last selected pressure independent control valve material ID.
	TCHAR m_strPICvConnectID[ _ID_LENGTH + 1 ];					// Last selected pressure independent control valve connection ID.
	TCHAR m_strPICvVersionID[ _ID_LENGTH + 1 ];					// Last selected pressure independent control valve version ID.
	TCHAR m_strPICvPNID[ _ID_LENGTH + 1 ];						// Last selected pressure independent control valve PN ID.
	TCHAR m_strPICvActPowerSupplyID[ _ID_LENGTH + 1 ];			// Last selected pressure independent control valve power supply actuator ID.
	TCHAR m_strPICvActInputSignalID[ _ID_LENGTH + 1 ];			// Last selected pressure independent control valve input signal actuator ID.
	CDB_ControlValve::DRPFunction m_ePICvDRPFct;		// Last selected pressure independent control valve default return position function.
	int m_iPICvFailSafeFct;		// Last selected pressure independent control valve fail-safe function.
	int m_iPICvPackageChecked;									// last selected pressure independent control package checked.
	int m_iPICvDpMaxChecked;									// Version 19: Last Dp Max check box state.

	// Additional parameters for balancing and control valves.
	int m_iBCVRadioFlowPowerDT;									// Version 17: Last radio state.
	int m_iBCVDpChecked;										// Version 17: Last Dp check box state.
	CDB_ControlProperties::CV2W3W m_eBCV2W3W;					// Last selected into combo 2W3W.
	CDB_ControlProperties::CvCtrlType m_eBCVCBCtrlType;		// Last selected into combo ctrl type.
	TCHAR m_strBCVTypeID[ _ID_LENGTH + 1 ];						// Last selected balancing and control valve type ID.
	TCHAR m_strBCVFamilyID[ _ID_LENGTH + 1 ];					// Last selected balancing and control valve family ID.
	TCHAR m_strBCVMaterialID[ _ID_LENGTH + 1 ];					// Last selected balancing and control valve material ID.
	TCHAR m_strBCVConnectID[ _ID_LENGTH + 1 ];					// Last selected balancing and control valve connection ID.
	TCHAR m_strBCVVersionID[ _ID_LENGTH + 1 ];					// Last selected balancing and control valve version ID.
	TCHAR m_strBCVPNID[ _ID_LENGTH + 1 ];						// Last selected balancing and control valve PN ID.
	TCHAR m_strBCVActPowerSupplyID[ _ID_LENGTH + 1 ];			// Last selected balancing and control valve power supply actuator ID.
	TCHAR m_strBCVActInputSignalID[ _ID_LENGTH + 1 ];			// Last selected balancing and control valve input signal actuator ID.
	CDB_ControlValve::DRPFunction m_eBCVDRPFct;		// Last selected balancing and control valve default return position function.
	int m_iBCVFailSafeFct;		// Last selected balancing and control valve fail-safe function.
	int m_iBCVPackageChecked;									// last selected balancing and control package checked.

	// Additional parameters for separators.
	int m_iSepRadioFlowPowerDT;									// Version 17: Last radio state.
	TCHAR m_strSeparatorTypeID[ _ID_LENGTH + 1 ];				// Last selected separator type ID.
	TCHAR m_strSeparatorFamilyID[ _ID_LENGTH + 1 ];				// Last selected separator family ID.
	TCHAR m_strSeparatorConnectID[ _ID_LENGTH + 1 ];			// Last selected separator connection ID.
	TCHAR m_strSeparatorVersionID[ _ID_LENGTH + 1 ];			// Last selected separator version ID.

	// Additional parameters for pressure maintenance - Common.
	ProjectType m_ePMApplicationType;							// Last selected 'Application type'.
	TCHAR m_strPMPressureMaintenanceTypeID[_ID_LENGTH + 1];		// Last selected 'Pressure maintenance type'.
	TCHAR m_strPMWaterMakeUpTypeID[_ID_LENGTH + 1];				// Last selected 'Water make-up'.					// Version 20.
	int m_iPMDegassingChecked;									// Last 'Degassing' checkbox state.
	TCHAR m_strPMNorm[_ID_LENGTH + 1];							// Last selected 'Norm' (for heating only).
	PressurON m_ePMPressOn;										// Last selected 'Pressurization on'.
	bool m_fPMWaterMakeupBreakTank;								// break tank proposed by default

	// Parameters for combined Dp controller and balancing & control valves.
	// Version 21.
	int m_iDpCBCVRadioFlowPowerDT;								// Last radio state.
	CDB_ControlProperties::CvCtrlType m_eDpCBCVCBCtrlType;		// Last selected into combo ctrl type.
	TCHAR m_strDpCBCVTypeID[ _ID_LENGTH + 1 ];					// Last selected combined Dp controller and balancing & control valve type ID.
	TCHAR m_strDpCBCVFamilyID[ _ID_LENGTH + 1 ];				// Last selected combined Dp controller and balancing & control valve family ID.
	TCHAR m_strDpCBCVMaterialID[ _ID_LENGTH + 1 ];				// Last selected combined Dp controller and balancing & control valve material ID.
	TCHAR m_strDpCBCVConnectID[ _ID_LENGTH + 1 ];				// Last selected combined Dp controller and balancing & control valve connection ID.
	TCHAR m_strDpCBCVVersionID[ _ID_LENGTH + 1 ];				// Last selected combined Dp controller and balancing & control valve version ID.
	TCHAR m_strDpCBCVPNID[ _ID_LENGTH + 1 ];					// Last selected combined Dp controller and balancing & control valve PN ID.
	TCHAR m_strDpCBCVActPowerSupplyID[ _ID_LENGTH + 1 ];		// Last selected combined Dp controller and balancing & control valve power supply actuator ID.
	TCHAR m_strDpCBCVActInputSignalID[ _ID_LENGTH + 1 ];		// Last selected combined Dp controller and balancing & control valve input signal actuator ID.
	CDB_ControlValve::DRPFunction m_eDpCBCVDRPFct;	// Last selected combined Dp controller and balancing & control valve Default return position function.
	int m_iDpCBCVFailSafeFct;	// Last selected combined Dp controller and balancing & control valve fail-safe function.
	int m_iDpCBCVPackageChecked;								// last selected combined Dp controller and balancing & control package checked.
	int m_iDpCBCVDpToStabilizeChecked;							// Last Dp to stabilize check box state.

	// Parameters for shut-off valves.
	// Version 22.
	int m_iSvRadioFlowPowerDT;									// Last radio state.
	TCHAR m_strSvTypeID[ _ID_LENGTH + 1 ];						// Last selected shut-off valve type ID.
	TCHAR m_strSvFamilyID[ _ID_LENGTH + 1 ];					// Last selected shut-offl valve family ID.
	TCHAR m_strSvMaterialID[ _ID_LENGTH + 1 ];					// Last selected shut-off valve material ID.
	TCHAR m_strSvConnectID[ _ID_LENGTH + 1 ];					// Last selected shut-off valve connection ID.
	TCHAR m_strSvVersionID[ _ID_LENGTH + 1 ];					// Last selected shut-off valve version ID.
	TCHAR m_strSvPNID[ _ID_LENGTH + 1 ];						// Last selected shut-off valve PN ID.

	// Additional parameters for combined Dp controller and balancing & control valves.
	// Version 23.
	int m_iDpCBCVWithSTSChecked;								// Last 'With STS' check box state.
	int m_iDpCBCVInfoWnd;										// Show/Hide combined Dp controller, control and balancing valve info Wnd.

	// Version 24: parameters for safety valves.
	ProjectType m_eSafetyValveSystemType;
	TCHAR m_strSafetyValveSystemHeatGeneratorTypeID[_ID_LENGTH + 1];
	TCHAR m_strSafetyValveNormID[_ID_LENGTH + 1];
	TCHAR m_strSafetyValveFamilyID[_ID_LENGTH + 1];
	TCHAR m_strSafetyValveConnectionID[_ID_LENGTH + 1];
	double m_dSafetyValveSetPressure;

	// Version 25: HYS-1058: we need to save pressurisation maintenance and water quality preferences.
	CPMWQPrefs *m_pclPMWQPrefs;	
	
	// For 6-way valve.
	e6WayValveSelectionMode m_e6WValveSelectionMode;
	int m_i6WValveRadioFlowPowerDT;
	TCHAR m_str6WValveConnectID[_ID_LENGTH + 1];
	TCHAR m_str6WValveVersionID[_ID_LENGTH + 1];
	bool m_b6WValveCheckFastElectConnection;
	// HYS-1877: Version 31
	bool m_b6WValveCheckOnlyForSet;
	// PICV.
	bool m_bIs6WValvePICvDpMaxChecked;
	CDB_ControlProperties::CvCtrlType m_e6WValvePICvCBCtrlType;
	TCHAR m_str6WValvePICvTypeID[_ID_LENGTH + 1];
	TCHAR m_str6WValvePICvFamilyID[_ID_LENGTH + 1];
	TCHAR m_str6WValvePICvMaterialID[_ID_LENGTH + 1];
	TCHAR m_str6WValvePICvConnectID[_ID_LENGTH + 1];
	TCHAR m_str6WValvePICvVersionID[_ID_LENGTH + 1];
	TCHAR m_str6WValvePICvPNID[_ID_LENGTH + 1];
	TCHAR m_str6WValvePICvActInputSignalID[_ID_LENGTH + 1];
	// BV.
	bool m_bIs6WValveBvDpChecked;
	TCHAR m_str6WValveBvTypeID[_ID_LENGTH + 1];						// Last selected balancing valve type ID.
	TCHAR m_str6WValveBvFamilyID[_ID_LENGTH + 1];					// Last selected balancing valve family ID.
	TCHAR m_str6WValveBvMaterialID[_ID_LENGTH + 1];					// Last selected balancing valve material ID.
	TCHAR m_str6WValveBvConnectID[_ID_LENGTH + 1];					// Last selected balancing valve connection ID.
	TCHAR m_str6WValveBvVersionID[_ID_LENGTH + 1];					// Last selected balancing valve version ID.
	TCHAR m_str6WValveBvPNID[_ID_LENGTH + 1];						// Last selected balancing valve PN ID.

	// For smart control valve.
	int m_iSmartControlValveRadioFlowPowerDT;
	TCHAR m_strSmartControlValveBodyMaterialID[_ID_LENGTH + 1];
	TCHAR m_strSmartControlValveConnectID[_ID_LENGTH + 1];
	TCHAR m_strSmartControlValveVersionID[_ID_LENGTH + 1];
	int m_iSmartControlValveDpMaxChecked;                           // HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.

	// For smart DpC. Version 32
	// HYS-1937: TA-Smart Dp - 04 - Individual selection: left tab.
	int m_iSmartDpCRadioFlowPowerDT;
	int m_iSmartDpCDpBranchChecked;										// Last Dp branch check box state.
	int m_iSmartDpCDpMaxChecked;                                        // HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	TCHAR m_strSmartDpCBodyMaterialID[_ID_LENGTH + 1];
	TCHAR m_strSmartDpCConnectID[_ID_LENGTH + 1];
	TCHAR m_strSmartDpCVersionID[_ID_LENGTH + 1];
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	// HYS-1992: Remove set possibility for TA Smart Dp.
	bool m_bSmartDpCCheckOnlyForSet; // No more used
	// HYS-1384. Version 30
	// Vector to save all show/hide all priorities arrow states.
	std::map<int, std::map<int, int>> m_mapShowHideAllPrioritiesState;
};

class CDS_IndSelParameter : public CDS_SelParameterBase, protected IndividualSelectionParameters
{
	DEFINE_DATA( CDS_IndSelParameter )
	
protected:
	CDS_IndSelParameter( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_IndSelParameter();

public:
	// Get parameters common to all selections.
	LPCTSTR GetPipeSeriesID( CProductSelelectionParameters *pclProductSelParams ) { return _VerifyPipeSeries( pclProductSelParams ); }
	LPCTSTR GetPipeSizeID() { return m_strPipeSizeID; }

	// Get additional parameters for balancing valves.
	int GetBvRadioFlowPowerDT() { return m_iBvRadioFlowPowerDT; }					// Version 17.
	int GetBvDpCheckBox() { return m_iBvDpChecked; }
	LPCTSTR GetBvTypeID() { return m_strBvTypeID; }
	LPCTSTR GetBvFamilyID() { return m_strBvFamilyID; }
	LPCTSTR GetBvMaterialID() { return m_strBvMaterialID; }
	LPCTSTR GetBvConnectID() { return m_strBvConnectID; }
	LPCTSTR GetBvVersionID() { return m_strBvVersionID; }
	LPCTSTR GetBvPNID() { return m_strBvPNID; }

	// Get additional parameters for differential pressure controllers.
	int GetDpCRadioFlowPowerDT() { return m_iDpCRadioFlowPowerDT; }					// Version 17.
	int GetDpCDpStab() { return m_iDpCDpStab; }
	int GetDpCMvLoc() { return m_iDpCMvLoc; }
	int GetDpCLoc() { return m_iDpCLocation; }
	int GetDpCDpCheckBox() { return m_iDpCDpChecked; }
	int GetDpCDpMaxCheckBox() { return m_iDpCDpMaxChecked; }						// Version 19.
	LPCTSTR GetDpCTypeID() { return m_strDpCTypeID; }
	LPCTSTR GetDpCFamilyID() { return m_strDpCFamilyID; }
	LPCTSTR GetDpCMaterialID() { return m_strDpCMaterialID; }
	LPCTSTR GetDpCConnectID() { return m_strDpCConnectID; }
	LPCTSTR GetDpCVersionID() { return m_strDpCVersionID; }
	LPCTSTR GetDpCPNID() { return m_strDpCPNID; }
	int GetDpCPackageChecked() { return m_iDpCPackageChecked; }
	int GetDpCInfoWnd() { return m_iDpCInfoWnd; }

	// Get additional parameters for thermostatic and return valves.
	int GetTrvRadioFlowPowerDT() { return m_iTrvRadioFlowPowerDT; }
	int GetTrvDpCheckbox() { return m_iTrvDpChecked; }
	int GetTrvActuatorType() { return m_iTrvActuatorType; }
	int GetTrvRadiatorValveType() { return m_iTrvRadiatorValveType; }
	LPCTSTR GetTrvSVFamilyID() { return m_strTrvSVFamilyID; }
	LPCTSTR GetTrvSVVersionID() { return m_strTrvSVVersionID; }
	LPCTSTR GetTrvSVConnectID() { return m_strTrvSVConnectID; }
	LPCTSTR GetTrvRVFamilyID() { return m_strTrvRVFamilyID; }
	LPCTSTR GetTrvRVVersionID() { return m_strTrvRVVersionID; }
	LPCTSTR GetTrvRVConnectID() { return m_strTrvRVConnectID; }
	LPCTSTR GetTrvSVInsertName() { return m_strTrvSVInsertName; }
	LPCTSTR GetTrvSVInsertFamilyID() { return m_strTrvSVInsertFamilyID; }
	LPCTSTR GetTrvSVInsertTypeID() { return m_strTrvSVInsertTypeID; }
	bool GetTrvSVInsertInKv() { return m_bTrvSVInsertInKv; }
	double GetTrvSVInsertKvValue() { return m_dTrvSVInsertKvValue; }
	LPCTSTR GetTrvRVInsertFamilyID() { return m_strTrvRVInsertFamilyID; }
	LPCTSTR GetTrvActuatorTypeID() { return m_strTrvActuatorTypeID; }
	LPCTSTR GetTrvActuatorFamilyID() { return m_strTrvActuatorFamilyID; }
	LPCTSTR GetTrvActuatorVersionID() { return m_strTrvActuatorVersionID; }
	RadiatorReturnValveMode GetTrvRVMode() { return m_eTrvRVMode; }
	void GetTrvRadInfos( RadInfo_struct *pRadInfos ) { if( NULL != pRadInfos ) memcpy( pRadInfos, &m_rTrvRadInfos, sizeof( RadInfo_struct ) ); }

	// Get additional parameters for control valves.
	int GetCvRadioFlowPowerDT() { return m_iCvRadioFlowPowerDT; }					// Version 17.
	CDB_ControlProperties::CV2W3W GetCvCB2W3W() { return m_eCvCB2W3W; }
	CDB_ControlProperties::CvCtrlType GetCvCBCtrlType() { return m_eCvCBCtrlType; }
	int GetCvCheckDpKvs() { return m_iCvCheckDpKvs; }
	int GetCvRadioKvs() { return m_iCvRadioKvs; }
	LPCTSTR GetCvTypeID() { return m_strCvTypeID; }
	LPCTSTR GetCvFamilyID() { return m_strCvFamilyID; }
	LPCTSTR GetCvMaterialID() { return m_strCvMaterialID; }
	LPCTSTR GetCvConnectID() { return m_strCvConnectID; }
	LPCTSTR GetCvVersionID() { return m_strCvVersionID; }
	LPCTSTR GetCvPNID() { return m_strCvPNID; }
	LPCTSTR GetCvActPowerSupplyID() { return m_strCvActPowerSupplyID; }
	LPCTSTR GetCvActInputSignalID() { return m_strCvActInputSignalID; }
	int GetCvFailSafeFct() { return m_iCvFailSafeFct; }
	CDB_ControlValve::DRPFunction GetCvDRPFct() { return m_eCvDRPFct; }
	int GetCvPackageChecked() { return m_iCvPackageChecked; }

	// Get additional parameters for pressure independent control valves.
	int GetPICvRadioFlowPowerDT() { return m_iPICvRadioFlowPowerDT; }					// Version 17.
	CDB_ControlProperties::CvCtrlType GetPICvCBCtrlType() { return m_ePICvCBCtrlType; }
	LPCTSTR GetPICvTypeID() { return m_strPICvTypeID; }
	LPCTSTR GetPICvFamilyID() { return m_strPICvFamilyID; }
	LPCTSTR GetPICvMaterialID() { return m_strPICvMaterialID; }
	LPCTSTR GetPICvConnectID() { return m_strPICvConnectID; }
	LPCTSTR GetPICvVersionID() { return m_strPICvVersionID; }
	LPCTSTR GetPICvPNID() { return m_strPICvPNID; }
	LPCTSTR GetPICvActPowerSupplyID() { return m_strPICvActPowerSupplyID; }
	LPCTSTR GetPICvActInputSignalID() { return m_strPICvActInputSignalID; }
	int GetPICvFailSafeFct() { return m_iPICvFailSafeFct; }
	CDB_ControlValve::DRPFunction GetPICvDRPFct() { return m_ePICvDRPFct; }
	int GetPICvPackageChecked() { return m_iPICvPackageChecked; }
	int GetPICvDpMaxCheckBox() { return m_iPICvDpMaxChecked; }						// Version 19.

	// Get additional parameters for balancing and control valves.
	int GetBCVRadioFlowPowerDT() { return m_iBCVRadioFlowPowerDT; }					// Version 17.
	int GetBCVDpCheckBox() { return m_iBCVDpChecked; }							// Version 17.
	CDB_ControlProperties::CV2W3W GetBCVCB2W3W() { return m_eBCV2W3W; }
	CDB_ControlProperties::CvCtrlType GetBCVCBCtrlType() { return m_eBCVCBCtrlType; }
	LPCTSTR GetBCVTypeID() { return m_strBCVTypeID; }
	LPCTSTR GetBCVFamilyID() { return m_strBCVFamilyID; }
	LPCTSTR GetBCVMaterialID() { return m_strBCVMaterialID; }
	LPCTSTR GetBCVConnectID() { return m_strBCVConnectID; }
	LPCTSTR GetBCVVersionID() { return m_strBCVVersionID; }
	LPCTSTR GetBCVPNID() { return m_strBCVPNID; }
	LPCTSTR GetBCVActPowerSupplyID() { return m_strBCVActPowerSupplyID; }
	LPCTSTR GetBCVActInputSignalID() { return m_strBCVActInputSignalID; }
	int GetBCVFailSafeFct() { return m_iBCVFailSafeFct; }
	CDB_ControlValve::DRPFunction GetBCVDRPFct() { return m_eBCVDRPFct; }
	int GetBCVPackageChecked() { return m_iBCVPackageChecked; }

	// Get additional parameters for separators.
	int GetSepRadioFlowPowerDT() { return m_iSepRadioFlowPowerDT; }						// Version 17.
	LPCTSTR GetSeparatorTypeID() { return m_strSeparatorTypeID; }
	LPCTSTR GetSeparatorFamilyID() { return m_strSeparatorFamilyID; }
	LPCTSTR GetSeparatorConnectID() { return m_strSeparatorConnectID; }
	LPCTSTR GetSeparatorVersionID() { return m_strSeparatorVersionID; }

	// Get additional parameters for pressure maintenance.
	ProjectType GetPMApplicationType() { return m_ePMApplicationType; }					// Version 18.
	LPCTSTR GetPMPressMaintTypeID() { return m_strPMPressureMaintenanceTypeID; }		// Version 18.
	LPCTSTR GetPMWaterMakeUpTypeID() { return m_strPMWaterMakeUpTypeID; }				// Version 20. 
	int GetPMDegassingChecked() { return m_iPMDegassingChecked; }						// Version 18.
	LPCTSTR GetPMNorm() { return m_strPMNorm; }											// Version 18.
	PressurON GetPMPressOn() { return m_ePMPressOn; }									// version 18.

	bool GetPMWaterMakeupBreakTank() { return m_fPMWaterMakeupBreakTank; }

	// Get additional parameters for combined Dp controller and balancing & control valves.
	// Version 21.
	int GetDpCBCVRadioFlowPowerDT() { return m_iDpCBCVRadioFlowPowerDT; }
	CDB_ControlProperties::CvCtrlType GetDpCBCVCBCtrlType() { return m_eDpCBCVCBCtrlType; }
	LPCTSTR GetDpCBCVTypeID() { return m_strDpCBCVTypeID; }
	LPCTSTR GetDpCBCVFamilyID() { return m_strDpCBCVFamilyID; }
	LPCTSTR GetDpCBCVMaterialID() { return m_strDpCBCVMaterialID; }
	LPCTSTR GetDpCBCVConnectID() { return m_strDpCBCVConnectID; }
	LPCTSTR GetDpCBCVVersionID() { return m_strDpCBCVVersionID; }
	LPCTSTR GetDpCBCVPNID() { return m_strDpCBCVPNID; }
	LPCTSTR GetDpCBCVActPowerSupplyID() { return m_strDpCBCVActPowerSupplyID; }
	LPCTSTR GetDpCBCVActInputSignalID() { return m_strDpCBCVActInputSignalID; }
	int GetDpCBCVFailSafeFct() { return m_iDpCBCVFailSafeFct; }
	CDB_ControlValve::DRPFunction GetDpCBCVDRPFct() { return m_eDpCBCVDRPFct; }
	int GetDpCBCVPackageChecked() { return m_iDpCBCVPackageChecked; }
	int GetDpCBCVDpToStabilizeCheckBox() { return m_iDpCBCVDpToStabilizeChecked; }
	int GetDpCBCVWithSTSCheckBox() { return m_iDpCBCVWithSTSChecked; }
	int GetDpCBCVInfoWnd() { return m_iDpCBCVInfoWnd; }

	// Get additional parameters for shut-off valves.
	// Version 22.
	int GetSvRadioFlowPowerDT() { return m_iSvRadioFlowPowerDT; }
	LPCTSTR GetSvTypeID() { return m_strSvTypeID; }
	LPCTSTR GetSvFamilyID() { return m_strSvFamilyID; }
	LPCTSTR GetSvMaterialID() { return m_strSvMaterialID; }
	LPCTSTR GetSvConnectID() { return m_strSvConnectID; }
	LPCTSTR GetSvVersionID() { return m_strSvVersionID; }
	LPCTSTR GetSvPNID() { return m_strSvPNID; }

	// Version 24: parameters for safety valves.
	ProjectType GetSafetyValveSystemType() { return m_eSafetyValveSystemType; }
	LPCTSTR GetSafetyValveSystemHeatGeneratorTypeID() { return m_strSafetyValveSystemHeatGeneratorTypeID; }
	LPCTSTR GetSafetyValveNormID() { return m_strSafetyValveNormID; }
	LPCTSTR GetSafetyValveFamilyID() { return m_strSafetyValveFamilyID; }
	LPCTSTR GetSafetyValveConnectionID() { return m_strSafetyValveConnectionID; }
	double GetSafetyValveSetPressure() { return m_dSafetyValveSetPressure; }

	// Version 25: HYS-1058: we need to save pressurisation maintenance and water quality preferences.
	CPMWQPrefs *GetpPMWQPrefs() { return m_pclPMWQPrefs; }

	// Parameters for 6 way valves
	e6WayValveSelectionMode Get6WValveSelectionMode() { return m_e6WValveSelectionMode; }
	int Get6WValveRadioFlowPowerDT() { return m_i6WValveRadioFlowPowerDT; }	
	LPCTSTR Get6WValveConnectID() { return m_str6WValveConnectID; }
	LPCTSTR Get6WValveVersionID() { return m_str6WValveVersionID; }
	bool Get6WValveCheckFastConnection() { return m_b6WValveCheckFastElectConnection; }
	// HYS-1877: Version 31
	bool Get6WValveKitCheckbox() { return m_b6WValveCheckOnlyForSet; }
	// 6-way PICV
	bool Get6WValvePICvDpMaxChecked() { return m_bIs6WValvePICvDpMaxChecked; }
	CDB_ControlProperties::CvCtrlType Get6WValvePICvCBCtrlType() { return m_e6WValvePICvCBCtrlType; }
	LPCTSTR Get6WValvePICvTypeID() { return m_str6WValvePICvTypeID; }
	LPCTSTR Get6WValvePICvFamilyID() { return m_str6WValvePICvFamilyID; }
	LPCTSTR Get6WValvePICvMaterialID() { return m_str6WValvePICvMaterialID; }
	LPCTSTR Get6WValvePICvConnectID() { return m_str6WValvePICvConnectID; }
	LPCTSTR Get6WValvePICvVersionID() { return m_str6WValvePICvVersionID; }
	LPCTSTR Get6WValvePICvPNID() { return m_str6WValvePICvPNID; }
	LPCTSTR Get6WValvePICvActInputSignalID() { return m_str6WValvePICvActInputSignalID; }
	// 6way BV
	bool Get6WValveBvDpChecked() { return m_bIs6WValveBvDpChecked; }
	LPCTSTR Get6WValveBVTypeID() { return m_str6WValveBvTypeID; }
	LPCTSTR Get6WValveBVFamilyID() { return m_str6WValveBvFamilyID; }
	LPCTSTR Get6WValveBVMaterialID() { return m_str6WValveBvMaterialID; }
	LPCTSTR Get6WValveBVConnectID() { return m_str6WValveBvConnectID; }
	LPCTSTR Get6WValveBVVersionID() { return m_str6WValveBvVersionID; }
	LPCTSTR Get6WValveBVPNID() { return m_str6WValveBvPNID; }

	// Parameters for smart control valves.
	int GetSmartControlValveRadioFlowPowerDT() { return m_iSmartControlValveRadioFlowPowerDT; }
	LPCTSTR GetSmartControlValveMaterialID() { return m_strSmartControlValveBodyMaterialID; }
	LPCTSTR GetSmartControlValveConnectID() { return m_strSmartControlValveConnectID; }
	LPCTSTR GetSmartControlValvePNID( ) { return m_strSmartControlValveVersionID; }
	int GetSmartControlValveDpMaxCheckBox() { return m_iSmartControlValveDpMaxChecked; }           // Version 32

	// Version 32: HYS-1937: TA-Smart Dp - 04 - Individual selection: left tab..
	int GetSmartDpRadioFlowPowerDT() { return m_iSmartDpCRadioFlowPowerDT; }
	int GetSmartDpCDpBranchCheckBox() { return m_iSmartDpCDpBranchChecked; }
	int GetSmartDpCDpMaxCheckBox() { return m_iSmartDpCDpMaxChecked; }
	LPCTSTR GetSmartDpCMaterialID() { return m_strSmartDpCBodyMaterialID; }
	LPCTSTR GetSmartDpCConnectID() { return m_strSmartDpCConnectID; }
	LPCTSTR GetSmartDpCPNID( ) { return m_strSmartDpCVersionID; }
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	// HYS-1992: Remove set possibility for TA Smart Dp.
	//bool GetSmartDpCSetCheckBox() { return m_bSmartDpCCheckOnlyForSet; }

	// HYS-1384.
	int GetWindowShowAllPrioritiesState( int iWindowID, int iButtonID );

	// Set parameters common to all selections.
	void SetPipeSeriesID( LPCTSTR ID ) { _tcsncpy_s( m_strPipeSeriesID, SIZEOFINTCHAR( m_strPipeSeriesID ), ID, SIZEOFINTCHAR( m_strPipeSeriesID ) - 1 ); }
	void SetPipeSizeID( LPCTSTR ID ) { _tcsncpy_s( m_strPipeSizeID, SIZEOFINTCHAR( m_strPipeSizeID ), ID, SIZEOFINTCHAR( m_strPipeSizeID ) - 1 ); }

	// Set additional parameters for balancing valves.
	void SetBvRadioFlowPowerDT( int iState ) { m_iBvRadioFlowPowerDT = iState; }		// Version 17.
	void SetBvDpCheckBox( int iState ) { m_iBvDpChecked = iState; }
	void SetBvTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strBvTypeID, SIZEOFINTCHAR( m_strBvTypeID ), ID, SIZEOFINTCHAR( m_strBvTypeID ) - 1 ); }
	void SetBvFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strBvFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBvMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strBvMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBvConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strBvConnectID, SIZEOFINTCHAR( m_strBvConnectID ), ID, SIZEOFINTCHAR( m_strBvConnectID ) - 1 ); }
	void SetBvVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strBvVersionID, SIZEOFINTCHAR( m_strBvVersionID ), ID, SIZEOFINTCHAR( m_strBvVersionID ) - 1 ); }
	void SetBvPNID( LPCTSTR ID ) { _tcsncpy_s( m_strBvPNID, SIZEOFINTCHAR( m_strBvPNID ), ID, SIZEOFINTCHAR( m_strBvPNID ) - 1 ); }

	// Set additional parameters for Dp controller.
	void SetDpCRadioFlowPowerDT( int iState )		{ m_iDpCRadioFlowPowerDT = iState; }		// Version 17.
	void SetDpCDpStab( int dpstab ) { m_iDpCDpStab = dpstab; }
	void SetDpCMvLoc(int mvloc) { m_iDpCMvLoc = mvloc; }
	void SetDpCLoc(int DpCloc) { m_iDpCLocation = DpCloc; }
	void SetDpCDpCheckBox( int iState ) { m_iDpCDpChecked = iState; }
	void SetDpCDpMaxCheckBox( int iState ) { m_iDpCDpMaxChecked = iState; }						// Version 19.
	void SetDpCTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCTypeID, SIZEOFINTCHAR( m_strDpCTypeID ), ID, SIZEOFINTCHAR( m_strDpCTypeID ) - 1 ); }
	void SetDpCFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCFamilyID, SIZEOFINTCHAR( m_strDpCFamilyID ), ID, SIZEOFINTCHAR( m_strDpCFamilyID ) - 1 ); }
	void SetDpCMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCMaterialID, SIZEOFINTCHAR( m_strDpCMaterialID ), ID, SIZEOFINTCHAR( m_strDpCMaterialID ) - 1 ); }
	void SetDpCConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCConnectID, SIZEOFINTCHAR( m_strDpCConnectID ), ID, SIZEOFINTCHAR( m_strDpCConnectID ) - 1 ); }
	void SetDpCVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCVersionID, SIZEOFINTCHAR( m_strDpCVersionID ), ID, SIZEOFINTCHAR( m_strDpCVersionID ) - 1 ); }
	void SetDpCPNID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCPNID, SIZEOFINTCHAR( m_strDpCPNID ), ID, SIZEOFINTCHAR( m_strDpCPNID ) - 1 ); }
	void SetDpCPackageChecked(int iDpCPackageSet) { m_iDpCPackageChecked = iDpCPackageSet; }
	void SetDpCInfoWndChecked(int iInfoWnd)	 { m_iDpCInfoWnd = iInfoWnd; }

	// Set additional parameters for thermostatic and return valves.
	void SetTrvRadioFlowPowerDT( int iState ) { m_iTrvRadioFlowPowerDT = iState; }
	void SetTrvDpCheckBox( int iState ) { m_iTrvDpChecked = iState; }
	void SetTrvActuatorType( int iState ) { m_iTrvActuatorType = iState; }
	void SetTrvRadiatorValveType( int iState ) { m_iTrvRadiatorValveType = iState; }
	void SetTrvSVFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvSVFamilyID, SIZEOFINTCHAR( m_strTrvSVFamilyID ), ID, SIZEOFINTCHAR( m_strTrvSVFamilyID ) - 1 ); }
	void SetTrvSVVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvSVVersionID, SIZEOFINTCHAR( m_strTrvSVVersionID ), ID, SIZEOFINTCHAR( m_strTrvSVVersionID ) - 1 ); }
	void SetTrvSVConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvSVConnectID, SIZEOFINTCHAR( m_strTrvSVConnectID ), ID, SIZEOFINTCHAR( m_strTrvSVConnectID ) - 1 ); }
	void SetTrvRVFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvRVFamilyID, SIZEOFINTCHAR( m_strTrvRVFamilyID ), ID, SIZEOFINTCHAR( m_strTrvRVFamilyID ) - 1 ); }
	void SetTrvRVVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvRVVersionID, SIZEOFINTCHAR( m_strTrvRVVersionID ), ID, SIZEOFINTCHAR( m_strTrvRVVersionID ) - 1 ); }
	void SetTrvRVConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvRVConnectID, SIZEOFINTCHAR( m_strTrvRVConnectID ), ID, SIZEOFINTCHAR( m_strTrvRVConnectID ) - 1 ); }
	void SetTrvSVInsertName(LPCTSTR Name) { _tcsncpy_s( m_strTrvSVInsertName, SIZEOFINTCHAR( m_strTrvSVInsertName), Name, SIZEOFINTCHAR( m_strTrvSVInsertName) - 1); }
	void SetTrvSVInsertFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvSVInsertFamilyID, SIZEOFINTCHAR( m_strTrvSVInsertFamilyID ), ID, SIZEOFINTCHAR( m_strTrvSVInsertFamilyID ) - 1 ); }
	void SetTrvSVInsertTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvSVInsertTypeID, SIZEOFINTCHAR( m_strTrvSVInsertTypeID ), ID, SIZEOFINTCHAR( m_strTrvSVInsertTypeID ) - 1 ); }
	void SetTrvSVInsertInKv( bool fFlag ) { m_bTrvSVInsertInKv = fFlag; }
	void SetTrvSVInsertKvValue( double dKv ) { m_dTrvSVInsertKvValue = dKv; }
	void SetTrvRVInsertFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvRVInsertFamilyID, SIZEOFINTCHAR( m_strTrvRVInsertFamilyID ), ID, SIZEOFINTCHAR( m_strTrvRVInsertFamilyID ) - 1 ); }
	void SetTrvActuatorTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvActuatorTypeID, SIZEOFINTCHAR( m_strTrvActuatorTypeID ), ID, SIZEOFINTCHAR( m_strTrvActuatorTypeID ) - 1 ); }
	void SetTrvActuatorFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvActuatorFamilyID, SIZEOFINTCHAR( m_strTrvActuatorFamilyID ), ID, SIZEOFINTCHAR( m_strTrvActuatorFamilyID ) - 1 ); }
	void SetTrvActuatorVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strTrvActuatorVersionID, SIZEOFINTCHAR( m_strTrvActuatorVersionID ), ID, SIZEOFINTCHAR( m_strTrvActuatorVersionID ) - 1 ); }
	void SetTrvRVMode( RadiatorReturnValveMode RetVMode ) { m_eTrvRVMode = RetVMode; }
	void SetTrvRadInfos( RadInfo_struct *pRadInfos ) { memcpy( &m_rTrvRadInfos, pRadInfos, sizeof( RadInfo_struct ) ); }

	// Set additional parameters for control valves.
	void SetCvRadioFlowPowerDT( int iState ) { m_iCvRadioFlowPowerDT = iState; }		// Version 17.
	void SetCvCB2W3W( CDB_ControlProperties::CV2W3W Cv2w3w ){ m_eCvCB2W3W = Cv2w3w; }
	void SetCvCBCtrlType( CDB_ControlProperties::CvCtrlType CvCtrlType ){ m_eCvCBCtrlType = CvCtrlType; }
	void SetCvCheckDpKvs( int state ) { m_iCvCheckDpKvs = state; }
	void SetCvRadioKvs( int state ) { m_iCvRadioKvs = state; }
	void SetCvTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strCvTypeID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetCvFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strCvFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetCvMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strCvMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetCvConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strCvConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetCvVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strCvVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetCvPNID( LPCTSTR ID ) { _tcsncpy_s( m_strCvPNID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetCvActPowerSupplyID( LPCTSTR ID ) { _tcsncpy_s( m_strCvActPowerSupplyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetCvActInputSignalID( LPCTSTR ID ) { _tcsncpy_s( m_strCvActInputSignalID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetCvFailSafeFct( int iFailSafeFct ) { m_iCvFailSafeFct = iFailSafeFct; }
	void SetCvDRPFct( CDB_ControlValve::DRPFunction eDRPFct ) { m_eCvDRPFct = eDRPFct; }
	void SetCvPackageChecked( int iCvPackageSet ) { m_iCvPackageChecked = iCvPackageSet; }

	// Set additional parameters for pressure independent control valves.
	void SetPICvRadioFlowPowerDT( int iState ) { m_iPICvRadioFlowPowerDT = iState; }	// Version 17.
	void SetPICvCBCtrlType( CDB_ControlProperties::CvCtrlType CvCtrlType ){ m_ePICvCBCtrlType = CvCtrlType; }
	void SetPICvTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvTypeID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvPNID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvPNID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvActPowerSupplyID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvActPowerSupplyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvActInputSignalID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvActInputSignalID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvFailSafeFct( int iFailSafeFct ) { m_iPICvFailSafeFct = iFailSafeFct; }
	void SetPICvDRPFct( CDB_ControlValve::DRPFunction eDRPFct ) { m_ePICvDRPFct = eDRPFct; }
	void SetPICvPackageChecked( int iPICvPackageSet ) { m_iPICvPackageChecked = iPICvPackageSet; }
	void SetPICvDpMaxCheckBox( int iState ) { m_iPICvDpMaxChecked = iState; }				// Version 19.

	// Set additional parameters for balancing and control valves.
	void SetBCVRadioFlowPowerDT( int iState ) { m_iBCVRadioFlowPowerDT = iState; }			// Version 17.
	void SetBCVDpCheckBox( int iState ) { m_iBCVDpChecked = iState; }						// Version 17.
	void SetBCVCB2W3W( CDB_ControlProperties::CV2W3W Cv2w3w ) { m_eBCV2W3W = Cv2w3w; }
	void SetBCVCBCtrlType( CDB_ControlProperties::CvCtrlType CvCtrlType ){ m_eBCVCBCtrlType = CvCtrlType; }
	void SetBCVTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strBCVTypeID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCVFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strBCVFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCVMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strBCVMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCVConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strBCVConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCVVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strBCVVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCVPNID( LPCTSTR ID ) { _tcsncpy_s( m_strBCVPNID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCVActPowerSupplyID( LPCTSTR ID ) { _tcsncpy_s( m_strBCVActPowerSupplyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCVActInputSignalID( LPCTSTR ID ) { _tcsncpy_s( m_strBCVActInputSignalID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCVFailSafeFct( int FailSafeFct ) { m_iBCVFailSafeFct = FailSafeFct; }
	void SetBCVDRPFct( CDB_ControlValve::DRPFunction DRPFct ) { m_eBCVDRPFct = DRPFct; }
	void SetBCVPackageChecked(int iBCVPackageSetChecked) { m_iBCVPackageChecked = iBCVPackageSetChecked; }

	// Set additional parameters for separators.
	void SetSepRadioFlowPowerDT( int iState ) { m_iSepRadioFlowPowerDT = iState; }		// Version 17.
	void SetSeparatorTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strSeparatorTypeID, SIZEOFINTCHAR( m_strSeparatorTypeID ), ID, SIZEOFINTCHAR( m_strSeparatorTypeID ) - 1 ); }
	void SetSeparatorFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strSeparatorFamilyID, SIZEOFINTCHAR( m_strSeparatorFamilyID ), ID, SIZEOFINTCHAR( m_strSeparatorFamilyID ) - 1 ); }
	void SetSeparatorConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strSeparatorConnectID, SIZEOFINTCHAR( m_strSeparatorConnectID ), ID, SIZEOFINTCHAR( m_strSeparatorConnectID ) - 1 ); }
	void SetSeparatorVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strSeparatorVersionID, SIZEOFINTCHAR( m_strSeparatorVersionID ), ID, SIZEOFINTCHAR( m_strSeparatorVersionID ) - 1 ); }

	// Set additional parameters for pressure maintenance.
	void SetPMApplicationType( ProjectType ProjectType ) { m_ePMApplicationType = ProjectType; }
	void SetPMPressMaintTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strPMPressureMaintenanceTypeID, SIZEOFINTCHAR( m_strPMPressureMaintenanceTypeID ), ID, SIZEOFINTCHAR( m_strPMPressureMaintenanceTypeID ) - 1 ); }		// Version 18.
	void SetPMWaterMakeUpTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strPMWaterMakeUpTypeID, SIZEOFINTCHAR( m_strPMWaterMakeUpTypeID ), ID, SIZEOFINTCHAR( m_strPMWaterMakeUpTypeID ) - 1 ); }		// Version 20.
	void SetPMDegassingChecked( int iDegassing ) { m_iPMDegassingChecked = iDegassing; }		// Version 18.
	void SetPMNorm( LPCTSTR ID ) { _tcsncpy_s( m_strPMNorm, SIZEOFINTCHAR( m_strPMNorm ), ID, SIZEOFINTCHAR( m_strPMNorm ) - 1 ); }		// Version 18.
	void SetPMPressOn( PressurON pressOn ) { m_ePMPressOn = pressOn; }							// Version 18.

	// Set additional parameters for combined Dp controller and balancing & control valves.
	// Version 21.
	void SetDpCBCVRadioFlowPowerDT( int iState ) { m_iDpCBCVRadioFlowPowerDT = iState; }
	void SetDpCBCVCBCtrlType( CDB_ControlProperties::CvCtrlType CvCtrlType ){ m_eDpCBCVCBCtrlType = CvCtrlType; }
	void SetDpCBCVTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCBCVTypeID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetDpCBCVFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCBCVFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetDpCBCVMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCBCVMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetDpCBCVConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCBCVConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetDpCBCVVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCBCVVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetDpCBCVPNID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCBCVPNID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetDpCBCVActPowerSupplyID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCBCVActPowerSupplyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetDpCBCVActInputSignalID( LPCTSTR ID ) { _tcsncpy_s( m_strDpCBCVActInputSignalID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetDpCBCVFailSafeFct( int iFailSafeFct ) { m_iDpCBCVFailSafeFct = iFailSafeFct; }
	void SetDpCBCVDRPFct( CDB_ControlValve::DRPFunction eDRPFct ) { m_eDpCBCVDRPFct = eDRPFct; }
	void SetDpCBCVPackageChecked( int iDpCBCVPackageSet ) { m_iDpCBCVPackageChecked = iDpCBCVPackageSet; }
	void SetDpCBCVDpToStabilizeCheckBox( int iState ) { m_iDpCBCVDpToStabilizeChecked = iState; }
	void SetDpCBCVWithSTSCheckBox( int iState ) { m_iDpCBCVWithSTSChecked = iState; }
	void SetDpCBCVInfoWndChecked(int iInfoWnd) { m_iDpCBCVInfoWnd = iInfoWnd; }

	// Set additional parameters for shut-off valves.
	// Version 22.
	void SetSvRadioFlowPowerDT( int iState ) { m_iSvRadioFlowPowerDT = iState; }
	void SetSvTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strSvTypeID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSvFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strSvFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSvMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strSvMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSvConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strSvConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSvVersionID( LPCTSTR ID ) { _tcsncpy_s( m_strSvVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSvPNID( LPCTSTR ID ) { _tcsncpy_s( m_strSvPNID, _ID_LENGTH, ID, _ID_LENGTH ); }

	void SetPMWaterMakeupBreakTank( bool fWaterMakeupBreakTank) { m_fPMWaterMakeupBreakTank = fWaterMakeupBreakTank; }

	// Version 24: parameters for safety valves.
	void SetSafetyValveSystemType( ProjectType eSystemType ) { m_eSafetyValveSystemType = eSystemType; }
	void SetSafetyValveSystemHeatGeneratorTypeID( LPCTSTR ID ) { _tcsncpy_s( m_strSafetyValveSystemHeatGeneratorTypeID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSafetyValveNormID( LPCTSTR ID ) { _tcsncpy_s( m_strSafetyValveNormID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSafetyValveFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_strSafetyValveFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSafetyValveConnectionID( LPCTSTR ID ) { _tcsncpy_s( m_strSafetyValveConnectionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSafetyValveSetPressure( double dSetPressure ) { m_dSafetyValveSetPressure = dSetPressure; }

	// Version 25: HYS-1058: we need to save pressurisation maintenance and water quality preferences.
	void SetPMWQPrefs( CPMWQPrefs *pclPMWQPrefs );

	// Parameters for 6-way valves
	void Set6WValveSelectionMode( e6WayValveSelectionMode eSelectionMode ) { m_e6WValveSelectionMode = eSelectionMode; }
	void Set6WValveRadioFlowPowerDT(int iRadioValue ) { m_i6WValveRadioFlowPowerDT = iRadioValue; }
	void Set6WValveConnectID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValveConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValveVersionID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValveVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValveCheckFastElectConnection( bool bChecked ) { m_b6WValveCheckFastElectConnection = bChecked; }
	// HYS-1877: Version 31
	void Set6WValveKitCheckbox( bool bChecked ) { m_b6WValveCheckOnlyForSet = bChecked; }
	// 6-way PICV
	void Set6WValvePICvDpMaxChecked( bool bIsChecked ) { m_bIs6WValvePICvDpMaxChecked = bIsChecked; }
	void Set6WValvePICvCBCtrlType( CDB_ControlProperties::CvCtrlType CtrlType ) { m_e6WValvePICvCBCtrlType = CtrlType; }
	void Set6WValvePICvTypeID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValvePICvTypeID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValvePICvFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValvePICvFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValvePICvMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValvePICvMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValvePICvConnectID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValvePICvConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValvePICvVersionID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValvePICvVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValvePICvPNID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValvePICvPNID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValvePICvActInputSignalID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValvePICvActInputSignalID, _ID_LENGTH, ID, _ID_LENGTH ); }
	// 6-way BV
	void Set6WValveBvDpChecked( bool bIsEnabled ) { m_bIs6WValveBvDpChecked = bIsEnabled; }
	void Set6WValveBVTypeID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValveBvTypeID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValveBVFamilyID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValveBvFamilyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValveBVMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValveBvMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValveBVConnectID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValveBvConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValveBVVersionID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValveBvVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void Set6WValveBVPNID( LPCTSTR ID ) { _tcsncpy_s( m_str6WValveBvPNID, _ID_LENGTH, ID, _ID_LENGTH ); }

	// Parameters for smart control valves.
	void SetSmartControlValveRadioFlowPowerDT( int iRadioValue ) { m_iSmartControlValveRadioFlowPowerDT = iRadioValue; }
	void SetSmartControlValveBodyMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strSmartControlValveBodyMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSmartControlValveConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strSmartControlValveConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSmartControlValvePNID( LPCTSTR ID ) { _tcsncpy_s( m_strSmartControlValveVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSmartControlValveDpMaxCheckBox( int iState ) { m_iSmartControlValveDpMaxChecked = iState; }                         // Version 32
	
	// Version 32: HYS-1937: TA-Smart Dp - 04 - Individual selection: left tab.
	void SetSmartDpCRadioFlowPowerDT( int iRadioValue ) { m_iSmartDpCRadioFlowPowerDT = iRadioValue; }
	void SetSmartDpCDpBranchCheckBox( int iState ) { m_iSmartDpCDpBranchChecked = iState; }
	void SetSmartDpCDpMaxCheckBox( int iState ) { m_iSmartDpCDpMaxChecked = iState; }
	void SetSmartDpCBodyMaterialID( LPCTSTR ID ) { _tcsncpy_s( m_strSmartDpCBodyMaterialID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSmartDpCConnectID( LPCTSTR ID ) { _tcsncpy_s( m_strSmartDpCConnectID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetSmartDpCPNID( LPCTSTR ID ) { _tcsncpy_s( m_strSmartDpCVersionID, _ID_LENGTH, ID, _ID_LENGTH ); }
	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	// HYS-1992: Remove set possibility for TA Smart Dp.
	//void SetSmartDpCSetCheckBox( bool iState ) { m_bSmartDpCCheckOnlyForSet = iState; }

	// HYS-1384.
	void SetWindowShowAllPrioritiesState( int iWindowID, int iButtonID, int iState );

// Public methods.
public:
	// Overrides 'CDS_SelectionParameters'.
	virtual void WriteToRegistry();
	virtual void ReadFromRegistry();

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination ); 
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	TCHAR *_VerifyPipeSeries( CProductSelelectionParameters *pclProductSelParams );
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_BatchSelParameter
//
///////////////////////////////////////////////////////////////////////////////////
struct BatchSelectionParameters
{
	// Parameters common to all selections.
	TCHAR m_strPipeSeriesID[ _ID_LENGTH + 1 ];					// Last selected pipe series ID.
	TCHAR m_strPipeSizeID[ _ID_LENGTH + 1 ];					// Last selected pipe size ID.

	// Additional parameters for balancing valves.
	int m_iBvRadioFlow;											// Last 'Flow' radio state.
	int m_iBvCheckDp;											// Last 'Dp' checkbox state.
	TCHAR m_strBvTypeBelow65ID[ _ID_LENGTH + 1 ];				// Last selected balancing valve type below DN 65 ID
	TCHAR m_strBvFamilyBelow65ID[ _ID_LENGTH + 1 ];				// Last selected balancing valve family below DN 65 ID.
	TCHAR m_strBvMaterialBelow65ID[ _ID_LENGTH + 1 ];			// Last selected balancing valve material below DN 65 ID.
	TCHAR m_strBvConnectBelow65ID[ _ID_LENGTH + 1 ];			// Last selected balancing valve connection below DN 65 ID.
	TCHAR m_strBvVersionBelow65ID[ _ID_LENGTH + 1 ];			// Last selected balancing valve version below DN 65 ID.
	TCHAR m_strBvPNBelow65ID[ _ID_LENGTH + 1 ];					// Last selected balancing valve PN below DN 65 ID.
	
	TCHAR m_strBvTypeAbove50ID[ _ID_LENGTH + 1 ];				// Last selected balancing valve type above DN 50 ID.
	TCHAR m_strBvFamilyAbove50ID[ _ID_LENGTH + 1 ];				// Last selected balancing valve family above DN 50 ID.
	TCHAR m_strBvMaterialAbove50ID[ _ID_LENGTH + 1 ];			// Last selected balancing valve material above DN 50 ID.
	TCHAR m_strBvConnectAbove50ID[ _ID_LENGTH + 1 ];			// Last selected balancing valve connection above DN 50 ID.
	TCHAR m_strBvVersionAbove50ID[ _ID_LENGTH + 1 ];			// Last selected balancing valve version above DN 50 ID.
	TCHAR m_strBvPNAbove50ID[ _ID_LENGTH + 1 ];					// Last selected balancing valve PN above DN 50 ID.

	// Additional parameters for pressure independent control valves.
	int m_iPICvRadioFlow;										// Version 2: Last 'Flow' radio state.
	bool m_bPICvCtrlTypeStrictChecked;							// Version 7: Last 'Strict' checkbox state.
	CDB_ControlProperties::CvCtrlType m_ePICvCBCtrlType;		// Version 2: Last selected into combo ctrl type.
	TCHAR m_strPICvTypeBelow65ID[ _ID_LENGTH + 1 ];				// Version 2: Last selected pressure independent control valve type below DN 65 ID
	TCHAR m_strPICvFamilyBelow65ID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve family below DN 65 ID.
	TCHAR m_strPICvMaterialBelow65ID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve material below DN 65 ID.
	TCHAR m_strPICvConnectBelow65ID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve connection below DN 65 ID.
	TCHAR m_strPICvVersionBelow65ID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve version below DN 65 ID.
	TCHAR m_strPICvPNBelow65ID[ _ID_LENGTH + 1 ];				// Version 2: Last selected pressure independent control valve PN below DN 65 ID.
	
	TCHAR m_strPICvTypeAbove50ID[ _ID_LENGTH + 1 ];				// Version 2: Last selected pressure independent control valve type above DN 50 ID.
	TCHAR m_strPICvFamilyAbove50ID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve family above DN 50 ID.
	TCHAR m_strPICvMaterialAbove50ID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve material above DN 50 ID.
	TCHAR m_strPICvConnectAbove50ID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve connection above DN 50 ID.
	TCHAR m_strPICvVersionAbove50ID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve version above DN 50 ID.
	TCHAR m_strPICvPNAbove50ID[ _ID_LENGTH + 1 ];				// Version 2: Last selected pressure independent control valve PN above DN 50 ID.

	TCHAR m_strPICvActPowerSupplyID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve power supply actuator ID.
	TCHAR m_strPICvActInputSignalID[ _ID_LENGTH + 1 ];			// Version 2: Last selected pressure independent control valve input signal actuator ID.
	int m_iPICvFailSafeFct;										// Version 2: Last selected pressure independent control valve fail-safe function.
	CDB_ControlValve::DRPFunction m_ePICvDRPFct;				// Version 8: Last selected pressure independent control valve default return position function.
	
	// Additional parameters for balancing and control valves.
	int m_iBCvRadioFlow;										// Version 3: Last 'Flow' radio state.
	int m_iBCvCheckDp;											// Version 3: Last 'Dp' checkbox state.
	bool m_bBCvCtrlTypeStrictChecked;							// Version 7: Last 'Strict' checkbox state.
	CDB_ControlProperties::CV2W3W m_eBCv2W3W;					// Version 6: Last selected into combo 2W3W.
	CDB_ControlProperties::CvCtrlType m_eBCvCBCtrlType;			// Version 3: Last selected into combo ctrl type.
	TCHAR m_strBCvTypeBelow65ID[ _ID_LENGTH + 1 ];				// Version 3: Last selected balancing & control valve type below DN 65 ID
	TCHAR m_strBCvFamilyBelow65ID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve family below DN 65 ID.
	TCHAR m_strBCvMaterialBelow65ID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve material below DN 65 ID.
	TCHAR m_strBCvConnectBelow65ID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve connection below DN 65 ID.
	TCHAR m_strBCvVersionBelow65ID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve version below DN 65 ID.
	TCHAR m_strBCvPNBelow65ID[ _ID_LENGTH + 1 ];				// Version 3: Last selected balancing & control valve PN below DN 65 ID.
	
	TCHAR m_strBCvTypeAbove50ID[ _ID_LENGTH + 1 ];				// Version 3: Last selected balancing & control valve type above DN 50 ID.
	TCHAR m_strBCvFamilyAbove50ID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve family above DN 50 ID.
	TCHAR m_strBCvMaterialAbove50ID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve material above DN 50 ID.
	TCHAR m_strBCvConnectAbove50ID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve connection above DN 50 ID.
	TCHAR m_strBCvVersionAbove50ID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve version above DN 50 ID.
	TCHAR m_strBCvPNAbove50ID[ _ID_LENGTH + 1 ];				// Version 3: Last selected balancing & control valve PN above DN 50 ID.

	TCHAR m_strBCvActPowerSupplyID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve power supply actuator ID.
	TCHAR m_strBCvActInputSignalID[ _ID_LENGTH + 1 ];			// Version 3: Last selected balancing & control valve input signal actuator ID.
	int m_iBCvFailSafeFct;										// Version 3: Last selected balancing & control valve fail-safe function.
	CDB_ControlValve::DRPFunction m_eBCvDRPFct;					// Version 8: Last selected balancing & control valve Default return position function.

	// Additional parameters for separators.
	int m_iSepRadioFlow;										// Version 4: Last 'Flow' radio state.
	TCHAR m_strSeparatorTypeID[ _ID_LENGTH + 1 ];				// Version 4: Last selected separator type.
	TCHAR m_strSeparatorFamilyID[ _ID_LENGTH + 1 ];				// Version 4: Last selected separator family.
	TCHAR m_strSeparatorConnectID[ _ID_LENGTH + 1 ];			// Version 4: Last selected separator connection.
	TCHAR m_strSeparatorVersionID[ _ID_LENGTH + 1 ];			// Version 4: Last selected separator version.

	// Additional parameters for Dp controller.
	int m_iDpCRadioFlow;										// Version 5: Last 'Flow' radio state.
	
	// HYS-1188: We split 'm_iDpCCheckDpKvs' in two variables. The old one will be saved in 'm_iDpCCheckDpBranch'.
	int m_iDpCCheckDpBranch;									// Version 5: Last 'Dp branch' checkbox state.
	int m_iDpCCheckKvs;											// Version 9: HYS-1188: Last 'Kvs control valve' checkbox state.
	int m_iDpCCheckDpMax;										// Version 5: Last 'Dp max' checkbox state.
	int m_iDpCDpStab;											// Version 5: Last Dp Stab option (branch/control valve).
	int m_iDpCDpCLocation;										// Version 5: Last DpC location (supply/return)
	int m_iDpCMvLocation;										// Version 5: Last measuring valve location (primary/secondary).
	TCHAR m_strDpCTypeBelow65ID[ _ID_LENGTH + 1 ];				// Version 5: Last selected Dp controller valve type below DN 65 ID
	TCHAR m_strDpCFamilyBelow65ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected Dp controller valve family below DN 65 ID.
	TCHAR m_strDpCMaterialBelow65ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected Dp controller valve material below DN 65 ID.
	TCHAR m_strDpCConnectBelow65ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected Dp controller valve connection below DN 65 ID.
	TCHAR m_strDpCVersionBelow65ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected Dp controller valve version below DN 65 ID.
	TCHAR m_strDpCPNBelow65ID[ _ID_LENGTH + 1 ];				// Version 5: Last selected Dp controller valve PN below DN 65 ID.
	TCHAR m_strDpCBvTypeBelow65ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected balancing valve selected with DpC type below DN 65 ID
	TCHAR m_strDpCBvFamilyBelow65ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected balancing valve selected with DpC family below DN 65 ID.
	TCHAR m_strDpCBvMaterialBelow65ID[ _ID_LENGTH + 1 ];		// Version 5: Last selected balancing valve selected with DpC material below DN 65 ID.
	TCHAR m_strDpCBvConnectBelow65ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected balancing valve selected with DpC connection below DN 65 ID.
	TCHAR m_strDpCBvVersionBelow65ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected balancing valve selected with DpC version below DN 65 ID.
	TCHAR m_strDpCBvPNBelow65ID[ _ID_LENGTH + 1 ];				// Version 5: Last selected balancing valve selected with DpC PN below DN 65 ID.

	TCHAR m_strDpCTypeAbove50ID[ _ID_LENGTH + 1 ];				// Version 5: Last selected Dp controller valve type above DN 50 ID.
	TCHAR m_strDpCFamilyAbove50ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected Dp controller valve family above DN 50 ID.
	TCHAR m_strDpCMaterialAbove50ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected Dp controller valve material above DN 50 ID.
	TCHAR m_strDpCConnectAbove50ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected Dp controller valve connection above DN 50 ID.
	TCHAR m_strDpCVersionAbove50ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected Dp controller valve version above DN 50 ID.
	TCHAR m_strDpCPNAbove50ID[ _ID_LENGTH + 1 ];				// Version 5: Last selected Dp controller valve PN above DN 50 ID.
	TCHAR m_strDpCBvTypeAbove50ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected balancing valve selected with DpC type above DN 50 ID.
	TCHAR m_strDpCBvFamilyAbove50ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected balancing valve selected with DpC family above DN 50 ID.
	TCHAR m_strDpCBvMaterialAbove50ID[ _ID_LENGTH + 1 ];		// Version 5: Last selected balancing valve selected with DpC material above DN 50 ID.
	TCHAR m_strDpCBvConnectAbove50ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected balancing valve selected with DpC connection above DN 50 ID.
	TCHAR m_strDpCBvVersionAbove50ID[ _ID_LENGTH + 1 ];			// Version 5: Last selected balancing valve selected with DpC version above DN 50 ID.
	TCHAR m_strDpCBvPNAbove50ID[ _ID_LENGTH + 1 ];				// Version 5: Last selected balancing valve selected with DpC PN above DN 50 ID.

	// Additional parameters for smart control valves.
	int m_iSmartControlValveRadioFlow;									// Version 10: Last 'Flow' radio state.
	TCHAR m_strSmartControlValveMaterialBelow65ID[ _ID_LENGTH + 1 ];	// Version 10: Last selected smart control valve material below DN 65 ID.
	TCHAR m_strSmartControlValveConnectBelow65ID[ _ID_LENGTH + 1 ];		// Version 10: Last selected smart control valve connection below DN 65 ID.
	TCHAR m_strSmartControlValvePNBelow65ID[ _ID_LENGTH + 1 ];			// Version 10: Last selected smart control valve PN below DN 65 ID.
	
	TCHAR m_strSmartControlValveMaterialAbove50ID[ _ID_LENGTH + 1 ];	// Version 10: Last selected smart control valve material above DN 50 ID.
	TCHAR m_strSmartControlValveConnectAbove50ID[ _ID_LENGTH + 1 ];		// Version 10: Last selected smart control valve connection above DN 50 ID.
	TCHAR m_strSmartControlValvePNAbove50ID[ _ID_LENGTH + 1 ];			// Version 10: Last selected smart control valve PN above DN 50 ID.

	// Additional parameters for smart differential pressure controllers.
	int m_iSmartDpCRadioFlow;											// Version 11: Last 'Flow' radio state.
	int m_iSmartDpCCheckDpBranch;										// Version 11: Last 'Dp branch' checkbox state.
	int m_iSmartDpCCheckDpMax;											// Version 11: Last 'Dp max' checkbox state.
	TCHAR m_strSmartDpCMaterialBelow65ID[ _ID_LENGTH + 1 ];				// Version 11: Last selected smart differential pressure controller material below DN 65 ID.
	TCHAR m_strSmartDpCConnectBelow65ID[ _ID_LENGTH + 1 ];				// Version 11: Last selected smart differential pressure controller connection below DN 65 ID.
	TCHAR m_strSmartDpCPNBelow65ID[ _ID_LENGTH + 1 ];					// Version 11: Last selected smart differential pressure controller PN below DN 65 ID.
	
	TCHAR m_strSmartDpCMaterialAbove50ID[ _ID_LENGTH + 1 ];				// Version 11: Last selected smart differential pressure controller material above DN 50 ID.
	TCHAR m_strSmartDpCConnectAbove50ID[ _ID_LENGTH + 1 ];				// Version 11: Last selected smart differential pressure controller connection above DN 50 ID.
	TCHAR m_strSmartDpCPNAbove50ID[ _ID_LENGTH + 1 ];					// Version 11: Last selected smart differential pressure controller PN above DN 50 ID.
};

class CDS_BatchSelParameter : public CDS_SelParameterBase, protected BatchSelectionParameters
{
	DEFINE_DATA( CDS_BatchSelParameter )
	
protected:
	CDS_BatchSelParameter( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_BatchSelParameter() {}

public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Class helper to manage column width for all windows.
	class CCWBatchSelContainer;
	class CCWBatchSelWindow;
	class CCWBatchSelSheet : public CCWSheet
	{
	public:
		CCWBatchSelSheet() {}
		CCWBatchSelSheet( short nSheetID ) : CCWSheet( nSheetID ) {}
		~CCWBatchSelSheet();

		typedef struct _ColumnInfo
		{
			bool m_bIsVisible;
			long m_lWidth;
			int m_iAfterColumnID;
			struct _ColumnInfo()
			{
				m_bIsVisible = false;
				m_lWidth = -1;
				m_iAfterColumnID = -1;
			};
		}ColumnInfo;
		typedef std::map<short, ColumnInfo> mapShortColumnInfo;
		typedef mapShortColumnInfo::iterator mapShortColumnInfoIter;
		mapShortColumnInfo& GetMap( void ) { return m_mapColumnInfo; }

		virtual void ReadRegistry( CMemFile& clCWSheet );
		virtual void ReadRegistryOldVersion( CMemFile& clCWBurst ) { /* NO OLD VERSION FOR BATCH SELECTIN */ }
		virtual void WriteRegistry( CMemFile& clCWSheet );
	private:
		mapShortColumnInfo m_mapColumnInfo;
	};

	class CCWBatchSelWindow : public CCWWindow
	{
	public:
		CCWBatchSelWindow( short nWindowID ) : CCWWindow( nWindowID ) {}
	protected:
		virtual CCWSheet* CreateNewCWSSheet( short nSheetID );
	};

	class CCWBatchSelContainer : public CCWContainer
	{
	protected:
		virtual CCWWindow* CreateNewCWSWindow( short nWindowID );
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	enum SelProdType
	{
		BCv,
		Bv,
		Cv,
		DpC,
		PICv,
		TRv,
		Separator
	};

public:
	// Get parameters common to all selections.
	LPCTSTR GetPipeSeriesID( CProductSelelectionParameters *pclProductSelParams ) { return _VerifyPipeSeries( pclProductSelParams ); }		// Version 1.
	LPCTSTR GetPipeSizeID() { return m_strPipeSizeID; }																						// Version 1.

	// Get additional parameters for balancing valves.
	int GetBvRadioFlowState() { return m_iBvRadioFlow; }						// Version 1.
	int GetBvDpCheckboxState() { return m_iBvCheckDp; }							// Version 1.
	LPCTSTR GetBvTypeBelow65ID() { return m_strBvTypeBelow65ID; }				// Version 1.
	LPCTSTR GetBvFamilyBelow65ID() { return m_strBvFamilyBelow65ID; }			// Version 1.
	LPCTSTR GetBvMaterialBelow65ID() { return m_strBvMaterialBelow65ID; }		// Version 1.
	LPCTSTR GetBvConnectBelow65ID() { return m_strBvConnectBelow65ID; }			// Version 1.
	LPCTSTR GetBvVersionBelow65ID() { return m_strBvVersionBelow65ID; }			// Version 1.
	LPCTSTR GetBvPNBelow65ID() { return m_strBvPNBelow65ID; }					// Version 1.
	LPCTSTR GetBvTypeAbove50ID() { return m_strBvTypeAbove50ID; }				// Version 1.
	LPCTSTR GetBvFamilyAbove50ID() { return m_strBvFamilyAbove50ID; }			// Version 1.
	LPCTSTR GetBvMaterialAbove50ID() { return m_strBvMaterialAbove50ID; }		// Version 1.
	LPCTSTR GetBvConnectAbove50ID() { return m_strBvConnectAbove50ID; }			// Version 1.
	LPCTSTR GetBvVersionAbove50ID() { return m_strBvVersionAbove50ID; }			// Version 1.
	LPCTSTR GetBvPNAbove50ID() { return m_strBvPNAbove50ID; }					// Version 1.

	// Get additional parameters for pressure independent control valves.
	int GetPICvRadioFlowState() { return m_iPICvRadioFlow; }									// Version 2.
	bool GetPICvCtrlTypeStrictChecked() { return m_bPICvCtrlTypeStrictChecked; }				// Version 7.
	CDB_ControlProperties::CvCtrlType GetPICvCBCtrlType() { return m_ePICvCBCtrlType; }			// Version 2.
	LPCTSTR GetPICvTypeBelow65ID() { return m_strPICvTypeBelow65ID; }							// Version 2.
	LPCTSTR GetPICvFamilyBelow65ID() { return m_strPICvFamilyBelow65ID; }						// Version 2.
	LPCTSTR GetPICvMaterialBelow65ID() { return m_strPICvMaterialBelow65ID; }					// Version 2.
	LPCTSTR GetPICvConnectBelow65ID() { return m_strPICvConnectBelow65ID; }						// Version 2.
	LPCTSTR GetPICvVersionBelow65ID() { return m_strPICvVersionBelow65ID; }						// Version 2.
	LPCTSTR GetPICvPNBelow65ID() { return m_strPICvPNBelow65ID; }								// Version 2.
	LPCTSTR GetPICvTypeAbove50ID() { return m_strPICvTypeAbove50ID; }							// Version 2.
	LPCTSTR GetPICvFamilyAbove50ID() { return m_strPICvFamilyAbove50ID; }						// Version 2.
	LPCTSTR GetPICvMaterialAbove50ID() { return m_strPICvMaterialAbove50ID; }					// Version 2.
	LPCTSTR GetPICvConnectAbove50ID() { return m_strPICvConnectAbove50ID; }						// Version 2.
	LPCTSTR GetPICvVersionAbove50ID() { return m_strPICvVersionAbove50ID; }						// Version 2.
	LPCTSTR GetPICvPNAbove50ID() { return m_strPICvPNAbove50ID; }								// Version 2.
	
	LPCTSTR GetPICvActPowerSupplyID() { return m_strPICvActPowerSupplyID; }						// Version 2.
	LPCTSTR GetPICvActInputSignalID() { return m_strPICvActInputSignalID; }						// Version 2.
	int GetPICvFailSafeFct() { return m_iPICvFailSafeFct; }										// Version 2.
	CDB_ControlValve::DRPFunction GetPICvDRPFct() { return m_ePICvDRPFct; }						// Version 8.

	// Get additional parameters for balancing & control valves.
	int GetBCvRadioFlowState() { return m_iBCvRadioFlow; }										// Version 3.
	int GetBCvDpCheckboxState() { return m_iBCvCheckDp; }										// Version 3.
	bool GetBCvCtrlTypeStrictChecked() { return m_bBCvCtrlTypeStrictChecked; }					// Version 7.
	CDB_ControlProperties::CV2W3W GetBCvCB2W3W() { return m_eBCv2W3W; }							// Version 6.
	CDB_ControlProperties::CvCtrlType GetBCvCBCtrlType() { return m_eBCvCBCtrlType; }			// Version 3.
	LPCTSTR GetBCvTypeBelow65ID() { return m_strBCvTypeBelow65ID; }								// Version 3.
	LPCTSTR GetBCvFamilyBelow65ID() { return m_strBCvFamilyBelow65ID; }							// Version 3.
	LPCTSTR GetBCvMaterialBelow65ID() { return m_strBCvMaterialBelow65ID; }						// Version 3.
	LPCTSTR GetBCvConnectBelow65ID() { return m_strBCvConnectBelow65ID; }						// Version 3.
	LPCTSTR GetBCvVersionBelow65ID() { return m_strBCvVersionBelow65ID; }						// Version 3.
	LPCTSTR GetBCvPNBelow65ID() { return m_strBCvPNBelow65ID; }									// Version 3.
	LPCTSTR GetBCvTypeAbove50ID() { return m_strBCvTypeAbove50ID; }								// Version 3.
	LPCTSTR GetBCvFamilyAbove50ID() { return m_strBCvFamilyAbove50ID; }							// Version 3.
	LPCTSTR GetBCvMaterialAbove50ID() { return m_strBCvMaterialAbove50ID; }						// Version 3.
	LPCTSTR GetBCvConnectAbove50ID() { return m_strBCvConnectAbove50ID; }						// Version 3.
	LPCTSTR GetBCvVersionAbove50ID() { return m_strBCvVersionAbove50ID; }						// Version 3.
	LPCTSTR GetBCvPNAbove50ID() { return m_strBCvPNAbove50ID; }									// Version 3.
	
	LPCTSTR GetBCvActPowerSupplyID() { return m_strBCvActPowerSupplyID; }						// Version 3.
	LPCTSTR GetBCvActInputSignalID() { return m_strBCvActInputSignalID; }						// Version 3.
	int GetBCvFailSafeFct() { return m_iBCvFailSafeFct; }										// Version 3.
	CDB_ControlValve::DRPFunction GetBCvDRPFct() { return m_eBCvDRPFct; }						// Version 8.

	// Get additional parameters for separators.
	int GetSeparatorRadioFlowState(){ return m_iSepRadioFlow; }									// Version 4.
	LPCTSTR GetSeparatorTypeID() { return m_strSeparatorTypeID; }								// Version 4.
	LPCTSTR GetSeparatorFamilyID() { return m_strSeparatorFamilyID; }							// Version 4.
	LPCTSTR GetSeparatorConnectID() { return m_strSeparatorConnectID; }							// Version 4.
	LPCTSTR GetSeparatorVersionID() { return m_strSeparatorVersionID; }							// Version 4.

	// Get additional parameters for Dp controller valves.
	int GetDpCRadioFlowState() { return m_iDpCRadioFlow; }										// Version 5.
	
	// HYS-1188: We split 'm_iDpCCheckDpKvs' in two variables.
	int GetDpCDpBranchCheckboxState() { return m_iDpCCheckDpBranch; }							// Version 5.
	int GetDpCKvsCheckboxState() { return m_iDpCCheckKvs; }										// Version 9.
	
	int GetDpCDpMaxCheckboxState() { return m_iDpCCheckDpMax; }									// Version 5.
	int GetDpCDpStab() { return m_iDpCDpStab; }													// Version 5.
	int GetDpCLocation() { return m_iDpCDpCLocation; }											// Version 5.
	int GetDpCMvLocation() { return m_iDpCMvLocation; }											// Version 5.
	LPCTSTR GetDpCTypeBelow65ID() { return m_strDpCTypeBelow65ID; }								// Version 5.
	LPCTSTR GetDpCFamilyBelow65ID() { return m_strDpCFamilyBelow65ID; }							// Version 5.
	LPCTSTR GetDpCMaterialBelow65ID() { return m_strDpCMaterialBelow65ID; }						// Version 5.
	LPCTSTR GetDpCConnectBelow65ID() { return m_strDpCConnectBelow65ID; }						// Version 5.
	LPCTSTR GetDpCVersionBelow65ID() { return m_strDpCVersionBelow65ID; }						// Version 5.
	LPCTSTR GetDpCPNBelow65ID() { return m_strDpCPNBelow65ID; }									// Version 5.
	LPCTSTR GetDpCTypeAbove50ID() { return m_strDpCTypeAbove50ID; }								// Version 5.
	LPCTSTR GetDpCFamilyAbove50ID() { return m_strDpCFamilyAbove50ID; }							// Version 5.
	LPCTSTR GetDpCMaterialAbove50ID() { return m_strDpCMaterialAbove50ID; }						// Version 5.
	LPCTSTR GetDpCConnectAbove50ID() { return m_strDpCConnectAbove50ID; }						// Version 5.
	LPCTSTR GetDpCVersionAbove50ID() { return m_strDpCVersionAbove50ID; }						// Version 5.
	LPCTSTR GetDpCPNAbove50ID() { return m_strDpCPNAbove50ID; }									// Version 5.
	// For balancing valves selected with Dp controller valves.									// Version 5.
	LPCTSTR GetDpCBvTypeBelow65ID() { return m_strDpCBvTypeBelow65ID; }							// Version 5.
	LPCTSTR GetDpCBvFamilyBelow65ID() { return m_strDpCBvFamilyBelow65ID; }						// Version 5.
	LPCTSTR GetDpCBvMaterialBelow65ID() { return m_strDpCBvMaterialBelow65ID; }					// Version 5.
	LPCTSTR GetDpCBvConnectBelow65ID() { return m_strDpCBvConnectBelow65ID; }					// Version 5.
	LPCTSTR GetDpCBvVersionBelow65ID() { return m_strDpCBvVersionBelow65ID; }					// Version 5.
	LPCTSTR GetDpCBvPNBelow65ID() { return m_strDpCBvPNBelow65ID; }								// Version 5.
	LPCTSTR GetDpCBvTypeAbove50ID() { return m_strDpCBvTypeAbove50ID; }							// Version 5.
	LPCTSTR GetDpCBvFamilyAbove50ID() { return m_strDpCBvFamilyAbove50ID; }						// Version 5.
	LPCTSTR GetDpCBvMaterialAbove50ID() { return m_strDpCBvMaterialAbove50ID; }					// Version 5.
	LPCTSTR GetDpCBvConnectAbove50ID() { return m_strDpCBvConnectAbove50ID; }					// Version 5.
	LPCTSTR GetDpCBvVersionAbove50ID() { return m_strDpCBvVersionAbove50ID; }					// Version 5.
	LPCTSTR GetDpCBvPNAbove50ID() { return m_strDpCBvPNAbove50ID; }								// Version 5.

	// Get additional parameters for smart control valves.
	int GetSmartControlValveRadioFlowState() { return m_iSmartControlValveRadioFlow; }						// Version 10.
	LPCTSTR GetSmartControlValveMaterialBelow65ID() { return m_strSmartControlValveMaterialBelow65ID; }		// Version 10.
	LPCTSTR GetSmartControlValveConnectBelow65ID() { return m_strSmartControlValveConnectBelow65ID; }		// Version 10.
	LPCTSTR GetSmartControlValvePNBelow65ID() { return m_strSmartControlValvePNBelow65ID; }					// Version 10.
	LPCTSTR GetSmartControlValveMaterialAbove50ID() { return m_strSmartControlValveMaterialAbove50ID; }		// Version 10.
	LPCTSTR GetSmartControlValveConnectAbove50ID() { return m_strSmartControlValveConnectAbove50ID; }		// Version 10.
	LPCTSTR GetSmartControlValvePNAbove50ID() { return m_strSmartControlValvePNAbove50ID; }					// Version 10.

	// Get additional parameters for smart differential pressure controllers.
	int GetSmartDpCRadioFlowState() { return m_iSmartDpCRadioFlow; }						// Version 11.
	int GetSmartDpCDpBranchCheckboxState() { return m_iSmartDpCCheckDpBranch; }				// Version 11.
	int GetSmartDpCDpMaxCheckboxState() { return m_iSmartDpCCheckDpMax; }					// Version 11.
	LPCTSTR GetSmartDpCMaterialBelow65ID() { return m_strSmartDpCMaterialBelow65ID; }		// Version 11.
	LPCTSTR GetSmartDpCConnectBelow65ID() { return m_strSmartDpCConnectBelow65ID; }			// Version 11.
	LPCTSTR GetSmartDpCPNBelow65ID() { return m_strSmartDpCPNBelow65ID; }					// Version 11.
	LPCTSTR GetSmartDpCMaterialAbove50ID() { return m_strSmartDpCMaterialAbove50ID; }		// Version 11.
	LPCTSTR GetSmartDpCConnectAbove50ID() { return m_strSmartDpCConnectAbove50ID; }			// Version 11.
	LPCTSTR GetSmartDpCPNAbove50ID() { return m_strSmartDpCPNAbove50ID; }					// Version 11.

	// Set parameters common to all selections.
	void SetPipeSeriesID( LPCTSTR ID ) { if( _tcscmp( m_strPipeSeriesID, ID ) ) { _tcsncpy_s( m_strPipeSeriesID, SIZEOFINTCHAR( m_strPipeSeriesID ), ID, SIZEOFINTCHAR( m_strPipeSeriesID ) - 1 ); } }
	void SetPipeSizeID( LPCTSTR ID ) { if( _tcscmp( m_strPipeSizeID, ID ) ) { _tcsncpy_s( m_strPipeSizeID, SIZEOFINTCHAR( m_strPipeSizeID ), ID, SIZEOFINTCHAR( m_strPipeSizeID ) - 1 ); } }

	// Set additional parameters for balancing valves.
	void SetBvFlowRadioState( int iState ) { if( m_iBvRadioFlow != iState ) { m_iBvRadioFlow = iState; } }
	void SetBvDpCheckboxState( int iState ) { if( m_iBvCheckDp != iState ) { m_iBvCheckDp = iState; } }
	void SetBvTypeBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBvTypeBelow65ID, ID ) ) { _tcsncpy_s( m_strBvTypeBelow65ID, SIZEOFINTCHAR( m_strBvTypeBelow65ID ), ID, SIZEOFINTCHAR( m_strBvTypeBelow65ID ) - 1 ); } }
	void SetBvFamilyBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBvFamilyBelow65ID, ID ) ) { _tcsncpy_s( m_strBvFamilyBelow65ID, SIZEOFINTCHAR( m_strBvFamilyBelow65ID ), ID, SIZEOFINTCHAR( m_strBvFamilyBelow65ID ) - 1 ); } }
	void SetBvMaterialBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBvMaterialBelow65ID, ID ) ) { _tcsncpy_s( m_strBvMaterialBelow65ID, SIZEOFINTCHAR( m_strBvMaterialBelow65ID ), ID, SIZEOFINTCHAR( m_strBvMaterialBelow65ID ) - 1 ); } }
	void SetBvConnectBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBvConnectBelow65ID, ID ) ) { _tcsncpy_s( m_strBvConnectBelow65ID, SIZEOFINTCHAR( m_strBvConnectBelow65ID ), ID, SIZEOFINTCHAR( m_strBvConnectBelow65ID ) - 1 ); } }
	void SetBvVersionBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBvVersionBelow65ID, ID ) ) { _tcsncpy_s( m_strBvVersionBelow65ID, SIZEOFINTCHAR( m_strBvVersionBelow65ID ), ID, SIZEOFINTCHAR( m_strBvVersionBelow65ID ) - 1 ); } }
	void SetBvPNBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBvPNBelow65ID, ID ) ) { _tcsncpy_s( m_strBvPNBelow65ID, SIZEOFINTCHAR( m_strBvPNBelow65ID ), ID, SIZEOFINTCHAR( m_strBvPNBelow65ID ) - 1 ); } }
	void SetBvTypeAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBvTypeAbove50ID, ID ) ) { _tcsncpy_s( m_strBvTypeAbove50ID, SIZEOFINTCHAR( m_strBvTypeAbove50ID ), ID, SIZEOFINTCHAR( m_strBvTypeAbove50ID ) - 1 ); } }
	void SetBvFamilyAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBvFamilyAbove50ID, ID ) ) { _tcsncpy_s( m_strBvFamilyAbove50ID, SIZEOFINTCHAR( m_strBvFamilyAbove50ID ), ID, SIZEOFINTCHAR( m_strBvFamilyAbove50ID ) - 1 ); } }
	void SetBvMaterialAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBvMaterialAbove50ID, ID ) ) { _tcsncpy_s( m_strBvMaterialAbove50ID, SIZEOFINTCHAR( m_strBvMaterialAbove50ID ), ID, SIZEOFINTCHAR( m_strBvMaterialAbove50ID ) - 1 ); } }
	void SetBvConnectAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBvConnectAbove50ID, ID ) ) { _tcsncpy_s( m_strBvConnectAbove50ID, SIZEOFINTCHAR( m_strBvConnectAbove50ID ), ID, SIZEOFINTCHAR( m_strBvConnectAbove50ID ) - 1 ); } }
	void SetBvVersionAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBvVersionAbove50ID, ID ) ) { _tcsncpy_s( m_strBvVersionAbove50ID, SIZEOFINTCHAR( m_strBvVersionAbove50ID ), ID, SIZEOFINTCHAR( m_strBvVersionAbove50ID ) - 1 ); } }
	void SetBvPNAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBvPNAbove50ID, ID ) ) { _tcsncpy_s( m_strBvPNAbove50ID, SIZEOFINTCHAR( m_strBvPNAbove50ID ), ID, SIZEOFINTCHAR( m_strBvPNAbove50ID ) - 1 ); } }

	// Set additional parameters for pressure independent control valves.
	void SetPICvFlowRadioState( int iState ) { if( m_iPICvRadioFlow != iState ) { m_iPICvRadioFlow = iState; } }
	void SetPICvCtrlTypeStrictChecked( bool bState ) { if( m_bPICvCtrlTypeStrictChecked != bState ) { m_bPICvCtrlTypeStrictChecked = bState; } }		// Version 7.
	void SetPICvCBCtrlType( CDB_ControlProperties::CvCtrlType CvCtrlType ) { m_ePICvCBCtrlType = CvCtrlType; }
	void SetPICvTypeBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvTypeBelow65ID, ID ) ) { _tcsncpy_s( m_strPICvTypeBelow65ID, SIZEOFINTCHAR( m_strPICvTypeBelow65ID ), ID, SIZEOFINTCHAR( m_strPICvTypeBelow65ID ) - 1 ); } }
	void SetPICvFamilyBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvFamilyBelow65ID, ID ) ) { _tcsncpy_s( m_strPICvFamilyBelow65ID, SIZEOFINTCHAR( m_strPICvFamilyBelow65ID ), ID, SIZEOFINTCHAR( m_strPICvFamilyBelow65ID ) - 1 ); } }
	void SetPICvMaterialBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvMaterialBelow65ID, ID ) ) { _tcsncpy_s( m_strPICvMaterialBelow65ID, SIZEOFINTCHAR( m_strPICvMaterialBelow65ID ), ID, SIZEOFINTCHAR( m_strPICvMaterialBelow65ID ) - 1 ); } }
	void SetPICvConnectBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvConnectBelow65ID, ID ) ) { _tcsncpy_s( m_strPICvConnectBelow65ID, SIZEOFINTCHAR( m_strPICvConnectBelow65ID ), ID, SIZEOFINTCHAR( m_strPICvConnectBelow65ID ) - 1 ); } }
	void SetPICvVersionBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvVersionBelow65ID, ID ) ) { _tcsncpy_s( m_strPICvVersionBelow65ID, SIZEOFINTCHAR( m_strPICvVersionBelow65ID ), ID, SIZEOFINTCHAR( m_strPICvVersionBelow65ID ) - 1 ); } }
	void SetPICvPNBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvPNBelow65ID, ID ) ) { _tcsncpy_s( m_strPICvPNBelow65ID, SIZEOFINTCHAR( m_strPICvPNBelow65ID ), ID, SIZEOFINTCHAR( m_strPICvPNBelow65ID ) - 1 ); } }
	void SetPICvTypeAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvTypeAbove50ID, ID ) ) { _tcsncpy_s( m_strPICvTypeAbove50ID, SIZEOFINTCHAR( m_strPICvTypeAbove50ID ), ID, SIZEOFINTCHAR( m_strPICvTypeAbove50ID ) - 1 ); } }
	void SetPICvFamilyAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvFamilyAbove50ID, ID ) ) { _tcsncpy_s( m_strPICvFamilyAbove50ID, SIZEOFINTCHAR( m_strPICvFamilyAbove50ID ), ID, SIZEOFINTCHAR( m_strPICvFamilyAbove50ID ) - 1 ); } }
	void SetPICvMaterialAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvMaterialAbove50ID, ID ) ) { _tcsncpy_s( m_strPICvMaterialAbove50ID, SIZEOFINTCHAR( m_strPICvMaterialAbove50ID ), ID, SIZEOFINTCHAR( m_strPICvMaterialAbove50ID ) - 1 ); } }
	void SetPICvConnectAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvConnectAbove50ID, ID ) ) { _tcsncpy_s( m_strPICvConnectAbove50ID, SIZEOFINTCHAR( m_strPICvConnectAbove50ID ), ID, SIZEOFINTCHAR( m_strPICvConnectAbove50ID ) - 1 ); } }
	void SetPICvVersionAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvVersionAbove50ID, ID ) ) { _tcsncpy_s( m_strPICvVersionAbove50ID, SIZEOFINTCHAR( m_strPICvVersionAbove50ID ), ID, SIZEOFINTCHAR( m_strPICvVersionAbove50ID ) - 1 ); } }
	void SetPICvPNAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strPICvPNAbove50ID, ID ) ) { _tcsncpy_s( m_strPICvPNAbove50ID, SIZEOFINTCHAR( m_strPICvPNAbove50ID ), ID, SIZEOFINTCHAR( m_strPICvPNAbove50ID ) - 1 ); } }

	void SetPICvActPowerSupplyID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvActPowerSupplyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvActInputSignalID( LPCTSTR ID ) { _tcsncpy_s( m_strPICvActInputSignalID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetPICvFailSafeFct( int iFailSafeFct ) { m_iPICvFailSafeFct = iFailSafeFct; }
	void SetPICvDRPFct( CDB_ControlValve::DRPFunction eDRPFct ) { m_ePICvDRPFct = eDRPFct; }

	// Set additional parameters for balancing & control valves.
	void SetBCvFlowRadioState( int iState ) { if( m_iBCvRadioFlow != iState ) { m_iBCvRadioFlow = iState; } }
	void SetBCvDpCheckboxState( int iState ) { if( m_iBCvCheckDp != iState ) { m_iBCvCheckDp = iState; } }
	void SetBCvCtrlTypeStrictChecked( bool bState ) { if( m_bBCvCtrlTypeStrictChecked != bState ) { m_bBCvCtrlTypeStrictChecked = bState; } }		// Version 7.
	void SetBCVCB2W3W( CDB_ControlProperties::CV2W3W Cv2w3w ) { m_eBCv2W3W = Cv2w3w; }		// Version 6.
	void SetBCvCBCtrlType( CDB_ControlProperties::CvCtrlType CvCtrlType ) { m_eBCvCBCtrlType = CvCtrlType; }
	void SetBCvTypeBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvTypeBelow65ID, ID ) ) { _tcsncpy_s( m_strBCvTypeBelow65ID, SIZEOFINTCHAR( m_strBCvTypeBelow65ID ), ID, SIZEOFINTCHAR( m_strBCvTypeBelow65ID ) - 1 ); } }
	void SetBCvFamilyBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvFamilyBelow65ID, ID ) ) { _tcsncpy_s( m_strBCvFamilyBelow65ID, SIZEOFINTCHAR( m_strBCvFamilyBelow65ID ), ID, SIZEOFINTCHAR( m_strBCvFamilyBelow65ID ) - 1 ); } }
	void SetBCvMaterialBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvMaterialBelow65ID, ID ) ) { _tcsncpy_s( m_strBCvMaterialBelow65ID, SIZEOFINTCHAR( m_strBCvMaterialBelow65ID ), ID, SIZEOFINTCHAR( m_strBCvMaterialBelow65ID ) - 1 ); } }
	void SetBCvConnectBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvConnectBelow65ID, ID ) ) { _tcsncpy_s( m_strBCvConnectBelow65ID, SIZEOFINTCHAR( m_strBCvConnectBelow65ID ), ID, SIZEOFINTCHAR( m_strBCvConnectBelow65ID ) - 1 ); } }
	void SetBCvVersionBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvVersionBelow65ID, ID ) ) { _tcsncpy_s( m_strBCvVersionBelow65ID, SIZEOFINTCHAR( m_strBCvVersionBelow65ID ), ID, SIZEOFINTCHAR( m_strBCvVersionBelow65ID ) - 1 ); } }
	void SetBCvPNBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvPNBelow65ID, ID ) ) { _tcsncpy_s( m_strBCvPNBelow65ID, SIZEOFINTCHAR( m_strBCvPNBelow65ID ), ID, SIZEOFINTCHAR( m_strBCvPNBelow65ID ) - 1 ); } }
	void SetBCvTypeAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvTypeAbove50ID, ID ) ) { _tcsncpy_s( m_strBCvTypeAbove50ID, SIZEOFINTCHAR( m_strBCvTypeAbove50ID ), ID, SIZEOFINTCHAR( m_strBCvTypeAbove50ID ) - 1 ); } }
	void SetBCvFamilyAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvFamilyAbove50ID, ID ) ) { _tcsncpy_s( m_strBCvFamilyAbove50ID, SIZEOFINTCHAR( m_strBCvFamilyAbove50ID ), ID, SIZEOFINTCHAR( m_strBCvFamilyAbove50ID ) - 1 ); } }
	void SetBCvMaterialAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvMaterialAbove50ID, ID ) ) { _tcsncpy_s( m_strBCvMaterialAbove50ID, SIZEOFINTCHAR( m_strBCvMaterialAbove50ID ), ID, SIZEOFINTCHAR( m_strBCvMaterialAbove50ID ) - 1 ); } }
	void SetBCvConnectAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvConnectAbove50ID, ID ) ) { _tcsncpy_s( m_strBCvConnectAbove50ID, SIZEOFINTCHAR( m_strBCvConnectAbove50ID ), ID, SIZEOFINTCHAR( m_strBCvConnectAbove50ID ) - 1 ); } }
	void SetBCvVersionAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvVersionAbove50ID, ID ) ) { _tcsncpy_s( m_strBCvVersionAbove50ID, SIZEOFINTCHAR( m_strBCvVersionAbove50ID ), ID, SIZEOFINTCHAR( m_strBCvVersionAbove50ID ) - 1 ); } }
	void SetBCvPNAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strBCvPNAbove50ID, ID ) ) { _tcsncpy_s( m_strBCvPNAbove50ID, SIZEOFINTCHAR( m_strBCvPNAbove50ID ), ID, SIZEOFINTCHAR( m_strBCvPNAbove50ID ) - 1 ); } }

	void SetBCvActPowerSupplyID( LPCTSTR ID ) { _tcsncpy_s( m_strBCvActPowerSupplyID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCvActInputSignalID( LPCTSTR ID ) { _tcsncpy_s( m_strBCvActInputSignalID, _ID_LENGTH, ID, _ID_LENGTH ); }
	void SetBCvFailSafeFct( int iFailSafeFct ) { m_iBCvFailSafeFct = iFailSafeFct; }
	void SetBCvDRPFct( CDB_ControlValve::DRPFunction eDRPFct ) { m_eBCvDRPFct = eDRPFct; }

	// Set additional parameters for separators.
	void SetSeparatorFlowRadioState( int iState ) { if( m_iSepRadioFlow != iState ) { m_iSepRadioFlow = iState; } }
	void SetSeparatorTypeID( LPCTSTR ID ) { if( _tcscmp( m_strSeparatorTypeID, ID ) ) { _tcsncpy_s( m_strSeparatorTypeID, SIZEOFINTCHAR( m_strSeparatorTypeID ), ID, SIZEOFINTCHAR( m_strSeparatorTypeID ) - 1 ); } }
	void SetSeparatorFamilyID( LPCTSTR ID ) { if( _tcscmp( m_strSeparatorFamilyID, ID ) ) { _tcsncpy_s( m_strSeparatorFamilyID, SIZEOFINTCHAR( m_strSeparatorFamilyID ), ID, SIZEOFINTCHAR( m_strSeparatorFamilyID ) - 1 ); } }
	void SetSeparatorConnectID( LPCTSTR ID ) { if( _tcscmp( m_strSeparatorConnectID, ID ) ) { _tcsncpy_s( m_strSeparatorConnectID, SIZEOFINTCHAR( m_strSeparatorConnectID ), ID, SIZEOFINTCHAR( m_strSeparatorConnectID ) - 1 ); } }
	void SetSeparatorVersionID( LPCTSTR ID ) { if( _tcscmp( m_strSeparatorVersionID, ID ) ) { _tcsncpy_s( m_strSeparatorVersionID, SIZEOFINTCHAR( m_strSeparatorVersionID ), ID, SIZEOFINTCHAR( m_strSeparatorVersionID ) - 1 ); } }

	// Set additional parameters for Dp controller valves.
	void SetDpCFlowRadioState( int iState ) { if( m_iDpCRadioFlow != iState ) { m_iDpCRadioFlow = iState; } }
	
	// HYS-1188: We split 'm_iDpCCheckDpKvs' in two variables.
	void SetDpCDpBranchCheckboxState( int iState ) { if( m_iDpCCheckDpBranch != iState ) { m_iDpCCheckDpBranch = iState; } }
	void SetDpCKvsCheckboxState( int iState ) { if( m_iDpCCheckKvs != iState ) { m_iDpCCheckKvs = iState; } }

	void SetDpCDpMaxCheckboxState( int iState ) { if( m_iDpCCheckDpMax != iState ) { m_iDpCCheckDpMax = iState; } }
	void SetDpCDpStab( int iState ) { if( m_iDpCDpStab != iState ) { m_iDpCDpStab = iState; } }
	void SetDpCDpCLocation( int iState ) { if( m_iDpCDpCLocation != iState ) { m_iDpCDpCLocation = iState; } }
	void SetDpCMvLocation( int iState ) { if( m_iDpCMvLocation != iState ) { m_iDpCMvLocation = iState; } }
	void SetDpCTypeBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCTypeBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCTypeBelow65ID, SIZEOFINTCHAR( m_strDpCTypeBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCTypeBelow65ID ) - 1 ); } }
	void SetDpCFamilyBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCFamilyBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCFamilyBelow65ID, SIZEOFINTCHAR( m_strDpCFamilyBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCFamilyBelow65ID ) - 1 ); } }
	void SetDpCMaterialBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCMaterialBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCMaterialBelow65ID, SIZEOFINTCHAR( m_strDpCMaterialBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCMaterialBelow65ID ) - 1 ); } }
	void SetDpCConnectBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCConnectBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCConnectBelow65ID, SIZEOFINTCHAR( m_strDpCConnectBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCConnectBelow65ID ) - 1 ); } }
	void SetDpCVersionBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCVersionBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCVersionBelow65ID, SIZEOFINTCHAR( m_strDpCVersionBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCVersionBelow65ID ) - 1 ); } }
	void SetDpCPNBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCPNBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCPNBelow65ID, SIZEOFINTCHAR( m_strDpCPNBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCPNBelow65ID ) - 1 ); } }
	void SetDpCTypeAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCTypeAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCTypeAbove50ID, SIZEOFINTCHAR( m_strDpCTypeAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCTypeAbove50ID ) - 1 ); } }
	void SetDpCFamilyAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCFamilyAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCFamilyAbove50ID, SIZEOFINTCHAR( m_strDpCFamilyAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCFamilyAbove50ID ) - 1 ); } }
	void SetDpCMaterialAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCMaterialAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCMaterialAbove50ID, SIZEOFINTCHAR( m_strDpCMaterialAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCMaterialAbove50ID ) - 1 ); } }
	void SetDpCConnectAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCConnectAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCConnectAbove50ID, SIZEOFINTCHAR( m_strDpCConnectAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCConnectAbove50ID ) - 1 ); } }
	void SetDpCVersionAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCVersionAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCVersionAbove50ID, SIZEOFINTCHAR( m_strDpCVersionAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCVersionAbove50ID ) - 1 ); } }
	void SetDpCPNAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCPNAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCPNAbove50ID, SIZEOFINTCHAR( m_strDpCPNAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCPNAbove50ID ) - 1 ); } }
	// For balancing valves selected with Dp controller valves.
	void SetDpCBvTypeBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvTypeBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCBvTypeBelow65ID, SIZEOFINTCHAR( m_strDpCBvTypeBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCBvTypeBelow65ID ) - 1 ); } }
	void SetDpCBvFamilyBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvFamilyBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCBvFamilyBelow65ID, SIZEOFINTCHAR( m_strDpCBvFamilyBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCBvFamilyBelow65ID ) - 1 ); } }
	void SetDpCBvMaterialBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvMaterialBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCBvMaterialBelow65ID, SIZEOFINTCHAR( m_strDpCBvMaterialBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCBvMaterialBelow65ID ) - 1 ); } }
	void SetDpCBvConnectBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvConnectBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCBvConnectBelow65ID, SIZEOFINTCHAR( m_strDpCBvConnectBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCBvConnectBelow65ID ) - 1 ); } }
	void SetDpCBvVersionBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvVersionBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCBvVersionBelow65ID, SIZEOFINTCHAR( m_strDpCBvVersionBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCBvVersionBelow65ID ) - 1 ); } }
	void SetDpCBvPNBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvPNBelow65ID, ID ) ) { _tcsncpy_s( m_strDpCBvPNBelow65ID, SIZEOFINTCHAR( m_strDpCBvPNBelow65ID ), ID, SIZEOFINTCHAR( m_strDpCBvPNBelow65ID ) - 1 ); } }
	void SetDpCBvTypeAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvTypeAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCBvTypeAbove50ID, SIZEOFINTCHAR( m_strDpCBvTypeAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCBvTypeAbove50ID ) - 1 ); } }
	void SetDpCBvFamilyAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvFamilyAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCBvFamilyAbove50ID, SIZEOFINTCHAR( m_strDpCBvFamilyAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCBvFamilyAbove50ID ) - 1 ); } }
	void SetDpCBvMaterialAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvMaterialAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCBvMaterialAbove50ID, SIZEOFINTCHAR( m_strDpCBvMaterialAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCBvMaterialAbove50ID ) - 1 ); } }
	void SetDpCBvConnectAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvConnectAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCBvConnectAbove50ID, SIZEOFINTCHAR( m_strDpCBvConnectAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCBvConnectAbove50ID ) - 1 ); } }
	void SetDpCBvVersionAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvVersionAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCBvVersionAbove50ID, SIZEOFINTCHAR( m_strDpCBvVersionAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCBvVersionAbove50ID ) - 1 ); } }
	void SetDpCBvPNAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strDpCBvPNAbove50ID, ID ) ) { _tcsncpy_s( m_strDpCBvPNAbove50ID, SIZEOFINTCHAR( m_strDpCBvPNAbove50ID ), ID, SIZEOFINTCHAR( m_strDpCBvPNAbove50ID ) - 1 ); } }

	// Version 10: Set additional parameters for smart control valves.
	void SetSmartControlValveRadioFlowState( int iState ) { if( m_iSmartControlValveRadioFlow != iState ) { m_iSmartControlValveRadioFlow = iState; } }
	void SetSmartControlValveMaterialBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartControlValveMaterialBelow65ID, ID ) ) { _tcsncpy_s( m_strSmartControlValveMaterialBelow65ID, SIZEOFINTCHAR( m_strSmartControlValveMaterialBelow65ID ), ID, SIZEOFINTCHAR( m_strSmartControlValveMaterialBelow65ID ) - 1 ); } }
	void SetSmartControlValveConnectBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartControlValveConnectBelow65ID, ID ) ) { _tcsncpy_s( m_strSmartControlValveConnectBelow65ID, SIZEOFINTCHAR( m_strSmartControlValveConnectBelow65ID ), ID, SIZEOFINTCHAR( m_strSmartControlValveConnectBelow65ID ) - 1 ); } }
	void SetSmartControlValvePNBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartControlValvePNBelow65ID, ID ) ) { _tcsncpy_s( m_strSmartControlValvePNBelow65ID, SIZEOFINTCHAR( m_strSmartControlValvePNBelow65ID ), ID, SIZEOFINTCHAR( m_strSmartControlValvePNBelow65ID ) - 1 ); } }
	void SetSmartControlValveMaterialAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartControlValveMaterialAbove50ID, ID ) ) { _tcsncpy_s( m_strSmartControlValveMaterialAbove50ID, SIZEOFINTCHAR( m_strSmartControlValveMaterialAbove50ID ), ID, SIZEOFINTCHAR( m_strSmartControlValveMaterialAbove50ID ) - 1 ); } }
	void SetSmartControlValveConnectAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartControlValveConnectAbove50ID, ID ) ) { _tcsncpy_s( m_strSmartControlValveConnectAbove50ID, SIZEOFINTCHAR( m_strSmartControlValveConnectAbove50ID ), ID, SIZEOFINTCHAR( m_strSmartControlValveConnectAbove50ID ) - 1 ); } }
	void SetSmartControlValvePNAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartControlValvePNAbove50ID, ID ) ) { _tcsncpy_s( m_strSmartControlValvePNAbove50ID, SIZEOFINTCHAR( m_strSmartControlValvePNAbove50ID ), ID, SIZEOFINTCHAR( m_strSmartControlValvePNAbove50ID ) - 1 ); } }

	// Version 11: Set additional parameters for smart differential pressure controllers.
	void SetSmartDpCRadioFlowState( int iState ) { if( m_iSmartDpCRadioFlow != iState ) { m_iSmartDpCRadioFlow = iState; } }
	void SetSmartDpCDpBranchCheckboxState( int iState ) { if( m_iSmartDpCCheckDpBranch != iState ) { m_iSmartDpCCheckDpBranch = iState; } }
	void SetSmartDpCDpMaxCheckboxState( int iState ) { if( m_iSmartDpCCheckDpMax != iState ) { m_iSmartDpCCheckDpMax = iState; } }
	void SetSmartDpCMaterialBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartDpCMaterialBelow65ID, ID ) ) { _tcsncpy_s( m_strSmartDpCMaterialBelow65ID, SIZEOFINTCHAR( m_strSmartDpCMaterialBelow65ID ), ID, SIZEOFINTCHAR( m_strSmartDpCMaterialBelow65ID ) - 1 ); } }
	void SetSmartDpCConnectBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartDpCConnectBelow65ID, ID ) ) { _tcsncpy_s( m_strSmartDpCConnectBelow65ID, SIZEOFINTCHAR( m_strSmartDpCConnectBelow65ID ), ID, SIZEOFINTCHAR( m_strSmartDpCConnectBelow65ID ) - 1 ); } }
	void SetSmartDpCPNBelow65ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartDpCPNBelow65ID, ID ) ) { _tcsncpy_s( m_strSmartDpCPNBelow65ID, SIZEOFINTCHAR( m_strSmartDpCPNBelow65ID ), ID, SIZEOFINTCHAR( m_strSmartDpCPNBelow65ID ) - 1 ); } }
	void SetSmartDpCMaterialAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartDpCMaterialAbove50ID, ID ) ) { _tcsncpy_s( m_strSmartDpCMaterialAbove50ID, SIZEOFINTCHAR( m_strSmartDpCMaterialAbove50ID ), ID, SIZEOFINTCHAR( m_strSmartDpCMaterialAbove50ID ) - 1 ); } }
	void SetSmartDpCConnectAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartDpCConnectAbove50ID, ID ) ) { _tcsncpy_s( m_strSmartDpCConnectAbove50ID, SIZEOFINTCHAR( m_strSmartDpCConnectAbove50ID ), ID, SIZEOFINTCHAR( m_strSmartDpCConnectAbove50ID ) - 1 ); } }
	void SetSmartDpCPNAbove50ID( LPCTSTR ID ) { if( _tcscmp( m_strSmartDpCPNAbove50ID, ID ) ) { _tcsncpy_s( m_strSmartDpCPNAbove50ID, SIZEOFINTCHAR( m_strSmartDpCPNAbove50ID ), ID, SIZEOFINTCHAR( m_strSmartDpCPNAbove50ID ) - 1 ); } }

	// FF: 2014-03-27: A bug has been introduced in previous version before 4.2.3.4 concerning saving of the 'CDS_BatchSelParameter' in
	//                 a '.tsp' file. We need to verify if data in 'CDS_BatchSelParameter' are relevant.
	//                 This method is called by 'CTADatastruct::VerifyBatchSelParameter'.
	void VerifyIntegrity();

// Public methods.
public:
	// Overrides 'CDS_SelectionParameter'.
	virtual void WriteToRegistry();
	virtual void ReadFromRegistry();

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	TCHAR *_VerifyPipeSeries( CProductSelelectionParameters *pclProductSelParams );
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_WizardSelParameter
//
///////////////////////////////////////////////////////////////////////////////////
struct WizardSelectionParameters
{
	// Parameters common to all selections.
	TCHAR m_strPipeSeriesID[ _ID_LENGTH + 1 ];					// Last selected pipe series ID.
	TCHAR m_strPipeSizeID[ _ID_LENGTH + 1 ];					// Last selected pipe size ID.

	// Additional parameters for pressure maintenance - Common.
	ProjectType m_ePMApplicationType;							// Last selected 'Application type'.
	TCHAR m_strPMPressureMaintenanceTypeID[_ID_LENGTH + 1];		// Last selected 'Pressure maintenance type'.
	TCHAR m_strPMWaterMakeUpTypeID[_ID_LENGTH + 1];				// Last selected 'Water make-up'.
	int m_iPMDegassingChecked;									// Last 'Degassing' checkbox state.
	TCHAR m_strPMNorm[_ID_LENGTH + 1];							// Last selected 'Norm' (for heating only).
	PressurON m_ePMPressOn;										// Last selected 'Pressurization on'.
	bool m_fPMWaterMakeupBreakTank;								// break tank proposed by default
	bool m_fShowAllPriorities;									// 'true' if user wants to show all valves (included ones with lowest priority)
};

class CDS_WizardSelParameter : public CDS_SelParameterBase, protected WizardSelectionParameters
{
	DEFINE_DATA( CDS_WizardSelParameter )
	
protected:
	CDS_WizardSelParameter( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_WizardSelParameter() {}

public:
	// Get parameters common to all selections.
	LPCTSTR GetPipeSeriesID( CProductSelelectionParameters *pclProductSelParams ) { return _VerifyPipeSeries( pclProductSelParams ); }
	LPCTSTR GetPipeSizeID() { return m_strPipeSizeID; }

	// Get additional parameters for pressure maintenance.
	ProjectType GetPMApplicationType() { return m_ePMApplicationType; }
	LPCTSTR GetPMPressMaintTypeID() { return m_strPMPressureMaintenanceTypeID; }
	LPCTSTR GetPMWaterMakeUpTypeID() { return m_strPMWaterMakeUpTypeID; }
	int GetPMDegassingChecked() { return m_iPMDegassingChecked; }
	LPCTSTR GetPMNorm() { return m_strPMNorm; }
	PressurON GetPMPressOn() { return m_ePMPressOn; }

	bool GetShowAllPriorities() { return m_fShowAllPriorities; }
	bool GetPMWaterMakeupBreakTank() { return m_fPMWaterMakeupBreakTank; }

	// Set parameters common to all selections.
	void SetPipeSeriesID( LPCTSTR ID )				{ if( _tcscmp( m_strPipeSeriesID, ID ) ) { _tcsncpy_s( m_strPipeSeriesID, SIZEOFINTCHAR( m_strPipeSeriesID ), ID, SIZEOFINTCHAR( m_strPipeSeriesID ) - 1 ); } }
	void SetPipeSizeID( LPCTSTR ID )				{ if( _tcscmp( m_strPipeSizeID, ID ) ) { _tcsncpy_s( m_strPipeSizeID, SIZEOFINTCHAR( m_strPipeSizeID ), ID, SIZEOFINTCHAR( m_strPipeSizeID ) - 1 ); } }

	// Set additional parameters for pressure maintenance.
	void SetPMApplicationType( ProjectType ProjectType ) { m_ePMApplicationType = ProjectType; }
	void SetPMPressMaintType( LPCTSTR ID ) { _tcsncpy_s( m_strPMPressureMaintenanceTypeID, SIZEOFINTCHAR( m_strPMPressureMaintenanceTypeID ), ID, SIZEOFINTCHAR( m_strPMPressureMaintenanceTypeID ) - 1 ); }
	void SetPMWaterMakeUpType( LPCTSTR ID ) { _tcsncpy_s( m_strPMWaterMakeUpTypeID, SIZEOFINTCHAR( m_strPMWaterMakeUpTypeID ), ID, SIZEOFINTCHAR( m_strPMWaterMakeUpTypeID ) - 1 ); }
	void SetPMDegassingChecked( int iDegassing ) { m_iPMDegassingChecked = iDegassing; }
	void SetPMNorm( LPCTSTR ID ) { _tcsncpy_s( m_strPMNorm, SIZEOFINTCHAR( m_strPMNorm ), ID, SIZEOFINTCHAR( m_strPMNorm ) - 1 ); }
	void SetPMPressOn( PressurON pressOn ) { m_ePMPressOn = pressOn; }

	void SetShowAllPriorities( bool fShowAllPriorities ) { m_fShowAllPriorities = fShowAllPriorities; }
	void SetPMWaterMakeupBreakTank( bool fWaterMakeupBreakTank) { m_fPMWaterMakeupBreakTank = fWaterMakeupBreakTank; }

// Public methods.
public:
	// Overrides 'CDS_SelectionParameter'.
	virtual void WriteToRegistry();
	virtual void ReadFromRegistry();

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	TCHAR *_VerifyPipeSeries( CProductSelelectionParameters *pclProductSelParams );
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SelThing
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SelThing : public CDS_WaterCharacteristic
{
	DEFINE_DATA( CDS_SelThing )

protected:
	CDS_SelThing( CDataBase *pDataBase, LPCTSTR ID );
	~CDS_SelThing() {}

// Public methods.
public:
	LPCTSTR GetModuleID() { return m_tcHydroModID; }
	const IDPTR &GetValveIDPtr();
	LPCTSTR GetID();	
	LPCTSTR GetArtNum();
	LPCTSTR GetName();		
	LPCTSTR GetVersionID();	
	LPCTSTR GetFamilyID();	
	LPCTSTR GetTypeID();	
	LPCTSTR GetSizeID();
	IDPTR GetSizeIDPtr();
	int GetSizeKey();
	LPCTSTR GetConnectID();	

	void GetWC( CWaterChar *pwc );
	LPCTSTR	GetRef1() { return m_tcReference1; }
	LPCTSTR	GetRef2() { return m_tcReference2; }
	int GetQuantity() { return m_iQuantity; }
	double GetPrice() { return m_dPrice; }
	LPCTSTR	GetRemark() { return m_tcRemark; }
	int GetRemarkIndex() { return m_iRemarkIndex; }
	int GetRowIndex() { return m_iRowIndex; }
	LPCTSTR GetCBIType() { return m_tcCBITypeID; }
	LPCTSTR GetCBISize() { return m_tcCBISizeID; }
	LPCTSTR GetCBISizeInch() { return m_tcCBISizeInchID; }
	

	void SetModuleID( LPCTSTR ID ) { if(_tcscmp( m_tcHydroModID, ID ) ) { _tcsncpy_s( m_tcHydroModID, SIZEOFINTCHAR( m_tcHydroModID ), ID, SIZEOFINTCHAR( m_tcHydroModID ) - 1 ); Modified(); } }
	void SetID( LPCTSTR ID );
	void SetWC( CWaterChar *pwc );
	void SetRef1( LPCTSTR strReference1 ) { if( _tcscmp( m_tcReference1, strReference1 ) ) { _tcsncpy_s( m_tcReference1, SIZEOFINTCHAR( m_tcReference1 ), strReference1, SIZEOFINTCHAR( m_tcReference1 ) - 1 ); Modified(); } }
	void SetRef2( LPCTSTR strReference2 ) { if( _tcscmp( m_tcReference2, strReference2 ) ) { _tcsncpy_s( m_tcReference2, SIZEOFINTCHAR( m_tcReference2 ), strReference2, SIZEOFINTCHAR( m_tcReference2 ) - 1 ); Modified(); } }
	void SetQuantity( int iQuantity ) { if( ( m_iQuantity != iQuantity ) ) { m_iQuantity = iQuantity; Modified(); } }
	void SetPrice( double dPrice ) { if( ( m_dPrice != dPrice ) ) { m_dPrice = dPrice; Modified(); } }
	void SetRemark( LPCTSTR strRemark )	{ if( _tcscmp( m_tcRemark, strRemark ) ) { _tcsncpy_s( m_tcRemark, SIZEOFINTCHAR( m_tcRemark ), strRemark, SIZEOFINTCHAR( m_tcRemark ) - 1 ); Modified(); } }
	void SetRemarkIndex( int iIndex ) { m_iRemarkIndex = iIndex; } // don't change modified flag, Remark index is just used for visualization
	void SetRowIndex( int iIndex ) { if( m_iRowIndex != iIndex ) { m_iRowIndex = iIndex; Modified(); } }
	void SetCBIType( LPCTSTR CBITypeID ) { if( _tcscmp( m_tcCBITypeID, CBITypeID ) ) { _tcsncpy_s( m_tcCBITypeID, SIZEOFINTCHAR( m_tcCBITypeID ), CBITypeID, SIZEOFINTCHAR( m_tcCBITypeID ) - 1 ); Modified(); } }
	void SetCBISize( LPCTSTR CBISizeID ) { if( _tcscmp( m_tcCBISizeID, CBISizeID ) ) { _tcsncpy_s( m_tcCBISizeID, SIZEOFINTCHAR( m_tcCBISizeID ), CBISizeID, SIZEOFINTCHAR( m_tcCBISizeID ) - 1 ); Modified(); }  }
	void SetCBISizeInch( LPCTSTR CBISizeInchID ) { if( _tcscmp( m_tcCBISizeInchID, CBISizeInchID ) ) { _tcsncpy_s( m_tcCBISizeInchID, SIZEOFINTCHAR( m_tcCBISizeInchID ), CBISizeInchID, SIZEOFINTCHAR( m_tcCBISizeInchID ) - 1 ); Modified(); } }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' public virtual methods.
	virtual void Copy( CData *pclDestination );

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData *pclSelectionCompareWith, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	IDPTR m_ValveIDPtr;
	TCHAR m_tcHydroModID[ _ID_LENGTH + 1 ];				// Hydraulic module ID where this selected valve is used.
	TCHAR m_tcCBITypeID[_CBI_TYPE_LENGTH + 1];
	TCHAR m_tcCBISizeID[_CBI_SIZE_LENGTH + 1];
	TCHAR m_tcCBISizeInchID[_CBI_SIZE_LENGTH + 1];
	TCHAR m_tcReference1[_STRING_LENGTH];				// User Reference 
	TCHAR m_tcReference2[_STRING_LENGTH];				// User Reference 
	int m_iQuantity;									// Number of same Items.
	double m_dPrice;
	TCHAR m_tcRemark[_DBLSTRING_LENGTHSTR2];			// User remark.
	int m_iRemarkIndex;									// Remark Index used for visualisation; computed each time the table is displayed.
	int m_iRowIndex;									// Position in Table used for visualization.
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SelHydroThing
//
///////////////////////////////////////////////////////////////////////////////////

struct SelHydroThing_struct {
	double Flow;								//
	double Dp;									// 
	double Setting;								// Setting of the valve
	bool TFlag;									// Temperature validity flag
	bool PNFlag;								// PN (Static pressure) validity flag
	TCHAR PipeName[_TABLE_NAME_LENGTH+1];		// Pipe Serie
	TCHAR PipeSize[_PIPE_NAME_LENGTH+1];			// Pipe Size
	double LinDp;								// Linear Dp
	bool LinDpFlag;								// Linear Dp validity flag
	double U;									// Water velocity
	bool UFlag;									// Water velocity validity flag
	bool TpFlag;								// Temperature and pressure validity flag
};

	
class CDS_SelHydroThing : public CDS_SelThing , protected SelHydroThing_struct
{
	DEFINE_DATA(CDS_SelHydroThing)

// Attribute
public:
	
// Constructor and destructor.
protected:
	CDS_SelHydroThing(CDataBase* pDataBase, LPCTSTR ID);
	~CDS_SelHydroThing() {}

// Access
public:
	double GetFlow() { return Flow; }
	double GetDp() { return Dp; }
	double GetSetting() { return Setting; }
	bool GetTFlag() { return TFlag; }
	bool GetPNFlag() { return PNFlag ; }
	LPCTSTR GetPipeName() { return PipeName; }
	LPCTSTR GetPipeSize() { return PipeSize; }
	double GetLinDp() { return LinDp; }
	bool GetLinDpFlag() { return LinDpFlag; }
	double GetU() { return U; }
	bool GetUFlag() { return UFlag; }
	bool GetTpFlag() { return TpFlag; }


	void SetFlow( double dFlow ) { if( ( Flow != dFlow ) ) { Flow = dFlow; Modified(); } }
	void SetDp( double dDp) { if( ( Dp != dDp ) ) { Dp = dDp; Modified(); } }
	void SetSetting ( double dSetting) { if( ( Setting != dSetting ) ) { Setting = dSetting; Modified(); } }
	void SetTFlag( bool dTFlag) { if( ( TFlag != dTFlag ) ) { TFlag = dTFlag; Modified(); } }
	void SetPNFlag( bool dPNFlag ) { if( ( PNFlag!=dPNFlag ) ) { PNFlag = dPNFlag; Modified(); } }
	void SetPipeName( LPCTSTR ptcPipeName) { if( 0 != _tcscmp( PipeName, ptcPipeName ) ) { _tcsncpy_s( PipeName, SIZEOFINTCHAR( PipeName ), ptcPipeName, SIZEOFINTCHAR( PipeName ) - 1); Modified(); } }
	void SetPipeSize( LPCTSTR pcPipeSize) { if( 0 == _tcscmp( PipeSize, pcPipeSize ) ) { _tcsncpy_s( PipeSize, SIZEOFINTCHAR( PipeSize ), pcPipeSize, SIZEOFINTCHAR( PipeSize ) - 1 ); Modified(); } };
	void SetLinDp( double dLinearDp ) { if( ( LinDp != dLinearDp ) ) { LinDp = dLinearDp; Modified(); } }
	void SetLinDpFlag( bool bLinearDpFlag ) { if( ( LinDpFlag != bLinearDpFlag ) ) { LinDpFlag = bLinearDpFlag; Modified(); } }
	void SetU( double dU ) { if( ( U!=dU ) ) { U = dU; Modified(); } }
	void SetUFlag( bool bUFlag ) { if( ( UFlag != bUFlag ) ) { UFlag = bUFlag; Modified(); } }
	void SetTpFlag( bool bTPFlag ) { if( ( TpFlag != bTPFlag ) ) { TpFlag = bTPFlag; Modified(); } }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SelManBv
//
///////////////////////////////////////////////////////////////////////////////////

struct SelManBv_struct {
	double Dp2;									// BV Dp 100% ; FO or CS Dp
	double Dp3;									// BV Dp 50% ; FO or CS Not Used
};

class CDS_SelManBv : public CDS_SelHydroThing , protected SelManBv_struct
{
	DEFINE_DATA(CDS_SelManBv)

// Constructor and destructor.
protected:
	CDS_SelManBv(CDataBase* pDataBase, LPCTSTR ID);
	~CDS_SelManBv() {}

// Public methods.s
public:
	double GetDp2() { return Dp2; }
	double GetDp3() { return Dp3; }

	void SetDp2( double dDp ) { if( ( Dp2 != dDp ) ) { Dp2 = dDp; Modified(); } }
	void SetDp3( double dDp ) { if( ( Dp3 != dDp ) ) { Dp3 = dDp; Modified(); } }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SelDpC
//
///////////////////////////////////////////////////////////////////////////////////

struct SelDpC_struct {
	CString SchemeName;							// Not used anymore, kept for DS compatibility, could be removed when the DS version change
	int	 SchemeIndex;							// Used only for visualisation
	double Hmin;								// Minimum primary Dp
	double Authority;							// Authority
	double Dpmin;								// Minimum Dp of the DpC to get q and DpL
	bool DpminFlag;
	double DpL;									// Dp load as defined in catalogue
	double DpBranch;							// Dp branch = Dp of the branch or Dp control valve
	double DpCSetting;							// Setting of DpC
	double KvCv;								// Kv or Cv value of the control valve
	int DpStab;									// 0: on branch / 1: on control valve
	int MvLoc;									// 0: MV in primary / 1: MV in secondary
	bool DpCTFlag;
	bool DpCPNFlag;	
	double DpCPrice;
	double DpCFlow;								// Store DpC flow when it's different of Bv flow
};

class CDS_SelDpC : public CDS_SelManBv , protected SelDpC_struct
{
	DEFINE_DATA(CDS_SelDpC)

// Attribute
public:
	
// Constructor and destructor.
protected:
	CDS_SelDpC(CDataBase* pDataBase, LPCTSTR ID);
	~CDS_SelDpC() {}

	IDPTR m_DpCIDPtr;

public:
	void SetDpCID( LPCTSTR ID );
	void SetSchemeIndex( int iIndex ) { SchemeIndex = iIndex; }	// SchemeIndex is only used for visualization
	void SetHmin( double dHMin ) { if( Hmin != dHMin ) { Hmin = dHMin; Modified(); } }
	void SetAuthority( double dAuthority ) { if( Authority != dAuthority ) { Authority = dAuthority; Modified(); } }
	void SetDpmin( double dDpMin ) { if( Dpmin != dDpMin ) { Dpmin = dDpMin; Modified(); } }
	void SetDpminFlag( bool bDpMinFlag ) { if( DpminFlag != bDpMinFlag ) { DpminFlag = bDpMinFlag; Modified(); } }
	void SetDpL( double dDpL ) { if( DpL != dDpL ) { DpL = dDpL; Modified(); } }
	void SetDpBranch( double dDpBranch ) { if( DpBranch != dDpBranch ) { DpBranch = dDpBranch; Modified(); } }
	void SetKvCv( double dKvCv ) { if( KvCv != dKvCv ) { KvCv = dKvCv; Modified(); } }
	void SetDpStab( int iDpStab ) { if( DpStab != iDpStab ) { DpStab = iDpStab; Modified(); } }
	void SetMvLoc( int iMvLoc ) { if( MvLoc != iMvLoc ) { MvLoc = iMvLoc; Modified(); } }
	void SetDpCTFlag( bool bDpCTFlag ) { if( DpCTFlag != bDpCTFlag ) { DpCTFlag = bDpCTFlag; Modified(); } }
	void SetDpCPNFlag( bool bDpCPNFlag ) { if( DpCPNFlag != bDpCPNFlag ) { DpCPNFlag = bDpCPNFlag; Modified(); } }
	void SetDpCPrice( double dPrice ) { if( DpCPrice != dPrice ) { DpCPrice = dPrice; Modified(); } }
	void SetDpCSetting( double dSetting ) { if( dSetting != DpCSetting ){ DpCSetting = dSetting; Modified(); } }
	void SetDpCFlow( double dFlow ) { if( dFlow != DpCFlow ) { DpCFlow = dFlow; Modified(); } }

	const IDPTR& GetDpCIDPtr();
	LPCTSTR GetDpCID();
	LPCTSTR GetDpCArtNum();
	LPCTSTR GetDpCName();
	LPCTSTR GetDpCVersionID();
	LPCTSTR GetDpCFamilyID();
	LPCTSTR GetDpCTypeID();
	LPCTSTR GetDpCSizeID();
	LPCTSTR GetDpCConnectID();
	LPCTSTR GetSchemeName();
	int GetSchemeIndex() { return SchemeIndex; }
	double GetHmin() { return Hmin; }
	double GetAuthority() { return Authority; }
	double GetDpmin() { return Dpmin; }
	bool GetDpminFlag() { return DpminFlag; }
	double GetDpL() { return DpL; }
	double GetDpBranch() { return DpBranch; }
	double GetKvCv() { return KvCv; }
	int GetDpStab() { return DpStab; }
	int GetMvLoc() { return MvLoc; }
	bool GetDpCTFlag() { return DpCTFlag; }
	bool GetDpCPNFlag() { return DpCPNFlag; }
	double GetDpCPrice() { return DpCPrice; }
	double GetDpCSetting() { return DpCSetting; }
	double GetDpCFlow() { return DpCFlow; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_DirectSel
//
///////////////////////////////////////////////////////////////////////////////////

struct DirectSel_struct {
	bool fTAProduct;							// true when TA products (TADB), false for user product(USERDB)
	TCHAR TabID[ _ID_LENGTH + 1 ];					// Store Table ID, it will be used to customize CDlgConfSel
	CString Comment;							// User's comment (only for user product)
};
class CDS_DirectSel : public CDS_SelThing , protected DirectSel_struct
{
	DEFINE_DATA(CDS_DirectSel)

// Attribute
public:
	
// Constructor and destructor.
protected:
	CDS_DirectSel( CDataBase *pDataBase, LPCTSTR ID );
	~CDS_DirectSel() {}

public:
	LPCTSTR GetTabID() { return TabID; }
	LPCTSTR	GetComment() { return Comment; }
	bool IsTAProduct() { return fTAProduct; }

	void SetTAProduct( bool fTAp ) { if( fTAp != fTAProduct ) { fTAProduct = fTAp; Modified(); } }
	void SetTabID( LPCTSTR ID ) { if( _tcscmp( TabID, ID ) ) { _tcsncpy_s( TabID, SIZEOFINTCHAR( TabID ), ID, SIZEOFINTCHAR( TabID ) - 1 ); Modified(); } }
	void SetComment( LPCTSTR ref ) { if( Comment != ref ) { Comment =ref; Modified(); } }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};	


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SelRadSet
//
///////////////////////////////////////////////////////////////////////////////////
struct RadSet_struct {

	bool PresetTrv;								// Trv is presetable
	RadiatorReturnValveMode RetVMode;						// Return Valve Mode Nothing, Other, Imi
	double DpTrv;								// DP of the Trv	
	double DpTot;								// Dp Total
	double SettingTrv;							// Setting of the Return Valve
	double KvRetV;								// Kv of the return Valve
	bool   TrvTFlag;
	bool   TrvPNFlag;	
	double PReq;								// Request Power
	double PInst;								// Installed Power
	double Ti;
	double Ts;
	double Tr;
	double NCTi;								// Nominal conditions for Ti,Ts,Tr
	double NCTs;
	double NCTr;
	
	double TrvPrice;
};

class CDS_SelRadSet : public CDS_SelHydroThing, protected RadSet_struct
{
	DEFINE_DATA(CDS_SelRadSet)

// Constructor and destructor.
protected:
	CDS_SelRadSet(CDataBase* pDataBase, LPCTSTR ID);
	~CDS_SelRadSet() {}

IDPTR m_TrvIDPtr;

public:
	const IDPTR &GetTrvIDPtr();
	const LPCTSTR GetTrvID() { return GetTrvIDPtr().ID; }
	LPCTSTR GetTrvArtNum();		
	LPCTSTR GetTrvName();		
	LPCTSTR GetTrvVersionID();	
	LPCTSTR GetTrvFamilyID();	
	LPCTSTR GetTrvTypeID();	
	LPCTSTR GetTrvSizeID();		
	LPCTSTR GetTrvConnectID();	

	bool GetTrvPreset() { return PresetTrv; }							
	bool GetTrvTFlag() { return TrvTFlag; }
	bool GetTrvPNFlag() { return TrvPNFlag; } 
	RadiatorReturnValveMode GetRetVMode()	{ return RetVMode; }				
	double GetTrvDp() { return DpTrv; }								
	double GetDpTot() { return DpTot; }								
	double GetTrvSetting() { return SettingTrv; }						
	double GetKvRetV() { return KvRetV; }								
	double GetTrvPrice() { return TrvPrice; }
	double GetPReq() { return PReq; }
	double GetPInst() { return PInst; }
	double GetTi() { return Ti; }
	double GetTs() { return Ts; }
	double GetTr() { return Tr; }
	double GetNCTi() { return NCTi; }
	double GetNCTs() { return NCTs; }
	double GetNCTr() { return NCTr; }

	void SetTrvID( LPCTSTR ID );	
	void SetTrvTFlag( bool bTRVFlag ) { if( bTRVFlag!=TrvTFlag ) { TrvTFlag = bTRVFlag; Modified(); } } 
	void SetTrvPNFlag( bool bTRVPNFlag ) { if( bTRVPNFlag != TrvPNFlag ) { TrvPNFlag=bTRVPNFlag; Modified(); } } 
	void SetTrvPreset( bool bPresetableTRV ) { if( bPresetableTRV != PresetTrv) { PresetTrv = bPresetableTRV; Modified(); } } 
	void SetRetVMode( RadiatorReturnValveMode eRadiatorReturnValveMode ) { if( eRadiatorReturnValveMode != RetVMode ) { RetVMode = eRadiatorReturnValveMode; Modified(); } } 
	void SetTrvDp( double dDpTRV ) { if( dDpTRV != DpTrv ) { DpTrv = dDpTRV; Modified(); } } 
	void SetDpTot( double dDpTotal ) { if( dDpTotal != DpTot ) { DpTot = dDpTotal; Modified(); } } 
	void SetTrvSetting( double dSetting ) { if( dSetting != SettingTrv ) { SettingTrv = dSetting; Modified(); } } 
	void SetKvRetV( double dKv ) { if( dKv != KvRetV ) { KvRetV = dKv; Modified(); } } 
	void SetTrvPrice( double dPrice ) { if( TrvPrice != dPrice ) { TrvPrice = dPrice; Modified(); } }
	void SetPReq( double val ) { if( PReq != val ) { PReq = val; Modified(); } }
	void SetPInst( double val ) { if( PInst != val ) { PInst = val; Modified(); } }
	void SetTi( double val ) { if( Ti != val ) { Ti =val; Modified(); } }
	void SetTs( double val ) { if( Ts != val ) { Ts =val; Modified(); } }
	void SetTr( double val ) { if( Tr != val ) { Tr =val; Modified(); } }
	void SetNCTi( double val ) { if( NCTi != val ) { NCTi = val; Modified(); } }
	void SetNCTs( double val ) { if( NCTs != val ) { NCTs = val; Modified(); } }
	void SetNCTr( double val ) { if( NCTr != val ) { NCTr = val; Modified(); } }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};		
	

///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_ProjectRef
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_ProjectRef : public CDB_MultiString
{

DEFINE_DATA(CDS_ProjectRef)

// Attribute
public:
enum enPrjRef {
	Name,
	Comment,
	CustRef,
	CustName,
	CustCompany,
	CustAddress,
	CustZip,
	CustTown,
	CustCountry,
	CustEmail,
	CustTel,
	CustFax
	};
	
protected:
	CDS_ProjectRef( CDataBase *pDataBase, LPCTSTR ID );
	~CDS_ProjectRef() {}

// Public methods.
public:
	void SetString( int iIndex, LPCTSTR str ) { if ( 0 != _tcscmp( CDB_MultiString::GetString( iIndex ), str ) ) { CDB_MultiString::SetString( iIndex, str ); Modified(); } }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_UserRef
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_UserRef : public CDB_MultiString
{
	DEFINE_DATA( CDS_UserRef )

// Public enum defines.
public:
	// HYS-1090: The 'Path' field is added after 'Other'
	enum enUserRef {
		Name,
		Company,
		Address,
		Zip,
		Town,
		Country,
		Tel,
		Fax,
		Email,
		Other,
		Path
		};
	
// Constructor and destructor.
protected:
	CDS_UserRef(CDataBase* pDataBase, LPCTSTR ID);
	~CDS_UserRef() {}

// Public methods.
public:
	void SetString( int iIndex, LPCTSTR str) { if( 0 != _tcscmp( CDB_MultiString::GetString( iIndex ), str ) ) { CDB_MultiString::SetString( iIndex, str ); Modified(); } }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_LoggedData
//
///////////////////////////////////////////////////////////////////////////////////
#define MAXLOGGEDCOLUMN	6
#define LOGGEDDATANAME	31
#define LD_SITE	20
#define LD_REF	10
#define LD_DATE	10
#define LD_TIME	8

struct LoggedData_Struct {
	TCHAR			Name[LOGGEDDATANAME+1];
	TCHAR			Site[LD_SITE+1];
	TCHAR			Ref[LD_REF+1];
	BYTE			Valve;
	TCHAR			Date[LD_DATE+1];
	TCHAR			Time[LD_TIME+1];
	BYTE	Interval;
	BYTE	Mode;
	CList<double,double&> DblList[MAXLOGGEDCOLUMN];
};
#endif

///////////////////////////////////////////////////////////////////////////////////
//
//								CLog
//
// Tool Base class for CDS_LogDataX and CDS_LoggedData no saved common variable
// could be defined here!
// This class use only vitual functions to interpret
// both class on the same way.
///////////////////////////////////////////////////////////////////////////////////

class CLog : public CData
{
	DEFINE_DATA(CLog)

// Public enum defines.
public:
	enum LogType	// Keep same order than in the enum_LogMode 
	{
		LOGTYPE_UNDEF = -1,
		LOGTYPE_DP,			// Dp
		LOGTYPE_FLOW,		// Q , Dp 
		LOGTYPE_TEMP,		// T1 or T2
		LOGTYPE_DPTEMP,		// Dp, T
		LOGTYPE_FLOWTEMP,	// Q, T, Dp
		LOGTYPE_DT,			// DT, T1, T2
		LOGTYPE_POWER,		// P , Q, Dp, DT, T1, T2
		LOGTYPE_LAST
	};
	
	enum ToolUsed // Which tool were used to do the log?
	{
		TOOL_TACBI,		// TA-CBI used
		TOOL_TASCOPE,	// TA-SCOPE used
		TOOL_LAST
	};
	
	enum eChannelType
	{
		ect_Dp,
		ect_Temp1,
		ect_Temp2,
		ect_FirstLogicalChan,				// Don't use it it's logical/physical channel border
		ect_Flow=ect_FirstLogicalChan,
		ect_DeltaT,
		ect_Power,
		ect_LastChannelType
	};

// Public methods.
public:
	CLog( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CLog(){};
	
#ifndef TACBX
	virtual void SetName(_string name){};
	virtual LPCTSTR GetName() { return _T(""); }
	virtual	LPCTSTR GetSite() { return _T(""); }
	virtual	LPCTSTR GetRef() { return _T(""); }
	virtual BYTE GetValveIndex() { return 0; }
	virtual unsigned short GetInterval() { return 0; }
	virtual IDPTR GetValveIDPtr() { return _NULL_IDPTR; }
	virtual CString GetValveName() { return _T(""); }

	// Return the logged Mode (0 pressure; 1 flow; 2 temperature; 3 P&T; 4 Q&T
	virtual	BYTE GetMode() { return 0; }
	virtual	bool GetValue(eChannelType chan, int row, double &dVal){return false; }
		// Give the value with a date time closed to dt
		// Return false if dt is less than StartDateTime
		// Give  the last value if dt is greater than the last DateTime
	virtual	bool GetValue(eChannelType chan, CTime &dt, double &dVal){return false; }
		// Give the value with a date time closed to StartDateTime + ts
	virtual	bool GetValue(eChannelType chan,CTimeSpan &ts, double &dVal){return false; }
	// Position iterator at the beginning of the list and give the value 
	// at this position in dVal
	virtual bool GetFirst(eChannelType chan, double &dVal, int index=0){return false; }
	// Position iterator at next element in the list and give the value 
	// at this position in dVal
	virtual bool GetNext(eChannelType chan, double &dVal){return false; }
	virtual	CTime	GetLogDateTime() { return CTime(); }
		// Return the number of logged values 
	virtual	int GetLength() { return 0; }
		// Return the date time for the (col,row) value
	virtual	bool GetDateTime(int row, CTime &dt){return false; }
	virtual	bool GetOleDateTime(int row, COleDateTime &dt){return false; }
	virtual	bool GetHighestValue(eChannelType chan, double &dVal){return false; }
	virtual	bool GetLowestValue(eChannelType chan, double &dVal){return false; }
	// Write logged data as a character delimited text file
	virtual bool WriteAsText(CString PathName, bool bPointIndex=true, bool bHeader=true, CString Delimiter=_T(",")){return false; }
	virtual ToolUsed ToolUsedForMeasure() { return ToolUsed::TOOL_LAST; }
	// Function used only for TA-SCOPE: Return true if T1 is used, false for T2
	// Used for Dp-Temp log, Flow-Temp log or Temp log only
	virtual bool IsT1SensorUsed() { return true; }
#endif
};


#ifndef TACBX
class CDS_LoggedData : public CLog, protected LoggedData_Struct
{
	DEFINE_DATA(CDS_LoggedData)

// Constructor and destructor.
protected:
	CDS_LoggedData(CDataBase* pDataBase, LPCTSTR ID);
	~CDS_LoggedData();

// Public methods.
public:
	void SetName( _string name ) { if( 0 != _tcsncmp( name.c_str(), Name, LOGGEDDATANAME ) ) { _tcsncpy_s( Name, SIZEOFINTCHAR( Name ), name.c_str(), SIZEOFINTCHAR( Name ) -1 ); Name[LOGGEDDATANAME] = _T('\0'); Modified(); } }
	void SetSite( LPCTSTR ptcSite ) {if( 0 != _tcsncmp( ptcSite, Site, LD_SITE ) ) { _tcsncpy_s( Site, SIZEOFINTCHAR( Site ), ptcSite, SIZEOFINTCHAR( Site ) -1 ); Site[LD_SITE] = _T('\0'); Modified(); } }
	void SetRef( LPCTSTR ptcReference ) { if( 0 != _tcsncmp( ptcReference, Ref, LD_REF ) ) { _tcsncpy_s( Ref, SIZEOFINTCHAR( Ref ), ptcReference, SIZEOFINTCHAR( ptcReference ) -1 ); Ref[LD_REF] = _T('\0');Modified(); } }
	void SetValveIndex( BYTE bValveIndex ) { if( bValveIndex != Valve ) { Valve = bValveIndex; Modified(); } } 
	void SetDate( LPCTSTR ptcDate ) { if( 0 != _tcsncmp( ptcDate, Date, LD_DATE ) ) { _tcsncpy_s( Date, SIZEOFINTCHAR( Date ), ptcDate, SIZEOFINTCHAR( Date ) -1 ); Date[LD_DATE] = _T('\0'); Modified(); } } 
	void SetTime( LPCTSTR ptcTime ) { if( 0 != _tcsncmp( ptcTime, Time, LD_TIME ) ) { _tcsncpy_s( Time, SIZEOFINTCHAR( Time ), ptcTime, SIZEOFINTCHAR( Time ) -1 ); Time[LD_TIME] = _T('\0'); Modified(); } } 
	void SetInterval( BYTE bInterval ) { if( bInterval != Interval ) { Interval = bInterval; Modified(); } } 
	void SetMode( BYTE mode ) { if( mode != Mode ) { Mode = mode; Modified(); } } 
	bool SetValue( int iColumn, int iRow,double dVal );

	bool AddValue( int iColumn, double dVal );

	virtual LPCTSTR GetName() { return Name; }
	virtual LPCTSTR GetSite() { return Site; }
	virtual LPCTSTR GetRef() { return Ref; }
	LPCTSTR GetDate() { return Date; }
	LPCTSTR GetTime() { return Time; }
	virtual BYTE GetValveIndex() { return Valve; }
	virtual unsigned short GetInterval() { return Interval; }
	
	// Return the logged Mode (0 pressure; 1 flow; 2 temperature; 3 DP&T; 4 Q&T
	virtual BYTE GetMode() { return Mode; }
	virtual bool GetValue( eChannelType chan, int iRow, double &dVal );
	
	// Give the value with a date time closed to dt
	// Return false if dt is less than StartDateTime
	// Give  the last value if dt is greater than the last DateTime
	virtual bool GetValue( eChannelType chan, CTime &dt, double &dVal );
	
	// Give the value with a date time closed to StartDateTime + ts.
	virtual bool GetValue( eChannelType chan, CTimeSpan &ts, double &dVal );
	
	// Position iterator at the beginning of the list and give the value at this position in dVal.
	bool GetFirst( eChannelType chan, double &dVal, int iIndex = 0 );
	
	// Position iterator at next element in the list and give the value at this position in dVal.
	bool GetNext( eChannelType chan, double &dVal );

	virtual CTime GetLogDateTime();
	
	// Return the number of logged values for this column.
	virtual int GetLength();

	// Return the date time for the (col,row) value.
	virtual bool GetDateTime( int iRow, CTime &dt );
	virtual bool GetOleDateTime( int iRow, COleDateTime &dt );

	virtual bool GetHighestValue( eChannelType chan, double &dVal );
	virtual bool GetLowestValue( eChannelType chan, double &dVal );

	// Write logged data as a character delimited text file
	virtual bool WriteAsText(  CString strPathName, bool bPointIndex, bool bHeader, CString strDelimiter = _T(",") );

	virtual ToolUsed ToolUsedForMeasure() { return CLog/*::ToolUsed*/::TOOL_TACBI; }

	// Check the coherence of logged data
	bool CheckLoggedData();

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

	int GetCBICol(eChannelType Chan);
	CLog::eChannelType GetCBIFirstCol();

// Private variables.
private:
	bool m_bStartDate;				// flag is set when startdate is initialised
	CTime	m_StartDate;
	POSITION m_Pos[MAXLOGGEDCOLUMN];	// Position in the list for GetFirst() and GetNext()
	bool InitStartDate();
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_ProjectParams
//
///////////////////////////////////////////////////////////////////////////////////
class CPrjParams
{
public:
	CPrjParams( CData *pOwner );
	virtual ~CPrjParams() {}

// Public enum defines.
public:
	typedef enum _PrjParamsID
	{
		BvBlw65TypeID,
		BvBlw65ConnectID,
		BvBlw65VersionID,
		BvBlw65PNID,
		BvAbv50TypeID,
		BvAbv50ConnectID,
		BvAbv50VersionID,
		BvAbv50PNID,

		DpCBlw65TypeID,
		DpCBlw65ConnectID,
		DpCBlw65PNID,
		DpCAbv50TypeID,
		DpCAbv50ConnectID,
		DpCAbv50PNID,

		PipeCircSerieID,
		PipeDistSupplySerieID,						// Version 7: 'PipeDistSerieID' becomes 'PipeDistSupplySerieID'.

		// Version 6 (17/06/2010) -> removing  of these variables.
		// BvBypBlw65TypeID,
		// BvBypBlw65ConnectID,
		// BvBypBlw65VersionID,
		// BvBypBlw65PNID,
		// BvBypAbv50TypeID,
		// BvBypAbv50ConnectID,
		// BvBypAbv50VersionID,
		// BvBypAbv50PNID,

		CvBlw65TypeID,
		CvBlw65ConnectID,
		CvBlw65VersionID,
		CvBlw65PNID,
		CvAbv50TypeID,
		CvAbv50ConnectID,
		CvAbv50VersionID,
		CvAbv50PNID,

		// Missing IDs added in Version 4
		BvBlw65FamilyID,
		BvBlw65BdyMatID,
		BvAbv50FamilyID,
		BvAbv50BdyMatID,

		DpCBlw65FamilyID,
		DpCBlw65BdyMatID,
		DpCBlw65VersionID,
		DpCAbv50FamilyID,
		DpCAbv50BdyMatID,
		DpCAbv50VersionID,

		BvBypBlw65FamilyID,
		BvBypBlw65BdyMatID,
		BvBypAbv50FamilyID,
		BvBypAbv50BdyMatID,
		
		CvBlw65FamilyID,
		CvBlw65BdyMatID,
		CvAbv50FamilyID,
		CvAbv50BdyMatID,

		PICvBlw65TypeID,
		PICvBlw65FamilyID,
		PICvBlw65BdyMatID,
		PICvBlw65ConnectID,
		PICvBlw65VersionID,
		PICvBlw65PNID,
		PICvAbv50TypeID,
		PICvAbv50FamilyID,
		PICvAbv50BdyMatID,
		PICvAbv50ConnectID,
		PICvAbv50VersionID,
		PICvAbv50PNID,

		PipeDistReturnSerieID,						// Version 7: new entry -> 'PipeDistReturnSerieID'.

		// Version 8.
		BCvBlw65TypeID,
		BCvBlw65FamilyID,
		BCvBlw65BdyMatID,
		BCvBlw65ConnectID,
		BCvBlw65VersionID,
		BCvBlw65PNID,
		BCvAbv50TypeID,
		BCvAbv50FamilyID,
		BCvAbv50BdyMatID,
		BCvAbv50ConnectID,
		BCvAbv50VersionID,
		BCvAbv50PNID,

		ActuatorPowerSupplyID,
		ActuatorInputSignalID,

		// Version 10 (2014/03/20)
		SeparatorTypeID,
		SeparatorFamilyID,
		SeparatorConnectID,
		SeparatorVersionID,

		// Version 11 (2016/07/11)
		DpCBCValveBlw65TypeID,
		DpCBCValveBlw65FamilyID,
		DpCBCValveBlw65BdyMatID,
		DpCBCValveBlw65ConnectID,
		DpCBCValveBlw65VersionID,
		DpCBCValveBlw65PNID,
		DpCBCValveAbv50TypeID,
		DpCBCValveAbv50FamilyID,
		DpCBCValveAbv50BdyMatID,
		DpCBCValveAbv50ConnectID,
		DpCBCValveAbv50VersionID,
		DpCBCValveAbv50PNID,

		// Version 14 (2020/11/02)
		SmartControlValveBlw65TypeID,
		SmartControlValveBlw65FamilyID,
		SmartControlValveBlw65BdyMatID,
		SmartControlValveBlw65ConnectID,
		SmartControlValveBlw65VersionID,
		SmartControlValveBlw65PNID,
		SmartControlValveAbv50TypeID,
		SmartControlValveAbv50FamilyID,
		SmartControlValveAbv50BdyMatID,
		SmartControlValveAbv50ConnectID,
		SmartControlValveAbv50VersionID,
		SmartControlValveAbv50PNID,

		// Version 15 (2022/07/11)
		SmartDpCBlw65TypeID,
		SmartDpCBlw65FamilyID,
		SmartDpCBlw65BdyMatID,
		SmartDpCBlw65ConnectID,
		SmartDpCBlw65VersionID,
		SmartDpCBlw65PNID,
		SmartDpCAbv50TypeID,
		SmartDpCAbv50FamilyID,
		SmartDpCAbv50BdyMatID,
		SmartDpCAbv50ConnectID,
		SmartDpCAbv50VersionID,
		SmartDpCAbv50PNID,
		PipeByPassSerieID,						// Version 15: new entry -> 'PipeByPassSerieID'.
		
		LASTID
	};

	typedef enum _CVMode
	{
		KvMode,
		TAMode
	};

// Public methods.
public:
	// If 'bForceUpdate' is set to 'true' we will update the param even if the ID is the same.
	// It is mainly used by the 'CDlgCustPipe::_SaveDefaultPipeSeries' method.
	void SetPrjParamID( _PrjParamsID ID, CString str, bool bForceUpdate = false );
	CString GetPrjParamID( _PrjParamsID ID );
	IDPTR GetPrjParamIDPtr( _PrjParamsID ID );
	
	// Verify if ID below and above are different, return true if different.
	bool CompareBvID();
	bool CompareDpCID();

	void ResetPrjParams( bool bToDefault = false, CString strVersion = _T("") );

	void SetCVMode( _CVMode CVMode ) { if( m_CVMode != CVMode ) { m_CVMode = CVMode; if( NULL != m_pOwner ) m_pOwner->Modified(); } }
	_CVMode GetCVMode() { return m_CVMode; }
	
	void SetCVCtrlType( CDB_ControlProperties::CvCtrlType CVCtrl ) { if( m_CVCtrlType != CVCtrl ) { m_CVCtrlType = CVCtrl; if( NULL != m_pOwner ) m_pOwner->Modified(); } }
	CDB_ControlProperties::CvCtrlType GetCVCtrlType() { return m_CVCtrlType; }

	void SetCVType( CDB_ControlProperties::eCVFUNC CVFunc ){ if( m_CVFunc != CVFunc ) { m_CVFunc = CVFunc; if( NULL != m_pOwner ) m_pOwner->Modified(); } }
	CDB_ControlProperties::eCVFUNC GetCVType() { return m_CVFunc; }

	void SetActuatorAutomaticallySelect( bool fActuatorAutomaticallySelect ) { if( m_bActuatorAutomaticallySelect != fActuatorAutomaticallySelect ) { m_bActuatorAutomaticallySelect = fActuatorAutomaticallySelect; if( NULL != m_pOwner ) m_pOwner->Modified(); } }
	bool GetActuatorAutomaticallySelect( void ) { return m_bActuatorAutomaticallySelect; }

	void SetActuatorSelectedByPackage( bool fActuatorSelectedByPackage ) { if( m_bActuatorSelectedByPackage != fActuatorSelectedByPackage ) { m_bActuatorSelectedByPackage = fActuatorSelectedByPackage; if( NULL != m_pOwner ) m_pOwner->Modified(); } }
	bool GetActuatorSelectedByPackage( void ) { return m_bActuatorSelectedByPackage; }

	void SetActuatorFailSafeFct( int iActuatorFailSafe ) { if( m_iActuatorFailSafeFct != iActuatorFailSafe ) { m_iActuatorFailSafeFct = iActuatorFailSafe; if( NULL != m_pOwner ) m_pOwner->Modified(); } }
	void SetActuatorDRPFct( CDB_ControlValve::DRPFunction eActuatorDRP ) { if( m_eActuatorDRPFct != eActuatorDRP ) { m_eActuatorDRPFct = eActuatorDRP; if( NULL != m_pOwner ) m_pOwner->Modified(); } }
	int GetActuatorFailSafeFct( void ) { return m_iActuatorFailSafeFct; }
	CDB_ControlValve::DRPFunction GetActuatorDRPFct( void ) { return m_eActuatorDRPFct; }

	void SetUserForcedHMCalcModeFlag( bool fUserForcedHMCalcMode ) { m_fUserForcedHMCalcMode = fUserForcedHMCalcMode; }
	bool GetUserForcedHMCalcModeFlag() { return m_fUserForcedHMCalcMode; }
	bool IsOldProject( void ) { return ( m_Version < 9 ) ? true : false; }
	// HYS-1221 : Accessor for m_bOldProductsAvailable member variable
	void SetCheckOldProductsAvailableForHC( bool bCheck ) { m_bOldProductsAvailable = bCheck; }
	bool IsOldProductsAvailableForHC( void ) { return m_bOldProductsAvailable; }

	bool IfInitialized() { return m_bInitialized; }

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );
	void Copy( CPrjParams *pPrjParams );
	
	bool operator==( CPrjParams &PrjParams );

// Private methods.
private:
	void _VerifyIfPipeSeriesExist( CString &strIDToVerify );

// Private variables.
private:
	CData *m_pOwner;
	bool m_bInitialized;
	struct sID
	{
		IDPTR IDptr;
		unsigned int index;
	};
	CArray<sID> m_ParamID;
	_CVMode m_CVMode;
	CDB_ControlProperties::CvCtrlType m_CVCtrlType;
	CDB_ControlProperties::eCVFUNC m_CVFunc;
	bool m_bActuatorAutomaticallySelect;
	bool m_bActuatorSelectedByPackage;
	int m_iActuatorFailSafeFct;
	CDB_ControlValve::DRPFunction m_eActuatorDRPFct;
	
	// This variable is set to 'true' when user intentionally converts his project from CBI mode to hydronic calculation mode.
	// Remark: from version 9.
	BYTE m_Version;
	bool m_fUserForcedHMCalcMode;
	bool m_bOldProductsAvailable;
};


class CDS_ProjectParams : public CData
{
	DEFINE_DATA( CDS_ProjectParams )

protected:
	CDS_ProjectParams( CDataBase* pDataBase, LPCTSTR ID );
	~CDS_ProjectParams();

public:
	unsigned short GetLastUid() { return uid; }
	unsigned short GetNextUid() { return uid++; }
	void ResetUid( int iUid = 1 ) { uid = iUid; }
	void SetLastUid( unsigned short Uid ) { uid = Uid + 1; }
	
	// Scan all HydroMod and complete Missing UID.
	void CompleteUID( CTable *pTab );
	
	void SetSafetyFactor( double sf ){ if( sf != m_SafetyFactor ) { m_SafetyFactor = sf; Modified(); } }
	double GetSafetyFactor() { return m_SafetyFactor; }
	CPrjParams *GetpHmCalcParams() { return m_pPrjParams; }
	void SetFreeze( bool bset ) { m_bFreeze = bset; }
	bool IsFreezed() { return m_bFreeze; }
	void SetCompatibleCBI( bool bset ) { m_bCompatibleCBI=bset; }
	bool IsCompatibleCBI() { return m_bCompatibleCBI; }
	bool GetHNAutomaticCheck() const { return m_bHNAutomaticCheck; }
	void SetHNAutomaticCheck( bool val ) { m_bHNAutomaticCheck = val; }
	void SetUserForcedHMCalcModeFlag( bool fUserForcedHMCalcMode ) { m_pPrjParams->SetUserForcedHMCalcModeFlag( fUserForcedHMCalcMode ); }
	bool GetUserForcedHMCalcModeFlag() { return m_pPrjParams->GetUserForcedHMCalcModeFlag(); }
	bool IsOldProject( void ) { return m_pPrjParams->IsOldProject(); }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	unsigned short uid;
	CPrjParams *m_pPrjParams; 
	double m_SafetyFactor;
	bool m_bFreeze;
	bool m_bCompatibleCBI;
	bool m_bHNAutomaticCheck;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CSelectedInfos
//
///////////////////////////////////////////////////////////////////////////////////

class CSelectedInfos
{
public:
	CSelectedInfos( CData *pOwner );
	virtual ~CSelectedInfos() {}

public:
	enum eReferences
	{
		eRef1,
		eRef2
	};
	void SetReference( eReferences eRef, CString str );
	void SetQuantity( int iQuantity );
	void SetPrice( double dPrice );
	void SetRemark( CString strRemark );
	void SetRemarkIndex( int iRemarkIndex );
	void SetRowIndex( int iRowIndex );
	void SetpWCData( CWaterChar *pWC ) { if( NULL != pWC ) { m_WC = *pWC; } }
	void SetModifiedObjIDPtr( IDPTR idptr ) { m_ModIDPtr = idptr; }
	void ClearModifiedObjIDptr() { m_ModIDPtr=_NULL_IDPTR; }
	void SetDT( double dDT ) { m_dDT = dDT; }
	void SetCoolingDT( double dDT ) { m_dCoolingDT = dDT; }
	void SetHeatingDT( double dDT ) { m_dHeatingDT = dDT; }
	void SetApplicationType( ProjectType eApplicationType ) { m_eApplicationType = eApplicationType; }

	CString GetReference( eReferences eWhichReference ) { if( eRef1 == eWhichReference ) return m_strReference1; else return m_strReference2; }
	int GetQuantity() { return m_iQuantity; }
	double GetPrice() { return m_dPrice; }
	CString GetRemark() { return m_strRemark; }
	int GetRemarkIndex() { return m_iRemarkIndex; }
	int GetRowIndex() { return m_iRowIndex; }
	CWaterChar *GetpWCData() { return &m_WC; }
	IDPTR GetModifiedObjIDPtr()	{ return m_ModIDPtr; }
	double GetDT() { return m_dDT; }
	double GetCoolingDT() { return m_dCoolingDT; }
	double GetHeatingDT() { return m_dHeatingDT; }
	ProjectType GetApplicationType() { return m_eApplicationType; }

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );
	void Copy( CSelectedInfos *pSelectedInfos );

protected:
	CData *m_pOwner;
	CString m_strReference1;			// User reference 1.
	CString m_strReference2;			// User reference 2.
	int m_iQuantity;					// Number of same items.
	double m_dPrice;
	CString m_strRemark;				// User remark .
	int m_iRemarkIndex;					// Remark Index used for visualisation; computed each time the table is displayed.
	int m_iRowIndex;					// Position in table used for visualization.
	CWaterChar m_WC;
	double m_dDT;
	ProjectType m_eApplicationType;
	// For ChangeOver only
	double m_dCoolingDT;
	double m_dHeatingDT;

	// Variable not saved.
	IDPTR	m_ModIDPtr;					// IDPtr of modified object
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SelProd
//
///////////////////////////////////////////////////////////////////////////////////
class CSelectPipe;
class CDS_SelProd : public CData
{
	DEFINE_DATA( CDS_SelProd )

public:
	enum eFlowDef
	{
		efdFlow,					// defined as a flow
		efdPower,					// defined as power
		efdLast
	};

// Constructor and destructor.
protected:
	CDS_SelProd( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SelProd();

// Public methods.
public:
	// Allow to return what is the kind of inherited class.
	ProductSubCategory GetProductSubCategory( void ) { return m_eProductSubCategory; }

	virtual IDPTR GetProductIDPtr() { return _NULL_IDPTR; }

	eFlowDef GetFlowDef() { return m_FlowDef; }
	double GetQ() { return m_dQ; }
	double GetPower() { return m_dPower; }
	double GetDT() { return m_dDT; }
	bool IsFromDirSel() { return m_bDirSel; }
	virtual bool IsSelectedAsAPackage( bool bVerifyPackage = false );

	CSelectPipe *GetpSelPipe() { return m_pSelPipe; }
	LPCTSTR GetHMID() { return m_HMIDptr.ID; }
	IDPTR GetHMIDptr();
	
	CString GetTypeID() { return m_strTypeID; }
	CString GetFamilyID() { return m_strFamilyID; }
	CString GetMaterialID() { return m_strMaterialID; }
	CString GetConnectID() { return m_strConnectID; }
	CString GetVersionID() { return m_strVersionID; }
	CString GetPNID() { return m_strPNID; }
	CString GetPipeSeriesID() { return m_strPipeSeriesID; }
	CString GetPipeID() { return m_strPipeID; }

	void SetFlowDef( eFlowDef FlowDef ) { if( FlowDef != m_FlowDef ) { m_FlowDef = FlowDef; Modified(); } }
	void SetPower( double dPower ) { SetMbreDouble( dPower, m_dPower ); }
	void SetDT( double dDT ) { SetMbreDouble(dDT,m_dDT); }
	void SetQ( double dQ ) { SetMbreDouble( dQ, m_dQ ); }
	void SetFromDirSel( bool bFlag ) { if( bFlag != m_bDirSel ) { m_bDirSel = bFlag; Modified(); } }
	void SetSelectedAsAPackage(bool val) { m_bSelectedAsAPackage = val; }
	void SetHMIDptr( const IDPTR &idptr );
	void SetTypeID( CString strTypeID );
	void SetFamilyID( CString strFamilyID );
	void SetMaterialID( CString strMaterialID );
	void SetConnectID( CString strConnectID );
	void SetVersionID( CString strVersionID );
	void SetPNID( CString strPNID );
	void SetPipeSeriesID( CString strPipeSeriesID );
	void SetPipeID( CString strPipeID );
	void SetProductSubCategory( ProductSubCategory eProductSubCategory );
	
	virtual CSelectedInfos *GetpSelectedInfos() { return m_pSelectedInfos; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	IDPTR GetMbrIDPtr( IDPTR &IDPtr, CTADatabase *pDB = NULL );
	void SetMbreIDPtr( IDPTR &TrgIDptr, const IDPTR &IDPtr, CTADatabase *pDB = NULL );
	void SetMbreDouble( double dFrom, double &dTo ) { if( dFrom != dTo ) { dTo = dFrom; Modified(); } }
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Private variables.
protected:
	double m_dQ;
	eFlowDef m_FlowDef;
	double m_dPower;
	double m_dDT;
	bool m_bSelectedAsAPackage;
	bool m_bDirSel;						// From direct selection
	
	CSelectedInfos *m_pSelectedInfos;	// Selection informations
	CSelectPipe *m_pSelPipe;			// Selected Pipe
	IDPTR m_HMIDptr;					// No more used but need it when calling 'CTADatastruct::CleanOtherUnNeededSelectedProduct()'.

	CString m_strTypeID;
	CString m_strFamilyID;
	CString m_strMaterialID;
	CString m_strConnectID;
	CString m_strVersionID;
	CString m_strPNID;
	CString m_strPipeSeriesID;
	CString m_strPipeID;

	ProductSubCategory m_eProductSubCategory;

	// Remark: 'SPT' is for 'Selection Product Type'.
	// 2017-06-01: Keep this definition only to convert this old enum definition to the new
	//             one 'ProductSubCategory' defined in the 'DataBObj.h' file.
	//             DO NOT USED IT ANYMORE.
	enum Obsolete_SelProdType
	{
		Obsolete_eSPT_BCv,
		Obsolete_eSPT_Bv,
		Obsolete_eSPT_Cv,
		Obsolete_eSPT_DpC,
		Obsolete_eSPT_PICv,
		Obsolete_eSPT_Trv,
		Obsolete_eSPT_AirVent,
		Obsolete_eSPT_Separator,
		Obsolete_eSPT_DpCBCV,
		Obsolete_eSPT_Sv,
		Obsolete_eSPT_None
	};
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CAccessoryList
//
///////////////////////////////////////////////////////////////////////////////////
class CAccessoryList
{
public:
	enum AccessoryType
	{
		_AT_Undefined = -1,
		_AT_Accessory,
		_AT_Adapter,
		_AT_SetAccessory
	};
	
	typedef struct _AccessoryItem
	{
		IDPTR IDPtr;
		AccessoryType eAccessoryType;
		bool fByPair;
		bool fDistributed;
		// HYS-987: lEditedQty working variable to save the accessory quantity edited in confirm sel
		long lEditedQty;
		_AccessoryItem() : IDPtr(_NULL_IDPTR)
		{
			eAccessoryType = _AT_Undefined;
			fByPair = false;
			fDistributed = false;
			lEditedQty = -1;
		}
	}AccessoryItem;
	
	typedef std::multimap<AccessoryType, AccessoryItem>	mmapTypeAccessory;
	typedef mmapTypeAccessory::iterator mmapTypeAccessoryIter;

	CAccessoryList() {};
	~CAccessoryList() {};

	// Allow to remove all object from list.
	// Param: 'eAccessoryType' set to '_AT_Undefined' to clear all list or set to another value to clear only specific object.
	void Clear( AccessoryType eAccessoryType = _AT_Undefined );

	// Allow to add an object in the list.
	// Param: 'idptr' is the accessory to add.
	// Param: 'eAccessoryType' set the accessory type.
	// Param: 'pclRuledTable' in which belongs the accessory or NULL.
	// Param: 'bCheckType' set to 'true' if we must check what is the accessory type!
	// Remark: Typically, 'bCheckType' is set to 'true' with old CDS_SSel version that has no 'CAccessoryList' member variable (thus
	//         no identification of the type). Hopefully, before adding 'CAccessory' there was only accessory or adapter. And 
	//         there was only adapter type 'ADAPTYPE_CV'.
	void Add( const IDPTR &idptr, AccessoryType eAccessoryType, CDB_RuledTableBase *pclRuledTable = NULL, bool bCheckType = false );

	// REMARK: INTERNAL USE ONLY.
	void Add( AccessoryType eAccessoryType, AccessoryItem *prAccessoryItem );
		
	// Allow to return how many objects there is in the accessory list.
	// Param: 'eAccessoryType' returns the number of specific object or set to '_AT_Undefined' to returns the number of all objects.
	int GetCount( AccessoryType eAccessoryType = _AT_Undefined );

	// Allow to retrieve first object from the accessory list.
	// Param: 'eAccessoryType' set to '_AT_Undefined' if you want to loop all objects in the list. Or set to another value to loop
	//        only specific object.
	AccessoryItem GetFirst( AccessoryType eAccessoryType = _AT_Undefined );
		
	// Allow to retrieve next object from the accessory list.
	// Param: 'eAccessoryType' set to '_AT_Undefined' if you want to loop all objects in the list. Or set to another value to loop
	//        only specific object.
	AccessoryItem GetNext( AccessoryType eAccessoryType = _AT_Undefined );

	// Allow to retrieve accessory from the accessory list.
	// Param: iIndex of the object to retrieve.
	AccessoryItem GetItem( int iIndex );

	// Allow to return the accessory type of the accessory passed as argument.
	// Returns '_AT_Undefined' if accessory doesn't exist.
	AccessoryType GetAccessoryType( IDPTR IDPtr );

	// Allow to copy accessory list in an another.
	void CopyTo( CAccessoryList *pclAccessoryList );

	// Allow to properly write all accessories.
	void Write( OUTSTREAM outf );

	// Allow to properly read all accessories.
	bool Read( INPSTREAM inpf, bool bWithVersion );

	// Set lEditedQty for a given AccessoryItem AccItem
	void SetEditedQty( AccessoryItem AccItem, long lQty );

	// remove a given AccessoryItem from m_mmapAccessories and return true if the item is succesfully removed
	bool Remove( AccessoryItem AccessoryItem );

	// Find AccessoryItem.IDPTR in m_mmapAccessories
	bool FindItem( IDPTR IdptrTofind );

	// return m_mmapIter->second
	AccessoryItem GetGlobalPostion();
private:
	mmapTypeAccessory m_mmapAccessories;
	mmapTypeAccessoryIter m_mmapIter;			// Used by 'GetFirst' and 'GetNext'.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSel
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSel : public CDS_SelProd
{
public:
	DEFINE_DATA(CDS_SSel)

protected:
	CDS_SSel( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSel() {}

public:
	double GetDp() const { return m_dDpKv; }
	void SetDp( double dValue ) { if( m_dDpKv != dValue ) { m_dDpKv = dValue; Modified(); } }

	virtual double GetOpening() const	{ return m_dOpening; }
	virtual void SetOpening( double dValue ) { if( m_dOpening != dValue) { m_dOpening = dValue; Modified(); } }

	// Remark: 'Valve' has been changed to 'Product' to allow to add inherited classes that are not valve (like 'CDS_SSelAirVentSeparator').
	void SetProductIDPtr( const IDPTR &idptr ) { SetMbreIDPtr( m_ProductIDPtr, idptr); }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	virtual IDPTR GetProductIDPtr() { return GetMbrIDPtr( m_ProductIDPtr); }
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	template<typename T> T *GetProductAs() { return dynamic_cast<T*>( GetProductIDPtr().MP ); }

	bool IsAccessoryExist( void ) { return ( 0 == m_clAccessoryList.GetCount() ) ? false : true; }
	CAccessoryList *GetAccessoryList( void ) { return &m_clAccessoryList; }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	virtual void Copy( CData *pclDestination );

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData *pclSelectionCompareWith, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////
	
// Protected methods.
protected:

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dDpKv;				// Used to store Dp (or Kv in case of DpC on CV).
	double m_dOpening;			// Valve opening.
	IDPTR m_ProductIDPtr;
	CAccessoryList m_clAccessoryList;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelBv
//
///////////////////////////////////////////////////////////////////////////////////

class CDS_SSelBv : public CDS_SSel
{
	DEFINE_DATA(CDS_SSelBv)

protected:
	CDS_SSelBv( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelBv() {}

public:
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelSv
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelSv : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelSv )

protected:
	CDS_SSelSv( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelSv() {}

public:
	CAccessoryList *GetSvAccessoryList() { return &m_clAccessoryList; }

	IDPTR GetActrIDPtr() { return GetMbrIDPtr( m_ActrIDPtr); }
	CDB_Actuator *GetpActuator() { return dynamic_cast<CDB_Actuator*>(GetActrIDPtr().MP); }

	bool IsActuatorAccessoryExist( void ) { return ( 0 == m_clActuatorAccessoryList.GetCount() ) ? false : true; }
	CAccessoryList *GetActuatorAccessoryList() { return &m_clActuatorAccessoryList; }

	void SetActrIDPtr( const IDPTR &idptr ) { SetMbreIDPtr( m_ActrIDPtr, idptr); }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	IDPTR m_ActrIDPtr;
	CAccessoryList m_clActuatorAccessoryList;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelDpC
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelDpC : public CDS_SSel
{
	DEFINE_DATA(CDS_SSelDpC)

protected:
	CDS_SSelDpC( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelDpC() {}

public:
	double GetKv() const { return m_dKv; }					//Stored into m_DpKv in base class, never used together
	double GetDpL() const { return m_dDpL; }
	double GetDpMax() const { return m_dDpMax; }
	eDpCLoc GetDpCLoc() const { return m_eDpCLoc; }			// Branch or Kvs.
	eDpStab GetDpStab() const { return m_eDpStab; }			// DpCLocUpStream or DpCLocDownStream.
	eMvLoc GetMvLoc() const { return m_eMvLoc; }			// primary/secondary.
	double GetDpCSetting();

	bool IsDpCAccessoryExist( void ) { return ( 0 == m_clDpCAccessoryList.GetCount() ) ? false : true; }
	CAccessoryList *GetDpCAccessoryList( void ) { return &m_clDpCAccessoryList; }
	
	IDPTR GetDpCIDPtr() { return GetMbrIDPtr( m_DpCIDPtr); }
	IDPTR GetDpCMvPackageIDPtr();
	void ResetDpCMvPackageIDPtr() { m_DpCMvPackageIDPtr = _NULL_IDPTR; }
	CString GetSchemeName();
	unsigned int GetSchemeIndex() const { return m_uiSchemeIndex; }
	double GetDpMin();
	double GetHMin();
	double GetAuthority();
	double GetDpToStab();	// Based on flow, CV Kvs if exist return Dp to stabilize


	virtual bool IsSelectedAsAPackage( bool fVerifyPackage = false );

	void SetKv( double val ) { if( m_dKv != val) { m_dKv = val; Modified(); } }
	void SetDpL( double val ) { if( m_dDpL != val) { m_dDpL = val; Modified(); } }
	void SetDpMax( double val ) { if( m_dDpMax != val) { m_dDpMax = val; Modified(); } }
	void SetDpStab( eDpStab val ) { if( m_eDpStab != val) { m_eDpStab = val; Modified(); } }
	void SetDpCLoc( eDpCLoc val ) { if( m_eDpCLoc != val) { m_eDpCLoc = val; Modified(); } }
	void SetMvLoc( eMvLoc val ) { if( m_eMvLoc != val) { m_eMvLoc = val; Modified(); } }
	void SetDpCIDPtr( const IDPTR &idptr ) { SetMbreIDPtr( m_DpCIDPtr, idptr); }
	void SetSchemeIndex( unsigned int val ) { m_uiSchemeIndex = val; }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	virtual void Copy( CData *pclDestination );

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData* pclSelectionToCompare, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////
	
// Protected members.
protected:

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	eDpStab m_eDpStab;
	eDpCLoc m_eDpCLoc;
	eMvLoc m_eMvLoc;
	CAccessoryList m_clDpCAccessoryList;
	IDPTR m_DpCIDPtr;
	IDPTR m_DpCMvPackageIDPtr;
	double m_dDpL;				//Dp Load
	double m_dKv;				//Cv Kv when stabilization on Cv
	double m_dDpMax;
	
	// Working variable.
	unsigned int m_uiSchemeIndex;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelDpReliefValve
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelDpReliefValve : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelDpReliefValve )

protected:
	CDS_SSelDpReliefValve(CDataBase *pDataBase, LPCTSTR ID);
	virtual ~CDS_SSelDpReliefValve() {}

public:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////
	
// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelRadSet
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelRadSet : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelRadSet )

protected:
	CDS_SSelRadSet( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelRadSet() {}

public:
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Data filled by 'CDlgIndSelTrv::OnBnClickedButtonselect'.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// These 4 next methods are defined just to clarify difference between supply and return valve (call corresponding base class methods).
	CString GetSupplyValveFamilyID() { return GetFamilyID(); }
	CString GetSupplyValveTypeID() { return GetTypeID(); }
	CString GetSupplyValveVersionID() { return GetVersionID(); }
	CString GetSupplyValveConnectID() { return GetConnectID(); }

	CString GetReturnValveFamilyID() { return m_strRVFamilyID; }
	CString GetReturnValveVersionID() { return m_strRVVersionID; }
	CString GetReturnValveConnectID() { return m_strRVConnectID; }

	// These 4 next methods are defined just to clarify difference between supply and return valve (call corresponding base class methods).
	void SetSupplyValveFamilyID( CString strSupplyValveFamilyID );
	void SetSupplyValveTypeID( CString strSupplyValveTypeID );
	void SetSupplyValveVersionID( CString strSupplyValveVersionID );
	void SetSupplyValveConnectID( CString strSupplyValveConnectID );

	void SetReturnValveFamilyID( CString strReturnValveFamilyID );
	void SetReturnValveVersionID( CString strReturnValveVersionID );
	void SetReturnValveConnectID( CString strReturnValveConnectID );

	// Call to copy variable from DialogSizeRad.
	void GetRadInfos( RadInfo_struct *prRadInfos );
	void SetRadInfos( RadInfo_struct *prRadInfos );

	RadiatorReturnValveMode GetReturnValveMode( void ) { return m_eReturnValveMode; }
	void SetReturnValveMode( RadiatorReturnValveMode eReturnValveMode ) { m_eReturnValveMode = eReturnValveMode; }

	int GetActuatorType( void ) { return m_iActuatorType; }
	void SetActuatorType( int iActuatorType ) { m_iActuatorType = iActuatorType; }
		
	int GetRadiatorValveType(void ) { return m_iRadiatorValveType; }
	void SetRadiatorValveType( int iRadiatorValveType ) { m_iRadiatorValveType = iRadiatorValveType; }

	// Supply valve insert name is 'Heimeier insert' or 'Other insert'.
	LPCTSTR GetSVInsertName( void ) { return m_strSVInsertName; }
	void SetSVInsertName( CString strName ) { _tcsncpy_s( m_strSVInsertName, SIZEOFINTCHAR( m_strSVInsertName ), strName, SIZEOFINTCHAR( m_strSVInsertName ) - 1 ); }

	// Returns 'true' if user is working with Kv for insert.
	bool IsInsertInKv( void ) { return m_fIsInsertInKv; }
	void SetInsertInKv( bool fInsertInKv ) { m_fIsInsertInKv = fInsertInKv; }
	
	// Contains Kv if user is working with Kv for insert.
	double GetInsertKvValue( void ) { return m_dInsertKvValue; }
	void SetInsertKvValue( double dInsertKvValue ) { m_dInsertKvValue = dInsertKvValue; }

	// Contains information about thermostatic head.
	void SetActuatorTypeID( CString strActuatorTypeID );
	void SetActuatorFamilyID( CString strActuatorFamilyID );
	void SetActuatorVersionID( CString strActuatorVersionID );
	
	CString GetActuatorTypeID() { return m_strActuatorTypeID; }
	CString GetActuatorFamilyID() { return m_strActuatorFamilyID; }
	CString GetActuatorVersionID() { return m_strActuatorVersionID; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Data filled by 'CRViewSSelTrv::FillInSelected'.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// SUPPLY VALVE.

	// These 2 next methods are defined just to clarify difference between supply and return valve (call corresponding base class methods).
	IDPTR GetSupplyValveIDPtr() { return GetProductIDPtr(); }
	
	// Remark: There is NO 'SetSupplyValveAccessoryList'. When you want to add accessory, call this methods to retrieve
	//         internal accessory container class and use its methods!
	CAccessoryList *GetSupplyValveAccessoryList( void ) { return &m_clAccessoryList; }
	
	bool IsSupplyValveAccessoryExist( void ) { return ( m_clAccessoryList.GetCount() > 0 ) ? true : false; }
	
	// There is no actuator is base class 'CDS_SSel'.
	IDPTR GetSupplyValveActuatorIDPtr() { return GetMbrIDPtr( m_strSupplyValveActuatorID ); }
	bool IsSupplyValveActuatorExist( void ) { return ( _NULL_IDPTR == m_strSupplyValveActuatorID ) ? false : true; } 

	// Return list of accessories on the actuator.
	// Remark: There is NO 'SetSVActuatorAccessoryList'. When you want to add accessory, call this methods to retrieve
	//         internal accessory container class and use its methods!
	CAccessoryList *GetSVActuatorAccessoryList( void ) { return &m_clSVActuatorAccessoryList; }
	bool IsSVActuatorAccessoryExist( void ) { return ( m_clSVActuatorAccessoryList.GetCount() > 0 ) ? true : false; }
	
	// Pay attention: 'm_dDpKv' in the class bass is the total Dp available (entered by user).
	double GetSupplyValveDp( void ) { return m_dSupplyValveDp; }

	// This method is defined just to clarify difference between supply and return valve (call corresponding base class methods).
	double GetSupplyValveOpening( void ) { return GetOpening(); }

	// This method is defined just to clarify difference between supply and return valve (call corresponding base class methods).
	void SetSupplyValveIDPtr( const IDPTR &idptr ) { SetProductIDPtr( idptr ); }

	// There is no actuator is base class 'CDS_SSel'.
	void SetSupplyValveActuatorIDPtr( const IDPTR &idptr ) { SetMbreIDPtr( m_strSupplyValveActuatorID, idptr ); }

	// Pay attention: 'm_dDpKv' in the class bass is the total Dp available (entered by user).
	void SetSupplyValveDp( double dDp ) { m_dSupplyValveDp = dDp; }

	// This method is defined just to clarify difference between supply and return valve (call corresponding base class methods).
	void SetSupplyValveOpening( double dOpening ) { SetOpening( dOpening ); }

	// RETURN VALVE.

	// Return valve and accessories methods.
	IDPTR GetReturnValveIDPtr() { return GetMbrIDPtr( m_strReturnValveID ); }
	bool IsReturnValveExist( void ) { return ( _NULL_IDPTR == m_strReturnValveID ) ? false : true; }

	// Remark: There is NO 'SetReturnValveAccessoryList'. When you want to add accessory, call this methods to retrieve
	//         internal accessory container class and use its methods!
	CAccessoryList *GetReturnValveAccessoryList( void ) { return &m_clReturnValveAccessoryList; }
	bool IsReturnValveAccessoryExist( void ) { return ( m_clReturnValveAccessoryList.GetCount() > 0 ) ? true : false; }
	
	double GetReturnValveOpening( void ) { return m_dReturnValveOpening; }
	double GetReturnValveDp( void ) { return m_dReturnValveDp; }

	void SetReturnValveIDPtr( const IDPTR &idptr ) { SetMbreIDPtr( m_strReturnValveID, idptr ); }
	void SetReturnValveOpening( double dOpening ) { m_dReturnValveOpening = dOpening; }
	void SetReturnValveDp( double dDp ) { m_dReturnValveDp = dDp; }
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' public virtual methods.
	virtual void Copy( CData *pclDestination );

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData *pclSelectionCompareWith, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////
	
// Protected members.
protected:

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	// Remark: Supply valve infos are saved in base class (respectively in 'm_FamilyIDPtr', 'm_VersionIDPtr' and 'm_ConnectIDPtr').
	CString m_strRVFamilyID;
	CString m_strRVVersionID;
	CString m_strRVConnectID;
	RadInfo_struct m_RadInfos;
	RadiatorReturnValveMode m_eReturnValveMode;
	int m_iActuatorType;
	int m_iRadiatorValveType;
	TCHAR m_strSVInsertName[ _COMBO_ITEMNAME_MAX + 1 ];
	bool m_fIsInsertInKv;
	double m_dInsertKvValue;
	CString m_strActuatorTypeID;
	CString m_strActuatorFamilyID;
	CString m_strActuatorVersionID;

	// There is no actuator is base class 'CDS_SSel'.
	IDPTR m_strSupplyValveActuatorID;
	CAccessoryList m_clSVActuatorAccessoryList;

	// Remark: Dp here is the Dp computed when running select supply & return valve in 'CSelectedTrv'. Total Dp entered by user
	//         is saved in 'm_dDp' in the base class!!
	double m_dSupplyValveDp;

	// Remark: Supply valve is saved in base class (in 'm_ValveIDPtr').
	IDPTR m_strReturnValveID;

	// Remark: Accessories for supply valve are saved in base class (in 'm_clAccessoryList').
	CAccessoryList m_clReturnValveAccessoryList;

	// Remark: Supply valve opening is saved in base class (in 'CDS_SSel::m_dOpening').
	double m_dReturnValveOpening;

	// Remark: Dp here is the Dp computed when running select supply & return valve in 'CSelectedTrv'. Total Dp entered by user
	//         is saved in 'm_dDp' in the base class!!
	double m_dReturnValveDp;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelCtrl
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelCtrl : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelCtrl )

protected:
	CDS_SSelCtrl( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelCtrl() {}

public:
	// Overrides 'CDS_SelProd'.
	virtual bool IsSelectedAsAPackage( bool bVerifyPackage = false );

	CDB_ControlProperties::CV2W3W GetCtrlProp() { return m_CtrlProp; }
	CDB_ControlProperties::CvCtrlType GetCtrlType() { return m_CtrlType; }
	IDPTR GetActrIDPtr() { return GetMbrIDPtr( m_ActrIDPtr); }
	virtual IDPTR GetCvActrSetIDPtr();
	virtual void ResetCVActrSetIDPtr();

	CAccessoryList *GetCvAccessoryList() { return &m_clAccessoryList; }

	bool IsActuatorAccessoryExist( void ) { return ( 0 == m_clActuatorAccessoryList.GetCount() ) ? false : true; }
	CAccessoryList *GetActuatorAccessoryList() { return &m_clActuatorAccessoryList; }

	CDB_Actuator *GetpActuator() { return dynamic_cast<CDB_Actuator*>(GetActrIDPtr().MP); }

	void SetCtrlProp( CDB_ControlProperties::CV2W3W CtrlProp ) { if( CtrlProp!=m_CtrlProp ) { m_CtrlProp = CtrlProp; Modified(); } }
	void SetCtrlType( CDB_ControlProperties::CvCtrlType CtrlType ) { if( CtrlType!=m_CtrlType ) { m_CtrlType = CtrlType; Modified(); } }
	void SetActrIDPtr( const IDPTR &idptr ) { SetMbreIDPtr( m_ActrIDPtr, idptr); }

	void SetCheckPackageEnable( BOOL fEnable ) { m_fCheckPackageEnable = fEnable; }
	BOOL GetCheckPackageEnable( void ) { return m_fCheckPackageEnable; }

	// Contains information about actuators.
	void SetActuatorPowerSupplyID( CString strActuatorPowerSupplyID );
	void SetActuatorInputSignalID( CString strActuatorInputSignalID );
	void SetFailSafeFunction( int iFailSafeFct );
	void SetDRPFunction( CDB_ControlValve::DRPFunction eDefaultReturnPosFct );
	CString GetActuatorPowerSupplyIDPtr() { return m_strActuatorPowerSupplyID; }
	CString GetActuatorInputSignalIDPtr() { return m_strActuatorInputSignalID; }
	int GetFailSafeFunction() { return m_iFailSafeFunction; }
	CDB_ControlValve::DRPFunction GetDRPFunction() { return m_eDefaultRPFunction; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CDB_ControlProperties::CV2W3W m_CtrlProp;
	CDB_ControlProperties::CvCtrlType m_CtrlType;
	IDPTR m_ActrIDPtr;
	IDPTR m_CvActrSetIDPtr;
	CAccessoryList m_clActuatorAccessoryList;
	BOOL m_fCheckPackageEnable;
	CString m_strActuatorPowerSupplyID;
	CString m_strActuatorInputSignalID;
	int m_iFailSafeFunction;
	CDB_ControlValve::DRPFunction m_eDefaultRPFunction;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelCv
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelCv : public CDS_SSelCtrl
{
	DEFINE_DATA( CDS_SSelCv )

public:
	enum KvsOrDp
	{
		Kvs,					// defined as a Kvs
		Dp,						// defined as Dp
		Last
	};

protected:
	CDS_SSelCv( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelCv() {}

// Public methods.
public:
	double GetDp() { return m_dDp; }
	double GetKvs() { return m_dKvs; }
	bool GetCheckBoxKvsDp() { return m_bCheckBoxKvsDp; }
	KvsOrDp GetRadioKvsOrDp() { return m_eRadioKvsOrDp; }

	void SetKvs( double dKvs ) { SetMbreDouble( dKvs, m_dKvs ); }
	void SetDp( double dDp ) { SetMbreDouble( dDp, m_dDp ); }
	void SetCheckBoxKvsDp( bool bSet ) { if( bSet!=m_bCheckBoxKvsDp ) { m_bCheckBoxKvsDp = bSet; Modified(); } }
	void SetRadioKvsOrDp( KvsOrDp eKvsOrDp ) { if( eKvsOrDp != m_eRadioKvsOrDp ) { m_eRadioKvsOrDp = eKvsOrDp; Modified(); } }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dDp;
	double m_dKvs;
	bool m_bCheckBoxKvsDp;
	KvsOrDp m_eRadioKvsOrDp;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelPICv
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelPICv : public CDS_SSelCtrl
{
	DEFINE_DATA( CDS_SSelPICv )

protected:
	CDS_SSelPICv( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelPICv() {}

public:
	virtual IDPTR GetCvActrSetIDPtr() { return GetPICVActrSetIDPtr(); }
	virtual void ResetCVActrSetIDPtr();
	IDPTR GetPICVActrSetIDPtr();

	double GetDpMax() const { return m_dDpMax; }
	void SetDpMax( double val ) { if( m_dDpMax != val) { m_dDpMax = val; Modified(); } }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	IDPTR m_PICVActrSetIDPtr;
	double m_dDpMax;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelBCv
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelBCv : public CDS_SSelCtrl
{
	DEFINE_DATA( CDS_SSelBCv )

protected:
	CDS_SSelBCv( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelBCv() {}

// Public methods.
public:
	virtual IDPTR GetCvActrSetIDPtr() { return GetBCVActrSetIDPtr(); }
	virtual void ResetCVActrSetIDPtr();
	IDPTR GetBCVActrSetIDPtr();
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	IDPTR m_BCVActrSetIDPtr;
};


///////////////////////////////////////////////////////////////////////////////////
//	HYS-1150							
//								CDS_SSel6WayValve
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSel6WayValve : public CDS_SSelCtrl
{
	DEFINE_DATA( CDS_SSel6WayValve )

protected:
	CDS_SSel6WayValve( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSel6WayValve();

public:
	// Just to point on the 'm_ProductIDPtr' member of the 'CDS_SSel' base class.
	IDPTR Get6WayValveIDPtr() { return CDS_SSel::GetProductIDPtr(); }
	void Set6WayValveIDPtr( const IDPTR &idptr ) { CDS_SSel::SetProductIDPtr( idptr ); }

	// Just to point on the 'm_ActrIDPtr' member of the 'CDS_SSelCtrl' base class.
	IDPTR Get6WayValveActuatorIDPtr() { return CDS_SSelCtrl::GetActrIDPtr(); }
	void Set6WayValveActuatorIDPtr( const IDPTR &idptr ) { CDS_SSelCtrl::SetActrIDPtr( idptr ); }

	// Just to point on the 'm_clAccessoryList' member of the 'CDS_SSel' base class.
	bool Is6WayValveAccessoryExist( void ) { return CDS_SSel::IsAccessoryExist(); }
	CAccessoryList *Get6WayValveAccessoryList( void ) { return CDS_SSel::GetAccessoryList(); }

	// Just to point on the 'm_clActuatorAccessoryList' member of the 'CDS_SSelCtrl' base class.
	bool Is6WayValveActuatorAccessoryExist( void ) { return CDS_SSelCtrl::IsActuatorAccessoryExist(); }
	CAccessoryList *Get6WayValveActuatorAccessoryList( void ) { return CDS_SSelCtrl::GetActuatorAccessoryList(); }

	// For pressure independent balancing & control valve.
	CDS_SSelPICv *GetCDSSSelPICv( SideDefinition eSideDefinition );

	// For balancing valve.
	CDS_SSelBv *GetCDSSSelBv( SideDefinition eSideDefinition );

	// For internal use only!
	void SetCDSSelPIVvIDPtr1( IDPTR IDPtrCDSSSelPICv1 );
	void SetCDSSelPIVvIDPtr2( IDPTR IDPtrCDSSSelPICv2 );
	void SetCDSSelBvIDPtr1( IDPTR IDPtrCDSSSelBv1 );
	void SetCDSSelBvIDPtr2( IDPTR IDPtrCDSSSelBv2 );

	int GetCheckboxFastConnection() const { return m_iCheckboxFastConnection; }
	void SetCheckboxFastConnection( int iCheckboxWithCOActr ) { if( m_iCheckboxFastConnection != iCheckboxWithCOActr ) { m_iCheckboxFastConnection = iCheckboxWithCOActr; Modified(); } }
	
	// HYS-1877: Version 2
	int GetCheckboxOnlyForSet() const { return m_iCheckboxOnlyForSet; }
	void SetCheckboxOnlyForSet( int iCheckboxOnlyForSet ) { if( m_iCheckboxOnlyForSet != iCheckboxOnlyForSet ) { m_iCheckboxOnlyForSet = iCheckboxOnlyForSet; Modified(); } }

	int GetSelectionMode() const { return m_eSelectionMode; }
	void SetSelectionMode( e6WayValveSelectionMode eCircuitType ) { if( m_eSelectionMode != eCircuitType ) { m_eSelectionMode = eCircuitType; Modified(); } }

	// Pay attention: here for cooling and heating, we have local variables defined in this class.
	// For the current water characteristic corresponding to the ribbon, it's in the base class.
	double GetCoolingFlow() { return m_dCoolingFlow; }
	double GetCoolingPower() { return m_dCoolingPower; }
	double GetCoolingDT() { return m_dCoolingDT; }
	CWaterChar &GetCoolingWC() { return m_clCoolingWC; }

	void SetCoolingFlow( double dCoolingFlow ) { SetMbreDouble( dCoolingFlow, m_dCoolingFlow ); }
	void SetCoolingPower( double dCoolingPower ) { SetMbreDouble( dCoolingPower, m_dCoolingPower ); }
	void SetCoolingDT( double dCoolingDT ) { SetMbreDouble( dCoolingDT, m_dCoolingDT ); }
	void SetCoolingWC( CWaterChar clCoolingWC ) { m_clCoolingWC = clCoolingWC; }

	double GetHeatingFlow() { return m_dHeatingFlow; }
	double GetHeatingPower() { return m_dHeatingPower; }
	double GetHeatingDT() { return m_dHeatingDT; }
	CWaterChar &GetHeatingWC() { return m_clHeatingWC; }
	
	void SetHeatingFlow( double dHeatingFlow ) { SetMbreDouble( dHeatingFlow, m_dHeatingFlow ); }
	void SetHeatingPower( double dHeatingPower ) { SetMbreDouble( dHeatingPower, m_dHeatingPower ); }
	void SetHeatingDT( double dHeatingDT ) { SetMbreDouble( dHeatingDT, m_dHeatingDT ); }
	void SetHeatingWC( CWaterChar clHeatingWC ) { m_clHeatingWC = clHeatingWC; }

	double GetBiggestFlow() { return max( GetHeatingFlow(), GetCoolingFlow() ); }
	double GetBiggestPower() { return ( GetCoolingFlow() > GetHeatingFlow() ) ? GetCoolingPower() : GetHeatingPower(); }
	double GetBiggestDT() { return ( GetCoolingFlow() > GetHeatingFlow() ) ? GetCoolingDT() : GetHeatingDT(); }

	// HYS-1380: These functions should not be called when the SideDefinition is BothSide.
	double GetFlow( SideDefinition eSideDefinition ) { return ( CoolingSide == eSideDefinition ) ? GetCoolingFlow() : GetHeatingFlow(); }
	double GetPower( SideDefinition eSideDefinition ) { return ( CoolingSide == eSideDefinition ) ? GetCoolingPower() : GetHeatingPower(); }
	double GetDT( SideDefinition eSideDefinition ) { return ( CoolingSide == eSideDefinition ) ? GetCoolingDT() : GetHeatingDT(); }
	CWaterChar &GetWC( SideDefinition eSideDefinition ) { return ( CoolingSide == eSideDefinition ) ? GetCoolingWC() : GetHeatingWC(); }
	
	// HYS-1877: Use 6-way valve set tab.
	virtual IDPTR GetCvActrSetIDPtr() { return Get6WayValveActrSetIDPtr(); }
	IDPTR Get6WayValveActrSetIDPtr();
	void ResetCVActrSetIDPtr();
	bool IsSelectedAsAPackage( bool bVerifyPackage = false );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dCoolingFlow;
	double m_dCoolingPower;
	double m_dCoolingDT;
	CWaterChar m_clCoolingWC;

	double m_dHeatingFlow;
	double m_dHeatingPower;
	double m_dHeatingDT;
	CWaterChar m_clHeatingWC;
	
	///////////////////////////////////////////////////////////////
	// For pressure independent balancing & control valve.

	// These variables are saved.
	IDPTR m_IDPtrCDSSSelPICv1;
	IDPTR m_IDPtrCDSSSelPICv2;
	
	// These variables are not saved.
	CDS_SSelPICv *m_pclCDSSelPICv1;
	CDS_SSelPICv *m_pclCDSSelPICv2;
	///////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////
	// For balancing valve.

	// These variables are saved.
	IDPTR m_IDPtrCDSSSelBv1;
	IDPTR m_IDPtrCDSSSelBv2;

	// These variables are not saved.
	CDS_SSelBv *m_pclCDSSelBv1;
	CDS_SSelBv *m_pclCDSSelBv2;

	int m_iCheckboxFastConnection;
	e6WayValveSelectionMode m_eSelectionMode;
	// HYS-1877: Version 2
	int m_iCheckboxOnlyForSet;
	IDPTR m_6WayValveSetIDPtr;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelAirVentSeparator
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelAirVentSeparator : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelAirVentSeparator )

protected:
	CDS_SSelAirVentSeparator( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelAirVentSeparator() {}

public:
	virtual double GetOpening() const { return -1; }
	virtual void SetOpening( double dValue ) { /* Do nothing for separator */ }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelPMaint
//
///////////////////////////////////////////////////////////////////////////////////
class CPMInputUser;
class CDS_SSelPMaint : public CData
{
	DEFINE_DATA( CDS_SSelPMaint )

protected:
	CDS_SSelPMaint( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelPMaint();

public:
	enum SelectionType
	{
		Undefined = -1,
		Statico = 0,
		Compresso,
		Transfero,
		NoPressurization,			// Pleno/Vento alone.
	};

	void Reset();
	bool IsFromDirSel() { return m_bDirectSel; }
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Getter.
	CPMInputUser *GetpInputUser( void ) { return m_pclInputUser; }
	CSelectedInfos *GetpSelectedInfos( void ) { return m_pSelectedInfos; }

	ProductSelectionMode GetSelectionMode( void ) { return m_eSelectionMode; }
	SelectionType GetSelectionType( void ) { return m_eSelectionType; }

	// Values for vessel.
	// Returns either the Statico vessel, the primary Compresso vessel or the primary Transfero vessel.
	IDPTR GetVesselIDPtr() { return _GetMbrIDPtr( m_VesselIDPtr ); }
	short GetVesselNumber( void ) { return m_nVesselNumber; }

	// HYS-1022: 'pa' becomes 'pa,min' and is now common to vessel, pump and compressor selection. 
	// HYS-1022: 'pa,opt' becomes 'pa'.
	double GetVesselInitialPressure( void ) { return m_dVesselInitialPressure; }

	// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
	double GetVesselWaterReserve( void ) { return m_dVesselWaterReserve; }

	CAccessoryList *GetVesselAccessoryList( void ) { return &m_clVesselAccessoryList; }

	// Values for Tecbox.
	IDPTR GetTecBoxCompTransfIDPtr() { return _GetMbrIDPtr( m_TecBoxCompTransfIDPtr ); }
	short GetTecBoxCompTransfNumber( void ) { return m_nTecBoxCompTransfNumber; }
	CAccessoryList *GetTecBoxCompTransfAccessoryList( void ) { return &m_clTecBoxCompTransfAccessoryList; }
	// HYS-872
	CAccessoryList *GetTecBoxIntegratedVesselAccessoryList(void) { return &m_clTecBoxIntegratedVesselAccessoryList; }
	IDPTR GetTecBoxIntegratedVesselIDPtr() { return _GetMbrIDPtr( m_TecBoxIntegratedVesselIDPtr ); }

	// Values for secondary Compresso & Transfero vessel.
	IDPTR GetSecondaryVesselIDPtr() { return _GetMbrIDPtr( m_SecondaryVesselIDPtr ); }
	short GetSecondaryVesselNumber( void ) { return m_nSecondaryVesselNumber; }

	// Values for intermediate vessels.
	IDPTR GetIntermediateVesselIDPtr() { return _GetMbrIDPtr( m_IntermediateVesselIDPtr ); }
	short GetIntermediateVesselNumber( void ) { return m_nIntermediateVesselNumber; }
	CAccessoryList*	GetIntermediateVesselAccessoryList( void ) { return &m_clIntermediateVesselAccessoryList; }

	// Values for Pleno (Pleno alone).
	IDPTR GetTecBoxPlenoIDPtr() { return _GetMbrIDPtr( m_TecBoxPlenoIDPtr ); }
	// Remark: this is a IDPTR on a 'CDB_Set' object!
	IDPTR GetAdditionalTecBoxPlenoIDPtr() { return _GetMbrIDPtr( m_AdditionalTecBoxPlenoIDPtr ); }
	IDPTR GetTecBoxPlenoProtectionIDPtr() { return _GetMbrIDPtr( m_TecBoxPlenoProtectionIDPtr ); }
	short GetTecBoxPlenoNumber( void ) { return m_nTecBoxPlenoNumber; }
	CAccessoryList*	GetTecBoxPlenoAccessoryList( void ) { return &m_clTecBoxPlenoAccessoryList; }
	CAccessoryList*	GetTecBoxPlenoProtecAccessoryList( void ) { return &m_clTecBoxPlenoProtecAccessoryList; }

	// Values for Vento (Vento alone or a combination of VP).
	IDPTR GetTecBoxVentoIDPtr() { return _GetMbrIDPtr( m_TecBoxVentoIDPtr ); }
	short GetTecBoxVentoNumber( void ) { return m_nTecBoxVentoNumber; }
	CAccessoryList*	GetTecBoxVentoAccessoryList( void ) { return &m_clTecBoxVentoAccessoryList; }

	// Values for Pleno Refill (Selected with a Pleno or alone (in direct selection)).
	IDPTR GetPlenoRefillIDPtr() { return _GetMbrIDPtr( m_PlenoRefillIDPtr ); }
	short GetPlenoRefillNumber( void ) { return m_nPlenoRefillNumber; }
	CAccessoryList*	GetPlenoRefillAccessoryList( void ) { return &m_clPlenoRefillAccessoryList; }

	// If selection has been done in direct selection, this method returns the only one object selected. Otherwise if the selection
	// has been done without pressurisation, the returns will be either a Pleno or a Vento. Otherwise if it's a complete pressurisation
	// maintenance selection the method will return the main product (Vessel, Transfero or Compresso).
	CDB_Product* GetSelectedProduct( void );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Setter.
	void SetFromDirSel( bool fFlag ) { m_bDirectSel = fFlag; }

	// Both 'Individual' and 'Wizard' selection mode for pressurisation are saved in this class.
	// We just need to differentiate it when user wants to edit a result to go in individual or
	// wizard mode.
	void SetSelectionMode( ProductSelectionMode eProductSelectionMode ) { m_eSelectionMode = eProductSelectionMode; }

	void SetSelectionType( SelectionType eSelectionType ) { m_eSelectionType = eSelectionType; }
	
	// Values common to vessel.
	// Set either the Statico vessel, the primary Compresso vessel or the primary Transfero vessel.
	void SetVesselIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_VesselIDPtr, idptr ); }
	void SetVesselNumber( short nValue ) { m_nVesselNumber = nValue; }

	// HYS-1022: 'pa' becomes 'pa,min' and is now common to vessel, pump and compressor selection. 
	// HYS-1022: 'pa,opt' becomes 'pa'.
	void SetVesselInitialPressure( double dValue ) { m_dVesselInitialPressure = dValue; }

	// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
	void SetVesselWaterReserve( double dValue ) { m_dVesselWaterReserve = dValue; }

	void SetVesselAccessoryList( CAccessoryList* pclAccessory ) { m_clVesselAccessoryList.Clear(); pclAccessory->CopyTo( &m_clVesselAccessoryList ); }

	// Values for Tecbox.
	void SetTecBoxCompTransfIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_TecBoxCompTransfIDPtr, idptr ); }
	void SetTecBoxCompTransfNumber( short nValue ) { m_nTecBoxCompTransfNumber = nValue; }
	void SetTecBoxCompTransfAccessoryList( CAccessoryList* pclAccessory ) { m_clTecBoxCompTransfAccessoryList.Clear(); pclAccessory->CopyTo( &m_clTecBoxCompTransfAccessoryList ); }
	// HYS-872
	void SetTecBoxIntegratedVesselAccessoryList(CAccessoryList* pclAccessory) { m_clTecBoxIntegratedVesselAccessoryList.Clear(); pclAccessory->CopyTo(&m_clTecBoxIntegratedVesselAccessoryList); }
	void SetTecBoxIntegratedVesselIDPtr( const IDPTR& idtrp ) { _SetMbreIDPtr( m_TecBoxIntegratedVesselIDPtr, idtrp ); }

	// Values for secondary Compresso & Transfero vessel.
	void SetSecondaryVesselIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_SecondaryVesselIDPtr, idptr ); }
	void SetSecondaryVesselNumber( short nValue ) { m_nSecondaryVesselNumber = nValue; }
	
	// Values for intermediate vessels.
	void SetIntermediateVesselIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_IntermediateVesselIDPtr, idptr ); }
	void SetIntermediateVesselNumber( short nValue ) { m_nIntermediateVesselNumber = nValue; }
	void SetIntermediateVesselAccessoryList( CAccessoryList* pclAccessory ) { m_clIntermediateVesselAccessoryList.Clear(); pclAccessory->CopyTo( &m_clIntermediateVesselAccessoryList ); }

	// Values for Pleno (Pleno alone).
	void SetTecBoxPlenoIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_TecBoxPlenoIDPtr, idptr ); }
	// Remark: this is a IDPTR on a 'CDB_Set' object!
	void SetAdditionalTecBoxPlenoIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_AdditionalTecBoxPlenoIDPtr, idptr ); }
	void SetTecBoxPlenoProtectionIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_TecBoxPlenoProtectionIDPtr, idptr ); }
	void SetTecBoxPlenoNumber( short nValue ) { m_nTecBoxPlenoNumber = nValue; }
	void SetTecBoxPlenoAccessoryList( CAccessoryList *pclAccessory ) { m_clTecBoxPlenoAccessoryList.Clear(); pclAccessory->CopyTo( &m_clTecBoxPlenoAccessoryList ); }
	void SetTecBoxPlenoProtecAccessoryList( CAccessoryList *pclAccessory ) { m_clTecBoxPlenoProtecAccessoryList.Clear(); pclAccessory->CopyTo( &m_clTecBoxPlenoProtecAccessoryList ); }

	// Values for Vento (Vento alone or a combination of VP).
	void SetTecBoxVentoIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_TecBoxVentoIDPtr, idptr ); }
	void SetTecBoxVentoNumber( short nValue ) { m_nTecBoxVentoNumber = nValue; }
	void SetTecBoxVentoAccessoryList( CAccessoryList *pclAccessory ) { m_clTecBoxVentoAccessoryList.Clear(); pclAccessory->CopyTo( &m_clTecBoxVentoAccessoryList ); }

	// Values for Pleno Refill (Selected with a Pleno or alone (in direct selection)).
	void SetPlenoRefillIDPtr( const IDPTR& idptr ) { _SetMbreIDPtr( m_PlenoRefillIDPtr, idptr ); }
	void SetPlenoRefillNumber( short nValue ) { m_nPlenoRefillNumber = nValue; }
	void SetPlenoRefillAccessoryList( CAccessoryList *pclAccessory ) { m_clPlenoRefillAccessoryList.Clear(); pclAccessory->CopyTo( &m_clPlenoRefillAccessoryList ); }
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' public virtual methods.
	virtual void Copy( CData *pclDestination );

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData *pclSelectionCompareWith, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	IDPTR _GetMbrIDPtr( IDPTR &idptr, CTADatabase *pDB = NULL );
	void _SetMbreIDPtr( IDPTR &TrgIDPtr, const IDPTR &idptr, CTADatabase *pDB = NULL );

private:
	bool m_bDirectSel;
	CPMInputUser *m_pclInputUser;
	CSelectedInfos *m_pSelectedInfos;	// Selection informations.

	ProductSelectionMode m_eSelectionMode;
	SelectionType m_eSelectionType;	

	// Values for vessel.
	// In this variable is saved either the Statico vessel, primary Compresso vessel or primary Transfero vessel.
	IDPTR m_VesselIDPtr;
	short m_nVesselNumber;

	// HYS-1022: 'pa' becomes 'pa,min' and is now common to vessel, pump and compressor selection. 
	// Thus no more need of the 'm_dVesselInitialPressure' method.
	// HYS-1022: 'pa,opt' becomes 'pa'.
	double m_dVesselInitialPressure;

	// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
	double m_dVesselWaterReserve;

	CAccessoryList m_clVesselAccessoryList;

	// Values for Tecbox Compresso or Transfero.
	IDPTR m_TecBoxCompTransfIDPtr;
	short m_nTecBoxCompTransfNumber;
	CAccessoryList m_clTecBoxCompTransfAccessoryList;
	// HYS-872: Accessories for Tecbox integrated vessel
	CAccessoryList m_clTecBoxIntegratedVesselAccessoryList;
	IDPTR m_TecBoxIntegratedVesselIDPtr;

	// Values for secondary Compresso & Transfero vessel.
	IDPTR m_SecondaryVesselIDPtr;
	short m_nSecondaryVesselNumber;

	// Values for intermediate vessels.
	IDPTR m_IntermediateVesselIDPtr;
	short m_nIntermediateVesselNumber;
	CAccessoryList m_clIntermediateVesselAccessoryList;

	// Values for Pleno (Pleno alone).
	IDPTR m_TecBoxPlenoIDPtr;
	IDPTR m_AdditionalTecBoxPlenoIDPtr;		// Remark: this is a IDPTR on a 'CDB_Set' object!
											// We can have for example "P BA4 P" + "P AB5-R".
	IDPTR m_TecBoxPlenoProtectionIDPtr;
	short m_nTecBoxPlenoNumber;
	CAccessoryList m_clTecBoxPlenoAccessoryList;
	CAccessoryList m_clTecBoxPlenoProtecAccessoryList;

	// Values for Vento (Vento alone or a combination of VP).
	IDPTR m_TecBoxVentoIDPtr;
	short m_nTecBoxVentoNumber;
	CAccessoryList m_clTecBoxVentoAccessoryList;

	// Values for Pleno Refill (Selected with a Pleno or alone (in direct selection)).
	IDPTR m_PlenoRefillIDPtr;
	short m_nPlenoRefillNumber;
	CAccessoryList m_clPlenoRefillAccessoryList;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelDpCBCV
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelDpCBCV : public CDS_SSelCtrl
{
	DEFINE_DATA( CDS_SSelDpCBCV )

protected:
	CDS_SSelDpCBCV( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelDpCBCV() {}

public:
	virtual IDPTR GetCvActrSetIDPtr() { return GetDpCBCVActrSetIDPtr(); }
	virtual void ResetCVActrSetIDPtr();
	IDPTR GetDpCBCVActrSetIDPtr();

	double GetDpToStalibize() const { return m_dDpToStabilize; }
	void SetDpToStalibize( double dDpToStabilize ) { if( m_dDpToStabilize != dDpToStabilize ) { m_dDpToStabilize = dDpToStabilize; Modified(); } }

	double GetHMin( void ) const { return m_dHMin; }
	void SetHMin( double dHMin ) { if( m_dHMin != dHMin ) { m_dHMin = dHMin; Modified(); } }

	bool IsSVAccessoryExist( void ) { return ( 0 == m_clSVAccessoryList.GetCount() ) ? false : true; }
	CAccessoryList *GetSVAccessoryList( void ) { return &m_clSVAccessoryList; }

	IDPTR GetSVIDPtr() { return GetMbrIDPtr( m_SVIDPtr ); }
	void SetSVIDPtr( const IDPTR &idptr ) { SetMbreIDPtr( m_SVIDPtr, idptr ); }

	int GetCheckboxWithSTS() const { return m_iCheckboxWithSTS; }
	void SetCheckboxWithSTS( int iCheckboxWithSTS ) { if( m_iCheckboxWithSTS != iCheckboxWithSTS ) { m_iCheckboxWithSTS = iCheckboxWithSTS; Modified(); } }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	IDPTR m_DpCBCVActrSetIDPtr;
	double m_dDpToStabilize;
	double m_dHMin;
	CAccessoryList m_clSVAccessoryList;
	IDPTR m_SVIDPtr;
	int m_iCheckboxWithSTS;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelSafetyValve
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelSafetyValve : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelSafetyValve )

protected:
	CDS_SSelSafetyValve( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelSafetyValve() {}

public:
	CString GetSystemHeatGeneratorTypeID() { return m_strSystemHeatGeneratorTypeID; }
	CString GetNormID() { return m_strNormID; }
	double GetInstalledPower() { return m_dInstalledPower; }
	double GetInstalledCollector() { return m_dInstalledCollector; }
	CString GetSafetyValveFamilyID() { return m_strSafetyValveFamilyID; }
	CString GetSafetyValveConnectID() {return m_strSafetyValveConnectID; }
	double GetSetPressure() { return m_dSetPressure; }
	int GetQuantityNeeded() { return m_iQuantityNeeded; }

	// In direct selection.
	IDPTR GetBlowTankIDPtr() { return GetMbrIDPtr( m_BlowTankIDPtr ); }
	bool IsBlowTankAccessoryExist( void ) { return ( 0 == m_clBlowTankAccessoryList.GetCount() ) ? false : true; }
	CAccessoryList *GetBlowTankAccessoryList( void ) { return &m_clBlowTankAccessoryList; }
	
	void SetSystemHeatGeneratorTypeID( CString strSystemHeatGeneratorTypeID );
	void SetNormID( CString strNormID );
	void SetInstalledPower( double dInstalledPower ) { if( dInstalledPower != m_dInstalledPower ) { m_dInstalledPower = dInstalledPower; Modified(); } }
	void SetInstalledCollector( double dInstalledCollector ) { if( dInstalledCollector != m_dInstalledCollector ) { m_dInstalledCollector = dInstalledCollector; Modified(); } }
	void SetSafetyValveFamilyID( CString strSafetyValveTypeID );
	void SetSafetyValveConnectID( CString strSafetyValveConnectID );
	void SetSetPressure( double dSetPressure ) { if( dSetPressure != m_dSetPressure ) { m_dSetPressure = dSetPressure; Modified(); } }
	void SetQuantityNeeded( int iQuantityNeeded ) { if( iQuantityNeeded != m_iQuantityNeeded ) { m_iQuantityNeeded = iQuantityNeeded; Modified(); } }
	
	void SetBlowTankIDPtr( const IDPTR &idptr ) { SetMbreIDPtr( m_BlowTankIDPtr, idptr ); }
	
	virtual double GetOpening() const { return -1; }
	virtual void SetOpening( double dValue ) { /* Do nothing for safety valve */ }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CString m_strSystemHeatGeneratorTypeID;
	CString m_strNormID;
	double m_dInstalledPower;
	double m_dInstalledCollector;
	CString m_strSafetyValveFamilyID;
	CString m_strSafetyValveConnectID;
	double m_dSetPressure;
	int m_iQuantityNeeded;
	IDPTR m_BlowTankIDPtr;
	CAccessoryList m_clBlowTankAccessoryList;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelFloorHeatingManifold
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelFloorHeatingManifold : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelFloorHeatingManifold )

protected:
	CDS_SSelFloorHeatingManifold( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelFloorHeatingManifold() {}

public:
	virtual double GetOpening() const { return -1; }
	virtual void SetOpening( double dValue ) { /* Do nothing for floor heating manifold */ }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelFloorHeatingValve
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelFloorHeatingValve : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelFloorHeatingValve )

protected:
	CDS_SSelFloorHeatingValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelFloorHeatingValve() {}

public:
	virtual double GetOpening() const { return -1; }
	virtual void SetOpening( double dValue ) { /* Do nothing for floor heating control valve */ }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelFloorHeatingController
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelFloorHeatingController : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelFloorHeatingController )

protected:
	CDS_SSelFloorHeatingController( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelFloorHeatingController() {}

public:
	virtual double GetOpening() const { return -1; }
	virtual void SetOpening( double dValue ) { /* Do nothing for floor heating controller */ }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelTapWaterControl
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelTapWaterControl : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelTapWaterControl )

protected:
	CDS_SSelTapWaterControl( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelTapWaterControl() {}

public:
	virtual double GetOpening() const { return -1; }
	virtual void SetOpening( double dValue ) { /* Do nothing for tap water control */ }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelSmartControlValve
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelSmartControlValve : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelSmartControlValve )

protected:
	CDS_SSelSmartControlValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelSmartControlValve() {}

// Public methods.
public:
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

	bool GetCheckboxDpMax() const { return m_bCheckboxDpMax; }
	void SetCheckboxDpMax( bool bCheckboxDpMax ) { if( m_bCheckboxDpMax != bCheckboxDpMax ) { m_bCheckboxDpMax = bCheckboxDpMax; Modified(); } }
	bool IsCheckboxDpMaxChecked() { return ( BST_CHECKED == m_bCheckboxDpMax ) ? true : false; }
	
	double GetDpMax() const { return m_dDpMax; }
	void SetDpMax( double dDpMaxValue ) { if( m_dDpMax != dDpMaxValue ) { m_dDpMax = dDpMaxValue; Modified(); } }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

private:
	bool m_bCheckboxDpMax;
	double m_dDpMax;
};

///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelSmartDpC
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SSelSmartDpC : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelSmartDpC )

protected:
	CDS_SSelSmartDpC( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelSmartDpC() {}

// Public methods.
public:
	IDPTR GetSmartDpCIDPtr() { return CDS_SSel::GetProductIDPtr(); }
	void SetSmartDpCIDPtr( const IDPTR& idptr ) { CDS_SSel::SetProductIDPtr( idptr ); }

	int GetCheckboxDpBranch() const { return m_iCheckboxDpBranch; }
	void SetCheckboxDpBranch( int iCheckboxDpBranch ) { if( m_iCheckboxDpBranch != iCheckboxDpBranch ) { m_iCheckboxDpBranch = iCheckboxDpBranch; Modified(); } }
	bool IsCheckboxDpBranchChecked() { return ( BST_CHECKED == m_iCheckboxDpBranch ) ? true : false; }

	double GetDpBranchValue() const { return m_dDpBranch; }
	void SetDpBranchValue( double dDpBranch ) { if( m_dDpBranch != dDpBranch ) { m_dDpBranch = dDpBranch; Modified(); } }

	int GetCheckboxDpMax() const { return m_iCheckboxDpMax; }
	void SetCheckboxDpMax( int iCheckboxDpMax ) { if( m_iCheckboxDpMax != iCheckboxDpMax ) { m_iCheckboxDpMax = iCheckboxDpMax; Modified(); } }
	bool IsCheckboxDpMaxChecked() { return ( BST_CHECKED == m_iCheckboxDpMax ) ? true : false; }
	
	double GetDpMaxValue() const { return m_dDpMax; }
	void SetDpMaxValue( double dDpMax ) { if( m_dDpMax != dDpMax ) { m_dDpMax = dDpMax; Modified(); } }

	// We can have Dp sensor not in a set with the smart differential pressure controller selected.
	void SetDpSensorIDPtr( const IDPTR &idptr ) { _SetMbreIDPtr( m_DpSensorIDPtr, idptr ); }
	IDPTR GetDpSensorIDPtr() { return _GetMbrIDPtr( m_DpSensorIDPtr ); }

	void SetDpSensorAccessoryList( CAccessoryList *pclDpSensorAccessoryList ) { m_clDpSensorAccessoryList.Clear( CAccessoryList::_AT_Accessory ); pclDpSensorAccessoryList->CopyTo( &m_clDpSensorAccessoryList ); }
	CAccessoryList *GetDpSensorAccessoryList( void ) { return &m_clDpSensorAccessoryList; }
	bool IsDpSensorAccessoryExist( void ) { return ( 0 == m_clDpSensorAccessoryList.GetCount() ) ? false : true; }
	
	// If selected by a set, we now have instead of Dp sensor group the "Sets" group that contains 2 Dp sensors and a virtual object "Connection set" to
	// allow to selected smart differential pressure controller without Dp sensor but with connection accessories set.
	void SetSetIDPtr( const IDPTR &idptr ) { _SetMbreIDPtr( m_SetIDPtr, idptr ); }
	IDPTR GetSetIDPtr() { return _GetMbrIDPtr( m_SetIDPtr ); }

	void SetSetContentAccessoryList( CAccessoryList *pclSetContentAccessoryList ) { m_clSetContentAccessoryList.Clear( CAccessoryList::_AT_SetAccessory ); pclSetContentAccessoryList->CopyTo( &m_clSetContentAccessoryList ); }
	CAccessoryList *GetSetContentAccessoryList( void ) { return &m_clSetContentAccessoryList; }
	bool IsSetContentAccessoryExist( void ) { return ( 0 == m_clSetContentAccessoryList.GetCount() ) ? false : true; }
	
	// HYS-1992: Remove set possibility for TA Smart Dp.
	//int GetCheckboxOnlyForSet() const { return m_iCheckboxOnlyForSet; }
	//void SetCheckboxOnlyForSet( int iCheckboxOnlyForSet ) { if( m_iCheckboxOnlyForSet != iCheckboxOnlyForSet ) { m_iCheckboxOnlyForSet = iCheckboxOnlyForSet; Modified(); } }
	//bool IsCheckboxOnlyForSetChecked() { return ( BST_CHECKED == m_iCheckboxOnlyForSet ) ? true : false; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Private members.
private:
	IDPTR _GetMbrIDPtr( IDPTR &idptr, CTADatabase *pDB = NULL );
	void _SetMbreIDPtr( IDPTR &TrgIDPtr, const IDPTR &idptr, CTADatabase *pDB = NULL );

// Private variables.
private:
	IDPTR m_DpSensorIDPtr;
	IDPTR m_SetIDPtr;
	CAccessoryList m_clDpSensorAccessoryList;
	CAccessoryList m_clSetContentAccessoryList;
	int m_iCheckboxDpBranch;
	double m_dDpBranch;
	int m_iCheckboxDpMax;
	double m_dDpMax;
	// HYS-1992: Remove set possibility for TA Smart Dp.
	int m_iCheckboxOnlyForSet; // No more used
};


///////////////////////////////////////////////////////////////////////////////////
//
//								HYS-1741: CDS_SelPWQAccServices
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SelPWQAccServices : public CDS_SSel
{
	DEFINE_DATA( CDS_SelPWQAccServices )

protected:
	CDS_SelPWQAccServices( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SelPWQAccServices() {}

public:
	virtual double GetOpening() const { return -1; }
	virtual void SetOpening( double dValue ) { /* Do nothing */ }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SelPWQPressureReducer
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_SelPWQPressureReducer : public CDS_SelPWQAccServices
{
	DEFINE_DATA( CDS_SelPWQPressureReducer )

protected:
	CDS_SelPWQPressureReducer( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SelPWQPressureReducer() {}

public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.
	double GetMaxInletPressure() { return m_dMaxInletPressure; }
	double GetMinOutletPressure() { return m_dMinOutletPressure; }
	double GetMaxOutletPressure() { return m_dMaxOutletPressure; }
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.
	void SetMaxInletPressure( double dMaxInletPressure ) { m_dMaxInletPressure = dMaxInletPressure; }
	void SetMinOutletPressure( double dMinOuletPressure ) { m_dMinOutletPressure = dMinOuletPressure; }
	void SetMaxOutletPressure( double dMaxOuletPressure ) { m_dMaxOutletPressure = dMaxOuletPressure; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables
protected:
	// Pressure information.
	double m_dMaxInletPressure;
	double m_dMinOutletPressure;
	double m_dMaxOutletPressure;
};

///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_SSelDpSensor
//
///////////////////////////////////////////////////////////////////////////////////
// HYS - 2007: Add CDS_SSelDpSensor object for TA Link selected alone.
class CDS_SSelDpSensor : public CDS_SSel
{
	DEFINE_DATA( CDS_SSelDpSensor )

protected:
	CDS_SSelDpSensor( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CDS_SSelDpSensor();

	// Public methods.
public:
	virtual CSelectedInfos* GetpSelectedInfos() { return m_pSelectedInfos; }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' public virtual methods.
	virtual void Copy( CData* pclDestination );

	// Allow to compare a selection inherited class with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData* pclSelectionCompareWith, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CSelectedInfos* m_pSelectedInfos;
};

///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_Actuator
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_Actuator : public CData
{
	DEFINE_DATA( CDS_Actuator )

	typedef struct AccessoryItem
	{
		IDPTR IDPtr;
		bool fByPair;
		// HYS-987: lEditedQty to save the accessory quantity edited in confirm sel 
		long lEditedQty;

	};

protected:
	CDS_Actuator( CDataBase *pDataBase, LPCTSTR ID );
	~CDS_Actuator();

// Public methods.
public:
	virtual CSelectedInfos *GetpSelectedInfos() { return m_pSelectedInfos; }
	void SetActuatorIDPtr( const IDPTR &idptr );
	void AddActuatorAccessory( const IDPTR &idptr, CDB_RuledTableBase *pclRuledTable = NULL );
	void AddActuatorAccessory( AccessoryItem *prAccessoryItem );
	// Set lEditedQty
	void SetEditedQty( AccessoryItem AccItem, long lQty );
	IDPTR GetActuatorIDPtr();
	std::vector<AccessoryItem> *GetpActuatorAccessoryList() { return &m_vecActuatorAccessory; }
	void ClearActuatorAccessories() { m_vecActuatorAccessory.clear(); }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' public virtual methods.
	virtual void Copy( CData *pclDestination );

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData *pclSelectionCompareWith, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables
protected:
	// Selection informations.
	CSelectedInfos *m_pSelectedInfos;
	IDPTR m_ActuatorIDPtr;
	std::vector<AccessoryItem> m_vecActuatorAccessory;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_ElectroActuator
//
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_Accessory
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_Accessory : public CData
{
	DEFINE_DATA( CDS_Accessory )

protected:
	CDS_Accessory( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_Accessory();

// Public methods.
public:
	virtual CSelectedInfos *GetpSelectedInfos() { return m_pSelectedInfos; }
	void SetAccessoryIDPtr( const IDPTR &idptr );

	IDPTR GetAccessoryIDPtr();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' public virtual methods.
	virtual void Copy( CData *pclDestination );

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData *pclSelectionCompareWith, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////
	
// Protected variables.
protected:
	CSelectedInfos *m_pSelectedInfos;
	IDPTR m_AccessoryIDPtr;
};
#endif


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CTableHM
// Tools table replace the 'standard' CTable for PIPING_TAB
// 
///////////////////////////////////////////////////////////////////////////////////
class CTableHM : public CTable
{
	DEFINE_DATA( CTableHM )

protected:
	CTableHM( CDataBase *pDataBase, LPCTSTR ID );
	
public:
	bool SaveToHMXTable( CTable *pTabSrcHM, CTable *pTabTrgHMX, CDS_WaterCharacteristic* pWC );
	
	// Load/Create all HM based on the HMX table.
	bool LoadFromHMXTable( bool fFromScratch, CTable *pTabHMX, CDS_HydroMod *pHMParent = NULL, int iLevel = 0 );

	// Allow to check what is the upper and lower limit for date and time of all HydroMod.
	void GetLowerUpperDataTime( COleDateTime& clLowerDateTime, COleDateTime& clUpperDateTime );

	// Access to the Diversity flags
	bool IsDiversityExistSomewhere(CTable *pTab=NULL);
	// "IsDiversityExist()" is a method used in the sheethmcal to display columns related to diversity
	bool IsDiversityExist() const { return m_bDiversityExist; }
	void SetDiversityExist(bool val) { m_bDiversityExist = val; }

	// Used during computing phase to save valve w/o diversity
	bool GetDiversityActive() const { return m_fDiversityActive; }
	void SetDiversityActive(bool val) { m_fDiversityActive = val; }

	// Allow to retrieve a list of modules on which it has been applied a diversity factor by the user.
	// Param: 'pHMRoot' is NULL if we must scan all root modules in PIPING_TAB or not NULL for a specific root module.
	// Param: 'ModuleList' is a vector that will receive the list of concerned modules.
	// Param: 'dDiversityFactor' will received the diversity factor choose by user.
	typedef std::vector<CDS_HydroMod*> vecHydroMod;
	bool GetDFModuleList( CDS_HydroMod *pHMRoot, vecHydroMod& ModuleList, double& dDiversityFactor );
								  
	// Scan all children return true if constant flow, false for variable flow
	// Update variable m_bConstantFlow of pressure interface
	// function update variable m_bConstantFlow for the branch's pressure interface
	bool CheckIsConstantFlow(CDS_HydroMod *pPressIntHM);

	// Function used to identify and list module including a bypass as injection circuits, pump, bypass
	std::multimap<int, CDS_HydroMod *> *GetpPressureInterfaceMap() { return &m_mapPressureInterfaces; }
	void FillPressureInterfaceMap( CDS_HydroMod *pHM, bool bOnlyInjectionPressureInterface = false );

	// HYS-1716: allows to retrieve a list of all design temperature interfaces.
	// Param 'bDeepestToHighest': if 'true' the first design interface is the one that is the deepest in the network and the last
	// is generally the 'pHM' itself.
	void FillDesignTemperatureInterfaceMap( CDS_HydroMod *pHM, bool bDeepestToHighest = true );
	std::multimap<int, CDS_HydroMod *> *GetpDesignTemperatureInterfaceMap() { return &m_mapDesignTemperatureInterfaces; }

	// HYS-1716: allows to retrieve a list of all hydraulic injection circuits (Except 3-way mixing circuit).
	// Param 'bAddParent' -> 'true' if we must add 'pHM' or 'false' if it's only its children.
	// Param 'bDeepestToHighest': if 'true' the first design interface is the one that is the deepest in the network and the last
	// is generally the 'pHM' itself.
	// This method is called by the 'IsInjectionCircuitTemperatureErrorForPasteOperation' and 'IsInjectionCircuitTemperatureErrorForEditOperation' methods.
	// The first method is called when we do a copy/paste operation while the second is called when we edit design temperatures at the secondary side of an injection circuit
	// or when we change global supply temperature of the project.
	// For a paste operation, we need top verify the temperatures for the parent and its children -> 'bAddParent' set to 'true'.
	// For a edit operation, it will depend of where is the edit:
	//   * global settings       : all the network will be parse, so the 'FillInjectionCircuitMap' will be called for each root module -> 'bAddParent' set to 'true'.
	//   * secondady temperatures: only the children must be verified -> 'bAddParent' set to 'false'.
	void FillInjectionCircuitMap( std::map<int, CDS_HydroMod *> *pmapHydraulicInjectionCircuit, CDS_HydroMod *pHM, bool bAddParent = true, bool bDeepestToHighest = true );

	// HYS-1716: Verify if there are errors for a source table containing design temperature interfaces with one other table in 
	// which we want to insert this source table.
	// This method can be called when doing a Copy(Cut)/Paste operation in the network. In this case, the 'IsInjectionCircuitTemperatureErrorForPasteOperation' method 
	// will be called for each object in the clipboard (pclSourceTable) and for each selected line (pclDestinationTable).
	//
	// With the both 'pclSourceTable' and 'pclDestinatio' table we can have these different cases:
	//    +---------+---------+---------------------+------------------------------------------+
	// 	  |  Source |  Dest   |       Source        |                 Dest                     |
	//    +---------+---------+---------------------+------------------------------------------+
	//    | NULL    | NULL    | All the network     | Design temperature from general settings |
	//    | Defined | NULL    | HMSource & children | Design temperature from general settings |
	//    | NULL    | Defined | /                   | /                                        |
	//    | Defined | Defined | HMSource & children | Design temperature from HMwith           |
	// 	  +---------+---------+---------------------+------------------------------------------+
	//
	// Param 'pmmapInjectionCircuitWithTemperatureError': if defined, we keep a trace of all parent hydraulic circuits with errors in first position
	// and their parent at second position (Or NULL if general settings).
	// Remark: we use multimap because we can copy a injection circuit in two different modules for example. So we need to keep errors for 
	// both modules if exist.
	// Remark: the first elements in the 'pmmapInjectionCircuitWithTemperatureError' multimap is always an injection circuit (Without 
	// 3-way mixing circuit). 
	// !! ONLY the parent on which there is an error is inserted in the 'pmmapInjectionCircuitWithTemperatureError' multimap.
	// When calling the 'CorrectAllInjectionCircuits' method, all children will be verified and corrected if needed.
	// Remark: This method is also called when importing a part (Or all) of an hydraulic network from a project to the current one.
	// Param 'pvecAllInjectionCircuitsWithTemperatureError': if defined, we keep a trace of all hydraulic circuits with errors. Here, we insert
	// parent and children if also errors. It's to ease the list of injection circuit with errors in the 'CDlgInjectionError' dialog.
	// Param 'bProjectTypeDifferent' is set to 'true' when we try to import a project that has a application type different from the current one.
	bool IsInjectionCircuitTemperatureErrorForPasteOperation( CTable *pclSourceTable = NULL, CTable *pclDestinationTable = NULL,
			std::multimap<CDS_HydroMod *, CTable *> *pmmapInjectionCircuitWithTemperatureError = NULL, 
			std::vector<CDS_HydroMod *> *pvecAllInjectionCircuitsWithTemperatureError = NULL, bool bProjectTypeDifferent = false );

	// HYS-1716: Verify if there are errors for a source table containing design temperature interfaces with new design temperature that user wants to change
	// in the general settings or in a specific injection hydraulic circuit.
	// 'pclSourceTable' can be NULL, in that case we run all the network and the both 'dNewSupplyTemperature' and 'dNewReturnTemperature' variable are in 
	// fact the general setting for the project.
	// If 'pclSourceTable' is defined, it means that user wants to change design temperature for a specific injection hydraulic circuit. In that case we run
	// only the children.
	// 'bForGeneralSettings' must not be set when calling this method. If calling this method with 'pclSourceTable' to NULL, the method will run all
	// the roots existing in the network. And for this case, we need to add the root in the 'mapInjectionHydraulicCircuit' list to check the temperatures
	// Param 'pvecAllInjectionCircuitsWithTemperatureError': if defined, we keep a trace of all hydraulic circuits with errors. Here, we insert
	// parent and children if also errors. It's to ease the list of injection circuit with errors in the 'CDlgInjectionError' dialog.
	// Param 'bForApplicationTypeChange' is set to 'true' when the call comes from the change of the application type (Cooling <-> heating) in the ribbon.
	bool IsInjectionCircuitTemperatureErrorForEditOperation( CTable *pclSourceTable, double dNewSupplyTemperature, 
			std::multimap<CDS_HydroMod *, CTable *> *pmmapInjectionCircuitWithTemperatureError = NULL, 
			std::vector<CDS_HydroMod *> *pvecAllInjectionCircuitsWithTemperatureError = NULL, bool bForGeneralSettings = false, bool bForApplicationTypeChange = false );

	// HYS-1716: allows to correct all hydraulic circuits that has been collected by the 'IsInjectionCircuitTemperatureErrorForPasteOperation' method.
	// Param 'bForApplicationTypeChange' is set to 'true' when the call comes from the change of the application type (Cooling <-> heating) in the ribbon.
	void CorrectAllInjectionCircuits( std::multimap<CDS_HydroMod *, CTable *> *pmmapInjectionCircuitWithTemperatureError, bool bForApplicationTypeChange = false );

	// HYS-1716: allows to correct one circuit (Or module with children) that has just been pasted in one other module with a
	// copy(cut)/paste operation.
	// Param 'bForApplicationTypeChange' is set to 'true' when the call comes from the change of the application type (Cooling <-> heating) in the ribbon.
	void CorrectOneInjectionCircuit( CDS_HydroMod *pclSourceInjectionCircuit, CTable *pclDestinationTable, bool bForApplicationTypeChange = false );

	// HYS-1716: when doing a copy/paste, editing a secondary temperature of an injection circuit or changing the global temperature of the project,
	// it has an impact on all primary flows for injection circuit.
	// Before HYS-1716, when editing secondary temperature for example in the SheetPanelCirc2, we systematically called the 'CDS_HydroMod::ComputeHM' 
	// method in the 'CDS_HmInj::SetTs' and 'CDS_HmInj::SetTr' methods. And this call allowed to update the primary flow.
	// Because these 2 methods could be called even when reading a projet (In the middle of nowwhere), we removed this call to the
	// 'CDS_HydroMod::ComputeHM' method.
	// Remark: we need only to compute primary flow for injection circuits not the modules (Modules are automatically computed when
	//         the 'CDS_HydroMod::ComputeAll' method is called).
	void CorrectAllPrimaryFlow( CTable *pclParentTable = NULL );

	// Sometimes there is a bug with the date & time of measures coming from TA Scope (Values are negative).
	// In this case, we reset the date & time.
	void VerifyMeasTABalDateTime( void );

	// HYS-1716: When opening old project, we need to run the hydraulic circuits to update the water characteristic
	// in the pipes.
	// HYS-1882: Add a booleen to set the direction when we run through the project.
	void VerifyAllWaterCharacteristics( bool bDeepestToHighest = true );

	// HYS-1857: From HySelect v4.5.0.1 until v4.5.0.6 (Included) an error has been introduced concerning BV at secondary side for 3-way mixing circuits.
	// It was set in the "tadb.txt" file that the BV was optional but it's no true. So, we need to correct now all projects where user
	// has intentionally removed the BV.
	void VerifyAllBVSecForAll3WayCircuits( CTable *pTab = NULL );

	// HYS-1777: Get and fill the 3-way mixing circuit vector.
	std::vector<CDS_HydroMod*>* GetpVecHMToReviewChildrenAuthority() { return &m_vecHMToReviewForChildrenAuthority; }
	void FillHMToReviewForChildrenAuthority( CDS_HydroMod* pHM );

	// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the 
	// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
	virtual void Copy( CData *pTo, bool fResetDiversityFactor = true );
	virtual void DropData( OUTSTREAM outf, int* piLevel );

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _GetLowerUpperDataTime( CTable *pclHydroMod, COleDateTime& clLowerDateTime, COleDateTime& clUpperDateTime );
	bool _GetDFModuleList( CDS_HydroMod *pHMRoot, vecHydroMod& ModuleList, double& dDiversityFactor );
	void _VerifyMeasTABalDateTime( CTable *pTable );

// Private variables.
private:	
	bool m_bDiversityExist;
	bool m_fDiversityActive;
	int m_iResetMeasDateTimeIndex;
	int m_iResetTABalDateTimeIndex;
	std::multimap<int, CDS_HydroMod *> m_mapPressureInterfaces;
	std::multimap<int, CDS_HydroMod *> m_mapDesignTemperatureInterfaces;

	// HYS-1777: Add a vector to store all 3-way mixing modules
	std::vector<CDS_HydroMod*> m_vecHMToReviewForChildrenAuthority;
};
#endif	// TACBX


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_HMXTable
// Tools table replace the 'standard' CTable for PIPING_TAB
// 
///////////////////////////////////////////////////////////////////////////////////
class CDS_HydroModX;
class CDS_HMXTable : public CTable
{
	DEFINE_DATA( CDS_HMXTable )

protected:
	CDS_HMXTable( CDataBase* pDataBase, LPCTSTR ID ) : CTable( pDataBase, ID ){}

public:
	struct sHMXTab
	{
		double dHMXTotalQ;
		uint16_t HMXNbrMod;
		uint16_t HMXBalMod;
		uint16_t HMXNbrValve;
		uint16_t HMXAdjValve;
	};

	static bool WriteShortInfos( OUTSTREAM outf, sHMXTab *pHMXData );
	bool WriteShortInfos( OUTSTREAM outf );
	static bool ReadShortInfos( INPSTREAM  inpf, sHMXTab *pHMXData );
	void SetFlagReadyForBalancing( CTable *pTab = NULL );

	void CountBalancedValves( CTable *pTab, sHMXTab *pHMXData );
		
#ifndef TACBX
	// Run all distribution/circuit pipes in all modules to check if they exist in user database.
	bool IsAllPipeExistsInUserDB( CTable *pTable, CDataBase *pUserDB );

	// Replace user pipe with closest default pipe.
	void ReplaceUserPipeByGenericPipe( CTable *pTable, CDataBase *pUserDB );

	// Sometimes there is a bug with the date & time of measures coming from TA Scope (Values are negative).
	// In this case, we reset the date & time.
	void VerifyMeasTABalDateTime( void );
#endif

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

#ifndef TACBX
private:
	void _VerifyMeasTABalDateTime( CTable *pTable );
	int m_iMeasDateTimeIndex;
	int m_iTABalDateTimeIndex;
#endif
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_LogTable
// Tool table replace the 'standard' CTable for LOGGEDDATA_TAB
// 
///////////////////////////////////////////////////////////////////////////////////
class CDS_LogDataX;
class CDS_LogTable : public CTable
{
	DEFINE_DATA( CDS_LogTable )

// Constructor and destructor.
protected:
	CDS_LogTable( CDataBase *pDataBase, LPCTSTR ID ): CTable( pDataBase, ID ) {}

// Public structure defines.
public:
	struct sLogTable
	{
		unsigned LogType;
		wchar_t	Name[_NAME_MAX_LENGTH+1];
		unsigned uSize;
		__time32_t tStartingDateTime;
		unsigned short MeasStep;
	};
	
// Public methods.
public:
	static bool WriteShortInfos(OUTSTREAM outf,  std::vector<CDS_LogTable::sLogTable> *pList);
	bool WriteShortInfos( OUTSTREAM outf );
	static bool ReadShortInfos(INPSTREAM  inpf, std::vector<CDS_LogTable::sLogTable> *pList);

	//Function that verify the existance of the object (Logged data)
	// in the table by comparing the date time and the Name
	// -------------------------------------------------------------------------------------
	// In  : CDS_LogDataX *pLog				pointer to the object to verify
	//		 IDPTR *idptr					an uninitialize pointer
	// -------------------------------------------------------------------------------------
	// Out : bool bReturn					Return function that tells the object were found
	//		 IDPTR *idptr					Complete the idptr pointer of the found object in
	//										the table
	// -------------------------------------------------------------------------------------
	bool VerifyLogExist(CDS_LogDataX *pLog, IDPTR *idptr);

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_PlantX
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_PlantX : public CDB_MultiString
{
	DEFINE_DATA( CDS_PlantX )
	
protected:
	CDS_PlantX( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_PlantX() {}

// Public enum defines.
public:
	enum ePlant
	{
		Name,
		Description,
		ProjectUID,
		LastItem
	};

// Public methods.
public:
#ifndef TACBX
	#pragma warning ( disable : 4996)
#endif
	void SetTSCFileName(const char * fn){strcpy ( m_TSCFn,fn); Modified(); }
#ifndef TACBX
	#pragma warning ( default : 4996)
#endif
	const char *GetTSCFilename() { return m_TSCFn; }
	void SetLastModDate( __time32_t date ) { m_ModDate = date; Modified(); }
	__time32_t GetLastModDate() { return m_ModDate; }
	
	CDS_HMXTable::sHMXTab *GetpHMXTab() { return &m_HMXData; }

	// Just to get access to both 'Write' and 'Read' protected inherited methods.
	void WriteAccess( OUTSTREAM outf ) { Write( outf ); }
	bool ReadAccess( INPSTREAM inpf ) { return Read( inpf ); }
	
// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	std::map<__time32_t, uint16_t> m_QMMap;
	CDS_HMXTable::sHMXTab m_HMXData;
	std::vector<CDS_LogTable::sLogTable> m_LogVector;

	// TSC file name, used into the SCOPE, limited to 24+4 characters defined into vfilecnf.h
	char m_TSCFn[30];

	// Last modification date
	__time32_t m_ModDate;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_QMTable
//
///////////////////////////////////////////////////////////////////////////////////
class CDS_QMTable : public CTable
{
	DEFINE_DATA( CDS_QMTable )

protected:
	CDS_QMTable( CDataBase *pDataBase, LPCTSTR ID ) : CTable( pDataBase, ID ) {}

public:
	static bool WriteShortInfos( OUTSTREAM outf, std::map<__time32_t, uint16_t> *pMap );
	bool WriteShortInfos( OUTSTREAM outf );
	static bool ReadShortInfos( INPSTREAM  inpf,std::map<__time32_t, uint16_t> *pMap );

	// Function that verify the existence of the object (Quick measurement)
	// in the table by comparing the date time and the Name.
	// -------------------------------------------------------------------------------------
	// In  : CDS_HydroModX *pQM				pointer to the object to verify
	//		 IDPTR *idptr					an uninitialize pointer
	// -------------------------------------------------------------------------------------
	// Out : bool bReturn					Return function that tells the object were found
	//		 IDPTR *idptr					Complete the idptr pointer of the found object in
	//										the table
	// -------------------------------------------------------------------------------------
	bool VerifyQMExist( CDS_HydroModX *pQM, IDPTR *idptr );

	// Scan all measurement to find associated (same AssociatedHMXID) 
	CDS_HydroModX *GetAssociatedHM (CDS_HydroModX *pQM, int beforePos = 1000);		//Check existance BEFORE position in table

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

#ifndef TACBX
public:
	// Allow to check what is the upper and lower limit for date and time of all HydroMod.
	void GetLowerUpperDataTime( COleDateTime& clLowerDateTime, COleDateTime& clUpperDateTime );
private:
	void _GetLowerUpperDataTime( CTable *pclQMTable, COleDateTime& clLowerDateTime, COleDateTime& clUpperDateTime );
#endif
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CHMXPipe
//
// Remarks: why CPipe is no more implemented in CDS_HydroModX? Because when 
// importing *.TSC into TASelect we need some informations about pipes and terminal
// units. It's easier to share a common class between CDS_HydroMod (TASelect) and
// CDS_HydroModX (TAScope).
///////////////////////////////////////////////////////////////////////////////////
class CDS_HydroModX;
class CHMXPipe 
{
public:
	enum ePipeLoc
	{
		epipeOnDistributionSupply,
		epipeOnCircuit,
		epipeOnDistributionReturn,		// Version 3.
		epipeOnSecondaryCircuit,		// 2017-01-17: Version 4.
		epipeLast,
	};
	
	CHMXPipe( CWaterChar *pWC, ePipeLoc PipeLoc );
	CHMXPipe( CDS_HydroModX *pHMXParent, ePipeLoc PipeLoc );
	virtual ~CHMXPipe();
	
public:
	void SetPipeLoc( ePipeLoc PipeLoc );
	void SetPipeID( _string ID );
	void SetRLin( double rlin );
	void SetKvFix( double KvFix );
	void SetLength( double Length );
	void SetFlow( double dflow );
	void SetpParentHMX( CDS_HydroModX *pHMXParent );

	ePipeLoc GetPipeLoc() { return m_PipeLoc; }
	
	CDB_Pipe *GetpPipe();
	LPCTSTR GetPipeID() { return m_tcPipeID; }
	// IDPTR GetPipeIDPtr();
	
	double GetRLin() { return m_RLin; }
	double GetKvFix() { return m_KvFix; }
	double GetLength() { return m_Length; }
	double GetFlow() { return m_dQ; }
	CDS_HydroModX *GetpParentHMX() { return m_pHMXParent; }

	double GetTotalDp();

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );
	void Copy( CHMXPipe *pTo );

protected:
	CWaterChar *m_pWC;
	CDS_HydroModX *m_pHMXParent;		// Circuit parent pointer
	ePipeLoc m_PipeLoc;
	TCHAR m_tcPipeID[_ID_LENGTH + 1];
	double m_RLin;						// Linear resistance, used with unknown pipe
	double m_KvFix;						// Kv of fixed device as tee, angle, ...
	double m_Length;					// Pipe length
	double m_dQ;						// Flow

private:
	enum eOwner
	{
		eMeasDataOwner,
		eHMXOwner,
		eUndefinedOwner
	};
	eOwner m_eOwner;								// Tell if class belongs to a CDS_HydroMod::CMeasData or to
													// a CDS_HydroModX.
	// Version 3.
	int	m_iHySelectExchangeDataSize;
	unsigned char *m_pHySelectExchangeData;			// This buffer is only use to exchange data between TA-Scope and HySelect.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CHMXTU
//
// Remarks: why CPipe is no more implemented in CDS_HydroModX? Because when 
// importing *.TSC into TASelect we need some informations about pipes and terminal
// units. It's easier to share a common class between CDS_HydroMod (TASelect) and
// CDS_HydroModX (TAScope).
///////////////////////////////////////////////////////////////////////////////////
class CHMXTU
{
public:
	CHMXTU( void );
	CHMXTU( CDS_HydroModX *pHMXParent );
	virtual ~CHMXTU();

	// Setter.
	void SetDp( double dDp );
	void SetDesignFlow( double dQ );
	void SetpParentHMX( CDS_HydroModX *pHMXParent );

	// Getter.
	double GetDp( void ) const { return( m_dDp ); }
	double GetDesignFlow( void ) const { return( m_dQ ); }
	CDS_HydroModX *GetpParentHMX() { return m_pHMXParent; }

	// 2016-09-22: We save now the complete structure to allow to well retrieve data input by user when
	//             importing back project in HySelect.
	void SaveTermUnitData( unsigned char *pucTermUnitData, int iBufferSize );
	int GetTermUnitDataSize( void ) { return m_iTermUnitDataSize; }
	unsigned char *GetTermUnitDataBuffer( void ) { return m_pTermUnitData; }

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );
	void Copy( CHMXTU *pTo );

protected:
	CDS_HydroModX *m_pHMXParent;			// Circuit parent pointer.
	double m_dDp;							// This variable is the one used by TA-Scope.
	double m_dQ;							// This variable is the one used by TA-Scope.

	// Version 2.
	int	m_iTermUnitDataSize;
	unsigned char *m_pTermUnitData;			// This buffer is only use to exchange data between TA-Scope and HySelect.

private:
	enum eOwner
	{
		eMeasDataOwner,
		eHMXOwner,
		eUndefinedOwner
	};
	eOwner						m_eOwner;				// Tell if class belongs to a CDS_HydroMod::CMeasData or to
														// a CDS_HydroModX.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_HydroModX
//
///////////////////////////////////////////////////////////////////////////////////
#ifdef TACBX
#include "TAMethod.h"
#endif

class CDS_HydroModX : public CTable
{
	DEFINE_DATA(CDS_HydroModX)
	
// Constructor and destructor.
protected:
	CDS_HydroModX(CDataBase* pDataBase, LPCTSTR ID);
	~CDS_HydroModX();
	//Nested class
public:
	
	enum eHMStatusFlags
	{
		sfBlackBox = 1,			// Module is a black box, TASCOPE cannot modify TU flow or Dp
		sfOrphan = 2			// Send from the PC as a orphan module, can't be moved
	};

	enum eQMtype
	{
		QMundef = 0x00,	
		QMDp	= 0x01,
		QMFlow	= 0x02,
		QMTemp	= 0x04,
		QMPower = 0x08,
		QMDpl	= 0x10,
		LastQMType = 0x20
	};

	enum eLocate
	{
		undef,
		InPrimary,
		InSecondary,
		InBypass,
		LastItem
	};

	enum eMode
	{
		TAmode,
		Kvmode,
		ALIENmode,
		LastMode
	};
	
	// Keep synchronized with CTAGenSensor::eSensorType
	enum eDTS
	{
		DTSUndef = -1,
		DTSFIRST,
		DTS2onDPS = DTSFIRST,
		DTS1onDPS,
		DTSonHH,
		DTSLastMeas=DTSonHH,
		DTSRef,
		InternalReference = DTSRef,
		LastDTS
	};
	
	enum eHMXBalType
	{
		NOBalancing,
		BV,						// Balancing valves
		BVCV,					// Combined balancing valves and control valves
		DPC,					// Dp controller
		PICV,					// Pressure independent control valve
		DPC_CV,					// Dp controller on a presettable (or with measuring points) control valve.
		DPC_BCV,				// Combined Dp controller, control and balancing valve.
		SmartControlValve,		// Smart control valve.
		SmartDpC,				// Smart differential pressure controller.
		LastHMXBalType
	};
	
	enum ReturnType		// TODO duplicated with HYDROMOD
	{
		Direct,
		Reverse,
		LastReturnType
	};

	enum eTABalMeasCompleted
	{
		emcNoMeas,
		emcAsCircuit,// When the measure was done on children circuit
		emcAsPV,	 // When the measure was done on the partner valve
		emcAsTAWManual,
		emcLastMeasCompleted
	};
	
	enum eTABalanceMethod
	{
		etabmNoBalancing,
		etabmTABalPlus,						//TA Balance Plus completed
		etabmTABalWireless,					//TA Balance Wireless completed
		etabmTABalPlusStarted,
		etabmTABalWirelessStarted,
		etabmForced,
		etabmLast
	};

	enum eTAMeasureMethod
	{
		etammQM_Dp,
		etammQM_Q,
		etammTAD,
		etammTAW_RefValve,
		etammTAW_OtherValve,
		etammComputerMethod,
		etammTroubleShooting,
		etammTrbl_ReqDp,
		etammTrbl_AvailDp_RefValve,
		etammTrbl_AvailDp_OtherValve,
		etammTrbl_AvailDp_IntermediateValve,
		etammVSP_Optimize,
		etammBAL_VerifyFlow,
	};

	// Must be the same as the one defined in the 'CDS_HydroMod' class.
	enum eUseProduct
	{
		Auto = -1,
		Never,
		Always,
		Last
	};

	class CMeasData
	{
	public: 
		CMeasData(CDS_HydroModX *pParent);
		virtual ~CMeasData(){};

		CDS_HydroModX::eLocate GetLocate()	{return m_Locate; }
		CDS_HydroModX::eTABalanceMethod GetBalanced() const { return m_eBalanced; }
		__time32_t GetMeasDateTime() const { return m_MeasDateTime; }
		__time32_t GetTABalDateTime() const { return m_TABalDateTime; }
		unsigned GetQMType() const { return m_QMType; }
		bool GetMeasFlowDone() const { return m_MeasFlowDone; }
		double GetMeasTemp(CDS_HydroModX::eDTS dts) const { if(dts <= CDS_HydroModX::DTSUndef || dts >=CDS_HydroModX::LastDTS) return -273.15; else return m_dMeasTemp[dts]; }
		double GetMeasDp() const { return m_dMeasDp; }
		double GetMeasDpL() const { return m_dMeasDpL; }
		double GetTaBalMeasDp_2() const { return m_dTaBalMeasDp_2; }
		double GetTaBalPipeDp() const { return m_dTaBalPipeDp; }
		double GetTaBalUnitDp() const { return m_dTaBalUnitDp; }
		CDS_HydroModX::eTABalMeasCompleted GetTaBalMeasCompleted() const { return m_eTaBalMeasCompleted; }
		CDS_HydroModX::eDTS GetUsedDTSensor(UINT pos) const { return m_UsedDTSensor[pos]; }
		CDS_HydroModX::eDTS GetWaterTempRef() const { return m_WaterTempRef; }

		void SetLocate(eLocate loc) { if( m_Locate == loc) return; m_Locate = loc; m_pParent->Modified(); }				
		void SetBalanced(CDS_HydroModX::eTABalanceMethod val) { if( m_eBalanced == val ) return; m_eBalanced = val; m_pParent->Modified(); }
		void SetMeasDateTime(__time32_t val) {  if( m_MeasDateTime == val ) return; m_MeasDateTime = val; m_pParent->Modified(); }
		void SetTABalDateTime(__time32_t val) {  if( m_TABalDateTime == val ) return; m_TABalDateTime = val; m_pParent->Modified(); }
		void SetQMType(unsigned val) {  if( m_QMType == val ) return; m_QMType = val; m_pParent->Modified(); }
		void SetMeasFlowDone(bool val) {  if( m_MeasFlowDone == val ) return; m_MeasFlowDone = val; m_pParent->Modified(); }
		void SetMeasTemp(CDS_HydroModX::eDTS dts, double val) {  if(  m_dMeasTemp[dts] == val ) return; m_dMeasTemp[dts] = val; m_pParent->Modified(); }
		void SetMeasDp(double val) {  if( m_dMeasDp == val ) return; m_dMeasDp = val; m_pParent->Modified(); }
		void SetMeasDpL(double val) {  if( m_dMeasDpL == val ) return; m_dMeasDpL = val; m_pParent->Modified(); }
		void SetTaBalMeasDp_2(double val) {  if( m_dTaBalMeasDp_2 == val ) return; m_dTaBalMeasDp_2 = val; m_pParent->Modified(); }
		void SetTaBalPipeDp(double val) {  if( m_dTaBalPipeDp == val ) return; m_dTaBalPipeDp = val; m_pParent->Modified(); }
		void SetTaBalUnitDp(double val) {  if( m_dTaBalUnitDp == val ) return; m_dTaBalUnitDp = val; m_pParent->Modified(); }
		void SetTaBalMeasCompleted(CDS_HydroModX::eTABalMeasCompleted val) {  if( m_eTaBalMeasCompleted == val ) return; m_eTaBalMeasCompleted = val; m_pParent->Modified(); }
		void SetUsedDTSensor(UINT pos, CDS_HydroModX::eDTS val) {  if( m_UsedDTSensor[pos] == val ) return; m_UsedDTSensor[pos] = val; m_pParent->Modified(); }
		void SetWaterTempRef(CDS_HydroModX::eDTS val) {  if( m_WaterTempRef == val ) return; m_WaterTempRef = val; m_pParent->Modified(); }

		void Write( OUTSTREAM outf );
		bool Read(INPSTREAM inpf);
		void Copy( CDS_HydroModX::CMeasData *pTo );
		void ClearData();
	
	protected:
		CDS_HydroModX *m_pParent;			// Circuit parent pointer
		eLocate m_Locate;					// Define where does data comes from

		CDS_HydroModX::eTABalanceMethod	m_eBalanced;	// Set with balancing type after a TABalance
		__time32_t m_MeasDateTime;						// Measuring Date/Time
		__time32_t m_TABalDateTime;						// TABalance Date/Time
		unsigned m_QMType;								// Quick measurement type
		bool m_MeasFlowDone;							// True is measuring flow/power as been done
		double m_dMeasTemp[LastDTS];					// Measured temperature
		double m_dMeasDp;								// Measured Dp used when valve is not defined
		double m_dMeasDpL;								// Measured Dp Load
		double m_dTaBalMeasDp_2;						// Measured Dp on PV
		double m_dTaBalPipeDp;							// Computed Dp of distribution pipe
		double m_dTaBalUnitDp;							// Computed Dp of Unit; be carefull for the last unit of a module
		// Unit Dp and Pipe Dp are mixed together.
		// TA Balance completion flag
		eTABalMeasCompleted m_eTaBalMeasCompleted;	// Set when measuring step 1 & 2 are done 
		CDS_HydroModX::eDTS m_UsedDTSensor[2];		// Temp sensor used to measure a DT
		CDS_HydroModX::eDTS m_WaterTempRef;			// Temp sensor used as temperature reference
	};

	class CBase
	{
	public:
		CBase(CDS_HydroModX *pParent);
		virtual ~CBase();

	public:
		virtual eLocate	GetLocate() {return InPrimary; }
		virtual CvLocation	GetCvLocate() { return CvLocNone; }
		virtual void	SetValveID(_string ID)				{if(!IDcmp(ID.c_str(),m_ValveIDPtr.ID)) return;m_ValveIDPtr.SetID(ID);m_pParent->Modified(); }
		virtual void	SetDesignFlow(double dVal)			{if( m_dDesignQ == dVal) return; m_dDesignQ = dVal;m_pParent->Modified(); }
		void			SetMeasDp(double dVal)				{if( m_dMeasDp == dVal) return;m_dMeasDp = dVal;m_pParent->Modified(); }

		virtual double	GetDesignFlow()						{return m_dDesignQ; }
		virtual double 	GetFlowRatio()						{if(GetDesignFlow()>0) return(GetMeasFlow()/GetDesignFlow()*100);return 0; }
		// Openings are maximized to the valve max opening when reading  
		virtual double	GetDesignOpening();
		virtual double	GetCurOpening();
		virtual void	SetDesignOpening(double dOpening);
		virtual void	SetCurOpening(double dCurOpening)	{if( m_dCurOpening == dCurOpening) return; m_dCurOpening = max(0,dCurOpening);m_pParent->Modified(); }
		virtual eMode	GetMode()							{return TAmode; }

		CDS_HydroModX *GetpParentHMX() { return m_pParent; }
		LPCTSTR GetValveName();
		IDPTR GetValveIDPtr();
		double GetMeasDp() { return m_dMeasDp; }

		// Measuring Date/time
		void SetMeasDateTime(__time32_t dt){if( m_MeasDateTime == dt) return; m_MeasDateTime = dt;m_pParent->Modified(); }
		__time32_t GetMeasDateTime() { return ( m_MeasDateTime); }

		virtual double GetDpFullOpening( double dQ );
		virtual double GetOpening( double dQ, double dDp );
		virtual double GetFlow( double dOpening, double dDp, bool *pbVisCorActing = NULL );
		virtual double GetMeasFlow( bool *pbVisCorActing = NULL ) { return GetFlow( GetCurOpening(), GetMeasDp(), pbVisCorActing); }

		bool	IsKvSignalEquipped();

		// TA Balance function
		void SetTABalOpening_1( double dVal ) { if( m_dTaBalOpening_1 == dVal ) return; m_dTaBalOpening_1 = dVal; m_pParent->Modified(); }	
		void SetTABalMeasDp_1( double dVal ) { if( m_dTaBalMeasDp_1 == dVal ) return; m_dTaBalMeasDp_1 = dVal; m_pParent->Modified(); }	
		void SetTABalResultOpening( double dVal ) { if( m_dTaBalOpeningResult == dVal ) return; m_dTaBalOpeningResult = dVal; m_pParent->Modified(); }
		void SetTABalWRatioFactor( double dVal ) { if( m_dTaBalWRatioFactor == dVal ) return; m_dTaBalWRatioFactor = dVal; m_pParent->Modified(); }

		double GetTABalOpening_1() { return m_dTaBalOpening_1; }
		double GetTABalMeasDp_1() { return m_dTaBalMeasDp_1; }
		double GetTABalResultOpening() { return m_dTaBalOpeningResult; }
		double GetTABalWRatioFactor() { return m_dTaBalWRatioFactor; }

		double GetTABalFlow_1() { return GetFlow( GetTABalOpening_1(), GetTABalMeasDp_1() ); }

		eUseProduct GetUseProduct() { return m_eUseProduct; }
		void SetUseProduct( eUseProduct UseProduct ) { if( m_eUseProduct == UseProduct ) return; m_eUseProduct = UseProduct; m_pParent->Modified(); }
		
		// Functions used into TA Wireless to manage modification of Qd, Valve and compute setting with respect of Dpd
		// GetDesignDp() compute design dp based on Qd, Valve and presetting
		// bVerifyIndex == true: verify if the valve is index, if yes return Dpmin for index position
		double GetDesignDp( bool bVerifyIndex );
	
		virtual void Write( OUTSTREAM outf );
		virtual bool Read( INPSTREAM inpf );
		virtual void Copy( CDS_HydroModX::CBase *pTo );

		// 2016-09-23: We save now the additional data to allow to well retrieve data input by user when
		//             importing back project in HySelect.
		void SaveHySelectExchangeData( unsigned char *pHySelectExchangeData, int iBufferSize );
		int GetHySelectExchangeDataSize( void ) { return m_iHySelectExchangeDataSize; }
		unsigned char *GetHySelectExchangeDataBuffer( void ) { return m_pHySelectExchangeData; }

	protected:
		CDS_HydroModX *m_pParent;			// Circuit parent pointer
		IDPTR m_ValveIDPtr;					// TADB Valve IDPTR 
		double m_dDesignQ;					// Design flow
		double m_dDesignOpening;			// Design opening
		double m_dCurOpening;				// Current Opening
		double m_dMeasDp;					// Measured Dp
		__time32_t m_MeasDateTime;			// Measuring Date/Time

		// TA Balance Stage 1
		double m_dTaBalOpening_1;			// Current Valve Opening
		double m_dTaBalMeasDp_1;			// Measured Dp

		// TA Balance Stage 2
		// TA Balance Results
		double m_dTaBalOpeningResult;
		double m_dTaBalWRatioFactor;		// Used to store the flow ratio factor Q%i/Q%n

		eUseProduct m_eUseProduct;			// Auto, never, Always.

	private:
		// Version 2.
		int	m_iHySelectExchangeDataSize;
		unsigned char *m_pHySelectExchangeData;			// This buffer is only use to exchange data between TA-Scope and HySelect.
	};
	
	class CDpC : public CBase
	{
	public:
		CDpC( CDS_HydroModX *pParent, eDpStab StabOn = DpStabOnBranch) ;
		virtual ~CDpC()	{}

	public:
		void SetStabilizedOn( eDpStab stab ) { if( m_StabilizeOn == stab ) { return; } m_StabilizeOn = stab; m_pParent->Modified(); }
		void SetMvLoc( eMvLoc mvloc ) { if( m_MvLoc == mvloc) { return; } m_MvLoc = mvloc; m_pParent->Modified(); }

		eDpStab GetStabilizedOn() { return m_StabilizeOn; }
		eMvLoc GetMvLoc() { return m_MvLoc; }

		// Overrides 'CDS_HydroModX::CBase' public methods.
		virtual void Write( OUTSTREAM outf );
		virtual bool Read( INPSTREAM inpf);
		virtual void Copy( CDS_HydroModX::CBase *pTo );

	protected:
		eDpStab m_StabilizeOn;
		eMvLoc m_MvLoc;
	};

	class CBV  : public CBase
	{
	public:
		CBV(CDS_HydroModX *pParent, eMode mode=TAmode);
		~CBV() {}

	public:
		virtual eLocate GetLocate() { return m_Locate; }
		double GetKv() { return m_dKv; }
		
		double GetFlow( double dOpening, double dDp, bool *pbVisCorActing = NULL );

		eMode GetMode() { return m_Mode; }
		
		void SetMode( eMode mode ) { if( m_Mode == mode) return; m_Mode = mode; m_pParent->Modified(); }
		void SetValveID( _string ID, eMode mode = TAmode ) { CBase::SetValveID( ID ); m_Mode = mode; }
		void SetKv( double dVal, eMode mode = Kvmode ) { if( m_dKv == dVal ) return; m_dKv = dVal; m_Mode = mode; m_pParent->Modified(); }
		void SetLocate( eLocate loc ) { if( m_Locate == loc ) return; m_Locate = loc; m_pParent->Modified(); }

		// Overrides 'CDS_HydroModX::CBase' public methods.
		virtual void Write( OUTSTREAM outf );
		virtual bool Read( INPSTREAM inpf );
		virtual void Copy( CDS_HydroModX::CBase *pTo );

	protected:
		eLocate m_Locate;				// Define where is located the balancing valve
		double m_dKv;
		eMode m_Mode;					// TA valve IDPtr exist, Kv Mode only Kv exist
	};

	class CCv : public CBase
	{
	public:
		CCv( CDS_HydroModX *pParent );
		virtual ~CCv() {}

		virtual CvLocation GetCvLocate() { return m_eCvLocate; }
		void SetCvLocate( CvLocation eCvLocate ) { if( m_eCvLocate == eCvLocate ) return; m_eCvLocate = eCvLocate; m_pParent->Modified(); }

		// Overrides 'CDS_HydroModX::CBase' public methods.
		virtual void Write( OUTSTREAM outf );
		virtual bool Read( INPSTREAM inpf );
		virtual void Copy( CDS_HydroModX::CBase *pTo );

	private:
		CvLocation m_eCvLocate;				// Define where is located the control valve.
	};

	class CShutoffValve : public CBase
	{
	public:
		CShutoffValve( CDS_HydroModX *pParent );
		virtual ~CShutoffValve() {}

		// Overrides 'CDS_HydroModX::CBase' public methods.
		virtual void Write( OUTSTREAM outf );
		virtual bool Read( INPSTREAM inpf );
		virtual void Copy( CDS_HydroModX::CBase *pTo );
	};

	// To complete !!!
	class CSmartControlValve : public CBase
	{
	public:
		CSmartControlValve( CDS_HydroModX *pParent );
		virtual ~CSmartControlValve() {}

		// Overrides 'CDS_HydroModX::CBase' public methods.
		virtual void Write( OUTSTREAM outf );
		virtual bool Read( INPSTREAM inpf );
		virtual void Copy( CDS_HydroModX::CBase *pTo );
	};

	// To complete !!!
	class CSmartDpC : public CBase
	{
	public:
		CSmartDpC( CDS_HydroModX *pParent );
		virtual ~CSmartDpC() {}

		// Overrides 'CDS_HydroModX::CBase' public methods.
		virtual void Write( OUTSTREAM outf );
		virtual bool Read( INPSTREAM inpf );
		virtual void Copy( CDS_HydroModX::CBase *pTo );
	};

// Variable declaration
protected:
	_string							m_HMName;				// HydroMod Name
	_string							m_Description;			// Extended circuit description
	UINT							m_Pos;					// Circuit position in his parent
	IDPTR							m_WaterCharIDptr;		// Water characteristic IDPTR into WC_TAB
	IDPTR							m_SchemeIDPtr;			// ID of selected scheme
	_string							m_AssociatedHMXID;	// IDPTR of associated circuit, useful for QM on two sensors
	ReturnType						m_eReturnType;			// Direct or reverse return.
	wchar_t							m_HMID[ _ID_LENGTH + 1 ];	// Cross reference to the Hydromod circuit
	unsigned						m_Uid;					// needed to validate cross reference with HM circuit
	bool							m_bIndexCircuit;		// True if this circuit is the index circuit
	bool							m_bModule;				// True if this circuit is a module
	double							m_dTABalValMinDp;		// TA Balance min DP used for balancing operation
	unsigned char					m_ModuleStatus;			// set of flag that describe the module status see eHMStatusFlags	
	double 							m_dDesignFlow;			// In case of frozen module without balancing device
	double 							m_dDesignFlow_Sec;		// Secondary flow (2wInj)
	double 							m_dTempSecondary;		// Secondary temperature  (2wInj)
	double 							m_dTempReturn;			// Return temperature (2wInj)
	short	 						m_usZeroPointOffsetRaw;	// Keep a trace of raw offset	 
	_string m_dQMNote;										// Add a cell note in Sheet QuickMeasurement

	typedef	std::map<eLocate,CBV>	mapBV;
	typedef	std::pair<eLocate,CBV>	pairBV;
	mapBV m_mapBV;
	CDpC *m_pDpC;
	CCv *m_pCv;
	CShutoffValve *m_pShutoffValveSupply;
	CShutoffValve *m_pShutoffValveReturn;
	CSmartControlValve *m_pSmartControlValve;
	CSmartDpC *m_pSmartDpC;
	CHMXPipe *m_pPipe[CHMXPipe::epipeLast];
	CHMXTU *m_pTU;
	typedef	std::map<eLocate,CMeasData>	mapMeasData;
	typedef	std::pair<eLocate,CMeasData> pairMeasData;
	mapMeasData						m_mapMeasData;

	// Working variable not saved	
	bool							m_bReadyForBalancing;	//	True if all children modules are balanced
	bool							m_bNonMeasurablePoint;	//	True if module has a non measurable valve
#ifdef TACBX
	CComputerMethod	*m_pCM;	
	CTABalWireless *m_pTAW;
#else
	void		*m_pCM;
	void 		*m_pTAW;
#endif
															// and if it's parent is not already balanced
public:
	// Non Located (Primary/Secondary) functions
	bool BuildCircuit( CDB_CircSchemeCateg *pSchCat, eHMXBalType balwith, eDpStab DpStab, eMvLoc MvLoc, bool bSecondaryBvNeeded, bool bShutoffValveNeeded, bool bCleanValvesAndData );
	bool BuildCircuit( CDB_CircuitScheme *pSch, bool bSecondaryBvNeeded, bool bShutoffValveNeeded, bool bCleanValvesAndData );
	bool SwitchToKvMode( double dKv );
	bool SwitchToAlienMode();
	
	unsigned int GetHMXStatus() { return m_ModuleStatus; }
	bool HasHMXStatus( unsigned int status ){ return ( ( m_ModuleStatus & status ) ? true : false ); }
	void SetHMXStatus( unsigned int status ){ m_ModuleStatus = status; }

	//Set/Get Unic identifier.
	unsigned GetUid() { return m_Uid; }
	void SetUid( unsigned uid ) { if( uid !=m_Uid ) { m_Uid=uid; Modified(); } }

	// Circuit position.
	void SetPosition( UINT uiPos ) { if( m_Pos == uiPos ) return; m_Pos = uiPos; Modified(); }
	UINT GetPosition() { return m_Pos; }

	// Set ZeroPointOffset Raw.
	void SetZeroPointOffsetRaw( short zpor ) { if( m_usZeroPointOffsetRaw == zpor ) return; m_usZeroPointOffsetRaw = zpor; Modified(); } 
	short GetZeroPointOffsetRaw() { return m_usZeroPointOffsetRaw; }

	// Return the reference valve used into TAWireless method.
	CDS_HydroModX *GetpRefValve();

	void SetTABalValMinDp( double tabalvalvmindp ) { if( m_dTABalValMinDp == tabalvalvmindp ) return; m_dTABalValMinDp = tabalvalvmindp; Modified(); }
	double GetTABalValMinDp() { return m_dTABalValMinDp; }

	// Circuit name.
	void SetHMName( _string HMName ) { if( m_HMName == HMName ) return; m_HMName = HMName; Modified(); }
	_string *GetHMName () { return &m_HMName; }

	// Circuit description.
	void SetDescription( _string description ) { if( m_Description == description ) return; m_Description = description; Modified(); }
	_string *GetDescription() { return &m_Description; }

	// fill map with ordered children hydromod by position
	// return 0 if it's not a module, otherwise number of children ordered
	unsigned short GetOrdered( std::map<int, CDS_HydroModX*> *pMap );

	// HydroMod ID
#ifdef TACBX
	void SetHMID(wchar_t *pID){if(!IDcmp( m_HMID, pID)) return;wcsncpy( m_HMID,pID, SIZEOFINTCHAR( m_HMID ) - 1 );Modified(); }
#else
	void SetHMID(wchar_t *pID){if(!IDcmp( m_HMID, pID)) return;wcsncpy_s( m_HMID, SIZEOFINTCHAR( m_HMID),pID, SIZEOFINTCHAR( m_HMID ) - 1 );Modified(); }
#endif	
	wchar_t *GetHMID() { return m_HMID; }
	
	// Index flag.
	bool IsCircuitIndex() { return m_bIndexCircuit; }
	void SetFlagIndexCircuit( bool bFlag ) { if( m_bIndexCircuit == bFlag ) return; m_bIndexCircuit = bFlag; Modified(); }

	// SetFlagModule to true is the hydromod is a module.
	void SetFlagModule( bool bFlag ) { if( m_bModule == bFlag ) return; m_bModule = bFlag; Modified(); }

	bool IsaModule() { return m_bModule; }
	
	// Circuit Water Characteristic IDPtr; be carefull this function return Water Char IDPTR but don't take into
	// account the circuit temperature
	const IDPTR	&GetWaterCharIDPtr();
	void SetWaterCharIDPtr(IDPTR &IDPtr){if( m_WaterCharIDptr == IDPtr) return; m_WaterCharIDptr=IDPtr;Modified(); }

	// BalGPS method Return if a module could be balanced.
	bool IsReadyForBalancing() { return m_bReadyForBalancing; }

	// BalGPS method analyze the HydromodX table to identify module that could be balanced.
	// Used by CDS_HMXTable::SetFlagReadyForBalancing() when we diplay balancing marker.
	void SetReadyForBalancing( bool bSet ) { m_bReadyForBalancing = bSet; }
	void ResetReadyForBalancing( bool bAlsoChildren = true );

	// Used to mark that this point as not to be balanced by TAD or TAW.
	void SetNonMeasurablePoint( bool bSet ) { m_bNonMeasurablePoint = bSet; }
	bool isNonMeasurablePoint() {return m_bNonMeasurablePoint; }

	bool GetBalancingIndex( double *pdBalIndex );

	// Located function
	// Changing balancing TA product could impact on the balancing type
	bool ChangeSelectedValve(CDB_TAProduct *pTap, eLocate locate = InPrimary);
	// Used to change a circuit to fit a new valve
	// Try to match the new circuit with the old one
	bool UpdateCircuitForNewValve(CDB_TAProduct *pTap, eLocate locate = InPrimary);

	// Fill Waterchar pointer with Water Characteristic data corrected with circuit temperature.
	// In: CWaterChar *, could be NULL in this case memory allocation is done into the function itself.
	//		Destruction should be done by the caller
	// Out: return CWaterChar *
	CWaterChar *GetpWaterChar(CWaterChar *pWC, eLocate locate = InPrimary);
	// QM type
	void SetQMType(eQMtype QMType, eLocate locate = InPrimary);
	unsigned GetQMType( eLocate locate = InPrimary);
	bool IsQMType (eQMtype QMType, eLocate locate = InPrimary);

	// Quick access to the measured values
	_string GetQMstrDp(bool bWithUnit=false, eLocate locate = InPrimary);
	_string GetQMstrDpL( bool bWithUnit = false, eLocate locate = InPrimary );
	_string GetQMstrQ(bool bWithUnit=false,bool *pbVisCorActing = NULL, eLocate locate = InPrimary);
	_string GetQMstrT(int i,bool bWithUnit=false, eLocate locate = InPrimary);
	_string GetQMstrDT(bool bWithUnit=false, eLocate locate = InPrimary);
	_string GetQMstrPower(bool bWithUnit=false, eLocate locate = InPrimary);
	_string GetQMstrOpening( eLocate locate = InPrimary);
	_string GetQMstrKv(eLocate loc,bool bWithUnit=false);

	double GetCurOpening(eLocate locate = InPrimary);
	
	//GetpMeasValve() return pointer on measuring device
	CDS_HydroModX::CBase *GetpMeasValve(eLocate locate = InPrimary);			
	
	// Set/Get Design flow in all existing device in a circuit, WARNING what about bypass valve
	void SetDesignFlow( double dDesignFlow );
	double GetDesignFlow(eLocate locate = InPrimary);
	
	double ComputePrimaryFlow(double dQsec, double dTprimary);	

	void SetFrozenDesignFlow(double d) { m_dDesignFlow = d; }
	void SetFrozenDesignFlow_Sec(double d) { m_dDesignFlow_Sec = d; }
	
	// Temperature
	double GetSupplyTemperature();

	void SetTempSecondary(double Temp) { m_dTempSecondary = Temp; }
	double GetTempSecondary() { return m_dTempSecondary; }
	
	void SetTempReturn(double Temp) { m_dTempReturn = Temp; }
	double GetTempReturn() { return m_dTempReturn; }
	
	// Return CBase* of measuring device
	double GetMeasFlow(bool *pbVisCorActing = NULL, eLocate locate = InPrimary);

	//Set/Get used temp sensor for DT
	eDTS GetDTSensor(short i, eLocate locate = InPrimary);
	void SetDTSensor(short i,eDTS TSens, eLocate locate = InPrimary);

	// Measurement flag true when flow or power measurement done, false otherwise
	void SetFlagMeasFlow(bool bFlag, eLocate locate = InPrimary);
	bool IsMeasFlowDone(eLocate locate = InPrimary);
	
	//--------------
	// TABalance
	//--------------
	// Balanced flag is true when the balancing operation has been completed.
	// User can also force the balancing flag!
	void ResetTABalanceMode( eTABalanceMethod TABalMethod, bool bReset = false );
	void SetTABalanceMode( eTABalanceMethod TABalMethod );
	eTABalanceMethod GetTABalanceMode( );
	bool IsBalanced( ); 
	// Access to the TABalance Measured Dp step 2 (valve closed)
	double GetTABalMeasDp_2( eLocate locate = InPrimary );
	void SetTABalMeasDp_2(double dVal,  eLocate locate = InPrimary );
	// TABalance measures has been completed 
	// In TABal+ method set to true only if TABal Measures 1 & 2 exist
	bool IsTABalMeasCompleted(eLocate locate = InPrimary);
	void SetTABalMeasCompleted(eTABalMeasCompleted BalMeasCompleted, __time32_t time=0, eLocate locate = InPrimary);
	eTABalMeasCompleted	GetTABalMeasCompleted(eLocate locate = InPrimary);

#ifdef TACBX	
	// In BV mode:
	// Call TAWireless method as soon as two measurement step as been done
	// 1- Opening & Flow
	// 2- Dp valve shut
	// This method use values stored into TABalMeasDp1 & 2
	//
	// In VENTURI mode
	// Call functions direct from CTABalWireless
	//
	CTABalWireless::eCTABALWrlssRtc TAWirelessMethod(CDS_HydroModX *pHMXtobalance, CTABalWireless::eCTABALWrlssMode tabalwrlssMode = CTABalWireless::tblwrlssBV);
	CTABalWireless * GetpTAW() { return m_pTAW; }
#endif	
	// Must be called at the end of TAWireless Method to free allocated memory
	void RelaxTAWirelessMethod();
	
	// Scan all children, return the latest (position) circuit
	// that is not yet balanced 
	CDS_HydroModX *GetNextValveToBalance();
	
	// Scan all children 
	// Reset flag m_eTaBalMeasCompleted when TABal was done on a PV
	// useful when the HM is balanced inside his parent
	void ResetTaBalMeasforChildrenModule();

	double GetTaBalPipeDp(eLocate locate = InPrimary);				
	double GetTaBalUnitDp(eLocate locate = InPrimary);			
	void SetTaBalPipeDp(double m_dDp, eLocate locate = InPrimary);				
	void SetTaBalUnitDp(double m_dDp, eLocate locate = InPrimary);			

	// Verify if all measurements are done for launching TABalance method
	// if bAll = false return true if one valve exist with measurement
	bool IsTABalMeasDone(bool bAll=true);

	// Computer Method
	// During measurement process, call this function to compute an approached value 
	// of needed opening for the requested flow
	// This function use Measured Dp and Measured Flow.
	// First call dReqFlow is not needed, only bFirst should be true
	// other call dReqFlow is filled with user requested flow, dComputedOpening is filled with computed value.
	// Return Code: see eCCMRetCode enum
#ifdef TACBX
	CComputerMethod::eCCMRetCode ComputerMethod(double dReqFlow, double &dComputedOpening, bool bfirst=false, eLocate locate = InPrimary);
#endif
	// Called at the end of Computer Method to free allocated memory
	void RelaxComputerMethod();

	// Reset Measurements.
	void ResetMeasurements( bool bResetDT, eLocate locate = InPrimary );
	
	// Balancing type
	eHMXBalType GetHMXBalType( );
	
	// Changing balancing TA product could impact on the balancing type
	CDB_TAProduct *GetpTAP( eLocate locate = InPrimary );
	CDB_AlienRegValve *GetpALIENP(eLocate locate = InPrimary);
	CDB_Thing *GetpAnyProduct(eLocate locate = InPrimary);
	
	// Return true when measurable points exist on TA product or when valve is defined in Kv mode
	// when eMeasData is defined test valve's measuring properties 
	bool MeasurableDeviceExist(CDB_TAProduct::eMeasurableData eMeasData = CDB_TAProduct::emdNone, eLocate locate = InPrimary);

	// Return true when characteristics are OK for the selected measuring method
	bool CircuitFitForMethod(eTAMeasureMethod eTaMeasMethod, eLocate locate = InPrimary);

	// Define sensor used as temperature reference
	void SetWaterTempRef(eDTS TRef, eLocate locate = InPrimary );
	eDTS GetWaterTempRef( eLocate locate = InPrimary );
	// Circuit temperatures
	void SetWaterTemp(int i, double dTemp, eLocate locate = InPrimary );
	double GetWaterTemp(int i, eLocate locate = InPrimary );
	double GetQMDT( eLocate locate = InPrimary );

	// Measured DP (only used for a fast dp measure without valve)
	void SetMeasDp(double dQMMeas, eLocate locate = InPrimary );
	double GetMeasDp( eLocate locate = InPrimary );
	// Measured DPL (only used for DpC circuit)
	void SetMeasDpL(double dQMMeas, eLocate locate = InPrimary );
	double GetMeasDpL( eLocate locate = InPrimary );
	
	double GetMeasPower( eLocate locate = InPrimary );

	// Measuring Date/time
	void SetMeasDateTime(__time32_t dt, eLocate locate = InPrimary );
	__time32_t GetMeasDateTime( eLocate locate = InPrimary );
	void ResetMeasDateTime( bool bAlsoChildren = true  );
	bool MeasExist();
	
	// TA Balance Date/time
	void SetTABalDateTime(__time32_t dt,  eLocate locate = InPrimary );
	__time32_t GetTABalDateTime( eLocate locate = InPrimary );

	// Balancing function
	// TABalance() return true for success.
	// This function will balance all included circuit in (this) module.
	// Prerequise all BV measurments (step 1 and 2) should be done before calling it
	bool TABalance();
	// All valves included into the module have measuring possibilities
	// Usefull for TAWireless 
	bool ContainsOnlyMeasurableDevices();
	
	// CMeasData Access
	CMeasData *AddMeasData(eLocate loc);
	bool RemoveMeasData(eLocate loc);
	CMeasData *GetpMeasData(eLocate loc);

	// BV access 
	// Add a balancing valve at a specific position (eLocate), return BV pointer in case of success null otherwise
	// only one Bv could exist at a certain position
	CBV *AddBv(eLocate loc, eMode mode = TAmode);
	// Remove balancing valve from a position
	bool RemoveBv(eLocate loc);
	// return localized balancing valve pointer if exist, null pointer otherwise  
	CBV *GetpBV(eLocate loc);

	CDpC *AddDpC(eDpStab StabOn); 
	bool RemoveDpC();
	// return DpC pointer if exist, null pointer otherwise  
	CDpC *GetpDpC();

	CCv *AddCv( CvLocation eCvLocate ); 
	bool RemoveCv();
	// return DpC pointer if exist, null pointer otherwise  
	CCv *GetpCv();

	CShutoffValve *AddShutoffValve( ShutoffValveLoc eShutoffValveLocation ); 
	bool RemoveShutoffValve( ShutoffValveLoc eShutoffValveLocation );
	// return ShutoffValve pointer if exist, null pointer otherwise.
	CShutoffValve *GetpShutoffValve( ShutoffValveLoc eShutoffValveLocation );

	CSmartControlValve *AddSmartControlValve(); 
	bool RemoveSmartControlValve();
	// return smart control valve pointer if exist, null pointer otherwise  
	CSmartControlValve *GetpSmartControlValve();

	CSmartDpC *AddSmartDpC(); 
	bool RemoveSmartDpC();
	// return smart differential pressure controller pointer if exist, null pointer otherwise  
	CSmartDpC *GetpSmartDpC();
	
	CHMXPipe *AddPipe( CHMXPipe::ePipeLoc PipeLoc );
	bool RemovePipe( CHMXPipe::ePipeLoc PipeLoc );
	// return Pipe pointer if exist, null pointer otherwise  
	CHMXPipe *GetpPipe( CHMXPipe::ePipeLoc PipeLoc );

	// Terminal unit.
	CHMXTU *AddTU();
	bool RemoveTU();
	CHMXTU *GetpTU();

	IDPTR GetSchemeIDPtr();
	void SetSchemeIDPtr( IDPTR val );

	CDB_CircuitScheme *GetpSch() { return ( (CDB_CircuitScheme *)GetSchemeIDPtr().MP ); }
	CDB_CircSchemeCateg *GetpSchcat();

	CDS_HydroModX::ReturnType GetReturnType() const { return m_eReturnType; }
	void SetReturnType(CDS_HydroModX::ReturnType val) { m_eReturnType = val; }
	CDS_HydroModX::ReturnType GetChildrenReturnType();

	// Scan all selected products and return a 'OR' flag of Hidden/Deleted
	void CheckValveStatus( bool *pbHidden, bool *pbDeleted );
	void ReadShortInfos( INPSTREAM inpf );

	// Access to associated HydroModX
	_string GetAssociatedHMXID();
	void SetAssociatedHMXID(_string val) { m_AssociatedHMXID = val; }

	//Used to clear parameters after a copy (measurement, uid, HMID, ...
	void ClearUnwantedCopiedParameters();
	void CopyWithChildrens(CDS_HydroModX *pTarget, bool bEraseData = false );

	const IDPTR &GetPropertyIDPtr( CDataBase *pDB, IDPTR &idptr );

	// Note management
	void SetQMNote(_string strVal) { if(m_dQMNote == strVal) return; m_dQMNote = strVal; }
	_string GetQMNote() { return m_dQMNote; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDS_LogDataX
//
///////////////////////////////////////////////////////////////////////////////////
#define SENSORIDLENGTH	8
class CDS_LogDataX : public CLog
{
	DEFINE_DATA(CDS_LogDataX)
	
// Constructor and destructor.
protected:
	CDS_LogDataX(CDataBase* pDataBase, LPCTSTR ID);
	~CDS_LogDataX();

	//Nested class
public:
	enum LogType			// Keep same order that enum_LogMode 
	{
		LOGTYPE_UNDEF = -1,
		LOGTYPE_DP,				// Dp
		LOGTYPE_FLOW,			// Q , Dp 
		LOGTYPE_TEMP,			// T1 or T2
		LOGTYPE_DPTEMP,			// Dp, T
		LOGTYPE_FLOWTEMP,		// Q, T, Dp
		LOGTYPE_DT,				// DT, T1, T2
		LOGTYPE_POWER,			// P , Q, Dp, DT, T1, T2
		LOGTYPE_LAST
	};
////////////////////////////////////////////////////////////
//	Nested Class  -> CDS_LogDataX::CChan
////////////////////////////////////////////////////////////
	class CChan
	{
	friend class CDS_LogDataX;
	public:
		CChan(CDS_LogDataX *pP);
		virtual ~CChan(){};

	protected:
		eChannelType	m_ChanType;						// Type of entry
		unsigned		m_uKey;
		CDS_LogDataX	*m_pParent;
	public:
		void SetChanType(eChannelType ChanType)	{if( m_ChanType == ChanType) return; m_ChanType = ChanType;m_pParent->Modified(); }
		void SetKey(unsigned uKey)				{if( m_uKey == uKey) return; m_uKey = uKey;m_pParent->Modified(); }
		eChannelType GetChanType()				{return m_ChanType; }
		unsigned GetKey()						{return m_uKey; }
		
		virtual void SetEquation (CEquation &Eq){};
		virtual void AddRawData	(short usVal){}; 
		virtual bool GetRawData	(unsigned index, short &RawData){return false; }
		virtual bool GetData	(unsigned index, double &dVall){return false; }
		virtual unsigned GetDataArraySize() { return 0; }
		virtual bool GetMinValue(double *pdMinValue, unsigned *puIndex = NULL) = 0;
		virtual bool GetMaxValue(double *pdMaxValue, unsigned *puIndex = NULL) = 0;
		virtual bool GetMinMaxValues(double *pdMinValue, double *pdMaxValue, unsigned *puMinIndex = NULL, unsigned *puMaxIndex = NULL) = 0;
		virtual void Copy( CChan *pChan );
	protected:
		virtual void Write( OUTSTREAM outf );
		virtual bool Read(INPSTREAM inpf);
	};

////////////////////////////////////////////////////////////
//	Nested Class  -> CDS_LogDataX::CPhysChan
////////////////////////////////////////////////////////////
	class CPhysChan:public CChan
	{
	friend class CDS_LogDataX;
	public:
		CPhysChan(CDS_LogDataX *pP);
		~CPhysChan(){};

	protected:
		unsigned		m_uPos;							// Raw Data vector input position
		unsigned char	m_SensorNum;					// Sensor number into the sensor unit
		CEquation		m_Equation;						// Channel Equation
		short			m_sOffset;						// Offset value used to compensate 
		std::vector<short> m_RawData;					// Raw Datas
		std::vector<unsigned char> m_ValidationData;	// Validation code of Datas

	public:
		void SetPhysicalChan	( unsigned char ucSensorNum,eChannelType ChanType);
		void SetOffset	  (short offset)	{if( m_sOffset == offset) return; m_sOffset = offset;m_pParent->Modified(); }
		void SetDataArraySize(unsigned uPos){ m_uPos=uPos; }
		void SetSensorNumber(unsigned char SensorNbr){ m_SensorNum=SensorNbr; }
		// To escape to a possible reallocation set the vector size
		// Pay attention: 'SetSize' must be called only before data insertion. This function set size of vector at the beginning.
		//                'SetNewSize' can be called after data insertion. This function permits to change size of vector and set
		//                the 'm_uPos' variable.
		void SetSize	  (unsigned usSize){ m_RawData.resize(usSize,0);m_ValidationData.resize(usSize,0);m_pParent->Modified(); }
		void SetNewSize	  (unsigned usSize){ m_RawData.resize(usSize,0);m_ValidationData.resize(usSize,0);m_uPos = usSize;m_pParent->Modified(); }
		virtual void SetEquation (CEquation &Eq)	{ m_Equation = Eq; }

		eChannelType GetPhysicalChan ( unsigned char *pucSensorNum);
		short GetRawOffset() { return m_sOffset; }
		double GetOffset();
		CEquation *GetpEquation()	{return &m_Equation; }

		void AddRawData( short usVal );
		bool GetRawData( unsigned uIndex, int &RawData );
		void AddValidationData( unsigned char ucValue );
		bool GetValidationData( unsigned uIndex, unsigned char &ucValidationData );
		bool GetData( unsigned uIndex, double &dValue );
		unsigned GetDataArraySize() { return m_uPos; }
		bool ScanForErrors();
		virtual bool GetMinValue(double *pdMinValue, unsigned *puIndex = NULL);
		virtual bool GetMaxValue(double *pdMaxValue, unsigned *puIndex = NULL);
		virtual bool GetMinMaxValues(double *pdMinValue, double *pdMaxValue, unsigned *puMinIndex = NULL, unsigned *puMaxIndex = NULL);
		virtual void Copy( CPhysChan *pPhysChan );
	protected:
		virtual void Write( OUTSTREAM outf );
		virtual bool Read(INPSTREAM inpf);
	};

////////////////////////////////////////////////////////////
//	Nested Class  -> CDS_LogDataX::CLogChan
////////////////////////////////////////////////////////////
	class CLogChan:public CChan
	{
	friend class CDS_LogDataX;
	public:
		CLogChan(CDS_LogDataX *pP);
		~CLogChan(){};
	protected:	
		std::vector<unsigned> m_PhysChanNum;

	public:
		unsigned GetNumberOfPhysChan() { return (unsigned)m_PhysChanNum.size(); }
		void SetNumberOfPhysChan(unsigned uiNbr){ m_PhysChanNum.resize(uiNbr,0); }
		unsigned AttachPhysChan( unsigned uPhysChanNum );
		// Fill array with Physical Channel number used for a given Logging channel index
		// return number of Physical channels, or -1 in case of incorrect LogChannelNumber
		int	 GetPhysicalChanArray(std::vector<unsigned> *pAr);
		bool GetData		( unsigned uIndex, double &dVal );
		bool GetDeltaTData	( unsigned uIndex, double &dVal );
		bool GetPowerData	( unsigned uIndex, double &dVal );
		bool GetFlowData	( unsigned uIndex, double &dVal );
		unsigned GetDataArraySize();
		virtual bool GetMinValue(double *pdMinValue, unsigned *puIndex = NULL);
		virtual bool GetMaxValue(double *pdMaxValue, unsigned *puIndex = NULL);
		virtual bool GetMinMaxValues(double *pdMinValue, double *pdMaxValue, unsigned *puMinIndex = NULL, unsigned *puMaxIndex = NULL);
		virtual void Copy( CLogChan *pLogChan );
	protected:
		virtual void Write( OUTSTREAM outf );
		virtual bool Read(INPSTREAM inpf);
	};

////////////////////////////////////////////////////////////
//	Main Class  -> CDS_LogDataX
////////////////////////////////////////////////////////////
	friend class CDS_LogDataX::CChan;
	friend class CDS_LogDataX::CPhysChan;
	friend class CDS_LogDataX::CLogChan;

protected:	
	LogType			m_LogType;
	wchar_t			m_Name[_NAME_MAX_LENGTH+1];
	_string			m_Description;
	// Variables needed for storing/retrieving tsp information
	wchar_t			m_HMID[ _ID_LENGTH + 1 ];
	wchar_t			m_DSUID[_ID_LENGTH + 1];
	wchar_t			m_PlantName[_NAME_MAX_LENGTH + 1];
	wchar_t			m_ModuleName[_NAME_MAX_LENGTH + 1];
	unsigned short 	m_ValvePosition;		// Circuit position in his module
	CDS_HydroModX::eLocate	m_ValveLoc;		// valve localization in his circuit
	CDS_HydroModX::eHMXBalType	m_BalType;		// Type of Balancing

	
	UINT64			m_SensorMAC;					// Sensor Identification (MAC)	
	__time32_t		m_StartingDateTime;				// Measurement Starting Date-Time
	unsigned short	m_MeasuringStep;				// Measuring step in second
	unsigned short  m_SampleNumber;					// Number of sample requested
	unsigned char	m_ucLogStatus;					// Log status: 	IN_COURSE = 0x00,
													//				NORMAL_END = 0x01,
													//				STOP_BY_OPERATOR = 0x02,
													//				STOP_BAT_LOW = 0x03,
													//				STOP_POWER_OFF = 0x04
	bool			m_bTAMode;						// TAMode or KvMode
	IDPTR			m_ValveIDPtr;					// In case of Q measurment Valve IDPtr in TADB
	double			m_dOpening;						// Valve Opening
	double			m_dKv;							// KV value for non TAValve
	CWaterChar		m_WaterChar;					// Water characteristics, used for Q, and Power
	CWaterChar		m_WaterCharComputed; 			// Water characteristics, corrected with temperature data
	CPhysChan*		m_pSPTP;						// Keep a link with SPTP for computing WaterChar
	
	typedef std::map<unsigned,CChan *> m_map;
	m_map m_LogChan;
	m_map::iterator m_It;
	void ClearLogChan();
	// Add references to a physical channel, storing is done on Physical Channels
	// Return logging channel index
	CPhysChan* AddPhysicalChan(CLog::eChannelType ChannelType, unsigned char ucSensorNum);
	//CPhysChan* AddTempChan(unsigned char ucSensorNum){return AddPhysicalChan(ucSensorNum, ucSensorNum); }
	//CPhysChan* AddDpChan(unsigned char ucSensorNum){return AddPhysicalChan(CChan::ect_Dp, ucSensorNum); }

	// Logical channels use existing physical channels.
	// Return allocated Logical chan or -1 in case of missing physical channel
	int AddFlowChan(unsigned usDp);
	int AddDeltaTChan( unsigned usT1, unsigned usT2);
	int AddPowerChan( unsigned usQ, unsigned usT1, unsigned usT2);

public:
	void SetTAMode(bool bTAMode=true){if( m_bTAMode!=bTAMode){ m_bTAMode = bTAMode;Modified();}};
	// In case of known valve. 
	bool SetLogType(LogType logtype, uint8_t TempSensorUsed, _string Valveid, double dOpening, CWaterChar *pWC);
	// In case of unknown valve.
	bool SetLogType(LogType logtype, uint8_t TempSensorUsed, double dKV, CWaterChar *pWC);
	bool SetLogType(LogType logtype, uint8_t TempSensorUsed);
	void SetLogType(LogType logtype){ m_LogType=logtype; }
	LogType GetLogType() { return m_LogType; }
	unsigned short GetMeasuringStep() { return m_MeasuringStep; }
	unsigned short GetSampleNumber() { return m_SampleNumber; }
	void SetSampleNumber(unsigned short nbr){if( m_SampleNumber != nbr) { m_SampleNumber = nbr; Modified(); }};
	
	void SetSensorMac(UINT64 Mac){ m_SensorMAC=Mac; }

	void SetDescription(_string description){ m_Description=description; }
#ifdef TACBX
	void SetName(_string name){if(!IDcmp( m_Name,name.c_str())) return; wcsncpy( m_Name,name.c_str(), SIZEOFINTCHAR( m_Name)-1);Modified(); }
#else
	void SetName(_string name){if(!IDcmp( m_Name,name.c_str())) return;  wcsncpy_s( m_Name, SIZEOFINTCHAR( m_Name),name.c_str(), SIZEOFINTCHAR( m_Name)-1);Modified(); }
#endif
	

	// Access to the Hydromod circuit
#ifdef TACBX
	void SetHMID(wchar_t *pID){if(!IDcmp( m_HMID,pID)) return; wcsncpy( m_HMID,pID, SIZEOFINTCHAR( m_HMID ) - 1 );Modified(); }
	void SetDSUID(wchar_t *pID){if(!IDcmp( m_DSUID,pID)) return; wcsncpy( m_DSUID,pID, SIZEOFINTCHAR( m_DSUID ) - 1 );Modified(); }
	void SetPlantName(wchar_t *pID){if(!IDcmp( m_PlantName,pID)) return; wcsncpy( m_PlantName,pID, SIZEOFINTCHAR( m_PlantName)-1);Modified(); }
	void SetModuleName(wchar_t *pID){if(!IDcmp( m_ModuleName,pID)) return; wcsncpy( m_ModuleName,pID, SIZEOFINTCHAR( m_ModuleName)-1);Modified(); }
	void SaveLogData(void *pVoidSensor, void* pVoidLog);
#else
	void SetHMID(wchar_t *pID){if(!IDcmp( m_HMID,pID)) return; wcsncpy_s( m_HMID, SIZEOFINTCHAR( m_HMID),pID, SIZEOFINTCHAR( m_HMID ) - 1 );Modified(); }
	void SetDSUID(wchar_t *pID){if(!IDcmp( m_DSUID,pID)) return; wcsncpy_s( m_DSUID, SIZEOFINTCHAR( m_DSUID),pID, SIZEOFINTCHAR( m_DSUID ) - 1 );Modified(); }
	void SetPlantName(wchar_t *pID){if(!IDcmp( m_PlantName,pID)) return; wcsncpy_s( m_PlantName, SIZEOFINTCHAR( m_PlantName),pID, SIZEOFINTCHAR( m_PlantName)-1);Modified(); }
	void SetModuleName(wchar_t *pID){if(!IDcmp( m_ModuleName,pID)) return; wcsncpy_s( m_ModuleName, SIZEOFINTCHAR( m_ModuleName),pID, SIZEOFINTCHAR( m_ModuleName)-1);Modified(); }
#endif

	void SetValvePosition(unsigned short Pos){if( m_ValvePosition == Pos) return; m_ValvePosition = Pos; Modified(); }
	unsigned short GetValvePosition() { return m_ValvePosition; }

	void SetValveLocation(CDS_HydroModX::eLocate Loc){if( m_ValveLoc == Loc) return; m_ValveLoc = Loc; Modified(); }
	CDS_HydroModX::eLocate GetValveLocation() { return m_ValveLoc; }

	void SetBalType(CDS_HydroModX::eHMXBalType BalType){if( m_BalType == BalType) return; m_BalType = BalType; Modified(); }
	CDS_HydroModX::eHMXBalType GetBalType() { return m_BalType; }

	void SetKv(double dKv){if( m_dKv == dKv) return; m_dKv = dKv;Modified(); }
	void SetValveID(_string id);
	void SetOpening(double dOpening){if( m_dOpening == dOpening) return; m_dOpening = dOpening;Modified(); }
	void SetWaterChar(CWaterChar *pWC);

	
	wchar_t *GetHMID() { return m_HMID; }
	wchar_t *GetDSUID() { return m_DSUID; }
	wchar_t *GetPlantName() { return m_PlantName; }
	wchar_t *GetModuleName() { return m_ModuleName; }
	_string GetDescription() { return m_Description; }
	LPCTSTR GetName() { return m_Name; }
	IDPTR GetValveIDPtr(/*CTADatabase *pTADB*/);

	bool	IsTAMode() { return m_bTAMode; }
	double GetOpening()	{return m_dOpening; }
	double GetKv() { return m_dKv; }
	CWaterChar *GetWaterChar( int iIndex = -1 );

	// Access to a CChan pointer
	CChan *GetpChannel(unsigned LogChannelKey);
	CChan *GetFirstpChan();
	CChan *GetNextpChan();
	m_map *GetChannelList() { return &m_LogChan; }
	unsigned GetChannelsCount() { return (unsigned)m_LogChan.size(); }
	bool DeleteChan(unsigned LogChannelKey);
	bool GetData(unsigned uKey, unsigned uIndex, double &dVal);

	void SetSize(unsigned usSize);
	unsigned GetSize();

	// Date & Time management 
	// Year  -> tm_year as 2008
	// month -> tm_mon from 1 to 12
	// day	 -> tm_day from 1 to 31
	bool SetStartingDateTime(__time32_t StartDT, unsigned short usMesuringStep);
	unsigned short GetStartingDateTime(__time32_t *pStartingDateTime);
	void GetStopDateTime(__time32_t *pStopDateTime);
	bool GetDataDateTime (unsigned index, __time32_t *pDataTime);
	
	void SetLogStatus(unsigned char ucStatus){if( m_ucLogStatus == ucStatus) return; m_ucLogStatus = ucStatus;Modified(); }
	unsigned char GetLogStatus() { return m_ucLogStatus; }

#ifndef TACBX
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Use virtual functions derived from CLog that will be
	// used to show information in the RViewChart
	// !!!!!!! Used only for HySelect !!!!!!!!!!!!!!!!!!!!!!!!
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool		m_bStartDate; // flag is set when startdate is initialised
	CTime		m_StartDate;
	int	m_Pos[MAXLOGGEDCOLUMN];	// Position in the list for GetFirst() and GetNext()

	virtual	LPCTSTR GetSite();
	virtual	LPCTSTR GetRef();
	virtual BYTE GetValveIndex();
	virtual unsigned short GetInterval();
	virtual CString GetValveName();
	
	// Return the logged Mode (0 pressure; 1 flow; 2 ...).
	virtual	BYTE GetMode();
	
	// Use the GetPrototypeValue() function to match the differences between
	// the TACBI and the TASCOPE.
	virtual	bool GetValue( CLog::eChannelType chan, int iRow, double &dVal );
	
	// Give the value with a date time closed to dt.
	// Return false if dt is less than StartDateTime.
	// Give  the last value if dt is greater than the last DateTime.
	virtual	bool GetValue( CLog::eChannelType chan, CTime &dt, double &dVal );
	
	// Give the value with a date time closed to StartDateTime + ts.
	virtual	bool GetValue( CLog::eChannelType chan, CTimeSpan &ts, double &dVal );
	
	// Position iterator at the beginning of the list and give the value at this position in dVal.
	virtual bool GetFirst( CLog::eChannelType chan, double &dVal, int iIndex = 0 );
	
	// Position iterator at next element in the list and give the value at this position in dVal.
	virtual bool GetNext( CLog::eChannelType chan, double &dVal );
	
	virtual	CTime GetLogDateTime() { if( true == InitStartDate() ) return m_StartDate; else return CTime( 0 ); }
	
	// Return the number of logged values for this column.
	virtual	int GetLength();
	bool InitStartDate();
	
	// Return the date time for the (col,row) value.
	virtual	bool GetDateTime( int iRow, CTime &dt );
	virtual	bool GetOleDateTime( int iRow, COleDateTime &dt );
	virtual	bool GetHighestValue( CLog::eChannelType chan, double &dVal );
	virtual	bool GetLowestValue( CLog::eChannelType chan, double &dVal );

	// Write logged data as a character delimited text file.
	virtual bool WriteAsText( CString strPathName, bool bPointIndex = true, bool bHeader = true, CString strDelimiter = _T(",") );
	virtual ToolUsed ToolUsedForMeasure() { return CLog/*::ToolUsed*/::TOOL_TASCOPE; }

	// Function used only for TA-SCOPE: Return true if T1 is used, false for T2.
	// Used for Dp-Temp log, Flow-Temp log or Temp log only.
	virtual bool IsT1SensorUsed();
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif	// TACBX

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

private:
	bool _SetLogTypeTerminate(LogType logtype, uint8_t TempSensorUsed);
#ifndef TACBX
	void GetTempString(int index, CString &TxtLine, CString Delimiter);
#endif
};

#endif // ifndef PRODUCTION (line number about 118 just after the CEquation class definition).

#endif // !defined(DATASTRUCT_H__INCLUDED_)
