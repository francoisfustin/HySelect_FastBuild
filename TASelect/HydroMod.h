
//
// HydroMod.h: interface for the CDS_HydroMod class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HYDROMOD_H__6418221E_811A_49EE_91BF_6FA1CA79AC96__INCLUDED_)
#define AFX_HYDROMOD_H__6418221E_811A_49EE_91BF_6FA1CA79AC96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "DataBase.h"
#include "DataBObj.h"
#include "Global.h"
#include "datastruct.h"
#include "HydroMod_DebugLog.h"

//////////////////////////////////////////////////////////////////////
//
//	A CDS_HydroMod is a CTable appended with a valve. This valve is the
//  partner valve of the module if the CDS_HydroMod has children.
//	Each CDS_HydroMod is indexed with its position in its parent.
//
enum enum_TreatmentType
{
	ett_None,
	ett_Delete,
	ett_Kept,
	ett_Added,
	ett_Moved,
	ett_Reinsert,
	ett_ValveModified,
	ett_Last
};

enum enum_VDescriptionType
{
	edt_Undefined = -1,
	edt_TADBValve = 0,
	edt_CBISizeValve,
	edt_CBISizeInchValve,
	edt_KvCv
};

enum enum_ValveInCBI
{
	evi_ValveNotPresent,
	evi_ValvePresent,
	evi_Undetermined
};

class CTADatastruct;
class CTADatabase;
class CDS_WaterCharacteristic;
class CPipes;
class CTermUnit;
class CPump;

extern CDS_HydroMod_DebugLog m_clHydroModDebugLog;

class CValvesData
{
public:
	CValvesData( CDB_TAProduct *pTAP = NULL ) { Clear(); m_pTAP = pTAP; }

	CDB_TAProduct *GetpTAP() const { return m_pTAP; }
	double GetDp() const { return m_dDp; }
	double GetDpFO() const { return m_DpFO; }
	double GetSignal() const { return m_dSignal; }
	double GetSetting() const { return m_dSetting; }

	void SetpTAP( CDB_TAProduct *val ) { m_pTAP = val; }
	void SetDp( double val ) { m_dDp = val; }
	void SetDpFO( double val ) { m_DpFO = val; }
	void SetSignal( double val ) { m_dSignal = val; }
	void SetSetting( double val ) { m_dSetting = val; }

	void Clear() { m_pTAP = NULL; m_dDp = -1; m_dSignal = -1; m_dSetting = -1; m_DpFO = -1;}
private:
	CDB_TAProduct *m_pTAP;
	double m_dDp;
	double m_DpFO;
	double m_dSignal;
	double m_dSetting;
};

class CDpCData
{
public:
	CDpCData( CDB_DpController *pDpC = NULL )
	{
		Clear();
		m_pDpC = pDpC;
		m_dDpMin = 0.0;
	}

	CDB_DpController *GetpDpC() const { return m_pDpC; }
	double GetSetting() const { return m_dSetting; }
	double GetDpMin() const { return m_dDpMin; }

	void SetpDpC( CDB_DpController *val ) { m_pDpC = val; }
	void SetSetting( double val ) { m_dSetting = val; }
	void SetDpMin( double val ) { m_dDpMin = val; }

	void Clear() { m_pDpC = NULL; m_dSetting = -1; }
private:
	CDB_DpController *m_pDpC;
	double m_dSetting;
	double m_dDpMin;
};

class CCVData
{
public:
	CCVData( CDB_ControlValve *pCV = NULL ) { Clear(); m_pCV = pCV; }
	CCVData( const CCVData &clCCVData )
	{ 
		Clear(); 
		m_pCV = clCCVData.m_pCV;
		m_dDp = clCCVData.m_dDp;
		m_dHMin = clCCVData.m_dHMin;
		m_dSetting = clCCVData.m_dSetting;
		m_dDplmin = clCCVData.m_dDplmin;
		m_dDplmax = clCCVData.m_dDplmax;
		m_dDpToStabilise = clCCVData.m_dDpToStabilise;
	}

	CDB_ControlValve *GetpCV() const { return m_pCV; }
	double GetDp() const { return m_dDp; }

	// Data for combined Dp controller, control and balancing valve (TA-COMPACT-DP).
	double GetHMin() const { return m_dHMin; }
	double GetSetting() const { return m_dSetting; }
	double GetDplmin() const { return m_dDplmin; }
	double GetDplmax() const { return m_dDplmax; }
	double GetDpToStabilise() const { return m_dDpToStabilise; }

	void SetpCV( CDB_ControlValve *val ) { m_pCV = val; }
	void SetDp( double val ) { m_dDp = val; }

	// Data for combined Dp controller, control and balancing valve (TA-COMPACT-DP).
	void SetHMin( double dHMin ) { m_dHMin = dHMin; }
	void SetSetting( double dSetting ) { m_dSetting = dSetting; }
	void SetDplmin( double dDplmin ) { m_dDplmin = dDplmin; }
	void SetDplmax( double dDplmax ) { m_dDplmax = dDplmax; }
	void SetDpToStabilise( double dDpToStabilise ) { m_dDpToStabilise = dDpToStabilise; }

	void Clear() { m_pCV = NULL; m_dDp = -1.0; m_dHMin = -1.0; m_dSetting = -1.0; m_dDplmin = -1.0; m_dDplmax = -1.0; m_dDpToStabilise = 1.0; }
private:
	CDB_ControlValve *m_pCV;
	double m_dDp;

	// Data for combined Dp controller, control and balancing valve (TA-COMPACT-DP).
	double m_dHMin;
	double m_dSetting;
	double m_dDplmin;
	double m_dDplmax;
	double m_dDpToStabilise;
};

class CShutoffValveData
{
public:
	CShutoffValveData( CDB_ShutoffValve *pclShutoffValve = NULL )
	{
		Clear();
		m_pclShutoffValve = pclShutoffValve;
	}

	CDB_ShutoffValve *GetpShutoffValve() const { return m_pclShutoffValve; }
	double GetDp() const { return m_dDp; }

	void SetpDpC( CDB_ShutoffValve *pclShutoffValve ) { m_pclShutoffValve = pclShutoffValve; }
	void SetDp( double dDp ) { m_dDp = dDp; }

	void Clear() { m_pclShutoffValve = NULL; m_dDp = -1.0; }

private:
	CDB_ShutoffValve *m_pclShutoffValve;
	double m_dDp;
};

class CSmartControlValveData
{
public:
	CSmartControlValveData( CDB_SmartControlValve *pclSmartControlValve = NULL )
	{
		Clear();
		m_pclSmartControlValve = pclSmartControlValve;
	}

	CDB_SmartControlValve *GetpSmartControlValve() const { return m_pclSmartControlValve; }
	double GetDpMin() const { return m_dDpMin; }
	double GetQMax() const { return m_dQMax; }
	double GetPowerMax() const { return m_dPowerMax; }

	void SetDpMin( double dDpMin ) { m_dDpMin = dDpMin; }
	void SetQMax( double dQMax ) { m_dQMax = dQMax; }
	void SetPowerMax( double dPowerMax ) { m_dPowerMax = dPowerMax; }

	void Clear() { m_pclSmartControlValve = NULL; m_dDpMin = -1.0; m_dQMax = -1.0; m_dPowerMax = -1.0; }

private:
	CDB_SmartControlValve *m_pclSmartControlValve;
	double m_dDpMin;
	double m_dQMax;
	double m_dPowerMax;
};

class CSmartDDpCData
{
public:
	CSmartDDpCData( CDB_SmartControlValve *pclSmartDpC = NULL )
	{
		Clear();
		m_pclSmartDpC = pclSmartDpC;
	}

	CDB_SmartControlValve *GetpSmartDpC() const { return m_pclSmartDpC; }
	double GetDpMin() const { return m_dDpMin; }
	double GetQMax() const { return m_dQMax; }
	double GetPowerMax() const { return m_dPowerMax; }

	void SetDpMin( double dDpMin ) { m_dDpMin = dDpMin; }
	void SetQMax( double dQMax ) { m_dQMax = dQMax; }
	void SetPowerMax( double dPowerMax ) { m_dPowerMax = dPowerMax; }

	void Clear() { m_pclSmartDpC = NULL; m_dDpMin = -1.0; m_dQMax = -1.0; m_dPowerMax = -1.0; }

private:
	CDB_SmartControlValve *m_pclSmartDpC;
	double m_dDpMin;
	double m_dQMax;
	double m_dPowerMax;
};

class CDS_HydroMod : public CTable
{
	DEFINE_DATA( CDS_HydroMod )
public:

	// Group for error/warning messages.
	enum ErrorWarningMessageFlags
	{
		EWMF_First = 0x0001,
		EWMF_Temp = EWMF_First,
		EWMF_Flow = 0x0002,
		EWMF_Dp = 0x0004,
		EWMF_DpSignal = 0x0008,
		EWMF_MaxDp = 0x0010,
		EWMF_Dpl = 0x0020,
		EWMF_Power = 0x0040,
		EWMF_SecondaryPump = 0x0080,
		EWMF_Last = EWMF_SecondaryPump,
		EWMF_All = 0xF000
	};

	// For each group, be more specific for the kinf of error/warning messages.
	enum eValidityFlags
	{
		evfOK = 0,
		evfTempTooHigh = 0x0001,			// Group: EWMF_Temp
		evfTempTooLow = 0x0002,				// Group: EWMF_Temp
		evfFlowTooLow = 0x0004,				// Group: EWMF_Temp
		evfFlowTooHigh = 0x0008,			// Group: EWMF_Temp
		evfDpTooHigh = 0x0010,				// Group: EWMF_Dp
		evfDpTooLow = 0x0020,				// Group: EWMF_Dp
		evfDpSignalTooHigh = 0x0040,		// Group: EWMF_DpSignal
		evfDpSignalTooLow = 0x0080,			// Group: EWMF_DpSignal
		evfMaxDpTooHigh = 0x0100,			// Group: EWMF_MaxDp
		evfDplTooHigh = 0x0200,				// Group: EWMF_Dpl
		evfDplTooLow = 0x0400,				// Group: EWMF_Dpl
		evfPowerTooLow = 0x0800,			// Group: EWMF_Power
		evfPowerTooHigh = 0x1000,			// Group: EWMF_Power
		evfSecPumpHMinTechParam = 0x2000,	// Group: EWMF_SecondaryPump
		evfSecPumpNotNeeded = 0x4000,		// Group: EWMF_SecondaryPump
		evfLast = 0x1000,
	};

	enum eInstrumentUsed
	{
		TACBI,
		TASCOPE,
		NotUsed
	};

	// Used to manage TASCOPE file exportation
	enum eHMStatusFlags
	{
		sfBlackBox = 1,			// Module is a black box, TASCOPE cannot modify TU flow or Dp
		sfOrphan = 2,			// Send from the PC as a orphan module, can't be moved
		sfSelected = 4,			// Module/Circuit selected for exportation
		sfIncludedIntoBlackBox = 8// This Module/circuit is included into an orphan module
	};

	enum ePressIntType
	{
		pitStatic,
		pitDpC,
		pitPump
	};

	enum eDpType
	{
		None,
		Dp,
		Kv,
		Cv,
		dzeta,
		QDpref,
		Pipelength,
		last
	};

	enum eCVnWay
	{
		CV2W = 2,					// 2W
		CV3W = 3,					// 3W
		CV3WByp = 4,				// 3W with integrated bypass
		CV3WRdcByp = 5			// 3W with reduced bypass port
	};

	typedef enum eUseProduct
	{
		Auto = -1,
		Never,
		Always,
		Last
	};

	enum eComputeHMEvent
	{
		eceNone,
		eceResize,
		// Terminal Unit
		eceTUQ,
		eceTUDp,
		// Pipe
		ecePipeSerie,
		ecePipeSize,
		ecePipeLength,
		// BV
		eceBV,
		eceBVType,
		eceBVConnect,
		eceBVVers,
		eceBVPN,
		// DpC
		eceDpC,
		eceDpCType,
		eceDpCConnect,
		eceDpCPN,
		// CV
		eceCVKvs,
		eceCV,
		eceCVConnect,
		eceCVVers,
		eceCVBdy,
		eceCVPN,
		// BypassBV
		eceBypBvAdded,					// Special case when user select 'Always' or 'Auto' to correctly resize valve.
		// ShutOffValves
		eceShutOffValves,
		eceShutoffValveType,
		eceShutoffValveConnect,
		eceShutoffValveVersion,
		eceShutoffValvePN,
		eceShutoffValveAdded,			// Special case when user select 'Always' or 'Auto' to correctly resize valve.
		// Smart control valves
		eceSmartControlValve,
		eceSmartControlValveBodyMaterial,
		eceSmartControlValveConnect,
		eceSmartControlValvePN,
		// Smart differential pressure controllers.
		eceSmartDpC,
		eceSmartDpCBodyMaterial,
		eceSmartDpCConnect,
		eceSmartDpCPN,
		// Last EVENT
		eceLAST
	};

	// HYS-1583: NEVER change the order of this enum. These are saved in a project.
	// If you need a new variable, put it at the end !!
	enum eHMObj
	{
		eNone = 0x0000,
		eCircuitPrimaryPipe	= 0x0001,
		eCircuitSecondaryPipe	= 0x0002,
		eDistributionSupplyPipe	= 0x0004,
		eBVprim = 0x0008,
		eBVbyp = 0x0010,
		eBVsec = 0x0020,
		eDpC = 0x0040,
		eCV = 0x0080,
		eShutoffValveSupply	= 0x0100,
		eShutoffValveReturn	= 0x0200,
		eDistributionReturnPipe	= 0x0400,
		eSmartControlValve = 0x0800,
		eSmartDpC = 0x1000,
		eCircuitBypassPrimaryPipe = 0x2000,
		eAllPipes = eCircuitPrimaryPipe + eCircuitSecondaryPipe + eDistributionSupplyPipe + eDistributionReturnPipe + eCircuitBypassPrimaryPipe,
		eALL = eAllPipes + eBVprim + eBVbyp + eBVsec + eDpC + eCV + eShutoffValveSupply + eShutoffValveReturn + eSmartControlValve + eSmartDpC
	};

	enum ReturnType
	{
		Direct,
		Reverse,
		LastReturnType
	};

	union _uDpVal
	{
		double Dp;
		double Kv;
		double Cv;
		double Length;	// Equivalent pipe length
		struct _Dzeta
		{
			double dzeta;
			double d;
		} sDzeta;
		struct _QDpref
		{
			double Dpref;
			double Qref;
		} sQDpRef;
		struct _Val
		{
			double val1;
			double val2;
		} sVal;
	} uDpVal;

	// HYS-1882
	enum TemperatureErrors
	{
		TE_OK = 0x0000,
		TE_TemperatureEquals = 0x0001,
		TE_ReturnTemp_FreezingPoint = 0x0002,
		TE_Heating_TrpGreaterThanTsp = 0x0004,
		TE_Cooling_TrpLowerThanTsp = 0x0008
	};

public:
	//********************** Sub-CLASS
	class CBase
	{
	public:
		CBase( CDS_HydroMod *pParent ) { m_pParent = pParent; }
		virtual ~CBase() {}
		CDS_HydroMod *GetpParentHM() { return m_pParent; }
		virtual IDPTR GetTAPIDPtr() { return _NULL_IDPTR; }
		// HYS-1734: GetHMObjectType base to get location of each valve.
		virtual eHMObj GetHMObjectType() = 0;

	protected:
		void _FreeBuffer( CMemFile *pclMemFile );
		void _DropDataHelp( CString &str, CDB_StringID *pCDBStringID, CString strName );
		bool _ReadString( CMemFile &clMemFile, CString &strResult );

	protected:
		CDS_HydroMod *m_pParent;
	};

	//********************** Sub-CLASS
	// Measured values
	class CMeasData : public CBase
	{
		friend class CDS_HydroMod;
	protected:
		// Saved variables
		__time32_t				m_MeasDateAndTime;		// Date and time for the current measurement
		__time32_t				m_TABalDateAndTime;		// Date and time for balancing operation
		eInstrumentUsed			m_Instrument;
		unsigned int			m_uiQMType;				// Type of Quick Measurement

		// Valve
		CDS_HydroModX::eLocate	m_eLocate;				// Define where is located the balancing valve
		CDS_HydroModX::eMode	m_eMode;					// TAMode, KvMode
		TCHAR					m_TAPID[_ID_LENGTH + 1];	// IDPtr of selected TAP in TADB
		double					m_dKv;					// Valve Kv in case of non TAMode
		CWaterChar				m_WC;

		// Measures
		double					m_dMeasDp;				// Measured Dp / could be the last Dp measure in case of balancing
		double					m_dMeasDpl;
		double					m_dCurOpening;			// Valve Opening for current measure
		double					m_dT[CDS_HydroModX::eDTS::LastDTS];
		CDS_HydroModX::eDTS		m_UsedDTSensor[2];		// Temp sensor used for DT/Power measurement

