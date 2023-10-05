//////////////////////////////////////////////////////////////////////////
//  Units.cpp : implementation file
//	Version: 1.1							Date: 14/04/08
//////////////////////////////////////////////////////////////////////////


#include "stdafx.h"


#ifdef TACBX
#include "Utilities.h"
#endif
#include "Units.h"

/////////////////////////////////////////////////////////////////////////////
// Unit identificator strings
/////////////////////////////////////////////////////////////////////////////


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// WARNING TO KEEP TASCOPE COMPATIBILTY ANY NEW UNIT MUST BE ADDED AT THE END OF LIST!
// AND: CTAPERSISTDATA_FORMAT_VERSION version must be updated.
// AND: the "CTAPersistData::FillMissingUnits" method in the "datastruct.cpp" file must be adapted
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
LPCTSTR	 _UNIT_STRING_ID[_UNITS_NUMBER_OF] =	{	_T("PHYS_NODIM"),
													_T("PHYS_PRESSURE"),
													_T("PHYS_DIFFPRESS"),
													_T("PHYS_LINDP"),
													_T("PHYS_FLOW"),
													_T("PHYS_VELOCITY"),
													_T("PHYS_TEMP"),
													_T("PHYS_DIFFTEMP"),
													_T("PHYS_TH_POWER"),
													_T("PHYS_ELEC_POWER"),
													_T("PHYS_LENGTH"),
													_T("PHYS_DIAMETER"),
													_T("PHYS_ROUGHNESS"),
													_T("PHYS_THICKNESS"),
													_T("PHYS_AREA"),
													_T("PHYS_VOLUME"),
													_T("PHYS_DENSITY"),
													_T("PHYS_KINVISC"),
													_T("PHYS_DYNVISC"),
													_T("PHYS_SPECIFHEAT"),
													_T("PHYS_THERMCOND"),
													_T("PHYS_HEATTRANSF"),
													_T("PHYS_FORCE"),
													_T("PHYS_TORQUE"),
													_T("PHYS_TIME"),
													_T("PHYS_VALVCOEFF"),
													_T("PHYS_PERCENT"),
													_T("PHYS_MASS"),
													_T("PHYS_WATERHARDNESS"),
													_T("PHYS_ENERGY")
											 };

/////////////////////////////////////////////////////////////////////////////
// Functions to manipulate UnitDesign_struct
/////////////////////////////////////////////////////////////////////////////

UnitDesign_struct UnitDesign( LPCTSTR name, double conv, double offset, int MaxDig/*=3*/, int MinDec/*=0*/, int MaxDec/*=-1*/ )
{
	UnitDesign_struct temp;
	_tcsncpy_s( temp.Name, SIZEOFINTCHAR( temp.Name ), name, SIZEOFINTCHAR( temp.Name ) - 1 );
	temp.Conv = conv;
	temp.Offset = offset;
	temp.MaxDig = MaxDig;
	temp.MinDec = MinDec;
	// HYS-1922: Add MaxDec
	temp.MaxDec = MaxDec;
	return temp;
}
void GetNameOf( const UnitDesign_struct &ud, TCHAR *str )
{
	_tcsncpy_s( str, SIZEOFINTCHAR( ud.Name ), ud.Name, SIZEOFINTCHAR( ud.Name ) - 1 );	// ud.Name size is: _MAXCHARS
}

_string GetNameOf( const UnitDesign_struct &ud )
{
	return ( _string )ud.Name;
};

double GetConvOf( const UnitDesign_struct &ud )
{
	return ud.Conv;
}

double GetOffsetOf( const UnitDesign_struct &ud )
{
	return ud.Offset;
}

int GetMaxDig( const UnitDesign_struct &ud )
{
	return ud.MaxDig;
};

int GetMinDec( const UnitDesign_struct &ud )
{
	return ud.MinDec;
};

int GetMaxDec( const UnitDesign_struct &ud )
{
	return ud.MaxDec;
};

/////////////////////////////////////////////////////////////////////////////
//
//					Class CUnitDatabase
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Constructor and destructor.

