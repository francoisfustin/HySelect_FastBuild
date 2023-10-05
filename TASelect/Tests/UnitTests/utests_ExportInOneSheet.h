#pragma once


#include <UnitTests/UTests_Common.h>

class utests_ExportInOneSheet : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_ExportInOneSheet );
	CPPUNIT_TEST( TestExportInOneSheet );
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();
	virtual void tearDown();

	void TestExportInOneSheet();

};

