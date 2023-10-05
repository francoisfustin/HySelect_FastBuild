#pragma once


#include <UnitTests/UTests_Common.h>
#include "CurvFit.h"
#include "SurfFit.h"

class utests_CDBPageField : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_CDBPageField );
	CPPUNIT_TEST( ConstructorTest );
	CPPUNIT_TEST( WriteCompletePageField );
	CPPUNIT_TEST( Version13Test );
	CPPUNIT_TEST( Version14Test );
	CPPUNIT_TEST_SUITE_END();

public:
	void ConstructorTest();
	void WriteCompletePageField();
	void Version13Test();
	void Version14Test();

private:
	void ReadyToTest();
};