CUnitDatabase::CUnitDatabase()
{
	for( int i = 0; i < _UNITS_NUMBER_OF; i++ )
	{
		m_Units[i].pUDesign = NULL;
		m_Units[i].IndexMax = -1;
		m_Units[i].Default = -1;
	}

	// Store new units.
	// UnitDesign is: The unit name, the conversion factor, offset, max. decimal and min. decimal.
	Add( _U_NODIM, UnitDesign( _T("-"), 1.0, 0.0, 2, 0 ) );

	Add( _U_PRESSURE, UnitDesign( _T("Pa"), 1.0, 0.0, 0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("kPa"), 1.0E3, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("bar"), 1.0E5, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("mbar"), 1.0E2, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("m H2O"), 9810.0, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("mm H2O"), 9.81, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("atm"), 1.01325E5, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("ft H2O"), 2990.088, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("in H2O"), 248.84, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("psi"), 6894.75729, 0.0 ) );
	Add( _U_PRESSURE, UnitDesign( _T("kgf/cm2"), 9.81E4, 0.0 ) );

	Add( _U_DIFFPRESS, UnitDesign( _T("Pa"), 1.0, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("kPa"), 1.0E3, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("bar"), 1.0E5, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("mbar"), 1.0E2, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("m H2O"), 9810.0, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("mm H2O"), 9.81, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("atm"), 1.01325E5, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("ft H2O"), 2990.088, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("in H2O"), 248.84, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("psi"), 6894.75729, 0.0 ) );
	Add( _U_DIFFPRESS, UnitDesign( _T("kgf/cm2"), 9.81E4, 0.0 ) );

	Add( _U_LINPRESSDROP, UnitDesign( _T("Pa/m"), 1.0, 0.0 ) );
	Add( _U_LINPRESSDROP, UnitDesign( _T("dPa/m"), 10.0, 0.0 ) );
	Add( _U_LINPRESSDROP, UnitDesign( _T("mm H2O/m"), 9.81, 0.0 ) );
	Add( _U_LINPRESSDROP, UnitDesign( _T("ft H2O/100ft"), 98.1, 0.0 ) );
	Add( _U_LINPRESSDROP, UnitDesign( _T("psi/100ft"), 226.20595, 0.0 ) );

	Add( _U_FLOW, UnitDesign( _T("m3/s"), 1.0, 0.0 ) );
	Add( _U_FLOW, UnitDesign( _T("m3/h"), 2.77777777E-4, 0.0 ) );
	Add( _U_FLOW, UnitDesign( _T("l/s"), 1.0E-3, 0.0, 3, 2 ) );
	Add( _U_FLOW, UnitDesign( _T("l/min"), 1.666666666E-5, 0.0 ) );
	Add( _U_FLOW, UnitDesign( _T("l/h"), 2.777777777E-7, 0.0 ) );
	Add( _U_FLOW, UnitDesign( _T("USGPM"), 6.30901964E-5, 0.0 ) );
	Add( _U_FLOW, UnitDesign( _T("USGPH"), 1.05150327E-6, 0.0 ) );
	Add( _U_FLOW, UnitDesign( _T("UKGPM"), 7.5768E-5, 0.0 ) );
	Add( _U_FLOW, UnitDesign( _T("CFM"), 4.719474432E-4, 0.0 ) );

	Add( _U_VELOCITY, UnitDesign( _T("m/s"), 1.0, 0.0 ) );
	Add( _U_VELOCITY, UnitDesign( _T("ft/s"), 0.3048, 0.0 ) );
	Add( _U_VELOCITY, UnitDesign( _T("ft/min"), 5.08E-3, 0.0 ) );
	#ifndef TACBX
#pragma warning (disable : 4309)		// truncation of constant value
	#endif
	// Some characters are not taken by the CBX, replace by hexadecimal values
#ifndef TACBX
	// HYS-1922: Add MaxDec and use it for temperature definition. The value of MaxDec is 1.
	Add( _U_TEMPERATURE, UnitDesign( _T("°C"), 1.0, 0.0, 3, 0, 1 ) );
	Add( _U_TEMPERATURE, UnitDesign( _T("°F"), 0.5555555555, -32.0, 3, 0, 1 ) );
	Add( _U_TEMPERATURE, UnitDesign( _T("°R"), 0.5555555555, -491.67, 3, 0, 1 ) );
	Add( _U_DIFFTEMP, UnitDesign( _T("°C"), 1.0, 0.0, 3, 0, 1 ) );
	Add( _U_DIFFTEMP, UnitDesign( _T("°F"), 0.5555555555, 0.0, 3, 0, 1 ) );
	Add( _U_DIFFTEMP, UnitDesign( _T("°R"), 0.5555555555, 0.0, 3, 0, 1 ) );
#else
	TCHAR buf[20] = {0xB0, 'C', 0x00};	//_T("°C")
	Add( _U_TEMPERATURE, UnitDesign( buf, 1.0, 0.0, 1, 1 ) );
	buf[1] = 'F';	//_T("°F")
	Add( _U_TEMPERATURE, UnitDesign( buf, 0.5555555555, -32.0, 1, 1 ) );
	buf[1] = 'R';	//_T("°R")
	Add( _U_TEMPERATURE, UnitDesign( buf, 0.5555555555, -491.67, 1, 1 ) );
	Add( _U_TEMPERATURE, UnitDesign( _T( "K" ), 1.0, -273.15, 1, 1 ) );
	buf[1] = 'C';	//_T("°C")
	Add( _U_DIFFTEMP, UnitDesign( buf, 1.0, 0.0, 1, 1 ) );
	buf[1] = 'F';	//_T("°F")
	Add( _U_DIFFTEMP, UnitDesign( buf, 0.5555555555, 0.0, 1, 1 ) );
	buf[1] = 'R';	//_T("°R")
	Add( _U_DIFFTEMP, UnitDesign( buf, 0.5555555555, 0.0, 1, 1 ) );
#endif
	Add( _U_DIFFTEMP, UnitDesign( _T( "K" ), 1.0, 0.0, 2, 1, 1 ) );

	Add( _U_TH_POWER, UnitDesign( _T("W"), 1.0, 0.0 ) );
	Add( _U_TH_POWER, UnitDesign( _T("kW"), 1.0E3, 0.0 ) );
	Add( _U_TH_POWER, UnitDesign( _T("Cal/h"), 1.161666666E-3, 0.0 ) );
	Add( _U_TH_POWER, UnitDesign( _T("kCal/h"), 1.161666666, 0.0 ) );
	Add( _U_TH_POWER, UnitDesign( _T("Btu/h"), 0.293071070172, 0.0 ) );
	Add( _U_TH_POWER, UnitDesign( _T("kBtu/h"), 293.071070172, 0.0 ) );
	Add( _U_TH_POWER, UnitDesign( _T("HP"), 745.7, 0.0 ) );
	Add( _U_TH_POWER, UnitDesign( _T("ton (refrig.)"), 3517, 0.0 ) );

	Add( _U_ELEC_POWER, UnitDesign( _T("W"), 1.0, 0.0 ) );
	Add( _U_ELEC_POWER, UnitDesign( _T("kW"), 1.0E3, 0.0 ) );
	Add( _U_ELEC_POWER, UnitDesign( _T("mW"), 1.0E-3, 0.0 ) );

	Add( _U_LENGTH, UnitDesign( _T("m"), 1.0, 0.0 ) );
	Add( _U_LENGTH, UnitDesign( _T("cm"), 1.0E-2, 0.0 ) );
	Add( _U_LENGTH, UnitDesign( _T("mm"), 1.0E-3, 0.0 ) );
	#ifndef TACBX
	Add( _U_LENGTH, UnitDesign( _T("µm"), 1.0E-6, 0.0 ) );
	#else
	buf[0] = 0xB5;	//_T("µ")
	buf[1] = 'm';	//_T("m")
	Add( _U_LENGTH, UnitDesign( buf, 1.0E-6, 0.0 ) );
	#endif
	Add( _U_LENGTH, UnitDesign( _T("ft"), 0.3048, 0.0 ) );
	Add( _U_LENGTH, UnitDesign( _T("in"), 0.0254, 0.0 ) );
	Add( _U_LENGTH, UnitDesign( _T("mil"), 0.0000254, 0.0 ) );

	Add( _U_DIAMETER, UnitDesign( _T("m"), 1.0, 0.0 ) );
	Add( _U_DIAMETER, UnitDesign( _T("cm"), 1.0E-2, 0.0 ) );
	Add( _U_DIAMETER, UnitDesign( _T("mm"), 1.0E-3, 0.0 ) );
	#ifndef TACBX
	Add( _U_DIAMETER, UnitDesign( _T("µm"), 1.0E-6, 0.0 ) );
	#else
	Add( _U_DIAMETER, UnitDesign( buf, 1.0E-6, 0.0 ) );
	#endif
	Add( _U_DIAMETER, UnitDesign( _T("ft"), 0.3048, 0.0 ) );
	Add( _U_DIAMETER, UnitDesign( _T("in"), 0.0254, 0.0 ) );
	Add( _U_DIAMETER, UnitDesign( _T("mil"), 0.0000254, 0.0 ) );

	Add( _U_ROUGHNESS, UnitDesign( _T("m"), 1.0, 0.0 ) );
	Add( _U_ROUGHNESS, UnitDesign( _T("cm"), 1.0E-2, 0.0 ) );
	Add( _U_ROUGHNESS, UnitDesign( _T("mm"), 1.0E-3, 0.0 ) );
	#ifndef TACBX
	Add( _U_ROUGHNESS, UnitDesign( _T("µm"), 1.0E-6, 0.0 ) );
	#else
	Add( _U_ROUGHNESS, UnitDesign( buf, 1.0E-6, 0.0 ) );
	#endif
	Add( _U_ROUGHNESS, UnitDesign( _T("ft"), 0.3048, 0.0 ) );
	Add( _U_ROUGHNESS, UnitDesign( _T("in"), 0.0254, 0.0 ) );
	Add( _U_ROUGHNESS, UnitDesign( _T("mil"), 0.0000254, 0.0 ) );

	Add( _U_THICKNESS, UnitDesign( _T("m"), 1.0, 0.0 ) );
	Add( _U_THICKNESS, UnitDesign( _T("cm"), 1.0E-2, 0.0 ) );
	Add( _U_THICKNESS, UnitDesign( _T("mm"), 1.0E-3, 0.0 ) );
	#ifndef TACBX
	Add( _U_THICKNESS, UnitDesign( _T("µm"), 1.0E-6, 0.0 ) );
	#else
	Add( _U_THICKNESS, UnitDesign( buf, 1.0E-6, 0.0 ) );
	#endif
	Add( _U_THICKNESS, UnitDesign( _T("ft"), 0.3048, 0.0 ) );
	Add( _U_THICKNESS, UnitDesign( _T("in"), 0.0254, 0.0 ) );
	Add( _U_THICKNESS, UnitDesign( _T("mil"), 0.0000254, 0.0 ) );

	Add( _U_AREA, UnitDesign( _T("m2"), 1.0, 0.0 ) );
	Add( _U_AREA, UnitDesign( _T("cm2"), 1.0E-4, 0.0 ) );
	Add( _U_AREA, UnitDesign( _T("mm2"), 1.0E-6, 0.0 ) );
	Add( _U_AREA, UnitDesign( _T("ft2"), 9.290304E-2, 0.0 ) );
	Add( _U_AREA, UnitDesign( _T("inch2"), 6.4516E-4, 0.0 ) );

	Add( _U_VOLUME, UnitDesign( _T("m3"), 1.0, 0.0 ) );
	Add( _U_VOLUME, UnitDesign( _T("dm3"), 1.0E-3, 0.0 ) );
	Add( _U_VOLUME, UnitDesign( _T("l"), 1.0E-3, 0.0 ) );
	Add( _U_VOLUME, UnitDesign( _T("ft3"), 2.8316846592E-2, 0.0 ) );
	Add( _U_VOLUME, UnitDesign( _T("inch3"), 1.6387064E-5, 0.0 ) );
	Add( _U_VOLUME, UnitDesign( _T("Gal(UK)"), 4.546092E-3, 0.0 ) );
	Add( _U_VOLUME, UnitDesign( _T("Gal(US)"), 3.785411784E-3, 0.0 ) );

	Add( _U_DENSITY, UnitDesign( _T("kg/m3"), 1.0, 0.0 ) );
	Add( _U_DENSITY, UnitDesign( _T("lb/ft3"), 16.0183796782, 0.0 ) );

	Add( _U_KINVISCOSITY, UnitDesign( _T("m2/s"), 1.0, 0.0 ) );
	Add( _U_KINVISCOSITY, UnitDesign( _T("cm2/s"), 1.0E-4, 0.0 ) );
	Add( _U_KINVISCOSITY, UnitDesign( _T("mm2/s"), 1.0E-6, 0.0 ) );
	Add( _U_KINVISCOSITY, UnitDesign( _T("Stks"), 1.0E-4, 0.0 ) );
	Add( _U_KINVISCOSITY, UnitDesign( _T("cStks"), 1.0E-6, 0.0 ) );
	Add( _U_KINVISCOSITY, UnitDesign( _T("ft2/s"), 9.2936802974E-2, 0.0 ) );

	Add( _U_DYNVISCOSITY, UnitDesign( _T("Pa.s"), 1.0, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("N.s/m2"), 1.0, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("kg/m.s"), 1.0, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("Poise"), 0.1, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("cPoise"), 1.0E-3, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("g/cm.s"), 0.1, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("lb/ft.s"), 1.488818897638, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("lb/ft.h"), 4.13385826772E-4, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("lb.s/ft2"), 47.8974207948, 0.0 ) );
	Add( _U_DYNVISCOSITY, UnitDesign( _T("lb.s/in2"), 6897.22859446, 0.0 ) );

	Add( _U_SPECIFHEAT, UnitDesign( _T("J/kg.K"), 1.0, 0.0 ) );
	Add( _U_SPECIFHEAT, UnitDesign( _T("kJ/kg.K"), 1.0e+3, 0.0 ) );

	#ifndef TACBX
	Add( _U_SPECIFHEAT, UnitDesign( _T("kCal/kg.°C"), 4186.8, 0.0 ) );
	Add( _U_SPECIFHEAT, UnitDesign( _T("Btu/lb.°F"), 4186.8, 0.0 ) );

	Add( _U_THERMCOND, UnitDesign( _T("W/m.K"), 1.0, 0.0 ) );
	Add( _U_THERMCOND, UnitDesign( _T("Btu/h.ft.°F"), 1.73073466637, 0.0 ) );

	Add( _U_HEATTRANSF, UnitDesign( _T("W/m2.K"), 1.0, 0.0 ) );
	Add( _U_HEATTRANSF, UnitDesign( _T("Btu/h.ft2.°F"), 5.67826334111, 0.0 ) );
	#else
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("kCal/kg._C") );
	buf[8] = 0xB0;		// Replace _ by °
	Add( _U_SPECIFHEAT, UnitDesign( buf, 4186.8, 0.0 ) );
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("Btu/lb._F") );
	buf[7] = 0xB0;		// Replace _ by °
	Add( _U_SPECIFHEAT, UnitDesign( buf, 4186.8, 0.0 ) );

	Add( _U_THERMCOND, UnitDesign( _T("W/m.K"), 1.0, 0.0 ) );
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("Btu/h.ft._F") );
	buf[8] = 0xB0;		// Replace _ by °
	Add( _U_THERMCOND, UnitDesign( buf, 1.73073466637, 0.0 ) );

	Add( _U_HEATTRANSF, UnitDesign( _T("W/m2.K"), 1.0, 0.0 ) );
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("Btu/h.ft2._F") );
	buf[9] = 0xB0;		// Replace _ by °
	Add( _U_HEATTRANSF, UnitDesign( buf, 5.67826334111, 0.0 ) );
	#endif
	Add( _U_FORCE, UnitDesign( _T("N"), 1.0, 0.0 ) );
	Add( _U_FORCE, UnitDesign( _T("kN"), 1.0E+3, 0.0 ) );
	Add( _U_FORCE, UnitDesign( _T("kgf"), 9.81, 0.0 ) );
	Add( _U_FORCE, UnitDesign( _T("lbf"), 4.4482216, 0.0 ) );
	Add( _U_FORCE, UnitDesign( _T("dyne"), 1.0E-5, 0.0 ) );

	Add( _U_TORQUE, UnitDesign( _T("Nm"), 1.0, 0.0 ) );
	Add( _U_TORQUE, UnitDesign( _T("lbf.ft"), 1.355817952, 0.0 ) );
	Add( _U_TORQUE, UnitDesign( _T("lbf.in"), 1.12984829E-1, 0.0 ) );

	Add( _U_TIME, UnitDesign( _T("s"), 1.0, 0.0 ) );

	Add( _C_KVCVCOEFF, UnitDesign( _T("Kv"), 1.0, 0.0 ) );
	Add( _C_KVCVCOEFF, UnitDesign( _T("Cv"), 0.864976092, 0.0 ) );

	Add( _U_PERCENT, UnitDesign( _T("%"), 1.0, 0.0 ) );

	Add( _U_MASS, UnitDesign( _T("kg"), 1.0, 0.0 ) );
	Add( _U_MASS, UnitDesign( _T("g"), 1.0E-3, 0.0 ) );
	Add( _U_MASS, UnitDesign( _T("lb"), 0.45359237, 0.0 ) );
	Add( _U_MASS, UnitDesign( _T("oz"), 0.028349523, 0.0 ) );

	Add( _U_WATERHARDNESS, UnitDesign( _T("mol/m3"), 1.0, 0.0, 9 ) );		// SI
	Add( _U_WATERHARDNESS, UnitDesign( _T("mmol/l"), 1.0, 0.0, 9 ) );		
