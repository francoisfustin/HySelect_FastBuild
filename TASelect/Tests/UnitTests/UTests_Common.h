#pragma once


#include <msxml6.h>

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestAssert.h"
#include "cppunit/Portability.h"

#include "ZipArchive.h"
#include "utilities.h"


// #define  CPPUNIT_ASSERT(condition)												Assertions that a condition is true.
// #define  CPPUNIT_ASSERT_MESSAGE(message, condition)								Assertion with a user specified message.
// #define  CPPUNIT_FAIL(message)													Fails with the specified message.
// #define  CPPUNIT_ASSERT_EQUAL(expected, actual)									Asserts that two values are equals.
// #define  CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expected, actual)					Asserts that two values are equals, provides additional message on failure.
// #define  CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)					Macro for primitive double value comparisons.
// #define  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(message, expected, actual, delta)  Macro for primitive double value comparisons, setting a user - supplied message in case of failure.
// #define  CPPUNIT_ASSERT_THROW(expression, ExceptionType)							Asserts that the given expression throws an exception of the specified type.
// #define  CPPUNIT_ASSERT_THROW_MESSAGE(message, expression, ExceptionType)		Asserts that the given expression throws an exception of the specified type, setting a user supplied message in case of failure.
// #define  CPPUNIT_ASSERT_NO_THROW(expression)										Asserts that the given expression does not throw any exceptions.
// #define  CPPUNIT_ASSERT_NO_THROW_MESSAGE(message, expression)					Asserts that the given expression does not throw any exceptions, setting a user supplied message in case of failure.
// #define  CPPUNIT_ASSERT_ASSERTION_FAIL(assertion)								Asserts that an assertion fail
// #define  CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE(message, assertion)				Asserts that an assertion fail, with a user - supplied message in case of error.
// #define  CPPUNIT_ASSERT_ASSERTION_PASS(assertion)								Asserts that an assertion pass.
// #define  CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE(message, assertion)				Asserts that an assertion pass, with a user - supplied message in case of failure.

#define DOUBLE_DELTA_TOLERANCE 0.000001
#define CPPUNIT_ASSERT_DBL_EQUAL_TOLERANCE(x,y) CPPUNIT_ASSERT_DOUBLES_EQUAL(x,y,DOUBLE_DELTA_TOLERANCE)

#define CPPUNIT_TEST_CATEGORYNAME_ACTUATORS					"Actuators"
#define CPPUNIT_TEST_CATEGORYNAME_DATABASE					"Database"
#define CPPUNIT_TEST_CATEGORYNAME_HYDRONICCALCULATION		"Hydronic calculation"
#define CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS	"Import hydraulic circuits"
#define CPPUNIT_TEST_CATEGORYNAME_OUTPUT					"Output"
#define CPPUNIT_TEST_CATEGORYNAME_PRESSURISATION			"Pressurisation"
#define CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION			"Product selection"
#define CPPUNIT_TEST_CATEGORYNAME_TECHNICALPARAMETERS		"Technical parameters"

class CWaterChar;

class CSAXContentHandlerImpl;
class CRelationshipsXMLReader;
class utests_Init : public CPPUNIT_NS::TestFixture
{
public:
	static bool InitFluidWater( CWaterChar *pWC, double dTemp = 75 );
	static bool InitFluidGlycol( CWaterChar *pWC, double dTemp = -10 );
	static void InitDefaultDbValues();
	static void RestoreDefaultDbValues();
	
	// Create an output text file.
	// Path is created with the "strCategoryName" and "strCurrentTestClass" arguments -> See the "GetTxtFilePath" method just below.
	// File name is given by "strCurrentTestFile".
	// For example: strCategoryName = "Product selection"; strCurrentTestClass = "utests_ProductSelection"; strCurrentTestFile = "TestBatchSelBV.txt"
	//              The file created: "...UnitTests\Out\INT\Product Selection\utests_ProductSelection\TestBatchSelBV.txt".
	static CFileTxt *CreateOutTxtFile( CString strCategoryName, CString strCurrentTestClass, CString strCurrentTestFile );

	static CFileTxt *CreateOutTxtFile( CString strNewPath );

	// Retrieve the path name for the current category name and the current class tested.
	// For example: strCategoryName = "Product selection"; strCurrentTestClass = "utests_ProductSelection"
	//              The path is "...UnitTests\Data\INT\Product Selection\utests_ProductSelection\".
	static CString GetTxtFilePath( CString strCategoryName, CString strCurrentTestClass, bool bReference = false );
	
