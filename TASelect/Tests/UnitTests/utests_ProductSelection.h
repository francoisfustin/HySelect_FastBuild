#pragma once


#include <UnitTests/UTests_Common.h>
#include "taselect.h"
#include "Mainfrm.h"
#include "HMInclude.h"

class utests_ProductSelection : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_ProductSelection );
//	CPPUNIT_TEST( IndividualSelectionBCV );
	CPPUNIT_TEST( IndividualSelectionBV );
	CPPUNIT_TEST( IndividualSelectionCV );
	CPPUNIT_TEST( IndividualSelectionDpC );
	CPPUNIT_TEST( IndividualSelectionDpCBCV );
	CPPUNIT_TEST( IndividualSelectionPIBCV );
	CPPUNIT_TEST( IndividualSelectionPM );
	CPPUNIT_TEST( IndividualSelectionSafetyValve );
	CPPUNIT_TEST( IndividualSelectionSeparator );
	CPPUNIT_TEST( IndividualSelectionShutoffValve );
	CPPUNIT_TEST( IndividualSelectionTRV );
	CPPUNIT_TEST( IndividualSelection6WayValve ); // HYS-1375 : Add unit test for TA-6-way valve
	CPPUNIT_TEST( IndividualSelectionSmartControlValve ); // HYS-1515 : Add unit test for smart control valve
//	CPPUNIT_TEST( BatchSelectionBCV );
	CPPUNIT_TEST( BatchSelectionBV );
	CPPUNIT_TEST( BatchSelectionDpC );
	CPPUNIT_TEST( BatchSelectionPIBCV );
	CPPUNIT_TEST( BatchSelectionSeparator );
	CPPUNIT_TEST( BatchSelectionSmartControlValve );	// HYS-1515 : Add unit test for smart control valve
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();
	virtual void tearDown();

	void TestProductSelection();

	void IndividualSelectionBCV();
	void IndividualSelectionBV();
	void IndividualSelectionCV();
	void IndividualSelectionDpC();
	void IndividualSelectionDpCBCV();
	void IndividualSelectionPIBCV();
	void IndividualSelectionPM();
	void IndividualSelectionSafetyValve();
	void IndividualSelectionSeparator();
	void IndividualSelectionShutoffValve();
	void IndividualSelectionTRV();
	// HYS-1375 : Add unit test for TA-6-way valve
	void IndividualSelection6WayValve();
	// HYS-1515 : Add unit test for smart control valve
	void IndividualSelectionSmartControlValve();
	void BatchSelectionBCV();
	void BatchSelectionBV();
	void BatchSelectionDpC();
	void BatchSelectionPIBCV();
	void BatchSelectionSeparator();
	// HYS-1515 : Add unit test for smart control valve
	void BatchSelectionSmartControlValve();

	CTUProdSelLauncher m_clTUProdSelLauncher;

private:
	void _ExecuteTest( CString strInputFile );
};
