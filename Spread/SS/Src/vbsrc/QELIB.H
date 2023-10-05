
		/*-----------------------------+
		|           QELIB.H            |
		+------------------------------+
		|  Q+E Library Include File    |
		+-----------------------------*/

/*--------------------------------------------------------------+
|  Copyright: 1988-1993 Q+E Software, Inc.			|
|								|
|	This software contains confidential and	proprietary	|
|	information of Q+E Software, Inc.			|
+--------------------------------------------------------------*/

#ifndef QELIBH
#define QELIBH

// ***************** IMPORTANT NOTES FOR BORLAND C USERS ********************
// Due to differences in the way Microsoft C returns floating point values, 
// all routines that return a float or double must be passed an extra       
// argument.  This argument must point to a floating point value ON THE     
// STACK.  Furthermore, the routine returns a pointer to the floating point 
// value.  For example, where a Microsoft user would type:		    
//	fltval = qeValFloat (hstmt, colno);				    
// a Borland C user would type (assuming fltval is a local variable):	    
//	fltval = *qeValFloat (hstmt, colno, &fltval);			    
// or simply:								    
//	qeValFloat (hstmt, colno, &fltval);				    
// If fltval is not a local variable, then the following will work:         
//	{float temp; fltval = *qeValFloat (hstmt, colno, &temp);}	    
// **************************************************************************

#ifdef __BORLANDC__
#define qeAPI(rtn_type)	rtn_type far pascal
#define qeLPSTR		char far *
#define qeLPDEC		qeLPSTR
#define qeLPVOID	void far *
#define qeLPLONG	long far *
#define qeHANDLE	short
#define qeSTATUS	short
#define qeRETFLT	float far *
#define qeRETDBL	double far *
#define qeHIDDENFLT	, float _ss *
#define qeHIDDENDBL	, double _ss *
#define qeLPFLT		float far *
#define qeLPDBL		double far *
#define qeLPINT		short far *
#endif

// **************** IMPORTANT NOTES FOR IBM OS/2 2.0 USERS ******************
// QELIB is a 16-bit DLL.  32-bit applications must take special care when  
// calling a 16-bit DLL.  First, all QELIB routines must be declared FAR16. 
// Second, any buffers passed to QELIB must not cross a 64K boundary.  This 
// can be assured by allocating tiled memory.				    
// **************************************************************************

#ifdef __IBMC__
#define qeAPI(rtn_type)	rtn_type _Far16 _Pascal
#define qeLPSTR		char *
#define qeLPDEC		qeLPSTR
#define qeLPVOID	void *
#define qeLPLONG	long *
#define qeHANDLE	short
#define qeSTATUS	short
#define qeRETFLT	float
#define qeRETDBL	double
#define qeHIDDENFLT
#define qeHIDDENDBL
#define qeLPFLT		float *
#define qeLPDBL		double *
#define qeLPINT		short *
#endif

	 
#ifdef QE_MAC
#define qeAPI(rtn_type)	pascal rtn_type
#define qeLPSTR		char *
#define qeLPDEC		qeLPSTR
#define qeLPVOID	void *
#define qeLPLONG	long *
#define qeHANDLE	short
#define qeSTATUS	short
#define qeRETFLT	float
#define qeRETDBL	double
#define qeHIDDENFLT
#define qeHIDDENDBL
#define qeLPFLT		float *
#define qeLPDBL		double *
#define qeLPINT		short *
#endif


// BUG - don't know what these should really be for WIN32
// Or even what define to use - John

#ifdef QE_WIN32
// This __stdcall is a C8 thing!
#define qeAPI(rtn_type)	rtn_type __stdcall
#define qeLPSTR		char *
#define qeLPDEC		qeLPSTR
#define qeLPVOID	void *
#define qeLPLONG	long *
#define qeHANDLE	short
#define qeSTATUS	short
#define qeRETFLT	float
#define qeRETDBL	double
#define qeHIDDENFLT
#define qeHIDDENDBL
#define qeLPFLT		float *
#define qeLPDBL		double *
#define qeLPINT		short *
#endif


// These are the declarations when using the Microsoft C compiler 

#ifndef qeAPI
#define qeAPI(rtn_type)	rtn_type __loadds __export far pascal
#define qeLPSTR		char far *
#define qeLPDEC		qeLPSTR
#define qeLPVOID	void far *
#define qeLPLONG	long far *
#define qeHANDLE	short
#define qeSTATUS	short
#define qeRETFLT	float
#define qeRETDBL	double
#define qeHIDDENFLT
#define qeHIDDENDBL
#define qeLPFLT		float far *
#define qeLPDBL		double far *
#define qeLPINT		short far *
#endif

//--------------------------------------------------------------------------
// Result Code Values.  All routines which return qeSTATUS will return a 
// value from this set. This also defines the set of values which will be 
// returned from qeErr 

#define qeLOCK_CHANGE_REC	-8	// Attempt to lock a record and 
					// the lock was obtained, but the 
					// record has been changed since it 
					// was originally read.  This can 
					// only occur for dbms that require 
					// a log file. 
#define qeLOCK_MULTI_REC	-7	// Attempt to lock a record and 
					// the lock was obtained, but more 
					// than one record was locked. This 
					// occured because the primary key 
					// fields caused more than one 
					// record to be selected. 
#define qeLOCK_NO_REC		-6	// Attempt to lock a record, but 
					// no record in the database was 
					// selected by the primary key, 
					// the record has been deleted by 
					// another user, or another user 
					// has changed the value of a 
					// key field. 
#define qeEOF			-5	// No more records to read. 
#define qeUSER_CANCELLED	-4	// User cancelled out of the logon 
					// dialog box. 
#define qeOUT_OF_MEMORY		-3	// Insufficient memory for operation
#define qeNULL_DATA		-2	// NULL value.  Returned as the 
					// length from a qeDataLen call 
#define qeTRUNCATION		-1	// The buffer passed in to hold 
					// a result is not large 
					// enough to hold the result.  
					// A partial result has been 
					// returned in the buffer. 
#define qeSUCCESS		0	// Success 
#define qeSUCCESS_WITH_INFO	1	// Success with information(warning)
#define qeNO_DATA_WITH_INFO	2	// EOF with additional information 
					// (usually ESC during a fetch) 
#define qeDBSYS_ERROR		4	// Error detected by the underlying 
					// database system or driver. The 
					// native error returned by qeDBErr 
					// gives more specifics. 
//---------------------------------------------------------------------------

// QELIB Data Types.  These are the values returned from qeColType 

#define qeCHAR			1	// Blank padded fixed length string 
#define qeVARCHAR		2	// Variable length string 
#define qeDECIMAL		3	// BCD number 
#define qeINTEGER		4	// 4-byte signed integer 
#define qeSMALLINT		5	// 2-byte signed integer 
#define qeFLOAT			6	// 4-byte floating point 
#define qeDOUBLEPRECISION	7	// 8-byte floating point 
#define qeDATETIME		8	// 26-byte date time value 
					// YYYY-MM-DD HH:MM:SS.FFFFFF 
//---------------------------------------------------------------------------

// Native data types. These types, plus the 8 QELIB types, are the values 
// returned by qeColDBType. 

#define qeLONGVARCHAR		100	// Large text fields 
#define qeBINARY		101	// Fixed length binary or raw data 
#define qeVARBINARY		102	// Variable length binary data 
#define qeLONGVARBINARY		103	// Large binary data fields 
#define qeNATIONALCHAR		104	// Fixed length char strings may 
					// have 2-byte chars (e.g. Kanji) 
#define qeVARNATIONALCHAR	105	// Variable length national strings 
#define qeLONGVARNATIONALCHAR	106	// Large national strings 
#define qeNUMERIC		107	// Numeric, no fixed scale 
#define qeBIGINT		108	// 8-byte signed integer 
#define qeTINYINT		109	// 1-byte signed integer 
#define qeBIT			110	// Bit value (0 or 1) 
#define qeDATE			111	// Date 
#define qeTIME			112	// Time 
#define qeRECID			113	// Unique row ID 
#define qeRECVER		114	// Row version number 
#define qeSERIAL		115	// Automatically generated 
					// sequential number 
#define qeMONEY			116	// Money 
//---------------------------------------------------------------------------

// Routines

//--------------------------
// Data dictionary routines 
//--------------------------
//--------------------------------------------------------------------------

qeAPI(qeHANDLE) qeSources (
	short		options);
			  // Options determine which Sources are to be 
			  // returned, and must be one of the values from 
			  // the OPTIONS constants defined below. 

// Creates a handle to a statement execution (HSTMT) that can be used to 
// return information on the database Sources (database systems) that can 
// be accessed.  The records can be subsequently read using the qeFetchXXX 
// functions, and column values can be retrieved using qeVal functions, 
// or via the qeBindCol functions. 
// RETURNS: the statement execution (HSTMT), or 0 if the function fails. 
// When you have finished retrieving Sources (ie you receive EOF from 
// qeFetchNext), you must call qeEndSQL to terminate the hstmt. 

// The resulting records contain the following columns:			   
//	Name		Char(32) - Source name				   
//	Extension	Char(32) - File Extension (may be null)		   
//	QELIB HDBC	Short    - If qeConnect has been used to	   
//				   connect to this Source, the QELIB HDBC. 
//				   This is 0 if not currently connected.   
//	Remarks		Char(256)- Comment field for data sources	   
//---------------------------------------------------------------------------

// OPTIONS: Values that may be used for the options parameter to qeSources. 
// The value determines which Sources will be returned by the hstmt. 

#define qeSRC_AVAIL_LOGON	1	// All Sources 
#define qeSRC_CONN_LOGON	2	// All Connected Sources 

//---------------------------------------------------------------------------
// Number of columns in result set and defines for returned columns 
#define qeSRC_NUM_COLS		4	// Number of columns returned 
#define qeSRC_NAME_COL		1	// Source name 
#define	qeSRC_EXT_COL		2	// file extension 
#define qeSRC_HDBC_COL		3	// QELIB HDBC 
#define qeSRC_REMARKS_COL	4	// Remarks 

//---------------------------------------------------------------------------
// Maximum lengths of returned character column values (not including 
// 0-terminator) 

#define qeSRC_MAX_LEN		32	// Maximum length of a source name 
#define qeSRC_REMARK_MAX_LEN	256	// Maximum length of a data source 
					// remark field. 
//--------------------------------------------------------------------------

qeAPI(qeHANDLE) qeDatabases (
	qeHANDLE	hdbc);	// Connection Handle 

// Creates a statement execution (hstmt) that will return information on 
// the databases that are available to the connection represented by hdbc. 
// The records can be subsequently read using the qeFetchXXX functions, and 
// column values can be retrieved using qeVal functions, or via the 
// qeBindCol functions.
// RETURNS: the statement execution (HSTMT), or 0 if the function fails. 
// When you have finished retrieving Databases (ie you receive EOF from 
// qeFetchNext), you must call qeEndSQL to terminate the hstmt. 

// The resulting records contain the following columns: 
//	Database        Char(128) - Database Name 
//	Remarks         Char(256) - Comment field for databases 

//---------------------------------------------------------------------------
// Number of columns and column numbers 

#define qeDB_NUM_COLS		2	// Number of columns returned 
#define qeDB_DATABASE_COL	1	// Database name 
#define qeDB_REMARKS_COL	2	// Remarks 

//---------------------------------------------------------------------------
// Maximum lengths of returned character column values (not including 
// 0-terminator) 

#define qeDB_MAX_LEN		128	// Maximum length of a database name 
#define qeDB_REMARK_MAX_LEN	256	// Maximum length of a database 
					// remark field 
//--------------------------------------------------------------------------

qeAPI(qeHANDLE) qeTables (
	qeHANDLE	hdbc,		// Connection handle		    
	qeLPSTR		qualifer_pattern,// Database (or directory) pattern  
	qeLPSTR		user_pattern,	// Pattern for selecting Users. If  
					// the pattern is "", the Current   
					// User is assumed.  If the pattern 
					// is "*", all users are selected.  
					// Ignored for File-based databases 
					// - the Current Working Directory  
					// is assumed.			    
	qeLPSTR		table_pattern,	// Pattern for selecting Tables (or 
					// Files).  If the pattern is "",   
					// all tables are selected.	    
	short		flags);		// Options that determine which     
					// types of items are to be	    
					// returned.  See OPTIONS defined   
					// below 

// Creates a statement execution (hstmt) that will return the set of 
// database Tables (or Files) that can be accessed by the hdbc.
// RETURNS: the statement execution (HSTMT), or 0 if the function fails. 
// The records can be subsequently read using the qeFetchXXX functions, and 
// column values can be retrieved using qeVal functions, or via the 
// qeBindCol functions.
// When you have finished retrieving Tables (ie you receive EOF	from 
// qeFetchNext), you must call qeEndSQL to terminate the hstmt. 

// The resulting records contain the following columns: 
//	Table Qualifier	Char(128)- Table Qualifier 
//	Table User	Char(128)- Table Owner 
//	Table Name	Char(128)- Table Name 
//	Type		Short    - The type of item returned. For 
//				   Table-based Sources, this can be used to 
//				   distinguish Tables, Views, Stored 
//				   Procedures, and Synonym names, all of 
//				   which may be returned by this query. For 
//				   File-based Sources, the type is always 
//				   File. 
//	Remarks		Char(256)- Remarks field for table 

//--------------------------------------------------------------------------
// OPTIONS: Values that may be used for the options parameter to qeTables. 
// The value sent determines which types of items are to be returned by the 
// hstmt. These values can be combined by adding them together.  For example 
// qeTBL_TABLE+qeTBL_VIEW causes both Tables and Views to be returned. 
// These are also the values that will be returned in the Type column. 
// NOTE: qeTBL_DATABASE cannot be combined with the other values. 

#define qeTBL_TABLE	0x0001		// Get table names 
#define qeTBL_VIEW	0x0002		// Get view names 
#define qeTBL_PROCEDURE	0x0004		// Get stored procedure names 
#define qeTBL_SYSTABLE	0x0008		// Get system table names 
#define qeTBL_SYNONYM	0x0010		// Get synonym names 
#define qeTBL_DATABASE	0x0080		// Get database names 
//--------------------------------------------------------------------------
// Number of columns and column numbers 
#define qeTBL_NUM_COLS		5	// Number of columns returned 
#define qeTBL_TABLE_QUAL_COL	1	// Table qualifier 
#define qeTBL_TABLE_OWNER_COL	2	// Table owner 
#define qeTBL_TABLE_NAME_COL	3	// Table name 
#define qeTBL_TYPE_COL		4	// Type of item returned 
#define qeTBL_REMARK_COL	5	// Remarks 
//--------------------------------------------------------------------------
// Maximum lengths of returned char column values (not including 
// 0-terminator) 

