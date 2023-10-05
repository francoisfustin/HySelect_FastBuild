#pragma once


#include <UnitTests/UTests_Common.h>

#define _TASRC_COMP_XML_FILE			_T("Localization\\TasRc-compl.xml")
#define _STRINGTAB_COMP_XML_FILE		_T("Localization\\TadbST-compl.xml")
#define _TASTIPS_COMP_XML_FILE			_T("Localization\\TasTips-compl.xml")

class utests_LocalizedString : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_LocalizedString );
	CPPUNIT_TEST(checkParameterCountRC);
	CPPUNIT_TEST(checkParameterCountTips);
	CPPUNIT_TEST(checkParameterCountST);
	CPPUNIT_TEST_SUITE_END();

public:
	void checkParameterCountRC();
	void checkParameterCountTips();
	void checkParameterCountST();

	void checkParameterCount( const char* xmlFile, const char* fileType );
private:

};

