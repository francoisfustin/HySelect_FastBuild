#pragma once


#include <UnitTests/UTests_Common.h>

// This class just help us to set the locale settings of the application to "en" before launching
// the test and to reset to the default one.
class utests_base : public CPPUNIT_NS::TestFixture
{
public:
	void setUp();
	void tearDown();

private:public:
	TCHAR m_tcLocaleName[ LOCALE_NAME_MAX_LENGTH];
	LCID m_CurrentLocale;
};