#define qeUSER_MAX_LEN		128	// Maximum length of a 
					// user/directory name 
#define qeTBL_MAX_LEN		128	// Maximum length of a 
					// table/file/qual name 
#define qeMAX_REMARK_LEN	256	// Maximum length of a remark fields 
//---------------------------------------------------------------------------

qeAPI(qeHANDLE) qeColumns (
	qeHANDLE	hdbc,		// Connection handle 
	qeLPSTR		table_name);	// Table whose columns are to be 
					// returned 

// Creates a statement execution (hstmt) that will return the set of 
// database Column definitions for a Table (or File). 
// RETURNS: the statement execution (HSTMT), or 0 if the function fails. 
// The records can be subsequently read using the qeFetchXXX functions, and 
// column values can be retrieved using qeVal functions, or via the 
// qeBindCol functions. 
// When you have finished retrieving Columns (ie you receive EOF from 
// qeFetchNext), you must call qeEndSQL to terminate the hstmt. 

// The resulting records contain the following columns: 
//	Table Qualifier Char(128)- Table qualifier 
//	Table Owner Char(128)- Table Owner 
//	Table Name  Char(128)- Table Name 
//	Column	    Char(128)- Column name 
//	Type	    Short    - Data type 
//	Width	    Short    - Width in bytes 
//	DBType	    Short    - Database data type 
//	DBType Name Char(128)- Database name string 
//	Attr1	    Short    - Precision for Decimal, Date start for dates. 
//	Attr2	    Short    - Scale for Decimal, Date end for dates 
//	Nullable    Short    - Is Column Nullable 
//	Remarks	    Char(256)- Remarks field for column 

//---------------------------------------------------------------------------
// Number of columns and column numbers 

#define qeCOL_NUM_COLS		12	// Number of columns in record 

#define qeCOL_TABLE_QUAL_COL	1	// Table qualifier 
#define qeCOL_TABLE_OWN_COL	2	// Table owner 
#define qeCOL_TABLE_NAME_COL	3	// Table name 
#define qeCOL_NAME_COL		4	// Column name 
#define qeCOL_TYPE_COL		5	// Column type 
#define qeCOL_WIDTH_COL		6	// Column width 
#define qeCOL_DBTYPE_COL	7	// Column database type 
#define qeCOL_DBTYPE_NAME_COL	8	// Column database type name 
#define qeCOL_ATTR1_COL		9	// Column precision/date start 
#define qeCOL_ATTR2_COL		10	// Column scale/date end 
#define qeCOL_NULLABLE_COL	11	// Column nullable 
#define qeCOL_REMARKS_COL	12	// Column remarks 
//---------------------------------------------------------------------------
// Maximum length of returned char column value (not including 0-terminator) 
 
#define qeCOL_MAX_LEN		128	// Maximum length of a column name 
//--------------------------------------------------------------------------

qeAPI(qeHANDLE) qeTypeInfo (
	qeHANDLE	hdbc);	// Connection handle 

// Creates a statement execution (hstmt) that will return the set of data 
// types and their attributes for a Database System. 
// RETURNS: the statement execution (HSTMT), or 0 if the function fails. 
// The records can be subsequently read using the qeFetchXXX functions, and 
// column values can be retrieved using qeVal functions, or via the 
// qeBindCol functions.
// When you have finished retrieving TypeInfo (ie you receive EOF from 
// qeFetchNext), you must call qeEndSQL to terminate the hstmt. 

// The resulting records contain the following columns: 
//	Type_Name	Char(128) - Data source dependent data type name 
//	Type		Short     - Data type 
//	DB_Type		Short     - Database data type 
//	Width		Long	  - Type size 
//	Attr1		Short     - Precision for Decimal, 
//				    Date start for dates.
//	Attr2		Short     - Scale for Decimal, Date end for dates 
//	Litteral_Prefix Char(128) - Characters used to prefix a	literal, 
//				    NULL if not applicable. 
//	Litteral_Suffix Char(128) - Characters used to terminate a literal. 
//				    NULL if not applicable. 
//	Create_Params   Char(128) - The parameters necessary to use the type 
//				    in a create table statement 
//				    (Decimal would be "Precision,Scale"). 
//	Nullable	Short	    Whether the type can be NULL. 
//				    Values: qeCOL_NULLABLE, 
//				    qeCOL_NOT_NULLABLE, and qeCOL_UNKNOWN 
//	Case_Sensitive  Short     - T/F, Whether the type can be treated as 
//				    case sensitive for sorting. 
//	Searchable	Short     - How the type can be used in a WHERE 
//				    clause. Values: qeCOL_UNSEARCHABLE, 
//				    qeCOL_LIKE_ONLY, qeCOL_ALL_EXCEPT_LIKE, 
//				    qeCOL_SEARCHABLE. 
//	Unsigned	Short     - T/F, whether type is Unsigned, NULL if 
//				    not applicable. 
//	Money		Short     - T/F, whether type is a money data type. 
//	Auto_Increment  Short     - T/F, whether data type is 
//				    auto-incrementing.NULL if not applicable.
//	Local_Type_Name Char(128) - Localized version of the data 
//				    source-dependent name of the data type. 
//				    NULL if not applicable. 
  
//---------------------------------------------------------------------------
// Number of columns and column numbers 

#define qeTYPE_NUM_COLS		16	// Number of columns in record 

#define qeTYPE_TYPE_NAME_COL	1	// Database type name 
#define qeTYPE_TYPE_COL		2	// QELIB type 
#define qeTYPE_DBTYPE_COL	3	// Database type 
#define qeTYPE_WIDTH_COL	4	// Bytes to represent type 
#define qeTYPE_ATTR1_COL	5	// Precision/ date start 
#define qeTYPE_ATTR2_COL	6	// Scale/ date end 
#define qeTYPE_LIT_PREFIX_COL	7	// Literal prefix 
#define qeTYPE_LIT_SUFFIX_COL	8	// Literal suffix 
#define qeTYPE_CREATE_PARAM_COL	9	// Parameters in create table stmt 
#define qeTYPE_NULLABLE_COL	10	// If type is nullable 
#define qeTYPE_CASE_SENS_COL	11	// If type is case-sensitive for sort
#define qeTYPE_SEARCHABLE_COL	12	// If type can appear in WHERE clause
#define qeTYPE_UNSIGNED_COL	13	// If type is unsigned 
#define qeTYPE_MONEY_COL	14	// If type is a money type 
#define qeTYPE_AUTO_INCRE_COL	15	// If type is auto-incrementing 
#define qeTYPE_LOCAL_NAME_COL	16	// Local type name 
//---------------------------------------------------------------------------
// Maximum length of returned char column value (not including 0-terminator) 
 
#define qeTYPE_MAX_LEN		128	// Maximum length of a column name 
//---------------------------------------------------------------------------

qeAPI(qeHANDLE) qeProcedureColumns (
	qeHANDLE	hdbc,		// Connection Handle 
	qeLPSTR		proc_name);	// Procedure name to return info on 

// Creates a statement execution (hstmt) that will return the set of 
// database Column definitions for a Procedure. 
// RETURNS: the statement execution (HSTMT), or 0 if the function fails. 
// The records can be subsequently read using the qeFetchXXX functions, and 
// column values can be retrieved using qeVal functions, or via the 
// qeBindCol functions.
// When you have finished retrieving Procedure Columns (ie you receive 
// EOF from qeFetchNext), you must call qeEndSQL to terminate the hstmt. 

// The resulting records contain the following columns: 
//	Proc Qualifier  Char(128)- Proc qualifier 
//	Proc Owner  Char(128)- Proc Owner 
//	Proc Name   Char(128)- Proc Name 
//	Column	    Char(128)- Column name 
//	Col Type    Short    - If column is IN, OUT, or IN/OUT. From defines 
//			       listed below 
//	Data Type   Short    - Data type 
//	DBType Name Short    - Database type name 
//	Width	    Short    - Width in bytes 
//	Attr1	    Short    - Precision for Decimal, Datestart for dates. 
//	Attr2	    Short    - Scale for Decimal, Date end for dates 
//	Nullable    Short    - Is Column Nullable 
//	Remarks	    Char(256)- Remarks field for column 

//---------------------------------------------------------------------------
// Number of columns and column numbers 

#define qePROC_NUM_COLS		12	// Number of columns in result 

#define qePROC_QUAL_COL		1	// Procedure qualifier 
#define qePROC_OWN_COL		2	// Procedure owner 
#define qePROC_NAME_COL		3	// Procedure name 
#define qePROC_COL_NAME_COL	4	// Column name 
#define qePROC_TYPE_COL		5	// See Col Type defines below 
#define qePROC_DATA_TYPE_COL	6	// Data type 
#define qePROC_DBTYPE_NAME_COL	7	// Database type name 
#define qePROC_WIDTH_COL	8	// Width in bytes 
#define qePROC_ATTR1_COL	9	// Precision / date start 
#define qePROC_ATTR2_COL	10	// Scale / date end 
#define qePROC_NULLABLE_COL	11	// If column can be NULL 
#define qePROC_REMARKS_COL	12	// Remarks for column 
//---------------------------------------------------------------------------
// Maximum length of returned char column values (not including 0-terminator)
 
#define qePROC_MAX_LEN		128	// Maximum length of a column name 
//---------------------------------------------------------------------------
// Values for the Col Type column 
#define qePARAM_UNKNOWN		1	// Don't know 
#define qePARAM_INPUT		2	// Column only an input parameter 
#define	qePARAM_INOUT		3	// Column is an input/output param 
#define	qeRESULT_COL		4	// Column only an output parameter 
//--------------------------------------------------------------------------

//--------------------------
// Error Handling functions 
//--------------------------

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeErr (void);
// RETURNS: the result code from the most recent call to QELIB.  The values 
// that this function can return are defined at the top of this file, under 
// the heading of Result Code Values 
//--------------------------------------------------------------------------
qeAPI(long) qeDBErr (void);
// RETURNS: the database system error code from the most recent call to 
// a QELIB routine. These values are database dependent, and do not have 
// defines in QELIB.H 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeWarning (void);
// RETURNS: the warning code from the most recent call to QELIB.  This value 
// is database dependent, and there are no QELIB.H defines for it. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeErrMsg (void);
qeAPI(qeSTATUS) qeErrMsgBuf (
	qeLPSTR		msg_buf);	// Buffer for error message 
// RETURNS: a pointer to the error message(s) from the most recent call to 
// QELIB, either as the return value (qeErrMsg) or in a provided buffer 
// (qeErrMsgBuf).  Multiple errors will be concatenated up to 
// qeMAX_ERR_MSG_LEN bytes.  msg_buf must be at least this large, or an 
// error may result. 

#define qeMAX_ERR_MSG_LEN 512	// Maximum length of an error message (not 
				// including 0-terminator).
//--------------------------------------------------------------------------

//---------------------
// Debugging functions 
//---------------------

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeTraceOn (
	qeLPSTR		file_pathname);	// Pathname of trace file to write to
// Turns debug tracing on. By default, tracing of all non-qeVal calls is 
// enabled, as are calls to qeTraceUser. RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeTraceOff (void);
// Turns debug tracing off. RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetTraceOptions (
	short		flags);		// Flags to set, from flags below 
// Sets the type of information that is sent to the trace file.  Each call 
// resets the options to the value passed in flags. The default when
// qeTraceOn is called will be qeTRACE_NON_VAL_CALLS + qeTRACE_USER. 
// RETURNS: a result code 
//--------------------------------------------------------------------------
qeAPI(short) qeGetTraceOptions (void);
// Gets the type of information that is sent to the trace file.
// RETURNS: current trace options 
//---------------------------------------------------------------------------
// Options for qeSet/GetTraceOptions 
#define qeTRACE_NON_VAL_CALLS	0x0001 // Trace all calls except qeValXXX 
#define qeTRACE_USER		0x0002 // Write calls from qeTraceUser 
#define qeTRACE_VAL_CALLS	0x0004 // Trace qeValXXX calls 
#define qeTRACE_WINDOW		0x0008 // Write trace info to a window 
#define qeTRACE_ODBC		0x0010 // Tell ODBC driver to turn on tracing
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeTraceUser (
	qeLPSTR		trace_string);	// String to write to trace file 
// Writes the trace_string to the tracefile, if tracing is turned on and 
// user strings are enabled (qeTRACE_USER bit is set).  Otherwise, it does 
// nothing. RETURNS: a result code.
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetDriverTracefile (
	qeHANDLE	hdbc,		// Connection Handle 
	qeLPSTR		file_name);	// Pathname of file for ODBC tracing 
// Sets a file as the target of the ODBC trace file.  The ODBC trace file 
// logs all calls to the ODBC driver.  If ODBC tracing is enabled, and no 
// file is specified, it will be written to sql.log by default. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------

//-------------------------------
// Task functions		 
//-------------------------------

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeLibInit (void);
// Performs task initialization.  Each task that calls QELIB should call 
// this first, if possible. RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeLibTerm (void);
// Performs task Termination.  Each task that calls QELIB should call 
// this when it is finished using QELIB. RETURNS: a result code. 
//--------------------------------------------------------------------------

//-------------------------------
// Database Connection functions 
//-------------------------------

//--------------------------------------------------------------------------
qeAPI(qeHANDLE) qeConnect (
	qeLPSTR		con_string);	// Connection string 
// Opens a connection to a database system and creates a handle to the 
// connection (HDBC). RETURNS: the connection handle, or 0 if connect fails. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeDisconnect (
	qeHANDLE	hdbc);		// Connection Handle 
// Closes a connection to a database system. RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetLoginTimeout (
	long		seconds);	// Time to wait before timeout, in 
					// seconds 