#ifndef TACBX
	Add( _U_WATERHARDNESS, UnitDesign( _T("°dH"), 0.178324958, 0.0 ) );		// German
	Add( _U_WATERHARDNESS, UnitDesign( _T("°dGH"), 0.178324958, 0.0 ) );		// German
	Add( _U_WATERHARDNESS, UnitDesign( _T("°fH"), 0.09982582785, 0.0 ) );		// French
	Add( _U_WATERHARDNESS, UnitDesign( _T("°e"), 0.14241349, 0.0 ) );			// English
	Add( _U_WATERHARDNESS, UnitDesign( _T("°Clark"), 0.14241349, 0.0 ) );		// English
#else
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("_dH") );
	buf[0] = 0xB0;		// Replace _ by °
	Add( _U_WATERHARDNESS, UnitDesign( buf, 0.178324958, 0.0 ) );
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("_dGH") );
	buf[0] = 0xB0;		// Replace _ by °
	Add( _U_WATERHARDNESS, UnitDesign( buf, 0.178324958, 0.0 ) );
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("_fH") );
	buf[0] = 0xB0;		// Replace _ by °
	Add( _U_WATERHARDNESS, UnitDesign( buf, 0.09982582785, 0.0 ) );
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("_e") );
	buf[0] = 0xB0;		// Replace _ by °
	Add( _U_WATERHARDNESS, UnitDesign( buf, 0.14241349, 0.0 ) );
	_tcscpy_s( buf, SIZEOFINTCHAR( buf ), _T("_Clark") );
	buf[0] = 0xB0;		// Replace _ by °
	Add( _U_WATERHARDNESS, UnitDesign( buf, 0.14241349, 0.0 ) );