	static void CloseTxtFile( CFileTxt *pOutf );

	// Create an output binary file.
	// Path is created with the "strCategoryName" and "strCurrentTestClass" arguments -> See the "GetTxtFilePath" method just above.
	// File name is given by "strCurrentTestFile".
	// For example: strCategoryName = "Technical parameters"; strCurrentTestClass = "utests_PrjParas"; strCurrentTestFile = "PrjParamsReadWrite.bin"
	//              The file created: "...UnitTests\Out\INT\Technical parameters\utests_PrjParas\PrjParamsReadWrite.bin".
	static std::fstream *OpenBinFile( CString strCategoryName, CString strCurrentTestClass, CString strCurrentTestFile, bool bWrite );
	
	static void CloseBinFile( std::fstream* pfs );

	// Compare two files with the same name (strCurrentTestFile), one in the reference folder (Data) and the other in the out folder.
	// For example: strCategoryName = "Product selection"; strCurrentTestClass = "utests_ProductSelection"; strCurrentTestFile = "TestBatchSelBV.txt".
	//              Reference file : "...UnitTests\Data\INT\Product selection\utests_ProductSelection\TestBatchSelBV.txt".
	//              File to compare: "...UnitTests\Out\INT\Product selection\utests_ProductSelection\TestBatchSelBV.txt".
	static void CompareOutAndReferenceFiles( CString strCategoryName, CString strCurrentTestClass, CString strCurrentTestFile );

	// Here we don't have the current class name but directly the full path of files to compare (and the file name is only the name).
	static void CompareOutAndReferenceFiles2( CString strRefFile, CString strOutFile, CString strCurrentTestFile );
	
	// Compare two XML files with the same name (strCurrentTestFile), one in the reference folder (Data) and the other in the out folder.
	// For example: strCategoryName = "Output"; strCurrentTestClass = "utests_ExportInOneSheet"; strCurrentTestFile = "DirSel_AllSelection.xlsx".
	//              Reference file : "...UnitTests\Data\INT\Output\utests_ExportInOneSheet\DirSel_AllSelection.xlsx".
	//              File to compare: "...UnitTests\Out\INT\Output\utests_ExportInOneSheet\DirSel_AllSelection.xlsx".
	static void CompareOutAndReferenceXLSXFiles( CString strCategoryName, CString strCurrentTestClass, CString strCurrentTestFile );
	
// Private methods.
private:
	static void _ReadXMLFile( CZipArchive &zipArchive, WORD wIndex, CSAXContentHandlerImpl &clSAXContentHandlerImpl );
	static void _ExtractXMLToTempFile( CZipArchive &zipArchive, WORD wIndex, CString &strTempFileName );
	static void _CompareRelationships( std::map<int, CRelationshipsXMLReader> &mapReference, std::map<int, CRelationshipsXMLReader> &mapToCompare );
	static void _Compare2XMLFile( CString strRefTempFileName, CString strToCompTempFileName, CString strFileName, CString strXMLFileName );

	// Initialize an Unicode text file in the data folder linked to current utest_*** file
	// Example:
	// CurrentTestFile = ....\UnitTests\utests_sample.cpp
	// Text file will be
	// ....\UnitTests\Out\utests_sample\Filename
	static CString _CreateFullDirPath( CString strCategoryName, CString strCurrentClassName, CString strCurrentTestFile );

// Private variables.
private:
	static ISAXXMLReader *m_pSAXReader;
};

bool IsFilesEqual(CString lFilePath, CString rFilePath);

class CSAXErrorHandlerImpl : public ISAXErrorHandler  
{
public:
	CSAXErrorHandlerImpl() {}
	virtual ~CSAXErrorHandlerImpl() {}

	// This must be correctly implemented, if your handler must be a COM Object (in this example it does not)
	long __stdcall QueryInterface( const struct _GUID &, void ** ) { return 0; }
	unsigned long __stdcall AddRef( void ) { return 0; }
	unsigned long __stdcall Release( void ) { return 0; }

	virtual HRESULT STDMETHODCALLTYPE error( ISAXLocator *pLocator, const wchar_t *pwchErrorMessage, HRESULT hrErrorCode );
	virtual HRESULT STDMETHODCALLTYPE fatalError( ISAXLocator *pLocator, const wchar_t *pwchErrorMessage, HRESULT hrErrorCode );
	virtual HRESULT STDMETHODCALLTYPE ignorableWarning( ISAXLocator *pLocator, const wchar_t *pwchErrorMessage, HRESULT hrErrorCode ) { return S_OK; }
};

