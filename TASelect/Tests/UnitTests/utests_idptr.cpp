#include "stdafx.h"

#include "utests_base.h"
#include "utests_IDPTR.h"
#include "DataBase.h"
#include "TASelect.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_IDPTR, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_IDPTR::idptrConstructorTest()
{
	IDPTR id;
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id.MP);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id.DB);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id.PP);
	CPPUNIT_ASSERT_EQUAL(TCHAR(0), id.ID[0]);

	IDPTR id2( _string(_T("TEST")) );
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id2.MP);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id2.DB);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id2.PP);
	CPPUNIT_ASSERT_EQUAL(TCHAR('T'), id2.ID[0]);
	CPPUNIT_ASSERT_EQUAL(TCHAR('E'), id2.ID[1]);
	CPPUNIT_ASSERT_EQUAL(TCHAR('S'), id2.ID[2]);
	CPPUNIT_ASSERT_EQUAL(TCHAR('T'), id2.ID[3]);
	CPPUNIT_ASSERT_EQUAL(TCHAR(0), id2.ID[4]);

	TCHAR test2[_ID_LENGTH + 1] = _T("TWO");
	IDPTR id3(test2);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id3.MP);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id3.DB);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id3.PP);
	CPPUNIT_ASSERT_EQUAL(TCHAR('T'), id3.ID[0]);
	CPPUNIT_ASSERT_EQUAL(TCHAR('W'), id3.ID[1]);
	CPPUNIT_ASSERT_EQUAL(TCHAR('O'), id3.ID[2]);
	CPPUNIT_ASSERT_EQUAL(TCHAR(0), id3.ID[3]);
}

void utests_IDPTR::idptrOperator()
{
	IDPTR id;
	IDPTR id2(_string(_T("TEST")));

	CPPUNIT_ASSERT_EQUAL(false, id != id);
	CPPUNIT_ASSERT_EQUAL(true, id != id2);

	CPPUNIT_ASSERT_EQUAL(true, id == id);
	CPPUNIT_ASSERT_EQUAL(false, id == id2);
}

void utests_IDPTR::idptrMatch()
{
	IDPTR id;
	IDPTR id2(_string(_T("TEST")));

	CPPUNIT_ASSERT_EQUAL(false, id.IDMatch(id2.ID));
	CPPUNIT_ASSERT_EQUAL(true, id2.IDMatch(id2.ID));
}

void utests_IDPTR::idptrClear()
{
	IDPTR id = TASApp.GetpTADB()->Get(_T("WATER"));
	CPPUNIT_ASSERT(id.MP);
	CPPUNIT_ASSERT(id.DB);
	CPPUNIT_ASSERT(id.PP);
	CPPUNIT_ASSERT(id.ID[0]);
	id.Clear();
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id.MP);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id.DB);
	CPPUNIT_ASSERT_EQUAL((void*)0, (void*)id.PP);
	CPPUNIT_ASSERT_EQUAL(TCHAR(0), id.ID[0]);
}