#endif
	Add( _U_WATERHARDNESS, UnitDesign( _T("ppm"), 0.0099912875972, 0.0 ) );
	Add( _U_WATERHARDNESS, UnitDesign( _T("mg/l"), 0.0099912875972, 0.0 ) );
	Add( _U_WATERHARDNESS, UnitDesign( _T("mval/l"), 0.5, 0.0 ) );
	Add( _U_WATERHARDNESS, UnitDesign( _T("gpg"), 0.171031461, 0.0 ) );		// US

	Add( _U_ENERGY, UnitDesign( _T( "J" ), 1, 0.0 ) );                  // SI
	Add( _U_ENERGY, UnitDesign( _T( "kJ" ), 1.0E+3, 0.0 ) );
	Add( _U_ENERGY, UnitDesign( _T( "MJ" ), 1.0E+6, 0.0 ) );
	Add( _U_ENERGY, UnitDesign( _T( "kWh" ), 3.6E+6, 0.0 ) );
	Add( _U_ENERGY, UnitDesign( _T( "Cal" ), 4.1868, 0.0 ) );
	Add( _U_ENERGY, UnitDesign( _T( "kCal" ), 4186.8, 0.0 ) );
	Add( _U_ENERGY, UnitDesign( _T( "Btu" ), 1055.0558526, 0.0 ) );
	Add( _U_ENERGY, UnitDesign( _T( "kBtu" ), 1055055.8526, 0.0 ) );
	Add( _U_ENERGY, UnitDesign( _T( "MBtu" ), 1055055852.6, 0.0 ) );
	Add( _U_ENERGY, UnitDesign( _T( "ton.h" ), 12660670.231, 0.0 ) );
	#ifndef TACBX