// Sets the login timeout for connecting to a database.  This may have no 
// effect on an individual connection, depending on whether the database 
// system has implemented this feature.  The default is 15 seconds.  Setting 
// the value to 0 indicates that logins are to wait indefinitely. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeGetLoginTimeout (void);
// Gets the login timeout for connecting to a database. The default is 15 
// seconds.  A value of 0 indicates that logins are to wait indefinitely. 
// RETURNS: the current login timeout 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetDB (
	qeHANDLE	hdbc,		// Connection Handle 
	qeLPSTR		database);	// Database to become current 
// Change the current database.  This becomes the default database when 
// SQL statements are executed.  This is only supported by a few database 
// systems. RETURNS: a result code.
//--------------------------------------------------------------------------

//-------------------------
// SQL execution functions 
//-------------------------

//--------------------------------------------------------------------------
qeAPI(qeHANDLE) qeSQLPrepare (
	qeHANDLE	hdbc,		// Connection Handle 
	qeLPSTR		stmt);		// SQL statement to prepare 
// Prepare an SQL statement for execution.  Following prepare, qeSetParam 
// can be called to set parameter values. If sql_stmt is null, the statement
// is gotten from the HDBC's SQL statement buffer, which is built up from 
// calls to qeSetSQL and qeAppendSQL. 
// To execute the statement, call qeSQLExecute. 
// RETURNS: a handle to statement execution (HSTMT), or 0 if the function 
// fails. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSQLExecute (
	qeHANDLE	hstmt);	// Statement Handle 
// Execute an SQL statement that was previously prepared. 
// RETURNS: a handle to statement execution (HSTMT), or 0 if the function 
// fails. 
//--------------------------------------------------------------------------
qeAPI(qeHANDLE) qeExecSQL (
	qeHANDLE	hdbc,		// Connection Handle 
	qeLPSTR		sql_stmt);	// SQL statement to execute. 
// Execute an SQL statement. If sql_stmt is null, the statement is gotten 
// from the HDBC's SQL statement buffer, which is built up from calls to 
// qeSetSQL and qeAppendSQL. 
// RETURNS: a handle to statement execution (HSTMT), or 0 if the function 
// fails. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetSQL (
	qeHANDLE	hdbc,		// Connection Handle 
	qeLPSTR		partial_stmt);	// Initial part of SQL statement 
// Assigns sql_stmt to HDBC's SQL statement buffer.  Subsequent calls 
// to qeAppend SQL will concatenate to the SQL statement to build the 
// complete statement in pieces. 
// RETURNS: a result code. 
// Used to send SQL statements to QELIB in pieces.  Used primarily 
// by Excel and other macro languages that have limits on the size 
// of a character string.  Not needed by C programmers. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeAppendSQL (
	qeHANDLE	hdbc,		// Handle to database connection 
	qeLPSTR		partial_stmt);	// First part of SQL statement 
// Appends partial_stmt to HDBC's SQL statement buffer. 
// RETURNS: a result code. 
// Used to send SQL statements to QELIB in pieces.  Used primarily 
// by Excel and other macro languages that have limits on the size 
// of a character string.  Not needed by C programmers. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeEndSQL (
	qeHANDLE	hstmt);		// Statement Handle 
// Ends a SQL statement's execution. RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeNumModRecs (
	qeHANDLE	hstmt);		// Statement Handle 
// RETURNS: the number of records affected by Insert, Update, or Delete 
// SQL statements, or 0 if the statement is a Select statement. 
//--------------------------------------------------------------------------

//-----------------------------
// Column Definition functions 
//-----------------------------

//--------------------------------------------------------------------------
qeAPI(short) qeNumCols (
	qeHANDLE	hstmt);		// Statement Handle 
// RETURNS: the number of columns in a Select statement, or 0 for 
// non-SELECT statements. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeColName (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
qeAPI(qeSTATUS) qeColNameBuf (
	qeHANDLE	hstmt,		// Statement Handle 
	qeLPSTR		col_name,	// Buffer for column name 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the name of the col_numth column in a SELECT statement, either 
// as a return value (qeColName) or in a provided buffer (qeColNameBuf). 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeColAlias (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
qeAPI(qeSTATUS) qeColAliasBuf (
	qeHANDLE	hstmt,		// Statement Handle 
	qeLPSTR		col_alias,	// Buffer for column alias 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the alias of the col_numth column in a SELECT statement, either 
// as a return value (qeAliasName) or in a provided buffer (qeAliasNameBuf). 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeColExpr (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
qeAPI(qeSTATUS) qeColExprBuf (
	qeHANDLE	hstmt,		// Statement Handle 
	qeLPSTR		col_expr,	// Buffer for column expression 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the expression of the col_numth column in a SELECT statement, 
// either as a return value (qeColExpr) or in a provided buffer 
// (qeColExprBuf). 
//--------------------------------------------------------------------------
qeAPI(short) qeColType (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the QELIB data type of a column in a Select statement. See the 
// top of this file for the list of QELIB data types. 
//--------------------------------------------------------------------------
qeAPI(short) qeColDBType (
	qeHANDLE	hstmt,		// Statment Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the database data type of a column in a Select statement. See 
// the top of this file for the list of values this function returns. 
//--------------------------------------------------------------------------
qeAPI(long) qeColWidth (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the width (in bytes) of a column in a Select statement. Includes 
// the zero terminator for Character or Date data types. 
//--------------------------------------------------------------------------
qeAPI(short) qeColPrecision (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the number of significant digits for Decimal columns in a Select 
// statement 
//--------------------------------------------------------------------------
qeAPI(short) qeColScale (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the number of digits to the right of decimal point for Decimal 
// columns in a Select statement 
//--------------------------------------------------------------------------
qeAPI(short) qeColDateStart (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the offset of the first significant character for Date/Time 
// columns in a Select statement 
//--------------------------------------------------------------------------
qeAPI(short) qeColDateEnd (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: the offset of the last significant character for Date/Time 
// columns in a Select statement 
//--------------------------------------------------------------------------
qeAPI (short) qeColTypeAttr (
	qeHANDLE	hstmt,		// Identifies a statement execution 
	short		col_num,	// Identifies column in statement 
	short		attribute);	// Which attribute? 
// RETURNS: a column attribute of a Select statement may be searchable,  
// nullable, updatable, money, auto-increment, unsigned 
//---------------------------------------------------------------------------
// These constants are results from the qeColTypeAttr function above. 
// They are also returned in several of the data dictionary routines. 

#define qeCOL_UNKNOWN		100


#define qeATTRIBUTE_UPDATABLE	1			// Attribute 
// qeCOL_UNKNOWN is a possible return value 
#define	qeCOL_READ_ONLY		0			// return value 
#define	qeCOL_WRITABLE		1			// return value 


#define qeATTRIBUTE_NULLABLE	2			// Attribute 
// qeCOL_UNKNOWN is a possible return value 
#define qeCOL_NOT_NULLABLE	0			// return value 
#define qeCOL_NULLABLE		1			// return value 


#define	qeATTRIBUTE_SEARCHABLE	3			// Attribute 
// qeCOL_UNKNOWN is possible return value 
#define	qeCOL_UNSEARCHABLE	0			// return value 
#define	qeCOL_LIKE_ONLY		1			// return value 
#define	qeCOL_ALL_EXCEPT_LIKE	2			// return value 
#define qeCOL_SEARCHABLE	3			// return value 


#define	qeATTRIBUTE_UNSIGNED	4			// Attribute 
#define qeCOL_SIGNED		0			// return value 
#define	qeCOL_UNSIGNED		1			// return value 


#define qeATTRIBUTE_MONEY	5			// Attribute 
#define qeCOL_NOT_MONEY		0			// return value 
#define qeCOL_MONEY		1			// return value 


#define qeATTRIBUTE_AUTO_INCRE	6			// Attribute 
#define qeCOL_NOT_AUTO_INCRE	0			// return value 
#define	qeCOL_AUTO_INCRE	1			// return value 

//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeColDBTypeName (
	qeHANDLE	hstmt,		// Statement Handle
	short		col_num);	// Column number (1-origin)
qeAPI(qeSTATUS) qeColDBTypeNameBuf (
	qeHANDLE	hstmt,		// Statement Handle
	qeLPSTR		type_name,	// Result buffer
	short		col_num);	// Column number (1-origin)
// RETURNS: the database type name for a column in a SELECT statement.  The 
// value will be returned as the result of the function (qeColDBTypeName) or 
// in the supplied buffer (qeColDBTypeNameBuf) 
//--------------------------------------------------------------------------

//-------------------------
// Data Fetching functions 
//-------------------------

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindCol (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPVOID	value_ptr,	// Buffer to receive column values. 
	qeLPLONG	len_ptr);	// Buffer to receive column value 
					// length. 
// Specify the buffers to receive the value and lengths for one column 
// from a Select statement.  Subsequent calls to qeFetch functions will get 
// record from the database and put the column values and their lengths 
// in the buffers. When called, len_ptr must specify the size in 
// bytes of value_ptr. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindColChar (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-origin) 
	qeLPSTR		value_ptr,	// Buffer to receive column values. 
	qeLPLONG	len_ptr,	// Buffer to receive column value 
					// length 
	qeLPSTR		fmt_string);	// Display format 
// Specify the buffers to receive the value and lengths for one column 
// from a Select statement.  Subsequent calls to qeFetch functions will get 
// record from the database and put the column values and their lengths 
// in the buffers.  Converts the value to CHAR. 
// When called, len_ptr must hold the size in bytes of value_ptr. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindColInt (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPINT		value_ptr,	// Buffer to receive column values 
	qeLPLONG	len_ptr);	// Buffer to receive column value 
					// length 
// Specify the buffers to receive the value and lengths for one column 
// from a Select statement.  Subsequent calls to qeFetch functions will get 
// record from the database and put the column values and their lengths 
// in the buffers.  Converts the value to INT. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindColLong (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPLONG	value_ptr,	// Buffer to receive column values 
	qeLPLONG	len_ptr);	// Buffer to receive column value 
					// length 
// Specify the buffers to receive the value and lengths for one column 
// from a Select statement.  Subsequent calls to qeFetch functions will get 
// record from the database and put the column values and their lengths 
// in the buffers.  Converts the value to LONG. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindColFloat (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPFLT		value_ptr,	// Buffer to receive column values 
	qeLPLONG	len_ptr);	// Buffer to receive column value 
					// length 
// Specify the buffers to receive the value and lengths for one column 
// from a Select statement.  Subsequent calls to qeFetch functions will get 
// record from the database and put the column values and their lengths 
// in the buffers.  Converts the value to FLOAT. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindColDouble (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPDBL		value_ptr,	// Buffer to receive column values 
	qeLPLONG	len_ptr);	// Buffer to receive column value 
					// length 
// Specify the buffers to receive the value and lengths for one column 
// from a Select statement.  Subsequent calls to qeFetch functions will get 
// record from the database and put the column values and their lengths 
// in the buffers.  Converts the value to DOUBLE. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindColDecimal (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPDEC		value_ptr,	// Buffer to receive column values 
	qeLPLONG	len_ptr,	// Buffer to receive column value 
					// length 
	short		precision,	// User requested precision 
	short		scale);		// User requested scale 
// Specify the buffers to receive the value and lengths for one column 
// from a Select statement.  Subsequent calls to qeFetch functions will get 
// record from the database and put the column values and their lengths 
// in the buffers.  Converts the value to DECIMAL. 
// When called, len_ptr must contain the size of value_ptr in bytes. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeFetchSetOptions (
	qeHANDLE	hstmt,		// Statement handle 
	long		options);	// Option flags 
// Sets options that determine which fetch commands are allowed. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeFetchGetOptions (
	qeHANDLE	hstmt);	// Statement handle 
// RETURNS: options that determine which fetch commands are allowed.
//---------------------------------------------------------------------------
// Option flag values for qeFetchSet/GetOptions 

#define qeFETCH_FORWARD     0x0000	// Only Fetch Next is allowed 
#define qeFETCH_RANDOM	    0x0001	// Whether Random and Previous 
					// fetching is enabled. 

#define qeFORCE_LOG	    0x0002	// Whether to force a log file when 
					// it is not required. 
//--------------------------------------------------------------------------
// Option flag values for qeSet/GetSelectOptions 

#define qeFETCH_FORWARD_DIR 0x0001	// Only Fetch in the forward 
					// direction is allowed 
#define qeFETCH_ANY_DIR	    0x0002	// Fetching is forward or backward 
					// enabled. 

#define	qeLOG_IF_NEEDED	    0x0008	// Log only if needed for fetch 
					// direction support 
#define qeLOG_ALWAYS	    0x0010	// Whether to force a log file when 
					// it is not required. 

#define qeSELECT_INVALIDATE 0x0020	// disable fetching at EOT 
#define qeSELECT_TRUNCATE   0x0040	// truncate the select solution set 
					// at EOT 
#define qeSELECT_PERSIST    0x0060	// select persists at EOT 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetSelectOptions (
	qeHANDLE	hdbc,		// handle to a connection 
	long		flags);		// New options 
// Sets options that determine which fetch commands and cursor behaviors 
// are allowed. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeGetSelectOptions (
	qeHANDLE	hdbc);		// handle to a connection 
// RETURNS: options that determine which fetch commands are allowed. 
//--------------------------------------------------------------------------
// Option flag values for qeSet/GetOneHstmtPerHdbcOptions 

#define qeREADAHEAD_AT_EXEC        0x0001  // Read entire solution set into 
					   // log at execute time 
#define qeREADAHEAD_AT_UPDATE	   0x0002  // Read remainder of solution set 
					   // into log at update, delete, or 
					   // lock 
#define qeREADAHEAD_COMMIT_UPDATES 0x0003  // All updates must be commited 
					   // before any additional fetching 

#define qeROUTING_READ		   0x0008  // Sends the statement to a read 
					   // connection. 
#define qeROUTING_UPDATE	   0x0010  // Sends the statement to the 
					   // transaction connection 
#define qeROUTING_DEFAULT	   0x0018  // Lets qelib decide which 
					   // connection to send the 
					   // statement 

#define qeHSTMT_LOCAL		   0x0020  // States that this hstmt cannot 
					   // affect any other active hstmts 
					   // in the same app 
#define	qeHSTMT_NONLOCAL	   0x0040  // States that this hstmt may 
					   // affect other hstmts in the 
					   // same app 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetOneHstmtPerHdbcOptions (
	qeHANDLE	hdbc,		// Handle to a connection 
	long		flags);		// New options 

