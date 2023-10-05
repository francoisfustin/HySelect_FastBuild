#pragma once


#include <UnitTests/UTests_Common.h>
#include "CurvFit.h"
#include "SurfFit.h"


class utests_CDBAdditChar : public utests_base
{
	CPPUNIT_TEST_SUITE(utests_CDBAdditChar);
	CPPUNIT_TEST(IsMonotonic);
	CPPUNIT_TEST(NoAdditIsWater);
	CPPUNIT_TEST(TestAdditCharOnCurve);
	CPPUNIT_TEST(TestAdditCharBetweenCurve);
	CPPUNIT_TEST(TestAdditCharInvalid);
	CPPUNIT_TEST(IsADilute);
	CPPUNIT_TEST_SUITE_END();

public:
	void IsMonotonic();
	void NoAdditIsWater();
	void TestAdditCharOnCurve();
	void TestAdditCharBetweenCurve();
	void TestAdditCharInvalid();
	void IsADilute();

private:
	void CheckAdditChar(TCHAR* tAdditId, double dPc, double dT, double pdDensity, double pdKinVisc, double pdSpecifHeat, double pdVaporPressure, SurfValReturn_enum expectedRet);
};

