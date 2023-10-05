#ifndef DATABASE_H__INCLUDED_
#define DATABASE_H__INCLUDED_

#include <string>
#include <typeinfo>

#include "global.h"			// needed for TA-CBX
#include "utilities.h"	
extern bool g_bSBCSFile;
extern bool g_bWriteTACBXMode;

#ifndef TACBX
#include "DataBase.hpp"
#endif

class CTADatabase;
extern CTADatabase m_TADB;
CTADatabase *GetpTADB(); 

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	General purpose functions
//	
//////////////////////////////////////////////////////////////////////////////////////////////
#define _NO_ID	_T("NO_ID")
#define _OLD_ID_LENGTH 15
#define _ID_LENGTH 23				// the maximum length for the ID. The ID is a null terminated string.
#define _CLASS_NAME_LENGTH 95		// the maximum length for the class names.
#define _DATABASE_VERSION_LENGTH 31	// the maximum length for database version.
#define _NAME_MAX_LENGTH 40			// Used as maximum size for name into TA Scope Logging
#define _COMBO_ITEMNAME_MAX 31		// the maximum length for a text in a combo box.
#define _DB_OBJECTSEPARATOR 0x26
#define _DB_ENDMARKER 0x23
#define _ID(arg) _T(#arg)

inline int IDcmp( const TCHAR *p1, const TCHAR *p2 )
{
	for( int j = 0; j < _ID_LENGTH; j++, p1++, p2++ )
	{
		if( 0 == *p1 && 0 == *p2 )
			return 0;
		
		if( 0 == *p1 )
			return -1;
		
		if( 0 == *p2 )
			return 1;
		
		if( *p1 == *p2 )
			continue;
		
		if( *p1 < *p2 )
			return -1;
		
		if( *p1 > *p2 )
			return 1;
	}
	return 0;
}

// Write string to disk .
void WriteString( OUTSTREAM outf, LPCTSTR ptcString );


template <typename T> void WriteData( OUTSTREAM outf, T value, int iLen=-1 )
{
	char *pchar = reinterpret_cast<char*>(&value);
	if ( -1 == iLen )
	{
		iLen = sizeof(value);
	}
	outf.write( pchar, iLen );
};

// Read a string from disk. 
bool ReadString( INPSTREAM inpf, LPTSTR ptcString, int iMaxLen );
bool ReadString( INPSTREAM inpf, _string &str );

#ifndef TACBX
bool ReadString( INPSTREAM inpf, CString &str );
#endif

template <typename T> void ReadData( INPSTREAM inpf, T &value )
{
	char buf[sizeof( value )] = { 0 };
	inpf.read( buf, sizeof( value ) );
	T *pT = reinterpret_cast<T *>( buf );
	value = *pT;
};

// Read/Write double taking into account endian organization for ARM processor.
double ReadDouble( INPSTREAM inpf );
void WriteDouble( OUTSTREAM outf, double d );

/////////////////////////////////////////////////////////////////////////////
// Cleans a string from double spaces, etc.
/////////////////////////////////////////////////////////////////////////////
TCHAR *PurgeString( TCHAR *ptcBuffer, LPCTSTR ptcSource, int iBufferLength );

////////////////////////////////
#ifndef TACBX
void WriteFormatedStringW( OUTSTREAM outf, CString str, CString strTab = _T(""), bool bWithQuotes = false );
void WriteFormatedStringA( OUTSTREAM outf, CString str, CString strTab = _T(""), bool bWithQuotes = false );
void WriteFormatedStringA2( OUTSTREAM outf, CString str, CString strTab = _T("") );
bool InheritedID( TCHAR *pline, CString &ID );

// Read a line from a text file.
// All text after // is ignored. Spaces and tab TCHAR are canceled.
// A blank line is ignored. Return 0 if end of file is reached.
// LineCount is incremented of the number of read lines (blank or not).
// LPTSTR ReadTextLine( INPSTREAM  inpf, unsigned short* pusLineCount/*, bool fExcludeTab = true*/ );
// void ReadDoubleFromTextLine( INPSTREAM inpf, double &dValue, unsigned short* pusLineCount );
// void ReadIntFromTextLine( INPSTREAM inpf, int &iValue, unsigned short* pusLineCount );

// Parse a line with fields delimited by a backslash. 
// Parsing takes place for nField fields. If nField==0, the number of fields 
// is automatically detected. A vector of pointers to the beginning of 
// each field is returned.
TCHAR **ParseTextLine( LPTSTR ptcLine, int *piField, TCHAR tcSeparator = _T('\\') );

// Convert a line composed of 0 & 1 to an integer
int InterpretBinaryLine(CString str);


#endif

/*////////////////////////////////////////////////////////////////////////////////////////////

	This file contains declarator for database facilities.

	CLASS
			CData			This is the root class for all other objects
							which have to be incorporated in a database.
			
			CTable			This is derived from CData (so it is a data type class).
							This object is a collection of any CData object.
			
			CDataBase		This is the database.
							The database object is the starting point to access information.
							
			IDPtr			is a structure. It is an extended definition of a pointer.
							It contains an ID identificator, a CData and a CDatabase pointers.
							The ID is managed by the CDatabase. It is a Primary Key.

*/////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	IDPtr is an extended definition of the pointer.
//		ID is the an object identificator. This ID is the "exportable on disk" pointer.
//		MP is the memory pointer to the CData object.
//		DB is the database object which the CData object belongs to.
//		PP is a pointer on Table that contains the object	
//////////////////////////////////////////////////////////////////////////////////////////////
class CData;		// later defined
class CDataBase;	// later defined
class CTable;		// later defined
class IDPTR
{
public:
	IDPTR();
	IDPTR( _string ID );
	IDPTR( TCHAR *pID );
	~IDPTR() {}
	