		// Design values
		double					m_dDesignQ;				// Design flow
		double					m_dDesignOpening;		// Design opening
		double					m_dDesignDT;			// Design DT
		// 		double					m_dDesignPower;			// Design power
		// 		double					m_dDesignDp;			// Design Dp

		// Balancing
		CDS_HydroModX::eTABalanceMethod		m_eBalanced;	// balancing done
		bool					m_bTaBalMeasCompleted;		// All measures done
		double					m_dTaBalOpening_1;			// Opening for stage 1 of TA Balance
		double					m_dTaBalMeasDp_1;			// Measured DP for stage 1 of TA Balance
		double					m_dTaBalMeasDp_2;			// Measured Dp for stage 2 of TA Balance
		double					m_dTaBalOpeningResult;
		double					m_DpPVTABal;				// TA Balance: Dp on valve closed and considered as PV of the HydroMod

		// Computed
		double					m_dTABalDistribDp;
		double					m_dTABalCircuitDp;

		CHMXPipe				*m_pDistributionPipe;		// Distribution pipe retrieve from HydroModX (TAScope)
		CHMXPipe				*m_pCircuitPipe;				// Circuit pipe retrieve from HydroModX (TAScope)
		CHMXTU					*m_pTerminalUnit;			// Terminal unit retrieve from HydroModX (TAScope)

		// CBI specific
		double					m_dCBIQFinal;					// final flow
		double					m_dCBIQTABal1;					// Flow for TA Balance stage 1

		void Write( OUTSTREAM outf );
		bool Read( INPSTREAM inpf );

	public:
		void Copy( CDS_HydroMod::CMeasData *pTo );

		CMeasData( CDS_HydroMod *pHM );
		~CMeasData();

		__time32_t				GetMeasDateTime() { return m_MeasDateAndTime; }
		eInstrumentUsed			GetInstrument() { return m_Instrument; }
		unsigned				GetQMType() { return m_uiQMType; }

		// Valve
		// HYS-1734: GetHMObjectType because the base is pure virtual but it is not necessary here.
		eHMObj GetHMObjectType() { return eHMObj::eNone; }
		// HYS-1734: GetLocate renamed to GetPipeLocation().
		CDS_HydroModX::eLocate	GetPipeLocation() { return m_eLocate; }						// Define where is located the balancing valve.
		CDS_HydroModX::eMode	GetMode() { return m_eMode; }								// TAMode, KvMode
		CDB_TAProduct 			*GetpTAP() { return ( CDB_TAProduct * ) TASApp.GetpTADB()->Get( m_TAPID ).MP; }	// Pointer to the product in the TADB
		double					GetKv() { return m_dKv; }									// Valve Kv in case of non TAMode
		CWaterChar				*GetpWC() { return &m_WC; }
		// Measures
		double					GetMeasDp() { return m_dMeasDp; }							// Measured Dp / could be the last Dp measure in case of balancing
		double					GetMeasDpL() { return m_dMeasDpl; }							// Measured Dpl / could be the last Dp measure in case of balancing
		double					GetFlow( double dOpening, double dDp );						// Return the flow for the current valve with an opening and a Dp
		double					GetMeasFlow();												// Measured flow for the current opening and the measured Dp value
		double 					GetFlowRatio() {if( GetDesignQ() > 0 ) { return ( GetMeasFlow() / GetDesignQ() * 100 ); } return 0;}  // Return the ratio in % (MeasFlow/DesignQ*100)
		double					GetCurOpening();											// Valve Opening for current measure
		double					GetT( CDS_HydroModX::eDTS dts ) {if( dts < CDS_HydroModX::eDTS::LastDTS && dts > CDS_HydroModX::eDTS::DTSUndef ) { return m_dT[dts]; } else { return -273.15; } ;}
		double 					GetDTRatio();												// Return the ratio in % (MeasDT/DesignDT*100)
		double 					GetPowerRatio();											// Return the ratio in % (MeasPower/DesignPower*100)
		double					GetPower();													// Return the power value for the measuring values
		CDS_HydroModX::eDTS		GetUsedDTSensor( int i ) {if( i < 2 ) { return ( m_UsedDTSensor[i] ); } else { return ( CDS_HydroModX::eDTS::LastDTS ); }}	// Temp sensor used for DT/Power measurement
		_string					GetstrCurOpening();											// Return an _string with the current opening
		_string					GetstrDesignOpening();										// Return an _string with the Design opening
		_string					GetstrKv();													// Return an _string with the Kv value
		_string					GetstrMeasDp();												// Return an _string with the Dp
		_string					GetstrMeasQ();												// Return an _string with the Flow
		_string					GetstrDesignQ();											// Return an _string with the design flow
		_string					GetstrT( CDS_HydroModX::eDTS dts );							// Return an _string with the Temperature
		_string					GetstrDT( CDS_HydroModX::eDTS dts1, CDS_HydroModX::eDTS dts2 );  // Return an _string with the DT between two arguments
		_string					GetstrDesignDT();											// Return an _string with the design DT
		_string					GetstrPower();												// Return an _string with the design power
		_string					GetstrDesignPower();										// Return an _string with the design power
		// Design values
		double					GetDesignQ() {return m_dDesignQ;}								// Design flow
		double					GetDesignOpening() {return m_dDesignOpening;}					// Design opening
		double					GetDesignDT() {return m_dDesignDT;}								// Design DT
		double					GetDesignPower();												// Design power
		double					GetDesignDp();													// Design Dp
		// Balancing
		__time32_t				GetTABalDateTime() {return m_TABalDateAndTime;}
		CDS_HydroModX::eTABalanceMethod		GetFlagBalanced() {return m_eBalanced;}						// balancing done
		bool					GetFlagTaBalMeasCompleted() {return m_bTaBalMeasCompleted;}	// All measures done
		double					GetTaBalOpening_1() {return m_dTaBalOpening_1;}				// Opening for stage 1 of TA Balance
		double					GetTaBalMeasDp_1() {return m_dTaBalMeasDp_1;}					// Measured DP for stage 1 of TA Balance
		double					GetTaBalMeasDp_2() {return m_dTaBalMeasDp_2;}					// Measured Dp for stage 2 of TA Balance
		double					GetTaBalOpeningResult() {return m_dTaBalOpeningResult;}
		double					GetDpPVTABal() {return m_DpPVTABal;}							// TA Balance: Dp on valve closed and considered as PV of the HydroMod

		// Computed
		double					GetDistribDpComputed() { return m_dTABalDistribDp; }
		double					GetCircDpComputed() { return m_dTABalCircuitDp; }

		CHMXPipe				*GetpDistributionPipe() { return m_pDistributionPipe; }
		CHMXPipe				*GetpCircuitPipe() { return m_pCircuitPipe; }
		CHMXTU					*GetpTerminalUnit() { return m_pTerminalUnit; }

		// CBI specific
		// TODO adapt Getqf to the case of scope compute flow using Valve ID (or Kv) saved water char, and Dp
		double					Getqf() {return m_dCBIQFinal;}											// final flow
		double					GetqTABal1() {return m_dCBIQTABal1;}								// Flow for TA Balance stage 1


		void SetMeasDateTime( __time32_t DateAndTime )					{m_MeasDateAndTime = DateAndTime;}
		void SetInstrument( eInstrumentUsed Instrument )				{m_Instrument = Instrument; m_pParent->Modified();}
		void SetQMType( CDS_HydroModX::eQMtype QMType )				{m_uiQMType |= QMType; m_pParent->Modified();}				// Type of Quick Measurement
		void SetQMType( unsigned int QMType )							{m_uiQMType = QMType; m_pParent->Modified();}				// Type of Quick Measurement
		bool IsQMType( CDS_HydroModX::eQMtype QMType )				{return ( ( m_uiQMType & QMType ) == QMType );}
		// Valve
		void SetLocate( CDS_HydroModX::eLocate Locate )				{m_eLocate = Locate; m_pParent->Modified();}				// Define where is located the balancing valve
		void SetMode( CDS_HydroModX::eMode Mode )						{m_eMode = Mode ; m_pParent->Modified();}					// TAMode, KvMode
		void SetTAPID( TCHAR *TAPID )								{wcscpy_s( m_TAPID, TAPID ) ; m_pParent->Modified();}	// IDPtr of selected TAP in TADB
		void SetKv( double dKv )										{m_dKv = dKv; m_pParent->Modified();}						// Valve Kv in case of non TAMode
		void CopyWCData( CWaterChar *WC )       	 		                {m_WC = *WC ; m_pParent->Modified();}
		// Measures
		void SetMeasDp( double dMeasDp )								{m_dMeasDp = dMeasDp ; m_pParent->Modified();}				// Measured Dp / could be the last Dp measure in case of balancing
		void SetMeasDpL( double dMeasDpL )							{m_dMeasDpl = dMeasDpL ; m_pParent->Modified();}			// Measured Dpl / could be the last Dpl measure in case of balancing
		void SetCurOpening( double dCurOpening )						{m_dCurOpening = dCurOpening; m_pParent->Modified();}		// Valve Opening for current measure
		void SetT( CDS_HydroModX::eDTS dts, double dT )				{if( dts < CDS_HydroModX::eDTS::LastDTS ) {m_dT[dts] = dT ; m_pParent->Modified();}}
		void SetUsedDTSensor( int i, CDS_HydroModX::eDTS dts )		{if( i < 2 ) {m_UsedDTSensor[i] = dts ; m_pParent->Modified();}}	// Temp sensor used for DT/Power measurement
		// Design values
		void SetDesignQ( double dDesignQ )							{m_dDesignQ = dDesignQ; m_pParent->Modified();}				// Design flow
		void SetDesignOpening( double dDesignOpening )				{m_dDesignOpening = dDesignOpening; m_pParent->Modified();}	// Design opening
		void SetDesignDT( double dDesignDT )							{m_dDesignDT = dDesignDT; m_pParent->Modified();}			// Design DT
		// Balancing
		void SetTABalDateTime( __time32_t DateAndTime )					{m_TABalDateAndTime = DateAndTime;}					// Balancing Date and Time
		void SetFlagBalanced( CDS_HydroModX::eTABalanceMethod Balanced )	{m_eBalanced = Balanced; m_pParent->Modified();}			// balancing done
		void SetFlagTaBalMeasCompleted( bool bTaBalMeasCompleted )	{m_bTaBalMeasCompleted = bTaBalMeasCompleted; m_pParent->Modified();}	// All measures done
		void SetTaBalOpening_1( double dTaBalOpening_1 )				{m_dTaBalOpening_1 = dTaBalOpening_1 ; m_pParent->Modified();}		// Opening for stage 1 of TA Balance
		void SetTaBalMeasDp_1( double dTaBalMeasDp_1 )				{m_dTaBalMeasDp_1 = dTaBalMeasDp_1; m_pParent->Modified();}			// Measured DP for stage 1 of TA Balance
		void SetTaBalMeasDp_2( double dTaBalMeasDp_2 )				{m_dTaBalMeasDp_2 = dTaBalMeasDp_2 ; m_pParent->Modified();}			// Measured Dp for stage 2 of TA Balance
		void SetTaBalOpeningResult( double dTaBalOpeningResult )		{m_dTaBalOpeningResult = dTaBalOpeningResult ; m_pParent->Modified();}
		void SetDpPVTABal( double DpPVTABal )							{m_DpPVTABal = DpPVTABal ; m_pParent->Modified();}					// TA Balance: Dp on valve closed and considered as PV of the HydroMod

		// Computed
		void SetTABalDistribDp( double dTABalDistribDp )			{ m_dTABalDistribDp = dTABalDistribDp; m_pParent->Modified(); }
		void SetTABalCircDp( double dTABalCircDp )					{ m_dTABalCircuitDp = dTABalCircDp; m_pParent->Modified(); }

		void SetpDistributionPipe( CHMXPipe *pDistributionPipe );
		void SetpCircuitPipe( CHMXPipe *pCircuitPipe );
		void SetpTerminalUnit( CHMXTU *pTerminalUnit );

		// CBI specific
		void Setqf( double qf )										{m_dCBIQFinal = qf ; m_pParent->Modified();}									// final flow
		void SetqTABal1( double qTABal1 )								{m_dCBIQTABal1 = qTABal1 ; m_pParent->Modified();}						// Flow for TA Balance stage 1

		virtual void DropData( OUTSTREAM outf, int *piLevel );
	};

	//********************** Sub-CLASS
	// Shut-Off Valve
	class CShutoffValve : public CBase
	{
		friend class CDS_HydroMod;
	public:
		CShutoffValve( CDS_HydroMod *pHM, eHMObj Located );
		~CShutoffValve();

		// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
		// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
		void Copy( CDS_HydroMod::CShutoffValve *pclTargetHMShutoffValve, bool bResetDiversityFactor = true  );

		IDPTR SelectBestShutoffValve();
		void InvalidateSelection();

		std::multimap <int, CShutoffValveData > *GetpMapShutoffValve() { return &m_mapShutoffValve; }

		CDB_TAProduct *GetpTAP() { return dynamic_cast<CDB_TAProduct *>( this->GetIDPtr().MP ); }
		IDPTR GetIDPtr();
		IDPTR GetBestIDPtr();

		int GetAvailableType( std::map<int, CDB_StringID *> *pmap );
		int GetAvailableConnections( std::map<int, CDB_StringID *> *pmap );
		int GetAvailableVersion( std::map<int, CDB_StringID *> *pmap );
		int GetAvailablePN( std::map<int, CDB_StringID *> *pmap );

		CDB_StringID *GetpSelType();
		CDB_StringID *GetpSelConnection();
		CDB_StringID *GetpSelVersion();
		CDB_StringID *GetpSelPN();

		void SetpSelTypeAccordingUserChoice( CDB_StringID *pSelpType );
		void SetpSelConnAccordingUserChoice( CDB_StringID *pSelpConn );
		void SetpSelVersionAccordingUserChoice( CDB_StringID *pSelpVersion );
		void SetpSelPNAccordingUserChoice( CDB_StringID *pSelpPN );

		double GetQ() { return m_dQ; }
		double GetDp();
		CDB_ValveCharacteristic *GetpValveChar();
		unsigned short GetValidtyFlags() const { return m_usValidityFlags; }
		eHMObj GetHMObjectType() { return m_eLocated; }
		eBool3 CheckDpMinMax();

		IDPTR GetDiversityIDPtr();
		void SetDiversityIDPtr( IDPTR val ) { m_DiversityIDPtr = val; }
		
		void SetLocate( eHMObj locate )	{ if( locate != m_eLocated ) { m_eLocated = locate; m_pParent->Modified(); } }
		void SetIDPtr( IDPTR idptr );
		void SetBestIDPtr( IDPTR IDPtr ) { m_IDPtrBest = IDPtr; }
		void SetDp( double dDp );
		void SetQ( double dQ ) {ASSERT( dQ >= 0 ); if( m_dQ != dQ ) { m_dQ = dQ; m_pParent->Modified(); } }
		void SetValidtyFlags( unsigned short usValidityFlags ) { m_usValidityFlags = usValidityFlags; }

		bool IsCompletelyDefined();
		void ResizeShutoffValve();
		bool IsBestShutoffValve( CData *pData );
		IDPTR GetTAPIDPtr() {return m_IDPtr;}

		bool FillShutoffValveInfoIntoHMX( CDS_HydroModX *pHMX, eHMObj HMObj );
		bool ForceShutoffValveSelection( IDPTR DeviceIDptr );

		// Verify flow, temp, ... compatibility for each computed device.
		int CheckValidity();

		virtual void DropData( OUTSTREAM outf, int *piLevel );
		void ClearUserChoiceTemporaryVariables();

		bool IsShutoffValveExistInPreselectedList( CDB_TAProduct *pclShutoffValve );
	
	protected:
		void Write( OUTSTREAM outf );
		bool Read( INPSTREAM inpf );

	private:
		void _AbortSelectBestShutoffValve();
		void _SetpSelShutoffValve( LPARAM pTADBShutoffValve, bool bForceLocker = false );
	
	protected:
		// Saved variables.
		IDPTR m_IDPtr;						// IDPtr of selected valve in TADB
		IDPTR m_IDPtrBest;					// IDPtr of best selected valve in TADB
		IDPTR m_DiversityIDPtr;				// IDPtr of selected valve w/o diversity
		double m_dQ;						// used to store Q
		double m_dDp;
		eHMObj m_eLocated;

		// work variables
		CDB_StringID *m_pUserChoiceType;
		CDB_StringID *m_pUserChoiceConn;
		CDB_StringID *m_pUserChoiceVersion;
		CDB_StringID *m_pUserChoicePN;
		unsigned short m_usValidityFlags;		// Combination from eValidityFlags

		std::multimap <int, CShutoffValveData > m_mapShutoffValve;
	};

	//********************** Sub-CLASS
	//
	class CBV : public CBase
	{
		friend class CDS_HydroMod;
	public:
		CBV( CDS_HydroMod *pHM, eHMObj Located );
		~CBV();

		// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
		// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
		void Copy( CDS_HydroMod::CBV *pclTargetHMBv, bool bResetDiversityFactor = true );

		std::multimap <int, CValvesData > *GetpMapValves() { return &m_mapValves; }
		IDPTR GetTAPIDPtr() {return GetIDPtr();}
		IDPTR GetIDPtr();
		IDPTR GetDiversityIDPtr();
		void SetDiversityIDPtr( IDPTR val ) { m_DiversityIDPtr = val; }
		double GetQ();
		double GetSetting();
		double GetDp();
		double GetDpSignal();
		double GetOpeningMax();
		CDB_ValveCharacteristic *GetpValveChar();
		CDB_TAProduct *GetpTAP() { return dynamic_cast<CDB_TAProduct *>( GetIDPtr().MP ) ; }

		unsigned short GetValidtyFlags() const { return m_usValidityFlags; }
		// DpMin for BV with a selection on a unknown Dp (close to 3kPa)
		double GetDpMin( bool bBounds = true );
		eHMObj GetHMObjectType() { return m_eLocated; }

		CDB_StringID *GetpSelBVType();
		CDB_StringID *GetpSelBVConn();
		CDB_StringID *GetpSelBVVers();
		CDB_StringID *GetpSelBVPN();

		CPipes *GetpPipe();

		IDPTR GetBestBvIDPtr();

		void SetLocate( eHMObj locate )	{ if( locate != m_eLocated ) { m_eLocated = locate; m_pParent->Modified(); } }
		void SetIDPtr( IDPTR idptr );
		void SetBestBvIDPtr( IDPTR IDPtr ) { m_IDPtrBestBV = IDPtr; }
		void SetDp( double Dp );
		void SetSetting( double setting );
		void SetDpSignal( double dpsignal );
		void SetQ( double q );
		void SetValidtyFlags( unsigned short usValidityFlags ) { m_usValidityFlags = usValidityFlags; }

		bool ForceBVSelection( IDPTR DeviceIDptr );

		// Return Version, connection, PN available in the suggestion list mapValves
		int GetAvailableVersions( std::map<int, CDB_StringID *> *pmap );
		int GetAvailableConnections( std::map<int, CDB_StringID *> *pmap );
		int GetAvailablePN( std::map<int, CDB_StringID *> *pmap );

		// User Change Type, connection, ...
		void SetpSelBVTypeAccordingUserChoice( CDB_StringID *pSelBVType );
		void SetpSelBVConnAccordingUserChoice( CDB_StringID *pSelBVConn );
		void SetpSelBVVersAccordingUserChoice( CDB_StringID *pSelBVVers );
		void SetpSelBVPNAccordingUserChoice( CDB_StringID *pSelBVPN );

		IDPTR GetDp2MVabv3kPa( double &dMaxDp );
		bool IsCompletelyDefined();
		void ResizeBv( double Havail );
		bool IsBestBv( CData *pData );
		bool IsKvSignalEquipped();
		double GetTechParamMinDp();
		eBool3 CheckDpMinMax( bool bSignal = false );
		eBool3 CheckMinOpen();
		CString GetSettingStr( bool bWithUnit = false, bool *pbFullSetting = NULL );
		void InvalidateSelection();

		//TA-SCOPE Transfert
		bool FillBvInfoIntoHMX( CDS_HydroModX *pHMX );
		
		// Verify flow, temp, ... compatibility for each computed device.
		int CheckValidity();

		virtual void DropData( OUTSTREAM outf, int *piLevel );

		std::multimap <int, CValvesData > *GetpMapValve() { return &m_mapValves; }
		bool IsBvExistInPreselectedList( CDB_TAProduct *pclBalancingValve );

		void ClearUserChoiceTemporaryVariables();


	protected:
		// Saved variables
		IDPTR m_IDPtr;						// IDPtr of selected valve in TADB
		IDPTR m_DiversityIDPtr;				// IDPtr of selected valve w/o diversity
		IDPTR m_IDPtrBestBV;				// IDPtr of best selected valve in TADB
		double m_dQ;							// used to store Q, needed if m_IDPTR is null (no valid selection found
		double m_dDp, m_dDpSignal, m_dSetting;	// Added to store data locally and to remove reference to CDS_Selp.. in the futur
		// Work variables.
		eHMObj m_eLocated;
		CDB_StringID *m_pUserChoiceBVType, *m_pUserChoiceBVConn, *m_pUserChoiceBVVers, *m_pUserChoiceBVPN;

		unsigned short m_usValidityFlags;		// Combination from eValidityFlags

		std::multimap <int, CValvesData > m_mapValves;
		void Write( OUTSTREAM outf );
		bool Read( INPSTREAM inpf );
		void _SelectBestBV( double Dp );
		void _SetpSelBV( LPARAM pTaDbBV, bool bForceLocker = false );
	};

	//********************** Sub-CLASS
	//
	class CDpC : public CBase
	{
		friend class CDS_HydroMod;
	public:
		CDpC( CDS_HydroMod *pHM );
		~CDpC();

		// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
		// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
		void Copy( CDS_HydroMod::CDpC *pclTargetHMDpC, bool bResetDiversityFactor = true );

		std::multimap <int, CDpCData > *GetpMapDpC() { return &m_mapDpC; }

		IDPTR GetIDPtr();
		CDB_TAProduct *GetpTAP() { return dynamic_cast<CDB_TAProduct *>( this->GetIDPtr().MP ); }
		eHMObj GetHMObjectType() { return eHMObj::eDpC; }

		void SetIDPtr( IDPTR idptr );
		IDPTR GetDiversityIDPtr();
		void SetDiversityIDPtr( IDPTR val ) { m_DiversityIDPtr = val; }

		unsigned short GetValidtyFlags() const { return m_usValidityFlags; }
		void SetValidtyFlags( unsigned short usValidityFlags ) { m_usValidityFlags = usValidityFlags; }

		double GetDpL();
		double GetDplmin();
		double GetDplmax();
		double GetDpmin( bool fBounds = true );
		double GetQ();
		double GetDp();
		double GetDpCSetting( CDB_DpController *pDpC = NULL );

		// Return Type, connection, PN available in the suggestion list mapValves
		int GetAvailableType( std::map<int, CDB_StringID *> *pmap );
		int GetAvailableConnections( std::map<int, CDB_StringID *> *pmap );
		int GetAvailablePN( std::map<int, CDB_StringID *> *pmap );

		CDB_StringID *GetpSelDpCType();
		CDB_StringID *GetpSelDpCConn();
		CDB_StringID *GetpSelDpCPN();

		IDPTR GetBestDpCIDPtr();

		void SetQ( double q );
		void SetBestDpCIDPtr( IDPTR IDPtr ) {m_IDPtrBestDpC = IDPtr;}

		// If "bLowestDplmin" is true, it means that we don't want a particular differential pressure drop to stabilise
		// but the Dp controller that fit with the flow and that has the lowest Dplmin.
		// For more details about "bLowestDplmin" see comments in the "CDS_Hm3WInj::ComputeHM" for explanation.
		
		// HYS-1930: If "bBypassStabilized" if true, it means that we are working with the auto-adative variable flow with
		// decoupling bypass. This variable is set to "true" when we are distributing the H available at the entry of the
		// circuit (For example when going from the pump in the compute all process). See the "CDS_HydroMod::SetHavail" method
		// for more description.
		void SelectBestDpC( double dHAvailable = 0.0, bool bLowestDplmin = false, bool bBypassStabilized = false );
		bool ForceDpCSelection( IDPTR DeviceIDptr );

		// User Change Combo selection
		void SetpSelDpCTypeAccordingUserChoice( CDB_StringID *pSelDpCpType );
		void SetpSelDpCConnAccordingUserChoice( CDB_StringID *pSelDpCpConn );
		void SetpSelDpCPNAccordingUserChoice( CDB_StringID *pSelDpCpPN );

		void SetMvLoc( eMvLoc MvLoc );
		void SetMvLocLocked( bool bLock ) {if( m_fMvLocLocked != bLock ) {m_fMvLocLocked = bLock; m_pParent->Modified();}}
		bool HasMvLockLocked() {if( m_fMvLocLocked ) { return true; } return false;}

		void ResizeDpC( double dHAvailable );
		bool IsCompletelyDefined();
		bool IsBestDpC( CData *pData );
		bool GetKvsBounds( double *LowestKvs, double *HighestKvs, double *LowestDpl, double *HighestDpl );
		double GetDpToStab( eMvLoc MvLoc = eMvLoc::MvLocLast, bool bToPump = false );
		eMvLoc GetMvLoc() {return ( m_fMvLoc );}
		eBool3 CheckDpMinMax();
		eBool3 CheckDpLRange( CDB_DpController *pDpC = NULL );
		CString GetSettingStr( bool bWithUnit = false, bool *pfFullSetting = NULL );

		// TA-SCOPE transfer
		bool FillDpCInfoIntoHMX( CDS_HydroModX *pHMX );
		// Verify flow, temp, ... compatibility for each computed device
		int CheckValidity();

		virtual void DropData( OUTSTREAM outf, int *piLevel );
		void ClearUserChoiceTemporaryVariables();

		bool IsDpCExistInPreselectedList( CDB_TAProduct *pclDpControllerValve );
		void InvalidateSelection();

	protected:
		// Saved variables
		IDPTR m_IDPtr;						// DpC IDPtr in case of dynamic balancing
		IDPTR m_DiversityIDPtr;				// IDPtr of selected valve w/o diversity
		IDPTR m_IDPtrBestDpC;				// IDPtr of best selected DpC in TADB
		IDPTR m_IDPtrSelMv;					// Selected Mv

		// work variables
		CDB_StringID *m_pUserChoiceDpCType, *m_pUserChoiceDpCConn, *m_pUserChoiceDpCPN;

		eMvLoc m_fMvLoc;
		bool m_fMvLocLocked;
		double m_dQ;
		unsigned short m_usValidityFlags;		// Combination from eValidityFlags

		std::multimap <int, CDpCData > m_mapDpC;

		void Write( OUTSTREAM outf );
		bool Read( INPSTREAM inpf );
		void ResizeBvForMvLoc();
		void ResetMVLoc();
		void _SetpSelDpC( LPARAM pTaDbDpC, bool bForceLocker = false );
		void _AbortSelectBestDpC();
	};

	//********************** Sub-CLASS
	//
	class CCv : public CBase
	{
		friend class CDS_HydroMod;

	public:
		CCv( CDS_HydroMod *pHM, CvLocation eCvLocated );
		~CCv();

		// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
		// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
		void Copy( CDS_HydroMod::CCv *pclTargetHMCv, bool bResetDiversityFactor = true );

		unsigned short GetValidtyFlags() const { return m_usValidityFlags; }
		void SetValidtyFlags( unsigned short usValidityFlags ) { m_usValidityFlags = usValidityFlags; }

		void SetDescription( CString description ) { if( description != m_strDescription ) { m_strDescription = description; m_pParent->Modified(); } }
		void SetCvLocate( CvLocation eCvLocated )	{ if( eCvLocated != m_eCvLocated ) { m_eCvLocated = eCvLocated; m_pParent->Modified(); } }
		void SetnWay( eCVnWay nway ) {if( nway != m_eWay ) {m_eWay = nway; m_pParent->Modified();}}
		void SetKvs( double kvs );
		void SetCtrlType( CDB_ControlProperties::CvCtrlType CtrlType );
		void SetTACv( bool bTACv ) {if( m_bTACv != bTACv ) {m_bTACv = bTACv; m_pParent->Modified();}}

		void SetQ( double Q, bool bforced = false );
		void SetKvsmax( double Kvsmax );
		void SetSetting( double dSetting );
		void SetCvIDPtr( IDPTR idptr );
		void SetBestKvs( double Kvs, bool bWithRounding = true);
		void SetBestCvIDPtr( IDPTR IDPtr ) {m_IDPtrBestCV = IDPtr;}
		void SetCVLocAuto(int val) { m_bCVLocAuto = val; m_pParent->Modified(); }
		void SetCVSelectedAsaPackage( eBool3 flag );

		void ForceCVSelection( IDPTR DeviceIDptr );
		
		// User Change Combo selection
		void SetpUserChoiceCVConn( CDB_StringID *pSelCVConn );
		void SetpUserChoiceCVVers( CDB_StringID *pSelCVVers );
		void SetpUserChoiceCVPN( CDB_StringID *pSelCVPN );

		void SetpSelCV( LPARAM pSelCV, bool bForceLocker = false );

		bool FillCvInfoIntoHMX( CDS_HydroModX *pHMX );

		CvLocation GetCvLocate() { return m_eCvLocated; }
		CString GetDescription() { return m_strDescription; }
		// For TA presetting valve return Kv value for current presetting
		double GetKv();
		double GetKvs();
		double GetKvsmax();
		double GetQ();
		double GetDp();
		double GetDpMin( bool bBounds = true );
		double GetAuth( bool bDesign = false );
		double GetBestKvs() {return m_dBestKvs; }
		eCVnWay GetnWay() { return m_eWay; }
		CDB_ControlProperties::CvCtrlType GetCtrlType() { return m_eControlType; }
		IDPTR GetBestCvIDPtr();
		IDPTR GetCvIDPtr();
		IDPTR GetTAPIDPtr() { return GetCvIDPtr(); }
		CDB_TAProduct *GetpTAP() { return dynamic_cast<CDB_TAProduct *>( GetCvIDPtr().MP ) ; }
		// HYS-1734: Get location of each valve.
		eHMObj GetHMObjectType() { return eHMObj::eCV; }
		CDB_ControlValve *GetpCV();
		bool IsCVLocAuto() { return ( m_bCVLocAuto!=0 ); }
		eBool3 IsCVLocInPrimary();
		eBool3 GetCVSelectedAsaPackage() const { return m_bCVSelectedAsaPackage; }

		IDPTR GetDiversityIDPtr();
		void SetDiversityIDPtr( IDPTR val ) { m_DiversityIDPtr = val; }

		double GetSetting();

		CDB_StringID *GetpSelCVType();
		CDB_StringID *GetpSelCVConn();
		CDB_StringID *GetpSelCVVers();
		CDB_StringID *GetpSelCVBdy();
		CDB_StringID *GetpSelCVPN();

		// Return Version, connection, PN available in the suggestion list mapValves
		int GetAvailableVersions( std::map<int, CDB_StringID *> *pmap );
		int GetAvailableConnections( std::map<int, CDB_StringID *> *pmap );
		int GetAvailablePN( std::map<int, CDB_StringID *> *pmap );
		std::multimap <int, CCVData > *GetpMapCV() { return &m_mapCV; }

		CString GetSettingStr( bool bWithUnit = false, bool *pbFullSetting = NULL );
		eBool3 IsPICV();
		eBool3 IsDpCBCV();
		bool IsOn_Off();
		bool IsPresettable();
		bool IsTaCV() {return ( m_bTACv );}
		bool IsCompletelyDefined();
		bool IsBestCV( CData *pData );
		bool IsBestCV( double Kvs );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>	Resize presettable CV (as a BV). </summary>
		/// <param name="Havail">	The H available. </param>
		/// <param name="fTryToFindSamllestValve">	True if we want to resize the valve! be carefull with DpC to fall down in a infinite loop. </param>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		void ResizePresetCV( double Havail, bool fTryToFindSamllestValve );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>	Size control valve. </summary>
		/// <param name="HavailAtPI">	The H avail at pressure interface. </param>
		/// <returns>	H needed at pressure inteface. </returns>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		double SizeControlValveForBetaMin(double HavailAtPI);
		
		// Sizing for pressure independant balancing & control valve and Dp combined, balancing & control valve.
		double SizePIBCVorDpCBCV( double dHAvailAtPI );

		void Size3WCvCstFlow( double dDp, bool fFromPump );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>	Size PICV. </summary>
		/// <remarks>	Select a PIVC; Alen, 14/09/2010. </remarks>
		/// <param name="HavailAtPI">	The H avail at pressure interface. </param>
		/// <param name="pMap">			[in,out] If non-null, the map. </param>
		/// <returns>	Min Dp for best CV. </returns>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		double SelectBestPICV( double HavailAtPI, std::multimap<double, CDB_PIControlValve *> *pMap = NULL );

		// For the moment, it is exactly the same function as 'SelectBestPICV'.
		double SelectBestDpCBCV( double HavailAtPI, std::multimap<double, CDB_DpCBCValve *> *pMap = NULL );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>	Invalidate selection. </summary>
		///
		/// <remarks>	Clean selected product; Alen, 14/09/2010. </remarks>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		void InvalidateSelection();
		eBool3 CheckMinAuthor( bool bDesign = false );
		eBool3 CheckDpMinMax();
		eBool3 CheckMinOpen();

