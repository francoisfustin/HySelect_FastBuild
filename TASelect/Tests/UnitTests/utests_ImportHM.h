#pragma once


#include <UnitTests/UTests_Common.h>
#include "taselect.h"
#include "Mainfrm.h"
#include "HMInclude.h"

/**
*  @file    utests_ImportHM.cpp/h
*  @author  Awa
*  @date    13/05/2020
*  @version 1.0
*
*  @brief Test unit for covering import process
*
*  @section DESCRIPTION
*	This unit test use some bugs reported repeatedly to create scenarios
*/


class utests_ImportHM : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_ImportHM );
	CPPUNIT_TEST( ImportNewHMCalc );
	CPPUNIT_TEST( ImportNewNonHMCalc );
	CPPUNIT_TEST( ImportExistingHMCalcToHMCalC );
	CPPUNIT_TEST( ImportExistingNonHMCalcToNonHMCalc );
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp();
	virtual void tearDown();

	void ImportNewHMCalc();
	void ImportNewNonHMCalc();
	void ImportExistingHMCalcToHMCalC();
	void ImportExistingNonHMCalcToNonHMCalc();
	// Import
	void ImportForUnitTest( CString strFileNameToImport, CStringArray* pArrayParam, int iArraySize );

private:
	bool _ReadInputInfos( CStringArray* pListStr, int &nElt, CString strfn );
	bool _WriteInputInfos( CString strSrc, CString strDest );
	void _FillNonHMCalc( CFileTxt* pfw, CTable* pTab = NULL );
	void _WriteImportNewHMCalcInfo( CString strfn, CString strInfos, bool fClearFile, CString filename );
	void _WriteImportNewNonHMCalcInfo( CString strfn, CString strInfos, CString fileNameImported, bool fClearFile );
	void _WriteImportExistingHMCalcInfo( CString strfn, CString strInfos, bool fClearFile, CString filename );
	void _FillHMCalc( CFileTxt* pfw, CTable* pTab = NULL );
	void _WriteHMInfo( CFileTxt* pfw, CDS_HydroMod* pHM );
	void _WritePipeAccessoryInfo( CFileTxt* pfw, CString LineHeader, CPipes* pPipe );
	void _WritePipeInfo( CFileTxt* pfw, CString LineHeader, CPipes* pPipe );
	void _WriteDpCInfo( CFileTxt* pfw, CString LineHeader, CDS_HydroMod::CDpC* pDpC );
	void _WriteBVInfo( CFileTxt* pfw, CString LineHeader, CDS_HydroMod::CBV* pBV );
	void _WriteCVInfo( CFileTxt* pfw, CString LineHeader, CDS_HydroMod::CCv* pCV );

};
