#include "stdafx.h"

#include "utests_base.h"
#include "utests_CProductSortKey.h"
#include "DataBase.h"
#include "TASelect.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_CProductSortKey, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_CProductSortKey::sortKeyAsDouble()
{
	CProductSortKey psk1;
	CTAPSortKey tapsk1;

	CTAPSortKey::TAPSortKeyMask eMask = (CTAPSortKey::TAPSortKeyMask)(CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Size);
	psk1.InitWithDouble(0x1FFFFFFFF);
	tapsk1.InitWithDouble(0x1FFFFFFFF);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1048575.0, psk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4428136447.0, tapsk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);

	eMask = (CTAPSortKey::TAPSortKeyMask)(eMask & ~CTAPSortKey::TAPSKM_PN);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1044735.0, psk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4428120319.0, tapsk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);

	eMask = (CTAPSortKey::TAPSortKeyMask)(eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(983295.0, psk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4427088127.0, tapsk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);

	eMask = (CTAPSortKey::TAPSortKeyMask)(eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(255.0, psk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(4294967551.0, tapsk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);

	eMask = (CTAPSortKey::TAPSortKeyMask)(eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(255.0, psk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(255.0, tapsk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);

	eMask = (CTAPSortKey::TAPSortKeyMask)(eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_Fam);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, psk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, tapsk1.GetSortingKeyAsDouble(&CTAPSortKey::CTAPSKMask(eMask)), DOUBLE_DELTA_TOLERANCE);
}