		// Actuator dedicated functions
		// Extract list of available actuators, take into account current selected CV and H avail at the pressure interface
		// return the number of actuator available
		// if pRkList exist fill it with actuators found
		int GetCOCCompliantActuatorList( std::multimap< double, CDB_Actuator *> *pActrList, bool bWorkingForASet = true );

		void ForceActuatorSelection( CDS_HydroMod::CCv *pHMCv );
		void RemoveActuator();
		void SetActrIDPtr( IDPTR idptr );
		void SetActrAdaptIDPtr( IDPTR idptr );
		void SetActrSelectedVoltageIDPtr( IDPTR idptr );
		void SetActrSelectedSignalIDPtr( IDPTR idptr );
		void SetActrSelectedAsaPackage( eBool3 flag );
		void SetActrSelectionAllowed( eBool3 val );
		// Select actuator associated to the CV
		CDB_Actuator *SelectActuator();

		// HYS-1868: This function is created to check if the actuator or adapter are always 
		// compatible with the valve. There are old projects with wrong actuator or adapter and
		// when we open them if the valve is locked no compute is done and the wrong actuator, adapter
		// stays with the valve. The in/output parameters return true if the compatibility is there.
		void VerifyActuatorAndAdapter( bool *bActrOK, bool *bAdaptOk );
		IDPTR GetActrIDPtr();
		IDPTR GetActrAdaptIDPtr( );
		IDPTR GetActrSelectedVoltageIDPtr( );
		IDPTR GetActrSelectedSignalIDPtr( );
		eBool3 GetActrSelectedAsaPackage() const { return m_bActrSelectedAsaPackage; }
		eBool3 GetActrSelectionAllowed() const { return m_bActrSelectioAllowed; }
		
		void AddCVAccIDPtr( IDPTR IDPtr );
		void AddActrAccIDPtr( IDPTR IDPtr );
		void AddCVActrAccSetIDPtr( IDPTR IDPtr );
		int GetCVAccCount( void ) { return m_ArCVAccIDPtr.GetCount(); }
		int GetActrAccCount( void ) { return m_ArActrAccIDPtr.GetCount(); }
		int GetCVActrAccSetCount( void );
		IDPTR GetCVAccIDPtr( int iIndex );
		IDPTR GetActrAccIDPtr( int iIndex );
		IDPTR GetCVActrAccSetIDPtr( int iIndex );
		CArray <IDPTR> *GetpCVAccIDPtrArray() { return &m_ArCVAccIDPtr; }
		CArray <IDPTR> *GetpActrAccIDPtrArray() { return &m_ArActrAccIDPtr; }
		CArray <IDPTR> *GetpCVActrAccSetIDPtrArray();
		void ClearCVAccIDPtrArray( void ) { m_ArCVAccIDPtr.RemoveAll(); }
		void ClearActrAccIDPtrArray( void ) { m_ArActrAccIDPtr.RemoveAll(); }
		void ClearCVActrAccSetIDPtrArray( void ) { m_ArCVActrAccSetIDPtr.RemoveAll(); m_bCVActrAccSetRefreshed = false; }
		
		// Verify flow, temp, ... compatibility for each computed device
		int CheckValidity();
		// HYS-1685: This function check only for actors with CDB_CloseOffChar::CloseOffDp. KTM for example are not considered
		bool IsActuatorStrongEnough();

		// Methods for combined Dp controller, control and balancing valve (TA-COMPACT-DP).
		eBool3 CheckDpLRange( CDB_DpCBCValve *pDpCBCValve = NULL );
		double GetDpToStab( void );

		virtual void DropData( OUTSTREAM outf, int *piLevel );

		void ClearUserChoiceTemporaryVariables();

		bool IsCvExistInPreselectedList( CDB_ControlValve *pclControlValve );

	// Protected members.
	protected:		
		void Write( OUTSTREAM outf );
		bool Read( INPSTREAM inpf );
		// Return the first Reynard value below dVal
		double GetReynardVal( double dVal, bool fAbove = false );
		void _BuildSortingKeys (CTAPSortKey* psKeyTechParamBlw65, CTAPSortKey* psKeyTechParamAbv50);
		double _SelectBestTACV( double dKvs, bool fFromPump );
		void _VerifyCVActrAccSet( void );

		// Helpers for the 'SizeControlValveForBetaMin' method.
		double _SizeControlValveForBetaMinHMwithDpConPrimary( double dHAvailAtPI );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>	Size kvs. </summary>
		/// <param name="dDp">			[in,out] The dp. </param>
		/// <param name="bFromPump">	true in descending phase from the pump. </param>
		/// <returns>	Selected best Kvs. </returns>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Helper for all methods that need to size the control valve.
		double _SizeKvs( double &dDp, bool bFromPump );

	// Protected variables.
	protected:
		CDB_MultiString	*m_pRHSerie;
		IDPTR m_IDPtrCv;							// IDPtr of selected valve in TADB
		IDPTR m_DiversityIDPtr;						// IDPtr of selected valve w/o diversity
		IDPTR m_IDPtrBestCV;						// IDPtr of best selected valve in TADB
		CvLocation m_eCvLocated;
		eBool3 m_bCVSelectedAsaPackage;				// True when the selection was done as a package.

		IDPTR m_ActrIDPtr;							// IDPTR for selected actuator.
		IDPTR m_ActrAdaptIDPtr;						// IDPTR for selected adapter.
		IDPTR m_ActrSelectedVoltageIDPtr;			// IDPTR for selected selected voltage
		IDPTR m_ActrSelectedSignalIDPtr;			// IDPTR for selected selected signal
		eBool3 m_bActrSelectedAsaPackage;			// True when the selection was done as a package
		eBool3 m_bActrSelectioAllowed;				// True when the selection can be automatically done.
		CArray <IDPTR> m_ArCVAccIDPtr;				// IDPTRs for selected CV accessories
		CArray <IDPTR> m_ArActrAccIDPtr;			// IDPTRs for selected ACTR accessories
		CArray <IDPTR> m_ArCVActrAccSetIDPtr;		// IDPTRs for selected CV-ACTR set accessories.
		bool m_bCVActrAccSetRefreshed;				// 'False' to signal that 'm_arCVActrAccSetIDPtr' is not yet refreshed.
		int m_bCVLocAuto;							// CV Localization in the circuit
		std::multimap<int, CCVData > m_mapCV;

		CString	m_strDescription;
		double m_dKvsMax, m_dKvs, m_dQ, m_dAuthority, m_dSetting;
		double m_dBestKvs;
		CDB_ControlProperties::CvCtrlType m_eControlType;				// 2:3 point, 1:On/Off, 0:modulating
		eCVnWay m_eWay;
		bool m_bTACv;
		double m_DpMin;						// DpMin of selected valve SelectManBv with unknown Dp (close to 3kPa)

		CDB_StringID *m_pUserChoiceCVConn, *m_pUserChoiceCVVers, *m_pUserChoiceCVPN;

		unsigned short m_usValidityFlags;		// Combination from eValidityFlags

	// Private members.
	private:
		void SetAuth( double Auth ) { m_dAuthority = Auth;	} // Deprecated function, kept for file compatibility
	};

	// Nothing for the moment, it's just to prepare the 'CSelProdPage6WayValve' class.
	class C6WayValve : public CBase
	{
		friend class CDS_HydroMod;

	public:
		C6WayValve( CDS_HydroMod *pHM );
		~C6WayValve() {}

		// 'bResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
		// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
		void Copy( CDS_HydroMod::C6WayValve *pHM6WayValve, bool bResetDiversityFactor = true );

		void Add6WayValveAccessoryIDPtr( IDPTR IDPtr );
		void Add6WayValveActuatorAccessoryIDPtr( IDPTR IDPtr );

		IDPTR Get6WayValveIDPtr();
		IDPTR Get6WayValveActuatorIDPtr();
		IDPTR Get6WayValveActuatorAdapterIDPtr();

		void Set6WayValveIDPtr( IDPTR idptr );
		void Set6WayValveActuatorIDPtr( IDPTR idptr );
		void Set6WayValveActuatorAdapterIDPtr( IDPTR idptr );

		int Get6WayValveAccessoryCount( void ) { return m_Ar6WayValveAccessoryIDPtr.GetCount(); }
		int Get6WayValveActuatorAccessoryCount( void ) { return m_Ar6WayValveActuatorAccessoryIDPtr.GetCount(); }
		IDPTR Get6WayValveAccessoryIDPtr( int iIndex );
		IDPTR Get6WayValveActuatorAccessoryIDPtr( int iIndex );
		CArray <IDPTR> *Getp6WayValveAccessoryIDPtrArray() { return &m_Ar6WayValveAccessoryIDPtr; }
		CArray <IDPTR> *Getp6WayValveActuatorAccessoryIDPtrArray() { return &m_Ar6WayValveActuatorAccessoryIDPtr; }
		
		// Protected members.
	protected:		
		void Write( OUTSTREAM outf );
		bool Read( INPSTREAM inpf );

	// Protected variables.
	protected:
		IDPTR m_IDPtr6WayValve;									// IDPtr of selected valve in TADB
		IDPTR m_IDPtr6WayValveActuator;							// IDPTR for selected actuator.
		IDPTR m_IDPtr6WayValveActuatorAdapter;					// IDPTR for selected adapter.

		CArray<IDPTR> m_Ar6WayValveAccessoryIDPtr;				// IDPTRs for selected CV accessories
		CArray<IDPTR> m_Ar6WayValveActuatorAccessoryIDPtr;		// IDPTRs for selected ACTR accessories
	};

	class CSmartControlValve : public CBase
	{
		friend class CDS_HydroMod;

	public:
		CSmartControlValve( CDS_HydroMod *pHM );
		~CSmartControlValve() {}

		// 'bResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
		// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
		void Copy( CDS_HydroMod::CSmartControlValve *pHMTargetHMSmartControlValve, bool bResetDiversityFactor = true );

		void SelectBestSmartControlValve();
		void InvalidateSelection();

		std::multimap <int, CSmartControlValveData > *GetpMapSmartControlValve() { return &m_mapSmartControlValve; }

		void AddSmartControlValveAccessoryIDPtr( IDPTR IDPtr );

		CDB_TAProduct *GetpTAP() { return dynamic_cast<CDB_TAProduct *>( this->GetIDPtr().MP ); }
		IDPTR GetIDPtr();
		IDPTR GetBestIDPtr();
		// HYS-1734: Get location of each valve.
		eHMObj GetHMObjectType() { return eHMObj::eSmartControlValve; }

		int GetAvailableBodyMaterial( std::map<int, CDB_StringID *> *pmap );
		int GetAvailableConnection( std::map<int, CDB_StringID *> *pmap );
		int GetAvailablePN( std::map<int, CDB_StringID *> *pmap );

		CDB_StringID *GetpSelBodyMaterial();
		CDB_StringID *GetpSelConnection();
		CDB_StringID *GetpSelPN();
		SmartValveLocalization GetSelLocalization() { return m_eUserChoiceLocalization; }

		void SetpSelBodyMaterialAccordingUserChoice( CDB_StringID *pSelBodyMaterial );
		void SetpSelConnectionAccordingUserChoice( CDB_StringID *pSelConnection );
		void SetpSelPNAccordingUserChoice( CDB_StringID *pSelPN );
		void SetSelLocalizationUserChoice( SmartValveLocalization eLocalization ) { m_eUserChoiceLocalization = eLocalization; }

		SmartValveControlMode GetControlMode() { return m_eControlMode; }
		SmartValveLocalization GetLocalization() { return m_eLocalization; }
		// HYS-1676: Get string location of the smart control valve
		CString GetStrLocalization();
		double GetQ() { return m_dQ; }
		double GetDp();
		double GetDpMin();
		double GetFlowMax();
		double GetPowerMax();
		double GetDT();
		CDB_ValveCharacteristic *GetpValveChar();
		unsigned short GetValidtyFlags() const { return m_usValidityFlags; }

		IDPTR GetDiversityIDPtr();
		void SetDiversityIDPtr( IDPTR val ) { m_DiversityIDPtr = val; }

		void SetLocalization( SmartValveLocalization eLocalization ) { if( eLocalization != m_eLocalization ) { m_eLocalization = eLocalization; m_pParent->Modified(); } }
		void SetControlMode( SmartValveControlMode eControlMode ) { if( eControlMode != m_eControlMode ) { m_eControlMode = eControlMode; m_pParent->Modified(); } }
		void SetIDPtr( IDPTR idptr );
		void SetBestIDPtr( IDPTR IDPtr ) { m_IDPtrBest = IDPtr; }

		// HYS-1914: Dp is equal to Dp min except if Havail at the entry of the circuit increases, in that case it's the Dp that takes the difference.
		void SetDp( double dDp );

		void SetQ( double dQ ) {ASSERT( dQ >= 0 ); if( m_dQ != dQ ) { m_dQ = dQ; m_pParent->Modified(); } }
		void SetPowerMax( double dPowerMax ) {ASSERT( dPowerMax >= 0 ); if( m_dPowerMax != dPowerMax ) { m_dPowerMax = dPowerMax; m_pParent->Modified(); } }
		void SetValidtyFlags( unsigned short usValidityFlags ) { m_usValidityFlags = usValidityFlags; }

		bool IsCompletelyDefined();
		void ResizeSmartControlValve( double dHAvailable );
		bool IsBestSmartControlValve( CData *pData );
		IDPTR GetTAPIDPtr() { return m_IDPtr; }

		int GetAccessoryCount( void ) { return m_ArAccessoryIDPtr.GetCount(); }
		IDPTR GetAccessoryIDPtr( int iIndex );
		CArray <IDPTR> *GetpAccessoryIDPtrArray() { return &m_ArAccessoryIDPtr; }

		bool FillInfoIntoHMX( CDS_HydroModX *pHMX, eHMObj HMObj );
		bool ForceSelection( IDPTR DeviceIDptr );

		// Verify flow, temp, ... compatibility for each computed device.
		int CheckValidity();

		virtual void DropData( OUTSTREAM outf, int *piLevel );
		void ClearUserChoiceTemporaryVariables();

		bool IsSmartControlValveExistInPreselectedList( CDB_TAProduct *pclSmartControlValve );

		// Retrieve all error messages in a vector. Each message occupies one position in the vector.
		std::vector<CString> GetErrorMessageList( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags );

		// Retrieve all error messages in a string. All messages are concatenated with a prefix and return line if asked.
		CString GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags, CString strPrefix = _T("- "), bool bReturnLine = true );
		
	// Protected members.
	protected:		
		void Write( OUTSTREAM outf );
		bool Read( INPSTREAM inpf );

	private:
		void _AbortSelectBest();
		void _SetpSelSmartControlValve( LPARAM pTADBSmartControlValve, bool bForceLocker = false );

	// Protected variables.
	protected:
		SmartValveLocalization m_eLocalization;
		SmartValveControlMode m_eControlMode;
		IDPTR m_IDPtr;									// IDPtr of selected smart control valve in TADB.
		IDPTR m_IDPtrBest;								// IDPtr of best selected smart control valve in TADB
		IDPTR m_DiversityIDPtr;							// IDPtr of selected smart control valve w/o diversity
		double m_dQ;									// Used to store Q
		double m_dPowerMax;
		double m_dDp;									// HYS-1914: Dp is equal to Dp min except if Havail at the entry of the circuit increases, in that case it's the Dp that takes the difference.

		// Work variables.
		CDB_StringID *m_pUserChoiceBodyMaterial;
		CDB_StringID *m_pUserChoiceConnection;
		CDB_StringID *m_pUserChoicePN;
		SmartValveLocalization m_eUserChoiceLocalization;
		unsigned short m_usValidityFlags;				// Combination from eValidityFlags
		
		std::multimap <int, CSmartControlValveData > m_mapSmartControlValve;
		CArray<IDPTR> m_ArAccessoryIDPtr;				// IDPTRs for selected smart control valve accessories

