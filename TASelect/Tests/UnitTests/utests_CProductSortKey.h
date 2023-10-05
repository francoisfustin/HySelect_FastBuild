#pragma once


#include <UnitTests/UTests_Common.h>

class utests_CProductSortKey : public utests_base
{
	CPPUNIT_TEST_SUITE(utests_CProductSortKey);
	CPPUNIT_TEST(sortKeyAsDouble);
	CPPUNIT_TEST_SUITE_END();

public:
	void sortKeyAsDouble();
};