// Sets options that determine which fetch commands and cursor behaviors 
// are allowed. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeGetOneHstmtPerHdbcOptions (
	qeHANDLE	hdbc);		// Handle to a connection 
// RETURNS: options that determine which fetch commands are allowed.
//--------------------------------------------------------------------------
qeAPI(long) qeRecNum (
	qeHANDLE	hstmt);		// Statement handle 
// RETURNS: the current record number.  Returns 0 if there is no current 
// record.
//--------------------------------------------------------------------------
qeAPI(short) qeRecState (
	qeHANDLE	hstmt);		// Statement handle 
// RETURNS: the state of current record, from the values defined below. 
//---------------------------------------------------------------------------
// Values to be returned from qeRecState 
#define qeSTATE_NEW		1 // A new record which has not been sent to 
				  // the database 
#define qeSTATE_UNCHANGED	2 // Record has no changes waiting to be 
				  // sent to the database 
#define qeSTATE_CHANGED		3 // Record has changes waiting to be sent 
				  // to the database 
#define qeSTATE_NOREC		4 // The cursor is not currently positioned 
				  // on a record 
#define qeSTATE_NEW_CHANGED	5 // A new record which has not been sent to 
				  // the database, and which has fields which
				  // have been updated. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeFetchLogClose (
	qeHANDLE	hstmt);		// Statement handle 
// Closes the log files associated with an hstmt.  These files will 
// automatically be re-opened on the next qeFetch call.  Only used 
// to reduce the number of open files. 
// RETURNS: a result code.
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeFetchNext (
	qeHANDLE	hstmt);		// Statement Handle 
// Get the next record from the database system. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeFetchPrev (
	qeHANDLE	hstmt);		// Statement handle 
// Get the previous record from the database system. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeFetchRandom (
	qeHANDLE	hstmt,		// Statement handle 
	long		rec_num);	// Record number (1-origin) 
// Get a record from the database system given its record number. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeFetchNumRecs (
	qeHANDLE	hstmt);		// Statement Handle 
// RETURNS: the number of records resulting from the current Select stmt. 
// Can only be used if qeFetchSetOptions is called enabling Random Reads. 
//--------------------------------------------------------------------------
qeAPI(long) qeDataLen (
	qeHANDLE	hstmt);		// Statement Handle 
// RETURNS: the length of the column value most recently retrieved via a 
// call to a qeVal function.  If the value is Null, qeNULL_DATA (-2) is 
// returned.  If the values is longer than the amount returned, 
// (e.g., qeValChar was called with a non-zero req_len and the 
// value was longer than req_len), qeTRUNCATION (-1) is returned. 
// If a long char field is returned in chunks, the value returned only 
// reflects the last chunk received. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeValChar (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPSTR		fmt_string,	// Controls formatting of dates and 
					// numbers 
	short		max_len);	// The maximum length of data 
					// to be returned. 
qeAPI(qeSTATUS) qeValCharBuf (
	qeHANDLE	hstmt,		// Statement Handle 
	qeLPSTR		char_val,	// Buffer for value 
	short		col_num,	// Column number (1-origin) 
	qeLPSTR		fmt_string,	// Controls formatting of dates and 
					// numbers 
	short		max_len);	// The requested maximum length of 
					// data to be returned. 
// Retrieves a column value as a character string, and returns 
// it (qeValChar) or puts it in a user-defined buffer (qeValCharBuf).  The 
// value will have a zero-terminator appended to it. 
// RETURNS: the string, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeValMultiChar (
	qeHANDLE	hstmt,		// Handle to Select statement 
	short		start_col_num,	// First column to return 
	short		end_col_num,	// Last column to return 
	qeLPSTR		num_fmt_string, // Numeric format string 
	qeLPSTR		date_fmt_string, // Date format string 
	qeLPSTR		separator);	// Character separator 
qeAPI(qeSTATUS) qeValMultiCharBuf (
	qeHANDLE	hstmt,		// Handle to Select statement 
	qeLPSTR		dest_buf,	// Buffer for result string 
	short		start_col_num,	// First column to return 
	short		end_col_num,	// Last column to return 
	qeLPSTR		num_fmt_string, // Numeric format string 
	qeLPSTR		date_fmt_string, // Date format string 
	qeLPSTR		separator);	// Character separator 
// Returns/retrieves a pointer to a set of column values in a single 
// character string.  Each column's value will be separated by the 
// specified character (typically tab or comma).  The last value will be 
// followed by a  zero-terminator character.  All numeric values will be 
// formatted using the num_fmt_string and all date values will be formatted 
// using the date_fmt_string. 
// RETURNS: the pointer to the set of column values, or a result code if 
// using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(short) qeValInt (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: a column value as a 2-byte integer 
//--------------------------------------------------------------------------
qeAPI(long) qeValLong (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// RETURNS: a column value as a 4-byte integer 
//--------------------------------------------------------------------------
qeAPI(qeRETFLT) qeValFloat (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num		// Column number (1-origin) 
	qeHIDDENFLT);			// see Borland notes 
// RETURNS: a column value as a 4-byte floating point number 
//--------------------------------------------------------------------------
qeAPI(qeRETDBL) qeValDouble (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num		// Column number (1-origin) 
	qeHIDDENDBL);			// see Borland notes 
// RETURNS: a column value as an 8-byte floating point number 
//--------------------------------------------------------------------------
qeAPI(qeLPDEC) qeValDecimal (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	short		precision,	// Number of significant digits in 
					// result 
	short		scale);		// Number of digits right of decimal 
					// in result. 
qeAPI(qeSTATUS) qeValDecimalBuf (
	qeHANDLE	hstmt,		// Statement Handle 
	qeLPSTR		dec_val,	// Buffer for value 
	short		col_num,	// Column number (1-origin) 
	short		precision,	// Number of significant digits in 
					// result 
	short		scale);		// Number of digits right of decimal 
					// in result. 
// Returns/Retrieves a column value as a decimal number in BCD format. 
// RETURNS: the column value, or a result code if using the buf version. 
//--------------------------------------------------------------------------

//-----------------------
// Update functions      
//-----------------------

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetKey (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	short		value);		// TRUE/FALSE column is part of key 
// Declares that the column is part of a primary key for the record. 
// The primary key is one or more columns that together uniquely identify 
// records.  This is used by qeUpdate and qeDelete to identify the record 
// that is to be deleted. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(short) qeRecGetKey (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// Checks if the column is part of a primary key for the record. 
// The primary key is one or more columns that together uniquely identify 
// records.  This is used by qeUpdate and qeDelete to identify the record 
// that is to be deleted. 
// RETURNS: TRUE (1) if the column is part of the key, FALSE (0) otherwise. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecNew (
	qeHANDLE	hstmt,		// Statement handle 
	long		rec_num);	// Position for the new record 
// Clear the field values in the current record buffer.  Following this, 
// values may be written to the buffer with qePut* functions, and the 
// record can be inserted with qeUpdate. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecLock (
	qeHANDLE	hstmt);	// Statement handle 
// Locks the current record. RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecUpdate (
	qeHANDLE	hstmt);	// Statement handle 
// Update the current record with new values set using qePut functions. 
// This will Insert records created via qeRecNew. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecDelete (
	qeHANDLE	hstmt);	// Statement handle 
// Deletes the current record.  RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecUndo (
	qeHANDLE	hstmt);	// Statement handle 
// Undo all changes that have been made to the current record.  This will 
// undo changes made to the current record that have not been sent to the 
// database via qeUpdate. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutChar (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPSTR		fmt_string,	// Controls formatting of dates and 
					// numbers 
	qeLPSTR		new_val);	// New value for this column 
// Updates a column value in the current record 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutBinary (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	qeLPSTR		new_val,	// New value for this column 
	long		val_len);	// Length of new value in bytes 
// Updates a column value in the current record 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutDecimal (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	short		precision,	// Number of significant digits in 
					// result 
	short		scale,		// Number of digits right of decimal 
					// in result 
	qeLPSTR		new_val);	// New value for this column 
// Updates a column value in the current record 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutDouble (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	double		new_val);	// New value for this column 
// Updates a column value in the current record 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutFloat (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	float		new_val);	// New value for this column 
// Updates a column value in the current record 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutInt (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	short		new_val);	// New value for this column 
// Updates a column value in the current record 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutLong (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num,	// Column number (1-origin) 
	long		new_val);	// New value for this column 
// Updates a column value in the current record 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutNull (
	qeHANDLE	hstmt,		// Statement Handle 
	short		col_num);	// Column number (1-origin) 
// Updates a column value in the current record to have the value NULL 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qePutUsingBindColumns (
	qeHANDLE	hstmt);	// Statement Handle 
// Updates the column values in the current record to reflect the values 
// in the current bind buffers.  If the length buffer is equal to 
// qeNO_DATA_CHANGE, then that column is ignored and the column is unchanged 
// RETURNS: a result code. 

#define qeNO_DATA_CHANGE	(-9)	// Indicates column has not changed 
					// if length buffer of bound columns 
					// is set to this value 
//--------------------------------------------------------------------------

//-----------------------
// Transaction functions 
//-----------------------

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBeginTran (
	qeHANDLE	hdbc);		// Connection Handle 
// Begin a transaction. RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeCommit (
	qeHANDLE	hdbc);		// Connection Handle 
// Commits all changes since qeBeginTran and ends the transaction. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRollback (
	qeHANDLE	hdbc);		// Connection Handle 
// Rolls back all changes since qeBeginTran and ends the transaction. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------

//---------------------------
// Data Conversion Functions 
//---------------------------

//--------------------------------------------------------------------------

	//---------------------
	// To Hex String       
	//---------------------

//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeBinToHex (
	qeLPSTR		bin_value,	// Value to convert 
	short		length);	// Length of value to convert 
qeAPI(qeSTATUS) qeBinToHexBuf (
	qeLPSTR		hex_value,	// buffer to convert value into 
	qeLPSTR		bin_value,	// Value to convert 
	short		length);	// Length of value to convert 
// Converts a binary value into a hex string. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//--------------------------------------------------------------------------

	//---------------------
	// From Hex String     
	//---------------------

//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeHexToBin (
	qeLPSTR		hex_value,	// Value to convert 
	long		length);	// Length of value to convert 
qeAPI(qeSTATUS) qeHexToBinBuf (
	qeLPSTR		bin_value,	// buffer to convert value into 
	qeLPSTR		hex_value,	// Value to convert 
	long		length);	// Length of value to convert 
// Convert a hex string into a binary value. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//--------------------------------------------------------------------------

	//---------------------
	// To Character String 
	//---------------------

//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeDateToChar (
	qeLPSTR		date_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
qeAPI(qeSTATUS) qeDateToCharBuf (
	qeLPSTR		char_val,	// Buffer for character string result
	qeLPSTR		date_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
// Convert a standard date value to a character string formatted using 
// the format string. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeIntToChar (
	short		int_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion
qeAPI(qeSTATUS) qeIntToCharBuf (
	qeLPSTR		char_val,	// Buffer for character string result
	short		int_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
// Convert a 2-byte integer value to a character string formatted using 
// the format string. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeLongToChar (
	long		long_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion
qeAPI(qeSTATUS) qeLongToCharBuf (
	qeLPSTR		char_val,	// Buffer for character string result
	long		long_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
// Convert a 4-byte integer value to a character string formatted using 
// the format string. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeFloatToChar (
	float		flt_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion
qeAPI(qeSTATUS) qeFloatToCharBuf (
	qeLPSTR		char_val,	// Buffer for character string result
	float		flt_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
// Convert a 4-byte floating value to a character string formatted using 
// the format string. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeDoubleToChar (
	double		dbl_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion
qeAPI(qeSTATUS) qeDoubleToCharBuf (
	qeLPSTR		char_val,	// Buffer for character string result
	double		dbl_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
// Convert an 8-byte floating value to a character string formatted using 
// the format string. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeDecimalToChar (
	qeLPDEC		dec_val,	// Value to convert 
	short		precision,	// Number of significant digits in 
					// value 
	short		scale,		// Number of digits right of decimal 
					// in value. 
	qeLPSTR		fmt_string);	// Format string for the conversion
qeAPI(qeSTATUS) qeDecimalToCharBuf (
	qeLPSTR		char_val,	// Buffer for character string result
	qeLPDEC		dec_val,	// Value to convert 
	short		precision,	// Number of significant digits in 
					// value 
	short		scale,		// Number of digits right of decimal 
					// in  value. 
	qeLPSTR		fmt_string);	// Format string for the conversion
// Convert a decimal value to a character string formatted using 
// the format string. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//-------------------------------------------------------------------------

	//------------
	//   To Date  
	//------------

//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeCharToDate (
	qeLPSTR		char_value,	// Char string of a formatted date 
					// value 
	qeLPSTR		format_string);	// Format template 
qeAPI(qeSTATUS) qeCharToDateBuf (
	qeLPSTR		date_value,	// Destination buffer 
	qeLPSTR		char_value,	// Char string of a formatted date 
					// value 
	qeLPSTR		fmt_string);	// Format template 
// Converts a char string of a formatted date value into the default date 
// value. 
// RETURNS: the converted value, or a result code if using the Buf version. 
//--------------------------------------------------------------------------

	//------------
	// To Integer 
	//------------

//--------------------------------------------------------------------------
qeAPI(short) qeCharToInt (
	qeLPSTR		char_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
// Convert a character string to a 2-byte integer.  The character string 
// is formatted as specified by the format_string.  
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(short) qeLongToInt (
	long		long_val);	// Value to convert 
// Convert a 4-byte integer to a 2-byte integer. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(short) qeFloatToInt (
	float		flt_val);	// Value to convert 
// Convert a 4-byte float to a 2-byte integer. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(short) qeDoubleToInt (
	double		dbl_val);	// Value to convert 
// Convert a 8-byte float to a 2-byte integer. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(short) qeDecimalToInt (
	qeLPDEC		dec_val,	// Value to convert 
	short		precision,	// Number of significant digits in 
					// value 
	short		scale);		// Number of digits right of decimal 
					// in value. 
// Convert a decimal number to a 2-byte integer.  
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------

	//-----------------
	// To Long Integer 
	//-----------------

//--------------------------------------------------------------------------
qeAPI(long) qeCharToLong (
	qeLPSTR		char_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
// Convert a character string to a 4-byte integer.  The character string 
// is formatted as specified by the format_string. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(long) qeDateToLong (
	qeLPSTR		date_val);	// Value to convert 
// Convert a standard date value to a Julian value returned as a 4-byte 
// integer. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(long) qeIntToLong (
	short		int_val);	// Value to convert 