		std::map<CDS_HydroMod::eValidityFlags, CString>	m_mapErrorMessages;
	};

	class CSmartDpC : public CBase
	{
		friend class CDS_HydroMod;

	public:
		CSmartDpC( CDS_HydroMod *pHM );
		~CSmartDpC() {}

		// 'bResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
		// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
		void Copy( CDS_HydroMod::CSmartDpC *pHMTargetHMSmartDpC, bool bResetDiversityFactor = true );

		void SelectBest();
		void InvalidateSelection();

		std::multimap <int, CSmartDDpCData > *GetpMapSmartDpC() { return &m_mapSmartDpC; }

		void AddDpSensorIDPtr( IDPTR IDPtr );
		void AddProductSetIDPtr( IDPTR IDPtr );

		void AddAccessoryIDPtr( IDPTR IDPtr );
		void AddDpSensorAccessoryIDPtr( IDPTR IDPtr );
		void AddSetContentIDPtr( IDPTR IDPtr );

		void ClearAccessoryIDPtrArray( void ) { m_ArAccessoryIDPtr.RemoveAll(); }
		void ClearDpSensorAccessoryIDPtrArray( void ) { m_ArDpSensorAccessoryIDPtr.RemoveAll(); }
		void ClearSetContentIDPtrArray( void ) { m_ArSetContentIDPtr.RemoveAll(); m_bSetContentRefreshed = false; }

		CDB_TAProduct *GetpTAP() { return dynamic_cast<CDB_TAProduct *>( this->GetIDPtr().MP ); }
		CDB_SmartControlValve *GetpSmartDpC();
		IDPTR GetIDPtr();
		IDPTR GetBestIDPtr();
		// HYS-1734: Get location of each valve.
		eHMObj GetHMObjectType() { return eHMObj::eSmartDpC; }

		int GetAvailableBodyMaterial( std::map<int, CDB_StringID *> *pmap );
		int GetAvailableConnection( std::map<int, CDB_StringID *> *pmap );
		int GetAvailablePN( std::map<int, CDB_StringID *> *pmap );

		CDB_StringID *GetpSelType();
		CDB_StringID *GetpSelBodyMaterial();
		CDB_StringID *GetpSelConnection();
		CDB_StringID *GetpSelPN();
		SmartValveLocalization GetSelLocalization() { return m_eUserChoiceLocalization; }

		void SetpSelBodyMaterialAccordingUserChoice( CDB_StringID *pSelBodyMaterial );
		void SetpSelConnectionAccordingUserChoice( CDB_StringID *pSelConnection );
		void SetpSelPNAccordingUserChoice( CDB_StringID *pSelPN );
		void SetSelLocalizationUserChoice( SmartValveLocalization eLocalization ) { m_eUserChoiceLocalization = eLocalization; }

		SmartValveLocalization GetLocalization() { return m_eLocalization; }
		// HYS-1676: Get string location of the smart differential pressure controller.
		CString GetStrLocalization();
		double GetQ() { return m_dQ; }
		double GetDp();
		double GetDpMin();
		double GetDpToStabilize();
		double GetDplmin();
		double GetDplmax();
		double GetFlowMax();
		double GetDT();
		CDB_ValveCharacteristic *GetpValveChar();
		unsigned short GetValidtyFlags() const { return m_usValidityFlags; }

		IDPTR GetDiversityIDPtr();
		void SetDiversityIDPtr( IDPTR val ) { m_DiversityIDPtr = val; }

		void SetLocalization( SmartValveLocalization eLocalization ) { if( eLocalization != m_eLocalization ) { m_eLocalization = eLocalization; m_pParent->Modified(); } }
		void SetIDPtr( IDPTR idptr );
		void SetBestIDPtr( IDPTR IDPtr ) { m_IDPtrBest = IDPtr; }

		// HYS-1914: Dp is equal to Dp min except if Havail at the entry of the circuit increases, in that case it's the Dp that takes the difference.
		void SetDp( double dDp );

		void SetQ( double dQ ) {ASSERT( dQ >= 0 ); if( m_dQ != dQ ) { m_dQ = dQ; m_pParent->Modified(); } }
		void SetValidtyFlags( unsigned short usValidityFlags ) { m_usValidityFlags = usValidityFlags; }

		bool IsCompletelyDefined();
		void ResizeValve( double dHAvailable );
		bool IsBestValve( CData *pData );
		IDPTR GetTAPIDPtr() { return m_IDPtr; }

		int GetAccessoryCount( void ) { return m_ArAccessoryIDPtr.GetCount(); }
		IDPTR GetAccessoryIDPtr( int iIndex );
		CArray<IDPTR> *GetpAccessoryIDPtrArray() { return &m_ArAccessoryIDPtr; }

		int GetDpSensorAccessoryCount( void ) { return m_ArDpSensorAccessoryIDPtr.GetCount(); }
		IDPTR GetDpSensorAccessoryIDPtr( int iIndex );
		CArray<IDPTR> *GetpDpSensorAccessoryIDPtrArray() { return &m_ArDpSensorAccessoryIDPtr; }

		int GetSetContentCount( void );
		IDPTR GetSetContentIDPtr( int iIndex );
		CArray<IDPTR> *GetpSetContentIDPtrArray();

		IDPTR GetDpSensorIDPtr();
		CDB_DpSensor *GetpDpSensor();

		// By 'Product set' we mean 'Dp sensor set' and 'Connection set'.
		eBool3 IsSelectedAsaPackage() const { return m_bSelectedAsaPackage; }

		// Returns an IDPTR that either points on dp sensor set or is NULL if it's a connection set.
		IDPTR GetProductSetIDPtr();
		
		// Returns either a dp sensor set or a connection set.
		CDB_Product *GetpProductSet();

		// Return either a pointer on a Dp sensor set object or NULL if it's a connection set.
		CDB_DpSensor *GetpDpSensorSet();

		// Returns an IDPTR that either points on the corresponding Dp sensor in the Dp sensor set or _NULL_IDPTR if it's a connection set.
		IDPTR GetDpSensorIDPtrInDpSensorSet();

		// Returns either a pointer on the Dp sensor linked to the current Dp sensor set or NULL if it's a connection set.
		CDB_DpSensor *GetDpSensorInDpSensorSet();

		// Return the set itself that contains the smart differential pressure controller and the Dp sensor set (Or connection set).
		CDB_Set *GetpSet();

		void SetDpSensorIDPtr( IDPTR DpSensorIDPtr );

		void SetSelectedAsaPackage( eBool3 eFlag );
		void SetProductSetIDPtr( IDPTR ProductSetIDPtr );

		bool ForceSelection( IDPTR DeviceIDptr );

		// Verify flow, temp, ... compatibility for each computed device.
		int CheckValidity();

		virtual void DropData( OUTSTREAM outf, int *piLevel );
		void ClearUserChoiceTemporaryVariables();

		bool IsSmartDpCExistInPreselectedList( CDB_TAProduct *pclSmartDpC );

		typedef enum
		{
			eErrorMessageObject_SmartDpC,
			eErrorMessageObject_DpSensor,
		}ErrorMessageObject;

		// Add error messages.
		void AddErrorMessage( ErrorMessageObject eErrorMessageObject, CDS_HydroMod::eValidityFlags eValidityFlags, CString strErrorMessage );

		// Retrieve all error messages in a vector. Each message occupies one position in the vector.
		std::vector<CString> GetErrorMessageList( ErrorMessageObject eErrorMessageObject, CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags );

		// Retrieve all error messages in a string. All messages are concatenated with a prefix and return line if asked.
		CString GetErrorMessageString( ErrorMessageObject eErrorMessageObject, CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags, CString strPrefix = _T("- "), bool bReturnLine = true );
		
	// Protected members.
	protected:		
		void Write( OUTSTREAM outf );
		bool Read( INPSTREAM inpf );

	private:
		void _AbortSelectBest();
		void _SetpSelSmartDpC( LPARAM pTADBSmartDpC, bool bForceLocker = false );

		void _SelectBestDpSensor();

		// Allow to fill the set content in the 'm_ArSetContentIDPtr' if not yet done.
		void _VerifySetContent();

	// Protected variables.
	protected:
		SmartValveLocalization m_eLocalization;
		IDPTR m_IDPtr;									// IDPtr of selected smart differential pressure controller in TADB.
		IDPTR m_IDPtrBest;								// IDPtr of best selected smart differential pressure controller in TADB
		IDPTR m_DiversityIDPtr;							// IDPtr of selected smart differential pressure controller w/o diversity
		double m_dQ;									// Used to store Q
		double m_dDp;									// HYS-1914: Dp is equal to Dp min except if Havail at the entry of the circuit increases, in that case it's the Dp that takes the difference.

		// Work variables.
		CDB_StringID *m_pUserChoiceBodyMaterial;
		CDB_StringID *m_pUserChoiceConnection;
		CDB_StringID *m_pUserChoicePN;
		SmartValveLocalization m_eUserChoiceLocalization;
		unsigned short m_usValidityFlags;				// Combination from eValidityFlags
		
		std::multimap <int, CSmartDDpCData > m_mapSmartDpC;
		CArray<IDPTR> m_ArAccessoryIDPtr;				// IDPTRs for selected smart differential pressure controller accessories.
		CArray<IDPTR> m_ArDpSensorAccessoryIDPtr;		// IDPTRs for selected Dp sensor accessories.
		CArray<IDPTR> m_ArSetContentIDPtr;				// IDPTRs for selected smart differential pressure controller set content.
		bool m_bSetContentRefreshed;					// 'False' to signal that 'm_ArSetContentIDPtr' is not yet refreshed.
		eBool3 m_bSelectedAsaPackage;					// True when the selection was done as a package.
		IDPTR m_DpSensorIDPtr;							// IDPTRs for selected Dp sensor.
		IDPTR m_ProductSetIDPtr;						// IDPTRs for selected smart differential pressure controller set (Dp sensor set or connection set).

		typedef std::map<CDS_HydroMod::eValidityFlags, CString>	mapValidtyFlagsCString;
		std::map<ErrorMessageObject, mapValidtyFlagsCString> m_mapErrorMessages;
	};

