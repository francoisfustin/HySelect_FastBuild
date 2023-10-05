//////////////////////////////////////////////////////////////////////////
//  Units.h : header file
//	Version: 1.1							Date: 14/04/08
//////////////////////////////////////////////////////////////////////////
//
//	This file contains all the required declarations to store a physical type
//  unit database.
//	What is a physical type ?
//
//	1- It is a physical dimension which is a combination of
//	   basic physical dimensions : mass, length and time.
//	2- It is a set of different names and conversion/offset factor
//	   to a standard unit convention (kilogram, metre and second for example).
//	   Indeed, the same physical type can be expressed in different 
//     conventional ways.
//	   Ex : The velocitity (L^1*T^-1) can be expressed with metre/second, 
//          millimeter/hour, ...
//			but it always remains the same physical type.
//
//	The different physical types wich can be used in the program are hardcoded. 
//  They are described with the UNITS_PHYSICAL_TYPE enumerator. 
//  The CUnitsDatabase manages the units and the different ways to express 
//  the same physical type.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef UNITS_H__INCLUDED_
#define UNITS_H__INCLUDED_

#include <string>
#include "global.h"


#define _MAXCHARS 16        // maximum string length for the units names.
 
// Unit identificators.

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// WARNING TO KEEP TASCOPE COMPATIBILTY ANY NEW UNIT MUST BE ADDED AT THE END OF LIST!
// AND: CTAPERSISTDATA_FORMAT_VERSION version must be updated.
// AND: the "CTAPersistData::FillMissingUnits" method in the "datastruct.cpp" file must be adapted
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
enum ePHYSTYPE
{
	_U_NODIM=0,			// Always the first
	_U_PRESSURE,
	_U_DIFFPRESS,
	_U_LINPRESSDROP,
	_U_FLOW,
	_U_VELOCITY,
	_U_TEMPERATURE,
	_U_DIFFTEMP,
	_U_TH_POWER,
	_U_ELEC_POWER,
	_U_LENGTH,
	_U_DIAMETER,
	_U_ROUGHNESS,
	_U_THICKNESS,
	_U_AREA,
	_U_VOLUME,
	_U_DENSITY,
	_U_KINVISCOSITY,
	_U_DYNVISCOSITY,
	_U_SPECIFHEAT,
	_U_THERMCOND,
	_U_HEATTRANSF,
	_U_FORCE,
	_U_TORQUE,
	_U_TIME,
	_C_KVCVCOEFF,
	_U_PERCENT,
	_U_MASS,
	_U_WATERHARDNESS,
	_U_ENERGY,
	_UNITS_NUMBER_OF	// Always write it at the end of the enumerator list
}; 

enum PressureUnits
{
	_U_PRESSURE_PA = 0,
	_U_PRESSURE_KPA,
	_U_PRESSURE_BAR,
	_U_PRESSURE_MBAR,
	_U_PRESSURE_MH20,
	_U_PRESSURE_MMH20,
	_U_PRESSURE_ATM,
	_U_PRESSURE_FTH20,
	_U_PRESSURE_INH20,
	_U_PRESSURE_PSI,
	_U_PRESSURE_KGF_CM2
};

enum DiffPressureUnits
{
	_U_DIFFPRESS_PA = 0,
	_U_DIFFPRESS_KPA,
	_U_DIFFPRESS_BAR,
	_U_DIFFPRESS_MBAR,
	_U_DIFFPRESS_MH20,
	_U_DIFFPRESS_MMH20,
	_U_DIFFPRESS_ATM,
	_U_DIFFPRESS_FTH20,
	_U_DIFFPRESS_INH20,
	_U_DIFFPRESS_PSI,
	_U_DIFFPRESS_KGF_CM2
};

enum LinearPressureDropUnits
{
	_U_LINPRESSDROP_PA_M = 0,
	_U_LINPRESSDROP_DPA_M,
	_U_LINPRESSDROP_MMH20_M,
	_U_LINPRESSDROP_FTH20_100FT,
	_U_LINPRESSDROP_PSI_100FT
};

enum FlowUnits
{
	_U_FLOW_M3_S = 0,
	_U_FLOW_M3_L,
	_U_FLOW_L_S,
	_U_FLOW_L_MIN,
	_U_FLOW_L_H,
	_U_FLOW_USGPM,
	_U_FLOW_USGPH,
	_U_FLOW_UKGPM,
	_U_FLOW_CFM
};