// Convert a 2-byte integer to a 4-byte integer. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(long) qeFloatToLong (
	float		flt_val);	// Value to convert 
// Convert a 4-byte float to a 4-byte integer. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(long) qeDoubleToLong (
	double		dbl_val);	// Value to convert 
// Convert a 8-byte float to a 4-byte integer. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------
qeAPI(long) qeDecimalToLong (
	qeLPDEC		dec_val,	// Value to convert 
	short		precision,	// Number of significant digits in 
					// value 
	short		scale);		// Number of digits right of decimal 
					// in value. 
// Convert a decimal number to a 4-byte integer. 
// RETURNS: the converted integer. 
//--------------------------------------------------------------------------

	//-------------------
	// To Floating Point 
	//-------------------

//--------------------------------------------------------------------------
qeAPI(qeRETFLT) qeCharToFloat (
	qeLPSTR		char_val,	// Value to convert 
	qeLPSTR		fmt_string	// Format string for the conversion 
	qeHIDDENFLT);			// see Borland notes 
// Convert a character string to a 4-byte float.  The character string 
// is formatted as specified by the format_string. 
// RETURNS: the converted float. 
//--------------------------------------------------------------------------
qeAPI(qeRETFLT) qeIntToFloat (
	short		int_val		// Value to convert 
	qeHIDDENFLT);			// see Borland notes 
// Convert a 2-byte integer to a 4-byte float. 
// RETURNS: the converted float. 
//--------------------------------------------------------------------------
qeAPI(qeRETFLT) qeLongToFloat (
	long		long_val	// Value to convert 
	qeHIDDENFLT);			// see Borland notes 
// Convert a 4-byte integer to a 4-byte float. 
// RETURNS: the converted float. 
//--------------------------------------------------------------------------
qeAPI(qeRETFLT) qeDoubleToFloat (
	double		dbl_val		// Value to convert 
	qeHIDDENFLT);			// see Borland notes 
// Convert an 8-byte float to a 4-byte float. 
// RETURNS: the converted float. 
//--------------------------------------------------------------------------
qeAPI(qeRETFLT) qeDecimalToFloat (
	qeLPDEC		dec_value,	// Value to convert 
	short		precision,	// Number of significant digits in 
					// value 
	short		scale		// Number of digits right of decimal 
					// in value 
	qeHIDDENFLT);			// see Borland notes 
// Convert a decimal number to a 4-byte float. 
// RETURNS: the converted float. 
//--------------------------------------------------------------------------

	//------------------------------------
	// To Double Precision Floating Point 
	//------------------------------------

//--------------------------------------------------------------------------
qeAPI(qeRETDBL) qeCharToDouble (
	qeLPSTR		char_val,	// Value to convert 
	qeLPSTR		fmt_string	// Format string for the conversion 
	qeHIDDENDBL);			// see Borland notes 
// Convert a character string to an 8-byte float.  The character string 
// is formatted as specified by the format_string. 
// RETURNS: the converted double. 
//--------------------------------------------------------------------------
qeAPI(qeRETDBL) qeDateToDouble (
	qeLPSTR		date_val	// Value to convert 
	qeHIDDENDBL);			// see Borland notes 
// Convert a standard date value to a Julian value returned as an 8-byte 
// float. 
// RETURNS: the converted double. 
//--------------------------------------------------------------------------
qeAPI(qeRETDBL) qeIntToDouble (
	short		int_val		// Value to convert 
	qeHIDDENDBL);			// see Borland notes 
// Convert a 2-byte integer to an 8-byte float. 
// RETURNS: the converted double. 
//--------------------------------------------------------------------------
qeAPI(qeRETDBL) qeLongToDouble (
	long		long_val	// Value to convert 
	qeHIDDENDBL);			// see Borland notes 
// Convert a 4-byte integer to an 8-byte float. 
// RETURNS: the converted double. 
//--------------------------------------------------------------------------
qeAPI(qeRETDBL) qeFloatToDouble (
	float		flt_value	// Value to convert 
	qeHIDDENDBL);			// see Borland notes 
// Convert a 4-byte float to an 8-byte float. 
// RETURNS: the converted double. 
//--------------------------------------------------------------------------
qeAPI(qeRETDBL) qeDecimalToDouble (
	qeLPDEC		dec_val,	// Value to convert 
	short		precision,	// Number of significant digits in 
					// value 
	short		scale		// Number of digits right of decimal 
					// in value 
	qeHIDDENDBL);			// see Borland notes 
// Convert a decimal number to an 8-byte float.  
// RETURNS: the converted double. 
//--------------------------------------------------------------------------

	//------------
	// To Decimal 
	//------------

//--------------------------------------------------------------------------
qeAPI(qeLPDEC) qeCharToDecimal (
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of 
					// decimal for  the result. 
	qeLPSTR		char_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
qeAPI(qeSTATUS) qeCharToDecimalBuf (
	qeLPDEC		dec_val,	// Buffer for the decimal result. 
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result. 
	qeLPSTR		char_val,	// Value to convert 
	qeLPSTR		fmt_string);	// Format string for the conversion 
// Convert a character string to a decimal number. 
// RETURNS: the converted decimal, or a result code if using the Buf version 
//--------------------------------------------------------------------------
qeAPI(qeLPDEC) qeIntToDecimal (
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result. 
	short		int_val);	// Value to convert 
qeAPI(qeSTATUS) qeIntToDecimalBuf (
	qeLPDEC		dec_val,	// Buffer for the decimal result. 
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result 
	short		int_val);	// Value to convert 
// Convert a 2-byte integer to a decimal number. 
// RETURNS: the converted decimal, or a result code if using the Buf version 
//--------------------------------------------------------------------------
qeAPI(qeLPDEC) qeLongToDecimal (
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result 
	long		long_val);	// Value to convert 
qeAPI(qeSTATUS) qeLongToDecimalBuf (
	qeLPDEC		dec_val,	// Buffer for the decimal result. 
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result. 
	long		long_val);	// Value to convert 
// Convert a 4-byte integer to a decimal number. 
// RETURNS: the converted decimal, or a result code if using the Buf version 
//--------------------------------------------------------------------------
qeAPI(qeLPDEC) qeFloatToDecimal (
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result. 
	float		flt_val);	// Value to convert 
qeAPI(qeSTATUS) qeFloatToDecimalBuf (
	qeLPDEC		dec_val,	// Buffer for the decimal result. 
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result. 
	float		flt_val);	// Value to convert 
// Convert a 4-byte float to a decimal number.  Returns a result code. 
// RETURNS: the converted decimal, or a result code if using the Buf version 
//--------------------------------------------------------------------------
qeAPI(qeLPDEC) qeDoubleToDecimal (
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result. 
	double		dbl_val);	// Value to convert 
qeAPI(qeSTATUS) qeDoubleToDecimalBuf (
	qeLPDEC		dec_val,	// Buffer for the decimal result. 
	short		precision,	// Number of significant digits for 
					// the result 
	short		scale,		// Number of digits right of decimal 
					// for the result. 
	double		dbl_val);	// Value to convert 
// Convert an 8-byte float to a decimal number.  Returns a result code. 
// RETURNS: the converted decimal, or a result code if using the Buf version 
//--------------------------------------------------------------------------

//-----------------------
// System Info functions 
//-----------------------

//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeVerNum (void);
qeAPI(qeSTATUS) qeVerNumBuf (
	qeLPSTR		ver_num);	// Buffer for the version number 
// Retrieves/Returns a character string containing the QELIB version number.
// RETURNS: the version string, or a result code if using the Buf version.
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeSetupInfo (void);
qeAPI(qeSTATUS) qeSetupInfoBuf (
	qeLPSTR		dest_buf);	// Result buffer 
// Retrieves/Returns a character string containing the User Name, 
// Company Name, and Serial Number as a tab-separated character string. 
// RETURNS: the setup string, or a result code if using the Buf version.
//--------------------------------------------------------------------------

//-----------------------
// Find Functions	 
//-----------------------
//---------------------------------------------------------------------------
// Find operator definitions 

#define qeFIND_LESS_THAN		1
#define qeFIND_LESS_THAN_OR_EQ		2
#define qeFIND_GREATER_THAN		3
#define qeFIND_GREATER_THAN_OR_EQ	4
#define qeFIND_EQUAL			5
#define qeFIND_NOT_EQUAL		6
#define qeFIND_LIKE			7
#define qeFIND_NOT_LIKE			8
#define qeFIND_IN			9
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecClearConditions (
	qeHANDLE	hstmt);		// Statement handle 
// Clear all find conditions for a stmt. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetConditionBinary (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-based) 
	short		oper,		// Comparison operator (see above) 
	qeLPSTR		value,		// Binary comparison value 
	short		length);	// Length of value (in bytes) 
// Sets a condition on a column of type Binary.  This condition will be used 
// in calls to qeRecFind and qeQBEPrepare. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetConditionChar (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-based) 
	short		oper,		// Comparison operator (see above) 
	qeLPSTR		value,		// Char comp value (0-terminated) 
	qeLPSTR		fmt_string,	// Format string 
	short		case_sens);	// Case-sensitive comparison? 
// Sets a condition on a column of type Character. This condition will be 
// used in calls to qeRecFind and qeQBEPrepare. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetConditionDecimal (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-based) 
	short		oper,		// Comparison operator (see above) 
	qeLPSTR		value,		// Decimal comparison value 
	short		precision,	// Precision of value (in digits) 
	short		scale);		// Scale (for decimals) 
// Sets a condition on a column of type Decimal. This condition will be used 
// in calls to qeRecFind and qeQBEPrepare. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetConditionDouble (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-based) 
	short		oper,		// Comparison operator (see above) 
	double		value);		// Double comparison value 
// Sets a condition on a column of type Double.  This condition will be used 
// in calls to qeRecFind and qeQBEPrepare. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetConditionFloat (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-based) 
	short		oper,		// Comparison operator (see above) 
	float		value);		// Float comparison value 
// Sets a condition on a column of type Float.  This condition will be used 
// in calls to qeRecFind and qeQBEPrepare. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetConditionLong (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-based) 
	short		oper,		// Comparison operator (see above) 
	long		value);		// Long comparison value 
// Sets a condition on a column of type Long.  This condition will be used 
// in calls to qeRecFind and qeQBEPrepare. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetConditionInt (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-based) 
	short		oper,		// Comparison operator (see above) 
	short		value);		// Int comparison value 
// Sets a condition on a column of type Integer. This condition will be used 
// in calls to qeRecFind and qeQBEPrepare. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeRecSetConditionNull (
	qeHANDLE	hstmt,		// Statement handle 
	short		col_num,	// Column number (1-based) 
	short		oper);		// Comparison operator (see above) 
// Sets a condition on a column of type NULL.  This condition will be used 
// in calls to qeRecFind and qeQBEPrepare. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeRecFind (
	qeHANDLE	hstmt,		// Statement handle 
	short		start_pos,	// Starting position 
	short		flags);		// Direction to search 
// Searches the current result set for a row which matches the search 
// conditions which were set using the qeRecSetCondition functions. 
// Uses start_pos to figure where to begin the search, and direction 
// to indicate which direction to search in.  If the record is found, 
// it becomes the current record.  Otherwise, the current record remains 
// unchanged. 
// RETURNS: the new record number, if the find is successful, 0 otherwise. 
//---------------------------------------------------------------------------
// Start position constants 

#define	qeFIND_BEGIN	1	// from beginning of result set 
#define	qeFIND_END	2	// from end of result set 
#define	qeFIND_CURRENT	3	// from current position in result set 
//---------------------------------------------------------------------------
// Direction constants 

#define qeFIND_BACKWARD 0x0001	// set this flag for backward finds 
				// default is forward 
#define qeFIND_SKIP_ROW 0x0002	// set this flag to skip the current row 
				// meaningful only for qeFIND_CURRENT 
				// default is to start with current row 
//-------------------------------------------------------------------------
qeAPI(qeHANDLE) qeQBEPrepare (
	qeHANDLE	hstmt);		// Statement handle 
// Create a new hstmt which is the result of adding the FIND conditions 
// of the old hstmt into the WHERE clause of the old statement. This 
// statement can be executed with qeSQLExecute. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeClauseGet (
	qeHANDLE	hstmt,		// Statement handle 
	short		which_clause);	// Which clause to return 
qeAPI(qeSTATUS) qeClauseGetBuf (
	qeHANDLE	hstmt,		// Statement handle 
	short		which_clause,	// Which clause to return 
	qeLPSTR		clause_buf);	// Clause returned 
// RETURNS: a clause from the SELECT statement, or a result code if using 
// the Buf version.
//---------------------------------------------------------------------------
// Clause defines 

#define qeCLAUSE_WHERE		1	// Return WHERE clause 
#define qeCLAUSE_HAVING		2	// Return HAVING clause 
#define qeCLAUSE_GROUPBY	3	// Return GROUP BY clause 
#define qeCLAUSE_ORDERBY	4	// Return ORDER BY clause 
#define qeCLAUSE_COMPUTEBY	5	// Return COMPUTE BY clause 
#define	qeCLAUSE_FROM		6	// Return FROM clause 
#define qeCLAUSE_OTHER		7	// Return OTHER clause 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeUniqueWhereClause (
	qeHANDLE	hstmt);		// Statement handle 
qeAPI(qeSTATUS) qeUniqueWhereClauseBuf (
	qeHANDLE	hstmt,		// Statement handle 
	qeLPSTR		clause_buf);	// Unique where clause returned 
// Return/Retrieve a where clause which uniquely identifies the current row 
// in an active SELECT statement. 
// Will use the columns specified to qeKeySet (if that function has 
// been called) or will generate the list of columns on its own. 
// Returns: the clause, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeNativeSQL (
	qeHANDLE	hstmt);		// Statement handle 
qeAPI(qeSTATUS) qeNativeSQLBuf (
	qeHANDLE	hstmt,		// Statement handle 
	qeLPSTR		stmt_buf);	// Native SQL returned 
// RETURNS: the SQL string as translated by driver, or a result code if 
// using the Buf version. 
//--------------------------------------------------------------------------

