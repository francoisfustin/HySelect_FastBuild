#pragma once


#include <UnitTests/UTests_Common.h>
#include "taselect.h"
#include "Mainfrm.h"

/**
*  @file    utests_TADBCopy.cpp/h
*  @author  Alen
*  @date    13/09/2018
*  @version 1.0
*
*  @brief Test unit for testing DB copy
*
*  @section DESCRIPTION
*	This unit test use standard tsp file as reference.
*	Open TSP, duplicate it and save it in out folder.
*	Output is then compared with the reference file saved in the data folder.
*	How to add a reference file
*		- Create/verify tsp, and save it in the data folder.
*/

class utests_TADBCopy : public utests_base
{
	CPPUNIT_TEST_SUITE(utests_TADBCopy);
	CPPUNIT_TEST(TestCopyOfTsp);
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();

	void TestCopyOfTsp();
protected:
private:
	CString _GetCpyFileName(CString strFN);
};
