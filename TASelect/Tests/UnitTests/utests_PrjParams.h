#pragma once


#include <UnitTests/UTests_Common.h>

class utests_PrjParams : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_PrjParams );
	CPPUNIT_TEST( TestClearOutputFolder );
	CPPUNIT_TEST( TestPrjParams );
	CPPUNIT_TEST( TestReadWritePrjParams );
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();
	virtual void tearDown();

	void TestClearOutputFolder();
	void TestPrjParams();
	void TestReadWritePrjParams();

private:
	CStringArray m_strList;

};