#pragma warning (default : 4309)		// truncation of constant value
	#endif
}

CUnitDatabase::~CUnitDatabase()
{
	for( int i = 0; i < _UNITS_NUMBER_OF; i++ )
	{
		delete [] m_Units[i].pUDesign;
	}

	if( CDimValue::AccessUDB() == this )
	{
		CDimValue::AccessUDB() = 0;
		TRACE( _T("CUnitDatabase object connected to the CDimValue objects was deleted\r\n") );
	}
}

/////////////////////////////////////////////////////////////////////////////
// Add a custom unit to a specified physical unit.

void CUnitDatabase::Add( int PhysicalType, const UnitDesign_struct &NewUnit )
{
	ASSERT( ( PhysicalType >= 0 ) && ( PhysicalType < _UNITS_NUMBER_OF ) );

	if( AlreadyExist( PhysicalType, NewUnit ) )
	{
		return;
	}

	int NewIndexMax = m_Units[PhysicalType].IndexMax + 1;
	UnitDesign_struct *pOldUDesign = m_Units[PhysicalType].pUDesign;
	m_Units[PhysicalType].pUDesign = new UnitDesign_struct[NewIndexMax + 1];

	for( int i = 0; i < NewIndexMax; i++ )
	{
		m_Units[PhysicalType].pUDesign[i] = pOldUDesign[i];
	}

	m_Units[PhysicalType].pUDesign[NewIndexMax] = NewUnit;
	m_Units[PhysicalType].IndexMax = NewIndexMax;

	if( m_Units[PhysicalType].Default < 0 )
	{
		m_Units[PhysicalType].Default = 0;
	}

	delete [] pOldUDesign;
}