	bool operator!=( const IDPTR& idptr ) const;
	bool operator==( const IDPTR& idptr ) const { return !operator!=( idptr ); }
	bool IDMatch(_string IDstr);
	void Clear();
	void SetID( _string ID ) { SetID( (TCHAR *)ID.c_str() ); }
	void SetID( TCHAR *pID );
	template<typename T>T* GetAs() { return dynamic_cast<T*>(MP); }
	bool Read(INPSTREAM inpf, CTADatabase *pTADB);

#ifndef TACBX
	bool ReadTxt( INPSTREAM inpf, unsigned short *pusLineCount, CTADatabase *pTADB );
#endif

	// Public variables.
public:
	TCHAR ID[_ID_LENGTH+1];			// Identificator
	CData *MP;						// Memory pointer
	CDataBase *DB;					// Pointer on database
	CTable *PP;						// Pointer on parent table
};

extern const IDPTR _NULL_IDPTR;	// The null extended pointer.

// Check the validity and the integrity of an extended pointer.
// If the memory pointer is not build, complete it.
bool Extend( IDPTR* idptr );

bool ReadIDPtr(INPSTREAM inpf, IDPTR *pIDPTR, CTADatabase *pTADB);


//////////////////////////////////////////////////////////////////////////////////////////////
//
//	CDataBase class.
//
//	The database is a collection of CData derived class object.
//	The CDataBase manages all the object which are created. A pointer to the created object
//	is inserted in a set (Implemented in nested class CDataList). It is not possible to directly
//	access this set. The CDataBase access is possible through its CTable member object called
//	the Root Table. See CTable class properties for more information on how to work with it.
//
//	Construction
//		Construction automatically creates a root table with a predifined ID. If you Read
//		a database from the disk, the complete database contebt is substitued and the 
//		root table ID is as specified in the file.
// 		Use the Init() function to empty the database and reinitialise it.
//		It produces the same effect as destroying then constructing the database 
//
//	Creation/Destruction
//		Always create a CData derived class object with function CreateObject and delete
//		it with the DeleteObject function. Since the CData constructor is protected, you have
//		no other choice.
//		During database destruction, the destructor call the virtual OnDatabaseDestroy() function
//		for each object in the database
//
//	Version identification
//		Use GetVersion to know which version of the database it is
//
//	Access rules
//		You have access to the root table of the databes through Access and operator() functions
//		You can also retrieve an object from its ID (see Get)
//
//	Modified state
//		Use IsModified to know if the database has been modified
//		Use Modified to set the modified flag to true (default) or false
//
//	Updating function
//		Use the UpdatePrices function to update the prices
//
//	Disk IO
//		Use Write when you want to drop the database in a file and use Read when you want to
//		load the database from the disk. In this case, the database is firstly cleared !
//		Use ReadText when you want to load a text formatted database.
//		Use DropData to drop the database data to a text file				
//			
//////////////////////////////////////////////////////////////////////////////////////////////
class CTable;	// later defined

#define _ROOT_TABLE _T("{ROOT_TABLE}")	// The root table default ID.

class CDataList
{
public:
	CDataList();
	~CDataList();
	
	// Retrieve a complete extended pointer from the ID only.
	const IDPTR &GetIDPtrFromID( LPCTSTR ptcID );
	
	// Get the first CData derived object.
	const IDPTR &GetFirst( unsigned short &usPos );
	const IDPTR &GetFirst();

	// Get the next CData derived object.
	const IDPTR &GetNext( unsigned short &usPos );
	const IDPTR &GetNext();
	
	// Re-Initialize cursor position.
	int GetPos();
	void SetPos( int iPos );
	void SetPos( LPCTSTR ptcID );
	
	// Insert a valid CData derived object in the list. Return IDPtr.
	// const IDPTR& Insert(const IDPTR& IDPtr);
	void Insert( const IDPTR &IDPtr );
	
	// Delete a valid CData derived object from the list. Return IDPtr.
	const IDPTR &Remove( LPCTSTR ptcID );
	const unsigned short GetCount() { return m_usDataAmount; }
		
	// Tool.
	// Create an ID which does not exist yet.
	LPCTSTR CreateID();
	
	// Make the list empty.
	void MakeEmpty() { delete [] m_ppDataArray; m_ppDataArray = 0; m_usArraySize = m_usDataAmount = 0; }
		
// Private methods.
private:
	// Locate an ID in the m_DataArray array. pos will contain the position in the array
	// Return zero if ID was not correctly found.
	// In this case pos is the position where ID should be inserted regarding alphabetical order
	int Locate( LPCTSTR ptcID, unsigned short *pusPos );

// Private variables.
private:
	// This is an array to pointers of CData derived objects.
	// Each created CData derived object is inserted in this table.
	CData **m_ppDataArray;				// array of CData pointers
	unsigned short m_usArraySize;		// Dynamically allocated size of the m_DataArray array
	const BYTE m_ArrayStep;				// The memory allocation step.
	unsigned short m_usDataAmount;		// number of element in the m_DataArray array
	short m_sCursor;					// Used in GetFirst/GetNext function
};

#define DBFH_APPNAME		_T("HySelect")
#define DBFH_OLDAPPNAME		_T("TA Select 4")
class CDBFileHeader
{
public:
	CDBFileHeader();
	virtual ~CDBFileHeader() {}

	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf, short nVersion );

	// GETTER.
	LPCTSTR GetVersion( void ) { return m_strVersion; }
	_string GetUID( void ) { return m_strUID; }

#ifndef TACBX
	CString GetAppName( void ) { return m_strAppName; }
	CString GetTADBKey( void ) { return m_strTADBKey; }
	CString GetAppVersionStr( void ) { return m_strAppVersion; }
	int GetAppVersionNbr( void );
	CString GetDBArea( void ) { return m_strDBArea; }
	CString GetDBVersion( void ) { return m_strDBVersion; }
	CString GetHUBSchemesDLLName( void ) { return m_strHUBSchemesDLLName; }
	CString GetHUBSchemesDLLVersion( void ) { return m_strHUBSchemesDLLVersion; }
#endif

	// SETTER.
	void SetVersion( LPCTSTR strVersion ) { wcscpy_s(m_strVersion, strVersion ); }
	void SetUID( _string strUID ) { m_strUID = strUID; }

