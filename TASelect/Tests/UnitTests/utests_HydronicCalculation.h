#pragma once


#include <UnitTests/UTests_Common.h>
#include "taselect.h"
#include "Mainfrm.h"
#include "HMInclude.h"

/**
*  @file    utests_HydronicCalculation.cpp/h
*  @author  Alen
*  @date    28/03/2018
*  @version 1.0
*
*  @brief Test unit for covering existing circuits in Hydromod
*
*  @section DESCRIPTION
*	This unit test use standard tsp file as reference.
*	Open TSP, extract to a ".txt" file saved in out folder some representative values for each hydromod.
*	Output is then compared with the reference file saved in the data folder.
*	How to add a reference file
*		- Create/verify tsp, and save it in the data folder.
*		- run test unit on this tsp, this test unit will fail, but a ".txt" file (same name as tsp, extension is ".txt") with reference value will be dropped in out folder.
*		- copy this ".txt" into the data folder. It will be used as reference file for next run of Unit test
*/


class utests_HydronicCalculation : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_HydronicCalculation );
	CPPUNIT_TEST(TestHydraulicNetwork_CV);
	CPPUNIT_TEST(TestHydraulicNetwork_DPC);
	CPPUNIT_TEST(TestHydraulicNetwork_2WInj);
	CPPUNIT_TEST(TestHydraulicNetwork_3WDiv);
	CPPUNIT_TEST(TestHydraulicNetwork_3WInj);
	CPPUNIT_TEST(TestHydraulicNetwork_3WMix);
//	CPPUNIT_TEST(TestHydraulicNetwork_SmartControlValve);
	CPPUNIT_TEST(TestHydraulicNetwork_Pending);
	CPPUNIT_TEST(TestHydraulicNetwork_Other);
	CPPUNIT_TEST(TestHydraulicNetwork_AutoAdaptive);
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();
	virtual void tearDown();

	void TestHydraulicNetwork_CV();
	void TestHydraulicNetwork_DPC();
	void TestHydraulicNetwork_2WInj();
	void TestHydraulicNetwork_3WInj();
	void TestHydraulicNetwork_3WDiv();
	void TestHydraulicNetwork_3WMix();
	void TestHydraulicNetwork_SmartControlValve();
	void TestHydraulicNetwork_Pending();
	void TestHydraulicNetwork_Other();
	void TestHydraulicNetwork_AutoAdaptive();

protected:
	void _TestHydraulicNetwork( CString ExtPath );
	void _DropHMContent(CString strfn, bool bClearFile);
	void _FillHM( CFileTxt *pfw, CTable *pTab = NULL );
	void _WriteHMInfo( CFileTxt *pfw, CDS_HydroMod *pHM );
	void _WritePipeInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod *pHM, CDS_HydroMod::eHMObj ePipeLocation );
	void _WritePipeAccessoryInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod *pHM, CDS_HydroMod::eHMObj ePipeLocation );
	void _WriteDpCInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CDpC *pHMDpC );
	void _WriteBVInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CBV *pHMBV );
	void _WriteCVInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CCv *pHMCV );
	void _WriteShutOffValveInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CShutoffValve *pHMShutoffValve );
	void _WriteSmartControlValveInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CSmartControlValve *pHMSmartControlValve );

private:
	std::map<CString, CString> m_mapFolderToTest;
};