//---------------------------------
// Parameter Bind and Set Routines 
//---------------------------------

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeClearParam (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num);	// Parameter number (1-based) 
// Clears any parameter information for a parameter. 
// qeSetParamNull, qeSetParam<type>, or qeBindParam<type> must 
// be called before the	statement is executed again. 
// Otherwise an error will be returned on the next execution. 
// This call allows resetting a chunked parameter. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamNull (
	qeHANDLE hstmt,		// Statement handle 
	short	param_num,	// Parameter number (1-based) 
	short	param_type,	// QELIB data type 
	long	precision,	// Decimal's precision, maximum 
				// size of a character, or length (in 
				// bytes) of a datetime value 
	short	scale);		// Decimal's scale 
// Sets this parameter to NULL.  To supply a value for 
// this parameter, call qeSetParam<type> or qeBindParam<type>. 
// qeNO_DATA_TYPE is legal only if this parameter has 
// previously been given a type with qeSetParamNull,
// qeSetParam<type>, or qeBindParam<type>. 
// RETURNS: a result code. 

#define qeNO_DATA_TYPE 0
//--------------------------------------------------------------------------
qeAPI(short) qeParamNum (
	qeHANDLE	hstmt,		// Statement handle 
	qeLPSTR		param_name);	// Parameter name 
// RETURNS: the parameter number corresponding to the parameter name
// (1-based).If the name does not match any parameter, 0 (zero) is returned. 
// If the parameter name matches more than one parameter number, the 
// lowest number will be returned. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamBinary (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Binary value 
	qeLPLONG	param_len);	// Binary value length in bytes 
// Binds this parameter to the value passed.  The param_val pointer and 
// param_len pointer are saved and both must be valid when the statement 
// is executed. 
// When called, param_len should contain the maximum length for the value. 
// Set param_len to qeNULL_DATA before execution for this parameter to be 
// null. This parameter will continue to point to this value until 
// qeSetParamNull, qeSetParam<type>, or qeBindParam<type> is called again 
// for this parameter. Note that all parameters with the same name as 
// parameter number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamBinary (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Binary value 
	long		param_len);	// Binary value length in bytes 
// Sets this parameter to the param_val passed. A copy of the value passed is
// made so the pointer need not remain valid after this call. 
// This parameter will continue to have this value until qeSetParamNull, 
// qeSetParam<type>, or qeBindParam<type> is called again for this 
// parameter. Note that all parameters with the same name as parameter 
// number will be changed/affected. This function may called multiple times 
// for large values. At statement execution, the values will be put 
// together in the order this function was called. Lengths of zero will be 
// ignored. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamChar (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Character value, (0-terminated) 
	qeLPLONG	param_len);	// Character value length in bytes 
// Binds this parameter to the value passed.  The param_val and param_len 
// pointers are saved and must be valid when the statement is executed. 
// When called, param_len should contain the maximum length for the value. 
// Set param_len to qeNULL_DATA before execution for this parameter to be 
// null. This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>,  or qeBindParam<type> is called 
// again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamChar (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Character value, (0-terminated) 
	long		max_len);	// Maximum length of char value 
// Sets this parameter to the param_val passed. A copy of the value passed is
// made so the pointer need not remain valid after this call. 
// This parameter will continue to have this value until qeSetParamNull, 
// qeSetParam<type>, or qeBindParam<type> is called again for this 
// parameter. Note that all parameters with the same name as parameter 
// number will be changed/affected. This function may called multiple 
// times for large values. At statement execution, the values will be put 
// together in the order this function was called. Lengths of zero will 
// be ignored. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamDate (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Date value, 26-byte string 
	qeLPLONG	param_len);	// Date value length in bytes 
// Binds this parameter to the param_val passed.  The param_val and 
// param_len pointers are saved and must be valid when the statement is 
// executed. When called, param_len should contain the maximum length for 
// the value.  Set length to qeNULL_DATA before execution for this parameter 
// to be null.  This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>,  or qeBindParam<type> is called 
// again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamDate (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val);	// Date value, 26-byte string 
// Sets this parameter to the param_val passed. A copy of the value passed is
// made so the pointer need not remain valid after this call. This 
// parameter will continue to have this value until qeSetParamNull, 
// qeSetParam<type>, or qeBindParam<type> is called again for this 
// parameter. Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamDateTime (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Date time value, 26-byte string 
	qeLPLONG	param_len);	// Date time value length in bytes 
// Binds this parameter to the value passed.  The param_val and param_len 
// pointers are saved and must be valid when the statement is executed. 
// When called, param_len should contain the maximum length for the value. 
// Set length to qeNULL_DATA before execution for this parameter to be null. 
// This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>, 
// or qeBindParam<type> is called again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamDateTime (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Date time value, 26-byte string 
	short		precision);	// Date time precision 
// Sets this parameter to the param_val passed. A copy of the value passed is
// made so the pointer need not remain valid after this call. This 
// parameter will continue to have this value until qeSetParamNull, 
// qeSetParam<type>, or qeBindParam<type> is called again for this 
// parameter. Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamDecimal (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Decimal value 
	qeLPLONG	param_len,	// Decimal value length in bytes 
	short		scale);		// Decimal's scale 
// Binds this parameter to the value passed.  The param_val and param_len 
// pointers are saved and must be valid when the statement is executed. 
// When called, param_len should contain the maximum length for the value. 
// Set length to qeNULL_DATA before execution for this parameter to be null. 
// This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>, 
// or qeBindParam<type> is called again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamDecimal (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Decimal value 
	short		precision,	// Decimal's precision 
	short		scale);		// Decimal's scale 
// Sets this parameter to the param_val passed. A copy of the value passed is
// made so the pointer need not remain valid after this call. This 
// parameter will continue to have this value until qeSetParamNull, 
// qeSetParam<type>, or qeBindParam<type> is called again for this 
// parameter. Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamDouble (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPDBL		param_val,	// Double value 
	qeLPLONG	param_len);	// Double value length in bytes 
// Binds this parameter to the value passed.  The param_val and param_len 
// pointers are saved and must be valid when the statement is executed. 
// The param_len pointer may be NULL. 
// Set length to qeNULL_DATA before execution for this parameter to be null. 
// This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>, 
// or qeBindParam<type> is called again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamDouble (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	double		param_val);	// Double value 
// Sets this parameter to the param_val passed. This parameter will 
// continue to have this value until qeSetParamNull, qeSetParam<type>, or 
// qeBindParam<type> is called again for this parameter.  Note that all 
// parameters with the same name as parameter number will be 
// changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamFloat (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPFLT		param_val,	// Float value 
	qeLPLONG	param_len);	// Float value length in bytes 
// Binds this parameter to the param_val pointer passed.  The param_val and 
// param_len pointers are saved and must be valid when the statement is 
// executed. The param_len pointer may be NULL. 
// Set length to qeNULL_DATA before execution for this parameter to be null. 
// This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>, 
// or qeBindParam<type> is called again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamFloat (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	float		value);		// Float value 
// Sets this parameter to the value passed.  This parameter will continue to
// have this value until qeSetParamNull, 
// qeSetParam<type>, or qeBindParam<type> is called again for this 
// parameter.  Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamInt (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPINT		param_val,	// Int value 
	qeLPLONG	param_len);	// Int value length in bytes 
// Binds this parameter to the value passed.  The param_val and param_len 
// pointers are saved and must be valid when the statement is executed. 
// The length pointer may be NULL. 
// Set length to qeNULL_DATA before execution for this parameter to be null. 
// This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>, 
// or qeBindParam<type> is called again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamInt (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	short		param_val);	// Int value 
// Sets this parameter to the param_val passed. A copy of the value passed is
// made so the pointer need not remain valid after this call. This 
// parameter will continue to have this value until qeSetParamNull, 
// qeSetParam<type>, or qeBindParam<type> is called again for this 
// parameter. Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamLong (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPLONG	param_val,	// Long value 
	qeLPLONG	param_len);	// Long value length in bytes 
// Binds this parameter to the value passed.  The param_val and param_len 
// pointers are saved and must be valid when the statement is executed. 
// The length pointer may be NULL. 
// Set param_len to qeNULL_DATA before execution for this parameter to be 
// null. This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>, 
// or qeBindParam<type> is called again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamLong (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	long		param_val);	// Long value 
// Sets this parameter to the param_val passed. This parameter will continue 
// to have this value until qeSetParamNull, qeSetParam<type>, or 
// qeBindParam<type> is called again for this parameter. Note that all 
// parameters with the same name as parameter number will be 
// changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeBindParamTime (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val,	// Time value, 26-byte string 
	qeLPLONG	param_len);	// Time value length in bytes 
// Binds this parameter to the value passed.  The param_val and param_len 
// pointers  are saved and must be valid when the statement is executed. 
// When called, param_len should contain the maximum length for the value. 
// Set length to qeNULL_DATA before execution for this parameter to be null. 
// This parameter will continue to point to this value 
// until qeSetParamNull, qeSetParam<type>, 
// or qeBindParam<type> is called again for this parameter. 
// Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetParamTime (
	qeHANDLE	hstmt,		// Statement handle 
	short		param_num,	// Parameter number (1-based) 
	qeLPSTR		param_val);	// Date time value, 26-byte string 
// Sets this parameter to the param_val passed. A copy of the value passed is
// made so the pointer need not remain valid after this call. This 
// parameter will continue to have this value until qeSetParamNull, 
// qeSetParam<type>, or qeBindParam<type> is called again for this 
// parameter. Note that all parameters with the same name as parameter 
// number will be changed/affected. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(short) qeNumParams (
	qeHANDLE	hstmt);		// Statement handle 
// RETURNS: the number of question mark parameters in the statement 
// represented by hstmt. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeMoreResults (
	qeHANDLE	hstmt);		// Statement handle 
// Ends the current result set and starts a new result set. If EOF is 
// returned, there are on more result sets. This procedure is used to 
// handle statements which can return multiple result sets, such as stored 
// procedures. 
// RETURNS: a result code, which is qeEOF if there are no more results. 
//--------------------------------------------------------------------------
qeAPI(qeLPVOID) qeGetODBCHenv (void);
// RETURNS: the ODBC henv for QELIB. qeLibInit and qeLibTerm invalidate the 
// henv returned. 
//--------------------------------------------------------------------------
qeAPI(qeLPVOID) qeGetODBCHdbc (
	qeHANDLE	hdbc);		// Connection handle 
// RETURNS: the ODBC hdbc associated with the QELIB hdbc. 
// qeLibInit, qeLibTerm, and qeDisconnect invalidate the hdbc returned.
//--------------------------------------------------------------------------
qeAPI(qeLPVOID) qeGetODBCHstmt (
	qeHANDLE	hstmt);		// Statement handle 
// RETURNS: the ODBC hstmt associated with the QELIB hstmt. 
// qeLibInit, qeLibTerm, qeDisconnect, and qeEndSQL 
// invalidate the hdbc returned. 
//--------------------------------------------------------------------------
qeAPI(qeHANDLE) qeSetODBCHdbc (
	qeLPVOID	ODBCHdbc);		// ODBC connection handle 
// RETURNS: a QELIB connection for the ODBC hdbc. 
//--------------------------------------------------------------------------
// Definitions for the qeGetInfo calls 

#define qeINFO_ACTIVE_CONNECTIONS	 0	// Long 
#define qeINFO_ACTIVE_STATEMENTS	 1	// Long 
#define qeINFO_DRIVER_VER		 7	// Char 
#define qeINFO_SEARCH_PATTERN_ESCAPE	14	// Char 
#define qeINFO_DATA_SOURCE_READ_ONLY	25	// Char 
#define qeINFO_EXPRESSIONS_IN_ORDERBY	27	// Char 
#define qeINFO_IDENTIFIER_CASE		28	// Long 
#define qeINFO_IDENTIFIER_QUOTE_CHAR	29	// Char 
#define qeINFO_MAX_COLUMN_NAME_LEN	30	// Long 
#define qeINFO_MAX_CURSOR_NAME_LEN	31	// Long 
#define qeINFO_MAX_OWNER_NAME_LEN	32	// Long 
#define qeINFO_MAX_PROCEDURE_NAME_LEN	33	// Long 
#define qeINFO_MAX_QUALIFIER_NAME_LEN	34	// Long 
#define qeINFO_MAX_TABLE_NAME_LEN	35	// Long 
#define qeINFO_OUTER_JOINS		38	// Char 
#define qeINFO_OWNER_TERM		39	// Char 
#define qeINFO_PROCEDURE_TERM		40	// Char 
#define qeINFO_QUALIFIER_NAME_SEPARATOR	41	// Char 
#define qeINFO_QUALIFIER_TERM		42	// Char 
#define qeINFO_TABLE_TERM		45	// Char 
#define qeINFO_CONVERT_FUNCTIONS	48	// Long 
#define qeINFO_NUMERIC_FUNCTIONS	49	// Long 
#define qeINFO_STRING_FUNCTIONS		50	// Long 
#define qeINFO_SYSTEM_FUNCTIONS		51	// Long 
#define qeINFO_TIMEDATE_FUNCTIONS	52	// Long 

// Conversion functions 
#define qeSQL_FN_CVT_CONVERT                  0x00000001L

// String functions 

#define qeSQL_FN_STR_CONCAT                 0x00000001L
#define qeSQL_FN_STR_INSERT                 0x00000002L
#define qeSQL_FN_STR_LEFT                   0x00000004L
#define qeSQL_FN_STR_LTRIM                  0x00000008L
#define qeSQL_FN_STR_LENGTH                 0x00000010L
#define qeSQL_FN_STR_LOCATE                 0x00000020L
#define qeSQL_FN_STR_LCASE                  0x00000040L
#define qeSQL_FN_STR_REPEAT                 0x00000080L
#define qeSQL_FN_STR_REPLACE                0x00000100L
#define qeSQL_FN_STR_RIGHT                  0x00000200L
#define qeSQL_FN_STR_RTRIM                  0x00000400L
#define qeSQL_FN_STR_SUBSTRING              0x00000800L
#define qeSQL_FN_STR_UCASE                  0x00001000L
#define qeSQL_FN_STR_ASCII                  0x00002000L
#define qeSQL_FN_STR_CHAR                   0x00004000L

// Numeric functions 

