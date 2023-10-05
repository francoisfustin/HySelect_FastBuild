#include "stdafx.h"

#include "utests_base.h"
#include "utests_CDBAdditChar.h"
#include "DataBObj.h"
#include "TASelect.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_CDBAdditChar, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_CDBAdditChar::IsADilute()
{
	IDPTR idWater = GetpTADB()->Get( _T("C_WATER") );
	CDB_AdditCharacteristic *pCDBWC = dynamic_cast<CDB_AdditCharacteristic *>( idWater.MP );
	CPPUNIT_ASSERT( pCDBWC );

	CPPUNIT_ASSERT_EQUAL( false, pCDBWC->IsaDilutedAddit() );

	IDPTR idEthyl = GetpTADB()->Get( _T("C_ETHYLGLYC") );
	CDB_AdditCharacteristic *pCDBEC = dynamic_cast<CDB_AdditCharacteristic *>( idEthyl.MP );
	CPPUNIT_ASSERT( pCDBEC );

	CPPUNIT_ASSERT_EQUAL( true, pCDBEC->IsaDilutedAddit() );

}

void utests_CDBAdditChar::IsMonotonic()
{
	IDPTR id = GetpTADB()->Get( _T("ADDIT_CHAR_TAB") );
	CTable *pTab = dynamic_cast<CTable *>( id.MP );

	for( IDPTR idptr = pTab->GetFirst(); *idptr.ID; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_AdditCharacteristic *pCDBAC = dynamic_cast<CDB_AdditCharacteristic *>( idptr.MP );
		CPPUNIT_ASSERT( pCDBAC );

		double minPc = pCDBAC->GetMinPc();
		double maxPc = pCDBAC->GetMaxPc();

		for( double dPc = minPc; dPc <= maxPc; dPc += 1.0 ) // Loop for dilution
		{
			double tfreeze = pCDBAC->GetTfreez( dPc );
			double tmax = pCDBAC->GetMaxT( dPc );

			double dDensity = 0.0;
			double dKinVisc = 0.0;
			double dSpecifHeat = 0.0;
			double dVaporPressure = 0.0;

			pCDBAC->GetAdditProp( dPc, tfreeze, &dDensity, &dKinVisc, &dSpecifHeat, &dVaporPressure );

			for( double dT = tfreeze; dT <= tmax; dT += 5.0 ) // Loop for temperature
			{
				double dD = 0.0;
				double dK = 0.0;
				double dSp = 0.0;
				double dV = 0.0;
				pCDBAC->GetAdditProp( dPc, dT, &dD, &dK, &dSp, &dV );

				// Density, SpecificHeat and VaporPressure are not monotonic
				CPPUNIT_ASSERT_EQUAL( true, dKinVisc >= dK );

				dDensity = dD;
				dKinVisc = dK;
				dSpecifHeat = dSp;
				dVaporPressure = dV;
			}
		}
	}
}