// Public methods.
public:
	double GetHreq( CDS_HydroMod *pHMOrg );

	// Return Dp see by the primary side, when the circuit is 2WInjection Dp = 0;
	virtual double GetDpOnOutOfPrimSide();

	// HYS-1716: Allows to retrieve the list of the children sorted either by ascending order (bAscendingOrder = 'true')
	// regarding their position (1 -> n) or descending order (bAscendingOrder = 'false') (n -> 1).
	void GetAllChildren( std::map<int, CDS_HydroMod *> &mapChildrenList, bool bAscendingOrder = true );
	void VerifyModuleCircuitPipe( CPipes *pPipe );

	CString ConvertOldSchemeID( TCHAR *pBuf );

	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy() { CTable::OnDatabaseDestroy(); }

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	CDS_HydroMod( CDataBase *pDataBase, LPCTSTR tcsID );
	virtual ~CDS_HydroMod();

	// Return a pointer on the first parent HM that is either the root module, an hydraulic circuit with a DpC or
	// an injection circuit.
	CDS_HydroMod *GetpPressIntHM( std::vector<CDS_HydroMod *> *pHMListOntheWay = NULL );

	CDS_HydroMod *GetpRootHM();
	CDS_HydroMod *GetFirstHMChild();

	// HYS-1716: Allows to retrieve what is the design temperature interface that is driven the DESIGN temperatures for the
	// current hydraulic circuit. It can be an injection module, the root module or NULL if this method is called on the root.
	// Why the last case? Because if root module is an injection module, we can not send back the circuit itself. 
	// Otherwise the 'GetpOutWaterChar' method will return the water characteristic at the secondary side of the root module.
	// And in this case we want to know what is the DESIGN water characteristic and temperature for the root that are the global project setttings.
	CDS_HydroMod *GetpDesignTemperatureInterface();

	// HYS-1716: Allows to check if the current hydraulic circuit is a design temperature interface for the DESIGN temperatures (See above).
	bool IsDesignTemperatureInterface();

	CDS_ProjectParams *GetpPrjParam() const { return m_pPrjParam; }
	CDS_TechnicalParameter *GetpTechParam() const { return m_pTechParam; }
	
	CDS_HydroMod::ePressIntType GetPressIntType();

	// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
	// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
	virtual void Copy( CDS_HydroMod *pHMTarget, bool bValveDataOnly = false, bool bCreateLinkedDevice = true, bool bToClipboard = false, bool bResetDiversityFactor = true );
	virtual void Copy( CData *pTo );

	// Verify flow, temp, ... compatibility for each computed device
	int CheckValidity();

	// Import the HM argument as a child HM.
	CDS_HydroMod *ImportHydroMod( CDS_HydroMod *pHM, int iNewIndex );

	// Return true if this object is HydroMod or a HydroMod's Child.
	virtual bool IsHMFamily() { return true; }

	// Rename Hydromod, give a automatic name if current name is empty or the first char is '*' or bForceAuto is set.
	virtual CString RenameMe( bool bForceAuto = false );

	// Create BV if flag is true.
	void Init( IDPTR SchIDPtr, CTable *pclParent = NULL, bool bCreateBV = false );

	virtual CSelectedInfos *GetpSelectedInfos() { return NULL; }

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TA SCOPE related function.
	// Load all information from a CDS_HydroModX.
	// bImportInExistingHM: To specify that CDS_HydroMod already exist in the current opened project.
	// Return true if all information have been loaded correctly.
	bool LoadFromHMX( CDS_HydroModX *pHMX, CTable *pclParent, int iLevel, bool bImportInExistingHM );

	// Prepare a CDS_HydroMod from a CDS_HydroModX.
	// It deletes nonexistent valves, define water characteristics,
	// define a name and a position, a scheme, the pipes, the TU etc.
	// It doesn't define the valves!
	// bImportInExistingHM: To specify that CDS_HydroMod already exist in the current opened project.
	void CleanAndPrepareHM( CDS_HydroModX *pHMX, CTable *pclParent, int iLevel, bool bImportInExistingHM );

	// Transfer all measurements from a CDS_HydroModX into a CDS_HydroMod.
	void TransferMeasFromHMX( CDS_HydroModX *pHMX );

	// Transfer all valves properties from a CDS_HydroModX into a CDS_HydroMod.
	virtual void TransferValvesInfoFromHMX( CDS_HydroModX *pHMX, int iHMObjToCheck = eHMObj::eALL );

	// Return true if a HM could be send to the TASCOPE.
	bool IsAvailableForTASCOPE();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Return a pointer on the Owner table.
	CTable *GetOwnerTable();

	void EnableComputeHM( bool bComputedAllowed ) { m_bComputeAllowed = bComputedAllowed; }

	CPipes *GetpCircuitPrimaryPipe() { return m_pCircuitPrimaryPipe; }
	CPipes *GetpCircuitSecondaryPipe() { return m_pCircuitSecondaryPipe; }
	CPipes *GetpCircuitBypassPipe() { return m_pCircuitBypassPipe; }
	CAnchorPt::CircuitSide GetBypassPipeSide();

	// Remark: if hydromod is in direct return mode, only 'm_pDistrSupplyPipe' will contain valid data as usual.
	//         If it is reverse return mode, both variables are needed.
	CPipes *GetpDistrSupplyPipe() { return m_pDistrSupplyPipe; }
	CPipes *GetpDistrReturnPipe() { return m_pDistrReturnPipe; }

	// HYS-1716: Allow to retrieve the good pipes regarding of the product that is on it.
	CPipes *GetpPipe( eHMObj eProductLocation );

	// If user opens a tsp project that contains pipes that are not yet in HySelect, he has two choices: either import new pipes in the
	// 'PIPE_TAB' or temporary use these pipes that will be no more available as soon as HySelect is closed. For this second option, we
	// completely reset the 'PIPE_TAB', fill it with default HySelect pipes and pipes from tsp project. By 'filling' we mean, we create
	// new object and we insert it in 'PIPE_TAB'. That means all IDPTR.MP for pipe are no more correct.
	void UpdateAllPipesIDPtr();

	CDpC *GetpDpC() { return m_pDpC; }
	CCv *GetpCV() { return m_pCv; }
	CBV	*GetpBv() { return m_pBv; }
	CBV **GetppBv() { return &m_pBv; }
	CBV	*GetpBypBv() { return m_pBypBv; }
	CBV	**GetppBypBv() { return &m_pBypBv; }
	CBV	*GetpSecBv() { return m_pSecBv; }
	CBV	**GetppSecBv() { return &m_pSecBv; }
	CSmartControlValve *GetpSmartControlValve() { return m_pSmartControlValve; }
	CSmartControlValve **GetppSmartControlValve() { return &m_pSmartControlValve; }
	CSmartDpC *GetpSmartDpC() { return m_pSmartDpC; }
	CSmartDpC **GetppSmartDpC() { return &m_pSmartDpC; }
	void SetpDpC( CDpC *pclDpC ) { m_pDpC = pclDpC; }
	void SetpBv( CBV *pclBv ) { m_pBv = pclBv; }
	void SetpBypBv( CBV *pclBv ) { m_pBypBv = pclBv; }
	void SetpSecBv( CBV *pclBv ) { m_pSecBv = pclBv; }
	void SetpSmartControlValve( CSmartControlValve *pclSmartControlValve ) { m_pSmartControlValve = pclSmartControlValve; }
	void SetpSmartDpC( CSmartDpC *pclSmartDpC ) { m_pSmartDpC = pclSmartDpC; }
	CShutoffValve *GetpShutoffValve( eHMObj eLocated ) { if( eHMObj::eShutoffValveSupply == eLocated ) { return m_pShutoffValveSupply; } if( eHMObj::eShutoffValveReturn == eLocated ) { return m_pShutoffValveReturn; } return NULL; }
	CShutoffValve **GetppSV( eHMObj eLocated ) { if( eHMObj::eShutoffValveSupply == eLocated ) { return &m_pShutoffValveSupply; } if( eHMObj::eShutoffValveReturn == eLocated ) { return &m_pShutoffValveReturn; } return NULL; }
	CTermUnit *GetpTermUnit() { return m_pTermUnit; }
	CPump *GetpPump() { return m_pPump; }
	CDS_HydroMod *GetParent( void );

	// Return CMeasData* for a specific date NULL if doesn't exist.
	CDS_HydroMod::CMeasData *GetpMeasData( unsigned uiIndex );

	// Return CMeasData* for CBI NULL if doesn't exist.
	CDS_HydroMod::CMeasData *GetpMeasDataForCBI();

	// Return number of elements stored into DateTimeMap.
	unsigned GetMeasDataSize() { return m_MeasDataDateTimeVector.size(); }
	// HYS-1605: Try to get if there is measurements information.
	bool IsMeasurementExistsInProject( CDS_HydroMod* pRootModule );

	void SetCurrentMeasDataIndex( unsigned uIndex ) { m_uCurrentMeasDataIndex = uIndex; }
	unsigned GetCurrentMeasDataIndex() { if( m_uCurrentMeasDataIndex >= m_MeasDataDateTimeVector.size() ) { m_uCurrentMeasDataIndex = 0; } return m_uCurrentMeasDataIndex; }

	CString GetHMName() { return m_HMName; }
	CString GetDescription() { return m_strDescription; }
	int GetLevel() { return m_iLevel; }
	unsigned short GetUid() {return m_usUniqueID; }
	int GetPos() { return m_iPosition; }
	bool IsaModule() { return m_bModule; }
	bool HasaValve();
	CString GetDepecratedSelThingID() { return m_strDeprecatedSelThingID; }
	LPCTSTR	GetCBIValveID() { return m_CBIValveIDPtr.ID; }
	IDPTR GetCBIValveIDPtr();
	LPCTSTR GetCBIType() { return m_tcCBIType; }
	LPCTSTR GetCBISize() { return m_tcCBISize; }
	int GetDN50() { return m_iDN50; }
	int GetDN15() { return m_iDN15; }
	double GetQDesign() { return m_dQDesign; }
	double GetQ() {	return GetQDesign(); }
	virtual double GetQ( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );
	double GetHMPrimaryQ( );

	double GetDpDesign() { return m_dDpDesign; }
	double GetHAvail( CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_Primary, bool bHmin = false );
	double GetPresetting() { return m_dPresetting; }

	CDB_CircuitScheme *GetpSch() { return ( ( CDB_CircuitScheme * ) GetSchemeIDPtr().MP ); }
	CDB_CircSchemeCateg *GetpSchcat();
	enum_VDescriptionType GetVDescrType() { return m_eVDescrType; }
	double GetKvCv() { return m_dKvCv; }
	enum enum_TreatmentType GetTreatment() { return m_eTreatment; }
	LPARAM GetTreatmentLparam() { return m_lpTreatment; }
	CString GetRemL1() { return m_strRemL1; }
	int GetRemarkIndex() { return m_iRemarkIndex; }
	CString GetComment() { return m_strComment; }
	CString GetSchemeID() { return m_SchemeIDPtr.ID; }
	IDPTR GetSchemeIDPtr() { return m_SchemeIDPtr; }

	// Return corrected HMin, CV min Dp is replaced by real CV Dp.
	double GetHminForUserDisplay();
	// Analyze a module and recursively find index circuit
	// stop when it reach deeper index circuit
	CDS_HydroMod *FindIndexCircuit();
	bool AnalyzeVSPDpSensorPosition( CDS_HydroMod **pHMIndex, double &dRequestedDp );
	unsigned short GetLock() { return m_usLock; }
	ReturnType GetReturnType() const { return m_eReturnType; }

	// HYS-1716: This method allows to retrieve the water characteristic for a specific pipe.
	// Here we can determine the pipe with its type (Distribution or circuit), its location (Supply or return) and in the case
	// of a circuit pipe its side (Primary or secondary).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is the inherited class
	// that is called and not this one.
	// ATTENTION: this method for the distribution return pipe or the circuit return pipe at the primary side will always return the 
	// water characteristic for the COMPUTED return temperature! This computed temperature can be the same as the design one in some cases.
	// To have the design water characteristic for the distribution return pipe or the circuit return pipe at the primary side, call the 'GetpDesignWaterChar' method instead.
	virtual CWaterChar *GetpWaterChar( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );

	// HYS-1716: This method allows to retrieve the water characteristic for a pipe regarding the current product location.
	// This method is not virtual because we first check the circuit schematic and call the 'GetpWaterChar' method with the 
	// 'ePipeType', 'ePipeLocation' and 'eCircuitSide' that is virtual.
	// Remark: this method is only for product. Don't call it with "eCircuitPrimaryPipe", "eCircuitSecondaryPipe", "eDistributionSupplyPipe" or "eDistributionReturnPipe".
	CWaterChar *GetpWaterChar( eHMObj eProductLocation );

	// HYS-1716: This method allows the terminal unit to know what are the water characteristic at its inlet and outlet.
	bool GetWaterCharTU( CWaterChar &clWaterCharIN, CWaterChar &clWaterCharOUT );

	// HYS-1716: This method allows to return the DESIGN water characteristic and temperature that drives the current hydraulic circuit and not the COMPUTED one.
	// It is mostly used for the distribution and circuit at the primary side when building and initialiazing circuit.
	// The distribution return pipe and the circuit return pipe at the primary side can have a DESIGN temperature different from the 
	// COMPUTED one.
	// Let's take an example: with a simple module with distribution circuit, when user adds it, the temperatures (distribution pipe and circuit pipe at the primary side) 
	// are the same as the parent design temperature interface (The one that drives the design of the current hydraulic circuit). 
	// When user adds children, the return temperature in the distribution pipe for each child is computed when going to this parent. 
	// Now the temperature in the distribution return pipe and in the circuit return pipe become different of the DESIGN ones.
	// Param: 'pclDesignTemperatureInterfaceHM' -> if NULL, the concerned hydraulic circuit will search itself what is its design temperature interface.
	//										    -> if defined, we take this design temperature interface to know what is the DESIGN water characteristic of the current hydraulic circuit.
	// ATTENTION! This 'pclDesignTemperatureInterfaceHM' variable is just for optimisation. When we have the design temperature interface for the current hydraulic circuit, 
	// we give it to avoid to do a search. It's obviously impredictable if you give a design temperature interface that is not the one for the current hydraulic circuit.
	bool GetDesignWaterChar( CWaterChar &clWaterChar, CDS_HydroMod *pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation ePipeLocation );

	// HYS-1716: This method allows to retrieve the water characteristic at the inlet of the circuit on the supply or return pipe.
	// This method is not virtual because the entry of a circuit is always the same.
	// ATTENTION: this method for the circuit return pipe at the primary side will always return the water characteristic for the COMPUTED return temperature!
	// Of course, this computed temperature can be the same as the design one.
	// To have the IN design water characteristic for the circuit pipes at the primary side, call the 'GetInDesignWaterChar' method instead.
	CWaterChar *GetpInWaterChar( CAnchorPt::PipeLocation ePipeLocation );

	// HYS-1716: This method allows to retrieve the water characteristic at the outlet of the circuit on the supply or return pipe.
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is the inherited class
	// that is called and not this one.
	// ATTENTION: this method for the circuit return pipe at the primary side will always return the water characteristic for the COMPUTED return temperature!
	// This computed temperature can be the same as the design one in some cases.
	// To have the OUT design water characteristic for the circuit pipes at the primary side, call the 'GetpOutDesignWaterChar' method instead.
	virtual CWaterChar *GetpOutWaterChar( CAnchorPt::PipeLocation ePipeLocation );

	// This method allows to retrieve the temperature for a specific pipe.
	// Here we can determine the pipe with its type (Distribution or circuit), its location (Supply or return) and in the case
	// of a circuit pipe its side (Primary or secondary).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is the inherited class
	// that is called and not this one.
	// ATTENTION: this method for the distribution return pipe or the circuit return pipe at the primary side will always return the COMPUTED temperature!
	// This computed temperature can be the same as the design one in some cases.
	// To have the design temperature for the distribution pipes or the circuit pipes at the primary side, call the 'GetDesignTemperature' method instead.
	virtual double GetTemperature( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );

	// HYS-1716: Same remark as for the "GetDesignWaterChar" method.
	bool GetDesignTemperature( double &dTemperature, CDS_HydroMod *pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation ePipeLocation );

	// HYS-1716: It's to have access to the design temperatures that user set at the secondary side for an injection circuit.
	virtual double GetDesignCircuitSupplySecondaryTemperature() { return -273.15; }
	virtual double GetDesignCircuitReturnSecondaryTemperature() { return -273.15; }

	// HYS-1716: This method allows to retrieve the temperature at the inlet of the circuit on the supply or return pipe.
	// This method is not virtual because the entry of a circuit is always the same.
	// ATTENTION: this method for the circuit return pipe at the primary side will always return the COMPUTED return temperature!
	// This computed temperature can be the same as the design one in some cases.
	// To have the design temperature for the circuit pipes at the primary side, call the 'GetInDesignTemperature' method instead.
	double GetInTemperature( CAnchorPt::PipeLocation ePipeLocation );

	// HYS-1716: This method allows to retrieve the temperature at the outlet of the circuit on the supply or return pipe.
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is the inherited class
	// that is called and not this one.
	// ATTENTION: this method for the circuit return pipe at the primary side will always return the COMPUTED return temperature!
	// This computed temperature can be the same as the design one in some cases.
	// To have the design temperature for the circuit pipes at the primary side, call the 'GetOutDesignTemperature' method instead.
	virtual double GetOutTemperature( CAnchorPt::PipeLocation ePipeLocation );

	// HYS-1716: This method allow to retrieve the power for the current hydraulic circuit at the primary or secondary side.
	// Remark: This method is virtual because if we have an injection circuit and we want this side, it will the inherited method
	// that will be called and not the base one.
	virtual double GetPower( CAnchorPt::CircuitSide eCircuitSide );

	// HYS-1882: In the previous version, when the fluid characteristics changes, we have not the new information
    // in all pipes of the circuit. Some pipes are updated but others are not.
    // This function update all pipes with the correct project additive by begenning from the root.
    // Supply temperatures are also updated.
	void DistributeWaterCharacteristics( CDS_HydroMod* pclDesignTemperatureInterfaceHM );

	// HYS-1716: This method allows to parse all children of a design temperature interface and to update their water characteristics.
	void VerifyAllWaterCharacteristics( CDS_HydroMod *pclDesignTemperatureInterfaceHM );

	// HYS-1716: This method allows to verify water characteristic for the current hydraluic circuit.
	// Param: 'pclDesignTemperatureInterfaceHM': To know what is the supply temperature starting from this design temperature interface.
	// Param: 'pclPreviousCircuit': Allows to compute the return temperature.
	// Normally, this method must not be manually called. It must be called only by the 'VerifyAllWaterCharacteristic'
	// in charge to verify a design temperature interface and all its children.
	// This method is also called in the "CDS_HydroMod::_UpdateQDp" method.
	virtual void VerifyWaterCharacteristic( CDS_HydroMod *pclDesignTemperatureInterfaceHM = NULL, CDS_HydroMod *pclPreviousCircuit = NULL );

	CDB_Pipe *GetPipeSizeShift( int &iHighestSize, int &iLowestSize, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_Primary, 
			CAnchorPt::PipeLocation ePipeLocation = CAnchorPt::PipeLocation::PipeLocation_Undefined );

	CDB_ControlProperties::CvCtrlType GetCvCtrlType();
	void SetHMName( LPCTSTR tcsName ) { if( _tcscmp( tcsName, m_HMName ) ) { _tcsncpy_s( m_HMName, SIZEOFINTCHAR( m_HMName ), tcsName, _TABLE_NAME_LENGTH ); Modified(); } }
	void SetDescription( CString strDescription );
	void SetLevel( int iLevel ) { if( iLevel != m_iLevel ) { m_iLevel = iLevel; Modified(); } }
	void SetUid( unsigned short usUniqueID ) { if( m_usUniqueID != usUniqueID ) { m_usUniqueID = usUniqueID; Modified(); } }
	void SetPos( int iPosition ) { if( m_iPosition != iPosition ) { m_iPosition = iPosition; Modified(); } }
	void SetFlagModule( bool bModule );
	void SetCBIValveID( LPCTSTR tcsCBIValveID );
	void SetCBIType( LPCTSTR tcsType ) { if( _tcscmp( tcsType, m_tcCBIType ) ) { _tcsncpy_s( m_tcCBIType, SIZEOFINTCHAR( m_tcCBIType ), tcsType, SIZEOFINTCHAR( m_tcCBIType ) - 1 ); Modified(); } }
	void SetCBISize( LPCTSTR tcsSize ) { if( _tcscmp( tcsSize, m_tcCBISize ) ) { _tcsncpy_s( m_tcCBISize, SIZEOFINTCHAR( m_tcCBISize ), tcsSize, SIZEOFINTCHAR( m_tcCBISize ) - 1 ); Modified(); } }
	void SetQDesign( double dQDesign ) { if( m_dQDesign != dQDesign ) { m_dQDesign = dQDesign; Modified(); } }
	void SetDpDesign( double dDpDesign ) { if( m_dDpDesign != dDpDesign ) { m_dDpDesign = dDpDesign; Modified(); } }

	// "pbHasHAvailSecChanged" if defined will contain 'true' if Hmin of the secondary pump has changed.
	// Typically at now it is ony for 3-way mixing circuit.
	bool SetHavail( double dHAvail, CAnchorPt::CircuitSide eCircuitSide, bool bGoingToPump = false, bool *pbHasHAvailSecChanged = NULL );
	void SetPresetting( double dPresseting ) { if( m_dPresetting != dPresseting ) { m_dPresetting = dPresseting; Modified(); } }

	void SetVDescrType( enum_VDescriptionType eType ) { if( m_eVDescrType != eType ) { m_eVDescrType = eType; Modified(); } }
	void SetKvCv( double dKvCv ) { if( m_dKvCv != dKvCv ) { m_dKvCv = dKvCv; Modified(); } }
	void SetTreatment( enum enum_TreatmentType eTreatmentType ) { m_eTreatment = eTreatmentType; }
	void SetTreatmentLparam( LPARAM lpTreatment ) { m_lpTreatment = lpTreatment; }
	void SetRemL1( CString strRemL1 ) { if( m_strRemL1 != strRemL1 ) { m_strRemL1 = strRemL1; Modified(); } }
	void SetRemarkIndex( int iRemarkIndex );

	// It's just to help us to display correctly remarks and comment on the result page. This variable is not saved.
	void SetComment( CString strComment ) { m_strComment = strComment; }
	
	void SetSchemeID( CString strSchemID );
	void SetSchemeIDPtr( IDPTR SchemeIDPtr );
	void SetFlagCircuitIndex( bool fIsCircuitIndex );
	void SetLock( eHMObj eObj, bool bFlag, bool bResizeNow = true, bool bDontCareFreezed = false );
	void SetLock( unsigned short usLock ) { m_usLock = usLock; }
	void SetRecursiveLock( eHMObj eObj, bool bFlag, bool bResizeNow = true, bool bDontCareFreezed = false );
	void SetChecked( bool bChecked ) { m_bCheck = bChecked; }
	void SetHMCalcMode( bool bHMCalcMode ) { m_bHMCalcMode = bHMCalcMode; }
	// Update Q in each HM component;
	// this function DOES NOT launch an automatic recompute of the module
	virtual void SetHMPrimAndSecFlow( double dQ );
	void SetFlagConstantFlow( bool fIsConstantFlow ) { m_bConstantFlow = fIsConstantFlow; }
	void SetReturnType( ReturnType eReturnType );

	// HYS-1716: this method allows to set the water characteristic for a specific pipe.
	// Here we can determine the pipe with its type (Distribution or circuit), its location (Supply or return) and in the case
	// of a circuit pipe its side (Primary or secondary).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is first this base method
	// that is called and next the inherited one.
	virtual void SetWaterChar( CWaterChar *pclWaterChar, CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );

	// HYS-1716: this method allows to set the water characteristic for a set of pipes (For example when importing from TA-Scope).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is first this base method
	// that is called and next the inherited one.
	virtual void SetWaterChar( CWaterChar *pclWaterChar );

	// HYS-1716: this method allows to set the temperature for a specific pipe.
	// Here we can determine the pipe with its type (Distribution or circuit), its location (Supply or return) and in the case
	// of a circuit pipe its side (Primary or secondary).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is first this base method
	// that is called and next the inherited one.
	virtual void SetTemperature( double dTemperature, CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );

	//  Verify if the valve can be sent to the CBI
	//	return 	evi_ValveNotPresent,
	//			evi_ValvePresent,
	//			evi_Undetermined		in the case where the valve can't be found in TADB
	enum enum_ValveInCBI ValveIsInCBI( CString &strCBIVersion, int iCBICompat );

	// This function will complete the hydronic module to be readable in HMCalc mode.
	// This function is called typically when the user create an hydronic module from the CBI or in non hydrocalc mode.
	bool CompleteHMForHydroCalcMode( CTable *pclParent, bool bLock = true );

	// We can now specify which type of pipes we want to create or delete with 'eWhichPipe' argument.
	bool CreatePipes( bool bWithSingularity = true, int iWhichPipe = eHMObj::eAllPipes );
	void DeletePipes( int iWhichPipe = eHMObj::eAllPipes );

	bool CreateBv( eHMObj eBvLocated );
	void DeleteBv( CBV **pBv );
	void DeleteMeasData();
	CMeasData *CreateMeasData();
	bool CreateDpC();
	void DeleteDpC();
	bool CreateTermUnit();
	void DeleteTermUnit();
	bool CreatePump();
	void DeletePump();
	bool CreateCv( CvLocation eCvLocated );
	void DeleteCv();
	bool CreateSV( eHMObj eSVLocated );
	void DeleteSV( CShutoffValve **pSV );
	bool CreateSmartControlValve( SmartValveLocalization eSmartValveLocalization );
	void DeleteSmartControlValve();
	bool CreateSmartDpC( SmartValveLocalization eSmartValveLocalization );
	void DeleteSmartDpC();
	bool GetBalancingIndex( double *pdBalIndex );
	// To count recursively, let's 'iCount' parameter to '0'!
	int GetCount( bool bRecursive = false, bool bOnlyModule = false, int iCount = 0 );
	int GetValveCount( bool bRecursive = false );

	// Used to manage TASCOPE file exportation.
	unsigned int GetHMStatus() { return ( unsigned int ) m_ucModuleStatus; }
	bool HasHMStatus( unsigned int uiStatus ) { return ( ( m_ucModuleStatus & uiStatus ) ? true : false ); }
	void SetHMStatus( unsigned int uiStatus ) { m_ucModuleStatus = uiStatus; }

	bool IsChecked() { return m_bCheck; }

	// 'bAndSelected' = false -> the method returns 'true' only if a 'CDS_HydroMod::CBV' is defined.
	// 'bAndSelected' = true -> the method returns 'true' if a 'CDS_HydroMod::CBV' is defined AND if a valve is selected.
	bool IsBvExist( bool bAndSelected = false );
	bool IsBvBypExist( bool bAndSelected = false );
	bool IsBvSecExist( bool bAndSelected = false );
	bool IsDpCExist( bool bAndSelected = false );
	bool IsTermUnitExist();
	bool IsPressureInterfaceExist(); 						// Return true when a short cut bypass exist, for now linked to the pump
	virtual bool IsInjectionCircuit() { return false; }
	bool IsPumpExist();
	bool IsCvExist( bool bAndSelected = false );
	bool IsShutoffValveExist( eHMObj eLocated,  bool bAndSelected = false );
	bool IsShutoffValveExist( bool bAndSelected = false );
	bool IsSmartControlValveExist( bool bAndSelected = false );
	bool IsSmartDpCExist( bool bAndSelected = false );
	bool IsPipeExist( eHMObj ePipeLocation, bool bAndSelected = false );

	bool IsEmpty( void );
	bool IsHMCalcMode() { return m_bHMCalcMode; }

	// For a specific object, the function will return 'false' if:
	//		1) Product is not defined in the chosen circuit schematic.
	//		2) Corresponding hydromod object not created
	//		3) Corresponding hydromod object is created but not completely defined.
	//		4) Corresponding hydromod object is created, completely defined but not locked.
	bool IsLocked( eHMObj eObj );

	// If 'bExcludeCPForModule' is set to 'true' we exclude the circuit pipe of a module.
	bool IsAtLeastOneObjectLocked( bool bExcludeCPForModule );
	bool IsAtLeastOneObjectUnlocked( bool bExcludeCPForModule );
	bool IsAtLeastOneObjectLockedRecursive( bool bExcludeRoot, bool bExcludeCPForModule );
	bool IsAtLeastOneObjectUnlockedRecursive( bool bExcludeRoot, bool bExcludeCPForModule );

	eBool3 IsLockedTriState( eHMObj eObj );
	bool IsUnLocked( eHMObj eObj );
	bool IsConstantFlow() { return m_bConstantFlow; }

	bool IsPending();
	bool GetPending() const { return m_bPending; }
	void SetPending( bool val ) { m_bPending = val; }

	bool IsDiversityExist();

	// Return true if it's circuit index.
	bool IsCircuitIndex() { return m_bIsCircuitIndex; }

	// Verify pipe existence.
	bool IsPipeUsed( IDPTR IDPtrPipeSerie, IDPTR IDPtrPipeSize = _NULL_IDPTR );

	// Return IDPTR of TADB valve measured into the CBI.
	virtual IDPTR GetTADBValveIDPtr();

	virtual bool IsForHub() { return false; }


	// Compute flow for the current distribution pipe, take in count flow for terminal unit and flow for
	// sibling items with a greater index.
	double GetDistrTotQ();

	// Compute modules under the main pump, goto the pump, Localize each pressure interface and compute each branch.
	void ComputeAll( bool bResize = false );

	double GetDiversityFactor() const { return m_dDiversityFactor; }

	// Allow to specify that it's on this module that user wants to apply a diversity factor.
	// Remark: - Values must be set in the range ]0.0;1.0].
	//		   - All other values is equivalent to calling 'ResetDiversityFactor.
	void SetDiversityFactor( double dDiversityFactor );

	// Reset diversity factor if exist.
	void ResetDiversityFactor( void );

	// Clean recursively all data concerned by diversity factor ('m_dDiversityFactor' and all diversity applied flag in distribution pipe).
	void CleanAllDiversityInfos( CDS_HydroMod *pHM );

	void SaveDiversityValvesPipes( CDS_HydroMod *pHM );

    void FillVectorWithModulatingCV( CDS_HydroMod *pHM, std::multimap<int, CDS_HydroMod *> *pMMap, bool bForActuatorSelection = false );

	// After collect data on a CBI some product can be modified (Type, Size,...
	// Select a product in the database close to the.
	bool SolvePartDef();

	// HYS-1882: This function check if the DT field should be displayed in flow mode or not.
	bool IsDTFieldDisplayedInFlowMode();

	// Found correct schema
	virtual void ResetSchemeIDPtr();

	// Return MinDp needed for Proportionnal CV, based on technical parameters, this value can be overwritten in children class 
	virtual double GetTechParamCVMinDpProp();

	// Compute a new setting point for balancing device in function of Havail.
	// "pbHasHAvailSecChanged" if defined will contain 'true' if Hmin of the secondary pump has changed.
	// Typically at now it is ony for auto-adaptive flow with decoupling bypass.
	virtual void ResizeBalDevForH( double dHAvail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged = NULL );

	// HYS-1930: auto-adapting variable flow decoupling circuit.
	// In the first loop of the "_ComputeAllToThePressureInterface", we save in a vector all modules that are auto-adaptive flow with
	// decoupling bypass and for which a child circuit with CV that needs the H available at the secondary side of the auto-adaptife flow
	// must be changed to satisfay authority. In this case, we don't follow the normal procedure as for the other circuits.
	void ResizePumpSecFollowingCVAuthorityChanges( CDS_HydroMod *pHM );

	virtual double AdjustPumpHmin(){ return 0.0; }

	// Needed for 2wInj Circuit
	virtual bool CheckIfTempAreValid( int *piErrors = NULL, std::vector<CString> *pvecStrErrors = NULL );

	// Return true if all defined HM objects are defined.
	virtual bool IsCompletelyDefined( int iHMObjToCheck = eHMObj::eALL );

	// Spread any change for other elements.
	virtual void ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj = NULL );

	// Return current Dp for the circuit.
	virtual double GetDp( bool bDpMin = false );
	virtual double GetBypOverFlow() { return 0.0; }

	virtual double GetTotQwoDiversity( );

	virtual double GetPartialPendingQ( );

	virtual void ResizeShutOffValves();

	virtual void DropData( OUTSTREAM outf, int *piLevel );

	// Debugging.
	void SendEventToConsole( eComputeHMEvent event );
	bool BreakOnHMName( CString HMName, bool bBreakOnFailure = true );

	// Allow to inherited class ( ie: 'CDS_HmInj', 'CDS_Hm2WInj' and so no) to fill specific data.
	virtual void FillOtherInfoIntoHMX( CDS_HydroModX *pHMX ) {}

	// HYS-1716: This method is called when 'CDS_HydroMod::Init' is called. Typically it is when creating a project
	// and to update water characteristic of the different pipes of the current hydraulic circuit with its design temperature
	// interface.
	// This method is virtual because we need also to update circuit secondary pipes if they exist.
	// ATTENTION!!! 'pclParent' is not necessarily the design temperature interface. We need to verify before to use it.
	// If it's a design temperature interface, we can use it. Otherwise, we retrieve the design temperature interface for this
	// 'pclParent' itself. If 'pclParent' is NULL, we will take the general settings.
	virtual void InitAllWaterCharWithParent( CTable *pclParent, eHMObj eWhichPipe = eHMObj::eALL );

	// Retrieve all error messages in a vector. Each message occupies one position in the vector.
	std::vector<CString> GetErrorMessageList( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags );

	// Retrieve all error messages in a string. All messages are concatenated with a prefix and return line if asked.
	CString GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags, CString strPrefix = _T("- "), bool bReturnLine = true );

	void ClearErrorMessage( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags );

// Protected methods.
protected:
	// Allow to inherited class ( ie: 'CDS_HmInj', 'CDS_Hm2WInj' and so no) to fill specific data.
	virtual void TransferOtherInfoFromHMX( CDS_HydroModX *pHMX ) {}
	void _ProcessCircuitPipes();
	void _ProcessBypassPipe();

// Private methods
private:
	void _SetComputeHMEngine( eComputeHMEvent event, LPARAM lparam = 0 ) { m_ComputeHMState = event; m_ComputeHMParam = lparam; }
	void _VerifyConnection( CDS_HydroMod *pHM, CDS_HydroMod *pParentHM );
	void _UpdateTUDpAccordingHavailable();
	void _UpdateHAvailAccordingTUDp( CDS_HydroMod *pHM );
	
	// Compute requested H for the HM,take in count pipe Dp on the way to the pump.
	double _GetHreqPressInt();
	
	// Update Q through distribution pipe, resize balancing device if needed.
	void _ComputeAllToThePump( bool bResize, bool bResizeForPendingCircuit, CDS_HydroMod *pOrgHM );
	
	// Distribute Havail, resize CV, resize balancing device.
	bool _AdjustPumpHminForModulatingCV();
	bool _ResizeCv( );
	
	// For a module compute children total flow, compute distributed Q (children distribution pipe)
	// compute and refresh available Dp for each children return total flow.
	void _UpdateQDp( bool bResize, bool bResizeForPendingCircuit, bool OnlyUpdateHNeededAtTheCircuitInterface );
	
	void _UpdateDp( std::map<int, CDS_HydroMod *> *pmapChildren, bool bResizeForPendingCircuit, double *dHUpStream, double *dHNeedCurrent, bool bResizeToFullOpening );
	
	void _DistributeHAvailable( CTable *pPHM, double dHAvail, bool bResizeForPendingCircuit = false );
	
	void _AdjustDpAfterCVResizing( CDS_HydroMod *pOrgHM );

	void _VerifyPumpHeadUser( CDS_HydroMod *pclHydroMod, bool *pbHasHuserChanged = NULL );

	// Allows to set the 'Diversity applied' flag in distribution pipes from the 'pStartHM' to the root module.
	// Remark: Don't fill 'pHM' for the first call.
	//         For example, if user wants to apply 0.65 on *A.3.2, pipes will be flagged on the following modules: *A.3.2; *A.3.1; *A.3 and *A.
	void _SetPipeDiversityFlag( CDS_HydroMod *pStartHM, bool bDiversityFlag, CDS_HydroMod *pHM = NULL );
	void ResetHavailRecursively( );

	bool _UpdateDPCDp( CDS_HydroMod *pHM, double dHPressIntSaved, double dNewHAtPressureInt );
	void _TakeSnapShot( CDS_HydroMod *pHM, CString str, int iDeep = 0 );

	// HYS-1716: This method is specific for the treatment of temperatures in hydraulic circuits.
	CDS_HydroMod *_GetpPressIntHMForTemperature();

	void _ComputeAllPressureInterface( CTableHM *pPipingTab, bool bResize, bool bForDiversity, bool bForPending );

// Public variables.
public:
	CTable *m_pSingulTab;
	CTableDN *m_pclTableDN;

// Protected variables.
protected:
	TCHAR m_HMName[_TABLE_NAME_LENGTH + 1];			//!< Module name (automatically built or user given)
	CString m_strDescription;						//!< Module description
	int m_iLevel;									//!< Tree level (root is 0)
	unsigned short m_usUniqueID;					//!< Unique identifier of the valve in the project (start at 1, till 65535)
	int m_iPosition;								//!< Valve's position in parent module
	bool m_bModule;									//!< true if it's a module, false when just a Valve (no children)
	bool m_bPending;								//!< Valve marked as pending circuit
	CString m_strDeprecatedSelThingID;				//!< ID of selected object in TADB
	TCHAR m_tcCBIType[_CBI_TYPE_LENGTH + 1];		//!< Type of the valve as defined in CBI
	TCHAR m_tcCBISize[_CBI_SIZE_LENGTH + 1];		//!< Size of the valve as defined in CBI
	double m_dQDesign;								//!< Design flow
	double m_dDpDesign;								//!< Design Dp
	double m_dPresetting;							//!< Presetting (from design)
	double m_dDiversityFactor;						//!< Diversity factor (0 to 1)
	enum_VDescriptionType m_eVDescrType;			//!< Valve "description"
	double m_dKvCv;									//!< Kv or Cv value defining the valve
	enum enum_TreatmentType m_eTreatment;			/**< Tmp Flag indicating the type of treatment applied to the valve
													 when matching plant structure from plant data collected from CBI
													 !!! NOT WRITTEN because only temporary used */
	IDPTR m_SchemeIDPtr;							//!< ID of selected scheme
	CString m_strRemL1;								//!< Remark line 1
	int m_iRemarkIndex;								//!< Remark Index used for visualisation; computed each time the table is displayed
	CString m_strComment;							//!< This variable is just to help us to fill remark and comment in the result page. This variable is not saved.
	bool m_bIsCircuitIndex;							//!< True if it's circuit index
	double m_dHAvail;								//!< H available at the HM entry (without distribution pipe )
	double m_dHPressInt;							//!< H pressure interface before distribution pipe (static circuit) after DpC in case of Dyn. Circuit
	unsigned short m_usLock;						//!< Locking flag one bit per HM component
	ReturnType m_eReturnType;						//!< Direct or reverse return.

	// Working variables
	bool m_bCheck;									// used display if a HM is selected or not
	bool m_bHMCalcMode;								// used to verify if the HM can switch into HMCalc mode
	bool m_bConstantFlow;
	CPipes *m_pCircuitPrimaryPipe;
	CPipes *m_pCircuitSecondaryPipe;
	CPipes *m_pCircuitBypassPipe;
	CPipes *m_pDistrSupplyPipe;
	CPipes *m_pDistrReturnPipe;
	IDPTR m_CBIValveIDPtr;
	CDpC *m_pDpC;
	CCv *m_pCv;
	CBV *m_pBv;
	CBV *m_pBypBv;
	CBV *m_pSecBv;
	CShutoffValve *m_pShutoffValveSupply;
	CShutoffValve *m_pShutoffValveReturn;
	CSmartControlValve *m_pSmartControlValve;
	CSmartDpC *m_pSmartDpC;
	CTermUnit *m_pTermUnit;
	CPump *m_pPump;
	bool m_bComputeAllInCourse;
	int m_iDN50;
	int	m_iDN15;
	LPARAM m_lpTreatment;
	unsigned m_uCurrentMeasDataIndex;				//!< Current index in MeasDataDateTimeVector
													//!< Used to manage TASCOPE file exportation
	unsigned char m_ucModuleStatus;
	CDS_ProjectParams *m_pPrjParam;
	CDS_TechnicalParameter *m_pTechParam;

	std::vector <CMeasData *> m_MeasDataDateTimeVector; //!< Create a vector that contain the measured value for each specified date&time
	
	bool m_bComputeAllowed;							//!< During copy of a hydromod; ComputeHM is desactivated

	// Will contain all error and/or warning messages for the current hydraulic circuit.
	// HYS-1930: Remark: Only implemented at now for the auto-adaptive with decoupling bypass circuit.
	// Or directly for the smart control valve and the smart differential pressure controller.
	std::map<CDS_HydroMod::eValidityFlags, CString>	m_mapErrorMessages;
	
// private variables.
private:
	eComputeHMEvent m_ComputeHMState;
	LPARAM m_ComputeHMParam;
};