class CSAXContentHandlerImpl : public ISAXContentHandler  
{
public:
	CSAXContentHandlerImpl() {}
	virtual ~CSAXContentHandlerImpl() {}

	long __stdcall QueryInterface( const struct _GUID &,void ** ) { return 0; }
	unsigned long __stdcall AddRef( void ) { return 0; }
	unsigned long __stdcall Release( void ) { return 0; }

	virtual HRESULT STDMETHODCALLTYPE putDocumentLocator( ISAXLocator __RPC_FAR *pLocator ) { return S_OK; }
	
	virtual HRESULT STDMETHODCALLTYPE startDocument( void ) { return S_OK; }
	
	virtual HRESULT STDMETHODCALLTYPE endDocument( void ) { return S_OK; }
	
	virtual HRESULT STDMETHODCALLTYPE startPrefixMapping( const wchar_t __RPC_FAR *pwchPrefix, int cchPrefix, const wchar_t __RPC_FAR *pwchUri,
				int cchUri ) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE endPrefixMapping( const wchar_t __RPC_FAR *pwchPrefix, int cchPrefix ) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE startElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, const wchar_t __RPC_FAR *pwchLocalName,
				int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName, ISAXAttributes __RPC_FAR *pAttributes ) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE endElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, const wchar_t __RPC_FAR *pwchLocalName,
				int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName ) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE characters( const wchar_t __RPC_FAR *pwchChars, int cchChars ) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE ignorableWhitespace( const wchar_t __RPC_FAR *pwchChars, int cchChars ) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE processingInstruction( const wchar_t __RPC_FAR *pwchTarget, int cchTarget, const wchar_t __RPC_FAR *pwchData,
				int cchData ) { return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE skippedEntity( const wchar_t __RPC_FAR *pwchName, int cchName ) { return S_OK; }
	
	void RawName2CString( const wchar_t __RPC_FAR *pwchRawName, int cchRawName );

public:
	wchar_t m_tcValue[1000];
	CString m_strValue;
};

class CShareStringsXMLReader : public CSAXContentHandlerImpl  
{
public:
	CShareStringsXMLReader();
	virtual ~CShareStringsXMLReader() {}

	int GetStringNbr( void ) { return (int)m_vecShareStrings.size(); }
	CString GetString( int iIndex );

	virtual HRESULT STDMETHODCALLTYPE startElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, const wchar_t __RPC_FAR *pwchLocalName,
				int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName, ISAXAttributes __RPC_FAR *pAttributes );

	virtual HRESULT STDMETHODCALLTYPE endElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, const wchar_t __RPC_FAR *pwchLocalName,
				int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName );

	virtual HRESULT STDMETHODCALLTYPE characters( const wchar_t __RPC_FAR *pwchChars, int cchChars );

private:
	std::vector<CString> m_vecShareStrings;
	bool m_bSIStarted;
	bool m_bTStarted;
};

class CRelationshipsXMLReader : public CSAXContentHandlerImpl  
{
public:
	CRelationshipsXMLReader();
	virtual ~CRelationshipsXMLReader();

	typedef struct _RelationshipsAttributes
	{
		CString m_strID;
		CString m_strType;
		CString m_strTarget;
	}RelationshipsAttributes;

	RelationshipsAttributes GetRelationships( int iIndex );

	virtual HRESULT STDMETHODCALLTYPE startElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, const wchar_t __RPC_FAR *pwchLocalName,
				int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName, ISAXAttributes __RPC_FAR *pAttributes );

	virtual HRESULT STDMETHODCALLTYPE endElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, const wchar_t __RPC_FAR *pwchLocalName,
				int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName );

	virtual HRESULT STDMETHODCALLTYPE characters( const wchar_t __RPC_FAR *pwchChars, int cchChars );

private:
	std::vector<RelationshipsAttributes> m_vecRelationships;
};

class CXMLElementComparator
{
public:
	enum XMLElementType
	{
		XMLType_StartElement,
		XMLType_EndElement,
		XMLType_Character
	};
	
	CXMLElementComparator( wchar_t *tcElement, XMLElementType eXMLElementType );
	XMLElementType GetElementType( void ) { return m_eXMLElementType; }
	CString GetElementTypeStr( void );