// Private methods.
private:
#ifndef TACBX
	void _ExtractInfo( LPTSTR str );
#endif

// Private variables.
private:
	// This is the version identifier for the database.
	// You can assign a version identifier to make the link between the database and the document produced thanks to it.
	TCHAR m_strVersion[_DATABASE_VERSION_LENGTH + 1];
	_string m_strUID;							// Unique identifier

#ifndef TACBX
	CString m_strAppName;
	CString m_strTADBKey;
	CString m_strAppVersion;
	CString m_strDBArea;
	CString m_strDBVersion;
	CString m_strHUBSchemesDLLName;
	CString m_strHUBSchemesDLLVersion;
#endif
};

class CDataBase
{
public:
	enum ChildID
	{
		TADatabase,
		UserDatabase,
		PipeUserDatabase,
		TAMetaData,
		TAPersistData,
		TADatastruct,
		TADatastructX,
	};

	CDataBase( ChildID eChildID );
	virtual ~CDataBase();

	void DuplicateDataBaseTO(CDataBase *pDBto);
	ChildID GetDatabaseType(void) { return m_eChildID; }
	void Init();
	
	bool IsSuperUserPsw() { return m_bSuperUserPsw; }
	void SetSuperUserPsw( bool bflag ) { m_bSuperUserPsw = bflag; }
	
	bool IsDebugPsw() { return m_bDebugPsw; }
	void SetDebugPsw( bool bflag ) { m_bDebugPsw = bflag; }

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Creation and destruction of CData derived objects.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Create a CData derived object. If ID is NULL, the ID is automatically generated.
	// Return an extended pointer to the object. Return null extended pointer if not succeed.
	void CreateObject( IDPTR &idptr, LPCTSTR ptcClassName, LPCTSTR ptcID = 0 );

	// Delete object and prevent to delete the object if it is locked.
	// Return non zero if the delete operation succeed. ExtID is no more valid after that.
	// Use of bForce=true should be exceptional used with caution, Owner is not checked!!
	int DeleteObject( IDPTR& ExtID, bool bForce = false );
	int DeleteObjectRecursive( CData *pData, bool bForce = false );

#ifndef TACBX
	// Remove CData object from TADB.
	void RemoveCDataObject( CArray<CData *> *pAr );
#endif

	void CheckCrossReferenceID();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Version identification.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// m_UID is the unique identifier of Datastruct.
	_string GetUID() { return m_UID; }
	LPCTSTR GetVersion() { return m_tcVersion; }
	void SetVersion( LPCTSTR Version ) { memset( m_tcVersion, 0, _DATABASE_VERSION_LENGTH + 1); _tcsncpy_s( m_tcVersion, SIZEOFINTCHAR( m_tcVersion ), Version, _DATABASE_VERSION_LENGTH ); }

	// Stored also into the CDatastructX it give us a way to match files between TAScope and TAS.
	void SetUID( _string uid ) { m_UID = uid; }

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Access.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Access to the root table.
	CTable &Access() { ASSERT( m_pRootTable ); return *m_pRootTable; }
	CTable &operator()() { ASSERT( m_pRootTable ); return *m_pRootTable; }

	// Access an object through its ID.
	const IDPTR &Get( LPCTSTR ID ) { return (*ID) ? m_DataSet.GetIDPtrFromID(ID) : _NULL_IDPTR; }

	// Limited access for scanning all stored object
	CDataList *GetpDataList() { return &m_DataSet; }
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Modified state.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	BYTE IsModified() { return m_IsModified; }
#ifndef TACBX
	void Modified( BYTE bModified = 1 ) { m_IsModified = bModified != 0; if( bModified ) m_bRefreshResults = true; }
	void RefreshResults( bool bRefresh ) { m_bRefreshResults = bRefresh; }
	bool IsRefreshResults() { return m_bRefreshResults; }
#else
	void Modified( BYTE bModified = 1) { m_IsModified = bModified != 0; }
#endif

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Read and write on the disk.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void Write( char *fn );
	virtual void Write( OUTSTREAM outf );
	void WriteFileIdentificator( OUTSTREAM outf, short Version );
	virtual void WriteHeader( OUTSTREAM outf );
	virtual void WriteObjects( OUTSTREAM outf );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Buffered read.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// TACBX : use ReadFileFast()
#ifndef TACBX
	virtual void Read( TCHAR *fn );
#else
	virtual void Read( char *fn );
#endif
	
	virtual void Read( INPSTREAM inpf );
	static short ReadFileIdentificator( INPSTREAM  inpf );
	bool ReadHeader( INPSTREAM  inpf, short Version );
	virtual void ReadObjects( INPSTREAM inpf, short nVersion );

	// Check the file validity and the database format version.
	// return 0 if the file is correct.
	// return -1 in case of invalid binary file.
	// return -2 in case of the file format version is more recent than the current version.
	enum CheckDBFileRetCode
	{
		OkSbcs = 1,
		Ok = 0,
		Invalid = -1,
		TooRecent = -2,
		NotOkUnicode = -3,
		FileNotFound = -4
	};

	void CrossVerifyID();

#ifndef TACBX
	static int CheckDBFile( INPSTREAM inpf );
	LPTSTR Append( INPSTREAM inpf, unsigned short LineStart = 0, bool bReadingObjectFromDeletedFile = false );
	void ReadText( INPSTREAM inpf );
	void RemoveAllEnumDefinition();
	
	// Drop the data to a text file.
	void DropData( OUTSTREAM outf );
	int ReadEnum( CString str );
	int ReadEnumLine( INPSTREAM  inpf, unsigned short *pusLineCount );
	void ParseEnumStrLine( INPSTREAM  inpf, CString strLine, int &iKey, CString &strEnum );
	void ApplyFilters();
	
	// Cross verification of all CTable.
	// Only used in Debug mode
	void VerifyTable();