enum VelocityUnits
{
	_U_VELOCITY_M_S = 0,
	_U_VELOCITY_FT_S,
	_U_VELOCITY_FT_MIN,
};

enum TemperatureUnits
{
	_U_TEMPERATURE_C = 0,
	_U_TEMPERATURE_F,
	_U_TEMPERATURE_R
};

enum DifferentialTemperatureUnits
{
	_U_DIFFTEMP_C = 0,
	_U_DIFFTEMP_F,
	_U_DIFFTEMP_R
};

enum ThermalPowerUnits
{
	_U_TH_POWER_W = 0,
	_U_TH_POWER_KW,
	_U_TH_POWER_CAL_H,
	_U_TH_POWER_KCAL_H,
	_U_TH_POWER_BTU_H,
	_U_TH_POWER_KBTU_H,
	_U_TH_POWER_HP,
	_U_TH_POWER_TON_REFRIG
};

enum ElectricalPowerUnits
{
	_U_ELEC_POWER_W = 0,
	_U_ELEC_POWER_KW,
	_U_ELEC_POWER_mW
};

enum LengthUnits
{
	_U_LENGTH_M = 0,
	_U_LENGTH_CM,
	_U_LENGTH_MM,
	_U_LENGTH_UM,
	_U_LENGTH_FT,
	_U_LENGTH_IN,
	_U_LENGTH_MIL
};

enum DiameterUnits
{
	_U_DIAMETER_M = 0,
	_U_DIAMETER_CM,
	_U_DIAMETER_MM,
	_U_DIAMETER_UM,
	_U_DIAMETER_FT,
	_U_DIAMETER_IN,
	_U_DIAMETER_MIL
};

enum RoughnessUnits
{
	_U_ROUGHNESS_M = 0,
	_U_ROUGHNESS_CM,
	_U_ROUGHNESS_MM,
	_U_ROUGHNESS_UM,
	_U_ROUGHNESS_FT,
	_U_ROUGHNESS_IN,
	_U_ROUGHNESS_MIL
};

enum ThicknessUnits
{
	_U_THICKNESS_M = 0,
	_U_THICKNESS_CM,
	_U_THICKNESS_MM,
	_U_THICKNESS_UM,
	_U_THICKNESS_FT,
	_U_THICKNESS_IN,
	_U_THICKNESS_MIL
};

enum AreaUnits
{
	_U_AREA_M2 = 0,
	_U_AREA_CM2,
	_U_AREA_MM2,
	_U_AREA_FT2,
	_U_AREA_INCH2
};

enum VolumeUnits
{
	_U_VOLUME_M3 = 0,
	_U_VOLUME_DM3,
	_U_VOLUME_L,
	_U_VOLUME_FT3,
	_U_VOLUME_INCH3,
	_U_VOLUME_GALUK,
	_U_VOLUME_GALUS
};

enum DensityUnits
{
	_U_DENSITY_KG_M3 = 0,
	_U_DENSITY_LB_FT3
};

enum KinematicViscosityUnits
{
	_U_KINVISCOSITY_M2_S = 0,
	_U_KINVISCOSITY_CM2_S,
	_U_KINVISCOSITY_MM2_S,
	_U_KINVISCOSITY_STKS,
	_U_KINVISCOSITY_CSTKS,
	_U_KINVISCOSITY_FT2_S
};

enum DynamicViscosityUnits
{
	_U_DYNVISCOSITY_PAxS = 0,
	_U_DYNVISCOSITY_NxS_M2,
	_U_DYNVISCOSITY_KG_MxS,
	_U_DYNVISCOSITY_POISE,
	_U_DYNVISCOSITY_CPOISE,
	_U_DYNVISCOSITY_G_CMxS,
	_U_DYNVISCOSITY_LB_FTxS,
	_U_DYNVISCOSITY_LF_FTxH,
	_U_DYNVISCOSITY_LBxS_FT2,
	_U_DYNVISCOSITY_LBxS_IN2
};

enum SpecificHeatUnits
{
	_U_SPECIFHEAT_J_KGxK = 0,
	_U_SPECIFHEAT_KJ_KGxK,
	_U_SPECIFHEAT_KCAL_KGxC,
	_U_SPECIFHEAT_BTU_LBxF
};

enum ThermalConductivityUnits
{
	_U_THERMCOND_W_MxK = 0,
	_U_THERMCOND_BTU_HxFTxF
};

enum HeatTransferUnits
{
	_U_HEATTRANSF_W_M2XK = 0,
	_U_HEATTRANSF_BTU_HxFT2xF
};

