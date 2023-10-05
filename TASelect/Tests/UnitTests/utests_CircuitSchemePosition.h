#pragma once


#include <UnitTests/UTests_Common.h>
#include "CurvFit.h"
#include "SurfFit.h"


class utests_CircuitSchemePosition : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_CircuitSchemePosition );
	CPPUNIT_TEST( TestCircuitSchemePosition );
	CPPUNIT_TEST_SUITE_END();

public:
	void TestCircuitSchemePosition();
};