	CDBFileHeader *GetDBFileHeader( void ) { return &m_clDBFileHeader; }
#ifdef DEBUG
	// FF: I wrote this method to allow to export all object present in m_DataSet.
	// Param: 'strFileName' is where to export.
	// Param: 'bOnlyNoOwner' set to 'true' if you want only object that has no owner.
	void ExportDataSet( CString strFileName, bool bOnlyNoOwner );

	void ExportAllArticleNumber( CString strFileName );

#endif

   /**
    * This function Search DB elements by article number and put Ids in a CArray
    * @author awa
     * @param  (I) strArticleNumber: The article number to search
    * @param  (O) pArrstr         : List of IDs found
    * @remarks : Created by HYS791
    */
	void SearchByArticleNumber(CString strArticleNumber, CArray <CString> *pArrstr);

	// Loop all objects in 'm_DataSet' and ASSERT as soon as one of them has no owner.
	void VerifyDataSetIntegrity( void );
#endif //TACBX////////////////////////////////////////////////////////
	// Delete all objects which are in the database.
	void MakeEmpty();

// Protected members.
protected:
	void CreateObjectAndInsertInTable( CDataBase *pDBto, IDPTR idptrObj );

// Private variables.
private:
#ifndef TACBX
	// Use to store temporary value of enum.
	// Format of enum entry used in TADB.TXT is "## Enum_Name = 123".
	// Enum entry must be read before database object.
	CMapStringToPtr	m_MapOfEnum;
#endif
	
	// Use to refresh the results sheet. By this way you avoid to recalculate the cells each time you want to display the results.
	bool m_bRefreshResults;
	
	// Set to non zero if the database has been modified.
	BYTE m_IsModified;
	
	// The root Table is an object which is used as the first access to the database.
	// It is created during construction or it is loaded from the disk.
	CTable *m_pRootTable;
	
	// The data set is a collection of CData derived objects.
	CDataList m_DataSet;

	// Allow to know what is the inherited class. It is used for example in the 'CDatabase::ReadObjects()' method.
	ChildID m_eChildID;

// Protected variables.
protected:
	// This is the version identificator for the database.
	// You can assign a version identificator to make the link between the database and the document produced thanks to it.
	TCHAR m_tcVersion[_DATABASE_VERSION_LENGTH + 1];
	_string m_UID;							// Unique identifier