enum ForceUnits
{
	_U_FORCE_N = 0,
	_U_FORCE_KN,
	_U_FORCE_KGF,
	_U_FORCE_LBF,
	_U_FORCE_DYNE
};

enum TorqueUnits
{
	_U_TORQUE_NM = 0,
	_U_TORQUE_LBFxFT,
	_U_TORQUE_LBFxIN
};

enum TimeUnits
{
	_U_TIME_S = 0
};

enum KCCVCoefficientUnits
{
	_C_KVCVCOEFF_KV = 0,
	_C_KVCVCOEFF_CV
};

enum PercentUnits
{
	_U_PERCENT_PER = 0
};

enum MassUnits
{
	_U_MASS_KG = 0,
	_U_MASS_G,
	_U_MASS_LB,
	_U_MASS_OZ
};

enum WaterHardnessUnits
{
	_U_WATERHARDNESS_MOL_M3 = 0,
	_U_WATERHARDNESS_MMOL_L,
	_U_WATERHARDNESS_DH,
	_U_WATERHARDNESS_DGH,
	_U_WATERHARDNESS_FH,
	_U_WATERHARDNESS_E,
	_U_WATERHARDNESS_CLARK,
	_U_WATERHARDNESS_PPM,
	_U_WATERHARDNESS_MG_L,
	_U_WATERHARDNESS_MVAL_L,
	_U_WATERHARDNESS_GPG
};

enum EnergyUnits
{
	_U_ENERGY_J = 0,
	_U_ENERGY_KJ,
	_U_ENERGY_MJ,
	_U_ENERGY_KWH,
	_U_ENERGY_CAL,
	_U_ENERGY_KCAL,
	_U_ENERGY_BTU,
	_U_ENERGY_KBTU,
	_U_ENERGY_MBTU,
	_U_ENERGY_TONxH
};

// Unit identificator strings external definition
extern LPCTSTR _UNIT_STRING_ID[_UNITS_NUMBER_OF];

// Aggregate the name (Name[]) of a given unit and the conversion factor to 
// the standard conventional unit (Conv).
// "Value(with standard unit)" = Conv * ( "Value(with my unit)" + offset ). 
// HYS-1922: MaxDig = The maximum number of digits for the number. Add MaxDec = The maximum number of digits in the decimal part.
struct UnitDesign_struct			
{                           
	TCHAR Name[_MAXCHARS];  
	double Conv;
	double Offset;
	int MaxDig;
	int MinDec;
	int MaxDec;
};
  
/////////////////////////////////////////////////////////////////////////////
// Functions to manipulate UnitDesign_struct
/////////////////////////////////////////////////////////////////////////////

// Constructs a UnitDesign_struct from a name and a conversion factor.
UnitDesign_struct UnitDesign(LPCTSTR name, double conv, double offset = 0.0, int MaxDig=3, int MinDec=0, int MaxDec=-1 );

// Get the name and the conversion factor from a UnitDesign_struct.
void GetNameOf(const UnitDesign_struct &ud, TCHAR *str);
_string GetNameOf(const UnitDesign_struct &ud);
double GetConvOf(const UnitDesign_struct &ud);
double GetOffsetOf(const UnitDesign_struct &ud);
// HYS-1922
int GetMaxDig(const UnitDesign_struct &ud);
int GetMinDec(const UnitDesign_struct &ud);
int GetMaxDec(const UnitDesign_struct &ud);

/////////////////////////////////////////////////////////////////////////////
//
//					Class CUnitDatabase
//
//	The CUnitDatabase is used to store all the available custom units
//	according to their physical types.
//
//	CONTRUCTOR/DESTRUCTOR
//		CUnitDatabase()
//		~CUnitDatabase()
//
//	OPERATIONS ON THE DATABASE
//		Add : add a unit description in the database.
//		Del : Delete ...
//		SetDefaultUnit : allow you to change the default units.
//		Operator= : dynamic copy of a CUnitDatabase object.
//
//	GET INFORMATION
//		AlreadyExist : let you know if a given unit is already stored.
//		GetLength : give you the number of units for a given physical type.
//		GetUnit : retrieve an unit description from the database.
//		GetDefaultUnit : retrieve the default unit for a given physical type.
//
/////////////////////////////////////////////////////////////////////////////

class CUnitDatabase
{
private:

	struct Units_struct
	{
		UnitDesign_struct *pUDesign;	// UDesign is the declarator for a dynamic allocated array.
		int IndexMax;					// IndexMax is the # of different ways to express the same unit -1.
		int Default;					// Default unit index. Sometimes you only know the unit type but 
	} m_Units[_UNITS_NUMBER_OF];		// not the custom choice for it. So use the default one.

public:	

	// Constructor and destructor.
	CUnitDatabase();
	~CUnitDatabase();
	
public:

// Manipulate the database.
//--------------------------

	// Add a custom unit to a specified physical unit.
	void Add(int PhysicalType, const UnitDesign_struct &NewUnit);
	
	// delete a specified custom unit
	void Del(int PhysicalType, int Index);

	// set the default unit for a specified physical type.	
	int SetDefaultUnit(int PhysicalType, int index);
	
	// dynamic copy of CUnitDatabase object.
	CUnitDatabase &operator=(const CUnitDatabase &udb);

// Get information from the database.
//---------------------------------------

	// test if the unit already exist in the database.
	// return 1 if yes, 0 if no.	
	int AlreadyExist(int PhysicalType, const UnitDesign_struct &CompareUnit) const;
	
	// return the number of custom units for a given physical type
	int GetLength(int PhysicalType) const;
	
	// return the name and the conversion coefficient (struct UnitDesign_struct)
	// for a given physical type and a given custom unit.
	const UnitDesign_struct& GetUnit(int PhysicalType, int Index) const;
	
	void ResetToSI();
	// Return the default unit for a given physical type.
	int GetDefaultUnitIndex(int PhysicalType) const;
	const UnitDesign_struct& GetDefaultUnit(int PhysicalType) const;
	_string GetNameOfDefaultUnit(int PhysicalType) const;
};


/////////////////////////////////////////////////////////////////////////////
//
//					Class CDimValue
//
//	A CDimValue Object is a double value with a description of its units.
//	The user can fix these units and fix its value in standard units
//	or in customised units.
//	Most of arithmetic and logical comparison operators are overloaded.
//
//	CONTRUCTORS
//		You can construct a CDimValue object with a CDimValue object,
//		double or int value.
//
//	ACCESS VALUE AND UNIT DESCRIPTION
//		SetUnit and GetUnit :	allow you to set and get the physical type
//								and the unit description of your value.
//		Set/GetCustomValue : 	allow you to set and to retrieve the value
//								expressed in customised unit.
//		
//	OPERATORS
//		= assigns only the standard value, physical type and unit are unchanged.
//		(double) allow you to retrieve the value expressed in standard unit.
//		+, -, *, / are overloaded
//		==, !=, <, <=, >, >= are overloaded.
//
//	
/////////////////////////////////////////////////////////////////////////////

class CDimValue
{
private:

	int m_PhysType;					// physical type of the value
	UnitDesign_struct m_Unit;		// Definition of the unit (name and conversion factor)
	double m_Value;					// The value expressed in standard unit (e.g. MKS)
	
	static CUnitDatabase* m_pUnitDB;	// Shared unit database.
	
public:
		
// Constructors (simple, copy and conversion) and destructor
	CDimValue(double val = 0.0, int phtp = _U_NODIM);
	CDimValue(int val, int phtp = _U_NODIM);
	CDimValue(const CDimValue &dv);
	~CDimValue();
	
// about unit specification
	void SetUnit(int phystype, int index);
	void SetUnit(int phystype);
	int SetUnit(int phystype, const UnitDesign_struct &ud);
	void GetUnit(int *phystype, UnitDesign_struct *ud) const;

// Set/get value expressed in custom units.
	void SetCustomValue(double val);
	double GetCustomValue() const;

// Convert the current unit to the default unit.
	// return the custom value in default unit
	double ConvertToDefaultUnit();

// Conversion between a value expressed in default custom units and 
// a value in standard unit
	// Custom unit to standard unit
	static double CUtoSI(int phystype, double CUValue);
	// Standard to custom
	static double SItoCU(int phystype, double SIValue);
	
// Access to the unit database shared by all instances of CDimValue
	static CUnitDatabase* &AccessUDB() {return m_pUnitDB; };
	
// Operators

	// Allow you to retrieve the value expressed in standard units.
	operator double() const;

	// Assignement operator. Only the value expressed in standard units is assigned.
	// Combined with conversion constructor, you can assign int and double.
	CDimValue &operator=(const CDimValue &dv);
};

#endif // !defined(UNITS_H__INCLUDED_)
