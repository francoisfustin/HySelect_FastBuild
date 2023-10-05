#pragma once


#include <UnitTests/UTests_Common.h>

class utests_Printout : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_Printout );
	CPPUNIT_TEST( TestPrintOut );
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();
	virtual void tearDown();

	void TestPrintOut();

};