	bool m_bSuperUserPsw;
	bool m_bDebugPsw;
	CDBFileHeader m_clDBFileHeader;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	CData is the basic data object class.
//	All classes which are derived from CData can be incorporated in the database
//
//	Construction
//		When you construct a CData object or derived, you have to specify its owner database
//		and its well qualified ID in this database. The constructor is protected. You have to use
//		CDataBase::CreateObject(...) to dynamically create a CData derived class object.
//
//	Database destruction
//		During database destruction and before any CData object is destroyed, the OnDatabaseDestroy()
//		function is called.
//
//	Identification
//		*GetIDPtr return an extended pointer to the object
//		*IsClass identify the class of the object.
//		*GetDB return a pointer to the database the object belongs to.
//
//	Locking function
//		You can lock and unlock an object. An object is locked by another thanks to the Lock()
//		function. The locked object remember which object locked it. Unless the object is unlocked
//		by the same object, the object cannot be destroyed. An object can be locked by several
//		other ones. An object can lock itself too. See Lock, Unlock and IsLocked functions.
//
//	Modification state.
//		When you modify the content of an object, call Modified() to warn the database it has been changed.
//
//	Status Functions.
//		If you want to know if a data object is priced call IsPriced() virtual function
//		Use Get/SetPrice() function to change it
//
//	read/Write on disk functions
//		When you want to write an object to the disk, call DumpOnDisk. This function will write specific 
//		CData information and will call the virtual Write function. So for derived object you have
//		to overwrite the Write function. Do not forget to call base class Write function from overwritten
//		When you want to read an object from the disk, do not allocate an instance of the object (you
//		do not know which class it is). Call the static ExtractFromDisk function. The object will be 
//		automatically created by the function and inserted in the database. Macros DECLARE_DATA and
//		IMPLEMENT_DATA are part of this mechanism.
//		Call ExtractFromTextFile to read an object in formatted text file.
//		This function calls the ReadText virtual function;
//		If you want to diagnose the object, Implement the DropData function 
//
//	HOW TO DERIVE A CLASS FROM CDATA CLASS ?
//		---Constructor---
//		The constructor is preferably protected and should be from the same syntax
//		as the CData constructor. The base class constructor has to be initialised as there is
//		no default constructor for CData class.
//
//		---Disk IO---
//		Overwrite Read, ReadText and Write virtual functions to implement your specific derived class disk IO.
//		Do not forget to call base class function.				
//
//		---Macros---
//		The DEFINE_DATA macro has to be placed in each CData derived class declaration
//		Its argument has to be the name of the class.
//		The IMPLEMENT_DATA macro has to be placed in each CData derived class implementation file
//		Its argument has to be the name of the class.
//	
//////////////////////////////////////////////////////////////////////////////////////////////

// Insert this macro in each CData derived class definition. arg is the name of the class
#define DEFINE_DATA(arg)\
	public: virtual LPCTSTR GetClassName() const { return _T(#arg);};\
	public: static CData* NewDataObject(CDataBase* pDataBase, LPCTSTR ptcID) { return (new arg(pDataBase, ptcID));};

// Insert this macro in each CData implementation file. arg is the name of the class
#define IMPLEMENT_DATA(arg) void* pBrol##arg = CData::RegisterDataClass(_T(#arg), &arg::NewDataObject);

// Create a class name
#define CLASS(arg) _T(#arg)

// 2016-10-20:  This macro is placed in the beginning of the the 'DataBase.cpp' file. It will be called by the early 
// dynamic initialization (before launching HySelect). 'IMPLEMENT_DATA' in the 'DataBObj.cpp' and 'DataStruct.cpp' files
// is also executed by the early dynamic initialization. 'IMPLEMENT_DATA' will fill the array prepared by the 
// 'IMPLEMENT_CLASS_REGISTER'. The order of the dynamic initialization is important. If early dynamic initialization begins 
// for example by the 'DataBObj.cpp' file, followed by the call to the 'IMPLEMENT_CLASS_REGISTER' macro, the array will be 
// reseted. Initialization continues with 'DataStruct.cpp' and fills the array from start. When HySelect reads the database, 
// it will stop at the first CDB_XXX object encountered because there are not in the array. This is why it's important to order
// the early dynamic initialization. But how? Hopefully, there is a trick. Files in a project are build in the same order defined
// in the 'TASelect.vcxproj' file of the solution. Files to be compiled are defined with the <ClCompile> tag.
// Just be sure that 'DataBase.cpp' is set before 'DataBObj.cpp' and 'DataStruct.cpp'. 
#define IMPLEMENT_CLASS_REGISTER CData::CDataClassRegistry CData::m_DataClassRegister(_T(__FILE__));

//////////////////////////////////////////////////////////////////////////////////////////////
class CFilterTab; 
class CRedefineTab;
class CSelectedInfos;
class CData
{
	DEFINE_DATA( CData )

	friend class CDataBase;
	
// Construction/Destruction is protected.
// The creation of a Derived CData object is only allowed from the CreateObject function in CDataBase.
protected:
	CData( CDataBase *pDataBase, LPCTSTR ptcID );
	virtual ~CData(); 

// Public methods.
public:
 	// 'Deleted', 'Hidden' and'Available' flags.
	virtual void SetDeleted( bool bFlag ) { m_bDeleted = bFlag; }
	virtual void SetHidden( bool bFlag ) { m_bHidden = bFlag; }
	virtual void SetAvailable( bool bFlag ) { m_bAvailable = bFlag; }
	virtual void SetExistInTASCOPE( bool bFlag ) { m_bExistInTASCOPE = bFlag; }

	// All products that are in the 'tadb-deleted.txt' file are automatically marked as deleted. By default these articles are not shown.
	// If user asks to show old valve (In the technical parameters), then these products are shown but the article
	// number is replaced by *. You can get these products in the product selection (individual, bacth, direct or wizard) 
	// and in HMCalc but without to have the possibility to select them. You can also view these products in
	// old .tsp project.
	virtual bool IsDeleted() { return( m_bDeleted != false ); }

	// By default, if the 'hidden' flag of a product is set to 'false', we don't show this product anywhere.
	// If you press [CTRL] + [SHIFT] + [P] in the right view and type the hidden user password (defined in the 'localdb.txt')
	// the 'hidden' flag is bypassed.
	virtual bool IsHidden() { return( m_bHidden != false ); }

	// By default, if the 'available' flag of a product is set to 'false', we don't show this product anywhere.
	// If you press [CTRL] + [SHIFT] + [P] in the right view and type the super user password (defined in the 'localdb.txt')
	// the 'available' flag is bypassed and thus all products are available.
	virtual bool IsAvailable( bool b = false ) { return ( m_bAvailable != false || m_Identificator.DB->IsSuperUserPsw() ); }
	
	// HYS-1221 : Add bIsForHMCalc parameter to check if deleted products are available 
	// for HM Calc then they can be selectable.
	// HYS-1398 : Add bIsForTools parameter to check is deleted products are available for Hydronic calculator dialog
	virtual bool IsSelectable( bool bForceForaNewPlant = false, bool bForHub = false, bool bIsForHMCalc = false, bool bIsForTools = false );

	bool IsExistInTASCOPE() { return m_bExistInTASCOPE; }

	// Returns 'true' if 'CData' object is a 'CTable'.
	bool IsaTable() { return m_bTable; }
	
	// Returns the first Owner....
	LPCTSTR GetFirstOwnerID() { return m_pOwners->ID; }

	CTable *GetTrueOwner();

	bool HasOwner() { return ( NULL != m_pOwners ) ? true : false; }
	
	// HYS-1299 : This function help to get the real parent between owners
	bool FindOwner( LPCTSTR ptcID );

	// Initialize 'PP' to the table pointer.
	void SetParentTable( CTable *pParent ) { m_Identificator.PP = pParent; }
	
	// Return an extended pointer of this object.
	const IDPTR &GetIDPtr() const { return m_Identificator; }
	
	// Check if this object is of the specified class. Return non zero if true.
	bool IsClass( LPCTSTR ClassName ) const { return ( 0 == _tcscmp( ClassName, GetClassName() ) ) ? true : false; }
	
	// Return a pointer to the database the object belongs to.
	CDataBase *GetDB() { ASSERT( m_Identificator.DB ); return m_Identificator.DB; }


	////////////////////////////////////////////////////////////////////////////////////////////////
	// Locking facilities.
	
	// Lock this object. Give the ID of the object which requests the locking.
	void Lock( const IDPTR& LockerID );
	
	// The specified object (UnlockerID) decide to free this object.
	void Unlock( const IDPTR& UnlockerID );
	
	// Check if the object is locked. Return true if yes.
	int IsLocked() const { return m_pOwners != 0; }

	// The owner database modified state. Call this when you modify the CData object.
	void Modified() { ASSERT( NULL != m_Identificator.DB ); m_Identificator.DB->Modified( 1 ); }

	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Return true if this object is HydroMod or a HydroMod's Child.
	virtual bool IsHMFamily() { return false; }

	// Used to Cross check Database objects.
	virtual void CrossVerifyID() {}

	// Return 0 in case the object doesn't have a key.
	virtual int GetKeyOrder() { return 0; }
	virtual void Copy( CData *pclDestination );
	virtual void CopyFrom( CData *pclSource ) {}
	virtual CSelectedInfos *GetpSelectedInfos() { return NULL; }
	virtual LPCTSTR GetArtNum( bool fWithoutConnection = false ) { return _T(""); }

	// Register the class name and a pointer to a function which can dynamically create an instance of
	// this class (new).
	static void *RegisterDataClass( LPCTSTR ClassName, CData* (*NewFunc)( CDataBase*, LPCTSTR ) )
		 { m_DataClassRegister.Add( ClassName, NewFunc ); return 0; }

#ifndef TACBX
	// PAY ATTENTION!! Must not be used without knowing what we are doing! Changing the ID of an object is
	// completely forbidden. The only use of this method is for renaming old pipe ID (See Jira card HYS-1590).
	void ChangeID( LPTSTR lpNewID );

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData* pclSelectionCompareWith, int iKey );

	bool BreakOnID( CString strID, bool fBreakOnFailure = true );

	// Drop the data to the text file os.
	// Level is the indentation level which is required when printing this data
	virtual void DropData( OUTSTREAM outf, int* piLevel );

	static void GetInheritedModifiedData(INPSTREAM  inpf, unsigned short *pusLineCount, CStringArray *pStrar, int iMaxLines = 30 );
	static void ReadDoubleFromTextLine( INPSTREAM  inpf, double &dValue, unsigned short *pusLineCount );
	static LPTSTR ReadTextLine( INPSTREAM  inpf, unsigned short *pusLineCount, bool fExcludeTab = true );
	static void ReadIntFromTextLine( INPSTREAM  inpf, int &iValue, unsigned short *pusLineCount );

	virtual void DeleteArticle( void );

	// HYS-1941: Allow to have access to the "WriteText" method without changing the scope of this method.
	void WriteTextAccess( OUTSTREAM outf, CString &strTab ) { WriteText( outf, strTab ); }
#endif

// Protected methods.
protected:
	// Not written for now in the database.db
	void SetFlagTable( bool flag = true) { m_bTable = flag; }
	void SetTACBXVer( unsigned short usVer ) { m_usTACBXVer = usVer; }
	unsigned short GetTACBXVer() { return m_usTACBXVer; }

	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Overwrite Write and read to implement the disk IO facilities.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX
	virtual void CheckRedefinition( CRedefineTab *pRedef, CString redefinition ) {}
	virtual void InterpretInheritedData( CString *ppStr ) {}
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount ) {}
	
	enum CheckFilterReturnValue
	{
		CFRV_Error,
		CFRV_FilterApplied,
		CFRV_FilterNotApplied,
	};
	virtual CheckFilterReturnValue CheckFilter( CFilterTab *pFilter, CString strFilter );
	
	int ReadEnumLine( INPSTREAM  inpf, unsigned short *pusLineCount ) { return GetDB()->ReadEnumLine( inpf, pusLineCount ); }
	int ReadEnum( CString str ) { return GetDB()->ReadEnum( str ); }
	void ParseEnumStrLine( INPSTREAM  inpf, CString strLine, int &iKey, CString &strEnum ) { GetDB()->ParseEnumStrLine( inpf, strLine, iKey, strEnum ); }
	void ParseIDList( CArray <IDPTR> *pAr, CString list );
	void ParseIDList( std::vector<IDPTR> &vecIDList, CString list, TCHAR tcSeparator = _T('\\') );
	void ParseDoubleList( CArray <double> *pAr, CString list );
	CString DeleteArticleNumber( CString strArticleNumber );
#endif

// Private methods.
private:
	// Call this function to write the object on the disk.
	// This function call virtual function 'Write'.
	void DumpToDisk( OUTSTREAM outf );

	// Static function which recognize the object class from the disk.
	// It creates it and call virtual function 'Read'.
	// pDataBase is the database where this CData derived object should be recorded.
	static void ExtractFromDisk( INPSTREAM  inpf, CDataBase* pDataBase );

	// Registration of class names and dynamic creation of instance.
	// Create new CData derived object from its class name, the database it will belongs to
	// and its ID. Return a CData pointer to the object. The ID has to be valid.
	static CData *New( LPCTSTR ClassName, CDataBase* pDataBase, LPCTSTR ID )
		{ return m_DataClassRegister.New( ClassName, pDataBase, ID ); }

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
	// The same as ExtractFromDisk but read the information from a text file in special format.
	// After the object is created, the function calls virtual ReadText.
	// pusLineCount is the line cursor in the text file.
	// If the first line to be interpreted has already been read, give it in FirstLine.
	// Ignore is true if the object should be ignored and not read. This is the case if the 
	// 1st line defining the object begins with "*new:" in lieu of "new:".
	static void ExtractFromTextFile( INPSTREAM  inpf, CDataBase *pDataBase, bool bReadingObjectFromDeletedFile, unsigned short *pusLineCount, 
			LPCTSTR pctstrFirstLine = NULL, bool bIgnore = false );
#endif //TACBX////////////////////////////////////////////////////////

// Protected variables.
protected:
	static bool m_bUnicodeFile;		// Used when reading text file, two first character are used to determine if the file use unicode characters
	bool m_bTable;					// used to identify a table, set to true by table creation.
	bool m_bDeleted;				// 1 if the production of this object is terminated.
	bool m_bAvailable;				// 1 if this product is available in this (country) version.
	bool m_bHidden;					// 1 if this product is hidden.
	
	// Not written for now in the database.db
	unsigned short m_usTACBXVer;	// TACBX version is defined by default to 1.
									// Tested into CDataBase::Write in combination with WRITECBXMODE.
									//			== 0 object is skipped.
									//			>0	 object is writed.

	bool m_bExistInTASCOPE;			// 1 if object could be exported to the instrument
									// Initialized by reading.

// Private class.
private:
	// Manage the association between class names and function which create its instance.
	static class CDataClassRegistry
	{
	public:
		CDataClassRegistry( LPCTSTR file );
		~CDataClassRegistry();

		// Add an association between a class name and a pointer to a function returning a CData pointer.
		void Add( LPCTSTR ClassName, CData *( *NewFunc )( CDataBase*, LPCTSTR ) );

		// Dynamically create an object of ClassName. return a pointer to base class CData.
		// The object will belong to pDataBase and its ID will be set.
		CData *New( LPCTSTR ClassName, CDataBase *pDataBase, LPCTSTR ID );

	private:
		struct Assoc_struct
		{
			TCHAR ClassName[_CLASS_NAME_LENGTH + 1];
			CData *( *NewFunc )( CDataBase*, LPCTSTR );
		};
		Assoc_struct *m_prClassTable;				// Class registry table.
		unsigned short m_usClassAmount;			// Number of registration.
	} m_DataClassRegister;	

// Private variables.
private:
	IDPTR m_Identificator;		// Extended pointer for this

public:
	// WARNING
	// struct OWNERS_ID can't be modified without modification of void CData::Read(INPSTREAM  inpf)
	// each structure element size is used in this function.
	struct OWNERS_ID
	{
		TCHAR ID[_ID_LENGTH + 1];
		OWNERS_ID *m_pNext;
	} *m_pOwners;				// List which contains all the object ID which use this object.
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
//	CTable class.
//
//	The CTable class is a collection of CData derived objects extended pointers
//	which is derived itself from CData.
//	So that, a CTable object can be inserted in an other CTable object.
//	This allow a tree structure construction.
//	The CData objects in a CTable object are identified by their ID BUT you cannot insert a CData
//	derived object if this object does not exist yet in the database.
//
//	Insert/remove functions
//		Use Insert and Remove to add or cancel en entry (extended pointer to CData object)
//
//	Access functions
//		Use GetFirst and GetNext for sequential access
//		Use GetChildTable to access to a child table.
//
// new:CTable(ID) -> TABLEID_WHERETOINSERT
// {
//		"Table description"
//		TrueParent				0/1: 1 if this table is the true parent of the IDs in it.
//      [ID1],
//		[ID2],
//      ...
//      [IDn]
// }
//
// HYS-1301: What is the 'TrueParent'. An ID can be inserted in more that one table.
//           But we want to keep a trace of the true parent of the object.
//           For example the "COMPACT_P_10A" object can appears in the "6WAYV_PICV_TAB" and "PICTRLVALV_TAB" table.
//			 We consider here that "PICTRLVALV_TAB" is the true parent.
//			
//////////////////////////////////////////////////////////////////////////////////////////////

#define _TABLE_NAME_LENGTH 63 

class CTable : public CData
{
	DEFINE_DATA( CTable )

// Construction / Destruction.
protected:
	CTable( CDataBase* pDataBase, LPCTSTR ID );
	virtual ~CTable();
		
// Public cleaning methods.
public:
	
	// This function deletes all entries in the table (not the objects they point to !).
	// Moreover, the object belonging to this table are unlocked by it.
	void MakeEmpty();
	
	// If  there is any temporary object stored; move it into the table structure.
	void CheckAndTransferTmpObj();

// Modification of properties.
public:
	// Set/Get the name property. If the table is locked by other object, any attempt
	// to change the name will fail. Both functions return the table name.
	LPCTSTR SetName( LPCTSTR ptstrName );
	void SetTrueParent( bool bTrueParent ) { m_bTrueParent = bTrueParent; }

	LPCTSTR GetName();
	LPCTSTR GetNameIDS() { return m_Name.c_str(); }
	bool GetTrueParent() { return m_bTrueParent; }
	
// Insert / remove element from the table.
public:
	// Insert/Append an ID in the table. The inserted object is locked.
	// You cannot insert twice the same ID in the table, nor two CTables object with the same name.
	// Return value is 'true' if successful, otherwise 'false'.
	virtual void Insert( IDPTR &IDPtr, bool bSetParentTable = true );
	
	// Remove an object from the table and unlock it.
	// Return value is non zero if successful, 0 if the ID was not in the table.
	int Remove( IDPTR &IDPtr );
	
	// Copy only table Name, doesn't copy table items.
	virtual void Copy( CData *pTo );

	CDataList *GetpDataList() { return &m_IDPtrList; }

// Access.
public:
	// Return an extended pointer to the ID item.
	const IDPTR &Get( LPCTSTR ID );
	
	// Return an extended pointer to the first item in the table.
	// If 'ptstrClassName' is not null, the method returns the first object of this type.
	const IDPTR &GetFirst( LPCTSTR ptstrClassName = NULL );
	
	// Return the next item in the table according to specified (if any) class in GetFirst.
	const IDPTR &GetNext();
	
	// Return first Item following pDataObj, this function is initialized with GetFirst but doesn't update
	// member variable m_pAccess, authorize overlap loops.
	const IDPTR &GetNext( CData *pDataObj );

	// Same as 'GetFirst' but here sends back only ID without extending the corresponding IDPTR.
	const _string GetFirstID( LPCTSTR ptstrClassName = NULL );
	
	// Same as 'GetNext' but here sends back only ID without extending the corresponding IDPTR.
	const _string GetNextID();
	
	// Same as 'GetNext' but here sends back only ID without extending the corresponding IDPTR.
	const _string GetNextID( _string strID );
	
	// Return the number of items of the table.
	// If ClassName is not null, return the number of items of this type.
	int GetItemCount( LPCTSTR ptstrClassName = NULL );
	
	// Return MP of object ID included in the table.
	CData *GetMP( LPCTSTR ptstrClassName = NULL, LPCTSTR ID = NULL );
	
	// Return true od false depending existence of ClassName Object in the database.
	// Doesn't use GetFirst GetNext function.
	bool IsExist( LPCTSTR ptstrClassName = NULL );

	// Allow to retrieve an IDPTR from the temporary 'm_arIDptr' table. This is the table where are saved
	// objects when reading before to be validated and transfered to 'm_IDPtrList'.
	IDPTR GetIDInTempTable( LPCTSTR ID );
	void ChangeIDInTempTable( LPCTSTR ptstrOldID, LPCTSTR ptstrNewID );

	// Overriding 'CData'.
	// It's only when reading the database in text mode.
	// In binary mode it's done in the 'CDataBase::ReadObjects' method.
	virtual void CrossVerifyID();

// Protected methods.
protected:
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );
#ifndef TACBX //TACBX////////////////////////////////////////////////////////
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );

public:
	// Drop the data to the text file os.
	// Level is the indentation level which is required when printing this data
	virtual void DropData( OUTSTREAM outf, int *piLevel );
#endif //TACBX////////////////////////////////////////////////////////

// Private variables.
private:
	CDataList m_IDPtrList;
	std::vector <IDPTR> m_arIDptr;
	TCHAR m_CNAccess[_CLASS_NAME_LENGTH];	// Used in GetFirst/GetNext functions.
	bool m_bTrueParent;
	_string m_Name;							// table name;
	_string *m_pStr;
};