#define qeSQL_FN_NUM_ABS                    0x00000001L
#define qeSQL_FN_NUM_ACOS                   0x00000002L
#define qeSQL_FN_NUM_ASIN                   0x00000004L
#define qeSQL_FN_NUM_ATAN                   0x00000008L
#define qeSQL_FN_NUM_ATAN2                  0x00000010L
#define qeSQL_FN_NUM_CEILING                0x00000020L
#define qeSQL_FN_NUM_COS                    0x00000040L
#define qeSQL_FN_NUM_COT                    0x00000080L
#define qeSQL_FN_NUM_EXP                    0x00000100L
#define qeSQL_FN_NUM_FLOOR                  0x00000200L
#define qeSQL_FN_NUM_LOG                    0x00000400L
#define qeSQL_FN_NUM_MOD                    0x00000800L
#define qeSQL_FN_NUM_SIGN                   0x00001000L
#define qeSQL_FN_NUM_SIN                    0x00002000L
#define qeSQL_FN_NUM_SQRT                   0x00004000L
#define qeSQL_FN_NUM_TAN                    0x00008000L
#define qeSQL_FN_NUM_PI                     0x00010000L
#define qeSQL_FN_NUM_RAND                   0x00020000L

// Time/date functions 

#define qeSQL_FN_TD_NOW                     0x00000001L
#define qeSQL_FN_TD_CURDATE                 0x00000002L
#define qeSQL_FN_TD_DAYOFMONTH              0x00000004L
#define qeSQL_FN_TD_DAYOFWEEK               0x00000008L
#define qeSQL_FN_TD_DAYOFYEAR               0x00000010L
#define qeSQL_FN_TD_MONTH                   0x00000020L
#define qeSQL_FN_TD_QUARTER                 0x00000040L
#define qeSQL_FN_TD_WEEK                    0x00000080L
#define qeSQL_FN_TD_YEAR                    0x00000100L
#define qeSQL_FN_TD_CURTIME                 0x00000200L
#define qeSQL_FN_TD_HOUR                    0x00000400L
#define qeSQL_FN_TD_MINUTE                  0x00000800L
#define qeSQL_FN_TD_SECOND                  0x00001000L

// System functions 

#define qeSQL_FN_SYS_USERNAME               0x00000001L
#define qeSQL_FN_SYS_DBNAME                 0x00000002L
#define qeSQL_FN_SYS_IFNULL                 0x00000004L

//---------------------------------------------------------------------------
qeAPI(long) qeGetODBCInfoLong (
	qeHANDLE	hdbc,		//Connection handle 
	short		option);	// Which info to get (see above) 
// RETURNS: the information requested. 
//---------------------------------------------------------------------------
qeAPI(qeSTATUS) qeGetODBCInfoCharBuf (
	qeHANDLE	hdbc,		// Connection handle 
	short		option,		// Which info to get (see above) 
	qeLPSTR		char_val);		// Value returned 
// RETURNS: the information requested. 
//---------------------------------------------------------------------------
qeAPI(qeLPSTR) qeGetODBCInfoChar (
	qeHANDLE	hdbc,		// Connection handle 
	short		option);	// Which info to get (see above) 
// RETURNS: the information requested. 
//--------------------------------------------------------------------------

//--------------
// Query Builder
//--------------

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQryBuilder (
	qeHANDLE	hqry,		// Query Builder handle 
	short		parent_window,	// Handle to parent window of 
					// caller.  May be zero. 
	short		flags,		// Options: see choices below 
	short		init_dialog);	// Initial dialog box: see below 
// Runs the Query Builder based on the query represented by hqry.  Any 
// editing is applied to the hqry. 
// RETURNS: a result code. 
//---------------------------------------------------------------------------
// Definitions for options parameter 

#define qeQRY_NO_COL_ALIAS	0x0001	// Column aliases disallowed 
#define qeQRY_EXIT_AFTER_DLG	0x0002	// Exit after dialog is exited. 
					//	Valid only if initial 
					//	dialog is specified. 
#define qeQRY_ALLOW_SRC_CHANGE	0x0004	// Source can be changed in file 
					//	open box. 
#define qeQRY_SYSTABLES		0x0008	// List system tables in table 
					//	dialog box. 
#define qeQRY_SYNONYMS		0x0010	// List synonyms in table 
					//	dialog box. 
#define qeQRY_TABLES		0x0020	// List tables in table 
					//	dialog box. 
#define qeQRY_VIEWS		0x0040	// List views in table 
					//	dialog box. 
#define qeQRY_NO_PARAMS		0x0080	// Disallow parameters. 
#define qeQRY_BIG_ICONS		0x0100	// Use big icons in icon bar. 
#define qeQRY_VALIDATE		0x0200	// Use database to validate SQL. 
#define qeQRY_SAMPLE		0x0400	// Show sample values in conditions 
					//	dialog box. 
//---------------------------------------------------------------------------
// Definitions for init_dialog parameter 

#define qeQRY_DEFAULT		1	// Bring up the default initial 
					//	dialog box. 
#define qeQRY_FILE		2	// File dialog box. 
#define qeQRY_JOIN		3	// Join dialog box. 
#define qeQRY_SELECT		4	// Select dialog box. 
#define qeQRY_ORDER		5	// Order By dialog box. 
#define qeQRY_WHERE		6	// Where By dialog box. 
#define qeQRY_GROUP		7	// Group dialog box. 
#define qeQRY_HAVING		8	// Having dialog box. 
#define qeQRY_TEXT		9	// Edit query text dialog box. 
//--------------------------------------------------------------------------
qeAPI(qeHANDLE) qeQryAllocate (
	qeHANDLE	hdbc,		// Connection handle 
	qeLPSTR		statement);	// Statement text, may be NULL 
// RETURNS: a handle to a query builder statement. 
//--------------------------------------------------------------------------
qeAPI(qeHANDLE) qeQryPrepare (
	qeHANDLE	hqry);		// Query Builder handle 
// Invokes the Query Builder. RETURNS: a handle for statement execution. 
// Call qeSQLExecute to execute the statement. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQryFree (
	qeHANDLE	hqry);		// Query Builder handle 
// Frees any memory associated with the Query Builder statement. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeHANDLE) qeQryOpenQueryFile (
	qeLPSTR		pathname);	// File pathname 
// RETURNS: a handle to a query builder statement taken from the query file. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySaveQueryFile (
	qeHANDLE	hqry,		// Query Builder handle 
	qeLPSTR		pathname);	// File pathname, may be NULL 
// Writes a query out to a file.  If pathname is NULL, 
// hqry must have a file name assoicated with it. 
// If the query was initially read from a query file, 
// the contents of the query file which do not deal 
// with the query or the parameters are preserved. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeHANDLE) qeQryGetHdbc (
	qeHANDLE	hqry);		// Query Builder handle 
// RETURNS: the hdbc associated with a Query Builder statement. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetHdbc (
	qeHANDLE	hqry,		// Query Builder handle 
	qeHANDLE	hdbc);		// Connection handle 
// Sets the hdbc associated with a Query Builder statement. 
// RETURNS: a result code.
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeQryGetSource (
	qeHANDLE	hqry);		// Query Builder handle 
qeAPI(qeSTATUS) qeQryGetSourceBuf (
	qeHANDLE	hqry,		// Query Builder handle 
	qeLPSTR		source_name);	// Buffer for source name 
// Gets the source name associated with a query.  This is the name used in 
// the query file that indicates which data source name the query file is 
// meant to operate on. 
// RETURNS: the source name, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetSource (
	qeHANDLE	hqry,		// Query Builder handle 
	qeLPSTR		source);	// New source name for hqry 
// Sets the source name associated with a query.  This is the name used in 
// the query file that indicates which data source name the query file is 
// meant to operate on. 
// RETURNS: a result code.  
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeQryGetStmt (
	qeHANDLE	hqry);		// Query Builder handle 
qeAPI(qeSTATUS) qeQryGetStmtBuf (
	qeHANDLE	hqry,		// Query Builder handle 
	qeLPSTR		stmt);		// Statement text returned 
// RETURNS: the statement text associated with a Query Builder stmt, or 
// a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetStmt (
	qeHANDLE	hqry,		// Query Builder handle 
	qeLPSTR		stmt);		// Statement text 
// Sets the statement text associated with a Query Builder stmt. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(short) qeQryGetNumParams (
	qeHANDLE	hqry);		// Query Builder handle 
// RETURNS: the number of parameters associated with a Query Builder stmt. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetNumParams (
	qeHANDLE	hqry,		// Query Builder handle 
	short		num_params);	// Number of parameters 
// Sets the number of parameters associated with a Query Builder stmt. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeQryGetFileName (
	qeHANDLE	hqry);		// Query Builder handle 
qeAPI(qeSTATUS) qeQryGetFileNameBuf (
	qeHANDLE	hqry,		// Query Builder handle 
	qeLPSTR		file_name);	// File name returned 
// RETURNS: the file name associated with a Query Builder stmt, or a result 
// code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetFileName (
	qeHANDLE	hqry,		// Query Builder handle 
	qeLPSTR		file_name);	// File name 
// Sets the file name associated with a Query Builder stmt. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeQryGetFileOffset (
	qeHANDLE	hqry);		// Query Builder handle 
// RETURNS: the offset within the query file of extra information 
// associated with a Query Builder stmt. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeQryGetParamName (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num);	// Parameter number 
qeAPI(qeSTATUS) qeQryGetParamNameBuf (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	qeLPSTR		param_name);	// Parameter name returned 
// RETURNS: the name associated with a parameter number in a Query Builder 
// statement, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetParamName (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	qeLPSTR		param_name);	// Parameter name 
// Sets the name associated with a parameter number in a Query Builder stmt. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(short) qeQryGetParamType (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num);	// Parameter number 
// RETURNS: the type associated with a parameter number in a Query Builder 
// statement. 
//--------------------------------------------------------------------------
// Definition of datatypes for parameters 
#define qeQRYPARM_CHAR		1
#define qeQRYPARM_NUM		2
#define qeQRYPARM_DATE		3
#define qeQRYPARM_TIME		4
#define qeQRYPARM_DATETIME	5
#define qeQRYPARM_LOGICAL	6
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetParamType (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	short		param_type);	// Parameter type 
// Sets the type associated with a parameter number in a Query Builder stmt. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeQryGetParamPrompt (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num);	// Parameter number 
qeAPI(qeSTATUS) qeQryGetParamPromptBuf (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	qeLPSTR		param_prompt);	// Parameter prompt returned 
// RETURNS: the prompt associated with a parameter number in a Query Builder 
// statement, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetParamPrompt (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	qeLPSTR		param_prompt);	// Parameter prompt 
// Sets the prompt associated with a parameter number in a Query Builder 
// statement. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeQryGetParamDefault (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num);	// Parameter number 
qeAPI(qeSTATUS) qeQryGetParamDefaultBuf (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	qeLPSTR		param_default);	// Parameter default value returned 
// RETURNS: the default value associated with a parameter number in a Query 
// Builder statement, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetParamDefault (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	qeLPSTR		param_default);	// Parameter default value 
// Sets the default value associated with a parameter number in a Query 
// Builder statement. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeLPSTR) qeQryGetParamFormat (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num);	// Parameter number 
qeAPI(qeSTATUS) qeQryGetParamFormatBuf (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	qeLPSTR		param_fmt);	// Parameter format string returned 
// RETURNS: the format string associated with a parameter number in a Query 
// Builder statement, or a result code if using the Buf version. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeQrySetParamFormat (
	qeHANDLE	hqry,		// Query Builder handle 
	short		param_num,	// Parameter number 
	qeLPSTR		param_fmt);	// Parameter format string 
// Sets the format string associated with a parameter number in a Query 
// Builder statement. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------

//---------
// Options 
//---------

//---------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetQueryTimeout (
	qeHANDLE	hdbc,		// Connection Handle 
	long		seconds);	// Query timeout, in seconds 
					//	Zero for no timeout 
// Sets the query timeout for statements created after this function is 
// called. 
// RETURNS: a result code. 
//---------------------------------------------------------------------------
qeAPI(long) qeGetQueryTimeout (
	qeHANDLE	hdbc);		// Connection Handle 
// Gets the query timeout for statements. 
// RETURNS: query timeout 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetTableCaching (
	qeHANDLE	hdbc,		// Connection Handle 
	short		setting);	// Table cache options: see below 
// Sets the table caching for this connection. 
// RETURNS: a result code. 
//---------------------------------------------------------------------------
qeAPI(short) qeGetTableCaching (
	qeHANDLE	hdbc);		// Connection Handle 
// Gets the table caching for this connection. 
// RETURNS: caching setting. 
//---------------------------------------------------------------------------
// Definitions for qeSetTableCaching setting 

#define qeCACHE_PERMANENT	1	// Turn caching on and set .INI file 
					//	to have it remain on across 
					//	sessions 
#define qeCACHE_SESSION		2	// Turn caching on for this session 
#define qeCACHE_OFF		3	// Turn caching off and set INIT file
					//	to have it remain off across 
					//	sessions 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetCacheFileName (
	qeHANDLE	hdbc,		// Connection Handle 
	qeLPSTR		file_name);	// File name for table caching 
					//	NULL for a system generated 
					//	file name 
// Sets the table cache file for this connection. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetMaxRows (
	qeHANDLE	hdbc,		// Connection Handle 
	long		max_rows);	// Maximum number of row statement 
					// will return 
// Sets the maximum number of rows to return for select statements 
// created after this function is called. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(long) qeGetMaxRows (
	qeHANDLE	hdbc);		// Connection Handle 
// Gets the maximum number of rows to return for select statements. 
// RETURNS: the max rows setting. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetLockOptions (
	qeHANDLE	hdbc,		// Connection Handle 
	short		option);	// Lock option: see below 
// Sets the locking options for the statement. 
// RETURNS: a result code. 
//---------------------------------------------------------------------------
qeAPI(short) qeGetLockOptions (
	qeHANDLE	hdbc);		// Connection Handle 
// Gets the locking options for the statement. 
// RETURNS: Locking options. 
//---------------------------------------------------------------------------
// Values for qeSetLockOptions 
#define	qeLOCK_NO_OPTIONS  0	// Neither comparing nor refreshing the row  
				// wil be done				     
#define qeLOCK_COMPARE	   1	// Lock will compare current buffer with row 
				// in logfile. Raise a warning if different. 