/////////////////////////////////////////////////////////////////////////////
// Delete a specified custom unit.

void CUnitDatabase::Del( int PhysicalType, int Index )
{
	ASSERT( ( PhysicalType >= 0 ) && ( PhysicalType < _UNITS_NUMBER_OF ) );
	ASSERT( ( Index >= 0 ) && ( Index <= m_Units[PhysicalType].IndexMax ) );

	int NewIndexMax = m_Units[PhysicalType].IndexMax - 1;
	UnitDesign_struct *pOldUDesign = m_Units[PhysicalType].pUDesign;

	if( NewIndexMax < 0 )
	{
		m_Units[PhysicalType].pUDesign = NULL;
		m_Units[PhysicalType].Default = -1;
	}
	else
	{
		m_Units[PhysicalType].pUDesign = new UnitDesign_struct[NewIndexMax + 1];
		int i;

		for( i = 0; i < Index; i++ )
		{
			m_Units[PhysicalType].pUDesign[i] = pOldUDesign[i];
		}

		for( i = Index; i <= NewIndexMax; i++ )
		{
			m_Units[PhysicalType].pUDesign[i] = pOldUDesign[i + 1];
		}

		if( ( m_Units[PhysicalType].Default >= Index ) && ( m_Units[PhysicalType].Default > 0 ) )
		{
			m_Units[PhysicalType].Default--;
		}
	}

	m_Units[PhysicalType].IndexMax = NewIndexMax;

	delete [] pOldUDesign;
}