void utests_CDBAdditChar::NoAdditIsWater()
{
	// Test if dilute additive at 0 pc of dilution have same properties than water
	double dDensity = 0.0;
	double dKinVisc = 0.0;
	double dSpecifHeat = 0.0;
	double dVaporPressure = 0.0;

	IDPTR idWater = GetpTADB()->Get( _T("C_WATER") );
	CDB_AdditCharacteristic *pCDBWC = dynamic_cast<CDB_AdditCharacteristic *>( idWater.MP );
	CPPUNIT_ASSERT( pCDBWC );

	pCDBWC->GetAdditProp( 0.0/*dilution*/, 0.0/*temp*/, &dDensity, &dKinVisc, &dSpecifHeat, &dVaporPressure );

	IDPTR id = GetpTADB()->Get( _T("ADDIT_CHAR_TAB") );
	CTable *pTab = dynamic_cast<CTable *>( id.MP );

	for( IDPTR idptr = pTab->GetFirst(); *idptr.ID; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_AdditCharacteristic *pCDBAC = dynamic_cast<CDB_AdditCharacteristic *>( idptr.MP );
		CPPUNIT_ASSERT( pCDBAC );

		if( pCDBAC->GetMinPc() > 0.0 )
		{
			// Can't test if no data at 0% weight (The case of Greenway for example).
			continue;
		}

		if( false == pCDBAC->IsaDilutedAddit() )
		{
			continue;
		}

		double dD = 0.0;
		double dK = 0.0;
		double dSp = 0.0;
		double dV = 0.0;
		pCDBAC->GetAdditProp( 0.0/*dilution*/, 0.0/*temp*/, &dD, &dK, &dSp, &dV );
		CPPUNIT_ASSERT_DOUBLES_EQUAL( dDensity, dD, DOUBLE_DELTA_TOLERANCE );
		CPPUNIT_ASSERT_DOUBLES_EQUAL( dKinVisc, dK, DOUBLE_DELTA_TOLERANCE );
		CPPUNIT_ASSERT_DOUBLES_EQUAL( dSpecifHeat, dSp, DOUBLE_DELTA_TOLERANCE );

		if( 0.0 != dV ) // Test only if there is a value
		{
			CPPUNIT_ASSERT_DOUBLES_EQUAL( dVaporPressure, dV, DOUBLE_DELTA_TOLERANCE );
		}
	}
}

void utests_CDBAdditChar::CheckAdditChar( TCHAR *tAdditId, double dPc, double dT, double pdDensity, double pdKinVisc, double pdSpecifHeat,
		double pdVaporPressure, SurfValReturn_enum expectedRet )
{
	IDPTR idAddit = GetpTADB()->Get( const_cast<LPCTSTR>( tAdditId ) );
	CDB_AdditCharacteristic *pCDBWC = dynamic_cast<CDB_AdditCharacteristic *>( idAddit.MP );
	CPPUNIT_ASSERT( pCDBWC );

	double dDensity = 0.0;
	double dKinVisc = 0.0;
	double dSpecifHeat = 0.0;
	double dVaporPressure = 0.0;

	CPPUNIT_ASSERT_EQUAL( expectedRet, pCDBWC->GetAdditProp( dPc, dT, &dDensity, &dKinVisc, &dSpecifHeat, &dVaporPressure ) );

	if( expectedRet != SV_OK )
	{
		return;
	}

	CPPUNIT_ASSERT_DOUBLES_EQUAL( pdDensity, dDensity, DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( pdKinVisc, dKinVisc, DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( pdSpecifHeat, dSpecifHeat, DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( pdVaporPressure, dVaporPressure, DOUBLE_DELTA_TOLERANCE );

}

void utests_CDBAdditChar::TestAdditCharOnCurve()
{
	CheckAdditChar( _T("C_WATER"), 0.0, 70.0, 977.7, 0.420e-6, 4.190e+3, 3.120220e+04, SV_OK );
	CheckAdditChar( _T("C_MIXIGEL"), 1.0, 50.0, 1.264e+3, 2.250e-6, 2.990e+3, 0.0, SV_OK );
	CheckAdditChar( _T("C_ETHYLGLYC"), 45.5, 70.0, 1029.539, 9.780e-7, 3.591965e+3, 2.525531e+4, SV_OK );
	CheckAdditChar( _T("C_PROPYLGLYC"), 54.0, 70.0, 1004.46, 1.462e-6, 3.67659e+03, 3.113236e+4, SV_OK );
}

void utests_CDBAdditChar::TestAdditCharBetweenCurve()
{
	CheckAdditChar( _T("C_CACL2"), 3.0, 15.0, 1024.472916, 1.188e-6, 4011.0, 0.0, SV_OK );
}

void utests_CDBAdditChar::TestAdditCharInvalid()
{
#ifndef _DEBUG
	CheckAdditChar( _T("C_NACL"), -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, SV_OUTOFXRANGE );
	CheckAdditChar( _T("C_MGCL2"), 21.0, 0.0, 0.0, 0.0, 0.0, 0.0, SV_OUTOFXRANGE );
#endif
}