class CTableOrdered : public CTable
{
	DEFINE_DATA( CTableOrdered )

protected:
	CTableOrdered( CDataBase *pDataBase, LPCTSTR ID );

public:
	void FillMapListOrdered( std::map<int, CData *> *pMap );
	virtual int GetKeyOrder() { return m_iKeyOrder; }

protected:
	void SetKeyOrder( int iKey ) { m_iKeyOrder = iKey; }
	// Disk IO.
	virtual bool Read( INPSTREAM  inpf );
	virtual void Write( OUTSTREAM outf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif //TACBX

// Protected variables.
protected:
	int m_iKeyOrder;
};

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
class CRank;
class CRankEx;

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TACBX //TACBX////////////////////////////////////////////////////////
class CUserDatabase : public CDataBase
{
public:
	CUserDatabase();
};
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
struct mapcomp {
  bool operator() (const WCHAR* p1, WCHAR* p2) const
	{
		if (wcscmp(p1, p2)<0)
			return true;
		return false;
	}
};

class CXmlStrTab
{
private:
	std::map<WCHAR *, WCHAR *,mapcomp> m_mIDS;
	WCHAR *m_MapBuf;
	_string m_retStr;
public:
	CXmlStrTab();
	~CXmlStrTab();
	
	void Clear();
	