/////////////////////////////////////////////////////////////////////////////
// set the default unit for a specified physical type.
// Return the old one.

int CUnitDatabase::SetDefaultUnit( int PhysicalType, int Index )
{
	ASSERT( ( PhysicalType >= 0 ) && ( PhysicalType < _UNITS_NUMBER_OF ) );
	ASSERT( ( Index >= 0 ) && ( Index <= m_Units[PhysicalType].IndexMax ) );

	if( Index > m_Units[PhysicalType].IndexMax )
	{
		Index = 0;
	}

	int temp = m_Units[PhysicalType].Default;
	m_Units[PhysicalType].Default = Index;
	return temp;
}

/////////////////////////////////////////////////////////////////////////////
// make a copy of the CUnitDatabase object.

CUnitDatabase &CUnitDatabase::operator=( const CUnitDatabase &udb )
{
	for( int i = 0; i < _UNITS_NUMBER_OF; i++ )
	{
		delete [] m_Units[i].pUDesign;
		m_Units[i].pUDesign = NULL;
		m_Units[i].Default = -1;
		m_Units[i].IndexMax = -1;

		for( int j = 0; j < udb.GetLength( i ); j++ )
		{
			Add( i, udb.GetUnit( i, j ) );
		}

		m_Units[i].Default = udb.GetDefaultUnitIndex( i );
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// Test if a given unit already exist in the database.

int CUnitDatabase::AlreadyExist( int PhysicalType, const UnitDesign_struct &CompareUnit ) const
{
	ASSERT( ( PhysicalType >= 0 ) && ( PhysicalType < _UNITS_NUMBER_OF ) );

	for( int i = 0; i <= m_Units[PhysicalType].IndexMax; i++ )
		if( !_tcscmp( m_Units[PhysicalType].pUDesign[i].Name, CompareUnit.Name ) )
		{
			return 1;
		}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// return the number of custom units for a specified physical type.

int CUnitDatabase::GetLength( int PhysicalType ) const
{
	ASSERT( ( PhysicalType >= 0 ) && ( PhysicalType < _UNITS_NUMBER_OF ) );

	return m_Units[PhysicalType].IndexMax + 1;
}

/////////////////////////////////////////////////////////////////////////////
// return the name and the conversion coefficient (struct UnitDesign_struct)
// for a given physical type and a given custom unit.

const UnitDesign_struct &CUnitDatabase::GetUnit( int PhysicalType, int Index ) const
{
	ASSERT( ( PhysicalType >= 0 ) && ( PhysicalType < _UNITS_NUMBER_OF ) );
	ASSERT( ( Index >= 0 ) && ( Index <= m_Units[PhysicalType].IndexMax ) );

	return m_Units[PhysicalType].pUDesign[Index];
}

/////////////////////////////////////////////////////////////////////////////
// reset all unit to SI.

void CUnitDatabase::ResetToSI()
{
	for( int i = 0; i < _UNITS_NUMBER_OF; i++ )
	{
		m_Units[i].Default = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// return the default custom unit for specified physical type.

int CUnitDatabase::GetDefaultUnitIndex( int PhysicalType ) const
{
	ASSERT( ( PhysicalType >= 0 ) && ( PhysicalType < _UNITS_NUMBER_OF ) );

	return m_Units[PhysicalType].Default;
}

const UnitDesign_struct &CUnitDatabase::GetDefaultUnit( int PhysicalType ) const
{
	ASSERT( ( PhysicalType >= 0 ) && ( PhysicalType < _UNITS_NUMBER_OF ) );

	int def = m_Units[PhysicalType].Default;

	if( def < 0 )
	{
		static UnitDesign_struct NullUnit = UnitDesign( _T("?"), 1.0, 0.0 );
		return NullUnit;
	}
	else
	{
		return m_Units[PhysicalType].pUDesign[def];
	}
}

_string CUnitDatabase::GetNameOfDefaultUnit( int PhysicalType ) const
{
	TCHAR unitname[_MAXCHARS];
	::GetNameOf( GetDefaultUnit( PhysicalType ), unitname );
	return _string( unitname );
}

/////////////////////////////////////////////////////////////////////////////
//
//					Class CDimValue
//
/////////////////////////////////////////////////////////////////////////////

CUnitDatabase *CDimValue::m_pUnitDB = 0;

/////////////////////////////////////////////////////////////////////////////
// Constructors and destructors.

CDimValue::CDimValue( double val /*= 0.0*/, int phtp /*= _U_NODIM*/ )
{
	ASSERT( ( phtp >= 0 ) && ( phtp < _UNITS_NUMBER_OF ) );

	if( !m_pUnitDB )
	{
		m_pUnitDB = new CUnitDatabase;
	}

	m_PhysType = phtp;
	m_Unit = m_pUnitDB->GetDefaultUnit( phtp );
	m_Value = val;
}

CDimValue::CDimValue( int val, int phtp /*= _U_NODIM*/ )
{
	ASSERT( ( phtp >= 0 ) && ( phtp < _UNITS_NUMBER_OF ) );

	if( !m_pUnitDB )
	{
		m_pUnitDB = new CUnitDatabase;
	}

	m_PhysType = phtp;
	m_Unit = m_pUnitDB->GetDefaultUnit( phtp );
	m_Value = ( double )val;
}

CDimValue::CDimValue( const CDimValue &dv )
{
	if( !m_pUnitDB )
	{
		m_pUnitDB = new CUnitDatabase;
	}

	dv.GetUnit( &m_PhysType, &m_Unit );
	m_Value = ( double )dv;
}

CDimValue::~CDimValue()
{
}

/////////////////////////////////////////////////////////////////////////////
// Set unit information.
// To set the unit, give the physical type and the index in the units database
// of the unit or the unit design. If the unit type does not match with
// one of the database for the specified physicaltype, return 0.

void CDimValue::SetUnit( int phystype, int index )
{
	ASSERT( m_pUnitDB );
	ASSERT( ( phystype >= 0 ) && ( phystype < _UNITS_NUMBER_OF ) );
	ASSERT( ( index >= 0 ) && ( index < m_pUnitDB->GetLength( phystype ) ) );

	m_PhysType = phystype;
	m_Unit = m_pUnitDB->GetUnit( phystype, index );
}

void CDimValue::SetUnit( int phystype )
{
	ASSERT( m_pUnitDB );
	ASSERT( ( phystype >= 0 ) && ( phystype < _UNITS_NUMBER_OF ) );

	m_PhysType = phystype;
	m_Unit = m_pUnitDB->GetDefaultUnit( phystype );
}

int CDimValue::SetUnit( int phystype, const UnitDesign_struct &ud )
{
	ASSERT( m_pUnitDB );
	ASSERT( ( phystype >= 0 ) && ( phystype < _UNITS_NUMBER_OF ) );

	if( !m_pUnitDB->AlreadyExist( phystype, ud ) )
	{
		return 0;
	}

	m_PhysType = phystype;
	m_Unit = ud;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Get unit information.
// To get the units information, the physical type and the unit design are returned.

void CDimValue::GetUnit( int *phystype, UnitDesign_struct *ud ) const
{
	*phystype = m_PhysType;
	*ud = m_Unit;
}

/////////////////////////////////////////////////////////////////////////////
// Acces to custom value.
// The custom value is the value expressed in the non standard unit.
// The calculation is "Value(with standard unit)" = Conv * ( "Value(with my unit)" + offset ).
// To find Offset and Conv, just say : X [my unit] = (X + Offset) * Conv [standard unit].

void CDimValue::SetCustomValue( double val )
{
	m_Value = ( val + GetOffsetOf( m_Unit ) ) * GetConvOf( m_Unit );
}

double CDimValue::GetCustomValue() const
{
	return ( m_Value / GetConvOf( m_Unit ) - GetOffsetOf( m_Unit ) );
}


// Convert the current unit to the default unit.
// return the custom value in default unit
inline double CDimValue::ConvertToDefaultUnit()
{
	m_Unit = m_pUnitDB->GetDefaultUnit( m_PhysType );
	return GetCustomValue();
}

// Conversion between a value expressed in custom units and a value in standard unit
// Custom unit to standard unit
double CDimValue::CUtoSI( int phystype, double CUValue )
{
	ASSERT( m_pUnitDB );
	ASSERT( ( phystype >= 0 ) && ( phystype < _UNITS_NUMBER_OF ) );

	UnitDesign_struct ud = m_pUnitDB->GetDefaultUnit( phystype );
	return ( CUValue + GetOffsetOf( ud ) ) * GetConvOf( ud );
}
// Standard to custom
double CDimValue::SItoCU( int phystype, double SIValue )
{
	ASSERT( m_pUnitDB );
	ASSERT( ( phystype >= 0 ) && ( phystype < _UNITS_NUMBER_OF ) );

	UnitDesign_struct ud = m_pUnitDB->GetDefaultUnit( phystype );
	return SIValue / GetConvOf( ud ) - GetOffsetOf( ud );
}

/////////////////////////////////////////////////////////////////////////////
// Get the value expressed in standard unit through the (double) type cast.

CDimValue::operator double() const
{
	return m_Value;
}

/////////////////////////////////////////////////////////////////////////////
// Assignement operator. Only the value is assigned.
// The unit design is unchanged.

CDimValue &CDimValue::operator=( const CDimValue &dv )
{
	m_Value = ( double )dv;
	return *this;
}

