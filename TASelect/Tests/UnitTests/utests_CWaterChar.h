#pragma once


#include <UnitTests/UTests_Common.h>

class utests_CWaterChar : public utests_base
{
	CPPUNIT_TEST_SUITE(utests_CWaterChar);
	CPPUNIT_TEST(watercharConstructorTest);
	CPPUNIT_TEST(DataSheetExpensionWater);
	CPPUNIT_TEST(DataSheetExpensionMEG);
	CPPUNIT_TEST(DataSheetExpensionMPG);
	CPPUNIT_TEST(DataSheetVapourWater);
	CPPUNIT_TEST(DataSheetVapourMEG);
	CPPUNIT_TEST(DataSheetVapourMPG);
	CPPUNIT_TEST(DataSheetDensityWater);
	CPPUNIT_TEST(DataSheetDensityMEG);
	CPPUNIT_TEST(DataSheetDensityMPG);
	CPPUNIT_TEST_SUITE_END();

public:
	void watercharConstructorTest();
	void GetExpCoefTest();
	void DataSheetExpensionWater();
	void DataSheetExpensionMEG();
	void DataSheetExpensionMPG();
	void DataSheetVapourWater();
	void DataSheetVapourMEG();
	void DataSheetVapourMPG();
	void DataSheetDensityWater();
	void DataSheetDensityMEG();
	void DataSheetDensityMPG();
};