	// Init
	//	Input :	strFileName: XML filename 
	//			strLanguage: language
	//	Output : Number of added string,
	//			-1 if fails
	void Init( std::string strFileName, _string strLanguage );
	_string* GetIDSStr( _string IDS );

	// Find IDS and replace by his translation.
	bool FindAndReplace( _string &IDS );

protected:
	bool ProcessNewLineChar( WCHAR *pos, WCHAR *posBuf );
};

extern CXmlStrTab XmlStrTab;
extern CXmlStrTab *GetpXmlStrTab();

#ifndef TACBX
class CXmlReader;
class CXmlRcTab
{
public:
	CXmlRcTab();
	virtual ~CXmlRcTab();
	
	static void GetLangList( std::string strFileName, std::vector<std::wstring> &vLangList );
	void Clear();

	// Init
	//	Input :	strFileName: XML filename 
	//			strLanguage: language
	//	Output : Number of added string,
	//			-1 if fails
	void Init( std::string strFileName, _string strLanguage );

	WCHAR *GetIDSStr( unsigned int uiIDS );
	
	// Methods add to facilitate management with tips (see 'DglTip.cpp').
	int GetIDCount( void ) { return m_mIDS.size(); }
	WCHAR *GetIDSStrByPos( unsigned int uiPos );
	int GetIDSByPos( unsigned int uiPos );
	WCHAR *GetFirstIDSStr();
	WCHAR *GetNextIDSStr();

protected:
	bool ProcessNewLineChar( WCHAR *ptcPos, WCHAR *ptcPosBuffer );

private:
	std::map<unsigned int, WCHAR *> m_mIDS;
	std::map<unsigned int, WCHAR *>::iterator m_It;
	WCHAR *m_MapBuf;
};

extern CXmlRcTab XmlRcTab;
extern CXmlRcTab *GetpXmlRcTab();
extern CXmlRcTab XmlTipTab;
extern CXmlRcTab *GetpXmlTipTab();
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
class CFieldTab
{
public:
	CFieldTab();
	~CFieldTab();