////////////////////////////////////////////////////////////////////////////////////////////
class CDS_Hm2W : public CDS_HydroMod
{
	DEFINE_DATA( CDS_Hm2W )

public:
	CDS_Hm2W( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_Hm2W() {}

	void Init( IDPTR SchIDPtr, CTable *plcParent = NULL, bool bUseTACV = false, bool bCreateLinkedDevice = true );

	// Compute a new setting point for balancing device in function of Havail.
	// "pbHasHAvailSecChanged" if defined will contain 'true' if Hmin of the secondary pump has changed.
	// Typically at now it is ony for auto-adaptive flow with decoupling bypass.
	virtual void ResizeBalDevForH( double Havail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged = NULL );

	// Spread any change for other elements
	virtual void ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj = NULL );

	// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
	// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
	virtual void Copy( CDS_Hm2W *pHM2W, bool bValveDataOnly = false, bool bCreateLinkedDevice = true, bool bToClipboard = false, bool fResetDiversityFactor = true );
	virtual void Copy(CData *pTo);


	void SetUseShutoffValve( eUseProduct eUseShutoffValve, eHMObj eWhichSV );
	eUseProduct GetUseShutoffValve() { return m_eUseShutoffValve; }
	eHMObj GetShutoffValveLocation() { return m_eSVObj; }

	// Return true if all defined HM objects are defined.
	virtual bool IsCompletelyDefined( int iHMObjToCheck = eHMObj::eALL );

	// Allow to inherited class ( ie: 'CDS_HmInj', 'CDS_Hm2WInj' and so on) to fill data concerning valves.
	virtual void TransferValvesInfoFromHMX( CDS_HydroModX *pHMX, int iHMObjToCheck = eHMObj::eALL );

protected:
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy() { CDS_HydroMod::OnDatabaseDestroy(); }

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

// Private variable.
private:
	eComputeHMEvent m_ComputeHM2WState;
	LPARAM m_ComputeHM2WParam;
	eUseProduct	m_eUseShutoffValve;		// Allow to memorize user choice.
	eHMObj m_eSVObj;
	IDPTR m_SavedSVIDPtr;

// Private methods.
private:
	void _SetComputeHM2WEngine( eComputeHMEvent event, LPARAM lparam = 0 ) { m_ComputeHM2WState = event; m_ComputeHM2WParam = lparam;}
};

////////////////////////////////////////////////////////////////////////////////////////////
class CDS_HmInj : public CDS_HydroMod
{
	//DEFINE_DATA( CDS_HmInj )

public:
	CDS_HmInj( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_HmInj() {}

	void SetDesignCircuitSupplySecondaryTemperature( double dDesignCircuitSupplySecondaryTemperature );
	void SetDesignCircuitReturnSecondaryTemperature( double dDesignCircuitReturnSecondaryTemperature );
	virtual double GetDesignCircuitSupplySecondaryTemperature() { return m_dDesignCircuitSupplySecondaryTemperature; }
	virtual double GetDesignCircuitReturnSecondaryTemperature() { return m_dDesignCircuitReturnSecondaryTemperature; }

	// HYS-1716: This method allows to retrieve the water characteristic for a specific pipe.
	// Here we can determine the pipe with its type (Distribution or circuit), its location (Supply or return) and in the case
	// of a circuit pipe its side (Primary or secondary).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is this method that is
	// called and not the base one.
	// ATTENTION: this method for circuit return pipe at the secondary side will always return the water characteristic for the 
	// COMPUTED return temperature! This computed temperature can be the same as the design one in some cases.
	// To have the design water characteristic for the circuit return pipe at the secondary side, call the 'GetpDesignWaterChar' method instead.
	virtual CWaterChar *GetpWaterChar( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );

	// HYS-1716: This method allows to retreive the water characteristic at the outlet of the circuit on the supply or return pipe.
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is this method that is
	// called and not the base one.
	// ATTENTION: this method for the circuit return pipe at the secondary side will always return the water characteristic for the COMPUTED return temperature!
	// This computed temperature can be the same as the design one in some cases.
	// To have the OUT design water characteristic for the circuit pipes at the secondary side, call the 'GetpOutDesignWaterChar' method instead.
	virtual CWaterChar *GetpOutWaterChar( CAnchorPt::PipeLocation ePipeLocation );

	// HYS-1716: This method allows to retrieve the temperature for a specific pipe.
	// Here we can determine the pipe with its type (Distribution or circuit), its location (Supply or return) and in the case
	// of a circuit pipe its side (Primary or secondary).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is this method that is
	// called and not the base one.
	// ATTENTION: this method for the circuit return pipe at the secondary side will always return the COMPUTED temperature!
	// This computed temperature can be the same as the design one in some cases.
	// To have the design temperature for the circuit pipes at the secondary side, call the 'GetDesignTemperature' method instead.
	virtual double GetTemperature( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );

	// HYS-1716: This method allows to retreive the temperature at the outlet of the circuit on the supply or return pipe.
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is this method that is
	// called and not the base one.
	// ATTENTION: this method for the circuit return pipe at the secondary side will always return the COMPUTED return temperature!
	// This computed temperature can be the same as the design one in some cases.
	// To have the design temperature for the circuit pipes at the secondary side, call the 'GetOutDesignTemperature' method instead.
	virtual double GetOutTemperature( CAnchorPt::PipeLocation ePipeLocation );

	// HYS-1716: This method allow to retrieve the power for the current hydraulic circuit at the primary or secondary side.
	// Remark: This method is virtual because if we have an injection circuit and we want this side, it is this method that is called
	// and not the base one.
	virtual double GetPower( CAnchorPt::CircuitSide eCircuitSide );

	// HYS-1716: This method allows to verify water characteristic for the current hydraluic circuit.
	// Param: 'pclPressureInterfaceHM': To know what is the supply temperature starting from this pressure interface.
	// Param: 'pclPreviousCircuit': Allows to compute the return temperature.
	// Normally, this method must not be manually called. It must be called only by the 'VerifyAllWaterCharacteristic'
	// in charge to verify a pressure interface and all its children.
	virtual void VerifyWaterCharacteristic( CDS_HydroMod *pclPressureInterfaceHM = NULL, CDS_HydroMod *pclPreviousCircuit = NULL );

	// HYS-1716: This method allows to set the water characteristic for a specific pipe.
	// Here we can determine the pipe with its type (Distribution or circuit), its location (Supply or return) and in the case
	// of a circuit pipe its side (Primary or secondary).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is this method that is
	// called and not the base one.
	virtual void SetWaterChar( CWaterChar *pclWaterChar, CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );

	// HYS-1716: This method allows to set the temperature for a specific pipe.
	// Here we can determine the pipe with its type (Distribution or circuit), its location (Supply or return) and in the case
	// of a circuit pipe its side (Primary or secondary).
	// This method is virtual because we can have an injection circuit with a secondary side. In that case, it is this method that is
	// called and not the base one.
	virtual void SetTemperature( double dTemperature, CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );

	// Update Q in each HM component;
	// this function DOES NOT launch an automatic recompute of the module
	virtual void SetHMPrimAndSecFlow( double dQ ) = 0;

	virtual double GetQ( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide_NotApplicable );
	
	// Based on Terminal unit Q and temperature return primary flow
	double ComputePrimaryFlow();
	double ComputePrimaryFlow( double dSecondaryFlow );
	double GetDpSecSide();

	virtual double GetTotQwoDiversity();

	virtual double GetPartialPendingQ();

	virtual double AdjustPumpHmin();

	enum InjectionTemperatureErrors
	{
		ITE_OK							= 0x0000,
		ITE_TemperatureEquals			= 0x0001,
		ITE_SupplyTemp_FreezingPoint	= 0x0002,
		ITE_ReturnTemp_FreezingPoint	= 0x0004,
		ITE_Heating_TssGreaterThanTsp	= 0x0008,
		ITE_Heating_TssLowerThanTrs		= 0x0010,
		ITE_Heating_TrsGreaterThanTsp		= 0x0020,
		ITE_Cooling_TssLowerThanTsp	= 0x0040,
		ITE_Cooling_TssGreaterThanTrs	= 0x0080,
		ITE_Cooling_TrsLowerThanTsp	= 0x0100,
	};
	virtual bool CheckIfTempAreValid( int *piErrors = NULL, std::vector<CString> *pvecStrErrors = NULL );

	// Return true if all defined HM objects are defined.
	virtual bool IsCompletelyDefined( int iHMObjToCheck = eHMObj::eALL );

	void Init( IDPTR SchIDPtr, CTable *plcParent = NULL, bool bUseTACV = false, bool bCreateLinkedDevice = true );

	// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
	// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
	virtual void Copy( CDS_HydroMod *pHM2W, bool bValveDataOnly = false, bool bCreateLinkedDevice = true, bool bToClipboard = false, bool bResetDiversityFactor = true );
	virtual void Copy( CData *pTo );

	// Compute a new setting point for balancing device in function of Havail.
	// "pbHasHAvailSecChanged" if defined will contain 'true' if Hmin of the secondary pump has changed.
	// Typically at now it is ony for auto-adaptive flow with decoupling bypass.
	virtual void ResizeBalDevForH( double Havail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged = NULL );

	// Spread any change for other elements
	virtual void ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj = NULL ) = 0;

	// Allow to inherited class ( ie: 'CDS_HmInj', 'CDS_Hm2WInj' and so no) to fill specific data.
	virtual void FillOtherInfoIntoHMX( CDS_HydroModX *pHMX );

	virtual void UpdateTUTemperaturesForInjectionCircuit();

	// HYS-1716: This method is called when 'CDS_HydroMod::Init' is called. Typically it is when creating a project
	// and to update water characteristic of the different pipes of the current hydraulic circuit with its design
	// temperature interface.
	// This method is virtual because we need also to update circuit secondary pipes if they exist.
	// ATTENTION!!! 'pclParent' is not necessarily the design temperature interface. We need to verify between to use it.
	// If it's a dedsign temperature interface, we can use it. Otherwise, we retrieve the design temperature interface for this
	// 'pclParent' itself. If 'pclParent' is NULL, we will take the general settings.
	virtual void InitAllWaterCharWithParent( CTable *pclParent, eHMObj eWhichPipe = eHMObj::eALL );

	// Protected methods.
protected:
	void SetComputeHMInjEngine( eComputeHMEvent event, LPARAM lparam = 0 ) { m_ComputeHMInjState = event; m_ComputeHMInjParam = lparam; }
	virtual double GetDpOnOutOfPrimSide() { return 0.0; }			//bypass

	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy() { CDS_HydroMod::OnDatabaseDestroy(); }

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	// Allow to inherited class ( ie: 'CDS_HmInj', 'CDS_Hm2WInj' and so no) to fill specific data.
	virtual void TransferOtherInfoFromHMX( CDS_HydroModX *pHMX );
	
// Protected variables.
protected:
	eComputeHMEvent m_ComputeHMInjState;
	LPARAM m_ComputeHMInjParam;
	bool m_bComputeHMInjAllowed;

	// These are the design temperatures that user set for the injection circuit.
	// Remark: the return temperature can be different that the one that will be computed later in the return pipe !!
	double m_dDesignCircuitSupplySecondaryTemperature;
	double m_dDesignCircuitReturnSecondaryTemperature;
};

class CDS_Hm2WInj : public CDS_HmInj
{
	DEFINE_DATA( CDS_Hm2WInj )

public:
	CDS_Hm2WInj( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_Hm2WInj() {}

	void Init( IDPTR SchIDPtr, CTable *plcParent = NULL, bool bUseTACV = false, bool bCreateLinkedDevice = true );

	// Update Q in each HM component;
	// this function DOES NOT launch an automatic recompute of the module
	virtual void SetHMPrimAndSecFlow( double dQ );

	// Compute a new setting point for balancing device in function of Havail.
	// "pbHasHAvailSecChanged" if defined will contain 'true' if Hmin of the secondary pump has changed.
	// Typically at now it is ony for auto-adaptive flow with decoupling bypass.
	virtual void ResizeBalDevForH( double Havail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged = NULL );

	// Spread any change for other elements
	virtual void ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj = NULL );

	// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
	// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
	virtual void Copy( CDS_HydroMod *pHM2W, bool bValveDataOnly = false, bool bCreateLinkedDevice = true, bool bToClipboard = false, bool fResetDiversityFactor = true );
	virtual void Copy(CData *pTo);

	// For 2-way injection circuit we DO NOT HAVE exception. All the 'CDS_Hm2WInj' are well injection. 
	virtual bool IsInjectionCircuit() { return true; }

protected:
	// Disk IO
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
};

////////////////////////////////////////////////////////////////////////////////////////////
class CDS_Hm3WInj : public CDS_HmInj
{
	DEFINE_DATA( CDS_Hm3WInj )
public:
	CDS_Hm3WInj( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_Hm3WInj() {}

	virtual double GetTechParamCVMinDpProp();

	void AdjustCircuitIDForValveLocalisation(CvLocation CvLoc);

	void Init( IDPTR SchIDPtr, CTable *plcParent = NULL, bool bUseTACV = false, bool bCreateLinkedDevice = true );

	// Update Q in each HM component;
	// this function DOES NOT launch an automatic recompute of the module
	virtual void SetHMPrimAndSecFlow( double dQ );

	// Compute a new setting point for balancing device in function of Havail.
	// "pbHasHAvailSecChanged" if defined will contain 'true' if Hmin of the secondary pump has changed.
	// Typically at now it is ony for auto-adaptive flow with decoupling bypass.
	virtual void ResizeBalDevForH( double Havail, bool fForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged = NULL );

	// Spread any change for other elements
	virtual void ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj = NULL );

	// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
	// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
	virtual void Copy( CDS_HydroMod *pHM2W, bool bValveDataOnly = false, bool bCreateLinkedDevice = true, bool bToClipboard = false, bool fResetDiversityFactor = true );
	virtual void Copy(CData *pTo);

	virtual double AdjustPumpHmin();

	// For 3-way injection circuit we HAVE exception. The 3-way mixing  (3W_INJ_BV1 and 3W_INJ_DP12) are not 
	// considered as injection (Historically this circuit has been introduced as an injection with the possibility
	// to change temperature at the secondary side. But normally this kind of circuit is for passive distribution.
	// So, we kept for some user but we removed the possibility to change the temperatures).
	virtual bool IsInjectionCircuit();

protected:
	bool _FindAndSetMatchingScheme(CvLocation CvLoc);
	// Disk IO
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
};

////////////////////////////////////////////////////////////////////////////////////////////
class CDS_Hm3W : public CDS_HydroMod
{
	DEFINE_DATA( CDS_Hm3W )

public:
	CDS_Hm3W( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_Hm3W() {}

	void Init( IDPTR SchIDPtr, CTable *plcParent = NULL, bool bUseTACV = false, bool bCreateLinkedDevice = true );

	// Compute a new setting point for balancing device in function of Havail.
	// "pbHasHAvailSecChanged" if defined will contain 'true' if Hmin of the secondary pump has changed.
	// Typically at now it is ony for auto-adaptive flow with decoupling bypass.
	virtual void ResizeBalDevForH( double Havail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged = NULL );

	// Spread any change for other elements
	virtual void ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj = NULL );

	// 'fResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
	// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
	virtual void Copy( CDS_HydroMod *pHM3W, bool bValveDataOnly = false, bool bCreateLinkedDevice = true, bool bToClipboard = false, bool fResetDiversityFactor = true );
	virtual void Copy(CData *pTo);

	void SetUseBypBv( eUseProduct UseBypBv );
	eUseProduct GetUseBypBv() { return m_UseBypBv; }
	virtual double GetBypOverFlow();

	// Return true if all defined HM objects are defined.
	virtual bool IsCompletelyDefined( int iHMObjToCheck = eHMObj::eALL );

	// Allow to inherited class ( ie: 'CDS_HmInj', 'CDS_Hm2WInj' and so on) to fill data concerning valves.
	virtual void TransferValvesInfoFromHMX( CDS_HydroModX *pHMX, int iHMObjToCheck = eHMObj::eALL );

	// Allow to inherited class ( ie: 'CDS_HmInj', 'CDS_Hm2WInj' and so no) to fill specific data.
	virtual void FillOtherInfoIntoHMX( CDS_HydroModX *pHMX );
	
// Protected methods.
protected:
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy() {CDS_HydroMod::OnDatabaseDestroy();}
	// Disk IO
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

// Protected variables.
protected:
	// Saved variable
	eUseProduct	m_UseBypBv;

// Private methods.
private:
	void SetComputeHM3WEngine( eComputeHMEvent event, LPARAM lparam = 0 ) { m_ComputeHM3WState = event; m_ComputeHM3WParam = lparam;}

// Private variables.
private:
	IDPTR m_SavedIDPtr;
	eComputeHMEvent m_ComputeHM3WState;
	LPARAM m_ComputeHM3WParam;
};


#endif // !defined(AFX_HYDROMOD_H__6418221E_811A_49EE_91BF_6FA1CA79AC96__INCLUDED_)
