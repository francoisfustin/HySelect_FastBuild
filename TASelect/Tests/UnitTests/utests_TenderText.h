#pragma once


#include <UnitTests/UTests_Common.h>

class utests_TenderText : public utests_base
{
	CPPUNIT_TEST_SUITE( utests_TenderText );
	CPPUNIT_TEST( CheckCharset );
	CPPUNIT_TEST( CheckIndexEncodingSupport );
	CPPUNIT_TEST( CsvFormatRule1 );
	CPPUNIT_TEST( CsvFormatRule2 );
	CPPUNIT_TEST( CsvFormatRule5 );
	CPPUNIT_TEST( CsvFormatRule6 );
	CPPUNIT_TEST( CsvFormatRule7 );
	CPPUNIT_TEST( CsvFormatEmpty1 );
	CPPUNIT_TEST( CsvFormatEmpty2 );
	//CPPUNIT_TEST(CheckTenderMissing);
	//CPPUNIT_TEST(CheckTenderNotUsed);
	//CPPUNIT_TEST(RemoveUnusedTender);
	CPPUNIT_TEST_SUITE_END();

public:
	void CheckIndexEncodingSupport();
	void CheckCharset();
	void CheckTenderMissing();
	void CheckTenderNotUsed();
	void RemoveUnusedTender();
	// Testing RFC 4180
	void CsvFormatRule1();
	void CsvFormatRule2();
	void CsvFormatRule5();
	void CsvFormatRule6();
	void CsvFormatRule7();
	void CsvFormatEmpty1();
	void CsvFormatEmpty2();
private:
	std::vector<_string> GetArtNumVector();
};