#define qeLOCK_REFRESH	   2	// Lock will refresh the logfile with new    
				//	values.				     
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetAutoUpdate (
	qeHANDLE	hdbc,		// Connection Handle 
	short		option);	// TRUE or FALSE; TRUE: generate 
					// updates or inserts for changed 
					// columns when user moves off row 
					// without an explicit update. 
// Determines what is done when moving off a row with modified columns and 
// no explict update or insert has been done. 
// RETURNS: a result code. 
//--------------------------------------------------------------------------
qeAPI(short) qeGetAutoUpdate (
	qeHANDLE	hdbc);		// Connection Handle 
// Returns what is done when moving off a row with modified columns and 
// no explict update or insert has been done. 
// RETURNS: Current auto update option setting from list below.
//--------------------------------------------------------------------------
// Values for qeSet/GetAutoUpdate 
#define qeAUTOUPD_DISCARD	1	// Modifications to the current 
					// record will be discarded when 
					// user moves off record. 
#define qeAUTOUPD_DEFER		2	// Modifications to the current 
					// will be saved when user 
					// moves off record.  They may 
					// be applied to the database or 
					// discarded by later calls. 
#define qeAUTOUPD_UPDATE	3	// Modifications to the current 
					// record are immediately sent to 
					// the database. 
//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetIsolationLevel (
	qeHANDLE	hdbc,		// Connection Handle 
	short		level);		// Isolation level to set 
// Sets the isolation level for the connection. 
// RETURNS: A result code. 
//--------------------------------------------------------------------------
qeAPI(short) qeGetIsolationLevel (
	qeHANDLE	hdbc);		// Connection Handle 
// Returns the isolation level for the connection. 
// RETURNS: an isolation level. 
//--------------------------------------------------------------------------
qeAPI(short) qeGetSupportedIsolationLevels (
	qeHANDLE	hdbc);		// Connection Handle 
// Returns the legal isolation levels for the connection. 
// RETURNS: a short with bits set for all supported isolation levels.
//--------------------------------------------------------------------------
// Values for qeSet/GetIsolationLevel 
#define qeISO_READ_UNCOMMITTED  0x0001 // Dirty read, non-repeatable read, 
				       // phantoms possible. 
#define qeISO_READ_COMMITTED    0x0002 // Non-repeatable read, phantoms 
				       // possible 
#define qeISO_REPEATABLE_READ   0x0004 // Repeatable read, phantoms possible 
#define qeISO_SERIALIZABLE      0x0008 // Repeatable read, no phantoms 
#define qeISO_VERSIONING        0x0010 // Repeatable read, no phantoms, 
				       // more concurrency. 

//--------------------------------------------------------------------------
qeAPI (qeSTATUS) qeRecUndo (
	qeHANDLE	hstmt);		// Statement handle 
// Undo all deferred (not applied to database by 
// by qeRecUpdate) change to the current row. 
// RETURNS: a result code. 

//--------------------------------------------------------------------------
qeAPI (qeSTATUS) qeUndoAll (
	qeHANDLE	hstmt);		// Statement handle 
// Undo all deferred (not applied to database by 
// by qeRecUpdate) changes to the statement. 
// Equivalent to qeRecUndo on every result row. 
// RETURNS: a result code. 

//--------------------------------------------------------------------------
qeAPI (qeSTATUS) qeApplyAll (
	qeHANDLE	hstmt);		// Statement handle 
// Apply all deferred (not applied to database by 
// by qeRecUpdate) changes for the statement to the database. 
// Equivalent to qeRecUpdate on every result row. 
// RETURNS: a result code. 



//--------------------------------------------------------------------------
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// The following section is for code which belongs in QELIB.H, but which we 
// will not expose to our users. 

qeAPI(qeSTATUS) qeOEMSetLicenseInfo (
	qeLPSTR		filename,
	qeLPSTR		password);
// Set license info for an OEM to use their own license file 

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeMakeValidName (
	qeHANDLE	hdbc,
	qeLPSTR		name,
	short		column);
// Make the column or table name into a valid name by adding the column 
// quoting character when necessary 


//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeSetDllPrefix (
	qeLPSTR prefix);
// Sets the DLL prefix for driver dlls 

qeAPI(qeSTATUS) qeRecInsert ( // ????? Still exists??? 
	qeHANDLE hstmt,		// Statement handle 
	long	record_num);	// Position for the new record 
// Insert a new record using values in the current buffer. The values 
// may be loaded using qePutChar 



// Special DBASE-only funtions 

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeGetTags (
	qeHANDLE hdbc,
	qeLPSTR	filename,
	qeLPSTR	names);
// Get index tagnames from a dbase MDX file 
// The names buffer must be big enough to hold all 47 names (11 chars each) 
// The names are returned as NULL terminated values with double nulls at 
// the end 

#define qeMDX_DESC	0x8	// Index is a descending index 
#define qeMDX_UNIQUE	0x40	// Only unique entries are indexed 

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeGetExp (
	qeHANDLE hdbc,
	qeLPSTR	filename,
	qeLPSTR	tag,
	qeLPSTR	exp,
	short	*flags);
// Get an expression from a dBASE index filename (NDX) or tag (MDX) 
// Expression must be larger enough to hold the maximum dBASE index 
// expression 

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeIDelete (
	qeHANDLE hdbc,
	qeLPSTR	dbname,
	qeLPSTR	filename,
	qeLPSTR	tag);
// Delete a dBASE index file (NDX or MDX) or an index tag (MDX) 

//--------------------------------------------------------------------------
qeAPI(qeSTATUS) qeIBuild (
	qeHANDLE hdbc,
	qeLPSTR	dbname,
	qeLPSTR	iname,
	qeLPSTR	tag,
	qeLPSTR	exp,
	short	flags);
// Build a dBASE index 

qeAPI(qeSTATUS) qeCharSubstitute (
	qeHANDLE	hdbc,		// Connection handle 
	qeLPSTR		stmt,		// String to be translated 
	char		oldchar,	// Character to search for 
	char		newchar);	// Character to replace with 
// Substitute newchar for every occurrence of oldchar in stmt. 
//--------------------------------------------------------------------------
#define qeQBLD_BIGICONS	    0x2		// Use big icons 
#define qeQBLD_DBVALIDATE   0x4         // Use database to validate 
#define qeQBLD_DBSAMPLES    0x8         // Show sample values in conditions 

qeAPI(qeSTATUS) qeDirectQueryBuilder (
	qeHANDLE hdbc,		// QELIB connection handle 
	HWND	parentWindow,	// Parent window in calling app 
	qeLPSTR	inSelect,	// Incoming Select statement, may be null 
	qeLPSTR	inParms,	// Incoming Parameter definition, may be null 
	qeLPSTR	outSelect,	// Buffer for outgoing Select statement 
	qeLPSTR outParms,	// Buffer for outgoing Parmaeter definition 
	unsigned short options,	// Must be 0 at this time 
	unsigned short pref,	// Initial user preferences - recommend 14 
	unsigned short initDialog); // Initial dialog box - must be 0 
// Run the Query Builder 
// Returns 1 if user clicked OK, 0 if user clicked Cancel. 
// If returns 0, no parameter values have been changed.  In particular, 
// the outSelect has not been set. 

// Query builder Parameter string definition. 

// inParms and outParms contain the definition of the parameters in the 
// Select statement.  There may be any number of parameters defined for 
// a select statement.  The definitions are concatenated together.  The 
// end of the set of definitions is specified by a data type value of 0. 
// Each parameter definition is made up of: 

//	Data type (1-byte)	The parameter's data type.  The values 
//				are the ones defined for the qeSetParm 
//				function's parm_type.  A value of 0 means 
//				there are no more parameter definitions. 
//	Parm name (0-term str)	The parameter's name.  This must correspond 
//				to a name following the ? char in the Select 
//				stmt. 
//	Parm prompt (0-term str) The prompt to use in dialog boxes when 
//				requesting a parameter value from the user. 
//	Parm default (0-term str) The default value for the parameter. 
//	Parm format (0-term str) The format string to use with this parameter. 


qeAPI (qeSTATUS) qeSQLParse (
	qeHANDLE	hdbc,		// Connection handle 
	qeLPSTR		stmt,		// Statement to parse 
	qeLPVOID FAR *	parsed);	// Parsed structure returned 
// Parse a statement and return the internal parsed structure. 

qeAPI (void) qeSQLParseFree (
	qeLPVOID	parsed);	// Parsed structure to be freed 
// Frees internal parsed structure. 

qeAPI (short) qeCMGConFlags (
	qeHANDLE	hdbc);		// Connection handle 
// Returns the connection flags for a hdbc. 

qeAPI (qeSTATUS) qeLockChange (
	qeHANDLE	hstmt);		// Statement handle 
// Log the current record, which was changed since last logged. 

qeAPI (qeSTATUS) qeUpperFunction (
	qeHANDLE	hdbc,		// Connection handle 
	qeLPSTR		upperFunction);	// UPPER function 
// Return the upper function for this connection. 

qeAPI (qeSTATUS) qeDBMS (
	qeHANDLE	hdbc,		// Connection handle 
	qeLPINT		DBMSType);	// Which DBMS 
// Return which DBMS for this connection. 

qeAPI (qeSTATUS) qeDBext (
	qeHANDLE	hdbc,		// Connection handle 
	qeLPSTR		dbExt);		// Database extension 
// Return the database extension for this connenction. 
qeAPI(qeSTATUS) qeSetCurrent (
	qeHANDLE hdbc,		// Connection handle 
	short options,		// Options that determine which context is 
				// to be changed.  See defines below.
	qeLPSTR name);		// Name of new context: Database name, User 
				// name, or Current Working Directory path. 

// Changes the Current Database, Current User, or Current Working Directory. 
// Returns a status code. 
// For File-based databases, changing the Current User changes the current 
// working directory.  This will effect the items returned by subsequent 
// calls to qeTables.  This function also affects the execution of SQL 
// statements.  Unqualified file names, as in SELECT * FROM EMP, will be 
// searched for in the current context. 
// For Table-based databases changing the Current User has very little 
// effect.  Most DBMS do not support this concept. QELIB saves the Current 
// User, so a subsequent call to qeGetCurrent will return the last value 
// sent to qeSetCurrent. Also, subsequent calls to qeTables with 
// user_pattern == "" will return tables owned by the Current User.  However,
// unqualified table names in FROM clauses will NOT be affected by calls to 
// qeSetCurrent.  Unqualified table names are assumed to be owned by the 
// Defualt User, the User Name assigned when the qeConnect occurred. 
// Immediately after calling qeConnect, a call to qeGetCurrent will return 
// the Default User. 

// OPTIONS: Values that may be used for the options parameter to 
// qeSetCurrent. The value sent determines which current context is to be 
// changed. 

#define qeCUR_DATABASE	6	// For Table-based sources, changes the 
				// current database.  For File-based sources,
				// changes the current disk drive. 
#define qeCUR_USER	7	// For Table-based sources, changes the 
				// current User.  For File-based sources, 
				// changes the current working directory. 

//--------------------------------------------------------------------------

qeAPI(qeLPSTR) qeGetCurrent (
	qeHANDLE hdbc,		// Connection handle
	short options);		// Options that determine which context is
				// to be retrieved.  See above definitions.

qeAPI(qeSTATUS) qeGetCurrentBuf (
	qeHANDLE hdbc,		// Connection handle
	short options,		// Options that determine which context is
				// to be retrieved.  See above definitions.
	qeLPSTR name);		// Returned context name: Database name, User
				// name, or Current Working Directory path.

// Gets the Current Database, Current User, or Current Working Directory. 
// qeGetCurrent returns the string, qeGetCurrentBuf returns a status code 
// and writes the returned value in the provided buffer. 

// If options == qeCurUser:
//	For File-based databases, returns the current working directory. 
//	For Table-based databases, returns the Current User. 
// Immediately after calling qeConnect, a call to qeGetCurrent returns the 
// Default User.  Once qeSetCurrent has been called, qeGetCurrent returns 
// the User Name last passed to qeSetCurrent. 

// NOTES FOR CONSTRUCTING FROM CLAUSES: 
// For File-based databases, if simple file names are given (e.g. EMP.DBF), 
// the file is assumed to be in the Current Working Directory.  The Current 
// Working Directory can be set by calling qeSetCurrent. 
// For Table-based databases, if unqualified names are given, e.g. EMP, the 
// tables are assumed to be owned by the Default User.  The Default User is 
// NOT affected by calls to qeSetCurrent.  The suggested algorithm is: 
//	1 - Immediately after calling qeConnect, call qeGetCurrent to get 
//	    the Default User and save the name.
//	2 - When constructing FROM clauses, call qeGetCurrent to get the 
//	    Current User.  If the Current User is not equal to the Default 
//	    User, qualify the Table name with the Current User Name. 

// Values that may be used for the options parameter are the same as 
// those for qeSetCurrent. The value sent determines which 
// current context name is to be returned. 
// #define qeCUR_DATABASE 6      For Table-based sources, returns the 
//				 current database.  For File-based sources, 
//				 changes the current disk drive. 
// #define qeCUR_USER	  7	 For Table-based sources, changes the 
//				 current User.  For File-based sources, 
//				 changes the current working directory. 
//---------------------------------------------------------------------------

#define qeQRY_VB_CALL		0x8000	//Query builder called from MultiLink

// Enables or disables processing escape (button). 
//	Currently unimplemented 
qeAPI (qeSTATUS) qeProcessEscape (
	qeHANDLE	hdbc,		// Connection handle 
	short		enable);	// Enable or disable: TRUE or FALSE 


// Sets the name, type, prompt, default, and format
//	associated with a parameter number
//	in a Query Builder stmt.
qeAPI (qeSTATUS) qeQrySetParam (
	qeHANDLE	hqry,		// Query Builder handle
	short		param_num,	// Parameter number
	qeLPSTR		param_name,	// Parameter name
	short		param_type,	// Parameter type
	qeLPSTR		prompt,		// Parameter prompt
	qeLPSTR		param_default,	// Parameter default value
	qeLPSTR		param_format);	// Parameter format string


// Purpose: Return the cmg capabilities structure for this connenction.
qeAPI (qeLPVOID) qeCmgCapabilities (
	qeHANDLE	hdbc);		// Connection handle

#endif
