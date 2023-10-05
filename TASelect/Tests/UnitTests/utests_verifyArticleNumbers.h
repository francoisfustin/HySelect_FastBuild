#pragma once


#include <UnitTests/UTests_Common.h>

class utests_VerifyArticleNumbers : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_VerifyArticleNumbers );
	CPPUNIT_TEST( VerifyProductArticleNumbers );
	CPPUNIT_TEST_SUITE_END();

public:
	void VerifyProductArticleNumbers();

private:
	std::vector<CString>* m_pvectArticleList;
};
