#pragma once


#include <UnitTests/UTests_Common.h>

class CTUProdSelLauncher;
class CDlgSpecActHyToolsPMCompare;
class utests_HytoolsPMCompare : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_HytoolsPMCompare );
	CPPUNIT_TEST( TestHyToolsPMCompare );
	CPPUNIT_TEST_SUITE_END();

public:
	void VerifyResults( CString strFileNameIn, CString strFileNameTemp, CString strFileNameOut );
	void TestHyToolsPMCompare();
};

