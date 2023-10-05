#pragma once


#include <UnitTests/UTests_Common.h>

class utests_CDBCloseOffDp : public utests_base
{
	CPPUNIT_TEST_SUITE(utests_CDBCloseOffDp);
	CPPUNIT_TEST(GetCloseOffDp);
	CPPUNIT_TEST_SUITE_END();

public:
	void GetCloseOffDp();
};

