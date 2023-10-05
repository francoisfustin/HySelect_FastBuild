#pragma once


#include <UnitTests/UTests_Common.h>

class utests_IDPTR : public utests_base
{
	CPPUNIT_TEST_SUITE(utests_IDPTR);
	CPPUNIT_TEST(idptrConstructorTest);
	CPPUNIT_TEST(idptrOperator);
	CPPUNIT_TEST(idptrMatch);
	CPPUNIT_TEST(idptrClear);
	CPPUNIT_TEST_SUITE_END();

public:
	void idptrConstructorTest();
	void idptrOperator();
	void idptrMatch();
	void idptrClear();
};

