#include "stdafx.h"

#include "utests_base.h"
#include "utests_CEquation.h"
#include "DataStruct.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_CEquation, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_CEquation::equationConstructorTest()
{
    __time32_t zeroTime;
    memset(&zeroTime, 0, sizeof(__time32_t));

    // Test default constructor
    {
        CEquation eq1;
        CPPUNIT_ASSERT_EQUAL(zeroTime, eq1.GetCalibrationDate());
        CPPUNIT_ASSERT_EQUAL(CEquation::EqT_None, eq1.GetEquationType());
        CPPUNIT_ASSERT_EQUAL('\0', eq1.GetEquationSize());
        CPPUNIT_ASSERT_EQUAL(CEquation::est_None, eq1.GetSensorType());
        CPPUNIT_ASSERT_EQUAL(uint8_t(0), eq1.GetSensorVersion());
        CPPUNIT_ASSERT_EQUAL(uint8_t(0), eq1.GetSensorMac());
    }

    // Test constructor : CEquation(eEquType EType, unsigned char ucOrder)
    {
        CEquation eq2(CEquation::EqT_Linear, 1);
        CPPUNIT_ASSERT_EQUAL(zeroTime, eq2.GetCalibrationDate());
        CPPUNIT_ASSERT_EQUAL(CEquation::EqT_Linear, eq2.GetEquationType());
        CPPUNIT_ASSERT_EQUAL(char(1), eq2.GetEquationSize());
        CPPUNIT_ASSERT_EQUAL(CEquation::est_None, eq2.GetSensorType());
        CPPUNIT_ASSERT_EQUAL(uint8_t(0), eq2.GetSensorVersion());
        CPPUNIT_ASSERT_EQUAL(uint8_t(0), eq2.GetSensorMac());
    }

}

void utests_CEquation::equationSizeTest()
{
    // Test equation size
    CEquation eq3(CEquation::EqT_Linear, 1);
    // order > MAXPOLYORDER
    CPPUNIT_ASSERT_EQUAL(false, eq3.SetEquationSize(11));
    CPPUNIT_ASSERT_EQUAL(char(1), eq3.GetEquationSize());
    // order <= MAXPOLYORDER
    CPPUNIT_ASSERT_EQUAL(true, eq3.SetEquationSize(10));
    CPPUNIT_ASSERT_EQUAL(char(10), eq3.GetEquationSize());
    // set last coef
    for (int i = 0; i < 10; i++)
    {
        CPPUNIT_ASSERT_EQUAL(true, eq3.SetEquationCoefs(i, i, i));
        CPPUNIT_ASSERT_EQUAL(unsigned short(i), eq3.GetEquationYCoefs(i));
        CPPUNIT_ASSERT_EQUAL(double(i), eq3.GetEquationCoefs(i));
    }

    CPPUNIT_ASSERT_EQUAL(false, eq3.SetEquationCoefs(10, 10, 10.0));
    CPPUNIT_ASSERT_EQUAL(unsigned short(0), eq3.GetEquationYCoefs(10));
    CPPUNIT_ASSERT_EQUAL(double(0.0), eq3.GetEquationCoefs(10));
    for (int i = 0; i < 10; i++)
    {
        // CAUTION : Y Coefficients are not modified.
        CPPUNIT_ASSERT_EQUAL(true, eq3.SetEquationCoefs(i, i + 1));
        CPPUNIT_ASSERT_EQUAL(unsigned short(i), eq3.GetEquationYCoefs(i));
        CPPUNIT_ASSERT_EQUAL(double(i + 1), eq3.GetEquationCoefs(i));
    }

}

void utests_CEquation::equationSolveTest()
{
    // Test solve
    CEquation eq4(CEquation::EqT_None, 10);

    // Test None
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, eq4.Solve(1.0), DOUBLE_DELTA_TOLERANCE);

    // Test Linear
    eq4.SetEquationType(CEquation::EqT_Linear);

    for (int i = 0; i < 10; i++)
    {
        // initialize coefficients
        CPPUNIT_ASSERT_EQUAL(true, eq4.SetEquationCoefs(i, i, i));
    }

    double startVal = 0.0;
    while (startVal < 10.0)
    {
        CPPUNIT_ASSERT_EQUAL(startVal, eq4.Solve(startVal));
        startVal += 0.1;
    }

    // Test Poly
    eq4.SetEquationType(CEquation::EqT_Poly);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(45.0, eq4.Solve(1.0), DOUBLE_DELTA_TOLERANCE);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(570361.79602475674, eq4.Solve(3.3), DOUBLE_DELTA_TOLERANCE);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(49408192.388671875, eq4.Solve(5.5), DOUBLE_DELTA_TOLERANCE);

    // Test Ntc
    eq4.SetEquationType(CEquation::EqT_Ntc);
    for (int i = 0; i < 10; i++)
    {
        // initialize coefficients
        CPPUNIT_ASSERT_EQUAL(true, eq4.SetEquationCoefs(i, i + 10.0));
    }
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-273.15, eq4.Solve(0.0), DOUBLE_DELTA_TOLERANCE);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-273.15, eq4.Solve(-1.0), DOUBLE_DELTA_TOLERANCE);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-273.14530178089478, eq4.Solve(5.5), DOUBLE_DELTA_TOLERANCE);
}