	virtual wchar_t *GetElement( void ) { return _T(""); }
	virtual bool IsTheSame( CXMLElementComparator *pToCompare ) = 0;

	void RawName2CString( const wchar_t __RPC_FAR *pwchRawName, int cchRawName );

public:
	wchar_t m_tcValue[1000];
	XMLElementType m_eXMLElementType;
};

class CXMLStartElementComparator : public CXMLElementComparator
{
public:
	CXMLStartElementComparator( wchar_t *tcElement, ISAXAttributes *pStartElementAttributes );
	virtual ~CXMLStartElementComparator() {}

	wchar_t *GetStartElement( void ) { return m_tcValue; }
	ISAXAttributes *GetISAXAttributes( void ) { return m_pStartElementAttributes; }

	virtual wchar_t *GetElement( void ) { return GetStartElement(); }
	virtual bool IsTheSame( CXMLElementComparator *pToCompare );

private:
	ISAXAttributes *m_pStartElementAttributes;
};

class CXMLEndElementComparator : public CXMLElementComparator
{
public:
	CXMLEndElementComparator( wchar_t *tcElement );
	virtual ~CXMLEndElementComparator() {}

	wchar_t *GetEndElement( void ) { return m_tcValue; }

	virtual wchar_t *GetElement( void ) { return GetEndElement(); }
	virtual bool IsTheSame( CXMLElementComparator *pToCompare );
};

class CXMLCharacterElementComparator : public CXMLElementComparator
{
public:
	CXMLCharacterElementComparator( wchar_t *tcElement );
	virtual ~CXMLCharacterElementComparator() {}

	wchar_t *GetCharacter( void ) { return m_tcValue; }

	virtual wchar_t *GetElement( void ) { return GetCharacter(); }
	virtual bool IsTheSame( CXMLElementComparator *pToCompare );
};

class CThreadXMLSheetComparator : public CSAXContentHandlerImpl  
{
public:
	enum EngineState
	{
		ES_Nothing			= 0x0000,
		ES_Parsing			= 0x0001,
		ES_StartElement		= 0x0002,
		ES_Character		= 0x0003,
		ES_EndElement		= 0x0004,
		ES_FinishedOK		= 0x0005,
		ES_FinishedError	= 0x0006,
		ES_WaitComparaison	= 0x0100,
		ES_Stop				= 0x1000,
	};

	CThreadXMLSheetComparator();
	virtual ~CThreadXMLSheetComparator();
	
	void Start( CString strFileName, CEvent *pclCallerReadyToBeCompared, CEvent *pclCallerStop );
	void Continue( void );
	void Stop( void );

    bool CompareElements( CThreadXMLSheetComparator *pToCompThreadXMLSheetComparator );
	
	int GetCurrentState( void );
	CString GetCurrentStateStr( void );
	CXMLElementComparator *GetXMLElementComparator( void ) { return m_pXMLElementComparator; }
	
	static UINT ThreadXMLSheetParser( LPVOID pParam );

	virtual HRESULT STDMETHODCALLTYPE startElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, const wchar_t __RPC_FAR *pwchLocalName,
				int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName, ISAXAttributes __RPC_FAR *pAttributes );

	virtual HRESULT STDMETHODCALLTYPE endElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, const wchar_t __RPC_FAR *pwchLocalName,
				int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName );

	virtual HRESULT STDMETHODCALLTYPE characters( const wchar_t __RPC_FAR *pwchChars, int cchChars );

// Private methods.
private:
	void _WaitComparaison( void );

// Private variables.
private:
	CWinThread *m_pThread;
	
	// Events belonging to the thread.
	CEvent *m_pclThreadContinueEvent;
	CEvent *m_pclThreadStopEvent;
	
	// Events belonging to the caller. It allows the thread to set event when it is ready with an element to compare
	// with the other thread.
	CEvent *m_pclCallerReadyToBeCompared;

	// Events belonging to the caller. It allows the thread to set event to signal when it stops.
	CEvent *m_pclCallerStop;

	CRITICAL_SECTION m_CriticalSection;
	int m_iEngineState;
	ISAXXMLReader *m_pSAXReader;
	CSAXErrorHandlerImpl m_clSAXErrorHandlerImpl;
	CString m_szTempXMLFilename;

	// Element to compare.
	CXMLElementComparator *m_pXMLElementComparator;
};
