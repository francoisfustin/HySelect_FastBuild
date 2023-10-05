#pragma once


#include <UnitTests/UTests_Common.h>

class utests_ExpansionPipeSize : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_ExpansionPipeSize );
	CPPUNIT_TEST( TestExpansionPipeSize );
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();
	virtual void tearDown();
	void TestExpansionPipeSize();
};
