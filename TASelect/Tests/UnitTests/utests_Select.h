#pragma once


#include <UnitTests/UTests_Common.h>

class utests_SelectPipeList : public utests_base
{
	CPPUNIT_TEST_SUITE(utests_SelectPipeList);
	CPPUNIT_TEST( TestPipeList );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void TestPipeList();

};


class utests_SelectPIBCVList : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_SelectPIBCVList );
	CPPUNIT_TEST( TestPIBCVList );
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();
	virtual void tearDown();

	void TestPIBCVList();
};