	virtual void ReadFieldTab( CFileTxt &inpf );
	
	// Return the string field [index] or Null if not found (index start from 0)
	// pos is the starting position in the string to analyze.
	LPCTSTR GetField( int index, LPCTSTR pstr, int pos = 0 );
	LPCTSTR GetFirstField( LPCTSTR pstr );
	LPCTSTR GetNextField( LPCTSTR pstr );
	CString GetFieldLineString( POSITION Pos );
	int GetNumberOfFields( LPCTSTR pstr );
	bool IsEmpty() { return ( TRUE == m_List.IsEmpty() ) ? true : false; }

protected:
	CStringList m_List;
	int m_iCurrentField;
	int m_iCurrentPos;
	TCHAR m_tcBuffer[256];
};

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
class CFilterTab : public CFieldTab
{
public:
	CFilterTab() {}
	~CFilterTab() {}

	// 'FS' for 'Filter State'.
	enum
	{
		FS_MustBeDeleted = -1,
		FS_NotFiltered = 0,
		FS_Filtered = 1
	};

	// Pos is updated with the position of the next filter string; or null at the end of the list 
	int IsFiltered( POSITION &Pos, LPCTSTR pClassName, LPCTSTR pTabId, LPCTSTR pID );
};
extern CFilterTab FilterTab;

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
class CRedefineTab : public CFieldTab
{
public:
	CRedefineTab() {};
	~CRedefineTab() {};
	// Return 0 if the object is not redefined
	// Return 1 if yes
	// Pos is updated with the position of the next filter string; or null at the end of the list 
	int IsRedefined(POSITION &Pos,LPCTSTR pID);
};
extern CRedefineTab RedefineTab;

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

class CDB_MapStrStr;
class CLocArtNumTab
{
public:
	CLocArtNumTab() { m_pMapLocalArticleNumber = NULL; }
	~CLocArtNumTab() {}

	virtual void ReadFile( CFileTxt &inpf );
	CString GetLocalArticleNumber( CString strFirst );

// Private methods.
private:
	void _CleanArticleNumber( CString &strArticleNumber );

// Private variables.
private:
	CDB_MapStrStr *m_pMapLocalArticleNumber;
};
extern CLocArtNumTab LocArtNumTab;
#endif //TACBX////////////////////////////////////////////////////////

#endif // !defined(DATABASE_H__INCLUDED_)
