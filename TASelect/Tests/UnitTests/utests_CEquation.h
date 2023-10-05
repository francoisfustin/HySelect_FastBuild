#pragma once


#include <UnitTests/UTests_Common.h>

class utests_CEquation : public utests_base
{
	CPPUNIT_TEST_SUITE(utests_CEquation);
	CPPUNIT_TEST(equationConstructorTest);
	CPPUNIT_TEST(equationSizeTest);
	CPPUNIT_TEST(equationSolveTest);
	CPPUNIT_TEST(equationOperatorTest);
	CPPUNIT_TEST(equationCopyTest);
	//CPPUNIT_TEST(equationSerializeWriteTest);
	//CPPUNIT_TEST(equationSerializeReadTest);
	CPPUNIT_TEST_SUITE_END();

public:
	void equationConstructorTest();
	void equationSizeTest();
	void equationSolveTest();
	void equationOperatorTest();
	void equationCopyTest();
	//void equationSerializeWriteTest();
	//void equationSerializeReadTest();
};
