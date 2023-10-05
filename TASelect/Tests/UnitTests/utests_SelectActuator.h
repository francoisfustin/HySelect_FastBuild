#pragma once


#include <UnitTests/UTests_Common.h>

class utests_SelectActuator : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_SelectActuator );
	CPPUNIT_TEST( getActuatorList );
	CPPUNIT_TEST( getCVActuatorList );
	CPPUNIT_TEST( getTRVActuatorList );
	CPPUNIT_TEST( getBCVActuatorList );
	CPPUNIT_TEST( getBALActuatorList );
	CPPUNIT_TEST( getDPCActuatorList );
	CPPUNIT_TEST( verifyCVActuatorSet );
	CPPUNIT_TEST( verifyPICVActuatorSet );
	CPPUNIT_TEST( verifyBCVActuatorSet );
	CPPUNIT_TEST_SUITE_END();

public:
	void getActuatorList();
	void getBCVActuatorList();
	void getTRVActuatorList();
	void getBALActuatorList();
	void getDPCActuatorList();
	void getCVActuatorList();
	void verifyCVActuatorSet();
	void verifyPICVActuatorSet();
	void verifyBCVActuatorSet();

	void verifyActuatorSet( TCHAR *actSet, TCHAR *valveTab );

};