void utests_CEquation::equationOperatorTest()
{
    CEquation eq1(CEquation::EqT_Linear, 10);
    eq1.SetSensorType(CEquation::est_Dp);
    eq1.SetSensorVersion(125);
    eq1.SetSensorMac(126);

    __time32_t zeroTime;
    memset(&zeroTime, 0, sizeof(__time32_t));

    eq1.SetCalibrationDate(zeroTime);

    for (int i = 0; i < 10; i++)
    {
        eq1.SetEquationCoefs(i, i + 1, i + 2.0);
    }

    CEquation eq2 = eq1;
    CPPUNIT_ASSERT_EQUAL(CEquation::EqT_Linear, eq2.GetEquationType());
    CPPUNIT_ASSERT_EQUAL(char(10), eq2.GetEquationSize());
    CPPUNIT_ASSERT_EQUAL(CEquation::est_Dp, eq2.GetSensorType());
    CPPUNIT_ASSERT_EQUAL(uint8_t(125), eq2.GetSensorVersion());
    CPPUNIT_ASSERT_EQUAL(uint8_t(126), eq2.GetSensorMac());

    for (int i = 0; i < 10; i++)
    {
        CPPUNIT_ASSERT_EQUAL(i + 2.0, eq1.GetEquationCoefs(i));
        CPPUNIT_ASSERT_EQUAL(unsigned short(i + 1), eq1.GetEquationYCoefs(i));
    }
}

void utests_CEquation::equationCopyTest()
{
	CEquation eq1(CEquation::EqT_Linear, 10);
	eq1.SetSensorType(CEquation::est_Dp);
	eq1.SetSensorVersion(125);
	eq1.SetSensorMac(126);

	__time32_t zeroTime;
	memset(&zeroTime, 0, sizeof(__time32_t));

	eq1.SetCalibrationDate(zeroTime);

	for (int i = 0; i < 10; i++)
	{
		eq1.SetEquationCoefs(i, i + 1, i + 2.0);
	}

	CEquation eq2;
	eq1.Copy(&eq2);
	CPPUNIT_ASSERT_EQUAL(CEquation::EqT_Linear, eq2.GetEquationType());
	CPPUNIT_ASSERT_EQUAL(char(10), eq2.GetEquationSize());
	CPPUNIT_ASSERT_EQUAL(CEquation::est_Dp, eq2.GetSensorType());
	CPPUNIT_ASSERT_EQUAL(uint8_t(125), eq2.GetSensorVersion());
	CPPUNIT_ASSERT_EQUAL(uint8_t(126), eq2.GetSensorMac());

	for (int i = 0; i < 10; i++)
	{
		CPPUNIT_ASSERT_EQUAL(i + 2.0, eq1.GetEquationCoefs(i));
		CPPUNIT_ASSERT_EQUAL(unsigned short(i + 1), eq1.GetEquationYCoefs(i));
	}
}

//void utests_CEquation::equationSerializeWriteTest()
//{
//	CEquation eq1(CEquation::EqT_Linear, 10);
//	eq1.SetSensorType(CEquation::est_Dp);
//	eq1.SetSensorVersion(125);
//	eq1.SetSensorMac(126);
//
//	__time32_t zeroTime;
//	memset(&zeroTime, 0, sizeof(__time32_t));
//
//	eq1.SetCalibrationDate(zeroTime);
//
//	for (int i = 0; i < 10; i++)
//	{
//		eq1.SetEquationCoefs(i, i + 1, i + 2.0);
//	}
//
//	char outPath[_MAX_PATH] = "";
//	GetTempResultPath("equation", outPath);
//
//	std::ofstream eqwrite;
//	eqwrite.open(outPath, std::ofstream::out | std::ofstream::trunc);
//	eq1.Write(eqwrite);
//	eqwrite.close();
//
//	char refPath[_MAX_PATH] = "";
//	GetExpectedResultPath("equation", refPath);
//
//	CPPUNIT_ASSERT_EQUAL(true, IsFilesEqual(outPath, refPath));
//}
//
//void utests_CEquation::equationSerializeReadTest()
//{
//	char refPath[_MAX_PATH] = "";
//	GetExpectedResultPath("equation", refPath);
//
//	std::ifstream eqread;
//	eqread.open(refPath);
//
//	CEquation eq1;
//	eq1.Read(eqread);
//
//	eqread.close();
//
//	char outPath[_MAX_PATH] = "";
//	GetTempResultPath("equation", outPath);
//
//	std::ofstream eqwrite;
//	eqwrite.open(outPath, std::ofstream::out | std::ofstream::trunc);
//	eq1.Write(eqwrite);
//	eqwrite.close();
//
//	CPPUNIT_ASSERT_EQUAL(true, IsFilesEqual(outPath, refPath));
//}
